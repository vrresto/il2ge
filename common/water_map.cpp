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

#include <il2ge/water_map.h>
#include <render_util/image_util.h>


#include <glm/glm.hpp>
#include <iostream>

using namespace glm;
using namespace std;

namespace
{


using namespace il2ge;

enum
{
  ORIG_CHUNK_SIZE = 32,
  SUPER_CHUNK_SIZE = 4,
  CHUNK_SIZE = ORIG_CHUNK_SIZE * SUPER_CHUNK_SIZE
};

enum ChunkType
{
  CHUNK_EMPTY,
  CHUNK_FULL,
  CHUNK_MIXED
};


render_util::ImageGreyScale::Ptr createEmptyImage(int size)
{
  return render_util::image::create<unsigned char>(0, ivec2(size));
}


render_util::ImageGreyScale::Ptr createFullImage(int size)
{
  return render_util::image::create<unsigned char>(255, ivec2(size));
}


ChunkType classifyChunk(render_util::ImageGreyScale::ConstPtr image)
{
  int num_empty = 0;
  int num_full = 0;
  int num_other = 0;

  for (int y = 0; y < image->w(); y++)
  {
    for (int x = 0; x < image->h(); x++)
    {
      unsigned int color = image->get(x,y);

      if (color == 0)
        num_empty++;
      else if (color == 255)
        num_full++;
      else
        num_other++;
    }
  }

  if (num_other)
    return CHUNK_MIXED;
  else
  {
    if (num_full && !num_empty)
      return CHUNK_FULL;
    else if (num_empty && !num_full)
      return CHUNK_EMPTY;
    else
      return CHUNK_MIXED;
  }
}


class Chunk
{
  render_util::ImageGreyScale::ConstPtr m_image;
  ChunkType m_type = CHUNK_MIXED;

public:
  bool isEmpty() const
  {
    return m_type == CHUNK_EMPTY;
  }

  bool isFull() const
  {
    return m_type == CHUNK_FULL;
  }

  void setEmpty()
  {
    m_type = CHUNK_EMPTY;
    m_image = nullptr;
  }

  void setFull()
  {
    m_type = CHUNK_FULL;
    m_image = nullptr;
  }

  void setType(ChunkType type)
  {
    m_type = type;
    if (type != CHUNK_MIXED)
      m_image = nullptr;
  }

  render_util::ImageGreyScale::ConstPtr getImage()
  {
    assert(!isEmpty());
    assert(!isFull());
    assert(m_image);

    return m_image;
  }

  void setImage(render_util::ImageGreyScale::ConstPtr image)
  {
    assert(image);
    m_type = CHUNK_MIXED;
    m_image = image;
  }
};


class SuperChunk
{
  render_util::Array2D<Chunk*> sub_chunks;
  int num_full_chunks = 0;
  int num_chunks = 0;

  enum
  {
    CROP_LEFT = 1 << 0,
    CROP_RIGHT = 1 << 1,
    CROP_TOP = 1 << 2,
    CROP_BOTTOM = 1 << 3
  };

public:
  SuperChunk() : sub_chunks(SUPER_CHUNK_SIZE, SUPER_CHUNK_SIZE) {}

  int w() { return sub_chunks.w(); }
  int h() { return sub_chunks.h(); }

  void setSubChunk(int x, int y, Chunk *chunk)
  {
    assert(!chunk->isEmpty());
    assert(!sub_chunks.at(x, y));

    sub_chunks.at(x, y) = chunk;
    num_chunks++;
    if (chunk->isFull())
      num_full_chunks++;

    assert(!isEmpty());
  }

  bool isFull() { return sub_chunks.size() == num_full_chunks; }
  bool isEmpty() { return !num_chunks; }

