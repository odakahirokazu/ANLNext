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

#ifndef ANL_ModuleParameter_H
#define ANL_ModuleParameter_H 1

#include "VModuleParameter.hh"

#include <ostream>
#include <istream>
#include <sstream>
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <set>
#include <iomanip>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include <boost/call_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/icl/type_traits/is_container.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tuple/tuple.hpp>

#if ANL_USE_TVECTOR
#include "TVector2.h"
#include "TVector3.h"
#endif

#if ANL_USE_HEPVECTOR
#include "CLHEP/Vector/TwoVector.h"
#include "CLHEP/Vector/ThreeVector.h"
#endif

#include "ANLException.hh"
#include "ANLType.hh"

namespace anl
{

template <typename T>
struct param_call_type
{
  typedef typename boost::call_traits<T>::param_type type;
};

/* this template specialization is a workaround for 32-bit Linux. */
template <>
struct param_call_type<double>
{
  typedef double type;
};


/**
 * A class template for an ANL module parameter.
 * @author Hirokazu Odaka
 * @date 2011-07-12
 * @date 2012-12-12
 */
template <typename T>
class ModuleParameter : public VModuleParameter
{
  typedef boost::integral_constant<bool,
                                   boost::icl::is_container<T>::value
                                   && !boost::is_same<T, std::string>::value> container_truth_type;
  
  typedef boost::integral_constant<bool,
                                   boost::is_arithmetic<T>::value> arithmetic_truth_type;
    
  typedef boost::integral_constant<bool,
                                   boost::is_floating_point<T>::value> float_truth_type;

  typedef boost::integral_constant<bool,
                                   boost::is_integral<T>::value> int_truth_type;

  typedef typename param_call_type<T>::type call_type;
  
public:
  ModuleParameter(T* ptr, const std::string& name);
  
  ModuleParameter(T* ptr, const std::string& name,
                  double unit, const std::string& unit_name);
  
  ModuleParameter(T* ptr, const std::string& name,
                  const std::string& expression);
  
  ModuleParameter(T* ptr, const std::string& name,
                  const std::string& expression,
                  const std::string& default_string);

  std::string type_name() const { return type_info<T>::name(); }

  bool ask();

  void set_value(call_type val)
  {
    set_value_impl(val, container_truth_type(), arithmetic_truth_type(), float_truth_type());
  }

  using VModuleParameter::set_value;
  
  void clear_array()
  {
    clear_array_impl(container_truth_type());
  }
  
  void output(std::ostream& os) const
  {
    output_impl(os, container_truth_type(), arithmetic_truth_type(), float_truth_type());
  }
  
  void input(std::istream& is)
  {
    input_impl(is, container_truth_type(), arithmetic_truth_type(), float_truth_type());
  }

  void get(void* const value_ptr) const
  {
    *static_cast<T* const>(value_ptr) = *_ptr;
  }
  
  void set(const void* const value_ptr)
  {
    *_ptr = *static_cast<const T* const>(value_ptr);
  }

protected:
  template <bool b0>
  bool ask_sequential(const boost::integral_constant<bool, b0>&)
  { return ask_base(); }

  bool ask_sequential(const boost::true_type&);

private:
  template <bool b0, bool b1, bool b2>
  void set_value_impl(call_type val,
                      const boost::integral_constant<bool, b0>&,
                      const boost::integral_constant<bool, b1>&,
                      const boost::integral_constant<bool, b2>&)
  {
    // non-container, non-number
    *_ptr = val;
  }
  
  template <bool b0, bool b2>
  void set_value_impl(call_type val,
                      const boost::integral_constant<bool, b0>&,
                      const boost::true_type&,
                      const boost::integral_constant<bool, b2>&)
  {
    // non-container, number, int
    *_ptr = val;
  }
  
  template <bool b0>
  void set_value_impl(call_type val,
                      const boost::integral_constant<bool, b0>&,
                      const boost::true_type&,
                      const boost::true_type&)
  {
    // non-container, number, float
    *_ptr = val * unit();
  }
  
  template <bool b1, bool b2>
  void set_value_impl(call_type val,
                      const boost::true_type&,
                      const boost::integral_constant<bool, b1>&,
                      const boost::integral_constant<bool, b2>&)
  {
    // container
    typedef typename T::iterator iter_type;
    typedef typename std::iterator_traits<iter_type>::iterator_category IterCategory;
    set_value_impl(val, IterCategory());
  }
  
