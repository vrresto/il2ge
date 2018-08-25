/**
 *    IL-2 Graphics Extender
 *    Copyright (C) 2018 Jan Lepper
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

#include "sfs.h"
#include "sfs_p.h"

#include <iostream>
#include <string>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <windef.h>
#include <winbase.h>

using std::cout;
using std::endl;

namespace
{

  typedef int __stdcall SFS_open_T (const char *filename, int flags);
  typedef int __stdcall SFS_openf_T (unsigned __int64 hash, int flags);
  typedef int __stdcall SFS_close_T(int fd);
  typedef int __stdcall SFS_read_T (int fd, void *buf, int nbyte);
  typedef unsigned int __stdcall SFS_lseek_T (int fd, unsigned int offset, int whence);

  bool initialized = false;
  SFS_openf_T *openf_f = 0;
  SFS_close_T *close_f = 0;
  SFS_read_T *read_f = 0;
  SFS_lseek_T *lseek_f = 0;

}


namespace SFS
{


  void init()
  {
    if (!initialized)
    {
      HMODULE m = GetModuleHandle(0);
      assert(m);

      close_f = (SFS_close_T*) GetProcAddress(m, "SFS_close");
      assert(close_f);

      read_f = (SFS_read_T*) GetProcAddress(m, "SFS_read");
      assert(read_f);

      lseek_f = (SFS_lseek_T*) GetProcAddress(m, "SFS_lseek");
      assert(lseek_f);

      m = GetModuleHandle("wrapper.dll");
      assert(m);

      openf_f = (SFS_openf_T*) GetProcAddress(m, "__SFS_openf");
      assert(openf_f);

      initialized = true;
    }
  }


  __int64 getHash(const char *filename)
  {
    std::string filename_uppercase = filename;
    for (auto &c : filename_uppercase)
    {
      if (c == '/')
        c = '\\';
      else
        c = toupper(c);
    }

    __int64 hash = makeHash(0, filename_uppercase.c_str(),
                            filename_uppercase.length());
    return hash;
  }


  int open(const char *filename, int flags)
  {
    return openf(getHash(filename), 0);
  }


  int openf(uint64_t hash, int flags)
  {
    assert(openf_f);
    return openf_f(hash, flags);
  }


  int close(int fd)
  {
    return close_f(fd);
  }


  int read(int fd, void *buf, int nbyte)
  {
    return read_f(fd, buf, nbyte);
  }


  unsigned int lseek(int fd, unsigned int offset, int whence)
  {
    return lseek_f(fd, offset, whence);
  }


  bool readFile(const std::string &filename, std::vector<char> &out)
  {
    int fd = open(filename.c_str(), 0);

    if (fd == -1)
      return false;

    unsigned int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    out.resize(size);

    int ret = read(fd, out.data(), size);

    close(fd);

    return ret > 0 ? ((unsigned int)ret) == size : false;
  }


} // namespace SFS
