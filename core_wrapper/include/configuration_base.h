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

#ifndef IL2GE_CORE_WRAPPER_CONFIGURATION_BASE_H
#define IL2GE_CORE_WRAPPER_CONFIGURATION_BASE_H

#include <util.h>

#include <sstream>
#include <string>
#include <vector>
#include <functional>


namespace il2ge::core_wrapper::configuration
{


using std::to_string;


inline std::string to_string(bool value)
{
  return value ? "on" : "off";
}


template <typename T>
inline void parseValue(std::string in, T &out)
{
  std::istringstream s(in);
  s >> out;
}


template <>
inline void parseValue<bool>(std::string in, bool &out)
{
  if (in == "on" || in == "true" || in == "1")
    out = true;
  else if (in == "off" || in == "false" || in == "0")
    out = false;
}


class SettingBase
{
public:
  SettingBase(const SettingBase&) = delete;
  SettingBase& operator=(const SettingBase&) = delete;

  SettingBase(std::string name, std::string description) :
    m_name(name), m_description(description) {}

  std::string getName() { return m_name; }

  virtual void writeDescription(std::ostream &out)
  {
    auto desc = m_description;
    if (desc.empty())
      desc = "no description";
    out << "# " << desc << std::endl;
  }

  virtual void parse(std::string value) = 0;
  virtual std::string getValueStr() = 0;

private:
  std::string m_name;
  std::string m_description;
};


template <typename T>
class Setting : public SettingBase
{
public:
  Setting(std::string name, const T &default_value, std::string description) :
    SettingBase(name, description),
    m_value(default_value)
  {
  }

  const T &get() const { return m_value; }
  operator const T &() const { return get(); }

  void parse(std::string value) override
  {
    parseValue(value, m_value);
  }

  std::string getValueStr() override
  {
    return to_string(m_value);
  }

private:
  T m_value = {};
};


template <typename T>
class MultipleChoice : public SettingBase
{
public:
  struct Choice
  {
    T value;
    std::string name;
    std::vector<std::string> description_lines;
  };

  using Choices = std::vector<Choice>;

  MultipleChoice(std::string name, Choices choices, T default_value, std::string description) :
    SettingBase(name, description),
    m_choices(choices),
    m_value(default_value)
  {
  }

  const T &get() const { return m_value; }
  operator const T &() const { return get(); }

  void parse(std::string value) override
  {
    for (auto &choice : m_choices)
    {
      if (util::makeLowercase(value) == choice.name)
      {
        m_value = choice.value;
        break;
      }
    }
  }

  std::string getValueStr() override
  {
    for (auto &choice : m_choices)
    {
      if (m_value == choice.value)
      {
        return choice.name;
      }
    }
    return {};
  }

  void writeDescription(std::ostream &out) override
  {
    SettingBase::writeDescription(out);

    out << "#" << std::endl;
    out << "# choices:" << std::endl;

    for (auto &choice : m_choices)
    {
      out << "#" << std::endl;
      out << "#     " << choice.name << ":" << std::endl;
      if (!choice.description_lines.empty())
      {
        for (auto &line : choice.description_lines)
        {
          out << "#         " << line << std::endl;
        }
      }
    }
    out << "#" << std::endl;
  }

private:
  T m_value = {};
  Choices m_choices;
};


class Section
{
  std::string m_name;
  std::vector<std::unique_ptr<SettingBase>> m_settings;

public:
  Section(std::string name) : m_name(name) {}
  Section& operator=(const Section&) = delete;

  std::string getName() { return m_name; }

  virtual void write(std::ostream &out)
  {
    for (auto &setting : m_settings)
    {
      out << std::endl;
      setting->writeDescription(out);
      out << setting->getName() << "=" << setting->getValueStr() << std::endl;
    }
  }

  void read(std::function<std::string(std::string,std::string)> read_value)
  {
    for (auto &setting : m_settings)
    {
      auto value = read_value(m_name, setting->getName());
      if (!value.empty())
        setting->parse(value);
    }
  }

  void visit(std::function<void(std::string,std::string,std::string)> func)
  {
    for (auto &setting : m_settings)
      func(m_name, setting->getName(), setting->getValueStr());
  }

protected:
  template <typename T>
  T &addSetting(std::unique_ptr<T> &setting)
  {
    auto &s = *setting;
    m_settings.push_back(std::move(setting));
    return s;
  }

  template <typename T>
  Setting<T> &addSetting(std::string name, T default_value, std::string description)
  {
    auto setting = std::make_unique<Setting<T>>(name, default_value, description);
    return addSetting(setting);
  }

  template <typename T>
  MultipleChoice<T> &addMultipleChoice(std::string name,
                                       typename MultipleChoice<T>::Choices choices,
                                       T default_value,
                                       std::string description)
  {
    auto setting = std::make_unique<MultipleChoice<T>>(name, choices, default_value, description);
    return addSetting(setting);
  }
};


class ConfigurationBase : public Section
{
public:
  ConfigurationBase() : Section("") {}

  void write(std::ostream &out) override
  {
    Section::write(out);
    for (auto &section : m_sections)
    {
      out << std::endl;
      out << std::endl;
      out << "[" << section->getName() << "]" << std::endl;
      section->write(out);
    }
  }

  void read(std::function<std::string(std::string,std::string)> read_value)
  {
    Section::read(read_value);
    for (auto &section: m_sections)
      section->read(read_value);
  }

  void visit(std::function<void(std::string,std::string,std::string)> func)
  {
    Section::visit(func);
    for (auto &section: m_sections)
      section->visit(func);
  }

protected:
  template <class T>
  T &addSection()
  {
    auto section = std::make_unique<T>();
    auto &ref = *section;
    m_sections.push_back(std::move(section));
    return ref;
  }

private:
  std::vector<std::unique_ptr<Section>> m_sections;
};


}

#endif
