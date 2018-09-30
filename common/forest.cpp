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

#include "forest.h"
#include "map_loader_private.h"
#include <render_util/texture_util.h>
#include <render_util/image_util.h>
#include <render_util/image_resample.h>
#include <render_util/map_textures.h>
#include <render_util/texunits.h>

using namespace il2ge;
using namespace std;
using namespace glm;
using namespace render_util;

namespace
{


ImageGreyScale::Ptr createForestMap(ImageGreyScale::ConstPtr type_map)
{
  auto map = image::clone(type_map);

  map->forEach([] (unsigned char &pixel)
  {
    if (il2ge::isForest(pixel & 0x1F))
      pixel = 255;
    else
      pixel = 0;
  });

  return map;
}


vector<ImageRGBA::ConstPtr> getForestLayers(il2ge::RessourceLoader *loader)
{
  vector<ImageRGBA::ConstPtr> textures;

  for (int i = 0; i <= 4; i++)
  {
    cout<<"getForestTexture() - layer "<<i<<endl;

    auto name = string("Wood") + to_string(i);

    ImageRGBA::Ptr texture = il2ge::getTexture("WOOD", name.c_str(), "", loader);
    if (!texture)
      continue;

    textures.push_back(texture);
  }

  return textures;
}


ImageRGBA::Ptr createForestFarTexture(il2ge::RessourceLoader *loader)
{
  ImageRGBA::Ptr combined_texture;

  const int num_layers = 10;

  for (int layer = 0; layer <= num_layers; layer++)
  {

    int offset_x = 8;
    int offset_y = 8;

    offset_x *= layer * 15;
    offset_y *= layer * 15;

    for (int i = 1; i < 5; i++)
    {
      cout<<"getForestTexture() - layer "<<i<<endl;

      auto name = string("Wood") + to_string(i);

      ImageRGBA::Ptr texture = il2ge::getTexture("WOOD", name.c_str(), "", loader);
      if (!texture)
        continue;

//       vec4 avg = pixelToVector(image::getAverageColor(texture.get()));
  //     color = mix(color, avg, avg.a);
  //     color.a = glm::max(color.a, avg.a);

      if (!combined_texture)
      {
        combined_texture = image::clone(texture);
      }
      else
      {
        assert(combined_texture->size() == texture->size());
        for (int y = 0; y < combined_texture->h(); y++)
        {
          for (int x = 0; x < combined_texture->w(); x++)
          {
            vec4 combined = pixelToVector(combined_texture->getPixel(x, y));

            int x_src = (x + (i * offset_x)) % texture->w();
            int y_src = (y + (i * offset_y)) % texture->h();

            vec4 src = pixelToVector(texture->getPixel(x_src, y_src));

            vec4 new_combined = mix(combined, src, src.w);
            new_combined.w = glm::max(combined.w, src.w);
            new_combined.w = 1;

            combined_texture->setPixel(x, y, vectorToPixel(new_combined));
          }
        }
      }
    }
  }

  combined_texture = downSample(combined_texture, 2);

  dump<ImageRGBA>(combined_texture, "WOOD_far.tga", loader->getDumpDir());

//   vec4 color = pixelToVector(image::getAverageColor(combined_texture.get()));

//   color.a = glm::mix(color.a, 1.0f, 0.5f);
//   color.a = 1;

  return combined_texture;
}


ImageRGBA::Ptr createForestFarTexture_alt(il2ge::RessourceLoader *loader)
{
  ImageRGBA::Ptr texture = il2ge::getTexture("WOOD", "Wood1", "", loader);
  assert(texture);

  texture->forEach(3, [] (unsigned char &alpha)
  {
    alpha = 255;
  });

  return texture;
}


bool isForest(const ivec2 &pos, ImageGreyScale::ConstPtr type_map)
{
  return il2ge::isForest(type_map->get(pos) & 0x1F);
}


} // namespace



namespace il2ge
{


void createForestTextures(ImageGreyScale::ConstPtr type_map,
                          MapTextures *map_textures,
                          il2ge::RessourceLoader *loader,
                          ImageGreyScale::ConstPtr type_map_base)
{
  cout<<"loading forest texture ..."<<endl;
  auto forest_map = createForestMap(type_map);
  assert(forest_map);
  map_textures->setForestMap(forest_map);

  if (type_map_base)
  {
    auto forest_map_base = createForestMap(type_map_base);
    assert(forest_map_base);
    map_textures->setTexture(TEXUNIT_FOREST_MAP_BASE, forest_map_base);
  }

//   map_textures->setTexture(TEXUNIT_FOREST_FAR, createForestFarTexture(loader));
  map_textures->setTexture(TEXUNIT_FOREST_FAR, createForestFarTexture_alt(loader));

  vector<ImageRGBA::ConstPtr> forest_layers = getForestLayers(loader);
  assert(!forest_layers.empty());
  map_textures->setForestLayers(forest_layers);
  cout<<"loading forest texture done."<<endl;
}


} // namespace il2ge
