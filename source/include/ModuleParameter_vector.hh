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

#include <cstddef>
#include <tuple>
#include <array>
#include <memory>
#include <type_traits>
#include <boost/format.hpp>

namespace anl
{

/**
 * A class template for an ANL module parameter. Partial specialization.
 * @author Hirokazu Odaka
 * @date 2014-12-09
 * @date 2015-11-10 | rename {set/get/output}_value<I>() to value_info_{set/get/output}().
 */
template <typename... Ts>
class ModuleParameter<std::vector<std::tuple<Ts...>>> : public VModuleParameter
{
  typedef std::vector<std::tuple<Ts...>> container_type;
  typedef std::tuple<Ts...> value_type;
  typedef typename container_type::iterator iter_type;
  typedef typename size_of_value<value_type>::type value_category;
 
  constexpr static std::size_t ValueSize = size_of_value<value_type>::value;
  typedef std::integral_constant<std::size_t, ValueSize> ValueEnd_t;

public:
  ModuleParameter(container_type* ptr, const std::string& name)
    : VModuleParameter(name), ptr_(ptr), first_input_(true)
  {
  }
  
  ModuleParameter(container_type* ptr, const std::string& name,
                  const std::vector<ModuleParam_sptr>& values_info)
    : VModuleParameter(name), ptr_(ptr),
      value_info_(values_info),
      first_input_(true)
  {
  }

  std::string type_name() const { return "vector"; }

  void output(std::ostream& os) const
  {
    os << '\n';
    std::size_t index = 0;
    for (iter_type it=ptr_->begin(); it!=ptr_->end(); ++it) {
      os << "  index: " << index++ << '\n';
      value_type tmpValue = *it;
      value_info_set<0>(&tmpValue, value_category());
      value_info_output<0>(os, value_category());
    }
    os.flush();
  }
  
  void input(std::istream&) {}
  
  std::size_t num_value_elements() const { return value_info_.size(); }
  std::shared_ptr<VModuleParameter const> value_element_info(std::size_t index) const
  { return value_info_[index]; }
  void add_value_element(ModuleParam_sptr param) { value_info_.push_back(param); }

  std::size_t size_of_container() const { return ptr_->size(); }
  void clear_container() { ptr_->clear(); }

  void insert_to_container()
  {
    value_type tmpValue;
    value_info_get<0>(&tmpValue, value_category());
    ptr_->push_back(tmpValue);
    
    value_info_set<0>(&default_value_, value_category());
  }

  void retrieve_from_container(std::size_t index) const
  {
    value_type value = ptr_->at(index);
    value_info_set<0>(&value, value_category());
  }
  using VModuleParameter::retrieve_from_container;
  
  void set_value_element(const std::string& name, bool val)
  { set_value_element_impl(name, val); }

  void set_value_element(const std::string& name, int val)
  { set_value_element_impl(name, val); }

  void set_value_element(const std::string& name, double val)
  { set_value_element_impl(name, val); }

  void set_value_element(const std::string& name, const std::string& val)
  { set_value_element_impl(name, val); }

  bool get_value_element(const std::string& name, bool dummy) const
  { return get_value_element_impl(name, dummy); }
  
  int get_value_element(const std::string& name, int dummy) const
  { return get_value_element_impl(name, dummy); }

  double get_value_element(const std::string& name, double dummy) const
  { return get_value_element_impl(name, dummy); }

  std::string get_value_element(const std::string& name, const std::string& dummy) const
  { return get_value_element_impl(name, dummy); }

  bool ask()
  {
    std::cout << "Define table of " << name() << ":" << std::endl;
    std::string buffer = "OK";
    ModuleParameter<std::string> tmpKeyParam(&buffer, "continue");
    tmpKeyParam.set_question(name()+" (OK for exit)");
    
    ptr_->clear();
    if (first_input()) { initialize_default_value_elements(); }

    while (1) {
      tmpKeyParam.ask();
      if (buffer=="ok" || buffer=="OK") break;
      
      value_info_set<0>(&default_value_, value_category());
      
      const std::size_t NumValue = value_info_.size();
      for (std::size_t i=0; i<NumValue; ++i) {
        value_info_[i]->ask();
      }
      
      value_type tmpValue;
      value_info_get<0>(&tmpValue, value_category());
      ptr_->push_back(tmpValue);
      buffer = "OK";
    }
    return true;
  }

  boost::property_tree::ptree to_property_tree() const
  {
    using boost::property_tree::ptree;
    ptree pt;
    pt.put("name", name());
    pt.put("type", type_name());

    ptree pt_values;
    const std::size_t n = size_of_container();
    for (std::size_t i=0; i<n; i++) {
      ptree pt_value;
      retrieve_from_container(i);
      const std::size_t num_elements = num_value_elements();
      for (std::size_t j=0; j<num_elements; j++) {
        std::shared_ptr<VModuleParameter const> element = value_element_info(j);
        ptree pt_element = element->to_property_tree();
        pt_value.push_back(std::make_pair("", std::move(pt_element))); 
      }
      pt_values.push_back(std::make_pair("", std::move(pt_value)));
    }
    pt.add_child("value", std::move(pt_values));
    return pt;
  }
  
private:
  bool first_input()
  {
    if (first_input_) {
      first_input_ = false;
      return true;
    }
    return false;
  }

