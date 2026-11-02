#include "BasicModule.hh"
#include "ANLManager.hh"
#include "ANLManagerMT.hh"
#include "OrderKeeper.hh"

#include <algorithm>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

using namespace anlnext;

void require(bool condition, const char* message)
{
  if (!condition) { throw std::runtime_error(message); }
}

struct State {
  std::mutex mutex;
  std::condition_variable cv;
  bool later_started = false;
  std::string mode;
  std::vector<std::size_t> input;
  std::vector<std::size_t> output;
};

class Input : public BasicModule {
  DEFINE_ANL_MODULE(Input, 1.0);
  ENABLE_PARALLEL_RUN();
public:
  explicit Input(State& state) : state_(&state) {}
  ANLStatus mod_analyze() override
  {
    const auto index = get_loop_index();
    {
      std::unique_lock<std::mutex> lock(state_->mutex);
      state_->input.push_back(index);
      if (index > 0) {
        state_->later_started = true;
        state_->cv.notify_all();
      }
      if (index == 0 && state_->mode.find("exception") != std::string::npos) {
        // Ensure another event can be waiting at the downstream ordered module.
        state_->cv.wait(lock, [this] { return state_->later_started; });
      }
    }
    if (index == 0) {
      if (state_->mode == "exception") { throw std::runtime_error("worker failure"); }
      if (state_->mode == "anl_exception") {
        ANLException ex("worker failure");
        ex.request_treatment(ANLException::Treatment::finalize);
        throw ex;
      }
      if (state_->mode == "terminate_exception") {
        ANLException ex("worker failure");
        ex.request_treatment(ANLException::Treatment::terminate);
        throw ex;
      }
      if (state_->mode == "quit") { return AS_QUIT; }
      if (state_->mode == "quit_all") { return AS_QUIT_ALL; }
      if (state_->mode == "critical") { return AS_CRITICAL_ERROR_TO_FINALIZE; }
    }
    if (state_->mode == "skip" && index % 2 == 0) { return AS_SKIP; }
    std::this_thread::yield();
    return AS_OK;
  }
  ANLStatus mod_reduce(const std::list<BasicModule*>& parallel_modules) override
  {
    if (state_->mode == "reduce") { return AS_CRITICAL_ERROR_TO_TERMINATE; }
    return BasicModule::mod_reduce(parallel_modules);
  }
private:
  State* state_;
};

class Output : public BasicModule {
  DEFINE_ANL_MODULE(Output, 1.0);
  ENABLE_PARALLEL_RUN();
public:
  explicit Output(State& state) : state_(&state) { set_order_sensitive(true); }
  ANLStatus mod_analyze() override
  {
    std::lock_guard<std::mutex> lock(state_->mutex);
    state_->output.push_back(get_loop_index());
    return AS_OK;
  }
private:
  State* state_;
};

void verify_complete(const State& state, bool skip)
{
  auto input = state.input;
  std::sort(input.begin(), input.end());
  std::vector<std::size_t> expected_input, expected_output;
  for (std::size_t i = 0; i < 100; ++i) {
    expected_input.push_back(i);
    if (!skip || i % 2 != 0) { expected_output.push_back(i); }
  }
  require(input == expected_input, "each attempt must receive one distinct index");
  require(state.output == expected_output, "ordered output must follow event indices");
}

int main(int argc, char** argv)
{
  try {
    require(argc == 2, "one test mode required");
    State state;
    state.mode = argv[1];
    Input input(state);
    Output disabled(state), output(state);
    disabled.set_module_id("disabled");
    disabled.off();
    std::unique_ptr<ANLManager> manager;
    if (state.mode == "serial") { manager = std::make_unique<ANLManager>(); }
    else { manager = std::make_unique<ANLManagerMT>(4); }
    manager->set_modules({&input, &disabled, &output});
    manager->set_display_period(1000);
    require(manager->define_analysis() == AS_OK, "Define failed");
    require(manager->pre_initialize_analysis() == AS_OK, "PreInitialize failed");
    require(manager->initialize_analysis() == AS_OK, "Initialize failed");

    bool caught = false;
    ANLStatus status = AS_OK;
    try { status = manager->run_analysis(100, false); }
    catch (const std::runtime_error& ex) {
      require(state.mode == "exception" && std::string(ex.what()) == "worker failure",
              "unexpected exception");
      caught = true;
    }
    if (state.mode == "exception") { require(caught, "worker exception was lost"); }
    else if (state.mode == "anl_exception") {
      require(status == ANLStatus::critical_error_to_finalize_from_exception, "lost finalize treatment");
    }
    else if (state.mode == "terminate_exception") {
      require(status == ANLStatus::critical_error_to_terminate_from_exception, "lost terminate treatment");
    }
    else if (state.mode == "critical") {
      require(status == AS_CRITICAL_ERROR_TO_FINALIZE, "lost critical status");
    }
    else if (state.mode == "reduce") {
      require(status == AS_CRITICAL_ERROR_TO_TERMINATE, "lost reduction status");
      verify_complete(state, false);
    }
    else {
      require(status == AS_OK, "Analyze failed");
      if (state.mode == "quit" || state.mode == "quit_all") {
        require(std::is_sorted(state.output.begin(), state.output.end()), "quit broke ordering");
        require(std::find(state.output.begin(), state.output.end(), 0) == state.output.end(), "quit event continued");
        if (state.mode == "quit") { require(state.input.size() == 100, "QUIT stopped other workers"); }
      }
      else { verify_complete(state, state.mode == "skip"); }
    }

    // Reuse the manager: neither completed nor cancelled keepers may retain state.
    state.mode = "ordered";
    state.input.clear();
    state.output.clear();
    require(manager->run_analysis(100, false) == AS_OK, "second run failed");
    verify_complete(state, false);
    require(manager->finalize_analysis() == AS_OK, "Finalize failed");
  }
  catch (const std::exception& ex) {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
