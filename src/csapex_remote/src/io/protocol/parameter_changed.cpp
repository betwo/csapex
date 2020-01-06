/// HEADER
#include <csapex/io/protcol/parameter_changed.h>

/// PROJECT
#include <csapex/serialization/broadcast_message_serializer.h>
#include <csapex/utility/uuid_provider.h>
#include <csapex/serialization/io/std_io.h>
#include <csapex/serialization/io/csapex_io.h>

/// SYSTEM
#include <iostream>

CSAPEX_REGISTER_BROADCAST_SERIALIZER(ParameterChanged)

using namespace csapex;

ParameterChanged::ParameterChanged(const UUID& id, const std::any& value) : uuid(id), value(value)
{
}

ParameterChanged::ParameterChanged()
{
}

void ParameterChanged::serialize(SerializationBuffer& data, SemanticVersion& version) const
{
    data << uuid;
    data << value;
}

void ParameterChanged::deserialize(const SerializationBuffer& data, const SemanticVersion& version)
{
    data >> uuid;
    data >> value;
}

AUUID ParameterChanged::getUUID() const
{
    return uuid;
}

std::any ParameterChanged::getValue() const
{
    return value;
}
