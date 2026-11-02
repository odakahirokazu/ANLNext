#include "ANLManager.hh"
#include "ANLManagerMT.hh"
#include "BasicModule.hh"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <sys/ioctl.h>
#include <unistd.h>

using namespace anlnext;

void require(bool condition, const char* message)
{
  if (!condition) { throw std::runtime_error(message); }
}

template <typename Predicate>
void wait_for(Predicate ready)
{
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
  while (!ready()) {
    require(std::chrono::steady_clock::now() < deadline, "console did not consume input");
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

class ConsoleInput {
public:
  ConsoleInput(const std::string& input, bool eof)
  {
    int descriptors[2];
    require(pipe(descriptors) == 0, "pipe failed");
    saved_stdin_ = dup(STDIN_FILENO);
    require(saved_stdin_ >= 0, "dup failed");
    require(dup2(descriptors[0], STDIN_FILENO) >= 0, "dup2 failed");
    close(descriptors[0]);
    writer_ = descriptors[1];
    require(write(writer_, input.data(), input.size()) == static_cast<ssize_t>(input.size()),
            "writing console input failed");
    if (eof) {
      close(writer_);
      writer_ = -1;
    }
  }
  ~ConsoleInput()
  {
    dup2(saved_stdin_, STDIN_FILENO);
    close(saved_stdin_);
    if (writer_ >= 0) { close(writer_); }
  }
  ConsoleInput(const ConsoleInput&) = delete;
  ConsoleInput& operator=(const ConsoleInput&) = delete;
private:
  int saved_stdin_ = -1;
  int writer_ = -1;
};

template <typename Base>
class ConsoleManager : public Base {
public:
  using Base::Base;
  bool wait_for_input = false;
  ANLRequest expected_request = ANLRequest::none;
  ANLRequest observed_request = ANLRequest::none;
protected:
  ANLStatus process_analysis() override
  {
    if (wait_for_input) {
      // Ensure partial input reached the console before analysis can finish or throw.
      wait_for([] {
        int available = 0;
        require(ioctl(STDIN_FILENO, FIONREAD, &available) == 0, "ioctl failed");
        return available == 0;
      });
    }
    if (expected_request != ANLRequest::none) {
      wait_for([this] { return this->requested_.load() == expected_request; });
      observed_request = this->requested_.load();
    }
    return Base::process_analysis();
  }
};

class ConsoleModule : public BasicModule {
  DEFINE_ANL_MODULE(ConsoleModule, 1.0);
  ENABLE_PARALLEL_RUN();
public:
  ConsoleModule(bool fail, std::atomic<int>& calls) : fail_(fail), calls_(&calls)
  { set_order_sensitive(true); }
  ANLStatus mod_analyze() override
  {
    ++*calls_;
    if (fail_) { throw std::runtime_error("console analysis failure"); }
    return AS_OK;
  }
private:
  bool fail_;
  std::atomic<int>* calls_;
};

template <typename Manager>
void verify(Manager& manager, bool fail)
{
  std::atomic<int> calls{0};
  ConsoleModule module(fail, calls);
  manager.set_modules({&module});
  require(manager.define_analysis() == AS_OK, "definition failed");
  require(manager.pre_initialize_analysis() == AS_OK, "pre-initialization failed");
  require(manager.initialize_analysis() == AS_OK, "initialization failed");
  bool caught = false;
  try { require(manager.run_analysis(3, true) == AS_OK, "analysis failed"); }
  catch (const std::runtime_error& error) {
    require(fail && std::string(error.what()) == "console analysis failure", "unexpected exception");
    caught = true;
  }
  require(caught == fail, "analysis exception did not reach caller");
  require(manager.observed_request == manager.expected_request, "console command was lost");
  if (!fail) {
    if (manager.expected_request == ANLRequest::quit) {
      require(calls < 3, "console quit did not stop loop processing");
    }
    else {
      require(calls == 3, "console disturbed loop processing");
    }
  }
  require(manager.finalize_analysis() == AS_OK, "finalization failed");
}

int main(int argc, char** argv)
{
  // Report an unjoined thread as a failed test without producing a crash dump.
  std::set_terminate([] { std::_Exit(90); });
  try {
    require(argc == 2, "one console test mode required");
    const std::string mode = argv[1];
    const bool fail = mode.find("exception") != std::string::npos;
    std::string input;
    ANLRequest request = ANLRequest::none;
    if (mode == "quit" || mode == "quit_eof") {
      input = mode == "quit" ? ".q\n" : ".q";
      request = ANLRequest::quit;
    }
    else if (mode == "index") {
      input = ".i  \r\n";
      request = ANLRequest::show_loop_index;
    }
    else if (mode == "summary") {
      input = ".s\n";
      request = ANLRequest::show_evs_summary;
    }
    else if (fail || mode.find("partial") != std::string::npos) { input = ".i"; }
    ConsoleInput console_input(input, mode == "eof" || mode == "quit_eof");
    if (mode.find("parallel") == 0) {
      ConsoleManager<ANLManagerMT> manager(3);
      manager.wait_for_input = !input.empty();
      manager.expected_request = request;
      verify(manager, fail);
    }
    else {
      ConsoleManager<ANLManager> manager;
      manager.wait_for_input = !input.empty();
      manager.expected_request = request;
      verify(manager, fail);
    }
  }
  catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
