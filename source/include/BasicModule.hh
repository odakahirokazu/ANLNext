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

#include "ANLStatus.hh"
#include "ModuleParameter.hh"
#include "ANLException.hh"
#include "ANLAccess.hh"
#include "ANLMacro.hh"

#if ANL_USE_TVECTOR
#include "TVector2.h"
#include "TVector3.h"
#endif

#if ANL_USE_HEPVECTOR
#include "CLHEP/Vector/TwoVector.h"
#include "CLHEP/Vector/ThreeVector.h"
#endif

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
 */
class BasicModule
{
  DEFINE_ANL_MODULE(BasicModule, 0.0);
public:
  BasicModule();
  BasicModule(const BasicModule& r);
  virtual ~BasicModule() {}

  std::string module_name() const { return __module_name__(); }
  std::string module_version() const { return __module_version__(); }
  
  void set_module_id(const std::string& v);
  std::string module_id() const { return (this->*m_ModuleIDMethod)(); }
  
  virtual ANLStatus mod_startup()  { return AS_OK; }
  virtual ANLStatus mod_com()      { ask_parameters(); return AS_OK; }
  virtual ANLStatus mod_prepare()  { return AS_OK; }
  virtual ANLStatus mod_init()     { return AS_OK; }
  virtual ANLStatus mod_his()      { return AS_OK; }
  virtual ANLStatus mod_bgnrun()   { return AS_OK; }
  virtual ANLStatus mod_ana()      { return AS_OK; }
  virtual ANLStatus mod_endrun()   { return AS_OK; }
  virtual ANLStatus mod_exit()     { return AS_OK; }

  std::vector<std::string> get_alias() const { return m_Alias; }
  void add_alias(const std::string& name) { m_Alias.push_back(name); }
  int copy_id() { return m_MyCopyID; }

  std::string module_description() const { return m_ModuleDescription; }
  void set_module_description(const std::string& v) { m_ModuleDescription = v; }

  /**
   * enable this module.
   */
  void on() { m_ModuleOn = true; }
  
  /**
   * disable this module.
   */
  void off() { m_ModuleOn = false; }

  /**
   * @return true if this module is on.
   */
  bool is_on() const { return m_ModuleOn; }

  /**
   * @return true if this module is off.
   */
  bool is_off() const { return !m_ModuleOn; }

  /**
   * expose a module parameter specified by "name" and set it as the current parameter.
   * @param name module parameter name
   */
  void expose_parameter(const std::string& name)
  { hide_parameter(name, false); }
  
  template<typename T>
  void set_parameter(const std::string& name, T val) throw(ANLException);

  void clear_array(const std::string& name) throw(ANLException);

  void set_vector(const std::string& name,
                  double x, double y) throw(ANLException);
  void set_vector(const std::string& name,
                  double x, double y, double z) throw(ANLException);

  void set_map_key(const std::string& key)
  {
    m_CurrentParameter->set_map_key(key);
  }
  
  template <typename T>
  void set_map_value(const std::string& name, T val);

  void insert_map() { m_CurrentParameter->insert_map(); }
  
  void print_parameters();
  void ask_parameters();

  void set_evs_manager(EvsManager* man) { m_Evs = man; }
  void set_anl_access(const ANLAccess& aa) { m_ANLAccess = aa; }

  bool accessible(const std::string& name);
  
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
  { m_CurrentParameter->set_question(q); }
  void set_parameter_description(const std::string& q)
  { m_CurrentParameter->set_description(q); }
  
  void unregister_parameter(const std::string& name);
  void hide_parameter(const std::string& name, bool hidden=true);
  void ask_parameter(const std::string& name, const std::string& question="");

  template <typename T>
  void add_map_value(T* ptr, const std::string& name);
  template <typename T>
  void add_map_value(T* ptr, const std::string& name,
                     double unit, const std::string& unit_name,
                     const std::string& question="");
  template <typename T>
  void add_map_value(T* ptr, const std::string& name,
                     const std::string& expression,
                     const std::string& question="");
  
