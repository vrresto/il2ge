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

#include "map_loader_private.h"
#include "water_map.h"
#include "forest.h"
#include <render_util/image.h>
#include <render_util/image_util.h>
#include <render_util/image_resample.h>
#include <render_util/image_loader.h>
#include <render_util/texture_util.h>
#include <render_util/texunits.h>
#include <render_util/map_textures.h>
#include <render_util/terrain.h>
#include <render_util/terrain_cdlod.h>
#include <render_util/elevation_map.h>
#include <render_util/water.h>
#include <il2ge/map_loader.h>
#include <il2ge/ressource_loader.h>
#include <il2ge/image_loader.h>
#include <log.h>

#include <FastNoise.h>
#include <glm/glm.hpp>

#include <filesystem>
#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <cstring>
#include <cmath>
#include <memory>
#include <glm/glm.hpp>

#include <GL/gl.h>
#include <GL/glext.h>

#ifdef max
#undef max
#endif

using namespace std;
using namespace render_util;
using namespace glm;
using namespace il2ge;
using namespace il2ge::map_loader;

namespace
{

constexpr auto RANDOM_CIRRUS_TEXTURE_DIR = "il2ge_random_cirrus_textures";

const vec3 default_water_color = vec3(45,51,40) / vec3(255);

void dumpFile(string name, const char *data, size_t data_size, const string &dump_dir)
{
  if (!isDumpEnabled() || dump_dir.empty())
    return;
  util::writeFile(dump_dir + '/' +  name, data, data_size);
}

enum
{
  NUM_FIELDS = 32
};

const char * const field_names[NUM_FIELDS] =
{
  "LowLand0",
  "LowLand1",
  "LowLand2",
  "LowLand3",
  "MidLand0",
  "MidLand1",
  "MidLand2",
  "MidLand3",
  "Mount0",
  "Mount1",
  "Mount2",
  "Mount3",
  "Country0",
  "Country1",
  "Country2",
  "Country3",
  "City0",
  "City1",
  "City2",
  "City3",
  "AirField0",
  "AirField1",
  "AirField2",
  "AirField3",
  "Wood0",
  "Wood1",
  "Wood2",
  "Wood3",
  "Water0",
  "Water1",
  "Water2",
  "Water3",
};


float elevation_table[256] =
{
  #include "height_table"
};


auto &getRandomNumberGenerator()
{
  static std::mt19937 gen(time(nullptr));
  return gen;
}


void createWaterMap
  (
    ivec2 type_map_size,
    il2ge::RessourceLoader *loader,
    il2ge::WaterMap &map,
    render_util::Image<water_map::ChunkType>::Ptr &small_water_map
  )
{
  il2ge::WaterMap water_map;

  render_util::ImageGreyScale::ConstPtr color_map;

  color_map = getTexture<ImageGreyScale>("MAP", "ColorMap", "map_c.tga", true, loader);
  assert(color_map);
  assert(color_map->w() == 1024);

  const int chunk_size = 32;

  createChunks(color_map, chunk_size, water_map.chunks);


  {
    ivec2 table_size = (type_map_size * ivec2(4)) / ivec2(chunk_size);
    size_t data_size = table_size.x * table_size.y;

    istringstream file;

    {
      vector<char> data;
      if (!loader->readFile("MAP", "ColorMap", "map_c.tga", "_table", data))
      {
        assert(0);
      }
      dumpFile("MAP_ColorMap.tga_table", data.data(), data.size(), loader->getDumpDir());

      file.str(string(data.data(), data.size()));
    }

    vector<unsigned int> data;

    assert(file.good());

    file.seekg(16);
    assert(file.good());

    while (data.size() < data_size)
    {
      assert(file.good());
      unsigned int value = file.get() << 24 | file.get() << 16 | file.get() << 8 | file.get();
      data.push_back(value);
    }

    assert(data.size() == data_size);

    water_map.table = std::make_shared<Image<unsigned int>>(table_size, data);
    water_map.table = image::flipY(water_map.table);

    il2ge::convertWaterMap(water_map, map, small_water_map);
  }
}


void createFieldTextures(ImageGreyScale::ConstPtr type_map_,
                         render_util::LandTextures &land_textures,
                         il2ge::RessourceLoader *loader,
                         bool enable_normal_maps)
{
  vector<ImageRGBA::Ptr> textures;
  vector<ImageRGB::Ptr> textures_nm;
#if 0
  auto red_texture = make_shared<ImageRGBA>(ivec2(512));
  image::fill(red_texture, ImageRGBA::PixelType{255, 0, 0, 255});

  texture_arrays[getTerrainTextureArrayIndex(red_texture->w())].push_back(red_texture);
  textures.push_back(red_texture);
#endif

  std::vector<float> texture_scale;

//   ImageRGBA::Ptr default_normal_map(new ImageRGBA(ivec2(128,128)));
//   RGBA default_normal = { 127, 127, 255, 255 };
//   image::fill<RGBA>(default_normal_map, default_normal);
//   vector<ImageRGBA::ConstPtr> normal_maps;

  for (int i = 0; i < NUM_FIELDS; i++)
  {
    const char *field_name = field_names[i];

    LOG_TRACE<<"loading texture: "<<field_name<<" ..."<<endl;

#if 0
    {
      string bumpmap_path = path.substr(0, path.find_last_of('.')) + ".BumpH";
      vector<char> content;
      if (SFS::readFile(bumpmap_path, content))
      {
        dumpFile(string(field_names[i]) + "_bump.tga", content.data(), content.size());

        render_util::ImageGreyScale::Ptr bump_map(render_util::loadImageFromMemory<render_util::ImageGreyScale>(content));
        assert(bump_map);
        render_util::ImageRGBA::Ptr normal_map(render_util::createNormalMap(bump_map, 8.0));

        render_util::saveImageToFile(dump_dir + field_names[i] + "_normal.tga", normal_map.get());

        normal_maps.push_back(image::flipY<ImageRGBA>(normal_map));
      }
      else
        normal_maps.push_back(default_normal_map);
    }
#endif

    float scale = 1.0;

    ImageRGBA::Ptr image = getTexture("FIELDS", field_name, "", loader, true, &scale);

    if (enable_normal_maps)
    {
      std::vector<char> content;

      float scale = 1;
      bool was_read = false;

      was_read = loader->readTextureFile("FIELDS", field_name, "", content,
                                         false, true, &scale, true);
      if (was_read)
      {
        dumpFile(string("FIELDS_") +  field_name + "_nm.tga", content.data(), content.size(),
                 loader->getDumpDir());

        auto normal_map = loadImageRGBFromMemory(content, field_name);
        normal_map = image::flipY(normal_map);
        textures_nm.push_back(normal_map);
      }
      else
        textures_nm.push_back({});
    }
    else
    {
      assert(textures_nm.empty());
    }

    if (image)
    {
      image = image::flipY(image);
      assert(image->w() == image->h());
    }

    textures.push_back(image);
    texture_scale.push_back(scale);
  }

  auto type_map = make_shared<ImageGreyScale>(type_map_->getSize());

  for (int y = 0; y < type_map->h(); y++)
  {
    for (int x = 0; x < type_map->w(); x++)
    {
      unsigned int index = type_map_->get(x,y) & 0x1F;

      if (strcmp(field_names[index], "Wood1") == 0 ||
          strcmp(field_names[index], "Wood3") == 0)
      {
        index -= 1;
      }

      type_map->at(x,y) = index;
    }
  }

  dump(type_map, "type_map", loader->getDumpDir());

  land_textures.type_map = type_map;
  land_textures.textures = textures;
  land_textures.textures_nm = textures_nm;
  land_textures.texture_scale = texture_scale;

  LOG_INFO << "generating far texture ..." <<endl;
  std::vector<ImageRGBA::ConstPtr> textures_const;
  for (auto texture : textures)
    textures_const.push_back(texture);
  land_textures.far_texture =
    createMapFarTexture(type_map,
                        textures_const,
                        TYPE_MAP_METERS_PER_PIXEL,
                        TERRAIN_METERS_PER_TEXTURE_TILE);
  dump(land_textures.far_texture, "far_texture", loader->getDumpDir());
}


void createWaterTypeMap(ImageGreyScale::ConstPtr type_map, MapTextures *map_textures, il2ge::RessourceLoader *loader)
{
  ImageGreyScale::Ptr water_type_map = image::create<unsigned char>(0, type_map->size());
  for (int y = 0; y < type_map->h(); y++)
  {
    for (int x = 0; x < type_map->w(); x++)
    {
      unsigned int index = type_map->get(x,y) & 0x1F;

      int water_type = glm::clamp((int)index - 28, -1, 31);
      water_type += 1;
      assert(water_type >= 0);
      assert(water_type <= 4);
//       assert(water_type == 0);

      unsigned char water_type_c = water_type;

      assert(water_type_c >= 0);
      assert(water_type_c <= 4);

      water_type_map->at(x,y) = water_type_c;
    }
  }
  map_textures->setWaterTypeMap(water_type_map);
}


template <typename T, class Container>
bool loadWaterTexture(const char *prefix,
                      const char *suffix,
                      int i,
                      Container &container,
                      il2ge::RessourceLoader *loader)
{
  char basename[100];
  snprintf(basename, sizeof(basename), "%s%.2d%s", prefix, i, suffix);
  auto filename = string(basename) + ".tga";

  LOG_TRACE << "loading " << filename << endl;

  vector<char> data;
  if (!loader->readWaterAnimation(filename, data))
  {
    return false;
  }

  auto image = loadImageFromMemory<T>(data, filename.c_str());
  assert(image);

  if (i != 0 && image->getSize() != container.at(0)->getSize())
  {
    LOG_WARNING << filename << " has wrong size." << endl;
    LOG_WARNING << "Expected: " << container.at(0)->getSize() << endl;
    LOG_WARNING << "Got: " << image->getSize() << endl;
    return false;
  }

  dump(image, basename, loader->getDumpDir());
  container.push_back(image);

  return true;
};


void createWaterNormalMaps(render_util::WaterAnimation *water_animation,
                           render_util::MapTextures *map_textures,
                           il2ge::RessourceLoader *loader)
{

  LOG_INFO << "loading water textures..." << endl;

  //     const char *foam_detail_name = "water_textures/FoamNV40.tga";
  //     foam_detail_texture = SOIL_load_OGL_texture(foam_detail_name, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
  //         SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS);
  //
  //     if (!foam_detail_texture) {
  //       printf("Fail\n");
  //       exit(1);
  //     }

  vector<ImageRGBA::ConstPtr> normal_maps;
  vector<ImageGreyScale::ConstPtr> foam_masks;

  int i = 0;

  while (true)
  {
    if (!loadWaterTexture<ImageRGBA>("WaterNoise", "Dot3", i, normal_maps, loader))
      break;

    if (!loadWaterTexture<ImageGreyScale>("WaterNoiseFoam", "", i, foam_masks, loader))
    {
      normal_maps.pop_back();
      break;
    }

    i++;
  }

  assert(normal_maps.size() == foam_masks.size());

  water_animation->createTextures(map_textures, normal_maps, foam_masks);
}


std::shared_ptr<render_util::GenericImage> loadRandomCirrusTexture()
{
  std::vector<std::string> file_paths;

  try
  {
    std::filesystem::directory_iterator it(RANDOM_CIRRUS_TEXTURE_DIR);
    for (auto &entry : it)
    {
      if (!entry.is_regular_file())
        continue;

      if (util::makeLowercase(entry.path().extension().generic_string()) == ".tga")
        file_paths.push_back(entry.path().generic_string());
    }
  }
  catch (std::exception &e)
  {
    LOG_WARNING << e.what() << std::endl;
  }

  if (!file_paths.empty())
  {
    uniform_int_distribution<unsigned int> dist(0, file_paths.size()-1);
    auto random_index = dist(getRandomNumberGenerator());
    auto data = util::readFile<char>(file_paths.at(random_index));
    if (!data.empty())
      return loadImageFromMemory(data, "cirrus");
  }

  return {};
}


void createCirrusTextures(MapBase *map,
                          il2ge::RessourceLoader *loader)
{
  auto cirrus_texture = loadRandomCirrusTexture();
  if (!cirrus_texture)
    cirrus_texture = getTexture<GenericImage>("APPENDIX", "HighClouds", "", false, loader);
//   auto cirrus_noise_texture = getTexture<GenericImage>("APPENDIX", "HighCloudsNoise", "", false, loader);

  map->setCirrusTexture(cirrus_texture);
}



} // namespace


