//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscface.cpp,v 1.102 2022/04/08 21:18:53 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscface.h"
//
#include "aubox.h"
#include "auglobell.h"
#include "aunoise.h"
#include "auplane.h"
#include <QDebug>
#include <QFile>

#define d_FeetPerMeter					(3.28083989501312)
#define d_PlaneTol						(FUZZ)	//(.001 * d_FeetPerMeter)
#define d_SegTol						(FUZZ)	// (.001 * d_FeetPerMeter)
#define d_SliceFuzz						(FUZZ)	//(.001 * d_FeetPerMeter)
#define d_TriTol						(FUZZ)	// (.001 * d_FeetPerMeter)

static int s_AuScIdSetMetaType = AuScIdSet::RegisterMetaType();

//static
int AuScIdSet::RegisterMetaType()
{
	// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
	// The Q_DECLARE_METATYPE macro is also required.
	int metaType = qRegisterMetaType<AuScIdSet>();

	// Must call qRegisterMetaTypeStreamOperators in order to use this class type in containers of variants.
	qRegisterMetaTypeStreamOperators<AuScIdSet>();

	return metaType;
}

QDebug operator<<(QDebug dbg, const AuScTri& t)
{
	dbg.nospace() << t.V0() << " " << t.V1() << " " << t.V2();
	return dbg.space();
}

QDebug operator<<(QDebug dbg, const AuScTriList& t)
{
	dbg.nospace() << t.size();

	int limit = 10;
	for(AuScTriList::const_iterator ti = t.cbegin(); ti != t.cend() && limit; ti++, limit--)
		dbg.nospace() << "\n " << *ti;

	return dbg.space();
}

