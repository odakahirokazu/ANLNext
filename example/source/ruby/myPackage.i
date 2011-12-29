%module myPackage
%{
#include "ANLVModule.hh"

// include headers of my modules
#include "MyModule.hh"

%}

%include "std_vector.i"
%include "anl.i"


// interface to my modules

class MyModule : public anl::ANLVModule
{
public:
  MyModule(const std::string& module_name="MyModule",
           const std::string& module_version="1.0");
  ~MyModule();
};
