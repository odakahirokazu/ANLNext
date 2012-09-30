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

#ifndef ANL_ANLVModule_H
#define ANL_ANLVModule_H 1

#include "BasicModule.hh"


namespace anl
{

/**
 * A virtual class for an ANL Next module.
 *
 * @author Hirokazu Odaka
 * @date 2010-06-xx
 * @date 2012-09-29
 */
class ANLVModule : public BasicModule
{
public:
  explicit ANLVModule(const std::string& name="ANLVModule",
                      const std::string& version="0.1");
  ANLVModule(const ANLVModule& r);

  virtual ~ANLVModule() {}

  std::string module_name()    const { return m_ModuleName; }
  std::string module_version() const { return m_ModuleVersion; }

private:
  std::string m_ModuleName;
  std::string m_ModuleVersion;
};

}

#endif /* ANL_ANLVModule_H */