QDebug operator<<(QDebug dbg, const AuScFace& f)
{
	dbg.nospace() << (quint8)f.Type() << " " << f.VertexCount();
	int limit = 8;
	for(int v=0; v<f.VertexCount() && limit; v++, limit--)
		dbg.nospace() << "\n " << f.Vertex(v);

	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// qHash is needed to use AuScSelectInfo in a QList in Windows.
unsigned int qHash(const AuScSelectInfo& key)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << key;
	return qHash(ba);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

bool AuScSelection::HasEdgeSelection() const
{
	for(const_iterator si = cbegin(); si != cend(); si++)
		if((*si).HasEdgeSelection())
			return true;
	return false;
}

bool AuScSelection::HasFaceSelection() const
{
	for(const_iterator si = cbegin(); si != cend(); si++)
		if((*si).HasFaceSelection())
			return true;
	return false;
}

bool AuScSelection::HasObjectSelection() const
{
	for(const_iterator si = cbegin(); si != cend(); si++)
		if((*si).HasObjectSelection())
			return true;
	return false;
}

bool AuScSelection::HasVertexSelection() const
{
	for(const_iterator si = cbegin(); si != cend(); si++)
		if((*si).HasVertexSelection())
			return true;
	return false;
}

void AuScSelection::InvertObjectSelection(const AuScSelection& allSel)
{
	for(iterator si = begin(); si != end(); si++)
	{
		const_iterator api = allSel.find(si.key());
		if(api != allSel.cend())
		{
			QSet<quint32> nuSelEdges;
			for(QSet<quint32>::const_iterator avi = (*api).m_Edges.cbegin(); avi != (*api).m_Edges.cend(); avi++)
			{
				if(!(*si).m_Edges.contains(*avi))
					nuSelEdges.insert(*avi);
			}
			(*si).m_Edges = nuSelEdges;

			QSet<quint32> nuSelFaces;
			for(QSet<quint32>::const_iterator avi = (*api).m_Faces.cbegin(); avi != (*api).m_Faces.cend(); avi++)
			{
				if(!(*si).m_Faces.contains(*avi))
					nuSelFaces.insert(*avi);
			}
			(*si).m_Faces = nuSelFaces;

			QSet<quint32> nuSelVerts;
			for(QSet<quint32>::const_iterator avi = (*api).m_Vertices.cbegin(); avi != (*api).m_Vertices.cend(); avi++)
			{
				if(!(*si).m_Vertices.contains(*avi))
					nuSelVerts.insert(*avi);
			}
			(*si).m_Vertices = nuSelVerts;
		}
		else
		{
			// Selected part not found in allSel? wtf
			(*si).m_Edges.clear();
			(*si).m_Faces.clear();
			(*si).m_Vertices.clear();
		}
	}
}

void AuScSelection::TogglePartObjectSelection(const AuScSelection& toggleSel)
{
	for(const_iterator ri = toggleSel.cbegin(); ri != toggleSel.cend(); ri++)
	{
		iterator si = find(ri.key());
		if(si == end())
			continue;

		// Toggle selection state of edges.
		for(QSet<quint32>::const_iterator ti = (*ri).m_Edges.cbegin(); ti != (*ri).m_Edges.cend(); ti++)
		{
			if((*si).m_Edges.contains(*ti))
				(*si).m_Edges.remove(*ti);
			else
				(*si).m_Edges.insert(*ti);
		}

		// Toggle selection state of faces.
		for(QSet<quint32>::const_iterator ti = (*ri).m_Faces.cbegin(); ti != (*ri).m_Faces.cend(); ti++)
		{
			if((*si).m_Faces.contains(*ti))
				(*si).m_Faces.remove(*ti);
			else
				(*si).m_Faces.insert(*ti);
		}

		// Toggle selection state of vertices.
		for(QSet<quint32>::const_iterator ti = (*ri).m_Vertices.cbegin(); ti != (*ri).m_Vertices.cend(); ti++)
		{
			if((*si).m_Vertices.contains(*ti))
				(*si).m_Vertices.remove(*ti);
			else
				(*si).m_Vertices.insert(*ti);
		}
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

bool AuScTri::ColinearEdge(const AuScTri& tri) const
{
	AuPoint te0 = tri.V1() - tri.V0();
	te0.Unit();
	AuPoint te1 = tri.V2() - tri.V1();
	te1.Unit();
	AuPoint te2 = tri.V0() - tri.V2();
	te2.Unit();

	// Check my edge 0.
	AuPoint e0 = V1() - V0();
	e0.Unit();
	if(e0.IsParallel(te0) && V0().DistToLine(tri.V0(), tri.V1()) <= FUZZ)
	{
		// Edge line 0 is colinear with triangle's edge line 0.
		return V0().Between(tri.V0(), tri.V1(), FUZZ) ||
				V1().Between(tri.V0(), tri.V1(), FUZZ) ||
				tri.V0().Between(V0(), V1(), FUZZ) ||
				tri.V1().Between(V0(), V1(), FUZZ);
	}
	else if(e0.IsParallel(te1) && V0().DistToLine(tri.V1(), tri.V2()) <= FUZZ)
	{
		// Edge line 0 is colinear with triangle's edge line 1.
		return V0().Between(tri.V1(), tri.V2(), FUZZ) ||
				V1().Between(tri.V1(), tri.V2(), FUZZ) ||
				tri.V1().Between(V0(), V1(), FUZZ) ||
				tri.V2().Between(V0(), V1(), FUZZ);
	}
	else if(e0.IsParallel(te2) && V0().DistToLine(tri.V2(), tri.V0()) <= FUZZ)
	{
		// Edge line 0 is colinear with triangle's edge line 2.
		return V0().Between(tri.V2(), tri.V0(), FUZZ) ||
				V1().Between(tri.V2(), tri.V0(), FUZZ) ||
				tri.V2().Between(V0(), V1(), FUZZ) ||
				tri.V0().Between(V0(), V1(), FUZZ);
	}

	// Check my edge 1.
	AuPoint e1 = V2() - V1();
	e1.Unit();
	if(e1.IsParallel(te0) && V1().DistToLine(tri.V0(), tri.V1()) <= FUZZ)
	{
		// Edge line 1 is colinear with triangle's edge line 0.
		return V1().Between(tri.V0(), tri.V1(), FUZZ) ||
				V2().Between(tri.V0(), tri.V1(), FUZZ) ||
				tri.V0().Between(V1(), V2(), FUZZ) ||
				tri.V1().Between(V1(), V2(), FUZZ);
	}
	else if(e1.IsParallel(te1) && V1().DistToLine(tri.V1(), tri.V2()) <= FUZZ)
	{
		// Edge line 1 is colinear with triangle's edge line 1.
		return V1().Between(tri.V1(), tri.V2(), FUZZ) ||
				V2().Between(tri.V1(), tri.V2(), FUZZ) ||
				tri.V1().Between(V1(), V2(), FUZZ) ||
				tri.V2().Between(V1(), V2(), FUZZ);
	}
	else if(e1.IsParallel(te2) && V1().DistToLine(tri.V2(), tri.V0()) <= FUZZ)
	{
		// Edge line 1 is colinear with triangle's edge line 2.
		return V1().Between(tri.V2(), tri.V0(), FUZZ) ||
				V2().Between(tri.V2(), tri.V0(), FUZZ) ||
				tri.V2().Between(V1(), V2(), FUZZ) ||
				tri.V0().Between(V1(), V2(), FUZZ);
	}

	// Check my edge 2.
	AuPoint e2 = V0() - V2();
	e2.Unit();
	if(e2.IsParallel(te0) && V2().DistToLine(tri.V0(), tri.V1()) <= FUZZ)
	{
		// Edge line 2 is colinear with triangle's edge line 0.
		return V2().Between(tri.V0(), tri.V1(), FUZZ) ||
				V0().Between(tri.V0(), tri.V1(), FUZZ) ||
				tri.V0().Between(V2(), V0(), FUZZ) ||
				tri.V1().Between(V2(), V0(), FUZZ);
	}
	else if(e2.IsParallel(te1) && V2().DistToLine(tri.V1(), tri.V2()) <= FUZZ)
	{
		// Edge line 2 is colinear with triangle's edge line 1.
		return V2().Between(tri.V1(), tri.V2(), FUZZ) ||
				V0().Between(tri.V1(), tri.V2(), FUZZ) ||
				tri.V1().Between(V2(), V0(), FUZZ) ||
				tri.V2().Between(V2(), V0(), FUZZ);
	}
	else if(e2.IsParallel(te2) && V2().DistToLine(tri.V2(), tri.V0()) <= FUZZ)
	{
		// Edge line 2 is colinear with triangle's edge line 2.
		return V2().Between(tri.V2(), tri.V0(), FUZZ) ||
				V0().Between(tri.V2(), tri.V0(), FUZZ) ||
				tri.V2().Between(V2(), V0(), FUZZ) ||
				tri.V0().Between(V2(), V0(), FUZZ);
	}

	return false;
}

void AuScTri::EdgeIntersections(const AuLine& line, AuPointArray& intersections) const
{
	// Find the intersections, if any.
	AuPoint linePoint = line.U + line.V;

	AuPoint e0int;
	if(e0int.SegmentIntersection(V0(), V1(), line.U, linePoint, true, false, d_SliceFuzz))
		intersections.AddIfUnique(e0int);

	AuPoint e1int;
	if(e1int.SegmentIntersection(V1(), V2(), line.U, linePoint, true, false, d_SliceFuzz))
		intersections.AddIfUnique(e1int);

	AuPoint e2int;
	if(e2int.SegmentIntersection(V2(), V0(), line.U, linePoint, true, false, d_SliceFuzz))
		intersections.AddIfUnique(e2int);
}

bool AuScTri::EdgeOnLine(const AuLine& line, AuSegment& edgeSeg) const
{
	double v0dist = line.Distance(V0());
	double v1dist = line.Distance(V1());
	if(v0dist <= d_SliceFuzz && v1dist <= d_SliceFuzz)
	{
		edgeSeg.m_fLoc1 = V0();
		edgeSeg.m_fLoc2 = V1();
		return true;
	}
	else if(v1dist <= d_SliceFuzz)
	{
		double v2dist = line.Distance(V2());
		if(v2dist <= d_SliceFuzz)
		{
			edgeSeg.m_fLoc1 = V1();
			edgeSeg.m_fLoc2 = V2();
			return true;
		}
	}
	else if(v0dist <= d_SliceFuzz)
	{
		double v2dist = line.Distance(V2());
		if(v2dist <= d_SliceFuzz)
		{
			edgeSeg.m_fLoc1 = V2();
			edgeSeg.m_fLoc2 = V0();
			return true;
		}
	}
	return false;
}

bool AuScTri::Slice(const AuLine& cutLine, AuScTriList& pieces) const
{
	// Find the line intersections, if any.
	AuPoint linePoint = cutLine.U + cutLine.V;

	AuPoint e0int;
	bool e0hit = e0int.SegmentIntersection(V0(), V1(), cutLine.U, linePoint, true, false, d_SliceFuzz);

	AuPoint e1int;
	bool e1hit = e1int.SegmentIntersection(V1(), V2(), cutLine.U, linePoint, true, false, d_SliceFuzz);

	AuPoint e2int;
	bool e2hit = e2int.SegmentIntersection(V2(), V0(), cutLine.U, linePoint, true, false, d_SliceFuzz);

	// Determine the intersections. There could be 0, 1, 2 or 3, since it will hit at vertices.
	if(!e0hit && !e1hit && !e2hit)
		return false;

	if(e0hit && e1hit && e2hit)
	{
		// We have hit a line and a vertex close enough to count as a hit on both of those vertexes segments.
		// Eliminate one so the next logic doesn't fail because of it.
		if(e0hit && e0int.FuzzyEquals(V0()))
			e0hit = false;
		if(e1hit && e1int.FuzzyEquals(V1()))
			e1hit = false;
		if(e2hit && e2int.FuzzyEquals(V2()))
			e2hit = false;
	}

	AuScPoint int1, int2;
	double frac;
	if(e0hit && e1hit)
	{
		frac = V0().Distance(e0int)/V0().Distance(V1());
		int1 = V0().LinearInterpolate(V1(), frac);

		frac = V1().Distance(e1int)/V1().Distance(V2());
		int2 = V1().LinearInterpolate(V2(), frac);
	}
	else if(e1hit && e2hit)
	{
		frac = V1().Distance(e1int)/V1().Distance(V2());
		int1 = V1().LinearInterpolate(V2(), frac);

		frac = V2().Distance(e2int)/V2().Distance(V0());
		int2 = V2().LinearInterpolate(V0(), frac);
	}
	else if(e2hit && e0hit)
	{
		frac = V2().Distance(e2int)/V2().Distance(V0());
		int1 = V2().LinearInterpolate(V0(), frac);

		frac = V0().Distance(e0int)/V0().Distance(V1());
		int2 = V0().LinearInterpolate(V1(), frac);
	}
	else
		return false;					// Only one intersection so one vertex hit; no clip.

	// If both intersections are vertices then it was an edge hit and we are not clipped.
	bool int1atv0 = int1.FuzzyEquals(V0());
	bool int1atv1 = int1.FuzzyEquals(V1());
	bool int1atv2 = int1.FuzzyEquals(V2());
	bool int2atv0 = int2.FuzzyEquals(V0());
	bool int2atv1 = int2.FuzzyEquals(V1());
	bool int2atv2 = int2.FuzzyEquals(V2());
	bool int1atvertex = int1atv0 || int1atv1 || int1atv2;
	bool int2atvertex = int2atv0 || int2atv1 || int2atv2;
	if(int1atvertex && int2atvertex)
		return false;					// Hit along an edge.

	// If either intersection is at a vertex, then two triangles are formed.
	// Otherwise, three triangles are formed. In that case, the first two triangles will be ordered to form a quad, for the caller.
	if(int1atvertex)
	{
		// Slice into two triangles across the vertex at int1.
		if(int1atv0)
		{
			pieces.append(AuScTri(V0(), V1(), int2));
			pieces.append(AuScTri(int2, V2(), V0()));
		}
		else if(int1atv1)
		{
			pieces.append(AuScTri(V1(), V2(), int2));
			pieces.append(AuScTri(int2, V0(), V1()));
		}
		else
		{
			pieces.append(AuScTri(V2(), V0(), int2));
			pieces.append(AuScTri(int2, V1(), V2()));
		}
	}
	else if(int2atvertex)
	{
		// Slice into two triangles across the vertex at int2.
		if(int2atv0)
		{
			pieces.append(AuScTri(V0(), V1(), int1));
			pieces.append(AuScTri(int1, V2(), V0()));
		}
		else if(int2atv1)
		{
			pieces.append(AuScTri(V1(), V2(), int1));
			pieces.append(AuScTri(int1, V0(), V1()));
		}
		else
		{
			pieces.append(AuScTri(V2(), V0(), int1));
			pieces.append(AuScTri(int1, V1(), V2()));
		}
	}
	else
	{
		// Slice into three triangles.
		if(e0hit && e1hit)
		{
			// Quad
			pieces.append(AuScTri(V2(), V0(), int1));
			pieces.append(AuScTri(int1, int2, V2()));

			// Triangle
			pieces.append(AuScTri(int1, V1(), int2));
		}
		else if(e1hit && e2hit)
		{
			// Quad
			pieces.append(AuScTri(V0(), V1(), int1));
			pieces.append(AuScTri(int1, int2, V0()));

			// Triangle
			pieces.append(AuScTri(int1, V2(), int2));
		}
		else
		{
			// Quad
			pieces.append(AuScTri(V1(), V2(), int1));
			pieces.append(AuScTri(int1, int2, V1()));

			// Triangle
			pieces.append(AuScTri(int1, V0(), int2));
		}
	}
	return true;
}

bool AuScTri::Collapsed() const
{
	if(m_V[0].FuzzyEquals(m_V[1]) || m_V[1].FuzzyEquals(m_V[2]) || m_V[2].FuzzyEquals(m_V[0]))
		return true;

	AuPoint v1 = m_V[1] - m_V[0];
	v1.Unit();
	AuPoint v2 = m_V[2] - m_V[0];
	v2.Unit();

	double dot = fabs(v1.Dot(v2));
	return dot == 1.;
	// too fuzzy return v1.IsParallel(v2);
}

bool AuScTri::IntersectZ(const AuPoint& loc, AuPoint& intersect) const
{
	// Test for 2D intersect with triangle.
	if(!loc.InTriangle(m_V[0], m_V[1], m_V[2]))
		return false;

	// Find the exact intersection with the plane of the triangle.
	AuPoint n(Normal());
	AuPlane fp(m_V[0], n);
	AuLine fl(loc.x,loc.y,loc.z, 0,0,1);					// Infinite line along the Z axis.
	if(!fp.LineIntersection(fl, intersect))
		return false;										// The line does not intersect the plane.

	return true;
}

bool AuScTri::IntersectRay(const AuPoint& loc, const AuPoint& vec, AuPoint& intersect) const
{
	// Find the exact intersection with the plane of the triangle.
	AuPoint n(Normal());
	AuPlane fp(m_V[0], n);
	AuLine fl(loc.x,loc.y,loc.z, vec.x,vec.y,vec.z);			// Infinite line along the ray.
	if(!fp.LineIntersection(fl, intersect))
		return false;											// The line does not intersect the plane.

	// Test for intersection at loc.
	if(intersect.FuzzyEquals(loc))
		return true;											// Intersection is at loc.

	// Test intersection for correct direction along the ray.
	double raydot = (intersect - loc).Dot(vec);
	if(raydot < 0)
		return false;											// Intersection hit the ray in the wrong direction.

	// Intersection is in the right direction. Is it in the triangle?
	if(!intersect.InTriangleFuzzy_3D(m_V[0], m_V[1], m_V[2], n, d_PlaneTol))
		return false;											// Intersection did not hit inside the triangle.

	// Intersection is on the ray inside the triangle.
	return true;
}

bool AuScTri::Sight(const AuPoint& eyeLoc, AuPoint& sightLoc) const
{
	if(eyeLoc == sightLoc)
		return false;

	// Find the exact intersection with the plane of the triangle.
	AuPoint n(Normal());										// Normal to the triangle plane.
	AuPoint eyeVec = sightLoc - eyeLoc;
	double sightDist = eyeVec.Length();
	eyeVec /= sightDist;										// Sight direction as a unit vector.

	double rDot = eyeVec.Dot(n);
	if(rDot == 0)
		return false;											// Sight line is parallel to the plane of the triangle.
	double d = m_V[0].Dot(n);
	double t = (d - eyeLoc.Dot(n))/rDot;
	if(t < 0)
		return false;											// Intersection is behind eyeLoc.
	else if(t == 0)
	{
		// Intersection is at eyeLoc.
		sightLoc = eyeLoc;
		return true;	
	}
	else if(t >= sightDist)
		return false;											// Intersection is at or beyond sightLoc.

	AuPoint intersect = eyeLoc + eyeVec*t;

	// Intersection is in the right direction. Is it in the triangle?
	if(!intersect.InTriangleFuzzy_3D(m_V[0], m_V[1], m_V[2], n, d_PlaneTol))
		return false;											// Intersection did not hit inside the triangle.

	// Intersection is on the ray inside the triangle.
	sightLoc = intersect;
	return true;
}

bool AuScTri::InSite(const AuPoint& loc) const
{
	// Test for 2D intersect with triangle.
	return loc.InTriangle(m_V[0], m_V[1], m_V[2]);
}

float AuScTri::SiteInfluence(const AuPoint& loc, AuPoint& siteLoc) const
{
	// Test for 2D intersect with triangle.
	if(!loc.InTriangle(m_V[0], m_V[1], m_V[2]))
		return 0;												// loc is not inside the 2D triangle.

	// Find the exact intersection with the plane of the triangle.
	AuPoint n(Normal());
	AuPlane fp(m_V[0], n);
	AuLine fl(loc.x,loc.y,loc.z, 0,0,1);						// Infinite line along the Z axis.
	if(!fp.LineIntersection(fl, siteLoc))
		return 0;												// The line does not intersect the plane.

	// Red glow is used to modulate terrain influence between vertices. Red glow reduces influence.
	float g1 = m_V[0].Glow().redF();
	if(loc.FuzzyEquals(m_V[0]))
		return 1 - g1;
	float g2 = m_V[1].Glow().redF();
	float g3 = m_V[2].Glow().redF();
	if(g1 == 0 && g2 == 0 && g3 == 0)
		return 1;												// No red glow results in max site influence.

	// Calculate influence from interpolated glow.
	// Find intersect on the opposite triangle edge from v0, with line through v0 and loc.
	double ow = m_V[1].Distance_2D(m_V[2]);
	if(ow > 0)
	{
		// Opposite edge is not collapsed in 2D.
		AuPoint oi;
		if(oi.SegmentIntersection_2D(m_V[0], loc, m_V[1], m_V[2], true, true))
		{
			// Intersection with opposite edge was found.
			double oid = m_V[1].Distance_2D(oi);
			double ofrac = oid / ow;	// Fraction of distance from v1 to v2 to the oi.
			float og = AuNoiseF::Linear_Interpolate(g2, g3, ofrac);
			double od = m_V[0].Distance_2D(oi);
			double ld = m_V[0].Distance_2D(loc);
			double lfrac = ld / od;		// Fraction of distance from v0 to oi to the loc.
			float fg = AuNoiseF::Linear_Interpolate(g1, og, lfrac);
			return 1.f - fg;
		}
	}

	// Interpolate glow to between two vertices.
	double ed = m_V[0].Distance_2D(m_V[1]);
	double ld = m_V[0].Distance_2D(loc);
	double lfrac = ld / ed;		// Fraction of distance from v0 to oi to the loc.
	float fg = AuNoiseF::Linear_Interpolate(g1, g2, lfrac);
	return 1.f - fg;
}

double AuScTri::Distance(const AuPoint& v) const
{
	// D: Oh the math.
	AuPoint ex(m_V[2] - m_V[0]);
	double xlen = ex.Length();
	AuPoint vx = ex/xlen;

	AuPoint ey(m_V[1] - m_V[0]);
	AuPoint vy(ey);
	vy.Unit();

	AuPoint vz = vx.Cross(vy);
	vz.Unit();

	vy = vz.Cross(vx);
	vy.Unit();

	// Triangle vertices relative to it's own plane.
	AuPoint v0;							// Vertex 1 is 0,0
	AuPoint v1(ey.Dot(vx), ey.Dot(vy));	// Vertex 2
	AuPoint v2(xlen, 0);				// Vertex 3 is on the X axis.

	// Search location relative to the plane of this triangle.
	AuPoint vec(v - m_V[0]);
	double x = vec.Dot(vx);
	double y = vec.Dot(vy);
	AuPoint vp(x,y);					// Search location in the plane of the triangle.

	// If the search location is inside the triangle (in 2D in its own plane), return the distance above/below the plane.
	if(vp.InTriangle(v0, v1, v2))
		return fabs(vec.Dot(vz));

	// Search location is outside the triangle so return the closest edge segment distance.
	double e0 = v.DistToSegment(m_V[0], m_V[1]);
	double e1 = v.DistToSegment(m_V[1], m_V[2]);
	double e2 = v.DistToSegment(m_V[2], m_V[0]);
	if(e0 < e1 && e0 < e2)
		return e0;
	else if(e1 < e2)
		return e1;
	else
		return e2;
}

void AuScTri::Divide(AuScTriList& pieces) const
{
	AuScPoint c(m_V[0]);
	c.m_N = Normal();

	AuScPoint ce1 = m_V[0].LinearInterpolate(m_V[1], .5);
	AuScPoint ce2 = m_V[1].LinearInterpolate(m_V[2], .5);
	AuScPoint ce3 = m_V[2].LinearInterpolate(m_V[0], .5);

	pieces.append(AuScTri(m_V[0], ce1, ce3));
	pieces.append(AuScTri(ce3, ce2, m_V[2]));
	pieces.append(AuScTri(ce1, ce2, ce3));
	pieces.append(AuScTri(ce1, m_V[1], ce2));
}

void AuScTri::TextureCylindrical(quint16 texRepsAtEquator, double texSize, const AuMatrix4x4& gridPlane)
{
	// Orient the plane so positive Z aligns with the grid plane's X axis.
	// The X axis is the easiest for the user to control in the designer.
	AuMatrix4x4 t(gridPlane);
	t.RotateY(PI/2);

	AuPoint v0 = t.UnProject(V0());
	AuPoint v1 = t.UnProject(V1());
	AuPoint v2 = t.UnProject(V2());

	// Triangle is a side.
	AuGlobeLL ll0, ll1, ll2;
	ll0.SetFromGlobeLocation(v0);
	ll1.SetFromGlobeLocation(v1);
	ll2.SetFromGlobeLocation(v2);

	// Wrap texture properly at the seam.
	double ll0long = ll0.Longitude();
	double ll1long = ll1.Longitude();
	double ll2long = ll2.Longitude();
	if(ll0.Longitude() > PI/2 && (ll1.Longitude() < -PI/2 || ll2.Longitude() < -PI/2))
		ll0long -= PI*2;
	if(ll1.Longitude() > PI/2 && (ll0.Longitude() < -PI/2 || ll2.Longitude() < -PI/2))
		ll1long -= PI*2;
	if(ll2.Longitude() > PI/2 && (ll0.Longitude() < -PI/2 || ll1.Longitude() < -PI/2))
		ll2long -= PI*2;

	// Calculate texture coordinates.
	float s0 = (float)((ll0long)/(PI*2) * texRepsAtEquator);
	float t0 = v0.z /= texSize;
	float s1 = (float)((ll1long)/(PI*2) * texRepsAtEquator);
	float t1 = v1.z /= texSize;
	float s2 = (float)((ll2long)/(PI*2) * texRepsAtEquator);
	float t2 = v2.z /= texSize;

	SetVertexTextureCoord(0, s0, t0);
	SetVertexTextureCoord(1, s1, t1);
	SetVertexTextureCoord(2, s2, t2);
}

void AuScTri::TextureProject(double texSize, const AuMatrix4x4& gridPlane)
{
	AuPoint v0 = gridPlane.UnProject(V0());
	v0 /= texSize;
	AuPoint v1 = gridPlane.UnProject(V1());
	v1 /= texSize;
	AuPoint v2 = gridPlane.UnProject(V2());
	v2 /= texSize;

	SetVertexTextureCoord(0, v0.x, v0.y);
	SetVertexTextureCoord(1, v1.x, v1.y);
	SetVertexTextureCoord(2, v2.x, v2.y);
}

void AuScTri::TextureSpherical(quint16 texRepsAtEquator, const AuMatrix4x4& gridPlane)
{
	AuPoint v0 = gridPlane.UnProject(V0());
	AuPoint v1 = gridPlane.UnProject(V1());
	AuPoint v2 = gridPlane.UnProject(V2());

	AuGlobeLL ll0, ll1, ll2;
	ll0.SetFromGlobeLocation(v0);
	ll1.SetFromGlobeLocation(v1);
	ll2.SetFromGlobeLocation(v2);

	// Wrap texture properly at the seam.
	double ll0long = ll0.Longitude();
	double ll1long = ll1.Longitude();
	double ll2long = ll2.Longitude();
	if(ll0.Longitude() > PI/2 && (ll1.Longitude() < -PI/2 || ll2.Longitude() < -PI/2))
		ll0long -= PI*2;
	if(ll1.Longitude() > PI/2 && (ll0.Longitude() < -PI/2 || ll2.Longitude() < -PI/2))
		ll1long -= PI*2;
	if(ll2.Longitude() > PI/2 && (ll0.Longitude() < -PI/2 || ll1.Longitude() < -PI/2))
		ll2long -= PI*2;

	// Calculate texture coordinates.
	float s0 = (float)((ll0long)/(PI*2) * texRepsAtEquator);
	float t0 = (float)((ll0.Latitude() + PIf/2)/PIf * texRepsAtEquator);
	float s1 = (float)((ll1long)/(PI*2) * texRepsAtEquator);
	float t1 = (float)((ll1.Latitude() + PIf/2)/PIf * texRepsAtEquator);
	float s2 = (float)((ll2long)/(PI*2) * texRepsAtEquator);
	float t2 = (float)((ll2.Latitude() + PIf/2)/PIf * texRepsAtEquator);

	SetVertexTextureCoord(0, s0, t0);
	SetVertexTextureCoord(1, s1, t1);
	SetVertexTextureCoord(2, s2, t2);
}

void AuScTri::TextureMove(const QPointF& vec)
{
	TextureMove(0, vec);
	TextureMove(1, vec);
	TextureMove(2, vec);
}

void AuScTri::TextureMove(int vertexIndex, const QPointF& vec)
{
	if(vertexIndex < 0 || vertexIndex > 2)
		return;

	float tx = m_V[vertexIndex].m_Tx + vec.x();
	float ty = m_V[vertexIndex].m_Ty + vec.y();

	SetVertexTextureCoord(vertexIndex, tx, ty);
}

void AuScTri::TextureRotate(int vertexIndex, const QPointF& pivot, double angle)
{
	if(vertexIndex < 0 || vertexIndex > 2)
		return;

	AuPoint tex(m_V[vertexIndex].m_Tx, m_V[vertexIndex].m_Ty);
	tex -= pivot;
	tex = tex.RotateZ(angle);
	tex += pivot;

	SetVertexTextureCoord(vertexIndex, (float)tex.x, (float)tex.y);
}

void AuScTri::TextureStretch(int vertexIndex, const QPointF& base, double scaleX, double scaleY)
{
	if(vertexIndex < 0 || vertexIndex > 2)
		return;

	AuPoint tex(m_V[vertexIndex].m_Tx, m_V[vertexIndex].m_Ty);
	tex -= base;
	tex.x *= scaleX;
	tex.y *= scaleY;
	tex += base;

	SetVertexTextureCoord(vertexIndex, (float)tex.x, (float)tex.y);
}

void AuScTri::EffectCylindrical(quint16 texRepsAtEquator, double texSize, const AuMatrix4x4& gridPlane)
{
	// Orient the plane so positive Z aligns with the grid plane's X axis.
	// The X axis is the easiest for the user to control in the designer.
	AuMatrix4x4 t(gridPlane);
	t.RotateY(PI/2);

	AuPoint v0 = t.UnProject(V0());
	AuPoint v1 = t.UnProject(V1());
	AuPoint v2 = t.UnProject(V2());

	// Triangle is a side.
	AuGlobeLL ll0, ll1, ll2;
	ll0.SetFromGlobeLocation(v0);
	ll1.SetFromGlobeLocation(v1);
	ll2.SetFromGlobeLocation(v2);

	// Wrap texture properly at the seam.
	double ll0long = ll0.Longitude();
	double ll1long = ll1.Longitude();
	double ll2long = ll2.Longitude();
	if(ll0.Longitude() > PI/2 && (ll1.Longitude() < -PI/2 || ll2.Longitude() < -PI/2))
		ll0long -= PI*2;
	if(ll1.Longitude() > PI/2 && (ll0.Longitude() < -PI/2 || ll2.Longitude() < -PI/2))
		ll1long -= PI*2;
	if(ll2.Longitude() > PI/2 && (ll0.Longitude() < -PI/2 || ll1.Longitude() < -PI/2))
		ll2long -= PI*2;

	// Calculate texture coordinates.
	float s0 = (float)((ll0long)/(PI*2) * texRepsAtEquator);
	float t0 = v0.z /= texSize;
	float s1 = (float)((ll1long)/(PI*2) * texRepsAtEquator);
	float t1 = v1.z /= texSize;
	float s2 = (float)((ll2long)/(PI*2) * texRepsAtEquator);
	float t2 = v2.z /= texSize;

	SetVertexEffectCoord(0, s0, t0);
	SetVertexEffectCoord(1, s1, t1);
	SetVertexEffectCoord(2, s2, t2);
}

void AuScTri::EffectProject(double texSize, const AuMatrix4x4& gridPlane)
{
	AuPoint v0 = gridPlane.UnProject(V0());
	v0 /= texSize;
	AuPoint v1 = gridPlane.UnProject(V1());
	v1 /= texSize;
	AuPoint v2 = gridPlane.UnProject(V2());
	v2 /= texSize;

	SetVertexEffectCoord(0, v0.x, v0.y);
	SetVertexEffectCoord(1, v1.x, v1.y);
	SetVertexEffectCoord(2, v2.x, v2.y);
}

void AuScTri::EffectSpherical(quint16 texRepsAtEquator, const AuMatrix4x4& gridPlane)
{
	AuPoint v0 = gridPlane.UnProject(V0());
	AuPoint v1 = gridPlane.UnProject(V1());
	AuPoint v2 = gridPlane.UnProject(V2());

	AuGlobeLL ll0, ll1, ll2;
	ll0.SetFromGlobeLocation(v0);
	ll1.SetFromGlobeLocation(v1);
	ll2.SetFromGlobeLocation(v2);

	// Wrap texture properly at the seam.
	double ll0long = ll0.Longitude();
	double ll1long = ll1.Longitude();
	double ll2long = ll2.Longitude();
	if(ll0.Longitude() > PI/2 && (ll1.Longitude() < -PI/2 || ll2.Longitude() < -PI/2))
		ll0long -= PI*2;
	if(ll1.Longitude() > PI/2 && (ll0.Longitude() < -PI/2 || ll2.Longitude() < -PI/2))
		ll1long -= PI*2;
	if(ll2.Longitude() > PI/2 && (ll0.Longitude() < -PI/2 || ll1.Longitude() < -PI/2))
		ll2long -= PI*2;

	// Calculate texture coordinates.
	float s0 = (float)((ll0long)/(PI*2) * texRepsAtEquator);
	float t0 = (float)((ll0.Latitude() + PIf/2)/PIf * texRepsAtEquator);
	float s1 = (float)((ll1long)/(PI*2) * texRepsAtEquator);
	float t1 = (float)((ll1.Latitude() + PIf/2)/PIf * texRepsAtEquator);
	float s2 = (float)((ll2long)/(PI*2) * texRepsAtEquator);
	float t2 = (float)((ll2.Latitude() + PIf/2)/PIf * texRepsAtEquator);

	SetVertexEffectCoord(0, s0, t0);
	SetVertexEffectCoord(1, s1, t1);
	SetVertexEffectCoord(2, s2, t2);
}

void AuScTri::EffectMove(const QPointF& vec)
{
	EffectMove(0, vec);
	EffectMove(1, vec);
	EffectMove(2, vec);
}

void AuScTri::EffectMove(int vertexIndex, const QPointF& vec)
{
	if(vertexIndex < 0 || vertexIndex > 2)
		return;

	float tx = m_V[vertexIndex].EffectTx() + vec.x();
	float ty = m_V[vertexIndex].EffectTy() + vec.y();

	SetVertexEffectCoord(vertexIndex, tx, ty);
}

void AuScTri::EffectRotate(int vertexIndex, const QPointF& pivot, double angle)
{
	if(vertexIndex < 0 || vertexIndex > 2)
		return;

	AuPoint tex(m_V[vertexIndex].EffectTx(), m_V[vertexIndex].EffectTy());
	tex -= pivot;
	tex = tex.RotateZ(angle);
	tex += pivot;

	SetVertexEffectCoord(vertexIndex, (float)tex.x, (float)tex.y);
}

void AuScTri::EffectStretch(int vertexIndex, const QPointF& base, double scaleX, double scaleY)
{
	if(vertexIndex < 0 || vertexIndex > 2)
		return;

	AuPoint tex(m_V[vertexIndex].EffectTx(), m_V[vertexIndex].EffectTy());
	tex -= base;
	tex.x *= scaleX;
	tex.y *= scaleY;
	tex += base;

	SetVertexEffectCoord(vertexIndex, (float)tex.x, (float)tex.y);
}

double AuScTri::Area() const
{
	// Base of triangle.
	AuPoint vx = m_V[2] - m_V[0];
	double base = vx.Length();
	if(base == 0)
		return 0;
	vx /= base;							// Unit vector X axis of triangle plane.

	// Apex of triangle.
	AuPoint va = m_V[1] - m_V[0];
	double la = va.Length();
	if(la == 0)
		return 0;
	AuPoint vy = va / la;				// Unit vector Y axis of triangle plane, not perpendicular to vx yet.

	// Calculate normal to triangle.
	AuPoint vz = vx.Cross(vy);
	vz.Unit();							// Unit vector Z axis of triangle plane.

	// Make the Y axis perpendicular to the X axis.
	vy = vz.Cross(vx);
	vy.Unit();

	// Calculate height of the triangle at the apex, perpendicular above the base.
	double ht = fabs(va.Dot(vy));

	// Area of a triangle is 1/2 * base * ht.
	return (ht * base) / 2;
}

void AuScTri::MorphX(const AuMatrix4x4& fromM, const AuMatrix4x4& toM, double morphDistance)
{
	m_V[0].MorphX(fromM, toM, morphDistance);
	m_V[1].MorphX(fromM, toM, morphDistance);
	m_V[2].MorphX(fromM, toM, morphDistance);
	RecalcNormal();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// qHash is needed to use AuScSelectInfo in a QList in Windows.
unsigned int qHash(const AuScTri& key)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << key;
	return qHash(ba);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScPoint AuScTriList::VertexAt(const AuPoint& v) const
{
	for(AuScTriList::const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		if((*ti).V0().FuzzyEquals(v))
			return (*ti).V0();
		if((*ti).V1().FuzzyEquals(v))
			return (*ti).V1();
		if((*ti).V2().FuzzyEquals(v))
			return (*ti).V2();
	}
#if !defined(QT_NO_DEBUG)
	qWarning() << "AuScTriList::VertexAt not found at" << v;
#endif
	return AuScPoint();
}

void AuScTriList::EdgeIntersections(const AuLine& line, AuPointArray& intersections) const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
		(*ti).EdgeIntersections(line, intersections);
}

bool AuScTriList::Collapsed() const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
		if(!(*ti).Collapsed())
			return false;
	return true;
}

bool AuScTriList::ColinearEdge(const AuScTriList& tris) const
{
	for(AuScTriList::const_iterator mi = cbegin(); mi != cend(); mi++)
	{
		for(AuScTriList::const_iterator ti = tris.cbegin(); ti != tris.cend(); ti++)
		{
			if((*mi).ColinearEdge(*ti))
				return true;
		}
	}
	return false;
}

void AuScTriList::Divide()
{
	AuScTriList result;
	for(AuScTriList::const_iterator mi = cbegin(); mi != cend(); mi++)
		(*mi).Divide(result);
	*this = result;
}

void AuScTriList::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris, quint8 textureIndex,
		const QColor& paint, float reflectivity, float shine, float emission, float opacity, SURFACE_EFFECT effect) const
{
	for(AuScTriList::const_iterator mi = cbegin(); mi != cend(); mi++)
	{
		AuScTriRender tri(*mi, textureIndex, paint, reflectivity, shine, emission, opacity, effect);
		if(opacity == 1)
			opaqueTris.insert(effect, tri);
		else
			transparentTris.insert(effect, tri);
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuScTriRenderList::Insert(const AuScTriRenderList& l)
{
	for(const_iterator ti = l.cbegin(); ti != l.cend(); ti++)
		insert(ti.key(), *ti);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//static
AuScFace* AuScFace::Create(quint8 type)
{
	if(type == AuScFaceTri::TypeStatic())
		return new AuScFaceTri;
	else if(type == AuScFaceQuad::TypeStatic())
		return new AuScFaceQuad;
	else if(type == AuScFacePolygon::TypeStatic())
		return new AuScFacePolygon;

	qWarning() << "error: AuScFace::Create invalid type" << type;
	return 0;
}

void AuScFace::operator>>(QDataStream& out) const
{
	quint8 texIndex = m_TexIndex;
	quint8 hasWhat = 0;
	if(m_ColorTheme < THEME_Count)
		hasWhat |= 0x01;
	if(texIndex)
		hasWhat |= 0x02;
	if(m_Opacity != 1)
		hasWhat |= 0x04;
	if(m_Paint.alpha())
		hasWhat |= 0x08;
	if(m_Reflectivity != 0)
		hasWhat |= 0x10;
	if(m_SelfIllumination != 0)
		hasWhat |= 0x20;
	if(m_Shine != 0)
		hasWhat |= 0x40;
	if(m_Effect != EFF_None)
		hasWhat |= 0x80;

	out << hasWhat;

	if(hasWhat & 0x01)
		out << m_ColorTheme;

	if(hasWhat & 0x02)
		out << texIndex;

	if(hasWhat & 0x04)
		out << m_Opacity;

	if(hasWhat & 0x08)
		out << m_Paint;

	if(hasWhat & 0x10)
		out << m_Reflectivity;

	if(hasWhat & 0x20)
		out << m_SelfIllumination;

	if(hasWhat & 0x40)
		out << m_Shine;

	if(hasWhat & 0x80)
		out << m_Effect;
}

void AuScFace::operator<<(QDataStream& in)
{
	quint8 hasWhat;
	in >> hasWhat;

	if(hasWhat & 0x01)
		in >> m_ColorTheme;
	else
		m_ColorTheme = THEME_Count;

	if(hasWhat & 0x02)
	{
		in >> m_TexIndex;
		m_TexIndex &= 0x0f;	// Mask off old effect bits, which could only exist in my own test data.
	}
	else
		m_TexIndex = 0;

	if(hasWhat & 0x04)
		in >> m_Opacity;
	else
		m_Opacity = 1;

	if(hasWhat & 0x08)
		in >> m_Paint;
	else
		m_Paint = QColor(0,0,0,0);

	if(hasWhat & 0x10)
		in >> m_Reflectivity;
	else
		m_Reflectivity = 0;

	if(hasWhat & 0x20)
		in >> m_SelfIllumination;
	else
		m_SelfIllumination = 0;

	if(hasWhat & 0x40)
		in >> m_Shine;
	else
		m_Shine = 0;

	if(hasWhat & 0x80)
		in >> m_Effect;
	else
		m_Effect = EFF_None;
}

bool AuScFace::ColinearTriangleEdge(const AuScFace& face) const
{
	AuScTriList mytris;
	Triangles(mytris);

	AuScTriList ftris;
	face.Triangles(ftris);

	return mytris.ColinearEdge(ftris);
}

bool AuScFace::Cross(AuPointArray& path, AuSegment& exitEdge) const
{
	if(path.empty())
		return false;

	AuSegmentArray segs;
	EdgeSegments(segs);
	for(AuSegmentArray::const_iterator si = segs.cbegin(); si != segs.cend(); si++)
		if(path.last().IsOnSegment((*si).m_fLoc1, (*si).m_fLoc2))
			if(Cross(path, *si, exitEdge))
				return true;

	return false;
}

bool AuScFace::SamePlace(const AuScFace& face) const
{
	int vcount = VertexCount();
	if(vcount != face.VertexCount())
		return false;

	for(int vi=0; vi<vcount; vi++)
		if(!Vertex(vi).FuzzyEquals(face.Vertex(vi)))
			return false;

	return true;
}

bool AuScFace::InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, bool findVertices, bool findEdges,
		bool findFaces) const
{
	double zNear = 0;	//mvp.PerspectiveZNear();

	if(findFaces)
	{
		AuPoint floc = mvp.Project(FaceLocation());
		if(floc.z > zNear && pixelRect.contains(QPointF(floc.x, floc.y)))
			return true;
	}

	if(findVertices)
	{
		int verts = VertexCount();
		for(int v=0; v<verts; v++)
		{
			AuPoint vloc = mvp.Project(Vertex(v));
			if(vloc.z > zNear && pixelRect.contains(QPointF(vloc.x, vloc.y)))
				return true;
		}
	}

	if(findEdges)
	{
		int segs = Edges();
		AuSegment seg;
		for(int s=0; s<segs; s++)
		{
			EdgeSegment(s, seg);
			seg.Transform(mvp);
			if(seg.m_fLoc1.z > zNear && pixelRect.contains(QPointF(seg.m_fLoc1.x, seg.m_fLoc1.y)))
				return true;
			if(seg.m_fLoc2.z > zNear && pixelRect.contains(QPointF(seg.m_fLoc2.x, seg.m_fLoc2.y)))
				return true;
		}
	}

	return false;
}

bool AuScFace::Slice(const AuScFace& cutter, AuScFaceList& nufaces) const
{
	AuPoint fn(FaceNormal());
	AuPoint cfn(cutter.FaceNormal());

	// Find the line intersection of the planes of the two faces.
	AuPlane p(Vertex(0), fn);
	AuPlane cp(cutter.Vertex(0), cfn);
	AuLine il;							// Intersection of the planes of the faces.
	if(!p.PlaneIntersection(cp, il))
		return false;					// Planes of the faces are parallel.

	// Get all the cutter's edge intersections with the line.
	AuPointArray cutterints;
	cutter.EdgeIntersections(il, cutterints);
	if(cutterints.empty())
		return false;					// Cutter does not intersect the plane intersection line.

	// Get all my edge intersections with the line.
	AuPointArray myints;
	EdgeIntersections(il, myints);
	if(myints.empty())
		return false;					// I do not intersect the plane intersection line.

	// Determine my intersection distances along the plane intersection line.
	double minldot = 0;
	double maxldot = 0;
	for(AuPointArray::const_iterator mi = myints.cbegin(); mi != myints.cend(); mi++)
	{
		AuPoint v(*mi - il.U);
		double dot = v.Dot(il.V);	// Distance along the line from it's origin, positive or negative.
		if(mi == myints.cbegin())
		{
			minldot = dot;
			maxldot = dot;
		}
		else
		{
			if(minldot > dot)
				minldot = dot;
			if(maxldot < dot)
				maxldot = dot;
		}
	}

	// Determine cutter's intersection distances along the plane intersection line.
	double mincdot = 0;
	double maxcdot = 0;
	for(AuPointArray::const_iterator mi = cutterints.cbegin(); mi != cutterints.cend(); mi++)
	{
		AuPoint v(*mi - il.U);
		double dot = v.Dot(il.V);	// Distance along the line from it's origin, positive or negative.
		if(mi == cutterints.cbegin())
		{
			mincdot = dot;
			maxcdot = dot;
		}
		else
		{
			if(mincdot > dot)
				mincdot = dot;
			if(maxcdot < dot)
				maxcdot = dot;
		}
	}

	// Test to see if the faces could possibly touch each other.
	if(GreaterThanOrFuzzyEquals(minldot, maxcdot) ||
			LessThanOrFuzzyEquals(maxldot, mincdot))
		return false;

	// Close enough. Start slicing.
	AuScFaceList slices;
	Slice(il, slices);
	if(slices.empty())
		return false;

	// If only one face resulted, then I didn't get sliced.
	if(slices.size() == 1)
		return false;

	nufaces.append(slices);
	nufaces.RecalcExtents();
	slices.clear();
	return true;
}

bool AuScFace::SharesUncutEdge(const AuScFace& f, const AuPointArray& cutLine) const
{
	AuSegmentArray mysegs;
	EdgeSegments(mysegs);
	if(mysegs.empty())
		return false;

	AuSegmentArray fsegs;
	f.EdgeSegments(fsegs);
	if(fsegs.empty())
		return false;

	// Look for shared edges.
	for(AuSegmentArray::const_iterator msi = mysegs.cbegin(); msi != mysegs.cend(); msi++)
	{
		if(fsegs.Contains(*msi))
		{
			// A common edge segment was found.
			// Skip it if it is on the cut line.
			bool onCutLine = false;
			AuPoint pc(cutLine.first());
			for(AuPointArray::const_iterator ci = cutLine.cbegin()+1; ci != cutLine.cend(); ci++)
			{
				AuSegment cseg(pc, *ci);
				if((*msi).Overlaps(cseg))
				{
					onCutLine = true;
					break;
				}

				pc = *ci;
			}
			if(!onCutLine)
				return true;	// Found a shared edge that is not touched by the cut line.
		}
	}
	return false;
}

bool AuScFace::Cut(const AuPointArray& cutLine, AuScFaceList& result)
{
	// Build a list of slices, starting with my whole face.
	AuScFaceList slices;
	slices.AddFace(*this);

	// Cut the slices into bits with the cut line.
	AuPoint pc = cutLine.first();
	for(AuPointArray::const_iterator ci = cutLine.cbegin()+1; ci != cutLine.cend(); ci++)
	{
		// Cut all the faces using the current cut line.
		AuScFaceList nuslices;
		AuLine cut(pc, *ci);
		for(AuScFaceList::const_iterator si = slices.cbegin(); si != slices.cend(); si++)
		{
			if((*si)->TestSlice(pc, *ci))
			{
				AuScFaceList pieces;
				(*si)->Slice(cut, pieces);
				nuslices.append(pieces);
				pieces.clear();
			}
			else
				nuslices.append((*si)->Clone());
		}

		// Update the faces with the slices, to be cut by the next cut segment.
		slices = nuslices;
		pc = *ci;
	}

	//qDebug() << "AuScFace::Cut slices" << slices.size();
	// If we end up with only one slice, we didn't get cut.
	if(slices.size() == 1)
		return false;

	// Combine sliced faces to make as few as possible in the result.
	while(!slices.empty())
	{
		AuScFace* f = slices.takeFirst();

		bool anyJoined;
		do
		{
			anyJoined = false;

			// Join 'f' with all other faces that share an edge that is not on the cut line.
			AuScFaceList::iterator ji = slices.begin();
			while(ji != slices.end())
			{
				if(f->SharesUncutEdge(**ji, cutLine))
				{
					// Join ji to f.
					AuScFace* nf = f->Join(**ji);
					if(nf)
					{
						// Face ji joined with f to produce nf.
						//qDebug() << "f" << *f;
						//qDebug() << "ji" << **ji;
						//qDebug() << "nf" << *nf;
						delete *ji;
						delete f;
						f = nf;
						ji = slices.erase(ji);
						anyJoined = true;
					}
					else
						ji++;	// They could not join.
				}
				else
					ji++;
			}
		}
		while(anyJoined);

		result.append(f);
	}
	return true;
}

bool AuScFace::Punch(const AuPointArray& cutBnd, AuScFaceList& result)
{
	// Build a list of slices, starting with my whole face.
	AuScFaceList slices;
	slices.AddFace(*this);

	// Cut the slices into bits with the cut line.
	AuPoint pc = cutBnd.first();
	for(AuPointArray::const_iterator ci = cutBnd.cbegin()+1; ci != cutBnd.cend(); ci++)
	{
		// Cut all the faces using the current cut line.
		AuScFaceList nuslices;
		AuLine cut(pc, *ci);
		for(AuScFaceList::const_iterator si = slices.cbegin(); si != slices.cend(); si++)
		{
			if((*si)->TestSlice(pc, *ci))
			{
				AuScFaceList pieces;
				(*si)->Slice(cut, pieces);
				nuslices.append(pieces);
				pieces.clear();
			}
			else
				nuslices.append((*si)->Clone());
		}

		// Update the faces with the slices, to be cut by the next cut segment.
		slices = nuslices;
		pc = *ci;
	}

	//qDebug() << "AuScFace::Cut slices" << slices.size();
	// If we end up with only one slice, we didn't get cut.
	if(slices.size() == 1)
		return false;

	// Make a matrix that can flip everything down to the XY plane for boundary testing.
	AuMatrix4x4 myPlaneM(FacePlane());
	AuPointArray bnd2d(cutBnd);
	bnd2d.Transform(myPlaneM.Inverse());

	AuScFaceList inside, outside;
	while(!slices.empty())
	{
		AuScFace* f = slices.takeFirst();
		AuPoint floc2d = myPlaneM.UnProject(f->FaceLocation());
		if(bnd2d.PointInBoundary(floc2d))
			inside.append(f);
		else
			outside.append(f);
	}

	inside.JoinFaces();
	outside.JoinFaces();

	while(!inside.empty())
	{
		AuScFace* f = inside.takeFirst();
		result.append(f);
	}

	while(!outside.empty())
	{
		AuScFace* f = outside.takeFirst();
		result.append(f);
	}

	return true;
}

AuMatrix4x4 AuScFace::FacePlane() const
{
	AuPoint n(FaceNormal());
	AuPoint x;
	n.ArbitraryAxis(x);
	AuPoint y(n.Cross(x));
	y.Unit();
	return AuMatrix4x4(Vertex(0), x, y);
}

void AuScFace::TextureUnwrap(quint8 texIndex, double texSize, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	Transform(gridPlane.Inverse());

	// Get the plane matrix for the face.
	AuMatrix4x4 fplane;
	fplane.Rotate(FacePlane().GetRotation());
	fplane.RotateX(PI);											// Flip vertically.

	if(gridAlign)
	{
		// Align the plane to a grid axis.
		double zrot;
		switch(alignAxis)
		{
		case 0:	// x
			// Grid X vector in the plane of the face.
			zrot = fplane.UnProjectVector(AuPoint(1,0,0)).ZRotation() - PI/2;
			break;
		case 1:	// y
			// Grid Y vector in the plane of the face.
			zrot = fplane.UnProjectVector(AuPoint(0,1,0)).ZRotation() - PI/2;
			break;
		case 2:	// z
			// Grid Z vector in the plane of the face.
			zrot = fplane.UnProjectVector(AuPoint(0,0,1)).ZRotation() - PI/2;
			break;
		default:
			zrot = 0;
			break;
		}
		if(zrot != 0)
			fplane.RotateZ(zrot);
	}

	// Project the texture using the plane.
	TextureProject(texIndex, texSize, fplane);

	Transform(gridPlane);
}

void AuScFace::EffectUnwrap(SURFACE_EFFECT effect, double texSize, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	Transform(gridPlane.Inverse());

	// Get the plane matrix for the face.
	AuMatrix4x4 fplane;
	fplane.Rotate(FacePlane().GetRotation());
	fplane.RotateX(PI);											// Flip vertically.

	if(gridAlign)
	{
		// Align the plane to a grid axis.
		double zrot;
		switch(alignAxis)
		{
		case 0:	// x
			// Grid X vector in the plane of the face.
			zrot = fplane.UnProjectVector(AuPoint(1,0,0)).ZRotation() - PI/2;
			break;
		case 1:	// y
			// Grid Y vector in the plane of the face.
			zrot = fplane.UnProjectVector(AuPoint(0,1,0)).ZRotation() - PI/2;
			break;
		case 2:	// z
			// Grid Z vector in the plane of the face.
			zrot = fplane.UnProjectVector(AuPoint(0,0,1)).ZRotation() - PI/2;
			break;
		default:
			zrot = 0;
			break;
		}
		if(zrot != 0)
			fplane.RotateZ(zrot);
	}

	// Project the texture using the plane.
	EffectProject(effect, texSize, fplane);

	Transform(gridPlane);
}

bool AuScFace::CommonEdgeVertices(const AuScFace& f, AuScPoint& v1, AuScPoint& v2) const
{
	AuSegmentArray mysegs;
	EdgeSegments(mysegs);
	if(mysegs.empty())
		return false;

	AuSegmentArray fsegs;
	f.EdgeSegments(fsegs);
	if(fsegs.empty())
		return false;

	// Look for shared edges.
	for(AuSegmentArray::const_iterator msi = mysegs.cbegin(); msi != mysegs.cend(); msi++)
	{
		if(fsegs.Contains(*msi))
		{
			// A common edge segment was found.
			int myv1 = VertexAt((*msi).m_fLoc1);
			int myv2 = VertexAt((*msi).m_fLoc2);
			v1 = Vertex(myv1);
			v2 = Vertex(myv2);
			return true;
		}
	}
	return false;
}

bool AuScFace::CommonEdgeVerticesHorizontal(const AuScFace& f, AuScPoint& v1, AuScPoint& v2) const
{
	AuSegmentArray mysegs;
	EdgeSegments(mysegs);
	if(mysegs.empty())
		return false;

	AuSegmentArray fsegs;
	f.EdgeSegments(fsegs);
	if(fsegs.empty())
		return false;

	AuPoint fn(FaceNormal());
	bool is2D = fn.FuzzyEquals(AuPoint(0,0,1)) || fn.FuzzyEquals(AuPoint(0,0,-1));

	// Look for shared edges.
	for(AuSegmentArray::const_iterator msi = mysegs.cbegin(); msi != mysegs.cend(); msi++)
	{
		// We are only looking for horizontal connection.
		// Skip my edge segments that might make a vertical connection. Horizontal lines form vertical connections.
		if(is2D)
		{
			// I am flat. Discard my edges with more than 45 degrees of slope.
			AuPoint evec((*msi).m_fLoc2 - (*msi).m_fLoc1);
			double zrot = fabs(evec.ZRotation());
			if(zrot < PI/4 || zrot > PI*3/4)
				continue;	// Skip line that is too horizontal.
		}
		else
		{
			// I am not flat.
			AuPoint evec((*msi).m_fLoc2 - (*msi).m_fLoc1);
			double incline = fabs(evec.PolarIncline());
			if(incline < PI/4)
				continue;	// Skip line that is too horizontal.
		}

		if(fsegs.Contains(*msi))
		{
			// A common edge segment was found.
			int myv1 = VertexAt((*msi).m_fLoc1);
			int myv2 = VertexAt((*msi).m_fLoc2);
			v1 = Vertex(myv1);
			v2 = Vertex(myv2);
			return true;
		}
	}
	return false;
}

bool AuScFace::CommonEdgeVerticesVertical(const AuScFace& f, AuScPoint& v1, AuScPoint& v2) const
{
	AuSegmentArray mysegs;
	EdgeSegments(mysegs);
	if(mysegs.empty())
		return false;

	AuSegmentArray fsegs;
	f.EdgeSegments(fsegs);
	if(fsegs.empty())
		return false;

	AuPoint fn(FaceNormal());
	bool is2D = fn.FuzzyEquals(AuPoint(0,0,1)) || fn.FuzzyEquals(AuPoint(0,0,-1));

	// Look for shared edges.
	for(AuSegmentArray::const_iterator msi = mysegs.cbegin(); msi != mysegs.cend(); msi++)
	{
		// We are only looking for vertical connection.
		// Skip my edge segments that might make a horizontal connection. Vertical lines form horizontal connections.
		if(is2D)
		{
			// I am flat. Discard my edges with more than 45 degrees of slope.
			AuPoint evec((*msi).m_fLoc2 - (*msi).m_fLoc1);
			double zrot = fabs(evec.ZRotation());
			if(zrot >= PI/4 && zrot <= PI*3/4)
				continue;	// Skip line that is too vertical.
		}
		else
		{
			// I am not flat.
			AuPoint evec((*msi).m_fLoc2 - (*msi).m_fLoc1);
			double incline = fabs(evec.PolarIncline());
			if(incline >= PI/4)
				continue;	// Skip line that is too vertical.
		}

		if(fsegs.Contains(*msi))
		{
			// A common edge segment was found.
			int myv1 = VertexAt((*msi).m_fLoc1);
			int myv2 = VertexAt((*msi).m_fLoc2);
			v1 = Vertex(myv1);
			v2 = Vertex(myv2);
			return true;
		}
	}
	return false;
}

quint64 AuScFace::MemoryUsage() const
{
	quint64 mem = 0;
	mem += sizeof(m_ColorTheme);
	mem += sizeof(m_Effect);
	mem += sizeof(m_Opacity);
	mem += sizeof(m_Paint);
	mem += sizeof(m_Reflectivity);
	mem += sizeof(m_SelfIllumination);
	mem += sizeof(m_Shine);
	mem += sizeof(m_TexIndex);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScFaceTri::AuScFaceTri()
{
}

AuScFaceTri::AuScFaceTri(const AuScPoint& v0, const AuScPoint& v1, const AuScPoint& v2) : m_Tri(v0, v1, v2)
{
}

AuScFaceTri::AuScFaceTri(const AuScFace& c, const AuScPoint& v0, const AuScPoint& v1, const AuScPoint& v2) :
	AuScFace(c), m_Tri(v0, v1, v2)
{
}

AuScFaceTri::AuScFaceTri(const AuScFace& c, const AuScTri& t) : AuScFace(c), m_Tri(t)
{
}

bool AuScFaceTri::Collapsed() const
{
	return m_Tri.Collapsed();
}

void AuScFaceTri::EdgeIntersections(const AuLine& line, AuPointArray& intersections) const
{
	m_Tri.EdgeIntersections(line, intersections);
}

void AuScFaceTri::Transform(const AuMatrix4x4& t, quint32 firstVertex, const QSet<quint32>& vertexIndices,
		quint32 firstEdge, const QSet<quint32>& edgeIndices, bool faceIsSelected)
{
	if(faceIsSelected || vertexIndices.contains(firstVertex) ||
			edgeIndices.contains(firstEdge+2) || edgeIndices.contains(firstEdge))
		m_Tri.TransformVertex(t, 0);
	if(faceIsSelected || vertexIndices.contains(firstVertex+1) ||
			edgeIndices.contains(firstEdge) || edgeIndices.contains(firstEdge+1))
		m_Tri.TransformVertex(t, 1);
	if(faceIsSelected || vertexIndices.contains(firstVertex+2) ||
			edgeIndices.contains(firstEdge+1) || edgeIndices.contains(firstEdge+2))
		m_Tri.TransformVertex(t, 2);
}

void AuScFaceTri::operator>>(QDataStream& out) const
{
	AuScFace::operator>>(out);
	out << m_Tri;
}

void AuScFaceTri::operator<<(QDataStream& in)
{
	AuScFace::operator<<(in);
	in >> m_Tri;
}

bool AuScFaceTri::Cross(AuPointArray& path, const AuSegment& entryEdge, AuSegment& exitEdge) const
{
	Q_UNUSED(path);
	Q_UNUSED(entryEdge);
	Q_UNUSED(exitEdge);
	return false;
	/*
	// If the point is at a vertex, don't do anything.
	if(path.last().FuzzyEquals(entryEdge.m_fLoc1) || path.last().FuzzyEquals(entryEdge.m_fLoc2))
		return false;

	AuSegmentArray edges;
	EdgeSegments(edges);
	double frac = 0;
	if(entryEdge.IsSegment(edges[0]))
	{
		double sl = entryEdge.Length();
		if(sl == 0)
			return false;
		exitEdge.m_fLoc1 = Vertex(1);
		exitEdge.m_fLoc2 = Vertex(2);

		double pl = path.last().Distance(Vertex(0));
		frac = pl / sl;
	}
	else if(entryEdge.IsSegment(edges[1]))
	{
		double sl = entryEdge.Length();
		if(sl == 0)
			return false;
		exitEdge.m_fLoc1 = Vertex(2);
		exitEdge.m_fLoc2 = Vertex(0);

		double pl = path.last().Distance(Vertex(1));
		frac = pl / sl;
	}
	else if(entryEdge.IsSegment(edges[2]))
	{
		double sl = entryEdge.Length();
		if(sl == 0)
			return false;
		exitEdge.m_fLoc1 = Vertex(0);
		exitEdge.m_fLoc2 = Vertex(1);

		double pl = path.last().Distance(Vertex(2));
		frac = pl / sl;
	}

	if(frac == 0)
		return false;

	AuPoint a = exitEdge.m_fLoc2 + (exitEdge.m_fLoc1 - exitEdge.m_fLoc2) * frac;
	//AuPoint a = exitEdge.m_fLoc1 + (exitEdge.m_fLoc2 - exitEdge.m_fLoc1) * frac;
	path.append(a);
	return true;
	*/
}

double AuScFaceTri::Distance(const AuPoint& v) const
{
	return m_Tri.Distance(v);
}

AuScFace* AuScFaceTri::Join(const AuScFace& f) const
{
	// If 'f' is collapsed, completely absorb it.
	if(f.Collapsed())
		return Clone();					// Absorb 'f' by returning a clone of myself without changes.

	// Let the higher order faces handle joining triangles like me to them.
	if(f.VertexCount() != 3)
		return f.Join(*this);

	// If our planes are not parallel, we cannot join.
	AuPoint n(FaceNormal());
	if(!n.IsParallel(f.FaceNormal()))
		return 0;						// Planes are not parallel.

	// Compare height difference between our planes.
	double dist = (f.Vertex(0) - Vertex(0)).Dot(n);
	if(FuzzyGreaterThanFactor(dist, 0, d_InTrianglePlaneTol))
		return 0;						// Planes are too far apart.

	// Check for the shared vertices.
	AuPointArray shared;
	for(int vi=0; vi<3; vi++)
		for(int fvi=0; fvi<3; fvi++)
			if(Vertex(vi).FuzzyEquals(f.Vertex(fvi)))
				shared.AddIfUnique(Vertex(vi));

	// If less that two vertices are shared, 'f' cannot be joined.
	if(shared.size() < 2)
		return 0;

	// If shared result is three vertices, 'f' is completely absorbed by me.
	if(shared.size() == 3)
		return Clone();					// Absorb 'f' by returning a clone of myself without changes.

	// Two vertices are shared. Join along an edge.
	// Create a new triangle for f, with the non-shared vertex second.
	AuScTri t2;
	if(!shared.DoesContain(f.Vertex(0)))
	{
		// Vertex 0 of f is not on the shared side.
		if(ContainsFuzzy(f.Vertex(0)))
			return Clone();				// Vertex falls inside my triangle so 'f' is absorbed.
		t2 = AuScTri(f.Vertex(2), f.Vertex(0), f.Vertex(1));
	}
	else if(!shared.DoesContain(f.Vertex(1)))
	{
		// Vertex 1 of f is not on the shared side.
		if(ContainsFuzzy(f.Vertex(1)))
			return Clone();				// Vertex falls inside my triangle so 'f' is absorbed.
		t2 = AuScTri(f.Vertex(0), f.Vertex(1), f.Vertex(2));
	}
	else
	{
		// Vertex 2 of f is not on the shared side.
		if(ContainsFuzzy(f.Vertex(2)))
			return Clone();				// Vertex falls inside my triangle so 'f' is absorbed.
		t2 = AuScTri(f.Vertex(1), f.Vertex(2), f.Vertex(0));
	}

	// Create a new triangle for me, with the non-shared vertex second.
	AuScTri t1;
	if(!shared.DoesContain(Vertex(0)))
	{
		// My vertex 0 is not on the shared side.
		if(f.ContainsFuzzy(Vertex(0)))
			return f.Clone();			// My vertex falls inside 'f' so I am absorbed.
		t1 = AuScTri(Vertex(2), Vertex(0), Vertex(1));
	}
	else if(!shared.DoesContain(Vertex(1)))
	{
		// My vertex 1 is not on the shared side.
		if(f.ContainsFuzzy(Vertex(1)))
			return f.Clone();			// My vertex falls inside 'f' so I am absorbed.
		t1 = AuScTri(Vertex(0), Vertex(1), Vertex(2));
	}
	else
	{
		// My vertex 2 is not on the shared side.
		if(f.ContainsFuzzy(Vertex(2)))
			return f.Clone();			// My vertex falls inside 'f' so I am absorbed.
		t1 = AuScTri(Vertex(1), Vertex(2), Vertex(0));
	}

	// Return a triangle if either t1 or t2 collapsed.
	if(t2.Collapsed())
		return new AuScFaceTri(*this, t1);
	else if(t1.Collapsed())
		return new AuScFaceTri(*this, t2);

	// Do the two triangles form a triangle?
	if(t1.V0().IsOnSegment(t1.V1(), t2.V1()))
	{
		// It's a triangle because t1's vertex 0 is on the segement from t2 vertex 1 to t1 vertex 1.
		AuScTri t = AuScTri(t2.V1(), t1.V1(), t1.V2());
		return new AuScFaceTri(*this, t);
	}
	else if(t2.V0().IsOnSegment(t1.V1(), t2.V1()))
	{
		// It's a triangle because t2's vertex 0 is on the segement from t1 vertex 1 to t2 vertex 1.
		AuScTri t = AuScTri(t1.V1(), t2.V1(), t2.V2());
		return new AuScFaceTri(*this, t);
	}

	// Return a quad made from t1 and t2.
	return new AuScFaceQuad(*this, t1, t2);
}

bool AuScFaceTri::ContainsFuzzy(const AuPoint& v) const
{
	return m_Tri.ContainsFuzzy(v);
}

bool AuScFaceTri::TestSlice(const AuPoint& seg1, const AuPoint& seg2) const
{
	// Test seg ends for being inside my triangle.
	AuPoint n(m_Tri.Normal());
	if(seg1.InTriangleFuzzy_3D(m_Tri.V0(), m_Tri.V1(), m_Tri.V2(), n, d_TriTol))
		return true;
	if(seg2.InTriangleFuzzy_3D(m_Tri.V0(), m_Tri.V1(), m_Tri.V2(), n, d_TriTol))
		return true;

	// Test intersect with my edge segments.
	AuPoint eint;
	bool ehit = eint.SegmentIntersection(m_Tri.V0(), m_Tri.V1(), seg1, seg2, false, false, d_TriTol);
	if(ehit && !eint.FuzzyEquals(m_Tri.V0()) && !eint.FuzzyEquals(m_Tri.V1()))
		return true;

	ehit = eint.SegmentIntersection(m_Tri.V1(), m_Tri.V2(), seg1, seg2, false, false, d_TriTol);
	if(ehit && !eint.FuzzyEquals(m_Tri.V1()) && !eint.FuzzyEquals(m_Tri.V2()))
		return true;

	ehit = eint.SegmentIntersection(m_Tri.V2(), m_Tri.V0(), seg1, seg2, false, false, d_TriTol);
	if(ehit && !eint.FuzzyEquals(m_Tri.V2()) && !eint.FuzzyEquals(m_Tri.V0()))
		return true;

	//qDebug() << "AuScFaceTri::TestSlice" << m_Tri.V0() << m_Tri.V1() << m_Tri.V2() << "seg" << seg1 << seg2
		//<< seg1.DistToSegment(m_Tri.V0(), m_Tri.V1())
		//<< seg1.DistToSegment(m_Tri.V1(), m_Tri.V2())
		//<< seg1.DistToSegment(m_Tri.V2(), m_Tri.V0())
		//<< seg2.DistToSegment(m_Tri.V0(), m_Tri.V1())
		//<< seg2.DistToSegment(m_Tri.V1(), m_Tri.V2())
		//<< seg2.DistToSegment(m_Tri.V2(), m_Tri.V0());
	return false;
}

void AuScFaceTri::Slice(const AuLine& cutLine, AuScFaceList& nufaces) const
{
	AuScTriList pieces;
	if(m_Tri.Slice(cutLine, pieces) && pieces.size() > 1)
	{
		// Make faces from the pieces.
		if(pieces.size() == 3)
		{
			// Make a quad from the first two triangles if neither of them collapsed.
			if(pieces[0].Collapsed())
				nufaces.AddFace(AuScFaceTri(*this, pieces[1]));
			else if(pieces[1].Collapsed())
				nufaces.AddFace(AuScFaceTri(*this, pieces[0]));
			else
				nufaces.AddFace(AuScFaceQuad(*this, pieces[0], pieces[1]));

			nufaces.AddFace(AuScFaceTri(*this, pieces[2]));
		}
		else
		{
			// Make a triangle from each piece.
			for(AuScTriList::const_iterator ti = pieces.cbegin(); ti != pieces.cend(); ti++)
				nufaces.AddFace(AuScFaceTri(*this, *ti));
		}
	}
	else
		nufaces.AddFace(*this);			// I did not get sliced.
}

void AuScFaceTri::FlattenVertexNormals()
{
	AuPoint n = FaceNormal();
	m_Tri.SetVertexNormal(0, n);
	m_Tri.SetVertexNormal(1, n);
	m_Tri.SetVertexNormal(2, n);
}

void AuScFaceTri::Divide(quint8 divisions, AuScFaceList& nufaces) const
{
	QList<AuScPoint> prevpeaks;
	prevpeaks.append(m_Tri.V1());
	int rows = divisions + 1;
	for(int d=1; d<=rows; d++)
	{
		double frac = (double)d / (double)rows;
		AuScPoint bl = m_Tri.V1().LinearInterpolate(m_Tri.V0(), frac);
		AuScPoint br = m_Tri.V1().LinearInterpolate(m_Tri.V2(), frac);

		double peaks = prevpeaks.size();
		double prfrac = 0;
		QList<AuScPoint> nextpeaks;
		for(int r=1; r<=peaks; r++)
		{
			double rfrac = r / peaks;
			AuScPoint tl = bl.LinearInterpolate(br, prfrac);
			AuScPoint tr = bl.LinearInterpolate(br, rfrac);
			AuScPoint peak = prevpeaks.takeFirst();

			nufaces.AddFace(AuScFaceTri(*this, tl, peak, tr));
			if(r < peaks)
				nufaces.AddFace(AuScFaceTri(*this, peak, prevpeaks.first(), tr));

			if(r == 1)
				nextpeaks.append(tl);
			nextpeaks.append(tr);

			prfrac = rfrac;
		}
		prevpeaks = nextpeaks;
	}
}

void AuScFaceTri::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count]) const
{
	AuScTriRender tri(m_Tri, TextureIndex(), PaintColor(themeColors), Reflectivity(), Shine(), SelfIllumination(), Opacity(),
			Effect());
	if(Opacity() == 1)
		opaqueTris.insert(Effect(), tri);
	else
		transparentTris.insert(Effect(), tri);
}

void AuScFaceTri::TextureCylindrical(quint8 texIndex, quint16 texRepsAtEquator, double texSize, const AuMatrix4x4& gridPlane)
{
	SetTextureIndex(texIndex);
	m_Tri.TextureCylindrical(texRepsAtEquator, texSize, gridPlane);
}

void AuScFaceTri::TextureProject(quint8 texIndex, double texSize, const AuMatrix4x4& gridPlane)
{
	SetTextureIndex(texIndex);
	m_Tri.TextureProject(texSize, gridPlane);
}

void AuScFaceTri::TextureSpherical(quint8 texIndex, quint16 texRepsAtEquator, const AuMatrix4x4& gridPlane)
{
	SetTextureIndex(texIndex);
	m_Tri.TextureSpherical(texRepsAtEquator, gridPlane);
}

void AuScFaceTri::TextureMove(const QPointF& vec)
{
	m_Tri.TextureMove(vec);
}

void AuScFaceTri::TextureMove(int vertexIndex, const QPointF& vec)
{
	m_Tri.TextureMove(vertexIndex, vec);
}

void AuScFaceTri::TextureRotate(int vertexIndex, const QPointF& pivot, double angle)
{
	m_Tri.TextureRotate(vertexIndex, pivot, angle);
}

void AuScFaceTri::TextureStretch(int vertexIndex, const QPointF& base, double scaleX, double scaleY)
{
	m_Tri.TextureStretch(vertexIndex, base, scaleX, scaleY);
}

void AuScFaceTri::EffectCylindrical(SURFACE_EFFECT effect, quint16 texRepsAtEquator, double texSize, const AuMatrix4x4& gridPlane)
{
	SetEffect(effect);
	m_Tri.EffectCylindrical(texRepsAtEquator, texSize, gridPlane);
}

void AuScFaceTri::EffectProject(SURFACE_EFFECT effect, double texSize, const AuMatrix4x4& gridPlane)
{
	SetEffect(effect);
	m_Tri.EffectProject(texSize, gridPlane);
}

void AuScFaceTri::EffectSpherical(SURFACE_EFFECT effect, quint16 texRepsAtEquator, const AuMatrix4x4& gridPlane)
{
	SetEffect(effect);
	m_Tri.EffectSpherical(texRepsAtEquator, gridPlane);
}

void AuScFaceTri::EffectMove(const QPointF& vec)
{
	m_Tri.EffectMove(vec);
}

void AuScFaceTri::EffectMove(int vertexIndex, const QPointF& vec)
{
	m_Tri.EffectMove(vertexIndex, vec);
}

void AuScFaceTri::EffectRotate(int vertexIndex, const QPointF& pivot, double angle)
{
	m_Tri.EffectRotate(vertexIndex, pivot, angle);
}

void AuScFaceTri::EffectStretch(int vertexIndex, const QPointF& base, double scaleX, double scaleY)
{
	m_Tri.EffectStretch(vertexIndex, base, scaleX, scaleY);
}

double AuScFaceTri::Area() const
{
	return m_Tri.Area();
}

bool AuScFaceTri::Sight(const AuPoint& eyeLoc, AuPoint& sightLoc) const
{
	return m_Tri.Sight(eyeLoc, sightLoc);
}

float AuScFaceTri::SiteInfluence(const AuPoint& loc, AuPoint& siteLoc) const
{
	return m_Tri.SiteInfluence(loc, siteLoc);
}

bool AuScFaceTri::InSite(const AuPoint& loc) const
{
	return m_Tri.InSite(loc);
}

quint64 AuScFaceTri::MemoryUsage() const
{
	quint64 mem = AuScFace::MemoryUsage();
	mem += sizeof(m_Tri);
	return mem;
}

void AuScFaceTri::MorphX(const AuMatrix4x4& fromM, const AuMatrix4x4& toM, double morphDistance)
{
	m_Tri.MorphX(fromM, toM, morphDistance);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScFaceQuad::AuScFaceQuad()
{
}

AuScFaceQuad::AuScFaceQuad(const AuScPoint& ll, const AuScPoint& ul, const AuScPoint& ur, const AuScPoint& lr) :
	m_Tri1(ll, ul, ur), m_Tri2(ur, lr, ll)
{
}

AuScFaceQuad::AuScFaceQuad(const AuScFace& c, const AuScPoint& ll, const AuScPoint& ul, const AuScPoint& ur, const AuScPoint& lr) :
	AuScFace(c), m_Tri1(ll, ul, ur), m_Tri2(ur, lr, ll)
{
}

AuScFaceQuad::AuScFaceQuad(const AuScFace& c, const AuScTri& t1, const AuScTri& t2) : AuScFace(c), m_Tri1(t1), m_Tri2(t2)
{
}

void AuScFaceQuad::operator>>(QDataStream& out) const
{
	AuScFace::operator>>(out);
	out << m_Tri1;
	out << m_Tri2;
}

void AuScFaceQuad::operator<<(QDataStream& in)
{
	AuScFace::operator<<(in);
	in >> m_Tri1;
	in >> m_Tri2;
}

bool AuScFaceQuad::Collapsed() const
{
	return m_Tri1.Collapsed() && m_Tri2.Collapsed();
}

AuPoint AuScFaceQuad::FaceNormal() const
{
	if(!m_Tri1.Collapsed())
		return m_Tri1.Normal();
	else
		return m_Tri2.Normal();
}

bool AuScFaceQuad::IsPlane() const
{
	if(m_Tri1.Collapsed() || m_Tri2.Collapsed())
		return true;

	AuPoint n(m_Tri1.Normal());
	AuPoint v = m_Tri2.V1() - m_Tri1.V0();
	return IsFuzzyZero(v.Dot(n));
}

void AuScFaceQuad::EdgeIntersections(const AuLine& line, AuPointArray& intersections) const
{
	// Find the intersections, if any.
	AuPoint linePoint = line.U + line.V;

	AuPoint e0int;
	if(e0int.SegmentIntersection(m_Tri1.V0(), m_Tri1.V1(), line.U, linePoint, true, false, d_SliceFuzz))
		intersections.AddIfUnique(e0int);

	AuPoint e1int;
	if(e1int.SegmentIntersection(m_Tri1.V1(), m_Tri1.V2(), line.U, linePoint, true, false, d_SliceFuzz))
		intersections.AddIfUnique(e1int);

	AuPoint e2int;
	if(e2int.SegmentIntersection(m_Tri2.V0(), m_Tri2.V1(), line.U, linePoint, true, false, d_SliceFuzz))
		intersections.AddIfUnique(e2int);

	AuPoint e3int;
	if(e3int.SegmentIntersection(m_Tri2.V1(), m_Tri2.V2(), line.U, linePoint, true, false, d_SliceFuzz))
		intersections.AddIfUnique(e3int);
}

void AuScFaceQuad::Transform(const AuMatrix4x4& t, quint32 firstVertex, const QSet<quint32>& vertexIndices,
		quint32 firstEdge, const QSet<quint32>& edgeIndices, bool faceIsSelected)
{
	// ll
	if(faceIsSelected || vertexIndices.contains(firstVertex) ||
			edgeIndices.contains(firstEdge+3) || edgeIndices.contains(firstEdge))
	{
		m_Tri1.TransformVertex(t, 0);
		m_Tri2.TransformVertex(t, 2);
	}

	// ul
	if(faceIsSelected || vertexIndices.contains(firstVertex+1) ||
			edgeIndices.contains(firstEdge) || edgeIndices.contains(firstEdge+1))
		m_Tri1.TransformVertex(t, 1);

	// ur
	if(faceIsSelected || vertexIndices.contains(firstVertex+2) ||
			edgeIndices.contains(firstEdge+1) || edgeIndices.contains(firstEdge+2))
	{
		m_Tri1.TransformVertex(t, 2);
		m_Tri2.TransformVertex(t, 0);
	}

	// lr
	if(faceIsSelected || vertexIndices.contains(firstVertex+3) ||
			edgeIndices.contains(firstEdge+2) || edgeIndices.contains(firstEdge+3))
		m_Tri2.TransformVertex(t, 1);
}

void AuScFaceQuad::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count]) const
{
	AuScTriList t;
	Triangles(t);
	t.RenderTriangles(opaqueTris, transparentTris, TextureIndex(), PaintColor(themeColors), Reflectivity(), Shine(),
			SelfIllumination(), Opacity(), Effect());
}

void AuScFaceQuad::Triangles(AuScTriList& tris) const
{
	// If either triangle is collapsed, return the other one.
	if(m_Tri1.Collapsed())
	{
		tris.append(m_Tri2);
		return;
	}
	else if(m_Tri2.Collapsed())
	{
		tris.append(m_Tri1);
		return;
	}

	// Neither triangle is collapsed.
	if(IsPlane())
	{
		// Check for concavity that forces split across quad the other way.
		if(m_Tri1.ContainsFuzzy(m_Tri2.V1()) ||
				m_Tri2.ContainsFuzzy(m_Tri1.V1()))
		{
			// A vertex has been dragged into the other triangle. Split the quad across that way.
			tris.append(AuScTri(m_Tri2.V1(), m_Tri1.V0(), m_Tri1.V1()));
			tris.append(AuScTri(m_Tri1.V1(), m_Tri2.V0(), m_Tri2.V1()));
		}
		else
		{
			// Concave or not, these two triangles will work.
			tris.append(m_Tri1);
			tris.append(m_Tri2);
		}
	}
	else
	{
		// Non-planar quad is broken. Return four triangles that meet in the average center for a nice predictable/symmetrical bend.
		AuScPoint c1 = m_Tri1.V0().LinearInterpolate(m_Tri2.V0(), .5);
		AuScPoint c2 = m_Tri1.V1().LinearInterpolate(m_Tri2.V1(), .5);
		AuScPoint cp = c1.LinearInterpolate(c2, .5);

		tris.append(AuScTri(m_Tri1.V0(), m_Tri1.V1(), cp));
		tris.append(AuScTri(m_Tri1.V1(), m_Tri1.V2(), cp));
		tris.append(AuScTri(m_Tri2.V0(), m_Tri2.V1(), cp));
		tris.append(AuScTri(m_Tri2.V1(), m_Tri2.V2(), cp));
	}
}

bool AuScFaceQuad::Cross(AuPointArray& path, const AuSegment& entryEdge, AuSegment& exitEdge) const
{
	// If the point is at a vertex, don't do anything.
	if(path.last().FuzzyEquals(entryEdge.m_fLoc1) || path.last().FuzzyEquals(entryEdge.m_fLoc2))
		return false;

	AuSegmentArray edges;
	EdgeSegments(edges);
	double frac = 0;
	if(entryEdge.IsSegment(edges[0]))
	{
		double sl = entryEdge.Length();
		if(sl == 0)
			return false;
		exitEdge = edges[2];
		double pl = path.last().Distance(edges[0].m_fLoc1);
		frac = pl / sl;
	}
	else if(entryEdge.IsSegment(edges[1]))
	{
		double sl = entryEdge.Length();
		if(sl == 0)
			return false;
		exitEdge = edges[3];
		double pl = path.last().Distance(edges[1].m_fLoc1);
		frac = pl / sl;
	}
	else if(entryEdge.IsSegment(edges[2]))
	{
		double sl = entryEdge.Length();
		if(sl == 0)
			return false;
		exitEdge = edges[0];
		double pl = path.last().Distance(edges[2].m_fLoc1);
		frac = pl / sl;
	}
	else if(entryEdge.IsSegment(edges[3]))
	{
		double sl = entryEdge.Length();
		if(sl == 0)
			return false;
		exitEdge = edges[1];
		double pl = path.last().Distance(edges[3].m_fLoc1);
		frac = pl / sl;
	}
	else
		return false;

	AuPoint a = exitEdge.m_fLoc2 + (exitEdge.m_fLoc1 - exitEdge.m_fLoc2) * frac;
	path.append(a);
	return true;
}

double AuScFaceQuad::Distance(const AuPoint& v) const
{
	double t1 = m_Tri1.Distance(v);
	double t2 = m_Tri2.Distance(v);
	if(t1 < t2)
		return t1;
	else
		return t2;
}

AuScFace* AuScFaceQuad::Join(const AuScFace& f) const
{
	// If 'f' is collapsed, completely absorb it.
	if(f.Collapsed())
		return Clone();					// Absorb 'f' by returning a clone of myself without changes.

	// Create a polygon from myself to do the work.
	return AuScFacePolygon(*this).Join(f);
}

bool AuScFaceQuad::ContainsFuzzy(const AuPoint& v) const
{
	return m_Tri1.ContainsFuzzy(v) || m_Tri2.ContainsFuzzy(v);
}

bool AuScFaceQuad::TestSlice(const AuPoint& seg1, const AuPoint& seg2) const
{
	// Test seg ends for being inside either of my triangles.
	AuPoint n1(m_Tri1.Normal());
	if(seg1.InTriangleFuzzy_3D(m_Tri1.V0(), m_Tri1.V1(), m_Tri1.V2(), n1, d_TriTol))
		return true;
	if(seg2.InTriangleFuzzy_3D(m_Tri1.V0(), m_Tri1.V1(), m_Tri1.V2(), n1, d_TriTol))
		return true;
	AuPoint n2(m_Tri2.Normal());
	if(seg1.InTriangleFuzzy_3D(m_Tri2.V0(), m_Tri2.V1(), m_Tri2.V2(), n2, d_TriTol))
		return true;
	if(seg2.InTriangleFuzzy_3D(m_Tri2.V0(), m_Tri2.V1(), m_Tri2.V2(), n2, d_TriTol))
		return true;

	// Test seg ends for being on the line between my triangles.
	if(seg1.IsOnSegment(m_Tri1.V0(), m_Tri2.V0(), d_SegTol))
		return true;
	if(seg2.IsOnSegment(m_Tri1.V0(), m_Tri2.V0(), d_SegTol))
		return true;

	// Test intersect with my edge segments.
	AuPoint eint;
	bool ehit = eint.SegmentIntersection(m_Tri1.V0(), m_Tri1.V1(), seg1, seg2, false, false, d_SliceFuzz);
	if(ehit && !eint.FuzzyEquals(m_Tri1.V0()) && !eint.FuzzyEquals(m_Tri1.V1()))
		return true;

	ehit = eint.SegmentIntersection(m_Tri1.V1(), m_Tri1.V2(), seg1, seg2, false, false, d_SliceFuzz);
	if(ehit && !eint.FuzzyEquals(m_Tri1.V1()) && !eint.FuzzyEquals(m_Tri1.V2()))
		return true;

	ehit = eint.SegmentIntersection(m_Tri2.V0(), m_Tri2.V1(), seg1, seg2, false, false, d_SliceFuzz);
	if(ehit && !eint.FuzzyEquals(m_Tri2.V0()) && !eint.FuzzyEquals(m_Tri2.V1()))
		return true;

	ehit = eint.SegmentIntersection(m_Tri2.V1(), m_Tri2.V2(), seg1, seg2, false, false, d_SliceFuzz);
	if(ehit && !eint.FuzzyEquals(m_Tri2.V1()) && !eint.FuzzyEquals(m_Tri2.V2()))
		return true;

	return false;
}

void AuScFaceQuad::Slice(const AuLine& cutLine, AuScFaceList& nufaces) const
{
	if(m_Tri1.Collapsed())
	{
		AuScFaceTri t(*this, m_Tri2);
		t.Slice(cutLine, nufaces);
		return;
	}
	else if(m_Tri2.Collapsed())
	{
		AuScFaceTri t(*this, m_Tri1);
		t.Slice(cutLine, nufaces);
		return;
	}

	// Do this as triangles.
	AuScFaceList nu1;
	{
		AuScFaceTri t1(*this, m_Tri1);
		t1.Slice(cutLine, nu1);

		AuScFaceTri t2(*this, m_Tri2);
		t2.Slice(cutLine, nu1);

		// If we only have two triangles in the result, then neither triangle was cut.
		// The cut line could be crossing me right across the corners.
		if(nu1.size() == 2)
		{
			if(cutLine.Distance(t1.Vertex(0)) <= d_SliceFuzz &&
					cutLine.Distance(t2.Vertex(0)) <= d_SliceFuzz)
			{
				// I got sliced across the diagonal.
				nufaces.AddFace(t1);
				nufaces.AddFace(t2);
			}
			else
				nufaces.AddFace(*this);
			return;
		}
	}

	AuScFaceList nu2;
	{
		// Divide the quad the other way because it can produce nicer results.
		AuScFaceTri t1(*this, m_Tri2.V1(), m_Tri1.V0(), m_Tri1.V1());
		t1.Slice(cutLine, nu2);

		AuScFaceTri t2(*this, m_Tri1.V1(), m_Tri2.V0(), m_Tri2.V1());
		t2.Slice(cutLine, nu2);

		// If we only have two triangles in the result, then neither triangle was cut.
		// The cut line could be crossing me right across the corners.
		if(nu2.size() == 2)
		{
			if(cutLine.Distance(t1.Vertex(0)) <= d_SliceFuzz &&
					cutLine.Distance(t2.Vertex(0)) <= d_SliceFuzz)
			{
				// I got sliced across the diagonal.
				nufaces.AddFace(t1);
				nufaces.AddFace(t2);
			}
			else
				nufaces.AddFace(*this);
			return;
		}
	}

	if(nu1.size() < nu2.size())
		nufaces.AddFaces(nu1);
	else
		nufaces.AddFaces(nu2);
}

void AuScFaceQuad::FlattenVertexNormals()
{
	AuPoint n1 = m_Tri1.Normal();
	m_Tri1.SetVertexNormal(0, n1);
	m_Tri1.SetVertexNormal(1, n1);
	m_Tri1.SetVertexNormal(2, n1);

	AuPoint n2 = m_Tri2.Normal();
	m_Tri2.SetVertexNormal(0, n2);
	m_Tri2.SetVertexNormal(1, n2);
	m_Tri2.SetVertexNormal(2, n2);
}

void AuScFaceQuad::Divide(quint8 divisions, AuScFaceList& nufaces) const
{
	// Divide myself into little quads.
	AuSegmentArray edges;
	EdgeSegments(edges);

	int steps = divisions + 1;
	double yfracp = 0;
	for(int yd=1; yd<=steps; yd++)
	{
		// Walk up from the bottom.
		double yfrac = (double)yd / (double)steps;
		AuScPoint lb = Vertex(0).LinearInterpolate(Vertex(1), yfracp);	// Left bottom, along my edge line.
		AuScPoint lt = Vertex(0).LinearInterpolate(Vertex(1), yfrac);	// Left top, along my edge line.
		AuScPoint rb = Vertex(3).LinearInterpolate(Vertex(2), yfracp);	// Right bottom, along my edge line.
		AuScPoint rt = Vertex(3).LinearInterpolate(Vertex(2), yfrac);	// Right top, along my edge line.

		double xfracp = 0;
		for(int xd=1; xd<=steps; xd++)
		{
			double xfrac = (double)xd / (double)steps;

			AuScPoint ll = lb.LinearInterpolate(rb, xfracp);
			AuScPoint ul = lt.LinearInterpolate(rt, xfracp);
			AuScPoint lr = lb.LinearInterpolate(rb, xfrac);
			AuScPoint ur = lt.LinearInterpolate(rt, xfrac);

			nufaces.AddFace(AuScFaceQuad(*this, ll, ul, ur, lr));

			xfracp = xfrac;
		}

		yfracp = yfrac;
	}
}

void AuScFaceQuad::TextureCylindrical(quint8 texIndex, quint16 texRepsAtEquator, double texSize, const AuMatrix4x4& gridPlane)
{
	SetTextureIndex(texIndex);
	m_Tri1.TextureCylindrical(texRepsAtEquator, texSize, gridPlane);
	m_Tri2.TextureCylindrical(texRepsAtEquator, texSize, gridPlane);
}

void AuScFaceQuad::TextureProject(quint8 texIndex, double texSize, const AuMatrix4x4& gridPlane)
{
	SetTextureIndex(texIndex);
	m_Tri1.TextureProject(texSize, gridPlane);
	m_Tri2.TextureProject(texSize, gridPlane);
}

void AuScFaceQuad::TextureSpherical(quint8 texIndex, quint16 texRepsAtEquator, const AuMatrix4x4& gridPlane)
{
	SetTextureIndex(texIndex);
	m_Tri1.TextureSpherical(texRepsAtEquator, gridPlane);
	m_Tri2.TextureSpherical(texRepsAtEquator, gridPlane);
}

void AuScFaceQuad::TextureMove(const QPointF& vec)
{
	m_Tri1.TextureMove(vec);
	m_Tri2.TextureMove(vec);
}

void AuScFaceQuad::TextureMove(int vertexIndex, const QPointF& vec)
{
	if(vertexIndex == 0)
	{
		m_Tri1.TextureMove(0, vec);
		m_Tri2.TextureMove(2, vec);
	}
	else if(vertexIndex == 1)
	{
		m_Tri1.TextureMove(1, vec);
	}
	else if(vertexIndex == 2)
	{
		m_Tri1.TextureMove(2, vec);
		m_Tri2.TextureMove(0, vec);
	}
	else
	{
		m_Tri2.TextureMove(1, vec);
	}
}

void AuScFaceQuad::TextureRotate(int vertexIndex, const QPointF& pivot, double angle)
{
	if(vertexIndex == 0)
	{
		m_Tri1.TextureRotate(0, pivot, angle);
		m_Tri2.TextureRotate(2, pivot, angle);
	}
	else if(vertexIndex == 1)
	{
		m_Tri1.TextureRotate(1, pivot, angle);
	}
	else if(vertexIndex == 2)
	{
		m_Tri1.TextureRotate(2, pivot, angle);
		m_Tri2.TextureRotate(0, pivot, angle);
	}
	else
	{
		m_Tri2.TextureRotate(1, pivot, angle);
	}
}

void AuScFaceQuad::TextureStretch(int vertexIndex, const QPointF& base, double scaleX, double scaleY)
{
	if(vertexIndex == 0)
	{
		m_Tri1.TextureStretch(0, base, scaleX, scaleY);
		m_Tri2.TextureStretch(2, base, scaleX, scaleY);
	}
	else if(vertexIndex == 1)
	{
		m_Tri1.TextureStretch(1, base, scaleX, scaleY);
	}
	else if(vertexIndex == 2)
	{
		m_Tri1.TextureStretch(2, base, scaleX, scaleY);
		m_Tri2.TextureStretch(0, base, scaleX, scaleY);
	}
	else
	{
		m_Tri2.TextureStretch(1, base, scaleX, scaleY);
	}
}

void AuScFaceQuad::EffectCylindrical(SURFACE_EFFECT effect, quint16 texRepsAtEquator, double texSize, const AuMatrix4x4& gridPlane)
{
	SetEffect(effect);
	m_Tri1.EffectCylindrical(texRepsAtEquator, texSize, gridPlane);
	m_Tri2.EffectCylindrical(texRepsAtEquator, texSize, gridPlane);
}

void AuScFaceQuad::EffectProject(SURFACE_EFFECT effect, double texSize, const AuMatrix4x4& gridPlane)
{
	SetEffect(effect);
	m_Tri1.EffectProject(texSize, gridPlane);
	m_Tri2.EffectProject(texSize, gridPlane);
}

void AuScFaceQuad::EffectSpherical(SURFACE_EFFECT effect, quint16 texRepsAtEquator, const AuMatrix4x4& gridPlane)
{
	SetEffect(effect);
	m_Tri1.EffectSpherical(texRepsAtEquator, gridPlane);
	m_Tri2.EffectSpherical(texRepsAtEquator, gridPlane);
}

void AuScFaceQuad::EffectMove(const QPointF& vec)
{
	m_Tri1.EffectMove(vec);
	m_Tri2.EffectMove(vec);
}

void AuScFaceQuad::EffectMove(int vertexIndex, const QPointF& vec)
{
	if(vertexIndex == 0)
	{
		m_Tri1.EffectMove(0, vec);
		m_Tri2.EffectMove(2, vec);
	}
	else if(vertexIndex == 1)
	{
		m_Tri1.EffectMove(1, vec);
	}
	else if(vertexIndex == 2)
	{
		m_Tri1.EffectMove(2, vec);
		m_Tri2.EffectMove(0, vec);
	}
	else
	{
		m_Tri2.EffectMove(1, vec);
	}
}

void AuScFaceQuad::EffectRotate(int vertexIndex, const QPointF& pivot, double angle)
{
	if(vertexIndex == 0)
	{
		m_Tri1.EffectRotate(0, pivot, angle);
		m_Tri2.EffectRotate(2, pivot, angle);
	}
	else if(vertexIndex == 1)
	{
		m_Tri1.EffectRotate(1, pivot, angle);
	}
	else if(vertexIndex == 2)
	{
		m_Tri1.EffectRotate(2, pivot, angle);
		m_Tri2.EffectRotate(0, pivot, angle);
	}
	else
	{
		m_Tri2.EffectRotate(1, pivot, angle);
	}
}

void AuScFaceQuad::EffectStretch(int vertexIndex, const QPointF& base, double scaleX, double scaleY)
{
	if(vertexIndex == 0)
	{
		m_Tri1.EffectStretch(0, base, scaleX, scaleY);
		m_Tri2.EffectStretch(2, base, scaleX, scaleY);
	}
	else if(vertexIndex == 1)
	{
		m_Tri1.EffectStretch(1, base, scaleX, scaleY);
	}
	else if(vertexIndex == 2)
	{
		m_Tri1.EffectStretch(2, base, scaleX, scaleY);
		m_Tri2.EffectStretch(0, base, scaleX, scaleY);
	}
	else
	{
		m_Tri2.EffectStretch(1, base, scaleX, scaleY);
	}
}

AuMatrix4x4 AuScFaceQuad::FacePlane() const
{
	AuPoint n(FaceNormal());
	AuPoint x(Vertex(3) - Vertex(0));
	x.Unit();
	AuPoint y(n.Cross(x));
	y.Unit();
	return AuMatrix4x4(Vertex(0), x, y);
}

double AuScFaceQuad::Area() const
{
	return m_Tri1.Area() + m_Tri2.Area();
}

bool AuScFaceQuad::Sight(const AuPoint& eyeLoc, AuPoint& sightLoc) const
{
	// We are more concerned about speed than folded over faces. Faces are supposed to be planar, more or less.
	if(m_Tri1.Sight(eyeLoc, sightLoc))
		return true;
	return m_Tri2.Sight(eyeLoc, sightLoc);
}

float AuScFaceQuad::SiteInfluence(const AuPoint& loc, AuPoint& siteLoc) const
{
	float siteI = m_Tri1.SiteInfluence(loc, siteLoc);
	if(siteI > 0)
		return siteI;
	return m_Tri2.SiteInfluence(loc, siteLoc);
}

bool AuScFaceQuad::InSite(const AuPoint& loc) const
{
	if(m_Tri1.InSite(loc))
		return true;
	return m_Tri2.InSite(loc);
}

quint64 AuScFaceQuad::MemoryUsage() const
{
	quint64 mem = AuScFace::MemoryUsage();
	mem += sizeof(m_Tri1);
	mem += sizeof(m_Tri2);
	return mem;
}

void AuScFaceQuad::MorphX(const AuMatrix4x4& fromM, const AuMatrix4x4& toM, double morphDistance)
{
	m_Tri1.MorphX(fromM, toM, morphDistance);
	m_Tri2.MorphX(fromM, toM, morphDistance);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScFacePolygon::AuScFacePolygon()
{
}


AuScFacePolygon::AuScFacePolygon(const AuPointArray& tris, const QVector<bool>& edges)
{
	QVector<bool>::const_iterator ei = edges.cbegin();
	for(AuPointArray::const_iterator ti = tris.cbegin(); ti != tris.cend(); ti+=3, ei+=3)
	{
		AuScPoint v0(*ti, 0,0, AuPoint(0,0,1));
		AuScPoint v1(*(ti+1), 0,0, AuPoint(0,0,1));
		AuScPoint v2(*(ti+2), 0,0, AuPoint(0,0,1));

		m_Tris.append(AuScTri(v0, v1, v2));

		if(*ei)
			m_Edges.append(AuSegment(v0, v1));

		if(*(ei+1))
			m_Edges.append(AuSegment(v1, v2));

		if(*(ei+2))
			m_Edges.append(AuSegment(v2, v0));
	}
}

AuScFacePolygon::AuScFacePolygon(const AuPointArray& tris, const QVector<bool>& edges, const AuPoint& offset)
{
	QVector<bool>::const_iterator ei = edges.cbegin();
	for(AuPointArray::const_iterator ti = tris.cbegin(); ti != tris.cend(); ti+=3, ei+=3)
	{
		AuScPoint v0(*ti, 0,0, AuPoint(0,0,1));
		v0 += offset;
		AuScPoint v1(*(ti+1), 0,0, AuPoint(0,0,1));
		v1 += offset;
		AuScPoint v2(*(ti+2), 0,0, AuPoint(0,0,1));
		v2 += offset;

		m_Tris.append(AuScTri(v0, v1, v2));

		if(*ei)
			m_Edges.append(AuSegment(v0, v1));

		if(*(ei+1))
			m_Edges.append(AuSegment(v1, v2));

		if(*(ei+2))
			m_Edges.append(AuSegment(v2, v0));
	}
}

AuScFacePolygon::AuScFacePolygon(const AuScFace& c, const AuPointArray& tris, const QVector<bool>& edges) : AuScFace(c)
{
	QVector<bool>::const_iterator ei = edges.cbegin();
	for(AuPointArray::const_iterator ti = tris.cbegin(); ti != tris.cend(); ti+=3, ei+=3)
	{
		AuScPoint v0(*ti, 0,0, AuPoint(0,0,1));
		AuScPoint v1(*(ti+1), 0,0, AuPoint(0,0,1));
		AuScPoint v2(*(ti+2), 0,0, AuPoint(0,0,1));

		m_Tris.append(AuScTri(v0, v1, v2));

		if(*ei)
			m_Edges.append(AuSegment(v0, v1));

		if(*(ei+1))
			m_Edges.append(AuSegment(v1, v2));

		if(*(ei+2))
			m_Edges.append(AuSegment(v2, v0));
	}
}

AuScFacePolygon::AuScFacePolygon(const AuScFace& c, const AuPointArray& tris, const QVector<bool>& edges, const AuPoint& offset) :
	AuScFace(c)
{
	QVector<bool>::const_iterator ei = edges.cbegin();
	for(AuPointArray::const_iterator ti = tris.cbegin(); ti != tris.cend(); ti+=3, ei+=3)
	{
		AuScPoint v0(*ti, 0,0, AuPoint(0,0,1));
		v0 += offset;
		AuScPoint v1(*(ti+1), 0,0, AuPoint(0,0,1));
		v1 += offset;
		AuScPoint v2(*(ti+2), 0,0, AuPoint(0,0,1));
		v2 += offset;

		m_Tris.append(AuScTri(v0, v1, v2));

		if(*ei)
			m_Edges.append(AuSegment(v0, v1));

		if(*(ei+1))
			m_Edges.append(AuSegment(v1, v2));

		if(*(ei+2))
			m_Edges.append(AuSegment(v2, v0));
	}
}

AuScFacePolygon::AuScFacePolygon(const AuScFaceQuad& q) : AuScFace(q)
{
	q.Triangles(m_Tris);
	q.EdgeSegments(m_Edges);
}

AuScFacePolygon::AuScFacePolygon(const AuScFace& c, const AuScTriList& tris, const AuSegmentArray& edges) : AuScFace(c),
	m_Edges(edges), m_Tris(tris)
{
}

void AuScFacePolygon::operator>>(QDataStream& out) const
{
	AuScFace::operator>>(out);
	out << m_Edges;
	out << m_Tris;
}

void AuScFacePolygon::operator<<(QDataStream& in)
{
	AuScFace::operator<<(in);
	in >> m_Edges;
	in >> m_Tris;
}

bool AuScFacePolygon::Collapsed() const
{
	return m_Tris.Collapsed();
}

bool AuScFacePolygon::IsPlane() const
{
	if(m_Tris.empty())
		return false;

	// Compare normals of all my triangles.
	AuPoint o = m_Tris.first().V0();
	AuPoint n = m_Tris.first().Normal();
	for(AuScTriList::const_iterator ti = m_Tris.cbegin(); ti != m_Tris.cend(); ti++)
	{
		if((*ti).Collapsed())
			continue;

		// Compare face normals.
		if(!(*ti).Normal().FuzzyEquals(n))
			return false;

		// Compare altitude above plane of first triangle.
		AuPoint v = (*ti).V0() - o;
		double d = v.Dot(n);
		if(!IsFuzzyZero(d))
			return false;
	}

	return true;
}

void AuScFacePolygon::EdgeIntersections(const AuLine& line, AuPointArray& intersections) const
{
	// Find the intersections, if any.
	AuPoint linePoint = line.U + line.V;

	AuPoint eint;
	for(AuSegmentArray::const_iterator ei = m_Edges.cbegin(); ei != m_Edges.cend(); ei++)
		if(eint.SegmentIntersection((*ei).m_fLoc1, (*ei).m_fLoc2, line.U, linePoint, true, false, d_SliceFuzz))
			intersections.AddIfUnique(eint);
}

void AuScFacePolygon::Transform(const AuMatrix4x4& t, quint32 firstVertex, const QSet<quint32>& vertexIndices,
		quint32 firstEdge, const QSet<quint32>& edgeIndices, bool faceIsSelected)
{
	AuPointArray tlocs;	// Locations that get transformed, so we can do them in the edge segments too.

	AuPointArray movingEdgeLocs;
	quint32 eindex = 0;
	for(AuSegmentArray::const_iterator ei = m_Edges.begin(); ei != m_Edges.end(); ei++, eindex++)
	{
		if(edgeIndices.contains(firstEdge + eindex))
		{
			// The edge is moving so both ends must move.
			movingEdgeLocs.append((*ei).m_fLoc1);
			movingEdgeLocs.append((*ei).m_fLoc2);
		}
	}

	int vindex = firstVertex;
	for(AuScTriList::iterator ti = m_Tris.begin(); ti != m_Tris.end(); ti++)
	{
		if(faceIsSelected || vertexIndices.contains(vindex) || movingEdgeLocs.DoesContain((*ti).V0()))
		{
			tlocs.append((*ti).V0());
			(*ti).TransformVertex(t, 0);
		}
		vindex++;

		if(faceIsSelected || vertexIndices.contains(vindex) || movingEdgeLocs.DoesContain((*ti).V1()))
		{
			tlocs.append((*ti).V1());
			(*ti).TransformVertex(t, 1);
		}
		vindex++;

		if(faceIsSelected || vertexIndices.contains(vindex) || movingEdgeLocs.DoesContain((*ti).V2()))
		{
			tlocs.append((*ti).V2());
			(*ti).TransformVertex(t, 2);
		}
		vindex++;
	}

	// Move edge line vertices that correspond to triangle vertices that moved.
	for(AuSegmentArray::iterator ei = m_Edges.begin(); ei != m_Edges.end(); ei++)
	{
		if(tlocs.DoesContain((*ei).m_fLoc1))
			(*ei).m_fLoc1 = t.Project((*ei).m_fLoc1);
		if(tlocs.DoesContain((*ei).m_fLoc2))
			(*ei).m_fLoc2 = t.Project((*ei).m_fLoc2);
	}

	// Remove triangles that collapsed.
	/* This should be done somewhere else because it messes up the face indices while we are transforming them.
	AuScTriList::iterator ti = m_Tris.begin();
	while(ti != m_Tris.end())
	{
		if((*ti).Collapsed())
			ti = m_Tris.erase(ti);
		else
			ti++;
	}
	*/
}

bool AuScFacePolygon::Cross(AuPointArray& path, const AuSegment& entryEdge, AuSegment& exitEdge) const
{
	// If the point is at a vertex, don't do anything.
	if(path.last().FuzzyEquals(entryEdge.m_fLoc1) || path.last().FuzzyEquals(entryEdge.m_fLoc2))
		return false;

	double sl = entryEdge.Length();
	if(sl == 0)
		return false;

	// Get my edges and fuse them so I can cross them sensibly.
	AuSegmentArray edges;
	EdgeSegments(edges);
	edges.Fuse();
	if(edges.size() < 4)
		return false;					// Not enough edges to go across.

	// Sort the edge lines to see if they make a boundary.
	if(!edges.Align())
		return false;					// Edges do not form a boundary. Hard to guess which edge is across without more work.

	// Get our entry edge index to see if we are next to be loop cut (see usage of this function).
	int entryEdgeIndex = edges.FindSegmentFuzzy(entryEdge);
	if(entryEdgeIndex < 0)
		return false;					// I am not adjacent to whoever is getting loop cut.

	// Guess at the exit edge index, based on edge count. This works for quad-like polygons. Cross your fingers otherwise.
	int exitEdgeIndex = entryEdgeIndex + edges.size()/2;
	if(exitEdgeIndex >= edges.size())
		exitEdgeIndex -= edges.size();
	exitEdge = edges[exitEdgeIndex];

	// Calculate fractional distance along exit edge.
	double pl = path.last().Distance(edges[entryEdgeIndex].m_fLoc1);
	double frac = pl / sl;

	// Calculate crossing point on exit edge.
	AuPoint a = exitEdge.m_fLoc2 + (exitEdge.m_fLoc1 - exitEdge.m_fLoc2) * frac;
	path.append(a);
	return true;
}

double AuScFacePolygon::Distance(const AuPoint& v) const
{
	double closestDist = 0;
	for(AuScTriList::const_iterator ti = m_Tris.cbegin(); ti != m_Tris.cend(); ti++)
	{
		double dist = (*ti).Distance(v);
		if(closestDist == 0 || dist < closestDist)
		{
			if(dist == 0)
				return 0;
			closestDist = dist;
		}
	}
	return closestDist;
}

AuScFace* AuScFacePolygon::Join(const AuScFace& f) const
{
	// If 'f' is collapsed, completely absorb it.
	if(f.Collapsed())
		return Clone();					// Absorb 'f' by returning a clone of myself without changes.

	// I cannot join things if I am not a plane.
	if(!IsPlane())
		return 0;

	// I cannot join things that do not form a plane.
	if(!f.IsPlane())
		return 0;

	// If our planes are not parallel, we cannot join.
	AuPoint n(FaceNormal());
	if(!n.IsParallel(f.FaceNormal()))
		return 0;						// Planes are not parallel.

	// Compare height difference between our planes.
	double dist = (f.Vertex(0) - Vertex(0)).Dot(n);
	if(FuzzyGreaterThanFactor(dist, 0, d_InTrianglePlaneTol))
		return 0;						// Planes are too far apart.

	// Locate a common edge.
	AuSegmentArray fsegs;
	f.EdgeSegments(fsegs);

	// The resulting edge segments are created by joining our two edge arrays, omitting any common segments.
	AuSegmentArray nuedges;
	bool commonEdge = false;
	for(AuSegmentArray::const_iterator si = m_Edges.cbegin(); si != m_Edges.cend(); si++)
	{
		// Skip my segments that are contained in fsegs.
		if(fsegs.Contains(*si))
		{
			commonEdge = true;
			continue;
		}
		nuedges.append(*si);
	}

	for(AuSegmentArray::const_iterator fsi = fsegs.cbegin(); fsi != fsegs.cend(); fsi++)
	{
		// Skip fsegs segments that are contained in m_Edges.
		if(m_Edges.Contains(*fsi))
		{
			commonEdge = true;
			continue;
		}
		nuedges.append(*fsi);
	}

	if(!commonEdge)
		return 0;						// No common edge segments.

	// Resulting triangles are just the triangles of 'f' added to mine.
	AuScTriList nutris(m_Tris);
	f.Triangles(nutris);

	if(nutris.size() == 1)
	{
		// Everything reduced to a single triangle.
		return new AuScFaceTri(*this, nutris.first());
	}
	else if(nutris.size() == 2)
	{
		// Two triangles might join into a quad.
		AuScFaceTri t1(*this, nutris.first());
		AuScFaceTri t2(*this, *(nutris.begin() + 1));
		AuScFace* nf = t1.Join(t2);
		if(nf)
			return nf;					// The two triangles joined into a single triangle or a quad.
	}

	// See if I can turn myself into a quad or triangle.
	// Get my edges and fuse them.
	AuSegmentArray edges(nuedges);
	edges.Fuse();
	if(edges.size() == 3 && edges.Align())
	{
		// I have three edges that form a boundary.
		// It looks like I turned into a triangle.
		AuScPoint v0 = nutris.VertexAt(edges[0].m_fLoc1);
		AuScPoint v1 = nutris.VertexAt(edges[1].m_fLoc1);
		AuScPoint v2 = nutris.VertexAt(edges[2].m_fLoc1);
		//qDebug() << "made a quad" << v0 << v1 << v2 << v3;
		return new AuScFaceTri(*this, v0, v1, v2);
	}
	else if(edges.size() == 4 && edges.Align())
	{
		// I have four edges that form a boundary.
		// It looks like I turned into a quad.
		AuScPoint v0 = nutris.VertexAt(edges[0].m_fLoc1);
		AuScPoint v1 = nutris.VertexAt(edges[1].m_fLoc1);
		AuScPoint v2 = nutris.VertexAt(edges[2].m_fLoc1);
		AuScPoint v3 = nutris.VertexAt(edges[3].m_fLoc1);
		//qDebug() << "made a quad" << v0 << v1 << v2 << v3;
		return new AuScFaceQuad(*this, v0, v1, v2, v3);
	}

	// Create a new polygon.
	return new AuScFacePolygon(*this, nutris, nuedges);
}

AuScPoint AuScFacePolygon::Vertex(int index) const
{
	int triIndex = index / 3;
	int vertexIndex = index % 3;
	return m_Tris[triIndex].Vertex(vertexIndex);
}

int AuScFacePolygon::VertexAt(const AuPoint& loc) const
{
	int vindex = 0;
	for(AuScTriList::const_iterator ti = m_Tris.cbegin(); ti != m_Tris.cend(); ti++)
	{
		if((*ti).Vertex(0).FuzzyEquals(loc))
			return vindex;
		vindex++;
		if((*ti).Vertex(1).FuzzyEquals(loc))
			return vindex;
		vindex++;
		if((*ti).Vertex(2).FuzzyEquals(loc))
			return vindex;
		vindex++;
	}
	return -1;
}

int AuScFacePolygon::VertexCount() const
{
	return m_Tris.size() * 3;
}

void AuScFacePolygon::VertexLocations(AuPointArray& verts) const
{
	for(AuScTriList::const_iterator ti = m_Tris.cbegin(); ti != m_Tris.cend(); ti++)
	{
		verts.append((*ti).Vertex(0));
		verts.append((*ti).Vertex(1));
		verts.append((*ti).Vertex(2));
	}
}

bool AuScFacePolygon::Sight(const AuPoint& eyeLoc, AuPoint& sightLoc) const
{
	// We are more concerned about speed than folded over faces. Faces are supposed to be planar, more or less.
	for(AuScTriList::const_iterator ti = m_Tris.cbegin(); ti != m_Tris.cend(); ti++)
		if((*ti).Sight(eyeLoc, sightLoc))
			return true;
	return false;
}

float AuScFacePolygon::SiteInfluence(const AuPoint& loc, AuPoint& siteLoc) const
{
	for(AuScTriList::const_iterator ti = m_Tris.cbegin(); ti != m_Tris.cend(); ti++)
	{
		float siteI = (*ti).SiteInfluence(loc, siteLoc);
		if(siteI > 0)
			return siteI;
	}
	return 0;
}

bool AuScFacePolygon::InSite(const AuPoint& loc) const
{
	for(AuScTriList::const_iterator ti = m_Tris.cbegin(); ti != m_Tris.cend(); ti++)
		if((*ti).InSite(loc))
			return true;
	return false;
}

/*
AuScPoint AuScFacePolygon::Vertex(int index) const
{
	int edgeIndex = index / 2;
	int vertexIndex = index % 2;
	if(vertexIndex)
		return m_Tris.VertexAt(m_Edges[edgeIndex].m_fLoc2);
	else
		return m_Tris.VertexAt(m_Edges[edgeIndex].m_fLoc1);
}

int AuScFacePolygon::VertexCount() const
{
	return m_Edges.size() * 2;
}

void AuScFacePolygon::VertexLocations(AuPointArray& verts) const
{
	for(AuSegmentArray::const_iterator ei = m_Edges.cbegin(); ei != m_Edges.cend(); ei++)
	{
		verts.append((*ei).m_fLoc1);
		verts.append((*ei).m_fLoc2);
	}
}
*/

bool AuScFacePolygon::ContainsFuzzy(const AuPoint& v) const
{
	for(AuScTriList::const_iterator ti = m_Tris.cbegin(); ti != m_Tris.cend(); ti++)
		if((*ti).ContainsFuzzy(v))
			return true;
	return false;
}

bool AuScFacePolygon::TestSlice(const AuPoint& seg1, const AuPoint& seg2) const
{
	// Do this as triangles.
	for(AuScTriList::const_iterator ti = m_Tris.cbegin(); ti != m_Tris.cend(); ti++)
	{
		AuScFaceTri ft(*this, *ti);
		if(ft.TestSlice(seg1, seg2))
			return true;
	}

	// No triangle was sliced. The line could have crossed right on a triangle edge line.
	// Check to see if the line is colinear with the edge of any triangle that is not an edge of the face.
	AuLine cutLine(seg1, seg2);
	AuSegment edgeSeg;
	for(AuScTriList::const_iterator ti = m_Tris.cbegin(); ti != m_Tris.cend(); ti++)
	{
		// Test for an edge of the triangle being on the line.
		if(!(*ti).EdgeOnLine(cutLine, edgeSeg))
			continue;				// Line does not share an edge with the triangle.

		// Triangle edge is on the line. Is it an edge line?
		if(!m_Edges.Contains(edgeSeg))
		{
			// The triangle edge is not one of the face edges so it is an internal edge. I got sliced.
			if(edgeSeg.m_fLoc1.Between(seg1, seg2) || edgeSeg.m_fLoc2.Between(seg1, seg2) ||
					seg1.Between(edgeSeg.m_fLoc1, edgeSeg.m_fLoc2))
				return true;
		}
	}

	return false;
}

void AuScFacePolygon::Slice(const AuLine& cutLine, AuScFaceList& nufaces) const
{
	// Do this as triangles.
	AuScFaceList tfaces;
	for(AuScTriList::const_iterator ti = m_Tris.cbegin(); ti != m_Tris.cend(); ti++)
	{
		AuScFaceTri ft(*this, *ti);
		ft.Slice(cutLine, tfaces);
	}

	bool anySliced = tfaces.size() != m_Tris.size();
	if(!anySliced)
	{
		// No triangle was sliced. The line could have crossed right on a triangle edge line.
		// Check to see if the line is colinear with the edge of any triangle that is not an edge of the face.
		AuSegment edgeSeg;
		for(AuScTriList::const_iterator ti = m_Tris.cbegin(); ti != m_Tris.cend(); ti++)
		{
			// Test for an edge of the triangle being on the line.
			if(!(*ti).EdgeOnLine(cutLine, edgeSeg))
				continue;				// Line does not share an edge with the triangle.

			// Triangle edge is on the line. Is it an edge line?
			if(!m_Edges.Contains(edgeSeg))
			{
				// The triangle edge is not one of the face edges so it is an internal edge. I got sliced.
				anySliced = true;
				break;
			}
		}
	}

	if(anySliced)
		nufaces.AddFaces(tfaces);		// Add all the individual triangles to the result as separate faces.
	else
		nufaces.AddFace(*this);			// Add my one single unsliced face to the result.
}

void AuScFacePolygon::FlattenVertexNormals()
{
	// Calculate normal for every triangle.
	for(AuScTriList::iterator ti = m_Tris.begin(); ti != m_Tris.end(); ti++)
	{
		AuPoint n = (*ti).Normal();
		(*ti).SetVertexNormal(0, n);
		(*ti).SetVertexNormal(1, n);
		(*ti).SetVertexNormal(2, n);
	}
}

void AuScFacePolygon::Divide(quint8 divisions, AuScFaceList& nufaces) const
{
	// See if I can do this using a quad.
	// Get my edges and fuse them.
	AuSegmentArray edges;
	EdgeSegments(edges);
	edges.Fuse();
	if(edges.size() == 3 && edges.Align())
	{
		// I have three edges that form a boundary.
		// It looks like I turned into a triangle.
		AuScPoint v0 = m_Tris.VertexAt(edges[0].m_fLoc1);
		AuScPoint v1 = m_Tris.VertexAt(edges[1].m_fLoc1);
		AuScPoint v2 = m_Tris.VertexAt(edges[2].m_fLoc1);
		AuScFaceTri t(*this, v0, v1, v2);
		t.Divide(divisions, nufaces);
		return;
	}
	else if(edges.size() == 4 && edges.Align())
	{
		// I have four edges that form a boundary.
		// It looks like I turned into a quad.
		AuScPoint v0 = m_Tris.VertexAt(edges[0].m_fLoc1);
		AuScPoint v1 = m_Tris.VertexAt(edges[1].m_fLoc1);
		AuScPoint v2 = m_Tris.VertexAt(edges[2].m_fLoc1);
		AuScPoint v3 = m_Tris.VertexAt(edges[3].m_fLoc1);
		AuScFaceQuad q(*this, v0, v1, v2, v3);
		q.Divide(divisions, nufaces);
		return;
	}

	nufaces.AddFace(*this);
}

void AuScFacePolygon::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count]) const
{
	m_Tris.RenderTriangles(opaqueTris, transparentTris, TextureIndex(), PaintColor(themeColors), Reflectivity(), Shine(),
			SelfIllumination(), Opacity(), Effect());
}

