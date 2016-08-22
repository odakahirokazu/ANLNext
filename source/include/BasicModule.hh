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

#ifndef ANL_BasicModule_H
#define ANL_BasicModule_H 1

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <algorithm>
#include <iterator>

#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>

#include "ANLStatus.hh"
#include "ModuleParameter.hh"
#include "ANLException.hh"
#include "ModuleAccess.hh"
#include "ANLMacro.hh"

#ifdef ANL_USE_TVECTOR
#include "TVector2.h"
#include "TVector3.h"
#endif /* ANL_USE_TVECTOR */

#ifdef ANL_USE_HEPVECTOR
#include "CLHEP/Vector/TwoVector.h"
#include "CLHEP/Vector/ThreeVector.h"
#endif /* ANL_USE_HEPVECTOR */

namespace anl
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
 */
class BasicModule
{
  DEFINE_ANL_MODULE(BasicModule, 0.0);
public:
  BasicModule();
  BasicModule(const BasicModule& r);
  virtual ~BasicModule();

  std::string module_name() const { return __module_name__(); }
  std::string module_version() const { return __module_version__(); }
  
  void set_module_id(const std::string& v);
  std::string module_id() const { return (this->*moduleIDMethod_)(); }
  
  virtual ANLStatus mod_startup()  { return AS_OK; }
  virtual ANLStatus mod_com()      { ask_parameters(); return AS_OK; }
  virtual ANLStatus mod_prepare()  { return AS_OK; }
  virtual ANLStatus mod_init()     { return AS_OK; }
  virtual ANLStatus mod_his()      { return AS_OK; }
  virtual ANLStatus mod_bgnrun()   { return AS_OK; }
  virtual ANLStatus mod_ana()      { return AS_OK; }
  virtual ANLStatus mod_endrun()   { return AS_OK; }
  virtual ANLStatus mod_exit()     { return AS_OK; }

  std::vector<std::string> get_alias() const { return aliases_; }
  void add_alias(const std::string& name) { aliases_.push_back(name); }
  int copy_id() { return myCopyID_; }

  std::string module_description() const { return moduleDescription_; }
  void set_module_description(const std::string& v) { moduleDescription_ = v; }

  void set_evs_manager(EvsManager* man) { evsManager_ = man; }
  void set_anl_access(const ModuleAccess& aa) { moduleAccess_ = aa; }
  bool accessible(const std::string& name);

  /**
   * enable this module.
   */
  void on() { moduleOn_ = true; }
  
  /**
   * disable this module.
   */
  void off() { moduleOn_ = false; }

  /**
   * @return true if this module is on.
   */
  bool is_on() const { return moduleOn_; }

  /**
   * @return true if this module is off.
   */
  bool is_off() const { return !moduleOn_; }

  void set_event_loop_index(long int index) { eventIndex_ = index; }
  long int get_event_loop_index() const { return eventIndex_; }
  
  /**
   * expose a module parameter specified by "name" and set it as the current parameter.
   * @param name module parameter name
   */
  void expose_parameter(const std::string& name)
  { hide_parameter(name, false); }
  
  ModuleParamConstIter parameter_begin() const
  { return std::begin(moduleParameters_); }
  ModuleParamConstIter parameter_end() const
  { return std::end(moduleParameters_); }
  ModuleParamConstIter find_parameter(const std::string& name) const throw(ANLException);
  const VModuleParameter* get_parameter(const std::string& name) const throw(ANLException)
  {
    return find_parameter(name)->get();
  }
  
  template<typename T>
  void set_parameter(const std::string& name, T val) throw(ANLException);

  void set_parameter(const std::string& name,
                     double x, double y) throw(ANLException);
  void set_parameter(const std::string& name,
                     double x, double y, double z) throw(ANLException);

  void clear_array(const std::string& name) throw(ANLException);

  void set_map_key(const std::string& key)
  {
    currentParameter_->set_map_key(key);
  }
  
  template <typename T>
  void set_value_element(const std::string& name, T val);

  void insert_to_container() { currentParameter_->insert_to_container(); }

  void print_parameters();
  void ask_parameters();

  boost::property_tree::ptree parameters_to_property_tree() const;

protected:
  template <typename T>
  void register_parameter(T* ptr, const std::string& name);

  template <typename T>
  void register_parameter(T* ptr, const std::string& name,
                          double unit, const std::string& unit_name);

