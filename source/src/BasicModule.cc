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
#include "ANLManager.hh"

namespace anl
{

int BasicModule::CopyID__ = 0;

BasicModule::BasicModule()
  : moduleID_(""),
    moduleDescription_(""),
    moduleOn_(true), evsManager_(0), currentParameter_(),
    requiringFullAccess_(true),
    eventIndex_(-1)
{
  myCopyID_ = CopyID__;
  CopyID__++;

  moduleIDMethod_ = &BasicModule::module_name;
}

BasicModule::BasicModule(const BasicModule& r)
  : moduleID_(r.moduleID_),
    aliases_(r.aliases_),
    moduleDescription_(r.moduleDescription_),
    moduleOn_(r.moduleOn_),
    evsManager_(r.evsManager_),
    moduleAccess_(r.moduleAccess_),
    accessibleModules_(r.accessibleModules_),
    requiringFullAccess_(true)
{
  myCopyID_ = CopyID__;
  CopyID__++;
  
  moduleID_ = r.module_id()+"#"+boost::lexical_cast<std::string>(myCopyID_);
  moduleIDMethod_ = &BasicModule::get_module_id;
}

BasicModule::~BasicModule() = default;

void BasicModule::set_module_id(const std::string& module_id)
{
  moduleID_ = module_id;
  moduleIDMethod_ = &BasicModule::get_module_id;
}

void BasicModule::print_parameters()
{
  for (ModuleParamIter it=parameter_begin(); it!=parameter_end(); ++it) {
    (*it)->print(std::cout);
    std::cout << std::endl;
  }
}

void BasicModule::ask_parameters()
{
  for (ModuleParamIter it=parameter_begin(); it!=parameter_end(); ++it) {
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
  return requiringFullAccess_ || accessibleModules_.count(name);
}

void BasicModule::unregister_parameter(const std::string& name)
{
  ModuleParamIter it=parameter_begin();
  while (it != parameter_end()) {
    if (name == (*it)->name()) {
      it = moduleParameters_.erase(it);
    }
    else {
      ++it;
    }
  }
}

void BasicModule::hide_parameter(const std::string& name, bool hidden)
{
  for (ModuleParamIter it=parameter_begin(); it!=parameter_end(); ++it) {
    if (name == (*it)->name()) {
      if (!hidden) { currentParameter_ = *it; }
      (*it)->set_hidden(hidden);
      break;
    }
  }
}

void BasicModule::ask_parameter(const std::string& name,
                               const std::string& question)
{
  for (ModuleParamIter it=parameter_begin(); it!=parameter_end(); ++it) {
    if (name == (*it)->name()) {
      currentParameter_ = *it;
      
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
  accessibleModules_.insert(name);
}

void BasicModule::require_full_access(bool v)
{
  requiringFullAccess_ = v;
}

void BasicModule::EvsDef(const std::string& key)
{
  evsManager_->define(key);
}

void BasicModule::EvsUndef(const std::string& key)
{
  evsManager_->undefine(key);
}

bool BasicModule::EvsIsDef(const std::string& key) const
{
  return evsManager_->isDefined(key);
}

bool BasicModule::Evs(const std::string& key) const
{
  return evsManager_->get(key);
}

void BasicModule::EvsSet(const std::string& key)
{
  evsManager_->set(key);
}

void BasicModule::EvsReset(const std::string& key)
{
  evsManager_->reset(key);
}

boost::property_tree::ptree BasicModule::parameters_to_property_tree() const
{
  boost::property_tree::ptree pt;
  pt.put("module_id", module_id());
  pt.put("name", module_name());
  pt.put("version", module_version());
  boost::property_tree::ptree pt_parameters;
  for (const auto& parameter: moduleParameters_) {
    pt_parameters.push_back(std::make_pair("", parameter->to_property_tree()));
  }
  pt.add_child("parameter_list", std::move(pt_parameters));
  return std::move(pt);
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
void BasicModule::set_value_element(const std::string& name, bool val);

template
void BasicModule::set_value_element(const std::string& name, int val);

template
void BasicModule::set_value_element(const std::string& name, double val);

template
void BasicModule::set_value_element(const std::string& name,
                                    const std::string& val);

} /* namespace anl */
