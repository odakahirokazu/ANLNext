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

#include "ModuleParameter.hh"

namespace anlnext
{

#ifdef ANLNEXT_USE_TVECTOR
ModuleParameter<TVector2>::ModuleParameter(const std::string& name, TVector2* ptr)
  : VModuleParameter(name), ptr_(ptr)
{}

std::shared_ptr<VModuleParameter> ModuleParameter<TVector2>::clone()
{ return std::shared_ptr<VModuleParameter>(new ModuleParameter(*this)); }

std::string ModuleParameter<TVector2>::type_name() const
{ return "2-vector"; }

void ModuleParameter<TVector2>::set_value(double x, double y)
{
  __ref__().Set(x*unit(), y*unit());
}

std::vector<double> ModuleParameter<TVector2>::get_value(double, double) const
{
  return std::vector<double>{__ref__().X()/unit(), __ref__().Y()/unit()};
}

void ModuleParameter<TVector2>::output(std::ostream& os) const
{
  os << __ref__().X()/unit() << " " << __ref__().Y()/unit();
}

void ModuleParameter<TVector2>::input(std::istream& is)
{
  double x(0.0), y(0.0);
  is >> x >> y;
  if (is) {
    __ref__().Set(x*unit(), y*unit());
  }
}

boost::property_tree::ptree ModuleParameter<TVector2>::to_property_tree() const
{
  boost::property_tree::ptree pt;
  pt.put("name", name());
  pt.put("type", type_name());
  pt.put("unit_name", unit_name());
  pt.put("unit", unit());
  boost::property_tree::ptree pt_values;
  std::vector<double> values = get_value(0.0, 0.0);
  for (const auto& v: values) {
    boost::property_tree::ptree pt_value;
    pt_value.put("", v);
    pt_values.push_back(std::make_pair("", pt_value));
  }
  pt.add_child("value", pt_values);
  return pt;
}

ModuleParameter<TVector3>::ModuleParameter(const std::string& name, TVector3* ptr)
  : VModuleParameter(name), ptr_(ptr)
{}

std::shared_ptr<VModuleParameter> ModuleParameter<TVector3>::clone()
{ return std::shared_ptr<VModuleParameter>(new ModuleParameter(*this)); }

std::string ModuleParameter<TVector3>::type_name() const
{ return "3-vector"; }

void ModuleParameter<TVector3>::set_value(double x, double y, double z)
{ __ref__().SetXYZ(x*unit(), y*unit(), z*unit()); }

std::vector<double> ModuleParameter<TVector3>::get_value(double, double, double) const
{
  return std::vector<double>{__ref__().X()/unit(), __ref__().Y()/unit(), __ref__().Z()/unit()};
}

void ModuleParameter<TVector3>::output(std::ostream& os) const
{
  os << __ref__().X()/unit() << " "
     << __ref__().Y()/unit() << " "
     << __ref__().Z()/unit();
}

void ModuleParameter<TVector3>::input(std::istream& is)
{
  double x(0.0), y(0.0), z(0.0);
  is >> x >> y >> z;
  if (is) {
    __ref__().SetXYZ(x*unit(), y*unit(), z*unit());
  }
}

boost::property_tree::ptree ModuleParameter<TVector3>::to_property_tree() const
{
  boost::property_tree::ptree pt;
  pt.put("name", name());
  pt.put("type", type_name());
  pt.put("unit_name", unit_name());
  pt.put("unit", unit());
  boost::property_tree::ptree pt_values;
  std::vector<double> values = get_value(0.0, 0.0, 0.0);
  for (const auto& v: values) {
    boost::property_tree::ptree pt_value;
    pt_value.put("", v);
    pt_values.push_back(std::make_pair("", pt_value));
  }
  pt.add_child("value", pt_values);
  return pt;
}

#endif /* ANLNEXT_USE_TVECTOR */

#ifdef ANLNEXT_USE_HEPVECTOR
ModuleParameter<CLHEP::Hep2Vector>::ModuleParameter(const std::string& name, CLHEP::Hep2Vector* ptr)
  : VModuleParameter(name), ptr_(ptr)
{}

std::shared_ptr<VModuleParameter> ModuleParameter<CLHEP::Hep2Vector>::clone()
{ return std::shared_ptr<VModuleParameter>(new ModuleParameter(*this)); }

std::string ModuleParameter<CLHEP::Hep2Vector>::type_name() const
{ return "2-vector"; }

void ModuleParameter<CLHEP::Hep2Vector>::set_value(double x, double y)
{ __ref__().set(x*unit(), y*unit()); }

std::vector<double> ModuleParameter<CLHEP::Hep2Vector>::get_value(double, double) const
{
  return std::vector<double>{__ref__().x()/unit(), __ref__().y()/unit()};
}

void ModuleParameter<CLHEP::Hep2Vector>::output(std::ostream& os) const
{
  os << __ref__().x()/unit() << " " << __ref__().y()/unit();
}

void ModuleParameter<CLHEP::Hep2Vector>::input(std::istream& is)
{
  double x(0.0), y(0.0);
  is >> x >> y;
  if (is) {
    __ref__().set(x*unit(), y*unit());
  }
}

boost::property_tree::ptree ModuleParameter<CLHEP::Hep2Vector>::to_property_tree() const
{
  boost::property_tree::ptree pt;
  pt.put("name", name());
  pt.put("type", type_name());
  pt.put("unit_name", unit_name());
  pt.put("unit", unit());
  boost::property_tree::ptree pt_values;
  std::vector<double> values = get_value(0.0, 0.0);
  for (const auto& v: values) {
    boost::property_tree::ptree pt_value;
    pt_value.put("", v);
    pt_values.push_back(std::make_pair("", pt_value));
  }
  pt.add_child("value", pt_values);
  return pt;
}

ModuleParameter<CLHEP::Hep3Vector>::ModuleParameter(const std::string& name, CLHEP::Hep3Vector* ptr)
  : VModuleParameter(name), ptr_(ptr)
{}

std::shared_ptr<VModuleParameter> ModuleParameter<CLHEP::Hep3Vector>::clone()
{ return std::shared_ptr<VModuleParameter>(new ModuleParameter(*this)); }

std::string ModuleParameter<CLHEP::Hep3Vector>::type_name() const
{ return "3-vector"; }

void ModuleParameter<CLHEP::Hep3Vector>::set_value(double x, double y, double z)
{ __ref__().set(x*unit(), y*unit(), z*unit()); }

std::vector<double> ModuleParameter<CLHEP::Hep3Vector>::get_value(double, double, double) const
{
  return std::vector<double>{__ref__().x()/unit(), __ref__().y()/unit(), __ref__().z()/unit()};
}

void ModuleParameter<CLHEP::Hep3Vector>::output(std::ostream& os) const
{
  os << __ref__().x()/unit() << " "
     << __ref__().y()/unit() << " "
     << __ref__().z()/unit();
}

void ModuleParameter<CLHEP::Hep3Vector>::input(std::istream& is)
{
  double x(0.0), y(0.0), z(0.0);
  is >> x >> y >> z;
  if (is) {
    __ref__().set(x*unit(), y*unit(), z*unit());
  }
}

boost::property_tree::ptree ModuleParameter<CLHEP::Hep3Vector>::to_property_tree() const
{
  boost::property_tree::ptree pt;
  pt.put("name", name());
  pt.put("type", type_name());
  pt.put("unit_name", unit_name());
  pt.put("unit", unit());
  boost::property_tree::ptree pt_values;
  std::vector<double> values = get_value(0.0, 0.0, 0.0);
  for (const auto& v: values) {
    boost::property_tree::ptree pt_value;
    pt_value.put("", v);
    pt_values.push_back(std::make_pair("", pt_value));
  }
  pt.add_child("value", pt_values);
  return pt;
}

#endif /* ANLNEXT_USE_HEPVECTOR */

} /* namespace anlnext */
