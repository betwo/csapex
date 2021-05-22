/// HEADER
#include <csapex/model/token.h>

using namespace csapex;

Token::Token(const TokenDataConstPtr& token) : data_(token), activity_modifier_(ActivityModifier::NONE), seq_no_(-1)
{
}

Token::Token() : activity_modifier_(ActivityModifier::NONE), seq_no_(-1)
{
}

void Token::setActivityModifier(ActivityModifier active)
{
    activity_modifier_ = active;
}

bool Token::hasActivityModifier() const
{
    return activity_modifier_ != ActivityModifier::NONE;
}

ActivityModifier Token::getActivityModifier() const
{
    return activity_modifier_;
}

TokenDataConstPtr Token::getTokenData() const
{
    return data_;
}

TokenTypeConstPtr Token::getTokenType() const
{
    return data_->getType();
}

int Token::getSequenceNumber() const
{
    return seq_no_;
}

void Token::setSequenceNumber(int seq_no) const
{
    seq_no_ = seq_no;
}

bool Token::cloneData(const Token& other)
{
    data_ = other.data_->cloneAs<TokenData>();
    activity_modifier_ = other.activity_modifier_;
    seq_no_ = other.seq_no_;

    return true;
}

Token::Ptr Token::makeEmpty()
{
    return Ptr{ new Token };
}
