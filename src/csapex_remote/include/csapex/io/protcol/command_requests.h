#ifndef COMMAND_REQUESTS_H
#define COMMAND_REQUESTS_H

/// PROJECT
#include <csapex/io/request_impl.hpp>
#include <csapex/io/response_impl.hpp>
#include <csapex/param/parameter.h>
#include <csapex/serialization/serialization_fwd.h>

namespace csapex
{
class CommandRequests
{
public:
    enum class CommandRequestType
    {
        Execute,
        ExecuteLater,

        IsDirty,
        CanUndo,
        CanRedo,

        Undo,
        Redo
    };

    class CommandRequest : public RequestImplementation<CommandRequest>
    {
    public:
        CommandRequest(uint8_t request_id);
        CommandRequest(CommandRequestType request_type);

        CommandRequest(CommandRequestType request_type, const CommandPtr& param) : CommandRequest(request_type)
        {
            command_ = param;
        }

        void serialize(SerializationBuffer& data, SemanticVersion& version) const override;
        void deserialize(const SerializationBuffer& data, const SemanticVersion& version) override;

        ResponsePtr execute(const SessionPtr& session, CsApexCore& core) const override;

        std::string getType() const override
        {
            return "CommandRequests";
        }

    private:
        CommandRequestType request_type_;

        CommandPtr command_;
    };

    class CommandResponse : public ResponseImplementation<CommandResponse>
    {
    public:
        CommandResponse(uint8_t request_id);
        CommandResponse(CommandRequestType request_type, uint8_t request_id);
        CommandResponse(CommandRequestType request_type, bool result, uint8_t request_id);

        void serialize(SerializationBuffer& data, SemanticVersion& version) const override;
        void deserialize(const SerializationBuffer& data, const SemanticVersion& version) override;

        template <typename R>
        R getResult() const
        {
            return result_;
        }

        std::string getType() const override
        {
            return "CommandRequests";
        }

    private:
        CommandRequestType request_type_;

        bool result_;
    };

public:
    using RequestT = CommandRequest;
    using ResponseT = CommandResponse;
};

}  // namespace csapex

#endif  // COMMAND_REQUESTS_H
