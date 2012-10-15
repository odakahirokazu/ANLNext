#include "MyModule.hh"

using namespace anl;

MyModule::MyModule()
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
