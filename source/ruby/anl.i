%module anl
%{
#include "ANLManager.hh"
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

namespace boost
{
  template<class T> class shared_ptr {
  public:
    T *operator->();
  };
}

%template(SPtrModParam) boost::shared_ptr<anl::VModuleParameter>;
%template(SPtrModParamConst) boost::shared_ptr<anl::VModuleParameter const>;
%template(ListModParam) std::list<boost::shared_ptr<anl::VModuleParameter> >;

%include ANLStatus.hh

namespace anl
{

class ANLException
{
 public:
  ANLException();
  ~ANLException();
  const std::string print();
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
  
  std::string map_key_name() const;
  size_t num_map_value() const;
  boost::shared_ptr<VModuleParameter const> get_map_value(size_t i) const;

  void print(std::ostream& os) const;
  std::string value_string() const;
};

typedef boost::shared_ptr<VModuleParameter> ModParam;
typedef boost::shared_ptr<VModuleParameter const> ModParamConst;
typedef std::list<ModParam> ModParamList;
typedef ModParamList::iterator ModParamIter;
//typedef ModParamList::const_iterator ModParamConstIter;
typedef std::list<boost::shared_ptr<VModuleParameter> >::const_iterator ModParamConstIter;


class ANLManager
{
 public:
  ANLManager();
  ~ANLManager();
  
  %exception{
    try {
      $action
	}
    catch (const anl::ANLException& ex) {
      SWIG_exception(SWIG_RuntimeError, ex.print().c_str());
    }
  }

  void SetDisplayFrequency(int v);
  int DisplayFrequency() const;
  
  void SetModules(std::vector<anl::BasicModule*> modules);
  ANLStatus Startup();
  ANLStatus Initialize();
  ANLStatus Analyze(int num_event, bool thread_mode=true);
  ANLStatus Exit();
  ANLStatus Prepare();

  ANLStatus InteractiveCom();
  ANLStatus InteractiveAna();

  %exception;
};

 
class BasicModule
{
 public:
  BasicModule();
  BasicModule(const BasicModule& r);
  virtual ~BasicModule();

  virtual ANLStatus mod_prepare();

  std::string module_name() const;
  std::string module_version() const;
  
  void set_module_id(const std::string& v);
  std::string module_id() const;
  
  std::vector<std::string> get_alias() const;
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
    catch (const anl::ANLException& ex) {
      SWIG_exception(SWIG_RuntimeError, ex.print().c_str());
    }
  }

  void expose_parameter(const std::string& name);
  
  %rename(set_param) set_parameter(const std::string& name, bool val);
  %rename(set_param) set_parameter(const std::string& name, int val);
  %rename(set_param) set_parameter(const std::string& name, double val);
  %rename(set_param) set_parameter(const std::string& name, const std::string& val);
  %rename(set_ivec) set_parameter(const std::string&,
                                  const std::vector<int>&);
  %rename(set_fvec) set_parameter(const std::string&,
                                  const std::vector<double>&);
  %rename(set_svec) set_parameter(const std::string&,
                                  const std::vector<std::string>&);

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
  
  void clear_array(const std::string& name);
  
  void set_vector(const std::string& name, double x, double y);
  void set_vector(const std::string& name, double x, double y, double z);

  void set_map_key(const std::string& key);
  void set_map_value(const std::string& name, int val);
  void set_map_value(const std::string& name, double val);
  void set_map_value(const std::string& name, const std::string& val);
  void insert_map();

  %exception;

  void print_parameters();

  ModParamConstIter ModParamBegin() const;
  ModParamConstIter ModParamEnd() const;

  %extend {
    void param_map_insert(const std::string map_name, const std::string& key)
    {
      $self->expose_parameter(map_name);
      $self->set_map_key(key);
      if (rb_block_given_p()) {
        VALUE r = swig::from<anl::BasicModule*>(self);
        rb_yield(r);
      }
      $self->insert_map();
    }
  }
};

}
