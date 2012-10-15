%module myPackage
%{
#include "BasicModule.hh"

// include headers of my modules
#include "MyModule.hh"

%}

%include "std_vector.i"
%include "anl.i"


// interface to my modules

class MyModule : public anl::BasicModule
{
public:
  MyModule();
  ~MyModule();
};
