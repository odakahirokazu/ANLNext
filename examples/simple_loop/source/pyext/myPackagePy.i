%module myPackagePy
%{
#include <anlnext/BasicModule.hh>

// include headers of my modules
#include "MyModule.hh"
#include "MyMapModule.hh"
#include "MyVectorModule.hh"
  
%}

%import(module="anlnext.anlpy") "anlnext/python/anlpy.i"

// interface to my modules

class MyModule : public anlnext::BasicModule {};
class MyMapModule : public anlnext::BasicModule {};
class MyVectorModule : public anlnext::BasicModule {};
