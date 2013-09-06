/// HEADER
#include <csapex/command_move_fulcrum.h>

/// COMPONENT
#include <csapex/command.h>
#include <csapex/graph.h>

using namespace csapex::command;

MoveFulcrum::MoveFulcrum(int connection_id, int fulcrum_id, const QPoint &from, const QPoint &to)
    : connection_id(connection_id), fulcrum_id(fulcrum_id), from(from), to(to)
{
}

bool MoveFulcrum::execute()
{
    Graph::root()->getConnectionWithId(connection_id)->moveFulcrum(fulcrum_id, to);
    return true;
}

bool MoveFulcrum::undo()
{
    Graph::root()->getConnectionWithId(connection_id)->moveFulcrum(fulcrum_id, from);
    return true;
}

bool MoveFulcrum::redo()
{
    return execute();
}