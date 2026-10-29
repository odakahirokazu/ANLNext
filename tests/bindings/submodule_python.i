%module submodule_python
%{
#include "SubmoduleExample.hh"
%}
%import(module="anlnext.anlnextpy") "anlnextpy.i"
class SubmoduleExample : public anlnext::BasicModule {};
