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

#include "ANLManager.hh"
#include "BasicSubModule.hh"

namespace anlnext
{

BasicModule::BasicModule()
  : order_sensitive_(false),
    module_ID_(""),
    access_permission_(ModuleAccess::Permission::full_access),
    module_on_(true),
    loop_index_(-1),
    copy_ID_(0),
    last_copy_(0),
    singleton_(false),
    singleton_copy_ID_(0)
{
  module_ID_method_ = &BasicModule::module_name;
  singleton_ptr_ = std::make_shared<BasicModule*>(this);
}

BasicModule::~BasicModule() = default;

BasicModule::BasicModule(const BasicModule& r)
  : ParameterRegistry(),
    ModuleDescription(r),
    EvsInterface(),
    ModuleAccessInterface(),
    order_sensitive_(r.order_sensitive_),
    module_ID_(r.module_ID_),
    aliases_(r.aliases_),
    access_permission_(r.access_permission_),
    module_on_(r.module_on_),
    loop_index_(-1),
    copy_ID_(r.last_copy_+1),
    last_copy_(0),
    singleton_(r.singleton_),
    singleton_copy_ID_(r.singleton_copy_ID_),
    singleton_ptr_(r.singleton_ptr_)
{
  if (module_ID_=="") {
    module_ID_method_ = &BasicModule::module_name;
  }
  else {
    module_ID_method_ = &BasicModule::get_module_id;
  }

  if (singleton_copy_ID_ == copy_ID_) {
    *singleton_ptr_ = this;
  }
}

std::unique_ptr<BasicModule> BasicModule::duplicate()
{
  std::unique_ptr<BasicModule> m = __duplicate__();
  if (!m) {
    BOOST_THROW_EXCEPTION( ModuleDuplicationError(this) );
  }
  return m;
}

std::unique_ptr<BasicModule> BasicModule::clone() const
{
  return __clone__();
}

void BasicModule::set_module_id(const std::string& module_id)
{
  module_ID_ = module_id;
  module_ID_method_ = &BasicModule::get_module_id;
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
  ParameterRegistry::print_parameters();
  for (const auto& entry: submodule_map_) {
    std::cout << "\n  [" << entry.first << ": ";
    if (entry.second) {
      std::cout << entry.second->submodule_name() << " / "
                << entry.second->submodule_version() << "]\n";
      entry.second->print_parameters();
    }
    else {
      std::cout << "unset]\n";
    }
  }
}

boost::property_tree::ptree BasicModule::parameters_to_property_tree() const
{
  boost::property_tree::ptree pt;
  pt.put("module_id", module_id());
  pt.put("name", module_name());
  pt.put("version", module_version());
  boost::property_tree::ptree pt_parameters = ParameterRegistry::parameters_to_property_tree();
  pt.add_child("parameter_list", std::move(pt_parameters));
  if (!submodule_map_.empty()) {
    boost::property_tree::ptree submodules;
    for (const auto& entry: submodule_map_) {
      boost::property_tree::ptree submodule;
      if (entry.second) {
        submodule = entry.second->parameters_to_property_tree();
      }
      submodule.put("registered_name", entry.first);
      submodule.put("state", entry.second ? "set" : "unset");
      submodules.push_back(std::make_pair("", std::move(submodule)));
    }
    pt.add_child("submodule_list", std::move(submodules));
  }
  return pt;
}

void BasicModule::automatic_switch_for_singleton()
{
  if (is_singleton()) {
    if (singleton_copy_id() == copy_id()) {
      on();
    }
    else {
      off();
    }
  }
}

void BasicModule::add_parameter_error_info(ANLException& ex) const
{
  ex.set_module_info(this);
}

void BasicModule::define_submodule(const std::string& name)
{
  check_submodule_configuration(name);
  if (is_submodule_defined(name)) {
    BOOST_THROW_EXCEPTION(SubModuleAlreadyDefinedError(this, name));
  }
  submodule_map_.emplace(name, nullptr);
}

bool BasicModule::is_submodule_defined(const std::string& name) const
{
  auto it = submodule_map_.find(name);
  if (it != submodule_map_.end()) {
    return true;
  }
  return false;
}

bool BasicModule::is_submodule_set(const std::string& name) const
{
  auto it = submodule_map_.find(name);
  if (it == submodule_map_.end()) {
    BOOST_THROW_EXCEPTION(SubModuleUndefinedError(this, name));
  }
  return static_cast<bool>(it->second);
}

void BasicModule::check_submodule_configuration(const std::string& name) const
{
  if (!is_master() || last_copy_ != 0) {
    BOOST_THROW_EXCEPTION(SubModuleConfigurationError(this, name));
  }
}

std::vector<std::string> BasicModule::submodule_names() const
{
  std::vector<std::string> names;
  for (auto& kv : submodule_map_) {
    names.push_back(kv.first);
  }
  return names;
}

void BasicModule::set_submodule(const std::string& name, std::unique_ptr<BasicSubModule>&& submod)
{
  check_submodule_configuration(name);
  auto it = submodule_map_.find(name);
  if (it == submodule_map_.end()) {
    BOOST_THROW_EXCEPTION( SubModuleUndefinedError(this, name) );
  }
  if (!submod) {
    BOOST_THROW_EXCEPTION(SubModuleNullError(this, name));
  }
  connect_evs_to(*submod);
  connect_module_access_to(*submod);
  submod->define_parameters();
  it->second = std::move(submod);
}

BasicSubModule* BasicModule::get_submodule(const std::string& name)
{
  auto it = submodule_map_.find(name);
  if (it == submodule_map_.end()) {
    BOOST_THROW_EXCEPTION(SubModuleUndefinedError(this, name));
  }
  if (!it->second) {
    BOOST_THROW_EXCEPTION(SubModuleNotSetError(this, name));
  }
  return it->second.get();
}

const BasicSubModule* BasicModule::get_submodule(const std::string& name) const
{
  auto it = submodule_map_.find(name);
  if (it == submodule_map_.end()) {
    BOOST_THROW_EXCEPTION( SubModuleUndefinedError(this, name) );
  }

  if (!it->second) {
    BOOST_THROW_EXCEPTION(SubModuleNotSetError(this, name));
  }
  return it->second.get();
}

void BasicModule::copy_submodules(const BasicModule& source)
{
  for (const auto& entry: source.submodule_map_) {
    submodule_map_.emplace(entry.first,
                          entry.second ? entry.second->duplicate() : nullptr);
  }
}

void BasicModule::set_evs_manager(EvsManager* evs_manager)
{
  EvsInterface::set_evs_manager(evs_manager);
  
  for (auto& kv : submodule_map_) {
    if (kv.second) { kv.second->set_evs_manager(evs_manager); }
  }
}

void BasicModule::set_module_access(const ModuleAccess* module_access)
{
  ModuleAccessInterface::set_module_access(module_access);
  
  for (auto& kv : submodule_map_) {
    if (kv.second) { kv.second->set_module_access(module_access); }
  }
}

} /* namespace anlnext */
