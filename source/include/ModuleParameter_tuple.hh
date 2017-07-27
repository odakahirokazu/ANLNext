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
#include <utility>
#include <tuple>

namespace anlnext
{

struct value_category_scalar {};
struct value_category_pair {};
struct value_category_tuple {};

template <typename T>
struct size_of_value
{
  using type = value_category_scalar;
  constexpr static std::size_t value = 1;
};

template <typename T1, typename T2>
struct size_of_value<std::pair<T1, T2>>
{
  using type = value_category_pair;
  constexpr static std::size_t value = 2;
};

template <typename... Ts>
struct size_of_value<std::tuple<Ts...>>
{
  using type = value_category_tuple;
  using tuple_type = std::tuple<Ts...>;
  constexpr static std::size_t value = std::tuple_size<tuple_type>::value;
};

/**
 * class template for an ANL module parameter. Partial specialization.
 * @author Hirokazu Odaka
 * @date 2017-07-10 | separate file, update according to new design of ModuleParameter
 */
template <typename... Ts>
class ModuleParameter<std::tuple<Ts...>> : public VModuleParameter
{
  using tuple_type = std::tuple<Ts...>;

public:
  ModuleParameter(const std::string& name, tuple_type* ptr)
    : VModuleParameter(name), ptr_(ptr)
  {}

  std::shared_ptr<VModuleParameter> clone() override
  { return std::shared_ptr<VModuleParameter>(new ModuleParameter(*this)); }

protected:
  ModuleParameter(const ModuleParameter&) = default;

public:
  std::string type_name() const override
  {
    std::string t("tuple<");
    t += param_type_info<Ts...>::name();
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

protected:
  virtual tuple_type& __ref__() { return *ptr_; }
  virtual const tuple_type& __ref__() const { return *ptr_; }

private:
  template <int I>
  void set_value_tuple_impl() {}

  template <int I, typename T0, typename... TRest>
  void set_value_tuple_impl(T0 value, TRest... values)
  {
    using float_truth_type =
      boost::integral_constant<bool,
                               boost::is_floating_point<T0>::value>;
    set_value_tuple_one<I>(value, float_truth_type());
    set_value_tuple_impl<I+1>(values...);
  }
  
  template <int I, bool B, typename T0>
  void set_value_tuple_one(T0 value, const boost::integral_constant<bool, B>&)
  {
    // non-floating point
    std::get<I>(__ref__()) = value;
  }

  template <int I, typename T0>
  void set_value_tuple_one(T0 value, const boost::true_type&)
  {
    // floating point
    std::get<I>(__ref__()) = value * unit();
  }

  template <int I>
  void output_tuple_impl(std::ostream&) const {}

  template <int I, typename T0, typename... TRest>
  void output_tuple_impl(std::ostream& os) const
  {
    using float_truth_type =
      boost::integral_constant<bool,
                               boost::is_floating_point<T0>::value>;
    output_tuple_one<I, T0>(os, float_truth_type());
    os << " ";
    output_tuple_impl<I+1, TRest...>(os);
  }

  template <int I, typename T0, bool B>
  void output_tuple_one(std::ostream& os,
                        const boost::integral_constant<bool, B>&) const
  {
    // non-floating point
    os << std::get<I>(__ref__());
  }

  template <int I, typename T0>
  void output_tuple_one(std::ostream& os,
                        const boost::true_type&) const
  {
    // floating point
    os << std::get<I>(__ref__())/unit();
  }

  template <int I>
  void input_tuple_impl(std::istream&) {}

  template <int I, typename T0, typename... TRest>
  void input_tuple_impl(std::istream& is)
  {
    using float_truth_type =
      boost::integral_constant<bool,
                               boost::is_floating_point<T0>::value>;
    input_tuple_one<I, T0>(is, float_truth_type());
    input_tuple_impl<I+1, TRest...>(is);
  }

  template <int I, typename T0, bool B>
  void input_tuple_one(std::istream& is,
                       const boost::integral_constant<bool, B>&)
  {
    // non-floating point
    is >> std::get<I>(__ref__());
  }

  template <int I, typename T0>
  void input_tuple_one(std::istream& is,
                       const boost::true_type&)
  {
    // floating point
    is >> std::get<I>(__ref__());
    if (is) {
      std::get<I>(__ref__()) *= unit();
    }
  }

private:
  std::tuple<Ts...>* ptr_;
};

} /* namespace anlnext */
