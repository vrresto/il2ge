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
  virtual void prepareStates() = 0;
  virtual void flushBegin() = 0;
  virtual void flushEnd() = 0;

  bool clearStatesOnFlush() { return m_clear_states_on_flush; }

  RenderWrapper(bool clear_states_on_flush) : m_clear_states_on_flush(clear_states_on_flush) {}

private:
  const bool m_clear_states_on_flush = false;
};


struct RenderWrapper3D1 : public RenderWrapper
{
  RenderWrapper3D1() : RenderWrapper(true) {}

  void prepareStates() override {}

  void flushBegin() override
  {
    core::onRender3D1FlushBegin();
  }

  void flushEnd() override
  {
    core::onRender3D1FlushEnd();
  }
};


struct RenderWrapperCockpit: public RenderWrapper
{
  RenderWrapperCockpit() : RenderWrapper(false) {}

  void prepareStates() override
  {
    core::onRenderCockpitBegin();
  }

  void flushBegin() override {}

  void flushEnd() override
  {
    core::onRenderCockpitEnd();
  }
};


struct RenderWrapperSpritesFog: public RenderWrapper
{
  RenderWrapperSpritesFog() : RenderWrapper(false) {}

  void prepareStates() override
  {
    core::onRenderSpritesFogBegin();
  }

  void flushBegin() override {}

  void flushEnd() override
  {
    core::onRenderSpritesFogEnd();
  }
};


unordered_map<string, unique_ptr<RenderWrapper>> createWrappers()
{
  unordered_map<string, unique_ptr<RenderWrapper>> map;

  map["render3D1"] = make_unique<RenderWrapper3D1>();
  map["renderCockpit"] = make_unique<RenderWrapperCockpit>();
  map["renderSpritesFog"] = make_unique<RenderWrapperSpritesFog>();

  return move(map);
}


RenderWrapper *getRenderWrapper(jobject obj)
{
  if (obj)
  {
    java::il2::engine::Actor actor(obj);
    return g_render_wrappers[actor.getName()].get();
  }
  else
  {
    return nullptr;
  }
}


jint JNICALL prepareStates(JNIEnv *env, jobject obj)
{
  auto ret = import.prepareStates(env, obj);

  auto wrapper = getRenderWrapper(java::getClass<java::il2::engine::Renders>().current());
  if (wrapper)
    wrapper->prepareStates();

  return ret;
}


jint JNICALL clearStates(JNIEnv *env, jobject obj)
{
  return import.clearStates(env, obj);
}


jint JNICALL flush(JNIEnv *env, jobject obj)
{
  auto wrapper = getRenderWrapper(java::getClass<java::il2::engine::Renders>().current());

  if (wrapper && wrapper->clearStatesOnFlush())
    import.clearStates(env, obj);

  if (wrapper)
    wrapper->flushBegin();

  auto ret = import.flush(env, obj);

  if (wrapper)
  {
    if (wrapper->clearStatesOnFlush())
      import.clearStates(env, obj);

    wrapper->flushEnd();
  }

  return ret;
}


jint JNICALL shadows(JNIEnv *env, jobject obj)
{
  return import.shadows(env, obj);
}


} // namespace

#include <_generated/jni_wrapper/il2.engine.Render_registration>
