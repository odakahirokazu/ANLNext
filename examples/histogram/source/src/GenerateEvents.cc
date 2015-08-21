#include "GenerateEvents.hh"
#include "TRandom3.h"

using namespace anl;

GenerateEvents::GenerateEvents() :
  _center(59.5), _sigma1(1.0), _sigma2(5.0),
  _energy_generated(0.0),
  _random(0)
{
  _random = new TRandom3();
}

GenerateEvents::~GenerateEvents()
{
  delete _random;
}

ANLStatus GenerateEvents::mod_startup()
{
  register_parameter(&_center, "energy", 1.0, "keV");
  register_parameter(&_sigma1, "detector1_sigma", 1.0, "keV");
  set_parameter_description("Energy resolution (1-sigma) of the detector 1");
  register_parameter(&_sigma2, "detector2_sigma");
  set_parameter_description("Energy resolution (1-sigma) of the detector 2");

  return AS_OK;
}

ANLStatus GenerateEvents::mod_init()
{
  EvsDef("GenerateEvents:Detector1");
  EvsDef("GenerateEvents:Detector2");

  return AS_OK;
}

ANLStatus GenerateEvents::mod_ana()
{
  int detector = _random->Integer(10);

  if (detector <= 3) {
    _energy_generated = _random->Gaus(_center, _sigma1);
    EvsSet("GenerateEvents:Detector1");
  }
  else if (detector <= 8) {
    _energy_generated = _random->Gaus(_center, _sigma2);
    EvsSet("GenerateEvents:Detector2");
  }
  else {
    return AS_SKIP;
  }
  
  return AS_OK;
}
