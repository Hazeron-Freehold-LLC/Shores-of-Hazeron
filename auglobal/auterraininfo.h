//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auterraininfo.h,v 1.25 2022/02/19 18:31:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuTerrainInfo_H
#define AuTerrainInfo_H

#include "audef.h"
#include "augloballib.h"
#include "aupoint.h"

class AUGLOBAL_DLL AuTerrainInfo  
{
public:
	quint8			m_byFoliage;				// Vegetation quantity (0-100)
	quint8			m_byWater;					// Percentage of planet surface covered in water (0-100)
	TER_ELEV_TYPE	m_ElevType;					// Terrain elevation type (TER_ELEV_TYPE)
	int				m_InEnclosure;				// 0=no, 1=land Enclosure, 2=undersea Enclosure
	int				m_Irrigation;				// 0=none, 1=irrigated, 2=defoliated
	quint16			m_Radius100km;				// Radius of planet in 100's of kilometers.
	double			m_rSeaLevelRadius;			// Radius of planet's sea level. Value returned by DaPlanet::Radius()
	double			m_rSurfaceRadius;			// Radius to planet's surface for a particular location.

public:
	inline AuTerrainInfo()
	{
		m_byFoliage = 0;
		m_byWater = 0;
		m_ElevType = TER_Peak;
		m_InEnclosure = 0;
		m_Irrigation = 0;
		m_Radius100km = 0;
		m_rSeaLevelRadius = 0.0;
		m_rSurfaceRadius = 0.0;
	}
	
	inline ~AuTerrainInfo()
	{
	}

	inline AuTerrainInfo(const AuTerrainInfo& c)
	{
		m_byFoliage = c.m_byFoliage;
		m_byWater = c.m_byWater;
		m_ElevType = c.m_ElevType;
		m_InEnclosure = c.m_InEnclosure;
		m_Irrigation = c.m_Irrigation;
		m_Radius100km = c.m_Radius100km;
		m_rSeaLevelRadius = c.m_rSeaLevelRadius;
		m_rSurfaceRadius = c.m_rSurfaceRadius;
	}

public:	// Operators
	inline const AuTerrainInfo& operator=(const AuTerrainInfo& c)
	{
		m_byFoliage = c.m_byFoliage;
		m_byWater = c.m_byWater;
		m_ElevType = c.m_ElevType;
		m_InEnclosure = c.m_InEnclosure;
		m_Irrigation = c.m_Irrigation;
		m_Radius100km = c.m_Radius100km;
		m_rSeaLevelRadius = c.m_rSeaLevelRadius;
		m_rSurfaceRadius = c.m_rSurfaceRadius;
		return *this;
	}

public:	// Methods
	inline bool InEnclosure() const
	{
		return m_InEnclosure > 0;
	}

	inline bool InEnclosureOnLand() const
	{
		return m_InEnclosure == 1;
	}

	inline bool InEnclosureUnderSea() const
	{
		return m_InEnclosure == 2;
	}

	inline bool IsDefoliated() const
	{
		return m_Irrigation == 2;
	}

	inline bool IsGasGiant() const
	{
		return m_Radius100km >= 200 && m_Radius100km <= 400;
	}

	inline bool IsIrrigated() const
	{
		return m_Irrigation == 1;
	}

	inline bool IsRingworld() const
	{
		return m_Radius100km == 500;
	}

	inline bool IsWater() const
	{
		if(!m_byWater)
			return false;				// World has no standing water.
		return m_rSurfaceRadius < m_rSeaLevelRadius;
	}

	// Returns maximum number of animals that will spawn in a terrain patch.
	// The caller must already insure there are animals to spawn, i.e. the terrain is habitable and contains fauna.
	// Range of return values is 3 to 8, depending on foliage density.
	inline int MaxAnimals() const
	{
		// Max foliage is 100.
		return 3 + m_byFoliage/20;
	}

	inline TER_ELEV_TYPE TerrainElevationType() const
	{
		return m_ElevType;
	}

	inline TERRAIN TerrainType() const
	{
		if(m_ElevType >= TER_ContinentalShelf)
			return Underwater;

		const int treechance = m_byFoliage * m_byWater/100;
		const int trees = m_byFoliage * treechance / 100;	// number of trees probable
		if(m_ElevType == TER_Coastal)
		{
			// Sea Level
			if(m_byFoliage < 15)
				return SeaLevelDesert;
			else if(trees > 15)
				return SeaLevelForest;
			else
				return SeaLevelSavanna;
		}
		else if(m_ElevType == TER_Lowlands)
		{
			// Lowland
			if(m_byFoliage < 15)
				return LowlandDesert;
			else if(trees > 15)
				return LowlandForest;
			else
				return LowlandSavanna;
		}
		else
		{
			// Highland
			if(m_byFoliage < 15)
				return HighlandDesert;
			else if(trees > 15)
				return HighlandForest;
			else
				return HighlandSavanna;
		}
	}

	QString TerrainTypeName() const;

	inline double WaterDepth() const
	{
		if(!IsWater())
			return 0;				// World has no standing water.
		return m_rSeaLevelRadius - m_rSurfaceRadius;
	}
};

#endif // AuTerrainInfo_H

