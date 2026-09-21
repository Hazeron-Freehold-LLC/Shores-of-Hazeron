//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aupoint.cpp,v 1.39 2022/02/19 18:38:15 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aupoint.h"
//
#include "auglobell.h"
#include "auline.h"
#include "aumatrix3x3.h"
#include "aumatrix4x4.h"
#include "auplane.h"
#include "ausphere.h"

// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
// The Q_DECLARE_METATYPE macro is also required.
static int s_AuPointMetaType = qRegisterMetaType<AuPoint>();

static AuPoint s_InvalidPoint(-10.023,-400.056,-7000.089);		// Never store this value in a file, if you can help it.
static AuPoint s_NullPoint(0,0,0);

QDataStream& operator<<(QDataStream& out, const AuPoint& p)
{
	p >> out;
	return out;
}

QDataStream& operator>>(QDataStream& in, AuPoint& p)
{
	p << in;
	return in;
}

// qHash is needed to use AuPoint in a QList in Windows.
unsigned int qHash(const AuPoint& key)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << key.x;
	out << key.y;
	out << key.z;
	return qHash(ba);
}

QDebug operator<<(QDebug dbg, const AuPoint& p)
{
	if(p.z == 0)
		dbg.nospace() << "(" << p.x << ", " << p.y << ")";
	else
		dbg.nospace() << "(" << p.x << ", " << p.y << ", " << p.z << ")";
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuPoint AuPoint::operator%(const AuMatrix3x3 &r) const
{
	return r.Inverse() * *this;
}

AuPoint AuPoint::operator/(const AuMatrix3x3 &r) const
{
	return AuPoint(
		r.v[0].x*x+r.v[1].x*y+r.v[2].x*z,
		r.v[0].y*x+r.v[1].y*y+r.v[2].y*z,
		r.v[0].z*x+r.v[1].z*y+r.v[2].z*z);
}

//static
const AuPoint& AuPoint::Invalid()
{
	return s_InvalidPoint;
}

//static
const AuPoint& AuPoint::Null()
{
	return s_NullPoint;
}

void AuPoint::StringValue(const QString& sParse, double scale/*=1.*/, const AuMatrix4x4* designPlane/*=0*/)
{
	// init values
	x = 0.0;
	y = 0.0;
	z = 0.0;

	// Check for absolute specifier.
	QString sInput = sParse.trimmed();
	bool isAbsolute = false;
	QChar ch = sInput[0];
	if(ch.toLower() == 'a')
	{
		// an absolute value that is not offset by the design plane
		sInput = sInput.mid(1).trimmed();
		isAbsolute = true;				// Not relative to design plane, absolute world space coordinate as entered.
	}

	// turn the string into a coordinate value
	if(!sInput.isEmpty())
	{
		if(ch.toLower() == 'p')
		{
			// polar coordinate (pLEN,ANGLE,INCLINE)
			sInput = sInput.mid(1).trimmed();					// remainder after leading character
			StringPolarValue(sInput);
		}
		else
		{
			// default is a cartesian coordinate (X,Y,Z)
			// omitted values are taken as 0
			int iComma = sInput.indexOf(',');
			if(iComma < 0)
			{
				// A single value was entered.
				// In this case, it is treated as a distance value in the default compass direction.
				double rval = sInput.toDouble();				// no commas, X value only (empty field is rejected)
				x = rval;
			}
			else
			{
				// there is a comma
				QString sX = sInput.left(iComma).trimmed();		// the X component
				sInput = sInput.mid(iComma+1).trimmed();		// remainder after X part
				x = sX.toDouble();								// X value (take empty field as 0)

				iComma = sInput.indexOf(',');					// look for another comma
				if (iComma < 0)
					y = sInput.toDouble();						// Y value only, no Z (take empty field as 0)
				else
				{
					// there is a second comma, this is an XYZ value
					QString sY = sInput.left(iComma).trimmed();	// the Y component
					sInput = sInput.mid(iComma+1).trimmed();	// remainder after Y part
					y = sY.toDouble();							// Y value (take empty field as 0)
					z = sInput.toDouble();						// Z value (take empty field as 0)
				}
			}
		}
	}

	// Apply scale for units conversion before projecting through the workplane matrix.
	if(scale > 0. && scale != 1.)
		operator*=(scale);

	// Project the result into the design plane, unless the origin type is absolute. 
	if(!isAbsolute && designPlane)
		*this = designPlane->Project(*this);
}

// Return fillet information based on AutoCAD bulge factor.
// This routine assumes 2 line created by fprev to 'this and
// 'this' to fnext are going to be filleted.  this routine 
// will return the points and bulge factor needed to create
// the autocad fillet.  if success then this routine returns true,
// else false
//
bool AuPoint::AcadFillet(const AuPoint &fPrev, const AuPoint &fNext, const double rFillet,
		                AuPoint &fP1, AuPoint &fP2, double &rbulge) const
{
	AuPoint pCenter;
	AuPoint p1 = fPrev - *this;
	AuPoint p2(0,0,0);
	AuPoint p3 = fNext - *this;

	AuPoint pA1, pA2, pB1, pB2;
	AuPoint ptmp1, ptmp2;
	
	ptmp1 = p3.Perpendicular(p1, p2);
	ptmp2 = p3 - ptmp1;
	ptmp2.Unit(rFillet);
	pA1 = ptmp2;
	pA2 = p1+ptmp2;

	ptmp1 = p1.Perpendicular(p3, p2);
	ptmp2 = p1 - ptmp1;
	ptmp2.Unit(rFillet);
	pB1 = ptmp2;
	pB2 = p3+ptmp2;

	AuLine lseg1(pA1, pA2);
	AuLine lseg2(pB1, pB2);
	if (lseg1.Intersect(pCenter, lseg2))
	{
		// determine endpoints
		fP1 = pCenter.Perpendicular(p1, p2);
		fP2 = pCenter.Perpendicular(p2, p3);
		
		// check for positioning
		if (!fP1.Between(p1,p2))
			return(false);
		if (!fP2.Between(p2,p3))
			return(false);

		// add in original offset
		fP1 += *this;
		fP2 += *this;

		//
		// now calcuate the bulge factor for Autocad.  Autocad
		// uses the tangent of 1/4 of the included angle  of the arc and the direction
		// controls the sign of the bulge factor.
		AuPointArray aTemp;
		aTemp.append(fPrev);
		aTemp.append(*this);
		aTemp.append(fNext);
		int nSign = 1;
		nSign = (aTemp.IsClockwise()) ? -1 : 1;
		p1.Unit();
		p3.Unit();
		double rAngle = PI - acos(p1.Dot(p3));
		rbulge = nSign * tan(0.25 * rAngle);

		return(true);
	}

	return(false);
	
}

// Returns rotation required to align 'this' vector to another vector.
AuMatrix4x4 AuPoint::GetAlignmentTransform(const AuPoint &vAlignTo) const
{
	// Create a unit transform for a return value.
	AuMatrix4x4 tRot;

	// Make unit vectors.
	if(!IsZero() && !vAlignTo.IsZero())
	{
		AuPoint vThis(*this);
		vThis.Unit();
		AuPoint vAlign(vAlignTo);
		vAlign.Unit();

		// Determine normal vector to the two vectors.
		if(vAlign != vThis)
		{
			AuPoint vN = vAlign.Cross(vThis);
			if(!vN.IsZero())
			{
				// Rotate a transformation matrix around the normal vector by the angle between the vectors.
				vN.Unit();
				double rAng = vAlign.AngleTo(vThis);
				tRot.RotateAxis(-rAng, vN);
			}
			else if(vAlign.Dot(vThis) < 0.)
			{
				// The vectors point exactly away from each other.
				tRot.RotateX(PI);
			}
		}
	}
	return tRot;
}

// Create the 3D box points from this point and an opposite corner point.
// The points returned are clockwise from this point.
void AuPoint::MakeRectangle(const AuPoint &fCorner,AuPoint fLocs[4],AuPoint *pRot/*=NULL*/) const
{
	AuMatrix4x4 tRot;
	if (pRot)
		tRot.Rotate(*pRot);

	AuPoint fX = tRot.Project(AuPoint(1,0,0));
	AuPoint fY = tRot.Project(AuPoint(0,1,0));

	AuPoint fDiag = fCorner - *this;
	double rx = fDiag.Dot(fX);
	double ry = fDiag.Dot(fY);

	fLocs[0] = *this;
	fLocs[2] = fCorner;
	if ((rx < 0.0 && ry > 0.0) || (rx > 0.0 && ry < 0.0))
	{
		// The signs of the x and y offsets are different.
		fLocs[3] = *this + fY*ry;
		fLocs[1] = *this + fX*rx;
	}
	else
	{
		// The signs of the x and y offsets are the same.
		fLocs[1] = *this + fY*ry;
		fLocs[3] = *this + fX*rx;
	}
}

bool AuPoint::InTriangle_3D(const AuPoint& f1, const AuPoint& f2, const AuPoint& f3) const
{
	if(f1.z == f2.z && f2.z == f3.z)
		return InTriangle(f1, f2, f3);	// We can use the 2D solution.

	// Plane vectors of the triangle.
	AuPoint v2(f2 - f1);
	AuPoint v3(f3 - f1);

	// Make unit vectors to define the plane of the triangle.
	AuPoint vX(v3);
	vX.Unit();
	AuPoint vY(v2);
	vY.Unit();

	// Plane normal of the triangle.
	AuPoint vN = vX.Cross(vY);
	vN.Unit();

	// Vector from the origin of the triangle to me.
	AuPoint v(*this - f1);

	// Calculate altitude above or below the plane of the triangle.
	double tz = v.Dot(vN);
	if(!::IsFuzzyZero(tz))
		return false;

	// 2D triangle X locs in its own plane.
	// f1 is at the origin.
	double t2x = v2.Dot(vX);
	double t3x = v3.Dot(vX);

	// Search X in plane of triangle. We already calculated the Z value.
	double tx = v.Dot(vX);
	if((tx < 0 && tx < t2x && tx < t3x) ||
			(tx > 0 && tx > t2x && tx > t3x))
		return false;					// X coord is beyond the rectangular extents of the triangle.

	// Recalculate vY to be perpendicular.
	vY = vN.Cross(vX);
	vY.Unit();

	// 2D triangle Y locs in its own plane.
	// f1 is at the origin.
	double t2y = v2.Dot(vY);
	double t3y = v3.Dot(vY);

	// Search Y in plane of triangle. We already calculated the Z value.
	double ty = v.Dot(vY);
	if((ty < 0 && ty < t2y && ty < t3y) ||
			(ty > 0 && ty > t2y && ty > t3y))
		return false;					// Y coord is beyond the rectangular extents of the triangle.

	// Now we can calculate like a 2D triangle.
	return AuPoint(tx,ty).InTriangle(AuPoint(0,0), AuPoint(t2x,t2y), AuPoint(t3x,t3y));
}

bool AuPoint::InTriangle_3D(const AuPoint& f1, const AuPoint& f2, const AuPoint& f3, const AuPoint& vN) const
{
	if(f1.z == f2.z && f2.z == f3.z)
		return InTriangle(f1, f2, f3);	// We can use the 2D solution.

	// Vector from the origin of the triangle to me.
	AuPoint v(*this - f1);

	// Calculate altitude above or below the plane of the triangle.
	double tz = v.Dot(vN);
	if(!::IsFuzzyZero(tz))
		return false;

	// Plane vectors of the triangle.
	AuPoint v2(f2 - f1);
	AuPoint v3(f3 - f1);

	// Make unit vectors to define the plane of the triangle.
	AuPoint vX(v3);
	vX.Unit();

	// 2D triangle X locs in its own plane.
	// f1 is at the origin.
	double t2x = v2.Dot(vX);
	double t3x = v3.Dot(vX);

	// Search X in plane of triangle. We already calculated the Z value.
	double tx = v.Dot(vX);
	if((tx < 0 && tx < t2x && tx < t3x) ||
			(tx > 0 && tx > t2x && tx > t3x))
		return false;					// X coord is beyond the rectangular extents of the triangle.

	// Calculate vY to be perpendicular.
	AuPoint vY = vN.Cross(vX);
	vY.Unit();

	// 2D triangle Y locs in its own plane.
	// f1 is at the origin.
	double t2y = v2.Dot(vY);
	double t3y = v3.Dot(vY);

	// Search Y in plane of triangle. We already calculated the Z value.
	double ty = v.Dot(vY);
	if((ty < 0 && ty < t2y && ty < t3y) ||
			(ty > 0 && ty > t2y && ty > t3y))
		return false;					// Y coord is beyond the rectangular extents of the triangle.

	// Now we can calculate like a 2D triangle.
	return AuPoint(tx,ty).InTriangle(AuPoint(0,0), AuPoint(t2x,t2y), AuPoint(t3x,t3y));
}

bool AuPoint::InTriangleFuzzy_3D(const AuPoint& f1, const AuPoint& f2, const AuPoint& f3, const AuPoint& vN, double planeTol) const
{
	if(f1.z == f2.z && f2.z == f3.z)
		return InTriangle(f1, f2, f3);	// We can use the 2D solution.

	// Vector from the origin of the triangle to me.
	AuPoint v(*this - f1);

	// Calculate altitude above or below the plane of the triangle.
	double tz = v.Dot(vN);
	if(fabs(tz) > planeTol)
		return false;

	// Plane vectors of the triangle.
	AuPoint v2(f2 - f1);
	AuPoint v3(f3 - f1);

	// Make unit vectors to define the plane of the triangle.
	AuPoint vX(v3);
	vX.Unit();

	// 2D triangle X locs in its own plane.
	// f1 is at the origin.
	double t2x = v2.Dot(vX);
	double t3x = v3.Dot(vX);

	// Search X in plane of triangle. We already calculated the Z value.
	double tx = v.Dot(vX);
	//if((tx < 0 && tx < t2x && tx < t3x) ||
			//(tx > 0 && tx > t2x && tx > t3x))
	if((FuzzyLessThan(tx,0) && FuzzyLessThan(tx,t2x) && FuzzyLessThan(tx,t3x)) ||
			(FuzzyGreaterThan(tx,0) && FuzzyGreaterThan(tx,t2x) && FuzzyGreaterThan(tx,t3x)))
		return false;					// X coord is beyond the rectangular extents of the triangle.

	// Calculate vY to be perpendicular.
	AuPoint vY = vN.Cross(vX);
	vY.Unit();

	// 2D triangle Y locs in its own plane.
	// f1 is at the origin.
	double t2y = v2.Dot(vY);
	double t3y = v3.Dot(vY);

	// Search Y in plane of triangle. We already calculated the Z value.
	double ty = v.Dot(vY);
	//if((ty < 0 && ty < t2y && ty < t3y) ||
			//(ty > 0 && ty > t2y && ty > t3y))
	if((FuzzyLessThan(ty,0) && FuzzyLessThan(ty,t2y) && FuzzyLessThan(ty,t3y)) ||
			(FuzzyGreaterThan(ty,0) && FuzzyGreaterThan(ty,t2y) && FuzzyGreaterThan(ty,t3y)))
		return false;					// Y coord is beyond the rectangular extents of the triangle.

	// Now we can calculate like a 2D triangle.
	return AuPoint(tx,ty).InTriangleFuzzy(AuPoint(0,0), AuPoint(t2x,t2y), AuPoint(t3x,t3y));
}

// Note: This function can be used in place of GetDistFromPlane() when speed is of the essence.
// Determines perpendicular distance this AuPoint is from Plane.
// Sets rPerpDistFromPlane equal to this distance.
// bIncludeSign indicates whether to return a signed or unsigned (absolute) value.
// Sign on returned distance indicates direction relative to normal vector of Plane.
void AuPoint::GetPerpDistFromPlane(double& rPerpDistFromPlane, const AuPlane& Plane, bool bIncludeSign/*=false*/) const
{
	Q_ASSERT(::FuzzyEquals(Plane.N.Length(), 1));
	AuPoint PtOfOrigin = Plane.N * Plane.d;
	AuPoint VecFromOrigin = *this - PtOfOrigin;
	double rPerpDistFromPlane_Signed = VecFromOrigin.Dot(Plane.N);
	if (bIncludeSign)
		rPerpDistFromPlane = rPerpDistFromPlane_Signed;
	else
		rPerpDistFromPlane = fabs(rPerpDistFromPlane_Signed);
}

static AuPoint zero=AuPoint(0,0,0);
static AuMatrix3x3 iden=AuMatrix3x3(AuPoint(1,0,0),AuPoint(0,1,0),AuPoint(0,0,1));

AuMatrix3x3 AuPoint::operator/(const AuPoint &p) const
{
	if (*this==zero || p==zero)
		return iden;
	AuPoint cross=(*this).Cross(p);
	double dot=(*this).Dot(p);
	if (cross==zero)
	{
		if (dot>=0.) return iden;
		cross=p.Cross(AuPoint(0,0,1));
		if (cross==zero)
			cross=p.Cross(AuPoint(0,1,0));
	}
	AuPoint ppp(atan2(cross.y,sqrt(cross.x*cross.x+cross.z*cross.z)),-atan2(cross.x,cross.z),0);
	AuMatrix3x3 a(ppp);
	AuMatrix3x3 b=a;
	b.RotateZ(-ACOS(dot/((*this).Length()*p.Length())));
	return b/a;
}

AuMatrix3x3 AuPoint::operator%(const AuPoint &p) const
{
	double l=p.Length();
	if (l==0)
		return iden;
	return (*this/p)*((*this).Length()/l);
}

bool AuPoint::RimPoint(const AuPoint &fBeg, const AuPoint &fEnd,
		const double rBulge)
{

	// make sure points are not on top of each other and that bulge factor exists
	AuPoint fVec = fEnd - fBeg;
	double rChordLength = fVec.Length();
	if ( (rChordLength < FUZZ) ||
		 (fabs(rBulge) < FUZZ) )
		return(false);
	// set the unit vector from fEnd to fBeg
	fVec.Unit();

	// calulate the distance from the rim point of the arc to the chord.
	// do this by calcuating the radius, then the distance from
	// the center of the arc to the chord.  The offset distance
	// will be the radius - center distance.
	double rAngle = 4*atan(rBulge);
	double rRadius = (0.5 * rChordLength)/sin(rAngle/2.0);
	double rDist = rRadius * cos(rAngle/2.0);
	double rOffset = rRadius - rDist;

	// establish the center point of the arc by creating a perpendicular
	// from the midpoint of the segment and offsetting by radius - dist
	AuPoint fMid = fBeg + fVec * (rChordLength/2.0);
	AuPoint fPerp, fRim;
	fVec.ArbitraryAxis(fPerp);

	// to establish the correct rim point.  Offset a point from fmid, then
	// see if the area formed by fbeg, fEnd, fRim is clockwise or counter clockwise
	//
	fRim = fMid + (fPerp * rOffset);
	AuPointArray faBoundary;
	faBoundary.append(fBeg);
	faBoundary.append(fEnd);
	faBoundary.append(fRim);
	bool bClockWise = faBoundary.IsClockwise();
	if ( (bClockWise && (rBulge < 0.0)) ||
		 (!bClockWise && (rBulge > 0.0)) )
	{
		fRim = fMid + (fPerp * -rOffset);
	}
	x = fRim.x;
	y = fRim.y;
	z = fRim.z;

	return(true);
}

void AuPoint::CubeExtents(double r, const AuMatrix4x4& tModel, AuPoint& min, AuPoint& max)
{
	/*double yt = y + r;
	double yb = y - r;
	double xl = x - r;
	double xr = x + r;
	double zn = z + r;
	double zf = z - r;

	AuPoint pts[] = {
		AuPoint(xl,yt,zn),
		AuPoint(xr,yt,zn),
		AuPoint(xl,yb,zn),
		AuPoint(xr,yb,zn),
		AuPoint(xl,yt,zf),
		AuPoint(xr,yt,zf),
		AuPoint(xl,yb,zf),
		AuPoint(xr,yb,zf)
	};

	for(int i=0; i<8; i++)
	{
		pts[i] = tModel.Project(pts[i]);

		min = min.Min(pts[i]);
		max = max.Max(pts[i]);
	}*/

	AuPoint a, b;

	a.x = x - r;
	a.y = y - r;
	a.z = z - r;

	b.x = x + r;
	b.y = y + r;
	b.z = z + r;

	a = tModel.Project(a);
	b = tModel.Project(b);
	
	AuPoint localMin = a.Min(b);
	AuPoint localMax = a.Max(b);

	min = min.Min(localMin);
	max = max.Max(localMax);
}

// static function to calculate rotation of a plane with the vectors vx and vy.
// vx and vy do not have to be unit vectors.
AuPoint AuPoint::CalculateRotation(const AuPoint &vx, const AuPoint &vy)
{
	// Insure we have unit vectors.
	AuPoint fX(vx);
	if (fX.IsZero())
		fX = AuPoint(1,0);
	else
		fX.Unit();
	AuPoint fY(vy);
	if (fY.IsZero())
	{
		if (fX == AuPoint(0,1))
			fY = AuPoint(-1,0);
		else if (fX == AuPoint(0,-1))
			fY = AuPoint(1,0);
		else
			fY = AuPoint(0,1);
	}
	fY.Unit();

	AuPoint fRot;	// This will be our return value.

	AuPoint fZ(fX.Cross(fY));
	if (fZ.y!=0)
	{
		fRot.x=-atan2(fZ.y,fZ.z);
		if (fZ.x!=0)
			fRot.y=atan2(fZ.x,sqrt(fZ.z*fZ.z+fZ.y*fZ.y));
	}
	else if (fZ.x!=0)
		fRot.y=atan2(fZ.x,fZ.z);
	else if (fZ.z<0)
		fRot.y=PI;

	AuMatrix3x3 xMatrix(fRot);
	AuPoint tem=fX/xMatrix;
	fRot.z = atan2(tem.y,tem.x);

	if(::IsFuzzyZero(fRot.x))
		fRot.x = 0.0;
	if(::IsFuzzyZero(fRot.y))
		fRot.y = 0.0;
	if(::IsFuzzyZero(fRot.z))
		fRot.z = 0.0;

	return fRot;
}

AuPoint AuPoint::ClosestPointOnCircle(const AuPoint& cen, double r, const AuPoint& vn) const
{
	AuPlane p(cen, vn);					// The plane of the circle.
	AuLine l(*this, *this + vn);			// Line through 'this' point that is perpendicular to the plane of the circle.
	AuPoint pp;							// this point dropped into the plane of the circle.
	p.LineIntersection(l, pp);			// This could not fail because we know the line is not parallel to the plane.
	if(pp.FuzzyEquals(cen))
	{
		// This point is directly above or below the center of the circle. The answer is ambiguous.
		// Return any point on the circle.
		AuPoint vx;
		vn.ArbitraryAxis(vx);			// Calculate a unit vector in the plane of the circle.
		return cen + vx * r;			// Point on the circle that is on the arbitrary X axis line.
	}
	else
	{
		// This point is not directly above or below the center of the circle. Calculate closest point on circle to drop point.
		AuPoint vu(pp - cen);			// Vector to plane point from center.
		vu.Unit(r);						// Scale vector to the length of the circle radius.
		return cen + vu;				// Point on the circle that is on the line through the plane intersection point.
	}
}

// Finds the 2D intersection of infinite lines passing through points a0-a1 and b0-b1.
// Sets 'this' point to the 3D intersection along the a0-a1 line at the 2D intersection point.
// Returns true if an intersection was found.
bool AuPoint::LineIntersection2D_Result3D(const AuPoint& a0, const AuPoint& a1, const AuPoint& b0, const AuPoint& b1)
{
	// Set this to be the 2D intersection of the two lines, at Z=0.
	AuLine lineA(AuPoint(a0.x, a0.y, 0), AuPoint(a1.x, a1.y, 0));
	AuLine lineB(AuPoint(b0.x, b0.y, 0), AuPoint(b1.x, b1.y, 0));
	if(!lineA.Intersect(*this, lineB))
	{
		// The lines are parallel.
		*this = a0;
		return false;
	}

	// Determine the Z altitude of this intersection point along the line a0-a1.
	// Using the formula from AuLine: N.Dot.P + c = 0, solve for P.z.
	//   N is lineA.N
	//   P is this
	//   c is lineA.c
	//     N.Dot.P + c = 0
	//     N.Dot.P = -c
	//     N.x * P.x + N.y * P.y + N.z * P.z = -c
	//     N.z * P.z = -c - N.x * P.x - N.y * P.y
	//     P.z = (-c - N.x * P.x - N.y * P.y) / N.z
	if(lineA.N.z == 0)
	{
		// Avoid dividing by zero.
		z = a0.z;
	}
	else
	{
		// P.z = (-c - N.x * P.x - N.y * P.y) / N.z
		z = (-lineA.c - lineA.N.x * x - lineA.N.y * y) / lineA.N.z;
	}
	return true;
}

bool AuPoint::InEllipsoid(const AuPoint& center, double xrad, double yrad, double zrad) const
{
	if(xrad <= 0 || yrad <= 0 || zrad <= 0)
		return false;
	AuPoint s(*this - center);
	s.x /= xrad;
	s.y /= yrad;
	s.z /= zrad;
	return s.Length2() <= 1;
}

bool AuPoint::OnEllipsoid(const AuPoint& center, double xrad, double yrad, double zrad) const
{
	if(xrad <= 0 || yrad <= 0 || zrad <= 0)
		return false;
	return FuzzyEquals(ToEllipsoid(center, xrad, yrad, zrad));
}

AuPoint AuPoint::ToEllipsoid(const AuPoint& center, double xrad, double yrad, double zrad) const
{
	if(xrad <= 0 || yrad <= 0 || zrad <= 0)
		return AuPoint(0,0);
	AuPoint s(*this - center);
	s.x /= xrad;
	s.y /= yrad;
	s.z /= zrad;
	s.Unit();
	s.x *= xrad;
	s.y *= yrad;
	s.z *= zrad;
	s += center;
	return s;
}

int AuPoint::IntersectEllipsoid(const AuPoint& center, double xrad, double yrad, double zrad, const AuPoint& l, AuPoint& intersect0,
		AuPoint& intersect1) const
{
	AuPoint se(*this - center);
	se.x /= xrad;
	se.y /= yrad;
	se.z /= zrad;
	AuPoint sl(l - center);
	sl.x /= xrad;
	sl.y /= yrad;
	sl.z /= zrad;
	AuSphere su(AuPoint(0,0), 1);
	int isect = su.Intersect(se, sl, intersect0, intersect1);
	if(isect >= 1)
	{
		intersect0.x *= xrad;
		intersect0.y *= yrad;
		intersect0.z *= zrad;
		intersect0 += center;
	}
	if(isect == 2)
	{
		intersect1.x *= xrad;
		intersect1.y *= yrad;
		intersect1.z *= zrad;
		intersect1 += center;
	}
	return isect;
}

bool AuPoint::InSphere(const AuPoint& center, double rad) const
{
	if(rad <= 0)
		return false;
	AuPoint s((*this - center) / rad);
	return s.Length2() <= 1;
}

bool AuPoint::OnSphere(const AuPoint& center, double rad) const
{
	if(rad <= 0)
		return false;
	return FuzzyEquals(ToSphere(center, rad));
}

AuPoint AuPoint::ToSphere(const AuPoint& center, double rad) const
{
	if(rad <= 0)
		return AuPoint(0,0);
	AuPoint s(*this - center);
	s.Unit(rad);
	return s + center;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Inserts a vector sorted along a direction vector. The order ascends through the array
// in the direction of the sort direction vector.
// Assumes the existing vectors in this array are already sorted.
// The AcrossDir vector is used to insure the the sequence of points through the array forms a continuous
// polyline without reversing back on itself. This only makes a difference when multiple vectors are at the
// same offset along the SortDir vector.
// 'rInitAcrossDot' is used for initial lateral sorting, particularly necessary when the first two points align in the sort direction.
// NOTE: Inserting into an array is inherently slow.
void AuPointArray::InsertSortedVectorPoly(const AuPoint &fVec, const AuPoint &fSortDir, const AuPoint &fAcrossDir, double rInitAcrossDot)
{
	// The dot product gives us the sort magnitude along the vector.
	double rSortDot = fVec.Dot(fSortDir);
	double rSortAcrossDot = fVec.Dot(fAcrossDir);
	double rLastAcrossDot = 0.0;

	// Insert the new vector before the first vector that is greater than the new one.
	AuPoint *pLoc = begin();
	for (int lIndex=0; lIndex < count(); lIndex++, pLoc++)
	{
		double rDot = pLoc->Dot(fSortDir);
		double rAcross = pLoc->Dot(fAcrossDir);
		if (FuzzyEquals(rSortDot, rDot))
		{
			// The offsets are about the same. Sort them laterally so as to form a continuous path from the last
			// point to the next point that does not reverse on itself.

			// When it's the first point in the array, the lateral sort order must be determined.
			if (lIndex == 0)
			{
				// If there's only one point in the array so far, the initial across dot product is used to determine the sort direction.
				rLastAcrossDot = rInitAcrossDot;

				// If there's more than one other point, we'll look ahead to determine the sort order.
				const AuPoint *pAheadLoc = pLoc+1;
				for (int lAheadIndex = lIndex + 1; lAheadIndex < count(); lAheadIndex++, pAheadLoc++)
				{
					double rAheadAcrossDot = pAheadLoc->Dot(fAcrossDir);
					if (!FuzzyEquals(rAheadAcrossDot, rAcross))
					{
						// The across offset to this next location is different from the current location.
						double rAheadDotVec = rAheadAcrossDot - rAcross;	// Across vector from this loc to the next one that's not in line.
						rLastAcrossDot = rSortAcrossDot - rAheadDotVec;
					}
				}
			}

			if (rSortAcrossDot > rLastAcrossDot)
			{
				// The point we're inserting turns in a positive direction away from the last point in the array.
				// Insert it here if the current point is even farther in the same direction.
				if (rAcross > rSortAcrossDot)
				{
					insert(pLoc, fVec);
					return;
				}
			}
			else
			{
				// The point we're inserting turns in a negative direction away from the last point in the array.
				// Insert it here if the current point is even farther in the same direction.
				if (rAcross < rSortAcrossDot)
				{
					insert(pLoc, fVec);
					return;
				}
			}
		}
		else if (rSortDot < rDot)
		{
			insert(pLoc, fVec);
			return;
		}
		rLastAcrossDot = rAcross;
	}

	// If we made it to here, the new vector is greater than all vectors currently in
	// this array. Just add it to the end.
	append(fVec);
}

// Triangulates a boundary with holes.
// Assumes all boundary segments are visible.
// 'this' array of points is modified.
// The resulting triangle points and edge flags are stored in pTri and bTedge.
void AuPointArray::Triangulate(AuPointArray &pTri, QVector<bool> &bTedge) const
{
	// Build properly ordered boundaries and edge flags from this array.
	AuPointArray pBndry;
	QVector<bool> bBedge;
	const AuPoint *p = begin();
	AuPoint fb;					// Begin point of the current boundary.
	bool bb = false;			// true when we've started a boundary.
	int ip=count();
	for (; ip; ip--, p++)
	{
		if (bb && p->FuzzyEquals(fb))
		{
			p++;
			ip--;
			break;
		}
		else
		{
			pBndry.append(*p);
			bBedge.append(true);
			if (!bb)
			{
				fb = *p;
				bb = true;
			}
		}
	}

	// list all opening locations
	AuPoint fOmin, fOmax;
	bb = false;
	AuPointArray pOpen;
	for (; ip; ip--, p++)
	{
		if (bb && p->FuzzyEquals(fb))
		{
			// Add the extents points of the opening to the array.
			pOpen.append(fOmin);
			pOpen.append(AuPoint(fOmin.x, fOmax.y, fOmin.z));
			pOpen.append(fOmax);
			pOpen.append(AuPoint(fOmax.x, fOmin.y, fOmin.z));
			bb = false;
		}
		else
		{
			if (!bb)
			{
				fOmin = *p;
				fOmax = *p;
				fb = *p;
				bb = true;
			}
			else
			{
				fOmin = fOmin.Min(*p);
				fOmax = fOmax.Max(*p);
			}
		}
	}

	// add openings to current boundary as CW holes
	pBndry.AddBoundaryOpenings(bBedge, pOpen);

	// Triangulate the surface.
	pBndry.Triangulate(bBedge, pTri, bTedge);

	// Triangulate remaining portions of holes between hole boundaries and openings.
	AuPoint *po = pOpen.begin();
	int ihole = 0;
	for (int io=0; io<pOpen.count(); io+=4, po+=4)
	{
		// Get hole boundary that fits within the opening.
		AuPointArray aHole;
		aHole.clear();
		int iboundary = ihole+2;		// Which boundary we're looking for.
		p = begin();
		bb = false;
		for (int ip=count(); ip; ip--, p++)
		{
			if (bb && p->FuzzyEquals(fb))
			{
				bb = false;
				if (iboundary == 0)
				{
					aHole.append(*p);
					break;
				}
			}
			else
			{
				if (!bb)
				{
					fb = *p;
					bb = true;
					iboundary--;
				}
				if (iboundary == 0)
					aHole.append(*p);
			}
		}
		if (!aHole[0].FuzzyEquals(aHole[aHole.count()-1]))
			aHole.append(aHole[0]);	// Close the hole boundary.

		// We have the closed hole boundary; triangulate it to each of the corners.
		// At each corner of the opening, triangulate to hole edge lines that result in clockwise triangles.
		// The hole boundary is assumed to be drawn counter-clockwise.
		// The hole boundary is also assumed to be convex.
		// The hole boundary may have edge lines that are colinear with the opening edges.
		AuPoint *poc = po;
		for (int ic=0; ic<4; ic++, poc++)
		{
			// Look for hole edges that result in clockwise triangles.
			AuPoint *ph = aHole.begin();
			for (int ih=aHole.count()-1; ih; ih--, ph++)
			{
				if ((ph+1)->SideOfLine_2D(*poc, *ph) == -1)
				{
					// The next boundary point is to the right of the line from the corner to 
					// the current boundary point. It is clockwise.
					// We can triangulate to this hole edge line.
					pTri.append(*poc);
					bTedge.append(false);
					pTri.append(*ph);
					bTedge.append(true);		// The edge that aligns with the hole edge is visible.
					pTri.append(*(ph+1));
					bTedge.append(false);
				}
			}
		}

		// It's possible now that the resulting hole is bounded by an opening triangle.
		// That triangle edge must be flagged as visible.
		// Test hole edges for matches with existing triangle edges; make them visible.
		AuPoint *ph = aHole.begin();
		for (int ih = aHole.count()-1; ih; ih--, ph++)
		{
			// Search all triangles in the data.
			AuPoint *pt = pTri.begin();
			bool *pb = bTedge.begin();
			for (int it=pTri.count(); it; it-=3, pt+=3, pb+=3)
			{
				// Test first triangle edge against all hole sides.
				if ((pt->FuzzyEquals(*ph) && (pt+1)->FuzzyEquals(*(ph+1))) ||
					(pt->FuzzyEquals(*(ph+1)) && (pt+1)->FuzzyEquals(*ph)))
				{
					// The first edge of this triangle matches an edge of the hole.
					*pb = true;
				}

				// Test second triangle edge against all hole sides.
				if (((pt+1)->FuzzyEquals(*ph) && (pt+2)->FuzzyEquals(*(ph+1))) ||
					((pt+1)->FuzzyEquals(*(ph+1)) && (pt+2)->FuzzyEquals(*ph)))
				{
					// The first edge of this triangle matches an edge of the hole.
					*(pb+1) = true;
				}

				// Test third triangle edge against all hole sides.
				if (((pt+2)->FuzzyEquals(*ph) && pt->FuzzyEquals(*(ph+1))) ||
					((pt+2)->FuzzyEquals(*(ph+1)) && pt->FuzzyEquals(*ph)))
				{
					// The first edge of this triangle matches an edge of the hole.
					*(pb+2) = true;
				}
			}
		}

		ihole++;
	}
}

// pass:	pBndry CC boundary
//			pOpen  CW rectangular holes to add		
// add all openings to the wall boundary
void AuPointArray::AddBoundaryOpeningsCut(QVector<bool> &bBedge, const AuPointArray &pOpen, double rDistToCut)
{
	int iOpencnt = pOpen.count()/4;
	QVector<bool> bOpen;
	bOpen.resize(iOpencnt);
	for (int ix=0; ix<iOpencnt; ix++)
		bOpen[ix] = false;

  // PART I - ADD OPENINGS THAT ARE COLINEAR TO BOUNDARY
	// this is done so outside edge displays correctly.
	// without this part, we still obtain proper boundary, but there is no way to  generate correct edge display condition.
	
	//POSITION pos = m_pOpenings->GetHeadPosition();
	for (int iOpen=0; iOpen<iOpencnt; iOpen++)
	{
		// look at rectangular openings only
		// CWallOpening *pOpening;
		// pOpening = STATIC_DOWNCAST(CWallOpening, m_pOpenings->GetNext(pos));
		// if (pOpening->m_eShape  != OPENSHAPE_Rectangle)
		//	 continue;
		
		bool bFound = false;
		int io, ionext, ib, ibnext;
		for (io=(iOpen*4); io<(iOpen*4)+4; io++)
		{
			ionext = io+1;
			if (ionext==(iOpen*4)+4)
				ionext=iOpen*4;

			for (ib=0; ib<count(); ib++)
			{
				ibnext = ib+1;
				if (ibnext==count())
					ibnext = 0;
				
				// is this opening colinear and between any boundary segment
				bool bCutIn = false;
				if (rDistToCut <= 0.0)
				{
					if ((pOpen[io    ].IsOnSegment(at(ib), at(ibnext))) &&
						(pOpen[ionext].IsOnSegment(at(ib), at(ibnext))))
						bCutIn = true;
				}
				else
				{
					if ((pOpen[io    ].DistToSegment(at(ib), at(ibnext)) <= rDistToCut) &&
						(pOpen[ionext].DistToSegment(at(ib), at(ibnext)) <= rDistToCut))
						bCutIn = true;
				}
				if (bCutIn)
				{
					insert(begin()+ibnext, pOpen[io]);
					bBedge.insert(bBedge.begin()+ibnext, bBedge[ib]); // use current edge display on this boundary
					for (int ix=0; ix<3; ix++)
					{
						io--;
						if (io==(iOpen*4)-1)
							io = iOpen*4+3;
						insert(begin()+ibnext, pOpen[io]);
						bBedge.insert(bBedge.begin()+ibnext, false);
					}
					bFound = true;
					bOpen[iOpen]=true; // mark as used
					break;
				}
			}
			if (bFound)
				break;
		}
	}

  // PART II - ADD REMAINING OPENINGS TO CLOSEST BOUNDARY VERTEX

	// find closest opening to closest bndry location - iterate till all openings added
	while (true)
	{
		int iBindex = -1;
		int iOpennum = -1;
		int iOindex = -1;
		double dmax = -1;

		for (int iOpen=0; iOpen<iOpencnt; iOpen++)
		{
			// has this opening already been used
			if (bOpen[iOpen])
				continue;
			
			// find closest distance from bndry to opening
			int io, ib;
			for (io=(iOpen*4); io<(iOpen*4)+4; io++)
			{
				for (ib=0; ib<count(); ib++)
				{
					double dlen = at(ib).Distance_2D(pOpen[io]);
					if (iBindex == -1 || dlen<dmax)
					{
						iBindex = ib;
						iOpennum = iOpen;
						iOindex = io;
						dmax = dlen;
					}
				}
			}
		}

		if (iBindex == -1)
			return; // no more openings to process - get out
		
		int io = iOindex;

		bool bsave = bBedge[iBindex];
		bBedge[iBindex] = false;
		int ix=1;
		for (ix=1;ix<=4; ix++)
		{
			insert(begin()+iBindex+ix, pOpen[io++]);
			bBedge.insert(bBedge.begin()+iBindex+ix, false);
			if (io==(iOpennum*4)+4)
				io=iOpennum*4;
		}
		insert(begin()+iBindex+ix, pOpen[iOindex]);
		bBedge.insert(bBedge.begin()+iBindex+ix, false);		
		insert(begin()+iBindex+ix+1, at(iBindex));
		bBedge.insert(bBedge.begin()+iBindex+ix+1, bsave);	
		bOpen[iOpennum] = true;
	}
}

// Triangulates a boundary with holes.
// 'bBedge' is an array that corresponds to 'this' array. true indicates that the edge starting with the corresponding
// point in 'this' array is visible.
// 'this' and 'bBedge' arrays are modified.
// The resulting triangle points and edge flags are stored in pTri and bTedge.
void AuPointArray::Triangulate(QVector<bool> &bBedge, AuPointArray &pTri, QVector<bool> &bTedge)
{ 
	// debug
	//int ixp=count();
	//for (ixp=0; ixp<count(); ixp++)
	//{
	//	AuPoint p1 = at(ixp);
	//	bool bdisp = bBedge[ixp];
	//}

	pTri.clear();
	bTedge.clear();
	while(true)
	{
		int isize = count();
		if (isize<3)
			break; // under three points - get out

	  // REMOVE COLINEAR POINTS

		bool bRemoved = false;
		int ibeg=0;
		for (ibeg=0; ibeg<isize; ibeg++)
		{
			int imid, iend;
			imid = ibeg+1;
			if (imid==isize)
				imid = 0;
			iend = imid+1;
			if (iend==isize)
				iend = 0;
			if (at(iend).SideOfLine_2D(at(ibeg), at(imid))==0) // points are colinear - delete mid point and continue
			{
				erase(begin()+imid); // remove target point and continue
				bBedge.erase(bBedge.begin()+imid); // keep edge display in sync
				bRemoved = true;
				break; // reprocess
			}
		}
		if (bRemoved)
			continue;

		if (isize==3) // down to final triangle - store and get out
		{
			pTri.append(at(0));
			bTedge.append(bBedge[0]);
			pTri.append(at(1));
			bTedge.append(bBedge[1]);			
			pTri.append(at(2));
			bTedge.append(bBedge[2]);			
			break; // done
		}

	  // LOOP THRU ALL ANGLES

		double dMin = -1.;
		int itarget_beg = -1;
		int itarget_mid = -1;
		int itarget_end = -1;
		for (ibeg=0; ibeg<isize; ibeg++)
		{
			int imid, iend;
			imid = ibeg+1;
			if (imid==isize)
				imid = 0;
			iend = imid+1;
			if (iend==isize)
				iend = 0;
			
	  // TEST 1 - ONLY CONSIDER CONCAVE ANGLES
				
			// SideOfLine_2D(const AuPoint &p1,const AuPoint &p2) const;// 1=left,0=on,-1=right
			if (at(imid).SideOfLine_2D(at(ibeg), at(iend))!=-1) // concave angles only
				continue;

	  // TEST 2 - POLYGON VERTICIES MUST NOT BE WITHIN TRIANGLE
			
			bool bUse = true;
			for (int ix=0; ix<isize; ix++)
			{
				if (ix==ibeg || ix==imid || ix==iend) // skip triangle verticies
					continue;
				int i1 = at(ix).SideOfLine_2D(at(ibeg), at(imid));
				int i2 = at(ix).SideOfLine_2D(at(imid), at(iend));
				int i3 = at(ix).SideOfLine_2D(at(iend), at(ibeg));
				if (i1==1 && i2==1 && i3==1)
				{
					bUse = false;
					break;
				}
			}
			if (bUse!=true)
				continue;

			//	  this test is not necessary anymore - TEST 2 above has replaced this
			//	  // TEST 2A - POLYGON MUST NOT BE ENTIRELY ON RIGHT SIDE OF DIAGONAL
			//
			//			bool bUse = false;			
			//			for(int ix=iend+1; ix!=ibeg; ix++)
			//			{
			//				if (ix==isize)
			//				{
			//					ix=0;
			//					if (ibeg==0)
			//						break; // came full circle with no results
			//				}
			//				if (at(ix).SideOfLine_2D(at(ibeg), at(iend))==1) 
			//				{
			//					// this diagonal is acceptable
			//					bUse = true; // at least one point is on far side - get out
			//					break;
			//				}
			//			}
			//			if (bUse!=true)
			//				continue;

	  // TEST 3 - IS THIS THE SHORTEST DIAGONAL FOUND SO FAR

			double dDiag = at(ibeg).Distance_2D(at(iend)); // diagonal distance
			if (dMin == -1 || dDiag < dMin)
			{
				dMin = dDiag;
				itarget_beg = ibeg;
				itarget_mid = imid; // the location opposite the diagonal
				itarget_end = iend;
			}
		}

	  // NIBBLE THIS TRIANGLE OFF OF POLYGON

		if (itarget_mid == -1)
			return; // safety valve - should never get here
		
		pTri.append(at(itarget_beg));
		bTedge.append(bBedge[itarget_beg]);		
		pTri.append(at(itarget_mid));
		bTedge.append(bBedge[itarget_mid]);		
		pTri.append(at(itarget_end));
		bTedge.append(false);

		bBedge[itarget_beg] = false; // change status of display edge

		erase(begin()+itarget_mid); // remove target point and continue
		bBedge.erase(bBedge.begin()+itarget_mid); // keep edge display in sync
	}
}

// Applies the transformation t to this array of points.
void AuPointArray::Transform(const AuMatrix4x4 &t)
{
	AuPoint *p = begin();
	for (int ip=count(); ip; ip--, p++)
		*p = t.Project(*p);
}

// Applies the transformation t to this array of points.
void AuPointArray::TransformVectors(const AuMatrix4x4 &t)
{
	AuPoint *p = begin();
	for (int ip=count(); ip; ip--, p++)
		*p = t.ProjectVector(*p);
}

void AuPointArray::TriangulateBoundary(AuPointArray &pTri, QVector<bool> &bTedge) const
{
	// Copy the points into a boundary list that we can modify.
	QVector<AuPoint> lBoundary;
	const AuPoint *pLoc = begin();
	AuPoint fLast(s_InvalidPoint);
	for (int iloc=count(); iloc; iloc--, pLoc++)
	{
		// Only add unique points. Don't add repeats of the same point.
		if(!pLoc->FuzzyEquals(fLast))
		{
			lBoundary.append(*pLoc);
			fLast = *pLoc;
		}
	}
	
	// Remove points at the end of the boundary that match the beginning of the boundary.
	if(lBoundary.count() > 1)
		while(lBoundary.last().FuzzyEquals(lBoundary.first()))
			lBoundary.remove(lBoundary.count()-1);

	// Create triangles within the boundary.
	while (lBoundary.count() >= 3)
	{
		// Search for a place to add a triangle.
		bool bAddedTri = false;
		QVector<AuPoint>::iterator pos = lBoundary.begin();
		while (pos != lBoundary.end())
		{
			// Get the next three points in the list.
			const AuPoint &fLoc = *pos;
			pos++;

			QVector<AuPoint>::iterator npos = pos;	// Pos of next point in boundary.
			if (npos == lBoundary.end())
				npos = lBoundary.begin();
			QVector<AuPoint>::iterator rpos = npos;	// Pos to remove a pt if we add a tri.

			// Get next two boundary points.
			const AuPoint &fNextLoc1 = *npos;
			npos++;
			if (npos == lBoundary.end())
				npos = lBoundary.begin();
			const AuPoint &fNextLoc2 = *npos;

			// Test for clockwise or counterclockwise triangle.
			if (fLoc.SideOfLine_2D(fNextLoc1, fNextLoc2) >= 0)
				continue;	// Skip triangle that goes outside the boundary.

			// Test for overlap of edge lines with any existing edge lines.
			bool bOverlap = false;
			QVector<AuPoint>::iterator ipos = lBoundary.begin();
			while (ipos != lBoundary.end())
			{
				// Get the next two points in the list.
				const AuPoint &fILoc = *ipos;
				ipos++;
				QVector<AuPoint>::iterator nipos = ipos;
				if (nipos == lBoundary.end())
					nipos = lBoundary.begin();
				const AuPoint &fNextILoc = *nipos;

				// Skip segments that abut my triangle segments.
				if (fILoc.FuzzyEquals(fLoc) || fILoc.FuzzyEquals(fNextLoc1) || fILoc.FuzzyEquals(fNextLoc2))
					continue;

				// Test for boundary points inside of this triangle.
				if (fILoc.InTriangle_3D(fLoc, fNextLoc1, fNextLoc2))
				{
					bOverlap = true;
					break;
				}

				if (fNextILoc.FuzzyEquals(fLoc) || fNextILoc.FuzzyEquals(fNextLoc1) || fNextILoc.FuzzyEquals(fNextLoc2))
					continue;

				// Test for overlap with each potential triangle segment.
				AuPoint fInt;
				if (fInt.SegmentIntersection(fILoc, fNextILoc, fLoc, fNextLoc1) ||
					fInt.SegmentIntersection(fILoc, fNextILoc, fNextLoc1, fNextLoc2) ||
					fInt.SegmentIntersection(fILoc, fNextILoc, fNextLoc2, fLoc))
				{
					bOverlap = true;
					break;
				}
			}
			if (bOverlap)
				continue;	// Skip triangle that would overlap any existing boundary segment.

			// Determine edge visibility.
			bool bEdge0 = false;
			bool bEdge1 = false;
			bool bEdge2 = false;
			const AuPoint *pLoc = begin();
			for (int iloc=0; iloc < count(); iloc++, pLoc++)
			{
				const AuPoint *pNextLoc = pLoc + 1;
				if (iloc == count() - 1)
					pNextLoc = begin();
				if (pLoc->FuzzyEquals(fLoc) && pNextLoc->FuzzyEquals(fNextLoc1))
					bEdge0 = true;
				else if (pLoc->FuzzyEquals(fNextLoc1) && pNextLoc->FuzzyEquals(fNextLoc2))
					bEdge1 = true;
				else if (pLoc->FuzzyEquals(fNextLoc2) && pNextLoc->FuzzyEquals(fLoc))
					bEdge2 = true;
			}

			// Add a triangle.
			pTri.append(fLoc);
			bTedge.append(bEdge0);
			pTri.append(fNextLoc1);
			bTedge.append(bEdge1);
			pTri.append(fNextLoc2);
			bTedge.append(bEdge2);
			lBoundary.erase(rpos);
			bAddedTri = true;
			break;
		}
		if (!bAddedTri)
			break;
	}
}

// Clip this boundary using another boundary.
// This array is assumed to define a closed boundary.
// The clip poly must also form a closed boundary.
// The overlap region of the closed boundaries is removed from this boundary.
// If the clip poly is completely inside or outside this boundary, there is no effect.
// 'bKeepPrimaryChunk' is true to keep the main piece, false to keep the other piece.
void AuPointArray::ClipBoundary(const AuPointArray &aPoly, bool bKeepPrimaryChunk)
{
	// Do I have enough vertices to form a boundary?
	if (count() < 3)
		return;

	// Make a local copy of the clip array so we can modify it.
	AuPointArray aClip(aPoly);

	// Insert points in this boundary and in the clip poly where they intersect each other.
	// There must be an even number of intersections.
	int ics=0;
	for (ics=0; ics<aClip.count()-1; ics++)
	{
		bool bClipGrew = false;
		for (int ibnd=0; ibnd<count(); ibnd++)
		{
			// Determine index of next boundary vertex.
			int ibnd2 = ibnd + 1;
			if (ibnd2 >= count())
				ibnd2 = 0;

			// Get the boundary and clip poly segment vertices that we care about.
			AuPoint fc1 = aClip.at(ics);
			fc1.z = 0.0;
			AuPoint fc2 = aClip.at(ics + 1);
			fc2.z = 0.0;
			double rcl = fc1.Distance_2D(fc2);
			if (::IsFuzzyZero(rcl))
				continue;
			AuPoint fb1 = at(ibnd);
			fb1.z = 0.0;
			AuPoint fb2 = at(ibnd2);
			fb2.z = 0.0;
			double rbl = fb1.Distance_2D(fb2);
			if (::IsFuzzyZero(rbl))
				continue;

			// Test the segments for intersection.
			AuPoint fint;
			if (fint.SegmentIntersection(fc1, fc2, fb1, fb2))
			{
				// Insert the intersection point into the clip poly.
				if (!fc1.FuzzyEquals_2D(fint) && !fc2.FuzzyEquals_2D(fint))
				{
					// The intersection is not at an end point.
					double r = fint.Distance_2D(fc1)/rcl;
					AuPoint fs = aClip.at(ics);
					AuPoint v = (aClip.at(ics+1) - fs) * r;
					AuPoint fi = fs + v;
					aClip.insert(aClip.begin()+ics+1, fi);
					bClipGrew = true;
				}

				// Insert the intersection point into the boundary.
				if (!fb1.FuzzyEquals_2D(fint) && !fb2.FuzzyEquals_2D(fint))
				{
					// The intersection is not at an end point.
					double r = fint.Distance_2D(fb1)/rbl;
					AuPoint fs = at(ibnd);
					AuPoint v = (at(ibnd2) - fs) * r;
					AuPoint fi = fs + v;
					insert(begin()+ibnd2, fi);
					
					// Decrement the loop controller so we'll redo the segment that now goes from
					// 'ibnd' to the intersect point we just added.
					ibnd--;
				}
			}
		}
		
		if (bClipGrew)
		{
			// Decrement the loop controller so we'll redo the segment that now goes from
			// 'ics' to the intersect point we just added.
			ics--;
		}
	}

	// Determine a starting point by finding one of the intersections along the boundary.
	bool bFoundStart = false;
	const AuPoint *pb = begin();
	int ibnd=0;
	for (ibnd=0; ibnd<count(); ibnd++, pb++)
	{
		if (aClip.DoesContain_2D(*pb))
		{
			bFoundStart = true;
			// Break out leaving ibnd and pb set to the starting boundary point that we'll use.
			break;
		}
	}
	if (!bFoundStart)
		return;			// The boundaries must not intersect at all.

	RemoveBoundaryDuplicates_2D();
	aClip.RemoveBoundaryDuplicates_2D();
	bool bBoundaryDir = IsClockwise();
	bool bClipDir = aClip.IsClockwise();
	if (bKeepPrimaryChunk)
	{
		if (bBoundaryDir == bClipDir)
			aClip.ReverseOrder();		// This helps when tracing the boundaries.
	}
	else
	{
		if (bBoundaryDir != bClipDir)
			aClip.ReverseOrder();		// This helps when tracing the boundaries.
	}

	// Now build a new boundary by walking around this boundary. At intersections with
	// the clip poly, we will turn in the direction that is toward the inside of the boundary.
	AuPointArray aNewBnd;
	int iFirstBnd = ibnd;
	do
	{
		AuPoint fb = at(ibnd);
		aNewBnd.AddIfDifferentFromLast(fb);
		if (aNewBnd.Find_2D(fb) != aNewBnd.count()-1)
			break;	// Stop if we've somehow looped back onto the boundary.

		// Test to see if this is an intersection with the clip boundary.
		bool bFollowClip = false;
		ics = aClip.Find_2D(fb);
		if(ics != -1 && ics < aClip.count() - 1)
		{
			// This is an intersection point.
			int ibnd2 = ibnd + 1;
			if (ibnd2 >= count())
				ibnd2 = 0;
			AuPoint fbn = at(ibnd2);

			// Test the clip line to see if we are to follow it instead of the existing boundary.
			AuPoint fcn = aClip.at(ics + 1);	// Next clip boundary point.
			if (bBoundaryDir)
			{
				// Clockwise boundary so take right turns.
				if (fcn.SideOfLine_2D(fb, fbn) == -1)
				{
					// fb to fcn is a right turn.
					bFollowClip = true;
				}
			}
			else
			{
				// Counter clockwise boundary so take left turns.
				if (fcn.SideOfLine_2D(fb, fbn) == 1)
				{
					// fb to fcn is a left turn.
					bFollowClip = true;
				}
			}
		}
		if (bFollowClip)
		{
			// Follow the clip line until it rejoins with the boundary.
			for (;ics < aClip.count() - 1; ics++)
			{
				AuPoint fcn = aClip.at(ics + 1);	// Next clip boundary point.
				int iretbnd = Find_2D(fcn);
				if (iretbnd == -1)
				{
					// This clip point is not a boundary intersection point so add it to the new boundary.
					aNewBnd.AddIfDifferentFromLast(fcn);
					if (aNewBnd.Find_2D(fcn) != aNewBnd.count()-1)
					{
						ibnd = iFirstBnd;	// Stop if we've somehow looped back onto the boundary.
						break;
					}
				}
				else
				{
					// We've found the point at which we rejoin the boundary.
					ibnd = iretbnd;
					break;
				}
			}
		}
		else
		{
			ibnd += 1;
			if (ibnd >= count())
				ibnd = 0;
		}
	}
	while (ibnd != iFirstBnd);

	// If the new boundary looped back on itself, remove leading trailer up to loop point.
	int inewsize = aNewBnd.count();
	if (inewsize > 1)
	{
		int iloop = aNewBnd.Find_2D(aNewBnd.at(inewsize-1));
		if (iloop != inewsize-1)
		{
			// The last point appears earlier in the array.
			// Remove the points up to and including the first instance of the point.
			//for (iloop++;iloop;iloop--)
				//aNewBnd.RemoveAt(0);
			aNewBnd.erase(begin(), begin()+iloop);
		}
	}

	// Replace this boundary with the new one.
	clear();
	//Append(aNewBnd);
	AuPoint *pn = aNewBnd.begin();
	for (int ipn=0; ipn<aNewBnd.count(); ipn++, pn++)
		append(*pn);
}

// Join this boundary with another boundary.
// This array is assumed to define a closed boundary.
// The join poly must also form a closed boundary.
// If the join poly is completely inside or outside this boundary, there is no effect.
void AuPointArray::JoinBoundary(const AuPointArray &aPoly)
{
	// Do I have enough vertices to form a boundary?
	if ( (count() < 3) || (aPoly.count() < 3) )
		return;


	//
	// make sure boundaries are clockwise
	//
	bool bBoundaryDir = IsClockwise();
	if (!bBoundaryDir)
	{
		ReverseOrder();
		bBoundaryDir = true;
	}

	// Make a local copy of the join array so we can modify it.
	AuPointArray aJoin(aPoly);

	bool bJoinDir = aJoin.IsClockwise();
	if (bBoundaryDir != bJoinDir)
	{
		aJoin.ReverseOrder();		// This helps when tracing the boundaries.
		bJoinDir = true;
	}


	
	// Insert points in this boundary and in the join poly where they intersect each other.
	// There must be an even number of intersections.
	int ics2, ics=0;
	for (ics=0; ics<aJoin.count(); ics++)
	{
		bool bJoinGrew = false;
		for (int ibnd=0; ibnd<count(); ibnd++)
		{
			// Determine index of next boundary vertex.
			int ibnd2 = ibnd + 1;
			if (ibnd2 >= count())
				ibnd2 = 0;

			// Get the boundary and join poly segment vertices that we care about.
			AuPoint fc1 = aJoin.at(ics);
			fc1.z = 0.0;
			ics2 = (ics != aJoin.count()-1) ? ics+1: 0;
			AuPoint fc2 = aJoin.at(ics2);
			fc2.z = 0.0;
			double rcl = fc1.Distance_2D(fc2);
			if (::IsFuzzyZero(rcl))
				continue;
			AuPoint fb1 = at(ibnd);
			fb1.z = 0.0;
			AuPoint fb2 = at(ibnd2);
			fb2.z = 0.0;
			double rbl = fb1.Distance_2D(fb2);
			if (::IsFuzzyZero(rbl))
				continue;

			// Test the segments for intersection.
			AuPoint fint;
			if (fint.SegmentIntersection(fc1, fc2, fb1, fb2))
			{
				// Insert the intersection point into the join poly.
				if (!fc1.FuzzyEquals_2D(fint) && !fc2.FuzzyEquals_2D(fint))
				{
					// The intersection is not at an end point.
					double r = fint.Distance_2D(fc1)/rcl;
					AuPoint fs = aJoin.at(ics);
					AuPoint v = (aJoin.at(ics2) - fs) * r;
					AuPoint fi = fs + v;
					aJoin.insert(aJoin.begin()+ics2, fi);
					bJoinGrew = true;
				}

				// Insert the intersection point into the boundary.
				if (!fb1.FuzzyEquals_2D(fint) && !fb2.FuzzyEquals_2D(fint))
				{
					// The intersection is not at an end point.
					double r = fint.Distance_2D(fb1)/rbl;
					AuPoint fs = at(ibnd);
					AuPoint v = (at(ibnd2) - fs) * r;
					AuPoint fi = fs + v;
					insert(begin()+ibnd2, fi);
					
					// Decrement the loop controller so we'll redo the segment that now goes from
					// 'ibnd' to the intersect point we just added.
					ibnd--;
				}
			}
		}
		
		if (bJoinGrew)
		{
			// Decrement the loop controller so we'll redo the segment that now goes from
			// 'ics' to the intersect point we just added.
			ics--;
		}
	}

	RemoveBoundaryDuplicates_2D();
	aJoin.RemoveBoundaryDuplicates_2D();


	//
	// this routine is trying to return a super boundary.  If the calculated boundary ends
	// up counter clock wise then the routine found an interior boundary.  Need
	// to try and use a different start point.
	//
	bool bFoundStart;
	const AuPoint *pb;
	AuPointArray aUsedStartPoints;
	bool bRoutineCompleted = false;
	AuPointArray aNewBnd;
	
	//
	// do some pre-screening of the bndy to be joined.  if every point of the child (aJoin) is
	// already on the parent (this), then there is no need to ever turn toward the child.
	//
	bool bAllPointsContained = true;
	int i;
	for (i = 0; i < aJoin.count(); i++)
	{
		if (!DoesContain_2D(aJoin.at(i)))
		{
			bAllPointsContained = false;
			break;
		}
	}
	
	while (!bRoutineCompleted)
	{
	
		// Determine a starting point by finding one of the non intersections along the boundary that
		// has not yet been tried and is not inside of the join boundary
		bFoundStart = false;
		pb = begin();
		int ibnd=0;
		for (ibnd=0; ibnd<count(); ibnd++, pb++)
		{
			if ( (!aJoin.DoesContain_2D(*pb)) && (!aUsedStartPoints.DoesContain_2D(*pb)) &&
				 (!aJoin.PointInBoundary(*pb)))
			{
				bFoundStart = true;
				// Break out leaving ibnd and pb set to the starting boundary point that we'll use.
				break;
			}
		}
		if (!bFoundStart)
			return;			// The boundaries must not intersect at all.
		//
		// keep track of this start point
		//
		aUsedStartPoints.append(*pb);
	

		// Now build a new boundary by walking around this boundary. At intersections with
		// the Join poly, we will turn in the direction that is toward the outside of the boundary.
		int iFirstBnd = ibnd;
		bool bOnBnd = true;
		do
		{
			AuPoint fb = at(ibnd);
			aNewBnd.AddIfDifferentFromLast(fb);
			if (aNewBnd.Find_2D(fb) != aNewBnd.count()-1)
				break;	// Stop if we've somehow looped back onto the boundary.

			// Test to see if this is an intersection with the join boundary.
			bool bFollowJoin = false;
			ics = aJoin.Find_2D(fb);
			if (ics != -1 && ics < aJoin.count())
			{
				ics2 = (ics == aJoin.count() - 1) ? 0 : ics+1;
				// This is an intersection point.
				int ibnd2 = ibnd + 1;
				if (ibnd2 >= count())
					ibnd2 = 0;

				//
				// if we are on the boundary, then want to switch to the child
				//
				if (bOnBnd)
				{
					ics2 = (ics == aJoin.count() - 1) ? 0 : ics+1;
					AuPoint fcn = aJoin.at(ics2);

					//
					// don't switch to the join boundary if the join boundary next
					// point already exists on the new boundary.
					//
					if (!aNewBnd.DoesContain_2D(fcn))
						bFollowJoin = true;

				}
				else
				{
					bOnBnd = true;
				}

			}
			if (bFollowJoin)
			{
				// Follow the Join line until it rejoins with the boundary.
				for (int i = 0; i < aJoin.count(); i++)
				{
					ics = (ics < aJoin.count()) ? ics: 0;
					ics2 = (ics == aJoin.count() - 1) ? 0 : ics+1;
					AuPoint fcn = aJoin.at(ics2);	// Next join boundary point.
					int iretbnd = Find_2D(fcn);
					if (iretbnd == -1)
					{
						// This join point is not a boundary intersection point so add it to the new boundary.
						aNewBnd.AddIfDifferentFromLast(fcn);
						if (aNewBnd.Find_2D(fcn) != aNewBnd.count()-1)
						{
							ibnd = iFirstBnd;	// Stop if we've somehow looped back onto the boundary.
							break;
						}
						else
						{
							// point was added so we are now adding points from the child(join) boundary
							bOnBnd = false;
						}
					}
					else
					{
						// We've found the point at which we rejoin the boundary.
						ibnd = iretbnd;
						//
						// if all the child points are contained, then tell the parent
						// that it has added points from the child
						//
						if (bAllPointsContained)
							bOnBnd = false;
						break;
					}
					ics++;
				}
			}
			else
			{
				ibnd += 1;
				if (ibnd >= count())
					ibnd = 0;
			}
		}
		while (ibnd != iFirstBnd);

		// If the new boundary looped back on itself, remove leading trailer up to loop point.
		int inewsize = aNewBnd.count();
		if (inewsize > 1)
		{
			int iloop = aNewBnd.Find_2D(aNewBnd.at(inewsize-1));
			if (iloop != inewsize-1)
			{
				// The last point appears earlier in the array.
				// Remove the points up to and including the first instance of the point.
				//for (iloop++;iloop;iloop--)
					//aNewBnd.RemoveAt(0);
				aNewBnd.erase(begin(), begin()+iloop);
			}
		}
		
		//
		// if aNewBnd is clockwise then routine is completed
		//
		if (aNewBnd.IsClockwise())
			bRoutineCompleted = true;
		else
			aNewBnd.clear();
	}
	// Replace this boundary with the new one.
	clear();
	//Append(aNewBnd);
	AuPoint *pn = aNewBnd.begin();
	for(int ipn=0; ipn<aNewBnd.count(); ipn++, pn++)
		append(*pn);
}

void AuPointArray::CreateEllipse(const AuPoint& center, const AuPoint& ax, const AuPoint& ay, int facets)
{
	clear();

	AuPoint ve(ax - center);
	double radX = ve.Length_2D();
	ve /= radX;
	AuPoint vs(ay - center);
	double radY = vs.Length_2D();
	vs /= radY;

	for(int vert = 0; vert<facets; vert++)
	{
		double ang = (PI*2) - (PI*2) * vert / facets;			// Calculate angle for clockwise rotation.
		double ca = cos(ang);
		double sa = sin(ang);
		ca *= radX;
		sa *= radY;
		AuPoint v = center + ve*ca + vs*sa;
		append(v);
	}
}

bool AuPointArray::InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect) const
{
	for(const_iterator pi = cbegin(); pi != cend(); pi++)
	{
		AuPoint px = mvp.Project(*pi);
		if(pixelRect.contains(QPointF(px.x, px.y)))
			return true;
	}
	return false;
}

void AuPointArray::MakeCircle(const AuPoint& center, const AuPoint& start, int steps)
{
	AuPoint radiusVec(start - center);
	for(int step=0; step<steps; step++)
		append(center + radiusVec.RotateZ((PI*2) * step / steps));
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuPointTexN::Transform(const AuMatrix4x4& t)
{
	AuPoint::operator=(t.Project(*this));
	m_N = t.ProjectVector(m_N);
	m_N.Unit();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuPointAlpha::Transform(const AuMatrix4x4 t)
{
	AuPoint::operator=(t.Project(*this));
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

QDataStream& operator<<(QDataStream& out, const AuPoint2d& p)
{
	p >> out;
	return out;
}

QDataStream& operator>>(QDataStream& in, AuPoint2d& p)
{
	p << in;
	return in;
}

// qHash is needed to use AuPoint in a QList in Windows.
unsigned int qHash(const AuPoint2d& key)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << key.x;
	out << key.y;
	return qHash(ba);
}

QDebug operator<<(QDebug dbg, const AuPoint2d& p)
{
	dbg.nospace() << "(" << p.x << ", " << p.y << ")";
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuPoint2dArray::OffsetBoundary(double offsetLeft)
{
	if(offsetLeft == 0)
		return;
	if(size() < 3)
		return;

	// Calculate left-side perpendicular vector of closing segment, at the offset distance.
	AuPoint2d cvn;
	cvn = AuPoint2d(first() - last());
	cvn = cvn.RotateZ(PI/2);
	cvn.Unit(offsetLeft);

	AuPoint2d cv1(last() + cvn);
	AuPoint2d cv2(first() + cvn);
	AuLine lc(AuPoint(cv1.x, cv1.y), AuPoint(cv2.x, cv2.y));	// Closing offset line.
	AuLine lp(lc);
	for(iterator vi = begin(); vi != end(); vi++)
	{
		AuPoint2d vn;

		// Next vertex wraps at end for closure.
		iterator nvi = vi + 1;
		if(nvi == end())
		{
			// Use closing line normal.
			nvi = begin();
			vn = cvn;
		}
		else
		{
			// Calculate left-side perpendicular vector of next segment, at the offset distance.
			vn = AuPoint2d(*nvi - *vi);
			vn = vn.RotateZ(PI/2);
			vn.Unit(offsetLeft);
		}

		AuPoint2d v1(*vi + vn);
		AuPoint2d v2(*nvi + vn);
		AuLine ln(AuPoint(v1.x, v1.y), AuPoint(v2.x, v2.y));

		// Calculate new position for the current vertex.
		AuPoint i;
		if(lp.Intersect(i, ln))
			*vi = AuPoint2d(i.x, i.y);
		else
			*vi += vn;					// Lines were parallel.

		// Shuffle values for the next loop iteration.
		lp = ln;
	}
}

bool AuPoint2dArray::PointInBoundary(const AuPoint2d& loc) const
{
	// Insure this array contains enough vertices to form a boundary.
	if(count() < 3)
		return false;

	// Get the extents of this boundary.
	AuPoint2d minpt, maxpt;
	Extents(minpt, maxpt);
	if(loc.x <= minpt.x || loc.x >= maxpt.x || loc.y <= minpt.y || loc.y >= maxpt.y)
		return false;	// Point is outside the extents of the boundary.

	// Calculate the other end of a segment that can be tested against the boundary segments.
	AuPoint2d segVec(maxpt - minpt);
	segVec = segVec.RotateZ(DEG2RAD);	// Rotate test segment by a degree so we're less likely to hit a vertex the first time.
	AuPoint2d segEnd(loc + segVec);

	// Count the number of boundary segments that intersect the test segment.
	bool hitVertex;
	int intersects;
	int tries = 0;
	do
	{
		hitVertex = false;
		intersects = 0;
		AuPoint2d prev(last());	// Grab last boundary point so first segment tested is the closing segment.
		AuPoint2d intersect, fInt2;
		for(const_iterator it = constBegin(); it != constEnd(); it++)
		{
			if((*it) != prev)
			{
				// Test for intersection with the boundary segment from the iterator to the previous/last point.
				// This code was cobbled from SegmentIntersection_2D.
				// Perform 2D perpconn.
				if(!AuPoint2d(loc.x,loc.y).PerpConn(AuPoint2d(segEnd.x,segEnd.y),
							AuPoint2d((*it).x,(*it).y), AuPoint2d(prev.x,prev.y), intersect, fInt2))
				{
					// Segments are parallel. This is a problem if they overlap.
					if(::IsFuzzyZero((*it).DistToSegment(loc,segEnd)))
						hitVertex = true;	// Close enough of an overlap that we should rotate the test vector.
				}
				else if(intersect.FuzzyEquals(fInt2) && intersect.Between(loc,segEnd) && intersect.Between(*it,prev))
				{
					// The test segment intersected the boundary segment.
					if(intersect.FuzzyEquals(loc))
						return false;	// Test location is on the boundary segment.
					if(intersect.FuzzyEquals(*it) || intersect.FuzzyEquals(prev))
					{
						// Intersect at a boundary vertex introduces several potential problems.
						// Let's not get stuck doing this forever.
						tries++;
						if(tries == 360)
							return false;	// We've tried every degree direction; that's enough.
						hitVertex = true;
					}

					// Count segment intersections.
					intersects++;
				}
				if(hitVertex)
				{
					// Rotate the test segment one degree and count boundary segment intersections again.
					segVec = segVec.RotateZ(DEG2RAD);
					segEnd = loc + segVec;
					break;
				}
				prev = (*it);	// Current point becomes previous for next loop iteration.
			}
		}
	}
	while(hitVertex);

	// The point is in the boundary if an odd number of boundary segments must be crossed to leave the boundary.
	return (intersects&1)?true:false;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuPointN::Transform(const AuMatrix4x4& t)
{
	AuPoint::operator=(t.Project(*this));
	m_N = t.ProjectVector(m_N);
	m_N.Unit();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuPointNArray::Append(const AuGlobeLL& ll, double altitude, const AuMatrix4x4& m)
{
	AuPoint n(ll.GlobeLocation(1));		// Normal to the globe at ll.
	AuPoint p(n * altitude);			// Point on the globe surface at ll.
	AuPointN pn(p, n);
	pn.Transform(m);					// Transformed using m.
	append(pn);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

