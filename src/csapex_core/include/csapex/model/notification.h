#ifndef NOTIFICATION_H
#define NOTIFICATION_H

/// COMPONENT
#include <csapex/utility/uuid.h>
#include <csapex/model/error_state.h>
#include <csapex/serialization/serializable.h>

/// SYSTEM
#include <sstream>

namespace csapex {

class Notification : public Serializable
{
public:
    AUUID auuid;
    std::stringstream message;
    ErrorState::ErrorLevel error;

    Notification() = default;

    Notification(const Notification& copy);
    void operator = (const Notification& copy);

    Notification(const std::string& message);
    Notification(AUUID uuid, const std::string& message);
    Notification(AUUID uuid, const std::string& message, ErrorState::ErrorLevel error);

    template <typename T>
    Notification& operator << (const T& val)
    {
        message << val;
        msg_dirty_ = true;
        return *this;
    }

    bool operator == (const Notification& other) const;

    std::string getMessage() const;

    virtual void serialize(SerializationBuffer &data) const override;
    virtual void deserialize(const SerializationBuffer& data) override;

protected:
    virtual std::shared_ptr<Clonable> makeEmptyClone() const override;

private:
    mutable bool msg_dirty_;
    mutable std::string msg_cache_;
};

}

#endif // NOTIFICATION_H
