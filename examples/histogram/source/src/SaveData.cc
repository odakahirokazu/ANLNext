#include "SaveData.hh"
#include <cstdio>
#include <iostream>

using namespace anl;

SaveData::SaveData() :
  m_FileName("output.root"), m_RootFile(0)
{
}

ANLStatus SaveData::mod_define()
{
  define_parameter("filename", &mod_class::m_FileName);
  set_parameter_description("Output file name");
  return AS_OK;
}

ANLStatus SaveData::mod_initialize()
{
  m_RootFile = new TFile(m_FileName.c_str(), "recreate");
  if ( !m_RootFile ) {
    std::cout << "SaveData: cannot create ROOT file" << std::endl;
    return AS_QUIT;
  }
  
  return AS_OK;
}

ANLStatus SaveData::mod_finalize()
{
  std::cout << "SaveData: saving data to ROOT file" << std::endl;
  m_RootFile->Write();
  std::cout << "SaveData: closing ROOT file" << std::endl;
  m_RootFile->Close();
  std::cout << "SaveData: ROOT file closed " << std::endl;
  
  return AS_OK;
}
