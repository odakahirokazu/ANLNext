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

template <typename T>
ModuleParameter<T>::ModuleParameter(T* ptr, const std::string& name)
  : VModuleParameter(name), ptr_(ptr)
{
}

template <typename T>
ModuleParameter<T>::ModuleParameter(T* ptr, const std::string& name,
                                    double unit,
                                    const std::string& unit_name)
  : VModuleParameter(name, unit, unit_name), ptr_(ptr)
{
}

template <typename T>
ModuleParameter<T>::ModuleParameter(T* ptr, const std::string& name,
                                    const std::string& expression)
  : VModuleParameter(name, expression), ptr_(ptr)
{
}

template <typename T>
ModuleParameter<T>::ModuleParameter(T* ptr, const std::string& name,
                                    const std::string& expression,
                                    const std::string& default_string)
  : VModuleParameter(name, expression, default_string), ptr_(ptr)
{
}

template <typename T>
bool ModuleParameter<T>::ask()
{
  if (expression().find("seq")!=std::string::npos) {
    return ask_sequential(is_container_type());
  }
  return ask_base();
}

template <typename T>
std::string ModuleParameter<T>::special_message_to_ask()
{
  return special_message_to_ask_impl(is_container_type());
}

template <typename T>
bool ModuleParameter<T>::ask_sequential(const std::true_type&)
{
  typedef typename T::iterator iter_type;
  typedef typename std::iterator_traits<iter_type>::value_type value_type;

  std::string tmpString(default_string());
  ModuleParameter<std::string> tmpStringParam(&tmpString, name()); 
  tmpStringParam.set_question(name()+" (OK for exit)");
  value_type t;
  ModuleParameter<value_type> tmpParam(&t, name());
  
  ptr_->clear();
  while (1) {
    tmpStringParam.ask();
    if (tmpString=="ok" || tmpString=="OK") break;
    tmpParam.set_value(boost::lexical_cast<value_type>(tmpString));
    ptr_->push_back(t);
    tmpString = "OK";
  }
  return true;
}

template <typename T>
void ModuleParameter<T>::set_value_impl(call_type val,
                                        std::random_access_iterator_tag)
{
  typedef typename T::const_iterator iter_type;
  typedef typename std::iterator_traits<iter_type>::value_type value_type;

  const std::size_t n = val.size();
  ptr_->resize(n);
  for (std::size_t i=0; i<n; ++i) {
    value_type t;
    ModuleParameter<value_type> tmpParam(&t, "");
    tmpParam.set_unit(unit(), unit_name());
    tmpParam.set_expression(expression());
    tmpParam.set_value(val[i]);
    ptr_->at(i) = t;
  }
}

template <typename T>
void ModuleParameter<T>::set_value_impl(call_type val,
                                        std::forward_iterator_tag)
{
  typedef typename T::const_iterator iter_type;
  typedef typename std::iterator_traits<iter_type>::value_type value_type;

  for (iter_type it=val.begin(); it!=val.end(); ++it) {
    value_type t;
    ModuleParameter<value_type> tmpParam(&t, "");
    tmpParam.set_unit(unit(), unit_name());
    tmpParam.set_expression(expression());
    tmpParam.set_value(*it);
    ptr_->push_back(t);
  }
}

template <typename T>
T ModuleParameter<T>::get_value_impl(call_type,
                                     std::random_access_iterator_tag) const
{
  typedef typename T::const_iterator iter_type;
  typedef typename std::iterator_traits<iter_type>::value_type value_type;

  T rval;
  const std::size_t n = ptr_->size();
  rval.resize(n);
  value_type dummy = value_type();

  for (std::size_t i=0; i<n; ++i) {
    value_type* t = &((*ptr_)[i]);
    ModuleParameter<value_type> tmpParam(t, "");
    tmpParam.set_unit(unit(), unit_name());
    tmpParam.set_expression(expression());
    rval[i] = tmpParam.get_value(dummy);
  }
  return rval;
}

template <typename T>
T ModuleParameter<T>::get_value_impl(call_type,
                                     std::forward_iterator_tag) const
{
  typedef typename T::const_iterator iter_type;
  typedef typename std::iterator_traits<iter_type>::value_type value_type;

  T rval;
  value_type dummy = value_type();

  for (iter_type it=ptr_->begin(); it!=ptr_->end(); ++it) {
    value_type* t = &(*it);
    ModuleParameter<value_type> tmpParam(t, "");
    tmpParam.set_unit(unit(), unit_name());
    tmpParam.set_expression(expression());
    rval.push_back(tmpParam.get_value(dummy));
  }
  return rval;
}

template <typename T>
void ModuleParameter<T>::output_impl(std::ostream& os,
                                     std::forward_iterator_tag) const
{
  typedef typename T::iterator iter_type;
  typedef typename std::iterator_traits<iter_type>::value_type value_type;

  for (iter_type it=ptr_->begin(); it!=ptr_->end(); ++it) {
    ModuleParameter<value_type> tmpParam(&(*it), "");
    tmpParam.set_unit(unit(), unit_name());
    tmpParam.set_expression(expression());
    tmpParam.output(os);
    os << " ";
  }
}

template <typename T>
void ModuleParameter<T>::input_impl(std::istream& is,
                                    std::random_access_iterator_tag)
{
  typedef typename T::iterator iter_type;
  typedef typename std::iterator_traits<iter_type>::value_type value_type;

  std::size_t n = 0;
  is >> n;
  if (!is) return;
  ptr_->resize(n);
  for (std::size_t i=0; i<n; ++i) {
    value_type t;
    ModuleParameter<value_type> tmpParam(&t, "");
    tmpParam.set_unit(unit(), unit_name());
    tmpParam.set_expression(expression());
    is >> tmpParam;
    ptr_->at(i) = t;
  }
}

template <typename T>
void ModuleParameter<T>::input_impl(std::istream& is,
                                    std::forward_iterator_tag)
{
  typedef typename T::iterator iter_type;
  typedef typename std::iterator_traits<iter_type>::value_type value_type;

  std::size_t n = 0;
  is >> n;
  if (!is) return;
  for (std::size_t i=0; i<n; ++i) {
    value_type t;
    ModuleParameter<value_type> tmpParam(&t, "");
    tmpParam.set_unit(unit(), unit_name());
    tmpParam.set_expression(expression());
    is >> tmpParam;
    ptr_->push_back(t);
  }
}

} /* namespace anl */
