#include "MyVectorModule.hh"

using namespace anlnext;

MyVectorModule::MyVectorModule()
{
}

MyVectorModule::~MyVectorModule() = default;

ANLStatus MyVectorModule::mod_define()
{
  define_parameter("my_vector", &mod_class::myVector_);
  add_value_element("ID", &mod_class::bufferID_);
  add_value_element("type", &mod_class::bufferType_);
  add_value_element("x", &mod_class::bufferX_);
  add_value_element("y", &mod_class::bufferY_);
  
  return AS_OK;
}

ANLStatus MyVectorModule::mod_analyze()
{
  return AS_OK;
}
