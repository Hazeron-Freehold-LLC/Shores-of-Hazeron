//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscreporthelpers.h,v 1.6 2022/04/08 14:45:23 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScReportHelpers_H
#define AuScReportHelpers_H

#include "audata.h"
#include "auid.h"
#include "aupoint.h"
#include "dbcommodity.h"

class AuScParts;

#define d_VehicleLauncherHits			4						// AuScSpaceVehicleLauncher::ListComponents adds this many.
#define d_VehicleLandingSystemHits		3						// AuScSpaceVehicleLandingSystem::ListComponents adds this many.

class AUDATA_DLL AuScDoorState
{
public:
	enum DOORTYPE
	{
		DOOR_Interior,					// Door between interior spaces.
		DOOR_Hull,						// Door between an interior space and an exterior space
		DOOR_InteriorLift,				// Door between interior space and turbo lift.
		DOOR_HullLift,					// Door between exterior space and turbo lift.
		DOOR_Launcher,					// Door between launch tube interior space and exterior space.
	};

private:
	quint8								m_DoorType;				// DOORTYPE enum. Top bit (b7) on is a manually operated door.
	AuPoint								m_OpLoc1;				// Location of one side of the door, where AI goes to operate it.
	AuPoint								m_OpLoc2;				// Location of other side of the door, where AI goes to operate it.
	quint16								m_PartId;				// Turbo lift id for lift door. Launcher id for launcher door.
	quint8								m_State;				// AuScDoor::STATES enum value. Includes a "locked" state.

public:	// Constructors
	inline AuScDoorState()
	{
		m_DoorType = DOOR_Interior;
		m_PartId = 0;
		m_State = 0;
	}

	inline AuScDoorState(quint8 state)
	{
		m_DoorType = DOOR_Interior;
		m_PartId = 0;
		m_State = state;
	}

	inline AuScDoorState(DOORTYPE doorType, const AuPoint& opLoc1, const AuPoint& opLoc2, quint16 partId, quint8 state,
			bool isManual, bool remoteSpacecraft, bool remoteVehicle)
	{
		m_DoorType = doorType;
		if(isManual)
			m_DoorType |= 0x80;	// Turn on manual bit.
		if(remoteSpacecraft)
			m_DoorType |= 0x40;
		if(remoteVehicle)
			m_DoorType |= 0x20;
		m_OpLoc1 = opLoc1;
		m_OpLoc2 = opLoc2;
		m_PartId = partId;
		m_State = state;
	}

	inline AuScDoorState(const AuScDoorState& c)
	{
		m_DoorType = c.m_DoorType;
		m_OpLoc1 = c.m_OpLoc1;
		m_OpLoc2 = c.m_OpLoc2;
		m_PartId = c.m_PartId;
		m_State = c.m_State;
	}

public:	// Operators
	inline const AuScDoorState& operator=(const AuScDoorState& c)
	{
		m_DoorType = c.m_DoorType;
		m_OpLoc1 = c.m_OpLoc1;
		m_OpLoc2 = c.m_OpLoc2;
		m_PartId = c.m_PartId;
		m_State = c.m_State;
		return *this;
	}

	inline void operator>>(QDataStream& out) const
	{
		quint8 hstate = m_State;	// Need 4 bits for state.
		// We used to only store hull doors as a single bit. Door types were added later.
		if(m_DoorType == DOOR_Hull)
			hstate |= 0x80;
		if(m_PartId)
			hstate |= 0x40;
		if(m_DoorType && m_DoorType != DOOR_Hull)
			hstate |= 0x20;

		out << hstate;
		if(hstate & 0x40)
			out << m_PartId;
		out << m_OpLoc1;
		out << m_OpLoc2;
		if(hstate & 0x20)
			out << m_DoorType;	// Door type and manual bit.
	}

	inline void operator<<(QDataStream& in)
	{
		quint8 hstate;

		in >> hstate;

		m_State = hstate & 0x0f;
		//m_HullDoor = hstate & 0x80;
		if(hstate & 0x40)
			in >> m_PartId;
		in >> m_OpLoc1;
		in >> m_OpLoc2;
		if(hstate & 0x80)
			m_DoorType = DOOR_Hull;
		else if(hstate & 0x20)
			in >> m_DoorType;
		else
			m_DoorType = DOOR_Interior;
	}

