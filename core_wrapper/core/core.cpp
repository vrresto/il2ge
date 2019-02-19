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

#include "core_p.h"
#include <core.h>
#include <core/scene.h>
#include <wgl_wrapper.h>
#include <misc.h>
#include <jni.h>
#include <il2ge/map_loader.h>

#include <atmosphere_map.h>
#include <curvature_map.h>
#include <render_util/render_util.h>

#include <INIReader.h>
#include <functional>
#include <iostream>
#include <cassert>
#include <sys/types.h>
#include <sys/stat.h>

#include <render_util/gl_binding/gl_functions.h>

using namespace std;

namespace
{


class GameState
{
  int m_builder_id = 0;
  int m_id = 0;

public:
  GameState(JNIEnv*);
  bool isBuilder() { return m_id == m_builder_id; }
};


GameState::GameState(JNIEnv *env)
{
  auto class_id_main = env->FindClass("com/maddox/il2/game/Main");
  assert(class_id_main);
  auto method_id_main_state =
    env->GetStaticMethodID(class_id_main, "state", "()Lcom/maddox/il2/game/GameState;");
  assert(method_id_main_state);

  auto class_id_game_state = env->FindClass("com/maddox/il2/game/GameState");
  assert(class_id_game_state);
  auto method_id_game_state_id =
    env->GetMethodID(class_id_game_state, "id", "()I");
  assert(method_id_game_state_id);

  auto field_id_game_state_builder =
    env->GetStaticFieldID(class_id_game_state, "BUILDER", "I");
  assert(field_id_game_state_builder);

  m_builder_id = env->GetStaticIntField(class_id_game_state, field_id_game_state_builder);
  assert(m_builder_id);

  auto state_obj = env->CallStaticObjectMethod(class_id_main, method_id_main_state);
  assert(state_obj);

  m_id = env->CallIntMethod(state_obj, method_id_game_state_id);
  assert(m_id);
}


void refreshFile(const char *path,
                 size_t size,
                 std::function<bool(const char*)> generate_func)
{
  string core_wrapper_file = il2ge::core_wrapper::getWrapperLibraryFilePath();
  cout<<"core wrapper library: "<<core_wrapper_file<<endl;

  struct stat stat_res;

  if (stat(core_wrapper_file.c_str(), &stat_res) != 0)
  {
    assert(0);
  }

  auto program_mtime = stat_res.st_mtime;

  if (stat(path, &stat_res) == 0)
  {
    if (!(program_mtime > stat_res.st_mtime))
    {
      if (stat_res.st_size != size)
      {
        cout << path << " size mismatch." << endl;
      }
      else
      {
        cout << path << " is up to date." << endl;
        return;
      }
    }
  }

  cout << "generating " << path << " ..." << endl;

  bool success = generate_func(path);
  assert(success);
}


} // namespace


namespace il2ge::map_loader
{
  bool isDumpEnabled() { return il2ge::core_wrapper::getConfig().enable_dump; }
}


namespace core
{


void init()
{
#ifndef NO_REFRESH_MAPS
  auto res = util::mkdir(IL2GE_CACHE_DIR);
  assert(res);
  refreshFile(IL2GE_CACHE_DIR "/atmosphere_map", atmosphere_map_size_bytes, render_util::createAtmosphereMap);
  refreshFile(IL2GE_CACHE_DIR "/curvature_map", curvature_map_size_bytes, render_util::createCurvatureMap);
#endif
}


Scene *getScene()
{
  return wgl_wrapper::getScene();
}


void unloadMap()
{
  FORCE_CHECK_GL_ERROR();
  getScene()->unloadMap();
  FORCE_CHECK_GL_ERROR();

  core::setFMBActive(false);
}


void loadMap(const char *path, void *env_)
{
  FORCE_CHECK_GL_ERROR();

  ProgressReporter progress((JNIEnv*)env_);

  GameState game_state((JNIEnv*)env_);

  core::setFMBActive(game_state.isBuilder());

  if (!game_state.isBuilder())
    getScene()->loadMap(path, &progress);

  FORCE_CHECK_GL_ERROR();
}


bool isMapLoaded()
{
  return getScene()->isMapLoaded();
}

void updateUniforms(render_util::ShaderProgramPtr program)
{
  program->setUniform("terrainColor", glm::vec3(0,1,0));
  program->setUniform("sunDir", core::getSunDir());

  //FIXME
//   program->setUniform("shore_wave_scroll", core::getShoreWavePos());

  getScene()->updateUniforms(program);

  CHECK_GL_ERROR();
}


render_util::TextureManager &textureManager()
{
  return getScene()->texture_manager;
}


render_util::TerrainRenderer &getTerrainRenderer()
{
  return getScene()->getTerrainRenderer();
}


il2ge::Effect3D *getEffect(int cpp_obj)
{
  assert(getScene()->effects.get(cpp_obj));
  return getScene()->effects.get(cpp_obj);
}


void addEffect(std::unique_ptr<il2ge::Effect3D> effect, int cpp_obj)
{
  getScene()->effects.add(std::move(effect), cpp_obj);
}


bool removeEffect(int cpp_obj)
{
  return getScene()->effects.remove(cpp_obj);
}


void renderEffects()
{
  getScene()->effects.render();
}


ProgressReporter::ProgressReporter(JNIEnv *env) : env(env)
{
  class_id = env->FindClass("com/maddox/rts/BackgroundTask");
  assert(class_id);
  method_id = env->GetStaticMethodID(class_id, "step", "(FLjava/lang/String;)Z");
  assert(method_id);
}


void ProgressReporter::report(float percent, const string &description_, bool is_il2ge)
{
  string description = description_;
  if (is_il2ge)
    description += " (IL2GE)";
  env->CallStaticBooleanMethod(class_id, method_id, percent, env->NewStringUTF(description.c_str()));
}


} // namespace core
