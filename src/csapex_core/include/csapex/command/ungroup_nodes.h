#ifndef UNGROUP_NODES_H
#define UNGROUP_NODES_H

/// COMPONENT
#include "group_base.h"
#include <csapex/utility/yaml.h>

namespace csapex
{
namespace command
{
class CSAPEX_COMMAND_EXPORT UngroupNodes : public GroupBase
{
    COMMAND_HEADER(UngroupNodes);

public:
    UngroupNodes(const AUUID& graph_uuid, const UUID& uuid);

    std::string getDescription() const override;

    void serialize(SerializationBuffer& data, SemanticVersion& version) const override;
    void deserialize(const SerializationBuffer& data, const SemanticVersion& version) override;

    std::string getType() const override
    {
        return typeName();
    }
    static std::string typeName()
    {
        return type2nameWithoutNamespace(typeid(UngroupNodes));
    }

    void clear() override;

protected:
    void unmapConnections(AUUID parent_auuid, AUUID sub_graph_auuid);

protected:
    bool doExecute() override;
    bool doUndo() override;
    bool doRedo() override;

private:
    UUID uuid;

    SubgraphNodePtr subgraph;

    std::unordered_map<UUID, UUID, UUID::Hasher> old_connections_in;
    std::unordered_map<UUID, std::vector<UUID>, UUID::Hasher> old_connections_out;

    std::unordered_map<UUID, std::vector<UUID>, UUID::Hasher> old_signals_in;
    std::unordered_map<UUID, std::vector<UUID>, UUID::Hasher> old_signals_out;
};

}  // namespace command

}  // namespace csapex

#endif  // UNGROUP_NODES_H
