//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auplane.cpp,v 1.2 2022/02/19 18:38:14 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auplane.h"
//

// Returns true if this plane intersects plane 'p'.
// The intersection is returned in 'l'.
bool AuPlane::PlaneIntersection(const AuPlane& p, AuLine& l) const
{
	double pdot = N.Dot(p.N);
	if(FuzzyEquals(fabs(pdot), 1))
		return false;				// 'p' is parallel to this plane.

	AuPoint V = N.Cross(p.N);		// Vector of the intersection line.
	V.Unit();

	AuPoint pv = p.N.Cross(V);		// 'p' plane vector to intersection line.
	pv.Unit();						// Should already be a unit vector.

	double rDot = pv.Dot(N);

	AuPoint pO = p.Origin();
	double t = (d - pO.Dot(N))/rDot;
	AuPoint U = pO + pv*t;

#if !defined(Q_NO_DEBUG)
	// Test to see if this algorithm fails.
	Q_ASSERT(IsFuzzyZero((U - Origin()).Dot(N)));
	Q_ASSERT(IsFuzzyZero((U - p.Origin()).Dot(p.N)));
#endif
	l = AuLine(U.x, U.y, U.z, V.x, V.y, V.z);	// Intersection line.
	return true;
}

