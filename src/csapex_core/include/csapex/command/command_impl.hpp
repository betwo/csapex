#ifndef COMMAND_IMPL_HPP
#define COMMAND_IMPL_HPP

/// PROJECT
#include <csapex/command/command.h>
#include <csapex/utility/type.h>

namespace csapex
{
template <typename I>
class CommandImplementation : public Command
{
protected:
    CLONABLE_IMPLEMENTATION_NO_ASSIGNMENT(I);

public:
    static std::string typeName()
    {
        return type2nameWithoutNamespace(typeid(I));
    }

protected:
    CommandImplementation(const AUUID& graph_uuid) : Command(graph_uuid)
    {
    }

    CommandImplementation()
    {
    }

    std::string getType() const override
    {
        return typeName();
    }

    virtual bool cloneData(const I& other)
    {
        dynamic_cast<I&>(*this) = other;
        return true;
    }
};

}  // namespace csapex

#define COMMAND_HEADER_NO_DEFAULT(Instance)                                                                                                                                                            \
protected:                                                                                                                                                                                             \
    friend class CommandImplementation<Instance>

#define COMMAND_HEADER(Instance)                                                                                                                                                                       \
public:                                                                                                                                                                                                \
    Instance()                                                                                                                                                                                         \
    {                                                                                                                                                                                                  \
    }                                                                                                                                                                                                  \
                                                                                                                                                                                                       \
protected:                                                                                                                                                                                             \
    COMMAND_HEADER_NO_DEFAULT(Instance)

#endif  // COMMAND_IMPL_HPP
