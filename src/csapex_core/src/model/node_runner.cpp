/// HEADER
#include <csapex/model/node_runner.h>

/// PROJECT
#include <csapex/model/node_handle.h>
#include <csapex/model/node_worker.h>
#include <csapex/model/node.h>
#include <csapex/scheduling/scheduler.h>
#include <csapex/scheduling/task.h>
#include <csapex/utility/assert.h>
#include <csapex/model/node_state.h>
#include <csapex/utility/thread.h>
#include <csapex/model/subgraph_node.h>
#include <csapex/utility/exceptions.h>

/// SYSTEM
#include <memory>
#include <iostream>

using namespace csapex;

NodeRunner::NodeRunner(NodeWorkerPtr worker)
  : worker_(worker)
  , nh_(worker->getNodeHandle())
  , scheduler_(nullptr)
  , paused_(false)
  , stepping_(false)
  , possible_steps_(0)
  , step_done_(false)
  , guard_(-1)
  , waiting_for_execution_(false)
  , waiting_for_step_(false)
  , suppress_exceptions_(true)
{
    nh_->getNodeState()->max_frequency_changed.connect([this]() {
        max_frequency_ = nh_->getNodeState()->getMaximumFrequency();

        if (max_frequency_ > 0.0) {
            nh_->getRate().setFrequency(max_frequency_);
        } else {
            nh_->getRate().setFrequency(0.0);
        }
    });
    max_frequency_ = nh_->getNodeState()->getMaximumFrequency();
    nh_->getRate().setFrequency(max_frequency_);
}

NodeRunner::~NodeRunner()
{
    stopObserving();

    if (scheduler_) {
        detach();
    }

    guard_ = 0xDEADBEEF;
}

void NodeRunner::measureFrequency()
{
    if (worker_->isProcessingEnabled()) {
        nh_->getRate().tick();
    }
}

void NodeRunner::reset()
{
    waiting_for_execution_ = false;
    waiting_for_step_ = false;
    remaining_tasks_.clear();

    execute_->setScheduled(false);
    check_parameters_->setScheduled(false);

    worker_->reset();
}

void NodeRunner::connectNodeWorker()
{
    NodeRunnerWeakPtr this_weak = std::dynamic_pointer_cast<NodeRunner>(shared_from_this());
    apex_assert_hard(this_weak.lock() != nullptr);

    check_parameters_ = std::make_shared<Task>(
        std::string("check parameters for ") + nh_->getUUID().getFullName(),
        [this_weak]() {
            if (auto self = this_weak.lock()) {
                self->checkParameters();
            }
        },
        0, this);
    execute_ = std::make_shared<Task>(
        std::string("process ") + nh_->getUUID().getFullName(),
        [this_weak]() {
            if (auto self = this_weak.lock()) {
                self->execute();
            }
        },
        0, this);

    notify_processed_ = std::make_shared<Task>(
        std::string("notify ") + nh_->getUUID().getFullName(),
        [this_weak]() {
            if (auto self = this_weak.lock()) {
                self->notify();
            }
        },
        0, this);

    observe(worker_->try_process_changed, [this]() { scheduleProcess(); });

    observe(worker_->outgoing_messages_processed, [this]() { schedule(notify_processed_); });

    observe(worker_->messages_processed, [this]() {
        measureFrequency();
        step_done_ = true;
        // TRACE worker_->getNode()->ainfo << "end step" << std::endl;
        end_step();
    });
}

void NodeRunner::assignToScheduler(Scheduler* scheduler)
{
    std::unique_lock<std::recursive_mutex> lock(mutex_);

    apex_assert_hard(scheduler_ == nullptr);
    apex_assert_hard(scheduler != nullptr);

    scheduler_ = scheduler;

    scheduler_->add(shared_from_this(), remaining_tasks_);
    nh_->getNodeState()->setThread(scheduler->getName(), scheduler->id());

    remaining_tasks_.clear();

    lock.unlock();

    stopObserving();

    lock.lock();

    // signals
    observe(scheduler->scheduler_changed, [this]() {
        NodeHandlePtr nh = nh_;
        nh->getNodeState()->setThread(scheduler_->getName(), scheduler_->id());
    });

    // node tasks
    connectNodeWorker();

    // parameter change
    observe(nh_->parameters_changed, [this]() { schedule(check_parameters_); });

    // processing enabled change
    observe(nh_->getNodeState()->enabled_changed, [this]() {
        if (!nh_->getNodeState()->isEnabled()) {
            waiting_for_execution_ = false;
        }
    });

    schedule(check_parameters_);

    // generic task
    observe(nh_->execution_requested, [this](std::function<void()> cb) { schedule(std::make_shared<Task>("anonymous", cb, 0, this)); });
}

Scheduler* NodeRunner::getScheduler() const
{
    return scheduler_;
}

void NodeRunner::scheduleProcess()
{
    apex_assert_hard(guard_ == -1);
    if (!paused_) {
        bool source = nh_->isSource();
        if (!source || !stepping_ || possible_steps_) {
            // execute_->setPriority(std::max<long>(0, worker_->getSequenceNumber()));
            // if(worker_->canExecute()) {
            if (!waiting_for_execution_) {
                if (!execute_->isScheduled()) {
                    schedule(execute_);
                }
            }
            //}
        }
    }
}

void NodeRunner::checkParameters()
{
    worker_->handleChangedParameters();
}

void NodeRunner::notify()
{
    worker_->notifyMessagesProcessedDownstream();
}

