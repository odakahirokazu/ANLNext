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

#include "ANLException.hh"
#include <boost/format.hpp>
#include "BasicModule.hh"

namespace anl
{

int ANLException::__VerboseLevel__ = 1;

void ANLException::setVerboseLevel(int v)
{
  __VerboseLevel__ = v;
}

int ANLException::VerboseLevel()
{
  return __VerboseLevel__;
}

ANLException::ANLException(const BasicModule* mod)
{
  *this << ANLErrorInfoOnModule(mod->module_id());
}

ANLException::ANLException(const std::string& message)
{
  *this << ANLErrorInfo(message);
}

ANLException::ANLException(const BasicModule* mod,
                           const std::string& message)
{
  *this << ANLErrorInfoOnModule(mod->module_id());
  *this << ANLErrorInfo(message);
}

void ANLException::setMessage(const std::string& message)
{
  *this << ANLErrorInfo(message);
}

std::string ANLException::getMessage() const
{
  const std::string* message = boost::get_error_info<ANLErrorInfo>(*this);
  if (message==nullptr) {
    return std::string();
  }
  return *message;
}

void ANLException::setModuleInfo(const BasicModule* mod)
{
  *this << ANLErrorInfoOnModule(mod->module_id());
}

std::string ANLException::toString() const
{
  if (VerboseLevel() == 1) {
    std::ostringstream oss;
    const std::string* message = boost::get_error_info<ANLErrorInfo>(*this);
    const std::string* method = boost::get_error_info<ANLErrorInfoOnMethod>(*this);
    const std::string* module = boost::get_error_info<ANLErrorInfoOnModule>(*this);
    const int64_t* loopIndex = boost::get_error_info<ANLErrorInfoOnLoopIndex>(*this);
    if (message) { oss << *message << "\n"; }
    if (module) { oss << "Module ID: " << *module << "\n"; }
    if (method) { oss << "Method: " << *method << "\n"; }
    if (loopIndex) { oss << "Loop index: " << *loopIndex << "\n"; }
    return oss.str();
  }
  else if (VerboseLevel() >= 2) {
    return boost::diagnostic_information(*this);
  }

  return "";
}

} /* namespace anl */
