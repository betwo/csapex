#ifndef NODE_SERVER_H
#define NODE_SERVER_H

/// PROJECT
#include <csapex/model/model_fwd.h>
#include <csapex/io/remote_io_fwd.h>
#include <csapex/model/observer.h>
#include <csapex/utility/uuid.h>

/// SYSTEM
#include <unordered_map>

namespace csapex
{
class NodeServer : public Observer
{
public:
    NodeServer(SessionPtr session);
    ~NodeServer() override;

    void startObservingNode(const NodeFacadeImplementationPtr& graph);
    void stopObservingNode(const NodeFacadeImplementationPtr& graph);

private:
    SessionPtr session_;
    ConnectorServerPtr connector_server_;

    std::unordered_map<AUUID, io::ChannelPtr, AUUID::Hasher> channels_;
};
}  // namespace csapex

#endif  // NODE_SERVER_H
