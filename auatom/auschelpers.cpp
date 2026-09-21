//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auschelpers.cpp,v 1.11 2022/05/06 16:55:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auschelpers.h"
//
#include "aubdreport.h"
#include "auglobal.h"
#include "auitem.h"
#include "auscparts.h"
#include "auscreport.h"
#include "aushipsystem.h"
#include "ausound.h"

// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
// The Q_DECLARE_METATYPE macro is also required.
static int s_AuSystemInfo = qRegisterMetaType<AuSystemInfo>();

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

quint64 AuLiftCall::MemoryUsage() const
{
	quint64 mem = 0;
	mem += sizeof(m_LiftId);
	mem += StringMemoryUsage(m_Stop);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuLiftCalls::Call(quint16 liftId, const QString& stop)
{
	// Check for pending call to the lift stop.
	if(HasCall(liftId, stop))
		return;							// A call for that lift stop is already in the list.
	// Add a new call for the lift stop.
	append(AuLiftCall(liftId, stop));
}

bool AuLiftCalls::HasCall(quint16 liftId) const
{
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
	{
		if((*ci).LiftId() != liftId)
			continue;
		// A call for that lift is already in the list.
		return true;
	}
	// No call for the lift was found.
	return false;
}

bool AuLiftCalls::HasCall(quint16 liftId, const QString& stop) const
{
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
	{
		if((*ci).LiftId() != liftId)
			continue;
		if((*ci).Stop() != stop)
			continue;
		// A call for that stop is already in the list.
		return true;
	}
	// No call for the stop was found.
	return false;
}

AuLiftCalls AuLiftCalls::PendingCalls()
{
	AuLiftCalls pendingCalls;
	iterator ci = begin();
	while(ci != end())
	{
		if(!pendingCalls.HasCall((*ci).LiftId()))
		{
			pendingCalls.append(*ci);
			ci = erase(ci);
		}
		else
			ci++;
	}
	return pendingCalls;
}

quint64 AuLiftCalls::MemoryUsage() const
{
	quint64 mem = sizeof(AuLiftCalls);
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
		mem += (*ci).MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuAirPressureHash::operator>>(QDataStream& out) const
{
	out << (quint16)size();
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
	{
		out << ci.key();
		out << *ci;
	}
}

void AuAirPressureHash::operator<<(QDataStream& in)
{
	clear();

	quint16 c;
	in >> c;

	while(c--)
	{
		QString k;
		quint8 v;

		in >> k;
		in >> v;

		insert(k, v);
	}
}

quint64 AuAirPressureHash::MemoryUsage() const
{
	quint64 mem = (sizeof(quint16) + sizeof(quint8)) * capacity();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuFTLDriveStateHash::operator>>(QDataStream& out) const
{
	out << (quint16)size();
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
	{
		out << ci.key();
		out << *ci;
	}
}

void AuFTLDriveStateHash::operator<<(QDataStream& in)
{
	clear();

	quint16 c;
	in >> c;

	while(c--)
	{
		quint16 k;
		AuFTLDriveState v;

		in >> k;
		in >> v;

		insert(k, v);
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuLandingSystemStateHash::operator>>(QDataStream& out) const
{
	out << (quint16)size();
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
	{
		out << ci.key();
		out << *ci;
	}
}

void AuLandingSystemStateHash::operator<<(QDataStream& in)
{
	clear();

	quint16 c;
	in >> c;

	while(c--)
	{
		quint16 k;
		quint8 v;

		in >> k;
		in >> v;

		insert(k, v);
	}
}

quint64 AuLandingSystemStateHash::MemoryUsage() const
{
	quint64 mem = (sizeof(quint16) + sizeof(quint8)) * capacity();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuLauncherStateHash::operator>>(QDataStream& out) const
{
	out << (quint16)size();
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
	{
		out << ci.key();
		out << *ci;
	}
}

void AuLauncherStateHash::operator<<(QDataStream& in)
{
	clear();

	quint16 c;
	in >> c;

	while(c--)
	{
		quint16 k;
		quint8 v;

		in >> k;
		in >> v;

		insert(k, v);
	}
}

quint64 AuLauncherStateHash::MemoryUsage() const
{
	quint64 mem = (sizeof(quint16) + sizeof(quint8)) * capacity();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuRoomLightStateHash::operator>>(QDataStream& out) const
{
	out << (quint16)size();
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
	{
		out << ci.key();
		out << *ci;
	}
}

void AuRoomLightStateHash::operator<<(QDataStream& in)
{
	clear();

	quint16 c;
	in >> c;

	while(c--)
	{
		QString k;
		quint8 v;

		in >> k;
		in >> v;

		insert(k, v);
	}
}

quint64 AuRoomLightStateHash::MemoryUsage() const
{
	quint64 mem = sizeof(AuRoomLightStateHash);
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
		mem += StringMemoryUsage(ci.key()) + sizeof(quint8);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuTransporterSystemStateHash::operator>>(QDataStream& out) const
{
	out << (quint16)size();
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
	{
		out << ci.key();
		out << *ci;
	}
}

void AuTransporterSystemStateHash::operator<<(QDataStream& in)
{
	clear();

	quint16 c;
	in >> c;

	while(c--)
	{
		quint16 k;
		quint8 v;

		in >> k;
		in >> v;

		insert(k, v);
	}
}

quint64 AuTransporterSystemStateHash::MemoryUsage() const
{
	quint64 mem = (sizeof(quint16) + sizeof(quint8)) * capacity();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

quint8 AuTurretState::State() const
{
	return AuScTurret::Gun;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuTurretStateHash::operator>>(QDataStream& out) const
{
	out << (quint16)size();
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
	{
		out << ci.key();
		out << *ci;
	}
}

void AuTurretStateHash::operator<<(QDataStream& in)
{
	clear();

	quint16 c;
	in >> c;

	while(c--)
	{
		quint16 k;
		AuTurretState v;

		in >> k;
		in >> v;

		insert(k, v);
	}
}

quint64 AuTurretStateHash::MemoryUsage() const
{
	quint64 mem = (sizeof(quint16) + sizeof(AuTurretState)) * capacity();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSystemInfo::AuSystemInfo()
{
	m_Quality = 1;
	m_System = SYS_Count;	// Invalid
	m_Which = 0;
}

AuSystemInfo::AuSystemInfo(const QString& desc, SYSTEM system, quint16 which, quint8 quality, COMMODITYID module)
{
	Q_ASSERT(quality);
	if(module != CID_Invalid)
		m_Desc = QObject::tr("%1, %2").arg(desc).arg(g_Au.CommodityName(module));
	else
		m_Desc = desc;
	m_Quality = quality;
	m_System = system;
	m_Which = which;
}

AuSystemInfo::AuSystemInfo(const AuShipSystem& system, const AuScReport& design)
{
	m_Desc = design.SystemName(system.SysId(), system.Module());
	m_Quality = system.Quality();
	m_System = system.GetSystemType();
	m_Which = system.GetSystemWhich();
}

AuSystemInfo::AuSystemInfo(const AuShipSystem& system, const AuBdReport& design)
{
	m_Desc = design.SystemName(system.SysId(), system.Module());
	m_Quality = system.Quality();
	m_System = system.GetSystemType();
	m_Which = system.GetSystemWhich();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBaySetting::AuBaySetting(const AuItem& ammo)
{
	m_Ammo = ammo.Clone();
	m_Ammo->SetCount(1);
	m_Setting = 40;
}

AuBaySetting::~AuBaySetting()
{
	if(m_Ammo)
		delete m_Ammo;
}

AuBaySetting::AuBaySetting(const AuBaySetting& c)
{
	if(c.m_Ammo)
		m_Ammo = c.m_Ammo->Clone();
	else
		m_Ammo = 0;
	m_Setting = c.m_Setting;
	m_TargetId = c.m_TargetId;
}

const AuBaySetting& AuBaySetting::operator=(const AuBaySetting& c)
{
	if(m_Ammo)
		delete m_Ammo;
	if(c.m_Ammo)
		m_Ammo = c.m_Ammo->Clone();
	else
		m_Ammo = 0;
	m_Setting = c.m_Setting;
	m_TargetId = c.m_TargetId;

	return *this;
}

void AuBaySetting::operator>>(QDataStream& out) const
{
	out << m_TargetId;

	// Originally, m_Setting was stored next. It had a max value of 40. We added ammo so squeezed in a hasWhat with a version.
	quint8 hasWhat = 0x80;				// Top bit pushes the value out of the range of 0..40, allowing us to squeeze in a version.
	if(m_Ammo)
		hasWhat |= 0x01;				// Some kind of ammo is loaded.

	out << hasWhat;
	out << m_Setting;

	// Ammo
	if(hasWhat & 0x01)
	{
		out << m_Ammo->Type();
		out << *m_Ammo;
	}
	else
		out << (quint8)0;
}

void AuBaySetting::operator<<(QDataStream& in)
{
	in >> m_TargetId;

	quint8 hasWhat;
	in >> hasWhat;
	//qDebug() << "AuBaySetting::operator<<" << m_TargetId << "hasWhat" << hasWhat;
	
	// Ammo Loaded
	if(m_Ammo)
		delete m_Ammo;
	m_Ammo = 0;
	if(hasWhat & 0x80)
	{
		// Newer version.
		in >> m_Setting;

		// May include ammo.
		quint8 itemType;
		if(hasWhat & 0x01)
		{
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
		}
		else
			in >> itemType;	// This is unnecessary but I broke it by storing this unnecessarily and this was the best fix.
	}
	else
	{
		// Old version before ammo.
		m_Setting = hasWhat;
	}
}

void AuBaySetting::Load(const AuItem& item)
{
	if(m_Ammo)
		delete m_Ammo;
	m_Ammo = item.Clone();
	m_Ammo->SetCount(1);
}

QString AuBaySetting::BitmapName() const
{
	if(!m_Ammo)
		return "";
	return m_Ammo->BitmapName();
}

// Specifically, the ammunition commodity, not the commodity harvested by harvesters.
COMMODITYID AuBaySetting::CommodityId() const
{
	if(!m_Ammo)
		return CID_Invalid;
	return m_Ammo->CommodityId();
}

QString AuBaySetting::Desc() const
{
	if(!m_Ammo)
		return QObject::tr("No ammunition loaded.");
	return m_Ammo->HtmlDesc();
}

QString AuBaySetting::ListName() const
{
	if(!m_Ammo)
		return QObject::tr("Empty");
	return QObject::tr("<b>%1</b>").arg(m_Ammo->ListName());
}

QString AuBaySetting::Name() const
{
	if(!m_Ammo)
		return "";
	return m_Ammo->ListName();
}

int AuBaySetting::SoundFired() const
{
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
		return SOUND_None;
	}
	return SOUND_None;
}

