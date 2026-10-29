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

#include "ModuleAccess.hh"
#include "BasicModule.hh"

namespace anlnext
{

ModuleAccess::~ModuleAccess() = default;

const BasicModule* ModuleAccess::get_module(const std::string& name) const
{
  ANLModuleMap::const_iterator it = module_map_.find(name);
  if (it != std::end(module_map_)) {
    const BasicModule* const m = it->second;
    const Permission permission = m->access_permission();
    if (permission == Permission::full_access || permission == Permission::read_only_access) {
      return m;
    }
    else {
      BOOST_THROW_EXCEPTION( ModuleAccessError("Module access is denied", name) );
    }
  }
  else {
    BOOST_THROW_EXCEPTION( ModuleAccessError("Module is not found", name) );
  }
  return nullptr;
}

BasicModule* ModuleAccess::get_module_NC(const std::string& name) const
{
  ANLModuleMap::const_iterator it = module_map_.find(name);
  if (it != std::end(module_map_)) {
    BasicModule* const m = it->second;
    if (m->access_permission() == Permission::full_access) {
      return m;
    }
    else {
      BOOST_THROW_EXCEPTION( ModuleAccessError("Module access is denied", name) );
    }
  }
  else {
    BOOST_THROW_EXCEPTION( ModuleAccessError("Module is not found", name) );
  }
  return nullptr;
}

const BasicModule* ModuleAccess::request_module(const std::string& name) const
{
  ANLModuleMap::const_iterator it = module_map_.find(name);
  if (it != std::end(module_map_)) {
    const BasicModule* const m = it->second;
    const Permission permission = m->access_permission();
    if (permission == Permission::full_access || permission == Permission::read_only_access) {
      return m;
    }
  }
  return nullptr;
}

BasicModule* ModuleAccess::request_module_NC(const std::string& name) const
{
  ANLModuleMap::const_iterator it = module_map_.find(name);
  if (it != std::end(module_map_)) {
    BasicModule* const m = it->second;
    if (m->access_permission() == Permission::full_access) {
      return m;
    }
  }
  return nullptr;
}

void ModuleAccess::register_module(const std::string& name,
                                   BasicModule* module,
                                   ModuleAccess::ConflictOption conflict)
{
  if (exist(name)) {
    switch (conflict) {
      case ConflictOption::yield:
        break;
      case ConflictOption::overwrite:
        module_map_[name] = module;
        break;
      case ConflictOption::remove:
        module_map_.erase(name);
        break;
      case ConflictOption::error:
        BOOST_THROW_EXCEPTION( ModuleAccessError("Module ID or alias already exists", name) );
        break;
    }
  }
  else {
    switch (conflict) {
      case ConflictOption::remove:
        break;
      default:
        module_map_[name] = module;
    }
  }
}

void ModuleAccess::set_module_order(const std::vector<BasicModule*>& modules)
{
  module_positions_.clear();
  for (std::size_t index=0; index<modules.size(); ++index) {
    module_positions_.emplace(modules[index], index);
  }
}

void ModuleAccess::require_module(const std::string& name, const BasicModule* requester,
                                ModulePosition position) const
{
  auto target = module_map_.find(name);
  if (target == module_map_.end()) {
    BOOST_THROW_EXCEPTION(ModuleRequirementError(requester, name, position, "Module is not found."));
  }
  auto source_position = module_positions_.find(requester);
  auto target_position = module_positions_.find(target->second);
  if (source_position == module_positions_.end() || target_position == module_positions_.end()) {
    BOOST_THROW_EXCEPTION(ModuleRequirementError(requester, name, position, "Module position is not registered."));
  }
  bool satisfied = false;
  switch (position) {
  case ModulePosition::before:
    satisfied = target_position->second < source_position->second;
    break;
  case ModulePosition::after:
    satisfied = target_position->second > source_position->second;
    break;
  case ModulePosition::any:
    satisfied = true;
    break;
  }
  if (!satisfied) {
    BOOST_THROW_EXCEPTION(ModuleRequirementError(requester, name, position,
      "Module order does not satisfy the requirement. Target module ID: " + target->second->module_id()));
  }
}

} /* namespace anlnext */
