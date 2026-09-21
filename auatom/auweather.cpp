//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auweather.cpp,v 1.67 2022/02/19 17:41:36 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auweather.h"
//
#include "audice.h"
#include "aunoise.h"
#include "daworldringsection.h"
#include <QPointF>

AuWeather::AuWeather()
{
	m_LatDivs = 0;
	m_LonDivs = 0;
	m_Moisture = 0;
	m_Pressures = 0;
}

AuWeather::~AuWeather()
{
	if(m_Moisture)
	{
		delete [] m_Moisture;
		m_Moisture = 0;
	}
	if(m_Pressures)
	{
		delete [] m_Pressures;
		m_Pressures = 0;
	}
}

AuWeather::AuWeather(const AuWeather& c)
{
	m_LatDivs = c.m_LatDivs;
	m_LonDivs = c.m_LonDivs;
	if(c.m_Moisture)
	{
		unsigned int latDivs = c.m_LatDivs;
		unsigned int arraysz = latDivs * c.m_LonDivs;
		m_Moisture = new float[arraysz];
		memcpy(m_Moisture, c.m_Moisture, arraysz*sizeof(float));
	}
	else
		m_Moisture = 0;
	if(c.m_Pressures)
	{
		unsigned int latDivs = c.m_LatDivs;
		unsigned int arraysz = latDivs * c.m_LonDivs;
		m_Pressures = new float[arraysz];
		memcpy(m_Pressures, c.m_Pressures, arraysz*sizeof(float));
	}
	else
		m_Pressures = 0;
}

AuWeather AuWeather::operator=(const AuWeather& c)
{
	if(c.m_Moisture)
	{
		if(m_Moisture && m_LonDivs != c.m_LonDivs)
		{
			delete [] m_Moisture;
			m_Moisture = 0;
		}
		unsigned int latDivs = c.m_LatDivs;
		unsigned int arraysz = latDivs * c.m_LonDivs;
		if(!m_Moisture)
			m_Moisture = new float[arraysz];
		memcpy(m_Moisture, c.m_Moisture, arraysz*sizeof(float));
	}
	else if(m_Moisture)
	{
		delete [] m_Moisture;
		m_Moisture = 0;
	}
	if(c.m_Pressures)
	{
		if(m_Pressures && m_LonDivs != c.m_LonDivs)
		{
			delete [] m_Pressures;
			m_Pressures = 0;
		}
		unsigned int latDivs = c.m_LatDivs;
		unsigned int arraysz = latDivs * c.m_LonDivs;
		if(!m_Pressures)
			m_Pressures = new float[arraysz];
		memcpy(m_Pressures, c.m_Pressures, arraysz*sizeof(float));
	}
	else if(m_Pressures)
	{
		delete [] m_Pressures;
		m_Pressures = 0;
	}
	m_LatDivs = c.m_LatDivs;
	m_LonDivs = c.m_LonDivs;
	
	return *this;
}

void AuWeather::InitRingSection(const DaWorldRingSection& planet, const AuDice& Cdice, float solarOrbitRadius, float sunLightAtten)
{
	AuDice dice(Cdice);
	m_LatDivs = planet.NorthSouthDivisions() * 4;	// Calculated for similar appearance to spheres.
	m_LonDivs = planet.SeaLevelEquatorDivisions();	// Weather is elongated along the ring. This looks ok and is faster to generate.
	m_SolarOrbitRadius = solarOrbitRadius;
	m_SunLightAtten = sunLightAtten;

	InitMoisture(planet, dice);
	dice.Random();
	InitPressure(planet, dice);
}

void AuWeather::InitSphere(const DaWorld& planet, quint16 radius100km, const AuDice& Cdice, float solarOrbitRadius,
		float sunLightAtten)
{
	AuDice dice(Cdice);
	// Earth has six weather bands or cells: Hadley Cell, Ferrel Cell, and Polar Cell.
	// Hadley Cell is from 0 to 30 degrees N and S.
	// Ferrel Cell is from 30 to 60 degrees N and S.
	// Polar Cell is from 60 to 90 degrees N and S.
	// The number of bands must be an even number. The equator is a L and each band
	// toward the poles is separated by alternating H's then L's, ending with a H at
	// each pole.
	// The smallest world with an atmosphere has 16 latitude divisions; Earth has 64; the max is 100.
	m_LatDivs = LatitudeDivisions(radius100km);
	m_LonDivs = LongitudeDivisions(radius100km);
	m_SolarOrbitRadius = solarOrbitRadius;
	m_SunLightAtten = sunLightAtten;

	InitMoisture(planet, dice);
	dice.Random();
	InitPressure(planet, dice);
}

