#ifndef YAML_HPP
#define YAML_HPP

/// PROJECT
#include <csapex/model/model_fwd.h>
#include <csapex_core/csapex_core_export.h>

/// SYSTEM
#include <csapex/utility/yaml.h>

/// YAML
namespace YAML
{
// template<>
// struct CSAPEX_CORE_EXPORT convert<csapex::TokenData> {
//    static Node encode(const csapex::TokenData& rhs);
//    static bool decode(const Node& node, csapex::TokenData& rhs);
//};
template <>
struct CSAPEX_CORE_EXPORT convert<csapex::TokenDataPtr>
{
    static Node encode(const csapex::TokenDataPtr& rhs);
    static bool decode(const Node& node, csapex::TokenDataPtr& rhs);
};
template <>
struct CSAPEX_CORE_EXPORT convert<csapex::TokenDataConstPtr>
{
    static Node encode(const csapex::TokenDataConstPtr& rhs);
    static bool decode(const Node& node, csapex::TokenDataConstPtr& rhs);
};
}  // namespace YAML

#endif  // YAML_HPP
