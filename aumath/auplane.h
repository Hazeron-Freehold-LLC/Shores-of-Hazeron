//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auplane.h,v 1.6 2022/02/19 18:38:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuPlane_H
#define AuPlane_H

#include "auline.h"
#include "aupoint.h"

// Description:
//		AuPlane class represents a 3-dimensional plane.
//		Note that this plane is infinite.
//		Data for a plane is stored in two variables, a vector representing the normal to the plane, and an offset 
//		from the origin to the plane.
class AUMATH_DLL AuPlane
{
public:
	AuPoint		N;	// Unit vector normal to the plane.
	double		d;	// Offset from the origin to the plane.

public:
	inline AuPlane(const AuPoint &fOrigin,const AuPoint &fXVec,const AuPoint &fYVec)
	{
		N = fXVec.Cross(fYVec);
		N.Unit();
		d = fOrigin.Dot(N);
	}

	inline AuPlane(const AuPoint &fOrigin,const AuPoint &fNormal)
	{
		N = fNormal;
		N.Unit();
		d = fOrigin.Dot(N);
	}

	inline AuPlane()
	{
		N = AuPoint(0,0,1);
		d = 0;
	}
	
	//  Summary:
	//		Returns the altitude of a given point above/below this plane.
	//  Parameters:
	//		PtToTest - Address of given point to check.	
	//  Return Value:
	//		double - The distance from the given point to the plane.  Positive is above, negative is below.
	//  Description:
	//		Returns the altitude of a given point above/below this plane.
	//		Positive is above, negative is below.
	//  See Also:
	//      ContainsPoint()
	inline double Altitude(const AuPoint &PtToTest) const
	{
		AuPoint PtOfOrigin = N * d;
		AuPoint VecFromOrigin = PtToTest - PtOfOrigin;
		double rPerpDistFromPlane_Signed = VecFromOrigin.Dot(N);
		return rPerpDistFromPlane_Signed;
	}
	
	//  Summary:
	//		Ensures that the normal vector doesn't point down.
	//  Description:
	//		Ensures that the normal vector doesn't point down.
	//		(i.e. have a negative 'z' component).
	inline void MakeNormalPointUp()
	{
		if (N.z < 0)
		{
			N *= -1;
			d *= -1;
		}
	}
	
	//  Summary:
	//		Determines if this plane is fuzzy vertical
	//  Return Value:
	//		TRUE if fuzzy vertical, FALSE if not.   
	//  Description:
	//		Determines if this plane is fuzzy vertical
	//		(i.e. normal has no 'z' component).
	inline bool IsPlaneFuzzyVertical() const
	{
		if (IsFuzzyZero(N.z))
			return true;
		else
			return false;
	}
	
	//  Summary:
	//		Determines if given point lies within this plane.
	//  Parameters:
	//		PtToTest -	Given point to test.
	//  Return Value:
	//      TRUE if the given point lies in the plane, FALSE if not.
	//  Description:
	//		Determines if given point lies within this plane.
	inline bool ContainsPoint(const AuPoint &PtToTest) const
	{
		double rPerpDistFromPlane_Signed = Altitude(PtToTest);
		if (IsFuzzyZero(rPerpDistFromPlane_Signed))
			return true;
		else
			return false;
	}
	
	//  Summary:
	//		Find the 3D intersection a point in this plane with a givne line.
	//  Parameters:
	//		fLine - The address of the given line to intersect with.
	//		fIntersect -	The return address for the intersection point.
	//  Return Value:
	//      TRUE if the line and plane intersect, FALSE if they do not (indicating line is parallel to the plane).
	//  Description:
	//		Find the 3D intersection a point in this plane with a givne line.
	//		Note that the line is infinite in both directions.
	//		returns FALSE if the line is parallel to the plane.
	//  See Also:
	//      SegmentIntersection()
	inline bool LineIntersection(const AuLine &fLine,AuPoint &fIntersect) const
	{
		double rDot = fLine.V.Dot(N);
		if(rDot == 0)
			return(false);	// fLine is parallel to this plane
		
		double t = (d - fLine.U.Dot(N))/rDot;
		fIntersect = fLine.U + fLine.V*t;
		return(true);
	}

	// Returns true if this plane intersects plane 'p'.
	// The intersection is returned in 'l'.
	bool PlaneIntersection(const AuPlane& p, AuLine& l) const;

	//  Summary:
	//		Attempts to find an intersection point between this plane and a given line segment.
	//  Parameters:
	//		PtSegmentEnd1 - Address of first end of given line segment.	
	//		PtSegmentEnd2 - Address of second end of given line segment.	
	//		PtOfIntersection -	The return address for the intersectoin point
	//  Return Value:
	//      TRUE if the line segment and plane intersect, FALSE if they do not.
	//  Description:
	//		Attempts to find an intersection point between this plane and a given line segment.
	//		If found, sets PtOfIntersection and returns TRUE. Otherwise, returns FALSE.     
	//  See Also:
	//      LineIntersection()
	inline bool SegmentIntersection(const AuPoint& PtSegmentEnd1,
			const AuPoint& PtSegmentEnd2, AuPoint& PtOfIntersection) const
	{
		bool bFound = false;
		
		AuLine InfiniteLine(PtSegmentEnd1, PtSegmentEnd2);
		AuPoint PtOfIntersection_;
		if (LineIntersection(InfiniteLine, PtOfIntersection_) && PtOfIntersection_.Between(PtSegmentEnd1, PtSegmentEnd2))
		{
			PtOfIntersection = PtOfIntersection_;
			bFound = true;
		}
		
		return bFound;
	}
	
	//  Summary:
	//      Returns the closest point on the plane to the origin.
	//  Return Value:
	//      AuPoint - The closest point on the plane to the origin.
	//  Description:
	//      Returns the closest point on the plane to the origin.
	inline AuPoint Origin() const
	{
		return N * d;
	}
};

#endif // AuPlane_H
