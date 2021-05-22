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

TokenType::TokenType(const std::string& type_name) : TokenType(type_name, type_name)
{
}

TokenType::TokenType(const std::string& type_name, const std::string& descriptive_name) : type_name_(type_name), descriptive_name_(descriptive_name)
{
    connector_fn_ = [](const TokenType& left, const TokenType& right) { return right.canConnectTo(&left); };
    acceptor_fn_ = [](const TokenType& left, const TokenType& right) { return left.typeName() == right.typeName(); };
}

TokenType::TokenType(std::function<bool(const TokenType&, const TokenType&)> acceptor_fn, const std::string& type_name, const std::string& descriptive_name)
  : type_name_(type_name), descriptive_name_(descriptive_name)
{
    connector_fn_ = [](const TokenType& left, const TokenType& right) { return right.canConnectTo(&left); };
    acceptor_fn_ = acceptor_fn;
}

TokenType::TokenType(std::function<bool(const TokenType&, const TokenType&)> acceptor_fn, std::function<bool(const TokenType&, const TokenType&)> connector_fn, const std::string& type_name,
                     const std::string& descriptive_name)
  : type_name_(type_name), descriptive_name_(descriptive_name)
{
    connector_fn_ = connector_fn;
    acceptor_fn_ = acceptor_fn;
}

void TokenType::setDescriptiveName(const std::string& name)
{
    descriptive_name_ = name;
}

bool TokenType::canConnectTo(const TokenType* other_side) const
{
    // return other_side->acceptsConnectionFrom(this);
    return connector_fn_(*this, *other_side);
}

bool TokenType::acceptsConnectionFrom(const TokenType* other_side) const
{
    // return type_name_ == other_side->typeName();
    return acceptor_fn_(*this, *other_side);
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

uint8_t TokenType::getPacketType() const
{
    return PACKET_TYPE_ID;
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