#ifndef ANLNEXT_TEST_REQUIREMENT_EXAMPLE_H
#define ANLNEXT_TEST_REQUIREMENT_EXAMPLE_H
#include "BasicModule.hh"

class RequirementExample : public anlnext::BasicModule {
  DEFINE_ANL_MODULE(RequirementExample, 1.0);
  ENABLE_PARALLEL_RUN();
public:
  anlnext::ANLStatus mod_define() override;
};
#endif
