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

#include "ANLManager.hh"

#include <iomanip>
#include <algorithm>
#include <functional>
#include <memory>
#include <thread>
#include <cctype>
#include <cerrno>
#include <limits>

#if ANLNEXT_ANALYZE_INTERRUPT || ANLNEXT_INITIALIZE_INTERRUPT || ANLNEXT_FINALIZE_INTERRUPT
#include "SigintHandlerGuard.hh"
#endif

#include <boost/format.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "BasicModule.hh"
#include "EvsManager.hh"
#include "ModuleAccess.hh"
#include "ANLException.hh"
#include "ANLManager_impl.hh"
#include "OrderKeeper.hh"

#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>


namespace anlnext
{

/* version definition */
const int ANLManager::__version1__ = 3;
const int ANLManager::__version2__ = -2;
const int ANLManager::__version3__ = 8;


ANLManager::ANLManager()
  : print_clone_parameters_(false),
    num_loops_(0),
    evs_manager_(new EvsManager),
    requested_(ANLRequest::none),
    exception_propagation_(true),
    display_period_(0),
    module_access_(new ModuleAccess),
    analysis_thread_finished_(false)
{
  evs_manager_->initialize();
}

ANLManager::~ANLManager() = default;

void ANLManager::set_modules(std::vector<BasicModule*> modules)
{
  modules_ = modules;

  for (BasicModule* mod: modules_) {
    mod->set_evs_manager(evs_manager_.get());
    mod->set_module_access(module_access_.get());
  }

  reset_counters();
}

std::size_t ANLManager::display_period() const
{
  if (display_period_ == 0) {
    return (num_loops_ > 0) ? (num_loops_/100) : 10000;
  }
  return display_period_;
}

ANLStatus ANLManager::define_analysis()
{
  const std::string version = std::to_string(__version1__) + "." +
    std::to_string(__version2__) + "." + std::to_string(__version3__);
  std::cout << '\n'
            << "######################################################\n"
            << "#                                                    #\n"
            << "#          ANL Next Data Analysis Framework          #\n"
            << "#                                                    #\n"
            << boost::format("#    version: %-39s#\n") % version
            << "#    author: Hirokazu Odaka                          #\n"
            << "#    URL: https://github.com/odakahirokazu/ANLNext   #\n"
            << "#                                                    #\n"
            << "######################################################\n"
            << '\n';

  std::cout << '\n'
            << "        **************************************\n"
            << "        ****          Definition          ****\n"
            << "        **************************************\n"
            << '\n'
            << std::flush;

  ANLStatus status = routine_define();
  if (status != AS_OK) {
    goto final;
  }

  for (BasicModule* mod: modules_) {
    if (mod->access_permission() != ModuleAccess::Permission::privacy) {
      const std::string module_id = mod->module_id();
      module_access_->register_module(module_id,
                                      mod,
                                      ModuleAccess::ConflictOption::error);

      for (const std::pair<std::string, ModuleAccess::ConflictOption>& alias: mod->get_aliases()) {
        if (alias.first != module_id) {
          module_access_->register_module(alias.first,
                                          mod,
                                          alias.second);
        }
      }
    }
  }

  module_access_->set_module_order(modules_);
  for (const BasicModule* mod: modules_) {
    mod->check_module_requirements();
  }

  final:
  std::cout << std::endl;

  return status;
}

ANLStatus ANLManager::pre_initialize_analysis()
{
  std::cout << '\n'
            << "        **************************************\n"
            << "        ****      Pre-Initialization      ****\n"
            << "        **************************************\n"
            << '\n'
            << std::flush;

  ANLStatus status =  routine_pre_initialize();
  if (status != AS_OK) {
    goto final;
  }

  duplicate_chains();

  final:
    std::cout << std::endl;

  return status;
}

ANLStatus ANLManager::initialize_analysis()
{
  std::cout << '\n'
            << "        **************************************\n"
            << "        ****        Initialization        ****\n"
            << "        **************************************\n"
            << '\n'
            << std::flush;

#if ANLNEXT_INITIALIZE_INTERRUPT
  SigintHandlerGuard sigint_guard;
  if (!sigint_guard.is_active()) {
    return ANLStatus::critical_error_to_terminate;
  }
#endif

  show_analysis();
  print_parameters();
  run_index_ = undefined_run_index;
  reset_counters();
  requested_ = ANLRequest::none;

  ANLStatus status = routine_initialize();
  if (status != AS_OK) {
    goto final;
  }

  final:
    std::cout << std::endl;

#if ANLNEXT_INITIALIZE_INTERRUPT
  if (!sigint_guard.restore()) {
    return ANLStatus::critical_error_to_terminate;
  }
#endif

  return status;
}

ANLStatus ANLManager::run_analysis(std::size_t num_loops, bool enable_console)
{
  if (run_index_ == undefined_run_index - 1) {
    std::cerr << "ANLManager: run index limit reached.\n";
    return AS_CRITICAL_ERROR_TO_TERMINATE;
  }

  run_index_ = (run_index_ == undefined_run_index) ? 0 : (run_index_ + 1);
  num_loops_ = num_loops;
  requested_ = ANLRequest::none;
  prepare_run();

  std::cout << '\n'
            << "        **************************************\n"
            << "        ****        Main Analysis         ****\n"
            << "        **************************************\n"
            << '\n';

  if (num_loops==std::numeric_limits<std::size_t>::max()) {
    std::cout << "Number of loops: " << "max" << '\n';
  }
  else {
    std::cout << "Number of loops: " << num_loops << '\n';
  }

  std::cout << '\n' << std::flush;

#if ANLNEXT_ANALYZE_INTERRUPT
  SigintHandlerGuard sigint_guard;
  if (!sigint_guard.is_active()) {
    return ANLStatus::critical_error_to_terminate;
  }
#endif

  ANLStatus status = AS_OK;

  status = routine_begin_run();
  if (status != AS_OK) {
    goto final;
  }

  if (enable_console) {
    std::cout << '\n'
              << "ANLManager: starting analysis loop (with user-console mode on).\n"
              << "----------------------------------------------------------------------------\n"
              << "  input '.q' => quit the analysis loop\n"
              << "  input '.i' => show the current loop index\n"
              << "  input '.s' => show the status of event selections (of the master thread)\n"
              << "----------------------------------------------------------------------------\n"
              << '\n'
              << std::flush;

    analysis_thread_finished_ = false;
    std::thread interactive_thread(std::bind(&ANLManager::interactive_session, this));

    try {
      const bool use_analysis_thread = false;
      if (use_analysis_thread) {
        std::promise<ANLStatus> status_promise;
        std::future<ANLStatus> status_future = status_promise.get_future();
        std::thread analysis_thread(std::bind(&ANLManager::process_analysis_for_the_thread,
                                              this,
                                              std::placeholders::_1),
                                    std::move(status_promise));
        analysis_thread.join();
        status = status_future.get();
      }
      else {
        status = process_analysis();
      }
    }
    catch (...) {
      analysis_thread_finished_ = true;
      interactive_thread.join();
      throw;
    }
    analysis_thread_finished_ = true;
    interactive_thread.join();
  }
  else {
    std::cout << '\n'
              << "ANLManager: starting analysis loop.\n\n"
              << std::flush;
    status = process_analysis();
  }

  if (status != AS_OK) {
    goto final;
  }

  std::cout << '\n'
            << "ANLManager: analysis loop successfully done.\n\n"
            << std::flush;

  status = routine_end_run();
  if (status != AS_OK) {
    goto final;
  }

  status = reduce_modules();

final:
  std::cout << '\n' << std::flush;
  reduce_statistics();
  print_summary();
  evs_manager_->print_summary();
  print_results();
  requested_ = ANLRequest::none;

#if ANLNEXT_ANALYZE_INTERRUPT
  if (!sigint_guard.restore()) {
    return ANLStatus::critical_error_to_terminate;
  }
#endif

  return status;
}

ANLStatus ANLManager::finalize_analysis()
{
  std::cout << '\n'
            << "        **************************************\n"
            << "        ****         Finalization         ****\n"
            << "        **************************************\n"
            << '\n'
            << std::flush;

#if ANLNEXT_FINALIZE_INTERRUPT
  SigintHandlerGuard sigint_guard;
  if (!sigint_guard.is_active()) {
    return ANLStatus::critical_error_to_terminate;
  }
#endif

  ANLStatus status = routine_finalize();
  if (status != AS_OK) {
    goto final;
  }

  final:
    std::cout << std::endl;

#if ANLNEXT_FINALIZE_INTERRUPT
  if (!sigint_guard.restore()) {
    return ANLStatus::critical_error_to_terminate;
  }
#endif

  return status;
}

BasicModule* ANLManager::access_to_module(std::size_t chain_id,
                                          const std::string& module_id)
{
  if (chain_id == 0) {
    return module_access_->get_module_non_const(module_id);
  }

  BOOST_THROW_EXCEPTION( ChainNotFoundError(chain_id) );
  return nullptr;
}

std::size_t ANLManager::module_index(const std::string& module_id, bool strict) const
{
  std::size_t index = 0;
  if (strict) {
    AMConstIter module_iter = std::find_if(std::begin(modules_), std::end(modules_),
                                           [&](const BasicModule* m){ return (m->module_id() == module_id); });
    index = std::distance(modules_.begin(), module_iter);
  }
  else {
    std::string lower1(module_id);
    std::transform(lower1.begin(), lower1.end(), lower1.begin(),
                   [](unsigned char c){ return std::tolower(c);});
    AMConstIter module_iter = std::find_if(std::begin(modules_), std::end(modules_),
                                           [&](const BasicModule* m) {
                                             std::string lower2(m->module_id());
                                             std::transform(lower2.begin(), lower2.end(), lower2.begin(),
                                                            [](unsigned char c){ return std::tolower(c);});
                                             return (lower2 == lower1); });
    index = std::distance(modules_.begin(), module_iter);
  }
  return index;
}

void ANLManager::show_analysis()
{
  std::cout << '\n'
            << "        **************************************\n"
            << "        ****        Analysis chain        ****\n"
            << "        **************************************\n"
            << '\n';
  
  if (modules_.size() < 1) {
    std::cout << "No analysis chain is defined. \n";
  }
  else {
    std::cout
      << "   #" << "    "
      << "    Module ID                                " << "  "
      << " Version " << "  " << " ON/OFF \n"
      << "----------------------------------------------------------------------------\n";

    for (std::size_t i=0; i<modules_.size(); i++) {
      std::cout << std::right << std::setw(4) << i << "    ";
      std::string module_id(modules_[i]->module_id());
      if (modules_[i]->module_id() != modules_[i]->module_name()) {
        module_id += " (" + modules_[i]->module_name() + ")";
      }
      std::cout << std::left << std::setw(48) << module_id;
      std::cout << "  "
                << std::left << std::setw(9) << modules_[i]->module_version()
                << "  "
                << std::left << std::setw(8)
                << (modules_[i]->is_on() ? "ON" : "OFF")
                << '\n';
    }
  }
  std::cout << std::right << '\n' << std::flush;
}

void ANLManager::print_parameters()
{
  std::cout << '\n'
            << "        **************************************\n"
            << "        ****      Module parameters       ****\n"
            << "        **************************************\n"
            << '\n';
  
  for (BasicModule* mod: modules_) {
    std::cout << "--- " << mod->module_id() << " ---\n";
    mod->print_parameters();
    std::cout << '\n';
  }
  std::cout << std::flush;
}

void ANLManager::print_results()
{
  std::cout << '\n'
            << "        **************************************\n"
            << "        ****       Module results         ****\n"
            << "        **************************************\n"
            << '\n';
  
  for (BasicModule* mod: modules_) {
    std::cout << "--- " << mod->module_id() << " ---\n";
    mod->print_results();
    std::cout << '\n';
  }
  std::cout << std::flush;
}

void ANLManager::reset_counters()
{
  counters_.resize(modules_.size());
  for (LoopCounter& c: counters_) {
    c.reset();
  }
  evs_manager_->reset_all_counts();
  evs_manager_->reset_all_flags();
}

void ANLManager::prepare_run()
{
  reset_counters();
  for (BasicModule* mod: modules_) {
    mod->set_run_index(run_index_);
    mod->set_loop_index(BasicModule::undefined_loop_index);
  }
}

ANLStatus ANLManager::process_analysis()
{
  ANLStatus status = AS_OK;

  const std::vector<BasicModule*>& modules = modules_;
  const std::size_t period_disp = display_period();
  const std::size_t num_loops = number_of_loops();

  try {
    for (std::size_t loop_index=0; loop_index<num_loops; ++loop_index) {
      if (period_disp != 0 && loop_index%period_disp == 0) {
        print_loop_index(loop_index);
      }

      status = process_one_iteration(loop_index, modules, counters_, *evs_manager_);
      
      if (is_critical_error(status)) {
        return status;
      }

      if (status==AS_QUIT || status==AS_QUIT_ALL) {
        break;
      }

      if (requested_ != ANLRequest::none) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (requested_ == ANLRequest::quit) {
          break;
        }
        else if (requested_ == ANLRequest::show_loop_index) {
          print_loop_index(loop_index);
        }
        else if (requested_ == ANLRequest::show_evs_summary) {
          print_loop_index(loop_index);
          evs_manager_->print_summary();
        }
        requested_ = ANLRequest::none;
      }
    }
  }
  catch (ANLException& ex) {
    if (const ANLException::Treatment* t = boost::get_error_info<ExceptionTreatment>(ex)) {
      if (*t == ANLException::Treatment::rethrow) {
        throw;
      }
      else if (*t == ANLException::Treatment::finalize) {
        print_exception(ex);
        return ANLStatus::critical_error_to_finalize_from_exception;
      }
      else if (*t == ANLException::Treatment::terminate) {
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

void ANLManager::print_summary()
{
  const std::size_t n = modules_.size();
  std::cout << '\n'
            << "        **************************************\n"
            << "        ****        Analysis chain        ****\n"
            << "        **************************************\n"
            << "               Put: " << counters_[0].entry() << '\n'
            << "                |\n";

  for (std::size_t i=0; i<n; i++) {
    std::string module_id = modules_[i]->module_name();
    if (modules_[i]->module_id() != modules_[i]->module_name()) {
      module_id += "/" + modules_[i]->module_id();
    }
    module_id += "  version  " + modules_[i]->module_version();
    std::cout << boost::format("    [%4d]  %-40s") % i % module_id;
    if (counters_[i].quit() > 0) { std::cout << "         ---> [Quit]"; }
    std::cout <<  '\n';
    std::cout << boost::format("    %10d  |  OK: %10d | Skip: %10d | Error: %10d")
      % counters_[i].entry()
      % counters_[i].ok()
      % counters_[i].skip()
      % counters_[i].error();
    std::cout << '\n';
  }
  std::cout << "               Get: " << counters_[n-1].ok() << '\n';
  std::cout << '\n' << std::flush;
}

boost::property_tree::ptree ANLManager::parameters_to_property_tree() const
{
  boost::property_tree::ptree pt;
  boost::property_tree::ptree pt_modules;
  for (const auto& module: modules_) {
    pt_modules.push_back(std::make_pair("", module->parameters_to_property_tree()));
  }
  pt.add_child("application.module_list", std::move(pt_modules));
  return pt;
}

void ANLManager::parameters_to_json(const std::string& filename) const
{
  boost::property_tree::ptree pt = parameters_to_property_tree();
  write_json(filename.c_str(), pt);
}

ANLStatus ANLManager::routine_define()
{
  return routine_modfn(&BasicModule::mod_define, "define", modules_);
}

ANLStatus ANLManager::routine_pre_initialize()
{
  return routine_modfn(&BasicModule::mod_pre_initialize, "pre_initialize", modules_);
}

ANLStatus ANLManager::routine_initialize()
{
  return routine_modfn(&BasicModule::mod_initialize, "initialize", modules_);
}

ANLStatus ANLManager::routine_begin_run()
{
  return routine_modfn(&BasicModule::mod_begin_run, "begin_run", modules_);
}

ANLStatus ANLManager::routine_end_run()
{
  return routine_modfn(&BasicModule::mod_end_run, "end_run", modules_);
}

ANLStatus ANLManager::routine_finalize()
{
  return routine_modfn(&BasicModule::mod_finalize, "finalize", modules_);
}

void ANLManager::process_analysis_for_the_thread(std::promise<ANLStatus> status_promise)
{
  try {
    ANLStatus s = process_analysis();
    status_promise.set_value(s);
  }
  catch (...) {
    if (exception_propagation()) {
      status_promise.set_exception(std::current_exception());
    }
    else {
      throw;
    }
  }
}

void ANLManager::interactive_session()
{
  std::string line;
  std::cout << "ANL> " << std::flush;
  while (!analysis_thread_finished_) {
    fd_set read_fd_set;
    FD_ZERO(&read_fd_set);
    FD_SET(STDIN_FILENO, &read_fd_set);
    struct timeval timeout{1, 0};
    const int ready = select(STDIN_FILENO+1, &read_fd_set, nullptr, nullptr, &timeout);
    if (ready < 0) {
      if (errno == EINTR) { continue; }
      std::cout << "Error by select() in ANLManager::interactive_session()\n" << std::flush;
      return;
    }
    if (analysis_thread_finished_) { return; }
    if (ready == 0) { continue; }

    // Read available characters rather than waiting for a complete input line.
    char buffer[256];
    ssize_t count = read(STDIN_FILENO, buffer, sizeof(buffer));
    if (count < 0) {
      if (errno == EINTR) { continue; }
      std::cout << "Error by read() in ANLManager::interactive_session()\n" << std::flush;
      return;
    }
    const bool eof = count == 0;
    if (eof) {
      if (line.empty()) { return; }
      buffer[0] = '\n';
      count = 1;
    }
    for (ssize_t i=0; i<count && !analysis_thread_finished_; ++i) {
      if (buffer[i] != '\n') {
        line += buffer[i];
        continue;
      }
      while (!line.empty() && std::isspace(static_cast<unsigned char>(line.back()))) {
        line.pop_back();
      }
      if (line == ".q") {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << " ---> Quit\n\n" << std::flush;
        requested_ = ANLRequest::quit;
        return;
      }
      else if (line == ".i") {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << " ---> Show loop index\n\n" << std::flush;
        requested_ = ANLRequest::show_loop_index;
      }
      else if (line == ".s") {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << " ---> Show evs summary\n\n" << std::flush;
        requested_ = ANLRequest::show_evs_summary;
      }
      line.clear();
      if (!eof) { std::cout << "ANL> " << std::flush; }
    }
    if (eof) { return; }
  }
}

ANLStatus process_one_iteration(std::size_t loop_index,
                                const std::vector<BasicModule*>& modules,
                                std::vector<LoopCounter>& counters,
                                EvsManager& evs_manager)
{
  evs_manager.reset_all_flags();
  ANLStatus status = AS_OK;

  for (BasicModule* mod: modules) {
    mod->set_loop_index(loop_index);
  }

  const std::size_t num_modules = modules.size();
  for (std::size_t module_index=0; module_index<num_modules; module_index++) {
    BasicModule* mod = modules[module_index];

    if (mod->is_on()) {
      counters[module_index].count_up_by_entry();

      try {
        status = mod->mod_analyze();
      }
      catch (boost::exception& ex) {
        ex << ErrorInfoOnLoopIndex(loop_index);
        ex << ErrorInfoOnMethod( mod->module_name() + "::mod_analyze" );
        ex << ErrorInfoOnModuleID( mod->module_id() );
        ex << ErrorInfoOnModuleName( mod->module_name() );
        ex << ErrorInfoOnChainID( mod->copy_id() );
        throw;
      }

      counters[module_index].count_up_by_result(status);
      status = eliminate_normal_error_status(status);

      if (status != AS_OK) {
        break;
      }
    }
  }

  count_evs(status, evs_manager);
  return status;
}

ANLStatus process_one_iteration(std::size_t loop_index,
                                const std::vector<BasicModule*>& modules,
                                std::vector<LoopCounter>& counters,
                                EvsManager& evs_manager,
                                std::vector<std::unique_ptr<OrderKeeper>>& order_keepers)
{
  evs_manager.reset_all_flags();
  ANLStatus status = AS_OK;

  for (BasicModule* mod: modules) {
    mod->set_loop_index(loop_index);
  }

  const std::size_t num_modules = modules.size();
  for (std::size_t module_index=0; module_index<num_modules; module_index++) {
    BasicModule* mod = modules[module_index];
  
    const KeeperBlock<OrderKeeper, std::size_t> block(order_keepers[module_index].get(), loop_index);
    if (!block.can_proceed()) {
      return is_critical_error(status) ? status : AS_QUIT_ALL;
    }

    if (status == AS_OK && mod->is_on()) {
      counters[module_index].count_up_by_entry();

      try {
        status = mod->mod_analyze();
      }
      catch (ANLException& ex) {
        ex << ErrorInfoOnLoopIndex(loop_index);
        ex << ErrorInfoOnMethod( mod->module_name() + "::mod_analyze" );
        ex << ErrorInfoOnModuleID( mod->module_id() );
        ex << ErrorInfoOnModuleName( mod->module_name() );
        ex << ErrorInfoOnChainID( mod->copy_id() );
        throw;
      }

      counters[module_index].count_up_by_result(status);
      status = eliminate_normal_error_status(status);
    }
  }

  count_evs(status, evs_manager);
  return status;
}

void count_evs(ANLStatus status, EvsManager& evs_manager)
{
  if (status == AS_OK) {
    evs_manager.count();
    evs_manager.count_completed();
  }
  else if(status == AS_SKIP) {
    evs_manager.count();
  }
  else if (status == AS_QUIT) {
    evs_manager.count();
  }
  else if (status == AS_QUIT_ALL) {
    evs_manager.count();
  }
}

} /* namespace anlnext */
