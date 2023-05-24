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

#include "ClonedChainSet.hh"
#include "ClonedChainSet_impl.hh"

#include "BasicModule.hh"
#include "EvsManager.hh"
#include "ModuleAccess.hh"

namespace anlnext
{

ClonedChainSet::ClonedChainSet(int chain_id, const EvsManager& evs)
  : id_(chain_id),
    evs_manager_(new EvsManager(evs)),
    module_access_(new ModuleAccess)
{
}

ClonedChainSet::~ClonedChainSet() = default;

void ClonedChainSet::push(std::unique_ptr<BasicModule>&& cloned_module)
{
  std::unique_ptr<BasicModule> m = std::move(cloned_module);
  m->set_evs_manager(evs_manager_.get());
  m->set_module_access(module_access_.get());
  modules_ref_.push_back(m.get());
  modules_.push_back(std::move(m));
  counters_.push_back(LoopCounter());
}

void ClonedChainSet::setup_module_access()
{
  for (BasicModule* mod: modules_ref_) {
    if (mod->access_permission() != ModuleAccess::Permission::privacy) {
      const std::string module_ID = mod->module_id();
      module_access_->register_module(module_ID,
                                      mod,
                                      ModuleAccess::ConflictOption::error);
    
      for (const std::pair<std::string, ModuleAccess::ConflictOption>& alias: mod->get_aliases()) {
        if (alias.first != module_ID) {
          module_access_->register_module(alias.first,
                                          mod,
                                          alias.second);
        }
      }
    }
  }
}

void ClonedChainSet::reset_counters()
{
  for (LoopCounter& c: counters_) {
    c.reset();
  }
}

BasicModule* ClonedChainSet::access_to_module(const std::string& module_ID)
{
  return module_access_->get_module_NC(module_ID);
}

void ClonedChainSet::automatic_switch_for_singletons()
{
  for (auto& mod: modules_) {
    mod->automatic_switch_for_singleton();
  }
}

} /* namespace anlnext */
