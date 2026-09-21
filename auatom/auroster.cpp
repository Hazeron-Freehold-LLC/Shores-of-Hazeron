//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auroster.cpp,v 1.30 2022/02/19 17:40:58 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auroster.h"

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

AuRosterEntry::AuRosterEntry()
{
	m_BerthId = 0;
	m_Type = Invalid;
}

AuRosterEntry::AuRosterEntry(TYPE type, quint32 berthId, const AuId& occId, const QString& occName)
{
	m_BerthId = berthId;
	m_OccId = occId;
	m_OccName = occName;
	m_Type = type;
}

AuRosterEntry::~AuRosterEntry()
{
}

AuRosterEntry::AuRosterEntry(const AuRosterEntry& c)
{
	m_BerthId = c.m_BerthId;
	m_OccId = c.m_OccId;
	m_OccName = c.m_OccName;
	m_Type = c.m_Type;
}

const AuRosterEntry& AuRosterEntry::operator=(const AuRosterEntry& c)
{
	m_BerthId = c.m_BerthId;
	m_OccId = c.m_OccId;
	m_OccName = c.m_OccName;
	m_Type = c.m_Type;

	return *this;
}

void AuRosterEntry::operator<<(QDataStream& in)
{
	in >> m_BerthId;
	quint8 ui8;
	in >> ui8;
	m_Type = (TYPE)ui8;
	m_OccId << in;
	in >> m_OccName;
}

void AuRosterEntry::operator>>(QDataStream& out) const
{
	out << m_BerthId;
	out << (quint8)m_Type;
	m_OccId >> out;
	out << m_OccName;
}

QString AuRosterEntry::TypeName() const
{
	switch(m_Type)
	{
	case Invalid:
	case CaptainSlot:
		break;
	case Captain:
		return QObject::tr("Capt Berth");
	case OfficerHuman:
	case OfficerAI:
		return QObject::tr("Officer Berth");
	case Crew:
		return QObject::tr("Crew Berth");
	case PassengerAI:
	case PassengerHuman:
		return QObject::tr("Passenger Berth");
	case Troop:
		return QObject::tr("Troop Berth");
	}
	return QString();
}

QString AuRosterEntry::BerthDesc() const
{
	if(m_OccId.IsNull() || m_OccName.isEmpty())
		return TypeName();
	else
		return QObject::tr("%1, %2").arg(TypeName()).arg(m_OccName);
}

quint64 AuRosterEntry::MemoryUsage() const
{
	quint64 mem = sizeof(AuRosterEntry);
	mem += StringMemoryUsage(m_OccName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

AuRoster::AuRoster()
{
}

AuRoster::~AuRoster()
{
}

AuRoster::AuRoster(const AuRoster& c) : QList<AuRosterEntry>(c)
{
}

const AuRoster& AuRoster::operator=(const AuRoster& c)
{
	QList<AuRosterEntry>::operator=(c);
	return *this;
}

void AuRoster::operator<<(QDataStream& in)
{
	clear();
	quint16 num;
	in >> num;
	for(;num;num--)
	{
		AuRosterEntry re;
		re << in;
		append(re);
	}
}

void AuRoster::operator>>(QDataStream& out) const
{
	out << (quint16)count();
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		(*it) >> out;
}

quint32 AuRoster::BerthDetailOccupied(const AuId& occupantId) const
{
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		if((*it).m_OccId == occupantId)
			return (*it).m_BerthId;
	return 0;
}

AuRosterEntry::TYPE AuRoster::BerthTypeOccupied(const AuId& occupantId) const
{
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		if((*it).m_OccId == occupantId)
			return (*it).m_Type;
	return AuRosterEntry::Invalid;
}

// 'type' is the type to query.
unsigned int AuRoster::BerthsOccupied(AuRosterEntry::TYPE type) const
{
	int occ = 0;
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		if((*it).m_Type == type)
			occ++;
	return occ;
}

// Returns the atom id that occupies the captains berth.
// Returns 0 if nobody occupies the captains berth.
AuId AuRoster::CaptainId() const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it).IsCaptainBerth())
			return (*it).m_OccId;
	return AuId(0);
}

QString AuRoster::CaptainName() const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it).IsCaptainBerth())
			return (*it).m_OccName;
	return QString();
}

unsigned int AuRoster::BerthOccupants(quint32 berthId) const
{
	unsigned int occ = 0;
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		if((*it).m_BerthId == berthId)
			occ++;
	return occ;
}

void AuRoster::OccupyBerth(AuRosterEntry::TYPE type, quint32 berthId, const AuId& occupantId, const QString& occupantName)
{
	Q_ASSERT(type != AuRosterEntry::Invalid);
	Q_ASSERT((type == AuRosterEntry::CaptainSlot && berthId == 0) ||
			(type != AuRosterEntry::CaptainSlot && berthId != 0));

	// Add a nu roster entry for the occupant.
	AuRosterEntry entry(type, berthId, occupantId, occupantName);
	append(entry);
}

bool AuRoster::ClearCrewAndTroops()
{
	bool removedAny = false;
	iterator it = begin();
	while(it != end())
	{
		if((*it).IsCrew() || (*it).m_Type == AuRosterEntry::Troop)
		{
			// Found a crew member or a troop; remove them.
			it = erase(it);
			removedAny = true;
		}
		else
			it++;
	}
	return removedAny;
}

bool AuRoster::RemoveOccupant(const AuId& occupantId)
{
	iterator it = begin();
	while(it != end())
	{
		if((*it).m_OccId == occupantId)
		{
			it = erase(it);
			return true;
		}
		else
			it++;
	}
	return false;
}

bool AuRoster::HasAIOfficer() const
{
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		if((*it).m_Type == AuRosterEntry::OfficerAI)
			return true;
	return false;
}

bool AuRoster::HasHumanOfficer() const
{
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		if((*it).m_Type == AuRosterEntry::OfficerHuman)
			return true;
	return false;
}

// Returns true if any crew members remain in the roster. i.e. Captain, Officers, or Crew.
bool AuRoster::HasAnyCrew() const
{
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		if((*it).IsCrew())
			return true;
	return false;
}

// Returns true if any crew members or troops remain in the roster. i.e. Captain, Officers, Crew, or Troops.
bool AuRoster::HasAnyCrewOrTroops() const
{
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		if((*it).IsCrew() || (*it).m_Type == AuRosterEntry::Troop)
			return true;
	return false;
}

QString AuRoster::BerthOccupantName(const AuId& occupantId) const
{
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		if((*it).m_OccId == occupantId)
			return (*it).OccupantName();
	return QString();
}

QString AuRoster::BerthDesc(quint32 berthId) const
{
	//qDebug() << "AuRoster::BerthDesc" << berthId;
	QString occ;
	for(const_iterator it = cbegin(); it != cend(); it++)
	{
		//qDebug() << " AuRoster::BerthDesc" << (*it).m_BerthId << (*it).BerthDesc();
		if((*it).m_BerthId != berthId)
			continue;
		if(occ.isEmpty())
			occ = (*it).BerthDesc();
		else
			occ = QObject::tr("%1 and %2").arg(occ).arg((*it).BerthDesc());	// Old style berths can have two occupants.
	}
	return occ;
}

quint64 AuRoster::MemoryUsage() const
{
	quint64 mem = sizeof(AuRoster);
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		mem += (*it).MemoryUsage();
	return mem;
}
