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

#include "BasicSubModule.hh"

namespace anlnext
{

BasicSubModule::BasicSubModule() = default;

BasicSubModule::~BasicSubModule() = default;

std::unique_ptr<BasicSubModule> BasicSubModule::duplicate()
{
  std::unique_ptr<BasicSubModule> m = __duplicate__();
  if (!m) {
    BOOST_THROW_EXCEPTION( ModuleDuplicationError(this) );
  }
  return m;
}

std::unique_ptr<BasicSubModule> BasicSubModule::clone() const
{
  return __clone__();
}

boost::property_tree::ptree BasicSubModule::parameters_to_property_tree() const
{
  boost::property_tree::ptree pt;
  pt.put("name", submodule_name());
  pt.put("version", submodule_version());
  boost::property_tree::ptree pt_parameters = ParameterRegistry::parameters_to_property_tree();
  pt.add_child("parameter_list", std::move(pt_parameters));
  return pt;
}

void BasicSubModule::add_parameter_error_info(ANLException& ex) const
{
  ex.set_module_info(this);
}

} /* namespace anlnext */
