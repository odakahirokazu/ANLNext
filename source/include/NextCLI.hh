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

/**
 * This program provide the same functions (but renamed) as the ANL CLI.
 * @file NextCLI.hh
 * @author Hirokazu Odaka
 */

#ifndef ANL_NextCLI_H
#define ANL_NextCLI_H 1

#if ANL_USE_TVECTOR
#include "TVector2.h"
#include "TVector3.h"
#endif

#if ANL_USE_HEPVECTOR
#include "CLHEP/Vector/TwoVector.h"
#include "CLHEP/Vector/ThreeVector.h"
#endif

#include "ModuleParameter.hh"

namespace anl
{

/**
 * a function for command line read.
 * @param[in] text a text for prompt
 * @param[out] ptr a pointer to a output variable
 */  
template <typename T>
void CLread(const std::string& text, T* ptr);

/**
 * a function for command line read.
 * @param[in] text a text for prompt
 * @param[out] ptr a pointer to a output variable
 * @param[in] unit unit
 * @param[in] unit_name name of the unit. This is used for the asking prompt.
 */
template <typename T>
void CLread(const std::string& text, T* ptr,
            double unit, const std::string& unit_name);

/**
 * a function for command line read.
 * @param[in] text a text for prompt
 * @param[out] ptr a pointer to a output variable
 * @param[in] expression input expression. e.g. hex.
 */
template <typename T>
void CLread(const std::string& text, T* ptr,
            const std::string& expression);

/**
 * a function for sequential command line read for a vector/list.
 * @param[in] text a text for prompt
 * @param[out] ptr a pointer to a output variable
 * @param[in] default_value default value of each element of the vector/list.
 */
template <typename T>
void CLread_seq(const std::string& text, T* ptr,
                const std::string& default_value="");


/* implementation */

template<typename T>
void CLread(const std::string& text, T* ptr)
{
  ModuleParameter<T> param(ptr, text);
  param.ask();
}

template<typename T>
void CLread(const std::string& text, T* ptr,
            double unit, const std::string& unit_name)
{
  ModuleParameter<T> param(ptr, text, unit, unit_name);
  param.ask();
}

template<typename T>
void CLread(const std::string& text, T* ptr, const std::string& expression)
  
{
  ModuleParameter<T> param(ptr, text, expression);
  param.ask();
}

template<typename T>
void CLread_seq(const std::string& text, T* ptr,
                const std::string& default_value) 
{
  ModuleParameter<T> param(ptr, text, "seq", default_value);
  param.ask();
}

}

#endif /* ANL_NextCLI_H */