  template <typename T>
  void register_parameter(T* ptr, const std::string& name,
                          const std::string& expression);

  template <typename T>
  void register_parameter(T* ptr, const std::string& name,
                          const std::string& expression,
                          const std::string& default_string);

  template <typename T>
  void register_parameter_map(T* ptr, const std::string& name,
                              const std::string& key_name,
                              const std::string& key_default);

  void set_parameter_question(const std::string& q)
  { currentParameter_->set_question(q); }
  void set_parameter_description(const std::string& q)
  { currentParameter_->set_description(q); }
  
  void unregister_parameter(const std::string& name);
  void hide_parameter(const std::string& name, bool hidden=true);
  void ask_parameter(const std::string& name, const std::string& question="");

  template <typename T>
  void add_value_element(T* ptr, const std::string& name);
  template <typename T>
  void add_value_element(T* ptr, const std::string& name,
                         double unit, const std::string& unit_name,
                         const std::string& question="");
  template <typename T>
  void add_value_element(T* ptr, const std::string& name,
                         const std::string& expression,
                         const std::string& question="");
  
  void enable_value_elements(int type, const std::vector<std::size_t>& enable)
  { currentParameter_->enable_value_elements(type, enable); }
  
  void require_module_access(const std::string& name);
  void require_full_access(bool v=true);
  
  template <typename T>
  void GetANLModule(const std::string& name, T* ptr)
  { *ptr = static_cast<T>(moduleAccess_.getModule(name)); }

  template <typename T>
  void GetANLModuleNC(const std::string& name, T* ptr)
  { *ptr = static_cast<T>(moduleAccess_.getModuleNC(name)); }

  template <typename T>
  const T* GetANLModule(const std::string& name)
  { return static_cast<const T*>(moduleAccess_.getModule(name)); }

  template <typename T>
  T* GetANLModuleNC(const std::string& name)
  { return static_cast<T*>(moduleAccess_.getModuleNC(name)); }

  template <typename T>
  void GetANLModuleIF(const std::string& name, T* ptr);

  template <typename T>
  void GetANLModuleIFNC(const std::string& name, T* ptr);

  bool ModuleExist(const std::string& name)
  { return moduleAccess_.exist(name); }
  
  void EvsDef(const std::string& key);
  void EvsUndef(const std::string& key);
  bool EvsIsDef(const std::string& key) const;
  bool Evs(const std::string& key) const;
  void EvsSet(const std::string& key);
  void EvsReset(const std::string& key);

private:
  ModuleParamIter parameter_begin()
  { return std::begin(moduleParameters_); }
  ModuleParamIter parameter_end()
  { return std::end(moduleParameters_); }
  ModuleParamIter find_parameter(const std::string& name) throw(ANLException);

  std::string get_module_id() const { return moduleID_; }
  
private:
  std::string moduleID_;
  std::vector<std::string> aliases_;
  std::string moduleDescription_;
  bool moduleOn_;
  EvsManager* evsManager_;
  ModuleAccess moduleAccess_;
  ModuleParamList moduleParameters_;
  ModuleParam_sptr currentParameter_;
  std::set<std::string> accessibleModules_;
  bool requiringFullAccess_;
  long int eventIndex_;
    
  int myCopyID_;
  static int CopyID__;

  std::string (BasicModule::*moduleIDMethod_)() const;
  
private:
  BasicModule& operator=(const BasicModule& r);
};


typedef std::vector<BasicModule*>::iterator AMIter;


template<typename T>
void BasicModule::register_parameter(T* ptr, const std::string& name)
{
  ModuleParam_sptr p(new ModuleParameter<T>(ptr, name));
  moduleParameters_.push_back(p);
  currentParameter_ = p;
}

template<typename T>
void BasicModule::register_parameter(T* ptr, const std::string& name,
                                     double unit, const std::string& unit_name)
{
  ModuleParam_sptr p(new ModuleParameter<T>(ptr, name, unit, unit_name));
  moduleParameters_.push_back(p);
  currentParameter_ = p;
}

template<typename T>
void BasicModule::register_parameter(T* ptr, const std::string& name,
                                     const std::string& expression)
{
  ModuleParam_sptr p(new ModuleParameter<T>(ptr, name, expression));
  moduleParameters_.push_back(p);
  currentParameter_ = p;
}

