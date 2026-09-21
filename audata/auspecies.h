//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auspecies.h,v 1.4 2022/02/19 18:19:20 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSpecies_H
#define AuSpecies_H

#include "audata.h"
#include "audef.h"
#include <QByteArray>
#include <QString>

using namespace std;

class AuDice;
class AuDNABase;

class AuSpecies
{
	QByteArray							m_Key;

public:	// Construction
	AuSpecies(const QByteArray& ba);
	AuSpecies();
	virtual ~AuSpecies();
	AuSpecies(const AuSpecies& c);

public:	// Operators
	inline const AuSpecies& operator=(const AuSpecies& c)
	{
		m_Key = c.m_Key;
		return *this;
	}

	inline bool operator==(const AuSpecies& c) const
	{
		return m_Key == c.m_Key;
	}

	inline bool operator!=(const AuSpecies& c) const
	{
		return m_Key != c.m_Key;
	}

	inline bool operator<(const AuSpecies& c) const
	{
		return m_Key < c.m_Key;
	}

	inline bool operator>(const AuSpecies& c) const
	{
		return m_Key > c.m_Key;
	}

	void operator<<(QDataStream& in);
	void operator>>(QDataStream& out) const;

public:	// Methods
	// Creates DNA from this species. Randomizes sizes to vary slightly.
	AuDNABase* CreateDNA() const;

	// Creates DNA from this species without randomizing.
	AuDNABase* CreateTypicalDNA() const;

	QString Description() const;

	ECOROLE EcoRole() const;

	inline bool IsNull() const
	{
		return m_Key.isEmpty();
	}

	bool IsSexuallyCompatible(const AuSpecies& species) const;

	inline const QByteArray& Key() const
	{
		return m_Key;
	}

	quint64 MemoryUsage() const;

	// This function gets the name from DgData, which means it calls LockDataForRead.
	QString Name() const;

	// This function gets the name from DgData, which means it calls LockDataForRead.
	// If the species has not been named, then Description is returned.
	QString NameOrDesc() const;

	// Configures this species to be some kind of rat.
	void RandomVermin(const AuDice& dice);

	static int RegisterMetaType();

	TOD TimeActive() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuSpecies& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuSpecies& o)
{
	o << in;
	return in;
}

class AuSpeciesHash
{
public:
	inline size_t operator()(const AuSpecies& o) const
	{
		return hash<unsigned int>()(qHash(o.Key()));
	}
};

Q_DECLARE_METATYPE(AuSpecies)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

#endif // AuSpecies_H
