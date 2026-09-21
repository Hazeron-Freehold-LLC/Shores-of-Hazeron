//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dprock.cpp,v 1.42 2022/02/19 17:45:11 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dprock.h"
//
#include <auzoneresource.h>
#include <audice.h>
#include "daworld.h"
#include <qgl.h>
#include <qimage.h>
#include <augleffects.h>

DpRock::DpRock()
{
	m_fXScale = 1.;
	m_fYScale = 1.;
	m_fZScale = 1.;
}

DpRock::~DpRock()
{

}

DpRock::DpRock(const DpRock &c) : DpNaturalResource(c)
{
	m_fXScale = c.m_fXScale;
	m_fYScale = c.m_fYScale;
	m_fZScale = c.m_fZScale;
}

void DpRock::InitSize(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	if(m_idCommodity == CID_Gems)
	{
		// Gems are symmetrically scaled.
		m_fXScale = (float)dice.Roll(1,8)/20.f;	// Gems are very small.
		m_fYScale = m_fXScale;
		m_fZScale = m_fXScale;
	}
	else if(m_idCommodity == CID_Crystals)
	{
		// Crystals are symmetrically scaled.
		m_fXScale = (float)dice.Roll(1,8)/3.f;
		m_fYScale = m_fXScale;
		m_fZScale = m_fXScale;
	}
	else if(m_idCommodity == CID_Eludium)
	{
		// Eludium is symmetrically scaled shards.
		m_fXScale = (float)dice.Roll(1,8)/3.f;
		m_fYScale = m_fXScale/8;
		m_fZScale = m_fYScale;
	}
	else if(m_idCommodity == CID_Radioactives)
	{
		// Radioactives are smaller than standard rocks.
		m_fXScale = (float)dice.Roll(1,8)/4.f;
		m_fYScale = (float)dice.Roll(1,8)/4.f;
		m_fZScale = (float)dice.Roll(1,8)/4.f;
	}
	else if(m_idCommodity == CID_Lumenite)
	{
		// Lumenite is larger than radioactives and smaller than standard rocks.
		m_fXScale = (float)dice.Roll(1,8)/3.f;
		m_fYScale = (float)dice.Roll(1,8)/3.f;
		m_fZScale = (float)dice.Roll(1,8)/3.f;
	}
	else
	{
		// 'Standard' rocks.
		m_fXScale = (float)dice.Roll(1,8)/2.f;	// Scale is .5 to 4. in .5 incs
		m_fYScale = (float)dice.Roll(1,8)/2.f;	// Scale is .5 to 4. in .5 incs
		m_fZScale = (float)dice.Roll(1,8)/2.f;	// Scale is .5 to 4. in .5 incs
	}
}

// Must be a child of a planet before calling this.
void DpRock::InitCommodityId(const DaWorld& world, int resourceZone, AuFloraDNA::ZONE latRegion, TER_ELEV_TYPE tert,
		const AuDice& dice)
{
	AuZoneResource res;
	world.QueryNaturalResources(res, resourceZone);

	// Ice chunks appear on snow capped peaks and in polar regions.
	// Ice is present among the rocks in the environment of cold worlds.
	int iceFactor;
	if(tert == TER_Peak || latRegion == AuFloraDNA::Polar)
		iceFactor = 50;	// lots of ice on snow capped peaks and poles
	else if(world.OrbitZone() == OZONE_Outer)
		iceFactor = 1;	// cold worlds have ice lying about
	else if(world.OrbitZone() == OZONE_Frigid)
		iceFactor = 2;	// frozen worlds have a bit more ice lying about
	else
		iceFactor = 0;	// Warmer worlds don't have ice lying about.

	m_idCommodity = res.GenerateRockResource(dice, iceFactor);
	//qDebug("Rock commodity set to: %d", m_idCommodity);
}

//static
int DpRock::TextureTileIndex(COMMODITYID rockCID)
{
	int tileIndex;
	switch(rockCID)
	{
	case CID_Stone:
	default:
		tileIndex = 0;
		break;
	case CID_Coal:
		tileIndex = 1;
		break;
	case CID_Crystals:
		tileIndex = 2;
		break;
	case CID_Gems:
		tileIndex = 3;
		break;
	case CID_Minerals:
		tileIndex = 4;
		break;
	case CID_Ore:
		tileIndex = 5;
		break;
	case CID_Radioactives:
		tileIndex = 6;
		break;
	case CID_Lumenite:
		tileIndex = 7;
		break;
	case CID_Ice:
		tileIndex = 8;
		break;
	case CID_CrudeOil:
		tileIndex = 9;
		break;
	case CID_Eludium:
		tileIndex = 10;
		break;
	case CID_Bolite:
		tileIndex = 11;
		break;
	case CID_Phlogiston:
		tileIndex = 12;
		break;
	case CID_Polytaride:
		tileIndex = 13;
		break;
	case CID_Vulcanite:
		tileIndex = 14;
		break;
	case CID_Adamantite:
		tileIndex = 15;
		break;
	case CID_Viathol:
		tileIndex = 16;
		break;
	case CID_GoldOre:
		tileIndex = 17;
		break;
	}
	return tileIndex;
}

//static
int DpRock::RockTypes()
{
	return 18;
}

void DpRock::TextureCoords(COMMODITYID rockCID, float& s, float& t, float& u, float& v)
{
	int tileIndex = TextureTileIndex(rockCID);
	const int tileCount = RockTypes();

	// Calculate texture coordinates of appropriate tile.
	const int tileSize = 64;	// texture tile size
	float maxx = tileCount * tileSize;
	float maxy = tileSize;
	s = (tileIndex * tileSize + 1.) / maxx;
	t = 1./maxy;
	u = ((tileIndex+1) * tileSize - 2.) / maxx;
	v = ((float)tileSize - 2.) / maxy;
}

bool DpRock::TestCollision(const AuPoint& /*fromLoc*/, const AuPoint& /*toLoc*/, AuPoint& /*hitNormal*/) const
{
	return false;
	//DHH This is pointless because most rocks are too low to hit unless you are a belly crawler.
	/*
	// Some rocks are too small to hit.
	if(m_idCommodity == CID_Gems || m_idCommodity == CID_Eludium)
		return false;

	AuPoint myLoc = m_Transform.Translation();
	hitNormal = toLoc - myLoc;
	double dist2 = hitNormal.Length2();
	if(dist2 > 1.5*1.5)
		return false;											// Destination point is beyond the rock radius.
	if(dist2 > myLoc.Distance2(fromLoc))
		return false;											// Do not block movement away from the rock.

	// Calculate a hit normal.
	if(dist2 > 0)
		hitNormal /= sqrt(dist2);
	else
		hitNormal = m_Transform.ProjectVector(AuPoint(0,0,1));
	return true;
	*/
}

AuPoint DpRock::Rotation(const AuDice& Cdice) const
{
	AuDice dice(Cdice);
	// Random number from 0 to 359 for rock x,y,z rotations.
	AuPoint rot;
	rot.x = dice.Roll(1, 360, -1) * DEG2RADf;
	rot.y = dice.Roll(1, 360, -1) * DEG2RADf;
	rot.z = dice.Roll(1, 360, -1) * DEG2RADf;
	return rot;
}

quint64 DpRock::MemoryUsage() const
{
	quint64 mem = DpNaturalResource::MemoryUsage();
	mem += sizeof(m_fXScale);
	mem += sizeof(m_fYScale);
	mem += sizeof(m_fZScale);
	return mem;
}
