%module testHistogramMTPy
%{
#include <anlnext/BasicModule.hh>

// include headers of my modules
#include "CreateRootFile.hh"
#include "GenerateEvents.hh"
#include "FillHistogram.hh"

%}

%import(module="anlnext.anlnextpy") "anlnext/python/anlnextpy.i"


// interface to my modules

namespace comptonsoft
{
class CreateRootFile : public anlnext::BasicModule {};
}

class GenerateEvents : public anlnext::BasicModule {};
class FillHistogram : public anlnext::BasicModule {};
