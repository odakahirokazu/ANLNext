#include "MyModule.hh"

using namespace anl;

MyModule::MyModule()
  : myParameter1_(1), myParameter2_(2.0), myParameter3_("test")
{
}

MyModule::~MyModule() = default;

ANLStatus MyModule::mod_define()
{
  register_parameter(&myParameter1_, "MyParameter1");
  register_parameter(&myParameter2_, "MyParameter2");
  register_parameter(&myParameter3_, "MyParameter3");
  register_parameter(&myVector1_, "MyVector1");
  register_parameter(&myVector2_, "MyVector2");
  register_parameter(&myVector3_, "MyVector3");
  return AS_OK;
}

ANLStatus MyModule::mod_pre_initialize()
{
  return AS_OK;
}

ANLStatus MyModule::mod_initialize()
{
  return AS_OK;
}

ANLStatus MyModule::mod_begin_run()
{
  return AS_OK;
}

ANLStatus MyModule::mod_analyze()
{
  return AS_OK;
}

ANLStatus MyModule::mod_end_run()
{
  return AS_OK;
}

ANLStatus MyModule::mod_finalize()
{
  return AS_OK;
}