// Init vapor pressure data
void AuWeather::InitMoisture(const DaWorld& planet, const AuDice& Cdice)
{
	AuDice dice(Cdice);
	float atmDensity = planet.MSLAtmosphereDensity();
	float surfaceWater = (float)planet.Hydrographics()/100.f;
	if(planet.OrbitZone() == OZONE_Inferno)
		surfaceWater = 0;

	// Allocate an array of floats to store atmospheric moisture in a rectangular
	// array, one value for each lat/long cell division.
	unsigned int arraysz = Height() * Width();
	m_Moisture = new float[arraysz];
	memset(m_Moisture, 0, arraysz*sizeof(float));
	if(atmDensity == 0 || surfaceWater == 0)
		return;

	int octaves = 7;
	float xoff = (float)dice.Roll(1,100000,-1) + (float)dice.Roll(1,100)/100.f;	// Offset into 2D perlin noise
	float yoff = (float)dice.Roll(1,100000,-1) + (float)dice.Roll(1,100)/100.f;	// Offset into 2D perlin noise
	float peakLevel = 0.f;
	float persistence = (float)dice.Roll(1, 30, 70)/100.f;
	float freqx = 4.f / (float)Width();
	float freqy = 4.f / (float)Height();
	int xblendwid = Width() * 20 / 100;		// Last 20% of array is blended to avoid a shear line.
	unsigned int xblendstart = Width() - xblendwid;
	for(unsigned int ifx=0; ifx<xblendstart; ifx++)
		for(unsigned int ify=0; ify<Height(); ify++)
		{
			float fx = (float)(ifx * freqx) + xoff;
			float fy = (float)(ify * freqy) + yoff;
			float vp = fabs(AuNoiseF::PerlinNoise(octaves, persistence, fx, fy) * 100);
/*
if(ify < Height()/2 && ifx < Width()/2)
	vp = 0;
else
	vp = 100;
	*/
			if(vp > peakLevel)
				peakLevel = vp;

			float* vapor = m_Moisture + ifx + ify * Width();
			*vapor = vp;
		}
	for(unsigned int ifx=xblendstart; ifx<Width(); ifx++)
	{
		float blendFactor = (float)(ifx - xblendstart)/(float)xblendwid;
		for(unsigned int ify=0; ify<Height(); ify++)
		{
			float fx = (float)(ifx * freqx) + xoff;
			float fy = (float)(ify * freqy) + yoff;
			float vp1 = fabs(AuNoiseF::PerlinNoise(octaves, persistence, fx, fy) * 100);

			// Calculate blend
			int ibx = (int)(ifx - xblendstart) - xblendwid;
			float bx = (float)(ibx * freqx) + xoff;
			float vp2 = fabs(AuNoiseF::PerlinNoise(octaves, persistence, bx, fy) * 100);

			float vp = AuNoiseF::Cosine_Interpolate(vp1, vp2, blendFactor);
/*
if(ify < Height()/2 && ifx < Width()/2)
	vp = 0;
else
	vp = 100;
	*/
			if(vp > peakLevel)
				peakLevel = vp;

			float* vapor = m_Moisture + ifx + ify * Width();
			*vapor = vp;
		}
	}

	/*
	QImage bmCont(Width(), Height(), QImage::Format_ARGB32);
	for(unsigned int ifx=0; ifx<Width(); ifx++)
		for(unsigned int ify=0; ify<Height(); ify++)
		{
			float* vapor = m_Moisture + ifx + ify * Width();
			if(peakLevel > 0)
			{
				int colVal = *vapor * 255/peakLevel;
				QRgb col = qRgb(colVal, colVal, colVal);
				bmCont.setPixel(ifx, ify, col);
			}
			else
				bmCont.setPixel(ifx, ify, 0);
		}
	QString saveName = QString("moisture.%1.png").arg(planet.Id().GetID());
	qDebug() << "AuWeather::InitMoisture" << planet.Id() << planet.Name() << "saving" << saveName;
	bmCont.save(saveName);
	*/

	// Convert moisture from the range [0..peakLevel] to vapor pressure in bars.
	// Earth ranges from almost zero to about 4% water vapor by volume, or .04 bar.
	// We'll relate water vapor in the atmosphere to planetary hydrographics percentage and to overall atmosphere density.
	// Earth is 70% water and has 50% atmosphere density.
	// Calculate a cloudiness factor based on atmosphere density and surface water. Otherwise all planets will have about the
	// same cloudiness. A lower cloudiness factor produces less clouds; a higher cloudiness factor produces more clouds.
	float cloudiness = (atmDensity * surfaceWater) * 4;	// Range is 1 to 4, 1.4 for Earth.

	// Calculate how much actual vapor there is per increment currently in the m_Moisture array, which was created in whole
	// increments. The largest increment currently in the m_Moisture array is the peakLevel value.
	float eqTemp = planet.SeaLevelTempAt(AuGlobeLLF(0,0), m_SolarOrbitRadius, m_SunLightAtten);	// Temp at equator.
	float eqVP;
	if(planet.OrbitZone() == OZONE_Inner || planet.OrbitZone() == OZONE_Habitable)
		eqVP = VaporPressureOverWater(eqTemp);			// Vapor pressure at the equator on warm worlds.
	else
		eqVP = VaporPressureOverIce(eqTemp);			// Vapor pressure at the equator on cold worlds.
	float maxVapor = eqVP * cloudiness;					// Maximum vapor pressure in the atmosphere.
	float vaporInc = maxVapor / peakLevel;				// Vapor per m_Moisture array increment.

	// The distribution must be skewed with distance from the equator otherwise the colder
	// temperatures at the equator cause solid domes at the poles and leave the equator empty.
	// Planet temperature drops off toward the poles.
	// For now, let ringworlds have weather like a globe. It is colder near the hull walls.
	for(unsigned int ipy=0;ipy<Height();ipy++)
	{
		// Calculate a skew factor for the moisture at this latitude.
		float lat = ((float)ipy + .5f)/Height() * PIf - PIf/2;
		float latTemp = planet.SeaLevelTempAt(AuGlobeLLF(0,lat), m_SolarOrbitRadius, m_SunLightAtten);
		float latVP;
		if(planet.OrbitZone() == OZONE_Inner || planet.OrbitZone() == OZONE_Habitable)
			latVP = VaporPressureOverWater(latTemp);	// Vapor pressure on warm worlds.
		else
			latVP = VaporPressureOverIce(latTemp);		// Vapor pressure on cold worlds.
		float skew;
		if(eqVP > 0)
			skew = latVP / eqVP;
		else
			skew = 0;
		for(unsigned int ipx=0;ipx<Width();ipx++)
		{
			float* vapor = m_Moisture + ipx + ipy * Width();
			*vapor = *vapor * skew * vaporInc;
		}
	}
}