void AuScFacePolygon::TextureCylindrical(quint8 texIndex, quint16 texRepsAtEquator, double texSize, const AuMatrix4x4& gridPlane)
{
	SetTextureIndex(texIndex);
	for(AuScTriList::iterator ti = m_Tris.begin(); ti != m_Tris.end(); ti++)
		(*ti).TextureCylindrical(texRepsAtEquator, texSize, gridPlane);
}

void AuScFacePolygon::TextureProject(quint8 texIndex, double texSize, const AuMatrix4x4& gridPlane)
{
	SetTextureIndex(texIndex);
	for(AuScTriList::iterator ti = m_Tris.begin(); ti != m_Tris.end(); ti++)
		(*ti).TextureProject(texSize, gridPlane);
}

void AuScFacePolygon::TextureSpherical(quint8 texIndex, quint16 texRepsAtEquator, const AuMatrix4x4& gridPlane)
{
	SetTextureIndex(texIndex);
	for(AuScTriList::iterator ti = m_Tris.begin(); ti != m_Tris.end(); ti++)
		(*ti).TextureSpherical(texRepsAtEquator, gridPlane);
}

void AuScFacePolygon::TextureMove(const QPointF& vec)
{
	for(AuScTriList::iterator ti = m_Tris.begin(); ti != m_Tris.end(); ti++)
		(*ti).TextureMove(vec);
}