  void enable_map_value(int type, const std::vector<int>& enable)
  { m_CurrentParameter->enable_map_value(type, enable); }
  
  void require_module_access(const std::string& name);
  void require_full_access(bool v=true);
  
  template <typename T>
  void GetANLModule(const std::string& name, T *ptr)
  { *ptr = static_cast<T>(m_ANLAccess.GetModule(name)); }

  template <typename T>
  void GetANLModuleNC(const std::string& name, T *ptr)
  { *ptr = static_cast<T>(m_ANLAccess.GetModuleNC(name)); }

  template <typename T>
  const T* GetANLModule(const std::string& name)
  { return static_cast<const T*>(m_ANLAccess.GetModule(name)); }

  template <typename T>
  T* GetANLModuleNC(const std::string& name)
  { return static_cast<T*>(m_ANLAccess.GetModuleNC(name)); }

  template <typename T>
  void GetANLModuleIF(const std::string& name, T *ptr);

  template <typename T>
  void GetANLModuleIFNC(const std::string& name, T *ptr);

  bool ModuleExist(const std::string& name)
  { return m_ANLAccess.Exist(name); }
  
  void EvsDef(const std::string& key);
  void EvsUndef(const std::string& key);
  bool EvsIsDef(const std::string& key) const;

  bool Evs(const std::string& key) const;
  void EvsSet(const std::string& key);
  void EvsReset(const std::string& key);

public:
  ModParamConstIter ModParamBegin() const { return m_ModuleParameters.begin(); }
  ModParamConstIter ModParamEnd() const   { return m_ModuleParameters.end(); }
  
private:
  ModParamIter ModParamBegin() { return m_ModuleParameters.begin(); }
  ModParamIter ModParamEnd()   { return m_ModuleParameters.end(); }

  std::string get_module_id() const { return m_ModuleID; }
  
private:
  std::string m_ModuleID;
  std::vector<std::string> m_Alias;
  std::string m_ModuleDescription;
  bool m_ModuleOn;
  EvsManager* m_Evs;
  ANLAccess m_ANLAccess;
  ModParamList m_ModuleParameters;
  ModParam m_CurrentParameter;
  std::set<std::string> m_AccessibleModule;
  bool m_RequireFullAccess;
    
  int m_MyCopyID;
  static int m_CopyID;

  std::string (BasicModule::*m_ModuleIDMethod)() const;
  
private:
  BasicModule& operator=(const BasicModule& r);
};


typedef std::vector<BasicModule*>::iterator AMIter;

}

