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

#include "jni_wrapper.h"
#include "meta_class_registrators.h"
#include <core.h>
#include <misc.h>

#include <iostream>
#include <list>
#include <unordered_set>
#include <windows.h>

using namespace jni_wrapper;
using namespace std;


namespace
{

#include <_generated/jni_wrapper/il2.engine.GObj_definitions>


class MutexLocker
{
  HANDLE m_mutex;

public:
  MutexLocker(HANDLE mutex) : m_mutex(mutex)
  {
    assert(m_mutex);

    SetLastError(ERROR_SUCCESS);
    auto wait_res = WAIT_FAILED;

    while (wait_res != WAIT_OBJECT_0)
    {
      wait_res = WaitForSingleObject(m_mutex, INFINITE);
      if (wait_res != WAIT_OBJECT_0)
        printf("WaitForSingleObject() failed - wait_res: 0x%x - error: 0x%x\n", wait_res, GetLastError());
    }
  }
  ~MutexLocker()
  {
    auto release_res = ReleaseMutex(m_mutex);
    assert(release_res);
  }
};


unordered_set<jint> g_objects;
list<jint> g_garbage;
HANDLE g_mutex = 0;
Interface import;


void JNICALL Finalize(JNIEnv *env, jobject obj,
    jint arg0)
{
  MutexLocker lock(g_mutex);

  if (g_objects.find(arg0) == g_objects.end())
  {
    import.Finalize(env, obj, arg0);
  }
  else if (il2ge::core_wrapper::isMainThread())
  {
    core::removeEffect(arg0);
    g_objects.erase(arg0);
  }
  else
  {
    // we must have been called from java's garbage collector thread
    g_garbage.push_back(arg0);
  }
}


} // namespace


#include <_generated/jni_wrapper/il2.engine.GObj_registration>

namespace jni_wrapper
{


void initGObj()
{
  g_mutex = CreateMutexA(
      NULL,              // default security attributes
      FALSE,             // initially not owned
      NULL);             // unnamed mutex

  assert(g_mutex);
}


void addGObj(jint cpp_obj)
{
  MutexLocker lock(g_mutex);
  g_objects.insert(cpp_obj);
}


void cleanGarbage()
{
  MutexLocker lock(g_mutex);

  while (!g_garbage.empty())
  {
    auto obj = g_garbage.front();
    core::removeEffect(obj);
    g_garbage.pop_front();
    g_objects.erase(obj);
  }
}


} // namespace jni_wrapper
