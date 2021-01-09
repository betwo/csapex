#ifndef TOKEN_TYPE_H
#define TOKEN_TYPE_H

#include <csapex_core/csapex_core_export.h>
#include <csapex/serialization/streamable.h>

namespace csapex
{
class CSAPEX_CORE_EXPORT TokenType : public Streamable
{
public:
    typedef std::shared_ptr<TokenType> Ptr;
    typedef std::shared_ptr<const TokenType> ConstPtr;

public:
    TokenType(const std::string& type_name);
    TokenType(const std::string& type_name, const std::string& descriptive_name);

    ~TokenType() = default;

    virtual bool canConnectTo(const TokenType* other_side) const;
    virtual bool acceptsConnectionFrom(const TokenType* other_side) const;

    virtual std::string descriptiveName() const;
    std::string typeName() const;

    virtual bool isContainer() const;
    virtual Ptr nestedType() const;

    void serialize(SerializationBuffer& data, SemanticVersion& version) const override;
    void deserialize(const SerializationBuffer& data, const SemanticVersion& version) override;

protected:
    TokenType() = default;
    void setDescriptiveName(const std::string& descriptiveName);

protected:
    std::string type_name_;
    std::string descriptive_name_;
};

}  // namespace csapex

#endif  // TOKEN_TYPE_H
