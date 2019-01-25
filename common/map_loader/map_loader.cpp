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
#include "imf.h"
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

#include <FastNoise.h>
#include <glm/glm.hpp>

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

#include <gl_wrapper/gl_functions.h>

#ifdef max
#undef max
#endif

using namespace std;
using namespace render_util;
using namespace glm;
using namespace gl_wrapper::gl_functions;
using namespace il2ge;
using namespace il2ge::map_loader;

namespace
{


const vec3 default_water_color = vec3(45,51,40) / vec3(255);

void dumpFile(string name, const char *data, size_t data_size, const string &dump_dir)
{
  if (!isDumpEnabled() || dump_dir.empty())
    return;
  util::writeFile(dump_dir + '/' +  name, data, data_size);
}

bool isIMF(const vector<char> &data)
{
  static const string imf_header =
  {
    'I', 'M', 'F', 0x1A, 0x31, 0x30, 0x00
  };

  if (data.size() > imf_header.size())
    return imf_header.compare(0, string::npos, data.data(), imf_header.size()) == 0;
  else
    return false;
}

ImageRGBA::Ptr loadImageFromIMF(const vector<char> &data, const char *field_name)
{
  vector<unsigned char> rgba_data;
  int width, height;
  loadIMF(data, rgba_data, width, height, field_name);

  return make_shared<ImageRGBA>(width, height, std::move(rgba_data));
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


enum
{
  METERS_PER_TILE = 1600
};


float elevation_table[256] =
{
  #include "height_table"
};


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

    water_map.table.reset(new Image<unsigned int>(table_size.x,
                                                          table_size.y,
                                                          data_size * sizeof(unsigned int),
                                                          reinterpret_cast<unsigned char*>(data.data())));
    water_map.table = image::flipY(water_map.table);

    il2ge::convertWaterMap(water_map, map, small_water_map);
  }
}


int getTerrainTextureArrayIndex(int size)
{
  constexpr double SMALLEST_SIZE = 256;

  int index = glm::log2(double(size)) - glm::log2(SMALLEST_SIZE);
  assert(index >= 0);
  assert(index < MAX_TERRAIN_TEXUNITS);
  return index;
}


void createFieldTextures(ImageGreyScale::ConstPtr type_map_,
                         render_util::MapTextures *map_textures,
                         il2ge::RessourceLoader *loader)
{
  using TextureArray = vector<ImageRGBA::ConstPtr>;

  std::array<TextureArray, MAX_TERRAIN_TEXUNITS> texture_arrays;
  TextureArray textures_single_array;
#if 0
  auto red_texture = make_shared<ImageRGBA>(ivec2(512));
  image::fill(red_texture, ImageRGBA::PixelType{255, 0, 0, 255});

  texture_arrays[getTerrainTextureArrayIndex(red_texture->w())].push_back(red_texture);
  textures_single_array.push_back(red_texture);
#endif
  map<unsigned, glm::uvec3> mapping;
  map<unsigned, unsigned> orig_to_single_array_mapping;

//   ImageRGBA::Ptr default_normal_map(new ImageRGBA(ivec2(128,128)));
//   RGBA default_normal = { 127, 127, 255, 255 };
//   image::fill<RGBA>(default_normal_map, default_normal);
//   vector<ImageRGBA::ConstPtr> normal_maps;

  for (int i = 0; i < NUM_FIELDS; i++)
  {
    const char *field_name = field_names[i];

    cout<<"loading texture: "<<field_name<<" ..."<<endl;

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
    if (!image)
      continue;

    image = image::flipY(image);
    assert(image->w() == image->h());

    assert(scale != 0);
    assert(fract(scale) == 0);
    assert(scale <= render_util::MapTextures::MAX_TERRAIN_TEXTURE_SCALE);

    scale += 128;

    assert(scale >= 0);
    assert(scale <= 255);

    auto index = getTerrainTextureArrayIndex(image->w());
    assert(index < MAX_TERRAIN_TEXUNITS);

    texture_arrays[index].push_back(image);
    textures_single_array.push_back(image);

    mapping.insert(make_pair(i, glm::uvec3{index, texture_arrays[index].size()-1, scale}));

    orig_to_single_array_mapping.insert(make_pair(i, textures_single_array.size()-1));
  }

  auto type_map = make_shared<ImageRGBA>(type_map_->getSize());
  auto type_map_single_array = make_shared<ImageGreyScale>(type_map_->getSize());

  cout << "uploading textures ..." <<endl;
  for (size_t i = 0; i < MAX_TERRAIN_TEXUNITS; i++)
  {
    if (!texture_arrays[i].empty())
      map_textures->setTextures(texture_arrays[i], i);
  }
  cout << "uploading textures ... done." <<endl;

  for (int y = 0; y < type_map->h(); y++)
  {
    for (int x = 0; x < type_map->w(); x++)
    {
      unsigned int orig_index = type_map_->get(x,y) & 0x1F;

      if (strcmp(field_names[orig_index], "Wood1") == 0 ||
          strcmp(field_names[orig_index], "Wood3") == 0)
      {
        orig_index -= 1;
      }

#if 0
      uvec3 new_index{getTerrainTextureArrayIndex(red_texture->w()), 0, 1};
#else
      uvec3 new_index{0};
#endif
      try
      {
        new_index = mapping.at(orig_index);
      }
      catch(...)
      {
        for (int i = 0; i < 4; i++)
        {
          try
          {
            new_index = mapping.at(orig_index - (orig_index % 4) + i);
            break;
          }
          catch(...) {}
        }
      }
      assert(new_index.y <= 0x1F+1);
      type_map->at(x,y,0) = new_index.x;
      type_map->at(x,y,1) = new_index.y;
      type_map->at(x,y,2) = new_index.z;
      type_map->at(x,y,3) = 255;

      unsigned new_index_single_array = 0;
      try
      {
        new_index_single_array = orig_to_single_array_mapping.at(orig_index);
      }
      catch (...)
      {
        for (int i = 0; i < 4; i++)
        {
          try
          {
            new_index_single_array = orig_to_single_array_mapping.at(orig_index - (orig_index % 4) + i);
            break;
          }
          catch(...) {}
        }
      }
      assert(new_index_single_array <= 0x1F+1);
      type_map_single_array->at(x,y) = new_index_single_array;
    }
  }

  dump(type_map, "type_map", loader->getDumpDir());

  map_textures->setTypeMap(type_map);

  cout << "generating far texture ..." <<endl;

  ImageRGBA::Ptr far_texture =
    createMapFarTexture(type_map_single_array,
                        textures_single_array,
                        TYPE_MAP_METERS_PER_PIXEL,
                        METERS_PER_TILE);
  map_textures->setTexture(TEXUNIT_TERRAIN_FAR, far_texture);

  dump(far_texture, "far_texture", loader->getDumpDir());
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




void createWaterNormalMaps(render_util::WaterAnimation *water_animation,
                           render_util::MapTextures *map_textures,
                           il2ge::RessourceLoader *loader)
{

  printf("loading water textures...\n");

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
    char basename[100];

    snprintf(basename, sizeof(basename), "WaterNoise%.2dDot3", i);
    auto filename = string(basename) + ".tga";
    cout << "loading " << filename << endl;
    vector<char> data;
    if (!loader->readWaterAnimation(filename, data))
    {
      break;
    }
    auto normal_map = loadImageFromMemory(data, filename.c_str());
    assert(normal_map);
    dump(normal_map, basename, loader->getDumpDir());
    normal_maps.push_back(normal_map);

    snprintf(basename, sizeof(basename), "WaterNoiseFoam%.2d", i);
    filename = string(basename) + ".tga";
    cout << "loading " << filename << endl;;
    if (!loader->readWaterAnimation(filename, data))
    {
      break;
    }
    auto foam_mask = render_util::loadImageFromMemory<ImageGreyScale>(data);
    assert(foam_mask);
    dump(foam_mask, basename, loader->getDumpDir());
    foam_masks.push_back(foam_mask);

    i++;
  }

  assert(normal_maps.size() == foam_masks.size());

  water_animation->createTextures(map_textures, normal_maps, foam_masks);
}


} // namespace


