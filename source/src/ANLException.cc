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
#include "VModuleParameter.hh"

namespace anlnext
{

int ANLException::__VerboseLevel__ = 1;

void ANLException::SetVerboseLevel(int v)
{
  __VerboseLevel__ = v;
}

int ANLException::VerboseLevel()
{
  return __VerboseLevel__;
}

ANLException::ANLException()
{
  *this << ExceptionTreatment(Treatment::finalize);
}

ANLException::ANLException(const BasicModule* mod)
{
  *this << ExceptionTreatment(Treatment::finalize);
  *this << ErrorInfoOnModuleID(mod->module_id());
  *this << ErrorInfoOnModuleName(mod->module_name());
}

ANLException::ANLException(const std::string& message)
{
  *this << ExceptionTreatment(Treatment::finalize);
  *this << ErrorMessage(message);
}

ANLException::ANLException(const BasicModule* mod,
                           const std::string& message)
{
  *this << ErrorInfoOnModuleID(mod->module_id());
  *this << ErrorInfoOnModuleName(mod->module_name());
  *this << ErrorMessage(message);
}

const ANLException& ANLException::set_message(const std::string& message) const
{
  *this << ErrorMessage(message);
  return *this;
}

const ANLException& ANLException::append_message(const std::string& message) const
{
  std::ostringstream oss;
  oss << get_message() << "\n" << message;
  *this << ErrorMessage(oss.str());
  return *this;
}

const ANLException& ANLException::prepend_message(const std::string& message) const
{
  std::ostringstream oss;
  oss << message << "\n" << get_message();
  *this << ErrorMessage(oss.str());
  return *this;
}

const ANLException& ANLException::request_treatment(ANLException::Treatment t) const
{
  *this << ExceptionTreatment(t);
  return *this;
}

std::string ANLException::get_message() const
{
  const std::string* message = boost::get_error_info<ErrorMessage>(*this);
  if (message==nullptr) {
    return std::string();
  }
  return *message;
}

const ANLException& ANLException::set_module_info(const BasicModule* mod) const
{
  *this << ErrorInfoOnModuleID(mod->module_id());
  *this << ErrorInfoOnModuleName(mod->module_name());
  return *this;
}

std::string ANLException::to_string() const
{
  if (VerboseLevel() == 1) {
    const std::string* message    = boost::get_error_info<ErrorMessage>(*this);
    const std::string* moduleID   = boost::get_error_info<ErrorInfoOnModuleID>(*this);
    const std::string* moduleName = boost::get_error_info<ErrorInfoOnModuleName>(*this);
    const std::string* method     = boost::get_error_info<ErrorInfoOnMethod>(*this);
    const int64_t* loopIndex      = boost::get_error_info<ErrorInfoOnLoopIndex>(*this);
    const int* chainID            = boost::get_error_info<ErrorInfoOnChainID>(*this);
    const std::string* parameter  = boost::get_error_info<ErrorInfoOnParameter>(*this);

    std::ostringstream oss;
    if (message)    { oss << *message << "\n"; }
    if (message)    { oss << "<Error information>\n"; }
    if (chainID)    { oss << "Chain ID: " << *chainID << "\n"; }
    if (moduleID)   { oss << "Module ID: " << *moduleID << "\n"; }
    if (moduleName) { oss << "Module Name: " << *moduleName << "\n"; }
    if (method)     { oss << "Method: " << *method << "\n"; }
    if (loopIndex)  { oss << "Loop index: " << *loopIndex << "\n"; }
    if (parameter)  { oss << "Parameter:" << *parameter << "\n"; }
    return oss.str();
  }
  else if (VerboseLevel() >= 2) {
    return boost::diagnostic_information(*this);
  }

  return "";
}

ModuleCloningError::ModuleCloningError(const BasicModule* mod)
  : ANLException(mod)
{
  set_message("<ModuleCloningError>");
  append_message((boost::format("Module %s (ID: %s) can not be cloned.")
                  % mod->module_name()
                  % mod->module_id()).str());
}

ModuleAccessError::ModuleAccessError(const std::string& message, const std::string& module_key)
{
  set_message("<ModuleAccessError>");
  append_message(message+": "+module_key);
}

ParameterNotFoundError::ParameterNotFoundError(const BasicModule* mod, const std::string& name)
  : ANLException(mod)
{
  set_message("<ParameterNotFound>");
  append_message((boost::format("Parameter is not found: %s / %s")
                  % mod->module_id()
                  % name).str());
}

ParameterError::ParameterError(const VModuleParameter* param, const std::string& message)
{
  *this << ErrorInfoOnParameter(param->name());
  set_message("<ParameterError>");
  append_message((boost::format("Error in parameter: %s [%s]")
                  % param->name()
                  % param->type_name()).str());
  append_message(message);
}

ParameterInputError::ParameterInputError(const VModuleParameter* param)
  : ParameterError(param, "<ParameterInputError>")
{
  append_message("Invalid input!");
}

ParameterTypeError::ParameterTypeError(const VModuleParameter* param,
                                       const std::string& type_tried)
  : ParameterError(param, "<ParameterTypeError>")
{
  append_message((boost::format("Type does not match: %s [%s] => [%s]")
                  % param->name()
                  % param->type_name()
                  % type_tried).str());
}

ParameterTypeError::ParameterTypeError(const VModuleParameter* param,
                                       const std::string& type_tried,
                                       const std::string& value_tried)
  : ParameterError(param, "<ParameterTypeError>")
{
  append_message((boost::format("Type does not match in assignment: %s [%s] <= %s [%s]")
                  % param->name()
                  % param->type_name()
                  % value_tried
                  % type_tried).str());
}

} /* namespace anlnext */
