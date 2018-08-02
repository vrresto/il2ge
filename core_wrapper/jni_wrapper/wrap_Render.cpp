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

using namespace jni_wrapper;

namespace
{

#include <_generated/jni_wrapper/Render_definitions>

Interface import;

int JNICALL transform3f(JNIEnv *env, jobject obj,
    jobject arg0,
    jobject arg1)
{
  return import.transform3f(env, obj, arg0, arg1);
}

int JNICALL transform3fInv(JNIEnv *env, jobject obj,
    jobject arg0,
    jobject arg1)
{
  return import.transform3fInv(env, obj, arg0, arg1);
}

int JNICALL transformVirt3f(JNIEnv *env, jobject obj,
    jobject arg0,
    jobject arg1)
{
  return import.transformVirt3f(env, obj, arg0, arg1);
}

int JNICALL prepareStates(JNIEnv *env, jobject obj)
{
  return import.prepareStates(env, obj);
}

int JNICALL clearStates(JNIEnv *env, jobject obj)
{
  core::onClearStates();
  return import.clearStates(env, obj);
}

int JNICALL flush(JNIEnv *env, jobject obj)
{
  return import.flush(env, obj);
}

int JNICALL shadows(JNIEnv *env, jobject obj)
{
  return import.shadows(env, obj);
}

int JNICALL drawBeginTriangleLists(JNIEnv *env, jobject obj,
    jobject arg0)
{
  return import.drawBeginTriangleLists(env, obj, arg0);
}

int JNICALL drawEnd(JNIEnv *env, jobject obj)
{
  return import.drawEnd(env, obj);
}

int JNICALL drawTriangleList___3FIII_3II(JNIEnv *env, jobject obj,
    jobject arg0,
    jobject arg1,
    jobject arg2,
    jobject arg3,
    jobject arg4,
    jobject arg5)
{
  return import.drawTriangleList___3FIII_3II(env, obj, arg0, arg1, arg2, arg3, arg4, arg5);
}

int JNICALL drawTriangleList___3F_3II_3II(JNIEnv *env, jobject obj,
    jobject arg0,
    jobject arg1,
    jobject arg2,
    jobject arg3,
    jobject arg4)
{
  return import.drawTriangleList___3F_3II_3II(env, obj, arg0, arg1, arg2, arg3, arg4);
}

int JNICALL vertexArraysTransformAndLock(JNIEnv *env, jobject obj,
    jobject arg0,
    jobject arg1)
{
  return import.vertexArraysTransformAndLock(env, obj, arg0, arg1);
}

int JNICALL vertexArraysUnLock(JNIEnv *env, jobject obj)
{
  return import.vertexArraysUnLock(env, obj);
}

#include <_generated/jni_wrapper/Render_registration>

} // namespace
