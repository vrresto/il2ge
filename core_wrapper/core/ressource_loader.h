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

#ifndef CORE_RESSOURCE_LOADER_H
#define CORE_RESSOURCE_LOADER_H

#include <il2ge/ressource_loader.h>
#include <INIReader.h>

#include <memory>

namespace core
{


  class RessourceLoader : public il2ge::RessourceLoader
  {
    std::unique_ptr<INIReader> reader;
    std::string map_dir;
    std::string dump_dir;

  public:
    RessourceLoader(const std::string &map_dir, const std::string &ini_path, const std::string &dump_path);

    std::string getDumpDir() override;

    glm::vec3 getWaterColor(const glm::vec3 &default_value) override;

    bool readFile(const char *section,
              const char *name,
              const char *default_path,
              const char *suffix,
              std::vector<char> &content) override;

    bool readTextureFile(const char *section,
              const char *name,
              const char *default_path,
              std::vector<char> &content,
              bool from_map_dir,
              bool redirect,
              float *scale,
              bool is_bumpmap) override;

    bool readWaterAnimation(const std::string &file_name, std::vector<char> &content) override;

  };


}

#endif
