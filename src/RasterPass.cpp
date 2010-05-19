#include <string>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <list>
#include "boost/format.hpp"

#include "RasterPass.h"
#include "pjl.h"
#include "LaserJob.h"

using namespace boost;
using namespace std;

int tile_cnt = 0;

//TODO rewrite class to be really OO

void RasterPass::addTile(Tile* tile) {
  this->tiles.push_back(tile);
}

void RasterPass::serializeTo(ostream& out) {
  list<Tile*>::iterator it;
  // Raster Orientation
  out << format(R_ORIENTATION) % 0;
  // Raster power
  out << format(R_POWER) % ((lconf->raster_mode == 'c' || lconf->raster_mode
                             == 'g') ? 100 : lconf->raster_power);

  out << PCL_UNKNOWN_BLAFOO3;
  // Raster speed
  out << format(R_SPEED) % lconf->raster_speed;
  out << format(R_HEIGHT) % (lconf->height * lconf->y_repeat);
  out << format(R_WIDTH) % (lconf->width * lconf->x_repeat);
  // Raster compression
  int compressionLevel = 2;
  if (lconf->raster_mode == 'c' || lconf->raster_mode == 'g')
    compressionLevel = 7;

  out << format(R_COMPRESSION) % compressionLevel;
  // Raster direction (1 = up)
  out << R_DIRECTION_UP;
  // start at current position
  out << R_START_AT_POS;

  for (it = tiles.begin(); it != tiles.end(); it++) {
    serializeTileTo(*it, out);
  }
  out << "\e*rC"; // end raster
  out << "\26" << "\4"; // some end of file markers
}

