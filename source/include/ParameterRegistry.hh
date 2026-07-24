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

#ifndef ANLNEXT_ParameterRegistry_H
#define ANLNEXT_ParameterRegistry_H 1

#include <string>
#include <vector>
#include <list>
#include <iterator>
#include <memory>

#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>

#include "ModuleParameter.hh"
#include "ANLException.hh"

#ifdef ANLNEXT_USE_TVECTOR
#include "TVector2.h"
#include "TVector3.h"
#endif /* ANLNEXT_USE_TVECTOR */

#ifdef ANLNEXT_USE_HEPVECTOR
#include "CLHEP/Vector/TwoVector.h"
#include "CLHEP/Vector/ThreeVector.h"
#endif /* ANLNEXT_USE_HEPVECTOR */

namespace anlnext
{

/**
 * A parameter registry class
 *
 * @author Hirokazu Odaka
 * @date 2026-06-24 | separate from BasicModule
 */
class ParameterRegistry
{
public:
  ParameterRegistry();
  virtual ~ParameterRegistry();

  ParameterRegistry(const ParameterRegistry& r) = default;
  ParameterRegistry(ParameterRegistry&& r) = default;
  ParameterRegistry& operator=(const ParameterRegistry& r) = default;
  ParameterRegistry& operator=(ParameterRegistry&& r) = default;

public:
  /**
   * expose a module parameter specified by "name" and set it as the current parameter.
   * @param name module parameter name
   */
  void expose_parameter(const std::string& name)
  { hide_parameter(name, false); }
  
  ModuleParamConstIter parameter_begin() const
  { return std::begin(module_parameters_); }
  ModuleParamConstIter parameter_end() const
  { return std::end(module_parameters_); }
  ModuleParamConstIter find_parameter(const std::string& name) const;
  const VModuleParameter* get_parameter(const std::string& name) const
  {
    return find_parameter(name)->get();
  }
  
  template<typename T>
  void set_parameter(const std::string& name, T val);
  void set_parameter(const std::string& name, double x, double y);
  void set_parameter(const std::string& name, double x, double y, double z);
  void set_parameter_integer(const std::string& name, intmax_t val);
  void clear_array(const std::string& name);
  void set_map_key(const std::string& key)
  {
    current_parameter_->set_map_key(key);
  }
  
  template <typename T>
  void set_value_element(const std::string& name, T val);

  void insert_to_container() { current_parameter_->insert_to_container(); }

  void print_parameters() const;
  void ask_parameters();
  void print_results() const;

  virtual boost::property_tree::ptree parameters_to_property_tree() const;

protected:
  /*
   * define-parameter methods
   */

  template <typename ModuleClass, typename T>
  void define_parameter(const std::string& name, T ModuleClass::* ptr);

  template <typename ModuleClass, typename T>
  void define_parameter(const std::string& name, T ModuleClass::* ptr, double unit, const std::string& unit_name);

  void define_map_key(const std::string& name, const std::string& default_key="")
  { current_parameter_->set_map_key_properties(name, default_key); }

  void undefine_parameter(const std::string& name);
  void hide_parameter(const std::string& name, bool hidden=true);

  void set_parameter_unit(double unit, const std::string& unit_name)
  { current_parameter_->set_unit(unit, unit_name); }

  void set_parameter_expression(const std::string& v)
  { current_parameter_->set_expression(v); }

  void set_parameter_question(const std::string& v)
  { current_parameter_->set_question(v); }

  void set_parameter_default_string(const std::string& v)
  { current_parameter_->set_default_string(v); }

  void set_parameter_description(const std::string& v)
  { current_parameter_->set_description(v); }

  template <typename ModuleClass, typename T>
  void add_value_element(const std::string& name, T ModuleClass::* ptr);

  template <typename ModuleClass, typename T>
  void add_value_element(const std::string& name, T ModuleClass::* ptr, double unit, const std::string& unit_name);

  void enable_value_elements(int type, const std::vector<std::size_t>& enable)
  { current_value_element_->enable_value_elements(type, enable); }

  void set_value_element_unit(double unit, const std::string& unit_name)
  { current_value_element_->set_unit(unit, unit_name); }

  void set_value_element_expression(const std::string& v)
  { current_value_element_->set_expression(v); }

  void set_value_element_question(const std::string& v)
  { current_value_element_->set_question(v); }

  void set_value_element_default_string(const std::string& v)
  { current_value_element_->set_default_string(v); }

  void set_value_element_description(const std::string& v)
  { current_value_element_->set_description(v); }

  void ask_parameter(const std::string& name, const std::string& question="");

  /*
   * define-parameter methods (non-member pointer) [conventional]
   */

  template <typename T>
  void register_parameter(T* ptr, const std::string& name);

  template <typename T>
  void register_parameter(T* ptr, const std::string& name, double unit, const std::string& unit_name);

  template <typename T>
  void register_parameter_map(T* ptr, const std::string& name, const std::string& key_name, const std::string& key_default);

  void unregister_parameter(const std::string& name)
  { undefine_parameter(name); }

