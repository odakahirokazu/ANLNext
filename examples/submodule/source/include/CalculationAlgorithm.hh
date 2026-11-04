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

#ifndef ANLNEXT_EXAMPLE_CALCULATION_ALGORITHM_H
#define ANLNEXT_EXAMPLE_CALCULATION_ALGORITHM_H

#include "BasicSubModule.hh"

class CalculationAlgorithm : public anlnext::BasicSubModule
{
public:
  virtual double calculate(double input) = 0;
};

class MultiplyAlgorithm : public CalculationAlgorithm
{
  DEFINE_ANL_SUBMODULE(MultiplyAlgorithm, 1.0);
public:
  void define_parameters() override;
  double calculate(double input) override;

private:
  double factor_ = 2.0;
};

class OffsetAlgorithm : public CalculationAlgorithm
{
  DEFINE_ANL_SUBMODULE(OffsetAlgorithm, 1.0);
public:
  void define_parameters() override;
  double calculate(double input) override;

private:
  double offset_ = 1.0;
};

#endif
