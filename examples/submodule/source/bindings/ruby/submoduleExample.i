%module submoduleExample
%{
#include "AlgorithmModule.hh"
%}

%import(module="anlnext/ANL") "ANL.i"

class AlgorithmModule : public anlnext::BasicModule {};
