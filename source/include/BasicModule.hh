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

#ifndef ANLNEXT_BasicModule_H
#define ANLNEXT_BasicModule_H 1

#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <algorithm>
#include <iterator>
#include <memory>

#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>

#include "ANLStatus.hh"
#include "ModuleParameter.hh"
#include "ANLException.hh"
#include "ModuleAccess.hh"
#include "ANLMacro.hh"

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

class EvsManager;

/**
 * A basic class for an ANL Next module.
 *
 * @author Hirokazu Odaka
 * @date 2010-06-xx
 * @date 2010-09-18
 * @date 2013-05-22
 * @date 2014-12-18
 * @date 2015-11-10 | review parameter setter/getter methods
 * @date 2017-07-02 | do not own ModuleAccess, always fully accessible
 * @date 2017-07-07 | new model (mod-methods are renamed)
 * @date 2019-12-25 | get-result
 * @date 2023-05-10 | singleton module
 * @date 2024-09-02 | add module information in set_parameter() exception
 */
class BasicModule
{
private:
  virtual std::string __module_name__() const
  { return "BasicModule"; }
  virtual std::string __module_version__() const
  { return "0.0"; }
  virtual std::unique_ptr<BasicModule> __clone__()
  { return nullptr; }
  virtual BasicModule* __this_ptr__() { return this; }

public:
  BasicModule();
  virtual ~BasicModule();

  BasicModule(BasicModule&& r) = delete;
  BasicModule& operator=(const BasicModule& r) = delete;
  BasicModule& operator=(BasicModule&& r) = delete;

  virtual std::unique_ptr<BasicModule> clone();

protected:
  BasicModule(const BasicModule& r);

public:
  std::string module_name() const { return __module_name__(); }
  std::string module_version() const { return __module_version__(); }
  
  void set_module_id(const std::string& v);
  std::string module_id() const { return (this->*module_ID_method_)(); }

  int copy_id() const { return copy_ID_; }
  bool is_master() const { return (copy_ID_ == 0); }

  void set_order_sensitive(bool v) { order_sensitive_ = v; }
  bool is_order_sensitive() const { return order_sensitive_; }

  void set_singleton(int copyID)
  {
    singleton_ = true;
    singleton_copy_ID_ = copyID;
  }
  void unset_singleton()
  {
    singleton_ = false;
    singleton_copy_ID_ = -1;
  }

  bool is_singleton() const { return singleton_; }
  int singleton_copy_id() const { return singleton_copy_ID_; }

  void automatic_switch_for_singleton();
  
  virtual ANLStatus mod_define()         { return AS_OK; }
  virtual ANLStatus mod_pre_initialize() { return AS_OK; }
  virtual ANLStatus mod_initialize()     { return AS_OK; }
  virtual ANLStatus mod_begin_run()      { return AS_OK; }
  virtual ANLStatus mod_analyze()        { return AS_OK; }
  virtual ANLStatus mod_end_run()        { return AS_OK; }
  virtual ANLStatus mod_finalize()       { return AS_OK; }

  virtual ANLStatus mod_reduce(const std::list<BasicModule*>& parallel_modules);
  virtual ANLStatus mod_merge(const BasicModule*) { return AS_OK; }

  virtual ANLStatus mod_communicate() { ask_parameters(); return AS_OK; }

  std::vector<std::pair<std::string, ModuleAccess::ConflictOption>> get_aliases() const { return aliases_; }
  std::vector<std::string> get_aliases_string() const;
  void add_alias(const std::string& name,
                 ModuleAccess::ConflictOption conflict=ModuleAccess::ConflictOption::error)
  {
    aliases_.emplace_back(name, conflict);
  }

  std::string module_description() const { return module_description_; }
  void set_module_description(const std::string& v) { module_description_ = v; }

  void set_evs_manager(EvsManager* man) { evs_manager_ = man; }
  void set_module_access(const ModuleAccess* aa) { module_access_ = aa; }

  ModuleAccess::Permission access_permission() const
  { return access_permission_; }

  /**
   * enable this module.
   */
  void on() { module_on_ = true; }
  
  /**
   * disable this module.
   */
  void off() { module_on_ = false; }

  /**
   * @return true if this module is on.
   */
  bool is_on() const { return module_on_; }

  /**
   * @return true if this module is off.
   */
  bool is_off() const { return !module_on_; }

