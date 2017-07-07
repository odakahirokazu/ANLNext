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

namespace anl
{

ClonedChainSet::ClonedChainSet(const EvsManager& evs)
  : evsManager_(new EvsManager(evs)),
    moduleAccess_(new ModuleAccess)
{
}

ClonedChainSet::~ClonedChainSet() = default;

void ClonedChainSet::push(std::unique_ptr<BasicModule>&& clonedModule)
{
  std::unique_ptr<BasicModule> m = std::move(clonedModule);
  m->set_evs_manager(evsManager_.get());
  m->set_module_access(moduleAccess_.get());
  modules_ref_.push_back(m.get());
  modules_.push_back(std::move(m));
  counters_.push_back(LoopCounter());
}

void ClonedChainSet::setup_module_access()
{
  for (BasicModule* mod: modules_ref_) {
    const std::string moduleID = mod->module_id();
    moduleAccess_->registerModule(moduleID,
                                  mod,
                                  ModuleAccess::ConflictOption::error);
    
    for (const std::pair<std::string, ModuleAccess::ConflictOption>& alias: mod->get_aliases()) {
      if (alias.first != moduleID) {
        moduleAccess_->registerModule(alias.first,
                                      mod,
                                      alias.second);
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

} /* namespace anl */
