//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscsilhouette.cpp,v 1.21 2022/02/19 18:38:16 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscsilhouette.h"
//
#include "auscface.h"
#include "ausegment.h"
#include <QPainter>

#define d_FeetPerMeter					(3.28083989501312)
#define d_SilhouetteImageSize			(3201)			// Must fit -1600 to 1600, which is 3201 pixels, including the 0 column/row.

bool AuScSilhouette3d::IsValid() const
{
	return !m_XYSilhouette.isNull() && !m_XZSilhouette.isNull() && !m_YZSilhouette.isNull();
}

void AuScSilhouette3d::Build(const AuScFaceList& faces, float margin)
{
	m_Margin = margin;

	// Get all the triangles from the faces.
	AuScTriList tris;
	faces.Triangles(tris);
	if(tris.empty())
	{
		m_XYSilhouette = QImage();
		m_XYMax = 0;
		m_XYMin = 0;
		m_XZSilhouette = QImage();
		m_XZMax = 0;
		m_XZMin = 0;
		m_YZSilhouette = QImage();
		m_YZMax = 0;
		m_YZMin = 0;
		return;
	}

	// Set depth extents for each silhouette plane.
	AuPoint min,max;
	tris.Extents(min, max);
	m_XYMax = max.z + margin;
	m_XYMin = min.z - margin;
	m_XZMax = max.y + margin;
	m_XZMin = min.y - margin;
	m_YZMax = max.x + margin;
	m_YZMin = min.x - margin;

	float xlen = max.x - min.x + margin*2;
	float ylen = max.y - min.y + margin*2;
	float zlen = max.z - min.z + margin*2;

	AuPoint cen(-Center());

	AuMatrix4x4 tXY;
	tXY.Translate(cen);
	PaintSilhouette(m_XYSilhouette, tris, tXY, xlen, ylen, margin);
	//m_XYSilhouette.save("filledXY.png");

	AuMatrix4x4 tXZ;
	tXZ.RotateX(-PI/2);
	tXZ.Translate(cen);
	PaintSilhouette(m_XZSilhouette, tris, tXZ, xlen, zlen, margin);
	//m_XZSilhouette.save("filledXZ.png");

	AuMatrix4x4 tYZ;
	tYZ.RotateZ(-PI/2);
	tYZ.RotateY(-PI/2);
	tYZ.Translate(cen);
	PaintSilhouette(m_YZSilhouette, tris, tYZ, ylen, zlen, margin);
	//m_YZSilhouette.save("filledYZ.png");
}

bool AuScSilhouette3d::IsInside(const AuPoint& loc) const
{
	if(!IsValid())
		return false;

	// Test box extents of silhouettes.
	if(loc.z <= m_XYMin || loc.z >= m_XYMax)
		return false;					// Loc does not fall within the extrusion extents of the XY plane silhouette.
	if(loc.y <= m_XZMin || loc.y >= m_XZMax)
		return false;					// Loc does not fall within the extrusion extents of the XZ plane silhouette.
	if(loc.x <= m_YZMin || loc.x >= m_YZMax)
		return false;					// Loc does not fall within the extrusion extents of the YZ plane silhouette.

	// Test loc against three 2D silhouette images, one from each planar direction.
	// If all three 2D silhouettes contain the point, then it is inside this 3D silhouette.
	AuMatrix4x4 tXY;
	tXY.Translate(-Center());
	if(!IsPointInSilhouette(loc, m_XYSilhouette, tXY))
		return false;

	AuMatrix4x4 tXZ;
	tXZ.RotateX(-PI/2);
	tXZ.Translate(-Center());
	if(!IsPointInSilhouette(loc, m_XZSilhouette, tXZ))
		return false;

	AuMatrix4x4 tYZ;
	tYZ.RotateZ(-PI/2);
	tYZ.RotateY(-PI/2);
	tYZ.Translate(-Center());
	if(!IsPointInSilhouette(loc, m_YZSilhouette, tYZ))
		return false;

	// Location is inside all three 2D silhouettes, when viewed from their respective sides.
	return true;
}

