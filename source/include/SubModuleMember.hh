/*************************************************************************
 *                                                                       *
 * Copyright (c) 2026 Hirokazu Odaka                                     *
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

#ifndef ANLNEXT_SubModuleMember_H
#define ANLNEXT_SubModuleMember_H 1

#include <type_traits>

#include "ANLException.hh"
#include "SubModuleEntry.hh"

namespace anlnext
{

// Implementation detail of BasicModule::define_submodule().
template <typename ModuleClass, typename SubModuleType>
class SubModuleMember : public SubModuleEntry
{
  using member_pointer_type = SubModuleType* ModuleClass::*;
  static_assert(std::is_base_of<BasicModule, ModuleClass>::value,
                "Submodule binding requires a BasicModule owner.");
  static_assert(std::is_base_of<BasicSubModule, SubModuleType>::value,
                "Submodule binding requires a BasicSubModule pointer.");

public:
  SubModuleMember(const std::string& name, const BasicModule* module, member_pointer_type member);

protected:
  SubModuleMember(const SubModuleMember&) = default;
  std::unique_ptr<SubModuleEntry> clone() const override;
  void bind_instance(BasicModule* module, BasicSubModule* submodule) const override;

private:
  member_pointer_type member_;
};

template <typename ModuleClass, typename SubModuleType>
SubModuleMember<ModuleClass, SubModuleType>::SubModuleMember(
  const std::string& name, const BasicModule* module, member_pointer_type member)
  : SubModuleEntry(name), member_(member)
{
  if (!member_) {
    BOOST_THROW_EXCEPTION(SubModuleBindingError(module, name, "Member pointer is null."));
  }
}

template <typename ModuleClass, typename SubModuleType>
std::unique_ptr<SubModuleEntry> SubModuleMember<ModuleClass, SubModuleType>::clone() const
{
  return std::unique_ptr<SubModuleEntry>(new SubModuleMember(*this));
}

template <typename ModuleClass, typename SubModuleType>
void SubModuleMember<ModuleClass, SubModuleType>::bind_instance(BasicModule* module, BasicSubModule* submodule) const
{
  auto* owner = dynamic_cast<ModuleClass*>(module);
  if (!owner) {
    BOOST_THROW_EXCEPTION(SubModuleBindingError(module, name(), "Owner type does not match the member pointer."));
  }
  auto* instance = dynamic_cast<SubModuleType*>(submodule);
  if (submodule && !instance) {
    BOOST_THROW_EXCEPTION(SubModuleBindingError(module, name(), "Submodule type does not match the member pointer."));
  }
  owner->*member_ = instance;
}

} /* namespace anlnext */

#endif /* ANLNEXT_SubModuleMember_H */
