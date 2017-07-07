%module ANL
%{
#include "ANLManager.hh"
#include "ANLManagerMT.hh"
#include "VModuleParameter.hh"
#include "BasicModule.hh"
#include "ANLException.hh"
%}

%include "exception.i"
%include "std_string.i"
%include "std_vector.i"
%include "std_list.i"

%template(ModuleVector) std::vector<anl::BasicModule*>;

%template(VectorI) std::vector<int>;
%template(VectorD) std::vector<double>;
%template(VectorStr) std::vector<std::string>;

%template(ListI) std::list<int>;

namespace std
{
  template<class T> class shared_ptr {
  public:
    T *operator->();
  };
}

%template(SPtrModParam) std::shared_ptr<anl::VModuleParameter>;
%template(SPtrModParamConst) std::shared_ptr<anl::VModuleParameter const>;
%template(ListModParam) std::list<std::shared_ptr<anl::VModuleParameter> >;

namespace anl
{

enum class ANLStatus { AS_OK, AS_SKIP, AS_SKIP_ERR, AS_QUIT, AS_QUIT_ERR };


struct ANLException
{
  static void setVerboseLevel(int v);
  static int VerboseLevel();
  ANLException();
  ~ANLException();
  const std::string toString();
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
    
  void print(std::ostream& os) const;
  std::string value_string() const;
};

typedef std::shared_ptr<VModuleParameter> ModuleParam_sptr;
typedef std::list<ModuleParam> ModuleParamList;
typedef ModuleParamList::iterator ModuleParamIter;
typedef std::list<std::shared_ptr<VModuleParameter> >::const_iterator ModuleParamConstIter;


class ANLManager
{
 public:
  ANLManager();
  virtual ~ANLManager();
  
  %exception{
    try {
      $action
    }
    catch (const anl::ANLException& ex) {
      SWIG_exception(SWIG_RuntimeError, ex.toString().c_str());
    }
  }

  void SetDisplayFrequency(long int v);
  int DisplayFrequency() const;
  
  void SetModules(std::vector<anl::BasicModule*> modules);
  virtual ANLStatus Define();
  virtual ANLStatus PreInitialize();
  virtual ANLStatus Initialize();
  virtual ANLStatus Analyze(long int num_events, bool thread_mode=true);
  virtual ANLStatus Finalize();

  void parameters_to_json(const std::string& filename) const;

  virtual ANLStatus InteractiveComunication();
  virtual ANLStatus InteractiveAnalysis();

  %exception;
};

class ANLManagerMT : public ANLManager
{
public:
  explicit ANLManagerMT(int num_parallels=1);
  virtual ~ANLManagerMT();
};
 
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
    catch (anl::ANLException& ex) {
      SWIG_exception(SWIG_RuntimeError, ex.toString().c_str());
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

  void clear_array(const std::string& name);

  void set_map_key(const std::string& key);
  void set_value_element(const std::string& name, int val);
  void set_value_element(const std::string& name, double val);
  void set_value_element(const std::string& name, const std::string& val);
  void insert_to_container();

  %exception;

  void print_parameters();

  %extend {
    void insert_to_map(const std::string& map_name, const std::string& key)
    {
      $self->expose_parameter(map_name);
      $self->set_map_key(key);
      if (rb_block_given_p()) {
        VALUE r = swig::from<anl::BasicModule*>(self);
        rb_yield(r);
      }
      $self->insert_to_container();
    }

    void push_to_vector(const std::string& vector_name)
    {
      $self->expose_parameter(vector_name);
      if (rb_block_given_p()) {
        VALUE r = swig::from<anl::BasicModule*>(self);
        rb_yield(r);
      }
      $self->insert_to_container();
    }
  }
};

} /* namespace anl */
