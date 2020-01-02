#ifndef APEX_MESSAGE_PROVIDER_H
#define APEX_MESSAGE_PROVIDER_H

/// COMPONENT
#include <csapex/msg/message_provider.h>
#include <csapex_core/csapex_core_export.h>

namespace csapex
{
class CSAPEX_CORE_EXPORT ApexMessageProvider : public MessageProvider
{
public:
    static std::shared_ptr<MessageProvider> make();

public:
    void load(const std::string& file) override;

    bool hasNext() override;
    connection_types::Message::Ptr next(std::size_t slot) override;
    std::string getLabel(std::size_t slot) const override;

    std::vector<std::string> getExtensions() const override;

    GenericStatePtr getState() const override;
    void setParameterState(GenericStatePtr memento) override;

private:
    std::string file_;
    connection_types::Message::Ptr msg_;
};

}  // namespace csapex

#endif  // APEX_MESSAGE_PROVIDER_H
