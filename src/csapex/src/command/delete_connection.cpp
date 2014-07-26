/// HEADER
#include <csapex/command/delete_connection.h>

/// COMPONENT
#include <csapex/command/command.h>

#include <csapex/msg/input.h>
#include <csapex/msg/output.h>
#include <csapex/model/graph.h>
#include <csapex/model/node.h>

/// SYSTEM
#include <boost/foreach.hpp>

using namespace csapex;
using namespace csapex::command;

DeleteConnection::DeleteConnection(Connectable* a, Connectable* b)
    : Meta("delete connection and fulcrums"), from_uuid(UUID::NONE), to_uuid(UUID::NONE)
{
    from = dynamic_cast<Output*>(a);
    if(from) {
        to = dynamic_cast<Input*>(b);
    } else {
        from = dynamic_cast<Output*>(b);
        to = dynamic_cast<Input*>(a);
    }
    apex_assert_hard(from);
    apex_assert_hard(to);

    from_uuid = from->getUUID();
    to_uuid = to->getUUID();
}

std::string DeleteConnection::getType() const
{
    return "DeleteConnection";
}

std::string DeleteConnection::getDescription() const
{
    return std::string("deleted connection between ") + from_uuid.getFullName() + " and " + to_uuid.getFullName();
}


bool DeleteConnection::doExecute()
{
    Connection::Ptr connection(new Connection(from, to));

    connection_id = graph_->getConnectionId(connection);

    locked = false;
    clear();
    add(graph_->deleteAllConnectionFulcrumsCommand(connection));
    locked = true;

    if(Meta::doExecute()) {
        graph_->deleteConnection(connection);
    }

    return true;
}

bool DeleteConnection::doUndo()
{
    if(!refresh()) {
        return false;
    }
    graph_->addConnection(Connection::Ptr(new Connection(from, to, connection_id)));

    return Meta::doUndo();
}

bool DeleteConnection::doRedo()
{
    if(!refresh()) {
        throw std::runtime_error("cannot redo DeleteConnection");
    }
    return doExecute();
}

bool DeleteConnection::refresh()
{
    Node* from_node = graph_->findNodeForConnector(from_uuid);
    Node* to_node = graph_->findNodeForConnector(to_uuid);

    from = NULL;
    to = NULL;

    if(!from_node || !to_node) {
        return false;
    }

    from = from_node->getOutput(from_uuid);
    to = to_node->getInput(to_uuid);

    apex_assert_hard(from);
    apex_assert_hard(to);

    return true;
}
