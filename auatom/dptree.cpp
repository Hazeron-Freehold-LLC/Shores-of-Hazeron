//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dptree.cpp,v 1.27 2022/02/19 17:46:29 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dptree.h"
//
#include <auzoneresource.h>
#include <audice.h>
#include "daworld.h"
#include <qgl.h>
#include <qimage.h>
#include <QVariant>
#include "augleffects.h"

DpTree::DpTree()
{
	m_TrunkLength = 0;
	m_TrunkRadius = 0;
}

DpTree::~DpTree()
{

}

DpTree::DpTree(const DpTree &c) : DpPlant(c)
{
	m_TrunkLength = c.m_TrunkLength;
	m_TrunkRadius = c.m_TrunkRadius;
}

void DpTree::InitSize(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	m_fScale = (float)dice.Roll(1,4) * .25f + 1.f;

	// Trunk sizes were set in InitCommodityId to nominal DNA size.
	m_TrunkLength *= m_fScale;
	m_TrunkRadius *= m_fScale;
}

// Must be a child of a planet before calling this.
void DpTree::InitCommodityId(const DaWorld& world, int resourceZone, AuFloraDNA::ZONE latRegion, TER_ELEV_TYPE /*tert*/,
		const AuDice& dice)
{
	AuZoneResource res;
	world.QueryNaturalResources(res, resourceZone);
	SetCommodity(res.GenerateTreeResource(dice));				// Set the commodity type for this tree.
	InitTrunkSizes(world, resourceZone, latRegion);				// Get tree trunk sizes for collision testing.
}

// Must be a child of a planet before calling this.
void DpTree::InitTrunkSizes(const DaWorld& world, int resourceZone, AuFloraDNA::ZONE latRegion)
{
	world.QueryTreeTrunkSize(CommodityId(), resourceZone, latRegion, m_TrunkLength, m_TrunkRadius);
}

bool DpTree::TestCollision(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitNormal) const
{
	// Calculate the squared radius of the collision zone around the tree.
	float collisionRadius2 = m_TrunkRadius + 1;					// Collision zone includes a 1' margin.
	collisionRadius2 *= collisionRadius2;						// Square the final result to avoid sqrt later.

	// Only test for collision with the main trunk segment. It's the only piece we know the location without doing a lot more work.
	AuPoint baseLoc = m_Transform.Project(0,0,-100);			// Down into the ground so people can't get under on steep hills.
	AuPoint topLoc = m_Transform.Project(0,0,m_TrunkLength);
	AuPoint hitPoint = toLoc.PointOnSegment(baseLoc, topLoc);
	hitNormal = toLoc - hitPoint;
	double distToSeg2 = hitNormal.Length2();
	if(distToSeg2 > collisionRadius2)
		return false;											// Closest point is beyond the trunk radius.
	AuPoint fromHitPoint = fromLoc.PointOnSegment(baseLoc, topLoc);
	if(distToSeg2 > fromLoc.Distance2(fromHitPoint))
		return false;											// Do not block movement away from the tree.
	//qDebug() << "DpTree::TestCollision trunkRadius" << m_TrunkRadius << "trunkLength" << m_TrunkLength;

	// Calculate a hit normal.
	if(distToSeg2 > 0)
		hitNormal /= sqrt(distToSeg2);
	else
		hitNormal = m_Transform.ProjectVector(AuPoint(1,0,0));
	return true;
}

quint64 DpTree::MemoryUsage() const
{
	quint64 mem = DpPlant::MemoryUsage();
	mem += sizeof(m_TrunkLength);
	mem += sizeof(m_TrunkRadius);
	return mem;
}
