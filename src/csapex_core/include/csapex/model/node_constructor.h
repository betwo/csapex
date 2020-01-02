#ifndef NODE_CONSTRUCTOR_H
#define NODE_CONSTRUCTOR_H

/// COMPONENT
#include <csapex/command/command_fwd.h>
#include <csapex_core/csapex_core_export.h>
#include <csapex/model/model_fwd.h>
#include <csapex/param/param_fwd.h>
#include <csapex/serialization/serializable.h>
#include <csapex/utility/utility_fwd.h>
#include <csapex/utility/uuid.h>

/// SYSTEM
#include <typeinfo>
#include <functional>
#include <csapex/utility/slim_signal.h>

namespace csapex
{
class CSAPEX_CORE_EXPORT NodeConstructor : public Serializable
{
protected:
    CLONABLE_IMPLEMENTATION(NodeConstructor);

    friend class command::AddNode;
    friend class NodeFactory;
    friend class SerializationBuffer;

public:
    struct NodeConstructionException : public std::runtime_error
    {
        NodeConstructionException(const std::string& what);
    };

public:
    typedef std::shared_ptr<NodeConstructor> Ptr;

public:
    NodeConstructor();
    NodeConstructor(const std::string& type, std::function<NodePtr()> c);

    ~NodeConstructor() override;

    std::string getType() const;

    NodeConstructor& setTags(const std::string& tags);
    NodeConstructor& setTags(const std::vector<std::string>& tags);
    NodeConstructor& setTags(const std::vector<TagPtr>& tags);
    std::vector<TagPtr> getTags() const;

    NodeConstructor& setProperties(const std::vector<std::string>& properties);
    std::vector<std::string> getProperties() const;

    NodeConstructor& setIcon(const std::string& icon);
    std::string getIcon() const;

    NodeConstructor& setDescription(const std::string& description);
    std::string getDescription() const;

    NodeHandlePtr makePrototype() const;
    NodeHandlePtr makeNodeHandle(const UUID& uuid, const UUIDProviderPtr& uuid_provider) const;

    std::vector<param::ParameterPtr> getParameters() const;

    NodePtr makeNode() const;

    void serialize(SerializationBuffer& data, SemanticVersion& version) const override;
    void deserialize(const SerializationBuffer& data, const SemanticVersion& version) override;

protected:
    NodeConstructor(const std::string& type);

protected:
    std::string type_;
    std::string descr_;
    std::string icon_;
    std::vector<TagPtr> tags_;

    mutable std::vector<std::string> properties_;
    mutable bool properties_loaded_;

    mutable std::shared_ptr<UUIDProvider> tmp_uuid_provider_;

    std::function<NodePtr()> c;
};

}  // namespace csapex
#endif  // NODE_CONSTRUCTOR_H
