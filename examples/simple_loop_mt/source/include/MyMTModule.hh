/**
 * MyMTModule sample module of the ANL Next framework
 *
 * @author 
 * @date
 *
 */

#ifndef MyMTModule_H
#define MyMTModule_H 1

#include "BasicModule.hh"

class MyMTModule : public anl::BasicModule
{
  DEFINE_ANL_MODULE(MyMTModule, 1.0);
public:
  MyMTModule();
  virtual ~MyMTModule();

  std::unique_ptr<BasicModule> clone() override
  { return make_clone(new MyMTModule(*this)); }

protected:
  MyMTModule(const MyMTModule&) = default;

public:
  anl::ANLStatus mod_define() override;
  anl::ANLStatus mod_pre_initialize() override;
  anl::ANLStatus mod_initialize() override;
  anl::ANLStatus mod_begin_run() override;
  anl::ANLStatus mod_analyze() override;
  anl::ANLStatus mod_end_run() override;
  anl::ANLStatus mod_finalize() override;


private:
  int m_QuitIndex = -1;
  bool m_QuitAll = true;
};

#endif /* MyMTModule_H */
