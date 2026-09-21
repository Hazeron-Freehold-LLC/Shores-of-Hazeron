//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscface.h,v 1.85 2022/04/08 00:17:20 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScFace_H
#define AuScFace_H

#include "aumatrix4x4.h"
#include "ausegment.h"
#include <QSet>
#include <typeinfo>						// for bad_cast

using namespace std;

class AuLine;
class AuMatrix4x4;
class AuScFaceList;
class AuScTriList;
class AuScTriRenderList;
class QDebug;

#define d_SearchTolerance				.025f					// Pixels search tolerance when finding things near the cursor.
#define d_InTrianglePlaneTol			FUZZ

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Surface effects are applied onto rendered triangles to give the appearance of motion or system activity or other conditions.
// NOTE: Do not change these enum values. Add new values at the end. This number gets stored in a file somewhere.
enum SURFACE_EFFECT
{
	EFF_None,
	EFF_FTLDrive1,
	EFF_FTLDrive2,
	EFF_FTLDrive3,
	EFF_FTLDrive4,
	EFF_LifeSupport,
	EFF_ManeuverDrive,
	EFF_PowerPlant,
	EFF_Sensor,
	EFF_Shield,
	EFF_TransporterPad1,
	EFF_TransporterPad2,
	EFF_TransporterPad3,
	EFF_TransporterPad4,
	EFF_TransporterPad5,
	EFF_TransporterPad6,
	EFF_TransporterPad7,
	EFF_TransporterPad8,
	EFF_TransporterPad9,
	EFF_TransporterPad10,
	EFF_Weapon1,
	EFF_Weapon2,
	EFF_Weapon3,
	EFF_Weapon4,
	EFF_Weapon5,
	EFF_Weapon6,
	EFF_Weapon7,
	EFF_Weapon8,
	EFF_Weapon9,
	EFF_Weapon10,
	EFF_Capacitor,
	EFF_Hull,							// Hull surface effect when wormhole or warp drive is hot.
	EFF_Count,
};

inline QString SurfaceEffectName(SURFACE_EFFECT effect)
{
	if(effect >= EFF_Count)
		return "";
	QString effNames[EFF_Count] =
	{
		QObject::tr("No Effect"),
		QObject::tr("FTL Drive 1"),
		QObject::tr("FTL Drive 2"),
		QObject::tr("FTL Drive 3"),
		QObject::tr("FTL Drive 4"),
		QObject::tr("Life Support"),
		QObject::tr("Maneuver Drive"),
		QObject::tr("Power Plant"),
		QObject::tr("Sensor"),
		QObject::tr("Shield"),
		QObject::tr("Transporter Pad 1"),
		QObject::tr("Transporter Pad 2"),
		QObject::tr("Transporter Pad 3"),
		QObject::tr("Transporter Pad 4"),
		QObject::tr("Transporter Pad 5"),
		QObject::tr("Transporter Pad 6"),
		QObject::tr("Transporter Pad 7"),
		QObject::tr("Transporter Pad 8"),
		QObject::tr("Transporter Pad 9"),
		QObject::tr("Transporter Pad 10"),
		QObject::tr("Weapon 1"),
		QObject::tr("Weapon 2"),
		QObject::tr("Weapon 3"),
		QObject::tr("Weapon 4"),
		QObject::tr("Weapon 5"),
		QObject::tr("Weapon 6"),
		QObject::tr("Weapon 7"),
		QObject::tr("Weapon 8"),
		QObject::tr("Weapon 9"),
		QObject::tr("Weapon 10"),
		QObject::tr("Capacitor"),
		QObject::tr("Hull FTL Hot"),
	};
	return effNames[effect];
}

