#include "BasicModule.hh"
#include "BasicSubModule.hh"
#include "EvsManager.hh"
#include <iostream>
#include <stdexcept>

using namespace anlnext;

void require(bool value, const char* message)
{
  if (!value) { throw std::runtime_error(message); }
}

template<class E, class F> void expect_error(F action)
{
  try { action(); }
  catch (const E& ex) {
    require(boost::get_error_info<ErrorInfoOnModuleName>(ex) != nullptr,
            "exception lacks parent module information");
    return;
  }
  throw std::runtime_error("expected ANLException");
}

class Algorithm : public BasicSubModule {
  DEFINE_ANL_SUBMODULE(Algorithm, 1.0);
public:
  void define_parameters() override
  {
    ++definitions;
    define_parameter("value", &Algorithm::value);
    require(is_evs_defined("first") && exist_module("first"),
            "services must be connected before parameter definition");
  }
  bool has_services(const std::string& key) const
  { return is_evs_defined(key) && exist_module(key); }
  bool is_disconnected() const
  {
    struct Probe : EvsInterface, ModuleAccessInterface {
      bool evs_connected = true;
      bool access_connected = true;
      void set_evs_manager(EvsManager* manager) override
      { evs_connected = manager != nullptr; }
      void set_module_access(const ModuleAccess* access) override
      { access_connected = access != nullptr; }
    } probe;
    connect_evs_to(probe);
    connect_module_access_to(probe);
    return !probe.evs_connected && !probe.access_connected;
  }
  int value = 10;
  int definitions = 0;
};

class Parent : public BasicModule {
  DEFINE_ANL_MODULE(Parent, 1.0);
  ENABLE_PARALLEL_RUN();
public:
  using BasicModule::define_submodule;
  using BasicModule::set_submodule;
  ANLStatus mod_define() override
  {
    define_submodule("algorithm");
    define_submodule("optional");
    return AS_OK;
  }
  void set_submodule_by_key(const std::string& name, const std::string& key) override
  {
    if (key != "algorithm") { throw ANLException("Unknown algorithm"); }
    set_submodule(name, std::make_unique<Algorithm>());
  }
};

int main()
{
  try {
    EvsManager evs1, evs2;
    ModuleAccess access1, access2;
    Parent parent;
    evs1.define("first");
    evs2.define("second");
    access1.register_module("first", &parent);
    access2.register_module("second", &parent);
    parent.set_evs_manager(&evs1);
    parent.set_module_access(&access1);
    require(parent.mod_define() == AS_OK, "define failed");
    require(parent.is_submodule_defined("algorithm"), "declaration missing");
    require(!parent.is_submodule_set("algorithm"), "declaration created an instance");
    expect_error<SubModuleAlreadyDefinedError>([&] { parent.define_submodule("algorithm"); });
    expect_error<SubModuleNotSetError>([&] { parent.get_submodule("algorithm"); });
    expect_error<SubModuleUndefinedError>([&] { parent.get_submodule("missing"); });
    expect_error<SubModuleUndefinedError>([&] { parent.is_submodule_set("missing"); });
    expect_error<SubModuleUndefinedError>([&] { parent.set_submodule_by_key("missing", "algorithm"); });
    expect_error<SubModuleNullError>([&] { parent.set_submodule("algorithm", nullptr); });
    parent.set_submodule_by_key("algorithm", "algorithm");
    auto* original = dynamic_cast<Algorithm*>(parent.get_submodule("algorithm"));
    require(original && original->definitions == 1, "parameters not defined once");
    original->set_parameter("value", 42);
    parent.set_submodule_by_key("algorithm", "algorithm");
    original = dynamic_cast<Algorithm*>(parent.get_submodule("algorithm"));
    require(original->value == 10 && original->definitions == 1, "replacement retained settings");
    original->set_parameter("value", 23);
    parent.set_evs_manager(&evs2);
    parent.set_module_access(&access2);
    require(original->has_services("second") && !original->has_services("first"),
            "service updates not propagated");
    auto cloned = parent.duplicate();
    auto* child = dynamic_cast<Algorithm*>(cloned->get_submodule("algorithm"));
    require(child && child != original && child->value == 23 && child->definitions == 1,
            "duplication lost state or redefined parameters");
    require(child->is_disconnected(), "clone retained source services");
    require(cloned->is_submodule_defined("optional") && !cloned->is_submodule_set("optional"),
            "unset declaration lost");
    child->set_parameter("value", 77);
    require(original->value == 23 && child->value == 77, "cloned parameter refers to source");
    cloned->set_evs_manager(&evs1);
    cloned->set_module_access(&access1);
    require(child->has_services("first") && !child->has_services("second"),
            "clone services not propagated");
    expect_error<SubModuleConfigurationError>([&] { parent.set_submodule_by_key("algorithm", "algorithm"); });
    expect_error<SubModuleConfigurationError>([&] { cloned->set_submodule_by_key("algorithm", "algorithm"); });
    expect_error<SubModuleConfigurationError>([&] { parent.define_submodule("late"); });
    auto second = parent.duplicate();
    require(second->is_submodule_set("algorithm"), "second duplication failed");
  }
  catch (const ANLException& ex) { std::cerr << ex.to_string() << '\n'; return 1; }
  catch (const std::exception& ex) { std::cerr << ex.what() << '\n'; return 1; }
}
