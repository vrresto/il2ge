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

#ifndef CORE_CORE_P_H
#define CORE_CORE_P_H

#include "menu.h"
#include <core.h>
#include <jni.h>

namespace core
{
  class Scene;

  Il2RenderPhase getRenderPhase();

//   float getFrameDelta();
//   glm::vec4 getShoreWavePos();
//   float getWaterAnimationFrameDelta();
//   int getWaterAnimationStep();

  Scene *getScene();

  Menu &getMenu();

  void setFMBActive(bool);

  class ProgressReporter
  {
    JNIEnv *env = nullptr;
    jclass class_id {};
    jmethodID method_id {};

  public:
    ProgressReporter(JNIEnv *env);

    void report(float percent, const std::string &description, bool is_il2ge = true);
  };
}

#endif
