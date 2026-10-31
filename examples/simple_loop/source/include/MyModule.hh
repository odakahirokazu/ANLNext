/**
 * MyModule sample module of the ANL Next framework
 *
 * @author 
 * @date
 *
 */

#ifndef MyModule_H
#define MyModule_H 1

#include <anlnext/BasicModule.hh>
#include <cstdint>

class MyModule : public anlnext::BasicModule
{
  DEFINE_ANL_MODULE(MyModule, 1.0);
public:
  MyModule();
  virtual ~MyModule();
  
  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_pre_initialize() override;
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_begin_run() override;
  anlnext::ANLStatus mod_analyze() override;
  anlnext::ANLStatus mod_end_run() override;
  anlnext::ANLStatus mod_finalize() override;

private:
  int my_parameter1_;
  double my_parameter2_;
  std::string my_parameter3_;
  std::vector<int> my_vector1_;
  std::vector<double> my_vector2_;
  std::vector<std::string> my_vector3_;

  int16_t my_parameter11_;
  int32_t my_parameter12_;
  int64_t my_parameter13_;

  bool my_parameter21_;
};

#endif /* MyModule_H */
