//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auarenaready.cpp,v 1.5 2022/02/19 18:32:57 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auarenaready.h"
//

// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
// The Q_DECLARE_METATYPE macro is also required.
static int s_AuArenaReadyMetaType = qRegisterMetaType<AuArenaReady>();

void AuArenaReady::operator>>(QDataStream& out) const
{
	out << m_Id;
	out << m_MatchState;
	out << m_MatchType;
	out << m_MaximizeShipQ;
	out << m_MaxShipQ;
	out << m_Name;
	out << m_OpponentDesignsOk;
	out << m_Password;
	out << m_ShipDesign;
	out << m_ShipDesignDesc;
	out << m_SpawnCrew;
	out << m_SpawnOfficer;
	out << m_SpawnPassengers;
	out << m_SpawnTroops;
	out << m_SpawnVehicles;
	out << m_TeamShips;
	out << m_Team1Ships;
	out << m_Team2Ships;
	m_When >> out;
}

void AuArenaReady::operator<<(QDataStream& in)
{
	in >> m_Id;
	in >> m_MatchState;
	in >> m_MatchType;
	in >> m_MaximizeShipQ;
	in >> m_MaxShipQ;
	if(m_MaxShipQ)
		m_MaxShipQ = 1;
	in >> m_Name;
	in >> m_OpponentDesignsOk;
	in >> m_Password;
	in >> m_ShipDesign;
	in >> m_ShipDesignDesc;
	in >> m_SpawnCrew;
	in >> m_SpawnOfficer;
	in >> m_SpawnPassengers;
	in >> m_SpawnTroops;
	in >> m_SpawnVehicles;
	in >> m_TeamShips;
	in >> m_Team1Ships;
	in >> m_Team2Ships;
	m_When << in;
}