  void set_loop_index(long int index) { loop_index_ = index; }
  long int get_loop_index() const { return loop_index_; }
  
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
  void set_parameter(const std::string& name,
                     double x, double y);
  void set_parameter(const std::string& name,
                     double x, double y, double z);
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

  boost::property_tree::ptree parameters_to_property_tree() const;

protected:

  /*
   * define-parameter methods
   */

  template <typename ModuleClass, typename T>
  void define_parameter(const std::string& name,
                        T ModuleClass::* ptr);

  template <typename ModuleClass, typename T>
  void define_parameter(const std::string& name,
                        T ModuleClass::* ptr,
                        double unit,
                        const std::string& unit_name);

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
  void add_value_element(const std::string& name,
                         T ModuleClass::* ptr);

  template <typename ModuleClass, typename T>
  void add_value_element(const std::string& name,
                         T ModuleClass::* ptr,
                         double unit,
                         const std::string& unit_name);

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
  void register_parameter(T* ptr, const std::string& name,
                          double unit, const std::string& unit_name);

  template <typename T>
  void register_parameter_map(T* ptr, const std::string& name,
                              const std::string& key_name,
                              const std::string& key_default);

  void unregister_parameter(const std::string& name)
  { undefine_parameter(name); }

  template <typename T>
  void add_value_element(T* ptr, const std::string& name);
  template <typename T>
  void add_value_element(T* ptr, const std::string& name,
                         double unit, const std::string& unit_name);

  /*
   * define-result methods
   */

  template <typename ModuleClass, typename T>
  void define_result(const std::string& name,
                     T ModuleClass::* ptr);

  template <typename ModuleClass, typename T>
  void define_result(const std::string& name,
                     T ModuleClass::* ptr,
                     double unit,
                     const std::string& unit_name);

  /*
   * access permission
   */
  void set_access_permission(ModuleAccess::Permission v)
  { access_permission_ = v; }
  
  /*
   * get-module methods
   */

  template <typename T>
  void get_module(const std::string& name, const T** ptr)
  { *ptr = static_cast<const T*>(module_access_->get_module(name)); }

  template <typename T>
  void get_module_NC(const std::string& name, T** ptr)
  { *ptr = static_cast<T*>(module_access_->get_module_NC(name)); }

  template <typename T>
  const T* get_module(const std::string& name)
  { return static_cast<const T*>(module_access_->get_module(name)); }

  template <typename T>
  T* get_module_NC(const std::string& name)
  { return static_cast<T*>(module_access_->get_module_NC(name)); }

  template <typename T>
  void get_module_IF(const std::string& name, const T** ptr);

  template <typename T>
  void get_module_IFNC(const std::string& name, T** ptr);

  bool exist_module(const std::string& name)
  { return module_access_->exist(name); }

  template <typename T>
  void request_module(const std::string& name, const T** ptr)
  { *ptr = static_cast<const T*>(module_access_->request_module(name)); }

  template <typename T>
  void request_module_NC(const std::string& name, T** ptr)
  { *ptr = static_cast<T*>(module_access_->request_module_NC(name)); }

  template <typename T>
  const T* request_module(const std::string& name)
  { return static_cast<const T*>(module_access_->request_module(name)); }

  template <typename T>
  T* request_module_NC(const std::string& name)
  { return static_cast<T*>(module_access_->request_module_NC(name)); }

  template <typename T>
  void request_module_IF(const std::string& name, const T** ptr);

  template <typename T>
  void request_module_IFNC(const std::string& name, T** ptr);

  /*
   * access to singleton
   */
  const BasicModule* __singleton_ptr__() const { return *singleton_ptr_; }
  BasicModule* __singleton_ptr__() { return *singleton_ptr_; }

  /*
   * EVS methods
   */

  void define_evs(const std::string& key);
  void undefine_evs(const std::string& key);
  bool is_evs_defined(const std::string& key) const;
  bool evs(const std::string& key) const;
  void set_evs(const std::string& key);
  void reset_evs(const std::string& key);

protected:
  template <typename ModuleType>
  std::unique_ptr<BasicModule> make_clone(ModuleType*&& copied);

private:
  ModuleParamIter find_parameter(const std::string& name);
  std::string get_module_id() const { return module_ID_; }
  void copy_parameters(const BasicModule& r);

private:
  bool order_sensitive_ = false;
  std::string module_ID_;
  std::vector<std::pair<std::string, ModuleAccess::ConflictOption>> aliases_;
  ModuleAccess::Permission access_permission_ = ModuleAccess::Permission::full_access;
  std::string module_description_;
  bool module_on_ = true;
  EvsManager* evs_manager_ = nullptr;
  const ModuleAccess* module_access_ = nullptr;
  ModuleParamList module_parameters_;
  ModuleParam_sptr current_parameter_;
  ModuleParam_sptr current_value_element_;
  long int loop_index_ = -1;

