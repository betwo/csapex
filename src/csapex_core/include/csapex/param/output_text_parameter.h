#ifndef OUTPUT_TEXT_PARAMETER_H
#define OUTPUT_TEXT_PARAMETER_H

/// COMPONENT
#include <csapex/param/parameter_impl.hpp>
#include <csapex_core/csapex_param_export.h>

namespace csapex
{
namespace param
{
class CSAPEX_PARAM_EXPORT OutputTextParameter : public ParameterImplementation<OutputTextParameter>
{
public:
    typedef std::shared_ptr<OutputTextParameter> Ptr;

public:
    OutputTextParameter();
    explicit OutputTextParameter(const std::string& name, const ParameterDescription& description);
    virtual ~OutputTextParameter();

    virtual const std::type_info& type() const override;

    virtual void serialize(SerializationBuffer& data, SemanticVersion& version) const override;
    virtual void deserialize(const SerializationBuffer& data, const SemanticVersion& version) override;

protected:
    virtual void get_unsafe(boost::any& out) const override;
    virtual bool set_unsafe(const boost::any& v) override;
    virtual std::string toStringImpl() const override;

    virtual void doSerialize(YAML::Node& n) const override;
    virtual void doDeserialize(const YAML::Node& n) override;

    virtual bool cloneDataFrom(const Clonable& other) override;

private:
    std::string text_;
};

template <>
inline std::string serializationName<OutputTextParameter>()
{
    return "outtext";
}

}  // namespace param
}  // namespace csapex
#endif  // OUTPUT_TEXT_PARAMETER_H
