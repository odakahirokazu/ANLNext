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

#if ANL_ANALYZE_INTERRUPT || ANL_INITIALIZE_INTERRUPT || ANL_EXIT_INTERRUPT
#include <signal.h>
#endif

#include <unistd.h>

#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "BasicModule.hh"
#include "EvsManager.hh"
#include "ModuleAccess.hh"
#include "ANLException.hh"
#include "ANLManager_impl.hh"

#if ANL_USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif /* ANL_USE_READLINE */

namespace anl
{

ANLManager::ANLManager()
  : evsManager_(new EvsManager),
    displayFrequency_(-1), interrupted_(false)
{
  evsManager_->initialize();
}

ANLManager::~ANLManager() = default;

void ANLManager::SetModules(std::vector<BasicModule*> modules) throw(ANLException)
{
  modules_ = modules;
  
  for (AMIter mod = modules_.begin(); mod != modules_.end(); ++mod) {
    (*mod)->set_evs_manager(evsManager_.get());
  }

  for (AMIter mod = modules_.begin(); mod != modules_.end(); ++mod) {
    ModuleAccess access;
    
    for (AMIter r = modules_.begin(); r != modules_.end(); ++r) {
      std::string name;
      name = (*r)->module_id();
      if ( (*mod)->accessible(name) ) {
        access.registerModule(name, *r);
      }

      std::vector<std::string> alias = (*r)->get_alias();
      for (std::size_t i=0; i<alias.size(); i++) {
        name = alias[i];
        if ( (*mod)->accessible(name) ) {
          access.registerModule(name, *r);
        }
      }
    }

    (*mod)->set_anl_access(access);
  }
  
  counters_.resize(modules_.size());
  reset_counter();
}

ANLStatus ANLManager::Startup() throw(ANLException)
{
  std::cout << '\n'
            << "      ***********************************\n"
            << "      ****          ANL Next         ****\n"
            << "      ***********************************\n"
            << std::endl;

  return routine_startup();
}

ANLStatus ANLManager::Initialize() throw(ANLException)
{
#if ANL_INITIALIZE_INTERRUPT
  struct sigaction sa;
  struct sigaction sa_org;
  std::memset(&sa, 0, sizeof(sa));
  std::memset(&sa_org, 0, sizeof(sa_org));
  sa.sa_handler = SIG_DFL;
  sa.sa_flags |= SA_RESTART;
  if( sigaction(SIGINT, &sa, &sa_org) != 0 ) {
    std::cout << "sigaction(2) error!" << std::endl;
    return AS_QUIT_ERR;
  }
#endif

  show_analysis();
  print_parameters();
  reset_counter();

  ANLStatus status = AS_OK;
  status = routine_init();
  if (status != AS_OK) goto final;

  status = routine_his();
  if (status != AS_OK) goto final;
  
  std::cout << "ANLManager: initialization done." << std::endl;
  
 final:
  std::cout << std::endl;
#if ANL_INITIALIZE_INTERRUPT
  if( sigaction(SIGINT, &sa_org, 0) != 0 ) {
    std::cout << "sigaction(2) error!" << std::endl;
    return AS_QUIT_ERR;
  }
#endif
  
  return status;
}

ANLStatus ANLManager::Analyze(long int num_events, bool thread_mode) throw(ANLException)
{
#if ANL_ANALYZE_INTERRUPT
  struct sigaction sa;
  struct sigaction sa_org;
  std::memset(&sa, 0, sizeof(sa));
  std::memset(&sa_org, 0, sizeof(sa_org));
  sa.sa_handler = SIG_DFL;
  sa.sa_flags |= SA_RESTART;
  if( sigaction(SIGINT, &sa, &sa_org) != 0 ) {
    std::cout << "sigaction(2) error!" << std::endl;
    return AS_QUIT_ERR;
  }
#endif

  ANLStatus status = AS_OK;
  std::cout << "ANLManager: start bgnrun routine." << std::endl;
  status = routine_bgnrun();
  if (status != AS_OK) {
    goto final;
  }

  if (thread_mode) {
    std::cout << "ANLManager: start analysis (with thread mode on)." << std::endl;
    std::cout << "You can quit the analysis routine by input 'q'." << std::endl;
    
    boost::thread analysisThread(std::bind(&ANLManager::__void_process_analysis, this, num_events, &status));
    boost::thread interactiveThread(std::bind(&ANLManager::interactive_session, this));
    analysisThread.join();
    boost::chrono::seconds wait(1);
    interactiveThread.try_join_for(wait);
    
#if ANL_USE_READLINE
    rl_initialize();
    rl_deprep_terminal();
#endif
  }
  else {
    std::cout << "ANLManager: start analysis." << std::endl;
    status = process_analysis(num_events);
  }

  if (status != AS_OK) {
    print_summary();
    goto final;
  }
  
  std::cout << "ANLManager: start endrun routine." << std::endl;
  status = routine_endrun();
  if (status != AS_OK) {
    print_summary();
    goto final;
  }

 final:
  std::cout << std::endl;
  print_summary();

#if ANL_ANALYZE_INTERRUPT
  if( sigaction(SIGINT, &sa_org, 0) != 0 ) {
    std::cout << "sigaction(2) error!" << std::endl;
    return AS_QUIT_ERR;
  }
#endif
  
  return status;
}

ANLStatus ANLManager::Exit() throw(ANLException)
{
#if ANL_EXIT_INTERRUPT
  struct sigaction sa;
  struct sigaction sa_org;
  std::memset(&sa, 0, sizeof(sa));
  std::memset(&sa_org, 0, sizeof(sa_org));
  sa.sa_handler = SIG_DFL;
  sa.sa_flags |= SA_RESTART;
  if( sigaction(SIGINT, &sa, &sa_org) != 0 ) {
    std::cout << "sigaction(2) error!" << std::endl;
    return AS_QUIT_ERR;
  }
#endif

  ANLStatus status = AS_OK;
  status = routine_exit();
  if (status != AS_OK) {
    return status;
  }

  std::cout << std::endl;
  evsManager_->printSummary();

  std::cout << "\nANLManager: exiting..." << std::endl;

#if ANL_EXIT_INTERRUPT
  if( sigaction(SIGINT, &sa_org, 0) != 0 ) {
    std::cout << "sigaction(2) error!" << std::endl;
    return AS_QUIT_ERR;
  }
#endif
  
  return status;
}

ANLStatus ANLManager::Prepare() throw(ANLException)
{
  return routine_prepare();
}

int ANLManager::getModuleNumber(const std::string& id, bool strict)
{
  int modNumber = -1;
  if (strict) {
    for (std::size_t i=0; i<modules_.size(); ++i) {
      if (modules_[i]->module_id()==id) {
        if (modNumber==-1) {
          modNumber = i+1;
        }
        else {
          return -2;
        }
      }
    }
  }
  else {
    for (std::size_t i=0; i<modules_.size(); ++i) {
      std::string name2(id);
      std::transform(name2.begin(), name2.end(), name2.begin(), ANLToLower());
      std::string moduleNameLower(modules_[i]->module_id());
      std::transform(moduleNameLower.begin(), moduleNameLower.end(),
                     moduleNameLower.begin(), ANLToLower());
      if (moduleNameLower.find(name2)==0) {
        if (modNumber==-1) {
          modNumber = i+1;
        }
        else {
          return -2;
        }
      }
    }
  }
  return modNumber;
}

void ANLManager::show_analysis()
{
  std::cout << '\n'
            << "      ***********************************\n"
            << "      ****      Analysis chain      *****\n"
            << "      ***********************************\n"
            << std::endl;
  
  if (modules_.size() < 1) {
    std::cout << "No analysis chain is defined. " << std::endl;
  }
  else {
    std::cout
      << " #  " << "    "
      << "              Module ID                      " << "  " 
      << " Version " << "  " << " ON/OFF \n"
      << "----------------------------------------------------------------------------"
      << std::endl;
    for (std::size_t i = 0; i < modules_.size(); i++) {
      std::cout << std::right << std::setw(4) << i+1 << "    ";
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
            << "      ***********************************\n"
            << "      ****     Module parameters     ****\n"
            << "      ***********************************\n"
            << std::endl;
  
  AMIter const mod_end = modules_.end();
  for (AMIter mod = modules_.begin(); mod != mod_end; ++mod) {
    std::cout << "--- " << (*mod)->module_id() << " ---"<< std::endl;
    (*mod)->print_parameters();
    std::cout << std::endl;
  }
}

void ANLManager::reset_counter()
{
  for (std::size_t i=0; i<counters_.size(); i++) {
    counters_[i].entry = 0;
    counters_[i].ok = 0;
    counters_[i].err = 0;
    counters_[i].skip = 0;
    counters_[i].quit = 0;
  }
}

ANLStatus ANLManager::process_analysis(long int num_events)
{
  ANLStatus status = AS_OK;
  long int iEvent = 0;
  AMIter mod = modules_.begin();
  AMIter const mod_end = modules_.end();
  int iModule = 0;

  long int display_freq = DisplayFrequency();
  if (display_freq < 0) {
    if (num_events > 0) { display_freq = num_events/100; }
    else { display_freq = 10000; }
  }

  while (iEvent != num_events) {
    if (display_freq != 0 && iEvent%display_freq == 0) {
      std::cout << "Event : " << std::dec << std::setw(10) << iEvent << std::endl;
      std::cout.width(0);
    }

    evsManager_->resetAllFlags();
    mod = modules_.begin(); iModule = 0;
    while (mod != mod_end) {
      if ((*mod)->is_on()) {
        ++counters_[iModule].entry;
        (*mod)->set_event_loop_index(iEvent);

        try {
          status = (*mod)->mod_ana();
        }
        catch (const ANLException& ex) {
          ex << ANLErrModFnInfo( (*mod)->module_id() + "::mod_ana" );
          ex << ANLErrEventIDInfo(iEvent);
          throw;
        }

        if (status == AS_OK) {
          ++counters_[iModule].ok;
        }
        else {
          break;
        }
      }
      ++mod; ++iModule;
    }

    if (status == AS_OK) {
      evsManager_->count();
    }
    else if(status == AS_SKIP) {
      ++counters_[iModule].skip;
      evsManager_->count();
    }
    else if (status == AS_SKIP_ERR) {
      ++counters_[iModule].skip;
      ++counters_[iModule].err;
    }
    else if (status == AS_QUIT) {
      ++counters_[iModule].quit;
      evsManager_->count();
      break;
    }
    else if (status == AS_QUIT_ERR) {
      ++counters_[iModule].quit;
      ++counters_[iModule].err;
      break;
    }
    else {
      break;
    }

    if (interrupted_) break;
       
    ++iEvent;
  }

  if (status==AS_SKIP_ERR || status==AS_QUIT_ERR) return status;
  
  return AS_OK;
}

void ANLManager::print_summary()
{
  const std::size_t n = modules_.size();
  std::cout << "      ***********************************\n"
            << "      ****      Analysis chain      *****\n"
            << "      ***********************************\n"
            << "               PUT: " << counters_[0].entry << '\n'
            << "                |\n";

  for (std::size_t i=0; i<n; i++) {
    std::string moduleID = modules_[i]->module_name();
    if (modules_[i]->module_id() != modules_[i]->module_name()) {
      moduleID += "/" + modules_[i]->module_id();
    }
    moduleID += "  version  " + modules_[i]->module_version();
    std::cout << boost::format("     [%3d]  %-40s") % (i+1) % moduleID;
    if (counters_[i].quit>1) { std::cout << "  ---> Quit"; }
    std::cout <<  '\n';
    std::cout << "    " << std::setw(10) << counters_[i].entry << "  |"
              << "  OK: " << counters_[i].ok
              << "  SKIP: " << counters_[i].skip
              << "  ERR: " << counters_[i].err << '\n';
    std::cout.width(0);
  }
  std::cout << "               GET: " << counters_[n-1].ok << '\n';
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
  return std::move(pt);
}

void ANLManager::parameters_to_json(const std::string& filename) const
{
  boost::property_tree::ptree pt = parameters_to_property_tree();
  write_json(filename.c_str(), pt);
}

ANLStatus ANLManager::routine_startup()
{
  return routine_modfn(&BasicModule::mod_startup, "startup");
}

ANLStatus ANLManager::routine_prepare()
{
  return routine_modfn(&BasicModule::mod_prepare, "prepare");
}

ANLStatus ANLManager::routine_init()
{
  return routine_modfn(&BasicModule::mod_init, "init");
}

ANLStatus ANLManager::routine_his()
{
  return routine_modfn(&BasicModule::mod_his, "his");
}

ANLStatus ANLManager::routine_bgnrun()
{
  return routine_modfn(&BasicModule::mod_bgnrun, "bgnrun");
}

ANLStatus ANLManager::routine_endrun()
{
  return routine_modfn(&BasicModule::mod_endrun, "endrun");
}

ANLStatus ANLManager::routine_exit()
{
  return routine_modfn(&BasicModule::mod_exit, "exit");
}

void ANLManager::__void_process_analysis(long int num_events, ANLStatus* status)
{
  ANLStatus s = process_analysis(num_events);
  *status = s;
}

void ANLManager::interactive_session()
{
#if ANL_USE_READLINE
  std::shared_ptr<char> line;
  while (1) {
    line = std::shared_ptr<char>(readline(""));
    if (line.get()) {
      if (std::strcmp(line.get(), "q") == 0) {
        std::cout << "---> QUIT" << std::endl;
        interrupted_ = true;
        return;
      }
    }
    else {
      return;
    }
  }
#else
  std::string buf;
  while (1) {
    std::cin >> buf;
    if (buf=="q") {
      std::cout << "---> QUIT by user interruption" << std::endl; 
      interrupted_ = true;
      return;
    }
  }
#endif
}

} /* namespace anl */
