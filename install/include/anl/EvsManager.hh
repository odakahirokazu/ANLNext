/*************************************************************************
 *                                                                       *
 * Copyright (c) 2011 Hirokazu Odaka                                     *
 *                                                                       *
 * This program is free software: you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                       *
 *************************************************************************/

#ifndef ANL_EvsManager_H
#define ANL_EvsManager_H 1

#include <cstdint>
#include <map>
#include <string>
#include <iostream>

namespace anl
{

struct EvsData
{
  bool flag = false;
  uint64_t counts = 0;
};

typedef std::map<std::string, EvsData> EvsMap;
typedef EvsMap::iterator EvsIter;
typedef EvsMap::const_iterator EvsConstIter;

/**
 * The Evs (event selection) management class.
 * This class provides a flag that can be accessed from any ANL module for event selection.
 *
 * @author Hirokazu Odaka
 * @date 2010-06-xx
 * @date 2014-12-18
 */
class EvsManager
{
public:
  EvsManager() = default;
  ~EvsManager();

  void initialize();

  /**
   * define an Evs flag.
   */
  void define(const std::string& key)
  { data_[key] = EvsData(); }

  /**
   * unregister an Evs flag.
   */
  void undefine(const std::string& key) { data_.erase(key); }

  bool isDefined(const std::string& key) const
  { return data_.count(key); }

  /**
   * get an Evs flag value.
   */
  bool get(const std::string& key) const;

  /**
   * set an Evs flag as true.
   */
  void set(const std::string& key);

  /**
   * set an Evs flag as false.
   */
  void reset(const std::string& key);
  
  void resetAllFlags();
  void resetAllCounts();

  void count();
  void printSummary();

private:
  EvsMap data_;
};

inline bool EvsManager::get(const std::string& key) const
{
  EvsConstIter it = data_.find(key);
  if (it==data_.end()) {
    std::cout << "EvsManager: Undefined key is given: " << key << std::endl;
    return false;
  }
  return it->second.flag;
}

inline void EvsManager::set(const std::string& key)
{
  EvsIter it = data_.find(key);
  if (it==data_.end()) {
    std::cout << "EvsManager: Undefined key is given: " << key << std::endl;
    return;
  }
  it->second.flag = true;
}

inline void EvsManager::reset(const std::string& key)
{
  EvsIter it = data_.find(key);
  if (it==data_.end()) {
    std::cout << "EvsManager: Undefined key is given: " << key << std::endl;
    return; 
  }
  it->second.flag = false;
}

} /* namespace anl */

#endif /* ANL_EvsManager_H */
