#ifndef MSG_IO_H
#define MSG_IO_H

/// PROJECT
#include <csapex/model/model_fwd.h>
#include <csapex/msg/msg_fwd.h>
#include <csapex/msg/token_traits.h>
#include <csapex/utility/uuid.h>
#include <csapex/msg/message_allocator.h>

namespace boost
{
template <typename T>
class shared_ptr;
}

namespace shared_ptr_tools
{
template <typename T>
std::shared_ptr<T> to_std_shared(const boost::shared_ptr<T>& p);
}

namespace csapex
{
namespace msg
{
/// COMMON
CSAPEX_CORE_EXPORT bool hasMessage(Input* input);
CSAPEX_CORE_EXPORT bool hasMessage(Output* output);

CSAPEX_CORE_EXPORT bool isConnected(Input* input);
CSAPEX_CORE_EXPORT bool isConnected(Output* output);

CSAPEX_CORE_EXPORT bool isEnabled(Input* input);
CSAPEX_CORE_EXPORT bool isEnabled(Output* output);

CSAPEX_CORE_EXPORT void enable(Input* input);
CSAPEX_CORE_EXPORT void disable(Input* input);
CSAPEX_CORE_EXPORT void enable(Output* output);
CSAPEX_CORE_EXPORT void disable(Output* output);

CSAPEX_CORE_EXPORT UUID getUUID(Input* input);
CSAPEX_CORE_EXPORT UUID getUUID(Output* input);

CSAPEX_CORE_EXPORT void setLabel(Input* input, const std::string& label);
CSAPEX_CORE_EXPORT void setLabel(Output* input, const std::string& label);

CSAPEX_CORE_EXPORT void throwError(const TokenDataConstPtr& msg, const std::type_info& type);

CSAPEX_CORE_EXPORT void trigger(Event* event);
CSAPEX_CORE_EXPORT void trigger(Event* event, const TokenPtr& token);
CSAPEX_CORE_EXPORT void trigger(Event* event, const TokenDataConstPtr& token_data);

template <typename T>
void trigger(Event* input, const T value, typename std::enable_if<connection_types::should_use_value_message<T>::value>::type* = 0)
{
    auto message = std::make_shared<connection_types::GenericValueMessage<T>>(value);
    trigger(input, message);
}
template <typename T>
void trigger(Event* input, const T value, typename std::enable_if<connection_types::should_use_pointer_message<T>::value>::type* = 0)
{
    auto message = std::make_shared<connection_types::GenericPointerMessage<T>>(value);
    trigger(input, message);
}

/// CASTING

template <typename R, typename S>
struct DefaultMessageCaster
{
    static std::shared_ptr<R const> constcast(const std::shared_ptr<S const>& msg)
    {
        return std::dynamic_pointer_cast<R const>(msg);
    }
    static std::shared_ptr<R> cast(const std::shared_ptr<S>& msg)
    {
        return std::dynamic_pointer_cast<R>(msg);
    }
};

template <typename R, typename S, typename Enable = void>
struct MessageCaster
{
    static std::shared_ptr<R const> constcast(const std::shared_ptr<S const>& msg)
    {
        return DefaultMessageCaster<R, S>::constcast(msg);
    }
    static std::shared_ptr<R> cast(const std::shared_ptr<S>& msg)
    {
        return DefaultMessageCaster<R, S>::cast(msg);
    }
};

template <typename R, typename S>
std::shared_ptr<R const> message_cast(const std::shared_ptr<S const>& msg)
{
    return MessageCaster<typename std::remove_const<R>::type, typename std::remove_const<S>::type>::constcast(msg);
}
template <typename R, typename S>
std::shared_ptr<R> message_cast(const std::shared_ptr<S>& msg)
{
    return MessageCaster<typename std::remove_const<R>::type, typename std::remove_const<S>::type>::cast(msg);
}

/// TOKEN CREATION
template <typename T>
TokenPtr createToken(T&& v, typename std::enable_if<connection_types::should_use_value_message<T>::value>::type* = 0)
{
    const auto& msg = std::make_shared<connection_types::GenericValueMessage<T>>(v);
    return std::make_shared<Token>(msg);
}

template <typename T>
TokenPtr createToken(T&& v, typename std::enable_if<connection_types::should_use_pointer_message<T>::value>::type* = 0)
{
    const auto& msg = std::make_shared<connection_types::GenericPointerMessage<T>>();
    msg->value = std::make_shared<T>(std::move(v));
    return std::make_shared<Token>(msg);
}

/// INPUT
CSAPEX_CORE_EXPORT TokenDataConstPtr getMessage(Input* input);

template <typename R>
std::shared_ptr<R const> getMessage(Input* input, typename std::enable_if<std::is_base_of<TokenData, R>::value>::type* /*dummy*/ = 0)
{
    const auto& msg = getMessage(input);
    typename std::shared_ptr<R const> result = message_cast<R const>(msg);
    if (!result) {
        throwError(msg, typeid(R));
    }
    return result;
}

template <typename R>
std::shared_ptr<R const> getMessage(Input* input, typename std::enable_if<!std::is_base_of<TokenData, R>::value>::type* /*dummy*/ = 0)
{
    const auto& msg = getMessage(input);
    auto result = message_cast<connection_types::GenericPointerMessage<R> const>(msg);
    if (!result) {
        throwError(msg, typeid(R));
    }
    return result->value;
}

template <typename Container, typename R>
std::shared_ptr<typename Container::template TypeMap<R>::type const> getMessage(Input* input)
{
    const auto& msg = getMessage(input);
    typename std::shared_ptr<Container const> result = message_cast<Container const>(msg);
    if (!result) {
        throwError(msg, typeid(Container));
    }
    return result->template makeShared<R>();
}

template <typename R>
std::shared_ptr<R const> getOptionalMessage(Input* input, typename std::enable_if<std::is_base_of<TokenData, R>::value>::type* /*dummy*/ = 0)
{
    const auto& msg = getMessage(input);
    typename std::shared_ptr<R const> result = message_cast<R const>(msg);
    if (!result) {
        return nullptr;
    }
    return result;
}

template <typename R>
std::shared_ptr<R const> getOptionalMessage(Input* input, typename std::enable_if<!std::is_base_of<TokenData, R>::value>::type* /*dummy*/ = 0)
{
    const auto& msg = getMessage(input);
    auto result = message_cast<connection_types::GenericPointerMessage<R> const>(msg);
    if (!result) {
        return nullptr;
    }
    return result->value;
}

template <typename Container, typename R>
std::shared_ptr<typename Container::template TypeMap<R>::type const> getOptionalMessage(Input* input)
{
    const auto& msg = getMessage(input);
    typename std::shared_ptr<Container const> result = message_cast<Container const>(msg);
    if (!result) {
        return nullptr;
    }
    return result->template makeShared<R>();
}

template <typename R>
std::shared_ptr<R> getClonedMessage(Input* input)
{
    const auto& msg = getMessage<R>(input);
    if (msg == nullptr) {
        return nullptr;
    }
    return message_cast<R>(msg->cloneRaw());
}

template <typename R>
R getValue(Input* input)
{
    const auto& msg = getMessage<connection_types::GenericValueMessage<R>>(input);
    if (!msg) {
        throw std::logic_error("cannot convert message to requested value");
    }
    return msg->value;
}

template <typename R>
bool isMessage(Input* input, typename std::enable_if<std::is_base_of<TokenData, R>::value>::type* /*dummy*/ = 0)
{
    const auto& msg = getMessage(input);
    auto test = message_cast<R const>(msg);
    return test != nullptr;
}

template <typename R>
bool isMessage(Input* input, typename std::enable_if<!std::is_base_of<TokenData, R>::value>::type* /*dummy*/ = 0)
{
    const auto& msg = getMessage(input);
    auto test = message_cast<connection_types::GenericPointerMessage<R> const>(msg);
    return test != nullptr;
}

template <typename R>
bool isExactMessage(Input* input)
{
    const auto& msg_ptr = getMessage(input);
    const auto& msg = *msg_ptr;
    return typeid(msg) == typeid(R);
}

template <typename R>
bool isValue(Input* input)
{
    return isMessage<connection_types::GenericValueMessage<R>>(input);
}

template <typename R>
bool isExactValue(Input* input)
{
    return isExactMessage<connection_types::GenericValueMessage<R>>(input);
}

/// OUTPUT
MessageAllocator& getMessageAllocator(Output* output);

template <typename T, typename... Args>
std::shared_ptr<T> allocate(Output* output, Args&&... args)
{
    MessageAllocator& allocator = getMessageAllocator(output);
    return allocator.allocate<T>(std::forward<Args>(args)...);
}

CSAPEX_CORE_EXPORT void publish(Output* output, TokenDataConstPtr message);

template <typename T, typename = typename std::enable_if<connection_types::should_use_pointer_message<T>::value && !connection_types::is_std_vector<T>::value>::type>
void publish(Output* output, typename std::shared_ptr<T> message, std::string frame_id = "/")
{
    typename connection_types::GenericPointerMessage<T>::Ptr msg(new connection_types::GenericPointerMessage<T>(frame_id));
    msg->value = message;
    publish(output, message_cast<TokenData>(msg));
}

template <typename T, typename = typename std::enable_if<connection_types::should_use_pointer_message<T>::value>::type>
void publish(Output* output, typename boost::shared_ptr<T> message, std::string frame_id = "/")
{
    typename connection_types::GenericPointerMessage<T>::Ptr msg(new connection_types::GenericPointerMessage<T>(frame_id));
    msg->value = shared_ptr_tools::to_std_shared(message);
    publish(output, message_cast<TokenData>(msg));
}

template <typename T, typename = typename std::enable_if<connection_types::should_use_value_message<T>::value>::type>
void publish(Output* output, T message, std::string frame_id = "/")
{
    typename connection_types::GenericValueMessage<T>::Ptr msg(new connection_types::GenericValueMessage<T>(message, frame_id));
    publish(output, message_cast<TokenData>(msg));
}

template <class Container, typename T>
void publish(Output* output, const typename Container::template TypeMap<T>::Ptr& message)
{
    typename std::shared_ptr<Container> msg(Container::template make<T>());
    msg->template set<T>(message);
    publish(output, msg);
}

template <typename T>
void publish(Output* output, const std::shared_ptr<std::vector<T>>& message)
{
    publish<connection_types::GenericVectorMessage, T>(output, message);
}
}  // namespace msg
}  // namespace csapex

#endif  // MSG_IO_H
