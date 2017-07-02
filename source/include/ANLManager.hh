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

#ifndef ANL_ANLManager_H
#define ANL_ANLManager_H 1

#define ANL_ANALYZE_INTERRUPT 1
#define ANL_INITIALIZE_INTERRUPT 1
#define ANL_EXIT_INTERRUPT 1

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <boost/property_tree/ptree.hpp>

#include "ANLStatus.hh"
#include "ANLException.hh"
#include "LoopCounter.hh"

namespace anl
{

class EvsManager;
class ModuleAccess;
class BasicModule;

/**
 * The ANL Next manager class.
 *
 * @author Hirokazu Odaka
 * @date 2010-06-xx
 * @date 2015-08-15 | version 1.7
 * @date 2017-07-02 | version 1.9 | simpler event loop
 */
class ANLManager
{
public:
  ANLManager();
  virtual ~ANLManager();

  ANLManager(const ANLManager&) = delete;
  ANLManager(ANLManager&&) = delete;
  ANLManager& operator=(const ANLManager&) = delete;
  ANLManager& operator=(ANLManager&&) = delete;

  /**
   * set ANL modules
   */
  void SetModules(std::vector<BasicModule*> modules);

  ANLStatus Startup();
  ANLStatus Prepare();
  ANLStatus Initialize();
  ANLStatus Analyze(long int num_events, bool thread_mode=false);
  ANLStatus Exit();

  ANLStatus InteractiveCom();
  ANLStatus InteractiveAna();

  void SetDisplayFrequency(long int v) { displayFrequency_ = v; }
  long int DisplayFrequency() const { return displayFrequency_; }

  boost::property_tree::ptree parameters_to_property_tree() const;
  void parameters_to_json(const std::string& filename) const;
  
private:
  ANLStatus routine_startup();
  ANLStatus routine_prepare();
  ANLStatus routine_init();
  ANLStatus routine_his();
  ANLStatus routine_bgnrun();
  ANLStatus routine_endrun();
  ANLStatus routine_exit();

  void show_analysis();
  void print_parameters();
  void reset_counters();
  ANLStatus process_analysis(long int num_events);
  void print_summary();

  int ModuleIndex(const std::string& module_id, bool strict=true) const;

#if ANL_ENABLE_INTERACTIVE_MODE
  void InteractiveComHelp();
  ANLStatus InteractiveModifyParam(int n);
  void InteractivePrintParam(int n);
  void InteractiveModuleSwitch(int n, bool module_sw);
  void InteractiveAnaHelp();
#endif /* ANL_ENABLE_INTERACTIVE_MODE */

  template<typename T>
  ANLStatus routine_modfn(T func, const std::string& func_id);

  // thread mode
private:
  void __void_process_analysis(long int num_events, ANLStatus* status);
  void interactive_session();

private:
  std::vector<BasicModule*> modules_;
  std::vector<LoopCounter> counters_;
  std::unique_ptr<EvsManager> evsManager_;
  std::unique_ptr<ModuleAccess> moduleAccess_;
  long int displayFrequency_ = -1;
  bool interrupted_ = false;
};

} /* namespace anl */

#endif /* ANL_ANLManager_H */
