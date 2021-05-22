#ifndef GRAPHIO_H
#define GRAPHIO_H

/// COMPONENT
#include <csapex/model/graph.h>

/// PROJECT
#include <csapex/factory/factory_fwd.h>
#include <csapex/data/point.h>
#include <csapex/profiling/profilable.h>
#include <csapex/serialization/serialization_fwd.h>
#include <csapex/model/connection_description.h>
#include <csapex/utility/yaml.h>

/// SYSTEM
#include <unordered_map>

namespace csapex
{
class CSAPEX_CORE_EXPORT GraphIO : public Profilable
{
public:
    GraphIO(GraphFacadeImplementation& graph, NodeFactoryImplementation* node_factory, bool throw_on_error = false);

public:
    // options
    void setIgnoreForwardingConnections(bool ignore);

    // api
    void saveSettings(YAML::Node& yaml);
    void loadSettings(const YAML::Node& doc);

    Snippet saveGraph();
    void saveGraphTo(YAML::Node& yaml);

    void loadGraph(const Snippet& doc);
    void loadGraphFrom(const YAML::Node& doc);

    Snippet saveSelectedGraph(const std::vector<UUID>& nodes);

    std::unordered_map<UUID, UUID, UUID::Hasher> loadIntoGraph(const Snippet& blueprint, const csapex::Point& position, SemanticVersion version = {});

public:
    csapex::slim_signal::Signal<void(const GraphFacade&, YAML::Node& e)> saveViewRequest;
    csapex::slim_signal::Signal<void(GraphFacade&, const YAML::Node& n)> loadViewRequest;

private:
    void saveNodes(YAML::Node& yaml);
    void loadNodes(const YAML::Node& doc, SemanticVersion version);
    void loadNode(const YAML::Node& doc, SemanticVersion version);

    void saveConnections(YAML::Node& yaml);
    void loadConnections(const YAML::Node& doc, SemanticVersion version);
    void loadConnection(const YAML::Node& connection, SemanticVersion version);

    void saveFulcrums(YAML::Node& fulcrum, const ConnectionDescription& connection);
    void loadFulcrum(const YAML::Node& fulcrum, SemanticVersion version);

    void sendNotification(const std::string& notification);

protected:
    void saveNodes(YAML::Node& yaml, const std::vector<NodeFacadeImplementationPtr>& nodes);
    void saveConnections(YAML::Node& yaml, const std::vector<ConnectionDescription>& connections);

    void serializeNode(YAML::Node& doc, NodeFacadeImplementationConstPtr node_handle);
    void deserializeNode(const YAML::Node& doc, NodeFacadeImplementationPtr node_handle, SemanticVersion version);

    void loadConnection(ConnectorPtr from, const UUID& to_uuid, const std::string& connection_type, SemanticVersion version);

    UUID readNodeUUID(std::weak_ptr<UUIDProvider> parent, const YAML::Node& doc);
    UUID readConnectorUUID(std::weak_ptr<UUIDProvider> parent, const YAML::Node& doc);

private:
    GraphFacadeImplementation& graph_;
    NodeFactoryImplementation* node_factory_;

    std::unordered_map<UUID, UUID, UUID::Hasher> old_node_uuid_to_new_;
    double position_offset_x_;
    double position_offset_y_;

    bool ignore_forwarding_connections_;
    bool throw_on_error_;
};

}  // namespace csapex

#endif  // GRAPHIO_H
