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

#ifndef ANL_ANLStatus_H
#define ANL_ANLStatus_H 1

#include <ostream>

namespace anl
{

/**
 * scoped enum indicating a status of an ANL module.
 * @author Hirokazu Odaka
 * @date 2014-12-10
 * @date 2017-07-07 | add as_quit_all, as_quit_all_error
 */
enum class ANLStatus {
  ok,
  skip,
  skip_error,
  quit,
  quit_error,
  quit_all,
  quit_all_error
};

std::string status_to_string(ANLStatus status);

constexpr ANLStatus AS_OK             = ANLStatus::ok;
constexpr ANLStatus AS_SKIP           = ANLStatus::skip;
constexpr ANLStatus AS_SKIP_ERROR     = ANLStatus::skip_error;
constexpr ANLStatus AS_QUIT           = ANLStatus::quit;
constexpr ANLStatus AS_QUIT_ERROR     = ANLStatus::quit_error;
constexpr ANLStatus AS_QUIT_ALL       = ANLStatus::quit_all;
constexpr ANLStatus AS_QUIT_ALL_ERROR = ANLStatus::quit_all_error;

enum class ANLRequest {
  none,
  quit,
  show_event_index,
  show_evs_summary
};

} /* namespace anl */

std::ostream& operator<< (std::ostream& os, anl::ANLStatus status);

#endif /* ANL_ANLStatus_H */
