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

#ifndef ANLNEXT_ModuleParameter_special_H
#define ANLNEXT_ModuleParameter_special_H 1

namespace anlnext
{

// Keep the specializations inline so consumers can opt in to these types.
#ifdef ANLNEXT_USE_TVECTOR
template <> class ModuleParameter<TVector2> : public VModuleParameter
{
  using T = TVector2;

public:
  ModuleParameter(const std::string& name, T* ptr);

  std::shared_ptr<VModuleParameter> clone() override;

protected:
  ModuleParameter(const ModuleParameter&) = default;

public:
  std::string type_name() const override;

  void set_value(double x, double y) override;
  using VModuleParameter::set_value;

  std::vector<double> get_value(double, double) const override;
  using VModuleParameter::get_value;

  void output(std::ostream& os) const override;

  void input(std::istream& is) override;

  boost::property_tree::ptree to_property_tree() const override;

protected:
  virtual T& __ref__() { return *ptr_; }
  virtual const T& __ref__() const { return *ptr_; }

private:
  TVector2* ptr_;
};

template <> class ModuleParameter<TVector3> : public VModuleParameter
{
  using T = TVector3;

public:
  ModuleParameter(const std::string& name, T* ptr);

  std::shared_ptr<VModuleParameter> clone() override;

protected:
  ModuleParameter(const ModuleParameter&) = default;

public:
  std::string type_name() const override;

  void set_value(double x, double y, double z) override;
  using VModuleParameter::set_value;

  std::vector<double> get_value(double, double, double) const override;
  using VModuleParameter::get_value;

  void output(std::ostream& os) const override;

  void input(std::istream& is) override;

  boost::property_tree::ptree to_property_tree() const override;

protected:
  virtual T& __ref__() { return *ptr_; }
  virtual const T& __ref__() const { return *ptr_; }

private:
  TVector3* ptr_;
};

inline ModuleParameter<TVector2>::ModuleParameter(const std::string& name, TVector2* ptr)
  : VModuleParameter(name), ptr_(ptr)
{}

inline std::shared_ptr<VModuleParameter> ModuleParameter<TVector2>::clone()
{ return std::shared_ptr<VModuleParameter>(new ModuleParameter(*this)); }

inline std::string ModuleParameter<TVector2>::type_name() const
{ return "2-vector"; }

inline void ModuleParameter<TVector2>::set_value(double x, double y)
{
  __ref__().Set(x*unit(), y*unit());
}

inline std::vector<double> ModuleParameter<TVector2>::get_value(double, double) const
{
  return std::vector<double>{__ref__().X()/unit(), __ref__().Y()/unit()};
}

inline void ModuleParameter<TVector2>::output(std::ostream& os) const
{
  os << __ref__().X()/unit() << " " << __ref__().Y()/unit();
}

inline void ModuleParameter<TVector2>::input(std::istream& is)
{
  double x(0.0), y(0.0);
  is >> x >> y;
  if (is) {
    __ref__().Set(x*unit(), y*unit());
  }
}

inline boost::property_tree::ptree ModuleParameter<TVector2>::to_property_tree() const
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

inline ModuleParameter<TVector3>::ModuleParameter(const std::string& name, TVector3* ptr)
  : VModuleParameter(name), ptr_(ptr)
{}

inline std::shared_ptr<VModuleParameter> ModuleParameter<TVector3>::clone()
{ return std::shared_ptr<VModuleParameter>(new ModuleParameter(*this)); }

inline std::string ModuleParameter<TVector3>::type_name() const
{ return "3-vector"; }

inline void ModuleParameter<TVector3>::set_value(double x, double y, double z)
{ __ref__().SetXYZ(x*unit(), y*unit(), z*unit()); }

inline std::vector<double> ModuleParameter<TVector3>::get_value(double, double, double) const
{
  return std::vector<double>{__ref__().X()/unit(), __ref__().Y()/unit(), __ref__().Z()/unit()};
}

inline void ModuleParameter<TVector3>::output(std::ostream& os) const
{
  os << __ref__().X()/unit() << " "
     << __ref__().Y()/unit() << " "
     << __ref__().Z()/unit();
}

inline void ModuleParameter<TVector3>::input(std::istream& is)
{
  double x(0.0), y(0.0), z(0.0);
  is >> x >> y >> z;
  if (is) {
    __ref__().SetXYZ(x*unit(), y*unit(), z*unit());
  }
}

inline boost::property_tree::ptree ModuleParameter<TVector3>::to_property_tree() const
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
template <> class ModuleParameter<CLHEP::Hep2Vector> : public VModuleParameter
{
  using T = CLHEP::Hep2Vector;

public:
  ModuleParameter(const std::string& name, T* ptr);

  std::shared_ptr<VModuleParameter> clone() override;

protected:
  ModuleParameter(const ModuleParameter&) = default;

public:
  std::string type_name() const override;

  void set_value(double x, double y) override;
  using VModuleParameter::set_value;

  std::vector<double> get_value(double, double) const override;
  using VModuleParameter::get_value;

  void output(std::ostream& os) const override;

  void input(std::istream& is) override;

  boost::property_tree::ptree to_property_tree() const override;

protected:
  virtual T& __ref__() { return *ptr_; }
  virtual const T& __ref__() const { return *ptr_; }

private:
  CLHEP::Hep2Vector* ptr_;
};

template <> class ModuleParameter<CLHEP::Hep3Vector> : public VModuleParameter
{
  using T = CLHEP::Hep3Vector;

public:
  ModuleParameter(const std::string& name, T* ptr);

  std::shared_ptr<VModuleParameter> clone() override;

protected:
  ModuleParameter(const ModuleParameter&) = default;

public:
  std::string type_name() const override;

  void set_value(double x, double y, double z) override;
  using VModuleParameter::set_value;

  std::vector<double> get_value(double, double, double) const override;
  using VModuleParameter::get_value;

  void output(std::ostream& os) const override;

  void input(std::istream& is) override;

  boost::property_tree::ptree to_property_tree() const override;

protected:
  virtual T& __ref__() { return *ptr_; }
  virtual const T& __ref__() const { return *ptr_; }

private:
  CLHEP::Hep3Vector* ptr_;
};

inline ModuleParameter<CLHEP::Hep2Vector>::ModuleParameter(const std::string& name, CLHEP::Hep2Vector* ptr)
  : VModuleParameter(name), ptr_(ptr)
{}

inline std::shared_ptr<VModuleParameter> ModuleParameter<CLHEP::Hep2Vector>::clone()
{ return std::shared_ptr<VModuleParameter>(new ModuleParameter(*this)); }

inline std::string ModuleParameter<CLHEP::Hep2Vector>::type_name() const
{ return "2-vector"; }

inline void ModuleParameter<CLHEP::Hep2Vector>::set_value(double x, double y)
{ __ref__().set(x*unit(), y*unit()); }

inline std::vector<double> ModuleParameter<CLHEP::Hep2Vector>::get_value(double, double) const
{
  return std::vector<double>{__ref__().x()/unit(), __ref__().y()/unit()};
}

inline void ModuleParameter<CLHEP::Hep2Vector>::output(std::ostream& os) const
{
  os << __ref__().x()/unit() << " " << __ref__().y()/unit();
}

inline void ModuleParameter<CLHEP::Hep2Vector>::input(std::istream& is)
{
  double x(0.0), y(0.0);
  is >> x >> y;
  if (is) {
    __ref__().set(x*unit(), y*unit());
  }
}

inline boost::property_tree::ptree ModuleParameter<CLHEP::Hep2Vector>::to_property_tree() const
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

inline ModuleParameter<CLHEP::Hep3Vector>::ModuleParameter(const std::string& name, CLHEP::Hep3Vector* ptr)
  : VModuleParameter(name), ptr_(ptr)
{}

inline std::shared_ptr<VModuleParameter> ModuleParameter<CLHEP::Hep3Vector>::clone()
{ return std::shared_ptr<VModuleParameter>(new ModuleParameter(*this)); }

inline std::string ModuleParameter<CLHEP::Hep3Vector>::type_name() const
{ return "3-vector"; }

inline void ModuleParameter<CLHEP::Hep3Vector>::set_value(double x, double y, double z)
{ __ref__().set(x*unit(), y*unit(), z*unit()); }

inline std::vector<double> ModuleParameter<CLHEP::Hep3Vector>::get_value(double, double, double) const
{
  return std::vector<double>{__ref__().x()/unit(), __ref__().y()/unit(), __ref__().z()/unit()};
}

inline void ModuleParameter<CLHEP::Hep3Vector>::output(std::ostream& os) const
{
  os << __ref__().x()/unit() << " "
     << __ref__().y()/unit() << " "
     << __ref__().z()/unit();
}

inline void ModuleParameter<CLHEP::Hep3Vector>::input(std::istream& is)
{
  double x(0.0), y(0.0), z(0.0);
  is >> x >> y >> z;
  if (is) {
    __ref__().set(x*unit(), y*unit(), z*unit());
  }
}

inline boost::property_tree::ptree ModuleParameter<CLHEP::Hep3Vector>::to_property_tree() const
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

#endif /* ANLNEXT_ModuleParameter_special_H */
