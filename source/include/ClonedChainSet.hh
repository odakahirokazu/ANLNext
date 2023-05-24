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

#ifndef ANLNEXT_ClonedChainSet_H
#define ANLNEXT_ClonedChainSet_H 1

#include "ANLManager.hh"

namespace anlnext
{

class EvsManager;
class ModuleAccess;
class BasicModule;

/**
 * class for cloned modules chain.
 *
 * @author Hirokazu Odaka
 * @date 2017-07-05
 */
class ClonedChainSet
{
public:
  ClonedChainSet(int chain_id, const EvsManager& evs);
  ~ClonedChainSet();
  ClonedChainSet(ClonedChainSet&&) = default;
  ClonedChainSet& operator=(ClonedChainSet&&) = default;
  
  ClonedChainSet(const ClonedChainSet&) = delete;
  ClonedChainSet& operator=(const ClonedChainSet&) = delete;

  int chain_id() const { return id_; }
  
  void push(std::unique_ptr<BasicModule>&& mod);
  void setup_module_access();
  void reset_counters();

  const std::vector<BasicModule*>& modules_reference() const
  { return modules_ref_; }

  template <typename T>
  ANLStatus process(T func);

  const LoopCounter& get_counter(std::size_t i) const
  { return counters_[i]; }

  const EvsManager& get_evs() const
  { return *evs_manager_; }

  BasicModule* access_to_module(const std::string& module_ID);

  void automatic_switch_for_singletons();
  
private:
  int id_;
  std::unique_ptr<EvsManager> evs_manager_;
  std::unique_ptr<ModuleAccess> module_access_;
  std::vector<std::unique_ptr<BasicModule>> modules_;
  std::vector<BasicModule*> modules_ref_;
  std::vector<LoopCounter> counters_;
};

} /* namespace anlnext */

#endif /* ANLNEXT_ClonedChainSet_H */
