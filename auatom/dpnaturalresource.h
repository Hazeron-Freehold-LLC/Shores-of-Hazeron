//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpnaturalresource.h,v 1.30 2022/02/19 17:36:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DpNaturalResource_H
#define DpNaturalResource_H

#include "auatom.h"
#include "aufloradna.h"
#include "aumatrix4x4.h"
#include "dbcommodity.h"
#include <QPoint>
#include <QVector>

class AuDice;
class DaWorld;

class DpNaturalResource
{
protected:
	COMMODITYID			m_idCommodity;		// Db id of raw commodity.
	QPoint				m_Tile;
	AuMatrix4x4			m_Transform;		// Relative to terrain patch.

public:
	DpNaturalResource();
	virtual ~DpNaturalResource();
protected:
	DpNaturalResource(const DpNaturalResource &c);

public:
	virtual COMMODITYID BaseCommodityId() const = 0;	// Base commodity id.
	
	virtual DpNaturalResource* Clone() const = 0;

	inline COMMODITYID CommodityId() const	// Additional commodity besides base commodity.
	{								// May return base commodity.
		return m_idCommodity;
	}
	virtual void InitCommodityId(const DaWorld& world, int resourceZone, AuFloraDNA::ZONE latRegion, TER_ELEV_TYPE tert,
			const AuDice& dice) = 0;

	virtual void InitSize(const AuDice& dice) = 0;
	inline bool InTile(const QPoint& tile) const
	{
		return tile == m_Tile;
	}
	inline bool InTile(int tilex, int tiley) const
	{
		return tilex == m_Tile.x() && tiley == m_Tile.y();
	}

	virtual quint64 MemoryUsage() const;

	virtual AuPoint Rotation(const AuDice& dice) const = 0;

	inline void SetCommodity(COMMODITYID cid)
	{
		m_idCommodity = cid;
	}
	inline void SetLocation(const QPoint& tile, const AuMatrix4x4& t)
	{
		m_Tile = tile;
		m_Transform = t;
	}

	// Test segment 'fromLoc' to 'toLoc' for collision with this resource.
	// Returns true if a collision is detected. On true return, 'hitNormal' is set to the surface normal of the part of this object
	// that was hit.
	// This base class function returns false.
	virtual bool TestCollision(const AuPoint& fromLoc, const AuPoint& toLoc, AuPoint& hitNormal) const;

	inline const QPoint& Tile() const
	{
		return m_Tile;
	}

	inline const AuMatrix4x4& Transform() const
	{
		return m_Transform;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class DpNaturalResourceList : public QVector<DpNaturalResource*>
{
public:
	quint64 MemoryUsage() const;
};

#endif // DpNaturalResource_H

