/**
 * MyMapModule sample module of the ANL Next framework
 *
 * @author 
 * @date
 *
 */

#ifndef MyMapModule_H
#define MyMapModule_H 1

#include <anlnext/BasicModule.hh>
#include <map>
#include <tuple>

class MyMapModule : public anlnext::BasicModule
{
  DEFINE_ANL_MODULE(MyMapModule, 1.0);
public:
  MyMapModule();
  virtual ~MyMapModule();
  
  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_analyze() override;

private:
  std::map<std::string, std::tuple<int, std::string, double, double>> my_map_;
};

#endif /* MyMapModule_H */
