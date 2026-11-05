#ifndef ANLNEXT_TEST_SUBMODULE_EXAMPLE_H
#define ANLNEXT_TEST_SUBMODULE_EXAMPLE_H
#include <map>
#include <vector>
#include "BasicModule.hh"
#include "BasicSubModule.hh"

class ExampleAlgorithm : public anlnext::BasicSubModule {
  DEFINE_ANL_SUBMODULE(ExampleAlgorithm, 1.0);
public:
  void define_parameters() override;
  int calculate(int input, const anlnext::BasicModule* parent);
private:
  int factor_ = 2;
};

class ContainerAlgorithm : public ExampleAlgorithm {
  DEFINE_ANL_SUBMODULE(ContainerAlgorithm, 1.0);
public:
  void define_parameters() override;
private:
  std::vector<int> samples_;
  std::map<std::string, double> weights_;
  bool enabled_ = false;
  std::string label_;
};

class SubmoduleExample : public anlnext::BasicModule {
  DEFINE_ANL_MODULE(SubmoduleExample, 1.0);
  ENABLE_PARALLEL_RUN();
public:
  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_analyze() override;
  void set_submodule_by_key(const std::string& name, const std::string& key) override;
private:
  int input_ = 3;
  int result_ = 0;
  ExampleAlgorithm* algorithm_ = nullptr;
};
#endif
