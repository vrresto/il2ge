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

#ifndef IL2GE_RESSOURCE_LOADER_H
#define IL2GE_RESSOURCE_LOADER_H


#include <string>
#include <vector>
#include <glm/glm.hpp>


namespace il2ge
{
  class RessourceLoader
  {
  public:
    virtual std::string getDumpDir() = 0;

    virtual glm::vec3 getWaterColor(const glm::vec3 &default_value) = 0;

    virtual bool readFile(const char *section,
                            const char *name,
                            const char *default_path,
                            const char *suffix,
                            std::vector<char> &content) = 0;

    virtual bool readTextureFile(const char *section,
                            const char *name,
                            const char *default_path,
                            std::vector<char> &content,
                            bool from_map_dir,
                            bool redirect,
                            float *scale = nullptr) = 0;
  };
}


#endif
