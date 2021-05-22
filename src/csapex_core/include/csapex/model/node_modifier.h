#ifndef NODE_MODIFIER_H
#define NODE_MODIFIER_H

/// COMPONENT
#include <csapex/model/model_fwd.h>
#include <csapex/msg/msg_fwd.h>
#include <csapex/signal/signal_fwd.h>
#include <csapex/model/multi_token_data.h>
#include <csapex/msg/message.h>
#include <csapex/msg/token_traits.h>
#include <csapex/model/token.h>
#include <csapex/utility/uuid.h>

/// SYSTEM
#include <functional>

namespace csapex
{
class CSAPEX_CORE_EXPORT NodeModifier
{
public:
    NodeModifier();
    virtual ~NodeModifier();

    void setNodeWorker(NodeWorker* worker);

    /// "real" messages
    template <typename T>
    Input* addInput(const std::string& label, typename std::enable_if<std::is_base_of<TokenData, T>::value>::type* = 0)
    {
        return addInput(connection_types::makeTokenType<T>(), label, false);
    }
    template <typename T>
    Input* addOptionalInput(const std::string& label, typename std::enable_if<std::is_base_of<TokenData, T>::value>::type* = 0)
    {
        return addInput(connection_types::makeTokenType<T>(), label, true);
    }
    template <typename T>
    Output* addOutput(const std::string& label, typename std::enable_if<std::is_base_of<TokenData, T>::value>::type* = 0)
    {
        return addOutput(connection_types::makeTokenType<T>(), label);
    }

    /// "container" messages
    template <typename Container, typename T>
    Input* addInput(const std::string& label)
    {
        Container::template registerType<T>();
        return addInput(Container::template make<T>(), label, false);
    }
    template <typename Container, typename T>
    Input* addOptionalInput(const std::string& label)
    {
        Container::template registerType<T>();
        return addInput(Container::template make<T>(), label, true);
    }
    template <typename Container, typename T>
    Output* addOutput(const std::string& label)
    {
        Container::template registerType<T>();
        return addOutput(Container::template make<T>(), label);
    }
    template <typename Container, typename T>
    Event* addEvent(const std::string& label)
    {
        Container::template registerType<T>();
        return addEvent(Container::template make<T>(), label);
    }
    template <typename Container, typename T>
    Slot* addSlot(const std::string& label, std::function<void(const TokenPtr&)> callback, bool active = false, bool blocking = true)
    {
        Container::template registerType<T>();
        return addSlot(Container::template make<T>(), label, callback, active, blocking);
    }

    /// "direct" messages
    template <typename T>
    Input* addInput(const std::string& label, typename std::enable_if<connection_types::should_use_pointer_message<T>::value>::type* = 0)
    {
        static_assert(IS_COMPLETE(connection_types::GenericPointerMessage<T>), "connection_types::GenericPointerMessage is not included: "
                                                                               "#include <csapex/msg/generic_pointer_message.hpp>");
        connection_types::MessageConversionHook<connection_types::GenericPointerMessage, T>::registerConversion();
        return addInput(connection_types::makeTokenType<connection_types::GenericPointerMessage<T>>(), label, false);
    }
    template <typename T>
    Input* addOptionalInput(const std::string& label, typename std::enable_if<connection_types::should_use_pointer_message<T>::value>::type* = 0)
    {
        static_assert(IS_COMPLETE(connection_types::GenericPointerMessage<T>), "connection_types::GenericPointerMessage is not included: "
                                                                               "#include <csapex/msg/generic_pointer_message.hpp>");
        connection_types::MessageConversionHook<connection_types::GenericPointerMessage, T>::registerConversion();
        return addInput(connection_types::makeTokenType<connection_types::GenericPointerMessage<T>>(), label, true);
    }
    template <typename T>
    Output* addOutput(const std::string& label, typename std::enable_if<connection_types::should_use_pointer_message<T>::value>::type* = 0)
    {
        static_assert(IS_COMPLETE(connection_types::GenericPointerMessage<T>), "connection_types::GenericPointerMessage is not included: "
                                                                               "#include <csapex/msg/generic_pointer_message.hpp>");
        connection_types::MessageConversionHook<connection_types::GenericPointerMessage, T>::registerConversion();
        return addOutput(connection_types::makeTokenType<connection_types::GenericPointerMessage<T>>(), label);
    }
    template <typename T>
    Slot* addSlot(const std::string& label, std::function<void(const T)> callback, bool active = false, bool blocking = true,
                  typename std::enable_if<connection_types::should_use_pointer_message<T>::value>::type* = 0)
    {
        static_assert(IS_COMPLETE(connection_types::GenericPointerMessage<T>), "connection_types::GenericPointerMessage is not included: "
                                                                               "#include <csapex/msg/generic_pointer_message.hpp>");
        return addSlot(
            connection_types::makeTokenType<connection_types::GenericPointerMessage<T>>(), label,
            [callback](TokenPtr token) {
                auto input_msg = std::dynamic_pointer_cast<connection_types::GenericPointerMessage<T> const>(token->getTokenData());
                callback(input_msg->value);
            },
            active, blocking);
    }
    template <typename T>
    Event* addEvent(const std::string& label, typename std::enable_if<connection_types::should_use_pointer_message<T>::value>::type* = 0)
    {
        static_assert(IS_COMPLETE(connection_types::GenericPointerMessage<T>), "connection_types::GenericPointerMessage is not included: "
                                                                               "#include <csapex/msg/generic_pointer_message.hpp>");
        return addEvent(connection_types::makeTokenType<connection_types::GenericPointerMessage<T>>(), label);
    }

