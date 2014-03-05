/**
 * MyModule sample module of the ANL Next framework
 *
 * @author 
 * @date
 *
 */

#ifndef MyModule_H
#define MyModule_H 1

#include "BasicModule.hh"


class MyModule : public anl::BasicModule
{
  DEFINE_ANL_MODULE(MyModule, 1.0);
public:
  MyModule();
  ~MyModule();
  
  anl::ANLStatus mod_startup();
  anl::ANLStatus mod_init();
  anl::ANLStatus mod_his();
  anl::ANLStatus mod_bgnrun();
  anl::ANLStatus mod_ana();
  anl::ANLStatus mod_endrun();
  anl::ANLStatus mod_exit();

private:
  int myParameter;
};

#endif /* MyModule_H */
