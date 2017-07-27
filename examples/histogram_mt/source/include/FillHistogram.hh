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
  ENABLE_PARALLEL_RUN();
public:
  FillHistogram();

protected:
  FillHistogram(const FillHistogram&) = default;

public:
  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_analyze() override;

  anlnext::ANLStatus mod_merge(const BasicModule* parallel) override;

private:
  int nbins_;
  double energy0_;
  double energy1_;
  
  TH1* spectrum_ = nullptr;

  const GenerateEvents* event_ = nullptr;
};

#endif /* FillHistogram_H */
