#include "BasicModule.hh"
#include "BasicSubModule.hh"
#include "EvsManager.hh"
#include <iostream>
#include <stdexcept>
#include <sstream>

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

class BindingBase : public BasicModule {
public:
  Algorithm* algorithm = nullptr;
};

class BoundParent : public BindingBase {
  DEFINE_ANL_MODULE(BoundParent, 1.0);
  ENABLE_PARALLEL_RUN();
public:
  using BasicModule::define_submodule;
  using BasicModule::set_submodule;
  ANLStatus mod_define() override
  {
    define_submodule("algorithm", &BindingBase::algorithm);
    define_submodule("optional", &BoundParent::optional);
    define_submodule("base", &BoundParent::base);
    return AS_OK;
  }
  Algorithm* optional = nullptr;
  BasicSubModule* base = nullptr;
};

class OtherOwner : public BasicModule {
public:
  Algorithm* algorithm = nullptr;
};

void verify_member_binding()
{
  BoundParent parent;
  EvsManager evs;
  ModuleAccess access;
  evs.define("first");
  access.register_module("first", &parent);
  parent.set_evs_manager(&evs);
  parent.set_module_access(&access);
  Algorithm external;
  parent.algorithm = &external;
  parent.optional = &external;
  parent.base = &external;
  require(parent.mod_define() == AS_OK, "bound declaration failed");
  require(!parent.algorithm && !parent.optional && !parent.base, "declaration retained a member pointer");
  parent.set_submodule("algorithm", std::make_unique<Algorithm>());
  require(parent.algorithm == parent.get_submodule("algorithm"), "registration did not bind the member");
  parent.algorithm->set_parameter("value", 23);
  auto* original = parent.algorithm;
  expect_error<SubModuleAlreadyDefinedError>([&] {
    parent.define_submodule("algorithm", &BoundParent::optional);
  });
  require(parent.algorithm == original && !parent.optional, "duplicate declaration changed a binding");
  expect_error<SubModuleBindingError>([&] {
    parent.set_submodule("algorithm", std::make_unique<BasicSubModule>());
  });
  require(parent.algorithm == original && parent.get_submodule("algorithm") == original,
          "type mismatch changed the registered instance or member");
  expect_error<SubModuleBindingError>([&] {
    parent.define_submodule("wrong_owner", &OtherOwner::algorithm);
  });
  require(!parent.is_submodule_defined("wrong_owner"), "invalid owner left a declaration");
  Algorithm* BoundParent::* null_member = nullptr;
  expect_error<SubModuleBindingError>([&] { parent.define_submodule("null_member", null_member); });
  parent.set_submodule("base", std::make_unique<Algorithm>());
  require(parent.base == parent.get_submodule("base"), "base pointer binding failed");

  for (int i=0; i<2; ++i) {
    auto copy = parent.duplicate();
    auto* bound = dynamic_cast<BoundParent*>(copy.get());
    require(bound && bound->algorithm == bound->get_submodule("algorithm") &&
            bound->algorithm != original, "copy retained the source member pointer");
    require(bound->algorithm->value == 23 && !bound->optional,
            "copy lost a value or an unset binding");
    require(bound->base == bound->get_submodule("base") && bound->base != parent.base,
            "copy lost the base pointer binding");
    bound->algorithm->set_parameter("value", 77);
    require(original->value == 23, "bound copy modified the source");
  }

  BoundParent replacement;
  replacement.set_evs_manager(&evs);
  replacement.set_module_access(&access);
  replacement.mod_define();
  replacement.set_submodule("algorithm", std::make_unique<Algorithm>());
  auto next = std::make_unique<Algorithm>();
  auto* next_pointer = next.get();
  replacement.set_submodule("algorithm", std::move(next));
  require(replacement.algorithm == next_pointer, "replacement did not update the member");
}

int main()
{
  try {
    verify_member_binding();
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
    BasicModule empty;
    require(!empty.parameters_to_property_tree().get_child_optional("submodule_list"),
            "undeclared submodule list must be omitted");
    const auto tree = parent.parameters_to_property_tree();
    const auto& entries = tree.get_child("submodule_list");
    require(entries.size() == 2, "submodule output lost declarations");
    const auto& set = entries.front().second;
    require(set.get<std::string>("registered_name") == "algorithm" &&
            set.get<std::string>("name") == "Algorithm" &&
            set.get<std::string>("version") == "1.0" &&
            set.get<std::string>("state") == "set", "submodule metadata mismatch");
    require(set.get_child("parameter_list").front().second.get<int>("value") == 23,
            "submodule parameter output mismatch");
    const auto& unset = entries.back().second;
    require(unset.size() == 2 && unset.get<std::string>("registered_name") == "optional" &&
            unset.get<std::string>("state") == "unset", "unset output mismatch");
    std::ostringstream output;
    auto* saved = std::cout.rdbuf(output.rdbuf());
    try {
      const ParameterRegistry& registry = parent;
      registry.print_parameters();
    }
    catch (...) {
      std::cout.rdbuf(saved);
      throw;
    }
    std::cout.rdbuf(saved);
    require(output.str().find("[algorithm: Algorithm / 1.0]") != std::string::npos &&
            output.str().find("[optional: unset]") != std::string::npos &&
            output.str().find("23") != std::string::npos, "submodule display mismatch");
    auto second = parent.duplicate();
    require(second->is_submodule_set("algorithm"), "second duplication failed");
  }
  catch (const ANLException& ex) { std::cerr << ex.to_string() << '\n'; return 1; }
  catch (const std::exception& ex) { std::cerr << ex.what() << '\n'; return 1; }
}
