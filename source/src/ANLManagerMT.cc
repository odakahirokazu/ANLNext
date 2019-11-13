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

#include <boost/format.hpp>
#include <functional>
#include <thread>

#include "BasicModule.hh"
#include "EvsManager.hh"
#include "ModuleAccess.hh"
#include "ANLException.hh"
#include "ANLManager_impl.hh"
#include "ClonedChainSet_impl.hh"
#include "OrderKeeper.hh"

namespace anlnext
{

ANLManagerMT::ANLManagerMT(int num_parallels)
  : NumParallels_(num_parallels),
    loopIndex_(-1)
{
  set_print_parallel_modules();
}

ANLManagerMT::~ANLManagerMT() = default;

BasicModule* ANLManagerMT::access_to_module(int chainID, const std::string& moduleID)
{
  if (chainID==0) {
    return ANLManager::access_to_module(chainID, moduleID);
  }
  for (auto& chain: clonedChains_) {
    if (chain.chain_id() == chainID) {
      return chain.access_to_module(moduleID);
    }
  }

  BOOST_THROW_EXCEPTION( ANLException((boost::format("Chain does not exist ===> Chain ID: %d") % chainID).str()) );
  return nullptr;
}

void ANLManagerMT::clone_modules(int chainID)
{
  ClonedChainSet chain(chainID, *evsManager_);
  for (BasicModule* mod: modules_) {
    chain.push(mod->clone());
  }
  chain.setup_module_access();
  clonedChains_.push_back(std::move(chain));
}

void ANLManagerMT::duplicate_chains()
{
  orderKeepers_.clear();
  for (BasicModule* mod: modules_) {
    if (mod->is_order_sensitive()) {
      orderKeepers_.emplace_back(new OrderKeeper);
    }
    else {
      orderKeepers_.emplace_back(nullptr);
    }
  }

  for (int i=1; i<NumParallels_; i++) {
    clone_modules(i);
  }
  std::cout << "\n"
            << "<Module chain duplication>\n"
            << (NumParallels_-1) << " chains have been duplicated. => "
            << "Total: " << NumParallels_ << " chains.\n"
            << std::endl;
}

void ANLManagerMT::print_parameters()
{
  ANLManager::print_parameters();

  if (printCloneParameters_) {
    for (auto& chain: clonedChains_) {
      for (const BasicModule* mod: chain.modules_reference()) {
        std::cout << "--- " << mod->module_id() << "[" << chain.chain_id() << "] ---"<< std::endl;
        mod->print_parameters();
        std::cout << std::endl;
      }
    }
  }
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
      status = routine_modfn(&BasicModule::mod_initialize,
                             boost::str(boost::format("initialize:%d")%chain.chain_id()),
                             chain.modules_reference());
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
      status = routine_modfn(&BasicModule::mod_begin_run,
                             boost::str(boost::format("begin_run:%d")%chain.chain_id()),
                             chain.modules_reference());
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
      status = routine_modfn(&BasicModule::mod_end_run,
                             boost::str(boost::format("end_run:%d")%chain.chain_id()),
                             chain.modules_reference());
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
      status = routine_modfn(&BasicModule::mod_finalize,
                             boost::str(boost::format("finalize:%d")%chain.chain_id()),
                             chain.modules_reference());
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
  if (requested_ == ANLRequest::quit) {
    return N;
  }

  return loopIndex_;
}

void ANLManagerMT::decrement_event_index()
{
  std::lock_guard<std::mutex> lock(mutex_);
  --loopIndex_;
}

ANLStatus ANLManagerMT::process_analysis()
{
  std::vector<std::future<ANLStatus>> statusFutureVector;
  std::vector<std::thread> analysisThreads(NumParallels_);
  for (int i=0; i<NumParallels_; i++) {
    std::promise<ANLStatus> statusPromise;
    statusFutureVector.push_back(statusPromise.get_future());
    analysisThreads[i] = std::thread(std::bind(&ANLManagerMT::process_analysis_in_each_thread, this, i, std::placeholders::_1),
                                     std::move(statusPromise));
  }

  for (int i=0; i<NumParallels_; i++) {
    analysisThreads[i].join();
  }

  std::vector<ANLStatus> statusVector(NumParallels_, AS_OK);
  for (int i=0; i<NumParallels_; i++) {
    statusVector[i] = statusFutureVector[i].get();
  }

  ANLStatus status = AS_OK;
  for (ANLStatus s: statusVector) {
    if (s == ANLStatus::critical_error_to_finalize) {
      status = s;
    }
  }
  for (ANLStatus s: statusVector) {
    if (s == ANLStatus::critical_error_to_finalize_from_exception) {
      status = s;
    }
  }
  for (ANLStatus s: statusVector) {
    if (s == ANLStatus::critical_error_to_terminate) {
      status = s;
    }
  }
  for (ANLStatus s: statusVector) {
    if (s == ANLStatus::critical_error_to_terminate_from_exception) {
      status = s;
    }
  }

  return status;
}

void ANLManagerMT::process_analysis_in_each_thread(int iThread, std::promise<ANLStatus> statusPromise)
{
  try {
    ANLStatus status = AS_OK;
    if (iThread==0) {
      status = process_analysis_impl(modules_, counters_, *evsManager_);
    }
    else {
      using std::placeholders::_1;
      using std::placeholders::_2;
      using std::placeholders::_3;
      status = clonedChains_[iThread-1].process(std::bind(&ANLManagerMT::process_analysis_impl, this, _1, _2, _3));
    }
    statusPromise.set_value(status);
  }
  catch (...) {
    if (exception_propagation()) {
      requested_ = ANLRequest::quit;
      statusPromise.set_exception(std::current_exception());
    }
    else {
      throw;
    }
  }
}

ANLStatus ANLManagerMT::process_analysis_impl(const std::vector<BasicModule*>& modules,
                                              std::vector<LoopCounter>& counters,
                                              EvsManager& evsManager)
{
  ANLStatus status = AS_OK;

  const long int displayFrequency = display_frequency();
  const long int numNvents = number_of_loops();

  try {
    while (true) {
      const long int iEvent = event_index_to_process();
      if (iEvent == numNvents) { break; }

      if (displayFrequency != 0 && iEvent%displayFrequency == 0) {
        print_event_index(iEvent);
      }

      status = process_one_event(iEvent, modules, counters, evsManager, orderKeepers_);

      if (is_critical_error(status)) {
        requested_ = ANLRequest::quit;
        return status;
      }

      if (status == AS_QUIT || status == AS_QUIT_ALL) {
        break;
      }

      if (requested_ != ANLRequest::none) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (requested_ == ANLRequest::quit) {
          break;
        }
        else if (requested_ == ANLRequest::show_event_index) {
          print_event_index(iEvent);
        }
        else if (requested_ == ANLRequest::show_evs_summary) {
          print_event_index(iEvent);
          evsManager_->print_summary();
        }
        requested_ = ANLRequest::none;
      }

      if (status==ANLStatus::skip) {
        ;
      }
      else if (status==ANLStatus::redo) {
        decrement_event_index();
      }
    }

    if (status == AS_QUIT_ALL) {
      requested_ = ANLRequest::quit;
    }
  }
  catch (ANLException& ex) {
    if (const ANLException::Treatment* t = boost::get_error_info<ExceptionTreatment>(ex)) {
      if (*t == ANLException::Treatment::rethrow) {
        throw;
      }
      else if (*t == ANLException::Treatment::finalize) {
        requested_ = ANLRequest::quit;
        print_exception(ex);
        return ANLStatus::critical_error_to_finalize_from_exception;
      }
      else if (*t == ANLException::Treatment::terminate) {
        requested_ = ANLRequest::quit;
        print_exception(ex);
        return ANLStatus::critical_error_to_terminate_from_exception;
      }
      else if (*t == ANLException::Treatment::hard_terminate) {
        print_exception(ex);
        std::terminate();
      }
    }
    throw;
  }

  return AS_OK;
}

ANLStatus ANLManagerMT::reduce_modules()
{
  ANLStatus status = AS_OK;
  for (std::size_t iModule=0; iModule<modules_.size(); iModule++) {
    BasicModule* mod = modules_[iModule];
    std::list<BasicModule*> moduleList;
    for (const ClonedChainSet& chain: clonedChains_) {
      moduleList.push_back(chain.modules_reference()[iModule]);
    }
    status = mod->mod_reduce(moduleList);
    if (status != AS_OK) {
      break;
    }
  }
  return status;
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

boost::property_tree::ptree ANLManagerMT::parameters_to_property_tree() const
{
  boost::property_tree::ptree pt = ANLManager::parameters_to_property_tree();
  for (const ClonedChainSet& chain: clonedChains_) {
    boost::property_tree::ptree pt_modules;
    for (const BasicModule* module: chain.modules_reference()) {
      pt_modules.push_back(std::make_pair("", module->parameters_to_property_tree()));
    }
    pt.add_child(boost::str(boost::format("application.chain%d")%chain.chain_id()), std::move(pt_modules));
  }
  return pt;
}

} /* namespace anlnext*/
