#ifndef IL2GE_SWIG_INTERFACE_H
#define IL2GE_SWIG_INTERFACE_H

#include <string>

namespace il2ge::java_interface
{
  int getInterfaceVersion();

  int getNumCommandNames();
  std::string getCommandName(int index);

  std::string getCommandDisplayText(std::string command_name);
  bool isDebugCommand(std::string name);
  void executeCommand(std::string command_name);

  void showMenu(bool show);
  void handleKey(int key, bool ctrl, bool alt, bool shift);
}

#endif
