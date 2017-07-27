%module testHistogramMT
%{
#include <anlnext/BasicModule.hh>

// include headers of my modules
#include "CreateRootFile.hh"
#include "GenerateEvents.hh"
#include "FillHistogram.hh"

%}

%import(module="anlnext/ANL") "anlnext/ruby/ANL.i"


// interface to my modules

namespace comptonsoft
{
class CreateRootFile : public anlnext::BasicModule {};
}

class GenerateEvents : public anlnext::BasicModule {};
class FillHistogram : public anlnext::BasicModule {};
