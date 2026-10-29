/*************************************************************************
 *                                                                       *
 * Copyright (c) 2011 Hirokazu Odaka                                     *
 *                                                                       *
 * This program is free software: you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                       *
 *************************************************************************/

#ifndef ANLNEXT_ANLType_H
#define ANLNEXT_ANLType_H 1

#include <string>
#include <cstdint>
#include <vector>
#include <map>

namespace anlnext
{

template <typename T, bool b>
struct param_type_info_one
{
  static std::string name() { return "unknown"; }
};

template <typename T>
struct param_type_info_one<T, true>
{
  static std::string name() { return "integer"; }
};

template <typename... Ts>
struct param_type_info
{
  static std::string name() { return "unknowns"; }
};

template <typename T, typename... Ts>
struct param_type_info<T, Ts...>
{
  static std::string name()
  {
    return param_type_info<T>::name() + ", " + param_type_info<Ts...>::name();
  }
};

template <typename T>
struct param_type_info<T>
{
  static std::string name()
  { return param_type_info_one<T, std::is_integral<T>::value>::name(); }
};

template <>
struct param_type_info<bool>
{
  static std::string name() { return "bool"; }
};

template <>
struct param_type_info<int>
{
  static std::string name() { return "int"; }
};

template <>
struct param_type_info<uintmax_t>
{
  static std::string name() { return "unsigned_integer"; }
};

template <>
struct param_type_info<double>
{
  static std::string name() { return "double"; }
};

template <>
struct param_type_info<std::string>
{
  static std::string name() { return "string"; }
};

template <>
struct param_type_info<std::vector<int>>
{
  static std::string name() { return "vector<int>"; }
};

template <>
struct param_type_info<std::vector<double>>
{
  static std::string name() { return "vector<double>"; }
};

template <>
struct param_type_info<std::vector<std::string>>
{
  static std::string name() { return "vector<string>"; }
};

template <>
struct param_type_info<std::map<int, int>>
{
  static std::string name() { return "map<int,int>"; }
};

template <>
struct param_type_info<std::map<int, double>>
{
  static std::string name() { return "map<int,double>"; }
};

template <>
struct param_type_info<std::map<int, std::string>>
{
  static std::string name() { return "map<int,string>"; }
};

template <>
struct param_type_info<std::map<std::string, int>>
{
  static std::string name() { return "map<string,int>"; }
};

template <>
struct param_type_info<std::map<std::string, double>>
{
  static std::string name() { return "map<string,double>"; }
};

template <>
struct param_type_info<std::map<std::string, std::string>>
{
  static std::string name() { return "map<string,string>"; }
};

} /* namespace anlnext */

#endif /* ANLNEXT_ANLType_H */
