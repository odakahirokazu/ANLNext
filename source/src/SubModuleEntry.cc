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

#include "SubModuleEntry.hh"
#include "BasicSubModule.hh"

#include <utility>

namespace anlnext
{

SubModuleEntry::SubModuleEntry(const std::string& name)
  : name_(name)
{}

SubModuleEntry::~SubModuleEntry() = default;

SubModuleEntry::SubModuleEntry(const SubModuleEntry& other)
  : name_(other.name_)
{}

std::unique_ptr<SubModuleEntry> SubModuleEntry::clone() const
{
  return std::unique_ptr<SubModuleEntry>(new SubModuleEntry(*this));
}

void SubModuleEntry::bind(BasicModule* module) const
{
  bind_instance(module, instance_.get());
}

void SubModuleEntry::bind_instance(BasicModule*, BasicSubModule*) const
{}

void SubModuleEntry::set_instance(BasicModule* module, std::unique_ptr<BasicSubModule>&& instance)
{
  bind_instance(module, instance.get());
  instance_ = std::move(instance);
}

std::unique_ptr<SubModuleEntry> SubModuleEntry::duplicate(BasicModule* module) const
{
  auto copy = clone();
  auto instance = (instance_) ? (instance_->duplicate()) : nullptr;
  copy->set_instance(module, std::move(instance));
  return copy;
}

} /* namespace anlnext */
