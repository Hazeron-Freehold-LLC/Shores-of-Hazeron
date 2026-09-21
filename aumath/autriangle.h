//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: autriangle.h,v 1.20 2022/02/19 18:38:09 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuTriangle_H
#define AuTriangle_H

#include "auline.h"
#include "aumatrix4x4.h"
#include "auplane.h"
#include "aupoint.h"

class AuTriangleList;

class AUMATH_DLL AuTri
{
public:
	AuPoint								m_V0;					// Vertex location.
	AuPoint								m_V1;					// Vertex location.
	AuPoint								m_V2;					// Vertex location.

public:
	inline AuTri(const AuPoint& v0, const AuPoint& v1, const AuPoint& v2)
	{
		m_V0 = v0;
		m_V1 = v1;
		m_V2 = v2;
	}

	inline AuTri()
	{
	}

	inline AuTri(const AuTri& c)
	{
		m_V0 = c.m_V0;
		m_V1 = c.m_V1;
		m_V2 = c.m_V2;
	}

	// Calculates the intersection of the Z axis through 'loc' with this triangle.
	// Returns true if an intersection is found.
	// 'intersect' is set to the result if an intersection is found.
	inline bool IntersectZ(const AuPoint& loc, AuPoint& intersect) const
	{
		// Test for 2D intersect with triangle.
		if(!loc.InTriangle(m_V0, m_V1, m_V2))
			return false;

		// Find the exact intersection with the plane of the triangle.
		AuPlane fp(m_V0, m_V2 - m_V0, m_V1 - m_V0);				// Infinite plane through the triangle. Clockwise orientation.
		AuLine fl(loc.x,loc.y,loc.z, 0,0,1);					// Infinite line along the Z axis.
		if(!fp.LineIntersection(fl, intersect))
			return false;										// The line does not intersect the plane.

		return true;
	}

	// Calculates the intersection of the Z axis through 'loc' with this triangle.
	// Returns true if an intersection is found.
	// 'intersect' is set to the result if an intersection is found.
	// 'normal' is set to the normal to the triangle that is found, considering the triangle to be clockwise.
	inline bool IntersectZ(const AuPoint& loc, AuPoint& intersect, AuPoint& normal) const
	{
		// Test for 2D intersect with triangle.
		if(!loc.InTriangle(m_V0, m_V1, m_V2))
			return false;

		// Find the exact intersection with the plane of the triangle.
		AuPlane fp(m_V0, m_V2 - m_V0, m_V1 - m_V0);				// Infinite plane through the triangle. Clockwise orientation.
		AuLine fl(loc.x,loc.y,loc.z, 0,0,1);					// Infinite line along the Z axis.
		if(!fp.LineIntersection(fl, intersect))
			return false;										// The line does not intersect the plane.

		// Caller wants to know the normal to the plane.
		normal = fp.N;
		return true;
	}

