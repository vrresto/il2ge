/**
 *    IL-2 Graphics Extender
 *    Copyright (C) 2019 Jan Lepper
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

#ifndef IL2GE_CORE_WRAPPER_MUTEX_LOCKER_H
#define IL2GE_CORE_WRAPPER_MUTEX_LOCKER_H

#include <windows.h>

namespace il2ge::core_wrapper
{

  class MutexLocker
  {
    CRITICAL_SECTION &m_mutex;

  public:
    MutexLocker(CRITICAL_SECTION &mutex) : m_mutex(mutex)
    {
      EnterCriticalSection(&m_mutex);
    }
    ~MutexLocker()
    {
      LeaveCriticalSection(&m_mutex);
    }
  };

}

#endif
