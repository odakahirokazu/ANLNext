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

#ifdef ANL_USE_TVECTOR
template <> class ModuleParameter<TVector2> : public VModuleParameter
{
public:
  ModuleParameter(TVector2* ptr, const std::string& name)
    : VModuleParameter(name), _ptr(ptr)
  {}
  
  ModuleParameter(TVector2* ptr, const std::string& name,
                  double unit, const std::string& unit_name)
    : VModuleParameter(name, unit, unit_name), _ptr(ptr)
  {}

  std::string type_name() const { return "2-vector"; }
  
  void set_value(double x, double y) { _ptr->Set(x*unit(), y*unit()); }
  
  void output(std::ostream& os) const
  {
    os << _ptr->X()/unit() << " " << _ptr->Y()/unit();
  }
  
  void input(std::istream& is)
  {
    double x(0.0), y(0.0);
    is >> x >> y;
    if (is) {
      _ptr->Set(x*unit(), y*unit());
    }
  }

private:
  TVector2* _ptr;
};


template <> class ModuleParameter<TVector3> : public VModuleParameter
{
public:
  ModuleParameter(TVector3* ptr, const std::string& name)
    : VModuleParameter(name), _ptr(ptr)
  {}
  
  ModuleParameter(TVector3* ptr, const std::string& name,
                  double unit, const std::string& unit_name)
    : VModuleParameter(name, unit, unit_name), _ptr(ptr)
  {}

  std::string type_name() const { return "3-vector"; }
  
  void set_value(double x, double y, double z)
  { _ptr->SetXYZ(x*unit(), y*unit(), z*unit()); }
  
  void output(std::ostream& os) const
  {
    os << _ptr->X()/unit() << " "
       << _ptr->Y()/unit() << " "
       << _ptr->Z()/unit();
  }

  void input(std::istream& is)
  {
    double x(0.0), y(0.0), z(0.0);
    is >> x >> y >> z;
    if (is) {
      _ptr->SetXYZ(x*unit(), y*unit(), z*unit());
    }
  }

private:
  TVector3* _ptr;
};
#endif


#ifdef ANL_USE_HEPVECTOR
template <> class ModuleParameter<CLHEP::Hep2Vector> : public VModuleParameter
{
public:
  ModuleParameter(CLHEP::Hep2Vector* ptr, const std::string& name)
    : VModuleParameter(name), _ptr(ptr)
  {}
  
  ModuleParameter(CLHEP::Hep2Vector* ptr, const std::string& name,
                  double unit, const std::string& unit_name)
    : VModuleParameter(name, unit, unit_name), _ptr(ptr)
  {}
  
  std::string type_name() const { return "2-vector"; }

  void set_value(double x, double y) { _ptr->set(x*unit(), y*unit()); }
  void output(std::ostream& os) const
  {
    os << _ptr->x()/unit() << " " << _ptr->y()/unit();
  }
  void input(std::istream& is)
  {
    double x(0.0), y(0.0);
    is >> x >> y;
    if (is) {
      _ptr->set(x*unit(), y*unit());
    }
  }
 
private:
  CLHEP::Hep2Vector* _ptr;
};


template <> class ModuleParameter<CLHEP::Hep3Vector> : public VModuleParameter
{
public:
  ModuleParameter(CLHEP::Hep3Vector* ptr, const std::string& name)
    : VModuleParameter(name), _ptr(ptr)
  {}
  
  ModuleParameter(CLHEP::Hep3Vector* ptr, const std::string& name,
                  double unit, const std::string& unit_name)
    : VModuleParameter(name, unit, unit_name), _ptr(ptr)
  {}
  
  std::string type_name() const { return "3-vector"; }

  void set_value(double x, double y, double z)
  { _ptr->set(x*unit(), y*unit(), z*unit()); }
  
  void output(std::ostream& os) const
  {
    os << _ptr->x()/unit() << " "
       << _ptr->y()/unit() << " "
       << _ptr->z()/unit();
  }

  void input(std::istream& is)
  {
    double x(0.0), y(0.0), z(0.0);
    is >> x >> y >> z;
    if (is) {
      _ptr->set(x*unit(), y*unit(), z*unit());
    }
  }

private:
  CLHEP::Hep3Vector* _ptr;
};
#endif

}