  render_util::ImageGreyScale::Ptr createImage()
  {
    assert(!isFull() && !isEmpty());

    render_util::ImageGreyScale::Ptr image = createEmptyImage(CHUNK_SIZE);

    for (int y = 0; y < h(); y++)
    {
      for (int x = 0; x < w(); x++)
      {
        Chunk *chunk = sub_chunks.at(x, y);

        unsigned int crop_border = 0;
#if 1
//           if (y > 0)
//           {
//             crop_border |= CROP_TOP;
//             cout<<"CROP_TOP: "<<hex<<CROP_TOP<<endl;
//             cout<<"crop_border: "<<crop_border<<endl;
// //             exit(0);
//           }
//           if (x > 0)
//             crop_border |= CROP_LEFT;
        if (y < h()-1)
          crop_border |= CROP_BOTTOM;
        if (x < w()-1)
          crop_border |= CROP_RIGHT;
#endif
        crop_border = CROP_LEFT|CROP_TOP;

        ivec2 pos = (ivec2(x, y) * ivec2(ORIG_CHUNK_SIZE - 1)) + ivec2(0);
//           cout<<pos.x<<endl;

//           if ((crop_border & CROP_TOP))
//             pos.y++;
//           if ((crop_border & CROP_LEFT))
//             pos.x++;

        if (chunk)
        {
          assert(!chunk->isEmpty());

          render_util::ImageGreyScale::ConstPtr src;

          if (chunk->isFull())
          {
            src = createFullImage(ORIG_CHUNK_SIZE);
//               static auto src = createFullImage(ORIG_CHUNK_SIZE);
//               render_util::image::blit<render_util::ImageGreyScale>(src.get(), image.get(), pos);
          }
          else
          {
            src = chunk->getImage();
//               assert(chunk->getImage().get());
//               static auto src = createEmptyImage(ORIG_CHUNK_SIZE);
//               auto src = chunk->getImage();
//               render_util::image::blit<render_util::ImageGreyScale>(chunk->getImage().get(), image.get(), ivec2(0));
//               render_util::image::blit<render_util::ImageGreyScale>(src.get(), image.get(), pos);
          }

          if (crop_border)
          {
            int x = 0;
            int y = 0;
            int w = ORIG_CHUNK_SIZE;
            int h = ORIG_CHUNK_SIZE;
#if 0
            if (crop_border & CROP_LEFT)
            {
//                 x++;
//                 w--;
            }
            if (crop_border & CROP_RIGHT)
            {
              w--;
            }
            if (crop_border & CROP_TOP)
            {
//                 y++;
//                 h--;
            }
            if (crop_border & CROP_BOTTOM)
            {
              h--;
            }
#endif
            src = render_util::image::subImage(src.get(), x, y, w, h);
          }
          else
          {
            cout<<"crop_border: "<<hex<<crop_border<<endl;
          }

          render_util::image::blit<render_util::ImageGreyScale>(src.get(), image.get(), pos);
        }
        else
        {
//            //FIXME
//             assert(0);
        }
      }
    }

    return image;
  }
};


class Map
{
  render_util::Array2D<Chunk> chunks;

public:
  Map(int w, int h) : chunks(w, h) {}

  Map(const WaterMap &src) : chunks(src.table->w(), src.table->h())
  {
    for (int y = 0; y < src.table->h(); y++)
    {
      for (int x = 0; x < src.table->w(); x++)
      {
        unsigned int index = src.table->get(x,y);
        assert(index < src.chunks.size());

        Chunk &dst_chunk = chunks.at(x, y);
        render_util::ImageGreyScale::ConstPtr image = src.chunks[index];

        dst_chunk.setImage(image);
        dst_chunk.setType(classifyChunk(image));
      }
    }
  }

  Chunk *getChunk(ivec2 pos)
  {
    if (pos.x < chunks.w() && pos.y < chunks.h())
    {
      return &chunks.at(pos.x, pos.y);
    }
    else
      return nullptr;

  }

  int w() { return chunks.w(); }
  int h() { return chunks.h(); }
};


void fillSuperChunk(Map &src, ivec2 pos, SuperChunk &chunk)
{
  ivec2 src_coord(pos * ivec2(SUPER_CHUNK_SIZE));

  for (int y = 0; y < chunk.w(); y++)
  {
    for (int x = 0; x < chunk.h(); x++)
    {
      Chunk *sub_chunk = src.getChunk(src_coord + ivec2(x, y));
      if (sub_chunk && !sub_chunk->isEmpty())
      {
        chunk.setSubChunk(x, y, sub_chunk);
      }
    }
  }
}


void fillWaterMap(Map &src, WaterMap &dst)
{
  dst.chunks.push_back(createEmptyImage(CHUNK_SIZE));
  dst.chunks.push_back(createFullImage(CHUNK_SIZE));

  render_util::Image<unsigned int>::Ptr table;
  table.reset(new render_util::Image<unsigned int>(ivec2(src.w(), src.h())));

  for (int y = 0; y < src.h(); y++)
  {
    for (int x = 0; x < src.w(); x++)
    {
      Chunk *chunk = src.getChunk(ivec2(x, y));
      unsigned int index = 0;

      if (chunk->isFull())
      {
        index = 1;
      }
      else if (!chunk->isEmpty())
      {
        dst.chunks.push_back(chunk->getImage());
        index = dst.chunks.size() - 1;
      }

      table->at(x,y) = index;
    }
  }

  dst.table = table;
}


} // namespace


namespace il2ge
{


void convertWaterMap(const WaterMap &src, WaterMap &dst)
{
  Map src_map(src);

  Map dst_map(ceil((float)src_map.w() / SUPER_CHUNK_SIZE), ceil((float)src_map.h() / SUPER_CHUNK_SIZE));

  for (int y = 0; y < dst_map.h(); y++)
  {
    for (int x = 0; x < dst_map.w(); x++)
    {
      SuperChunk super_chunk;
      fillSuperChunk(src_map, ivec2(x,y), super_chunk);

      Chunk *dst_chunk = dst_map.getChunk(ivec2(x, y));

      if (super_chunk.isFull())
      {
        dst_chunk->setFull();
      }
      else if (super_chunk.isEmpty())
      {
        dst_chunk->setEmpty();
      }
      else
      {
        dst_chunk->setImage(super_chunk.createImage());
      }
    }
  }

  fillWaterMap(dst_map, dst);
}


} // namespace il2ge
