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

#if ANL_ANALYZE_INTERRUPT || ANL_INITIALIZE_INTERRUPT || ANL_EXIT_INTERRUPT
#include <signal.h>
#endif

#include <unistd.h>

#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include "BasicModule.hh"
#include "EvsManager.hh"
#include "ANLAccess.hh"
#include "ANLException.hh"
#include "ANLManager_impl.hh"

#define ANLNEXT_USE_READLINE 1
#if ANLNEXT_USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif


using namespace anl;


ANLManager::ANLManager()
  : m_DisplayFrequency(-1), m_Interrupt(false)
{
  m_Evs = new EvsManager;
  m_Evs->Initialize();
}


ANLManager::~ANLManager()
{
  delete m_Evs;
}


void ANLManager::SetModules(std::vector<BasicModule*> modules) throw(ANLException)
{
  m_Modules = modules;
  
  for (AMIter mod = m_Modules.begin(); mod != m_Modules.end(); ++mod) {
    (*mod)->set_evs_manager(m_Evs);
  }

  for (AMIter mod = m_Modules.begin(); mod != m_Modules.end(); ++mod) {
    ANLAccess anlAccess;
    
    for (AMIter r = m_Modules.begin(); r != m_Modules.end(); ++r) {
      std::string name;
      name = (*r)->module_id();
      if ( (*mod)->accessible(name) ) {
        anlAccess.RegisterModule(name, *r);
      }

      std::vector<std::string> alias = (*r)->get_alias();
      for (size_t i=0; i<alias.size(); i++) {
        name = alias[i];
        if ( (*mod)->accessible(name) ) {
          anlAccess.RegisterModule(name, *r);
        }
      }
    }

    (*mod)->set_anl_access(anlAccess);
  }
  
  m_Counter.resize(m_Modules.size());
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


ANLStatus ANLManager::Analyze(int num_event, bool thread_mode) throw(ANLException)
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
    
    boost::thread analysisThread(boost::bind(&ANLManager::__void_process_analysis, this, num_event, &status));
    boost::thread interactiveThread(boost::bind(&ANLManager::interactive_session, this));
    analysisThread.join();
    boost::chrono::seconds wait(1);
    interactiveThread.try_join_for(wait);
    
#if ANLNEXT_USE_READLINE
    rl_initialize();
#endif
  }
  else {
    std::cout << "ANLManager: start analysis." << std::endl;
    status = process_analysis(num_event);
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
  m_Evs->PrintSummary();

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
    for (size_t i=0; i<m_Modules.size(); ++i) {
      if (m_Modules[i]->module_id()==id) {
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
    for (size_t i=0; i<m_Modules.size(); ++i) {
      std::string name2(id);
      std::transform(name2.begin(), name2.end(), name2.begin(), ANLToLower());
      std::string moduleNameLower(m_Modules[i]->module_id());
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
  
  if (m_Modules.size() < 1) {
    std::cout << "No analysis chain is defined. " << std::endl;
  }
  else {
    std::cout
      << " #  " << "    "
      << "              Module ID                      " << "  " 
      << " Version " << "  " << " ON/OFF \n"
      << "----------------------------------------------------------------------------"
      << std::endl;
    for(size_t i = 0; i < m_Modules.size(); i++) {
      std::cout << std::right << std::setw(4) << i+1 << "    ";
      std::string moduleID(m_Modules[i]->module_id());
      if (m_Modules[i]->module_id() != m_Modules[i]->module_name()) {
        moduleID += " (" + m_Modules[i]->module_name() + ")";
      }
      std::cout << std::left << std::setw(48) << moduleID;
      std::cout << "  "
                << std::left << std::setw(9) << m_Modules[i]->module_version()
                << "  "
                << std::left << std::setw(8)
                << (m_Modules[i]->is_on() ? "ON" : "OFF")
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
  
  AMIter const mod_end = m_Modules.end();
  for (AMIter mod = m_Modules.begin(); mod != mod_end; ++mod) {
    std::cout << "--- " << (*mod)->module_id() << " ---"<< std::endl;
    (*mod)->print_parameters();
    std::cout << std::endl;
  }
}


void ANLManager::reset_counter()
{
  for (size_t i=0; i<m_Counter.size(); i++) {
    m_Counter[i].entry = 0;
    m_Counter[i].ok = 0;
    m_Counter[i].err = 0;
    m_Counter[i].skip = 0;
    m_Counter[i].quit = 0;
  }
}


ANLStatus ANLManager::process_analysis(int num_event)
{
  ANLStatus status = AS_OK;
  int iEvent = 0;
  AMIter mod = m_Modules.begin();
  AMIter const mod_end = m_Modules.end();
  int iModule = 0;

  int display_freq = DisplayFrequency();
  if (display_freq < 0) {
    if (num_event > 0) { display_freq = num_event/100; }
    else { display_freq = 10000; }
  }

  while (iEvent != num_event) {
    if (display_freq != 0 && iEvent%display_freq == 0) {
      std::cout << "Event : " << std::dec << std::setw(10) << iEvent << std::endl;
      std::cout.width(0);
    }

    m_Evs->ResetAll();
    mod = m_Modules.begin(); iModule = 0;
    while (mod != mod_end) {
      if ((*mod)->is_on()) {
        ++m_Counter[iModule].entry;
        
        try {
          status = (*mod)->mod_ana();
        }
        catch (const ANLException& ex) {
          ex << ANLErrModFnInfo( (*mod)->module_id() + "::mod_ana" );
          ex << ANLErrEventIDInfo(iEvent);
          throw;
        }

        if (status == AS_OK ) {
          ++m_Counter[iModule].ok;
        }
        else {
          break;
        }
      }
      ++mod; ++iModule;
    }

    if (status == AS_OK) {
      m_Evs->Count();
    }
    else if(status == AS_SKIP) {
      ++m_Counter[iModule].skip;
      m_Evs->Count();
    }
    else if (status == AS_SKIP_ERR) {
      ++m_Counter[iModule].skip;
      ++m_Counter[iModule].err;
    }
    else if (status == AS_QUIT) {
      ++m_Counter[iModule].quit;
      m_Evs->Count();
      break;
    }
    else if (status == AS_QUIT_ERR) {
      ++m_Counter[iModule].quit;
      ++m_Counter[iModule].err;
      break;
    }
    else {
      break;
    }

    if (m_Interrupt) break;
       
    ++iEvent;
  }

  if (status==AS_SKIP_ERR || status==AS_QUIT_ERR) return status;
  
  return AS_OK;
}


void ANLManager::print_summary()
{
  const size_t n = m_Modules.size();
  std::cout << "      ***********************************\n"
            << "      ****      Analysis chain      *****\n"
            << "      ***********************************\n"
            << "               PUT: " << m_Counter[0].entry << '\n'
            << "                |\n";

  for (size_t i=0; i<n; i++) {
    std::string moduleID = m_Modules[i]->module_name();
    if (m_Modules[i]->module_id() != m_Modules[i]->module_name()) {
      moduleID += "/" + m_Modules[i]->module_id();
    }
    moduleID += "  version  " + m_Modules[i]->module_version();
    std::cout << boost::format("     [%3d]  %-40s") % (i+1) % moduleID;
    if (m_Counter[i].quit>1) { std::cout << "  ---> Quit"; }
    std::cout <<  '\n';
    std::cout << "    " << std::setw(10) << m_Counter[i].entry << "  |"
              << "  OK: " << m_Counter[i].ok
              << "  SKIP: " << m_Counter[i].skip
              << "  ERR: " << m_Counter[i].err << '\n';
    std::cout.width(0);
  }
  std::cout << "               GET: " << m_Counter[n-1].ok << '\n';
  std::cout << std::endl;
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


void ANLManager::__void_process_analysis(int num_event, ANLStatus* status)
{
  ANLStatus s = process_analysis(num_event);
  *status = s;
}


void ANLManager::interactive_session()
{
#if ANLNEXT_USE_READLINE
  boost::shared_ptr<char> line;
  while (1) {
    line = boost::shared_ptr<char>(readline(""));
    if (line.get()) { 
      if (std::strcmp(line.get(), "q") == 0) {
        std::cout << "---> QUIT" << std::endl;
        m_Interrupt = true;
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
      m_Interrupt = true;
      return;
    }
  }
#endif
}
