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

#ifndef IL2GE_MAP_LOADER
#define IL2GE_MAP_LOADER

#include <il2ge/ressource_loader.h>
#include <render_util/image.h>
#include <render_util/image_loader.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace il2ge
{


const bool dump_enabled = false;

bool isForest(unsigned int index);


template <typename T>
void dump(std::shared_ptr<T> image, const std::string &name, const std::string &dump_dir)
{
  if (dump_enabled && !dump_dir.empty())
  {
    std::string dump_path = dump_dir + name + ".tga";
    render_util::saveImageToFile<T>(dump_path, image.get());
  }
}


template <typename T>
void dump(const std::vector<typename T::ConstPtr> &images, const std::string &name, const std::string &dump_dir)
{
  if (dump_enabled && !dump_dir.empty())
  {
    for (size_t i = 0; i < images.size(); i++)
    {
      std::stringstream dump_path;
      dump_path << dump_dir << name << '_' << i << ".tga";
      render_util::saveImageToFile<T>(dump_path.str(), images[i].get());
    }
  }
}


render_util::ImageRGBA::Ptr getTexture(const char *section,
                          const char *name,
                          const char *default_path,
                          il2ge::RessourceLoader *loader,
                          bool redirect = false,
                          float *scale = 0);

template <typename T>
typename T::Ptr getTexture(const char *section,
                          const char *name,
                          const char *default_path,
                          bool from_map_dir,
                          il2ge::RessourceLoader *loader)
{
  using namespace std;

  typename T::Ptr image;

  std::string dump_name = std::string(section) + '_' + name;

  std::vector<char> data;
  if (loader->readTextureFile(section, name, default_path, data, from_map_dir, false))
  {
    image.reset(render_util::loadImageFromMemory<T>(data));
    dump<T>(image, dump_name, loader->getDumpDir());
  }
  else
    cout<<"getTexture() "<<name<<" error"<<endl;

  return image;
}


void createChunks(render_util::ImageGreyScale::ConstPtr image, int chunk_size, std::vector<render_util::ImageGreyScale::ConstPtr> &chunks);


} // namespace il2ge

#endif