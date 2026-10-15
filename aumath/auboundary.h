//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auboundary.h,v 1.3 2022/04/08 00:17:20 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBoundary_H
#define AuBoundary_H

#include "auline.h"
#include "aunoise.h"
#include "aupoint.h"

// Boundary point is used when defining a 2D boundary. The Z rotation indicates the outside direction of the boundary; it is the Z
// rotation of the vector that points outside the boundary, such as the surface normal. This eliminates the need for clockwise or
// counterclockwise boundary directions.
// e.g. if the surface normal to the boundary is (1,0), the Z rotation is 0, which is AuPoint(1,0,0).ZRotation().

class AuBoundaryComplex;
class AuScFaceList;

class AUMATH_DLL AuBoundaryPoint : public AuPoint
{
public:
	double								m_RotZ;					// Rotation around Z axis.

public:	// Constructors
	inline AuBoundaryPoint(const AuPoint& loc, double rotZ) : AuPoint(loc)
	{
		m_RotZ = rotZ;
	}

	inline AuBoundaryPoint(const AuPoint& loc, const AuPoint& outsideVec) : AuPoint(loc)
	{
		m_RotZ = outsideVec.ZRotation();
	}

	inline AuBoundaryPoint()
	{
		m_RotZ = 0;
	}

	inline AuBoundaryPoint(const AuBoundaryPoint& c) : AuPoint(c)
	{
		m_RotZ = c.m_RotZ;
	}

public:	// Operators
	inline const AuBoundaryPoint& operator=(const AuBoundaryPoint& p)
	{
		AuPoint::operator=(p);
		m_RotZ = p.m_RotZ;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		AuPoint::operator>>(out);
		out << m_RotZ;
	}

	inline void operator<<(QDataStream& in)
	{
		AuPoint::operator<<(in);
		in >> m_RotZ;
	}

	inline bool operator==(const AuBoundaryPoint& p) const
	{
		if(!AuPoint::operator==(p))
			return false;
		if(m_RotZ != p.m_RotZ)
			return false;
		return true;
	}

	inline bool operator!=(const AuBoundaryPoint& p) const
	{
		return !operator==(p);
	}

public:	// Methods
	inline AuBoundaryPoint LinearInterpolate(const AuBoundaryPoint& o, double frac) const
	{
		// Interpolate the angle in the shortest sweep direction to orot;
		double orot = o.Rotation();
		double rot = AuNoise::Linear_Interpolate(m_RotZ, orot, frac);
		if(fabs(orot - m_RotZ) > PI)
			rot -= PI;		// We calculated the long way around the circle. Go the other way.

		// Normalize the rotation angle to -PI < rot < PI.
		while(rot < -PI)
			rot += PI*2;
		while(rot > PI)
			rot -= PI*2;

		return AuBoundaryPoint(AuPoint::LinearInterpolate(o, frac), rot);
	}

	// Returns a 2D unit vector in the XY plane pointing to the outside of this boundary point.
	inline AuPoint OutsideVector() const
	{
		return AuPoint(cos(Rotation()), sin(Rotation()));
	}

	inline void ReverseRotation()
	{
		if(m_RotZ > 0)
			m_RotZ -= PI;
		else
			m_RotZ += PI;
	}

	inline double Rotation() const
	{
		return m_RotZ;
	}

	inline void SetCoord(double cx, double cy, double cz)
	{
		x = cx;
		y = cy;
		z = cz;
	}

	inline void SetCoord(const AuPoint& c)
	{
		SetCoord(c.x, c.y, c.z);
	}

	inline void SetRotation(double rotZ)
	{
		m_RotZ = UnwindAngle(rotZ);	// Normalize rotation to the range of -Pi to Pi.
	}
};

AUMATH_DLL inline QDataStream& operator<<(QDataStream& out, const AuBoundaryPoint& o)
{
	o >> out;
	return out;
}

