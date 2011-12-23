#ifndef GEOMETRY_H_
#define GEOMETRY_H_

#include <algorithm>
#include <iostream>
#include <stdint.h>
#include <deque>
#include <limits>
#include <list>
#include <assert.h>
#include <boost/math/special_functions.hpp>
#include <boost/tuple/tuple.hpp>

static const float CC_PI = atan2(0,-1);

struct OpParams {
  int power;
  int speed;
  int frequency;

  OpParams(): power(0), speed(0), frequency(0) {};
  OpParams(int power, int speed, int frequency = 5000): power(power), speed(speed), frequency(frequency) {}
};

class Point {
public:
  int32_t x;
  int32_t y;

  Point(): x(0), y(0) {}
  Point(int32_t x, int32_t y): x(x), y(y) {}

  const int32_t &operator[](size_t idx) const {
    assert(idx < 2);
    if (idx == 0)
      return x;
    else
      return y;
  }

  bool operator<(const Point&  other) const {
      return  (this->x < other.x) || ((this->x == other.x) && (this->y < other.y));
  }

  bool operator==(const Point&  other) const {
    return this->x == other.x && this->y == other.y;
  }

  bool operator!=(const Point&  other) const {
    return this->x != other.x || this->y != other.y;
  }

  Point& operator-(const Point&  other) const {
    int32_t x_diff = this->x - other.x;
    int32_t y_diff = this->y - other.y;
    return (* new Point(x_diff, y_diff));
  }

  void operator=(const Point&  other) {
    this->x = other.x;
    this->y = other.y;
  }

  double distance(const Point&  other) const {
    return hypot(double(this->x - other.x), double(this->y - other.y));
  }
};

class Box {
public:
  const Point min_corner;
  const Point max_corner;

  Box(const Point&  min_corner, const Point&  max_corner) : min_corner(min_corner) , max_corner(max_corner) {}
};

class Sphere {
public:
  const Point center;
  const uint32_t radius;

  Sphere(const Point&  center, const uint32_t& radius) : center(center) , radius(radius) {}
};

class Segment {
public:
  Point first;
  Point second;

  OpParams settings;
  Segment() {}
  Segment(const Point&  first, const Point&  second, const OpParams& settings) : first(first), second(second), settings(settings), box(NULL), sphere(NULL) {
    if(first < second)
      this->box = new Box(first, second);
    else
      this->box = new Box(second, first);

    Point& diff = this->box->max_corner - this->box->min_corner;
    Point center = (* new Point(this->box->min_corner.x + (diff.x / 2), this->box->min_corner.y + (diff.y / 2)));
    uint32_t radius = boost::math::hypot((diff.x / 2), (diff.y / 2));
    this->sphere = new Sphere(center, radius);
  }

  const Point& operator[](size_t idx) const {
    assert(idx < 2);
    if (idx == 0)
      return first;
    else
      return second;
  }

  Point& operator[](size_t idx) {
    assert(idx < 2);
    if (idx == 0)
      return first;
    else
      return second;
  }

  bool operator<(const Segment& other) const {
      return  (this->first < other.first) || ((this->first == other.first) && (this->second < other.second));
  }

  bool operator==(const Segment& other) const {
    return this->first == other.first && this->second == other.second;
  }

  bool operator!=(const Segment& other) const {
    return ! (*this == other);
  }

  const Box& getBox() const {
    return *this->box;
  }

  const Sphere& getSphere() const {
    return *this->sphere;
  }

  /*!
    Calculates the distance from the Point to the infinite segment
  */
  float distance(const Point& p) const;
  /*!
    Returns angle to the positive Y axis
  */
  float getSlope(bool invert = false) const;

  const class SegmentString* getOwner() {
    return this->owner;
  }

  void setOwner(SegmentString* string) {
    assert(this->owner == NULL);
    this->owner = string;
  }
private:
  Box* box;
  Sphere* sphere;
  SegmentString* owner;
};

class SegmentString {
public:
  typedef uint64_t id_t;
  typedef std::list<Segment*> SegmentList;
  typedef std::list<Point*> PointList;
  typedef SegmentList::iterator SegmentIter;
  typedef SegmentList::const_iterator SegmentConstIter;
  typedef PointList::iterator PointIter;
  typedef PointList::const_iterator PointConstIter;
  ~SegmentString() {
    for(SegmentIter it = beginSegments(); it != endSegments(); it++) {
     // (*it)->setOwner(NULL);
    }
  }
  SegmentIter beginSegments() { return this->segments.begin(); }
  SegmentConstIter beginSegments() const  { return this->segments.begin(); }
  SegmentIter endSegments() { return this->segments.end(); }
  SegmentConstIter endSegments() const  { return this->segments.end(); }
  SegmentList::reference frontSegments() { return this->segments.front(); }
  SegmentList::reference backSegments() { return this->segments.back(); }
  SegmentList::const_reference frontSegments() const { return this->segments.front(); }
  SegmentList::const_reference backSegments() const { return this->segments.back(); }
  size_t numSegments() const { return this->segments.size(); }
  bool segmentsEmpty() const { return this->segments.empty(); }

