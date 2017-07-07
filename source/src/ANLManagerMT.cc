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

#include "ANLManagerMT.hh"

#include <functional>
#include <thread>

#include "BasicModule.hh"
#include "EvsManager.hh"
#include "ModuleAccess.hh"
#include "ANLException.hh"
#include "ANLManager_impl.hh"
#include "ClonedChainSet_impl.hh"

namespace anl
{

ANLManagerMT::ANLManagerMT(int num_parallels)
  : NumParallels_(num_parallels)
{
}

ANLManagerMT::~ANLManagerMT() = default;

void ANLManagerMT::clone_modules()
{
  ClonedChainSet chain(*evsManager_);
  for (BasicModule* mod: modules_) {
    chain.push(mod->clone());
  }
  chain.setup_module_access();
  clonedChains_.push_back(std::move(chain));
}

void ANLManagerMT::duplicate_chains()
{
  for (int i=1; i<NumParallels_; i++) {
    clone_modules();
  }
  std::cout << "\n"
            << "<Module chain duplication>\n"
            << (NumParallels_-1) << " chains have been duplicated. => "
            << "Total: " << NumParallels_ << " chains.\n"
            << std::endl;
}

void ANLManagerMT::reset_counters()
{
  ANLManager::reset_counters();
  for (auto& chain: clonedChains_) {
    chain.reset_counters();
  }
}

ANLStatus ANLManagerMT::routine_initialize()
{
  ANLStatus status = AS_OK;
  status = ANLManager::routine_initialize();
  if (status == AS_OK) {
    for (auto& chain: clonedChains_) {
      status = routine_modfn(&BasicModule::mod_initialize, "initialize", chain.modules_reference());
      if (status != AS_OK) { break; }
    }
  }
  return status;
}

ANLStatus ANLManagerMT::routine_begin_run()
{
  ANLStatus status = AS_OK;
  status = ANLManager::routine_begin_run();
  if (status == AS_OK) {
    for (auto& chain: clonedChains_) {
      status = routine_modfn(&BasicModule::mod_begin_run, "begin_run", chain.modules_reference());
      if (status != AS_OK) { break; }
    }
  }
  return status;
}

ANLStatus ANLManagerMT::routine_end_run()
{
  ANLStatus status = AS_OK;
  status = ANLManager::routine_end_run();
  if (status == AS_OK) {
    for (auto& chain: clonedChains_) {
      status = routine_modfn(&BasicModule::mod_end_run, "end_run", chain.modules_reference());
      if (status != AS_OK) { break; }
    }
  }
  return status;
}

ANLStatus ANLManagerMT::routine_finalize()
{
  ANLStatus status = AS_OK;
  status = ANLManager::routine_finalize();
  if (status == AS_OK) {
    for (auto& chain: clonedChains_) {
      status = routine_modfn(&BasicModule::mod_finalize, "finalize", chain.modules_reference());
      if (status != AS_OK) { break; }
    }
  }
  return status;
}

long int ANLManagerMT::event_index_to_process()
{
  std::lock_guard<std::mutex> lock(mutex_);

  const long int N = number_of_loops();
  ++loopIndex_;
  if (loopIndex_ >= N) {
    return N;
  }
  if (interrupted_) {
    return N;
  }
  return loopIndex_;
}

ANLStatus ANLManagerMT::process_analysis()
{
  std::vector<ANLStatus> statusVector(NumParallels_, AS_OK);
  std::vector<std::thread> analysisThreads(NumParallels_);
  for (int i=0; i<NumParallels_; i++) {
    analysisThreads[i] = std::thread(std::bind(&ANLManagerMT::process_analysis_in_each_thread, this, i, statusVector[i]));
  }

  for (int i=0; i<NumParallels_; i++) {
    analysisThreads[i].join();
  }

  ANLStatus status = AS_OK;
  for (ANLStatus s: statusVector) {
    if (s == AS_SKIP_ERROR || s == AS_QUIT_ERROR || s == AS_QUIT_ALL_ERROR) {
      status = s;
    }
  }

  return status;
}

void ANLManagerMT::process_analysis_in_each_thread(int iThread, ANLStatus& status)
{
  status = AS_OK;
  if (iThread==0) {
    status = process_analysis_impl(modules_, counters_, *evsManager_);
  }
  else {
    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;
    status = clonedChains_[iThread-1].process(std::bind(&ANLManagerMT::process_analysis_impl, this, _1, _2, _3));
  }
}

ANLStatus ANLManagerMT::process_analysis_impl(const std::vector<BasicModule*>& modules,
                                              std::vector<LoopCounter>& counters,
                                              EvsManager& evsManager)
{
  ANLStatus status = AS_OK;

  const long int displayFrequency = display_frequency();
  const long int numNvents = number_of_loops();

  while (true) {
    const long int iEvent = event_index_to_process();
    if (iEvent == numNvents) { break; }

    if (displayFrequency != 0 && iEvent%displayFrequency == 0) {
      std::cout << "Event : " << std::dec << std::setw(10) << iEvent << std::endl;
      std::cout.width(0);
    }

    status = process_one_event(iEvent, modules, counters, evsManager);

    if (status == AS_QUIT || status == AS_QUIT_ERROR) {
      break;
    }

    if (status == AS_QUIT_ALL || status == AS_QUIT_ALL_ERROR) {
      break;
    }
  }

  if (status == AS_QUIT_ALL || status == AS_QUIT_ALL_ERROR) {
    std::lock_guard<std::mutex> lock(mutex_);
    interrupted_ = true;
    return status;
  }

  if (status==AS_SKIP_ERROR || status==AS_QUIT_ERROR) {
    return status;
  }

  return AS_OK;
}

void ANLManagerMT::reduce_statistics()
{
  for (const ClonedChainSet& chain: clonedChains_) {
    for (std::size_t i=0; i<modules_.size(); i++) {
      counters_[i] += chain.get_counter(i);
    }
    evsManager_->merge(chain.get_evs());
  }
}

} /* namespace anl*/
