#ifndef MESSAGE_RENDERER_H
#define MESSAGE_RENDERER_H

/// COMPONENT
#include <csapex_core/csapex_core_export.h>

/// PROJECT
#include <csapex/param/param_fwd.h>
#include <csapex/model/model_fwd.h>

/// SYSTEM
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <vector>

/// FORWARD DECLARATION
class QImage;

namespace csapex
{
class CSAPEX_CORE_EXPORT MessageRenderer
{
public:
    typedef std::shared_ptr<MessageRenderer> Ptr;

public:
    virtual ~MessageRenderer();

    virtual std::unique_ptr<QImage> render(const TokenDataConstPtr& msg) = 0;
    virtual std::type_index messageType() const = 0;

    virtual std::vector<csapex::param::ParameterPtr> getParameters() const
    {
        return std::vector<csapex::param::ParameterPtr>();
    }
};

template <class Message>
class MessageRendererImplementation : public MessageRenderer
{
public:
    virtual std::unique_ptr<QImage> render(const TokenDataConstPtr& msg) final override
    {
        const auto& real_msg = std::dynamic_pointer_cast<Message const>(msg);
        if (real_msg) {
            return doRender(*real_msg);
        } else {
            throw std::runtime_error(std::string("cannot render message of type ") + typeid(Message).name());
        }
    }

    std::type_index messageType() const override
    {
        return std::type_index(typeid(Message));
    }

    virtual std::unique_ptr<QImage> doRender(const Message& msg) = 0;
};

}  // namespace csapex

#endif  // MESSAGE_RENDERER_H
