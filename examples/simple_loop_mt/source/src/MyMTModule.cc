#include "MyMTModule.hh"

using namespace anl;

MyMTModule::MyMTModule()
  : myParameter1_(1), myParameter2_(2.0), myParameter3_("test")
{
}

MyMTModule::~MyMTModule() = default;

ANLStatus MyMTModule::mod_define()
{
  register_parameter(&myParameter1_, "MyParameter1");
  register_parameter(&myParameter2_, "MyParameter2");
  register_parameter(&myParameter3_, "MyParameter3");
  register_parameter(&myVector1_, "MyVector1");
  register_parameter(&myVector2_, "MyVector2");
  register_parameter(&myVector3_, "MyVector3");
  return AS_OK;
}

ANLStatus MyMTModule::mod_pre_initialize()
{
  return AS_OK;
}

ANLStatus MyMTModule::mod_initialize()
{
  std::cout << module_id() << ".mod_initialize() | copy ID = " << copy_id() << std::endl;
  return AS_OK;
}

ANLStatus MyMTModule::mod_begin_run()
{
  return AS_OK;
}

ANLStatus MyMTModule::mod_analyze()
{
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
