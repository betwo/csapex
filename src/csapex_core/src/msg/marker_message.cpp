/// HEADER
#include <csapex/msg/marker_message.h>

using namespace csapex;
using namespace connection_types;

MarkerMessage::MarkerMessage(const std::string& name, Stamp stamp) : Message(name, "/", stamp)
{
}

bool MarkerMessage::canConnectTo(const TokenType*) const
{
    return true;
}

bool MarkerMessage::acceptsConnectionFrom(const TokenType*) const
{
    return true;
}
