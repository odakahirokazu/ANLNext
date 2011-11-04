#include "ANLNext.hh"
#include "ANLVModule.hh"

#include <sstream>
#include <iomanip>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#define ANLNEXT_USE_READLINE 1
#if ANLNEXT_USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#else
#include <histedit.h>
#endif


namespace
{
#if !ANLNEXT_USE_READLINE
const char* prompt(EditLine *e)
{
  return "ANL> ";
}
#endif
}

using namespace anl;

ANLStatus ANLNext::InteractiveCom() throw(ANLException)
{
  ANLStatus status = AS_OK;
  
#if ANLNEXT_USE_READLINE
  char *line;
  int count = 0;
#else
  const char *line;
  int count = 0;
  EditLine *el;
  History *anlHistory;
  HistEvent event;
  el = el_init("ANLNext_Com", stdin, stdout, stderr);
  el_set(el, EL_PROMPT, &prompt);
  el_set(el, EL_EDITOR, "emacs");

  anlHistory = history_init();
  if (anlHistory == 0) {
    std::cerr << "history initialization falied" << std::endl;
    return AS_QUIT;
  }
  
  history(anlHistory, &event, H_SETSIZE, 1000);
  el_set(el, EL_HIST, history, anlHistory);
#endif
  
  std::string cmd;
  std::istringstream iss;

  std::cout << "\n ** type \"help\" if you need. ** \n" << std::endl;

  while (iss.str()=="") {
#if ANLNEXT_USE_READLINE
    line = readline("ANL> ");
    if (line) count = strlen(line)+1;
    else {
      exit(1);
    }
#else
    line = el_gets(el, &count);
#endif
    
    if (count > 1) {
#if ANLNEXT_USE_READLINE
      add_history(line);
#else
      history(anlHistory, &event, H_ENTER, line);
#endif

      iss.str(line);
      
#if ANLNEXT_USE_READLINE
      free(line);
      line = 0;
#endif

      iss >> cmd;
 
      if (cmd == "quit") {
        std::cout << "ANL quitting..." << std::endl;
        status = AS_QUIT;
        break;
      }
      else if (cmd == "help") {
        InteractiveComHelp();
      }
      else if (cmd == "mod") {
        std::string moduleName;
        int n;
        iss >> moduleName;
        if (iss) {
          try {
            if (std::isdigit(moduleName[0])) {
              n = boost::lexical_cast<int>(moduleName);
            }
            else {
              n = getModuleNumber(moduleName, false);
            }
            status = InteractiveModifyParam(n);
          }
          catch (const ANLException& ex) {
            std::cout << "Exception!\n"
                      << *boost::get_error_info<ANLErrInfo>(ex) << std::endl;
          }
        }
        else {
          std::cout << "usage: mod MODULE_ID" << std::endl;
        }
      }
      else if (cmd == "show") {
        show_analysis();
      }
      else if (cmd == "print") {
        std::string moduleName;
        int n;
        iss >> moduleName;
        if (iss) {
          if (std::isdigit(moduleName[0])) {
            n = boost::lexical_cast<int>(moduleName);
          }
          else {
            n = getModuleNumber(moduleName, false);
          }
          InteractivePrintParam(n);
        }
        else {
          std::cout << "usage: print MODULE_ID" << std::endl;
        }
      }
      else if (cmd == "on") {
        std::string moduleName;
        int n;
        iss >> moduleName;
        if (iss) {
          if (std::isdigit(moduleName[0])) {
            n = boost::lexical_cast<int>(moduleName);
          }
          else {
            n = getModuleNumber(moduleName, false);
          }
          InteractiveModuleSwitch(n, true);
        }
        else {
          std::cout << "usage: on MODULE_ID" << std::endl;
        }
      }
      else if (cmd == "off") {
        std::string moduleName;
        int n;
        iss >> moduleName;
        if (iss) {
          if (std::isdigit(moduleName[0])) {
            n = boost::lexical_cast<int>(moduleName);
          }
          else {
            n = getModuleNumber(moduleName, false);
          }
          InteractiveModuleSwitch(n, false);
        }
        else {
          std::cout << "usage: off MODULE_ID" << std::endl;
        }
      }
      else if (cmd == "init") {
        status = Initialize();
        break;
      }
      else {
        std::cout << "command not found." << std::endl;
      }
    }
    else {
#if ANLNEXT_USE_READLINE
      free(line);
      line = 0;
#endif
    }
    
    iss.str("");
    iss.clear();
  }
  
#if ANLNEXT_USE_READLINE
#else
  history_end(anlHistory);
  el_end(el);
#endif
  return status;
}


void ANLNext::InteractiveComHelp()
{
  std::cout << "-------------------------------------------------------\n"
            << "  help              : show this help\n"
            << "  show              : show analysis chain\n"
            << "  print MODULE_ID   : show paramters of the module\n"
            << "  mod MODULE_ID     : modify parameters of the module\n"
            << "                      (enter mod_com() method)\n"
            << "  on MODULE_ID      : switch on the module\n"
            << "  off MODULE_ID     : switch off the module\n"
            << "  init              : initialize to start analysis\n"
            << "  quit              : quit this program\n"
            << "\n"
            << "    MODULE_ID = 0 for all\n"
            << "-------------------------------------------------------\n"
            << std::endl;
}


