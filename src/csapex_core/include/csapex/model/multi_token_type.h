#ifndef MULTI_TOKEN_TYPE_H
#define MULTI_TOKEN_TYPE_H

#include <csapex_core/csapex_core_export.h>
#include <csapex/model/token_type.h>

#include <functional>

namespace csapex
{
class CSAPEX_CORE_EXPORT MultiTokenType : public TokenType
{
private:
    CLONABLE_IMPLEMENTATION(MultiTokenType);

public:
    typedef std::shared_ptr<MultiTokenType> Ptr;
    typedef std::shared_ptr<const MultiTokenType> ConstPtr;

public:
    MultiTokenType(const std::string& type_name);
    MultiTokenType(const std::string& type_name, const std::string& descriptive_name);

    ~MultiTokenType() = default;

    bool canConnectTo(const TokenType* other_side) const override;
    bool acceptsConnectionFrom(const TokenType* other_side) const override;

    bool isContainer() const override;
    TokenType::Ptr nestedType() const override;

    void serialize(SerializationBuffer& data, SemanticVersion& version) const override;
    void deserialize(const SerializationBuffer& data, const SemanticVersion& version) override;

protected:
    MultiTokenType() = default;
};

}  // namespace csapex

#endif  // MULTI_TOKEN_TYPE_H
