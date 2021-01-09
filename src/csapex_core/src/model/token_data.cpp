/// HEADER
#include <csapex/model/token.h>

/// COMPONENT
#include <csapex/msg/message.h>
#include <csapex/msg/token_traits.h>
#include <csapex/utility/assert.h>
#include <csapex/serialization/io/std_io.h>
#include <csapex/model/token_type.h>

/// SYSTEM
#include <iostream>

using namespace csapex;

TokenData::TokenData()
{
}

TokenData::TokenData(const std::string& type_name) : TokenType(type_name)
{
    setDescriptiveName(type_name);
}

TokenData::TokenData(const std::string& type_name, const std::string& descriptive_name) : TokenType(type_name, descriptive_name)
{
}

TokenData::~TokenData()
{
}

std::shared_ptr<TokenType> TokenData::toType() const
{
    return cloneAs<TokenType>();
}

bool TokenData::isValid() const
{
    return true;
}

TokenData::ConstPtr TokenData::nestedValue(std::size_t index) const
{
    throw std::logic_error("cannot get nested value for non-container messages");
}
std::size_t TokenData::nestedValueCount() const
{
    throw std::logic_error("cannot get nested count for non-container messages");
}
void TokenData::addNestedValue(const ConstPtr& msg)
{
    throw std::logic_error("cannot add nested value to non-container messages");
}

void TokenData::writeNative(const std::string& /*file*/, const std::string& /*base*/, const std::string& /*suffix*/) const
{
    std::cerr << "error: writeRaw not implemented for message type " << descriptiveName() << std::endl;
}

uint8_t TokenData::getPacketType() const
{
    return PACKET_TYPE_ID;
}
