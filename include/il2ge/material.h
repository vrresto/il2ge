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

#ifndef IL2GE_MATERIAL_H
#define IL2GE_MATERIAL_H

#include <il2ge/parameter_file.h>
// #include <render_util/texture_manager.h>

// #include <memory>
// #include <functional>
#include <vector>
#include <string>

namespace il2ge
{

class Material
{
public:
  struct Layer
  {
    std::string texture_path;
    bool tfBlend = false;
    bool tfBlendAdd = false;
    bool tfNoTexture = false;
    bool tfNoWriteZ = false;
    bool tfTestA = false;
    float AlphaTestVal = 0.5;
  };

  Material() {}
  const std::vector<Layer> &getLayers() const { return m_layers; }
  void applyParameters(ParameterFiles &files, std::string path);

  bool tfDoubleSided = false;

private:
  std::vector<Layer> m_layers;
};


std::unique_ptr<Material> loadMaterial(ParameterFiles&, std::string path);


} // namespace il2ge

#endif
