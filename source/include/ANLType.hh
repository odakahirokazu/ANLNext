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

#ifndef ANL_ANLType_H
#define ANL_ANLType_H 1

#include <string>
#include <vector>

namespace anl
{
template <typename... Ts>
struct type_info
{
  static std::string name() { return ""; }
};

template <typename T, typename... Ts>
struct type_info<T, Ts...>
{
  static std::string name()
  {
    return type_info<T>::name() + ", " + type_info<Ts...>::name();
  }
};

template <>
struct type_info<bool>
{
  static std::string name() { return "bool"; }
};

template <>
struct type_info<int>
{
  static std::string name() { return "int"; }
};

template <>
struct type_info<double>
{
  static std::string name() { return "double"; }
};

template <>
struct type_info<std::string>
{
  static std::string name() { return "string"; }
};

template <>
struct type_info<std::vector<int>>
{
  static std::string name() { return "vector<int>"; }
};

template <>
struct type_info<std::vector<double>>
{
  static std::string name() { return "vector<double>"; }
};

template <>
struct type_info<std::vector<std::string>>
{
  static std::string name() { return "vector<string>"; }
};

} /* namespace anl */

#endif /* ANL_ANLType_H */
