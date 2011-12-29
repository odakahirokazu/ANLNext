#ifndef VModuleParameter_hh
#define VModuleParameter_hh

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <boost/shared_ptr.hpp>

#define ANLNEXT_USE_READLINE 1
#if ANLNEXT_USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

namespace anl
{

/**
 * A virtual class for an ANL module parameter.
 * @author Hirokazu Odaka
 * @date June, 2010
 * @date 2011-07-12
 * @date 2011-12-28
 */

class VModuleParameter
{
public:
  VModuleParameter(const std::string& name);
  VModuleParameter(const std::string& name,
                   double unit, const std::string& unit_name);  
  VModuleParameter(const std::string& name, const std::string& expression);
  VModuleParameter(const std::string& name, const std::string& expression,
                   const std::string& default_string);

  virtual ~VModuleParameter() {}
  
  std::string name() const { return _name; }
  double unit() const { return _unit; }
  std::string unit_name() const { return _unitname; }

  std::string expression() const { return _expr; }
  std::string question() const { return _question; }
  std::string default_string() const { return _default_string; }

  void set_unit(double unit, const std::string& unitname)
  { _unit = unit; _unitname = unitname; }
  
  void set_expression(const std::string& v) { _expr = v; }
  void set_question(const std::string& v) { _question = v; }
  void set_default_string(const std::string& v) { _default_string = v; }

  void set_hidden(bool v=true) { _hidden = v; }
  void set_exposed() { _hidden = false; }
  bool is_hidden() { return _hidden; }

  void set_description(const std::string& v) { _description = v; }
  std::string description() const { return _description; }

  virtual std::string type_name() const { return ""; }
  
  virtual bool ask() { return ask_base(); }
  
  virtual void set_value(int v);
  virtual void set_value(double v);
  virtual void set_value(const std::string& v);
  virtual void set_value(const std::vector<int>& v);
  virtual void set_value(const std::vector<double>& v);
  virtual void set_value(const std::vector<std::string>& v);
  virtual void set_value(double x, double y);
  virtual void set_value(double x, double y, double z);

  virtual void clear_array() {}

  virtual void output(std::ostream& ) const {}
  virtual void input(std::istream& ) {}

  virtual void get(void* const /* value_ptr */) const {}
  virtual void set(const void* const /* value_ptr */) {}

  virtual std::string map_key_name() const { return ""; }
  virtual void set_map_key(const std::string& /* key */) {}
  virtual size_t num_map_value() const { return 0; }
  virtual boost::shared_ptr<VModuleParameter const> get_map_value(size_t /* i */) const
  { return boost::shared_ptr<VModuleParameter>(); }
  virtual void add_map_value(boost::shared_ptr<VModuleParameter> /* param */) {}
  virtual void enable_map_value(int /* type */, const std::vector<int>& /* enables */) {}
  virtual void set_map_value(const std::string& /* name */, int /* val */) {}
  virtual void set_map_value(const std::string& /* name */, double /* val */) {}
  virtual void set_map_value(const std::string& /* name */, const std::string& /* val */) {}
  virtual void insert_map() {}
  
  void print(std::ostream& os) const;
  std::string value_string() const;
  
protected:
  virtual bool ask_base();
  virtual void ask_base_out(std::ostream& ost);
  virtual bool ask_base_in(std::istream& ist);
  std::string special_message_to_ask();
  
  void throw_type_match_exception(const std::string& message="");
  
private:
  std::string _name;
  double _unit;
  std::string _unitname;
  std::string _expr;
  std::string _question;
  std::string _default_string;
  bool _hidden;
  std::string _description;
};

typedef boost::shared_ptr<VModuleParameter> ModParam;
typedef boost::shared_ptr<VModuleParameter const> ModParamConst;
typedef std::list<ModParam> ModParamList;
typedef ModParamList::iterator ModParamIter;
typedef ModParamList::const_iterator ModParamConstIter;
}

inline
std::ostream& operator<<(std::ostream& os, const anl::VModuleParameter& p)
{
  p.output(os);
  return os;
}


inline
std::istream& operator>>(std::istream& is, anl::VModuleParameter& p)
{
  p.input(is);
  return is;
}

#endif // VModuleParameter_hh
