//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumath.h,v 1.10 2022/02/19 18:38:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuMath_H
#define AuMath_H

#include <QtGui>

/* Defines needed for dll linkage.
#if defined AUMATH
#define AUMATH_DLL Q_DECL_EXPORT
#else
#define AUMATH_DLL Q_DECL_IMPORT
#endif
*/
#define AUMATH_DLL	// Defined as nothing for static lib.

class AUMATH_DLL AuMath
{
public:
};

#include <math.h>

// Setups for SSE math functions.
//#define USE_SSE_MATH
#if defined(USE_SSE_MATH)
#include <malloc.h>
#include <xmmintrin.h>
#endif

// double versions
#ifdef PI
#	undef PI
#endif
#define PI 3.141592653589793238462
#define DEG2RAD		(PI/180.0)
#define RAD2DEG		(180.0/PI)
#define GRAD2RAD	(PI/50.0)
#define RAD2GRAD	(50.0/PI)
#define	ACOS(x)		(double)((x) > 1.0 ? 0.0 : (x) < -1.0 ? PI : acos((double)(x)))
#define FROUND(val,rndval) ((int)((val + (val > 0.0 ? 0.5*rndval : -0.5*rndval))/rndval))*rndval

// float versions
#ifdef PIf
#	undef PIf
#endif
#define PIf 3.141592653589793238462f
#define DEG2RADf	(PIf/180.f)
#define RAD2DEGf	(180.f/PIf)
#define GRAD2RADf	(PIf/50.f)
#define RAD2GRADf	(50.f/PIf)
#define	ACOSf(x)	(float)((x) > 1.f ? 0.f : (x) < -1.f ? PIf : acosf((float)(x)))
#define FROUNDf(val,rndval) ((int)((val + (val > 0.f ? 0.5f*rndval : -0.5f*rndval))/rndval))*rndval

#define RANDOM(imin,imax)	(imin+rand()%(imax-imin+1))	// calculate a random number between imin and imax, inclusively
#define RANDOM_INTERVAL		(RANDOM(50,150))	// return random range for status update

#define MIN(x,y)	((x)<(y)?(x):(y))
#define MAX(x,y)	((x)>(y)?(x):(y))


// Fuzzy Zero Definitions

#define FUZZ	((double)0.00001)		// Largest magnitude considered to be 0.

// true if magnitude of rTest <= FUZZ
inline bool IsFuzzyZero(double rTest)
{
	return fabs(rTest) <= FUZZ;
}

inline bool IsFuzzyZero(float rTest)
{
	return fabsf(rTest) <= FUZZ;
}

// true if the difference between r1 and r2 is less than or equal to fuzzy zero.
inline bool FuzzyEquals(double r1, double r2)
{
	return IsFuzzyZero(r1 - r2);
}

inline bool FuzzyEquals(float r1, float r2)
{
	return IsFuzzyZero(r1 - r2);
}

inline bool FuzzyEqualsFactor(double r1, double r2, double rFactor)
{
	return fabs(r1-r2) <= rFactor;
}

// true if rHigh is greater than rLow by at least fuzzy zero.
inline bool FuzzyGreaterThan(double rHigh, double rLow)
{
	return rHigh - rLow > FUZZ;
}

// true if rHigh is greater than rLow by at least fuzzy zero.
inline bool FuzzyGreaterThanFactor(double rHigh, double rLow, double tol)
{
	return rHigh - rLow > tol;
}

// true if rLow is less than rHigh by at least fuzzy zero.
inline bool FuzzyLessThan(double rLow, double rHigh)
{
	return rLow - rHigh < -FUZZ;
}

// true if rLow is less than rHigh by at least fuzzy zero.
inline bool FuzzyLessThanFactor(double rLow, double rHigh, double tol)
{
	return rLow - rHigh < -tol;
}

// true if rLow is less than or fuzzy equal to rHigh.
inline bool LessThanOrFuzzyEquals(double rLow, double rHigh)
{
	return rLow - rHigh <= FUZZ;
}

// true if rLow is less than or fuzzy equal to rHigh.
inline bool LessThanOrFuzzyEqualsFactor(double rLow, double rHigh, double tol)
{
	return rLow - rHigh <= tol;
}

// true if rHigh is greater than or fuzzy equal to rLow.
inline bool GreaterThanOrFuzzyEquals(double rHigh, double rLow)
{
	return rHigh - rLow >= -FUZZ;
}

// true if rHigh is greater than or fuzzy equal to rLow.
inline bool GreaterThanOrFuzzyEqualsFactor(double rHigh, double rLow, double tol)
{
	return rHigh - rLow >= -tol;
}

inline bool FuzzyBetween(double rBetween, double rLow, double rHigh)
{
	//QAssert(rLow <= rHigh); // Caller should ensure rLow is less than or equal to rHigh (or call FuzzyBetweenUnsorted).
	return GreaterThanOrFuzzyEquals(rBetween, rLow) &&
		LessThanOrFuzzyEquals(rBetween, rHigh);
}

inline bool FuzzyBetweenUnsorted(double rBetween, double r1, double r2)
{
	if (r1 < r2)
		return FuzzyBetween(rBetween, r1, r2);
	else
		return FuzzyBetween(rBetween, r2, r1);
}

// Returns 0 if rValue is fuzzy zero, otherwise returns rValue.
inline double FuzzyClamp(double rValue)
{
	if(IsFuzzyZero(rValue))
		return 0;
	else
		return rValue;
}

#endif // AuMath_H
