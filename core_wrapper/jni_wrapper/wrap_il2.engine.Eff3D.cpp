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
#include <sfs.h>
#include <il2ge/effect3d.h>
#include <il2ge/material.h>
#include <util.h>


#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#define ENABLE_PRELOAD 0

using namespace std;
using namespace glm;
using namespace il2ge;
using namespace jni_wrapper;


namespace
{


std::string resolveRelativePath(std::string base_dir, std::string path)
{
  return util::resolveRelativePathComponents(base_dir + '/' + path);
}


#include <_generated/jni_wrapper/il2.engine.Eff3D_definitions>

Interface import;


struct InitParams
{
  string param_file_name;
  vec3 pos{0};
  vec3 yaw_pitch_roll_deg {0};
};


InitParams g_init_params;
unordered_map<string, unique_ptr<Effect3DParameters>> g_param_map;
unordered_map<string, unique_ptr<ParameterFile>> g_param_file_map;
unordered_map<string, shared_ptr<const Material>> g_material_map;
ofstream g_preload_out;
bool g_preload_done = false;


const ParameterFile &getParamFile(const string &file_path)
{
  auto &file = g_param_file_map[file_path];
  if (!file)
  {
    vector<char> content;
    if (sfs::readFile(file_path, content))
    {
      try
      {
        file = make_unique<ParameterFile>(content.data(), content.size());
      }
      catch(...)
      {
        cout<<"error in parameter file: "<<file_path<<endl;
      }
    }
  }

  assert(file);
  return *file;
}


string getMaterialPath(const string &parameter_file_path)
{
  auto &params = getParamFile(parameter_file_path);
  auto &general = params.getSection("General");
  auto path = general.get("MatName");

  if (!path.empty())
  {
    return resolveRelativePath(util::getDirFromPath(parameter_file_path), path);
  }
  else
  {
    auto &class_info = params.getSection("ClassInfo");
    auto &based_on = class_info.at("BasedOn");
    auto path = resolveRelativePath(util::getDirFromPath(parameter_file_path), based_on);
    return getMaterialPath(path);
  }
}


const shared_ptr<const Material> &getMaterial(const string &parameter_file_path)
{
  auto path = getMaterialPath(parameter_file_path);

  auto &mat = g_material_map[path];
  if (!mat)
  {
    mat = std::make_shared<Material>(getParamFile(path), util::getDirFromPath(path));
  }
  return mat;
}


const Effect3DParameters &getParams(const string &file_name)
{
  auto &params = g_param_map[file_name];
  if (!params)
  {
    cout<<"getParams: "<<file_name<<endl;

#if ENABLE_PRELOAD
    if (!g_preload_out.is_open())
    {
      g_preload_out.open("il2ge_preload_out", ios_base::trunc | ios_base::binary);
    }
    g_preload_out << file_name << endl;
#endif

    auto &file = getParamFile(file_name);
    auto &class_info = file.getSection("ClassInfo");
    auto class_name = class_info.at("ClassName");

    params = il2ge::createEffect3DParameters(class_name);
    assert(params);

    auto based_on = class_info.get("BasedOn");

    if (!based_on.empty())
    {
      auto path = util::getDirFromPath(file_name);
      assert(!path.empty());
      path += '/' + based_on;

      auto &base = getParams(path);
      params->set(base);
    }

    params->loaded_from = file_name;
    // params->loaded_from_content = std::string(content.data(), content.size());
    params->applyFrom(file);
  }
  assert(params);
  return *params;
}


class Factory
{
  JNIEnv *m_env = nullptr;
  jclass m_java_class = nullptr;
  jmethodID m_constructor_id = nullptr;
  const Effect3DParameters &m_params;
  std::shared_ptr<const Material> m_material;

public:
  Factory &operator=(const Factory&) = delete;
  Factory(const Factory&) = delete;
  Factory(const Factory&&) = delete;

  Factory(const string &file_name, JNIEnv *env) :
    m_env(env),
    m_params(getParams(file_name))
  {
    auto java_class = m_env->FindClass(m_params.getJavaClassName());
    assert(java_class);
    m_java_class = (jclass) m_env->NewGlobalRef((jobject)java_class);
    m_constructor_id = m_env->GetMethodID(m_java_class, "<init>", "(I)V");
    assert(m_constructor_id);

    m_material = getMaterial(file_name);
  }

