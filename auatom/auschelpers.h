//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auschelpers.h,v 1.17 2022/04/08 15:41:34 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScHelpers_H
#define AuScHelpers_H

#include "auatom.h"
#include "audef.h"
#include "aueffect.h"
#include "auid.h"
#include "aupoint.h"
#include "dbcommodity.h"

class AuBdReport;
class AuItem;
class AuScReport;
class AuShipSystem;

class AUATOM_DLL AuLensGlow
{
private:
	QColor								m_Color;
	AuPoint								m_Loc;

public:	// Constructors
	inline AuLensGlow(const AuPoint& loc, const QColor& color)
	{
		m_Color = color;
		m_Loc = loc;
	}

	inline AuLensGlow()
	{
	}

	inline AuLensGlow(const AuLensGlow& c)
	{
		m_Color = c.m_Color;
		m_Loc = c.m_Loc;
	}

public:	// Operators
	inline const AuLensGlow& operator=(const AuLensGlow& c)
	{
		m_Color = c.m_Color;
		m_Loc = c.m_Loc;
		return *this;
	}

public:	// Methods
	const QColor& Color() const
	{
		return m_Color;
	}

	const AuPoint& Loc() const
	{
		return m_Loc;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuLiftCall
{
private:
	quint16								m_LiftId;				// Part id of turbo lift.
	QString								m_Stop;					// Stop requested.

public:
	inline AuLiftCall(quint16 liftId, const QString& stop)
	{
		m_LiftId = liftId;
		m_Stop = stop;
	}

	inline AuLiftCall()
	{
		m_LiftId = 0;
	}

	inline AuLiftCall(const AuLiftCall& c)
	{
		m_LiftId = c.m_LiftId;
		m_Stop = c.m_Stop;
	}

public:
	inline const AuLiftCall& operator=(const AuLiftCall& c)
	{
		m_LiftId = c.m_LiftId;
		m_Stop = c.m_Stop;
		return *this;
	}

	inline bool operator==(const AuLiftCall& c) const
	{
		if(m_LiftId != c.m_LiftId)
			return false;
		if(m_Stop != c.m_Stop)
			return false;
		return true;
	}

public:	// Operators
	inline void operator>>(QDataStream& out) const
	{
		out << m_LiftId;
		out << m_Stop;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_LiftId;
		in >> m_Stop;
	}

public:
	inline quint16 LiftId() const
	{
		return m_LiftId;
	}

	quint64 MemoryUsage() const;

	inline const QString& Stop() const
	{
		return m_Stop;
	}
};

inline unsigned int qHash(const AuLiftCall& c)
{
	return c.LiftId();
}

inline QDataStream& operator<<(QDataStream& out, const AuLiftCall& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuLiftCall& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuLiftCalls : public QList<AuLiftCall>
{
public:
	// Adds a call for a lift.
	void Call(quint16 liftId, const QString& stop);

	// Returns true if this list contains any call for 'liftId'.
	bool HasCall(quint16 liftId) const;

	// Returns true if this list contains any call for the 'liftId' 'stop'.
	bool HasCall(quint16 liftId, const QString& stop) const;

	quint64 MemoryUsage() const;

	// Returns the list of pending calls for each lift, if any. Oldest calls are returned first.
	// Removes those calls from this list of calls.
	AuLiftCalls PendingCalls();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuAirPressureHash : public QHash<QString, quint8>
{
public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:
	quint64 MemoryUsage() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuAirPressureHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuAirPressureHash& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuFTLDriveState
{
private:
	quint8								m_Commit;				// FTL_COMMIT drive charging state.
	quint8								m_Power;				// FTL_DRIVE_STATE powered on state
	quint8								m_Setting;				// Wormhole polarity (0=+, 1=-, 2=neutral), or warp factor (0..9).

public:	// Constructors
	inline AuFTLDriveState()
	{
		m_Commit = FTL_COMMIT_Dormant;
		m_Power = FTL_DRIVE_Off;
		m_Setting = 0;
	}

	inline AuFTLDriveState(const AuFTLDriveState& c)
	{
		m_Commit = c.m_Commit;
		m_Power = c.m_Power;
		m_Setting = c.m_Setting;
	}

public:	// Operators
	inline const AuFTLDriveState& operator=(const AuFTLDriveState& c)
	{
		m_Commit = c.m_Commit;
		m_Power = c.m_Power;
		m_Setting = c.m_Setting;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_Commit;
		out << m_Power;
		out << m_Setting;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_Commit;
		in >> m_Power;
		in >> m_Setting;
	}

	inline bool operator==(const AuFTLDriveState& c) const
	{
		if(m_Commit != c.m_Commit)
			return false;
		if(m_Power != c.m_Power)
			return false;
		if(m_Setting != c.m_Setting)
			return false;
		return true;
	}

	inline bool operator!=(const AuFTLDriveState& c) const
	{
		return operator==(c);
	}

public:	// Methods
	inline FTL_COMMIT Commit() const
	{
		return (FTL_COMMIT)m_Commit;
	}

	inline FTL_DRIVE_STATE Power() const
	{
		return (FTL_DRIVE_STATE)m_Power;
	}

	inline void SetCommit(FTL_COMMIT c)
	{
		m_Commit = c;
	}

	inline void SetPower(FTL_DRIVE_STATE p)
	{
		m_Power = p;
	}

	inline void SetSetting(quint8 s)
	{
		m_Setting = s;
	}

	inline quint8 Setting() const
	{
		return m_Setting;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuFTLDriveState& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuFTLDriveState& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuFTLDriveStateHash : public QHash<quint16, AuFTLDriveState>
{
public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	quint64 MemoryUsage() const
	{
		return (sizeof(quint16) + sizeof(AuFTLDriveState)) * capacity();
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuFTLDriveStateHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuFTLDriveStateHash& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuLandingSystemStateHash : public QHash<quint16, quint8>
{
public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:
	quint64 MemoryUsage() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuLandingSystemStateHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuLandingSystemStateHash& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuLauncherStateHash : public QHash<quint16, quint8>
{
public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:
	quint64 MemoryUsage() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuLauncherStateHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuLauncherStateHash& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Key is room name.
// Value bits have meaning as follows.
//  b0-b1 INTERIOR_LIGHT value.
//  b2 Bridge indicators: course, station beacons, city beacons
//  b3 Orbit rings of moons visible.
//  b4 Orbit rings of planets and stars visible.
//  b5 Wormhole lines visible.
//  b6-b7 Star names visible. 00=no, 01=named solar systems, 10=all solar systems

class AUATOM_DLL AuRoomLightStateHash : public QHash<QString, quint8>
{
public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	inline bool AnyLightOn() const
	{
		for(const_iterator li = cbegin(); li != cend(); li++)
			if(InteriorLight(*li) != INTLIGHT_Dark)
				return true;
		return false;
	}

	inline static bool BridgeVisible(quint8 roomLightStateVal)
	{
		return roomLightStateVal & 0x04;
	}

	inline bool BridgeVisible(const QString& roomName) const
	{
		const_iterator li = find(roomName);
		if(li == cend())
			return false;
		return BridgeVisible(*li);
	}

	inline static INTERIOR_LIGHT InteriorLight(quint8 roomLightStateVal)
	{
		return (INTERIOR_LIGHT)(roomLightStateVal & 0x03);
	}

	inline INTERIOR_LIGHT InteriorLight(const QString& roomName) const
	{
		const_iterator li = find(roomName);
		if(li == cend())
			return INTLIGHT_Invalid;
		return InteriorLight(*li);
	}

	quint64 MemoryUsage() const;

	inline static bool MoonOrbitRingsVisible(quint8 roomLightStateVal)
	{
		return roomLightStateVal & 0x08;
	}

	inline bool MoonOrbitRingsVisible(const QString& roomName) const
	{
		const_iterator li = find(roomName);
		if(li == cend())
			return false;
		return MoonOrbitRingsVisible(*li);
	}

	inline static bool OrbitRingsVisible(quint8 roomLightStateVal)
	{
		return roomLightStateVal & 0x10;
	}

	inline bool OrbitRingsVisible(const QString& roomName) const
	{
		const_iterator li = find(roomName);
		if(li == cend())
			return false;
		return OrbitRingsVisible(*li);
	}

	inline bool SetBridgeVisible(const QString& roomName, bool visible)
	{
		iterator li = find(roomName);
		if(li == end())
			return false;

		if(BridgeVisible(*li) == visible)
			return false;

		if(visible)
			*li |= 0x04;				// Turn on wormhole lines bit.
		else
			*li &= ~0x04;				// Turn off wormhole lines bit.
		return true;
	}

	inline bool SetInteriorLight(INTERIOR_LIGHT light)
	{
		bool anySet = false;
		for(iterator li = begin(); li != end(); li++)
			if(InteriorLight(*li) != light)
			{
				*li &= ~0x03;			// Turn off interior light bits.
				*li |= light;			// Turn on desired interior light bits.
				anySet = true;
			}
		return anySet;
	}

	inline bool SetInteriorLight(const QString& roomName, INTERIOR_LIGHT light)
	{
		iterator li = find(roomName);
		if(li == end())
			return false;

		if(InteriorLight(*li) == light)
			return false;

		*li &= ~0x03;					// Turn off all interior light bits.
		*li |= light;					// Turn on desired interior light bits.
		return true;
	}

	inline bool SetMoonOrbitRingsVisible(const QString& roomName, bool visible)
	{
		iterator li = find(roomName);
		if(li == end())
			return false;

		if(MoonOrbitRingsVisible(*li) == visible)
			return false;

		if(visible)
			*li |= 0x08;				// Turn on orbit rings bit.
		else
			*li &= ~0x08;				// Turn off orbit rings bit.
		return true;
	}

	inline bool SetOrbitRingsVisible(const QString& roomName, bool visible)
	{
		iterator li = find(roomName);
		if(li == end())
			return false;

		if(OrbitRingsVisible(*li) == visible)
			return false;

		if(visible)
			*li |= 0x10;				// Turn on orbit rings bit.
		else
			*li &= ~0x10;				// Turn off orbit rings bit.
		return true;
	}

	inline bool SetSystemNamesVisible(const QString& roomName, int visible)
	{
		iterator li = find(roomName);
		if(li == end())
			return false;

		if(SystemNamesVisible(*li) == visible)
			return false;

		if(visible < 0 || visible > 2)
			visible = 0;

		*li &= ~0xc0;					// Turn off all solar system names bits.
		*li |= visible << 6;			// Turn on desired solar system names bits.
		return true;
	}

	inline bool SetWormholeLinesVisible(const QString& roomName, bool visible)
	{
		iterator li = find(roomName);
		if(li == end())
			return false;

		if(WormholeLinesVisible(*li) == visible)
			return false;

		if(visible)
			*li |= 0x20;				// Turn on wormhole lines bit.
		else
			*li &= ~0x20;				// Turn off wormhole lines bit.
		return true;
	}

	inline static int SystemNamesVisible(quint8 roomLightStateVal)
	{
		return roomLightStateVal >> 6;
	}

	inline int SystemNamesVisible(const QString& roomName) const
	{
		const_iterator li = find(roomName);
		if(li == cend())
			return 0;
		return SystemNamesVisible(*li);
	}

	inline static bool WormholeLinesVisible(quint8 roomLightStateVal)
	{
		return roomLightStateVal & 0x20;
	}

	inline bool WormholeLinesVisible(const QString& roomName) const
	{
		const_iterator li = find(roomName);
		if(li == cend())
			return false;
		return WormholeLinesVisible(*li);
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuRoomLightStateHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuRoomLightStateHash& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Value is TRANSPORTER_STATE

class AUATOM_DLL AuTransporterSystemStateHash : public QHash<quint16, quint8>
{
public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:
	quint64 MemoryUsage() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuTransporterSystemStateHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuTransporterSystemStateHash& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuTurretState
{
private:
	float								m_BaseRot;				// Rotation around base axis.
	float								m_GunRot;				// Rotation around gun axis.

public:	// Constructors
	inline AuTurretState()
	{
		m_BaseRot = 0;
		m_GunRot = 0;
	}

	inline AuTurretState(const AuTurretState& c)
	{
		m_BaseRot = c.m_BaseRot;
		m_GunRot = c.m_GunRot;
	}

public:	// Operators
	inline const AuTurretState& operator=(const AuTurretState& c)
	{
		m_BaseRot = c.m_BaseRot;
		m_GunRot = c.m_GunRot;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << m_BaseRot;
		out << m_GunRot;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_BaseRot;
		in >> m_GunRot;
	}

	inline bool operator==(const AuTurretState& c) const
	{
		if(m_BaseRot != c.m_BaseRot)
			return false;
		if(m_GunRot != c.m_GunRot)
			return false;
		return true;
	}

	inline bool operator!=(const AuTurretState& c) const
	{
		return operator==(c);
	}

public:	// Methods
	inline float BaseRotation() const
	{
		return m_BaseRot;
	}

	inline float GunRotation() const
	{
		return m_GunRot;
	}

	inline void SetBaseRotation(float rot)
	{
		m_BaseRot = rot;
	}

	inline void SetGunRotation(float rot)
	{
		m_GunRot = rot;
	}

	quint8 State() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuTurretState& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuTurretState& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuTurretStateHash : public QHash<quint16, AuTurretState>
{
public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:
	quint64 MemoryUsage() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuTurretStateHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuTurretStateHash& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuSystemInfo
{
public:
	QString						m_Desc;
	quint8						m_Quality;
	SYSTEM						m_System;
	quint16						m_Which;

public:
	AuSystemInfo();

	AuSystemInfo(const QString& desc, SYSTEM system, quint16 which, quint8 quality, COMMODITYID module);

	AuSystemInfo(const AuShipSystem& system, const AuScReport& design);
	AuSystemInfo(const AuShipSystem& system, const AuBdReport& design);

	inline ~AuSystemInfo()
	{
	}

	inline AuSystemInfo(const AuSystemInfo& c)
	{
		m_Desc = c.m_Desc;
		m_Quality = c.m_Quality;
		m_System = c.m_System;
		m_Which = c.m_Which;
	}

	inline QString SystemIconName() const
	{
		return SystemIcon(m_System);
	}
};

Q_DECLARE_METATYPE(AuSystemInfo)	// Needed to this use class in variant. Must also call qRegisterMetaType.

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL AuBaySetting
{
public:
	AuItem*								m_Ammo;					// Ammo to selected for bay weapons. n/a for harvesters.
	AuId								m_TargetId;
	quint8								m_Setting;				// Power level setting 0..40 for weapons.
																// Commodity index for harvesters; see DpConsoleWeapon.

public:
	AuBaySetting(const AuItem& ammo);

	AuBaySetting()
	{
		m_Ammo = 0;
		m_Setting = 0;
	}

	~AuBaySetting();

	AuBaySetting(const AuBaySetting& c);

	const AuBaySetting& operator=(const AuBaySetting& c);

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:
	inline const AuItem* Ammo() const
	{
		return m_Ammo;
	}

	QString BitmapName() const;

	// Specifically, the ammunition commodity, not the commodity harvested by harvesters.
	COMMODITYID CommodityId() const;

	QString Desc() const;

	inline bool IsEnergyWeapon() const
	{
		return DbCommodity::IsEnergyWeapon(CommodityId());
	}

	inline bool IsMissile() const
	{
		return DbCommodity::IsMissile(CommodityId());
	}

	// Sets the count in our ammo item to 1.
	void Load(const AuItem& item);

	QString ListName() const;

	QString Name() const;

	// Return value is a SOUND_EFFECT
	inline int SoundFired() const;

	inline VISUAL_EFFECT VisualFired() const
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
			return VISUAL_None;
		}
		return VISUAL_None;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuBaySetting& bs)
{
	bs >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuBaySetting& bs)
{
	bs << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuScHelpers_H
