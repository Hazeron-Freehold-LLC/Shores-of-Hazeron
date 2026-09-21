//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auunitdamage.h,v 1.14 2022/02/19 17:33:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuUnitDamage_H
#define AuUnitDamage_H

#include "auhash.h"
#include "autime.h"
#include "dbcommodity.h"
#include <unordered_map>

// Each AuUnitDamage object is associated with an atom id in the AuUnitDamageHash.

class AuUnitDamage
{
public:
	qint16								m_CID;					// Commodity atom used last, including negative CID_ values.
	quint32								m_iDamage;				// Accumulated amount of damage from the atom.
	AuTime								m_LastDamage;			// Last time damage was dealt by the atom.
	quint8								m_Quality;				// Quality of the object used to inflict the damage.

public:
	inline AuUnitDamage()
	{
		m_CID = CID_Invalid;
		m_iDamage = 0;
		m_Quality = 1;
	}

	AuUnitDamage(const AuUnitDamage& c)
	{
		m_CID = c.m_CID;
		m_iDamage = c.m_iDamage;
		m_LastDamage = c.m_LastDamage;
		m_Quality = c.m_Quality;
	}

	inline COMMODITYID CommodityId() const
	{
		return (COMMODITYID)m_CID;
	}

	// Returns true when this unit damage memory is old enough to be forgotten.
	inline bool Decay() const
	{
		return m_LastDamage.ElapsedSec() > 5*60;					// Forget damage caused more than 5 minutes ago.
	}

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;
};

inline QDataStream& operator<<(QDataStream& out, const AuUnitDamage& dmg)
{
	dmg >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuUnitDamage& dmg)
{
	dmg << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuUnitDamageHash : public unordered_map<AuId, AuUnitDamage, AuIdHash>
{
public:	// Operators
	inline void operator>>(QDataStream& out) const
	{
		out << (quint16)size();
		for(const_iterator di = cbegin(); di != cend(); di++)
		{
			out << (*di).first;
			out << (*di).second;
		}
	}

	inline void operator<<(QDataStream& in)
	{
		clear();
		quint16 cnt;
		in >> cnt;
		while(cnt--)
		{
			AuId id;
			AuUnitDamage d;
			in >> id;
			in >> d;
			insert(pair<AuId, AuUnitDamage>(id, d));
		}
	}

public:
	// Clears damage caused by a particular atom id.
	inline void Clear(const AuId& byWho)
	{
		iterator udi = find(byWho);
		if(udi != end())
			erase(udi);
	}

	// Call this function once per minute to remove unit damage history that has gotten too old.
	inline void Decay()
	{
		iterator udi = begin();
		while(udi != end())
			if((*udi).second.Decay())
				udi = erase(udi);
			else
				udi++;
	}

	inline AuTime LastDamage(const AuId& byWho) const
	{
		const_iterator udi = find(byWho);
		if(udi != cend())
			return (*udi).second.m_LastDamage;
		return AuTime();
	}

	quint64 MemoryUsage() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuUnitDamageHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuUnitDamageHash& o)
{
	o << in;
	return in;
}

#endif // AuUnitDamage_H
