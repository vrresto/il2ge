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

#ifndef SFS_H
#define SFS_H

#include <vector>
#include <cstdint>
#include <string>

namespace SFS
{
  void init();

  int open(const char *filename, int flags);
  int openf(uint64_t hash, int flags);
  int close(int fd);
  int read(int fd, void *buf, int nbyte);
  unsigned int lseek(int fd, unsigned int offset, int whence);
  bool readFile(const std::string &filename, std::vector<char> &out);
  __int64 getHash(const char *filename);
  void redirect(__int64 hash, __int64 hash_redirection);
  void clearRedirections();
}

#endif
