//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auvillage.cpp,v 1.3 2022/02/19 17:41:32 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auvillage.h"
//
#include "audna.h"
#include "dhdna.h"

AuVillage::AuVillage(const AuSpecies& species, quint8 population, const AuGlobeLLF& ll) : m_Species(species)
{
	m_LastMoved = AuTime::Now();
	m_LL = ll;
	m_Population = population;
	m_State = New;
}

AuVillage::AuVillage()
{
	m_Population = 0;
	m_State = New;
}

AuVillage::AuVillage(const AuVillage& c) : m_FriendsAndEnemies(c.m_FriendsAndEnemies), m_Species(c.m_Species)
{
	m_LastContact = c.m_LastContact;
	m_LastMoved = c.m_LastMoved;
	m_LL = c.m_LL;
	m_Population = c.m_Population;
	m_State = c.m_State;
}

const AuVillage& AuVillage::operator=(const AuVillage& c)
{
	m_FriendsAndEnemies = c.m_FriendsAndEnemies;
	m_LastContact = c.m_LastContact;
	m_LastMoved = c.m_LastMoved;
	m_LL = c.m_LL;
	m_Population = c.m_Population;
	m_Species = c.m_Species;
	m_State = c.m_State;

	return *this;
}

void AuVillage::operator>>(QDataStream& out) const
{
	quint8 ui8 = m_State;		// bottom nibble
	ui8 |= 0x80;				// says we have species instead of old version DhDNA
	out << ui8;
	out << m_Species;

	out << (quint16)m_FriendsAndEnemies.count();
	for(QHash<AuId, qint8>::const_iterator fei = m_FriendsAndEnemies.cbegin(); fei != m_FriendsAndEnemies.cend(); fei++)
	{
		out << fei.key();
		out << fei.value();
	}

	out << m_LastContact;
	out << m_LastMoved;
	out << m_LL;
	out << m_Population;
}

void AuVillage::operator<<(QDataStream& in)
{
	quint8 ui8;
	in >> ui8;
	m_State = ui8 & 0x0f;	// bottom nibble
	if(ui8 & 0x80)
		in >> m_Species;
	else
	{
		// Old version serialized a DhDNA.
		DhDNA dna3;
		in >> dna3;
		m_Species = dna3.Species();
	}

	m_FriendsAndEnemies.clear();
	quint16 c;
	in >> c;
	while(c--)
	{
		AuId id;
		quint8 rep;

		in >> id;
		in >> rep;

		m_FriendsAndEnemies.insert(id, rep);
	}

	in >> m_LastContact;
	in >> m_LastMoved;
	in >> m_LL;
	in >> m_Population;
}

// Raises negative reputations of people, up to 0. Does not change positive or 0 reputations.
// 'howMuch' should be something like real days since last contact.
void AuVillage::SoftenReputations(int howMuch)
{
	for(QHash<AuId, qint8>::iterator fei = m_FriendsAndEnemies.begin(); fei != m_FriendsAndEnemies.end(); fei++)
	{
		if(*fei < 0)
		{
			// Found someone with a negative reputation.
			// Increase their current reputation.
			int newrep = *fei + howMuch;
			if(newrep > 0)
				newrep = 0;	// Up to 0.
			*fei = newrep;
		}
	}
}

void AuVillage::ChangeReputation(const AuId& personId, qint8 change)
{
	QHash<AuId, qint8>::iterator fei = m_FriendsAndEnemies.find(personId);
	if(fei == m_FriendsAndEnemies.end())
	{
		m_FriendsAndEnemies.insert(personId, change);
		//qDebug() << "AuVillage::ChangeReputation new person" << personId << "reputation" << change;
	}
	else
	{
		// Clamp reputation to the range -100 to 100.
		int rep = (int)(*fei) + (int)change;
		if(rep > 100)
			rep = 100;
		else if(rep < -100)
			rep = -100;

		//qDebug() << "AuVillage::ChangeReputation known person" << personId << "reputation old" << (int)(*fei)
			//<< "change" << (int)chhange << "new" << rep;
		*fei = (qint8)rep;
	}

	m_LastContact.Set();
}
