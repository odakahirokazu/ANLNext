#include "ANLManager.hh"
#include "BasicModule.hh"
#include "EvsManager.hh"
#include "OrderKeeper.hh"

#include <iostream>
#include <limits>
#include <stdexcept>
#include <type_traits>

using namespace anlnext;

static_assert(std::is_same_v<decltype(BasicModule().get_loop_index()), std::size_t>);
static_assert(std::is_same_v<decltype(BasicModule().get_run_index()), std::size_t>);
static_assert(std::is_same_v<decltype(ANLManager().run_index()), std::size_t>);
static_assert(BasicModule::undefined_run_index == ANLManager::undefined_run_index);

void require(bool condition, const char* message)
{
  if (!condition) { throw std::runtime_error(message); }
}

class IndexModule : public BasicModule {
  DEFINE_ANL_MODULE(IndexModule, 1.0);
  ENABLE_PARALLEL_RUN();
public:
  std::size_t expected = 0;
  bool fail = false;
  int calls = 0;

  ANLStatus mod_analyze() override
  {
    require(get_loop_index() == expected, "module received a different loop index");
    ++calls;
    if (fail) { throw ANLException("index failure"); }
    return AS_OK;
  }
};

void verify(std::size_t loop_index, bool parallel, bool fail)
{
  IndexModule module;
  require(module.get_loop_index() == BasicModule::undefined_loop_index,
          "new module already has a loop index");
  module.expected = loop_index;
  module.fail = fail;
  std::vector<BasicModule*> modules{&module};
  std::vector<LoopCounter> counters(1);
  std::vector<std::unique_ptr<OrderKeeper>> keepers(1);
  EvsManager evs;
  bool caught = false;
  try {
    const auto status = parallel
      ? process_one_iteration(loop_index, modules, counters, evs, keepers)
      : process_one_iteration(loop_index, modules, counters, evs);
    require(status == AS_OK, "iteration failed");
  }
  catch (const ANLException& error) {
    require(fail, "unexpected exception");
    const auto* reported_index = boost::get_error_info<ErrorInfoOnLoopIndex>(error);
    require(reported_index && *reported_index == loop_index, "exception lost the loop index");
    require(error.to_string().find("Loop index: " + std::to_string(loop_index) + "\n") != std::string::npos,
            "exception displayed a different loop index");
    caught = true;
  }
  require(caught == fail && module.calls == 1, "unexpected iteration outcome");
  require(module.get_loop_index() == loop_index, "module did not retain the loop index");
  require(counters[0].entry() == 1 && counters[0].ok() == (fail ? 0 : 1),
          "iteration statistics changed");

  module.set_run_index(7);
  auto copy = module.duplicate();
  require(copy->get_run_index() == BasicModule::undefined_run_index, "copy retained the source run index");
  require(copy->get_loop_index() == BasicModule::undefined_loop_index,
          "copy retained the source loop index");
}

int main()
{
  try {
    const auto max_index = std::numeric_limits<std::size_t>::max();
    for (const std::size_t loop_index: {std::size_t{0}, max_index / 2 + 1, max_index - 1}) {
      for (bool parallel: {false, true}) {
        for (bool fail: {false, true}) { verify(loop_index, parallel, fail); }
      }
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
