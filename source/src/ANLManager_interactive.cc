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

#if ANL_ENABLE_INTERACTIVE_MODE

#include "ANLManager.hh"
#include "BasicModule.hh"

#include <sstream>
#include <iomanip>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#if ANL_USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#else /* ANL_USE_READLINE */
#include <histedit.h>
#endif /* ANL_USE_READLINE */


namespace
{
#if !ANL_USE_READLINE
const char* prompt(EditLine*)
{
  return "ANL> ";
}
#endif /* !ANL_USE_READLINE */
}

namespace anl
{

ANLStatus ANLManager::do_interactive_comunication()
{
  ANLStatus status = AS_OK;
  
#if ANL_USE_READLINE
  char *line;
  int count = 0;
#else /* ANL_USE_READLINE */
  const char *line;
  int count = 0;
  EditLine *el;
  History *anlHistory;
  HistEvent event;
  el = el_init("ANL_Com", stdin, stdout, stderr);
  el_set(el, EL_PROMPT, &prompt);
  el_set(el, EL_EDITOR, "emacs");

  anlHistory = history_init();
  if (anlHistory == 0) {
    std::cerr << "history initialization falied" << std::endl;
    return AS_QUIT;
  }
  
  history(anlHistory, &event, H_SETSIZE, 1000);
  el_set(el, EL_HIST, history, anlHistory);
#endif /* ANL_USE_READLINE */
  
  std::string cmd;
  std::istringstream iss;

  std::cout << "\n ** type \"help\" if you need. ** \n" << std::endl;

  while (iss.str()=="") {
#if ANL_USE_READLINE
    line = readline("ANL> ");
    if (line) count = strlen(line)+1;
    else {
      exit(1);
    }
#else /* ANL_USE_READLINE */
    line = el_gets(el, &count);
#endif /* ANL_USE_READLINE */
    
    if (count > 1) {
#if ANL_USE_READLINE
      add_history(line);
#else /* ANL_USE_READLINE */
      history(anlHistory, &event, H_ENTER, line);
#endif /* ANL_USE_READLINE */

      iss.str(line);
      
#if ANL_USE_READLINE
      free(line);
      line = 0;
#endif /* ANL_USE_READLINE */

      iss >> cmd;
 
      if (cmd == "quit") {
        std::cout << "ANL quitting..." << std::endl;
        status = AS_QUIT;
        break;
      }
      else if (cmd == "help") {
        interactive_comunication_help();
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
              n = module_index(moduleName, false);
            }
            status = interactive_modify_param(n);
          }
          catch (const ANLException& ex) {
            std::cout << "Exception!\n"
                      << *boost::get_error_info<ANLErrorInfo>(ex) << std::endl;
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
            n = module_index(moduleName, false);
          }
          interactive_print_param(n);
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
            n = module_index(moduleName, false);
          }
          interactive_module_switch(n, true);
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
            n = module_index(moduleName, false);
          }
          interactive_module_switch(n, false);
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
#if ANL_USE_READLINE
      free(line);
      line = 0;
#endif /* ANL_USE_READLINE */
    }
    
    iss.str("");
    iss.clear();
  }
  
#if ANL_USE_READLINE
#else /* ANL_USE_READLINE */
  history_end(anlHistory);
  el_end(el);
#endif /* ANL_USE_READLINE */
  return status;
}

void ANLManager::interactive_comunication_help()
{
  std::cout << "-------------------------------------------------------\n"
            << "  help              : show this help\n"
            << "  show              : show analysis chain\n"
            << "  print MODULE_ID   : show paramters of the module\n"
            << "  mod MODULE_ID     : modify parameters of the module\n"
            << "                      (enter mod_communicate() method)\n"
            << "  on MODULE_ID      : switch on the module\n"
            << "  off MODULE_ID     : switch off the module\n"
            << "  init              : initialize to start analysis\n"
            << "  quit              : quit this program\n"
            << "\n"
            << "    MODULE_ID = 0 for all\n"
            << "-------------------------------------------------------\n"
            << std::endl;
}

