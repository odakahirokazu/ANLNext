#include "BasicModule.hh"

#include <iostream>
#include <map>
#include <stdexcept>
#include <tuple>
#include <vector>

using namespace anlnext;

class DefaultsModule : public BasicModule {
  DEFINE_ANL_MODULE(DefaultsModule, 1.0);
  ENABLE_PARALLEL_RUN();
public:
  void setup()
  {
    define_parameter("table", &DefaultsModule::table);
    add_value_element("number", 17);
    define_parameter("rows", &DefaultsModule::rows);
    add_value_element("number", 17);
    add_value_element("label", std::string("default"));
  }

  void insert(const std::string& name, const std::string& key)
  {
    expose_parameter(name);
    if (name == "table") { set_map_key(key); }
    insert_to_container();
  }

  std::map<std::string, std::tuple<int>> table;
  std::vector<std::tuple<int, std::string>> rows;
};

void require(bool condition, const char* message)
{
  if (!condition) { throw std::runtime_error(message); }
}

void verify(DefaultsModule& module, const std::string& name,
            const std::string& first_key, int first_value)
{
  module.insert(name, first_key);
  module.insert(name, first_key + "_default");
  if (name == "table") {
    require(std::get<0>(module.table.at(first_key)) == first_value, "map input was lost");
    require(std::get<0>(module.table.at(first_key + "_default")) == 17, "map default was lost");
  }
  else {
    const auto n = module.rows.size();
    require(std::get<0>(module.rows[n-2]) == first_value, "vector input was lost");
    require(std::get<0>(module.rows[n-1]) == 17, "vector default was lost");
    require(std::get<1>(module.rows[n-2]) == "default" &&
            std::get<1>(module.rows[n-1]) == "default", "unspecified field default was lost");
  }
}

void run(const std::string& name, bool initialized, bool direct)
{
  DefaultsModule source;
  source.setup();
  source.expose_parameter(name);
  if (initialized) { source.set_value_element("number", 23); }
  auto copy = source.duplicate();
  auto& cloned = dynamic_cast<DefaultsModule&>(*copy);
  cloned.expose_parameter(name);
  if (!direct) { cloned.set_value_element("number", 31); }
  verify(cloned, name, "copy", direct ? (initialized ? 23 : 17) : 31);
  require(source.table.empty() && source.rows.empty(), "copy modified source data");
  verify(source, name, "source", initialized ? 23 : 17);

  auto later_copy = source.duplicate();
  auto& later = dynamic_cast<DefaultsModule&>(*later_copy);
  verify(later, name, "later", 17);
  require(source.table.size() + source.rows.size() == 2, "later copy modified source");
}

int main()
{
  try {
    for (const std::string name: {"table", "rows"}) {
      for (bool initialized: {false, true}) {
        for (bool direct: {false, true}) { run(name, initialized, direct); }
      }
    }
  }
  catch (const ANLException& ex) {
    std::cerr << ex.to_string() << '\n';
    return 1;
  }
  catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
