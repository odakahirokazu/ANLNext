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

#include "EvsInterface.hh"
#include "EvsManager.hh"

namespace anlnext
{

EvsInterface::EvsInterface()
  : evs_manager_(nullptr)
{
}

EvsInterface::~EvsInterface() = default;

EvsInterface::EvsInterface(const EvsInterface&)
  : evs_manager_(nullptr)
{
}

void EvsInterface::set_evs_manager(EvsManager* man)
{
  evs_manager_ = man;
}

void EvsInterface::define_evs(const std::string& key)
{
  evs_manager_->define(key);
}

void EvsInterface::undefine_evs(const std::string& key)
{
  evs_manager_->undefine(key);
}

bool EvsInterface::is_evs_defined(const std::string& key) const
{
  return evs_manager_->is_defined(key);
}

bool EvsInterface::evs(const std::string& key) const
{
  return evs_manager_->get(key);
}

void EvsInterface::set_evs(const std::string& key)
{
  evs_manager_->set(key);
}

void EvsInterface::reset_evs(const std::string& key)
{
  evs_manager_->reset(key);
}

} /* namespace anlnext */
