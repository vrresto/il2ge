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

#undef NDEBUG

#include "core.h"
#include "misc.h"
#include "gl_wrapper.h"
#include "gl_wrapper_private.h"
#include <wgl_wrapper.h>
#include <render_util/shader.h>
#include <render_util/texunits.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <GL/gl.h>
#include <GL/glext.h>

#include <render_util/gl_binding/gl_functions.h>

using std::cout;
using std::cerr;
using std::endl;
using std::string;

using namespace core;
using namespace render_util::gl_binding;


namespace
{

using Context = core_gl_wrapper::arb_program::Context::Impl;

Context &getContext();


std::unordered_set<std::string> enabled_shaders;

constexpr const bool is_replace_arb_program_string_enabled = true;


const char replacement_path[] = "il2ge_shader_override";
const char main_shader_path[] = IL2GE_DATA_DIR "/shaders";


render_util::ShaderProgramPtr createGLSLProgram(const string &vertex_shader,
                                                const string &fragment_shader)
{
  using namespace std;

  cout<<"creating program: "<<vertex_shader<<", "<<fragment_shader<<endl;

  vector<string> vert;
  vector<string> frag;

  vert.push_back(vertex_shader);
  vert.push_back("main");
  frag.push_back(fragment_shader);
  frag.push_back("atmosphere");

  vector<string> paths;
  paths.push_back(replacement_path);
  paths.push_back(main_shader_path);

  auto program = make_shared<render_util::ShaderProgram>(fragment_shader, vert, frag, paths, false);

  CHECK_GL_ERROR();

  if (!program->isValid())
    return make_shared<render_util::ShaderProgram>();

  for (int i = 0; i < 20; i++)
  {
    char tmp[100];
    snprintf(tmp, sizeof(tmp), "sampler_%d", i);
    program->setUniformi(tmp, i);
  }

  //FIXME
  program->setUniformi("sampler_atmosphere_thickness_map", core::textureManager().getHighestUnit() + 1);

  auto vertex_uniform_block_index =
    gl::GetUniformBlockIndex(program->getId(), "local_vertex_parameters");
  if (vertex_uniform_block_index != GL_INVALID_INDEX)
  {
    gl::UniformBlockBinding(program->getId(), vertex_uniform_block_index, 0);
//     GLint data_size = 0;
//     gl::GetActiveUniformBlock(program->getId(), vertex_uniform_block_index,
//                               GL_UNIFORM_BLOCK_DATA_SIZE, &data_size);
  }


  gl::GetError();

  CHECK_GL_ERROR();


  cout<<"created program: "<<vertex_shader<<", "<<fragment_shader<<endl;

  return program;
}


struct ShaderHashTableEntry
{
  const char *name;
  unsigned long hash_value;
};

ShaderHashTableEntry shader_hashes[] =
{
  {"fpWaterSunLightFast", 2817968272},
  {"fpWaterSunLight", 632849162},
  {"fpWaterSunLightBest", 3531821305},
  {"fpCoastBump", 1124432921},
  {"fpCoastFoam", 2644930622},
  {"fpCoastFoamFast", 1435349844},
  {"fpCoastFoamFarFogTex", 314201521},
  {"fpCausticSimple", 3479977242},
  {"fpCaustic", 2728617329},
  {"fpSprites", 4119988601},
  {"fpObjectsL0", 3801593370},
  {"fpObjectsL0_2L", 991434619},
  {"fpSimpleGL", 3540320514},
  {"fpNearLandFog", 87025808},
  {"fpFarLandFog", 2232994666},
  {"fpRiverCoastAA", 3741666452},
  {"fpWaterDM_CPU", 1594865371},
  {"fpWaterDM_CPULo", 155489676},
  {"fpWaterLFogDM", 281904700},
  {"fpIceWater", 243049986},
  {"fpNearNoBlend", 415116501},
  {"fpNearNoBlendNoise", 1442106534},
  {"fpNearBlend", 1574944565},
  {"fpNearBlendNoise", 356709871},
  {"fpFarBlend", 2313301566},
  {"fpForestPlane", 4206698854},
  {"fpForestPlaneNoise", 1419226297},
  {"fpForestPlaneEdges", 4075197594},
  {"fpForestPlaneEdgesNoise", 109324726},
  {"vpFogFar2Tex2D", 3531779195},
  {"vpFog2Tex2DBlend", 2358603363},
  {"vpFogFar4Tex2D", 892403915},
  {"vpFogFar8Tex2D", 2387820957},
  {"vpFogNoTex", 462566402},
  {"vpFog4Tex2D", 3557282540},
  {"vpFog4Tex2D_UV2", 3947551348},
  {"vp4Tex2D", 3895209446},
  {"vp6Tex2D", 3003880186},
  {"vpTexUVTex2D", 2144006323},
  {"vpWaterGrid_NV", 3771374482},
  {"vpWaterSunLight_NV", 604803814},
  {"vpWaterSunLight_ATI", 2350930377},
  {"vpWaterSunLight_FP", 2803049495},
  {"vpTreeSprite", 2721248406},
  {"vpTreeTrunk", 2545682697},
  {"vpVAObjectsN", 1914100752},
  {"vpVAObjectsL0", 1135272293},
  {"vpSprites", 331927207},
  {"vpSimpleGL", 3662837049},

  // optimized variants
  {"fpWaterSunLightFast", 977067007},
  {"fpWaterSunLight", 2131239042},
  {"fpWaterSunLightBest", 4062392817},
  {"fpCoastBump", 1804904102},
  {"fpCoastFoam", 1083737417},
  {"fpCoastFoamFast", 3738753912},
  {"fpCoastFoamFarFogTex", 3120575906},
  {"fpCausticSimple", 292955408},
  {"fpCaustic", 1454717634},
  {"fpSprites", 1218231756},
  {"fpObjectsL0", 3814157290},
  {"fpObjectsL0_2L", 251922401},
  {"fpNearLandFog", 65822550},
  {"fpFarLandFog", 1339223649},
  {"vpWaterDM_GPU", 2334360762},
  {"vpWaterDM_GPU8800", 3267036497},
  {"fpCoastFoam8800", 1256648043},
  {"fpCoastFoamFarFogTex8800", 527202397},
  {"fpCoastBump8800", 1527700593},
  {"vpWaterDM_CPU", 2612379814},
  {"fpWaterNearDM", 2725810045},
  {"fpWaterMiddleDM", 1160002433},
  {"fpWaterFarDM", 2519986775},
  {"fpWaterDM_CPU", 4087157168},
  {"fpWaterDM_CPULo", 3355252201},
  {"fpWaterNearDM8800", 4056581137},
  {"fpWaterMiddleDM8800", 1621302650},
  {"fpWaterFarDM8800", 645514079},
  {"fpWaterLFogDM8800", 2197871837},
  {"fpIceWater", 1895331007},
  {"fpNearNoBlend", 1157933730},
  {"fpNearNoBlendNoise", 1634443498},
  {"fpNearBlend", 4033623987},
  {"fpNearBlendNoise", 3765832238},
  {"fpFarBlend", 2375977440},
  {"fpForestPlane", 2772098296},
  {"fpForestPlaneNoise", 2365797049},
  {"fpForestPlaneEdges", 1926783243},
  {"fpForestPlaneEdgesNoise", 162196373},
};
enum
{
  NUM_SHADER_HASHES = (sizeof(shader_hashes) / sizeof(ShaderHashTableEntry))
};


std::vector<std::string> g_shader_names;

size_t getShaderNameID(const std::string &name)
{
  for (size_t i = 0; i < g_shader_names.size(); i++)
  {
    if (name == g_shader_names[i])
      return i + 1;
  }
  g_shader_names.push_back(name);
  return g_shader_names.size();
}


class LocalParameters
{
  struct Parameter
  {
    glm::vec4 value;
    bool is_set = false;
  };