void AuScFacePolygon::TextureMove(int vertexIndex, const QPointF& vec)
{
	int triIndex = vertexIndex / 3;
	int triVertexIndex = vertexIndex % 3;
	return m_Tris[triIndex].TextureMove(triVertexIndex, vec);
}

void AuScFacePolygon::TextureRotate(int vertexIndex, const QPointF& pivot, double angle)
{
	int triIndex = vertexIndex / 3;
	int triVertexIndex = vertexIndex % 3;
	return m_Tris[triIndex].TextureRotate(triVertexIndex, pivot, angle);
}

void AuScFacePolygon::TextureStretch(int vertexIndex, const QPointF& base, double scaleX, double scaleY)
{
	int triIndex = vertexIndex / 3;
	int triVertexIndex = vertexIndex % 3;
	return m_Tris[triIndex].TextureStretch(triVertexIndex, base, scaleX, scaleY);
}

void AuScFacePolygon::EffectCylindrical(SURFACE_EFFECT effect, quint16 texRepsAtEquator, double texSize, const AuMatrix4x4& gridPlane)
{
	SetEffect(effect);
	for(AuScTriList::iterator ti = m_Tris.begin(); ti != m_Tris.end(); ti++)
		(*ti).EffectCylindrical(texRepsAtEquator, texSize, gridPlane);
}

