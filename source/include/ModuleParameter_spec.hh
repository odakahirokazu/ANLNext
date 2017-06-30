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

#include <tuple>

namespace anl
{

#define ANL_USE_SINGLE_TUPLE_PARAMETER 1
#if ANL_USE_SINGLE_TUPLE_PARAMETER
template <typename... Ts>
class ModuleParameter<std::tuple<Ts...>> : public VModuleParameter
{
  typedef std::tuple<Ts...> tuple_type;
public:
  ModuleParameter(tuple_type* ptr, const std::string& name)
    : VModuleParameter(name), ptr_(ptr)
  {}
  
  ModuleParameter(tuple_type* ptr, const std::string& name,
                  double unit, const std::string& unit_name)
    : VModuleParameter(name, unit, unit_name), ptr_(ptr)
  {}

  std::string type_name() const override
  {
    std::string t("tuple<");
    t += type_info<Ts...>::name();
    t += ">";
    return t;
  }

  void set_value(Ts... values) override
  {
    set_value_tuple_impl<0>(values...);
  }
  using VModuleParameter::set_value;
  
  void output(std::ostream& os) const override
  {
    output_tuple_impl<0, Ts...>(os);
  }
  
  void input(std::istream& is) override
  {
    input_tuple_impl<0, Ts...>(is);
  }

private:
  template <int I>
  void set_value_tuple_impl() {}

  template <int I, typename T0, typename... TRest>
  void set_value_tuple_impl(T0 value, TRest... values)
  {
    typedef boost::integral_constant<bool,
                                     boost::is_floating_point<T0>::value> float_truth_type;
    set_value_tuple_one<I>(value, float_truth_type());
    set_value_tuple_impl<I+1>(values...);
  }
  
  template <int I, bool B, typename T0>
  void set_value_tuple_one(T0 value, const boost::integral_constant<bool, B>&)
  {
    // non-floating point
    std::get<I>(*ptr_) = value;
  }

  template <int I, typename T0>
  void set_value_tuple_one(T0 value, const boost::true_type&)
  {
    // floating point
    std::get<I>(*ptr_) = value * unit();
  }

  template <int I>
  void output_tuple_impl(std::ostream&) const {}

  template <int I, typename T0, typename... TRest>
  void output_tuple_impl(std::ostream& os) const
  {
    typedef boost::integral_constant<bool,
                                     boost::is_floating_point<T0>::value> float_truth_type;
    output_tuple_one<I, T0>(os, float_truth_type());
    os << " ";
    output_tuple_impl<I+1, TRest...>(os);
  }

  template <int I, typename T0, bool B>
  void output_tuple_one(std::ostream& os,
                        const boost::integral_constant<bool, B>&) const
  {
    // non-floating point
    os << std::get<I>(*ptr_);
  }

  template <int I, typename T0>
  void output_tuple_one(std::ostream& os,
                        const boost::true_type&) const
  {
    // floating point
    os << std::get<I>(*ptr_)/unit();
  }

  template <int I>
  void input_tuple_impl(std::istream&) {}

  template <int I, typename T0, typename... TRest>
  void input_tuple_impl(std::istream& is)
  {
    typedef boost::integral_constant<bool,
                                     boost::is_floating_point<T0>::value> float_truth_type;
    input_tuple_one<I, T0>(is, float_truth_type());
    input_tuple_impl<I+1, TRest...>(is);
  }

  template <int I, typename T0, bool B>
  void input_tuple_one(std::istream& is,
                       const boost::integral_constant<bool, B>&)
  {
    // non-floating point
    is >> std::get<I>(*ptr_);
  }