  ~Factory()
  {
    m_env->DeleteGlobalRef((jobject)m_java_class);
  }

  jobject createJavaObject(int cpp_obj) const
  {
    return m_env->NewObject(m_java_class, m_constructor_id, cpp_obj);
  }

  unique_ptr<Effect3D> createEffect() const
  {
    auto e = m_params.createEffect();
    e->material = m_material;
    return e;
  }
};


unordered_map<string, unique_ptr<Factory>> g_factory_map;

const Factory &getFactory(string name, JNIEnv *env)
{
  auto &f = g_factory_map[name];
  if (!f)
    f = make_unique<Factory>(name, env);
  return *f;
}


void preload(JNIEnv *env)
{
#if ENABLE_PRELOAD
  if (g_preload_done)
    return;
  g_preload_done = true;

  ifstream in("il2ge_preload", ios_base::binary);

  while (in.good())
  {
    string file_name;
    getline(in, file_name);
    if (!file_name.empty())
      getFactory(file_name, env);
  }
#endif
}


jobject JNICALL New(JNIEnv *env, jobject obj)
{
  preload(env);

  assert(!g_init_params.param_file_name.empty());

  auto &factory = getFactory(g_init_params.param_file_name, env);

  auto effect = factory.createEffect();
  effect->setPos(g_init_params.pos);
  effect->setYawPitchRollDeg(g_init_params.yaw_pitch_roll_deg);

  int cpp_obj = (int)effect.get();

  addGObj(cpp_obj);

  auto new_obj = factory.createJavaObject(cpp_obj);
  assert(new_obj);

  core::addEffect(move(effect), cpp_obj);

  return new_obj;
}


jint JNICALL PreRender(JNIEnv *env, jobject obj,
    jint arg0)
{
  return 0;
//   return import.PreRender(env, obj, arg0);
}

void JNICALL Render(JNIEnv *env, jobject obj, jint arg0)
{
//   core::onEff3DRender();
//   import.Render(env, obj, arg0);
//   core::onEff3DRenderDone();
}

void JNICALL SetPos(JNIEnv *env, jobject obj,
    jint arg0,
    jfloatArray arg1)
{
  auto eff = core::getEffect(arg0);
  assert(eff);

  const jint pos_num_elements = env->GetArrayLength(arg1);
  assert(pos_num_elements >= 3);

  float pos[3];
  env->GetFloatArrayRegion(arg1, 0, 3, pos);

  eff->setPos(vec3(pos[0], pos[1], pos[2]));

//   import.SetPos(env, obj, arg0, arg1);
}

void JNICALL SetAnglesATK(JNIEnv *env, jobject obj,
    jint arg0,
    jfloatArray arg1)
{
  abort();

//   auto eff = core::getEffect(arg0);
//   assert(eff);
//   return import.SetAnglesATK(env, obj, arg0, arg1);
}

void JNICALL SetXYZATK(JNIEnv *env, jobject obj,
    jint arg0,
    jfloat arg1,
    jfloat arg2,
    jfloat arg3,
    jfloat arg4,
    jfloat arg5,
    jfloat arg6)
{
  auto eff = core::getEffect(arg0);
  assert(eff);

  eff->setPos(vec3(arg1, arg2, arg3));
  eff->setRotationDeg(arg4, arg5, arg6);

//   import.SetXYZATK(env, obj, arg0, arg1, arg2, arg3, arg4, arg5, arg6);
}

void JNICALL Pause(JNIEnv *env, jobject obj,
    jint arg0,
    jboolean arg1)
{
//   import.Pause(env, obj, arg0, arg1);
  core::getEffect(arg0)->pause(arg1);
}

jboolean JNICALL IsPaused(JNIEnv *env, jobject obj,
    jint arg0)
{
//   return import.IsPaused(env, obj, arg0);
  return core::getEffect(arg0)->isPaused();
}

void JNICALL SetIntesity(JNIEnv *env, jobject obj,
    jint arg0,
    jfloat arg1)
{
//   cout<<"SetIntesity: "<<arg1<<endl;
//   import.SetIntesity(env, obj, arg0, arg1);
  core::getEffect(arg0)->setIntensity(arg1);
}

jfloat JNICALL GetIntesity(JNIEnv *env, jobject obj,
    jint arg0)
{
//   return import.GetIntesity(env, obj, arg0);
  return core::getEffect(arg0)->getIntensity();
}

void JNICALL Finish(JNIEnv *env, jobject obj,
    jint arg0)
{
//   import.Finish(env, obj, arg0);
  core::getEffect(arg0)->finish();
}

jfloat JNICALL TimeLife(JNIEnv *env, jobject obj,
    jint arg0)
{
//   return import.TimeLife(env, obj, arg0);
  return core::getEffect(arg0)->getFinishTime();
//   return -1;
}

jfloat JNICALL TimeFinish(JNIEnv *env, jobject obj,
    jint arg0)
{

//   cout<<"import.TimeLife(env, obj, arg0): "<<import.TimeLife(env, obj, arg0)<<endl;
//   cout<<"import.TimeFinish(env, obj, arg0): "<<import.TimeFinish(env, obj, arg0)<<endl;
//   return import.TimeFinish(env, obj, arg0);
  return core::getEffect(arg0)->getLifeTime();
//   return 3.5;
}

jboolean JNICALL IsTimeReal(JNIEnv *env, jobject obj,
    jint arg0)
{
//   cout<<"IsTimeReal: "<<(int)import.IsTimeReal(env, obj, arg0)<<endl;
//   return import.IsTimeReal(env, obj, arg0);
  return false;
}

void JNICALL initSetProcessTime(JNIEnv *env, jobject obj,
    jfloat arg0)
{
//   cout<<"initSetProcessTime: "<<arg0<<endl;
  import.initSetProcessTime(env, obj, arg0);
}

void JNICALL initSetTypeTimer(JNIEnv *env, jobject obj,
    jboolean arg0)
{
  import.initSetTypeTimer(env, obj, arg0);
}

void JNICALL initSetBoundBox(JNIEnv *env, jobject obj,
    jfloat arg0,
    jfloat arg1,
    jfloat arg2,
    jfloat arg3,
    jfloat arg4,
    jfloat arg5)
{
  import.initSetBoundBox(env, obj, arg0, arg1, arg2, arg3, arg4, arg5);
}

void JNICALL initSetPos(JNIEnv *env, jobject obj,
    jfloat arg0,
    jfloat arg1,
    jfloat arg2)
{
  g_init_params.pos = vec3(arg0, arg1, arg2);
  import.initSetPos(env, obj, arg0, arg1, arg2);
}

void JNICALL initSetSize(JNIEnv *env, jobject obj,
    jfloat arg0)
{
  import.initSetSize(env, obj, arg0);
}

void JNICALL initSetAnglesATK(JNIEnv *env, jobject obj,
    jfloat arg0,
    jfloat arg1,
    jfloat arg2)
{
  g_init_params.yaw_pitch_roll_deg = vec3{arg0, arg1, arg2};
  import.initSetAnglesATK(env, obj, arg0, arg1, arg2);
}

void JNICALL initSetSubType(JNIEnv *env, jobject obj,
    jint arg0)
{
  cout<<"initSetSubType: "<<arg0<<endl;
  import.initSetSubType(env, obj, arg0);
}

void JNICALL initSetMaterialName(JNIEnv *env, jobject obj,
    jstring arg0)
{
  import.initSetMaterialName(env, obj, arg0);
}

void JNICALL initSetParamFileName(JNIEnv *env, jobject obj,
    jstring arg0)
{
  jsize len = env->GetStringUTFLength(arg0);
  char buf[len+1];
  env->GetStringUTFRegion(arg0, 0, len, buf);

  g_init_params.param_file_name = buf;

  import.initSetParamFileName(env, obj, arg0);
}


} // namespace


#include <_generated/jni_wrapper/il2.engine.Eff3D_registration>
