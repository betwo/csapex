/// HEADER
#include <csapex/model/generic_state.h>

/// PROJECT
#include <csapex/param/io.h>
#include <csapex/param/parameter_factory.h>
#include <csapex/utility/assert.h>
#include <csapex/utility/uuid_provider.h>
#include <csapex/serialization/io/std_io.h>

/// SYSTEM
#include <regex>

using namespace csapex;

GenericState::GenericState() : silent_(false)
{
}

GenericState::GenericState(const GenericState& copy) : GenericState()
{
    operator=(copy);
}

GenericState::GenericState(GenericState&& move) : GenericState()
{
    params = std::move(move.params);
    param_valid_name_cache = std::move(move.param_valid_name_cache);
    temporary = std::move(move.temporary);
    cached_parameter = std::move(move.cached_parameter);
    persistent = std::move(move.persistent);
    legacy = std::move(move.legacy);
    order = std::move(move.order);
}

void GenericState::operator=(const GenericState& copy)
{
    params = copy.params;
    param_valid_name_cache = copy.param_valid_name_cache;
    temporary = copy.temporary;
    cached_parameter = copy.cached_parameter;
    persistent = copy.persistent;
    legacy = copy.legacy;
    order = copy.order;
}

void GenericState::setParentUUID(const UUID& parent_uuid)
{
    apex_assert_hard(!parent_uuid.composite());

    parent_uuid_ = parent_uuid;

    for (std::map<std::string, csapex::param::Parameter::Ptr>::const_iterator it = params.begin(); it != params.end(); ++it) {
        it->second->setUUID(parent_uuid_);
    }
}

ClonablePtr GenericState::makeEmptyInstance() const
{
    return Ptr{ new GenericState };
}

bool GenericState::cloneDataFrom(const Clonable& other)
{
    if (const GenericState* other_state = dynamic_cast<const GenericState*>(&other)) {
        setFrom(*other_state);
        return true;
    }

    return false;
}

void GenericState::writeYaml(YAML::Node& out) const
{
    out["params"] = params;

    std::vector<std::string> persistent_v(persistent.begin(), persistent.end());
    out["persistent_params"] = persistent_v;
}

void GenericState::readYaml(const YAML::Node& node)
{
    if (node["params"].IsDefined()) {
        auto serialized_params = node["params"].as<std::map<std::string, csapex::param::Parameter::Ptr> >();
        for (auto pair : serialized_params) {
            if(pair.first != pair.second->name()) {
                // backwards compatibility: if the key of a parameter is not equal to its name, rename the parameter
                pair.second->setName(pair.first);
            }
            auto pos = params.find(pair.first);
            if (pos == params.end()) {
                params[pair.first] = pair.second;
                legacy_parameter_added(params[pair.first]);
            } else {
                param::ParameterPtr p = pos->second;
                p->cloneDataFrom(*pair.second);
            }
            legacy.insert(pair.first);
        }
    }
    if (node["persistent_params"].IsDefined()) {
        std::vector<std::string> persistent_v = node["persistent_params"].as<std::vector<std::string> >();
        persistent.clear();
        persistent.insert(persistent_v.begin(), persistent_v.end());
    }
}

void GenericState::serialize(SerializationBuffer& data, SemanticVersion& version) const
{
    version = { 0, 0, 0 };

    data << params;
    data << persistent;
}
void GenericState::deserialize(const SerializationBuffer& data, const SemanticVersion& version)
{
    data >> params;
    data >> persistent;
}

void GenericState::initializePersistentParameters()
{
    for (const std::string& name : persistent) {
        param::ParameterPtr param = params[name];
        param->setUUID(UUIDProvider::makeTypedUUID_forced(parent_uuid_, "param", param->name()));
        (parameter_added)(param);
    }
}

