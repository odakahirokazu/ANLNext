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
  : orderSensitive_(false),
    moduleID_(""),
    accessPermission_(ModuleAccess::Permission::full_access),
    moduleDescription_(""),
    moduleOn_(true),
    evsManager_(nullptr),
    moduleAccess_(nullptr),
    currentParameter_(nullptr),
    currentValueElement_(nullptr),
    loopIndex_(-1),
    copyID_(0),
    lastCopy_(0)
{
  moduleIDMethod_ = &BasicModule::module_name;
}

BasicModule::~BasicModule() = default;

BasicModule::BasicModule(const BasicModule& r)
  : orderSensitive_(r.orderSensitive_),
    moduleID_(r.moduleID_),
    aliases_(r.aliases_),
    accessPermission_(r.accessPermission_),
    moduleDescription_(r.moduleDescription_),
    moduleOn_(r.moduleOn_),
    evsManager_(nullptr),
    moduleAccess_(nullptr),
    currentParameter_(nullptr),
    currentValueElement_(nullptr),
    loopIndex_(-1),
    copyID_(r.lastCopy_+1),
    lastCopy_(0)
{
  if (moduleID_=="") {
    moduleIDMethod_ = &BasicModule::module_name;
  }
  else {
    moduleIDMethod_ = &BasicModule::get_module_id;
  }
}

std::unique_ptr<BasicModule> BasicModule::clone()
{
  std::unique_ptr<BasicModule> m(__clone__());
  if (m==nullptr) {
    BOOST_THROW_EXCEPTION( ModuleCloningError(this) );
  }
  return m;
}

void BasicModule::copy_parameters(const BasicModule& r)
{
  moduleParameters_.clear();
  for (ModuleParam_sptr p: r.moduleParameters_) {
    ModuleParam_sptr newParam = p->clone();
    p->set_module_pointer(this);
    moduleParameters_.push_back(newParam);
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

void BasicModule::print_parameters() const
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
  }
}

void BasicModule::undefine_parameter(const std::string& name)
{
  ModuleParamIter it = find_parameter(name);
  moduleParameters_.erase(it);
}

void BasicModule::hide_parameter(const std::string& name, bool hidden)
{
  ModuleParamIter it = find_parameter(name);
  if (!hidden) { currentParameter_ = *it; }
  (*it)->set_hidden(hidden);
}

void BasicModule::ask_parameter(const std::string& name,
                                const std::string& question)
{
  ModuleParamIter it = find_parameter(name);
  currentParameter_ = *it;
  if (question!="") {
    (*it)->set_question(question);
  }
  (*it)->ask();
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
