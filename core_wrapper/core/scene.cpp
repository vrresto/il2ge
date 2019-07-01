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
#include "map.h"
#include <sfs.h>
#include <core/scene.h>
#include <render_util/terrain.h>
#include <render_util/texture_util.h>
#include <render_util/render_util.h>
#include <render_util/water.h>
#include <render_util/texunits.h>

#include <string>
#include <memory>
#include <GL/gl.h>

#include <render_util/gl_binding/gl_functions.h>

using namespace render_util::gl_binding;
using namespace render_util;
using namespace std;


namespace
{
  const std::string g_shader_path = IL2GE_DATA_DIR "/shaders";
}


namespace core
{

  Scene::Scene()
  {
    text_renderer = make_unique<TextRenderer>();

    atmosphere = createAtmosphere(il2ge::core_wrapper::getConfig().atmosphere,
                                  texture_manager, g_shader_path);

    shader_search_path.push_back(g_shader_path + "/" + atmosphere->getShaderPath());
    shader_search_path.push_back(g_shader_path);

    FORCE_CHECK_GL_ERROR();
    curvature_map = render_util::createCurvatureTexture(texture_manager, IL2GE_CACHE_DIR);
    FORCE_CHECK_GL_ERROR();
    atmosphere_map = render_util::createAmosphereThicknessTexture(texture_manager, IL2GE_CACHE_DIR);
    FORCE_CHECK_GL_ERROR();

    GLenum active_unit_save;
    gl::GetIntegerv(GL_ACTIVE_TEXTURE, reinterpret_cast<GLint*>(&active_unit_save));
    FORCE_CHECK_GL_ERROR();

    int atmosphere_map_unit = texture_manager.getHighestUnit() + 1;
    assert(atmosphere_map_unit < texture_manager.getMaxUnits());

    gl::ActiveTexture(GL_TEXTURE0 + atmosphere_map_unit); //FIXME
    FORCE_CHECK_GL_ERROR();

    gl::BindTexture(atmosphere_map->getTarget(), atmosphere_map->getID());

    gl::ActiveTexture(active_unit_save);

    FORCE_CHECK_GL_ERROR();

    auto addParameter = [this] (std::string name, Parameter::GetFunc get, Parameter::SetFunc set)
    {
      parameters.push_back(Parameter(name, get, set));
    };

    auto addAtmosphereParameter = [this,addParameter] (std::string name, Atmosphere::Parameter p)
    {
      if (atmosphere->hasParameter(p))
      {
        addParameter(name,
                     [this,p] { return atmosphere->getParameter(p); },
                     [this,p] (auto value) { atmosphere->setParameter(p, value); });
      }
    };

    addAtmosphereParameter("exposure", Atmosphere::Parameter::EXPOSURE);
    addAtmosphereParameter("saturation", Atmosphere::Parameter::SATURATION);
    addAtmosphereParameter("brightness_curve_exponent",
                            Atmosphere::Parameter::BRIGHTNESS_CURVE_EXPONENT);
    addAtmosphereParameter("texture_brightness", Atmosphere::Parameter::TEXTURE_BRIGHTNESS);
    addAtmosphereParameter("texture_brightness_curve_exponent",
                            Atmosphere::Parameter::TEXTURE_BRIGHTNESS_CURVE_EXPONENT);
    addAtmosphereParameter("texture_saturation", Atmosphere::Parameter::TEXTURE_SATURATION);

    menu = std::make_unique<Menu>(*this, texture_manager, shader_search_path);
  }


  Scene::~Scene() {}


  void Scene::unloadMap()
  {
    printf("unloadMap()\n");

    map.reset();
    gl::Finish();
    sfs::clearRedirections();
  }

  void Scene::loadMap(const char *path, ProgressReporter *progress)
  {
    printf("load map: %s\n", path);

    unloadMap();

    map = make_unique<Map>(path, progress, shader_search_path, atmosphere->getShaderParameters());
  }


  void Scene::update(float delta, const glm::vec2 &wind_speed)
  {
    effects.update(delta, wind_speed);
    if (map)
      map->getWaterAnimation()->update();
  }


  void Scene::updateUniforms(render_util::ShaderProgramPtr program)
  {
    atmosphere->setUniforms(program, *core::getCamera());
    if (map)
      map->setUniforms(program);
  }


  render_util::TerrainBase &Scene::getTerrain()
  {
    assert(map);
    return map->getTerrain();
  }


  render_util::ImageGreyScale::ConstPtr Scene::getPixelMapH()
  {
    assert(map);
    return map->getPixelMapH();
  }


  render_util::ShaderParameters Scene::getShaderParameters()
  {
    return atmosphere->getShaderParameters();
  }


}
