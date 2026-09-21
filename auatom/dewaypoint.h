//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dewaypoint.h,v 1.8 2022/02/19 17:35:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DeWaypoint_H
#define DeWaypoint_H

#include "auid.h"
#include "aupoint.h"

class DaAtom;

class DeWaypoint
{
public:
	AuId			m_idSystem;			// Id of the destination solar system atom.
private:
	AuId			m_idDestination;	// Id of the destination atom.
	AuPoint			m_fLocation;		// Location relative to the destination atom.

public:	// Constructors
	inline DeWaypoint()
	{
	}

	inline DeWaypoint(const AuId& idDest, const AuId& idParentSystem)
	{
		m_idDestination = idDest;
		m_idSystem = idParentSystem;
	}

	inline DeWaypoint(const AuId& idDest, const AuId& idParentSystem, const AuPoint& fLoc)
	{
		m_idDestination = idDest;
		m_idSystem = idParentSystem;
		m_fLocation = fLoc;
	}

public:	// Operators
	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

	bool operator==(const DeWaypoint& c) const;
	inline bool operator!=(const DeWaypoint& c) const
	{
		return !operator==(c);
	}

public:	// Methods
	inline const AuId& Destination() const
	{
		return m_idDestination;
	}

	inline const AuPoint& DestinationOffset() const
	{
		return m_fLocation;
	}

	const DaAtom* Location(const DaAtom* pParent, AuPoint& fRelLoc) const;

public:
	void Rx(QDataStream& in);
	void Tx(QDataStream& out) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class DeWaypointRoute : public QVector<DeWaypoint>
{
public:	// Operators
	void operator>>(QDataStream& out) const;
	void operator<<(QDataStream& in);

public:
	void AddDestination(const AuId& idDest, const AuId& idParentSystem);
	void AddDestination(const AuId& idDest, const AuPoint& fRelLoc, const AuId& idParentSystem);
	void AddDestination(const DeWaypoint& wp);
	void RemoveFirst();

	void Rx(QDataStream& in);
	void Tx(QDataStream& out) const;
};

inline QDataStream& operator<<(QDataStream& out, const DeWaypointRoute& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, DeWaypointRoute& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // DeWaypoint_H
