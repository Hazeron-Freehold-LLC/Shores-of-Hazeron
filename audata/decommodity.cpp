//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: decommodity.cpp,v 1.65 2022/02/19 18:20:40 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "decommodity.h"
//
#include "auglobal.h"
#include "auitem.h"

float DeCommodity::Cost() const
{
	return g_Au.CommodityQualityCost(CommodityId(), Quality());
}

// Returns the commodity name of this item.
QString DeCommodity::CommodityName() const
{
	return g_Au.CommodityName(CommodityId());
}
