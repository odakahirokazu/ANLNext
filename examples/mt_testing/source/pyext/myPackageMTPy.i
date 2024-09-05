%module myPackageMTPy
%{
#include <anlnext/BasicModule.hh>

// include headers of my modules
#include "MyMTModule.hh"
  
%}

%import(module="anlnext.anlnextpy") "anlnext/python/anlnextpy.i"


// interface to my modules

class MyMTModule : public anlnext::BasicModule {};
