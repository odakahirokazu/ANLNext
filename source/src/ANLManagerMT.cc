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
  : num_parallels_(num_parallels),
    loop_index_(-1)
{
  set_print_parallel_modules();
}

ANLManagerMT::~ANLManagerMT() = default;

BasicModule* ANLManagerMT::access_to_module(int chain_ID, const std::string& module_ID)
{
  if (chain_ID==0) {
    return ANLManager::access_to_module(chain_ID, module_ID);
  }
  for (auto& chain: cloned_chains_) {
    if (chain.chain_id() == chain_ID) {
      return chain.access_to_module(module_ID);
    }
  }

  BOOST_THROW_EXCEPTION( ANLException((boost::format("Chain does not exist ===> Chain ID: %d") % chain_ID).str()) );
  return nullptr;
}

void ANLManagerMT::clone_modules(int chain_ID)
{
  ClonedChainSet chain(chain_ID, *evs_manager_);
  for (BasicModule* mod: modules_) {
    chain.push(mod->clone());
  }
  chain.setup_module_access();
  cloned_chains_.push_back(std::move(chain));
}

void ANLManagerMT::duplicate_chains()
{
  order_keepers_.clear();
  for (BasicModule* mod: modules_) {
    if (mod->is_order_sensitive()) {
      order_keepers_.emplace_back(new OrderKeeper);
    }
    else {
      order_keepers_.emplace_back(nullptr);
    }
  }

  for (int i=1; i<num_parallels_; i++) {
    clone_modules(i);
  }
  std::cout << "\n"
            << "<Module chain duplication>\n"
            << (num_parallels_-1) << " chains have been duplicated. => "
            << "Total: " << num_parallels_ << " chains.\n"
            << std::endl;
}

void ANLManagerMT::print_parameters()
{
  ANLManager::print_parameters();

  if (print_clone_parameters_) {
    for (auto& chain: cloned_chains_) {
      for (const BasicModule* mod: chain.modules_reference()) {
        std::cout << "--- " << mod->module_id() << "[" << chain.chain_id() << "] ---"<< std::endl;
        mod->print_parameters();
        std::cout << std::endl;
      }
    }
  }
}

void ANLManagerMT::print_results()
{
  ANLManager::print_results();

  if (print_clone_parameters_) {
    for (auto& chain: cloned_chains_) {
      for (const BasicModule* mod: chain.modules_reference()) {
        std::cout << "--- " << mod->module_id() << "[" << chain.chain_id() << "] ---"<< std::endl;
        mod->print_results();
        std::cout << std::endl;
      }
    }
  }
}

void ANLManagerMT::reset_counters()
{
  ANLManager::reset_counters();
  for (auto& chain: cloned_chains_) {
    chain.reset_counters();
  }
}

