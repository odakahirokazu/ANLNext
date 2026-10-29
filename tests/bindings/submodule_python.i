%module submodule_python
%{
#include "SubmoduleExample.hh"
#include "MapExample.hh"
%}
%import(module="anlnext.anlnextpy") "anlnextpy.i"
class SubmoduleExample : public anlnext::BasicModule {};

class MapExample : public anlnext::BasicModule {};
