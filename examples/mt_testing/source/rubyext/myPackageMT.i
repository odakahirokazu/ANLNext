%module myPackageMT
%{
#include "BasicModule.hh"

// include headers of my modules
#include "MyMTModule.hh"
  
%}

%import "ANL.i"


// interface to my modules

class MyMTModule : public anl::BasicModule
{
public:
  MyMTModule();
  ~MyMTModule();
};
