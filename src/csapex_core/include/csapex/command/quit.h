#ifndef QUIT_H
#define QUIT_H

/// COMPONENT
#include "command_impl.hpp"
#include <csapex/utility/uuid.h>
#include <csapex/utility/assert.h>

/// SYSTEM
#include <boost/any.hpp>

namespace csapex
{
namespace command
{
struct CSAPEX_COMMAND_EXPORT Quit : public CommandImplementation<Quit>
{
    COMMAND_HEADER_NO_DEFAULT(Quit);

public:
    typedef std::shared_ptr<Quit> Ptr;

public:
    Quit();

    std::string getDescription() const override;

    bool isHidden() const override;
    bool isUndoable() const override;

    void serialize(SerializationBuffer& data, SemanticVersion& version) const override;
    void deserialize(const SerializationBuffer& data, const SemanticVersion& version) override;

protected:
    bool doExecute() override;
    bool doUndo() override;
    bool doRedo() override;

private:
    AUUID uuid;

    boost::any value;
};

}  // namespace command

}  // namespace csapex

#endif  // QUIT_H
