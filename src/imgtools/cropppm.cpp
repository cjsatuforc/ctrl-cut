/*
 * Ctrl-Cut - Laser cutter toolchain
 * See LICENSE file
 * Copyright (C) 2011 Amir Hassan <amir@viel-zu.org> and Marius kintel <kintel@kintel.net>
 */

#include <stdint.h>
#include <iostream>
#include <fstream>
#include "MMapMatrix.hpp"
#include "PPMFile.hpp"

using std::ofstream;
using std::cout;
using std::cerr;
using std::endl;

cropppm::cropppm() {}
cropppm::~cropppm() {}

int main(int argc, char *argv[]) {
  if(argc != 4) {
    cerr << "Usage: cropppm <geometry> <ppmin> <ppmout>" << endl;
    return 1;
  }
  string geom(argv[1]);
  string fin(argv[2]);
  ofstream out(argv[3]);

  BoundingBox* crop = BoundingBox::createFromGeometryString(geom);
  Image *img = loadppm(fin);
  dim oWidth = crop->lr.x - crop->ul.x;
  dim oHeight = crop->lr.y - crop->ul.y;
  out << "P6" << endl << oWidth << " " << oHeight << endl << "255" << endl;

  Pixel<uint8_t> pix;
  for (coord y = crop->ul.y; y < crop->lr.y; ++y) {
    for (coord x = crop->ul.x; x < crop->lr.x; ++x) {
      img->readPixel(x, y, pix);
      out << pix.r << pix.g << pix.b;
    }
  }

  return 0;
}
