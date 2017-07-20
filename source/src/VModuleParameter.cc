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
#include <limits>
#include <boost/format.hpp>
#include "ANLException.hh"

#if ANL_USE_READLINE
#include "CLIUtility.hh"
#endif /* ANL_USE_READLINE */

namespace {

void trim_right(std::string& line)
{
  std::string::iterator it =
    std::find_if(line.rbegin(), line.rend(), [](int c) {
        return !std::isspace(c);
      }).base();
  line.erase(it, line.end());
}

} /* anonynous namespace */

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

void VModuleParameter::set_value_integer(intmax_t v)
{
  std::ostringstream oss;
  oss << v << " " << "[intmax_t]";
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

intmax_t VModuleParameter::get_value_integer() const
{
  throw_type_match_exception("intmax_t");
  return 0;
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

void VModuleParameter::ask_base_out(std::ostream& os)
{
  if (question()=="") {
    os << name() << " ";
  }
  else {
    os << question() << " ";
  }
  
  os << ((unit_name()!="") ? "[" : "")
     << unit_name()
     << ((unit_name()!="") ? "] " : "")
     << "? " << *this;

  os << special_message_to_ask();
  os << " : ";
  os.flush();
}

bool VModuleParameter::ask_base_in(std::istream& is)
{
  std::string line;
  std::getline(is, line);
  trim_right(line);
  if (line.size() == 0) { return false; }

  std::istringstream iss(line);
  if (line==";") { std::string tmp; iss >> tmp; }
  iss >> *this;
  if (!iss) {
    const std::string message
      = (boost::format("Input error: %s [%s]") % name() % type_name()).str();
    BOOST_THROW_EXCEPTION( ANLException(message) );
  }
  return true;
}

#if ANL_USE_READLINE
bool VModuleParameter::ask_base()
{
  std::ostringstream os;
  ask_base_out(os);

  ReadLine reader;
  const int count = reader.read(os.str().c_str());
  if (count == -1) { return false; }
  if (count == 0) { return false; }
  std::string line = reader.str();
  std::istringstream iss(line);
  if (line==";") { std::string tmp; iss >> tmp; }
  iss >> *this;
  if (!iss) {
    const std::string message
      = (boost::format("Input error: %s [%s]") % name() % type_name()).str();
    BOOST_THROW_EXCEPTION( ANLException(message) );
  }
  return true;
}
#else /* ANL_USE_READLINE */
bool VModuleParameter::ask_base()
{
  ask_base_out(std::cout);
  return ask_base_in(std::cin);
}
#endif /* ANL_USE_READLINE */

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