  template <int I, typename T0>
  void input_tuple_one(std::istream& is,
                       const boost::true_type&)
  {
    // floating point
    is >> std::get<I>(*ptr_);
    if (is) {
      std::get<I>(*ptr_) *= unit();
    }
  }

private:
  std::tuple<Ts...>* ptr_;
};
#endif

#ifdef ANL_USE_TVECTOR
template <> class ModuleParameter<TVector2> : public VModuleParameter
{
public:
  ModuleParameter(TVector2* ptr, const std::string& name)
    : VModuleParameter(name), ptr_(ptr)
  {}
  
  ModuleParameter(TVector2* ptr, const std::string& name,
                  double unit, const std::string& unit_name)
    : VModuleParameter(name, unit, unit_name), ptr_(ptr)
  {}

  std::string type_name() const override
  { return "2-vector"; }
  
  void set_value(double x, double y) override
  {
    ptr_->Set(x*unit(), y*unit());
  }
  using VModuleParameter::set_value;

  std::vector<double> get_value(double, double) const override
  {
    return std::vector<double>{ptr_->X()/unit(), ptr_->Y()/unit()};
  }
  using VModuleParameter::get_value;
  
  void output(std::ostream& os) const override
  {
    os << ptr_->X()/unit() << " " << ptr_->Y()/unit();
  }
  
  void input(std::istream& is) override
  {
    double x(0.0), y(0.0);
    is >> x >> y;
    if (is) {
      ptr_->Set(x*unit(), y*unit());
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

private:
  TVector2* ptr_;
};

template <> class ModuleParameter<TVector3> : public VModuleParameter
{
public:
  ModuleParameter(TVector3* ptr, const std::string& name)
    : VModuleParameter(name), ptr_(ptr)
  {}
  
  ModuleParameter(TVector3* ptr, const std::string& name,
                  double unit, const std::string& unit_name)
    : VModuleParameter(name, unit, unit_name), ptr_(ptr)
  {}

  std::string type_name() const override
  { return "3-vector"; }
  
  void set_value(double x, double y, double z) override
  { ptr_->SetXYZ(x*unit(), y*unit(), z*unit()); }
  using VModuleParameter::set_value;
  
  std::vector<double> get_value(double, double, double) const override
  {
    return std::vector<double>{ptr_->X()/unit(), ptr_->Y()/unit(), ptr_->Z()/unit()};
  }
  using VModuleParameter::get_value;
  
  void output(std::ostream& os) const override
  {
    os << ptr_->X()/unit() << " "
       << ptr_->Y()/unit() << " "
       << ptr_->Z()/unit();
  }

  void input(std::istream& is) override
  {
    double x(0.0), y(0.0), z(0.0);
    is >> x >> y >> z;
    if (is) {
      ptr_->SetXYZ(x*unit(), y*unit(), z*unit());
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

private:
  TVector3* ptr_;
};
#endif /* ANL_USE_TVECTOR */

#ifdef ANL_USE_HEPVECTOR
template <> class ModuleParameter<CLHEP::Hep2Vector> : public VModuleParameter
{
public:
  ModuleParameter(CLHEP::Hep2Vector* ptr, const std::string& name)
    : VModuleParameter(name), ptr_(ptr)
  {}
  
  ModuleParameter(CLHEP::Hep2Vector* ptr, const std::string& name,
                  double unit, const std::string& unit_name)
    : VModuleParameter(name, unit, unit_name), ptr_(ptr)
  {}
  
  std::string type_name() const override
  { return "2-vector"; }

  void set_value(double x, double y) override
  { ptr_->set(x*unit(), y*unit()); }
  using VModuleParameter::set_value;

  std::vector<double> get_value(double, double) const override
  {
    return std::vector<double>{ptr_->x()/unit(), ptr_->y()/unit()};
  }
  using VModuleParameter::get_value;

  void output(std::ostream& os) const override
  {
    os << ptr_->x()/unit() << " " << ptr_->y()/unit();
  }

  void input(std::istream& is) override
  {
    double x(0.0), y(0.0);
    is >> x >> y;
    if (is) {
      ptr_->set(x*unit(), y*unit());
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

private:
  CLHEP::Hep2Vector* ptr_;
};

template <> class ModuleParameter<CLHEP::Hep3Vector> : public VModuleParameter
{
public:
  ModuleParameter(CLHEP::Hep3Vector* ptr, const std::string& name)
    : VModuleParameter(name), ptr_(ptr)
  {}
  
  ModuleParameter(CLHEP::Hep3Vector* ptr, const std::string& name,
                  double unit, const std::string& unit_name)
    : VModuleParameter(name, unit, unit_name), ptr_(ptr)
  {}
  
  std::string type_name() const override
  { return "3-vector"; }

  void set_value(double x, double y, double z) override
  { ptr_->set(x*unit(), y*unit(), z*unit()); }
  using VModuleParameter::set_value;

  std::vector<double> get_value(double, double, double) const override
  {
    return std::vector<double>{ptr_->x()/unit(), ptr_->y()/unit(), ptr_->z()/unit()};
  }
  using VModuleParameter::get_value;

  void output(std::ostream& os) const override
  {
    os << ptr_->x()/unit() << " "
       << ptr_->y()/unit() << " "
       << ptr_->z()/unit();
  }

  void input(std::istream& is) override
  {
    double x(0.0), y(0.0), z(0.0);
    is >> x >> y >> z;
    if (is) {
      ptr_->set(x*unit(), y*unit(), z*unit());
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

private:
  CLHEP::Hep3Vector* ptr_;
};
#endif /* ANL_USE_HEPVECTOR */

} /* namespace anl */
