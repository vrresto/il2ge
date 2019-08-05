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

#include <vector>

using namespace std;


namespace
{


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


std::shared_ptr<render_util::GenericImage> loadImageFromMemory(const std::vector<char> &data,
                                                               const char *name)
{
  if (isIMF(data))
    return loadImageFromIMF(data, name);
  else
    return render_util::loadImageFromMemory<render_util::GenericImage>(data);
}


std::shared_ptr<render_util::ImageRGBA> loadImageRGBAFromMemory(const std::vector<char> &data,
                                                                const char *name)
{
  if (isIMF(data))
    return loadImageRGBAFromIMF(data, name);
  else
    return render_util::loadImageFromMemory<render_util::ImageRGBA>(data);
}


std::shared_ptr<render_util::ImageRGB> loadImageRGBFromMemory(const std::vector<char> &data,
                                                               const char *name)
{
  assert(!isIMF(data));
  if (isIMF(data))
  {
    abort();
  }
  else
    return render_util::loadImageFromMemory<render_util::ImageRGB>(data);
}


} // namespace il2ge
