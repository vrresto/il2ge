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

#include "imf.h"
#include <il2ge/image_loader.h>
#include <render_util/image_loader.h>
#include <render_util/image.h>
#include <render_util/image_util.h>

#include <vector>

using namespace std;


namespace
{


const string g_imf_header =
{
  'I', 'M', 'F', 0x1A, 0x31, 0x30, 0x00
};


bool isIMF(const vector<char> &data)
{
  if (data.size() > g_imf_header.size())
    return g_imf_header.compare(0, string::npos, data.data(), g_imf_header.size()) == 0;
  else
    return false;
}


std::shared_ptr<render_util::GenericImage>
loadImageFromIMF(const vector<char> &data, const char *field_name)
{
  vector<unsigned char> rgba_data;
  int width, height;
  loadIMF(data, rgba_data, width, height, field_name);

  return make_shared<render_util::GenericImage>(glm::ivec2(width, height), std::move(rgba_data), 4);
}


std::shared_ptr<render_util::ImageRGBA>
loadImageRGBAFromIMF(const vector<char> &data, const char *field_name)
{
  vector<unsigned char> rgba_data;
  int width, height;
  loadIMF(data, rgba_data, width, height, field_name);

  return make_shared<render_util::ImageRGBA>(glm::ivec2(width, height), std::move(rgba_data));
}


} // namespace


namespace il2ge
{


template <>
std::shared_ptr<render_util::ImageRGBA>
loadImageFromMemory<render_util::ImageRGBA>(const std::vector<char> &data, const char *name)
{
  return loadImageRGBAFromMemory(data, name);
}


bool isIMF(util::File &file)
{
  bool is_imf = false;

  try
  {
    vector<char> data(g_imf_header.size());

    auto read = file.read(data.data(), data.size());

    if (read == data.size())
    {
      is_imf = g_imf_header.compare(0, string::npos, data.data(), data.size()) == 0;
    }
  }
  catch(...) {}

  file.rewind();

  return is_imf;
}


void getIMFInfo(util::File &file, int &w, int &h)
{
  return ::getIMFInfo(file, w, h);
}


std::unique_ptr<render_util::GenericImage>
loadIMF(const std::vector<char> &data, int force_channels)
{
  vector<unsigned char> image_data;
  int width, height;
  ::loadIMF(data, image_data, width, height, "");

  auto image = make_unique<render_util::GenericImage>(glm::ivec2(width, height),
                                                      std::move(image_data), 4);

  if (force_channels && force_channels != 4)
  {
    auto new_image = render_util::image::makeGeneric(image, force_channels);
    image = std::move(new_image);
  }

  return image;
}


std::shared_ptr<render_util::GenericImage> loadImageFromMemory(const std::vector<char> &data,
                                                               const char *name)
{
  if (::isIMF(data))
    return loadImageFromIMF(data, name);
  else
    return render_util::loadImageFromMemory<render_util::GenericImage>(data);
}


std::shared_ptr<render_util::ImageRGBA> loadImageRGBAFromMemory(const std::vector<char> &data,
                                                                const char *name)
{
  if (::isIMF(data))
    return loadImageRGBAFromIMF(data, name);
  else
    return render_util::loadImageFromMemory<render_util::ImageRGBA>(data);
}


std::shared_ptr<render_util::ImageRGB> loadImageRGBFromMemory(const std::vector<char> &data,
                                                               const char *name)
{
  assert(!::isIMF(data));
  if (::isIMF(data))
  {
    abort();
  }
  else
    return render_util::loadImageFromMemory<render_util::ImageRGB>(data);
}


} // namespace il2ge
