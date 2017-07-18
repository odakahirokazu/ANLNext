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

#ifndef ANL_ANLManagerMT_H
#define ANL_ANLManagerMT_H 1

#include "ANLManager.hh"

#include "ClonedChainSet.hh"

namespace anl
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

  int number_of_parallels() const override { return NumParallels_; }
  BasicModule* access_to_module(int chainID,
                                const std::string& moduleID) override;

protected:
  void clone_modules(int chainID);

  ANLStatus routine_initialize() override;
  ANLStatus routine_begin_run() override;
  ANLStatus routine_end_run() override;
  ANLStatus routine_finalize() override;

  void print_parameters() override;
  void reset_counters() override;
  
  ANLStatus process_analysis() override;
  virtual void process_analysis_in_each_thread(int iThread, ANLStatus& status);
  virtual long int event_index_to_process();

  boost::property_tree::ptree parameters_to_property_tree() const override;

private:
  void duplicate_chains() override;
  ANLStatus process_analysis_impl(const std::vector<BasicModule*>& modules,
                                  std::vector<LoopCounter>& counters,
                                  EvsManager& evsManager);
  ANLStatus reduce_modules() override;
  void reduce_statistics() override;

private:
  const int NumParallels_ = 1;
  long int loopIndex_ = -1;
  std::vector<ClonedChainSet> clonedChains_;
  std::vector<std::unique_ptr<OrderKeeper>> orderKeepers_;
};

} /* namespace anl */

#endif /* ANL_ANLManagerMT_H */
