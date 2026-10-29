%module submodule_ruby
%{
#include "SubmoduleExample.hh"
#include "MapExample.hh"
%}
%import(module="anlnext/ANL") "ANL.i"
class SubmoduleExample : public anlnext::BasicModule {};

class MapExample : public anlnext::BasicModule {};
