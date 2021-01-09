#ifndef TOKEN_DATA_H
#define TOKEN_DATA_H

/// COMPONENT
#include <csapex_core/csapex_core_export.h>
#include <csapex/serialization/streamable.h>
#include <csapex/model/token_type.h>

/// SYSTEM
#include <memory>
#include <string>

namespace csapex
{
class CSAPEX_CORE_EXPORT TokenData : public TokenType
{
public:
    static const uint8_t PACKET_TYPE_ID = 8;

    typedef std::shared_ptr<TokenData> Ptr;
    typedef std::shared_ptr<const TokenData> ConstPtr;

public:
    TokenData(const std::string& type_name);
    TokenData(const std::string& type_name, const std::string& descriptive_name);
    ~TokenData() override;

    std::shared_ptr<TokenType> toType() const;

    virtual bool isValid() const;

    virtual ConstPtr nestedValue(std::size_t i) const;
    virtual void addNestedValue(const ConstPtr& msg);
    virtual std::size_t nestedValueCount() const;

    virtual void writeNative(const std::string& file, const std::string& base, const std::string& suffix) const;

    uint8_t getPacketType() const final;

protected:
    TokenData();
};

}  // namespace csapex

#endif  // TOKEN_H
