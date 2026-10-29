#ifndef ANLNEXT_TEST_UNSIGNED_INTEGER_EXAMPLE_H
#define ANLNEXT_TEST_UNSIGNED_INTEGER_EXAMPLE_H
#include "BasicModule.hh"

class UnsignedIntegerExample : public anlnext::BasicModule {
  DEFINE_ANL_MODULE(UnsignedIntegerExample, 1.0);
  ENABLE_PARALLEL_RUN();
public:
  anlnext::ANLStatus mod_define() override;
  static uintmax_t maximum();
  static uintmax_t signed_maximum();
  uintmax_t address = 0;
  int count = 0;
  intmax_t signed_value = 0;
};
#endif
