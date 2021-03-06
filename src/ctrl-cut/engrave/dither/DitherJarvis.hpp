/*
 * Ctrl-Cut - Laser cutter toolchain
 * See LICENSE file
 * Copyright (C) 2011 Amir Hassan <amir@viel-zu.org> and Marius kintel <kintel@kintel.net>
 */

#ifndef DITHERJARVIS_H_
#define DITHERJARVIS_H_

#include "Dither.hpp"

class DitherJarvis: public Dither {
public:
  DitherJarvis(GrayscaleImage& img): Dither(img) {}
  virtual ~DitherJarvis(){}
  void ditherPixel(const uint32_t x, const uint32_t y, Pixel<uint8_t>& pix, const uint32_t colors = 1);

};

#endif /* DITHERJARVIS_H_ */
