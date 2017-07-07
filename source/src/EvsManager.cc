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

#include "EvsManager.hh"
#include <iomanip>

namespace anl
{

EvsManager::~EvsManager() = default;

void EvsManager::initialize()
{
  data_.clear();
}

void EvsManager::reset_all_flags()
{
  for (auto& e: data_) {
    e.second.flag = false;
  }
}

void EvsManager::reset_all_counts()
{
  for (auto& e: data_) {
    e.second.counts = 0;
  }
}

void EvsManager::count()
{
  for (auto& e: data_) {
    if (e.second.flag) {
      ++(e.second.counts);
    }
  }
}

void EvsManager::count_completed()
{
  for (auto& e: data_) {
    if (e.second.flag) {
      ++(e.second.counts_ok);
    }
  }
}

void EvsManager::print_summary() const
{
  std::cout << '\n'
            << "      ***********************************\n"
            << "      *** Results of Event Selections ***\n"
            << "      ***********************************\n"
            << std::endl;
  std::cout << "  Number of EVS : " << data_.size() << '\n'
            << "------------------------------------------------------------------------------\n"
            << "                 key                        |     counts     |   completed    \n"
            << "------------------------------------------------------------------------------\n";
  for (auto& e: data_) {
    std::cout << std::setw(44) << std::left << e.first << ' '
              << std::setw(16) << std::right << e.second.counts << ' '
              << std::setw(16) << std::right << e.second.counts_ok
              << std::setw(0) << '\n';
  }
  std::cout << "------------------------------------------------------------------------------\n"
            << std::endl;
}

void EvsManager::merge(const EvsManager& r)
{
  for (const auto& evs: r.data()) {
    const std::string& key = evs.first;
    if (is_defined(key)) {
      data_[key] += evs.second;
    }
    else {
      data_[key] = evs.second;
    }
  }
}

} /* namespace anl */
