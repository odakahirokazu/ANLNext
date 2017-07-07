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
#define ANL_FINALIZE_INTERRUPT 1

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <atomic>
#include <mutex>
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
 * @date 2017-07-04 | new model
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

  long int NumberOfLoops() const { return numEvents_; }

  void SetDisplayFrequency(long int v) { displayFrequency_ = v; }
  long int DisplayFrequency() const;

  virtual ANLStatus Define();
  virtual ANLStatus PreInitialize();
  virtual ANLStatus Initialize();
  virtual ANLStatus Analyze(long int num_events, bool thread_mode=false);
  virtual ANLStatus Finalize();

  virtual ANLStatus InteractiveComunication();
  virtual ANLStatus InteractiveAnalysis();

  boost::property_tree::ptree parameters_to_property_tree() const;
  void parameters_to_json(const std::string& filename) const;

protected:
  virtual ANLStatus routine_define();
  virtual ANLStatus routine_pre_initialize();
  virtual ANLStatus routine_initialize();
  virtual ANLStatus routine_begin_run();
  virtual ANLStatus routine_end_run();
  virtual ANLStatus routine_finalize();

  void show_analysis();
  void print_parameters();
  virtual void reset_counters();
  virtual ANLStatus process_analysis();
  void print_summary();

  int ModuleIndex(const std::string& module_id, bool strict=true) const;

#if ANL_ENABLE_INTERACTIVE_MODE
  void InteractiveComHelp();
  ANLStatus InteractiveModifyParam(int n);
  void InteractivePrintParam(int n);
  void InteractiveModuleSwitch(int n, bool module_sw);
  void InteractiveAnaHelp();
#endif /* ANL_ENABLE_INTERACTIVE_MODE */

private:
  virtual void duplicateChains() {}
  virtual void reduceStatistics() {}

  // thread mode
private:
  void __void_process_analysis(ANLStatus* status);
  void interactive_session();

protected:
  long int numEvents_ = 0;
  std::vector<BasicModule*> modules_;
  std::vector<LoopCounter> counters_;
  std::unique_ptr<EvsManager> evsManager_;
  std::mutex mutex_;
  std::atomic<bool> interrupted_{false};

private:
  std::unique_ptr<ModuleAccess> moduleAccess_;
  long int displayFrequency_ = -1;
};

/**
 * non-member functions
 */

template<typename T>
ANLStatus routine_modfn(T func,
                        const std::string& func_id,
                        const std::vector<BasicModule*>& modules);

ANLStatus process_one_event(long int iEvent,
                            const std::vector<BasicModule*>& modules,
                            std::vector<LoopCounter>& counters,
                            EvsManager& evsManager);

} /* namespace anl */

#endif /* ANL_ANLManager_H */
