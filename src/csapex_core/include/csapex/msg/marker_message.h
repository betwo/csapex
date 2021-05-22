#ifndef MARKER_MESSAGE_H
#define MARKER_MESSAGE_H

/// COMPONENT
#include <csapex/msg/message.h>

namespace csapex
{
namespace connection_types
{
class CSAPEX_CORE_EXPORT MarkerMessage : public Message
{
public:
    typedef std::shared_ptr<MarkerMessage> Ptr;

protected:
    MarkerMessage(const std::string& name, Stamp stamp);

public:
    bool canConnectTo(const TokenType* other_side) const override;
    bool acceptsConnectionFrom(const TokenType* other_side) const override;
};
template <>
struct type<MarkerMessage>
{
    static std::string name()
    {
        return "Marker";
    }
};

}  // namespace connection_types
}  // namespace csapex

#endif  // MARKER_MESSAGE_H
