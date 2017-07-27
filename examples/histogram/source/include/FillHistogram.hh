/**
 * FillHistogram.
 *
 * @author 
 * @date
 *
 */

#ifndef FillHistogram_H
#define FillHistogram_H 1

#include <anlnext/BasicModule.hh>

class TH1;
class GenerateEvents;


class FillHistogram : public anlnext::BasicModule
{
  DEFINE_ANL_MODULE(FillHistogram, 1.0);
public:
  FillHistogram();
  
  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_analyze() override;

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
