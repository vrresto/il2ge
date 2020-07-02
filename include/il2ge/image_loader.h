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

#ifndef IL2GE_IMAGE_LOADER_H
#define IL2GE_IMAGE_LOADER_H

#include <render_util/image.h>
#include <render_util/image_loader.h>
#include <file.h>

namespace il2ge
{


template <class T>
std::shared_ptr<T> loadImageFromMemory(const std::vector<char> &data, const char *name)
{
  return render_util::loadImageFromMemory<T>(data);
}

template <>
std::shared_ptr<render_util::ImageRGBA>
loadImageFromMemory<render_util::ImageRGBA>(const std::vector<char> &data, const char *name);

bool isIMF(util::File &file);
void getIMFInfo(util::File&, int &w, int &h);
std::unique_ptr<render_util::GenericImage> loadIMF(const std::vector<char> &data, int force_channels);

std::shared_ptr<render_util::GenericImage> loadImageFromMemory(const std::vector<char> &data,
                                                               const char *name);

std::shared_ptr<render_util::ImageRGBA> loadImageRGBAFromMemory(const std::vector<char> &data,
                                                                const char *name);

std::shared_ptr<render_util::ImageRGB> loadImageRGBFromMemory(const std::vector<char> &data,
                                                              const char *name);

}

#endif
