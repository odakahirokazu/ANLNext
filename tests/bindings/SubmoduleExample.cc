#include "SubmoduleExample.hh"
using namespace anlnext;

void ExampleAlgorithm::define_parameters()
{
  define_parameter("factor", &ExampleAlgorithm::factor_);
  define_evs("algorithm_used");
}

int ExampleAlgorithm::calculate(int input, const BasicModule* parent)
{
  if (get_module<BasicModule>(parent->module_id()) != parent) {
    throw ModuleAccessError("Submodule connected to another chain", parent->module_id());
  }
  if (!is_evs_defined("algorithm_used")) {
    throw ANLException("Submodule Evs connection is missing");
  }
  set_evs("algorithm_used");
  return input * factor_;
}

ANLStatus SubmoduleExample::mod_define()
{
  define_parameter("input", &SubmoduleExample::input_);
  define_parameter("result", &SubmoduleExample::result_);
  define_submodule("algorithm");
  define_submodule("optional");
  return AS_OK;
}

void SubmoduleExample::set_submodule_by_key(const std::string& name, const std::string& key)
{
  if (key != "multiply") { throw ANLException("Unknown algorithm: " + key); }
  set_submodule(name, std::make_unique<ExampleAlgorithm>());
}

ANLStatus SubmoduleExample::mod_initialize()
{
  return mod_analyze();
}

ANLStatus SubmoduleExample::mod_analyze()
{
  auto& algorithm = dynamic_cast<ExampleAlgorithm&>(*get_submodule("algorithm"));
  reset_evs("algorithm_used");
  result_ = algorithm.calculate(input_, this);
  if (!evs("algorithm_used")) {
    throw ANLException("Submodule Evs is connected to another chain");
  }
  return AS_OK;
}
