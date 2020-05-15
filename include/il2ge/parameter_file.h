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

#ifndef IL2GE_PARAMETER_FILE_H
#define IL2GE_PARAMETER_FILE_H

#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <functional>
#include <memory>

namespace il2ge
{


class ParameterFile
{
public:
  class Section
  {
    friend class ParameterFile;

  public:
    template <typename T>
    bool get(const char *name, T &value) const
    {
      try
      {
        getImp(name, value);
        return true;
      }
      catch(std::exception &e)
      {
        std::cout<<"failed to get parameter "<<name<<" : "<<e.what()<<std::endl;
        return false;
      }
    }

    std::string get(const char *param) const
    {
      std::string s;
      get(param, s);
      return std::move(s);
    }

    const std::string &at(const char *param) const;

  private:
    void getImp(const char *name, std::string &value) const
    {
      value = m_values.at(name).at(0);
    }

    void getImp(const char *name, int &value) const
    {
      value = std::stoi(m_values.at(name).at(0));
    }

    void getImp(const char *name, bool &value) const
    {
      value = std::stoi(m_values.at(name).at(0));
    }

    void getImp(const char *name, float &value) const
    {
      value = std::stof(m_values.at(name).at(0));
    }

    void getImp(const char *name, glm::vec4 &value) const;
    void getImp(const char *name, glm::vec3 &value) const;
    void getImp(const char *name, glm::vec2 &value) const;

    std::unordered_map<std::string, std::vector<std::string>> m_values;
  };

  ParameterFile(const char *content, size_t size);

  const Section &getSection(const char *name) const;

private:
  std::unordered_map<std::string, Section> m_sections;
};


class ParameterFiles
{
public:
  using ReadFileFunc = std::function<std::vector<char>(std::string)>;

  ParameterFiles(ReadFileFunc f) : m_read_file(f) {}

  const ParameterFile &get(const std::string &file_path);

private:
  ReadFileFunc m_read_file;
  std::unordered_map<std::string, std::unique_ptr<ParameterFile>> m_file_map;
};


}

#endif
