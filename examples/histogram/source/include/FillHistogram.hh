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
  ~FillHistogram();
  
  anl::ANLStatus mod_startup();
  anl::ANLStatus mod_init();
  anl::ANLStatus mod_ana();

private:
  int _nbin;
  double _energy0;
  double _energy1;
  
  TH1* _spectrum1;
  TH1* _spectrum2;
  TH1* _spectrum_sum;

  const GenerateEvents* _event;
};

#endif /* FillHistogram_H */
