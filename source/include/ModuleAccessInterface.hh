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

#ifndef ANLNEXT_ModuleAccessInterface_H
#define ANLNEXT_ModuleAccessInterface_H 1

#include <string>
#include "ANLException.hh"
#include "ModuleAccess.hh"

namespace anlnext
{

/**
 * A basic class for an ANL Next module.
 *
 * @author Hirokazu Odaka
 * @date 2026-06-28 | separate from ModuleAccessInterface
 */
class ModuleAccessInterface
{
public:
  ModuleAccessInterface();
  virtual ~ModuleAccessInterface();
  ModuleAccessInterface(const ModuleAccessInterface&);

  virtual void set_module_access(const ModuleAccess* module_access);

protected:
  template <typename T>
  void get_module(const std::string& name, const T** ptr) const
  { *ptr = static_cast<const T*>(module_access_->get_module(name)); }

  template <typename T>
  void get_module_NC(const std::string& name, T** ptr) const
  { *ptr = static_cast<T*>(module_access_->get_module_NC(name)); }

  template <typename T>
  const T* get_module(const std::string& name) const
  { return static_cast<const T*>(module_access_->get_module(name)); }

  template <typename T>
  T* get_module_NC(const std::string& name) const
  { return static_cast<T*>(module_access_->get_module_NC(name)); }

  template <typename T>
  void get_module_IF(const std::string& name, const T** ptr) const;

  template <typename T>
  void get_module_IFNC(const std::string& name, T** ptr) const;

  bool exist_module(const std::string& name) const
  { return module_access_->exist(name); }

  template <typename T>
  void request_module(const std::string& name, const T** ptr) const
  { *ptr = static_cast<const T*>(module_access_->request_module(name)); }

  template <typename T>
  void request_module_NC(const std::string& name, T** ptr) const
  { *ptr = static_cast<T*>(module_access_->request_module_NC(name)); }

  template <typename T>
  const T* request_module(const std::string& name) const
  { return static_cast<const T*>(module_access_->request_module(name)); }

  template <typename T>
  T* request_module_NC(const std::string& name) const
  { return static_cast<T*>(module_access_->request_module_NC(name)); }

  template <typename T>
  void request_module_IF(const std::string& name, const T** ptr) const;

  template <typename T>
  void request_module_IFNC(const std::string& name, T** ptr) const;

private:
  const ModuleAccess* module_access_ = nullptr;
};

template <typename T>
void ModuleAccessInterface::get_module_IF(const std::string& name, const T** ptr) const
{
  *ptr = dynamic_cast<const T*>(module_access_->get_module(name));
  if (*ptr==0) {
    BOOST_THROW_EXCEPTION( ModuleAccessError("Dynamic cast failed -- Module", name) );
  }
}

template <typename T>
void ModuleAccessInterface::get_module_IFNC(const std::string& name, T** ptr) const
{
  *ptr = dynamic_cast<T*>(module_access_->get_module_NC(name));
  if (*ptr==0) {
    BOOST_THROW_EXCEPTION( ModuleAccessError("Dynamic cast failed -- Module", name) );
  }
}

template <typename T>
void ModuleAccessInterface::request_module_IF(const std::string& name, const T** ptr) const
{
  const BasicModule* m = module_access_->request_module(name);
  if (m) {
    *ptr = dynamic_cast<const T*>(m);
  }
  else {
    *ptr = m;
  }
}

template <typename T>
void ModuleAccessInterface::request_module_IFNC(const std::string& name, T** ptr) const
{
  BasicModule* m = module_access_->request_module_NC(name);
  if (m) {
    *ptr = dynamic_cast<T*>(m);
  }
  else {
    *ptr = m;
  }
}

} /* namespace anlnext */

#endif /* ANLNEXT_ModuleAccessInterface_H */
