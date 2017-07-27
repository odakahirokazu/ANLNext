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

/**
 * This program provide similar functions (but renamed) as the ANL CLI.
 * @file CLIUtility.hh
 * @author Hirokazu Odaka
 * @date 2017-07-20 | update
 */

#ifndef ANLNEXT_CLIUtility_H
#define ANLNEXT_CLIUtility_H 1

#include <vector>
#include <string>

#if ANLNEXT_USE_READLINE
#include <atomic>
#endif /* ANLNEXT_USE_READLINE */

#ifdef ANLNEXT_USE_TVECTOR
#include "TVector2.h"
#include "TVector3.h"
#endif /* ANLNEXT_USE_TVECTOR */

#ifdef ANLNEXT_USE_HEPVECTOR
#include "CLHEP/Vector/TwoVector.h"
#include "CLHEP/Vector/ThreeVector.h"
#endif /* ANLNEXT_USE_HEPVECTOR */

#include "ModuleParameter.hh"

namespace anlnext
{

#if ANLNEXT_USE_READLINE
class ReadLine
{
public:
  ReadLine();
  ~ReadLine();

  int read(const char* prompt, bool history=true, bool trim_right=true);
  int load(const char* prompt) { return read(prompt, false, false); }

  std::string str() const { return std::string(line_); }
  const char* c_str() const { return line_; }

  void set_completion_candidates(const std::vector<std::string>& keys);

private:
  char* line_ = nullptr;

private:
  ReadLine(const ReadLine&) = delete;
  ReadLine(ReadLine&&) = delete;
  ReadLine& operator=(const ReadLine&) = delete;
  ReadLine& operator=(ReadLine&&) = delete;  
};
#endif /* ANLNEXT_USE_READLINE */

/**
 * a function for command line read.
 * @param[in] prompt a prompt message
 * @param[out] ptr a pointer to a output variable
 */  
template <typename T>
void cli_read(const std::string& prompt, T* ptr);

/**
 * a function for command line read.
 * @param[in] prompt a prompt message
 * @param[out] ptr a pointer to a output variable
 * @param[in] unit unit
 * @param[in] unit_name name of the unit. This is used for the asking prompt.
 */
template <typename T>
void cli_read(const std::string& prompt, T* ptr,
              double unit, const std::string& unit_name);

/**
 * a function for command line read.
 * @param[in] prompt a prompt message
 * @param[out] ptr a pointer to a output variable
 * @param[in] expression input expression. e.g. hex.
 */
template <typename T>
void cli_read(const std::string& prompt, T* ptr,
              const std::string& expression);

/**
 * a function for sequential command line read for a vector/list.
 * @param[in] prompt a prompt message
 * @param[out] ptr a pointer to a output variable
 * @param[in] default_value default value of each element of the vector/list.
 */
template <typename T>
void cli_read_seq(const std::string& prompt, T* ptr,
                  const std::string& default_value="");

/* implementation */

template<typename T>
void cli_read(const std::string& prompt, T* ptr)
{
  ModuleParameter<T> param(prompt, ptr);
  param.ask();
}

template<typename T>
void cli_read(const std::string& prompt, T* ptr,
              double unit, const std::string& unit_name)
{
  ModuleParameter<T> param(prompt, ptr);
  param.set_unit(unit, unit_name);
  param.ask();
}

template<typename T>
void cli_read(const std::string& prompt, T* ptr, const std::string& expression)
  
{
  ModuleParameter<T> param(prompt, ptr);
  param.set_expression(expression);
  param.ask();
}

template<typename T>
void cli_read_seq(const std::string& prompt, T* ptr,
                  const std::string& default_value) 
{
  ModuleParameter<T> param(prompt, ptr);
  param.set_expression("seq");
  param.set_default_string(default_value);
  param.ask();
}

} /* namespace anlnext */

#endif /* ANLNEXT_CLIUtility_H */
