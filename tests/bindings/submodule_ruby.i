%module submodule_ruby
%{
#include "SubmoduleExample.hh"
%}
%import(module="anlnext/ANL") "ANL.i"
class SubmoduleExample : public anlnext::BasicModule {};
