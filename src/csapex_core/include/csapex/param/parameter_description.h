#ifndef PARAMETER_DESCRIPTION_H
#define PARAMETER_DESCRIPTION_H

/// COMPONENT
#include <csapex_core/csapex_param_export.h>

/// SYSTEM
#include <string>

namespace csapex
{
namespace param
{
class CSAPEX_PARAM_EXPORT ParameterDescription
{
public:
    ParameterDescription(const std::string& toString);
    ParameterDescription();

    std::string toString() const;
    bool empty() const;

private:
    std::string description_;
};
}  // namespace param
}  // namespace csapex

#endif  // PARAMETER_DESCRIPTION_H