  struct ParameterValue
  {
    GLfloat x = 0;
    GLfloat y = 0;
    GLfloat z = 0;
    GLfloat w = 0;
  };

  std::vector<Parameter> params;
  bool needs_update = false;
  GLuint uniform_buffer_id = 0;
  std::vector<ParameterValue> uniform_buffer_content;

  Parameter &getParameter(size_t index)
  {
    if (params.size() < index+1)
      params.resize(index+1);
    return params[index];
  }

  ParameterValue &getUniformBufferElement(size_t index)
  {
    if (uniform_buffer_content.size() < index+1)
      uniform_buffer_content.resize(index+1);
    return uniform_buffer_content[index];
  }

  size_t getUniformBufferSize()
  {
    return uniform_buffer_content.size() * sizeof(ParameterValue);
  }

  void *getUniformBufferData()
  {
    return (void*) uniform_buffer_content.data();
  }

public:
  ~LocalParameters()
  {
    if (uniform_buffer_id)
      gl::DeleteBuffers(1, &uniform_buffer_id);
  }

  GLuint getUniformBufferID()
  {
    if (!uniform_buffer_id)
      gl::GenBuffers(1, &uniform_buffer_id);
    assert(uniform_buffer_id);
    return uniform_buffer_id;
  }

  bool needsUpdate() { return needs_update; }

