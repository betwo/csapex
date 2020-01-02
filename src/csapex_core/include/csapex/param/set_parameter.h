#ifndef SET_PARAMETER_H
#define SET_PARAMETER_H

/// COMPONENT
#include <csapex/param/parameter_impl.hpp>
#include <csapex_core/csapex_param_export.h>

/// SYSTEM
#include <vector>
#include <boost/any.hpp>

namespace csapex
{
namespace param
{
class CSAPEX_PARAM_EXPORT SetParameter : public ParameterImplementation<SetParameter>
{
public:
    typedef std::shared_ptr<SetParameter> Ptr;

public:
    SetParameter();
    explicit SetParameter(const std::string& name, const ParameterDescription& description);

    template <typename T>
    SetParameter(const std::string& name, const ParameterDescription& description, T def) : SetParameter(name, description)
    {
        def_ = def;
    }

    virtual ~SetParameter();

    const std::type_info& type() const override;
    std::string toStringImpl() const override;

    bool cloneDataFrom(const Clonable& other) override;

    void doSerialize(YAML::Node& e) const override;
    void doDeserialize(const YAML::Node& n) override;

    void serialize(SerializationBuffer& data, SemanticVersion& version) const override;
    void deserialize(const SerializationBuffer& data, const SemanticVersion& version) override;

    bool accepts(const std::type_info& type) const override;

    std::string defText() const;

    template <typename T>
    void setSet(const std::vector<std::pair<std::string, T> >& set)
    {
        set_.clear();
        for (typename std::vector<std::pair<std::string, T> >::const_iterator it = set.begin(); it != set.end(); ++it) {
            set_[it->first] = it->second;
        }
    }

    template <typename T>
    void setSet(const std::map<std::string, T>& set)
    {
        set_.clear();
        for (typename std::map<std::string, T>::const_iterator it = set.begin(); it != set.end(); ++it) {
            set_[it->first] = it->second;
        }
        scope_changed(this);
    }

    void setSet(const std::vector<std::string>& set);
    std::vector<std::string> getSetTexts() const;

    void setByName(const std::string& name);

    std::string getText(int idx) const;
    std::string getText() const;

    template <typename T>
    bool contains(const T& value);
    bool contains(const boost::any& value);

    int noParameters() const;

protected:
    void get_unsafe(boost::any& out) const override;
    bool set_unsafe(const boost::any& v) override;

    template <typename T>
    void doSerializeImplementation(const std::string& type_name, YAML::Node& e) const;
    template <typename T>
    void doDeserializeImplementation(const std::string& type_name, const YAML::Node& n);

    std::string convertToString(const boost::any& v) const;

private:
    template <typename T>
    T read(const boost::any& var) const
    {
        try {
            return boost::any_cast<T>(var);

        } catch (const boost::bad_any_cast& e) {
            throw std::logic_error(std::string("typeof SetParameter is not ") + typeid(T).name() + ": " + e.what());
        }
    }

private:
    boost::any value_;
    std::string txt_;
    std::map<std::string, boost::any> set_;
    boost::any def_;
};

template <>
inline std::string serializationName<SetParameter>()
{
    return "set";
}

}  // namespace param
}  // namespace csapex

#endif  // SET_PARAMETER_H
