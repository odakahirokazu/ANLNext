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

#ifndef ANLNEXT_EXAMPLE_ALGORITHM_MODULE_H
#define ANLNEXT_EXAMPLE_ALGORITHM_MODULE_H

#include "BasicModule.hh"

class CalculationAlgorithm;

struct AlgorithmSelectionError : anlnext::ANLException
{
  AlgorithmSelectionError(const anlnext::BasicModule* module, const std::string& key);
};

class AlgorithmModule : public anlnext::BasicModule
{
  DEFINE_ANL_MODULE(AlgorithmModule, 1.0);
  ENABLE_PARALLEL_RUN();
public:
  anlnext::ANLStatus mod_define() override;
  anlnext::ANLStatus mod_begin_run() override;
  anlnext::ANLStatus mod_analyze() override;
  anlnext::ANLStatus mod_merge(const anlnext::BasicModule* module) override;
  void set_submodule_by_key(const std::string& name, const std::string& key) override;

private:
  double input_ = 4.0;
  double result_ = 0.0;
  double total_ = 0.0;
  CalculationAlgorithm* algorithm_ = nullptr;
};

#endif