  const int copy_ID_ = 0;
  int last_copy_ = 0;

  bool singleton_ = false;
  int singleton_copy_ID_ = 0;
  std::shared_ptr<BasicModule*> singleton_ptr_;

  std::string (BasicModule::*module_ID_method_)() const;
};

using AMIter = std::vector<BasicModule*>::iterator;
using AMConstIter = std::vector<BasicModule*>::const_iterator;

template <typename ModuleClass, typename T>
void BasicModule::define_parameter(const std::string& name, T ModuleClass::* ptr)
{
  ModuleParam_sptr p(new ModuleParameterMember<ModuleClass, T>(name, dynamic_cast<ModuleClass*>(this), ptr));
  module_parameters_.push_back(p);
  current_parameter_ = p;
}

template <typename ModuleClass, typename T>
void BasicModule::define_parameter(const std::string& name, T ModuleClass::* ptr,
                                   double unit, const std::string& unit_name)
{
  ModuleParam_sptr p(new ModuleParameterMember<ModuleClass, T>(name, dynamic_cast<ModuleClass*>(this), ptr));
  p->set_unit(unit, unit_name);
  module_parameters_.push_back(p);
  current_parameter_ = p;
}

template <typename ModuleClass, typename T>
void BasicModule::define_result(const std::string& name, T ModuleClass::* ptr)
{
  define_parameter(name, ptr);
  current_parameter_->set_result();
}

template <typename ModuleClass, typename T>
void BasicModule::define_result(const std::string& name, T ModuleClass::* ptr,
                                double unit, const std::string& unit_name)
{
  define_parameter(name, ptr, unit, unit_name);
  current_parameter_->set_result();
}

template <typename ModuleClass, typename T>
void BasicModule::add_value_element(const std::string& name,
                                    T ModuleClass::* ptr)
{
  ModuleParam_sptr p(new ModuleParameterMember<ModuleClass, T>(name, dynamic_cast<ModuleClass*>(this), ptr));
  current_parameter_->add_value_element(p);
  current_value_element_ = p;
}

template <typename ModuleClass, typename T>
void BasicModule::add_value_element(const std::string& name,
                                    T ModuleClass::* ptr,
                                    double unit,
                                    const std::string& unit_name)
{
  ModuleParam_sptr p(new ModuleParameterMember<ModuleClass, T>(name, dynamic_cast<ModuleClass*>(this), ptr));
  p->set_unit(unit, unit_name);
  current_parameter_->add_value_element(p);
  current_value_element_ = p;
}

template<typename T>
void BasicModule::register_parameter(T* ptr, const std::string& name)
{
  ModuleParam_sptr p(new ModuleParameter<T>(name, ptr));
  module_parameters_.push_back(p);
  current_parameter_ = p;
}

template<typename T>
void BasicModule::register_parameter(T* ptr, const std::string& name,
                                     double unit, const std::string& unit_name)
{
  ModuleParam_sptr p(new ModuleParameter<T>(name, ptr));
  p->set_unit(unit, unit_name);
  module_parameters_.push_back(p);
  current_parameter_ = p;
}

template <typename T>
void BasicModule::register_parameter_map(T* ptr, const std::string& name,
                                         const std::string& key_name,
                                         const std::string& key_default)
{
  ModuleParam_sptr p(new ModuleParameter<T>(name, ptr));
  p->set_map_key_properties(key_name, key_default);
  module_parameters_.push_back(p);
  current_parameter_ = p;
}

template <typename T>
void BasicModule::add_value_element(T* ptr, const std::string& name)
{
  ModuleParam_sptr p(new ModuleParameter<T>(name, ptr));
  current_parameter_->add_value_element(p);
  current_value_element_ = p;
}