    template <typename T>
    Input* addInput(const std::string& label, typename std::enable_if<connection_types::should_use_value_message<T>::value>::type* = 0)
    {
        static_assert(IS_COMPLETE(connection_types::GenericValueMessage<T>), "connection_types::GenericValueMessage is not included: "
                                                                             "#include <csapex/msg/generic_value_message.hpp>");
        return addInput(connection_types::makeTokenType<connection_types::GenericValueMessage<T>>(), label, false);
    }
    template <typename T>
    Input* addOptionalInput(const std::string& label, typename std::enable_if<connection_types::should_use_value_message<T>::value>::type* = 0)
    {
        static_assert(IS_COMPLETE(connection_types::GenericValueMessage<T>), "connection_types::GenericValueMessage is not included: "
                                                                             "#include <csapex/msg/generic_value_message.hpp>");
        return addInput(connection_types::makeTokenType<connection_types::GenericValueMessage<T>>(), label, true);
    }
    template <typename T>
    Output* addOutput(const std::string& label, typename std::enable_if<connection_types::should_use_value_message<T>::value>::type* = 0)
    {
        static_assert(IS_COMPLETE(connection_types::GenericValueMessage<T>), "connection_types::GenericValueMessage is not included: "
                                                                             "#include <csapex/msg/generic_value_message.hpp>");
        return addOutput(connection_types::makeTokenType<connection_types::GenericValueMessage<T>>(), label);
    }
    template <typename T>
    Slot* addSlot(const std::string& label, std::function<void(const T)> callback, bool active = false, bool blocking = true,
                  typename std::enable_if<connection_types::should_use_value_message<T>::value>::type* = 0)
    {
        static_assert(IS_COMPLETE(connection_types::GenericValueMessage<T>), "connection_types::GenericValueMessage is not included: "
                                                                             "#include <csapex/msg/generic_value_message.hpp>");
        return addSlot(
            connection_types::makeTokenType<connection_types::GenericValueMessage<T>>(), label,
            [callback](TokenPtr token) {
                auto input_msg = std::dynamic_pointer_cast<connection_types::GenericValueMessage<T> const>(token->getTokenData());
                callback(input_msg->value);
            },
            active, blocking);
    }
    template <typename T, typename Instance, typename R = void>
    Slot* addSlot(const std::string& label, Instance* instance, R (Instance::*member_fn)(const T), bool active = false, bool blocking = true,
                  typename std::enable_if<connection_types::should_use_value_message<T>::value>::type* = 0)
    {
        static_assert(IS_COMPLETE(connection_types::GenericValueMessage<T>), "connection_types::GenericValueMessage is not included: "
                                                                             "#include <csapex/msg/generic_value_message.hpp>");
        return addSlot(
            connection_types::makeTokenType<connection_types::GenericValueMessage<T>>(), label,
            [instance, member_fn](TokenPtr token) {
                auto input_msg = std::dynamic_pointer_cast<connection_types::GenericValueMessage<T> const>(token->getTokenData());
                (instance->*member_fn)(input_msg->value);
            },
            active, blocking);
    }
    template <typename T>
    Event* addEvent(const std::string& label, typename std::enable_if<connection_types::should_use_value_message<T>::value>::type* = 0)
    {
        static_assert(IS_COMPLETE(connection_types::GenericValueMessage<T>), "connection_types::GenericValueMessage is not included: "
                                                                             "#include <csapex/msg/generic_value_message.hpp>");
        return addEvent(connection_types::makeTokenType<connection_types::GenericValueMessage<T>>(), label);
    }

