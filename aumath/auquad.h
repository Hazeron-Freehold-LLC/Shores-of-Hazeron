//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auquad.h,v 1.4 2022/02/19 18:38:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuQuad_H
#define AuQuad_H

#include "aupoint.h"

#include "aumatrix4x4.h"
#include "autriangle.h"

class AUMATH_DLL AuQuad  
{
public:
	AuPoint		vll;	// "lower-left" vertex
	AuPoint		vul;	// "upper-left" vertex
	AuPoint		vur;	// "upper-right" vertex
	AuPoint		vlr;	// "lower-right" vertex
	AuPoint		nll;	// surface normal (vlr, vll, vul).
	AuPoint		nur;	// surface normal (vul, vur, vlr).
	bool		bl;		// A bool for the left edge.
	bool		bt;		// A bool for the top edge.
	bool		br;		// A bool for the right edge.
	bool		bb;		// A bool for the bottom edge.
	double		ts,tt;	// Texture s,t coord (ll).
	double		tu,tv;	// Texture u,v coord (ur).

public:
	inline AuQuad()
	{
		bl = 0;
		bt = 0;
		br = 0;
		bb = 0;
		ts = 0.0;
		tt = 0.0;
		tu = 1.0;
		tv = 1.0;
	}
	inline AuQuad(const AuPoint &fll, const AuPoint &ful, const AuPoint &fur,
			const AuPoint &flr)
	{
		vll = fll;
		vul = ful;
		vur = fur;
		vlr = flr;
		RecalcNormals();

		bl = 0;
		bt = 0;
		br = 0;
		bb = 0;
		ts = 0.0;
		tt = 0.0;
		tu = 1.0;
		tv = 1.0;
	}

	// Returns closest edge segment distance.
	inline double EdgeDistance(const AuPoint &fTest) const
	{
		double rbest = fTest.DistToSegment(vll, vul);
		double rseg = fTest.DistToSegment(vul, vur);
		if (rseg < rbest)
			rbest = rseg;
		rseg = fTest.DistToSegment(vur, vlr);
		if (rseg < rbest)
			rbest = rseg;
		rseg = fTest.DistToSegment(vlr, vll);
		if (rseg < rbest)
			rbest = rseg;

		return rbest;
	}

	inline void Offset(const AuPoint& v)
	{
		vll += v;
		vul += v;
		vur += v;
		vlr += v;
	}

	inline bool operator==(const AuQuad& c) const
	{
		if(bl != c.bl)
			return false;
		if(bt != c.bt)
			return false;
		if(br != c.br)
			return false;
		if(bb != c.bb)
			return false;
		if(ts != c.ts)
			return false;
		if(tt != c.tt)
			return false;
		if(tu != c.tu)
			return false;
		if(tv != c.tv)
			return false;
		if(vll != c.vll)
			return false;
		if(vul != c.vul)
			return false;
		if(vur != c.vur)
			return false;
		if(vlr != c.vlr)
			return false;
		if(nll != c.nll)
			return false;
		if(nur != c.nur)
			return false;
		return true;
	}

	inline void RecalcNormals()
	{
		// Calculate first normal at lower left corner of quad.
		nll = AuPoint(0,0);
		AuPoint vx = vlr - vll;
		if (!vx.IsZero())
		{
			vx.Unit();
			AuPoint vy = vul - vll;
			if (!vy.IsZero())
			{
				nll = vx.Cross(vy);
				nll.Unit();
			}
		}

		// Calculate second normal at upper right corner of quad.
		nur = AuPoint(0,0);
		vx = vul - vur;
		if (!vx.IsZero())
		{
			vx.Unit();
			AuPoint vy = vlr - vur;
			if (!vy.IsZero())
			{
				nur = vx.Cross(vy);
				nur.Unit();
			}
		}

		// Make sure normals are sane.
		if (nll.IsZero())
			nll = nur;
		if (nur.IsZero())
			nur = nll;
		if (nll.IsZero())
		{
			nll = AuPoint(0,0,1);
			nur = nll;
		}
	}

