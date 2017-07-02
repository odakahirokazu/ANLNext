#include "FillHistogram.hh"
#include "TH1.h"
#include "GenerateEvents.hh"
#include "SaveData.hh"

using namespace anl;

FillHistogram::FillHistogram() :
  _nbin(128), _energy0(0.0), _energy1(100.0),
  _spectrum1(0), _spectrum2(0), _spectrum_sum(0),
  _event(0)
{
}


FillHistogram::~FillHistogram()
{
}


ANLStatus FillHistogram::mod_startup()
{
  register_parameter(&_nbin, "nbin");
  set_parameter_description("Number of bins");
  register_parameter(&_energy0, "energy_min", 1.0, "keV");
  set_parameter_description("Lower bound of the histograms");
  register_parameter(&_energy1, "energy_max", 1.0, "keV");
  set_parameter_description("Upper bound of the histograms");

  return AS_OK;
}


ANLStatus FillHistogram::mod_init()
{
  GetModule("GenerateEvents", &_event);

  if (ModuleExist("SaveData")) {
    SaveData* saveModule;
    GetModuleNC("SaveData", &saveModule);
    saveModule->cd();
  }
  
  _spectrum1 = new TH1I("spectrum1", "Spectrum 1", _nbin, _energy0, _energy1);
  _spectrum2 = new TH1I("spectrum2", "Spectrum 2", _nbin, _energy0, _energy1);
  _spectrum_sum = new TH1I("spectrum_sum", "Spectrum 1+2", _nbin, _energy0, _energy1);
  
  return AS_OK;
}


ANLStatus FillHistogram::mod_ana()
{
  double energy = _event->Energy();
  if (Evs("GenerateEvents:Detector1")) {
    _spectrum1->Fill(energy);
    _spectrum_sum->Fill(energy);
  }

  if (Evs("GenerateEvents:Detector2")) {
    _spectrum2->Fill(energy);
    _spectrum_sum->Fill(energy);
  }

  return AS_OK;
}
