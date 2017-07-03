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

typedef boost::error_info<struct tag_ANLError, std::string> ANLErrorInfo;
typedef boost::error_info<struct tag_ANLError_Module, std::string> ANLErrorInfoOnModule;
typedef boost::error_info<struct tag_ANLError_Method, std::string> ANLErrorInfoOnMethod;
typedef boost::error_info<struct tag_ANLError_LoopIndex, int64_t> ANLErrorInfoOnLoopIndex;

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
 */
struct ANLException : virtual exception_base
{
public:
  static void setVerboseLevel(int v);
  static int VerboseLevel();

public:
  ANLException() = default;
  explicit ANLException(const BasicModule* mod);
  explicit ANLException(const std::string& message);
  ANLException(const BasicModule* mod,
               const std::string& message);

  void setModuleInfo(const BasicModule* module);
  void setMessage(const std::string& message);
  std::string getMessage() const;
  std::string toString() const;

private:
  static int __VerboseLevel__;
};

} /* namespace anl */

#endif /* ANL_ANLException_H */
