/*
 * Ctrl-Cut - A laser cutter CUPS driver
 * Copyright (C) 2009-2010 Amir Hassan <amir@viel-zu.org> and Marius Kintel <marius@kintel.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MMAPMATRIX_H_
#define MMAPMATRIX_H_

#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <fstream>
#include <string>
#include <iostream>
#include "util/Logger.h"
#include "util/2D.h"
#include "stdint.h"

using namespace boost::interprocess;

static const float bayer_matrix[4][4] = {
    {1,9 ,3,11},
    {13,5,15,7},
    {4,12,2,10},
    {16,8,14,6}
};

template<class T>
class MMapMatrix {
public:
	string filename;
	file_mapping* m_file;
	mapped_region m_region;
	void * addr;
	std::size_t size;
	size_t bytes_per_pixel;
	size_t x;
	size_t y;
	size_t w;
	size_t h;

	MMapMatrix(string filename, uint16_t width, uint16_t height,
			uint16_t x, uint16_t y, uint64_t region_off) {
	  LOG_DEBUG(region_off);
		this->filename = filename;
		this->m_file = new file_mapping(filename.c_str(), read_write);
		this->bytes_per_pixel = sizeof(T) * 3;
    LOG_DEBUG(bytes_per_pixel);
		this->m_region = mapped_region(*this->m_file, read_write, region_off + (x
				* y * bytes_per_pixel), width * height * bytes_per_pixel);
		this->addr = m_region.get_address();
		this->size = m_region.get_size();
    LOG_DEBUG(this->size);
		this->x = x;
		this->y = y;
		this->w = width;
		this->h = height;
	}

	MMapMatrix(file_mapping* m_file, uint16_t width, uint16_t height,
			uint16_t x, uint16_t y) {
		this->filename = filename;
		this->m_file = m_file;
		this->bytes_per_pixel = sizeof(T) * 3;
		this->m_region = mapped_region(*this->m_file, read_write, x * y
				* bytes_per_pixel, width * height * bytes_per_pixel);
		this->addr = m_region.get_address();
		this->size = m_region.get_size();
		this->x = x;
		this->y = y;
		this->w = width;
		this->h = height;
	}

	size_t width() {
		return this->w;
	}

	size_t height() {
		return this->h;
	}

	size_t offsetX() {
		return this->x;
	}

	size_t offsetY() {
		return this->y;
	}

	void readPixel(const uint32_t x, const uint32_t y, Pixel<T>& pix) const {
	  T* sample = (static_cast<T*> (addr)) + ((y * w + x) * 3);
	  pix.setRGB(sample);
	}

  void writePixel(uint32_t x, uint32_t y, const Pixel<T>& pix) {
    T* sample = (static_cast<T*> (addr)) + ((y * w + x) * 3);
    *sample = pix.i;
    *(sample + 1) = pix.i;
    *(sample + 2) = pix.i;
  }

  //FIXME use T!
  void averageXSequence(const uint32_t fromX, const uint32_t toX, const uint32_t y, Pixel<uint8_t>& p){
    float cumm = 0;

    for (uint32_t x = fromX; x < toX; x++){
      this->readPixel(x, y, p);
      cumm += p.i;
    }
    p.i = cumm / (toX - fromX);
  }

  //FIXME use T!
  void writeXSequence(uint32_t fromX, uint32_t toX, uint32_t y, const Pixel<uint8_t>& p){
    for (uint32_t x = fromX; x < toX; x++){
      this->writePixel(x, y, p);
    }
  }

  void carryOver(const uint32_t fromX, const uint32_t toX, const uint32_t atY, const int8_t carryover, Pixel<uint8_t>& p) {
    if (toX < w && fromX < w && atY < h && atY < h) {
      averageXSequence(fromX, toX, atY, p);
      p.i = add(p.i, carryover);
      writeXSequence(fromX, toX, atY, p);
    }
  }

  void ditherNull(const uint32_t x, const uint32_t y, Pixel<T>& newpixel, const uint32_t xrange = 8, const uint32_t colors = 16) {
    averageXSequence(x, x + xrange, y, newpixel);
    writeXSequence(x, x + xrange, y, newpixel);
  }

  //FIXME use T!
  void ditherBayer(const uint32_t x, const uint32_t y, Pixel<T>& newpixel, const uint32_t xrange = 8, const uint32_t colors = 32) {
    Pixel<T> oldpix;

    averageXSequence(x, x + xrange, y, oldpix);
    oldpix.i = add(oldpix.i,bayer_matrix[(x/8) % 4][y % 4]);
    newpixel.i = reduce(oldpix.i, colors);
    writeXSequence(x, x + xrange, y, newpixel);
  }

  void ditherFloydSteinberg(const uint32_t x, const uint32_t y, Pixel<T>& newpixel, const uint32_t xrange = 8, const uint32_t colors = 32) {
    Pixel<T> oldpix;
    uint32_t fromX = x;
    uint32_t toX = x + xrange;
    uint32_t atY = y;

    averageXSequence(fromX, toX, atY, oldpix);
    newpixel.i = reduce(oldpix.i, colors);
    writeXSequence(fromX, toX, atY, newpixel);
    const float quant_error = oldpix.i - newpixel.i;

    if(quant_error != 0) {
      int8_t seven = (7 * quant_error) / 16;
      int8_t five = (5 * quant_error)  / 16;
      int8_t three = (3 * quant_error) / 16;
      int8_t one = (1 * quant_error)   / 16;

      fromX = x +  xrange;
      toX = fromX + xrange;

      carryOver(fromX, toX, atY, seven, oldpix);

      fromX = x -  xrange;
      toX = fromX + xrange;
      atY = y - 1;

      carryOver(fromX, toX, atY, three, oldpix);

      fromX = x;
      toX = fromX + xrange;
      atY = y - 1;

      carryOver(fromX, toX, atY, five, oldpix);

      fromX = x + xrange;
      toX = fromX + xrange;
      atY = y - 1;

      carryOver(fromX, toX, atY, one, oldpix);
    }
  }


  void ditherJJN(const uint32_t x, const uint32_t y, Pixel<T>& newpixel, const uint32_t xrange = 8, const uint32_t colors = 32) {
    Pixel<T> oldpix;
    uint32_t fromX = x;
    uint32_t toX = x + xrange;
    uint32_t atY = y;

    averageXSequence(fromX, toX, atY, oldpix);
    newpixel.i = reduce(oldpix.i, colors);
    writeXSequence(fromX, toX, atY, newpixel);
    const float quant_error = oldpix.i - newpixel.i;

    if(quant_error > 0) {
      int8_t one = (1 * quant_error) / 48;
      int8_t three = (3 * quant_error) / 48;
      int8_t five = (5 * quant_error) / 48;
      int8_t seven = (7 * quant_error) / 48;

      fromX = x + xrange;
      toX = fromX + xrange;

      carryOver(fromX, toX, atY, seven, oldpix);

      fromX = x + (xrange * 2);
      toX = fromX + xrange;

      carryOver(fromX, toX, atY, five, oldpix);

      fromX = x - (xrange * 2);
      toX = fromX + xrange;
      atY = y - 1;

      carryOver(fromX, toX, atY, three, oldpix);

      fromX = x -  xrange;
      toX = fromX + xrange;
      atY = y - 1;

      carryOver(fromX, toX, atY, five, oldpix);

      fromX = x;
      toX = fromX + xrange;
      atY = y - 1;

      carryOver(fromX, toX, atY, seven, oldpix);

      fromX = x + xrange;
      toX = fromX + xrange;
      atY = y - 1;

      carryOver(fromX, toX, atY, five, oldpix);

      fromX = x + (xrange * 2);
      toX = fromX + xrange;
      atY = y - 1;

      carryOver(fromX, toX, atY, three, oldpix);

      fromX = x - (xrange * 2);
      toX = fromX + xrange;
      atY = y - 2;

      carryOver(fromX, toX, atY, one, oldpix);

      fromX = x -  xrange;
      toX = fromX + xrange;
      atY = y - 2;

      carryOver(fromX, toX, atY, three, oldpix);

      fromX = x;
      toX = fromX + xrange;
      atY = y - 2;

      carryOver(fromX, toX, atY, five, oldpix);

      fromX = x + xrange;
      toX = fromX + xrange;
      atY = y - 2;

      carryOver(fromX, toX, atY, three, oldpix);

      fromX = x + (xrange * 2);
      toX = fromX + xrange;
      atY = y - 2;

      carryOver(fromX, toX, atY, one, oldpix);
    }
  }

  void ditherStucki(const uint32_t x, const uint32_t y, Pixel<T>& newpixel, const uint32_t xrange = 8, const uint32_t colors = 16) {
     Pixel<T> oldpix;
     uint32_t fromX = x;
     uint32_t toX = x + xrange;
     uint32_t atY = y;

     averageXSequence(fromX, toX, atY, oldpix);
     newpixel.i = reduce(oldpix.i, colors);
     writeXSequence(fromX, toX, atY, newpixel);
     const float quant_error = oldpix.i - newpixel.i;

     if(quant_error > 0) {
       int8_t eight = (8 * quant_error) / 42;
       int8_t four = (4 * quant_error) / 42;
       int8_t two = (2 * quant_error) / 42;
       int8_t one = (1 * quant_error) / 42;

       fromX = x + xrange;
       toX = fromX + xrange;

       carryOver(fromX, toX, atY, eight, oldpix);

       fromX = x + (xrange * 2);
       toX = fromX + xrange;

       carryOver(fromX, toX, atY, four, oldpix);

       fromX = x - (xrange * 2);
       toX = fromX + xrange;
       atY = y - 1;

       carryOver(fromX, toX, atY, two, oldpix);

       fromX = x -  xrange;
       toX = fromX + xrange;
       atY = y - 1;

       carryOver(fromX, toX, atY, four, oldpix);

       fromX = x;
       toX = fromX + xrange;
       atY = y - 1;

       carryOver(fromX, toX, atY, eight, oldpix);

       fromX = x + xrange;
       toX = fromX + xrange;
       atY = y - 1;

       carryOver(fromX, toX, atY, four, oldpix);

       fromX = x + (xrange * 2);
       toX = fromX + xrange;
       atY = y - 1;

       carryOver(fromX, toX, atY, two, oldpix);

       fromX = x - (xrange * 2);
       toX = fromX + xrange;
       atY = y - 2;

       carryOver(fromX, toX, atY, one, oldpix);

       fromX = x -  xrange;
       toX = fromX + xrange;
       atY = y - 2;

       carryOver(fromX, toX, atY, two, oldpix);

       fromX = x;
       toX = fromX + xrange;
       atY = y - 2;

       carryOver(fromX, toX, atY, four, oldpix);

       fromX = x + xrange;
       toX = fromX + xrange;
       atY = y - 2;

       carryOver(fromX, toX, atY, two, oldpix);

       fromX = x + (xrange * 2);
       toX = fromX + xrange;
       atY = y - 2;

       carryOver(fromX, toX, atY, one, oldpix);
     }
   }

  void ditherBurke(const uint32_t x, const uint32_t y, Pixel<T>& newpixel, const uint32_t xrange = 8, const uint32_t colors = 16) {
     Pixel<T> oldpix;
     uint32_t fromX = x;
     uint32_t toX = x + xrange;
     uint32_t atY = y;

     averageXSequence(x, toX, y, oldpix);
     newpixel.i = (((uint8_t) (((float)colors) * (oldpix.i / 255.0f))) / ((float)colors)) * 255;
     writeXSequence(x, toX, y, newpixel);
     const float quant_error = oldpix.i - newpixel.i;

     if(quant_error > 0) {


       int8_t eight = (8 * quant_error) / 32;
       int8_t four = eight >> 1;
       int8_t two = four >> 1;

       fromX = x + xrange;
       toX = fromX + xrange;

       if (toX < w) {
         averageXSequence(x + xrange, toX + xrange, atY, oldpix);
         oldpix.i = add(oldpix.i, eight);
         writeXSequence(x + xrange, toX + xrange, atY, oldpix);
       }

       fromX = x + (xrange * 2);
       toX = fromX + xrange;

       if (toX < w) {
         averageXSequence(x + xrange, toX + xrange, atY, oldpix);
         oldpix.i = add(oldpix.i, four);
         writeXSequence(x + xrange, toX + xrange, atY, oldpix);
       }

       fromX = x - (xrange * 2);
       toX = fromX + xrange;
       atY = y - 1;

       if (x >= (xrange * 2) && y >= 1) {
         averageXSequence(fromX, toX, atY, oldpix);
         oldpix.i = add(oldpix.i, two);
         writeXSequence(fromX, toX, atY, oldpix);
       }

       fromX = x -  xrange;
       toX = fromX + xrange;
       atY = y - 1;

       if (x >= xrange && y >= 1) {
         averageXSequence(fromX, toX, atY, oldpix);
         oldpix.i = add(oldpix.i, four);
         writeXSequence(fromX, toX, atY, oldpix);
       }

       fromX = x;
       toX = fromX + xrange;
       atY = y - 1;

       if (toX < w && y >= 1) {
         averageXSequence(x, toX, atY, oldpix);
         oldpix.i = add(oldpix.i, eight);
         writeXSequence(x, toX, atY, oldpix);
       }

       fromX = x + xrange;
       toX = fromX + xrange;
       atY = y - 1;

       if (toX < w && y >= 1) {
         averageXSequence(x + xrange, toX + xrange, atY, oldpix);
         oldpix.i = add(oldpix.i, four);
         writeXSequence(x + xrange, toX + xrange, atY, oldpix);
       }

       fromX = x + (xrange * 2);
       toX = fromX + xrange;
       atY = y - 1;

       if (toX < w && y >= 1) {
         averageXSequence(x + xrange, toX + xrange, atY, oldpix);
         oldpix.i = add(oldpix.i, two);
         writeXSequence(x + xrange, toX + xrange, atY, oldpix);
       }
     }
   }

  void ditherSierra3(const uint32_t x, const uint32_t y, Pixel<T>& newpixel, const uint32_t xrange = 8, const uint32_t colors = 32) {
     Pixel<T> oldpix;
     uint32_t fromX = x;
     uint32_t toX = x + xrange;
     uint32_t atY = y;

     averageXSequence(fromX, toX, atY, oldpix);
     newpixel.i = reduce(oldpix.i, colors);
     writeXSequence(fromX, toX, atY, newpixel);
     const float quant_error = oldpix.i - newpixel.i;

     if(quant_error > 0) {

       int8_t five = (5 * quant_error) / 32;
       int8_t four = (4 * quant_error) / 32;
       int8_t three = (3 * quant_error) / 32;
       int8_t two = (2 * quant_error) / 32;

       fromX = x + xrange;
       toX = fromX + xrange;

       carryOver(fromX, toX, atY, five, oldpix);

       fromX = x + (xrange * 2);
       toX = fromX + xrange;

       carryOver(fromX, toX, atY, three, oldpix);

       fromX = x - (xrange * 2);
       toX = fromX + xrange;
       atY = y - 1;

       carryOver(fromX, toX, atY, two, oldpix);

       fromX = x -  xrange;
       toX = fromX + xrange;
       atY = y - 1;

       carryOver(fromX, toX, atY, four, oldpix);

       fromX = x;
       toX = fromX + xrange;
       atY = y - 1;

       carryOver(fromX, toX, atY, five, oldpix);

       fromX = x + xrange;
       toX = fromX + xrange;
       atY = y - 1;

       carryOver(fromX, toX, atY, four, oldpix);

       fromX = x + (xrange * 2);
       toX = fromX + xrange;
       atY = y - 1;

       carryOver(fromX, toX, atY, two, oldpix);

       fromX = x -  xrange;
       toX = fromX + xrange;
       atY = y - 2;

       carryOver(fromX, toX, atY, two, oldpix);

       fromX = x;
       toX = fromX + xrange;
       atY = y - 2;

       carryOver(fromX, toX, atY, three, oldpix);

       fromX = x + xrange;
       toX = fromX + xrange;
       atY = y - 2;

       carryOver(fromX, toX, atY, two, oldpix);
     }
   }

  void ditherSierra2(const uint32_t x, const uint32_t y, Pixel<T>& newpixel, const uint32_t xrange = 8, const uint32_t colors = 16) {
     Pixel<T> oldpix;
     uint32_t fromX = x;
     uint32_t toX = x + xrange;
     uint32_t atY = y;

     averageXSequence(x, toX, y, oldpix);
     newpixel.i = (((uint8_t) (((float)colors) * (oldpix.i / 255.0f))) / ((float)colors)) * 255;
     uint8_t quant_error = oldpix.i - newpixel.i;
     if(quant_error > 0) {
       writeXSequence(x, toX, y, newpixel);

       uint8_t four = (4 * quant_error) >> 4;
       uint8_t three = (3 * quant_error) >> 4;
       uint8_t two = four >> 1;
       uint8_t one = two >> 1;

       fromX = x + xrange;
       toX = fromX + xrange;

       if (toX < w) {
         averageXSequence(x + xrange, toX + xrange, atY, oldpix);
         oldpix.i = add(oldpix.i, four);
         writeXSequence(x + xrange, toX + xrange, atY, oldpix);
       }

       fromX = x + (xrange * 2);
       toX = fromX + xrange;

       if (toX < w) {
         averageXSequence(x + xrange, toX + xrange, atY, oldpix);
         oldpix.i = add(oldpix.i, three);
         writeXSequence(x + xrange, toX + xrange, atY, oldpix);
       }

       fromX = x - (xrange * 2);
       toX = fromX + xrange;
       atY = y - 1;

       if (x >= (xrange * 2) && y >= 1) {
         averageXSequence(fromX, toX, atY, oldpix);
         oldpix.i = add(oldpix.i, one);
         writeXSequence(fromX, toX, atY, oldpix);
       }

       fromX = x -  xrange;
       toX = fromX + xrange;
       atY = y - 1;

       if (x >= xrange && y >= 1) {
         averageXSequence(fromX, toX, atY, oldpix);
         oldpix.i = add(oldpix.i, two);
         writeXSequence(fromX, toX, atY, oldpix);
       }

       fromX = x;
       toX = fromX + xrange;
       atY = y - 1;

       if (toX < w && y >= 1) {
         averageXSequence(x, toX, atY, oldpix);
         oldpix.i = add(oldpix.i, three);
         writeXSequence(x, toX, atY, oldpix);
       }

       fromX = x + xrange;
       toX = fromX + xrange;
       atY = y - 1;

       if (toX < w && y >= 1) {
         averageXSequence(x + xrange, toX + xrange, atY, oldpix);
         oldpix.i = add(oldpix.i, two);
         writeXSequence(x + xrange, toX + xrange, atY, oldpix);
       }

       fromX = x + (xrange * 2);
       toX = fromX + xrange;
       atY = y - 1;

       if (toX < w && y >= 1) {
         averageXSequence(x + xrange, toX + xrange, atY, oldpix);
         oldpix.i = add(oldpix.i, one);
         writeXSequence(x + xrange, toX + xrange, atY, oldpix);
       }
     }
   }

  T reduce(const T intensity, const T colors) {
    return round((float)((float)round( colors * (float)(intensity / 255.0f) ) / colors) * 255);
  }

  //FIXME how to guarantee T is unsigned?
  uint8_t add(const uint8_t intensity, const int8_t carry) {
	  uint8_t sum = intensity + carry;

	  //overflow?
	  if(carry > 0 && sum < intensity)
	    sum = 255;

    if(carry < 0 && sum > intensity)
      sum = 0;

	  return sum;
	}

  MMapMatrix<T>* tile(offset_t x, offset_t y, size_t width, size_t height) {
		return new MMapMatrix<T> (this->m_file, this->filename, width, height, x, y);
	}
};


typedef MMapMatrix<uint8_t> Image;
#endif /* MMAPMATRIX_H_ */