namespace il2ge::map_loader
{


ImageGreyScale::Ptr createTypeMap(il2ge::RessourceLoader *loader)
{
  LOG_INFO<<"loading type map ..."<<endl;
  auto type_map = getTexture<ImageGreyScale>("MAP", "TypeMap", "map_T.tga", true, loader);
  assert(type_map);
  type_map = image::flipY(type_map);
  return type_map;
}


void createLandTextures(il2ge::RessourceLoader *loader,
                        ImageGreyScale::ConstPtr type_map,
                        LandTextures &land_textures,
                        bool enable_normal_maps)
{
  createFieldTextures(type_map, land_textures, loader, enable_normal_maps);
}


void createMapTextures(il2ge::RessourceLoader *loader,
                       ImageGreyScale::ConstPtr type_map,
                       render_util::MapBase *map)
{
//   getTexture("APPENDIX", "BeachFoam", "", reader);
//   getTexture("APPENDIX", "BeachSurf", "", reader);
//   getTexture("APPENDIX", "BeachLand", "", reader);
//   getTexture("WATER", "Water", "", reader);
//   getTexture("WOOD", "WoodMask2", "", reader);
//   getTexture("WOOD", "WoodMask3", "", reader);
//   getTexture("WOOD", "WoodMiniMasks", "", reader);
//   getTexture("APPENDIX", "ForestNoise", "", loader);
//   getTexture("APPENDIX", "WaterNoise", "", loader, true);
//   getTexture("WOOD", "WoodMiniMasks", "", loader, true);

  auto map_textures = &map->getTextures();
  auto water_animation = &map->getWaterAnimation();

  createWaterNormalMaps(water_animation, map_textures, loader);

#if 1
  LOG_DEBUG<<"creating water map ..."<<endl;
  il2ge::WaterMap water_map;
  render_util::Image<water_map::ChunkType>::Ptr small_water_map;
  createWaterMap(
    type_map->size(),
    loader,
    water_map,
    small_water_map);
  LOG_DEBUG<<"creating water map done."<<endl;
  map_textures->setWaterMap(water_map.chunks, water_map.table);
#endif

  createWaterTypeMap(type_map, map_textures, loader);

  auto material_map = image::convert<TerrainBase::MaterialMap::ComponentType>(type_map);

  material_map->forEach
  (
    [] (auto &pixel)
    {
      unsigned int material = 0;
      if (isForest(pixel & 0x1F))
        material = TerrainBase::MaterialID::FOREST;
      else
        material = TerrainBase::MaterialID::LAND;
      pixel = material;
    }
  );

  assert(material_map->w() <= small_water_map->w());
  assert(material_map->h() <= small_water_map->h());

  for (int y = 0; y < material_map->h(); y++)
  {
    for (int x = 0; x < material_map->w(); x++)
    {
      unsigned int material = material_map->get(x,y);
      switch (small_water_map->get(x,y))
      {
        case water_map::CHUNK_EMPTY:
          material = TerrainBase::MaterialID::WATER;
          break;
        case water_map::CHUNK_MIXED:
          material |= TerrainBase::MaterialID::WATER;
        case water_map::CHUNK_FULL:
          break;
      }
      material_map->at(x,y) = material;
    }
  }

  map->setMaterialMap(material_map);

#if 1
//   LOG_INFO<<"loading noise texture ..."<<endl;
  ImageGreyScale::Ptr noise_texture = getTexture<ImageGreyScale>("APPENDIX", "ShadeNoise", "land/Noise.tga", false, loader);
  assert(noise_texture);
  map_textures->setTexture(TEXUNIT_TERRAIN_NOISE, noise_texture);
#endif

#if 1
  ImageRGBA::Ptr shallow_water = getTexture("FIELDS", "Water2", "", loader);
  assert(shallow_water);
  map_textures->setTexture(TEXUNIT_SHALLOW_WATER, shallow_water);

  vector<ImageRGBA::ConstPtr> beach;

  ImageRGBA::ConstPtr beach_foam = getTexture("APPENDIX", "BeachFoam", "", loader);
  assert(beach_foam);
  beach.push_back(beach_foam);

  ImageRGBA::ConstPtr beach_surf = getTexture("APPENDIX", "BeachSurf", "", loader);
  assert(beach_surf);
  beach.push_back(beach_surf);

  ImageRGBA::ConstPtr beach_land = getTexture("APPENDIX", "BeachLand", "", loader);
  assert(beach_land);
  beach.push_back(beach_land);

  map_textures->setBeach(beach);

  map_textures->setWaterColor(loader->getWaterColor(default_water_color));
#endif

  createForestTextures(type_map, map_textures, loader);

  createCirrusTextures(map, loader);

  LOG_INFO<<"creating map textures done."<<endl;
}


bool isForest(unsigned int index)
{
  assert(index < NUM_FIELDS);

  if (index < NUM_FIELDS)
    return
      strcmp(field_names[index], "Wood0") == 0
      ||
      strcmp(field_names[index], "Wood1") == 0
      ||
      strcmp(field_names[index], "Wood2") == 0
      ||
      strcmp(field_names[index], "Wood3") == 0
      ;
  else
    return false;
}


ImageRGBA::Ptr getTexture(const char *section,
                          const char *name,
                          const char *default_path,
                          il2ge::RessourceLoader *loader,
                          bool redirect,
                          float *scale)
{
  ImageRGBA::Ptr image;

  string dump_name = string(section) + '_' + name;

  vector<char> data;
  if (loader->readTextureFile(section, name, default_path, data, false, redirect, scale))
  {
    image = loadImageRGBAFromMemory(data, dump_name.c_str());

    dump<ImageRGBA>(image, dump_name, loader->getDumpDir());
    if (scale)
      dump(to_string(*scale), dump_name + "_scale", loader->getDumpDir());

  }

  return image;
}


void createChunks(render_util::ImageGreyScale::ConstPtr image,
                          int chunk_size,
                          std::vector<render_util::ImageGreyScale::ConstPtr> &chunks)
{
  int chunks_per_row = image->w() / chunk_size;
  int num_rows = image->h() / chunk_size;

  for (int y = 0; y < num_rows; y++)
  {
    for (int x = 0; x < chunks_per_row; x++)
    {
      ImageGreyScale::Ptr chunk =
        image::subImage(image.get(), x * chunk_size, y * chunk_size, chunk_size, chunk_size);

      chunk = image::flipY(chunk);
      chunks.push_back(chunk);
    }
  }
}


render_util::ImageGreyScale::Ptr createPixelMapH(il2ge::RessourceLoader *loader)
{
  auto height_map =
    getTexture<ImageGreyScale>("MAP", "HeightMap", "map_h.tga", true, loader);
  assert(height_map);

  return height_map;
}


render_util::ElevationMap::Ptr createElevationMap(render_util::ImageGreyScale::ConstPtr height_map)
{
  auto elevation_map = make_shared<ElevationMap>(height_map->getSize());
  for (int y = 0; y < elevation_map->h(); y++)
  {
    for (int x = 0; x < elevation_map->w(); x++)
    {
      unsigned char elevation_index = height_map->get(x,y);
      elevation_map->at(x,y) = elevation_table[elevation_index];
    }
  }

  return elevation_map;
}


unsigned getFieldIndex(const string &name)
{
  for (size_t i = 0; i < NUM_FIELDS; i++)
  {
    if (name == field_names[i])
      return i;
  }
  assert(0);
  return 0;
}


} // namespace il2ge::map_loader