  void set(size_t index, const glm::vec4 &value)
  {
    auto &p = getParameter(index);
    if (!p.is_set || (value != p.value))
      needs_update = true;
    p.value = value;
    p.is_set = true;

    getUniformBufferElement(index) = {value.x, value.y, value.z, value.w};
  }

  void updateUniformBuffer()
  {
    gl::BindBuffer(GL_UNIFORM_BUFFER, getUniformBufferID());
    gl::BufferData(GL_UNIFORM_BUFFER, getUniformBufferSize(), nullptr, GL_STREAM_DRAW);
    gl::BufferData(GL_UNIFORM_BUFFER, getUniformBufferSize(), getUniformBufferData(), GL_STREAM_DRAW);
    gl::BindBuffer(GL_UNIFORM_BUFFER, 0);
    needs_update = false;
  }

};


enum ProgramState
{
  PROGRAM_STATE_NEW,
  PROGRAM_STATE_SOURCE_SET,
};


struct ProgramBase
{
  ProgramState state = PROGRAM_STATE_NEW;
  GLenum shader_stage = 0;
  GLuint real_id = 0;
  GLenum target = 0;
  Context *context = 0;
  size_t name_id = 0;

  std::string name;
  std::string file_extension;
  render_util::ShaderProgramPtr default_glsl_program;
  LocalParameters params;

  ProgramBase(Context *ctx) : context(ctx) {}

  bool isFragmentProgram() const
  {
    return target == GL_FRAGMENT_PROGRAM_ARB;
  }

  size_t getNameID()
  {
    if (!name_id)
    {
      assert(!name.empty());
      name_id = getShaderNameID(name);
    }
    return name_id;
  }

  render_util::ShaderProgramPtr getDefaultGLSLProgram()
  {
    if (!default_glsl_program)
    {
      assert(!name.empty());
      if (name == "vpTreeSprite")
      {
        default_glsl_program = createGLSLProgram(name, "fpTreeSprite");
      }
      else if (name == "vpTreeTrunk")
      {
        default_glsl_program = createGLSLProgram(name, "fpTreeTrunk");
      }
      else if (name == "vpTexUVTex2D")
      {
        default_glsl_program = createGLSLProgram(name, "fpTexUVTex2D");
      }
      else if (name == "vp4Tex2D")
      {
        default_glsl_program = createGLSLProgram(name, "fp4Tex2D");
      }
      else if (name == "vpFogFar2Tex2D")
      {
        default_glsl_program = createGLSLProgram(name, "fpFogFar2Tex2D");
      }
      else
      {
        cout<<"no default program for "<<name<<endl;
        default_glsl_program = std::make_shared<render_util::ShaderProgram>();
      }
    }
    return default_glsl_program;
  }
};


struct FragmentProgram : public ProgramBase
{
  std::unordered_map<size_t, render_util::ShaderProgramPtr> glsl_program_for_vertex_shader;
  bool is_object_program = false;

  FragmentProgram(Context *ctx) : ProgramBase(ctx) {}

  render_util::ShaderProgramPtr getGLSLProgram(ProgramBase *vertex_program)
  {
    auto it = glsl_program_for_vertex_shader.find(vertex_program->getNameID());
    if (it != glsl_program_for_vertex_shader.end())
    {
      return it->second;
    }
    else
    {
      auto program = createGLSLProgram(vertex_program->name, name);
      glsl_program_for_vertex_shader[vertex_program->getNameID()] = program;
      return program;
    }
  }
};


unsigned long myHashFunc(const void *str, size_t len)
{
  unsigned long hash = 5381;

  for (int i = 0; i < len; i++)
  {
    unsigned char c = ((const unsigned char*)str)[i];
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }

  return hash;
}


const char * getShaderName(const void *source, size_t len)
{
  unsigned long hash_value = myHashFunc(source, len);

  const char *name = 0;

  for (int i = 0; i < NUM_SHADER_HASHES; i++)
  {
    if (shader_hashes[i].hash_value != hash_value)
      continue;

    name = shader_hashes[i].name;
    break;
  }

  if (!name)
  {
    printf("unknown shader: %lu\n", hash_value);
    assert(0);
    abort();
  }

  return name;
}


} // namespace


