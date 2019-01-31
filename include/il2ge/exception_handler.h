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

#ifndef IL2GE_EXCEPTION_HANDLER_H
#define IL2GE_EXCEPTION_HANDLER_H


#include <functional>
#include <string>
#include <windef.h>


namespace il2ge::exception_handler
{
  void install(const std::string &log_file_name,
               std::function<void(const char*)> fatal_error_handler =  {});
  void watchModule(HMODULE module);
}


#endif