	inline bool operator==(const AuTri& c) const
	{
		if(m_V0 != c.m_V0)
			return false;
		if(m_V1 != c.m_V1)
			return false;
		if(m_V2 != c.m_V2)
			return false;
		return true;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUMATH_DLL AuTriList : public QVector<AuTri>
{
public:
	// Calculates the intersection of the Z axis through 'loc' with this list of triangles.
	// Returns true if an intersection is found. Returns the first intersection found, not sorted.
	// 'intersect' is set to the result if an intersection is found.
	inline bool IntersectZ(const AuPoint& loc, AuPoint& intersect) const
	{
		for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
			if((*ti).IntersectZ(loc, intersect))
				return true;
		return false;
	}

	// Calculates the intersection of the Z axis through 'loc' with this list of triangles.
	// Returns true if an intersection is found. Returns the first intersection found, not sorted.
	// 'intersect' is set to the result if an intersection is found.
	// 'normal' is set to the normal to the triangle that is found, considering the triangle to be clockwise.
	inline bool IntersectZ(const AuPoint& loc, AuPoint& intersect, AuPoint& normal) const
	{
		for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
			if((*ti).IntersectZ(loc, intersect, normal))
				return true;
		return false;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUMATH_DLL AuTriangle  
{
public:
	AuPoint		m_fV[3];		// Vertex locations. (lower-left, upper-left, lower-right)
	AuPoint		m_vN;			// Surface lighting normal.
	bool		m_bEdge[3];		// Edge visibility.

public:
	inline AuTriangle()
	{
		m_fV[0] = AuPoint(0,0);
		m_fV[1] = AuPoint(0,0);
		m_fV[2] = AuPoint(0,0);

		m_vN = AuPoint(0,0);

		m_bEdge[0] = false;
		m_bEdge[1] = false;
		m_bEdge[2] = false;
	}

	inline AuTriangle(const AuPoint &fV0, const AuPoint &fV1, const AuPoint &fV2)
	{
		m_fV[0] = fV0;
		m_fV[1] = fV1;
		m_fV[2] = fV2;

		CalculateNormal();

		m_bEdge[0] = false;
		m_bEdge[1] = false;
		m_bEdge[2] = false;
	}

	inline AuTriangle(const AuPoint &fV0, const AuPoint &fV1, const AuPoint &fV2,
			bool bE0, bool bE1, bool bE2)
	{
		m_fV[0] = fV0;
		m_fV[1] = fV1;
		m_fV[2] = fV2;

		CalculateNormal();

		m_bEdge[0] = bE0;
		m_bEdge[1] = bE1;
		m_bEdge[2] = bE2;
	}

	inline void CalculateNormal()
	{
		AuPoint vx(m_fV[2] - m_fV[0]);
		vx.Unit();
		AuPoint vy(m_fV[1] - m_fV[0]);
		vy.Unit();
		m_vN = vx.Cross(vy);
		m_vN.Unit();
	}

	// Fuzzy compare vertices independent of order.
	inline bool CompareVertices(const AuTriangle &cTri) const
	{
		if (cTri.m_fV[0].FuzzyEquals(m_fV[0]))
		{
			if (cTri.m_fV[1].FuzzyEquals(m_fV[1]) && cTri.m_fV[2].FuzzyEquals(m_fV[2]))
				return true;
			else if (cTri.m_fV[1].FuzzyEquals(m_fV[2]) && cTri.m_fV[2].FuzzyEquals(m_fV[1]))
				return true;
		}
		else if (cTri.m_fV[0].FuzzyEquals(m_fV[1]))
		{
			if (cTri.m_fV[1].FuzzyEquals(m_fV[0]) && cTri.m_fV[2].FuzzyEquals(m_fV[2]))
				return true;
			else if (cTri.m_fV[1].FuzzyEquals(m_fV[2]) && cTri.m_fV[2].FuzzyEquals(m_fV[0]))
				return true;
		}
		else if (cTri.m_fV[0].FuzzyEquals(m_fV[2]))
		{
			if (cTri.m_fV[1].FuzzyEquals(m_fV[0]) && cTri.m_fV[2].FuzzyEquals(m_fV[1]))
				return true;
			else if (cTri.m_fV[1].FuzzyEquals(m_fV[1]) && cTri.m_fV[2].FuzzyEquals(m_fV[0]))
				return true;
		}
		else if (cTri.m_fV[1].FuzzyEquals(m_fV[0]))
		{
			if (cTri.m_fV[0].FuzzyEquals(m_fV[1]) && cTri.m_fV[2].FuzzyEquals(m_fV[2]))
				return true;
			else if (cTri.m_fV[0].FuzzyEquals(m_fV[2]) && cTri.m_fV[2].FuzzyEquals(m_fV[1]))
				return true;
		}
		else if (cTri.m_fV[1].FuzzyEquals(m_fV[1]))
		{
			if (cTri.m_fV[0].FuzzyEquals(m_fV[0]) && cTri.m_fV[2].FuzzyEquals(m_fV[2]))
				return true;
			else if (cTri.m_fV[0].FuzzyEquals(m_fV[2]) && cTri.m_fV[2].FuzzyEquals(m_fV[0]))
				return true;
		}
		else if (cTri.m_fV[1].FuzzyEquals(m_fV[2]))
		{
			if (cTri.m_fV[0].FuzzyEquals(m_fV[0]) && cTri.m_fV[2].FuzzyEquals(m_fV[1]))
				return true;
			else if (cTri.m_fV[0].FuzzyEquals(m_fV[1]) && cTri.m_fV[2].FuzzyEquals(m_fV[0]))
				return true;
		}
		else if (cTri.m_fV[2].FuzzyEquals(m_fV[0]))
		{
			if (cTri.m_fV[0].FuzzyEquals(m_fV[1]) && cTri.m_fV[1].FuzzyEquals(m_fV[2]))
				return true;
			else if (cTri.m_fV[0].FuzzyEquals(m_fV[2]) && cTri.m_fV[1].FuzzyEquals(m_fV[1]))
				return true;
		}
		else if (cTri.m_fV[2].FuzzyEquals(m_fV[1]))
		{
			if (cTri.m_fV[0].FuzzyEquals(m_fV[0]) && cTri.m_fV[1].FuzzyEquals(m_fV[2]))
				return true;
			else if (cTri.m_fV[0].FuzzyEquals(m_fV[2]) && cTri.m_fV[1].FuzzyEquals(m_fV[0]))
				return true;
		}
		else if (cTri.m_fV[2].FuzzyEquals(m_fV[2]))
		{
			if (cTri.m_fV[0].FuzzyEquals(m_fV[0]) && cTri.m_fV[1].FuzzyEquals(m_fV[1]))
				return true;
			else if (cTri.m_fV[0].FuzzyEquals(m_fV[1]) && cTri.m_fV[1].FuzzyEquals(m_fV[0]))
				return true;
		}
		return false;
	}

	// Adds two or three triangles to cResult after slicing this triangle
	// against the segment from fSeg0 to fSeg1.
	// Nothing is added to cResult if this triangle is not cut by the segment.
	void Slice_2D(const AuPoint &fSeg0, const AuPoint &fSeg1,
			AuTriangleList &cResult) const;

	// Tests a segment for intersection with this triangle.
	// 'fIntersect' is set to the intersection of the line through fSeg1~fSeg2 to the plane of this triangle.
	// 0 is returned if there is no intersection at all. i.e. The line is parallel to the plane of the triangle or the triangle is
	//   collapsed.
	// 1 is returned if the intersection point is within the segment.
	// 2 is returned if the intersection point is within this triangle.
	// 3 is returned if the intersection point is within this triangle and within the segment
	inline int SegmentIntersection(AuPoint &fIntersect, const AuPoint &fSeg1, const AuPoint &fSeg2) const
	{
		AuPlane fp(m_fV[0], m_vN);	// Plane of this quad.
		AuLine fl(fSeg1,fSeg2);		// Infinite line through fSeg1 and fSeg2.
		if (!fp.LineIntersection(fl, fIntersect))
			return 0;					// The line does not intersect the plane.

		// Test the intersection point to see if it is within the line segment.
		int iret = 0;
		if (fIntersect.Between(fSeg1, fSeg2))
			iret |= 1;

		// Test the intersection point to see if it is within the triangle.
		if (m_fV[0].z == m_fV[1].z && m_fV[1].z == m_fV[2].z)
		{
			// 2D triangle test is sufficient.
			if (fIntersect.InTriangle(m_fV[0], m_fV[1], m_fV[2]))
				iret |= 2;
		}
		else
		{
			// 3D triangle test is required.
			if (fIntersect.InTriangle_3D(m_fV[0], m_fV[1], m_fV[2]))
				iret |= 2;
		}
		
		return iret;
	}
	
	//  Summary:
	//      Query to determine center point of this triangle.
	//  Return Value:
	//      Returns the center point of this triangle.
	inline AuPoint Center() const
	{
		// From http://mathforum.org/library/drmath/view/54899.html
		return (m_fV[0] + m_fV[1] + m_fV[2]) / 3.;
		/*
		AuPoint f0 = (m_fV[0] + m_fV[1])/2.0;
		AuPoint f1 = (m_fV[0] + m_fV[2])/2.0;
		AuPoint fcen;
		if (!fcen.SegmentIntersection(f0, m_fV[2], f1, m_fV[1], true, true))
			return m_fV[0];	// This triangle must be smashed into a line.
		else
			return fcen;
			*/
	}

	inline void Transform(const AuMatrix4x4 &tModel)
	{
		m_fV[0] = tModel.Project(m_fV[0]);
		m_fV[1] = tModel.Project(m_fV[1]);
		m_fV[2] = tModel.Project(m_fV[2]);

		m_vN = tModel.ProjectVector(m_vN);
		m_vN.Unit();
	}

	inline bool operator==(const AuTriangle& cTri) const
	{
		return CompareVertices(cTri);
	}
};

//+---------------------------------------------------------------------------+

class AUMATH_DLL AuTriangleList : public QVector<AuTriangle>
{
public:
	inline void Transform(const AuMatrix4x4 &tModel)
	{
		for(iterator it = begin(); it != end(); it++)
			(*it).Transform(tModel);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Triangle where all three vertices have texture coordinates.

class AUMATH_DLL AuTriTex
{
public:
	AuPointTex							m_V0;
	AuPointTex							m_V1;
	AuPointTex							m_V2;

public:
	inline AuTriTex(const AuPointTex& v0, const AuPointTex& v1, const AuPointTex& v2) : m_V0(v0), m_V1(v1), m_V2(v2)
	{
	}

	inline AuTriTex()
	{
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Triangle where all three vertices have texture coordinates and a single normal is shared by all three of them.

class AUMATH_DLL AuTriNTex
{
public:
	AuPoint								m_N;
	AuPointTex							m_V0;
	AuPointTex							m_V1;
	AuPointTex							m_V2;

public:
	inline AuTriNTex(const AuPointTex& v0, const AuPointTex& v1, const AuPointTex& v2) : m_V0(v0), m_V1(v1), m_V2(v2)
	{
		CalculateNormal();
	}

	inline AuTriNTex(const AuPoint& v0, const QPointF& t0, const AuPoint& v1, const QPointF& t1, const AuPoint& v2,
			const QPointF& t2) : m_V0(v0, t0), m_V1(v1, t1), m_V2(v2, t2)
	{
		CalculateNormal();
	}

	// Calculates m_N assuming clockwise triangle direction.
	inline void CalculateNormal()
	{
		AuPoint vx(m_V2 - m_V0);
		vx.Unit();
		AuPoint vy(m_V1 - m_V0);
		vy.Unit();
		m_N = vx.Cross(vy);
		m_N.Unit();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Triangle where all three vertices have texture coordinates and separate normals.

class AUMATH_DLL AuTriTexN
{
public:
	AuPointTexN							m_V0;
	AuPointTexN							m_V1;
	AuPointTexN							m_V2;

public:
	inline AuTriTexN(const AuPointTexN& v0, const AuPointTexN& v1, const AuPointTexN& v2)
	{
		m_V0 = v0;
		m_V1 = v1;
		m_V2 = v2;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Triangle where all three vertices have texture coordinates, separate normals and shininess.

class AUMATH_DLL AuTriTexNS
{
public:
	AuPointTexNS						m_V0;
	AuPointTexNS						m_V1;
	AuPointTexNS						m_V2;

public:
	inline AuTriTexNS(const AuPointTexNS& v0, const AuPointTexNS& v1, const AuPointTexNS& v2)
	{
		m_V0 = v0;
		m_V1 = v1;
		m_V2 = v2;
	}

	inline AuTriTexNS()
	{
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_V0;
		in >> m_V1;
		in >> m_V2;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_V0;
		out << m_V1;
		out << m_V2;
	}
};

AUMATH_DLL inline QDataStream& operator<<(QDataStream& out, const AuTriTexNS& t)
{
	t >> out;
	return out;
}

AUMATH_DLL inline QDataStream& operator>>(QDataStream& in, AuTriTexNS& t)
{
	t << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Triangle with a texture id, where all three vertices have texture coordinates, separate normals and shininess.

class AUMATH_DLL AuTriTexNST
{
public:
	int									m_Texture;
	AuPointTexNS						m_V0;
	AuPointTexNS						m_V1;
	AuPointTexNS						m_V2;

public:
	inline AuTriTexNST(int texture, const AuPointTexNS& v0, const AuPointTexNS& v1, const AuPointTexNS& v2)
	{
		m_Texture = texture;
		m_V0 = v0;
		m_V1 = v1;
		m_V2 = v2;
	}

	inline void Transform(const AuMatrix4x4& t)
	{
		m_V0.Transform(t);
		m_V1.Transform(t);
		m_V2.Transform(t);
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Triangle for rendering creature bodies.

class AUMATH_DLL AuTriBody
{
public:
	AuPointBody							m_V0;
	AuPointBody							m_V1;
	AuPointBody							m_V2;

public:
	inline AuTriBody(const AuPointBody& v0, const AuPointBody& v1, const AuPointBody& v2)
	{
		m_V0 = v0;
		m_V1 = v1;
		m_V2 = v2;
	}

	inline AuTriBody()
	{
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_V0;
		in >> m_V1;
		in >> m_V2;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_V0;
		out << m_V1;
		out << m_V2;
	}
};

AUMATH_DLL inline QDataStream& operator<<(QDataStream& out, const AuTriBody& t)
{
	t >> out;
	return out;
}

AUMATH_DLL inline QDataStream& operator>>(QDataStream& in, AuTriBody& t)
{
	t << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Triangle for rendering spacecraft.

class AUMATH_DLL AuTriSpacecraft
{
public:
	AuPointSpacecraft					m_V0;
	AuPointSpacecraft					m_V1;
	AuPointSpacecraft					m_V2;

public:
	inline AuTriSpacecraft(const AuPointSpacecraft& v0, const AuPointSpacecraft& v1, const AuPointSpacecraft& v2)
	{
		m_V0 = v0;
		m_V1 = v1;
		m_V2 = v2;
	}

	inline AuTriSpacecraft()
	{
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_V0;
		in >> m_V1;
		in >> m_V2;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_V0;
		out << m_V1;
		out << m_V2;
	}
};

AUMATH_DLL inline QDataStream& operator<<(QDataStream& out, const AuTriSpacecraft& t)
{
	t >> out;
	return out;
}

AUMATH_DLL inline QDataStream& operator>>(QDataStream& in, AuTriSpacecraft& t)
{
	t << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuTriangle_H
