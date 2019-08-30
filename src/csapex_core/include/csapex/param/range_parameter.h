#ifndef RANGE_PARAMETER_H
#define RANGE_PARAMETER_H

/// COMPONENT
#include <csapex/param/parameter_impl.hpp>
#include <csapex_core/csapex_param_export.h>

/// SYSTEM
#include <boost/mpl/vector.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/any.hpp>

namespace csapex
{
namespace param
{
typedef boost::mpl::vector<double, int> RangeParameterTypes;

namespace range
{
template <typename T>
T limitStep(const T min, const T max, const T step)
{
    return step;
}

template <>
double limitStep(const double min, const double max, const double step);
template <>
int limitStep(const int min, const int max, const int step);
}  // namespace range

class CSAPEX_PARAM_EXPORT RangeParameter : public ParameterImplementation<RangeParameter>
{
public:
    typedef std::shared_ptr<RangeParameter> Ptr;

public:
    slim_signal::Signal<void(Parameter*)> step_changed;

public:
    RangeParameter();
    explicit RangeParameter(const std::string& name, const ParameterDescription& description);

    template <typename T>
    RangeParameter(const std::string& name, const ParameterDescription& description, T def_value, T def_min, T def_max, T step) : RangeParameter(name, description)
    {
        def_value_ = def_value;
        def_min_ = def_min;
        max_ = def_max;
        min_ = def_min;
        def_max_ = def_max;
        step_ = step;
    }

    virtual ~RangeParameter();

    RangeParameter& operator=(const RangeParameter& p);

    virtual const std::type_info& type() const override;
    virtual std::string toStringImpl() const override;

    bool cloneDataFrom(const Clonable& other) override;

    void doSerialize(YAML::Node& e) const override;
    void doDeserialize(const YAML::Node& n) override;

    virtual void serialize(SerializationBuffer& data, SemanticVersion& version) const override;
    virtual void deserialize(const SerializationBuffer& data, const SemanticVersion& version) override;

    template <typename T>
    T min() const
    {
        return read<T>(min_);
    }

    template <typename T>
    T max() const
    {
        return read<T>(max_);
    }

    template <typename T>
    T def() const
    {
        return read<T>(def_value_);
    }

    template <typename T>
    T step() const
    {
        return read<T>(step_);
    }

    template <typename T>
    void setInterval(T min, T max)
    {
        if (min != read<T>(min_) || max != read<T>(max_)) {
            min_ = min;
            max_ = max;
            scope_changed(this);
        }
    }

    template <typename T>
    void setMin(T min)
    {
        T _min = read<T>(min_);
        if (min != _min) {
            min_ = min;
            T _max = read<T>(max_);
            if (_max < min) {
                max_ = min;
            }
            scope_changed(this);
        }
    }

    template <typename T>
    void setMax(T max)
    {
        T _max = read<T>(max_);
        if (_max != max) {
            max_ = max;
            T _min = read<T>(min_);
            if (max < _min) {
                min_ = max;
            }
            scope_changed(this);
        }
    }

    template <typename T>
    void setStep(T step)
    {
        T _step = read<T>(step_);
        if (_step != step) {
            // test, if difference between max and min is bigger than step
            T _max = read<T>(max_);
            T _min = read<T>(min_);
            if (((_min + step) < _max) && ((_min - step) < _max)) {
                step_ = step;
            } else {
                step_ = _max - _min;
            }

            step_changed(this);
        }
    }

protected:
    virtual void get_unsafe(boost::any& out) const override;
    virtual bool set_unsafe(const boost::any& v) override;

    RangeParameter(const RangeParameter& other) = default;

private:
    template <typename T>
    T read(const boost::any& var) const
    {
        BOOST_STATIC_ASSERT((boost::mpl::contains<RangeParameterTypes, T>::value));
        try {
            return boost::any_cast<T>(var);

        } catch (const boost::bad_any_cast& e) {
            throw std::logic_error(std::string("typeof RangeParameter is not ") + typeid(T).name() + ": " + e.what());
        }
    }

private:
    boost::any value_;
    boost::any min_;
    boost::any max_;
    boost::any def_value_;
    boost::any def_min_;
    boost::any def_max_;
    boost::any step_;
};

template <>
inline std::string serializationName<RangeParameter>()
{
    return "range";
}

}  // namespace param
}  // namespace csapex

#endif  // RANGE_PARAMETER_H