struct core_gl_wrapper::arb_program::Context::Impl
{
  ProgramBase *active_vertex_program = 0;
  FragmentProgram *active_fragment_program = 0;
  bool is_vertex_program_enabled = false;
  bool is_fragment_program_enabled = false;
  bool program_needs_update = false;

  std::unordered_map<GLuint, std::unique_ptr<ProgramBase>> programs;

  void enableVertexProgram(int enable)
  {
    assert(&::getContext() == this);

    if (is_vertex_program_enabled == enable)
      return;

    is_vertex_program_enabled = enable;

    program_needs_update = true;
  }

  void enableFragmentProgram(int enable)
  {
    assert(&::getContext() == this);

    if (is_fragment_program_enabled == enable)
      return;

    is_fragment_program_enabled = enable;

    program_needs_update = true;
  }

  void createProgram(GLint id, GLenum target)
  {
    assert(&::getContext() == this);

    printf("createProgram: %d, %d\n", id, target);

    std::unique_ptr<ProgramBase> p;
    if (target == GL_VERTEX_PROGRAM_ARB) {
      p = std::make_unique<ProgramBase>(this);
      p->shader_stage = GL_VERTEX_SHADER;
      p->file_extension = "vert";
    }
    else if(target == GL_FRAGMENT_PROGRAM_ARB) {
      p = std::make_unique<FragmentProgram>(this);
      p->shader_stage = GL_FRAGMENT_SHADER;
      p->file_extension = "frag";
    }
    else {
      assert(0);
      abort();
    }

    p->real_id = id;
    p->target = target;

    programs.insert(std::make_pair(id, std::move(p)));
  }

  ProgramBase *programForID(GLint id, GLenum target)
  {
    assert(&::getContext() == this);

    ProgramBase *p = 0;

    auto it = programs.find(id);
    if (it != programs.end())
    {
      p = it->second.get();
    }

    if (!p)
    {
      assert(target);
      createProgram(id, target);
      p = programs[id].get();
    }

    assert(!target || p->target == target);

    return p;
  }

  ProgramBase *getActiveProgram(GLenum target)
  {
    assert(&::getContext() == this);

    ProgramBase *p = 0;

    if (target == GL_VERTEX_PROGRAM_ARB)
      p = active_vertex_program;
    else if (target == GL_FRAGMENT_PROGRAM_ARB)
      p = active_fragment_program;
    else {
      printf("invalid target.\n");
      exit(0);
    }

    assert(p);

    return p;
  }

  void deleteProgram(GLuint id)
  {
    assert(&::getContext() == this);

    ProgramBase *p = programForID(id, 0);
    assert(p);

    if (p->target == GL_VERTEX_PROGRAM_ARB) {
      if (active_vertex_program == p)
        active_vertex_program = 0;
    }
    else if(p->target == GL_FRAGMENT_PROGRAM_ARB) {
      if (active_fragment_program == p)
        active_fragment_program = 0;
    }
    else {
      assert(0);
      abort();
    }

    programs.erase(id);

    program_needs_update = true;
  }

  void bindProgram(GLenum target, GLuint id)
  {
    assert(&::getContext() == this);

    ProgramBase *p = programForID(id, target);
    if (!p)
    {
      assert(0);
      abort();
    }

    assert(p->target == target);

    gl::BindProgramARB(target, p->real_id);

    assert(target == GL_VERTEX_PROGRAM_ARB || target == GL_FRAGMENT_PROGRAM_ARB);

    if (target == GL_VERTEX_PROGRAM_ARB)
    {
      active_vertex_program = p;
    }
    else if(target == GL_FRAGMENT_PROGRAM_ARB)
    {
      active_fragment_program = static_cast<FragmentProgram*>(p);
    }
    else
    {
      assert(0);
      abort();
    }

    program_needs_update = true;
  }

  void updateLocalParameters(ProgramBase *program)
  {
    if (program->params.needsUpdate())
      program->params.updateUniformBuffer();
  }

  void updateLocalParameters()
  {
    if (active_vertex_program)
      updateLocalParameters(active_vertex_program);
    if (active_fragment_program)
      updateLocalParameters(active_fragment_program);
  }

