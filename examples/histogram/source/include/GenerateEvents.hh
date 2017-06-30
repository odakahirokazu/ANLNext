/**
 * GenerateEvents.
 * This module generates an event with an energy property.
 *
 * @author 
 * @date
 *
 */

#ifndef GenerateEvents_H
#define GenerateEvents_H 1

#include "BasicModule.hh"

class TRandom3;

class GenerateEvents : public anl::BasicModule
{
  DEFINE_ANL_MODULE(GenerateEvents, 1.0);
public:
  GenerateEvents();
  ~GenerateEvents();
  
  anl::ANLStatus mod_startup() override;
  anl::ANLStatus mod_init() override;
  anl::ANLStatus mod_ana() override;

  double Energy() const { return _energy_generated; }

private:
  double _center;
  double _sigma1;
  double _sigma2;
  double _energy_generated;

  TRandom3* _random;
};

#endif /* GenerateEvents_H */
