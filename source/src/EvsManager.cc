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

void EvsManager::resetAllFlags()
{
  for (auto& e: data_) {
    e.second.flag = false;
  }
}

void EvsManager::resetAllCounts()
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

void EvsManager::countCompleted()
{
  for (auto& e: data_) {
    if (e.second.flag) {
      ++(e.second.counts_ok);
    }
  }
}

void EvsManager::printSummary()
{
  std::cout << "***** Results of Event Selection *****\n"
            << "    Number of EVS : " << std::setw(8) << data_.size() << '\n'
            << "                 key                        |     counts     |   completed \n";
  for (auto& e: data_) {
    std::cout << std::setw(44) << std::left << e.first << ' '
              << std::setw(16) << std::right << e.second.counts << ' '
              << std::setw(16) << std::right << e.second.counts_ok
              << std::setw(0) << '\n';
  }
  std::cout.flush();
}

} /* namespace anl */
