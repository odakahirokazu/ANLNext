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
  register_parameter(&nbins_, "nbin");
  set_parameter_description("Number of bins");
  register_parameter(&energy0_, "energy_min", 1.0, "keV");
  set_parameter_description("Lower bound of the histograms");
  register_parameter(&energy1_, "energy_max", 1.0, "keV");
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
  const ThisModuleClass* m = dynamic_cast<const ThisModuleClass*>(parallel);
  if (m==nullptr) { return AS_QUIT_ERROR; }

  spectrum_->Add(m->spectrum_);
  
  return AS_OK;
}
