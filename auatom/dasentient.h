//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dasentient.h,v 1.193 2022/05/17 16:35:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaSentient_H
#define DaSentient_H

#include "aucombat.h"
#include "augear.h"
#include "auroster.h"
#include "ausensorblip.h"
#include "auskill.h"
#include "dacreature.h"
#include "davessel.h"

class AuCommPacket;
class AuCommBeamAtom;
class AuCommBeamCoordinate;
class AuCommBeamPlanet;
class AuCommBeamShip;
class AuCommBeamTransporter;
class AuCommBeamVehicleAtom;
class AuCommBeamVehicleCoordinate;
class AuCommBeamVehiclePlanet;
class AuCommBeamVehicleShip;
class AuCommBeamVehicleTransporter;
class AuCommChat;
class AuCommCitizenOrder;
class AuCommCitizenPosture;
class AuCommPacket;
class AuEmojiList;
class AuScRoute;
class DaSentient;
class DaWorldLL;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuAIOrder
{
public:
	AuPoint								m_Loc;					// Location relative to the order target.
	quint16								m_StationId;			// Id of station when going to a station.
	AuId								m_Target;				// All orders have a target associated with them. See audef.h
	QString								m_Text;					// Order text. Room name for go to room order.
	AuTime								m_Time;					// Time the last order was issued to me.
	CITIZEN_ORDER						m_Type;					// The specific kind of order. See audef.h

public:	// Constructors
	AuAIOrder()
	{
		m_Type = ORDER_Stay;
	}

	AuAIOrder(CITIZEN_ORDER order, const AuId& target, const AuPoint& loc, quint16 stationId, const QString& text)
	{
		m_Loc = loc;
		m_StationId = stationId;
		m_Target = target;
		m_Text = text;
		m_Time.Set();
		m_Type = order;
	}

	AuAIOrder(const AuAIOrder& c)
	{
		m_Loc = c.m_Loc;
		m_StationId = c.m_StationId;
		m_Target = c.m_Target;
		m_Text = c.m_Text;
		m_Time = c.m_Time;
		m_Type = c.m_Type;
	}

public:	// Operators
	AuAIOrder& operator=(const AuAIOrder& c)
	{
		m_Loc = c.m_Loc;
		m_StationId = c.m_StationId;
		m_Target = c.m_Target;
		m_Text = c.m_Text;
		m_Time = c.m_Time;
		m_Type = c.m_Type;
		return *this;
	}

	void operator>>(QDataStream& outStream) const;

