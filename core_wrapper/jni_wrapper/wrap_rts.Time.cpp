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

#include <iostream>

using namespace jni_wrapper;

namespace
{

#include <_generated/jni_wrapper/rts.Time_definitions>

Interface import;

void JNICALL setSpeedReal(JNIEnv *env, jobject obj,
  jfloat arg0)
{
  import.setSpeedReal(env, obj, arg0);
}

void JNICALL setCurrent(JNIEnv *env, jobject obj,
    jlong arg0,
    jlong arg1)
{
  core::setTime(arg0);
  import.setCurrent(env, obj, arg0, arg1);
}


} // namespace


#include <_generated/jni_wrapper/rts.Time_registration>
