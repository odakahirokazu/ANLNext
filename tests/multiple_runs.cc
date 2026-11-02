#include "ANLManager.hh"
#include "ANLManagerMT.hh"
#include "BasicModule.hh"
#include "EvsManager.hh"

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

using namespace anlnext;

void require(bool condition, const char* message)
{
  if (!condition) { throw std::runtime_error(message); }
}

struct State {
  std::mutex mutex;
  std::condition_variable cv;
  std::size_t parallels = 1;
  std::size_t arrivals = 0;
  std::size_t generation = 0;
  bool synchronize = false;
  std::string failure;

  void wait_for_workers()
  {
    std::unique_lock<std::mutex> lock(mutex);
    const auto current = generation;
    if (++arrivals == parallels) {
      arrivals = 0;
      ++generation;
      cv.notify_all();
    }
    else {
      require(cv.wait_for(lock, std::chrono::seconds(5), [&] { return generation != current; }),
              "workers did not all start");
    }
  }
};

class RunModule : public BasicModule {
  DEFINE_ANL_MODULE(RunModule, 1.0);
  ENABLE_PARALLEL_RUN();
public:
  RunModule(State& state, bool cumulative) : state_(&state), cumulative_(cumulative) {}

  void set_evs_manager(EvsManager* manager) override
  {
    BasicModule::set_evs_manager(manager);
    evs_ = manager;
  }

  ANLStatus mod_define() override
  {
    ++define_calls;
    define_evs(module_id());
    return AS_OK;
  }
  ANLStatus mod_pre_initialize() override { ++pre_initialize_calls; return AS_OK; }
  ANLStatus mod_initialize() override
  {
    require(get_run_index() == undefined_run_index, "initialization already has a run index");
    ++initialize_calls;
    return AS_OK;
  }
  ANLStatus mod_begin_run() override
  {
    require(get_run_index() == begin_runs.size(), "incorrect run index at begin");
    require(get_loop_index() == undefined_loop_index, "begin_run retained a loop index");
    for (const auto& entry: evs_->data()) {
      require(entry.second.counts == 0 && entry.second.counts_ok == 0 && !entry.second.flag,
              "begin_run retained Evs statistics or flags");
    }
    begin_runs.push_back(get_run_index());
    loops.emplace_back();
    if (!cumulative_ || !is_master()) { result = 0; }
    if (!cumulative_ && is_master()) {
      if (state_->failure == "exception") { throw std::runtime_error("begin failure"); }
      if (state_->failure == "critical") { return AS_CRITICAL_ERROR_TO_TERMINATE; }
    }
    return AS_OK;
  }
  ANLStatus mod_analyze() override
  {
    require(get_run_index() == begin_runs.back(), "analysis received a different run index");
    if (!cumulative_ && state_->synchronize && loops.back().empty()) { state_->wait_for_workers(); }
    loops.back().push_back(get_loop_index());
    ++lifetime_calls;
    ++result;
    set_evs(module_id());
    return AS_OK;
  }
  ANLStatus mod_end_run() override
  {
    require(get_run_index() == begin_runs.back(), "end_run received a different run index");
    end_runs.push_back(get_run_index());
    return AS_OK;
  }
  ANLStatus mod_reduce(const std::vector<BasicModule*>& parallel_modules) override
  {
    require(end_runs.back() == get_run_index(), "reduction preceded end_run");
    for (const BasicModule* module: parallel_modules) {
      const auto& copy = dynamic_cast<const RunModule&>(*module);
      require(copy.get_run_index() == get_run_index() && copy.end_runs.back() == get_run_index(),
              "parallel end_run was not completed before reduction");
    }
    reduce_runs.push_back(get_run_index());
    return BasicModule::mod_reduce(parallel_modules);
  }
  ANLStatus mod_merge(const BasicModule* module) override
  {
    const auto& copy = dynamic_cast<const RunModule&>(*module);
    result += copy.result;
    return AS_OK;
  }
  ANLStatus mod_finalize() override { ++finalize_calls; return AS_OK; }

  int define_calls = 0;
  int pre_initialize_calls = 0;
  int initialize_calls = 0;
  int finalize_calls = 0;
  std::size_t lifetime_calls = 0;
  std::size_t result = 0;
  std::vector<std::size_t> begin_runs;
  std::vector<std::size_t> end_runs;
  std::vector<std::size_t> reduce_runs;
  std::vector<std::vector<std::size_t>> loops;
private:
  State* state_;
  bool cumulative_;
  EvsManager* evs_ = nullptr;
};

template<class Base>
class Manager : public Base {
public:
  using Base::Base;
  void verify_statistics(std::size_t count) const
  {
    for (const auto& counter: this->counters_) {
      require(counter.entry() == static_cast<long>(count) && counter.ok() == static_cast<long>(count) &&
              counter.error() == 0 && counter.skip() == 0 && counter.quit() == 0,
              "framework counters are not local to the run");
    }
    for (const auto& entry: this->evs_manager_->data()) {
      require(entry.second.counts == count && entry.second.counts_ok == count,
              "Evs counters are not local to the run");
    }
  }
protected:
  ANLStatus routine_begin_run() override
  {
    verify_statistics(0);
    for (std::size_t chain_index=0; chain_index<this->number_of_parallels(); ++chain_index) {
      for (const BasicModule* master: this->modules_) {
        const BasicModule* module = this->access_to_module(chain_index, master->module_id());
        require(module->get_run_index() == this->run_index() &&
                module->get_loop_index() == BasicModule::undefined_loop_index,
                "routine_begin_run started before run preparation completed");
      }
    }
    return Base::routine_begin_run();
  }
};

