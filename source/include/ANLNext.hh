#ifndef ANLNext_hh
#define ANLNext_hh

#define ANL_ANALYZE_INTERRUPT 1

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <boost/utility.hpp>

#include "ANLStatus.hh"
#include "ANLException.hh"


namespace anl
{

class EvsManager;
class ANLVModule;

struct ANLModuleCounter
{
  int entry;
  int ok;
  int err;
  int skip;
  int quit;
};

struct ANLToLower
{
  char operator() (char c) { return std::tolower(c); }
};


/**
 * The ANL Next manager class.
 * @author Hirokazu Odaka
 * @date June, 2010
 */

class ANLNext : private boost::noncopyable
{
public:
  ANLNext();
  ~ANLNext(); // non-virtual destructor

  /**
   * set ANL modules
   */
  void SetModules(std::vector<ANLVModule*> modules) throw(ANLException);

  ANLStatus Startup() throw(ANLException);
  ANLStatus Initialize() throw(ANLException);
  ANLStatus Analyze(int num_event, int display_freq) throw(ANLException);
  ANLStatus Exit() throw(ANLException);
  
  ANLStatus Prepare() throw(ANLException);

  ANLStatus InteractiveCom() throw(ANLException);
  ANLStatus InteractiveAna() throw(ANLException);
  
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
  void reset_counter();
  ANLStatus process_analysis(int num_event, int display_freq);
  void print_summary();

  int getModuleNumber(const std::string& name, bool strict=true);

  void InteractiveComHelp();
  ANLStatus InteractiveModifyParam(int n);
  void InteractivePrintParam(int n);
  void InteractiveModuleSwitch(int n, bool module_sw);
  void InteractiveAnaHelp();
  
  template<typename T>
  ANLStatus routine_modfn(T func, const std::string& func_id);

private:
  std::vector<ANLVModule*> m_Modules;
  EvsManager* m_Evs;
  std::vector<ANLModuleCounter> m_Counter;
};

} // namespace anl

#endif // ANLNext_hh