  PointIter beginPoints() { return this->points.begin(); }
  PointConstIter beginPoints() const  { return this->points.begin(); }
  PointIter endPoints() { return this->points.end(); }
  PointConstIter endPoints() const  { return this->points.end(); }
  PointList::reference frontPoints()  { return this->points.front(); }
  PointList::reference backPoints()  { return this->points.back(); }
  PointList::const_reference frontPoints() const { return this->points.front(); }
  PointList::const_reference backPoints() const { return this->points.back(); }
  void clear() {
    this->segments.clear();
    this->points.clear();
  }

  bool empty() {
    return this->segments.empty() && this->points.empty();
  }

  size_t numPoints() const { return this->points.size(); }
  bool pointsEmpty() const { return this->points.empty(); }

  bool isClosed() const { return *this->frontPoints() == *this->backPoints(); }

  bool rotate(const Point&  first) {
    assert(isClosed());
    if(first != *frontPoints()) {
      for(PointIter it = beginPoints()++; it != endPoints(); ++it) {
        if(**it == first) {
          points.splice(endPoints(), points, beginPoints(), --it);
          break;
        }
      }

      for(SegmentIter it = beginSegments()++; it != endSegments(); ++it) {
        if((*it)->first == first) {
          segments.splice(endSegments(), segments, beginSegments(), --it);
          return true;
        }
      }
    } else
      return true;

    return false;
  }

  bool add(Segment* const seg) {
    if (segments.empty()) {
      segments.push_back(seg);
      points.push_back(&seg->first);
      points.push_back(&seg->second);
      return true;
    } else {
      const Point&  last = *points.back();
      if (last == seg->first) {
        segments.push_back(seg);
        points.push_back(&seg->second);
      } else if (last == seg->second) {
        segments.push_back(new Segment(seg->second, seg->first, seg->settings));
        points.push_back(&seg->first);
      } else {
        const Point&  first = *points.front();
        if (first == seg->first) {
          segments.push_front(new Segment(seg->second, seg->first, seg->settings));
          points.push_front(&seg->second);
        } else if (first == seg->second) {
          segments.push_front(seg);
          points.push_front(&seg->first);
        } else {
          return false;
        }
      }
      return true;
    }
    return false;
  }
private:
  SegmentList segments;
  PointList points;
};

typedef std::list<Segment*> SegmentList;
typedef std::list<SegmentString*> StringList;

enum intersection_result { ALIGN_NONE, ALIGN_INTERSECT, ALIGN_COINCIDENCE, ALIGN_PARALLEL };

/*!
  Calculates alignment of two segments to each other and sets the supplied point to the resulting point if they intersect
  Tip intersections are reported as ordinary intersections.
*/
inline intersection_result intersects(const Segment& s1, const Segment&s2, Point& intersection)
{
  float denom =
    ((s2[1][1] - s2[0][1]) * (s1[1][0] - s1[0][0])) -
    ((s2[1][0] - s2[0][0]) * (s1[1][1] - s1[0][1]));

  float nume_a =
    ((s2[1][0] - s2[0][0]) * (s1[0][1] - s2[0][1])) -
    ((s2[1][1] - s2[0][1]) * (s1[0][0] - s2[0][0]));

  float nume_b =
    ((s1[1][0] - s1[0][0]) * (s1[0][1] - s2[0][1])) -
    ((s1[1][1] - s1[0][1]) * (s1[0][0] - s2[0][0]));

  if (denom == 0.0f) {
    if (nume_a == 0.0f && nume_b == 0.0f)
      return ALIGN_COINCIDENCE;
    else
      return ALIGN_PARALLEL;
  }

  float ua = nume_a / denom;
  float ub = nume_b / denom;

  if (ua >= 0.0f && ua <= 1.0f && ub >= 0.0f && ub <= 1.0f) {
    intersection.x = s1[0][0] + ua * (s1[1][0] - s1[0][0]);
    intersection.y = s1[0][1] + ua * (s1[1][1] - s1[0][1]);

    return ALIGN_INTERSECT;
  }

  return ALIGN_NONE;
}
inline std::ostream& operator<<(std::ostream &os, const Point& p)  {
  os << "<point x=\"" << p.x << "\" y=\"" << p.y << "\" key=\"" << p.x << "/" << p.y << "\" />";
  return os;
}

inline std::ostream& operator<<(std::ostream &os, const Segment& segment) {
  os << "<segment>" << std::endl;
  os << "  " << segment.first << std::endl;
  os << "  " << segment.second << std::endl;
  os << "</segment>";
  return os;
}

inline std::ostream& operator<<(std::ostream &os, const SegmentString& string) {
  os << "<string>" << std::endl;
  for(SegmentString::PointConstIter it=string.beginPoints(); it != string.endPoints(); ++it)
    os << **it << std::endl;

  for(SegmentString::SegmentConstIter it=string.beginSegments(); it != string.endSegments(); ++it)
    os << **it << std::endl;

  os << "</string>" << std::endl;
  return os;
}

// explicit mixed sign comparison to prevent warnings
inline bool greater_than(int32_t s, uint32_t us) {
  return s > (int32_t)us;
}

// explicit mixed sign comparison to prevent warnings
inline bool lesser_than(int32_t s, uint32_t us) {
  return s < (int32_t)us;
}

void translate(SegmentList& segments, SegmentList::iterator first, SegmentList::iterator last, const Point& translation);

#endif /* GEOMETRY_H_ */
