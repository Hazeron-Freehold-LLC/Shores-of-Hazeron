//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausphere.h,v 1.5 2022/02/19 18:38:08 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSphere_H
#define AuSphere_H

#include "aupoint.h"

class AUMATH_DLL AuSphere
{
private:
	AuPoint								m_Center;
	double								m_Radius;

public:
	inline AuSphere()
	{
		m_Radius = 1;
	}

	inline AuSphere(const AuPoint& center, double radius)
	{
		m_Center = center;
		m_Radius = radius;
	}

	inline AuSphere(const AuSphere& c)
	{
		m_Center = c.m_Center;
		m_Radius = c.m_Radius;
	}

	inline const AuPoint& Center() const
	{
		return m_Center;
	}

	// Calculate intersection(s) with this sphere.
	// 'seg0' and 'seg1' are the ends of a line segment to intersect with this sphere.
	// 'intersect0' and 'intersect1' are the resulting intersect points, if any.
	// Return value:
	//  -1 = no intersect, segment is inside sphere
	//   0 = no intersect, segment is outside sphere
	//   1 = 1 intersection, result in 'intersect0'
	//   2 = 2 intersections, results in 'intersect0' and 'intersect1' where 'intersect0' is the closest intersection to 'seg0'.
	// From: http://local.wasp.uwa.edu.au/~pbourke/geometry/sphereline/
	inline int Intersect(const AuPoint& seg0, const AuPoint& seg1, AuPoint& intersect0, AuPoint& intersect1) const
	{
		// Points P (x,y) on a line defined by two points P1 (x1,y1,z1) and P2 (x2,y2,z2) is described by
		//   P = P1 + u (P2 - P1)
		// or in each coordinate
		//   x = x1 + u (x2 - x1)
		//   y = y1 + u (y2 - y1)
		//   z = z1 + u (z2 - z1)
		//
		// A sphere centered at P3 (x3,y3,z3) with radius r is described by
		//   (x - x3)^2 + (y - y3)^2 + (z - z3)^2 = r^2
		//
		// Substituting the equation of the line into the sphere gives a quadratic equation of the form
		//   a u^2 + b u + c = 0
		//
		// where:
		//   a = (x2 - x1)^2 + (y2 - y1)^2 + (z2 - z1)^2
		AuPoint segVec = seg1 - seg0;
		double a = segVec.x*segVec.x + segVec.y*segVec.y + segVec.z*segVec.z;

		//   b = 2[ (x2 - x1) (x1 - x3) + (y2 - y1) (y1 - y3) + (z2 - z1) (z1 - z3) ]
		AuPoint cenVec = seg0 - m_Center;
		double b = 2*(segVec.x*cenVec.x + segVec.y*cenVec.y + segVec.z*cenVec.z);

		//   c = x3^2 + y3^2 + z3^2 + x1^2 + y1^2 + z1^2 - 2[x3 x1 + y3 y1 + z3 z1] - r^2
		double r2 = m_Radius*m_Radius;
		double c = m_Center.x*m_Center.x + m_Center.y*m_Center.y + m_Center.z*m_Center.z +
			seg0.x*seg0.x + seg0.y*seg0.y + seg0.z*seg0.z - 2*(m_Center.x*seg0.x + m_Center.y*seg0.y + m_Center.z*seg0.z) - r2;

		// The solutions to this quadratic are described by
		//          -b +- sqrt(b^2 - 4ac)
		//         -----------------------
		//                  2a
		//
		// The exact behaviour is determined by the expression within the square root
		double b24ac = b*b - 4*a*c;
		if(IsFuzzyZero(b24ac))
		{
			// If it equals 0 then the line is a tangent to the sphere intersecting it at one point, namely at u = -b/2a.
			double u = -b/(2*a);
			if(u >= 0 && u <= 1)
			{
				// Line intersection is between the ends of the segment.
				intersect0 = seg0 + segVec * u;
				return 1;
			}
		}
		else if(b24ac > 0)
		{
			// If it is greater then 0 the line intersects the sphere at two points.
			// If this is less than 0 then the line does not intersect the sphere and it is outside the sphere. 
			double sqb24ac = sqrt(b24ac);
			double a2 = 2 * a;
			double u0 = (-b - sqb24ac) / a2;
			double u1 = (-b + sqb24ac) / a2;
			if((u0 < 0 && u1 < 0) || (u0 > 1 && u1 > 1))
				return 0;				// Line segment doesn't intersect and on outside of sphere, in which case both values of u
										// will either be less than 0 or greater than 1.

			if((u0 < 0 && u1 > 1) || (u0 > 1 && u1 < 0))
				return -1;				// Line segment doesn't intersect and is inside sphere, in which case one value of u
										// will be negative and the other greater than 1.

			bool u0ok = (u0 >= 0 && u0 <= 1) ? true : false;
			bool u1ok = (u1 >= 0 && u1 <= 1) ? true : false;
			if(u0ok && u1ok)
			{
				if(::FuzzyEquals(u0, u1))
				{
					// Line segment is tangential to the sphere, in which case both values of u will be the same and between 0 and 1.
					intersect0 = seg0 + segVec * u0;
					return 1;
				}
				else
				{
					// Line segment intersects at two points, in which case both values of u will be between 0 and 1.
					if(u0 < u1)
					{
						// u0 is the closest intersection to seg0.
						intersect0 = seg0 + segVec * u0;
						intersect1 = seg0 + segVec * u1;
					}
					else
					{
						// u1 is the closest intersection to seg0.
						intersect0 = seg0 + segVec * u1;
						intersect1 = seg0 + segVec * u0;
					}
					return 2;
				}
			}
			else if(u0ok)
			{
				// Line segment intersects at one point, in which case one value of u will be between 0 and 1 and the other not.
				intersect0 = seg0 + segVec * u0;
				return 1;
			}
			else if(u1ok)
			{
				// Line segment intersects at one point, in which case one value of u will be between 0 and 1 and the other not.
				intersect0 = seg0 + segVec * u1;
				return 1;
			}
		}

		// No intersection was found between the segment end points.
		return 0;
	}

	inline double Radius() const
	{
		return m_Radius;
	}

	// Returns the radius squared.
	inline double Radius2() const
	{
		return m_Radius*m_Radius;
	}
};

#endif // AuSphere_H
