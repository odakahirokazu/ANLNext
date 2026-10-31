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
  add_value_element("ID", &mod_class::buffer_id_);
  add_value_element("type", &mod_class::buffer_type_);
  add_value_element("x", &mod_class::buffer_x_);
  add_value_element("y", &mod_class::buffer_y_);
  
  return AS_OK;
}

ANLStatus MyMapModule::mod_analyze()
{
  return AS_OK;
}
