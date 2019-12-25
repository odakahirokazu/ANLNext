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

#ifndef ANLNEXT_ClonedChainSet_impl_H
#define ANLNEXT_ClonedChainSet_impl_H 1

#include "ClonedChainSet.hh"
#include "EvsManager.hh"

namespace anlnext
{

template <typename T>
ANLStatus ClonedChainSet::process(T func)
{
  return func(modules_ref_, counters_, *evs_manager_);
}

} /* namespace anlnext */

#endif /* ANLNEXT_ClonedChainSet_impl_H */
