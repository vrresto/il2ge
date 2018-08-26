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

#ifndef IL2GE_CORE_WRAPPER_H
#define IL2GE_CORE_WRAPPER_H

struct LoaderInterface;

namespace il2ge
{
  typedef void CoreWrapperInitFunc(HMODULE core_module, const LoaderInterface *loader);
  typedef void* CoreWrapperGetProcAddressFunc(const char* name);

  void *get_SFS_openf_wrapper();
}

extern "C"
{
  il2ge::CoreWrapperInitFunc il2ge_coreWrapperInit;
  il2ge::CoreWrapperGetProcAddressFunc il2ge_coreWrapperGetProcAddress;
}

#endif
