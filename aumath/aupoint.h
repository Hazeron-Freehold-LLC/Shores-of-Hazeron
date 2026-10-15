//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aupoint.h,v 1.85 2022/02/19 18:38:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuPoint_H
#define AuPoint_H

#include "aumath.h"
#include <QDataStream>
#include <QDebug>
#include <QMetaType>
#include <QPoint>
#include <QPointF>
#include <QVector>

class AuGlobeLL;
class AuMatrix3x3;
class AuMatrix4x4;
class AuPlane;

//  Summary:
//		3-dimensional floating point point/vector value.
//      
//  Description:
//		The AuPoint class is used to store 3-dimensional locations in the CAD Drawing and to perform vector math functions including
//		scaling and rotating.
//      
//  See Also:
//      
class AUMATH_DLL AuPoint
{
public:
	// Quadrant directions.
	// Left = +X axis direction.
	// Right = -X axis direction.
	// Front = +Y axis direction.
	// Back = -Y axis direction.
	// Top = +Z axis direction.
	// Bottom = -Z axis direction.
	enum DIRECTION {DIR_Front, DIR_Back, DIR_Left, DIR_Right, DIR_Top, DIR_Bottom};

	// Returns a reference to a static constructed AuPoint that can be used for validity testing.
	// Don't serialize this value.
	static const AuPoint& Invalid();

	// Returns a reference to a static default constructed AuPoint.
	static const AuPoint& Null();
	
public:
	double		x;	// floating point value of the AuPoint x member variable
	double		y;	// floating point value of the AuPoint y member variable
	double		z;	// floating point value of the AuPoint z member variable

public:
	
	//  Summary:
	//		Constructs a AuPoint initialized to abosolute zero.
	//      
	//  Description:
	//		Constructs a AuPoint with the x,y, and z, member variable initialized to zero.
	//      
	//	Example:
	//		AuPoint p1;
	//
	inline AuPoint()
	{
		x=0;
		y=0;
		z=0;
	}
	
	//  Summary:
	//		Constructs a AuPoint by passing specific x and y values and an optional z value.
	//      
	//  Parameters:
	//		i - floating point value to be assigned to the x member variable the AuPoint
	//		j - floating point value to be assigned to the y member variable the AuPoint
	//		k -  floating point value to be assigned to the z member variable the AuPoint	
	//      
	//  Description:
	//		Constructs a AuPoint by passing the x and y values as parameters i and j respectively.  The z-component of
	//		the resulting AuPoint can optionally be passed as the third parameter, k.  If the third parameter is not
	//		passed, the resulting AuPoint z member variable will be initialized with the value of 0.0.
	//      
	//	Example:
	//		AuPoint p1(rXval, rYval, rZval);
	inline AuPoint(double i, double j, double k=0.0)
	{
		x = i;
		y = j;
		z = k;
	}

	inline AuPoint(const AuPoint& c)
	{
		x = c.x;
		y = c.y;
		z = c.z;
	}

	//  Summary:
	//		Constructs a AuPoint by passing a reference to an existing AuPoint.
	//      
	//  Parameters:
	//		&p -	Reference to an existing AuPoint
	//      
	//  Description:
	//		Constructs a AuPoint by passing a reference to an existing AuPoint.  The x, y, and z, member variables
	//		of the new AuPoint will be initialized to their respective x, y, and z, values of the AuPoint referenced by &p.
	//      
	//	Example:
	//		AuPoint p1(pExist);
	inline AuPoint(const QPoint& p)
	{
		x = (double)p.x();
		y = (double)p.y();
		z = 0.0;
	}

	inline AuPoint(const QPointF& p)
	{
		x = (double)p.x();
		y = (double)p.y();
		z = 0.0;
	}

	//  Summary:
	//		Constructs a AuPoint by passing a string representing a point value.
	//      
	//  Parameters:
	//		&sText -	Reference to a QString representing a point value.
	//
	//  Description:
	//		Constructs a AuPoint by passing a string representing a point value.
	//		Parses &sText and sets 'this' point to the resulting point value.  Parsing is based on the
	//		separation of values by commas and interpretation of values using atof.
	//     
	//	Example:
	//      QString sFormattedPoint("1.2, 3.4, 5.67");
	//		AuPoint p1(sFormattedPoint);
	AuPoint(const QString& sText)
	{
		StringValue(sText);
	}

	inline AuPoint(const AuPoint& loc, const AuPoint& offset, double rotateZ)
	{
		*this = loc.RotateZ(rotateZ) + offset;
	}
	
	inline AuPoint(double i, double j, double k, const AuPoint& offset, double rotateZ)
	{
		*this = AuPoint(i, j, k).RotateZ(rotateZ) + offset;
	}
	
	// Quadrant directions.
	// The algorithm is simply based on magnitude.
	inline DIRECTION Direction() const
	{
		if(fabs(x) > fabs(y) && fabs(x) > fabs(z))
		{
			if(x > 0)
				return DIR_Right;
			else
				return DIR_Left;
		}
		else if(fabs(y) > fabs(z))
		{
			if(y > 0)
				return DIR_Front;
			else
				return DIR_Back;
		}
		else
		{
			if(z > 0)
				return DIR_Top;
			else
				return DIR_Bottom;
		}
	}

	inline AuPoint LinearInterpolate(const AuPoint& o, double frac) const
	{
		return *this * (1-frac) + o * frac;
	}

	inline double ManhattanLength() const
	{
		return fabs(x) + fabs(y) + fabs(z);
	}

	inline double ManhattanLength_2D() const
	{
		return fabs(x) + fabs(y);
	}

	//  Summary:
	//		Sets this point from polar coordinate values.
	//      
	//  Parameters:
	//		rLength - 3D distance (measured in database units) from absolute zero to this point location. 
	//		rAngle - Angle (measured in radians) that this point radiates from the Z-axis along the X-Y plane.
	//		rIncline -	Angle (measured in radians) that this point deviates above(+) or below(-) the X-Y plane.
	//
	//  Description:
	//		Sets this point based on an angle of inclination above or below the X-Y plane, a direction around the Z-axis, and a length.
	//      
	//	Example:
	//		m_fVector.PolarValue(rLen,rAngle,rIncline);
	//
	inline void PolarValue(const double rLength,const double rAngle,const double rIncline)
	{
		z = sin(rIncline) * rLength;
		double rLen2D = cos(rIncline) * rLength;
		y = sin(rAngle) * rLen2D;
		x = cos(rAngle) * rLen2D;
	}

	inline bool RoundedEquals(const AuPoint& c, double round) const
	{
		return Rounded(round) == c.Rounded(round);
	}

	inline bool RoundedFuzzyEquals(const AuPoint& c, double round) const
	{
		return Rounded(round).FuzzyEquals(c.Rounded(round));
	}

	//  Summary:
	//		Sets &newaxis to be a unit vector perpendicular to 'this' unit vector.
	// The AutoCAD arbitrary axis algorithm.
	// Sets 'newaxis' to be a unit vector that is perpendicular to 'this' unit vector.
	// e.g.
	// AuPoint vN(0,0,1);
	// AuPoint vX;
	// vN.ArbitraryAxis(vX)
	// vX == AuPoint(1,0,0)
	//      
	//  Parameters:
	//		&newaxis - Reference to a AuPoint which will contain the resulting unit vector.
	//
	//  Return Value:
	//		Returns true if the operation was successful, else false.
	//      
	//  Description:
	//		Creates an arbitrary unit vector perpendicular to 'this' unit vector.  The AuPoint reference, &newaxis, will
	//		contain the resulting unit vector.
	//      
	//  Example:
	//		AuPoint vline = pe - ps;
	//		vline.Unit();
	//		AuPoint vside;
	//		vline.ArbitraryAxis(vside);
	//      
	inline bool ArbitraryAxis(AuPoint& newaxis) const
	{
		if(fabs(x) < 1./64. && fabs(y) < 1./64.)
		{
			// This vector is vertical, more or less.
			AuPoint y(0,1,0);
			newaxis = y.Cross(*this);
		}
		else
		{
			// This vector is not vertical; we can safely cross with 0,0,1.
			AuPoint z(0,0,1);
			newaxis = z.Cross(*this);
		}
		if(newaxis.IsZero())
			return false;
		newaxis.Unit();
		return true;
	}

	inline bool ArbitraryAxisX(AuPoint& axisX) const
	{
		if(fabs(x) < 1./64. && fabs(y) < 1./64.)
		{
			// This vector is vertical, more or less.
			AuPoint y(0,1,0);
			axisX = y.Cross(*this);
		}
		else
		{
			// This vector is not vertical; this Y axis can safely cross with 0,0,1 to make an X axis.
			AuPoint z(0,0,1);
			axisX = Cross(z);
		}
		if(axisX.IsZero())
			return false;
		axisX.Unit();
		return true;
	}

	//  Summary:
	//      Calculates 3D rotation value from two plane vectors.
	//  Parameters:
	//      vx - Plane X vector. Doesn't have to be unit.
	//      vy - Plane Y vector. Doesn't have to be unit.
	//  Return Value:
	//      Returns a 3D rotation value.
	//  Description:
	//      Calculates the 3D rotation required to rotate (1,0,0) to match vx and (0,1,0) to match vy.
	static AuPoint CalculateRotation(const AuPoint &vx, const AuPoint &vy);

	//  Summary:
	//      Calculates 3D rotation value from one plane normal vector.
	//  Parameters:
	//      vn - Plane Z vector. Doesn't have to be unit.
	//  Return Value:
	//      Returns a 3D rotation value.
	//  Description:
	//      Calculates the 3D rotation required to rotate (0,0,1) to match vn.
	inline static AuPoint CalculateRotation(const AuPoint& vn)
	{
		// Insure vn is a unit vector.
		AuPoint vnu(vn);
		if(vnu.IsZero())
			vnu = AuPoint(0,0,1);
		else
			vnu.Unit();

		// Determine consistent X and Y axes from unit normal.
		AuPoint vy;
		vnu.ArbitraryAxis(vy);
		AuPoint vx(vy.Cross(vnu));

		// Return 3D rotation.
		return CalculateRotation(vx, vy);
	}
	
	//  Summary:
	//		Sets this point as a point on the arc defined by fBeg, fEnd and rBulge.
	// Create a rimpoint on an arc defined by 2 points and an AutoCAD bulge factor. 
	// the AutoCAD bulge factor is tan(0.25 * included angle) where
	// a positive sign is clockwise and a negative sign is counter clockwise
	//      
	//  Parameters:
	//		&fBeg - start point of the arc.
	//      &fEnd - end point of the arc.
	//      &rBulge - bulge factor of arc, equal to tan(0.25*included angle) where (+) = counter clockwise.
	//
	//  Return Value:
	//		Returns true if the operation was successful, else false.
	//      
	//  Description:
	//		Creates a point on an arc defined by 2 points and a bulge factor.  The 2 points are the
	//      start and end point of the arc.  The bulge factor is the tangent of one quarter of the
	//      included angle of the arc.  If the bulge factor is positive the arc is assummed to 
	//      be clockwise from the start point to the end point of the arc.  A negative bulge
	//      factor is used for a counter clockwise arc.
	//      
	//  Example:
	//		AuPoint fRimPoint.RimPoint(fStart, fEnd, rBf);
	bool RimPoint(const AuPoint &fBeg, const AuPoint &fEnd, const double rBulge);

	AuMatrix3x3 operator/(const AuPoint &p) const; // Rotation only, no scaling
	AuMatrix3x3 operator%(const AuPoint &p) const; // Rotation and scaling 
	inline AuPoint operator+(const AuPoint &p) const
		{return(AuPoint(x + p.x, y + p.y, z + p.z));}
	inline AuPoint operator-(const AuPoint &p) const
		{return(AuPoint(x - p.x, y - p.y, z - p.z));}
	inline AuPoint operator-() const
		{return(AuPoint(-x, -y, -z));}
	inline AuPoint operator*(double r) const
		{return((r != 0.0) ? AuPoint(x * r, y * r, z * r) : AuPoint(0,0));}
	inline AuPoint operator/(double r) const
		{return((r != 0.0) ? AuPoint(x / r, y / r, z / r) : AuPoint(0,0));}
	AuPoint operator/(const AuMatrix3x3 &r) const; // AuMatrix3x3 must be unit (no scaling)
	AuPoint operator%(const AuMatrix3x3 &r) const; // Scaling and rotation
	inline const AuPoint& operator=(const AuPoint &p)
		{x = p.x; y = p.y; z = p.z; return(*this);}
	inline bool operator==(const AuPoint &p) const
		{return ((x == p.x) && (y == p.y) && (z == p.z));}
	inline bool operator!=(const AuPoint &p) const
		{return ((x != p.x) || (y != p.y) || (z != p.z));}
	inline bool operator>=(const AuPoint &p) const	// true if ALL components are >= corresponding components
		{return ((x >= p.x) && (y >= p.y) && (z >= p.z));}
	inline bool operator<=(const AuPoint &p) const	// true if ALL components are <= corresponding components
		{return ((x <= p.x) && (y <= p.y) && (z <= p.z));}

	// Precedence is Z, Y then X. Do not change this. The AuMeatMesh sort order depends on this.
	inline bool operator<(const AuPoint &p) const
	{
		if(z < p.z)
			return true;
		else if(z > p.z)
			return false;

		if(y < p.y)
			return true;
		else if(y > p.y)
			return false;

		if(x < p.x)
			return true;
		else if(x > p.x)
			return false;

		return false;
	}

	inline bool operator>(const AuPoint &p) const
	{
		if(z > p.z)
			return true;
		else if(z < p.z)
			return false;

		if(y > p.y)
			return true;
		else if(y < p.y)
			return false;

		if(x > p.x)
			return true;
		else if(x < p.x)
			return false;

		return false;
	}

	inline AuPoint& operator+=(const AuPoint &p)
	{
		x += p.x;
		y += p.y;
		z += p.z;
		return *this;
	}

	inline AuPoint& operator-=(const AuPoint &p)
	{
		x -= p.x;
		y -= p.y;
		z -= p.z;
		return *this;
	}

	inline AuPoint& operator*=(const double r)
	{
		x *= r;
		y *= r;
		z *= r;
		return *this;
	}

	inline AuPoint& operator/=(const double r)
	{
		x /= r;
		y /= r;
		z /= r;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << x;
		out << y;
		out << z;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> x;
		in >> y;
		in >> z;
	}

	inline double Length() const											// 3D length
	{
#if defined(USE_SSE_MATH)
		// Create an aligned memory buffer on the stack large enough for two doubles, since we get results back in 128-bit pairs.
		unsigned char buf[sizeof(double)*2 + 15];			// Byte buffer that will allow 16-byte address alignment of two doubles.
		unsigned char* buf16 = (unsigned char*)(((long int)buf+15)&~0xf);	// Bump memory address forward to next 16-byte boundary.

		// Do the math using SSE functions.
		__m128d xy = _mm_set_pd(y,x);
		__m128d x2y2 = _mm_mul_pd(xy, xy);									// First pair of results x^2 and y^2
		__m128d zw = _mm_set_pd(0,z);
		__m128d z2w2 = _mm_mul_sd(zw, zw);									// Second pair of results z^2 and 0 ignored
		__m128d sum1 = _mm_add_sd(x2y2, z2w2);								// First add, x^2+z^2 and y^2 passed through
		__m128d sum2 = _mm_unpackhi_pd(sum1, sum1);							// Sum2 is now y^2, y^2
		__m128d sum3 = _mm_add_sd(sum1, sum2);								// Second add, (x^2+z^2)+y^2 and y^2 passed through

		// Store the result of the final square root into our 16-byte aligned buffer address.
		*(__m128d*)buf16 = _mm_sqrt_pd(sum3);								// Finally the square root

		// Return the result.
		return *(double*)buf16;
#else
		return(sqrt(x*x + y*y + z*z));
#endif
	}

	inline double Length2() const											// square of 3D length
		{return(x*x + y*y + z*z);}
	inline double Length_2D() const											// 2D length
		{return(sqrt(x*x + y*y));}
	inline double Length2_2D() const										// square of 2D length
		{return(x*x + y*y);}

	// Returns the 2D angle of the vector.
	// 'this' vector does not have to be a unit vector.
	inline double ArcTangent_2D() const
	{
		return ArcTangent_2D(x,y);
	}

	// Returns the angle of the vector around the X axis.
	// 'this' vector does not have to be a unit vector.
	inline double XRotation() const
	{
		return ArcTangent_2D(y,z);
	}

	// Returns the angle of the vector around the Y axis.
	// 'this' vector does not have to be a unit vector.
	inline double YRotation() const
	{
		return ArcTangent_2D(-x,z);
	}

	// Returns the angle of the vector around the Z axis.
	// 'this' vector does not have to be a unit vector.
	inline double ZRotation() const
	{
		return ArcTangent_2D(x,y);
	}

	inline static double ArcTangent_2D(double x, double y)
	{
		double	ang;
		if(::IsFuzzyZero(y))
		{
			if(GreaterThanOrFuzzyEquals(x, 0.0))
				ang = 0.0;
			else
				ang = PI;
		}
		else
		{
			if(::IsFuzzyZero(x))
			{
				if(GreaterThanOrFuzzyEquals(y, 0.0))
					ang = PI/2.0;
				else
					ang = -PI/2.0;
			}
			else
				ang = atan2(y, x);
		}
		return(ang);
	}

	// Returns the angle formed by 2 3D vectors.
	// The return value is in the range: 0 <= r <= PI
	// NOTE: "This" vector and "vec" must be unit vectors.
	inline double AngleTo(const AuPoint &vec) const
	{
		double d = Dot(vec);
		if(d >= 1)
			return 0;
		if(d <= -1)
			return(PI);
		else if(d == 0)
			return(PI/2);
		else
			return(acos(d));
	}

	// return the angle of inclination above/below the XY plane
	// 'this' vector does not have to be a unit vector.
	inline double PolarIncline() const
	{
		AuPoint fPolar(Length_2D(), z);
		return fPolar.ArcTangent_2D();
	}

