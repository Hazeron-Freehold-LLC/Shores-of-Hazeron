//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: davessel.h,v 1.256 2022/05/17 17:10:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Base class for all units that are large enough to walk around inside of, like spacecraft, ships, airliners. Not sit-in units
// like vehicles.

#ifndef DaVessel_H
#define DaVessel_H

#include "aucryoberths.h"
#include "audamage.h"
#include "auordnance.h"
#include "auroster.h"
#include "auschelpers.h"
#include "aushipment.h"
#include "aushipsystem.h"
#include "auskill.h"
#include "auspecies.h"
#include "dacapsule.h"
#include "dewaypoint.h"

class AuCommBeamOver;
class AuCommBeamVehicleAtom;
class AuCommBeamVehicleCoordinate;
class AuCommBeamVehiclePlanet;
class AuCommBeamVehicleShip;
class AuCommBeamVehicleTransporter;
class AuCommCombine;
class AuCommContactShip;
class AuCommHailRequest;
class AuCommJettison;
class AuCommMissionNew;
class AuCommMissionOrderAdd;
class AuCommMissionOrderInsert;
class AuCommMissionOrderNew;
class AuCommMissionProperty;
class AuCommOpenTEU;
class AuCommPacket;
class AuCommRequestBerthAny;
class AuCommRequestBerthCaptain;
class AuCommRequestBerthCaptainSlot;
class AuCommRequestBerthCrew;
class AuCommRequestBerthOfficer;
class AuCommRequestBerthPassenger;
class AuCommRequestBerthTroop;
class AuCommRequestInventory;
class AuDialog;
class AuGame;
class AuItem;
class AuPose;
class AuRefitMaterials;
class AuRefitShip;
class AuScReport;
class AuTractorBeam;
class DaAnimal;
class DaNpc;
class DaCreature;
class DaSentient;
class DaVehicle;

#define MAX_DISPLAYED_SYSTEMS 8		// Number of each displayed, suppliers and consumers.

class AUATOM_DLL DaVessel : public DaCapsule
{
public:
	enum GRAVITY_DRIVE_STATE
	{
		GD_Off = 0,
		GD_On = 1,
		GD_Hold = 2,
		GD_Invalid = 3
	};

	enum MONSTER_ORIGIN
	{
		MONSTER_Unknown,
		MONSTER_Officer,
		MONSTER_Crew,
		MONSTER_Troop,
		MONSTER_Passenger,
		MONSTER_Cargo,
	};

	enum SHIELD_STATE
	{
		SS_Off = 0,
		SS_Up = 1,
		SS_Down = 2,	// Check Tx/Rx if this exceeds 2 bits
		SS_Invalid = 3
	};

	enum SHIELD
	{
		SHIELD_Front,
		SHIELD_Back,
		SHIELD_Left,
		SHIELD_Right,
		SHIELD_Top,
		SHIELD_Bottom,
	};

protected:
	// Unguarded
	bool								m_AcceptCrew;			// true to accept all join requests
	bool								m_AcceptPassengers;		// true to accept all join requests
	bool								m_AcceptTroops;			// true to accept all join requests
	ACCESS_POLICY						m_AccessPolicy;			// ACCESS_POLICY
	bool								m_AllowPassengerSeaChests;	// true to allow passengers access to their sea chest
	bool								m_AutoFetchVehicles;	// True to automatically fetch vehicles when spots are empty.
	bool								m_bExtLightOn;			// true if the exterior light is on.
	bool								m_bPowerPlantOn;		// true if power plant is on
	bool								m_bSensorsOn;			// true if sensors are on
	unsigned char						m_byAccelSkewDur; 		// This value drifts so the engineer has something to do.
	unsigned char						m_byPowerPlantSkewDur;	// This value drifts so the engineer has something to do.
	unsigned char						m_byTurnRateSkewDur; 	// This value drifts so the engineer has something to do.
	char								m_chAccelSkew; 			// This value drifts so the engineer has something to do.
	char								m_chPowerPlantSkew;		// This value drifts so the engineer has something to do.
	char								m_chTurnRateSkewX;		// This value drifts so the engineer has something to do.
	char								m_chTurnRateSkewY;		// This value drifts so the engineer has something to do.
	mutable quint8						m_ConCaptAI;			// 0=unknown, 1=no, 2=yes if AI has the con, could be demiavatar
	mutable quint8						m_ConEngineerAI;		// 0=unknown, 1=no, 2=yes, could be demiavatar
	mutable quint8						m_ConEngineerPerson;	// 0=unknown, 1=no, 2=yes
	mutable quint8						m_ConNavigatorAI;		// 0=unknown, 1=no, 2=yes, could be demiavatar
	mutable quint8						m_ConNavigatorPerson;	// 0=unknown, 1=no, 2=yes
	mutable quint8						m_ConPilotAI;			// 0=unknown, 1=no, 2=yes, could be demiavatar
	mutable quint8						m_ConPilotPerson;		// 0=unknown, 1=no, 2=yes
	mutable quint8						m_ConPowerRelayAI;		// 0=unknown, 1=no, 2=yes, could be demiavatar
	mutable quint8						m_ConPowerRelayPerson;	// 0=unknown, 1=no, 2=yes
	mutable quint8						m_ConSensorAI;			// 0=unknown, 1=no, 2=yes, could be demiavatar
	mutable quint8						m_ConSensorPerson;		// 0=unknown, 1=no, 2=yes
	mutable quint8						m_ConShieldAI;			// 0=unknown, 1=no, 2=yes, could be demiavatar
	mutable quint8						m_ConShieldPerson;		// 0=unknown, 1=no, 2=yes
	mutable quint8						m_ConWeaponAI;			// 0=unknown, 1=no, 2=yes, could be demiavatar
	mutable quint8						m_ConWeaponPerson;		// 0=unknown, 1=no, 2=yes
	quint8								m_FuelQuality;			// Average quality of hydrogen in fuel cells.
	bool								m_InTow;				// True while someone has a tractor beam attached to me.
	bool								m_IsHumanLock;			// True if locked on blip selected by a human.
	bool								m_IsPrivate;			// true if privately owned
	quint8								m_LockedIsCivilian;		// My politial stance toward the locked blip. ONLY VALID ON CLIENT.
	quint8								m_LockedStance;			// My politial stance toward the locked blip. ONLY VALID ON CLIENT.
	bool								m_Moored;				// true when moored
	PILOT_UP_POLICY						m_PilotUpPolicy;		// PILOT_UP_POLICY
	quint8								m_PowerPlantShunts;		// Power Plant Shunts in use, 1 bit per shunt.
	SHIELDFUEL_POLICY					m_ShieldFuelPolicy;		// SHIELDFUEL_POLICY
	SHIELD_POLICY						m_ShieldPolicy;			// SHIELD_POLICY
	SHIELD_STATE						m_ShieldState;			// SHIELD_STATE value
	quint8								m_QMade;				// Quality of original construction. May exceed Q of design.

protected:
	mutable QReadWriteLock				m_VesselLock;
	float								m_CapacitorLevel;		// Quantity of energy units in the capacitor.
	int									m_CapacitorTrend;		// 1, 0, -1 =increasing, neutral, decreasing
	mutable AuId						m_ConAICaptId;			// First officer assigned to this ship, preferably with the con.
	GRAVITY_DRIVE_STATE					m_eGravityDriveState;
	mutable int							m_ETE;					// ETE calculated during AnimateDeviceObjects, client only.
	double								m_fFuelQuantity;		// Total current fuel on board in fuel cells
	AuId								m_LockedBlip;			// Target that the ship sensors are "locked-on" to.
	AuPoint								m_LockedSysLoc;			// Sent to client with float accuracy. ONLY VALID ON CLIENT.
	quint16								m_Monster;				// Monster encounter counter. Probability goes up as this increases.
	MONSTER_ORIGIN						m_MonsterOrigin;
	AuSpecies							m_RatSpecies;			// Species of rats that appear aboard.
	quint16								m_RepairSecs;			// Seconds remaining for repairs to be completed.
	QString								m_WhereMade;			// City and world of manufacture.

protected:
	// Ships treasury
	mutable QReadWriteLock				m_AccountLock;
	double								m_Bank;					// Current currency aboard ship
	double								m_BankSubsidyTax;		// Subsidy taxes accumulated aboard this ship for its empire.
	AuElapsedTimer						m_tmLastBank;			// for client updates, not stored in db

protected:
	mutable QReadWriteLock				m_BayLock;
	unordered_map<quint16, AuBayFired>	m_BayFired;				// Key is 'which' bay.
	AuElapsedTimer						m_tmBayFired;			// for client updates, not stored in db
	unordered_map<quint16, AuBaySetting>	m_BaySetting;		// Settings and ammo for each bay, weapons and harvesters.

protected:
	mutable QReadWriteLock				m_CargoLock;
	AuItemHash							m_Cargo;				// Items in cargo hold.
	AuShipmentList						m_Shipments;			// Shipments to cities. Station only.
	AuElapsedTimer						m_tmCargoChange;

protected:
	mutable QReadWriteLock				m_CommLock;
	bool								m_ATCAtRest;			// At rest state when last tracked by ATC.
	AuId								m_ATCCity;				// City tracking me on sensors.
	AuId								m_ATCCityEnemy;			// Enemy city tracking me on sensors.
	AuElapsedTimer						m_ATCContact;			// When ATC contact last updated.
	AuId								m_ATCStation;			// Station tracking me on sensors.
	AuId								m_ATCStationEnemy;		// Enemy station tracking me on sensors.
	AuElapsedTimer						m_tmAlert;				// Time of last Alert or attack enemy broadcast.
	AuElapsedTimer						m_tmEnvDamage;			// Time of last EnvDamage msg, so we don't send them too often.
	AuElapsedTimer						m_tmSOS;				// Time of last SOS broadcast, so we don't send them too often.

protected:
	mutable QReadWriteLock				m_ControlLock;
	unsigned char						m_byRotSpeedX;			// Rotation speed about X in degrees per second (roll left/right).
	unsigned char						m_byRotSpeedY;			// Rotation speed about Y in degrees per second (pitch up/down).
	unsigned char						m_byRotSpeedZ;			// Rotation speed about Z in degrees per second (turn left/right).
	float								m_Pitch;				// Pitch to be added. Set by AI when maneuvering. Not stored.
	float								m_Roll;					// Roll to be added. Set by AI when maneuvering. Not stored.
	qint8								m_Throttle;				// Throttle setting
	qint8								m_ThrustLateral;		// momentary lateral thrust throttle, - left, + right
	qint8								m_ThrustLong;			// momentary longitudinal thrust throttle, - backward, + forward
	qint8								m_ThrustVertical;		// momentary vertical thrust throttle, - down, + up
	AuElapsedTimer						m_tmLastBrake;			// Time last control received.
	AuElapsedTimer						m_tmLastBrakeLV;		// Time last control received.
	AuElapsedTimer						m_tmLastPitchControl;	// Time last control received.
	AuElapsedTimer						m_tmLastRollControl;	// Time last control received.
	AuElapsedTimer						m_tmLastRPYControl;		// Time last RollPitchYaw called.
	AuElapsedTimer						m_tmLastThrottleControl;	// Time last control received.
	AuElapsedTimer						m_tmLastThrottleFControl;	// Time last control received. Forward/back.
	AuElapsedTimer						m_tmLastThrottleLControl;	// Time last control received. Lateral.
	AuElapsedTimer						m_tmLastThrottleVControl;	// Time last control received. Vertical.
	AuElapsedTimer						m_tmLastTurnControl;	// Time last control received.
	float								m_Yaw;					// Yaw to be added. Set by AI when maneuvering. Not stored.
	int									m_wActions;				// ACT_ Bits representing current actions to process during Tick.

protected:
	mutable QReadWriteLock				m_DamageLock;
	AuDamage							m_Damage;				// Damage to this spacecraft.
	AuTime								m_tmDamageChange;		// Also the time the ship was put in stasis, to delay restoration.

protected:
	// Games at gamet tables are allocated when created.
	mutable QReadWriteLock				m_GamesLock;
	unordered_map<quint16, AuGame*>		m_Games;				// Games in progress at game tables. Key is table 'which' index.
	AuElapsedTimer						m_GamesChanged;			// For updates to clients. Not serialized.

protected:
	mutable QReadWriteLock				m_LastInRangeLock;		// Last range time is mutable so I can reset it in SensorScan.
	mutable AuElapsedTimer				m_LastInEnemyRange;		// When in range of enemy bay weapon. Not stored or sent to client.

protected:
	mutable QReadWriteLock				m_MissionLock;
	AuMission							m_Mission;				// The mission of this spacecraft.
	AuElapsedTimer						m_tmMissionChange;		// Time m_Mission changed.

protected:
	mutable QReadWriteLock				m_NameWithEmpireLock;
	mutable QString						m_NameWithEmpire;		// ""=unknown

protected:
	mutable QReadWriteLock				m_RosterLock;
	AuCryoBerths						m_CryoBerths;
	AuId								m_OwnerId;				// For privately owned ship.
	QString								m_OwnerName;			// For privately owned ship.
	AuRoster							m_Roster;				// Crew and passengers.
	AuElapsedTimer						m_tmRosterChange;

protected:
	mutable QReadWriteLock				m_ShieldsLock;
	double								m_ShieldCap[6];			// Shield capacitors, in SHIELD order. Max ShieldUnitsWorking() ea.
	quint8								m_ShieldScram[6];		// Shield scram time, in SHIELD order, in seconds.

protected:
	mutable QReadWriteLock				m_SystemsLock;
	AuShipSystemArray					m_ssaShipSystems;		// List of all Ship Systems. ONLY A SUBSET IS SENT TO THE CLIENT.
	AuElapsedTimer						m_tmSystemsUpg;			// Time a system quality changed in the m_ssaShipSystems data.

protected:
	mutable QReadWriteLock				m_TurretLock;
	unordered_map<quint16, AuOrdnance>	m_Turrets;				// Key is 'which' turret. Tracks ammo and firing state.

protected:
	mutable QReadWriteLock				m_WaypointLock;
	DeWaypointRoute						m_aWaypoints;			// Route of waypoints.
	AuElapsedTimer						m_tmWaypointChange;

	// OpenGL
	float								m_fShieldDuration[MAX_SHIELD_HIT_EFFECTS];
	float								m_fShieldEffect[MAX_SHIELD_HIT_EFFECTS*4];
	float								m_fShieldTexOffset;		// Texture offset used for shield rendering.
	AuElapsedTimer						m_shieldTime;			// Timer used for shield effect.
	AuTractorBeam*						m_TractorBeam;


public:	// Constructors
	DaVessel(const QString& name, int idEmpire);
	DaVessel();
	virtual ~DaVessel();

protected:
	DaVessel(const DaVessel& rhs);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:	// Methods
	// Acceleration/deceleration in ft/sec/sec.
	// Adjusted for damage.
	virtual AuPoint Acceleration() const;

	// Skew factors to give the engineer something to do.
	inline char AccelSkew() const
	{
		return m_chAccelSkew;
	}

	inline void AccelSkewChange(char chVal, unsigned char byDur)
	{
		const int skewMax = 40;								// Person can adjust throughout the range, regardless of fuel quality.
		m_chAccelSkew += chVal;
		if(m_chAccelSkew > skewMax)
			m_chAccelSkew = skewMax;
		else if(m_chAccelSkew < -skewMax)
			m_chAccelSkew = -skewMax;
		m_byAccelSkewDur = byDur;
	}

	void AccelSkewWander();

	inline void AcceptCrew(bool accept)
	{
		m_AcceptCrew = accept;
		SetModified();
	}

	inline void AcceptPassengers(bool accept)
	{
		m_AcceptPassengers = accept;
		SetModified();
	}

	inline void AcceptTroops(bool accept)
	{
		m_AcceptTroops = accept;
		SetModified();
	}

	ACCESS_POLICY AccessPolicy() const
	{
		return m_AccessPolicy;
	}

	inline double AccountBalance() const
	{
		m_AccountLock.lockForRead();
		double v = m_Bank;
		m_AccountLock.unlock();
		return v;
	}

	inline double AccountBalanceSubsidyTax() const
	{
		m_AccountLock.lockForRead();
		double v = m_BankSubsidyTax;
		m_AccountLock.unlock();
		return v;
	}

	// Be sure to call CanAddCargo first. This function doesn't check.
	virtual void AddCargo(const AuItem& iNew);

	// Add a visual and/or audio effect to this atom.
	// 'effect' may be modified to something appropriate. e.g. VISUAL_HitPierce (looks like blood) is changed to VISUAL_HitPierceBug
	// when this atom is an exoskeletal creature, and something else when this atom is not even a creature.
	virtual void AddEffect(AuEffect& effect);

	// Adds a number of seconds to the repair time remaining.
	inline void AddRepairTime(unsigned int secsToAdd)
	{
		// A ship cannot have repair time while it is in 'nu' state or it will be stuck forever.
		if(IsNew())
			SetNotNew();										// Ship is no longer "nu".

		m_VesselLock.lockForWrite();

		// Calculate nu repair time without overrunning 16 bits.
		int nuSecs = m_RepairSecs + secsToAdd;
		if(nuSecs > 0xffff)
			m_RepairSecs = 0xffff;								// Max is 18hrs 12min 15sec
		else
			m_RepairSecs = (quint16)nuSecs;

		m_VesselLock.unlock();
	}

	// NOTE: This AddWaypoint expects the m_WaypointLock to be locked on entry.
	void AddWaypoint(const DeWaypoint* wp);

	void AddWaypoint(const AuId& idDestSolarSystem, const AuId& idDestAtom);

	void AddWaypoint(const AuId& idDestSolarSystem, const AuId& idDestAtom, const AuPoint& vFromDestAtom);

	// Adds waypoints to this ship's route to go to a destination solar system that is/contains 'idDest'.
	// 'vFromDestAtom' is used for locating wormholes in my current solar system.
	inline void AddWaypointRouteTo(const AuId &idDest, const AuPoint& vFromDestAtom)
	{
		bool negativeOk = CanEnterNegativeWormhole();
		bool neutralOk = CanEnterNeutralWormhole();
		m_WaypointLock.lockForWrite();
		if(DaCapsule::AddWaypointRouteTo(m_aWaypoints, AuId(0), idDest, vFromDestAtom, negativeOk, neutralOk))
		{
			m_tmWaypointChange.Set();
			SetModified();
		}
		m_WaypointLock.unlock();
	}

	AuId AICitizenAtPost() const;

	// Returns AI crew manning the first engineer station. Return NULL id if no AI is manning an engineer station.
	DaSentient* AIEngineOperator() const;

	// Returns id of AI crew manning the first fire control station. Return NULL id if no AI is manning a fire control station.
	AuId AIFireControl() const;

	// Returns id of the first AI officer assigned to this ship, preferably one with the con. Could be a demiavatar.
	// Note: The first officer may not be in command of the ship at present. Test IsCommandedByAI for that.
	inline AuId AIFirstOfficer() const
	{
		IsCommandedByAI();										// Updates m_ConAICaptId if necessary.

		m_VesselLock.lockForRead();
		AuId v(m_ConAICaptId);
		m_VesselLock.unlock();
		return v;
	}

	// Returns id of AI sentient manning a game table. Return NULL id if none.
	AuId AIGamer() const;

	AuId AILivestockAtPost() const;

	// Returns id of AI crew manning the first medic station. Return NULL id if no AI is manning a medic station.
	AuId AIMedic() const;

	// Returns id of AI crew manning the first navigator station. Return NULL id if no AI is manning a navigator station.
	AuId AINavigator() const;

	// Returns id of sentient occupying the first surgery station as a patient. Return NULL id if none.
	AuId AIPatient() const;

	// Returns id of AI crew manning the first helm station. Return NULL id if no AI is manning a helm station.
	AuId AIPilot() const;

	// Returns AI crew manning the first power relay station. Return NULL id if no AI is manning a power relay station.
	DaSentient* AIPowerRelayOperator();

	// Returns id of AI crew manning the first power relay station. Return NULL id if no AI is manning a power relay station.
	AuId AIPowerRelay() const;

	// Returns id of AI crew manning the first sensor station. Return NULL id if no AI is manning a sensor station.
	AuId AISensor() const;

	// Returns AI crew manning the first sensor station. Return NULL id if no AI is manning a sensor station.
	DaSentient* AISensorOperator();

	// Returns id of AI crew manning the first shield station. Return NULL id if no AI is manning a shield station.
	AuId AIShield() const;

	// Returns id of AI crew manning the first transporter station. Return NULL id if none.
	AuId AITransporter() const;

	AuId AITroopAtPost() const;

	// Returns id of AI troop manning the first turret station. Return NULL id if none.
	AuId AITurretGunner() const;

	// Returns the alert condition of the ship, based on the worse condition of the structural and non-structural systems.
	// This is based on current conditions, not the alert condition setting on DaSc ships.
	ALERT_CONDITION AlertCondition() const;

	// Return alert condition of structural systems.
	ALERT_CONDITION AlertConditionOfStructure() const;

	// Return alert condition of non-structural systems.
	ALERT_CONDITION AlertConditionOfSystems() const;

	inline void AllowPassengerSeaChests(bool allow)
	{
		m_AllowPassengerSeaChests = allow;
		SetModified();
	}

