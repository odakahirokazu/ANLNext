#include "EvsManager.hh"
#include <iomanip>

using namespace anl;

void EvsManager::ResetAll()
{
  EvsMapIter const iEnd = data.end();
  for (EvsMapIter i = data.begin(); i!=iEnd; ++i) {
    i->second.first = false;
  }
}


void EvsManager::Count()
{
  EvsMapIter const iEnd = data.end();
  for (EvsMapIter i = data.begin(); i!=iEnd; ++i) {
    if (i->second.first) {
      ++(i->second.second);
    }
  }
}


void EvsManager::PrintSummary()
{
  std::cout << "***** Results of Event Selection *****\n"
            << "    Number of EVS : " << std::setw(8) << data.size() << '\n';
  EvsMapIter const iEnd = data.end();
  for (EvsMapIter i = data.begin(); i!=iEnd; ++i) {
    std::cout << std::setw(12) << i->second.second
              << std::setw(0) << " : "
              << i->first << '\n';
  }
  std::cout.flush();
}
