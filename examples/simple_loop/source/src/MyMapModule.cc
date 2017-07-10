#include "MyMapModule.hh"

using namespace anl;

MyMapModule::MyMapModule()
{
}

MyMapModule::~MyMapModule() = default;

ANLStatus MyMapModule::mod_define()
{
  define_parameter("my_map", &mod_class::myMap_);
  define_map_key("name");
  add_value_element("ID", &mod_class::bufferID_);
  add_value_element("type", &mod_class::bufferType_);
  add_value_element("x", &mod_class::bufferX_);
  add_value_element("y", &mod_class::bufferY_);
  
  return AS_OK;
}

ANLStatus MyMapModule::mod_analyze()
{
  return AS_OK;
}