bool AuScSilhouette3d::IsPathBlocked(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitLoc, AuPoint& hitNormal) const
{
	if(!fromLoc.FuzzyEquals(toLoc))
	{
		// Check closest point on movement segment if it passes my center, to keep from phasing through the entire design.
		AuPoint po = Center().PointOnSegment(fromLoc, toLoc);
		if(!po.FuzzyEquals(toLoc) && !po.FuzzyEquals(fromLoc) && IsInside(po))
		{
			// Closest point on the segment is between its end points and closest point is inside this silhouette.
			//hitNormal = po - Center();
			//hitNormal.Unit();
			hitLoc = fromLoc;
			hitNormal = AuPoint(0,0);	// No bounce.
			return true;
		}
	}

	// Test the destination loc to see if it is blocked.
	if(!IsInside(toLoc))
		return false;

	// Try to let them escape. Check ahead in the movement direction, a distance that is larger than the margin.
	AuPoint v(toLoc - fromLoc);
	//v.Unit(m_Margin + d_FeetPerMeter);
	v.Unit(m_Margin + d_FeetPerMeter/10);
	AuPoint toNext = toLoc + v;
	if(!IsInside(toNext))
		return false;

	// toLoc is inside this silhouette.
	hitLoc = fromLoc;
	//hitNormal = toLoc - Center();
	//hitNormal.Unit();
	hitNormal = AuPoint(0,0);	// No bounce.
	return true;
}

double AuScSilhouette3d::Volume() const
{
	// Calculate box dimensions.
	double x = m_YZMax - m_YZMin;
	if(x == 0)
		return 0;
	double y = m_XZMax - m_XZMin;
	if(y == 0)
		return 0;
	double z = m_XYMax - m_XYMin;
	if(z == 0)
		return 0;

	// Calculate area of each outline.
	double areaXY = Area(m_XYSilhouette);
	if(areaXY == 0)
		return 0;
	double areaXZ = Area(m_XZSilhouette);
	if(areaXZ == 0)
		return 0;
	double areaYZ = Area(m_YZSilhouette);
	if(areaYZ == 0)
		return 0;

	// Calculate volume of the XY extrusion.
	double vol = areaXY * z;

	// Reduce volume by fractional area of each of the other silhouettes.
	double fracXZ = areaXZ / (x * z);
	vol *= fracXZ;

	double fracYZ = areaYZ / (y * z);
	vol *= fracYZ;

	return vol;
}

//static
double AuScSilhouette3d::Area(const QImage& silhouette)
{
	int whitepixels = 0;
	for(int iy=0; iy<silhouette.height(); iy++)
		for(int ix=0; ix<silhouette.width(); ix++)
			if(silhouette.pixel(ix, iy) == qRgb(255,255,255))
				whitepixels++;
	// Each pixel is 1/2 meter on a side.
	return (double)whitepixels * (d_FeetPerMeter/2) * (d_FeetPerMeter/2);
}

//static
void AuScSilhouette3d::PaintSilhouette(QImage& img, const AuScTriList& tris, const AuMatrix4x4& model, float width, float height,
		float margin)
{
	// Largest ship dimension is 1600m. We will do this in 1/2 meter increments.
	// Width and height args already include the margin.
	double pxwid = ceil(width / (d_FeetPerMeter/2));
	if(pxwid < 1)
		pxwid = 1;
	double pxht = ceil(height / (d_FeetPerMeter/2));
	if(pxht < 1)
		pxht = 1;
	img = QImage((int)pxwid, (int)pxht, QImage::Format_Mono);
	img.fill(Qt::white);				// This creates a black image?

	// Create a projection matrix to put the origin at the bitmap center and scale so every pixel is 1/2 meter.
	AuMatrix4x4 proj;
	proj.Translate(pxwid/2, pxht/2, 0);
	proj.Scale(2/d_FeetPerMeter, 2/d_FeetPerMeter, 1);			// Scale outline to 1/2 meter increments.

	// Include model matrix.
	proj *= model;

	// Paint triangles into image.
	QPainter p(&img);
	float penwid = (margin * 2.f) / (d_FeetPerMeter/2);			// Margin is created by painting with a thick pen.
	p.setPen(QPen(QColor(255,255,255), penwid));
	p.setBrush(QColor(255,255,255));
	for(AuScTriList::const_iterator ti = tris.cbegin(); ti != tris.cend(); ti++)
	{
		AuPoint v1(proj.Project((*ti).V0()));
		AuPoint v2(proj.Project((*ti).V1()));
		AuPoint v3(proj.Project((*ti).V2()));
		QPointF qv1(v1.x, v1.y);
		QPointF qv2(v2.x, v2.y);
		QPointF qv3(v3.x, v3.y);

		QPolygonF poly;
		poly << qv1;
		poly << qv2;
		poly << qv3;

		QPainterPath path;
		path.addPolygon(poly);
		p.drawPath(path);
	}
}