	inline double Dot(const AuPoint &p) const								// 3D dot product
	{
		return x*p.x+y*p.y+z*p.z;
	}

	// Calculates the dot product of the X and Y components only.
	inline double Dot_2D(const AuPoint &p) const
	{
		return x*p.x + y*p.y;
	}

	// Calculates the 3D cross product of 'this' vector and the vector 'p'.
	// 'this' vector and 'p' do not have to be unit vectors.
	// If 'this' and 'p' are unit vectors, the result is a unit vector.
	inline AuPoint Cross(const AuPoint &p) const							// cross product
	{
		return AuPoint(y*p.z - z*p.y, z*p.x - x*p.z, x*p.y - y*p.x);
	}

	inline double Distance(const AuPoint &p) const							// 3D distance to another point
		{return(AuPoint(*this - p).Length());}
	inline double Distance_2D(const AuPoint &p) const						// 2D distance to another point
		{return(AuPoint(*this - p).Length_2D());}
	inline double Distance2(const AuPoint &p) const							// square of 3D distance to another point
		{return(AuPoint(*this - p).Length2());}
	inline double Distance2_2D(const AuPoint &p) const						// square of 2D distance to another point
		{return(AuPoint(*this - p).Length2_2D());}

	// find the 2D distance from a point to a line segment
	// if the locate point is beyond either end point of the segment, the distance to the
	// closest end point is returned
	inline double DistToSegment_2D(const AuPoint &lp1,const AuPoint &lp2) const
	{
		AuPoint pint(PointOnSegment_2D(lp1,lp2));
		return(Distance_2D(pint));
	}

	// find the square of the 2D distance from a point to a line segment
	// if the locate point is beyond either end point of the segment, the distance to the
	// closest end point is returned
	inline double DistToSegment2_2D(const AuPoint &lp1, const AuPoint &lp2) const
	{
		AuPoint pint(PointOnSegment_2D(lp1,lp2));
		return(Distance2_2D(pint));
	}

	// find the 3D distance from a point to a line segment
	// if the locate point is beyond either end point of the segment, the distance to the
	// closest end point is returned
	inline double DistToSegment(const AuPoint &lp1,const AuPoint &lp2) const	
	{
		AuPoint pint(PointOnSegment(lp1,lp2));
		return(Distance(pint));
	}

	// find the square of the 3D distance from a point to a line segment
	// if the locate point is beyond either end point of the segment, the distance to the
	// closest end point is returned
	inline double DistToSegment2(const AuPoint &lp1,const AuPoint &lp2) const	
	{
		AuPoint pint(PointOnSegment(lp1,lp2));
		return(Distance2(pint));
	}

	// returns the distance from this point to a line through lp1 and lp2
	inline double DistToLine(const AuPoint &lp1,const AuPoint &lp2) const
	{
		AuPoint segvec(lp2 - lp1);
		double rlen = segvec.Length();
		if (rlen == 0.0)
			return(Distance(lp1));	// seg is too short, return distance to end point

		// return length of perpendicular distance to intersect drop point
		AuPoint locvec(*this - lp1);
		double rdot = locvec.Dot(segvec)/rlen;
		AuPoint pint(lp1 + segvec*rdot/rlen);
		return Distance(pint);
	}

	// Make the 3D vector a specific length.
	// Passing 1.0 (or 0.0) for len results in a unit vector.
	// If this vector is initially 0, the result points down the X axis.
	inline void Unit(double len)
	{
		double vlen = Length();
		if(vlen != 0.0)
		{
			if(vlen!=1.0)
				len /= vlen;
			if(len != 1.0)
				*this *= len;
		}
		else
		{
			x = len;
		}
	}

	inline void Unit()
	{
		double vlen = Length();
		if(vlen != 0.0)
			*this /= vlen;
		else
			x = 1;
	}

	// Make XY into a 2D unit vector, changing Z to 0.
	// If this vector is initially 0, the result points down the X axis.
	inline void Unit_2D(double len)
	{
		double vlen = Length_2D();
		if(vlen != 0.0)
		{
			if(vlen!=1.0)
				len /= vlen;
			if(len != 1.0)
			{
				x *= len;
				y *= len;
			}
		}
		else
		{
			x = len;
		}
		z = 0.0;
	}

	// Make XY into a 2D unit vector, changing Z to 0.
	// If this vector is initially 0, the result points down the X axis.
	inline void Unit_2D()
	{
		double vlen = Length_2D();
		if(vlen != 0.0)
		{
			x /= vlen;
			y /= vlen;
		}
		else
			x = 1;
		z = 0.0;
	}

	// Make XY into a 2D unit vector, without changing Z.
	// If this vector is initially 0, the result points down the X axis.
	inline void Unit_2D_AtZ(double len)
	{
		double vlen = Length_2D();
		if(vlen != 0.0)
		{
			if(vlen!=1.0)
				len /= vlen;
			if(len != 1.0)
			{
				x *= len;
				y *= len;
			}
		}
		else
		{
			x = len;
		}
	}

	// Make XY into a 2D unit vector, without changing Z.
	// If this vector is initially 0, the result points down the X axis.
	inline void Unit_2D_AtZ()
	{
		double vlen = Length_2D();
		if(vlen != 0.0)
		{
			x /= vlen;
			y /= vlen;
		}
		else
			x = 1;
	}

	inline void UnwindRotation()
	{
		x = UnwindAngle(x);
		y = UnwindAngle(y);
		z = UnwindAngle(z);
	}

	inline void Round(double rndval)
	{
		x = FROUND(x, rndval);
		y = FROUND(y, rndval);
		z = FROUND(z, rndval);
	}

	inline AuPoint Rounded() const
	{
		return AuPoint(FROUND(x, 1.), FROUND(y, 1.), FROUND(z, 1.));
	}

	inline AuPoint Rounded(double rndval) const
	{
		return AuPoint(FROUND(x, rndval), FROUND(y, rndval), FROUND(z, rndval));
	}

	//  Summary:
	//		Tests two AuPoint to see if they are within FUZZYZERO of being equal.
	//      
	//  Parameters:
	//		&p - Reference to a AuPoint which will be compared against 'this' AuPoint.
	//      
	//  Return Value:
	//		true if 'this' AuPoint is within FUZZYZERO of equaling the AuPoint referenced by &p.
	//      
	//  Description:
	//		Tests the x, y, and z, member variables of 'this' AuPoint to see if each is within FUZZYZERO of the x, y, and z, member
	//		variables of the point referenced by &p.  If so, true is returned, else false is returned.
	//      
	//  Example:
	//		if (p1.FuzzyEquals(p2))
	//		{...}
	//      
	inline bool FuzzyEquals(const AuPoint& p) const	// fuzzy comparison
	{ 
		if(!::FuzzyEquals(x, p.x))
			return false;
		if(!::FuzzyEquals(y, p.y))
			return false;
		if(!::FuzzyEquals(z, p.z))
			return false;
		return true;
	}

	inline bool FuzzyEquals(const AuPoint& p, double tol) const	// fuzzy comparison
	{ 
		if(!::FuzzyEqualsFactor(x, p.x, tol))
			return false;
		if(!::FuzzyEqualsFactor(y, p.y, tol))
			return false;
		if(!::FuzzyEqualsFactor(z, p.z, tol))
			return false;
		return true;
	}

	//  Summary:
	//		Tests two AuPoints to see if their 2D components are within FUZZYZERO of being equal.
	//      
	//  Parameters:
	//		&p - Reference to a AuPoint which will be compared against 'this' AuPoint.
	//      
	//  Return Value:
	//		true if the 2D components of 'this' AuPoint are within FUZZYZERO of equaling the 2D components of the AuPoint referenced by &p.
	//      
	//  Description:
	//		Tests the x and y member variables of 'this' AuPoint to see if each is within FUZZYZERO of the x and y member
	//		variables of the point referenced by &p.  If so, true is returned, else false is returned.
	//      
	//  Example:
	//		if (p1.FuzzyEquals_2D(p2))
	//		{...}
	//      
	inline bool FuzzyEquals_2D(const AuPoint& p) const	// fuzzy comparison...ignores Z
	{ 
		if(!::IsFuzzyZero(x - p.x))
			return false;
		if(!::IsFuzzyZero(y - p.y))
			return false;
		return true;
	}

	inline bool FuzzyEquals_2D(const AuPoint& p, double tol) const	// fuzzy comparison...ignores Z
	{ 
		if(!::FuzzyEqualsFactor(x, p.x, tol))
			return false;
		if(!::FuzzyEqualsFactor(y, p.y, tol))
			return false;
		return true;
	}

	// Perform not-a-number test on each x,y,z value.
	// True if any value x,y,z is not-a-number
	inline bool IsNaN() const
	{
		if(std::isnan(x) || std::isnan(y) || std::isnan(z))
			return true;
		return false;
	}
	
	//  Summary:
	//		Tests 'this' AuPoint to see if it equal to absolute zero (or within FUZZYZERO of absolute zero).
	//      
	//  Return Value:
	//		true if 'this' AuPoint equals absolute zero, false if not.
	//      
	//  Description:
	//		Tests the x, y, and z, member variables of 'this' AuPoint to see if any is greater than FUZZYZERO.  If so, false is returned, else true is returned.
	//      
	//  Example:
	//		if (p1.IsFuzzyZero())
	//		{...}
	//      
	inline bool IsFuzzyZero() const								// fuzzy test for 0,0,0 vector
	{
		if(!::IsFuzzyZero(x))
			return false;
		if(!::IsFuzzyZero(y))
			return false;
		if(!::IsFuzzyZero(z))
			return false;
		return true;
	}

	// Test for X, Y and Z exactly zero, not a fuzzy test.
	inline bool IsZero() const
	{
		return x == 0. && y == 0. && z == 0.;
	}

	//  Summary:
	//		Tests the 2D components of 'this' AuPoint to see if it equal to absolute zero (or within FUZZYZERO of absolute zero).
	//      
	//  Return Value:
	//		true if the 2D components of 'this' AuPoint equals absolute zero, false if not.
	//      
	//  Description:
	//		Tests the x and y member variables of 'this' AuPoint to see if either is greater than FUZZYZERO.  If so, false is returned, else true is returned.
	//      
	//  Example:
	//		if (p1.IsFuzzyZero_2D())
	//		{...}
	//      
	inline bool IsFuzzyZero_2D() const							// fuzzy test for 0,0,n vector
	{
		if(!::IsFuzzyZero(x))
			return false;
		if(!::IsFuzzyZero(y))
			return false;
		return true;
	}
	
	// Test for X and Y exactly zero, not a fuzzy test.
	inline bool IsZero_2D() const
	{
		return x == 0. && y == 0.;
	}

	//  Summary:
	//		Tests two unit vectors to see if they are within FUZZYZERO of being parallel.
	//      
	//  Parameters:
	//		&unitVec - Reference to AuPoint to test against 'this' AuPoint for parallel.
	//
	//  Return Value:
	//		true if 'this' AuPoint and the AuPoint referenced by &unitVec are parallel.
	//      
	//  Description:
	//		Compares 'this' AuPoint against &unitVec to determine whether or not the two unit vectors are within FUZZYZERO of being parallel.
	//      
	//  Example:
	//		p1.Unit();
	//		p2.Unit();
	//		if (p1.IsParallel(p2))
	//		{...}
	//      
	inline bool IsParallel(const AuPoint& unitVec) const // fuzzy test for parallel vectors (must be unit vectors)
	{
		return ::IsFuzzyZero(fabs(Dot(unitVec)) - 1.0);
	}

	// fuzzy test for 2D parallel vectors (need not be unit vectors)
	inline bool IsParallel_2D(const AuPoint& vec) const
	{
		if(vec.x == 0 && x == 0)
			return true;
		if(vec.x == 0 || x == 0)
			return false;
		double myslope = y / x;
		double otherslope = vec.y / vec.x;
		return ::FuzzyEquals(myslope, otherslope);
	}

	bool AcadFillet(const AuPoint &fPrev, const AuPoint &fNext, const double rFillet,
		           AuPoint &fP1, AuPoint &fP2, double &rbulge) const;

	//  Summary:
	//		Perform orthogonal forcing with rounding on 'this' AuPoint.
	//      
	//  Parameters:
	//		&pref -	AuPoint to use as the reference point for the orthogonal rounding.
	//		rndval - Rounding value	
	//		bZAxisOk -	true if OK to snap to the Z-axis
	//		bAngle45 = true -	If false, 'this' AuPoint will be forced to align to either the X or Y axis of &pref.  If true, 45
	//          degree increments between the axis will also be considered valid options.
	//
	//  Description:
	//		Perform orthogonal forced rounding on 'this' AuPoint.  The result will be rounded at an even 45 degree angle from the
	//      reference point.
	//      
	//  Example:
	//		p1.Ortho(pBase, rInc, true, CadServices()->IsOrtho45());
	//      
	inline void Ortho(const AuPoint& pref, double rndval, bool bZAxisOk, bool bAngle45 = true)
	{
		if(*this == pref)
			return;	// same point, do nothing

		// force the point to an axis
		// find out what the best angle is
		AuPoint pvec(*this - pref);
		double rangle = pvec.ArcTangent_2D() + PI/8.0;
		double rAdj = 0.0;
		if(!bAngle45)
		{
			rAdj = PI/4.0;
			rangle += PI/8.0;
		}
		while (rangle < 0.0)
			rangle += (PI*2.0);
		while (rangle >= (PI*2.0))
			rangle -= (PI*2.0);

		if (bZAxisOk && fabs(pvec.PolarIncline()) > PI/4.0)
		{
			// snap to the Z axis
			x = pref.x;
			y = pref.y;
			z = FROUND(z, rndval);
		}
		else
		{
			// we will snap to increments of 45 degrees in the XY plane through pref
			z = pref.z;
			if (rangle < (PI/4.0 + rAdj) || (rangle >= PI && rangle < (5.0*PI/4.0 + rAdj)))
			{
				// X axis
				y = pref.y;
				x = FROUND(x, rndval);
			}
			else if ((rangle >= PI/2.0 && rangle < (3.0*PI/4.0 + rAdj)) || (rangle >= 3.0*PI/2.0 && rangle < (7.0*PI/4.0 + rAdj)))
			{
				// Y axis
				x = pref.x;
				y = FROUND(y, rndval);
			}
			else 
			{
				AuPoint ptmp;
				if (rangle >= PI/4.0 && rangle < PI/2.0)
				{
					// 45 degrees
					ptmp.x = 1.0;
					ptmp.y = -1.0;
				}
				else if (rangle >= (3.0*PI/4.0) && rangle < PI)
				{
					// 135 degrees
					ptmp.x = 1.0;
					ptmp.y = 1.0;
				}
				else if (rangle >= (5.0*PI/4.0) && rangle < 3.0*PI/2.0)
				{
					// 225 degrees
					ptmp.x = -1.0;
					ptmp.y = 1.0;
				}
				else if (rangle >= (7.0*PI/4.0))
				{
					// 315 degrees
					ptmp.x = -1.0;
					ptmp.y = -1.0;
				}
				else
				{
					ptmp.x = 0;
					ptmp.y = 0;
				}
				ptmp.z = 0.0;
				ptmp.Unit();
				double rdot = ptmp.Dot(pvec);
				ptmp = ptmp * rdot;			// vector to axis
				*(this) = *(this) - ptmp;	// move our input point to the axis
				// the result point must be rounded to fall directly on a grid line
				// it may not be able to hit an intersection because the ortho point may
				// not be on a grid line
				// determine which axis is closest to the forced point we have calculated
				ptmp = *this;
				ptmp.Round(rndval);
				// which axis was rounded the least?
				if (fabs(x-ptmp.x) < fabs(y-ptmp.y))
				{
					// X axis rounded the least
					double rscl = (ptmp.x-pref.x)/(x-pref.x);
					y = pref.y + (y-pref.y)*rscl;
					x = ptmp.x;
				}
				else
				{
					// Y axis rounded the least
					double rscl = (ptmp.y-pref.y)/(y-pref.y);
					x = pref.x + (x-pref.x)*rscl;
					y = ptmp.y;
				}
			}
		}
	}

