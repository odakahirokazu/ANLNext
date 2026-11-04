/*************************************************************************
 *                                                                       *
 * Copyright (c) 2026 Hirokazu Odaka                                     *
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

#include "CalculationAlgorithm.hh"

void MultiplyAlgorithm::define_parameters()
{
  define_parameter("factor", &MultiplyAlgorithm::factor_);
  define_evs("multiply_used");
}

double MultiplyAlgorithm::calculate(double input)
{
  set_evs("multiply_used");
  return input * factor_;
}

void OffsetAlgorithm::define_parameters()
{
  define_parameter("offset", &OffsetAlgorithm::offset_);
  define_evs("offset_used");
}

double OffsetAlgorithm::calculate(double input)
{
  set_evs("offset_used");
  return input + offset_;
}
