%module anlnextpy
%{
#define SWIG_FILE_WITH_INIT
#include "ANLManager.hh"
#include "ANLManagerMT.hh"
#include "VModuleParameter.hh"
#include "ParameterRegistry.hh"
#include "BasicSubModule.hh"
#include "BasicModule.hh"
#include "ANLException.hh"
%}

%include "exception.i"
%include "stdint.i"
%include "std_string.i"
%include "std_vector.i"
%include "std_list.i"
%include "std_pair.i"
%include "std_map.i"

%template() std::map<int, int>;
%template() std::map<int, double>;
%template() std::map<int, std::string>;
%template() std::map<std::string, int>;
%template() std::map<std::string, double>;
%template() std::map<std::string, std::string>;


%template(ModuleVector) std::vector<anlnext::BasicModule*>;

%template(VectorI) std::vector<int>;
%template(VectorD) std::vector<double>;
%template(VectorStr) std::vector<std::string>;

%template(ListI) std::list<int>;

%template(PairII) std::pair<int, int>;
%template(PairDD) std::pair<double, double>;
%template(VectorPairII) std::vector<std::pair<int, int>>;
%template(VectorPairDD) std::vector<std::pair<double, double>>;

namespace std
{
  template<class T> class shared_ptr {
  public:
    T *operator->();
  };
}

%template(SPtrModParam) std::shared_ptr<anlnext::VModuleParameter>;
%template(SPtrModParamConst) std::shared_ptr<anlnext::VModuleParameter const>;
%template(ListModParam) std::list<std::shared_ptr<anlnext::VModuleParameter> >;
%template(VectorModParam) std::vector<std::shared_ptr<anlnext::VModuleParameter>>;

namespace anlnext
{

enum class ANLStatus {
  ok,
  error,
  skip,
  skip_error,
  quit,
  quit_error,
  quit_all,
  quit_all_error,
  critical_error_to_finalize,
  critical_error_to_terminate,
  critical_error_to_finalize_from_exception,
  critical_error_to_terminate_from_exception,
};

std::string status_to_string(ANLStatus status);

struct ANLException
{
  static void set_verbose_level(int v);
  static int verbose_level();

  ANLException();
  virtual ~ANLException();
  const std::string to_string();
};

%nodefaultctor VModuleParameter;
class VModuleParameter
{
 public:
  std::string name() const;
  double unit() const;
  std::string unit_name() const;

  std::string expression() const;
  std::string question() const;
  std::string default_string() const;

  bool is_hidden();

  std::string description() const;

  std::string type_name() const;
  
  %rename(get_value_vector_i) get_value(const std::vector<int>&) const;
  %rename(get_value_vector_d) get_value(const std::vector<double>&) const;
  %rename(get_value_vector_str) get_value(const std::vector<std::string>&) const;

  %exception {
    try { $action }
    catch (const anlnext::ANLException& ex) {
      SWIG_exception(SWIG_RuntimeError, ex.to_string().c_str());
    }
  }
  %rename(get_value_map_i_i) get_value(const std::map<int, int>&) const;
  std::map<int, int> get_value(const std::map<int, int>&) const;
  %rename(get_value_map_i_d) get_value(const std::map<int, double>&) const;
  std::map<int, double> get_value(const std::map<int, double>&) const;
  %rename(get_value_map_i_str) get_value(const std::map<int, std::string>&) const;
  std::map<int, std::string> get_value(const std::map<int, std::string>&) const;
  %rename(get_value_map_str_i) get_value(const std::map<std::string, int>&) const;
  std::map<std::string, int> get_value(const std::map<std::string, int>&) const;
  %rename(get_value_map_str_d) get_value(const std::map<std::string, double>&) const;
  std::map<std::string, double> get_value(const std::map<std::string, double>&) const;
  %rename(get_value_map_str_str) get_value(const std::map<std::string, std::string>&) const;
  std::map<std::string, std::string> get_value(const std::map<std::string, std::string>&) const;
  bool get_value(bool) const;
  int get_value(int) const;
  double get_value(double) const;
  std::string get_value(const std::string&) const;
  std::vector<int> get_value(const std::vector<int>&) const;
  std::vector<double> get_value(const std::vector<double>&) const;
  std::vector<std::string> get_value(const std::vector<std::string>&) const;
  // std::list<std::string> get_value(const std::list<std::string>&) const;
  std::vector<double> get_value(double, double) const;
  std::vector<double> get_value(double, double, double) const;

