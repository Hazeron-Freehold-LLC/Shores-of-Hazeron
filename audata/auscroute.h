//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscroute.h,v 1.5 2022/02/19 18:19:18 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScRoute_H
#define AuScRoute_H

#include "audata.h"
#include "aupoint.h"
#include <QDataStream>

class AuDoorStateHash;
class QDebug;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// A location or waypoint along a route through an AuScDesign.

class AUDATA_DLL AuScRouteLoc : public AuPoint
{
private:
	quint16								m_DoorId;				// Part id in AuScParts of design. 0 if none.
	bool								m_Ladder;				// Movement to this loc requires a laddder.
	quint16								m_LiftId;				// Part id in AuScParts of design. 0 if none.
	QString								m_LiftStop;				// Movement requires a lift to be moved to a stop.

public:	// Constructors
	inline AuScRouteLoc(const AuPoint& loc) : AuPoint(loc)
	{
		m_DoorId = 0;
		m_Ladder = false;
		m_LiftId = 0;
	}

	inline AuScRouteLoc()
	{
		m_DoorId = 0;
		m_Ladder = false;
		m_LiftId = 0;
	}

	inline AuScRouteLoc(const AuScRouteLoc& c) : AuPoint(c)
	{
		m_DoorId = c.m_DoorId;
		m_Ladder = c.m_Ladder;
		m_LiftId = c.m_LiftId;
		m_LiftStop = c.m_LiftStop;
	}

public:	// Methods
	inline quint16 DoorId() const
	{
		return m_DoorId;
	}

	// Returns true if 'loc' has arrived at this route location.
	bool HasArrived(const AuPoint& loc, double cgHt, const AuPoint& upVec, quint16 roomId, const QString& atStop,
			bool climbingLadder) const;

	inline bool IsDoor() const
	{
		return m_DoorId;
	}

	inline bool IsLadder() const
	{
		return m_Ladder;
	}

	inline bool IsLift() const
	{
		return m_LiftId;
	}

	inline quint16 LiftId() const
	{
		return m_LiftId;
	}

	inline const QString& LiftStop() const
	{
		return m_LiftStop;
	}

	inline const AuPoint& Loc() const
	{
		return *this;
	}

	inline void SetDoorId(quint16 doorId)
	{
		m_DoorId = doorId;
	}

	inline void SetLift(quint16 liftId, const QString& liftStop)
	{
		m_LiftId = liftId;
		m_LiftStop = liftStop;
	}

	inline void SetLadder(bool isLadder)
	{
		m_Ladder = isLadder;
	}

	inline void SetLoc(const AuPoint& loc)
	{
		AuPoint::operator=(loc);
	}
};

AUDATA_DLL QDebug operator<<(QDebug dbg, const AuScRouteLoc& c);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// A route comprised of one or more AuScRouteLoc's, used by new style AuScDesign designs.

class AUDATA_DLL AuScRoute : public QList<AuScRouteLoc>
{
public:	// Constructors
	inline AuScRoute()
	{
	}

	AuScRoute(const AuScRoute& c);

public:	// Operators

public:	// Methods
	bool DistanceLessThan(const AuPoint& startLoc, double distance) const;

	inline bool FirstIsDoor() const
	{
		if(empty())
			return false;
		return first().IsDoor();
	}

	inline bool FirstIsLadder() const
	{
		if(empty())
			return false;
		return first().IsLadder();
	}

	inline const AuPoint& FirstLoc() const
	{
		return first();
	}

	inline quint16 FirstDoorId() const
	{
		if(empty())
			return 0;
		return first().DoorId();
	}

	inline quint16 FirstLiftId() const
	{
		if(empty())
			return 0;
		return first().LiftId();
	}

	inline QString FirstLiftStop() const
	{
		if(empty())
			return QString();
		return first().LiftStop();
	}

	inline const AuPoint& LastLoc() const
	{
		return last();
	}

	// 'loc' is the current location of the person following this route.
	// 'cgHt' is their ground height above CG.
	// 'upVec' is their body's current up vector.
	// 'roomId' is the id of their current room.
	// 'atStop' is the stop name of their current turbo lift, if they are in one.
	// 'climbingLadder' is true if they are climbing a ladder.
	// 'touchingGround' is true if they are touching dirt, not the floor of the vessel.
	// 'doorStates' are the current states of all doors in the vessel.
	bool Update(const AuPoint& loc, double cgHt, const AuPoint& upVec, quint16 roomId, const QString& atStop,
			bool climbingLadder, bool touchingGround, const AuDoorStateHash& doorStates);
};

AUDATA_DLL QDebug operator<<(QDebug dbg, const AuScRoute& c);

#endif // AuScRoute_H
