#ifndef CORE_REQUESTS_H
#define CORE_REQUESTS_H

/// PROJECT
#include <csapex/io/request_impl.hpp>
#include <csapex/io/response_impl.hpp>
#include <csapex/param/parameter.h>
#include <csapex/serialization/serialization_fwd.h>

namespace csapex
{
class CoreRequests
{
public:
    enum class CoreRequestType
    {
        SettingsSavePersistent,
        SettingsLoadPersistent,
        CoreSave,
        CoreLoad,
        CoreSerialize,
        CoreStep,
        CoreShutdown,
        CoreResetActivity,
        CoreClearBlock,
        CoreReset,
        CoreSetPause,
        CoreSetSteppingMode,

        CoreSendNotification,

        CoreGetPause,
        CoreGetSteppingMode
    };

    class CoreRequest : public RequestImplementation<CoreRequest>
    {
    public:
        CoreRequest(uint8_t request_id);
        CoreRequest(CoreRequestType request_type);

        template <typename... Args>
        CoreRequest(CoreRequestType request_type, Args&&... args) : CoreRequest(request_type)
        {
            parameters_ = { args... };
        }

        void serialize(SerializationBuffer& data, SemanticVersion& version) const override;
        void deserialize(const SerializationBuffer& data, const SemanticVersion& version) override;

        ResponsePtr execute(const SessionPtr& session, CsApexCore& core) const override;

        std::string getType() const override
        {
            return "CoreRequests";
        }

    private:
        CoreRequestType request_type_;

        std::vector<std::any> parameters_;
    };

    class CoreResponse : public ResponseImplementation<CoreResponse>
    {
    public:
        CoreResponse(uint8_t request_id);
        CoreResponse(CoreRequestType request_type, uint8_t request_id);
        CoreResponse(CoreRequestType request_type, std::any result, uint8_t request_id);

        void serialize(SerializationBuffer& data, SemanticVersion& version) const override;
        void deserialize(const SerializationBuffer& data, const SemanticVersion& version) override;

        template <typename R>
        R getResult() const
        {
            return std::any_cast<R>(result_);
        }

        std::string getType() const override
        {
            return "CoreRequests";
        }

    private:
        CoreRequestType request_type_;

        std::any result_;
    };

public:
    using RequestT = CoreRequest;
    using ResponseT = CoreResponse;
};

}  // namespace csapex

#endif  // CORE_REQUESTS_H
