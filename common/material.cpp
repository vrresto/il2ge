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


Material::Material(const ParameterFile &params, const std::string &dir)
{
  for (size_t i = 0; i < MAX_LAYERS; i++)
  {
    string layer_name = "Layer" + to_string(i);
    string texture_name;

    try
    {
      auto &section = params.getSection(layer_name.c_str());
      texture_name = section.get("TextureName");
    }
    catch(...)
    {
      break;
    }

    if (texture_name.empty())
      break;

    Layer layer { util::resolveRelativePathComponents(dir + '/' + texture_name) };

    m_layers.push_back(layer);
  }
}


} // namespace il2ge
