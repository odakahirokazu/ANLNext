%module submodule_python
%{
#include "SubmoduleExample.hh"
#include "MapExample.hh"
#include "RequirementExample.hh"
#include "UnsignedIntegerExample.hh"
#include "DefinitionStatusExample.hh"
%}
%import(module="anlnext.anlnextpy") "anlnextpy.i"
class SubmoduleExample : public anlnext::BasicModule {};

class MapExample : public anlnext::BasicModule {};

class UnsignedIntegerExample : public anlnext::BasicModule {
public:
  static uintmax_t maximum();
  static uintmax_t signed_maximum();
};

class RequirementExample : public anlnext::BasicModule {};

class DefinitionStatusExample : public anlnext::BasicModule {
public:
  explicit DefinitionStatusExample(anlnext::ANLStatus status);
  int pre_initialize_calls;
  int initialize_calls;
  int analyze_calls;
  int finalize_calls;
};
