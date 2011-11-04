#ifndef ANLNext_impl_hh
#define ANLNext_impl_hh

#include "ANLNext.hh"
#include "ANLVModule.hh"
#include "ANLException.hh"

namespace anl
{

template<typename T>
ANLStatus ANLNext::routine_modfn(T func, const std::string& func_id)
{
  std::cout << "ANLNext: " << func_id << " routine started." << std::endl;

  ANLStatus status = AS_OK;
  AMIter const mod_end = m_Modules.end();
  for (AMIter mod = m_Modules.begin(); mod != mod_end; ++mod) {
    if ((*mod)->is_off()) continue;
    
    try {
      status = ((*mod)->*func)();
      if (status != AS_OK ) {
        std::cout << "ANLNext: " << func_id << " routine stopped.\n"
                  << (*mod)->module_name() << "::mod_" << func_id
                  << " return " << status << std::endl;
        break;
      }
    }
    catch (ANLException& ex) {
      ex << ANLErrModFnInfo( (*mod)->module_name() + "::mod_" + func_id );
      throw;
    }
  }
  return status;
}

}

#endif // ANLNext_impl_hh
