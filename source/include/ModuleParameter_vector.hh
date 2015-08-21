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

namespace anl
{

/**
 * A class template for an ANL module parameter. Partial specialization.
 * @author Hirokazu Odaka
 * @date 2014-12-09
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

  bool first_input()
  {
    if (first_input_) {
      first_input_ = false;
      return true;
    }
    return false;
  }

  void output(std::ostream& os) const
  {
    os << '\n';
    std::size_t index = 0;
    for (iter_type it=ptr_->begin(); it!=ptr_->end(); ++it) {
      os << "  index: " << index++ << '\n';
      value_type tmpValue = *it;
      set_value<0>(&tmpValue, value_category());
      output_value<0>(os, value_category());
    }
    os.flush();
  }
  
  void input(std::istream&) {}
  
  std::size_t num_value_elements() const { return value_info_.size(); }
  std::shared_ptr<VModuleParameter const> get_value_element(std::size_t i) const
  { return value_info_[i]; }
  void add_value_element(ModuleParam_sptr param) { value_info_.push_back(param); }

  void set_default_value_element()
  {
    get_value<0>(&default_value_, value_category());
  }

  void set_value_element(const std::string& name, int val)
  { set_value_element_impl(name, val); }

  void set_value_element(const std::string& name, double val)
  { set_value_element_impl(name, val); }

  void set_value_element(const std::string& name, const std::string& val)
  { set_value_element_impl(name, val); }

  void insert_to_container()
  {
    value_type tmpValue;
    get_value<0>(&tmpValue, value_category());
    ptr_->push_back(tmpValue);
    
    set_value<0>(&default_value_, value_category());
  }

  bool ask()
  {
    std::cout << "Define table of " << name() << ":" << std::endl;
    std::string buffer = "OK";
    ModuleParameter<std::string> tmpKeyParam(&buffer, "continue");
    tmpKeyParam.set_question(name()+" (OK for exit)");
    
    ptr_->clear();
    if (first_input()) { set_default_value_element(); }

    while (1) {
      tmpKeyParam.ask();
      if (buffer=="ok" || buffer=="OK") break;
      
      set_value<0>(&default_value_, value_category());
      
      const std::size_t NumValue = value_info_.size();
      for (std::size_t i=0; i<NumValue; ++i) {
        value_info_[i]->ask();
      }
      
      value_type tmpValue;
      get_value<0>(&tmpValue, value_category());
      ptr_->push_back(tmpValue);
      buffer = "OK";
    }
    return true;
  }
  
private:
  template <std::size_t Index>
  void set_value(value_type* pval, value_category_scalar) const
  {
    value_info_[0]->set(pval);
  }

  template <std::size_t Index>
  void set_value(value_type* pval, value_category_pair) const
  {
    value_info_[0]->set(&(pval->first));
    value_info_[1]->set(&(pval->second));
  }
  
  template <std::size_t Index>
  void set_value(value_type* pval, value_category_tuple) const
  {
    typedef std::integral_constant<std::size_t, Index> Index_t;
    set_value<Index>(pval, value_category_tuple(), Index_t());
  }

  template <std::size_t Index>
  void set_value(value_type* pval, value_category_tuple,
                 std::integral_constant<std::size_t, Index>) const
  {
    value_info_[Index]->set(&(std::get<Index>(*pval)));
    typedef std::integral_constant<std::size_t, Index+1> NextIndex_t;
    set_value<Index+1>(pval, value_category_tuple(), NextIndex_t());
  }

  template <std::size_t Index>
  void set_value(value_type*, value_category_tuple,
                 ValueEnd_t) const
  {
  }

  template <std::size_t Index>
  void get_value(value_type* pval, value_category_scalar) const
  {
    value_info_[0]->get(pval);
  }

  template <std::size_t Index>
  void get_value(value_type* pval, value_category_pair) const
  {
    value_info_[0]->get(&(pval->first));
    value_info_[1]->get(&(pval->second));
  }
  
  template <std::size_t Index>
  void get_value(value_type* pval, value_category_tuple) const
  {
    typedef std::integral_constant<std::size_t, Index> Index_t;
    get_value<Index>(pval, value_category_tuple(), Index_t());
  }

  template <std::size_t Index>
  void get_value(value_type* pval, value_category_tuple,
                 std::integral_constant<std::size_t, Index>) const
  {
    value_info_[Index]->get(&(std::get<Index>(*pval)));
    typedef std::integral_constant<std::size_t, Index+1> NextIndex_t;
    get_value<Index+1>(pval, value_category_tuple(), NextIndex_t());
  }

  template <std::size_t Index>
  void get_value(value_type*, value_category_tuple,
                 ValueEnd_t) const
  {
  }

  template <std::size_t Index>
  void output_value(std::ostream& os, value_category_scalar) const
  {
    os << "    "; value_info_[0]->print(os); os << '\n';
  } 

  template <std::size_t Index>
  void output_value(std::ostream& os, value_category_pair) const
  {
    os << "    "; value_info_[0]->print(os); os << '\n';
    os << "    "; value_info_[1]->print(os); os << '\n';
  } 

  template <std::size_t Index>
  void output_value(std::ostream& os, value_category_tuple) const
  {
    typedef std::integral_constant<std::size_t, Index> Index_t;
    output_value<Index>(os, value_category_tuple(), Index_t());
  }

  template <std::size_t Index>
  void output_value(std::ostream& os, value_category_tuple,
                    std::integral_constant<std::size_t, Index>) const
  {
    os << "    "; value_info_[Index]->print(os); os << '\n';
    typedef std::integral_constant<std::size_t, Index+1> NextIndex_t;
    output_value<Index+1>(os, value_category_tuple(), NextIndex_t());
  }

  template <std::size_t Index>
  void output_value(std::ostream&, value_category_tuple,
                    ValueEnd_t) const
  {
  }

  template <typename ValueT>
  void set_value_element_impl(const std::string& name, ValueT val)
  {
    typedef std::vector<ModuleParam_sptr>::iterator IterT;

    if (first_input()) { set_default_value_element(); }

    for (IterT it=value_info_.begin(); it!=value_info_.end(); ++it) {
      if ((*it)->name()==name) {
        (*it)->set_value(val);
        return;
      }
    }

    BOOST_THROW_EXCEPTION( ANLException() <<
                           ANLErrInfo(std::string("Parameter is not found: ")
                                      +name) );
  }

private:
  container_type* ptr_;
  value_type default_value_;
  std::vector<ModuleParam_sptr> value_info_;
  bool first_input_;
};

} /* namespace anl */
