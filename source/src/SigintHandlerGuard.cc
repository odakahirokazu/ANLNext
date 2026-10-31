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

#include "SigintHandlerGuard.hh"

#include <cstdio>

namespace anlnext
{

SigintHandlerGuard::SigintHandlerGuard() noexcept
{
  struct sigaction action{};
  action.sa_handler = SIG_DFL;
  action.sa_flags = SA_RESTART;
  sigemptyset(&action.sa_mask);
  active_ = sigaction(SIGINT, &action, &original_) == 0;
  if (!active_) { std::fputs("sigaction(2) error!\n", stderr); }
}

SigintHandlerGuard::~SigintHandlerGuard()
{
  restore();
}

bool SigintHandlerGuard::restore() noexcept
{
  if (!active_) { return true; }
  active_ = false;
  if (sigaction(SIGINT, &original_, nullptr) != 0) {
    std::fputs("sigaction(2) error!\n", stderr);
    return false;
  }
  return true;
}

} /* namespace anlnext */