	//  Summary:
	//		Perform orthogonal forcing without rounding on 'this' AuPoint.
	//      
	//  Parameters:
	//		&pref -	AuPoint to use as the reference point for the orthogonal rounding.
	//		bZAxisOk -	true if OK to snap to the Z-axis
	//		bAngle45 = true -	If false, 'this' AuPoint will be forced to align to either the X or Y axis of &pref.  If true, 45
	//          degree increments between the axis will also be considered valid options.
	//
	//  Description:
	//		Perform orthogonal forced rounding on 'this' AuPoint.  The result will be rounded at an even 45 degree angle from the
	//      reference point.
	//      
	//  Example:
	//		p1.Ortho(pBase, true, CadServices()->IsOrtho45());
	//      
	inline void Ortho(const AuPoint& pref, bool bZAxisOk, bool bAngle45 = true)
	{
		if(*this == pref)
			return;	// same point, do nothing

		// force the point to an axis
		// find out what the best angle is
		AuPoint pvec(*this - pref);
		double rangle = pvec.ArcTangent_2D() + PI/8.0;
		double rAdj = 0.0;
		if(!bAngle45)
		{
			rAdj = PI/4.0;
			rangle += PI/8.0;
		}
		while (rangle < 0.0)
			rangle += (PI*2.0);
		while (rangle >= (PI*2.0))
			rangle -= (PI*2.0);

		if (bZAxisOk && fabs(pvec.PolarIncline()) > PI/4.0)
		{
			// snap to the Z axis
			x = pref.x;
			y = pref.y;
		}
		else
		{
			// we will snap to increments of 45 degrees in the XY plane through pref
			z = pref.z;
			if (rangle < (PI/4.0 + rAdj) || (rangle >= PI && rangle < (5.0*PI/4.0 + rAdj)))
			{
				// X axis
				y = pref.y;
			}
			else if ((rangle >= PI/2.0 && rangle < (3.0*PI/4.0 + rAdj)) || (rangle >= 3.0*PI/2.0 && rangle < (7.0*PI/4.0 + rAdj)))
			{
				// Y axis
				x = pref.x;
			}
			else 
			{
				AuPoint ptmp;
				if (rangle >= PI/4.0 && rangle < PI/2.0)
				{
					// 45 degrees
					ptmp.x = 1.0;
					ptmp.y = -1.0;
				}
				else if (rangle >= (3.0*PI/4.0) && rangle < PI)
				{
					// 135 degrees
					ptmp.x = 1.0;
					ptmp.y = 1.0;
				}
				else if (rangle >= (5.0*PI/4.0) && rangle < 3.0*PI/2.0)
				{
					// 225 degrees
					ptmp.x = -1.0;
					ptmp.y = 1.0;
				}
				else if (rangle >= (7.0*PI/4.0))
				{
					// 315 degrees
					ptmp.x = -1.0;
					ptmp.y = -1.0;
				}
				else
				{
					ptmp.x = 0;
					ptmp.y = 0;
				}
				ptmp.z = 0.0;
				ptmp.Unit();
				double rdot = ptmp.Dot(pvec);
				ptmp = ptmp * rdot;			// vector to axis
				*(this) = *(this) - ptmp;	// move our input point to the axis
				// the result point must be rounded to fall directly on a grid line
				// it may not be able to hit an intersection because the ortho point may
				// not be on a grid line
				// determine which axis is closest to the forced point we have calculated
				ptmp = *this;
				// which axis was rounded the least?
				if (fabs(x-ptmp.x) < fabs(y-ptmp.y))
				{
					// X axis rounded the least
					double rscl = (ptmp.x-pref.x)/(x-pref.x);
					y = pref.y + (y-pref.y)*rscl;
					x = ptmp.x;
				}
				else
				{
					// Y axis rounded the least
					double rscl = (ptmp.y-pref.y)/(y-pref.y);
					x = pref.x + (x-pref.x)*rscl;
					y = ptmp.y;
				}
			}
		}
	}
	
	//  Summary:
	//		Snap 'this' AuPoint to one of the corners created by a rectangle through &fRef1 and &fRef2.
	//      
	//  Parameters:
	//		&fRef1 -	AuPoint reference identifying one corner of a rectangle.
	//		&fRef2 -	AuPoint reference identifying a second corner of a rectangle
	//
	//  Description:
	//		Snap this point to one of the corners created by a rectangle through &fRef1 and &fRef2
	//		the corner that 'this' point is closest to will be selected.
	//      
	//  Example:
	//		fPt.OrthoRectangle(fR1,fR2);
	//      
	inline void OrthoRectangle(const AuPoint &fRef1,const AuPoint &fRef2)
	{
		AuPoint fc1(fRef1),fc2(fRef2);
		fc1.y = fRef2.y;
		fc2.y = fRef1.y;
		double r1 = Distance(fc1);
		double r2 = Distance(fc2);
		if (r1 < r2)
			*this = fc1;
		else
			*this = fc2;
	}

// return perp loc from this to the line between fLoc1 and fLoc2
	
	//  Summary:
	//		Returns a AuPoint that represents a point on a line perpendicular to 'this' AuPoint.
	//      
	//  Parameters:
	//		&fLoc1 - First AuPoint reference used to define a line.	
	//		&fLoc2 - Second AuPoint reference used to define a line.
	//
	//  Return Value:
	//		AuPoint representing a point location perpendicular to 'this' point and a line between &fLoc1 and &fLoc2.
	//      
	//  Description:
	//      
	//  See Also:
	//      
	//	Example:
	//
	inline AuPoint Perpendicular(const AuPoint &fLoc1,const AuPoint &fLoc2) const
	{
		// unit vector in direction of line segment
		AuPoint funit(fLoc2 - fLoc1);
		funit.Unit();

		// vector from segment start to this point
		AuPoint fvec(*this - fLoc1);

		// calculate intersection point using dot product
		double rdot = fvec.Dot(funit);
		AuPoint fperp(fLoc1 + funit * rdot);

		return fperp;
	}

	// return 2D perp loc from this to the line between fLoc1 and fLoc2
	// result has the same Z height as this point
	inline AuPoint Perpendicular_2D(const AuPoint &fLoc1,const AuPoint &fLoc2) const
	{
		//AuPoint fLoc1_2D(fLoc1.x,fLoc1.y,z);
		//AuPoint fLoc2_2D(fLoc2.x,fLoc2.y,z);
		//return(Perpendicular(fLoc1_2D,fLoc2_2D));

		// unit vector in direction of line segment
		AuPoint funit;
		funit.x = fLoc2.x - fLoc1.x;
		funit.y = fLoc2.y - fLoc1.y;
		funit.z = 0;
		funit.Unit();

		// vector from segment start to this point
		AuPoint fvec;
		fvec.x = x - fLoc1.x;
		fvec.y = y - fLoc1.y;
		fvec.z = 0;

		// calculate intersection point using dot product
		double rdot = fvec.Dot_2D(funit);
		AuPoint fperp;
		fperp.x = fLoc1.x + funit.x * rdot;
		fperp.y = fLoc1.y + funit.y * rdot;
		fperp.z = z;

		return(fperp);
	}
	
	inline AuPoint PerpendicularVector_2D() const	// return perpendicular vector
		{return(AuPoint(-y,x,0.0));}

	inline AuPoint Reflect_2D() const		// return reflected vector
		{return(AuPoint(-y,-x,0.0));}

	// true if this is between p1 and p2.
	// Does not test for distance from the points, just that the drop point onto that vector falls between or fuzzy at its ends.
	inline bool Between(const AuPoint &p1,const AuPoint &p2, double tol=FUZZ) const
	{
		if(FuzzyEquals(p1, tol) || FuzzyEquals(p2, tol))
			return true;

		AuPoint pvec1(p2 - p1);
		double rlen = pvec1.Length2();
		if(rlen == 0)
			return false;				// zero length vector

		rlen = sqrt(rlen);
		pvec1 /= rlen;					// make unit vector

		AuPoint pvec2(*this - p1);
		double rdot = pvec2.Dot(pvec1);
		if(rdot < 0 || rdot > rlen)
			return false;				// not between the points

		return true;
	}

	// true if between p1 and p2 at Z height of this
	inline bool Between_2D(const AuPoint &p1,const AuPoint &p2) const
	{
		AuPoint p1_2D(p1.x, p1.y, z);
		AuPoint p2_2D(p2.x, p2.y, z);
		return Between(p1_2D, p2_2D);
	}

	inline AuPoint Min(const AuPoint &fcomp) const
	{
		return(AuPoint(MIN(fcomp.x,x), MIN(fcomp.y,y), MIN(fcomp.z,z)));
	}

	inline AuPoint Max(const AuPoint &fcomp) const
	{
		return(AuPoint(MAX(fcomp.x,x), MAX(fcomp.y,y), MAX(fcomp.z,z)));
	}

	// Returns the absolute value of the maximum of X, Y or Z.
	inline double MaxDimension() const
	{
		double fx = fabs(x);
		double fy = fabs(y);
		double fz = fabs(z);
		if(fx > fy)
		{
			if(fx > fz)
				return fx;
		}
		else
		{
			if(fy > fz)
				return fy;
		}
		return fz;
	}

	// Returns the absolute value of the maximum of X or Y.
	inline double MaxDimension_2D() const
	{
		double fx = fabs(x);
		double fy = fabs(y);
		if(fx > fy)
			return fx;
		return fy;
	}

	// find closest point on a line segment
	// if the locate point is beyond either end point of the segment, the 
	// closest end point is returned
	inline AuPoint PointOnSegment_2D(const AuPoint &lp1,const AuPoint &lp2) const
	{
		AuPoint segvec(lp2 - lp1);
		double rlen = segvec.Length_2D();
		if(rlen == 0.0)
			return(lp1);	// seg is too short, return end point

		AuPoint locvec(*this - lp1);
		double rdot = locvec.Dot_2D(segvec)/rlen;
		if(rdot <= 0.0)
			return lp1;	// beyond first segment end
		else if(rdot >= rlen)
			return lp2;	// beyond other end of segment
		else
		{
			// between segment points, get length of perpendicular distance
			AuPoint pint(lp1 + segvec*rdot/rlen);
			return pint;
		}
	}

	// find closest point on a line segment
	// if the locate point is beyond either end point of the segment, the 
	// closest end point is returned
	inline AuPoint PointOnSegment(const AuPoint &lp1,const AuPoint &lp2) const
	{
		AuPoint segvec(lp2 - lp1);
		double rlen = segvec.Length();
		if (rlen == 0.0)
			return(lp1);	// seg is too short, return end point

		AuPoint locvec(*this - lp1);
		double rdot = locvec.Dot(segvec)/rlen;
		if (rdot <= 0.0)
			return lp1;	// beyond first segment end
		else if(rdot >= rlen)
			return lp2;	// beyond other end of segment
		else
		{
			// between segment points, get length of perpendicular distance
			AuPoint pint(lp1 + segvec*rdot/rlen);
			return pint;
		}
	}

	inline bool SegmentIntersection(const AuPoint &fA1, const AuPoint &fA2, const AuPoint &fB1, const AuPoint &fB2,
			bool bBInfinite=false, bool bAInfinite=false, double tol=FUZZ)
	{
		AuPoint fInt2;
		if(!fA1.PerpConn(fA2,fB1,fB2,*this,fInt2, tol))
			return false;

		if(!FuzzyEquals(fInt2, tol))
			return false;

		if(!bAInfinite && !Between(fA1,fA2, tol))
			return false;

		if(!bBInfinite && !Between(fB1,fB2, tol))
			return false;

		return true;
	}

	inline bool SegmentIntersection_2D(const AuPoint &fA1, const AuPoint &fA2, const AuPoint &fB1, const AuPoint &fB2,
			bool bBInfinite=false, bool bAInfinite=false)
	{
		// Perform 2D perpconn.
		AuPoint fInt2;
		if(!AuPoint(fA1.x,fA1.y,0).PerpConn(AuPoint(fA2.x,fA2.y,0),AuPoint(fB1.x,fB1.y,0),AuPoint(fB2.x,fB2.y,0),*this,fInt2))
			return false;

		if(!FuzzyEquals_2D(fInt2))
			return false;

		if(!bAInfinite && !Between_2D(fA1,fA2))
			return false;

		if(!bBInfinite && !Between_2D(fB1,fB2))
			return false;

		return true;
	}

	// Finds the 2D intersection of infinite lines passing through points a0-a1 and b0-b1.
	// Sets 'this' point to the 3D intersection along the a0-a1 line at the 2D intersection point.
	// Returns true if an intersection was found.
	bool LineIntersection2D_Result3D(const AuPoint &a0, const AuPoint &a1, const AuPoint &b0, const AuPoint &b1);

	// returns the side of the line from fLine1 to fLine2 that this point is on
	// 1=left, 0=on, -1=right
	inline int SideOfLine_2D(const AuPoint &fLine1,const AuPoint &fLine2) const	// 1=left,0=on,-1=right
	{
		double rX1 = fLine1.x - x;
		double rY1 = fLine1.y - y;
		double rX2 = fLine2.x - x;
		double rY2 = fLine2.y - y;
		double rCross = (rX1 * rY2) - (rX2 * rY1);
		if (rCross < 0)
			return -1;
		else if (rCross > 0)
			return 1;
		else
			return 0;
	}

	inline bool IsOnSegment(const AuPoint &ptSegEnd1, const AuPoint &ptSegEnd2) const
	{
		if(DistToLine(ptSegEnd1, ptSegEnd2) > FUZZ)
			return false;
		return Between(ptSegEnd1, ptSegEnd2, FUZZ);
	}

	inline bool IsOnSegment(const AuPoint &ptSegEnd1, const AuPoint &ptSegEnd2, double tol) const
	{
		if(DistToLine(ptSegEnd1, ptSegEnd2) > tol)
			return false;
		return Between(ptSegEnd1, ptSegEnd2, tol);
	}

	inline bool IsOnSegment_2D(const AuPoint &ptSegEnd1, const AuPoint &ptSegEnd2) const
	{
		if(DistToSegment_2D(ptSegEnd1, ptSegEnd2) > FUZZ)
			return false;
		return true;
	}

	inline bool IsOnSegment_2D(const AuPoint &ptSegEnd1, const AuPoint &ptSegEnd2, double tol) const
	{
		if(DistToSegment_2D(ptSegEnd1, ptSegEnd2) > tol)
			return false;
		return true;
	}

	// scale this vector
	inline void Scale(const AuPoint& s)
	{
		Scale(s.x, s.y, s.z);
	}

	// scale this vector
	inline void Scale(double sx, double sy, double sz)
	{
		x *= sx;
		y *= sy;
		z *= sz;
	}

	inline AuPoint RotateX(double angle) const	// Rotate around X by angle in radians.
	{
		double c = cos(angle);
		double s = sin(angle);
		AuPoint rotvec;
		rotvec.x = x;
		rotvec.y = c*y + -s*z;
		rotvec.z = s*y + c*z;
		return(rotvec);
	}
	inline AuPoint RotateY(double angle) const	// Rotate around Y by angle in radians.
	{
		double c = cos(angle);
		double s = sin(angle);
		AuPoint rotvec;
		rotvec.x = c*x + s*z;
		rotvec.y = y;
		rotvec.z = -s*x + c*z;
		return(rotvec);
	}
	inline AuPoint RotateZ(double angle) const	// Rotate around Z by angle in radians.
	{
		double c = cos(angle);
		double s = sin(angle);
		AuPoint rotvec;
		rotvec.x = c*x + -s*y;
		rotvec.y = s*x + c*y;
		rotvec.z = z;
		return(rotvec);
	}

	// Returns rotation transformation matrix required to align 'this' vector to another vector.
	AuMatrix4x4 GetAlignmentTransform(const AuPoint &vAlignTo) const;

	// parses a string and sets 'this' point to the resulting point value
	// 'scale' is applied to the resulting coordinate before projecting into the design plane, useful for units conversion when
	// implied input units are not the same as the underlying units. e.g. User enters values in meters but we want them stored
	// in feet so 'scale' is FeetPerMeter;
	// 'designPlane' can be passed to unproject the point that is input into the real universe.
	// All coordinates except absolute coordinates, those prefixed with 'a', are projected using designPlane.Project.
	void StringValue(const QString& sInput, double scale=1., const AuMatrix4x4* designPlane=0);

	// parses a string containing a polar coordinate and sets 'this' point to the resulting point value
	// "length,angle,inclination"
	inline void StringPolarValue(const QString &sParse)
	{
		QString sInput = sParse.trimmed();

		double rang = 0.0, rincline = 0.0;
		int iComma = sInput.indexOf(',');
		if (iComma >= 0)
		{
			QString sAng = sInput.mid(iComma+1);		// extract ANG portion
			sInput = sInput.left(iComma);				// truncate at comma
			iComma = sAng.indexOf(',');					// look for comma before inclination
			if (iComma >= 0)
			{
				QString sIncline = sAng.mid(iComma+1);	// extract inclination portion
				sAng = sAng.left(iComma);				// truncate at comma
				rincline = sIncline.toDouble();			// Inclination angle entered in degrees.
				rincline *= DEG2RAD;					// All angle values stored in radians.
			}
			rang = sAng.toDouble();						// Z angle entered in degrees.
			rang *= DEG2RAD;							// All angle values stored in radians.
		}

		double rlen = sInput.toDouble();				// Length of polar vector.
		PolarValue(rlen, rang, rincline);				// convert polar values into rectangular coords
	}

	// parses a string and sets 'this' point to the resulting rotation value
	// the string is analyzed as a rotation value
	// if only one value is entered, all three scales are set to the same value
	inline void StringRotationValue(const QString &sParse)
	{
		x = 0.0;
		y = 0.0;
		z = 0.0;

		QString sInput(sParse);
		int iComma = sInput.indexOf(',');
		if (iComma >= 0)
		{
			// There are commas so take the values as X, Y, and Z angles.
			QString sY = sInput.mid(iComma+1);			// extract Y portion
			sInput = sInput.left(iComma);				// truncate at comma
			iComma = sY.indexOf(',');					// look for comma before Z
			if (iComma >= 0)
			{
				QString sZ = sY.mid(iComma+1);			// extract Z portion
				sY = sY.left(iComma);					// truncate at comma
				z = sZ.toDouble() * DEG2RAD;
			}
			y = sY.toDouble() * DEG2RAD;
			x = sInput.toDouble() * DEG2RAD;
		}
		else
		{
			// No commas so take the single value as a Z angle only.
			z = sInput.toDouble() * DEG2RAD;
		}
	}

	void MakeRectangle(const AuPoint &fCorner,AuPoint fLocs[4],AuPoint *pRot=NULL) const;

	// Create the 3D box points from this point, the lower right corner (fX), and a point in the plane that also defines the 
	// height (fPlane).
	inline void MakeRectangle(const AuPoint &fX, const AuPoint &fPlane, AuPoint fLocs[4])
	{
		AuPoint fXVec(fX - *this);
		fXVec.Unit();
		AuPoint fPlaneVec(fPlane - *this);
		AuPoint fP2(fPlaneVec);
		fP2.Unit();
		AuPoint fNormal(fXVec.Cross(fP2));
		fNormal.Unit();
		AuPoint fYVec = fNormal.Cross(fXVec);
		fYVec.Unit();
		double rY = fPlaneVec.Dot(fYVec);
		fYVec *= rY;
		fLocs[0] = *this;
		fLocs[1] = *this + fYVec;
		fLocs[2] = fX + fYVec;
		fLocs[3] = fX;
	}

