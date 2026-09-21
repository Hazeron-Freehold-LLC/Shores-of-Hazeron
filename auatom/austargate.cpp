//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austargate.cpp,v 1.5 2022/02/19 17:41:18 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "austargate.h"
//

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuStargate

void AuStargate::operator>>(QDataStream& out) const
{
	out << m_Expiry.UTC();
	out << (quint8)m_Origin;
	out << m_ToGalaxy;
	out << m_ToLoc;
}

void AuStargate::operator<<(QDataStream& in)
{
	quint8 ui8;

	QDateTime tm;
	in >> tm;
	tm.setTimeSpec(Qt::UTC);
	m_Expiry = tm;

	in >> ui8;
	m_Origin = ui8;
	in >> m_ToGalaxy;
	in >> m_ToLoc;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuStargateList

void AuStargateList::operator>>(QDataStream& out) const
{
	Q_ASSERT(count() < 256);	// That's a LOT of stargates.
	out << (quint8)count();
	for(const_iterator si = constBegin(); si != constEnd(); si++)
		out << *si;
}

void AuStargateList::operator<<(QDataStream& in)
{
	clear();
	quint8 cnt;
	in >> cnt;
	AuStargate stargate;
	while(cnt--)
	{
		in >> stargate;
		append(stargate);
	}
}

// Removes stargates that have reached their expiry time.
int AuStargateList::Decay(bool closeOrigins)
{
	int closed = 0;
	iterator si = begin();
	while(si != end())
	{
		if((*si).IsExpired() || (closeOrigins && (*si).IsOrigin()))
		{
			if((*si).IsOrigin())
				closed = 2;
			else if(!closed)
				closed = 1;

			si = erase(si);
		}
		else
			si++;
	}
	return closed;
}

quint64 AuStargateList::MemoryUsage() const
{
	quint64 mem = sizeof(AuStargateList);
	mem += sizeof(AuStargate) * size();
	return mem;
}
