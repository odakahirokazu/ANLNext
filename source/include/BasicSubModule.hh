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

#ifndef ANLNEXT_BasicSubModule_H
#define ANLNEXT_BasicSubModule_H 1

#include "ParameterRegistry.hh"
#include "ModuleDescription.hh"
#include "EvsInterface.hh"
#include "ANLStatus.hh"
#include "ANLException.hh"
#include "ANLMacro.hh"

namespace anlnext
{

/**
 * A basic class for an ANL Next module.
 *
 * @author Hirokazu Odaka
 * @date 2026-06-24
 */
class BasicSubModule : public ParameterRegistry, public ModuleDescription, public EvsInterface
{
private:
  virtual std::string __submodule_name__() const
  { return "BasicSubModule"; }
  virtual std::string __submodule_version__() const
  { return "0.0"; }
  virtual std::unique_ptr<BasicSubModule> __clone__() const
  { return std::unique_ptr<BasicSubModule>(nullptr); }
  virtual std::unique_ptr<BasicSubModule> __duplicate__()
  { return std::unique_ptr<BasicSubModule>(nullptr); }
  virtual BasicSubModule* __this_ptr__() { return this; }

public:
  BasicSubModule();
  virtual ~BasicSubModule();

  BasicSubModule(BasicSubModule&& r) = delete;
  BasicSubModule& operator=(const BasicSubModule& r) = delete;
  BasicSubModule& operator=(BasicSubModule&& r) = delete;

  virtual std::unique_ptr<BasicSubModule> duplicate();

protected:
  BasicSubModule(const BasicSubModule& r) = default;

public:
  std::string submodule_name() const { return __submodule_name__(); }
  std::string submodule_version() const { return __submodule_version__(); }
   
  virtual void define_parameters() {}

  boost::property_tree::ptree parameters_to_property_tree() const override final;

protected:
  template <typename ModuleType>
  std::unique_ptr<BasicSubModule> help_duplication(std::unique_ptr<ModuleType>&& cloned);

  std::unique_ptr<BasicSubModule> clone() const;

private:
  void add_parameter_error_info(ANLException& ex) const override final;
};

template <typename ModuleType>
std::unique_ptr<BasicSubModule> BasicSubModule::help_duplication(std::unique_ptr<ModuleType>&& cloned)
{
  cloned->copy_parameters(*this);
  return cloned;
}

} /* namespace anlnext */

#endif /* ANLNEXT_BasicSubModule_H */