AUMATH_DLL inline QDataStream& operator>>(QDataStream& in, AuBoundaryPoint& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// A 2D boundary. The Z locations of the boundary points are preserved as much as possible but otherwise they are ignored.

class AUMATH_DLL AuBoundary : public QVector<AuBoundaryPoint>
{
public:	// Enumerated Types
	enum OverlapStatus
	{
		NoOverlap,
		Overlapped,
		ContainsChild,
		ContainedByChild,
	};

public:	// Methods
	// Adds 'b' to this boundary if they overlap.
	// This boundary will be the outer boundary around the overlapping pair. Interior voids are lost.
	// Returns 0 if they do not overlap at all.
	// Returns 1 if this boundary is changed as a result of the overlap, including 'b' completely absorbing me.
	// Returns 2 if this boundary is not changed as a result of completely absorbing 'b'.
	int Add(const AuBoundary& b);

	// Adds 'p' only if it's location is not the same as the last boundary point in the array.
	// FuzzyEquals_2D test. Returns true if added.
	inline bool AddIfDifferentFromLast(const AuBoundaryPoint& p)
	{
		if(empty() || !last().FuzzyEquals_2D(p))
		{
			append(p);
			return true;
		}
		return false;
	}

	// Finds closest location in this boundary to 'loc', using 2D checks.
	const_iterator Closest(const AuPoint& loc) const;

	// Finds the closest point on this boundary to the closest point on 'b'.
	// Returns my iterator and sets 'bi' to the iterator in 'b'.
	// All checks are 2D.
	const_iterator Closest(const AuBoundary& b, const_iterator& bi) const;

	// Treats this array as a series of segments. Finds the closest point on this array of segments to 'p'.
	// Returns the iterator of the boundary location at the first point of the segment.
	// cend()-1 is returned for the final wrap around segment back to cbegin().
	// Returns cend() if this boundary has less than 3 points.
	const_iterator ClosestSegment(const AuPoint& p) const;

	// Returns true if this boundary contains a location at 'p'. FuzzyEquals_2D test ignores Z component.
	inline bool Contains(const AuPoint &p) const
	{
		for(const_iterator bi = cbegin(); bi != cend(); bi++)
			if((*bi).FuzzyEquals_2D(p))
				return true;
		return false;
	}

	// Returns the squared distance between the closest one of my vertices to one of the vertices of 'b'.
	// 2D distance calculations are used.
	double Distance2_2D(const AuBoundary& b) const;

	// Returns the squared distance between the closest one of my vertices to 'p'.
	// 2D distance calculations are used.
	double Distance2_2D(const AuBoundaryPoint& p) const;

	// Returns false if this boundary is empty and it has no extents.
	inline bool Extents(AuPoint &fMin, AuPoint &fMax) const
	{
		// if no points then just return 0's
		if(empty())
		{
			fMin = AuPoint(0.0, 0.0, 0.0);
			fMax = AuPoint(0.0, 0.0, 0.0);
			return false;
		}

		// set initial min and max to first element and then loop for
		// rest of elements
		const_iterator bi = cbegin();
		fMin = *bi;
		fMax = fMin;
		while(bi != cend())
		{
			fMin = fMin.Min(*bi);
			fMax = fMax.Max(*bi);
			bi++;
		}
		return true;
	}

	void FacesBottom(AuScFaceList& faces) const;

	// Interconnects this boundary with 'above', which is above this boundary in Z.
	void FacesInterconnect(AuScFaceList& faces, const AuBoundary& above) const;

	void FacesTop(AuScFaceList& faces) const;

	// Returns index of 'p' using FuzzyEquals_2D check.
	// Returns -1 if 'p' is not found.
	inline int Find(const AuPoint& p) const
	{
		int index=0;
		for(const_iterator bi = cbegin(); bi != cend(); bi++, index++)
			if((*bi).FuzzyEquals_2D(p))
				return index;
		return -1;
	}

	// Returns the first iterator in this boundary that points to a vertex that is not in 'b' and is not at an intersection.
	inline const_iterator FirstNotInBoundary(const AuBoundary& b, const AuPointArray& intersections) const
	{
		for(const_iterator bi = cbegin(); bi != cend(); bi++)
		{
			if(intersections.DoesContain_2D(*bi))
				continue;
			if(!b.PointInBoundary(*bi))
				return bi;
		}
		return cend();
	}

	// Builds this boundary using 'pa'. Current contents of this boundary are lost.
	void FromPointArray(const AuPointArray& pa);

	// Finds the intersections with 'line' and puts the iterators to the closest boundary points in 'intersections'.
	void Intersect(const AuLine& line, QList<const_iterator>& intersections) const;

	// Return overlap condition between 'this' boundary and another boundary.
	// This is a 2d routine, the z coordinates are ignored.
	// 'this' is assumed to be the parent boundary.
	// Return values are
	//   NoOverlap - boundaries do not overlap
	//   Overlapped - boundaries overlap
	//   ContainsChild - 'this' surrounds the child boundary
	//   ContainedByChild - 'this' is surrounded by child
	OverlapStatus Overlap(const AuBoundary& b) const;

	// Returns true if 'loc' is inside the 2D boundary formed by this array of points.
	// Returns false if 'loc' is on the 2D boundary.
	// Returns false if 'loc' is not inside the 2D boundary.
	bool PointInBoundary(const AuPoint& loc) const;

	// Returns true if 'loc' is on the 2D boundary formed by this array of points.
	// Returns false if 'loc' is not on the 2D boundary formed by this array of points.
	bool PointOnBoundary(const AuPoint& loc) const;

	void RemoveBoundaryDuplicates();

	// Recalculates the rotation angles at all vertexes. Inside/outside is determined by the current rotation at vertex index 1.
	void Smooth();

	// Subtracts 'b' from this boundary if they overlap.
	// This boundary will be modified, or emptied, where there is overlap. Fragments that get cut off will be added to 'pieces'.
	// This boundary will be emptied if 'b' completely contains it.
	void Subtract(const AuBoundary& b, AuBoundaryComplex& pieces);

	void TriangulateBoundary(AuPointArray& pTri, QVector<bool>& bTedge) const;

	// 'bnd' is the boundary we are constructing.
	// 'mi' is the place to start walking on this boundary.
	// 'mdir' is the direction to walk this boundary, +1 forward or -1 backward.
	// 'ob' is the other boundary.
	// 'oi' is an output parameter, set to the place on 'ob' to continue walking, if the return value is non-zero.
	// Returns 0 to remain on the current boundary.
	// Returns 1 to walk forward or -1 to walk backward on the other boundary, starting at 'oi'.
	// Returns 2 when the boundary 'bnd' is complete.
	// Returns 3 when it loops back on itself without completing the boundary.
	int WalkAdd(AuBoundary& bnd, const_iterator mi, int mdir, const AuBoundary& ob, const_iterator& oi,
			const AuPointArray& intersections) const;

	// 'bnd' is the boundary we are constructing.
	// 'reverseEm' is true to reverse the rotation on the points that are added to 'bnd', as when walking a hole.
	// 'mi' is the place to start walking on this boundary.
	// 'mdir' is the direction to walk this boundary, +1 forward or -1 backward.
	// 'ob' is the other boundary.
	// 'oi' is an output parameter, set to the place on 'ob' to continue walking, if the return value is non-zero.
	// 'pieces' are the pieces of this boundary accumulated so far in the subtraction.
	// Returns 0 to remain on the current boundary.
	// Returns 1 to walk forward or -1 to walk backward on the other boundary, starting at 'oi'.
	// Returns 2 when the boundary 'bnd' is complete.
	// Returns 3 when it loops back on itself without completing the boundary.
	int WalkSubtract(AuBoundary& bnd, bool reverseEm, const_iterator mi, int mdir, const AuBoundary& ob, const_iterator& oi,
			const AuPointArray& intersections, const AuBoundaryComplex& pieces) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// A complex 2D boundary consists of one or more boundaries.

class AUMATH_DLL AuBoundaryComplex : public QList<AuBoundary>
{
public:	// Methods
	// Adds 'b' to this boundary complex. Joins overlapping boundaries.
	void Add(const AuBoundary& b);

	// Creates a boundary of 'b' and adds it to this boundary complex. Joins overlapping boundaries.
	void Add(const AuPointArray& b);

	// Adds 'bc' to this boundary complex. Joins overlapping boundaries.
	void Add(const AuBoundaryComplex& bc);

	// Finds my boundary that is closest to 'b', using 2D checks.
	iterator Closest(const AuBoundary& b);
	const_iterator Closest(const AuBoundary& b) const;

	// Returns true if any of my boundaries contains a boundary point 'p'.
	bool Contains(const AuBoundaryPoint& p) const;

	void FacesBottom(AuScFaceList& faces) const;

	// For debugging, returns faces to show my boundary lines.
	void FacesAsLines(AuScFaceList& faces) const;

	// Interconnects this complex of boundaries with 'above', which is above this complex in Z.
	void FacesInterconnect(AuScFaceList& faces, const AuBoundaryComplex& above, double stepSize) const;

	void FacesTop(AuScFaceList& faces) const;

	// Finds one of my boundaries that overlaps 'b' in 2D, including containment.
	iterator FindOverlap(const AuBoundary& b);
	const_iterator FindOverlap(const AuBoundary& b) const;

	// Returns the number of my boundaries that are overlapped by 'b' in 2D, including containment.
	int OverlapCount(const AuBoundary& b) const;

	// Lists all of my boundaries that overlap 'b' in 2D, including containment.
	void Overlaps(const AuBoundary& b, AuBoundaryComplex& overlaps) const;

	// Subtracts 'b' from this boundary complex. Cuts overlapping boundaries.
	void Subtract(const AuBoundary& b);

	// Subtracts 'bc' from this boundary complex. Cuts overlapping boundaries.
	void Subtract(const AuBoundaryComplex& bc);

	// Takes all of my boundaries that overlap 'b' in 2D, including containment.
	void TakeOverlaps(const AuBoundary& b, AuBoundaryComplex& overlaps);
};

#endif // AuBoundary_H
