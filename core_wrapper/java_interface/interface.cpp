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


#include <il2ge_java_interface/interface.h>
#include <java_util.h>
#include <core.h>
#include <config.h>
#include <gl_wrapper.h>
#include <wgl_wrapper.h>
#include <core/scene.h>
#include <log.h>

#include <iostream>
#include <sstream>
#include <unordered_map>

using namespace std;

namespace
{


struct Command
{
  using ExecFunc = std::function<void()>;

  ExecFunc execute;
  bool is_debug_command = false;
};


unordered_map<string, Command> g_commands;
vector<string> g_command_names;
bool g_java_classes_initialized = false;


void addCommand(string name, Command::ExecFunc exec, bool is_debug_command = false)
{
  g_commands[name] = { exec, is_debug_command };
  g_command_names.push_back(name);
}


void addParameterCommands(int index)
{
  auto &p = wgl_wrapper::getScene()->getParameter(index);

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
        auto &p = wgl_wrapper::getScene()->getParameter(index);
        p.set(p.get() - increment);
      };
      addCommand(name, command);
    }

    {
      auto name = p.name + ".increase_by_" + increment_str;
      auto command = [index, increment] ()
      {
        auto &p = wgl_wrapper::getScene()->getParameter(index);
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
  if (g_java_classes_initialized)
    return;

  for (int i = 0; i < wgl_wrapper::getScene()->getNumParameters(); i++)
    addParameterCommands(i);

#if ENABLE_SHORTCUTS
  addCommand("ToggleEnable", &core_gl_wrapper::toggleEnable, true);
  addCommand("ToggleObjectShaders", &core_gl_wrapper::toggleObjectShaders, true);
  addCommand("ToggleTransparentShader", &core_gl_wrapper::toggleTransparentShader, true);
  addCommand("ToggleTerrain", &core_gl_wrapper::toggleTerrain, true);
#endif

  LOG_INFO << "Loading class com/maddox/il2ge/HotKeys ..." << endl;
  il2ge::java::getEnv()->FindClass("com/maddox/il2ge/HotKeys");

  if (il2ge::java::getEnv()->ExceptionCheck())
  {
    LOG_WARNING  << "Exception encountered when loading class com/maddox/il2ge/HotKeys." << endl;
    il2ge::java::getEnv()->ExceptionDescribe();
  }
  else
  {
    LOG_INFO << "Loading class com/maddox/il2ge/HotKeys ... done." << endl;
  }

  il2ge::java::getEnv()->ExceptionClear();

  g_java_classes_initialized = true;
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


bool isDebugCommand(std::string name)
{
  return g_commands.at(name).is_debug_command;
}


void executeCommand(std::string command_name)
{
  auto it = g_commands.find(command_name);

  if (it != g_commands.end())
  {
    it->second.execute();
  }
  else
  {
    std::cout << "IL2GE: Warning: No such command: " << command_name << std::endl;
  }
}


}
