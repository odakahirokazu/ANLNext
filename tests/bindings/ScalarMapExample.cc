#include "ScalarMapExample.hh"
using namespace anlnext;

ANLStatus ScalarMapExample::mod_define()
{
  define_parameter("map_i_i", &ScalarMapExample::map_i_i);
  define_parameter("map_i_d", &ScalarMapExample::map_i_d, 10.0, "test_unit");
  define_parameter("map_i_str", &ScalarMapExample::map_i_str);
  define_parameter("map_str_i", &ScalarMapExample::map_str_i);
  define_parameter("map_str_d", &ScalarMapExample::map_str_d, 10.0, "test_unit");
  define_parameter("map_str_str", &ScalarMapExample::map_str_str);
  define_parameter("table", &ScalarMapExample::table);
  define_map_key("key");
  add_value_element("count", 0);
  add_value_element("weight", 0.0);
  return AS_OK;
}
