#ifndef INTERVAL_PARAMETER_H
#define INTERVAL_PARAMETER_H

/// COMPONENT
#include <csapex/param/parameter_impl.hpp>
#include <csapex_core/csapex_param_export.h>

/// SYSTEM
#include <boost/mpl/vector.hpp>
#include <boost/mpl/contains.hpp>
#include <any>

namespace csapex
{
namespace param
{
typedef boost::mpl::vector<double, int> IntervalParameterTypes;

class CSAPEX_PARAM_EXPORT IntervalParameter : public ParameterImplementation<IntervalParameter>
{
public:
    typedef std::shared_ptr<IntervalParameter> Ptr;

public:
    slim_signal::Signal<void(Parameter*)> step_changed;

public:
    IntervalParameter();
    explicit IntervalParameter(const std::string& name, const ParameterDescription& description);

    template <typename T>
    IntervalParameter(const std::string& name, const ParameterDescription& description, std::pair<T, T> def_value, T min, T max, T step) : IntervalParameter(name, description)
    {
        def_ = def_value;
        values_ = def_value;
        min_ = min;
        max_ = max;
        step_ = step;
    }

    virtual ~IntervalParameter();

    IntervalParameter& operator=(const IntervalParameter& p);

    bool accepts(const std::type_info& type) const override;

    const std::type_info& type() const override;
    std::string toStringImpl() const override;

    bool cloneDataFrom(const Clonable& other) override;

    void doSerialize(YAML::Node& e) const override;
    void doDeserialize(const YAML::Node& n) override;

    void serialize(SerializationBuffer& data, SemanticVersion& version) const override;
    void deserialize(const SerializationBuffer& data, const SemanticVersion& version) override;

    template <typename T>
    void setLower(T v)
    {
        Lock l = lock();
        values_.first = v;
        triggerChange();
    }
    template <typename T>
    void setUpper(T v)
    {
        Lock l = lock();
        values_.second = v;
        triggerChange();
    }

    template <typename T>
    T lower() const
    {
        return read<T>(values_.first);
    }

    template <typename T>
    T upper() const
    {
        return read<T>(values_.second);
    }

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
    std::pair<T, T> def() const
    {
        return std::make_pair(read<T>(def_.first), read<T>(def_.second));
    }

    template <typename T>
    T step() const
    {
        return read<T>(step_);
    }

    template <typename T>
    void setInterval(T min, T max)
    {
        Lock l = lock();
        if (min != read<T>(min_) || max != read<T>(max_)) {
            min_ = min;
            max_ = max;
            scope_changed(this);
        }
    }

    template <typename T>
    void setMin(T min)
    {
        if (min != read<T>(min_)) {
            Lock l = lock();
            min_ = min;
            scope_changed(this);
        }
    }

    template <typename T>
    void setMax(T max)
    {
        if (read<T>(max_) != max) {
            Lock l = lock();
            max_ = max;
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
    void get_unsafe(std::any& out) const override;
    bool set_unsafe(const std::any& v) override;

private:
    template <typename T>
    T read(const std::any& var) const
    {
        static_assert(boost::mpl::contains<IntervalParameterTypes, T>::value);
        try {
            Lock l = lock();
            return std::any_cast<T>(var);

        } catch (const std::bad_any_cast& e) {
            throw std::logic_error(std::string("typeof IntervalParameter is not ") + typeid(T).name() + ": " + e.what());
        }
    }

private:
    std::pair<std::any, std::any> values_;
    std::any min_;
    std::any max_;
    std::pair<std::any, std::any> def_;
    std::any step_;
};

template <>
inline std::string serializationName<IntervalParameter>()
{
    return "interval";
}

}  // namespace param
}  // namespace csapex

#endif  // INTERVAL_PARAMETER_H