  void bindUniformBuffers()
  {
    if (active_vertex_program)
      gl::BindBufferBase(GL_UNIFORM_BUFFER, 0, active_vertex_program->params.getUniformBufferID());
    if (active_fragment_program)
      gl::BindBufferBase(GL_UNIFORM_BUFFER, 1, active_fragment_program->params.getUniformBufferID());
    gl::BindBuffer(GL_UNIFORM_BUFFER, 0);
  }

  void updateProgram()
  {
    if (!program_needs_update)
      return;

    assert(&::getContext() == this);

    bool is_arb_program_active = false;

    render_util::ShaderProgramPtr glsl_program;

    if (is_fragment_program_enabled && active_fragment_program ||
        is_vertex_program_enabled && active_vertex_program)
    {
      is_arb_program_active = true;

      if (is_fragment_program_enabled && active_fragment_program)
      {
        if (is_vertex_program_enabled && active_vertex_program)
        {
          glsl_program = active_fragment_program->getGLSLProgram(active_vertex_program);
        }
        else
        {
          glsl_program = active_fragment_program->getDefaultGLSLProgram();
        }
      }
      else if (is_vertex_program_enabled && active_vertex_program)
      {
        glsl_program = active_vertex_program->getDefaultGLSLProgram();
      }
    }

    core_gl_wrapper::setIsARBProgramActive(is_arb_program_active);

    if (glsl_program && glsl_program->isValid())
    {
      core_gl_wrapper::setActiveARBProgram(glsl_program);
      core_gl_wrapper::updateUniforms(glsl_program);
      glsl_program->assertUniformsAreSet();
    }
    else
    {
      core_gl_wrapper::setActiveARBProgram(nullptr);
    }

    bindUniformBuffers();

    program_needs_update = false;
  }

  void update()
  {
    updateProgram();
    updateLocalParameters();
  }

  bool isObjectProgramActive()
  {
    if (is_fragment_program_enabled && active_fragment_program)
        return active_fragment_program->is_object_program ;
    else
      return false;
  }

};


core_gl_wrapper::arb_program::Context::Context() :
  impl(std::make_unique<core_gl_wrapper::arb_program::Context::Impl>())
{
}

core_gl_wrapper::arb_program::Context::~Context() {}