	void operator<<(QDataStream& inStream);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuAIOrderArray : public QList<AuAIOrder>
{
public:	// Operators
	void operator>>(QDataStream& outStream) const;
	void operator<<(QDataStream& inStream);

public:
	inline bool HasOrder(CITIZEN_ORDER order) const
	{
		for(const_iterator oi = constBegin(); oi != constEnd(); oi++)
			if((*oi).m_Type == order)
				return true;
		return false;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuAIOrderArray& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuAIOrderArray& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSentientDecision
{
public:
	enum DECISION
	{
		D_Invalid,
		D_PickUp,
		D_ReloadWeapon,
		D_ShipAnnounceOrder,
		D_ShipBoardCrew,
		D_ShipBoardCryo,
		D_ShipBoardPassengers,
		D_ShipBoardTroops,
		D_ShipBuyCargoCategory,
		D_ShipBuyCargoCommodity,
		D_ShipCloseAllHullDoors,
		D_ShipCrewPosture,
		D_ShipDebarkCryo,
		D_ShipDebarkPassengers,
		D_ShipDoorClose,
		D_ShipDoorOpen,
		D_ShipExterminateRats,
		D_ShipLootShip,
		D_ShipMissionAttack,
		D_ShipMissionOrderComplete,
		D_ShipMissionState,
		D_ShipPatientDefib,
		D_ShipPatientHeal,
		D_ShipPatientTreat,
		D_ShipPilotReportContacts,
		D_ShipPilotPolicy,
		D_ShipReconcileRoster,
		D_ShipScuttle,
		D_ShipSellCargoCategory,
		D_ShipSellCargoCommodity,
		D_ShipSensorsReportContacts,
		D_ShipShieldsPolicy,
		D_ShipTransporterState,
		D_ShipWeaponFire,
		D_ShipWeaponSetPower,
		D_COUNT,						// LAST ITEM
	};

public:
	AuSentientDecision()
	{
	}

	virtual ~AuSentientDecision()
	{
	}

	static AuSentientDecision* Create(DECISION type);

	virtual void Execute(DaSentient* sent, int msElapsed) = 0;

	// Some decisions do not have to be executed because the AI will make that decision again frequently, such as when piloting.
	inline static bool MustWaitForExecution(DECISION decision)
	{
		switch(decision)
		{
		case D_COUNT:
		case D_Invalid:
			return false;

		case D_PickUp:
		case D_ReloadWeapon:
		case D_ShipAnnounceOrder:
		case D_ShipBoardCrew:
		case D_ShipBoardCryo:
		case D_ShipBoardPassengers:
		case D_ShipBoardTroops:
		case D_ShipBuyCargoCategory:
		case D_ShipBuyCargoCommodity:
		case D_ShipCloseAllHullDoors:
		case D_ShipCrewPosture:
		case D_ShipDebarkCryo:
		case D_ShipDebarkPassengers:
		case D_ShipDoorClose:
		case D_ShipDoorOpen:
		case D_ShipExterminateRats:
		case D_ShipLootShip:
		case D_ShipMissionAttack:
		case D_ShipMissionOrderComplete:
		case D_ShipMissionState:
		case D_ShipPatientDefib:
		case D_ShipPatientHeal:
		case D_ShipPatientTreat:
		case D_ShipPilotPolicy:
		case D_ShipPilotReportContacts:
		case D_ShipReconcileRoster:
		case D_ShipScuttle:
		case D_ShipSellCargoCategory:
		case D_ShipSellCargoCommodity:
		case D_ShipSensorsReportContacts:
		case D_ShipShieldsPolicy:
		case D_ShipTransporterState:
		case D_ShipWeaponFire:
		case D_ShipWeaponSetPower:
			break;
		}
		return true;
	}

	virtual DECISION Type() const = 0;
};

class AuSDPickUp : public AuSentientDecision
{
	AuId								m_PickUpId;

public:
	AuSDPickUp(const AuId& pickUpId)
	{
		m_PickUpId = pickUpId;
	}

	AuSDPickUp()
	{
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(const AuId& pickUpId)
	{
		m_PickUpId = pickUpId;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_PickUp;
	}
};

class AuSDReloadWeapon : public AuSentientDecision
{
	AMMOTYPE							m_AmmoType;
	AuGearPos							m_UsePos;
	INVENTORYSUB						m_UseSub;

public:
	AuSDReloadWeapon(AMMOTYPE ammoType, const AuGearPos& usePos, INVENTORYSUB useSub)
	{
		m_AmmoType = ammoType;
		m_UsePos = usePos;
		m_UseSub = useSub;
	}

	AuSDReloadWeapon()
	{
		m_AmmoType = AMMO_None;
		m_UseSub = IPS_Item;
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(AMMOTYPE ammoType, const AuGearPos& usePos, INVENTORYSUB useSub)
	{
		m_AmmoType = ammoType;
		m_UsePos = usePos;
		m_UseSub = useSub;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ReloadWeapon;
	}
};

class AuSDShipSellCargoCommodity : public AuSentientDecision
{
	COMMODITYID							m_CargoCID;
	int									m_CargoToUnload;

public:
	AuSDShipSellCargoCommodity(COMMODITYID cargoCID, int cargoToUnload)
	{
		m_CargoCID = cargoCID;
		m_CargoToUnload = cargoToUnload;
	}

	AuSDShipSellCargoCommodity()
	{
		m_CargoCID = CID_Invalid;
		m_CargoToUnload = 0;
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(COMMODITYID cargoCID, int cargoToUnload)
	{
		m_CargoCID = cargoCID;
		m_CargoToUnload = cargoToUnload;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipSellCargoCommodity;
	}
};

class AuSDShipBuyCargoCommodity : public AuSentientDecision
{
	COMMODITYID							m_CargoCID;
	int									m_CargoToLoad;

public:
	AuSDShipBuyCargoCommodity(COMMODITYID cargoCID, int cargoToLoad)
	{
		m_CargoCID = cargoCID;
		m_CargoToLoad = cargoToLoad;
	}

	AuSDShipBuyCargoCommodity()
	{
		m_CargoCID = CID_Invalid;
		m_CargoToLoad = 0;
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(COMMODITYID cargoCID, int cargoToLoad)
	{
		m_CargoCID = cargoCID;
		m_CargoToLoad = cargoToLoad;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipBuyCargoCommodity;
	}
};

class AuSDShipBuyCargoCategory : public AuSentientDecision
{
	int									m_CargoWanted;
	QList<COMMODITYID>					m_Commodities;
	int									m_EachWanted;
	int									m_SpaceAvail;

public:
	AuSDShipBuyCargoCategory(const QList<COMMODITYID>& commodities, int eachWanted, int spaceAvail, int cargoWanted) :
		m_Commodities(commodities)
	{
		m_CargoWanted = cargoWanted;
		m_EachWanted = eachWanted;
		m_SpaceAvail = spaceAvail;
	}

	AuSDShipBuyCargoCategory()
	{
		m_CargoWanted = 0;
		m_EachWanted = 0;
		m_SpaceAvail = 0;
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(const QList<COMMODITYID>& commodities, int eachWanted, int spaceAvail, int cargoWanted)
	{
		m_CargoWanted = cargoWanted;
		m_Commodities = commodities;
		m_EachWanted = eachWanted;
		m_SpaceAvail = spaceAvail;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipBuyCargoCategory;
	}
};

class AuSDShipSellCargoCategory : public AuSentientDecision
{
	QList<COMMODITYID>					m_Commodities;
	int									m_MissionAmount;
	int									m_MissionAmountType;

public:
	AuSDShipSellCargoCategory(const QList<COMMODITYID>& commodities, int missionAmountType, int missionAmount) :
		m_Commodities(commodities)
	{
		m_MissionAmount = missionAmount;
		m_MissionAmountType = missionAmountType;
	}

	AuSDShipSellCargoCategory()
	{
		m_MissionAmount = 0;
		m_MissionAmountType = 0;
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(const QList<COMMODITYID>& commodities, int missionAmountType, int missionAmount)
	{
		m_Commodities = commodities;
		m_MissionAmount = missionAmount;
		m_MissionAmountType = missionAmountType;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipSellCargoCategory;
	}
};

class AuSDShipBoardCrew : public AuSentientDecision
{
	int									m_MissionHeadCount;

public:
	AuSDShipBoardCrew(int missionHeadCount)
	{
		m_MissionHeadCount = missionHeadCount;
	}

	AuSDShipBoardCrew()
	{
		m_MissionHeadCount = 0;
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(int missionHeadCount)
	{
		m_MissionHeadCount = missionHeadCount;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipBoardCrew;
	}
};

class AuSDShipBoardCryo : public AuSentientDecision
{
	int									m_MissionHeadCount;

public:
	AuSDShipBoardCryo(int missionHeadCount)
	{
		m_MissionHeadCount = missionHeadCount;
	}

	AuSDShipBoardCryo()
	{
		m_MissionHeadCount = 0;
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(int missionHeadCount)
	{
		m_MissionHeadCount = missionHeadCount;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipBoardCryo;
	}
};

class AuSDShipBoardPassengers : public AuSentientDecision
{
	int									m_MissionHeadCount;

public:
	AuSDShipBoardPassengers(int missionHeadCount)
	{
		m_MissionHeadCount = missionHeadCount;
	}

	AuSDShipBoardPassengers()
	{
		m_MissionHeadCount = 0;
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(int missionHeadCount)
	{
		m_MissionHeadCount = missionHeadCount;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipBoardPassengers;
	}
};

class AuSDShipBoardTroops : public AuSentientDecision
{
	int									m_MissionHeadCount;

public:
	AuSDShipBoardTroops(int missionHeadCount)
	{
		m_MissionHeadCount = missionHeadCount;
	}

	AuSDShipBoardTroops()
	{
		m_MissionHeadCount = 0;
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(int missionHeadCount)
	{
		m_MissionHeadCount = missionHeadCount;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipBoardTroops;
	}
};

class AuSDShipDebarkCryo : public AuSentientDecision
{
	int									m_CryoBerths;

public:
	AuSDShipDebarkCryo(int cryoBerths)
	{
		m_CryoBerths = cryoBerths;
	}

	AuSDShipDebarkCryo()
	{
		m_CryoBerths = 0;
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(int cryoBerths)
	{
		m_CryoBerths = cryoBerths;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipDebarkCryo;
	}
};

class AuSDShipDebarkPassengers : public AuSentientDecision
{
	int									m_Passengers;

public:
	AuSDShipDebarkPassengers(int passengers)
	{
		m_Passengers = passengers;
	}

	AuSDShipDebarkPassengers()
	{
		m_Passengers = 0;
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(int passengers)
	{
		m_Passengers = passengers;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipDebarkPassengers;
	}
};

class AuSDShipReconcileRoster : public AuSentientDecision
{
public:
	AuSDShipReconcileRoster()
	{
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipReconcileRoster;
	}
};

class AuSDShipCrewPosture : public AuSentientDecision
{
	CITIZEN_POSTURE						m_Posture;

public:
	AuSDShipCrewPosture(CITIZEN_POSTURE posture)
	{
		m_Posture = posture;
	}

	AuSDShipCrewPosture()
	{
		m_Posture = POSTURE_Offensive;
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(CITIZEN_POSTURE posture)
	{
		m_Posture = posture;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipCrewPosture;
	}
};

class AuSDShipShieldsPolicy : public AuSentientDecision
{
	SHIELDFUEL_POLICY					m_FuelPolicy;
	SHIELD_POLICY						m_UpPolicy;

public:
	AuSDShipShieldsPolicy(SHIELDFUEL_POLICY fuelPolicy, SHIELD_POLICY upPolicy)
	{
		m_FuelPolicy = fuelPolicy;
		m_UpPolicy = upPolicy;
	}

	AuSDShipShieldsPolicy()
	{
		m_FuelPolicy = ShieldUpUnlessNoFuel;
		m_UpPolicy = ShieldUp;
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(SHIELDFUEL_POLICY fuelPolicy, SHIELD_POLICY upPolicy)
	{
		m_FuelPolicy = fuelPolicy;
		m_UpPolicy = upPolicy;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipShieldsPolicy;
	}
};

class AuSDShipPilotPolicy : public AuSentientDecision
{
	PILOT_UP_POLICY					m_UpPolicy;

public:
	AuSDShipPilotPolicy(PILOT_UP_POLICY upPolicy)
	{
		m_UpPolicy = upPolicy;
	}

	AuSDShipPilotPolicy()
	{
		m_UpPolicy = UpAwayFromGravity;
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(PILOT_UP_POLICY upPolicy)
	{
		m_UpPolicy = upPolicy;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipPilotPolicy;
	}
};

class AuSDShipMissionAttack : public AuSentientDecision
{
	AuId								m_TargetId;
	QString								m_TargetName;

public:
	AuSDShipMissionAttack(const AuId& targetId, const QString& targetName)
	{
		m_TargetId = targetId;
		m_TargetName = targetName;
	}

	AuSDShipMissionAttack()
	{
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(const AuId& targetId, const QString& targetName)
	{
		m_TargetId = targetId;
		m_TargetName = targetName;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipMissionAttack;
	}
};

class AuSDShipMissionOrderComplete : public AuSentientDecision
{
	int									m_CompletionCode;

public:
	AuSDShipMissionOrderComplete(int completionCode)
	{
		m_CompletionCode = completionCode;
	}

	AuSDShipMissionOrderComplete()
	{
		m_CompletionCode = 0;
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(int completionCode)
	{
		m_CompletionCode = completionCode;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipMissionOrderComplete;
	}
};

class AuSDShipMissionState : public AuSentientDecision
{
	bool								m_Active;

public:
	AuSDShipMissionState(bool active)
	{
		m_Active = active;
	}

	AuSDShipMissionState()
	{
		m_Active = false;
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(bool active)
	{
		m_Active = active;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipMissionState;
	}
};

class AuSDShipAnnounceOrder : public AuSentientDecision
{
public:
	AuSDShipAnnounceOrder()
	{
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipAnnounceOrder;
	}
};

class AuSDShipScuttle : public AuSentientDecision
{
public:
	AuSDShipScuttle()
	{
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipScuttle;
	}
};

class AuSDShipCloseAllHullDoors : public AuSentientDecision
{
public:
	AuSDShipCloseAllHullDoors()
	{
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipCloseAllHullDoors;
	}
};

class AuSDShipDoorClose : public AuSentientDecision
{
	int									m_DoorIndex;
	AuId								m_ShipId;

public:
	AuSDShipDoorClose(const AuId& shipId, int doorIndex)
	{
		m_DoorIndex = doorIndex;
		m_ShipId = shipId;
	}

	AuSDShipDoorClose()
	{
		m_DoorIndex = 0;
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(const AuId& shipId, int doorIndex)
	{
		m_DoorIndex = doorIndex;
		m_ShipId = shipId;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipDoorClose;
	}
};

class AuSDShipDoorOpen : public AuSentientDecision
{
	int									m_DoorIndex;
	AuId								m_ShipId;

public:
	AuSDShipDoorOpen(const AuId& shipId, int doorIndex)
	{
		m_DoorIndex = doorIndex;
		m_ShipId = shipId;
	}

	AuSDShipDoorOpen()
	{
		m_DoorIndex = 0;
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(const AuId& shipId, int doorIndex)
	{
		m_DoorIndex = doorIndex;
		m_ShipId = shipId;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipDoorOpen;
	}
};

class AuSDShipExterminateRats : public AuSentientDecision
{
public:
	AuSDShipExterminateRats()
	{
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipExterminateRats;
	}
};

class AuSDShipTransporterState : public AuSentientDecision
{
	bool								m_On;
	int									m_RoomIndex;

public:
	AuSDShipTransporterState(int roomIndex, bool on)
	{
		m_On = on;
		m_RoomIndex = roomIndex;
	}

	AuSDShipTransporterState()
	{
		m_On = false;
		m_RoomIndex = 0;
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(int roomIndex, bool on)
	{
		m_On = on;
		m_RoomIndex = roomIndex;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipTransporterState;
	}
};

class AuSDShipPatientDefib : public AuSentientDecision
{
	AuId								m_PatientId;

public:
	AuSDShipPatientDefib(const AuId& patientId)
	{
		m_PatientId = patientId;
	}

	AuSDShipPatientDefib()
	{
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(const AuId& patientId)
	{
		m_PatientId = patientId;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipPatientDefib;
	}
};

class AuSDShipPatientHeal : public AuSentientDecision
{
	AuId								m_PatientId;

public:
	AuSDShipPatientHeal(const AuId& patientId)
	{
		m_PatientId = patientId;
	}

	AuSDShipPatientHeal()
	{
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(const AuId& patientId)
	{
		m_PatientId = patientId;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipPatientHeal;
	}
};

class AuSDShipPatientTreat : public AuSentientDecision
{
	AuId								m_PatientId;

public:
	AuSDShipPatientTreat(const AuId& patientId)
	{
		m_PatientId = patientId;
	}

	AuSDShipPatientTreat()
	{
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(const AuId& patientId)
	{
		m_PatientId = patientId;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipPatientTreat;
	}
};

class AuSDShipPilotReportContacts : public AuSentientDecision
{
public:
	AuSDShipPilotReportContacts()
	{
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipPilotReportContacts;
	}
};

class AuSDShipSensorsReportContacts : public AuSentientDecision
{
	AuSensorBlipList					m_SensorBlips;
	double								m_SensorRange;

public:
	AuSDShipSensorsReportContacts(const AuSensorBlipList& sensorBlips, double sensorRange) : m_SensorBlips(sensorBlips)
	{
		m_SensorRange = sensorRange;
	}

	AuSDShipSensorsReportContacts()
	{
		m_SensorRange = 0;
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(const AuSensorBlipList& sensorBlips, double sensorRange)
	{
		m_SensorBlips = sensorBlips;
		m_SensorRange = sensorRange;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipSensorsReportContacts;
	}
};

class AuSDShipWeaponFire : public AuSentientDecision
{
	int									m_HarvestIndex;
	bool								m_IsHarvester;
	AuId								m_TargetId;
	AuPoint								m_TargetSysLoc;
	int									m_UnitsFired;
	quint16								m_Which;

public:
	AuSDShipWeaponFire(quint16 which, int unitsFired, const AuId& targetId, const AuPoint& targetSysLoc, bool isHarvester,
			int harvestIndex)
	{
		m_HarvestIndex = harvestIndex;
		m_IsHarvester = isHarvester;
		m_TargetId = targetId;
		m_TargetSysLoc = targetSysLoc;
		m_UnitsFired = unitsFired;
		m_Which = which;
	}

	AuSDShipWeaponFire()
	{
		m_HarvestIndex = 0;
		m_IsHarvester = false;
		m_UnitsFired = 0;
		m_Which = 0;
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(quint16 which, int unitsFired, const AuId& targetId, const AuPoint& targetSysLoc, bool isHarvester,
			int harvestIndex)
	{
		m_HarvestIndex = harvestIndex;
		m_IsHarvester = isHarvester;
		m_TargetId = targetId;
		m_TargetSysLoc = targetSysLoc;
		m_UnitsFired = unitsFired;
		m_Which = which;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipWeaponFire;
	}
};

class AuSDShipWeaponSetPower : public AuSentientDecision
{
	quint16								m_Which;

public:
	AuSDShipWeaponSetPower(quint16 which)
	{
		m_Which = which;
	}

	AuSDShipWeaponSetPower()
	{
		m_Which = 0;
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(quint16 which)
	{
		m_Which = which;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipWeaponSetPower;
	}
};

class AuSDShipLootShip : public AuSentientDecision
{
	AuId								m_VictimShipId;

public:
	AuSDShipLootShip(const AuId& victimShipId)
	{
		m_VictimShipId = victimShipId;
	}

	AuSDShipLootShip()
	{
	}

	virtual void Execute(DaSentient* sent, int msElapsed);

	inline void SetParameters(const AuId& victimShipId)
	{
		m_VictimShipId = victimShipId;
	}

	virtual DECISION Type() const
	{
		return StaticType();
	}

	static DECISION StaticType()
	{
		return D_ShipLootShip;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL DaSentient : public DaCreature
{
friend class AuSentientDecision;
friend class AuSDPickUp;
friend class AuSDReloadWeapon;
friend class AuSDShipBuyCargoCategory;
friend class AuSDShipBuyCargoCommodity;
friend class AuSDShipExterminateRats;
friend class AuSDShipLootShip;
friend class AuSDShipPatientDefib;
friend class AuSDShipPatientHeal;
friend class AuSDShipPatientTreat;
friend class AuSDShipPilotReportContacts;
friend class AuSDShipSellCargoCategory;
friend class AuSDShipSellCargoCommodity;
friend class AuSDShipSensorsReportContacts;
friend class AuSDShipWeaponFire;
	// Sentient AI

	// AI variables are not stored in the db.
protected:
	enum RECENT_MEMORY
	{
		MEM_None						= 0x00,
		MEM_OnApproach					= 0x01,
		MEM_OrderAnnounced				= 0x02,
	};
	quint8								m_RecentMemory;			// RECENT_MEMORY bits.

private:
	mutable QReadWriteLock				m_ThinkLock;
	AuElapsedTimer						m_AIAttackTime;			// How long since I last attacked a target.
	bool								m_AIIsAlert;			// true when this citizen's senses are alert
	AuAIOrderArray						m_AIOrders;				// Orders to be followed by this citizen in sequence. Stored in db.
	CITIZEN_POSTURE						m_AIPosture;			// Threat stance. i.e. Offensive, DefendArea, DefendSelf.
	quint16								m_AIThinkRate;			// milliseconds between thoughts
	AuElapsedTimer						m_AIThinkTime;			// Time since last thought. Not stored in db.
	float								m_TurnZ;				// Amount of left/right turn remaining.

	// Decisions made during Think and related data.
	// The decision queue eliminates a bottleneck when using nu and delete in the core of the decision making process, particularly
	// due to the slowness of delete. This logic depends on an assumption that each kind of decision in the queue is unique until
	// the queue is emptied, i.e. the AI does not make the same decision with different parameters in a given think cycle.
	// A short static list of indexes is maintained in m_Decisions. This is the current decision queue. The index stored in the
	// decisions queue is the DECISION enum value.
	// The decision index references into m_DecisionsTable, a table that contains one of each possible type of decision, indexed by
	// DECISION enum value. This is where the parameters of each decision are stored.
	// When a decision is made, the pointer to the corresponding object in this table is returned so the caller can configure the
	// parameters.
	unsigned int						m_DecisionCount;		// Number of valid decisions in m_Decisions.
	QVector<AuSentientDecision::DECISION>	m_Decisions;		// The queue of my decisions. Indices into m_DecisionTable.

private:
	// The m_DecisionsTable and each entry is allocated when requested, so most AI will probably have sparsely populated tables
	// depending on the kinds of decisions they make.
	// These tables are not serialized.
	mutable QMutex						m_DecisionTableLock;
	AuSentientDecision**				m_DecisionTable;		// Array of D_COUNT pointers, allocated when needed.

protected:
	mutable QReadWriteLock				m_SentientLock;
	AuId								m_AIRecentCollide;		// Atom id last hit while moving.
	AuId								m_AIRecentParent;		// Parent id of locations in m_AIRecentPath.
	AuPointArray						m_AIRecentPath;			// Recent locations in my path so I can watch for stuck situations.
	AuScRoute*							m_AIScRoute;			// Path in AuScDesign designs.
	int									m_AIStrategy;			// Current strategy.
	AuElapsedTimer						m_AIStrategyTime;		// How long the strategy has been current.
	int									m_EnemyBlips;			// Sensor/weapon operator current count of enemy contacts.
	bool								m_InEnemyRange;			// true if I am in range of an enemy bay weapon
	AuId								m_LastPlunder;			// Last ship that was plundered, so we don't plunder it repeatedly.
	QString								m_LastShipName;			// Name of last ship I released a berth from.
	float								m_MoveDistLimit;		// When non-zero, limits forward move distance for proper stop.
	AuElapsedTimer						m_tmLastLook;			// Time of last call to LookForThreats. Not stored in db.
	AuElapsedTimer						m_tmLastScan;			// Time of last sensor scan. Not stored in db.
	AuElapsedTimer						m_tmLastThought;		// Time of last thought. Not stored in db.

protected:
	mutable QReadWriteLock				m_SkillsLock;
	AuElapsedTimer						m_SkillsChg;
	AuSkillSet							m_Skills;

protected:
	mutable QReadWriteLock				m_TalkLock;
	AuId								m_Interlocutor;			// Person with whom I am conversing.
	AuJobOffer							m_JobOffer;
	QStringList							m_LastSaid;				// Not stored in db.
	AuElapsedTimer						m_tmLastSpoke;			// Time of last verbal statement.

public:
	// Person
	DaSentient(const AuDNABase& dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand, const QString& name,
			quint32 empire, GROWTHSTAGE stage);
	// Citizen
	DaSentient(const AuDNABase& dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand, const QString& name,
			GROWTHSTAGE stage);
	DaSentient(const AuSpecies& species, AuDNABase::CHROMATID chroma, HANDEDNESS hand, const QString& name,
			GROWTHSTAGE stage);
	DaSentient();
	virtual ~DaSentient();

protected:
	DaSentient(const DaSentient& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:	// AI
	virtual double AccountBalance() const
	{
		return 0;
	}

	// Gear management.
	// Returns false if the item could not be added.
	bool AddGearItem(const AuItem& cItem);

	// The item will replace an existing item and cause it to be dropped if it is already at the inventory position, unless
	// it is a container.
	inline void AddGearItemAt(AuGearSlot::SLOTTYPE slotType, const AuItem& cItem)
	{
		AuGear* gear = LockGearForWrite();
		if(gear)
			AddGearItemAt(*gear, slotType, cItem, true);
		UnlockGear();
	}

	// The item will replace an existing item and, if 'dropIt' is true, will cause the replaced item to be dropped if it is used
	// for the same purpose. i.e. a helmet will cause me to drop my current helmet and use the nu helmet instead.
	// 'useIt' is 0=no, 1=yes, 2=only put in dominant hand if it's a weapon
	// 'useIt' puts weapons in my dominant hand or wear clothing and armor.
	// 'useIt' is 0 to put the item somewhere besides my dominant hand, clothing or armor positions.
	// Returns false if unable to add the item.
	virtual bool AddGearItemUseDrop(const AuItem &cItem, int useIt, bool dropIt)
	{
		Q_UNUSED(cItem);
		Q_UNUSED(useIt);
		Q_UNUSED(dropIt);
		return false;
	}

	// Add a nu order to the stack of orders. The nu order becomes the current order until it is completed.
	// 'stationId' is the console station id, when going to man a station.
	// 'orderText' is the room name for go to room order.
	void AmendOrder(CITIZEN_ORDER order, const AuId& atom, const AuPoint& loc, quint16 stationId,
			const QString& orderText=QString(""));

	virtual void AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim);

	// Notifies human officers and crew of the current order.
	// At the time of the call, the current mission order has just become active under my command. I could have just been given
	// the con.
	void AnnounceMissionOrder(DaVessel* ship);

	// Notifies human officers that the current mission order is about to be completed or skipped.
	// Called on the officer with the con whenever the current mission order is about to be declared complete by the ship.
	// At the time of the call, the current mission order is the order that will be completed.
	// 'completionCode' is only known to be non-zero when passed by DaSentient::CommandShip.
	void AnnounceMissionOrderComplete(DaVessel* ship, int completionCode);

	virtual AuId AssignedShip() const
	{
		return AuId(0);
	}

	virtual AuRosterEntry::TYPE AssignedShipBerthType() const
	{
		return AuRosterEntry::Invalid;
	}

	AuSkill::SKILL BestCrewStationSkill() const;

	virtual bool BestWeaponAtRange(float targetRange, AuDNABase::WEAPONLOC& loc, AuCombatWeapon& weapon, int& ammoCur,
			int& ammoFull) const;

	virtual void BuildingSurrendered(quint32 surrenderingEmpire, quint32 toEmpire);

	virtual bool CanUseGear(const AuItem& /*item*/, AuGearSlot::SLOTTYPE /*slotType*/) const
	{
		return true;
	}

	// Citizens choose a hair and shave style at random, except Unshaven, Unkempt, etc.
	virtual void ChooseHairStyle();

	quint32 CommandExperiencePoints() const;

	void ConsumeGear(const AuGearPos& pos, quint32 qty);

	void ConsumeGearNoLock(AuGear& gear, const AuGearPos& pos, quint32 qty);

	// Consumes specific items from my gear.
	// 'itemToConsume.Count()' determines how many items to consume.
	// Returns the actual number of items consumed, which could be less than itemToConsume.Count().
	quint32 ConsumeItem(const AuItem& itemToConsume);

	// Returns true if text contains any euphemism for sex. e.g. "fuck", "make love", "have sex"
	// Returns false when playing solo to nerf solo sex.
	static bool ContainsEuphemismForSex(const QString& text);

	virtual void ConversationChoicePicked(DaPerson& person, qint8 pickValue);

	void Create();

	virtual void CreditAccount(double amount)
	{
		Q_UNUSED(amount);
	}

	quint32 CrewExperiencePoints() const;

	// Returns the index to the current inventory item in use. This base class returns the index of the first hand slot.
	// Returns AuGearPos() if there is no current item.
	// Locks gear to find the first item in hand.
	virtual AuGearPos CurrentItemPos() const;

	virtual INVENTORYSUB CurrentItemSub() const;

	virtual void DebitAccount(double amount)
	{
		Q_UNUSED(amount);
	}

	// Returns true if travel is ok, false if decided not to go.
	bool DecideWhetherToTravel(DaVessel* ship) const;

	inline unsigned int DecisionCount() const
	{
		m_ThinkLock.lockForRead();
		unsigned int v = m_DecisionCount;
		m_ThinkLock.unlock();
		return v;
	}

	virtual void Defibrillated(quint32 byEmpireId);

	virtual QString DescribeActivity() const;

	DaItem* DropGearItem(COMMODITYID cid);

	// Drops the item that is in the 'pos' position.
	// Returns the item that is dropped.
	DaItem* DropGearAt(const AuGearPos& pos);

	// Drops the item that is in the 'slotType' slot.
	// Returns the item that is dropped.
	DaItem* DropGearSlot(AuGearSlot::SLOTTYPE slotType);

	virtual quint32 ExperiencePointsForKilling() const;

	void FireWeapon();

	// Called after placement/spawn at nu job site, such as aboard a nuly assigned ship.
	virtual void GoToWork()
	{
	}

	// Looks in containers.
	inline bool HasItem(COMMODITYID cid) const
	{
		const AuGear* gear = LockGearForRead();
		bool v = gear && gear->HasItem(cid);
		UnlockGear();
		return v;
	}

	// Checks for a specific item in my inventory.
	// 'inscription' is only compared if 'inscription' is not blank.
	bool HasItemSpecific(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, const QString& inscription) const;

	// Only officers and demiavatars ever have the con.
	virtual bool HasTheCon() const
	{
		return false;
	}

	virtual GALAXY HomeGalaxy() const
	{
		return GALAXY_Count;
	}

	virtual QString HomeSectorName() const
	{
		return QString();
	}

	virtual QString HomeSolarSystemName() const
	{
		return QString();
	}

	virtual BODY_TYPE HomeWorldBodyType() const
	{
		return BODY_TYPE_Count;
	}

	virtual QString HomeWorldName() const
	{
		return QString();
	}

	virtual void IncreaseSkill(AuSkill::SKILL skill, quint32 xp);

	inline AuId Interlocutor() const
	{
		m_TalkLock.lockForRead();
		AuId v(m_Interlocutor);
		m_TalkLock.unlock();
		return v;
	}

	// Returns true if this is an officer in a person's chain-of-command, could be demiavatar.
	// If true, the person is permitted to command this officer.
	virtual bool InChainOfCommand(const AuId& personId, quint32 personEmpire) const
	{
		Q_UNUSED(personId);
		Q_UNUSED(personEmpire);
		return false;
	}

	virtual bool IsInScene(const DaPerson* sceneAtom, const AuPoint& cameraEnvLoc) const;

	virtual bool IsInStoryInstance(quint32 storyInstance) const
	{
		Q_UNUSED(storyInstance);
		return false;
	}

	inline bool IsNpcCrew() const
	{
		return NpcRole() == ROLE_Crew;
	}

	// Returns true if my role is an officer, crew, troop, or space marine.
	inline bool IsNpcCrewmate() const
	{
		switch(NpcRole())
		{
		case ROLE_Citizen:
		case ROLE_IndigenousPerson:
		case ROLE_IndigenousElder:
		case ROLE_IndigenousHealer:
		case ROLE_IndigenousStargazer:
		case ROLE_Worker:
		case ROLE_Passenger:
		case ROLE_Count:
			break;
		case ROLE_Crew:
		case ROLE_Troop:
		case ROLE_Officer:
			return true;
		}
		return false;
	}

	// Returns true if my role is an officer, crew, troop, or space marine.
	inline bool IsNpcIndigenous() const
	{
		switch(NpcRole())
		{
		case ROLE_Citizen:
		case ROLE_Count:
		case ROLE_Worker:
		case ROLE_Passenger:
		case ROLE_Crew:
		case ROLE_Troop:
		case ROLE_Officer:
			break;
		case ROLE_IndigenousPerson:
		case ROLE_IndigenousElder:
		case ROLE_IndigenousHealer:
		case ROLE_IndigenousStargazer:
			return true;
		}
		return false;
	}

	// Returns true if my role is an officer.
	inline bool IsNpcOfficer() const
	{
		return NpcRole() == ROLE_Officer;
	}

	// Returns true if my role is a passenger.
	inline bool IsNpcPassenger() const
	{
		return NpcRole() == ROLE_Passenger;
	}

	// Returns true if my role is a troop or space marine.
	inline bool IsNpcTroop() const
	{
		return NpcRole() == ROLE_Troop;
	}

	inline bool IsNpcWorker() const
	{
		return NpcRole() == ROLE_Worker;
	}

	virtual bool IsProtectedByEnvironmentSuit() const;

	bool IsRouteEmpty() const;

	bool IsRouteDistanceLessThan(double closeEnough) const;

	// Returns experience points appropriate to my job. E.g. Crew returns CrewExperiencePoints.
	virtual quint32 JobExperiencePoints() const
	{
		return 0;
	}

	virtual const AuGear* LockGearForRead() const
	{
		return 0;
	}

	virtual void MovedByTurboLift();

	// Orient and OrientationSpeed have no effect unless this function returns true.
	virtual bool OkToOrient() const;

	// 'visage' is optional, may be 0. Specifies the final stationary pose for the mouth the same as if the message was suffixed
	// with the appropriate emoji|emote. 'visage' is only used if the message does not contain an emoji|emote suffix.
	void PoseMouthForSaying(const AuCommPacket& msg, const AuVisage* visage);	// Uses g_Au.Emojis
	void PoseMouthForSaying(const AuCommPacket& msg, const AuVisage* visage, const AuEmojiList& emojis);

	inline void RestartAIAttackTime()
	{
		m_ThinkLock.lockForWrite();
		m_AIAttackTime.restart();
		m_ThinkLock.unlock();
	}

	virtual void RxChatMessage(const AuCommPacket* msg, quint32& countSupplied, bool firstInGroup=true);

	inline AuElapsedTimer SkillsChangeTime() const
	{
		m_SkillsLock.lockForRead();
		AuElapsedTimer t(m_SkillsChg);
		m_SkillsLock.unlock();
		return t;
	}

	inline void SetAIIsAlert(bool isAlert)
	{
		m_ThinkLock.lockForWrite();
		m_AIIsAlert = isAlert;
		m_ThinkLock.unlock();
	}

	inline void StartThinking()
	{
		m_ThinkLock.lockForWrite();
		m_AIAttackTime.start();
		m_AIThinkTime.start();
		m_ThinkLock.unlock();

		UpdateThinkRate();			// Rate of thinking is based on current order.
	}

	// Called on environment atom to recursively cause all sentient atoms in the conversation address cone of 'talker' to start
	// listening on the Voice comm channel. See InConeOfAddress.
	virtual void StartVoiceConversationRecursive(const DaPerson& talker) const;

	// Clears the 'storyInstance' of a specific item in my inventory that is currently associated with a story instance.
	// This prevents the item from self destructing after the story instance ends.
	// 'inscription' is only compared if 'inscription' is not blank.
	void StoryItemMakePermanent(quint32 storyInstance, COMMODITYID cid, quint8 quality, quint32 empireOfOrigin,
			const QString& inscription);

	// 'drop' 0=default random drop, 1=never drop, 2=always drop
	// 'inscription' is only compared if 'inscription' is not blank.
	void StoryItemSetDropOnDeath(int drop, quint32 storyInstance, COMMODITYID cid, quint8 quality, quint32 empireOfOrigin,
			const QString& inscription);

	virtual void TakeTheCon()
	{
	}

	quint32 TotalExperiencePoints() const;

	quint32 TroopExperiencePoints() const;

	// Returns ms between shots based on skill.
	quint32 TurretFireMs() const;

	virtual void UnlockGear() const
	{
	}

	// NOTE: The item will be deleted from inventory if it is used up by this call.
	// Returns true if the item functions, though it may be used up by this last use.
	// Returns false if the item could not function because it needs ammo.
	// Returns false if there is no item at that position.
	inline bool UseGearItem(const AuGearPos& gearPos, INVENTORYSUB sub)
	{
		if(!gearPos.IsGear())
			return false;

		int itemFunctioned = -1;
		AuGear* gear = LockGearForWrite();
		if(gear)
		{
			AuItem* gearItem = gear->ItemAt(gearPos);
			if(gearItem)
			{
				quint32 oldCount = gearItem->Count();
				itemFunctioned = UseItem(*gearItem, sub);
				if(gearItem->Count() != oldCount)
				{
					// The count of items changed.
					if(gearItem->Count() == 0)
						gear->Erase(gearPos);	// The item was completely used up.

					// Need to update my current inventory weight carried.
					UpdateGearWeight();
				}
			}
		}
		UnlockGear();

		return itemFunctioned >= 0;
	}

	virtual void WeaponAtUsePos(const AuGearPos& pos, INVENTORYSUB sub, AuCombatWeapon& weapon, int& ammoCur, int& ammoFull) const;

	inline bool AIIsAlert() const
	{
		m_ThinkLock.lockForRead();
		bool isAlert = m_AIIsAlert;
		m_ThinkLock.unlock();
		return isAlert;
	}

	virtual bool IsAlert() const
	{
		return IsAIControlled() && AIIsAlert();
	}

	// returns true if this citizen is going to man a station, such as a control console or turret or surgery unit.
	bool IsGoingToConsole() const;

	// Returns true if currently executing any sleep order.
	inline bool IsNapping() const
	{
		switch(Order())
		{
		case ORDER_Sleep:
		case ORDER_SleepDayDream:
		case ORDER_SleepEat:
		case ORDER_SleepExercise:
		case ORDER_SleepGame:
		case ORDER_SleepLetter:
		case ORDER_SleepMovie:
		case ORDER_SleepMusic:
		case ORDER_SleepPine:
		case ORDER_SleepPhone:
		case ORDER_SleepRead:
		case ORDER_SleepTalk:
		case ORDER_SleepText:
		case ORDER_SleepTV:
		case ORDER_SleepWeb:
			return true;
		default:
			break;
		}
		return false;
	}

	inline const AuSkillSet& LockSkillsForRead() const
	{
		m_SkillsLock.lockForRead();
		return m_Skills;
	}

	inline void UnlockSkills() const
	{
		m_SkillsLock.unlock();
	}

	virtual void LogEvent(AuEventLogEntry::TYPE type, const QString& desc, const AuId& atomId=AuId())
	{
		Q_UNUSED(type);
		Q_UNUSED(desc);
		Q_UNUSED(atomId);
	}

	// Depending upon current posture, looks for threats.
	// Amends an attack order to go attack threats that are spotted.
	// 'returnToPost' true also amends an order to return to my current location after the attack order is completed.
	bool LookForThreats(bool returnToPost);

	// Returns the html header attached to all of this citizen's mail communications when aboard a ship.
	QString MailHeader(const DaVessel* ship, const QString& subject, const QString salutation) const;

	// Returns the html header attached to all of this citizen's mail communications when not aboard a ship.
	// NOTE: This function looks for a parent spacecraft and calls the other MailHeader function if it finds one.
	QString MailHeader(const QString& subject, const QString& salutation) const;

	virtual quint64 MemoryUsage() const;

	// Called on the officer with the con by the ship whenever the current mission order changes for any reason.
	// 'prevOrder' is the order that was current before the change. It will be a default constructed order if none was current.
	// Returns true if the officer wants to accept the nu mission order as current.
	// Returns false if the officer wants to reject the nu mission order, such as a repair order when the ship is not damaged.
	bool MissionOrderChanged(DaVessel* ship, const AuMissionOrder& prevOrder);

	virtual void OccupyBerth(DaVessel* /*ship*/)
	{
		qWarning() << "DaSentient::OccupyBerth" << Id() << Name() << "I don't know how to occupy a berth.";
	}

	void OperateEngineerConsole(DaVessel* ship);

	void OperateFireControlConsole(DaVessel* ship);

	void OperateNavigatorConsole(DaVessel* ship);

	void OperatePilotConsole(DaVessel* ship);

	void OperatePowerRelayConsole(DaVessel* ship);

	void OperateSensorConsole(DaVessel* ship);

	void OperateShieldConsole(DaVessel* ship);

	void OperateSurgeryConsole(const DaCapsule* ship);

	void OperateTransporterConsole(const DaCapsule* ship);

	void OperateTurret(DaVessel* ship);

	inline CITIZEN_ORDER Order() const
	{
		CITIZEN_ORDER order;
		m_ThinkLock.lockForRead();

		if(m_AIOrders.isEmpty())
			order = ORDER_Stay;
		else
			order = m_AIOrders.last().m_Type;

		m_ThinkLock.unlock();
		return order;
	}

	AuPoint OrderLoc() const;

	inline quint16 OrderStationId() const
	{
		quint16 orderStationId;
		m_ThinkLock.lockForRead();

		if(!m_AIOrders.isEmpty())
			orderStationId = m_AIOrders.last().m_StationId;
		else
			orderStationId = 0;

		m_ThinkLock.unlock();
		return orderStationId;
	}

	inline AuId OrderTarget() const
	{
		AuId id;
		m_ThinkLock.lockForRead();

		if(!m_AIOrders.isEmpty())
			id = m_AIOrders.last().m_Target;

		m_ThinkLock.unlock();
		return id;
	}

	// Returns true if this sentient has any orders. On the client, a non-zero order list is an indication of AI control.
	inline bool HasOrder() const
	{
		bool hasOrder;
		m_ThinkLock.lockForRead();

		hasOrder = !m_AIOrders.isEmpty();

		m_ThinkLock.unlock();
		return hasOrder;
	}

	inline bool HasOrder(CITIZEN_ORDER order) const
	{
		bool hasOrder;
		m_ThinkLock.lockForRead();

		hasOrder = m_AIOrders.HasOrder(order);

		m_ThinkLock.unlock();
		return hasOrder;
	}

	bool PirateMissionGoHome(DaVessel* ship);

	void PirateMissionOrderChanged(DaVessel* ship, const AuMissionOrder& prevOrder);

	bool PirateMissionStartHunting(DaVessel* ship);

	// Return the total number of experience points needed to get promoted to the next rank.
	// This is the total experience points needed for the rank, not the experience points remaining to get there.
	// 0 is returned if currently at the maximum rank.
	virtual quint32 PromotionGoal() const
	{
		return 0;
	}

	inline CITIZEN_POSTURE Posture() const
	{
		m_ThinkLock.lockForRead();
		CITIZEN_POSTURE posture = m_AIPosture;
		m_ThinkLock.unlock();
		return posture;
	}

	// The second order is considered the primary order.
	// The first order is always the default Stay fallback order.
	// Third and subsequent orders are always temporary orders, pending return to the primary order.
	inline CITIZEN_ORDER PrimaryOrder() const
	{
		CITIZEN_ORDER order;
		m_ThinkLock.lockForRead();

		if(m_AIOrders.isEmpty())
			order = ORDER_Stay;
		else if(m_AIOrders.count() == 1)
			order = m_AIOrders.last().m_Type;
		else
			order = m_AIOrders[1].m_Type;

		m_ThinkLock.unlock();
		return order;
	}

	virtual quint8 Rank() const
	{
		return 0;
	}

	virtual QString RankAbbr() const
	{
		return "";
	}

	// Rank when addressed by a superior. e.g. Sergeant instead of MSGT.
	virtual QString RankCrew() const
	{
		return "";
	}

	virtual QString RankIcon() const
	{
		return QString(":data/ContactNew.png");
	}

	virtual QString RankName() const
	{
		return "";
	}

	virtual bool Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari);

	virtual void ReleaseTheCon()
	{
	}

	// Used when conducting barter transactions. Removes cItem.Count() items from my inventory.
	void RemoveGearItems(const AuItem& cItem);

	// Select next order in order queue.
	void RequestOrder();

	// Call this function to restart the time this AI has been thinking about doing the current task.
	// e.g. A citizen's think time is restarted whenever a the current order changes.
	// e.g. A citizen's think time is restarted each time treatment is applied in a surgery unit so they will not decide to leave
	// until the doctor stops working on them.
	inline void RestartThinkTime()
	{
		m_ThinkLock.lockForWrite();
		m_AIThinkTime.restart();
		m_ThinkLock.unlock();
	}

	virtual void RxUpdate(QDataStream& fromServer, bool isFirstUpdate);

	// Sets scope data in msg appropriately.
	// Updates m_LastSaid, m_tmLastSpoke time and m_Interlocutor.
	void Say(AuCommPacket& msg, const AuId& toWho=AuId::NullId());

	// Leaves mouth in 'visage' when done, as if the message was suffixed with the appropriate emote.
	void Say(AuCommPacket& msg, const AuVisage& visage, const AuId& toWho);

	// Says 'msg' without repeating it, based on full text of the message.
	// The no repeat test is based on the last message in m_LastSaid, not all the messages there.
	// Returns false if 'msg' is a repeat of the last thing I said.
	bool SayNoRepeat(AuCommPacket& msg, const AuVisage& visage, const AuId& toWho=AuId::NullId());

	void SetOrder(CITIZEN_ORDER order, const AuId& orderTarget, const AuPoint& orderLoc, const QString& orderText=QString(""));

	virtual void SetGarrisonBd(const AuId& bdId)
	{
		Q_UNUSED(bdId);
	}

	// Sets the posture of this unit to be the default posture when told to garrison
	virtual void SetGarrisonPosture()
	{
		m_ThinkLock.lockForWrite();
		m_AIPosture = POSTURE_DefendSelf;
		m_ThinkLock.unlock();
	}

	inline void SetPosture(CITIZEN_POSTURE posture)
	{
		m_ThinkLock.lockForWrite();
		m_AIPosture = posture;
		m_ThinkLock.unlock();
	}

	// 'notified' is an output parameter, filled with atom id's who are sent notifications.
	virtual void ShipLogFinal(const DaVessel* ship, const QString& reasonFinal, const QString& killerName, COMMODITYID weaponCID,
			quint8 weaponQ, AuIdList& notified)
	{
		Q_UNUSED(ship);
		Q_UNUSED(reasonFinal);
		Q_UNUSED(killerName);
		Q_UNUSED(weaponCID);
		Q_UNUSED(weaponQ);
		Q_UNUSED(notified);
	}

	virtual QString SpouseName() const
	{
		return QString();
	}

	// Issue myself a sleep order amendment, of some random type.
	void TakeANap();

	// 'orderStationId' is only used when going to man a station.
	virtual void TakeOrder(const AuCommPacket& msg, CITIZEN_ORDER order, const AuId& orderTarget, const AuPoint& orderLoc,
			bool firstInGroup, const QString& orderText=QString(""));

	// Only atoms that return true to IsTicker will have their Tick function called.
	virtual bool IsTicker() const;

	virtual void Tick(const AuElapsedTimer& last, const AuElapsedTimer& now);

	virtual unsigned int IsTocker() const;

	virtual void Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans);

	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
			const DaPerson& sceneAtom);

	// Wakes this citizen up if it is napping.
	void WakeUp();

	virtual COMMODITYID WornArmor() const;

	virtual COMMODITYID WornClothing() const;

	virtual COMMODITYID WornHat() const;

protected:	// AI
	// Returns false if the item could not be added.
	bool AddGearItemAt(AuGear& gear, AuGearSlot::SLOTTYPE slotType, const AuItem& cItem, bool dropIt);

	// Returns false if the item could not be added.
	virtual bool AddGearItemNoLock(AuGear& gear, const AuItem& cItem);

	inline AuElapsedTimer AIAttackTime() const
	{
		m_ThinkLock.lockForRead();
		AuElapsedTimer v(m_AIAttackTime);
		m_ThinkLock.unlock();
		return v;
	}

	inline quint16 AIThinkRate() const
	{
		m_ThinkLock.lockForRead();
		quint16 v = m_AIThinkRate;
		m_ThinkLock.unlock();
		return v;
	}

	inline AuId AIRecentCollide() const
	{
		m_SentientLock.lockForRead();
		AuId v(m_AIRecentCollide);
		m_SentientLock.unlock();
		return v;
	}

	// Sets m_AIIsAlert. Does not notify anybody.
	void BecomeAlert();

	// Sets m_AIIsAlert and notifies comrades in the area.
	// 'sayWhat' is what to say to report becoming alert.
	// When 'sayWhat' is not blank, nearby troops are alerted if they are in my empire and not manning a turret or console.
	void BecomeAlert(const QString& sayWhat);

	virtual void BodyHasDied(COMMODITYID weaponCID, quint8 weaponQ, WEAPONEFFECT effect1, WEAPONEFFECT effect2,
			const QString& killerName);

	int BuyCargo(DaVessel* ship, COMMODITYID cargoCID, int cargoCount, DaAtomList& tradeCities);

	void ClearInterlocutor()
	{
		m_TalkLock.lockForWrite();
		m_Interlocutor = AuId::NullId();
		m_TalkLock.unlock();
	}

	virtual void ClearShipAssignment()
	{
	}

	void CommandShip(DaVessel* ship);

	// Consumes specific items from my gear.
	// If 'curPos' is valid and 'curSub' is IPS_Item, the current item is consumed first.
	// After that, the entire gear is searched.
	// 'itemToConsume.Count()' determines how many items to consume.
	// Returns the actual number of items consumed, which could be less than itemToConsume.Count().
	quint32 ConsumeItemNoLock(AuGear& gear, const AuGearPos& curPos, INVENTORYSUB curSub, const AuItem& itemToConsume);
	quint32 ConsumeItemRecursiveNoLock(AuItem& item, const AuItem& itemToConsume, quint32 countConsumed);

	// Notifies chain-of-command of my location when I am not in command or I am lost.
	// ContactLog may result in my suicide due to abandonment.
	virtual void ContactLog()
	{
	}

	inline void DecidePickUp(const AuId& pickUpId)
	{
		AuSDPickUp* d = dynamic_cast<AuSDPickUp*>(LockMakeDecision(AuSDPickUp::StaticType()));
		if(d)
			d->SetParameters(pickUpId);
		UnlockDecision();
	}

	inline void DecideReloadWeapon(AMMOTYPE ammoType, const AuGearPos& usePos, INVENTORYSUB useSub)
	{
		AuSDReloadWeapon* d = dynamic_cast<AuSDReloadWeapon*>(LockMakeDecision(AuSDReloadWeapon::StaticType()));
		if(d)
			d->SetParameters(ammoType, usePos, useSub);
		UnlockDecision();
	}

	inline void DecideShipAnnounceOrder()
	{
		LockMakeDecision(AuSDShipAnnounceOrder::StaticType());
		UnlockDecision();
	}

	inline void DecideShipBoardCrew(int missionHeadCount)
	{
		AuSDShipBoardCrew* d = dynamic_cast<AuSDShipBoardCrew*>(LockMakeDecision(AuSDShipBoardCrew::StaticType()));
		if(d)
			d->SetParameters(missionHeadCount);
		UnlockDecision();
	}

	inline void DecideShipBoardCryo(int missionHeadCount)
	{
		AuSDShipBoardCryo* d =
			dynamic_cast<AuSDShipBoardCryo*>(LockMakeDecision(AuSDShipBoardCryo::StaticType()));
		if(d)
			d->SetParameters(missionHeadCount);
		UnlockDecision();
	}

	inline void DecideShipBoardPassengers(int missionHeadCount)
	{
		AuSDShipBoardPassengers* d =
			dynamic_cast<AuSDShipBoardPassengers*>(LockMakeDecision(AuSDShipBoardPassengers::StaticType()));
		if(d)
			d->SetParameters(missionHeadCount);
		UnlockDecision();
	}

	inline void DecideShipBoardTroops(int missionHeadCount)
	{
		AuSDShipBoardTroops* d = dynamic_cast<AuSDShipBoardTroops*>(LockMakeDecision(AuSDShipBoardTroops::StaticType()));
		if(d)
			d->SetParameters(missionHeadCount);
		UnlockDecision();
	}

	inline void DecideShipBuyCargoCategory(const QList<COMMODITYID>& commodities, int eachWanted, int spaceAvail, int cargoWanted)
	{
		AuSDShipBuyCargoCategory* d =
			dynamic_cast<AuSDShipBuyCargoCategory*>(LockMakeDecision(AuSDShipBuyCargoCategory::StaticType()));
		if(d)
			d->SetParameters(commodities, eachWanted, spaceAvail, cargoWanted);
		UnlockDecision();
	}

	inline void DecideShipBuyCargoCommodity(COMMODITYID cargoCID, int cargoToLoad)
	{
		AuSDShipBuyCargoCommodity* d =
			dynamic_cast<AuSDShipBuyCargoCommodity*>(LockMakeDecision(AuSDShipBuyCargoCommodity::StaticType()));
		if(d)
			d->SetParameters(cargoCID, cargoToLoad);
		UnlockDecision();
	}

	inline void DecideShipCloseAllHullDoors()
	{
		LockMakeDecision(AuSDShipCloseAllHullDoors::StaticType());
		UnlockDecision();
	}

	inline void DecideShipCrewPosture(CITIZEN_POSTURE posture)
	{
		AuSDShipCrewPosture* d = dynamic_cast<AuSDShipCrewPosture*>(LockMakeDecision(AuSDShipCrewPosture::StaticType()));
		if(d)
			d->SetParameters(posture);
		UnlockDecision();
	}

	inline void DecideShipDebarkCryo(int cryoBerths)
	{
		AuSDShipDebarkCryo* d =
			dynamic_cast<AuSDShipDebarkCryo*>(LockMakeDecision(AuSDShipDebarkCryo::StaticType()));
		if(d)
			d->SetParameters(cryoBerths);
		UnlockDecision();
	}

	inline void DecideShipDebarkPassengers(int passengers)
	{
		AuSDShipDebarkPassengers* d =
			dynamic_cast<AuSDShipDebarkPassengers*>(LockMakeDecision(AuSDShipDebarkPassengers::StaticType()));
		if(d)
			d->SetParameters(passengers);
		UnlockDecision();
	}

	inline void DecideShipDoorClose(const AuId& shipId, int doorIndex)
	{
		AuSDShipDoorClose* d = dynamic_cast<AuSDShipDoorClose*>(LockMakeDecision(AuSDShipDoorClose::StaticType()));
		if(d)
			d->SetParameters(shipId, doorIndex);
		UnlockDecision();
	}

	inline void DecideShipDoorOpen(const AuId& shipId, int doorIndex)
	{
		AuSDShipDoorOpen* d = dynamic_cast<AuSDShipDoorOpen*>(LockMakeDecision(AuSDShipDoorOpen::StaticType()));
		if(d)
			d->SetParameters(shipId, doorIndex);
		UnlockDecision();
	}

	inline void DecideShipExterminateRats()
	{
		LockMakeDecision(AuSDShipExterminateRats::StaticType());
		UnlockDecision();
	}

	inline void DecideShipLootShip(const AuId& victimShipId)
	{
		AuSDShipLootShip* d = dynamic_cast<AuSDShipLootShip*>(LockMakeDecision(AuSDShipLootShip::StaticType()));
		if(d)
			d->SetParameters(victimShipId);
		UnlockDecision();
	}

	inline void DecideShipMissionAttack(const AuId& targetId, const QString& targetName)
	{
		AuSDShipMissionAttack* d = dynamic_cast<AuSDShipMissionAttack*>(LockMakeDecision(AuSDShipMissionAttack::StaticType()));
		if(d)
			d->SetParameters(targetId, targetName);
		UnlockDecision();
	}

	inline void DecideShipMissionOrderComplete(int completionCode)
	{
		AuSDShipMissionOrderComplete* d =
			dynamic_cast<AuSDShipMissionOrderComplete*>(LockMakeDecision(AuSDShipMissionOrderComplete::StaticType()));
		if(d)
			d->SetParameters(completionCode);
		UnlockDecision();
	}

	inline void DecideShipMissionState(bool active)
	{
		AuSDShipMissionState* d = dynamic_cast<AuSDShipMissionState*>(LockMakeDecision(AuSDShipMissionState::StaticType()));
		if(d)
			d->SetParameters(active);
		UnlockDecision();
	}

	inline void DecideShipPatientDefib(const AuId& patientId)
	{
		AuSDShipPatientDefib* d = dynamic_cast<AuSDShipPatientDefib*>(LockMakeDecision(AuSDShipPatientDefib::StaticType()));
		if(d)
			d->SetParameters(patientId);
		UnlockDecision();
	}

	inline void DecideShipPatientHeal(const AuId& patientId)
	{
		AuSDShipPatientHeal* d = dynamic_cast<AuSDShipPatientHeal*>(LockMakeDecision(AuSDShipPatientHeal::StaticType()));
		if(d)
			d->SetParameters(patientId);
		UnlockDecision();
	}

	inline void DecideShipPatientTreat(const AuId& patientId)
	{
		AuSDShipPatientTreat* d = dynamic_cast<AuSDShipPatientTreat*>(LockMakeDecision(AuSDShipPatientTreat::StaticType()));
		if(d)
			d->SetParameters(patientId);
		UnlockDecision();
	}

	inline void DecideShipPilotReportContacts()
	{
		LockMakeDecision(AuSDShipPilotReportContacts::StaticType());
		UnlockDecision();
	}

	inline void DecideShipReconcileRoster()
	{
		LockMakeDecision(AuSDShipReconcileRoster::StaticType());
		UnlockDecision();
	}

	inline void DecideShipScuttle()
	{
		LockMakeDecision(AuSDShipScuttle::StaticType());
		UnlockDecision();
	}

	inline void DecideShipSellCargoCategory(const QList<COMMODITYID>& commodities, int missionAmountType, int missionAmount)
	{
		AuSDShipSellCargoCategory* d =
			dynamic_cast<AuSDShipSellCargoCategory*>(LockMakeDecision(AuSDShipSellCargoCategory::StaticType()));
		if(d)
			d->SetParameters(commodities, missionAmountType, missionAmount);
		UnlockDecision();
	}

	inline void DecideShipSellCargoCommodity(COMMODITYID cargoCID, int cargoToUnload)
	{
		AuSDShipSellCargoCommodity* d =
			dynamic_cast<AuSDShipSellCargoCommodity*>(LockMakeDecision(AuSDShipSellCargoCommodity::StaticType()));
		if(d)
			d->SetParameters(cargoCID, cargoToUnload);
		UnlockDecision();
	}

	inline void DecideShipSensorsReportContacts(const AuSensorBlipList& sensorBlips, double sensorRange)
	{
		AuSDShipSensorsReportContacts* d =
			dynamic_cast<AuSDShipSensorsReportContacts*>(LockMakeDecision(AuSDShipSensorsReportContacts::StaticType()));
		if(d)
			d->SetParameters(sensorBlips, sensorRange);
		UnlockDecision();
	}

	inline void DecideShipPilotPolicy(PILOT_UP_POLICY upPolicy)
	{
		AuSDShipPilotPolicy* d = dynamic_cast<AuSDShipPilotPolicy*>(LockMakeDecision(AuSDShipPilotPolicy::StaticType()));
		if(d)
			d->SetParameters(upPolicy);
		UnlockDecision();
	}

	inline void DecideShipShieldsPolicy(SHIELDFUEL_POLICY fuelPolicy, SHIELD_POLICY upPolicy)
	{
		AuSDShipShieldsPolicy* d = dynamic_cast<AuSDShipShieldsPolicy*>(LockMakeDecision(AuSDShipShieldsPolicy::StaticType()));
		if(d)
			d->SetParameters(fuelPolicy, upPolicy);
		UnlockDecision();
	}

	inline void DecideShipTransporterState(int roomIndex, bool on)
	{
		AuSDShipTransporterState* d =
			dynamic_cast<AuSDShipTransporterState*>(LockMakeDecision(AuSDShipTransporterState::StaticType()));
		if(d)
			d->SetParameters(roomIndex, on);
		UnlockDecision();
	}

	inline void DecideShipWeaponFire(quint16 which, int unitsFired, const AuId& targetId, const AuPoint& targetSysLoc,
			bool isHarvester, int harvestIndex)
	{
		AuSDShipWeaponFire* d = dynamic_cast<AuSDShipWeaponFire*>(LockMakeDecision(AuSDShipWeaponFire::StaticType()));
		if(d)
			d->SetParameters(which, unitsFired, targetId, targetSysLoc, isHarvester, harvestIndex);
		UnlockDecision();
	}

	inline void DecideShipWeaponSetPower(quint16 which)
	{
		AuSDShipWeaponSetPower* d = dynamic_cast<AuSDShipWeaponSetPower*>(LockMakeDecision(AuSDShipWeaponSetPower::StaticType()));
		if(d)
			d->SetParameters(which);
		UnlockDecision();
	}

	void DoAttackTarget(DaAtom* targetAtom, const AuPoint& sightedLoc, bool sightedShield, bool targetIsFriendlyOrDying,
			const AuCombatWeapon& weapon, bool useSpecWpn, const AuGearPos& usePos, INVENTORYSUB useSub);

	void DoEnterVehicle(DaVehicle* vehicle);

	void DoMoveInCrew();

	void DoMoveInOfficer();

	void DoMoveInPassenger();

	void DoMoveInTroop();

	void DoPose(const AuPose& pose);

	void DoShipArrivedAtDestination(DaVessel* ship, const AuId& destId);

	void DoShipArrivedAtDestinationStar(DaVessel* ship, const AuId& destId, bool announce, const DaSolarSystem* system,
			const DaStar* primaryStar);

	void DoShipBrakeState(DaVessel* ship, bool brakeF, bool brakeLV);

	void DoShipDisengageFTLDrive(DaVessel* ship, int announce, quint16 whichFTL, const QString& destName);

	void DoShipEngageFTLDrive(DaVessel* ship, int announce, const QString& destName, quint16 whichFTL);

	void DoShipEngineTune(DaVessel* ship);

	void DoShipFireTurret(DaVessel* ship, const AuId& target);

	void DoShipFTLDriveSetting(DaVessel* ship, quint8 setting, quint16 whichFTL);

	void DoShipFTLDriveState(DaVessel* ship, bool on, quint16 whichFTL);

	void DoShipGravityDriveState(DaVessel* ship, DaVessel::GRAVITY_DRIVE_STATE state);

	void DoShipInteriorLightState(DaVessel* ship, INTERIOR_LIGHT state, const QString& roomName);

	void DoShipKillThrottles(DaVessel* ship);

	// 'extended' is true to extend landing gear, false to retract the landing gear.
	void DoShipLandingGear(DaVessel* ship, bool extended);

	void DoShipLightState(DaVessel* ship, bool headlight, bool navLight, bool beacon, bool strobe);

	void DoShipMoorState(DaVessel* ship, bool moor);

	void DoShipPowerPlantState(DaVessel* ship, bool on);

	void DoShipPowerRelayTune(DaVessel* ship);

	void DoShipReloadTurret(DaVessel* ship, quint16 which);

	void DoShipRollPitchYaw(DaVessel* ship, float roll, float pitch, float yaw);

	void DoShipSensorsLock(DaVessel* ship, const AuId& lockId);

	void DoShipSensorsState(DaVessel* ship, bool on);

	void DoShipShieldsBalance(DaVessel* ship);

	void DoShipShieldsState(DaVessel* ship, DaVessel::SHIELD_STATE state, bool announce, const QString& sayWhy);

	void DoShipThrottleState(DaVessel* ship, qint8 state);

	void DoShipThrusterLateralState(DaVessel* ship, qint8 thrust);

	void DoShipThrusterLongitudinalState(DaVessel* ship, qint8 thrust);

	void DoShipThrusterVerticalState(DaVessel* ship, qint8 thrust);

	void DoSitAtConsole(quint16 stationId, const AuStation& station, quint16 which);

	void DoTeleport(const AuPoint& loc);

	// AI functions called during Think. See ExecuteDecision for a discussion of how this works.
	void DoAI();
	virtual void DoAttackTarget();
	virtual void DoBattleStation();
	virtual void DoCloseDoor();
	virtual void DoComeHere();
	virtual void DoComeHereExactly();
	virtual void DoDamageReport();
	virtual void DoDropRider();
	virtual void DoEnterSurgery();
	virtual void DoFollowMe();
	virtual void DoFollowTarget();
	virtual void DoGarrison();				// Derived classes MUST override to call RequestNextOrder at least.
	virtual void DoGetIn();
	virtual void DoGetItem();
	virtual void DoGetOut();
	virtual void DoGoToRoom();
	virtual void DoGoToTarget();
	virtual void DoGoToTargetExactly();
	virtual void DoHengeCeremony();
	virtual void DoMountMe();
	virtual void DoMountTarget();
	virtual void DoOpenDoor();
	virtual void DoReleaseCommand();
	virtual void DoScan();
	virtual void DoSleep();
	virtual void DoStayHere();
	virtual void DoTakeCommand();

	inline int EnemyBlips() const
	{
		m_SentientLock.lockForRead();
		int v = m_EnemyBlips;
		m_SentientLock.unlock();
		return v;
	}

	// AI decision making and execution happens in two steps.
	// 1. The Think function decides about what to do. It does nothing except set state values on this atom.
	// 2. ExecuteDecision is called during Tick to carry out the decisions that have been made. It must work quickly with minimal
	//    processing.
	void ExecuteDecision(int msElapsed);

	// Check to see if I need to run to the sick bay for surgery.
	// Insures the ship I am aboard has a functioning surgery unit and medical kits in the hold.
	// Returns true if an order is amended to get medical attention.
	// This base class will get a checkup if I have lost any hit points at all, kind of wimpy.
	virtual bool GetACheckup();

	bool InConeOfAddress(const DaPerson& talker) const;

	inline bool InEnemyRange() const
	{
		m_SentientLock.lockForRead();
		bool v = m_InEnemyRange;
		m_SentientLock.unlock();
		return v;
	}

	// Returns true if my current job offer is valid and un-expired, and still exists in the global job offers list.
	bool IsJobOfferValid() const;

	// Returns the last time my Look function was called.
	inline AuElapsedTimer LastLookForThreatsTime() const
	{
		m_SentientLock.lockForRead();
		AuElapsedTimer v = m_tmLastLook;
		m_SentientLock.unlock();
		return v;
	}

	// Last ship that was plundered, so we don't plunder it repeatedly.
	inline AuId LastPlunder() const
	{
		m_SentientLock.lockForRead();
		AuId v(m_LastPlunder);
		m_SentientLock.unlock();
		return v;
	}

	inline QString LastShipName() const
	{
		m_SentientLock.lockForRead();
		QString v(m_LastShipName);
		m_SentientLock.unlock();
		return v;
	}

	inline void SetLastShipName(const QString& shipName)
	{
		m_SentientLock.lockForWrite();
		m_LastShipName = shipName;
		m_SentientLock.unlock();
	}

	virtual AuGear* LockGearForWrite()
	{
		return 0;
	}

	inline const AuAIOrderArray& LockOrdersForRead() const
	{
		m_ThinkLock.lockForRead();
		return m_AIOrders;
	}

	inline AuAIOrderArray& LockOrdersForWrite()
	{
		m_ThinkLock.lockForWrite();
		return m_AIOrders;
	}

	inline void UnlockOrders() const
	{
		m_ThinkLock.unlock();
	}

	void LookForAmmoCID(COMMODITYID ammoCID, DaItem*& itemAtom, AuPoint& itemDir, float& itemRange) const;

	void LookForAmmoType(AMMOTYPE ammoType, DaItem*& itemAtom, AuPoint& itemDir, float& itemRange) const;

	virtual AuPoint MovementBlocked(DaAtom* hit, const AuPoint& hitLoc, const AuPoint& hitNormal, bool hitShield);

	inline float MoveDistLimit() const
	{
		m_SentientLock.lockForRead();
		float v = m_MoveDistLimit;
		m_SentientLock.unlock();
		return v;
	}

	QString OrderTargetName() const;

	// So far, this is only room name when the order is go to a room.
	QString OrderTargetText() const;

	static QString QuotableQuote();

	// Looks for the ship in this atom's current solar system.
	void ReleaseBerth();

	// Ship may be 0, in which case only my assigned ship id is cleared.
	void ReleaseBerth(DaVessel* ship);

	inline void ResetOrderTime()
	{
		m_ThinkLock.lockForWrite();

		if(!m_AIOrders.isEmpty())
			m_AIOrders.last().m_Time.Set();

		m_ThinkLock.unlock();
	}

	inline void ResetStrategy(bool resetDestinationAlso)
	{
		m_SentientLock.lockForWrite();
		ResetStrategyNoLock(resetDestinationAlso);
		m_SentientLock.unlock();
	}

	void RxBeamAtom(const AuCommBeamAtom* msg);

	void RxBeamCoordinate(const AuCommBeamCoordinate* msg);

	void RxBeamPlanet(const AuCommBeamPlanet* msg);

	void RxBeamShip(const AuCommBeamShip* msg);

	void RxBeamTransporter(const AuCommBeamTransporter* msg);

	void RxBeamVehicleAtom(const AuCommBeamVehicleAtom* msg);

	void RxBeamVehicleCoordinate(const AuCommBeamVehicleCoordinate* msg);

	void RxBeamVehiclePlanet(const AuCommBeamVehiclePlanet* msg);

	void RxBeamVehicleShip(const AuCommBeamVehicleShip* msg);

	void RxBeamVehicleTransporter(const AuCommBeamVehicleTransporter* msg);

	void RxChat(const AuCommChat* msg);

	void RxCitizenOrder(const AuCommCitizenOrder* msgOrder, bool firstInGroup);

	void RxCitizenPosture(const AuCommCitizenPosture* msg, bool firstInGroup);

	virtual void RxHailReply(const AuCommPacket* msg);

	virtual void RxHailRequest(const AuCommPacket* msg);

	void SayJobOffer(DaPerson& person);

	void SayPleasantry(const DaPerson& person);

	// Does nothing if my current job offer has not expired yet.
	// Configures my current job offer for a 'person'.
	// Returns false if the person already has the job I am offering.
	// Returns true if I have a job offer.
	bool SelectJobOffer(DaPerson& person);

	void SellCargo(DaVessel* ship, COMMODITYID cargoCID, int cargoCount, DaAtomList& tradeCities);

	// May destroy the ship.
	virtual void ShipLog(DaVessel* ship)
	{
		Q_UNUSED(ship);
	}

	// Returns true if this dude has spoken recently, within the time 'withinSecs' seconds prior to now.
	inline bool SpokeRecently(int withinSecs) const
	{
		m_TalkLock.lockForRead();
		bool v = m_tmLastSpoke.isValid() && m_tmLastSpoke.elapsed()/d_MSecPerSec < withinSecs;
		m_TalkLock.unlock();
		return v;
	}

	virtual void StayIdle();

	virtual void TakeOrderAmendment(CITIZEN_ORDER order, const AuId& orderTarget, const AuPoint& orderLoc,
			const QString& orderText=QString(""));

	virtual void TakePosture(const AuCommPacket& msg, CITIZEN_POSTURE order, bool firstInGroup);

	QString ThinkOfSomethingToSay(const DaPerson& person) const;

	inline float TurnZ() const
	{
		m_ThinkLock.lockForRead();
		float tz = m_TurnZ;
		m_ThinkLock.unlock();
		return tz;
	}

	inline void SetTurnZ(float tz)
	{
		m_ThinkLock.lockForWrite();
		m_TurnZ = tz;
		m_ThinkLock.unlock();
		TickTock();
	}

	// Expects inventory lock to be locked for write.
	virtual void UpdateGearWeight()
	{
	}

	void UpdateThinkRate();

	// NOTE: Expects gear lock to be locked already.
	// NOTE: The item count could be reduced to 0 by this function. The caller must clean up if that happens.
	//       See DaSentient::UseItem for an example.
	// Detonator and fuse weapons always return success. Caller is expected to drop or throw them.
	// Food is eaten by consuming one count from the item. This returns a successful result.
	// Weapons that use ammo decrement ammo count when return result indicates success, though not explicitly the ammo usage.
	// Non-food items are tested for wear and one unit is consumed on failure. This returns a successful result.
	// Returns:
	//  0 = 'item' used successfully.
	//  1 = 'item' used successfully and one unit of it was consumed in the process. It could be used up; caller should clean up.
	//  2 = 'item' used successfully. Item hit points degraded due to wear.
	//  -1 = 'item' not used; it is invalid.
	//  -2 = 'item' not used; it is a weapon that uses ammo but it is out of ammo.
	virtual int UseItem(AuItem& item, INVENTORYSUB sub);

private:	// AI
	// Returns a number from 0 to 7, where 0 is smart and 7 is slow.
	inline int Acuity() const
	{
		return Id().GetID()&0x7;
	}

	// Turn toward desired direction, as determined by m_TurnZ.
	// Returns 'true' when I finished turning to the desired direction.
	// Returns 'false' when not turning or not done turning.
	bool AnimateTurn(unsigned int msElapsed);

	void DoOrderReport(DaVessel* ship);

	void DoOrderTransferCargoCategoryFrom(DaVessel* ship);

	void DoOrderTransferCargoCategoryTo(DaVessel* ship);

	void DoOrderTransferCargoFrom(DaVessel* ship);

	void DoOrderTransferCargoTo(DaVessel* ship);

	void DoOrderTransferMoneyFrom(DaVessel* ship);

	void DoOrderTransferMoneyTo(DaVessel* ship);

	void DoOrderTransferPeopleFrom(DaVessel* ship);

	void DoOrderTransferPeopleTo(DaVessel* ship);

	void EnergizeTransporter(const AuCommPacket* msg, bool isFrom, int padLimit, DaAtom* targAtom, const AuPoint& targLoc,
			const AuPoint& targZ, quint16 targWhich, AuPointArray& targPads);

	bool EnergizeVehicleTransporter(const AuCommPacket* msg, bool isFrom, DaAtom* targAtom, const AuPoint& targLoc,
			const AuPoint& targZ, quint16 targWhich, QList<AuMatrix4x4>& targParkingSpots);

	AuPoint FindAirportLandingSite(const DaVessel* ship, const DaWorld* planet, const DaBd* bd) const;

	AuPoint FindLandingSite(const DaVessel* ship, const DaWorld* planet, const DaBd* bd) const;

	virtual void FindShipMatesRecursive(const AuId& shipAssigned, DaAtomList& mates) const;

	bool HasJobToOffer(DaPerson& person) const;

	inline bool IsOfferingJob(const AuJobOffer& jobOffer) const
	{
		m_TalkLock.lockForRead();
		bool v = m_JobOffer == jobOffer;
		m_TalkLock.unlock();
		return v;
	}

	// A false return causes AI crew to stay in the solar system.
	// NOTE: This will return true for pirates even though they will be destroyed on exit from the system. This is purposeful so
	//       the crew on the pirate ship will actually navigate out of the system in order to be destroyed by dark matter.
	bool IsOkToLeaveSolarSystem(const DaVessel* ship) const;

	// Returns true if I am ready to shoot my weapon or attack with my body.
	bool IsReadyToShootAimedWeapon() const;

	// Induces some overhead time for starting a nu mission order based on the rank of this officer.
	// Low ranking officers take longer to reply true.
	bool IsReadyToStartMissionOrder(const DaVessel* ship) const;

	// Is it time for me to think?
	inline bool IsTimeToThink()
	{
		// When the time between thoughts has passed, I'll think about what to do next.
		m_ThinkLock.lockForRead();
		bool v = !m_AIThinkTime.isValid() || m_AIThinkTime.elapsed() > m_AIThinkRate;
		m_ThinkLock.unlock();
		if(v)
		{
			// It's time for this citizen to think.
			RestartThinkTime();
			return true;
		}

		// It's not time yet for this citizen to think.
		return false;
	}

	void Look(DaAtom*& itemAtom, AuPoint& itemDir, float& itemRange,		// Item of interest to this citizen: item, vehicle.
			DaAtom*& targetAtom, AuPoint& targetDir, float& targetRange,	// Target of this citizen's AI order.
			DaAtom*& threatAtom, AuPoint& threatDir, float& threatRange,	// Threat atom: ship, vehicle, animal, citizen, person.
			DaAtom*& comradeAtom, AuPoint& comradeDir, float& comradeRange,	// Comrade.
			DaAtom*& fallenAtom, AuPoint& fallenDir, float& fallenRange);	// Fallen comrade.

	// Non-recursive look at the subatoms of 'parent'.
	// 'commonParent' is a parent atom of mine at some level that is also a parent of 'parent', 'commonParent' may be 'parent'.
	// 'myT' is my transformation matrix relative to 'commonParent'.
	// 'eyeVec' is my look vector, relative to my body.
	// 'eyeAngle' is my field of view angle, as returned by DhDNA::FieldOfViewAngle.
	// 'eyeRange' is the distance I can see, in feet.
	// 'noseRange' is the distance I can smell, in feet.
	// 'earRange' is the distnce I can hear, in feet.
	// Remaining parameters are output parameters.
	void LookAtSubatoms(const DaAtom* parent, const DaAtom* commonParent, const AuMatrix4x4& myT,
			const AuPoint& eyeLoc, const AuPoint& eyeVec, float eyeAngle, float eyeRange, float noseRange, float earRange,
			float weaponRange,												// Range of my weapon.
			DaAtom*& itemAtom, AuPoint& itemDir, float& itemRange,			// Item of interest to this citizen: item, vehicle.
			DaAtom*& targetAtom, AuPoint& targetDir, float& targetRange,	// Target of this citizen's AI order.
			DaAtom*& threatAtom, AuPoint& threatDir, float& threatRange,	// Threat atom: ship, vehicle, animal, citizen, person.
			DaAtom*& comradeAtom, AuPoint& comradeDir, float& comradeRange,	// Comrade.
			DaAtom*& fallenAtom, AuPoint& fallenDir, float& fallenRange) const;	// Fallen comrade.

	inline AuSentientDecision* LockMakeDecision(AuSentientDecision::DECISION decide)
	{
		m_DecisionTableLock.lock();

		// First decision causes me to allocate my decision table.
		if(!m_DecisionTable)
		{
			m_DecisionTable = new AuSentientDecision*[AuSentientDecision::D_COUNT];
			for(int dt=0; dt<AuSentientDecision::D_COUNT; dt++)
				m_DecisionTable[dt] = 0;
		}

		// First decision of each type causes me to allocate an entry in the decision table. We will reference it by index.
		if(!m_DecisionTable[decide])
			m_DecisionTable[decide] = AuSentientDecision::Create(decide);

		// Queue the decision.
		m_ThinkLock.lockForWrite();
		if((int)m_DecisionCount < m_Decisions.size())
			m_Decisions[m_DecisionCount] = decide;
		else
			m_Decisions.append(decide);
		m_DecisionCount++;
		m_ThinkLock.unlock();

		// Return the corresponding decision object so the caller can configure its parameters.
		return m_DecisionTable[decide];
	}

	inline void UnlockDecision()
	{
		m_DecisionTableLock.unlock();
	}

	// Attempts to mount a creature if all conditions permit.
	// Creature must be an animal or a person. Citizens do not mount other citizens.
	// Assumes distance has already been verified.
	// Returns true if the creature is successfully mounted.
	bool MountCreatureIfPossible(DaCreature* creature);

	void MoveInDirection(const AuPoint& moveVec, float headTilt, const AuPose& pose);

	// Perform movement actions inside a capsule.
	void MoveInsideCapsule(DaCapsule* capsule, bool isCrew);

	// 'capsule' is my parent capsule.
	// 'toLoc' is relative to my parent capsule; it may be outside.
	// Returns 0 when I am still moving to the target.
	// Returns 1 when I have reached the target location.
	// Returns 2 when no path to the target room exists.
	int MoveToCapsuleLoc(DaCapsule* capsule, const AuPoint& toLoc);

	// 'destLoc' is relative to my current globe world. IsNull if the destination is not in my current globe world.
	// 'moveVec' is the movement vector relative to me to get to the destination loc.
	// 'distToMove' is the distance (usually 2D) to move to the target loc from my current loc.
	// 'headTilt' is my head tilt to the target loc.
	void MoveToGlobeLoc(DaWorldLL* glworld, const AuPoint& destLoc, const AuPoint& moveVec, double distToMove, float headTilt);

	// Perform movement actions to get to the order location.
	// 'closeEnough' is the distance at which I am considered to have arrived at the target, in addition to bumping into the target.
	// 'lookAtTarget' is true when I am to look at the target even if I am already close enough to the target.
	// 'orderTarget' is an output parameter which is the atom pointer of the order target when return result is 1.
	// Returns 0 when I am still moving to the target.
	// Returns 1 when I have reached the target. 'orderTarget' will be set as an output parameter in this case.
	// Returns 2 when no path to the target exists.
	int MoveToOrderLoc(double closeEnough, bool lookAtTarget, DaAtom*& orderTarget);

	// Returns the type of the next order after the current order.
	// Returns the current order if there is only one order, which will be the default fallback stay order.
	inline CITIZEN_ORDER NextOrder() const
	{
		CITIZEN_ORDER order;
		m_ThinkLock.lockForRead();

		if(m_AIOrders.count() > 1)
			order = m_AIOrders[m_AIOrders.count()-2].m_Type;
		else if(m_AIOrders.isEmpty())
			order = ORDER_Stay;
		else
			order = m_AIOrders.last().m_Type;

		m_ThinkLock.unlock();
		return order;
	}

	inline int OrderCount() const
	{
		int count;
		m_ThinkLock.lockForRead();

		count = m_AIOrders.count();

		m_ThinkLock.unlock();
		return count;
	}

	inline AuTime OrderTime() const
	{
		AuTime t;
		m_ThinkLock.lockForRead();

		t = m_AIOrders.last().m_Time;

		m_ThinkLock.unlock();
		return t;
	}

	void PilotAttackUnit(DaVessel* ship, const DaAtom* unit);
	void PilotFollowUnit(DaVessel* ship, const DaAtom* unit);
	void PilotHoldPosition(DaVessel* ship);
	void PilotHoldPositionClearWormhole(DaVessel* ship);
	bool PilotObstacleAvoid(AuPoint& courseLoc, const DaVessel* ship) const;
	void PilotPlunderUnit(DaVessel* ship, const DaUnit* unit);
	void PilotToBd(DaVessel* ship, const DaBd* bd, const DaWorld* planet);
	void PilotToHoverSite(DaVessel* ship, const DaWorld* planet, const AuPoint& landLoc);
	void PilotToLandingSite(DaVessel* ship, const DaWorld* planet, const AuPoint& landLoc);
	void PilotToPlanet(DaVessel* ship, const DaWorld* planet, const AuPoint& destOffset);
	void PilotToPlanetLocation(DaVessel* ship, const AuMatrix4x4& shipT, const AuPoint& shipLoc, const DaWorld* planet,
			const AuPoint& destLoc, double speedLimit, double closeness, double terrainClearance, bool isLanding);
	void PilotToStar(DaVessel* ship, const DaStar* star, const AuPoint& destOffset);
	void PilotToStarLocation(DaVessel* ship, const AuMatrix4x4& shipT, const AuPoint& shipLoc, const DaStar* star,
			const AuPoint& destLoc, double speedLimit, double closeness);
	void PilotToStation(DaVessel* ship, const DaVessel* station, const DaWorld* planet);
	void PilotToSystemLoc(DaVessel* ship, const AuMatrix4x4& shipT, const DaAtom* system, const AuPoint& sysLoc,
			AuWormhole::POLARITY wormholePolarity, bool doFAF, const QString& destName, double destRadius);
	void PilotToUnit(DaVessel* ship, const DaAtom* unit, double clearance, bool okToLand);
	void PilotToWaypoint(DaVessel* ship);
	void PilotVerticalLanding(DaVessel* ship);
	void PilotVerticalTakeOff(DaVessel* ship);

	void ReportEnemySensorContact(DaVessel* ship, const AuSensorBlipList& sensorBlips, float sensorRange);

	void ReportEnemyWeaponContact(DaVessel* ship);

	// NOTE: Expects m_SentientLock to be locked.
	inline void ResetStrategyNoLock(bool resetDestinationAlso)
	{
		m_AIRecentPath.clear();
		if(resetDestinationAlso)
			ClearRouteSentientLocked();
		m_AIStrategy = 0;
		m_AIStrategyTime.restart();
	}

	quint16 ThinkRateForPose(CITIZEN_ORDER forOrder) const;

	// Insure that I will think in the next tick cycle.
	inline void ThinkSoon()
	{
		m_ThinkLock.lockForWrite();
		m_AIThinkTime = AuElapsedTimer();
		m_ThinkLock.unlock();
	}

	inline float ThoughtSpeed() const
	{
		float msThought;										// Milliseconds before I expect to think again.
		if(!TockTime().IsNull())
		{
			// Calculate the actual milliseconds since my last tock cycle, which is when I think.
			msThought = TockTime().ElapsedMSec();
			quint16 tr = AIThinkRate();
			if(msThought < tr)
				msThought = tr;									// Don't expect to think sooner than my think rate.
		}
		else
			msThought = AIThinkRate();
		return msThought;
	}

	void TreatPatientConditions(DaCreature* patient, DaCapsule* capsule);

protected:
	void ClearRoute()
	{
		m_SentientLock.lockForWrite();
		ClearRouteSentientLocked();
		m_SentientLock.unlock();
	}

	void ClearRouteSentientLocked();

	FLIGHT_ATTITUDE FlightAttitude(const DaVessel* ship) const;

	// This can be called often but it only actually does something two minutes after the last skills change time.
	// Tests and updates m_SkillsChg; does nothing else.
	// Returns true if it is time to gain experience, i.e. it has been at least two minutes since the last skill change time.
	virtual bool GainExperienceForWorking();

	bool InterpretVisage(const AuEmojiList& emojis, const QString& saidWord, AuVisage& visage) const;

	// Recent memories
	inline bool IsOnApproach() const
	{
		return m_RecentMemory & MEM_OnApproach;
	}

	inline bool IsOrderAnnounced() const
	{
		return m_RecentMemory & MEM_OrderAnnounced;
	}

	void RepairPirate(DaVessel* ship);

	// Repairs the ship if possible. 
	// Returns result code:
	//   0=repairs completed
	//   1=no bds or cities at which to get repairs
	//   2=too expensive (company only)
	//   3=insufficient parts
	int RepairShip(DaVessel* ship);

	inline void ResetOnApproach()
	{
		m_RecentMemory &= ~MEM_OnApproach;
	}

	inline void ResetOrderAnnounced()
	{
		m_RecentMemory &= ~MEM_OrderAnnounced;
	}

	inline void SetOnApproach()
	{
		m_RecentMemory |= MEM_OnApproach;
	}

	inline void SetOrderAnnounced()
	{
		m_RecentMemory |= MEM_OrderAnnounced;
	}

	inline float TurretTurnRate()
	{
		return PIf*2;					// Radians per second turn rate for turrets.
	}
};

#endif // DaSentient_H
