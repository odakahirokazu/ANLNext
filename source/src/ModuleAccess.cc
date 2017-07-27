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
  ANLModuleMap::const_iterator it = moduleMap_.find(name);
  if (it != std::end(moduleMap_)) {
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
  ANLModuleMap::const_iterator it = moduleMap_.find(name);
  if (it != std::end(moduleMap_)) {
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
  ANLModuleMap::const_iterator it = moduleMap_.find(name);
  if (it != std::end(moduleMap_)) {
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
  ANLModuleMap::const_iterator it = moduleMap_.find(name);
  if (it != std::end(moduleMap_)) {
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
        moduleMap_[name] = module;
        break;
      case ConflictOption::remove:
        moduleMap_.erase(name);
        break;
      case ConflictOption::error:
        BOOST_THROW_EXCEPTION( ANLException((boost::format("Module ID or alias %s already exists.") % name).str()) );
        break;
    }
  }
  else {
    switch (conflict) {
      case ConflictOption::remove:
        break;
      default:
        moduleMap_[name] = module;
    }
  }
}

} /* namespace anlnext */
