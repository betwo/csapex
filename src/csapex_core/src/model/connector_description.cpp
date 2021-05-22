/// HEADER
#include <csapex/model/connector_description.h>

/// PROJECT
#include <csapex/msg/any_message.h>
#include <csapex/serialization/io/std_io.h>
#include <csapex/serialization/io/csapex_io.h>
#include <csapex/utility/uuid_provider.h>

using namespace csapex;

ConnectorDescription::ConnectorDescription() : connector_type(ConnectorType::NONE), optional(false), is_parameter(false), is_variadic(false), valid(false)
{
}

ConnectorDescription::ConnectorDescription(const AUUID& owner, ConnectorType connector_type, const TokenTypeConstPtr& token_type, const std::string& label)
  : owner(owner), connector_type(connector_type), label(label), optional(false), is_parameter(false), token_type(token_type), id(UUID::NONE), valid(true)
{
}

ConnectorDescription::ConnectorDescription(const AUUID& owner, ConnectorType connector_type, const std::string& label)
  : owner(owner)
  , connector_type(connector_type)
  , label(label)
  , optional(false)
  , is_parameter(false)
  , is_variadic(false)
  , token_type(connection_types::makeTokenType<connection_types::AnyMessage>())
  , id(UUID::NONE)
  , valid(true)
{
}

ConnectorDescription::ConnectorDescription(const AUUID& owner, const UUID& uuid, ConnectorType connector_type, const TokenTypeConstPtr& token_type, const std::string& label)
  : owner(owner), connector_type(connector_type), label(label), optional(false), is_parameter(false), is_variadic(false), token_type(token_type), id(uuid), valid(true)
{
}

ConnectorDescription& ConnectorDescription::setOptional(bool optional)
{
    this->optional = optional;
    return *this;
}
ConnectorDescription& ConnectorDescription::setParameter(bool parameter)
{
    is_parameter = parameter;
    return *this;
}
ConnectorDescription& ConnectorDescription::setVariadic(bool variadic)
{
    is_variadic = variadic;
    return *this;
}

bool ConnectorDescription::isOutput() const
{
    return connector_type == ConnectorType::OUTPUT || connector_type == ConnectorType::EVENT;
}

void ConnectorDescription::serialize(SerializationBuffer& data, SemanticVersion& version) const
{
    data << owner;
    data << connector_type;
    data << label;
    data << optional;
    data << is_parameter;
    data << is_variadic;

    data << token_type;

    data << id;

    data << targets;
    data << valid;
}
void ConnectorDescription::deserialize(const SerializationBuffer& data, const SemanticVersion& version)
{
    data >> owner;
    data >> connector_type;
    data >> label;
    data >> optional;
    data >> is_parameter;
    data >> is_variadic;

    data >> token_type;

    data >> id;

    data >> targets;
    data >> valid;
}

void ConnectorDescription::Target::serialize(SerializationBuffer& data, SemanticVersion& version) const
{
    data << auuid;
    data << active;
}

void ConnectorDescription::Target::deserialize(const SerializationBuffer& data, const SemanticVersion& version)
{
    data >> auuid;
    data >> active;
}

AUUID ConnectorDescription::getAUUID() const
{
    return AUUID(UUIDProvider::makeDerivedUUID_forced(owner, id.id()));
}
