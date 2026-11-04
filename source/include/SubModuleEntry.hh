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

#ifndef ANLNEXT_SubModuleEntry_H
#define ANLNEXT_SubModuleEntry_H 1

#include <memory>
#include <string>

namespace anlnext
{

class BasicModule;
class BasicSubModule;

// Implementation detail of BasicModule's submodule registration.
class SubModuleEntry
{
public:
  explicit SubModuleEntry(const std::string& name);
  virtual ~SubModuleEntry();

  const std::string& name() const { return name_; }
  BasicSubModule* instance() { return instance_.get(); }
  const BasicSubModule* instance() const { return instance_.get(); }

  void bind(BasicModule* module) const;
  void set_instance(BasicModule* module, std::unique_ptr<BasicSubModule>&& instance);
  std::unique_ptr<SubModuleEntry> duplicate(BasicModule* module) const;

protected:
  // Copy only the registration metadata; duplicate() also copies the instance.
  SubModuleEntry(const SubModuleEntry& other);
  virtual std::unique_ptr<SubModuleEntry> clone() const;
  virtual void bind_instance(BasicModule* module, BasicSubModule* instance) const;

private:
  std::string name_;
  std::unique_ptr<BasicSubModule> instance_;
};

} /* namespace anlnext */

#endif /* ANLNEXT_SubModuleEntry_H */