void NodeRunner::execute()
{
    if (stepping_ && possible_steps_ <= 0) {
        return;
    }

    apex_assert_hard(guard_ == -1);
    if (worker_->canExecute()) {
        if (max_frequency_ > 0.0) {
            const Rate& rate = nh_->getRate();
            double f = rate.getEffectiveFrequency();
            if (f > max_frequency_) {
                auto next_process = rate.endOfCycle();

                auto now = std::chrono::system_clock::now();

                if (next_process > now) {
                    scheduleDelayed(execute_, next_process);
                    waiting_for_execution_ = true;
                    return;
                }
            }
        }

        waiting_for_execution_ = false;

        nh_->getRate().startCycle();

        if (stepping_) {
            apex_assert_hard(possible_steps_);
        }

        possible_steps_--;

        try {
            if (worker_->canExecute()) {
                if (!worker_->startProcessingMessages()) {
                    possible_steps_++;
                }
            }
        } catch (const std::exception& e) {
            if (!suppress_exceptions_)
                throw;
            else {
                AUUID auuid = nh_->getAUUID();
                NOTIFICATION_AUUID(auuid, std::string("Node could not be executed: ") + e.what());
            }
        } catch (const Failure& e) {
            if (!suppress_exceptions_)
                throw;
            else {
                AUUID auuid = nh_->getAUUID();
                NOTIFICATION_AUUID(auuid, std::string("Node experienced failure: ") + e.what());
            }
        } catch (...) {
            if (!suppress_exceptions_)
                throw;
            else {
                AUUID auuid = nh_->getAUUID();
                NOTIFICATION_AUUID(auuid, "Node could not be executed: Unknown exception");
            }
        }
    } else {
        // can_step_++;
        waiting_for_execution_ = false;
    }
}

void NodeRunner::schedule(TaskPtr task)
{
    std::unique_lock<std::recursive_mutex> lock(mutex_);
    remaining_tasks_.push_back(task);

    if (scheduler_) {
        for (const TaskPtr& t : remaining_tasks_) {
            scheduler_->schedule(t);
        }
        remaining_tasks_.clear();
    }
}

void NodeRunner::scheduleDelayed(TaskPtr task, std::chrono::system_clock::time_point time)
{
    std::unique_lock<std::recursive_mutex> lock(mutex_);
    scheduler_->scheduleDelayed(task, time);
}

void NodeRunner::detach()
{
    std::unique_lock<std::recursive_mutex> lock(mutex_);

    if (scheduler_) {
        auto t = scheduler_->remove(this);
        remaining_tasks_.insert(remaining_tasks_.end(), t.begin(), t.end());
        scheduler_ = nullptr;
    }
}

bool NodeRunner::isPaused() const
{
    return paused_;
}

void NodeRunner::setPause(bool pause)
{
    paused_ = pause;
    if (!paused_) {
        scheduleProcess();
    }
}

bool NodeRunner::canStartStepping() const
{
    if (auto subgraph = std::dynamic_pointer_cast<SubgraphNode>(worker_->getNode())) {
        // if the node is an iterating sub graph, we need to wait until the current iteration is done...
        return !subgraph->isIterating();
    }
    return true;
}

void NodeRunner::setSteppingMode(bool stepping)
{
    bool can_start_stepping = true;
    if (stepping) {
        if (!canStartStepping()) {
            can_start_stepping = false;
            waiting_for_step_ = true;
            wait_for_step_connection_ = worker_->messages_processed.connect([this]() { stepping_enabled(); });
        }
    }

    stepping_ = stepping;
    possible_steps_ = 0;

    if (stepping_) {
        if (can_start_stepping) {
            stepping_enabled();
        }

    } else {
        scheduleProcess();
    }
}

void NodeRunner::step()
{
    if (waiting_for_step_) {
        wait_for_step_connection_.disconnect();
    }

    bool is_enabled = worker_->isProcessingEnabled();

    if (is_enabled) {
        possible_steps_++;
    }

    step_done_ = false;
    begin_step();

    bool can_perform_step = is_enabled && worker_->canProcess();

    if (!can_perform_step) {
        // TRACE worker_->getNode()->ainfo << "cannot step: "
        // TRACE                           << (worker_->canProcess() ? "can" : "cannot") << " process,"
        // TRACE                           << (worker_->canSend() ? "can" : "cannot") << " send,"
        // TRACE                           << (worker_->canReceive() ? "can" : "cannot") << " receive,"
        // TRACE                           << "processing " << (worker_->isProcessingEnabled() ? "is" : "is not") << " enabled"
        // TRACE                           << std::endl;
        step_done_ = true;
        end_step();
        return;
    }

    // TRACE worker_->getNode()->ainfo << "step" << std::endl;
    //    if(source) {
    scheduleProcess();
    //    }
}

bool NodeRunner::isStepping() const
{
    return stepping_;
}

bool NodeRunner::isStepDone() const
{
    return step_done_;
}

UUID NodeRunner::getUUID() const
{
    return worker_->getUUID();
}

void NodeRunner::setError(const std::string& msg)
{
    std::cerr << "error happened: " << msg << std::endl;
    worker_->setError(true, msg);
}

void NodeRunner::setSuppressExceptions(bool suppress_exceptions)
{
    suppress_exceptions_ = suppress_exceptions;
}

void NodeRunner::setNodeWorker(NodeWorkerPtr worker)
{
    worker_ = worker;
    connectNodeWorker();
}
