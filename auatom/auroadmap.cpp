//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auroadmap.cpp,v 1.7 2022/02/19 17:40:55 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auroadmap.h"
//
#include "daworldll.h"

AuRoadSegment::AuRoadSegment(TYPE type, const AuGlobeLLF& startLL, double startMSL, const AuGlobeLLF& endLL, double endMSL,
		float width, const AuId& atomId)
{
	m_EndLL = endLL;
	m_EndMSL = endMSL;
	m_Id = atomId;
	m_StartLL = startLL;
	m_StartMSL = startMSL;
	m_Type = type;
	m_Width = width;
}

bool AuRoadSegment::TestConnection(const DaWorldLL& world, const AuRoadSegment& road) const
{
	const double roadConRange = 10*FeetPerMeter();								// Margin distance permitted between the two roads.
	// Test to see if this road segment contains the 'road'.
	if(road.m_StartLL == road.m_EndLL)
	{
		if(Contains(world, road.m_StartLL, road.m_Width/2 + roadConRange))		// This segment is close to the start of the road.
			return true;
	}
	else
	{
		if(Contains(world, road.m_StartLL, road.m_Width/2 + roadConRange) ||	// This segment is close to the start of the road.
				Contains(world, road.m_EndLL, road.m_Width/2 + roadConRange))	// This segment is close to the end of the road.
			return true;
	}

	// Test to see if 'road' contains this road segment.
	if(m_StartLL == m_EndLL)
	{
		if(road.Contains(world, m_StartLL, m_Width/2 + roadConRange))			// The road is close to the start of this segment.
			return true;
	}
	else
	{
		if(road.Contains(world, m_StartLL, m_Width/2 + roadConRange) ||			// The road is close to the start of this segment.
				road.Contains(world, m_EndLL, m_Width/2 + roadConRange))		// The road is close to the end of this segment.
			return true;
	}
	return false;
}

bool AuRoadSegment::Contains(const DaWorldLL& world, const AuGlobeLLF& ll, double searchRadius) const
{
	return Contains(world, ll, m_StartLL, m_StartMSL, m_EndLL, m_EndMSL, m_Width/2, searchRadius);
}

//static
bool AuRoadSegment::Contains(const DaWorldLL& world, const AuGlobeLLF& ll, const AuGlobeLLF& startLL, float startMSL,
		const AuGlobeLLF& endLL, float endMSL, float halfWidth, double searchRadius)
{
	float segFrac;
	AuGlobeLLF segLL = ll.CoordOnSegment(startLL, endLL, &segFrac, world.LLShape());
	AuPoint segLoc;
	// The ends are allowed to have a radius so drawing near the end of a road doesn't seem strange.
	double segAlt;
	if(segFrac <= 0)
	{
		// Closest point is the starting location.
		segAlt = startMSL;
		segLoc = world.LocOfLL(startLL, startMSL);
	}
	else if(segFrac >= 1)
	{
		// Closest point is the ending location.
		segAlt = endMSL;
		segLoc = world.LocOfLL(endLL, segAlt);
	}
	else
	{
		// Calculate the location along this road segment.
		segAlt = startMSL + (endMSL - startMSL) * segFrac;
		segLoc = world.LocOfLL(segLL, segAlt);
	}

	// Calculate the location of ll at the altitude of the segment location.
	AuPoint llLoc = world.LocOfLL(ll, segAlt);

	// Determine if the search location is within the build radius of the development, at the altitude of the development.
	float dist2 = (float)llLoc.Distance2(segLoc);
	float bldRad = halfWidth;
	if(segFrac >= 0 && segFrac <= 1)
	{
		// The point is between my end points.
		bldRad += searchRadius;
		float bldRad2 = bldRad * bldRad;						// squared
		if(dist2 <= bldRad2)
			return true;										// Location is within my paved area.
	}
	else if(searchRadius > 0)
	{
		// Point is not between my end points and the radius requires me to calculate how far off the end of the segment it is.
		// This is a lot of work. Luckily, it only happens when drawing roads and buildings.
		double length = Length(world, startLL, startMSL, endLL, endMSL);	// Length of this road segment.
		double endDist;
		if(segFrac < 0)
			endDist = -segFrac*length;							// Distance from the start location along the segment vector.
		else
			endDist = segFrac*length - length;					// Distance from the end location along the segment vector.
		if(endDist <= searchRadius)
		{
			// The distance is within the search radius of a segment end, along the segment vector. It's on the pavement if the
			// lateral distance from the center line is within the build radius.
			AuPoint vup = -world.GravityDir(segLoc);			// Unit vector up at segLoc, which is one of the ends.
			AuPoint vseg;
			if(segFrac < 0)
			{
				// segLoc is already calculated at the start of the road segment. Calculate the other end.
				AuPoint endLoc = world.LocOfLL(endLL, endMSL);
				vseg = endLoc - segLoc;
			}
			else
			{
				// segLoc is already calculated at the end of the road segment. Calculate the other end.
				AuPoint startLoc = world.LocOfLL(startLL, startMSL);
				vseg = segLoc - startLoc;
			}
			vseg.Unit();										// Unit vector parallel to the road segment.
			AuPoint vright = vseg.Cross(vup);
			vright.Unit();										// Lateral unit vector, perpendicular to the up and segment vectors.

			AuPoint llseg = llLoc - segLoc;						// Vector to ll from end of segment.
			double latdist = llseg.Dot(vright);					// Lateral distance from segment center line.
			if(fabs(latdist) <= bldRad + searchRadius)
				return true;									// It's between the road edge lines.

			// The pain we have to check the radius to the nearest corner. D:
			AuPoint cornLoc;
			if(latdist > 0)
				cornLoc = segLoc + vright*bldRad;
			else
				cornLoc = segLoc - vright*bldRad;

			double cornDist2 = cornLoc.Distance2(llLoc);
			float radius2 = searchRadius * searchRadius;		// squared
			if(cornDist2 <= radius2)
				return true;									// It's in the radial corner zone.
		}
	}
	return false;
}

