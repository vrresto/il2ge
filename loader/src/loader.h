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

#ifndef IL2GE_LOADER_H
#define IL2GE_LOADER_H

#include <vector>
#include <fstream>
#include <windef.h>

struct Logger
{
  std::vector<std::ostream*> m_outputs;

  template <typename T>
  Logger &operator<<(T arg)
  {
    for (auto o : m_outputs)
      *o << arg;
    return *this;
  }

  void printSeparator()
  {
    *this << "-----------------------------------------------------------\n";
  }

  void flush()
  {
    for (auto o : m_outputs)
      o->flush();
  }
};

extern Logger g_log;

void printBacktrace();
void installExceptionHandler();
HMODULE getLoaderModule();
HMODULE getCoreWrapperModule();
const char *getLogFileName();

extern "C" void WINAPI il2ge_init();


#endif
