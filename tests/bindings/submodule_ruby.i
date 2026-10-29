%module submodule_ruby
%{
#include "SubmoduleExample.hh"
#include "ScalarMapExample.hh"
%}
%import(module="anlnext/ANL") "ANL.i"
class SubmoduleExample : public anlnext::BasicModule {};

class ScalarMapExample : public anlnext::BasicModule {};
