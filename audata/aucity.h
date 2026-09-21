//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucity.h,v 1.22 2022/02/19 18:18:55 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCity_H
#define AuCity_H

#include "aucityreport.h"
#include "audef.h"
#include "auhash.h"

// This class is a list of all the cities the empire has control of as well as which planet, system, and sector its in.
class AUDATA_DLL AuCity
{
private:
	quint8			m_AlertCondition;	// ALERT_CONDITION
	quint8			m_CityType;			// CITYTYPE
	AuCityIndustryHash	m_Facilities;	// Facilities present in the city.
	AuId			m_FounderId;		// Founder/owner of the city.
	QString			m_FounderName;
	quint8			m_Galaxy;			// GALAXY for quick reference;
	bool			m_GalaxyCapital;	// true if the city is the capital city of the galaxy.
	AuId			m_idGalaxy;			// Id of the galaxy;
	AuId			m_idSector;			// Id of the sector;
	AuId			m_idSystem;			// Id of the system in this sector;
	AuId			m_idPlanet;			// Id of the planet/moon in this system;
	AuId			m_idCity;			// Id of the city for this planet/moon;
	qint8			m_Morale;			// Current morale.
	QString			m_sSector;			// Name of the sector for quick reference;
	QString			m_sSystem;			// Name of the system for quick reference;
	QString			m_sPlanet;			// Name of the planet/moon for quick reference;
	QString			m_sCity;			// Name of the city for quick reference;
	bool			m_SectorCapital;	// true if the city is the capital city of the sector.
	AuPoint			m_SystemLoc;		// Solar system galactic coordinates.

public:	// Constructors
	inline AuCity(CITYTYPE cityType, const AuId& cityId, const QString& cityName, const AuId& founderId, const QString& founderName,
			const AuId& planetId, const QString& planetName,
			const AuId& systemId, const QString& systemName, const AuPoint& systemLoc,
			const AuId& sectorId, const QString& sectorName,
			const AuId& galaxyId, GALAXY galaxy, bool isSectorCapital, bool isGalaxyCapital)
	{
		m_AlertCondition = ConditionGreen;
		m_CityType = cityType;
		m_FounderId = founderId;
		m_FounderName = founderName;
		m_Galaxy = galaxy;
		m_GalaxyCapital = isGalaxyCapital;
		m_idGalaxy = galaxyId;
		m_idSector = sectorId;
		m_idSystem = systemId;
		m_idPlanet = planetId;
		m_idCity = cityId;
		m_Morale = 0;
		m_sSector = sectorName;
		m_sSystem = systemName;
		m_sPlanet = planetName;
		m_sCity = cityName;
		m_SectorCapital = isSectorCapital;
		m_SystemLoc = systemLoc;
	}

	inline AuCity()
	{
		m_AlertCondition = ConditionGreen;
		m_CityType = CITY_Capitol;
		m_Galaxy = ShoresOfHazeron;
		m_GalaxyCapital = false;
		m_Morale = 0;
		m_SectorCapital = false;
	}

