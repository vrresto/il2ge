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

#include "jni_wrapper.h"
#include <core.h>

#include <glm/glm.hpp>

using namespace jni_wrapper;

namespace
{

#include <_generated/jni_wrapper/Sun_definitions>

Interface import;

int JNICALL setNative(JNIEnv *env, jobject obj,
    jfloat arg0,
    jfloat arg1,
    jfloat arg2,
    jfloat arg3,
    jfloat arg4,
    jfloat arg5,
    jfloat arg6,
    jfloat arg7,
    jfloat arg8)
{
  core::setSunDir(glm::normalize(glm::vec3(arg0, arg1, arg2)));
  return import.setNative(env, obj, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
}

#include <_generated/jni_wrapper/Sun_registration>

} // namespace
