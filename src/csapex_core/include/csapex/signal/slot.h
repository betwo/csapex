#ifndef SLOT_H
#define SLOT_H

/// COMPONENT
#include <csapex/signal/signal_fwd.h>
#include <csapex/msg/input.h>
#include <csapex/model/token.h>

/// SYSTEM
#include <mutex>
#include <condition_variable>
#include <deque>

namespace csapex
{
class CSAPEX_CORE_EXPORT Slot : public Input
{
    friend class Event;

public:
    Slot(std::function<void()> callback, const UUID& uuid, bool active, bool blocking = true, ConnectableOwnerWeakPtr owner = ConnectableOwnerWeakPtr());
    Slot(std::function<void(const TokenPtr&)> callback, const UUID& uuid, bool active, bool blocking = true, ConnectableOwnerWeakPtr owner = ConnectableOwnerWeakPtr());
    Slot(std::function<void(Slot*, const TokenPtr&)> callback, const UUID& uuid, bool active, bool blocking = true, ConnectableOwnerWeakPtr owner = ConnectableOwnerWeakPtr());

    template <typename TokenType>
    Slot(std::function<void(const std::shared_ptr<TokenType const>&)> callback, const UUID& uuid, bool active)
      : Slot(
            [callback](const TokenPtr& token) {
                auto t = std::dynamic_pointer_cast<TokenType const>(token->getTokenData());
                apex_assert_hard(t);
                callback(t);
            },
            uuid, active)
    {
    }

    ~Slot() override;

    void setToken(TokenPtr message) override;

    int maxConnectionCount() const override;

    ConnectorType getConnectorType() const override
    {
        return ConnectorType::SLOT_T;
    }

    bool isActive() const;
    bool isBlocking() const;
    bool isEnabled() const override;

    bool isSynchronous() const override;

    void removeConnection(Connectable* connection) override;

    void notifyMessageAvailable(Connection* connection) override;
    void notifyMessageProcessed() override;

    void enable() override;
    void disable() override;

    void reset() override;

    void handleEvent();

    void notifyEventHandled();

public:
    slim_signal::Signal<void(const TokenPtr&)> token_set;
    slim_signal::Signal<void()> triggered;

protected:
    void addStatusInformation(std::stringstream& status_stream) const override;

private:
    void tryNextToken();

protected:
    std::function<void(Slot*, const TokenPtr&)> callback_;

    bool active_;
    bool blocking_;

    long guard_;

private:
    std::deque<Connection*> available_connections_;

    std::recursive_mutex available_connections_mutex_;
};

}  // namespace csapex
#endif  // SLOT_H
