#ifndef MESSAGE_FACTORY_H
#define MESSAGE_FACTORY_H

/// COMPONENT
#include <csapex/model/token_data.h>
#include <csapex/msg/message.h>
#include <csapex/msg/token_traits.h>
#include <csapex/msg/serialization_format.h>

/// PROJECT
#include <csapex/utility/singleton.hpp>

/// SYSTEM
#include <map>
#include <string>
#include <functional>
#include <boost/type_traits.hpp>
#include <typeindex>

namespace YAML
{
class Emitter;
}

namespace csapex
{
class CSAPEX_CORE_EXPORT MessageFactory : public Singleton<MessageFactory>
{
    friend class Singleton<MessageFactory>;

public:
    typedef std::function<TokenData::Ptr()> MessageConstructor;
    typedef std::function<TokenType::Ptr()> TokenTypeConstructor;

public:
    bool isMessageRegistered(const std::string& type) const;

    template <typename M>
    static TokenData::Ptr createMessage()
    {
        return makeEmpty<M>();
    }
    template <template <typename> class Wrapper, typename M>
    static TokenData::Ptr createDirectMessage()
    {
        return makeEmpty<Wrapper<M>>();
    }
    template <typename M>
    static TokenType::Ptr createTokenType()
    {
        return connection_types::makeTokenType<M>();
    }

    static TokenData::Ptr createMessage(const std::string& type);
    static TokenType::Ptr createMessageType(const std::string& type);

    static TokenData::Ptr readFile(const std::string& path);
    static int writeFile(const std::string& path, const std::string& base, const int suffix, const TokenData& msg, serialization::Format format);

    static TokenData::Ptr readYamlFile(const std::string& path);
    static void writeYamlFile(const std::string& path, const TokenData& msg);

    static TokenData::Ptr readBinaryFile(const std::string& path);
    static void writeBinaryFile(const std::string& path, const TokenData& msg);

    void shutdown() override;

public:
    template <template <typename> class Wrapper, typename M>
    static void registerDirectMessage()
    {
        MessageFactory& instance = MessageFactory::instance();
        std::string type = connection_types::serializationName<Wrapper<M>>();
        if (!instance.isMessageRegistered(type)) {
            instance.registerMessage(type, std::type_index(typeid(Wrapper<M>)), std::bind(&MessageFactory::createDirectMessage<Wrapper, M>), std::bind(&MessageFactory::createTokenType<Wrapper<M>>));
        }
    }

    template <template <typename> class Wrapper, typename M>
    static void deregisterDirectMessage()
    {
        MessageFactory& instance = MessageFactory::instance();
        std::string type = connection_types::serializationName<Wrapper<M>>();
        instance.deregisterMessage(type);
    }

    template <typename M>
    static void registerMessage()
    {
        MessageFactory::instance().registerMessage(connection_types::serializationName<M>(), std::type_index(typeid(M)), std::bind(&MessageFactory::createMessage<M>),
                                                   std::bind(&MessageFactory::createTokenType<M>));
    }
    template <typename M>
    static void deregisterMessage()
    {
        MessageFactory::instance().deregisterMessage(connection_types::serializationName<M>());
    }

private:
    MessageFactory();
    ~MessageFactory() override;

    static void registerMessage(std::string type, std::type_index typeindex, MessageConstructor msg_constructor, TokenTypeConstructor type_constructor);
    static void deregisterMessage(std::string type);

private:
    std::map<std::string, MessageConstructor> typename_to_message_constructor;
    std::map<std::string, TokenTypeConstructor> typename_to_token_type_constructor;
    std::map<std::string, std::type_index> typename_to_type_index;
};

template <typename T>
struct MessageConstructorRegistered
{
    MessageConstructorRegistered()
    {
        csapex::MessageFactory::registerMessage<T>();
    }
    ~MessageConstructorRegistered()
    {
        csapex::MessageFactory::deregisterMessage<T>();
    }
};

template <template <typename> class Wrapper, typename T>
struct DirectMessageConstructorRegistered
{
    DirectMessageConstructorRegistered()
    {
        csapex::MessageFactory::registerDirectMessage<Wrapper, T>();
    }
    ~DirectMessageConstructorRegistered()
    {
        csapex::MessageFactory::deregisterDirectMessage<Wrapper, T>();
    }
};

}  // namespace csapex

#endif  // MESSAGE_FACTORY_H
