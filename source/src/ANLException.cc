#include "ANLException.hh"
#include "ANLVModule.hh"

using namespace anl;

ANLException::ANLException(const ANLVModule* mod)
{
  *this << ANLErrInfo(std::string("ANL Module: ") + mod->module_name());
}


ANLException::ANLException(const std::string& msg)
{
  *this << ANLErrInfo(msg);
}


void ANLException::setModule(const ANLVModule* mod)
{
  *this << ANLErrInfo(std::string("ANL Module: ") + mod->module_name());
}
