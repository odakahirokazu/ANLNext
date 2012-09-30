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

#include "ANLVModule.hh"

#include <boost/lexical_cast.hpp>

using namespace anl;


ANLVModule::ANLVModule(const std::string& name, const std::string& version)
  : m_ModuleName(name), m_ModuleVersion(version)
{
}


ANLVModule::ANLVModule(const ANLVModule& r)
{
  m_ModuleName = r.m_ModuleName +
    "#" + boost::lexical_cast<std::string>(copy_id());
  m_ModuleVersion = r.m_ModuleVersion;
}