	inline bool InBoundary(const AuPoint* pBoundary, int lCount, bool bClockwise=true) const
	{
		if(lCount == 2)
			return(SideOfLine_2D(pBoundary[0],pBoundary[1]) == 0 ? true : false);	// this boundary is only a line segment
		if(lCount < 3)
			return(false);

		int lTriangles = 0;
		for(int lVert=2;lVert<lCount;lVert++)
		{
			// calculate the area of the triangle formed by this vertex, the previous vertex, and vertex 0
			// we will add 1 for each clockwise triangle and subtract 1 for each counterclockwise triangle that contains the point
			int lSide = pBoundary[lVert].SideOfLine_2D(pBoundary[0],pBoundary[lVert-1]);
			if(lSide != 0 && InTriangle(pBoundary[0],pBoundary[lVert-1],pBoundary[lVert]))
				lTriangles -= lSide;	// lSide is +1 or -1
		}

		// the signs are reversed when the boundary is counterclockwise
		if(!bClockwise)
			lTriangles = -lTriangles;

		// the lTriangles count will be positive if the point is inside the boundary
		if(lTriangles > 0)
			return(true);
		else
			return(false);
	}

	// Returns true if this point is inside an ellipsoid centered at 'center' with radii 'xrad', 'yrad' and 'zrad'.
	bool InEllipsoid(const AuPoint& center, double xrad, double yrad, double zrad) const;
	inline bool InEllipsoid(const AuPoint& center, const AuPoint& radii) const
	{
		return InEllipsoid(center, radii.x, radii.y, radii.z);
	}

	// Returns true if this point is inside a sphere centered at 'center' with radius 'rad'.
	bool InSphere(const AuPoint& center, double rad) const;

	// Calculate intersection(s) with an ellipsoid centered at 'center' with radii 'xrad', 'yrad' and 'zrad'.
	// 'this' and 'l' are the ends of a line segment to intersect with the ellipsoid.
	// 'intersect0' and 'intersect1' are the resulting intersect points, if any.
	// Return value:
	//  -1 = no intersect, segment is inside ellipsoid
	//   0 = no intersect, segment is outside ellipsoid
	//   1 = 1 intersection, result in 'intersect0'
	//   2 = 2 intersections, results in 'intersect0' and 'intersect1' where 'intersect0' is the closest intersection to 'seg0'.
	int IntersectEllipsoid(const AuPoint& center, double xrad, double yrad, double zrad, const AuPoint& l, AuPoint& intersect0,
			AuPoint& intersect1) const;
	inline int IntersectEllipsoid(const AuPoint& center, const AuPoint& radii, const AuPoint& l, AuPoint& intersect0,
			AuPoint& intersect1) const
	{
		return IntersectEllipsoid(center, radii.x, radii.y, radii.z, l, intersect0, intersect1);
	}

	// Returns true if this point is on the surface of an ellipsoid centered at 'center' with radii 'xrad', 'yrad' and 'zrad'.
	bool OnEllipsoid(const AuPoint& center, double xrad, double yrad, double zrad) const;
	inline bool OnEllipsoid(const AuPoint& center, const AuPoint& radii) const
	{
		return OnEllipsoid(center, radii.x, radii.y, radii.z);
	}

	// Returns true if this point is on the surface of a sphere centered at 'center' with radius 'rad'.
	bool OnSphere(const AuPoint& center, double rad) const;

	// Returns this point scaled to the surface of an ellipsoid centered at 'center' with radii 'xrad', 'yrad' and 'zrad'.
	AuPoint ToEllipsoid(const AuPoint& center, double xrad, double yrad, double zrad) const;
	inline AuPoint ToEllipsoid(const AuPoint& center, const AuPoint& radii) const
	{
		return ToEllipsoid(center, radii.x, radii.y, radii.z);
	}

	// Returns this point scaled to the surface of a sphere centered at 'center' with radius 'rad'.
	AuPoint ToSphere(const AuPoint& center, double rad) const;

	// Return true if this point is fuzzy inside of a triangle in 2D.
	// The triangle can be clockwise or counterclockwise.
	// This function fails if the plane of the triangle is perpendicular to the XY plane.
	inline bool InTriangleFuzzy(const AuPoint &f1, const AuPoint &f2, const AuPoint &f3, double tol=FUZZ) const
	{
		int lSide0 = SideOfLine_2D(f1,f2);
		int lSide1 = SideOfLine_2D(f2,f3);
		int lSide2 = SideOfLine_2D(f3,f1);
		if(LessThanOrFuzzyEqualsFactor(lSide0, 0, tol) &&
				LessThanOrFuzzyEqualsFactor(lSide1, 0, tol) &&
				LessThanOrFuzzyEqualsFactor(lSide2, 0, tol))
			return true;	// this point is within a clockwise triangle formed by three consecutive boundary vertices
		else if(GreaterThanOrFuzzyEqualsFactor(lSide0, 0, tol) &&
				GreaterThanOrFuzzyEqualsFactor(lSide1, 0, tol) &&
				GreaterThanOrFuzzyEqualsFactor(lSide2, 0, tol))
			return true;	// this point is within a counterclockwise triangle formed by three consecutive boundary vertices
		return false;
	}

	// Return true if this point is inside of a triangle in 2D.
	// The triangle can be clockwise or counterclockwise.
	// This function fails if the plane of the triangle is perpendicular to the XY plane.
	inline bool InTriangle(const AuPoint &f1, const AuPoint &f2, const AuPoint &f3, bool bClockwiseOnly=false) const
	{
		int lSide0 = SideOfLine_2D(f1,f2);
		int lSide1 = SideOfLine_2D(f2,f3);
		int lSide2 = SideOfLine_2D(f3,f1);
		if(lSide0 <= 0 && lSide1 <= 0 && lSide2 <= 0)
			return true;	// this point is within a clockwise triangle formed by three consecutive boundary vertices
		else if(!bClockwiseOnly && lSide0 >= 0 && lSide1 >= 0 && lSide2 >= 0)
			return true;	// this point is within a counterclockwise triangle formed by three consecutive boundary vertices
		return false;
	}

	// Returns true if this point is inside of a triangle in 3D.
	// The triangle can be clockwise or counterclockwise.
	// This point must be IN the plane of the triangle to return true.
	bool InTriangle_3D(const AuPoint& f1,const AuPoint& f2,const AuPoint& f3) const;

	// Returns true if this point is inside of a triangle in 3D.
	// The triangle can be clockwise or counterclockwise.
	// This point must be IN the plane of the triangle to return true.
	// Useful when the normal to the plane of the triangle 'vN' is already known.
	bool InTriangle_3D(const AuPoint& f1,const AuPoint& f2,const AuPoint& f3, const AuPoint& vN) const;

	// Uses InTriangleFuzzy test once we get the locations into the same plane.
	// 'planeTol' is the Z difference from the plane of the triangle considered to be too far away.
	bool InTriangleFuzzy_3D(const AuPoint& f1,const AuPoint& f2,const AuPoint& f3, const AuPoint& vN, double planeTol) const;

	// Connect two 3D lines with shortest line (perpendicular to both)
	// 1st line is from "this" to p1, second line is from p2 to p3
	// pa and pb are endpoints of connector
	// If false is returned, lines are parallel, or one is zero length
	// If lines intersect, then pa==pb
	// pa and pb must be tested with AuPoint::Between
	inline bool PerpConn(const AuPoint &p1,const AuPoint &p2,const AuPoint &p3,AuPoint &pa,AuPoint &pb, double tol=FUZZ) const
	{
		AuPoint d0(p1 - *this);
		AuPoint d1(p3 - p2);
		AuPoint d2(p2 - *this);

		double d00 = d0.Dot(d0);
		double d01 = d0.Dot(d1);
		double d11 = d1.Dot(d1);
		double fa=d01*d01;
		double fb=d00*d11;
		double det = fa-fb;
	//	double det = d01*d01-d00*d11;
	//	if (det == 0.0)
	//	if (::IsFuzzyZero(det))
		if (fabs(det) <= tol*(fabs(fa)+fabs(fb)))
		{
			pa.x = 0;
			pa.y = 0;
			pa.z = 0;
			pb.x = 0;
			pb.y = 0;
			pb.z = 0;
			return false;
		}
		double r0 = d2.Dot(d0);
		double r1 = d2.Dot(d1);
		pa = *this + d0 * (( r1*d01 - r0*d11 ) / det);
		pb = p2 + d1 * (( d00*r1 - d01*r0 ) / det);
		return true;
	}

	// Sets this to center of circle passing through p1,p2,p3
	inline bool CircleCenter(const AuPoint &p1,const AuPoint &p2,const AuPoint &p3)
	{
		AuPoint d0(p2 - p1);
		AuPoint d1(p3 - p1);
		AuPoint d2(d0.Cross(d1));

		double t0 = d1.y*d2.z - d2.y*d1.z;
		double t1 = d2.y*d0.z - d0.y*d2.z;
		double t2 = d0.y*d1.z - d1.y*d0.z;

		double det = d0.x*t0 + d1.x*t1 + d2.x*t2;
		if (det==0)
			return false;

		double a0 = d0.Dot((p1+p2)*.5);
		double a1 = d1.Dot((p1+p3)*.5);
		double a2 = p1.Dot(d2);

		x = a0*t0 + a1*t1 + a2*t2;

		y	= a0 * (d1.z*d2.x - d2.z*d1.x)
			+ a1 * (d2.z*d0.x - d0.z*d2.x)
			+ a2 * (d0.z*d1.x - d1.z*d0.x);

		z	= a0 * (d1.x*d2.y - d2.x*d1.y)
			+ a1 * (d2.x*d0.y - d0.x*d2.y)
			+ a2 * (d0.x*d1.y - d1.x*d0.y);

		*this *= 1./det;
		return true;
	}

	// Returns the closest point on a circle to 'this' point.
	// Circle is defined by 'cen' = center, 'r' = radius, 'vn' = normal to plane of circle.
	AuPoint ClosestPointOnCircle(const AuPoint& cen, double r, const AuPoint& vn) const;

	inline void PerpendicularPlane(AuPoint &vX, AuPoint &vY)
	{
		// get reference vector i.e. ourselves
		AuPoint vA(*this);
		vA.Unit();

		// get second vector not equal to reference
		AuPoint vN(0.0,0.0,1.0);
		if(vA.FuzzyEquals(vN) || vA.FuzzyEquals(vN*-1.0))
			vN = AuPoint(1.0,0.0,0.0);

		// get first perpendicular vector
		vX = vA.Cross(vN);
		vX.Unit();

		// get vector perpendicular to fXVec
		vY = vX.Cross(vA);
		vY.Unit();
	}

	void GetPerpDistFromPlane(double& rPerpDistFromPlane, const AuPlane& Plane, bool bIncludeSign=false) const;

	// Convert 3D vector to 32 bits
	// Need not be unit - magnitude is lost
	inline unsigned long Encode() const
	{
#define VFC32 (32764.00/PI)
		return
			((unsigned long)(atan2(y,x)*VFC32+32768.49)<<16)
			| (unsigned long)(atan2(z,sqrt(x*x+y*y))*(VFC32*2)+32768.49);
	}

	// Convert 32 bit unsigned integer to a 3D unit vector
	inline void Decode(const unsigned long &k)
	{
		double t=((double)(k>>16)-32768)*(1./VFC32);
		double r=((double)(k&0xffff)-32768)*(.5/VFC32);
		double q=cos(r);
		x = cos(t)*q;
		y = sin(t)*q;
		z = sin(r);
	}

	// Convert 3D vector to 16 bits
	// Need not be unit - magnitude is lost
	inline unsigned int Encode16() const
	{
#define VFC16 (124.00/PI)
		return
			((unsigned long)(atan2(y,x)*VFC16+128.49)<<8)
			| (unsigned long)(atan2(z,sqrt(x*x+y*y))*(VFC16*2)+128.49);
	}

	// Convert 16 bit integer to a 3D unit vector
	inline void Decode16(const unsigned int &k)
	{
		double t=((double)(k>>8)-128)*(1./VFC16);
		double r=((double)(k&0xff)-128)*(.5/VFC16);
		double q=cos(r);
		x = cos(t)*q;
		y = sin(t)*q;
		z = sin(r);
	}

	void CubeExtents(double r, const AuMatrix4x4& tModel, AuPoint& min, AuPoint& max);

#if !defined(QT_NO_DEBUG)
	inline void Dump(const char* msg) const
	{
		if(msg)
			qDebug("AuPoint::Dump - %s - %g, %g, %g", msg, x,y,z);
		else
			qDebug("AuPoint::Dump - %g, %g, %g", x,y,z);
	}
#endif
};

Q_DECLARE_METATYPE(AuPoint)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

AUMATH_DLL QDataStream& operator<<(QDataStream& out, const AuPoint& p);

AUMATH_DLL QDataStream& operator>>(QDataStream& in, AuPoint& p);

AUMATH_DLL QDebug operator<<(QDebug dbg, const AuPoint& p);

// qHash is needed to use AuPoint in a QList in Windows.
AUMATH_DLL unsigned int qHash(const AuPoint& key);

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// A handy array object for AuPoints
class AUMATH_DLL AuPointArray : public QVector<AuPoint>
{
public:
	enum OverlapStatus
	{
		NoOverlap,
		Overlapped,
		ContainsChild,
		ContainedByChild
	};

public:
	// Adds fLoc only if not already in this array. FuzzyEquals test.
	// Returns true if added.
	inline bool AddIfUnique(const AuPoint &fLoc)
	{
		if (DoesContain(fLoc))
			return(false);
		else
		{
			append(fLoc);
			return(true);
		}
	}

	// Adds fLoc only if it's not the same as the last point in the array.
	// FuzzyEquals test. Returns true if added.
	inline bool AddIfDifferentFromLast(const AuPoint &fLoc)
	{
		int uiSize = count();
		if (uiSize && fLoc.FuzzyEquals(at(uiSize-1)))
			return(false);
		else
		{
			append(fLoc);
			return(true);
		}
	}

	void ClipBoundary(const AuPointArray &aPoly, bool bKeepPrimaryChunk);		// Clip this boundary using a polyline.

	// Treats this array as a series of segments. Finds the closest point on this array of segments to 'p'.
	// Returns the closest segment index. The returned index is the index of the first point of the segment, so the
	// return value will be in the range 0 to size()-2.
	// Returns -1 if this array is empty.
	// Returns 0 if this array has only 1 or 2 points.
	inline int ClosestSegmentIndex(const AuPoint& p) const
	{
		if(empty())
			return -1;
		else if(size() <= 2)
			return 0;

		int closeidx = 0;
		double closedist2 = 0;
		bool hasclosest = false;
		const_iterator pi = cbegin();
		AuPoint prev = *pi;
		int index = 0;
		for(pi++; pi != cend(); pi++, index++)
		{
			AuPoint pint(p.PointOnSegment(prev,*pi));
			double dist2 = p.Distance2(pint);
			if(!hasclosest || dist2 < closedist2)
			{
				if(dist2 == 0)
					return index;
				closedist2 = dist2;
				closeidx = index;
				hasclosest = true;
			}
			prev = *pi;
		}
		return closeidx;
	}

	// Treats this array as a series of segments. Finds the closest point on this array of segments to 'p'.
	// Returns the closest segment location.
	// Returns the point if this array contains only one point.
	// Returns 0,0 if this array is empty.
	inline AuPoint ClosestSegmentLoc(const AuPoint& p) const
	{
		if(isEmpty())
			return AuPoint(0,0);
		else if(size() == 1)
			return first();

		AuPoint pclosest;
		double closedist2 = 0;
		bool hasclosest = false;
		const_iterator pi = cbegin();
		AuPoint prev = *pi;
		for(pi++; pi != cend(); pi++)
		{
			AuPoint pint(p.PointOnSegment(prev,*pi));
			double dist2 = p.Distance2(pint);
			if(!hasclosest || dist2 < closedist2)
			{
				pclosest = pint;
				if(dist2 == 0)
					return pint;
				closedist2 = dist2;
				hasclosest = true;
			}
			prev = *pi;
		}
		return pclosest;
	}

	// Clears/recreates this array to form an ellipse. The boundary points will be clockwise when viewed from above.
	// NOTE: 'ax' and 'ay' are expected to be points on the positive X and Y axes.
	void CreateEllipse(const AuPoint& center, const AuPoint& ax, const AuPoint& ay, int facets);

	inline bool DoesContain(const AuPoint &fLoc) const	// Returns true if this array contains fLoc. FuzzyEquals test.
	{
		const AuPoint *pLoc = begin();
		for (int lCount=count(); lCount; lCount--, pLoc++)
			if (pLoc->FuzzyEquals(fLoc))
				return(true);
		return(false);
	}

	// Returns true if this array contains fLoc. FuzzyEquals test. 
	// Ignores Z component.
	inline bool DoesContain_2D(const AuPoint &fLoc) const	// Returns true if this array contains fLoc. FuzzyEquals_2D test.
	{
		const AuPoint *pLoc = begin();
		for (int lCount=count(); lCount; lCount--, pLoc++)
			if (pLoc->FuzzyEquals_2D(fLoc))
				return(true);
		return(false);
	}

	// Returns true if loc.Rounded() matches the rounded locations in this array.
	inline bool DoesContainRounded(const AuPoint& loc) const
	{
		AuPoint rndloc(loc.Rounded());
		for(const_iterator ai = constBegin(); ai != constEnd(); ai++)
			if((*ai).Rounded() == rndloc)
				return true;
		return false;
	}

	// Returns true if loc.Rounded() matches the rounded locations in this array.
	inline bool DoesContainRounded(const AuPoint& loc, double rndval) const
	{
		AuPoint rndloc(loc.Rounded(rndval));
		for(const_iterator ai = constBegin(); ai != constEnd(); ai++)
			if((*ai).Rounded(rndval) == rndloc)
				return true;
		return false;
	}

