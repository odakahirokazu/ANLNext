#include "MyMapModule.hh"

using namespace anlnext;

MyMapModule::MyMapModule()
{
}

MyMapModule::~MyMapModule() = default;

ANLStatus MyMapModule::mod_define()
{
  define_parameter("my_map", &mod_class::my_map_);
  define_map_key("name");
  add_value_element("ID", 0);
  add_value_element("type", std::string("pixel"));
  add_value_element("x", 0.0);
  add_value_element("y", 0.0);
  
  return AS_OK;
}

ANLStatus MyMapModule::mod_analyze()
{
  return AS_OK;
}
