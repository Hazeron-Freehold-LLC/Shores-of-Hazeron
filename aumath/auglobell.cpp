//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auglobell.cpp,v 1.20 2022/02/19 18:38:11 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auglobell.h"
//

// Must call qRegisterMetaType in order to use a class type in queued signal and slot connections.
// The Q_DECLARE_METATYPE macro is also required.
static int s_AuGlobeLLF = AuGlobeLLF::RegisterMetaType();
static int s_AuGlobeLL = AuGlobeLL::RegisterMetaType();

//static
int AuGlobeLLF::RegisterMetaType()
{
	// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
	// The Q_DECLARE_METATYPE macro is also required.
	int metaType = qRegisterMetaType<AuGlobeLLF>();

	// Must call qRegisterMetaTypeStreamOperators in order to use this class type in containers of variants.
	qRegisterMetaTypeStreamOperators<AuGlobeLLF>();

	return metaType;
}

//static
int AuGlobeLL::RegisterMetaType()
{
	// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
	// The Q_DECLARE_METATYPE macro is also required.
	int metaType = qRegisterMetaType<AuGlobeLL>();

	// Must call qRegisterMetaTypeStreamOperators in order to use this class type in containers of variants.
	qRegisterMetaTypeStreamOperators<AuGlobeLL>();

	return metaType;
}

AuGlobeLLF::AuGlobeLLF(const QVariant& v)
{
	QByteArray ba = v.toByteArray();
	QDataStream in(ba);
	in.setVersion(QDataStream::Qt_4_1);
	in >> m_Latitude;
	in >> m_Longitude;
}

QVariant AuGlobeLLF::ToVariant() const
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << m_Latitude;
	out << m_Longitude;
	return QVariant(ba);
}

// Latitude and longitude values of 'c' are added to 'this' coordinate.
// Wraps longitude and latitude when they rollover the poles or meridian.
AuGlobeLLF AuGlobeLLF::operator+=(const AuGlobeLLF& c)
{
	m_Latitude += c.m_Latitude;
	m_Longitude += c.m_Longitude;

	// Latitude rollover will not work if the wrap goes around both poles.
	if(m_Latitude > PIf)
	{
		// Wrap over north pole sends longitude around to the other side.
		m_Longitude += PIf;
		m_Latitude = PIf*2 - m_Latitude;
	}
	else if(m_Latitude < -PIf)
	{
		// Wrap over south pole sends longitude around to the other side.
		m_Longitude += PIf;
		m_Latitude = -PIf*2 - m_Latitude;
	}

	// Longitude rollover is easier.
	while(m_Longitude < -PIf)
		m_Longitude += PIf*2;
	while(m_Longitude > PIf)
		m_Longitude -= PIf*2;

	return *this;
}

// Returns the side of the line from 'll0' to 'll1' that this location is on.
// 1=left, 0=on, -1=right
int AuGlobeLLF::SideOfLine(const AuGlobeLLF& ll0, const AuGlobeLLF& ll1, LLSHAPE llShape) const
{
	if(llShape == LL_Cylinder)
	{
		// Cylinder can be treated linearly in both longitude and latitude.
		float rX1 = ll0.Longitude() - Longitude();
		float rY1 = ll0.Latitude() - Latitude();
		float rX2 = ll1.Longitude() - Longitude();
		float rY2 = ll1.Latitude() - Latitude();
		float rCross = (rX1 * rY2) - (rX2 * rY1);
		if(rCross < 0)
			return -1;
		else if(rCross > 0)
			return 1;
		else
			return 0;
	}
	else
	{
		// Globe must deal with narrowing longitudes as we move closer to the poles.
		AuPoint gl0 = ll0.GlobeLocation(1);
		AuPoint gl1 = ll1.GlobeLocation(1);

		AuPoint gv(gl1 - gl0);
		//gv.Unit();

		AuPoint up((gl0 + gl1)/2);
		//up.Unit();

		AuPoint xv = up.Cross(gv);

		AuPoint l = GlobeLocation(1);
		AuPoint lv = l - gl0;
		double rdot = lv.Dot(xv);
		if(rdot < 0.)
			return -1;
		else if(rdot > 0.)
			return 1;
		else
			return 0;
	}
}

