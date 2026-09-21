//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auselectedunit.h,v 1.8 2022/02/19 18:00:31 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSelectedUnit_H
#define AuSelectedUnit_H

#include "auclientlib.h"

class AUCLIENT_DLL AuSelectedUnit
{
private:
	AuId		m_Id;
	QString		m_Name;
	NPC_ROLE	m_NpcRole;	// Value returned from DaSentient::NpcRole
	int			m_Rank;		// For sorting. Selected units are sorted from highest rank to lowest rank.
	QString		m_RankAbbr;	// rank abbreviation
	QString		m_RankName;	// rank full name

public:
	inline AuSelectedUnit(const AuId& id, const QString& name, int rank, const QString& rankAbbr, const QString& rankName, 
			NPC_ROLE npcRole)
	{
		m_Id = id;
		m_Name = name;
		m_NpcRole = npcRole;
		m_Rank = rank;
		m_RankAbbr = rankAbbr;
		m_RankName = rankName;
	}

	inline AuSelectedUnit()
	{
	}

public:	// Operators
	inline bool operator==(const AuSelectedUnit& c) const
	{
		if(m_Id != c.m_Id)
			return false;
		if(m_Name != c.m_Name)
			return false;
		if(m_NpcRole != c.m_NpcRole)
			return false;
		if(m_Rank != c.m_Rank)
			return false;
		if(m_RankAbbr != c.m_RankAbbr)
			return false;
		if(m_RankName != c.m_RankName)
			return false;
		return true;
	}

public:	// Methods
	inline const AuId& Id() const
	{
		return m_Id;
	}

	inline const QString& Name() const
	{
		return m_Name;
	}

	inline NPC_ROLE NpcRole() const
	{
		return m_NpcRole;
	}

	inline int Rank() const
	{
		return m_Rank;
	}

	inline const QString& RankAbbr() const
	{
		return m_RankAbbr;
	}

	inline const QString& RankName() const
	{
		return m_RankName;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuSelectedUnitArray : public QVector<AuSelectedUnit>
{
public:
	// Adds a new unit selection in rank order, highest rank to lowest rank.
	inline void Add(const AuSelectedUnit& selUnit)
	{
		// Attempt to insert the new unit in rank order.
		for(iterator ai = begin(); ai != end(); ai++)
			if(selUnit.Rank() > (*ai).Rank())
			{
				// The new selected unit gets inserted before the current iterator.
				insert(ai, selUnit);
				return;
			}

		// We get here if the new selected unit did not get inserted before an existing entry in the sort order.
		append(selUnit);
	}

	inline bool Contains(const AuId& id) const
	{
		for(const_iterator ai = constBegin(); ai != constEnd(); ai++)
			if((*ai).Id() == id)
				return true;
		return false;
	}

	inline void Remove(const AuId& id)
	{
		for(iterator ai = begin(); ai != end(); ai++)
			if((*ai).Id() == id)
			{
				erase(ai);
				break;
			}
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuSelectedUnit_H