// Init atmospheric pressure data
void AuWeather::InitPressure(const DaWorld& planet, const AuDice& Cdice)
{
	float seaLevelPressure = planet.SeaLevelPressure();	// in bars
	if(seaLevelPressure == 0)
		return;
	AuDice dice(Cdice);

	// Allocate an array of floats to store atmospheric pressures in a
	// rectangular array, one value for each lat/long cell division.
	unsigned int arraysz = Height() * Width();
	m_Pressures = new float[arraysz];
	memset(m_Pressures, 0, arraysz*sizeof(float));

	int octaves = 6;
	float xoff = (float)dice.Roll(1,100000,-1) + (float)dice.Roll(1,100)/100.f;	// Offset into 2D perlin noise
	float yoff = (float)dice.Roll(1,100000,-1) + (float)dice.Roll(1,100)/100.f;	// Offset into 2D perlin noise
	float peakLevel = 0.f;
	float lowLevel = 0.f;
	float persistence = (float)dice.Roll(1, 30, 70)/100.f;
	float freqx = 4.f / (float)Width();
	float freqy = 4.f / (float)Height();
	int xblendwid = Width() * 20 / 100;		// Last 20% of array is blended to avoid a shear line.
	unsigned int xblendstart = Width() - xblendwid;
	for(unsigned int ifx=0; ifx<xblendstart; ifx++)
		for(unsigned int ify=0; ify<Height(); ify++)
		{
			float fx = (float)(ifx * freqx) + xoff;
			float fy = (float)(ify * freqy) + yoff;
			float press = AuNoiseF::PerlinNoise(octaves, persistence, fx, fy) * 100;
/*
if(ify < Height()/2 && ifx < Width()/2)
	press = 0;
else
	press = 100;
	*/
			if(press > peakLevel)
				peakLevel = press;
			if(press < lowLevel)
				lowLevel = press;

			float* pressure = m_Pressures + ifx + ify * Width();
			*pressure = press;
		}
	for(unsigned int ifx=xblendstart; ifx<Width(); ifx++)
	{
		float blendFactor = (float)(ifx - xblendstart)/(float)xblendwid;
		for(unsigned int ify=0; ify<Height(); ify++)
		{
			float fx = (float)(ifx * freqx) + xoff;
			float fy = (float)(ify * freqy) + yoff;
			float press1 = AuNoiseF::PerlinNoise(octaves, persistence, fx, fy) * 100;

			// Calculate blend
			int ibx = (int)(ifx - xblendstart) - xblendwid;
			float bx = (float)(ibx * freqx) + xoff;
			float press2 = AuNoiseF::PerlinNoise(octaves, persistence, bx, fy) * 100;

			float press = AuNoiseF::Cosine_Interpolate(press1, press2, blendFactor);
/*
if(ify < Height()/2 && ifx < Width()/2)
	press = 0;
else
	press = 100;
	*/
			if(press > peakLevel)
				peakLevel = press;
			if(press < lowLevel)
				lowLevel = press;

			float* pressure = m_Pressures + ifx + ify * Width();
			*pressure = press;
		}
	}

	/*
	float range = peakLevel - lowLevel;
	QImage bmCont(Width(), Height(), QImage::Format_ARGB32);
	for(unsigned int ifx=0; ifx<Width(); ifx++)
		for(unsigned int ify=0; ify<Height(); ify++)
		{
			float* pressure = m_Pressures + ifx + ify * Width();
			if(range > 0)
			{
				int colVal = (int)((*pressure - lowLevel) * 255.f/range);
				QRgb col;
				if(*pressure < 0)
					col = qRgb(0, 0, colVal);
				else
					col = qRgb(colVal, 0, 0);
				bmCont.setPixel(ifx, ify, col);
			}
			else
				bmCont.setPixel(ifx, ify, 0);
		}
	QString saveName = QString("pressure.%1.png").arg(planet.Id().GetID());
	qDebug() << "AuWeather::InitPressure" << planet.Id() << planet.Name() << "saving" << saveName;
	bmCont.save(saveName);
	*/

	// Convert pressures from the range [lowLevel..peakLevel] to relative pressures in bars.
	// Sea level pressure is a factor of planet's atmosphere density, which
	// has already been weighted due to its size so small worlds have thinner atmospheres.
	// Earth standard atmospheric pressures are denoted in 4 millibar increments.
	// Note: Gas giants are created with size 20-40 and atm density 90 to 100%.
	// Earth ranges from about 900 in severe L's to 1032 mbars in the Siberian H.
	const float maxPressure = seaLevelPressure * .032;	// 1032mb is sea level pressure (1mb) + .032mb
	//const float minPressure = seaLevelPressure * -.10;	// 900mb is sea level pressure (1mb) - .1mb
	const float maxpressfact = fabs(maxPressure / peakLevel);
	//const float minpressfact = fabs(minPressure / lowLevel);
	for(unsigned int ipy=0;ipy<Height();ipy++)
	{
		for(unsigned int ipx=0;ipx<Width();ipx++)
		{
			float* pressure = m_Pressures + ipx + ipy * Width();
			//if(*pressure >= 0)
				*pressure *= maxpressfact;
			//else
				//*pressure *= minpressfact;
		}
	}
}

