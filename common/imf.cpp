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

#include "imf.h"

#include <sstream>
#include <vector>
#include <cmath>
#include <cassert>
#include <iostream>

using std::cout;
using std::endl;

namespace
{

  typedef unsigned char byte;

  int paethPredictor(int a, int b, int c)
  {
      a = a & 255;
      b = b & 255;
      c = c & 255;
      int p = (a + b - c);
      int pa = abs(p - a);
      int pb = abs(p - b);
      int pc = abs(p - c);

      if ((pa <= pb) && (pa <= pc))
          return a;

      if (pb <= pc)
          return b;

      return c;
  }

  int Average(int a, int b)
  {
    return (a+b)/2;
  }

}

void loadIMF(const std::vector<char> &src, std::vector<unsigned char> &out,
             int &width, int &height, const std::string &name)
{
  std::istringstream br(std::string(src.data(), src.size()));
  br.seekg(7);

  int flag = br.get();
  (void)flag;

  width = br.get() | (br.get() << 8);
  height = br.get() | (br.get() << 8);

  int imageSize = width * height * 4;

  out.resize(imageSize);

  std::vector<byte> data(imageSize);

  std::vector<byte> mode(height);
  br.read((char*) mode.data(), mode.size());

  std::vector<byte> alpha(width * height);

  int pos = 0;
  int pos2 = 0;
  int prev = width * 4;
  byte r = 0;
  byte g = 0;
  byte b = 0;
  byte a = 0;

  for (int y = 0; y < height; y++)
  {
      if (mode[y] < 4)
      {
          r = g = b = 0;
      }

      for (int x = 0; x < width; x++)
      {
          switch (mode[y])
          {
              // none
              case 0:
                  r = br.get();
                  g = br.get();
                  b = br.get();
                  a = 255;
                  break;

              // sub
              case 1:
                  r += br.get();
                  g += br.get();
                  b += br.get();
                  a = 255;
                  break;

              // up
              case 2:
                  b = data[pos - prev];
                  g = data[pos - prev + 1];
                  r = data[pos - prev + 2];

                  r += br.get();
                  g += br.get();
                  b += br.get();
                  a = 255;
                  break;

              // average
              case 3:
                  r = Average(r, data[pos - prev + 2]);
                  g = Average(g, data[pos - prev + 1]);
                  b = Average(b, data[pos - prev]);

                  r += br.get();
                  g += br.get();
                  b += br.get();
                  a = 255;
                  break;

              // paeth
              case 4:
                  if (x > 0)
                  {
                      r = (byte)paethPredictor(r, data[pos - prev + 2], data[pos - prev - 2]);
                      g = (byte)paethPredictor(g, data[pos - prev + 1], data[pos - prev - 3]);
                      b = (byte)paethPredictor(b, data[pos - prev], data[pos - prev - 4]);
                  }
                  else
                  {
                      r = (byte)paethPredictor(data[pos - prev + 2], data[pos - prev + 2], 0);//data[pos - prev - 2]);
                      g = (byte)paethPredictor(data[pos - prev + 1], data[pos - prev + 1], 0);//data[pos - prev - 3]);
                      b = (byte)paethPredictor(data[pos - prev], data[pos - prev], 0);//data[pos - prev - 4]);
                  }

                  r += br.get();
                  g += br.get();
                  b += br.get();
                  a = 255;
                  break;
              default:
                cout<<"IMF: unknown mode: "<<mode[y]<<" - "<<name<<endl;

          }

          out[pos] = r;
          out[pos+1] = g;
          out[pos+2] = b;
          out[pos+3] = a;

          data[pos++] = b;
          data[pos++] = g;
          data[pos++] = r;
          data[pos++] = a;
      }
  }

  if (flag > 0)
  {
      pos = 0;
      br.read((char*) mode.data(), mode.size());
      size_t pos_out = 3;

      for (int y = 0; y < height; y++)
      {
          if (mode[y] < 4)
          {
              r = 0;
          }
          for (int x = 0; x < width; x++)
          {
              switch (mode[y])
              {
                  case 0:
                      r = br.get();
                      break;
                  case 1:
                      r += br.get();
                      break;
                  case 2:
                      r = alpha[pos - width];
                      r += br.get();
                      break;
                  case 3:
                      if (y == 0)
                      {
                          r = Average(r, 0);
                      }
                      else
                      {
                          r = Average(r, alpha[pos - width]);
                      }
                      r += br.get();
                      break;
                  case 4:
                      {
                          if (x > 0)
                          {
                              r = (byte)paethPredictor(r, alpha[pos - width], alpha[pos - width - 1]);
                              r += br.get();
                          }
                          else
                          {
                              if (y > 1)
                                  r = (byte)paethPredictor(alpha[pos - width], alpha[pos - width], alpha[pos - width - 1]);
                              else
                                  r = (byte)paethPredictor(alpha[pos - width], alpha[pos - width], 0);

                              r += br.get();
                          }
                      }
                      break;

              }

              alpha[pos++] = r;
              assert(pos_out < out.size());
              out[pos_out] = r;
              pos_out+= 4;
          }
      }

  }

}
