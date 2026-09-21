//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpplant.cpp,v 1.17 2022/04/14 18:46:07 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dpplant.h"
//
#include <audice.h>

DpPlant::DpPlant()
{
	m_fScale = 1.;
}

DpPlant::~DpPlant()
{

}

DpPlant::DpPlant(const DpPlant &c) : DpNaturalResource(c)
{
	m_fScale = c.m_fScale;
}

void DpPlant::InitSize(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	m_fScale = (float)dice.Roll(1,6) * .25f + .5f;
}

AuPoint DpPlant::Rotation(const AuDice& Cdice) const
{
	AuDice dice(Cdice);
	// Random number from 0 to 359 for plant z rotation.
	AuPoint rot;
	rot.z = dice.Roll(1, 360, -1) * DEG2RADf;
	return rot;
}

quint64 DpPlant::MemoryUsage() const
{
	quint64 mem = DpNaturalResource::MemoryUsage();
	mem += sizeof(m_fScale);
	return mem;
}
