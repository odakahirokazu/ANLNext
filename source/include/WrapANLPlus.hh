#ifndef WrapANLPlus_hh
#define WrapANLPlus_hh

#include "ANLVModule.hh"

namespace anl
{

template <typename T>
class WrapANLPlus : public ANLVModule
{
public:
  explicit WrapANLPlus(T* amod);
  ~WrapANLPlus() { delete module; }

  ANLStatus mod_startup();
  ANLStatus mod_com();
  ANLStatus mod_prepare() { return AS_OK; }
  ANLStatus mod_init();
  ANLStatus mod_his();
  ANLStatus mod_bgnrun();
  ANLStatus mod_ana();
  ANLStatus mod_endrun();

  T* GetPointer() { return module; }
private:
  T* module;
};


template <typename T>
ANLStatus WrapANLPuls::WrapANLPuls(T* amod)
  : ANLVModule(amod->module_name, amod->module_version)
{
  module = amod;  
}


template <typename T>
inline ANLStatus WrapANLPuls::mod_startup()
{
  int s(0);
  module->mod_startup(s);
  ANLStatus status(s);
  return status;
}


template <typename T>
inline ANLStatus WrapANLPuls::mod_com()
{
  int s(0);
  module->mod_com(s);
  ANLStatus status(s);
  return status;
}


template <typename T>
inline ANLStatus WrapANLPuls::mod_init()
{
  int s(0);
  module->mod_init(s);
  ANLStatus status(s);
  return status;
}


template <typename T>
inline ANLStatus WrapANLPuls::mod_his()
{
  int s(0);
  module->mod_his(s);
  ANLStatus status(s);
  return status;
}


template <typename T>
inline ANLStatus WrapANLPuls::mod_bgnrun()
{
  int s(0);
  module->mod_bgnrun(s);
  ANLStatus status(s);
  return status;
}


template <typename T>
inline ANLStatus WrapANLPuls::mod_ana()
{
  int s(0);
  module->mod_ana(s);
  ANLStatus status(s);
  return status;
}


template <typename T>
inline ANLStatus WrapANLPuls::mod_endrun()
{
  int s(0);
  module->mod_endrun(s);
  ANLStatus status(s);
  return status;
}


template <typename T>
inline ANLStatus WrapANLPuls::mod_exit()
{
  int s(0);
  module->mod_exit(s);
  ANLStatus status(s);
  return status;
}

}

#endif // WrapANLPlus_hh
