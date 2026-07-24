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

#include "EvsInterface.hh"
#ifndef ANLNEXT_BasicModule_H
#define ANLNEXT_BasicModule_H 1

#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <list>
#include <memory>

#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>

#include "ParameterRegistry.hh"
#include "ModuleDescription.hh"
#include "EvsInterface.hh"
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
class BasicSubModule;

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
 * @date 2026-06-24 | seperate module registry
 * @date 2026-06-24 | new feature: sub module
 */
class BasicModule : public ParameterRegistry, public ModuleDescription, public EvsInterface
{
private:
  virtual std::string __module_name__() const
  { return "BasicModule"; }
  virtual std::string __module_version__() const
  { return "0.0"; }
  virtual std::unique_ptr<BasicModule> __clone__() const
  { return std::unique_ptr<BasicModule>(nullptr); }
  virtual std::unique_ptr<BasicModule> __duplicate__()
  { return std::unique_ptr<BasicModule>(nullptr); }
  virtual BasicModule* __this_ptr__() { return this; }

public:
  BasicModule();
  virtual ~BasicModule();

  BasicModule(BasicModule&& r) = delete;
  BasicModule& operator=(const BasicModule& r) = delete;
  BasicModule& operator=(BasicModule&& r) = delete;

  virtual std::unique_ptr<BasicModule> duplicate();

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

  boost::property_tree::ptree parameters_to_property_tree() const override;
  
protected:
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
   * Sub module
   */
#if 0
  void define_submodule(const std::string& name);
  void set_submodule(const std::string& name, BasicSubModule* submod);
  virtual void set_submodule_by_key(const std::string& name, const std::string& keyword);

  template <typename T>
  void get_submodule(const std::string& name, const T* submod) const;
  template <typename T>
  void get_submodule_NC(const std::string& name, T* submod);
#endif

protected:
  template <typename ModuleType>
  std::unique_ptr<BasicModule> help_duplication(std::unique_ptr<ModuleType>&& cloned);

  std::unique_ptr<BasicModule> clone() const;

private:
  std::string get_module_id() const { return module_ID_; }
  void add_parameter_error_info(ANLException& ex) const override final;

private:
  bool order_sensitive_ = false;
  std::string module_ID_;
  std::vector<std::pair<std::string, ModuleAccess::ConflictOption>> aliases_;
  ModuleAccess::Permission access_permission_ = ModuleAccess::Permission::full_access;
  bool module_on_ = true;
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

template <typename T>
void BasicModule::get_module_IF(const std::string& name, const T** ptr)
{
  *ptr = dynamic_cast<const T*>(module_access_->get_module(name));
  if (*ptr==0) {
    BOOST_THROW_EXCEPTION( ModuleAccessError("Dynamic cast failed -- Module", name) );
  }
}

template <typename T>
void BasicModule::get_module_IFNC(const std::string& name, T** ptr)
{
  *ptr = dynamic_cast<T*>(module_access_->get_module_NC(name));
  if (*ptr==0) {
    BOOST_THROW_EXCEPTION( ModuleAccessError("Dynamic cast failed -- Module", name) );
  }
}

template <typename T>
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
std::unique_ptr<BasicModule> BasicModule::help_duplication(std::unique_ptr<ModuleType>&& cloned)
{
  if (this->is_master()) {
    cloned->copy_parameters(*this);
    this->last_copy_ += 1;
  }
  else {
    cloned.reset();
  }
  return cloned;
}

} /* namespace anlnext */

#endif /* ANLNEXT_BasicModule_H */
