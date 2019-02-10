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
#include <render_util/camera.h>

#include <iostream>

using namespace std;
using namespace jni_wrapper;


namespace
{

#include <_generated/jni_wrapper/il2.engine.Camera_definitions>

Interface import;

int JNICALL SetCameraPos(JNIEnv *env, jobject obj, jdoubleArray arg0)
{
  if (core::getCameraMode() == core::IL2_CAMERA_MODE_3D)
  {
    const int pos_num_elements = env->GetArrayLength(arg0);
    assert(pos_num_elements > 6);
    double pos[pos_num_elements];
    env->GetDoubleArrayRegion(arg0, 0, pos_num_elements, pos);

    core::getCamera()->setTransform(pos[0], pos[1], pos[2], pos[3] + 360, pos[4], pos[5]);
  }

  return import.SetCameraPos(env, obj, arg0);
}

int JNICALL SetOrtho2D(JNIEnv *env, jobject obj,
                       jobject arg0,
                       jobject arg1,
                       jobject arg2,
                       jobject arg3,
                       jobject arg4,
                       jobject arg5)
{
  core::setCameraMode(core::IL2_CAMERA_MODE_2D);

  return import.SetOrtho2D(env, obj, arg0, arg1, arg2, arg3, arg4, arg5);
}

int JNICALL SetViewportCrop(JNIEnv *env, jobject obj,
                            jfloat arg0,
                            jint arg1,
                            jint arg2,
                            jint arg3,
                            jint arg4,
                            jint arg5,
                            jint arg6,
                            jint arg7,
                            jint arg8,
                            jint arg9,
                            jint arg10)
{
  core::getCamera()->setViewportSize(arg1, arg2);
  return import.SetViewportCrop(env, obj, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
}

int JNICALL SetFOV(JNIEnv *env, jobject obj, jfloat arg0, jfloat arg1, jfloat arg2)
{
  core::setCameraMode(core::IL2_CAMERA_MODE_3D);
  core::getCamera()->setProjection(arg0, arg1, arg2);
  return import.SetFOV(env, obj, arg0, arg1, arg2);
}

#define ADD_METHOD(m) meta_class.addMethod<m##_t>(#m, &import.m, &m)


} // namespace


void jni_wrapper::registrator::il2::engine::Camera(MetaClass &meta_class)
{
  meta_class.package = "com.maddox.il2.engine";
  meta_class.name = "Camera";

  ADD_METHOD(SetCameraPos);
  ADD_METHOD(SetOrtho2D);
  ADD_METHOD(SetViewportCrop);
  ADD_METHOD(SetFOV);
}
