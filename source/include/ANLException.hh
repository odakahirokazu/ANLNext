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

#ifndef ANLNEXT_ANLException_H
#define ANLNEXT_ANLException_H 1

#include <stdexcept>
#include <cstdint>
#include <string>
#include <boost/exception/all.hpp>

namespace anlnext
{

using ErrorMessage          = boost::error_info<struct tag_ErrorMessage, std::string>;
using ErrorInfoOnModuleID   = boost::error_info<struct tag_ErrorModuleID, std::string>;
using ErrorInfoOnModuleName = boost::error_info<struct tag_ErrorModuleName, std::string>;
using ErrorInfoOnMethod     = boost::error_info<struct tag_ErrorMethod, std::string>;
using ErrorInfoOnLoopIndex  = boost::error_info<struct tag_ErrorLoopIndex, int64_t>;
using ErrorInfoOnChainID    = boost::error_info<struct tag_ErrorChainID, int>;
using ErrorInfoOnParameter  = boost::error_info<struct tag_ErrorParameter, std::string>;

class BasicModule;
class VModuleParameter;

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
 * @date 2017-07-24 | ANL exception class system is redesigned
 */
struct ANLException : virtual exception_base
{
public:
  enum class Treatment { rethrow, finalize, terminate, hard_terminate };

public:
  static void SetVerboseLevel(int v);
  static int VerboseLevel();

public:
  ANLException();
  explicit ANLException(const BasicModule* mod);
  explicit ANLException(const std::string& message);
  ANLException(const BasicModule* mod,
               const std::string& message);

  virtual ~ANLException() = default;

  /* apply const specifiers similarly to operator<<(const boost::exception&, v) */
  const ANLException& set_module_info(const BasicModule* module) const;
  const ANLException& set_message(const std::string& message) const;
  const ANLException& append_message(const std::string& message) const;
  const ANLException& prepend_message(const std::string& message) const;

  const ANLException& request_treatment(Treatment t) const;

  std::string get_message() const;
  std::string to_string() const;

private:
  static int __VerboseLevel__;
};

using ExceptionTreatment = boost::error_info<struct tag_ExceptionTreatment, ANLException::Treatment>;

struct ModuleCloningError : anlnext::ANLException
{
  explicit ModuleCloningError(const BasicModule* mod);
};

struct ModuleAccessError : anlnext::ANLException
{
  explicit ModuleAccessError(const std::string& message, const std::string& module_key);
};

struct ParameterNotFoundError : anlnext::ANLException
{
  ParameterNotFoundError(const BasicModule* mod, const std::string& name);
};

struct ParameterError : anlnext::ANLException
{
  ParameterError(const VModuleParameter* param, const std::string& message);
};

struct ParameterInputError : ParameterError
{
  ParameterInputError(const VModuleParameter* param);
};

struct ParameterTypeError : ParameterError
{
  ParameterTypeError(const VModuleParameter* param,
                     const std::string& type_tried);
  ParameterTypeError(const VModuleParameter* param,
                     const std::string& type_tried,
                     const std::string& value_tried);
};

} /* namespace anlnext */

#endif /* ANLNEXT_ANLException_H */