// Returns a number used to sort surface effects in a list control.
// Returns 0 for things that should not be listed.
inline int SurfaceEffectSort(SURFACE_EFFECT effect)
{
	if(effect >= EFF_Count)
		return 0;
	int effSort[EFF_Count] =
	{
		1,	// QObject::tr("None"),
		8,	// QObject::tr("FTL Drive 1"),
		9,	// QObject::tr("FTL Drive 2"),
		10,	// QObject::tr("FTL Drive 3"),
		11,	// QObject::tr("FTL Drive 4"),
		3,	// QObject::tr("Life Support"),
		4,	// QObject::tr("Maneuver Drive"),
		5,	// QObject::tr("Power Plant"),
		6,	// QObject::tr("Sensor"),
		7,	// QObject::tr("Shield"),
		12,	// QObject::tr("Transporter Pad 1"),
		13,	// QObject::tr("Transporter Pad 2"),
		14,	// QObject::tr("Transporter Pad 3"),
		15,	// QObject::tr("Transporter Pad 4"),
		16,	// QObject::tr("Transporter Pad 5"),
		17,	// QObject::tr("Transporter Pad 6"),
		18,	// QObject::tr("Transporter Pad 7"),
		19,	// QObject::tr("Transporter Pad 8"),
		20,	// QObject::tr("Transporter Pad 9"),
		21,	// QObject::tr("Transporter Pad 10"),
		22,	// QObject::tr("Weapon 1"),
		23,	// QObject::tr("Weapon 2"),
		24,	// QObject::tr("Weapon 3"),
		25,	// QObject::tr("Weapon 4"),
		26,	// QObject::tr("Weapon 5"),
		27,	// QObject::tr("Weapon 6"),
		28,	// QObject::tr("Weapon 7"),
		29,	// QObject::tr("Weapon 8"),
		30,	// QObject::tr("Weapon 9"),
		31,	// QObject::tr("Weapon 10"),
		2,	// QObject::tr("Capacitor"),
		0,	// EFF_Hull is not listed.
	};
	return effSort[effect];
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUMATH_DLL AuScSelectInfo
{
public:
	QSet<quint32>						m_Edges;				// Ids of selected edges. Empty = all.
	QSet<quint32>						m_Faces;				// Ids of selected faces. Empty = all.
	QSet<quint32>						m_Vertices;				// Ids of selected vertexes. Empty = all.

public:	// Constructors
	inline AuScSelectInfo()
	{
	}

	inline ~AuScSelectInfo()
	{
	}

	inline AuScSelectInfo(const AuScSelectInfo& c) : m_Edges(c.m_Edges), m_Faces(c.m_Faces), m_Vertices(c.m_Vertices)
	{
	}

public:	// Operators
	inline const AuScSelectInfo& operator=(const AuScSelectInfo& c)
	{
		m_Edges != c.m_Edges;
		m_Faces != c.m_Faces;
		m_Vertices != c.m_Vertices;
		return *this;
	}

	inline bool operator==(const AuScSelectInfo& c) const
	{
		if(m_Edges != c.m_Edges)
			return false;
		if(m_Faces != c.m_Faces)
			return false;
		if(m_Vertices != c.m_Vertices)
			return false;
		return true;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_Edges;
		out << m_Faces;
		out << m_Vertices;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_Edges;
		in >> m_Faces;
		in >> m_Vertices;
	}

public:	// Methods
	inline bool HasEdgeSelection() const
	{
		return !m_Edges.empty();
	}

	inline bool HasFaceSelection() const
	{
		return !m_Faces.empty();
	}

	inline bool HasObjectSelection() const
	{
		return !m_Vertices.empty() || !m_Edges.empty() || HasFaceSelection();
	}

	inline bool HasVertexSelection() const
	{
		return !m_Vertices.empty();
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuScSelectInfo& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScSelectInfo& o)
{
	o << in;
	return in;
}

// qHash is needed to use AuScSelectInfo in a QList in Windows.
AUMATH_DLL unsigned int qHash(const AuScSelectInfo& key);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Container for a set of top level sc object ids.

class AUMATH_DLL AuScIdSet
{
public:
	QSet<quint16>						m_Ids;

public:	// Operators
	inline void operator>>(QDataStream& out) const
	{
		out << (quint16)m_Ids.size();

		for(QSet<quint16>::const_iterator si = m_Ids.cbegin(); si != m_Ids.cend(); si++)
			out << *si;
	}

	inline void operator<<(QDataStream& in)
	{
		m_Ids.clear();

		quint16 c;
		in >> c;

		quint16 id;
		while(c--)
		{
			in >> id;
			m_Ids.insert(id);
		}
	}

public:	// Methods
	inline bool contains(quint16 id) const
	{
		return m_Ids.contains(id);
	}

	static int RegisterMetaType();
};

inline QDataStream& operator<<(QDataStream& out, const AuScIdSet& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScIdSet& o)
{
	o << in;
	return in;
}

Q_DECLARE_METATYPE(AuScIdSet)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Key is hull part id. Value is subselection info.
class AUMATH_DLL AuScSelection : public QHash<quint16, AuScSelectInfo>
{
public:	// Operators
	inline void operator>>(QDataStream& out) const
	{
		out << (quint16)size();

		for(const_iterator si = cbegin(); si != cend(); si++)
		{
			out << si.key();
			out << *si;
		}
	}

	inline void operator<<(QDataStream& in)
	{
		clear();

		quint16 c;
		in >> c;

		quint16 id;
		AuScSelectInfo si;
		while(c--)
		{
			in >> id;
			in >> si;
			insert(id, si);
		}
	}

public:	// Methods
	inline void ClearEdges()
	{
		for(iterator si = begin(); si != end(); si++)
			(*si).m_Edges.clear();
	}

	inline void ClearFaces()
	{
		for(iterator si = begin(); si != end(); si++)
			(*si).m_Faces.clear();
	}

	inline void ClearObjectSelections()
	{
		ClearEdges();
		ClearFaces();
		ClearVertices();
	}

	inline void ClearVertices()
	{
		for(iterator si = begin(); si != end(); si++)
			(*si).m_Vertices.clear();
	}

	bool HasEdgeSelection() const;

	bool HasFaceSelection() const;

	bool HasObjectSelection() const;

	bool HasVertexSelection() const;

	inline AuScIdSet IdSet() const
	{
		AuScIdSet selIds;
		for(const_iterator si = cbegin(); si != cend(); si++)
			selIds.m_Ids.insert(si.key());
		return selIds;
	}

	// 'allSel' contains all possible part object ids for the selected parts.
	void InvertObjectSelection(const AuScSelection& allSel);

	void TogglePartObjectSelection(const AuScSelection& toggleSel);
};

inline QDataStream& operator<<(QDataStream& out, const AuScSelection& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScSelection& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUMATH_DLL AuScPoint : public AuPointTexN
{
	float								m_EffInt;				// Intensity of effect texture.
	float								m_EffTx;				// Effect texture coord.
	float								m_EffTy;				// Effect texture coord.
	QColor								m_Glow;					// Glow color. Alpha ignored.

public:	// Constructors
	inline AuScPoint(const AuPoint& loc, const QPointF& texCoord, const AuPoint& surfN) :
		AuPointTexN(loc, texCoord, surfN)
	{
		m_EffInt = 1;
		m_EffTx = 0;
		m_EffTy = 0;
		m_Glow = QColor(0,0,0);
	}

	inline AuScPoint(const AuPoint& loc, float texX, float texY, const AuPoint& surfN) :
		AuPointTexN(loc, texX, texY, surfN)
	{
		m_EffInt = 1;
		m_EffTx = 0;
		m_EffTy = 0;
		m_Glow = QColor(0,0,0);
	}

	inline AuScPoint(double x, double y, double z, float texX, float texY, const AuPoint& surfN) :
		AuPointTexN(x, y, z, texX, texY, surfN)
	{
		m_EffInt = 1;
		m_EffTx = 0;
		m_EffTy = 0;
		m_Glow = QColor(0,0,0);
	}

	inline AuScPoint(const AuPointTexN& c, float effTx, float effTy, float effInt, const QColor& glow) : AuPointTexN(c)
	{
		m_EffInt = effInt;
		m_EffTx = effTx;
		m_EffTy = effTy;
		m_Glow = glow;
	}

	inline AuScPoint(const AuPointTexN& c) : AuPointTexN(c)
	{
		m_EffInt = 1;
		m_EffTx = 0;
		m_EffTy = 0;
		m_Glow = QColor(0,0,0);
	}

	inline AuScPoint()
	{
		m_EffInt = 1;
		m_EffTx = 0;
		m_EffTy = 0;
		m_Glow = QColor(0,0,0);
	}

	inline AuScPoint(const AuScPoint& c) : AuPointTexN(c)
	{
		m_EffInt = c.m_EffInt;
		m_EffTx = c.m_EffTx;
		m_EffTy = c.m_EffTy;
		m_Glow = c.m_Glow;
	}


public:	// Operators
	inline const AuScPoint& operator=(const AuScPoint& c)
	{
		AuPointTexN::operator=(c);
		m_EffInt = c.m_EffInt;
		m_EffTx = c.m_EffTx;
		m_EffTy = c.m_EffTy;
		m_Glow = c.m_Glow;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		AuPointTexN::operator>>(out);

		quint8 hasWhat = 0;
		if(m_EffTx || m_EffTy)
			hasWhat |= 0x02;
		if(m_EffInt != 1)
			hasWhat |= 0x04;
		if(IsGlowing())
			hasWhat |= 0x10;

		out << hasWhat;

		if(hasWhat & 0x02)
		{
			out << m_EffTx;
			out << m_EffTy;
		}

		if(hasWhat & 0x04)
			out << m_EffInt;

		if(hasWhat & 0x10)
			out << m_Glow;
	}

	inline void operator<<(QDataStream& in)
	{
		AuPointTexN::operator<<(in);

		quint8 hasWhat;
		in >> hasWhat;

		if(hasWhat & 0x02)
		{
			in >> m_EffTx;
			in >> m_EffTy;
		}
		else
		{
			m_EffTx = 0;
			m_EffTy = 0;
		}

		if(hasWhat & 0x04)
			in >> m_EffInt;
		else
			m_EffInt = 1;

		if(hasWhat & 0x10)
			in >> m_Glow;
		else
			m_Glow = QColor(0,0,0);
	}

	inline bool operator==(const AuScPoint& p) const
	{
		if(!AuPointTexN::operator==(p))
			return false;
		if(m_EffInt != p.m_EffInt)
			return false;
		if(m_EffTx != p.m_EffTx)
			return false;
		if(m_EffTy != p.m_EffTy)
			return false;
		if(m_Glow != p.m_Glow)
			return false;
		return true;
	}

	inline bool operator!=(const AuScPoint& p) const
	{
		return !operator==(p);
	}

public:	// Methods
	inline QPointF EffectCoord() const
	{
		return QPointF(m_EffTx, m_EffTy);
	}

	inline float EffectIntensity() const
	{
		return m_EffInt;
	}

	inline float EffectTx() const
	{
		return m_EffTx;
	}

	inline float EffectTy() const
	{
		return m_EffTy;
	}

	inline const QColor& Glow() const
	{
		return m_Glow;
	}

	inline bool IsGlowing() const
	{
		return m_Glow.red() || m_Glow.green() || m_Glow.blue();
	}

	inline AuScPoint LinearInterpolate(const AuScPoint& o, double frac) const
	{
		float mfrac = 1.f - (float)frac;
		float ofrac = (float)frac;
		float effTx = m_EffTx * mfrac + o.m_EffTx * ofrac;
		float effTy = m_EffTy * mfrac + o.m_EffTy * ofrac;
		float effInt = m_EffInt * mfrac + o.m_EffInt * ofrac;
		float gr = m_Glow.redF() * mfrac + o.m_Glow.redF() * ofrac;
		float gg = m_Glow.greenF() * mfrac + o.m_Glow.greenF() * ofrac;
		float gb = m_Glow.blueF() * mfrac + o.m_Glow.blueF() * ofrac;
		QColor glow;
		glow.setRedF(gr);
		glow.setGreenF(gg);
		glow.setBlueF(gb);
		return AuScPoint(AuPointTexN::LinearInterpolate(o, frac), effTx, effTy, effInt, glow);
	}

	inline void MorphX(const AuMatrix4x4& fromM, const AuMatrix4x4& toM, double morphDistance)
	{
		AuPoint pf = fromM.Project(*this);
		AuPoint pt = toM.Project(*this);
		double blend = x / morphDistance;
		if(blend < 0)
			blend = 0;
		else if(blend > 1)
			blend = 1;
		AuPoint::operator=(pf.LinearInterpolate(pt, blend));
	}

	inline void SetEffectCoord(float s, float t)
	{
		m_EffTx = s;
		m_EffTy = t;
	}

	inline void SetEffectIntensity(float i)
	{
		m_EffInt = i;
	}

	inline void SetGlow(const QColor& color)
	{
		m_Glow = color;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuScPoint& t)
{
	t >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScPoint& t)
{
	t << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUMATH_DLL AuScTri
{
private:
	AuPoint								m_N;					// Normal to the clockwise direction of the triangle. Not stored.
	AuScPoint							m_V[3];

public:	// Constructors
	inline AuScTri(const AuScPoint& v0, const AuScPoint& v1, const AuScPoint& v2)
	{
		m_V[0] = v0;
		m_V[1] = v1;
		m_V[2] = v2;
		RecalcNormal();
	}

	inline AuScTri()
	{
	}

	inline AuScTri(const AuScTri& c)
	{
		m_N = c.m_N;
		m_V[0] = c.m_V[0];
		m_V[1] = c.m_V[1];
		m_V[2] = c.m_V[2];
	}

public:	// Operators
	inline const AuScTri& operator=(const AuScTri& c)
	{
		m_N = c.m_N;
		m_V[0] = c.m_V[0];
		m_V[1] = c.m_V[1];
		m_V[2] = c.m_V[2];
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_V[0];
		out << m_V[1];
		out << m_V[2];
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_V[0];
		in >> m_V[1];
		in >> m_V[2];
		RecalcNormal();
	}

	inline bool operator==(const AuScTri& t) const
	{
		if(m_N != t.m_N)
			return false;
		if(m_V[0] != t.m_V[0])
			return false;
		if(m_V[1] != t.m_V[1])
			return false;
		if(m_V[2] != t.m_V[2])
			return false;
		return true;
	}

	inline bool operator!=(const AuScTri& t) const
	{
		return !operator==(t);
	}

public:	// Methods
	double Area() const;

	inline AuPoint Center() const
	{
		// From http://mathforum.org/library/drmath/view/54899.html
		return (m_V[0] + m_V[1] + m_V[2]) / 3.;
	}

	bool Collapsed() const;

	// Returns true if this triangle shares an edge line 'tri'.
	bool ColinearEdge(const AuScTri& tri) const;

	inline bool Contains(const AuScPoint& v) const
	{
		return v.InTriangle_3D(m_V[0], m_V[1], m_V[2]);
	}

	inline bool ContainsFuzzy(const AuPoint& v) const
	{
		return v.InTriangleFuzzy_3D(m_V[0], m_V[1], m_V[2], Normal(), d_InTrianglePlaneTol);
	}

	double Distance(const AuPoint& v) const;

	void Divide(AuScTriList& pieces) const;

	// Fills 'intersections' with all unique intersections between 'line' and my edge lines.
	void EdgeIntersections(const AuLine& line, AuPointArray& intersections) const;

	// Returns true if any of my edges is on the 'line'. Fuzzy test.
	// 'edgeSeg' is set to the edge that is on the line, if any; otherwise it is untouched.
	bool EdgeOnLine(const AuLine& line, AuSegment& edgeSeg) const;

	void EffectCylindrical(quint16 texRepsAtEquator, double texSize, const AuMatrix4x4& gridPlane);

	// Effect coords at all vertices are moved using the same vector.
	void EffectMove(const QPointF& vec);

	void EffectMove(int vertexIndex, const QPointF& vec);

	void EffectProject(double texSize, const AuMatrix4x4& gridPlane);

	void EffectRotate(int vertexIndex, const QPointF& pivot, double angle);

	void EffectSpherical(quint16 texRepsAtEquator, const AuMatrix4x4& gridPlane);

	void EffectStretch(int vertexIndex, const QPointF& base, double ScaleX, double scaleY);

	inline void Extents(AuPoint& min, AuPoint& max) const
	{
		min = m_V[0];
		min = m_V[1].Min(min);
		min = m_V[2].Min(min);

		max = m_V[0];
		max = m_V[1].Max(max);
		max = m_V[2].Max(max);
	}

	// Used by a site mesh to find the 2D intersection of 'loc' with this mesh. Returns true if intersection found.
	bool InSite(const AuPoint& loc) const;

	// Calculates the intersection of a ray from 'loc' with this triangle.
	// Returns true if an intersection is found.
	// 'intersect' is set to the result if an intersection is found.
	bool IntersectRay(const AuPoint& loc, const AuPoint& vec, AuPoint& intersect) const;

	// Calculates the intersection of the Z axis through 'loc' with this triangle.
	// Returns true if an intersection is found.
	// 'intersect' is set to the result if an intersection is found.
	bool IntersectZ(const AuPoint& loc, AuPoint& intersect) const;

	void MorphX(const AuMatrix4x4& fromM, const AuMatrix4x4& toM, double morphDistance);

	// Returns triangle normal based on clockwise draw direction.
	inline const AuPoint& Normal() const
	{
		return m_N;
	}

	inline void RecalcNormal()
	{
		AuPoint vx = m_V[2] - m_V[0];
		vx.Unit();
		AuPoint vy = m_V[1] - m_V[0];
		vy.Unit();
		m_N = vx.Cross(vy);
		m_N.Unit();
	}

	inline void ReverseFacing()
	{
		// Note this reversal works with AuScFaceQuad to keep the implied edges to the inside.
		AuScPoint s(m_V[2]);
		m_V[2] = m_V[0];
		m_V[0] = s;
		RecalcNormal();		// So we always get the same result, instead of just negating m_N.
	}

	inline void ReverseNormals()
	{
		m_V[0].m_N = -m_V[0].m_N;
		m_V[1].m_N = -m_V[1].m_N;
		m_V[2].m_N = -m_V[2].m_N;
	}

	inline void ScalePositiveY(double factor)
	{
		bool anyChange = false;
		for(int v=0; v<3; v++)
			if(m_V[v].y > 0)
			{
				m_V[v].y *= factor;
				m_V[v].m_N.y *= factor;
				m_V[v].m_N.Unit();
				anyChange = true;
			}
		if(anyChange)
			RecalcNormal();
	}

	inline void SetVertexEffectCoord(int index, float u, float v)
	{
		m_V[index].SetEffectCoord(u, v);
	}

	inline void SetVertexEffectIntensity(int index, float intensity)
	{
		m_V[index].SetEffectIntensity(intensity);
	}

	inline void SetVertexGlow(int index, const QColor& color)
	{
		m_V[index].SetGlow(color);
	}

	inline void SetVertexNormal(int index, const AuPoint& n)
	{
		m_V[index].m_N = n;
	}

	inline void SetVertexTextureCoord(int index, float u, float v)
	{
		m_V[index].SetTexCoord(u, v);
	}

	// Returns true if the segment from 'eyeLoc' to 'sightLoc' intersects this triangle.
	// Adjusts 'sightLoc' closer to 'eyeLoc' to the intersection location if there is one.
	// Returns false if there is no intersection and 'sightLoc' is unchanged.
	bool Sight(const AuPoint& eyeLoc, AuPoint& sightLoc) const;

	// Used by a site mesh to find the 2D intersection of 'loc' with this mesh. Returns the 3D face location in 'siteLoc'.
	// Interpolates the red glow between vertices to return a number from 1=no glow to 0=max red glow.
	float SiteInfluence(const AuPoint& loc, AuPoint& siteLoc) const;

	// Returns true if this triangle is sliced by the 'cutLine'. Pieces are added to 'pieces'.
	// Returns false if this triangle is not sliced and nothing is added to 'result'.
	bool Slice(const AuLine& cutLine, AuScTriList& pieces) const;

	void TextureCylindrical(quint16 texRepsAtEquator, double texSize, const AuMatrix4x4& gridPlane);

	// Texture coords at all vertices are moved using the same vector.
	void TextureMove(const QPointF& vec);

	void TextureMove(int vertexIndex, const QPointF& vec);

	void TextureProject(double texSize, const AuMatrix4x4& gridPlane);

	void TextureRotate(int vertexIndex, const QPointF& pivot, double angle);

	void TextureSpherical(quint16 texRepsAtEquator, const AuMatrix4x4& gridPlane);

	void TextureStretch(int vertexIndex, const QPointF& base, double ScaleX, double scaleY);

	inline void Transform(const AuMatrix4x4& t)
	{
		m_V[0].Transform(t);
		m_V[1].Transform(t);
		m_V[2].Transform(t);
		RecalcNormal();
	}

	inline void TransformVertex(const AuMatrix4x4& t, int index)
	{
		m_V[index].Transform(t);
		RecalcNormal();
	}

	inline const AuScPoint& V0() const
	{
		return m_V[0];
	}

	inline const AuScPoint& V1() const
	{
		return m_V[1];
	}

	inline const AuScPoint& V2() const
	{
		return m_V[2];
	}

	inline const AuScPoint& Vertex(int index) const
	{
		return m_V[index];
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuScTri& t)
{
	t >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScTri& t)
{
	t << in;
	return in;
}

AUMATH_DLL QDebug operator<<(QDebug dbg, const AuScTri& t);

// qHash is needed to use AuScTri in a QList in Windows.
AUMATH_DLL unsigned int qHash(const AuScTri& key);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUMATH_DLL AuScTriList : public QList<AuScTri>
{
public:	// Constructors

public:	// Operators
	inline void operator>>(QDataStream& out) const
	{
		out << (quint32)count();
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
			AuScTri t;
			in >> t;
			append(t);
		}
	}

	inline bool operator==(const AuScTriList& l) const
	{
		if(size() != l.size())
			return false;

		for(const_iterator ti = cbegin(), lti=l.cbegin(); ti != cend() && lti != l.cend(); ti++, lti++)
			if(*ti != *lti)
				return false;
		
		return true;
	}

	inline bool operator!=(const AuScTriList& l) const
	{
		return !operator==(l);
	}

public:	// Methods
	inline AuPoint Center() const
	{
		AuPoint avg;
		if(size())
		{
			avg = first().Center();
			if(size() > 1)
			{
				for(const_iterator ti = cbegin() + 1; ti != cend(); ti++)
					avg += (*ti).Center();
				avg /= size();
			}
		}
		return avg;
	}

	// Returns true if any of my triangles shares an edge line with any of 'tris'.
	bool ColinearEdge(const AuScTriList& tris) const;

	// Returns true if all triangles are collapsed.
	bool Collapsed() const;

	// Fills 'intersections' with all unique intersections between 'line' and my triangle edge lines.
	void EdgeIntersections(const AuLine& line, AuPointArray& intersections) const;

	void Divide();

	inline void Extents(AuPoint& min, AuPoint& max) const
	{
		if(!size())
			return;

		first().Extents(min, max);
		if(size() > 1)
		{
			AuPoint tmin, tmax;
			for(const_iterator ti = cbegin() + 1; ti != cend(); ti++)
			{
				(*ti).Extents(tmin, tmax);
				min = min.Min(tmin);
				max = max.Max(tmax);
			}
		}
	}

	inline bool IntersectRay(const AuPoint& loc, const AuPoint& vec, AuPoint& intersect) const
	{
		for(const_iterator ti = cbegin(); ti != cend(); ti++)
			if((*ti).IntersectRay(loc, vec, intersect))
				return true;
		return false;
	}

	inline bool IntersectZ(const AuPoint& loc, AuPoint& intersect) const
	{
		for(const_iterator ti = cbegin(); ti != cend(); ti++)
			if((*ti).IntersectZ(loc, intersect))
				return true;
		return false;
	}

	inline quint64 MemoryUsage() const
	{
		return sizeof(AuScTri) * size();
	}

	// Retrieves triangles ready for rendering.
	void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris, quint8 textureIndex,
			const QColor& paint, float reflectivity, float shine, float selfIllum, float opacity, SURFACE_EFFECT effect) const;

	inline void ReverseFacing()
	{
		for(iterator ti = begin(); ti != end(); ti++)
			(*ti).ReverseFacing();
	}

	inline void ReverseNormals()
	{
		for(iterator ti = begin(); ti != end(); ti++)
			(*ti).ReverseNormals();
	}

	inline void ScalePositiveY(double factor)
	{
		for(iterator ti = begin(); ti != end(); ti++)
			(*ti).ScalePositiveY(factor);
	}

	inline void Transform(const AuMatrix4x4& t)
	{
		for(iterator ti = begin(); ti != end(); ti++)
			(*ti).Transform(t);
	}

	AuScPoint VertexAt(const AuPoint& v) const;
};

inline QDataStream& operator<<(QDataStream& out, const AuScTriList& t)
{
	t >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScTriList& t)
{
	t << in;
	return in;
}

AUMATH_DLL QDebug operator<<(QDebug dbg, const AuScTriList& t);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUMATH_DLL AuScTriRender : public AuScTri
{
	quint8								m_Effect;				// SURFACE_EFFECT.
	float								m_Opacity;				// 0=invisible to 1=opaque.
	QColor								m_Paint;				// Paint color including alpha blend. Alpha 0 = no paint.
	float								m_Reflectivity;			// Should be in the range 0 to 1. a.k.a. specular intensity
	float								m_SelfIllumination;		// 0=lit naturally to 1=fully self lit
	float								m_Shine;				// Specular exponent >= 1. a.k.a. specular hardness
	quint8								m_TexIndex;				// Texture index (0 to 14), for Sc.f shader.

public:	// Constructors
	inline AuScTriRender(const AuScTri& t, quint8 textureIndex, const QColor& paint, float reflectivity, float shine,
			float selfIllum, float opacity, SURFACE_EFFECT effect) :
		AuScTri(t), m_Paint(paint)
	{
		m_Effect = effect;
		m_Opacity = opacity;
		m_Reflectivity = reflectivity;
		m_SelfIllumination = selfIllum;
		m_Shine = shine;
		m_TexIndex = textureIndex;
	}

	inline AuScTriRender()
	{
		m_Effect = EFF_None;
		m_Opacity = 1;
		m_Paint = QColor(0,0,0,0);
		m_Reflectivity = 0;
		m_SelfIllumination = 0;
		m_Shine = 0;
		m_TexIndex = 0;
	}

	inline AuScTriRender(const AuScTriRender& c) : AuScTri(c)
	{
		m_Effect = c.m_Effect;
		m_Opacity = c.m_Opacity;
		m_Paint = c.m_Paint;
		m_Reflectivity = c.m_Reflectivity;
		m_SelfIllumination = c.m_SelfIllumination;
		m_Shine = c.m_Shine;
		m_TexIndex = c.m_TexIndex;
	}

public:	// Operators
	inline const AuScTriRender& operator=(const AuScTriRender& c)
	{
		AuScTri::operator=(c);
		m_Effect = c.m_Effect;
		m_Opacity = c.m_Opacity;
		m_Paint = c.m_Paint;
		m_Reflectivity = c.m_Reflectivity;
		m_SelfIllumination = c.m_SelfIllumination;
		m_Shine = c.m_Shine;
		m_TexIndex = c.m_TexIndex;
		return *this;
	}

public:	// Methods
	inline SURFACE_EFFECT Effect() const
	{
		return (SURFACE_EFFECT)m_Effect;
	}

	// Returns the application layer for the effect, based on the kind of effect.
	// 0=no effect, 1=lit, 2=unlit
	inline quint8 EffectAppl() const
	{
		switch(m_Effect)
		{
		case EFF_None:
		case EFF_Count:
			break;
		case EFF_Capacitor:
		case EFF_FTLDrive1:
		case EFF_FTLDrive2:
		case EFF_FTLDrive3:
		case EFF_FTLDrive4:
		case EFF_Hull:
		case EFF_LifeSupport:
		case EFF_ManeuverDrive:
		case EFF_PowerPlant:
		case EFF_Sensor:
		case EFF_Shield:
		case EFF_TransporterPad1:
		case EFF_TransporterPad2:
		case EFF_TransporterPad3:
		case EFF_TransporterPad4:
		case EFF_TransporterPad5:
		case EFF_TransporterPad6:
		case EFF_TransporterPad7:
		case EFF_TransporterPad8:
		case EFF_TransporterPad9:
		case EFF_TransporterPad10:
		case EFF_Weapon1:
		case EFF_Weapon2:
		case EFF_Weapon3:
		case EFF_Weapon4:
		case EFF_Weapon5:
		case EFF_Weapon6:
		case EFF_Weapon7:
		case EFF_Weapon8:
		case EFF_Weapon9:
		case EFF_Weapon10:
			// These effects appear at full brightness. They are not lit by environmental lighting.
			return 2;
		}
		return 0;
	}

	inline float Opacity() const
	{
		return m_Opacity;
	}

	inline const QColor& Paint() const
	{
		return m_Paint;
	}

	inline float Reflectivity() const
	{
		return m_Reflectivity;
	}

	inline float SelfIllumination() const
	{
		return m_SelfIllumination;
	}

	inline void SetSelfIllumination(float si)
	{
		m_SelfIllumination = si;
	}

	inline float Shine() const
	{
		return m_Shine;
	}

	inline quint8 TextureIndex() const
	{
		return m_TexIndex;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUMATH_DLL AuScTriRenderList : public QMultiMap<SURFACE_EFFECT, AuScTriRender>
{
public:	// Constructors

public:	// Operators
	inline bool operator==(const AuScTriRenderList& l) const
	{
		if(size() != l.size())
			return false;

		for(const_iterator ti = cbegin(), lti=l.cbegin(); ti != cend() && lti != l.cend(); ti++, lti++)
			if(*ti != *lti)
				return false;
		
		return true;
	}

	inline bool operator!=(const AuScTriRenderList& l) const
	{
		return !operator==(l);
	}

public:	// Methods
	// Merges 'l' with this list.
	void Insert(const AuScTriRenderList& l);

	inline void SetSelfIllumination(float si)
	{
		for(iterator ti = begin(); ti != end(); ti++)
			(*ti).SetSelfIllumination(si);
	}

	inline void Transform(const AuMatrix4x4& t)
	{
		for(iterator ti = begin(); ti != end(); ti++)
			(*ti).Transform(t);
	}
};

#if defined(Q_OS_WIN)
inline const AuScTriRender QMultiMap<SURFACE_EFFECT, AuScTriRender>::operator[](const SURFACE_EFFECT& e) const
{
	const_iterator ti = find(e);
	if(ti != cend())
		return *ti;
	else
		return AuScTriRender();
}

inline AuScTriRender& QMultiMap<SURFACE_EFFECT, AuScTriRender>::operator[](const SURFACE_EFFECT& e)
{
	iterator ti = find(e);
	Q_ASSERT(ti != end());
	return *ti;
}
#endif

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// A surface with n vertices. e.g. Triangle, quad, polygon.

class AUMATH_DLL AuScFace
{
public:
	enum COLOR_THEME
	{
		THEME_ExteriorPrimary,
		THEME_ExteriorSecondary,
		THEME_ExteriorTertiary,
		THEME_InteriorPrimary,
		THEME_InteriorSecondary,
		THEME_InteriorTertiary,
		THEME_Count						// LAST ITEM
	};

private:
	quint8								m_ColorTheme;			// COLOR_THEME with m_Paint's alpha. m_Paint used when THEME_Count.
	quint8								m_Effect;				// SURFACE_EFFECT.
	float								m_Opacity;				// 0=invisible to 1=opaque.
	QColor								m_Paint;				// Paint color including alpha blend. Alpha 0 = no paint.
	float								m_Reflectivity;
	float								m_SelfIllumination;		// 0=lit naturally to 1=fully self lit
	float								m_Shine;
	quint8								m_TexIndex;				// Texture index (0 to 15).

public:	// Constructors
	inline AuScFace()
	{
		m_ColorTheme = THEME_Count;
		m_Effect = EFF_None;
		m_Opacity = 1;
		m_Paint = QColor(0,0,0,0);
		m_Reflectivity = 0;
		m_SelfIllumination = 0;
		m_Shine = 0;
		m_TexIndex = 0;
	}

	inline AuScFace(const AuScFace& c)
	{
		m_ColorTheme = c.m_ColorTheme;
		m_Effect = c.m_Effect;
		m_Opacity = c.m_Opacity;
		m_Paint = c.m_Paint;
		m_Reflectivity = c.m_Reflectivity;
		m_SelfIllumination = c.m_SelfIllumination;
		m_Shine = c.m_Shine;
		m_TexIndex = c.m_TexIndex;
	}

	virtual ~AuScFace()
	{
	}

public:	// Operators
	inline const AuScFace& operator=(const AuScFace& c)
	{
		m_ColorTheme = c.m_ColorTheme;
		m_Effect = c.m_Effect;
		m_Opacity = c.m_Opacity;
		m_Paint = c.m_Paint;
		m_Reflectivity = c.m_Reflectivity;
		m_SelfIllumination = c.m_SelfIllumination;
		m_Shine = c.m_Shine;
		m_TexIndex = c.m_TexIndex;
		return *this;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScFace& f) const
	{
		if(m_ColorTheme != f.m_ColorTheme)
			return false;
		if(m_Effect != f.m_Effect)
			return false;
		if(m_TexIndex != f.m_TexIndex)
			return false;
		if(m_Opacity != f.m_Opacity)
			return false;
		if(m_Paint != f.m_Paint)
			return false;
		if(m_Reflectivity != f.m_Reflectivity)
			return false;
		if(m_SelfIllumination != f.m_SelfIllumination)
			return false;
		if(m_Shine != f.m_Shine)
			return false;
		return true;
	}

	inline bool operator!=(const AuScFace& f) const
	{
		return !operator==(f);
	}

public:	// Methods;
	virtual double Area() const = 0;

	virtual AuScFace* Clone() const = 0;

	// Returns true if any of this face's triangles shares an edge with 'face'.
	bool ColinearTriangleEdge(const AuScFace& face) const;

	virtual bool Collapsed() const = 0;

	// When THEME_Count is returned, Paint() color will be used.
	inline COLOR_THEME ColorTheme() const
	{
		return (COLOR_THEME)m_ColorTheme;
	}

	// Finds a common edge with 'f'.
	// Returns true if a common edge is found and edge vertices are returned in 'v1' and 'v2'.
	// Returns false if no common edge is found and 'v1' and 'v2' are unchanged.
	bool CommonEdgeVertices(const AuScFace& f, AuScPoint& v1, AuScPoint& v2) const;

	// Finds a common edge with 'f' horizontally.
	// Returns true if a common edge is found and edge vertices are returned in 'v1' and 'v2'.
	// Returns false if no common edge is found and 'v1' and 'v2' are unchanged.
	bool CommonEdgeVerticesHorizontal(const AuScFace& f, AuScPoint& v1, AuScPoint& v2) const;

	// Finds a common edge with 'f' vertically.
	// Returns true if a common edge is found and edge vertices are returned in 'v1' and 'v2'.
	// Returns false if no common edge is found and 'v1' and 'v2' are unchanged.
	bool CommonEdgeVerticesVertical(const AuScFace& f, AuScPoint& v1, AuScPoint& v2) const;

	// Returns true if any of my triangles ContainsFuzzy 'v'.
	virtual bool ContainsFuzzy(const AuPoint& v) const = 0;

	static AuScFace* Create(quint8 type);

	// Looks for the last point in 'path' to be on an edge. If found, finds a path across this face and appends it to 'path'.
	// The edge crossed on the opposite side is returned in 'exitEdge'.
	// Returns true on success.
	// Returns false if the end of 'path' is not on an edge line.
	// Returns false if no path across can be determined.
	bool Cross(AuPointArray& path, AuSegment& exitEdge) const;

	// Looks for an edge that matches 'entryEdge'. If found, finds a path across this face, starting from the last point in 'path',
	// and appends it to 'path'.
	// The edge crossed on the opposite side is returned in 'exitEdge'.
	// Returns true on success.
	// Returns false if 'entryEdge' is not one of my edge lines.
	// Returns false if no path across can be determined.
	virtual bool Cross(AuPointArray& path, const AuSegment& entryEdge, AuSegment& exitEdge) const = 0;

	// Slices faces using a polyline. If the cut is known to be a closed boundary, consider using Punch instead.
	bool Cut(const AuPointArray& cutLine, AuScFaceList& result);

	virtual double Distance(const AuPoint& v) const = 0;

	virtual void Divide(quint8 divisions, AuScFaceList& nufaces) const = 0;

	// Fills 'intersections' with all unique intersections between 'line' and my edge lines.
	virtual void EdgeIntersections(const AuLine& line, AuPointArray& intersections) const = 0;

	virtual int Edges() const = 0;

	virtual void EdgeSegment(quint32 edgeIndex, AuSegment& edges) const = 0;

	virtual void EdgeSegments(AuSegmentArray& edges) const = 0;

	inline SURFACE_EFFECT Effect() const
	{
		return (SURFACE_EFFECT)m_Effect;
	}

	virtual void EffectCylindrical(SURFACE_EFFECT effect, quint16 texRepsAtEquator, double texSize,
			const AuMatrix4x4& gridPlane) = 0;

	// Effect coords at all vertices are moved using the same vector.
	virtual void EffectMove(const QPointF& vec) = 0;

	virtual void EffectMove(int vertexIndex, const QPointF& vec) = 0;

	virtual void EffectProject(SURFACE_EFFECT effect, double texSize, const AuMatrix4x4& gridPlane) = 0;

	virtual void EffectRotate(int vertexIndex, const QPointF& pivot, double angle) = 0;

	virtual void EffectSpherical(SURFACE_EFFECT effect, quint16 texRepsAtEquator, const AuMatrix4x4& gridPlane) = 0;

	virtual void EffectStretch(int vertexIndex, const QPointF& base, double ScaleX, double scaleY) = 0;

	void EffectUnwrap(SURFACE_EFFECT effect, double texSize, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane);

	// Sets 'min' and 'max' to the 3D extents of my vertices.
	virtual void Extents(AuPoint& min, AuPoint& max) const = 0;

	virtual AuPoint FaceLocation() const = 0;

	virtual AuPoint FaceNormal() const = 0;

	// Returns a plane matrix oriented to this face.
	// This base class implementation creates a plane from my face normal.
	virtual AuMatrix4x4 FacePlane() const;

	virtual void FlattenVertexNormals() = 0;

	// Used by a site mesh to find the 2D intersection of 'loc' with this mesh. Returns true if intersection found.
	virtual bool InSite(const AuPoint& loc) const = 0;

	// Calculates the intersection of a ray from 'loc' with this face.
	// Returns true if an intersection is found.
	// 'intersect' is set to the result if an intersection is found.
	virtual bool IntersectRay(const AuPoint& loc, const AuPoint& vec, AuPoint& intersect) const = 0;

	// Calculates the intersection of the Z axis through 'loc' with this face.
	// Returns true if an intersection is found.
	// 'intersect' is set to the result if an intersection is found.
	virtual bool IntersectZ(const AuPoint& loc, AuPoint& intersect) const = 0;

	bool InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, bool findVertices, bool findEdges, bool findFaces) const;

	inline bool IsPainted() const
	{
		return m_Paint.alpha();
	}

	// Returns true if all my vertices are in the same plane.
	virtual bool IsPlane() const = 0;

	inline bool IsTransparent() const
	{
		return Opacity() != 1;
	}

	// Joins face 'f' with this face if possible.
	// Returns 0 if they cannot be joined.
	// Returns the result if they can be joined.
	virtual AuScFace* Join(const AuScFace& f) const = 0;

	virtual quint64 MemoryUsage() const;

	virtual void MorphX(const AuMatrix4x4& fromM, const AuMatrix4x4& toM, double morphDistance) = 0;

	inline float Opacity() const
	{
		return m_Opacity;
	}

	// Packs this patches' type and data into a byte array using operator>>. Use Unpack to restore.
	QByteArray Pack() const;

	inline const QColor& Paint() const
	{
		return m_Paint;
	}

	inline QColor PaintColor(const QColor themeColors[THEME_Count]) const
	{
		switch(ColorTheme())
		{
		case THEME_ExteriorPrimary:
		case THEME_ExteriorSecondary:
		case THEME_ExteriorTertiary:
		case THEME_InteriorPrimary:
		case THEME_InteriorSecondary:
		case THEME_InteriorTertiary:
			{
				// Get the theme color but use the blend factor from the paint color.
				QColor color(themeColors[ColorTheme()]);
				color.setAlpha(m_Paint.alpha());
				return color;
			}
		case THEME_Count:
			break;
		}
		return m_Paint;
	}

	// Slices faces using a closed boundary. If the array is not a closed boundary, use Cut instead.
	bool Punch(const AuPointArray& cutBnd, AuScFaceList& result);

	// Retrieves triangles ready for rendering.
	virtual void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count]) const = 0;

	inline float Reflectivity() const
	{
		return m_Reflectivity;
	}

	virtual void ReverseTriangleFacing() = 0;

	virtual void ReverseVertexNormals() = 0;

	// Fuzzy comparison of vertices.
	bool SamePlace(const AuScFace& face) const;

	// Multiplies the 'y' ordinate of all coordinates by 'factor' whose 'y' is already greater than zero.
	// This is used when creating a space helmet, to stretch the front (+Y) part to fit facial features.
	virtual void ScalePositiveY(double factor) = 0;

	inline float SelfIllumination() const
	{
		return m_SelfIllumination;
	}

	inline void SetEffect(SURFACE_EFFECT effect)
	{
		m_Effect = effect;
	}

	inline void SetSelfIllumination(float selfIllum)
	{
		m_SelfIllumination = selfIllum;
	}

	inline void SetOpacity(float o)
	{
		m_Opacity = o;
	}

	// Alpha of the paint color is the blend factor onto the underlying texture.
	inline void SetPaint(const QColor& paint)
	{
		m_ColorTheme = THEME_Count;
		m_Paint = paint;
	}

	inline void SetPaint(COLOR_THEME theme, quint8 opacity)
	{
		if(theme < 0 || theme > THEME_Count)
			return;
		m_ColorTheme = theme;
		m_Paint.setAlpha(opacity);
	}

	inline void SetReflectivity(float r)
	{
		m_Reflectivity = r;
	}

	inline void SetShine(float s)
	{
		m_Shine = s;
	}

	inline void SetTextureIndex(quint8 texIndex)
	{
		m_TexIndex = texIndex;
	}

	virtual void SetVertexEffectIntensity(int index, float intensity) = 0;

	virtual void SetVertexGlow(int index, const QColor& color) = 0;

	virtual void SetVertexNormal(int index, const AuPoint& n) = 0;

	// Returns true if this face shares an edge with 'f' that is not on any of the 'cutLine' segments.
	bool SharesUncutEdge(const AuScFace& f, const AuPointArray& cutLine) const;

	inline float Shine() const
	{
		return m_Shine;
	}

	// Returns true if the segment from 'eyeLoc' to 'sightLoc' intersects this face.
	// Adjusts 'sightLoc' closer to 'eyeLoc' to the intersection location if there is one.
	// Returns false if there is no intersection and 'sightLoc' is unchanged.
	virtual bool Sight(const AuPoint& eyeLoc, AuPoint& sightLoc) const = 0;

	// Used by a site mesh to find the 2D intersection of 'loc' with this mesh. Returns the 3D face location in 'siteLoc'.
	// Interpolates the red glow between vertices to return a number from 1=no glow to 0=max red glow.
	virtual float SiteInfluence(const AuPoint& loc, AuPoint& siteLoc) const = 0;

	// Slices this face using face 'cutter'.
	// Returns true if this face is sliced and appends result to 'nufaces'.
	bool Slice(const AuScFace& cutter, AuScFaceList& nufaces) const;

	// Slices this face using 'cutLine'.
	// Appends the result to 'nufaces'.
	// Appends this face to 'nufaces' if it is not sliced.
	virtual void Slice(const AuLine& cutLine, AuScFaceList& nufaces) const = 0;

	// Returns true if a segment from 'seg1' to 'seg2' would slice this face into multiple faces.
	virtual bool TestSlice(const AuPoint& seg1, const AuPoint& seg2) const = 0;

	inline quint8 TextureIndex() const
	{
		return m_TexIndex;
	}

	virtual void TextureCylindrical(quint8 texIndex, quint16 texRepsAtEquator, double texSize, const AuMatrix4x4& gridPlane) = 0;

	// Texture coords at all vertices are moved using the same vector.
	virtual void TextureMove(const QPointF& vec) = 0;

	virtual void TextureMove(int vertexIndex, const QPointF& vec) = 0;

	virtual void TextureProject(quint8 texIndex, double texSize, const AuMatrix4x4& gridPlane) = 0;

	virtual void TextureRotate(int vertexIndex, const QPointF& pivot, double angle) = 0;

	virtual void TextureSpherical(quint8 texIndex, quint16 texRepsAtEquator, const AuMatrix4x4& gridPlane) = 0;

	virtual void TextureStretch(int vertexIndex, const QPointF& base, double ScaleX, double scaleY) = 0;

	void TextureUnwrap(quint8 texIndex, double texSize, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane);

	virtual void Transform(const AuMatrix4x4& t) = 0;

	// 't' is the transformation matrix.
	// 'firstVertex' is the id of my first vertex.
	// 'vertexIndices' are the vertex ids that are to be transformed.
	virtual void Transform(const AuMatrix4x4& t, quint32 firstVertex, const QSet<quint32>& vertexIndices,
			quint32 firstEdge, const QSet<quint32>& edgeIndices, bool faceIsSelected) = 0;

	virtual void Triangles(AuScTriList& tris) const = 0;

	virtual quint8 Type() const = 0;

	// Restores a patch from a byte array that was stored using Pack.
	// The returned pointer is allocated on the heap and becomes the property of the caller.
	static AuScFace* Unpack(const QByteArray& ba);

	virtual AuScPoint Vertex(int index) const = 0;

	// Returns index of vertex at 'loc'. Uses FuzzyEquals.
	// Returns -1 if none is there.
	virtual int VertexAt(const AuPoint& loc) const = 0;

	virtual int VertexCount() const = 0;

	virtual void VertexLocations(AuPointArray& verts) const = 0;

	inline double ViewEdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& edgeIndex, AuPoint& edgeLoc) const
	{
		AuSegmentArray segs;
		EdgeSegments(segs);
		return segs.ViewDistance(mvp, pixel, edgeIndex, edgeLoc);
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuScFace& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScFace& o)
{
	o << in;
	return in;
}

AUMATH_DLL QDebug operator<<(QDebug dbg, const AuScFace& f);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUMATH_DLL AuScFaceTri : public AuScFace
{
	AuScTri								m_Tri;

public:	// Constructors
	AuScFaceTri(const AuScPoint& v0, const AuScPoint& v1, const AuScPoint& v2);

	AuScFaceTri(const AuScFace& c, const AuScPoint& v0, const AuScPoint& v1, const AuScPoint& v2);

	AuScFaceTri(const AuScFace& c, const AuScTri& t);

	inline AuScFaceTri(const AuScFaceTri& c) : AuScFace(c), m_Tri(c.m_Tri)
	{
	}

	AuScFaceTri();

public:	// Operators
	inline const AuScFaceTri& operator=(const AuScFaceTri& c)
	{
		AuScFace::operator=(c);
		m_Tri = c.m_Tri;
		return *this;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScFace& f) const
	{
		if(!AuScFace::operator==(f))
			return false;

		try
		{
			const AuScFaceTri& o = dynamic_cast<const AuScFaceTri&>(f);

			if(m_Tri != o.m_Tri)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:	// Methods
	virtual double Area() const;

	virtual AuScFace* Clone() const
	{
		return new AuScFaceTri(*this);
	}

	virtual bool Collapsed() const;

	virtual bool ContainsFuzzy(const AuPoint& v) const;

	virtual bool Cross(AuPointArray& path, const AuSegment& entryEdge, AuSegment& exitEdge) const;

	virtual double Distance(const AuPoint& v) const;

	virtual void Divide(quint8 divisions, AuScFaceList& nufaces) const;

	virtual void EdgeIntersections(const AuLine& line, AuPointArray& intersections) const;

	virtual int Edges() const
	{
		return 3;
	}

	virtual void EdgeSegment(quint32 edgeIndex, AuSegment& edge) const
	{
		if(edgeIndex == 0)
		{
			edge = AuSegment(m_Tri.V0(), m_Tri.V1());
		}
		else if(edgeIndex == 1)
		{
			edge = AuSegment(m_Tri.V1(), m_Tri.V2());
		}
		else
		{
			edge = AuSegment(m_Tri.V2(), m_Tri.V0());
		}
	}

	virtual void EdgeSegments(AuSegmentArray& edges) const
	{
		edges.append(AuSegment(m_Tri.V0(), m_Tri.V1()));
		edges.append(AuSegment(m_Tri.V1(), m_Tri.V2()));
		edges.append(AuSegment(m_Tri.V2(), m_Tri.V0()));
	}

	virtual void EffectCylindrical(SURFACE_EFFECT effect, quint16 texRepsAtEquator, double texSize, const AuMatrix4x4& gridPlane);

	virtual void EffectMove(const QPointF& vec);

	virtual void EffectMove(int vertexIndex, const QPointF& vec);

	virtual void EffectProject(SURFACE_EFFECT effect, double texSize, const AuMatrix4x4& gridPlane);

	virtual void EffectRotate(int vertexIndex, const QPointF& pivot, double angle);

	virtual void EffectSpherical(SURFACE_EFFECT effect, quint16 texRepsAtEquator, const AuMatrix4x4& gridPlane);

	virtual void EffectStretch(int vertexIndex, const QPointF& base, double ScaleX, double scaleY);

	virtual void Extents(AuPoint& min, AuPoint& max) const
	{
		m_Tri.Extents(min, max);
	}

	virtual AuPoint FaceLocation() const
	{
		return m_Tri.Center();
	}

	virtual AuPoint FaceNormal() const
	{
		return m_Tri.Normal();
	}

	virtual void FlattenVertexNormals();

	virtual bool InSite(const AuPoint& loc) const;

	virtual bool IntersectRay(const AuPoint& loc, const AuPoint& vec, AuPoint& intersect) const
	{
		return m_Tri.IntersectRay(loc, vec, intersect);
	}

	virtual bool IntersectZ(const AuPoint& loc, AuPoint& intersect) const
	{
		return m_Tri.IntersectZ(loc, intersect);
	}

	virtual bool IsPlane() const
	{
		return true;
	}

	virtual AuScFace* Join(const AuScFace& f) const;

	virtual quint64 MemoryUsage() const;

	virtual void MorphX(const AuMatrix4x4& fromM, const AuMatrix4x4& toM, double morphDistance);

	virtual void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count]) const;

	virtual void ReverseTriangleFacing()
	{
		m_Tri.ReverseFacing();
	}

	virtual void ReverseVertexNormals()
	{
		m_Tri.ReverseNormals();
	}

	virtual void ScalePositiveY(double factor)
	{
		m_Tri.ScalePositiveY(factor);
	}

	virtual void SetVertexEffectIntensity(int index, float intensity)
	{
		m_Tri.SetVertexEffectIntensity(index, intensity);
	}

	virtual void SetVertexGlow(int index, const QColor& color)
	{
		m_Tri.SetVertexGlow(index, color);
	}

	virtual void SetVertexNormal(int index, const AuPoint& n)
	{
		m_Tri.SetVertexNormal(index, n);
	}

	virtual bool Sight(const AuPoint& eyeLoc, AuPoint& sightLoc) const;

	virtual float SiteInfluence(const AuPoint& loc, AuPoint& siteLoc) const;

	virtual void Slice(const AuLine& cutLine, AuScFaceList& nufaces) const;

	virtual bool TestSlice(const AuPoint& seg1, const AuPoint& seg2) const;

	virtual void TextureCylindrical(quint8 texIndex, quint16 texRepsAtEquator, double texSize, const AuMatrix4x4& gridPlane);

	virtual void TextureMove(const QPointF& vec);

	virtual void TextureMove(int vertexIndex, const QPointF& vec);

	virtual void TextureProject(quint8 texIndex, double texSize, const AuMatrix4x4& gridPlane);

	virtual void TextureRotate(int vertexIndex, const QPointF& pivot, double angle);

	virtual void TextureSpherical(quint8 texIndex, quint16 texRepsAtEquator, const AuMatrix4x4& gridPlane);

	virtual void TextureStretch(int vertexIndex, const QPointF& base, double ScaleX, double scaleY);

	virtual void Transform(const AuMatrix4x4& t)
	{
		m_Tri.Transform(t);
	}

	virtual void Transform(const AuMatrix4x4& t, quint32 firstVertex, const QSet<quint32>& vertexIndices,
			quint32 firstEdge, const QSet<quint32>& edgeIndices, bool faceIsSelected);

	virtual void Triangles(AuScTriList& tris) const
	{
		tris.append(m_Tri);
	}

	virtual quint8 Type() const
	{
		return TypeStatic();
	}

	static quint8 TypeStatic()
	{
		return 1;
	}

	virtual AuScPoint Vertex(int index) const
	{
		return m_Tri.Vertex(index);
	}

	virtual int VertexAt(const AuPoint& loc) const
	{
		if(m_Tri.Vertex(0).FuzzyEquals(loc))
			return 0;
		if(m_Tri.Vertex(1).FuzzyEquals(loc))
			return 1;
		if(m_Tri.Vertex(2).FuzzyEquals(loc))
			return 2;
		return -1;
	}

	virtual int VertexCount() const
	{
		return 3;
	}

	virtual void VertexLocations(AuPointArray& verts) const
	{
		verts.append(m_Tri.Vertex(0));
		verts.append(m_Tri.Vertex(1));
		verts.append(m_Tri.Vertex(2));
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUMATH_DLL AuScFaceQuad : public AuScFace
{
	AuScTri								m_Tri1;		// Quad ll, ul, and ur vertices.
	AuScTri								m_Tri2;		// Quad ur, lr, and ll vertices.

public:	// Constructors
	AuScFaceQuad(const AuScPoint& ll, const AuScPoint& ul, const AuScPoint& ur, const AuScPoint& lr);

	AuScFaceQuad(const AuScFace& c, const AuScPoint& ll, const AuScPoint& ul, const AuScPoint& ur, const AuScPoint& lr);

	AuScFaceQuad(const AuScFace& c, const AuScTri& t1, const AuScTri& t2);

	inline AuScFaceQuad(const AuScFaceQuad& c) : AuScFace(c), m_Tri1(c.m_Tri1), m_Tri2(c.m_Tri2)
	{
	}

	AuScFaceQuad();

public:	// Operators
	inline const AuScFaceQuad& operator=(const AuScFaceQuad& c)
	{
		AuScFace::operator=(c);
		m_Tri1 = c.m_Tri1;
		m_Tri2 = c.m_Tri2;
		return *this;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScFace& f) const
	{
		if(!AuScFace::operator==(f))
			return false;

		try
		{
			const AuScFaceQuad& o = dynamic_cast<const AuScFaceQuad&>(f);

			if(m_Tri1 != o.m_Tri1)
				return false;
			if(m_Tri2 != o.m_Tri2)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:	// Methods
	virtual double Area() const;

	virtual AuScFace* Clone() const
	{
		return new AuScFaceQuad(*this);
	}

	// Returns true if both triangles are collapsed.
	virtual bool Collapsed() const;

	virtual bool ContainsFuzzy(const AuPoint& v) const;

	virtual bool Cross(AuPointArray& path, const AuSegment& entryEdge, AuSegment& exitEdge) const;

	virtual double Distance(const AuPoint& v) const;

	virtual void Divide(quint8 divisions, AuScFaceList& nufaces) const;

	virtual void EdgeIntersections(const AuLine& line, AuPointArray& intersections) const;

	virtual int Edges() const
	{
		return 4;
	}

	virtual void EdgeSegment(quint32 edgeIndex, AuSegment& edge) const
	{
		if(edgeIndex == 0)
		{
			edge = AuSegment(m_Tri1.V0(), m_Tri1.V1());
		}
		else if(edgeIndex == 1)
		{
			edge = AuSegment(m_Tri1.V1(), m_Tri1.V2());
		}
		else if(edgeIndex == 2)
		{
			edge = AuSegment(m_Tri2.V0(), m_Tri2.V1());
		}
		else
		{
			edge = AuSegment(m_Tri2.V1(), m_Tri2.V2());
		}
	}

	virtual void EdgeSegments(AuSegmentArray& edges) const
	{
		edges.append(AuSegment(m_Tri1.V0(), m_Tri1.V1()));
		edges.append(AuSegment(m_Tri1.V1(), m_Tri1.V2()));
		edges.append(AuSegment(m_Tri2.V0(), m_Tri2.V1()));
		edges.append(AuSegment(m_Tri2.V1(), m_Tri2.V2()));
	}

	virtual void EffectCylindrical(SURFACE_EFFECT effect, quint16 texRepsAtEquator, double texSize, const AuMatrix4x4& gridPlane);

	virtual void EffectMove(const QPointF& vec);

	virtual void EffectMove(int vertexIndex, const QPointF& vec);

	virtual void EffectProject(SURFACE_EFFECT effect, double texSize, const AuMatrix4x4& gridPlane);

	virtual void EffectRotate(int vertexIndex, const QPointF& pivot, double angle);

	virtual void EffectSpherical(SURFACE_EFFECT effect, quint16 texRepsAtEquator, const AuMatrix4x4& gridPlane);

	virtual void EffectStretch(int vertexIndex, const QPointF& base, double ScaleX, double scaleY);

	virtual void Extents(AuPoint& min, AuPoint& max) const
	{
		m_Tri1.Extents(min, max);

		AuPoint min2,max2;
		m_Tri2.Extents(min2, max2);
		min = min.Min(min2);
		max = max.Max(max2);
	}

	// This face location works when the quad is not a plane.
	virtual AuPoint FaceLocation() const
	{
		return (m_Tri1.V0() + m_Tri1.V1() + m_Tri2.V0() + m_Tri2.V1())/4;
	}

	virtual AuPoint FaceNormal() const;

	// X axis is from vertex 0 to vertex 3.
	virtual AuMatrix4x4 FacePlane() const;

	virtual void FlattenVertexNormals();

	virtual bool InSite(const AuPoint& loc) const;

	virtual bool IntersectRay(const AuPoint& loc, const AuPoint& vec, AuPoint& intersect) const
	{
		if(m_Tri1.IntersectRay(loc, vec, intersect))
			return true;
		return m_Tri2.IntersectRay(loc, vec, intersect);
	}

	virtual bool IntersectZ(const AuPoint& loc, AuPoint& intersect) const
	{
		if(m_Tri1.IntersectZ(loc, intersect))
			return true;
		return m_Tri2.IntersectZ(loc, intersect);
	}

	virtual bool IsPlane() const;

	virtual quint64 MemoryUsage() const;

	virtual void MorphX(const AuMatrix4x4& fromM, const AuMatrix4x4& toM, double morphDistance);

	virtual AuScFace* Join(const AuScFace& f) const;

	virtual void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count]) const;

	virtual void ReverseTriangleFacing()
	{
		m_Tri1.ReverseFacing();
		m_Tri2.ReverseFacing();
	}

	virtual void ReverseVertexNormals()
	{
		m_Tri1.ReverseNormals();
		m_Tri2.ReverseNormals();
	}

	virtual void ScalePositiveY(double factor)
	{
		m_Tri1.ScalePositiveY(factor);
		m_Tri2.ScalePositiveY(factor);
	}

	virtual void SetVertexEffectIntensity(int index, float intensity)
	{
		if(index == 0)
		{
			m_Tri1.SetVertexEffectIntensity(0, intensity);
			m_Tri2.SetVertexEffectIntensity(2, intensity);
		}
		else if(index == 1)
		{
			m_Tri1.SetVertexEffectIntensity(1, intensity);
		}
		else if(index == 2)
		{
			m_Tri1.SetVertexEffectIntensity(2, intensity);
			m_Tri2.SetVertexEffectIntensity(0, intensity);
		}
		else if(index == 3)
		{
			m_Tri2.SetVertexEffectIntensity(1, intensity);
		}
	}

	virtual void SetVertexGlow(int index, const QColor& color)
	{
		if(index == 0)
		{
			m_Tri1.SetVertexGlow(0, color);
			m_Tri2.SetVertexGlow(2, color);
		}
		else if(index == 1)
		{
			m_Tri1.SetVertexGlow(1, color);
		}
		else if(index == 2)
		{
			m_Tri1.SetVertexGlow(2, color);
			m_Tri2.SetVertexGlow(0, color);
		}
		else if(index == 3)
		{
			m_Tri2.SetVertexGlow(1, color);
		}
	}

	virtual void SetVertexNormal(int index, const AuPoint& n)
	{
		if(index == 0)
		{
			m_Tri1.SetVertexNormal(0, n);
			m_Tri2.SetVertexNormal(2, n);
		}
		else if(index == 1)
		{
			m_Tri1.SetVertexNormal(1, n);
		}
		else if(index == 2)
		{
			m_Tri1.SetVertexNormal(2, n);
			m_Tri2.SetVertexNormal(0, n);
		}
		else if(index == 3)
		{
			m_Tri2.SetVertexNormal(1, n);
		}
	}

	virtual bool Sight(const AuPoint& eyeLoc, AuPoint& sightLoc) const;

	virtual float SiteInfluence(const AuPoint& loc, AuPoint& siteLoc) const;

	virtual void Slice(const AuLine& cutLine, AuScFaceList& nufaces) const;

	virtual bool TestSlice(const AuPoint& seg1, const AuPoint& seg2) const;

	virtual void TextureCylindrical(quint8 texIndex, quint16 texRepsAtEquator, double texSize, const AuMatrix4x4& gridPlane);

	virtual void TextureMove(const QPointF& vec);

	virtual void TextureMove(int vertexIndex, const QPointF& vec);

	virtual void TextureProject(quint8 texIndex, double texSize, const AuMatrix4x4& gridPlane);

	virtual void TextureRotate(int vertexIndex, const QPointF& pivot, double angle);

	virtual void TextureSpherical(quint8 texIndex, quint16 texRepsAtEquator, const AuMatrix4x4& gridPlane);

	virtual void TextureStretch(int vertexIndex, const QPointF& base, double ScaleX, double scaleY);

	virtual void Transform(const AuMatrix4x4& t)
	{
		m_Tri1.Transform(t);
		m_Tri2.Transform(t);
	}

	virtual void Transform(const AuMatrix4x4& t, quint32 firstVertex, const QSet<quint32>& vertexIndices,
			quint32 firstEdge, const QSet<quint32>& edgeIndices, bool faceIsSelected);

	virtual void Triangles(AuScTriList& tris) const;

	virtual quint8 Type() const
	{
		return TypeStatic();
	}

	static quint8 TypeStatic()
	{
		return 2;
	}

	virtual AuScPoint Vertex(int index) const
	{
		if(index <= 2)
			return m_Tri1.Vertex(index);
		else
			return m_Tri2.Vertex(1);
	}

	virtual int VertexAt(const AuPoint& loc) const
	{
		if(m_Tri1.Vertex(0).FuzzyEquals(loc))
			return 0;
		if(m_Tri1.Vertex(1).FuzzyEquals(loc))
			return 1;
		if(m_Tri2.Vertex(0).FuzzyEquals(loc))
			return 2;
		if(m_Tri2.Vertex(1).FuzzyEquals(loc))
			return 3;
		return -1;
	}

	virtual int VertexCount() const
	{
		return 4;
	}

	virtual void VertexLocations(AuPointArray& verts) const
	{
		verts.append(m_Tri1.Vertex(0));
		verts.append(m_Tri1.Vertex(1));
		verts.append(m_Tri1.Vertex(2));
		verts.append(m_Tri2.Vertex(1));
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUMATH_DLL AuScFacePolygon : public AuScFace
{
	AuSegmentArray						m_Edges;
	AuScTriList							m_Tris;

public:	// Constructors
	AuScFacePolygon(const AuPointArray& tris, const QVector<bool>& edges);

	AuScFacePolygon(const AuPointArray& tris, const QVector<bool>& edges, const AuPoint& offset);

	AuScFacePolygon(const AuScFace& c, const AuPointArray& tris, const QVector<bool>& edges);

	AuScFacePolygon(const AuScFace& c, const AuPointArray& tris, const QVector<bool>& edges, const AuPoint& offset);

	AuScFacePolygon(const AuScFace& c, const AuScTriList& tris, const AuSegmentArray& edges);

	AuScFacePolygon(const AuScFaceQuad& q);

	inline AuScFacePolygon(const AuScFacePolygon& c) : AuScFace(c), m_Edges(c.m_Edges), m_Tris(c.m_Tris)
	{
	}

	AuScFacePolygon();

public:	// Operators
	inline const AuScFacePolygon& operator=(const AuScFacePolygon& c)
	{
		AuScFace::operator=(c);
		m_Edges = c.m_Edges;
		m_Tris = c.m_Tris;
		return *this;
	}

	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	virtual bool operator==(const AuScFace& f) const
	{
		if(!AuScFace::operator==(f))
			return false;

		try
		{
			const AuScFacePolygon& o = dynamic_cast<const AuScFacePolygon&>(f);

			if(m_Edges != o.m_Edges)
				return false;
			if(m_Tris != o.m_Tris)
				return false;
			return true;
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
			return false;
		}
	}

public:	// Methods
	virtual double Area() const;

	virtual AuScFace* Clone() const
	{
		return new AuScFacePolygon(*this);
	}

	virtual bool Collapsed() const;

	virtual bool ContainsFuzzy(const AuPoint& v) const;

	virtual bool Cross(AuPointArray& path, const AuSegment& entryEdge, AuSegment& exitEdge) const;

	virtual double Distance(const AuPoint& v) const;

	virtual void Divide(quint8 divisions, AuScFaceList& nufaces) const;

	virtual void EdgeIntersections(const AuLine& line, AuPointArray& intersections) const;

	virtual int Edges() const
	{
		return m_Edges.size();
	}

	virtual void EdgeSegment(quint32 edgeIndex, AuSegment& edge) const
	{
		edge = m_Edges[edgeIndex];
	}

	virtual void EdgeSegments(AuSegmentArray& edges) const
	{
		edges.append(m_Edges);
	}

	virtual void EffectCylindrical(SURFACE_EFFECT effect, quint16 texRepsAtEquator, double texSize, const AuMatrix4x4& gridPlane);

	virtual void EffectMove(const QPointF& vec);

	virtual void EffectMove(int vertexIndex, const QPointF& vec);

	virtual void EffectProject(SURFACE_EFFECT effect, double texSize, const AuMatrix4x4& gridPlane);

	virtual void EffectRotate(int vertexIndex, const QPointF& pivot, double angle);

	virtual void EffectSpherical(SURFACE_EFFECT effect, quint16 texRepsAtEquator, const AuMatrix4x4& gridPlane);

	virtual void EffectStretch(int vertexIndex, const QPointF& base, double ScaleX, double scaleY);

	virtual void Extents(AuPoint& min, AuPoint& max) const
	{
		m_Tris.Extents(min, max);
	}

	virtual AuPoint FaceLocation() const
	{
		return m_Tris.Center();
	}

	virtual AuPoint FaceNormal() const
	{
		if(m_Tris.empty())
			return AuPoint(0,0,1);

		return m_Tris.first().Normal();
	}

	virtual void FlattenVertexNormals();

	virtual bool InSite(const AuPoint& loc) const;

	virtual bool IntersectRay(const AuPoint& loc, const AuPoint& vec, AuPoint& intersect) const
	{
		return m_Tris.IntersectRay(loc, vec, intersect);
	}

	virtual bool IntersectZ(const AuPoint& loc, AuPoint& intersect) const
	{
		return m_Tris.IntersectZ(loc, intersect);
	}

	virtual bool IsPlane() const;

	virtual AuScFace* Join(const AuScFace& f) const;

	virtual quint64 MemoryUsage() const;

	virtual void MorphX(const AuMatrix4x4& fromM, const AuMatrix4x4& toM, double morphDistance);

	virtual void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count]) const;

	virtual void ReverseTriangleFacing()
	{
		m_Tris.ReverseFacing();
	}

	virtual void ReverseVertexNormals()
	{
		m_Tris.ReverseNormals();
	}

	virtual void ScalePositiveY(double factor)
	{
		m_Tris.ScalePositiveY(factor);
	}

	virtual void SetVertexEffectIntensity(int index, float intensity)
	{
		int triIndex = index / 3;
		int vertexIndex = index % 3;
		m_Tris[triIndex].SetVertexEffectIntensity(vertexIndex, intensity);
	}

	virtual void SetVertexGlow(int index, const QColor& color)
	{
		int triIndex = index / 3;
		int vertexIndex = index % 3;
		m_Tris[triIndex].SetVertexGlow(vertexIndex, color);
	}

	virtual void SetVertexNormal(int index, const AuPoint& n)
	{
		int triIndex = index / 3;
		int vertexIndex = index % 3;
		m_Tris[triIndex].SetVertexNormal(vertexIndex, n);
	}

	virtual bool Sight(const AuPoint& eyeLoc, AuPoint& sightLoc) const;

	virtual float SiteInfluence(const AuPoint& loc, AuPoint& siteLoc) const;

	virtual void Slice(const AuLine& cutLine, AuScFaceList& nufaces) const;

	virtual bool TestSlice(const AuPoint& seg1, const AuPoint& seg2) const;

	virtual void TextureCylindrical(quint8 texIndex, quint16 texRepsAtEquator, double texSize, const AuMatrix4x4& gridPlane);

	virtual void TextureMove(const QPointF& vec);

	virtual void TextureMove(int vertexIndex, const QPointF& vec);

	virtual void TextureProject(quint8 texIndex, double texSize, const AuMatrix4x4& gridPlane);

	virtual void TextureRotate(int vertexIndex, const QPointF& pivot, double angle);

	virtual void TextureSpherical(quint8 texIndex, quint16 texRepsAtEquator, const AuMatrix4x4& gridPlane);

	virtual void TextureStretch(int vertexIndex, const QPointF& base, double ScaleX, double scaleY);

	virtual void Transform(const AuMatrix4x4& t)
	{
		m_Edges.Transform(t);
		m_Tris.Transform(t);
	}

	virtual void Transform(const AuMatrix4x4& t, quint32 firstVertex, const QSet<quint32>& vertexIndices,
			quint32 firstEdge, const QSet<quint32>& edgeIndices, bool faceIsSelected);

	virtual void Triangles(AuScTriList& tris) const
	{
		tris.append(m_Tris);
	}

	virtual quint8 Type() const
	{
		return TypeStatic();
	}

	static quint8 TypeStatic()
	{
		return 3;
	}

	virtual AuScPoint Vertex(int index) const;

	virtual int VertexAt(const AuPoint& loc) const;

	virtual int VertexCount() const;

	virtual void VertexLocations(AuPointArray& verts) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUMATH_DLL AuScFaceList : public QList<AuScFace*>
{
friend class AuScFace;
public:
	enum UNWRAP_METHOD
	{
		UNWRAP_ByFace,					// Every face individually unwrapped without regard to neighboring faces.
		UNWRAP_ByFacing,				// All faces with the same facing textured the same.
		UNWRAP_Edges,
		UNWRAP_Horizontal,
		UNWRAP_Vertical,
	};

private:
	AuPoint								m_Min;
	AuPoint								m_Max;

public:	// Constructors
	AuScFaceList();
	AuScFaceList(const AuScFaceList& c);

	inline AuScFaceList(const AuScFaceList& c, const AuScSelectInfo& selInfo)
	{
		// Construct with only the selected faces.
		quint32 findex = 0;
		for(const_iterator ti = c.begin(); ti != c.end(); ti++)
		{
			if(selInfo.m_Faces.contains(findex))
				AddFace(**ti);
			findex++;
		}
	}

	inline ~AuScFaceList()
	{
		Clear();
	}

public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	const AuScFaceList& operator=(const AuScFaceList& c);

	bool operator==(const AuScFaceList& c) const;

	inline bool operator!=(const AuScFaceList& c) const
	{
		return !operator==(c);
	}

public:	// Methods
	void AddFace(const AuScFace& f);

	void AddFaceIfUnique(const AuScFace& s);

	void AddFaces(const AuScFaceList& l);

	// Applies the EFF_Hull effect to all faces that do not already have a surface effect.
	// 'shipHull' is false to apply the effect to buildings.
	// 'shipHull' is true to rotate the effect to spacecraft hulls.
	void ApplyHullFTLEffect(double texSize, bool shipHull);

	// Returns the surface area of all faces.
	double Area() const;

	// Returns true if all faces are planar, i.e. their IsPlane function returns true.
	bool CheckFacesPlanar() const;

	// Deletes all part data from the heap.
	void Clear();

	// Clips clipMe against all of my faces. Fragments that get clipped are put into 'inside' when on the inside of the face that
	// clipped them.
	void Clip(const AuScFace& clipMe, AuScFaceList& inside) const;

	// Returns true if any of these faces' triangles shares an edge with 'face'.
	bool ColinearTriangleEdge(const AuScFace& face) const;

	// Slices faces using a polyline. If the cut is known to be a closed boundary, consider using PunchFaces instead.
	// Returns true if any of my faces is cut by 'cutLine'.
	bool CutFaces(const AuPointArray& cutLine);

	inline void DeleteFaces(const QSet<quint32>& faceIds)
	{
		// Delete face pointers by index, leaving zeros behind so we don't mess up the indices.
		for(QSet<quint32>::const_iterator idi = faceIds.cbegin(); idi != faceIds.cend(); idi++)
			if(*idi < (quint32)size())
			{
				iterator fi = begin() + *idi;
				if(*fi)
				{
					delete *fi;
					*fi = 0;
				}
			}

		// Now remove the zero entries.
		iterator fi = begin();
		while(fi != end())
			if(!*fi)
				fi = erase(fi);				// Remove 0 face pointer.
			else
				fi++;						// Skip non-zero face pointer.

		RecalcExtents();
	}

	inline bool DeselectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const
	{
		bool anyChange = false;

		// Add indexes of all overlapping vertices.
		AuSegmentArray segs;
		for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
			(*ti)->EdgeSegments(segs);

		quint32 eindex = 0;
		for(AuSegmentArray::const_iterator si = segs.cbegin(); si != segs.cend(); si++)
		{
			if(selInfo.m_Edges.contains(eindex) && (*si).IsSegment(edgeSeg))
			{
				selInfo.m_Edges.remove(eindex);
				anyChange = true;
			}
			eindex++;
		}

		return anyChange;
	}

	inline bool DeselectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
	{
		bool anyChange = false;

		// Remove indexes of all overlapping vertices.
		quint32 findex = 0;
		for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
		{
			AuPoint v = (*ti)->FaceLocation();
			if(v.FuzzyEquals(atLoc) && selInfo.m_Faces.contains(findex))
			{
				selInfo.m_Faces.remove(findex);
				anyChange = true;
			}
			findex++;
		}

		return anyChange;
	}

	inline bool DeselectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
	{
		bool anyChange = false;

		// Remove indexes of all overlapping vertices.
		quint32 vindex = 0;
		for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
		{
			int verts = (*ti)->VertexCount();
			for(int vi=0; vi<verts; vi++)
			{
				AuPoint v = (*ti)->Vertex(vi);
				if(v.FuzzyEquals(atLoc) && selInfo.m_Vertices.contains(vindex))
				{
					selInfo.m_Vertices.remove(vindex);
					anyChange = true;
				}
				vindex++;
			}
		}

		return anyChange;
	}

	bool DeselectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	bool DeselectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	void DivideFaces(const AuScSelectInfo& selInfo, quint8 divisions);

	inline double EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuPoint* snapLoc) const
	{
		double bseg = d_SearchTolerance;	// Best edge distance.
		bool foundMe = false;				// True if we find a segment within the search tolerance.

		AuPoint edgeLoc;
		quint32 eindex = 0;
		quint32 faceEdgeIndex;
		for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
		{
			double dseg = (*ti)->ViewEdgeDistance(mvp, pixel, faceEdgeIndex, edgeLoc);
			if(dseg < bseg)
			{
				// Closest edge so far.
				bseg = dseg;
				foundMe = true;
				if(snapLoc)
					*snapLoc = edgeLoc;
			}
			eindex += (*ti)->Edges();
		}

		if(!foundMe)
			return d_SearchTolerance;

		return bseg;
	}

	inline double EdgeDistance(const AuMatrix4x4& mvp, const QPointF& pixel, AuSegment& edgeSeg) const
	{
		double bseg = d_SearchTolerance;	// Best edge distance.
		bool foundMe = false;				// True if we find a segment within the search tolerance.

		AuPoint edgeLoc;
		quint32 eindex = 0;
		quint32 faceEdgeIndex;
		for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
		{
			double dseg = (*ti)->ViewEdgeDistance(mvp, pixel, faceEdgeIndex, edgeLoc);
			if(dseg < bseg)
			{
				// Closest edge so far.
				bseg = dseg;
				foundMe = true;
				(*ti)->EdgeSegment(faceEdgeIndex, edgeSeg);
			}
			eindex += (*ti)->Edges();
		}

		if(!foundMe)
			return d_SearchTolerance;

		return bseg;
	}

	inline void EdgeIds(QSet<quint32>& edgeIds) const
	{
		// Each edge of each face has an index.
		quint32 edgeIndex = 0;
		for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
			for(int v=0; v<(*ti)->Edges(); v++)
				edgeIds.insert(edgeIndex++);
	}

	void EdgeSegments(AuSegmentArray& edges) const;

	inline void EdgeSegments(AuPointArray& edges) const
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

				edges.append((*si).m_fLoc1);
				edges.append((*si).m_fLoc2);
			}
		}
	}

	// Returns the SURFACE_EFFECT of the selected faces.
	// Returns -1 if the effect differs between selected faces.
	// Returns -2 if no faces are in the selection.
	qint8 Effect(const AuScSelectInfo& selInfo) const;

	void EffectCylindrical(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, quint16 texRepsAtEquator, double texSize,
			const AuMatrix4x4& gridPlane);

	// Configures effect layer intensity on 'selInfo' vertices of selected faces.
	void EffectIntensityVertices(const AuScSelectInfo& selInfo, float intensity);

	void EffectMove(const AuScSelectInfo& selInfo, const QPointF& vec);

	void EffectProject(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize, const AuMatrix4x4& gridPlane);

	void EffectRotate(const AuScSelectInfo& selInfo, const QPointF& pivot, double angle);

	void EffectSpherical(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
			const AuMatrix4x4& gridPlane);

	void EffectStretch(const AuScSelectInfo& selInfo, const QPointF& base, double ScaleX, double scaleY);

	void EffectUnwrap(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize, UNWRAP_METHOD unwrap, float angleTol,
			bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane);

	inline void Extents(AuPoint& min, AuPoint& max) const
	{
		max = m_Max;
		min = m_Min;
	}

	double FaceDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& faceIndex, AuPoint& gripLoc) const;

	inline void FaceIds(QSet<quint32>& faceIds) const
	{
		// Each face has an index.
		quint32 faceIndex = 0;
		for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
			faceIds.insert(faceIndex++);
	}

	void FaceLocations(AuPointArray& faces) const;

	inline void Faces(AuScFaceList& faces, const AuScSelectInfo& selInfo) const
	{
		quint32 findex=0;
		for(const_iterator fi = cbegin(); fi != cend(); fi++)
		{
			if(selInfo.m_Faces.contains(findex))
				faces.AddFace(**fi);
			findex++;
		}
	}

	inline void FaceSpriteLocs(const QSet<quint32>& selFaces, int selSize, QList<AuPointAlpha>& selLocs, int unselSize,
			QList<AuPointAlpha>& unselLocs) const
	{
		int faceIndex = 0;
		QMap<AuPoint, bool> alreadySel;
		QMap<AuPoint, bool> alreadyUnsel;
		for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
		{
			AuPoint v((*ti)->FaceLocation());
			if(selFaces.contains(faceIndex))
			{
				if(!alreadySel.contains(v))
				{
					alreadySel.insert(v, true);
					selLocs.append(AuPointAlpha(v, selSize));
				}
			}
			else
			{
				if(!alreadyUnsel.contains(v))
				{
					alreadyUnsel.insert(v, true);
					unselLocs.append(AuPointAlpha(v, unselSize));
				}
			}
			faceIndex++;
		}
	}

	// This function distorts this rectangular fence panel to conform to end posts that align with the gravity up
	// vector at their locations. The height(z) and thickness(y) of this fence panel does not change.
	// This fence panel was drawn in the XZ plane, starting at X=0, extending in the positive direction. The X extent of this list
	// of faces is the length of the panel.
	// 'post1' and 'post2' are the end locations of the fence panel on the terrain. 'vUp1' and 'vUp2' are the gravity up directions
	// at their respective posts.
	// 'length' is the length of the untransformed fence panel.
	void FenceConform(const AuPoint& post1, const AuPoint& vUp1, const AuPoint& post2, const AuPoint& vUp2, double length);

	inline void FenceConform(const AuSegment& fenceSegment, const AuPoint& vUp1, const AuPoint& vUp2, double length)
	{
		FenceConform(fenceSegment.m_fLoc1, vUp1, fenceSegment.m_fLoc2, vUp2, length);
	}

	const_iterator FindFace(const AuScFace& f) const;

	const_iterator FindFaceInSamePlace(const AuScFace& f) const;

	// Finds the nearest face from 'loc' in the direction of 'dir', which is expected to be a unit vector.
	// In usage, TestInside has already been called to determine if 'loc' is inside this shell.
	// Returns true if a face is found and 'faceLoc' and 'faceN' are set based on the face that is hit.
	// Returns false if no face is found and 'faceLoc' and 'faceN' are unchanged.
	bool FindNearestFace(const AuPoint& loc, const AuPoint& dir, AuPoint& faceLoc, AuPoint& faceN) const;

	// Returns the first face quad, or 0.
	const AuScFaceQuad* FindQuad() const;

	void FlattenVertexNormals();

	void FlattenVertexNormals(const AuScSelectInfo& selInfo);

	bool Fracture(const AuScSelectInfo& selInfo);

	bool Fuse(const AuScSelectInfo& selInfo);

	void GlowVertices(const AuScSelectInfo& selInfo, const QColor& glow);

	// Used by a site mesh to find the 2D intersection of 'loc' with this mesh. Returns true if intersection found.
	bool InSite(const AuPoint& loc) const;

	// Calculates the intersection of a ray from 'loc' with this face list.
	// Returns true if an intersection is found.
	// 'intersect' is set to the closest result to 'loc' if an intersection is found.
	bool IntersectRay(const AuPoint& loc, const AuPoint& vec, AuPoint& intersect) const;

	// Calculates the intersection of the Z axis through 'loc' with this face list.
	// Returns true if an intersection is found.
	// 'intersect' is set to the closest result to 'loc' if an intersection is found.
	bool IntersectZ(const AuPoint& loc, AuPoint& intersect) const;

	inline bool InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, bool findVertices, bool findEdges,
			bool findFaces) const
	{
		// Test all projected vertices to see if they are inside the pixel rectangle.
		for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
			if((*ti)->InViewRectangle(mvp, pixelRect, findVertices, findEdges, findFaces))
				return true;
		return false;
	}

	bool InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect, AuScSelectInfo& selInfo, bool findVertices,
			bool findEdges, bool findFaces) const;

	void JigCut(AuScFaceList& cut1Faces, AuScFaceList& cut2Faces, AuScFaceList& lining) const;

	// Builds quads and polygons from coplanar faces that share an edge.
	void JoinFaces();

	// 'extVec' is relative to the gridPlane.
	// Does not clear this face list before proceeding.
	void MakeExtrusion(const AuPointArray& bnd, const AuPoint& extVec, const AuMatrix4x4& gridPlane, unsigned int steps,
			bool closeEnds);

	// Makes a lat/long type sphere.
	// Does not clear this face list before proceeding.
	void MakeSphereLongLat(const AuPoint& center, double radius, quint16 lonSteps, quint16 latSteps, const AuMatrix4x4& gridPlane);

	// Makes an evenly tessellated sphere.
	// Does not clear this face list before proceeding.
	void MakeSphereUniform(const AuPoint& center, double radius, quint16 equatorSteps, quint16 textureRepetitions,
			const AuMatrix4x4& gridPlane);

	// 'profile' is drawn in the XY gridPlane. It spins around the X axis. Drawn from -X to +X, the triangles face outward.
	// Does not clear this face list before proceeding.
	void MakeSpindle(const AuPointArray& profile, quint16 steps, const AuMatrix4x4& gridPlane);
	void MakeSpindle(const AuPointArray& profile, quint16 steps, const AuMatrix4x4& gridPlane, double sweepAngle);

	// Does not clear this face list before proceeding.
	void MakeTube(const AuPointArray& path, double diameter, int sides, bool smoothSides);

	// 'shape' is expected to be drawn in 2D. The origin is the guide along which 'shape' follows 'path'.
	// Does not clear this face list before proceeding.
	void MakeTube(const AuPointArray& path, const AuPointArray& shape, bool smoothSides);

	inline const AuPoint& Max() const
	{
		return m_Max;
	}

	quint64 MemoryUsage() const;

	inline const AuPoint& Min() const
	{
		return m_Min;
	}

	// Transforms my vertices based on morph from 'fromM' to 'toM' along the X axis, 1=full morph to 'toM' at 'morphDistance'.
	void MorphX(const AuMatrix4x4& fromM, const AuMatrix4x4& toM, double morphDistance);

	void PaintFaces(const AuScSelectInfo& selInfo, const QColor& paint);

	void PaintFaces(const AuScSelectInfo& selInfo, AuScFace::COLOR_THEME theme, quint8 opacity);

	// Gets the current paint in use, if any. Only gets the AuScFace::Paint() color. Does not get the AuScFace::Opacity().
	void PaintInUse(QList<QColor>& paint) const;

	// 'shine' is the specular hardness. It is 0 to disable specular highlights, >=1 for effect.
	// 'reflectivity' is the specular intensity. It is 0 to 1 to linearly modulate specular highlights. 
	void PolishFaces(const AuScSelectInfo& selInfo, float shine, float reflectivity);

	// Slices faces using a closed boundary. If the array is not a closed boundary, use CutFaces instead.
	// Returns true if any of my faces is cut by 'cutBnd'.
	bool PunchFaces(const AuPointArray& cutBnd);

	// Retrieves triangles ready for rendering.
	void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris,
			const QColor themeColors[AuScFace::THEME_Count]) const;

	// Creates an array of white theme colors.
	void RenderTriangles(AuScTriRenderList& opaqueTris, AuScTriRenderList& transparentTris) const;

	void ReverseTriangleFacing();

	inline void ReverseTriangleFacing(const AuScSelectInfo& selInfo)
	{
		quint32 findex = 0;
		for(iterator fi = begin(); fi != end(); fi++)
		{
			if(selInfo.m_Faces.contains(findex))
				(*fi)->ReverseTriangleFacing();
			findex++;
		}
	}

	inline void ReverseVertexNormals()
	{
		for(iterator fi = begin(); fi != end(); fi++)
		{
			int vcount = (*fi)->VertexCount();
			for(int vi=0; vi<vcount; vi++)
			{
				AuPoint n = (*fi)->Vertex(vi).m_N;
				(*fi)->SetVertexNormal(vi, -n);
			}
		}
	}

	inline void ReverseVertexNormals(const AuScSelectInfo& selInfo)
	{
		int vindex = 0;
		for(iterator fi = begin(); fi != end(); fi++)
		{
			int vcount = (*fi)->VertexCount();
			for(int vi=0; vi<vcount; vi++)
			{
				if(selInfo.m_Vertices.contains(vindex))
				{
					AuPoint n = (*fi)->Vertex(vi).m_N;
					(*fi)->SetVertexNormal(vi, -n);
				}
				vindex++;
			}
		}
	}

	// Multiplies the 'y' ordinate of all coordinates by 'factor' whose 'y' is already greater than zero.
	// This is used when creating a space helmet, to stretch the front (+Y) part to fit facial features.
	void ScalePositiveY(double factor);

	// Adds faces to 'selInfo' of faces that share an edge with a selected face and is coplanar within 'angleTol'.
	// This is done iteratively until no more faces are added to the selection, in an attempt to select all coplanar faces that
	// share an edge.
	bool SelectContiguous(double angleTol, AuScSelectInfo& selInfo) const;

	inline bool SelectEdgesAt(const AuSegment& edgeSeg, AuScSelectInfo& selInfo) const
	{
		bool anyChange = false;

		// Add indexes of all overlapping vertices.
		AuSegmentArray segs;
		for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
			(*ti)->EdgeSegments(segs);

		quint32 eindex = 0;
		for(AuSegmentArray::const_iterator si = segs.cbegin(); si != segs.cend(); si++)
		{
			if(!selInfo.m_Edges.contains(eindex) && (*si).IsSegment(edgeSeg))
			{
				selInfo.m_Edges.insert(eindex);
				anyChange = true;
			}
			eindex++;
		}

		return anyChange;
	}

	inline bool SelectFaceAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
	{
		bool anyChange = false;

		// Add indexes of one overlapping faces.
		quint32 findex = 0;
		for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
		{
			AuPoint v = (*ti)->FaceLocation();
			if(v.FuzzyEquals(atLoc) && !selInfo.m_Faces.contains(findex))
			{
				selInfo.m_Faces.insert(findex);
				anyChange = true;
				break;
			}
			findex++;
		}

		return anyChange;
	}

	inline bool SelectFacesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
	{
		bool anyChange = false;

		// Add indexes of all overlapping faces.
		quint32 findex = 0;
		for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
		{
			AuPoint v = (*ti)->FaceLocation();
			if(v.FuzzyEquals(atLoc) && !selInfo.m_Faces.contains(findex))
			{
				selInfo.m_Faces.insert(findex);
				anyChange = true;
			}
			findex++;
		}

		return anyChange;
	}

	inline bool SelectVerticesAt(const AuPoint& atLoc, AuScSelectInfo& selInfo) const
	{
		bool anyChange = false;

		// Add indexes of all overlapping vertices.
		quint32 vindex = 0;
		for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
		{
			int verts = (*ti)->VertexCount();
			for(int vi=0; vi<verts; vi++)
			{
				AuPoint v = (*ti)->Vertex(vi);
				if(v.FuzzyEquals(atLoc) && !selInfo.m_Vertices.contains(vindex))
				{
					selInfo.m_Vertices.insert(vindex);
					anyChange = true;
				}
				vindex++;
			}
		}

		return anyChange;
	}

	bool SelectVerticesAtEffectCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	bool SelectVerticesAtTextureCoord(const QPointF& texLoc, AuScSelectInfo& selInfo) const;

	void SetEffect(SURFACE_EFFECT effect);

	void SetEffect(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect);

	void SetOpacity(float opacity);

	void SetOpacity(const AuScSelectInfo& selInfo, float opacity);

	void SetSelfIllumination(float selfIllum);

	void SetSelfIllumination(const AuScSelectInfo& selInfo, float selfIllum);

	void SetTextureIndex(quint8 texIndex);

	void SetTextureIndex(const AuScSelectInfo& selInfo, quint8 texIndex);

	// Look from 'eyeLoc' to 'sightLoc'
	// Returns true if the segment intersects any of my faces.
	// 'sightLoc' is adjusted to the hit location on true return.
	bool Sight(const AuPoint& eyeLoc, AuPoint& sightLoc) const;

	// Look from 'eyeLoc' to 'sightLoc'
	// Returns true if the segment intersects any of my faces.
	// 'sightLoc' is adjusted to the hit location on true return.
	// 'sightedFaceNormal' is set on true return.
	bool Sight(const AuPoint& eyeLoc, AuPoint& sightLoc, AuPoint& sightedFaceNormal) const;

	// Used by a site mesh to find the 2D intersection of 'loc' with this mesh. Returns the 3D face location in 'siteLoc'.
	// Interpolates the red glow between vertices to return a number from 1=no glow to 0=max red glow.
	float SiteInfluence(const AuPoint& loc, AuPoint& siteLoc) const;

	// Slices my faces where they are intersected by face 'cutter', creating more faces in this list.
	void Slice(const AuScFace& cutter);

	void SmoothVertices();

	void SmoothVertices(const AuScSelectInfo& selInfo);

	// returns 0 on success.
	int STLReadASCII(const QString& fileName);

	// returns 0 on success.
	int STLReadBinary(const QString& fileName);

	// returns 0 on success.
	int STLWriteASCII(const QString& fileName) const;

	// returns 0 on success.
	int STLWriteBinary(const QString& fileName) const;

	// Every one of my faces that is inside the 'shell' is moved to the 'subtracted' list.
	void Subtract(const AuScFaceList& shell, AuScFaceList& subtracted);

	// Tests a location 'v' to see if it is inside the 3D shell created by these faces. Assumes these faces form a closed 3D shell.
	// Returns: 1=outside, 0=on the surface, -1=inside
	int TestInside(const AuPoint& v) const;

	// Tests a face 'f' to see if it is inside the 3D shell created by these faces. Assumes these faces form a closed 3D shell.
	// Returns true if all vertices of 'f' are inside or on the surface.
	bool TestInside(const AuScFace& f) const;

	void TextureCylindrical(const AuScSelectInfo& selInfo, quint8 texIndex, quint16 texRepsAtEquator, double texSize,
			const AuMatrix4x4& gridPlane);

	// Returns the texture index of the selected faces.
	// Returns -1 if the texture index differs between selected faces.
	// Returns -2 if no faces are in the selection.
	qint8 TextureIndex(const AuScSelectInfo& selInfo) const;

	void TextureMove(const AuScSelectInfo& selInfo, const QPointF& vec);

	void TextureProject(const AuScSelectInfo& selInfo, quint8 texIndex, double texSize, const AuMatrix4x4& gridPlane);

	void TextureRotate(const AuScSelectInfo& selInfo, const QPointF& pivot, double angle);

	void TextureSpherical(const AuScSelectInfo& selInfo, quint8 texIndex, quint16 texRepsAtEquator, const AuMatrix4x4& gridPlane);

	void TextureStretch(const AuScSelectInfo& selInfo, const QPointF& base, double ScaleX, double scaleY);

	void TextureUnwrap(const AuScSelectInfo& selInfo, quint8 texIndex, double texSize, UNWRAP_METHOD unwrap, float angleTol,
			bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane);

	inline void Transform(const AuMatrix4x4& t)
	{
		for(iterator ti = begin(); ti != end(); ti++)
			(*ti)->Transform(t);
		RecalcExtents();
	}

	void Transform(const AuMatrix4x4& t, const QSet<quint32>& vertexIndices, const QSet<quint32>& edgeIndices,
			const QSet<quint32>& faceIndices);

	inline void TransformCopy(const AuMatrix4x4& t, const AuScSelectInfo& selInfo)
	{
		AuScFaceList nuTris;

		quint32 vindex = 0;
		quint32 eindex = 0;
		quint32 findex = 0;
		for(iterator ti = begin(); ti != end(); ti++)
		{
			bool faceSelected = false;
			if(selInfo.m_Faces.contains(findex))
				faceSelected = true;

			if(!faceSelected)
			{
				for(int v=0; v<(*ti)->VertexCount(); v++)
				{
					if(selInfo.m_Vertices.contains(vindex + v))
					{
						faceSelected = true;
						break;
					}
				}
			}

			if(!faceSelected)
			{
				for(int v=0; v<(*ti)->Edges(); v++)
				{
					if(selInfo.m_Edges.contains(eindex + v))
					{
						faceSelected = true;
						break;
					}
				}
			}

			if(faceSelected)
			{
				// Face is affected by the transform.
				// Duplicate the face and transform it.
				AuScFace* c = (*ti)->Clone();
				c->Transform(t, vindex, selInfo.m_Vertices, eindex, selInfo.m_Edges, faceSelected);
				nuTris.append(c);
			}

			vindex += (*ti)->VertexCount();
			eindex += (*ti)->Edges();
			findex++;
		}

		append(nuTris);
		nuTris.clear();						// Don't let it's destructor delete the faces because we took them.
		RecalcExtents();
	}

	void Triangles(AuScTriList& tris) const;

	double VertexDistance(const AuMatrix4x4& mvp, const QPointF& pixel, quint32& vertexIndex, AuPoint* snapLoc) const;

	inline void VertexIds(QSet<quint32>& vertexIds) const
	{
		// Each vertex of each face has an index.
		quint32 vertexIndex = 0;
		for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
			for(int v=0; v<(*ti)->VertexCount(); v++)
				vertexIds.insert(vertexIndex++);
	}

	AuScPoint VertexLocation(quint32 vertexIndex) const;

	// The 'verts' returned by this function can be safely indexed using a vertex id/index.
	void VertexLocations(AuPointArray& verts) const;

	inline void VertexSpriteLocs(const QSet<quint32>& selVerts, int selSize, QList<AuPointAlpha>& selLocs, int unselSize,
			QList<AuPointAlpha>& unselLocs) const
	{
		int vertexIndex = 0;
		QMap<AuPoint, bool> alreadySel;
		QMap<AuPoint, bool> alreadyUnsel;
		for(const_iterator ti = constBegin(); ti != constEnd(); ti++)
		{
			int verts = (*ti)->VertexCount();
			for(int vi=0; vi<verts; vi++)
			{
				AuPoint v((*ti)->Vertex(vi));
				if(selVerts.contains(vertexIndex))
				{
					if(!alreadySel.contains(v))
					{
						alreadySel.insert(v, true);
						selLocs.append(AuPointAlpha(v, selSize));
					}
				}
				else
				{
					if(!alreadyUnsel.contains(v))
					{
						alreadyUnsel.insert(v, true);
						unselLocs.append(AuPointAlpha(v, unselSize));
					}
				}
				vertexIndex++;
			}
		}
	}

