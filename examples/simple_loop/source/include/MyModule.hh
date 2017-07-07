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
  virtual ~MyModule();
  
  anl::ANLStatus mod_define() override;
  anl::ANLStatus mod_pre_initialize() override;
  anl::ANLStatus mod_initialize() override;
  anl::ANLStatus mod_begin_run() override;
  anl::ANLStatus mod_analyze() override;
  anl::ANLStatus mod_end_run() override;
  anl::ANLStatus mod_finalize() override;

private:
  int myParameter1_;
  double myParameter2_;
  std::string myParameter3_;
  std::vector<int> myVector1_;
  std::vector<double> myVector2_;
  std::vector<std::string> myVector3_;
};

#endif /* MyModule_H */