namespace il2ge::map_loader
{


void createMapTextures(il2ge::RessourceLoader *loader,
             render_util::MapTextures *map_textures,
             render_util::WaterAnimation *water_animation,
             render_util::TerrainBase::MaterialMap::Ptr &material_map)
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

  cout<<"loading type map ..."<<endl;
  auto type_map = getTexture<ImageGreyScale>("MAP", "TypeMap", "map_T.tga", true, loader);
  assert(type_map);
  type_map = image::flipY(type_map);

  createWaterNormalMaps(water_animation, map_textures, loader);

#if 1
  cout<<"creating water map ..."<<endl;
  il2ge::WaterMap water_map;
  render_util::Image<water_map::ChunkType>::Ptr small_water_map;
  createWaterMap(
    type_map->size(),
    loader,
    water_map,
    small_water_map);
  cout<<"creating water map done."<<endl;
  map_textures->setWaterMap(water_map.chunks, water_map.table);
#endif

  createWaterTypeMap(type_map, map_textures, loader);

#if 1
//   cout<<"loading noise texture ..."<<endl;
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

  createFieldTextures(type_map, map_textures, loader);
  createForestTextures(type_map, map_textures, loader);

  assert(!material_map);

  material_map = image::convert<TerrainBase::MaterialMap::ComponentType>(type_map);

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

  cout<<"creating map textures done."<<endl;
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
    if (isIMF(data))
      image = loadImageFromIMF(data, dump_name.c_str());
    else
      image = render_util::loadImageFromMemory<ImageRGBA>(data);

    dump<ImageRGBA>(image, dump_name, loader->getDumpDir());
    if (scale)
      dump(to_string(*scale), dump_name + "_scale", loader->getDumpDir());

  }

  return image;
}


render_util::ImageRGBA::Ptr loadImageFromMemory(const std::vector<char> &data,
                                                       const char *name)
{
  if (isIMF(data))
    return loadImageFromIMF(data, name);
  else
    return render_util::loadImageFromMemory<ImageRGBA>(data);
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


render_util::ElevationMap::Ptr createElevationMap(il2ge::RessourceLoader *loader)
{
  auto height_map =
    getTexture<ImageGreyScale>("MAP", "HeightMap", "map_h.tga", true, loader);
  assert(height_map);

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
