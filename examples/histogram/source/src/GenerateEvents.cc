#include "GenerateEvents.hh"
#include "TRandom3.h"

using namespace anl;

GenerateEvents::GenerateEvents() :
  center_(59.5), sigma1_(1.0), sigma2_(5.0),
  energy_generated_(0.0),
  random_(new TRandom3)
{
}

ANLStatus GenerateEvents::mod_define()
{
  define_parameter("energy", &mod_class::center_, 1.0, "keV");
  define_parameter("detector1_sigma", &mod_class::sigma1_, 1.0, "keV");
  set_parameter_description("Energy resolution (1-sigma) of the detector 1");
  define_parameter("detector2_sigma", &mod_class::sigma2_);
  set_parameter_description("Energy resolution (1-sigma) of the detector 2");

  return AS_OK;
}

ANLStatus GenerateEvents::mod_initialize()
{
  define_evs("GenerateEvents:Detector1");
  define_evs("GenerateEvents:Detector2");

  return AS_OK;
}

ANLStatus GenerateEvents::mod_analyze()
{
  int detector = random_->Integer(10);

  if (detector <= 3) {
    energy_generated_ = random_->Gaus(center_, sigma1_);
    set_evs("GenerateEvents:Detector1");
  }
  else if (detector <= 8) {
    energy_generated_ = random_->Gaus(center_, sigma2_);
    set_evs("GenerateEvents:Detector2");
  }
  else {
    return AS_SKIP;
  }
  
  return AS_OK;
}
