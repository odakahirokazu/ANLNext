%module testHistogramMT
%{
#include <anl/BasicModule.hh>

// include headers of my modules
#include "CreateRootFile.hh"
#include "GenerateEvents.hh"
#include "FillHistogram.hh"

%}

%import "anl/ruby/ANL.i"


// interface to my modules

namespace comptonsoft
{

class CreateRootFile : public anl::BasicModule
{
public:
  CreateRootFile();
  ~CreateRootFile();
};

}

class GenerateEvents : public anl::BasicModule
{
public:
  GenerateEvents();
};

class FillHistogram : public anl::BasicModule
{
public:
  FillHistogram();
};
