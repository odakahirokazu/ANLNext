#include "ANLManager.hh"
#include "ANLManagerMT.hh"
#include "BasicModule.hh"
#include <iostream>
#include <stdexcept>

using namespace anlnext;

void require(bool condition, const char* message)
{
  if (!condition) { throw std::runtime_error(message); }
}

template <typename F>
void expect_requirement_error(F action, const std::string& requester, const std::string& target,
                              const std::string& position)
{
  try { action(); }
  catch (const ModuleRequirementError& ex) {
    const auto* id = boost::get_error_info<ErrorInfoOnModuleID>(ex);
    const auto message = ex.get_message();
    require(id && *id == requester, "requester missing from error");
    require(message.find(target) != std::string::npos &&
            message.find("position: " + position) != std::string::npos, "requirement error lacks context");
    return;
  }
  throw std::runtime_error("expected ModuleRequirementError");
}

class RequirementModule : public BasicModule {
  DEFINE_ANL_MODULE(RequirementModule, 1.0);
  ENABLE_PARALLEL_RUN();
public:
  RequirementModule(const std::string& id, const std::string& dependency="",
                    ModulePosition position=ModulePosition::any)
    : id_(id), dependency_(dependency), position_(position)
  { set_module_id(id + "_pending"); }

  using BasicModule::set_access_permission;

  ANLStatus mod_define() override
  {
    defined = true;
    set_module_id(id_);
    if (!late_alias.empty()) { add_alias(late_alias); }
    if (!dependency_.empty()) { require_module(dependency_, position_); }
    return AS_OK;
  }

  std::string late_alias;
  bool defined = false;
private:
  std::string id_;
  std::string dependency_;
  ModulePosition position_;
};

void verify_positions()
{
  for (auto position: {ModulePosition::before, ModulePosition::after, ModulePosition::any}) {
    for (bool target_first: {false, true}) {
      RequirementModule target("producer");
      target.late_alias = "events";
      RequirementModule requester("consumer", "events", position);
      ANLManager manager;
      manager.set_modules(target_first ? std::vector<BasicModule*>{&target, &requester}
                                       : std::vector<BasicModule*>{&requester, &target});
      const bool succeeds = position == ModulePosition::any ||
                            (position == ModulePosition::before && target_first) ||
                            (position == ModulePosition::after && !target_first);
      if (succeeds) { require(manager.define_analysis() == AS_OK, "valid requirement failed"); }
      else {
        expect_requirement_error([&] { manager.define_analysis(); }, "consumer", "events",
                                 position == ModulePosition::before ? "before" : "after");
      }
      require(target.defined && requester.defined, "validated before all definitions finished");
    }
    RequirementModule missing("consumer", "missing", position);
    ANLManager missing_manager;
    missing_manager.set_modules({&missing});
    expect_requirement_error([&] { missing_manager.define_analysis(); }, "consumer", "missing",
      position == ModulePosition::before ? "before" : position == ModulePosition::after ? "after" : "any");
    RequirementModule self("self", "self_alias", position);
    self.late_alias = "self_alias";
    ANLManager self_manager;
    self_manager.set_modules({&self});
    if (position == ModulePosition::any) {
      require(self_manager.define_analysis() == AS_OK, "any must allow self");
    }
    else {
      expect_requirement_error([&] { self_manager.define_analysis(); }, "self", "self_alias",
                               position == ModulePosition::before ? "before" : "after");
    }
  }
}

void verify_alias_resolution()
{
  for (auto option: {ModuleAccess::ConflictOption::yield, ModuleAccess::ConflictOption::overwrite,
                     ModuleAccess::ConflictOption::remove}) {
    RequirementModule first("first"), requester("consumer", "shared", ModulePosition::before), last("last");
    first.add_alias("shared");
    last.add_alias("shared", option);
    ANLManager manager;
    manager.set_modules({&first, &requester, &last});
    if (option == ModuleAccess::ConflictOption::yield) {
      require(manager.define_analysis() == AS_OK, "alias yield lost first target");
    }
    else {
      expect_requirement_error([&] { manager.define_analysis(); }, "consumer", "shared", "before");
    }
  }
}

void verify_permissions()
{
  RequirementModule target("producer"), requester("consumer", "producer", ModulePosition::before);
  target.set_access_permission(ModuleAccess::Permission::no_access);
  requester.set_access_permission(ModuleAccess::Permission::privacy);
  ANLManager manager;
  manager.set_modules({&target, &requester});
  require(manager.define_analysis() == AS_OK, "order check must be independent of access permission");
  RequirementModule hidden("hidden"), dependent("dependent", "hidden");
  hidden.set_access_permission(ModuleAccess::Permission::privacy);
  ANLManager hidden_manager;
  hidden_manager.set_modules({&hidden, &dependent});
  expect_requirement_error([&] { hidden_manager.define_analysis(); }, "dependent", "hidden", "any");
}

void verify_clone_state()
{
  RequirementModule target("producer");
  RequirementModule requester("consumer", "producer", ModulePosition::before);
  ANLManagerMT manager(3);
  manager.set_modules({&target, &requester});
  require(manager.define_analysis() == AS_OK, "define failed");
  require(manager.pre_initialize_analysis() == AS_OK, "parallel duplication failed");
  for (std::size_t index=0; index<3; ++index) {
    manager.access_to_module(index, "consumer")->check_module_requirements();
  }
  auto detached = requester.duplicate();
  expect_requirement_error([&] { detached->check_module_requirements(); },
                           "consumer", "producer", "before");
}

int main()
{
  try {
    verify_positions();
    verify_alias_resolution();
    verify_permissions();
    verify_clone_state();
  }
  catch (const ANLException& ex) { std::cerr << ex.to_string() << '\n'; return 1; }
  catch (const std::exception& ex) { std::cerr << ex.what() << '\n'; return 1; }
}