// Returns true if this ll is in the triangle formed by 'll0', 'll1', and 'll2', in any order.
// CAVEAT: Does not work on triangles that wrap across the PI/-PI line.
bool AuGlobeLLF::InTriangle(const AuGlobeLLF& ll0, const AuGlobeLLF& ll1, const AuGlobeLLF& ll2, LLSHAPE llShape) const
{
	int lSide0 = SideOfLine(ll0,ll1, llShape);
	int lSide1 = SideOfLine(ll1,ll2, llShape);
	int lSide2 = SideOfLine(ll2,ll0, llShape);
	if(lSide0 <= 0 && lSide1 <= 0 && lSide2 <= 0)
		return true;				// This point is within a clockwise triangle formed by the three locations.
	else if(lSide0 >= 0 && lSide1 >= 0 && lSide2 >= 0)
		return true;				// This point is within a counterclockwise triangle formed by the three locations.
	return false;
}

// Returns true if this ll is in the triangle formed by 'll0', 'll1', and 'll2', in clockwise order.
// CAVEAT: Does not work on triangles that wrap across the PI/-PI line.
bool AuGlobeLLF::InTriangleClockwise(const AuGlobeLLF& ll0, const AuGlobeLLF& ll1, const AuGlobeLLF& ll2, LLSHAPE llShape) const
{
	int lSide0 = SideOfLine(ll0,ll1, llShape);
	int lSide1 = SideOfLine(ll1,ll2, llShape);
	int lSide2 = SideOfLine(ll2,ll0, llShape);
	if(lSide0 <= 0 && lSide1 <= 0 && lSide2 <= 0)
		return true;				// This point is within a clockwise triangle formed by the three locations.
	return false;
}

// Find closest point on a line segment.
// If the locate point is beyond either end point of the segment, the closest end point is returned
// 'segDot' is an optional parameter to receive the ratio of distance along the segment of the intersection, in the range 0..1.
// Takes the longitudinal angle from 'll1' to 'll2' that is less than 180 degrees.
AuGlobeLLF AuGlobeLLF::CoordOnSegment(const AuGlobeLLF& ll1, const AuGlobeLLF& ll2, float* segDot, LLSHAPE llShape) const
{
	if(llShape == LL_Cylinder)
	{
		// Cylinder can be treated linearly in both longitude and latitude.
		// Calculate vector of segment.
		float segvecX = ll2.LongitudePositive() - ll1.LongitudePositive();
		if(segvecX > PIf)
			segvecX -= PIf*2.f;
		else if(segvecX < -PIf)
			segvecX += PIf*2.f;
		float segvecY = ll2.Latitude() - ll1.Latitude();

		// Calculate length of segment.
		float rlen = sqrtf(segvecX*segvecX + segvecY*segvecY);
		if(rlen == 0.f)
		{
			if(segDot)
				*segDot = 0.f;
			return ll1;					// Segment length is 0.
		}

		// Calculate vector from ll1 to this location.
		float locvecX = LongitudePositive() - ll1.LongitudePositive();
		if(locvecX > PIf)
			locvecX -= PIf*2.f;
		else if(locvecX < -PIf)
			locvecX += PIf*2.f;
		float locvecY = Latitude() - ll1.Latitude();

		// Calculate dot product of locvec onto segvec.
		float rdot = (locvecX*segvecX + locvecY*segvecY)/rlen;
		float frac = rdot/rlen;
		if(segDot)
			*segDot = frac;

		if(rdot <= 0.f)
			return ll1;					// Beyond first segment end.
		else if(rdot >= rlen)
			return ll2;					// Beyond second end of segment.
		else
		{
			// Calculate intersection between segment points.
			float intX = ll1.LongitudePositive() + segvecX*frac;
			float intY = ll1.Latitude() + segvecY*frac;
			return AuGlobeLLF(intX, intY);						// Constructor normalizes the longitude into the proper range.
		}
	}
	else
	{
		// Globe must deal with narrowing longitudes as we move closer to the poles.
		AuPoint gl2 = ll2.GlobeLocation(1);
		AuPoint gl1 = ll1.GlobeLocation(1);
		AuPoint l = GlobeLocation(1);
		AuPoint segvec = gl2 - gl1;
		if(segvec.IsZero())
		{
			if(segDot)
				*segDot = 0.f;
			return ll1;
		}
		float rlen = segvec.Length();

		AuPoint locvec = l - gl1;
		float rdot = locvec.Dot(segvec)/rlen;
		float frac = rdot/rlen;
		if(segDot)
			*segDot = frac;

		if(rdot <= 0)
			return ll1;
		else if(rdot >= rlen)
			return ll2;

		AuPoint intloc = gl1 + segvec*frac;
		AuGlobeLLF ll;
		ll.SetFromGlobeLocation(intloc);
		return ll;
	}
}

