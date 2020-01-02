#ifndef TRIGGER_H
#define TRIGGER_H

/// COMPONENT
#include <csapex/signal/signal_fwd.h>
#include <csapex/msg/static_output.h>

namespace csapex
{
class CSAPEX_CORE_EXPORT Event : public StaticOutput
{
    friend class Graph;

public:
    Event(const UUID& uuid, ConnectableOwnerWeakPtr owner = ConnectableOwnerWeakPtr());
    ~Event() override;

    ConnectorType getConnectorType() const override
    {
        return ConnectorType::EVENT;
    }

    /**
     * @brief trigger triggers an event with a "Nothing" token
     */
    void trigger();

    /**
     * @brief triggerWith triggers an event with a specified token
     * @param token
     */
    void triggerWith(TokenPtr token);

    bool isSynchronous() const override;

    void reset() override;

public:
    slim_signal::Signal<void()> triggered;
};

}  // namespace csapex

#endif  // TRIGGER_H
