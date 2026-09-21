//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpnaturalresource.cpp,v 1.26 2022/02/19 17:45:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dpnaturalresource.h"
//

DpNaturalResource::DpNaturalResource()
{
	m_idCommodity = CID_Invalid;
}

DpNaturalResource::~DpNaturalResource()
{
}

DpNaturalResource::DpNaturalResource(const DpNaturalResource &c)
{
	m_idCommodity = c.m_idCommodity;
	m_Tile = c.m_Tile;
	m_Transform = c.m_Transform;		// Relative to terrain patch.
}

bool DpNaturalResource::TestCollision(const AuPoint& /*fromLoc*/, const AuPoint& /*toLoc*/, AuPoint& /*hitNormal*/) const
{
	return false;
}

quint64 DpNaturalResource::MemoryUsage() const
{
	quint64 mem = 0;
	mem += sizeof(m_idCommodity);
	mem += sizeof(m_Tile);
	mem += sizeof(m_Transform);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|


quint64 DpNaturalResourceList::MemoryUsage() const
{
	quint64 mem = sizeof(DpNaturalResourceList);
	for(const_iterator ri = cbegin(); ri != cend(); ri++)
		mem += (*ri)->MemoryUsage();
	return mem;
}
