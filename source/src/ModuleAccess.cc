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

namespace anl
{

ModuleAccess::~ModuleAccess() = default;

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

} /* namespace anl */