	// Returns true if a segment intersects a boundary.
	// this array is the boundary.
	// seg0 to seg1 is the segment.
	inline bool DoesIntersectBoundary(const AuPoint& seg0, const AuPoint& seg1) const
	{
		AuPoint prev(last());	// Grab last boundary point so first segment tested is the closing segment.
		for(const_iterator it = constBegin(); it != constEnd(); it++)
		{
			if((*it) != prev)
			{
				// Test for intersection with the boundary segment from the iterator to the previous/last point.
				AuPoint intersect;
				if(intersect.SegmentIntersection(seg0, seg1, *it, prev))
					return true;
				prev = (*it);	// Current point becomes previous for next loop iteration.
			}
		}
		return false;
	}

	// Returns true if a segment intersects a boundary.
	// this array is the boundary.
	// seg0 to seg1 is the segment.
	// segHit0 and segHit1 are output parms set to the boundary segment that is hit, if any.
	inline bool DoesIntersectBoundary_2D(const AuPoint& seg0, const AuPoint& seg1, AuPoint& segHit0, AuPoint& segHit1) const
	{
		AuPoint prev(last());	// Grab last boundary point so first segment tested is the closing segment.
		for(const_iterator it = constBegin(); it != constEnd(); it++)
		{
			if((*it) != prev)
			{
				// Test for intersection with the boundary segment from the iterator to the previous/last point.
				AuPoint intersect;
				if(intersect.SegmentIntersection_2D(seg0, seg1, *it, prev))
				{
					// Inform the caller what segment was hit.
					segHit0 = prev;
					segHit1 = *it;
					return true;
				}
				prev = (*it);	// Current point becomes previous for next loop iteration.
			}
		}
		return false;
	}

	// Calculates the dot product of every point in this array onto the vector 'vec'.
	// The min and max of that operation are returned in 'minDot' and 'maxDot'.
	// 'minDot' and 'maxDot' are not modified if this array is empty.
	inline void DotMinMax(const AuPoint& vec, double& minDot, double& maxDot) const
	{
		// 'minDot' and 'maxDot' are not modified if this array is empty.
		if(isEmpty())
			return;

		// Init min and max from the first point in this array.
		const_iterator it = cbegin();
		minDot = (*it).Dot(vec);
		maxDot = minDot;

		// Expand min and max using remaining points in this array.
		for(it++; it != cend(); it++)
		{
			double d = (*it).Dot(vec);
			if(minDot > d)
				minDot = d;
			if(maxDot < d)
				maxDot = d;
		}
	}

	// Walks along the poly line formed by these vertices and returns the location at the request distance along it.
	// Handy for walking around a boundary in specific steps.
	inline AuPoint InterpolateLocationAt(double dist) const
	{
		if(isEmpty())
			return AuPoint(0,0);

		const_iterator it = cbegin();
		if(dist <= 0)
			return *it;

		AuPoint p(*it);
		double pdist = 0;										// Cumulative distance to the previous vertex.
		double ndist = 0;										// Cumulative distance to the next vertex.
		for(it++; it != cend(); it++)
		{
			AuPoint v(*it - p);									// Vector from previous vertex to next vertex.
			double vdist = v.Length();							// Distance from previous vertex to next vertex.
			if(vdist > 0)
			{
				ndist += vdist;
				if(ndist > dist)
				{
					// We have passed the distance that is sought.
					double idist = dist - pdist;					// Distance into the final segment.
					v *= idist/vdist;								// Scale vector to desired distance.
					return p + v;
				}

				// Bump to next vertex.
				p = *it;
				pdist = ndist;
			}
		}

		// We get here if dist is beyond the length of this poly line.
		// Treat it like a closed polyline.
		it = cbegin();										// Wrap it around to the beginning.
		AuPoint v(*it - p);									// Vector from previous vertex to next vertex.
		double vdist = v.Length();							// Distance from previous vertex to next vertex.
		if(vdist > 0)
		{
			ndist += vdist;
			if(ndist > dist)
			{
				// We have passed the distance that is sought.
				double idist = dist - pdist;					// Distance into the final segment.
				v *= idist/vdist;								// Scale vector to desired distance.
				return p + v;
			}
		}

		// We get here if dist is beyond the length of this poly line even if it is considered closed.
		return AuPoint(0,0);
	}

	// Returns true if a segment intersects a boundary.
	// this array is the boundary.
	// seg0 to seg1 is the segment.
	// intersect is set to the intersection found, if any.
	inline bool IntersectBoundary_2D(const AuPoint& seg0, const AuPoint& seg1, AuPoint& intersect) const
	{
		AuPoint prev(last());	// Grab last boundary point so first segment tested is the closing segment.
		for(const_iterator it = constBegin(); it != constEnd(); it++)
		{
			if((*it) != prev)
			{
				// Test for intersection with the boundary segment from the iterator to the previous/last point.
				if(intersect.SegmentIntersection_2D(seg0, seg1, *it, prev))
					return true;
				prev = (*it);	// Current point becomes previous for next loop iteration.
			}
		}
		return false;
	}

	bool InViewRectangle(const AuMatrix4x4& mvp, const QRectF& pixelRect) const;

	inline quint64 MemoryUsage() const
	{
		return sizeof(AuPoint) * capacity();
	}

	inline bool DoesNotContain(const AuPoint &fLoc) const
	{
		return(!DoesContain(fLoc));
	}

	inline void Extents(AuPoint &fMin, AuPoint &fMax) const
	{
		// if no points then just return 0's
		if (isEmpty())
		{
			fMin = AuPoint(0.0, 0.0, 0.0);
			fMax = AuPoint(0.0, 0.0, 0.0);
			return;
		}

		//
		// set initial min and max to first element and then loop for
		// rest of elements
		//
		fMin = at(0);
		fMax = at(0);

		for(int i = 1; i < count(); i++)
		{
			fMin = fMin.Min(at(i));
			fMax = fMax.Max(at(i));
		}
	}

	inline int Find(const AuPoint& loc, double tol=FUZZ) const
	{
		const AuPoint* p = cbegin();
		for(int index=0; index<size(); index++, p++)
			if(p->FuzzyEquals(loc, tol))
				return index;
		return -1;
	}

	inline int Find_2D(const AuPoint& loc)
	{
		const AuPoint* p = cbegin();
		for(int index=0; index<count(); index++, p++)
			if(p->FuzzyEquals_2D(loc))
				return index;
		return -1;
	}

	// Inserts a vector sorted along a direction vector. The order ascends through the array
	// in the direction of the sort direction vector.
	// Assumes the existing vectors in this array are already sorted.
	// NOTE: Inserting into an array is inherently slow.
	inline void InsertSortedVector(const AuPoint &fVec, const AuPoint &fSortDir)
	{
		// The dot product gives us the sort magnitude along the vector.
		double rSortDot = fVec.Dot(fSortDir);

		// Insert the new vector before the first vector that is greater than the new one.
		AuPoint *pLoc = begin();
		for (int lIndex=0; lIndex<count(); lIndex++, pLoc++)
		{
			double rDot = pLoc->Dot(fSortDir);
			if (rSortDot < rDot)
			{
				insert(pLoc, fVec);
				return;
			}
		}

		// If we made it to here, the new vector is greater than all vectors currently in
		// this array. Just add it to the end.
		append(fVec);
	}

	inline void InsertSortedVectorPoly(const AuPoint &fVec, const AuPoint &fSortDir, const AuPoint &fAcrossDir, double rInitAcrossDot);

	// Inserts a vector sorted along a direction vector. The order ascends through the array
	// in the direction of the sort direction vector.
	// Assumes the existing vectors in this array are already sorted.
	// If the vector already exists in this array, it is not added and false is returned.
	// Returns true if the vector is inserted in this array.
	// NOTE: Inserting into an array is inherently slow.
	inline bool InsertSortedVectorIfUnique(const AuPoint &fVec, const AuPoint &fSortDir)
	{
		if (DoesContain(fVec))
			return(false);
		else
		{
			InsertSortedVector(fVec, fSortDir);
			return(true);
		}
	}

	// Find all intersections between a segment and a boundary.
	// this array is the boundary.
	// seg0 to seg1 is the segment.
	// intersects is filled with all the intersections.
	inline bool IntersectBoundary(const AuPoint& seg0, const AuPoint& seg1, AuPointArray& intersects) const
	{
		AuPoint prev(last());	// Grab last boundary point so first segment tested is the closing segment.
		bool intersected = false;
		for(const_iterator it = constBegin(); it != constEnd(); it++)
		{
			if((*it) != prev)
			{
				// Test for intersection with the boundary segment from the iterator to the previous/last point.
				AuPoint intersect;
				if(intersect.SegmentIntersection(seg0, seg1, *it, prev))
				{
					intersects.AddIfUnique(intersect);
					intersected = true;
				}
				prev = (*it);	// Current point becomes previous for next loop iteration.
			}
		}
		return intersected;
	}

	// Find all intersections between a segment and a boundary.
	// this array is the boundary.
	// seg0 to seg1 is the segment.
	// intersects is filled with all the intersections.
	inline bool IntersectBoundary_2D(const AuPoint& seg0, const AuPoint& seg1, AuPointArray& intersects) const
	{
		AuPoint prev(last());	// Grab last boundary point so first segment tested is the closing segment.
		bool intersected = false;
		for(const_iterator it = constBegin(); it != constEnd(); it++)
		{
			if((*it) != prev)
			{
				// Test for intersection with the boundary segment from the iterator to the previous/last point.
				AuPoint intersect;
				if(intersect.SegmentIntersection_2D(seg0, seg1, *it, prev))
				{
					intersects.AddIfUnique(intersect);
					intersected = true;
				}
				prev = (*it);	// Current point becomes previous for next loop iteration.
			}
		}
		return intersected;
	}

	// Find all intersections between a line and a boundary.
	// this array is the boundary.
	// line0 to line1 is the line.
	// intersects is filled with all the intersections.
	inline bool IntersectBoundaryLine_2D(const AuPoint& line0, const AuPoint& line1, AuPointArray& intersects) const
	{
		AuPoint prev(last());	// Grab last boundary point so first segment tested is the closing segment.
		bool intersected = false;
		for(const_iterator it = constBegin(); it != constEnd(); it++)
		{
			if((*it) != prev)
			{
				// Test for intersection with the boundary segment from the iterator to the previous/last point.
				AuPoint intersect;
				if(intersect.SegmentIntersection_2D(line0, line1, *it, prev, false, true))
				{
					intersects.AddIfUnique(intersect);
					intersected = true;
				}
				prev = (*it);	// Current point becomes previous for next loop iteration.
			}
		}
		return intersected;
	}

	void JoinBoundary(const AuPointArray &aPoly);		// Join this boundary with another boundary using a polyline.

	// Moves this array by adding 'p' to each of its points.
	inline void Move(const AuPoint& p)
	{
		for(iterator it = begin(); it != end(); it++)
			(*it) += p;
	}

	// Creates a circle of points centered at 'center' with the first vertex at 'start'.
	// On exit, last vertex != first vertex.
	void MakeCircle(const AuPoint& center, const AuPoint& start, int steps);

	inline void operator>>(QDataStream& out) const
	{
		Q_ASSERT(count() <= 0xffff);	// We don't intend to serialize more than 65k points in an array.
		out << (quint16)count();
		for(const_iterator it = constBegin(); it != constEnd(); it++)
			out << *it;
	}

	inline void operator<<(QDataStream& in)
	{
		clear();
		quint16 cnt;
		in >> cnt;
		AuPoint p;
		for(; cnt; cnt--)
		{
			in >> p;
			append(p);
		}
	}

	// return overlap condition between 'this' boundary and
	// another boundary.  boundaries are assumed to be closed
	// polygons.  This is a 2d routine, the z coordinates are ignored.
	// 'this' is assumed to be the paren boundary.
	// return values are
	//   NoOverlap - boundaries do not overlap
	//   Overlapped - boundaries overlap
	//   ContainsChild - 'this' surrounds the child boundary
	//   ContainedByChild - 'this' is surrounded by child
	inline OverlapStatus Overlap(const AuPointArray &aChild) const
	{
		OverlapStatus eRetval = NoOverlap;

		//
		// make sure both boundaries are valid
		//
		if ( (count() < 3) || (aChild.count() < 3) )
			return eRetval;

		//
		// get the rectangular extents of each boundary and check
		// for overlap of the rectangular extents
		//
		AuPoint fParentMin, fParentMax, fChildMin, fChildMax;
		Extents(fParentMin, fParentMax);
		aChild.Extents(fChildMin, fChildMax);
		if ( (fChildMin.x > fParentMax.x) ||
			 (fChildMax.x < fParentMin.x) ||
			 (fChildMin.y > fParentMax.y) ||
			 (fChildMax.y < fParentMin.y) )
			 return eRetval;
		//
		// at this point it is possible that the boundaries overlap.
		// to test, need to try and find any intersections between
		// the boundaries.
		//
		// if there are at least 2 unique intersections then there is overlap.
		//
		int ip2, ic2;
		AuPointArray aIntersections;
		for (int iparent = 0; iparent < count() ; iparent++)
		{
			// get indices for parent segment
			ip2 = (iparent != count() - 1) ? iparent+1 : 0;
			
			for (int ichild=0; ichild < aChild.count(); ichild++)
			{
				// get indices for child segment
				ic2 = (ichild != aChild.count() - 1) ? ichild+1 : 0;

				// Get the segment vertices that we care about.
				AuPoint fc1(aChild.at(ichild));
				fc1.z = 0.0;
				AuPoint fc2(aChild.at(ic2));
				fc2.z = 0.0;
				double rcl = fc1.Distance_2D(fc2);
				if(::IsFuzzyZero(rcl))
					continue;

				AuPoint fb1(at(iparent));
				fb1.z = 0.0;
				AuPoint fb2(at(ip2));
				fb2.z = 0.0;
				double rbl = fb1.Distance_2D(fb2);
				if(::IsFuzzyZero(rbl))
					continue;

				// Test the segments for intersection.
				AuPoint fint;
				if(fint.SegmentIntersection(fc1, fc2, fb1, fb2))
				{
					aIntersections.AddIfUnique(fint);
					if(aIntersections.count() > 1)
					{
						eRetval = Overlapped;
						break;
					}
				}
			}
			if(eRetval == Overlapped)
				break;
		}

		//
		// if still no overlap has been found, then need to check for containment
		//
		if (eRetval == NoOverlap)
		{
			//
			// if there was one intersection point then these boundaries
			// are tangent.  We do not consider that an overlap case.
			//
			if (aIntersections.count() == 0)
			{
				//
				// test if parent contains the child
				//
				if(PointInBoundary(aChild.at(0)))
					eRetval = ContainsChild;
				else if(aChild.PointInBoundary(at(0)))
					eRetval = ContainedByChild;
			}
		}

		return eRetval;
		
	}

	// 3D distance to closest seg
	inline double DistToBoundary(const AuPoint& loc) const
	{
		if(count() < 2)
			return -1.;

		// Check segments of the array.
		double bseg = -1.;
		bool haseg = false;
		double dseg;
		for(const_iterator it = constBegin()+1; it != constEnd(); it++)
		{
			dseg = loc.DistToSegment(*(it-1), *it);
			if(!haseg || dseg < bseg)
			{
				bseg = dseg;
				haseg = true;
			}
		}

		// Check closing segment if there is more than two vertices.
		if(count() > 2)
		{
			dseg = loc.DistToSegment(last(), first());
			if(!haseg || dseg < bseg)
			{
				bseg = dseg;
				haseg = true;
			}
		}

		return bseg;
	}

	inline double DistToBoundary_2D(const AuPoint& loc) const	// 2D distance to closest seg
	{
		if (count() < 2)
			return -1.;

		// Check segments of the array.
		double bseg = -1.;
		bool haseg = false;
		double dseg;
		for(const_iterator it = constBegin()+1; it != constEnd(); it++)
		{
			dseg = loc.DistToSegment_2D(*(it-1), *it);
			if(!haseg || dseg < bseg)
			{
				bseg = dseg;
				haseg = true;
			}
		}

		// Check closing segment if there is more than two vertices.
		if(count() > 2)
		{
			dseg = loc.DistToSegment_2D(last(), first());
			if(!haseg || dseg < bseg)
			{
				bseg = dseg;
				haseg = true;
			}
		}

		return bseg;
	}

	inline double ClosestBoundarySegment_2D(const AuPoint& loc, AuPoint& seg0,
			AuPoint& seg1) const
	{
		if (count() < 2)
			return -1.;

		// Check segments of the array.
		double bseg = -1.;
		bool haseg = false;
		double dseg;
		for(const_iterator it = constBegin()+1; it != constEnd(); it++)
		{
			dseg = loc.DistToSegment_2D(*(it-1), *it);
			if(!haseg || dseg < bseg)
			{
				haseg = true;
				bseg = dseg;
				seg0 = *(it-1);
				seg1 = *it;
			}
		}

		// Check closing segment if there is more than two vertices.
		if(count() > 2)
		{
			dseg = loc.DistToSegment_2D(last(), first());
			if(!haseg || dseg < bseg)
			{
				haseg = true;
				bseg = dseg;
				seg0 = last();
				seg1 = first();
			}
		}

		return bseg;
	}

