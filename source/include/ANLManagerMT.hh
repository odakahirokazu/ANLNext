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
 */
class ANLManagerMT : public ANLManager
{
public:
  explicit ANLManagerMT(int num_parallels=1);
  virtual ~ANLManagerMT();

  int number_of_parallels() const override { return num_parallels_; }
  BasicModule* access_to_module(int chain_ID,
                                const std::string& module_ID) override;

protected:
  void clone_modules(int chain_ID);

  ANLStatus routine_initialize() override;
  ANLStatus routine_begin_run() override;
  ANLStatus routine_end_run() override;
  ANLStatus routine_finalize() override;

  void print_parameters() override;
  void reset_counters() override;
  
  ANLStatus process_analysis() override;
  virtual void process_analysis_in_each_thread(int i_thread, std::promise<ANLStatus> status_promise);
  virtual long int event_index_to_process();
  void decrement_event_index();

  boost::property_tree::ptree parameters_to_property_tree() const override;

private:
  void duplicate_chains() override;
  ANLStatus process_analysis_impl(const std::vector<BasicModule*>& modules,
                                  std::vector<LoopCounter>& counters,
                                  EvsManager& evs_manager);
  ANLStatus reduce_modules() override;
  void reduce_statistics() override;

private:
  const int num_parallels_ = 1;
  long int loop_index_ = -1;
  std::vector<ClonedChainSet> cloned_chains_;
  std::vector<std::unique_ptr<OrderKeeper>> order_keepers_;
};

} /* namespace anlnext */

#endif /* ANLNEXT_ANLManagerMT_H */
