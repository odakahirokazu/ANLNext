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

#ifndef ANL_ModuleAccess_H
#define ANL_ModuleAccess_H 1

#include <string>
#include <map>
#include "ANLException.hh"

namespace anl
{

class BasicModule;

/**
 * Interface to access other ANL modules.
 * 
 * @author Hirokazu Odaka
 * @date 2010-06-xx
 */
class ModuleAccess
{
public:
  const BasicModule* getModule(const std::string& name)
  { return getModuleNC(name); }
  
  BasicModule* getModuleNC(const std::string& name);

  void registerModule(const std::string& name, BasicModule* module);

  bool exist(const std::string& name);

private:
  typedef std::map<std::string, BasicModule*> ANLModuleMap;
  ANLModuleMap moduleMap_;
};

inline
BasicModule* ModuleAccess::getModuleNC(const std::string& name)
{
  ANLModuleMap::iterator it = moduleMap_.find(name);
  if (it == moduleMap_.end()) {
    BOOST_THROW_EXCEPTION( ANLException()
                           << ANLErrInfo(std::string("No module: ")+name) );
  }
  return it->second;
}

inline
void ModuleAccess::registerModule(const std::string& name, BasicModule* module)
{
  moduleMap_[name] = module;
}

inline
bool ModuleAccess::exist(const std::string& name)
{
  return static_cast<bool>(moduleMap_.count(name));
}

} /* namespace anl */

#endif /* ANL_ModuleAccess_H */
