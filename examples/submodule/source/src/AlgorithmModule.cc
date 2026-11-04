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

#include "AlgorithmModule.hh"
#include "CalculationAlgorithm.hh"

using namespace anlnext;

AlgorithmSelectionError::AlgorithmSelectionError(const BasicModule* module, const std::string& key)
  : ANLException("Unknown calculation algorithm: " + key)
{
  set_module_info(module);
}

ANLStatus AlgorithmModule::mod_define()
{
  define_parameter("input", &AlgorithmModule::input_);
  define_result("result", &AlgorithmModule::result_);
  define_result("total", &AlgorithmModule::total_);
  define_submodule("algorithm", &AlgorithmModule::algorithm_);
  return AS_OK;
}

void AlgorithmModule::set_submodule_by_key(const std::string& name, const std::string& key)
{
  if (key == "multiply") {
    set_submodule(name, std::make_unique<MultiplyAlgorithm>());
  }
  else if (key == "offset") {
    set_submodule(name, std::make_unique<OffsetAlgorithm>());
  }
  else {
    BOOST_THROW_EXCEPTION(AlgorithmSelectionError(this, key));
  }
}

ANLStatus AlgorithmModule::mod_begin_run()
{
  result_ = 0.0;
  total_ = 0.0;
  return AS_OK;
}

ANLStatus AlgorithmModule::mod_analyze()
{
  result_ = algorithm_->calculate(input_);
  total_ += result_;
  return AS_OK;
}

ANLStatus AlgorithmModule::mod_merge(const BasicModule* module)
{
  total_ += dynamic_cast<const AlgorithmModule&>(*module).total_;
  return AS_OK;
}
