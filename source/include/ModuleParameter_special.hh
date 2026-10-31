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

#ifndef ANLNEXT_ModuleParameter_special_H
#define ANLNEXT_ModuleParameter_special_H 1

namespace anlnext
{

#ifdef ANLNEXT_USE_TVECTOR
template <> class ModuleParameter<TVector2> : public VModuleParameter
{
  using T = TVector2;

public:
  ModuleParameter(const std::string& name, T* ptr);

  std::shared_ptr<VModuleParameter> clone() override;

protected:
  ModuleParameter(const ModuleParameter&) = default;

public:
  std::string type_name() const override;

  void set_value(double x, double y) override;
  using VModuleParameter::set_value;

  std::vector<double> get_value(double, double) const override;
  using VModuleParameter::get_value;

  void output(std::ostream& os) const override;

  void input(std::istream& is) override;

  boost::property_tree::ptree to_property_tree() const override;

protected:
  virtual T& __ref__() { return *ptr_; }
  virtual const T& __ref__() const { return *ptr_; }

private:
  TVector2* ptr_;
};

template <> class ModuleParameter<TVector3> : public VModuleParameter
{
  using T = TVector3;

public:
  ModuleParameter(const std::string& name, T* ptr);

  std::shared_ptr<VModuleParameter> clone() override;

protected:
  ModuleParameter(const ModuleParameter&) = default;

public:
  std::string type_name() const override;

  void set_value(double x, double y, double z) override;
  using VModuleParameter::set_value;

  std::vector<double> get_value(double, double, double) const override;
  using VModuleParameter::get_value;

  void output(std::ostream& os) const override;

  void input(std::istream& is) override;

  boost::property_tree::ptree to_property_tree() const override;

protected:
  virtual T& __ref__() { return *ptr_; }
  virtual const T& __ref__() const { return *ptr_; }

private:
  TVector3* ptr_;
};

#endif /* ANLNEXT_USE_TVECTOR */

#ifdef ANLNEXT_USE_HEPVECTOR
template <> class ModuleParameter<CLHEP::Hep2Vector> : public VModuleParameter
{
  using T = CLHEP::Hep2Vector;

public:
  ModuleParameter(const std::string& name, T* ptr);

  std::shared_ptr<VModuleParameter> clone() override;

protected:
  ModuleParameter(const ModuleParameter&) = default;

public:
  std::string type_name() const override;

  void set_value(double x, double y) override;
  using VModuleParameter::set_value;

  std::vector<double> get_value(double, double) const override;
  using VModuleParameter::get_value;

  void output(std::ostream& os) const override;

  void input(std::istream& is) override;

  boost::property_tree::ptree to_property_tree() const override;

protected:
  virtual T& __ref__() { return *ptr_; }
  virtual const T& __ref__() const { return *ptr_; }

private:
  CLHEP::Hep2Vector* ptr_;
};

template <> class ModuleParameter<CLHEP::Hep3Vector> : public VModuleParameter
{
  using T = CLHEP::Hep3Vector;

public:
  ModuleParameter(const std::string& name, T* ptr);

  std::shared_ptr<VModuleParameter> clone() override;

protected:
  ModuleParameter(const ModuleParameter&) = default;

public:
  std::string type_name() const override;

  void set_value(double x, double y, double z) override;
  using VModuleParameter::set_value;

  std::vector<double> get_value(double, double, double) const override;
  using VModuleParameter::get_value;

  void output(std::ostream& os) const override;

  void input(std::istream& is) override;

  boost::property_tree::ptree to_property_tree() const override;

protected:
  virtual T& __ref__() { return *ptr_; }
  virtual const T& __ref__() const { return *ptr_; }

private:
  CLHEP::Hep3Vector* ptr_;
};

#endif /* ANLNEXT_USE_HEPVECTOR */

} /* namespace anlnext */

#endif /* ANLNEXT_ModuleParameter_special_H */