namespace anl {

template<typename T>
void BasicModule::register_parameter(T* ptr, const std::string& name)
{
  ModParam p(new ModuleParameter<T>(ptr, name));
  m_ModuleParameters.push_back(p);
  m_CurrentParameter = p;
}


template<typename T>
void BasicModule::register_parameter(T* ptr, const std::string& name,
                                     double unit, const std::string& unit_name)
{
  ModParam p(new ModuleParameter<T>(ptr, name, unit, unit_name));
  m_ModuleParameters.push_back(p);
  m_CurrentParameter = p;
}


template<typename T>
void BasicModule::register_parameter(T* ptr, const std::string& name,
                                     const std::string& expression)
{
  ModParam p(new ModuleParameter<T>(ptr, name, expression));
  m_ModuleParameters.push_back(p);
  m_CurrentParameter = p;
}


template<typename T>
void BasicModule::register_parameter(T* ptr, const std::string& name,
                                     const std::string& expression,
                                     const std::string& default_string)
{
  ModParam p(new ModuleParameter<T>(ptr, name, expression, default_string));
  m_ModuleParameters.push_back(p);
  m_CurrentParameter = p;
}


template <typename T>
void BasicModule::register_parameter_map(T* ptr, const std::string& name,
                                         const std::string& key_name,
                                         const std::string& key_default)
{
  ModParam p(new ModuleParameter<T>(ptr, name, key_name, key_default));
  m_ModuleParameters.push_back(p);
  m_CurrentParameter = p;
}


template <typename T>
void BasicModule::add_map_value(T* ptr, const std::string& name)
{
  ModParam p(new ModuleParameter<T>(ptr, name));
  m_CurrentParameter->add_map_value(p);
}

template <typename T>
void BasicModule::add_map_value(T* ptr, const std::string& name,
                                double unit, const std::string& unit_name,
                                const std::string& question)
{
  ModParam p(new ModuleParameter<T>(ptr, name, unit, unit_name));
  p->set_question(question);
  m_CurrentParameter->add_map_value(p);
}

template <typename T>
void BasicModule::add_map_value(T* ptr, const std::string& name,
                                const std::string& expression,
                                const std::string& question)
{
  ModParam p(new ModuleParameter<T>(ptr, name, expression));
  p->set_question(question);
  m_CurrentParameter->add_map_value(p);
}

template <typename T>
void BasicModule::set_parameter(const std::string& name, T val) throw(ANLException)
{
  for (ModParamIter it=ModParamBegin(); it!=ModParamEnd(); ++it) {
    if (name == (*it)->name()) {
      (*it)->set_value(val);
      return;
    }
  }
  
  BOOST_THROW_EXCEPTION( ANLException(this) <<
                         ANLErrInfo(std::string("Parameter is not found: ")
                                    +name) );
}


inline
void BasicModule::clear_array(const std::string& name) throw(ANLException)
{
  for (ModParamIter it=ModParamBegin(); it!=ModParamEnd(); ++it) {
    if (name == (*it)->name()) {
      (*it)->clear_array();
      return;
    }
  }
  
  BOOST_THROW_EXCEPTION( ANLException(this) <<
                         ANLErrInfo(std::string("Parameter is not found: ")
                                    +name) );
}


inline
void BasicModule::set_vector(const std::string& name,
                             double x, double y) throw(ANLException)
{
  for (ModParamIter it=ModParamBegin(); it!=ModParamEnd(); ++it) {
    if (name == (*it)->name()) {
      (*it)->set_value(x, y);
      return;
    }
  }
  
  BOOST_THROW_EXCEPTION( ANLException(this) <<
                         ANLErrInfo(std::string("Parameter is not found: ")
                                    +name) );
}


inline
void BasicModule::set_vector(const std::string& name,
                             double x, double y, double z) throw(ANLException)
{
  for (ModParamIter it=ModParamBegin(); it!=ModParamEnd(); ++it) {
    if (name == (*it)->name()) {
      (*it)->set_value(x, y, z);
      return;
    }
  }

  BOOST_THROW_EXCEPTION( ANLException(this) <<
                         ANLErrInfo(std::string("Parameter is not found: ")
                                    +name) );
}


template <typename T>
void BasicModule::set_map_value(const std::string& name, T val)
{
  try {
    m_CurrentParameter->set_map_value(name, val);
  }
  catch (ANLException& e) {
    e.setModule(this);
    throw;
  }
}


template <typename T>
inline
void BasicModule::GetANLModuleIF(const std::string& name, T *ptr)
{
  *ptr = dynamic_cast<T>(m_ANLAccess.GetModule(name));
  if (ptr==0) {
    BOOST_THROW_EXCEPTION( ANLException(this) <<
                           ANLErrInfo(std::string("Dynamic cast failed: ")
                                      +name) );
  }
}


template <typename T>
inline
void BasicModule::GetANLModuleIFNC(const std::string& name, T *ptr)
{
  *ptr = dynamic_cast<T>(m_ANLAccess.GetModuleNC(name));
  if (ptr==0) {
    BOOST_THROW_EXCEPTION( ANLException(this) <<
                           ANLErrInfo(std::string("Dynamic cast failed: ")
                                      +name) );
  }
}

}

#endif /* ANL_BasicModule_H */
