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

#if ANLNEXT_ANALYZE_INTERRUPT || ANL_INITIALIZE_INTERRUPT || ANL_FINALIZE_INTERRUPT
#include <csignal>
#include <cstring>
#endif

#include <boost/format.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "BasicModule.hh"
#include "EvsManager.hh"
#include "ModuleAccess.hh"
#include "ANLException.hh"
#include "ANLManager_impl.hh"
#include "OrderKeeper.hh"

#if ANLNEXT_USE_READLINE
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include "CLIUtility.hh"
#endif /* ANLNEXT_USE_READLINE */


namespace anlnext
{

/* version definition */
const int ANLManager::__version1__ = 2;
const int ANLManager::__version2__ = 1;
const int ANLManager::__version3__ = 0;


ANLManager::ANLManager()
  : print_clone_parameters_(false),
    num_events_(0),
    evs_manager_(new EvsManager),
    requested_(ANLRequest::none),
    exception_propagation_(true),
    display_period_(-1),
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

long int ANLManager::display_period() const
{
  if (display_period_ < 0) {
    return (num_events_ > 0) ? (num_events_/100) : 10000;
  }
  return display_period_;
}

ANLStatus ANLManager::Define()
{
  std::cout << '\n'
            << "######################################################\n"
            << "#                                                    #\n"
            << "#          ANL Next Data Analysis Framework          #\n"
            << "#                                                    #\n"
            <<
    boost::format("#    version: %2d.%02d.%02d%31s#\n")
    % __version1__ % __version2__ % __version3__ % " "
            << "#    author: Hirokazu Odaka                          #\n"
            << "#    URL: https://github.com/odakahirokazu/ANLNext   #\n"
            << "#                                                    #\n"
            << "######################################################\n"
            << std::endl;

  std::cout << '\n'
            << "        **************************************\n"
            << "        ****          Definition          ****\n"
            << "        **************************************\n"
            << std::endl;

  ANLStatus status = routine_define();
  if (status != AS_OK) {
    goto final;
  }

  for (BasicModule* mod: modules_) {
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

  final:
    std::cout << std::endl;
  return status;
}

ANLStatus ANLManager::PreInitialize()
{
  std::cout << '\n'
            << "        **************************************\n"
            << "        ****      Pre-Initialization      ****\n"
            << "        **************************************\n"
            << std::endl;

  ANLStatus status =  routine_pre_initialize();
  if (status != AS_OK) {
    goto final;
  }

  duplicate_chains();

  final:
    std::cout << std::endl;
  return status;
}

ANLStatus ANLManager::Initialize()
{
  std::cout << '\n'
            << "        **************************************\n"
            << "        ****        Initialization        ****\n"
            << "        **************************************\n"
            << std::endl;

#if ANLNEXT_INITIALIZE_INTERRUPT
  struct sigaction sa;
  struct sigaction sa_org;
  std::memset(&sa, 0, sizeof(sa));
  std::memset(&sa_org, 0, sizeof(sa_org));
  sa.sa_handler = SIG_DFL;
  sa.sa_flags |= SA_RESTART;
  if ( sigaction(SIGINT, &sa, &sa_org) != 0 ) {
    std::cout << "sigaction(2) error!" << std::endl;
    return ANLStatus::critical_error_to_terminate;
  }
#endif

  show_analysis();
  print_parameters();
  reset_counters();
  requested_ = ANLRequest::none;

  ANLStatus status = routine_initialize();
  if (status != AS_OK) {
    goto final;
  }

  final:
    std::cout << std::endl;
#if ANLNEXT_INITIALIZE_INTERRUPT
  if ( sigaction(SIGINT, &sa_org, 0) != 0 ) {
    std::cout << "sigaction(2) error!" << std::endl;
    return ANLStatus::critical_error_to_terminate;
  }
#endif

  return status;
}

ANLStatus ANLManager::Analyze(long int num_events, bool enable_console)
{
  std::cout << '\n'
            << "        **************************************\n"
            << "        ****        Main Analysis         ****\n"
            << "        **************************************\n"
            << std::endl;

  num_events_ = num_events;
  requested_ = ANLRequest::none;

  std::cout << "Number of events: " << num_events << '\n'
            << std::endl;

#if ANLNEXT_ANALYZE_INTERRUPT
  struct sigaction sa;
  struct sigaction sa_org;
  std::memset(&sa, 0, sizeof(sa));
  std::memset(&sa_org, 0, sizeof(sa_org));
  sa.sa_handler = SIG_DFL;
  sa.sa_flags |= SA_RESTART;
  if ( sigaction(SIGINT, &sa, &sa_org) != 0 ) {
    std::cout << "sigaction(2) error!" << std::endl;
    return ANLStatus::critical_error_to_terminate;
  }
#endif

  ANLStatus status = AS_OK;

  status = routine_begin_run();
  if (status != AS_OK) {
    goto final;
  }

  if (enable_console) {
    std::cout << "\n"
              << "ANLManager: starting analysis loop (with user-console mode on).\n"
              << "----------------------------------------------------------------------------\n"
              << "  input '.q' => quit the analysis loop\n"
              << "  input '.i' => show the current event index\n"
              << "  input '.s' => show the status of event selections (of the master thread)\n"
              << "----------------------------------------------------------------------------\n"
              << std::endl;

    analysis_thread_finished_ = false;
    std::promise<ANLStatus> status_promise;
    std::future<ANLStatus> status_future = status_promise.get_future();
    std::thread analysis_thread(std::bind(&ANLManager::process_analysis_for_the_thread, this, std::placeholders::_1),
                                std::move(status_promise));
    std::thread interactive_thread(std::bind(&ANLManager::interactive_session, this));
    analysis_thread.join();
    analysis_thread_finished_ = true;
    interactive_thread.join();
    status = status_future.get();
  }
  else {
    std::cout << "\n"
              << "ANLManager: starting analysis loop.\n"
              << std::endl;
    status = process_analysis();
  }

  if (status != AS_OK) {
    goto final;
  }

  std::cout << "\n"
            << "ANLManager: analysis loop successfully done.\n"
            << std::endl;

  status = routine_end_run();
  if (status != AS_OK) {
    goto final;
  }

  reduce_modules();

  final:
    std::cout << std::endl;
  reduce_statistics();
  print_summary();
  evs_manager_->print_summary();
  requested_ = ANLRequest::none;

#if ANLNEXT_ANALYZE_INTERRUPT
  if ( sigaction(SIGINT, &sa_org, 0) != 0 ) {
    std::cout << "sigaction(2) error!" << std::endl;
    return ANLStatus::critical_error_to_terminate;
  }
#endif

  return status;
}

ANLStatus ANLManager::Finalize()
{
  std::cout << '\n'
            << "        **************************************\n"
            << "        ****         Finalization         ****\n"
            << "        **************************************\n"
            << std::endl;

#if ANLNEXT_FINALIZE_INTERRUPT
  struct sigaction sa;
  struct sigaction sa_org;
  std::memset(&sa, 0, sizeof(sa));
  std::memset(&sa_org, 0, sizeof(sa_org));
  sa.sa_handler = SIG_DFL;
  sa.sa_flags |= SA_RESTART;
  if ( sigaction(SIGINT, &sa, &sa_org) != 0 ) {
    std::cout << "sigaction(2) error!" << std::endl;
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
  if ( sigaction(SIGINT, &sa_org, 0) != 0 ) {
    std::cout << "sigaction(2) error!" << std::endl;
    return ANLStatus::critical_error_to_terminate;
  }
#endif

  return status;
}

BasicModule* ANLManager::access_to_module(int chain_ID,
                                          const std::string& module_ID)
{
  if (chain_ID == 0) {
    return module_access_->get_module_NC(module_ID);
  }

  BOOST_THROW_EXCEPTION( ANLException((boost::format("Chain does not exist ===> Chain ID: %d") % chain_ID).str()) );
  return nullptr;
}

int ANLManager::module_index(const std::string& module_ID, bool strict) const
{
  int index = -1;
  if (strict) {
    AMConstIter module_iter = std::find_if(std::begin(modules_), std::end(modules_),
                                           [&](const BasicModule* m){ return (m->module_id() == module_ID); });
    if (module_iter != modules_.cend()) {
      index = module_iter - modules_.cbegin();
    }
  }
  else {
    std::string lower1(module_ID);
    std::transform(lower1.begin(), lower1.end(), lower1.begin(),
                   [](unsigned char c){ return std::tolower(c);});
    AMConstIter module_iter = std::find_if(std::begin(modules_), std::end(modules_),
                                           [&](const BasicModule* m) {
                                             std::string lower2(m->module_id());
                                             std::transform(lower2.begin(), lower2.end(), lower2.begin(),
                                                            [](unsigned char c){ return std::tolower(c);});
                                             return (lower2 == lower1); });
    if (module_iter != modules_.cend()) {
      index = module_iter - modules_.cbegin();
    }
  }
  return index;
}

void ANLManager::show_analysis()
{
  std::cout << '\n'
            << "        **************************************\n"
            << "        ****        Analysis chain        ****\n"
            << "        **************************************\n"
            << std::endl;
  
  if (modules_.size() < 1) {
    std::cout << "No analysis chain is defined. " << std::endl;
  }
  else {
    std::cout
      << "   #" << "    "
      << "    Module ID                                " << "  "
      << " Version " << "  " << " ON/OFF \n"
      << "----------------------------------------------------------------------------"
      << std::endl;
    for (std::size_t i=0; i<modules_.size(); i++) {
      std::cout << std::right << std::setw(4) << i << "    ";
      std::string module_ID(modules_[i]->module_id());
      if (modules_[i]->module_id() != modules_[i]->module_name()) {
        module_ID += " (" + modules_[i]->module_name() + ")";
      }
      std::cout << std::left << std::setw(48) << module_ID;
      std::cout << "  "
                << std::left << std::setw(9) << modules_[i]->module_version()
                << "  "
                << std::left << std::setw(8)
                << (modules_[i]->is_on() ? "ON" : "OFF")
                << '\n';
    }
  }
  std::cout << std::right << std::setw(0) << std::endl;
}

void ANLManager::print_parameters()
{
  std::cout << '\n'
            << "        **************************************\n"
            << "        ****      Module parameters       ****\n"
            << "        **************************************\n"
            << std::endl;
  
  for (BasicModule* mod: modules_) {
    std::cout << "--- " << mod->module_id() << " ---"<< std::endl;
    mod->print_parameters();
    std::cout << std::endl;
  }
}

void ANLManager::reset_counters()
{
  counters_.resize(modules_.size());
  for (LoopCounter& c: counters_) {
    c.reset();
  }
}

ANLStatus ANLManager::process_analysis()
{
  ANLStatus status = AS_OK;

  const std::vector<BasicModule*>& modules = modules_;
  const long int period_disp = display_period();
  const long int num_events = number_of_loops();

  try {
    for (long int i_event=0; i_event!=num_events; i_event++) {
      if (period_disp != 0 && i_event%period_disp == 0) {
        print_event_index(i_event);
      }

      status = process_one_event(i_event, modules, counters_, *evs_manager_);

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
        i_event--;
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
    std::string module_ID = modules_[i]->module_name();
    if (modules_[i]->module_id() != modules_[i]->module_name()) {
      module_ID += "/" + modules_[i]->module_id();
    }
    module_ID += "  version  " + modules_[i]->module_version();
    std::cout << boost::format("    [%4d]  %-40s") % i % module_ID;
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
  std::cout << std::endl;
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
  while (true) {
#if ANLNEXT_USE_READLINE
    {
      fd_set readFDSet;
      FD_ZERO(&readFDSet);
      FD_SET(0, &readFDSet); // check STDIN (= 0)
      struct timeval timeout{1, 0}; // 1 s, 0 us
      const int retval = select(1, &readFDSet, nullptr, nullptr, &timeout);
      if (retval == -1) {
        std::cout << "Error by select() in ANLManager::interactive_sesson()" << std::endl;
        return;
      }
      if (retval == 0) {
        if (analysis_thread_finished_) {
          return;
        }
        continue;
      }
    }

    ReadLine reader;
    const int count = reader.read("ANL> ");
    if (count == -1) { return; }
    if (count == 0) { continue; }
    std::string line = reader.str();
#else /* ANLNEXT_USE_READLINE */
    std::string line;
    std::getline(std::cin, line);
    std::cout << "ANL>> " << line;
    if (analysisThreadFinished_) {
      return;
    }
#endif /* ANLNEXT_USE_READLINE */
    if (line == ".q") {
      std::lock_guard<std::mutex> lock(mutex_);
      std::cout << " ---> Quit\n" << std::endl;
      requested_ = ANLRequest::quit;
      return;
    }
    else if (line == ".i") {
      std::lock_guard<std::mutex> lock(mutex_);
      std::cout << " ---> Show event index\n" << std::endl;
      requested_ = ANLRequest::show_event_index;
    }
    else if (line == ".s") {
      std::lock_guard<std::mutex> lock(mutex_);
      std::cout << " ---> Show evs summary\n" << std::endl;
      requested_ = ANLRequest::show_evs_summary;
    }
    else {
      ;
    }
  }
}

ANLStatus process_one_event(long int i_event,
                            const std::vector<BasicModule*>& modules,
                            std::vector<LoopCounter>& counters,
                            EvsManager& evs_manager)
{
  evs_manager.reset_all_flags();
  ANLStatus status = AS_OK;

  for (BasicModule* mod: modules) {
    mod->set_loop_index(i_event);
  }

  const std::size_t NumberOfModules = modules.size();
  for (std::size_t i_module=0; i_module<NumberOfModules; i_module++) {
    BasicModule* mod = modules[i_module];

    if (mod->is_on()) {
      counters[i_module].count_up_by_entry();

      try {
        status = mod->mod_analyze();
      }
      catch (boost::exception& ex) {
        ex << ErrorInfoOnLoopIndex(i_event);
        ex << ErrorInfoOnMethod( mod->module_name() + "::mod_analyze" );
        ex << ErrorInfoOnModuleID( mod->module_id() );
        ex << ErrorInfoOnModuleName( mod->module_name() );
        ex << ErrorInfoOnChainID( mod->copy_id() );
        throw;
      }

      counters[i_module].count_up_by_result(status);
      status = eliminate_normal_error_status(status);

      if (status != AS_OK) {
        break;
      }
    }
  }

  count_evs(status, evs_manager);
  return status;
}

ANLStatus process_one_event(long int i_event,
                            const std::vector<BasicModule*>& modules,
                            std::vector<LoopCounter>& counters,
                            EvsManager& evs_manager,
                            std::vector<std::unique_ptr<OrderKeeper>>& order_keepers)
{
  evs_manager.reset_all_flags();
  ANLStatus status = AS_OK;

  for (BasicModule* mod: modules) {
    mod->set_loop_index(i_event);
  }

  const std::size_t NumberOfModules = modules.size();
  for (std::size_t i_module=0; i_module<NumberOfModules; i_module++) {
    BasicModule* mod = modules[i_module];
  
    const KeeperBlock<OrderKeeper, long int> block(order_keepers[i_module].get(), i_event);

    if (status == AS_OK && mod->is_on()) {
      counters[i_module].count_up_by_entry();

      try {
        status = mod->mod_analyze();
      }
      catch (ANLException& ex) {
        ex << ErrorInfoOnLoopIndex(i_event);
        ex << ErrorInfoOnMethod( mod->module_name() + "::mod_analyze" );
        ex << ErrorInfoOnModuleID( mod->module_id() );
        ex << ErrorInfoOnModuleName( mod->module_name() );
        ex << ErrorInfoOnChainID( mod->copy_id() );
        throw;
      }

      counters[i_module].count_up_by_result(status);
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
