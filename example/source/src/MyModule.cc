#include "MyModule.hh"

using namespace anl;

MyModule::MyModule(const std::string& module_name,
                   const std::string& module_version)
  : anl::ANLVModule(module_name, module_version)
{
}


MyModule::~MyModule()
{
}


ANLStatus MyModule::mod_startup()
{
  return AS_OK;
}


ANLStatus MyModule::mod_init()
{
  return AS_OK;
}


ANLStatus MyModule::mod_his()
{
  return AS_OK;
}


ANLStatus MyModule::mod_bgnrun()
{
  return AS_OK;
}


ANLStatus MyModule::mod_ana()
{
  return AS_OK;
}


ANLStatus MyModule::mod_endrun()
{
  return AS_OK;
}


ANLStatus MyModule::mod_exit()
{
  return AS_OK;
}