QString AuGlobeLLF::DirectionTo(const AuGlobeLLF& ll, LLSHAPE shape) const
{
	float chgLong = ll.Longitude() - Longitude();
	if(shape == LL_Cylinder)
		chgLong *= 11.;			// Scaled for long skinny ringworld.
	else
	{
		// Globe can wrap around the other direction.
		if(chgLong > PIf)
			chgLong -= PIf*2;
		else if(chgLong < -PIf)
			chgLong += PIf*2;
	}

	float chgLat = ll.Latitude() - Latitude();

	float dirAng = atan2f(chgLat, chgLong);
	if(dirAng < 0)
		dirAng += PIf*2;

	if(dirAng < 11.25f*DEG2RADf)
		return QObject::tr("east");
	else if(dirAng < 33.75f*DEG2RADf)
		return QObject::tr("east-northeast");
	else if(dirAng < 56.25f*DEG2RADf)
		return QObject::tr("northeast");
	else if(dirAng < 78.75f*DEG2RADf)
		return QObject::tr("north-northeast");
	else if(dirAng < 101.25f*DEG2RADf)
		return QObject::tr("north");
	else if(dirAng < 123.75f*DEG2RADf)
		return QObject::tr("north-northwest");
	else if(dirAng < 146.25f*DEG2RADf)
		return QObject::tr("northwest");
	else if(dirAng < 168.75f*DEG2RADf)
		return QObject::tr("west-northwest");
	else if(dirAng < 191.25f*DEG2RADf)
		return QObject::tr("west");
	else if(dirAng < 213.75f*DEG2RADf)
		return QObject::tr("west-southwest");
	else if(dirAng < 236.25f*DEG2RADf)
		return QObject::tr("southwest");
	else if(dirAng < 258.75f*DEG2RADf)
		return QObject::tr("south-southwest");
	else if(dirAng < 281.25f*DEG2RADf)
		return QObject::tr("south");
	else if(dirAng < 303.75f*DEG2RADf)
		return QObject::tr("south-southeast");
	else if(dirAng < 326.25f*DEG2RADf)
		return QObject::tr("southeast");
	else if(dirAng < 348.75f*DEG2RADf)
		return QObject::tr("east-southeast");
	else
		return QObject::tr("east");
}

const AuGlobeLLF& AuGlobeLLF::FromGlobeLL(const AuGlobeLL& ll)
{
	SetLatitude((float)ll.Latitude());
	SetLongitude((float)ll.Longitude());
	return *this;
}

// Latitude and longitude values of 'c' are added to 'this' coordinate.
// Wraps longitude and latitude when they rollover the poles or meridian.
AuGlobeLLF AuGlobeLLF::operator+(const AuGlobeLLF& c) const
{
	AuGlobeLLF resultLL(*this);
	resultLL += c;
	return resultLL;
}

void AuGlobeLLF::operator<<(QDataStream& in)
{
	in >> m_Longitude;
	in >> m_Latitude;
}

void AuGlobeLLF::operator>>(QDataStream& out) const
{
	out << m_Longitude;
	out << m_Latitude;
}