    /// multiple input types allowed
    template <typename... Types>
    Input* addMultiInput(const std::string& label)
    {
        return addInput(multi_type::make<Types...>(), label, false);
    }

    template <typename... Types>
    Input* addOptionalMultiInput(const std::string& label)
    {
        return addInput(multi_type::make<Types...>(), label, true);
    }

    virtual bool isParameterInput(const UUID& id) const = 0;
    virtual bool isParameterOutput(const UUID& id) const = 0;

    /*
     * SIGNALING
     */
    Slot* addActiveSlot(const std::string& label, std::function<void()> callback, bool blocking = true);
    Slot* addSlot(const std::string& label, std::function<void()> callback, bool active = false, bool blocking = true);

    template <typename T>
    Slot* addSlot(const std::string& label, std::function<void(const TokenPtr&)> callback, bool active = false, bool blocking = true)
    {
        return addSlot(connection_types::makeTokenType<T>(), label, callback, active, blocking);
    }

    template <typename T>
    Event* addEvent(const std::string& label, typename std::enable_if<connection_types::should_use_no_generic_message<T>::value>::type* = 0)
    {
        return addEvent(connection_types::makeTokenType<T>(), label);
    }
    Event* addEvent(const std::string& label);

    virtual void removeInput(const UUID& uuid) = 0;
    virtual void removeOutput(const UUID& uuid) = 0;
    virtual void removeEvent(const UUID& uuid) = 0;
    virtual void removeSlot(const UUID& uuid) = 0;

    /*
     * Accessors
     */

    std::vector<InputPtr> getMessageInputs() const;
    std::vector<OutputPtr> getMessageOutputs() const;
    std::vector<SlotPtr> getSlots() const;
    std::vector<EventPtr> getEvents() const;

    /*
     * MISCELLANEOUS
     */

    virtual bool isSource() const = 0;
    virtual bool isSink() const = 0;

    bool isProcessingEnabled() const;
    void setProcessingEnabled(bool enabled);

    bool isError() const;
    void setNoError();
    void setInfo(const std::string& msg);
    void setWarning(const std::string& msg);
    void setError(const std::string& msg);

    void setVariadic(bool variadic);

    /**
     * Raw construction, handle with care!
     */
    virtual Input* addInput(TokenTypeConstPtr type, const std::string& label, bool optional) = 0;
    virtual Output* addOutput(TokenTypeConstPtr type, const std::string& label) = 0;
    virtual Slot* addSlot(TokenTypeConstPtr type, const std::string& label, std::function<void(Slot*, const TokenPtr&)> callback, bool active, bool blocking) = 0;
    virtual Slot* addSlot(TokenTypeConstPtr type, const std::string& label, std::function<void(const TokenPtr&)> callback, bool active, bool blocking) = 0;
    virtual Slot* addSlot(TokenTypeConstPtr type, const std::string& label, std::function<void()> callback, bool active, bool blocking) = 0;
    virtual Event* addEvent(TokenTypeConstPtr type, const std::string& label) = 0;

protected:
    virtual std::vector<ConnectablePtr> getExternalConnectors() const = 0;
    virtual std::vector<InputPtr> getExternalInputs() const = 0;
    virtual std::vector<OutputPtr> getExternalOutputs() const = 0;
    virtual std::vector<SlotPtr> getExternalSlots() const = 0;
    virtual std::vector<EventPtr> getExternalEvents() const = 0;

protected:
    bool variadic_;

private:
    mutable NodeWorker* node_worker_;
};

}  // namespace csapex

#endif  // NODE_MODIFIER_H
