#include "MyVectorModule.hh"

using namespace anlnext;

MyVectorModule::MyVectorModule()
{
}

MyVectorModule::~MyVectorModule() = default;

ANLStatus MyVectorModule::mod_define()
{
  define_parameter("my_vector", &mod_class::my_vector_);
  add_value_element("ID", 0);
  add_value_element("type", std::string("pixel"));
  add_value_element("x", 0.0);
  add_value_element("y", 0.0);
  
  return AS_OK;
}

ANLStatus MyVectorModule::mod_analyze()
{
  return AS_OK;
}
