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

#include "ModuleAccessInterface.hh"

namespace anlnext
{

ModuleAccessInterface::ModuleAccessInterface()
  : module_access_(nullptr)
{
}

ModuleAccessInterface::~ModuleAccessInterface() = default;

ModuleAccessInterface::ModuleAccessInterface(const ModuleAccessInterface&)
  : module_access_(nullptr)
{
}

void ModuleAccessInterface::set_module_access(const ModuleAccess* module_access)
{
  module_access_ = module_access;
}

void ModuleAccessInterface::connect_module_access_to(ModuleAccessInterface& target) const
{
  target.set_module_access(module_access_);
}

void ModuleAccessInterface::check_module_requirement(const std::string& name, ModulePosition position,
                                                    const BasicModule* requester) const
{
  if (!module_access_) {
    BOOST_THROW_EXCEPTION(ModuleRequirementError(requester, name, position, "Module access is not connected."));
  }
  module_access_->require_module(name, requester, position);
}

} /* namespace anlnext */
