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
    case anl::ANLStatus::ok:
      return "AS_OK";
    case anl::ANLStatus::error:
      return "AS_ERROR";
    case anl::ANLStatus::skip:
      return "AS_SKIP";
    case anl::ANLStatus::skip_error:
      return "AS_SKIP_ERROR";
    case anl::ANLStatus::quit:
      return "AS_QUIT";
    case anl::ANLStatus::quit_error:
      return "AS_QUIT_ERROR";
    case anl::ANLStatus::quit_all:
      return "AS_QUIT_ALL";
    case anl::ANLStatus::quit_all_error:
      return "AS_QUIT_ALL_ERROR";
    case anl::ANLStatus::critical_error_to_finalize:
      return "AS_CRITICAL_ERROR_TO_FINALIZE";
    case anl::ANLStatus::critical_error_to_terminate:
      return "AS_CRITICAL_ERROR_TO_TERMINATE";
    case anl::ANLStatus::critical_error_to_finalize_from_exception:
      return "AS_CRITICAL_ERROR_TO_FINALIZE_FROM_EXCEPTION";
    case anl::ANLStatus::critical_error_to_terminate_from_exception:
      return "AS_CRITICAL_ERROR_TO_TERMINATE_FROM_EXCEPTION";

    default:
      return "AS_UNDEFINED";
  }
}

bool anl::is_normal_error(anl::ANLStatus status)
{
  return ((status==ANLStatus::error) ||
          (status==ANLStatus::skip_error) ||
          (status==ANLStatus::quit_error) ||
          (status==ANLStatus::quit_all_error));
}

bool anl::is_critical_error(anl::ANLStatus status)
{
  return ((status==ANLStatus::critical_error_to_finalize) ||
          (status==ANLStatus::critical_error_to_terminate) ||
          (status==ANLStatus::critical_error_to_finalize_from_exception) ||
          (status==ANLStatus::critical_error_to_terminate_from_exception));
}

anl::ANLStatus anl::eliminate_normal_error_status(ANLStatus status)
{
  if (status==ANLStatus::error) { return ANLStatus::ok; }
  if (status==ANLStatus::skip_error) { return ANLStatus::skip; }
  if (status==ANLStatus::quit_error) { return ANLStatus::quit; }
  if (status==ANLStatus::quit_all_error) { return ANLStatus::quit_all; }
  return status;
}

std::ostream& operator<< (std::ostream& os, anl::ANLStatus status)
{
  os << anl::status_to_string(status);
  return os;
}
