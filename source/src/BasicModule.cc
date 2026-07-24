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

namespace anlnext
{

BasicModule::BasicModule()
  : order_sensitive_(false),
    module_ID_(""),
    access_permission_(ModuleAccess::Permission::full_access),
    module_on_(true),
    module_access_(nullptr),
    current_parameter_(nullptr),
    current_value_element_(nullptr),
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
  : order_sensitive_(r.order_sensitive_),
    module_ID_(r.module_ID_),
    aliases_(r.aliases_),
    access_permission_(r.access_permission_),
    module_on_(r.module_on_),
    module_access_(nullptr),
    current_parameter_(nullptr),
    current_value_element_(nullptr),
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

boost::property_tree::ptree BasicModule::parameters_to_property_tree() const
{
  boost::property_tree::ptree pt;
  pt.put("module_id", module_id());
  pt.put("name", module_name());
  pt.put("version", module_version());
  boost::property_tree::ptree pt_parameters = ParameterRegistry::parameters_to_property_tree();
  pt.add_child("parameter_list", std::move(pt_parameters));
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

} /* namespace anlnext */