	inline bool operator==(const AuScDoorState& c) const
	{
		if(m_DoorType != c.m_DoorType)
			return false;
		if(m_PartId != c.m_PartId)
			return false;
		if(m_State != c.m_State)
			return false;
		if(m_OpLoc1 != c.m_OpLoc1)
			return false;
		if(m_OpLoc2 != c.m_OpLoc2)
			return false;
		return true;
	}

	inline bool operator!=(const AuScDoorState& c) const
	{
		return operator==(c);
	}

public:	// Methods
	void Close();

	inline DOORTYPE DoorType() const
	{
		return (DOORTYPE)(m_DoorType & 0x07);
	}

	// Hull doors and turbo lift doors are not automatic.
	// Locked interior doors are only automatic while they are open; they don't open automatically.
	bool IsAutomaticDoor() const;

	bool IsClosed() const;

	inline bool IsHullDoor() const
	{
		DOORTYPE doorType = DoorType();
		return doorType == DOOR_Hull || doorType == DOOR_HullLift || doorType == DOOR_Launcher;
	}

	bool IsJammed() const;

	bool IsLocked() const;

	// Some doors are specifically manual so they won't open automatically.
	inline bool IsManualDoor() const
	{
		return m_DoorType & 0x80;
	}

	bool IsOpen() const;

	// True if door responds to remote door remote signal from spacecraft.
	inline bool IsRemoteSpacecraft() const
	{
		return m_DoorType & 0x40;
	}

	// True if door responds to remote door remote signal from vehicles.
	inline bool IsRemoteVehicle() const
	{
		return m_DoorType & 0x20;
	}

	inline bool IsTurboLiftDoor() const
	{
		DOORTYPE doorType = DoorType();
		return doorType == DOOR_InteriorLift || doorType == DOOR_HullLift;
	}

	inline void Lock()
	{
		SetLock(true);
	}

	void Open();

	inline const AuPoint& OpLoc1() const
	{
		return m_OpLoc1;
	}

	inline const AuPoint& OpLoc2() const
	{
		return m_OpLoc2;
	}

	inline quint16 PartId() const
	{
		return m_PartId;
	}

	// Lock state can only be changed if door is currently Open, LockedOpen, Closed or LockedClosed.
	// Returns false if lock state could not be changed due to current door state.
	bool SetLock(bool locked);

	inline void SetOpLocs(const AuPoint& opLoc1, const AuPoint& opLoc2)
	{
		m_OpLoc1 = opLoc1;
		m_OpLoc2 = opLoc2;
	}

	inline void SetState(quint8 s)
	{
		m_State = s;
	}

	inline quint8 State() const
	{
		return m_State;
	}