template<class ManagerType>
void verify(ManagerType& manager, bool reduction, const std::string& failure)
{
  State state;
  state.parallels = manager.number_of_parallels();
  RunModule reset(state, false), cumulative(state, true);
  reset.set_module_id("reset");
  cumulative.set_module_id("cumulative");
  manager.set_modules({&reset, &cumulative});
  manager.set_print_parallel_modules(false);
  require(manager.run_index() == ANLManager::undefined_run_index, "new manager already has a run index");
  require(manager.define_analysis() == AS_OK, "definition failed");
  require(manager.pre_initialize_analysis() == AS_OK, "pre-initialization failed");
  require(manager.initialize_analysis() == AS_OK, "initialization failed");
  require(manager.run_index() == ANLManager::undefined_run_index, "initialization consumed a run index");

  std::vector<RunModule*> reset_modules, cumulative_modules;
  for (std::size_t chain_index=0; chain_index<state.parallels; ++chain_index) {
    reset_modules.push_back(dynamic_cast<RunModule*>(manager.access_to_module(chain_index, "reset")));
    cumulative_modules.push_back(dynamic_cast<RunModule*>(manager.access_to_module(chain_index, "cumulative")));
    require(reset_modules.back() && cumulative_modules.back(), "missing module instance");
  }

  std::size_t total = 0;
  const std::vector<std::size_t> run_sizes = failure.empty() ? std::vector<std::size_t>{8, 4, 0, 2}
                                                          : std::vector<std::size_t>{8};
  for (std::size_t run_index=0; run_index<run_sizes.size(); ++run_index) {
    const auto count = run_sizes[run_index];
    state.synchronize = state.parallels > 1 && count >= state.parallels;
    require(manager.run_analysis(count, false) == AS_OK, "run failed");
    require(manager.run_index() == run_index, "incorrect manager run index");
    manager.verify_statistics(count);
    total += count;
    require(reset.result == count, "per-run module result is incorrect");
    require(cumulative.result == total, "cumulative module result is incorrect");

    for (const auto& modules: {reset_modules, cumulative_modules}) {
      std::vector<std::size_t> loops;
      std::size_t lifetime = 0;
      for (std::size_t chain_index=0; chain_index<modules.size(); ++chain_index) {
        const RunModule* module = modules[chain_index];
        require(manager.access_to_module(chain_index, module->module_id()) == module, "module was recreated");
        require(module->initialize_calls == 1 && module->finalize_calls == 0, "run repeated initialization or finalization");
        require(module->get_run_index() == manager.run_index(), "chain received a different run index");
        require(module->begin_runs.size() == run_index + 1 && module->end_runs.size() == run_index + 1,
                "begin_run or end_run was not called once per run");
        loops.insert(loops.end(), module->loops.back().begin(), module->loops.back().end());
        lifetime += module->lifetime_calls;
      }
      std::sort(loops.begin(), loops.end());
      require(loops.size() == count && lifetime == total, "module state or processing count was lost");
      for (std::size_t loop_index=0; loop_index<count; ++loop_index) {
        require(loops[loop_index] == loop_index, "loop indices did not restart at zero");
      }
      require(modules[0]->reduce_runs.size() == (reduction ? run_index + 1 : 0), "incorrect reduction count");
    }
  }

  if (!failure.empty()) {
    state.failure = failure;
    bool caught = false;
    try {
      require(manager.run_analysis(8, false) == AS_CRITICAL_ERROR_TO_TERMINATE, "begin failure status was lost");
    }
    catch (const std::runtime_error& error) {
      require(failure == "exception" && std::string(error.what()) == "begin failure", "unexpected exception");
      caught = true;
    }
    require(caught == (failure == "exception"), "begin exception was lost");
    require(manager.run_index() == 1, "failed run did not consume its index");
    manager.verify_statistics(0);
    for (const auto& modules: {reset_modules, cumulative_modules}) {
      for (const auto* module: modules) {
        require(module->get_run_index() == 1, "run index was not propagated before begin_run failed");
      }
    }
  }

  require(reset.define_calls == 1 && reset.pre_initialize_calls == 1, "run repeated preparation");
  require(manager.finalize_analysis() == AS_OK, "finalization failed");
  for (const auto& modules: {reset_modules, cumulative_modules}) {
    for (const auto* module: modules) { require(module->finalize_calls == 1, "incorrect finalization count"); }
  }
}

int main(int argc, char** argv)
{
  try {
    require(argc == 2, "test mode required");
    const std::string mode = argv[1];
    if (mode == "serial") {
      Manager<ANLManager> manager;
      verify(manager, false, "");
    }
    else {
      Manager<ANLManagerMT> manager(mode == "mt_single" ? 1 : 4);
      verify(manager, true, mode == "begin_failure" ? "critical" : mode == "begin_exception" ? "exception" : "");
    }
  }
  catch (const ANLException& error) {
    std::cerr << error.to_string() << '\n';
    return 1;
  }
  catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
