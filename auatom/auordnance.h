//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auordnance.h,v 1.17 2022/05/17 16:53:10 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuOrdnance_H
#define AuOrdnance_H

#include "auatom.h"
#include "aueffect.h"
#include "auelapsedtimer.h"
#include "dbcommodity.h"

class AuItem;

class AUATOM_DLL AuOrdnance
{
public:
	AuItem*								m_Ammo;					// Count is rounds remaining. Turret modules used for energy beams.
	quint8								m_FireSeq;				// For alternation between left/right firing positions.
	AuElapsedTimer						m_tmLastFired;			// Time last fired, for rate of fire control. Not stored.

public:	// Constructors
	inline AuOrdnance()
	{
		m_Ammo = 0;
		m_FireSeq = 0;
	}

	~AuOrdnance();

	AuOrdnance(const AuOrdnance& c);

public:	// Operators
	const AuOrdnance& operator=(const AuOrdnance& c);

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	inline const AuItem* Ammo() const
	{
		return m_Ammo;
	}

	QString BitmapName() const;

	// Returns true if this weapon can fire based on rate-of-fire since last fired and ammunition when appropriate.
	bool CanFire() const;

	COMMODITYID CommodityId() const;

	QString Desc() const;

	// Caller should check CanFire first. Consumes a unit of ammo when appropriate.
	void Fired();

	inline quint8 FireSequence() const
	{
		return m_FireSeq;
	}

	bool IsDeployableWeapon() const;

	bool IsEnergyWeapon() const;

	bool IsMissile() const;

	inline const AuElapsedTimer& LastFired() const
	{
		return m_tmLastFired;
	}

	void Load(const AuItem& item, int roundsCount);

	QString ListName() const;

	quint64 MemoryUsage() const;

	QString Name() const;

	void Reload(int rounds);

	int RoundsCount() const;

	// Returns value is a SOUND_EFFECT
	inline int SoundFired() const
	{
		return SoundFired(CommodityId());
	}

	// Returns value is a SOUND_EFFECT
	static int SoundFired(COMMODITYID cid);

	inline VISUAL_EFFECT VisualFired() const
	{
		return VisualFired(CommodityId());
	}

	static inline VISUAL_EFFECT VisualFired(COMMODITYID cid)
	{
		switch(cid)
		{
		default:
			break;

		case CID_StasisBeam:
		case CID_NeutronBeam:
		case CID_ElectronBeam:
		case CID_ErgRay:
		case CID_FreezeRay:
		case CID_CompressorBeam:
		case CID_DisruptorBeam:
		case CID_HeatRay:
		case CID_ProtonBeam:
		case CID_DisintegratorRay:
			return VISUAL_FireEnergyBeam;

		case CID_BombBl:
		case CID_BombBlRa:
		case CID_BombBlPi:
		case CID_BombElSt:
		case CID_BombSuPo:
		case CID_BombSlCr:
			return VISUAL_None;

		case CID_HeavyArmsAmmoPi:
		case CID_HeavyArmsAmmoPiRa:
		case CID_HeavyArmsAmmoPiBl:
		case CID_HeavyArmsAmmoPiAc:
			return VISUAL_FireCannon;

		case CID_LargeMissileBl:
		case CID_LargeMissileBlBu:
		case CID_LargeMissileSlPo:
		case CID_LargeRocketBl:
		case CID_LargeRocketBlBu:
		case CID_LargeRocketBuDi:
		case CID_SmallMissileBl:
		case CID_SmallMissileBlBu:
		case CID_SmallMissileCrFr:
		case CID_SmallRocketBl:
		case CID_SmallRocketBlBu:
		case CID_SmallRocketSlPi:
			return VISUAL_None;
		}
		return VISUAL_None;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuOrdnance& c)
{
	c >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuOrdnance& c)
{
	c << in;
	return in;
}

#endif // AuOrdnance_H
