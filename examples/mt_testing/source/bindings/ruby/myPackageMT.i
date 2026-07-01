%module myPackageMT
%{
#include <anlnext/BasicModule.hh>

// include headers of my modules
#include "MyMTModule.hh"
  
%}

%import(module="anlnext/ANL") "anlnext/ruby/ANL.i"


// interface to my modules

class MyMTModule : public anlnext::BasicModule {};
