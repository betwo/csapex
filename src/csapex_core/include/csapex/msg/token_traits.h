#ifndef token_traits_H
#define token_traits_H

/// COMPONENT
#include <csapex/model/token_data.h>
#include <csapex/utility/tmp.hpp>
#include <csapex/model/model_fwd.h>
#include <csapex/utility/data_traits.hpp>

/// SYSTEM
#include <string>
#include <type_traits>
#include <vector>

namespace csapex
{
namespace connection_types
{
/// TRAITS
template <typename T>
struct type;

namespace detail
{
template <typename T, typename = void>
struct TokenTypeConstructor
{
    static TokenTypePtr makeTokenType()
    {
        typedef typename std::remove_const<T>::type TT;
        return std::make_shared<TokenType>(
            // name
            type2name(typeid(TT)),
            // description
            type<TT>::name());
    }
};

template <typename T>
struct TokenTypeConstructor<T, std::void_t<decltype(type<T>::makeTokenType())>>
{
    static TokenTypePtr makeTokenType()
    {
        typedef typename std::remove_const<T>::type TT;
        return type<TT>::makeTokenType();
    }
};

}  // namespace detail

template <typename T>
TokenTypePtr makeTokenType()
{
    return detail::TokenTypeConstructor<T>::makeTokenType();
}

// inline TokenTypePtr makeTokenType(const std::string& type_name)
// {
//     return std::make_shared<TokenType>(type_name);
// }

// inline TokenTypePtr makeTokenType(const std::string& type_name, const std::string& descriptive_name)
// {
//     return std::make_shared<TokenType>(type_name, descriptive_name);
// }

template <typename T>
inline std::string serializationName()
{
    typedef typename std::remove_const<T>::type TT;
    return type<TT>::name();
}

TokenPtr makeToken(const TokenDataConstPtr& data);

template <typename T>
inline TokenPtr makeEmptyToken()
{
    return makeToken(makeEmpty<T>());
}

template <typename M, bool is_message>
struct MessageContainer;

template <typename M>
struct MessageContainer<M, true>
{
    typedef M type;

    static M& access(M& msg)
    {
        return msg;
    }
    static const M& accessConst(const M& msg)
    {
        return msg;
    }
};

template <template <class> class Container, typename T, class Enable = void>
class MessageConversionHook
{
public:
    static void registerConversion()
    {
        // do nothing
    }
};

HAS_MEM_TYPE(Ptr, has_ptr_member);
HAS_MEM_TYPE(element_type, has_elem_type_member);

template <typename T>
struct is_std_vector : std::false_type
{
};
template <typename T, typename Allocator>
struct is_std_vector<std::vector<T, Allocator>> : std::true_type
{
};

constexpr std::size_t getMaxValueMessageSize()
{
    return 1024 * 1024;
}

// clang-format off
template <typename M>
struct should_use_pointer_message
{
    static constexpr bool value = 
        std::is_class<M>::value 
        && (
            has_ptr_member<M>::value // has nested ptr member type
            || sizeof(M) >= getMaxValueMessageSize() // is too large
            || !std::is_copy_assignable<M>::value // is not copyable
        )
        && !std::is_same<std::string, M>::value 
        && !std::is_base_of<TokenData, M>::value;
};
// clang-format on

template <typename M>
struct should_use_value_message
{
    static constexpr bool value = !should_use_pointer_message<M>::value && !has_elem_type_member<M>::value &&  // reject shared_ptr
                                  !std::is_base_of<TokenData, M>::value;
};

template <typename M>
struct should_use_no_generic_message
{
    static constexpr bool value = !should_use_pointer_message<M>::value && !should_use_value_message<M>::value;
};

}  // namespace connection_types
}  // namespace csapex

#endif  // token_traits_H