void AuScFacePolygon::EffectProject(SURFACE_EFFECT effect, double texSize, const AuMatrix4x4& gridPlane)
{
	SetEffect(effect);
	for(AuScTriList::iterator ti = m_Tris.begin(); ti != m_Tris.end(); ti++)
		(*ti).EffectProject(texSize, gridPlane);
}

void AuScFacePolygon::EffectSpherical(SURFACE_EFFECT effect, quint16 texRepsAtEquator, const AuMatrix4x4& gridPlane)
{
	SetEffect(effect);
	for(AuScTriList::iterator ti = m_Tris.begin(); ti != m_Tris.end(); ti++)
		(*ti).EffectSpherical(texRepsAtEquator, gridPlane);
}

void AuScFacePolygon::EffectMove(const QPointF& vec)
{
	for(AuScTriList::iterator ti = m_Tris.begin(); ti != m_Tris.end(); ti++)
		(*ti).EffectMove(vec);
}

void AuScFacePolygon::EffectMove(int vertexIndex, const QPointF& vec)
{
	int triIndex = vertexIndex / 3;
	int triVertexIndex = vertexIndex % 3;
	return m_Tris[triIndex].EffectMove(triVertexIndex, vec);
}

void AuScFacePolygon::EffectRotate(int vertexIndex, const QPointF& pivot, double angle)
{
	int triIndex = vertexIndex / 3;
	int triVertexIndex = vertexIndex % 3;
	return m_Tris[triIndex].EffectRotate(triVertexIndex, pivot, angle);
}

void AuScFacePolygon::EffectStretch(int vertexIndex, const QPointF& base, double scaleX, double scaleY)
{
	int triIndex = vertexIndex / 3;
	int triVertexIndex = vertexIndex % 3;
	return m_Tris[triIndex].EffectStretch(triVertexIndex, base, scaleX, scaleY);
}

double AuScFacePolygon::Area() const
{
	double a = 0;
	for(AuScTriList::const_iterator ti = m_Tris.cbegin(); ti != m_Tris.cend(); ti++)
		a += (*ti).Area();
	return a;
}

quint64 AuScFacePolygon::MemoryUsage() const
{
	quint64 mem = AuScFace::MemoryUsage();
	mem += m_Edges.MemoryUsage();
	mem += m_Tris.MemoryUsage();
	return mem;
}

void AuScFacePolygon::MorphX(const AuMatrix4x4& fromM, const AuMatrix4x4& toM, double morphDistance)
{
	for(AuScTriList::iterator ti = m_Tris.begin(); ti != m_Tris.end(); ti++)
		(*ti).MorphX(fromM, toM, morphDistance);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScFaceList::AuScFaceList()
{
}

AuScFaceList::AuScFaceList(const AuScFaceList& c) : QList<AuScFace*>()
{
	for(const_iterator fi = c.cbegin(); fi != c.cend(); fi++)
		AddFace(**fi);
}

const AuScFaceList& AuScFaceList::operator=(const AuScFaceList& c)
{
	// Don't have to copy extents because adding the faces is going to recalculate them.
	//m_Max = c.m_Max;
	//m_Min = c.m_Min;

	Clear();
	for(const_iterator fi = c.cbegin(); fi != c.cend(); fi++)
		AddFace(**fi);
	return *this;
}

void AuScFaceList::operator>>(QDataStream& out) const
{
	out << (quint32)size();

	for(const_iterator fi = cbegin(); fi != cend(); fi++)
	{
		out << (quint8)(*fi)->Type();
		out << *(*fi);
	}
}

void AuScFaceList::operator<<(QDataStream& in)
{
	Clear();

	quint32 c;
	in >> c;

	quint8 type;
	while(c--)
	{
		in >> type;
		AuScFace* f = AuScFace::Create(type);
		if(f)
		{
			in >> *f;
			append(f);
		}
	}

	RecalcExtents();
}

bool AuScFaceList::operator==(const AuScFaceList& c) const
{
	if(size() != c.size())
		return false;
	if(m_Max != c.m_Max)
		return false;
	if(m_Min != c.m_Min)
		return false;

	for(const_iterator fi = cbegin(), cfi = c.cbegin(); fi != cend() && cfi != c.cend(); fi++, cfi++)
		if(*(*fi) != *(*cfi))
			return false;

	return true;
}

void AuScFaceList::Clear()
{
	for(iterator ti = begin(); ti != end(); ti++)
		delete *ti;
	clear();
}

void AuScFaceList::Transform(const AuMatrix4x4& t, const QSet<quint32>& vertexIndices, const QSet<quint32>& edgeIndices,
		const QSet<quint32>& faceIndices)
{
	quint32 eindex = 0;
	quint32 findex = 0;
	quint32 vindex = 0;
	for(iterator ti = begin(); ti != end(); ti++)
	{
		(*ti)->Transform(t, vindex, vertexIndices, eindex, edgeIndices, faceIndices.contains(findex));
		eindex += (*ti)->Edges();
		findex++;
		vindex += (*ti)->VertexCount();
	}
	RecalcExtents();
}

void AuScFaceList::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
		const QColor themeColors[AuScFace::THEME_Count]) const
{
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
		(*fi)->RenderTriangles(opaqueTris, transparentTris, themeColors);
}

void AuScFaceList::RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris) const
{
	QColor themeColors[AuScFace::THEME_Count] =
	{
		QColor(255,255,255),
		QColor(255,255,255),
		QColor(255,255,255),
		QColor(255,255,255),
		QColor(255,255,255),
		QColor(255,255,255),
	};
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
		(*fi)->RenderTriangles(opaqueTris, transparentTris, themeColors);
}

void AuScFaceList::Triangles(AuScTriList& tris) const
{
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
		(*fi)->Triangles(tris);
}

void AuScFaceList::AddFace(const AuScFace& f)
{
	if(f.Collapsed())
		return;

	if(empty())
		f.Extents(m_Min, m_Max);
	else
	{
		AuPoint min, max;
		f.Extents(min, max);
		m_Min = m_Min.Min(min);
		m_Max = m_Max.Max(max);
	}

	append(f.Clone());
}

void AuScFaceList::AddFaceIfUnique(const AuScFace& s)
{
	// Look for the face in my list.
	const_iterator fi = FindFaceInSamePlace(s);
	if(fi == cend())
		AddFace(s);						// Face was not found so add it.
}

AuScFaceList::const_iterator AuScFaceList::FindFace(const AuScFace& f) const
{
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
		if(*(*fi) == f)
			return fi;
	return cend();
}

AuScFaceList::const_iterator AuScFaceList::FindFaceInSamePlace(const AuScFace& f) const
{
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
		if((*fi)->SamePlace(f))
			return fi;
	return cend();
}

const AuScFaceQuad* AuScFaceList::FindQuad() const
{
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
	{
		const AuScFaceQuad* q = dynamic_cast<const AuScFaceQuad*>(*fi);
		if(q)
			return q;
	}
	return 0;
}

void AuScFaceList::AddFaces(const AuScFaceList& l)
{
	for(const_iterator fi = l.cbegin(); fi != l.cend(); fi++)
		AddFace(*(*fi));
}

bool AuScFaceList::FindNearestFace(const AuPoint& loc, const AuPoint& dir, AuPoint& faceLoc, AuPoint& faceN) const
{
	// Shoot a ray out of 'loc' and find the closest intersection.
	bool anyHit = false;
	double bestDist2 = -1;
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
	{
		AuPoint i;
		if(!(*fi)->IntersectRay(loc, dir, i))
			continue;

		if(loc.FuzzyEquals(i))
		{
			anyHit = true;
			faceLoc = i;
			faceN = (*fi)->FaceNormal();
			break;
		}
		else
		{
			double dist2 = loc.Distance2(i);
			if(!anyHit || dist2 < bestDist2)
			{
				anyHit = true;
				bestDist2 = dist2;
				faceLoc = i;
				faceN = (*fi)->FaceNormal();
			}
		}
	}

	if(!anyHit)
		return false;

	// The face normal at the hit needs to point in the opposite general direction of dir.
	if(faceN.Dot(dir) > 0)
		faceN = -faceN;					// Reverse the face normal to point against dir.

	return true;
}

// Returns: 1=outside, 0=on the surface, -1=inside
int AuScFaceList::TestInside(const AuPoint& v) const
{
	AuBox box(m_Min, m_Max);
	if(!box.IsPointInside(v))
		return 1;

	// Brutal algorithm. Shoot a ray out of 'v' and count the intersections. Odd number of intersections is inside.
	AuPointArray intersections;
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
	{
		AuPoint i;
		if(!(*fi)->IntersectZ(v, i))
			continue;

		if(FuzzyEquals(i.z, v.z))
			return 0;					// Intersection is at 'v'. It must be on the surface.
		else if(i.z > v.z)
			continue;					// Ignore intersections above the point to treat the line as a ray.

		intersections.AddIfUnique(i);
	}

	// Odd number of intersections is inside.
	return (intersections.size() & 1) ? -1 : 1;

	/*
	if(size() < 2)
		return 1;	// Can't be inside if I don't have any faces.

	// Locate the closest face to 'v'.
	AuScFace* closestFace = 0;
	double closestDist = 0;
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
	{
		double dist = (*fi)->Distance(v);
		if(!closestFace || dist < closestDist)
		{
			if(dist == 0)
				return 0;				// Vertex is exactly in the plane of the face.

			closestFace = *fi;
			closestDist = dist;
		}

	}
	if(!closestFace)
		return 1;	// This should not happen.

	// Locate the second closest face to 'v'.
	AuScFace* secClosestFace = 0;
	double secClosestDist = 0;
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
	{
		if(*fi == closestFace)
			continue;
		double dist = (*fi)->Distance(v);
		if(!secClosestFace || dist < secClosestDist)
		{
			if(dist == 0)
				return 0;				// Vertex is exactly in the plane of the face.

			secClosestFace = *fi;
			secClosestDist = dist;
		}

	}
	if(!secClosestFace)
		return 1;	// This should not happen.

	// Test what side of the closest faces v falls upon. We already know it's not in the plane of either face.
	AuPoint vN;
	AuPoint vv;

	vN = closestFace->FaceNormal();
	vv = v - closestFace->Vertex(0);
	if(vv.IsZero())
		vv = v - closestFace->Vertex(1);
	double fdot = vv.Dot(vN);
	if(fdot > 0)
		return 1;						// 'v' is above the plane of the face.

	vN = secClosestFace->FaceNormal();
	vv = v - secClosestFace->Vertex(0);
	if(vv.IsZero())
		vv = v - secClosestFace->Vertex(1);
	double sdot = vv.Dot(vN);
	if(sdot > 0)
		return 1;						// 'v' is above the plane of the face.

	return -1;						// 'v' is below the plane of the face.
	*/
}

// Returns true if all vertices of 'f' are inside or on the surface.
bool AuScFaceList::TestInside(const AuScFace& f) const
{
	/*
	int vcount = f.VertexCount();
	for(int v=0; v < vcount; v++)
		if(TestInside(f.Vertex(v)) == 1)
			return false;	// Vertex is outside and not on the surface of my faces.
	return true;
			*/
	return TestInside(f.FaceLocation()) == -1;
}

void AuScFaceList::TextureCylindrical(const AuScSelectInfo& selInfo, quint8 texIndex, quint16 texRepsAtEquator, double texSize,
		const AuMatrix4x4& gridPlane)
{
	quint32 findex = 0;
	for(iterator fi = begin(); fi != end(); fi++)
	{
		if(selInfo.m_Faces.contains(findex))
			(*fi)->TextureCylindrical(texIndex, texRepsAtEquator, texSize, gridPlane);
		findex++;
	}
}

void AuScFaceList::TextureProject(const AuScSelectInfo& selInfo, quint8 texIndex, double texSize, const AuMatrix4x4& gridPlane)
{
	quint32 findex = 0;
	for(iterator fi = begin(); fi != end(); fi++)
	{
		if(selInfo.m_Faces.contains(findex))
			(*fi)->TextureProject(texIndex, texSize, gridPlane);
		findex++;
	}
}

void AuScFaceList::TextureSpherical(const AuScSelectInfo& selInfo, quint8 texIndex, quint16 texRepsAtEquator,
		const AuMatrix4x4& gridPlane)
{
	quint32 findex = 0;
	for(iterator fi = begin(); fi != end(); fi++)
	{
		if(selInfo.m_Faces.contains(findex))
			(*fi)->TextureSpherical(texIndex, texRepsAtEquator, gridPlane);
		findex++;
	}
}

void AuScFaceList::EffectCylindrical(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
		double texSize, const AuMatrix4x4& gridPlane)
{
	quint32 findex = 0;
	for(iterator fi = begin(); fi != end(); fi++)
	{
		if(selInfo.m_Faces.contains(findex))
			(*fi)->EffectCylindrical(effect, texRepsAtEquator, texSize, gridPlane);
		findex++;
	}
}

void AuScFaceList::ApplyHullFTLEffect(double texSize, bool shipHull)
{
	quint16 texRepsAtEquator = 4;
	AuMatrix4x4 gridPlane;
	gridPlane.RotateZ(PI/2);
	if(!shipHull)
		gridPlane.RotateY(-PI/2);
	for(iterator fi = begin(); fi != end(); fi++)
		if((*fi)->Effect() == EFF_None || (*fi)->Effect() == EFF_Hull)
			(*fi)->EffectCylindrical(EFF_Hull, texRepsAtEquator, texSize, gridPlane);
}

void AuScFaceList::EffectProject(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize, const AuMatrix4x4& gridPlane)
{
	quint32 findex = 0;
	for(iterator fi = begin(); fi != end(); fi++)
	{
		if(selInfo.m_Faces.contains(findex))
			(*fi)->EffectProject(effect, texSize, gridPlane);
		findex++;
	}
}

void AuScFaceList::EffectSpherical(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
		const AuMatrix4x4& gridPlane)
{
	quint32 findex = 0;
	for(iterator fi = begin(); fi != end(); fi++)
	{
		if(selInfo.m_Faces.contains(findex))
			(*fi)->EffectSpherical(effect, texRepsAtEquator, gridPlane);
		findex++;
	}
}

// Clips clipMe against all of my faces. Fragments that get clipped are put into 'inside' when on the inside of the face that
// clipped them.
void AuScFaceList::Clip(const AuScFace& clipMe, AuScFaceList& inside) const
{
	// We must repeatedly whittle down clipMe against our pieces until its fragments stops getting sliced.
	Q_ASSERT(!clipMe.Collapsed());
	AuScFaceList clipFrags;
	clipFrags.AddFace(clipMe);
	if(clipFrags.empty())
		return;	// Could happen if clipMe was collapsed.

	bool anyCut;
	int countSameCount = 0;
	do
	{
		anyCut = false;

		// Slice the clip fragments against all of my faces.
		AuScFaceList nufrags;
		for(AuScFaceList::const_iterator ci = clipFrags.cbegin(); ci != clipFrags.cend(); ci++)
		{
			// Slice the clip fragment against all of my faces.
			bool cutFace = false;
			for(const_iterator fi = cbegin(); fi != cend(); fi++)
			{
				// Slice the clip fragment against my face, breaking the clip fragment into more fragments.
				if((*fi)->IsPlane())
				{
					AuScFaceList nuclip;
					if((*ci)->Slice(*(*fi), nuclip) && nuclip.size() > 1)
					{
						// The face was sliced but more than one piece resulted.
						// Keep the portion of the clip fragment that is inside my face.
						AuPoint fN((*fi)->FaceNormal());
						while(!nuclip.empty())
						{
							AuScFace* f = nuclip.takeFirst();
							AuPoint fv(f->FaceLocation() - (*fi)->Vertex(0));
							if(fv.Dot(fN) < 0)
								nufrags.AddFaceIfUnique(*f);	// Keep face that is below the plane of my face.
							delete f;
						}
						cutFace = true;
					}
				}
				else
				{
					// Non-planar face must be split into triangles.
					AuScTriList tris;
					(*fi)->Triangles(tris);

					// Slice ci with each of the triangles of the bent face.
					for(AuScTriList::const_iterator ti = tris.cbegin(); ti != tris.cend(); ti++)
					{
						AuScFaceTri ft(**fi, *ti);

						AuScFaceList nuclip;
						if((*ci)->Slice(ft, nuclip) && nuclip.size() > 1)
						{
							// The face was sliced but more than one piece resulted.
							// Keep the portion of the clip fragment that is inside my face.
							AuPoint fN(ft.FaceNormal());
							while(!nuclip.empty())
							{
								AuScFace* f = nuclip.takeFirst();
								AuPoint fv(f->FaceLocation() - ft.Vertex(0));
								if(fv.Dot(fN) < 0)
									nufrags.AddFaceIfUnique(*f);	// Keep face that is below the plane of my face.
								delete f;
							}
							cutFace = true;
						}
					}
				}
			}
			if(!cutFace)
				nufrags.AddFaceIfUnique(*(*ci));
			else
				anyCut = true;
		}
		if(anyCut)
		{
			clipFrags = nufrags;

			// If the number of clip fragments remains unchanged three times in a row, break out of the loop.
			if(clipFrags.size() == nufrags.size())
			{
				countSameCount++;
				if(countSameCount >= 3)
					break;
			}
			else
				countSameCount = 0;
		}
	}
	while(anyCut);

	inside.append(clipFrags);
	clipFrags.clear();
}

// Slices my faces where they are intersected by face 'f', creating more faces in this list.
void AuScFaceList::Slice(const AuScFace& cutter)
{
	// Slice each of my faces into one or two other faces, using the cutter.
	AuScFaceList nufaces;
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
	{
		if((*fi)->IsPlane())
		{
			// Planar faces can be sliced.
			if(!(*fi)->Slice(cutter, nufaces))
				nufaces.AddFace(*(*fi));		// Face was not sliced so add the uncut face to the result.
		}
		else
		{
			// Non-planar faces must be split into triangles before slicing.
			AuScTriList tris;
			(*fi)->Triangles(tris);

			bool anyCut = false;
			AuScFaceList nutris;
			for(AuScTriList::const_iterator ti = tris.cbegin(); ti != tris.cend(); ti++)
			{
				if((*ti).Collapsed())
					continue;
				AuScFaceTri ft(**fi, *ti);
				if(ft.AuScFace::Slice(cutter, nutris))
					anyCut = true;
				else
					nutris.AddFace(ft);
			}

			if(anyCut)
				nufaces.AddFaces(nutris);		// The face got sliced.
			else
				nufaces.AddFace(*(*fi));		// The face did not get sliced.
		}
	}

	// Add my nu cut faces.
	Clear();
	append(nufaces);
	nufaces.clear();
}

void AuScFaceList::Subtract(const AuScFaceList& shell, AuScFaceList& subtracted)
{
	bool anyChange = false;
	iterator fi = begin();
	while(fi != end())
	{
		if(shell.TestInside(*(*fi)))
		{
			subtracted.append(*fi);
			fi = erase(fi);
			anyChange = true;
		}
		else
			fi++;
	}
	if(anyChange)
	{
		subtracted.RecalcExtents();
		RecalcExtents();
	}
}

void AuScFaceList::ReverseTriangleFacing()
{
	for(iterator fi = begin(); fi != end(); fi++)
		(*fi)->ReverseTriangleFacing();
}

void AuScFaceList::FlattenVertexNormals()
{
	for(iterator fi = begin(); fi != end(); fi++)
		(*fi)->FlattenVertexNormals();
}

void AuScFaceList::FlattenVertexNormals(const AuScSelectInfo& selInfo)
{
	quint32 findex = 0;
	quint32 vindex = 0;
	for(iterator fi = begin(); fi != end(); fi++)
	{
		int vcount = (*fi)->VertexCount();
		if(selInfo.m_Faces.contains(findex))
		{
			AuPoint faceN = (*fi)->FaceNormal();
			for(int vi=0; vi<vcount; vi++)
				if(selInfo.m_Vertices.contains(vindex + vi))
					(*fi)->SetVertexNormal(vi, faceN);
		}
		findex++;
		vindex += vcount;
	}
}

// I'm the jig.
void AuScFaceList::JigCut(AuScFaceList& cut1Faces, AuScFaceList& cut2Faces, AuScFaceList& lining) const
{
	// Slice the cut1 faces against this jig.
	for(const_iterator ji = cbegin(); ji != cend(); ji++)
		if(IsFuzzyZero((*ji)->FaceNormal().z))
			cut1Faces.Slice(*(*ji));							// Slice vertically first.
	for(const_iterator ji = cbegin(); ji != cend(); ji++)
		if(FuzzyEquals(fabs((*ji)->FaceNormal().z), 1))
			cut1Faces.Slice(*(*ji));							// Slice horizontally second.
	for(const_iterator ji = cbegin(); ji != cend(); ji++)
		if(!IsFuzzyZero((*ji)->FaceNormal().z) && !FuzzyEquals(fabs((*ji)->FaceNormal().z), 1))
			cut1Faces.Slice(*(*ji));							// Slice diagonally last.

	// Slice the cut2 faces against this jig.
	for(const_iterator ji = cbegin(); ji != cend(); ji++)
		if(IsFuzzyZero((*ji)->FaceNormal().z))
			cut2Faces.Slice(*(*ji));							// Slice vertically first.
	for(const_iterator ji = cbegin(); ji != cend(); ji++)
		if(FuzzyEquals(fabs((*ji)->FaceNormal().z), 1))
			cut2Faces.Slice(*(*ji));							// Slice horizontally second.
	for(const_iterator ji = cbegin(); ji != cend(); ji++)
		if(!IsFuzzyZero((*ji)->FaceNormal().z) && !FuzzyEquals(fabs((*ji)->FaceNormal().z), 1))
			cut2Faces.Slice(*(*ji));							// Slice diagonally last.

	// Clip the cutter faces against the resulting cut faces to create the interconnecting liner.
	AuScFaceList firstClip;
	for(const_iterator ji = cbegin(); ji != cend(); ji++)
		cut1Faces.Clip(*(*ji), firstClip);

	AuScFaceList secondClip;
	for(const_iterator ji = firstClip.cbegin(); ji != firstClip.cend(); ji++)
		cut2Faces.Clip(*(*ji), secondClip);

	// Second clip contains unwanted polygons left over from the jig.
	// Build the lining from faces of the second clip result that actually touch a cut2 face.
	lining.Clear();
	iterator ji = secondClip.begin();
	while(ji != secondClip.end())
	{
		if(cut2Faces.ColinearTriangleEdge(*(*ji)))
		{
			lining.append(*ji);
			ji = secondClip.erase(ji);
		}
		else
			ji++;
	}

	// Now repeat that until we stop finding polygons contiguous with the lining polygons.
	bool foundTouching;
	do
	{
		foundTouching = false;
		iterator ji = secondClip.begin();
		while(ji != secondClip.end())
		{
			if(lining.ColinearTriangleEdge(*(*ji)))
			{
				lining.append(*ji);
				ji = secondClip.erase(ji);
				foundTouching = true;
			}
			else
				ji++;
		}
	}
	while(foundTouching);

	// Lining still contains multiple overlapping faces. Eliminate those.
	lining.JoinFaces();

	lining.FlattenVertexNormals();
}

void AuScFaceList::RecalcExtents()
{
	if(empty())
	{
		m_Min = AuPoint(0,0);
		m_Max = AuPoint(0,0);
		return;
	}

	first()->Extents(m_Min, m_Max);
	if(size() == 1)
		return;

	AuPoint min, max;
	for(const_iterator fi = cbegin()+1; fi != cend(); fi++)
	{
		(*fi)->Extents(min, max);
		m_Min = m_Min.Min(min);
		m_Max = m_Max.Max(max);
	}
}

