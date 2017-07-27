#include "MyMTModule.hh"

using namespace anlnext;

MyMTModule::MyMTModule()
{
}

MyMTModule::~MyMTModule() = default;

ANLStatus MyMTModule::mod_define()
{
  define_parameter("quit_index", &mod_class::m_QuitIndex);
  define_parameter("quit_all", &mod_class::m_QuitAll);
  return AS_OK;
}

ANLStatus MyMTModule::mod_pre_initialize()
{
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
  if (get_loop_index() == m_QuitIndex) {
    if (m_QuitAll) {
      return AS_QUIT_ALL;
    }
    else {
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
