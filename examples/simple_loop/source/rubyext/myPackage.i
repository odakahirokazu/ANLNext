%module myPackage
%{
#include <anl/BasicModule.hh>

// include headers of my modules
#include "MyModule.hh"
#include "MyMapModule.hh"
#include "MyVectorModule.hh"
  
%}

%import(module="anlnext/ANL") "anl/ruby/ANL.i"

// interface to my modules

class MyModule : public anl::BasicModule {};
class MyMapModule : public anl::BasicModule {};
class MyVectorModule : public anl::BasicModule {};
