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

#include <anlnext/BasicModule.hh>
#include <memory>
#include "TRandom.h"

class GenerateEvents : public anlnext::BasicModule
{
  DEFINE_ANL_MODULE(GenerateEvents, 1.0);
  ENABLE_PARALLEL_RUN();
public:
  GenerateEvents();
  
protected:
  GenerateEvents(const GenerateEvents&);

public:
  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_begin_run() override;
  anlnext::ANLStatus mod_analyze() override;
  anlnext::ANLStatus mod_end_run() override;

  const std::vector<double>& Energies() const
  { return energies_generated_; }

private:
  double center_;
  double sigma_;
  std::vector<double> energies_generated_;
  double efficiency_;
  int num_detectors_;
  int random_seed_;
  std::unique_ptr<TRandom> random_;
  int sum_events_;
};

#endif /* GenerateEvents_H */
