%module submoduleExamplePy
%{
#include "AlgorithmModule.hh"
%}

%import(module="anlnext.anlnextpy") "anlnextpy.i"

class AlgorithmModule : public anlnext::BasicModule {};
