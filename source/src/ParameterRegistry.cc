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

#include "ParameterRegistry.hh"

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

namespace anlnext
{

ParameterRegistry::ParameterRegistry()
  : current_parameter_(nullptr),
    current_value_element_(nullptr)
{
}

ParameterRegistry::~ParameterRegistry() = default;

ModuleParamIter ParameterRegistry::find_parameter(const std::string& name)
{
  ModuleParamIter it = std::begin(module_parameters_);
  for (; it!=std::end(module_parameters_); ++it) {
    if ((*it)->name() == name) {
      return it;
    }
  }
  if (it == std::end(module_parameters_)) {
    auto e = ParameterNotFoundError(name);
    add_parameter_error_info(e);
    BOOST_THROW_EXCEPTION(e);
  }
  return it;
}

ModuleParamConstIter ParameterRegistry::find_parameter(const std::string& name) const
{
  ModuleParamConstIter it = std::begin(module_parameters_);
  for (; it!=std::end(module_parameters_); ++it) {
    if ((*it)->name() == name) {
      return it;
    }
  }
  if (it == std::end(module_parameters_)) {
    auto e = ParameterNotFoundError(name);
    add_parameter_error_info(e);
    BOOST_THROW_EXCEPTION(e);
  }
  return it;
}

void ParameterRegistry::clear_array(const std::string& name)
{
  ModuleParamIter it = find_parameter(name);
  (*it)->clear_array();
}

void ParameterRegistry::copy_parameters(const ParameterRegistry& r)
{
  module_parameters_.clear();
  for (ModuleParam_sptr p: r.module_parameters_) {
    ModuleParam_sptr new_param = p->clone();
    p->set_module_pointer(this);
    module_parameters_.push_back(new_param);
  }
}

void ParameterRegistry::print_parameters() const
{
  for (const auto& param: module_parameters_) {
    if (param->is_result()) { continue; }
    param->print(std::cout);
    std::cout << std::endl;
  }
}

void ParameterRegistry::ask_parameters()
{
  for (const auto& param: module_parameters_) {
    if (param->is_hidden()) { continue; }
    if (param->is_result()) { continue; }
    param->ask();
  }
}

void ParameterRegistry::print_results() const
{
  for (const auto& param: module_parameters_) {
    if (param->is_result()) {
      param->print(std::cout);
      std::cout << std::endl;
    }
  }
}

void ParameterRegistry::undefine_parameter(const std::string& name)
{
  ModuleParamIter it = find_parameter(name);
  module_parameters_.erase(it);
}

void ParameterRegistry::hide_parameter(const std::string& name, bool hidden)
{
  ModuleParamIter it = find_parameter(name);
  if (!hidden) { current_parameter_ = *it; }
  (*it)->set_hidden(hidden);
}

void ParameterRegistry::ask_parameter(const std::string& name,
                                const std::string& question)
{
  ModuleParamIter it = find_parameter(name);
  current_parameter_ = *it;
  if (question!="") {
    (*it)->set_question(question);
  }
  (*it)->ask();
}

boost::property_tree::ptree ParameterRegistry::parameters_to_property_tree() const
{
  boost::property_tree::ptree pt_parameters;
  for (const auto& parameter: module_parameters_) {
    pt_parameters.push_back(std::make_pair("", parameter->to_property_tree()));
  }
  return pt_parameters;
}

void ParameterRegistry::set_parameter(const std::string& name, double x, double y)
{
  ModuleParamIter it = find_parameter(name);
  try {
    (*it)->set_value(x, y);
  }
  catch (ANLException& e) {
    add_parameter_error_info(e);
    throw;
  }
}

void ParameterRegistry::set_parameter_integer(const std::string& name, intmax_t val)
{
  ModuleParamIter it = find_parameter(name);
  try {
    (*it)->set_value_integer(val);
  }
  catch (ANLException& e) {
    add_parameter_error_info(e);
    throw;
  }
}

void ParameterRegistry::set_parameter(const std::string& name, double x, double y, double z)
{
  ModuleParamIter it = find_parameter(name);
  try {
    (*it)->set_value(x, y, z);
  }
  catch (ANLException& e) {
    add_parameter_error_info(e);
    throw;
  }
}

// instantiation of function templates
template
void ParameterRegistry::set_parameter(const std::string& name, bool val);

template
void ParameterRegistry::set_parameter(const std::string& name, int val);

template
void ParameterRegistry::set_parameter(const std::string& name, double val);

template
void ParameterRegistry::set_parameter(const std::string& name, const std::string& val);

template
void ParameterRegistry::set_parameter(const std::string& name, const std::vector<int>& val);

template
void ParameterRegistry::set_parameter(const std::string& name, const std::vector<double>& val);

template
void ParameterRegistry::set_parameter(const std::string& name, const std::vector<std::string>& val);

template
void ParameterRegistry::set_value_element(const std::string& name, bool val);

template
void ParameterRegistry::set_value_element(const std::string& name, int val);

template
void ParameterRegistry::set_value_element(const std::string& name, double val);

template
void ParameterRegistry::set_value_element(const std::string& name, const std::string& val);

} /* namespace anlnext */
