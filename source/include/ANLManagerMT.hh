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

#ifndef ANLNEXT_ANLManagerMT_H
#define ANLNEXT_ANLManagerMT_H 1

#include "ANLManager.hh"
#include <future>

#include "ClonedChainSet.hh"

namespace anlnext
{

class EvsManager;
class ModuleAccess;
class BasicModule;

/**
 * The ANL Next manager class for multi-thread mode.
 *
 * @author Hirokazu Odaka
 * @date 2017-07-05
 * @date 2025-12-05 | analysis loop reviewed
 */
class ANLManagerMT : public ANLManager
{
public:
  explicit ANLManagerMT(std::size_t num_parallels=1);
  virtual ~ANLManagerMT();

  std::size_t number_of_parallels() const override { return num_parallels_; }
  BasicModule* access_to_module(std::size_t chain_ID,
                                const std::string& module_ID) override;

protected:
  void clone_modules(std::size_t chain_ID);

  ANLStatus routine_initialize() override;
  ANLStatus routine_begin_run() override;
  ANLStatus routine_end_run() override;
  ANLStatus routine_finalize() override;

  void print_parameters() override;
  void print_results() override;
  void reset_counters() override;
  
  ANLStatus process_analysis() override;
  virtual void process_analysis_in_each_thread(std::size_t i_thread, std::promise<ANLStatus> status_promise);
  virtual std::size_t event_index_to_process();

  boost::property_tree::ptree parameters_to_property_tree() const override;

private:
  void duplicate_chains() override;
  void automatic_switch_for_singletons();
  ANLStatus process_analysis_impl(const std::vector<BasicModule*>& modules,
                                  std::vector<LoopCounter>& counters,
                                  EvsManager& evs_manager);
  ANLStatus reduce_modules() override;
  void reduce_statistics() override;

private:
  const std::size_t num_parallels_ = 1;
  std::size_t loop_index_ = 0;
  std::vector<ClonedChainSet> cloned_chains_;
  std::vector<std::unique_ptr<OrderKeeper>> order_keepers_;
};

} /* namespace anlnext */

#endif /* ANLNEXT_ANLManagerMT_H */
