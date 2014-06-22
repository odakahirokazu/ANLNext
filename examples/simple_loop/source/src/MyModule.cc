#include "MyModule.hh"

using namespace anl;

MyModule::MyModule() :
  myParameter1(1), myParameter2(2), myParameter3(3)
{
}


MyModule::~MyModule()
{
}


ANLStatus MyModule::mod_startup()
{
  register_parameter(&myParameter1, "MyParameter1");
  register_parameter(&myParameter2, "MyParameter2");
  register_parameter(&myParameter3, "MyParameter3");
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
