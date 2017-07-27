#include "FillHistogram.hh"
#include "TH1.h"
#include "GenerateEvents.hh"
#include "SaveData.hh"

using namespace anlnext;

FillHistogram::FillHistogram() :
  nbins_(128), energy0_(0.0), energy1_(100.0)
{
}

ANLStatus FillHistogram::mod_define()
{
  define_parameter("nbin", &mod_class::nbins_);
  set_parameter_description("Number of bins");
  define_parameter("energy_min", &mod_class::energy0_, 1.0, "keV");
  set_parameter_description("Lower bound of the histograms");
  define_parameter("energy_max", &mod_class::energy1_, 1.0, "keV");
  set_parameter_description("Upper bound of the histograms");

  return AS_OK;
}

ANLStatus FillHistogram::mod_initialize()
{
  get_module("GenerateEvents", &event_);

  if (exist_module("SaveData")) {
    SaveData* saveModule = nullptr;
    get_module_NC("SaveData", &saveModule);
    saveModule->cd();
  }
  
  spectrum1_ = new TH1I("spectrum1", "Spectrum 1", nbins_, energy0_, energy1_);
  spectrum2_ = new TH1I("spectrum2", "Spectrum 2", nbins_, energy0_, energy1_);
  spectrum_sum_ = new TH1I("spectrum_sum", "Spectrum 1+2", nbins_, energy0_, energy1_);
  
  return AS_OK;
}

ANLStatus FillHistogram::mod_analyze()
{
  const double energy = event_->Energy();
  if (evs("GenerateEvents:Detector1")) {
    spectrum1_->Fill(energy);
    spectrum_sum_->Fill(energy);
  }

  if (evs("GenerateEvents:Detector2")) {
    spectrum2_->Fill(energy);
    spectrum_sum_->Fill(energy);
  }

  return AS_OK;
}
