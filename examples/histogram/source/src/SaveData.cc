#include "SaveData.hh"
#include <cstdio>
#include <iostream>

using namespace anlnext;

SaveData::SaveData() :
  filename_("output.root"), root_file_(0)
{
}

ANLStatus SaveData::mod_define()
{
  define_parameter("filename", &mod_class::filename_);
  set_parameter_description("Output file name");
  return AS_OK;
}

ANLStatus SaveData::mod_initialize()
{
  root_file_ = new TFile(filename_.c_str(), "recreate");
  if ( !root_file_ ) {
    std::cout << "SaveData: cannot create ROOT file" << std::endl;
    return AS_QUIT;
  }
  
  return AS_OK;
}

ANLStatus SaveData::mod_finalize()
{
  std::cout << "SaveData: saving data to ROOT file" << std::endl;
  root_file_->Write();
  std::cout << "SaveData: closing ROOT file" << std::endl;
  root_file_->Close();
  std::cout << "SaveData: ROOT file closed " << std::endl;
  
  return AS_OK;
}
