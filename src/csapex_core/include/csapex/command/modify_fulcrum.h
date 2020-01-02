#ifndef MODIFY_FULCRUM_H
#define MODIFY_FULCRUM_H

/// COMPONENT
#include "command_impl.hpp"
#include <csapex/data/point.h>

namespace csapex
{
namespace command
{
class CSAPEX_COMMAND_EXPORT ModifyFulcrum : public CommandImplementation<ModifyFulcrum>
{
    COMMAND_HEADER(ModifyFulcrum);

public:
    ModifyFulcrum(const AUUID& graph_uuid, int connection_id, int fulcrum_id, int f_type, const Point& f_handle_in, const Point& f_handle_out, int t_type, const Point& t_handle_in,
                  const Point& t_handle_out);

    std::string getDescription() const override;

    void serialize(SerializationBuffer& data, SemanticVersion& version) const override;
    void deserialize(const SerializationBuffer& data, const SemanticVersion& version) override;

protected:
    bool doExecute() override;
    bool doUndo() override;
    bool doRedo() override;

private:
    int connection_id;
    int fulcrum_id;

    int f_type;
    Point f_in;
    Point f_out;

    int t_type;
    Point t_in;
    Point t_out;
};

}  // namespace command

}  // namespace csapex

#endif  // MODIFY_FULCRUM_H