QDebug operator<<(QDebug dbg, const AuGlobeLLF& ll)
{
	// Degree symbol does not work on terminal.
	dbg.nospace() << QString("%1%2 %3%4")
		.arg(fabsf(ll.Longitude()*RAD2DEGf), 0,'f',2).arg(ll.Longitude() < 0 ? "W" : "E")
		.arg(fabsf(ll.Latitude()*RAD2DEGf), 0,'f',2).arg(ll.Latitude() < 0 ? "S" : "N");
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGlobeLL::AuGlobeLL(const QVariant& v)
{
	QByteArray ba = v.toByteArray();
	QDataStream in(ba);
	in.setVersion(QDataStream::Qt_4_1);
	in >> m_Latitude;
	in >> m_Longitude;
}

QVariant AuGlobeLL::ToVariant() const
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << m_Latitude;
	out << m_Longitude;
	return QVariant(ba);
}

// Latitude and longitude values of 'c' are added to 'this' coordinate.
// Wraps longitude and latitude when they rollover the poles or meridian.
AuGlobeLL AuGlobeLL::operator+=(const AuGlobeLL& c)
{
	m_Latitude += c.m_Latitude;
	m_Longitude += c.m_Longitude;

	// Latitude rollover will not work if the wrap goes around both poles.
	if(m_Latitude > PI)
	{
		// Wrap over north pole sends longitude around to the other side.
		m_Longitude += PI;
		m_Latitude = PI*2 - m_Latitude;
	}
	else if(m_Latitude < -PI)
	{
		// Wrap over south pole sends longitude around to the other side.
		m_Longitude += PI;
		m_Latitude = -PI*2 - m_Latitude;
	}

	// Longitude rollover is easier.
	while(m_Longitude < -PI)
		m_Longitude += PI*2;
	while(m_Longitude > PI)
		m_Longitude -= PI*2;

	return *this;
}

// Returns the side of the line from 'll0' to 'll1' that this location is on.
// 1=left, 0=on, -1=right
int AuGlobeLL::SideOfLine(const AuGlobeLL& ll0, const AuGlobeLL& ll1, LLSHAPE llShape) const
{
	if(llShape == LL_Cylinder)
	{
		// Cylinder can be treated linearly in both longitude and latitude.
		double rX1 = ll0.Longitude() - Longitude();
		double rY1 = ll0.Latitude() - Latitude();
		double rX2 = ll1.Longitude() - Longitude();
		double rY2 = ll1.Latitude() - Latitude();
		double rCross = (rX1 * rY2) - (rX2 * rY1);
		if(rCross < 0)
			return -1;
		else if(rCross > 0)
			return 1;
		else
			return 0;
	}
	else
	{
		// Globe must deal with narrowing longitudes as we move closer to the poles.
		AuPoint gl0 = ll0.GlobeLocation(1);
		AuPoint gl1 = ll1.GlobeLocation(1);

		AuPoint gv(gl1 - gl0);
		//gv.Unit();

		AuPoint up((gl0 + gl1)/2);
		//up.Unit();

		AuPoint xv = up.Cross(gv);

		AuPoint l = GlobeLocation(1);
		AuPoint lv = l - gl0;
		double rdot = lv.Dot(xv);
		if(rdot < 0.)
			return -1;
		else if(rdot > 0.)
			return 1;
		else
			return 0;
	}
}

// Returns true if this ll is in the triangle formed by 'll0', 'll1', and 'll2', in any order.
// CAVEAT: Does not work on triangles that wrap across the PI/-PI line.
bool AuGlobeLL::InTriangle(const AuGlobeLL& ll0, const AuGlobeLL& ll1, const AuGlobeLL& ll2, LLSHAPE llShape) const
{
	int lSide0 = SideOfLine(ll0,ll1, llShape);
	int lSide1 = SideOfLine(ll1,ll2, llShape);
	int lSide2 = SideOfLine(ll2,ll0, llShape);
	if(lSide0 <= 0 && lSide1 <= 0 && lSide2 <= 0)
		return true;				// This point is within a clockwise triangle formed by the three locations.
	else if(lSide0 >= 0 && lSide1 >= 0 && lSide2 >= 0)
		return true;				// This point is within a counterclockwise triangle formed by the three locations.
	return false;
}

