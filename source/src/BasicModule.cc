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

#include "BasicModule.hh"

#include <boost/lexical_cast.hpp>

#include "EvsManager.hh"
#include "ANLNext.hh"

using namespace anl;

int BasicModule::m_CopyID = 0;


BasicModule::BasicModule()
  : m_ModuleID(""),
    m_ModuleDescription(""),
    m_ModuleOn(true), m_Evs(0), m_CurrentParameter(),
    m_RequireFullAccess(true)
{
  m_MyCopyID = m_CopyID;
  m_CopyID++;

  m_ModuleIDMethod = &BasicModule::module_name;
}


BasicModule::BasicModule(const BasicModule& r)
  : m_ModuleID(r.m_ModuleID),
    m_Alias(r.m_Alias),
    m_ModuleDescription(r.m_ModuleDescription),
    m_ModuleOn(r.m_ModuleOn), m_Evs(r.m_Evs), m_ANLAccess(r.m_ANLAccess),
    m_AccessibleModule(r.m_AccessibleModule), m_RequireFullAccess(true)
{
  m_MyCopyID = m_CopyID;
  m_CopyID++;
  
  m_ModuleID = r.module_id()+"#"+boost::lexical_cast<std::string>(m_MyCopyID);
  m_ModuleIDMethod = &BasicModule::get_module_id;
}


void BasicModule::set_module_id(const std::string& module_id)
{
  m_ModuleID = module_id;
  m_ModuleIDMethod = &BasicModule::get_module_id;
}


void BasicModule::print_parameters()
{
  for (ModParamIter it=ModParamBegin(); it!=ModParamEnd(); ++it) {
    (*it)->print(std::cout);
    std::cout << std::endl;
  }
}


void BasicModule::ask_parameters()
{
  for (ModParamIter it=ModParamBegin(); it!=ModParamEnd(); ++it) {
    if ((*it)->is_hidden()) continue;
    
    (*it)->ask();
    
    if (!std::cin) {
      std::cin.clear();
      std::cin.ignore(INT_MAX, '\n');
      
      ANLException ex;
      BOOST_THROW_EXCEPTION( ex <<
                             ANLErrInfo(std::string("Input error: ")
                                        + (*it)->name()) );
    }
  }
}


bool BasicModule::accessible(const std::string& name) 
{
  return m_RequireFullAccess || m_AccessibleModule.count(name);
}


void BasicModule::unregister_parameter(const std::string& name)
{
  ModParamIter it=ModParamBegin();
  while (it != ModParamEnd()) {
    if (name == (*it)->name()) {
      it = m_ModuleParameters.erase(it);
    }
    else {
      ++it;
    }
  }
}


void BasicModule::hide_parameter(const std::string& name, bool hidden)
{
  for (ModParamIter it=ModParamBegin(); it!=ModParamEnd(); ++it) {
    if (name == (*it)->name()) {
      if (!hidden) { m_CurrentParameter = *it; }
      (*it)->set_hidden(hidden);
      break;
    }
  }
}


void BasicModule::ask_parameter(const std::string& name,
                               const std::string& question)
{
  for (ModParamIter it=ModParamBegin(); it!=ModParamEnd(); ++it) {
    if (name == (*it)->name()) {
      m_CurrentParameter = *it;
      
      if (question!="") {
        (*it)->set_question(question);
      }
      (*it)->ask();

      if (!std::cin) {
        std::cin.clear();
        std::cin.ignore(INT_MAX, '\n');
        
        ANLException ex;
        BOOST_THROW_EXCEPTION( ex <<
                               ANLErrInfo(std::string("Input error: ")
                                          + (*it)->name()) );
      }
      
      break;
    }
  }
}


void BasicModule::require_module_access(const std::string& name)
{
  m_AccessibleModule.insert(name);
}


void BasicModule::require_full_access(bool v)
{
  m_RequireFullAccess = v;
}


void BasicModule::EvsDef(const std::string& key)
{
  m_Evs->EvsDef(key);
}


void BasicModule::EvsUndef(const std::string& key)
{
  m_Evs->EvsUndef(key);
}


bool BasicModule::EvsIsDef(const std::string& key) const
{
  return m_Evs->EvsIsDef(key);
}


bool BasicModule::Evs(const std::string& key) const
{
  return m_Evs->Evs(key);
}


void BasicModule::EvsSet(const std::string& key)
{
  m_Evs->EvsSet(key);
}


void BasicModule::EvsReset(const std::string& key)
{
  m_Evs->EvsReset(key);
}


// instantiation of function templates
template
void BasicModule::set_parameter(const std::string& name, bool val);

template
void BasicModule::set_parameter(const std::string& name, int val);

template
void BasicModule::set_parameter(const std::string& name, double val);

template
void BasicModule::set_parameter(const std::string& name,
                               const std::string& val);

template
void BasicModule::set_parameter(const std::string& name,
                               const std::vector<int>& val);

template
void BasicModule::set_parameter(const std::string& name,
                               const std::vector<double>& val);

template
void BasicModule::set_parameter(const std::string& name,
                               const std::vector<std::string>& val);

template
void BasicModule::set_map_value(const std::string& name, bool val);

template
void BasicModule::set_map_value(const std::string& name, int val);

template
void BasicModule::set_map_value(const std::string& name, double val);

template
void BasicModule::set_map_value(const std::string& name,
                                const std::string& val);
