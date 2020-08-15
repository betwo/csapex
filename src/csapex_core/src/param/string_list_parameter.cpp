/// HEADER
#include <csapex/param/string_list_parameter.h>

/// PROJECT
#include <csapex/param/register_parameter.h>
#include <csapex/serialization/io/std_io.h>
#include <csapex/utility/yaml.h>
#include <csapex/utility/any.h>

using namespace csapex;
using namespace param;

CSAPEX_REGISTER_PARAM(StringListParameter)

StringListParameter::StringListParameter() : ParameterImplementation("noname", ParameterDescription())
{
}

StringListParameter::StringListParameter(const std::string& name, const ParameterDescription& description) : ParameterImplementation(name, description)
{
}

StringListParameter::~StringListParameter()
{
}

const std::type_info& StringListParameter::type() const
{
    Lock l = lock();
    return typeid(list_);
}

std::string StringListParameter::toStringImpl() const
{
    std::stringstream v;

    for (std::size_t i = 0, n = list_.size(); i < n; ++i) {
        if (i > 0) {
            v << ", ";
        }
        v << list_[i];
    }

    return std::string("[string_list: ") + v.str() + "]";
}

void StringListParameter::get_unsafe(std::any& out) const
{
    out = list_;
}

bool StringListParameter::set_unsafe(const std::any& v)
{
    auto l = std::any_cast<std::vector<std::string> >(v);
    if (list_ != l) {
        list_ = l;
        return true;
    }

    return false;
}

bool StringListParameter::cloneDataFrom(const Clonable& other)
{
    if (const StringListParameter* list = dynamic_cast<const StringListParameter*>(&other)) {
        *this = *list;
        triggerChange();
        return true;

    } else {
        throw std::runtime_error("bad setFrom, invalid types");
    }

    return false;
}

void StringListParameter::doSerialize(YAML::Node& n) const
{
    n["list"] = list_;
}

void StringListParameter::doDeserialize(const YAML::Node& n)
{
    if (n["list"].IsDefined()) {
        list_ = n["list"].as<std::vector<std::string> >();
    }
}

void StringListParameter::add(const std::string& value)
{
    list_.push_back(value);
}

void StringListParameter::setAt(std::size_t i, const std::string& value)
{
    list_.at(i) = value;
}

void StringListParameter::remove(std::size_t i)
{
    list_.erase(list_.begin() + i);
}

void StringListParameter::removeAll(const std::string& value)
{
    list_.erase(std::remove(list_.begin(), list_.end(), value), list_.end());
}

std::size_t StringListParameter::count() const
{
    return list_.size();
}
std::vector<std::string> StringListParameter::getValues() const
{
    return list_;
}

void StringListParameter::serialize(SerializationBuffer& data, SemanticVersion& version) const
{
    Parameter::serialize(data, version);

    data << list_;
}

void StringListParameter::deserialize(const SerializationBuffer& data, const SemanticVersion& version)
{
    Parameter::deserialize(data, version);

    data >> list_;
}
