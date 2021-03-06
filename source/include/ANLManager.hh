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

#ifndef ANLNEXT_ANLManager_H
#define ANLNEXT_ANLManager_H 1

#define ANLNEXT_ANALYZE_INTERRUPT 1
#define ANLNEXT_INITIALIZE_INTERRUPT 1
#define ANLNEXT_FINALIZE_INTERRUPT 1

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <atomic>
#include <mutex>
#include <future>
#include <boost/property_tree/ptree.hpp>

#include "ANLStatus.hh"
#include "ANLException.hh"
#include "LoopCounter.hh"

namespace anlnext
{

class EvsManager;
class ModuleAccess;
class BasicModule;
class OrderKeeper;

/**
 * The ANL Next manager class.
 *
 * @author Hirokazu Odaka
 * @date 2010-06-xx
 * @date 2015-08-15 | version 1.7
 * @date 2017-07-02 | version 1.9 | simpler event loop
 * @date 2017-07-04 | new model, for multi-thread mode
 * @date 2017-07-07 | rename methods
 * @date 2017-07-19 | introduce user request, modify print messages.
 * @date 2019-12-25 | add module results feature
 */
class ANLManager
{
public:
  static const int __version1__;
  static const int __version2__;
  static const int __version3__;

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
  void set_modules(std::vector<BasicModule*> modules);

  long int number_of_loops() const { return num_events_; }

  void set_display_period(long int v) { display_period_ = v; }
  long int display_period() const;

  void set_exception_propagation(bool v)
  { exception_propagation_ = v; }
  bool exception_propagation() const
  { return exception_propagation_; }

  virtual ANLStatus Define();
  virtual ANLStatus PreInitialize();
  virtual ANLStatus Initialize();
  virtual ANLStatus Analyze(long int num_events, bool enable_console=false);
  virtual ANLStatus Finalize();

  virtual int number_of_parallels() const { return 1; }
  void set_print_parallel_modules(bool v=true)
  { print_clone_parameters_ = v; }
  virtual BasicModule* access_to_module(int chain_ID,
                                        const std::string& module_ID);

  virtual ANLStatus do_interactive_comunication();
  virtual ANLStatus do_interactive_analysis();

  virtual boost::property_tree::ptree parameters_to_property_tree() const;
  void parameters_to_json(const std::string& filename) const;

protected:
  virtual ANLStatus routine_define();
  virtual ANLStatus routine_pre_initialize();
  virtual ANLStatus routine_initialize();
  virtual ANLStatus routine_begin_run();
  virtual ANLStatus routine_end_run();
  virtual ANLStatus routine_finalize();

  void show_analysis();
  virtual void print_parameters();
  virtual void print_results();
  virtual void reset_counters();
  virtual ANLStatus process_analysis();
  void print_summary();

  int module_index(const std::string& module_id, bool strict=true) const;

#if ANLNEXT_ENABLE_INTERACTIVE_MODE
  void interactive_comunication_help();
  ANLStatus interactive_modify_param(int n);
  void interactive_print_param(int n);
  void interactive_module_switch(int n, bool module_sw);
  void interactive_analysis_help();
#endif /* ANLNEXT_ENABLE_INTERACTIVE_MODE */

private:
  virtual void duplicate_chains() {}
  virtual ANLStatus reduce_modules() { return AS_OK; }
  virtual void reduce_statistics() {}

  // thread mode
private:
  void process_analysis_for_the_thread(std::promise<ANLStatus> status_promise);
  void interactive_session();

protected:
  bool print_clone_parameters_ = false;
  long int num_events_ = 0;
  std::vector<BasicModule*> modules_;
  std::vector<LoopCounter> counters_;
  std::unique_ptr<EvsManager> evs_manager_;
  std::mutex mutex_;
  std::atomic<ANLRequest> requested_{ANLRequest::none};
  bool exception_propagation_ = true;

private:
  long int display_period_ = -1;
  std::unique_ptr<ModuleAccess> module_access_;
  std::atomic<bool> analysis_thread_finished_{false};
};

/**
 * non-member functions
 */

template<typename T>
ANLStatus routine_modfn(T func,
                        const std::string& func_id,
                        const std::vector<BasicModule*>& modules);

ANLStatus process_one_event(long int i_event,
                            const std::vector<BasicModule*>& modules,
                            std::vector<LoopCounter>& counters,
                            EvsManager& evsManager);

ANLStatus process_one_event(long int i_event,
                            const std::vector<BasicModule*>& modules,
                            std::vector<LoopCounter>& counters,
                            EvsManager& evs_manager,
                            std::vector<std::unique_ptr<OrderKeeper>>& order_keepers);

void count_evs(ANLStatus status, EvsManager& evs_manager);

inline void print_event_index(long int index, std::ostream& os=std::cout)
{
  os << "Event : " << std::dec << std::setw(10) << index << std::endl;
  os.width(0);
}

inline void print_exception(const ANLException& ex, std::ostream& os=std::cout)
{
  os << "################################################################\n"
     << "#                                                              #\n"
     << "#                       ANL Exception                          #\n"
     << "#                                                              #\n"
     << "################################################################\n"
     << "\n"
     << ex.to_string()
     << "\n"
     << "################################################################\n"
     << "\n"
     << "---- displayed by ANLManager::print_exception() ----\n"
     << "\n"
     << std::endl;
}

} /* namespace anlnext */

#endif /* ANLNEXT_ANLManager_H */
