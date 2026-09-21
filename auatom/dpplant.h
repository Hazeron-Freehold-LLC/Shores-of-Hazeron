//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpplant.h,v 1.22 2022/04/14 18:47:11 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DpPlant_H
#define DpPlant_H

#include "dpnaturalresource.h"

class DpPlant : public DpNaturalResource  
{
public:
	DpPlant();
	virtual ~DpPlant();
protected:
	DpPlant(const DpPlant &c);

public:
	virtual COMMODITYID BaseCommodityId() const
	{
		return CID_Hay;
	}

	virtual DpNaturalResource* Clone() const
	{
		return new DpPlant(*this);
	}

	virtual void InitCommodityId(const DaWorld& /*world*/, int /*resourceZone*/, AuFloraDNA::ZONE /*latRegion*/,
			TER_ELEV_TYPE /*tert*/, const AuDice& /*dice*/)
	{
		m_idCommodity = CID_Hay;
	}

	virtual void InitSize(const AuDice& dice);

	virtual quint64 MemoryUsage() const;

	virtual AuPoint Rotation(const AuDice& dice) const;

protected:
	float				m_fScale;			// Scaling applied to nominal plant size.
};

#endif // DpPlant_H
