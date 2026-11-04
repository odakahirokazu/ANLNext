#include "SubmoduleExample.hh"
using namespace anlnext;

void ExampleAlgorithm::define_parameters()
{
  define_parameter("factor", &ExampleAlgorithm::factor_);
  define_evs("algorithm_used");
}

void ContainerAlgorithm::define_parameters()
{
  ExampleAlgorithm::define_parameters();
  define_parameter("samples", &ContainerAlgorithm::samples_);
  define_parameter("weights", &ContainerAlgorithm::weights_);
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
  define_submodule("algorithm", &SubmoduleExample::algorithm_);
  define_submodule("optional");
  return AS_OK;
}

void SubmoduleExample::set_submodule_by_key(const std::string& name, const std::string& key)
{
  if (key == "multiply") {
    set_submodule(name, std::make_unique<ExampleAlgorithm>());
  }
  else if (key == "containers") {
    set_submodule(name, std::make_unique<ContainerAlgorithm>());
  }
  else {
    throw ANLException("Unknown algorithm: " + key);
  }
}

ANLStatus SubmoduleExample::mod_initialize()
{
  return mod_analyze();
}

ANLStatus SubmoduleExample::mod_analyze()
{
  reset_evs("algorithm_used");
  result_ = algorithm_->calculate(input_, this);
  if (!evs("algorithm_used")) {
    throw ANLException("Submodule Evs is connected to another chain");
  }
  return AS_OK;
}
