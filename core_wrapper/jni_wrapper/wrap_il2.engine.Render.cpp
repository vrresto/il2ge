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
#include <java_classes.h>

#include <iostream>

using namespace il2ge;
using namespace jni_wrapper;
using namespace std;

namespace
{


#include <_generated/jni_wrapper/il2.engine.Render_definitions>


struct RenderWrapper;


unordered_map<string, unique_ptr<RenderWrapper>> createWrappers();


Interface import;
unordered_map<string, unique_ptr<RenderWrapper>> g_render_wrappers = createWrappers();


struct RenderWrapper
{
  virtual void flush() = 0;
};


struct RenderWrapper3D1 : public RenderWrapper
{
  void flush() override
  {
    core::onRender3D1Flush();
  }
};


unordered_map<string, unique_ptr<RenderWrapper>> createWrappers()
{
  unordered_map<string, unique_ptr<RenderWrapper>> map;

  map["render3D1"] = make_unique<RenderWrapper3D1>();

  return move(map);
}


RenderWrapper *getRenderWrapper(jobject obj)
{
  java::il2::engine::Actor actor(obj);
  return g_render_wrappers[actor.getName()].get();
}


jint JNICALL prepareStates(JNIEnv *env, jobject obj)
{
  return import.prepareStates(env, obj);
}

jint JNICALL clearStates(JNIEnv *env, jobject obj)
{
  core::onClearStates();
  return import.clearStates(env, obj);
}

jint JNICALL flush(JNIEnv *env, jobject obj)
{
  import.flush(env, obj);

  auto wrapper = getRenderWrapper(java::getClass<java::il2::engine::Renders>().current());
  if (wrapper)
    wrapper->flush();

  return 0;
}

jint JNICALL shadows(JNIEnv *env, jobject obj)
{
  return import.shadows(env, obj);
}


} // namespace

#include <_generated/jni_wrapper/il2.engine.Render_registration>
