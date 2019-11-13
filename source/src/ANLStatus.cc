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

namespace anlnext
{

std::string status_to_string(ANLStatus status)
{
  switch (status) {
    case ANLStatus::ok:
      return "AS_OK";
    case ANLStatus::error:
      return "AS_ERROR";
    case ANLStatus::skip:
      return "AS_SKIP";
    case ANLStatus::skip_error:
      return "AS_SKIP_ERROR";
    case ANLStatus::redo:
      return "AS_REDO";
    case ANLStatus::quit:
      return "AS_QUIT";
    case ANLStatus::quit_error:
      return "AS_QUIT_ERROR";
    case ANLStatus::quit_all:
      return "AS_QUIT_ALL";
    case ANLStatus::quit_all_error:
      return "AS_QUIT_ALL_ERROR";
    case ANLStatus::critical_error_to_finalize:
      return "AS_CRITICAL_ERROR_TO_FINALIZE";
    case ANLStatus::critical_error_to_terminate:
      return "AS_CRITICAL_ERROR_TO_TERMINATE";
    case ANLStatus::critical_error_to_finalize_from_exception:
      return "AS_CRITICAL_ERROR_TO_FINALIZE_FROM_EXCEPTION";
    case ANLStatus::critical_error_to_terminate_from_exception:
      return "AS_CRITICAL_ERROR_TO_TERMINATE_FROM_EXCEPTION";

    default:
      return "AS_UNDEFINED";
  }
}

bool is_normal_error(ANLStatus status)
{
  return ((status==ANLStatus::error) ||
          (status==ANLStatus::skip_error) ||
          (status==ANLStatus::quit_error) ||
          (status==ANLStatus::quit_all_error));
}

bool is_critical_error(ANLStatus status)
{
  return ((status==ANLStatus::critical_error_to_finalize) ||
          (status==ANLStatus::critical_error_to_terminate) ||
          (status==ANLStatus::critical_error_to_finalize_from_exception) ||
          (status==ANLStatus::critical_error_to_terminate_from_exception));
}

ANLStatus eliminate_normal_error_status(ANLStatus status)
{
  if (status==ANLStatus::error) { return ANLStatus::ok; }
  if (status==ANLStatus::skip_error) { return ANLStatus::skip; }
  if (status==ANLStatus::quit_error) { return ANLStatus::quit; }
  if (status==ANLStatus::quit_all_error) { return ANLStatus::quit_all; }
  return status;
}

} /* namespace anlnext */

std::ostream& operator<< (std::ostream& os, anlnext::ANLStatus status)
{
  os << anlnext::status_to_string(status);
  return os;
}
