//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscroute.cpp,v 1.8 2022/02/19 18:20:19 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscroute.h"
//
#include "aumatrix4x4.h"
#include "auscreport.h"
#include <QDebug>

QDebug operator<<(QDebug dbg, const AuScRouteLoc& c)
{
	dbg.nospace() << c.Loc() << "door" << c.DoorId() << "ladder" << c.IsLadder() << "lift" << c.LiftId() << c.LiftStop();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Returns true if 'loc' has arrived at this route location.
bool AuScRouteLoc::HasArrived(const AuPoint& loc, double cgHt, const AuPoint& upVec, quint16 roomId, const QString& atStop,
		bool climbingLadder) const
{
	if(m_LiftId)
	{
		// The route location is in a lift.
		if(m_LiftId != roomId)
			return false;				// They are not in the lift.
		if(m_LiftStop != atStop)
			return false;				// The lift is not at the correct stop.
		return true;
	}

	if(m_Ladder || climbingLadder)
	{
		// Check proximity to the path location.
		const double ladderTol2 = 1*1;	// squared closeness tolerance for waypoints.
		return Distance2(loc) < ladderTol2;
	}
	else
	{
		// Check proximity to a segment from their feet to their current location.
		AuPoint wayBottom(loc - upVec * cgHt);
		AuPoint wayTop(loc + upVec * FeetPerMeter());

		const double walkTol2 = 2*2;	// squared closeness tolerance for waypoints.
		return DistToSegment2(wayBottom, wayTop) < walkTol2;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScRoute::AuScRoute(const AuScRoute& c) : QList<AuScRouteLoc>(c)
{
}

bool AuScRoute::DistanceLessThan(const AuPoint& startLoc, double distance) const
{
	// Init the return value.
	double routeDist = 0;

	// Start measuring the route distance from 'startLoc'.
	AuPoint curLoc(startLoc);

	// Accumulate the distance from route point to route point.
	for(const_iterator ri = cbegin(); ri != cend(); ri++)
	{
		// Add the straight-line distance from route point to route point.
		routeDist += curLoc.Distance(*ri);

		// Stop when we have exceeded the distance queried.
		if(routeDist >= distance)
			return false;

		// Prepare for the next route segment, if any.
		curLoc = *ri;
	}

	// We get here if the total route distance is less than the distance queried.
	return true;
}

bool AuScRoute::Update(const AuPoint& loc, double cgHt, const AuPoint& upVec, quint16 roomId, const QString& atStop,
		bool climbingLadder, bool touchingGround, const AuDoorStateHash& doorStates)
{
	if(empty())
		return false;

	// Do not remove the first waypoint if it is a door that is closed. If we remove the waypoint too soon, the AI won't know
	// there is a door to open.
	if(first().IsDoor())
	{
		AuDoorStateHash::const_iterator di = doorStates.find(first().DoorId());
		if(di != doorStates.cend() && (*di).IsClosed())
			return false;
	}

	// Do not skip ladders until the caller is climbing a ladder or they won't know there is a ladder to climb.
	if(!climbingLadder && first().IsLadder())
		return false;

	// If the caller is touching the ground, specifically dirt not deck, don't let the next route point be below them.
	if(touchingGround)
	{
		double rz = first().Dot(upVec);	// Altitude of route point, relative to upVec.
		double lz = loc.Dot(upVec);		// Altitude of caller's loc, relative to upVec.
		if(rz < lz)
			*begin() += upVec * (lz - rz);	// Move up by the difference in altitudes; it is already at altitude rz.
	}

	// Have we arrived at the first path location?
	if(first().HasArrived(loc, cgHt, upVec, roomId, atStop, climbingLadder))
	{
		removeFirst();
		return true;
	}

	return false;
}

QDebug operator<<(QDebug dbg, const AuScRoute& c)
{
	dbg.nospace() << "ScRoute steps " << c.size();
	for(QList<AuScRouteLoc>::const_iterator ri = c.cbegin(); ri != c.cend(); ri++)
		dbg.nospace() << "\n  " << *ri;
	return dbg.space();
}