bool AuScFaceList::ColinearTriangleEdge(const AuScFace& face) const
{
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
		if((*fi)->ColinearTriangleEdge(face))
			return true;
	return false;
}

double AuScFaceList::VertexDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& vertexIndex, AuPoint* snapLoc) const
{
	double zNear = 0;	//mvp.PerspectiveZNear();

	AuPoint fpx(pixel);
	double closestDist2 = d_SearchTolerance*d_SearchTolerance;	// We won't find anything that is not closer than this dist.
	bool foundMe = false;

	quint32 vindex = 0;
	QMap<AuPoint, bool> alreadyChecked;
	for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
	{
		AuPointArray verts;
		(*ti)->VertexLocations(verts);
		for(AuPointArray::const_iterator vi = verts.cbegin(); vi != verts.cend(); vi++)
		{
			if(!alreadyChecked.contains(*vi))
			{
				alreadyChecked.insert(*vi, true);

				AuPoint vloc = mvp.Project(*vi);
				if(vloc.z <= zNear)
					continue;		// Behind eye plane.

				double dist2 = vloc.Distance2_2D(fpx);
				if(dist2 < closestDist2)
				{
					closestDist2 = dist2;
					vertexIndex = vindex;
					if(snapLoc)
						*snapLoc = *vi;
					foundMe = true;
				}
			}
			vindex++;
		}
	}

	if(!foundMe)
		return d_SearchTolerance;

	return sqrt(closestDist2);
}

double AuScFaceList::FaceDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& faceIndex, AuPoint& gripLoc) const
{
	double zNear = 0;	//mvp.PerspectiveZNear();

	AuPoint fpx(pixel);
	double closestDist2 = d_SearchTolerance*d_SearchTolerance;	// We won't find anything that is not closer than this dist.
	bool foundMe = false;

	quint32 findex = 0;
	QMap<AuPoint, bool> alreadyChecked;
	for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
	{
		AuPoint v = (*ti)->FaceLocation();
		if(!alreadyChecked.contains(v))
		{
			alreadyChecked.insert(v, true);

			AuPoint floc = mvp.Project(v);
			if(floc.z <= zNear)
				continue;			// Behind eye plane.

			double dist2 = floc.Distance2_2D(fpx);
			if(dist2 < closestDist2)
			{
				closestDist2 = dist2;
				faceIndex = findex;
				gripLoc = v;
				foundMe = true;
			}
		}
		findex++;
	}

	if(!foundMe)
		return d_SearchTolerance;

	return sqrt(closestDist2);
}

bool AuScFaceList::InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelectInfo& selInfo, bool findVertices,
		bool findEdges, bool findFaces) const
{
	double zNear = 0;	//mvp.PerspectiveZNear();

	bool anyInRect = false;

	if(findVertices)
	{
		// Test all projected vertices to see if they are inside the pixel rectangle.
		quint32 vertexIndex = 0;
		for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
		{
			int verts = (*ti)->VertexCount();
			for(int v=0; v<verts; v++)
			{
				AuPoint vloc = mvp.Project((*ti)->Vertex(v));
				if(vloc.z > zNear && pixelRect.contains(QPointF(vloc.x, vloc.y)))
				{
					selInfo.m_Vertices.insert(vertexIndex);
					anyInRect = true;
				}
				vertexIndex++;
			}
		}
	}

	if(findFaces)
	{
		quint32 faceIndex = 0;
		for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
		{
			AuPoint floc = mvp.Project((*ti)->FaceLocation());
			if(floc.z > zNear && pixelRect.contains(QPointF(floc.x, floc.y)))
			{
				selInfo.m_Faces.insert(faceIndex);
				anyInRect = true;
			}
			faceIndex++;
		}
	}

	if(findEdges)
	{
		quint32 edgeIndex = 0;
		for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
		{
			int segs = (*ti)->Edges();
			AuSegment seg;
			for(int s=0; s<segs; s++)
			{
				(*ti)->EdgeSegment(s, seg);
				seg.Transform(mvp);
				if((seg.m_fLoc1.z > zNear || seg.m_fLoc2.z > zNear) &&
						pixelRect.contains(QPointF(seg.m_fLoc1.x, seg.m_fLoc1.y)) &&
						pixelRect.contains(QPointF(seg.m_fLoc2.x, seg.m_fLoc2.y)))
				{
					selInfo.m_Edges.insert(edgeIndex);
					anyInRect = true;
				}
				edgeIndex++;
			}
		}
	}

	return anyInRect;
}

// Builds quads and polygons from coplanar faces that share an edge.
void AuScFaceList::JoinFaces()
{
	if(size() <= 1)
		return;

	// This sometimes takes a couple of passes to stop finding things to join together.
	bool listJoined;
	do
	{
		// Do a pass until we stop joining faces.
		listJoined = false;

		AuScFaceList nuFaceList;
		while(!empty())
		{
			// Grab a face.
			AuScFace* f = takeFirst();

			// Join faces to it until no more will join.
			bool anyJoined;
			do
			{
				anyJoined = false;
				iterator ai = begin();
				while(ai != end())
				{
					AuScFace* nuf = f->Join(*(*ai));
					if(nuf)
					{
						// Face ai joined with f to produce nuf.
						delete *ai;
						delete f;
						f = nuf;
						ai = erase(ai);
						anyJoined = true;
						listJoined = true;
					}
					else
						ai++;
				}
			}
			while(anyJoined);

			nuFaceList.append(f);
		}

		*this = nuFaceList;
	}
	while(listJoined);
}

bool AuScFaceList::CutFaces(const AuPointArray& cutLine)
{
	bool anyCut = false;
	AuScFaceList myFaces(*this);				// These are the faces we will cut.
	AuScFaceList nuFaceList;					// The result of the cut.
	while(!myFaces.empty())
	{
		// Grab a face.
		AuScFace* f = myFaces.takeFirst();

		// Attempt to cut f using the cutLine.
		if(f->Cut(cutLine, nuFaceList))
		{
			delete f;							// f was cut. All resulting pieces placed in nuFaceList.
			anyCut = true;
		}
		else
			nuFaceList.append(f);				// f was not cut.
	}

	if(!anyCut)
		return false;

	*this = nuFaceList;

	return true;
}

bool AuScFaceList::PunchFaces(const AuPointArray& cutBnd)
{
	// Insure the boundary is closed.
	AuPointArray bnd(cutBnd);
	if(bnd.first() != bnd.last())
		bnd.append(bnd.first());

	bool anyCut = false;
	AuScFaceList myFaces(*this);				// These are the faces we will cut.
	AuScFaceList nuFaceList;					// The result of the cut.
	while(!myFaces.empty())
	{
		// Grab a face.
		AuScFace* f = myFaces.takeFirst();

		// Attempt to cut f using the bnd.
		if(f->Punch(bnd, nuFaceList))
		{
			delete f;							// f was cut. All resulting pieces placed in nuFaceList.
			anyCut = true;
		}
		else
			nuFaceList.append(f);				// f was not cut.
	}

	if(!anyCut)
		return false;

	*this = nuFaceList;

	return true;
}

bool AuScFaceList::Fracture(const AuScSelectInfo& selInfo)
{
	AuScFaceList nufaces;
	QSet<quint32> facesChanged;
	for(QSet<quint32>::const_iterator sfi = selInfo.m_Faces.cbegin(); sfi != selInfo.m_Faces.cend(); sfi++)
	{
		if(*sfi >= (quint32)size())
			continue;

		AuScFaceList::const_iterator fi = begin() + *sfi;
		AuScTriList tris;
		(*fi)->Triangles(tris);
		if(tris.size() <= 1)
			continue;

		// Create a new triangle face from every face triangle.
		for(AuScTriList::const_iterator ti = tris.cbegin(); ti != tris.cend(); ti++)
			nufaces.AddFace(AuScFaceTri(**fi, *ti));

		// Remember the ones we fracture so we can delete them.
		facesChanged.insert(*sfi);
	}

	if(facesChanged.empty())
		return false;

	DeleteFaces(facesChanged);
	AddFaces(nufaces);
	return true;
}

bool AuScFaceList::Fuse(const AuScSelectInfo& selInfo)
{
	// Make a face list that contains only the selected faces.
	AuScFaceList fusedFaces;
	for(QSet<quint32>::const_iterator sfi = selInfo.m_Faces.cbegin(); sfi != selInfo.m_Faces.cend(); sfi++)
	{
		if(*sfi >= (quint32)size())
			continue;

		AuScFaceList::const_iterator fi = begin() + *sfi;
		fusedFaces.AddFace(**fi);
	}

	if(fusedFaces.empty())
		return false;

	int presize = fusedFaces.size();
	fusedFaces.JoinFaces();
	if(presize == fusedFaces.size())
		return false;

	DeleteFaces(selInfo.m_Faces);
	AddFaces(fusedFaces);
	return true;
}

void AuScFaceList::SmoothVertices()
{
	QMultiMap<AuPoint, AuPoint> verts;	// Key is vertex, value is normal perpendicular to its triangle.
	for(iterator fi = begin(); fi != end(); fi++)
	{
		int vcount = (*fi)->VertexCount();
		AuPoint faceN = (*fi)->FaceNormal();
		for(int vi=0; vi<vcount; vi++)
			verts.insert((*fi)->Vertex(vi), faceN);
	}

	AuPoint curVert;
	AuPoint curN;
	int curCount = 0;
	QMap<AuPoint, AuPoint> smoothVerts;	// Key is vertex, value is smooth normal.
	for(QMultiMap<AuPoint, AuPoint>::const_iterator vi = verts.cbegin(); vi != verts.cend(); vi++)
	{
		if(curCount == 0)
		{
			// First vertex in the mulitimap.
			curVert = vi.key();
			curN = vi.value();
			curCount = 1;
		}
		else if(vi.key().FuzzyEquals(curVert))
		{
			// Subsequent matching vertex. Accumulate normals so we can average them.
			curN += vi.value();
			curCount++;
		}
		else
		{
			// Subsequent vertex did not match. Calculate accumulated average of current normal.
			if(curCount > 1)
			{
				curN /= curCount;
				curN.Unit();
			}
			smoothVerts.insert(curVert, curN);

			// Start accumulating next vertex.
			curVert = vi.key();
			curN = vi.value();
			curCount = 1;
		}

		if(vi == verts.cend()-1)
		{
			// We just finished the last vertex. Close out our last accumulated vertex normal.
			if(curCount > 1)
			{
				curN /= curCount;
				curN.Unit();
			}
			smoothVerts.insert(curVert, curN);
		}
	}

	// Update all my triangles with new smooth normals.
	for(iterator fi = begin(); fi != end(); fi++)
	{
		int vcount = (*fi)->VertexCount();
		for(int vi=0; vi<vcount; vi++)
		{
			AuScPoint p((*fi)->Vertex(vi));
			QMap<AuPoint, AuPoint>::const_iterator si = smoothVerts.find(p);
			if(si == smoothVerts.cend())
				for(si = smoothVerts.cbegin(); si != smoothVerts.cend(); si++)
					if(si.key().FuzzyEquals(p))
						break;
			if(si != smoothVerts.cend())
				(*fi)->SetVertexNormal(vi, *si);
		}
	}
}

void AuScFaceList::SmoothVertices(const AuScSelectInfo& selInfo)
{
	QMultiMap<AuPoint, AuPoint> verts;	// Key is vertex, value is normal perpendicular to its triangle.
	quint32 findex = 0;
	quint32 vindex = 0;
	for(iterator fi = begin(); fi != end(); fi++)
	{
		int vcount = (*fi)->VertexCount();
		if(selInfo.m_Faces.contains(findex))
		{
			AuPoint faceN = (*fi)->FaceNormal();
			for(int vi=0; vi<vcount; vi++)
				if(selInfo.m_Vertices.contains(vindex + vi))
					verts.insert((*fi)->Vertex(vi), faceN);
		}
		findex++;
		vindex += vcount;
	}

	AuPoint curVert;
	AuPoint curN;
	int curCount = 0;
	QMap<AuPoint, AuPoint> smoothVerts;	// Key is vertex, value is smooth normal.
	for(QMultiMap<AuPoint, AuPoint>::const_iterator vi = verts.cbegin(); vi != verts.cend(); vi++)
	{
		if(curCount == 0)
		{
			// First vertex in the mulitimap.
			curVert = vi.key();
			curN = vi.value();
			curCount = 1;
		}
		else if(vi.key().FuzzyEquals(curVert))
		{
			// Subsequent matching vertex. Accumulate normals so we can average them.
			curN += vi.value();
			curCount++;
		}
		else
		{
			// Subsequent vertex did not match. Calculate accumulated average of current normal.
			if(curCount > 1)
			{
				curN /= curCount;
				curN.Unit();
			}
			smoothVerts.insert(curVert, curN);

			// Start accumulating next vertex.
			curVert = vi.key();
			curN = vi.value();
			curCount = 1;
		}

		if(vi == verts.cend()-1)
		{
			// We just finished the last vertex. Close out our last accumulated vertex normal.
			if(curCount > 1)
			{
				curN /= curCount;
				curN.Unit();
			}
			smoothVerts.insert(curVert, curN);
		}
	}

	// Update all my triangles with new smooth normals.
	findex = 0;
	vindex = 0;
	for(iterator fi = begin(); fi != end(); fi++)
	{
		int vcount = (*fi)->VertexCount();
		if(selInfo.m_Faces.contains(findex))
		{
			for(int vi=0; vi<vcount; vi++)
				if(selInfo.m_Vertices.contains(vindex + vi))
				{
					AuScPoint p((*fi)->Vertex(vi));
					QMap<AuPoint, AuPoint>::const_iterator si = smoothVerts.find(p);
					if(si == smoothVerts.cend())
						for(si = smoothVerts.cbegin(); si != smoothVerts.cend(); si++)
							if(si.key().FuzzyEquals(p))
								break;
					if(si != smoothVerts.cend())
						(*fi)->SetVertexNormal(vi, *si);
				}
		}
		findex++;
		vindex += vcount;
	}
}

void AuScFaceList::DivideFaces(const AuScSelectInfo& selInfo, quint8 divisions)
{
	AuScFaceList nufaces;
	quint32 findex = 0;
	for(iterator fi = begin(); fi != end(); fi++)
	{
		if(selInfo.m_Faces.contains(findex))
			(*fi)->Divide(divisions, nufaces);
		findex++;
	}

	DeleteFaces(selInfo.m_Faces);
	AddFaces(nufaces);
}

void AuScFaceList::PolishFaces(const AuScSelectInfo& selInfo, float shine, float reflectivity)
{
	quint32 findex = 0;
	for(iterator fi = begin(); fi != end(); fi++)
	{
		if(selInfo.m_Faces.contains(findex))
		{
			(*fi)->SetShine(shine);
			(*fi)->SetReflectivity(reflectivity);
		}
		findex++;
	}
}

void AuScFaceList::EffectIntensityVertices(const AuScSelectInfo& selInfo, float intensity)
{
	quint32 findex = 0;
	quint32 vindex = 0;
	for(iterator fi = begin(); fi != end(); fi++)
	{
		int vcount = (*fi)->VertexCount();
		if(selInfo.m_Faces.contains(findex))
		{
			for(int vi=0; vi<vcount; vi++)
				if(selInfo.m_Vertices.contains(vindex + vi))
					(*fi)->SetVertexEffectIntensity(vi, intensity);
		}
		findex++;
		vindex += vcount;
	}
}

void AuScFaceList::GlowVertices(const AuScSelectInfo& selInfo, const QColor& glow)
{
	quint32 findex = 0;
	quint32 vindex = 0;
	for(iterator fi = begin(); fi != end(); fi++)
	{
		int vcount = (*fi)->VertexCount();
		if(selInfo.m_Faces.contains(findex))
		{
			for(int vi=0; vi<vcount; vi++)
				if(selInfo.m_Vertices.contains(vindex + vi))
					(*fi)->SetVertexGlow(vi, glow);
		}
		findex++;
		vindex += vcount;
	}
}

void AuScFaceList::PaintFaces(const AuScSelectInfo& selInfo, const QColor& paint)
{
	quint32 findex = 0;
	for(iterator fi = begin(); fi != end(); fi++)
	{
		if(selInfo.m_Faces.contains(findex))
			(*fi)->SetPaint(paint);
		findex++;
	}
}

void AuScFaceList::PaintFaces(const AuScSelectInfo& selInfo, AuScFace::COLOR_THEME theme, quint8 opacity)
{
	quint32 findex = 0;
	for(iterator fi = begin(); fi != end(); fi++)
	{
		if(selInfo.m_Faces.contains(findex))
			(*fi)->SetPaint(theme, opacity);
		findex++;
	}
}

void AuScFaceList::SetSelfIllumination(float emission)
{
	for(iterator fi = begin(); fi != end(); fi++)
		(*fi)->SetSelfIllumination(emission);
}

void AuScFaceList::SetSelfIllumination(const AuScSelectInfo& selInfo, float emission)
{
	quint32 findex = 0;
	for(iterator fi = begin(); fi != end(); fi++)
	{
		if(selInfo.m_Faces.contains(findex))
			(*fi)->SetSelfIllumination(emission);
		findex++;
	}
}

void AuScFaceList::SetOpacity(float opacity)
{
	for(iterator fi = begin(); fi != end(); fi++)
		(*fi)->SetOpacity(opacity);
}

void AuScFaceList::SetOpacity(const AuScSelectInfo& selInfo, float opacity)
{
	quint32 findex = 0;
	for(iterator fi = begin(); fi != end(); fi++)
	{
		if(selInfo.m_Faces.contains(findex))
			(*fi)->SetOpacity(opacity);
		findex++;
	}
}

void AuScFaceList::SetTextureIndex(quint8 texture)
{
	for(iterator fi = begin(); fi != end(); fi++)
		(*fi)->SetTextureIndex(texture);
}

void AuScFaceList::SetTextureIndex(const AuScSelectInfo& selInfo, quint8 texture)
{
	quint32 findex = 0;
	for(iterator fi = begin(); fi != end(); fi++)
	{
		if(selInfo.m_Faces.contains(findex))
			(*fi)->SetTextureIndex(texture);
		findex++;
	}
}

qint8 AuScFaceList::TextureIndex(const AuScSelectInfo& selInfo) const
{
	qint8 texI = -2;	// -2=no selected faces, -1=tex indexes differ, >=0 = tex index
	quint32 findex = 0;
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
	{
		if(selInfo.m_Faces.contains(findex))
		{
			if(texI == -2)
				texI = (*fi)->TextureIndex();					// First selected face.
			else if(texI != (*fi)->TextureIndex())
				return -1;										// Texture indexes differ. No need to keep looking.
		}
		findex++;
	}
	return texI;
}

void AuScFaceList::SetEffect(SURFACE_EFFECT effect)
{
	for(iterator fi = begin(); fi != end(); fi++)
		(*fi)->SetEffect(effect);
}

void AuScFaceList::SetEffect(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect)
{
	quint32 findex = 0;
	for(iterator fi = begin(); fi != end(); fi++)
	{
		if(selInfo.m_Faces.contains(findex))
			(*fi)->SetEffect(effect);
		findex++;
	}
}

qint8 AuScFaceList::Effect(const AuScSelectInfo& selInfo) const
{
	qint8 texI = -2;	// -2=no selected faces, -1=effects differ, >=0 = tex index
	quint32 findex = 0;
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
	{
		if(selInfo.m_Faces.contains(findex))
		{
			if(texI == -2)
				texI = (*fi)->Effect();							// First selected face.
			else if(texI != (*fi)->Effect())
				return -1;										// Effects differ. No need to keep looking.
		}
		findex++;
	}
	return texI;
}

AuScPoint AuScFaceList::VertexLocation(quint32 vertexIndex) const
{
	quint32 vindex = 0;
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
	{
		int vcount = (*fi)->VertexCount();
		quint32 vnext = vindex + vcount;

		if(vnext > vertexIndex)
		{
			// vertexIndex is within the fi face.
			// Calculate face vertex index.
			quint32 fverti = vertexIndex - vindex;
			return (*fi)->Vertex(fverti);
		}

		vindex = vnext;
	}
	return AuScPoint();
}

void AuScFaceList::VertexLocations(AuPointArray& verts) const
{
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
		(*fi)->VertexLocations(verts);
}

void AuScFaceList::TextureMove(const AuScSelectInfo& selInfo, const QPointF& vec)
{
	quint32 findex = 0;
	quint32 vindex = 0;
	for(iterator fi = begin(); fi != end(); fi++)
	{
		int vcount = (*fi)->VertexCount();
		if(selInfo.m_Faces.contains(findex))
		{
			for(int vi=0; vi<vcount; vi++)
				if(selInfo.m_Vertices.contains(vindex + vi))
					(*fi)->TextureMove(vi, vec);
		}
		findex++;
		vindex += vcount;
	}
}

void AuScFaceList::TextureRotate(const AuScSelectInfo& selInfo, const QPointF& pivot, double angle)
{
	quint32 findex = 0;
	quint32 vindex = 0;
	for(iterator fi = begin(); fi != end(); fi++)
	{
		int vcount = (*fi)->VertexCount();
		if(selInfo.m_Faces.contains(findex))
		{
			for(int vi=0; vi<vcount; vi++)
				if(selInfo.m_Vertices.contains(vindex + vi))
					(*fi)->TextureRotate(vi, pivot, angle);
		}
		findex++;
		vindex += vcount;
	}
}

void AuScFaceList::TextureStretch(const AuScSelectInfo& selInfo, const QPointF& base, double scaleX, double scaleY)
{
	quint32 findex = 0;
	quint32 vindex = 0;
	for(iterator fi = begin(); fi != end(); fi++)
	{
		int vcount = (*fi)->VertexCount();
		if(selInfo.m_Faces.contains(findex))
		{
			for(int vi=0; vi<vcount; vi++)
				if(selInfo.m_Vertices.contains(vindex + vi))
					(*fi)->TextureStretch(vi, base, scaleX, scaleY);
		}
		findex++;
		vindex += vcount;
	}
}

void AuScFaceList::EffectMove(const AuScSelectInfo& selInfo, const QPointF& vec)
{
	quint32 findex = 0;
	quint32 vindex = 0;
	for(iterator fi = begin(); fi != end(); fi++)
	{
		int vcount = (*fi)->VertexCount();
		if(selInfo.m_Faces.contains(findex))
		{
			for(int vi=0; vi<vcount; vi++)
				if(selInfo.m_Vertices.contains(vindex + vi))
					(*fi)->EffectMove(vi, vec);
		}
		findex++;
		vindex += vcount;
	}
}

void AuScFaceList::EffectRotate(const AuScSelectInfo& selInfo, const QPointF& pivot, double angle)
{
	quint32 findex = 0;
	quint32 vindex = 0;
	for(iterator fi = begin(); fi != end(); fi++)
	{
		int vcount = (*fi)->VertexCount();
		if(selInfo.m_Faces.contains(findex))
		{
			for(int vi=0; vi<vcount; vi++)
				if(selInfo.m_Vertices.contains(vindex + vi))
					(*fi)->EffectRotate(vi, pivot, angle);
		}
		findex++;
		vindex += vcount;
	}
}

void AuScFaceList::EffectStretch(const AuScSelectInfo& selInfo, const QPointF& base, double scaleX, double scaleY)
{
	quint32 findex = 0;
	quint32 vindex = 0;
	for(iterator fi = begin(); fi != end(); fi++)
	{
		int vcount = (*fi)->VertexCount();
		if(selInfo.m_Faces.contains(findex))
		{
			for(int vi=0; vi<vcount; vi++)
				if(selInfo.m_Vertices.contains(vindex + vi))
					(*fi)->EffectStretch(vi, base, scaleX, scaleY);
		}
		findex++;
		vindex += vcount;
	}
}

bool AuScFaceList::SelectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	bool anyChange = false;

	// Add indexes of all overlapping vertices.
	quint32 findex = 0;
	quint32 vindex = 0;
	for(const_iterator ti = constBegin(); ti != constEnd(); ti++, findex++)
	{
		int verts = (*ti)->VertexCount();
		if(selInfo.m_Faces.contains(findex))
		{
			for(int vi=0; vi<verts; vi++)
			{
				AuScPoint v = (*ti)->Vertex(vi);
				if(FuzzyEquals(v.m_Tx, (float)texLoc.x()) && FuzzyEquals(v.m_Ty, (float)texLoc.y()) && !selInfo.m_Vertices.contains(vindex))
				{
					selInfo.m_Vertices.insert(vindex);
					anyChange = true;
				}
				vindex++;
			}
		}
		else
			vindex += verts;
	}

	return anyChange;
}

bool AuScFaceList::DeselectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	bool anyChange = false;

	// Remove indexes of all overlapping vertices.
	quint32 findex = 0;
	quint32 vindex = 0;
	for(const_iterator ti = constBegin(); ti != constEnd(); ti++, findex++)
	{
		int verts = (*ti)->VertexCount();
		if(selInfo.m_Faces.contains(findex))
		{
			for(int vi=0; vi<verts; vi++)
			{
				AuScPoint v = (*ti)->Vertex(vi);
				if(FuzzyEquals(v.m_Tx, (float)texLoc.x()) && FuzzyEquals(v.m_Ty, (float)texLoc.y()) && selInfo.m_Vertices.contains(vindex))
				{
					selInfo.m_Vertices.remove(vindex);
					anyChange = true;
				}
				vindex++;
			}
		}
		else
			vindex += verts;
	}

	return anyChange;
}

bool AuScFaceList::SelectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	bool anyChange = false;

	// Add indexes of all overlapping vertices.
	quint32 findex = 0;
	quint32 vindex = 0;
	for(const_iterator ti = constBegin(); ti != constEnd(); ti++, findex++)
	{
		int verts = (*ti)->VertexCount();
		if(selInfo.m_Faces.contains(findex))
		{
			for(int vi=0; vi<verts; vi++)
			{
				AuScPoint v = (*ti)->Vertex(vi);
				if(FuzzyEquals(v.EffectTx(), (float)texLoc.x()) && FuzzyEquals(v.EffectTy(), (float)texLoc.y()) &&
						!selInfo.m_Vertices.contains(vindex))
				{
					selInfo.m_Vertices.insert(vindex);
					anyChange = true;
				}
				vindex++;
			}
		}
		else
			vindex += verts;
	}

	return anyChange;
}

bool AuScFaceList::DeselectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const
{
	bool anyChange = false;

	// Remove indexes of all overlapping vertices.
	quint32 findex = 0;
	quint32 vindex = 0;
	for(const_iterator ti = constBegin(); ti != constEnd(); ti++, findex++)
	{
		int verts = (*ti)->VertexCount();
		if(selInfo.m_Faces.contains(findex))
		{
			for(int vi=0; vi<verts; vi++)
			{
				AuScPoint v = (*ti)->Vertex(vi);
				if(FuzzyEquals(v.EffectTx(), (float)texLoc.x()) && FuzzyEquals(v.EffectTy(), (float)texLoc.y()) &&
						selInfo.m_Vertices.contains(vindex))
				{
					selInfo.m_Vertices.remove(vindex);
					anyChange = true;
				}
				vindex++;
			}
		}
		else
			vindex += verts;
	}

	return anyChange;
}

void AuScFaceList::TextureUnwrap(const AuScSelectInfo& selInfo, quint8 texIndex, double texSize, UNWRAP_METHOD unwrap,
		float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	switch(unwrap)
	{
	case UNWRAP_ByFace:
		TextureUnwrapByFace(selInfo, texIndex, texSize, gridAlign, alignAxis, gridPlane);
		break;

	case UNWRAP_ByFacing:
		TextureUnwrapByFacing(selInfo, texIndex, texSize, angleTol, gridAlign, alignAxis, gridPlane);
		break;

	case UNWRAP_Edges:
		TextureUnwrapEdges(selInfo, texIndex, texSize, angleTol, gridAlign, alignAxis, gridPlane);
		break;

	case UNWRAP_Horizontal:
		TextureUnwrapHorizontal(selInfo, texIndex, texSize, angleTol, gridAlign, alignAxis, gridPlane);
		break;

	case UNWRAP_Vertical:
		TextureUnwrapVertical(selInfo, texIndex, texSize, angleTol, gridAlign, alignAxis, gridPlane);
		break;
	}
}

void AuScFaceList::TextureUnwrapByFace(const AuScSelectInfo& selInfo, quint8 texIndex, double texSize, bool gridAlign,
		int alignAxis, const AuMatrix4x4& gridPlane)
{
	int findex=0;
	for(iterator fi = begin(); fi != end(); fi++, findex++)
		if(selInfo.m_Faces.contains(findex))
			(*fi)->TextureUnwrap(texIndex, texSize, gridAlign, alignAxis, gridPlane);
}

void AuScFaceList::TextureUnwrapByFacing(const AuScSelectInfo& selInfo, quint8 texIndex, double texSize, float angleTol,
		bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	Transform(gridPlane.Inverse());

	unsigned int sanityCount = size() * size();	// So we don't get stuck doing this forever.
	QSet<int> facesDone;
	iterator fi;
	do
	{
		// Find the next selected face that has not been done.
		int findex=0;
		fi = begin();
		while(fi != end())
		{
			if(selInfo.m_Faces.contains(findex) && !facesDone.contains(findex))
				break;	// Found selected face that has not been done.
			fi++;
			findex++;
		}

		// We're done if we found no unfinished selected faces.
		if(fi != end())
		{
			// Found an unfinished selected face. This is our reference for all faces facing the same direction.
			// Get the plane matrix for the face.
			AuMatrix4x4 fplane;
			fplane.Rotate((*fi)->FacePlane().GetRotation());
			fplane.RotateX(PI);									// Flip vertically.

			if(gridAlign)
			{
				// Align the plane to a grid axis.
				double zrot;
				switch(alignAxis)
				{
				case 0:	// x
					// Grid X vector in the plane of the face.
					zrot = fplane.UnProjectVector(AuPoint(1,0,0)).ZRotation() - PI/2;
					break;
				case 1:	// y
					// Grid Y vector in the plane of the face.
					zrot = fplane.UnProjectVector(AuPoint(0,1,0)).ZRotation() - PI/2;
					break;
				case 2:	// z
					// Grid Z vector in the plane of the face.
					zrot = fplane.UnProjectVector(AuPoint(0,0,1)).ZRotation() - PI/2;
					break;
				default:
					zrot = 0;
					break;
				}
				if(zrot != 0)
					fplane.RotateZ(zrot);
			}

			AuPoint fn((*fi)->FaceNormal());

			// All unfinished faces with the same facing will be textured using the same plane as the reference face.
			findex = 0;
			for(iterator sfi = begin(); sfi != end(); sfi++, findex++)
				if(selInfo.m_Faces.contains(findex) && !facesDone.contains(findex) && (*sfi)->FaceNormal().AngleTo(fn) <= angleTol)
				{
					// Found selected face that has not been done that is close enough to parallel with the reference face.
					(*sfi)->TextureProject(texIndex, texSize, fplane);
					facesDone.insert(findex);
				}
		}
	}
	while(fi != end() && sanityCount--);

	Transform(gridPlane);
}

