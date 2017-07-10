#include "GenerateEvents.hh"
#include "TRandom3.h"

using namespace anl;

GenerateEvents::GenerateEvents()
  : center_(59.5), sigma_(2.0), efficiency_(0.1),
    num_detectors_(100), random_seed_(0),
    sum_events_(0)
{
}

GenerateEvents::GenerateEvents(const GenerateEvents& r)
  : BasicModule::BasicModule(r),
    center_(r.center_), sigma_(r.sigma_), efficiency_(r.efficiency_),
    num_detectors_(r.num_detectors_), random_seed_(r.random_seed_),
    random_(nullptr),
    sum_events_(r.sum_events_)
{
}

ANLStatus GenerateEvents::mod_define()
{
  register_parameter(&center_, "energy", 1.0, "keV");
  register_parameter(&sigma_, "sigma", 1.0, "keV");
  set_parameter_description("Energy resolution (1-sigma) of the detectors");
  register_parameter(&efficiency_, "efficiency");
  set_parameter_description("Detection efficiency");
  register_parameter(&num_detectors_, "num_detectors");
  register_parameter(&random_seed_, "random_seed");

  return AS_OK;
}

ANLStatus GenerateEvents::mod_initialize()
{
  random_.reset(new TRandom3(random_seed_+copy_id()));
  energies_generated_.reserve(num_detectors_);

  define_evs("GenerateEvents:Hit");

  return AS_OK;
}

ANLStatus GenerateEvents::mod_begin_run()
{
  sum_events_ = 0;
  return AS_OK;
}

ANLStatus GenerateEvents::mod_analyze()
{
  energies_generated_.clear();

  for (int i=0; i<num_detectors_; i++) {
    const double energy = random_->Gaus(center_, sigma_);
    if (random_->Uniform(1.0) < efficiency_) {
      energies_generated_.push_back(energy);
      ++sum_events_;
    }
  }

  if (energies_generated_.size() > 0) {
    set_evs("GenerateEvents:Hit");
  }
  
  return AS_OK;
}

ANLStatus GenerateEvents::mod_end_run()
{
  std::cout << "GenerateEvents[copy_id=" << copy_id() << "]: "
            << "sum events = " << sum_events_
            << std::endl;
  return AS_OK;
}
