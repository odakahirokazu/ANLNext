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
#include <boost/format.hpp>

#include "EvsManager.hh"
#include "ANLManager.hh"

namespace anl
{

BasicModule::BasicModule()
  : moduleID_(""),
    moduleDescription_(""),
    moduleOn_(true),
    loopIndex_(-1),
    copyID_(0),
    lastCopy_(0)
{
  moduleIDMethod_ = &BasicModule::module_name;
}

BasicModule::~BasicModule() = default;

BasicModule::BasicModule(const BasicModule& r)
  : moduleID_(r.moduleID_),
    aliases_(r.aliases_),
    moduleDescription_(r.moduleDescription_),
    moduleOn_(r.moduleOn_),
    evsManager_(r.evsManager_),
    moduleAccess_(r.moduleAccess_),
    loopIndex_(-1),
    copyID_(r.lastCopy_+1)
{
  if (moduleID_=="") {
    moduleIDMethod_ = &BasicModule::module_name;
  }
  else {
    moduleIDMethod_ = &BasicModule::get_module_id;
  }
}

void BasicModule::set_module_id(const std::string& module_id)
{
  moduleID_ = module_id;
  moduleIDMethod_ = &BasicModule::get_module_id;
}

ANLStatus BasicModule::mod_reduce(const std::list<BasicModule*>& parallel_modules)
{
  ANLStatus status = AS_OK;
  for (BasicModule* m: parallel_modules) {
    status = mod_merge(m);
    if (status != AS_OK) { break; }
  }
  return status;
}

std::vector<std::string> BasicModule::get_aliases_string() const
{
  std::vector<std::string> v;
  for (const auto& alias: aliases_) {
    v.push_back(alias.first);
  }
  return v;
}

void BasicModule::print_parameters()
{
  for (const auto& param: moduleParameters_) {
    param->print(std::cout);
    std::cout << std::endl;
  }
}

void BasicModule::ask_parameters()
{
  for (const auto& param: moduleParameters_) {
    if (param->is_hidden()) { continue; }
    
    param->ask();
    
    if (!std::cin) {
      std::cin.clear();
      std::cin.ignore(INT_MAX, '\n');

      std::string message
        = (boost::format("Input error: %s") % (param->name())).str();
      BOOST_THROW_EXCEPTION( ANLException(this, message) );
    }
  }
}

void BasicModule::unregister_parameter(const std::string& name)
{
  ModuleParamIter it = std::begin(moduleParameters_);
  while (it != std::end(moduleParameters_)) {
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
  for (auto& param: moduleParameters_) {
    if (param->name() == name) {
      if (!hidden) { currentParameter_ = param; }
      param->set_hidden(hidden);
      break;
    }
  }
}

void BasicModule::ask_parameter(const std::string& name,
                                const std::string& question)
{
  for (const auto& param: moduleParameters_) {
    if (param->name() == name) {
      currentParameter_ = param;
      
      if (question!="") {
        param->set_question(question);
      }
      param->ask();

      if (!std::cin) {
        std::cin.clear();
        std::cin.ignore(INT_MAX, '\n');

        std::string message
          = (boost::format("Input error: %s") % (param->name())).str();
        BOOST_THROW_EXCEPTION( ANLException(this, message) );
      }
      
      break;
    }
  }
}

void BasicModule::define_evs(const std::string& key)
{
  evsManager_->define(key);
}

void BasicModule::undefine_evs(const std::string& key)
{
  evsManager_->undefine(key);
}

bool BasicModule::is_evs_defined(const std::string& key) const
{
  return evsManager_->is_defined(key);
}

bool BasicModule::evs(const std::string& key) const
{
  return evsManager_->get(key);
}

void BasicModule::set_evs(const std::string& key)
{
  evsManager_->set(key);
}

void BasicModule::reset_evs(const std::string& key)
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
  return pt;
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
