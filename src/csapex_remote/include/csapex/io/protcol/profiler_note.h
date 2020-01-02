#ifndef PROFILER_NOTE_H
#define PROFILER_NOTE_H

/// PROJECT
#include <csapex/io/note_impl.hpp>
#include <csapex/serialization/serialization_fwd.h>
#include <csapex/utility/uuid.h>

/// SYSTEM
#include <boost/any.hpp>

namespace csapex
{
enum class ProfilerNoteType
{
    EnabledChanged
};

class ProfilerNote : public NoteImplementation<ProfilerNote>
{
public:
    ProfilerNote();
    ProfilerNote(ProfilerNoteType request_type, const AUUID& uuid);
    ProfilerNote(ProfilerNoteType request_type, const AUUID& uuid, const std::vector<boost::any>& payload);

    template <typename... Args>
    ProfilerNote(ProfilerNoteType request_type, const AUUID& uuid, Args... args) : ProfilerNote(request_type, uuid, { std::forward<Args>(args)... })
    {
    }

    void serialize(SerializationBuffer& data, SemanticVersion& version) const override;
    void deserialize(const SerializationBuffer& data, const SemanticVersion& version) override;

    ProfilerNoteType getNoteType() const
    {
        return note_type_;
    }

    std::size_t countPayload() const
    {
        return payload_.size();
    }

    template <typename T>
    T getPayload(const std::size_t pos) const
    {
        return boost::any_cast<T>(payload_.at(pos));
    }

private:
    ProfilerNoteType note_type_;
    std::vector<boost::any> payload_;
};

}  // namespace csapex

#endif  // PROFILER_NOTE_H
