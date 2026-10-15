//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubdpref.h,v 1.10 2022/02/19 18:18:54 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBdPref_H
#define AuBdPref_H

#include "dbindustry.h"
#include <QDataStream>
#include <QList>

class AuBlueprintPref
{
	quint32								m_BlueprintId;
	quint8								m_BuildEnvBits;			// BUILDING_ENVIRONMENT bits OR'd together

public:	// Constructors
	inline AuBlueprintPref(quint32 blueprintId, quint8 buildEnvBits)
	{
		m_BlueprintId = blueprintId;
		m_BuildEnvBits = buildEnvBits;
	}

	inline AuBlueprintPref()
	{
		m_BlueprintId = 0;
		m_BuildEnvBits = 0;
	}

	inline AuBlueprintPref(const AuBlueprintPref& c)
	{
		m_BlueprintId = c.m_BlueprintId;
		m_BuildEnvBits = c.m_BuildEnvBits;
	}

public:	// Operators
	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

	inline bool operator==(const AuBlueprintPref& c) const
	{
		if(m_BlueprintId != c.m_BlueprintId)
			return false;
		if(m_BuildEnvBits != c.m_BuildEnvBits)
			return false;
		return true;
	}

	inline bool operator!=(const AuBlueprintPref& c) const
	{
		return !operator==(c);
	}

	inline const AuBlueprintPref& operator=(const AuBlueprintPref& c)
	{
		m_BlueprintId = c.m_BlueprintId;
		m_BuildEnvBits = c.m_BuildEnvBits;
		return *this;
	}

public:	// Methods
	inline quint32 BlueprintId() const
	{
		return m_BlueprintId;
	}

	inline quint8 BuildEnvironmentBits() const
	{
		return m_BuildEnvBits;
	}

	// Returns true if this blueprint can be built in any of the 'envBits' BUILDING_ENVIRONMENT bits.
	inline bool BuildEnvironmentOk(quint8 envBits) const
	{
		return m_BuildEnvBits & envBits;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuBlueprintPref& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuBlueprintPref& c)
{
	c << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuBdPref
{
	QList<AuBlueprintPref>				m_Blueprints;			// List of blueprints in order of preference.

public:	// Constructors
	inline AuBdPref()
	{
	}

	inline AuBdPref(const AuBdPref& c) : m_Blueprints(c.m_Blueprints)
	{
	}

public:	// Operators
	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

	// A convenient way to add a blueprint to this list of preferences.
	inline void operator<<(const AuBlueprintPref bp)
	{
		AddBlueprint(bp);
	}

	inline const AuBdPref& operator=(const AuBdPref& c)
	{
		m_Blueprints = c.m_Blueprints;
		return *this;
	}

	inline bool operator==(const AuBdPref& c) const
	{
		if(m_Blueprints != c.m_Blueprints)
			return false;
		return true;
	}

	inline bool operator!=(const AuBdPref& c) const
	{
		return !operator==(c);
	}

public:	// Methods
	// Adds 'bp' to the list of preferred blueprints. Moves it to the end if it is already in the list.
	inline void AddBlueprint(const AuBlueprintPref& bp)
	{
		RemoveBlueprint(bp);
		if(m_Blueprints.size() < 0xff)	// Don't let some wise guy overrun us with too many blueprints for our streaming operators.
			m_Blueprints.append(bp);
	}

	inline const QList<AuBlueprintPref>& Blueprints() const
	{
		return m_Blueprints;
	}

	// Returns combined build environment bits of all preferred blueprints.
	quint8 BuildEnvironmentBits() const;

	// Removes blueprints that cannot be built in any of 'envBits' BUILDING_ENVIRONMENT bits.
	void FilterEnvironment(quint8 envBits);

	// Returns true if all blueprint preferences enable construction in all environments.
	inline bool HasAllEnvironments() const
	{
		return BuildEnvironmentBits() == BENV_All;
	}

	void RemoveBlueprint(quint32 blueprintId);

	inline void RemoveBlueprint(const AuBlueprintPref& bp)
	{
		RemoveBlueprint(bp.BlueprintId());
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuBdPref& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuBdPref& c)
{
	c << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuBdPrefs
{
	QHash<INDUSTRYID, AuBdPref>			m_Prefs;				// Blueprint preferences for every industry.

public:	// Constructors
	inline AuBdPrefs()
	{
	}

	inline AuBdPrefs(const AuBdPrefs& c) : m_Prefs(c.m_Prefs)
	{
	}

public:	// Operators
	inline const AuBdPrefs& operator=(const AuBdPrefs& c)
	{
		m_Prefs = c.m_Prefs;
		return *this;
	}

	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:	// Methods
	// Returns true if sufficient blueprint preferences exist for a pirate base in every environment, ignoring defaults.
	// 'baseBuildings' is the list of buildings preferred by the pirate empire.
	bool CanMakePirateBases(const QList<INDUSTRYID>& baseBuildings) const;

	inline const QHash<INDUSTRYID, AuBdPref>& PreferredBlueprints() const
	{
		return m_Prefs;
	}

	// A default AuBdPref() is returned if no preference has been configured.
	AuBdPref PreferredBlueprints(INDUSTRYID industryId) const;

	// Returns PreferredBlueprints for 'industryId' that can be built in any of the 'envBits' BUILDING_ENVIRONMENT bits.
	AuBdPref PreferredBlueprints(INDUSTRYID industryId, quint8 envBits) const;

	void SetPreferredBlueprints(INDUSTRYID industryId, const AuBdPref& pref);
};

inline QDataStream& operator<<(QDataStream& out, const AuBdPrefs& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuBdPrefs& c)
{
	c << in;
	return in;
}

#endif // AuBdPref_H
