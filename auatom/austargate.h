//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austargate.h,v 1.7 2022/02/19 17:32:50 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuStargate_H
#define AuStargate_H

#include "auid.h"
#include "aupoint.h"
#include "autime.h"
#include <QDataStream>
#include <QList>

class AuStargate
{
private:
	AuTime								m_Expiry;				// When the stargate expires.
	bool								m_Origin;				// true for origin, false for destination.
	AuId								m_ToGalaxy;				// Destination galaxy.
	AuPoint								m_ToLoc;				// Destination relative to the galaxy, in pc.

public:
	inline AuStargate(const AuId& toGalaxy, const AuPoint& toLoc, bool origin, const AuTime& expiry)
	{
		m_Expiry = expiry;
		m_Origin = origin;
		m_ToGalaxy = toGalaxy;
		m_ToLoc = toLoc;
	}

	inline AuStargate()
	{
		m_Origin = false;
	}

	inline AuStargate(const AuStargate& c)
	{
		m_Expiry = c.m_Expiry;
		m_Origin = c.m_Origin;
		m_ToGalaxy = c.m_ToGalaxy;
		m_ToLoc = c.m_ToLoc;
	}

public:
	const AuStargate& operator=(const AuStargate& c);

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:
	inline const AuTime& Expiry() const
	{
		return m_Expiry;
	}

	// Returns true when this stargate has expired, i.e. now is later than my expiration time.
	inline bool IsExpired()
	{
		return AuTime::Now() >= m_Expiry;
	}

	inline bool IsOrigin() const
	{
		return m_Origin;
	}

	inline const AuId& ToGalaxy() const
	{
		return m_ToGalaxy;
	}

	inline const AuPoint& ToGalaxyLoc() const
	{
		return m_ToLoc;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuStargate& stargate)
{
	stargate >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuStargate& stargate)
{
	stargate << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuStargateList : public QList<AuStargate>
{
public:
	inline bool Close(AuId fromGalaxy, const AuPoint& fromLoc)
	{
		bool anyClosed = false;
		iterator gi = begin();
		while(gi != end())
		{
			if((*gi).ToGalaxy() == fromGalaxy && (*gi).ToGalaxyLoc().FuzzyEquals(fromLoc))
			{
				gi = erase(gi);
				anyClosed = true;
			}
			else
				gi++;
		}
		return anyClosed;
	}

	// Removes stargates that have reached their expiry time.
	// 'closeOrigins' forces origin stargates to close. This happens when the shadow panels cut off the energy beams.
	// Returns 0 if no stargates are closed.
	// Returns 1 if any non-origin stargates are closed.
	// Returns 2 if any origin stargates are closed.
	int Decay(bool closeOrigins);

	inline bool HasDestination() const
	{
		for(const_iterator gi = cbegin(); gi != cend(); gi++)
			if(!(*gi).IsOrigin())
				return true;
		return false;
	}

	inline bool HasDestination(const AuId& toGalaxy, const AuPoint& toGalaxyLoc) const
	{
		for(const_iterator gi = cbegin(); gi != cend(); gi++)
			if(!(*gi).IsOrigin() &&
					(*gi).ToGalaxy() == toGalaxy &&
					(*gi).ToGalaxyLoc() == toGalaxyLoc)
				return true;
		return false;
	}

	inline bool HasOrigin() const
	{
		for(const_iterator gi = cbegin(); gi != cend(); gi++)
			if((*gi).IsOrigin())
				return true;
		return false;
	}

	quint64 MemoryUsage() const;

	inline void Open(const AuId& toGalaxy, const AuPoint& toLoc, bool origin, const AuTime& expiry)
	{
		append(AuStargate(toGalaxy, toLoc, origin, expiry));
	}

	inline AuTime PositiveStargateExpiry() const
	{
		for(const_iterator gi = cbegin(); gi != cend(); gi++)
			if((*gi).IsOrigin())
				return (*gi).Expiry();
		return AuTime();
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);
};

inline QDataStream& operator<<(QDataStream& out, const AuStargateList& stargates)
{
	stargates >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuStargateList& stargates)
{
	stargates << in;
	return in;
}


#endif // AuStargate_H
