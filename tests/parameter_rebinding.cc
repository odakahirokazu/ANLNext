#include "ParameterRegistry.hh"

#include <iostream>
#include <stdexcept>

using namespace anlnext;

struct Owner : ParameterRegistry {
  int value = 10;
};

struct OtherOwner : ParameterRegistry {};

void require(bool condition, const char* message)
{
  if (!condition) { throw std::runtime_error(message); }
}

void expect_rebinding_error(VModuleParameter& parameter, ParameterRegistry* target)
{
  try { parameter.set_module_pointer(target); }
  catch (const ParameterError& ex) {
    require(ex.get_parameter_name() == "value", "error lost the parameter name");
    require(ex.get_message().find("Module type mismatch") != std::string::npos,
            "error lost the failure reason");
    return;
  }
  throw std::runtime_error("invalid rebinding did not throw");
}

int main()
{
  try {
    Owner original, destination;
    OtherOwner incompatible;
    ModuleParameterMember<Owner, int> member("value", &original, &Owner::value);
    VModuleParameter& parameter = member;

    parameter.set_module_pointer(&destination);
    parameter.set_value(20);
    require(original.value == 10 && destination.value == 20,
            "successful rebinding did not select the destination");

    expect_rebinding_error(parameter, &incompatible);
    parameter.set_value(30);
    require(original.value == 10 && destination.value == 30,
            "failed rebinding changed the destination");

    expect_rebinding_error(parameter, nullptr);
    parameter.set_value(40);
    require(original.value == 10 && destination.value == 40,
            "null rebinding changed the destination");

    parameter.set_module_pointer(&original);
    parameter.set_value(50);
    require(original.value == 50 && destination.value == 40,
            "valid rebinding after errors failed");
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
