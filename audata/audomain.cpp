//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audomain.cpp,v 1.7 2022/02/19 18:20:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audomain.h"
//

// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
// The Q_DECLARE_METATYPE macro is also required.
static int s_AuDomainMetaType = qRegisterMetaType<AuDomain>();

void AuDomain::operator>>(QDataStream& out) const
{
	Q_ASSERT(size() <= 255);
	out << (quint8)size();
	for(const_iterator hi = cbegin(); hi != cend(); hi++)
	{
		out << hi.key();
		out << hi.value();
	}
}

void AuDomain::operator<<(QDataStream& in)
{
	clear();
	quint8 n;
	in >> n;
	while(n--)
	{
		AuId systemId;
		QString systemName;
		in >> systemId;
		in >> systemName;
		insert(systemId, systemName);
	}
}

bool AuDomain::operator==(const AuDomain& c) const
{
	if(size() != c.size())
		return false;
	const_iterator hi = cbegin();
	const_iterator ci = c.cbegin();
	while(hi != cend() && ci != c.cend())
	{
		if(hi.key() != ci.key())
			return false;
		if(hi.value() != ci.value())
			return false;
		hi++;
		ci++;
	}
	return true;
}

bool AuDomain::Add(const AuId& systemId, const QString& systemName)
{
	iterator si = find(systemId);
	if(si == end())
	{
		// New entry.
		if(size() >= LimitPerAvatar())
			return false;
		insert(systemId, systemName);
	}
	else
	{
		// Existing entry.
		*si = systemName;	// Update name.
	}
	return true;
}

void AuDomain::AddNoLimit(const AuId& systemId, const QString& systemName)
{
	iterator si = find(systemId);
	if(si == end())
	{
		// New entry.
		insert(systemId, systemName);
	}
	else
	{
		// Existing entry.
		*si = systemName;	// Update name.
	}
	Q_ASSERT(size() <= 255);
}

bool AuDomain::Remove(const AuId& systemId)
{
	iterator si = find(systemId);
	if(si == end())
		return false;
	erase(si);
	return true;
}

//static
int AuDomain::LimitPerAvatar()
{
	return 10;
}

void AuDomain::SystemIds(AuIdList& systemIds) const
{
	for(const_iterator hi = cbegin(); hi != cend(); hi++)
		systemIds.append(hi.key());
}
