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


namespace il2ge::core_wrapper
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
  Setting<bool> &better_shadows = addSetting("BetterShadows", false, "");
#endif

#if ENABLE_CONFIGURABLE_ATMOSPHERE
  MultipleChoice<render_util::Atmosphere::Type> &atmosphere =
    addMultipleChoice<render_util::Atmosphere::Type>("Atmosphere",
      {
        { render_util::Atmosphere::DEFAULT, "default", "" },
        { render_util::Atmosphere::PRECOMPUTED, "precomputed", "HDR - experimental" }
      },
      render_util::Atmosphere::DEFAULT,
      "atmosphere shader");
#endif

};


}

#endif
