//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausegment.h,v 1.26 2022/02/19 18:38:06 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSegment_H
#define AuSegment_H

#include <qcolor.h>
#include "aumatrix4x4.h"
#include "aupoint.h"

class AuSegmentArray;

class AUMATH_DLL AuSegment
{
public:
	AuPoint		m_fLoc1;
	AuPoint		m_fLoc2;

public:	// Constructors/Destructors
	inline AuSegment()
	{
	}

	inline AuSegment(const AuPoint &fLoc1, const AuPoint &fLoc2)
	{
		m_fLoc1 = fLoc1;
		m_fLoc2 = fLoc2;
	}

public:	// Operators
	inline bool operator==(const AuSegment& rhs) const
	{
		if(m_fLoc1 != rhs.m_fLoc1)
			return false;
		if(m_fLoc2 != rhs.m_fLoc2)
			return false;
		return true;
	}

	inline bool operator!=(const AuSegment& rhs) const
	{
		return !operator==(rhs);
	}

	inline bool operator<(const AuSegment& c) const
	{
		if(m_fLoc1 < c.m_fLoc1)
			return true;
		else if(m_fLoc1 > c.m_fLoc1)
			return false;

		if(m_fLoc2 < c.m_fLoc2)
			return true;
		else if(m_fLoc2 > c.m_fLoc2)
			return false;

		return false;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_fLoc1;
		out << m_fLoc2;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_fLoc1;
		in >> m_fLoc2;
	}

public:	// Methods
	inline AuPoint CenterPoint() const
	{
		return (m_fLoc1 + m_fLoc2)/2;
	}

	inline bool IsSegment(const AuSegment& seg) const
	{
		return (m_fLoc1.FuzzyEquals(seg.m_fLoc1) && m_fLoc2.FuzzyEquals(seg.m_fLoc2)) ||
			(m_fLoc1.FuzzyEquals(seg.m_fLoc2) && m_fLoc2.FuzzyEquals(seg.m_fLoc1));
	}

	inline double Length() const
	{
		return m_fLoc1.Distance(m_fLoc2);
	}

	// Returns true if this segment overlaps 's' more than just sharing an end point.
	bool Overlaps(const AuSegment& s) const;

	// Divies this segment into equal length pieces that will not exceed 'maxSegLen'.
	// Result is appended to 'segs'.
	// Existing content of 'segs' is unharmed.
	void SplitEvenly(double maxSegLen, AuSegmentArray& segs) const;

	inline void Transform(const AuMatrix4x4 &tModel)
	{
		m_fLoc1 = tModel.Project(m_fLoc1);
		m_fLoc2 = tModel.Project(m_fLoc2);
	}

	inline const AuPoint& Vertex(int vIndex) const
	{
		if(vIndex)
			return m_fLoc2;
		else
			return m_fLoc1;
	}

	// 'ignoreDist' when positive skips this segment if view distance is beyond.
	// Returns -1 if skipped.
	double ViewDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuPoint& edgeLoc, double ignoreDist) const;
};

