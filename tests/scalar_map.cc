#include "ScalarMapExample.hh"
#include <limits>
#include <sstream>
#include <stdexcept>

using namespace anlnext;
void require(bool condition, const char* message)
{
  if (!condition) { throw std::runtime_error(message); }
}

template <typename K, typename V>
void verify(ScalarMapExample& module, const std::string& name, const std::map<K, V>& values)
{
  using Map = std::map<K, V>;
  module.set_parameter(name, values);
  const auto* param = module.get_parameter(name);
  require(param->get_value(Map{}) == values, "map setting/reading failed");
  require(param->size_of_container() == values.size(), "map size mismatch");
  auto copy = module.duplicate();
  copy->set_parameter(name, Map{});
  require(copy->get_parameter(name)->get_value(Map{}).empty(), "empty map did not replace values");
  require(param->get_value(Map{}) == values, "clone changed source map");
  const auto pt = param->to_property_tree();
  require(pt.get<std::string>("type") == param_type_info<Map>::name(), "type output mismatch");
  require(pt.get_child("value").size() == values.size(), "JSON map entry count mismatch");
  auto expected = values.begin();
  for (const auto& entry: pt.get_child("value")) {
    require(entry.second.get<K>("key") == expected->first &&
            entry.second.get<V>("value") == expected->second, "JSON key or value mismatch");
    ++expected;
  }
  Map buffer;
  ModuleParameter<Map> standalone("map", &buffer);
  standalone.set_unit(10.0, "test_unit");
  std::istringstream input(param->value_string());
  standalone.input(input);
  require(!input.fail() && standalone.get_value(Map{}) == values, "map text round trip failed");
  auto cloned_parameter = standalone.clone();
  require(cloned_parameter->get_value(Map{}) == values, "parameter clone lost map specialization");
  std::istringstream malformed("\"unterminated");
  standalone.input(malformed);
  require(malformed.fail() && standalone.get_value(Map{}) == values, "invalid input modified map");
  std::istringstream empty("  ");
  standalone.input(empty);
  require(!empty.fail() && buffer.empty(), "empty input did not clear map");
  standalone.set_value(values);
  standalone.clear_array();
  require(buffer.empty(), "clear_array failed");
  try { module.set_parameter(name, 1); }
  catch (const ParameterTypeError&) { return; }
  throw std::runtime_error("incorrect value type accepted");
}

int main()
{
  try {
    ScalarMapExample module;
    module.mod_define();
    verify(module, "map_i_i", std::map<int, int>{{-1, 3}, {std::numeric_limits<int>::max(), std::numeric_limits<int>::min()}});
    verify(module, "map_i_d", std::map<int, double>{{-1, 0.5}, {2, 1.5}});
    verify(module, "map_i_str", std::map<int, std::string>{{-1, ""}, {2, "a b\"c\\d"}});
    verify(module, "map_str_i", std::map<std::string, int>{{"", 3}, {"a.b", 5}});
    verify(module, "map_str_d", std::map<std::string, double>{{"a b", 0.5}, {"a.b", 1.5}});
    verify(module, "map_str_str", std::map<std::string, std::string>{{"", ""}, {"a b", "q\"\\r"}});
    require(module.map_i_d.at(-1) == 5.0 && module.map_str_d.at("a b") == 5.0, "unit conversion failed");
    std::map<int, int> raw{{1, 5}};
    ModuleParameter<std::map<int, int>> param("m", &raw);
    for (const auto* text: {"1", "2147483648 2", "1.5 2", "1 2 garbage", "1 2147483648"}) {
      std::istringstream input(text);
      param.input(input);
      require(input.fail() && raw.at(1) == 5, "invalid integer map input accepted");
    }
    bool rejected = false;
    try { param.add_value_element(std::make_shared<ModuleParameterUnique<int>>("number", 17)); }
    catch (const ParameterError&) { rejected = true; }
    require(rejected, "table-style registration must be rejected for scalar maps");
    std::istringstream duplicate("1 7 1 9");
    param.input(duplicate);
    require(raw.at(1) == 9 && raw.size() == 1, "repeated key must use last value");
  }
  catch (const ANLException& ex) { std::cerr << ex.to_string() << '\n'; return 1; }
  catch (const std::exception& ex) { std::cerr << ex.what() << '\n'; return 1; }
}
