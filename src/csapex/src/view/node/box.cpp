/// HEADER
#include <csapex/view/node/box.h>

/// COMPONENT
#include "ui_box.h"
#include <csapex/model/node.h>
#include <csapex/factory/node_factory.h>
#include <csapex/msg/input.h>
#include <csapex/msg/output.h>
#include <csapex/model/node_worker.h>
#include <csapex/model/node_state.h>
#include <csapex/command/meta.h>
#include <csapex/command/dispatcher.h>
#include <csapex/view/node/node_adapter.h>
#include <csapex/utility/color.hpp>
#include <csapex/core/settings.h>

/// SYSTEM
#include <QDragMoveEvent>
#include <QGraphicsSceneDragDropEvent>
#include <QMenu>
#include <QTimer>
#include <QPainter>
#include <iostream>
#include <QSizeGrip>
#include <cmath>

using namespace csapex;

const QString NodeBox::MIME = "csapex/model/box";

NodeBox::NodeBox(Settings& settings, NodeWorker::Ptr worker, NodeAdapter::Ptr adapter, QIcon icon, QWidget* parent)
    : QWidget(parent), ui(new Ui::Box), settings_(settings), node_worker_(worker), adapter_(adapter), icon_(icon),
      down_(false), info_exec(nullptr), info_compo(nullptr), info_thread(nullptr), info_error(nullptr), is_placed_(false)
{
    worker->getNodeState()->flipped_changed->connect(std::bind(&NodeBox::flipSides, this));
    worker->getNodeState()->minimized_changed->connect(std::bind(&NodeBox::minimizeBox, this));

    QObject::connect(this, SIGNAL(updateVisualsRequest()), this, SLOT(updateVisuals()));

    grip_ = new QSizeGrip(this);

    setVisible(false);
}

NodeBox::~NodeBox()
{
    delete ui;
}


void NodeBox::setupUi()
{
    NodeWorkerPtr worker = node_worker_.lock();
    if(!worker) {
        return;
    }

    auto node = worker->getNode().lock();
    if(!node) {
        return;
    }
    node->checkConditions(true);

    if(!info_exec) {
        info_exec = new QLabel;
        info_exec->setProperty("exec", true);
        ui->infos->addWidget(info_exec);
    }
    if(!info_compo) {
        info_compo = new QLabel;
        info_compo->setProperty("component", true);
        ui->infos->addWidget(info_compo);
    }

    if(!info_thread) {
        info_thread = new QLabel;
        info_thread->setProperty("threadgroup", true);
        ui->infos->addWidget(info_thread);
    }

    if(!info_error) {
        info_error = new QLabel;
        info_error->setText("<img src=':/error.png' />");
        info_error->setProperty("error", true);
        info_error->setVisible(false);
        ui->infos->addWidget(info_error);
    }


    setupUiAgain();

    Q_EMIT changed(this);
}

void NodeBox::setupUiAgain()
{
    adapter_->doSetupUi(ui->content);

    setAutoFillBackground(false);

    setAttribute( Qt::WA_TranslucentBackground, true );
    setAttribute(Qt::WA_NoSystemBackground, true);
    //    setBackgroundMode (Qt::NoBackground, true);

    updateVisuals();
}

void NodeBox::construct()
{
    NodeWorkerPtr worker = node_worker_.lock();
    if(!worker) {
        return;
    }

    ui->setupUi(this);

    ui->input_layout->addSpacerItem(new QSpacerItem(16, 0));
    ui->output_layout->addSpacerItem(new QSpacerItem(16, 0));

    ui->enablebtn->setCheckable(true);
    ui->enablebtn->setChecked(worker->getNodeState()->isEnabled());

    QSize size(16, 16);
    ui->icon->setPixmap(icon_.pixmap(size));

    setFocusPolicy(Qt::ClickFocus);

    const UUID& uuid = worker->getUUID();
    setToolTip(uuid.c_str());

    setObjectName(uuid.c_str());

    ui->content->installEventFilter(this);
    ui->label->installEventFilter(this);

    setLabel(worker->getNodeState()->getLabel());

    QObject::connect(ui->enablebtn, SIGNAL(toggled(bool)), this, SIGNAL(toggled(bool)));

    QObject::connect(worker.get(), SIGNAL(destroyed()), this, SLOT(deleteLater()));

    worker->nodeStateChanged.connect([this]() { nodeStateChanged(); });
    QObject::connect(this, SIGNAL(nodeStateChanged()), this, SLOT(nodeStateChangedEvent()), Qt::QueuedConnection);

    worker->connectorCreated.connect([this](ConnectablePtr c) { registerEvent(c.get()); });
    worker->connectorRemoved.connect([this](ConnectablePtr c) { unregisterEvent(c.get()); });

    enabledChange(worker->isProcessingEnabled());
//    worker->enabled.connect([this](bool e){ enabledChange(e); });
    QObject::connect(this, SIGNAL(enabledChange(bool)), this, SLOT(enabledChangeEvent(bool)), Qt::QueuedConnection);

    worker->threadChanged.connect([this](){ updateThreadInformation(); });
    worker->errorHappened.connect([this](bool){ updateVisualsRequest(); });


    for(auto input : worker->getAllInputs()) {
        registerInputEvent(input.get());
    }
    for(auto output : worker->getAllOutputs()) {
        registerOutputEvent(output.get());
    }

    setupUi();
}

