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

#ifndef ANL_ANLManager_impl_H
#define ANL_ANLManager_impl_H 1

#include "ANLManager.hh"
#include "BasicModule.hh"
#include "ANLException.hh"

namespace anl
{

template<typename T>
ANLStatus routine_modfn(T func,
                        const std::string& func_id,
                        const std::vector<BasicModule*>& modules)
{
  std::cout << "\n"
            << "ANLManager: starting <" << func_id << "> routine.\n"
            << std::endl;

  ANLStatus status = AS_OK;
  for (auto& mod: modules) {
    if (mod->is_off()) { continue; }
    
    try {
      status = ((*mod).*func)();
    }
    catch (ANLException& ex) {
      ex << ErrorInfoOnMethod( mod->module_name() + "::mod_" + func_id );
      ex << ErrorInfoOnModuleID( mod->module_id() );
      ex << ErrorInfoOnModuleName( mod->module_name() );
      ex << ErrorInfoOnChainID( mod->copy_id() );
      print_exception(ex);
      status = AS_QUIT_ERROR;
      break;
    }

    if (status != AS_OK ) {
      std::cout << "\n"
                << "ANLManager: <" << func_id << "> routine stopped.\n"
                << mod->module_name() << "::mod_" << func_id
                << " returned " << status << std::endl;
      break;
    }
  }
  
  if (status == AS_OK) {
    std::cout << "\n"
              << "ANLManager: <" << func_id << "> routine successfully done.\n"
              << std::endl;
  }

  return status;
}

} /* namespace anl */

#endif /* ANL_ANLManager_impl_H */
