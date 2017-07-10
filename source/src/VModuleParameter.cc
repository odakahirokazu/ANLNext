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

#include "VModuleParameter.hh"
#include <sstream>
#include <climits>
#include <boost/format.hpp>
#include "ANLException.hh"

namespace anl {

VModuleParameter::VModuleParameter(const std::string& name)
  : name_(name),
    hidden_(false),
    unit_(1.0), unit_name_(""),
    expr_(""), question_(""), default_string_(""),
    description_("")
{
}

VModuleParameter::~VModuleParameter() = default;

void VModuleParameter::set_value(bool v)
{
  std::ostringstream oss;
  oss << v << " " << "[bool]";
  throw_type_match_exception(oss.str());
}

void VModuleParameter::set_value(int v)
{
  std::ostringstream oss;
  oss << v << " " << "[int]";
  throw_type_match_exception(oss.str());
}

void VModuleParameter::set_value(double v)
{
  std::ostringstream oss;
  oss << v << " " << "[double]";
  throw_type_match_exception(oss.str());
}

void VModuleParameter::set_value(const std::string& v)
{
  std::ostringstream oss;
  oss << v << " " << "[string]";
  throw_type_match_exception(oss.str());
}

void VModuleParameter::set_value(const std::vector<int>& v)
{
  std::ostringstream oss;
  oss << "vector(" << v.size() << ") " << "[vector<int>]";
  throw_type_match_exception(oss.str());
}

void VModuleParameter::set_value(const std::vector<double>& v)
{
  std::ostringstream oss;
  oss << "vector(" << v.size() << ") " << "[vector<double>]";
  throw_type_match_exception(oss.str());
}

void VModuleParameter::set_value(const std::vector<std::string>& v)
{
  std::ostringstream oss;
  oss << "vector(" << v.size() << ") " << "[vector<string>]";
  throw_type_match_exception(oss.str());
}

void VModuleParameter::set_value(const std::list<std::string>& v)
{
  std::ostringstream oss;
  oss << "list: " << v.front() << " ... "<< "[list<string>]";
  throw_type_match_exception(oss.str());
}

void VModuleParameter::set_value(double x, double y)
{
  std::ostringstream oss;
  oss << "( " << x << " " << y << " ) " << "[2-vector]";
  throw_type_match_exception(oss.str());
}

void VModuleParameter::set_value(double x, double y, double z)
{
  std::ostringstream oss;
  oss << "( " << x << " " << y << " " << z << " ) " << "[3-vector]";
  throw_type_match_exception(oss.str());
}

bool VModuleParameter::get_value(bool) const
{
  throw_type_match_exception("bool");
  return false;
}

int VModuleParameter::get_value(int) const
{
  throw_type_match_exception("int");
  return 0;
}

double VModuleParameter::get_value(double) const
{
  throw_type_match_exception("double");
  return 0.0;
}

std::string VModuleParameter::get_value(const std::string&) const
{
  throw_type_match_exception("string");
  return "";
}

std::vector<int> VModuleParameter::get_value(const std::vector<int>&) const
{
  throw_type_match_exception("vector<int>");
  return {};
}

std::vector<double> VModuleParameter::get_value(const std::vector<double>&) const
{
  throw_type_match_exception("vector<double>");
  return {};
}

std::vector<std::string> VModuleParameter::get_value(const std::vector<std::string>&) const
{
  throw_type_match_exception("vector<string>");
  return {};
}

std::list<std::string> VModuleParameter::get_value(const std::list<std::string>&) const
{
  throw_type_match_exception("list<string>");
  return {};
}

std::vector<double> VModuleParameter::get_value(double, double) const
{
  throw_type_match_exception("2-vector");
  return {};
}

std::vector<double> VModuleParameter::get_value(double, double, double) const
{
  throw_type_match_exception("3-vector");
  return {};
}

void VModuleParameter::print(std::ostream& os) const
{
  os << name() << ": " << *this
     << ((unit_name()!="") ? " [" : "")
     << unit_name()
     << ((unit_name()!="") ? "]" : "");
  if (hidden_) { os << " (hidden)"; }
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
#if ANL_USE_READLINE
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
#else /* ANL_USE_READLINE */
  ask_base_out(std::cout);
  return ask_base_in(std::cin);
#endif /* ANL_USE_READLINE */
}

std::string VModuleParameter::special_message_to_ask() const
{
  std::string message("");
  return message;
}

void VModuleParameter::throw_type_match_exception(const std::string& message) const
{
  const std::string message2
    = (boost::format("Type does not match: %s [%s] <= %s") % name() % type_name() % message).str();
  BOOST_THROW_EXCEPTION( ANLException(message2) );
}

} /* namespace anl */