// Returns 2D wind vector at a given longitude and latitude, in radians.
// Includes prevailing wind vector.
// Length of vector is wind strength in miles per hour.
// Our simple weather model doesn't account for wind direction changes at different altitudes.
QPointF AuWeather::WindAt(const DaWorld* planet, const AuGlobeLLF& longLat, float ewspoffset) const
{
	if(!m_Pressures)
		return QPointF(0,0);

	// Insure longitude and latitude are within valid ranges we're looking for.
	float fy = longLat.Latitude();
	if(fy < -PIf/2.f || fy > PIf/2.f)
		return QPointF(0,0);	// Latitude goes beyond poles.

	// Determine prevailing wind at latitude.
	// Strength of prevailing wind is affected by orbit zone. East-to-west winds
	// are stronger as solar radiation decreases, just right in the habitable zone.
	float prevSpeed;
	if(dynamic_cast<const DaWorldRingSection*>(planet))
		prevSpeed = PrevailingWindSpeed(planet->OrbitZone()) * fabs(cosf(fy));
	else
		prevSpeed = PrevailingWindSpeed(planet->OrbitZone()) * fabs(sinf(fy*6));

	// Planet genseed determines its direction of rotation.
	if(planet->GenSeed()&0x01)
		prevSpeed *= -1.f;	// ew rotation reversed

	// Determine prevailing winds at this latitude.
	// Can't get the weather shader to easily incorporate the NS portion of this.
	QPointF prevWind;
	if(fy >= 60*DEG2RADf)
	{
		// North Polar Circulation Cell
		prevWind = QPointF(-.7071f * prevSpeed,-.7071f * prevSpeed);	// NE wind
	}
	else if(fy >= 30*DEG2RADf)
	{
		// North Ferrel Circulation Cell
		prevWind = QPointF(.7071f * prevSpeed,.7071f * prevSpeed);	// SW wind
	}
	else if(fy >= 0)
	{
		// North Hadley Circulation Cell
		prevWind = QPointF(-.7071f * prevSpeed,-.7071f * prevSpeed);	// NE wind
	}
	else if(fy >= -30*DEG2RADf)
	{
		// South Hadley Circulation Cell
		prevWind = QPointF(-.7071f * prevSpeed,.7071f * prevSpeed);	// SE wind
	}
	else if(fy >= -60*DEG2RADf)
	{
		// South Ferrel Circulation Cell
		prevWind = QPointF(.7071f * prevSpeed,-.7071f * prevSpeed);	// NW wind
	}
	else
	{
		// South Polar Circulation Cell
		prevWind = QPointF(-.7071f * prevSpeed,.7071f * prevSpeed);	// SE wind
	}

	/*
	// Apply east-west offset due to band circulation.
	// Can't vary this offset because the cloud model isn't split into bands by the shader.
	float fx = longLat.x();
	if(fy >= 60.f*DEG2RADf)
	{
		// North Polar Circulation Cell
		//qDebug("North Polar Circulation Cell");
		//fx -= ewspoffset * PIf*2.f;	// E wind
		prevWind = QPointF(-prevSpeed,0);	// E wind
	}
	else if(fy >= 30.f*DEG2RADf)
	{
		// North Ferrel Circulation Cell
		//qDebug("North Ferrel Circulation Cell");
		//fx += ewspoffset * PIf*2.f;	// W wind
		prevWind = QPointF(prevSpeed,0);	// W wind
	}
	else if(fy >= 0)
	{
		// North Hadley Circulation Cell
		//qDebug("North Hadley Circulation Cell");
		//fx -= ewspoffset * PIf*2.f;	// E wind
		prevWind = QPointF(-prevSpeed,0);	// E wind
	}
	else if(fy >= -30.f*DEG2RADf)
	{
		// South Hadley Circulation Cell
		//qDebug("South Hadley Circulation Cell");
		//fx -= ewspoffset * PIf*2.f;	// E wind
		prevWind = QPointF(-prevSpeed,0);	// E wind
	}
	else if(fy >= -60.f*DEG2RADf)
	{
		// South Ferrel Circulation Cell
		//qDebug("South Ferrel Circulation Cell");
		//fx += ewspoffset * PIf*2.f;	// W wind
		prevWind = QPointF(prevSpeed,0);	// W wind
	}
	else
	{
		// South Polar Circulation Cell
		//qDebug("South Polar Circulation Cell");
		//fx -= ewspoffset * PIf*2.f;	// E wind
		prevWind = QPointF(-prevSpeed,0);	// E wind
	}
	*/
	// We can wrap longitude (fx) to be within the range 0 <= fx < PI*2
	float fx = longLat.Longitude() - ewspoffset * PIf*2.f;	// E wind because our shader doesn't do the bands
	while(fx < 0)
		fx += PIf*2.f;
	while(fx >= PIf*2.f)
		fx -= PIf*2.f;
	
	// Determine coordinates within rectangular projection of weather globe.
	unsigned int latDivs = Height();
	fx = fx/(PIf*2.f) * (float)Width();
	fy = (1 - (fy+PIf/2.f)/PIf) * (float)latDivs;
	int ix = (int)fx;	// Truncate to get pressure array column index.
	int iy = (int)fy;	// Truncate to get pressure array row index.
	if(iy >= (int)latDivs)
		iy = latDivs-1;

	// Determine pressure gradient force.
	QPointF gradvec;
	int gradvecs = 0;			// Number of vectors combined to calc average.
	float* pressure = m_Pressures + ix + iy * Width();
	float pressHere = *pressure;
	//qDebug("center pressure %g", *pressure);
	for(int relx=-2; relx<=2; relx++)
	{
		for(int rely=-2; rely<=2; rely++)
		{
			if(relx == 0 && rely == 0)
				continue;

			QPoint rel(ix + relx, iy + rely);
			if(!ClampLongLat(rel))
				continue;	// Skip polar rollovers.
			float dist = sqrt((float)(relx*relx + rely*rely));

			pressure = m_Pressures + rel.x() + rel.y() * Width();
			float pressNear = *pressure;
			// Press diff at unit dist
			float pressDiff = (pressHere - pressNear)/dist;

			QPointF relgrad(relx, rely);
			relgrad /= dist;		// Unit vector pointing toward nearby cell.
			//qDebug("relgrad %g,%g pressDiff %g",
					//relgrad.x(), relgrad.y(), pressDiff);
			relgrad *= pressDiff;	// Scale vector to pressure difference at
									// unit dist.

			// Accumulate sum of relative pressure gradient vectors.
			gradvec += relgrad;
			gradvecs++;
		}
	}

	// Calculate average of pressure gradient forces.
	gradvec /= gradvecs;

	// Isobar winds are perpendicular to pressure gradient forces due to effect of coriolis forces over time.
	QPointF isobarWind;
	if(iy < (int)latDivs/2)
	{
		// In north hemisphere, deflect pressure gradient vector to the right.
		isobarWind = QPointF(gradvec.y(), -gradvec.x());
	}
	else
	{
		// In south hemisphere, deflect pressure gradient vector to the left.
		isobarWind = QPointF(-gradvec.y(), gradvec.x());
	}

	// Scale the isobar wind to mph.
	// Wind is currently in bars. i.e. .001 = 1 millibar
	isobarWind *= 1000.f;
	isobarWind *= 10.f;		// multiply millibars by an arbitrary factor to produce mph

	// Combine isobar wind with prevailing wind.
	QPointF wind = isobarWind + prevWind;
	return wind;
}

