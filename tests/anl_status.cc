#include "ANLManager.hh"
#include "BasicModule.hh"
#include "EvsManager.hh"
#include "OrderKeeper.hh"

#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace anlnext;

static_assert(AS_OK == AS_PROCEED);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
static_assert(AS_ERROR == AS_PROCEED_WITH_ERROR);
static_assert(AS_SKIP_ERROR == AS_SKIP_WITH_ERROR);
static_assert(AS_QUIT_ERROR == AS_QUIT_WITH_ERROR);
static_assert(AS_QUIT_ALL_ERROR == AS_QUIT_ALL_WITH_ERROR);
#pragma GCC diagnostic pop

void require(bool condition, const char* message)
{
  if (!condition) { throw std::runtime_error(message); }
}

struct StatusCase {
  ANLStatus status;
  const char* name;
  ANLStatus flow;
  bool normal_error;
  bool critical_error;
};

class StatusModule : public BasicModule {
  DEFINE_ANL_MODULE(StatusModule, 1.0);
public:
  explicit StatusModule(ANLStatus status) : status_(status) {}
  ANLStatus mod_analyze() override { ++calls; return status_; }
  int calls = 0;
private:
  ANLStatus status_;
};

void verify_iteration(const StatusCase& test, bool parallel)
{
  StatusModule first(test.status), next(AS_PROCEED);
  std::vector<BasicModule*> modules{&first, &next};
  std::vector<LoopCounter> counters(2);
  std::vector<std::unique_ptr<OrderKeeper>> keepers;
  for (int i=0; i<2; ++i) { keepers.push_back(std::make_unique<OrderKeeper>()); }
  EvsManager evs;
  const ANLStatus result = (parallel)
    ? (process_one_iteration(0, modules, counters, evs, keepers))
    : (process_one_iteration(0, modules, counters, evs));
  require(result == test.flow, "iteration changed control flow");
  require(first.calls == 1 && next.calls == ((test.flow == AS_PROCEED) ? 1 : 0),
          "downstream execution changed");
  require(counters[0].entry() == 1, "missing module entry");
  require(counters[0].error() == ((test.normal_error || test.critical_error) ? 1 : 0),
          "error count changed");
  require(counters[0].ok() == ((test.flow == AS_PROCEED) ? 1 : 0), "proceed count changed");
  require(counters[0].skip() == ((test.flow == AS_SKIP) ? 1 : 0), "skip count changed");
  require(counters[0].quit() == ((test.flow == AS_QUIT || test.flow == AS_QUIT_ALL) ? 1 : 0),
          "quit count changed");
}

int main()
{
  try {
    const StatusCase cases[] = {
      {AS_PROCEED, "AS_PROCEED", AS_PROCEED, false, false},
      {AS_PROCEED_WITH_ERROR, "AS_PROCEED_WITH_ERROR", AS_PROCEED, true, false},
      {AS_SKIP, "AS_SKIP", AS_SKIP, false, false},
      {AS_SKIP_WITH_ERROR, "AS_SKIP_WITH_ERROR", AS_SKIP, true, false},
      {AS_QUIT, "AS_QUIT", AS_QUIT, false, false},
      {AS_QUIT_WITH_ERROR, "AS_QUIT_WITH_ERROR", AS_QUIT, true, false},
      {AS_QUIT_ALL, "AS_QUIT_ALL", AS_QUIT_ALL, false, false},
      {AS_QUIT_ALL_WITH_ERROR, "AS_QUIT_ALL_WITH_ERROR", AS_QUIT_ALL, true, false},
      {AS_CRITICAL_ERROR_TO_FINALIZE, "AS_CRITICAL_ERROR_TO_FINALIZE",
       AS_CRITICAL_ERROR_TO_FINALIZE, false, true},
      {AS_CRITICAL_ERROR_TO_TERMINATE, "AS_CRITICAL_ERROR_TO_TERMINATE",
       AS_CRITICAL_ERROR_TO_TERMINATE, false, true},
      {ANLStatus::critical_error_to_finalize_from_exception,
       "AS_CRITICAL_ERROR_TO_FINALIZE_FROM_EXCEPTION",
       ANLStatus::critical_error_to_finalize_from_exception, false, true},
      {ANLStatus::critical_error_to_terminate_from_exception,
       "AS_CRITICAL_ERROR_TO_TERMINATE_FROM_EXCEPTION",
       ANLStatus::critical_error_to_terminate_from_exception, false, true},
    };
    int value = 0;
    for (const auto& test: cases) {
      require(static_cast<int>(test.status) == value++, "status numeric value changed");
      require(status_to_string(test.status) == test.name, "unexpected status name");
      std::ostringstream output;
      output << test.status;
      require(output.str() == test.name, "stream output differs from status name");
      require(is_normal_error(test.status) == test.normal_error, "normal error classification changed");
      require(is_critical_error(test.status) == test.critical_error, "critical classification changed");
      require(eliminate_normal_error_status(test.status) == test.flow, "normalization changed");
      for (bool parallel: {false, true}) { verify_iteration(test, parallel); }
    }
    require(status_to_string(AS_OK) == "AS_PROCEED", "formal alias must use canonical display name");
    require(status_to_string(static_cast<ANLStatus>(-1)) == "AS_UNDEFINED", "invalid status display changed");
  }
  catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