namespace
{


Context &getContext()
{
  return *core_gl_wrapper::getContext()->getARBProgramContext()->impl.get();
}


ProgramBase *getActiveProgram(GLenum target)
{
  return getContext().getActiveProgram(target);
}


////////////////////////////////////////////////////////////////////////////


void GLAPIENTRY
wrap_GenProgramsARB(GLsizei n, GLuint *ids)
{
  gl::GenProgramsARB(n, ids);
}

void GLAPIENTRY
wrap_DeleteProgramsARB(GLsizei n, const GLuint *ids)
{
  for (GLsizei i = 0; i < n; i++)
  {
    getContext().deleteProgram(ids[i]);
  }
  gl::DeleteProgramsARB(n, ids);
}

GLboolean GLAPIENTRY
wrap_IsProgramARB(GLuint id)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_BindProgramARB(GLenum target, GLuint id)
{
  if (wgl_wrapper::isMainContextCurrent())
    getContext().bindProgram(target, id);
  else
    gl::BindProgramARB(target, id);
}

void GLAPIENTRY
wrap_ProgramStringARB(GLenum target, GLenum format, GLsizei len,
                       const GLvoid *string)
{
  ProgramBase *p = getActiveProgram(target);
  assert(p);

  if (p->state != PROGRAM_STATE_NEW)
  {
    printf("state %d\n", p->state);
    exit(0);
  }

  assert(p->state == PROGRAM_STATE_NEW);
  assert(p->target == target);

  p->state = PROGRAM_STATE_SOURCE_SET;

  const void *replacement_string = string;
  size_t replacement_len = len;

  gl::ProgramStringARB(target, format, replacement_len, replacement_string);

  const char *name = getShaderName(string, len);

  if (name)
  {
    p->name = name;
    if (p->isFragmentProgram())
    {
      static_cast<FragmentProgram*>(p)->is_object_program =
        (p->name == "fpObjectsL0_2L" || p->name == "fpObjectsL0");
    }
  }
}

void GLAPIENTRY
wrap_ProgramLocalParameter4fARB(GLenum target, GLuint index,
                                 GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
  gl::ProgramLocalParameter4fARB(target, index, x, y, z, w);

  ProgramBase *p = getActiveProgram(target);
  assert(p);

  glm::vec4 value(x,y,z,w);

  p->params.set(index, value);
}


void GLAPIENTRY
wrap_ProgramLocalParameter4fvARB(GLenum target, GLuint index,
                                  const GLfloat *params)
{
  wrap_ProgramLocalParameter4fARB(target, index, params[0], params[1], params[2], params[3]);
}

void GLAPIENTRY
wrap_ProgramEnvParameter4dARB(GLenum target, GLuint index,
                               GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_ProgramEnvParameter4dvARB(GLenum target, GLuint index,
                                const GLdouble *params)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_ProgramEnvParameter4fARB(GLenum target, GLuint index,
                               GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_ProgramEnvParameter4fvARB(GLenum target, GLuint index,
                                const GLfloat *params)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_ProgramEnvParameters4fvEXT(GLenum target, GLuint index, GLsizei count,
                                 const GLfloat *params)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_ProgramLocalParameter4dARB(GLenum target, GLuint index,
                                 GLdouble x, GLdouble y,
                                 GLdouble z, GLdouble w)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_ProgramLocalParameter4dvARB(GLenum target, GLuint index,
                                  const GLdouble *params)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_ProgramLocalParameters4fvEXT(GLenum target, GLuint index, GLsizei count,
                                   const GLfloat *params)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_GetProgramEnvParameterdvARB(GLenum target, GLuint index,
                                  GLdouble *params)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_GetProgramEnvParameterfvARB(GLenum target, GLuint index,
                                  GLfloat *params)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_GetProgramLocalParameterdvARB(GLenum target, GLuint index,
                                    GLdouble *params)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_GetProgramLocalParameterfvARB(GLenum target, GLuint index,
                                    GLfloat *params)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_GetProgramivARB(GLenum target, GLenum pname, GLint *params)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_GetProgramStringARB(GLenum target, GLenum pname, GLvoid *string)
{
  assert(0);
  abort();
}


void setEnabled(GLenum cap, bool enable)
{
  if (cap == GL_FRAGMENT_PROGRAM_ARB)
  {
    getContext().enableFragmentProgram(enable);
  }
  else if (cap == GL_VERTEX_PROGRAM_ARB)
  {
    getContext().enableVertexProgram(enable);
  }
}

void GLAPIENTRY wrap_Enable(GLenum cap)
{
  assert(wgl_wrapper::isMainThread());

  gl::Enable(cap);

  if (wgl_wrapper::isMainContextCurrent())
    setEnabled(cap, true);
}

void GLAPIENTRY wrap_Disable(GLenum cap)
{
  assert(wgl_wrapper::isMainThread());

  gl::Disable(cap);

  if (wgl_wrapper::isMainContextCurrent())
    setEnabled(cap, false);
}


} // namespace


namespace core_gl_wrapper::arb_program
{
  void update()
  {
    ::getContext().update();
  }


  bool isObjectProgramActive()
  {
    return ::getContext().isObjectProgramActive();
  }

  #define SET_OVERRIDE(func) core_gl_wrapper::setProc("gl"#func, (void*) wrap_##func);

  void init()
  {
    SET_OVERRIDE(Enable)
    SET_OVERRIDE(Disable)

    SET_OVERRIDE(BindProgramARB)
    SET_OVERRIDE(DeleteProgramsARB)
    SET_OVERRIDE(GenProgramsARB)
    SET_OVERRIDE(IsProgramARB)
    SET_OVERRIDE(ProgramStringARB)
    SET_OVERRIDE(ProgramEnvParameter4dARB)
    SET_OVERRIDE(ProgramEnvParameter4dvARB)
    SET_OVERRIDE(ProgramEnvParameter4fARB)
    SET_OVERRIDE(ProgramEnvParameter4fvARB)
    SET_OVERRIDE(ProgramEnvParameters4fvEXT)
    SET_OVERRIDE(ProgramLocalParameter4dARB)
    SET_OVERRIDE(ProgramLocalParameter4dvARB)
    SET_OVERRIDE(ProgramLocalParameter4fARB)
    SET_OVERRIDE(ProgramLocalParameter4fvARB)
    SET_OVERRIDE(ProgramLocalParameters4fvEXT)
    SET_OVERRIDE(GetProgramEnvParameterdvARB)
    SET_OVERRIDE(GetProgramEnvParameterfvARB)
    SET_OVERRIDE(GetProgramLocalParameterdvARB)
    SET_OVERRIDE(GetProgramLocalParameterfvARB)
    SET_OVERRIDE(GetProgramivARB)
    SET_OVERRIDE(GetProgramStringARB)
  }

}
