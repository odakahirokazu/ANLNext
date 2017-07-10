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

namespace anl
{

#ifdef ANL_USE_TVECTOR
template <> class ModuleParameter<TVector2> : public VModuleParameter
{
  using T = TVector2;

public:
  ModuleParameter(const std::string& name, T* ptr)
    : VModuleParameter(name), ptr_(ptr)
  {}

  std::shared_ptr<VModuleParameter> clone() override
  { return std::shared_ptr<VModuleParameter>(new ModuleParameter(*this)); }

protected:
  ModuleParameter(const ModuleParameter&) = default;

public:
  std::string type_name() const override
  { return "2-vector"; }
  
  void set_value(double x, double y) override
  {
    __ref__().Set(x*unit(), y*unit());
  }
  using VModuleParameter::set_value;

  std::vector<double> get_value(double, double) const override
  {
    return std::vector<double>{__ref__().X()/unit(), __ref__().Y()/unit()};
  }
  using VModuleParameter::get_value;
  
  void output(std::ostream& os) const override
  {
    os << __ref__().X()/unit() << " " << __ref__().Y()/unit();
  }
  
  void input(std::istream& is) override
  {
    double x(0.0), y(0.0);
    is >> x >> y;
    if (is) {
      __ref__().Set(x*unit(), y*unit());
    }
  }

  boost::property_tree::ptree to_property_tree() const override
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
  ModuleParameter(const std::string& name, T* ptr)
    : VModuleParameter(name), ptr_(ptr)
  {}

  std::shared_ptr<VModuleParameter> clone() override
  { return std::shared_ptr<VModuleParameter>(new ModuleParameter(*this)); }

protected:
  ModuleParameter(const ModuleParameter&) = default;

public:
  std::string type_name() const override
  { return "3-vector"; }
  
  void set_value(double x, double y, double z) override
  { __ref__().SetXYZ(x*unit(), y*unit(), z*unit()); }
  using VModuleParameter::set_value;
  
  std::vector<double> get_value(double, double, double) const override
  {
    return std::vector<double>{__ref__().X()/unit(), __ref__().Y()/unit(), __ref__().Z()/unit()};
  }
  using VModuleParameter::get_value;
  
  void output(std::ostream& os) const override
  {
    os << __ref__().X()/unit() << " "
       << __ref__().Y()/unit() << " "
       << __ref__().Z()/unit();
  }

  void input(std::istream& is) override
  {
    double x(0.0), y(0.0), z(0.0);
    is >> x >> y >> z;
    if (is) {
      __ref__().SetXYZ(x*unit(), y*unit(), z*unit());
    }
  }

  boost::property_tree::ptree to_property_tree() const override
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

protected:
  virtual T& __ref__() { return *ptr_; }
  virtual const T& __ref__() const { return *ptr_; }

private:
  TVector3* ptr_;
};
#endif /* ANL_USE_TVECTOR */

#ifdef ANL_USE_HEPVECTOR
template <> class ModuleParameter<CLHEP::Hep2Vector> : public VModuleParameter
{
  using T = CLHEP::Hep2Vector;

public:
  ModuleParameter(const std::string& name, T* ptr)
    : VModuleParameter(name), ptr_(ptr)
  {}
  
  std::shared_ptr<VModuleParameter> clone() override
  { return std::shared_ptr<VModuleParameter>(new ModuleParameter(*this)); }

protected:
  ModuleParameter(const ModuleParameter&) = default;

public:
  std::string type_name() const override
  { return "2-vector"; }

  void set_value(double x, double y) override
  { __ref__().set(x*unit(), y*unit()); }
  using VModuleParameter::set_value;

  std::vector<double> get_value(double, double) const override
  {
    return std::vector<double>{__ref__().x()/unit(), __ref__().y()/unit()};
  }
  using VModuleParameter::get_value;

  void output(std::ostream& os) const override
  {
    os << __ref__().x()/unit() << " " << __ref__().y()/unit();
  }

  void input(std::istream& is) override
  {
    double x(0.0), y(0.0);
    is >> x >> y;
    if (is) {
      __ref__().set(x*unit(), y*unit());
    }
  }
 
  boost::property_tree::ptree to_property_tree() const override
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
  ModuleParameter(const std::string& name, T* ptr)
    : VModuleParameter(name), ptr_(ptr)
  {}
  
  std::shared_ptr<VModuleParameter> clone() override
  { return std::shared_ptr<VModuleParameter>(new ModuleParameter(*this)); }

protected:
  ModuleParameter(const ModuleParameter&) = default;

public:
  std::string type_name() const override
  { return "3-vector"; }

  void set_value(double x, double y, double z) override
  { __ref__().set(x*unit(), y*unit(), z*unit()); }
  using VModuleParameter::set_value;

  std::vector<double> get_value(double, double, double) const override
  {
    return std::vector<double>{__ref__().x()/unit(), __ref__().y()/unit(), __ref__().z()/unit()};
  }
  using VModuleParameter::get_value;

  void output(std::ostream& os) const override
  {
    os << __ref__().x()/unit() << " "
       << __ref__().y()/unit() << " "
       << __ref__().z()/unit();
  }

  void input(std::istream& is) override
  {
    double x(0.0), y(0.0), z(0.0);
    is >> x >> y >> z;
    if (is) {
      __ref__().set(x*unit(), y*unit(), z*unit());
    }
  }

  boost::property_tree::ptree to_property_tree() const override
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

protected:
  virtual T& __ref__() { return *ptr_; }
  virtual const T& __ref__() const { return *ptr_; }

private:
  CLHEP::Hep3Vector* ptr_;
};
#endif /* ANL_USE_HEPVECTOR */

} /* namespace anl */
