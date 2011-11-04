// Hirokazu Odaka

#ifndef ANLException_hh
#define ANLException_hh

#include <stdexcept>
#include <string>
#include <boost/exception/all.hpp>

namespace anl
{

typedef boost::error_info<struct tag_ANLErr, std::string> ANLErrInfo;
typedef boost::error_info<struct tag_ANLModFn, std::string> ANLErrModFnInfo;
typedef boost::error_info<struct tag_ANLEventID, int> ANLErrEventIDInfo;

class ANLVModule;

struct ANLException : boost::exception, std::exception
{
  ANLException() {}
  explicit ANLException(const ANLVModule* mod);
  explicit ANLException(const std::string& msg);

  void setModule(const ANLVModule* mod);
  const std::string print() const { return diagnostic_information(*this); }
};

}

#endif // ANLException_hh
