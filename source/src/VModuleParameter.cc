#include "VModuleParameter.hh"
#include "ANLException.hh"

#include <sstream>

namespace anl {

VModuleParameter::VModuleParameter(const std::string& name)
  : _name(name),
    _unit(1.0), _unitname(""),
    _expr(""), _question(""), _default_string(""),
    _hidden(false),
    _description("")
{
}


VModuleParameter::VModuleParameter(const std::string& name,
                                   double unit, const std::string& unit_name)
  : _name(name),
    _unit(unit), _unitname(unit_name),
    _expr(""), _question(""), _default_string(""),
    _hidden(false)
{
}


VModuleParameter::VModuleParameter(const std::string& name,
                                   const std::string& expression)
  : _name(name),
    _unit(1.0), _unitname(""),
    _expr(expression), _question(""), _default_string(""),
    _hidden(false)
{
}


VModuleParameter::VModuleParameter(const std::string& name,
                                   const std::string& expression,
                                   const std::string& default_string)
  : _name(name),
    _unit(1.0), _unitname(""),
    _expr(expression), _question(""), _default_string(default_string),
    _hidden(false)
{
}


void VModuleParameter::print(std::ostream& os) const
{
  os << name() << " "
     << ((unit_name()!="") ? "[" : "")
     << unit_name()
     << ((unit_name()!="") ? "] " : "")
     << ": " << *this;
}


std::string VModuleParameter::value_string() const
{
  std::ostringstream oss;
  oss << *this;
  return oss.str();
}


void VModuleParameter::ask_base_out(std::ostream& ost)
{
  if (question()=="") {
    ost << name() << " ";
  }
  else {
    ost << question() << " ";
  }
  
  ost << ((unit_name()!="") ? "[" : "")
      << unit_name()
      << ((unit_name()!="") ? "] " : "")
      << "? " << *this;

  ost << special_message_to_ask();
  ost << " : ";
  ost.flush();
}


bool VModuleParameter::ask_base_in(std::istream& ist)
{  
  char c;
  ist.get(c);
  if (c=='\n') return false;
  ist.putback(c);
  ist >> *this;
  ist.ignore(INT_MAX, '\n');
  return true;
}


bool VModuleParameter::ask_base()
{
#if ANLNEXT_USE_READLINE
  std::ostringstream ost;
  ask_base_out(ost);
  
  char* line;
  line = readline(ost.str().c_str());
  if (strlen(line)>0) {
  add_history(line);
  std::istringstream ist(line);
    free(line);
    line = 0;
    ist >> *this;
    return true;
  }
  else {
    if (line) free(line);
    line = 0;
    return false;
  }
#else
  ask_base_out(std::cout);
  return ask_base_in(std::cin);
#endif
}


std::string VModuleParameter::special_message_to_ask()
{
  std::string message("");
  return message;
}


void VModuleParameter::throw_type_match_exception(const std::string& message)
{
  BOOST_THROW_EXCEPTION( ANLException() <<
                         ANLErrInfo(std::string("type does not match: ")
                                    + name() + " [" + message + "]") );
}

}
