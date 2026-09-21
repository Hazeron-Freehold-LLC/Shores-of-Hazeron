//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscpref.h,v 1.8 2022/02/19 18:19:16 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScPref_H
#define AuScPref_H

#include "dbcommodity.h"
#include <QDataStream>
#include <QList>

class AuScPref
{
	QList<quint32>						m_Blueprints;			// List of blueprints in order of preference.

public:	// Constructors
	inline AuScPref()
	{
	}

	inline AuScPref(const AuScPref& c) : m_Blueprints(c.m_Blueprints)
	{
	}

public:	// Operators
	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

	// A convenient way to add a blueprint to this list of preferences.
	inline void operator<<(quint32 blueprintId)
	{
		AddBlueprint(blueprintId);
	}

	inline const AuScPref& operator=(const AuScPref& c)
	{
		m_Blueprints = c.m_Blueprints;
		return *this;
	}

	inline bool operator==(const AuScPref& c) const
	{
		if(m_Blueprints != c.m_Blueprints)
			return false;
		return true;
	}

	inline bool operator!=(const AuScPref& c) const
	{
		return !operator==(c);
	}

public:	// Methods
	// Adds 'blueprintId' to the list of preferred blueprints. Moves it to the end if it is already in the list.
	inline void AddBlueprint(quint32 blueprintId)
	{
		RemoveBlueprint(blueprintId);
		if(m_Blueprints.size() < 0xff)	// Don't let some wise guy overrun us with too many blueprints for our streaming operators.
			m_Blueprints.append(blueprintId);
	}

	inline const QList<quint32>& Blueprints() const
	{
		return m_Blueprints;
	}

	inline void RemoveBlueprint(quint32 blueprintId)
	{
		int idx = m_Blueprints.indexOf(blueprintId);
		if(idx >= 0)
			m_Blueprints.removeAt(idx);
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuScPref& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScPref& c)
{
	c << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuScPrefs
{
public:
	enum SCROLE
	{
		SCROLE_Station,
		SCROLE_Explorer,
		SCROLE_Freighter,
		SCROLE_Military,
		SCROLE_Count
	};

private:
	QHash<SCROLE, AuScPref>				m_Prefs;				// Blueprint preferences for every SCROLE.

public:	// Constructors
	inline AuScPrefs()
	{
	}

	inline AuScPrefs(const AuScPrefs& c) : m_Prefs(c.m_Prefs)
	{
	}

public:	// Operators
	inline const AuScPrefs& operator=(const AuScPrefs& c)
	{
		m_Prefs = c.m_Prefs;
		return *this;
	}

	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:	// Methods
	// Returns true if all SCROLEs have a blueprint preference, ignoring defaults.
	bool CanMakePirateEncounters() const;

	inline const QHash<SCROLE, AuScPref>& PreferredBlueprints() const
	{
		return m_Prefs;
	}

	// Returns configured preferences for 'role'.
	// A default AuScPref() is returned if no preference has been configured.
	AuScPref PreferredBlueprints(SCROLE role) const;

	static QString RoleName(SCROLE role)
	{
		if(role < 0 || role >= SCROLE_Count)
			return "";
		QString roleNames[SCROLE_Count] =
		{
			QObject::tr("Space Station"),
			QObject::tr("Explorer Spacecraft"),
			QObject::tr("Freigher Spacecraft"),
			QObject::tr("Military Spacecraft"),
		};
		return roleNames[role];
	}

	void SetPreferredBlueprints(SCROLE role, const AuScPref& pref);
};

inline QDataStream& operator<<(QDataStream& out, const AuScPrefs& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScPrefs& c)
{
	c << in;
	return in;
}

#endif // AuScPref_H