//TODO rewrite to serialize directly from tiles & port to c++
void RasterPass::serializeTileTo(Tile* tile, ostream& out) {
  //TODO handle debug flag properly
  char debug = 1;
  int h;
  int d;
  int offx;
  int offy;
  int repeat;

  stringstream ss;
  ss << "/tmp/tile" << tile_cnt++ << "_" << tile->offsetX() << "_" << tile->offsetY() << ".bmp";
  string filename = ss.str();

  tile->save_bmp(filename.c_str());
  FILE* bitmap_file = fopen(filename.c_str(), "r");

  uint8_t bitmap_header[BITMAP_HEADER_NBYTES];

  repeat = this->lconf->raster_repeat;
  while (repeat--) {
    // repeated (over printed)
    int pass;
    int passes;
    long base_offset;
    if (lconf->raster_mode == 'c') {
      passes = 7;
    } else {
      passes = 1;
    }

    // Read in the bitmap header.
    fread(bitmap_header, 1, BITMAP_HEADER_NBYTES, bitmap_file);

    // Re-load width/height from bmp as it is possible that someone used  setpagedevice or some such
    // Bytes 18 - 21 are the bitmap width (little endian format).
    int bmpWidth = big_to_little_endian(bitmap_header + 18, 4);

    // Bytes 22 - 25 are the bitmap height (little endian format).
    int bmpHeight = big_to_little_endian(bitmap_header + 22, 4);

    // Bytes 10 - 13 base offset for the beginning of the bitmap data.
    base_offset = big_to_little_endian(bitmap_header + 10, 4);
    unsigned char buf[102400];

    if (lconf->raster_mode == 'c' || lconf->raster_mode == 'g') {
      // colour/grey are byte per pixel power levels
      h = bmpWidth;
      // BMP padded to 4 bytes per scan line
      d = (h * 3 + 3) / 4 * 4;
    } else {
      // mono
      h = (bmpWidth + 7) / 8;
      // BMP padded to 4 bytes per scan line
      d = (h + 3) / 4 * 4;
    }
    if (debug) {
      fprintf(stderr, "Width %d Height %d Bytes %d Line %d\n",
              bmpWidth, bmpHeight, h, d);
    }
    fseek(bitmap_file, base_offset, SEEK_SET);
    int l;
    int dir = 0;
    for(int i = 0; i < d; i++) {
      //cout << bmpWidth;
      l = fread(buf, 1, h, bitmap_file);
      //out << format(R_ROW_BYTES) % (l);
      //out << "\x03";

      /* if (dir) {
        // reverse bytes!
        for (int n = 0; n < l / 2; n++) {
          unsigned char t = buf[n];
          buf[n] = buf[l - n - 1];
          buf[l - n - 1] = t;
        }
      }
      
      ir = 1 - dir; */
      
      for (int j = 0; j < l; j++) {
        out << buf[j];
      }
    }
    
    /*
    for (offx = bmpWidth * (lconf->x_repeat - 1); offx >= 0; offx
           -= bmpWidth) {
      for (offy = bmpHeight * (lconf->y_repeat - 1); offy >= 0; offy
             -= bmpHeight) {
      
        for (pass = 0; pass < passes; pass++) {
          // raster (basic)
          int y;
          char dir = 0;

          fseek(bitmap_file, base_offset, SEEK_SET);
          for (y = bmpHeight - 1; y >= 0; y--) {
            int l;

            switch (lconf->raster_mode) {
            case 'c': // colour (passes)
              {
                unsigned char *f = buf;
                unsigned char *t = buf;
                if (d > sizeof(buf)) {
                  perror("Too wide");
                  return;
                }
                l = fread(buf, 1, d, bitmap_file);
                if (l != d) {
                  fprintf(stderr,
                          "Bad bit data from gs %d/%d (y=%d)\n",
                          l, d, y);
                  return;
                }
                while (l--) {
                  // pack and pass check RGB
                  int n = 0;
                  int v = 0;
                  int p = 0;
                  int c = 0;
                  for (c = 0; c < 3; c++) {
                    if (*f > 240) {
                      p |= (1 << c);
                    } else {
                      n++;
                      v += *f;
                    }
                    f++;
                  }
                  if (n) {
                    v /= n;
                  } else {
                    p = 0;
                    v = 255;
                  }
                  if (p != pass) {
                    v = 255;
                  }
                  *t++ = 255 - v;
                }
              }
              break;
            case 'g': // grey level
              {
                //BMP padded to 4 bytes per scan line
                int d = (h + 3) / 4 * 4;
                if (d > sizeof(buf)) {
                  fprintf(stderr, "Too wide\n");
                  return;
                }
                l = fread(buf, 1, d, bitmap_file);
                if (l != d) {
                  fprintf(stderr,
                          "Bad bit data from gs %d/%d (y=%d)\n",
                          l, d, y);
                  return;
                }
                for (l = 0; l < h; l++) {
                  buf[l] = (255 - (uint8_t) buf[l]);
                }
              }
              break;
            default: // mono
              {
                int d = (h + 3) / 4 * 4; // BMP padded to 4 bytes per scan line
                if (d > sizeof(buf)) {
                  perror("Too wide");
                  return;
                }
                l = fread(buf, 1, d, bitmap_file);
                if (l != d) {
                  fprintf(stderr,
                          "Bad bit data from gs %d/%d (y=%d)\n",
                          l, d, y);
                  return;
                }
              }
            }

            if (lconf->raster_mode == 'c' || lconf->raster_mode
                == 'g') {
              for (l = 0; l < h; l++) {
                //Raster value is multiplied by the power scale.
                buf[l] = (uint8_t) buf[l] * lconf->raster_power
                  / 255;
              }
            }

            // find left/right of data
            for (l = 0; l < h && !buf[l]; l++) {
              ;
            }

            if (l < h) {
              // a line to print
              int r;
              int n;
              unsigned char pack[sizeof(buf) * 5 / 4 + 1];
              for (r = h - 1; r > l && !buf[r]; r--) {
                ;
              }
              r++;
              out << format(PCL_POS_Y) % (tile->offsetY()
                                          + lconf->basey + offy + y);
              out << format(PCL_POS_X) % (tile->offsetX()
                                          + lconf->basex + offx
                                          + ((lconf->raster_mode == 'c'
                                              || lconf->raster_mode == 'g') ? l
                                             : l * 8));
              if (dir) {
                out << format(R_INTENSITY) % (-(r - l));
                // reverse bytes!
                for (n = 0; n < (r - l) / 2; n++) {
                  unsigned char t = buf[l + n];
                  buf[l + n] = buf[r - n - 1];
                  buf[r - n - 1] = t;
                }
              } else {
                out << format(R_INTENSITY) % (r - l);
              }
              dir = 1 - dir;
              // pack
              n = 0;
              while (l < r) {
                int p;
                for (p = l; p < r && p < l + 128 && buf[p]
                       == buf[l]; p++) {
                  ;
                }
                if (p - l >= 2) {
                  // run length
                  pack[n++] = 257 - (p - l);
                  pack[n++] = buf[l];
                  l = p;
                } else {
                  for (p = l; p < r && p < l + 127 && (p + 1
                                                       == r || buf[p] != buf[p + 1]); p++) {
                    ;
                  }

                  pack[n++] = p - l - 1;
                  while (l < p) {
                    pack[n++] = buf[l++];
                  }
                }
              }
              out << format(R_ROW_BYTES) % ((n + 7) / 8 * 8);
              r = 0;
              while (r < n) {
                out << pack[r++];
              }
              while (r & 7) {
                r++;
                out << "\x80";
              }
            }
          }
        }
      }
      } */

  }
  fclose(bitmap_file);
}

RasterPass *RasterPass::createFromFile(const string &filename) {
  return new RasterPass(new Image(filename.c_str()));
}