void AuScFaceList::TextureUnwrapEdges(const AuScSelectInfo& selInfo, quint8 texIndex, double texSize, float angleTol,
		bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	TextureUnwrapByFacing(selInfo, texIndex, texSize, angleTol, gridAlign, alignAxis, gridPlane);

	// Adjust textures so they continue from one face to the next.
	QSet<int> facesDone;
	iterator fi = end();
	bool didAny;
	do
	{
		didAny = false;

		// fi points to our reference face for wrapping.
		// Each loop ends with fi pointing to a face we just did, so it is our reference for the next pass.
		int findex=0;
		if(fi == end())
		{
			// First time through the loop.
			// Find the next selected face that has not been done.
			fi = begin();
			while(fi != end())
			{
				if(selInfo.m_Faces.contains(findex) && !facesDone.contains(findex))
					break;	// Found selected face that has not been done.
				fi++;
				findex++;
			}

			if(fi != end())
			{
				// Keep texture applied in TextureUnwrapByFacing.
				facesDone.insert(findex);
				didAny = true;
			}
		}

		// We're done if we have no reference face.
		if(fi != end())
		{
			// Look for an unfinished face that is adjacent to the reference face.
			findex = 0;
			for(iterator sfi = begin(); sfi != end(); sfi++, findex++)
			{
				// Skip unselected faces and faces that have already been done.
				if(!selInfo.m_Faces.contains(findex) || facesDone.contains(findex))
					continue;

				// Check for a common edge and get the vertices.
				AuScPoint myv1, myv2;
				if(!(*fi)->CommonEdgeVertices(**sfi, myv1, myv2))
					continue;

				// We share a common edge. This will be our basis for adjusting the texture on the adjacent face.
				int ov1i = (*sfi)->VertexAt(myv1);
				int ov2i = (*sfi)->VertexAt(myv2);
				if(ov1i < 0 || ov2i < 0)
					continue;
				AuScPoint ov1 = (*sfi)->Vertex(ov1i);
				AuScPoint ov2 = (*sfi)->Vertex(ov2i);

				// This will wrap nicely if the texture offset is the same at each vertex.
				AuPoint tvec1(ov1.TexCoord() - myv1.TexCoord());
				AuPoint tvec2(ov2.TexCoord() - myv2.TexCoord());
				if(!tvec1.FuzzyEquals(tvec2))
					continue;

				(*sfi)->TextureMove(QPointF((float)tvec1.x, (float)tvec1.y));
				facesDone.insert(findex);
				didAny = true;

				fi = sfi;
				break;
			}

			if(!didAny)
			{
				// We get here if the loop started with a valid fi and we did not find any adjacent faces.
				// Now we want to do the loop again with a fresh fi, if any can be found.
				fi = end();
				didAny = true;	// Keep the loop going for another round, to look for an unfinished fi one more time.
			}
		}
	}
	while(didAny);
}

void AuScFaceList::TextureUnwrapHorizontal(const AuScSelectInfo& selInfo, quint8 texIndex, double texSize, float angleTol,
		bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	TextureUnwrapByFacing(selInfo, texIndex, texSize, angleTol, gridAlign, alignAxis, gridPlane);

	// Adjust textures horizontally so they continue from one face to the next.
	QSet<int> facesDone;
	iterator fi = end();
	bool didAny;
	do
	{
		didAny = false;

		// fi points to our reference face for wrapping.
		// Each loop ends with fi pointing to a face we just did, so it is our reference for the next pass.
		int findex=0;
		if(fi == end())
		{
			// First time through the loop.
			// Find the next selected face that has not been done.
			fi = begin();
			while(fi != end())
			{
				if(selInfo.m_Faces.contains(findex) && !facesDone.contains(findex))
					break;	// Found selected face that has not been done.
				fi++;
				findex++;
			}

			if(fi != end())
			{
				// Keep texture applied in TextureUnwrapByFacing.
				facesDone.insert(findex);
				didAny = true;
			}
		}

		// We're done if we have no reference face.
		if(fi != end())
		{
			// Look for an unfinished face that is adjacent to the reference face.
			findex = 0;
			for(iterator sfi = begin(); sfi != end(); sfi++, findex++)
			{
				// Skip unselected faces and faces that have already been done.
				if(!selInfo.m_Faces.contains(findex) || facesDone.contains(findex))
					continue;

				// Check for a common edge and get the vertices.
				AuScPoint myv1, myv2;
				if(!(*fi)->CommonEdgeVerticesHorizontal(**sfi, myv1, myv2))
					continue;

				// We share a common edge. This will be our basis for adjusting the texture on the adjacent face.
				int ov1i = (*sfi)->VertexAt(myv1);
				int ov2i = (*sfi)->VertexAt(myv2);
				if(ov1i < 0 || ov2i < 0)
					continue;
				AuScPoint ov1 = (*sfi)->Vertex(ov1i);
				AuScPoint ov2 = (*sfi)->Vertex(ov2i);

				// This will wrap nicely if the texture offset is the same at each vertex.
				AuPoint tvec1(ov1.TexCoord() - myv1.TexCoord());
				AuPoint tvec2(ov2.TexCoord() - myv2.TexCoord());
				if(!tvec1.FuzzyEquals(tvec2))
					continue;

				(*sfi)->TextureMove(QPointF((float)tvec1.x, (float)tvec1.y));
				facesDone.insert(findex);
				didAny = true;

				fi = sfi;
				break;
			}

			if(!didAny)
			{
				// We get here if the loop started with a valid fi and we did not find any adjacent faces.
				// Now we want to do the loop again with a fresh fi, if any can be found.
				fi = end();
				didAny = true;	// Keep the loop going for another round, to look for an unfinished fi one more time.
			}
		}
	}
	while(didAny);
}

void AuScFaceList::TextureUnwrapVertical(const AuScSelectInfo& selInfo, quint8 texIndex, double texSize, float angleTol,
		bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	TextureUnwrapByFacing(selInfo, texIndex, texSize, angleTol, gridAlign, alignAxis, gridPlane);

	// Adjust textures horizontally so they continue from one face to the next.
	QSet<int> facesDone;
	iterator fi = end();
	bool didAny;
	do
	{
		didAny = false;

		// fi points to our reference face for wrapping.
		// Each loop ends with fi pointing to a face we just did, so it is our reference for the next pass.
		int findex=0;
		if(fi == end())
		{
			// First time through the loop.
			// Find the next selected face that has not been done.
			fi = begin();
			while(fi != end())
			{
				if(selInfo.m_Faces.contains(findex) && !facesDone.contains(findex))
					break;	// Found selected face that has not been done.
				fi++;
				findex++;
			}

			if(fi != end())
			{
				// Keep texture applied in TextureUnwrapByFacing.
				facesDone.insert(findex);
				didAny = true;
			}
		}

		// We're done if we have no reference face.
		if(fi != end())
		{
			// Look for an unfinished face that is adjacent to the reference face.
			findex = 0;
			for(iterator sfi = begin(); sfi != end(); sfi++, findex++)
			{
				// Skip unselected faces and faces that have already been done.
				if(!selInfo.m_Faces.contains(findex) || facesDone.contains(findex))
					continue;

				// Check for a common edge and get the vertices.
				AuScPoint myv1, myv2;
				if(!(*fi)->CommonEdgeVerticesVertical(**sfi, myv1, myv2))
					continue;

				// We share a common edge. This will be our basis for adjusting the texture on the adjacent face.
				int ov1i = (*sfi)->VertexAt(myv1);
				int ov2i = (*sfi)->VertexAt(myv2);
				if(ov1i < 0 || ov2i < 0)
					continue;
				AuScPoint ov1 = (*sfi)->Vertex(ov1i);
				AuScPoint ov2 = (*sfi)->Vertex(ov2i);

				// This will wrap nicely if the texture offset is the same at each vertex.
				AuPoint tvec1(ov1.TexCoord() - myv1.TexCoord());
				AuPoint tvec2(ov2.TexCoord() - myv2.TexCoord());
				if(!tvec1.FuzzyEquals(tvec2))
					continue;

				(*sfi)->TextureMove(QPointF((float)tvec1.x, (float)tvec1.y));
				facesDone.insert(findex);
				didAny = true;

				fi = sfi;
				break;
			}

			if(!didAny)
			{
				// We get here if the loop started with a valid fi and we did not find any adjacent faces.
				// Now we want to do the loop again with a fresh fi, if any can be found.
				fi = end();
				didAny = true;	// Keep the loop going for another round, to look for an unfinished fi one more time.
			}
		}
	}
	while(didAny);
}

void AuScFaceList::EffectUnwrap(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize, UNWRAP_METHOD unwrap,
		float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	switch(unwrap)
	{
	case UNWRAP_ByFace:
		EffectUnwrapByFace(selInfo, effect, texSize, gridAlign, alignAxis, gridPlane);
		break;

	case UNWRAP_ByFacing:
		EffectUnwrapByFacing(selInfo, effect, texSize, angleTol, gridAlign, alignAxis, gridPlane);
		break;

	case UNWRAP_Edges:
		EffectUnwrapEdges(selInfo, effect, texSize, angleTol, gridAlign, alignAxis, gridPlane);
		break;

	case UNWRAP_Horizontal:
		EffectUnwrapHorizontal(selInfo, effect, texSize, angleTol, gridAlign, alignAxis, gridPlane);
		break;

	case UNWRAP_Vertical:
		EffectUnwrapVertical(selInfo, effect, texSize, angleTol, gridAlign, alignAxis, gridPlane);
		break;
	}
}

void AuScFaceList::EffectUnwrapByFace(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize, bool gridAlign,
		int alignAxis, const AuMatrix4x4& gridPlane)
{
	int findex=0;
	for(iterator fi = begin(); fi != end(); fi++, findex++)
		if(selInfo.m_Faces.contains(findex))
			(*fi)->EffectUnwrap(effect, texSize, gridAlign, alignAxis, gridPlane);
}

void AuScFaceList::EffectUnwrapByFacing(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize, float angleTol,
		bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	Transform(gridPlane.Inverse());

	unsigned int sanityCount = size() * size();	// So we don't get stuck doing this forever.
	QSet<int> facesDone;
	iterator fi;
	do
	{
		// Find the next selected face that has not been done.
		int findex=0;
		fi = begin();
		while(fi != end())
		{
			if(selInfo.m_Faces.contains(findex) && !facesDone.contains(findex))
				break;	// Found selected face that has not been done.
			fi++;
			findex++;
		}

		// We're done if we found no unfinished selected faces.
		if(fi != end())
		{
			// Found an unfinished selected face. This is our reference for all faces facing the same direction.
			// Get the plane matrix for the face.
			AuMatrix4x4 fplane;
			fplane.Rotate((*fi)->FacePlane().GetRotation());
			fplane.RotateX(PI);									// Flip vertically.

			if(gridAlign)
			{
				// Align the plane to a grid axis.
				double zrot;
				switch(alignAxis)
				{
				case 0:	// x
					// Grid X vector in the plane of the face.
					zrot = fplane.UnProjectVector(AuPoint(1,0,0)).ZRotation() - PI/2;
					break;
				case 1:	// y
					// Grid Y vector in the plane of the face.
					zrot = fplane.UnProjectVector(AuPoint(0,1,0)).ZRotation() - PI/2;
					break;
				case 2:	// z
					// Grid Z vector in the plane of the face.
					zrot = fplane.UnProjectVector(AuPoint(0,0,1)).ZRotation() - PI/2;
					break;
				default:
					zrot = 0;
					break;
				}
				if(zrot != 0)
					fplane.RotateZ(zrot);
			}

			AuPoint fn((*fi)->FaceNormal());

			// All unfinished faces with the same facing will be textured using the same plane as the reference face.
			findex = 0;
			for(iterator sfi = begin(); sfi != end(); sfi++, findex++)
				if(selInfo.m_Faces.contains(findex) && !facesDone.contains(findex) && (*sfi)->FaceNormal().AngleTo(fn) <= angleTol)
				{
					// Found selected face that has not been done that is close enough to parallel with the reference face.
					(*sfi)->EffectProject(effect, texSize, fplane);
					facesDone.insert(findex);
				}
		}
	}
	while(fi != end() && sanityCount--);

	Transform(gridPlane);
}

void AuScFaceList::EffectUnwrapEdges(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize, float angleTol,
		bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	EffectUnwrapByFacing(selInfo, effect, texSize, angleTol, gridAlign, alignAxis, gridPlane);

	// Adjust textures so they continue from one face to the next.
	QSet<int> facesDone;
	iterator fi = end();
	bool didAny;
	do
	{
		didAny = false;

		// fi points to our reference face for wrapping.
		// Each loop ends with fi pointing to a face we just did, so it is our reference for the next pass.
		int findex=0;
		if(fi == end())
		{
			// First time through the loop.
			// Find the next selected face that has not been done.
			fi = begin();
			while(fi != end())
			{
				if(selInfo.m_Faces.contains(findex) && !facesDone.contains(findex))
					break;	// Found selected face that has not been done.
				fi++;
				findex++;
			}

			if(fi != end())
			{
				// Keep texture applied in EffectUnwrapByFacing.
				facesDone.insert(findex);
				didAny = true;
			}
		}

		// We're done if we have no reference face.
		if(fi != end())
		{
			// Look for an unfinished face that is adjacent to the reference face.
			findex = 0;
			for(iterator sfi = begin(); sfi != end(); sfi++, findex++)
			{
				// Skip unselected faces and faces that have already been done.
				if(!selInfo.m_Faces.contains(findex) || facesDone.contains(findex))
					continue;

				// Check for a common edge and get the vertices.
				AuScPoint myv1, myv2;
				if(!(*fi)->CommonEdgeVertices(**sfi, myv1, myv2))
					continue;

				// We share a common edge. This will be our basis for adjusting the texture on the adjacent face.
				int ov1i = (*sfi)->VertexAt(myv1);
				int ov2i = (*sfi)->VertexAt(myv2);
				if(ov1i < 0 || ov2i < 0)
					continue;
				AuScPoint ov1 = (*sfi)->Vertex(ov1i);
				AuScPoint ov2 = (*sfi)->Vertex(ov2i);

				// This will wrap nicely if the texture offset is the same at each vertex.
				AuPoint tvec1(ov1.EffectCoord() - myv1.EffectCoord());
				AuPoint tvec2(ov2.EffectCoord() - myv2.EffectCoord());
				if(!tvec1.FuzzyEquals(tvec2))
					continue;

				(*sfi)->EffectMove(QPointF((float)tvec1.x, (float)tvec1.y));
				facesDone.insert(findex);
				didAny = true;

				fi = sfi;
				break;
			}

			if(!didAny)
			{
				// We get here if the loop started with a valid fi and we did not find any adjacent faces.
				// Now we want to do the loop again with a fresh fi, if any can be found.
				fi = end();
				didAny = true;	// Keep the loop going for another round, to look for an unfinished fi one more time.
			}
		}
	}
	while(didAny);
}

void AuScFaceList::EffectUnwrapHorizontal(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize, float angleTol,
		bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	EffectUnwrapByFacing(selInfo, effect, texSize, angleTol, gridAlign, alignAxis, gridPlane);

	// Adjust textures horizontally so they continue from one face to the next.
	QSet<int> facesDone;
	iterator fi = end();
	bool didAny;
	do
	{
		didAny = false;

		// fi points to our reference face for wrapping.
		// Each loop ends with fi pointing to a face we just did, so it is our reference for the next pass.
		int findex=0;
		if(fi == end())
		{
			// First time through the loop.
			// Find the next selected face that has not been done.
			fi = begin();
			while(fi != end())
			{
				if(selInfo.m_Faces.contains(findex) && !facesDone.contains(findex))
					break;	// Found selected face that has not been done.
				fi++;
				findex++;
			}

			if(fi != end())
			{
				// Keep texture applied in EffectUnwrapByFacing.
				facesDone.insert(findex);
				didAny = true;
			}
		}

		// We're done if we have no reference face.
		if(fi != end())
		{
			// Look for an unfinished face that is adjacent to the reference face.
			findex = 0;
			for(iterator sfi = begin(); sfi != end(); sfi++, findex++)
			{
				// Skip unselected faces and faces that have already been done.
				if(!selInfo.m_Faces.contains(findex) || facesDone.contains(findex))
					continue;

				// Check for a common edge and get the vertices.
				AuScPoint myv1, myv2;
				if(!(*fi)->CommonEdgeVerticesHorizontal(**sfi, myv1, myv2))
					continue;

				// We share a common edge. This will be our basis for adjusting the texture on the adjacent face.
				int ov1i = (*sfi)->VertexAt(myv1);
				int ov2i = (*sfi)->VertexAt(myv2);
				if(ov1i < 0 || ov2i < 0)
					continue;
				AuScPoint ov1 = (*sfi)->Vertex(ov1i);
				AuScPoint ov2 = (*sfi)->Vertex(ov2i);

				// This will wrap nicely if the texture offset is the same at each vertex.
				AuPoint tvec1(ov1.EffectCoord() - myv1.EffectCoord());
				AuPoint tvec2(ov2.EffectCoord() - myv2.EffectCoord());
				if(!tvec1.FuzzyEquals(tvec2))
					continue;

				(*sfi)->EffectMove(QPointF((float)tvec1.x, (float)tvec1.y));
				facesDone.insert(findex);
				didAny = true;

				fi = sfi;
				break;
			}

			if(!didAny)
			{
				// We get here if the loop started with a valid fi and we did not find any adjacent faces.
				// Now we want to do the loop again with a fresh fi, if any can be found.
				fi = end();
				didAny = true;	// Keep the loop going for another round, to look for an unfinished fi one more time.
			}
		}
	}
	while(didAny);
}

void AuScFaceList::EffectUnwrapVertical(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize, float angleTol,
		bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	EffectUnwrapByFacing(selInfo, effect, texSize, angleTol, gridAlign, alignAxis, gridPlane);

	// Adjust textures horizontally so they continue from one face to the next.
	QSet<int> facesDone;
	iterator fi = end();
	bool didAny;
	do
	{
		didAny = false;

		// fi points to our reference face for wrapping.
		// Each loop ends with fi pointing to a face we just did, so it is our reference for the next pass.
		int findex=0;
		if(fi == end())
		{
			// First time through the loop.
			// Find the next selected face that has not been done.
			fi = begin();
			while(fi != end())
			{
				if(selInfo.m_Faces.contains(findex) && !facesDone.contains(findex))
					break;	// Found selected face that has not been done.
				fi++;
				findex++;
			}

			if(fi != end())
			{
				// Keep texture applied in EffectUnwrapByFacing.
				facesDone.insert(findex);
				didAny = true;
			}
		}

		// We're done if we have no reference face.
		if(fi != end())
		{
			// Look for an unfinished face that is adjacent to the reference face.
			findex = 0;
			for(iterator sfi = begin(); sfi != end(); sfi++, findex++)
			{
				// Skip unselected faces and faces that have already been done.
				if(!selInfo.m_Faces.contains(findex) || facesDone.contains(findex))
					continue;

				// Check for a common edge and get the vertices.
				AuScPoint myv1, myv2;
				if(!(*fi)->CommonEdgeVerticesVertical(**sfi, myv1, myv2))
					continue;

				// We share a common edge. This will be our basis for adjusting the texture on the adjacent face.
				int ov1i = (*sfi)->VertexAt(myv1);
				int ov2i = (*sfi)->VertexAt(myv2);
				if(ov1i < 0 || ov2i < 0)
					continue;
				AuScPoint ov1 = (*sfi)->Vertex(ov1i);
				AuScPoint ov2 = (*sfi)->Vertex(ov2i);

				// This will wrap nicely if the texture offset is the same at each vertex.
				AuPoint tvec1(ov1.EffectCoord() - myv1.EffectCoord());
				AuPoint tvec2(ov2.EffectCoord() - myv2.EffectCoord());
				if(!tvec1.FuzzyEquals(tvec2))
					continue;

				(*sfi)->EffectMove(QPointF((float)tvec1.x, (float)tvec1.y));
				facesDone.insert(findex);
				didAny = true;

				fi = sfi;
				break;
			}

			if(!didAny)
			{
				// We get here if the loop started with a valid fi and we did not find any adjacent faces.
				// Now we want to do the loop again with a fresh fi, if any can be found.
				fi = end();
				didAny = true;	// Keep the loop going for another round, to look for an unfinished fi one more time.
			}
		}
	}
	while(didAny);
}

double AuScFaceList::Area() const
{
	double a = 0;
	for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
		a += (*ti)->Area();
	return a;
}

bool AuScFaceList::Sight(const AuPoint& eyeLoc, AuPoint& sightLoc) const
{
	// At some size it becomes worthwhile to test my box extents first.
	if(size() > 10)
	{
		AuBox box(m_Min, m_Max);
		AuPoint int0, int1;
		if(!box.SegmentIntersection(eyeLoc, sightLoc, int0, int1) && !box.IsPointInside(eyeLoc))
			return false;				// No intersection with my extents box and the eye location is not inside the box extents.
	}

	// Look for closest intersection with any face.
	bool hitMe = false;
	for(const_iterator fi = constBegin(); fi != constEnd(); fi++)
		if((*fi)->Sight(eyeLoc, sightLoc))
			hitMe = true;
	return hitMe;
}

bool AuScFaceList::Sight(const AuPoint& eyeLoc, AuPoint& sightLoc, AuPoint& sightedFaceNormal) const
{
	// At some size it becomes worthwhile to test my box extents first.
	if(size() > 10)
	{
		AuBox box(m_Min, m_Max);
		AuPoint int0, int1;
		if(!box.SegmentIntersection(eyeLoc, sightLoc, int0, int1) && !box.IsPointInside(eyeLoc))
			return false;				// No intersection with my extents box and the eye location is not inside the box extents.
	}

	// Look for closest intersection with any face.
	bool hitMe = false;
	for(const_iterator fi = constBegin(); fi != constEnd(); fi++)
		if((*fi)->Sight(eyeLoc, sightLoc))
		{
			sightedFaceNormal = (*fi)->FaceNormal();
			hitMe = true;
		}
	return hitMe;
}

float AuScFaceList::SiteInfluence(const AuPoint& loc, AuPoint& siteLoc) const
{
	for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
	{
		float siteI = (*ti)->SiteInfluence(loc, siteLoc);
		if(siteI > 0)
			return siteI;
	}
	return 0;
}

bool AuScFaceList::InSite(const AuPoint& loc) const
{
	for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
		if((*ti)->InSite(loc))
			return true;
	return false;
}

bool AuScFaceList::IntersectRay(const AuPoint& loc, const AuPoint& vec, AuPoint& intersect) const
{
	bool insect = false;
	double insectDist = 0;
	AuPoint insectLoc;
	for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
		if((*ti)->IntersectRay(loc, vec, insectLoc))
		{
			double dist = insectLoc.Distance(loc);
			if(!insect)
			{
				// Found first intersection.
				insect = true;
				insectDist = dist;
				intersect = insectLoc;
			}
			else if(insectDist > dist)
			{
				// This one is closer than the closest we found so far.
				insectDist = dist;
				intersect = insectLoc;
			}
		}
	return insect;
}

bool AuScFaceList::IntersectZ(const AuPoint& loc, AuPoint& intersect) const
{
	bool insect = false;
	double insectDist = 0;
	AuPoint insectLoc;
	for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
		if((*ti)->IntersectZ(loc, insectLoc))
		{
			double dist = fabs(insectLoc.z - loc.z);
			if(!insect)
			{
				// Found first intersection.
				insect = true;
				insectDist = dist;
				intersect = loc;
				intersect.z = insectLoc.z;
			}
			else if(insectDist > dist)
			{
				// This one is closer than the closest we found so far.
				insectDist = dist;
				intersect.z = insectLoc.z;
			}
		}
	return insect;
}

void AuScFaceList::MakeExtrusion(const AuPointArray& bnd, const AuPoint& extVec, const AuMatrix4x4& gridPlane, unsigned int steps,
		bool closeEnds)
{
	// Transform the boundary to the real XY plane so the math is easier.
	AuPointArray cbnd(bnd);
	cbnd.Transform(gridPlane.Inverse());

	// Prepare boundary that will be used for the top and bottom.
	if(!cbnd.IsClockwise())
		cbnd.ReverseOrder();

	// Triangulate the boundary with edge lines at the boundary.
	AuPointArray tris;
	QVector<bool> edges;
	cbnd.TriangulateBoundary(tris, edges);

	// Tessellate the top and bottom.
	if(closeEnds)
	{
		AddFace(AuScFacePolygon(tris, edges, extVec));

		AuScFacePolygon bottom(tris, edges);
		bottom.ReverseTriangleFacing();			// Bottom triangles must be reversed to face downward.
		bottom.ReverseVertexNormals();			// Point their normals downward too.
		AddFace(bottom);
	}

	// Tessellate the sides.
	if(steps <= 1)
	{
		// Walk around the clockwise bottom boundary to make the sides.
		AuPoint vbl(cbnd.last());
		AuPoint vtl = vbl + extVec;
		for(AuPointArray::iterator bi = cbnd.begin(); bi != cbnd.end(); bi++)
		{
			AuPoint vb(*bi);
			AuPoint vt = vb + extVec;

			AuPoint vx(vbl - vb);
			vx.Unit();
			AuPoint vn = vx.Cross(AuPoint(0,0,1));
			vn.Unit();

			AuScPoint vll(vb, 0,0, vn);
			AuScPoint vul(vt, 0,0, vn);
			AuScPoint vur(vtl, 0,0, vn);
			AuScPoint vlr(vbl, 0,0, vn);

			AddFace(AuScFaceQuad(vll, vul, vur, vlr));

			vbl = vb;
			vtl = vt;
		}
	}
	else
	{
		// Walk around the clockwise bottom boundary to make the sides.
		AuPoint stepVec = extVec/steps;			// Extrusion vector at each step.
		for(unsigned int step=0; step<steps; step++)
		{
			AuPoint stepBase(stepVec * step);	// Vector to base of current step.
			AuPoint vbl(cbnd.last() + stepBase);
			AuPoint vtl = vbl + stepVec;
			for(AuPointArray::iterator bi = cbnd.begin(); bi != cbnd.end(); bi++)
			{
				AuPoint vb(*bi + stepBase);
				AuPoint vt = vb + stepVec;

				AuPoint vx(vbl - vb);
				vx.Unit();
				AuPoint vn = vx.Cross(AuPoint(0,0,1));
				vn.Unit();

				AuScPoint vll(vb, 0,0, vn);
				AuScPoint vul(vt, 0,0, vn);
				AuScPoint vur(vtl, 0,0, vn);
				AuScPoint vlr(vbl, 0,0, vn);

				AddFace(AuScFaceQuad(vll, vul, vur, vlr));

				vbl = vb;
				vtl = vt;
			}
		}
	}

	// Transform the result to the design plane.
	Transform(gridPlane);
}

void AuScFaceList::MakeSpindle(const AuPointArray& profile, quint16 steps, const AuMatrix4x4& gridPlane)
{
	if(steps < 3)
		return;

	// Transform the profile to the real XY plane so the math is easier.
	AuPointArray cprofile(profile);
	cprofile.Transform(gridPlane.Inverse());

	double prevang = 0;
	for(int step=1; step<=steps; step++)
	{
		double stepang = PI*2 * step / steps;

		// Build triangles along the spinning profile.
		AuPoint prev(cprofile.first());
		for(AuPointArray::const_iterator pi = cprofile.cbegin() + 1; pi != cprofile.cend(); pi++)
		{
			// Normals.
			AuPoint svec = (*pi) - prev;
			AuPoint n = svec.RotateZ(PI/2);
			n.Unit();

			AuPoint vll = prev.RotateX(prevang);
			AuPoint vul = (*pi).RotateX(prevang);
			AuPoint vur = (*pi).RotateX(stepang);
			AuPoint vlr = prev.RotateX(stepang);

			AuScPoint ll = AuScPoint(vll, 0,0, n);
			AuScPoint ul = AuScPoint(vul, 0,0, n);
			AuScPoint ur = AuScPoint(vur, 0,0, n);
			AuScPoint lr = AuScPoint(vlr, 0,0, n);

			if(ll.FuzzyEquals(lr))
				AddFace(AuScFaceTri(ll, ul, ur));
			else if(ul.FuzzyEquals(ur))
				AddFace(AuScFaceTri(ll, ul, lr));
			else
				AddFace(AuScFaceQuad(ll, ul, ur, lr));

			// Prepare for next profile segment.
			prev = *pi;
		}

		// Prepare for next step.
		prevang = stepang;
	}

	// Transform the result to the design plane.
	Transform(gridPlane);

	// Something isn't right about the smoothed normals. I always have to resmooth them after making a spindle. Just do it here.
	SmoothVertices();
}