float AuWeather::VaporPressureAt(const DaWorld& planet, const AuGlobeLLF& longLat, float altitudeMSL, float ewvpoffset) const
{
	if(!m_Moisture)
		return 0;

	// Insure longitude and latitude are within valid ranges we're looking for.
	float fy = longLat.Latitude();
	if(fy < -PIf/2.f || fy > PIf/2.f)
		return 0;	// Latitude goes beyond poles.

	// Apply east-west offset due to band circulation.
	float fx = longLat.Longitude();
	/*
	if(longLat.y() >= 60.f*DEG2RADf)
	{
		// North Polar Circulation Cell
		//qDebug("North Polar Circulation Cell");
		fx -= ewvpoffset * PIf*2.f;	// E wind
	}
	else if(longLat.y() >= 30.f*DEG2RADf)
	{
		// North Ferrel Circulation Cell
		//qDebug("North Ferrel Circulation Cell");
		fx += ewvpoffset * PIf*2.f;	// W wind
	}
	else if(longLat.y() >= 0)
	{
	*/
		// North Hadley Circulation Cell
		//qDebug("North Hadley Circulation Cell");
		fx -= ewvpoffset * PIf*2.f;	// E wind
		/*
	}
	else if(longLat.y() >= -30.f*DEG2RADf)
	{
		// South Hadley Circulation Cell
		//qDebug("South Hadley Circulation Cell");
		fx -= ewvpoffset * PIf*2.f;	// E wind
	}
	else if(longLat.y() >= -60.f*DEG2RADf)
	{
		// South Ferrel Circulation Cell
		//qDebug("South Ferrel Circulation Cell");
		fx += ewvpoffset * PIf*2.f;	// W wind
	}
	else
	{
		// South Polar Circulation Cell
		//qDebug("South Polar Circulation Cell");
		fx -= ewvpoffset * PIf*2.f;	// E wind
	}
	*/
	// We can wrap longitude (fx) to be within the range 0 <= fx < PI*2
	while(fx < 0)
		fx += PIf*2.f;
	while(fx >= PIf*2.f)
		fx -= PIf*2.f;
	
	// Determine coordinates within rectangular projection of weather globe.
	unsigned int latDivs = Height();
	fx = fx/(PIf*2.f) * (float)Width();
	fy = (1 - (fy+PIf/2.f)/PIf) * (float)latDivs;
	int ix = (int)fx;	// Truncate to get array column index.
	int iy = (int)fy;	// Truncate to get array row index.
	if(iy >= (int)latDivs)
		iy = latDivs-1;

	// Determine vapor pressure.
	// Unravel the m_Moisture data the way it was put in there by InitMoisture.
	const float* vapor = m_Moisture + ix + iy * Width();
	float vp = *vapor;		// Vapor pressure at sea level.

	// We don't need to interpolate here. The vapor pressure will remain constant according to the cloud layer the altitude falls
	// beneath. We are simulating cloud layers at 8k, 12k and 20k feet.
	if(altitudeMSL <= LayerAltitude(planet, LowClouds))
	{
		// Use low cloud layer vapor pressure.
		float lapseRate = VaporPressureLapse(LowClouds);
		vp *= lapseRate;
	}
	else if(altitudeMSL <= LayerAltitude(planet, MiddleClouds))
	{
		// Use middle cloud layer vapor pressure.
		float lapseRate = VaporPressureLapse(MiddleClouds);
		vp *= lapseRate;
	}
	/*
	else if(altitudeMSL <= LayerAltitude(planet, HighClouds))
	{
		// Use middle cloud layer vapor pressure.
		float lapseRate = VaporPressureLapse(HighClouds);
		vp *= lapseRate;
	}
	*/
	else
	{
		// Use top of atmosphere vapor pressure.
		float lapseRate = VaporPressureLapse(UpperLimit);
		vp *= lapseRate;
	}

	return vp;
}

// Returns temperature at a given long/lat and altitude in kelvin.
// Temperature is adjusted for relative atmospheric pressure.
float AuWeather::TemperatureAt(const DaWorld& planet, const AuGlobeLLF& longLat, LAYER layer) const
{
	if(!m_Pressures)
		return 0;
	if(layer == UpperLimit)
		return 0;	// 0 Kelvin at upper limit of atmosphere

	// Insure longitude and latitude are within valid ranges we're looking for.
	float fy = longLat.Latitude();
	if(fy < -PIf/2 || fy > PIf/2)
		return 0;	// Latitude goes beyond poles.
	// We can wrap longitude (fx) to be within the range 0 <= fx < PI*2
	float fx = longLat.Longitude();
	while(fx < 0)
		fx += PIf*2;
	while(fx >= PIf*2)
		fx -= PIf*2;
	
	// Determine coordinates within rectangular projection of weather globe.
	unsigned int latDivs = Height();
	fx = fx/(PIf*2) * (float)Width();
	fy = (1 - (fy+PIf/2)/PIf) * (float)latDivs;
	int ix = (int)fx;	// Truncate to get pressure array column index.
	int iy = (int)fy;	// Truncate to get pressure array row index.
	if(iy >= (int)latDivs)
		iy = latDivs-1;
	float* pressure = m_Pressures + ix + iy * Width();
	float atmPressure = *pressure;

	// We are simulating cloud layers at 8k, 12k and 20k feet.
	float seaLevelTemp = planet.SeaLevelTempAt(longLat, m_SolarOrbitRadius, m_SunLightAtten);	// in kelvin
	float temp = seaLevelTemp - TemperatureLapse(layer);

	// Adjust temperature with relative atmospheric pressure, which is in bars.
	// Due to adiabatic cooling, increased atmospheric pressure translates to lower
	// temperatures.
	// 1/2"Hg (12.7mm) equates to approximately one degree of temperature change.
	// 12.7mm Hg = 16.9333mbar (1mbar = .75mm Hg)
	temp -= (atmPressure / .0169333f) * 10;
	//if(planet.IsHabitablePlanetOrMoon())
		//qDebug("TemperatureAt %d,%d temp %g atmPressure %g change due to p %g", ix,iy,
				//temp, atmPressure, (atmPressure / .0169333f));
	if(temp < 0)
		temp = 0;

	return temp;
}

