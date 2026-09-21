//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpshrub.h,v 1.20 2022/02/19 17:37:45 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DPSHRUB_H
#define DPSHRUB_H

#include "dpplant.h"

class DpShrub : public DpPlant  
{
public:
	DpShrub();
	virtual ~DpShrub();
protected:
	DpShrub(const DpShrub &c);

public:
	virtual COMMODITYID BaseCommodityId() const
	{
		return CID_Hay;
	}

	virtual DpNaturalResource* Clone() const
	{
		return new DpShrub(*this);
	}

	virtual void InitCommodityId(const DaWorld& world, int resourceZone, AuFloraDNA::ZONE latRegion, TER_ELEV_TYPE tert,
			const AuDice& dice);

	virtual void InitSize(const AuDice& dice);
};

#endif // DPSHRUB_H