	inline double SecondClosestBoundarySegment_2D(const AuPoint& loc, AuPoint& seg0, AuPoint& seg1) const
	{
		if(count() < 2)
			return -1.;

		// Check segments of the array.
		double bseg = -1.;		// distance to closest boundary segment
		double bseg2 = -1.;		// distance to second closest boundary segment
		AuPoint seg0b, seg1b;	// current closest boundary segment
		bool haseg = false;
		bool haseg2 = false;
		double dseg;
		for(const_iterator it = constBegin()+1; it != constEnd(); it++)
		{
			dseg = loc.DistToSegment_2D(*(it-1), *it);
			if(!haseg)
			{
				// Store best seg info.
				haseg = true;
				bseg = dseg;
				seg0b = *(it-1);
				seg1b = *it;
			}
			else if(dseg < bseg)
			{
				// Previous best seg becomes the second best.
				haseg2 = true;
				bseg2 = bseg;
				seg0 = seg0b;
				seg1 = seg1b;

				// Store best seg info.
				bseg = dseg;
				seg0b = *(it-1);
				seg1b = *it;
			}
			else if(!haseg2 || dseg < bseg2)
			{
				// Store second best seg info.
				haseg2 = true;
				bseg2 = dseg;
				seg0 = *(it-1);
				seg1 = *it;
			}
		}

		// Check closing segment if there is more than two vertices.
		if(count() > 2)
		{
			dseg = loc.DistToSegment_2D(last(), first());
			if(!haseg)
			{
				// Store best seg info.
				haseg = true;
				bseg = dseg;
				seg0b = last();
				seg1b = first();
			}
			else if(dseg < bseg)
			{
				// Previous best seg becomes the second best.
				haseg2 = true;
				bseg2 = bseg;
				seg0 = seg0b;
				seg1 = seg1b;

				// Store best seg info.
				bseg = dseg;
				seg0b = last();
				seg1b = first();
			}
			else if(!haseg2 || dseg < bseg2)
			{
				// Store second best seg info.
				haseg2 = true;
				bseg2 = dseg;
				seg0 = last();
				seg1 = first();
			}
		}

		return bseg2;
	}

