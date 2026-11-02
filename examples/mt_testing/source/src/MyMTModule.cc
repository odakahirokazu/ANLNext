#include "MyMTModule.hh"
#include <limits>

using namespace anlnext;

MyMTModule::MyMTModule()
{
}

MyMTModule::~MyMTModule() = default;

ANLStatus MyMTModule::mod_define()
{
  define_parameter("quit_index", &mod_class::quit_index_);
  define_parameter("quit_all", &mod_class::quit_all_);
  return AS_OK;
}

ANLStatus MyMTModule::mod_pre_initialize()
{
  if (quit_index_ < 0) {
    quit_index_ = std::numeric_limits<int>().max();
  }
  
  return AS_OK;
}

ANLStatus MyMTModule::mod_initialize()
{
  std::cout << "Copy ID: " << copy_id() << " => " << module_id() << ".mod_initialize()" << std::endl;
  return AS_OK;
}

ANLStatus MyMTModule::mod_begin_run()
{
  return AS_OK;
}

ANLStatus MyMTModule::mod_analyze()
{
  const auto loop_index = get_loop_index();
  if (loop_index >= static_cast<std::size_t>(quit_index_)) {
    if (quit_all_) {
      std::cout << "loop_index = " << loop_index << " -> AS_QUIT_ALL" << std::endl;
      return AS_QUIT_ALL;
    }
    else {
      std::cout << "loop_index = " << loop_index << " -> AS_QUIT" << std::endl;
      return AS_QUIT;
    }
  }
  
  return AS_OK;
}

ANLStatus MyMTModule::mod_end_run()
{
  return AS_OK;
}

ANLStatus MyMTModule::mod_finalize()
{
  return AS_OK;
}
