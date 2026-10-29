%module submodule_python
%{
#include "SubmoduleExample.hh"
#include "ScalarMapExample.hh"
%}
%import(module="anlnext.anlnextpy") "anlnextpy.i"
class SubmoduleExample : public anlnext::BasicModule {};

class ScalarMapExample : public anlnext::BasicModule {};