Node* NodeBox::getNode()
{
    NodeWorkerPtr worker = node_worker_.lock();
    if(!worker) {
        return nullptr;
    }
    return worker->getNode().lock().get();
}

NodeWorker* NodeBox::getNodeWorker()
{
    NodeWorkerPtr worker = node_worker_.lock();
    if(!worker) {
        return nullptr;
    }
    return worker.get();
}

NodeAdapter::Ptr NodeBox::getNodeAdapter()
{
    return adapter_;
}


void NodeBox::updateBoxInformation(Graph* graph)
{
    updateComponentInformation(graph);
    updateThreadInformation();
}

namespace {
void setStyleForId(QLabel* label, int id) {
    // set color using HSV rotation
    double hue =  (id * 77) % 360;
    double r = 0, g = 0, b = 0;
    __HSV2RGB__(hue, 1., 1., r, g, b);
    double fr = 0, fb = 0, fg = 0;
    double min = std::min(b, std::min(g, r));
    double max = std::max(b, std::max(g, r));
    if(min < 100 && max < 100) {
        fr = fb = fg = 255;
    }
    std::stringstream ss;
    ss << "QLabel { background-color : rgb(" << r << "," << g << "," << b << "); color: rgb(" << fr << "," << fg << "," << fb << ");}";
    label->setStyleSheet(ss.str().c_str());
}
}

void NodeBox::updateComponentInformation(Graph* graph)
{
    NodeWorkerPtr worker = node_worker_.lock();
    if(!worker) {
        return;
    }

    if(!settings_.get<bool>("display-graph-components", false)) {
        info_compo->setVisible(false);
        return;
    } else {
        info_compo->setVisible(true);
    }

    if(info_compo) {
        int compo = graph->getComponent(worker->getUUID());
        int level = graph->getLevel(worker->getUUID());
        std::stringstream info;
        info << "C:" << compo;
        info << "L:" << level;
        info_compo->setText(info.str().c_str());

        setStyleForId(info_compo, compo);
    }
}

void NodeBox::updateThreadInformation()
{
    NodeWorkerPtr worker = node_worker_.lock();
    if(!worker) {
        return;
    }

    if(!settings_.get<bool>("display-threads", false)) {
        info_thread->setVisible(false);
        return;
    } else {
        info_thread->setVisible(true);
    }

    if(info_thread && worker->thread()) {
        int id = worker->getNodeState()->getThreadId();
        std::stringstream info;
        if(settings_.get<bool>("threadless")) {
            info << "<i><b><small>threadless</small></b></i>";
            info_thread->setStyleSheet("QLabel { background-color : rgb(0,0,0); color: rgb(255,255,255);}");
        } else if(id < 0) {
            info << "T:" << -id;
            setStyleForId(info_thread, id);
        } else if(id == 0) {
            info << "<i><b><small>private</small></b></i>";
            info_thread->setStyleSheet("QLabel { background-color : rgb(255,255,255); color: rgb(0,0,0);}");
        } else {
            info << worker->getNodeState()->getThreadName();
            setStyleForId(info_thread, id);
        }
        info_thread->setText(info.str().c_str());
    }
}

void NodeBox::contextMenuEvent(QContextMenuEvent* e)
{
    Q_EMIT showContextMenuForBox(this, e->globalPos());
}

QBoxLayout* NodeBox::getInputLayout()
{
    return ui->input_layout;
}

QBoxLayout* NodeBox::getOutputLayout()
{
    return ui->output_layout;
}

QBoxLayout* NodeBox::getSlotLayout()
{
    return ui->slot_layout;
}

QBoxLayout* NodeBox::getTriggerLayout()
{
    return ui->signal_layout;
}