	// Returns true if fLoc is in the 2D boundary formed by this array of points.
	// The point is in the boundary if an odd number of boundary segments must be crossed to leave the boundary.
	inline bool PointInBoundary(const AuPoint& loc) const
	{
		// Insure this array contains enough vertices to form a boundary.
		if(count() < 3)
			return false;

		// Get the extents of this boundary.
		AuPoint minpt, maxpt;
		Extents(minpt, maxpt);
		if(loc.x <= minpt.x || loc.x >= maxpt.x || loc.y <= minpt.y || loc.y >= maxpt.y)
			return false;	// Point is outside the extents of the boundary.

		// Calculate the other end of a segment that can be tested against the boundary segments.
		AuPoint segVec(maxpt - minpt);
		segVec.z = 0;
		segVec = segVec.RotateZ(DEG2RAD);	// Rotate test segment by a degree so we're less likely to hit a vertex the first time.
		AuPoint segEnd(loc + segVec);

		// Count the number of boundary segments that intersect the test segment.
		bool hitVertex;
		int intersects;
		int tries = 0;
		do
		{
			hitVertex = false;
			intersects = 0;
			AuPoint prev(last());	// Grab last boundary point so first segment tested is the closing segment.
			AuPoint intersect, fInt2;
			for(const_iterator it = constBegin(); it != constEnd(); it++)
			{
				if((*it) != prev)
				{
					// Test for intersection with the boundary segment from the iterator to the previous/last point.
					// This code was cobbled from SegmentIntersection_2D.
					// Perform 2D perpconn.
					if(!AuPoint(loc.x,loc.y,0).PerpConn(AuPoint(segEnd.x,segEnd.y,0),
								AuPoint((*it).x,(*it).y,0), AuPoint(prev.x,prev.y,0), intersect, fInt2))
					{
						// Segments are parallel. This is a problem if they overlap.
						if(::IsFuzzyZero((*it).DistToSegment_2D(loc,segEnd)))
							hitVertex = true;	// Close enough of an overlap that we should rotate the test vector.
					}
					else if(intersect.FuzzyEquals_2D(fInt2) && intersect.Between_2D(loc,segEnd) && intersect.Between_2D(*it,prev))
					{
						// The test segment intersected the boundary segment.
						if(intersect.FuzzyEquals_2D(loc))
							return false;	// Test location is on the boundary segment.
						if(intersect.FuzzyEquals_2D(*it) || intersect.FuzzyEquals_2D(prev))
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

	// Returns true if fLoc is on the boundary formed by this array of points.
	inline bool PointOnBoundary(const AuPoint &fLoc) const
	{
		if (count() < 2)
			return false;

		// Check segments of the array.
		for(const_iterator it = constBegin()+1; it != constEnd(); it++)
			if(fLoc.IsOnSegment(*(it-1), *it))
				return true;

		// Check closing segment if there is more than two vertices.
		if(count() > 2)
			if(fLoc.IsOnSegment(last(), first()))
				return true;

		return false;
	}

	// Returns true if fLoc is on the boundary formed by this array of points.
	inline bool PointOnBoundary_2D(const AuPoint &fLoc) const
	{
		if (count() < 2)
			return false;

		// Check segments of the array.
		for(const_iterator it = constBegin()+1; it != constEnd(); it++)
			if(fLoc.IsOnSegment_2D(*(it-1), *it))
				return true;

		// Check closing segment if there is more than two vertices.
		if(count() > 2)
			if(fLoc.IsOnSegment_2D(last(), first()))
				return true;

		return false;
	}

	// Returns true if fLoc is on the segments formed by this array of points.
	// Just like PointOnBoundary except that the closing segment of the boundary is not tested.
	inline bool PointOnSegments(const AuPoint &fLoc) const
	{
		if (count() < 2)
			return false;

		// Check segments of the array.
		for(const_iterator it = constBegin()+1; it != constEnd(); it++)
			if(fLoc.IsOnSegment(*(it-1), *it))
				return true;

		return false;
	}

	// Returns true if fLoc is on the segments formed by this array of points.
	// Just like PointOnBoundary_2D except that the closing segment of the boundary is not tested.
	inline bool PointOnSegments_2D(const AuPoint &fLoc) const
	{
		if (count() < 2)
			return false;

		// Check segments of the array.
		for(const_iterator it = constBegin()+1; it != constEnd(); it++)
			if(fLoc.IsOnSegment_2D(*(it-1), *it))
				return true;

		return false;
	}

	// Removes sequential duplicates, including start/end assuming closed boundary condition.
	inline void RemoveBoundaryDuplicates()	
	{
		if (count() <= 1)
			return;

		for(int ip=1; ip<count();)
		{
			if (at(ip).FuzzyEquals(at(ip-1)))
				erase(begin()+ip);
			else
				ip++;
		}

		while (count() > 1 && at(count()-1).FuzzyEquals(at(0)))
			erase(end());
	}

	inline void RemoveBoundaryDuplicates_2D()		// Removes sequential duplicates, including start/end assuming closed boundary condition.
	{
		if(count() <= 1)
			return;

		for(int ip=1; ip<count();)
		{
			if (at(ip).FuzzyEquals_2D(at(ip-1)))
				erase(begin()+ip);
			else
				ip++;
		}

		while (count() > 1 && at(count()-1).FuzzyEquals_2D(at(0)))
			erase(end());
	}
	inline void RemoveColinearPoints_2D()         // removes points that aren't needed for the boundary.
	{
		if (count() < 3)
			return;

		//
		// take points in groups of 3
		//
		AuPoint vPrev, vNext;
		int iPrev, iCur, iNext;
		for (iCur=0; iCur < count();)
		{
			iPrev = iCur - 1;
			iPrev = (iCur == 0) ? count()-1 : iPrev;
			iCur = (iCur >= count()) ? 0 : iCur;
			iNext = (iCur >= count() - 1) ? 0 : iCur+1;

			//
			// build the previous and next vectors and test if they are colinear
			//
			vPrev = at(iCur) - at(iPrev);
			vPrev.z = 0.0;
			vPrev.Unit();
			vNext = at(iNext) - at(iCur);
			vNext.z = 0.0;
			vNext.Unit();
			if (vPrev.IsParallel(vNext))
			{
				erase(begin()+iCur);
			}
			else
				iCur++;

			if (count() < 3)
				break;
		}
	}
	inline void ReverseOrder()					// Reverses the order of the elements in this array.
	{
		AuPoint *pf = begin();					// Pointer to first element in the array.
		AuPoint *pl = begin() + count() - 1;	// Pointer to last element in the array.
		int iswaps = count()/2;					// Number of swaps needed to reverse the order.
		for (int iswp=0; iswp<iswaps; iswp++, pf++, pl--)
		{
			AuPoint ftmp(*pf);
			*pf = *pl;
			*pl = ftmp;
		}
	}

	void Transform(const AuMatrix4x4 &t);	// Applies the transformation t to this array of points.

	void TransformVectors(const AuMatrix4x4 &t);	// Applies the transformation t to this array of points.

	// Add 'vec' to all locations in this array.
	inline void Translate(const AuPoint &vec)
	{
		for(iterator it = begin(); it != end(); it++)
			*it += vec;
	}

	// Adds the X,Y values of 'vec' to all locations in this array. Z values are unchanged.
	inline void Translate_2D(const AuPoint &vec)
	{
		for(iterator it = begin(); it != end(); it++)
		{
			(*it).x += vec.x;
			(*it).y += vec.y;
		}
	}

	void Triangulate(AuPointArray &pTri, QVector<bool> &bTedge) const;
	void Triangulate(QVector<bool> &bBedge, AuPointArray &pTri, QVector<bool> &bTedge);
	void TriangulateBoundary(AuPointArray &pTri, QVector<bool> &bTedge) const;
	inline void AddBoundaryOpenings(QVector<bool> &bBedge, const AuPointArray &pOpen)
	{
		AddBoundaryOpeningsCut(bBedge, pOpen, 0.0);
	}

	void AddBoundaryOpeningsCut(QVector<bool> &bBedge, const AuPointArray &pOpen, double rDistToCut);
	inline bool IsClockwise() const	// Returns true if the boundary formed by these points is clockwise.
	{
		return SubArea() >= 0.0;	// SubArea returns negative for counterclockwise boundaries.
	}

	inline double Area() const			// Returns the 2D area of the boundary formed by these points.
	{
		return fabs(SubArea());
	}

	inline double Perimeter() const	// Returns the 2D perimeter of the boundary formed by these points.
	{
		if (count() < 3)
			return 0.0;			// No perimeter if less than three vertices.

		double perim = 0.0;		// This is the perimeter value we will return.
		const AuPoint *cur = begin();				// Start cur at first array entry.
		const AuPoint *prev = begin() + count() - 1;	// Start prev at last for closure.
		for(int lvert=0; lvert<count(); lvert++, cur++)
		{
			perim += prev->Distance_2D(*cur);
			prev = cur;
		}

		// Return the accumulated perimeter value.
		return perim;
	}

	inline int FindClosest(const AuPoint &fSearchPt) const
	{
		double closestDist = 0.0;
		int iclosest = -1;
		const AuPoint *p = begin();
		for(int ip=0; ip<count(); ip++, p++)
		{
			double rdist = p->Distance2(fSearchPt);
			if(iclosest == -1 || rdist < closestDist)
			{
				closestDist = rdist;
				iclosest = ip;
				if(closestDist == 0.0)
					break;
			}
		}
		return iclosest;
	}

	// 'closestDist' is an output parameter, only valid if return index is >= 0.
	inline int FindClosest(const AuPoint &fSearchPt, double& closestDist) const
	{
		closestDist = 0.0;
		int iclosest = -1;
		const AuPoint *p = begin();
		for(int ip=0; ip<count(); ip++, p++)
		{
			double rdist = p->Distance2(fSearchPt);
			if(iclosest == -1 || rdist < closestDist)
			{
				closestDist = rdist;
				iclosest = ip;
				if(closestDist == 0.0)
					return iclosest;
			}
		}
		closestDist = sqrt(closestDist);
		return iclosest;
	}

	inline int FindClosest_2D(const AuPoint &fSearchPt) const
	{
		double rclosest = 0.0;
		int iclosest = -1;
		const AuPoint *p = begin();
		for(int ip=0; ip<count(); ip++, p++)
		{
			double rdist = p->Distance2_2D(fSearchPt);
			if(iclosest == -1 || rdist < rclosest)
			{
				rclosest = rdist;
				iclosest = ip;
				if(rclosest == 0.0)
					break;
			}
		}
		return iclosest;
	}

	inline bool operator==(const AuPointArray &c) const
	{
		int lMyCount = count();
		int lOtherCount = c.count();
		if (lMyCount != lOtherCount)
			return false;

		const AuPoint *pMyLoc = begin();
		const AuPoint *pOtherLoc = c.begin();
		for (;lMyCount; lMyCount--, pMyLoc++, pOtherLoc++)
		{
			if (*pMyLoc != *pOtherLoc)
				return false;
		}

		return true;
	}

	inline bool operator!=(const AuPointArray &c) const
	{
		return !operator==(c);
	}

private:
	// Returns the area of the boundary formed by this array of points.
	// Assumes the boundary lines do not cross.
	// The area value is positive if the boundary is clockwise, negative if the boundary is counterclockwise.
	// The result of this is dubious.
	inline double SubArea() const
	{
		if (count() < 3)
			return 0.0;			// No area if less than three vertices.

		double rArea = 0.0;		// This will be the accumulated area value that we return.

		const AuPoint *pStart = begin();
		const AuPoint *pCurLoc = pStart + 1;
		AuPoint fVec1(*pCurLoc - *pStart);		// Vector from start to first vertex.
		fVec1.z = 0.0;
		pCurLoc++;
		for (int lvert=2; lvert<count(); lvert++, pCurLoc++)
		{
			// Calculate the area of the triangle formed by this vertex, the previous vertex, and vertex 0.
			// We will add the area of clockwise triangles and subtract the area of counterclockwise triangles.
			AuPoint fVec2(*pCurLoc - *pStart);	// Vector to next vertex.
			fVec2.z = 0.0;
			switch (pCurLoc->SideOfLine_2D(*pStart,*(pCurLoc-1)))
			{
			case 0:		// on the line -> no area
				break;
			case 1:		// left of line
				rArea -= fVec1.Cross(fVec2).Length()/2.0;	// Subtract area of triangle formed by the two vectors.
				break;
			case -1:	// right of line
				rArea += fVec1.Cross(fVec2).Length()/2.0;	// Add area of triangle formed by the two vectors.
				break;
			}
			fVec1 = fVec2;
		}

		// The area value is currently negative if the boundary was counterclockwise.
		return rArea;
	}
};

AUMATH_DLL inline QDataStream& operator<<(QDataStream& out, const AuPointArray& p)
{
	p >> out;
	return out;
}

AUMATH_DLL inline QDataStream& operator>>(QDataStream& in, AuPointArray& p)
{
	p << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// A handy array object for AuPoints
class AUMATH_DLL AuPointList : public QList<AuPoint>
{
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUMATH_DLL AuPointTex : public AuPoint
{
public:
	float								m_Tx;					// Texture X coordinate at this location.
	float								m_Ty;					// Texture Y coordinate at this location.

public:	// Constructors
	inline AuPointTex(const AuPoint& loc, const QPointF& texCoord) : AuPoint(loc)
	{
		m_Tx = texCoord.x();
		m_Ty = texCoord.y();
	}

	inline AuPointTex(const AuPoint& loc, float texX, float texY) : AuPoint(loc)
	{
		m_Tx = texX;
		m_Ty = texY;
	}

	inline AuPointTex(double x, double y, double z, float texX, float texY) : AuPoint(x, y, z)
	{
		m_Tx = texX;
		m_Ty = texY;
	}

	inline AuPointTex()
	{
		m_Tx = 0.f;
		m_Ty = 0.f;
	}

	inline AuPointTex(const AuPointTex& c) : AuPoint(c)
	{
		m_Tx = c.m_Tx;
		m_Ty = c.m_Ty;
	}

public:	// Operators
	inline const AuPointTex& operator=(const AuPointTex& c)
	{
		AuPoint::operator=(c);
		m_Tx = c.m_Tx;
		m_Ty = c.m_Ty;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		AuPoint::operator>>(out);
		out << m_Tx;
		out << m_Ty;
	}

	inline void operator<<(QDataStream& in)
	{
		AuPoint::operator<<(in);
		in >> m_Tx;
		in >> m_Ty;
	}

	inline bool operator==(const AuPointTex& p) const
	{
		if(!AuPoint::operator==(p))
			return false;
		if(m_Tx != p.m_Tx)
			return false;
		if(m_Ty != p.m_Ty)
			return false;
		return true;
	}

	inline bool operator!=(const AuPointTex& p) const
	{
		return !operator==(p);
	}

public:	// Methods
	inline AuPointTex LinearInterpolate(const AuPointTex& o, double frac) const
	{
		float mfrac = 1.f - (float)frac;
		float ofrac = (float)frac;
		float tx = m_Tx * mfrac + m_Tx * ofrac;
		float ty = m_Ty * mfrac + m_Ty * ofrac;
		return AuPointTex(AuPoint::LinearInterpolate(o, frac), tx,ty);
	}

	inline void SetCoord(double cx, double cy, double cz)
	{
		x = cx;
		y = cy;
		z = cz;
	}

	inline void SetCoord(const AuPoint& c)
	{
		x = c.x;
		y = c.y;
		z = c.z;
	}

	inline void SetTexCoord(float s, float t)
	{
		m_Tx = s;
		m_Ty = t;
	}

	inline QPointF TexCoord() const
	{
		return QPointF(m_Tx, m_Ty);
	}
};

AUMATH_DLL inline QDataStream& operator<<(QDataStream& out, const AuPointTex& o)
{
	o >> out;
	return out;
}

AUMATH_DLL inline QDataStream& operator>>(QDataStream& in, AuPointTex& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUMATH_DLL AuPointTexN : public AuPointTex
{
public:
	AuPoint								m_N;					// Surface normal at this location.

public:	// Constructors
	inline AuPointTexN(const AuPoint& loc, const QPointF& texCoord, const AuPoint& surfN) : AuPointTex(loc, texCoord)
	{
		m_N = surfN;
	}

	inline AuPointTexN(const AuPoint& loc, float texX, float texY, const AuPoint& surfN) : AuPointTex(loc, texX, texY)
	{
		m_N = surfN;
	}

	inline AuPointTexN(double x, double y, double z, float texX, float texY, const AuPoint& surfN) : AuPointTex(x, y, z, texX, texY)
	{
		m_N = surfN;
	}

	inline AuPointTexN()
	{
	}

	inline AuPointTexN(const AuPointTexN& c) : AuPointTex(c)
	{
		m_N = c.m_N;
	}

	inline AuPointTexN(const AuPointTex& c, const AuPoint& n) : AuPointTex(c)
	{
		m_N = n;
	}

public:	// Operators
	inline const AuPointTexN& operator=(const AuPointTexN& c)
	{
		AuPointTex::operator=(c);
		m_N = c.m_N;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		AuPointTex::operator>>(out);
		out << m_N;
	}

	inline void operator<<(QDataStream& in)
	{
		AuPointTex::operator<<(in);
		in >> m_N;
	}

	inline bool operator==(const AuPointTexN& p) const
	{
		if(!AuPointTex::operator==(p))
			return false;
		if(m_N != p.m_N)
			return false;
		return true;
	}

	inline bool operator!=(const AuPointTexN& p) const
	{
		return !operator==(p);
	}

public:	// Methods
	inline AuPointTexN LinearInterpolate(const AuPointTexN& o, double frac) const
	{
		AuPoint n = m_N * (1-frac) + o.m_N * frac;
		n.Unit();
		return AuPointTexN(AuPointTex::LinearInterpolate(o, frac), n);
	}

	void Transform(const AuMatrix4x4& t);
};

AUMATH_DLL inline QDataStream& operator<<(QDataStream& out, const AuPointTexN& o)
{
	o >> out;
	return out;
}

AUMATH_DLL inline QDataStream& operator>>(QDataStream& in, AuPointTexN& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUMATH_DLL AuPointTexNS : public AuPointTexN
{
public:
	float								m_Shine;

public:	// Constructors
	inline AuPointTexNS(const AuPoint& loc, const QPointF& texCoord, const AuPoint& surfN, float shine) :
		AuPointTexN(loc, texCoord, surfN)
	{
		m_Shine = shine;
	}

	inline AuPointTexNS(const AuPoint& loc, float texX, float texY, const AuPoint& surfN, float shine) :
		AuPointTexN(loc, texX, texY, surfN)
	{
		m_Shine = shine;
	}

	inline AuPointTexNS(double x, double y, double z, float texX, float texY, const AuPoint& surfN, float shine) :
		AuPointTexN(x, y, z, texX, texY, surfN)
	{
		m_Shine = shine;
	}

	inline AuPointTexNS(const AuPointTexN& loc, float shine) : AuPointTexN(loc)
	{
		m_Shine = shine;
	}

	inline AuPointTexNS()
	{
		m_Shine = 0.f;
	}

	inline AuPointTexNS(const AuPointTexNS& c) : AuPointTexN(c)
	{
		m_Shine = c.m_Shine;
	}

public:	// Operators
	inline const AuPointTexNS& operator=(const AuPointTexNS& c)
	{
		AuPointTexN::operator=(c);
		m_Shine = c.m_Shine;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		AuPointTexN::operator>>(out);
		out << m_Shine;
	}

	inline void operator<<(QDataStream& in)
	{
		AuPointTexN::operator<<(in);
		in >> m_Shine;
	}

public:	// Methods
	inline float Shine() const
	{
		return m_Shine;
	}
};

AUMATH_DLL inline QDataStream& operator<<(QDataStream& out, const AuPointTexNS& o)
{
	o >> out;
	return out;
}

AUMATH_DLL inline QDataStream& operator>>(QDataStream& in, AuPointTexNS& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Vertex of a creature body model.

class AUMATH_DLL AuPointBody : public AuPointTexNS
{
public:
	int									m_BoneIndex[4];
	float								m_BoneWeight[4];
	bool								m_TopFace;				// True to use top skin texture, false for bottom skin texture.

public:
	inline AuPointBody(const AuPoint& loc, const QPointF& texCoord, const AuPoint& surfN, float shine, int boneIndex[4],
			float boneWeight[4], bool topFace) :
		AuPointTexNS(loc, texCoord, surfN, shine)
	{
		for(int ib=0; ib<4; ib++)
		{
			m_BoneIndex[ib] = boneIndex[ib];
			m_BoneWeight[ib] = boneWeight[ib];
		}
		m_TopFace = topFace;
	}

	inline AuPointBody(const AuPoint& loc, float texX, float texY, const AuPoint& surfN, float shine, int boneIndex[4],
			float boneWeight[4], bool topFace) :
		AuPointTexNS(loc, texX, texY, surfN, shine)
	{
		for(int ib=0; ib<4; ib++)
		{
			m_BoneIndex[ib] = boneIndex[ib];
			m_BoneWeight[ib] = boneWeight[ib];
		}
		m_TopFace = topFace;
	}

	inline AuPointBody(double x, double y, double z, float texX, float texY, const AuPoint& surfN, float shine, int boneIndex[4],
			float boneWeight[4], bool topFace) :
		AuPointTexNS(x, y, z, texX, texY, surfN, shine)
	{
		for(int ib=0; ib<4; ib++)
		{
			m_BoneIndex[ib] = boneIndex[ib];
			m_BoneWeight[ib] = boneWeight[ib];
		}
		m_TopFace = topFace;
	}

	inline AuPointBody()
	{
		for(int ib=0; ib<4; ib++)
		{
			m_BoneIndex[ib] = 0;
			m_BoneWeight[ib] = 0;
		}
		m_TopFace = false;
	}

	inline AuPointBody(const AuPointBody& c) : AuPointTexNS(c)
	{
		for(int ib=0; ib<4; ib++)
		{
			m_BoneIndex[ib] = c.m_BoneIndex[ib];
			m_BoneWeight[ib] = c.m_BoneWeight[ib];
		}
		m_TopFace = c.m_TopFace;
	}

public:	// Operators
	inline const AuPointBody& operator=(const AuPointBody& c)
	{
		AuPointTexNS::operator=(c);
		for(int ib=0; ib<4; ib++)
		{
			m_BoneIndex[ib] = c.m_BoneIndex[ib];
			m_BoneWeight[ib] = c.m_BoneWeight[ib];
		}
		m_TopFace = c.m_TopFace;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		AuPointTexNS::operator>>(out);
		for(int ib=0; ib<4; ib++)
		{
			out << m_BoneIndex[ib];
			out << m_BoneWeight[ib];
		}
		out << m_TopFace;
	}

	inline void operator<<(QDataStream& in)
	{
		AuPointTexNS::operator<<(in);
		for(int ib=0; ib<4; ib++)
		{
			in >> m_BoneIndex[ib];
			in >> m_BoneWeight[ib];
		}
		in >> m_TopFace;
	}
};

AUMATH_DLL inline QDataStream& operator<<(QDataStream& out, const AuPointBody& o)
{
	o >> out;
	return out;
}

AUMATH_DLL inline QDataStream& operator>>(QDataStream& in, AuPointBody& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Vertex of a spacecraft model.

class AUMATH_DLL AuPointSpacecraft : public AuPointTexNS
{
public:
	float								m_EffTx;				// Surface effect texture X coordinate at this location.
	float								m_EffTy;				// Surface effect texture Y coordinate at this location.
	int									m_TexOffIndex[3];		// Texture offset indexes for hull, lit effect, and unlit effect.

public:
	inline AuPointSpacecraft(const AuPoint& loc, const QPointF& texCoord, const AuPoint& surfN, float shine,
			const QPointF& effTexCoord, int texOffIndex[3]) :
		AuPointTexNS(loc, texCoord, surfN, shine)
	{
		m_EffTx = effTexCoord.x();
		m_EffTy = effTexCoord.y();
		for(int to=0; to<3; to++)
			m_TexOffIndex[to] = texOffIndex[to];
	}

	inline AuPointSpacecraft(const AuPoint& loc, float texX, float texY, const AuPoint& surfN, float shine,
			float effTexX, float effTexY, int texOffIndex[3]) :
		AuPointTexNS(loc, texX, texY, surfN, shine)
	{
		m_EffTx = effTexX;
		m_EffTy = effTexY;
		for(int to=0; to<3; to++)
			m_TexOffIndex[to] = texOffIndex[to];
	}

	inline AuPointSpacecraft(double x, double y, double z, float texX, float texY, const AuPoint& surfN, float shine,
			float effTexX, float effTexY, int texOffIndex[3]) :
		AuPointTexNS(x, y, z, texX, texY, surfN, shine)
	{
		m_EffTx = effTexX;
		m_EffTy = effTexY;
		for(int to=0; to<3; to++)
			m_TexOffIndex[to] = texOffIndex[to];
	}

	inline AuPointSpacecraft()
	{
		m_EffTx = 0;
		m_EffTy = 0;
		for(int to=0; to<3; to++)
			m_TexOffIndex[to] = 0;
	}

	inline AuPointSpacecraft(const AuPointSpacecraft& c) : AuPointTexNS(c)
	{
		m_EffTx = c.m_EffTx;
		m_EffTy = c.m_EffTy;
		for(int to=0; to<3; to++)
			m_TexOffIndex[to] = c.m_TexOffIndex[to];
	}

public:	// Operators
	inline const AuPointSpacecraft& operator=(const AuPointSpacecraft& c)
	{
		AuPointTexNS::operator=(c);
		m_EffTx = c.m_EffTx;
		m_EffTy = c.m_EffTy;
		for(int to=0; to<3; to++)
			m_TexOffIndex[to] = c.m_TexOffIndex[to];
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		AuPointTexNS::operator>>(out);
		out << m_EffTx;
		out << m_EffTy;
		for(int to=0; to<3; to++)
			out << m_TexOffIndex[to];
	}

	inline void operator<<(QDataStream& in)
	{
		AuPointTexNS::operator<<(in);
		in >> m_EffTx;
		in >> m_EffTy;
		for(int to=0; to<3; to++)
			in >> m_TexOffIndex[to];
	}
};

AUMATH_DLL inline QDataStream& operator<<(QDataStream& out, const AuPointSpacecraft& o)
{
	o >> out;
	return out;
}

AUMATH_DLL inline QDataStream& operator>>(QDataStream& in, AuPointSpacecraft& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// A location with an alpha value. Used for sprites.

class AUMATH_DLL AuPointAlpha : public AuPoint
{
public:
	float								m_Alpha;				// Alpha 0..1

public:
	inline AuPointAlpha(const AuPoint& loc, float alpha=1.f) : AuPoint(loc)
	{
		m_Alpha = alpha;
	}

	inline AuPointAlpha(double x, double y, double z, float alpha=1.f) : AuPoint(x, y, z)
	{
		m_Alpha = alpha;
	}

	inline AuPointAlpha()
	{
		m_Alpha = 1.f;
	}

	inline AuPointAlpha(const AuPointAlpha& c) : AuPoint(c)
	{
		m_Alpha = c.m_Alpha;
	}

public:	// Operators
	inline const AuPointAlpha& operator=(const AuPointAlpha& c)
	{
		AuPoint::operator=(c);
		m_Alpha = c.m_Alpha;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		AuPoint::operator>>(out);
		out << m_Alpha;
	}

	inline void operator<<(QDataStream& in)
	{
		AuPoint::operator<<(in);
		in >> m_Alpha;
	}

	inline void SetAlpha(float alpha)
	{
		m_Alpha = alpha;
	}

	inline void SetCoord(double cx, double cy, double cz)
	{
		x = cx;
		y = cy;
		z = cz;
	}

	inline void SetCoord(const AuPoint& c)
	{
		x = c.x;
		y = c.y;
		z = c.z;
	}

	void Transform(const AuMatrix4x4 t);
};

AUMATH_DLL inline QDataStream& operator<<(QDataStream& out, const AuPointAlpha& o)
{
	o >> out;
	return out;
}

AUMATH_DLL inline QDataStream& operator>>(QDataStream& in, AuPointAlpha& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUMATH_DLL AuPoint2d
{
public:
	double								x;
	double								y;

public:	// Constructors
	inline AuPoint2d()
	{
		x = 0;
		y = 0;
	}

	inline AuPoint2d(const double i, const double j)
	{
		x = i;
		y = j;
	}
	
	inline AuPoint2d(const AuPoint2d& c)
	{
		x = c.x;
		y = c.y;
	}

	inline AuPoint2d(const QPoint& p)
	{
		x = p.x();
		y = p.y();
	}

	inline AuPoint2d(const QPointF& p)
	{
		x = p.x();
		y = p.y();
	}

public:	// Operators
	inline AuPoint2d operator+(const AuPoint2d &p) const
		{return(AuPoint2d(x + p.x, y + p.y));}

	inline AuPoint2d operator-(const AuPoint2d &p) const
		{return(AuPoint2d(x - p.x, y - p.y));}

	inline AuPoint2d operator-() const
		{return(AuPoint2d(-x, -y));}

	inline AuPoint2d operator*(double r) const
		{return((r != 0.) ? AuPoint2d(x * r, y * r) : AuPoint2d(0,0));}

	inline AuPoint2d operator/(double r) const
		{return((r != 0.) ? AuPoint2d(x / r, y / r) : AuPoint2d(0,0));}

	inline const AuPoint2d& operator=(const AuPoint2d &p)
		{x = p.x; y = p.y; return(*this);}

	inline bool operator==(const AuPoint2d &p) const
		{return (x == p.x) && (y == p.y);}

	inline bool operator!=(const AuPoint2d &p) const
		{return (x != p.x) || (y != p.y);}

	inline bool operator>=(const AuPoint2d &p) const	// true if ALL components are >= corresponding components
		{return (x >= p.x) && (y >= p.y);}

	inline bool operator<=(const AuPoint2d &p) const	// true if ALL components are <= corresponding components
		{return (x <= p.x) && (y <= p.y);}

	// Precedence is Y then X. Do not change this. The AuMeatMesh sort order depends on this.
	inline bool operator<(const AuPoint2d &p) const
	{
		if(y < p.y)
			return true;
		else if(y > p.y)
			return false;

		if(x < p.x)
			return true;
		else if(x > p.x)
			return false;

		return false;
	}

	inline bool operator>(const AuPoint2d &p) const
	{
		if(y > p.y)
			return true;
		else if(y < p.y)
			return false;

		if(x > p.x)
			return true;
		else if(x < p.x)
			return false;

		return false;
	}

	inline AuPoint2d& operator+=(const AuPoint2d &p)
	{
		x += p.x;
		y += p.y;
		return *this;
	}

	inline AuPoint2d& operator-=(const AuPoint2d &p)
	{
		x -= p.x;
		y -= p.y;
		return *this;
	}

	inline AuPoint2d& operator*=(const double r)
	{
		x *= r;
		y *= r;
		return *this;
	}

	inline AuPoint2d& operator/=(const double r)
	{
		x /= r;
		y /= r;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << x;
		out << y;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> x;
		in >> y;
	}
	
public:	// Methods
	// Calculates the area of a triangle formed by this point and 'v2' and 'v3'. Draw direction is not a factor.
	inline double AreaOfTriangle(const AuPoint2d& v2, const AuPoint2d& v3) const
	{
		AuPoint2d vx = v3 - *this;
		AuPoint2d vy = v2 - *this;
		AuPoint2d vp = vx.PerpendicularVector();
		vp.Unit();
		double h = fabs(vy.Dot(vp));
		if(h == 0)
			return 0;
		return (h * vx.Length()) / 2.;
	}

	inline double Length() const
	{
		return(sqrt(x*x + y*y));
	}

	inline double Length2() const
		{return(x*x + y*y);}

	// Returns the 2D angle of the vector.
	// 'this' vector does not have to be a unit vector.
	inline double ArcTangent_2D() const
	{
		return ArcTangent_2D(x,y);
	}

	// Returns the angle of the vector around the Z axis.
	// 'this' vector does not have to be a unit vector.
	inline double ZRotation() const
	{
		return ArcTangent_2D(x,y);
	}

	inline static double ArcTangent_2D(double x, double y)
	{
		double	ang;
		if(::IsFuzzyZero(y))
		{
			if(GreaterThanOrFuzzyEquals(x, 0.))
				ang = 0.;
			else
				ang = PI;
		}
		else
		{
			if(::IsFuzzyZero(x))
			{
				if(GreaterThanOrFuzzyEquals(y, 0.))
					ang = PI/2.;
				else
					ang = -PI/2.;
			}
			else
				ang = atan2(y, x);
		}
		return(ang);
	}

	inline double Distance(const AuPoint2d &p) const							// 3D distance to another point
		{return(AuPoint2d(*this - p).Length());}

	inline double Distance2(const AuPoint2d &p) const							// square of 3D distance to another point
		{return(AuPoint2d(*this - p).Length2());}

	// returns the distance from this point to a line through lp1 and lp2
	inline double DistToLine(const AuPoint2d &lp1,const AuPoint2d &lp2) const
	{
		AuPoint2d segvec(lp2 - lp1);
		double rlen = segvec.Length();
		if (rlen == 0.)
			return(Distance(lp1));	// seg is too short, return distance to end point

		// return length of perpendicular distance to intersect drop point
		AuPoint2d locvec(*this - lp1);
		double rdot = locvec.Dot(segvec)/rlen;
		AuPoint2d pint(lp1 + segvec*rdot/rlen);
		return Distance(pint);
	}

	// find the 2D distance from a point to a line segment
	// if the locate point is beyond either end point of the segment, the distance to the
	// closest end point is returned
	inline double DistToSegment(const AuPoint2d &lp1,const AuPoint2d &lp2) const
	{
		AuPoint2d pint(PointOnSegment(lp1,lp2));
		return(Distance(pint));
	}

	// find the square of the 2D distance from a point to a line segment
	// if the locate point is beyond either end point of the segment, the distance to the
	// closest end point is returned
	inline double DistToSegment2(const AuPoint2d &lp1, const AuPoint2d &lp2) const
	{
		AuPoint2d pint(PointOnSegment(lp1,lp2));
		return(Distance2(pint));
	}

	inline double Dot(const AuPoint2d &p) const
		{return x*p.x+y*p.y;}

	// returns the side of the line from fLine1 to fLine2 that this point is on
	// 1=left, 0=on, -1=right
	inline int SideOfLine_2D(const AuPoint2d& fLine1, const AuPoint2d& fLine2) const	// 1=left,0=on,-1=right
	{
		double rX1 = fLine1.x - x;
		double rY1 = fLine1.y - y;
		double rX2 = fLine2.x - x;
		double rY2 = fLine2.y - y;
		double rCross = (rX1 * rY2) - (rX2 * rY1);
		if(rCross < 0)
			return -1;
		else if(rCross > 0)
			return 1;
		else
			return 0;
	}

	// Make the 3D vector a specific length.
	// Passing 1.0 (or 0.0) for len results in a unit vector.
	// If this vector is initially 0, the result points down the X axis.
	inline void Unit(double len)
	{
		double vlen = Length();
		if(vlen != 0.)
		{
			if(vlen!=1.)
				len /= vlen;
			if(len != 1.)
				*this *= len;
		}
		else
		{
			x = len;
		}
	}

	inline void Unit()
	{
		double vlen = Length();
		if(vlen != 0.)
			*this /= vlen;
		else
			x = 1;
	}

	inline void Round(double rndval)
	{
		x = FROUND(x, rndval);
		y = FROUND(y, rndval);
	}

	inline AuPoint2d Rounded() const
	{
		return AuPoint2d(FROUND(x, 1.), FROUND(y, 1.));
	}

	inline AuPoint2d Rounded(double rndval) const
	{
		return AuPoint2d(FROUND(x, rndval), FROUND(y, rndval));
	}

	inline bool FuzzyEquals(const AuPoint2d& p) const	// fuzzy comparison
	{ 
		if(!::FuzzyEquals(x, p.x))
			return false;
		if(!::FuzzyEquals(y, p.y))
			return false;
		return true;
	}

	inline bool FuzzyEquals(const AuPoint2d& p, double tol) const	// fuzzy comparison
	{ 
		if(!::FuzzyEqualsFactor(x, p.x, tol))
			return false;
		if(!::FuzzyEqualsFactor(y, p.y, tol))
			return false;
		return true;
	}

	// Perform not-a-number test on each x,y,z value.
	// True if any value x,y,z is not-a-number
	inline bool IsNaN() const
	{
		if(std::isnan(x) || std::isnan(y))
			return true;
		return false;
	}
	
	inline bool IsFuzzyZero() const								// fuzzy test for 0,0,0 vector
	{
		if(!::IsFuzzyZero(x))
			return false;
		if(!::IsFuzzyZero(y))
			return false;
		return true;
	}

	// Test for X, Y and Z exactly zero, not a fuzzy test.
	inline bool IsZero() const
	{
		return x == 0. && y == 0.;
	}

	// fuzzy test for 2D parallel vectors (need not be unit vectors)
	inline bool IsParallel_2D(const AuPoint2d& vec) const
	{
		if(vec.x == 0 && x == 0)
			return true;
		if(vec.x == 0 || x == 0)
			return false;
		double myslope = y / x;
		double otherslope = vec.y / vec.x;
		return ::FuzzyEquals(myslope, otherslope);
	}

	// return 2D perp loc from this to the line between fLoc1 and fLoc2
	// result has the same Z height as this point
	inline AuPoint2d Perpendicular(const AuPoint2d &fLoc1, const AuPoint2d &fLoc2) const
	{
		// unit vector in direction of line segment
		AuPoint2d funit;
		funit.x = fLoc2.x - fLoc1.x;
		funit.y = fLoc2.y - fLoc1.y;
		funit.Unit();

		// vector from segment start to this point
		AuPoint2d fvec;
		fvec.x = x - fLoc1.x;
		fvec.y = y - fLoc1.y;

		// calculate intersection point using dot product
		double rdot = fvec.Dot(funit);
		AuPoint2d fperp;
		fperp.x = fLoc1.x + funit.x * rdot;
		fperp.y = fLoc1.y + funit.y * rdot;

		return(fperp);
	}
	
	inline AuPoint2d PerpendicularVector() const	// return perpendicular vector
		{return(AuPoint2d(-y,x));}

	inline AuPoint2d Reflect() const		// return reflected vector
		{return(AuPoint2d(-y,-x));}

	// true if this is between p1 and p2.
	// Does not test for distance from the points, just that the drop point onto that vector falls between or fuzzy at its ends.
	inline bool Between(const AuPoint2d &p1,const AuPoint2d &p2, double tol=FUZZ) const
	{
		if(FuzzyEquals(p1, tol) || FuzzyEquals(p2, tol))
			return true;

		AuPoint2d pvec1(p2 - p1);
		double rlen = pvec1.Length2();
		if(rlen == 0)
			return false;				// zero length vector

		rlen = sqrt(rlen);
		pvec1 /= rlen;					// make unit vector

		AuPoint2d pvec2(*this - p1);
		double rdot = pvec2.Dot(pvec1);
		if(rdot < 0 || rdot > rlen)
			return false;				// not between the points

		return true;
	}

	inline AuPoint2d Min(const AuPoint2d &fcomp) const
	{
		return AuPoint2d(MIN(fcomp.x,x), MIN(fcomp.y,y));
	}

	inline AuPoint2d Max(const AuPoint2d &fcomp) const
	{
		return AuPoint2d(MAX(fcomp.x,x), MAX(fcomp.y,y));
	}

	// Returns the absolute value of the maximum of X or Y.
	inline double MaxDimension() const
	{
		double fx = fabs(x);
		double fy = fabs(y);
		if(fx > fy)
			return fx;
		return fy;
	}

	// find closest point on a line segment
	// if the locate point is beyond either end point of the segment, the 
	// closest end point is returned
	inline AuPoint2d PointOnSegment(const AuPoint2d &lp1,const AuPoint2d &lp2) const
	{
		AuPoint2d segvec(lp2 - lp1);
		double rlen = segvec.Length();
		if(rlen == 0.)
			return(lp1);	// seg is too short, return end point

		AuPoint2d locvec(*this - lp1);
		double rdot = locvec.Dot(segvec)/rlen;
		if(rdot <= 0.)
			return lp1;	// beyond first segment end
		else if(rdot >= rlen)
			return lp2;	// beyond other end of segment
		else
		{
			// between segment points, get length of perpendicular distance
			AuPoint2d pint(lp1 + segvec*rdot/rlen);
			return pint;
		}
	}

	inline bool SegmentIntersection(const AuPoint2d &fA1, const AuPoint2d &fA2, const AuPoint2d &fB1, const AuPoint2d &fB2,
			bool bBInfinite=false, bool bAInfinite=false, double tol=FUZZ)
	{
		AuPoint2d fInt2;
		if(!fA1.PerpConn(fA2,fB1,fB2,*this,fInt2, tol))
			return false;

		if(!FuzzyEquals(fInt2, tol))
			return false;

		if(!bAInfinite && !Between(fA1,fA2, tol))
			return false;

		if(!bBInfinite && !Between(fB1,fB2, tol))
			return false;

		return true;
	}

	// returns the side of the line from fLine1 to fLine2 that this point is on
	// 1=left, 0=on, -1=right
	inline int SideOfLine(const AuPoint2d &fLine1,const AuPoint2d &fLine2) const	// 1=left,0=on,-1=right
	{
		double rX1 = fLine1.x - x;
		double rY1 = fLine1.y - y;
		double rX2 = fLine2.x - x;
		double rY2 = fLine2.y - y;
		double rCross = (rX1 * rY2) - (rX2 * rY1);
		if (rCross < 0)
			return -1;
		else if (rCross > 0)
			return 1;
		else
			return 0;
	}

	inline bool IsOnSegment(const AuPoint2d &ptSegEnd1, const AuPoint2d &ptSegEnd2) const
	{
		if(DistToLine(ptSegEnd1, ptSegEnd2) > FUZZ)
			return false;
		return Between(ptSegEnd1, ptSegEnd2, FUZZ);
	}

	inline bool IsOnSegment(const AuPoint2d &ptSegEnd1, const AuPoint2d &ptSegEnd2, double tol) const
	{
		if(DistToLine(ptSegEnd1, ptSegEnd2) > tol)
			return false;
		return Between(ptSegEnd1, ptSegEnd2, tol);
	}

	inline void Scale(const AuPoint2d& fScale)		// scale this vector
	{
		x *= fScale.x;
		y *= fScale.y;
	}

	inline AuPoint2d RotateZ(double angle) const	// Rotate around Z by angle in radians.
	{
		double c = cos(angle);
		double s = sin(angle);
		AuPoint2d rotvec;
		rotvec.x = c*x + -s*y;
		rotvec.y = s*x + c*y;
		return(rotvec);
	}

	// Return true if this point is fuzzy inside of a triangle in 2D.
	// The triangle can be clockwise or counterclockwise.
	// This function fails if the plane of the triangle is perpendicular to the XY plane.
	inline bool InTriangleFuzzy(const AuPoint2d &f1, const AuPoint2d &f2, const AuPoint2d &f3, double tol=FUZZ) const
	{
		int lSide0 = SideOfLine(f1,f2);
		int lSide1 = SideOfLine(f2,f3);
		int lSide2 = SideOfLine(f3,f1);
		if(LessThanOrFuzzyEqualsFactor(lSide0, 0, tol) &&
				LessThanOrFuzzyEqualsFactor(lSide1, 0, tol) &&
				LessThanOrFuzzyEqualsFactor(lSide2, 0, tol))
			return true;	// this point is within a clockwise triangle formed by three consecutive boundary vertices
		else if(GreaterThanOrFuzzyEqualsFactor(lSide0, 0, tol) &&
				GreaterThanOrFuzzyEqualsFactor(lSide1, 0, tol) &&
				GreaterThanOrFuzzyEqualsFactor(lSide2, 0, tol))
			return true;	// this point is within a counterclockwise triangle formed by three consecutive boundary vertices
		return false;
	}

	// Return true if this point is inside of a triangle in 2D.
	// The triangle can be clockwise or counterclockwise.
	// This function fails if the plane of the triangle is perpendicular to the XY plane.
	inline bool InTriangle(const AuPoint2d &f1, const AuPoint2d &f2, const AuPoint2d &f3, bool bClockwiseOnly=false) const
	{
		int lSide0 = SideOfLine(f1,f2);
		int lSide1 = SideOfLine(f2,f3);
		int lSide2 = SideOfLine(f3,f1);
		if(lSide0 <= 0 && lSide1 <= 0 && lSide2 <= 0)
			return true;	// this point is within a clockwise triangle formed by three consecutive boundary vertices
		else if(!bClockwiseOnly && lSide0 >= 0 && lSide1 >= 0 && lSide2 >= 0)
			return true;	// this point is within a counterclockwise triangle formed by three consecutive boundary vertices
		return false;
	}

	inline bool PerpConn(const AuPoint2d &p1,const AuPoint2d &p2,const AuPoint2d &p3,AuPoint2d &pa,AuPoint2d &pb,
			double tol=FUZZ) const
	{
		AuPoint2d d0(p1 - *this);
		AuPoint2d d1(p3 - p2);
		AuPoint2d d2(p2 - *this);

		double d00 = d0.Dot(d0);
		double d01 = d0.Dot(d1);
		double d11 = d1.Dot(d1);
		double fa=d01*d01;
		double fb=d00*d11;
		double det = fa-fb;
	//	double det = d01*d01-d00*d11;
	//	if (det == 0.0)
	//	if (::IsFuzzyZero(det))
		if (fabs(det) <= tol*(fabs(fa)+fabs(fb)))
		{
			pa.x = 0;
			pa.y = 0;
			pb.x = 0;
			pb.y = 0;
			return false;
		}
		double r0 = d2.Dot(d0);
		double r1 = d2.Dot(d1);
		pa = *this + d0 * (( r1*d01 - r0*d11 ) / det);
		pb = p2 + d1 * (( d00*r1 - d01*r0 ) / det);
		return true;
	}
};

Q_DECLARE_METATYPE(AuPoint2d)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

AUMATH_DLL QDataStream& operator<<(QDataStream& out, const AuPoint2d& p);

AUMATH_DLL QDataStream& operator>>(QDataStream& in, AuPoint2d& p);

AUMATH_DLL QDebug operator<<(QDebug dbg, const AuPoint2d& p);

// qHash is needed to use AuPoint2d in a QList in Windows.
AUMATH_DLL unsigned int qHash(const AuPoint2d& key);

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// A handy array object for AuPoint2ds
class AUMATH_DLL AuPoint2dArray : public QVector<AuPoint2d>
{
public:	// Operators
	inline void operator>>(QDataStream& out) const
	{
		Q_ASSERT(count() <= 0xffff);	// We don't intend to serialize more than 65k points in an array.
		out << (quint16)count();
		for(const_iterator it = constBegin(); it != constEnd(); it++)
			out << *it;
	}

	inline void operator<<(QDataStream& in)
	{
		clear();
		quint16 cnt;
		in >> cnt;
		AuPoint2d p;
		for(; cnt; cnt--)
		{
			in >> p;
			append(p);
		}
	}

	inline bool operator==(const AuPoint2dArray &c) const
	{
		int lMyCount = count();
		int lOtherCount = c.count();
		if (lMyCount != lOtherCount)
			return false;

		const AuPoint2d *pMyLoc = begin();
		const AuPoint2d *pOtherLoc = c.begin();
		for (;lMyCount; lMyCount--, pMyLoc++, pOtherLoc++)
		{
			if (*pMyLoc != *pOtherLoc)
				return false;
		}

		return true;
	}

	inline bool operator!=(const AuPoint2dArray &c) const
	{
		return !operator==(c);
	}

public:	// Methods
	inline double Area() const
	{
		if(count() < 3)
			return 0.0;			// No area if less than three vertices.

		double area = 0.0;		// This will be the accumulated area value that we return.

		const AuPoint2d* start = begin();
		const AuPoint2d* prevLoc = start + 1;
		const AuPoint2d* curLoc = prevLoc + 1;
		for(int lvert=2; lvert<count(); lvert++, prevLoc++, curLoc++)
		{
			// Calculate the area of the triangle formed by this vertex, the previous vertex, and vertex 0.
			// We will add the area of clockwise triangles and subtract the area of counterclockwise triangles.
			switch(curLoc->SideOfLine_2D(*start, *prevLoc))
			{
			case 0:		// on the line -> no area
				break;
			case 1:		// left of line
				// Subtract area of triangle formed by the two vectors.
				area -= start->AreaOfTriangle(*prevLoc, *curLoc);
				break;
			case -1:	// right of line
				// Add area of triangle formed by the two vectors.
				area += start->AreaOfTriangle(*prevLoc, *curLoc);
				break;
			}
		}

		// The area value is currently negative if the boundary was counterclockwise.
		return fabs(area);
	}

	inline void Extents(AuPoint2d &fMin, AuPoint2d &fMax) const
	{
		// if no points then just return 0's
		if (isEmpty())
		{
			fMin = AuPoint2d(0., 0.);
			fMax = AuPoint2d(0., 0.);
			return;
		}

		//
		// set initial min and max to first element and then loop for
		// rest of elements
		//
		fMin = at(0);
		fMax = at(0);

		for(int i = 1; i < count(); i++)
		{
			fMin = fMin.Min(at(i));
			fMax = fMax.Max(at(i));
		}
	}

	// Offsets all points in this array by 'offsetLeft'.
	// Positive offsetLeft offsets to the left of the boundary lines, making a clockwise boundary bigger.
	// Negative offsetLeft offsets to the right of the boundary lines, making a clockwise boundary smaller.
	void OffsetBoundary(double offsetLeft);

	// Returns true if fLoc is in the 2D boundary formed by this array of points.
	// The point is in the boundary if an odd number of boundary segments must be crossed to leave the boundary.
	bool PointInBoundary(const AuPoint2d& loc) const;

	// Returns true if fLoc is on the boundary formed by this array of points.
	inline bool PointOnBoundary(const AuPoint2d &fLoc) const
	{
		if (count() < 2)
			return false;

		// Check segments of the array.
		for(const_iterator it = constBegin()+1; it != constEnd(); it++)
			if(fLoc.IsOnSegment(*(it-1), *it))
				return true;

		// Check closing segment if there is more than two vertices.
		if(count() > 2)
			if(fLoc.IsOnSegment(last(), first()))
				return true;

		return false;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUMATH_DLL AuPointN : public AuPoint
{
public:
	AuPoint								m_N;					// Normal at this location.

public:	// Constructors
	inline AuPointN(const AuPoint& loc, const AuPoint& n) : AuPoint(loc)
	{
		m_N = n;
	}

	inline AuPointN()
	{
	}

	inline AuPointN(const AuPointN& loc) : AuPoint(loc)
	{
		m_N = loc.m_N;
	}

public:	// Operators
	inline const AuPointN& operator=(const AuPointN& loc)
	{
		AuPoint::operator=(loc);
		m_N = loc.m_N;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		AuPoint::operator>>(out);
		out << m_N;
	}

	inline void operator<<(QDataStream& in)
	{
		AuPoint::operator<<(in);
		in >> m_N;
	}

	inline bool operator==(const AuPointN& p) const
	{
		if(!AuPoint::operator==(p))
			return false;
		if(m_N != p.m_N)
			return false;
		return true;
	}

	inline bool operator!=(const AuPointN& p) const
	{
		return !operator==(p);
	}

public:	// Methods
	inline AuPointN LinearInterpolate(const AuPointN& o, double frac) const
	{
		AuPoint n = m_N * (1-frac) + o.m_N * frac;
		n.Unit();
		return AuPointN(AuPoint::LinearInterpolate(o, frac), n);
	}

	void Transform(const AuMatrix4x4& t);
};

AUMATH_DLL inline QDataStream& operator<<(QDataStream& out, const AuPointN& o)
{
	o >> out;
	return out;
}

AUMATH_DLL inline QDataStream& operator>>(QDataStream& in, AuPointN& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUMATH_DLL AuPointNArray : public QVector<AuPointN>
{
public:
	void Append(const AuGlobeLL& ll, double altitude, const AuMatrix4x4& m);
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|


#endif // AuPoint_H
