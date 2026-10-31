#include "MyModule.hh"

using namespace anlnext;

MyModule::MyModule()
  : my_parameter1_(1), my_parameter2_(2.0), my_parameter3_("test"),
    my_parameter11_(200), my_parameter12_(20000), my_parameter13_(5000000000L),
    my_parameter21_(true)
{
}

MyModule::~MyModule() = default;

ANLStatus MyModule::mod_define()
{
  define_parameter("my_parameter1", &mod_class::my_parameter1_);
  define_parameter("my_parameter2", &mod_class::my_parameter2_);
  define_parameter("my_parameter3", &mod_class::my_parameter3_);
  define_parameter("my_vector1", &mod_class::my_vector1_);
  define_parameter("my_vector2", &mod_class::my_vector2_);
  define_parameter("my_vector3", &mod_class::my_vector3_);

  define_parameter("my_parameter11", &mod_class::my_parameter11_);
  define_parameter("my_parameter12", &mod_class::my_parameter12_);
  define_parameter("my_parameter13", &mod_class::my_parameter13_);

  define_parameter("my_parameter21", &mod_class::my_parameter21_);

  return AS_OK;
}

ANLStatus MyModule::mod_pre_initialize()
{
  return AS_OK;
}

ANLStatus MyModule::mod_initialize()
{
  return AS_OK;
}

ANLStatus MyModule::mod_begin_run()
{
  return AS_OK;
}

ANLStatus MyModule::mod_analyze()
{
  return AS_OK;
}

ANLStatus MyModule::mod_end_run()
{
  return AS_OK;
}

ANLStatus MyModule::mod_finalize()
{
  return AS_OK;
}
