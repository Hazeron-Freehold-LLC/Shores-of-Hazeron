//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dptree.h,v 1.23 2022/02/19 17:38:29 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DpTree_H
#define DpTree_H

#include "dpplant.h"

class DpTree : public DpPlant  
{
private:
	float								m_TrunkLength;			// Length of the first trunk segment.
	float								m_TrunkRadius;			// Radius at the base of the tree.

public:
	DpTree();
	virtual ~DpTree();
protected:
	DpTree(const DpTree &c);

public:
	virtual COMMODITYID BaseCommodityId() const
	{
		return CID_Wood;
	}

	virtual DpNaturalResource* Clone() const
	{
		return new DpTree(*this);
	}

	virtual void InitCommodityId(const DaWorld& world, int resourceZone, AuFloraDNA::ZONE latRegion, TER_ELEV_TYPE tert,
			const AuDice& dice);

	virtual void InitSize(const AuDice& dice);

	void InitTrunkSizes(const DaWorld& world, int resourceZone, AuFloraDNA::ZONE latRegion);

	virtual quint64 MemoryUsage() const;

	virtual bool TestCollision(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitNormal) const;

	inline float TrunkLength() const
	{
		return m_TrunkLength;
	}

	inline float TrunkRadius() const
	{
		return m_TrunkRadius;
	}
};

#endif // DpTree_H