  template <typename T>
  void add_value_element(T* ptr, const std::string& name);
  template <typename T>
  void add_value_element(T* ptr, const std::string& name, double unit, const std::string& unit_name);

  /*
   * define-result methods
   */

  template <typename ModuleClass, typename T>
  void define_result(const std::string& name, T ModuleClass::* ptr);

  template <typename ModuleClass, typename T>
  void define_result(const std::string& name, T ModuleClass::* ptr, double unit, const std::string& unit_name);

private:
  ModuleParamIter find_parameter(const std::string& name);
  void copy_parameters(const ParameterRegistry& r);
  virtual void add_parameter_error_info(ANLException&) const {}

private:
  ModuleParamList module_parameters_;
  ModuleParam_sptr current_parameter_;
  ModuleParam_sptr current_value_element_;
};

template <typename ModuleClass, typename T>
void ParameterRegistry::define_parameter(const std::string& name, T ModuleClass::* ptr)
{
  ModuleParam_sptr p(new ModuleParameterMember<ModuleClass, T>(name, dynamic_cast<ModuleClass*>(this), ptr));
  module_parameters_.push_back(p);
  current_parameter_ = p;
}

template <typename ModuleClass, typename T>
void ParameterRegistry::define_parameter(const std::string& name, T ModuleClass::* ptr, double unit, const std::string& unit_name)
{
  ModuleParam_sptr p(new ModuleParameterMember<ModuleClass, T>(name, dynamic_cast<ModuleClass*>(this), ptr));
  p->set_unit(unit, unit_name);
  module_parameters_.push_back(p);
  current_parameter_ = p;
}

template <typename ModuleClass, typename T>
void ParameterRegistry::define_result(const std::string& name, T ModuleClass::* ptr)
{
  define_parameter(name, ptr);
  current_parameter_->set_result();
}

template <typename ModuleClass, typename T>
void ParameterRegistry::define_result(const std::string& name, T ModuleClass::* ptr, double unit, const std::string& unit_name)
{
  define_parameter(name, ptr, unit, unit_name);
  current_parameter_->set_result();
}

template <typename ModuleClass, typename T>
void ParameterRegistry::add_value_element(const std::string& name, T ModuleClass::* ptr)
{
  ModuleParam_sptr p(new ModuleParameterMember<ModuleClass, T>(name, dynamic_cast<ModuleClass*>(this), ptr));
  current_parameter_->add_value_element(p);
  current_value_element_ = p;
}

template <typename ModuleClass, typename T>
void ParameterRegistry::add_value_element(const std::string& name, T ModuleClass::* ptr, double unit, const std::string& unit_name)
{
  ModuleParam_sptr p(new ModuleParameterMember<ModuleClass, T>(name, dynamic_cast<ModuleClass*>(this), ptr));
  p->set_unit(unit, unit_name);
  current_parameter_->add_value_element(p);
  current_value_element_ = p;
}

template<typename T>
void ParameterRegistry::register_parameter(T* ptr, const std::string& name)
{
  ModuleParam_sptr p(new ModuleParameter<T>(name, ptr));
  module_parameters_.push_back(p);
  current_parameter_ = p;
}

template<typename T>
void ParameterRegistry::register_parameter(T* ptr, const std::string& name, double unit, const std::string& unit_name)
{
  ModuleParam_sptr p(new ModuleParameter<T>(name, ptr));
  p->set_unit(unit, unit_name);
  module_parameters_.push_back(p);
  current_parameter_ = p;
}

template <typename T>
void ParameterRegistry::register_parameter_map(T* ptr, const std::string& name, const std::string& key_name, const std::string& key_default)
{
  ModuleParam_sptr p(new ModuleParameter<T>(name, ptr));
  p->set_map_key_properties(key_name, key_default);
  module_parameters_.push_back(p);
  current_parameter_ = p;
}

template <typename T>
void ParameterRegistry::add_value_element(T* ptr, const std::string& name)
{
  ModuleParam_sptr p(new ModuleParameter<T>(name, ptr));
  current_parameter_->add_value_element(p);
  current_value_element_ = p;
}

template <typename T>
void ParameterRegistry::add_value_element(T* ptr, const std::string& name, double unit, const std::string& unit_name)
{
  ModuleParam_sptr p(new ModuleParameter<T>(name, ptr));
  p->set_unit(unit, unit_name);
  current_parameter_->add_value_element(p);
  current_value_element_ = p;
}

template <typename T>
void ParameterRegistry::set_parameter(const std::string& name, T val)
{
  ModuleParamIter it = find_parameter(name);
  try {
    (*it)->set_value(val);
  }
  catch (ANLException& e) {
    add_parameter_error_info(e);
    throw;
  }
}

template <typename T>
void ParameterRegistry::set_value_element(const std::string& name, T val)
{
  try {
    current_parameter_->set_value_element(name, val);
  }
  catch (ANLException& e) {
    e.prepend_parameter_name(current_parameter_.get());
    add_parameter_error_info(e);
    throw;
  }
}

} /* namespace anlnext */

#endif /* ANLNEXT_ParameterRegistry_H */
