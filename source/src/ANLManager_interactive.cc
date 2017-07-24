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
#include "CLIUtility.hh"
#endif /* ANL_USE_READLINE */


namespace anl
{

ANLStatus ANLManager::do_interactive_comunication()
{
  ANLStatus status = AS_OK;
  
  std::cout << "\n ** type \"help\" if you need. ** \n" << std::endl;

  while (true) {
#if ANL_USE_READLINE
    ReadLine reader;
    reader.set_completion_candidates(
      {"quit", "help", "chain", "show", "review", "modify", "print", "on", "off", "initialize"}
      );
    const int count = reader.read("iANL> ");
    if (count == -1) { return AS_QUIT_ERROR; }
    if (count == 0) { continue; }
    std::istringstream is(reader.c_str());
#else
    std::cout << "iANL>> ";
    std::cout.flush();
    std::string line;
    std::getline(std::cin, line);
    std::istringstream is(line);
#endif /* ANL_USE_READLINE */

    std::string cmd;
    is >> cmd;
 
    if (cmd == "") {
      continue;
    }
    else if (cmd == "quit") {
      std::cout << "ANL quitting..." << std::endl;
      status = AS_QUIT;
      break;
    }
    else if (cmd == "help") {
      interactive_comunication_help();
    }
    else if (cmd == "chain") {
      show_analysis();
    }
    else if (cmd == "show") {
      show_analysis();
      interactive_print_param(-1);
    }
    else if (cmd == "review") {
      try {
        status = interactive_modify_param(-1);
      }
      catch (const ParameterInputError& ex) {
        print_exception(ex);
        std::cout << "You can retry." << std::endl;
      }
    }
    else if (cmd == "modify") {
      std::string moduleID;
      int n;
      is >> moduleID;
      if (is) {
        try {
          if (std::isdigit(moduleID[0])) {
            n = boost::lexical_cast<int>(moduleID);
          }
          else {
            n = module_index(moduleID, false);
            if (n<0) {
              std::cout << "Module not found: " << moduleID << std::endl;
              continue;
            }
          }
          status = interactive_modify_param(n);
        }
        catch (const ParameterInputError& ex) {
          print_exception(ex);
          std::cout << "You can retry." << std::endl;
        }
      }
      else {
        std::cout << "usage: mod <module_id>" << std::endl;
      }
    }
    else if (cmd == "print") {
      std::string moduleID;
      int n;
      is >> moduleID;
      if (is) {
        if (std::isdigit(moduleID[0])) {
          n = boost::lexical_cast<int>(moduleID);
        }
        else {
          n = module_index(moduleID, false);
          if (n<0) {
            std::cout << "Module not found: " << moduleID << std::endl;
            continue;
          }
        }
        interactive_print_param(n);
      }
      else {
        std::cout << "usage: print <module_id>" << std::endl;
      }
    }
    else if (cmd == "on") {
      std::string moduleID;
      int n;
      is >> moduleID;
      if (is) {
        if (std::isdigit(moduleID[0])) {
          n = boost::lexical_cast<int>(moduleID);
        }
        else {
          n = module_index(moduleID, false);
          if (n<0) {
            std::cout << "Module not found: " << moduleID << std::endl;
            continue;
          }
        }
        interactive_module_switch(n, true);
      }
      else {
        std::cout << "usage: on <module_id>" << std::endl;
      }
    }
    else if (cmd == "off") {
      std::string moduleID;
      int n;
      is >> moduleID;
      if (is) {
        if (std::isdigit(moduleID[0])) {
          n = boost::lexical_cast<int>(moduleID);
        }
        else {
          n = module_index(moduleID, false);
          if (n<0) {
            std::cout << "Module not found: " << moduleID << std::endl;
            continue;
          }
        }
        interactive_module_switch(n, false);
      }
      else {
        std::cout << "usage: off <module_id>" << std::endl;
      }
    }
    else if (cmd == "init" || cmd == "initialize") {
      break;
    }
    else {
      std::cout << "command not found." << std::endl;
    }
    
    is.clear();
  }
  
  return status;
}

void ANLManager::interactive_comunication_help()
{
  std::cout << "-------------------------------------------------------\n"
            << "  help               : show this help\n"
            << "  chain              : show analysis chain\n"
            << "  show               : show analysis chain\n"
            << "                       and all parameters\n"
            << "  print <module_id>  : show paramters of the module\n"
            << "  review             : review parameters of all modules\n"
            << "                       (same as \"modify -1\")\n"
            << "  modify <module_id> : modify parameters of the module\n"
            << "                       (enter mod_communicate() method)\n"
            << "  on <module_id>     : switch on the module\n"
            << "  off <module_id>    : switch off the module\n"
            << "  initialize         : initialize to start analysis\n"
            << "  init               : = initialize\n"
            << "  quit               : quit this program\n"
            << "\n"
            << "   <module_id> can be either module ID    (string)\n"
            << "                          or module index (integer)\n"
            << "   <module_id> = -1 for all\n"
            << "-------------------------------------------------------\n"
            << std::endl;
}

ANLStatus ANLManager::interactive_modify_param(int n)
{
  ANLStatus status = AS_OK;
  if (n==-1) {
    for (BasicModule* m: modules_) {
      if (m->is_off()) { continue; }
      std::cout << m->module_name() << " mod_communicate()" << std::endl;
      status = m->mod_communicate();
      if (status != AS_OK) {
        std::cout << m->module_name()
                  << " mod_communicate() returned "
                  << status << std::endl;
      }
      std::cout << std::endl;
    }
  }
  else if (0<=n && n<static_cast<int>(modules_.size())) {
    BasicModule* m = modules_[n];
    std::cout << m->module_name()
              << " mod_communicate()" << std::endl;
    status = m->mod_communicate();
    if (status != AS_OK) {
      std::cout << m->module_name()
                << " mod_communicate() returned "
                << status << std::endl;
    }
  }
  else {
    std::cout << "Module index " << n << " is out of range." << std::endl;
  }
  return status;
}

void ANLManager::interactive_print_param(int n)
{
  if (n==-1) {
    std::cout << '\n'
              << "        **************************************\n"
              << "        ****      Module parameters       ****\n"
              << "        **************************************\n"
              << std::endl;

    for (BasicModule* m: modules_) {
      std::cout << "--- " << m->module_id() << " ---"<< std::endl;
      m->print_parameters();
      std::cout << std::endl;
    }
  }
  else if (0<=n && n<static_cast<int>(modules_.size())) {
    BasicModule* m = modules_[n];
    std::cout << "--- " << m->module_id() << " ---"<< std::endl;
    m->print_parameters();
  }
  else {
    std::cout << "Module index " << n << " is out of range." << std::endl;
  } 
}

void ANLManager::interactive_module_switch(int n, bool module_sw)
{
  if (n==-1) {
    for (BasicModule* m: modules_) {
      if (module_sw) {
        m->on();
        std::cout << m->module_name() << " turned on." << std::endl;
      }
      else {
        m->off();
        std::cout << m->module_name() << " turned off." << std::endl;
      }
    }
  }
  else if (0<=n && n<static_cast<int>(modules_.size())) {
    BasicModule* m = modules_[n];
    if (module_sw) {
      m->on();
      std::cout << m->module_name() << " turned on." << std::endl;
    }
    else {
      m->off();
      std::cout << m->module_name() << " turned off." << std::endl;
    }
  }
  else {
    std::cout << "Module index " << n << " is out of range." << std::endl;
  }
}

ANLStatus ANLManager::do_interactive_analysis()
{
  ANLStatus status = AS_OK;

  std::cout << "\n ** type \"help\" if you need. ** \n" << std::endl;

  while (true) {
#if ANL_USE_READLINE
    ReadLine reader;
    reader.set_completion_candidates({"exit", "help", "run"});
    const int count = reader.read("iANL> ");
    if (count == -1) { return AS_QUIT_ERROR; }
    if (count == 0) { continue; }
    std::istringstream is(reader.c_str());
#else
    std::cout << "iANL>> ";
    std::cout.flush();
    std::string line;
    std::getline(std::cin, line);
    std::istringstream is(line);
#endif /* ANL_USE_READLINE */

    std::string cmd;
    is >> cmd;
 
    if (cmd == "") {
      continue;
    }
    else if (cmd == "exit") {
      break;
    }
    else if (cmd == "help") {
      interactive_analysis_help();
    }
    else if (cmd == "run") {
      int num_loops(0);
      is >> num_loops;
      if (!is) {
        std::cout << "usage: run <number>" << std::endl;
        std::cout << "usage: run <number> <display_frequence>" << std::endl;
      }
      else {
        int disp(0);
        is >> disp;
        if (is) {
          set_display_frequency(disp);
        }
        else {
          is.clear();
        }
        status = Analyze(num_loops, true);
      }
    }
    else {
      std::cout << "command not found." << std::endl;
    }

    is.clear();
  }
  
  return status;
}

void ANLManager::interactive_analysis_help()
{
  std::cout << "-------------------------------------------------------\n"
            << "  help              : show this help\n"
            << "  run <N> <display> : start analysis\n"
            << "                      <N>: number of loops\n"
            << "                      <display>: display frequency\n"
            << "  exit              : exit this program\n"
            << "                      (enter <finalize> stage)\n"
            << "-------------------------------------------------------\n"
            << std::endl;
}

} /* namespace anl */

#endif /* ANL_ENABLE_INTERACTIVE_MODE */
