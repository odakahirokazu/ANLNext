#ifndef ANLType_hh
#define ANLType_hh

#include <string>
#include <vector>

namespace anl
{
template <typename T>
struct type_info
{
  static std::string name() { return ""; }
};

template <>
struct type_info<int>
{
  static std::string name() { return "int"; }
};

template <>
struct type_info<double>
{
  static std::string name() { return "float"; }
};

template <>
struct type_info<std::string>
{
  static std::string name() { return "string"; }
};

template <>
struct type_info<std::vector<int> >
{
  static std::string name() { return "vector of int"; }
};

template <>
struct type_info<std::vector<double> >
{
  static std::string name() { return "vector of float"; }
};

template <>
struct type_info<std::vector<std::string> >
{
  static std::string name() { return "vector of string"; }
};

}

#endif // ANLType_hh
