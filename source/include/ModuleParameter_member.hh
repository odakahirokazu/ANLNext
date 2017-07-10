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

namespace anl
{

template <typename ModuleClass, typename T>
class MemberPointerPair
{
  using memptr_t = T ModuleClass::*;
public:
  MemberPointerPair(ModuleClass* o,
                    memptr_t ptr)
    : obj_(o), mem_ptr_(ptr)
  {}

  MemberPointerPair(const MemberPointerPair&) = default;
  MemberPointerPair(MemberPointerPair&&) = default;
  MemberPointerPair& operator=(const MemberPointerPair&) = default;
  MemberPointerPair& operator=(MemberPointerPair&&) = default;
 
  T& ref()
  { return obj_->*mem_ptr_; }

  const T& ref() const
  { return obj_->*mem_ptr_; }

  void set_object(ModuleClass* o)
  { obj_ = o; }

private:
  ModuleClass* obj_;
  memptr_t mem_ptr_;
};


/**
 * A class template for an ANL module parameter.
 * @author Hirokazu Odaka
 * @date 2017-07-09 | 
 */
template <typename ModuleClass, typename T>
class ModuleParameterMember : public ModuleParameter<T>, private MemberPointerPair<ModuleClass, T>
{
  using memptr_t = T ModuleClass::*;
public:
  ModuleParameterMember(const std::string& name,
                        ModuleClass* mod,
                        memptr_t ptr)
    : ModuleParameter<T>(name, nullptr),
      MemberPointerPair<ModuleClass, T>(mod, ptr)
  {}
  
  std::shared_ptr<VModuleParameter> clone() override
  { return std::shared_ptr<VModuleParameter>(new ModuleParameterMember(*this)); }

protected:
  ModuleParameterMember(const ModuleParameterMember& r) = default;

  void set_module_pointer(BasicModule* o) override
  {
    MemberPointerPair<ModuleClass, T>::set_object(dynamic_cast<ModuleClass*>(o));
    ModuleParameter<T>::set_module_pointer_of_value_info(o);
  }

protected:
  T& __ref__() override
  { return MemberPointerPair<ModuleClass, T>::ref(); }

  const T& __ref__() const override
  { return MemberPointerPair<ModuleClass, T>::ref(); }
};

} /* namespace anl */
