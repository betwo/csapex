#ifndef COMMAND_H
#define COMMAND_H

/// COMPONENT
#include <csapex/command/command_fwd.h>
#include <csapex/core/core_fwd.h>
#include <csapex/utility/uuid.h>
#include <csapex_core/csapex_command_export.h>
#include <csapex/serialization/streamable.h>

/// PROJECT
#include <csapex/model/model_fwd.h>
#include <csapex/scheduling/scheduling_fwd.h>
#include <csapex/factory/factory_fwd.h>

/// SYSTEM
#include <memory>
#include <vector>
#include <functional>

namespace csapex
{
class Designer;

class CSAPEX_COMMAND_EXPORT Command : public Streamable
{
public:
    class CSAPEX_COMMAND_EXPORT Access
    {
        friend class Group;
        friend class CommandDispatcher;
        friend class CommandDispatcherProxy;
        friend class command::Meta;

    private:
        static bool executeCommand(CommandPtr cmd);
        static bool undoCommand(CommandPtr cmd);
        static bool redoCommand(CommandPtr cmd);
    };

    friend class command::Meta;

    static const uint8_t PACKET_TYPE_ID = 1;

public:
    typedef std::shared_ptr<Command> Ptr;

public:
    Command(const AUUID& graph_uuid);

    virtual void init(GraphFacadeImplementation* graph_facade, CsApexCore& core);
    virtual bool isUndoable() const;
    virtual bool isHidden() const;

    void setAfterSavepoint(bool save);
    bool isAfterSavepoint();

    void setBeforeSavepoint(bool save);
    bool isBeforeSavepoint();

    virtual void accept(int level, std::function<void(int level, const Command&)> callback) const;

    virtual std::string getType() const = 0;
    virtual std::string getDescription() const = 0;

    virtual uint8_t getPacketType() const override;

    virtual void serialize(SerializationBuffer& data, SemanticVersion& version) const override = 0;
    virtual void deserialize(const SerializationBuffer& data, const SemanticVersion& version) override = 0;

protected:
    Command();

    bool executeCommand(CommandPtr cmd);
    bool undoCommand(CommandPtr cmd);
    bool redoCommand(CommandPtr cmd);

    virtual bool doExecute() = 0;
    virtual bool doUndo() = 0;
    virtual bool doRedo() = 0;

    GraphFacadeImplementation* getRoot();

    GraphFacadeImplementation* getGraphFacade();
    GraphImplementationPtr getGraph();
    SubgraphNodePtr getSubgraphNode();

    NodeFactoryImplementation* getNodeFactory();

    GraphFacade* getSubGraph(const UUID& graph_id);
    ThreadPool* getRootThreadPool();

protected:
    AUUID graph_uuid;
    CsApexCore* core_;

    GraphFacadeImplementation* root_graph_facade_;

private:
    static std::vector<Command::Ptr> undo_later;

    bool before_save_point_;
    bool after_save_point_;

    bool initialized_;
};

}  // namespace csapex

#endif  // COMMAND_H