// Returns temperature at a given long/lat and altitude in kelvin.
// Temperature is adjusted for relative atmospheric pressure.
float AuWeather::TemperatureAt(const DaWorld& planet, const AuGlobeLLF& longLat, float altitudeMSL, float ewspoffset) const
{
	if(!m_Pressures)
		return 0;

	// Insure longitude and latitude are within valid ranges we're looking for.
	float fy = longLat.Latitude();
	if(fy < -PIf/2 || fy > PIf/2)
		return 0;	// Latitude goes beyond poles.

	// Apply east-west offset due to band circulation.
	float fx = longLat.Longitude();
	/*
	if(longLat.y() >= 60.f*DEG2RADf)
	{
		// North Polar Circulation Cell
		//qDebug("North Polar Circulation Cell");
		fx -= ewspoffset * PIf*2.f;	// E wind
	}
	else if(longLat.y() >= 30.f*DEG2RADf)
	{
		// North Ferrel Circulation Cell
		//qDebug("North Ferrel Circulation Cell");
		fx += ewspoffset * PIf*2.f;	// W wind
	}
	else if(longLat.y() >= 0)
	{
	*/
		// North Hadley Circulation Cell
		//qDebug("North Hadley Circulation Cell");
		fx -= ewspoffset * PIf*2.f;	// E wind
		/*
	}
	else if(longLat.y() >= -30.f*DEG2RADf)
	{
		// South Hadley Circulation Cell
		//qDebug("South Hadley Circulation Cell");
		fx -= ewspoffset * PIf*2.f;	// E wind
	}
	else if(longLat.y() >= -60.f*DEG2RADf)
	{
		// South Ferrel Circulation Cell
		//qDebug("South Ferrel Circulation Cell");
		fx += ewspoffset * PIf*2.f;	// W wind
	}
	else
	{
		// South Polar Circulation Cell
		//qDebug("South Polar Circulation Cell");
		fx -= ewspoffset * PIf*2.f;	// E wind
	}
	*/

	// We can wrap longitude (fx) to be within the range 0 <= fx < PI*2
	while(fx < 0)
		fx += PIf*2;
	while(fx >= PIf*2)
		fx -= PIf*2;
	
	// Determine coordinates within rectangular projection of weather globe.
	unsigned int latDivs = Height();
	fx = fx/(PIf*2) * (float)Width();
	fy = (1 - (fy+PIf/2)/PIf) * (float)latDivs;
	int ix = (int)fx;	// Truncate to get pressure array column index.
	int iy = (int)fy;	// Truncate to get pressure array row index.
	if(iy >= (int)latDivs)
		iy = latDivs-1;
	float* pressure = m_Pressures + ix + iy * Width();
	float atmPressure = *pressure;

	// We need to calculate a nonuniform temperature drop-off because we are simulating
	// cloud layers at 8k, 12k and 20k feet but we are placing them where they look
	// nice in our distorted game universe.
	float temp;
	float seaLevelTemp = planet.SeaLevelTempAt(longLat, m_SolarOrbitRadius, m_SunLightAtten);	// in kelvin
	float lowClouds = LayerAltitude(planet, LowClouds);
	if(altitudeMSL <= lowClouds)
	{
		// Lapse temperature from ground to low cloud layer.
		float lowCloudTemp = seaLevelTemp - TemperatureLapse(LowClouds);
		float lapseRate = (lowCloudTemp - seaLevelTemp) / lowClouds;
		temp = seaLevelTemp + lapseRate * altitudeMSL;
	}
	else
	{
		float midClouds = LayerAltitude(planet, MiddleClouds);
		if(altitudeMSL <= midClouds)
		{
			// Lapse temperature from low cloud layer to middle cloud layer.
			float lowCloudTemp = seaLevelTemp - TemperatureLapse(LowClouds);
			float midCloudTemp = seaLevelTemp - TemperatureLapse(MiddleClouds);
			float lapseRate = (midCloudTemp - lowCloudTemp) / (midClouds - lowClouds);
			temp = lowCloudTemp + lapseRate * (altitudeMSL - lowClouds);
		}
		else
		{
			/*
			float highClouds = LayerAltitude(planet, HighClouds);
			if(altitudeMSL <= highClouds)
			{
				// Lapse temperature from middle cloud layer to high cloud layer.
				float midCloudTemp = seaLevelTemp - TemperatureLapse(MiddleClouds);
				float highCloudTemp = seaLevelTemp - TemperatureLapse(HighClouds);
				float lapseRate = (highCloudTemp - midCloudTemp) / (highClouds - midClouds);
				temp = midCloudTemp + lapseRate * (altitudeMSL - midClouds);
			}
			else
			{
				float atmTop = LayerAltitude(planet, UpperLimit);
				if(altitudeMSL <= atmTop)
				{
					// Lapse temperature high cloud layer to top of atmosphere.
					float highCloudTemp = seaLevelTemp - TemperatureLapse(HighClouds);
					float atmTopTemp = 0;
					float lapseRate = (atmTopTemp - highCloudTemp) / (atmTop - highClouds);
					temp = highCloudTemp + lapseRate * (altitudeMSL - highClouds);
				}
				else
				{
					temp = 0;
				}
			}
			*/
			float atmTop = LayerAltitude(planet, UpperLimit);
			if(altitudeMSL <= atmTop)
			{
				// Lapse temperature high cloud layer to top of atmosphere.
				float midCloudTemp = seaLevelTemp - TemperatureLapse(MiddleClouds);
				float atmTopTemp = 0;
				float lapseRate = (atmTopTemp - midCloudTemp) / (atmTop - midClouds);
				temp = midCloudTemp + lapseRate * (altitudeMSL - midClouds);
			}
			else
			{
				temp = 0;
			}
		}
	}

	// Adjust temperature with relative atmospheric pressure, which is in bars.
	// Due to adiabatic cooling, increased atmospheric pressure translates to lower
	// temperatures.
	// 1/2"Hg (12.7mm) equates to approximately one degree of temperature change.
	// 12.7mm Hg = 16.9333mbar (1mbar = .75mm Hg)
	temp -= (atmPressure / .0169333f) * 10;
	//if(planet.IsHabitablePlanetOrMoon())
		//qDebug("TemperatureAt %d,%d temp %g atmPressure %g change due to p %g", ix,iy,
				//temp, atmPressure, (atmPressure / .0169333f));
	if(temp < 0)
		temp = 0;

	return temp;
}

float AuWeather::PressureAt(const DaWorld& planet, const AuGlobeLLF& longLat, float altitudeMSL, float ewspoffset) const
{
	if(!m_Pressures)
		return 0;

	// Insure longitude and latitude are within valid ranges we're looking for.
	float fy = longLat.Latitude();
	if(fy < -PIf/2 || fy > PIf/2)
		return 0;	// Latitude goes beyond poles.

	// Apply east-west offset due to band circulation.
	float fx = longLat.Longitude();
	/*
	if(longLat.y() >= 60.f*DEG2RADf)
	{
		// North Polar Circulation Cell
		//qDebug("North Polar Circulation Cell");
		fx -= ewspoffset * PIf*2.f;	// E wind
	}
	else if(longLat.y() >= 30.f*DEG2RADf)
	{
		// North Ferrel Circulation Cell
		//qDebug("North Ferrel Circulation Cell");
		fx += ewspoffset * PIf*2.f;	// W wind
	}
	else if(longLat.y() >= 0)
	{
	*/
		// North Hadley Circulation Cell
		//qDebug("North Hadley Circulation Cell");
		fx -= ewspoffset * PIf*2.f;	// E wind
		/*
	}
	else if(longLat.y() >= -30.f*DEG2RADf)
	{
		// South Hadley Circulation Cell
		//qDebug("South Hadley Circulation Cell");
		fx -= ewspoffset * PIf*2.f;	// E wind
	}
	else if(longLat.y() >= -60.f*DEG2RADf)
	{
		// South Ferrel Circulation Cell
		//qDebug("South Ferrel Circulation Cell");
		fx += ewspoffset * PIf*2.f;	// W wind
	}
	else
	{
		// South Polar Circulation Cell
		//qDebug("South Polar Circulation Cell");
		fx -= ewspoffset * PIf*2.f;	// E wind
	}
	*/

	// We can wrap longitude (fx) to be within the range 0 <= fx < PI*2
	while(fx < 0)
		fx += PIf*2;
	while(fx >= PIf*2)
		fx -= PIf*2;
	
	// Determine coordinates within rectangular projection of weather globe.
	unsigned int latDivs = Height();
	fx = fx/(PIf*2) * (float)Width();
	fy = (1 - (fy+PIf/2)/PIf) * (float)latDivs;
	int ix = (int)fx;	// Truncate to get pressure array column index.
	int iy = (int)fy;	// Truncate to get pressure array row index.
	if(iy >= (int)latDivs)
		iy = latDivs-1;
	float* pressure = m_Pressures + ix + iy * Width();
	float atmPressure = *pressure;	// in bars, relative to sea level pressure.

	// We don't need to calculate a non-uniform pressure drop-off because this value is used for a non-critical UI display.
	// Calculate linear drop off from sea level pressure to 0 at the maximum height.
	float atmTop = LayerAltitude(planet, UpperLimit);	// MSL
	if(altitudeMSL < atmTop)
	{
		float lapsePressPerAlt = atmPressure / atmTop;	// pressure to lose per altitude gain from sea level
		atmPressure -= lapsePressPerAlt * altitudeMSL;	// pressure adjusted by altitude lapse
		atmPressure += planet.SeaLevelPressure();		// in bars
		if(atmPressure < 0)
			atmPressure = 0;
	}
	else
		atmPressure = 0;
	return atmPressure;
}