void AuScFaceList::MakeSpindle(const AuPointArray& profile, quint16 steps, const AuMatrix4x4& gridPlane, double sweepAngle)
{
	if(steps < 3)
		return;

	// Transform the profile to the real XY plane so the math is easier.
	AuPointArray cprofile(profile);
	cprofile.Transform(gridPlane.Inverse());

	double prevang = 0;
	for(int step=1; step<=steps; step++)
	{
		double stepang = sweepAngle * step / steps;

		// Build triangles along the spinning profile.
		AuPoint prev(cprofile.first());
		for(AuPointArray::const_iterator pi = cprofile.cbegin() + 1; pi != cprofile.cend(); pi++)
		{
			// Normals.
			AuPoint svec = (*pi) - prev;
			AuPoint n = svec.RotateZ(PI/2);
			n.Unit();

			AuPoint vll = prev.RotateX(prevang);
			AuPoint vul = (*pi).RotateX(prevang);
			AuPoint vur = (*pi).RotateX(stepang);
			AuPoint vlr = prev.RotateX(stepang);

			AuScPoint ll = AuScPoint(vll, 0,0, n);
			AuScPoint ul = AuScPoint(vul, 0,0, n);
			AuScPoint ur = AuScPoint(vur, 0,0, n);
			AuScPoint lr = AuScPoint(vlr, 0,0, n);

			if(ll.FuzzyEquals(lr))
				AddFace(AuScFaceTri(ll, ul, ur));
			else if(ul.FuzzyEquals(ur))
				AddFace(AuScFaceTri(ll, ul, lr));
			else
				AddFace(AuScFaceQuad(ll, ul, ur, lr));

			// Prepare for next profile segment.
			prev = *pi;
		}

		// Prepare for next step.
		prevang = stepang;
	}

	// Transform the result to the design plane.
	Transform(gridPlane);

	// Something isn't right about the smoothed normals. I always have to resmooth them after making a spindle. Just do it here.
	SmoothVertices();
}

void AuScFaceList::MakeSphereLongLat(const AuPoint& center, double radius, quint16 lonSteps, quint16 latSteps,
		const AuMatrix4x4& gridPlane)
{
	// This kind of sphere is just a spindle.
	AuPointArray profile;
	for(int step=0; step<=latSteps; step++)
	{
		double latang = PI * step / latSteps - PI/2;
		AuPoint v = AuPoint(radius,0,0).RotateZ(latang);
		profile.append(v);
	}

	AuMatrix4x4 t;
	t.RotateZ(PI/2);
	MakeSpindle(profile, lonSteps, t);

	// Transform the result to the design plane.
	AuMatrix4x4 cT;
	cT.Translate(center);
	cT.RotateX(PI/2);
	Transform(gridPlane * cT);
}

void AuScFaceList::MakeSphereUniform(const AuPoint& center, double radius, quint16 equatorSteps, quint16 textureRepetitions,
		const AuMatrix4x4& gridPlane)
{
	if(equatorSteps < 3)
		return;

	AuGlobeLLTriList tris;
	tris.CreateGlobeTriangles(equatorSteps);

	// Turn the globe tessellation into globe surface triangles with 3D coordinates and texture coordinates.
	for(QList<AuGlobeLLTri>::const_iterator ti = tris.cbegin(); ti != tris.cend(); ti++)
	{
		// Calculate triangle vertex coordinates.
		AuPoint terCoord0 = (*ti).m_LL0.GlobeLocation(radius);
		AuPoint terCoord1 = (*ti).m_LL1.GlobeLocation(radius);
		AuPoint terCoord2 = (*ti).m_LL2.GlobeLocation(radius);

		AuPoint n0 = (*ti).m_LL0.GlobeLocation(1);
		AuPoint n1 = (*ti).m_LL1.GlobeLocation(1);
		AuPoint n2 = (*ti).m_LL2.GlobeLocation(1);

		// Wrap texture properly at the seam.
		double ll0long = (*ti).m_LL0.Longitude();
		double ll1long = (*ti).m_LL1.Longitude();
		double ll2long = (*ti).m_LL2.Longitude();
		if((*ti).m_LL0.Longitude() > PI/2 && ((*ti).m_LL1.Longitude() < -PI/2 || (*ti).m_LL2.Longitude() < -PI/2))
			ll0long -= PI*2;
		if((*ti).m_LL1.Longitude() > PI/2 && ((*ti).m_LL0.Longitude() < -PI/2 || (*ti).m_LL2.Longitude() < -PI/2))
			ll1long -= PI*2;
		if((*ti).m_LL2.Longitude() > PI/2 && ((*ti).m_LL0.Longitude() < -PI/2 || (*ti).m_LL1.Longitude() < -PI/2))
			ll2long -= PI*2;

		// Calculate texture coordinates.
		QPointF terTex0;
		terTex0.setX((ll0long)/(PIf*2) * textureRepetitions * cosf((*ti).m_LL0.Latitude()));
		terTex0.setY(((*ti).m_LL0.Latitude() + PIf/2)/PIf * textureRepetitions);
		QPointF terTex1;
		terTex1.setX((ll1long)/(PIf*2) * textureRepetitions * cosf((*ti).m_LL1.Latitude()));
		terTex1.setY(((*ti).m_LL1.Latitude() + PIf/2)/PIf * textureRepetitions);
		QPointF terTex2;
		terTex2.setX((ll2long)/(PIf*2) * textureRepetitions * cosf((*ti).m_LL2.Latitude()));
		terTex2.setY(((*ti).m_LL2.Latitude() + PIf/2)/PIf * textureRepetitions);

		// Construct a globe triangle.
		AuScPoint v0 = AuScPoint(terCoord0, terTex0, n0);
		AuScPoint v1 = AuScPoint(terCoord1, terTex1, n1);
		AuScPoint v2 = AuScPoint(terCoord2, terTex2, n2);

		// Append the globe triangle to the list.
		AddFace(AuScFaceTri(v0, v1, v2));
	}

	// Transform the result to the design plane.
	AuMatrix4x4 cT;
	cT.Translate(center);
	Transform(gridPlane * cT);
}

void AuScFaceList::MakeTube(const AuPointArray& path, double diameter, int sides, bool smoothSides)
{
	// Create a circle shape in a point array.
	double radius = diameter/2;
	AuPointArray shape;
	shape.CreateEllipse(AuPoint(0,0), AuPoint(radius,0), AuPoint(0,radius), sides);
	MakeTube(path, shape, smoothSides);
}

void AuScFaceList::MakeTube(const AuPointArray& path, const AuPointArray& shape, bool smoothSides)
{
	if(path.size() < 2)
		return;
	if(shape.size() < 3)
		return;

	AuPointArray cbnd(shape);
	if(!cbnd.IsClockwise())
		cbnd.ReverseOrder();

	// Walk the path, creating the sides of the tube at each segment.
	AuMatrix4x4 st,et;					// Transforms of start and end caps.
	AuPoint prevec;						// Unit vector from previous point to current point in loop.
	AuPlane preplane;					// Tubing profile plane at previous intersection.
	AuPointArray preshape;				// Shape in the plane of the previous intersection.
	for(AuPointArray::const_iterator pi = path.cbegin(); pi != path.cend(); pi++)
	{
		if(pi == path.cbegin())
		{
			// First point. Don't make anything; just prepare to make the next segment.
			// Align the shape to the start end of the tube.
			prevec = *(pi+1) - *pi;								// Direction vector of the first tube segment.
			prevec.Unit();

			AuPoint px;
			prevec.ArbitraryAxis(px);
			AuPoint py = prevec.Cross(px);
			py.Unit();
			preplane = AuPlane(*pi, px, py);					// Plane at start of first tube segment.

			// Create the shape at the start location and rotation. This is the "previous shape" at the end of the first segment.
			st.Translate(*pi);
			st.Rotate(AuPoint::CalculateRotation(prevec));
			preshape = cbnd;
			preshape.Transform(st);
			if(preshape.last() != preshape.first())
			{
				// Close the boundary.
				if(preshape.last().FuzzyEquals(preshape.first()))
					*(preshape.end() - 1) = preshape.first();	// Close enough to just move the last point.
				else
					preshape.append(preshape.first());			// Must append a point for closure.
			}
		}
		else
		{
			// Not the first point.
			// Construct tubing walls from previous tubing point to current tubing point.
			AuPoint vec;										// Unit vector from current point to next point.
			if(pi == path.cend()-1)
			{
				// This is the final segment. It ends square.
				vec = prevec;									// Continue in same direction for square end.
				et.Translate(*pi);
				et.Rotate(AuPoint::CalculateRotation(prevec));
			}
			else
			{
				// This is an intermediate segment point.
				vec = AuPoint(*(pi+1) - *pi);					// Direction vector of the next tube segment.
				vec.Unit();
			}

			// Calculate plane of intersection between previous seg and current seg.
			AuPlane plane(*pi, (prevec + vec)/2);

			// Create the shape at the intersection.
			AuPointArray intshape;
			for(AuPointArray::const_iterator psi = preshape.cbegin(); psi != preshape.cend(); psi++)
			{
				// Find the intersection between a line through the shape point and the plane at the intersection.
				AuLine l((*psi).x, (*psi).y, (*psi).z, prevec.x, prevec.y, prevec.z);
				AuPoint i;
				if(plane.LineIntersection(l, i))
					intshape.append(i);
				else
					intshape.append(*psi);
			}

			// Create faces at the sides of the tube segment.
			AuPoint pprept, pcurpt;
			for(AuPointArray::const_iterator psi = preshape.cbegin(), si = intshape.cbegin();
					psi != preshape.cend() && si != intshape.cend(); psi++, si++)
			{
				AuPoint prept(*psi);
				AuPoint curpt(*si);
				if(psi != preshape.cbegin())
				{
					// Create a quad face.
					AuScPoint v0(pcurpt, 0,0, AuPoint(0,0,1));
					AuScPoint v1(pprept, 0,0, AuPoint(0,0,1));
					AuScPoint v2(prept, 0,0, AuPoint(0,0,1));
					AuScPoint v3(curpt, 0,0, AuPoint(0,0,1));
					AuScFaceQuad fq(v0, v1, v2, v3);

					// Transform the result to the grid plane.
					AddFace(fq);
				}
				pprept = prept;
				pcurpt = curpt;
			}

			// Prepare for the next tube segment.
			prevec = vec;
			preplane = plane;
			preshape = intshape;
		}
	}

	FlattenVertexNormals();

	if(smoothSides)
		SmoothVertices();

	// Cap starting end.
	{
		// Triangulate the boundary with edge lines at the boundary.
		AuPointArray tris;
		QVector<bool> edges;
		cbnd.TriangulateBoundary(tris, edges);

		AuScFaceList capfaces;
		if(tris.size() == 3)
		{
			// Create a triangle face.
			AuScPoint v0(tris[0], 0,0, AuPoint(0,0,1));
			AuScPoint v1(tris[1], 0,0, AuPoint(0,0,1));
			AuScPoint v2(tris[2], 0,0, AuPoint(0,0,1));
			AuScFaceTri ft(v0, v1, v2);

			// Transform the result to the grid plane.
			capfaces.AddFace(ft);
		}
		else if(tris.size() == 6)
		{
			AuScPoint v0(tris[0], 0,0, AuPoint(0,0,1));
			AuScPoint v1(tris[1], 0,0, AuPoint(0,0,1));
			AuScPoint v2(tris[2], 0,0, AuPoint(0,0,1));
			AuScFaceTri ft1(v0, v1, v2);

			AuScPoint v3(tris[3], 0,0, AuPoint(0,0,1));
			AuScPoint v4(tris[4], 0,0, AuPoint(0,0,1));
			AuScPoint v5(tris[5], 0,0, AuPoint(0,0,1));
			AuScFaceTri ft2(v3, v4, v5);

			AuScFace* fq = ft1.Join(ft2);
			if(fq)
			{
				// The two triangles joined together into something, hopefully a quad.
				// Transform the result to the grid plane.
				capfaces.AddFace(*fq);
				delete fq;
			}
		}

		if(capfaces.empty())
		{
			// Create a polygon face.
			AuScFacePolygon fp(tris, edges);

			// Transform the result to the grid plane.
			capfaces.AddFace(fp);
		}

		if(!capfaces.empty())
		{
			capfaces.Transform(st);
			capfaces.ReverseTriangleFacing();
			capfaces.ReverseVertexNormals();
			AddFaces(capfaces);
		}
	}

	// Cap ending end.
	{
		// Ending face is re triangulated because of its new orientation.
		preshape.Transform(et.Inverse());

		// Triangulate the boundary with edge lines at the boundary.
		AuPointArray tris;
		QVector<bool> edges;
		preshape.TriangulateBoundary(tris, edges);

		AuScFaceList capfaces;
		if(tris.size() == 3)
		{
			// Create a triangle face.
			AuScPoint v0(tris[0], 0,0, AuPoint(0,0,1));
			AuScPoint v1(tris[1], 0,0, AuPoint(0,0,1));
			AuScPoint v2(tris[2], 0,0, AuPoint(0,0,1));
			AuScFaceTri ft(v0, v1, v2);

			// Transform the result to the grid plane.
			capfaces.AddFace(ft);
		}
		else if(tris.size() == 6)
		{
			AuScPoint v0(tris[0], 0,0, AuPoint(0,0,1));
			AuScPoint v1(tris[1], 0,0, AuPoint(0,0,1));
			AuScPoint v2(tris[2], 0,0, AuPoint(0,0,1));
			AuScFaceTri ft1(v0, v1, v2);

			AuScPoint v3(tris[3], 0,0, AuPoint(0,0,1));
			AuScPoint v4(tris[4], 0,0, AuPoint(0,0,1));
			AuScPoint v5(tris[5], 0,0, AuPoint(0,0,1));
			AuScFaceTri ft2(v3, v4, v5);

			AuScFace* fq = ft1.Join(ft2);
			if(fq)
			{
				// The two triangles joined together into something, hopefully a quad.
				// Transform the result to the grid plane.
				capfaces.AddFace(*fq);
				delete fq;
			}
		}

		if(capfaces.empty())
		{
			// Create a polygon face.
			AuScFacePolygon fp(tris, edges);

			// Transform the result to the grid plane.
			capfaces.AddFace(fp);
		}

		if(!capfaces.empty())
		{
			capfaces.Transform(et);
			AddFaces(capfaces);
		}
	}
}

void AuScFaceList::EdgeSegments(AuSegmentArray& edges) const
{
	// Create an array of unique edge segments.
	QMap<AuSegment, bool> alreadyChecked;
	for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
	{
		AuSegmentArray fsegs;
		(*ti)->EdgeSegments(fsegs);

		for(AuSegmentArray::const_iterator si = fsegs.cbegin(); si != fsegs.cend(); si++)
		{
			if(alreadyChecked.contains(*si))
				continue;

			AuSegment segrev((*si).m_fLoc2, (*si).m_fLoc1);	// Segment with points in reverse order, for alreadyChecked check.
			if(alreadyChecked.contains(segrev))
				continue;

			edges.append(*si);
		}
	}
}

quint64 AuScFaceList::MemoryUsage() const
{
	quint64 mem = sizeof(AuScFaceList);
	for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
		mem += (*ti)->MemoryUsage();
	return mem;
}

// returns 0 on success.
int AuScFaceList::STLReadASCII(const QString& fileName)
{
	Clear();
	QFile file(fileName);
	if(!file.open(QFile::ReadOnly))
		return 1;	// File open failed.
	QString line;
	AuPoint normal, vert1, vert2, vert3;
	bool hasn = false, hasv1 = false, hasv2 = false, hasv3 = false;
	QTextStream in(&file);
	while(!in.atEnd())
	{
		line = in.readLine().simplified();
		if(line.isEmpty())
			continue;
		QStringList fields = line.split(" ");
		if(fields[0].compare("solid", Qt::CaseInsensitive) == 0)
		{
#if !defined(QT_NO_DEBUG)
			if(fields.size() > 1)
				qDebug() << "AuScFaceList::STLReadASCII" << fileName << "solid" << fields[1];
			else
				qDebug() << "AuScFaceList::STLReadASCII" << fileName << "unnamed solid";
#endif
		}
		else if(fields[0].compare("facet", Qt::CaseInsensitive) == 0)
		{
			hasn = false;
			hasv1 = false;
			hasv2 = false;
			hasv3 = false;
			if(fields.size() == 5 && fields[1].compare("normal", Qt::CaseInsensitive) == 0)
			{
				normal.x = fields[2].toDouble();
				normal.y = fields[3].toDouble();
				normal.z = fields[4].toDouble();
				normal.Unit();
				hasn = true;
			}
			else
				qWarning() << "error: Bad STL facet normal" << line;
		}
		else if(fields[0].compare("outer", Qt::CaseInsensitive) == 0)
		{
			if(fields.size() == 2 && fields[1].compare("loop", Qt::CaseInsensitive) == 0)
			{
				// Started face outer loop.
			}
			else
				qWarning() << "error: Bad STL outer loop" << line;
		}
		else if(fields[0].compare("vertex", Qt::CaseInsensitive) == 0)
		{
			if(fields.size() == 4)
			{
				AuPoint v;
				v.x = fields[1].toDouble();
				v.y = fields[2].toDouble();
				v.z = fields[3].toDouble();
				if(!hasv1)
				{
					vert1 = v;
					hasv1 = true;
				}
				else if(!hasv2)
				{
					vert2 = v;
					hasv2 = true;
				}
				else if(!hasv3)
				{
					vert3 = v;
					hasv3 = true;
				}
				else
				{
					qWarning() << "error: Bad STL unexpected vertex" << line;
				}
			}
			else
				qWarning() << "error: Bad STL vertex" << line;
		}
		else if(fields[0].compare("endloop", Qt::CaseInsensitive) == 0)
		{
			if(fields.size() != 1)
				qWarning() << "error: Bad STL endloop" << line;
		}
		else if(fields[0].compare("endfacet", Qt::CaseInsensitive) == 0)
		{
			if(fields.size() != 1)
				qWarning() << "error: Bad STL endfacet" << line;
			else if(!hasn)
				qWarning() << "error: Bad STL endfacet no normal";
			else if(!hasv1)
				qWarning() << "error: Bad STL endfacet no vertex 1";
			else if(!hasv2)
				qWarning() << "error: Bad STL endfacet no vertex 2";
			else if(!hasv3)
				qWarning() << "error: Bad STL endfacet no vertex 3";
			else
			{
				// Add a face triangle.
				// Vertices are ordered by the right hand rule of the triangle, which means they are counterclockwise around their
				// normal.
				AuScPoint v1(vert1, 0,0, normal);
				AuScPoint v2(vert2, 0,0, normal);
				AuScPoint v3(vert3, 0,0, normal);
				AuScFaceTri tri(v1,v3,v2);
				if(normal.IsZero())
					tri.FlattenVertexNormals();
				AddFace(tri);
			}
		}
		else if(fields[0].compare("endsolid", Qt::CaseInsensitive) == 0)
		{
#if !defined(QT_NO_DEBUG)
			if(fields.size() > 1)
				qDebug() << "AuScFaceList::STLReadASCII" << fileName << "end solid" << fields[1] << size() << "faces";
			else
				qDebug() << "AuScFaceList::STLReadASCII" << fileName << "end unnamed solid" << size() << "faces";
#endif
		}
		else
		{
			//qWarning() << "error: Bad STL line" << line;
			return 2;
		}
	}
	if(empty())
		return 2;	// File read failed.
	return 0;
}

// returns 0 on success.
int AuScFaceList::STLReadBinary(const QString& fileName)
{
	Clear();
	QFile file(fileName);
	if(!file.open(QFile::ReadOnly))
		return 1;	// File open failed.

	QDataStream in(&file);
	in.setByteOrder(QDataStream::LittleEndian);
	in.setFloatingPointPrecision(QDataStream::SinglePrecision);
	in.skipRawData(80);	// Skip the 80 character header.
	quint32 facetCount;
	in >> facetCount;
	while(facetCount-- && !in.atEnd())
	{
		float nx, ny, nz;
		in >> nx;
		in >> ny;
		in >> nz;

		float v1x, v1y, v1z;
		in >> v1x;
		in >> v1y;
		in >> v1z;

		float v2x, v2y, v2z;
		in >> v2x;
		in >> v2y;
		in >> v2z;

		float v3x, v3y, v3z;
		in >> v3x;
		in >> v3y;
		in >> v3z;

		quint16 attrBytes;
		in >> attrBytes;
		in.skipRawData(attrBytes);	// Skip the attribute bytes

		AuPoint normal(nx, ny, nz);
		normal.Unit();

		// Add a face triangle.
		// Vertices are ordered by the right hand rule of the triangle, which means they are counterclockwise around their normal.
		AuScPoint v1(v1x,v1y,v1z, 0,0, normal);
		AuScPoint v2(v2x,v2y,v2z, 0,0, normal);
		AuScPoint v3(v3x,v3y,v3z, 0,0, normal);
		AuScFaceTri tri(v1,v3,v2);
		if(normal.IsZero())
			tri.FlattenVertexNormals();
		AddFace(tri);
	}
	if(empty())
		return 2;	// File read failed.
#if !defined(QT_NO_DEBUG)
	qDebug() << "AuScFaceList::STLReadBinary" << fileName << size() << "faces";
#endif
	return 0;
}

// returns 0 on success.
int AuScFaceList::STLWriteASCII(const QString& fileName) const
{
	if(empty())
		return 2;	// Nothing to save.

	// Get my triangles.
	AuScTriList tris;
	Triangles(tris);
	if(tris.empty())
		return 3;	// No triangles?

	QFile file(fileName);
	if(!file.open(QIODevice::WriteOnly))
		return 1;	// Failed to open/create file.
	QTextStream out(&file);
	out.setCodec("UTF-8");
	out << "solid "
#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
		<< Qt::endl;
#else
		<< endl;
#endif
	for(AuScTriList::const_iterator ti = tris.cbegin(); ti != tris.cend(); ti++)
	{
		// Add a triangle.
		// Vertices are ordered by the right hand rule of the triangle, which means they are counterclockwise around their normal.
		AuPoint n = (*ti).Normal();
		AuPoint v1 = (*ti).Vertex(0);
		AuPoint v2 = (*ti).Vertex(1);
		AuPoint v3 = (*ti).Vertex(2);
		n.Round(FUZZ);
		v1.Round(FUZZ);
		v2.Round(FUZZ);
		v3.Round(FUZZ);
		out << "facet normal "
			<< QString("%1").arg(n.x, 0,'g',3) << " "
			<< QString("%1").arg(n.y, 0,'g',3) << " "
			<< QString("%1").arg(n.z, 0,'g',3)
#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
			<< Qt::endl;
#else
			<< endl;
#endif
		out << "  outer loop"
#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
			<< Qt::endl;
#else
			<< endl;
#endif
		out << "    vertex "
			<< QString("%1").arg(v1.x, 0,'g',3) << " "
			<< QString("%1").arg(v1.y, 0,'g',3) << " "
			<< QString("%1").arg(v1.z, 0,'g',3)
#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
			<< Qt::endl;
#else
			<< endl;
#endif
		out << "    vertex "
			<< QString("%1").arg(v3.x, 0,'g',3) << " "
			<< QString("%1").arg(v3.y, 0,'g',3) << " "
			<< QString("%1").arg(v3.z, 0,'g',3)
#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
			<< Qt::endl;
#else
			<< endl;
#endif
		out << "    vertex "
			<< QString("%1").arg(v2.x, 0,'g',3) << " "
			<< QString("%1").arg(v2.y, 0,'g',3) << " "
			<< QString("%1").arg(v2.z, 0,'g',3)
#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
			<< Qt::endl;
#else
			<< endl;
#endif
		out << "  endloop"
#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
			<< Qt::endl;
#else
			<< endl;
#endif
		out << "endfacet"
#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
			<< Qt::endl;
#else
			<< endl;
#endif
	}
	out << "endsolid "
#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
			<< Qt::endl;
#else
			<< endl;
#endif
	return 0;
}

// returns 0 on success.
int AuScFaceList::STLWriteBinary(const QString& fileName) const
{
	if(empty())
		return 2;	// Nothing to save.

	// Get my triangles.
	AuScTriList tris;
	Triangles(tris);
	if(tris.empty())
		return 3;	// No triangles?

	QFile file(fileName);
	if(!file.open(QIODevice::WriteOnly))
		return 1;	// Failed to open/create file.

	// Create an 80 byte header.
	quint8 header[80];
	for(int h=0; h<80; h++)
		header[h] = 0;

	QDataStream out(&file);
	out.setByteOrder(QDataStream::LittleEndian);
	out.setFloatingPointPrecision(QDataStream::SinglePrecision);
	for(int h=0; h<80; h++)
		out << header[h];
	out << (quint32)tris.size();
	for(AuScTriList::const_iterator ti = tris.cbegin(); ti != tris.cend(); ti++)
	{
		// Add a triangle.
		// Vertices are ordered by the right hand rule of the triangle, which means they are counterclockwise around their normal.
		AuPoint n = (*ti).Normal();
		AuPoint v1 = (*ti).Vertex(0);
		AuPoint v2 = (*ti).Vertex(1);
		AuPoint v3 = (*ti).Vertex(2);

		out << (float)n.x;
		out << (float)n.y;
		out << (float)n.z;

		out << (float)v1.x;
		out << (float)v1.y;
		out << (float)v1.z;

		out << (float)v3.x;
		out << (float)v3.y;
		out << (float)v3.z;

		out << (float)v2.x;
		out << (float)v2.y;
		out << (float)v2.z;

		out << (quint16)0;	// attribute byte count
	}
	return 0;
}

void AuScFaceList::PaintInUse(QList<QColor>& paint) const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
		if((*ti)->IsPainted())
		{
			QColor paintColor((*ti)->Paint());
			paintColor.setAlpha(255);
			if(!paint.contains(paintColor))
				paint.append(paintColor);
		}
}

bool AuScFaceList::CheckFacesPlanar() const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
		if(!(*ti)->IsPlane())
			return false;
	return true;
}

bool AuScFaceList::SelectContiguous(double angleTol, AuScSelectInfo& selInfo) const
{
	// In order to select vertices with the faces, we must know the starting vertex index of each face. Vertex indexes are
	// continuous across all faces.
	QList<quint32> faceVerts;					// Face vertex index at the beginning of each face, by index.
	int vindex = 0;
	for(const_iterator fi = cbegin(); fi != cend(); fi++)
	{
		faceVerts.append(vindex);
		vindex += (*fi)->VertexCount();			// Next face's vertices start after mine.
	}

	// Start selecting faces.
	int selCount = selInfo.m_Faces.size();		// Remember original number of selected faces, for return result.
	QSet<quint32> selFaces;						// The faces selected in each pass.
	QSet<quint32> facesChecked;					// Selected faces where all adjoining faces have already been found.
	do
	{
		// Find faces that are coplanar (within angleTol) and share a common edge with the current selection.
		selFaces.clear();						// Reset for each new pass. We want to which ones are selected in each pass.
		for(QSet<quint32>::const_iterator si = selInfo.m_Faces.cbegin(); si != selInfo.m_Faces.cend(); si++)
		{
			if(facesChecked.contains(*si))
				continue;						// The face's neighbors were already checked.

			// Add unselected faces that share an edge with the selected face and are in the same plane with it.
			const AuScFace* selFace = at(*si);
			if(!selFace)
				continue;						// wut?
			AuPoint fN(selFace->FaceNormal());
			int findex = 0;
			for(const_iterator fi = cbegin(); fi != cend(); fi++, findex++)
			{
				if(selInfo.m_Faces.contains(findex))
					continue;					// The face is already selected.
				// Can the face be selected.
				if(fN.AngleTo((*fi)->FaceNormal()) > angleTol)
					continue;					// The faces are not coplanar enough with each other.
				AuScPoint ev1, ev2;
				if(!selFace->CommonEdgeVertices(*(*fi), ev1, ev2))
					continue;					// Face fi does not share a common edge with the selected face.
				selFaces.insert(findex);		// The face will be selected.
			}
			facesChecked.insert(*si);			// We do not need to check again for faces that border si.
		}

		// Add contiguous coplanar faces and their vertices to the original selection.
		for(QSet<quint32>::const_iterator si = selFaces.cbegin(); si != selFaces.cend(); si++)
		{
			selInfo.m_Faces.insert(*si);
			const AuScFace* selFace = at(*si);
			if(!selFace)
				continue;						// wut?
			vindex = faceVerts[*si];			// First vindex for the face.
			for(int v=0; v < selFace->VertexCount(); v++, vindex++)
				selInfo.m_Vertices.insert(vindex);
		}
	}
	while(!selFaces.empty());					// Do this until we stop selecting faces.
	return selInfo.m_Faces.size() != selCount;	// True if we added to the selection.
}

void AuScFaceList::FenceConform(const AuPoint& post1, const AuPoint& vUp1, const AuPoint& post2, const AuPoint& vUp2, double length)
{
	// Validate calling parameters.
	/*
	if(post1.FuzzyEquals(post2) || vUp1.IsZero() || vUp2.IsZero())
	{
		// No fence. Clear geometry.
		clear();
		return;
	}
	*/

	if(length <= 0)
	{
		// No fence. Clear geometry.
		clear();
		return;
	}

	// Get fence vector for plane calculations.
	AuPoint fenceVec(post2 - post1);
	double fenceLength = fenceVec.Length();
	if(fenceLength == 0)
	{
		clear();
		return;
	}
	fenceVec /= fenceLength;	// Unit vector

	// Scale the fence section to fit between the posts.
	AuMatrix4x4 mLen;
	mLen.Scale(fenceLength/length, 1, 1);
	Transform(mLen);

	// Create a transform to position the fence panel at the first post.
	AuPoint p1v(vUp1.Cross(fenceVec));
	p1v.Unit();
	AuPoint p1x(p1v.Cross(vUp1));
	p1x.Unit();
	AuMatrix4x4 mPost1(post1, p1x, p1v);

	// Create a transform to position the fence panel at the second post.
	AuPoint p2v(vUp2.Cross(fenceVec));
	p2v.Unit();
	AuPoint p2x(p2v.Cross(vUp2));
	p2x.Unit();
	AuMatrix4x4 mPost2(post2, p2x, p2v);
	mPost2.Translate(-fenceLength,0,0);

	// Apply a transformation linearly interpolated between mPost1 and mPost2.
	MorphX(mPost1, mPost2, fenceLength);
	FlattenVertexNormals();
}

void AuScFaceList::MorphX(const AuMatrix4x4& fromM, const AuMatrix4x4& toM, double morphDistance)
{
	for(iterator ti = begin(); ti != end(); ti++)
		(*ti)->MorphX(fromM, toM, morphDistance);
	RecalcExtents();
}

void AuScFaceList::ScalePositiveY(double factor)
{
	for(iterator ti = begin(); ti != end(); ti++)
		(*ti)->ScalePositiveY(factor);
	RecalcExtents();
}