// Returns true if this ll is in the triangle formed by 'll0', 'll1', and 'll2', in clockwise order.
// CAVEAT: Does not work on triangles that wrap across the PI/-PI line.
bool AuGlobeLL::InTriangleClockwise(const AuGlobeLL& ll0, const AuGlobeLL& ll1, const AuGlobeLL& ll2, LLSHAPE llShape) const
{
	int lSide0 = SideOfLine(ll0,ll1, llShape);
	int lSide1 = SideOfLine(ll1,ll2, llShape);
	int lSide2 = SideOfLine(ll2,ll0, llShape);
	if(lSide0 <= 0 && lSide1 <= 0 && lSide2 <= 0)
		return true;				// This point is within a clockwise triangle formed by the three locations.
	return false;
}

// Find closest point on a line segment.
// If the locate point is beyond either end point of the segment, the closest end point is returned
// 'segDot' is an optional parameter to receive the ratio of distance along the segment of the intersection, in the range 0..1.
// Takes the longitudinal angle from 'll1' to 'll2' that is less than 180 degrees.
AuGlobeLL AuGlobeLL::CoordOnSegment(const AuGlobeLL& ll1, const AuGlobeLL& ll2, double* segDot, LLSHAPE llShape) const
{
	if(llShape == LL_Cylinder)
	{
		// Cylinder can be treated linearly in both longitude and latitude.
		// Calculate vector of segment.
		double segvecX = ll2.LongitudePositive() - ll1.LongitudePositive();
		if(segvecX > PI)
			segvecX -= PI*2.;
		else if(segvecX < -PI)
			segvecX += PI*2.;
		double segvecY = ll2.Latitude() - ll1.Latitude();

		// Calculate length of segment.
		double rlen = sqrt(segvecX*segvecX + segvecY*segvecY);
		if(rlen == 0.)
		{
			if(segDot)
				*segDot = 0.;
			return ll1;					// Segment length is 0.
		}

		// Calculate vector from ll1 to this location.
		double locvecX = LongitudePositive() - ll1.LongitudePositive();
		if(locvecX > PI)
			locvecX -= PI*2.;
		else if(locvecX < -PI)
			locvecX += PI*2.;
		double locvecY = Latitude() - ll1.Latitude();

		// Calculate dot product of locvec onto segvec.
		double rdot = (locvecX*segvecX + locvecY*segvecY)/rlen;
		double frac = rdot/rlen;
		if(frac < 0.)
			frac = 0.;
		else if(frac > 1.)
			frac = 1.;
		if(segDot)
			*segDot = frac;

		if(rdot <= 0.)
			return ll1;					// Beyond first segment end.
		else if(rdot >= rlen)
			return ll2;					// Beyond second end of segment.
		else
		{
			// Calculate intersection between segment points.
			double intX = ll1.LongitudePositive() + segvecX*frac;
			double intY = ll1.Latitude() + segvecY*frac;
			return AuGlobeLL(intX, intY);						// Constructor normalizes the longitude into the proper range.
		}
	}
	else
	{
		// Globe must deal with narrowing longitudes as we move closer to the poles.
		AuPoint gl2 = ll2.GlobeLocation(1);
		AuPoint gl1 = ll1.GlobeLocation(1);
		AuPoint l = GlobeLocation(1);
		AuPoint segvec = gl2 - gl1;
		if(segvec.IsZero())
		{
			if(segDot)
				*segDot = 0.;
			return ll1;
		}
		double rlen = segvec.Length();

		AuPoint locvec = l - gl1;
		double rdot = locvec.Dot(segvec)/rlen;
		double frac = rdot/rlen;
		if(segDot)
			*segDot = frac;

		if(rdot <= 0)
			return ll1;
		else if(rdot >= rlen)
			return ll2;

		AuPoint intloc = gl1 + segvec*frac;
		AuGlobeLL ll;
		ll.SetFromGlobeLocation(intloc);
		return ll;
	}
}