void GenericState::addParameter(csapex::param::Parameter::Ptr param)
{
    std::string param_name = param->name();

    apex_assert_hard(param_name != "noname");
    auto legacy_pos = legacy.find(param_name);
    auto param_pos = params.find(param_name);
    if (param_pos != params.end()) {
        if (legacy_pos == legacy.end()) {
            throw std::logic_error(std::string("a parameter with the name ") + param_name + " has already been added.");
        }
        param->cloneDataFrom(*param_pos->second);
    }
    registerParameter(param);

    if (legacy_pos != legacy.end()) {
        legacy.erase(legacy_pos);
    }
    if (std::find(order.begin(), order.end(), param_name) == order.end()) {
        order.push_back(param_name);
    }

    std::string valid_name = param_name;

    std::regex to_remove("~");
    valid_name = std::regex_replace(valid_name, to_remove, std::string(""));

    // generate a valid name, valid characters are a-z, A-Z, 0-9, / and _.
    std::regex invalid("[^0-9a-zA-Z/_]");
    if (std::regex_search(valid_name, invalid)) {
        valid_name = std::regex_replace(valid_name, invalid, std::string("_"));
    }

    param_valid_name_cache[valid_name] = param_name;

    param->parameter_changed.connect(parameter_changed);
}

void GenericState::removeParameter(csapex::param::ParameterPtr param)
{
    params.erase(param->name());

    auto pos = std::find(order.begin(), order.end(), param->name());
    if (pos != order.end()) {
        order.erase(pos);
    }

    unregisterParameter(param);
}

void GenericState::setParameterSetSilence(bool silent)
{
    silent_ = silent;
}

void GenericState::triggerParameterSetChanged()
{
    if (!silent_) {
        (parameter_set_changed)();
    }
}

void GenericState::addTemporaryParameter(const csapex::param::Parameter::Ptr& param)
{
    param->setTemporary(true);

    // check if there is an old version of the parameter
    csapex::param::Parameter::Ptr entry = param;
    std::string name = param->name();
    auto param_pos = params.find(name);
    if (param_pos != params.end()) {
        // the existing parameter should be temporary or legacy
        if (temporary.find(name) == temporary.end() && cached_parameter.find(name) == cached_parameter.end() && legacy.find(name) == legacy.end()) {
            throw std::runtime_error("trying to add a temporary parameter with the name "
                                     "of an existing parameter '" +
                                     name + "'");
        }
        param::Parameter::Ptr p = param_pos->second;
        entry->cloneDataFrom(*p);
        removeParameter(p);
    }
    temporary[entry->name()] = true;
    addParameter(entry);
}

void GenericState::removeTemporaryParameter(const csapex::param::Parameter::Ptr& param)
{
    removeParameter(param);

    auto pos = temporary.find(param->name());
    if (pos != temporary.end()) {
        temporary.erase(pos);
    }
}

void GenericState::removeTemporaryParameters()
{
    for (auto it = temporary.begin(); it != temporary.end(); ++it) {
        std::string name(it->first);
        csapex::param::Parameter::Ptr p = getParameter(name);

        // don't erase the param itself, remember the value for future!
        // don't -> params.erase(params.find(name));
        order.erase(std::find(order.begin(), order.end(), name));

        cached_parameter[name] = true;

        (parameter_removed)(p);
    }

    temporary.clear();

    triggerParameterSetChanged();
}

void GenericState::removePersistentParameter(const csapex::param::Parameter::Ptr& param)
{
    removeParameter(param);

    auto pos = persistent.find(param->name());
    if (pos != persistent.end()) {
        persistent.erase(pos);
    }
}

void GenericState::removePersistentParameters()
{
    for (const std::string& name : persistent) {
        csapex::param::Parameter::Ptr p = getParameter(name);

        removePersistentParameter(p);

        (parameter_removed)(p);
    }

    apex_assert_hard(persistent.empty());

    triggerParameterSetChanged();
}

void GenericState::addPersistentParameter(const csapex::param::Parameter::Ptr& param)
{
    persistent.insert(param->name());

    registerParameter(param);
}

void GenericState::registerParameter(const csapex::param::Parameter::Ptr& param)
{
    params[param->name()] = param;

    param->setUUID(UUIDProvider::makeTypedUUID_forced(parent_uuid_, "param", param->name()));

    (parameter_added)(param);
    triggerParameterSetChanged();
}

void GenericState::unregisterParameter(const csapex::param::Parameter::Ptr& param)
{
    params.erase(param->name());

    (parameter_removed)(param);
    triggerParameterSetChanged();
}

