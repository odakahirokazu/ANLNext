/**
 * MyMTModule sample module of the ANL Next framework
 *
 * @author 
 * @date
 *
 */

#ifndef MyMTModule_H
#define MyMTModule_H 1

#include <anlnext/BasicModule.hh>

class MyMTModule : public anlnext::BasicModule
{
  DEFINE_ANL_MODULE(MyMTModule, 1.0);
  ENABLE_PARALLEL_RUN();
public:
  MyMTModule();
  virtual ~MyMTModule();

protected:
  MyMTModule(const MyMTModule& r) = default;

public:
  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_pre_initialize() override;
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_begin_run() override;
  anlnext::ANLStatus mod_analyze() override;
  anlnext::ANLStatus mod_end_run() override;
  anlnext::ANLStatus mod_finalize() override;

private:
  int m_QuitIndex = -1;
  bool m_QuitAll = true;
};

#endif /* MyMTModule_H */
