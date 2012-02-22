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
