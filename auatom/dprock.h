//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dprock.h,v 1.25 2022/02/19 17:36:58 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DPROCK_H
#define DPROCK_H

#include "dpnaturalresource.h"

class DpRock : public DpNaturalResource  
{
private:
	float			m_fXScale;		// Scaling applied to nominal object.
	float			m_fYScale;		// Scaling applied to nominal object.
	float			m_fZScale;		// Scaling applied to nominal object.

public:
	DpRock();
	virtual ~DpRock();
protected:
	DpRock(const DpRock &c);

public:
	virtual COMMODITYID BaseCommodityId() const
	{
		return CID_Stone;
	}

	virtual DpNaturalResource* Clone() const
	{
		return new DpRock(*this);
	}

	virtual void InitCommodityId(const DaWorld& world, int resourceZone, AuFloraDNA::ZONE latRegion, TER_ELEV_TYPE tert,
			const AuDice& dice);

	virtual void InitSize(const AuDice& dice);

	virtual quint64 MemoryUsage() const;

	virtual AuPoint Rotation(const AuDice& dice) const;

	static int RockTypes();

	inline float RockVolume() const
	{
		// This isn't accurate but it's close enough.
		return m_fXScale * m_fYScale * m_fZScale;
	}

	virtual bool TestCollision(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitNormal) const;

	// Returns texture tile coordinates for 'rockCID' in txRock.png
	static void TextureCoords(COMMODITYID rockCID, float& s, float& t, float& u, float& v);

	// Returns tile index in txRock.png
	static int TextureTileIndex(COMMODITYID rockCID);
};

#endif // DPROCK_H