inline QDataStream& operator<<(QDataStream& out, const AuSegment& t)
{
	t >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuSegment& t)
{
	t << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUMATH_DLL AuSegmentArray : public QVector<AuSegment>
{
public:	// Operators
	inline void operator>>(QDataStream& out) const
	{
		out << (quint32)size();
		for(const_iterator ti = cbegin(); ti != cend(); ti++)
			out << *ti;
	}

	inline void operator<<(QDataStream& in)
	{
		clear();
		quint32 c;
		in >> c;
		while(c--)
		{
			AuSegment t;
			in >> t;
			append(t);
		}
	}

public:
	// IsSegment is used to test for uniqueness.
	void AddIfUnique(const AuSegment& seg);

	// IsSegment is used to test for uniqueness.
	void AddIfUnique(const AuSegmentArray& segs);

	// Changes the order of these segments to place them end-to-end. Fuzzy test. Will reverse the ends of segments if necessary.
	// Returns true if I end up where I started. This does not guarantee that a continuous boundary is formed.
	bool Align();

	// Break all segments that intersect other segments.
	void BreakIntersections();

	// Returns true if this array contains 'seg'. IsSegment is used, which is a fuzzy test.
	bool Contains(const AuSegment& seg) const;

	void Extents(AuPoint& min, AuPoint& max) const;

	// Returns index of segment; -1 if not found.
	inline int FindSegment(const AuSegment& seg) const
	{
		const AuSegment* pCheckSeg = cbegin();
		for(int lSegIndex=0; lSegIndex<size(); lSegIndex++, pCheckSeg++)
			if(*pCheckSeg == seg)
				return lSegIndex;
		return -1;
	}

	// Returns index of segment; -1 if not found. Fuzzy test using IsSegment, which fuzzy checks ends both ways.
	inline int FindSegmentFuzzy(const AuSegment& seg) const
	{
		const AuSegment* pCheckSeg = cbegin();
		for(int lSegIndex=0; lSegIndex<size(); lSegIndex++, pCheckSeg++)
			if(pCheckSeg->IsSegment(seg))
				return lSegIndex;
		return -1;
	}

	// Combine parallel edge segments that meet end-to-end or overlap.
	void Fuse();

	// Modifies this array to be the result of the intersection with 'segs', i.e. overlapping segments are removed from this array.
	void Intersect(const AuSegmentArray& segs);

	// Makes the outermost possible boundary of 2D points 'b' out of these segments.
	// This array should already be Fused, Broken and ZeroZed.
	void MakeBoundary(AuPoint2dArray& b) const;

	inline quint64 MemoryUsage() const
	{
		return sizeof(AuSegment) * capacity();
	}

	// Removes every segment that does not have another segment abutting each end.
	void RemoveDeadEnds(double tol);

	inline void Transform(const AuMatrix4x4& m)
	{
		for(iterator si = begin(); si != end(); si++)
			(*si).Transform(m);
	}

	// Fills 'locs' with the unique end points of my segments, so there are no duplicates.
	inline void UniqueLocations(AuPointArray& locs) const
	{
		for(const_iterator si = cbegin(); si != cend(); si++)
		{
			locs.AddIfUnique((*si).m_fLoc1);
			locs.AddIfUnique((*si).m_fLoc2);
		}
	}

	inline double ViewDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& edgeIndex, AuPoint& edgeLoc) const
	{
		double bestDist = -1;
		edgeIndex = 0;
		quint32 eidx = 0;
		AuPoint eloc;
		for(const_iterator si = cbegin(); si != cend(); si++, eidx++)
		{
			double edist = (*si).ViewDistance(mvp, pixel, eloc, bestDist);
			if(edist < 0)
				continue;

			if(si == cbegin() || edist < bestDist)
			{
				bestDist = edist;
				edgeIndex = eidx;
				edgeLoc = eloc;
			}
		}
		return bestDist;
	}

	// Sets all z values to 0. Discards segments that get smashed to zero. Not fuzzy.
	inline void ZeroZ()
	{
		iterator si = begin();
		while(si != end())
		{
			(*si).m_fLoc1.z = 0;
			(*si).m_fLoc2.z = 0;
			if((*si).m_fLoc1 == (*si).m_fLoc2)
				si = erase(si);
			else
				si++;
		}
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuSegmentArray& t)
{
	t >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuSegmentArray& t)
{
	t << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUMATH_DLL AuModelSegment
{
public:
	AuPoint		m_fLoc1;
	AuPoint		m_fLoc2;
	AuPoint		m_fNormal1;
	AuPoint		m_fNormal2;
	QColor		m_Color;
	double		m_Opacity;
	double		m_SpecularPower;
	double		m_Shininess;

public:
	inline AuModelSegment()
	{
		m_Color = QColor(255,255,255);
		m_Opacity = 1.0;
		m_SpecularPower = 0.0;
		m_Shininess = 0.0;
	}

	inline AuModelSegment(const AuPoint &fLoc1, const AuPoint &fLoc2,
			const AuPoint &fNormal1, const AuPoint &fNormal2,
			const QColor &color, double opacity, double specpower, double shine)
	{
		m_fLoc1 = fLoc1;
		m_fLoc2 = fLoc2;
		m_fNormal1 = fNormal1;
		m_fNormal2 = fNormal2;
		m_Color = color;
		m_Opacity = opacity;
		m_SpecularPower = specpower;
		m_Shininess = shine;
	}

	inline bool operator==(const AuModelSegment &rhs) const
	{
		if (m_Color != rhs.m_Color)
			return(false);
		if (m_Opacity != rhs.m_Opacity)
			return(false);
		if (m_SpecularPower != rhs.m_SpecularPower)
			return(false);
		if (m_Shininess != rhs.m_Shininess)
			return(false);
		if (m_fLoc1 != rhs.m_fLoc1)
			return(false);
		if (m_fLoc2 != rhs.m_fLoc2)
			return(false);
		if (m_fNormal1 != rhs.m_fNormal1)
			return(false);
		if (m_fNormal2 != rhs.m_fNormal2)
			return(false);
		return(true);
	}

	inline void Transform(const AuMatrix4x4 &tModel)
	{
		m_fLoc1 = tModel.Project(m_fLoc1);
		m_fLoc2 = tModel.Project(m_fLoc2);
		m_fNormal1 = tModel.ProjectVector(m_fNormal1);
		m_fNormal2 = tModel.ProjectVector(m_fNormal2);
	}
};

class AUMATH_DLL AuModelSegmentArray : public QVector<AuModelSegment>
{
public:
	// Returns index of segment; -1 if not found.
	inline int FindSegment(const AuModelSegment &seg) const
	{
		const AuModelSegment *pCheckSeg = begin();
		int lSegs=count();
		for (int lSegIndex=0; lSegIndex<lSegs; lSegIndex++, pCheckSeg++)
		{
			if (*pCheckSeg == seg)
				return(lSegIndex);
		}
		return(-1);
	}

	inline void Transform(const AuMatrix4x4 &tModel)
	{
		AuModelSegment *pSeg = begin();
		int lSegs=count();
		for (int lSegIndex=0; lSegIndex<lSegs; lSegIndex++, pSeg++)
			pSeg->Transform(tModel);
	}
};

#endif // AuSegment_H
