#ifndef PROFILER_REQUESTS_H
#define PROFILER_REQUESTS_H

/// PROJECT
#include <csapex/io/request_impl.hpp>
#include <csapex/io/response_impl.hpp>
#include <csapex/param/parameter.h>
#include <csapex/serialization/serialization_fwd.h>

namespace csapex
{
class ProfilerRequests
{
public:
    enum class ProfilerRequestType
    {
        SetEnabled
    };

    class ProfilerRequest : public RequestImplementation<ProfilerRequest>
    {
    public:
        ProfilerRequest(uint8_t request_id);
        ProfilerRequest(ProfilerRequestType request_type, const AUUID& uuid);

        template <typename... Args>
        ProfilerRequest(ProfilerRequestType request_type, const AUUID& uuid, Args&&... args) : ProfilerRequest(request_type, uuid)
        {
            arguments_ = { args... };
        }

        void serialize(SerializationBuffer& data, SemanticVersion& version) const override;
        void deserialize(const SerializationBuffer& data, const SemanticVersion& version) override;

        ResponsePtr execute(const SessionPtr& session, CsApexCore& core) const override;

        std::string getType() const override
        {
            return "ProfilerRequests";
        }

        template <typename R>
        R getArgument(const std::size_t i) const
        {
            return std::any_cast<R>(arguments_.at(i));
        }

    private:
        ProfilerRequestType request_type_;
        AUUID uuid_;
        std::vector<std::any> arguments_;
    };

    class ProfilerResponse : public ResponseImplementation<ProfilerResponse>
    {
    public:
        ProfilerResponse(uint8_t request_id);
        ProfilerResponse(ProfilerRequestType request_type, const AUUID& uuid, uint8_t request_id);
        ProfilerResponse(ProfilerRequestType request_type, const AUUID& uuid, std::any result, uint8_t request_id);

        void serialize(SerializationBuffer& data, SemanticVersion& version) const override;
        void deserialize(const SerializationBuffer& data, const SemanticVersion& version) override;

        std::string getType() const override
        {
            return "ProfilerRequests";
        }

        template <typename R>
        R getResult() const
        {
            return std::any_cast<R>(result_);
        }

    private:
        ProfilerRequestType request_type_;
        AUUID uuid_;

        std::any result_;
    };

public:
    using RequestT = ProfilerRequest;
    using ResponseT = ProfilerResponse;
};

}  // namespace csapex
#endif  // PROFILER_REQUESTS_H
