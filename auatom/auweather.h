//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auweather.h,v 1.40 2022/02/19 17:33:08 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AUWEATHER_H
#define AUWEATHER_H

#include "audef.h"
#include <math.h>
#include <QPoint>

class AuDice;
class AuGlobeLLF;
class DaWorld;
class DaWorldRingSection;
class QPoint;
class QPointF;

class AuWeather
{
public:
	enum LAYER
	{
		LowClouds,
		MiddleClouds,
		//HighClouds,
		UpperLimit,
	};

public:
	AuWeather();
	AuWeather(const AuWeather& c);
	~AuWeather();

public:
	AuWeather operator=(const AuWeather& c);

public:
	// Returns the weather conditions present.
	// 'relHumidity' is an output parameter set to the ratio of vapor pressure to saturation pressure. >=1 indicates precip
	// 'oat' is an output parameter indicating outside air temperature in celsius.
	// 'bar' is an output parameter indicating outside air pressure in bar.
	void ConditionsAt(const DaWorld& planet, const AuGlobeLLF& longLat, float altitudeMSL, float ewspoffset,
			float ewvpoffset, float& relHumidity, float& oat, float& bar) const;

	inline unsigned int Height() const
	{
		return m_LatDivs;
	}

	void InitRingSection(const DaWorldRingSection& planet, const AuDice& dice, float solarOrbitRadius, float sunLightAtten);

	void InitSphere(const DaWorld& planet, quint16 radius100km, const AuDice& dice, float solarOrbitRadius, float sunLightAtten);

	// Returns the altitude of a cloud layer above sea level.
	static float LayerAltitude(const DaWorld& planet, LAYER layer);

	// Returns atmospheric pressure, in bars.
	float PressureAt(const DaWorld& planet, const AuGlobeLLF& longLat, float altitudeMSL, float ewspoffset) const;

	// Returns prevailing wind speed at the equator in miles per hour.
	static inline float PrevailingWindSpeed(ORBITZONE zone)
	{
		switch(zone)
		{
		case OZONE_Inferno:
			return 1.f;
		case OZONE_Inner:
			return 4.f;
		case OZONE_Habitable:
			return 16.f;
		case OZONE_Outer:
			return 64.f;
		case OZONE_Frigid:
			return 256.f;
		case OZONE_DeepSpace:
			break;
		}
		return 0.f;
	}

	void ShaderData(const DaWorld& planet, float* wx, float& maxSP, float& maxVP, float& minSP, float& minVP, LAYER layer,
			const AuDice& dice) const;

	float TemperatureAt(const DaWorld& planet, const AuGlobeLLF& longLat, float altitudeMSL, float ewspoffset) const;

	inline unsigned int Width() const
	{
		return m_LonDivs;
	}

	// Length of vector is wind strength in miles per hour.
	QPointF WindAt(const DaWorld* planet, const AuGlobeLLF& longLat, float ewspoffset) const;

protected:
	// Wraps pLongLat to be within the acceptable range.
	// Returns false if latitude rolls over to wrap to the bottom/top of the planet.
	// The random terrain generator uses this.
	// Returns true if there is no latitude wrap.
	inline bool ClampLongLat(QPoint &pLongLat) const
	{
		// Longitude wraps horizontally.
		if(pLongLat.x() < 0)
			pLongLat.setX(pLongLat.x() + Width());
		else if(pLongLat.x() > (int)Width()-1)
			pLongLat.setX(pLongLat.x() - Width());

		// Latitude wraps vertically for use by the terrain generator. This isn't
		// generally useful for navigation.
		const unsigned int iLatDivs = Height();
		if(pLongLat.y() < 0)
		{
			pLongLat.setY(pLongLat.y() + iLatDivs);
			return false;
		}
		else if(pLongLat.y() > (int)iLatDivs-1)
		{
			pLongLat.setY(pLongLat.y() - iLatDivs);
			return false;
		}

		return true;
	}

	// Returns the temperature of the cloud layer above the altitude, in Kelvin.
	float CloudTemperatureAt(const DaWorld& planet, const AuGlobeLLF& longLat, float altitudeMSL, float ewspoffset) const;

	// returns dewpoint temp in kelvin
	// T is temperature in kelvin
	// RH is relative humidity in the range (0..1]
	static inline float DewPoint(float T, float RH)
	{
		if(RH <= 0.f)
			return T;

		// Formula from http://www.paroscientific.com/dewpoint.htm
		//
		// T is measured temperature C (0 < T < 60)
		// RH is measured relative humidity
		// Td is calculated dew point temperature C
		// a = 17.27
		// b = 237.7 C
		//
		// Td = b * f(T, RH) / (a - f(T, RH))
		//
		// f(T, RH) = a * T / (b + T) + ln(RH)
		const float a = 17.27f;
		const float b = 237.7f;
		T -= 273.f;			// convert kelvin to celsius
		if(b + T == 0.f)
			return T;
		float fTRH = a * T / (b + T) + logf(RH);
		if(a - fTRH == 0.f)
			return 0.f;
		float Td = b * fTRH / (a - fTRH);
		Td += 273.f;		// convert celsius to kelvin
		return Td;
	}