QString AuGlobeLL::DirectionTo(const AuGlobeLL& ll, LLSHAPE shape) const
{
	double chgLong = ll.Longitude() - Longitude();
	if(shape == LL_Cylinder)
		chgLong *= 11.;			// Scaled for long skinny ringworld.
	else
	{
		// Globe can wrap around the other direction.
		if(chgLong > PI)
			chgLong -= PI*2;
		else if(chgLong < -PI)
			chgLong += PI*2;
	}

	double chgLat = ll.Latitude() - Latitude();

	double dirAng = atan2(chgLat, chgLong);
	if(dirAng < 0)
		dirAng += PI*2;

	if(dirAng < 11.25*DEG2RAD)
		return QObject::tr("east");
	else if(dirAng < 33.75*DEG2RAD)
		return QObject::tr("east-northeast");
	else if(dirAng < 56.25*DEG2RAD)
		return QObject::tr("northeast");
	else if(dirAng < 78.75*DEG2RAD)
		return QObject::tr("north-northeast");
	else if(dirAng < 101.25*DEG2RAD)
		return QObject::tr("north");
	else if(dirAng < 123.75*DEG2RAD)
		return QObject::tr("north-northwest");
	else if(dirAng < 146.25*DEG2RAD)
		return QObject::tr("northwest");
	else if(dirAng < 168.75*DEG2RAD)
		return QObject::tr("west-northwest");
	else if(dirAng < 191.25*DEG2RAD)
		return QObject::tr("west");
	else if(dirAng < 213.75*DEG2RAD)
		return QObject::tr("west-southwest");
	else if(dirAng < 236.25*DEG2RAD)
		return QObject::tr("southwest");
	else if(dirAng < 258.75*DEG2RAD)
		return QObject::tr("south-southwest");
	else if(dirAng < 281.25*DEG2RAD)
		return QObject::tr("south");
	else if(dirAng < 303.75*DEG2RAD)
		return QObject::tr("south-southeast");
	else if(dirAng < 326.25*DEG2RAD)
		return QObject::tr("southeast");
	else if(dirAng < 348.75*DEG2RAD)
		return QObject::tr("east-southeast");
	else
		return QObject::tr("east");
}

// Latitude and longitude values of 'c' are added to 'this' coordinate.
// Wraps longitude and latitude when they rollover the poles or meridian.
AuGlobeLL AuGlobeLL::operator+(const AuGlobeLL& c) const
{
	AuGlobeLL resultLL(*this);
	resultLL += c;
	return resultLL;
}

void AuGlobeLL::operator<<(QDataStream& in)
{
	in >> m_Longitude;
	in >> m_Latitude;
}

void AuGlobeLL::operator>>(QDataStream& out) const
{
	out << m_Longitude;
	out << m_Latitude;
}

