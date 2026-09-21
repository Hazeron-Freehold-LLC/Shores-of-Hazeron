//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austarmapdata.h,v 1.45 2022/04/15 16:36:46 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuStarMapData_H
#define AuStarMapData_H

#include "audef.h"
#include "auelapsedtimer.h"
#include "augloballib.h"
#include "auhash.h"
#include "aupoint.h"
#include <QDataStream>
#include <QString>

using namespace std;
#include <unordered_map>

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Filter used when searching for solar systems in a star map.

class AUGLOBAL_DLL AuStarMapSystemSearch
{
public:
	// System Name
	QString								m_Name;					// Search system name text. Blank = no name search
	Qt::CaseSensitivity					m_CaseSense;
	bool								m_RegEx;				// True to treat m_Name as a regular expression for matching.
	bool								m_SystemCat;			// True to check catalog name
	bool								m_SystemName;			// True to check system name.

	// Search Location
	double								m_Range;				// Range of search, in pc. 0 = no location search.
	quint8								m_Galaxy;				// GALAXY enum for range search.
	AuPoint								m_SearchLoc;			// Galaxy location of search center, in pc.

	// Resources
	QList<int>							m_NaturalResources;		// System resources. Value = COMMODITYID. Empty = no resource check.
	bool								m_AndOr;				// false = AND = all >= MinQ, true = OR = one >= MinQ
	quint8								m_MinQ;					// Minimum Q required of m_NaturalResources, depending on m_AndOr.

	// Stars
	quint8								m_Stars;				// Minimum number of stars. 0 = no star check
	bool								m_BlackHole;			// True if a black hole must exist.
	bool								m_NeutronStar;			// True if a neutron star must exist.
	bool								m_StarTypeO;			// True if a type O star must exist.
	bool								m_StarTypeB;			// True if a type B star must exist.
	bool								m_StarTypeA;			// True if a type A star must exist.
	bool								m_StarTypeF;			// True if a type F star must exist.
	bool								m_StarTypeG;			// True if a type G star must exist.
	bool								m_StarTypeK;			// True if a type K star must exist.
	bool								m_StarTypeM;			// True if a type M star must exist.

	// Worlds
	quint8								m_Worlds;				// Minimum number of worlds, including moons. 0 = no world checks.
	// Gas giant tests are or'd. Any one of them true makes the gas giant test true.
	bool								m_GasGiantInferno;		// True if a gas giant must exist in the inferno zone.
	bool								m_GasGiantInner;		// True if a gas giant must exist in the inner zone.
	bool								m_GasGiantHabitable;	// True if a gas giant must exist in the habitable zone.
	bool								m_GasGiantOuter;		// True if a gas giant must exist in the outer zone.
	bool								m_GasGiantFrigid;		// True if a gas giant must exist in the frigid zone.
	// Globe minimum size tests are or'd. Any one of them true makes the globe test true.
	quint8								m_GlobeInferno;			// Minimum size globe that must exist in the inferno zone.
	quint8								m_GlobeInner;			// Minimum size globe that must exist in the inner zone.
	quint8								m_GlobeHabitable;		// Minimum size globe that must exist in the habitable zone.
	quint8								m_GlobeOuter;			// Minimum size globe that must exist in the outer zone.
	quint8								m_GlobeFrigid;			// Minimum size globe that must exist in the frigid zone.
	// Ring tests are or'd. Any one of them true makes the ring test true.
	bool								m_RingInferno;			// True if a ring must exist in the inferno zone.
	bool								m_RingInner;			// True if a ring must exist in the inner zone.
	bool								m_RingHabitable;		// True if a ring must exist in the habitable zone.
	bool								m_RingOuter;			// True if a ring must exist in the outer zone.
	bool								m_RingFrigid;			// True if a ring must exist in the frigid zone.
	quint8								m_RingworldArcs;		// Minimum number of ringworld arcs.

public:	// Construction
	AuStarMapSystemSearch();

public:	// Operators
	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:	// Methods
	// Test the part of this filter that can be tested without the need for solar system scan information.
	// 'systemName' is the DaSolarSystem::Name().
	// 'catalogName' is the DaSolarSystem::CatalogName().
	// 'galaxy' is the GALAXY enum that contains 'systemLoc'.
	// 'systemLoc' is the galaxy location of the solar system, in pc, relative to 'galaxy'.
	bool PreScanTest(const QString& systemName, const QString& catalogName, int galaxy, const AuPoint& systemLoc) const;

