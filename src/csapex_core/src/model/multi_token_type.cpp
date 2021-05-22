/// HEADER
#include <csapex/model/multi_token_type.h>

/// COMPONENT
#include <csapex/msg/message.h>
#include <csapex/msg/token_traits.h>
#include <csapex/utility/assert.h>
#include <csapex/serialization/io/std_io.h>
#include <csapex/model/token_type.h>

/// SYSTEM
#include <iostream>

using namespace csapex;

MultiTokenType::MultiTokenType(const std::string& type_name) : TokenType(type_name, type_name)
{
}

MultiTokenType::MultiTokenType(const std::string& type_name, const std::string& descriptive_name) : TokenType(type_name, descriptive_name)
{
}

bool MultiTokenType::canConnectTo(const TokenType* other_side) const
{
    // return other_side->acceptsConnectionFrom(this);
    return connector_fn_(*this, *other_side);
}

bool MultiTokenType::acceptsConnectionFrom(const TokenType* other_side) const
{
    // return type_name_ == other_side->typeName();
    return acceptor_fn_(*this, *other_side);
}

bool MultiTokenType::isContainer() const
{
    return false;
}

TokenType::Ptr MultiTokenType::nestedType() const
{
    throw std::logic_error("cannot get nested type for non-container messages");
}

void MultiTokenType::serialize(SerializationBuffer& data, SemanticVersion& version) const
{
    data << type_name_;
    data << descriptive_name_;
}
void MultiTokenType::deserialize(const SerializationBuffer& data, const SemanticVersion& version)
{
    data >> type_name_;
    data >> descriptive_name_;
}