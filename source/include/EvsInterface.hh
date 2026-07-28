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

#ifndef ANLNEXT_EvsInterface_H
#define ANLNEXT_EvsInterface_H 1

#include <string>

namespace anlnext
{

class EvsManager;

/**
 * A basic class for an ANL Next module.
 *
 * @author Hirokazu Odaka
 * @date 2026-06-24 | seperate from BasicModule
 */
class EvsInterface
{
public:
  EvsInterface();
  virtual ~EvsInterface();
  EvsInterface(const EvsInterface& r);
  EvsInterface& operator=(const EvsInterface& r) = delete;

  virtual void set_evs_manager(EvsManager* evs_manager);
  
protected:
  void define_evs(const std::string& key);
  void undefine_evs(const std::string& key);
  bool is_evs_defined(const std::string& key) const;
  bool evs(const std::string& key) const;
  void set_evs(const std::string& key);
  void reset_evs(const std::string& key);

private:
  EvsManager* evs_manager_ = nullptr;
};

} /* namespace anlnext */

#endif /* ANLNEXT_EvsInterface_H */