	inline bool TestCollision(const AuQuad &aTest) const
	{
		double rdn1 = nll.Dot(aTest.nll);
		double rdn2 = nur.Dot(aTest.nll);
		double rdn3 = nll.Dot(aTest.nur);
		double rdn4 = nur.Dot(aTest.nur);
		if (FuzzyEquals(fabs(rdn1), 1.0) &&
			FuzzyEquals(fabs(rdn2), 1.0) &&
			FuzzyEquals(fabs(rdn3), 1.0) &&
			FuzzyEquals(fabs(rdn4), 1.0))
			return false;	// Parallel surfaces do not collide.

		AuPoint v1 = aTest.vll - vll;
		double rd11 = v1.Dot(nll);
		double rd12 = v1.Dot(nur);
		AuPoint v2 = aTest.vul - vll;
		double rd21 = v2.Dot(nll);
		double rd22 = v2.Dot(nur);
		AuPoint v3 = aTest.vur - vll;
		double rd31 = v3.Dot(nll);
		double rd32 = v3.Dot(nur);
		AuPoint v4 = aTest.vlr - vll;
		double rd41 = v4.Dot(nll);
		double rd42 = v4.Dot(nur);

		if (rd11 >= 0.0 && rd21 >= 0.0 && rd31 >= 0.0 && rd41 >= 0.0 &&
			rd12 >= 0.0 && rd22 >= 0.0 && rd32 >= 0.0 && rd42 >= 0.0)
			return false;	// All test vertices above this plane.

		if (rd11 <= 0.0 && rd21 <= 0.0 && rd31 <= 0.0 && rd41 <= 0.0 &&
			rd12 <= 0.0 && rd22 <= 0.0 && rd32 <= 0.0 && rd42 <= 0.0)
			return false;	// All test vertices below this plane.

		AuPoint v1t = vll - aTest.vll;
		double rd11t = v1t.Dot(aTest.nll);
		double rd12t = v1t.Dot(aTest.nur);
		AuPoint v2t = vul - aTest.vll;
		double rd21t = v2t.Dot(aTest.nll);
		double rd22t = v2t.Dot(aTest.nur);
		AuPoint v3t = vur - aTest.vll;
		double rd31t = v3t.Dot(aTest.nll);
		double rd32t = v3t.Dot(aTest.nur);
		AuPoint v4t = vlr - aTest.vll;
		double rd41t = v4t.Dot(aTest.nll);
		double rd42t = v4t.Dot(aTest.nur);

		if (rd11t >= 0.0 && rd21t >= 0.0 && rd31t >= 0.0 && rd41t >= 0.0 &&
			rd12t >= 0.0 && rd22t >= 0.0 && rd32t >= 0.0 && rd42t >= 0.0)
			return false;	// All my vertices above test plane.

		if (rd11t <= 0.0 && rd21t <= 0.0 && rd31t <= 0.0 && rd41t <= 0.0 &&
			rd12t <= 0.0 && rd22t <= 0.0 && rd32t <= 0.0 && rd42t <= 0.0)
			return false;	// All my vertices below test plane.

		// Check all edge segments of the test poly that have vertices on either side of this plane.
		// Segment intersections are painful. We can tell a lot from the dot products before doing the hard work
		// of calling TestSegmentIntersection.

		// Segment 1 - aTest.vll to aTest.vul
		if (!aTest.vll.FuzzyEquals(aTest.vul))
		{
			if (!FuzzyEquals(rd11, rd21) || !FuzzyEquals(rd12, rd21) || !FuzzyEquals(rd11, rd22) || !FuzzyEquals(rd12, rd22))
			{
				// Now we know the edge segment is not parallel to my plane.
				if (((rd11 >= 0.0 || rd12 >= 0.0) && (rd21 <= 0.0 || rd22 <= 0.0)) ||
					((rd11 <= 0.0 || rd12 <= 0.0) && (rd21 >= 0.0 || rd22 >= 0.0)))
				{
					// Now we know that one end of the edge segment is above my plane and 
					// the other end of the edge segment is below my plane.
					AuPoint fInt, fN;
					if (TestSegmentIntersection(fInt, fN, aTest.vll, aTest.vul))
						return true;
				}
			}
		}

		// Segment 2 - aTest.vul to aTest.vur
		if (!aTest.vul.FuzzyEquals(aTest.vur))
		{
			if (!FuzzyEquals(rd21, rd31) || !FuzzyEquals(rd22, rd31) || !FuzzyEquals(rd21, rd32) || !FuzzyEquals(rd22, rd32))
			{
				// Now we know the edge segment is not parallel to my plane.
				if (((rd21 >= 0.0 || rd22 >= 0.0) && (rd31 <= 0.0 || rd32 <= 0.0)) ||
					((rd21 <= 0.0 || rd22 <= 0.0) && (rd31 >= 0.0 || rd32 >= 0.0)))
				{
					// Now we know that one end of the edge segment is above my plane and 
					// the other end of the edge segment is below my plane.
					AuPoint fInt, fN;
					if (TestSegmentIntersection(fInt, fN, aTest.vul, aTest.vur))
						return true;
				}
			}
		}


		// Segment 3 - aTest.vur to aTest.vlr
		if (!aTest.vur.FuzzyEquals(aTest.vlr))
		{
			if (!FuzzyEquals(rd31, rd41) || !FuzzyEquals(rd32, rd41) || !FuzzyEquals(rd31, rd42) || !FuzzyEquals(rd32, rd42))
			{
				// Now we know the edge segment is not parallel to my plane.
				if (((rd31 >= 0.0 || rd32 >= 0.0) && (rd41 <= 0.0 || rd42 <= 0.0)) ||
					((rd31 <= 0.0 || rd32 <= 0.0) && (rd41 >= 0.0 || rd42 >= 0.0)))
				{
					// Now we know that one end of the edge segment is above my plane and 
					// the other end of the edge segment is below my plane.
					AuPoint fInt, fN;
					if (TestSegmentIntersection(fInt, fN, aTest.vul, aTest.vur))
						return true;
				}
			}
		}


		// Segment 4 - aTest.vlr to aTest.vll
		if (!aTest.vlr.FuzzyEquals(aTest.vll))
		{
			if (!FuzzyEquals(rd41, rd11) || !FuzzyEquals(rd42, rd11) || !FuzzyEquals(rd41, rd12) || !FuzzyEquals(rd42, rd12))
			{
				// Now we know the edge segment is not parallel to my plane.
				if (((rd41 >= 0.0 || rd42 >= 0.0) && (rd11 <= 0.0 || rd12 <= 0.0)) ||
					((rd41 <= 0.0 || rd42 <= 0.0) && (rd11 >= 0.0 || rd12 >= 0.0)))
				{
					// Now we know that one end of the edge segment is above my plane and 
					// the other end of the edge segment is below my plane.
					AuPoint fInt, fN;
					if (TestSegmentIntersection(fInt, fN, aTest.vul, aTest.vur))
						return true;
				}
			}
		}

		// Check all edge segments of this poly that have vertices on either side of the test plane.

		// Segment 1 - vll to vul
		if (!vll.FuzzyEquals(vul))
		{
			if (!FuzzyEquals(rd11t, rd21t) || !FuzzyEquals(rd12t, rd21t) || !FuzzyEquals(rd11t, rd22t) || !FuzzyEquals(rd12t, rd22t))
			{
				// Now we know the edge segment is not parallel to my plane.
				if (((rd11t >= 0.0 || rd12t >= 0.0) && (rd21t <= 0.0 || rd22t <= 0.0)) ||
					((rd11t <= 0.0 || rd12t <= 0.0) && (rd21t >= 0.0 || rd22t >= 0.0)))
				{
					// Now we know that one end of the edge segment is above my plane and 
					// the other end of the edge segment is below my plane.
					AuPoint fInt, fN;
					if (aTest.TestSegmentIntersection(fInt, fN, vll, vul))
						return true;
				}
			}
		}

		// Segment 2 - vul to vur
		if (!vul.FuzzyEquals(vur))
		{
			if (!FuzzyEquals(rd21t, rd31t) || !FuzzyEquals(rd22t, rd31t) || !FuzzyEquals(rd21t, rd32t) || !FuzzyEquals(rd22t, rd32t))
			{
				// Now we know the edge segment is not parallel to my plane.
				if (((rd21t >= 0.0 || rd22t >= 0.0) && (rd31t <= 0.0 || rd32t <= 0.0)) ||
					((rd21t <= 0.0 || rd22t <= 0.0) && (rd31t >= 0.0 || rd32t >= 0.0)))
				{
					// Now we know that one end of the edge segment is above my plane and 
					// the other end of the edge segment is below my plane.
					AuPoint fInt, fN;
					if (aTest.TestSegmentIntersection(fInt, fN, vul, vur))
						return true;
				}
			}
		}


		// Segment 3 - vur to vlr
		if (!vur.FuzzyEquals(vlr))
		{
			if (!FuzzyEquals(rd31t, rd41t) || !FuzzyEquals(rd32t, rd41t) || !FuzzyEquals(rd31t, rd42t) || !FuzzyEquals(rd32t, rd42t))
			{
				// Now we know the edge segment is not parallel to my plane.
				if (((rd31t >= 0.0 || rd32t >= 0.0) && (rd41t <= 0.0 || rd42t <= 0.0)) ||
					((rd31t <= 0.0 || rd32t <= 0.0) && (rd41t >= 0.0 || rd42t >= 0.0)))
				{
					// Now we know that one end of the edge segment is above my plane and 
					// the other end of the edge segment is below my plane.
					AuPoint fInt, fN;
					if (aTest.TestSegmentIntersection(fInt, fN, vul, vur))
						return true;
				}
			}
		}


		// Segment 4 - vlr to vll
		if (!vlr.FuzzyEquals(vll))
		{
			if (!FuzzyEquals(rd41t, rd11t) || !FuzzyEquals(rd42t, rd11t) || !FuzzyEquals(rd41t, rd12t) || !FuzzyEquals(rd42t, rd12t))
			{
				// Now we know the edge segment is not parallel to my plane.
				if (((rd41t >= 0.0 || rd42t >= 0.0) && (rd11t <= 0.0 || rd12t <= 0.0)) ||
					((rd41t <= 0.0 || rd42t <= 0.0) && (rd11t >= 0.0 || rd12t >= 0.0)))
				{
					// Now we know that one end of the edge segment is above my
					// plane and the other end of the edge segment is below my
					// plane.
					AuPoint fInt, fN;
					if (aTest.TestSegmentIntersection(fInt, fN, vul, vur))
						return true;
				}
			}
		}

		return false;
	}

