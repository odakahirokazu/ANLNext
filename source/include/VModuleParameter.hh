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

#ifndef ANL_VModuleParameter_H
#define ANL_VModuleParameter_H 1

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <memory>
#include <utility>

#include <boost/property_tree/ptree.hpp>

namespace anl
{

class BasicModule;


/**
 * A virtual class for an ANL module parameter.
 *
 * @author Hirokazu Odaka
 * @date 2010-06-xx
 * @date 2011-07-12
 * @date 2011-12-28
 * @date 2015-11-11 | review set/get-value methods.
 * @date 2017-07-03 | rename get/set to __get__/__set__
 * @date 2017-07-10 | review ctor. define_parameter() for data member pointer
 */
class VModuleParameter
{
public:
  explicit VModuleParameter(const std::string& name);
  virtual ~VModuleParameter();

  VModuleParameter(VModuleParameter&&) = delete;
  VModuleParameter& operator=(const VModuleParameter&) = delete;
  VModuleParameter& operator=(VModuleParameter&&) = delete;

  virtual std::shared_ptr<VModuleParameter> clone()
  { return std::shared_ptr<VModuleParameter>(new VModuleParameter(*this)); }

protected:
  VModuleParameter(const VModuleParameter&) = default;

public:
  std::string name() const { return name_; }

  void set_hidden(bool v=true) { hidden_ = v; }
  void set_exposed() { hidden_ = false; }
  bool is_hidden() const { return hidden_; }

  void set_unit(double unit, const std::string& unit_name)
  { unit_ = unit; unit_name_ = unit_name; }

  double unit() const { return unit_; }
  std::string unit_name() const { return unit_name_; }

  void set_expression(const std::string& v) { expr_ = v; }
  std::string expression() const { return expr_; }

  void set_question(const std::string& v) { question_ = v; }
  std::string question() const { return question_; }

  void set_default_string(const std::string& v) { default_string_ = v; }
  std::string default_string() const { return default_string_; }

  void set_description(const std::string& v) { description_ = v; }
  std::string description() const { return description_; }

  virtual std::string type_name() const { return ""; }
  
  virtual bool ask() { return ask_base(); }
  
  virtual void set_value(bool v);
  virtual void set_value(int v);
  virtual void set_value(double v);
  virtual void set_value(const std::string& v);
  virtual void set_value(const std::vector<int>& v);
  virtual void set_value(const std::vector<double>& v);
  virtual void set_value(const std::vector<std::string>& v);
  virtual void set_value(const std::list<std::string>& v);
  virtual void set_value(double x, double y);
  virtual void set_value(double x, double y, double z);

  virtual void set_value_integer(intmax_t v);

  virtual void clear_array() {}

  virtual bool get_value(bool) const;
  virtual int get_value(int) const;
  virtual double get_value(double) const;
  virtual std::string get_value(const std::string&) const;
  virtual std::vector<int> get_value(const std::vector<int>&) const;
  virtual std::vector<double> get_value(const std::vector<double>&) const;
  virtual std::vector<std::string> get_value(const std::vector<std::string>&) const;
  virtual std::list<std::string> get_value(const std::list<std::string>&) const;
  virtual std::vector<double> get_value(double, double) const;
  virtual std::vector<double> get_value(double, double, double) const;

  virtual intmax_t get_value_integer() const;

  virtual void output(std::ostream& ) const {}
  virtual void input(std::istream& ) {}

  virtual void __get__(void* /* value_ptr */) const {}
  virtual void __set__(const void* /* value_ptr */) {}

  virtual void set_map_key_name(const std::string& /* name */) {}
  virtual void set_map_key_properties(const std::string& /* name */, const std::string& /* default_key */) {}
  virtual std::string map_key_name() const { return ""; }

  virtual std::size_t num_value_elements() const { return 0; }
  virtual std::shared_ptr<VModuleParameter const> value_element_info(std::size_t /* index */) const
  { return std::shared_ptr<VModuleParameter>(); }
  virtual std::string value_element_name(std::size_t /* index */) const
  { return ""; }
  virtual void add_value_element(std::shared_ptr<VModuleParameter> /* param */) {}
  virtual void enable_value_elements(int /* type */, const std::vector<std::size_t>& /* enables */) {}

  virtual std::size_t size_of_container() const { return 0; }
  virtual void clear_container() {}
  virtual std::vector<std::string> map_key_list() const { return {}; }
  virtual void insert_to_container() {}
  virtual void retrieve_from_container(const std::string& /* key */) const {}
  virtual void retrieve_from_container(std::size_t /* index */) const {}

  virtual void set_map_key(const std::string& /* key */) {}

  virtual void set_value_element(const std::string& /* name */, bool /* val */) {}
  virtual void set_value_element(const std::string& /* name */, int /* val */) {}
  virtual void set_value_element(const std::string& /* name */, double /* val */) {}
  virtual void set_value_element(const std::string& /* name */, const std::string& /* val */) {}

  virtual bool get_value_element(const std::string& /* name */, bool /* val */) const
  { return false; }
  virtual int get_value_element(const std::string& /* name */, int /* val */) const
  { return 0; }
  virtual double get_value_element(const std::string& /* name */, double /* val */) const
  { return 0.0; }
  virtual std::string get_value_element(const std::string& /* name */, const std::string& /* val */) const
  { return ""; }

  void print(std::ostream& os) const;
  std::string value_string() const;

  virtual boost::property_tree::ptree to_property_tree() const
  { return boost::property_tree::ptree(); }

  virtual void set_module_pointer(BasicModule*) {};

protected:
  virtual bool ask_base();
  virtual void ask_base_out(std::ostream& os);
  virtual bool ask_base_in(std::istream& is);
  std::string special_message_to_ask() const;

  virtual void set_module_pointer_of_value_info(BasicModule*) {}
  
private:
  std::string name_;
  bool hidden_;
  double unit_ = 1.0;
  std::string unit_name_;
  std::string expr_;
  std::string question_;
  std::string default_string_;
  std::string description_;
};

using ModuleParam_sptr = std::shared_ptr<VModuleParameter>;
using ModuleParamList = std::list<ModuleParam_sptr>;
using ModuleParamIter = ModuleParamList::iterator;
using ModuleParamConstIter = ModuleParamList::const_iterator;

} /* namespace anl */

inline
std::ostream& operator<<(std::ostream& os, const anl::VModuleParameter& p)
{
  p.output(os);
  return os;
}

inline
std::istream& operator>>(std::istream& is, anl::VModuleParameter& p)
{
  p.input(is);
  return is;
}

#endif /* ANL_VModuleParameter_H */
