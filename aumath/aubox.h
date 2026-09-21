//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubox.h,v 1.12 2022/02/19 18:37:57 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBox_H
#define AuBox_H

#include "aumatrix4x4.h"
#include "aupoint.h"
#include "auquad.h"

class AuQuadArray;
class AuScFaceList;

class AUMATH_DLL AuBox  
{
public:
	AuPoint		m_fCorners[8];	// bll (min,min,min), bul, bur, blr, tll, tul, tur (max,max,max), tlr (all viewed from the top)

public:
	inline AuBox()
	{
		for(int ic=0; ic<8; ic++)
			m_fCorners[ic] = AuPoint();
	}

	inline AuBox(const AuPoint& min, const AuPoint& max)
	{
		SetFromMinMax(min, max);
	}

	inline AuBox(const AuBox& c)
	{
		for(int ic=0; ic<8; ic++)
			m_fCorners[ic] = c.m_fCorners[ic];
	}

public:	// Operators
	inline const AuBox& operator=(const AuBox& c)
	{
		for(int ic=0; ic<8; ic++)
			m_fCorners[ic] = c.m_fCorners[ic];
		return *this;
	}

	inline bool operator==(const AuBox& c) const
	{
		const AuPoint* pf = m_fCorners;
		const AuPoint* pfc = c.m_fCorners;
		for(int ic=0; ic<8; ic++, pf++, pfc++)
			if(*pf != *pfc)
				return false;
		return true;
	}

public:	// Methods
	// Returns true if b intersects this.
	inline bool DoesIntersect(const AuBox& b) const
	{
		// Perform coarse test on mins/maxes.
		AuPoint fmin1, fmax1;
		GetMinMax(fmin1, fmax1);
		AuPoint fmin2, fmax2;
		b.GetMinMax(fmin2, fmax2);
		if(fmax2.x < fmin1.x || fmin2.x > fmax1.x)
			return false;
		else if(fmax2.y < fmin1.y || fmin2.y > fmax1.y)
			return false;
		else if(fmax2.z < fmin1.z || fmin2.z > fmax1.z)
			return false;

		// Test for any of b's vertices inside this box.
		AuQuadArray aq;
		GetQuads(aq);
		for(int ic=0; ic<8; ic++)
			if(aq.TestPointInside(b.m_fCorners[ic]))
				return true;	// A vertex of b is inside this box.

		// Test for any of my vertices inside of b.
		AuQuadArray bq;
		b.GetQuads(bq);
		for(int ic=0; ic<8; ic++)
			if(bq.TestPointInside(m_fCorners[ic]))
				return true;	// My vertex is inside b.

		// Test for collisions between any of our surfaces.
		if(aq.TestCollision(bq))
			return true;

		// No intersections detected.
		return false;
	}

	inline void GetMinMax(AuPoint &fMin, AuPoint &fMax) const
	{
		fMin = m_fCorners[0];
		fMax = fMin;
		const AuPoint *pf = m_fCorners + 1;
		for (int ic=1; ic<8; ic++, pf++)
		{
			fMin = pf->Min(fMin);
			fMax = pf->Max(fMax);
		}
	}

	inline void GetQuads(AuQuad aQuads[6]) const
	{
		aQuads[0] = AuQuad(m_fCorners[0], m_fCorners[4], m_fCorners[7], m_fCorners[3]);	// Front
		aQuads[1] = AuQuad(m_fCorners[1], m_fCorners[5], m_fCorners[4], m_fCorners[0]);	// Left
		aQuads[2] = AuQuad(m_fCorners[2], m_fCorners[6], m_fCorners[5], m_fCorners[1]);	// Back
		aQuads[3] = AuQuad(m_fCorners[3], m_fCorners[7], m_fCorners[6], m_fCorners[2]);	// Right
		aQuads[4] = AuQuad(m_fCorners[4], m_fCorners[5], m_fCorners[6], m_fCorners[7]);	// Top
		aQuads[5] = AuQuad(m_fCorners[1], m_fCorners[0], m_fCorners[3], m_fCorners[2]);	// Bottom
	}

	inline void GetQuads(AuQuadArray &aQuads) const
	{
		aQuads.AddQuad(m_fCorners[0], m_fCorners[4], m_fCorners[7], m_fCorners[3]);	// Front
		aQuads.AddQuad(m_fCorners[1], m_fCorners[5], m_fCorners[4], m_fCorners[0]);	// Left
		aQuads.AddQuad(m_fCorners[2], m_fCorners[6], m_fCorners[5], m_fCorners[1]);	// Back
		aQuads.AddQuad(m_fCorners[3], m_fCorners[7], m_fCorners[6], m_fCorners[2]);	// Right
		aQuads.AddQuad(m_fCorners[4], m_fCorners[5], m_fCorners[6], m_fCorners[7]);	// Top
		aQuads.AddQuad(m_fCorners[1], m_fCorners[0], m_fCorners[3], m_fCorners[2]);	// Bottom
	}

	void GetFaces(AuScFaceList& faces) const;

	inline double Height() const
	{
		return m_fCorners[6].z - m_fCorners[0].z;
	}

