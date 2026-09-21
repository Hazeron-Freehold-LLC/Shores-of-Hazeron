//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucityreport.h,v 1.75 2022/05/04 20:27:15 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCityReport_H
#define AuCityReport_H

#include "aubank.h"
#include "audata.h"
#include "audnabase.h"
#include "auglobell.h"
#include "auid.h"
#include "auknownpatents.h"
#include "autime.h"
#include "dbindustry.h"

#define d_AbandonDaysBegin				(2)						// Days since last visit, after which abandonment begins.
#define d_CityReportInterval			((RealDaysPerUYear() * SecondsPerDay() / 365) / 4)	// Seconds in each city report period.

class AuEventLog;
class AuItemHash;
class AuSpecies;

class AUDATA_DLL AuCityIndustry
{
private:
	quint32		m_LevelsInUse;	// Number of active levels for manufacturing buildings.
	quint32		m_Levels;		// Number of levels of this type of industry.

public:	// Constructors
	inline AuCityIndustry(quint32 levels, quint32 levelsInUse)
	{
		m_Levels = levels;
		m_LevelsInUse = levelsInUse;
	}

	inline AuCityIndustry()
	{
		m_Levels = 0;
		m_LevelsInUse = 0;
	}

	inline AuCityIndustry(const AuCityIndustry& c)
	{
		m_Levels = c.m_Levels;
		m_LevelsInUse = c.m_LevelsInUse;
	}

public:	// Operators
	inline const AuCityIndustry& operator=(const AuCityIndustry& c)
	{
		m_Levels = c.m_Levels;
		m_LevelsInUse = c.m_LevelsInUse;
		return *this;
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	inline bool operator==(const AuCityIndustry& c) const
	{
		if(m_Levels != c.m_Levels)
			return false;
		if(m_LevelsInUse != c.m_LevelsInUse)
			return false;
		return true;
	}

	inline bool operator!=(const AuCityIndustry& c) const
	{
		return !operator==(c);
	}

public:	// Methods
	inline void Add(const AuCityIndustry& c)
	{
		if(c.Levels())
		{
			m_Levels += c.Levels();
			m_LevelsInUse += c.LevelsInUse();
		}
	}

	inline quint32 Levels() const
	{
		return m_Levels;
	}

	inline quint32 LevelsInUse() const
	{
		return m_LevelsInUse;
	}
};

Q_DECLARE_METATYPE(AuCityIndustry)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

inline QDataStream& operator<<(QDataStream& out, const AuCityIndustry& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuCityIndustry& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCityIndustryHash : public QHash<INDUSTRYID, AuCityIndustry>
{
public:	// Constructors
	AuCityIndustryHash()
	{
	}

public:	// Operators
	inline void operator>>(QDataStream& out) const
	{
		out << (quint8)size();
		for(const_iterator ii = cbegin(); ii != cend(); ii++)
		{
			out << (quint8)ii.key();
			out << *ii;
		}
	}

	inline void operator<<(QDataStream& in)
	{
		clear();
		quint8 s;
		in >> s;
		while(s--)
		{
			quint8 iid;
			AuCityIndustry ci;
			in >> iid;
			in >> ci;
			insert((INDUSTRYID)iid, ci);
		}
	}

public:	// Methods
	inline void Add(INDUSTRYID iid, const AuCityIndustry& c)
	{
		iterator ii = find(iid);
		if(ii == end())
			insert(iid, c);				// First industry of its kind.
		else
			(*ii).Add(c);				// Industry of a kind we have already tallied.
	}

	inline bool HasAirBase() const
	{
		return Levels(IID_BaseAirTerminal);
	}

	inline bool HasAirport() const
	{
		return Levels(IID_AirportTerminal);
	}

	inline bool HasAirportRadar() const
	{
		return Levels(IID_AirportRadar);
	}

	inline bool HasAirportRepairShop() const
	{
		return Levels(IID_AirportRepairShop);
	}

	inline bool HasBase() const
	{
		return Levels(IID_BaseWeapon) ||
			Levels(IID_BaseShield);
	}

	inline bool HasBaseHQ() const
	{
		return Levels(IID_BaseHeadquarters);
	}

	inline bool HasBaseRadar() const
	{
		return Levels(IID_BaseRadar);
	}

	inline bool HasCapitol() const
	{
		return Levels(IID_Capitol);
	}

	inline bool HasSeaport() const
	{
		return Levels(IID_Wharf);
	}

	inline bool HasSensors() const
	{
		return Levels(IID_AirportTerminal) ||
			Levels(IID_BaseWeapon) ||
			Levels(IID_BaseShield);
	}

	inline bool HasSpacecraftFactory() const
	{
		return Levels(IID_SpacecraftFactory);
	}

	inline bool HasTownSquare() const
	{
		return Levels(IID_TownSquare);
	}

	inline quint32 Levels(INDUSTRYID ind) const
	{
		const_iterator ii = find(ind);
		if(ii == cend())
			return 0;
		return (*ii).Levels();
	}

	quint64 MemoryUsage() const;

	quint32 TotalLevels() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuCityIndustryHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuCityIndustryHash& o)
{
	o << in;
	return in;
}

Q_DECLARE_METATYPE(AuCityIndustryHash)	// Needed for queued signal/slot connections. Must also call qRegisterMetaType.

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCityIndustryIID : public AuCityIndustry
{
private:
	INDUSTRYID		m_IID;

public:
	AuCityIndustryIID(const AuCityIndustry& c, INDUSTRYID iid) : AuCityIndustry(c)
	{
		m_IID = iid;
	}

	AuCityIndustryIID(const AuCityIndustryIID& c) : AuCityIndustry(c)
	{
		m_IID = c.m_IID;
	}

	const AuCityIndustryIID& operator=(const AuCityIndustryIID& c)
	{
		AuCityIndustry::operator=(c);
		m_IID = c.m_IID;
		return *this;
	}

	inline INDUSTRYID IndustryId() const
	{
		return m_IID;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#define MaxFlagLevel	30

class AuFlag
{
public:
	AuTime					m_Capture;	// Time last captured. Used to stop exploit of inc reputation.
	AuGlobeLLF				m_DevLL;	// Longitude/latitude id on DaWorldLL.
	quint32					m_Empire;	// The DgEmpire id of this flag.
	INDUSTRYID				m_IID;		// Industry id for reporting occupation. Not stored in db.
	quint8					m_Level;	// Flag transition level.
	AuId					m_Terrain;	// World id on DaWorldLL.

public:
	AuFlag()
	{
		m_Empire = 0;
		m_Level = MaxFlagLevel;
		m_IID = IID_TownSquare;
	}

	AuFlag(const AuFlag& c)
	{
		m_Capture = c.m_Capture;
		m_DevLL = c.m_DevLL;
		m_Empire = c.m_Empire;
		m_IID = c.m_IID;
		m_Level = c.m_Level;
		m_Terrain = c.m_Terrain;
	}

	const AuFlag& operator=(const AuFlag& c)
	{
		m_Capture = c.m_Capture;
		m_DevLL = c.m_DevLL;
		m_Empire = c.m_Empire;
		m_IID = c.m_IID;
		m_Level = c.m_Level;
		m_Terrain = c.m_Terrain;
		
		return *this;
	}

	inline bool Is(const AuId& world, const AuGlobeLLF& devLL) const
	{
		return m_Terrain == world && DevLL() == devLL;
	}

	// Returns the DpBuilding::DevLL of the building.
	const AuGlobeLLF& DevLL() const
	{
		return m_DevLL;
	}

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;

	void SetGlobeLL(const AuGlobeLLF& ll)
	{
		m_DevLL = ll;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuFlagArray : public QVector<AuFlag>
{
public:
	// Returns the index of a flag given its terrain id and tile coordinate.
	// Returns -1 if not found.
	inline int IndexOf(const AuId& terrain, const AuGlobeLLF& ll) const
	{
		int index = 0;
		for(const_iterator it = constBegin(); it != constEnd(); it++, index++)
			if((*it).Is(terrain, ll))
				return index;
		return -1;	// not found
	}

	// Returns true if all flags are not owned by the same empire.
	inline bool IsContested() const
	{
		for(const_iterator it = constBegin() + 1; it != constEnd(); it++)
			if((*it).m_Empire != at(0).m_Empire)
				return true;
		return false;
	}

	const AuFlagArray& operator=(const AuFlagArray& c)
	{
		clear();
		for(const_iterator it = c.constBegin(); it != c.constEnd(); it++)
			append(*it);
		return *this;
	}

	void AUDATA_DLL operator<<(QDataStream& in);

	void AUDATA_DLL operator>>(QDataStream& out) const;
};

inline QDataStream& operator<<(QDataStream& out, const AuFlagArray& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuFlagArray& c)
{
	c << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuCityReportShip
{
public:
	bool								m_IsNew;
	bool								m_IsSpaceStation;
	quint8								m_Location;				// 0=orbit, 1=ground, 2=hangar
	AuGlobeLLF							m_ShipLL;
	QString								m_SpotDesc;

public:
	AuCityReportShip()
	{
		m_IsNew = false;
		m_IsSpaceStation = false;
		m_Location = 0;
	}

	AuCityReportShip(const AuCityReportShip& c)
	{
		m_IsNew = c.m_IsNew;
		m_IsSpaceStation = c.m_IsSpaceStation;
		m_Location = c.m_Location;
		m_ShipLL = c.m_ShipLL;
		m_SpotDesc = c.m_SpotDesc;
	}

	const AuCityReportShip& operator=(const AuCityReportShip& c)
	{
		m_IsNew = c.m_IsNew;
		m_IsSpaceStation = c.m_IsSpaceStation;
		m_Location = c.m_Location;
		m_ShipLL = c.m_ShipLL;
		m_SpotDesc = c.m_SpotDesc;

		return *this;
	}

	void operator>>(QDataStream& out) const
	{
		quint8 bits = 0x10;
		if(m_IsNew)
			bits |= 0x01;
		if(m_Location == 1)
			bits |= 0x02;
		else if(m_Location == 2)
			bits |= 0x08;
		if(m_IsSpaceStation)
			bits |= 0x04;
		out << bits;

		out << m_ShipLL;

		out << m_SpotDesc;
	}

	void operator<<(QDataStream& in)
	{
		quint8 bits;

		in >> bits;

		m_IsNew = (bits & 0x01);

		if(bits & 0x02)
			m_Location = 1;
		else if(bits & 0x08)
			m_Location = 2;

		m_IsSpaceStation = (bits & 0x04);

		if(bits & 0x10)
			in >> m_ShipLL;

		in >> m_SpotDesc;
	}

	quint64 MemoryUsage() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuCityReportShip& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuCityReportShip& c)
{
	c << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuCityCitizen
{
	ECOROLE								m_EcoRole;
	bool								m_IsAlien;				// True if not sexually compatible with indigenous.
	quint32								m_Population;			// Population of this kind of citizen.
	QString								m_SpeciesName;
	QString								m_SpeciesDesc;

public:	// Construction
	inline AuCityCitizen(const QString& speciesName, const QString& speciesDesc, ECOROLE ecoRole, bool isAlien,
			quint32 population)
	{
		m_EcoRole = ecoRole;
		m_IsAlien = isAlien;
		m_Population = population;
		m_SpeciesDesc = speciesDesc;
		m_SpeciesName = speciesName;
	}

	AuCityCitizen(const AuSpecies& species, ECOROLE ecoRole, bool isAlien, quint32 population);

	inline AuCityCitizen()
	{
		m_EcoRole = ECO_Omnivore;
		m_IsAlien = false;
		m_Population = 0;
	}

	inline AuCityCitizen(const AuCityCitizen& c)
	{
		m_EcoRole = c.m_EcoRole;
		m_IsAlien = c.m_IsAlien;
		m_Population = c.m_Population;
		m_SpeciesName = c.m_SpeciesName;
		m_SpeciesDesc = c.m_SpeciesDesc;
	}

public:	// Operators
	inline const AuCityCitizen& operator=(const AuCityCitizen& c)
	{
		m_EcoRole = c.m_EcoRole;
		m_IsAlien = c.m_IsAlien;
		m_Population = c.m_Population;
		m_SpeciesName = c.m_SpeciesName;
		m_SpeciesDesc = c.m_SpeciesDesc;
		return *this;
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	inline void AddPopulation(quint32 p)
	{
		m_Population += p;
	}

	inline ECOROLE EcoRole() const
	{
		return m_EcoRole;
	}

	inline bool IsAlien() const
	{
		return m_IsAlien;
	}

	inline const QString& SpeciesDesc() const
	{
		return m_SpeciesDesc;
	}

	inline const QString& SpeciesName() const
	{
		return m_SpeciesName;
	}

	inline quint32 Population() const
	{
		return m_Population;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuCityCitizen& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuCityCitizen& c)
{
	c << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCityReport
{
public:
	qint32								m_AirConsumed;			// -1 = unknown i.e. intermediate report. >= 0 = city cycle report.
	quint32								m_AirSupply;			// At start of report. Not after air removed from inventory.
	bool								m_AteEnoughFood;
	AuBank								m_Bank;					// Bank account balance and activity.
	QString								m_CaptName;
	QMap<AuSpecies, AuCityCitizen>		m_Citizens;
	quint8								m_CityType;				// CITYTYPE of this report.
	bool								m_ConsumedLiquor;
	INDUSTRYID							m_Decayed[3];			// Up to 3 industries can decay in a single city report.
	QString								m_DistressRpt;			// Accumulates reasons for distress.
	AuEventLog*							m_EventLog;				// For a summary of noteworthy events in each city report.
	AuFlagArray							m_Flags;				// First one is always town square. A flag is added for each base.
	quint32								m_FoodPerMeal;			// Per citizen.
	qint8								m_FoodTypes;			// -1 = unknown
	quint32								m_FoodUnits;			// At start of report. Not after food removed from inventory.
	AuId								m_FounderId;
	QString								m_FounderName;
	FOUNDER_STATUS						m_FounderStatus;
	quint8								m_Galaxy;
	bool								m_HasBreathableAtmo;
	quint8								m_HasDestroyedBuildings;	// b0=heavily damaged, b1=burning
	quint32								m_iCapacitor;			// Storage limit for electricity.
	AuCityIndustryHash					m_Industries;
	AuItemHash*							m_Inventory;			// AuItems held in the inventory of this city.
	bool 								m_IsCapitalOfEmpire;	// Updated at city report. Not stored in db.
	bool 								m_IsCapitalOfGalaxy;	// Updated at city report. Not stored in db.
	bool 								m_IsCapitalOfSector;	// Updated at city report. Not stored in db.
	bool								m_IsHabitablePlanet;
	bool								m_IsIsolated;
	bool								m_IsOnRingworld;
	quint32								m_iStorage;				// Storage limit per commodity.
	qint32								m_Jobs;					// Count of all jobs, including construction. Bd=manufacturing jobs
	qint32								m_JobsNonConst;			// Count of all non-construction jobs. Bd=construction jobs
	qint32								m_JobsRepair;			// Count of all building repair jobs. Bd=service jobs
	quint16								m_LastAvatarAccountActive;	// days since pinged by active avatar with a home in my sector
	double								m_LastTribute;
	quint16								m_LastVisit;			// days since system visited by a person on line
	AuGlobeLL							m_LL;
	float								m_LoyaltyOrig;
	quint32								m_MaxPop;
	quint8								m_MessageType;			// AuMailMsg::MSGTYPE
	QString								m_Name;					// HtmlName

	QString								m_NativeSpecies;		// Name of the indigenous species.
	QString								m_NativeSpeciesDesc;	// Description of the indigenous species.

	AuKnownPatents						m_Patents;
	QString								m_PlanetDesc;
	QString								m_PlanetName;
	quint32								m_PopOrig;
	quint32								m_Power;				// Currently in capacitor
	QMap<QString, quint16>				m_PatentRnD;
	qint8								m_SatOrig;
	QString								m_SectorName;
	QString								m_SectorDesc;
	QList<AuCityReportShip>				m_Ships;
	QString								m_SystemName;
	QString								m_SystemDesc;
	AuItemHash*							m_Tools;				// AuItems used for manufacturing and construction
	quint16								m_TroopsAtCity;
	quint8								m_TroopsDrafted;
	qint32								m_Unemployed;			// Count of unemployed citizens. -1 is unset.
	QList<COMMODITYID>					m_Vehicles;				// Vehicles in the city. One entry per vehicle to show presence.
	AuTime								m_WhenCompiled;
	quint8								m_Zone;					// Resource zone.
	quint32								m_ZonePop;				// World pop for DaBd.

	// Added for DaBd
	quint32								m_HomesLg;
	quint32								m_HomesMd;
	quint32								m_HomesSm;
	qint32								m_JobsPro;				// Count of professional jobs.
	quint32								m_Lounges;
	quint8								m_PopBirths;
	quint8								m_PopDeaths;
	quint8								m_PopIndigenousArr;
	quint8								m_PopIndigenousDep;
	quint8								m_PopSeafarersArr;
	quint8								m_PopSeafarersDep;
	quint8								m_PopTravellersArr;
	quint8								m_PopTravellersDep;
	quint8								m_PopWanderersArr;
	quint8								m_PopWanderersDep;
	quint32								m_StoresGrocery;
	quint32								m_StoresRetail;
	quint32								m_WorldBuildingCount;	// Total buildings on the world.
	quint32								m_WorldBuildingVol;		// Total building volume on the world.

public:	// Constructors
	AuCityReport();

	~AuCityReport();

	AuCityReport(const AuCityReport& c);

public:	// Operators
	const AuCityReport& operator=(const AuCityReport& c);

	inline bool operator==(const AuCityReport& c) const
	{
		// Only Windows seems to need this function to compile. I don't think I use it anywhere.
		return m_WhenCompiled == c.m_WhenCompiled && m_Name == c.m_Name;
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	// Returns morale modifier due to abandonment, plus or minus. May be 0 even if IsAbandoned returns true.
	int AbandonMod() const;

	void AddResidentSpecies(const AuSpecies& species, bool isAlien, quint32 residents);

	ALERT_CONDITION AlertCondition() const;

	inline int BuildingLevels(INDUSTRYID iid) const
	{
		return m_Industries.Levels(iid);
	}

	// Return worker DNA for this entire city.
	AuCityCitizen Citizen() const;

	// flagIndex 0 is always the town square empire id, which is the owner of
	// the city regardless of the ownership of the base flags, which will
	// occupy indices 1..n.
	inline quint32 EmpireId(int flagIndex) const
	{
		if(flagIndex >= 0 && flagIndex < m_Flags.count())
			return m_Flags.at(flagIndex).m_Empire;
		return 0;
	}

	bool HasAir() const;		// Return true if any air is in stock.

	inline bool HasAirport() const
	{
		return m_Industries.Levels(IID_AirportTerminal) ? true : false;
	}

	inline bool HasDecay() const
	{
		for(int di=0; di<3; di++)
			if(m_Decayed[di] != IID_None)
				return true;
		return false;
	}

	bool HasPower() const;		// Return true if any electricity is in stock.

	bool HasRadar() const;

	// Returns the total number of homes created by a given industry.
	inline int Homes(INDUSTRYID iid) const
	{
		switch(iid)
		{
		case IID_Apartment:
			return m_Industries.Levels(IID_Apartment) * HomesPerLevel(IID_Apartment);
		case IID_Farm:
			return m_Industries.Levels(IID_Farm) * HomesPerLevel(IID_Farm);
		case IID_Hotel:
			return m_Industries.Levels(IID_Hotel) * HomesPerLevel(IID_Hotel);
		case IID_House:
			return m_Industries.Levels(IID_House) * HomesPerLevel(IID_House);
		case IID_Orchard:
			return m_Industries.Levels(IID_Orchard) * HomesPerLevel(IID_Orchard);
		default:
			return 0;
		}
	}

	// Returns the total number of homes created by each level of an industry.
	static int HomesPerLevel(INDUSTRYID iid);

	// Returns true if this city is abandoned. May be true even if the AbandonMod is 0.
	bool IsAbandoned() const;

	quint64 MemoryUsage() const;

	// Eliminates bulky data from older reports.
	void MinimizeData();

	// Returns the result of accumulating all morale modifiers. Only used by Bd capitol and hq.
	qint8 Morale() const;

	// Returns the total population of all residents.
	quint32 Population() const;

	// Formats the data into an html formatted report.
	// 'prevReport' can be passed to show industry quality changes; null is ok.
	QString ReportHtml(const AuCityReport& prevReport, bool mailFormat) const;

	int RequiredArenas() const;

	int RequiredCantinas() const;

	int RequiredCasinos() const;

	int RequiredChurches() const;

	int RequiredGrocery() const;

	int RequiredHospitals() const;

	int RequiredParks() const;

	int RequiredRetail() const;

	int RequiredUniversities() const;

	int RequiredZoos() const;

	QString Title() const;

	// Returns the maximum volume of buildings allowed on the world, in cubic meters.
	// Limit scales linearly with world surface area, via m_MaxPop.
	quint32 WorldBuildingLimit() const;

	static quint32 WorldBuildingLimit(quint32 maxPopulation, bool isRingworld);

	// Returns the maximum number of buildings allowed on the world.
	static quint32 WorldBuildingCountLimit();

protected:
	qint8 MoraleCapitol() const;
	qint8 MoraleHQ() const;

	QString ReportCapitol(const AuCityReport& prevReport, bool mailFormat) const;
	QString ReportHQ(const AuCityReport& prevReport, bool mailFormat) const;
};

Q_DECLARE_METATYPE(AuCityReport)	// Needed for queued signal/slot connections. Must also call qRegisterMetaType.

inline QDataStream& operator<<(QDataStream& out, const AuCityReport& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuCityReport& c)
{
	c << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuCityReportHistory : protected QList<AuCityReport>
{
public:
	inline AuCityReportHistory()
	{
	}

	inline AuCityReportHistory(const AuCityReportHistory& c) : QList<AuCityReport>()
	{
		for(AuCityReportHistory::const_iterator ri = c.constBegin(); ri != c.constEnd(); ri++)
			append(*ri);
	}

	inline const AuCityReportHistory& operator=(const AuCityReportHistory& c)
	{
		for(AuCityReportHistory::const_iterator ri = c.constBegin(); ri != c.constEnd(); ri++)
			append(*ri);
		return *this;
	}

	void Append(const AuCityReport& report);

	inline const AuCityReport& At(int i) const
	{
		return at(i);
	}

	inline int Count() const
	{
		return count();
	}

	inline bool IsEmpty() const
	{
		return isEmpty();
	}

	inline const AuCityReport& Last() const
	{
		return last();
	}

	quint64 MemoryUsage() const;

	inline const AuCityReport& NextToLast() const
	{
		return *(constEnd() - 2);
	}

	inline void operator<<(QDataStream& in)
	{
		clear();
		quint16 count;
		in >> count;
		for(;count;count--)
		{
			AuCityReport report;
			in >> report;
			Append(report);
		}
	}

	inline void operator>>(QDataStream& out) const
	{
		out << (quint16)count();
		for(const_iterator di = constBegin(); di != constEnd(); di++)
			out << (*di);
	}
};

Q_DECLARE_METATYPE(AuCityReportHistory)	// Needed for queued signal/slot connections. Must also call qRegisterMetaType.

inline QDataStream& operator<<(QDataStream& out, const AuCityReportHistory& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuCityReportHistory& c)
{
	c << in;
	return in;
}

#endif // AuCityReport_H
