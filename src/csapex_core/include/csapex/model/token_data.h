#ifndef TOKEN_DATA_H
#define TOKEN_DATA_H

/// COMPONENT
#include <csapex_core/csapex_core_export.h>
#include <csapex/serialization/streamable.h>
#include <csapex/model/token_type.h>
#include <csapex/model/model_fwd.h>

/// SYSTEM
#include <memory>
#include <string>

namespace csapex
{
class CSAPEX_CORE_EXPORT TokenData : public Streamable
{
public:
    static const uint8_t PACKET_TYPE_ID = 8;

    typedef std::shared_ptr<TokenData> Ptr;
    typedef std::shared_ptr<const TokenData> ConstPtr;

public:
    TokenData(TokenTypePtr type);
    ~TokenData() override;

    std::shared_ptr<TokenType> getType() const;

    std::string typeName() const;
    std::string descriptiveName() const;

    virtual bool isValid() const;

    virtual ConstPtr nestedValue(std::size_t i) const;
    virtual void addNestedValue(const ConstPtr& msg);
    virtual std::size_t nestedValueCount() const;

    virtual void writeNative(const std::string& file, const std::string& base, const std::string& suffix) const;

    uint8_t getPacketType() const final;

protected:
    TokenData();

private:
    TokenTypePtr type_;
};

}  // namespace csapex

#endif  // TOKEN_H
