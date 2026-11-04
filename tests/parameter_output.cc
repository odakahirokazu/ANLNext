#include "ParameterRegistry.hh"

#include <limits>
#include <sstream>
#include <stdexcept>
#include <tuple>

using namespace anlnext;
using Weights = std::map<std::string, double>;
using Record = std::tuple<int, double, std::string, bool, std::vector<int>, Weights, uintmax_t>;

void require(bool condition, const char* message)
{
  if (!condition) { throw std::runtime_error(message); }
}

struct Owner : ParameterRegistry {
  int kind = 7;
  double energy = 50.0;
  std::string label = "default";
  bool flag = true;
  std::vector<int> indices{2, 3};
  Weights weights{{"default", 1.5}};
  uintmax_t address = std::numeric_limits<uintmax_t>::max();
};

template<class T>
ModuleParam_sptr make_element(Owner& owner, const std::string& name, T Owner::* member, bool bound)
{
  if (bound) { return std::make_shared<ModuleParameterMember<Owner, T>>(name, &owner, member); }
  return std::make_shared<ModuleParameterUnique<T>>(name, owner.*member);
}

std::vector<ModuleParam_sptr> make_elements(Owner& owner, bool bound)
{
  auto elements = std::vector<ModuleParam_sptr>{
    make_element(owner, "kind", &Owner::kind, bound),
    make_element(owner, "energy", &Owner::energy, bound),
    make_element(owner, "label", &Owner::label, bound),
    make_element(owner, "flag", &Owner::flag, bound),
    make_element(owner, "indices", &Owner::indices, bound),
    make_element(owner, "weights", &Owner::weights, bound),
    make_element(owner, "address", &Owner::address, bound),
  };
  elements[1]->set_unit(10.0, "keV");
  elements[1]->set_description("energy description");
  return elements;
}

Record read_buffer(const std::vector<ModuleParam_sptr>& elements)
{
  Record value;
  elements[0]->__get__(&std::get<0>(value));
  elements[1]->__get__(&std::get<1>(value));
  elements[2]->__get__(&std::get<2>(value));
  elements[3]->__get__(&std::get<3>(value));
  elements[4]->__get__(&std::get<4>(value));
  elements[5]->__get__(&std::get<5>(value));
  elements[6]->__get__(&std::get<6>(value));
  return value;
}

void write_buffer(const std::vector<ModuleParam_sptr>& elements, const Record& value)
{
  elements[0]->__set__(&std::get<0>(value));
  elements[1]->__set__(&std::get<1>(value));
  elements[2]->__set__(&std::get<2>(value));
  elements[3]->__set__(&std::get<3>(value));
  elements[4]->__set__(&std::get<4>(value));
  elements[5]->__set__(&std::get<5>(value));
  elements[6]->__set__(&std::get<6>(value));
}

class FailingBuffer : public std::streambuf {
  int overflow(int value) override
  {
    if (++count_ > 40) { throw std::runtime_error("output failure"); }
    return value;
  }
  int count_ = 0;
};

void verify_output(VModuleParameter& parameter, const std::string& mode)
{
  if (mode == "json") {
    const auto tree = parameter.to_property_tree();
    for (const auto& row: tree.get_child("value")) {
      auto element = row.second.begin();
      require(element->second.get<int>("value") == 1, "JSON did not read the stored record");
      ++element;
      require(element->second.get<double>("value") == 12.5 &&
              element->second.get<double>("unit") == 10.0 &&
              element->second.get<std::string>("unit_name") == "keV", "JSON lost value or unit information");
      require(row.second.size() == 7, "JSON lost record elements");
    }
  }
  else if (mode == "text") {
    std::ostringstream output;
    parameter.output(output);
    require(output.str().find("energy: 12.5 [keV]") != std::string::npos,
            "text output lost value or unit information");
    require(output.str().find("stored") != std::string::npos, "text output did not read the stored record");
  }
  else {
    FailingBuffer buffer;
    std::ostream output(&buffer);
    output.exceptions(std::ios::badbit);
    bool caught = false;
    try { parameter.output(output); }
    catch (const std::exception&) { caught = true; }
    require(caught, "output did not fail as requested");
  }
}

template<class Container>
void verify(bool bound, bool initialized, const std::string& mode)
{
  Owner owner;
  const auto elements = make_elements(owner, bound);
  const Record defaults = read_buffer(elements);
  const Record stored{1, 125.0, "stored", false, {8, 9}, {{"stored", 3.5}}, 123};
  Container records;
  if constexpr (std::is_same_v<Container, std::vector<Record>>) { records.push_back(stored); }
  else { records.emplace("stored.key", stored); }
  ModuleParameter<Container> parameter("records", &records);
  for (const auto& element: elements) { parameter.add_value_element(element); }
  if (initialized) { parameter.set_value_element("kind", 42); }
  const Record editing{99, 750.0, "editing", true, {4, 5}, {{"editing", 6.5}}, 456};
  write_buffer(elements, editing);

  verify_output(parameter, mode);
  require(read_buffer(elements) == editing, "output changed the editing buffer");
  require(elements[1]->description() == "energy description", "output changed the definition");

  std::vector<ModuleParam_sptr> detached;
  if constexpr (std::is_same_v<Container, std::vector<Record>>) {
    detached = parameter.value_elements_from_container(std::size_t{0});
  }
  else { detached = parameter.value_elements_from_container(std::string("stored.key")); }
  require(read_buffer(detached) == stored, "read-only retrieval did not read the stored record");
  require(detached[1]->description() == elements[1]->description() &&
          detached[1]->unit() == elements[1]->unit(), "read-only retrieval lost definition information");
  detached[0]->set_value(200);
  auto detached_copy = detached[0]->clone();
  detached_copy->set_value(300);
  require(detached[0]->get_value(0) == 200, "copy shares the detached value");
  require(read_buffer(elements) == editing, "read-only retrieval shares the editing buffer");
  if (bound) {
    require(owner.kind == 99 && owner.label == "editing", "output changed the owning object");
  }

  parameter.set_map_key("editing");
  parameter.insert_to_container();
  parameter.set_map_key("default");
  parameter.insert_to_container();
  if constexpr (std::is_same_v<Container, std::vector<Record>>) {
    require(records[1] == editing, "output changed the inserted record");
    require(records[2] == (initialized ? defaults : editing), "output changed the default record");
  }
  else {
    require(records.at("editing") == editing, "output changed the inserted record");
    require(records.at("default") == (initialized ? defaults : editing), "output changed the default record");
  }
}