	inline void Unlock()
	{
		SetLock(false);
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuScDoorState& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScDoorState& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuDoorStateHash : public QHash<quint16, AuScDoorState>
{
public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	bool AnyHullDoorOpen() const;

	bool AnyHullDoorUnlocked() const;

	bool AnyJammed(AuPoint& where) const;

	bool CloseAllDoors();

	// Returns true if all doors associated with 'partId' are closed on return, even if they were already closed.
	// Returns false if any of the part's doors are jammed open.
	bool CloseAssociatedDoors(quint16 partId);

	// Returns true if the state of the door changes
	bool CloseDoor(quint16 doorId);

	bool CloseHullDoors();

	quint16 ClosestClosedDoor(const AuPoint& loc, double& distance) const;

	quint16 ClosestDoor(const AuPoint& loc, double& distance) const;

	quint16 ClosestRemoteDoor(bool forVehicle, const AuPoint& loc, double& distance) const;

	quint16 ClosestOpenDoor(const AuPoint& loc, double& distance) const;

	// Returns the part id associated with a door, if any. e.g. lift or launcher.
	quint16 DoorAssociatedPart(quint16 doorId) const;

	inline quint8 DoorState(quint16 door) const
	{
		const_iterator di = find(door);
		if(di == cend())
			return 0;
		return (*di).State();
	}

	bool IsDoorJammed(quint16 doorId) const;

	bool IsDoorLocked(quint16 doorId) const;

	bool IsDoorOpen(quint16 doorId) const;

	bool IsHullDoor(quint16 doorId) const;

	bool IsTurboLiftDoor(quint16 doorId) const;

	// Returns true if the state of the door changes
	bool JamDoorClosed(quint16 doorId);

	// Returns true if the state of the door changes
	bool JamDoorOpen(quint16 doorId);

	quint64 MemoryUsage() const;

	// Opens all doors associated with 'partId'.
	bool OpenAssociatedDoors(quint16 partId);

	// Opens automatic doors within 'openDist' of the path segment from 'oloc' to 'nloc'.
	// 'opened' is filled with the ids of doors that are opened.
	bool OpenAutomaticDoors(const AuPoint& oloc, const AuPoint& nloc, double openDist, QSet<quint16>& opened);

	// Returns true if the state of the door changes
	bool OpenDoor(quint16 doorId);

	// Returns true if the state of the door changes
	bool UnjamDoor(quint16 doorId);
};

inline QDataStream& operator<<(QDataStream& out, const AuDoorStateHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuDoorStateHash& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Value is an AuScLandingGear::STATES enum value.

class AUDATA_DLL AuLandingGearStateHash
{
public:
	QHash<quint16, quint8>				m_States;

public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	bool IsAllExtended() const;

	// Returns true if anything changes.
	// Does not change stuck gear.
	bool Extend();

	quint64 MemoryUsage() const
	{
		return (sizeof(quint16) + sizeof(quint8)) * m_States.size();
	}

	// Returns true if anything changes.
	// Does not change stuck gear.
	bool Retract();

	inline quint8 State(quint16 partId) const
	{
		QHash<quint16, quint8>::const_iterator gi = m_States.find(partId);
		if(gi == m_States.cend())
			return 0;
		return *gi;
	}

	// Returns true if anything changes.
	bool Unstick();
};

inline QDataStream& operator<<(QDataStream& out, const AuLandingGearStateHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuLandingGearStateHash& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// At first, I derived this class from QSet but that caused multiple definitions of some QSet function.
class AUDATA_DLL AuScPartSet
{
	QSet<quint16>						m_Ids;

public:	// Constructors
	inline AuScPartSet()
	{
	}

	inline AuScPartSet(const AuScPartSet& c) : m_Ids(c.m_Ids)
	{
	}

public:	// Operators
	inline const AuScPartSet& operator=(const AuScPartSet& c)
	{
		m_Ids = c.m_Ids;
		return *this;
	}

	inline bool operator==(const AuScPartSet& c) const
	{
		if(m_Ids != c.m_Ids)
			return false;
		return true;
	}

	inline bool operator!=(const AuScPartSet& c) const
	{
		return !operator==(c);
	}

	inline void operator>>(QDataStream& out) const
	{
		out << (quint16)m_Ids.size();

		for(QSet<quint16>::const_iterator si = m_Ids.cbegin(); si != m_Ids.cend(); si++)
			out << *si;
	}

	inline void operator<<(QDataStream& in)
	{
		m_Ids.clear();

		quint16 c;
		in >> c;

		quint16 id;
		while(c--)
		{
			in >> id;
			m_Ids.insert(id);
		}
	}

public:	// Methods
	inline QSet<quint16>::const_iterator cbegin() const
	{
		return m_Ids.cbegin();
	}

	inline QSet<quint16>::const_iterator cend() const
	{
		return m_Ids.cend();
	}

	inline void clear()
	{
		m_Ids.clear();
	}

	inline bool contains(quint16 id) const
	{
		return m_Ids.contains(id);
	}

	inline bool empty() const
	{
		return m_Ids.empty();
	}

	inline QSet<quint16>::iterator insert(quint16 id)
	{
		return m_Ids.insert(id);
	}

	quint64 MemoryUsage() const;

	static int RegisterMetaType();

	inline int size() const
	{
		return m_Ids.size();
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuScPartSet& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScPartSet& o)
{
	o << in;
	return in;
}

Q_DECLARE_METATYPE(AuScPartSet)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScReportSysVol
{
	quint16								m_Module;				// COMMODITYID of module installed into the system.
	quint8								m_Quality;				// Quality of the module.
	quint32								m_Volume;				// Hull volume allocated to the system, in cubic meters.

public:	// Constructors
	inline AuScReportSysVol(COMMODITYID module, quint8 quality, quint32 volumeOfHull)
	{
		Q_ASSERT(quality);
		m_Module = module;
		m_Quality = quality;
		m_Volume = volumeOfHull;
	}

	inline AuScReportSysVol()
	{
		m_Module = CID_Invalid;
		m_Quality = 1;
		m_Volume = 0;
	}

	inline AuScReportSysVol(const AuScReportSysVol& c)
	{
		m_Module = c.m_Module;
		m_Quality = c.m_Quality;
		m_Volume = c.m_Volume;
	}

public:	// Operators
	inline const AuScReportSysVol& operator=(const AuScReportSysVol& c)
	{
		m_Module = c.m_Module;
		m_Quality = c.m_Quality;
		m_Volume = c.m_Volume;
		return *this;
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	inline bool operator==(const AuScReportSysVol& c) const
	{
		if(m_Module != c.m_Module)
			return false;
		if(m_Quality != c.m_Quality)
			return false;
		if(m_Volume != c.m_Volume)
			return false;
		return true;
	}

	inline bool operator!=(const AuScReportSysVol& c) const
	{
		return operator==(c);
	}

public:	// Methods
	inline COMMODITYID Module() const
	{
		return (COMMODITYID)m_Module;
	}

	inline quint8 Quality() const
	{
		return m_Quality;
	}

	inline void SetModule(COMMODITYID module)
	{
		m_Module = module;
	}

	inline void SetQuality(quint8 quality)
	{
		Q_ASSERT(quality);
		m_Quality = quality;
	}

	inline void SetVolume(quint32 volume)
	{
		m_Volume = volume;
	}

	inline quint32 Volume() const
	{
		return m_Volume;
	}

	inline float VolumeCuFt() const
	{
		return m_Volume * CubicFeetPerCubicMeter();
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuScReportSysVol& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScReportSysVol& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScReportSysVolHash : public QHash<quint16, AuScReportSysVol>
{
public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	inline void SetQuality(quint8 quality)
	{
		for(iterator si = begin(); si != end(); si++)
			(*si).SetQuality(quality);
	}

	quint64 MemoryUsage() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuScReportSysVolHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScReportSysVolHash& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScReportTurret
{
	quint16								m_Module;				// COMMODITYID of module installed into the turret.
	quint16								m_RoomId;				// Room void that contains the operator location.
	quint8								m_Quality;				// Quality of the module.

public:	// Constructors
	inline AuScReportTurret(COMMODITYID module, quint8 quality, quint16 roomId)
	{
		Q_ASSERT(quality);
		m_Module = module;
		m_RoomId = roomId;
		m_Quality = quality;
	}

	inline AuScReportTurret()
	{
		m_Module = CID_Invalid;
		m_RoomId = 0;
		m_Quality = 1;
	}

	inline AuScReportTurret(const AuScReportTurret& c)
	{
		m_Module = c.m_Module;
		m_RoomId = c.m_RoomId;
		m_Quality = c.m_Quality;
	}

public:	// Operators
	inline const AuScReportTurret& operator=(const AuScReportTurret& c)
	{
		m_Module = c.m_Module;
		m_RoomId = c.m_RoomId;
		m_Quality = c.m_Quality;
		return *this;
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	inline bool operator==(const AuScReportTurret& c) const
	{
		if(m_Module != c.m_Module)
			return false;
		if(m_RoomId != c.m_RoomId)
			return false;
		if(m_Quality != c.m_Quality)
			return false;
		return true;
	}

	inline bool operator!=(const AuScReportTurret& c) const
	{
		return operator==(c);
	}

public:	// Methods
	inline COMMODITYID Module() const
	{
		return (COMMODITYID)m_Module;
	}

	inline quint8 Quality() const
	{
		return m_Quality;
	}

	inline quint16 RoomId() const
	{
		return m_RoomId;
	}

	inline void SetModule(COMMODITYID module)
	{
		m_Module = module;
	}

	inline void SetQuality(quint8 quality)
	{
		Q_ASSERT(quality);
		m_Quality = quality;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuScReportTurret& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScReportTurret& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScReportTurretHash : public QHash<quint16, AuScReportTurret>
{
public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	quint64 MemoryUsage() const;

	inline void SetQuality(quint8 quality)
	{
		for(iterator si = begin(); si != end(); si++)
			(*si).SetQuality(quality);
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuScReportTurretHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScReportTurretHash& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScReportStation
{
	QString								m_Desc;					// AuScPart::ReportDesc
	quint16								m_Module;				// COMMODITYID of module installed into the station.
	quint8								m_PartType;				// AuScPart::PART_TYPE of station.
	quint16								m_RoomId;				// Room void that contains the station.
	quint8								m_Quality;				// Quality of the module.
	quint8								m_WhichEffect;			// Which effect for fire control station.

public:	// Constructors
	// 'partType' is a AuScPart::PART_TYPE;
	inline AuScReportStation(COMMODITYID module, quint8 quality, quint8 partType, const QString& desc, quint16 roomId,
			quint8 whichEffect)
	{
		Q_ASSERT(quality);
		m_Desc = desc;
		m_Module = module;
		m_PartType = partType;
		m_RoomId = roomId;
		m_Quality = quality;
		m_WhichEffect = whichEffect;
	}

	inline AuScReportStation()
	{
		m_Module = CID_Invalid;
		m_PartType = 0;
		m_RoomId = 0;
		m_Quality = 1;
		m_WhichEffect = 0;
	}

	inline AuScReportStation(const AuScReportStation& c)
	{
		m_Desc = c.m_Desc;
		m_Module = c.m_Module;
		m_PartType = c.m_PartType;
		m_RoomId = c.m_RoomId;
		m_Quality = c.m_Quality;
		m_WhichEffect = c.m_WhichEffect;
	}

public:	// Operators
	inline const AuScReportStation& operator=(const AuScReportStation& c)
	{
		m_Desc = c.m_Desc;
		m_Module = c.m_Module;
		m_PartType = c.m_PartType;
		m_RoomId = c.m_RoomId;
		m_Quality = c.m_Quality;
		m_WhichEffect = c.m_WhichEffect;
		return *this;
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	inline bool operator==(const AuScReportStation& c) const
	{
		if(m_Desc != c.m_Desc)
			return false;
		if(m_Module != c.m_Module)
			return false;
		if(m_PartType != c.m_PartType)
			return false;
		if(m_RoomId != c.m_RoomId)
			return false;
		if(m_Quality != c.m_Quality)
			return false;
		if(m_WhichEffect != c.m_WhichEffect)
			return false;
		return true;
	}

	inline bool operator!=(const AuScReportStation& c) const
	{
		return operator==(c);
	}

public:	// Methods
	inline const QString& Desc() const
	{
		return m_Desc;
	}

	// True if this is a medic station in 'roomId', false for surgery station and all other stations.
	bool IsMedic(quint16 roomId) const;

	quint64 MemoryUsage() const;

	inline COMMODITYID Module() const
	{
		return (COMMODITYID)m_Module;
	}

	// Returns a AuScPart::PART_TYPE.
	inline quint8 PartType() const
	{
		return m_PartType;
	}

	inline quint8 Quality() const
	{
		return m_Quality;
	}

	inline quint16 RoomId() const
	{
		return m_RoomId;
	}

	inline void SetModule(COMMODITYID module)
	{
		m_Module = module;
	}

	inline void SetQuality(quint8 quality)
	{
		Q_ASSERT(quality);
		m_Quality = quality;
	}

	inline void SetWhichEffect(quint8 whichEffect)
	{
		m_WhichEffect = whichEffect;
	}

	inline quint8 WhichEffect() const
	{
		return m_WhichEffect;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuScReportStation& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScReportStation& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScReportStationHash : public QHash<quint16, AuScReportStation>
{
public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	quint64 MemoryUsage() const;

	inline void SetQuality(quint8 quality)
	{
		for(iterator si = begin(); si != end(); si++)
			(*si).SetQuality(quality);
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuScReportStationHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScReportStationHash& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScReportTransporterStation
{
	QString								m_Desc;					// AuScPart::ReportDesc
	quint16								m_Module;				// COMMODITYID of module installed into the station.
	quint8								m_Pads;					// Number of pads.
	quint8								m_PartType;				// AuScPart::PART_TYPE of station.
	quint16								m_RoomId;				// Part id of the room void containing this station.
	quint8								m_Quality;				// Quality of the module.
	quint8								m_WhichEffect;			// Which effect to use for animating pads.

public:	// Constructors
	// 'partType' is a AuScPart::PART_TYPE;
	inline AuScReportTransporterStation(COMMODITYID module, quint8 quality, quint8 partType, const QString& desc, quint8 pads,
			quint16 roomId, quint8 whichEffect)
	{
		Q_ASSERT(quality);
		m_Desc = desc;
		m_Module = module;
		m_Pads = pads;
		m_PartType = partType;
		m_RoomId = roomId;
		m_Quality = quality;
		m_WhichEffect = whichEffect;
	}

	inline AuScReportTransporterStation()
	{
		m_Module = CID_Invalid;
		m_Pads = 0;
		m_PartType = 0;
		m_RoomId = 0;
		m_Quality = 1;
		m_WhichEffect = 0;
	}

	inline AuScReportTransporterStation(const AuScReportTransporterStation& c)
	{
		m_Desc = c.m_Desc;
		m_Module = c.m_Module;
		m_Pads = c.m_Pads;
		m_PartType = c.m_PartType;
		m_RoomId = c.m_RoomId;
		m_Quality = c.m_Quality;
		m_WhichEffect = c.m_WhichEffect;
	}

public:	// Operators
	inline const AuScReportTransporterStation& operator=(const AuScReportTransporterStation& c)
	{
		m_Desc = c.m_Desc;
		m_Module = c.m_Module;
		m_Pads = c.m_Pads;
		m_PartType = c.m_PartType;
		m_RoomId = c.m_RoomId;
		m_Quality = c.m_Quality;
		m_WhichEffect = c.m_WhichEffect;
		return *this;
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	inline bool operator==(const AuScReportTransporterStation& c) const
	{
		if(m_Desc != c.m_Desc)
			return false;
		if(m_Module != c.m_Module)
			return false;
		if(m_Pads != c.m_Pads)
			return false;
		if(m_PartType != c.m_PartType)
			return false;
		if(m_RoomId != c.m_RoomId)
			return false;
		if(m_Quality != c.m_Quality)
			return false;
		if(m_WhichEffect != c.m_WhichEffect)
			return false;
		return true;
	}

	inline bool operator!=(const AuScReportTransporterStation& c) const
	{
		return operator==(c);
	}

public:	// Methods
	inline const QString& Desc() const
	{
		return m_Desc;
	}

	inline quint32 Hits() const
	{
		return m_Pads;
	}

	quint64 MemoryUsage() const;

	inline COMMODITYID Module() const
	{
		return (COMMODITYID)m_Module;
	}

	inline quint8 Pads() const
	{
		return m_Pads;
	}

	// Returns a AuScPart::PART_TYPE.
	inline quint8 PartType() const
	{
		return m_PartType;
	}

	inline quint8 Quality() const
	{
		return m_Quality;
	}

	inline quint16 RoomId() const
	{
		return m_RoomId;
	}

	inline void SetModule(COMMODITYID module)
	{
		m_Module = module;
	}

	inline void SetQuality(quint8 quality)
	{
		Q_ASSERT(quality);
		m_Quality = quality;
	}

	inline void SetWhichEffect(quint8 whichEffect)
	{
		m_WhichEffect = whichEffect;
	}

	inline quint8 WhichEffect() const
	{
		return m_WhichEffect;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuScReportTransporterStation& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScReportTransporterStation& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScReportTransporterStationHash : public QHash<quint16, AuScReportTransporterStation>
{
public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	bool AnyInRoom(quint16 roomId) const;

	// Returns the room id of any ambiguous transporter rooms, that is a room that contains more than one transporter station.
	quint16 CheckAmbiguousRoom() const;

	quint64 MemoryUsage() const;

	inline void SetQuality(quint8 quality)
	{
		for(iterator si = begin(); si != end(); si++)
			(*si).SetQuality(quality);
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuScReportTransporterStationHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScReportTransporterStationHash& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScReportGameTable
{
	quint16								m_RoomId;				// Room void that contains the station.

public:	// Constructors
	// 'partType' is a AuScPart::PART_TYPE;
	inline AuScReportGameTable(quint16 roomId)
	{
		m_RoomId = roomId;
	}

	inline AuScReportGameTable()
	{
		m_RoomId = 0;
	}

	inline AuScReportGameTable(const AuScReportGameTable& c)
	{
		m_RoomId = c.m_RoomId;
	}

public:	// Operators
	inline const AuScReportGameTable& operator=(const AuScReportGameTable& c)
	{
		m_RoomId = c.m_RoomId;
		return *this;
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	inline bool operator==(const AuScReportGameTable& c) const
	{
		if(m_RoomId != c.m_RoomId)
			return false;
		return true;
	}

	inline bool operator!=(const AuScReportGameTable& c) const
	{
		return operator==(c);
	}

public:	// Methods
	inline quint16 RoomId() const
	{
		return m_RoomId;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuScReportGameTable& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScReportGameTable& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScReportGameTableHash : public QHash<quint16, AuScReportGameTable>
{
public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	bool AnyInRoom(quint16 roomId) const;

	quint64 MemoryUsage() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuScReportGameTableHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScReportGameTableHash& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScReportServicePanel
{
	QString								m_Desc;					// AuScPart::ReportDesc
	quint8								m_PartType;				// AuScPart::PART_TYPE of station.

public:	// Constructors
	// 'partType' is a AuScPart::PART_TYPE;
	inline AuScReportServicePanel(quint8 partType, const QString& desc)
	{
		m_Desc = desc;
		m_PartType = partType;
	}

	inline AuScReportServicePanel()
	{
		m_PartType = 0;
	}

	inline AuScReportServicePanel(const AuScReportServicePanel& c)
	{
		m_Desc = c.m_Desc;
		m_PartType = c.m_PartType;
	}

public:	// Operators
	inline const AuScReportServicePanel& operator=(const AuScReportServicePanel& c)
	{
		m_Desc = c.m_Desc;
		m_PartType = c.m_PartType;
		return *this;
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	inline bool operator==(const AuScReportServicePanel& c) const
	{
		if(m_Desc != c.m_Desc)
			return false;
		if(m_PartType != c.m_PartType)
			return false;
		return true;
	}

	inline bool operator!=(const AuScReportServicePanel& c) const
	{
		return operator==(c);
	}

public:	// Methods
	inline const QString& Desc() const
	{
		return m_Desc;
	}

	quint64 MemoryUsage() const;

	// Returns a AuScPart::PART_TYPE.
	inline quint8 PartType() const
	{
		return m_PartType;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuScReportServicePanel& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScReportServicePanel& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScReportServicePanelHash : public QHash<quint16, AuScReportServicePanel>
{
public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	quint64 MemoryUsage() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuScReportServicePanelHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScReportServicePanelHash& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScReportStatusDisplay
{
	quint16								m_AssociatedPartId;		// Associated part id for berth name decal.
	QString								m_Desc;					// AuScPart::ReportDesc
	quint8								m_PartType;				// AuScPart::PART_TYPE of station.
	QString								m_RoomName;				// Room name for life support display.

public:	// Constructors
	// 'partType' is a AuScPart::PART_TYPE;
	inline AuScReportStatusDisplay(quint8 partType, const QString& desc, const QString& roomName, quint16 associatedPartId)
	{
		m_AssociatedPartId = associatedPartId;
		m_Desc = desc;
		m_PartType = partType;
		m_RoomName = roomName;
	}

	inline AuScReportStatusDisplay()
	{
		m_AssociatedPartId = 0;
		m_PartType = 0;
	}

	inline AuScReportStatusDisplay(const AuScReportStatusDisplay& c)
	{
		m_AssociatedPartId = c.m_AssociatedPartId;
		m_Desc = c.m_Desc;
		m_PartType = c.m_PartType;
		m_RoomName = c.m_RoomName;
	}

public:	// Operators
	inline const AuScReportStatusDisplay& operator=(const AuScReportStatusDisplay& c)
	{
		m_AssociatedPartId = c.m_AssociatedPartId;
		m_Desc = c.m_Desc;
		m_PartType = c.m_PartType;
		m_RoomName = c.m_RoomName;
		return *this;
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	inline bool operator==(const AuScReportStatusDisplay& c) const
	{
		if(m_AssociatedPartId != c.m_AssociatedPartId)
			return false;
		if(m_Desc != c.m_Desc)
			return false;
		if(m_PartType != c.m_PartType)
			return false;
		if(m_RoomName != c.m_RoomName)
			return false;
		return true;
	}

	inline bool operator!=(const AuScReportStatusDisplay& c) const
	{
		return operator==(c);
	}

public:	// Methods
	inline quint16 AssociatedPartId() const
	{
		return m_AssociatedPartId;
	}

	inline const QString& Desc() const
	{
		return m_Desc;
	}

	// Returns true for empire flag, empire name, capt name and ship name, which are handled like status displays.
	bool IsDecal() const;

	quint64 MemoryUsage() const;

	// Returns a AuScPart::PART_TYPE.
	inline quint8 PartType() const
	{
		return m_PartType;
	}

	inline const QString& RoomName() const
	{
		return m_RoomName;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuScReportStatusDisplay& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScReportStatusDisplay& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScReportStatusDisplayHash : public QHash<quint16, AuScReportStatusDisplay>
{
public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	quint64 MemoryUsage() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuScReportStatusDisplayHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScReportStatusDisplayHash& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuPartArchitectHash
{
	QHash<AuId, QString>				m_Arch;					// Architects key=id, value=name

public:	// Constructors/destructors
	AuPartArchitectHash();

	AuPartArchitectHash(const AuPartArchitectHash& c);

public:	// Operators
	const AuPartArchitectHash& operator=(const AuPartArchitectHash& c);

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	bool operator==(const AuPartArchitectHash& c) const;

	inline bool operator!=(const AuPartArchitectHash& c) const
	{
		return !operator==(c);
	}

public:	// Methods
	inline bool Empty() const
	{
		return m_Arch.empty();
	}

	inline bool Contains(const AuId& id) const
	{
		return m_Arch.contains(id);
	}

	inline QHash<AuId, QString>::const_iterator Cbegin() const
	{
		return m_Arch.cbegin();
	}

	inline QHash<AuId, QString>::const_iterator Cend() const
	{
		return m_Arch.cend();
	}

	inline QHash<AuId, QString>::const_iterator Find(const AuId& id) const
	{
		return m_Arch.find(id);
	}

	inline QHash<AuId, QString>::iterator Insert(const AuId& id, const QString& name)
	{
		return m_Arch.insert(id, name);
	}

	quint64 MemoryUsage() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuPartArchitectHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuPartArchitectHash& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Key is turbo lift id, value is current stop name.

class AUDATA_DLL AuLiftHash : public QHash<quint16, QString>
{
public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	quint64 MemoryUsage() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuLiftHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuLiftHash& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuLiftStop
{
	quint16								m_DoorId;

public:	// Constructors
	inline AuLiftStop(quint16 doorId)
	{
		m_DoorId = doorId;
	}

	inline AuLiftStop()
	{
		m_DoorId = 0;
	}

	inline AuLiftStop(const AuLiftStop& c)
	{
		m_DoorId = c.m_DoorId;
	}

public:	// Operators
	inline const AuLiftStop& operator=(const AuLiftStop& c)
	{
		m_DoorId = c.m_DoorId;
		return *this;
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	inline bool operator==(const AuLiftStop& c) const
	{
		if(m_DoorId != c.m_DoorId)
			return false;
		return true;
	}

	inline bool operator!=(const AuLiftStop& c) const
	{
		return operator==(c);
	}

public:	// Methods
	inline quint16 DoorId() const
	{
		return m_DoorId;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuLiftStop& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuLiftStop& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Key is stop name, value is information about the stop.

class AUDATA_DLL AuLiftStopHash : public QHash<QString, AuLiftStop>
{
public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	QString DoorStopName(quint16 doorId) const;

	quint64 MemoryUsage() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuLiftStopHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuLiftStopHash& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuScReportLiftHash : public QHash<quint16, AuLiftStopHash>
{
public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	quint64 MemoryUsage() const;
};

inline QDataStream& operator<<(QDataStream& out, const AuScReportLiftHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuScReportLiftHash& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuScReportHelpers_H