	inline bool TestPointInside(const AuPoint &fTest) const
	{
		if(fTest.InTriangle_3D(vlr, vll, vul))
			return true;
		if(fTest.InTriangle_3D(vul, vur, vlr))
			return true;
		return false;
	}

	inline bool TestSegmentIntersection(AuPoint &fIntersect, AuPoint &fNormal,
			const AuPoint &fSeg1, const AuPoint &fSeg2) const
	{
		AuTriangle t1(vlr, vll, vul);
		int isect1 = t1.SegmentIntersection(fIntersect, fSeg1, fSeg2);
		if (isect1 == 3)
		{
			fNormal = nll;
			return true;
		}
		AuTriangle t2(vul, vur, vlr);
		int isect2 = t2.SegmentIntersection(fIntersect, fSeg1, fSeg2);
		if (isect2 == 3)
		{
			fNormal = nur;
			return true;
		}
		return false;
	}

	inline void Transform(const AuMatrix4x4 &t)
	{
		vll = t.Project(vll);
		vul = t.Project(vul);
		vur = t.Project(vur);
		vlr = t.Project(vlr);
		nll = t.ProjectVector(nll);
		nll.Unit();
		nur = t.ProjectVector(nur);
		nur.Unit();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUMATH_DLL AuQuadArray : public QVector<AuQuad>
{
public:
	inline void AddQuad(const AuPoint &vll, const AuPoint &vul, const AuPoint &vur,
			const AuPoint &vlr)
	{
		AuQuad q(vll, vul, vur, vlr);
		append(q);
	}

	// Returns closest edge segment distance.
	inline double EdgeDistance(const AuPoint &fTest) const
	{
		double rClosest = -1;
		const AuQuad *pq = begin();
		for (int iq=0; iq<count(); iq++, pq++)
		{
			double rDist = pq->EdgeDistance(fTest);
			if (rClosest == -1 || rDist < rClosest)
				rClosest = rDist;
		}
		return rClosest;
	}

	inline void RecalcNormals()
	{
		AuQuad *pq = begin();
		for(int iq=0; iq<count(); iq++, pq++)
			pq->RecalcNormals();
	}

	// Check each quad in this array against the quads in the 'aTest' array.
	// true is returned if there are any collisions.
	inline bool TestCollision(const AuQuadArray &aTest) const
	{
		// Test each quad in my array against all the quads in 'aTest'.
		const AuQuad *pq = begin();
		for(int iq=0; iq<count(); iq++, pq++)
			if(aTest.TestCollision(*pq))
				return true;
		return false;	// No intersections found.
	}

	// Check each quad in this array against the quad 'aTest'.
	// true is returned if there are any collisions.
	inline bool TestCollision(const AuQuad &aTest) const
	{
		// Test each quad in my array against the quad 'aTest'.
		const AuQuad *pq = begin();
		for(int iq=0; iq<count(); iq++, pq++)
			if(aTest.TestCollision(*pq))
				return true;
		return false;	// No intersection found.
	}

	// Tests if fTest is inside the convex volume defined by these quads.
	inline bool TestPointInside(const AuPoint &fTest) const
	{
		// Test each quad in my array against the point 'fTest'.
		const AuQuad *pq = begin();
		for(int iq=0; iq<count(); iq++, pq++)
		{
			const AuPoint v1 = fTest - pq->vul;
			double r1 = v1.Dot(pq->nll);
			const AuPoint v2 = fTest - pq->vlr;
			double r2 = v2.Dot(pq->nur);
			if(r1 > 0.0 && r2 > 0.0)
				return false;	// The point is 'outside' the quad.
		}
		return true;
	}

	inline void Transform(const AuMatrix4x4 &t)
	{
		AuQuad *pq = begin();
		for(int iq=0; iq<count(); iq++, pq++)
			pq->Transform(t);
	}
};

#endif // AuQuad_H
