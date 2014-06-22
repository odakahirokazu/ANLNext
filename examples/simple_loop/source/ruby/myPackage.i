%module myPackage
%{
#include "BasicModule.hh"

// include headers of my modules
#include "MyModule.hh"

%}

%import "ANL.i"


// interface to my modules

class MyModule : public anl::BasicModule
{
public:
  MyModule();
  ~MyModule();
};
