/**
 * MyModule sample module of the ANL Next framework
 *
 * @author 
 * @date
 *
 */

#ifndef MyModule_hh
#define MyModule_hh

#include "ANLVModule.hh"

class MyModule : public anl::ANLVModule
{
public:
  MyModule(const std::string& module_name="MyModule",
           const std::string& module_version="1.0");
  ~MyModule();
  
  anl::ANLStatus mod_startup();
  anl::ANLStatus mod_init();
  anl::ANLStatus mod_his();
  anl::ANLStatus mod_bgnrun();
  anl::ANLStatus mod_ana();
  anl::ANLStatus mod_endrun();
  anl::ANLStatus mod_exit();

private:
};

#endif
