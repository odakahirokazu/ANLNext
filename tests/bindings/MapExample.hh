#ifndef ANLNEXT_TEST_MAP_EXAMPLE_H
#define ANLNEXT_TEST_MAP_EXAMPLE_H
#include "BasicModule.hh"

class MapExample : public anlnext::BasicModule {
  DEFINE_ANL_MODULE(MapExample, 1.0);
  ENABLE_PARALLEL_RUN();
public:
  anlnext::ANLStatus mod_define() override;
  std::map<int, int> map_i_i;
  std::map<int, double> map_i_d;
  std::map<int, std::string> map_i_str;
  std::map<std::string, int> map_str_i;
  std::map<std::string, double> map_str_d;
  std::map<std::string, std::string> map_str_str;
  std::map<std::string, std::tuple<int, double>> table;
  std::vector<std::tuple<int>> rows;
};
#endif
