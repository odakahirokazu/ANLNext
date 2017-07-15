/**
 * MyMapModule sample module of the ANL Next framework
 *
 * @author 
 * @date
 *
 */

#ifndef MyMapModule_H
#define MyMapModule_H 1

#include <anl/BasicModule.hh>
#include <map>
#include <tuple>

class MyMapModule : public anl::BasicModule
{
  DEFINE_ANL_MODULE(MyMapModule, 1.0);
public:
  MyMapModule();
  virtual ~MyMapModule();
  
  anl::ANLStatus mod_define() override;
  anl::ANLStatus mod_analyze() override;

private:
  std::map<std::string, std::tuple<int, std::string, double, double>> myMap_;
  int bufferID_ = 0;
  std::string bufferType_ = "pixel";
  double bufferX_ = 0.0;
  double bufferY_ = 0.0;
};

#endif /* MyMapModule_H */