ANLStatus ANLManager::interactive_modify_param(int n)
{
  ANLStatus status = AS_OK;
  if (n==-1) {
    for (AMIter mod=modules_.begin(); mod!=modules_.end(); ++mod) {
      if ((*mod)->is_off()) continue;
      std::cout << (*mod)->module_name() << " mod_communicate()" << std::endl;
      status = (*mod)->mod_communicate();
      if (status != AS_OK) {
        std::cout << (*mod)->module_name()
                  << " mod_communicate() returned "
                  << status << std::endl;
      }
      status = (*mod)->mod_pre_initialize();
      if (status != AS_OK) {
        std::cout << (*mod)->module_name()
                  << " mod_pre_initialize() returned "
                  << status << std::endl;
      }
      std::cout << std::endl;
    }
  }
  else if (0<=n && n<static_cast<int>(modules_.size())) {
    BasicModule* mo = modules_[n];
    std::cout << mo->module_name()
              << " mod_communicate()" << std::endl;
    status = mo->mod_communicate();
    if (status != AS_OK) {
      std::cout << mo->module_name()
                << " mod_communicate() returned "
                << status << std::endl;
    }
    status = mo->mod_pre_initialize();
    if (status != AS_OK) {
      std::cout << mo->module_name()
                << " mod_pre_initialize() returned "
                << status << std::endl;
    }
  }
  else {
    std::cout << "input argument is out of range." << std::endl;
  }
  return status;
}

void ANLManager::interactive_print_param(int n)
{
  if (n==-1) {
    for (AMIter mod=modules_.begin(); mod!=modules_.end(); ++mod) {
      std::cout << (*mod)->module_name() << " parameters" << std::endl;
      (*mod)->print_parameters();
      std::cout << std::endl;
    }
  }
  else if (0<=n && n<static_cast<int>(modules_.size())) {
    BasicModule* mo = modules_[n];
    std::cout << mo->module_name() << " parameters" << std::endl;
    mo->print_parameters();
  }
  else {
    std::cout << "input argument is out of range." << std::endl;
  } 
}

void ANLManager::interactive_module_switch(int n, bool module_sw)
{
  if (n==-1) {
    for (AMIter mod=modules_.begin(); mod!=modules_.end(); ++mod) {
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
  else if (0<=n && n<static_cast<int>(modules_.size())) {
    BasicModule* mo = modules_[n];
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

ANLStatus ANLManager::do_interactive_analysis()
{
  ANLStatus status = AS_OK;

#if ANL_USE_READLINE
  int count = 0;
  char *line;
#else /* ANL_USE_READLINE */
  int count = 0;
  const char *line;
  EditLine *el;
  History *anlHistory;
  HistEvent event;

  el = el_init("ANL_Ana", stdin, stdout, stderr);
  el_set(el, EL_PROMPT, &prompt);
  el_set(el, EL_EDITOR, "emacs");

  anlHistory = history_init();
  if (anlHistory == 0) {
    std::cerr << "history initialization falied" << std::endl;
    return AS_QUIT;
  }
  
  history(anlHistory, &event, H_SETSIZE, 1000);
  el_set(el, EL_HIST, history, anlHistory);
#endif /* ANL_USE_READLINE */
  
  std::string cmd;
  std::istringstream iss;

  std::cout << "\n ** type \"help\" if you need. ** \n" << std::endl;

  while (iss.str()=="") {
#if ANL_USE_READLINE
    line = readline("ANL> ");
    count = strlen(line)+1;
#else /* ANL_USE_READLINE */
    line = el_gets(el, &count);
#endif /* ANL_USE_READLINE */

    if (count > 1) {
#if ANL_USE_READLINE
      add_history(line);
#else /* ANL_USE_READLINE */
      history(anlHistory, &event, H_ENTER, line);
#endif /* ANL_USE_READLINE */
      iss.str(line);

#if ANL_USE_READLINE
      free(line);
      line = 0;
#endif /* ANL_USE_READLINE */

      iss >> cmd;
      if (cmd == "exit") {
        break;
      }
      else if (cmd == "help") {
        interactive_analysis_help();
      }
      else if (cmd == "ana") {
        int n, disp;
        iss >> n >> disp;
        if (iss) {
          set_display_frequency(disp);
          status = Analyze(n, true);
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
#if ANL_USE_READLINE
      free(line);
      line = 0;
#endif /* ANL_USE_READLINE */
    }

    iss.str("");
    iss.clear();
  }
  
#if ANL_USE_READLINE
#else /* ANL_USE_READLINE */
  history_end(anlHistory);
  el_end(el);
#endif /* ANL_USE_READLINE */
  
  return status;
}

void ANLManager::interactive_analysis_help()
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

} /* namespace anl */

#endif /* ANL_ENABLE_INTERACTIVE_MODE */