ANLStatus ANLManagerMT::routine_initialize()
{
  ANLStatus status = AS_OK;
  status = ANLManager::routine_initialize();
  if (status == AS_OK) {
    for (auto& chain: cloned_chains_) {
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
    for (auto& chain: cloned_chains_) {
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
    for (auto& chain: cloned_chains_) {
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
    for (auto& chain: cloned_chains_) {
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
  ++loop_index_;
  if (loop_index_ >= N) {
    return N;
  }
  if (requested_ == ANLRequest::quit) {
    return N;
  }

  return loop_index_;
}

void ANLManagerMT::decrement_event_index()
{
  std::lock_guard<std::mutex> lock(mutex_);
  --loop_index_;
}

ANLStatus ANLManagerMT::process_analysis()
{
  std::vector<std::future<ANLStatus>> status_future_vector;
  std::vector<std::thread> analysis_threads(num_parallels_);
  for (int i=0; i<num_parallels_; i++) {
    std::promise<ANLStatus> status_promise;
    status_future_vector.push_back(status_promise.get_future());
    analysis_threads[i] = std::thread(std::bind(&ANLManagerMT::process_analysis_in_each_thread, this, i, std::placeholders::_1),
                                      std::move(status_promise));
  }

  for (int i=0; i<num_parallels_; i++) {
    analysis_threads[i].join();
  }

  std::vector<ANLStatus> status_vector(num_parallels_, AS_OK);
  for (int i=0; i<num_parallels_; i++) {
    status_vector[i] = status_future_vector[i].get();
  }

  ANLStatus status = AS_OK;
  for (ANLStatus s: status_vector) {
    if (s == ANLStatus::critical_error_to_finalize) {
      status = s;
    }
  }
  for (ANLStatus s: status_vector) {
    if (s == ANLStatus::critical_error_to_finalize_from_exception) {
      status = s;
    }
  }
  for (ANLStatus s: status_vector) {
    if (s == ANLStatus::critical_error_to_terminate) {
      status = s;
    }
  }
  for (ANLStatus s: status_vector) {
    if (s == ANLStatus::critical_error_to_terminate_from_exception) {
      status = s;
    }
  }

  return status;
}

void ANLManagerMT::process_analysis_in_each_thread(int i_thread, std::promise<ANLStatus> status_promise)
{
  try {
    ANLStatus status = AS_OK;
    if (i_thread==0) {
      status = process_analysis_impl(modules_, counters_, *evs_manager_);
    }
    else {
      using std::placeholders::_1;
      using std::placeholders::_2;
      using std::placeholders::_3;
      status = cloned_chains_[i_thread-1].process(std::bind(&ANLManagerMT::process_analysis_impl, this, _1, _2, _3));
    }
    status_promise.set_value(status);
  }
  catch (...) {
    if (exception_propagation()) {
      requested_ = ANLRequest::quit;
      status_promise.set_exception(std::current_exception());
    }
    else {
      throw;
    }
  }
}

ANLStatus ANLManagerMT::process_analysis_impl(const std::vector<BasicModule*>& modules,
                                              std::vector<LoopCounter>& counters,
                                              EvsManager& evs_manager)
{
  ANLStatus status = AS_OK;

  const long int period_disp = display_period();
  const long int num_events = number_of_loops();

  try {
    while (true) {
      const long int i_event = event_index_to_process();
      if (i_event == num_events) { break; }

      if (period_disp != 0 && i_event%period_disp == 0) {
        print_event_index(i_event);
      }

      status = process_one_event(i_event, modules, counters, evs_manager, order_keepers_);

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
          print_event_index(i_event);
        }
        else if (requested_ == ANLRequest::show_evs_summary) {
          print_event_index(i_event);
          evs_manager_->print_summary();
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
  for (std::size_t i_module=0; i_module<modules_.size(); i_module++) {
    BasicModule* mod = modules_[i_module];
    std::list<BasicModule*> module_list;
    for (const ClonedChainSet& chain: cloned_chains_) {
      module_list.push_back(chain.modules_reference()[i_module]);
    }
    status = mod->mod_reduce(module_list);
    if (status != AS_OK) {
      break;
    }
  }
  return status;
}

void ANLManagerMT::reduce_statistics()
{
  for (const ClonedChainSet& chain: cloned_chains_) {
    for (std::size_t i=0; i<modules_.size(); i++) {
      counters_[i] += chain.get_counter(i);
    }
    evs_manager_->merge(chain.get_evs());
  }
}

boost::property_tree::ptree ANLManagerMT::parameters_to_property_tree() const
{
  boost::property_tree::ptree pt = ANLManager::parameters_to_property_tree();
  for (const ClonedChainSet& chain: cloned_chains_) {
    boost::property_tree::ptree pt_modules;
    for (const BasicModule* module: chain.modules_reference()) {
      pt_modules.push_back(std::make_pair("", module->parameters_to_property_tree()));
    }
    pt.add_child(boost::str(boost::format("application.chain%d")%chain.chain_id()), std::move(pt_modules));
  }
  return pt;
}

} /* namespace anlnext*/
