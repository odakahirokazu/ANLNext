%module testHistogram
%{
#include <anlnext/BasicModule.hh>

// include headers of my modules
#include "SaveData.hh"
#include "GenerateEvents.hh"
#include "FillHistogram.hh"

%}

%import(module="anlnext/ANL") "anlnext/ruby/ANL.i"


// interface to my modules

class SaveData : public anlnext::BasicModule {};
class GenerateEvents : public anlnext::BasicModule {};
class FillHistogram : public anlnext::BasicModule {};
