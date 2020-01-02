#ifndef NODE_FACADE_H
#define NODE_FACADE_H

/// COMPONENT
#include <csapex/model/model_fwd.h>
#include <csapex/model/observer.h>
#include <csapex/model/notifier.h>
#include <csapex/utility/uuid.h>
#include <csapex/utility/slim_signal.hpp>
#include <csapex/profiling/profiling_fwd.h>
#include <csapex/model/tracing_type.h>
#include <csapex/model/execution_state.h>
#include <csapex/model/connector_description.h>
#include <csapex/param/param_fwd.h>

namespace csapex
{
class CSAPEX_CORE_EXPORT NodeFacade : public Observer, public Notifier, public ErrorState
{
protected:
    NodeFacade();

public:
    ~NodeFacade() override;

    virtual std::string getType() const = 0;
    virtual UUID getUUID() const = 0;
    virtual AUUID getAUUID() const = 0;

    virtual bool isActive() const = 0;

    virtual bool isProxy() const;

    virtual bool isProcessingEnabled() const = 0;

    virtual bool isGraph() const = 0;
    virtual AUUID getSubgraphAUUID() const = 0;
    virtual GraphPtr getSubgraph() const = 0;

    virtual bool isSource() const = 0;
    virtual bool isSink() const = 0;
    virtual bool isProcessingNothingMessages() const = 0;

    virtual bool isParameterInput(const UUID& id) = 0;
    virtual bool isParameterOutput(const UUID& id) = 0;

    virtual bool isVariadic() const = 0;
    virtual bool hasVariadicInputs() const = 0;
    virtual bool hasVariadicOutputs() const = 0;
    virtual bool hasVariadicEvents() const = 0;
    virtual bool hasVariadicSlots() const = 0;

    virtual std::vector<ConnectorDescription> getInputs() const;
    virtual std::vector<ConnectorDescription> getOutputs() const;
    virtual std::vector<ConnectorDescription> getEvents() const;
    virtual std::vector<ConnectorDescription> getSlots() const;

    virtual std::vector<ConnectorDescription> getInternalInputs() const = 0;
    virtual std::vector<ConnectorDescription> getInternalOutputs() const = 0;
    virtual std::vector<ConnectorDescription> getInternalEvents() const = 0;
    virtual std::vector<ConnectorDescription> getInternalSlots() const = 0;

    virtual std::vector<ConnectorDescription> getExternalInputs() const = 0;
    virtual std::vector<ConnectorDescription> getExternalOutputs() const = 0;
    virtual std::vector<ConnectorDescription> getExternalEvents() const = 0;
    virtual std::vector<ConnectorDescription> getExternalSlots() const = 0;

    virtual std::vector<ConnectorDescription> getExternalConnectors() const;
    virtual std::vector<ConnectorDescription> getInternalConnectors() const;

    virtual ConnectorPtr getConnector(const UUID& id) const = 0;
    virtual ConnectorPtr getConnectorNoThrow(const UUID& id) const noexcept = 0;

    virtual ConnectorPtr getParameterInput(const std::string& name) const = 0;
    virtual ConnectorPtr getParameterOutput(const std::string& name) const = 0;

    virtual NodeCharacteristics getNodeCharacteristics() const = 0;

    virtual bool canStartStepping() const = 0;

    virtual bool isProfiling() const = 0;
    virtual void setProfiling(bool profiling) = 0;

    virtual ExecutionState getExecutionState() const = 0;

    virtual std::string getLabel() const = 0;

    virtual int getSchedulerId() const = 0;

    virtual double getExecutionFrequency() const = 0;
    virtual double getMaximumFrequency() const = 0;

    // Parameterizable
    virtual std::vector<param::ParameterPtr> getParameters() const = 0;
    virtual param::ParameterPtr getParameter(const std::string& name) const = 0;
    virtual bool hasParameter(const std::string& name) const = 0;

    // Debug Access
    virtual std::string getDebugDescription() const = 0;
    virtual std::string getLoggerOutput(ErrorState::ErrorLevel level) const = 0;

    // TODO: proxies
    virtual ProfilerPtr getProfiler() = 0;

    virtual NodeStatePtr getNodeState() const = 0;
    virtual NodeStatePtr getNodeStateCopy() const = 0;

    template <typename T>
    T readParameter(const std::string& name) const;
    template <typename T>
    void setParameter(const std::string& name, const T& value);

public:
    /// access to data for node adapters
    slim_signal::ObservableSignal<void(StreamableConstPtr)> raw_data_connection;

    slim_signal::Signal<void(NodeFacade* facade)> start_profiling;
    slim_signal::Signal<void(NodeFacade* facade)> stop_profiling;

    slim_signal::Signal<void(ConnectorDescription)> connector_created;
    slim_signal::Signal<void(ConnectorDescription)> connector_removed;

    slim_signal::Signal<void(ConnectorDescription)> connection_added;
    slim_signal::Signal<void(ConnectorDescription)> connection_removed;
    slim_signal::Signal<void(ConnectorDescription)> connection_start;

    slim_signal::Signal<void()> messages_processed;

    slim_signal::Signal<void(std::string)> label_changed;
    slim_signal::Signal<void(int)> scheduler_changed;

    slim_signal::Signal<void(NodeStatePtr state)> node_state_changed;
    slim_signal::Signal<void()> parameters_changed;
    slim_signal::Signal<void()> parameter_set_changed;
    slim_signal::Signal<void()> activation_changed;

    slim_signal::Signal<void(param::ParameterPtr)> parameter_added;
    slim_signal::Signal<void(param::ParameterPtr)> parameter_changed;
    slim_signal::Signal<void(param::ParameterPtr)> parameter_removed;

    slim_signal::Signal<void(std::vector<ConnectorDescription>)> external_inputs_changed;
    slim_signal::Signal<void(std::vector<ConnectorDescription>)> external_outputs_changed;
    slim_signal::Signal<void(std::vector<ConnectorDescription>)> external_events_changed;
    slim_signal::Signal<void(std::vector<ConnectorDescription>)> external_slots_changed;

    slim_signal::Signal<void(std::vector<ConnectorDescription>)> internal_inputs_changed;
    slim_signal::Signal<void(std::vector<ConnectorDescription>)> internal_outputs_changed;
    slim_signal::Signal<void(std::vector<ConnectorDescription>)> internal_events_changed;
    slim_signal::Signal<void(std::vector<ConnectorDescription>)> internal_slots_changed;

    slim_signal::Signal<void()> destroyed;

    slim_signal::Signal<void(NodeFacade* facade, TracingType type, std::shared_ptr<const Interval> stamp)> interval_start;
    slim_signal::Signal<void(NodeFacade* facade, std::shared_ptr<const Interval> stamp)> interval_end;
};

}  // namespace csapex

#endif  // NODE_FACADE_H
