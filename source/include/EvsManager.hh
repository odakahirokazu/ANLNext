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
  uint64_t counts_ok = 0;

  EvsData& operator+=(const EvsData& a)
  {
    counts    += a.counts;
    counts_ok += a.counts_ok;
    return *this;
  }
};

inline EvsData operator+(const EvsData& a, const EvsData& b)
{
  EvsData c;
  c.counts = a.counts + b.counts;
  c.counts_ok = a.counts_ok + b.counts_ok;
  return c;
}

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
 * @date 2016-12-20 | add count_ok
 */
class EvsManager
{
public:
  EvsManager() = default;
  ~EvsManager();
  EvsManager(const EvsManager&) = default;
  EvsManager(EvsManager&&) = default;
  EvsManager& operator=(const EvsManager&) = default;
  EvsManager& operator=(EvsManager&&) = default;

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
  void countCompleted();
  void printSummary();

  const EvsMap& data() const { return data_; }
  void merge(const EvsManager& r);

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
