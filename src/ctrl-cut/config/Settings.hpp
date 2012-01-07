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

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <util/Logger.hpp>

#include <stdint.h>
#include <iostream>
#include <string>

#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <functional>

#include <boost/any.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include "util/Measurement.hpp"
#include "cut/geom/Geometry.hpp"

using std::string;

typedef uint64_t CtrlCutID_t;
class Settings
{
public:

  class setting_not_found : public std::exception
  {
  public:
    const string keyid;
    setting_not_found(const string keyid) : keyid(keyid) {}
    virtual ~setting_not_found() throw () {}

  public:
      virtual const char * what() const throw() {
          return ("setting_not_found: " + keyid).c_str();
      }
  };

  struct KeyBase {
    const string name;

    KeyBase(const char* name):
      name(name)
    {};

    KeyBase(const string& name):
       name(name)
     {};

    bool operator<(const KeyBase& other) const{
      return this->name < other.name;
    }

    operator std::string() const{
      return this->name;
    }
  };

  template<typename T>
  class Key : public KeyBase {
  public:
    Key(const char* name): KeyBase(name) {};
    Key(const string& name): KeyBase(name) {};
  };

  typedef boost::any Value;
  typedef std::map<KeyBase,Value>  SettingsMap;
  typedef SettingsMap::iterator iterator;
  typedef SettingsMap::const_iterator const_iterator;
  typedef SettingsMap::reference reference;
  typedef SettingsMap::const_reference const_reference;

  Settings(Settings& parent) : parent(&parent)  {};
  Settings(const Settings& other) : parent(NULL)  {
    if(other.hasParent()) {
      this->parent = &other.getParent();
    }

    this->properties = other.properties;
  };

  Settings() : parent(NULL)  {};

  string key(int i) {
    iterator it;
    for(it = properties.begin(); i > 0; --i) {
      it++;
    }
    return (*it).first;
  }

  string value(int i) {
    iterator it;
    for(it = properties.begin(); i > 0; --i) {
      it++;
    }
    boost::any prop = (*it).second;

    if(prop.type() == typeid(Measurement)) {
      return boost::lexical_cast<string>(boost::any_cast<Measurement>(prop));
    } else if(prop.type() == typeid(Point)) {
      return boost::lexical_cast<string>(boost::any_cast<Point>(prop));
    } else if(prop.type() == typeid(string)){
      return boost::lexical_cast<string>(boost::any_cast<string>(prop));
    } else if(prop.type() == typeid(float)){
      return boost::lexical_cast<string>(boost::any_cast<float>(prop));
    } else if(prop.type() == typeid(bool)){
      return boost::lexical_cast<string>(boost::any_cast<bool>(prop));
    } else if(prop.type() == typeid(uint16_t)){
      return boost::lexical_cast<string>(boost::any_cast<uint16_t>(prop));
    } else if(prop.type() == typeid(uint32_t)){
      return boost::lexical_cast<string>(boost::any_cast<uint32_t>(prop));
    }
    return "unknown" + i;
  }

  void set(int i, const string& val) {
    iterator it;
    for(it = properties.begin(); i > 0; --i) {
      it++;
    }
    KeyBase key = (*it).first;
    boost::any prop = (*it).second;
    if(prop.type() == typeid(Measurement)) {
      this->put(Key<Measurement>(key.name), boost::lexical_cast<Measurement>(val));
    } else if(prop.type() == typeid(Point)) {
      this->put(Key<Point>(key.name), boost::lexical_cast<Point>(val));
    } else if(prop.type() == typeid(string)){
      this->put(Key<string>(key.name), val);
    } else if(prop.type() == typeid(float)){
      this->put(Key<float>(key.name), boost::lexical_cast<float>(val));
    } else if(prop.type() == typeid(bool)){
      this->put(Key<bool>(key.name), boost::lexical_cast<bool>(val));
    } else if(prop.type() == typeid(uint16_t)){
      this->put(Key<uint16_t>(key.name), boost::lexical_cast<uint16_t>(val));
    } else if(prop.type() == typeid(uint32_t)){
      this->put(Key<uint32_t>(key.name), boost::lexical_cast<uint32_t>(val));
    }
    std::cerr << "unkown type for: " << key.name << std::endl;
  }

  void clear() { return this->properties.clear(); }
  size_t size() const { return this->properties.size(); }

  template<typename T, typename V>
  void put(const Settings::Key<T>& key, V value) {
    properties[key] = boost::any(static_cast<T>(value));
  }

  template<typename T>
  const T get(const Settings::Key<T>& key) const {
    const_iterator it = this->properties.find(key);
    if (it != this->properties.end()) {
      return boost::any_cast<T>((*it).second);
    } else if(parent != NULL) {
      return parent->get(key);
    }

    boost::throw_exception(setting_not_found(key));
  }

  bool hasParent() const {
    return this->parent != NULL;
  }

  Settings& getParent() const {
    assert(this->hasParent());
    return *this->parent;
  }

  void operator=(const Settings& other) {
    if(other.hasParent())
      this->parent = &other.getParent();

    this->properties = other.properties;
  }
protected:
  Settings* parent;
  SettingsMap properties;
};

#endif /* SETTINGS_H_ */
