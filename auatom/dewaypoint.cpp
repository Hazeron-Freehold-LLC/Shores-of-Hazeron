//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dewaypoint.cpp,v 1.17 2022/02/19 17:44:17 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dewaypoint.h"
//
#include "dasolarsystem.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

// Sets this waypoint's location relative to a parent atom into 'fRelLoc'.
// Returns the Atom pointer to the destination atom.
// Returns NULL if the waypoint's destination is not subordinate to 'pParent'.
const DaAtom *DeWaypoint::Location(const DaAtom* pParent, AuPoint& fRelLoc) const
{
	// Return false if this waypoint is not even in the system.
	if(!pParent->Is(m_idSystem))
		return NULL;

	if(pParent->Is(m_idDestination))
	{
		// I AM the destination atom.
		fRelLoc = m_fLocation;
		return pParent;
	}

	const DaAtom* pwp = pParent->FindIdRecursive(m_idDestination);
	if(pwp)
	{
		// Found destination atom subordinate to pParent.
		fRelLoc = pwp->EnvTransform().Project(m_fLocation);
		return pwp;
	}
	return 0;
}

bool DeWaypoint::operator==(const DeWaypoint &c) const
{
	if (m_idDestination != c.m_idDestination)
		return false;
	if (m_idSystem != c.m_idSystem)
		return false;
	if (m_fLocation != c.m_fLocation)
		return false;
	return true;
}

void DeWaypoint::Rx(QDataStream& in)
{
	m_idSystem << in;
	m_idDestination << in;
	double x,y,z;
	in >> x;
	in >> y;
	in >> z;
	m_fLocation = AuPoint(x,y,z);
}

void DeWaypoint::Tx(QDataStream& out) const
{
	m_idSystem >> out;
	m_idDestination >> out;
	out << m_fLocation.x;
	out << m_fLocation.y;
	out << m_fLocation.z;
}

void DeWaypoint::operator<<(QDataStream& in)
{
	m_idSystem << in;
	m_idDestination << in;
	double x,y,z;
	in >> x;
	in >> y;
	in >> z;
	m_fLocation = AuPoint(x,y,z);
}

void DeWaypoint::operator>>(QDataStream& out) const
{
	m_idSystem >> out;
	m_idDestination >> out;
	out << m_fLocation.x;
	out << m_fLocation.y;
	out << m_fLocation.z;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

// Adds a waypoint to a given destination id.
// A waypoint is not added if it duplicates the last waypoint already in the route.
void DeWaypointRoute::AddDestination(const AuId &idDest, const AuId &idParentSystem)
{
	DeWaypoint wp(idDest, idParentSystem);
	AddDestination(wp);
}

// Adds a waypoint to a given destination id and relative offset from that destination.
// A waypoint is not added if it duplicates the last waypoint already in the route.
void DeWaypointRoute::AddDestination(const AuId &idDest, const AuPoint &fRelLoc, const AuId &idParentSystem)
{
	DeWaypoint wp(idDest, idParentSystem, fRelLoc);
	AddDestination(wp);
}

// Adds a waypoint to this route.
// A waypoint is not added if it duplicates the last waypoint already in the route.
void DeWaypointRoute::AddDestination(const DeWaypoint &wp)
{
	if (count() == 0 || wp != last())
	{
		append(wp);
	}
}

// Removes the first waypoint from the route.
void DeWaypointRoute::RemoveFirst()
{
	DeWaypointRoute nuroute;
	DeWaypointRoute::iterator it;
	for(it = begin()+1; it != end(); ++it)
		nuroute.append(*it);
	operator=(nuroute);	// replace the current route, minus the first waypoint
}

// Load this array from the database.
/*
void DeWaypointRoute::Load(QSqlDatabase& db, int idOwner)
{
	clear();

	const char* sTableName = "e_Waypoint";
	QString sq;
	sq.sprintf("SELECT sequence,idss,idd,locX,locY,locZ FROM %s WHERE ids=%d;",
			sTableName, idOwner);
	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		unsigned short int sequence = qa.value(0).toUInt();	// Array index.

		unsigned int idss = qa.value(1).toUInt();
		unsigned int idd = qa.value(2).toUInt();
		double x = qa.value(3).toDouble();
		double y = qa.value(4).toDouble();
		double z = qa.value(5).toDouble();

		AuId uidd(idd);
		AuId uidss(idss);
		DeWaypoint way(uidd, uidss, AuPoint(x,y,z));

		// Grow the array if necessary to fit the waypoint at its desired index.
		if(sequence >= count())
			resize(sequence+1);

		(*this)[sequence] = way;
	}
}
*/

/*
// Store this array into the database.
void DeWaypointRoute::Store(QSqlDatabase& db, int idOwner)
{
	// Store the individual items.
	DeWaypoint *pi = begin();
	for (int sequence=0; sequence<count(); sequence++, pi++)
		pi->Store(db, idOwner, sequence);
}
*/

void DeWaypointRoute::Rx(QDataStream& in)
{
	quint16 count;
	in >> count;
	clear();
	while(count)
	{
		DeWaypoint wp;
		wp.Rx(in);
		append(wp);
		count--;
	}
}

void DeWaypointRoute::Tx(QDataStream& out) const
{
	out << (quint16)count();
	const DeWaypoint* pi = constBegin();
	for (int sequence=0; sequence<count(); sequence++, pi++)
		pi->Tx(out);
}

void DeWaypointRoute::operator<<(QDataStream& in)
{
	quint16 count;
	in >> count;
	clear();
	while(count)
	{
		DeWaypoint wp;
		wp << in;
		append(wp);
		count--;
	}
}

void DeWaypointRoute::operator>>(QDataStream& out) const
{
	Q_ASSERT(count() <= 0xffff);
	out << (quint16)count();
	for(DeWaypointRoute::const_iterator pi = constBegin(); pi != constEnd(); pi++)
		(*pi) >> out;
}
