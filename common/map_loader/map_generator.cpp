/**
 *    Rendering utilities
 *    Copyright (C) 2018  Jan Lepper
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

#include "map_loader_private.h"
#include <render_util/elevation_map.h>
#include <render_util/image_loader.h>
#include <render_util/image_resample.h>
#include <render_util/image_util.h>
#include <render_util/map.h>
#include <render_util/texunits.h>
#include <render_util/texture_util.h>
#include <render_util/render_util.h>
#include <FastNoise.h>

#include <vector>
#include <iostream>
#include <sstream>

using namespace std;
using namespace render_util;
using namespace il2ge::map_generator;


namespace
{


const string g_base_land_map_file_name = "base_map_land.tga";

enum
{
  TERRAIN_TYPE_WATER,
  TERRAIN_TYPE_GRASS,
  TERRAIN_TYPE_FIELD,
  TERRAIN_TYPE_FIELD2,
  TERRAIN_TYPE_FIELD3,
  TERRAIN_TYPE_FIELD4,
  TERRAIN_TYPE_FOREST,
  TERRAIN_TYPE_TUNDRA,
  TERRAIN_TYPE_TUNDRA2,
  TERRAIN_TYPE_ROCK,
  TERRAIN_TYPE_ICE
};

const glm::vec3 default_water_color = glm::vec3(0.140, 0.195, 0.230);
const float sea_level = 0;
const int type_map_meters_per_pixel = 200;
const int meters_per_tile = 1600;


ImageGreyScale::Ptr generateTypeMapPrivate(ElevationMap::ConstPtr elevation_map)
{
  FastNoise noise_generator;
//   noise_generator.SetFrequency(0.4);

  auto normal_map = createNormalMap(elevation_map, 200);
  auto type_map = image::create<unsigned char>(0, elevation_map->getSize());

  for (int y = 0; y < type_map->h(); y++)
  {
    for (int x = 0; x < type_map->w(); x++)
    {
      unsigned type = 0;
      float height = elevation_map->get(x,y);
      float noise = noise_generator.GetSimplex(x * 10, y * 10);

      if (height <= sea_level)
      {
        type = TERRAIN_TYPE_WATER;
      }
      else if (height < 1500)
      {
        type = TERRAIN_TYPE_FOREST;
      }
      else if (true || height < 1800)
      {
//         assert(0);

        type = TERRAIN_TYPE_ROCK;

//         if (height < 700 && noise_generator.GetValueFractal(x * 16, y * 16) < 0.2)
//         {
//           type = TERRAIN_TYPE_GRASS;
//         }
//         else
//         {
//           if (noise > 0)
//             type = TERRAIN_TYPE_TUNDRA;
//           else
//             type = TERRAIN_TYPE_TUNDRA2;
//         }
      }
      else
      {
        assert(0);
        type = TERRAIN_TYPE_ICE;
      }

      type_map->at(x,y) = type;
    }
  }

  for (int y = 0; y < type_map->h(); y++)
  {
    for (int x = 0; x < type_map->w(); x++)
    {
      float height = elevation_map->get(x,y);
      const Normal &normal = normal_map->at(x,y);
      unsigned type = type_map->at(x,y);

      if (type == TERRAIN_TYPE_FOREST && height < 200)
      {
        if (normal.z > 0.8)
        {
          float noise_scale = 4;

          float noise = noise_generator.GetValueFractal(x * noise_scale * 8, y * noise_scale * 8);
          if (noise > 0.0)
            type_map->at(x,y) = TERRAIN_TYPE_FIELD;
          else
            type_map->at(x,y) = TERRAIN_TYPE_FIELD3;

          if (noise_generator.GetValueFractal(x * noise_scale * 5, y * noise_scale * 5) > 0.0)
          {
            if (noise_generator.GetValueFractal(x * noise_scale * 8, y * noise_scale * 8) > 0)
              type_map->at(x,y) = TERRAIN_TYPE_FIELD2;
            else
              type_map->at(x,y) = TERRAIN_TYPE_FIELD4;
          }
        }
//         if (normal.z < 0.9)
//           type_map->at(x,y) = TERRAIN_TYPE_GRASS;
      }
      else if (type == TERRAIN_TYPE_FOREST)
      {
//         if (noise_generator.GetValueFractal(x * 16, y * 16) +
//             noise_generator.GetValueFractal(x * 2, y * 2) -
//             noise_generator.GetValueFractal((x+10000) * 6, (y+10000) * 6) - 0.3 > 0.0)
//         {
//           type_map->at(x,y) = TERRAIN_TYPE_GRASS;
//         }
//         if (normal.z < 0.9)
//           type_map->at(x,y) = TERRAIN_TYPE_GRASS;
      }


//       if (type == TERRAIN_TYPE_ICE && normal.z < 0.85)
//         type_map->at(x,y) = TERRAIN_TYPE_ROCK);

//       if (normal.z < 0.7)
//         type_map->at(x,y) = TERRAIN_TYPE_ROCK;

    }

  }


  return type_map;
}


} // namespace


render_util::ElevationMap::Ptr il2ge::map_generator::generateHeightMap(
    render_util::ImageGreyScale::ConstPtr land_map)
{
  FastNoise noise_generator;

  auto heightmap = render_util::image::create<float>(0, glm::ivec2(4096));

  shared_ptr<Surface<ImageGreyScale>> land_map_surface;

  if (land_map)
  {
    auto flipped = image::flipY(land_map);
    land_map_surface = make_shared<Surface<ImageGreyScale>>(flipped);
    land_map_surface->setSize(heightmap->getSize());
    land_map_surface->setPixelOffset(glm::vec2(0.5));
  }

  const float scale = 8;
  const float coarse_scale = 4;

  for (int y = 0; y < heightmap->h(); y++)
  {
    for (int x = 0; x < heightmap->w(); x++)
    {
      float height = noise_generator.GetValueFractal(x, y) * 1000;
      height += noise_generator.GetValueFractal(x * coarse_scale, y * coarse_scale) * 1500;
      height += noise_generator.GetValueFractal(x * scale, y * scale) * 400;
      height += noise_generator.GetValueFractal(x * 30, y * 30) * 200;
      height += 200;
      height = glm::max(10.f, height);

      if (land_map_surface)
      {
        float threshold = 0.2 + noise_generator.GetValueFractal(x * 8, y * 8) * 0.2;
        float land_map_value = (float)land_map_surface->get(x,y) / 255.0;

        land_map_value = glm::smoothstep(threshold, threshold + 0.3f, land_map_value);
        height *= land_map_value;
      }

      heightmap->at(x,y) = height;
    }
  }

  return heightmap;
}


ImageGreyScale::Ptr il2ge::map_generator::generateTypeMap(ElevationMap::ConstPtr elevation_map)
{
  assert(elevation_map);

  using namespace map_generator;
  using namespace il2ge::map_loader;

  auto map = generateTypeMapPrivate(elevation_map);

  map->forEach([&] (auto &pixel)
  {
    switch (pixel)
    {
      case TERRAIN_TYPE_WATER:
        pixel = getFieldIndex("Water2");
        break;
      case TERRAIN_TYPE_GRASS:
        assert(0);
        pixel = getFieldIndex("LowLand0");
        break;
      case TERRAIN_TYPE_FIELD:
        pixel = getFieldIndex("MidLand2");
        break;
      case TERRAIN_TYPE_FIELD2:
        pixel = getFieldIndex("LowLand2");
        break;
      case TERRAIN_TYPE_FIELD3:
        pixel = getFieldIndex("LowLand3");
        break;
      case TERRAIN_TYPE_FIELD4:
        pixel = getFieldIndex("MidLand0");
        break;
      case TERRAIN_TYPE_FOREST:
        pixel = getFieldIndex("Wood0");
        break;
      case TERRAIN_TYPE_ROCK:
        pixel = getFieldIndex("Mount0");
        break;
      default:
        assert(0);
    }
  });

  map = image::flipY(map);

  return map;
}

const std::string &il2ge::map_generator::getBaseLandMapFileName()
{
  return g_base_land_map_file_name;
}
