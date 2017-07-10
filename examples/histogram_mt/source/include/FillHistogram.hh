/**
 * FillHistogram.
 *
 * @author 
 * @date
 *
 */

#ifndef FillHistogram_H
#define FillHistogram_H 1

#include "BasicModule.hh"

class TH1;
class GenerateEvents;


class FillHistogram : public anl::BasicModule
{
  DEFINE_ANL_MODULE(FillHistogram, 1.0);
  ENABLE_PARALLEL_RUN();
public:
  FillHistogram();

protected:
  FillHistogram(const FillHistogram&) = default;

public:
  anl::ANLStatus mod_define() override;
  anl::ANLStatus mod_initialize() override;
  anl::ANLStatus mod_analyze() override;

  anl::ANLStatus mod_merge(const BasicModule* parallel) override;

private:
  int nbins_;
  double energy0_;
  double energy1_;
  
  TH1* spectrum_ = nullptr;

  const GenerateEvents* event_ = nullptr;
};

#endif /* FillHistogram_H */
