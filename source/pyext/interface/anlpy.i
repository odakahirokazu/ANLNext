%module anlpy
%{
#define SWIG_FILE_WITH_INIT
#include "ANLManager.hh"
#include "ANLManagerMT.hh"
#include "VModuleParameter.hh"
#include "BasicModule.hh"
#include "ANLException.hh"
%}

%include "exception.i"
%include "stdint.i"
%include "std_string.i"
%include "std_vector.i"
%include "std_list.i"
%include "std_pair.i"

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
  static void SetVerboseLevel(int v);
  static int VerboseLevel();

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

  %extend {
    std::string __str__()
    {
      return $self->value_string();
    }
  }
  std::string value_string() const;
};

using ModuleParam_sptr = std::shared_ptr<VModuleParameter>;
using ModuleParamList = std::list<ModuleParam_sptr>;
using ModuleParamIter = ModuleParamList::iterator;
using ModuleParamConstIter = ModuleParamList::const_iterator;

class BasicModule
{
 public:
  BasicModule();
  virtual ~BasicModule();

  virtual ANLStatus mod_pre_initialize();

  std::string module_name() const;
  std::string module_version() const;
  
  void set_module_id(const std::string& v);
  std::string module_id() const;

  int copy_id() const;
  bool is_master() const;

  void set_order_sensitive(bool v);
  bool is_order_sensitive() const;

  void set_singleton(int copyID);
  void unset_singleton();
  bool is_singleton() const;
  int singleton_copy_id() const;
  
  std::vector<std::string> get_aliases_string() const;
  void add_alias(const std::string& name);

  std::string module_description() const;
  void set_module_description(const std::string& v);
  
  void on();
  void off();
  bool is_on();
  bool is_off();
  
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

  void clear_array(const std::string& name);

  void set_map_key(const std::string& key);
  void set_value_element(const std::string& name, int val);
  void set_value_element(const std::string& name, double val);
  void set_value_element(const std::string& name, const std::string& val);
  void insert_to_container();

  void print_parameters() const;

  %extend {
    void insert_to_map(const std::string& map_name, const std::string& key, PyObject* dict)
    {
      $self->expose_parameter(map_name);
      $self->set_map_key(key);

      if (PyDict_Check(dict)) {
        PyObject* keys = PyDict_Keys(dict);
        const int n = PyList_Size(keys);
        for (int i=0; i<n; i++) {
          PyObject* key = PyList_GetItem(keys, i);
          PyObject* val = PyDict_GetItem(dict, key);
          const std::string name(PyString_AsString(key));
          if (PyInt_Check(val)) {
            const int v = PyInt_AsLong(val);
            $self->set_value_element(name, v);
          }
          else if (PyInt_Check(val)) {
            const double v = PyFloat_AsDouble(val);
            $self->set_value_element(name, v);
          }
          else if (PyString_Check(val)) {
            const std::string v(PyString_AsString(val));
            $self->set_value_element(name, v);
          }
        }
      }

      $self->insert_to_container();
    }

    void push_to_vector(const std::string& vector_name, PyObject* dict)
    {
      $self->expose_parameter(vector_name);

      if (PyDict_Check(dict)) {
        PyObject* keys = PyDict_Keys(dict);
        const int n = PyList_Size(keys);
        for (int i=0; i<n; i++) {
          PyObject* key = PyList_GetItem(keys, i);
          PyObject* val = PyDict_GetItem(dict, key);
          const std::string name(PyString_AsString(key));
          if (PyInt_Check(val)) {
            const int v = PyInt_AsLong(val);
            $self->set_value_element(name, v);
          }
          else if (PyInt_Check(val)) {
            const double v = PyFloat_AsDouble(val);
            $self->set_value_element(name, v);
          }
          else if (PyString_Check(val)) {
            const std::string v(PyString_AsString(val));
            $self->set_value_element(name, v);
          }
        }
      }

      $self->insert_to_container();
    }
  }

  %exception;
};

class ANLManager
{
 public:
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

  void set_display_period(long int v);
  int display_period() const;
  
  void set_modules(std::vector<anlnext::BasicModule*> modules);

  virtual ANLStatus Define();
  virtual ANLStatus PreInitialize();
  virtual ANLStatus Initialize();
  virtual ANLStatus Analyze(long int num_events, bool enable_console=true);
  virtual ANLStatus Finalize();

  virtual int number_of_parallels() const;
  void set_print_parallel_modules(bool v=true);
  virtual BasicModule* access_to_module(int chainID,
                                        const std::string& moduleID);

  void parameters_to_json(const std::string& filename) const;

  virtual ANLStatus do_interactive_comunication();
  virtual ANLStatus do_interactive_analysis();

  %exception;
};

class ANLManagerMT : public ANLManager
{
public:
  explicit ANLManagerMT(int num_parallels=1);
  virtual ~ANLManagerMT();
};
 
} /* namespace anlnext */
