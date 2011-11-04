// Hirokazu Odaka

#ifndef ANLAccess_hh
#define ANLAccess_hh

#include <string>
#include <map>

#include "ANLException.hh"

namespace anl
{

class ANLVModule;

class ANLAccess
{
public:
  const ANLVModule* GetModule(const std::string& name)
  { return GetModuleNC(name); }
  
  ANLVModule* GetModuleNC(const std::string& name);

  void RegisterModule(const std::string& name, ANLVModule* module);

  bool Exist(const std::string& name);

private:
  typedef std::map<std::string, ANLVModule*> ANLModuleMap;
  ANLModuleMap module_map;
};


inline
ANLVModule* ANLAccess::GetModuleNC(const std::string& name)
{
  ANLModuleMap::iterator it = module_map.find(name);
  if (it == module_map.end()) {
    BOOST_THROW_EXCEPTION( ANLException()
                           << ANLErrInfo(std::string("No module: ")+name) );
  }
  return it->second;
}


inline
void ANLAccess::RegisterModule(const std::string& name, ANLVModule* module)
{
  module_map[name] = module;
}


inline
bool ANLAccess::Exist(const std::string& name)
{
  return static_cast<bool>(module_map.count(name));
}

}

#endif // ANLAccess_hh
