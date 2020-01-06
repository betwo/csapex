#ifndef GRAPH_FACADE_NOTES_H
#define GRAPH_FACADE_NOTES_H

/// PROJECT
#include <csapex/io/note_impl.hpp>
#include <csapex/serialization/serialization_fwd.h>
#include <csapex/utility/uuid.h>

/// SYSTEM
#include <any>

namespace csapex
{
enum class GraphFacadeNoteType
{
    ChildAdded,
    ChildRemoved,

    ChildNodeFacadeAdded,
    ChildNodeFacadeRemoved,

    ForwardingConnectorAdded,
    ForwardingConnectorRemoved,

    PauseChanged,
    Notification,

/**
 * begin: connect signals
 **/
#define HANDLE_ACCESSOR(_enum, type, function)
#define HANDLE_STATIC_ACCESSOR(_enum, type, function)
#define HANDLE_DYNAMIC_ACCESSOR(_enum, signal, type, function) function##Changed,
#define HANDLE_SIGNAL(_enum, signal) _enum##Triggered,
#include <csapex/model/graph_facade_proxy_accessors.hpp>
    /**
     * end: connect signals
     **/
};

class GraphFacadeNote : public NoteImplementation<GraphFacadeNote>
{
public:
    GraphFacadeNote();
    GraphFacadeNote(GraphFacadeNoteType request_type, const AUUID& uuid);
    GraphFacadeNote(GraphFacadeNoteType request_type, const AUUID& uuid, const std::vector<std::any>& payload);

    template <typename... Args>
    GraphFacadeNote(GraphFacadeNoteType request_type, const AUUID& uuid, Args... args) : GraphFacadeNote(request_type, uuid, { std::forward<Args>(args)... })
    {
    }

    void serialize(SerializationBuffer& data, SemanticVersion& version) const override;
    void deserialize(const SerializationBuffer& data, const SemanticVersion& version) override;

    GraphFacadeNoteType getNoteType() const
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
        return std::any_cast<T>(payload_.at(pos));
    }

private:
    GraphFacadeNoteType note_type_;
    std::vector<std::any> payload_;
};

}  // namespace csapex

#endif  // GRAPH_FACADE_NOTES_H