// Returns temperature at the cloud layer above a given long/lat and altitude, in kelvin.
// Temperature is adjusted for relative atmospheric pressure.
float AuWeather::CloudTemperatureAt(const DaWorld& planet, const AuGlobeLLF& longLat, float altitudeMSL, float ewspoffset) const
{
	if(!m_Pressures)
		return 0;

	// Insure longitude and latitude are within valid ranges we're looking for.
	float fy = longLat.Latitude();
	if(fy < -PIf/2 || fy > PIf/2)
		return 0;	// Latitude goes beyond poles.

	// Apply east-west offset due to band circulation.
	float fx = longLat.Longitude();
	/*
	if(longLat.y() >= 60.f*DEG2RADf)
	{
		// North Polar Circulation Cell
		//qDebug("North Polar Circulation Cell");
		fx -= ewspoffset * PIf*2.f;	// E wind
	}
	else if(longLat.y() >= 30.f*DEG2RADf)
	{
		// North Ferrel Circulation Cell
		//qDebug("North Ferrel Circulation Cell");
		fx += ewspoffset * PIf*2.f;	// W wind
	}
	else if(longLat.y() >= 0)
	{
	*/
		// North Hadley Circulation Cell
		//qDebug("North Hadley Circulation Cell");
		fx -= ewspoffset * PIf*2.f;	// E wind
		/*
	}
	else if(longLat.y() >= -30.f*DEG2RADf)
	{
		// South Hadley Circulation Cell
		//qDebug("South Hadley Circulation Cell");
		fx -= ewspoffset * PIf*2.f;	// E wind
	}
	else if(longLat.y() >= -60.f*DEG2RADf)
	{
		// South Ferrel Circulation Cell
		//qDebug("South Ferrel Circulation Cell");
		fx += ewspoffset * PIf*2.f;	// W wind
	}
	else
	{
		// South Polar Circulation Cell
		//qDebug("South Polar Circulation Cell");
		fx -= ewspoffset * PIf*2.f;	// E wind
	}
	*/

	// We can wrap longitude (fx) to be within the range 0 <= fx < PI*2
	while(fx < 0)
		fx += PIf*2;
	while(fx >= PIf*2)
		fx -= PIf*2;
	
	// Determine coordinates within rectangular projection of weather globe.
	unsigned int latDivs = Height();
	fx = fx/(PIf*2) * (float)Width();
	fy = (1 - (fy+PIf/2)/PIf) * (float)latDivs;
	int ix = (int)fx;	// Truncate to get pressure array column index.
	int iy = (int)fy;	// Truncate to get pressure array row index.
	if(iy >= (int)latDivs)
		iy = latDivs-1;
	float* pressure = m_Pressures + ix + iy * Width();
	float atmPressure = *pressure;

	// We need to calculate a nonuniform temperature drop-off because we are simulating
	// cloud layers at 8k, 12k and 20k feet but we are placing them where they look
	// nice in our distorted game universe.
	float temp;
	float seaLevelTemp = planet.SeaLevelTempAt(longLat, m_SolarOrbitRadius, m_SunLightAtten);	// in kelvin
	float lowClouds = LayerAltitude(planet, LowClouds);
	if(altitudeMSL <= lowClouds)
	{
		temp = seaLevelTemp - TemperatureLapse(LowClouds);
	}
	else
	{
		float midClouds = LayerAltitude(planet, MiddleClouds);
		if(altitudeMSL <= midClouds)
		{
			temp = seaLevelTemp - TemperatureLapse(MiddleClouds);
		}
		else
		{
			/*
			float highClouds = LayerAltitude(planet, HighClouds);
			if(altitudeMSL <= highClouds)
			{
				temp = seaLevelTemp - TemperatureLapse(HighClouds);
			}
			else
			*/
			{
				temp = 0;
			}
		}
	}

	// Adjust temperature with relative atmospheric pressure, which is in bars.
	// Due to adiabatic cooling, increased atmospheric pressure translates to lower
	// temperatures.
	// 1/2"Hg (12.7mm) equates to approximately one degree of temperature change.
	// 12.7mm Hg = 16.9333mbar (1mbar = .75mm Hg)
	temp -= (atmPressure / .0169333f) * 10;
	//if(planet.IsHabitablePlanetOrMoon())
		//qDebug("TemperatureAt %d,%d temp %g atmPressure %g change due to p %g", ix,iy,
				//temp, atmPressure, (atmPressure / .0169333f));
	if(temp < 0)
		temp = 0;

	return temp;
}