  intmax_t get_value_integer() const;
  uintmax_t get_value_unsigned_integer() const;

  std::string map_key_name() const;
  std::size_t num_value_elements() const;

  /**
   * modify return type to normal pointer.
   * std::shared_ptr<VModuleParameter const> value_element_info(std::size_t index) const;
   */
  %extend {
    const VModuleParameter* value_element_info(std::size_t index) const
    {
      return $self->value_element_info(index).get();
    }
  }

  std::string value_element_name(std::size_t index) const;
  
  std::size_t size_of_container() const;
  std::vector<std::string> map_key_list() const;
  void retrieve_from_container(const std::string& /* key */) const;
  void retrieve_from_container(std::size_t /* index */) const;
  std::vector<std::shared_ptr<anlnext::VModuleParameter>> value_elements_from_container(const std::string& key) const;
  std::vector<std::shared_ptr<anlnext::VModuleParameter>> value_elements_from_container(std::size_t index) const;

  %extend {
    std::string __str__()
    {
      return $self->value_string();
    }
  }
  std::string value_string() const;

  %exception;
};

using ModuleParam_sptr = std::shared_ptr<VModuleParameter>;
using ModuleParamList = std::list<ModuleParam_sptr>;
using ModuleParamIter = ModuleParamList::iterator;
using ModuleParamConstIter = ModuleParamList::const_iterator;

class ParameterRegistry
{
 public:
  ParameterRegistry();
  virtual ~ParameterRegistry();

  %exception{
    try {
      $action
    }
    catch (anlnext::ANLException& ex) {
      SWIG_exception(SWIG_RuntimeError, ex.to_string().c_str());
    }
  }

  void expose_parameter(const std::string& name);
  
  ModuleParamConstIter parameter_begin() const;
  ModuleParamConstIter parameter_end() const;
  const VModuleParameter* get_parameter(const std::string& name) const;

  %rename(set_parameter_vector_i)
  set_parameter(const std::string&, const std::vector<int>&);
  %rename(set_parameter_vector_d)
  set_parameter(const std::string&, const std::vector<double>&);
  %rename(set_parameter_vector_str)
  set_parameter(const std::string&, const std::vector<std::string>&);

  %rename(set_parameter_map_i_i) set_parameter(const std::string&, const std::map<int, int>&);
  void set_parameter(const std::string& name, const std::map<int, int>& val);
  %rename(set_parameter_map_i_d) set_parameter(const std::string&, const std::map<int, double>&);
  void set_parameter(const std::string& name, const std::map<int, double>& val);
  %rename(set_parameter_map_i_str) set_parameter(const std::string&, const std::map<int, std::string>&);
  void set_parameter(const std::string& name, const std::map<int, std::string>& val);
  %rename(set_parameter_map_str_i) set_parameter(const std::string&, const std::map<std::string, int>&);
  void set_parameter(const std::string& name, const std::map<std::string, int>& val);
  %rename(set_parameter_map_str_d) set_parameter(const std::string&, const std::map<std::string, double>&);
  void set_parameter(const std::string& name, const std::map<std::string, double>& val);
  %rename(set_parameter_map_str_str) set_parameter(const std::string&, const std::map<std::string, std::string>&);
  void set_parameter(const std::string& name, const std::map<std::string, std::string>& val);

