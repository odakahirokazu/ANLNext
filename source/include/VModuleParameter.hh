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
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <memory>

#include <boost/property_tree/ptree.hpp>

#if ANL_USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif /* ANL_USE_READLINE */

namespace anl
{

/**
 * A virtual class for an ANL module parameter.
 *
 * @author Hirokazu Odaka
 * @date 2010-06-xx
 * @date 2011-07-12
 * @date 2011-12-28
 * @date 2015-11-11 | review set/get-value methods.
 */
class VModuleParameter
{
public:
  VModuleParameter(const std::string& name);
  VModuleParameter(const std::string& name,
                   double unit, const std::string& unit_name);  
  VModuleParameter(const std::string& name, const std::string& expression);
  VModuleParameter(const std::string& name, const std::string& expression,
                   const std::string& default_string);

  virtual ~VModuleParameter();
  
  std::string name() const { return name_; }
  double unit() const { return unit_; }
  std::string unit_name() const { return unit_name_; }

  std::string expression() const { return expr_; }
  std::string question() const { return question_; }
  std::string default_string() const { return default_string_; }

  void set_unit(double unit, const std::string& unit_name)
  { unit_ = unit; unit_name_ = unit_name; }
  
  void set_expression(const std::string& v) { expr_ = v; }
  void set_question(const std::string& v) { question_ = v; }
  void set_default_string(const std::string& v) { default_string_ = v; }

  void set_hidden(bool v=true) { hidden_ = v; }
  void set_exposed() { hidden_ = false; }
  bool is_hidden() const { return hidden_; }

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

  virtual void output(std::ostream& ) const {}
  virtual void input(std::istream& ) {}

  virtual void get(void* /* value_ptr */) const {}
  virtual void set(const void* /* value_ptr */) {}

  virtual std::string map_key_name() const { return ""; }
  virtual void set_map_key(const std::string& /* key */) {}

  virtual std::size_t num_value_elements() const { return 0; }
  virtual std::shared_ptr<VModuleParameter const> value_element_info(std::size_t /* index */) const
  { return std::shared_ptr<VModuleParameter>(); }
  virtual void add_value_element(std::shared_ptr<VModuleParameter> /* param */) {}
  virtual void enable_value_elements(int /* type */, const std::vector<std::size_t>& /* enables */) {}

  virtual std::size_t size_of_container() const { return 0; }
  virtual void clear_container() {}
  virtual std::vector<std::string> map_key_list() const { return {}; }
  virtual void insert_to_container() {}
  virtual void retrieve_from_container(const std::string& /* key */) const {}
  virtual void retrieve_from_container(std::size_t /* index */) const {}

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

protected:
  virtual bool ask_base();
  virtual void ask_base_out(std::ostream& ost);
  virtual bool ask_base_in(std::istream& ist);
  std::string special_message_to_ask() const;
  void throw_type_match_exception(const std::string& message="") const;
  
private:
  std::string name_;
  double unit_ = 1.0;
  std::string unit_name_;
  std::string expr_;
  std::string question_;
  std::string default_string_;
  bool hidden_;
  std::string description_;
};

typedef std::shared_ptr<VModuleParameter> ModuleParam_sptr;
typedef std::list<ModuleParam_sptr> ModuleParamList;
typedef ModuleParamList::iterator ModuleParamIter;
typedef ModuleParamList::const_iterator ModuleParamConstIter;

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
