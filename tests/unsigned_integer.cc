#include "UnsignedIntegerExample.hh"
#include "BasicSubModule.hh"
#include <limits>
#include <sstream>
#include <stdexcept>

using namespace anlnext;

void require(bool condition, const char* message)
{
  if (!condition) { throw std::runtime_error(message); }
}

template <typename Error, typename F>
void expect_error(F action)
{
  try { action(); }
  catch (const Error&) { return; }
  throw std::runtime_error("expected error");
}

class UnsignedSubmodule : public BasicSubModule {
  DEFINE_ANL_SUBMODULE(UnsignedSubmodule, 1.0);
public:
  void define_parameters() override { define_parameter("address", &UnsignedSubmodule::address); }
  uintmax_t address = 0;
};

int main()
{
  try {
    const auto maximum = UnsignedIntegerExample::maximum();
    const auto signed_maximum = UnsignedIntegerExample::signed_maximum();
    UnsignedIntegerExample module;
    module.mod_define();
    const auto* param = module.get_parameter("address");
    require(param->type_name() == "unsigned_integer", "incorrect type name");
    for (uintmax_t value: {uintmax_t{0}, signed_maximum, signed_maximum + 1, maximum}) {
      module.set_parameter_unsigned_integer("address", value);
      require(module.address == value && param->get_value_unsigned_integer() == value &&
              param->get_value(uintmax_t{}) == value, "unsigned API lost precision");
      module.set_parameter("address", value);
      require(module.address == value, "typed set_parameter lost precision");
      const auto tree = param->to_property_tree();
      require(tree.get<std::string>("type") == "unsigned_integer" &&
              tree.get<std::string>("value") == std::to_string(value), "JSON lost precision");
      auto copy = module.duplicate();
      require(copy->get_parameter("address")->get_value_unsigned_integer() == value, "copy lost value");
      copy->set_parameter_unsigned_integer("address", value == 0 ? maximum : 0);
      require(module.address == value, "copy changed source");
      uintmax_t buffer = 0;
      ModuleParameter<uintmax_t> standalone("address", &buffer);
      standalone.set_expression("hex");
      std::istringstream hex(param->value_string());
      standalone.input(hex);
      require(!hex.fail() && buffer == value, "hex round trip failed");
      standalone.set_expression("dec");
      std::istringstream decimal(std::to_string(value));
      standalone.input(decimal);
      require(!decimal.fail() && buffer == value, "decimal round trip failed");
    }
    expect_error<ParameterError>([&] { param->get_value(0); });
    expect_error<ParameterError>([&] { param->get_value_integer(); });
    expect_error<ParameterError>([&] { module.set_parameter("address", -1); });
    expect_error<ParameterError>([&] { module.set_parameter_integer("address", -1); });
    expect_error<ParameterTypeError>([&] { module.set_parameter("address", 1.5); });
    expect_error<ParameterTypeError>([&] { module.set_parameter_unsigned_integer("count", maximum); });
    expect_error<ParameterTypeError>([&] { module.get_parameter("count")->get_value_unsigned_integer(); });
    require(module.address == maximum, "failed setting changed value");
    module.set_parameter_integer("address", 23);
    require(param->get_value_integer() == 23 && param->get_value(0) == 23, "small signed API failed");
    module.set_parameter_integer("signed_value", std::numeric_limits<intmax_t>::min());
    require(module.get_parameter("signed_value")->get_value_integer() == std::numeric_limits<intmax_t>::min(),
            "signed API changed");

    uintmax_t buffer = 37;
    ModuleParameter<uintmax_t> standalone("address", &buffer);
    for (const auto& token: {std::string("-1"), std::string("-0"), std::string("1.5"),
                            std::string("12x"), std::string("+"), std::to_string(maximum) + "0"}) {
      std::istringstream input(token);
      standalone.input(input);
      require(input.fail() && buffer == 37, "invalid decimal input changed value");
    }
    standalone.set_expression("hex");
    for (const auto* token: {"0x25", "25", "+0X25"}) {
      std::istringstream input(token);
      standalone.input(input);
      require(!input.fail() && buffer == 37, "hex prefix parsing failed");
    }
    for (const auto* token: {"-1", "-0x1", "0x", "gg", "100000000000000000000000000000000000000000000000000"}) {
      std::istringstream input(token);
      standalone.input(input);
      require(input.fail() && buffer == 37, "invalid hex input changed value");
    }
    std::ostringstream output;
    standalone.output(output);
    output << ' ' << 16;
    require(output.str() == "25 16", "output leaked hex format");
    ModuleParameterUnique<uintmax_t> owned("owned", maximum);
    auto copied_parameter = owned.clone();
    copied_parameter->set_value_unsigned_integer(0);
    require(owned.get_value_unsigned_integer() == maximum, "owned parameter clone changed source");
    UnsignedSubmodule submodule;
    submodule.define_parameters();
    submodule.set_parameter_unsigned_integer("address", maximum);
    auto subcopy = submodule.duplicate();
    require(subcopy->get_parameter("address")->get_value_unsigned_integer() == maximum, "submodule copy lost value");
    subcopy->set_parameter_unsigned_integer("address", 0);
    require(submodule.address == maximum, "submodule copy changed source");
  }
  catch (const ANLException& ex) { std::cerr << ex.to_string() << '\n'; return 1; }
  catch (const std::exception& ex) { std::cerr << ex.what() << '\n'; return 1; }
}
