#include "ANLManager.hh"
#include "BasicModule.hh"

#include <csignal>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace anlnext;

void require(bool condition, const char* message)
{
  if (!condition) { throw std::runtime_error(message); }
}

void caller_handler(int) {}
void caller_info_handler(int, siginfo_t*, void*) {}

class CallerSignal {
public:
  explicit CallerSignal(int kind)
  {
    require(sigaction(SIGINT, nullptr, &original_) == 0, "cannot read original SIGINT action");
    struct sigaction action{};
    sigemptyset(&action.sa_mask);
    sigaddset(&action.sa_mask, SIGUSR1);
    sigaddset(&action.sa_mask, SIGTERM);
    if (kind == 2) {
      action.sa_flags = SA_SIGINFO;
      action.sa_sigaction = caller_info_handler;
    }
    else {
      action.sa_handler = kind == 1 ? SIG_IGN : caller_handler;
    }
    require(sigaction(SIGINT, &action, nullptr) == 0, "cannot install caller SIGINT action");
    require(sigaction(SIGINT, nullptr, &expected_) == 0, "cannot read caller SIGINT action");
  }
  ~CallerSignal() { sigaction(SIGINT, &original_, nullptr); }
  CallerSignal(const CallerSignal&) = delete;
  CallerSignal& operator=(const CallerSignal&) = delete;

  void verify() const
  {
    struct sigaction after{};
    require(sigaction(SIGINT, nullptr, &after) == 0, "cannot read restored SIGINT action");
    require(after.sa_flags == expected_.sa_flags, "SIGINT flags were not restored");
    if (expected_.sa_flags & SA_SIGINFO) {
      require(after.sa_sigaction == expected_.sa_sigaction, "SIGINT info handler was not restored");
    }
    else {
      require(after.sa_handler == expected_.sa_handler, "SIGINT handler was not restored");
    }
    for (int signal: {SIGINT, SIGUSR1, SIGTERM}) {
      require(sigismember(&after.sa_mask, signal) == sigismember(&expected_.sa_mask, signal),
              "SIGINT handler mask was not restored");
    }
  }
private:
  struct sigaction original_{};
  struct sigaction expected_{};
};

class SignalModule : public BasicModule {
  DEFINE_ANL_MODULE(SignalModule, 1.0);
public:
  std::string phase;
  std::string mode;
  ANLStatus mod_initialize() override { return step("initialize"); }
  ANLStatus mod_analyze() override { return step("run"); }
  ANLStatus mod_finalize() override { return step("finalize"); }
private:
  ANLStatus step(const std::string& current_phase)
  {
    if (current_phase != phase) { return AS_OK; }
    if (mode == "critical") { return AS_CRITICAL_ERROR_TO_TERMINATE; }
    if (mode == "exception") { throw std::runtime_error("signal phase failure"); }
    if (mode == "anl_exception") {
      ANLException error("signal phase failure");
      error.request_treatment(ANLException::Treatment::rethrow);
      throw error;
    }
    return AS_OK;
  }
};

void verify(const std::string& phase, const std::string& mode, int handler_kind)
{
  SignalModule module;
  module.phase = phase;
  module.mode = mode;
  ANLManager manager;
  manager.set_modules({&module});
  require(manager.define_analysis() == AS_OK, "definition failed");
  require(manager.pre_initialize_analysis() == AS_OK, "pre-initialization failed");
  if (phase != "initialize") {
    require(manager.initialize_analysis() == AS_OK, "preparation failed");
  }

  CallerSignal caller_signal(handler_kind);
  bool caught = false;
  ANLStatus status = AS_OK;
  try {
    if (phase == "initialize") { status = manager.initialize_analysis(); }
    else if (phase == "run") { status = manager.run_analysis(1, false); }
    else { status = manager.finalize_analysis(); }
  }
  catch (const ANLException& error) {
    require(mode == "anl_exception" && error.get_message().find("signal phase failure") != std::string::npos,
            "unexpected ANL exception");
    caught = true;
  }
  catch (const std::runtime_error& error) {
    require(mode == "exception" && std::string(error.what()) == "signal phase failure",
            "unexpected standard exception");
    caught = true;
  }
  require(caught == (mode == "exception" || mode == "anl_exception"), "exception was lost");
  if (!caught) {
    require(status == (mode == "critical" ? AS_CRITICAL_ERROR_TO_TERMINATE : AS_OK),
            "phase status changed");
  }
  caller_signal.verify();
}

int main(int argc, char** argv)
{
  try {
    require(argc == 3, "phase and mode required");
    for (int handler_kind: {0, 1, 2}) { verify(argv[1], argv[2], handler_kind); }
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
