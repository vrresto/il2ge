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


namespace il2ge::core_wrapper
{


class ConfigurationBase
{
public:
  class SettingBase
  {
  public:
    SettingBase(const SettingBase&) = delete;
    SettingBase& operator=(const SettingBase&) = delete;

    SettingBase(std::string name, std::string description) :
      m_name(name), m_description(description) {}

    std::string getName() { return m_name; }
    virtual std::string getDescription() { return m_description; }

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
      std::istringstream s(value);
      s >> m_value;
    }

    std::string getValueStr() override
    {
      return std::to_string(m_value);
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
      std::string description;
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

    std::string getDescription() override
    { 
      auto desc = SettingBase::getDescription();

      desc += "\n# choices:";

      for (auto &choice : m_choices)
      {
        desc += "\n#   " + choice.name;
        if (!choice.description.empty())
          desc += " (" + choice.description + ")";
      }

      return desc;
    }

  private:
    T m_value = {};
    Choices m_choices;
  };


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


  void write(std::ostream &out)
  {
    for (auto &setting : m_settings)
    {
      auto desc = setting->getDescription();
      if (desc.empty())
        desc = "no description";
      out << "# " << desc << std::endl;
      out << setting->getName() << "=" << setting->getValueStr() << std::endl;
      out << std::endl;
    }
  }


  void read(std::function<std::string(std::string)> read_value)
  {
    for (auto &setting : m_settings)
    {
      auto value = read_value(setting->getName());
      if (!value.empty())
        setting->parse(value);
    }
  }


  const std::vector<std::unique_ptr<SettingBase>> &getSettings() const
  {
    return m_settings;
  }


private:
  std::vector<std::unique_ptr<SettingBase>> m_settings;
};


}

#endif