  void set_parameter(const std::string& name, bool val);
  void set_parameter(const std::string& name, int val);
  void set_parameter(const std::string& name, double val);
  void set_parameter(const std::string& name, const std::string& val);
  void set_parameter(const std::string& name,
                     const std::vector<int>& val);
  void set_parameter(const std::string& name,
                     const std::vector<double>& val);
  void set_parameter(const std::string& name,
                     const std::vector<std::string>& val);
  void set_parameter(const std::string& name, double x, double y);
  void set_parameter(const std::string& name, double x, double y, double z);
  void set_parameter_integer(const std::string& name, intmax_t val);
  void set_parameter_unsigned_integer(const std::string& name, uintmax_t val);

  void clear_array(const std::string& name);

  void set_map_key(const std::string& key);
  void set_value_element(const std::string& name, bool val);
  void set_value_element(const std::string& name, int val);
  void set_value_element(const std::string& name, double val);
  void set_value_element(const std::string& name, const std::string& val);
  void insert_to_container();

  virtual void print_parameters() const;

  %exception;
};

class BasicSubModule : public ParameterRegistry
{
 public:
  BasicSubModule();
  virtual ~BasicSubModule();

  std::string submodule_name() const;
  std::string submodule_version() const;

  // mix-in ModuleDescription
  std::string module_description() const;
  void set_module_description(const std::string& v);
};

class BasicModule : public ParameterRegistry
{
 public:
  static const std::size_t undefined_run_index;
  static const std::size_t undefined_loop_index;

  BasicModule();
  virtual ~BasicModule();

  virtual ANLStatus mod_pre_initialize();

  std::string module_name() const;
  std::string module_version() const;
  
  void set_module_id(const std::string& v);
  std::string module_id() const;

  std::size_t get_run_index() const;
  std::size_t get_loop_index() const;

  int copy_id() const;
  bool is_master() const;

  void set_order_sensitive(bool v);
  bool is_order_sensitive() const;

  void set_singleton(int copy_id);
  void unset_singleton();
  bool is_singleton() const;
  int singleton_copy_id() const;
  
  std::vector<std::string> get_aliases_string() const;
  void add_alias(const std::string& name);
  
  void on();
  void off();
  bool is_on();
  bool is_off();

  %exception {
    try {
      $action
    }
    catch (const anlnext::ANLException& ex) {
      SWIG_exception(SWIG_RuntimeError, ex.to_string().c_str());
    }
  }

  virtual void set_submodule_by_key(const std::string& /* name */, const std::string& /* keyword */);
  bool is_submodule_defined(const std::string& name) const;
  bool is_submodule_set(const std::string& name) const;
  BasicSubModule* get_submodule(const std::string& name);
  const BasicSubModule* get_submodule(const std::string& name) const;
  std::vector<std::string> submodule_names() const;

  %exception;

  // mix-in ModuleDescription
  std::string module_description() const;
  void set_module_description(const std::string& v);
};

class ANLManager
{
 public:
  static const std::size_t undefined_run_index;

  ANLManager();
  virtual ~ANLManager();
  
  %exception{
    try {
      $action
    }
    catch (const anlnext::ANLException& ex) {
      SWIG_exception(SWIG_RuntimeError, ex.to_string().c_str());
    }
  }

  std::size_t run_index() const;

  void set_display_period(std::size_t v);
  std::size_t display_period() const;
  
  void set_modules(std::vector<anlnext::BasicModule*> modules);

  virtual ANLStatus define_analysis();
  virtual ANLStatus pre_initialize_analysis();
  virtual ANLStatus initialize_analysis();
  virtual ANLStatus run_analysis(std::size_t num_loops, bool enable_console=true);
  virtual ANLStatus finalize_analysis();

  virtual std::size_t number_of_parallels() const;
  void set_print_parallel_modules(bool v=true);
  virtual BasicModule* access_to_module(std::size_t chain_id,
                                        const std::string& module_id);

  void parameters_to_json(const std::string& filename) const;

  virtual ANLStatus do_interactive_comunication();
  virtual ANLStatus do_interactive_analysis();

  %exception;
};

class ANLManagerMT : public ANLManager
{
public:
  explicit ANLManagerMT(std::size_t num_parallels=1);
  virtual ~ANLManagerMT();
};
 
} /* namespace anlnext */
