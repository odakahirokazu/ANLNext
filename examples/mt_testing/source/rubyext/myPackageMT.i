%module myPackageMT
%{
#include <anl/BasicModule.hh>

// include headers of my modules
#include "MyMTModule.hh"
  
%}

%import "anl/ruby/ANL.i"


// interface to my modules

class MyMTModule : public anl::BasicModule
{
public:
  MyMTModule();
  ~MyMTModule();
};
