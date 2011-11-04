/**
 * @file NextCLI.hh
 * @author Hirokazu Odaka
 */

#ifndef NextCLI_hh
#define NextCLI_hh

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
            double unit=1.0, const std::string& unit_name="")
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

#endif // NextCLI_hh
