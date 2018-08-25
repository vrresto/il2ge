/**
 *    Copyright (C) 2013 SAS~Storebror <mike@sas1946.com>
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

#ifndef IL2GE_SFS_P_H
#define IL2GE_SFS_P_H

#include <windef.h>

namespace sfs_private
{
  unsigned __int64 makeHash(const unsigned __int64 hash, const void *buf, const int len);
}

#endif
