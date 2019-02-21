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

#include <glm/glm.hpp>

using namespace jni_wrapper;
using namespace glm;

namespace
{

#include <_generated/jni_wrapper/il2.fm.Wind_definitions>

Interface import;

jfloat JNICALL SetWind(JNIEnv *env, jobject obj,
    jfloat arg0,
    jfloat arg1,
    jfloat arg2,
    jfloat arg3,
    jfloat arg4)
{
  float direction = arg1;
  float speed = arg2;
  float gust = arg3;
  float turbulence = arg4;

  vec2 dir_vec {sin(radians(direction)), cos(radians(direction))};

  core::setWindSpeed(dir_vec * speed);

  return import.SetWind(env, obj, arg0, arg1, arg2, arg3, arg4);
}


} // namespace


#include <_generated/jni_wrapper/il2.fm.Wind_registration>
