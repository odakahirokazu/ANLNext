#include "MapExample.hh"
using namespace anlnext;

ANLStatus MapExample::mod_define()
{
  define_parameter("map_i_i", &MapExample::map_i_i);
  define_parameter("map_i_d", &MapExample::map_i_d, 10.0, "test_unit");
  define_parameter("map_i_str", &MapExample::map_i_str);
  define_parameter("map_str_i", &MapExample::map_str_i);
  define_parameter("map_str_d", &MapExample::map_str_d, 10.0, "test_unit");
  define_parameter("map_str_str", &MapExample::map_str_str);
  define_parameter("vector_i", &MapExample::vector_i);
  define_parameter("vector_d", &MapExample::vector_d, 10.0, "test_unit");
  define_parameter("vector_str", &MapExample::vector_str);
  define_parameter("table", &MapExample::table);
  define_map_key("key");
  add_value_element("count", 0);
  add_value_element("weight", 0.0);
  define_parameter("rows", &MapExample::rows);
  add_value_element("count", 0);
  define_parameter("flag_table", &MapExample::flag_table);
  add_value_element("count", 0);
  add_value_element("enabled", false);
  define_parameter("flags", &MapExample::flags);
  add_value_element("count", 0);
  add_value_element("enabled", false);
  return AS_OK;
}
