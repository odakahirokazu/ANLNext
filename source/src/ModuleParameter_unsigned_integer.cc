#include "ModuleParameter.hh"
#include <charconv>
#include <limits>

namespace anlnext
{

ModuleParameter<uintmax_t>::ModuleParameter(const std::string& name, uintmax_t* ptr)
  : VModuleParameter(name), ptr_(ptr)
{}

std::shared_ptr<VModuleParameter> ModuleParameter<uintmax_t>::clone()
{ return std::shared_ptr<VModuleParameter>(new ModuleParameter(*this)); }

std::string ModuleParameter<uintmax_t>::type_name() const
{ return param_type_info<uintmax_t>::name(); }

void ModuleParameter<uintmax_t>::set_value(int value)
{ set_value_integer(value); }

void ModuleParameter<uintmax_t>::set_value(uintmax_t value)
{ set_value_unsigned_integer(value); }

void ModuleParameter<uintmax_t>::set_value_integer(intmax_t value)
{
  if (value < 0) {
    BOOST_THROW_EXCEPTION(ParameterError(this, "Unsigned integer cannot be negative."));
  }
  __ref__() = static_cast<uintmax_t>(value);
}

void ModuleParameter<uintmax_t>::set_value_unsigned_integer(uintmax_t value)
{ __ref__() = value; }

int ModuleParameter<uintmax_t>::get_value(int) const
{
  if (__ref__() > static_cast<uintmax_t>(std::numeric_limits<int>::max())) {
    BOOST_THROW_EXCEPTION(ParameterError(this, "Unsigned integer is outside the range of int."));
  }
  return static_cast<int>(__ref__());
}

uintmax_t ModuleParameter<uintmax_t>::get_value(uintmax_t) const
{ return __ref__(); }

intmax_t ModuleParameter<uintmax_t>::get_value_integer() const
{
  if (__ref__() > static_cast<uintmax_t>(std::numeric_limits<intmax_t>::max())) {
    BOOST_THROW_EXCEPTION(ParameterError(this, "Unsigned integer is outside the range of intmax_t."));
  }
  return static_cast<intmax_t>(__ref__());
}

uintmax_t ModuleParameter<uintmax_t>::get_value_unsigned_integer() const
{ return __ref__(); }

void ModuleParameter<uintmax_t>::output(std::ostream& os) const
{
  const auto flags = os.flags();
  if (expression().find("hex") != std::string::npos) { os << std::hex; }
  else if (expression().find("dec") != std::string::npos) { os << std::dec; }
  os << __ref__();
  os.flags(flags);
}

void ModuleParameter<uintmax_t>::input(std::istream& is)
{
  std::string token;
  if (!(is >> token)) { return; }
  const char* first = token.data();
  const char* last = first + token.size();
  if (first != last && *first == '+') { ++first; }
  int base = 10;
  if (expression().find("hex") != std::string::npos ||
      (expression().find("dec") == std::string::npos &&
       (is.flags() & std::ios::basefield) == std::ios::hex)) {
    base = 16;
    if (last - first >= 2 && first[0] == '0' && (first[1] == 'x' || first[1] == 'X')) {
      first += 2;
    }
  }
  uintmax_t value{};
  const auto result = std::from_chars(first, last, value, base);
  if (result.ec != std::errc{} || result.ptr != last) {
    is.setstate(std::ios::failbit);
    return;
  }
  __ref__() = value;
}

void ModuleParameter<uintmax_t>::__get__(void* ptr) const
{ *static_cast<uintmax_t*>(ptr) = __ref__(); }

void ModuleParameter<uintmax_t>::__set__(const void* ptr)
{ __ref__() = *static_cast<const uintmax_t*>(ptr); }

uintmax_t& ModuleParameter<uintmax_t>::__ref__() { return *ptr_; }

const uintmax_t& ModuleParameter<uintmax_t>::__ref__() const { return *ptr_; }

boost::property_tree::ptree ModuleParameter<uintmax_t>::to_property_tree() const
{
  boost::property_tree::ptree pt;
  pt.put("name", name());
  pt.put("type", type_name());
  pt.put("value", __ref__());
  return pt;
}

} /* namespace anlnext */
