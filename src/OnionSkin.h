/*
 * OnionSkin.h
 *
 *  Created on: 03.10.2009
 *      Author: amir
 */

#ifndef ONIONSKIN_H_
#define ONIONSKIN_H_

#include <list>
#include "Polygon.h"

class OnionSkin: public Polygon {
	private:
	list<LineSegment*> segments;
public:
	OnionSkin();
	virtual ~OnionSkin();
	void addLineSegment(LineSegment* ls);
	list<LineSegment*> getLineSegments();
	list<LineSegment*>::iterator findLineSegment(LineSegment* ls);
	void eraseLineSegment(LineSegment* ls);
	bool hasLineSegment(LineSegment* ls);
	int getSegmentCount();
};

#endif /* ONIONSKIN_H_ */