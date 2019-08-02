/**
 *    IL-2 Graphics Extender
 *    Copyright (C) 2018 Jan Lepper
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "core_wrapper.h"
#include "wgl_wrapper.h"
#include <sfs.h>
#include <misc.h>
#include <core.h>
#include <render_util/render_util.h>
#include <log.h>


#include <iostream>
#include <unordered_map>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <csignal>
#include <memory>

#include <windef.h>
#include <winbase.h>


namespace
{


// typedef int __stdcall isCubeUpdated_T(void*, vo403id*);


bool g_initialized = false;
HMODULE g_core_module = 0;
// isCubeUpdated_T *is_cube_updated_func = nullptr;


} // namespace


void il2ge::core_wrapper::init(HMODULE core_module_)
{
  LOG_INFO << "*** il2_core wrapper initialisation ***\n";

  g_core_module = core_module_;
  assert(g_core_module);

  sfs::init();
  wgl_wrapper::init();
  core_gl_wrapper::init();
  core::init();

//   is_cube_updated_func = (isCubeUpdated_T*) GetProcAddress(g_core_module,
//                  "_Java_com_maddox_il2_engine_Landscape_cIsCubeUpdated@8");
//   assert(is_cube_updated_func);

  g_initialized = true;

  LOG_INFO << "*** il2_core wrapper initialisation finished ***\n";
}
