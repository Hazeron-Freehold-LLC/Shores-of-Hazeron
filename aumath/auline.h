//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auline.h,v 1.7 2022/02/19 18:37:59 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuLine_H
#define AuLine_H

#include "aupoint.h"

// Description:
//		AuLine class represents a 3-dimensional line.
//		Data for this line is (simultaneously) stored in two different forms:
//
//		Implicit form: Points P satisfy (N dot P) + c = 0
//
//		Explicit form: Points P satisfy P = U + Vt for some scalar t
class AUMATH_DLL AuLine
{
public:
// Description:
//		Implicit form data: Points P satisfy (N dot P) + c = 0
//		N = unit vector normal to line
	AuPoint		N;

// Description:
//		Implicit form data: Points P satisfy (N dot P) + c = 0
//		c = distance from origin to line
	double		c;	

// Description:
//		Explicit form data: Points P satisfy P = U + Vt for some scalar t
//		U = point on line
	AuPoint		U;	
// Description:
//		Explicit form data: Points P satisfy P = U + Vt for some scalar t
//		V = unit vector direction of line
	AuPoint		V;

private:
	bool		normalized;	// TRUE if normalized

public:
	inline AuLine(const AuPoint &p1,const AuPoint &p2)
	{
		// calculate explicit values
		U = p1;
		V = p2 - p1;
		V.Unit();

		// calculate implicit values
		ExplicitToImplicit();

		normalized = true;
	}

	// This constructor expects (vx,vy,vz) to describe a unit vector.
	inline AuLine(double ux, double uy, double uz, double vx, double vy, double vz)
	{
		U.x = ux;
		U.y = uy;
		U.z = uz;
		V.x = vx;
		V.y = vy;
		V.z = vz;

		// calculate implicit values
		ExplicitToImplicit();

		normalized = true;
	}

	inline AuLine()
	{
		N = AuPoint(0,1,0);
		c = 0.0;
		U = AuPoint(0,0,0);
		V = AuPoint(1,0,0);
		normalized = true;	
	}

	// Returns the shortest distance to this line from point 'p'.
	inline double Distance(const AuPoint& p) const
	{
		AuPoint vp(p - U);				// Vector from this line to the point.
		AuPoint vy = N.Cross(V);		// Perpendicular vector to N and V. This is our 2D plane Y vector; N is the X vector.
		double x = vp.Dot(N);			// Offset from line along N.
		double y = vp.Dot(vy);			// Offset from line along vy.
		return sqrt(x*x + y*y);
	}

	//  Summary:
	//		Calculate implicit values from explicit values.
	//  Description:
	//		Calculate implicit values from explicit values.
	//		That is, given U and V, solve for N and c. Refer to explicit and implicit definitions above.
	//  See Also:
	//		ImplicitToExplicit();
	void ExplicitToImplicit();

	//  Summary:
	//		Determines the point of intersection with another line.
	//  Parameters:
	//		intersection -	Address of return location for intersection point.
	//		line -	Address of line data for intersecting line.
	//  Return Value:
	//      TRUE if lines intersect, FALSE if line do not intersect (i.e. parallel).
	//  Description:
	//		Determines the point of intersection with another line.
	//		Returns FALSE if the lines do not intersect (i.e. parallel).
	inline bool Intersect(AuPoint& intersection, const AuLine& line) const
	{
		double d = N.Dot(line.V);
		if(d == 0)
			return false;	// lines are parallel
		intersection = line.U + line.V * ((c - N.Dot(line.U)) / d);
		return true;
	}
	
private:
	void Normalize();
};

#endif // AuLine_H