//static
double AuRoadSegment::Length(const DaWorldLL& world, const AuGlobeLLF& startLL, float startMSL, const AuGlobeLLF& endLL,
		float endMSL)
{
	AuPoint startLoc = world.LocOfLL(startLL, startMSL);
	AuPoint endLoc = world.LocOfLL(endLL, endMSL);
	return startLoc.Distance(endLoc);
}

void AuRoadSegment::Dump() const
{
	switch(m_Type)
	{
	case Bd:
		qDebug() << "  Bd" << m_Id << "at" << m_StartLL << m_StartMSL << "width" << m_Width;
		break;
	case GeoRoad:
		qDebug() << "  GeoRoad" << m_Id << "from" << m_StartLL << m_StartMSL << "to" << m_EndLL << m_EndMSL << "width" << m_Width;
		break;
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuRoadNetwork::AuRoadNetwork()
{
}

bool AuRoadNetwork::TestConnection(const DaWorldLL& world, const AuRoadSegment& road) const
{
	for(QList<AuRoadSegment>::const_iterator ri = m_Roads.cbegin(); ri != m_Roads.cend(); ri++)
		if((*ri).TestConnection(world, road))
			return true;
	return false;
}

void AuRoadNetwork::Add(const AuRoadSegment& road)
{
	// Insure we don't already have an exact duplicate of the nu road.
	for(QList<AuRoadSegment>::const_iterator ri = m_Roads.cbegin(); ri != m_Roads.cend(); ri++)
		if(*ri == road)
			return;
	m_Roads.append(road);
}

void AuRoadNetwork::Add(const AuRoadNetwork& net)
{
	for(QList<AuRoadSegment>::const_iterator ri = net.m_Roads.cbegin(); ri != net.m_Roads.cend(); ri++)
		Add(*ri);
}

void AuRoadNetwork::Connections(QSet<AuId>& connectedBdsAndCities) const
{
	for(QList<AuRoadSegment>::const_iterator ri = m_Roads.cbegin(); ri != m_Roads.cend(); ri++)
		if((*ri).IsBd())
			connectedBdsAndCities.insert((*ri).Id());
}

bool AuRoadNetwork::Contains(const AuId& id) const
{
	for(QList<AuRoadSegment>::const_iterator ri = m_Roads.cbegin(); ri != m_Roads.cend(); ri++)
		if((*ri).Id() == id)
			return true;
	return false;
}

void AuRoadNetwork::Dump() const
{
	qDebug() << " Network of" << m_Roads.size() << "roads/buildings";
	for(QList<AuRoadSegment>::const_iterator ri = m_Roads.cbegin(); ri != m_Roads.cend(); ri++)
		(*ri).Dump();
}

quint64 AuRoadNetwork::MemoryUsage() const
{
	quint64 mem = sizeof(AuRoadNetwork);
	for(QList<AuRoadSegment>::const_iterator ri = m_Roads.cbegin(); ri != m_Roads.cend(); ri++)
		mem += sizeof(AuRoadSegment);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuRoadMap::AuRoadMap()
{
}

void AuRoadMap::AddRoad(const DaWorldLL& world, const AuRoadSegment& road)
{
	// Look for the first network that connects with road.
	QList<AuRoadNetwork>::iterator ni = m_Networks.begin();
	while(ni != m_Networks.end())
	{
		if((*ni).TestConnection(world, road))
		{
			(*ni).Add(road);
			break;	// with ni still pointing at our network.
		}
		ni++;
	}
	if(ni != m_Networks.end())
	{
		// Additional networks that connect with road are absorbed into the ni network.
		QList<AuRoadNetwork>::iterator ai = ni;
		ai++;
		while(ai != m_Networks.end())
		{
			if((*ai).TestConnection(world, road))
			{
				(*ni).Add(*ai);
				ai = m_Networks.erase(ai);
			}
			else
				ai++;
		}
		return;
	}

	// Road did not connect with an existing network. Add a nu network.
	AuRoadNetwork nuNet;
	nuNet.Add(road);
	m_Networks.append(nuNet);
}

void AuRoadMap::Connections(const AuId& bdId, QSet<AuId>& connectedBdsAndCities) const
{
	for(QList<AuRoadNetwork>::const_iterator ni = m_Networks.cbegin(); ni != m_Networks.cend(); ni++)
	{
		if(!(*ni).Contains(bdId))
			continue;
		(*ni).Connections(connectedBdsAndCities);
		break;
	}
}

void AuRoadMap::Dump() const
{
	qDebug() << "Road Map of" << m_Networks.size() << "networks";
	for(QList<AuRoadNetwork>::const_iterator ni = m_Networks.cbegin(); ni != m_Networks.cend(); ni++)
		(*ni).Dump();
}

quint64 AuRoadMap::MemoryUsage() const
{
	quint64 mem = sizeof(AuRoadMap);
	for(QList<AuRoadNetwork>::const_iterator ni = m_Networks.cbegin(); ni != m_Networks.cend(); ni++)
		mem += (*ni).MemoryUsage();
	return mem;
}
