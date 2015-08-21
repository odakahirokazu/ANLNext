%module myPackage
%{
#include "BasicModule.hh"

// include headers of my modules
#include "MyModule.hh"
#include "MyMapModule.hh"
#include "MyVectorModule.hh"
  
%}

%import "ANL.i"


// interface to my modules

class MyModule : public anl::BasicModule
{
public:
  MyModule();
  ~MyModule();
};

class MyMapModule : public anl::BasicModule
{
public:
  MyMapModule();
  ~MyMapModule();
};

class MyVectorModule : public anl::BasicModule
{
public:
  MyVectorModule();
  ~MyVectorModule();
};