// Constructs an array of data suitable for creation of a floating point texture
// used by the cloud vertex shader.
// This data represents saturation vapor pressure, actual vapor pressure, and
// instability as floating point numbers in the range [0..1], like RGB values,
// so a texture can be created from the data.
void AuWeather::ShaderData(const DaWorld& planet, float* wx, float& maxSP, float& maxVP, float& minSP, float& minVP, LAYER layer,
		const AuDice& Cdice) const
{
	if(!m_Moisture || !m_Pressures)
		return;
	AuDice dice(Cdice);

	float* pwx = wx;
	unsigned int latDivs = Height();
	for(unsigned int iy=0; iy<latDivs; iy++)
	{
		for(unsigned int ix=0; ix<Width(); ix++)
		{
			// Get atmosphere layer temperature in kelvin adjusted for pressure.
			float lon = (float)(ix + .5f)/(float)Width() * PIf*2;					// ix=0 is 0 degrees longitude
			float lat = PIf/2 - (float)(iy + .5f)/(float)latDivs * PIf;				// iy=0 is 90 degrees latitude
			float atmLayerTemp = TemperatureAt(planet, AuGlobeLLF(lon,lat), layer);

			float saturationVaporPressure;
			switch(planet.OrbitZone())
			{
#if defined(QT_NO_DEBUG)
			default:											// Eliminates compiler warning in release build.
#endif
			case OZONE_Inferno:
			case OZONE_DeepSpace:
				saturationVaporPressure = 0;
				break;
			case OZONE_Inner:
			case OZONE_Habitable:
				saturationVaporPressure = VaporPressureOverWater(atmLayerTemp);
				break;
			case OZONE_Outer:
			case OZONE_Frigid:
				saturationVaporPressure = VaporPressureOverIce(atmLayerTemp);
				break;
			}

			*pwx = saturationVaporPressure;
			pwx++;
			if(iy == 0 && ix == 0)
			{
				minSP = saturationVaporPressure;
				maxSP = saturationVaporPressure;
			}
			else
			{
				if(saturationVaporPressure < minSP)
					minSP = saturationVaporPressure;
				if(saturationVaporPressure > maxSP)
					maxSP = saturationVaporPressure;
			}

			// Vapor pressure in bars.
			const float* vp = m_Moisture + ix + iy * Width();
			*pwx = *vp * VaporPressureLapse(layer);
			pwx++;
			if(iy == 0 && ix == 0)
			{
				minVP = *vp;
				maxVP = *vp;
			}
			else
			{
				if(*vp < minVP)
					minVP = *vp;
				if(*vp > maxVP)
					maxVP = *vp;
			}

			// Instability - Low pressure means more relative temp and more instability.
			float* pressure = m_Pressures + ix + iy * Width();
			float instability = (1.f - *pressure) * 100.f;
			if(instability < 0)
				instability = 0;
			else if(instability > 1)
				instability = 1;
			*pwx = (float)dice.Roll(1,11,-1) * instability;
			pwx++;
		}
	}

	// Scale values to the range 0..1
	// The scaling is necessary due to the 0..1 clamping applied somewhere
	// along the way to RGB values in the fragment shader.
	pwx = wx;
	float rangeSP = maxSP - minSP;
	float rangeVP = maxVP - minVP;
	for(unsigned int iy=0; iy<latDivs; iy++)
	{
		for(unsigned int ix=0; ix<Width(); ix++)
		{
			// Saturation Vapor Pressure
			if(rangeSP != 0.)
				*pwx = (*pwx - minSP)/rangeSP;
			pwx++;

			// Vapor Pressure
			if(rangeVP != 0.)
				*pwx = (*pwx - minVP)/rangeVP;
			pwx++;

			// Instability is already in the proper range.
			pwx++;
		}
	}
}

//static
float AuWeather::LayerAltitude(const DaWorld& planet, LAYER layer)
{
	const float atmHeight = planet.AtmosphereHeightMSL();
	float cloudLayerAltitude;
	switch(layer)
	{
		/*
	case HighClouds:
		cloudLayerAltitude = atmHeight*6./10.;
		break;
		*/
	case MiddleClouds:
		cloudLayerAltitude = atmHeight*4/10;
		break;
	case LowClouds:
		cloudLayerAltitude = atmHeight*2/10;
		break;
	case UpperLimit:
	default:
		cloudLayerAltitude = atmHeight;	// Limit of visible atmosphere.
		break;
	}
	return cloudLayerAltitude;
}

// 'relHumidity' is the ratio of vapor pressure to saturation pressure. >1 indicates precip
// 'oat' is output in celsius
void AuWeather::ConditionsAt(const DaWorld& planet, const AuGlobeLLF& longLat, float altitudeMSL,
		float ewspoffset, float ewvpoffset, float& relHumidity, float& oat, float& bar) const
{
	if(planet.OrbitZone() == OZONE_Inferno)
	{
		// Inferno worlds do not have humidity.
		relHumidity = 0;
	}
	else
	{
		// Calculate cloud air temperature and saturation vapor pressure.
		// Precip starts at the clouds and continues. Otherwise the temperature lapse results in too little moisture
		// reaching the ground. I guess we don't get virga in our world.
		float cloudTemp = CloudTemperatureAt(planet, longLat, altitudeMSL, ewspoffset);	// in Kelvin

		// Determine vapor pressure at the planet location and altitude.
		float vaporPressure = VaporPressureAt(planet, longLat, altitudeMSL, ewvpoffset);
		//const float vpThreshold = 0.002f;	// Amount of vapor pressure needed to calc relative humidity.
		//if(vaporPressure > vpThreshold)
		{
			float saturationVaporPressure;
			if(planet.OrbitZone() == OZONE_Inner || planet.OrbitZone() == OZONE_Habitable)
				saturationVaporPressure = VaporPressureOverWater(cloudTemp);
			else
				saturationVaporPressure = VaporPressureOverIce(cloudTemp);
			if(saturationVaporPressure != 0)
				relHumidity = vaporPressure / saturationVaporPressure;	// Calculate relative humidity.
			else
			{
				//qDebug() << "AuWeather::PrecipitationAt" << planet.Id() << planet.Name() << "low satVP" << saturationVaporPressure
					//<< "VP" << vaporPressure << "cloudTemp" << cloudTemp;
				relHumidity = 0;
			}
		}
		//else
		//{
			//qDebug() << "AuWeather::PrecipitationAt" << planet.Id() << planet.Name() << "low vp" << vaporPressure;
			//relHumidity = 0;
		//}
	}

	// Calculate outside air temperature.
	oat = TemperatureAt(planet, longLat, altitudeMSL, ewspoffset);	// in Kelvin
	oat -= 273.f;	// Convert Kelvin to Celsius

	// Calculate outside air pressure, in bars.
	bar = PressureAt(planet, longLat, altitudeMSL, ewspoffset);	// in bars
}