void verify_other_record_shapes()
{
  using Pair = std::pair<int, double>;
  std::map<std::string, Pair> pairs{{"row", {1, 125.0}}};
  ModuleParameter<decltype(pairs)> pair_parameter("pairs", &pairs);
  auto kind = std::make_shared<ModuleParameterUnique<int>>("kind", 99);
  auto energy = std::make_shared<ModuleParameterUnique<double>>("energy", 750.0);
  energy->set_unit(10.0, "keV");
  pair_parameter.add_value_element(kind);
  pair_parameter.add_value_element(energy);
  std::ostringstream output;
  pair_parameter.output(output);
  pair_parameter.to_property_tree();
  require(kind->get_value(0) == 99 && energy->get_value(0.0) == 75.0, "pair output changed the buffer");
  require(output.str().find("energy: 12.5 [keV]") != std::string::npos, "pair output changed its format");

  std::map<std::string, std::vector<int>> arrays{{"row", {1, 2}}};
  ModuleParameter<decltype(arrays)> array_parameter("arrays", &arrays);
  auto array = std::make_shared<ModuleParameterUnique<std::vector<int>>>("array", std::vector<int>{9});
  array_parameter.add_value_element(array);
  array_parameter.output(output);
  array_parameter.to_property_tree();
  require(array->get_value(std::vector<int>{}) == std::vector<int>{9}, "scalar record output changed the buffer");

  std::map<std::string, std::tuple<std::string, int>> strings{{"row", {"stored", 1}}};
  ModuleParameter<decltype(strings)> string_parameter("strings", &strings);
  auto label = std::make_shared<ModuleParameterUnique<std::string>>("label", std::string("editing"));
  string_parameter.add_value_element(label);
  string_parameter.add_value_element(kind);
  string_parameter.output(output);
  require(label->get_value(std::string{}) == "editing", "string-first record changed the buffer");

  std::map<std::string, std::tuple<int, std::string, std::string>> choices{
    {"row", {1, "enabled", "disabled"}}};
  ModuleParameter<decltype(choices)> choice_parameter("choices", &choices);
  choice_parameter.add_value_element(kind);
  choice_parameter.add_value_element(label);
  choice_parameter.add_value_element(std::make_shared<ModuleParameterUnique<std::string>>("other", "editing"));
  choice_parameter.enable_value_elements(1, {0, 1});
  std::ostringstream conditional;
  choice_parameter.output(conditional);
  require(conditional.str().find("label: enabled") != std::string::npos &&
          conditional.str().find("other:") == std::string::npos,
          "output used the buffer rather than the stored selector");
  require(choice_parameter.to_property_tree().get_child("value").front().second.size() == 3,
          "JSON omitted disabled elements");
  require(kind->get_value(0) == 99 && label->get_value(std::string{}) == "editing",
          "conditional output changed the buffer");

  std::vector<std::tuple<>> empty_records(1);
  ModuleParameter<decltype(empty_records)> empty_parameter("empty", &empty_records);
  empty_parameter.output(output);
  require(empty_parameter.to_property_tree().get_child("value").size() == 1, "empty tuple output changed");
}

void verify_detached_lifetime()
{
  const Record stored{1, 125.0, "stored", false, {8, 9}, {{"stored", 3.5}}, 123};
  std::vector<ModuleParam_sptr> detached;
  {
    Owner owner;
    std::vector<Record> records{stored};
    ModuleParameter<decltype(records)> parameter("records", &records);
    for (const auto& element: make_elements(owner, true)) { parameter.add_value_element(element); }
    detached = parameter.value_elements_from_container(std::size_t{0});
  }
  require(read_buffer(detached) == stored, "detached values depend on the original owner or container");
}

int main()
{
  try {
    for (bool bound: {false, true}) {
      for (bool initialized: {false, true}) {
        for (const std::string mode: {"json", "text", "failure"}) {
          verify<std::vector<Record>>(bound, initialized, mode);
          verify<std::map<std::string, Record>>(bound, initialized, mode);
        }
      }
    }
    verify_other_record_shapes();
    verify_detached_lifetime();
  }
  catch (const ANLException& error) { std::cerr << error.to_string() << '\n'; return 1; }
  catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
