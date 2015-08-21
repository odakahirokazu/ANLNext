#include "MyMapModule.hh"

using namespace anl;

MyMapModule::MyMapModule()
{
}

MyMapModule::~MyMapModule() = default;

ANLStatus MyMapModule::mod_startup()
{
  register_parameter_map(&myMap_, "my_map", "name", "Si");
  add_value_element(&bufferID_, "ID");
  add_value_element(&bufferType_, "type");
  add_value_element(&bufferX_, "x");
  add_value_element(&bufferY_, "y");
  
  return AS_OK;
}

ANLStatus MyMapModule::mod_ana()
{
  return AS_OK;
}
