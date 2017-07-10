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

#ifndef ANL_ANLException_H
#define ANL_ANLException_H 1

#include <stdexcept>
#include <cstdint>
#include <string>
#include <boost/exception/all.hpp>

namespace anl
{

using ANLErrorInfo = boost::error_info<struct tag_ANLError, std::string>;
using ANLErrorInfoOnModule = boost::error_info<struct tag_ANLError_Module, std::string>;
using ANLErrorInfoOnMethod = boost::error_info<struct tag_ANLError_Method, std::string>;
using ANLErrorInfoOnLoopIndex = boost::error_info<struct tag_ANLError_LoopIndex, int64_t>;

class BasicModule;

struct exception_base : virtual std::exception, virtual boost::exception
{
};

/**
 * Exception class for the ANL Next framework.
 * 
 * @author Hirokazu Odaka
 * @date 2010-06-xx
 * @date 2016-08-19 | modify design
 * @date 2017-07-07 | rename methods
 */
struct ANLException : virtual exception_base
{
public:
  static void SetVerboseLevel(int v);
  static int VerboseLevel();

public:
  ANLException() = default;
  explicit ANLException(const BasicModule* mod);
  explicit ANLException(const std::string& message);
  ANLException(const BasicModule* mod,
               const std::string& message);

  void set_module_info(const BasicModule* module);
  void set_message(const std::string& message);
  std::string get_message() const;
  std::string to_string() const;

private:
  static int __VerboseLevel__;
};

} /* namespace anl */

#endif /* ANL_ANLException_H */