bool AuScSilhouette3d::IsPointInSilhouette(const AuPoint& loc, const QImage& img, const AuMatrix4x4& model)
{
	// Create a projection matrix to put the origin at the bitmap center and scale so every pixel is 1/2 meter.
	AuMatrix4x4 proj;
	proj.Translate(img.width()/2, img.height()/2, 0);
	proj.Scale(2/d_FeetPerMeter, 2/d_FeetPerMeter, 1);			// Scale outline to 1/2 meter increments.

	// Include model matrix.
	proj *= model;

	AuPoint locXY(proj.Project(loc).Rounded());
	QPoint qlocXY((int)locXY.x, (int)locXY.y);
	if(qlocXY.x() < 0 || qlocXY.x() >= img.width())
		return false;
	if(qlocXY.y() < 0 || qlocXY.y() >= img.height())
		return false;

	QRgb pXY = img.pixel(qlocXY);
	return pXY == qRgb(255,255,255);
}

quint64 AuScSilhouette3d::MemoryUsage() const
{
	quint64 mem = 0;
	mem += sizeof(m_Margin);
	mem += sizeof(m_XYMax);
	mem += sizeof(m_XYMin);
#if(QT_VERSION >= QT_VERSION_CHECK(5,10,0))
	mem += m_XYSilhouette.sizeInBytes();
	mem += m_XZSilhouette.sizeInBytes();
	mem += m_YZSilhouette.sizeInBytes();
#else
	mem += m_XYSilhouette.byteCount();
	mem += m_XZSilhouette.byteCount();
	mem += m_YZSilhouette.byteCount();
#endif
	mem += sizeof(m_XZMax);
	mem += sizeof(m_XZMin);
	mem += sizeof(m_YZMax);
	mem += sizeof(m_YZMin);
	return mem;
}

void AuScSilhouette3d::operator>>(QDataStream& out) const
{
	out << m_Margin;
	out << m_XYMax;
	out << m_XYMin;
	out << m_XYSilhouette;
	out << m_XZMax;
	out << m_XZMin;
	out << m_XZSilhouette;
	out << m_YZMax;
	out << m_YZMin;
	out << m_YZSilhouette;
}

void AuScSilhouette3d::operator<<(QDataStream& in)
{
	in >> m_Margin;
	in >> m_XYMax;
	in >> m_XYMin;
	in >> m_XYSilhouette;
	in >> m_XZMax;
	in >> m_XZMin;
	in >> m_XZSilhouette;
	in >> m_YZMax;
	in >> m_YZMin;
	in >> m_YZSilhouette;
}

void AuScSilhouette3d::ReadOld(QDataStream& in)
{
	m_Margin = (float)d_FeetPerMeter;	// All margins were 1m in the old silhouettes.
	in >> m_XYMax;
	in >> m_XYMin;
	in >> m_XYSilhouette;
	in >> m_XZMax;
	in >> m_XZMin;
	in >> m_XZSilhouette;
	in >> m_YZMax;
	in >> m_YZMin;
	in >> m_YZSilhouette;
}
