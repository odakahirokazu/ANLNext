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

#if ANL_ANALYZE_INTERRUPT || ANL_INITIALIZE_INTERRUPT || ANL_FINALIZE_INTERRUPT
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

#if ANL_USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>

#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#endif /* ANL_USE_READLINE */


namespace anl
{

/* version definition */
const int ANLManager::__version1__ = 2;
const int ANLManager::__version2__ = 0;
const int ANLManager::__version3__ = 0;


ANLManager::ANLManager()
  : printCloneParameters_(false),
    numEvents_(0),
    evsManager_(new EvsManager),
    requested_(ANLRequest::none),
    displayFrequency_(-1),
    moduleAccess_(new ModuleAccess),
    analysisThreadFinished_(false)
{
  evsManager_->initialize();
}

ANLManager::~ANLManager() = default;

void ANLManager::set_modules(std::vector<BasicModule*> modules)
{
  modules_ = modules;

  for (BasicModule* mod: modules_) {
    mod->set_evs_manager(evsManager_.get());
    mod->set_module_access(moduleAccess_.get());
  }

  reset_counters();
}

long int ANLManager::display_frequency() const
{
  if (displayFrequency_ < 0) {
    return (numEvents_ > 0) ? (numEvents_/100) : 10000;
  }
  return displayFrequency_;
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
    const std::string moduleID = mod->module_id();
    moduleAccess_->register_module(moduleID,
                                   mod,
                                   ModuleAccess::ConflictOption::error);

    for (const std::pair<std::string, ModuleAccess::ConflictOption>& alias: mod->get_aliases()) {
      if (alias.first != moduleID) {
        moduleAccess_->register_module(alias.first,
                                       mod,
                                       alias.second);
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

#if ANL_INITIALIZE_INTERRUPT
  struct sigaction sa;
  struct sigaction sa_org;
  std::memset(&sa, 0, sizeof(sa));
  std::memset(&sa_org, 0, sizeof(sa_org));
  sa.sa_handler = SIG_DFL;
  sa.sa_flags |= SA_RESTART;
  if ( sigaction(SIGINT, &sa, &sa_org) != 0 ) {
    std::cout << "sigaction(2) error!" << std::endl;
    return AS_QUIT_ERROR;
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
#if ANL_INITIALIZE_INTERRUPT
  if ( sigaction(SIGINT, &sa_org, 0) != 0 ) {
    std::cout << "sigaction(2) error!" << std::endl;
    return AS_QUIT_ERROR;
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

  numEvents_ = num_events;
  requested_ = ANLRequest::none;

  std::cout << "Number of events: " << num_events << '\n'
            << std::endl;

#if ANL_ANALYZE_INTERRUPT
  struct sigaction sa;
  struct sigaction sa_org;
  std::memset(&sa, 0, sizeof(sa));
  std::memset(&sa_org, 0, sizeof(sa_org));
  sa.sa_handler = SIG_DFL;
  sa.sa_flags |= SA_RESTART;
  if ( sigaction(SIGINT, &sa, &sa_org) != 0 ) {
    std::cout << "sigaction(2) error!" << std::endl;
    return AS_QUIT_ERROR;
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

    analysisThreadFinished_ = false;
    std::thread analysisThread(std::bind(&ANLManager::__void_process_analysis, this, &status));
    std::thread interactiveThread(std::bind(&ANLManager::interactive_session, this));
    analysisThread.join();
    analysisThreadFinished_ = true;
    interactiveThread.join();
    
#if ANL_USE_READLINE
    rl_reset_terminal(NULL);
    rl_initialize();
    rl_deprep_terminal();
#endif
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
  evsManager_->print_summary();
  requested_ = ANLRequest::none;

#if ANL_ANALYZE_INTERRUPT
  if ( sigaction(SIGINT, &sa_org, 0) != 0 ) {
    std::cout << "sigaction(2) error!" << std::endl;
    return AS_QUIT_ERROR;
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

#if ANL_FINALIZE_INTERRUPT
  struct sigaction sa;
  struct sigaction sa_org;
  std::memset(&sa, 0, sizeof(sa));
  std::memset(&sa_org, 0, sizeof(sa_org));
  sa.sa_handler = SIG_DFL;
  sa.sa_flags |= SA_RESTART;
  if ( sigaction(SIGINT, &sa, &sa_org) != 0 ) {
    std::cout << "sigaction(2) error!" << std::endl;
    return AS_QUIT_ERROR;
  }
#endif

  ANLStatus status = routine_finalize();
  if (status != AS_OK) {
    goto final;
  }

final:
  std::cout << std::endl;

#if ANL_FINALIZE_INTERRUPT
  if ( sigaction(SIGINT, &sa_org, 0) != 0 ) {
    std::cout << "sigaction(2) error!" << std::endl;
    return AS_QUIT_ERROR;
  }
#endif

  return status;
}

BasicModule* ANLManager::access_to_module(int chainID,
                                          const std::string& moduleID)
{
  if (chainID == 0) {
    return moduleAccess_->get_module_NC(moduleID);
  }

  const std::string message
    = (boost::format("Chain ID is not found: %d") % chainID).str();
  BOOST_THROW_EXCEPTION( ANLException(message) );
  return nullptr;
}

int ANLManager::module_index(const std::string& module_id, bool strict) const
{
  int index = -1;
  if (strict) {
    AMConstIter moduleIter = std::find_if(std::begin(modules_), std::end(modules_),
                                          [&](const BasicModule* m){ return (m->module_id() == module_id); });
    if (moduleIter != modules_.cend()) {
      index = moduleIter - modules_.cbegin();
    }
  }
  else {
    std::string lower1(module_id);
    std::transform(lower1.begin(), lower1.end(), lower1.begin(),
                   [](unsigned char c){ return std::tolower(c);});
    AMConstIter moduleIter = std::find_if(std::begin(modules_), std::end(modules_),
                                          [&](const BasicModule* m) {
                                            std::string lower2(m->module_id());
                                            std::transform(lower2.begin(), lower2.end(), lower2.begin(),
                                                           [](unsigned char c){ return std::tolower(c);});
                                            return (lower2 == lower1); });
    if (moduleIter != modules_.cend()) {
      index = moduleIter - modules_.cbegin();
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
      std::string moduleID(modules_[i]->module_id());
      if (modules_[i]->module_id() != modules_[i]->module_name()) {
        moduleID += " (" + modules_[i]->module_name() + ")";
      }
      std::cout << std::left << std::setw(48) << moduleID;
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
  const long int displayFrequency = display_frequency();
  const long int numEvents = number_of_loops();

  for (long int iEvent=0; iEvent!=numEvents; iEvent++) {
    if (displayFrequency != 0 && iEvent%displayFrequency == 0) {
      print_event_index(iEvent);
    }

    status = process_one_event(iEvent, modules, counters_, *evsManager_);

    if (status==AS_QUIT_ALL) { status = AS_QUIT; }
    if (status==AS_QUIT_ALL_ERROR) { status = AS_QUIT_ERROR; }

    if (status==AS_QUIT || status==AS_QUIT_ERROR) {
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
  }

  if (status==AS_SKIP_ERROR || status==AS_QUIT_ERROR) {
    return status;
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
    std::string moduleID = modules_[i]->module_name();
    if (modules_[i]->module_id() != modules_[i]->module_name()) {
      moduleID += "/" + modules_[i]->module_id();
    }
    moduleID += "  version  " + modules_[i]->module_version();
    std::cout << boost::format("    [%4d]  %-40s") % i % moduleID;
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

void ANLManager::__void_process_analysis(ANLStatus* status)
{
  ANLStatus s = process_analysis();
  *status = s;
}

void ANLManager::interactive_session()
{
#if ANL_USE_READLINE
  while (1) {
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
      if (analysisThreadFinished_) {
        return;
      }
      continue;
    }

    std::unique_ptr<char> line(readline(""));
    if (line.get()) {
      if (std::strcmp(line.get(), ".q") == 0) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << "ANL> " << line.get() << " ---> Quit\n" << std::endl;
        requested_ = ANLRequest::quit;
        return;
      }
      else if (std::strcmp(line.get(), ".i") == 0) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << "ANL> " << line.get() << " ---> Show event index\n" << std::endl;
        requested_ = ANLRequest::show_event_index;
      }
      else if (std::strcmp(line.get(), ".s") == 0) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << "ANL> " << line.get() << " ---> Show evs summary\n" << std::endl;
        requested_ = ANLRequest::show_evs_summary;
      }
      else {
        std::cout << "ANL> " << line.get() << "\n" << std::endl;
      }
    }
    else {
      return;
    }
  }
#else /* ANL_USE_READLINE */
  std::string buf;
  while (1) {
    std::cin >> buf;
    if (buf==".q") {
      std::lock_guard<std::mutex> lock(mutex_);
      std::cout << "ANL>> " << line.get() << " ---> Quit\n" << std::endl;
      requested_ = ANLRequest::quit;
      return;
    }
    else if (buf==".i") {
      std::lock_guard<std::mutex> lock(mutex_);
      std::cout << "ANL>> " << line.get() << " ---> Show event index\n" << std::endl;
      requested_ = ANLRequest::show_event_index;
    }
    else if (buf==".s") {
      std::lock_guard<std::mutex> lock(mutex_);
      std::cout << "ANL>> " << line.get() << " ---> Show evs summary\n" << std::endl;
      requested_ = ANLRequest::show_evs_summary;
    }
    else {
      std::cout << "ANL>> " << line.get() << "\n" << std::endl;
    }
  }
#endif /* ANL_USE_READLINE */
}

ANLStatus process_one_event(long int iEvent,
                            const std::vector<BasicModule*>& modules,
                            std::vector<LoopCounter>& counters,
                            EvsManager& evsManager)
{
  evsManager.reset_all_flags();

  ANLStatus status = AS_OK;

  const std::size_t NumberOfModules = modules.size();
  for (std::size_t iModule=0; iModule<NumberOfModules; iModule++) {
    BasicModule* mod = modules[iModule];
    mod->set_loop_index(iEvent);

    if (mod->is_on()) {
      counters[iModule].count_up_by_entry();

      try {
        status = mod->mod_analyze();
      }
      catch (ANLException& ex) {
        ex << ANLErrorInfoOnLoopIndex(iEvent);
        ex << ANLErrorInfoOnMethod( mod->module_name() + "::mod_analyze" );
        ex << ANLErrorInfoOnModule( mod->module_id() );
        throw;
      }
        
      counters[iModule].count_up_by_result(status);
        
      if (status != AS_OK) {
        break;
      }
    }
  }

  count_evs(status, evsManager);

  return status;
}

ANLStatus process_one_event(long int iEvent,
                            const std::vector<BasicModule*>& modules,
                            std::vector<LoopCounter>& counters,
                            EvsManager& evsManager,
                            std::vector<std::unique_ptr<OrderKeeper>>& order_keepers)
{
  evsManager.reset_all_flags();

  ANLStatus status = AS_OK;

  const std::size_t NumberOfModules = modules.size();
  for (std::size_t iModule=0; iModule<NumberOfModules; iModule++) {
    BasicModule* mod = modules[iModule];
    mod->set_loop_index(iEvent);

    const KeeperBlock<OrderKeeper, long int> block(order_keepers[iModule].get(), iEvent);

    if (status == AS_OK && mod->is_on()) {
      counters[iModule].count_up_by_entry();

      try {
        status = mod->mod_analyze();
      }
      catch (ANLException& ex) {
        ex << ANLErrorInfoOnLoopIndex(iEvent);
        ex << ANLErrorInfoOnMethod( mod->module_name() + "::mod_analyze" );
        ex << ANLErrorInfoOnModule( mod->module_id() );
        throw;
      }

      counters[iModule].count_up_by_result(status);
    }
  }

  count_evs(status, evsManager);

  return status;
}

void count_evs(ANLStatus status, EvsManager& evsManager)
{
  if (status == AS_OK) {
    evsManager.count();
    evsManager.count_completed();
  }
  else if(status == AS_SKIP) {
    evsManager.count();
  }
  else if (status == AS_QUIT) {
    evsManager.count();
  }
  else if (status == AS_QUIT_ALL) {
    evsManager.count();
  }
}

} /* namespace anl */
