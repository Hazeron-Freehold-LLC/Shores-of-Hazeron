//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aufogofwar.h,v 1.4 2022/05/17 18:24:15 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuFogOfWar_H
#define AuFogOfWar_H

#include "auelapsedtimer.h"
#include "auid.h"
#include "aupoint.h"
#include <QHash>
#include <QList>

class AuFogOfWarData
{
	QList<AuPoint>						m_Fog;					// Unexplored sea level locs. Empty is fully explored.
	AuElapsedTimer						m_LastChange;			// Last change time, for tx. Not serialized.

public:
	AuFogOfWarData();
	AuFogOfWarData(const AuPointArray& fog);

public:
	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

	bool operator==(const AuFogOfWarData& c) const;

	inline bool operator!=(const AuFogOfWarData& c) const
	{
		return !operator==(c);
	}

public:
	// Combines 'fog' with this data. Only retains the intersecting cells.
	// Removes my cells that do not also exist in 'fog'.
	// Returns true if any data changes.
	bool Combine(const AuFogOfWarData& fog);

	inline bool Contains(const AuPoint& seaLevelLoc) const
	{
		return m_Fog.contains(seaLevelLoc);
	}

	// Returns true if any data changes.
	bool Explore(const AuPoint& seaLevelLoc, double sightRadius);

	// Returns true if any data changes.
	bool ExploreAll();

	inline const QList<AuPoint>& Fog() const
	{
		return m_Fog;
	}

	// Returns true if 'seaLevelLoc' is explored.
	bool IsExplored(const AuPoint& seaLevelLoc) const;

	inline const AuElapsedTimer& LastChanged() const
	{
		return m_LastChange;
	}

	// Radius of an unknown location.
	static double Radius();
};

inline QDataStream& operator<<(QDataStream& out, const AuFogOfWarData& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuFogOfWarData& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Data is stored per avatar only for those who have no empire. Otherwise the volume of data created could be very large, especially
// for ringworlds.

class AuFogOfWar
{
	QHash<AuId, AuFogOfWarData>			m_DataByAvatar;			// Avatars without empires who have personally explored.
	QHash<quint32, AuFogOfWarData>		m_DataByEmpire;			// Empires of avatars who have explored.

public:
	AuFogOfWar();
	~AuFogOfWar();
	AuFogOfWar(const AuFogOfWar& c);

public:
	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:
	// Removes any data associated with 'empireId'.
	// Returns true if any data changes.
	bool EmpireDeleted(quint32 empireId);

	// Explore on behalf of 'avatarId' or 'empireId'.
	// 'avatarId' may be null.
	// 'empireId' may be 0.
	// Both null does nothing.
	// Both not null causes 'avatarId's data to be combined with 'empireId's data. Then 'avatarId's data is removed.
	// Clears all fog within 'sightRadius' of 'seaLevelLoc'.
	// Returns true if any data changes.
	bool Explore(const AuId& avatarId, quint32 empireId, const AuPoint& seaLevelLoc, double sightRadius);

	// Explore on behalf of 'avatarId' or 'empireId'.
	// 'avatarId' may be null.
	// 'empireId' may be 0.
	// Both null does nothing.
	// Both not null causes 'avatarId's data to be combined with 'empireId's data. Then 'avatarId's data is removed.
	// Clears all fog.
	// Returns true if any data changes.
	bool ExploreAll(const AuId& avatarId, const quint32 empireId);

	// Retrieves fog of war data for an 'empireId', or an 'avatarId' if 'empireId' is 0.
	void GetData(const AuId& avatarId, const quint32 empireId, AuFogOfWarData& fow) const;

	// Used to decide whether to call Tx this time.
	bool HasChanged(const AuId& avatarId, const quint32 empireId, const AuElapsedTimer& tmLastKnownChange) const;

	// Initialize for an 'avatarId' or 'empireId'. Skips existing if already present.
	// 'avatarId' may be null,
	// 'empireId' may be 0.
	// Both null does nothing.
	// Both not null causes 'avatarId's data to be combined with 'empireId's data. Then 'avatarId's data is removed.
	// 'fogLocations' are the initial sea level locations for a given world.
	void Init(const AuId& avatarId, quint32 empireId, const AuPointArray& fog);

	// Returns true if 'seaLevelLoc' is explored by 'empireId', or by 'avatarId' if 'empireId' is null.
	bool IsExplored(const AuId& avatarId, quint32 empireId, const AuPoint& seaLevelLoc) const;

	// Returns true if Init must be called before exploring with 'avatarId' and 'empireId'.
	bool NeedsInit(const AuId& avatarId, quint32 empireId) const;

	void Rx(QDataStream& fromServer);

	void Tx(QDataStream& toClient, const AuId& avatarId, quint32 empireId);

protected:
	// Combines 'avatarId's data with 'empireId's data. Then removes 'avatarId's data.
	// Returns true if any data changes.
	bool Combine(const AuId& avatarId, quint32 empireId);
};

inline QDataStream& operator<<(QDataStream& out, const AuFogOfWar& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuFogOfWar& o)
{
	o << in;
	return in;
}

#endif // AuFogOfWar_H

