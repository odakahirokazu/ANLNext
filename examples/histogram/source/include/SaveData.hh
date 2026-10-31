/**
 * SaveData
 *
 * @author Hirokazu Odaka
 * @date 2008-04-30
 */

#ifndef SaveData_H
#define SaveData_H 1

#include <anlnext/BasicModule.hh>
#include "TFile.h"


class SaveData : public anlnext::BasicModule
{
  DEFINE_ANL_MODULE(SaveData, 2.0);
public:
  SaveData();
  
  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_initialize() override;
  anlnext::ANLStatus mod_finalize() override;

  TDirectory* get_directory() { return root_file_->GetDirectory(0); }
  bool cd() { return root_file_->cd(); }
  std::string filename() const { return filename_; }
private:
  std::string filename_;
  TFile* root_file_;
};

#endif /* SaveData_H */
