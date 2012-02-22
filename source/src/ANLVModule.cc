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

#include "ANLVModule.hh"

#include <boost/lexical_cast.hpp>

#include "EvsManager.hh"
#include "ANLNext.hh"

using namespace anl;

int ANLVModule::m_CopyID = 0;


ANLVModule::ANLVModule(const std::string& name, const std::string& version)
  : m_ModuleName(name), m_ModuleVersion(version),
    m_ModuleDescription(""),
    m_ModuleOn(true), m_Evs(0), m_CurrentParameter(),
    m_RequireFullAccess(true)
{
  m_MyCopyID = m_CopyID;
  m_CopyID++;
}


ANLVModule::ANLVModule(const ANLVModule& r)
{
  m_MyCopyID = m_CopyID;
  
  m_ModuleName = r.m_ModuleName +
    "#" + boost::lexical_cast<std::string>(m_MyCopyID);
  m_ModuleVersion = r.m_ModuleVersion;
  m_Alias.resize(r.m_Alias.size());
  for (size_t i=0; i<m_Alias.size(); i++) {
    m_Alias[i] = r.m_Alias[i] +
      "#" + boost::lexical_cast<std::string>(m_MyCopyID);
  }
  m_ModuleOn = r.m_ModuleOn;
  m_Evs = r.m_Evs;
  m_ANLAccess = r.m_ANLAccess;
  
  m_CopyID++;
}


ANLVModule::ANLVModule(const ANLVModule& r, const std::string& name)
{
  m_MyCopyID = m_CopyID;
  
  m_ModuleName = name;
  m_ModuleVersion = r.m_ModuleVersion;
  m_Alias.resize(r.m_Alias.size());
  for (size_t i=0; i<m_Alias.size(); i++) {
    m_Alias[i] = r.m_Alias[i] +
      "#" + boost::lexical_cast<std::string>(m_MyCopyID);
  }
  m_ModuleOn = r.m_ModuleOn;
  m_Evs = r.m_Evs;
  m_ANLAccess = r.m_ANLAccess;
  
  m_CopyID++;
}


void ANLVModule::print_parameters()
{
  for (ModParamIter it=ModParamBegin(); it!=ModParamEnd(); ++it) {
    (*it)->print(std::cout);
    std::cout << std::endl;
  }
}


void ANLVModule::ask_parameters()
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


bool ANLVModule::accessible(const std::string& name) 
{
  return m_RequireFullAccess || m_AccessibleModule.count(name);
}


void ANLVModule::unregister_parameter(const std::string& name)
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


void ANLVModule::hide_parameter(const std::string& name, bool hidden)
{
  for (ModParamIter it=ModParamBegin(); it!=ModParamEnd(); ++it) {
    if (name == (*it)->name()) {
      if (!hidden) { m_CurrentParameter = *it; }
      (*it)->set_hidden(hidden);
      break;
    }
  }
}


void ANLVModule::ask_parameter(const std::string& name,
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


void ANLVModule::require_module_access(const std::string& name)
{
  m_AccessibleModule.insert(name);
}


void ANLVModule::require_full_access(bool v)
{
  m_RequireFullAccess = v;
}


void ANLVModule::EvsDef(const std::string& key)
{
  m_Evs->EvsDef(key);
}


void ANLVModule::EvsUndef(const std::string& key)
{
  m_Evs->EvsUndef(key);
}


bool ANLVModule::EvsIsDef(const std::string& key) const
{
  return m_Evs->EvsIsDef(key);
}


bool ANLVModule::Evs(const std::string& key) const
{
  return m_Evs->Evs(key);
}


void ANLVModule::EvsSet(const std::string& key)
{
  m_Evs->EvsSet(key);
}


void ANLVModule::EvsReset(const std::string& key)
{
  m_Evs->EvsReset(key);
}


// instantiation of function templates
template
void ANLVModule::set_parameter(const std::string& name, int val);

template
void ANLVModule::set_parameter(const std::string& name, double val);

template
void ANLVModule::set_parameter(const std::string& name,
                               const std::string& val);

template
void ANLVModule::set_parameter(const std::string& name,
                               const std::vector<int>& val);

template
void ANLVModule::set_parameter(const std::string& name,
                               const std::vector<double>& val);

template
void ANLVModule::set_parameter(const std::string& name,
                               const std::vector<std::string>& val);

template
void ANLVModule::set_map_value(const std::string& name, int val);

template
void ANLVModule::set_map_value(const std::string& name, double val);

template
void ANLVModule::set_map_value(const std::string& name,
                               const std::string& val);
