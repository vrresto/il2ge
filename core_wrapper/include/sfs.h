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

#ifndef IL2GE_SFS_H
#define IL2GE_SFS_H

#include <file.h>

#include <vector>
#include <cstdint>
#include <string>

namespace sfs
{
  class File : public util::File
  {
    int m_fd = -1;
    long m_pos = 0;
    long m_size = 0;
    std::string m_path;

  public:
    File(std::string path);
    ~File() override;

    int read(char *out, int bytes) override;
    void skip(int bytes) override;
    void rewind() override;
    bool eof() override;
    void readAll(std::vector<char>&) override;
    int getSize() override;
  };

  void init();
  bool readFile(const std::string &filename, std::vector<char> &out);
  __int64 getHash(const char *filename);
  void redirect(__int64 hash, __int64 hash_redirection);
  void clearRedirections();
  void *get_openf_wrapper();
}

#endif