	virtual void AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim);

	void AnswerContactShip(AuCommStdChannel::RANGE range, bool contactedByCommand, const DaSentient* officer);

	void AnswerHail(const QString& channelName, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope,
			bool contactedByCommand, const DaSentient* officer);

	// Apparent alert condition is controlled by the commander on DaSc ships.
	virtual ALERT_CONDITION ApparentAlertCondition() const = 0;

	void ApplyRotation(unsigned int msElapsed, CONTROL_CONTEXT cc);

	// Returns armor level in the range of 0 to 10.
	virtual float ArmorLevel() const = 0;

	// Returns number of parts of type 'cid' to fix the 'sightedPart' system.
	// Returns number of hit points of non-part damage when 'cid' is CID_Invalid.
	quint32 AssessDamage(unsigned int sightedPart, COMMODITYID cid) const;

	inline bool AutoFetchVehicles() const
	{
		return m_AutoFetchVehicles;
	}

	virtual float BayWeaponPower(float unitsFired, quint16 which) const = 0;

	inline quint8 BayWeaponPowerSetting(quint16 which) const
	{
		quint8 s;

		m_BayLock.lockForRead();
		unordered_map<quint16, AuBaySetting>::const_iterator bsi = m_BaySetting.find(which);
		if(bsi != m_BaySetting.end())
			s = (*bsi).second.m_Setting;
		else
			s = 40;			// 100% power is the default. This will be the level used by the AI unless set otherwise.
		m_BayLock.unlock();

		return s;
	}

	virtual double BayWeaponRange(quint16 which) const = 0;

	inline AuId BayWeaponTarget(quint16 which) const
	{
		AuId target;

		m_BayLock.lockForRead();
		unordered_map<quint16, AuBaySetting>::const_iterator bsi = m_BaySetting.find(which);
		if(bsi != m_BaySetting.end())
			target = (*bsi).second.m_TargetId;
		m_BayLock.unlock();

		return target;
	}

	virtual float BayWeaponUnits(quint16 which) const = 0;

	// Returns the berth detail id occupied by 'id'.
	// In ScDesign based vessels, returns the part id.
	// Returns 0 if id is not berthed here.
	inline quint32 BerthDetailIdOccupied(const AuId& id) const
	{
		m_RosterLock.lockForRead();
		quint32 v = m_Roster.BerthDetailOccupied(id);
		m_RosterLock.unlock();
		return v;
	}

	// Returns the positioning matrix for the berth occupied by 'occupant'.
	// Returns false if id is not berthed in a bunk here. They could be berthed in a CaptainSlot.
	virtual bool BerthPosition(const DaCreature& occupant, AuMatrix4x4& berthM) const = 0;

	// Returns the name of the room that contains someone's berth.
	virtual QString BerthRoomName(const AuRosterEntry& rosterEntry) const = 0;

	// Returns the type of berth occupied by 'id'.
	// Returns AuRosterEntry::Invalid if id is not here.
	virtual AuRosterEntry::TYPE BerthTypeOccupied(const AuId& id) const
	{
		m_RosterLock.lockForRead();
		AuRosterEntry::TYPE v = m_Roster.BerthTypeOccupied(id);
		m_RosterLock.unlock();
		return v;
	}

	// 'inventory' is used to initialize the new passenger's gear.
	// 'fromWorld' may be 0
	DaNpc* BoardColonist(const AuId& fromAirport, const DaWorld* fromWorld, quint32 empireId, const QList<AuSpecies>& species,
			AuItemHash& inventory, quint8 bodyQ);

	// 'inventory' is used to initialize the new crewman's gear.
	DaNpc* BoardCrew(quint32 empireId, const QList<AuSpecies>& species, AuItemHash& inventory, quint8 bodyQ);

	void BoardCryoTraveller(const AuSpecies& species);

	// 'inventory' is used to initialize the new officer's gear.
	DaNpc* BoardOfficer(const DaWorldLL& llworld, quint32 empireId, const AuSpecies& species, AuItemHash& inventory, quint8 bodyQ);

	// 'inventory' is used to initialize the new troop's gear.
	DaNpc* BoardTroop(quint32 empireId, const QList<AuSpecies>& species, AuItemHash& inventory, quint8 bodyQ);

	// Apply forward/reverse braking.
	inline void Brake()
	{
		m_ControlLock.lockForWrite();
		m_wActions |= ACT_Brake;
		m_tmLastBrake.restart();
		m_ControlLock.unlock();
		SetModified();
	}

	// Apply lateral and vertical braking.
	inline void BrakeLV()
	{
		m_ControlLock.lockForWrite();
		m_wActions |= (ACT_BrakeLateral | ACT_BrakeVertical);
		m_tmLastBrakeLV.restart();
		m_ControlLock.unlock();
		SetModified();
	}

	inline void BrakeStop()
	{
		m_ControlLock.lockForWrite();
		m_wActions &= ~ACT_BrakeMask;
		m_ControlLock.unlock();
	}

	virtual bool BridgeVisible(const QString& roomName) const
	{
		Q_UNUSED(roomName);
		return false;
	}

	virtual void BuildingSurrendered(quint32 surrenderingEmpire, quint32 toEmpire);

	// Returns true if policy permits auto acceptance of a berth type request.
	inline bool CanAcceptBerthType(AuRosterEntry::TYPE type) const
	{
		bool canAccept = false;
		switch(type)
		{
		case AuRosterEntry::Invalid:
		case AuRosterEntry::OfficerAI:
			// These types can never be accepted automatically by the ship.
			break;
		case AuRosterEntry::CaptainSlot:
		case AuRosterEntry::Captain:
		case AuRosterEntry::OfficerHuman:
			// There are other conditions to accepting Captain's slot handled in the code.
			canAccept = true;
			break;
		case AuRosterEntry::Crew:
			canAccept = m_AcceptCrew;
			break;
		case AuRosterEntry::PassengerAI:
		case AuRosterEntry::PassengerHuman:
			canAccept = m_AcceptPassengers;
			break;
		case AuRosterEntry::Troop:
			canAccept = m_AcceptTroops;
			break;
		}
		return canAccept;
	}

	virtual bool CanAddCargo(const AuItem& iNew) const;

	// This version modifies 'iNew' so that it will fit. Returns false if the modified count is 0.
	bool CanAddCargoTrim(AuItem& iNew) const;

	void CancelWeaponBayFired(quint16 which);

	// true if 'whichFTL' can be disengaged.
	bool CanDisengageFTLNow(quint16 whichFTL) const;

	// true if all systems are go to commit FTL drive using FTL drive setting 0.
	bool CanEngageFTLAt0(quint16 whichFTL) const;

	// true if all systems are go to commit FTL drive using current FTL drive setting.
	bool CanEngageFTLNow(quint16 whichFTL) const;

	// Assumes current FTL drive module is a wormhole drive.
	// Insure the setting is sane for the current FTL drive module.
	virtual bool CanEnterNegativeWormhole() const = 0;

	// Assumes current FTL drive module is a wormhole drive.
	// Insure the setting is sane for the current FTL drive module.
	virtual bool CanEnterNeutralWormhole() const = 0;

	bool CanLoadTurretAmmunition(const AuItem& item, quint16 whichTurret) const;

	// Checks for officer aboard to accept a message. Officer takes the con if message is from an officer and nobody has the con.
	bool CanOfficerAcceptMessage(const AuCommPacket* msg);

	virtual bool CanOpenDoor(const DaSentient& who, unsigned int doorIndex) const;

	inline bool CanPassengersAccessSeaChest() const
	{
		return m_AllowPassengerSeaChests;
	}

	// true if all systems are go to keep FTL drive going current FTL drive setting.
	// Used to keep a warp drive going as long as there is power.
	bool CanReengageFTLNow(quint16 whichFTL) const;

	bool CanSelectBayWeaponAmmunition(const AuItem& item, quint16 whichBay) const;

	// Test to see if person 'id' from 'empireId' can spawn at aboard this vessel.
	// Returns 0=yes, 1=no life support inop, 2=no enemy non passenger, 3=no not berthed here, 4=no not normal
	// Privately owned ships never return 2.
	int CanSpawnHere(const AuId& id, quint32 empireId) const;

	virtual float CapacitorLevel() const
	{
		m_VesselLock.lockForRead();
		float v = m_CapacitorLevel;
		m_VesselLock.unlock();
		return v;
	}

	// Returns the number of energy units the capacitor can contain, adjusted for power plant system damage.
	virtual float CapacitorMax() const = 0;

	char CapacitorPercent() const;

	inline char CapacitorTrend() const
	{
		m_VesselLock.lockForRead();
		char v = m_CapacitorTrend;
		m_VesselLock.unlock();
		return v;
	}

	// Returns the Captain's atom id. 0 if no captain.
	// Only humans can be the captain. The captain is never an AI officer.
	inline AuId CaptainId() const
	{
		m_RosterLock.lockForRead();
		AuId v(m_Roster.CaptainId());
		m_RosterLock.unlock();
		return v;
	}

	inline QString CaptainName() const
	{
		m_RosterLock.lockForRead();
		QString v(m_Roster.CaptainName());
		m_RosterLock.unlock();
		return v;
	}

	// Returns true if the captain's berth/slot is occupied.
	inline bool CaptainsBerthOccupied() const
	{
		m_RosterLock.lockForRead();
		bool v = m_Roster.BerthsOccupied(AuRosterEntry::Captain) || m_Roster.BerthsOccupied(AuRosterEntry::CaptainSlot);
		m_RosterLock.unlock();
		return v;
	}

	// Specifically berths of the captain type.
	virtual unsigned int CaptainsBerths() const = 0;

	// Ship Roster
	// Returns the type of berth used as the captain's cabin.
	// CaptainSlot is returned when there are no berths in this design.
	AuRosterEntry::TYPE CaptainsBerthType() const;

	// Returns design hold capacity, in units.
	// Hold capacity is unaffected by damage to fuel tanks.
	virtual unsigned int CargoCapacityVolume() const = 0;

	virtual bool CargoHas(COMMODITYID cid) const
	{
		m_CargoLock.lockForRead();
		bool v = m_Cargo.contains(cid);
		m_CargoLock.unlock();
		return v;
	}

	inline quint32 CargoRawCount(COMMODITYID cid) const
	{
		m_CargoLock.lockForRead();
		quint32 v = m_Cargo.RawCount(cid);
		m_CargoLock.unlock();
		return v;
	}

	// Total volume is calculated a bit oddly. Containers count as their pack size x their count. Other items count as their count.
	inline unsigned int CargoTotalVolume() const
	{
		m_CargoLock.lockForRead();
		unsigned int v = m_Cargo.TotalCargoVolume();
		m_CargoLock.unlock();
		return v;
	}

	inline void ChargeCapacitor(float units)
	{
		float capMax = CapacitorMax();
		m_VesselLock.lockForWrite();
		if(m_CapacitorLevel < capMax)
		{
			m_CapacitorLevel += units;
			if(m_CapacitorLevel > capMax)
				m_CapacitorLevel = capMax;
			m_CapacitorTrend = 1;
		}
		m_VesselLock.unlock();
	}

	// Checks to see if 'unit' is granted access when calling for a beam up, recovery system, opening a hull door, etc.
	// Access does not override door locks.
	virtual bool CheckAccess(const DaUnit* unit) const;

	// Check for messages that trigger contact logic with AI officers, whether or not they are in command.
	// Triggers that contact when the message is of the proper type.
	// The only criteria is that the sender be in this ship's chain-of-command.
	bool CheckForContactByCommander(const AuCommPacket& msg);

	// Returns false if a wild animal is found or if no person on line is found.
	bool CheckSubatomsOkToSpawnMonster() const;

	// Checks for trespassers.
	void CheckTrespass(float sensorRange);

	virtual void ClaimTitleRecursive(const AuId& propertyId, const QString& propertyName, const AuId& ownerId,
			const QString& ownerName, quint32 ownerEmpire);

	// Cleans up food that is not on static display that would attract rats.
	// Returns true if any food is cleaned up.
	bool CleanHouse();

	inline void ClearCrewAndTroopsFromRoster()
	{
		// Roster returns true if anybody is removed.
		m_RosterLock.lockForWrite();
		if(m_Roster.ClearCrewAndTroops())
			m_tmRosterChange.Set();
		m_RosterLock.unlock();
	}

	void ClearRatSpecies();

	inline void ClearRoster()
	{
		m_RosterLock.lockForWrite();
		m_Roster.clear();
		m_tmRosterChange.Set();
		m_RosterLock.unlock();
	}

	// Does not call SetModified.
	virtual void CloseAllDoors() = 0;

	virtual void CloseAutomaticDoors() = 0;

	// Used by ParkingSpotInRange.
	double ClosestShipInSystem(const DaAtomList& ships, const AuPoint& loc, const AuId& shipToIgnore) const;

	// Locates the closest unmanned command station console to 'toLoc'.
	// Ignores stations that have no path from toLoc to the station, accessible to an officer of the ship.
	// Ignores stations that already have an AI dude to them, unless toLoc is in the same room as the station.
	virtual bool ClosestUnoccupiedCommandStation(const AuPoint& toLoc, quint16& stationId, AuStation& station) const = 0;

	// Locates the closest unmanned crew station console to 'toLoc'.
	// 'skill' is the best crew station skill of the searcher, for preference.
	// Ignores stations that have no path from toLoc to the station, accessible to crew of the ship.
	// Ignores stations that already have an AI dude to them, unless toLoc is in the same room as the station.
	virtual bool ClosestUnoccupiedCrewStation(const AuPoint& toLoc, quint16& stationId, AuStation& station,
			quint16& which, AuSkill::SKILL skill) const = 0;

	// Locates the closest unmanned Turret or TroopStation to 'toLoc'.
	// 'stationId' returns the part id in an AuScDesign.
	// Ignores stations that have no path from toLoc to the station, accessible to troops of the ship.
	// Ignores stations that already have an AI dude going to them, unless toLoc is in the same room as the station.
	// 'ignoreTroop' is passed to a call to ListOccupiedTroopStations.
	virtual bool ClosestUnoccupiedTroopStation(const AuPoint& toLoc, quint16& stationId, AuStation& station, quint16& which,
			const AuId& ignoreTroop) const = 0;

	// Returns the number of colonists aboard this ship.
	int ColonistsAboard() const;

	// Randomly locates a point inside the hull at which to place a colonist.
	// Returns false if it doesn't find a clear location within a reasonable amount of searching. Another try might succeed.
	// 'voidLoc' is an output parameter, only valid if return result is true.
	virtual bool ColonistVoidLocation(AuPoint& voidLoc) const = 0;
	
	virtual void CombatArmorEffectiveness(WEAPONMODE mode, WEAPONEFFECT effect, bool hitShield, const AuPoint& wpnLoc,
			const AuPoint& hitLoc, float& baseAbsorb, float& baseBlock, float& auxAbsorb, float& auxBlock) const;

	bool CombineCargo(const AuItem& intoItem, const AuItem& combItem);

	virtual bool CommAcceptBerth(int berthType, const AuCommAcceptBerth& msg);

	// Returns my location as briefly as possible in a single line suitable for use in a comm message.
	QString CommMessageLocation() const;

	virtual void ConfigureHarvester() = 0;

	// Used by AI to reconnect one disconnected system at the power relay console.
	// Returns true if anything is changed.
	bool ConnectOneSystem();

	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari);

	void CreditAccount(double amount);

	// Credit account with an amount due to sales of cargo aboard or transport of passengers.
	// Collects subsidy tax off the amount when appropriate.
	void CreditAccountSales(double amount);

	// Adds money to the subsidy tax account.
	void CreditAccountSubsidyTax(double amount);

	// Returns the gross number of crew berths aboard this ship, even if one is allocated to the captain.
	virtual unsigned int CrewBerths() const = 0;

	// Returns the number of berths available for occupants, considering that one might be allocated to the captain.
	inline unsigned int CrewBerthsAvailable() const
	{
		// Determine the number of crew berths on the ship that are not allocated to the captain.
		unsigned int berths = CrewBerths();
		if(!berths)
			return 0;	// This ship has no crew berths.
		if(CaptainsBerthType() == AuRosterEntry::Crew)
			berths--;	// One of the berths is allocated to the captain.
		if(!berths)
			return 0;	// This ship's only crew berth was allocated to the captain.

		// Check the roster for occupants.
		m_RosterLock.lockForRead();
		unsigned int occupants = m_Roster.BerthsOccupied(AuRosterEntry::Crew);
		m_RosterLock.unlock();
		if(!occupants)
			return berths;

		// Calculate the number of berths available.
		return berths - occupants;
	}

	// Returns the gross number of occupied crew berths aboard this ship, including any allocated to the captain.
	inline unsigned int CrewBerthsOccupied() const
	{
		// If the captain stays in this cabin type, count his berth too.
		unsigned int v;
		if(CaptainsBerthType() == AuRosterEntry::Crew)
		{
			m_RosterLock.lockForRead();
			v = m_Roster.BerthsOccupied(AuRosterEntry::Crew) + m_Roster.BerthsOccupied(AuRosterEntry::Captain);
			m_RosterLock.unlock();
		}
		else
		{
			m_RosterLock.lockForRead();
			v = m_Roster.BerthsOccupied(AuRosterEntry::Crew);
			m_RosterLock.unlock();
		}
		return v;
	}

	// Returns the gross number of cryo berths aboard this ship.
	virtual quint32 CryoBerths() const = 0;

	quint32 CryoBerthsAvailable() const;

	quint32 CryoBerthsOccupied() const;

	quint32 CryoBerthsOccupied(QStringList& cryoSpecies) const;

	// Complete the firing sequence of weapon bays.
	void CycleBayWeapons();

	// Generate/consume capacitor power for 1 second.
	// NOTE: CycleCapacitor is called on 1 second intervals.
	virtual void CycleCapacitor() = 0;

	virtual void CycleFTLDrive() = 0;

	// Induce enough damage to destroy the ship.
	virtual void DamageAndDestroy(const QString& reason) = 0;

	inline void DamageStateChanged()
	{
		m_SystemsLock.lockForWrite();
		if(UpdateSystemsHealthSystemsLocked())
			UpdateSystemsPowerSystemsLocked();
		m_SystemsLock.unlock();

		m_DamageLock.lockForWrite();
		m_tmDamageChange.Set();
		m_DamageLock.unlock();
		SetModified();
	}

	// Deactivates active rooms with no teleporter portals in them.
	virtual void DeactivateTransporterSystems() = 0;

	void DecayMoveActions();

	// Returns true when 'which' bay is no longer attached.
	// Calls CancelWeaponBayFired when no longer active.
	bool DecayTractorBay(quint16 which);

	inline void DecreaseThrottle()
	{
		// Do not permit throttle adjustments while moored. Throttle was required to be off to set moorings.
		if(m_Throttle > -10)
		{
			m_ControlLock.lockForWrite();
			m_Throttle--;
			m_tmLastThrottleControl.restart();
			m_ControlLock.unlock();
			SetInMotion();
			if(!IsTicking())
				TickTock();
		}
	}

	inline void DecreaseThrustLateral()
	{
		// Do not permit throttle adjustments while moored. Throttle was required to be off to set moorings.
		if(m_ThrustLateral > -5)
		{
			m_ControlLock.lockForWrite();
			m_ThrustLateral--;
			m_tmLastThrottleLControl.restart();
			m_ControlLock.unlock();
			SetInMotion();
			if(!IsTicking())
				TickTock();
		}
	}

	inline void DecreaseThrustLongitudinal()
	{
		// Do not permit throttle adjustments while moored. Throttle was required to be off to set moorings.
		if(m_ThrustLong > -10)
		{
			m_ControlLock.lockForWrite();
			m_ThrustLong--;
			m_tmLastThrottleFControl.restart();
			m_ControlLock.unlock();
			SetInMotion();
			if(!IsTicking())
				TickTock();
		}
	}

	inline void DecreaseThrustVertical()
	{
		// Do not permit throttle adjustments while moored. Throttle was required to be off to set moorings.
		if(m_ThrustVertical > -7)
		{
			m_ControlLock.lockForWrite();
			m_ThrustVertical--;
			m_tmLastThrottleVControl.restart();
			m_ControlLock.unlock();
			SetInMotion();
			if(!IsTicking())
				TickTock();
		}
	}

	void DebitAccount(double amount);

	void DebitAccountSubsidyTax(double amount);

	// Returns false if the atom is destroyed rather than depart a solar system.
	virtual bool DepartingSolarSystem();

	// Design CG offset from world CG, for old style designs that allowed the architect to move the CG.
	virtual AuPoint DesignCenterOffset() const = 0;

	virtual unsigned int DesignHitsStructural() const = 0;

	virtual unsigned int DesignHitsTotal() const = 0;

	virtual AuId DesignArchitectId() const = 0;

	virtual QString DesignArchitectName() const = 0;

	virtual quint32 DesignId() const = 0;

	virtual QString DesignName() const = 0;

	virtual QString DesignReport(quint8 minQ, bool showTips) const = 0;

	bool DestroyCargo(unsigned int unitsDamage);

	quint32 DestroyCryoBerthOccupants();

	bool DestroyFuel(unsigned int unitsDamage);

	void DestroyShip(const QString& reason);

	COMMODITYID DestroyVehicle();

	// Disconnects atoms that are not "inside" this ship and reparents them to my parent, with my movement vector.
	void DetachRiders();

	virtual void DetectWarpSignatureRecursive(const AuId& systemId, quint32 warpFactors, const AuPoint& galaxyLoc);

	virtual void DischargeCapacitor(float units)
	{
		m_VesselLock.lockForWrite();
		if(m_CapacitorLevel > 0)
		{
			if(units < m_CapacitorLevel)
				m_CapacitorLevel -= units;
			else
				m_CapacitorLevel = 0;
			m_CapacitorTrend = -1;
		}
		m_VesselLock.unlock();
	}

	// Switch to discharge state after actually wormholing.
	virtual void DischargeFTLDrive(quint16 whichFTL) = 0;

	// Asserts that the current order is a scuttle order.
	void DoScuttleOrder();

	bool EmergeFromWormhole(DaSolarSystem* inSystem);

	virtual void EmpireDeleted(quint32 empireId);

	// Returns the bunk index that is empty, if any.
	// 0=bottom berth, 1=top berth.
	// If any bunk is occupied, the other bunk is returned even if that other bunk is also occupied.
	virtual int EmptyBunkInBerth(const AuPoint& consoleLoc) const = 0;

	virtual void EngageFTLDrive(quint16 whichFTL) = 0;

	// Looks for the engineer, if there is one.
	// Returns a person on line who is at an engineer console, preferably.
	// Returns a crewman at an engineer console if no person on line is found at an engineer console.
	DaSentient* Engineer() const;

	// Return true to allow wander in acceleration, power plant, and turn skew values, which must be corrected at engineer station.
	// False is returned for small/auxiliary craft so they don't have to have an engineer station and engineer crewman.
	virtual bool EnginesRequireEngineer() const = 0;

	// Only Sc designs can be refit.
	// 'estCost', 'estSeconds', 'estQ' and 'estMaterials' are output parameters on good return.
	// Returns 0 if unable.
	// Returns 1 if ok.
	// Returns 2 if 'refit' is invalid.
	// Returns 3 if the refit doesn't change anything.
	virtual int EstimateRefit(const AuRefitShip& refit, double& estCost, quint32& estSeconds, AuRefitMaterials& estMaterials) const
	{
		Q_UNUSED(refit);
		Q_UNUSED(estCost);
		Q_UNUSED(estSeconds);
		Q_UNUSED(estMaterials);
		return 0;
	}

	inline int ETE() const
	{
		m_VesselLock.lockForRead();
		int v = m_ETE;
		m_VesselLock.unlock();
		return v;
	}

	virtual quint32 ExperiencePointsForKilling() const;

	double ExploreRadius(const DaWorldLL& world) const;

	// Clears rat species and removes all rats from the ship.
	// Returns bitmap: b0=rat species cleared, b1=rats killed.
	int ExterminateRats();

	virtual double FallSafeSpeed() const;

	virtual void FetchVehicle() = 0;

	// Fill up with fuel from a building.
	void FillUpFuel(DaBd* bd);

	virtual void FilterFinalMoveVector(AuPoint& vector) const;

	virtual void FilterPostMoveVector(AuPoint& vector) const;

	// Returns an atom that is in 'gunRange' of 'gunSysLoc', which is relative to the solar system.
	// 'gunnerEmpire' is the empire id of the gunner, who is operating the weapon.
	// 'targetAtom' and 'targetRange' are output parameters.
	virtual void FindClosestDefenseBaseTargetRecursive(int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
			const AuPoint& gunSysLoc, float gunRange, DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc);

	// Find atoms that might cause shields to be raised.
	virtual void FindClosestShipThreatRecursive(const AuId& shipId, int shipDomain, const AuId& shipWorldId, quint32 shipEmpire,
			SHIELD_POLICY shieldPolicy, const AuPoint& shipSysLoc, float sensorRange, const DaAtom*& targetAtom,
			double& targetRange) const;

	// Recursively locates the best target for a guided missile to lock on to.
	// 'launcherId' is the atom who actually launched the missile. This atom is never considered to be a target even when friendly
	// fire is enabled.
	// Should probably start this search at the solar system atom.
	// 'bestAtom' should be 0 on first call.
	// 'bestDistance' should be the maximum range of the sensor lock on first call. It can be -1 to ignore maximum range.
	// 'bestSysLoc' is an output parameter related to the atom that is returned.
	// This base class implementation checks all subatoms. Does not consider this atom to be a target.
	virtual const DaAtom* FindClosestMissileTarget(const AuId& launcherId, int missileDomain, const AuId& missileWorldId,
			quint32 missileEmpireId, const AuPoint& missileSysLoc, const DaAtom* bestAtom, AuPoint& bestSysLoc,
			double& bestDistance) const;

	virtual void FindClosestShipLockOnRecursive(int scannerDomain, const AuId& scannerWorldId, quint32 scannerEmpire,
			const AuId& scannerCapsuleId, const AuPoint& scannerSysLoc, float sensorRange,
			const DaVessel*& enemyVessel, double& vesselRange,
			const DaVessel*& nonEnemyStation, double& stationRange,
			const DaBd*& enemyMilBd, double& milRange,	// Enemy military building
			const DaBd*& anyBd, double& bdRange) const;

	// Returns an atom that is in 'gunRange' of 'gunSysLoc' and 'gunSysVec', which are relative to the solar system.
	// 'gunnerEmpire' is the empire id of the gunner, who is operating the weapon.
	// 'targetAtom' and 'targetRange' are output parameters.
	// 'targetSysLoc' is the location to target, relative to the solar system.
	virtual void FindClosestShipTurretTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
			quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
			const DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const;

	virtual void FindClosestShipWeaponTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
			quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& shipSysLoc, float gunRange,
			const DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const;

	// Locates a passenger aboard this ship.
	DaNpc* FindColonist() const;

	// Locates the AI officer who is commanding this ship.
	// Officer must have the con.
	DaSentient* FindCommandingOfficer() const;

	// Searches for any creature that prevents mothballing this ship.
	// Players and crew assigned to the ship prevent mothballing.
	const DaCreature* FindCreatureBlockingMothball() const;

	// Locates an AI officer assigned to this ship. Prefers the first officer.
	// Officer does not have to have the con.
	DaSentient* FindOfficer() const;

	// Locates a human officer aboard the ship who is on line.
	// So far this is only used to decide who to credit with exploring an undiscovered wormhole.
	// Returns in this order:
	//  1. The captain, if he is on line.
	//  2. A human officer who is on line.
	//  3. 0 if no on-line officer was found.
	DaPerson* FindOfficerOnLineRecursive();

	virtual DaVessel* FindParentVessel();

	virtual const DaVessel* FindParentVessel() const;

	virtual DaVessel* FindParentVesselInEnv();

	virtual const DaVessel* FindParentVesselInEnv() const;

	DaPerson* FindPersonOnLine(quint16 roomId) const;

	virtual void FindPirateStationsRecursive(DaAtomList& stations) const;

	// Lists all bds that can be reached by ground, station, or transporter. Starting point for various trade activities.
	void FindReachableBds(DaAtomList& reachableBds) const;

	// Lists cities in trade range that are capable of repairing this ship.
	// Cities will not be restricted by trade restrictions.
	// Cities will have the appropriate facilities.
	void FindRepairBds(DaAtomList& repairBds);

	// 'stationType' is an AuStation::TYPE enum. Not all station types are supported. See the code and usage in AuStory.
	// Returns the active operator, if there is one.
	AuId FindStationOccupant(int stationType) const;

	AuId FindStationOccupantInRoom(int stationType, const QString& room) const;

	// Lists cities in trade range that are capable of exporting 'tradeCID' to this ship.
	// Cities will not be restricted by trade restrictions.
	// Cities will have the appropriate facilities.
	void FindTradeBds(DaAtomList& tradeBds, COMMODITYID tradeCID) const;
	// 'reachableBds' is the result of calling FindReachableBds
	void FindTradeBds(DaAtomList& tradeBds, COMMODITYID tradeCID, const DaAtomList& reachableBds) const;

	// Called by stations and cities.
	// Searches recursively for any trespassing cities or stations.
	const DaAtom* FindTrespasserRecursive(quint32 empireId, const AuPoint& systemLoc, float sensorRange) const;

	// Locates a troop aboard this ship that is assigned to this ship.
	DaNpc* FindTroop() const;

	// Lists cities in trade range that are capable of exporting troops to this ship.
	// Cities will be in the empire of this ship.
	// Cities will have a base.
	void FindTroopBds(DaAtomList& troopBds);
	void FindCrewBds(DaAtomList& crewBds);
	void FindPassengerLoadBds(DaAtomList& bds);					// Bds from which passengers can be loaded.
	void FindPassengerUnloadBds(DaAtomList& bds);				// Bds to which passengers can be unloaded.

	virtual void FindVesselsRecursive(DaAtomList& spaceships) const;

	DaAnimal* FindWildAnimal(quint16 roomId) const;

	// 'unitsFired' is the number of weapon bay units to fire.
	// 'harvestIndex' is an index into an array of commodities for harvesters. See global Harvest functions in audef.h
	// Returns 'true' if the weapon is fired.
	bool FireBayWeapon(DaSentient& gunner, quint16 whichBay, float unitsFired, quint8 harvestIndex, const DaAtom* targetAtom,
			const AuPoint& targetSysLoc);

	// Returns the probability of catching fire when damaged by incendiary weapons.
	int FireProbability() const;

	// Returns 'true' if the turret is fired.
	// 'avoidFriends' is true, the weapon does not fire if it will hit a friendly unit.
	// 'target' may be null. If not null, it will be shot anyway even if it is a friend and 'avoidFriends' is true. 'target' is n/a
	//  if 'avoidFriends' is false.
	bool FireTurret(DaSentient& gunner, bool avoidFriends, const AuId& target);

	inline int FoodWantedByStation() const
	{
		// Stations like to keep 10% of their hold full of food.
		int cap = CargoCapacityVolume();
		int foodStockLvl = cap / 10;
		m_CargoLock.lockForRead();
		int food = m_Cargo.TotalFoodUnits();
		m_CargoLock.unlock();
		if(food >= foodStockLvl)
			return 0;	// No more food needed.
		return foodStockLvl - food;
	}

	// Returns friction in ft/sec/sec, typically due to surface contact.
	virtual AuPoint Friction() const;

	virtual FTL_COMMIT FTLDriveCommitState(quint16 whichFTL) const = 0;

	virtual quint8 FTLDriveSetting(quint16 whichFTL) const = 0;

	virtual FTL_DRIVE_STATE FTLDriveState(quint16 whichFTL) const = 0;

	// Returns the number of FTL drive units needed to engage an FTL drive, if I had one.
	virtual float FTLDriveUnitsToEngage() const = 0;

	// Test for FTL drive on and functioning.
	inline bool FTLDriveWorking(quint16 whichFTL) const
	{
		// Must be ON to work.
		if(FTLDriveState(whichFTL) != FTL_DRIVE_On)
			return false;

		// Capacitor must have some power for system to work.
		if(CapacitorLevel() <= 0)
			return false;

		// 100% damage to FTL drive will make it stop working.
		if(!IsDesignDataCacheValid() || !GetPowerConsumption(SYS_FTLDrive, whichFTL))
			return false;	// FTL drive is inop.

		// Everything looks good. FTL drive must be working.
		return true;
	}

	// Returns part id for the FTL drive that has a warp drive module.
	virtual quint16 FTLWarpDrive() const = 0;

	// Returns part id for the FTL drive that has a wormhole drive module.
	virtual quint16 FTLWormholeDrive() const = 0;

	virtual quint32 FuelCapacity() const = 0;

	quint8 FuelPercent() const;		// Fuel qty expressed as a percentage 0-100

	inline unsigned int FuelQuality() const
	{
		return m_FuelQuality;
	}

	// Returns current fuel quantity in cu/ft
	inline double FuelQuantity() const
	{
		m_VesselLock.lockForRead();
		float v = m_fFuelQuantity;
		m_VesselLock.unlock();
		return v;
	}

	// Returns the type of fuel required.
	COMMODITYID FuelType() const;

	inline quint32 FuelWanted() const
	{
		quint32 fuelCapacity = FuelCapacity();
		quint32 fuelQuantity = (quint32)FuelQuantity();
		if(fuelQuantity >= fuelCapacity)
			return 0;
		return fuelCapacity - fuelQuantity;
	}

	// Returns a user interface created by the current game, enabling the person to configure and submit their next turn when
	// it is their turn.
	virtual AuDialog* GameCreateTurnUI(quint16 whichTable, const AuId& personsTurn, QWidget* parent);

	// Exits a player from any game they're involved in. Terminates the game if it's the last person to leave.
	virtual void GameExitPlayer(quint16 whichTable, const AuId& playerId);

	// Returns the icon for the game in play at a table.
	QIcon GameIcon(quint16 whichTable) const;

	// Returns the name of the game in play at a table.
	QString GameName(quint16 whichTable) const;

	// Selects a nu game for a game table, if there is no game currently active at that table.
	virtual bool GameSelected(quint16 whichTable, quint8 gameType, const AuId& dealerId);

	// Sets the players in a game according to who is at the same table as the dealer and their seating order.
	// Can be called on the client.
	virtual void GameSetPlayers(quint16 whichTable, AuGame* game, const AuId& dealerId) const;

	// Starts the game at a table. Game settings are sent in a byte array so they are generic between all game types.
	virtual void GameStart(DaPerson* dealer, quint8 gameType, const QByteArray& gameSettings);

	// NOTE: Expects m_GamesLock to be locked.
	inline void GamesSetModified()
	{
		m_GamesChanged.Set();
		SetModified();
	}

	// Games only get ticked every even second. No table top games need faster response than that.
	// 'validatePlayers' is true to verify all players are still present and in the game.
	void GamesTick(bool validatePlayers);

	// Sends the game turn info when a player executes their turn.
	// Game turn data is sent in a byte array so it is generic between all game types.
	virtual void GameTurn(DaPerson* person, quint8 gameType, const QByteArray& gameTurn);

	// Checks to see if the game turn UI is still appropriate for the current game state.
	// Returns false if the turnUI should be deleted.
	virtual bool GameTurnUIValid(quint16 whichTable, const AuId& personsTurn, AuDialog* turnUI);

	// Returns the AuGame::GameType() in play at a game table.
	quint8 GameType(quint16 whichTable) const;

	bool GetNextWeaponBayFired(AuBayFired& bayFired, quint16& which);

	// Returns the Current Power Consumption for the indicated system
	// Returns 0 on unknown system
	inline unsigned int GetPowerConsumption(SYSTEM q8SystemType) const
	{
		m_SystemsLock.lockForRead();
		unsigned int v = m_ssaShipSystems.GetPowerConsumption(q8SystemType);
		m_SystemsLock.unlock();
		return v;
	}

	// Returns the Current Power Consumption for the indicated system
	// Returns 0 on unknown system
	inline unsigned int GetPowerConsumption(SYSTEM q8SystemType, quint16 which) const
	{
		m_SystemsLock.lockForRead();
		unsigned int v = m_ssaShipSystems.GetPowerConsumption(q8SystemType, which);
		m_SystemsLock.unlock();
		return v;
	}

	inline unsigned int GetPowerConsumption(const AuSysId& sysId) const
	{
		if(sysId.Which())
			return GetPowerConsumption(sysId.System(), sysId.Which());
		return GetPowerConsumption(sysId.System());
	}

	inline quint8 GetShunts() const
	{
		return m_PowerPlantShunts;
	}

	inline int GetSystemCount() const
	{
		// Scene client does rx enough m_ssaShipSystems data to do this.
		m_SystemsLock.lockForRead();
		int v = m_ssaShipSystems.count();
		m_SystemsLock.unlock();
		return v;
	}

	void GoDormant();

	// ManeuverDriveState
	inline GRAVITY_DRIVE_STATE GravityDriveState() const
	{
		m_VesselLock.lockForRead();
		GRAVITY_DRIVE_STATE v = m_eGravityDriveState;
		m_VesselLock.unlock();
		return v;
	}

	// Test for gravity drive on and functioning.
	virtual bool GravityDriveWorking() const;

	void Harvest(quint16 which, COMMODITYID bayModule, double wpnRange, double targDist, int probabilityToHarvest,
			COMMODITYID harvestCID, quint8 harvestQ);

	virtual float HarvesterBayRange(float unitsFired, quint16 which) const = 0;

	// Returns true if this ship has an AI officer on the roster, whether or not they have the con.
	// Note: The first officer may not be in command of the ship at present. Test IsCommandedByAI for that.
	inline bool HasFirstOfficer() const
	{
		return AIFirstOfficer().IsNull() ? false : true;
	}

	virtual bool HasFTLDrive() const = 0;

	// true if this ship has a captain or any human officer assigned.
	inline bool HasHumanOfficer() const
	{
		if(CaptainsBerthOccupied())
			return true;
		m_RosterLock.lockForRead();
		bool v = m_Roster.HasHumanOfficer();
		m_RosterLock.unlock();
		return v;
	}

	virtual bool HasManeuverDrive() const = 0;

	// true if this ship has a captain or any officers assigned.
	// This may include AI controlled officers.
	inline bool HasOfficers() const
	{
		return (CaptainsBerthOccupied() || OfficerBerthsOccupied()) ? true : false;
	}

	virtual bool HasPowerPlant() const = 0;

	inline bool HasRatSpecies() const
	{
		m_VesselLock.lockForRead();
		bool v = !m_RatSpecies.IsNull();
		m_VesselLock.unlock();
		return v;
	}

	virtual bool HasRetractableLandingGear() const
	{
		return false;
	}

	virtual bool HasSpaceVehicleLauncher() const
	{
		return false;
	}

	// Returns true if this ship's maneuver drive module is a rocket drive, regardless of whether it is functioning or not.
	inline bool HasRocketDrive() const
	{
		//Q_ASSERT(IsDesignDataCacheValid());
		return SystemModule(SYS_ManeuverDrive) == CID_RocketDriveModule;
	}

	virtual bool HasShields() const = 0;

	virtual bool HasVehicleTransporter() const;

	virtual bool HasWarpFTLDrive() const = 0;

	inline bool HasWaypoint() const
	{
		m_WaypointLock.lockForRead();
		bool v = !m_aWaypoints.isEmpty();
		m_WaypointLock.unlock();
		return v;
	}

	inline bool HasWaypoint(int sequence) const
	{
		m_WaypointLock.lockForRead();
		bool v = sequence >= 0 && sequence < m_aWaypoints.count();
		m_WaypointLock.unlock();
		return v;
	}

	virtual bool HasWeaponBay() const = 0;

	virtual bool HasWormholeFTLDrive() const = 0;

	void HeadlightToggle(bool onState)
	{
		m_bExtLightOn = onState;
	}

	virtual double Height() const = 0;

	// Returns the location of the first helm found in this design.
	virtual AuPoint HelmLocation() const = 0;

	// Returns the distance at which this ship always shows up on sensors, even if it is hiding in ground clutter.
	inline static double HideInGroundClutterRange2()
	{
		return AtmosphereTopMSL()*AtmosphereTopMSL();
	}

	quint8 HighestSystemQuality() const;

	inline unsigned int HitsDamageStructural() const
	{
		m_DamageLock.lockForRead();
		unsigned int v = m_Damage.HitsDamageStructural();
		m_DamageLock.unlock();
		return v;
	}

	// Returns the number of hits of damage to a system.
	unsigned int HitsDamageSystem(SYSTEM system) const;

	// Returns level of streamlining of the hull.
	virtual STREAMLINE HullStreamline() const = 0;

	// Returns current stress to the hull due to atmospheric drag.
	// QueryEnvironment has already been called and this vessel IsInAtmosphere.
	virtual float HullStress() const = 0;

	// Returns true if 'atom' is in the chain of command of this ship.
	// Returns true if 'atom' has agent power enabled.
	virtual bool InChainOfCommand(const DaAtom* atom) const;

	// Returns true if the person is in the chain of command of this ship.
	virtual bool InChainOfCommand(const AuId& personId, quint32 personEmpire) const;

	inline void IncreaseMonster(quint16 monsterInc, MONSTER_ORIGIN origin)
	{
		m_VesselLock.lockForWrite();
		quint32 monster = m_Monster + monsterInc;
		if(monster >= 0xffff)
			m_Monster = 0xffff;
		else
			m_Monster = (quint16)monster;
		m_MonsterOrigin = origin;
		m_VesselLock.unlock();
		SetModified(NOTIFY_Database);
	}

	inline void IncreaseThrottle()
	{
		// Do not permit throttle adjustments while moored. Throttle was required to be off to set moorings.
		if(m_Throttle < 10)
		{
			m_ControlLock.lockForWrite();
			m_Throttle++;
			m_tmLastThrottleControl.restart();
			m_ControlLock.unlock();
			SetInMotion();
			if(!IsTicking())
				TickTock();
		}
	}

	inline void IncreaseThrustLateral()
	{
		// Do not permit throttle adjustments while moored. Throttle was required to be off to set moorings.
		if(m_ThrustLateral < 5)
		{
			m_ControlLock.lockForWrite();
			m_ThrustLateral++;
			m_tmLastThrottleLControl.restart();
			m_ControlLock.unlock();
			SetInMotion();
			if(!IsTicking())
				TickTock();
		}
	}

	inline void IncreaseThrustLongitudinal()
	{
		// Do not permit throttle adjustments while moored. Throttle was required to be off to set moorings.
		if(m_ThrustLong < 10)
		{
			m_ControlLock.lockForWrite();
			m_ThrustLong++;
			m_tmLastThrottleFControl.restart();
			m_ControlLock.unlock();
			SetInMotion();
			if(!IsTicking())
				TickTock();
		}
	}

	inline void IncreaseThrustVertical()
	{
		// Do not permit throttle adjustments while moored. Throttle was required to be off to set moorings.
		if(m_ThrustVertical < 7)
		{
			m_ControlLock.lockForWrite();
			m_ThrustVertical++;
			m_tmLastThrottleVControl.restart();
			m_ControlLock.unlock();
			SetInMotion();
			if(!IsTicking())
				TickTock();
		}
	}

	void InsertWaypointAt(int sequence, const AuId& idDestSolarSystem, const AuId& idDestAtom);

	void InsertWaypointAt(int sequence, const AuId& idDestSolarSystem, const AuId& idDestAtom, const AuPoint& vFromDestAtom);

	// Install an upgrade module taking the time prescribed in 'installSecs'.
	// Returns true if the module is installed.
	// If installed, AddRepairTime is called with 'installSecs'.
	virtual bool InstallSystemModule(const AuSysId& sysId, const AuItem& module, quint16 installSecs);

	// Returns the state of the human pilot flag to unknown so AI will resume control.
	// Clears human control time so server-side movement physics will take over in Tick.
	inline void InvalidatePilotPerson()
	{
		m_ConPilotPerson = 0;		// unknown
		SetControlled(AuId());		// cleared
	}

	// true if any FTL drive is committed and hot
	virtual bool IsAnyFTLDriveHot() const = 0;

	virtual bool IsAnyFTLDriveWorking() const = 0;

	inline bool IsBayFiring(quint16 which) const
	{
		m_BayLock.lockForRead();
		bool cw = m_BayFired.find(which) != m_BayFired.cend();
		m_BayLock.unlock();
		return cw;
	}

	virtual bool IsBerthedHere(const AuId& id) const
	{
		return BerthTypeOccupied(id) != AuRosterEntry::Invalid;
	}

	// Returns true if this vessel is civilian.
	virtual bool IsCivilian() const;

	// Returns true if 'stationId' is a captain's chair.
	virtual bool IsCommandConsole(quint16 stationId) const = 0;

	// Returns true if this ship is currently under the command of an Npc officer.
	// The ship is commanded by AI when an officer has the con.
	virtual bool IsCommandedByAI() const;

	// Determine if a console is functioning for a given subsystem 'which'. It might be inoperative due to system damage.
	virtual bool IsConsoleOperational(const AuSysId& sysId) const;

	// Returns true if this ship is covered by ATC; it could be friendly or enemy.
	// Ships at rest are not covered by ATC.
	inline bool IsCoveredByATC() const
	{
		// Spacecraft inside other spacecraft are not covered by ATC.
		if(IsInSpacecraft())
			return false;
		// A station provides ATC coverage that always includes itself.
		if(IsSpaceStation())
			return true;
		// All spacecraft in an arena are covered.
		if(InArena())
			return true;
		if(InDesignEnvironment())
			return true;
		m_CommLock.lockForRead();
		bool v = !m_ATCCity.IsNull() || !m_ATCStation.IsNull() || !m_ATCCityEnemy.IsNull() || !m_ATCStationEnemy.IsNull();
		m_CommLock.unlock();
		return v;
	}

	// Check to see an id occupies a captain, officer, crew or troop berth. False for passengers and others.
	virtual bool IsCrewOrTroop(const AuId& id) const
	{
		return AuRosterEntry::IsCrewOrTroop(BerthTypeOccupied(id));
	}

	virtual bool IsDamaged() const;

	virtual bool IsDoorJammed(unsigned int doorIndex) const = 0;

	virtual bool IsDoorLocked(unsigned int doorIndex) const = 0;

	// Test for engineer console manned by an AI or a person.
	inline bool IsEngineerManned() const
	{
		return IsEngineerMannedByAI() || IsEngineerMannedByPerson();
	}

	// Test for engineer console manned by an AI.
	bool IsEngineerMannedByAI() const;

	// Test for engineer console manned by a person.
	bool IsEngineerMannedByPerson() const;

	virtual bool IsFirstClass() const = 0;

	// true if FTL drive is committed and hot
	virtual bool IsFTLDriveHot(quint16 whichFTL) const = 0;

	// Test for state of FTL drive power switch. Does not mean the FTL drive is working. See FTLDriveWorking.
	virtual bool IsFTLDriveOn(quint16 whichFTL) const = 0;

	// Returns an atom that is in 'gunRange' of 'gunLoc', which is relative to my parent world.
	// 'gunnerEmpire' is the empire id of the gunner, who is at my parent world.
	// 'gunnerPosture' is the posture of the gunner. Caller checks POSTURE_HoldFire before calling to save lots of redundant checks.
	// 'targetLoc' is an output parameter relative to my parent world.
	virtual bool IsGuardTowerTarget(quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunLoc, float gunRange,
			AuPoint& targetLoc) const;

	// This returns the current state of the light switch. Check CapacitorLevel to insure lights can operate.
	inline bool IsHeadlightOn() const
	{
		return m_bExtLightOn;
	}

	virtual bool IsHidingInGroundClutter() const;

	// Returns true if we've been in range of an enemy bay weapon within the last second.
	inline bool IsInEnemyRange() const
	{
		// A fire control station operator might only scan us once every 4500ms plus or minus.
		return WasInEnemyRange(8000);
	}

	// True when any air pressure in the ship is breathable. DaSc designs vary air pressure by room.
	// False when air pressure throughout the ship is not breathable.
	virtual bool IsInsideAirBreathable() const = 0;

	inline bool IsInSunSafeZone() const
	{
		return EnvLocation().Length2() < 10*10 && !InArena() && !InDesignEnvironment();
	}

	inline bool IsInTow() const
	{
		return m_InTow;
	}

	// Returns true for spacecraft that do not have landing gear, which implies a fixed gear design.
	virtual bool IsLandingGearExtended() const
	{
		return true;
	}

	// Returns true if the current mission is active, regardless of whether it has any orders.
	// Use 'IsOnAMission' to test for active AND non-empty mission.
	inline bool IsMissionActive() const
	{
		m_MissionLock.lockForRead();
		bool is = m_Mission.IsActive();
		m_MissionLock.unlock();
		return is;
	}

	inline bool IsMissionOrder(const AuMissionOrder& o) const
	{
		m_MissionLock.lockForRead();
		bool is = m_Mission.CurrentOrder() == o;
		m_MissionLock.unlock();
		return is;
	}

	inline bool IsMoored() const
	{
		return m_Moored;
	}

	// Test for navigator console manned by an AI or a person.
	inline bool IsNavigatorManned() const
	{
		return IsNavigatorMannedByAI() || IsNavigatorMannedByPerson();
	}

	// Test for navigator console manned by an AI.
	bool IsNavigatorMannedByAI() const;

	// Test for navigator console manned by a person.
	bool IsNavigatorMannedByPerson() const;

	// This returns the current state of the light switch. Check CapacitorLevel to insure lights can operate.
	virtual bool IsNavLightOn() const
	{
		return false;
	}

	// Check to see if a person is occupying an officer-level berth.
	inline bool IsOfficer(const AuId& id) const
	{
		AuRosterEntry::TYPE type = BerthTypeOccupied(id);
		return AuRosterEntry::IsOfficer(type);
	}

	// Returns true if the current mission is active AND has a mission order.
	// use 'IsMissionActive' to test only the mission active/hold state.
	inline bool IsOnAMission() const
	{
		m_MissionLock.lockForRead();
		bool is = m_Mission.IsActive() && !m_Mission.IsEmpty();
		m_MissionLock.unlock();
		return is;
	}

	// Test for pilot console manned by an AI or a person.
	inline bool IsPilotManned() const
	{
		return IsPilotMannedByAI() || IsPilotMannedByPerson();
	}

	// Test for pilot console manned by an AI.
	bool IsPilotMannedByAI() const;

	// Test for pilot console manned by a person.
	bool IsPilotMannedByPerson() const;

	// Returns true if this ship is commanded by a pirate AI.
	bool IsPirateCommander() const;

	// Returns true if this is a pirate owned ship that does not have a human captain.
	// So far, only an agent can be the captain of a pirate ship.
	// Returns true if this is an AI controlled ship in an arena.
	bool IsPirateWithFreeWill() const;

	inline bool IsPrivatelyOwned() const
	{
		return m_IsPrivate;
	}

	inline bool IsPowerPlantOn() const
	{
		return m_bPowerPlantOn;
	}

	// Test for power relay console manned by an AI or a person.
	inline bool IsPowerRelayManned() const
	{
		return IsPowerRelayMannedByAI() || IsPowerRelayMannedByPerson();
	}

	// Test for power relay console manned by an AI.
	bool IsPowerRelayMannedByAI() const;

	// Test for power relay console manned by a person.
	bool IsPowerRelayMannedByPerson() const;

	// Returns true if the ships is at rest and all systems are in a state that allows repairs.
	// Does not check location for proximity to stations or landing pad.
	bool IsReadyForRepairs() const;

	// true if wormhole FTL drive is committed and ready to go through a wormhole
	bool IsReadyToWormhole() const;

	// This returns the current state of the light switch. Check CapacitorLevel to insure lights can operate.
	virtual bool IsRotatingBeaconOn() const
	{
		return false;
	}

	bool IsSafeToLeaveSolarSystem() const;

	virtual bool IsSecondClass() const = 0;

	// Test for sensor console manned by an AI or a person.
	inline bool IsSensorManned() const
	{
		return IsSensorMannedByAI() || IsSensorMannedByPerson();
	}

	// Test for sensor console manned by an AI.
	bool IsSensorMannedByAI() const;

	// Test for sensor console manned by a person.
	bool IsSensorMannedByPerson() const;

	// Returns true if the current sensor lock was acquired by a human.
	inline bool IsSensorLockedByHuman() const
	{
		return m_IsHumanLock;
	}

	// True if the sensor power switch is on, regardless of whether it is functional.
	virtual bool IsSensorOn() const
	{
		return m_bSensorsOn;
	}

	// Test for shield console manned by an AI or a personn.
	inline bool IsShieldManned() const
	{
		return IsShieldMannedByAI() || IsShieldMannedByPerson();
	}

	// Test for shield console manned by an AI.
	bool IsShieldMannedByAI() const;

	// Test for shield console manned by a person.
	bool IsShieldMannedByPerson() const;

	// True if the shield power switch is on, regardless of whether it is functional.
	inline SHIELD_STATE ShieldState() const
	{
		return m_ShieldState;
	}

	// Return true if this atom can be targetted by a turret on a spacecraft.
	// 'gunnerEmpire' is the empire id of the gunner. It is 0 when political stance is to be ignored.
	// 'gunnerPosture' is the posture of the gunner. Caller checks POSTURE_HoldFire before calling to save lots of redundant checks.
	// 'gunSysLoc' and 'gunSysVec' are relative to the solar system.
	// 'targetSysLoc' is the location to target, relative to the solar system.
	virtual bool IsShipTurretTarget(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
			CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
			AuPoint& targetSysLoc) const;

	virtual bool IsShipWeaponTarget(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
			CITIZEN_POSTURE gunnerPosture, const AuPoint& shipSysLoc, float gunRange, AuPoint& targetSysLoc) const;

	virtual bool IsSightTarget(const DaAtom* /*toWho*/) const
	{
		// This atom can be sighted when targeting a weapon or tool.
		return true;
	}

	bool IsSmoldering() const;

	// Checks to see if this ship is standing down.
	// Returns false if not.
	bool IsStandingDown() const;

	inline bool IsStarship() const
	{
		return Structure() == SC_Starship;
	}

	// This returns the current state of the light switch. Check CapacitorLevel to insure lights can operate.
	virtual bool IsStrobeOn() const
	{
		return false;
	}

	inline bool IsSupplierOverclocked(int supplierIndex) const
	{
		//Q_ASSERT(!g_Au.IsSceneClient());	// Scene client does not rx enough m_ssaShipSystems data to do this.
		bool v;
		m_SystemsLock.lockForRead();
		if(supplierIndex >= 0 && supplierIndex < m_ssaShipSystems.count())
			v = m_ssaShipSystems.at(supplierIndex).GetOverclocking();
		else
			v = false;
		m_SystemsLock.unlock();
		return v;
	}

	// Returns true if 'targetShip' is being towed by my tractor beam.
	// 'tractorRange' is set to the maximum range of the tractor that is towing the target ship.
	// 'tractorAccel' is set to the maximum acceleration of the tractor that is towing the target ship.
	bool IsTowing(const AuId& targetShip, float targetMass, double& tractorRange, double& tractorAccel) const;

	inline bool IsTractorBeamAttached(AuPoint& epos) const
	{
		bool v = false;
		m_BayLock.lockForRead();
		for(unordered_map<quint16, AuBayFired>::const_iterator lfi = m_BayFired.cbegin(); lfi != m_BayFired.cend(); lfi++)
			if((*lfi).second.IsTractorAttached())
			{
				epos = (*lfi).second.m_EmitLoc;
				v = true;
				break;
			}
		m_BayLock.unlock();
		return v;
	}

	inline bool IsTurning() const
	{
		return m_wActions & (ACT_TurnMask|ACT_PitchMask|ACT_RollMask);
	}

	virtual bool IsVisibleNearWorld() const;

	// Checks to see if the current mission order is a Wait order.
	// Returns false if the mission is disabled or empty.
	inline bool IsWaiting() const
	{
		return (IsOnAMission() && MissionOrderType() == AuMissionOrder::Stay);
	}

	// Test for weapon console manned by an AI or a person.
	inline bool IsWeaponManned() const
	{
		return IsWeaponMannedByAI() || IsWeaponMannedByPerson();
	}

	// Test for weapon console manned by an AI.
	bool IsWeaponMannedByAI() const;

	// Test for weapon console manned by a person.
	bool IsWeaponMannedByPerson() const;

	// Kicks AI controlled operator of a console, if there is one.
	virtual void KickAIStationOccupant(quint16 stationId, unsigned int seatIndex);

	quint32 KickCryoBerthOccupants(const AuSpecies& species, quint32 count);

	// Locates all players aboard recursively.
	// People who are not on the roster are recalled home immediately.
	// When 'offLineOnly' is true, only off line players are kicked.
	// Agents with agent powers enabled are ignored.
	// 'stowaways' is an output parameter filled with the names of all people kicked.
	void KickStowaways(QStringList& stowaways, bool offLineOnly);

	void KillColonist();

	// Kills this ship atom if it is dormant and abandoned.
	// A dormant ship is either hangared or in stasis.
	// Hangared ships are abandoned when not in a hangar.
	// Stasis ships are abandoned when no human officer remains on the roster.
	// Returns true if this ship is killed.
	bool KillDormantAbandoned();

	// Called when this unit is killed by a person.
	virtual void KilledByPerson(DaPerson* killer);

	// Called when this unit is killed by another unit.
	virtual void KilledByUnit(const DaUnit* killer);

	inline void KillThrottles()
	{
		SetThrottle(0);
		SetThrustLateral(0);
		SetThrustLongitudinal(0);
		SetThrustVertical(0);
	}

	void KillTroop();

	virtual void LandingGearToggle(bool isExtended)
	{
		Q_UNUSED(isExtended);
	}

	virtual LANDING_ORIENT LandingOrientation() const
	{
		return LORI_Belly;
	}

	// Returns the time the last Alert message was sent by a call to SendAlert or firing alert message broadcast.
	inline AuElapsedTimer LastAlert() const
	{
		m_CommLock.lockForRead();
		AuElapsedTimer v(m_tmAlert);
		m_CommLock.unlock();
		return v;
	}

	inline AuElapsedTimer LastATCContact() const
	{
		m_CommLock.lockForRead();
		AuElapsedTimer v(m_ATCContact);
		m_CommLock.unlock();
		return v;
	}

	// Returns the time the last EnvDamage message was sent by a call to SendEnvDamage.
	inline AuElapsedTimer LastEnvDamage() const
	{
		m_CommLock.lockForRead();
		AuElapsedTimer v(m_tmEnvDamage);
		m_CommLock.unlock();
		return v;
	}

	// Returns the time the last SOS message was sent by a call to SendSOS.
	inline AuElapsedTimer LastSOS() const
	{
		m_CommLock.lockForRead();
		AuElapsedTimer v(m_tmSOS);
		m_CommLock.unlock();
		return v;
	}

	virtual double Length() const = 0;

	// Test for life support functioning.
	virtual bool LifeSupportWorking() const;

	// List locations of troop stations where troops of this ship's empire are standing, except 'exceptTroop'.
	// Locations are in design coordinates.
	virtual void ListOccupiedTroopStations(QSet<quint16>& troopStations, const AuId& exceptTroop) const = 0;

	void ListOrdnanceAvailable(AuItemHash& ord, quint32 empireId) const;

	// 'hitsNeeded' is set to the number of hit points of damage points that need repairing.
	// 'partsNeeded' is filled with parts needed for the repairs.
	// All parameters are output parameters. They will get set to something; 0 if no repairs are needed.
	void ListRepairsNeeded(quint32& hitsNeeded, AuRepairParts& partsNeeded) const;

	// 'hitsNeeded' is set to the number of hit points of damage points that need repairing.
	// 'partsNeeded' is filled with parts needed for the repairs.
	// Most parameters are output parameters. They will get set to something; 0 if no repairs are needed.
	void ListRepairsNeeded(const AuSysId& sysId, quint32& hitsNeeded, AuRepairParts& partsNeeded) const;

	virtual void ListSystems(QList<AuSystemInfo>& systems) const = 0;

	// Fills a list of the ids that are towed by tractor beams.
	// Returns false if no tractors are attached.
	inline bool ListTractorBeamTowed(AuIdList& towed) const
	{
		bool v = false;
		m_BayLock.lockForRead();
		for(unordered_map<quint16, AuBayFired>::const_iterator lfi = m_BayFired.cbegin(); lfi != m_BayFired.cend(); lfi++)
			if((*lfi).second.IsTractorAttached())
			{
				towed.append((*lfi).second.m_Target);
				v = true;
			}
		m_BayLock.unlock();
		return v;
	}

	// Loads up to one officer and fills empty crew and troop berths.
	void LoadCrew(quint32 empireId, const AuSpecies& species);

	void LoadPirateCrew(quint32 pirateEmpire);

	DaNpc* LoadPirateCrew(quint32 pirateEmpire, const AuSpecies& species);

	DaNpc* LoadPirateOfficer(quint32 pirateEmpire, const AuSpecies& species);

	DaNpc* LoadPiratePassenger(quint32 pirateEmpire, const AuSpecies& species);

	DaNpc* LoadPirateTroop(quint32 pirateEmpire, const AuSpecies& species);

	// 'whichTurret' is the system which identifier of the turret to load, from DaCreature::ConsoleWhich.
	void LoadTurretAmmunition(const AuItem& item, quint16 whichTurret);

	// 'atom' is expected to be subordinate to this vessel, not necessarily inside.
	virtual QString LocationDesc(const DaAtom& atom) const = 0;

	// Returns a pointer to the ammo item. The count is set the the number available in the hold.
	inline const AuItem* LockBayWeaponAmmunition(quint16 which) const
	{
		// This is locked for write because the code that calls this calls WeaponBayFired next.
		m_BayLock.lockForWrite();

		unordered_map<quint16, AuBaySetting>::const_iterator bsi = m_BaySetting.find(which);
		if(bsi != m_BaySetting.end() && (*bsi).second.m_Ammo)
		{
			m_CargoLock.lockForWrite();
			AuItemHash::const_iterator ammoI = m_Cargo.Find(*(*bsi).second.m_Ammo);
			if(ammoI == m_Cargo.end())
				(*bsi).second.m_Ammo->SetCount(0);
			else
				(*bsi).second.m_Ammo->SetCount((*ammoI).second->Count());
			m_CargoLock.unlock();
			return (*bsi).second.Ammo();
		}
		return 0;
	}

	virtual AuItemHash& LockCargoForWrite()
	{
		m_CargoLock.lockForWrite();
		return m_Cargo;
	}

	virtual const AuItemHash& LockCargoForRead() const
	{
		m_CargoLock.lockForRead();
		return m_Cargo;
	}

	inline const AuCryoBerths& LockCryoBerths() const
	{
		m_RosterLock.lockForRead();
		return m_CryoBerths;
	}

	// Returns atom id of locked sensor blip.
	virtual AuId LockedBlip() const
	{
		m_VesselLock.lockForRead();
		AuId v(m_LockedBlip);
		m_VesselLock.unlock();
		return v;
	}

	// Returns true if the locked sensor blip is civilian.
	// NOTE: Only valid on the client.
	virtual bool LockedIsCivilian() const
	{
		return m_LockedIsCivilian;
	}

	// Returns political stance toward locked sensor blip.
	// NOTE: Only valid on the client.
	virtual POLITICAL_STANCE LockedStance() const
	{
		return (POLITICAL_STANCE)m_LockedStance;
	}

	// Returns system location of locked sensor blip.
	// NOTE: Only valid on the client.
	virtual AuPoint LockedSysLoc() const
	{
		m_VesselLock.lockForRead();
		AuPoint v(m_LockedSysLoc);
		m_VesselLock.unlock();
		return v;
	}

	inline const AuMission& LockMission() const
	{
		m_MissionLock.lockForRead();
		return m_Mission;
	}

	inline const AuMissionOrder& LockMissionOrder() const
	{
		m_MissionLock.lockForRead();
		return m_Mission.CurrentOrder();
	}

	// Lock the sensors onto an atom in the sensor scan data.
	virtual void LockOn(const AuId& lockOnId, bool isLockedByHuman)
	{
		if(!lockOnId.IsNull())
		{
			m_VesselLock.lockForWrite();
			m_LockedBlip = lockOnId;
			m_IsHumanLock = isLockedByHuman;	// Remember when a human locks-on with the sensors so AI will not override.
			m_VesselLock.unlock();
		}
		else
		{
			m_VesselLock.lockForWrite();
			m_LockedBlip = AuId(0);
			m_IsHumanLock = false;				// Clear human lock flag when sensor lock cleared.
			m_VesselLock.unlock();
		}
	}

	// Read access to the roster, mainly for the ship roster manager dialog.
	inline const AuRoster& LockRoster() const
	{
		m_RosterLock.lockForRead();
		return m_Roster;
	}

	inline const AuShipSystemArray& LockSystems() const
	{
		m_SystemsLock.lockForRead();
		return m_ssaShipSystems;	// List of all Ship Systems. ONLY A SUBSET OF THIS DATA IS SENT TO THE SCENE CLIENT.
	}

	inline unordered_map<quint16, AuOrdnance>& LockTurretOrdnance()
	{
		m_TurretLock.lockForWrite();
		return m_Turrets;
	}

	const DeWaypoint* LockWaypoint(int sequence) const;

	// Notifies a commanding AI officer of an event worth noting in the ship's log.
	// Nothing happens if there is no AI officer aboard to log the event.
	virtual void LogEvent(AuEventLogEntry::TYPE type, const QString& desc, const AuId& atomId=AuId());

	//DHH Designer this should probably be changed.
	virtual int LowestDeck() const = 0;

	quint8 LowestSystemQuality() const;

	// Current maximum forward/reverse maneuver acceleration adjusted for damage.
	virtual float ManeuverAcceleration() const = 0;

	// Current maximum vertical maneuver acceleration adjusted for damage.
	inline float ManeuverAccelerationV() const
	{
		return ManeuverAccelerationV(ManeuverAcceleration());
	}

	// Maximum vertical maneuver acceleration based on maximum forward/reverse maneuver acceleration.
	static inline float ManeuverAccelerationV(float maneuverAccel)
	{
		return maneuverAccel * .08f;
	}

	// Returns the forward speed in ft/sec at which the hull stress hits 1 at sea level on a given planet.
	virtual float MaxAtmosphereSpeed(const DaWorld* planet) const = 0;

	// Returns the maximum throttle setting that will not exceed the hull stress in an atmosphere.
	virtual int MaxAtmosphereThrottle() const = 0;

	// Test current capacitor level to determine the highest warp factor that can be engaged.
	// Returns -1 if not enough power for even engage warp factor 0.
	virtual int MaxCapacitorWarpFactor() const = 0;

	// Returns the maximum transporter range.
	// 0 is returned if the ship has no transporters.
	// Returns 0 if the ship has no power.
	// Transporter rooms with no system hits remaining are ignored.
	// Transporter range is NominalTransporterRange * QualityBoost. At Q255, MaxTransporterRange is 21,101.5m, calculated 10/24/17.
	virtual float MaxTransporterRange() const = 0;

	inline void MissionAppend(const AuMission& mission)
	{
		m_MissionLock.lockForWrite();
		AuMissionOrder* prevOrder = m_Mission.CurrentOrder().Clone();
		m_Mission += mission;
		m_MissionLock.unlock();
		MissionOrderChanged(*prevOrder);
		delete prevOrder;
	}

	// Appends 'order' to the current ship's mission.
	inline void MissionAppendOrder(const AuMissionOrder& order)
	{
		m_MissionLock.lockForWrite();
		AuMissionOrder* prevOrder = m_Mission.CurrentOrder().Clone();
		m_Mission.Append(order);
		m_MissionLock.unlock();
		MissionOrderChanged(*prevOrder);
		delete prevOrder;
	}

	// Clear the mission.
	// Only humans call MissionClear. This mission modification does not notify a commanding pirate officer of the change, so they
	// do not immediately set up a new mission, which is annoying.
	inline void MissionClear()
	{
		m_MissionLock.lockForWrite();
		AuMissionOrder* prevOrder = m_Mission.CurrentOrder().Clone();
		m_Mission.Clear();
		m_MissionLock.unlock();
		MissionOrderChanged(*prevOrder, !IsPirateWithFreeWill());	// Prevent the pirate officer from setting up a new mission.
		delete prevOrder;
	}

	// Deletes 'orderId' from the current ship's mission.
	inline void MissionDeleteOrder(quint8 orderId)
	{
		m_MissionLock.lockForWrite();
		if(m_Mission.Contains(orderId))
		{
			// The order to delete was found in the current mission.
			AuMissionOrder* prevOrder = m_Mission.CurrentOrder().Clone();
			m_Mission.Delete(orderId);
			m_MissionLock.unlock();
			MissionOrderChanged(*prevOrder);
			delete prevOrder;
		}
		else
			m_MissionLock.unlock();
	}

	const DaAtom* MissionDestAttackFollow(const AuMissionOrder& order) const;

	const DaVessel* MissionDestGoToBestStation() const;

	const DaBd* MissionDestGoToCity(const AuMissionOrder& order) const;

	const DaBd* MissionDestGoToClosestCity() const;

	const DaVessel* MissionDestGoToClosestStation() const;

	// Returned atom could be a DaSc or a DaBd.
	const DaAtom* MissionDestGoToClosestStationOrCity() const;

	const DaAtom* MissionDestGoToCoordinate(const AuMissionOrder& order, AuPoint& destOffset) const;

	const DaGalaxy* MissionDestGoToGalaxy(const AuMissionOrder& order) const;

	const DaWorld* MissionDestGoToOrbitBestPlanet(AuPoint& destOffset) const;

	// Returns a DaStar* or a DaWorld*.
	const DaAtom* MissionDestGoToOrbitHoverLand(const AuMissionOrder& order, AuPoint& destOffset) const;

	const DaStar* MissionDestGoToStar(const AuMissionOrder& order, AuPoint& destOffset) const;

	const DaVessel* MissionDestGoToStation(const AuMissionOrder& order) const;

	const DaSolarSystem* MissionDestGoToSystem(const AuMissionOrder& order) const;

	// Return destination atom and offset based on ship's current mission order.
	// 'destOrder' is a return parameter that specifies the mission order type, which can vary from MissionOrderType.
	// 'destOffset' is a return parameter that specifies a destination location relative to the destination atom.
	const DaAtom* MissionDestination(AuMissionOrder::TYPE& destOrder, AuPoint& destOffset) const;

	// Remember which destination I last arrived at so MissionDestination can properly select the next destination.
	// Needed for city and station patrol orders.
	void MissionDestinationArrived(const AuId& destId)
	{
		m_MissionLock.lockForWrite();
		if(m_Mission.LastArrivalId() != destId)
		{
			AuMissionOrder* prevOrder = m_Mission.CurrentOrder().Clone();
			m_Mission.ArrivedAtDestination(destId);
			m_MissionLock.unlock();
			MissionOrderChanged(*prevOrder);
			delete prevOrder;
		}
		else
			m_MissionLock.unlock();
	}

	const DaBd* MissionDestPatrolCities() const;

	const DaWorld* MissionDestPatrolPlanets(AuPoint& destOffset) const;

	const DaVessel* MissionDestPatrolStations() const;

	const DaSolarSystem* MissionDestPatrolWormholes(AuPoint& destOffset) const;

	// Gets the mission harvest operation for 'whichBay'.
	// Return false if the current order is not a harvest by bay operation or if no operation is defined for 'whichBay'.
	bool MissionHarvestOperationByBay(quint16 whichBay, AuHarvestOp& op) const;

	// Gets the mission harvest operation for 'bayModule'.
	// Return false if the current order is not a harvest by module operation or if no operation is defined for 'bayModule'.
	bool MissionHarvestOperationByModule(COMMODITYID bayModule, AuHarvestOp& op) const;

	inline QString MissionHtmlReportTable() const
	{
		m_MissionLock.lockForRead();
		QString s = m_Mission.HtmlReportTable();
		m_MissionLock.unlock();
		return s;
	}

	// Inserts 'order' into the current ship's mission.
	// 'beforeId' is the id of the mission order before which the nu order is appended.
	inline void MissionInsertOrder(quint8 beforeId, const AuMissionOrder& order)
	{
		m_MissionLock.lockForWrite();
		AuMissionOrder* prevOrder = m_Mission.CurrentOrder().Clone();
		m_Mission.Insert(beforeId, order);
		m_MissionLock.unlock();
		MissionOrderChanged(*prevOrder);
		delete prevOrder;
	}

	inline bool MissionIsEmpty() const
	{
		m_MissionLock.lockForRead();
		bool is = m_Mission.IsEmpty();
		m_MissionLock.unlock();
		return is;
	}

	// Returns:
	//  0 = All harvest operations are satisfied.
	//  1 = Hold is full.
	//  2 = Current order is not a harvest operation.
	//  3 = Fuel is at or below the threshold.
	//  4 = Unsatisfied harvester operation exists.
	int MissionIsHarvestOperationComplete() const;

	inline AuId MissionLastArrivalId() const
	{
		m_MissionLock.lockForRead();
		AuId id = m_Mission.LastArrivalId();
		m_MissionLock.unlock();
		return id;
	}

	// Sets a nu mission.
	inline void MissionNew(const AuMission& mission)
	{
		m_MissionLock.lockForWrite();
		AuMissionOrder* prevOrder = m_Mission.CurrentOrder().Clone();
		m_Mission = mission;
		m_MissionLock.unlock();
		MissionOrderChanged(*prevOrder);
		delete prevOrder;
	}

	inline QString MissionNextSystem() const
	{
		m_MissionLock.lockForRead();
		QString i = m_Mission.NextSystem();
		m_MissionLock.unlock();
		return i;
	}

	inline quint16 MissionOrderAmount() const
	{
		m_MissionLock.lockForRead();
		quint16 i = m_Mission.CurrentOrder().Amount();
		m_MissionLock.unlock();
		return i;
	}

	inline double MissionOrderAmountOfMoney() const
	{
		m_MissionLock.lockForRead();
		double i = m_Mission.CurrentOrder().AmountOfMoney();
		m_MissionLock.unlock();
		return i;
	}

	inline quint8 MissionOrderAmountType() const
	{
		m_MissionLock.lockForRead();
		quint8 i = m_Mission.CurrentOrder().AmountType();
		m_MissionLock.unlock();
		return i;
	}

	inline int MissionOrderApplyMinQ() const
	{
		m_MissionLock.lockForRead();
		int i = m_Mission.CurrentOrder().ApplyMinQ();
		m_MissionLock.unlock();
		return i;
	}

	inline AuId MissionOrderAuthorization() const
	{
		m_MissionLock.lockForRead();
		AuId t = m_Mission.CurrentOrder().Authorization();
		m_MissionLock.unlock();
		return t;
	}

	inline CITIZEN_POSTURE MissionOrderCitizenPosture() const
	{
		m_MissionLock.lockForRead();
		CITIZEN_POSTURE i = m_Mission.CurrentOrder().CitizenPosture();
		m_MissionLock.unlock();
		return i;
	}

	inline COMMODITYFILTER MissionOrderCommodityFilter() const
	{
		m_MissionLock.lockForRead();
		COMMODITYFILTER i = m_Mission.CurrentOrder().CommodityFilter();
		m_MissionLock.unlock();
		return i;
	}

	inline COMMODITYID MissionOrderCommodityId() const
	{
		m_MissionLock.lockForRead();
		COMMODITYID i = m_Mission.CurrentOrder().CommodityId();
		m_MissionLock.unlock();
		return i;
	}

	inline AuId MissionOrderDestinationId() const
	{
		m_MissionLock.lockForRead();
		AuId id = m_Mission.CurrentOrder().DestinationId();
		m_MissionLock.unlock();
		return id;
	}

	inline QString MissionOrderDestinationName() const
	{
		m_MissionLock.lockForRead();
		QString name = m_Mission.CurrentOrder().DestinationName();
		m_MissionLock.unlock();
		return name;
	}

	inline QString MissionOrderHailIntentions() const
	{
		m_MissionLock.lockForRead();
		QString i = m_Mission.CurrentOrder().HailIntentions();
		m_MissionLock.unlock();
		return i;
	}

	inline quint32 MissionOrderHeadCount() const
	{
		m_MissionLock.lockForRead();
		quint32 i = m_Mission.CurrentOrder().HeadCount();
		m_MissionLock.unlock();
		return i;
	}

	inline bool MissionOrderHeadlightOn() const
	{
		m_MissionLock.lockForRead();
		bool i = m_Mission.CurrentOrder().HeadlightOn();
		m_MissionLock.unlock();
		return i;
	}

	inline bool MissionOrderIsExpired(const AuTime& orderStarted) const
	{
		m_MissionLock.lockForRead();
		bool is = m_Mission.CurrentOrder().IsExpired(orderStarted);
		m_MissionLock.unlock();
		return is;
	}

	inline bool MissionOrderIsPatrol() const
	{
		m_MissionLock.lockForRead();
		bool is = m_Mission.CurrentOrder().IsPatrol();
		m_MissionLock.unlock();
		return is;
	}

	inline bool MissionOrderIsStarTravelOrder() const
	{
		m_MissionLock.lockForRead();
		bool is = m_Mission.CurrentOrder().IsStarTravelOrder();
		m_MissionLock.unlock();
		return is;
	}

	inline bool MissionOrderIsTravelOrder() const
	{
		m_MissionLock.lockForRead();
		bool is = m_Mission.CurrentOrder().IsTravelOrder();
		m_MissionLock.unlock();
		return is;
	}

	inline QString MissionOrderListDescription() const
	{
		m_MissionLock.lockForRead();
		QString s = m_Mission.CurrentOrder().ListDescription();
		m_MissionLock.unlock();
		return s;
	}

	inline quint8 MissionOrderMinQ() const
	{
		m_MissionLock.lockForRead();
		quint8 i = m_Mission.CurrentOrder().MinQ();
		m_MissionLock.unlock();
		return i;
	}

	inline quint8 MissionOrderMissionOrderId() const
	{
		m_MissionLock.lockForRead();
		quint8 i = m_Mission.CurrentOrder().MissionOrderId();
		m_MissionLock.unlock();
		return i;
	}

	inline bool MissionOrderNavLightOn() const
	{
		m_MissionLock.lockForRead();
		bool i = m_Mission.CurrentOrder().NavLightOn();
		m_MissionLock.unlock();
		return i;
	}

	inline int MissionOrderOrderTime() const
	{
		m_MissionLock.lockForRead();
		int i = m_Mission.CurrentOrder().OrderTime();
		m_MissionLock.unlock();
		return i;
	}

	// Returns 0=officers, 1=crew, 2=troops, 3=passengers
	inline quint8 MissionOrderPeopleType() const
	{
		m_MissionLock.lockForRead();
		quint8 i = m_Mission.CurrentOrder().PeopleType();
		m_MissionLock.unlock();
		return i;
	}

	inline int MissionOrderPreboardingAnnouncementMinutes() const
	{
		m_MissionLock.lockForRead();
		int i = m_Mission.CurrentOrder().PreboardingAnnouncementMinutes();
		m_MissionLock.unlock();
		return i;
	}

	inline quint8 MissionOrderPriceMarginType() const
	{
		m_MissionLock.lockForRead();
		quint8 i = m_Mission.CurrentOrder().PriceMarginType();
		m_MissionLock.unlock();
		return i;
	}

	inline quint32 MissionOrderPriceMarginValue() const
	{
		m_MissionLock.lockForRead();
		quint32 i = m_Mission.CurrentOrder().PriceMarginValue();
		m_MissionLock.unlock();
		return i;
	}

	inline bool MissionOrderReportToCommand() const
	{
		m_MissionLock.lockForRead();
		bool is = m_Mission.CurrentOrder().ReportToCommand();
		m_MissionLock.unlock();
		return is;
	}

	inline bool MissionOrderRotatingBeaconOn() const
	{
		m_MissionLock.lockForRead();
		bool i = m_Mission.CurrentOrder().RotatingBeaconOn();
		m_MissionLock.unlock();
		return i;
	}

	inline QString MissionOrderShipName() const
	{
		m_MissionLock.lockForRead();
		QString i = m_Mission.CurrentOrder().ShipName();
		m_MissionLock.unlock();
		return i;
	}

	inline SHIELDFUEL_POLICY MissionOrderShipShieldFuelPolicy() const
	{
		m_MissionLock.lockForRead();
		SHIELDFUEL_POLICY i = m_Mission.CurrentOrder().ShipShieldFuelPolicy();
		m_MissionLock.unlock();
		return i;
	}

	inline SHIELD_POLICY MissionOrderShipShieldPolicy() const
	{
		m_MissionLock.lockForRead();
		SHIELD_POLICY i = m_Mission.CurrentOrder().ShipShieldPolicy();
		m_MissionLock.unlock();
		return i;
	}

	inline PILOT_UP_POLICY MissionOrderShipPilotUpPolicy() const
	{
		m_MissionLock.lockForRead();
		PILOT_UP_POLICY i = m_Mission.CurrentOrder().PilotUpPolicy();
		m_MissionLock.unlock();
		return i;
	}

	inline AuTime MissionOrderStart() const
	{
		m_MissionLock.lockForRead();
		AuTime i = m_Mission.OrderStart();
		m_MissionLock.unlock();
		return i;
	}

	inline bool MissionOrderStrobeOn() const
	{
		m_MissionLock.lockForRead();
		bool i = m_Mission.CurrentOrder().StrobeOn();
		m_MissionLock.unlock();
		return i;
	}

	inline AuId MissionOrderTarget() const
	{
		m_MissionLock.lockForRead();
		AuId t = m_Mission.CurrentOrder().TargetId();
		m_MissionLock.unlock();
		return t;
	}

	inline QString MissionOrderTargetName() const
	{
		m_MissionLock.lockForRead();
		QString t = m_Mission.CurrentOrder().TargetName();
		m_MissionLock.unlock();
		return t;
	}

	// 'officer' is the ships officer who is executing this order. It is used in the chain-of-command test on the target vessel.
	DaVessel* MissionOrderTransferTarget(const AuId& officer) const;

	virtual AuMissionOrder::TYPE MissionOrderType() const
	{
		m_MissionLock.lockForRead();
		AuMissionOrder::TYPE t = m_Mission.CurrentOrder().Type();
		m_MissionLock.unlock();
		return t;
	}

	inline QString MissionOrderVerbalization(const QString& pilot, const QString& navigator, const QString& engineer,
			const QString& power, const QString& sensor, const QString& shields) const
	{
		m_MissionLock.lockForRead();
		QString t = m_Mission.CurrentOrder().Verbalization(pilot, navigator, engineer, power, sensor, shields);
		m_MissionLock.unlock();
		return t;
	}

	// Called whenever the current mission order changes.
	// 'notifyOfficer' can be false to prevent pirate officers from immediately configuring a new mission.
	void MissionOrderChanged(const AuMissionOrder& prevOrder, bool notifyOfficer=true);

	// 'commandingOfficer' may be null, in which case there is no notification of the completed order.
	// 'completionCode' is used by the officer when announcing the result.
	void MissionOrderComplete(DaSentient* commandingOfficer, int completionCode);

	// Replaces 'order' in the current ship's mission, based on id.
	// 'orderId' is the id of the mission order that will be replaced by 'order'.
	// Returns true if 'orderId' is found and replaced.
	// Returns false if 'orderId' is not found. Nothing is changed in that case.
	inline bool MissionReplaceOrder(quint8 orderId, const AuMissionOrder& order)
	{
		m_MissionLock.lockForWrite();
		AuMissionOrder* prevOrder = m_Mission.CurrentOrder().Clone();
		bool didReplace = m_Mission.Replace(orderId, order);
		m_MissionLock.unlock();
		if(didReplace)
			MissionOrderChanged(*prevOrder);
		delete prevOrder;
		return didReplace;
	}

	// Set up the mission to scuttle the ship, with associated explosion, death and debris.
	inline void MissionScuttle(quint8 secsToWait)
	{
		m_MissionLock.lockForWrite();
		AuMissionOrder* prevOrder = m_Mission.CurrentOrder().Clone();
		m_Mission.Clear();
		m_Mission.Append(AuMissionOrderScuttle(secsToWait));
		m_Mission.SetActive(true);
		m_MissionLock.unlock();
		MissionOrderChanged(*prevOrder);
		delete prevOrder;
	}

	inline void MissionSetActive(bool isActive)
	{
		m_MissionLock.lockForWrite();
		AuMissionOrder* prevOrder = m_Mission.CurrentOrder().Clone();
		m_Mission.SetActive(isActive);
		m_MissionLock.unlock();
		MissionOrderChanged(*prevOrder);
		delete prevOrder;

		if(!isActive)
		{
			// When deactivating the mission, the crew clears the nav route and kills the throttle.
			if(IsPilotMannedByAI() && !IsPilotMannedByPerson())
				SetThrottle(0);			// Kill the throttle.
			if(HasWaypoint() && IsNavigatorMannedByAI() && !IsNavigatorMannedByPerson())
				RemoveAllWaypoints();		// Clear the current route.
		}
	}

	// 'missionType' is an AuMissionOrder::TYPE enum value.
	inline void MissionSetType(AuMission::TYPE missionType)
	{
		m_MissionLock.lockForRead();
		AuMissionOrder* prevOrder = m_Mission.CurrentOrder().Clone();
		m_Mission.SetType(missionType);
		m_MissionLock.unlock();
		MissionOrderChanged(*prevOrder);
		delete prevOrder;
	}

	inline AuMission::TYPE MissionType() const
	{
		m_MissionLock.lockForRead();
		AuMission::TYPE t = m_Mission.Type();
		m_MissionLock.unlock();
		return t;
	}

	// Monitor/refresh all channels this ship should be monitoring.
	void MonitorCommChannels();

	inline quint16 Monster() const
	{
		m_VesselLock.lockForRead();
		quint16 v = m_Monster;
		m_VesselLock.unlock();
		return v;
	}

	inline MONSTER_ORIGIN MonsterOrigin() const
	{
		m_VesselLock.lockForRead();
		MONSTER_ORIGIN v = m_MonsterOrigin;
		m_VesselLock.unlock();
		return v;
	}

	// Makes a random check to see if there is a monster encounter. Calls MonsterEncounterNow if an encounter happens.
	void MonsterEncounter();

	// Creates a monster encounter.
	// Returns the number of monsters spawned.
	int MonsterEncounterNow();

	// Can only set moorings when the ship is not moving and throttles are off and ship is in trade range of a station.
	void Moor();

	// 'byWho' is the person who is mothballing the ship.
	// 'hangarAtom' is a world or a terrain patch that will host the hangared ship atom. It must be a parent of 'byWho'.
	void Mothball(const DaPerson* byWho, DaAtom* hangarAtom);

	virtual AuPoint MovementBlocked(DaAtom* hit, const AuPoint& hitLoc, const AuPoint& hitNormal, bool hitShield);

	virtual quint64 MemoryUsage() const;

	inline int MunitionsWantedByStation() const
	{
		// Stations like to keep 10% of their hold full of munitions.
		int cap = CargoCapacityVolume();
		int munitionStockLvl = cap / 10;
		m_CargoLock.lockForRead();
		int munitions = m_Cargo.TotalMunitionUnits();
		m_CargoLock.unlock();
		if(munitions >= munitionStockLvl)
			return 0;	// No more food needed.
		return munitionStockLvl - munitions;
	}

	virtual QString Name() const;

	virtual QString NameWithEmpire() const;

	// Looks for the navigator, if there is one.
	// Returns a person on line who is at a nav console, preferably.
	// Returns a crewman at a nav console if no person on line is found at a nav console.
	DaSentient* Navigator() const;

	virtual void NavLightToggle(bool onState)
	{
		Q_UNUSED(onState);
	}

	// Returns actual radius of this object for avoidance when navigating and bay weapon fire obstruction.
	virtual double ObstacleRadius() const;

	// NOTE: This function assumes you have already insured there is a berth of the requested type available.
	virtual void OccupyBerth(AuRosterEntry::TYPE type, const AuId& occId, const QString& occName) = 0;

	// Returns the gross number of officer berths aboard this ship, even if one is allocated to the captain.
	virtual unsigned int OfficerBerths() const = 0;

	// Returns the number of berths available for occupants, considering that one might be allocated to the captain.
	inline unsigned int OfficerBerthsAvailable() const
	{
		// Determine the number of officer berths on the ship that are not allocated to the captain.
		unsigned int berths = OfficerBerths();
		if(!berths)
			return 0;	// This ship has no officer berths.
		if(CaptainsBerthType() == AuRosterEntry::OfficerHuman)
			berths--;	// One of the berths is allocated to the captain.
		if(!berths)
			return 0;	// This ship's only officer berth was allocated to the captain.

		// Check the roster for occupants.
		m_RosterLock.lockForRead();
		unsigned int occupants = m_Roster.BerthsOccupied(AuRosterEntry::OfficerAI) +
			m_Roster.BerthsOccupied(AuRosterEntry::OfficerHuman);
		m_RosterLock.unlock();

		if(!occupants)
			return berths;

		// Calculate the number of berths available.
		return berths - occupants;
	}

	// Returns the gross number of occupied officer berths aboard this ship, including any allocated to the captain.
	inline unsigned int OfficerBerthsOccupied() const
	{
		// If the captain stays in this cabin type, count his berth too.
		unsigned int v;
		if(CaptainsBerthType() == AuRosterEntry::OfficerHuman)
		{
			m_RosterLock.lockForRead();
			v = m_Roster.BerthsOccupied(AuRosterEntry::OfficerAI) + m_Roster.BerthsOccupied(AuRosterEntry::OfficerHuman) +
				m_Roster.BerthsOccupied(AuRosterEntry::Captain);
			m_RosterLock.unlock();
		}
		else
		{
			m_RosterLock.lockForRead();
			v = m_Roster.BerthsOccupied(AuRosterEntry::OfficerAI) + m_Roster.BerthsOccupied(AuRosterEntry::OfficerHuman);
			m_RosterLock.unlock();
		}
		return v;
	}

	// Orient and OrientationSpeed have no effect unless this function returns true.
	virtual bool OkToOrient() const;

	// If any AI aboard is enemy, the crewman cannot teleport to his station.
	// If any person aboard who is online is neutral or enemy, the crewman cannot teleport to his station.
	// If any wild animal is aboard, the crewman cannot teleport to his station.
	virtual bool OkToTeleportToStation(const DaSentient* crewman) const;

	// If this atom is "at rest", its rotation is changed immediately and true is returned.
	// If this atom is not "at rest", its m_OrientRotation is set and false is returned.
	// Always returns false if parent is a vehicle or an animal.
	virtual bool Orient(const AuPoint& atLoc, const AuPoint& terNormal);

	inline void OverclockSystemSupplier(int supplierIndex, bool bOverclock)
	{
		//Q_ASSERT(!g_Au.IsSceneClient());	// Scene client does not rx enough m_ssaShipSystems data to do this.
		m_SystemsLock.lockForWrite();
		if(supplierIndex >= 0 && supplierIndex < m_ssaShipSystems.count())
		{
			m_ssaShipSystems[supplierIndex].OverclockSupplier(bOverclock);
			UpdateSystemsPowerSystemsLocked();
		}
		m_SystemsLock.unlock();
	}

	// NOTE: Only applicable to privately owned ships.
	inline AuId OwnerId() const
	{
		m_RosterLock.lockForRead();
		AuId v(m_OwnerId);
		m_RosterLock.unlock();
		return v;
	}

	// NOTE: Only applicable to privately owned ships.
	inline QString OwnerName() const
	{
		m_RosterLock.lockForRead();
		QString v(m_OwnerName);
		m_RosterLock.unlock();
		return v;
	}

	// Locates an unobstructed spot within trade range to park 'parkShip'.
	// Returns false if no parking spot is found.
	bool ParkingSpotInRange(const DaVessel* parkShip, AuPoint& parkHere) const;

	// Returns the gross number of passenger berths aboard this ship, even if one is allocated to the captain.
	virtual unsigned int PassengerBerths() const = 0;

	// Returns the number of berths available for occupants, considering that one might be allocated to the captain.
	inline unsigned int PassengerBerthsAvailable() const
	{
		// Determine the number of passenger berths on the ship that are not allocated to the captain.
		unsigned int berths = PassengerBerths();
		if(!berths)
			return 0;	// This ship has no passenger berths.
		if(CaptainsBerthType() == AuRosterEntry::PassengerHuman)
			berths--;	// One of the berths is allocated to the captain.
		if(!berths)
			return 0;	// This ship's only passenger berth was allocated to the captain.

		// Check the roster for occupants.
		m_RosterLock.lockForRead();
		unsigned int occupants = m_Roster.BerthsOccupied(AuRosterEntry::PassengerAI) +
			m_Roster.BerthsOccupied(AuRosterEntry::PassengerHuman);
		m_RosterLock.unlock();
		if(!occupants)
			return berths;

		// Calculate the number of berths available.
		return berths - occupants;
	}

	// Returns the gross number of occupied passenger berths aboard this ship, including any allocated to the captain.
	// Colonists and Passengers occupy passenger berths.
	inline unsigned int PassengerBerthsOccupied() const
	{
		// If the captain stays in this cabin type, count his berth too.
		unsigned int v;
		if(CaptainsBerthType() == AuRosterEntry::PassengerHuman)
		{
			m_RosterLock.lockForRead();
			v = m_Roster.BerthsOccupied(AuRosterEntry::PassengerAI) + m_Roster.BerthsOccupied(AuRosterEntry::PassengerHuman) +
				m_Roster.BerthsOccupied(AuRosterEntry::Captain);
			m_RosterLock.unlock();
		}
		else
		{
			m_RosterLock.lockForRead();
			v = m_Roster.BerthsOccupied(AuRosterEntry::PassengerAI) + m_Roster.BerthsOccupied(AuRosterEntry::PassengerHuman);
			m_RosterLock.unlock();
		}
		return v;
	}

	void PerformFireControlConsoleScan(AuSensorBlipList& sensorBlips) const;

	virtual void PerformSensorConsoleScan(AuSensorBlipList& sensorBlips) const;

	virtual void PerformTransporterConsoleScan(AuSensorBlipList& sensorBlips, bool showAboard) const;

	// Looks for the pilot, if there is one.
	// Returns a person on line who is at a helm, preferably.
	// Returns a crewman at a helm if no person on line is found at a helm.
	DaSentient* Pilot() const;

	// Called by spacecraft to avoid objects.
	// Returns safe radius with a safety margin to avoid this object when navigating.
	// A return value of 0 means this atom is not avoided by spacecraft.
	virtual double PilotAvoidRadius() const;

	inline PILOT_UP_POLICY PilotUpPolicy() const
	{
		return m_PilotUpPolicy;
	}

	inline void PitchDown(unsigned char rotSpeed)
	{
		bool onG = IsOnGround();
		m_ControlLock.lockForWrite();
		m_wActions &= ~ACT_PitchMask;
		if(rotSpeed && !onG)
		{
			m_wActions |= ACT_PitchDown;
			m_byRotSpeedX = rotSpeed;
			m_tmLastPitchControl.restart();
			SetInMotion();
			if(!IsTicking())
				TickTock();
		}
		m_ControlLock.unlock();
	}

	inline void PitchDownTurnLeft(quint8 rangeValue)
	{
		m_ControlLock.lockForWrite();
		m_wActions &= ~(ACT_PitchMask|ACT_TurnMask);

		m_wActions |= ACT_PitchDown;
		m_byRotSpeedX = rangeValue;
		m_tmLastPitchControl.restart();

		m_wActions |= ACT_TurnLeft;
		m_byRotSpeedZ = rangeValue;
		m_tmLastTurnControl.restart();
		m_ControlLock.unlock();
	}

	inline void PitchDownTurnRight(quint8 rangeValue)
	{
		m_ControlLock.lockForWrite();
		m_wActions &= ~(ACT_PitchMask|ACT_TurnMask);

		m_wActions |= ACT_PitchDown;
		m_byRotSpeedX = rangeValue;
		m_tmLastPitchControl.restart();

		m_wActions |= ACT_TurnRight;
		m_byRotSpeedZ = rangeValue;
		m_tmLastTurnControl.restart();
		m_ControlLock.unlock();
	}

	void PitchStop()
	{
		m_ControlLock.lockForWrite();
		m_wActions &= ~ACT_PitchMask;
		m_byRotSpeedX = 0;
		m_ControlLock.unlock();
	}

	void PitchUp(unsigned char rotSpeed)
	{
		bool onG = IsOnGround();
		bool cgSub = IsCGSubmerged();
		double speed = ForwardSpeed();
		m_ControlLock.lockForWrite();
		m_wActions &= ~ACT_PitchMask;
		if(rotSpeed && (!onG || cgSub || speed > 50.*5280./3600.))
		{
			m_wActions |= ACT_PitchUp;
			m_byRotSpeedX = rotSpeed;
			m_tmLastPitchControl.restart();
			SetInMotion();
			if(!IsTicking())
				TickTock();
		}
		m_ControlLock.unlock();
	}

	inline void PitchUpTurnLeft(quint8 rangeValue)
	{
		m_ControlLock.lockForWrite();
		m_wActions &= ~(ACT_PitchMask|ACT_TurnMask);

		m_wActions |= ACT_PitchUp;
		m_byRotSpeedX = rangeValue;
		m_tmLastPitchControl.restart();

		m_wActions |= ACT_TurnLeft;
		m_byRotSpeedZ = rangeValue;
		m_tmLastTurnControl.restart();
		m_ControlLock.unlock();
	}

	inline void PitchUpTurnRight(quint8 rangeValue)
	{
		m_ControlLock.lockForWrite();
		m_wActions &= ~(ACT_PitchMask|ACT_TurnMask);

		m_wActions |= ACT_PitchUp;
		m_byRotSpeedX = rangeValue;
		m_tmLastPitchControl.restart();

		m_wActions |= ACT_TurnRight;
		m_byRotSpeedZ = rangeValue;
		m_tmLastTurnControl.restart();
		m_ControlLock.unlock();
	}

	virtual double PowerNeededToEngageFTLDrive(quint16 whichFTL, quint8 driveSetting) const = 0;

	// Looks for the power relay operator, if there is one.
	// Returns a person on line who is at a power relay console, preferably.
	// Returns a crewman at a power relay console if no person on line is found at a power relay console.
	DaSentient* PowerOp() const;

	void PowerPlantShuntOff(int shuntIndex);

	void PowerPlantShuntOn(int shuntIndex);

	inline int PowerPlantShuntsEnabled() const
	{
		return PowerPlantShuntsEnabled(m_PowerPlantShunts);
	}

	static inline int PowerPlantShuntsEnabled(quint8 shuntBits)
	{
		int shuntCount = 0;
		for(int shuntIndex=0; shuntIndex<8; shuntIndex++)
			if(shuntBits & (1<<shuntIndex))
				shuntCount++;
		return shuntCount;
	}

	inline void PowerPlantShuntToggle(int shuntIndex)
	{
		if(m_PowerPlantShunts & (1<<shuntIndex))
			PowerPlantShuntOff(shuntIndex);
		else
			PowerPlantShuntOn(shuntIndex);
	}

	inline char PowerPlantSkew() const
	{
		return m_chPowerPlantSkew;
	}

	inline void PowerPlantSkewChange(char chVal, unsigned char byDur)
	{
		const int skewMax = 16;								// Person can adjust throughout the range, regardless of fuel quality.
		m_chPowerPlantSkew += chVal;
		if(m_chPowerPlantSkew > skewMax)
			m_chPowerPlantSkew = skewMax;
		else if(m_chPowerPlantSkew < -skewMax)
			m_chPowerPlantSkew = -skewMax;
		m_byPowerPlantSkewDur = byDur;
	}

	void PowerPlantSkewWander();

	virtual float PowerPlantUnits() const = 0;

	// Test for power plant on and functioning.
	inline bool PowerPlantWorking() const
	{
		// Power plant must be on to work.
		if(!m_bPowerPlantOn)
			return false;

		// Must have some fuel for power plant to work.
		if(FuelQuantity() <= 0)
			return false;

		// Shunts always get the power plant working.
		if(m_PowerPlantShunts)
			return true;

		if(!IsDesignDataCacheValid())
			return true;	// Give the benefit of the doubt until the design is loaded.

		// 100% damage to power plant with no shunts active will make it stop working.
		if(!SystemHitsRemaining(SYS_PowerPlant))
			return false;

		// Everything looks good.
		return true;
	}

	inline AuSpecies RatSpecies() const
	{
		m_VesselLock.lockForRead();
		AuSpecies v(m_RatSpecies);
		m_VesselLock.unlock();
		return v;
	}

	// Called when a piece of food wants to decay that is in someone's scene.
	// 'loc' is the location of the food.
	void RatEncounter(const AuPoint& loc);

	virtual void Reactivate();

	// Checks to see if this ship is standing down.
	//  0 is returned if this ship is standing down.
	// Returns a bit mask of reasons why not.
	//  0x0001 = StandDown order is not the active mission order.
	//  0x0002 = Doors locked.
	//  0x0004 = Throttles on.
	//  0x0008 = Active movement actions.
	//  0x0010 = FTL drive active.
	//  0x0020 = Shields up
	//  0x0040 = Sensors locked
	//  0x0080 = Weapon bay fired
	quint16 ReasonNotStandingDown() const;

	inline void RechargeCapacitor()
	{
		//Q_ASSERT(IsDesignDataCacheValid());
		float capMax = CapacitorMax();
		m_VesselLock.lockForWrite();
		m_CapacitorLevel = capMax;
		m_VesselLock.unlock();
	}

	// Removes crew from the roster that cannot be found aboard the ship.
	// This function does nothing if this ship has human officers.
	void ReconcileCrew();

	// Removes passengers from the roster that cannot be found aboard the ship.
	// This function does nothing if this ship has human officers.
	void ReconcilePassengers();

	// Removes occupants from the roster that cannot be found aboard the ship.
	// This is similar to the code that reconciles the roster at the player/officer's request but it is not the same.
	// That code is in DtSceneServer::ReconcileRoster.
	// This function does nothing if this ship has human officers.
	void ReconcileRoster();

	// Removes troops from the roster that cannot be found aboard the ship.
	// This function does nothing if this ship has human officers.
	void ReconcileTroops();

	// Used to keep a warp drive going as long as there is power.
	// Caller should check CanReengageFTLNow first.
	virtual void ReengageFTLDrive(quint16 whichFTL) = 0;

	// All values must add up to the total hull volume. Values are in whole cubic meters.
	virtual void Refit(const AuRefitShip& refit, quint32 seconds)
	{
		Q_UNUSED(refit);
		Q_UNUSED(seconds);
	}

	virtual bool Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari);

	// Fills the fuel cells to the max and sets the current fuel quality.
	inline void Refuel(quint8 fuelQuality)
	{
		Q_ASSERT(fuelQuality);
		m_VesselLock.lockForWrite();
		m_fFuelQuantity = FuelCapacity();
		m_FuelQuality = fuelQuality;
		m_VesselLock.unlock();
	}

	virtual float RelativeAudioGain(const DaPerson* sceneAtom) const;

	// Reloads turret with current ordnance. Picks nu ordnance at random if current ordnance is gone.
	// Does nothing if current ordnance is full of ammo.
	// Returns true if the turret is loaded.
	virtual bool ReloadTurret(quint16 which);

	void RemoveAllWaypoints();

	// Returns species on success.
	// Returns empty string on failure.
	AuSpecies RemoveAnyCryoBerthOccupant();

	// Removes an occupant by id if they are occupying any berth.
	// I always find myself looking for this function using the wrong name ReleaseBerth, which doesn't exist. D.H.H. 01/12/2010
	inline void RemoveBerthOccupant(const AuId& id)
	{
		m_RosterLock.lockForWrite();
		if(m_Roster.RemoveOccupant(id))
		{
			m_tmRosterChange.Set();
			m_RosterLock.unlock();
			SetModified();
		}
		else
			m_RosterLock.unlock();
	}

	virtual bool RemoveCargo(const AuItem& iTake, quint32 howMany);

	bool RemoveCargoNoLock(const AuItem& iTake, quint32 howMany);

	// returns the quality of the component that is removed
	// returns 0 if the component can not be removed
	// 'inRoomType' is applied when removing parts from the hull, to prefer SYS_Hull, SYS_Fuel, or SYS_Hold.
	// 'status' is a return arg: 0=ok, 1=invalid 'sightedPart', 2=invalid component for system, 3=no more components to remove
	virtual quint8 RemoveComponent(ROOMTYPE inRoomType, quint16 sightedPart, COMMODITYID cid, AuSysId& sysIdAffected,
			int& status) = 0;

	// Removes one cryo traveller with 'dna'.
	// Returns true on success.
	bool RemoveCryoBerthOccupant(const AuSpecies& species);

	// Removes a unit of fuel. Used when servicing vehicles.
	virtual bool RemoveFuel(COMMODITYID fuelCID);

	void RemoveWaypoint(int sequence);

	// Render circular nav beacon for stations.
	void RenderStationNavBeacon(const AuMatrix4x4& projection, const AuMatrix4x4& model, const DaPerson* sceneAtom,
			float textBrightness, AuGLEffects& re);

	// This repair all fixes everything as if the materials were avaialable. Used by pirates.
	// Returns the number of seconds required.
	quint16 RepairAll();

	// Repair all damaged systems. Use whatever parts are needed from city's inventory.
	// This is used in response to a bona fide repair offer. The city insured it had what was needed before it made the repair
	// offer. If the stuff is missing now, too bad; we'll fake it and nobody will ever know.
	void RepairAll(DaBd* bd);

	// Repair all damaged systems, using whatever parts are available from city's inventory.
	// 'repairQ' is the minimum quality of parts to use.
	void RepairAllPossible(DaBd* bd, quint8 repairQ);
	void RepairAllPossible(DaBd* bd, const AuSysId& sysId, quint8 repairQ);

	void RepairAndResupplyStation();

	virtual void RepairCriticalSystems() = 0;

	// Repairs damage by applying parts from the cargo hold to the system identified by 'sightedPart'.
	// 'inRoomType' is applied when repairing the hull, to prefer SYS_Hull, SYS_Fuel, or SYS_Hold.
	// 'sightedPart' is 0 for the hull or 1..n for a detail id, as returned by SightTargetDetails.
	// 'fixCount' is the number to fix.
	// 'systemModuleChange' is an in/out arg. In permits|denies system module change. Out reports true if system module changed.
	// Returns the count actually fixed, if any damage is repaired. This implies that a part was consumed from the hold.
	quint32 RepairDamage(ROOMTYPE inRoomType, quint16 sightedPart, COMMODITYID cid, quint32 fixCount, AuSysId& sysIdRepaired,
			bool& systemModuleChange);

	// Repairs damage by applying 'sparePart' to the system identified by 'sightedPart'.
	// 'inRoomType' is applied when repairing the hull, to prefer SYS_Hull, SYS_Fuel, or SYS_Hold.
	// 'sightedPart' is 0 for the hull or 1..n for a detail id, as returned by SightTargetDetails.
	// 'fixCount' is the number to fix.
	// 'systemModuleChange' is an in/out arg. In permits|denies system module change. Out reports true if system module changed.
	// 'forceUpgrade' is set when called by an agent to force the module type of a system to change.
	// Returns the count actually fixed, if any damage is repaired. This implies that a part was consumed from the hold.
	quint32 RepairDamage(ROOMTYPE inRoomType, quint16 sightedPart, const AuItem& sparePart, quint32 fixCount,
			AuSysId& sysIdRepaired, bool& systemModuleChange, bool forceUpgrade);

	// Repairs 'fixCount' hit points of damage to a system.
	// Returns the number of hit points actually repaired.
	quint32 RepairHitPoints(quint16 sightedPart, quint32 fixCount, AuSysId& sysIdRepaired);

	// Returns the number of seconds in the current repair.
	quint16 RepairSecondsRemaining() const
	{
		m_VesselLock.lockForRead();
		quint16 v = m_RepairSecs;
		m_VesselLock.unlock();
		return v;
	}

	// Repair all damage to a certain system. Use whatever parts are needed from city's inventory.
	// This is used in response to a bona fide repair offer. The city insured it had what was needed before it made the repair
	// offer. If the stuff is missing now, too bad; we'll fake it and nobody will ever know.
	// 'repairQ' is the minimum quality of parts to use.
	void RepairSystem(DaBd* bd, const AuSysId& sysId, quint8 repairQ);

	// Repairs all damage to a certain system except for those components which contain the list in cidExceptions
	// 'repairQ' is the minimum quality of parts to use.
	void RepairSystemExcept(DaBd* bd, const AuSysId& sysId, QList<COMMODITYID> cidExceptions, quint8 repairQ);

	// 'inRoomType' is applied when repairing the hull, to prefer SYS_Hull, SYS_Fuel, or SYS_Hold.
	// 'fixCount' is the number to fix.
	// Returns the number actually fixed.
	// 'systemModuleChange' is an in/out arg. In permits|denies system module change. Out reports true if system module changed.
	// 'forceUpgrade' is set when called by an agent to force the module type of a system to change.
	virtual quint32 ReplaceComponent(ROOMTYPE inRoomType, unsigned int sightedPart, COMMODITYID cid, quint8 quality,
			unsigned int fixCount, AuSysId& sysIdRepaired, bool& systemModuleChange, bool forceUpgrade) = 0;

	void RequestBerth(const AuCommPacket& msg, AuRosterEntry::TYPE type, DaPerson* requestor);

	void RequestBerth(const AuCommPacket& msg, AuRosterEntry::TYPE type, const QString& reqName, const AuId& reqUnitId,
			quint32 reqEmpire);

	inline void ResetMonster()
	{
		m_VesselLock.lockForWrite();
		m_Monster = 0;
		m_VesselLock.unlock();
		SetModified(NOTIFY_Database);
	}

	inline void ResetMonsterOrigin()
	{
		m_VesselLock.lockForWrite();
		m_MonsterOrigin = MONSTER_Unknown;
		m_VesselLock.unlock();
		SetModified(NOTIFY_Database);
	}

	void ResetRepairTime()
	{
		m_VesselLock.lockForWrite();
		m_RepairSecs = 0;
		m_VesselLock.unlock();
	}

	// Selects a nearby solar system and goes there directly, without using wormholes.
	void Retreat();

	// 'role' is 0=none, 1=crew, 2=troop, 3=passenger
	// Returns the revived occupant, 0 on failure.
	DaNpc* ReviveCryoBerthOccupant(const AuMatrix4x4& where, const AuSpecies& species, quint8 role);

	// Splits a reward between all ship crew members that are aboard right now and on line.
	// This includes the captain, officers, crew, and troops in the roster.
	// The 'rewardXp' is split between crew members aboard, minimum 1 per dude if rewardXp is > 0.
	// 'unitKilled' may be 0.
	void RewardShip(int rewardXp, REWARDTYPE rewardType, DaUnit* unitKilled);

	inline void RollLeft(unsigned char rotSpeed)
	{
		bool onG = IsOnGround();
		m_ControlLock.lockForWrite();
		m_wActions &= ~ACT_RollMask;
		if(rotSpeed && !onG)
		{
			m_wActions |= ACT_RollLeft;
			m_byRotSpeedY = rotSpeed;
			m_tmLastRollControl.restart();
			SetInMotion();
			if(!IsTicking())
				TickTock();
		}
		m_ControlLock.unlock();
	}

	// Sets state values to cause this ship to roll, pitch and yaw by the angles specified.
	// Used by the AI to maneuver the ship.
	void RollPitchYaw(float rollAngle, float pitchAngle, float yawAngle);

	// Returns true if RollPitchYaw was called within a recent time period.
	inline AuElapsedTimer RollPitchYawTime() const
	{
		m_ControlLock.lockForRead();
		AuElapsedTimer v(m_tmLastRPYControl);
		m_ControlLock.unlock();
		return v;
	}

	inline void RollRight(unsigned char rotSpeed)
	{
		bool onG = IsOnGround();
		m_ControlLock.lockForWrite();
		m_wActions &= ~ACT_RollMask;
		if(rotSpeed && !onG)
		{
			m_wActions |= ACT_RollRight;
			m_byRotSpeedY = rotSpeed;
			m_tmLastRollControl.restart();
			SetInMotion();
			if(!IsTicking())
				TickTock();
		}
		m_ControlLock.unlock();
	}

	void RollStop()
	{
		m_ControlLock.lockForWrite();
		m_wActions &= ~ACT_RollMask;
		m_byRotSpeedY = 0;
		m_ControlLock.unlock();
	}

	virtual void RotatingBeaconToggle(bool onState)
	{
		Q_UNUSED(onState);
	}

	void RxBeamOver(const AuCommBeamOver* msg, DaPerson* person, POLITICAL_STANCE stanceToSender, AuCommStdChannel::RANGE range);

	void RxBeamVehicleAtom(const AuCommBeamVehicleAtom* msg, AuCommStdChannel::RANGE limitedRange);

	void RxBeamVehicleCoordinate(const AuCommBeamVehicleCoordinate* msg, AuCommStdChannel::RANGE limitedRange);

	void RxBeamVehiclePlanet(const AuCommBeamVehiclePlanet* msg, AuCommStdChannel::RANGE limitedRange);

	void RxBeamVehicleShip(const AuCommBeamVehicleShip* msg, DaPerson* person, AuCommStdChannel::RANGE limitedRange);

	void RxBeamVehicleTransporter(const AuCommBeamVehicleTransporter* msg, DaPerson* person, AuCommStdChannel::RANGE limitedRange);

	// Stations monitor the Hail channel so they can beam people aboard on request.
	virtual void RxChatMessage(const AuCommPacket* msg, quint32& countSupplied, bool firstInGroup=true);

	void RxCombine(const AuCommCombine* msg, AuCommStdChannel::RANGE limitedRange, const AuId& sectorId, DaPerson* person);

	void RxContactShip(const AuCommContactShip* msg);

	void RxHailRequest(const AuCommHailRequest* msg, DaPerson* person, AuCommStdChannel::RANGE limitedRange,
			const AuId& sectorId);

	void RxJettison(const AuCommJettison* msg, AuCommStdChannel::RANGE limitedRange, const AuId& sectorId, DaPerson* person);

	void RxMissionNew(const AuCommMissionNew* msg);

	void RxMissionOrderAdd(const AuCommMissionOrderAdd* msg);

	void RxMissionOrderInsert(const AuCommMissionOrderInsert* msg);

	void RxMissionOrderNew(const AuCommMissionOrderNew* msg);

	void RxMissionProperty(const AuCommMissionProperty* msg);

	void RxOpenTEU(const AuCommOpenTEU* msg, AuCommStdChannel::RANGE limitedRange, const AuId& sectorId, DaPerson* person);

	void RxRequestBerthAny(const AuCommRequestBerthAny* msg, DaPerson* person);

	void RxRequestBerthCaptain(const AuCommRequestBerthCaptain* msg, DaPerson* person);

	void RxRequestBerthCaptainSlot(const AuCommRequestBerthCaptainSlot* msg, DaPerson* person);

	void RxRequestBerthCrew(const AuCommRequestBerthCrew* msg, DaPerson* person);

	void RxRequestBerthOfficer(const AuCommRequestBerthOfficer* msg, DaPerson* person);

	void RxRequestBerthPassenger(const AuCommRequestBerthPassenger* msg, DaPerson* person);

	void RxRequestBerthTroop(const AuCommRequestBerthTroop* msg, DaPerson* person);

	void RxRequestInventory(const AuCommRequestInventory* msg, const AuId& sectorId, AuCommStdChannel::RANGE range,
			DaPerson* person);

	virtual void RxUpdate(QDataStream& fromServer, bool isFirstUpdate);

	// Scuttles this station (if it is a station) if conditions/rules warrant.
	// Returns true if this station is scuttled.
	bool ScuttleExcessStation();

	// Selects weapon to fire from bay. Picks nu ordnance at random if current ordnance is gone.
	// Does nothing if current ordnance is available.
	// Returns true if the weapon bay selection is changed.
	// Returns false if 'which' bay is not configured with a weapon bay module.
	virtual bool SelectBayWeapon(quint16 which);

	// Returns false if 'which' bay is not configured with a weapon bay module.
	bool SelectBayWeaponAmmunition(const AuItem& ammo, quint16 which);

	void SelfDestruct();

	// Transmits a message on the Friend channel at Galaxy range.
	// 'text' is the text of the message.
	void SendAlert(const QString& text);

	// Transmits a message on the crew channel at solar system range.
	// 'text' is the text of the message.
	// Does nothing if an EnvDamage message was sent too recently, to avoid spamming the player.
	void SendEnvDamage(const QString& text);

	// Transmits a message on the Friend channel at Galaxy range.
	// Stations also send the message on the Trade channel.
	// AI commanded spacecraft also send the message on the Hail channel.
	// 'msgType' selects the appropriate AuCommSOS-derived comm message type.
	//   0 = AuCommSOSManeuverDriveStation
	//   1 = AuCommSOSFuelStation
	//   2 = AuCommSOSManeuverDriveMarooned
	//   3 = AuCommSOSManeuverDriveAdrift
	//   4 = AuCommSOSFuelMarooned
	//   5 = AuCommSOSFuelShip
	//   6 = AuCommSOSAttack
	// 'msgText' and 'msgValue' are relevant to the comm message type.
	void SendSOS(int msgType, const QString& msgText, int msgValue);

	// Looks for the sensor operator, if there is one.
	// Returns a person on line who is at a sensor console, preferably.
	// Returns a crewman at a sensor console if no person on line is found at a sensor console.
	DaSentient* SensorOp() const;

	// Test for sensors on and functioning.
	virtual bool SensorsWorking() const
	{
		// Must be ON to work.
		if(!m_bSensorsOn)
			return false;

		// Capacitor must have some power for system to work.
		if(CapacitorLevel() <= 0)
			return false;
		
		if(!IsDesignDataCacheValid())
			return true;	// Give the benefit of the doubt until the design is loaded.

		// 100% damage to sensors will make them stop working.
		if(!GetPowerConsumption(SYS_Sensor))
			return false;

		// Everything looks good.
		return true;
	}

	inline void SetAccessPolicy(ACCESS_POLICY accessPolicy)
	{
		m_AccessPolicy = accessPolicy;
	}

	virtual void SetAlertCondition(ALERT_CONDITION alert)
	{
		Q_UNUSED(alert);
	}

	virtual void SetAtRest();

	inline void SetAutoFetchVehicles(bool autoFetch)
	{
		m_AutoFetchVehicles = autoFetch;
	}

	inline void SetBayWeaponPowerSetting(quint16 which, quint8 powerSetting)
	{
		m_BayLock.lockForWrite();
		unordered_map<quint16, AuBaySetting>::iterator bsi = m_BaySetting.find(which);
		if(bsi != m_BaySetting.end())
			(*bsi).second.m_Setting = powerSetting;
		else
		{
			AuBaySetting baySetting;
			baySetting.m_Setting = powerSetting;
			m_BaySetting.insert(pair<quint16, AuBaySetting>(which, baySetting));
		}
		m_BayLock.unlock();
	}

	inline void SetBayWeaponTarget(quint16 which, const AuId& targetId)
	{
		m_BayLock.lockForWrite();
		unordered_map<quint16, AuBaySetting>::iterator bsi = m_BaySetting.find(which);
		if(bsi != m_BaySetting.end())
			(*bsi).second.m_TargetId = targetId;
		else
		{
			AuBaySetting baySetting;
			baySetting.m_TargetId = targetId;
			m_BaySetting.insert(pair<quint16, AuBaySetting>(which, baySetting));
		}
		m_BayLock.unlock();
	}

	// 'byWho' just entered a console.
	virtual void SetConsoleEntered(const DaCreature* byWho);

	// Sets the posture of all officers, crew and troops who are aboard.
	void SetCrewPosture(CITIZEN_POSTURE posture);

	virtual void SetEmpireId(quint32 idEmpire)
	{
		if(Allegiance() != idEmpire)
		{
			DaCapsule::SetEmpireId(idEmpire);
			m_NameWithEmpireLock.lockForWrite();
			m_NameWithEmpire = "";
			m_NameWithEmpireLock.unlock();
		}
	}

	// Called by lower console when animating on the client.
	inline void SetETE(int ete) const
	{
		m_VesselLock.lockForWrite();
		m_ETE = ete;
		m_VesselLock.unlock();
	}

	virtual void SetFTLDriveSetting(quint16 whichFTL, quint8 setting) = 0;

	inline void SetGravityDriveState(GRAVITY_DRIVE_STATE nuState)
	{
		// Can't turn gravity drive on while getting repairs.
		if(RepairSecondsRemaining())
			return;

		m_VesselLock.lockForWrite();
		if(m_eGravityDriveState != nuState && HasManeuverDrive())
		{
			m_eGravityDriveState = nuState;
			m_VesselLock.unlock();

			if(IsNew())
				SetNotNew();	// Ship is no longer "nu".
			SetModified();
		}
		else
			m_VesselLock.unlock();
	}

	inline void SetInEnemyRange() const
	{
		m_LastInRangeLock.lockForWrite();
		m_LastInEnemyRange.restart();
		m_LastInRangeLock.unlock();
	}

	inline void SetInTow(bool inTow)
	{
		if(m_InTow != inTow)
		{
			m_InTow = inTow;

			if(m_InTow && IsNew())
				SetNotNew();	// This ship is no longer nu.

			if(!IsTicking())
				TickTock();
		}
	}

	virtual void SetName(const QString& name);

	virtual void SetNotNew() = 0;

	inline void SetPilotUpPolicy(PILOT_UP_POLICY pilotUpPolicy)
	{
		m_PilotUpPolicy = pilotUpPolicy;
	}

	inline void SetPrivateOwner(const AuId& ownerId, const QString& ownerName)
	{
		m_RosterLock.lockForWrite();
		m_OwnerId = ownerId;
		m_OwnerName = ownerName;
		m_tmRosterChange.Set();
		m_RosterLock.unlock();
	}

	inline void SetPrivateOwnership(bool isPrivate)
	{
		m_IsPrivate = isPrivate;
	}

	inline void SetShieldFuelPolicy(SHIELDFUEL_POLICY shieldFuelPolicy)
	{
		m_ShieldFuelPolicy = shieldFuelPolicy;
	}

	inline void SetShieldUpPolicy(SHIELD_POLICY shieldPolicy)
	{
		m_ShieldPolicy = shieldPolicy;
	}

	virtual void SetSystemQuality(const AuSysId& sysIdRepaired, quint8 quality);

	inline void SetThrottle(qint8 setting)
	{
		// Do not permit throttle adjustments while moored. Throttle was required to be off to set moorings.
		if(setting == m_Throttle)
			return;
		if(setting < -10)
			return;
		if(setting > 10)
			return;
		m_ControlLock.lockForWrite();
		m_Throttle = setting;
		m_tmLastThrottleControl.restart();
		m_ControlLock.unlock();
		if(setting)
		{
			SetInMotion();
			if(!IsTicking())
				TickTock();
		}
		SetModified();
	}

	inline void SetThrustLateral(qint8 setting)
	{
		// Do not permit throttle adjustments while moored. Throttle was required to be off to set moorings.
		if(setting == m_ThrustLateral)
			return;
		if(setting < -5)
			return;
		if(setting > 5)
			return;
		m_ControlLock.lockForWrite();
		m_ThrustLateral = setting;
		m_tmLastThrottleLControl.restart();
		m_ControlLock.unlock();
		if(setting)
		{
			SetInMotion();
			if(!IsTicking())
				TickTock();
		}
		SetModified();
	}

	inline void SetThrustLongitudinal(qint8 setting)
	{
		// Do not permit throttle adjustments while moored. Throttle was required to be off to set moorings.
		if(setting == m_ThrustLong)
			return;
		if(setting < -10)
			setting = -10;
		else if(setting > 10)
			setting = 10;
		m_ControlLock.lockForWrite();
		m_ThrustLong = setting;
		m_tmLastThrottleFControl.restart();
		m_ControlLock.unlock();
		if(setting)
		{
			SetInMotion();
			if(!IsTicking())
				TickTock();
		}
		SetModified();
	}

	inline void SetThrustVertical(qint8 setting)
	{
		// Do not permit throttle adjustments while moored. Throttle was required to be off to set moorings.
		if(setting == m_ThrustVertical)
			return;
		if(setting < -7)
			setting = -7;
		else if(setting > 7)
			setting = 7;
		m_ControlLock.lockForWrite();
		m_ThrustVertical = setting;
		m_tmLastThrottleVControl.restart();
		m_ControlLock.unlock();
		if(setting)
		{
			SetInMotion();
			if(!IsTicking())
				TickTock();
		}
		SetModified();
	}

	virtual void SetupArena(const QString& arenaName, quint8 team, quint8 shipQ, bool spawnOfficer, bool spawnCrew,
			bool spawnTroops, bool spawnPassengers, bool spawnVehicles) = 0;

	virtual void SetupPirate(quint32 empireId, quint8 quality) = 0;

	// 'whereMade' provides a brief description of where this ship was made.
	// e.g. 'Imperium city Mortox on Hazeron Alpha II in Hazeron, Hazeron Sector'
	//      'Seledon world Hazeron Alpha IIa in Hazeron, Hazeron Sector'
	void SetWhereMade(const QString& whereMade)
	{
		m_VesselLock.lockForWrite();
		m_WhereMade = whereMade;
		m_VesselLock.unlock();
	}

	// Spread shield energy to be the same on all sides.
	inline void ShieldBalance()
	{
		m_ShieldsLock.lockForWrite();
		float powerAll = m_ShieldCap[0] + m_ShieldCap[1] + m_ShieldCap[2] + m_ShieldCap[3] + m_ShieldCap[4] + m_ShieldCap[5];
		float powerEach = powerAll / 6;
		m_ShieldCap[0] = powerEach;
		m_ShieldCap[1] = powerEach;
		m_ShieldCap[2] = powerEach;
		m_ShieldCap[3] = powerEach;
		m_ShieldCap[4] = powerEach;
		m_ShieldCap[5] = powerEach;
		m_ShieldsLock.unlock();
		SetModified();
	}

	// Returns the energy in a given shield's capacitor.
	// Return value will be 0..ShieldUnitsWorking().
	inline double ShieldCapacitor(SHIELD shield) const
	{
		m_ShieldsLock.lockForRead();
		double v = m_ShieldCap[shield];
		m_ShieldsLock.unlock();
		return v;
	}

	// Increase shield power uniformly using power available.
	// Returns the actual amount of power used.
	inline double ShieldCharge(float powerAvailable)
	{
		// Calculate power needed to fill all shields to the max.
		double powerMax = ShieldUnitsWorking();
		double powerNeed = 0;
		m_ShieldsLock.lockForRead();
		for(int is=0; is<6; is++)
			if(m_ShieldCap[is] < powerMax)
				powerNeed += powerMax - m_ShieldCap[is];	// Accumulate amount needed by the shield.
			else if(m_ShieldCap[is] > powerMax)
				m_ShieldCap[is] = powerMax;					// Limit to max, which may have dropped due to system damage.
		m_ShieldsLock.unlock();
		if(powerNeed <= 0)
			return 0;	// No power needed by shields.

		// Limit the power taken to the power available.
		double powerTake;
		if(powerNeed > powerAvailable)
			powerTake = powerAvailable;
		else
			powerTake = powerNeed;

		// Disburse the power taken evenly between the shields that need power.
		m_ShieldsLock.lockForWrite();
		for(int is=0; is<6; is++)
			if(m_ShieldCap[is] < powerMax)
			{
				double powerFrac = (powerMax - m_ShieldCap[is]) / powerNeed;// Fraction of this shield's portion of power needed.
				m_ShieldCap[is] += powerFrac * powerTake;					// Take that fraction of the power taken.
			}
		m_ShieldsLock.unlock();

		// Return the actual amount of power taken.
		return powerTake;
	}

	SHIELDFUEL_POLICY ShieldFuelPolicy() const
	{
		return m_ShieldFuelPolicy;
	}

	// Looks for the shield operator, if there is one.
	// Returns a person on line who is at a shield console, preferably.
	// Returns a crewman at a shield console if no person on line is found at a shield console.
	DaSentient* ShieldOp() const;

	// Returns x, y, and z radii of shield bubble ellipsoid, even if this ship has no shields.
	virtual AuPoint ShieldRadii() const = 0;

	inline SHIELD_POLICY ShieldUpPolicy() const
	{
		return m_ShieldPolicy;
	}

	virtual bool ShieldsBlockTransportAboard() const
	{
		// Shields only block transporters if they are on and working.
		if(!ShieldsWorkingAndUp())
			return false;
		// Shields block transporters if every side has more than 2 units of energy.
		m_ShieldsLock.lockForRead();
		bool v = m_ShieldCap[0] > 2.f && !m_ShieldScram[0] &&
				m_ShieldCap[1] > 2.f && !m_ShieldScram[1] &&
				m_ShieldCap[2] > 2.f && !m_ShieldScram[2] &&
				m_ShieldCap[3] > 2.f && !m_ShieldScram[3] &&
				m_ShieldCap[4] > 2.f && !m_ShieldScram[4] &&
				m_ShieldCap[5] > 2.f && !m_ShieldScram[5];
		m_ShieldsLock.unlock();
		return v;
	}

	virtual bool ShieldsBlockWeapons() const
	{
		// Shields only block transporters if they are on and working.
		if(!ShieldsWorkingAndUp())
			return false;
		// Shields could block weapons if any side has more than 2 units of energy.
		m_ShieldsLock.lockForRead();
		bool v = (m_ShieldCap[0] > 2.f && !m_ShieldScram[0]) ||
				(m_ShieldCap[1] > 2.f && !m_ShieldScram[1]) ||
				(m_ShieldCap[2] > 2.f && !m_ShieldScram[2]) ||
				(m_ShieldCap[3] > 2.f && !m_ShieldScram[3]) ||
				(m_ShieldCap[4] > 2.f && !m_ShieldScram[4]) ||
				(m_ShieldCap[5] > 2.f && !m_ShieldScram[5]);
		m_ShieldsLock.unlock();
		return v;
	}

	// Returns the number of seconds remaining before shield is unscrambled.
	inline unsigned int ShieldScram(SHIELD shield) const
	{
		m_ShieldsLock.lockForRead();
		unsigned int v = m_ShieldScram[shield];
		m_ShieldsLock.unlock();
		return v;
	}

	// Returns true if any shield side is scrambled.
	inline bool ShieldScrambled() const
	{
		m_ShieldsLock.lockForRead();
		bool v = m_ShieldScram[0] ||
			m_ShieldScram[1] ||
			m_ShieldScram[2] ||
			m_ShieldScram[3] ||
			m_ShieldScram[4] ||
			m_ShieldScram[5];
		m_ShieldsLock.unlock();
		return v;
	}

	// One-second proc to reduce shield scram values.
	void ShieldStabilize();

	// Ship fuel status, shield policy and sensor scan information are used to determine whether the shields should be up.
	// 'why'
	//   0=Low fuel.
	//   1=Shields inop.
	//   2=policy says up
	//   3=policy says down
	//   4=no atom environment
	//   5=no threat detected
	//   6=threat detected
	// 'threatName' is set to the atom Name() if 'why' is 6.
	SHIELD_STATE ShieldStateDesired(int& why, QString& threatName) const;

	virtual bool ShieldsWorkingAndOn() const;

	virtual bool ShieldsWorkingAndUp() const;

	// Returns the total number of shield units in the ship's design.
	virtual double ShieldUnits() const = 0;

	// Returns the number of shield units functioning, adjusted for damage.
	double ShieldUnitsWorking() const;

	// Adjust shield balance to put all power on a particular shield.
	inline void ShieldFull(SHIELD shield)
	{
		double powerNeeded = ShieldUnitsWorking() - ShieldCapacitor(shield);
		if(powerNeeded <= 0)
			return;	// No power needed.

		// Calculate total power that can be drawn from other shields.
		double powerRem = 0;
		m_ShieldsLock.lockForRead();
		for(int is=0; is<6; is++)
			if(is != shield)
				powerRem += m_ShieldCap[is];
		m_ShieldsLock.unlock();
		if(powerRem <= 0)
			return;	// No power to draw from.

		m_ShieldsLock.lockForWrite();
		if(powerRem <= powerNeeded)
		{
			// Take all remaining power from other shields.
			m_ShieldCap[shield] += powerRem;
			for(int is=0; is<6; is++)
				if(is != shield)
					m_ShieldCap[is] = 0;
		}
		else
		{
			// Take a fraction of each other shield's power to fill the shield.
			m_ShieldCap[shield] += powerNeeded;
			for(int is=0; is<6; is++)
				if(is != shield)
				{
					double powerFrac = m_ShieldCap[is] / powerRem;	// Fraction of this shield's portion of power remaining.
					m_ShieldCap[is] -= powerFrac * powerNeeded;		// Contribute that fraction of the power needed.
				}
		}
		m_ShieldsLock.unlock();
		SetModified();
	}

	// Increase shield power to a particular shield by 10%.
	inline void ShieldIncrease(SHIELD shield)
	{
		double powerMax = ShieldUnitsWorking();
		double powerNeeded = powerMax - ShieldCapacitor(shield);
		if(powerNeeded <= 0)
			return;	// No power needed.

		// Calculate total power that can be drawn from other shields.
		double powerRem = 0;
		m_ShieldsLock.lockForRead();
		for(int is=0; is<6; is++)
			if(is != shield)
				powerRem += m_ShieldCap[is];
		m_ShieldsLock.unlock();
		if(powerRem <= 0)
			return;	// No power to draw from.

		// Limit increase of the shield to 10% of the max.
		double powerChange = powerMax / 10;
		if(powerNeeded > powerChange)
			powerNeeded = powerChange;

		m_ShieldsLock.lockForWrite();
		if(powerRem <= powerNeeded)
		{
			// Take all remaining power from other shields.
			m_ShieldCap[shield] += powerRem;
			for(int is=0; is<6; is++)
				if(is != shield)
					m_ShieldCap[is] = 0;
		}
		else
		{
			// Take a fraction of each other shield's power to fill the shield.
			m_ShieldCap[shield] += powerNeeded;
			for(int is=0; is<6; is++)
				if(is != shield)
				{
					double powerFrac = m_ShieldCap[is] / powerRem;	// Fraction of this shield's portion of power remaining.
					m_ShieldCap[is] -= powerFrac * powerNeeded;		// Contribute that fraction of the power needed.
				}
		}
		m_ShieldsLock.unlock();
		SetModified();
	}

	// Decrease shield power to a particular shield by 10%.
	inline void ShieldDecrease(SHIELD shield)
	{
		// Nothing to do if the shield is aleady at zero.
		if(ShieldCapacitor(shield) <= 0)
			return;

		// Calculate amount of power that can be moved to other shields.
		double powerMax = ShieldUnitsWorking();
		double powerNeed = 0;
		m_ShieldsLock.lockForRead();
		for(int is=0; is<6; is++)
			if(is != shield && m_ShieldCap[is] < powerMax)
				powerNeed += powerMax - m_ShieldCap[is];
		m_ShieldsLock.unlock();
		if(powerNeed <= 0)
			return;	// No power needed by other shields.

		// Limit the decrease of the shield to 10% of the max.
		double powerChange = powerMax / 10;
		double powerTake;
		if(powerNeed > powerChange)
			powerTake = powerChange;
		else
			powerTake = powerNeed;

		m_ShieldsLock.lockForWrite();
		if(m_ShieldCap[shield] <= powerTake)
		{
			// Take all that is left of the shield.
			powerTake = m_ShieldCap[shield];
			m_ShieldCap[shield] = 0;
		}
		else
		{
			// Take power from the shield.
			m_ShieldCap[shield] -= powerTake;
		}

		// Disburse the power taken evenly between the shields that need power.
		for(int is=0; is<6; is++)
			if(is != shield && m_ShieldCap[is] < powerMax)
			{
				double powerFrac = (powerMax - m_ShieldCap[is]) / powerNeed;// Fraction of this shield's portion of power needed.
				m_ShieldCap[is] += powerFrac * powerTake;					// Take that fraction of the power taken.
			}
		m_ShieldsLock.unlock();
		SetModified();
	}

	// Adjust shield balance to take all power away from a particular shield.
	inline void ShieldOff(SHIELD shield)
	{
		// Nothing to do if the shield is aleady at zero.
		if(ShieldCapacitor(shield) <= 0)
			return;

		// Calculate amount of power that can be moved to other shields.
		double powerMax = ShieldUnitsWorking();
		double powerNeed = 0;
		m_ShieldsLock.lockForRead();
		for(int is=0; is<6; is++)
			if(is != shield && m_ShieldCap[is] < powerMax)
				powerNeed += powerMax - m_ShieldCap[is];
		m_ShieldsLock.unlock();
		if(powerNeed <= 0)
			return;	// No power needed by other shields.

		double powerTake;
		m_ShieldsLock.lockForWrite();
		if(m_ShieldCap[shield] <= powerNeed)
		{
			// Take all that is left of the shield.
			powerTake = m_ShieldCap[shield];
			m_ShieldCap[shield] = 0;
		}
		else
		{
			// Take power from the shield.
			powerTake = powerNeed;
			m_ShieldCap[shield] -= powerNeed;
		}

		// Disburse the power taken evenly between the shields that need power.
		for(int is=0; is<6; is++)
			if(is != shield && m_ShieldCap[is] < powerMax)
			{
				double powerFrac = (powerMax - m_ShieldCap[is]) / powerNeed;// Fraction of this shield's portion of power needed.
				m_ShieldCap[is] += powerFrac * powerTake;					// Take that fraction of the power taken.
			}
		m_ShieldsLock.unlock();
		SetModified();
	}

	// Returns the distance at which this ship appears in a scene. Also the distance at which AI can see this ship with their eyes.
	inline static double SightRange2()
	{
		return 32000*FeetPerMeter() * 32000*FeetPerMeter();	// about 20 miles
	}

	// Spawns one single citizen if any is needed. Only one officer is spawned, regardless of berths. Others fill all empty berths.
	// The order of precedence is the order of the parameters.
	void SpawnArenaCrewman(bool spawnOfficer, bool spawnCrew, bool spawnTroops, bool spawnPassengers);

	// Spawns one single vehicle if any is needed. Checks environment for contingent of vehicles.
	virtual void SpawnArenaVehicle() = 0;

	// Creates a troop using the city's worker species.
	// Returns true if a troop is spawned.
	bool SpawnTroop(DaBd* bd);

	// Breaks this ship free from AtRest state when appropriate.
	// Returns true if I am set in motion.
	virtual bool StartMoving();

	// Returns the time this ship will be ready to restore from stasis. Assumes caller has already checked to see if this ship is 
	// in stasis.
	virtual AuTime StasisReadyTime() const = 0;

	// Parking range for stations.
	float StationParkingRange() const;

	// Parking range for 'otherShip' when it is moving to this station.
	float StationParkingRange(const DaVessel* otherShip) const;

	// Trade range for stations.
	float StationTradeRange() const;

	// Trade range for stations.
	float StationTradeRange(const DaVessel* otherShip) const;

	virtual void StrobeToggle(bool onState)
	{
		Q_UNUSED(onState);
	}

	// Returns a number from 0 to 1, which is the ratio of current structural hit points to design structural hit points.
	float StructuralIntegrity() const;

	virtual SPACECRAFT_STRUCTURE Structure() const = 0;

	inline int SuppliesWantedByStation() const
	{
		// Stations like to keep 20% of their hold full of spare parts.
		int cap = CargoCapacityVolume();
		int itemStockLvl = cap / 25;
		m_CargoLock.lockForRead();
		int items = m_Cargo.TotalSupplyUnits();
		m_CargoLock.unlock();
		if(items >= itemStockLvl)
			return 0;	// No more parts needed.
		return itemStockLvl - items;
	}

	void SystemConsumerSelection(int iConsumerIndex, int iSupplierIndex);

	// Returns the number of 'cidSparePart' components that can be removed from 'sysId' when the system is at full health.
	// 'cidSparePart' is expected to be one of DbCommodity::IsSparePart.
	quint32 SystemComponents(const AuSysId& sysId, COMMODITYID cidSparePart) const;

	// 'which' identifies a particular subsystem
	inline quint32 SystemHitsRemaining(const AuSysId& sysId) const
	{
		m_SystemsLock.lockForRead();
		quint32 v = SystemHitsRemainingSystemsLocked(sysId);
		m_SystemsLock.unlock();
		return v;
	}

	virtual quint32 SystemHitsRemainingSystemsLocked(const AuSysId& sysId) const = 0;

	// Retrieves a list of all system ids, into 'sysIds'.
	virtual void SystemIds(QSet<AuSysId>& sysIds, bool structural) const = 0;

	inline COMMODITYID SystemModule(const AuSysId& sysId) const
	{
		if(sysId.Which())
			return SystemModule(sysId.System(), sysId.Which());
		else
			return SystemModule(sysId.System());
	}

	// Current module of a system.
	inline COMMODITYID SystemModule(SYSTEM system) const
	{
		// Scene client does rx enough m_ssaShipSystems data to do this.
		m_SystemsLock.lockForRead();
		COMMODITYID v = SystemModuleSystemsLocked(system);
		m_SystemsLock.unlock();
		return v;
	}

	inline COMMODITYID SystemModuleSystemsLocked(SYSTEM system) const
	{
		// Scene client does rx enough m_ssaShipSystems data to do this.
		return m_ssaShipSystems.SystemModule(system);
	}

	// Current module of a system.
	inline COMMODITYID SystemModule(SYSTEM system, quint16 which) const
	{
		// Scene client does rx enough m_ssaShipSystems data to do this.
		m_SystemsLock.lockForRead();
		COMMODITYID v = SystemModuleSystemsLocked(system, which);
		m_SystemsLock.unlock();
		return v;
	}

	inline COMMODITYID SystemModuleSystemsLocked(SYSTEM system, quint16 which) const
	{
		// Scene client does rx enough m_ssaShipSystems data to do this.
		return m_ssaShipSystems.SystemModule(system, which);
	}

	// Returns the number of system modules needed to upgrade a system.
	virtual quint32 SystemModuleCount(const AuSysId& sysId) const = 0;

	// Returns the time required to install each module, in seconds, when upgrading a system.
	// Returns 0 if the module cannot be installed in the system.
	// Returns -1 if the module cannot be installed into 'which' because of system duplication. i.e. FTL drive module upgrades must
	//   not result in multiples of the same kind of FTL drive. Each different 'which' must be a different kind of FTL drive.
	int SystemModuleInstallTime(const AuSysId& sysId, COMMODITYID cid) const;

	// Current quality of a system.
	// Initially quality at which the ship was manufactured. This may be greater than the quality of the design.
	virtual quint8 SystemQuality(SYSTEM system) const
	{
		// Scene client does rx enough m_ssaShipSystems data to do this.
		m_SystemsLock.lockForRead();
		quint8 v = m_ssaShipSystems.SystemQuality(system);
		m_SystemsLock.unlock();
		return v;
	}

	// Current quality of a system.
	// Initially quality at which the ship was manufactured. This may be greater than the quality of the design.
	virtual quint8 SystemQuality(SYSTEM system, quint16 which) const
	{
		// Scene client does rx enough m_ssaShipSystems data to do this.
		m_SystemsLock.lockForRead();
		quint8 v = m_ssaShipSystems.SystemQuality(system, which);
		m_SystemsLock.unlock();
		return v;
	}

	bool SystemSupplierSelection(int iIndex);

	// Fills 'whiches' with all which ids for a given 'system'.
	// 'whiches' is emptied if the system is not subdivided into sub systems.
	// Returns true on success.
	// Returns false if this vessel does not have any 'system'.
	bool SystemWhiches(SYSTEM system, QSet<quint16>& whiches) const;

	inline void TakeFuel(double amount)
	{
		m_VesselLock.lockForWrite();
		m_fFuelQuantity -= amount;
		if(m_fFuelQuantity < 0)
			m_fFuelQuantity = 0;
		m_VesselLock.unlock();
	}

	// Removes one units of 'item' from my cargo to load into a vehicle belonging to 'empireId'.
	// Checks empire export policy toward 'empireId'.
	// Returns true if it was taken, false if not.
	bool TakeOrdnance(const AuItem& item, quint32 empireId);

	// Returns the quality at which this ship was constructed. Any or all systems may have been upgraded since then.
	inline quint8 QualityMade() const
	{
		return m_QMade;
	}

	// Quality of the ship's design. This may be less than the actual quality at which the ship is manufactured.
	virtual quint8 QualityOfDesign() const = 0;

	// Returns the deep space sensor range in pc.
	float SensorRangeDeepSpace() const;

	// Called during Tock to test for damage due to power system overrides and shunts.
	// Returns true if the ship is destroyed.
	virtual bool TestPowerSystem() = 0;

	inline qint8 Throttle() const
	{
		return m_Throttle;
	}

	virtual void TestHullStress();

	// Returns current throttle value for lateral thrust.
	inline qint8 ThrustLateral() const
	{
		return m_ThrustLateral;
	}

	// Returns current throttle value for longitudinal thrust.
	inline int ThrustLongitudinal() const
	{
		// Momemtary longitudinal thrust overrides the throttle setting.
		if(m_ThrustLong)
			return m_ThrustLong;
		return m_Throttle;
	}

	// Returns current throttle value for vertical thrust.
	inline qint8 ThrustVertical() const
	{
		return m_ThrustVertical;
	}

	virtual bool IsTicker() const;

	virtual void Tick(const AuElapsedTimer& last, const AuElapsedTimer& now);

	virtual void TickMovement(const AuElapsedTimer& last);

	virtual unsigned int IsTocker() const;

	virtual void Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans);

	// Toggle state of nearest large hull door.
	virtual void ToggleHullDoor(int doorIndex);

	virtual float TractorBeamRange(float unitsFired, quint16 which) const = 0;

	void TransferColonists(DaVessel* fromShip, int howMany);

	void TransferCryo(DaVessel* fromShip, int howMany);

	// Range at which this ship can transfer cargo to|from another ship, based on the maximum capabilities of each vessel.
	inline float TransferRange(const DaVessel* otherShip) const
	{
		// Calculate range for physical cargo transfer.
		Q_ASSERT(otherShip);
		float transferRange = RoughRadius() + 100*FeetPerMeter() + otherShip->RoughRadius();

		// Stations have a service area determined by the quality of their command and control module.
		float stationRange;
		if(IsSpaceStation())
			stationRange = StationTradeRange(otherShip);
		else
			stationRange = 0;
		float otherStationRange;
		if(otherShip->IsSpaceStation())
			otherStationRange = otherShip->StationTradeRange(this);
		else
			otherStationRange = 0;
		if(otherStationRange > stationRange)
			stationRange = otherStationRange;

		// Transporters may have a huge range.
		float transporterRange = MaxTransporterRange();
		float otherTransporterRange = otherShip->MaxTransporterRange();
		if(otherTransporterRange > transporterRange)
			transporterRange = otherTransporterRange;

		// Return the largest of the ranges available.
		if(transporterRange > transferRange && transporterRange > stationRange)
			return transporterRange;
		else if(stationRange > transferRange)
			return stationRange;
		return transferRange;
	}

	void TransferTroops(DaVessel* fromShip, int howMany);

	virtual COMMODITYID TransporterModule(quint16 which) const
	{
		return SystemModule(SYS_Transporter, which);
	}

	virtual int TransporterPadsWorking(quint16 which) const
	{
		return GetPowerConsumption(SYS_Transporter, which);
	}

	// Returns the transporter range in feet, limited to the current maximum sensor range.
	// Damage to transporters affects the number of pads that can be activated, not the range. 
	virtual float TransporterRange(quint16 which) const
	{
		float sensorRange = SensorRange();
		const float tlBoost = QualityBoostF(SystemQuality(SYS_Transporter, which));
		const float maxTransporterRange = NominalTransporterRange() * tlBoost;	// Maximum possible transporter range at quality.
		if(sensorRange < maxTransporterRange)
			return sensorRange;
		return maxTransporterRange;
	}

	virtual TRANSPORTER_TARGET TransporterTarget() const;

	void TraversedWormhole(const DaSolarSystem* system, const AuWormhole& wh);

	void TrimCargo();

	void TrimFighters();

	void TrimFuel();

	// Returns the gross number of troop berths aboard this ship, even if one is allocated to the captain.
	virtual unsigned int TroopBerths() const = 0;

	// Returns the number of berths available for occupants, considering that one might be allocated to the captain.
	inline unsigned int TroopBerthsAvailable() const
	{
		// Determine the number of troop berths on the ship that are not allocated to the captain.
		unsigned int berths = TroopBerths();
		if(!berths)
			return 0;	// This ship has no troop berths.
		if(CaptainsBerthType() == AuRosterEntry::Troop)
			berths--;	// One of the berths is allocated to the captain.
		if(!berths)
			return 0;	// This ship's only troop berth was allocated to the captain.

		// Check the roster for occupants.
		m_RosterLock.lockForRead();
		unsigned int occupants = m_Roster.BerthsOccupied(AuRosterEntry::Troop);
		m_RosterLock.unlock();
		if(!occupants)
			return berths;

		// Calculate the number of berths available.
		return berths - occupants;
	}

	// Returns the gross number of occupied troop berths aboard this ship, including any allocated to the captain.
	inline unsigned int TroopBerthsOccupied() const
	{
		// If the captain stays in this cabin type, count his berth too.
		unsigned int v;
		if(CaptainsBerthType() == AuRosterEntry::Troop)
		{
			m_RosterLock.lockForRead();
			v = m_Roster.BerthsOccupied(AuRosterEntry::Troop) + m_Roster.BerthsOccupied(AuRosterEntry::Captain);
			m_RosterLock.unlock();
		}
		else
		{
			m_RosterLock.lockForRead();
			v = m_Roster.BerthsOccupied(AuRosterEntry::Troop);
			m_RosterLock.unlock();
		}
		return v;
	}

	// Randomly locates a point inside the hull at which to place a troop or a crew member.
	// Returns false if it doesn't find a clear location within a reasonable amount of searching. Another try might succeed.
	// 'voidLoc' is an output parameter, only valid if return result is true.
	virtual bool TroopOrCrewVoidLocation(AuPoint& voidLoc) const = 0;

	int TroopsAboard() const;

	virtual void TurnAllFTLDrivesOff() = 0;

	virtual void TurnFTLDriveOn(quint16 whichFTL) = 0;

	virtual void TurnFTLDriveOff(quint16 whichFTL) = 0;

	void TurnLeft(unsigned char rotSpeed)
	{
		m_ControlLock.lockForWrite();
		m_wActions &= ~ACT_TurnMask;
		if(rotSpeed)
		{
			m_wActions |= ACT_TurnLeft;
			m_byRotSpeedZ = rotSpeed;
			m_tmLastTurnControl.restart();
			SetInMotion();
			if(!IsTicking())
				TickTock();
		}
		m_ControlLock.unlock();
	}

	// Used by AI to turn off one overclocked system that is not damaged, if any, at the power relay console.
	// Returns true if anything is changed.
	bool TurnOffOneOverclock();

	// Used by AI to turn off one power plant shunt if the power plant is not damaged, if any, at the power relay console.
	// Returns true if anything is changed.
	bool TurnOffOneShunt();

	// Turns off the power plant if it is on.
	inline void TurnPowerPlantOff()
	{
		if(m_bPowerPlantOn)
		{
			m_bPowerPlantOn = false;
			SetModified();
		}
	}

	// Turns on the power plant if it is not already on.
	// Sets the throttle to 0 if the power plant state is actually changed.
	inline void TurnPowerPlantOn()
	{
		// Can't turn power plant on while getting repairs.
		if(RepairSecondsRemaining())
			return;
		if(!m_bPowerPlantOn)
		{
			if(HasPowerPlant())
			{
				m_bPowerPlantOn = true;
				SetModified();

				// Turning on the power plant automatically sets the throttle to zero. That way the engineer station can't blow up
				// the ship in an atmosphere by turning on the power plant when the throttle has been left on high. The engineer
				// station does not otherwise have throttle control to prevent this or even know that it might happen.
				SetThrottle(0);
			}
		}
	}

	// Turn rate in radians per real world second. Adjusted for damage.
	virtual float TurnRate() const = 0;

	inline void TurnRateSkewChange(char chValX, char chValY, unsigned char byDur)
	{
		const int skewMax = 28;								// Person can adjust throughout the range, regardless of fuel quality.
		m_chTurnRateSkewX += chValX;
		if(m_chTurnRateSkewX > skewMax)
			m_chTurnRateSkewX = skewMax;
		else if(m_chTurnRateSkewX < -skewMax)
			m_chTurnRateSkewX = -skewMax;

		m_chTurnRateSkewY += chValY;
		if(m_chTurnRateSkewY > skewMax)
			m_chTurnRateSkewY = skewMax;
		else if(m_chTurnRateSkewY < -skewMax)
			m_chTurnRateSkewY = -skewMax;
		m_byTurnRateSkewDur = byDur;
	}

	void TurnRateSkewWander();

	inline char TurnRateSkewX() const
	{
		return m_chTurnRateSkewX;
	}

	inline char TurnRateSkewY() const
	{
		return m_chTurnRateSkewY;
	}

	void TurnRight(unsigned char rotSpeed)
	{
		m_ControlLock.lockForWrite();
		m_wActions &= ~ACT_TurnMask;
		if(rotSpeed)
		{
			m_wActions |= ACT_TurnRight;
			m_byRotSpeedZ = rotSpeed;
			m_tmLastTurnControl.restart();
			SetInMotion();
			if(!IsTicking())
				TickTock();
		}
		m_ControlLock.unlock();
	}

	virtual void TurnSensorsOff()
	{
		if(m_bSensorsOn)
		{
			m_bSensorsOn = false;
			SetModified();
		}
	}

	virtual void TurnSensorsOn()
	{
		// Can't turn sensors on while getting repairs.
		if(RepairSecondsRemaining())
			return;
		if(!m_bSensorsOn)
		{
			if(HasSensors())
			{
				m_bSensorsOn = true;
				SetModified();
			}
		}
	}

	inline void SetShieldState(SHIELD_STATE state)
	{
		if(m_ShieldState != state)
		{
			m_ShieldState = state;
			SetModified();
		}
	}

	void TurnStop()
	{
		m_ControlLock.lockForWrite();
		m_wActions &= ~ACT_TurnMask;
		m_byRotSpeedZ = 0;
		m_ControlLock.unlock();
	}

	virtual AuItem* TurretAmmunitionClone(quint16 which) const
	{
		AuItem* v;
		m_TurretLock.lockForRead();
		unordered_map<quint16, AuOrdnance>::const_iterator ti = m_Turrets.find(which);
		if(ti != m_Turrets.end() && (*ti).second.Ammo())
			v = (*ti).second.Ammo()->Clone();
		else
			v = 0;
		m_TurretLock.unlock();
		return v;
	}

	inline bool TurretCanFire(quint16 which) const
	{
		bool v;
		m_TurretLock.lockForRead();
		unordered_map<quint16, AuOrdnance>::const_iterator ti = m_Turrets.find(which);
		if(ti != m_Turrets.end())
			v = (*ti).second.CanFire();
		else
			v = false;
		m_TurretLock.unlock();
		return v;
	}

	// Returns the up vector of the turret gun's hemisphere of movement.
	virtual AuPoint TurretHemiVector(const AuPose& pose, quint16 which) const = 0;

	virtual double TurretRange(quint16 which) const = 0;

	inline int TurretRoundsCount(quint16 which) const
	{
		int v;
		m_TurretLock.lockForRead();
		unordered_map<quint16, AuOrdnance>::const_iterator ti = m_Turrets.find(which);
		if(ti != m_Turrets.cend())
			v = (*ti).second.RoundsCount();
		else
			v = 0;
		m_TurretLock.unlock();
		return v;
	}

	virtual int Turrets() const = 0;

	virtual void TxReplyChat(const QString& replyText, const AuCommPacket& msg, AuCommStdChannel::RANGE range);

	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
			const DaPerson& sceneAtom);

	// Called when this atom is credited with killing a unit.
	// All unit kills go through here.
	// This is where empires declare mutual enemies based on unit kills. See DaUnit::UnitKilled.
	virtual void UnitKilled(DaUnit* victim);

	virtual void UnjamDoor(unsigned int doorIndex) = 0;

	inline void UnlockBayWeaponAmmunition() const
	{
		m_BayLock.unlock();
	}

	virtual void UnlockCargo() const
	{
		m_CargoLock.unlock();
	}

	inline void UnlockCryoBerths() const
	{
		m_RosterLock.unlock();
	}

	inline void UnlockMission() const
	{
		m_MissionLock.unlock();
	}

	inline void UnlockMissionOrder() const
	{
		m_MissionLock.unlock();
	}

	inline void UnlockRoster() const
	{
		m_RosterLock.unlock();
	}

	inline void UnlockSystems() const
	{
		m_SystemsLock.unlock();
	}

	inline void UnlockTurretOrdnance() const
	{
		m_TurretLock.unlock();
	}

	inline void UnlockWaypoint() const
	{
		m_WaypointLock.unlock();
	}

	void Unmoor();
	
	// Translate the requested systems' information into a format that the Power Distribution Console can readily Transmit to a
	// client for their power relay console display screen.
	void UpdateDisplayedSystems(const qint32 vSuppliers[MAX_DISPLAYED_SYSTEMS],
			SYSTEM* q8SupplierType,
			quint8* q8SupplierDamage,
			quint8* q8SupplierPower,
			quint8* q8SupplierConnectionConsumer,
			const qint32 vConsumers[MAX_DISPLAYED_SYSTEMS],
			SYSTEM* q8ConsumerType,
			quint8* q8ConsumerConnectionAndPower,
			quint8& q8PowerPlantShunts) const;

	// Return true on system needing updating
	bool UpdateSystemsHealth();

	bool UpdateSystemsHealthSystemsLocked();

	void UpdateSystemsPower();

	void UpdateSystemsPowerSystemsLocked();

	// Returns true if we've been in range of an enemy bay weapon within the 'howRecentlyMs' ms of time.
	inline bool WasInEnemyRange(int howRecentlyMs) const
	{
		m_LastInRangeLock.lockForRead();
		bool inRange = (m_LastInEnemyRange.isValid() && m_LastInEnemyRange.elapsed() < howRecentlyMs) ? true : false;
		m_LastInRangeLock.unlock();
		return inRange;
	}

	inline int WaypointCount() const
	{
		m_WaypointLock.lockForRead();
		int v = m_aWaypoints.count();
		m_WaypointLock.unlock();
		return v;
	}

	virtual bool WeaponBayCanFire(quint16 which) const = 0;

	virtual AuPoint WeaponBayEmitLoc(quint16 whichBay, const AuPoint& gunnerLoc) const = 0;

	// Sets the last fired time for a weapon bay that has been fired 'now'.
	// 'firer' is the person who fired the weapon.
	// 'emitLoc' is the origin of the beam effect relative to the ship.
	// 'targetLoc' is relative to the ship.
	// 'locOnTarget' is relative to the target, for missile targeting.
	// 'ammo' is the ammo type fired, even if it is not consumed.
	// NOTE: m_BayLock is expected to be locked already.
	void WeaponBayFired(const AuId& firer, quint16 which, const AuPoint& emitLoc, float unitsFired, quint8 harvestIndex,
			const AuId& target, const AuPoint& targetLoc, const AuPoint& locOnTarget, const AuItem* ammo);

	virtual quint16 WeaponBayNextId(quint16 which) const = 0;

	virtual quint16 WeaponBayPreviousId(quint16 which) const = 0;

	virtual int WeaponBays() const = 0;

	virtual void WeaponBayIds(QList<quint16>& bayIds) const = 0;

	// Lists weapon bay ids that have a particular module installed.
	virtual void WeaponBaysWithModule(QList<quint16>& bayIds, COMMODITYID bayModule) const = 0;

	inline QString WhereMade() const
	{
		m_VesselLock.lockForRead();
		QString v(m_WhereMade);
		m_VesselLock.unlock();
		return v;
	}

	virtual double Width() const = 0;

	virtual void WorldCaptured(quint32 byEmpire);

	// Returns the name of the most pressing nonstructural system damage.
	// Returns empty string when no systems are damaged.
	QString WorstDamagedSystem() const;

	// Returns a percentage number, from 0 to 100, which is the current damage to nonstructural WorstDamagedSystem.
	// 0 indicates no systems are damaged.
	// 100 indicates a system is completely destroyed.
	unsigned int WorstSystemDamage() const;

