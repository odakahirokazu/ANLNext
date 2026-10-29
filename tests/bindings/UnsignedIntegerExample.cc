#include "UnsignedIntegerExample.hh"
#include <limits>
using namespace anlnext;

ANLStatus UnsignedIntegerExample::mod_define()
{
  define_parameter("address", &UnsignedIntegerExample::address);
  set_parameter_expression("hex");
  define_parameter("count", &UnsignedIntegerExample::count);
  define_parameter("signed_value", &UnsignedIntegerExample::signed_value);
  return AS_OK;
}

uintmax_t UnsignedIntegerExample::maximum()
{ return std::numeric_limits<uintmax_t>::max(); }

uintmax_t UnsignedIntegerExample::signed_maximum()
{ return static_cast<uintmax_t>(std::numeric_limits<intmax_t>::max()); }
