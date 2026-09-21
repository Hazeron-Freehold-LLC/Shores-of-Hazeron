//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auarenaready.h,v 1.9 2022/02/19 18:30:16 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuArenaReady_H
#define AuArenaReady_H

#include "augloballib.h"
#include "auid.h"
#include "autime.h"

class AUGLOBAL_DLL AuArenaReady
{
public:
	AuId								m_Id;
	quint8								m_MatchState;
	quint8								m_MatchType;
	quint8								m_MaximizeShipQ;
	quint8								m_MaxShipQ;
	QString								m_Name;
	quint8								m_OpponentDesignsOk;
	QString								m_Password;
	quint32								m_ShipDesign;
	QString								m_ShipDesignDesc;
	quint8								m_SpawnCrew;
	quint8								m_SpawnOfficer;
	quint8								m_SpawnPassengers;
	quint8								m_SpawnTroops;
	quint8								m_SpawnVehicles;
	quint8								m_TeamShips;
	quint8								m_Team1Ships;
	quint8								m_Team2Ships;
	AuTime								m_When;					// When last announced.

public:
	inline AuArenaReady()
	{
		m_MatchState = 0;
		m_MatchType = 0;
		m_MaximizeShipQ = 0;
		m_MaxShipQ = 1;
		m_OpponentDesignsOk = 0;
		m_ShipDesign = 0;
		m_SpawnCrew = 0;
		m_SpawnOfficer = 0;
		m_SpawnPassengers = 0;
		m_SpawnTroops = 0;
		m_SpawnVehicles = 0;
		m_TeamShips = 0;
		m_Team1Ships = 0;
		m_Team2Ships = 0;
	}

	inline AuArenaReady(const AuId& id, const QString& name, const QString& password, quint8 matchType, quint8 matchState,
			quint8 maxShipQ, quint8 maximizeShipQ, quint8 opponentDesignsOk, quint32 shipDesign, const QString& shipDesignDesc,
			quint8 spawnCrew, quint8 spawnOfficer, quint8 spawnPassengers, quint8 spawnTroops,
			quint8 spawnVehicles, quint8 teamShips, quint8 team1Ships, quint8 team2Ships, const AuTime& when)
	{
		Q_ASSERT(maxShipQ);
		m_Id = id;
		m_MatchState = matchState;
		m_MatchType = matchType;
		m_MaximizeShipQ = maximizeShipQ;
		m_MaxShipQ = maxShipQ;
		m_Name = name;
		m_OpponentDesignsOk = opponentDesignsOk;
		m_Password = password;
		m_ShipDesign = shipDesign;
		m_ShipDesignDesc = shipDesignDesc;
		m_SpawnCrew = spawnCrew;
		m_SpawnOfficer = spawnOfficer;
		m_SpawnPassengers = spawnPassengers;
		m_SpawnTroops = spawnTroops;
		m_SpawnVehicles = spawnVehicles;
		m_TeamShips = teamShips;
		m_Team1Ships = team1Ships;
		m_Team2Ships = team2Ships;
		m_When = when;
	}

	inline AuArenaReady(const AuArenaReady& c)
	{
		m_Id = c.m_Id;
		m_MatchState = c.m_MatchState;
		m_MatchType = c.m_MatchType;
		m_MaximizeShipQ = c.m_MaximizeShipQ;
		m_MaxShipQ = c.m_MaxShipQ;
		m_Name = c.m_Name;
		m_OpponentDesignsOk = c.m_OpponentDesignsOk;
		m_Password = c.m_Password;
		m_ShipDesign = c.m_ShipDesign;
		m_ShipDesignDesc = c.m_ShipDesignDesc;
		m_SpawnCrew = c.m_SpawnCrew;
		m_SpawnOfficer = c.m_SpawnOfficer;
		m_SpawnPassengers = c.m_SpawnPassengers;
		m_SpawnTroops = c.m_SpawnTroops;
		m_SpawnVehicles = c.m_SpawnVehicles;
		m_TeamShips = c.m_TeamShips;
		m_Team1Ships = c.m_Team1Ships;
		m_Team2Ships = c.m_Team2Ships;
		m_When = c.m_When;
	}

	inline const AuArenaReady& operator=(const AuArenaReady& c)
	{
		m_Id = c.m_Id;
		m_MatchState = c.m_MatchState;
		m_MatchType = c.m_MatchType;
		m_MaximizeShipQ = c.m_MaximizeShipQ;
		m_MaxShipQ = c.m_MaxShipQ;
		m_Name = c.m_Name;
		m_OpponentDesignsOk = c.m_OpponentDesignsOk;
		m_Password = c.m_Password;
		m_ShipDesign = c.m_ShipDesign;
		m_ShipDesignDesc = c.m_ShipDesignDesc;
		m_SpawnCrew = c.m_SpawnCrew;
		m_SpawnOfficer = c.m_SpawnOfficer;
		m_SpawnPassengers = c.m_SpawnPassengers;
		m_SpawnTroops = c.m_SpawnTroops;
		m_SpawnVehicles = c.m_SpawnVehicles;
		m_TeamShips = c.m_TeamShips;
		m_Team1Ships = c.m_Team1Ships;
		m_Team2Ships = c.m_Team2Ships;
		m_When = c.m_When;

		return *this;
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	// Arenas waiting for players send out their announcements during Tock, which is about every 10 seconds.
	// We don't want to keep these things around any longer than necessary.
	inline bool IsExpired() const
	{
		return m_When.ElapsedSec() > 15;
	}
};

Q_DECLARE_METATYPE(AuArenaReady)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

inline AUGLOBAL_DLL QDataStream& operator<<(QDataStream& out, const AuArenaReady& o)
{
	o >> out;
	return out;
}

inline AUGLOBAL_DLL QDataStream& operator>>(QDataStream& in, AuArenaReady& o)
{
	o << in;
	return in;
}

#endif // AuArenaReady_H
