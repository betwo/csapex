/// HEADER
#include <csapex/io/protcol/graph_facade_notes.h>

/// PROJECT
#include <csapex/serialization/io/std_io.h>
#include <csapex/serialization/note_serializer.h>

CSAPEX_REGISTER_NOTE_SERIALIZER(GraphFacadeNote)

using namespace csapex;

GraphFacadeNote::GraphFacadeNote()
{
}

GraphFacadeNote::GraphFacadeNote(GraphFacadeNoteType request_type, const AUUID& uuid) : NoteImplementation(uuid), note_type_(request_type)
{
}

GraphFacadeNote::GraphFacadeNote(GraphFacadeNoteType request_type, const AUUID& uuid, const std::vector<std::any>& payload) : NoteImplementation(uuid), note_type_(request_type), payload_(payload)
{
}

void GraphFacadeNote::serialize(SerializationBuffer& data, SemanticVersion& version) const
{
    Note::serialize(data, version);

    data << note_type_;
    data << payload_;
}

void GraphFacadeNote::deserialize(const SerializationBuffer& data, const SemanticVersion& version)
{
    Note::deserialize(data, version);

    data >> note_type_;
    data >> payload_;
}
