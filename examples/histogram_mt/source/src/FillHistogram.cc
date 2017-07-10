#include "FillHistogram.hh"
#include "TH1.h"
#include "GenerateEvents.hh"
#include "CreateRootFile.hh"

using namespace anl;

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

  if (exist_module("CreateRootFile")) {
    comptonsoft::CreateRootFile* fileManager = nullptr;
    get_module_NC("CreateRootFile", &fileManager);
    fileManager->cd();
  }

  spectrum_ = new TH1I("spectrum", "Spectrum", nbins_, energy0_, energy1_);
  
  return AS_OK;
}

ANLStatus FillHistogram::mod_analyze()
{
  const std::vector<double>& energies = event_->Energies();

  for (const auto& energy: energies) {
    spectrum_->Fill(energy);
  }

  return AS_OK;
}

ANLStatus FillHistogram::mod_merge(const BasicModule* parallel)
{
  const mod_class* m = dynamic_cast<const mod_class*>(parallel);
  if (m==nullptr) { return AS_QUIT_ERROR; }

  spectrum_->Add(m->spectrum_);
  
  return AS_OK;
}