bool NodeBox::isError() const
{
    NodeWorkerPtr worker = node_worker_.lock();
    if(!worker) {
        return false;
    }
    return worker->isError();
}
ErrorState::ErrorLevel NodeBox::errorLevel() const
{
    NodeWorkerPtr worker = node_worker_.lock();
    if(!worker) {
        return ErrorState::ErrorLevel::NONE;
    }
    return worker->errorLevel();
}
std::string NodeBox::errorMessage() const
{
    NodeWorkerPtr worker = node_worker_.lock();
    if(!worker) {
        return "";
    }
    return worker->errorMessage();
}

void NodeBox::setLabel(const std::string& label)
{
    NodeWorkerPtr worker = node_worker_.lock();
    if(!worker) {
        return;
    }
    apex_assert_hard(worker->getNodeState());
    worker->getNodeState()->setLabel(label);
    ui->label->setText(label.c_str());
    ui->label->setToolTip(label.c_str());
}

void NodeBox::setLabel(const QString &label)
{
    NodeWorkerPtr worker = node_worker_.lock();
    if(!worker) {
        return;
    }
    worker->getNodeState()->setLabel(label.toStdString());
    ui->label->setText(label);
}

std::string NodeBox::getLabel() const
{
    NodeWorkerPtr worker = node_worker_.lock();
    if(!worker) {
        return "";
    }
    return worker->getNodeState()->getLabel();
}

void NodeBox::registerEvent(Connectable* c)
{
    if(c->isOutput()) {
        registerOutputEvent(dynamic_cast<Output*>(c));
    } else {
        registerInputEvent(dynamic_cast<Input*>(c));
    }
}

void NodeBox::unregisterEvent(Connectable*)
{
}

void NodeBox::registerInputEvent(Input* /*in*/)
{
    Q_EMIT changed(this);
}

void NodeBox::registerOutputEvent(Output* out)
{
    apex_assert_hard(out);

    Q_EMIT changed(this);
}

void NodeBox::resizeEvent(QResizeEvent *e)
{
    Q_EMIT changed(this);
}

void NodeBox::init()
{
    NodeWorkerPtr worker = node_worker_.lock();
    if(!worker) {
        return;
    }

    Point pt = worker->getNodeState()->getPos();
    move(QPoint(pt.x, pt.y));
}

bool NodeBox::eventFilter(QObject* o, QEvent* e)
{
    QMouseEvent* em = dynamic_cast<QMouseEvent*>(e);

    if(o == ui->label) {
        if(e->type() == QEvent::MouseButtonDblClick && em->button() == Qt::LeftButton) {
            Q_EMIT renameRequest(this);
            e->accept();

            return true;
        }
    }

    return false;
}

void NodeBox::enabledChangeEvent(bool val)
{
    ui->boxframe->setProperty("disabled", !val);

    ui->enablebtn->blockSignals(true);
    ui->enablebtn->setChecked(val);
    ui->enablebtn->blockSignals(false);

    refreshStylesheet();
}

void NodeBox::paintEvent(QPaintEvent* /*e*/)
{
    NodeWorkerPtr worker = node_worker_.lock();
    if(!worker || !adapter_) {
        return;
    }

    QString state;
    switch(worker->getState()) {
    case NodeWorker::State::IDLE:
        state = "idle"; break;
    case NodeWorker::State::ENABLED:
        state = "enabled"; break;
    case NodeWorker::State::FIRED:
        state = "fired"; break;
    case NodeWorker::State::PROCESSING:
        state = "processing"; break;
    default:
        state = "unknown"; break;
    }

    info_exec->setVisible(true);
    info_exec->setText(QString("<img src=\":/node_") + state + ".png\" alt=\"" + state + "\" title=\"" + state + "\" /> ");

    bool is_error = worker->isError() && worker->errorLevel() == ErrorState::ErrorLevel::ERROR;
    bool is_warn = worker->isError() && worker->errorLevel() == ErrorState::ErrorLevel::WARNING;

    bool error_change = ui->boxframe->property("error").toBool() != is_error;
    bool warning_change = ui->boxframe->property("warning").toBool() != is_warn;

    ui->boxframe->setProperty("error", is_error);
    ui->boxframe->setProperty("warning", is_warn);

    if(error_change || warning_change) {
        if(is_error) {
            QString msg = QString::fromStdString(worker->errorMessage());
            setToolTip(msg);
            info_error->setToolTip(msg);
            info_error->setVisible(true);
        } else if(is_warn) {
            QString msg = QString::fromStdString(worker->errorMessage());
            setToolTip(msg);
            info_error->setToolTip(msg);
            info_error->setVisible(true);
        } else {
            setToolTip(ui->label->text());
            info_error->setVisible(false);
        }

        refreshStylesheet();
    }

    //    resize(sizeHint());
}