protected:
	void RecalcExtents();

	void EffectUnwrapByFace(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize, bool gridAlign, int alignAxis,
			const AuMatrix4x4& gridPlane); 

	void EffectUnwrapByFacing(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize, float angleTol,
			bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane);

	void EffectUnwrapEdges(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize, float angleTol, bool gridAlign,
			int alignAxis, const AuMatrix4x4& gridPlane);

	void EffectUnwrapHorizontal(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize, float angleTol,
			bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane);

	void EffectUnwrapVertical(const AuScSelectInfo& selInfo, SURFACE_EFFECT effect, double texSize, float angleTol,
			bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane);

	void TextureUnwrapByFace(const AuScSelectInfo& selInfo, quint8 texIndex, double texSize, bool gridAlign, int alignAxis,
			const AuMatrix4x4& gridPlane); 

	void TextureUnwrapByFacing(const AuScSelectInfo& selInfo, quint8 texIndex, double texSize, float angleTol, bool gridAlign,
			int alignAxis, const AuMatrix4x4& gridPlane);

	void TextureUnwrapEdges(const AuScSelectInfo& selInfo, quint8 texIndex, double texSize, float angleTol, bool gridAlign,
			int alignAxis, const AuMatrix4x4& gridPlane);

	void TextureUnwrapHorizontal(const AuScSelectInfo& selInfo, quint8 texIndex, double texSize, float angleTol, bool gridAlign,
			int alignAxis, const AuMatrix4x4& gridPlane);

	void TextureUnwrapVertical(const AuScSelectInfo& selInfo, quint8 texIndex, double texSize, float angleTol, bool gridAlign,
			int alignAxis, const AuMatrix4x4& gridPlane);

public:
	inline bool empty() const
	{
		return QList<AuScFace*>::empty();
	}

	inline int size() const
	{
		return QList<AuScFace*>::size();
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuScFaceList& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScFaceList& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuScFace_H
