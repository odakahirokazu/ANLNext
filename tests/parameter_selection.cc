#include "BasicModule.hh"
#include "BasicSubModule.hh"

#include <iostream>
#include <stdexcept>
#include <tuple>
#include <vector>

using namespace anlnext;

void require(bool condition, const char* message)
{
  if (!condition) { throw std::runtime_error(message); }
}

template <typename F>
void expect_unselected(F operation, const std::string& message)
{
  try { operation(); }
  catch (const ANLException& ex) {
    require(ex.get_message().find(message) != std::string::npos,
            "unexpected selection error");
    return;
  }
  throw std::runtime_error("unselected operation did not throw");
}

template <typename Base>
class Parameters : public Base {
public:
  void setup()
  {
    this->define_parameter("value", &Parameters::value_);
    this->define_parameter("rows", &Parameters::rows_);
    this->add_value_element("entry", 0);
    this->set_value_element("entry", 7);
    this->insert_to_container();
  }

  void describe_parameter() { this->set_parameter_description("changed"); }
  void describe_element() { this->set_value_element_description("changed"); }
  void add_element() { this->add_value_element("extra", 0); }
  int value() const { return value_; }
  const auto& rows() const { return rows_; }

private:
  int value_ = 10;
  std::vector<std::tuple<int>> rows_;
};

class TestModule : public Parameters<BasicModule> {
  DEFINE_ANL_MODULE(TestModule, 1.0);
  ENABLE_PARALLEL_RUN();
};

class TestSubModule : public Parameters<BasicSubModule> {
  DEFINE_ANL_SUBMODULE(TestSubModule, 1.0);
};

template <typename T>
void verify_unselected(T& object)
{
  expect_unselected([&] { object.set_map_key("key"); }, "No current parameter");
  expect_unselected([&] { object.insert_to_container(); }, "No current parameter");
  expect_unselected([&] { object.set_value_element("entry", 42); }, "No current parameter");
  expect_unselected([&] { object.describe_parameter(); }, "No current parameter");
  expect_unselected([&] { object.add_element(); }, "No current parameter");
  expect_unselected([&] { object.describe_element(); }, "No current value element");
}

template <typename T>
void verify_duplication()
{
  T source;
  verify_unselected(source);
  source.setup();
  source.set_module_description("source description");
  T copied(source);
  require(copied.module_description() == "source description",
          "copy construction lost module description");
  require(copied.parameter_begin() == copied.parameter_end(),
          "copy construction must start with an empty registry");
  require(copied.value() == source.value() && copied.rows() == source.rows(),
          "copy construction lost module data");
  verify_unselected(copied);
  auto copy1 = source.duplicate();
  auto copy2 = source.duplicate();
  auto& first = dynamic_cast<T&>(*copy1);
  auto& second = dynamic_cast<T&>(*copy2);
  require(first.module_description() == "source description" &&
          second.module_description() == "source description",
          "duplication lost module description");
  first.set_module_description("copy description");
  require(source.module_description() == "source description" &&
          second.module_description() == "source description",
          "module descriptions are not independent");
  verify_unselected(first);
  verify_unselected(second);

  // Named setters do not require a current selection.
  first.set_parameter("value", 20);
  second.set_parameter("value", 30);
  require(source.value() == 10 && first.value() == 20 && second.value() == 30,
          "named settings are not independent");

  // Explicitly select the cloned container; the source selection must remain intact.
  first.expose_parameter("rows");
  first.set_value_element("entry", 11);
  first.insert_to_container();
  source.set_value_element("entry", 13);
  source.insert_to_container();
  require(source.rows().size() == 2 && std::get<0>(source.rows().back()) == 13,
          "source selection changed during duplication");
  require(first.rows().size() == 2 && std::get<0>(first.rows().back()) == 11,
          "cloned selection did not target cloned data");
  require(second.rows().size() == 1, "another clone was modified");

  // Selecting a parameter does not restore a stale value-element selection.
  expect_unselected([&] { first.describe_element(); }, "No current value element");
  source.describe_element();
  require(source.get_parameter("rows")->value_element_info(0)->description() == "changed",
          "source element selection was lost");
  require(first.get_parameter("rows")->value_element_info(0)->description().empty(),
          "cloned element metadata is shared");
}

int main()
{
  try {
    verify_duplication<TestModule>();
    verify_duplication<TestSubModule>();
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