	inline AuCity(const AuCity& c) : m_Facilities(c.m_Facilities)
	{
		m_AlertCondition = c.m_AlertCondition;
		m_CityType = c.m_CityType;
		m_FounderId = c.m_FounderId;
		m_FounderName = c.m_FounderName;
		m_Galaxy = c.m_Galaxy;
		m_GalaxyCapital = c.m_GalaxyCapital;
		m_idGalaxy = c.m_idGalaxy;
		m_idSector = c.m_idSector;
		m_idSystem = c.m_idSystem;
		m_idPlanet = c.m_idPlanet;
		m_idCity = c.m_idCity;
		m_Morale = c.m_Morale;
		m_sSector = c.m_sSector;
		m_sSystem = c.m_sSystem;
		m_sPlanet = c.m_sPlanet;
		m_sCity = c.m_sCity;
		m_SectorCapital = c.m_SectorCapital;
		m_SystemLoc = c.m_SystemLoc;
	}

public:	// Operators
	inline AuCity& operator=(const AuCity& c)
	{
		m_AlertCondition = c.m_AlertCondition;
		m_CityType = c.m_CityType;
		m_Facilities = c.m_Facilities;
		m_FounderId = c.m_FounderId;
		m_FounderName = c.m_FounderName;
		m_Galaxy = c.m_Galaxy;
		m_GalaxyCapital = c.m_GalaxyCapital;
		m_idGalaxy = c.m_idGalaxy;
		m_idSector = c.m_idSector;
		m_idSystem = c.m_idSystem;
		m_idPlanet = c.m_idPlanet;
		m_idCity = c.m_idCity;
		m_Morale = c.m_Morale;
		m_sSector = c.m_sSector;
		m_sSystem = c.m_sSystem;
		m_sPlanet = c.m_sPlanet;
		m_sCity = c.m_sCity;
		m_SectorCapital = c.m_SectorCapital;
		m_SystemLoc = c.m_SystemLoc;
		
		return *this;
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	bool operator==(const AuCity& c) const;

public:	// Methods
	inline ALERT_CONDITION AlertCondition() const
	{
		return (ALERT_CONDITION)m_AlertCondition;
	}

	// Returns the name of a resource image that can be used as an icon for the city type.
	QString CityIcon() const;
	static QString CityIcon(CITYTYPE cityType);

	inline const AuId& CityId() const
	{
		return m_idCity;			// Id of the city for this planet/moon;
	}

	inline const QString& CityName() const
	{
		return m_sCity;
	}

	inline CITYTYPE CityType() const
	{
		return (CITYTYPE)m_CityType;
	}

	inline const AuCityIndustryHash& Facilities() const
	{
		return m_Facilities;
	}

	inline const AuId& FounderId() const
	{
		return m_FounderId;
	}

	inline const QString& FounderName() const
	{
		return m_FounderName;
	}

	inline GALAXY Galaxy() const
	{
		return (GALAXY)m_Galaxy;
	}

	inline const AuId& GalaxyId() const
	{
		return m_idGalaxy;
	}

	inline QString GalaxyName() const
	{
		return ::GalaxyName(Galaxy());
	}

	inline bool IsGalaxyCapital() const
	{
		return m_GalaxyCapital;
	}

	inline bool IsSectorCapital() const
	{
		return m_SectorCapital;
	}

	// Returns an arbitrary number used when decide which city a new avatar will join.
	int JoinScore(const AuId& empireCapitol) const;

	quint64 MemoryUsage() const;

	inline qint8 Morale() const
	{
		return m_Morale;
	}

	inline const AuId& PlanetId() const
	{
		return m_idPlanet;
	}

	inline const QString& PlanetName() const
	{
		return m_sPlanet;
	}

	inline const AuId& SectorId() const
	{
		return m_idSector;
	}

	inline const QString& SectorName() const
	{
		return m_sSector;
	}

	inline void SetAlertCondition(ALERT_CONDITION ac)
	{
		m_AlertCondition = ac;
	}

	inline void SetCityName(const QString& name)
	{
		m_sCity = name;
	}

	inline void SetCityType(CITYTYPE c)
	{
		m_CityType = c;
	}

	inline void SetFacilities(const AuCityIndustryHash& facil)
	{
		m_Facilities = facil;
	}

	inline void SetFounder(const AuId& id, const QString& name)
	{
		m_FounderId = id;
		m_FounderName = name;
	}

	inline void SetGalaxy(GALAXY g)
	{
		m_Galaxy = g;
	}

	inline void SetGalaxyCapital(bool is)
	{
		m_GalaxyCapital = is;
	}

	inline void SetGalaxyId(const AuId& id)
	{
		m_idGalaxy = id;
	}

	inline void SetMorale(qint8 morale)
	{
		m_Morale = morale;
	}

	inline void SetPlanetId(const AuId& id)
	{
		m_idPlanet = id;
	}

	inline void SetPlanetName(const QString& name)
	{
		m_sPlanet = name;
	}

	inline void SetSectorCapital(bool is)
	{
		m_SectorCapital = is;
	}

	inline void SetSectorId(const AuId& id)
	{
		m_idSector = id;
	}

	inline void SetSectorName(const QString& name)
	{
		m_sSector = name;
	}

	inline void SetSystemId(const AuId& id)
	{
		m_idSystem = id;
	}

	inline void SetSystemName(const QString& name)
	{
		m_sSystem = name;
	}

	inline const AuId& SystemId() const
	{
		return m_idSystem;
	}

	inline const QString& SystemName() const
	{
		return m_sSystem;
	}

	inline const AuPoint& SystemLoc() const
	{
		return m_SystemLoc;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuCity& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuCity& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCityHash : public unordered_map<AuId, AuCity, AuIdHash>
{
public:	// Constructors
	AuCityHash()
	{
	}

public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	bool AnyInSector(const AuId& sectorId) const;

	bool AnyInSystem(const AuId& systemId) const;

	// Returns the id of my military base in 'systemId'.
	AuId BaseInSystem(const AuId& systemId) const;

	// Returns the id of any system in 'sectorId' that contains a military base.
	AuId BaseSystemInSector(const AuId& sectorId) const;

	quint32 CitiesInGalaxy(const AuId& galaxy) const;

	// Returns id of a city on 'worldId', if any.
	AuId CityOnWorld(const AuId& worldId) const;

	// Returns id of the best joinable city for a new avatar entering the game.
	AuId Joinable(const AuId& empireCapitol) const;

	quint64 MemoryUsage() const;

	// Returns the id of a randomly selected city in the zone.
	// City id must not be in 'exceptIds'.
	AuId Random(const QSet<AuId>& exceptIds) const;

	// Returns true if any changes are made.
	bool Reconcile(quint32 empireId, const unordered_map<AuId, quint32, AuIdHash>& allCities);

	quint32 SolarSystemsInGalaxy(const AuId& galaxy) const;
};

inline QDataStream& operator<<(QDataStream& out, const AuCityHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuCityHash& o)
{
	o << in;
	return in;
}

#endif // AuCity_H
