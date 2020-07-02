/**
 *    Copyright (C) 2015 Stainless <http://stainlessbeer.weebly.com>
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

#ifndef IMF_H
#define IMF_H

#include <file.h>

#include <vector>
#include <string>

void getIMFInfo(util::File &file, int &w, int &h);
void loadIMF(const std::vector<char> &src, std::vector<unsigned char> &data,
             int &width, int &height, const std::string &name);

#endif
