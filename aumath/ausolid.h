//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausolid.h,v 1.14 2022/02/19 18:38:07 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSolid_H
#define AuSolid_H

#include "aumatrix4x4.h"
#include "aupoint.h"

class AuBoundaryComplex;
class AuScFaceList;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSolidShape
{
	AuMatrix4x4							m_M;					// Projection matrix for this shape.

public:	// Construction
	inline AuSolidShape(const AuMatrix4x4& m)
	{
		m_M = m;
	}

	virtual ~AuSolidShape()
	{
	}

	inline AuSolidShape(const AuSolidShape& c)
	{
		m_M = c.m_M;
	}

public:	// Operators

public:	// Methods
	virtual AuSolidShape* Clone() const = 0;

	virtual void Extents(AuPoint& min, AuPoint& max) const = 0;

	virtual void Faces(AuScFaceList& faces, double stepSize) const = 0;

	// Returns true if 'p' is inside this solid shape.
	virtual bool IsInside(const AuPoint& p) const = 0;

	inline const AuMatrix4x4& M() const
	{
		return m_M;
	}

	// Tests for intersection by a segment from 'eyeLoc' to 'sightLoc'.
	// Returns false if not hit.
	// Returns true if hit and sets 'sightLoc' to the closest point on the sight segment to 'eyeLoc' that hits this solid.
	virtual bool SightTarget(const AuPoint& eyeLoc, AuPoint& sightLoc) const = 0;

	virtual void SurfacePoints(AuPointNArray& points, double stepSize) const
	{
		Q_UNUSED(points);
		Q_UNUSED(stepSize);
	}

	// Adds my topology to 'topo', in the Z='zHeight' plane.
	virtual void TopologyAtZ(AuBoundaryComplex& topo, double zHeight, double stepSize) const
	{
		Q_UNUSED(topo);
		Q_UNUSED(zHeight);
		Q_UNUSED(stepSize);
	}

