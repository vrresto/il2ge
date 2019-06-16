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


#include <swig_interface.h>

#include <iostream>


namespace il2ge::java_interface
{


int getNumCommands()
{
  return 1;
}


std::string getCommandName(int index)
{
  if (index == 0)
    return "DummyCommand";
  else
    return {};
}


std::string getCommandDisplayText(int index)
{
  if (index == 0)
    return "Dummy Command";
  else
    return {};
}


void executeCommand(int index)
{
  if (index == 0)
    std::cout << "IL2GE: Dummy command successfully executed." << std::endl;
  else
    std::cout << "IL2GE: Warning: No command for index " << index << std::endl;
}


}
