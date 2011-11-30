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

#ifndef SVGFIX_H_
#define SVGFIX_H_

#include <stdio.h>
#include <ctype.h>
#include <string>
#include <iostream>
#include <sstream>

#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

#include <libxml++/libxml++.h>
#include <fstream>
#include "LaserConfig.h"
#include <iostream>
#include "SvgDocument.h"

using std::getline;
using std::istream;
using std::string;
using std::stringstream;
using std::pair;
using xmlpp::SaxParser;

class SvgFix {
private:
  int fdIn, fdOut;
public:
  typedef SaxParser::Attribute Attribute;
  typedef SaxParser::AttributeList AttributeList;

  enum DocGenerator {
    Unknown, Inkscape, CorelDraw
  };
  SvgDocument document;
  DocGenerator generator;

  void dump(std::ostream& out, const Glib::ustring& name, const SaxParser::AttributeList& properties);
  void findGenerator(const Glib::ustring& text);
  void fixViewbox(std::ostream& out, const Glib::ustring& name, const AttributeList& properties);

  SvgFix(int fdIn, int fdOut) : fdIn(fdIn), fdOut(fdOut), generator(Unknown){ }
  virtual ~SvgFix(){}
  void work();
};

#endif /* SVGFIX_H_ */
