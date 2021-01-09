/// HEADER
#include <csapex/model/token_type.h>

/// COMPONENT
#include <csapex/msg/message.h>
#include <csapex/msg/token_traits.h>
#include <csapex/utility/assert.h>
#include <csapex/serialization/io/std_io.h>
#include <csapex/model/token_type.h>

/// SYSTEM
#include <iostream>

using namespace csapex;

TokenType::TokenType(const std::string& type_name) : type_name_(type_name)
{
    setDescriptiveName(type_name);
}

TokenType::TokenType(const std::string& type_name, const std::string& descriptive_name) : type_name_(type_name), descriptive_name_(descriptive_name)
{
}

void TokenType::setDescriptiveName(const std::string& name)
{
    descriptive_name_ = name;
}

bool TokenType::canConnectTo(const TokenType* other_side) const
{
    return other_side->acceptsConnectionFrom(this);
}

bool TokenType::acceptsConnectionFrom(const TokenType* other_side) const
{
    return type_name_ == other_side->typeName();
}

std::string TokenType::descriptiveName() const
{
    return descriptive_name_;
}

std::string TokenType::typeName() const
{
    return type_name_;
}

bool TokenType::isContainer() const
{
    return false;
}

TokenType::Ptr TokenType::nestedType() const
{
    throw std::logic_error("cannot get nested type for non-container messages");
}

void TokenType::serialize(SerializationBuffer& data, SemanticVersion& version) const
{
    data << type_name_;
    data << descriptive_name_;
}
void TokenType::deserialize(const SerializationBuffer& data, const SemanticVersion& version)
{
    data >> type_name_;
    data >> descriptive_name_;
}