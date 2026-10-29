#include "ModuleParameter.hh"

namespace anlnext
{
namespace
{
template <typename T>
bool read_map_element(std::istream& is, T& value)
{
  if constexpr (std::is_same<T, std::string>::value) {
    return static_cast<bool>(is >> std::quoted(value));
  }
  else {
    std::string token;
    if (!(is >> token)) { return false; }
    std::istringstream input(token);
    if (!(input >> value)) {
      is.setstate(std::ios::failbit);
      return false;
    }
    input >> std::ws;
    if (!input.eof()) {
      is.setstate(std::ios::failbit);
      return false;
    }
    return true;
  }
}

template <typename T>
void write_map_element(std::ostream& os, const T& value)
{
  if constexpr (std::is_same<T, std::string>::value) { os << std::quoted(value); }
  else { os << value; }
}
}

template <typename K, typename V>
ModuleParameterMap<K, V>::ModuleParameterMap(const std::string& name, map_type* ptr)
  : VModuleParameter(name), ptr_(ptr)
{}

template <typename K, typename V>
std::string ModuleParameterMap<K, V>::type_name() const
{ return param_type_info<map_type>::name(); }

template <typename K, typename V>
void ModuleParameterMap<K, V>::set_value(const map_type& value)
{
  map_type converted = value;
  if constexpr (std::is_same<V, double>::value) {
    for (auto& entry: converted) { entry.second *= unit(); }
  }
  __ref__() = std::move(converted);
}

template <typename K, typename V>
std::map<K, V> ModuleParameterMap<K, V>::get_value(const map_type&) const
{
  map_type converted = __ref__();
  if constexpr (std::is_same<V, double>::value) {
    for (auto& entry: converted) { entry.second /= unit(); }
  }
  return converted;
}

template <typename K, typename V>
void ModuleParameterMap<K, V>::add_value_element(ModuleParam_sptr)
{
  BOOST_THROW_EXCEPTION(ParameterError(this, "Map parameters do not have value elements."));
}

template <typename K, typename V>
void ModuleParameterMap<K, V>::clear_array() { __ref__().clear(); }

template <typename K, typename V>
void ModuleParameterMap<K, V>::clear_container() { __ref__().clear(); }

template <typename K, typename V>
std::size_t ModuleParameterMap<K, V>::size_of_container() const { return __ref__().size(); }

template <typename K, typename V>
void ModuleParameterMap<K, V>::output(std::ostream& os) const
{
  bool first = true;
  for (const auto& entry: get_value(map_type{})) {
    if (!first) { os << ' '; }
    first = false;
    write_map_element(os, entry.first);
    os << ' ';
    write_map_element(os, entry.second);
  }
}

template <typename K, typename V>
void ModuleParameterMap<K, V>::input(std::istream& is)
{
  if (is.fail()) { return; }
  map_type value;
  while (true) {
    is >> std::ws;
    if (is.eof()) { break; }
    K key{};
    V element{};
    if (!read_map_element(is, key) || !read_map_element(is, element)) { return; }
    value.insert_or_assign(key, element);
    if (is.eof()) { break; }
  }
  if (is.bad()) { return; }
  is.clear();
  set_value(value);
}

template <typename K, typename V>
void ModuleParameterMap<K, V>::__get__(void* ptr) const
{ *static_cast<map_type*>(ptr) = __ref__(); }

template <typename K, typename V>
void ModuleParameterMap<K, V>::__set__(const void* ptr)
{ __ref__() = *static_cast<const map_type*>(ptr); }

template <typename K, typename V>
std::map<K, V>& ModuleParameterMap<K, V>::__ref__() { return *ptr_; }

template <typename K, typename V>
const std::map<K, V>& ModuleParameterMap<K, V>::__ref__() const { return *ptr_; }

template <typename K, typename V>
bool ModuleParameterMap<K, V>::ask()
{
  std::cout << "  key value pairs... (quote strings containing spaces)\n";
  return ask_base();
}

template <typename K, typename V>
boost::property_tree::ptree ModuleParameterMap<K, V>::to_property_tree() const
{
  boost::property_tree::ptree pt;
  pt.put("name", name());
  pt.put("type", type_name());
  if constexpr (std::is_same<V, double>::value) {
    pt.put("unit", unit());
    pt.put("unit_name", unit_name());
  }
  boost::property_tree::ptree entries;
  for (const auto& entry: get_value(map_type{})) {
    boost::property_tree::ptree element;
    element.put("key", entry.first);
    element.put("value", entry.second);
    entries.push_back(std::make_pair("", std::move(element)));
  }
  pt.add_child("value", std::move(entries));
  return pt;
}

template class ModuleParameterMap<int, int>;

std::shared_ptr<VModuleParameter> ModuleParameter<std::map<int, int>>::clone()
{ return std::shared_ptr<VModuleParameter>(new ModuleParameter(*this)); }

template class ModuleParameterMap<int, double>;

std::shared_ptr<VModuleParameter> ModuleParameter<std::map<int, double>>::clone()
{ return std::shared_ptr<VModuleParameter>(new ModuleParameter(*this)); }

template class ModuleParameterMap<int, std::string>;

std::shared_ptr<VModuleParameter> ModuleParameter<std::map<int, std::string>>::clone()
{ return std::shared_ptr<VModuleParameter>(new ModuleParameter(*this)); }

template class ModuleParameterMap<std::string, int>;

std::shared_ptr<VModuleParameter> ModuleParameter<std::map<std::string, int>>::clone()
{ return std::shared_ptr<VModuleParameter>(new ModuleParameter(*this)); }

template class ModuleParameterMap<std::string, double>;

std::shared_ptr<VModuleParameter> ModuleParameter<std::map<std::string, double>>::clone()
{ return std::shared_ptr<VModuleParameter>(new ModuleParameter(*this)); }

template class ModuleParameterMap<std::string, std::string>;

std::shared_ptr<VModuleParameter> ModuleParameter<std::map<std::string, std::string>>::clone()
{ return std::shared_ptr<VModuleParameter>(new ModuleParameter(*this)); }

} /* namespace anlnext */