	inline bool IsPointInside(const AuPoint &floc) const
	{
		// Make quads of the six sides.
		AuQuadArray aq;
		GetQuads(aq);
		return aq.TestPointInside(floc);
	}

	inline bool IsZero() const
	{
		AuPoint fmin, fmax;
		GetMinMax(fmin, fmax);
		if (fmin.IsZero() && fmax.IsZero())
			return true;
		return false;
	}

	inline double Length() const
	{
		return m_fCorners[6].y - m_fCorners[0].y;
	}

	// Returns the distance to the farthest vertex from the origin.
	inline double RoughRadius() const
	{
		double farthest = m_fCorners[0].Length2();	// Squared distance to first corner.

		const AuPoint* pf = m_fCorners + 1;
		for(int ic=1; ic<8; ic++, pf++)
		{
			double dist2 = (*pf).Length2();			// Squared distance to next corner.
			if(dist2 > farthest)
				farthest = dist2;
		}

		return sqrt(farthest);
	}

	// Tests a segment for intersection against each of my side quads.
	// Returns the number of intersections, which could only be 0, 1 or 2.
	// First intersection is stored in 'int0', second in 'int1'. These are not sorted by closeness to either end of the segment.
	inline int SegmentIntersection(const AuPoint& seg0, const AuPoint& seg1, AuPoint& int0, AuPoint& int1) const
	{
		AuQuad aq[6];
		GetQuads(aq);
		AuPoint fN, fInt;
		int intersections = 0;
		for(int qi=0; qi<6; qi++)
			if(!intersections)
			{
				if(aq[qi].TestSegmentIntersection(int0, fN, seg0, seg1))
					intersections = 1;	// First intersection. There could be another.
			}
			else if(aq[qi].TestSegmentIntersection(int1, fN, seg0, seg1))
				return 2;				// Second intersection. There can't be another one.
		return intersections;
	}

	inline void SetFromMinMax(const AuPoint& min, const AuPoint& max)
	{
		m_fCorners[0] = min;
		m_fCorners[1] = AuPoint(min.x, max.y, min.z);
		m_fCorners[2] = AuPoint(max.x, max.y, min.z);
		m_fCorners[3] = AuPoint(max.x, min.y, min.z);
		m_fCorners[4] = AuPoint(min.x, min.y, max.z);;
		m_fCorners[5] = AuPoint(min.x, max.y, max.z);
		m_fCorners[6] = max;
		m_fCorners[7] = AuPoint(max.x, min.y, max.z);
	}

	// Transform the corner locations of this box by a given transform 't'.
	inline void Transform(const AuMatrix4x4 &t)
	{
		AuPoint* pf = m_fCorners;
		for (int ic=0; ic<8; ic++, pf++)
			*pf = t.Project(*pf);
	}

	// Union of extents of two boxes.
	// Puts the result in this box. Returns false if the extents of the boxes do
	// not intersect.
	// 'this' box is modified to be the union of the two boxes.
	// Returns true if the unrotated extents of the two boxes intersect each other.
	// Resulting box is the unrotated extents of the union of the boxes.
	inline bool Union(const AuBox &fb1, const AuBox &fb2)
	{
		AuPoint fmin1, fmax1;
		fb1.GetMinMax(fmin1, fmax1);

		AuPoint fmin2, fmax2;
		fb2.GetMinMax(fmin2, fmax2);

		bool bIntersect = true;		// This will be our return value.
						// FALSE if the extents of the boxes do not intersect.

		if (fmax2.x < fmin1.x || fmin2.x > fmax1.x)
			bIntersect = false;
		else if (fmax2.y < fmin1.y || fmin2.y > fmax1.y)
			bIntersect = false;
		else if (fmax2.z < fmin1.z || fmin2.z > fmax1.z)
			bIntersect = false;

		// Extend the min/max to encompass both.
		fmin1 = fmin1.Min(fmin2);
		fmax1 = fmax1.Max(fmax2);

		// Rebuild this box out of the new min/max
		m_fCorners[0] = fmin1;
		m_fCorners[1] = AuPoint(fmin1.x, fmax1.y, fmin1.z);
		m_fCorners[2] = AuPoint(fmax1.x, fmax1.y, fmin1.z);
		m_fCorners[3] = AuPoint(fmax1.x, fmin1.y, fmin1.z);
		m_fCorners[4] = AuPoint(fmin1.x, fmin1.y, fmax1.z);;
		m_fCorners[5] = AuPoint(fmin1.x, fmax1.y, fmax1.z);
		m_fCorners[6] = AuPoint(fmax1.x, fmax1.y, fmax1.z);
		m_fCorners[7] = AuPoint(fmax1.x, fmin1.y, fmax1.z);

		return bIntersect;
	}

#if !defined(QT_NO_DEBUG)
	inline void Dump(const char* msg) const
	{
		if(msg)
			qDebug("AuBox::Dump - %s", msg);
		else
			qDebug("AuBox::Dump");
		for(int iv=0; iv<8; iv++)
			m_fCorners[iv].Dump(0);
	}
#endif

	inline double Width() const
	{
		return m_fCorners[6].x - m_fCorners[0].x;
	}
};

class AUMATH_DLL AuBoxArray : public QVector<AuBox>
{
};

#endif // AuBox_H
