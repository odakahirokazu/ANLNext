#ifndef ANLNEXT_TEST_DEFINITION_STATUS_EXAMPLE_H
#define ANLNEXT_TEST_DEFINITION_STATUS_EXAMPLE_H
#include "BasicModule.hh"

class DefinitionStatusExample : public anlnext::BasicModule {
  DEFINE_ANL_MODULE(DefinitionStatusExample, 1.0);
  ENABLE_PARALLEL_RUN();
public:
  explicit DefinitionStatusExample(anlnext::ANLStatus status);
  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_pre_initialize() override;
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_analyze() override;
  anlnext::ANLStatus mod_finalize() override;

  int pre_initialize_calls = 0;
  int initialize_calls = 0;
  int analyze_calls = 0;
  int finalize_calls = 0;
private:
  anlnext::ANLStatus define_status_;
};
#endif
