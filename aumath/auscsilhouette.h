//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscsilhouette.h,v 1.9 2022/02/19 18:38:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScSilhouette_H
#define AuScSilhouette_H

#include "aumath.h"
#include "aupoint.h"
#include <QImage>

using namespace std;

class AuScFaceList;
class AuScTriList;

class AUMATH_DLL AuScSilhouette3d
{
	// Silhouette outlines are used by IsPathBlocked test.
	float								m_Margin;
	double								m_XYMax;				// Silhouette extrusion max +Z.
	double								m_XYMin;				// Silhouette extrusion min -Z.
	QImage								m_XYSilhouette;			// Silhouette image in XY plane. 1 pixel = .5m.
	double								m_XZMax;				// Silhouette extrusion max +Y.
	double								m_XZMin;				// Silhouette extrusion min -Y.
	QImage								m_XZSilhouette;			// Silhouette image in XZ plane. 1 pixel = .5m.
	double								m_YZMax;				// Silhouette extrusion max +X.
	double								m_YZMin;				// Silhouette extrusion min -X.
	QImage								m_YZSilhouette;			// Silhouette image in YZ plane. 1 pixel = .5m.

public:	// Constructors
	inline AuScSilhouette3d()
	{
		m_Margin = 0;
		m_XYMax = 0;
		m_XYMin = 0;
		m_XZMax = 0;
		m_XZMin = 0;
		m_YZMax = 0;
		m_YZMin = 0;
	}

	inline AuScSilhouette3d(const AuScFaceList& f, float margin)
	{
		Build(f, margin);
	}

	inline AuScSilhouette3d(const AuScSilhouette3d& c) :
		m_XYSilhouette(c.m_XYSilhouette), m_XZSilhouette(c.m_XZSilhouette), m_YZSilhouette(c.m_YZSilhouette)
	{
		m_Margin = c.m_Margin;
		m_XYMax = c.m_XYMax;
		m_XYMin = c.m_XYMin;
		m_XZMax = c.m_XZMax;
		m_XZMin = c.m_XZMin;
		m_YZMax = c.m_YZMax;
		m_YZMin = c.m_YZMin;
	}

public:	// Operators
	inline const AuScSilhouette3d& operator=(const AuScSilhouette3d& c)
	{
		m_Margin = c.m_Margin;
		m_XYMax = c.m_XYMax;
		m_XYMin = c.m_XYMin;
		m_XYSilhouette = c.m_XYSilhouette;
		m_XZMax = c.m_XZMax;
		m_XZMin = c.m_XZMin;
		m_XZSilhouette = c.m_XZSilhouette;
		m_YZMax = c.m_YZMax;
		m_YZMin = c.m_YZMin;
		m_YZSilhouette = c.m_YZSilhouette;
		return *this;
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	void ReadOld(QDataStream& in);

public:	// Methods
	// Resets this silhouette based on 'faces'.
	void Build(const AuScFaceList& faces, float margin);

	inline AuPoint Center() const
	{
		return AuPoint((m_YZMin + m_YZMax)/2, (m_XZMin + m_XZMax)/2, (m_XYMin + m_XYMax)/2);
	}

	// Returns true if 'loc' is inside this 3d silhouette.
	bool IsInside(const AuPoint& loc) const;

	// Returns true if a face is hit when moving from 'fromLoc' to 'toLoc'.
	// If hit, 'hitNormal' is set to the vector pointing out from the surface that is hit, if any.
	// Returns false if no surface is hit and 'hitNormal' is unchanged.
	bool IsPathBlocked(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal) const;

	bool IsValid() const;

	inline float Margin() const
	{
		return m_Margin;
	}

	quint64 MemoryUsage() const;

	// Returns the volume in cubic units.
	double Volume() const;

private:
	static double Area(const QImage& silhouette);

	static bool IsPointInSilhouette(const AuPoint& loc, const QImage& img, const AuMatrix4x4& model);

	static void PaintSilhouette(QImage& img, const AuScTriList& tris, const AuMatrix4x4& model, float width, float height,
			float margin);
};

inline QDataStream& operator<<(QDataStream& out, const AuScSilhouette3d& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScSilhouette3d& o)
{
	o << in;
	return in;
}

#endif // AuScSilhouette_H