template <typename T>
void BasicModule::add_value_element(T* ptr, const std::string& name,
                                    double unit, const std::string& unit_name)
{
  ModuleParam_sptr p(new ModuleParameter<T>(name, ptr));
  p->set_unit(unit, unit_name);
  current_parameter_->add_value_element(p);
  current_value_element_ = p;
}

inline
ModuleParamIter BasicModule::find_parameter(const std::string& name)
{
  ModuleParamIter it = std::begin(module_parameters_);
  for (; it!=std::end(module_parameters_); ++it) {
    if ((*it)->name() == name) {
      return it;
    }
  }
  if (it == std::end(module_parameters_)) {
    BOOST_THROW_EXCEPTION( ParameterNotFoundError(this, name) );
  }
  return it;
}

inline
ModuleParamConstIter BasicModule::find_parameter(const std::string& name) const
{
  ModuleParamConstIter it = std::begin(module_parameters_);
  for (; it!=std::end(module_parameters_); ++it) {
    if ((*it)->name() == name) {
      return it;
    }
  }
  if (it == std::end(module_parameters_)) {
    BOOST_THROW_EXCEPTION( ParameterNotFoundError(this, name) );
  }
  return it;
}

template <typename T>
void BasicModule::set_parameter(const std::string& name, T val)
{
  ModuleParamIter it = find_parameter(name);
  try {
    (*it)->set_value(val);
  }
  catch (ANLException& e) {
    e.set_module_info(this);
    throw;
  }
}

inline
void BasicModule::clear_array(const std::string& name)
{
  ModuleParamIter it = find_parameter(name);
  (*it)->clear_array();
}

inline
void BasicModule::set_parameter(const std::string& name,
                                double x, double y)
{
  ModuleParamIter it = find_parameter(name);
  try {
    (*it)->set_value(x, y);
  }
  catch (ANLException& e) {
    e.set_module_info(this);
    throw;
  }
}

inline
void BasicModule::set_parameter_integer(const std::string& name, intmax_t val)
{
  ModuleParamIter it = find_parameter(name);
  try {
    (*it)->set_value_integer(val);
  }
  catch (ANLException& e) {
    e.set_module_info(this);
    throw;
  }
}

inline
void BasicModule::set_parameter(const std::string& name,
                                double x, double y, double z)
{
  ModuleParamIter it = find_parameter(name);
  try {
    (*it)->set_value(x, y, z);
  }
  catch (ANLException& e) {
    e.set_module_info(this);
    throw;
  }
}

template <typename T>
void BasicModule::set_value_element(const std::string& name, T val)
{
  try {
    current_parameter_->set_value_element(name, val);
  }
  catch (ANLException& e) {
    e.prepend_parameter_name(current_parameter_.get());
    e.set_module_info(this);
    throw;
  }
}

template <typename T>
inline
void BasicModule::get_module_IF(const std::string& name, const T** ptr)
{
  *ptr = dynamic_cast<const T*>(module_access_->get_module(name));
  if (*ptr==0) {
    BOOST_THROW_EXCEPTION( ModuleAccessError("Dynamic cast failed -- Module", name) );
  }
}

template <typename T>
inline
void BasicModule::get_module_IFNC(const std::string& name, T** ptr)
{
  *ptr = dynamic_cast<T*>(module_access_->get_module_NC(name));
  if (*ptr==0) {
    BOOST_THROW_EXCEPTION( ModuleAccessError("Dynamic cast failed -- Module", name) );
  }
}

template <typename T>
inline
void BasicModule::request_module_IF(const std::string& name, const T** ptr)
{
  const BasicModule* m = module_access_->request_module(name);
  if (m) {
    *ptr = dynamic_cast<const T*>(m);
  }
  else {
    *ptr = m;
  }
}

template <typename T>
inline
void BasicModule::request_module_IFNC(const std::string& name, T** ptr)
{
  BasicModule* m = module_access_->request_module_NC(name);
  if (m) {
    *ptr = dynamic_cast<T*>(m);
  }
  else {
    *ptr = m;
  }
}

template <typename ModuleType>
std::unique_ptr<BasicModule> BasicModule::make_clone(ModuleType*&& copied)
{
  std::unique_ptr<BasicModule> m(copied);
  if (this->is_master()) {
    m->copy_parameters(*this);
    this->last_copy_ += 1;
  }
  else {
    m.reset(nullptr);
  }
  return m;
}

} /* namespace anlnext */

#endif /* ANLNEXT_BasicModule_H */
