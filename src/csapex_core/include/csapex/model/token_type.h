#ifndef TOKEN_TYPE_H
#define TOKEN_TYPE_H

#include <csapex_core/csapex_core_export.h>
#include <csapex/serialization/streamable.h>

#include <functional>

namespace csapex
{
class CSAPEX_CORE_EXPORT TokenType : public Streamable
{
public:
    static const uint8_t PACKET_TYPE_ID = 9;

private:
    CLONABLE_IMPLEMENTATION(TokenType);

public:
    typedef std::shared_ptr<TokenType> Ptr;
    typedef std::shared_ptr<const TokenType> ConstPtr;

public:
    TokenType(const std::string& type_name);
    TokenType(const std::string& type_name, const std::string& descriptive_name);
    TokenType(std::function<bool(const TokenType&, const TokenType&)> acceptor_fn, const std::string& type_name, const std::string& descriptive_name);
    TokenType(std::function<bool(const TokenType&, const TokenType&)> acceptor_fn, std::function<bool(const TokenType&, const TokenType&)> connector_fn, const std::string& type_name,
              const std::string& descriptive_name);

    ~TokenType() = default;

    virtual bool canConnectTo(const TokenType* other_side) const;
    virtual bool acceptsConnectionFrom(const TokenType* other_side) const;

    virtual std::string descriptiveName() const;
    std::string typeName() const;

    virtual bool isContainer() const;
    virtual Ptr nestedType() const;

    void serialize(SerializationBuffer& data, SemanticVersion& version) const override;
    void deserialize(const SerializationBuffer& data, const SemanticVersion& version) override;

    uint8_t getPacketType() const final;

protected:
    TokenType() = default;
    void setDescriptiveName(const std::string& descriptiveName);

protected:
    std::string type_name_;
    std::string descriptive_name_;

    std::function<bool(const TokenType& left, const TokenType& right)> connector_fn_;
    std::function<bool(const TokenType& left, const TokenType& right)> acceptor_fn_;
};

}  // namespace csapex

#endif  // TOKEN_TYPE_H
