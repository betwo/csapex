#ifndef NODE_BROADCASTS_H
#define NODE_BROADCASTS_H

/// PROJECT
#include <csapex/io/broadcast_impl.hpp>
#include <csapex/io/response_impl.hpp>
#include <csapex/param/parameter.h>
#include <csapex/serialization/serialization_fwd.h>

namespace csapex
{
class NodeBroadcasts : public BroadcastImplementation<NodeBroadcasts>
{
public:
    enum class NodeBroadcastType
    {
        None
    };

    NodeBroadcasts();
    NodeBroadcasts(NodeBroadcastType Broadcast_type);
    NodeBroadcasts(NodeBroadcastType Broadcast_type, AUUID uuid);

    void serialize(SerializationBuffer& data, SemanticVersion& version) const override;
    void deserialize(const SerializationBuffer& data, const SemanticVersion& version) override;

    std::string getType() const override
    {
        return "NodeBroadcasts";
    }

    NodeBroadcastType getBroadcastType() const;
    AUUID getUUID() const;

private:
    NodeBroadcastType broadcast_type_;

    AUUID uuid_;
};

}  // namespace csapex

#endif  // NODE_BROADCASTS_H