void NodeBox::moveEvent(QMoveEvent* e)
{
    NodeWorkerPtr worker = node_worker_.lock();
    if(!worker) {
        return;
    }

    if(!is_placed_) {
        is_placed_ = true;
        return;
    }

    QPoint pos = e->pos();

    eventFilter(this, e);

    QPoint delta = pos - e->oldPos();

    worker->getNodeState()->setPos(Point(pos.x(), pos.y()));

    Q_EMIT moved(this, delta.x(), delta.y());
}

void NodeBox::triggerPlaced()
{
    Q_EMIT placed();
}

void NodeBox::setSelected(bool selected)
{
    ui->boxframe->setProperty("focused",selected);
    refreshStylesheet();
}

void NodeBox::keyPressEvent(QKeyEvent *)
{

}

void NodeBox::stop()
{
    QObject::disconnect(this);
    adapter_->stop();
}

void NodeBox::getInformation()
{
    Q_EMIT helpRequest(this);
}

void NodeBox::refreshStylesheet()
{
    apex_assert_hard(QThread::currentThread() == QApplication::instance()->thread());
    ui->boxframe->style()->unpolish(ui->boxframe);
    ui->boxframe->style()->polish(ui->boxframe);
    ui->boxframe->update();
}

void NodeBox::showProfiling(bool show)
{
    NodeWorkerPtr node = node_worker_.lock();
    if(node->isProfiling() != show) {
        node->setProfiling(show);
    }
}

void NodeBox::killContent()
{
    NodeWorkerPtr worker = node_worker_.lock();
    if(!worker) {
        return;
    }
    worker->killExecution();
}

void NodeBox::flipSides()
{
    NodeWorkerPtr worker = node_worker_.lock();
    if(!worker) {
        return;
    }
    updateVisuals();

    bool flip = worker->getNodeState()->isFlipped();
    Q_EMIT flipped(flip);
}

void NodeBox::minimizeBox()
{
    NodeWorkerPtr worker = node_worker_.lock();
    if(!worker) {
        return;
    }
    updateVisuals();

    bool minimize = worker->getNodeState()->isMinimized();
    Q_EMIT minimized(minimize);
}

void NodeBox::updateVisuals()
{
    NodeWorkerPtr worker = node_worker_.lock();
    if(!worker) {
        return;
    }
    bool flip = worker->getNodeState()->isFlipped();

    auto* layout = dynamic_cast<QGridLayout*>(ui->boxframe->layout());
    if(layout) {
        if(flip) {
            layout->addWidget(grip_, 3, 0, Qt::AlignBottom | Qt::AlignLeft);

        } else {
            layout->addWidget(grip_, 3, 2, Qt::AlignBottom | Qt::AlignRight);
        }
    }

    ui->boxframe->setProperty("flipped", flip);
    ui->boxframe->setLayoutDirection(flip ? Qt::RightToLeft : Qt::LeftToRight);
    ui->frame->setLayoutDirection(Qt::LeftToRight);

    if(isMinimizedSize()) {
        ui->frame->hide();
        ui->label->hide();
        ui->boxframe->setProperty("content_minimized", true);

    } else {
        ui->frame->show();
        ui->label->show();
        ui->boxframe->setProperty("content_minimized", false);
    }

    refreshStylesheet();

    resize(sizeHint());
}

bool NodeBox::isMinimizedSize() const
{
    NodeWorkerPtr worker = node_worker_.lock();
    if(!worker) {
        return false;
    }
    return worker->getNodeState()->isMinimized();
}

bool NodeBox::isFlipped() const
{
    NodeWorkerPtr worker = node_worker_.lock();
    if(!worker) {
        return false;
    }
    return worker->getNodeState()->isFlipped();
}

bool NodeBox::hasSubGraph()
{
    return false;
}

Graph::Ptr NodeBox::getSubGraph()
{
    throw std::runtime_error("cannot call getSubGraph() on Box! Check with hasSubGraph()!");
}

void NodeBox::nodeStateChangedEvent()
{
    NodeWorkerPtr worker = node_worker_.lock();
    if(!worker) {
        return;
    }
    minimizeBox();

    bool enabled = worker->getNodeState()->isEnabled();
    worker->setProcessingEnabled(enabled);
    ui->label->setEnabled(enabled);

    enabledChange(worker->getNodeState()->isEnabled());

    setLabel(worker->getNodeState()->getLabel());
    ui->label->setToolTip(worker->getUUID().c_str());

    updateThreadInformation();

    auto pt = worker->getNodeState()->getPos();
    move(QPoint(pt.x, pt.y));
}
/// MOC
#include "../../../include/csapex/view/node/moc_box.cpp"