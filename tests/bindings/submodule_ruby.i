%module submodule_ruby
%{
#include "SubmoduleExample.hh"
#include "MapExample.hh"
#include "RequirementExample.hh"
#include "UnsignedIntegerExample.hh"
#include "DefinitionStatusExample.hh"
%}
%import(module="anlnext/ANL") "ANL.i"
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
};