template<typename T>
void BasicModule::register_parameter(T* ptr, const std::string& name,
                                     const std::string& expression,
                                     const std::string& default_string)
{
  ModuleParam_sptr p(new ModuleParameter<T>(ptr, name, expression, default_string));
  moduleParameters_.push_back(p);
  currentParameter_ = p;
}

template <typename T>
void BasicModule::register_parameter_map(T* ptr, const std::string& name,
                                         const std::string& key_name,
                                         const std::string& key_default)
{
  ModuleParam_sptr p(new ModuleParameter<T>(ptr, name, key_name, key_default));
  moduleParameters_.push_back(p);
  currentParameter_ = p;
}

template <typename T>
void BasicModule::add_value_element(T* ptr, const std::string& name)
{
  ModuleParam_sptr p(new ModuleParameter<T>(ptr, name));
  currentParameter_->add_value_element(p);
}

template <typename T>
void BasicModule::add_value_element(T* ptr, const std::string& name,
                                    double unit, const std::string& unit_name,
                                    const std::string& question)
{
  ModuleParam_sptr p(new ModuleParameter<T>(ptr, name, unit, unit_name));
  p->set_question(question);
  currentParameter_->add_value_element(p);
}

template <typename T>
void BasicModule::add_value_element(T* ptr, const std::string& name,
                                    const std::string& expression,
                                    const std::string& question)
{
  ModuleParam_sptr p(new ModuleParameter<T>(ptr, name, expression));
  p->set_question(question);
  currentParameter_->add_value_element(p);
}

inline
ModuleParamIter BasicModule::find_parameter(const std::string& name) throw(ANLException)
{
  ModuleParamIter it = std::begin(moduleParameters_);
  for (; it!=std::end(moduleParameters_); ++it) {
    if ((*it)->name() == name) {
      return it;
    }
  }
  if (it == std::end(moduleParameters_)) {
    const std::string message
      = (boost::format("Parameter is not found: %s / %s") % module_id() % name).str();
    BOOST_THROW_EXCEPTION( ANLException(this, message) );
  }
  return it;
}

inline
ModuleParamConstIter BasicModule::find_parameter(const std::string& name) const throw(ANLException)
{
  ModuleParamConstIter it = std::begin(moduleParameters_);
  for (; it!=std::end(moduleParameters_); ++it) {
    if ((*it)->name() == name) {
      return it;
    }
  }
  if (it == std::end(moduleParameters_)) {
    const std::string message
      = (boost::format("Parameter is not found: %s / %s") % module_id() % name).str();
    BOOST_THROW_EXCEPTION( ANLException(this, message) );
  }
  return it;
}

template <typename T>
void BasicModule::set_parameter(const std::string& name, T val) throw(ANLException)
{
  ModuleParamIter it = find_parameter(name);
  (*it)->set_value(val);
}

inline
void BasicModule::clear_array(const std::string& name) throw(ANLException)
{
  ModuleParamIter it = find_parameter(name);
  (*it)->clear_array();
}

inline
void BasicModule::set_parameter(const std::string& name,
                                double x, double y) throw(ANLException)
{
  ModuleParamIter it = find_parameter(name);
  (*it)->set_value(x, y);
}

inline
void BasicModule::set_parameter(const std::string& name,
                                double x, double y, double z) throw(ANLException)
{
  ModuleParamIter it = find_parameter(name);
  (*it)->set_value(x, y, z);
}

template <typename T>
void BasicModule::set_value_element(const std::string& name, T val)
{
  try {
    currentParameter_->set_value_element(name, val);
  }
  catch (ANLException& e) {
    e.setModuleInfo(this);
    throw;
  }
}

template <typename T>
inline
void BasicModule::GetANLModuleIF(const std::string& name, T *ptr)
{
  *ptr = dynamic_cast<T>(moduleAccess_.getModule(name));
  if (ptr==0) {
    const std::string message
      = (boost::format("Dynamic cast failed from ANL Module: %s") % name).str();
    BOOST_THROW_EXCEPTION( ANLException(this, message) );
  }
}

template <typename T>
inline
void BasicModule::GetANLModuleIFNC(const std::string& name, T *ptr)
{
  *ptr = dynamic_cast<T>(moduleAccess_.getModuleNC(name));
  if (ptr==0) {
    const std::string message
      = (boost::format("Dynamic cast failed from ANL Module: %s") % name).str();
    BOOST_THROW_EXCEPTION( ANLException(this, message) );
  }
}

} /* namespace anl */

#endif /* ANL_BasicModule_H */