ANLStatus ANLNext::InteractiveModifyParam(int n)
{
  ANLStatus status = AS_OK;
  if (n==0) {
    for (AMIter mod=m_Modules.begin(); mod!=m_Modules.end(); ++mod) {
      if ((*mod)->is_off()) continue;
      std::cout << (*mod)->module_name() << " mod_com()" << std::endl;
      status = (*mod)->mod_com();
      if (status != AS_OK) {
        std::cout << (*mod)->module_name()
                  << " mod_com() returned "
                  << status << std::endl;
      }
      status = (*mod)->mod_prepare();
      if (status != AS_OK) {
        std::cout << (*mod)->module_name()
                  << " mod_prepare() returned "
                  << status << std::endl;
      }
      std::cout << std::endl;
    }
  }
  else if (0<n && n<=static_cast<int>(m_Modules.size())) {
    ANLVModule* mo = m_Modules[n-1];
    std::cout << mo->module_name()
              << " mod_com()" << std::endl;
    status = mo->mod_com();
    if (status != AS_OK) {
      std::cout << mo->module_name()
                << " mod_com() returned "
                << status << std::endl;
    }
    status = mo->mod_prepare();
    if (status != AS_OK) {
      std::cout << mo->module_name()
                << " mod_prepare() returned "
                << status << std::endl;
    }
  }
  else {
    std::cout << "input argument is out of range." << std::endl;
  }
  return status;
}


void ANLNext::InteractivePrintParam(int n)
{
  if (n==0) {
    for (AMIter mod=m_Modules.begin(); mod!=m_Modules.end(); ++mod) {
      std::cout << (*mod)->module_name() << " parameters" << std::endl;
      (*mod)->print_parameters();
      std::cout << std::endl;
    }
  }
  else if (0<n && n<=static_cast<int>(m_Modules.size())) {
    ANLVModule* mo = m_Modules[n-1];
    std::cout << mo->module_name() << " parameters" << std::endl;
    mo->print_parameters();
  }
  else {
    std::cout << "input argument is out of range." << std::endl;
  } 
}


void ANLNext::InteractiveModuleSwitch(int n, bool module_sw)
{
  if (n==0) {
    for (AMIter mod=m_Modules.begin(); mod!=m_Modules.end(); ++mod) {
      if (module_sw) {
        (*mod)->on();
        std::cout << (*mod)->module_name() << " turned on." << std::endl;  
      }
      else {
        (*mod)->off();
        std::cout << (*mod)->module_name() << " turned off." << std::endl;  
      }
    }
  }
  else if (0<n && n<=static_cast<int>(m_Modules.size())) {
    ANLVModule* mo = m_Modules[n-1];
    if (module_sw) {
      mo->on();
      std::cout << mo->module_name() << " turned on." << std::endl;
    }
    else {
      mo->off();
      std::cout << mo->module_name() << " turned off." << std::endl;
    }
  }
  else {
    std::cout << "input argument is out of range." << std::endl;
  }
}


ANLStatus ANLNext::InteractiveAna() throw(ANLException)
{
  ANLStatus status = AS_OK;

#if ANLNEXT_USE_READLINE
  int count = 0;
  char *line;
#else
  int count = 0;
  const char *line;
  EditLine *el;
  History *anlHistory;
  HistEvent event;

  el = el_init("ANLNext_Ana", stdin, stdout, stderr);
  el_set(el, EL_PROMPT, &prompt);
  el_set(el, EL_EDITOR, "emacs");

  anlHistory = history_init();
  if (anlHistory == 0) {
    std::cerr << "history initialization falied" << std::endl;
    return AS_QUIT;
  }
  
  history(anlHistory, &event, H_SETSIZE, 1000);
  el_set(el, EL_HIST, history, anlHistory);
#endif
  
  std::string cmd;
  std::istringstream iss;

  std::cout << "\n ** type \"help\" if you need. ** \n" << std::endl;

  while (iss.str()=="") {
#if ANLNEXT_USE_READLINE
    line = readline("ANL> ");
    count = strlen(line)+1;
#else
    line = el_gets(el, &count);
#endif

    if (count > 1) {
#if ANLNEXT_USE_READLINE
      add_history(line);
#else
      history(anlHistory, &event, H_ENTER, line);
#endif
      iss.str(line);

#if ANLNEXT_USE_READLINE
      free(line);
      line = 0;
#endif

      iss >> cmd;
      if (cmd == "exit") {
        break;
      }
      else if (cmd == "help") {
        InteractiveAnaHelp();
      }
      else if (cmd == "ana") {
        int n, disp;
        iss >> n >> disp;
        if (iss) {
          status = Analyze(n, disp);
        }
        else {
          std::cout << "usage: ana NUMBER DISPLAY" << std::endl;
        }
      }
      else {
        std::cout << "command not found." << std::endl;
      }
    }
    else {
#if ANLNEXT_USE_READLINE
      free(line);
      line = 0;
#endif
    }

    iss.str("");
    iss.clear();
  }
  
#if ANLNEXT_USE_READLINE
#else
  history_end(anlHistory);
  el_end(el);
#endif
  
  return status;
}


void ANLNext::InteractiveAnaHelp()
{
  std::cout << "-------------------------------------------------------\n"
            << "  help              : show this help\n"
            << "  ana TIMES DISPLAY : start analysis\n"
            << "                      TIMES: number of loop\n"
            << "                      DISPLAY: display frequency\n"
            << "  exit              : exit this program\n"
            << "                      (enter exit routine)\n"
            << "-------------------------------------------------------\n"
            << std::endl;
}
