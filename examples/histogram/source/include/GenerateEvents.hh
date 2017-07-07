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
#include <memory>
#include "TRandom.h"

class GenerateEvents : public anl::BasicModule
{
  DEFINE_ANL_MODULE(GenerateEvents, 1.0);
public:
  GenerateEvents();
  
  anl::ANLStatus mod_define() override;
  anl::ANLStatus mod_initialize() override;
  anl::ANLStatus mod_analyze() override;

  double Energy() const { return energy_generated_; }

private:
  double center_;
  double sigma1_;
  double sigma2_;
  double energy_generated_;

  std::unique_ptr<TRandom> random_;
};

#endif /* GenerateEvents_H */
