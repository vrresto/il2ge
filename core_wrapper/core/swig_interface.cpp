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


#include "core_p.h"
#include <swig_interface.h>
#include <java_util.h>
#include <core.h>
#include <core/scene.h>

#include <iostream>
#include <sstream>
#include <unordered_map>

using namespace std;

namespace
{


using Command = std::function<void()>;


unordered_map<string, Command> g_commands;
vector<string> g_command_names;
bool g_java_classes_initialized = false;


void addCommand(string name, Command command)
{
  g_commands[name] = command;
  g_command_names.push_back(name);
}


void addParameterCommands(int index)
{
  auto &p = core::getScene()->getParameter(index);

  vector<float> increments { 0.1, 1.0, 10.0, 100.0 };

  for (auto increment : increments)
  {
    ostringstream increment_stream;
    increment_stream << increment;

    auto increment_str = increment_stream.str();

    {
      auto name = p.name + ".decrease_by_" + increment_str;
      auto command = [index, increment] ()
      {
        auto &p = core::getScene()->getParameter(index);
        p.set(p.get() - increment);
      };
      addCommand(name, command);
    }

    {
      auto name = p.name + ".increase_by_" + increment_str;
      auto command = [index, increment] ()
      {
        auto &p = core::getScene()->getParameter(index);
        p.set(p.get() + increment);
      };
      addCommand(name, command);
    }
  }
}


}


namespace core
{


void initJavaClasses()
{
  if (!g_java_classes_initialized)
  {
    Scene *scene = getScene();

    for (int i = 0; i < scene->getNumParameters(); i++)
      addParameterCommands(i);

    cout << "loading class com/maddox/il2ge/HotKeys ..." << endl;
    il2ge::java::getEnv()->FindClass("com/maddox/il2ge/HotKeys");
    cout << "loading class com/maddox/il2ge/HotKeys ... done." << endl;

    il2ge::java::getEnv()->ExceptionClear();

    g_java_classes_initialized = true;
  }
}


}


namespace il2ge::java_interface
{


void showMenu(bool show)
{
  core::showMenu(show);
}


void handleKey(int key, bool ctrl, bool alt, bool shift)
{
  core::handleKey(key, ctrl, alt, shift);
}


int getNumCommandNames()
{
  return g_command_names.size();
}


std::string getCommandName(int index)
{
    return g_command_names.at(index);
}


std::string getCommandDisplayText(std::string command_name)
{
    return {};
}


void executeCommand(std::string command_name)
{
  auto it = g_commands.find(command_name);

  if (it != g_commands.end())
  {
    it->second();
  }
  else
  {
    std::cout << "IL2GE: Warning: No such command: " << command_name << std::endl;
  }
}


}
