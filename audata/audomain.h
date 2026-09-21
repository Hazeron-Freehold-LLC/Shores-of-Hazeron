//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audomain.h,v 1.5 2022/02/19 18:19:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDomain_H
#define AuDomain_H

#include "auid.h"
#include <QHash>

class AuDomain : public QHash<AuId, QString>
{
public:
	inline AuDomain()
	{
	}

	inline AuDomain(const AuDomain& c) : QHash<AuId, QString>(c)
	{
	}

public:
	inline const AuDomain& operator=(const AuDomain& c)
	{
		clear();
		for(const_iterator hi = c.cbegin(); hi != c.cend(); hi++)
			insert(hi.key(), *hi);
		return *this;
	}

	bool operator==(const AuDomain& c) const;

	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:
	// Adds or updates 'systemId' with 'systemName'.
	// Returns false if could not be added because of limit.
	bool Add(const AuId& systemId, const QString& systemName);

	// Adds or updates 'systemId' with 'systemName'.
	// Does not impose the LimitPerAvatar limit.
	void AddNoLimit(const AuId& systemId, const QString& systemName);

	// Returns the maximum number of domain systems that an be stored by each avatar.
	static int LimitPerAvatar();

	// Returns true if 'systemId' is removed.
	bool Remove(const AuId& systemId);

	// Retrieves the list of system ids, the keys from this hash.
	void SystemIds(AuIdList& systemIds) const;
};

inline QDataStream& operator<<(QDataStream& out, const AuDomain& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuDomain& o)
{
	o << in;
	return in;
}

Q_DECLARE_METATYPE(AuDomain)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

#endif // AuDomain_H
