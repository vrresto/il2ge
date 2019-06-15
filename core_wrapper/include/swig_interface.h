#ifndef IL2GE_SWIG_INTERFACE_H
#define IL2GE_SWIG_INTERFACE_H

#include <string>

namespace il2ge::java_interface
{
  int getInterfaceVersion();
  int getNumCommands();
  std::string getCommandName(int index);
  void executeCommand(int index);
}

#endif