	inline void Transform(const AuMatrix4x4& m)
	{
		m_M *= m;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSolidShapes : public QList<AuSolidShape*>
{
public:	// Construction
	inline AuSolidShapes()
	{
	}

	inline ~AuSolidShapes()
	{
		while(!empty())
			delete takeFirst();
	}

	inline AuSolidShapes(const AuSolidShapes& c) : QList<AuSolidShape*>()
	{
		for(const_iterator si = c.cbegin(); si != c.cend(); si++)
			append((*si)->Clone());
	}

public:	// Operators

public:	// Methods
	// Adds a clone of 's' to this list of shapes.
	void Add(const AuSolidShape& s);

	// Returns true if 'min' and 'max' are set.
	bool Extents(AuPoint& min, AuPoint& max) const;

	void Faces(AuScFaceList& faces, double stepSize) const;

	// Tests for intersection by a segment from 'eyeLoc' to 'sightLoc'.
	// Returns false if not hit.
	// Returns true if hit and sets 'sightLoc' to the closest point on the sight segment to 'eyeLoc' that hits this solid.
	bool SightTarget(const AuPoint& eyeLoc, AuPoint& sightLoc) const;

	void SurfacePoints(AuPointNArray& points, double stepSize) const;

	// Adds my topology to 'topo', in the Z='zHeight' plane.
	void TopologyAtZ(AuBoundaryComplex& topo, double zHeight, double stepSize) const;

	void Transform(const AuMatrix4x4& m);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSolidCylinder : public AuSolidShape
{
	bool								m_FillBase;
	bool								m_FillTop;
	double								m_Height;
	double								m_Radius;

public:
	inline AuSolidCylinder(const AuMatrix4x4& m, double height, double radius, bool fillBase, bool fillTop) : AuSolidShape(m)
	{
		m_FillBase = fillBase;
		m_FillTop = fillTop;
		m_Height = height;
		m_Radius = radius;
	}

	inline AuSolidCylinder(const AuSolidCylinder& c) : AuSolidShape(c)
	{
		m_FillBase = c.m_FillBase;
		m_FillTop = c.m_FillTop;
		m_Height = c.m_Height;
		m_Radius = c.m_Radius;
	}

public:
	virtual AuSolidShape* Clone() const;

	virtual void Extents(AuPoint& min, AuPoint& max) const;

	virtual void Faces(AuScFaceList& faces, double stepSize) const;

	virtual bool IsInside(const AuPoint& p) const;

	virtual bool SightTarget(const AuPoint& eyeLoc, AuPoint& sightLoc) const;

	virtual void TopologyAtZ(AuBoundaryComplex& topo, double zHeight, double stepSize) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSolidCone : public AuSolidShape
{
	bool								m_FillBase;
	bool								m_FillTop;				// n/a if m_RadiusTop is zero.
	double								m_Height;
	double								m_RadiusBase;			// Zero to make a pointed cone, non-zero for a flat end.
	double								m_RadiusTop;			// Zero to make a pointed cone, non-zero for a flat end.

public:
	inline AuSolidCone(const AuMatrix4x4& m, double height, double radiusBase, double radiusTop, bool fillBase, bool fillTop) :
		AuSolidShape(m)
	{
		m_FillBase = fillBase;
		m_FillTop = fillTop;
		m_Height = height;
		m_RadiusBase = radiusBase;
		m_RadiusTop = radiusTop;
	}

	inline AuSolidCone(const AuSolidCone& c) : AuSolidShape(c)
	{
		m_FillBase = c.m_FillBase;
		m_FillTop = c.m_FillTop;
		m_Height = c.m_Height;
		m_RadiusBase = c.m_RadiusBase;
		m_RadiusTop = c.m_RadiusTop;
	}

public:
	virtual AuSolidShape* Clone() const;

	virtual void Extents(AuPoint& min, AuPoint& max) const;

	virtual void Faces(AuScFaceList& faces, double stepSize) const;

	virtual bool IsInside(const AuPoint& p) const;

	double RadiusAtZ(double z) const;

	virtual bool SightTarget(const AuPoint& eyeLoc, AuPoint& sightLoc) const;

	virtual void SurfacePoints(AuPointNArray& points, double stepSize) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSolidConoid : public AuSolidShape
{
	bool								m_FillBase;
	bool								m_FillTop;				// n/a if m_RadiusTop is zero.
	double								m_Height;
	double								m_RadiusBaseX;			// Zero to make a pointed cone, non-zero for a flat end.
	double								m_RadiusBaseY;			// Zero to make a pointed cone, non-zero for a flat end.
	double								m_RadiusTopX;			// Zero to make a pointed cone, non-zero for a flat end.
	double								m_RadiusTopY;			// Zero to make a pointed cone, non-zero for a flat end.

public:
	inline AuSolidConoid(const AuMatrix4x4& m, double height, double radiusBaseX, double radiusBaseY, double radiusTopX,
			double radiusTopY, bool fillBase, bool fillTop) :
		AuSolidShape(m)
	{
		m_FillBase = fillBase;
		m_FillTop = fillTop;
		m_Height = height;
		m_RadiusBaseX = radiusBaseX;
		m_RadiusBaseY = radiusBaseY;
		m_RadiusTopX = radiusTopX;
		m_RadiusTopY = radiusTopY;
	}

	inline AuSolidConoid(const AuSolidConoid& c) : AuSolidShape(c)
	{
		m_FillBase = c.m_FillBase;
		m_FillTop = c.m_FillTop;
		m_Height = c.m_Height;
		m_RadiusBaseX = c.m_RadiusBaseX;
		m_RadiusBaseY = c.m_RadiusBaseY;
		m_RadiusTopX = c.m_RadiusTopX;
		m_RadiusTopY = c.m_RadiusTopY;
	}

public:
	virtual AuSolidShape* Clone() const;

	virtual void Extents(AuPoint& min, AuPoint& max) const;

	virtual void Faces(AuScFaceList& faces, double stepSize) const;

	virtual bool IsInside(const AuPoint& p) const;

	double RadiusXAtZ(double z) const;

	double RadiusYAtZ(double z) const;

	virtual bool SightTarget(const AuPoint& eyeLoc, AuPoint& sightLoc) const;

	virtual void SurfacePoints(AuPointNArray& points, double stepSize) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSolidDome : public AuSolidShape
{
	bool								m_FillBase;
	double								m_Radius;

public:
	inline AuSolidDome(const AuMatrix4x4& m, double radius, bool fillBase) : AuSolidShape(m)
	{
		m_FillBase = fillBase;
		m_Radius = radius;
	}

	inline AuSolidDome(const AuSolidDome& c) : AuSolidShape(c)
	{
		m_FillBase = c.m_FillBase;
		m_Radius = c.m_Radius;
	}

public:
	virtual AuSolidShape* Clone() const;

	virtual void Extents(AuPoint& min, AuPoint& max) const;

	virtual void Faces(AuScFaceList& faces, double stepSize) const;

	virtual bool IsInside(const AuPoint& p) const;

	virtual bool SightTarget(const AuPoint& eyeLoc, AuPoint& sightLoc) const;

	virtual void SurfacePoints(AuPointNArray& points, double stepSize) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSolidDomoid : public AuSolidShape
{
	bool								m_FillBase;
	double								m_RadiusX;
	double								m_RadiusY;
	double								m_RadiusZ;

public:
	inline AuSolidDomoid(const AuMatrix4x4& m, double radiusX, double radiusY, double radiusZ, bool fillBase) : AuSolidShape(m)
	{
		m_FillBase = fillBase;
		m_RadiusX = radiusX;
		m_RadiusY = radiusY;
		m_RadiusZ = radiusZ;
	}

	inline AuSolidDomoid(const AuSolidDomoid& c) : AuSolidShape(c)
	{
		m_FillBase = c.m_FillBase;
		m_RadiusX = c.m_RadiusX;
		m_RadiusY = c.m_RadiusY;
		m_RadiusZ = c.m_RadiusZ;
	}

public:
	virtual AuSolidShape* Clone() const;

	virtual void Extents(AuPoint& min, AuPoint& max) const;

	virtual void Faces(AuScFaceList& faces, double stepSize) const;

	virtual bool IsInside(const AuPoint& p) const;

	virtual bool SightTarget(const AuPoint& eyeLoc, AuPoint& sightLoc) const;

	virtual void SurfacePoints(AuPointNArray& points, double stepSize) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// e.g. a silo with a domed top.

class AuSolidCylindomoid : public AuSolidShape
{
	double								m_CylHt;				// Height of cylinder at base.
	bool								m_FillBase;
	double								m_RadiusX;
	double								m_RadiusY;
	double								m_RadiusZ;

public:
	inline AuSolidCylindomoid(const AuMatrix4x4& m, double cylHeight, double radiusX, double radiusY, double radiusZ, bool fillBase)
		: AuSolidShape(m)
	{
		m_CylHt = cylHeight;
		m_FillBase = fillBase;
		m_RadiusX = radiusX;
		m_RadiusY = radiusY;
		m_RadiusZ = radiusZ;
	}

	inline AuSolidCylindomoid(const AuSolidCylindomoid& c) : AuSolidShape(c)
	{
		m_CylHt = c.m_CylHt;
		m_FillBase = c.m_FillBase;
		m_RadiusX = c.m_RadiusX;
		m_RadiusY = c.m_RadiusY;
		m_RadiusZ = c.m_RadiusZ;
	}

public:
	virtual AuSolidShape* Clone() const;

	virtual void Extents(AuPoint& min, AuPoint& max) const;

	virtual void Faces(AuScFaceList& faces, double stepSize) const;

	virtual bool IsInside(const AuPoint& p) const;

	virtual bool SightTarget(const AuPoint& eyeLoc, AuPoint& sightLoc) const;

	virtual void SurfacePoints(AuPointNArray& points, double stepSize) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSolidSphere : public AuSolidShape
{
	double								m_Radius;

public:
	inline AuSolidSphere(const AuMatrix4x4& m, double radius) : AuSolidShape(m)
	{
		m_Radius = radius;
	}

	inline AuSolidSphere(const AuSolidSphere& c) : AuSolidShape(c)
	{
		m_Radius = c.m_Radius;
	}

public:
	virtual AuSolidShape* Clone() const;

	virtual void Extents(AuPoint& min, AuPoint& max) const;

	virtual void Faces(AuScFaceList& faces, double stepSize) const;

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
		AuPoint myCen = M().Translation();
		AuPoint cenVec = seg0 - myCen;
		double b = 2*(segVec.x*cenVec.x + segVec.y*cenVec.y + segVec.z*cenVec.z);

		//   c = x3^2 + y3^2 + z3^2 + x1^2 + y1^2 + z1^2 - 2[x3 x1 + y3 y1 + z3 z1] - r^2
		double r2 = m_Radius*m_Radius;
		double c = myCen.x*myCen.x + myCen.y*myCen.y + myCen.z*myCen.z +
			seg0.x*seg0.x + seg0.y*seg0.y + seg0.z*seg0.z - 2*(myCen.x*seg0.x + myCen.y*seg0.y + myCen.z*seg0.z) - r2;

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

	virtual bool IsInside(const AuPoint& p) const;

	virtual bool SightTarget(const AuPoint& eyeLoc, AuPoint& sightLoc) const;

	virtual void SurfacePoints(AuPointNArray& points, double stepSize) const;

	virtual void TopologyAtZ(AuBoundaryComplex& topo, double zHeight, double stepSize) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSolidEllipsoid : public AuSolidShape
{
	double								m_RadiusX;
	double								m_RadiusY;
	double								m_RadiusZ;

public:
	inline AuSolidEllipsoid(const AuMatrix4x4& m, double radiusX, double radiusY, double radiusZ) : AuSolidShape(m)
	{
		m_RadiusX = radiusX;
		m_RadiusY = radiusY;
		m_RadiusZ = radiusZ;
	}

	inline AuSolidEllipsoid(const AuSolidEllipsoid& c) : AuSolidShape(c)
	{
		m_RadiusX = c.m_RadiusX;
		m_RadiusY = c.m_RadiusY;
		m_RadiusZ = c.m_RadiusZ;
	}

public:
	virtual AuSolidShape* Clone() const;

	virtual void Extents(AuPoint& min, AuPoint& max) const;

	virtual void Faces(AuScFaceList& faces, double stepSize) const;

	virtual bool IsInside(const AuPoint& p) const;

	// Moves 'p' closer to my center, plus a 'margin'.
	// Returns the lowered point.
	// Returns 'p' if it is not at at higher altitude than the surface plus the margin on entry.
	AuPoint LowerToSurface(const AuPoint& p, double margin) const;

	inline double RadiusX() const
	{
		return m_RadiusX;
	}

	inline double RadiusY() const
	{
		return m_RadiusY;
	}

	inline double RadiusZ() const
	{
		return m_RadiusZ;
	}

	// Moves 'p' away from my center, plus a 'margin'. e.g. (1,0) in a sphere of radius 2, elevated with a margin of .1, is (2.1,0).
	// Returns the elevated point.
	// Returns 'p' if it is not at lower altitude than the surface plus the margin on entry.
	AuPoint RaiseToSurface(const AuPoint& p, double margin) const;

	virtual bool SightTarget(const AuPoint& eyeLoc, AuPoint& sightLoc) const;

	virtual void SurfacePoints(AuPointNArray& points, double stepSize) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSolidBox : public AuSolidShape
{
	double								m_RadiusX;
	double								m_RadiusY;
	double								m_RadiusZ;

public:
	inline AuSolidBox(const AuMatrix4x4& m, double radiusX, double radiusY, double radiusZ) : AuSolidShape(m)
	{
		m_RadiusX = radiusX;
		m_RadiusY = radiusY;
		m_RadiusZ = radiusZ;
	}

	inline AuSolidBox(const AuSolidBox& c) : AuSolidShape(c)
	{
		m_RadiusX = c.m_RadiusX;
		m_RadiusY = c.m_RadiusY;
		m_RadiusZ = c.m_RadiusZ;
	}

public:
	virtual AuSolidShape* Clone() const;

	virtual void Extents(AuPoint& min, AuPoint& max) const;

	virtual void Faces(AuScFaceList& faces, double stepSize) const;

	virtual bool IsInside(const AuPoint& p) const;

	virtual bool SightTarget(const AuPoint& eyeLoc, AuPoint& sightLoc) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSolid
{
	AuSolidShapes						m_ShapesAdd;
	AuSolidShapes						m_ShapesSubtract;

public:	// Construction
	AuSolid();
	virtual ~AuSolid();
	AuSolid(const AuSolid& c);

public:	// Operators

public:	// Methods
	void Add(const AuSolidShape& shape);

	// Returns true if 'min' and 'max' are set.
	bool Extents(AuPoint& min, AuPoint& max) const;

	void Faces(AuScFaceList& faces, double stepSize) const;

	inline bool IsEmpty() const
	{
		return m_ShapesAdd.empty();
	}

	// Tests for intersection by a segment from 'eyeLoc' to 'sightLoc'.
	// Returns false if not hit.
	// Returns true if hit and sets 'sightLoc' to the closest point on the sight segment to 'eyeLoc' that hits this solid.
	bool SightTarget(const AuPoint& eyeLoc, AuPoint& sightLoc) const;

	void Subtract(const AuSolidShape& shape);

	void SurfacePoints(AuPointNArray& points, double stepSize) const;

	void TopologyAtZ(AuBoundaryComplex& topo, double zHeight, double stepSize) const;

	void Transform(const AuMatrix4x4& m);
};

#endif // AuSolid_H
