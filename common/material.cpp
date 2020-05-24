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

#include <il2ge/material.h>
#include <util.h>

using namespace std;


namespace il2ge
{


constexpr int MAX_LAYERS = 8;


void Material::applyParameters(ParameterFiles &files, std::string path)
{
  auto dir = util::getDirFromPath(path);
  assert(!dir.empty());
  auto &params = files.get(path);

  if (params.hasSection("ClassInfo"))
  {
    auto &class_info = params.getSection("ClassInfo");

    std::string based_on;
    class_info.get_noexcept("BasedOn", based_on);

    if (!based_on.empty())
      applyParameters(files, dir + '/' + based_on);
  }

  if (params.hasSection("General"))
  {
    auto &general = params.getSection("General");
    general.get_noexcept("tfDoubleSide", this->tfDoubleSide);
  }

  for (size_t i = 0; i < MAX_LAYERS; i++)
  {
    string layer_name = "Layer" + to_string(i);

    if (params.hasSection(layer_name))
    {
      auto &section = params.getSection(layer_name.c_str());

      if (m_layers.size() < (i+1))
        m_layers.resize(i+1);

      assert(m_layers.size() > i);
      auto &layer = m_layers.at(i);

      string texture_name;
      section.get_noexcept("TextureName", texture_name);

      #define GET_PARAMETER(p) { section.get_noexcept(#p, layer.p); }
      GET_PARAMETER(tfBlend);
      GET_PARAMETER(tfBlendAdd);
      GET_PARAMETER(tfNoTexture);
      GET_PARAMETER(tfNoWriteZ);
      GET_PARAMETER(AlphaTestVal);
      GET_PARAMETER(tfTestA);
      #undef GET_PARAMETER

      if (!texture_name.empty())
        layer.texture_path = util::resolveRelativePathComponents(dir + '/' + texture_name);
    }
  }
}


std::unique_ptr<Material> loadMaterial(ParameterFiles &files, std::string path)
{
  auto mat = std::make_unique<Material>();
  mat->applyParameters(files, path);

  return mat;
}


} // namespace il2ge
