#include "DefinitionStatusExample.hh"
using namespace anlnext;

DefinitionStatusExample::DefinitionStatusExample(ANLStatus status)
  : define_status_(status)
{}

ANLStatus DefinitionStatusExample::mod_define()
{ return define_status_; }

ANLStatus DefinitionStatusExample::mod_pre_initialize()
{
  ++pre_initialize_calls;
  return AS_OK;
}

ANLStatus DefinitionStatusExample::mod_initialize()
{
  ++initialize_calls;
  return AS_OK;
}

ANLStatus DefinitionStatusExample::mod_analyze()
{
  ++analyze_calls;
  return AS_OK;
}

ANLStatus DefinitionStatusExample::mod_finalize()
{
  ++finalize_calls;
  return AS_OK;
}
