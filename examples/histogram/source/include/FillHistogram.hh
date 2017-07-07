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
public:
  FillHistogram();
  
  anl::ANLStatus mod_define() override;
  anl::ANLStatus mod_initialize() override;
  anl::ANLStatus mod_analyze() override;

private:
  int nbins_;
  double energy0_;
  double energy1_;
  
  TH1* spectrum1_ = nullptr;
  TH1* spectrum2_ = nullptr;
  TH1* spectrum_sum_ = nullptr;

  const GenerateEvents* event_ = nullptr;
};

#endif /* FillHistogram_H */
