#include "RequirementExample.hh"
using namespace anlnext;

ANLStatus RequirementExample::mod_define()
{
  require_module("producer", ModulePosition::before);
  require_module(module_id());
  return AS_OK;
}
