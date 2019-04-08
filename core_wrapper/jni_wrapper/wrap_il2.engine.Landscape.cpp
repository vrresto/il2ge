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
#include "meta_class_registrators.h"
#include <core.h>

using namespace jni_wrapper;

namespace
{

#include <_generated/jni_wrapper/il2.engine.Landscape_definitions>

Interface import;


jint JNICALL cPreRender(JNIEnv *env, jobject obj,
    jfloat arg0,
    jboolean arg1,
    jfloatArray arg2)
{
  const jint sun_moon_num_elements = env->GetArrayLength(arg2);
  assert(sun_moon_num_elements == 6);
  float sun_moon[sun_moon_num_elements];
  env->GetFloatArrayRegion(arg2, 0, sun_moon_num_elements, sun_moon);

  core::setSunDir(glm::normalize(glm::vec3(sun_moon[0], sun_moon[1], sun_moon[2])));
  core::onLandscapePreRender();

  return import.cPreRender(env, obj, arg0, arg1, arg2);
}

jint JNICALL cRender0(JNIEnv *env, jobject obj,
    jint arg0)
{
  core::onLandscapeRender0();
  jint ret = import.cRender0(env, obj, arg0);
  core::onLandscapeRender0Done();

  return ret;
}

jint JNICALL cRender1(JNIEnv *env, jobject obj,
    jint arg0)
{
  core::onLandscapeRender1();
  jint ret = import.cRender1(env, obj, arg0);
  core::onLandscapePostRender();

  return ret;
}

jint JNICALL cLoadMap(JNIEnv *env, jobject obj,
    jstring arg0,
    jintArray arg1,
    jint arg2,
    jboolean arg3)
{
  jsize len = env->GetStringUTFLength(arg0);
  char buf[len+1];
  env->GetStringUTFRegion(arg0, 0, len, buf);

  core::loadMap(buf, env);

  if (import.cLoadMap(env, obj, arg0, arg1, arg2, arg3))
  {
    auto pixel_map_h = core::getPixelMapH();
    assert(pixel_map_h);
    auto size = pixel_map_h->getSize();

    for (int y = 0; y < size.y; y++)
    {
      for (int x = 0; x < size.x; x++)
      {
        int pixel = pixel_map_h->get(x,y);
        import.setPixelMapH(env, obj, x, y, pixel);
      }
    }

    return true;
  }
  else
    return false;
}

jint JNICALL cUnloadMap(JNIEnv *env, jobject obj)
{
  core::unloadMap();
  return import.cUnloadMap(env, obj);
}

void JNICALL setPixelMapH(JNIEnv *env, jobject obj, jint x, jint y, jint value)
{
  return import.setPixelMapH(env, obj, x, y, value);
}


} // namespace

#include <_generated/jni_wrapper/il2.engine.Landscape_registration>
