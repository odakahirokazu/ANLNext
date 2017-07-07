#include "MyVectorModule.hh"

using namespace anl;

MyVectorModule::MyVectorModule()
{
}

MyVectorModule::~MyVectorModule() = default;

ANLStatus MyVectorModule::mod_define()
{
  register_parameter(&myVector_, "my_vector");
  add_value_element(&bufferID_, "ID");
  add_value_element(&bufferType_, "type");
  add_value_element(&bufferX_, "x");
  add_value_element(&bufferY_, "y");
  
  return AS_OK;
}

ANLStatus MyVectorModule::mod_analyze()
{
  return AS_OK;
}
