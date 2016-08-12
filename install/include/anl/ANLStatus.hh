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
 * @date 2014-12-10
 */
enum class ANLStatus { AS_OK, AS_SKIP, AS_SKIP_ERR, AS_QUIT, AS_QUIT_ERR };

std::string ANLStatusToString(ANLStatus status);

constexpr ANLStatus AS_OK       = ANLStatus::AS_OK;
constexpr ANLStatus AS_SKIP     = ANLStatus::AS_SKIP;
constexpr ANLStatus AS_SKIP_ERR = ANLStatus::AS_SKIP_ERR;
constexpr ANLStatus AS_QUIT     = ANLStatus::AS_QUIT;
constexpr ANLStatus AS_QUIT_ERR = ANLStatus::AS_QUIT_ERR;

} /* namespace anl */

std::ostream& operator<< (std::ostream& os, anl::ANLStatus status);

#endif /* ANL_ANLStatus_H */
