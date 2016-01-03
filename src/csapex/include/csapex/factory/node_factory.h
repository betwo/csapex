#ifndef NODE_FACTORY_H
#define NODE_FACTORY_H

/// COMPONENT
#include <csapex/model/node_constructor.h>
#include <csapex/utility/uuid.h>

/// PROJECT
#include <csapex/core/core_fwd.h>
#include <csapex/model/model_fwd.h>
#include <csapex/plugin/plugin_fwd.h>

/// SYSTEM
#include <vector>
#include <csapex/utility/slim_signal.h>
#include <unordered_map>

namespace csapex
{

class NodeFactory
{
public:
    typedef std::shared_ptr<NodeFactory> Ptr;

public:
    NodeFactory(PluginLocator *locator);
    ~NodeFactory();

    NodeFactory(const NodeFactory&) = delete;
    NodeFactory& operator = (const NodeFactory&) = delete;

    void loadPlugins();
    void shutdown();

public:
    void registerNodeType(NodeConstructor::Ptr provider, bool suppress_signals = false);

    bool isValidType(const std::string& type) const;

    NodeConstructorPtr getConstructor(const std::string& type);
    std::vector<NodeConstructorPtr> getConstructors();

    NodeHandlePtr makeNode(const std::string& type, const UUID& uuid);
    NodeHandlePtr makeNode(const std::string& type, const UUID& uuid, NodeStatePtr state);

    std::map<std::string, std::vector<NodeConstructor::Ptr> > getTagMap();

public:
    csapex::slim_signal::Signal<void(const std::string&)> loaded;
    csapex::slim_signal::Signal<void()> new_node_type;

protected:
    void ensureLoaded();
    void rebuildPrototypes();
    void rebuildMap();

    NodeHandlePtr makeSingleNode(NodeConstructor::Ptr content, const UUID& uuid);

protected:
    csapex::PluginLocator* plugin_locator_;

    std::map<std::string, std::vector<NodeConstructor::Ptr> > tag_map_;
    std::vector<NodeConstructor::Ptr> constructors_;

    std::shared_ptr<PluginManager<Node>> node_manager_;

    bool tag_map_has_to_be_rebuilt_;
};

}

#endif // NODE_FACTORY_H
