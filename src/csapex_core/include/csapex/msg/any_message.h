#ifndef ANY_MESSAGE_H
#define ANY_MESSAGE_H

/// COMPONENT
#include <csapex/msg/message.h>
#include <csapex_core/csapex_core_export.h>

namespace csapex
{
namespace connection_types
{
struct CSAPEX_CORE_EXPORT AnyMessage : public Message
{
protected:
    CLONABLE_IMPLEMENTATION(AnyMessage);

public:
    typedef std::shared_ptr<AnyMessage> Ptr;

public:
    AnyMessage();

public:
    void serialize(SerializationBuffer& data, SemanticVersion& version) const override;
    void deserialize(const SerializationBuffer& data, const SemanticVersion& version) override;
};

template <>
struct type<AnyMessage>
{
    static std::string name()
    {
        return "Anything";
    }

    static TokenTypePtr makeTokenType()
    {
        return std::make_shared<TokenType>(
            // acceptor
            [](const TokenType& left, const TokenType& right) { return true; },
            // connector
            [](const TokenType& left, const TokenType& right) { return true; },
            // name
            type2name(typeid(AnyMessage)),
            // description
            type<AnyMessage>::name());
    }
};

}  // namespace connection_types

template <>
inline std::shared_ptr<connection_types::AnyMessage> makeEmpty<connection_types::AnyMessage>()
{
    static std::shared_ptr<connection_types::AnyMessage> instance(new connection_types::AnyMessage);
    return instance;
}

}  // namespace csapex

/// YAML
namespace YAML
{
template <>
struct CSAPEX_CORE_EXPORT convert<csapex::connection_types::AnyMessage>
{
    static Node encode(const csapex::connection_types::AnyMessage& rhs);
    static bool decode(const Node& node, csapex::connection_types::AnyMessage& rhs);
};

}  // namespace YAML

#endif  // ANY_MESSAGE_H
