#ifndef NODE_FACADE_PROXY_H
#define NODE_FACADE_PROXY_H

/// PROJECT
#include <csapex/model/node_facade.h>
#include <csapex/io/io_fwd.h>
#include <csapex/serialization/streamable.h>
#include <csapex/io/proxy.h>

/// SYSTEM
#include <unordered_map>

namespace csapex
{
class ProfilerProxy;

class CSAPEX_CORE_EXPORT NodeFacadeProxy : public NodeFacade, public Proxy
{
public:
    NodeFacadeProxy(const SessionPtr& session, AUUID uuid);

    ~NodeFacadeProxy() override;

    bool isProxy() const override;

    UUID getUUID() const override;
    AUUID getAUUID() const override;

    void setProfiling(bool profiling) override;

    bool isParameterInput(const UUID& id) override;
    bool isParameterOutput(const UUID& id) override;

    GraphPtr getSubgraph() const override;

    ConnectorPtr getConnector(const UUID& id) const override;
    ConnectorPtr getConnectorNoThrow(const UUID& id) const noexcept override;

    ConnectorPtr getParameterInput(const std::string& name) const override;
    ConnectorPtr getParameterOutput(const std::string& name) const override;

    // Parameterizable
    std::vector<param::ParameterPtr> getParameters() const override;
    param::ParameterPtr getParameter(const std::string& name) const override;
    bool hasParameter(const std::string& name) const override;

    // Debug Access
    std::string getLoggerOutput(ErrorState::ErrorLevel level) const override;

    ProfilerPtr getProfiler() override;

    NodeStatePtr getNodeState() const override;
    NodeStatePtr getNodeStateCopy() const override;

/**
 * begin: generate getters
 **/
#define HANDLE_ACCESSOR(_enum, type, function) type function() const override;

#define HANDLE_STATIC_ACCESSOR(_enum, type, function) HANDLE_ACCESSOR(_enum, type, function)
#define HANDLE_DYNAMIC_ACCESSOR(_enum, signal, type, function) HANDLE_ACCESSOR(_enum, type, function)
#define HANDLE_SIGNAL(_enum, signal)

#include <csapex/model/node_facade_proxy_accessors.hpp>
    /**
     * end: generate getters
     **/

    void createConnectorProxy(const ConnectorDescription& cd);
    void removeConnectorProxy(const ConnectorDescription& cd);

private:
    void handleBroadcast(const BroadcastMessageConstPtr& message) override;

    void createParameterProxy(param::ParameterPtr proxy) const;

private:
    AUUID uuid_;

    io::ChannelPtr node_channel_;

/**
 * begin: generate caches
 **/
#define HANDLE_ACCESSOR(_enum, type, function)
#define HANDLE_STATIC_ACCESSOR(_enum, type, function)                                                                                                                                                  \
    mutable bool has_##function##_;                                                                                                                                                                    \
    mutable type cache_##function##_;
#define HANDLE_DYNAMIC_ACCESSOR(_enum, signal, type, function)                                                                                                                                         \
    mutable bool has_##function##_;                                                                                                                                                                    \
    mutable type value_##function##_;
#define HANDLE_SIGNAL(_enum, signal)

#include <csapex/model/node_facade_proxy_accessors.hpp>
    /**
     * end: generate caches
     **/

    long guard_;

    NodeStatePtr state_proxy_;

    mutable std::vector<param::ParameterPtr> parameters_;
    mutable std::map<std::string, param::ParameterPtr> parameter_cache_;
    std::shared_ptr<ProfilerProxy> profiler_proxy_;

    std::unordered_map<UUID, ConnectorPtr, UUID::Hasher> remote_connectors_;

    // TODO: auto generate
    std::unordered_map<UUID, bool, UUID::Hasher> is_parameter_output_;
    std::unordered_map<UUID, bool, UUID::Hasher> is_parameter_input_;
};

}  // namespace csapex

#endif  // NODE_FACADE_PROXY_H