QDebug operator<<(QDebug dbg, const AuGlobeLL& ll)
{
	// Degree symbol does not work on terminal.
	dbg.nospace() << QString("%1%2 %3%4")
		.arg(fabs(ll.Longitude()*RAD2DEG), 0,'f',2).arg(ll.Longitude() < 0 ? "W" : "E")
		.arg(fabs(ll.Latitude()*RAD2DEG), 0,'f',2).arg(ll.Latitude() < 0 ? "S" : "N");
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// qHash is needed to use AuPoint in a QList in Windows.
unsigned int qHash(const AuGlobeLLTri& key)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << key.m_LL0;
	out << key.m_LL1;
	out << key.m_LL2;
	return qHash(ba);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGlobeLLTriList::CreateGlobeTriangles(int equatorSteps)
{
	// Create triangles for the globe based on longitude and latitude.
	// Each pole will have a single vertex.
	// Equator has 'equatorSteps' vertexes.
	clear();

	// Poles
	AuGlobeLL poleN(0,PI/2);									// North pole
	AuGlobeLL poleS(0,-PI/2);									// South pole

	// North and south hemisphere except poles and equator.
	int latdivshemi = equatorSteps/4;							// Latitude divisions per hemisphere.
	if(latdivshemi < 2)
		latdivshemi = 2;
	double latDivAng = (PI/2) / latdivshemi;					// Radian angle per latitude division
	QList<AuGlobeLL> lastRowN, lastRowS;						// Previous rows of north and south globe locations.
	for(int latinc=1; latinc<latdivshemi; latinc++)
	{
		// We can do both hemispheres at once.
		double latitude = PI/2 - latDivAng * latinc;			// Range is PI/2 to 0.
		int londivs = (int)ceilf(cosf(latitude) * equatorSteps);
		if(londivs < 3)
			londivs = 3;										// Need at least 3 to draw triangles.
		double lonDivAng = (PI*2) / londivs;					// Radian angle per longitude division
		QList<AuGlobeLL> rowN, rowS;							// Current rows of north and south globe locations.
		AuGlobeLL lastN, lastS;
		int lastRowIdx=0;
		for(int loninc=0; loninc<=londivs; loninc++)
		{
			double longitude;
			if(loninc == londivs)
				longitude = -PI;								// For tight closure we don't calculate that last longitude.
			else
				longitude = lonDivAng * loninc - PI;			// Range is -PI to PI.
			AuGlobeLL llN(longitude, latitude);					// North hemisphere
			AuGlobeLL llS(longitude, -latitude);				// South hemisphere

			if(loninc)
			{
				// Not the first location at this latitude so we can draw a triangle.
				if(latinc == 1)
				{
					// Tessellate the first row to the poles.
					append(AuGlobeLLTri(poleN, llN, lastN));
					append(AuGlobeLLTri(poleS, lastS, llS));
				}
				else
				{
					// Tessellate subsequent row to the previous row.
					append(AuGlobeLLTri(lastRowN[lastRowIdx], llN, lastN));
					append(AuGlobeLLTri(lastRowS[lastRowIdx], lastS, llS));

					if(longitude >= lastRowN[lastRowIdx].Longitude() || loninc == londivs)
					{
						// We caught up to the vertex on the last row. Draw a triangle to that vertex to close the triangles up to
						// there and bump the previous row index.
						int nextLastRowIdx = lastRowIdx+1;
						if(nextLastRowIdx >= lastRowN.count())
							nextLastRowIdx = 0;					// Wrap to beginning of last row.
						append(AuGlobeLLTri(lastRowN[lastRowIdx], lastRowN[nextLastRowIdx], llN));
						append(AuGlobeLLTri(lastRowS[lastRowIdx], llS, lastRowS[nextLastRowIdx]));
						lastRowIdx = nextLastRowIdx;
					}
				}
			}
			lastN = llN;
			lastS = llS;
			rowN.append(llN);
			rowS.append(llS);
		}
		lastRowN = rowN;
		lastRowS = rowS;
	}

	// Equator
	int londivs = equatorSteps;
	double lonDivAng = (PI*2) / londivs;						// Radian angle per longitude division
	AuGlobeLL lastE;
	int lastRowIdx=0;
	for(int loninc=0; loninc<=londivs; loninc++)
	{
		double longitude;
		if(loninc == londivs)
			longitude = -PI;									// For tight closure we don't calculate that last longitude.
		else
			longitude = lonDivAng * loninc - PI;				// Range is -PI to PI.
		AuGlobeLL llE(longitude, 0);							// Equator

		if(loninc)
		{
			// Not the first location at the equator so we can draw a triangle.
			// Tessellate equator row to the previous row.
			append(AuGlobeLLTri(lastRowN[lastRowIdx], llE, lastE));
			append(AuGlobeLLTri(lastRowS[lastRowIdx], lastE, llE));

			if(longitude >= lastRowN[lastRowIdx].Longitude() || loninc == londivs)
			{
				// We caught up to the vertex on the last row. Draw a triangle to that vertex to close the triangles up to
				// there and bump the previous row index.
				int nextLastRowIdx = lastRowIdx+1;
				if(nextLastRowIdx >= lastRowN.count())
					nextLastRowIdx = 0;					// Wrap to beginning of last row.
				append(AuGlobeLLTri(lastRowN[lastRowIdx], lastRowN[nextLastRowIdx], llE));
				append(AuGlobeLLTri(lastRowS[lastRowIdx], llE, lastRowS[nextLastRowIdx]));
				lastRowIdx = nextLastRowIdx;
			}
		}
		lastE = llE;
	}
}
