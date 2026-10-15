//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auglobell.h,v 1.28 2022/02/19 18:37:58 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGlobeLL_H
#define AuGlobeLL_H

#include "aupoint.h"
#include <QDebug>
#include <QVariant>

class AuGlobeLL;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Perhaps misnamed, the AuGlobeLL objects are used for non-globes such as ringworlds. This affects linear surface calculations.

enum LLSHAPE
{
	LL_Globe,							// For true globes.
	LL_Cylinder,						// For ringworld.
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUMATH_DLL AuGlobeLLF
{
private:
	float								m_Latitude;				// Latitude angle in radians, in the range -PI/2 to PI/2.
	float								m_Longitude;			// Longitude angle in radians, in the range -PI to PI.

public:
	AuGlobeLLF()
	{
		m_Latitude = 0;
		m_Longitude = 0;
	}

	AuGlobeLLF(float longitude, float latitude)
	{
		SetLatitude(latitude);
		SetLongitude(longitude);
	}

	AuGlobeLLF(const QVariant& v);

	inline static QString DegreeSymbol()
	{
#if defined(Q_OS_WIN)
		return QString(QChar(0xb0));
#else
#if defined(UNICODE)
		return QString("°");
#else
		return QString::fromLocal8Bit("°");
#endif
#endif
	}

	// Compares this to 'll', ignoring the signs.
	// Returns true if we match.
	inline bool CompareIgnoreSign(const AuGlobeLLF& ll) const
	{
		if(fabsf(m_Latitude) != fabsf(ll.m_Latitude))
			return false;
		if(fabsf(m_Longitude) != fabsf(ll.m_Longitude))
			return false;
		return true;
	}

	// This is a coarse generalization, not exact.
	QString DirectionTo(const AuGlobeLLF& ll, LLSHAPE shape) const;

	const AuGlobeLLF& FromGlobeLL(const AuGlobeLL& ll);

	inline bool FuzzyCompare(const AuGlobeLLF& ll) const
	{
		if(!FuzzyEquals(m_Latitude, ll.m_Latitude))
			return false;
		if(!FuzzyEquals(m_Longitude, ll.m_Longitude))
			return false;
		return true;
	}

	inline bool FuzzyCompareIgnoreSign(const AuGlobeLLF& ll) const
	{
		if(!FuzzyEquals(fabsf(m_Latitude), fabsf(ll.m_Latitude)))
			return false;
		if(!FuzzyEquals(fabsf(m_Longitude), fabsf(ll.m_Longitude)))
			return false;
		return true;
	}

	// 'altitude' is above the center of the globe
	inline AuPoint GlobeLocation(double altitude) const
	{
		AuPoint loc;
		loc.PolarValue(altitude, m_Longitude, m_Latitude);
		return loc;
	}

	// Returns true if this ll is in the triangle formed by 'll0', 'll1', and 'll2', in any order.
	// CAVEAT: Does not work on triangles that wrap across the PI/-PI line.
	bool InTriangle(const AuGlobeLLF& ll0, const AuGlobeLLF& ll1, const AuGlobeLLF& ll2, LLSHAPE llShape) const;

	// Returns true if this ll is in the triangle formed by 'll0', 'll1', and 'll2', in clockwise order.
	// CAVEAT: Does not work on triangles that wrap across the PI/-PI line.
	bool InTriangleClockwise(const AuGlobeLLF& ll0, const AuGlobeLLF& ll1, const AuGlobeLLF& ll2, LLSHAPE llShape) const;

	// true is returned if 'longLat' can be parsed into valid longitude and latitude values.
	inline static bool IsLongLatStringValid(const QString& longLat)
	{
		float longitude, latitude;
		return ParseLongitudeLatitude(longLat, longitude, latitude);
	}

	inline float Latitude() const
	{
		return m_Latitude;
	}

	inline float Longitude() const
	{
		return m_Longitude;
	}

	inline float LongitudePositive() const
	{
		if(m_Longitude >= 0)
			return m_Longitude;
		else
			return m_Longitude + PIf*2;
	}

	inline bool operator==(const AuGlobeLLF& c) const
	{
		return m_Latitude == c.m_Latitude && m_Longitude == c.m_Longitude;
	}

	inline bool operator!=(const AuGlobeLLF& c) const
	{
		return m_Latitude != c.m_Latitude || m_Longitude != c.m_Longitude;
	}

	// Latitude and longitude values of 'c' are added to 'this' coordinate.
	// Wraps longitude and latitude when they rollover the poles or meridian.
	// CAVEAT: Latitude rollover will produce incorrect results when adding/subtracting enough to wrap over two poles.
	AuGlobeLLF operator+(const AuGlobeLLF& c) const;

	// Latitude and longitude values of 'c' are added to 'this' coordinate.
	// Wraps longitude and latitude when they rollover the poles or meridian.
	// CAVEAT: Latitude rollover will produce incorrect results when adding/subtracting enough to wrap over two poles.
	AuGlobeLLF operator+=(const AuGlobeLLF& c);

	void operator<<(QDataStream& in);
	void operator>>(QDataStream& out) const;

	bool operator<(const AuGlobeLLF& c) const
	{
		// Compare latitude.
		if(FuzzyLessThan(m_Latitude, c.m_Latitude))
			return true;										// this point is at a lower latitude.
		else if(FuzzyGreaterThan(m_Latitude, c.m_Latitude))
			return false;										// this point is at a higher latitude.

		// this point is at the same latitude as the comparison point
		// Compare longitude.
		if(FuzzyLessThan(m_Longitude, c.m_Longitude))
			return true;										// this point is at a lower longitude.
		return false;											// this point is at the same or higher longitude
	}

	// 'longitude' and 'latitude' are set to the signed radian values parsed from the 'longLat' string.
	// true is returned if 'longLat' can be parsed into 'longitude' and 'latitude' values.
	// 'longitude' and 'latitude' are unchanged on failure.
	inline bool ParseLongitudeLatitude(const QString& longLat)
	{
		return ParseLongitudeLatitude(longLat, m_Longitude, m_Latitude);
	}

	// 'longitude' and 'latitude' are set to the signed radian values parsed from the 'longLat' string.
	// true is returned if 'longLat' can be parsed into 'longitude' and 'latitude' values.
	// 'longitude' and 'latitude' are unchanged on failure.
	inline static bool ParseLongitudeLatitude(const QString& longLat, float& longitude, float& latitude)
	{
		QStringList fields = longLat.split(' ');
		if(fields.count() != 2 || fields[0].count() < 2 || fields[1].count() < 2)
			return false;

		// Determine sign of longitude value.
		int ewsign = 0;
		if(fields[0].right(1) == QChar('E') || fields[0].right(1) == QChar('e'))
			ewsign = 1;		// east is positive
		else if(fields[0].right(1) == QChar('W') || fields[0].right(1) == QChar('w'))
			ewsign = -1;	// west is negative
		else
			return false;	// Incorrect EW designator.

		// Determine sign of latitude value.
		int nssign = 0;
		if(fields[1].right(1) == QChar('N') || fields[1].right(1) == QChar('n'))
			nssign = 1;		// north is positive
		else if(fields[1].right(1) == QChar('S') || fields[1].right(1) == QChar('s'))
			nssign = -1;	// south is negative
		else
			return false;	// Incorrect NS designator.

		// Extract the longitude value.
		int lonvalidx = fields[0].indexOf(QRegExp("[EeWw]"));
		if(lonvalidx < 0)
			return false;
		lonvalidx = fields[0].indexOf(QRegExp("[EeWw°]"));
		float lon = QVariant(fields[0].left(lonvalidx)).toFloat();
		if(lon < 0 || lon > 180)
			return false;	// Invalid longitude value

		// Extract the latitude value.
		int latvalidx = fields[1].indexOf(QRegExp("[NnSs]"));
		if(latvalidx < 0)
			return false;
		latvalidx = fields[1].indexOf(QRegExp("[NnSs°]"));
		float lat = QVariant(fields[1].left(latvalidx)).toFloat();
		if(lat < 0 || lat > 90)
			return false;	// Invalid latitude value

		// Calculate the longitude latitude in signed radian values.
		longitude = lon * DEG2RADf * ewsign;
		latitude = lat * DEG2RADf * nssign;
		return true;
	}

	// Find closest point on a line segment.
	// If the locate point is beyond either end point of the segment, the closest end point is returned
	// 'segDot' is an optional parameter to receive the ratio of distance along the segment of the intersection, in the range 0..1.
	// Takes the longitudinal angle from 'll1' to 'll2' that is less than 180 degrees.
	AuGlobeLLF CoordOnSegment(const AuGlobeLLF& ll1, const AuGlobeLLF& ll2, float* segDot, LLSHAPE llShape) const;

	static int RegisterMetaType();

	// Sets the longitude and latitude based on a location relative to a globe, regardless of the altitude of 'globeLoc'.
	inline void SetFromGlobeLocation(const AuPoint& globeLoc)
	{
		m_Longitude = globeLoc.ArcTangent_2D();					// Negative longitude is west.
		m_Latitude = globeLoc.PolarIncline();					// Negative latitude is south.
	}

	// Sets the latitude angle. Clamps it to the range of -PI/2 to PI/2.
	inline void SetLatitude(float latitude)
	{
		m_Latitude = latitude;
		if(fabsf(latitude) > PIf/2)
		{
			if(m_Latitude < -PIf/2)
				m_Latitude = -PIf/2;
			else if(m_Latitude > PIf/2)
				m_Latitude = PIf/2;
		}
	}

	// Sets the longitude angle. Wraps it to the range of -PI to PI.
	inline void SetLongitude(float longitude)
	{
		m_Longitude = UnwindAngleF(longitude);
	}

	// Returns the side of the line from 'll0' to 'll1' that this location is on.
	// 1=left, 0=on, -1=right
	int SideOfLine(const AuGlobeLLF& ll0, const AuGlobeLLF& ll1, LLSHAPE llShape) const;

	inline QString ToString() const
	{
		return ToString(m_Longitude, m_Latitude);
	}

	// Formats longitude and latitude angles into a string. They are expected to be in radians.
	// 'longitude' is negative to the west and positive to the east, which matches AuPoint::ArcTangent_2D.
	// 'latitude' is negative to the north and positive to the south, which matches AuPoint::PolarIncline.
	inline static QString ToString(float longitude, float latitude)
	{
		return QString("%1%2%3 %4%5%6")
			.arg(fabsf(longitude*RAD2DEGf), 0,'f',2).arg(DegreeSymbol()).arg(longitude < 0 ? "W" : "E")
			.arg(fabsf(latitude*RAD2DEGf), 0,'f',2).arg(DegreeSymbol()).arg(latitude < 0 ? "S" : "N");
	}

	QVariant ToVariant() const;
};

Q_DECLARE_METATYPE(AuGlobeLLF)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

inline AUMATH_DLL QDataStream& operator<<(QDataStream& out, const AuGlobeLLF& ll)
{
	ll >> out;
	return out;
}

inline AUMATH_DLL QDataStream& operator>>(QDataStream& in, AuGlobeLLF& ll)
{
	ll << in;
	return in;
}

AUMATH_DLL QDebug operator<<(QDebug dbg, const AuGlobeLLF& ll);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUMATH_DLL AuGlobeLL
{
private:
	double								m_Latitude;				// Latitude angle in radians, in the range -PI/2 to PI/2.
	double								m_Longitude;			// Longitude angle in radians, in the range -PI to PI.

public:
	AuGlobeLL()
	{
		m_Latitude = 0;
		m_Longitude = 0;
	}

	AuGlobeLL(double longitude, double latitude)
	{
		SetLatitude(latitude);
		SetLongitude(longitude);
	}

	AuGlobeLL(const QVariant& v);

	inline static QString DegreeSymbol()
	{
#if defined(Q_OS_WIN)
		return QString(QChar(0xb0));
#else
#if defined(UNICODE)
		return QString("°");
#else
		return QString::fromLocal8Bit("°");
#endif
#endif
	}

	// Compares this to 'll', ignoring the signs.
	// Returns true if we match.
	inline bool CompareIgnoreSign(const AuGlobeLL& ll) const
	{
		if(fabs(m_Latitude) != fabs(ll.m_Latitude))
			return false;
		if(fabs(m_Longitude) != fabs(ll.m_Longitude))
			return false;
		return true;
	}

	// This is a coarse generalization, not exact.
	QString DirectionTo(const AuGlobeLL& ll, LLSHAPE shape) const;

	inline const AuGlobeLL& FromGlobeLLF(const AuGlobeLLF& llf)
	{
		SetLatitude(llf.Latitude());
		SetLongitude(llf.Longitude());
		return *this;
	}

	inline bool FuzzyCompare(const AuGlobeLL& ll) const
	{
		if(!FuzzyEquals(m_Latitude, ll.m_Latitude))
			return false;
		if(!FuzzyEquals(m_Longitude, ll.m_Longitude))
			return false;
		return true;
	}

	inline bool FuzzyCompareIgnoreSign(const AuGlobeLL& ll) const
	{
		if(!FuzzyEquals(fabs(m_Latitude), fabs(ll.m_Latitude)))
			return false;
		if(!FuzzyEquals(fabs(m_Longitude), fabs(ll.m_Longitude)))
			return false;
		return true;
	}

	// 'altitude' is above the center of the globe
	inline AuPoint GlobeLocation(double altitude) const
	{
		AuPoint loc;
		loc.PolarValue(altitude, m_Longitude, m_Latitude);
		return loc;
	}

	// Returns true if this ll is in the triangle formed by 'll0', 'll1', and 'll2', in any order.
	// CAVEAT: Does not work on triangles that wrap across the PI/-PI line.
	bool InTriangle(const AuGlobeLL& ll0, const AuGlobeLL& ll1, const AuGlobeLL& ll2, LLSHAPE llShape) const;

	// Returns true if this ll is in the triangle formed by 'll0', 'll1', and 'll2', in clockwise order.
	// CAVEAT: Does not work on triangles that wrap across the PI/-PI line.
	bool InTriangleClockwise(const AuGlobeLL& ll0, const AuGlobeLL& ll1, const AuGlobeLL& ll2, LLSHAPE llShape) const;

	// true is returned if 'longLat' can be parsed into valid longitude and latitude values.
	inline static bool IsLongLatStringValid(const QString& longLat)
	{
		double longitude, latitude;
		return ParseLongitudeLatitude(longLat, longitude, latitude);
	}

	inline double Latitude() const
	{
		return m_Latitude;
	}

	inline double Longitude() const
	{
		return m_Longitude;
	}

	inline double LongitudePositive() const
	{
		if(m_Longitude >= 0)
			return m_Longitude;
		else
			return m_Longitude + PI*2;
	}

	inline bool operator==(const AuGlobeLL& c) const
	{
		return m_Latitude == c.m_Latitude && m_Longitude == c.m_Longitude;
	}

	inline bool operator!=(const AuGlobeLL& c) const
	{
		return m_Latitude != c.m_Latitude || m_Longitude != c.m_Longitude;
	}

	// Latitude and longitude values of 'c' are added to 'this' coordinate.
	// Wraps longitude and latitude when they rollover the poles or meridian.
	// CAVEAT: Latitude rollover will produce incorrect results when adding/subtracting enough to wrap over two poles.
	AuGlobeLL operator+(const AuGlobeLL& c) const;

	// Latitude and longitude values of 'c' are added to 'this' coordinate.
	// Wraps longitude and latitude when they rollover the poles or meridian.
	// CAVEAT: Latitude rollover will produce incorrect results when adding/subtracting enough to wrap over two poles.
	AuGlobeLL operator+=(const AuGlobeLL& c);

	void operator<<(QDataStream& in);
	void operator>>(QDataStream& out) const;

	bool operator<(const AuGlobeLL& c) const
	{
		// Compare latitude.
		if(FuzzyLessThan(m_Latitude, c.m_Latitude))
			return true;										// this point is at a lower latitude.
		else if(FuzzyGreaterThan(m_Latitude, c.m_Latitude))
			return false;										// this point is at a higher latitude.

		// this point is at the same latitude as the comparison point
		// Compare longitude.
		if(FuzzyLessThan(m_Longitude, c.m_Longitude))
			return true;										// this point is at a lower longitude.
		return false;											// this point is at the same or higher longitude
	}

	// 'longitude' and 'latitude' are set to the signed radian values parsed from the 'longLat' string.
	// true is returned if 'longLat' can be parsed into 'longitude' and 'latitude' values.
	// 'longitude' and 'latitude' are unchanged on failure.
	inline bool ParseLongitudeLatitude(const QString& longLat)
	{
		return ParseLongitudeLatitude(longLat, m_Longitude, m_Latitude);
	}

	// 'longitude' and 'latitude' are set to the signed radian values parsed from the 'longLat' string.
	// true is returned if 'longLat' can be parsed into 'longitude' and 'latitude' values.
	// 'longitude' and 'latitude' are unchanged on failure.
	inline static bool ParseLongitudeLatitude(const QString& longLat, double& longitude, double& latitude)
	{
		QStringList fields = longLat.split(' ');
		if(fields.count() != 2 || fields[0].count() < 2 || fields[1].count() < 2)
			return false;

		// Determine sign of longitude value.
		int ewsign = 0;
		if(fields[0].right(1) == QChar('E') || fields[0].right(1) == QChar('e'))
			ewsign = 1;		// east is positive
		else if(fields[0].right(1) == QChar('W') || fields[0].right(1) == QChar('w'))
			ewsign = -1;	// west is negative
		else
			return false;	// Incorrect EW designator.

		// Determine sign of latitude value.
		int nssign = 0;
		if(fields[1].right(1) == QChar('N') || fields[1].right(1) == QChar('n'))
			nssign = 1;		// north is positive
		else if(fields[1].right(1) == QChar('S') || fields[1].right(1) == QChar('s'))
			nssign = -1;	// south is negative
		else
			return false;	// Incorrect NS designator.

		// Extract the longitude value.
		int lonvalidx = fields[0].indexOf(QRegExp("[EeWw]"));
		if(lonvalidx < 0)
			return false;
		lonvalidx = fields[0].indexOf(QRegExp("[EeWw°]"));
		double lon = QVariant(fields[0].left(lonvalidx)).toDouble();
		if(lon < 0 || lon > 180)
			return false;	// Invalid longitude value

		// Extract the latitude value.
		int latvalidx = fields[1].indexOf(QRegExp("[NnSs]"));
		if(latvalidx < 0)
			return false;
		latvalidx = fields[1].indexOf(QRegExp("[NnSs°]"));
		double lat = QVariant(fields[1].left(latvalidx)).toDouble();
		if(lat < 0 || lat > 90)
			return false;	// Invalid latitude value

		// Calculate the longitude latitude in signed radian values.
		longitude = lon * DEG2RAD * ewsign;
		latitude = lat * DEG2RAD * nssign;
		return true;
	}

	// Find closest point on a line segment.
	// If the locate point is beyond either end point of the segment, the closest end point is returned
	// 'segDot' is an optional parameter to receive the ratio of distance along the segment of the intersection, in the range 0..1.
	// Takes the longitudinal angle from 'll1' to 'll2' that is less than 180 degrees.
	AuGlobeLL CoordOnSegment(const AuGlobeLL& ll1, const AuGlobeLL& ll2, double* segDot, LLSHAPE llShape) const;

	static int RegisterMetaType();

	// Sets the longitude and latitude based on a location relative to a globe, regardless of the altitude of 'globeLoc'.
	inline void SetFromGlobeLocation(const AuPoint& globeLoc)
	{
		m_Longitude = globeLoc.ArcTangent_2D();					// Negative longitude is west.
		m_Latitude = globeLoc.PolarIncline();					// Negative latitude is south.
	}

	// Sets the latitude angle. Clamps it to the range of -PI/2 to PI/2.
	inline void SetLatitude(double latitude)
	{
		m_Latitude = latitude;
		if(fabs(latitude) > PI/2)
		{
			if(m_Latitude < -PI/2)
				m_Latitude = -PI/2;
			else if(m_Latitude > PI/2)
				m_Latitude = PI/2;
		}
	}

	// Sets the longitude angle. Wraps it to the range of -PI to PI.
	inline void SetLongitude(double longitude)
	{
		m_Longitude = UnwindAngle(longitude);
	}

	// Returns the side of the line from 'll0' to 'll1' that this location is on.
	// 1=left, 0=on, -1=right
	int SideOfLine(const AuGlobeLL& ll0, const AuGlobeLL& ll1, LLSHAPE llShape) const;

	inline AuGlobeLLF ToLLF() const
	{
		return AuGlobeLLF((float)m_Longitude, (float)m_Latitude);
	}

	inline QString ToString() const
	{
		return ToString(m_Longitude, m_Latitude);
	}

	// Formats longitude and latitude angles into a string. They are expected to be in radians.
	// 'longitude' is negative to the west and positive to the east, which matches AuPoint::ArcTangent_2D.
	// 'latitude' is negative to the north and positive to the south, which matches AuPoint::PolarIncline.
	inline static QString ToString(double longitude, double latitude)
	{
		return QString("%1%2%3 %4%5%6")
			.arg(fabs(longitude*RAD2DEG), 0,'f',2).arg(DegreeSymbol()).arg(longitude < 0 ? "W" : "E")
			.arg(fabs(latitude*RAD2DEG), 0,'f',2).arg(DegreeSymbol()).arg(latitude < 0 ? "S" : "N");
	}

	QVariant ToVariant() const;
};

Q_DECLARE_METATYPE(AuGlobeLL)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

inline AUMATH_DLL QDataStream& operator<<(QDataStream& out, const AuGlobeLL& ll)
{
	ll >> out;
	return out;
}

inline AUMATH_DLL QDataStream& operator>>(QDataStream& in, AuGlobeLL& ll)
{
	ll << in;
	return in;
}

AUMATH_DLL QDebug operator<<(QDebug dbg, const AuGlobeLL& ll);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUMATH_DLL AuGlobeLLTri
{
public:
	AuGlobeLL							m_LL0;
	AuGlobeLL							m_LL1;
	AuGlobeLL							m_LL2;
	
public:
	inline AuGlobeLLTri(const AuGlobeLL& ll0, const AuGlobeLL& ll1, const AuGlobeLL& ll2)
	{
		m_LL0 = ll0;
		m_LL1 = ll1;
		m_LL2 = ll2;
	}

	inline AuGlobeLLTri()
	{
	}

	inline bool operator==(const AuGlobeLLTri& c) const
	{
		return m_LL0 == c.m_LL0 && m_LL1 == c.m_LL1 && m_LL2 == c.m_LL2;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_LL0;
		in >> m_LL1;
		in >> m_LL2;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_LL0;
		out << m_LL1;
		out << m_LL2;
	}
};

inline AUMATH_DLL QDataStream& operator<<(QDataStream& out, const AuGlobeLLTri& o)
{
	o >> out;
	return out;
}

inline AUMATH_DLL QDataStream& operator>>(QDataStream& in, AuGlobeLLTri& o)
{
	o << in;
	return in;
}

// qHash is needed to use AuGlobeLLTri in a QList in Windows.
AUMATH_DLL unsigned int qHash(const AuGlobeLLTri& key);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUMATH_DLL AuGlobeLLTriList : public QList<AuGlobeLLTri>
{
public:	// Methods
	void CreateGlobeTriangles(int equatorSteps);
};

#endif // AuGlobeLL_H