  void initialize_default_value_elements()
  {
    value_info_get<0>(&default_value_, value_category());
  }

  template <std::size_t Index>
  void value_info_set(value_type* pval, value_category_scalar) const
  {
    value_info_[0]->set(pval);
  }

  template <std::size_t Index>
  void value_info_set(value_type* pval, value_category_pair) const
  {
    value_info_[0]->set(&(pval->first));
    value_info_[1]->set(&(pval->second));
  }
  
  template <std::size_t Index>
  void value_info_set(value_type* pval, value_category_tuple) const
  {
    typedef std::integral_constant<std::size_t, Index> Index_t;
    value_info_set<Index>(pval, value_category_tuple(), Index_t());
  }

  template <std::size_t Index>
  void value_info_set(value_type* pval, value_category_tuple,
                      std::integral_constant<std::size_t, Index>) const
  {
    value_info_[Index]->set(&(std::get<Index>(*pval)));
    typedef std::integral_constant<std::size_t, Index+1> NextIndex_t;
    value_info_set<Index+1>(pval, value_category_tuple(), NextIndex_t());
  }

  template <std::size_t Index>
  void value_info_set(value_type*, value_category_tuple,
                      ValueEnd_t) const
  {
  }

  template <std::size_t Index>
  void value_info_get(value_type* pval, value_category_scalar) const
  {
    value_info_[0]->get(pval);
  }

  template <std::size_t Index>
  void value_info_get(value_type* pval, value_category_pair) const
  {
    value_info_[0]->get(&(pval->first));
    value_info_[1]->get(&(pval->second));
  }
  
  template <std::size_t Index>
  void value_info_get(value_type* pval, value_category_tuple) const
  {
    typedef std::integral_constant<std::size_t, Index> Index_t;
    value_info_get<Index>(pval, value_category_tuple(), Index_t());
  }

  template <std::size_t Index>
  void value_info_get(value_type* pval, value_category_tuple,
                      std::integral_constant<std::size_t, Index>) const
  {
    value_info_[Index]->get(&(std::get<Index>(*pval)));
    typedef std::integral_constant<std::size_t, Index+1> NextIndex_t;
    value_info_get<Index+1>(pval, value_category_tuple(), NextIndex_t());
  }

  template <std::size_t Index>
  void value_info_get(value_type*, value_category_tuple,
                      ValueEnd_t) const
  {
  }

  template <std::size_t Index>
  void value_info_output(std::ostream& os, value_category_scalar) const
  {
    os << "    "; value_info_[0]->print(os); os << '\n';
  } 

  template <std::size_t Index>
  void value_info_output(std::ostream& os, value_category_pair) const
  {
    os << "    "; value_info_[0]->print(os); os << '\n';
    os << "    "; value_info_[1]->print(os); os << '\n';
  } 

  template <std::size_t Index>
  void value_info_output(std::ostream& os, value_category_tuple) const
  {
    typedef std::integral_constant<std::size_t, Index> Index_t;
    value_info_output<Index>(os, value_category_tuple(), Index_t());
  }

  template <std::size_t Index>
  void value_info_output(std::ostream& os, value_category_tuple,
                         std::integral_constant<std::size_t, Index>) const
  {
    os << "    "; value_info_[Index]->print(os); os << '\n';
    typedef std::integral_constant<std::size_t, Index+1> NextIndex_t;
    value_info_output<Index+1>(os, value_category_tuple(), NextIndex_t());
  }

  template <std::size_t Index>
  void value_info_output(std::ostream&, value_category_tuple,
                         ValueEnd_t) const
  {
  }

  template <typename ValueT>
  void set_value_element_impl(const std::string& name, ValueT val)
  {
    if (first_input()) { initialize_default_value_elements(); }

    auto it = find_value_info(name);
    (*it)->set_value(val);
  }

  template <typename ValueT>
  ValueT get_value_element_impl(const std::string& name, ValueT dummy) const
  {
    auto it = find_value_info(name);
    return (*it)->get_value(dummy);
  }

  std::vector<ModuleParam_sptr>::iterator
  find_value_info(const std::string& name)
  {
    std::vector<ModuleParam_sptr>::iterator it = std::begin(value_info_);
    for (; it!=std::end(value_info_); ++it) {
      if ((*it)->name()==name) {
        return it;
      }
    }

    if (it == std::end(value_info_)) {
      const std::string message
        = (boost::format("Parameter is not found: %s") % name).str();
      BOOST_THROW_EXCEPTION( ANLException(message) );
    }
    return it;
  }

  std::vector<ModuleParam_sptr>::const_iterator
  find_value_info(const std::string& name) const
  {
    std::vector<ModuleParam_sptr>::const_iterator it = std::begin(value_info_);
    for (; it!=std::end(value_info_); ++it) {
      if ((*it)->name()==name) {
        return it;
      }
    }

    if (it == std::end(value_info_)) {
      const std::string message
        = (boost::format("Parameter is not found: %s") % name).str();
      BOOST_THROW_EXCEPTION( ANLException(message) );
    }
    return it;
  }

private:
  container_type* ptr_;
  value_type default_value_;
  std::vector<ModuleParam_sptr> value_info_;
  bool first_input_;
};

} /* namespace anl */
