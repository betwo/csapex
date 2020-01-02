#ifndef NOTE_H
#define NOTE_H

/// PROJECT
#include <csapex/io/remote_io_fwd.h>
#include <csapex/serialization/streamable.h>
#include <csapex/utility/uuid.h>

/// SYSTEM
#include <string>

namespace csapex
{
namespace io
{
class Note : public Streamable
{
public:
    Note(AUUID uuid);

    static const uint8_t PACKET_TYPE_ID = 7;

    uint8_t getPacketType() const override;
    virtual std::string getType() const = 0;

    AUUID getAUUID() const;

    void serialize(SerializationBuffer& data, SemanticVersion& version) const override;
    void deserialize(const SerializationBuffer& data, const SemanticVersion& version) override;

protected:
    AUUID uuid_;
};

}  // namespace io
}  // namespace csapex

#endif  // NOTE_H
