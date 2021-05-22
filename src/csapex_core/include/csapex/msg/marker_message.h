#ifndef MARKER_MESSAGE_H
#define MARKER_MESSAGE_H

/// COMPONENT
#include <csapex/msg/message.h>
#include <csapex_core/csapex_core_export.h>

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
};

template <>
struct type<MarkerMessage>
{
    static std::string name()
    {
        return "Marker";
    }

    static TokenTypePtr makeTokenType()
    {
        return std::make_shared<TokenType>(
            // acceptor
            [](const TokenType& left, const TokenType& right) { return true; },
            // connector
            [](const TokenType& left, const TokenType& right) { return true; },
            // name
            type2name(typeid(MarkerMessage)),
            // description
            type<MarkerMessage>::name());
    }
};

}  // namespace connection_types
}  // namespace csapex

#endif  // MARKER_MESSAGE_H
