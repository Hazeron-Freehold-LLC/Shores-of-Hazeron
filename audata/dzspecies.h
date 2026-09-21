//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dzspecies.h,v 1.12 2022/02/19 18:19:53 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DzSpecies_H
#define DzSpecies_H

#include "audata.h"
#include "auid.h"
#include "auspecies.h"
#include "autime.h"

class QSqlDatabase;

class AUDATA_DLL DzSpecies
{
friend class DzSpeciesHash;
	QString								m_Name;					// Name assigned to this specie.
	AuSpecies							m_Species;
	AuTime								m_WhenNamed;			// When this specie was named.
	AuId								m_WhoNamed;				// Person atom who named this specie.

public:	// Constructors
	DzSpecies(const AuSpecies& species, const QString& name, const AuId& whoNamed);
	DzSpecies();
	~DzSpecies();
	DzSpecies(const DzSpecies &c);

public:	// Operators
	DzSpecies& operator=(const DzSpecies &c);

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;

	inline bool operator==(const DzSpecies& c) const
	{
		if(m_Name != c.m_Name)
			return false;
		if(m_Species != c.m_Species)
			return false;
		if(m_WhenNamed != c.m_WhenNamed)
			return false;
		if(m_WhoNamed != c.m_WhoNamed)
			return false;
		return true;
	}

public:	// Methods
	void Insert(QSqlDatabase& db) const;

	quint64 MemoryUsage() const;

	inline const QString& Name() const
	{
		return m_Name;
	}

	bool Select(QSqlDatabase& db, const AuSpecies& species);

	inline const AuSpecies& Species() const
	{
		return m_Species;
	}

	inline const AuTime& WhenNamed() const
	{
		return m_WhenNamed;
	}

	inline const AuId& WhoNamed() const
	{
		return m_WhoNamed;
	}

	void Update(QSqlDatabase& db) const;
};

inline QDataStream& operator<<(QDataStream& out, const DzSpecies& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, DzSpecies& o)
{
	o << in;
	return in;
}

Q_DECLARE_METATYPE(DzSpecies)	// Needed to this use class in queued signal/slot connections. Must also call qRegisterMetaType.

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL DzSpeciesHash : public unordered_map<AuSpecies, DzSpecies, AuSpeciesHash>
{
public:	// Operators
	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;

public:	// Members
	void Load();

	quint64 MemoryUsage() const;
};

inline QDataStream& operator<<(QDataStream& out, const DzSpeciesHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, DzSpeciesHash& o)
{
	o << in;
	return in;
}

#endif // DzSpecies_H
