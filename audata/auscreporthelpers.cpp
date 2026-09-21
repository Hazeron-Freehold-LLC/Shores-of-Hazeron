//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscreporthelpers.cpp,v 1.4 2022/02/19 18:20:18 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscreporthelpers.h"
//
#include "auscparts.h"

static int s_AuScPartSetMetaType = AuScPartSet::RegisterMetaType();

//static
int AuScPartSet::RegisterMetaType()
{
	// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
	// The Q_DECLARE_METATYPE macro is also required.
	int metaType = qRegisterMetaType<AuScPartSet>();

	// Must call qRegisterMetaTypeStreamOperators in order to use this class type in containers of variants.
	qRegisterMetaTypeStreamOperators<AuScPartSet>();

	return metaType;
}

quint64 AuScPartSet::MemoryUsage() const
{
	quint64 mem = sizeof(AuScPartSet);
	mem += sizeof(quint16) * m_Ids.capacity();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuLiftHash::operator>>(QDataStream& out) const
{
	out << (quint16)size();
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
	{
		out << ci.key();
		out << *ci;
	}
}

void AuLiftHash::operator<<(QDataStream& in)
{
	clear();

	quint16 c;
	in >> c;

	while(c--)
	{
		quint16 k;
		QString v;

		in >> k;
		in >> v;

		insert(k, v);
	}
}

quint64 AuLiftHash::MemoryUsage() const
{
	quint64 mem = sizeof(AuLiftHash);
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
		mem += sizeof(quint16) + StringMemoryUsage((*ci));
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

bool AuScDoorState::IsAutomaticDoor() const
{
	return !IsHullDoor() && !IsTurboLiftDoor() && !IsManualDoor() && (IsOpen() || !IsLocked());
}

bool AuScDoorState::IsJammed() const
{
	return m_State == AuScDoor::StuckOpen || m_State == AuScDoor::StuckClosed;
}

bool AuScDoorState::IsLocked() const
{
	return m_State == AuScDoor::LockedClosed || m_State == AuScDoor::LockedOpen;
}

bool AuScDoorState::SetLock(bool locked)
{
	if(locked)
	{
		if(m_State == AuScDoor::Closed)
		{
			m_State = AuScDoor::LockedClosed;
			return true;
		}
		else if(m_State == AuScDoor::Open)
		{
			m_State = AuScDoor::LockedOpen;
			return true;
		}
	}
	else
	{
		if(m_State == AuScDoor::LockedClosed)
		{
			m_State = AuScDoor::Closed;
			return true;
		}
		else if(m_State == AuScDoor::LockedOpen)
		{
			m_State = AuScDoor::Open;
			return true;
		}
	}
	return false;
}

bool AuScDoorState::IsOpen() const
{
	return m_State == AuScDoor::Open || m_State == AuScDoor::StuckOpen || m_State == AuScDoor::LockedOpen;
}

bool AuScDoorState::IsClosed() const
{
	return m_State == AuScDoor::Closed || m_State == AuScDoor::StuckClosed || m_State == AuScDoor::LockedClosed;
}

void AuScDoorState::Close()
{
	switch((AuScDoor::STATES)m_State)
	{
	case AuScDoor::Open:
	case AuScDoor::StuckOpen:
	case AuScDoor::StuckClosed:
		m_State = AuScDoor::Closed;
		break;
	case AuScDoor::LockedOpen:
		m_State = AuScDoor::LockedClosed;
		break;
	case AuScDoor::Closed:
	case AuScDoor::LockedClosed:
	case AuScDoor::Open10:
	case AuScDoor::Open20:
	case AuScDoor::Open30:
	case AuScDoor::Open40:
	case AuScDoor::Open50:
	case AuScDoor::Open60:
	case AuScDoor::Open70:
	case AuScDoor::Open80:
	case AuScDoor::Open90:
	case AuScDoor::STATES_Count:
		break;
	}
}

void AuScDoorState::Open()
{
	switch((AuScDoor::STATES)m_State)
	{
	case AuScDoor::Closed:
	case AuScDoor::StuckClosed:
	case AuScDoor::StuckOpen:
		m_State = AuScDoor::Open;
		break;
	case AuScDoor::LockedClosed:
		m_State = AuScDoor::LockedOpen;
		break;
	case AuScDoor::Open:
	case AuScDoor::LockedOpen:
	case AuScDoor::Open10:
	case AuScDoor::Open20:
	case AuScDoor::Open30:
	case AuScDoor::Open40:
	case AuScDoor::Open50:
	case AuScDoor::Open60:
	case AuScDoor::Open70:
	case AuScDoor::Open80:
	case AuScDoor::Open90:
	case AuScDoor::STATES_Count:
		break;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuLiftStop::operator>>(QDataStream& out) const
{
	out << m_DoorId;
	out << (quint16)0;	// used to store room id.
}

void AuLiftStop::operator<<(QDataStream& in)
{
	in >> m_DoorId;
	quint16 roomId;
	in >> roomId;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuLiftStopHash::operator>>(QDataStream& out) const
{
	out << (quint16)size();
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
	{
		out << ci.key();
		out << *ci;
	}
}

void AuLiftStopHash::operator<<(QDataStream& in)
{
	clear();

	quint16 c;
	in >> c;

	while(c--)
	{
		QString k;
		AuLiftStop v;

		in >> k;
		in >> v;

		insert(k, v);
	}
}

QString AuLiftStopHash::DoorStopName(quint16 doorId) const
{
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
		if((*ci).DoorId() == doorId)
			return ci.key();
	return "";
}

quint64 AuLiftStopHash::MemoryUsage() const
{
	quint64 mem = sizeof(AuLiftStopHash);
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
		mem += StringMemoryUsage(ci.key()) + sizeof(AuLiftStop);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

bool AuLandingGearStateHash::IsAllExtended() const
{
	// Empty landing gear implies fixed gear.
	if(m_States.empty())
		return true;

	for(QHash<quint16, quint8>::const_iterator gi = m_States.cbegin(); gi != m_States.cend(); gi++)
		if(!AuScLandingGear::IsExtended((AuScLandingGear::STATES)*gi))
			return false;	// A gear is not extended, possibly stuck.
	return true;
}

bool AuLandingGearStateHash::Extend()
{
	bool anyChange = false;
	for(QHash<quint16, quint8>::iterator gi = m_States.begin(); gi != m_States.end(); gi++)
		if(*gi == AuScLandingGear::Retracted)
		{
			*gi = AuScLandingGear::Extended;
			anyChange = true;
		}
	return anyChange;
}

bool AuLandingGearStateHash::Retract()
{
	bool anyChange = false;
	for(QHash<quint16, quint8>::iterator gi = m_States.begin(); gi != m_States.end(); gi++)
		if(*gi == AuScLandingGear::Extended)
		{
			*gi = AuScLandingGear::Retracted;
			anyChange = true;
		}
	return anyChange;
}

bool AuLandingGearStateHash::Unstick()
{
	bool anyChange = false;
	for(QHash<quint16, quint8>::iterator gi = m_States.begin(); gi != m_States.end(); gi++)
		if(*gi == AuScLandingGear::StuckExtended)
		{
			*gi = AuScLandingGear::Extended;
			anyChange = true;
		}
		else if(*gi == AuScLandingGear::StuckRetracted)
		{
			*gi = AuScLandingGear::Retracted;
			anyChange = true;
		}
	return anyChange;
}

void AuLandingGearStateHash::operator>>(QDataStream& out) const
{
	out << (quint16)m_States.size();
	for(QHash<quint16, quint8>::const_iterator ci = m_States.cbegin(); ci != m_States.cend(); ci++)
	{
		out << ci.key();
		out << *ci;
	}
}

void AuLandingGearStateHash::operator<<(QDataStream& in)
{
	m_States.clear();

	quint16 c;
	in >> c;

	while(c--)
	{
		quint16 k;
		quint8 v;

		in >> k;
		in >> v;

		m_States.insert(k, v);
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuScReportLiftHash::operator>>(QDataStream& out) const
{
	out << (quint16)size();
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
	{
		out << ci.key();
		out << *ci;
	}
}

void AuScReportLiftHash::operator<<(QDataStream& in)
{
	clear();

	quint16 c;
	in >> c;

	while(c--)
	{
		quint16 k;
		AuLiftStopHash v;

		in >> k;
		in >> v;

		insert(k, v);
	}
}

quint64 AuScReportLiftHash::MemoryUsage() const
{
	quint64 mem = sizeof(AuScReportLiftHash);
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
		mem += sizeof(quint16) + (*ci).MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuDoorStateHash::operator>>(QDataStream& out) const
{
	out << (quint16)size();
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
	{
		out << ci.key();
		out << *ci;
	}
}

void AuDoorStateHash::operator<<(QDataStream& in)
{
	clear();

	quint16 c;
	in >> c;

	while(c--)
	{
		quint16 k;
		AuScDoorState v;

		in >> k;
		in >> v;

		insert(k, v);
	}
}

bool AuDoorStateHash::AnyHullDoorOpen() const
{
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
		if((*ci).IsHullDoor() && (*ci).IsOpen())
			return true;
	return false;
}

bool AuDoorStateHash::AnyHullDoorUnlocked() const
{
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
		if((*ci).IsHullDoor() && (*ci).IsLocked())
			return true;
	return false;
}

bool AuDoorStateHash::AnyJammed(AuPoint& where) const
{
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
		if((*ci).IsJammed())
		{
			where = (*ci).OpLoc1();
			return true;
		}
	return false;
}

bool AuDoorStateHash::CloseAllDoors()
{
	bool anyChange = false;
	for(iterator di = begin(); di != end(); di++)
		if((*di).IsOpen())
		{
			(*di).Close();
			anyChange = true;
		}
	return anyChange;
}

bool AuDoorStateHash::CloseHullDoors()
{
	bool anyChange = false;
	for(iterator di = begin(); di != end(); di++)
		if((*di).IsHullDoor() && (*di).IsOpen() && !(*di).IsJammed())
		{
			(*di).Close();
			anyChange = true;
		}
	return anyChange;
}

bool AuDoorStateHash::CloseAssociatedDoors(quint16 liftId)
{
	if(!liftId)
		return true;

	bool anyStuckOpen = false;
	for(iterator di = begin(); di != end(); di++)
	{
		if((*di).PartId() != liftId)
			continue;
		if((*di).State() == AuScDoor::StuckOpen)
			anyStuckOpen = true;					// Lift door is stuck open.
		else if((*di).IsOpen())
			(*di).Close();
	}
	return !anyStuckOpen;
}

bool AuDoorStateHash::CloseDoor(quint16 doorId)
{
	bool anyChange = false;
	iterator di = find(doorId);
	if(di != end() && !(*di).IsClosed())
	{
		(*di).Close();
		anyChange = true;
	}
	return anyChange;
}

bool AuDoorStateHash::JamDoorClosed(quint16 doorId)
{
	bool anyChange = false;
	iterator di = find(doorId);
	if(di != end() && (*di).State() != AuScDoor::StuckClosed)
	{
		(*di).SetState(AuScDoor::StuckClosed);
		anyChange = true;
	}
	return anyChange;
}

bool AuDoorStateHash::JamDoorOpen(quint16 doorId)
{
	bool anyChange = false;
	iterator di = find(doorId);
	if(di != end() && (*di).State() != AuScDoor::StuckOpen)
	{
		(*di).SetState(AuScDoor::StuckOpen);
		anyChange = true;
	}
	return anyChange;
}

bool AuDoorStateHash::OpenDoor(quint16 doorId)
{
	bool anyChange = false;
	iterator di = find(doorId);
	if(di != end() && !(*di).IsOpen())
	{
		(*di).Open();
		anyChange = true;
	}
	return anyChange;
}

bool AuDoorStateHash::OpenAssociatedDoors(quint16 partId)
{
	if(!partId)
		return true;

	bool anyChange = false;
	for(iterator di = begin(); di != end(); di++)
	{
		if((*di).PartId() != partId)
			continue;
		if((*di).IsOpen())
			continue;
		(*di).Open();
		anyChange = true;
	}
	return anyChange;
}

bool AuDoorStateHash::UnjamDoor(quint16 doorId)
{
	bool anyChange = false;
	iterator di = find(doorId);
	if(di != end())
	{
		if((*di).State() == AuScDoor::StuckOpen)
		{
			(*di).Open();
			anyChange = true;
		}
		else if((*di).State() == AuScDoor::StuckClosed)
		{
			(*di).Close();
			anyChange = true;
		}
	}
	return anyChange;
}

bool AuDoorStateHash::IsTurboLiftDoor(quint16 doorId) const
{
	const_iterator di = find(doorId);
	return di != cend() && (*di).IsTurboLiftDoor();
}

bool AuDoorStateHash::IsHullDoor(quint16 doorId) const
{
	const_iterator di = find(doorId);
	return di != cend() && (*di).IsHullDoor();
}

bool AuDoorStateHash::IsDoorJammed(quint16 doorId) const
{
	const_iterator di = find(doorId);
	return di != cend() && (*di).IsJammed();
}

bool AuDoorStateHash::IsDoorLocked(quint16 doorId) const
{
	const_iterator di = find(doorId);
	return di != cend() && (*di).IsLocked();
}

bool AuDoorStateHash::IsDoorOpen(quint16 doorId) const
{
	const_iterator di = find(doorId);
	return di != cend() && (*di).IsOpen();
}

quint16 AuDoorStateHash::ClosestClosedDoor(const AuPoint& loc, double& distance) const
{
	quint16 bestid = 0;
	double bestdist2 = -1;
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
	{
		if(!(*ci).IsClosed())
			continue;
		double dist2 = loc.Distance2((*ci).OpLoc1());
		if(bestdist2 < 0 || dist2 < bestdist2)
		{
			bestdist2 = dist2;
			bestid = ci.key();
		}

		dist2 = loc.Distance2((*ci).OpLoc2());
		if(bestdist2 < 0 || dist2 < bestdist2)
		{
			bestdist2 = dist2;
			bestid = ci.key();
		}
	}
	if(bestid)
		distance = sqrt(bestdist2);
	return bestid;
}

quint16 AuDoorStateHash::ClosestDoor(const AuPoint& loc, double& distance) const
{
	quint16 bestid = 0;
	double bestdist2 = -1;
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
	{
		double dist2 = loc.Distance2((*ci).OpLoc1());
		if(bestdist2 < 0 || dist2 < bestdist2)
		{
			bestdist2 = dist2;
			bestid = ci.key();
		}

		dist2 = loc.Distance2((*ci).OpLoc2());
		if(bestdist2 < 0 || dist2 < bestdist2)
		{
			bestdist2 = dist2;
			bestid = ci.key();
		}
	}
	if(bestid)
		distance = sqrt(bestdist2);
	return bestid;
}

quint16 AuDoorStateHash::ClosestOpenDoor(const AuPoint& loc, double& distance) const
{
	quint16 bestid = 0;
	double bestdist2 = -1;
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
	{
		if(!(*ci).IsOpen())
			continue;
		double dist2 = loc.Distance2((*ci).OpLoc1());
		if(bestdist2 < 0 || dist2 < bestdist2)
		{
			bestdist2 = dist2;
			bestid = ci.key();
		}

		dist2 = loc.Distance2((*ci).OpLoc2());
		if(bestdist2 < 0 || dist2 < bestdist2)
		{
			bestdist2 = dist2;
			bestid = ci.key();
		}
	}
	if(bestid)
		distance = sqrt(bestdist2);
	return bestid;
}

quint16 AuDoorStateHash::ClosestRemoteDoor(bool forVehicle, const AuPoint& loc, double& distance) const
{
	quint16 bestid = 0;
	double bestdist2 = -1;
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
	{
		if(forVehicle)
		{
			if(!(*ci).IsRemoteVehicle())
				continue;
		}
		else
		{
			if(!(*ci).IsRemoteSpacecraft())
				continue;
		}
		double dist2 = loc.Distance2((*ci).OpLoc1());
		if(bestdist2 < 0 || dist2 < bestdist2)
		{
			bestdist2 = dist2;
			bestid = ci.key();
		}

		dist2 = loc.Distance2((*ci).OpLoc2());
		if(bestdist2 < 0 || dist2 < bestdist2)
		{
			bestdist2 = dist2;
			bestid = ci.key();
		}
	}
	if(bestid)
		distance = sqrt(bestdist2);
	return bestid;
}

quint16 AuDoorStateHash::DoorAssociatedPart(quint16 doorId) const
{
	const_iterator di = find(doorId);
	if(di == cend())
		return 0;
	return (*di).PartId();
}

bool AuDoorStateHash::OpenAutomaticDoors(const AuPoint& oloc, const AuPoint& nloc, double openDist, QSet<quint16>& opened)
{
	bool anyOpened = false;
	double openDist2 = openDist * openDist;
	for(iterator di = begin(); di != end(); di++)
	{
		if(!(*di).IsAutomaticDoor())
			continue;	// Not an automatic door or locked closed.
		if((*di).IsOpen())
			continue;	// Already open.
		if((*di).IsJammed())
			continue;	// Jammed so it can't open automatically.

		// Calculate distance from operator locations to the path segment.
		double opdist = (*di).OpLoc1().DistToSegment2(oloc, nloc);
		if(opdist > openDist2)
			opdist = (*di).OpLoc2().DistToSegment2(oloc, nloc);
		if(opdist > openDist2)
			continue;	// Too far away.

		// Open the unjammed automatic door that is close enough.
		(*di).Open();
		opened.insert(di.key());
		anyOpened = true;
	}
	return anyOpened;
}

quint64 AuDoorStateHash::MemoryUsage() const
{
	quint64 mem = sizeof(AuDoorStateHash);
	mem += (sizeof(quint16) + sizeof(AuScDoorState)) * capacity();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuScReportSysVol::operator>>(QDataStream& out) const
{
	out << m_Module;
	out << m_Quality;
	out << m_Volume;
}

void AuScReportSysVol::operator<<(QDataStream& in)
{
	in >> m_Module;
	in >> m_Quality;
	if(!m_Quality)
		m_Quality = 1;
	in >> m_Volume;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuScReportSysVolHash::operator>>(QDataStream& out) const
{
	out << (quint16)size();
	for(const_iterator ri = cbegin(); ri != cend(); ri++)
	{
		out << ri.key();
		out << *ri;
	}
}

void AuScReportSysVolHash::operator<<(QDataStream& in)
{
	quint16 c;
	in >> c;
	while(c--)
	{
		quint16 key;
		AuScReportSysVol v;

		in >> key;
		in >> v;

		insert(key, v);
	}
}

quint64 AuScReportSysVolHash::MemoryUsage() const
{
	quint64 mem = sizeof(AuScReportSysVolHash);
	mem += sizeof(AuScReportSysVol) * capacity();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuScReportStation::operator>>(QDataStream& out) const
{
	out << m_Desc;
	out << m_Module;
	out << m_PartType;

	quint8 hasWhat = 0;
	if(m_RoomId)
		hasWhat |= 0x80;
	if(m_WhichEffect)
		hasWhat |= 0x40;
	if(m_Quality > 1)
		hasWhat |= 0x20;

	out << hasWhat;

	if(hasWhat & 0x80)
		out << m_RoomId;
	if(hasWhat & 0x40)
		out << m_WhichEffect;
	if(hasWhat & 0x20)
		out << m_Quality;
}

void AuScReportStation::operator<<(QDataStream& in)
{
	in >> m_Desc;
	in >> m_Module;
	in >> m_PartType;

	quint8 hasWhat;
	in >> hasWhat;

	if(hasWhat & 0x80)
		in >> m_RoomId;
	else
		m_RoomId = 0;

	if(hasWhat & 0x40)
		in >> m_WhichEffect;
	else
		m_WhichEffect = 0;

	if(hasWhat & 0x20)
		in >> m_Quality;
	else
		m_Quality = 1;
}

quint64 AuScReportStation::MemoryUsage() const
{
	quint64 mem = 0;
	mem += StringMemoryUsage(m_Desc);
	mem += sizeof(m_Module);
	mem += sizeof(m_PartType);
	mem += sizeof(m_RoomId);
	mem += sizeof(m_Quality);
	mem += sizeof(m_WhichEffect);
	return mem;
}

bool AuScReportStation::IsMedic(quint16 roomId) const
{
	return RoomId() == roomId && (PartType() == AuScStationMedic::TypeStatic() ||
		PartType() == AuScConsoleMedic::TypeStatic());
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuScReportStationHash::operator>>(QDataStream& out) const
{
	out << (quint16)size();
	for(const_iterator ri = cbegin(); ri != cend(); ri++)
	{
		out << ri.key();
		out << *ri;
	}
}

void AuScReportStationHash::operator<<(QDataStream& in)
{
	quint16 c;
	in >> c;
	while(c--)
	{
		quint16 key;
		AuScReportStation v;

		in >> key;
		in >> v;

		insert(key, v);
	}
}

quint64 AuScReportStationHash::MemoryUsage() const
{
	quint64 mem = sizeof(AuScReportStationHash);
	for(const_iterator ri = cbegin(); ri != cend(); ri++)
		mem += (*ri).MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuScReportTransporterStation::operator>>(QDataStream& out) const
{
	out << m_Desc;
	out << m_Module;
	out << m_Pads;
	out << m_PartType;
	out << m_RoomId;

	quint8 hasWhat = 0;
	if(m_WhichEffect)
		hasWhat |= 0x80;
	if(m_Quality > 1)
		hasWhat |= 0x40;
	out << hasWhat;
	if(hasWhat & 0x80)
		out << m_WhichEffect;
	if(hasWhat & 0x40)
		out << m_Quality;
}

void AuScReportTransporterStation::operator<<(QDataStream& in)
{
	in >> m_Desc;
	in >> m_Module;
	in >> m_Pads;
	in >> m_PartType;
	in >> m_RoomId;
	quint8 hasWhat;
	in >> hasWhat;
	if(hasWhat & 0x80)
		in >> m_WhichEffect;
	else
		m_WhichEffect = 0;
	if(hasWhat & 0x40)
		in >> m_Quality;
	else
		m_Quality = 1;
}

quint64 AuScReportTransporterStation::MemoryUsage() const
{
	quint64 mem = 0;
	mem += StringMemoryUsage(m_Desc);
	mem += sizeof(m_Module);
	mem += sizeof(m_Pads);
	mem += sizeof(m_PartType);
	mem += sizeof(m_RoomId);
	mem += sizeof(m_Quality);
	mem += sizeof(m_WhichEffect);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuScReportTransporterStationHash::operator>>(QDataStream& out) const
{
	out << (quint16)size();
	for(const_iterator ri = cbegin(); ri != cend(); ri++)
	{
		out << ri.key();
		out << *ri;
	}
}

void AuScReportTransporterStationHash::operator<<(QDataStream& in)
{
	quint16 c;
	in >> c;
	while(c--)
	{
		quint16 key;
		AuScReportTransporterStation v;

		in >> key;
		in >> v;

		insert(key, v);
	}
}

bool AuScReportTransporterStationHash::AnyInRoom(quint16 roomId) const
{
	for(const_iterator ri = cbegin(); ri != cend(); ri++)
		if((*ri).RoomId() == roomId)
			return true;
	return false;
}

quint64 AuScReportTransporterStationHash::MemoryUsage() const
{
	quint64 mem = sizeof(AuScReportTransporterStationHash);
	for(const_iterator ri = cbegin(); ri != cend(); ri++)
		mem += sizeof(quint16) + (*ri).MemoryUsage();
	return mem;
}

quint16 AuScReportTransporterStationHash::CheckAmbiguousRoom() const
{
	QSet<quint16> trooms;		// ids of rooms that contain transporter stations
	for(const_iterator ri = cbegin(); ri != cend(); ri++)
		if(trooms.contains((*ri).RoomId()))
			return (*ri).RoomId();			// The room already contains a transporter station.
		else
			trooms.insert((*ri).RoomId());	// Remember the transporter room id.
	return 0;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuScReportServicePanel::operator>>(QDataStream& out) const
{
	out << m_Desc;
	out << m_PartType;
}

void AuScReportServicePanel::operator<<(QDataStream& in)
{
	in >> m_Desc;
	in >> m_PartType;
}

quint64 AuScReportServicePanel::MemoryUsage() const
{
	quint64 mem = 0;
	mem += StringMemoryUsage(m_Desc);
	mem += sizeof(m_PartType);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuScReportServicePanelHash::operator>>(QDataStream& out) const
{
	out << (quint16)size();
	for(const_iterator ri = cbegin(); ri != cend(); ri++)
	{
		out << ri.key();
		out << *ri;
	}
}

void AuScReportServicePanelHash::operator<<(QDataStream& in)
{
	quint16 c;
	in >> c;
	while(c--)
	{
		quint16 key;
		AuScReportServicePanel v;
		in >> key;
		in >> v;
		insert(key, v);
	}
}

quint64 AuScReportServicePanelHash::MemoryUsage() const
{
	quint64 mem = sizeof(AuScReportServicePanelHash);;
	for(const_iterator ri = cbegin(); ri != cend(); ri++)
		mem += sizeof(quint16) + (*ri).MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuScReportStatusDisplay::operator>>(QDataStream& out) const
{
	out << m_Desc;
	out << (quint8)0xff;	// Indicates the hasWhat byte follows.
	quint8 hasWhat = 0;
	if(!m_RoomName.isEmpty())
		hasWhat |= 0x01;
	if(m_AssociatedPartId)
		hasWhat |= 0x02;
	out << hasWhat;
	out << m_PartType;

	if(hasWhat & 0x01)
		out << m_RoomName;

	if(hasWhat & 0x02)
		out << m_AssociatedPartId;
}

void AuScReportStatusDisplay::operator<<(QDataStream& in)
{
	in >> m_Desc;
	quint8 partType;
	in >> partType;
	if(partType == 0xff)
	{
		// The hasWhat byte follows.
		quint8 hasWhat;
		in >> hasWhat;
		in >> m_PartType;

		if(hasWhat & 0x01)
			in >> m_RoomName;

		if(hasWhat & 0x02)
			in >> m_AssociatedPartId;
	}
	else
	{
		// Old format.
		m_PartType = partType & 0x7f;
		if(partType & 0x80)
			in >> m_RoomName;
	}
}

// Returns true for berth name, empire flag, empire name, capt name and ship name, which are handled like status displays.
bool AuScReportStatusDisplay::IsDecal() const
{
	return m_PartType == AuScPart::Part_BerthName ||
		m_PartType == AuScPart::Part_CaptName ||
		m_PartType == AuScPart::Part_EmpireFlag ||
		m_PartType == AuScPart::Part_EmpireName ||
		m_PartType == AuScPart::Part_ShipName;
}

quint64 AuScReportStatusDisplay::MemoryUsage() const
{
	quint64 mem = 0;
	mem += sizeof(m_AssociatedPartId);
	mem += StringMemoryUsage(m_Desc);
	mem += sizeof(m_PartType);
	mem += StringMemoryUsage(m_RoomName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuScReportStatusDisplayHash::operator>>(QDataStream& out) const
{
	out << (quint16)size();
	for(const_iterator ri = cbegin(); ri != cend(); ri++)
	{
		out << ri.key();
		out << *ri;
	}
}

void AuScReportStatusDisplayHash::operator<<(QDataStream& in)
{
	quint16 c;
	in >> c;
	while(c--)
	{
		quint16 key;
		AuScReportStatusDisplay v;
		in >> key;
		in >> v;
		insert(key, v);
	}
}

quint64 AuScReportStatusDisplayHash::MemoryUsage() const
{
	quint64 mem = sizeof(AuScReportStatusDisplayHash);;
	for(const_iterator ri = cbegin(); ri != cend(); ri++)
		mem += sizeof(quint16) + (*ri).MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuScReportGameTable::operator>>(QDataStream& out) const
{
	quint8 hasWhat = 0;
	if(m_RoomId)
		hasWhat |= 0x01;

	out << hasWhat;

	if(hasWhat & 0x01)
		out << m_RoomId;
}

void AuScReportGameTable::operator<<(QDataStream& in)
{
	quint8 hasWhat;
	in >> hasWhat;

	if(hasWhat & 0x01)
		in >> m_RoomId;
	else
		m_RoomId = 0;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuScReportGameTableHash::operator>>(QDataStream& out) const
{
	out << (quint16)size();
	for(const_iterator ri = cbegin(); ri != cend(); ri++)
	{
		out << ri.key();
		out << *ri;
	}
}

void AuScReportGameTableHash::operator<<(QDataStream& in)
{
	quint16 c;
	in >> c;
	while(c--)
	{
		quint16 key;
		AuScReportGameTable v;

		in >> key;
		in >> v;

		insert(key, v);
	}
}

bool AuScReportGameTableHash::AnyInRoom(quint16 roomId) const
{
	for(const_iterator ri = cbegin(); ri != cend(); ri++)
		if((*ri).RoomId() == roomId)
			return true;
	return false;
}

quint64 AuScReportGameTableHash::MemoryUsage() const
{
	quint64 mem = sizeof(AuScReportGameTableHash);
	mem += (sizeof(quint16) + sizeof(AuScReportGameTable)) * capacity();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuPartArchitectHash::AuPartArchitectHash()
{
}

AuPartArchitectHash::AuPartArchitectHash(const AuPartArchitectHash& c) : m_Arch(c.m_Arch)
{
}

const AuPartArchitectHash& AuPartArchitectHash::operator=(const AuPartArchitectHash& c)
{
	m_Arch = c.m_Arch;
	return *this;
}

bool AuPartArchitectHash::operator==(const AuPartArchitectHash& c) const
{
	if(m_Arch != c.m_Arch)
		return false;
	return true;
}

void AuPartArchitectHash::operator>>(QDataStream& out) const
{
	out << (quint16)m_Arch.size();
	for(QHash<AuId, QString>::const_iterator ri = m_Arch.cbegin(); ri != m_Arch.cend(); ri++)
	{
		out << ri.key();
		out << *ri;
	}
}

void AuPartArchitectHash::operator<<(QDataStream& in)
{
	m_Arch.clear();
	quint16 c;
	in >> c;
	while(c--)
	{
		AuId key;
		QString v;

		in >> key;
		in >> v;

		m_Arch.insert(key, v);
	}
}

quint64 AuPartArchitectHash::MemoryUsage() const
{
	quint64 mem = sizeof(AuPartArchitectHash);
	for(QHash<AuId, QString>::const_iterator ri = m_Arch.cbegin(); ri != m_Arch.cend(); ri++)
		mem += sizeof(ri.key()) + StringMemoryUsage((*ri));
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuScReportTurret::operator>>(QDataStream& out) const
{
	out << m_Module;

	quint8 hastl = 0;
	if(m_RoomId)
		hastl |= 0x80;
	if(m_Quality > 1)
		hastl |= 0x40;

	out << hastl;

	if(hastl & 0x80)
		out << m_RoomId;
	if(hastl & 0x40)
		out << m_Quality;
}

void AuScReportTurret::operator<<(QDataStream& in)
{
	in >> m_Module;

	quint8 hastl;
	in >> hastl;

	if(hastl & 0x80)
		in >> m_RoomId;
	else
		m_RoomId = 0;

	if(hastl & 0x40)
		in >> m_Quality;
	else
		m_Quality = 1;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuScReportTurretHash::operator>>(QDataStream& out) const
{
	out << (quint16)size();
	for(const_iterator ri = cbegin(); ri != cend(); ri++)
	{
		out << ri.key();
		out << *ri;
	}
}

void AuScReportTurretHash::operator<<(QDataStream& in)
{
	quint16 c;
	in >> c;
	while(c--)
	{
		quint16 key;
		AuScReportTurret v;

		in >> key;
		in >> v;

		insert(key, v);
	}
}

quint64 AuScReportTurretHash::MemoryUsage() const
{
	quint64 mem = sizeof(AuScReportTurretHash);
	mem += (sizeof(quint16) + sizeof(AuScReportTurret)) * capacity();
	return mem;
}