	void InitMoisture(const DaWorld& planet, const AuDice& dice);

	void InitPressure(const DaWorld& planet, const AuDice& dice);

	// Longitude/latitude division calculations.
	static inline int LatitudeDivisions(quint16 radius100km)
	{
		// Regular spherical planet.
		return LongitudeDivisions(radius100km)/2;
	}

	static inline int LongitudeDivisions(quint16 radius100km) 
	{
		int twid = 2 * radius100km;
		if(twid > 240)
			twid = 240;	// Don't get carried away. The size consistency only matters for worlds we can build upon.
		twid *= 5;
		return twid;
	}

	// Returns temperature at a given long/lat and altitude in kelvin.
	// Temperature is adjusted for relative atmospheric pressure.
	float TemperatureAt(const DaWorld& planet, const AuGlobeLLF& longLat, LAYER layer) const;

	// Returns temperature lapse.
	static inline float TemperatureLapse(LAYER layer)
	{
		float tempLapse;
		switch(layer)
		{
		//case HighClouds:
			//tempLapse = 60;		// 60 degrees cooler = 30k feet MSL
			//break;
		case MiddleClouds:
			tempLapse = 36;		// 36 degrees cooler = 18k feet MSL
			break;
		case LowClouds:
			tempLapse = 12;		// 12 degrees cooler = 6k feet MSL
			break;
		case UpperLimit:
		default:
			tempLapse = 0;		// Absolute zero at limit of atmosphere.
			break;
		}
		return tempLapse;
	}

	// Returns the vapor pressure of water in bars at temperature T in kelvin.
	// Returns HUGE_VALF at very high T.
	static inline float VaporPressureOverIce(float T)
	{
		// Formula for vapor pressure over ice from http://en.wikipedia.org/wiki/Water_data_page
		// For temperature range 173.15K to 273.15K.
		// The expf() call returns pressure in Pascals. When T is high, expf overflows and returns HUGE_VALF (inf).
		float vp = expf(-5504.4088f * powf(T,-1.f) - 3.5704628f - .017337458f * T + .0000065204209f * T * T + 6.1295027f * logf(T));
#if !defined(Q_OS_WIN)
		if(vp == HUGE_VALF)
			return HUGE_VALF;									// Don't convert HUGE_VALF to bars.
#endif

		// Multiply by .00001 to convert to bars. 1 Pa = .01 mbar
		return vp * .00001f;
	}

	// Returns the vapor pressure of water in bars at temperature T in kelvin.
	// Returns HUGE_VALF at very high T.
	static inline float VaporPressureOverWater(float T)
	{
		// Formula for vapor pressure over liquid water from http://en.wikipedia.org/wiki/Water_data_page
		// For temperature range 173.15K to 373.15K.
		// The expf() call returns pressure in Pascals. When T is high, expf overflows and returns HUGE_VALF (inf).
		float vp = expf(-6094.4642f * powf(T,-1.f) + 21.1249952f - .02724552f * T + .000016853396f * T * T + 2.4575506f * logf(T));
#if !defined(Q_OS_WIN)
		if(vp == HUGE_VALF)
			return HUGE_VALF;									// Don't convert HUGE_VALF to bars.
#endif

		// Multiply by .00001 to convert to bars. 1 Pa = .01 mbar
		return vp * .00001f;
	}

	float VaporPressureAt(const DaWorld& planet, const AuGlobeLLF& longLat, float altitudeMSL, float ewvpoffset) const;

	static inline float VaporPressureLapse(LAYER layer)
	{
		float vpLapse;
		switch(layer)
		{
		//case HighClouds:
			//vpLapse = .044f;	// Tiny fraction of water vapor pressure at this altitude.
			//break;
		case MiddleClouds:
			vpLapse = .22f;		// Less water vapor pressure at this altitude.
			break;
		case LowClouds:
			vpLapse = 1.f;		// Full water vapor pressure at this level.
			break;
		case UpperLimit:
		default:
			vpLapse = 0.f;		// No water vapor at limit of atmosphere.
			break;
		}
		return vpLapse;
	}

private:
	unsigned int						m_LatDivs;				// Latitude divisions of weather model.
	unsigned int						m_LonDivs;				// Longitude divisions of weather model.
	float*								m_Moisture;				// 2D array of vapor pressures at each lat/long in bars
	float*								m_Pressures;			// 2D array of air pressure at each lat/long in bars
	float								m_SolarOrbitRadius;		// Radius of this world's orbit from the sun (or its parent).
	float								m_SunLightAtten;		// DaStar::LightAttenuation.
};

#endif // AUWEATHER_H
