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

#ifndef ANLNEXT_SigintHandlerGuard_H
#define ANLNEXT_SigintHandlerGuard_H 1

#include <csignal>

namespace anlnext
{

class SigintHandlerGuard
{
public:
  SigintHandlerGuard() noexcept;
  ~SigintHandlerGuard();
  SigintHandlerGuard(const SigintHandlerGuard&) = delete;
  SigintHandlerGuard& operator=(const SigintHandlerGuard&) = delete;

  bool is_active() const { return active_; }
  bool restore() noexcept;

private:
  struct sigaction original_{};
  bool active_ = false;
};

} /* namespace anlnext */

#endif /* ANLNEXT_SigintHandlerGuard_H */
