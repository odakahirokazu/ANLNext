#include "MyModule.hh"

using namespace anl;

MyModule::MyModule()
  : myParameter1_(1), myParameter2_(2.0), myParameter3_("test")
{
}

MyModule::~MyModule() = default;

ANLStatus MyModule::mod_define()
{
  define_parameter("my_parameter1", &mod_class::myParameter1_);
  define_parameter("my_parameter2", &mod_class::myParameter2_);
  define_parameter("my_parameter3", &mod_class::myParameter3_);
  define_parameter("my_vector1", &mod_class::myVector1_);
  define_parameter("my_vector2", &mod_class::myVector2_);
  define_parameter("my_vector3", &mod_class::myVector3_);
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