	// Returns true if a system scan is needed to finish evaluating this search criteria.
	bool SystemScanNeeded() const;
};

Q_DECLARE_METATYPE(AuStarMapSystemSearch)	// Needed to use in queued signal/slot connections. Must also call qRegisterMetaType.

inline QDataStream& operator<<(QDataStream& out, const AuStarMapSystemSearch& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuStarMapSystemSearch& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuStarMapWormhole
{
private:
	quint8								m_ExploredPolarity;		// b7=explored, bottom nibble is AuWormhole::POLARITY
	AuPoint								m_VecToDest;			// AuWormhole::SectorVecToDestination.

public:
	inline AuStarMapWormhole(bool isExplored, int polarity, const AuPoint& sectorVecToDest)
	{
		m_ExploredPolarity = polarity;
		if(isExplored)
			m_ExploredPolarity |= 0x80;
		m_VecToDest = sectorVecToDest;
	}

	inline AuStarMapWormhole()
	{
		m_ExploredPolarity = 0;
	}

	inline AuStarMapWormhole(const AuStarMapWormhole& c)
	{
		m_ExploredPolarity = c.m_ExploredPolarity;
		m_VecToDest = c.m_VecToDest;
	}

	inline AuStarMapWormhole& operator=(const AuStarMapWormhole& c)
	{
		m_ExploredPolarity = c.m_ExploredPolarity;
		m_VecToDest = c.m_VecToDest;
		return *this;
	}

	inline bool IsExplored() const
	{
		return m_ExploredPolarity & 0x80;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_ExploredPolarity;
		out << (float)m_VecToDest.x;
		out << (float)m_VecToDest.y;
		out << (float)m_VecToDest.z;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_ExploredPolarity;
		float x,y,z;
		in >> x;
		in >> y;
		in >> z;
		m_VecToDest = AuPoint(x,y,z);
	}

	inline int Polarity() const
	{
		return m_ExploredPolarity & 0x7f;
	}

	inline const AuPoint& VecToDest() const
	{
		return m_VecToDest;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuStarMapCity
{
public:
	quint8								m_Info;					// b0=sector capital, b1=empire capital, b2=airport, b3=airbase,
																// b4=capitol building, b5=base hq, b6=galaxy capital
	quint32								m_Empire;				// empire id who owns the city
	QString								m_Name;

public:	// Constructors
	inline AuStarMapCity()
	{
		m_Info = 0;
		m_Empire = 0;
	}

	inline AuStarMapCity(const AuStarMapCity& c)
	{
		m_Info = c.m_Info;
		m_Empire = c.m_Empire;
		m_Name = c.m_Name;
	}

public:	// Operators
	inline AuStarMapCity& operator=(const AuStarMapCity& c)
	{
		m_Info = c.m_Info;
		m_Empire = c.m_Empire;
		m_Name = c.m_Name;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_Info;
		out << m_Empire;
		out << m_Name;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_Info;
		in >> m_Empire;
		in >> m_Name;
	}

public:	// Methods
	inline bool HasAirBase() const
	{
		return m_Info & 0x08;
	}

	inline bool HasAirport() const
	{
		return m_Info & 0x04;
	}

	inline bool HasBaseHeadquarters() const
	{
		return m_Info & 0x20;
	}

	inline bool HasCapitolBuilding() const
	{
		return m_Info & 0x10;
	}

	inline bool HasEmpireCapital() const
	{
		return m_Info & 0x02;
	}

	inline bool HasGalaxyCapital() const
	{
		return m_Info & 0x40;
	}

	inline bool HasSectorCapital() const
	{
		return m_Info & 0x01;
	}

	void ListEmpires(QSet<quint32>& empiresInMap) const;

	inline void SetAirBase()
	{
		m_Info |= 0x08;
	}

	inline void SetAirport()
	{
		m_Info |= 0x04;
	}

	inline void SetBaseHeadquarters()
	{
		m_Info |= 0x20;
	}

	inline void SetCapitolBuilding()
	{
		m_Info |= 0x10;
	}

	inline void SetEmpireCapital()
	{
		m_Info |= 0x02;
	}

	inline void SetGalaxyCapital()
	{
		m_Info |= 0x40;
	}

	inline void SetSectorCapital()
	{
		m_Info |= 0x01;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuStarMapSpacecraft
{
	enum INFOBITS
	{
		INFO_Station						= 0x0001,		// Spacecraft is a station.
		INFO_InAtmosphere					= 0x0002,		// Spacecraft is in the atmosphere zone of a world.
		INFO_Orbiting						= 0x0004,		// Spacecraft has a parent world.
		INFO_Moving							= 0x0008,		// Spacecraft is moving.
		INFO_System							= 0x0010,		// Spacecraft has a parent sun, is not in deep space.
		INFO_New							= 0x0020,		// Spacecraft is new.
		INFO_Captain						= 0x0040,		// Has an avatar captain.
		INFO_Officer						= 0x0080,		// Has an officer on the roster.
		INFO_AlertConditionMask				= 0x0300,		// ALERT_CONDITION bits.
		INFO_WarpFTLHot						= 0x0400,		// FTL drive is hot.
		INFO_WormholeFTLHot					= 0x0800,		// FTL drive is hot.
		INFO_WarpDrive						= 0x1000,		// Has warp FTL drive.
		INFO_WormholeDrive					= 0x2000,		// Has wormhole FTL drive.
		INFO_Sensors						= 0x4000,		// Spacecraft has functioning sensors.
		INFO_Company						= 0x8000,		// Company ship.
		INFO_FTLDriveMask					= 0x3000,		// FTL drive bits.
	};

public:
	quint8								m_Health;				// Scaled into the range 0 to 255.
	quint16								m_Info;					// INFOBITS bitmask
	quint32								m_Empire;				// empire id who owns the spacecraft
private:
	QString								m_Name;					// May include ">AboardSpacecraft" or "<AtBuilding"

public:	// Constructors
	inline AuStarMapSpacecraft(const QString& name, quint32 empire)
	{
		m_Empire = empire;
		m_Health = 0;
		m_Info = 0;
		m_Name = name;
	}

	inline AuStarMapSpacecraft()
	{
		m_Empire = 0;
		m_Health = 0;
		m_Info = 0;
	}

	inline AuStarMapSpacecraft(const AuStarMapSpacecraft& c)
	{
		m_Empire = c.m_Empire;
		m_Health = c.m_Health;
		m_Info = c.m_Info;
		m_Name = c.m_Name;
	}

public:	// Operators
	inline AuStarMapSpacecraft& operator=(const AuStarMapSpacecraft& c)
	{
		m_Empire = c.m_Empire;
		m_Health = c.m_Health;
		m_Info = c.m_Info;
		m_Name = c.m_Name;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_Empire;
		out << m_Info;
		out << m_Health;
		out << m_Name;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_Empire;
		in >> m_Info;
		in >> m_Health;
		in >> m_Name;
	}

public:	// Methods
	// Returns name of spacecraft or building that I am aboard.
	QString AboardName() const;

	inline ALERT_CONDITION AlertCondition() const
	{
		return (ALERT_CONDITION)((m_Info & INFO_AlertConditionMask) >> 8);
	}

	inline quint32 EmpireId() const
	{
		return m_Empire;
	}

	inline bool HasCaptain() const
	{
		return m_Info & INFO_Captain;
	}

	inline bool HasFTLDrive() const
	{
		return m_Info & INFO_FTLDriveMask;
	}

	inline bool HasOfficer() const
	{
		return m_Info & INFO_Officer;
	}

	inline bool HasSensors() const
	{
		return m_Info & INFO_Sensors;
	}

	inline bool HasWarpDrive() const
	{
		return m_Info & INFO_WarpDrive;
	}

	inline bool HasWormholeDrive() const
	{
		return m_Info & INFO_WormholeDrive;
	}

	// Returns health in the range 0 to 255.
	inline unsigned int Health() const
	{
		return m_Health;
	}

	bool IsAboardSpacecraft() const;

	bool IsAtBuilding() const;

	inline bool IsCompany() const
	{
		return m_Info & INFO_Company;
	}

	inline bool IsInAtmosphere() const
	{
		return m_Info & INFO_InAtmosphere;
	}

	inline bool IsInSystem() const
	{
		return m_Info & INFO_System;
	}

	inline bool IsMoving() const
	{
		return m_Info & INFO_Moving;
	}

	inline bool IsNew() const
	{
		return m_Info & INFO_New;
	}

	inline bool IsOrbiting() const
	{
		return m_Info & INFO_Orbiting;
	}

	inline bool IsSpaceStation() const
	{
		return m_Info & INFO_Station;
	}

	inline bool IsWarpFTLHot() const
	{
		return m_Info & INFO_WarpFTLHot;
	}

	inline bool IsWormholeFTLHot() const
	{
		return m_Info & INFO_WormholeFTLHot;
	}

	void ListEmpires(QSet<quint32>& empiresInMap) const;

	QString Name() const;

	inline void SetAlertCondition(ALERT_CONDITION alert)
	{
		int a = alert << 8;					// shift to the appropriate bit position.
		m_Info &= ~INFO_AlertConditionMask;	// zero the alert condition bits.
		m_Info |= a;						// turn on the health nibble bits we want.
	}

	void SetAtBuilding(const QString& cityOrBuildingName);

	void SetAboardSpacecraft(const QString& shipName);

	inline void SetCaptain()
	{
		m_Info |= INFO_Captain;
	}

	inline void SetCompany()
	{
		m_Info |= INFO_Company;
	}

	inline void SetHasSensors()
	{
		m_Info |= INFO_Sensors;
	}

	// 'health' must be in the range 0 to 255.
	inline void SetHealth(unsigned int health)
	{
		m_Health = health;
	}

	inline void SetInAtmosphere()
	{
		m_Info |= INFO_InAtmosphere;
	}

	inline void SetInSystem()
	{
		m_Info |= INFO_System;
	}

	inline void SetMoving()
	{
		m_Info |= INFO_Moving;
	}

	inline void SetNew()
	{
		m_Info |= INFO_New;
	}

	inline void SetOfficer()
	{
		m_Info |= INFO_Officer;
	}

	inline void SetOrbiting()
	{
		m_Info |= INFO_Orbiting;
	}

	inline void SetSpaceStation()
	{
		m_Info |= INFO_Station;
	}

	inline void SetWarpDrive()
	{
		m_Info |= INFO_WarpDrive;
	}

	inline void SetWormholeDrive()
	{
		m_Info |= INFO_WormholeDrive;
	}

	inline void SetWarpFTLHot()
	{
		m_Info |= INFO_WarpFTLHot;
	}

	inline void SetWormholeFTLHot()
	{
		m_Info |= INFO_WormholeFTLHot;
	}

	inline SPACECRAFT_STRUCTURE Structure() const
	{
		if(IsSpaceStation())
			return SC_Station;
		else if(HasFTLDrive())
			return SC_Starship;
		else
			return SC_Spaceship;
	}
};

Q_DECLARE_METATYPE(AuStarMapSpacecraft)	// Needed for queued signal/slot connections. Must also call qRegisterMetaType.

inline QDataStream& operator<<(QDataStream& out, const AuStarMapSpacecraft& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuStarMapSpacecraft& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuStarMapScPos : public AuStarMapSpacecraft
{
private:
	AuId								m_ScId;
	AuElapsedTimer						m_When;

public:
	inline AuStarMapScPos(const AuId& id, const AuStarMapSpacecraft& pos) : AuStarMapSpacecraft(pos)
	{
		m_ScId = id;
		m_When.start();
	}

	inline AuStarMapScPos()
	{
	}

	inline AuStarMapScPos(const AuStarMapScPos& c) : AuStarMapSpacecraft(c)
	{
		m_ScId = c.m_ScId;
		m_When = c.m_When;
	}

public:	// Operators
	inline AuStarMapScPos& operator=(const AuStarMapScPos& c)
	{
		AuStarMapSpacecraft::operator=(c);
		m_ScId = c.m_ScId;
		m_When = c.m_When;
		return *this;
	}

	/*
	inline void operator>>(QDataStream& out) const
	{
		AuStarMapSpacecraft::operator>>(out);
		out << m_ScId;
		out << m_When;
	}

	inline void operator<<(QDataStream& in)
	{
		AuStarMapSpacecraft::operator<<(in);
		in >> m_ScId;
		in >> m_When;
	}
	*/

public:	// Methods
	inline const AuId& Id() const
	{
		return m_ScId;
	}

	bool IsExpired() const;

	inline void SetPosition(const AuStarMapSpacecraft& pos)
	{
		AuStarMapSpacecraft::operator=(pos);
		m_When.restart();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuStarMapSystem
{
public:
	enum INFOBITS
	{
		Explored						= 0x01,				// System explored by querying empire.
		EmptySystem						= 0x02,				// System with no sun or worlds but could have ships.
		Scanned							= 0x04,				// System scanned by querying empire.
		CoreBlackHole					= 0x08,				// System is the core black hole system of its galaxy.
		EnergyDetectedMask				= 0xf0,				// Energy emission detected in unexplored system, only if !Explored.
	};

public:
	unordered_map<AuId, AuStarMapCity, AuIdHash>	m_Cities;
	quint8								m_GenHab;				// HABITABILITY enum
	quint32								m_GenSeed;				// System genseed
	quint8								m_InfoBits;				// INFOBITS bitmask
	AuPoint								m_Loc;
	QString								m_Name;
	unordered_map<AuId, AuStarMapSpacecraft, AuIdHash>	m_Spacecraft;
	QList<AuStarMapWormhole>			m_Wormholes;

public:
	inline AuStarMapSystem()
	{
		m_InfoBits = 0;
		m_GenHab =0;
		m_GenSeed = 0;
	}

	inline AuStarMapSystem(const AuStarMapSystem& c)
	{
		m_Cities = c.m_Cities;
		m_InfoBits = c.m_InfoBits;
		m_GenHab = c.m_GenHab;
		m_GenSeed = c.m_GenSeed;
		m_Loc = c.m_Loc;
		m_Name = c.m_Name;
		m_Spacecraft = c.m_Spacecraft;
		m_Wormholes = c.m_Wormholes;
	}

public:
	inline AuStarMapSystem& operator=(const AuStarMapSystem& c)
	{
		m_Cities = c.m_Cities;
		m_InfoBits = c.m_InfoBits;
		m_GenHab = c.m_GenHab;
		m_GenSeed = c.m_GenSeed;
		m_Loc = c.m_Loc;
		m_Name = c.m_Name;
		m_Spacecraft = c.m_Spacecraft;
		m_Wormholes = c.m_Wormholes;
		return *this;
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:
	// Returns 0 to 15.
	inline quint8 DetectedSignalStrength() const
	{
		return (m_InfoBits & EnergyDetectedMask) >> 4;
	}

	inline bool IsCoreBlackHole() const
	{
		return m_InfoBits & CoreBlackHole;
	}

	inline bool IsEmptySystem() const
	{
		return m_InfoBits & EmptySystem;
	}

	inline bool IsExplored() const
	{
		return m_InfoBits & Explored;
	}

	inline bool IsScanned() const
	{
		return m_InfoBits & Scanned;
	}

	void ListEmpires(QSet<quint32>& empiresInMap) const;

	void SetDetectedSignalStrength(quint32 signalStrength);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuStarMapSector
{
public:
	quint8								m_Galaxy;				// GALAXY enum value.
	AuPoint								m_Loc;
	QString								m_Name;
	unordered_map<AuId, AuStarMapSystem, AuIdHash>		m_Systems;

public:
	inline AuStarMapSector()
	{
		m_Galaxy = ShoresOfHazeron;
	}

	inline AuStarMapSector(const AuStarMapSector& c)
	{
		m_Galaxy = c.m_Galaxy;
		m_Loc = c.m_Loc;
		m_Name = c.m_Name;
		m_Systems = c.m_Systems;
	}

	inline AuStarMapSector& operator=(const AuStarMapSector& c)
	{
		m_Galaxy = c.m_Galaxy;
		m_Loc = c.m_Loc;
		m_Name = c.m_Name;
		m_Systems = c.m_Systems;
		return *this;
	}

	inline GALAXY Galaxy() const
	{
		return (GALAXY)m_Galaxy;
	}

	void ListEmpires(QSet<quint32>& empiresInMap) const;

	inline void operator>>(QDataStream& out) const
	{
		out << m_Galaxy;

		out << (float)m_Loc.x;
		out << (float)m_Loc.y;
		out << (float)m_Loc.z;
		out << m_Name;

		out << (quint16)m_Systems.size();
		for(unordered_map<AuId, AuStarMapSystem, AuIdHash>::const_iterator si = m_Systems.begin(); si != m_Systems.end(); si++)
		{
			out << (*si).first;
			(*si).second >> out;
		}
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_Galaxy;

		float x,y,z;
		in >> x;
		in >> y;
		in >> z;
		m_Loc = AuPoint(x,y,z);
		in >> m_Name;

		m_Systems.clear();
		quint16 sysCount;
		in >> sysCount;
		while(sysCount--)
		{
			AuId systemId;
			in >> systemId;
			AuStarMapSystem gvsystem;
			gvsystem << in;
			m_Systems.insert(pair<AuId, AuStarMapSystem>(systemId, gvsystem));
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL AuStarMapData
{
public:
	unordered_map<AuId, AuStarMapSector, AuIdHash>	m_Sectors;

public:
	inline AuStarMapData()
	{
	}

	inline AuStarMapData(const AuStarMapData& c)
	{
		m_Sectors = c.m_Sectors;
	}

	inline AuStarMapData& operator=(const AuStarMapData& c)
	{
		m_Sectors = c.m_Sectors;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << (quint16)m_Sectors.size();
		for(unordered_map<AuId, AuStarMapSector, AuIdHash>::const_iterator si = m_Sectors.begin(); si != m_Sectors.end(); si++)
		{
			out << (*si).first;
			(*si).second >> out;
		}
	}

	inline void operator<<(QDataStream& in)
	{
		m_Sectors.clear();
		quint16 sectorCount;
		in >> sectorCount;
		while(sectorCount--)
		{
			AuId sectorId;
			in >> sectorId;
			AuStarMapSector gvsector;
			gvsector << in;
			m_Sectors.insert(pair<AuId, AuStarMapSector>(sectorId, gvsector));
		}
	}

	void ListEmpires(QSet<quint32>& empiresInMap) const;

	QString SectorNameOfSystem(const AuId& systemId) const;

	AuId SectorOfSystem(const AuId& systemId) const;

	// 'wormDestLoc' is a galaxy-relative coordinate in parsecs(pc).
	// Returns true if the destination system is found.
	bool WormholeDest(GALAXY destGalaxy, const AuPoint& wormDestLoc, QString& destSector, QString& destSystem) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuStarMapData_H
