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
#include <il2ge/core_wrapper.h>

#include <iostream>
#include <string>
#include <unordered_map>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <windef.h>
#include <winbase.h>

using std::cout;
using std::endl;
using std::unordered_map;

// #define SFS_API __cdecl
#define SFS_API __stdcall


namespace
{

typedef int __cdecl SAS_SFS_openf_T(unsigned __int64 hash, int flags);
typedef int SFS_API SFS_close_T(int fd);
typedef int SFS_API SFS_read_T(int fd, void *buffer, unsigned int numberOfBytesToRead);
typedef long SFS_API SFS_lseek_T(int fd, long offset, int moveMethod);

SAS_SFS_openf_T wrap_SFS_openf;

bool g_initialized = false;
SAS_SFS_openf_T *g_openf_func = nullptr;
SFS_close_T *g_close_func = nullptr;
SFS_read_T *g_read_func = nullptr;
SFS_lseek_T *g_lseek_func = nullptr;
unordered_map<__int64, __int64> g_redirections;


int open(const char *filename)
{
  return g_openf_func(sfs::getHash(filename), 0);
}


int __cdecl wrap_SFS_openf(unsigned __int64 hash, int flags)
{
  assert(g_openf_func);

//   auto it = g_redirections.find(hash);
//   if (it != g_redirections.end())
//     hash = it->second;

  return g_openf_func(hash, flags);
}


} // namespace


namespace sfs
{

void init()
{
  if (!g_initialized)
  {
    HMODULE m = GetModuleHandle(0);
    assert(m);

    g_close_func = (SFS_close_T*) GetProcAddress(m, "SFS_close");
    assert(g_close_func);

    g_read_func = (SFS_read_T*) GetProcAddress(m, "SFS_read");
    assert(g_read_func);

    g_lseek_func = (SFS_lseek_T*) GetProcAddress(m, "SFS_lseek");
    assert(g_lseek_func);

    m = GetModuleHandle("wrapper.dll");
    assert(m);

    g_openf_func = (SAS_SFS_openf_T*) GetProcAddress(m, "__SFS_openf");
    assert(g_openf_func);

    g_initialized = true;
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

  __int64 hash = sfs_private::makeHash(0, filename_uppercase.c_str(),
                                       filename_uppercase.length());
  return hash;
}


bool readFile(const std::string &filename, std::vector<char> &out)
{
  auto fd = open(filename.c_str());

  if (fd == -1)
    return false;

  auto size = g_lseek_func(fd, 0, SEEK_END);
  g_lseek_func(fd, 0, SEEK_SET);

  out.resize(size);

  auto ret = g_read_func(fd, out.data(), size);

  g_close_func(fd);

  return ret > 0 ? ((unsigned int)ret) == size : false;
}


void redirect(__int64 hash, __int64 hash_redirection)
{
  g_redirections[hash] = hash_redirection;
}

void clearRedirections()
{
  cout<<"SFS wrapper: clearing "<<g_redirections.size()<<" redirections."<<endl;
  g_redirections.clear();
}


} // namespace sfs


void *il2ge::core_wrapper::get_SFS_openf_wrapper()
{
  sfs::init();
  return (void*) &wrap_SFS_openf;
}
