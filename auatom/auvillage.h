//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auvillage.h,v 1.5 2022/02/19 17:33:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuVillage_H
#define AuVillage_H

#include "auatom.h"
#include "auglobell.h"
#include "auid.h"
#include "auspecies.h"
#include "autime.h"

class AuSpecies;

class AuVillage
{
	enum STATE
	{
		New,							// Village has never been moved from its original location.
		Moved,							// Village has moved from its original location.
	};

	QHash<AuId, qint8>					m_FriendsAndEnemies;	// Individual avatar ids with their local reputation. (-100 to 100)
	AuTime								m_LastContact;			// Last time contacted by anyone.
	AuTime								m_LastMoved;			// Last time moved to a new location.
	AuGlobeLLF							m_LL;					// Current longitude and latitude.
	quint8								m_Population;			// Total number of people represented by the village.
	AuSpecies							m_Species;				// Usually matches indigenous. Stories could decide otherwise.
	quint8								m_State;				// State of the village.

public:	// Constructors
	AuVillage(const AuSpecies& species, quint8 population, const AuGlobeLLF& ll);
	AuVillage();
	AuVillage(const AuVillage& c);

public:	// Operators
	const AuVillage& operator=(const AuVillage& c);

	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:
	inline void AttackedBy(const AuId& attackerId)
	{
		ChangeReputation(attackerId, -1);
		m_LastContact.Set();
	}

	inline const AuSpecies& Species() const
	{
		return m_Species;
	}

	inline bool ContactedRecently() const
	{
		return m_LastContact.ElapsedHour() < 1;
	}

	inline bool HasMet(const AuId& personId) const
	{
		return m_FriendsAndEnemies.contains(personId);
	}

	// 'isContact' can be false so killing a wild animal can improve reputatation without stopping village encounters.
	inline void HelpedBy(const AuId& personId, bool isContact)
	{
		ChangeReputation(personId, 1);	// Increase reputation for doing a good deed.
		if(isContact)
			m_LastContact.Set();
	}

	inline void InjuredBy(const AuId& attackerId)
	{
		ChangeReputation(attackerId, -2);
		m_LastContact.Set();
	}

	inline bool IsEmpty() const
	{
		return m_Population == 0;
	}

	inline const AuGlobeLLF& LL() const
	{
		return m_LL;
	}

	inline void KilledBy(const AuId& attackerId)
	{
		ChangeReputation(attackerId, -10);
		m_LastContact.Set();
	}

	inline const AuTime& LastContacted() const
	{
		return m_LastContact;
	}

	inline bool MovedRecently() const
	{
		return m_LastMoved.ElapsedHour() < 1;
	}

	inline void MoveTo(const AuGlobeLLF& ll)
	{
		m_LL = ll;
		m_LastMoved.Set();
		m_State = Moved;
	}

	inline quint8 Population() const
	{
		return m_Population;
	}

	inline qint8 Reputation(const AuId& personId) const
	{
		QHash<AuId, qint8>::const_iterator fei = m_FriendsAndEnemies.find(personId);
		if(fei == m_FriendsAndEnemies.cend())
			return 0;
		return *fei;
	}

	inline void ResetReputation(const AuId& personId)
	{
		m_FriendsAndEnemies.remove(personId);
	}

	inline void SetContacted(const AuId& personId, qint8 reaction)
	{
		QHash<AuId, qint8>::iterator fei = m_FriendsAndEnemies.find(personId);
		if(fei == m_FriendsAndEnemies.end())
			m_FriendsAndEnemies.insert(personId, reaction);

		m_LastContact.Set();
	}

	inline void SetPopulation(quint8 population)
	{
		m_Population = population;
	}

	// Raises negative reputations of people, up to 0. Does not change positive or 0 reputations.
	// 'howMuch' should be something like real days since last contact.
	void SoftenReputations(int howMuch);

private:
	void ChangeReputation(const AuId& personId, qint8 change);
};

inline QDataStream& operator<<(QDataStream& out, const AuVillage& vlg)
{
	vlg >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuVillage& vlg)
{
	vlg << in;
	return in;
}

AUATOM_DLL QDebug operator<<(QDebug dbg, const AuVillage& vlg);

#endif // AuVillage_H
