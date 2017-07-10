%module testHistogram
%{
#include "BasicModule.hh"

// include headers of my modules
#include "SaveData.hh"
#include "GenerateEvents.hh"
#include "FillHistogram.hh"

%}

%import "ANL.i"


// interface to my modules

class SaveData : public anl::BasicModule
{
public:
  SaveData();
};

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
