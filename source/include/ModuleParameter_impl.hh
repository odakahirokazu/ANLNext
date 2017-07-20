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
ModuleParameter<T>::ModuleParameter(const std::string& name, T* ptr)
  : VModuleParameter(name), ptr_(ptr)
{
}

template <typename T>
bool ModuleParameter<T>::ask()
{
  if (expression().find("seq") != std::string::npos) {
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
  using iter_type = typename T::iterator;
  using value_type = typename std::iterator_traits<iter_type>::value_type;

  std::string buffer(default_string());
  ModuleParameter<std::string> stringParam(name(), &buffer);
  stringParam.set_question(name()+" | break => '!' | keep => '='");
  value_type t;
  ModuleParameter<value_type> param(name(), &t);
  param.set_unit(unit(), unit_name());
  
  T container;
  while (1) {
    stringParam.ask();
    if (buffer=="=") { return false; }
    if (buffer=="!") { break; }
    param.set_value(boost::lexical_cast<value_type>(buffer));
    container.push_back(t);
    buffer = "!";
  }

  __ref__() = std::move(container);
  return true;
}

template <typename T>
void ModuleParameter<T>::set_value_impl(call_type val,
                                        std::random_access_iterator_tag)
{
  using iter_type = typename T::iterator;
  using value_type = typename std::iterator_traits<iter_type>::value_type;

  const std::size_t n = val.size();
  __ref__().resize(n);
  for (std::size_t i=0; i<n; ++i) {
    value_type t;
    ModuleParameter<value_type> param("", &t);
    param.set_unit(unit(), unit_name());
    param.set_expression(expression());
    param.set_value(val[i]);
    __ref__().at(i) = t;
  }
}

template <typename T>
void ModuleParameter<T>::set_value_impl(call_type val,
                                        std::forward_iterator_tag)
{
  using iter_type = typename T::iterator;
  using value_type = typename std::iterator_traits<iter_type>::value_type;

  __ref__().clear();
  for (iter_type it=val.begin(); it!=val.end(); ++it) {
    value_type t;
    ModuleParameter<value_type> param("", &t);
    param.set_unit(unit(), unit_name());
    param.set_expression(expression());
    param.set_value(*it);
    __ref__().push_back(t);
  }
}

template <typename T>
T ModuleParameter<T>::get_value_impl(call_type,
                                     std::random_access_iterator_tag) const
{
  using iter_type = typename T::const_iterator;
  using value_type = typename std::iterator_traits<iter_type>::value_type;

  T rval;
  const std::size_t n = __ref__().size();
  rval.resize(n);
  value_type dummy = value_type();

  for (std::size_t i=0; i<n; ++i) {
    value_type t = __ref__()[i];
    ModuleParameter<value_type> param("", &t);
    param.set_unit(unit(), unit_name());
    param.set_expression(expression());
    rval[i] = param.get_value(dummy);
  }
  return rval;
}

template <typename T>
T ModuleParameter<T>::get_value_impl(call_type,
                                     std::forward_iterator_tag) const
{
  using iter_type = typename T::const_iterator;
  using value_type = typename std::iterator_traits<iter_type>::value_type;

  T rval;
  value_type dummy = value_type();

  for (iter_type it=__ref__().begin(); it!=__ref__().end(); ++it) {
    value_type t = *it;
    ModuleParameter<value_type> param("", &t);
    param.set_unit(unit(), unit_name());
    param.set_expression(expression());
    rval.push_back(param.get_value(dummy));
  }
  return rval;
}

template <typename T>
void ModuleParameter<T>::output_impl(std::ostream& os,
                                     std::forward_iterator_tag) const
{
  using iter_type = typename T::const_iterator;
  using value_type = typename std::iterator_traits<iter_type>::value_type;

  for (iter_type it=__ref__().begin(); it!=__ref__().end(); ++it) {
    value_type t = *it;
    ModuleParameter<value_type> param("", &t);
    param.set_unit(unit(), unit_name());
    param.set_expression(expression());
    param.output(os);
    os << " ";
  }
}

template <typename T>
void ModuleParameter<T>::input_impl(std::istream& is,
                                    std::forward_iterator_tag)
{
  using iter_type = typename T::iterator;
  using value_type = typename std::iterator_traits<iter_type>::value_type;

  if (is.eof()) {
    __ref__().clear();
    return;
  }

  T container;
  while (true) {
    value_type t;
    ModuleParameter<value_type> param("", &t);
    param.set_unit(unit(), unit_name());
    param.set_expression(expression());
    is >> param;
    if (is) {
      container.push_back(t);
      if (is.eof()) {
        is.clear();
        break;
      }
    }
    else {
      return;
    }
  }
  __ref__() = std::move(container);
}

} /* namespace anl */
