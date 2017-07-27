%module testHistogram
%{
#include <anl/BasicModule.hh>

// include headers of my modules
#include "SaveData.hh"
#include "GenerateEvents.hh"
#include "FillHistogram.hh"

%}

%import(module="anlnext/ANL") "anl/ruby/ANL.i"


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
