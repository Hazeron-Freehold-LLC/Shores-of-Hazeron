//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpshrub.cpp,v 1.22 2022/02/19 17:45:46 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dpshrub.h"
//
#include <auzoneresource.h>
#include <audice.h>
#include "daworld.h"
#include <qgl.h>
#include <qimage.h>
#include "augleffects.h"

DpShrub::DpShrub()
{
}

DpShrub::~DpShrub()
{

}

DpShrub::DpShrub(const DpShrub &c) : DpPlant(c)
{
}

void DpShrub::InitSize(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	m_fScale = (float)dice.Roll(1,4) * .5f + 1.25f;
}

// Must be a child of a planet before calling this.
void DpShrub::InitCommodityId(const DaWorld& world, int resourceZone, AuFloraDNA::ZONE /*latRegion*/, TER_ELEV_TYPE /*tert*/,
		const AuDice& dice)
{
	AuZoneResource res;
	world.QueryNaturalResources(res, resourceZone);
	m_idCommodity = res.GenerateShrubResource(dice);
	//qDebug("Shrub commodity set to: %d", m_idCommodity);
}
