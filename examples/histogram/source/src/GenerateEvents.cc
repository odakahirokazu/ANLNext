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
  register_parameter(&center_, "energy", 1.0, "keV");
  register_parameter(&sigma1_, "detector1_sigma", 1.0, "keV");
  set_parameter_description("Energy resolution (1-sigma) of the detector 1");
  register_parameter(&sigma2_, "detector2_sigma");
  set_parameter_description("Energy resolution (1-sigma) of the detector 2");

  return AS_OK;
}

ANLStatus GenerateEvents::mod_initialize()
{
  EvsDef("GenerateEvents:Detector1");
  EvsDef("GenerateEvents:Detector2");

  return AS_OK;
}

ANLStatus GenerateEvents::mod_analyze()
{
  int detector = random_->Integer(10);

  if (detector <= 3) {
    energy_generated_ = random_->Gaus(center_, sigma1_);
    EvsSet("GenerateEvents:Detector1");
  }
  else if (detector <= 8) {
    energy_generated_ = random_->Gaus(center_, sigma2_);
    EvsSet("GenerateEvents:Detector2");
  }
  else {
    return AS_SKIP;
  }
  
  return AS_OK;
}
