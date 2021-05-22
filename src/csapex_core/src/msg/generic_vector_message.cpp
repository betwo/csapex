/// HEADER
#include <csapex/msg/generic_vector_message.hpp>

/// COMPONENT
#include <csapex/msg/token_traits.h>
#include <csapex/utility/register_msg.h>
#include <csapex/serialization/yaml.h>
#include <csapex/msg/no_message.h>

CSAPEX_REGISTER_MESSAGE_WITH_NAME(csapex::connection_types::GenericVectorMessage, g_instance_generic_vector_)

using namespace csapex;
using namespace connection_types;

GenericVectorMessage::GenericVectorMessage(EntryInterface::Ptr pimpl, const std::string& frame_id, Message::Stamp stamp) : Message(pimpl->getType(), frame_id, stamp), pimpl(pimpl)
{
}

GenericVectorMessage::GenericVectorMessage() : Message(type<GenericVectorMessage>::makeTokenType(), "/", 0), pimpl(std::make_shared<InstancedImplementation>(type<AnyMessage>::makeTokenType()))
{
}

// bool GenericVectorMessage::canConnectTo(const TokenType* other_side) const
// {
//     return pimpl->canConnectTo(other_side);
// }

// bool GenericVectorMessage::acceptsConnectionFrom(const TokenType* other_side) const
// {
//     return pimpl->acceptsConnectionFrom(other_side);
// }

// std::string GenericVectorMessage::descriptiveName() const
// {
//     return pimpl->descriptiveName();
// }

/// ANYTHING

GenericVectorMessage::AnythingImplementation::AnythingImplementation() : EntryInterface("Anything")
{
}

void GenericVectorMessage::AnythingImplementation::encode(YAML::Node& node) const
{
}
void GenericVectorMessage::AnythingImplementation::decode(const YAML::Node& node)
{
}

// bool GenericVectorMessage::AnythingImplementation::canConnectTo(const TokenType* other_side) const
// {
//     if (dynamic_cast<const EntryInterface*>(other_side)) {
//         return true;
//     } else if (dynamic_cast<const GenericVectorMessage*>(other_side)) {
//         return true;
//     } else {
//         auto type = getType();
//         return other_side->canConnectTo(type.get());
//     }
// }

// bool GenericVectorMessage::AnythingImplementation::acceptsConnectionFrom(const TokenType* other_side) const
// {
//     if (dynamic_cast<const EntryInterface*>(other_side)) {
//         return true;
//     } else if (dynamic_cast<const GenericVectorMessage*>(other_side)) {
//         return true;
//     } else {
//         return dynamic_cast<const AnyMessage*>(other_side) != nullptr;
//     }
// }

void GenericVectorMessage::AnythingImplementation::serialize(SerializationBuffer& data, SemanticVersion& version) const
{
    EntryInterface::serialize(data, version);
}
void GenericVectorMessage::AnythingImplementation::deserialize(const SerializationBuffer& data, const SemanticVersion& version)
{
    EntryInterface::deserialize(data, version);
}

// INSTANCED

GenericVectorMessage::InstancedImplementation::InstancedImplementation(TokenType::ConstPtr type) : EntryInterface("Anything"), type_(type)
{
    apex_assert_hard(type_);
}

GenericVectorMessage::InstancedImplementation::InstancedImplementation() : EntryInterface("Anything"), type_(type<AnyMessage>::makeTokenType())
{
    apex_assert_hard(type_);
}

bool GenericVectorMessage::InstancedImplementation::cloneData(const GenericVectorMessage::InstancedImplementation& other)
{
    value.clear();
    value.reserve(other.value.size());
    for (const auto& v : other.value) {
        value.push_back(v->cloneAs<TokenData>());
    }
    return true;
}

// bool GenericVectorMessage::InstancedImplementation::canConnectTo(const TokenType* other_side) const
// {
//     if (const EntryInterface* ei = dynamic_cast<const EntryInterface*>(other_side)) {
//         return nestedType()->canConnectTo(ei->nestedType().get());
//     } else {
//         const GenericVectorMessage* vec = dynamic_cast<const GenericVectorMessage*>(other_side);
//         if (vec != 0) {
//             return vec->canConnectTo(this);
//         } else {
//             auto type = nestedType();
//             return other_side->canConnectTo(type.get());
//             // return dynamic_cast<const AnyMessage*> (other_side) != nullptr;
//         }
//     }
// }

// bool GenericVectorMessage::InstancedImplementation::acceptsConnectionFrom(const TokenType* other_side) const
// {
//     if (const EntryInterface* ei = dynamic_cast<const EntryInterface*>(other_side)) {
//         return nestedType()->canConnectTo(ei->nestedType().get());
//     } else {
//         return false;
//     }
// }

void GenericVectorMessage::InstancedImplementation::encode(YAML::Node& node) const
{
    apex_assert_hard(type_);
    node["value_type"] = type_->typeName();
    node["values"] = value;
}

void GenericVectorMessage::InstancedImplementation::decode(const YAML::Node& node)
{
    YAML::Emitter emitter;
    emitter << node;
    value = node["values"].as<std::vector<TokenData::Ptr> >();
}

// TokenType::Ptr GenericVectorMessage::InstancedImplementation::nestedType() const
// {
//     apex_assert_hard(type_);
//     return type_->cloneAs<TokenData>();
// }

void GenericVectorMessage::InstancedImplementation::addNestedValue(const TokenData::ConstPtr& msg)
{
    value.push_back(msg->cloneAs<TokenData>());
}
TokenData::ConstPtr GenericVectorMessage::InstancedImplementation::nestedValue(std::size_t i) const
{
    return value.at(i);
}
std::size_t GenericVectorMessage::InstancedImplementation::nestedValueCount() const
{
    return value.size();
}

void GenericVectorMessage::InstancedImplementation::serialize(SerializationBuffer& data, SemanticVersion& version) const
{
    data << value;
}
void GenericVectorMessage::InstancedImplementation::deserialize(const SerializationBuffer& data, const SemanticVersion& version)
{
    data >> value;
}

/// YAML
namespace YAML
{
Node convert<csapex::connection_types::GenericVectorMessage>::encode(const csapex::connection_types::GenericVectorMessage& rhs)
{
    Node node = convert<csapex::connection_types::Message>::encode(rhs);
    rhs.encode(node);
    return node;
}

bool convert<csapex::connection_types::GenericVectorMessage>::decode(const Node& node, csapex::connection_types::GenericVectorMessage& rhs)
{
    if (!node.IsMap()) {
        return false;
    }
    convert<csapex::connection_types::Message>::decode(node, rhs);
    rhs.decode(node);
    return true;
}
}  // namespace YAML
