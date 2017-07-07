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

#include "ANLStatus.hh"
#include <string>

std::string anl::status_to_string(anl::ANLStatus status)
{
  switch (status) {
    case anl::ANLStatus::AS_OK:
      return "AS_OK";
    case anl::ANLStatus::AS_SKIP:
      return "AS_SKIP";
    case anl::ANLStatus::AS_SKIP_ERROR:
      return "AS_SKIP_ERROR";
    case anl::ANLStatus::AS_QUIT:
      return "AS_QUIT";
    case anl::ANLStatus::AS_QUIT_ERROR:
      return "AS_QUIT_ERROR";
    case anl::ANLStatus::AS_QUIT_ALL:
      return "AS_QUIT_ALL";
    case anl::ANLStatus::AS_QUIT_ALL_ERROR:
      return "AS_QUIT_ALL_ERROR";
    default:
      return "AS_UNDEFINED";
  }
}

std::ostream& operator<< (std::ostream& os, anl::ANLStatus status)
{
  os << anl::status_to_string(status);
  return os;
}
