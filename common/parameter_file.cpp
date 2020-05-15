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

#include <il2ge/parameter_file.h>
#include <util.h>

#include <glm/glm.hpp>
#include <sstream>

using namespace std;


namespace
{


void stripComment(string &line)
{
  static const std::vector<string> prefixes { "//" };

  for (auto &p : prefixes)
  {
    auto comment_start = line.find(p);
    if (comment_start != string::npos)
    {
      line = move(line.substr(0, comment_start));
      return;
    }
  }
}


} // namespace


namespace il2ge
{


const std::string &ParameterFile::Section::at(const char *param) const
{
  return m_values.at(param).at(0);
}


void ParameterFile::Section::getImp(const char *name, glm::vec2 &value_) const
{
  glm::vec4 value{0};
  getImp(name, value);
  value_ = glm::vec2(value.x, value.y);
}


void ParameterFile::Section::getImp(const char *name, glm::vec4 &value_) const
{
  try
  {
    glm::vec4 value{0};

    auto &tokens = m_values.at(name);

//     try
//     {
      if (tokens.size() > 0)
        value.x = std::stof(tokens.at(0));
      if (tokens.size() > 1)
        value.y = std::stof(tokens.at(1));
      if (tokens.size() > 2)
        value.z = std::stof(tokens.at(2));
      if (tokens.size() > 3)
        value.w = std::stof(tokens.at(3));
//     }
//     catch (...)
//     {
//       cout<<"stof"<<endl;
//       abort();
//     }

    value_ = value;
  }
  catch(std::out_of_range)
  {
    cout<<"no such parameter: "<<name<<endl;
    throw;
  }
}


ParameterFile::ParameterFile(const char *content, size_t size)
{
  assert(content);
  assert(size);

  stringstream in(string(content, size));

  Section *section = nullptr;

  while (in.good())
  {
    string line;
    getline(in, line);

    stripComment(line);
    line = util::trim(line);

    if (line.empty())
    {
      continue;
    }
    else if (line.front() == '[')
    {
      if (line.back() != ']')
      {
        cout<<"unterminated section: "<<line<<endl;
        throw std::exception();
      }

      string section_name = line.substr(1, line.size()-2);
      section = &m_sections[section_name];
    }
    else
    {
      assert(section);

      auto tokens = util::tokenize(line);
      if (!tokens.empty())
      {
        string key = tokens.front();
        tokens.erase(tokens.begin());

        vector<string> values;

        for (auto &t : tokens)
        {
          string value = util::trim(t);
          if (!value.empty())
          {
            if (util::isPrefix("//", value))
              break;
            values.push_back(move(value));
          }
        }

        section->m_values[key] = move(values);
      }
    }

  }
}


const ParameterFile::Section &ParameterFile::getSection(const char *name) const
{
  return m_sections.at(name);
}


const ParameterFile &ParameterFiles::get(const string &file_path)
{
  auto &file = m_file_map[file_path];
  if (!file)
  {
    auto content = m_read_file(file_path);

    try
    {
      file = make_unique<ParameterFile>(content.data(), content.size());
    }
    catch(...)
    {
      cout<<"error in parameter file: "<<file_path<<endl;
      throw;
    }
  }

  assert(file);
  return *file;
}


} // namespace il2ge
