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

#ifndef CORE_SCENE_H
#define CORE_SCENE_H

#include <misc.h>
#include <core.h>
#include <core/effects.h>
#include <render_util/camera.h>
#include <render_util/parameter_wrapper.h>
#include <text_renderer/text_renderer.h>

#include <memory>
#include <vector>
#include <cassert>


namespace render_util
{
  class Atmosphere;
}


namespace core
{
  class ProgressReporter;
  class Menu;
  class Map;

  class Scene
  {
    using Parameter = render_util::ParameterWrapper<float>;

    render_util::ShaderSearchPath shader_search_path;
    render_util::TexturePtr atmosphere_map;
    render_util::TexturePtr curvature_map;
    std::unique_ptr<render_util::Atmosphere> atmosphere;
    std::unique_ptr<Map> map;
    std::unique_ptr<TextRenderer> text_renderer;
    std::unique_ptr<Menu> menu;
    std::vector<Parameter> parameters;

  public:
    render_util::TextureManager texture_manager = render_util::TextureManager(0, 64);
    Effects effects;

    Scene();
    ~Scene();

    void unloadMap();
    void loadMap(const char *path, ProgressReporter*);
    void update(float delta, const glm::vec2 &wind_speed);
    void updateUniforms(render_util::ShaderProgramPtr program);
    render_util::TerrainBase &getTerrain();

    render_util::ImageGreyScale::ConstPtr getPixelMapH();

    const render_util::ShaderSearchPath &getShaderSearchPath()
    {
      return shader_search_path;
    }

    render_util::ShaderParameters getShaderParameters();

    TextRenderer &getTextRenderer() { return *text_renderer; }

    Menu &getMenu() { return *menu; }

    bool isMapLoaded() { return map != nullptr; }

    int getNumParameters() { return parameters.size(); }
    Parameter &getParameter(int i) { return parameters.at(i); }
  };
};

#endif