public:	// Broker
	void FillShipments(bool shipNow);

	// Beware this is the same lock as the inventory lock.
	inline const AuShipmentList& LockShipmentsForRead() const
	{
		m_CargoLock.lockForRead();
		return m_Shipments;
	}

	// Returns: 0 if the shipment is created.
	//          1 Broker not found.
	//          2 Broker destroyed.
	//          3 Broker no levels.
	//          4 Broker at max.
	//          5 City at max.
	//          6 must not be insystem destination
	int ShipmentCreate(const AuShipment& shipment);

	// Returns: 0 if the shipment is deleted.
	//          1 Broker not found.
	//          2 Broker destroyed.
	//          3 Broker no levels.
	//          4 Shipment not found.
	int ShipmentDelete(const AuShipment& shipment);

	// Clear me to keep 'container' because the destination could not pay for delivery.
	void ShipmentDefaulted(const AuItemTEUContainer& container);

	inline void UnlockShipments() const
	{
		m_CargoLock.unlock();
	}

protected:
	// Update ATC contact information.
	void ATCContact(const DaSolarSystem* system);

	virtual void DowngradeSystemModules(quint8 repairQ, const QSet<AuSysId>& systemsRepaired);

	void RenderGameChair(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, const AuPoint& loc,
			float rotZ, float rotAroundTable);

	void RenderGameTable(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, const AuPoint& loc, float rotZ,
			quint16 whichTable);

	void RenderShieldBubble(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
			const DaPerson* sceneAtom);

	bool RenderTractorBeams(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re);

	void ReportPosition();
};

#endif // DaVessel_H