  void set_value_impl(call_type val, std::random_access_iterator_tag);
  void set_value_impl(call_type val, std::forward_iterator_tag);

  void set_value2(double , double ) {}
  void set_value3(double , double , double ) {}
  
  template <bool b0>
  void clear_array_impl(const boost::integral_constant<bool, b0>&)
  {
    // non-container
  }
  
  void clear_array_impl(const boost::true_type&)
  {
    // container
    _ptr->clear();
  }
  
  template <bool b0, bool b1, bool b2>
  void output_impl(std::ostream& os,
                   const boost::integral_constant<bool, b0>&,
                   const boost::integral_constant<bool, b1>&,
                   const boost::integral_constant<bool, b2>&) const
  {
    // non-container, non-number
    os << *_ptr;
  }
  
  template <bool b0, bool b2>
  void output_impl(std::ostream& os,
                   const boost::integral_constant<bool, b0>&,
                   const boost::true_type&,
                   const boost::integral_constant<bool, b2>&) const
  {
    // non-container, number, int
    using std::string;
    if (expression().find("hex")!=string::npos) os << std::hex;
    else if (expression().find("dec")!=string::npos) os << std::dec;
    os << *_ptr;
    os << std::dec;
  }

  template <bool b0>
  void output_impl(std::ostream& os,
                   const boost::integral_constant<bool, b0>&,
                   const boost::true_type&,
                   const boost::true_type&) const
  {
    // non-container, number, float
    os << *_ptr/unit();
  }
  
  template <bool b1, bool b2>
  void output_impl(std::ostream& os,
                   const boost::true_type&,
                   const boost::integral_constant<bool, b1>&,
                   const boost::integral_constant<bool, b2>&) const
  {
    // container
    typedef typename T::iterator iter_type;
    typedef typename std::iterator_traits<iter_type>::iterator_category IterCategory;
    output_impl(os, IterCategory());
  }
  
  void output_impl(std::ostream& os, std::forward_iterator_tag) const;

  template <bool b0, bool b1, bool b2>
  void input_impl(std::istream& is,
                   const boost::integral_constant<bool, b0>&,
                   const boost::integral_constant<bool, b1>&,
                   const boost::integral_constant<bool, b2>&)
  {
    // non-container, non-number
    is >> *_ptr;
  }
  
  template <bool b0, bool b2>
  void input_impl(std::istream& is,
                  const boost::integral_constant<bool, b0>&,
                  const boost::true_type&,
                  const boost::integral_constant<bool, b2>&)
  {

    // non-container, number, int
    using std::string;
    if (expression().find("hex")!=string::npos) is >> std::hex;
    else if (expression().find("dec")!=string::npos) is >> std::dec;
    is >> *_ptr;
    is >> std::dec;
  }

  template <bool b0>
  void input_impl(std::istream& is,
                  const boost::integral_constant<bool, b0>&,
                  const boost::true_type&,
                  const boost::true_type&)
  {
    // non-container, number, float
    T val(0.0);
    is >> val;
    if (is) { *_ptr = val * unit(); }
  }
  
  template <bool b1, bool b2>
  void input_impl(std::istream& is,
                  const boost::true_type&,
                  const boost::integral_constant<bool, b1>&,
                  const boost::integral_constant<bool, b2>&)
  {
    // container
    typedef typename T::iterator iter_type;
    typedef typename std::iterator_traits<iter_type>::iterator_category IterCategory;
    input_impl(is, IterCategory());
  }
  
  void input_impl(std::istream& is, std::random_access_iterator_tag);
  void input_impl(std::istream& is, std::forward_iterator_tag);

  std::string special_message_to_ask();

  template <bool b> std::string
  special_message_to_ask_impl(const boost::integral_constant<bool, b>&)
  { return VModuleParameter::special_message_to_ask(); }
  
  std::string  special_message_to_ask_impl(const boost::true_type&)
  {
    std::string message("\n  vector length, values... \n");
    return message;
  }

private:
  T* _ptr;
};

}

#include "ModuleParameter_impl.hh"
#include "ModuleParameter_spec.hh"
#include "ModuleParameter_map.hh"

#endif /* ANL_ModuleParameter_H */
