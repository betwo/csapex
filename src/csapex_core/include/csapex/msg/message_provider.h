#ifndef MESSAGE_PROVIDER_H
#define MESSAGE_PROVIDER_H

/// COMPONENT
#include <csapex/serialization/serializable.h>
#include <csapex/msg/message.h>
#include <csapex/model/generic_state.h>

/// SYSTEM
#include <memory>

namespace csapex
{
class CSAPEX_CORE_EXPORT MessageProvider
{
public:
    typedef std::shared_ptr<MessageProvider> Ptr;

public:
    virtual ~MessageProvider();

    TokenType::ConstPtr getType() const;

    virtual void load(const std::string& file) = 0;
    virtual void parameterChanged();

    void setName(const std::string& name);
    std::string getName() const;

    std::size_t slotCount() const;
    virtual std::string getLabel(std::size_t slot) const;

    virtual bool hasNext() = 0;
    virtual void prepareNext();
    virtual connection_types::Message::Ptr next(std::size_t slot) = 0;

    virtual void restart();

    virtual std::vector<std::string> getExtensions() const = 0;

    virtual GenericStatePtr getState() const = 0;
    virtual void setParameterState(GenericStatePtr memento) = 0;

    std::vector<csapex::param::ParameterPtr> getParameters() const;

public:
    slim_signal::Signal<void(std::size_t)> slot_count_changed;

    slim_signal::Signal<void()> begin;

protected:
    MessageProvider();
    void setType(TokenType::Ptr type);
    void setSlotCount(std::size_t slot_count);

protected:
    GenericState state;

private:
    TokenType::Ptr type_;

    std::string name_;
    std::size_t slot_count_;
};

}  // namespace csapex

#endif  // MESSAGE_PROVIDER_H
