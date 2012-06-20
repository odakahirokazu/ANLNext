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

#ifndef ANL_ANLAccess_H
#define ANL_ANLAccess_H 1

#include <string>
#include <map>

#include "ANLException.hh"

namespace anl
{

class ANLVModule;

/**
 * Interface to access other ANL modules.
 * 
 * @author Hirokazu Odaka
 * @date 2010-06-xx
 */
class ANLAccess
{
public:
  const ANLVModule* GetModule(const std::string& name)
  { return GetModuleNC(name); }
  
  ANLVModule* GetModuleNC(const std::string& name);

  void RegisterModule(const std::string& name, ANLVModule* module);

  bool Exist(const std::string& name);

private:
  typedef std::map<std::string, ANLVModule*> ANLModuleMap;
  ANLModuleMap module_map;
};


inline
ANLVModule* ANLAccess::GetModuleNC(const std::string& name)
{
  ANLModuleMap::iterator it = module_map.find(name);
  if (it == module_map.end()) {
    BOOST_THROW_EXCEPTION( ANLException()
                           << ANLErrInfo(std::string("No module: ")+name) );
  }
  return it->second;
}


inline
void ANLAccess::RegisterModule(const std::string& name, ANLVModule* module)
{
  module_map[name] = module;
}


inline
bool ANLAccess::Exist(const std::string& name)
{
  return static_cast<bool>(module_map.count(name));
}

}

#endif /* ANL_ANLAccess_H */
