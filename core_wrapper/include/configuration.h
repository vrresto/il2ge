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

#ifndef IL2GE_CORE_WRAPPER_CONFIGURATION_H
#define IL2GE_CORE_WRAPPER_CONFIGURATION_H


#include <configuration_base.h>
#include <config.h>
#include <render_util/atmosphere.h>


namespace il2ge::core_wrapper::configuration
{


class Configuration : public ConfigurationBase
{
public:
  Setting<bool> &enable_graphics_extender = addSetting("EnableGE", true,
                                                      "enable graphics extender");

  Setting<bool> &enable_object_shaders = addSetting("EnableObjectShaders", true,
                                                   "disabling this gives higher fps but looks bad");

  Setting<bool> &enable_bumph_maps = addSetting("EnableBumpH", false, "enable terrain bumpmapping");

  Setting<bool> &enable_cirrus_clouds = addSetting("EnableCirrusClouds", false,
                                                   "cirrus clouds - experimental");

  Setting<bool> &enable_transparent_shader = addSetting("EnableTransparentShader", false,
                                                        "enable shader for transparent objects - experimental");

#if ENABLE_WIP_FEATURES
  Setting<bool> &enable_effects = addSetting("EnableEffects", false,
                                            "new effect renderer - experimental");

  Setting<bool> &enable_light_point = addSetting("EnableLightPoint", false,
                                                "new lighting system - experimental");
#endif

//     Setting<bool> enable_base_map = addSetting("EnableBaseMap", false, "");

#if ENABLE_MAP_VIEWER
  Setting<bool> &enable_dump = addSetting("EnableDump", false, "");
#endif

#if ENABLE_CONFIGURABLE_SHADOWS
  Setting<bool> &better_shadows = addSetting("RealisticShadowColor", false, "experimental");
#endif

#if ENABLE_CONFIGURABLE_ATMOSPHERE
  MultipleChoice<render_util::Atmosphere::Type> &atmosphere =
    addMultipleChoice<render_util::Atmosphere::Type>("Atmosphere",
      {
        {
          render_util::Atmosphere::DEFAULT,
          "Default",
          { "the original shader" }
        },
        {
          render_util::Atmosphere::PRECOMPUTED,
          "Precomputed",
          {
            "(experimental)",
            "precomputed light scattering"
          }
        },
      },
      render_util::Atmosphere::DEFAULT,
      "atmospheric light scattering shader");

  struct AtmospherePrecomputedSection : public Section
  {
    AtmospherePrecomputedSection() : Section("Atmosphere.Precomputed") {}

    Setting<bool> &precomputed_luminance =
      addSetting("AccurateLuminance", false, "more accurate colors");

    Setting<float> &haziness =
      addSetting("Haziness", 0.f, "possible values are between 0.0 and 1.0");

    Setting<bool> &single_mie_horizon_hack =
      addSetting("SingleMieScatteringHorizonHack", false, "");
  };

  AtmospherePrecomputedSection &atmosphere_precomputed =
    addSection<AtmospherePrecomputedSection>();
#endif


};


}

#endif
