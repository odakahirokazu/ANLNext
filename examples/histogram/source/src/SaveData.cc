#include "SaveData.hh"
#include <cstdio>
#include <iostream>

using namespace anl;

SaveData::SaveData() :
  m_FileName("output.root"), m_RootFile(0)
{
}

ANLStatus SaveData::mod_startup()
{
  register_parameter(&m_FileName, "filename");
  set_parameter_description("Output file name");
  return AS_OK;
}

ANLStatus SaveData::mod_init()
{
  m_RootFile = new TFile(m_FileName.c_str(), "recreate");
  if ( !m_RootFile ) {
    std::cout << "SaveData: cannot create ROOT file" << std::endl;
    return AS_QUIT;
  }
  
  return AS_OK;
}

ANLStatus SaveData::mod_exit()
{
  std::cout << "SaveData: saving data to ROOT file" << std::endl;
  m_RootFile->Write();
  std::cout << "SaveData: closing ROOT file" << std::endl;
  m_RootFile->Close();
  std::cout << "SaveData: ROOT file closed " << std::endl;
  
  return AS_OK;
}
