#include "MyModule.hh"

using namespace anl;

MyModule::MyModule()
  : myParameter1_(1), myParameter2_(2.0), myParameter3_("test")
{
}

MyModule::~MyModule() = default;

ANLStatus MyModule::mod_startup()
{
  register_parameter(&myParameter1_, "MyParameter1");
  register_parameter(&myParameter2_, "MyParameter2");
  register_parameter(&myParameter3_, "MyParameter3");
  register_parameter(&myVector1_, "MyVector1");
  register_parameter(&myVector2_, "MyVector2");
  register_parameter(&myVector3_, "MyVector3");
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
