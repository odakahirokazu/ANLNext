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

#include <memory>

namespace anlnext
{

/**
 * A class template for a module parameter with an owned value.
 * @author Hirokazu Odaka
 * @date 2026-07-29 |
 */
template <typename T>
class ModuleParameterUnique : public ModuleParameter<T>
{
public:
  ModuleParameterUnique(const std::string& name, T value);

  ModuleParameterUnique(const ModuleParameter<T>& definition, const T& value);

  std::shared_ptr<VModuleParameter> clone() override;

protected:
  ModuleParameterUnique(const ModuleParameterUnique& r);

  T& __ref__() override { return *owned_ptr_; }
  const T& __ref__() const override { return *owned_ptr_; }

private:
  std::unique_ptr<T> owned_ptr_;
};

template <typename T>
ModuleParameterUnique<T>::ModuleParameterUnique(const std::string& name, T value)
  : ModuleParameter<T>(name, nullptr),
    owned_ptr_(std::make_unique<T>(value))
{}

template <typename T>
ModuleParameterUnique<T>::ModuleParameterUnique(const ModuleParameter<T>& definition, const T& value)
  : ModuleParameter<T>(definition),
    owned_ptr_(std::make_unique<T>(value))
{}

template <typename T>
std::shared_ptr<VModuleParameter> ModuleParameterUnique<T>::clone()
{ return std::shared_ptr<VModuleParameter>(new ModuleParameterUnique(*this)); }

template <typename T>
ModuleParameterUnique<T>::ModuleParameterUnique(const ModuleParameterUnique& r)
  : ModuleParameter<T>(r),
    owned_ptr_(std::make_unique<T>(r.__ref__()))
{}

template <typename T>
ModuleParam_sptr make_parameter_with_value(const VModuleParameter& definition, const T& value)
{
  const auto* parameter = dynamic_cast<const ModuleParameter<T>*>(&definition);
  if (!parameter) {
    BOOST_THROW_EXCEPTION(ParameterTypeError(&definition, param_type_info<T>::name()));
  }
  return std::make_shared<ModuleParameterUnique<T>>(*parameter, value);
}

} /* namespace anlnext */
