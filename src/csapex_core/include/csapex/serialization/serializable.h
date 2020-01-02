#ifndef SERIALIZABLE_H
#define SERIALIZABLE_H

/// PROJECT
#include <csapex/serialization/serialization_fwd.h>
#include <csapex/model/clonable.h>
#include <csapex/utility/semantic_version.h>

/// SYSTEM
#include <inttypes.h>
#include <vector>

namespace csapex
{
class Serializable : public Clonable
{
public:
    ~Serializable() override;

    virtual SemanticVersion getVersion() const;

    virtual void serialize(SerializationBuffer& data, SemanticVersion& version) const = 0;
    virtual void deserialize(const SerializationBuffer& data, const SemanticVersion& version) = 0;

    void serializeVersioned(SerializationBuffer& data) const;
    void deserializeVersioned(const SerializationBuffer& data);
};

}  // namespace csapex

#endif
