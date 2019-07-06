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

package com.maddox.il2ge;

import com.maddox.rts.*;

public class HotKeys
{
  static boolean created = false;


  static class CommandHotKey extends HotKeyCmd
  {
    String name;

    public CommandHotKey(String name, String sortingName)
    {
      super(true, "GraphicsExtender." + name);
      this.name = name;
    }

    public void end()
    {
      GraphicsExtender.executeCommand(name);
    }
  }


  static class ShowMenuHotKey extends HotKeyCmd
  {
    public ShowMenuHotKey()
    {
      super(true, "GraphicsExtender.ShowMenu");
    }

    public void end()
    {
      Keyboard.adapter().setFocus(new KeyboardListener());
      GraphicsExtender.showMenu(true);
    }
  }


  static class KeyboardListener implements MsgKeyboardListener
  {
    public void msgKeyboardKey(int key, boolean pressed)
    {
      if (!pressed && key == VK.ESCAPE)
      {
        GraphicsExtender.showMenu(false);
        Keyboard.adapter().setFocus(null);
      }
      else if (pressed)
      {
        boolean ctrlPressed = Keyboard.adapter().isPressed(VK.CONTROL);
        boolean altPressed = Keyboard.adapter().isPressed(VK.ALT);
        boolean shiftPressed = Keyboard.adapter().isPressed(VK.SHIFT);

        GraphicsExtender.handleKey(key, ctrlPressed, altPressed, shiftPressed);
      }
    }

    public void msgKeyboardChar(char c) {}
  }


  public static void create()
  {
    if (created)
      return;

    if (!GraphicsExtender.IS_AVAILABLE)
      return;

    HotKeyCmdEnv.addCmd("hotkeys", new ShowMenuHotKey());

    for (int i = 0; i < GraphicsExtender.getNumCommandNames(); i++)
    {
      String name = GraphicsExtender.getCommandName(i);
      String text = GraphicsExtender.getCommandDisplayText(name);

      HotKeyCmdEnv.addCmd("misc", new CommandHotKey(name, text));
    }
  }


  static
  {
    create();
  }
}
