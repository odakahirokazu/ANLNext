#ifndef EvsManager_hh
#define EvsManager_hh

#include <map>
#include <string>
#include <iostream>

namespace anl
{

typedef std::map<std::string, std::pair<int, int> > EvsMap;
typedef EvsMap::iterator EvsMapIter;

class EvsManager
{
public:
  EvsManager()  {}
  ~EvsManager() {}

  void EvsDef(const std::string& key)   { data[key] = std::make_pair(0, 0); }
  void EvsUndef(const std::string& key) { data.erase(key); }
  bool EvsIsDef(const std::string& key) { return data.count(key); }

  bool Evs(const std::string& key);
  void EvsSet(const std::string& key);
  void EvsReset(const std::string& key);
  
  void ResetAll();
  void Initialize() { data.clear(); }
  void Count();
  void PrintSummary();

private:
  EvsMap data;
};


inline bool EvsManager::Evs(const std::string& key)
{
  EvsMapIter i = data.find(key);
  if (i==data.end()) {
    std::cout << "EvsManager:: Undefined key is given: " << key << std::endl;
    return false; 
  }
  return i->second.first;
}


inline void EvsManager::EvsSet(const std::string& key)
{
  EvsMapIter i = data.find(key);
  if (i==data.end()) {
    std::cout << "EvsManager:: Undefined key is given: " << key << std::endl;
    return; 
  }
  i->second.first = true;
}


inline void EvsManager::EvsReset(const std::string& key)
{
  EvsMapIter i = data.find(key);
  if (i==data.end()) {
    std::cout << "EvsManager:: Undefined key is given: " << key << std::endl;
    return; 
  }
  i->second.first = false;
}

}

#endif