void GenericState::setFrom(const GenericState& rhs)
{
    persistent = rhs.persistent;

    for (std::map<std::string, csapex::param::Parameter::Ptr>::const_iterator it = rhs.params.begin(); it != rhs.params.end(); ++it) {
        csapex::param::Parameter::Ptr p = it->second;
        std::string name = p->name();
        if (params.find(name) != params.end()) {
            params[name]->cloneDataFrom(*p);
        } else {
            params[name] = csapex::param::factory::clone(p);
            legacy.insert(name);
        }
    }

    initializePersistentParameters();
}

csapex::param::Parameter& GenericState::operator[](const std::string& name) const
{
    try {
        return *params.at(name);
    } catch (const std::exception& e) {
        throw std::runtime_error("cannot get parameter '" + name + "', doesn't exist: " + e.what());
    }
}

csapex::param::Parameter::Ptr GenericState::getParameter(const std::string& name) const
{
    try {
        return params.at(name);
    } catch (const std::exception& e) {
        throw std::runtime_error("cannot get parameter '" + name + "', doesn't exist: " + e.what());
    }
}
csapex::param::Parameter::Ptr GenericState::getMappedParameter(const std::string& name) const
{
    try {
        auto pos = param_valid_name_cache.find(name);
        return params.at(pos->second);
    } catch (const std::exception& e) {
        throw std::runtime_error("cannot get parameter '" + name + "', doesn't exist: " + e.what());
    }
}

std::vector<csapex::param::Parameter::Ptr> GenericState::getParameters() const
{
    std::vector<csapex::param::Parameter::Ptr> result;
    for (std::vector<std::string>::const_iterator n = order.begin(); n != order.end(); ++n) {
        result.push_back(params.at(*n));
    }
    for (const std::string& p : persistent) {
        result.push_back(params.at(p));
    }

    return result;
}

std::vector<csapex::param::Parameter::Ptr> GenericState::getTemporaryParameters() const
{
    std::vector<csapex::param::Parameter::Ptr> result;
    for (const auto& pair : temporary) {
        auto pos = params.find(pair.first);
        if (pos != params.end()) {
            result.push_back(pos->second);
        }
    }

    return result;
}

std::vector<csapex::param::Parameter::Ptr> GenericState::getPersistentParameters() const
{
    std::vector<csapex::param::Parameter::Ptr> result;
    for (const auto& name : persistent) {
        auto pos = params.find(name);
        if (pos != params.end()) {
            result.push_back(pos->second);
        }
    }

    return result;
}

std::size_t GenericState::getParameterCount() const
{
    return getParameters().size();
}

bool GenericState::hasParameter(const std::string& name) const
{
    auto pos = param_valid_name_cache.find(name);
    if (pos == param_valid_name_cache.end()) {
        auto persistent_pos = persistent.find(name);
        if (persistent_pos != persistent.end()) {
            return true;
        }

        return false;
    }

    return params.find(pos->second) != params.end();
}

template <typename T>
T GenericState::readParameter(const std::string& name) const
{
    try {
        return getParameter(name)->as<T>();
    } catch (const std::out_of_range& e) {
        throw std::runtime_error(std::string("unknown parameter '") + name + "'");
    }
}

template CSAPEX_CORE_EXPORT bool GenericState::readParameter<bool>(const std::string& name) const;
template CSAPEX_CORE_EXPORT double GenericState::readParameter<double>(const std::string& name) const;
template CSAPEX_CORE_EXPORT int GenericState::readParameter<int>(const std::string& name) const;
template CSAPEX_CORE_EXPORT std::string GenericState::readParameter<std::string>(const std::string& name) const;
template CSAPEX_CORE_EXPORT std::pair<int, int> GenericState::readParameter<std::pair<int, int> >(const std::string& name) const;
template CSAPEX_CORE_EXPORT std::pair<double, double> GenericState::readParameter<std::pair<double, double> >(const std::string& name) const;
template CSAPEX_CORE_EXPORT std::vector<int> GenericState::readParameter<std::vector<int> >(const std::string& name) const;
template CSAPEX_CORE_EXPORT std::vector<double> GenericState::readParameter<std::vector<double> >(const std::string& name) const;

/// YAML
namespace YAML
{
Node convert<csapex::GenericState>::encode(const csapex::GenericState& rhs)
{
    Node n;
    rhs.writeYaml(n);
    return n;
}

bool convert<csapex::GenericState>::decode(const Node& node, csapex::GenericState& rhs)
{
    rhs.readYaml(node);
    return true;
}
}  // namespace YAML
