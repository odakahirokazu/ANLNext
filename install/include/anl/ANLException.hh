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
#include <string>
#include <boost/exception/all.hpp>

namespace anl
{

typedef boost::error_info<struct tag_ANLErr, std::string> ANLErrInfo;
typedef boost::error_info<struct tag_ANLModFn, std::string> ANLErrModFnInfo;
typedef boost::error_info<struct tag_ANLEventID, int> ANLErrEventIDInfo;

class BasicModule;

/**
 * Exception class for the ANL Next framework.
 * 
 * @author Hirokazu Odaka
 * @date 2010-06-xx
 */
struct ANLException : boost::exception, std::exception
{
  ANLException() {}
  explicit ANLException(const BasicModule* mod);
  explicit ANLException(const std::string& msg);

  void setModule(const BasicModule* mod);
  const std::string print() const
  { return diagnostic_information(*this); }
};

} /* namespace anl */

#endif /* ANL_ANLException_H */
