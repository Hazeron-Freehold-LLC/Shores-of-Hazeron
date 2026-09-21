//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auordnance.cpp,v 1.13 2022/05/06 16:54:32 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auordnance.h"
//
#include "auitem.h"
#include "ausound.h"

AuOrdnance::~AuOrdnance()
{
	if(m_Ammo)
		delete m_Ammo;
}

AuOrdnance::AuOrdnance(const AuOrdnance& c)
{
	if(c.m_Ammo)
		m_Ammo = c.m_Ammo->Clone();
	else
		m_Ammo = 0;
	m_FireSeq = c.m_FireSeq;
	m_tmLastFired = c.m_tmLastFired;
}

const AuOrdnance& AuOrdnance::operator=(const AuOrdnance& c)
{
	if(m_Ammo)
		delete m_Ammo;
	if(c.m_Ammo)
		m_Ammo = c.m_Ammo->Clone();
	else
		m_Ammo = 0;
	m_FireSeq = c.m_FireSeq;
	m_tmLastFired = c.m_tmLastFired;

	return *this;
}

QString AuOrdnance::BitmapName() const
{
	if(!m_Ammo)
		return "";
	return m_Ammo->BitmapName();
}

COMMODITYID AuOrdnance::CommodityId() const
{
	if(!m_Ammo)
		return CID_Invalid;
	return m_Ammo->CommodityId();
}

QString AuOrdnance::Desc() const
{
	if(!m_Ammo)
		return QObject::tr("No ammunition loaded.");
	return m_Ammo->HtmlDesc();
}

QString AuOrdnance::ListName() const
{
	if(!m_Ammo)
		return QObject::tr("Empty");
	return QObject::tr("<b>%1</b> %L2 rounds").arg(m_Ammo->ListName()).arg(m_Ammo->Count());
}

QString AuOrdnance::Name() const
{
	if(!m_Ammo)
		return "";
	return m_Ammo->ListName();
}

void AuOrdnance::Reload(int rounds)
{
	if(!m_Ammo)
		return;
	m_Ammo->SetCount(rounds);
	m_tmLastFired.Set();
}

int AuOrdnance::RoundsCount() const
{
	if(!m_Ammo)
		return 0;
	return m_Ammo->Count();
}

bool AuOrdnance::IsDeployableWeapon() const
{
	return DbCommodity::IsDeployableWeapon(CommodityId());
}

bool AuOrdnance::IsEnergyWeapon() const
{
	return DbCommodity::IsEnergyWeapon(CommodityId());
}

bool AuOrdnance::IsMissile() const
{
	return DbCommodity::IsMissile(CommodityId());
}

bool AuOrdnance::CanFire() const
{
	// Weapon cannot fire until defined with an item.
	if(!m_Ammo)
		return false;

	// Weapon can run out of ammo.
	if(!m_Ammo->Count())
		return false;

	// Weapon that has not fired can fire immediately.
	if(m_tmLastFired.IsNull())
		return true;

	// Impose rate of fire limitation based on ammo type.
	switch(CommodityId())
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
		if(m_tmLastFired.ElapsedMSec() < 400)
			return false;
		break;
	case CID_BombBl:
	case CID_BombBlRa:
	case CID_BombBlPi:
	case CID_BombElSt:
	case CID_BombSuPo:
	case CID_BombSlCr:
		if(m_tmLastFired.ElapsedMSec() < 500)
			return false;
		break;
	case CID_HeavyArmsAmmoPi:
	case CID_HeavyArmsAmmoPiRa:
	case CID_HeavyArmsAmmoPiBl:
	case CID_HeavyArmsAmmoPiAc:
		if(m_tmLastFired.ElapsedMSec() < 333)
			return false;
		break;
	case CID_LargeMissileBl:
	case CID_LargeMissileBlBu:
	case CID_LargeMissileSlPo:
		if(m_tmLastFired.ElapsedMSec() < 1000)
			return false;
		break;
	case CID_LargeRocketBl:
	case CID_LargeRocketBlBu:
	case CID_LargeRocketBuDi:
		if(m_tmLastFired.ElapsedMSec() < 800)
			return false;
		break;
	case CID_SmallMissileBl:
	case CID_SmallMissileBlBu:
	case CID_SmallMissileCrFr:
		if(m_tmLastFired.ElapsedMSec() < 600)
			return false;
		break;
	case CID_SmallRocketBl:
	case CID_SmallRocketBlBu:
	case CID_SmallRocketSlPi:
		if(m_tmLastFired.ElapsedMSec() < 400)
			return false;
		break;
	}

	return true;
}

void AuOrdnance::Fired()
{
	// Consume a unit of ammo if appropriate.
	if(m_Ammo && m_Ammo->Count())
		m_Ammo->SetCount(m_Ammo->Count() - 1);

	// Cycle the fire sequencer, for displaying fire effect on alternating left/right guns.
	m_FireSeq++;

	// Store last fired time, for rate-of-fire.
	m_tmLastFired.Set();
}

void AuOrdnance::Load(const AuItem& item, int roundsCount)
{
	if(m_Ammo)
		delete m_Ammo;
	m_Ammo = item.Clone();
	m_Ammo->SetCount(roundsCount);
	m_tmLastFired.Set();
}

void AuOrdnance::operator>>(QDataStream& out) const
{
	if(m_Ammo)
	{
		out << m_Ammo->Type();
		out << *m_Ammo;
	}
	else
		out << (quint8)0;

	out << m_FireSeq;

	// We don't store time last fired.
}

void AuOrdnance::operator<<(QDataStream& in)
{
	if(m_Ammo)
		delete m_Ammo;
	quint8 itemType;
	in >> itemType;
	if(itemType)
	{
		m_Ammo = AuItem::Create(itemType);
		if(m_Ammo)
		{
			in >> *m_Ammo;

			// Items rarely change type. e.g. Computer changed from a regular item to a storage device.
			AuItem* replacement = m_Ammo->FixItem();
			if(replacement)
			{
				delete m_Ammo;
				m_Ammo = replacement;
			}
		}
	}
	else
		m_Ammo = 0;

	in >> m_FireSeq;
}

quint64 AuOrdnance::MemoryUsage() const
{
	quint64 mem = sizeof(AuOrdnance);
	if(m_Ammo)
		mem += m_Ammo->MemoryUsage();
	return mem;
}

//static
int AuOrdnance::SoundFired(COMMODITYID cid)
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
		return SOUND_FireLaserBeam;

	case CID_BombBl:
	case CID_BombBlRa:
	case CID_BombBlPi:
	case CID_BombElSt:
	case CID_BombSuPo:
	case CID_BombSlCr:
		return SOUND_BombRelease;

	case CID_HeavyArmsAmmoPi:
	case CID_HeavyArmsAmmoPiRa:
	case CID_HeavyArmsAmmoPiBl:
	case CID_HeavyArmsAmmoPiAc:
		return SOUND_FireCannon;

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
		return SOUND_None;
	}
	return SOUND_None;
}

