//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: davessel.cpp,v 1.479 2022/05/17 17:10:38 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "davessel.h"
//
#include "auatomchat.h"
#include "auatomrenderinfo.h"
#include "aubox.h"
#include "aucommpacket.h"
#include "audesign.h"
#include "audialog.h"
#include "augame.h"
#include "auscreport.h"
#include "ausensorblip.h"
#include "ausound.h"
#include "ausphere.h"
#include "autractorbeam.h"
#include "daanimal.h"
#include "daarena.h"
#include "daasteroid.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dadesigner.h"
#include "daeffect.h"
#include "dagalaxy.h"
#include "damissile.h"
#include "danpc.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dateleporter.h"
#include "davehicle.h"
#include "daworldgasgiant.h"
#include "daworldglobe.h"
#include "daworldring.h"
#include "daworldringsection.h"
#include "dgdata.h"

static Au3dsGLSLModelInstance*			s_GameChair = 0;		// Game table model.
static bool								s_GameChairExt = false;	// true when we have the extents.
static AuPoint							s_GameChairBase, s_GameChairMax, s_GameChairMin;
static Au3dsGLSLModelInstance*			s_GameTable = 0;		// Game table chair model.
static bool								s_GameTableExt = false;	// true when we have the extents.
static AuPoint							s_GameTableBase, s_GameTableMax, s_GameTableMin;
static int								s_MinBounty = 10000;


DaVessel::DaVessel(const QString& name, int idEmpire) : DaCapsule(name, idEmpire)
{
	m_AcceptCrew = false;
	m_AcceptPassengers = false;
	m_AcceptTroops = false;
	m_AccessPolicy = ACCESS_Friend;
	m_AllowPassengerSeaChests = false;
	m_ATCAtRest = false;
	m_AutoFetchVehicles = true;
	m_Bank = 0;
	m_BankSubsidyTax = 0;
	m_bExtLightOn = false;
	m_bPowerPlantOn = false;
	m_bSensorsOn = false;
	m_byAccelSkewDur = 0;
	m_byTurnRateSkewDur = 0;
	m_byPowerPlantSkewDur = 0;
	m_byRotSpeedX = 0;
	m_byRotSpeedY = 0;
	m_byRotSpeedZ = 0;
	m_CapacitorLevel = 0;
	m_CapacitorTrend = 0;
	m_chAccelSkew = 0;
	m_chTurnRateSkewX = 0;
	m_chTurnRateSkewY = 0;
	m_chPowerPlantSkew = 0;
	m_ConCaptAI = 0;
	m_ConEngineerAI = 0;
	m_ConEngineerPerson = 0;
	m_ConNavigatorAI = 0;
	m_ConNavigatorPerson = 0;
	m_ConPilotAI = 0;
	m_ConPilotPerson = 0;
	m_ConPowerRelayAI = 0;
	m_ConPowerRelayPerson = 0;
	m_ConSensorAI = 0;
	m_ConSensorPerson = 0;
	m_ConShieldAI = 0;
	m_ConShieldPerson = 0;
	m_ConWeaponAI = 0;
	m_ConWeaponPerson = 0;
	m_eGravityDriveState = GD_Off;
	m_ETE = 0;
	m_fFuelQuantity = 0;
	for(int iSEffect=0; iSEffect<MAX_SHIELD_HIT_EFFECTS*4; iSEffect++)
		m_fShieldEffect[iSEffect] = 1;		// 1 because we can't divide by zero in fragment shader
	for(int iSDEffect=0; iSDEffect<MAX_SHIELD_HIT_EFFECTS; iSDEffect++)
		m_fShieldDuration[iSDEffect] = 0;	// 0 which turns off all hit effects
	m_fShieldTexOffset = 0;
	m_FuelQuality = 1;
	m_InTow = false;
	m_IsHumanLock = false;
	m_IsPrivate = false;
	m_LockedIsCivilian = false;
	m_LockedStance = POL_Empire;
	m_Monster = 0;
	m_MonsterOrigin = MONSTER_Unknown;
	m_Moored = true;					// New vessels start out moored.
	m_PilotUpPolicy = PortTowardGravity;
	m_Pitch = 0;
	m_PowerPlantShunts = 0;				// No shunts in use.
	m_QMade = 1;
	m_RepairSecs = 0;
	m_Roll = 0;
	for(int side=0; side<6; side++)
	{
		m_ShieldCap[side] = 0;
		m_ShieldScram[side] = 0;
	}
	m_ShieldFuelPolicy = ShieldUpUnlessNoFuel;
	m_ShieldPolicy = ShieldUp;
	m_ShieldState = SS_Off;
	m_Throttle = 0;
	m_ThrustLateral = 0;
	m_ThrustLong = 0;
	m_ThrustVertical = 0;
	m_TractorBeam = 0;
	m_wActions = ACT_None;
	m_Yaw = 0;
}

DaVessel::DaVessel()
{
	m_AcceptCrew = false;
	m_AcceptPassengers = false;
	m_AcceptTroops = false;
	m_AccessPolicy = ACCESS_Friend;
	m_AllowPassengerSeaChests = false;
	m_ATCAtRest = false;
	m_AutoFetchVehicles = true;
	m_Bank = 0;
	m_BankSubsidyTax = 0;
	m_bExtLightOn = false;
	m_bPowerPlantOn = false;
	m_bSensorsOn = false;
	m_byAccelSkewDur = 0;
	m_byTurnRateSkewDur = 0;
	m_byPowerPlantSkewDur = 0;
	m_byRotSpeedX = 0;
	m_byRotSpeedY = 0;
	m_byRotSpeedZ = 0;
	m_CapacitorLevel = 0;
	m_CapacitorTrend = 0;
	m_chAccelSkew = 0;
	m_chTurnRateSkewX = 0;
	m_chTurnRateSkewY = 0;
	m_chPowerPlantSkew = 0;
	m_ConCaptAI = 0;
	m_ConEngineerAI = 0;
	m_ConEngineerPerson = 0;
	m_ConNavigatorAI = 0;
	m_ConNavigatorPerson = 0;
	m_ConPilotAI = 0;
	m_ConPilotPerson = 0;
	m_ConPowerRelayAI = 0;
	m_ConPowerRelayPerson = 0;
	m_ConSensorAI = 0;
	m_ConSensorPerson = 0;
	m_ConShieldAI = 0;
	m_ConShieldPerson = 0;
	m_ConWeaponAI = 0;
	m_ConWeaponPerson = 0;
	m_eGravityDriveState = GD_Off;
	m_ETE = 0;
	m_fFuelQuantity = 0;
	for(int iSEffect=0; iSEffect<MAX_SHIELD_HIT_EFFECTS*4; iSEffect++)
		m_fShieldEffect[iSEffect] = 1;		// 1 because we can't divide by zero in fragment shader
	for(int iSDEffect=0; iSDEffect<MAX_SHIELD_HIT_EFFECTS; iSDEffect++)
		m_fShieldDuration[iSDEffect] = 0;	// 0 which turns off all hit effects
	m_fShieldTexOffset = 0;
	m_FuelQuality = 1;
	m_InTow = false;
	m_IsHumanLock = false;
	m_IsPrivate = false;
	m_LockedIsCivilian = false;
	m_LockedStance = POL_Empire;
	m_Monster = 0;
	m_MonsterOrigin = MONSTER_Unknown;
	m_Moored = true;					// New vessels start out moored.
	m_PilotUpPolicy = PortTowardGravity;
	m_Pitch = 0;
	m_PowerPlantShunts = 0;				// No shunts in use.
	m_QMade = 1;
	m_RepairSecs = 0;
	m_Roll = 0;
	for(int side=0; side<6; side++)
	{
		m_ShieldCap[side] = 0;
		m_ShieldScram[side] = 0;
	}
	m_ShieldFuelPolicy = ShieldUpUnlessNoFuel;
	m_ShieldPolicy = ShieldUp;
	m_ShieldState = SS_Off;
	m_Throttle = 0;
	m_ThrustLateral = 0;
	m_ThrustLong = 0;
	m_ThrustVertical = 0;
	m_TractorBeam = 0;
	m_wActions = ACT_None;
	m_Yaw = 0;
}

DaVessel::~DaVessel()
{
	// Delete games
	for(unordered_map<quint16, AuGame*>::iterator gi = m_Games.begin(); gi != m_Games.end(); gi++)
		delete (*gi).second;

	if(m_TractorBeam)
		delete m_TractorBeam;
}

DaVessel::DaVessel(const DaVessel& rhs) : DaCapsule(rhs)
{
	m_AcceptCrew = rhs.m_AcceptCrew;
	m_AcceptPassengers = rhs.m_AcceptPassengers;
	m_AcceptTroops = rhs.m_AcceptTroops;
	m_AccessPolicy = rhs.m_AccessPolicy;
	m_AllowPassengerSeaChests = rhs.m_AllowPassengerSeaChests;
	m_AutoFetchVehicles = rhs.m_AutoFetchVehicles;
	m_bExtLightOn = rhs.m_bExtLightOn;
	m_bPowerPlantOn = rhs.m_bPowerPlantOn;
	m_bSensorsOn = rhs.m_bSensorsOn;
	m_byAccelSkewDur = rhs.m_byAccelSkewDur;
	m_byPowerPlantSkewDur = rhs.m_byPowerPlantSkewDur;
	m_byTurnRateSkewDur = rhs.m_byTurnRateSkewDur;
	m_chAccelSkew = rhs.m_chAccelSkew;
	m_chPowerPlantSkew = rhs.m_chPowerPlantSkew;
	m_chTurnRateSkewX = rhs.m_chTurnRateSkewX;
	m_chTurnRateSkewY = rhs.m_chTurnRateSkewY;
	m_ConCaptAI = rhs.m_ConCaptAI;
	m_ConEngineerAI = rhs.m_ConEngineerAI;
	m_ConEngineerPerson = rhs.m_ConEngineerPerson;
	m_ConNavigatorAI = rhs.m_ConNavigatorAI;
	m_ConNavigatorPerson = rhs.m_ConNavigatorPerson;
	m_ConPilotAI = rhs.m_ConPilotAI;
	m_ConPilotPerson = rhs.m_ConPilotPerson;
	m_ConPowerRelayAI = rhs.m_ConPowerRelayAI;
	m_ConPowerRelayPerson = rhs.m_ConPowerRelayPerson;
	m_ConSensorAI = rhs.m_ConSensorAI;
	m_ConSensorPerson = rhs.m_ConSensorPerson;
	m_ConShieldAI = rhs.m_ConShieldAI;
	m_ConShieldPerson = rhs.m_ConShieldPerson;
	m_ConWeaponAI = rhs.m_ConWeaponAI;
	m_ConWeaponPerson = rhs.m_ConWeaponPerson;
	for(int iSDEffect=0; iSDEffect<MAX_SHIELD_HIT_EFFECTS; iSDEffect++)
		m_fShieldDuration[iSDEffect] = rhs.m_fShieldDuration[iSDEffect];
	for(int iSEffect=0; iSEffect<MAX_SHIELD_HIT_EFFECTS*4; iSEffect++)
		m_fShieldEffect[iSEffect] = rhs.m_fShieldEffect[iSEffect];
	m_fShieldTexOffset = rhs.m_fShieldTexOffset;
	m_FuelQuality = rhs.m_FuelQuality;
	m_InTow = rhs.m_InTow;
	m_IsHumanLock = rhs.m_IsHumanLock;
	m_IsPrivate = rhs.m_IsPrivate;
	m_LockedIsCivilian = rhs.m_LockedIsCivilian;
	m_LockedStance = rhs.m_LockedStance;
	m_Moored = rhs.m_Moored;
	m_PilotUpPolicy = rhs.m_PilotUpPolicy;
	m_PowerPlantShunts = rhs.m_PowerPlantShunts;
	m_QMade = rhs.m_QMade;
	m_ShieldFuelPolicy = rhs.m_ShieldFuelPolicy;
	m_ShieldPolicy = rhs.m_ShieldPolicy;
	m_ShieldState = rhs.m_ShieldState;

	rhs.m_VesselLock.lockForRead();
	m_CapacitorLevel = rhs.m_CapacitorLevel;
	m_CapacitorTrend = rhs.m_CapacitorTrend;
	m_ConAICaptId = rhs.m_ConAICaptId;
	m_eGravityDriveState = rhs.m_eGravityDriveState;
	m_ETE = rhs.m_ETE;
	m_fFuelQuantity = rhs.m_fFuelQuantity;
	m_LockedBlip = rhs.m_LockedBlip;
	m_LockedSysLoc = rhs.m_LockedSysLoc;
	m_Monster = rhs.m_Monster;
	m_MonsterOrigin = rhs.m_MonsterOrigin;
	m_RatSpecies = rhs.m_RatSpecies;
	m_RepairSecs = rhs.m_RepairSecs;
	m_WhereMade = rhs.m_WhereMade;
	rhs.m_VesselLock.unlock();

	rhs.m_AccountLock.lockForRead();
	m_Bank = rhs.m_Bank;
	m_BankSubsidyTax = rhs.m_BankSubsidyTax;
	m_tmLastBank = rhs.m_tmLastBank;
	rhs.m_AccountLock.unlock();

	rhs.m_BayLock.lockForRead();
	m_BayFired = rhs.m_BayFired;
	m_BaySetting = rhs.m_BaySetting;
	m_tmBayFired = rhs.m_tmBayFired;
	rhs.m_BayLock.unlock();

	rhs.m_CargoLock.lockForRead();
	for(AuItemHash::const_iterator ii = rhs.m_Cargo.begin(); ii != rhs.m_Cargo.end(); ii++)
		m_Cargo.insert(pair<COMMODITYID, AuItem*>((*ii).first, (*ii).second->Clone()));
	m_Shipments = rhs.m_Shipments;
	m_tmCargoChange = rhs.m_tmCargoChange;
	rhs.m_CargoLock.unlock();

	rhs.m_CommLock.lockForRead();
	m_ATCAtRest = rhs.m_ATCAtRest;
	m_ATCCity = rhs.m_ATCCity;
	m_ATCCityEnemy = rhs.m_ATCCityEnemy;
	m_ATCContact = rhs.m_ATCContact;
	m_ATCStation = rhs.m_ATCStation;
	m_ATCStationEnemy = rhs.m_ATCStationEnemy;
	m_tmAlert = rhs.m_tmAlert;
	m_tmEnvDamage = rhs.m_tmEnvDamage;
	m_tmSOS = rhs.m_tmSOS;
	rhs.m_CommLock.unlock();

	rhs.m_ControlLock.lockForRead();
	m_byRotSpeedX = rhs.m_byRotSpeedX;
	m_byRotSpeedY = rhs.m_byRotSpeedY;
	m_byRotSpeedZ = rhs.m_byRotSpeedZ;
	m_Pitch = rhs.m_Pitch;
	m_Roll = rhs.m_Roll;
	m_Throttle = rhs.m_Throttle;
	m_ThrustLateral = rhs.m_ThrustLateral;
	m_ThrustLong = rhs.m_ThrustLong;
	m_ThrustVertical = rhs.m_ThrustVertical;
	m_tmLastBrake = rhs.m_tmLastBrake;
	m_tmLastBrakeLV = rhs.m_tmLastBrakeLV;
	m_tmLastPitchControl = rhs.m_tmLastPitchControl;
	m_tmLastRollControl = rhs.m_tmLastRollControl;
	m_tmLastThrottleControl = rhs.m_tmLastThrottleControl;
	m_tmLastThrottleFControl = rhs.m_tmLastThrottleFControl;
	m_tmLastThrottleLControl = rhs.m_tmLastThrottleLControl;
	m_tmLastThrottleVControl = rhs.m_tmLastThrottleVControl;
	m_tmLastTurnControl = rhs.m_tmLastTurnControl;
	m_Yaw = rhs.m_Yaw;
	m_wActions = rhs.m_wActions;
	rhs.m_ControlLock.unlock();

	rhs.m_DamageLock.lockForRead();
	m_Damage = rhs.m_Damage;
	m_tmDamageChange = rhs.m_tmDamageChange;
	rhs.m_DamageLock.unlock();

	rhs.m_GamesLock.lockForRead();
	for(unordered_map<quint16, AuGame*>::const_iterator gi = rhs.m_Games.begin(); gi != rhs.m_Games.end(); gi++)
	{
		AuGame* game = (*gi).second->Clone();
		m_Games.insert(pair<quint16, AuGame*>((*gi).first, game));
	}
	m_GamesChanged = rhs.m_GamesChanged;
	rhs.m_GamesLock.unlock();

	rhs.m_LastInRangeLock.lockForRead();
	m_LastInEnemyRange = rhs.m_LastInEnemyRange;
	rhs.m_LastInRangeLock.unlock();

	rhs.m_MissionLock.lockForRead();
	m_Mission = rhs.m_Mission;
	m_tmMissionChange = rhs.m_tmMissionChange;
	rhs.m_MissionLock.unlock();

	rhs.m_NameWithEmpireLock.lockForRead();
	m_NameWithEmpire = rhs.m_NameWithEmpire;
	rhs.m_NameWithEmpireLock.unlock();

	rhs.m_RosterLock.lockForRead();
	m_OwnerId = rhs.m_OwnerId;
	m_OwnerName = rhs.m_OwnerName;
	m_Roster = rhs.m_Roster;
	m_CryoBerths = rhs.m_CryoBerths;
	m_tmRosterChange = rhs.m_tmRosterChange;
	rhs.m_RosterLock.unlock();

	rhs.m_ShieldsLock.lockForRead();
	for(int side=0; side<6; side++)
	{
		m_ShieldCap[side] = rhs.m_ShieldCap[side];
		m_ShieldScram[side] = rhs.m_ShieldScram[side];
	}
	rhs.m_ShieldsLock.unlock();

	rhs.m_SystemsLock.lockForRead();
	m_ssaShipSystems = rhs.m_ssaShipSystems;
	m_tmSystemsUpg = rhs.m_tmSystemsUpg;
	rhs.m_SystemsLock.unlock();

	rhs.m_TurretLock.lockForRead();
	m_Turrets = rhs.m_Turrets;
	rhs.m_TurretLock.unlock();

	rhs.m_WaypointLock.lockForRead();
	m_aWaypoints = rhs.m_aWaypoints;
	m_tmWaypointChange = rhs.m_tmWaypointChange;
	rhs.m_WaypointLock.unlock();

	m_shieldTime = rhs.m_shieldTime;
	m_TractorBeam = 0;
}

void DaVessel::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx, const DaPerson& sceneAtom)
{
	DaCapsule::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);

	quint16 hasWhat = 0x0000;

	// Sensor data is relevant only when the scene atom is aboard this ship and the sensors are working.
	quint8 lockedStance = POL_Neutral;
	AuPoint lockedSysLoc;
	if((const DaAtom*)sceneAtom.Parent() == this &&
			!LockedBlip().IsNull() &&
			(sceneAtom.IsCommandStationPose() ||
			 sceneAtom.IsNavStationPose() ||
			 sceneAtom.IsHelmStationPose() ||
			 sceneAtom.IsShieldStationPose() ||
			 sceneAtom.IsTurretPose()) &&
			SensorsWorking())
	{
		const DaAtom* lockedAtom = FindEnvironmentAtom()->FindIdRecursive(LockedBlip());
		if(lockedAtom)
		{
			// The ship needs to send the locked-on blip when the scene atom is sitting in a turret or the captain's chair.
			// Consoles send the sensor scan information that they need.
			// Turrets aren't actually consoles so there is no console object to send the blip for the gunner.
			// Captain's chair doesn't send the lock-on blip either.
			// Captain always gets the lock-on blip so they can select it as a destination on the mission user interface.
			if(dynamic_cast<const DaAsteroid*>(lockedAtom))
				lockedStance = POL_Enemy;
			else if(lockedAtom->EmpireId())
				lockedStance = PoliticalStance(EmpireId(), lockedAtom->EmpireId());
			else
				lockedStance = POL_Neutral;
			if(lockedAtom->IsCivilian())
				lockedStance |= 0x80;
			lockedSysLoc = lockedAtom->EnvLocation();
			hasWhat |= 0x0001;	// Scene atom needs me to send the lock-on blip.
		}
	}

	// Ship systems data.
	m_SystemsLock.lockForRead();
	if(lastTx.IsNull() || m_tmSystemsUpg >= lastTx)
		hasWhat |= 0x0002;		// ship systems (a subset of the info)
	m_SystemsLock.unlock();

	m_DamageLock.lockForRead();
	if(tmLastUpdate.IsNull() || m_tmDamageChange >= tmLastUpdate)
		hasWhat |= 0x0100;		// damage to this spacecraft
	m_DamageLock.unlock();

	// Use a bit to notify the client when they are in range of an enemy bay weapon.
	if(IsInEnemyRange())
		hasWhat |= 0x0004;

	m_WaypointLock.lockForRead();
	if(lastTx.IsNull() || m_tmWaypointChange >= lastTx)
	{
		// Need next waypoint at all consoles for lower pane display.
		if(m_aWaypoints.count() > 0)
			hasWhat |= 0x0010;	// next waypoint only
		else
			hasWhat |= 0x0020;	// clear waypoints
	}
	m_WaypointLock.unlock();

	if(m_Moored)
		hasWhat |= 0x0040;

	m_CargoLock.lockForRead();
	if(lastTx.IsNull() || m_tmCargoChange >= lastTx)
		hasWhat |= 0x0080;		// inventory of cargo
	m_CargoLock.unlock();

	if(CapacitorLevel())
		hasWhat |= 0x0200;

	if(RepairSecondsRemaining())
		hasWhat |= 0x0400;

	m_RosterLock.lockForRead();
	if(lastTx.IsNull() || m_tmRosterChange >= lastTx)
		hasWhat |= 0x0800;		// crew roster of this spacecraft
	m_RosterLock.unlock();

	m_BayLock.lockForRead();
	if(lastTx.IsNull() || m_tmBayFired >= lastTx)
		hasWhat |= 0x1000;
	m_BayLock.unlock();

	// Send a bit so the client knows if this ship is having a fuel emergency.
	if(!IsMoored() && FuelQuantity() < 1 && (IsCommandedByAI() || IsPilotMannedByAI()))
	{
		if(IsSpaceStation())
			hasWhat |= 0x2000;	// Fuel emergency.
		else if(IsOnGround() && !IsMoving())
		{
			if(BdId().IsNull())
				hasWhat |= 0x2000;	// Fuel emergency.
		}
		else
			hasWhat |= 0x2000;	// Fuel emergency.
	}
	
	double shieldMax = ShieldUnitsWorking();
	m_ShieldsLock.lockForRead();
	double s0 = m_ShieldCap[0];
	if(m_ShieldScram[0])
		s0 = -s0;
	double s1 = m_ShieldCap[1];
	if(m_ShieldScram[1])
		s1 = -s1;
	double s2 = m_ShieldCap[2];
	if(m_ShieldScram[2])
		s2 = -s2;
	double s3 = m_ShieldCap[3];
	if(m_ShieldScram[3])
		s3 = -s3;
	double s4 = m_ShieldCap[4];
	if(m_ShieldScram[4])
		s4 = -s4;
	double s5 = m_ShieldCap[5];
	if(m_ShieldScram[5])
		s5 = -s5;
	m_ShieldsLock.unlock();
	if(lastTx.IsNull())
		hasWhat |= 0x8000;	// Send all values first time. Client may not have design yet.
	else if(s0 >= shieldMax && s1 >= shieldMax && s2 >= shieldMax && s3 >= shieldMax && s4 >= shieldMax && s5 >= shieldMax)
		hasWhat |= 0x4000;	// All shields are maxed out.
	else
		hasWhat |= 0x8000;	// Shield capacitors are mixed.

	toClient << hasWhat;
	toClient << m_QMade;
	toClient << (quint8)m_PilotUpPolicy;

	// optimize switches into bits
	quint16 switches = 0x0000;
	if(m_bExtLightOn)
		switches |= 0x0001;
	if(m_bPowerPlantOn)
		switches |= 0x0002;

	m_VesselLock.lockForRead();
	if(m_eGravityDriveState == GD_On)
		switches |= 0x0010;
	else if(m_eGravityDriveState == GD_Hold)
		switches |= 0x0020;
	m_VesselLock.unlock();

	if(m_AutoFetchVehicles)
		switches |= 0x0080;
	if(m_AcceptCrew)
		switches |= 0x0100;
	if(m_AcceptPassengers)
		switches |= 0x0200;
	if(m_AcceptTroops)
		switches |= 0x0400;
	if(m_IsPrivate)
		switches |= 0x0800;
	if(m_ShieldState)
		switches |= m_ShieldState << 12;	// bits b12-b13
	if(m_bSensorsOn)
		switches |= 0x4000;

	// Shield policies in the top nibble.
	// b7-b5 = shield up policy
	// b4 = shield fuel policy
	// b3-b1 = access policy
	// b0 = passenger sea chest access
	quint8 switches2 = (m_ShieldPolicy<<5)|(m_ShieldFuelPolicy<<4)|(m_AccessPolicy<<1);
	if(m_AllowPassengerSeaChests)
		switches2 |= 0x01;

	toClient << switches;
	toClient << switches2;

	m_ControlLock.lockForRead();
	toClient << m_Throttle;			// Throttle setting
	toClient << m_ThrustLateral;	// Throttle setting
	toClient << m_ThrustLong;		// Throttle setting
	toClient << m_ThrustVertical;	// Throttle setting
	m_ControlLock.unlock();

	AuPoint v(Vector());
	toClient << (float)v.x;
	toClient << (float)v.y;
	toClient << (float)v.z;

	m_VesselLock.lockForRead();
	toClient << (float)m_fFuelQuantity;
	toClient << m_FuelQuality;
	m_VesselLock.unlock();

	//toClient << m_byAccelSkewDur;
	//toClient << m_byTurnRateSkewDur;
	//toClient << m_byPowerPlantSkewDur;
	//toClient << (unsigned char)m_chAccelSkew;
	toClient << (qint8)m_chTurnRateSkewX;
	toClient << (qint8)m_chTurnRateSkewY;
	//toClient << (unsigned char)m_chPowerPlantSkew;

	if(hasWhat & 0x0001)
	{
		toClient << LockedBlip();	// LOCKED sensor blip only
		toClient << lockedStance;
		toClient << (float)lockedSysLoc.x;
		toClient << (float)lockedSysLoc.y;
		toClient << (float)lockedSysLoc.z;
	}

	if(hasWhat & 0x0002)
	{
		m_SystemsLock.lockForRead();
		m_ssaShipSystems.Tx(toClient);
		m_SystemsLock.unlock();
	}

	//toClient << m_byRotSpeedX;
	//toClient << m_byRotSpeedY;
	//toClient << m_byRotSpeedZ;
	//toClient << m_wActions;
	if(hasWhat & 0x0010)
	{
		m_WaypointLock.lockForRead();
		m_aWaypoints[0].Tx(toClient);	// NEXT waypoint
		m_WaypointLock.unlock();
	}

	if(hasWhat & 0x0080)
	{
		m_CargoLock.lockForRead();
		toClient << m_Cargo;
		m_CargoLock.unlock();
	}

	if(hasWhat & 0x0100)
	{
		m_DamageLock.lockForRead();
		m_Damage >> toClient;	// Damage to spacecraft.
		m_DamageLock.unlock();
	}

	if(hasWhat & 0x0200)
		toClient << CapacitorLevel();	// Capacitor Level

	if(hasWhat & 0x0800)
	{
		m_RosterLock.lockForRead();
		toClient << m_OwnerId;
		toClient << m_OwnerName;
		toClient << m_Roster;
		toClient << m_CryoBerths;
		m_RosterLock.unlock();
	}

	if(hasWhat & 0x1000)
	{
		// The client only needs the last fired times for texture animation. Target id is not relevant except for tractor beams.
		m_BayLock.lockForRead();
		toClient << (quint8)m_BayFired.size();
		for(unordered_map<quint16, AuBayFired>::const_iterator lfi = m_BayFired.cbegin(); lfi != m_BayFired.cend(); lfi++)
		{
			toClient << (*lfi).first;			// 'which' bay fired.
			toClient << (*lfi).second.m_When;	// Last fired time for weapon bay.
			toClient << (*lfi).second.m_TractorState;
			if((*lfi).second.m_TractorState)
			{
				// Tractor beam sends the target atom for visual effect.
				toClient << (*lfi).second.m_EmitLoc;
				toClient << (*lfi).second.m_Target;
			}
		}
		m_BayLock.unlock();
	}

	if(hasWhat & 0x0400)
	{
		m_VesselLock.lockForRead();
		toClient << m_RepairSecs;
		m_VesselLock.unlock();
	}

	if(hasWhat & 0x8000)
	{
		toClient << s0;
		toClient << s1;
		toClient << s2;
		toClient << s3;
		toClient << s4;
		toClient << s5;
	}

	// Send another hasWhat bit mask because we used up the first one.
	quint8 hasWhat2 = 0;
	m_GamesLock.lockForRead();
	if((lastTx.IsNull() || m_GamesChanged >= lastTx) && (const DaAtom*)sceneAtom.Parent() == this)
		hasWhat2 |= 0x01;	// Game info only sent to people who are aboard.
	m_GamesLock.unlock();

	m_AccountLock.lockForRead();
	if(lastTx.IsNull() || m_tmLastBank >= lastTx)
		hasWhat2 |= 0x02;
	m_AccountLock.unlock();

	m_MissionLock.lockForRead();
	if(lastTx.IsNull() || m_tmMissionChange >= lastTx)
		hasWhat2 |= 0x04;	// Mission
	m_MissionLock.unlock();

	if(lastTx.IsNull())
		hasWhat2 |= 0x08;	// One-time only stuff.

	m_ControlLock.lockForRead();
	if(m_wActions & ACT_Brake)
		hasWhat2 |= 0x20;
	if(m_wActions & ACT_BrakeLateral)
		hasWhat2 |= 0x40;
	if(m_wActions & ACT_BrakeVertical)
		hasWhat2 |= 0x80;
	m_ControlLock.unlock();

	toClient << hasWhat2;

	// Send the second wave of has-whats.
	if(hasWhat2 & 0x01)
	{
		// Active games at tables. Also empty list when we are asserting there are no active games.
		m_GamesLock.lockForRead();
		Q_ASSERT(m_Games.size() <= 0xffff);
		toClient << (quint16)m_Games.size();
		for(unordered_map<quint16, AuGame*>::const_iterator gi = m_Games.begin(); gi != m_Games.end(); gi++)
		{
			toClient << (*gi).first;
			toClient << (*gi).second->Type();
			(*gi).second->Tx(toClient, sceneAtom);
		}
		m_GamesLock.unlock();
	}

	if(hasWhat2 & 0x02)
	{
		m_AccountLock.lockForRead();
		toClient << m_Bank;
		toClient << m_BankSubsidyTax;
		m_AccountLock.unlock();
	}

	if(hasWhat2 & 0x04)
	{
		m_MissionLock.lockForRead();
		m_Mission >> toClient;
		m_MissionLock.unlock();
	}

	// One-time only stuff.
	if(hasWhat2 & 0x08)
	{
		m_VesselLock.lockForRead();
		toClient << m_WhereMade;
		m_VesselLock.unlock();
	}
}

void DaVessel::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	QString oldName = AssignedName();
	quint32 oldEmpire = Allegiance();
	DaCapsule::RxUpdate(fromServer, isFirstUpdate);

	quint16 hasWhat;
	fromServer >> hasWhat;
	fromServer >> m_QMade;
	quint8 ui8;
	fromServer >> ui8;
	m_PilotUpPolicy = (PILOT_UP_POLICY)ui8;

	m_Moored = (hasWhat & 0x0040) ? true : false;
	m_Monster = (hasWhat & 0x2000) ? true : false;	// On client this variable means fuel emergency.

	quint16 switches;
	fromServer >> switches;
	m_bExtLightOn = (switches & 0x0001) ? true : false;
	m_bPowerPlantOn = (switches & 0x0002) ? true : false;
	if(switches & 0x0010)
		m_eGravityDriveState = GD_On;
	else if(switches & 0x0020)
		m_eGravityDriveState = GD_Hold;
	else
		m_eGravityDriveState = GD_Off;

	m_AutoFetchVehicles = (switches & 0x0080) ? true : false;
	m_AcceptCrew = (switches & 0x0100) ? true : false;
	m_AcceptPassengers = (switches & 0x0200) ? true : false;
	m_AcceptTroops = (switches & 0x0400) ? true : false;
	m_IsPrivate = (switches & 0x0800) ? true : false;
	m_bSensorsOn = (switches & 0x4000) ? true : false;
	m_ShieldState = (SHIELD_STATE)((switches & 0x3000) >> 12);	// bits b12 - b13

	quint8 switches2;
	fromServer >> switches2;
	// Shield policies in the top nibble.
	// b7-b5 = shield up policy
	// b4 = shield fuel policy
	m_ShieldPolicy = (SHIELD_POLICY)(switches2>>5);								// Top three bits.
	m_ShieldFuelPolicy = (SHIELDFUEL_POLICY)((switches2>>4) & 0x01);			// Mask it down to just one bit.
	m_AccessPolicy = (ACCESS_POLICY)((switches2>>1) & 0x07);					// Mask it down to just three bits.
	m_AllowPassengerSeaChests = (switches2 & 0x01) ? true : false;				// Bottom bit.

	fromServer >> m_Throttle;			// Throttle setting
	fromServer >> m_ThrustLateral;	// Throttle setting
	fromServer >> m_ThrustLong;		// Throttle setting
	fromServer >> m_ThrustVertical;	// Throttle setting
	float vx,vy,vz;
	fromServer >> vx;
	fromServer >> vy;
	fromServer >> vz;
	SetVector(AuPoint(vx,vy,vz));
	float val;
	fromServer >> val;
	m_fFuelQuantity = val;
	fromServer >> m_FuelQuality;
	if(!m_FuelQuality)
		m_FuelQuality = 1;
	//fromServer >> m_byAccelSkewDur;
	//fromServer >> m_byTurnRateSkewDur;
	//fromServer >> m_byPowerPlantSkewDur;
	//fromServer >> (unsigned char)m_chAccelSkew;
	qint8 i8;
	fromServer >> i8;
	m_chTurnRateSkewX = i8;
	fromServer >> i8;
	m_chTurnRateSkewY = i8;
	//fromServer >> (unsigned char)m_chPowerPlantSkew;
	//quint8 ui8;
	//fromServer >> ui8;

	if(hasWhat & 0x0001)
	{
		fromServer >> m_LockedBlip;
		quint8 ui8;
		fromServer >> ui8;
		m_LockedStance = ui8 & 0x0f;
		m_LockedIsCivilian = ui8 & 0x80;
		float x,y,z;
		fromServer >> x;
		fromServer >> y;
		fromServer >> z;
		m_LockedSysLoc = AuPoint(x,y,z);
	}
	else
		m_LockedBlip = 0;

	if(hasWhat & 0x0002)
		m_ssaShipSystems.Rx(fromServer);

	if(hasWhat & 0x0004)
	{
		m_LastInRangeLock.lockForWrite();
		m_LastInEnemyRange.restart();	// We were within range of an enemy bay weapon within the last second.
		m_LastInRangeLock.unlock();
	}

	//fromServer >> m_byRotSpeedX;
	//fromServer >> m_byRotSpeedY;
	//fromServer >> m_byRotSpeedZ;
	//fromServer >> m_wActions;

	if(hasWhat & 0x0010)
	{
		DeWaypoint way;
		way.Rx(fromServer);
		m_aWaypoints.clear();
		m_aWaypoints.append(way);
	}
	else if(hasWhat & 0x0020)
	{
		m_aWaypoints.clear();
	}

	if(hasWhat & 0x0080)
	{
		m_CargoLock.lockForWrite();
		fromServer >> m_Cargo;
		m_CargoLock.unlock();
	}

	if(hasWhat & 0x0100)
	{
		m_DamageLock.lockForWrite();
		m_Damage << fromServer;	// Damage to this spacecraft.
		m_DamageLock.unlock();
	}

	// Client doesn't actually send zero, just a bit.
	if(hasWhat & 0x0200)
		fromServer >> m_CapacitorLevel;
	else
		m_CapacitorLevel = 0;

	if(hasWhat & 0x0800)
	{
		fromServer >> m_OwnerId;
		fromServer >> m_OwnerName;
		fromServer >> m_Roster;
		fromServer >> m_CryoBerths;
	}
	if(hasWhat & 0x1000)
	{
		// Last fired times for weapon bays.
		// Server only sends times to the client, not target atom ids.
		//qDebug() << "DaVessel::RxUpdate" << Id() << Name() << "bayFired cleared";
		m_BayFired.clear();
		quint8 bays;
		fromServer >> bays;
		quint16 which;
		for(int bay=0; bay<bays; bay++)
		{
			fromServer >> which;
			fromServer >> m_BayFired[which].m_When;
			fromServer >> m_BayFired[which].m_TractorState;
			if(m_BayFired[which].m_TractorState)
			{
				// Tractor beam sends the target atom for visual effect.
				fromServer >> m_BayFired[which].m_EmitLoc;
				fromServer >> m_BayFired[which].m_Target;
			}
			//qDebug() << "DaVessel::RxUpdate" << Id() << Name() << "bayFired" << m_BayFired[which].IsTractor();
		}
	}

	if(hasWhat & 0x0400)
		fromServer >> m_RepairSecs;
	else
		m_RepairSecs = 0;

	m_ShieldsLock.lockForWrite();
	if(hasWhat & 0x8000)
	{
		// Mixed shield capacitor values sent.
		for(int side=0; side<6; side++)
		{
			double s;
			fromServer >> s;
			if(s < 0)
			{
				m_ShieldCap[side] = -s;
				m_ShieldScram[side] = 1;
			}
			else
			{
				m_ShieldCap[side] = s;
				m_ShieldScram[side] = 0;
			}
		}
	}
	else if(hasWhat & 0x4000)
	{
		// All shield capacitors maxed out.
		double shieldMax = ShieldUnitsWorking();
		for(int side=0; side<6; side++)
		{
			m_ShieldCap[side] = shieldMax;
			m_ShieldScram[side] = 0;
		}
	}
	else
	{
		// All shield capacitors empty.
		for(int side=0; side<6; side++)
		{
			m_ShieldCap[side] = 0;
			m_ShieldScram[side] = 0;
		}
	}
	m_ShieldsLock.unlock();
	//qDebug("DaVessel::RxUpdate %d design %d", Id().GetID(), m_idDesign);

	// Receive another hasWhat bit mask because we used up the first one.
	quint8 hasWhat2;
	fromServer >> hasWhat2;

	// Receive the second wave of has-whats.
	if(hasWhat2 & 0x01)
	{
		// Active games at tables.
		// Clear my local data, if any.
		for(unordered_map<quint16, AuGame*>::iterator gi = m_Games.begin(); gi != m_Games.end(); gi++)
			delete (*gi).second;
		m_Games.clear();

		quint16 gameCount;
		fromServer >> gameCount;

		quint8 gameType;
		quint16 whichTable;
		AuGame* game;
		for(; gameCount; gameCount--)
		{
			fromServer >> whichTable;
			fromServer >> gameType;
			game = AuGame::CreateGame(gameType);
			if(game)
			{
				// The game object was created.
				game->Rx(fromServer);
				m_Games.insert(pair<quint16, AuGame*>(whichTable, game));
			}
			else
			{
				qWarning() << "Spacecraft" << Id() << Name() << "Game table" << whichTable << "received unknown game type"
					<< gameType << "Software update probably needed.";
				break;
			}
		}
	}

	if(hasWhat2 & 0x02)
	{
		fromServer >> m_Bank;
		fromServer >> m_BankSubsidyTax;
	}

	if(hasWhat2 & 0x04)
		m_Mission << fromServer;

	if(hasWhat2 & 0x08)
		fromServer >> m_WhereMade;

	if(hasWhat2 & 0x20)
		m_wActions |= ACT_Brake;	// Brakes are on.
	if(hasWhat2 & 0x40)
		m_wActions |= ACT_BrakeLateral;	// Brakes are on.
	if(hasWhat2 & 0x80)
		m_wActions |= ACT_BrakeVertical;	// Brakes are on.

	if(oldName != AssignedName() || oldEmpire != Allegiance())
	{
		m_NameWithEmpireLock.lockForWrite();
		m_NameWithEmpire.clear();
		m_NameWithEmpireLock.unlock();
	}
}

// Refresh data from valid counterpart.
bool DaVessel::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	QString oldName = AssignedName();
	quint32 oldEmpire = Allegiance();
	bool transformChanged = DaCapsule::Refresh(pAtom, ari);

	DaVessel* pA = (DaVessel*)pAtom;

	m_AcceptCrew = pA->m_AcceptCrew;
	m_AcceptPassengers = pA->m_AcceptPassengers;
	m_AcceptTroops = pA->m_AcceptTroops;
	m_AccessPolicy = pA->m_AccessPolicy;
	m_AllowPassengerSeaChests = pA->m_AllowPassengerSeaChests;
	m_AutoFetchVehicles = pA->m_AutoFetchVehicles;
	m_bExtLightOn = pA->m_bExtLightOn;
	m_bPowerPlantOn = pA->m_bPowerPlantOn;
	m_bSensorsOn = pA->m_bSensorsOn;
	m_byAccelSkewDur = pA->m_byAccelSkewDur;
	m_byPowerPlantSkewDur = pA->m_byPowerPlantSkewDur;
	m_byTurnRateSkewDur = pA->m_byTurnRateSkewDur;
	m_chAccelSkew = pA->m_chAccelSkew;
	m_chPowerPlantSkew = pA->m_chPowerPlantSkew;
	m_chTurnRateSkewX = pA->m_chTurnRateSkewX;
	m_chTurnRateSkewY = pA->m_chTurnRateSkewY;
	m_FuelQuality = pA->m_FuelQuality;
	m_IsPrivate = pA->m_IsPrivate;
	m_LockedIsCivilian = pA->m_LockedIsCivilian;
	m_LockedStance = pA->m_LockedStance;
	m_Moored = pA->m_Moored;
	m_PilotUpPolicy = pA->m_PilotUpPolicy;
	m_ShieldFuelPolicy = pA->m_ShieldFuelPolicy;
	m_ShieldPolicy = pA->m_ShieldPolicy;
	m_ShieldState = pA->m_ShieldState;

	pA->m_VesselLock.lockForRead();
	m_VesselLock.lockForWrite();
	m_CapacitorLevel = pA->m_CapacitorLevel;
	m_CapacitorTrend = pA->m_CapacitorTrend;
	m_eGravityDriveState = pA->m_eGravityDriveState;
	m_fFuelQuantity = pA->m_fFuelQuantity;
	m_LockedBlip = pA->m_LockedBlip;
	m_LockedSysLoc = pA->m_LockedSysLoc;
	m_Monster = pA->m_Monster;				// On client, this indicates a fuel emergency.
	m_RepairSecs = pA->m_RepairSecs;
	m_WhereMade = pA->m_WhereMade;
	m_VesselLock.unlock();
	pA->m_VesselLock.unlock();

	pA->m_AccountLock.lockForRead();
	m_AccountLock.lockForWrite();
	m_Bank = pA->m_Bank;
	m_BankSubsidyTax = pA->m_BankSubsidyTax;
	m_AccountLock.unlock();
	pA->m_AccountLock.unlock();

	pA->m_BayLock.lockForRead();
	m_BayLock.lockForWrite();
	m_BayFired = pA->m_BayFired;
	m_BayLock.unlock();
	pA->m_BayLock.unlock();

	pA->m_CargoLock.lockForRead();
	m_CargoLock.lockForWrite();
	m_Cargo.Clear();
	for(AuItemHash::const_iterator ii = pA->m_Cargo.begin(); ii != pA->m_Cargo.end(); ii++)
		m_Cargo.insert(pair<COMMODITYID, AuItem*>((*ii).first, (*ii).second->Clone()));
	m_CargoLock.unlock();
	pA->m_CargoLock.unlock();

	pA->m_ControlLock.lockForRead();
	m_ControlLock.lockForWrite();
	m_Throttle = pA->m_Throttle;
	m_ThrustLateral = pA->m_ThrustLateral;
	m_ThrustLong = pA->m_ThrustLong;
	m_ThrustVertical = pA->m_ThrustVertical;

	// Only the brake bits are sent to the client, for braking sound effects.
	m_wActions &= ~ACT_BrakeMask;					// Turn off all brake bits.
	m_wActions |= (pA->m_wActions&ACT_BrakeMask);	// Turn on the brake bits we received from pA.

	m_ControlLock.unlock();
	pA->m_ControlLock.unlock();

	pA->m_DamageLock.lockForRead();
	m_DamageLock.lockForWrite();
	m_Damage = pA->m_Damage;
	m_DamageLock.unlock();
	pA->m_DamageLock.unlock();

	pA->m_LastInRangeLock.lockForRead();
	m_LastInRangeLock.lockForWrite();
	m_LastInEnemyRange = pA->m_LastInEnemyRange;
	m_LastInRangeLock.unlock();
	pA->m_LastInRangeLock.unlock();

	pA->m_MissionLock.lockForRead();
	m_MissionLock.lockForWrite();
	m_Mission = pA->m_Mission;
	m_MissionLock.unlock();
	pA->m_MissionLock.unlock();

	pA->m_SystemsLock.lockForRead();
	m_SystemsLock.lockForWrite();
	m_ssaShipSystems = pA->m_ssaShipSystems;	// CLIENT ONLY RECEIVES A SUBSET OF THIS DATA.
	m_SystemsLock.unlock();
	pA->m_SystemsLock.unlock();

	pA->m_RosterLock.lockForRead();
	m_RosterLock.lockForWrite();
	m_OwnerId = pA->m_OwnerId;
	m_OwnerName = pA->m_OwnerName;
	m_Roster = pA->m_Roster;
	m_CryoBerths = pA->m_CryoBerths;
	m_RosterLock.unlock();
	pA->m_RosterLock.unlock();

	pA->m_ShieldsLock.lockForRead();
	m_ShieldsLock.lockForWrite();
	for(int side=0; side<6; side++)
	{
		m_ShieldCap[side] = pA->m_ShieldCap[side];
		m_ShieldScram[side] = pA->m_ShieldScram[side];
	}
	m_ShieldsLock.unlock();
	pA->m_ShieldsLock.unlock();

	pA->m_WaypointLock.lockForRead();
	m_WaypointLock.lockForWrite();
	m_aWaypoints = pA->m_aWaypoints;
	m_WaypointLock.unlock();
	pA->m_WaypointLock.unlock();

	SetVector(pA->Vector());

	// Games at tables
	// We refresh games carefully or they lose local render animation information.
	{
		pA->m_GamesLock.lockForRead();
		m_GamesLock.lockForWrite();

		// Take a copy of my current games.
		unordered_map<quint16, AuGame*> oldGames(m_Games);
		m_Games.clear();

		// Update my local cames with the ones that just arrived.
		for(unordered_map<quint16, AuGame*>::const_iterator gi = pA->m_Games.begin(); gi != pA->m_Games.end(); gi++)
		{
			unordered_map<quint16, AuGame*>::iterator ogi = oldGames.find((*gi).first);
			if(ogi != oldGames.end())
			{
				// The old game list contains a game at the same table. Is it the same kind of game?
				if((*ogi).second->Type() == (*gi).second->Type())
				{
					// They are the same type of game.
					(*ogi).second->Refresh((*gi).second);				// Update the local game with fresh data.
					m_Games.insert(pair<quint16, AuGame*>((*gi).first, (*ogi).second));	// Restore the game to the current games hash
					oldGames.erase(ogi);				// Remove the game from the old games hash.
					continue;
				}
				else
				{
					// The old game type is different. Remove it.
					delete (*ogi).second;
					oldGames.erase(ogi);
				}
			}

			// Duplicate the game that has arrived and store it in my games hash.
			AuGame* game = (*gi).second->Clone();
			m_Games.insert(pair<quint16, AuGame*>((*gi).first, game));
		}

		// Now, any games left in the oldGames hash can be deleted.
		for(unordered_map<quint16, AuGame*>::iterator ogi = oldGames.begin(); ogi != oldGames.end(); ogi++)
			delete (*ogi).second;

		m_GamesLock.unlock();
		pA->m_GamesLock.unlock();
	}

	//qDebug("DaVessel::Refresh %x %d design %s", (unsigned int)this,
			//Id().GetID(), pA->m_DesignValid?"valid":"INVALID");

	// Shields balance not sent to client. Client needs to be manning a shield console to rx the info.

	if(oldName != AssignedName() || oldEmpire != Allegiance())
	{
		m_NameWithEmpireLock.lockForWrite();
		m_NameWithEmpire.clear();
		m_NameWithEmpireLock.unlock();
	}

	return transformChanged;
}

bool DaVessel::IsStandingDown() const
{
	//qDebug() << "DaVessel::IsStandingDown" << Id() << Name();
	// Stand down order must be active to enable cargo transfer.
	if(!IsOnAMission() || MissionOrderType() != AuMissionOrder::StandDown)
	{
		//qDebug() << "mission" << IsOnAMission() << MissionOrderType();
		return false;										// Mission not active or current order not stand down.
	}

	// Pilot must hold position.
	if(m_Throttle || m_ThrustLateral || m_ThrustLong || m_ThrustVertical)
	{
		//qDebug() << "throttle" << m_Throttle << m_ThrustLateral << m_ThrustLong << m_ThrustVertical;
		return false;										// Throttles on.
	}
	if(GravityDriveState() != GD_Hold && (m_wActions & (ACT_MoveMask|ACT_TurnMask|ACT_PitchMask|ACT_RollMask)))
	{
		//qDebug() << "actions" << m_wActions;
		return false;										// Movement actions active.
	}

	// Shields must be down.
	if(ShieldsWorkingAndUp())
	{
		//qDebug() << "shields" << ShieldsWorkingAndUp();
		return false;										// Shields up and on.
	}

	// Sensors must not lock on to anything.
	/*
	if(!LockedBlip().IsNull())
	{
		//qDebug() << "locked" << LockedBlip();
		return false;										// Sensors locked on.
	}
	*/

	// Weapons must hold fire.
	m_BayLock.lockForRead();
	bool be = m_BayFired.empty();
	m_BayLock.unlock();
	if(!be)
		return false;										// Weapon has fired.

	// The ship appears to be standing down.
	//qDebug() << "standing down";
	return true;
}

quint16 DaVessel::ReasonNotStandingDown() const
{
	//qDebug() << "DaVessel::IsStandingDown" << Id() << Name();
	quint16 reasonBits = 0x0000;

	// Stand down order must be active to enable cargo transfer.
	if(!IsOnAMission() || MissionOrderType() != AuMissionOrder::StandDown)
	{
		//qDebug() << "mission" << IsOnAMission() << MissionOrderType();
		reasonBits |= 0x0001;								// Mission not active or current order not stand down.
	}

	// Pilot must hold position.
	if(m_Throttle || m_ThrustLateral || m_ThrustLong || m_ThrustVertical)
	{
		//qDebug() << "throttle" << m_Throttle << m_ThrustLateral << m_ThrustLong << m_ThrustVertical;
		reasonBits |= 0x0004;								// Throttles on.
	}
	if(GravityDriveState() != GD_Hold && (m_wActions & (ACT_MoveMask|ACT_TurnMask|ACT_PitchMask|ACT_RollMask)))
	{
		//qDebug() << "actions" << m_wActions;
		reasonBits |= 0x0008;								// Movement actions active.
	}

	// Shields must be down.
	if(ShieldsWorkingAndUp())
	{
		//qDebug() << "shields" << ShieldsWorkingAndUp();
		reasonBits |= 0x0020;								// Shields up and on.
	}

	// Sensors must not lock on to anything.
	/*
	if(!LockedBlip().IsNull())
	{
		//qDebug() << "locked" << LockedBlip();
		reasonBits |= 0x0040;								// Sensors locked on.
	}
	*/

	// Weapons must hold fire.
	m_BayLock.lockForRead();
	if(!m_BayFired.empty())
		reasonBits |= 0x0080;								// Weapon has fired.
	m_BayLock.unlock();

	// The ship appears to be standing down.
	//qDebug() << "standing down";
	return reasonBits;
}

// Locates an unobstructed spot within trade range to park 'parkShip'.
bool DaVessel::ParkingSpotInRange(const DaVessel* parkShip, AuPoint& parkHere) const
{
	// Determine parking range to get them within trade range.
	double parkRange = StationParkingRange(parkShip) - 10*FeetPerMeter();

	// Start with the parking ship's location relative to me and move it to trade range, so they might not have to maneuver around.
	AuPoint parkShipLoc = EnvTransform().UnProject(parkShip->EnvLocation());
	parkShipLoc.z = 0;											// Start in the XY plane so our spinning around algo works.
	parkShipLoc.Unit(parkRange);

	// Move around me until we find a spot that is far enough from other vessels.
	const double circumference = parkRange * 2 * PI;			// parkRange is arc radius.
	const double segs = circumference / (100. * FeetPerMeter());	// Number of 100m arc segments.
	const double segAng = (PI*2) / segs;						// Angle per arc segment to move 100m.
	const double mindist = parkShip->RoughRadius() + 10*FeetPerMeter();	// min dist required from existing atoms

	const DaWorld* world = dynamic_cast<const DaWorld*>(Parent());
	if(world)
	{
		// Rotate around me in my Z plane to find a clear park location.
		parkHere = parkShipLoc;									// Start location relative to this ship.
		int tries = segs;										// Prevent infinite loop. Could happen when crowded.
		while(world->ClosestShipInOrbit(Transform().Project(parkHere), parkShip->Id(), false) < mindist && tries)
		{
			parkHere = parkHere.RotateZ(segAng);
			tries--;
		}
		if(tries)
			return true;	// Found a park location.

		// Failed to find a clear park location rotating around Z; try again rotating around Y.
		parkHere = parkShipLoc;									// Start location relative to the planet or ringworld.
		tries = segs;											// Prevent infinite loop. Could happen when crowded.
		while(world->ClosestShipInOrbit(Transform().Project(parkHere), parkShip->Id(), false) < mindist && tries)
		{
			parkHere = parkHere.RotateY(segAng);
			tries--;
		}
		if(tries)
			return true;	// Found a park location.

		// Failed to find a clear park location rotating around Z and Y; try again rotating around X.
		parkHere = parkShipLoc;									// Start location relative to the planet or ringworld.
		tries = segs;											// Prevent infinite loop. Could happen when crowded.
		while(world->ClosestShipInOrbit(Transform().Project(parkHere), parkShip->Id(), false) < mindist && tries)
		{
			parkHere = parkHere.RotateX(segAng);
			tries--;
		}
		if(tries)
			return true;	// Found a park location.

		return false;
	}
	else
	{
		// Not at a world.
		const DaAtom* envAtom = FindEnvironmentAtom();
		DaAtomList atoms;
		envAtom->FindVesselsRecursive(atoms);

		// Rotate around me in my Z plane to find a clear park location.
		parkHere = parkShipLoc;										// Start location relative to this ship.
		int tries = segs;											// Prevent infinite loop. Could happen when crowded.
		while(ClosestShipInSystem(atoms, parkHere, parkShip->Id()) < mindist && tries)
		{
			parkHere = parkHere.RotateZ(segAng);
			tries--;
		}
		if(tries)
			return true;	// Found a park location.

		// Failed to find a clear park location rotating around Z; try again rotating around Y.
		parkHere = parkShipLoc;									// Start location relative to the planet or ringworld.
		tries = segs;											// Prevent infinite loop. Could happen when crowded.
		while(ClosestShipInSystem(atoms, parkHere, parkShip->Id()) < mindist && tries)
		{
			parkHere = parkHere.RotateY(segAng);
			tries--;
		}
		if(tries)
			return true;	// Found a park location.

		// Failed to find a clear park location rotating around Z and Y; try again rotating around X.
		parkHere = parkShipLoc;									// Start location relative to the planet or ringworld.
		tries = segs;											// Prevent infinite loop. Could happen when crowded.
		while(ClosestShipInSystem(atoms, parkHere, parkShip->Id()) < mindist && tries)
		{
			parkHere = parkHere.RotateX(segAng);
			tries--;
		}
		if(tries)
			return true;	// Found a park location.

		return false;
	}
}

// Return destination atom and offset based on ship's current mission order.
// 'destOffset' is a return parameter that specifies a destination location relative to the destination atom.
const DaAtom* DaVessel::MissionDestination(AuMissionOrder::TYPE& destOrder, AuPoint& destOffset) const
{
	// Determine the destination atom from the current mission order.
	const DaAtom* dest = 0;
	m_MissionLock.lockForRead();
	const AuMissionOrder& order = m_Mission.CurrentOrder();
	destOrder = order.Type();
	switch(destOrder)
	{
	case AuMissionOrder::CargoBuyCategory:
	case AuMissionOrder::CargoBuyCommodity:
	case AuMissionOrder::CargoSellCategory:
	case AuMissionOrder::CargoSellCommodity:
	case AuMissionOrder::Christen:
	case AuMissionOrder::CrewLoad:
	case AuMissionOrder::CryoBerthsLoad:
	case AuMissionOrder::CryoBerthsUnload:
	case AuMissionOrder::DamageReport:
	case AuMissionOrder::ExterminateRats:
	case AuMissionOrder::Harvest:
	case AuMissionOrder::HarvestAtmosphere:
	case AuMissionOrder::HarvestMinerals:
	case AuMissionOrder::HarvestOperationByBay:
	case AuMissionOrder::HarvestOperationByModule:
	case AuMissionOrder::HarvestParticles:
	case AuMissionOrder::HarvestSea:
	case AuMissionOrder::HarvestWell:
	case AuMissionOrder::Hide:
	case AuMissionOrder::HoldMission:
	case AuMissionOrder::LightState:
	case AuMissionOrder::PassengersLoad:
	case AuMissionOrder::PassengersUnload:
	case AuMissionOrder::PilotPolicy:
	case AuMissionOrder::Posture:
	case AuMissionOrder::Repair:
	case AuMissionOrder::Report:
	case AuMissionOrder::ScanSurveyPlanet:
	case AuMissionOrder::ScanSurveySystem:
	case AuMissionOrder::ScanTactical:
	case AuMissionOrder::Scuttle:
	case AuMissionOrder::ShieldPolicy:
	case AuMissionOrder::ShieldsDown:
	case AuMissionOrder::StandDown:
	case AuMissionOrder::Stay:
	case AuMissionOrder::TransferCargoCategoryFrom:
	case AuMissionOrder::TransferCargoCategoryTo:
	case AuMissionOrder::TransferCargoFrom:
	case AuMissionOrder::TransferCargoTo:
	case AuMissionOrder::TransferMoneyFrom:
	case AuMissionOrder::TransferMoneyTo:
	case AuMissionOrder::TransferPeopleFrom:
	case AuMissionOrder::TransferPeopleTo:
	case AuMissionOrder::TroopsLoad:
	case AuMissionOrder::Invalid:
	case AuMissionOrder::OrderTypes:
		// These orders don't require the ship to move to a destination.
		destOffset = AuPoint(0,0);
		break;

	case AuMissionOrder::GoToCity:
	case AuMissionOrder::GoToCityAnywhere:
		destOffset = AuPoint(0,0);
		dest = MissionDestGoToCity(order);
		break;

	case AuMissionOrder::GoToStation:
		destOffset = AuPoint(0,0);
		dest = MissionDestGoToStation(order);
		break;

	case AuMissionOrder::GoToBestCity:
		//destOffset = AuPoint(0,0);
		//dest = MissionDestGoToBestCity();
		break;

	case AuMissionOrder::GoToBestStation:
		destOffset = AuPoint(0,0);
		dest = MissionDestGoToBestStation();
		break;

	case AuMissionOrder::GoToBestStationOrCity:
		//destOffset = AuPoint(0,0);
		//dest = MissionDestGoToBestCityOrStation();
		break;

	case AuMissionOrder::GoToCoordinate:
		dest = MissionDestGoToCoordinate(order, destOffset);
		break;

	case AuMissionOrder::GoToSystemDirect:
		dest = MissionDestGoToSystem(order);
		if(!dest)
		{
			dest = MissionDestGoToCoordinate(order, destOffset);
			if(dest)
				destOrder = AuMissionOrder::GoToCoordinate;
		}
		else
			destOffset = AuPoint(0,0);
		break;

	case AuMissionOrder::GoToSystemDirect_Deprecated:
	case AuMissionOrder::GoToSystemViaWormholes:
		destOffset = AuPoint(0,0);
		dest = MissionDestGoToSystem(order);
		if(!dest)
			dest = MissionDestGoToGalaxy(order);	// Try intergalactic wormhole.
		break;

	case AuMissionOrder::PatrolCities:
		destOffset = AuPoint(0,0);
		dest = MissionDestPatrolCities();
		break;

	case AuMissionOrder::PatrolStations:
		destOffset = AuPoint(0,0);
		dest = MissionDestPatrolStations();
		break;

	case AuMissionOrder::PatrolSystem:
		dest = MissionDestPatrolPlanets(destOffset);
		break;

	case AuMissionOrder::PatrolWormholes:
		dest = MissionDestPatrolWormholes(destOffset);
		break;

	case AuMissionOrder::GoToClosestCity:
		destOffset = AuPoint(0,0);
		dest = MissionDestGoToClosestCity();
		break;

	case AuMissionOrder::GoToClosestStation:
		destOffset = AuPoint(0,0);
		dest = MissionDestGoToClosestStation();
		break;

	case AuMissionOrder::GoToClosestStationOrCity:
		destOffset = AuPoint(0,0);
		dest = MissionDestGoToClosestStationOrCity();
		break;

	case AuMissionOrder::GoToOrbitBestPlanet:
		dest = MissionDestGoToOrbitBestPlanet(destOffset);
		break;

	case AuMissionOrder::GoToHoverSite:
	case AuMissionOrder::GoToLand:								// Any landing site, preferably in daylight.
	case AuMissionOrder::GoToLandingSite:
	case AuMissionOrder::GoToOrbit:
	case AuMissionOrder::GoToOrbitCity:							// Destination is a planet that hosts a city.
	case AuMissionOrder::GoToOrbitCoords:
		dest = MissionDestGoToOrbitHoverLand(order, destOffset);
		break;

	case AuMissionOrder::Attack:
	case AuMissionOrder::Plunder:
	case AuMissionOrder::Follow:
		destOffset = AuPoint(0,0);
		dest = MissionDestAttackFollow(order);
		break;
	}

	if(dest && order.IsTravelOrder())
	{
		// We are traveling to a destination that may be on a distant world.
		// Target the world that hosts the destination, so we can catch the ones riding on fast orbiting planetoids.
		const DaWorldLL* world = dest->FindParentWorldInEnv();	// Returns 'dest' if it IS a world.
		if(world && !world->Is(ParentId()) && world->IsInAsteroidBelt())
		{
			// Destination is on a world in an asteroid belt; the world could be the destination.
			AuMissionOrderGoToOrbit enrouteOrder(world->Id(), world->Name());
			dest = MissionDestGoToOrbitHoverLand(enrouteOrder, destOffset);
		}
	}
	m_MissionLock.unlock();
	return dest;
}

void DaVessel::MissionOrderChanged(const AuMissionOrder& prevOrder, bool notifyOfficer)
{
	m_MissionLock.lockForWrite();
	m_tmMissionChange.Set();

	//if(IsPirate())
		//qDebug() << "DaVessel::MissionOrderChanged" << Id() << Name()
			//<< "prevOrder" << prevOrder.ListName() << prevOrder.ListDescription()
			//<< "curOrder" << m_Mission.CurrentOrder().ListName() << m_Mission.CurrentOrder().ListDescription();

	// Avoid infinite loop caused by one single order, or the same order repeated, in a repeating mission.
	if(m_Mission.Type() == AuMission::Repeat && prevOrder.IsSame(m_Mission.CurrentOrder()))
	{
		m_MissionLock.unlock();
		SetModified();
		return;
	}
	m_MissionLock.unlock();

	DaSentient* officer;
	if(notifyOfficer)
	{
		// Notify commanding officer that the mission order has changed.
		officer = FindCommandingOfficer();			// Find officer who has the con. This could be null.
		if(officer && !officer->MissionOrderChanged(this, prevOrder))
		{
			// The officer doesn't want to accept the order. This happens when the nu order is a repair order and the ship does
			// not have any damage.
			if(MissionOrderType() == AuMissionOrder::Repair)
				MissionOrderComplete(officer, 2);
			//else if(MissionOrderType() == AuMissionOrder::ScanSurveySystem)
				//MissionOrderComplete(officer, 15);
			else
				MissionOrderComplete(officer, 99);
			SetModified();
			return;
		}
	}
	else
		officer = 0;

	// This static IsStarTravelOrder call will treat all GoToCoordinate orders as star travel. That's ok for our purposes here.
	m_MissionLock.lockForRead();
	if(prevOrder.IsStarTravelOrder() && !m_Mission.CurrentOrder().IsStarTravelOrder())
	{
		// Previous order was a travel order and current order is not a travel order. Don't let the warp drive stay hot.
		if(IsEngineerMannedByAI() && IsAnyFTLDriveHot())
			TurnAllFTLDrivesOff();				// Engineer turns off warp FTL drive when not traveling somewhere.ll
	}

	// Some orders require immediate attention by a crewman.
	if(m_Mission.CurrentOrder().Type() == AuMissionOrder::Scuttle)
	{
		if(m_Mission.IsActive())
			SetAlertCondition(ConditionRed);	// Red alert when scuttle order becomes the active order.
	}
	else if(m_Mission.CurrentOrder().Type() == AuMissionOrder::ScanSurveyPlanet)
	{
		// Sensor operator needs to make a scan report.
		DaSentient* sensorOp = AISensorOperator();
		if(sensorOp && sensorOp->Order() != ORDER_SurveyScanPlanet)
			sensorOp->AmendOrder(ORDER_SurveyScanPlanet, Id(), sensorOp->Location(), 0);
	}
	else if(m_Mission.CurrentOrder().Type() == AuMissionOrder::ScanSurveySystem)
	{
		const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(FindEnvironmentAtom());
		if(system && !system->IsScannedBy(EmpireId()))
		{
			// Sensor operator needs to make a scan report.
			DaSentient* sensorOp = AISensorOperator();
			if(sensorOp && sensorOp->Order() != ORDER_SurveyScanSystem)
				sensorOp->AmendOrder(ORDER_SurveyScanSystem, Id(), sensorOp->Location(), 0);
		}
	}
	else if(m_Mission.CurrentOrder().Type() == AuMissionOrder::ScanTactical)
	{
		// Sensor operator needs to make a scan report.
		DaSentient* sensorOp = AISensorOperator();
		if(sensorOp && sensorOp->Order() != ORDER_TacticalScan)
			sensorOp->AmendOrder(ORDER_TacticalScan, Id(), sensorOp->Location(), 0);
	}
	else if(m_Mission.CurrentOrder().Type() == AuMissionOrder::DamageReport)
	{
		// Engineer needs to make a damage report.
		DaSentient* engineOp = AIEngineOperator();
		if(engineOp)
		{
			if(engineOp->Order() != ORDER_DamageReport)
				engineOp->AmendOrder(ORDER_DamageReport, Id(), engineOp->Location(), 0);
		}
		else
		{
			// No engineer. How about a power relay operator.
			DaSentient* powerOp = AIPowerRelayOperator();
			if(powerOp)
			{
				if(powerOp->Order() != ORDER_DamageReport)
					powerOp->AmendOrder(ORDER_DamageReport, Id(), powerOp->Location(), 0);
			}
			else if(officer)	// Last resort is officer in command.
			{
				if(officer->Order() != ORDER_DamageReport)
					officer->AmendOrder(ORDER_DamageReport, Id(), officer->Location(), 0);
			}
		}
	}
	m_MissionLock.unlock();
	SetModified();
}

// 'commandingOfficer' may be null, in which case there is no notification of the completed order.
// 'completionCode' is used by the officer when announcing the result.
void DaVessel::MissionOrderComplete(DaSentient* commandingOfficer, int completionCode)
{
	// Commanding officer announces completion of the mission order on the crew channel.
	if(commandingOfficer)
		commandingOfficer->AnnounceMissionOrderComplete(this, completionCode);

	m_MissionLock.lockForWrite();

	// If previous order was a travel order, clear the waypoint route.
	if(HasWaypoint() && m_Mission.CurrentOrder().IsTravelOrder())
		if(IsNavigatorMannedByAI())
			RemoveAllWaypoints();							// Navigator clears the current route.

	// Select the next mission order.
	AuMissionOrder* prevOrder = m_Mission.CurrentOrder().Clone();
	bool did = m_Mission.SelectNextOrder();
	m_MissionLock.unlock();

	if(did)
		MissionOrderChanged(*prevOrder);					// Notify commander/crew that the mission order changed.

	delete prevOrder;
}

DaVessel* DaVessel::MissionOrderTransferTarget(const AuId& officer) const
{
	// The mission order target name is blank when we are supposed to find a station in trade range.
	QString targetName = MissionOrderTargetName();

	// Locate the transfer target ship.
	DaVessel* targetShip;
	if(targetName.isEmpty())
	{
		// Find a station in trade range.
		bool hasStation;
		targetShip = StationInTradeRange(RoughRadius(), hasStation);
	}
	else
	{
		// Find the named ship and insure it is in trade range.
		targetShip = 0;
		const DaAtom* system = FindEnvironmentAtom();
		if(system)
		{
			// We'll look in the entire solar system.
			AuPoint myLoc = EnvLocation();
			DaAtomList atoms;
			system->FindVesselsRecursive(atoms);
			for(DaAtomList::const_iterator it = atoms.begin(); it != atoms.end(); it++)
			{
				DaVessel* ss = dynamic_cast<DaVessel*>(*it);
				if(!ss)
					continue;	// Not likely due to FindVesselRecursive.
				if(ss == this)
					continue;	// Skip myself.
				if(ss->Name() != targetName)
					continue;
				if(PoliticalStance(ss->EmpireId(), EmpireId()) == POL_Enemy)
					continue;	// Skip enemy.

				AuPoint stationLoc = ss->EnvLocation();
				if(myLoc.Distance(stationLoc) < TransferRange(ss))
					targetShip = ss;
				break;
			}
		}
	}

	// Was a target ship found?
	if(!targetShip)
		return 0;

	// Test authority to perform the transfer.
	if(!targetShip->InChainOfCommand(officer, EmpireId()) &&
			!targetShip->InChainOfCommand(MissionOrderAuthorization(), EmpireId()))
		return 0;

	// Target ship found in trade range and it grants permission for the transfer.
	return targetShip;
}

void DaVessel::Retreat()
{
	// Locate a different solar system as an escape destination.
	const DaSolarSystem* curSystem = dynamic_cast<const DaSolarSystem*>(FindEnvironmentAtom());
	const DaSolarSystem* destSystem = 0;
	if(curSystem)
	{
		const DaSector* sector = FindParentSector();
		if(sector)
		{
			DaAtomList subs;
			sector->ListSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(*it);
				if(system && system != curSystem)
				{
					destSystem = system;
					break;
				}
			}

			if(!destSystem)
			{
				// I am in a sector with only one solar system.
				// Find a solar system in an adjacent sector.
				const DaGalaxy* galaxy = sector->FindParentGalaxy();
				if(galaxy)
				{
					AuPoint nbloc = sector->Location() +
						AuPoint(g_Au.Dice().Roll(1,3,-2)*10, g_Au.Dice().Roll(1,3,-2)*10, g_Au.Dice().Roll(1,3,-2)*10);
					sector = galaxy->FindSectorAt(nbloc);
					if(sector)
					{
						DaAtomList subs;
						sector->ListSubatoms(subs);
						for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
						{
							const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(*it);
							if(system)
							{
								destSystem = system;
								break;
							}
						}
					}
				}
			}
		}
	}

	// Do not hunt while fleeing.
	SetCrewPosture(POSTURE_DefendSelf);

	if(destSystem)
		MissionAppendOrder(AuMissionOrderGoToSystemDirect(destSystem->Id(), destSystem->Name(), destSystem->GalaxyLoc()));
	else
	{
		qWarning() << "warning: DaVessel::Retreat" << *this << "no system to retreat to, scuttling";
		MissionScuttle(3 * 60);				// No need to stick around.
	}
	SetModified();
}

bool DaVessel::CheckSubatomsOkToSpawnMonster() const
{
	bool personOnLineFound = false;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		// No encounter if there's already a wild animal aboard.
		const DaAnimal* animal = dynamic_cast<const DaAnimal*>(*it);
		if(animal)
		{
			if(!animal->IsWild())
				continue;
			if(!animal->IsInSpacecraft())
				continue;
			return false;									// Found wild animal aboard.
		}

		// No monster encounter if no people are a board to notice.
		if(!personOnLineFound)
		{
			const DaPerson* person = dynamic_cast<const DaPerson*>(*it);
			if(person && person->IsOnLine() && person->IsInSpacecraft())
				personOnLineFound = true;					// Found a person on line who is aboard.
		}
	}
	return personOnLineFound;
}

// Makes a random check to see if there is a monster encounter. Calls MonsterEncounterNow if an encounter happens.
void DaVessel::MonsterEncounter()
{
	if(!g_Au.m_EnableAnimalEncounters)
		return;

	if(!IsInAnyScene())
		return;

	// Already checked in Tock.
	//if(InArena())
		//return;

	quint16 monster = Monster();
	if(monster == 0)
		return;	// No probability of a monster.
	if(g_Au.Dice().Roll(1,3000) >= monster)
		return;	// No monster encounter this time.

	// No encounter if there's already a wild animal aboard or if no people on line are aboard.
	if(!CheckSubatomsOkToSpawnMonster())
		return;

	// Spawn one or more monsters.
	MonsterEncounterNow();

	// Having a monster encounter resets the monster probability so it can start accumulating again.
	ResetMonster();
}

// Creates a monster encounter.
int DaVessel::MonsterEncounterNow()
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	// Find a place to put a monster.
	AuPoint voidLoc;
	if(!TransporterVoidLocation(voidLoc))
	{
		//qDebug() << "DaVessel::MonsterEncounterNow" << Id() << Name() << "no void location to spawn" << Monster();
		return 0;												// No place to spawn a monster? Not likely.
	}

	// Create random monster DNA.
	quint8 monsterQ;
	if(QualityMade() > QualityOfDesign())
		monsterQ = QualityMade();
	else
		monsterQ = QualityOfDesign();
	DhDNA monsterDNA;	//DHH animalia
	unsigned int maxSize = monsterQ / 8;
	monsterDNA.RandomMonster(maxSize, g_Au.Dice());	// Create DNA for our monster.

	// Determine how many/much monsters might spawn.
	int monstersMax = 0;				// Maximum number of monsters that can spawn. To be determined.
	int monsterHitsMax = 0;				// Maximum amount of hit points worth of monsters to spawn. To be determined.
	int monsters = 0;
	int monsterHp = 0;

	// Spawn monster(s).
	do
	{
		// Create the animal so we can get physical dimensions.
		AuDNABase::CHROMATID chroma = (AuDNABase::CHROMATID)g_Au.Dice().Roll(1,2,-1);
		HANDEDNESS hand = (HANDEDNESS)g_Au.Dice().Roll(1,2,-1);
		DaAnimal* pan = new DaAnimal(monsterDNA.Species(), chroma, hand, Mature);
		pan->ConnectSignals();
		pan->Create();

		// Stand to establish ground height; clears NeedsInit state.
		pan->Stand();

		// Move spawnLoc up to the desired altitude.
		AuPoint spawnLoc(voidLoc);
		spawnLoc.z += pan->GroundHeightAboveCG();

		// Position the animal in the environment.
		// Move to location in terrain.
		AuMatrix4x4 t;
		t.Translate(spawnLoc);

		// Rotate the critter randomly on its z axis.
		double rot = g_Au.Dice().Roll(1,360,-1) * DEG2RAD;
		t.RotateZ(rot);
		pan->SetPosAndOrient(AuAtomPos(Id(), t));
		ether->AddAtom(pan);

		pan->SetGeneralPose(AuPose::Stand);
		pan->ResetHitPoints();			// Now that it has a body.
		pan->ResetStamina();
		pan->SetInScene();

		// Monster are spawned hungry.
		pan->SetHunger(1);

		g_Au.AtomAdded(pan->Id());
		pan->SetNormal();				// New atom is ready for normal thread processing.
		pan->TickTock();

		if(monstersMax == 0)
		{
			int potentialMonsters = monsterQ/8/7 + 1;
			monstersMax = g_Au.Dice().Roll(2, potentialMonsters, -1);
			monsterHitsMax = monstersMax * 1000;
		}

		monsters++;
		monsterHp += pan->MaxHitPoints();
		//qDebug() << "DaVessel::MonsterEncounterNow" << Id() << Name() << "monster" << pan->Id() << pan->Name()
			//<< "hp" << pan->MaxHitPoints() << "in" << pan->ShipRoomName();
	}
	while(monsters < monstersMax && monsterHp < monsterHitsMax);
	//qDebug() << "DaVessel::MonsterEncounterNow" << Id() << Name() << "monster" << Monster()
		//<< "potential" << (monsterQ/8/7 + 1) << "max" << monstersMax << "maxHp" << monsterHitsMax << "spawned" << monsters
		//<< "spawnedHp" << monsterHp << "DNA" << monsterDNA;
	return monsters;
}

// Returns true if this is a pirate owned ship that does not have a human captain.
// Returns true if this is an AI controlled ship in an arena.
bool DaVessel::IsPirateWithFreeWill() const
{
	if(InArena() && ArenaTeam() == DgEmpire::ArenaTeam2Id())
	{
		// I might be on an AI controlled team, depending on match type.
		const DaArena* arena = FindParentArena();
		if(arena && arena->MatchType() == DaArena::SpacecraftTrial)
			return true;
	}

	// So far, only an agent can be the captain of a pirate ship.
	return (IsPirate() && CaptainId().IsNull());
}

// 'whichBay' is the system which identifier of the turret to load
bool DaVessel::SelectBayWeaponAmmunition(const AuItem& item, quint16 whichBay)
{
	if(!CanSelectBayWeaponAmmunition(item, whichBay))
		return false;

	// The bay simply selects the ammo to use. No ammo is consumed until fired.
	m_BayLock.lockForWrite();
	unordered_map<quint16, AuBaySetting>::iterator bi = m_BaySetting.find(whichBay);
	if(bi != m_BaySetting.end())
		(*bi).second.Load(item);	// This preserves the power setting.
	else
		m_BaySetting.insert(pair<quint16, AuBaySetting>(whichBay, AuBaySetting(item)));
	m_BayLock.unlock();
	SetModified(NOTIFY_Database);
	return true;
}

// Selects weapon to fire from bay. Picks nu ordnance at random if current ordnance is gone.
// Does nothing if current ordnance is available.
// Returns true if the weapon bay selection is changed.
// Returns false if 'which' bay is not configured with a weapon bay module.
bool DaVessel::SelectBayWeapon(quint16 which)
{
	// Ammo can only be selected by weapon bay.
	COMMODITYID bayCID = SystemModule(SYS_Weapon, which);
	if(!DbCommodity::IsBayWeaponModule(bayCID))
		return false;

	m_BayLock.lockForWrite();
	unordered_map<quint16, AuBaySetting>::iterator bi = m_BaySetting.find(which);
	if(bi != m_BaySetting.end() && (*bi).second.CommodityId() != CID_Invalid)
	{
		// Ammunition is selected at that bay position.
		// Locate the exact ammunition in the hold.
		m_CargoLock.lockForRead();
		AuItemHash::iterator ci = m_Cargo.Find(*(*bi).second.Ammo());
		if(ci != m_Cargo.end())
		{
			m_CargoLock.unlock();
			m_BayLock.unlock();
			return false;	// We have ammo for that weapon.
		}

		// Locate similar ammunition.
		ci = m_Cargo.FindBest((*bi).second.CommodityId());
		if(ci != m_Cargo.end())
		{
			// We have similar ammo.
			(*bi).second.Load(*(*ci).second);
			m_CargoLock.unlock();
			m_BayLock.unlock();
			SetModified(NOTIFY_Database);
			return true;
		}
		m_CargoLock.unlock();
	}

	// Nothing already loaded in that bay weapon or the hold contains no more ammo for it.
	// We don't automatically replace an energy weapon due to low ammo.
	// List ammo usable by bay weapons, in order of preference.
	QList<COMMODITYID> ammoNeeded;
	ammoNeeded.append(CID_CompressorBeam);
	ammoNeeded.append(CID_DisintegratorRay);
	ammoNeeded.append(CID_ProtonBeam);
	ammoNeeded.append(CID_StasisBeam);
	ammoNeeded.append(CID_DisruptorBeam);
	ammoNeeded.append(CID_ErgRay);
	ammoNeeded.append(CID_HeatRay);
	ammoNeeded.append(CID_FreezeRay);
	ammoNeeded.append(CID_NeutronBeam);
	ammoNeeded.append(CID_ElectronBeam);
	ammoNeeded.append(CID_LargeMissileSlPo);
	ammoNeeded.append(CID_LargeMissileBlBu);
	ammoNeeded.append(CID_LargeMissileBl);
	ammoNeeded.append(CID_LargeRocketBuDi);
	ammoNeeded.append(CID_LargeRocketBlBu);
	ammoNeeded.append(CID_LargeRocketBl);
	ammoNeeded.append(CID_HeavyArmsAmmoPiRa);
	ammoNeeded.append(CID_HeavyArmsAmmoPiAc);
	ammoNeeded.append(CID_HeavyArmsAmmoPiBl);
	ammoNeeded.append(CID_HeavyArmsAmmoPi);
	for(QList<COMMODITYID>::const_iterator mi = ammoNeeded.cbegin(); mi != ammoNeeded.cend(); mi++)
	{
		if(!DbCommodity::IsAmmunitionForWeapon(bayCID, *mi))
			continue;

		m_CargoLock.lockForRead();
		AuItemHash::const_iterator ci = m_Cargo.FindBest(*mi);
		if(ci != m_Cargo.end())
		{
			// We have the ammo.
			if(bi != m_BaySetting.end())
				(*bi).second.Load(*(*ci).second);
			else
				m_BaySetting.insert(pair<quint16, AuBaySetting>(which, AuBaySetting(*(*ci).second)));
			m_CargoLock.unlock();
			m_BayLock.unlock();
			SetModified(NOTIFY_Database);
			return true;
		}
		m_CargoLock.unlock();
	}
	m_BayLock.unlock();
	return false;
}

// 'whichTurret' is the system which identifier of the turret to load
void DaVessel::LoadTurretAmmunition(const AuItem& item, quint16 whichTurret)
{
	if(!CanLoadTurretAmmunition(item, whichTurret))
		return;

	// If current turret ammo is full of rounds or if it's an energy weapon, we can put it back into the hold.
	m_TurretLock.lockForRead();
	unordered_map<quint16, AuOrdnance>::iterator ai = m_Turrets.find(whichTurret);
	if(ai != m_Turrets.end() && (*ai).second.CommodityId() != CID_Invalid)
	{
		// Ammunition is loaded at that turret position.
		// Can we put it back into the hold?
		if((*ai).second.IsEnergyWeapon() ||
				((*ai).second.CommodityId() != CID_Invalid &&
				 (*ai).second.RoundsCount() == DbCommodity::MaxTurretRounds((*ai).second.CommodityId())))
		{
			// Put one unit of the ammo back into the hold.
			AuItem* returnAmmo = (*ai).second.Ammo()->Clone();			// The count on the item is the rounds count.
			if(returnAmmo)
			{
				returnAmmo->SetCount(1);						// We only return one commodity.
				AddCargo(*returnAmmo);						// Add the commodity to the hold.
				delete returnAmmo;
			}
		}
	}
	m_TurretLock.unlock();

	AuOrdnance ammo;
	if(DbCommodity::IsEnergyWeapon(item.CommodityId()))
	{
		// Energy beam loads using one unit of energy.
		if(CapacitorLevel() > 1)
		{
			ammo.Load(item, DbCommodity::MaxTurretRounds(item.CommodityId()));
			DischargeCapacitor(1);
		}
		else
			ammo.Load(item, 0);
	}
	else
	{
		// Non-energy weapon is loaded by the ammo.
		ammo.Load(item, DbCommodity::MaxTurretRounds(item.CommodityId()));
	}

	m_TurretLock.lockForWrite();
	ai = m_Turrets.find(whichTurret);
	if(ai == m_Turrets.end())
		m_Turrets.insert(pair<quint16, AuOrdnance>(whichTurret, ammo));
	else
		(*ai).second = ammo;
	m_TurretLock.unlock();

	SetModified(NOTIFY_Database);
}

// Reloads turret with current ordnance. Picks nu ordnance at random if current ordnance is gone.
// Does nothing if current ordnance is full of ammo.
// Returns true if the turret is loaded.
bool DaVessel::ReloadTurret(quint16 which)
{
	m_TurretLock.lockForWrite();
	unordered_map<quint16, AuOrdnance>::iterator ai = m_Turrets.find(which);
	if(ai != m_Turrets.end() && (*ai).second.CommodityId() != CID_Invalid)
	{
		// Ammunition is loaded at that turret position.
		int maxRounds = DbCommodity::MaxTurretRounds((*ai).second.CommodityId());
		if((*ai).second.RoundsCount() >= maxRounds)
		{
			m_TurretLock.unlock();
			return false;	// Gun is full of ammo already.
		}

		if((*ai).second.IsEnergyWeapon())
		{
			// One unit of energy is used to charge up an energy weapon.
			if(CapacitorLevel() <= 1)
			{
				m_TurretLock.unlock();
				return false;
			}

			(*ai).second.Reload(maxRounds);
			m_TurretLock.unlock();
			DischargeCapacitor(1);
			SetModified();
			return true;
		}
		else
		{
			// Locate the exact ammunition in the hold.
			m_CargoLock.lockForWrite();
			AuItemHash::iterator ci = m_Cargo.Find(*(*ai).second.Ammo());
			if(ci != m_Cargo.end())
			{
				(*ai).second.Reload(maxRounds);
				RemoveCargoNoLock(*(*ci).second, 1);
				m_CargoLock.unlock();
				m_TurretLock.unlock();
				return true;
			}

			// Locate similar ammunition.
			ci = m_Cargo.FindBest((*ai).second.CommodityId());
			if(ci != m_Cargo.end())
			{
				m_TurretLock.unlock();

				// We have to clone the ammo item because both LoadTurretAmmunition and RemoveCargo may modify the cargo list.
				AuItem* ammo = (*ci).second->Clone();
				if(ammo)
				{
					RemoveCargoNoLock(*ammo, 1);
					m_CargoLock.unlock();

					LoadTurretAmmunition(*ammo, which);
					delete ammo;
				}
				else
					m_CargoLock.unlock();
				return true;
			}
			m_CargoLock.unlock();
		}
	}
	m_TurretLock.unlock();

	// Nothing already loaded in that turret or the hold contains no more ammo for it.
	// We don't automatically replace an energy weapon due to low ammo.
	// List ammo usable by turrets, or order of preference.
	QList<COMMODITYID> ammoNeeded;
	ammoNeeded.append(CID_CompressorBeam);
	ammoNeeded.append(CID_DisintegratorRay);
	ammoNeeded.append(CID_ProtonBeam);
	ammoNeeded.append(CID_StasisBeam);
	ammoNeeded.append(CID_DisruptorBeam);
	ammoNeeded.append(CID_ErgRay);
	ammoNeeded.append(CID_HeatRay);
	ammoNeeded.append(CID_FreezeRay);
	ammoNeeded.append(CID_NeutronBeam);
	ammoNeeded.append(CID_ElectronBeam);
	ammoNeeded.append(CID_LargeMissileSlPo);
	ammoNeeded.append(CID_LargeMissileBlBu);
	ammoNeeded.append(CID_LargeMissileBl);
	ammoNeeded.append(CID_LargeRocketBuDi);
	ammoNeeded.append(CID_LargeRocketBlBu);
	ammoNeeded.append(CID_LargeRocketBl);
	ammoNeeded.append(CID_SmallMissileCrFr);
	ammoNeeded.append(CID_SmallMissileBlBu);
	ammoNeeded.append(CID_SmallMissileBl);
	ammoNeeded.append(CID_SmallRocketSlPi);
	ammoNeeded.append(CID_SmallRocketBlBu);
	ammoNeeded.append(CID_SmallRocketBl);
	ammoNeeded.append(CID_HeavyArmsAmmoPiRa);
	ammoNeeded.append(CID_HeavyArmsAmmoPiAc);
	ammoNeeded.append(CID_HeavyArmsAmmoPiBl);
	ammoNeeded.append(CID_HeavyArmsAmmoPi);
	COMMODITYID turretCID = SystemModule(SYS_Turret, which);
	for(QList<COMMODITYID>::const_iterator mi = ammoNeeded.cbegin(); mi != ammoNeeded.cend(); mi++)
	{
		if(!DbCommodity::IsAmmunitionForWeapon(turretCID, *mi))
			continue;

		m_CargoLock.lockForWrite();
		AuItemHash::const_iterator ci = m_Cargo.FindBest(*mi);
		if(ci != m_Cargo.end())
		{
			// We have to clone the ammo item because both LoadTurretAmmunition and RemoveCargo may modify the cargo list.
			AuItem* ammo = (*ci).second->Clone();
			if(ammo)
			{
				RemoveCargoNoLock(*ammo, 1);
				m_CargoLock.unlock();

				LoadTurretAmmunition(*ammo, which);
				delete ammo;
			}
			else
				m_CargoLock.unlock();
			return true;
		}
		m_CargoLock.unlock();
	}
	return false;
}

void DaVessel::ListOrdnanceAvailable(AuItemHash& ord, quint32 empireId) const
{
	ord.Clear();

	// We do not provide ordnance to enemy under any circumstances.
	POLITICAL_STANCE stance = PoliticalStance(EmpireId(), empireId);
	if(stance == POL_Enemy)
		return;

	DgEmpire emp;
	emp.m_id = EmpireId();
	const DgData& gd = g_Au.LockDataForRead();
	bool hasMyEmp = gd.Empire(emp);
	g_Au.UnlockData();
	if(!hasMyEmp)
		return;

	m_CargoLock.lockForRead();
	for(AuItemHash::const_iterator ii = m_Cargo.begin(); ii != m_Cargo.end(); ii++)
	{
		if(!DbCommodity::IsOrdnance((*ii).second->CommodityId()))
			continue;

		EXPORT_POLICY commerce = emp.CommodityCommerce((*ii).second->CommodityId());
		switch(stance)
		{
		case POL_Empire:
		case POL_Vassal:
			break;
		case POL_Friend:
			if(commerce == EXPOL_ToNobody)
				continue;	// Commodity cannot be exported to anybody, only to my own empire.
			break;
		case POL_Neutral:
			if(commerce == EXPOL_ToNobody || commerce == EXPOL_ToFriend)
				continue;	// Commodity cannot be exported to neutrals, only to friends and my own empire.
			break;
		case POL_Enemy:
			if(commerce != EXPOL_ToEnemy)
				continue;	// Commodity cannot be exported to enemies.
			break;
		}

		// The ordnance item can be exported.
		ord.AddItem(*(*ii).second);
	}
	m_CargoLock.unlock();
}

// Removes one units of 'item' from my cargo to load into a vehicle belonging to 'empireId'.
// Checks empire export policy toward 'empireId'.
// Returns true if it was taken, false if not.
bool DaVessel::TakeOrdnance(const AuItem& item, quint32 empireId)
{
	if(!DbCommodity::IsOrdnance(item.CommodityId()))
		return false;

	// Verify export restrictions based on political stance.
	const DgData& gd = g_Au.LockDataForRead();
	POLITICAL_STANCE stance = gd.EmpirePoliticalStance(EmpireId(), empireId);
	EXPORT_POLICY commerce = gd.EmpireCommerce(EmpireId(), item.CommodityId());
	g_Au.UnlockData();
	switch(stance)
	{
	case POL_Empire:
	case POL_Vassal:
		break;
	case POL_Friend:
		if(commerce == EXPOL_ToNobody)
			return false;	// Commodity cannot be exported to anybody, only to my own empire.
		break;
	case POL_Neutral:
		if(commerce == EXPOL_ToNobody || commerce == EXPOL_ToFriend)
			return false;	// Commodity cannot be exported to neutrals, only to friends and my own empire.
		break;
	case POL_Enemy:
		return false;		// We do not load ordnance onto enemy vehicles.
	}

	return RemoveCargo(item, 1);
}

// 'whichTurret' is the system which identifier of the turret to load
bool DaVessel::CanLoadTurretAmmunition(const AuItem& item, quint16 whichTurret) const
{
	COMMODITYID turretCID = SystemModule(SYS_Turret, whichTurret);
	return DbCommodity::IsAmmunitionForWeapon(turretCID, item.CommodityId());
}

// 'whichBay' is the system which identifier of the bay to load
bool DaVessel::CanSelectBayWeaponAmmunition(const AuItem& item, quint16 whichBay) const
{
	COMMODITYID bayCID = SystemModule(SYS_Weapon, whichBay);
	return DbCommodity::IsAmmunitionForWeapon(bayCID, item.CommodityId());
}

// Ship fuel status, shield policy and sensor scan information are used to determine whether the shields should be up.
DaVessel::SHIELD_STATE DaVessel::ShieldStateDesired(int& why, QString& threatName) const
{
	// Shields are not desirable when the fuel level is extremely low.
	if(ShieldFuelPolicy() == ShieldUpUnlessNoFuel && FuelQuantity() < 20)
	{
		why = 0;
		return DaVessel::SS_Off;					// Shields down and policy says no shields when no fuel.
	}

	// Shields may not exist or may not be functioning due to damage.
	if(!ShieldUnitsWorking())
	{
		why = 1;
		return DaVessel::SS_Off;					// Shields off.
	}

	if(ShieldUpPolicy() == ShieldUp)
	{
		why = 2;
		return DaVessel::SS_Up;
	}
	else if(ShieldUpPolicy() == ShieldDown)
	{
		why = 3;
		return DaVessel::SS_Down;
	}

	// Determine the sensor range of the ship. Even ships with no sensors conduct sensor scans. The information is used for
	// targetting of turrets by AI units that are aboard.
	float sensorRange = SensorRange();
	if(sensorRange < 1000)
		sensorRange = 1000;					// Allow for a minimum "sight" range.

	// Shields policy determines normal running state.
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
	{
		why = 4;
		return DaVessel::SS_Up;	// D: We might need shields.
	}

	AuId shipWorldId;
	int shipDomain = GunnerDomain(shipWorldId);

	const DaAtom* targetAtom = 0;
	double targetRange = 0;
	system->FindClosestShipThreatRecursive(Id(), shipDomain, shipWorldId, EmpireId(), ShieldUpPolicy(), EnvLocation(), sensorRange,
			targetAtom, targetRange);
	if(!targetAtom)
	{
		// No threat detected in range.
		why = 5;
		return DaVessel::SS_Down;
	}

	// Threat detected in range.
	why = 6;
	quint32 targetEmpire = targetAtom->EmpireId();
	if(targetEmpire)
	{
		QString tempire;
		const DgData& gd = g_Au.LockDataForRead();
		gd.EmpireName(targetEmpire, tempire);
		g_Au.UnlockData();
		threatName = QString("%1 %2").arg(tempire).arg(targetAtom->Name());
	}
	else
		threatName = targetAtom->Name();
	return DaVessel::SS_Up;
}

// Searches for any creature that prevents mothballing this ship.
// Players and crew assigned to the ship prevent mothballing.
const DaCreature* DaVessel::FindCreatureBlockingMothball() const
{
	DaAtomList allsubs;
	ListSubatomsRecursive(allsubs);		// We must get abnormal atoms so we notice people who are spawning.
	for(DaAtomList::const_iterator ai = allsubs.begin(); ai != allsubs.end(); ai++)
	{
		// Ignore dead atoms.
		if((*ai)->IsDead())
			continue;

		// Any person aboard prevents this ship from being hangared.
		const DaPerson* person = dynamic_cast<const DaPerson*>(*ai);
		if(person)
			return person;

		// Any officer aboard prevents this ship from being hangared.
		const DaNpc* officer = dynamic_cast<const DaNpc*>(*ai);
		if(officer && officer->IsNpcOfficer())
			return officer;
	}

	// Did not find any creature aboard that prevents this ship from being hangared.
	return 0;
}

// 'byWho' is the person who is mothballing the ship.
// 'hangarAtom' is a world or a terrain patch that will host the hangared ship atom.
// DaVessel::Reactivate is called to wake it back up. Part of the work is done in DtChatChannel.
void DaVessel::Mothball(const DaPerson* byWho, DaAtom* hangarAtom)
{
	// At DaBd hangar, my transform is irrelevant.
	AuMatrix4x4 hangarT;
	if(!dynamic_cast<DaBd*>(hangarAtom))
		byWho->GetTransformRelativeToParent(hangarT, hangarAtom);

	// Move to the terrain patch or world containing the person, at the person's current location.
	ChangePosAndOrient(AuAtomPos(hangarAtom->Id(), hangarT));	// Move to become subordinate to the hangar.
	InvalidateEnvironment();

	// Prepare for storage. The only roster entry will be the captain, to identify the person who mothballed the ship.
	SetDormant();												// Ship atom becomes dormant until launched.
	Moor();														// Mooring indicates hangar vs stasis dormancy.
	if(IsNew())
		SetNotNew();											// Clear the 'nu' flag when mothballed.
	if(!CaptainId().IsNull())
		RemoveBerthOccupant(CaptainId());					// Remove the current captain from the roster.
	OccupyBerth(AuRosterEntry::Captain, byWho->Id(), byWho->Name());
	SetModified();
	g_Au.StopMonitoringChannels(Id());							// Stop monitoring channels.
}

bool DaVessel::KillDormantAbandoned()
{
	if(!IsDormant())
		return false;

	// Mooring distinguishes between hangared ship vs ship in stasis.
	if(!IsMoored())
	{
		// Ship in stasis.
		// Check for any human officers still on the roster.
		if(!HasHumanOfficer())
		{
			//qDebug() << "DaVessel::KillDormantAbandoned" << Id() << Name() << "in stasis with no human officer; Kill'd.";
			Kill();
			return true;
		}
	}
	else
	{
		// Ship in a hangar.
		// Check for a hangar.
		const DaBd* bd = dynamic_cast<const DaBd*>(Parent());
		if(!bd || bd->IndustryId() != IID_Hangar)
		{
			// Ship is not in a hangar.
			Kill();
			return true;
		}
	}

	// This ship is ok if we make it to here.
	return false;
}

void DaVessel::KickStowaways(QStringList& stowaways, bool offLineOnly)
{
	DaAtomList peopleAboard;
	FindPeopleRecursive(peopleAboard);
	for(DaAtomList::const_iterator pi = peopleAboard.begin(); pi != peopleAboard.end(); pi++)
	{
		// FindPeopleRecursive only returns 'normal' DaPerson atoms.
		DaPerson* persAboard = (DaPerson*)(*pi);

		// Person online is not kicked when only kicking off line people.
		if(offLineOnly && !persAboard->IsInvisibleOffLine())
			continue;

		// Don't kick people in the chain-of-command of the ship.
		// Agents with powers enabled are consider to be in the chain-of-command of all ships.
		if(InChainOfCommand(persAboard))
			continue;

		// Don't kick stowaways who are aboard subordinate vessels.
		if(persAboard->FindParentVesselInEnv() != this)
			continue;

		if(BerthTypeOccupied(persAboard->Id()) == AuRosterEntry::Invalid || !Is(persAboard->SpawnShip()))
		{
			// Found a stowaway.
			QString name = QString("%1 (%2)").arg(persAboard->Name()).arg(persAboard->Id().StringId());
			stowaways.append(name);
			persAboard->SpawnRecall(AuTime::Now());			// Recall home immediately, with no delay.
			persAboard->SetModified();
		}
	}
}

void DaVessel::LoadCrew(quint32 empireId, const AuSpecies& species)
{
	// Make a captain only if there is not already one.
	if(!FindOfficer() && OfficerBerthsAvailable())
		LoadPirateOfficer(empireId, species);

	// Make a crewman for each empty crew berth.
	int crew = CrewBerthsAvailable();
	while(crew--)
		LoadPirateCrew(empireId, species);

	// Make a troop for each empty troop berth.
	int troops = TroopBerthsAvailable();
	while(troops--)
		LoadPirateTroop(empireId, species);
}

void DaVessel::LoadPirateCrew(quint32 pirateEmpire)
{
	// Get DNA for the pirate empire.
	const DgData& gd = g_Au.LockDataForRead();
	AuSpecies species = gd.EmpireCapitolSpecies(pirateEmpire);
	g_Au.UnlockData();
	LoadCrew(pirateEmpire, species);
}

DaNpc* DaVessel::LoadPirateOfficer(quint32 pirateEmpire, const AuSpecies& species)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	Q_ASSERT(OfficerBerthsAvailable());							// Caller needs to insure there is an Officer berth available.

	// Create an officer.
	AuDNABase::CHROMATID chroma = (AuDNABase::CHROMATID)g_Au.Dice().Roll(1,2,-1);
	HANDEDNESS hand = (HANDEDNESS)g_Au.Dice().Roll(1,2,-1);
	DaNpc* officer = new DaNpc(ROLE_Officer, species, chroma, hand, g_Au.Dice().RandomPersonName());
	officer->ConnectSignals();
	officer->Create();

	const DaWorldLL* world = FindParentWorldInEnv();
	if(world)
		officer->SetHomeWorld(world);
	officer->ResetHitPoints();
	officer->ResetHunger();
	officer->ResetStamina();
	officer->SetEmpireId(pirateEmpire);
	officer->Stand();
	officer->InitTraining();

	// Occupy an AI Officer berth with the officer.
	officer->OccupyBerth(this);							// Also opens comm channels that need monitoring and takes the con.
	AuPoint berthLoc = BerthLocation(officer->Id());

	// Position the atom in the environment.
	//berthLoc.z += officer->GroundHeightAboveCG();
	berthLoc.z += .1;
	AuMatrix4x4 t;
	t.Translate(berthLoc);									// Move to location in spacecraft.
	officer->SetPosAndOrient(AuAtomPos(Id(), t));
	ether->AddAtom(officer);

	officer->InitPirateGear(QualityMade());
	officer->IncreaseSkill(AuSkill::Command, g_Au.Dice().Roll(1,0xffff));	// Before assignment to ship saves work.
	officer->GoToWork();

	g_Au.AtomAdded(officer->Id());
	officer->SetNormal();	// New atom is ready for normal thread processing.
	officer->TickTock();
	return officer;
}

DaNpc* DaVessel::LoadPirateCrew(quint32 pirateEmpire, const AuSpecies& species)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	// Create a crewman.
	AuDNABase::CHROMATID chroma = (AuDNABase::CHROMATID)g_Au.Dice().Roll(1,2,-1);
	HANDEDNESS hand = (HANDEDNESS)g_Au.Dice().Roll(1,2,-1);
	DaNpc* crew = new DaNpc(ROLE_Crew, species, chroma, hand, g_Au.Dice().RandomPersonName());
	crew->ConnectSignals();
	crew->Create();

	const DaWorldLL* world = FindParentWorldInEnv();
	if(world)
		crew->SetHomeWorld(world);
	crew->ResetHitPoints();
	crew->ResetHunger();
	crew->ResetStamina();
	crew->SetEmpireId(pirateEmpire);
	crew->Stand();
	crew->InitTraining();

	// Occupy a berth.
	crew->OccupyBerth(this);
	AuPoint berthLoc = BerthLocation(crew->Id());

	// Position the atom in the environment.
	//berthLoc.z += crew->GroundHeightAboveCG();
	berthLoc.z += .1;
	AuMatrix4x4 t;
	t.Translate(berthLoc);									// Move to location in spacecraft.
	crew->SetPosAndOrient(AuAtomPos(Id(), t));
	ether->AddAtom(crew);

	crew->InitPirateGear(QualityMade());
	crew->IncreaseSkill(AuSkill::SpacecraftOperations, g_Au.Dice().Roll(1,0xffff));	// Before assignment to ship saves work.
	crew->GoToWork();

	g_Au.AtomAdded(crew->Id());
	crew->SetNormal();	// New atom is ready for normal thread processing.
	crew->TickTock();
	return crew;
}

DaNpc* DaVessel::LoadPirateTroop(quint32 pirateEmpire, const AuSpecies& species)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	// Create a troop.
	AuDNABase::CHROMATID chroma = (AuDNABase::CHROMATID)g_Au.Dice().Roll(1,2,-1);
	HANDEDNESS hand = (HANDEDNESS)g_Au.Dice().Roll(1,2,-1);
	DaNpc* troop = new DaNpc(ROLE_Troop, species, chroma, hand, g_Au.Dice().RandomPersonName());
	troop->ConnectSignals();
	troop->Create();

	const DaWorldLL* world = FindParentWorldInEnv();
	if(world)
		troop->SetHomeWorld(world);
	troop->ResetHitPoints();
	troop->ResetHunger();
	troop->ResetStamina();
	troop->SetEmpireId(pirateEmpire);
	troop->Stand();
	troop->InitTraining();

	// Occupy a berth.
	troop->OccupyBerth(this);
	AuPoint berthLoc = BerthLocation(troop->Id());

	// Position the atom in the environment.
	//berthLoc.z += troop->GroundHeightAboveCG();
	berthLoc.z += .1;
	AuMatrix4x4 t;
	t.Translate(berthLoc);									// Move to location in spacecraft.
	troop->SetPosAndOrient(AuAtomPos(Id(), t));
	ether->AddAtom(troop);

	troop->InitPirateGear(QualityMade());
	troop->IncreaseSkill(AuSkill::Combat, g_Au.Dice().Roll(1,0xffff));	// Before assignment to ship saves work.
	troop->GoToWork();

	g_Au.AtomAdded(troop->Id());
	troop->SetNormal();	// New atom is ready for normal thread processing.
	troop->TickTock();
	return troop;
}

DaNpc* DaVessel::LoadPiratePassenger(quint32 pirateEmpire, const AuSpecies& species)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	// Create a passenger.
	AuDNABase::CHROMATID chroma = (AuDNABase::CHROMATID)g_Au.Dice().Roll(1,2,-1);
	HANDEDNESS hand = (HANDEDNESS)g_Au.Dice().Roll(1,2,-1);
	DaNpc* passenger = new DaNpc(ROLE_Passenger, species, chroma, hand, g_Au.Dice().RandomPersonName());
	passenger->ConnectSignals();
	passenger->Create();

	GROWTHSTAGE stage = g_Au.Dice().Roll(1,100) < 15 ? Juvenile : Mature;
	if(stage == Juvenile)
	{
		quint8 fullGrowthTime = passenger->GrowthTime();
		quint32 minutesToFullGrown = (fullGrowthTime + 1) / 2;	// At least 1.
		passenger->SetMinutesToFullGrown(minutesToFullGrown);
	}

	const DaWorldLL* world = FindParentWorldInEnv();
	if(world)
		passenger->SetHomeWorld(world);
	passenger->ResetHitPoints();
	passenger->ResetHunger();
	passenger->ResetStamina();
	passenger->SetEmpireId(pirateEmpire);
	passenger->Stand();
	passenger->InitTraining();

	// Occupy a berth.
	passenger->OccupyBerth(this);
	AuPoint berthLoc = BerthLocation(passenger->Id());

	// Position the atom in the environment.
	//berthLoc.z += passenger->GroundHeightAboveCG();
	berthLoc.z += .1;
	AuMatrix4x4 t;
	t.Translate(berthLoc);									// Move to location in spacecraft.
	passenger->SetPosAndOrient(AuAtomPos(Id(), t));
	ether->AddAtom(passenger);

	passenger->InitPirateGear(QualityMade());
	passenger->GoToWork();

	g_Au.AtomAdded(passenger->Id());
	passenger->SetNormal();	// New atom is ready for normal thread processing.
	passenger->TickTock();
	return passenger;
}

// Returns true if this ship is commanded by a pirate AI.
bool DaVessel::IsPirateCommander() const
{
	// If this ship's empire is not a pirate, then the commander cannot be a pirate. This test is fast.
	// This is not true when someone is the captain, which could only be true if they were an agent. ;)
	if(!IsPirateWithFreeWill())
		return false;

	// Find the AI who is in command.
	DaSentient* officer = FindOfficer();						// Find any officer, even if they do not have the con.
	return officer && officer->IsPirate();
}

void DaVessel::SendAlert(const QString& text)
{
	//qDebug() << "DaVessel::SendAlert" << Id() << Name() << text;
	// Parent sector always exists.
	const DaSector* sector = FindParentSector();
	Q_ASSERT(sector);

	// Tell them where I am.
	QString textWithLoc = text + " " + CommMessageLocation();

	// Send the SOS message on the Friend channel at sector range.
	AuCommChat msg(textWithLoc, AuCommStdChannel::FriendChannelName(), Id(), Name(), sector->Id(),
			Galaxy(), AuCommStdChannel::Sector, AuCommStdChannel::Friends);
	msg.SetScopeData(EmpireId());								// Friend channel needs empire to resolve the scope of the message.
	g_Au.SendAtomChat(msg);

	// Remember when we last cried out to the universe.
	m_CommLock.lockForWrite();
	m_tmAlert.restart();
	m_CommLock.unlock();
}

void DaVessel::SendEnvDamage(const QString& text)
{
	// Do not do this if I am a pirate without a human captain.
	if(IsPirateWithFreeWill())
		return;

	// Do not report this more than once per minute.
	AuElapsedTimer et(LastEnvDamage());
	if(et.isValid() && et.ElapsedSec() < 20)
		return;

	// Send the message on the Crew channel at system range.
	AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(),
			AuCommStdChannel::SolarSystem, AuCommStdChannel::CrewMates);
	msg.SetScopeData(Id().GetID());	// Crew channel needs ship's id to know the scope of the message.
	g_Au.SendAtomChat(msg);

	// Remember when we last cried out to the universe.
	m_CommLock.lockForWrite();
	m_tmEnvDamage.restart();
	m_CommLock.unlock();
}

void DaVessel::SendSOS(int msgType, const QString& msgText, int msgValue)
{
	if(InArena())
		return;
	if(InDesignEnvironment())
		return;

	//qDebug() << "DaVessel::SendSOS" << Id() << Name() << msgText;
	// Parent sector always exists.
	const DaSector* sector = FindParentSector();
	Q_ASSERT(sector);

	// Tell them where I am.
	QString loc = CommMessageLocation();

	switch(msgType)
	{
	case 0:	// Station Maneuver Drive Out
		{
			// Send the SOS message on the Friend channel at sector range.
			AuCommSOSManeuverDriveStation msg(loc, AuCommStdChannel::FriendChannelName(), Id(), Name(), sector->Id(),
					Galaxy(), AuCommStdChannel::Sector, AuCommStdChannel::Friends);
			msg.SetScopeData(EmpireId());						// Friend channel needs empire to resolve the scope of the message.
			g_Au.SendAtomChat(msg);

			// Stations send the SOS message on the Trade channel so it can be logged by cities in the system.
			AuCommSOSManeuverDriveStation msg2(loc, AuCommStdChannel::TradeChannelName(), Id(), Name(), sector->Id(),
					Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
			g_Au.SendAtomChat(msg2);
		}
		break;

	case 1:	// Station Fuel Emergency
		{
			// Send the SOS message on the Friend channel at sector range.
			AuCommSOSFuelStation msg(msgValue, loc, AuCommStdChannel::FriendChannelName(), Id(), Name(), sector->Id(),
					Galaxy(), AuCommStdChannel::Sector, AuCommStdChannel::Friends);
			msg.SetScopeData(EmpireId());						// Friend channel needs empire to resolve the scope of the message.
			g_Au.SendAtomChat(msg);

			// Stations send the SOS message on the Trade channel so it can be logged by cities in the system.
			AuCommSOSFuelStation msg2(msgValue, loc, AuCommStdChannel::TradeChannelName(), Id(), Name(), sector->Id(),
					Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
			g_Au.SendAtomChat(msg2);
		}
		break;

	case 2:	// Ship Maneuver Drive Out, Marooned
		{
			// Send the SOS message on the Friend channel at sector range.
			AuCommSOSManeuverDriveMarooned msg(loc, AuCommStdChannel::FriendChannelName(), Id(), Name(), sector->Id(),
					Galaxy(), AuCommStdChannel::Sector, AuCommStdChannel::Friends);
			msg.SetScopeData(EmpireId());						// Friend channel needs empire to resolve the scope of the message.
			g_Au.SendAtomChat(msg);
			
			// Ships send the SOS message on the Hail channel so it can be logged by ships and stations in the system.
			AuCommSOSManeuverDriveMarooned msg2(loc, AuCommStdChannel::HailChannelName(), Id(), Name(), sector->Id(),
					Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
			g_Au.SendAtomChat(msg2);
		}
		break;

	case 3:	// Ship Maneuver Drive Out, Adrift
		{
			// Send the SOS message on the Friend channel at sector range.
			AuCommSOSManeuverDriveAdrift msg(loc, AuCommStdChannel::FriendChannelName(), Id(), Name(), sector->Id(),
					Galaxy(), AuCommStdChannel::Sector, AuCommStdChannel::Friends);
			msg.SetScopeData(EmpireId());						// Friend channel needs empire to resolve the scope of the message.
			g_Au.SendAtomChat(msg);

			// Ships send the SOS message on the Hail channel so it can be logged by ships and stations in the system.
			AuCommSOSManeuverDriveAdrift msg2(loc, AuCommStdChannel::HailChannelName(), Id(), Name(), sector->Id(),
					Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
			g_Au.SendAtomChat(msg2);
		}
		break;

	case 4:	// Ship Fuel, Marooned
		{
			// Send the SOS message on the Friend channel at sector range.
			AuCommSOSFuelMarooned msg(loc, AuCommStdChannel::FriendChannelName(), Id(), Name(), sector->Id(),
					Galaxy(), AuCommStdChannel::Sector, AuCommStdChannel::Friends);
			msg.SetScopeData(EmpireId());						// Friend channel needs empire to resolve the scope of the message.
			g_Au.SendAtomChat(msg);

			// Ships send the SOS message on the Hail channel so it can be logged by ships and stations in the system.
			AuCommSOSFuelMarooned msg2(loc, AuCommStdChannel::HailChannelName(), Id(), Name(), sector->Id(),
					Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
			g_Au.SendAtomChat(msg2);
		}
		break;

	case 5:	// Ship Fuel
		{
			// Send the SOS message on the Friend channel at sector range.
			AuCommSOSFuelShip msg(msgValue, loc, AuCommStdChannel::FriendChannelName(), Id(), Name(), sector->Id(),
					Galaxy(), AuCommStdChannel::Sector, AuCommStdChannel::Friends);
			msg.SetScopeData(EmpireId());						// Friend channel needs empire to resolve the scope of the message.
			g_Au.SendAtomChat(msg);

			// Ships send the SOS message on the Hail channel so it can be logged by ships and stations in the system.
			AuCommSOSFuelShip msg2(msgValue, loc, AuCommStdChannel::HailChannelName(), Id(), Name(), sector->Id(),
					Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
			g_Au.SendAtomChat(msg2);
		}
		break;

	case 6:	// Under Attack
		{
			// Send the SOS message on the Friend channel at sector range.
			AuCommSOSAttack msg(NameWithEmpire(), msgText, loc, AuCommStdChannel::FriendChannelName(), Id(), Name(),
					sector->Id(), Galaxy(), AuCommStdChannel::Sector, AuCommStdChannel::Friends);
			msg.SetScopeData(EmpireId());						// Friend channel needs empire to resolve the scope of the message.
			g_Au.SendAtomChat(msg);
			if(IsSpaceStation())
			{
				// Stations send the SOS message on the Trade channel so it can be logged by cities in the system.
				AuCommSOSAttack msg2(NameWithEmpire(), msgText, loc, AuCommStdChannel::TradeChannelName(), Id(), Name(),
						sector->Id(), Galaxy(), AuCommStdChannel::SolarSystem,
						AuCommStdChannel::Everyone);
				g_Au.SendAtomChat(msg2);
			}
			else
			{
				// Ships send the SOS message on the Hail channel so it can be logged by ships and stations in the system.
				AuCommSOSAttack msg2(NameWithEmpire(), msgText, loc, AuCommStdChannel::HailChannelName(), Id(), Name(),
						sector->Id(), Galaxy(), AuCommStdChannel::SolarSystem,
						AuCommStdChannel::Everyone);
				g_Au.SendAtomChat(msg2);
			}
		}
		break;
	}

	// Remember when we last cried out to the universe.
	m_CommLock.lockForWrite();
	m_tmSOS.restart();
	m_CommLock.unlock();
}

void DaVessel::CreditAccount(double amount)
{
	if(amount <= 0)
		return;
	m_AccountLock.lockForWrite();
	m_Bank += amount;
	if(m_Bank > 1000000000000000)
		m_Bank = 1000000000000000;
	m_tmLastBank.Set();
	m_AccountLock.unlock();
	SetModified();
}

void DaVessel::CreditAccountSubsidyTax(double amount)
{
	Q_ASSERT(!IsPrivatelyOwned());
	if(amount <= 0)
		return;

	m_AccountLock.lockForWrite();
	m_BankSubsidyTax += amount;
	if(m_BankSubsidyTax > 1000000000000000)
		m_BankSubsidyTax = 1000000000000000;
	m_tmLastBank.Set();
	m_AccountLock.unlock();
	SetModified();
}

void DaVessel::CreditAccountSales(double amount)
{
	if(amount <= 0)
		return;

	if(IsPrivatelyOwned())
	{
		CreditAccount(amount);
		return;
	}

	// Subsidized ship must take subsidy tax off the money.
	const DgData& gd = g_Au.LockDataForRead();
	float subsidyTaxRate = gd.EmpireSubsidyTax(EmpireId());
	g_Au.UnlockData();
	if(subsidyTaxRate <= 0 || subsidyTaxRate > 100)
	{
		// Tax rate is 0 or it is invalid. Do not take any taxes off the money.
		CreditAccount(amount);
		return;
	}

	// Calculate tax amount.
	double taxAmount = amount * subsidyTaxRate / 100;

	// Credit the appropriate accounts.
	m_AccountLock.lockForWrite();
	m_BankSubsidyTax += taxAmount;
	m_Bank += amount - taxAmount;
	if(m_Bank > 1000000000000000)
		m_Bank = 1000000000000000;
	if(m_BankSubsidyTax > 1000000000000000)
		m_BankSubsidyTax = 1000000000000000;
	m_tmLastBank.Set();
	m_AccountLock.unlock();
	SetModified();
}

void DaVessel::DebitAccount(double amount)
{
	if(amount <= 0)
		return;

	m_AccountLock.lockForWrite();
	m_Bank -= amount;
	if(m_Bank < 0)
		m_Bank = 0;
	m_tmLastBank.Set();
	m_AccountLock.unlock();
	SetModified();
}

void DaVessel::DebitAccountSubsidyTax(double amount)
{
	if(amount <= 0)
		return;

	m_AccountLock.lockForWrite();
	m_BankSubsidyTax -= amount;
	if(m_BankSubsidyTax < 0)
		m_BankSubsidyTax = 0;
	m_tmLastBank.Set();
	m_AccountLock.unlock();
	SetModified();
}

void DaVessel::SetConsoleEntered(const DaCreature* byWho)
{
	Q_ASSERT(byWho->HasConsole());
	if(dynamic_cast<const DaPerson*>(byWho))
	{
		// A person sat at a console.
		switch(byWho->ConsoleType())
		{
		case AuStation::Console_Engine:
		case AuStation::Integrated_Engine:
			m_ConEngineerPerson = 2;
			break;
		case AuStation::Console_FireControl:
		case AuStation::Integrated_FireControl:
			m_ConWeaponPerson = 2;
			break;
		case AuStation::Console_Navigation:
		case AuStation::Integrated_Navigation:
			m_ConNavigatorPerson = 2;
			break;
		case AuStation::Console_Helm:
		case AuStation::Integrated_Helm:
			m_ConPilotPerson = 2;
			break;
		case AuStation::Console_Power:
		case AuStation::Integrated_Power:
			m_ConPowerRelayPerson = 2;
			break;
		case AuStation::Console_Sensor:
		case AuStation::Integrated_Sensor:
			m_ConSensorPerson = 2;
			break;
		case AuStation::Console_Shield:
		case AuStation::Integrated_Shield:
			m_ConShieldPerson = 2;
			break;
		default:
			break;
		}
	}
	else
	{
		// AI sat at a console.
		switch(byWho->ConsoleType())
		{
		case AuStation::Console_Command:
		case AuStation::Integrated_Command:
			break;
		case AuStation::Console_Engine:
		case AuStation::Integrated_Engine:
			m_ConEngineerAI = 2;
			break;
		case AuStation::Console_FireControl:
		case AuStation::Integrated_FireControl:
			m_ConWeaponAI = 2;
			break;
		case AuStation::Console_Navigation:
		case AuStation::Integrated_Navigation:
			m_ConNavigatorAI = 2;
			break;
		case AuStation::Console_Helm:
		case AuStation::Integrated_Helm:
			m_ConPilotAI = 2;
			break;
		case AuStation::Console_Power:
		case AuStation::Integrated_Power:
			m_ConPowerRelayAI = 2;
			break;
		case AuStation::Console_Sensor:
		case AuStation::Integrated_Sensor:
			m_ConSensorAI = 2;
			break;
		case AuStation::Console_Shield:
		case AuStation::Integrated_Shield:
			m_ConShieldAI = 2;
			break;
		default:
			break;
		}
	}
}

// Clears rat DNA and removes all rats from the ship.
int DaVessel::ExterminateRats()
{
	// This quietly resets the probability of monster encounters.
	ResetMonster();

	// Clear current rat DNA.
	int whatChanged = 0x00;
	if(HasRatSpecies())
	{
		ClearRatSpecies();
		SetModified(NOTIFY_Database);
		whatChanged = 0x01;
	}

	// Kill any live rats we find aboard.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator ait = subs.cbegin(); ait != subs.cend(); ait++)
	{
		DaAnimal* animal = dynamic_cast<DaAnimal*>(*ait);
		if(!animal)
			continue;
		if(!animal->LooksLikeVermin())
			continue;

		// Looks like a rat.
		animal->Kill();
		whatChanged |= 0x02;
	}
	return whatChanged;
}

bool DaVessel::CleanHouse()
{
	// This quietly resets the probability of monster encounters.
	ResetMonster();

	bool foodRemoved = false;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator ait = subs.cbegin(); ait != subs.cend(); ait++)
	{
		DaItem* item = dynamic_cast<DaItem*>(*ait);
		if(!item)
			continue;
		if(item->IsStaticDisplay())
			continue;
		if(!DbCommodity::IsScavengerFood(item->CommodityId()))
			continue;
		const AuItem* aui = item->LockItem();
		short int food = aui->FoodNutritionValue();
		item->UnlockItem();
		if(!food)
			continue;

		// Looks like food that will attract rats.
		item->Kill();
		foodRemoved = true;
	}
	return foodRemoved;
}

void DaVessel::RatEncounter(const AuPoint& loc)
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	//qDebug() << "DaVessel::RatEncounter" << Id() << Name();
	if(!g_Au.m_EnableAnimalEncounters)
	{
		//qDebug() << "DaVessel::RatEncounter" << Id() << Name() << "disabled";
		return;
	}

	// No rats spawn if the inside air is not breathable.
	if(!IsInsideAirBreathable())
		return;

	// This function gets called once per minute for each piece of food in a spaceship that is in someone's scene.
	// How about a 10% chance each minute of a rat appearing to claim the juicy morsel.
	if(g_Au.Dice().Roll(1,100) > 1)
		return;						// 99% chance that no rat appears

	// Count wild animals present in this ship.
	int wildAnimals = 0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator ait = subs.cbegin(); ait != subs.cend(); ait++)
	{
		DaAnimal* animal = dynamic_cast<DaAnimal*>(*ait);
		if(!animal)
			continue;
		if(animal->CurrentHitPoints() <= 0)
			continue;	// Skip dying animals.
		if(animal->IsWild())
			wildAnimals++;
	}

	// Don't bother if there are too many wild animals in this ship.
	if(wildAnimals >= 1)
		return;					// Won't spawn because too many wild animals are already aboard.

	// There's not too many animals.
	// Generate rat DNA if this is the first one.
	if(!HasRatSpecies())
	{
		m_VesselLock.lockForWrite();
		m_RatSpecies.RandomVermin(g_Au.Dice());
		m_VesselLock.unlock();
		SetModified(NOTIFY_Database);
	}

	// Create the animal so we can get physical dimensions.
	AuDNABase::CHROMATID chroma = (AuDNABase::CHROMATID)g_Au.Dice().Roll(1,2,-1);
	HANDEDNESS hand = (HANDEDNESS)g_Au.Dice().Roll(1,2,-1);
	DaAnimal* rat = new DaAnimal(RatSpecies(), chroma, hand, Mature);
	rat->ConnectSignals();
	rat->Create();

	// Position the rat in the environment.
	AuMatrix4x4 t;
	t.Translate(loc);
	t.RotateZ(g_Au.Dice().Roll(1,360,-1)*DEG2RADf);
	rat->SetPosAndOrient(AuAtomPos(Id(), t));
	ether->AddAtom(rat);

	rat->Stand();
	rat->ResetHitPoints();	// Now that it has a body.
	rat->ResetStamina();

	// Notify the universe that a nu atom was created/added.
	g_Au.AtomAdded(rat->Id());
	rat->SetNormal();	// New atom is ready for normal thread processing.
	rat->TickTock();
}

void DaVessel::ClearRatSpecies()
{
	// Clear the value we use in the rat DNA to indicate that it is valid.
	// It will be regenerated the next time it is needed.
	m_VesselLock.lockForWrite();
	m_RatSpecies = AuSpecies();
	m_VesselLock.unlock();
}

bool DaVessel::OkToTeleportToStation(const DaSentient* crewman) const
{
	if(InDesignEnvironment())
		return false;
	if(!crewman->IsAIControlled())
		return false;
	//qDebug() << "DaVessel::OkToTeleportToStation" << *crewman << "aliveSecs" << crewman->CreatedTime().ElapsedSec() 
		//<< "personHere" << FindPersonOnLineRecursive();
	if(crewman->CreatedTime().ElapsedSec() < 15 && !FindPersonOnLineRecursive())
		return true;	// I'm nu and there is nobody aboard. This helps stories populate ships with crew.
	return crewman->ShipRoomType() == ROOM_Hull;	// I can teleport when embedded in the hull.
}

bool DaVessel::GameSelected(quint16 whichTable, quint8 gameType, const AuId& dealerId)
{
	m_GamesLock.lockForWrite();
	unordered_map<quint16, AuGame*>::iterator gi = m_Games.find(whichTable);
	if(gi != m_Games.end())
	{
		// A game is already active.
		// Replace games in the Setup state if created by the same dealer. This happens when the dealer starts a game and exits
		// the setup dialog. A zombie game gets left around until he stands up, or we could just let him start another game.
		if((*gi).second->PlayerId(0) == dealerId && !(*gi).second->IsStarted())
		{
			// Replace the game since it's not running yet.
			delete (*gi).second;
			m_Games.erase(gi);
		}
		else
		{
			// A game is already running at the table.
			m_GamesLock.unlock();
			return false;
		}
	}

	// A game can be started at the game table.
	AuGame* game = AuGame::CreateGame(gameType);
	if(!game)
	{
		m_GamesLock.unlock();
		return false;	// Unrecognized gameType? This happens when developing a nu game and the server doesn't know about it yet.
	}

	GameSetPlayers(whichTable, game, dealerId);	// Init game with players at the table right now.

	// Add the table game to this ship's list of games.
	m_Games.insert(pair<quint16, AuGame*>(whichTable, game));
	GamesSetModified();
	m_GamesLock.unlock();
	return true;
}

void DaVessel::GameSetPlayers(quint16 whichTable, AuGame* game, const AuId& dealerId) const
{
	//qDebug() << "DaVessel::GameSetPlayers" << Id() << Name() << "whichTable" << whichTable << "dealer" << dealerId
		//<< "game" << game->Name();
	// Search for the players at the table. They are automatically enrolled in the nu game in clockwise order around the table,
	// according to their position relative to the dealer.
	if(HasSubatoms())
	{
		// Must find dealer first so we can get his pose.
		const DaPerson* dealer = dynamic_cast<const DaPerson*>(FindNormalSubatomId(dealerId));
		if(!dealer || !dealer->IsNormal() || !dealer->IsOnLine() || !dealer->IsGameTablePose() ||
				dealer->StationId() != whichTable)
		{
#if !defined(QT_NO_DEBUG)
			if(dealer)
				qWarning() << "warning: DaVessel::GameSetPlayers" << Id() << Name()
					<< "dealer" << dealer->Id() << dealer->Name()
					<< "normal" << dealer->IsNormal()
					<< "onLine" << dealer->IsOnLine()
					<< "gameTablePose" << dealer->IsGameTablePose()
					<< "StationId" << dealer->StationId();
			else
				qWarning() << "warning: DaVessel::GameSetPlayers" << Id() << Name() << "dealer not found";
#endif
			return;
		}

		// The dealer is always player 0.
		game->SetPlayer(0, dealer->Id(), dealer->Name());
		switch(dealer->CurrentGeneralPose().Pose())
		{
		case AuPose::GameTableSeat0_0:
		case AuPose::GameTableSeat0_45:
		default:
			game->SetDealerSeat(0);
			break;
		case AuPose::GameTableSeat1_0:
		case AuPose::GameTableSeat1_45:
			game->SetDealerSeat(1);
			break;
		case AuPose::GameTableSeat2_0:
		case AuPose::GameTableSeat2_45:
			game->SetDealerSeat(2);
			break;
		case AuPose::GameTableSeat3_0:
		case AuPose::GameTableSeat3_45:
			game->SetDealerSeat(3);
			break;
		}

		DaAtomList subs;
		ListNormalSubatoms(subs);
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		{
			DaPerson* person = dynamic_cast<DaPerson*>(*it);
			if(!person)
				continue;	// Skip non-people.
			if(person == dealer)
				continue;	// Skip the dealer; they are already done.
			if(!person->IsOnLine())
				continue;	// Skip people who are not on line.
			if(!person->IsGameTablePose())
				continue;	// Skip people who are not at game tables.
			if(person->StationId() != whichTable)
				continue;	// Skip people at other tables.

			// Calculate person's play order relative to the dealer.
			// This is also their seat index relative to the dealer, which is needed for rendering the scene properly.
			int playOrder = person->CurrentGeneralPose().Pose() - dealer->CurrentGeneralPose().Pose();
			Q_ASSERT(playOrder < 4);
			Q_ASSERT(playOrder > -4);
			if(playOrder < 0)
				playOrder = 4 + playOrder;	// Wrap
			game->SetPlayer(playOrder, person->Id(), person->Name());
		}
	}
}

void DaVessel::GameExitPlayer(quint16 whichTable, const AuId& playerId)
{
	//qDebug() << "DaVessel::GameExitPlayer" << Id() << Name() << "player" << playerId << "whichTable" << whichTable;
	m_GamesLock.lockForWrite();
	unordered_map<quint16, AuGame*>::iterator gi = m_Games.find(whichTable);
	if(gi == m_Games.end())
	{
		//qDebug() << "DaVessel::GameExitPlayer" << Id() << Name() << "game not found";
		m_GamesLock.unlock();
		return;
	}

	if(!(*gi).second->RemovePlayer(playerId))
	{
		// The last player exited the game.
		//qDebug() << "DaVessel::GameExitPlayer" << Id() << Name() << "game removed";
		delete (*gi).second;			// Delete the game.
		m_Games.erase(gi);	// Remove it from the list of active games.
	}

	GamesSetModified();
	m_GamesLock.unlock();
}

// Returns the AuGame::GameType() in play at a game table.
quint8 DaVessel::GameType(quint16 whichTable) const
{
	m_GamesLock.lockForRead();
	unordered_map<quint16, AuGame*>::const_iterator gi = m_Games.find(whichTable);
	if(gi == m_Games.end())
	{
		m_GamesLock.unlock();
		return AuGame::GameType();
	}

	quint8 v = (*gi).second->Type();
	m_GamesLock.unlock();
	return v;
}

// Returns the icon for the game in play at a table.
QIcon DaVessel::GameIcon(quint16 whichTable) const
{
	m_GamesLock.lockForRead();
	unordered_map<quint16, AuGame*>::const_iterator gi = m_Games.find(whichTable);
	if(gi == m_Games.end())
	{
		m_GamesLock.unlock();
		return QIcon(":data/Game.png");
	}

	QIcon v((*gi).second->Icon());
	m_GamesLock.unlock();
	return v;
}

// Returns the name of the game in play at a table.
QString DaVessel::GameName(quint16 whichTable) const
{
	m_GamesLock.lockForRead();
	unordered_map<quint16, AuGame*>::const_iterator gi = m_Games.find(whichTable);
	if(gi == m_Games.end())
	{
		m_GamesLock.unlock();
		return "No Game Active";
	}

	QString v((*gi).second->Name());
	m_GamesLock.unlock();
	return v;
}

// Game settings are sent from the client in a byte array so they are generic between all game types.
void DaVessel::GameStart(DaPerson* dealer, quint8 gameType, const QByteArray& gameSettings)
{
	quint16 whichTable = dealer->StationId();
	m_GamesLock.lockForWrite();
	unordered_map<quint16, AuGame*>::iterator gi = m_Games.find(whichTable);
	if(gi == m_Games.end())
	{
		m_GamesLock.unlock();
		return;		// The game is not active.
	}

	if((*gi).second->Type() != gameType || !dealer->Is((*gi).second->PlayerId(0)))
	{
		// Settings blob is for the wrong kind of game or the dealer did not just start the game or he would still be player 0.
		// The game is no longer valid.
		delete (*gi).second;			// Delete the game.
		m_Games.erase(gi);	// Remove it from the list of active games.
	}
	else
	{
		// Insure the dealer is still at the table.
		if(!dealer->IsGameTablePose())
		{
			// Dealer left the table before setup is complete. The game is no longer valid.
			delete (*gi).second;			// Delete the game.
			m_Games.erase(gi);	// Remove it from the list of active games.
		}
		else
		{
			// Insure the players in the game object reflect the current people seated at the table.
			GameSetPlayers(whichTable, (*gi).second, dealer->Id());

			// Locate the players so they can be disqualified in Start if they don't have enough money to ante.
			DaPerson* players[MAX_GAME_PLAYERS];
			for(int player=0; player<MAX_GAME_PLAYERS; player++)
				if(!(*gi).second->PlayerId(player).IsNull())
					players[player] = dynamic_cast<DaPerson*>(FindNormalSubatomId((*gi).second->PlayerId(player)));
				else
					players[player] = 0;

			// Configure game settings, which may remove players that don't match those listed in the settings.
			// This allows the dealer to kick players during setup.
			if(dealer == players[0] && (*gi).second->Start(players, gameSettings))
			{
				// Notify the first player that it is their turn.
				emit GameTurnPoll(Id(), (*gi).first, (*gi).second->TurnId());
			}
			else
			{
				// The resulting setup is not valid. The game is no longer valid.
				delete (*gi).second;			// Delete the game.
				m_Games.erase(gi);	// Remove it from the list of active games.
			}
		}
	}
	GamesSetModified();
	m_GamesLock.unlock();
}

// Games only get ticked every even second. No table top games need faster response than that.
void DaVessel::GamesTick(bool validatePlayers)
{
	m_GamesLock.lockForWrite();
	if(m_Games.empty())
	{
		m_GamesLock.unlock();
		return;
	}

	//qDebug() << "DaVessel::GamesTick" << Id() << Name() << "validate players" << validatePlayers;

	bool anyGamesChanged = false;
	unordered_map<quint16, AuGame*>::iterator gi = m_Games.begin();
	while(gi != m_Games.end())
	{
		// Tick the game. This may change its state.
		int tickResult = (*gi).second->Tick();

		// Send a turn poll to the players periodically to remind them whose turn it is.
		if((*gi).second->IsTurning())
			emit GameTurnPoll(Id(), (*gi).first, (*gi).second->TurnId());			// Send them a reminder.
		
		// Validate all players are still present and in the game.
		if(validatePlayers)
		{
			for(int player=0; player<MAX_GAME_PLAYERS; player++)
			{
				AuId playerId = (*gi).second->PlayerId(player);
				if(!playerId.IsNull())
				{
					DaPerson* person = dynamic_cast<DaPerson*>(FindNormalSubatomId(playerId));
					if(!person || !person->IsGameTablePose())
					{
						// Player was not found or they are not in the proper pose for a game table.
						//qDebug() << "DaVessel::GamesTick" << Id() << Name() << "removing player" << playerId;
						if(!(*gi).second->RemovePlayer(playerId))
							tickResult = 2;						// No players left in the game. Game over.
					}
				}
			}
		}

		// Handle the result of the tick.
		switch(tickResult)
		{
		case 0:		// No data changed in the game object.
			gi++;
			break;
		case 1:		// Data changed in the game object.
			//qDebug() << "DaVessel::GamesTick" << Id() << Name() << "game changed";
			anyGamesChanged = true;
			gi++;
			break;
		case 2:		// Game Over
			// Locate the players so the winner(s) can be credited with their winnings.
			//qDebug() << "DaVessel::GamesTick" << Id() << Name() << "game over";
			DaPerson* players[MAX_GAME_PLAYERS];
			for(int player=0; player<MAX_GAME_PLAYERS; player++)
				if(!(*gi).second->PlayerId(player).IsNull())
					players[player] = dynamic_cast<DaPerson*>(FindNormalSubatomId((*gi).second->PlayerId(player)));
				else
					players[player] = 0;

			// Send the results and terminate the game object.
			// This also credits the winner(s) with their winnings.
			QByteArray gameResults = (*gi).second->Finish(players);
			emit GameOver(Id(), (*gi).first, (*gi).second->Type(), gameResults);

			// Remove the game from the list of active games.
			anyGamesChanged = true;
			delete (*gi).second;
			gi = m_Games.erase(gi);
			break;
		}
	}
	if(anyGamesChanged)
		GamesSetModified();
	m_GamesLock.unlock();
}

void DaVessel::GameTurn(DaPerson* person, quint8 gameType, const QByteArray& gameTurn)
{
	quint16 whichTable = person->StationId();
	m_GamesLock.lockForWrite();
	unordered_map<quint16, AuGame*>::const_iterator gi = m_Games.find(whichTable);
	if(gi == m_Games.end())
	{
		m_GamesLock.unlock();
		return;		// The game is not active.
	}
	if((*gi).second->Type() != gameType)
	{
		m_GamesLock.unlock();
		return;		// Game turn blob is for the wrong kind of game.
	}

	// Locate the players so they can be affected by games as they progress.
	DaPerson* players[MAX_GAME_PLAYERS];
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
		if(!(*gi).second->PlayerId(player).IsNull())
			players[player] = dynamic_cast<DaPerson*>(FindNormalSubatomId((*gi).second->PlayerId(player)));
		else
			players[player] = 0;

	// Send the game turn info to the game. This won't terminate the game but it might change its state.
	if((*gi).second->TurnComplete(person, players, gameTurn))
	{
		// Game data changed.
		GamesSetModified();

		// Notify the next person it's their turn. Don't wait for tock to get around to it, which could be 10 seconds from now.
		if(!person->Is((*gi).second->TurnId()))
			emit GameTurnPoll(Id(), (*gi).first, (*gi).second->TurnId());
	}
	m_GamesLock.unlock();
}

AuDialog* DaVessel::GameCreateTurnUI(quint16 whichTable, const AuId& personsTurn, QWidget* parent)
{
	DaPerson* player = dynamic_cast<DaPerson*>(FindSubatomId(personsTurn));
	if(!player)
		return 0;		// The player is not here.

	m_GamesLock.lockForRead();
	unordered_map<quint16, AuGame*>::const_iterator gi = m_Games.find(whichTable);
	if(gi == m_Games.end())
	{
		m_GamesLock.unlock();
		return 0;		// The game is not active.
	}
	if((*gi).second->TurnId() != personsTurn)
	{
		m_GamesLock.unlock();
		return 0;		// It's not the person's turn. The client may not have received game data yet. Wait for next call.
	}
	AuDialog* v = (*gi).second->CreateTurnUI(*player, parent);
	m_GamesLock.unlock();
	return v;
}

// Checks to see if the game turn UI is still appropriate for the current game state.
// Returns false if the turnUI should be deleted.
bool DaVessel::GameTurnUIValid(quint16 whichTable, const AuId& personsTurn, AuDialog* turnUI)
{
	Q_ASSERT(turnUI);
	m_GamesLock.lockForRead();
	unordered_map<quint16, AuGame*>::const_iterator gi = m_Games.find(whichTable);
	if(gi == m_Games.end())
	{
		m_GamesLock.unlock();
		return false;		// The game is not active.
	}
	if((*gi).second->TurnId() != personsTurn)
	{
		m_GamesLock.unlock();
		return false;		// It's not the person's turn.
	}

	// Game turn UIs are uniquely identified by name based on game state.
	bool v = turnUI->objectName() == (*gi).second->TurnUIName();
	m_GamesLock.unlock();
	return v;
}

// Can only set moorings when the ship is not moving and throttles are off and ship is in trade range of a station.
void DaVessel::Moor()
{
	if(IsMoored())
		return;	// Already moored.

	// Test same criteria for friction in space, so we'll grind to a stop when moored.
	if(ThrustLongitudinal() || ThrustLateral() || ThrustVertical())
		return;	// Throttles on disables space friction.
	AuPoint vec(Vector());
	if(fabs(vec.x) >= 2 || fabs(vec.y) >= 2 || fabs(vec.z) >= 2)
		return;	// Moving too fast for space friction.

	QString text;
	if(IsInSpacecraft())
	{
		// This ship is subordinate to another ship. It can moor if it is inside the ship and touching the floor.
		if(!IsInsideSpacecraft() || !IsOnGround())
			return;						// Not inside the ship or not touching the floor.
		const DaAtom* parent = Parent();
		if(!parent)
			return;
		const DaVessel* ship = parent->FindParentVesselInEnv();
		if(!ship)
			return;						// Not on a ship?
		QString name = ship->NameWithEmpire();
		text = tr("Moorings secured aboard %1, %3 %4.")
			.arg(name)
			.arg(ShipRoomName())
			.arg(ShipRoomId());
	}
	else if(IsInAtmosphere())
	{
		// This ship is inside the atmosphere envelope of a world.
		// Must be sitting on the ground.
		if(!IsOnGround())
			return;						// Not sitting on the ground.
		const DaWorld* planet = FindParentPlanetInEnv();
		if(!planet)
			return;						// Not on a planet?
		text = tr("Moorings secured at %1 on %2.").arg(GlobeLL().ToString()).arg(planet->Name());
	}
	else
	{
		// This ship is in space.
		// Must be in trade range of a non-enemy station.
		bool hasStation = false;
		const DaVessel* station = StationInTradeRange(RoughRadius(), hasStation);
		if(!station)
			return;						// Not in trade range of a non-enemy station.
		const DaWorld* planet = FindParentPlanetInEnv();
		if(!planet)
			text = tr("Moorings secured at %1.").arg(station->NameWithEmpire());
		else
			text = tr("Moorings secured at %1, in orbit over %2.").arg(station->NameWithEmpire()).arg(planet->Name());
	}

	// Ok to moor the ship.
	m_Moored = true;

	// Notify all hands that we're secure.
	AuCommChat msg(text, AuCommStdChannel::IntercomChannelName(), Id(), Name(), FindParentSector()->Id(),
			Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Intercom);
	g_Au.SendAtomChat(msg);
}

void DaVessel::Unmoor()
{
	/*
	// Can't unmoor while getting repairs.
	// This test cannot be made here because it conflicts with DaVessel::GoDormant.
	if(RepairSecondsRemaining())
		return;
		*/
	if(!IsMoored())
		return;							// Not currently moored.

	if(IsNew())
		SetNotNew();					// Ship is no longer "nu".

	// Ok to unmoor the ship.
	m_Moored = false;
	ForceMotion();			// Kick loose so gravity will start to affect the ship.
	SetModified();

	// Notify all hands that we're adrift.
	QString text = tr("Moorings cast off.");
	AuCommChat msg(text, AuCommStdChannel::IntercomChannelName(), Id(), Name(), FindParentSector()->Id(),
			Galaxy(), AuCommStdChannel::Close, AuCommStdChannel::Intercom);
	g_Au.SendAtomChat(msg);
}

bool DaVessel::InstallSystemModule(const AuSysId& sysId, const AuItem& item, quint16 installSecs)
{
	bool didInstall = false;
	switch(sysId.System())
	{
	case SYS_CommandAndControl:
	case SYS_ManeuverDrive:
	case SYS_LifeSupport:
	case SYS_Capacitor:
	case SYS_PowerPlant:
	case SYS_Sensor:
	case SYS_Shield:
		// These systems do not use a 'which' identifier in the ship systems array. Consoles get segregated by which in damage data.
		m_SystemsLock.lockForWrite();
		didInstall = m_ssaShipSystems.UpgradeSystem(sysId.System(), item.CommodityId(), item.Quality());
		if(didInstall)
			m_tmSystemsUpg.Set();
		m_SystemsLock.unlock();
		break;

	case SYS_Hull:
		{
			// Hull passes a raw material, not a module.
			COMMODITYID moduleCID = HullMaterialModule(item.CommodityId());
			if(moduleCID == CID_Invalid)
				moduleCID = item.CommodityId();	// This should not happen but this may be a safe fallback.

			// Hull system does not use a 'which' identifier.
			m_SystemsLock.lockForWrite();
			didInstall = m_ssaShipSystems.UpgradeSystem(sysId.System(), moduleCID, item.Quality());
			if(didInstall)
				m_tmSystemsUpg.Set();
			m_SystemsLock.unlock();
		}
		break;

	case SYS_FTLDrive:
	case SYS_SurgeryUnit:
	case SYS_Transporter:
	case SYS_VehicleLaunch:
	case SYS_VehicleLanding:
		m_SystemsLock.lockForWrite();
		didInstall = m_ssaShipSystems.UpgradeSystem(sysId.System(), sysId.Which(), item.CommodityId(), item.Quality());
		if(didInstall)
			m_tmSystemsUpg.Set();
		m_SystemsLock.unlock();
		break;

	case SYS_Turret:
		m_SystemsLock.lockForWrite();
		didInstall = m_ssaShipSystems.UpgradeSystem(sysId.System(), sysId.Which(), item.CommodityId(), item.Quality());
		if(didInstall)
			m_tmSystemsUpg.Set();
		m_SystemsLock.unlock();

		// Must forget current ammo selection for the turret.
		if(didInstall)
		{
			m_TurretLock.lockForWrite();
			unordered_map<quint16, AuOrdnance>::iterator ti = m_Turrets.find(sysId.Which());
			if(ti != m_Turrets.end())
				m_Turrets.erase(ti);
			m_TurretLock.unlock();
		}
		break;

	case SYS_Weapon:
		m_SystemsLock.lockForWrite();
		didInstall = m_ssaShipSystems.UpgradeSystem(sysId.System(), sysId.Which(), item.CommodityId(), item.Quality());
		if(didInstall)
			m_tmSystemsUpg.Set();
		m_SystemsLock.unlock();

		// Must forget current ammo selection for the bay.
		if(didInstall)
		{
			m_BayLock.lockForWrite();
			unordered_map<quint16, AuBaySetting>::iterator bi = m_BaySetting.find(sysId.Which());
			if(bi != m_BaySetting.end())
				m_BaySetting.erase(bi);
			m_BayLock.unlock();
		}
		break;

	case SYS_Fuel:
	case SYS_Hold:
	case SYS_Count:
		Q_ASSERT(false);	// Cannot upgrade these systems here.
		break;
	}

	if(didInstall)
	{
		//qDebug() << "didInstall secs" << installSecs;
		AddRepairTime(installSecs);
		SetModified();
	}
	return didInstall;
}

bool DaVessel::IsReadyForRepairs() const
{
	// Ship must be at rest to get repairs.
	if(IsMoving())
		return false;

	// This test is only used by AI officer when getting repairs to his ship.
	// AI crew exits their console when it is broken. So, we can expect the console to be manned if there is enough crew and the
	// console is not broken. We don't prevent repairs if the system cannot be turned off due to a broken console or insufficent
	// crew.

	// Power plant must be off.
	// Power plant can be operated at engineer console or helm.
	if(PowerPlantWorking() && (IsEngineerManned() || IsPilotManned()))
		return false;

	// Gravity drive must be off.
	// Gravity drive can be operated at engineer console or helm.
	if(GravityDriveWorking() && (IsEngineerManned() || IsPilotManned()))
		return false;

	// FTL drive must be off.
	// FTL drive can be operated at engineer console.
	if(IsAnyFTLDriveWorking() && IsEngineerManned())
		return false;

	// Sensors must be off.
	// Sensors can be operated at sensor console.
	if(SensorsWorking() && IsSensorManned())
		return false;

	// Shields must be off.
	// Shields can be operated at shield console.
	if(ShieldsWorkingAndOn() && IsShieldManned())
		return false;

	// Ran the gauntlet.
	// Must be ok to repair the ship.
	return true;
}

QString DaVessel::CommMessageLocation() const
{
	if(InArena())
	{
		if(FindParentArena())
			return FindParentArena()->Name();
		return tr("Arena.");
	}
	else if(InDesignEnvironment())
	{
		const DaDesignBase* de = FindParentDesignBaseInEnv();
		return de->Name();
	}

	const DaSolarSystem* system = FindParentSolarSystem();
	if(!system)
		return tr("Lost in limbo.");	// Not likely.
	QString systemName;
	if(system->IsEmptySystem())
		systemName = tr("deep space");
	else
		systemName = tr("the %1 system").arg(system->Name());
	const DaSector* sector = system->FindParentSector();
	if(!sector)
		return tr("Solar system lost in limbo.");	// Not likely.
	const DaGalaxy* galaxy = system->FindParentGalaxy();
	if(!galaxy)
		return tr("Sector lost in limbo.");	// Not likely.

	const DaVessel* ship = Parent()->FindParentVesselInEnv();
	const DaWorldLL* planet = FindParentWorldInEnv();
	if(planet)
	{
		AuPoint planetLoc;
		GetLocationRelativeToParent(planetLoc, planet);
		Q_ASSERT(!planetLoc.IsNaN());

		QString strLongLat;
		AuGlobeLLF planetLL;
		if(planet->LLOfLoc(planetLL, planetLoc))
			strLongLat = planetLL.ToString();

		if(ship)
		{
			QString liquid;
			switch(planet->OrbitZone())
			{
			case OZONE_Inferno:
				liquid = tr("lava");
				break;
			case OZONE_Inner:
			case OZONE_Habitable:
			case OZONE_Outer:
				liquid = tr("water");
				break;
			case OZONE_Frigid:
				liquid = tr("supercooled liquid");
				break;
			case OZONE_DeepSpace:
				break;
			}

			if(ship->IsOnGround())
				return tr("This ship is aboard %7 on the ground on %2 at %3 in the %4 %5, %6.")
					.arg(planet->Name())
					.arg(strLongLat)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(ship->NameWithEmpire());
			else if(ship->IsInWater())
				return tr("This ship is aboard %7 in the %1 on %2 at %3 in the %4 %5, %6.")
					.arg(liquid)
					.arg(planet->Name())
					.arg(strLongLat)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(ship->NameWithEmpire());
			else if(ship->IsInAtmosphere())
				return tr("This ship is aboard %7 flying over %2 at %3 in the %4 %5, %6.")
					.arg(planet->Name())
					.arg(strLongLat)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(ship->NameWithEmpire());
			else if(!ship->IsMoving())
				return tr("This ship is aboard %7 orbiting %2 at %3 in the %4 %5, %6.")
					.arg(planet->Name())
					.arg(strLongLat)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(ship->NameWithEmpire());
			else
				return tr("This ship is aboard %7 maneuvering near %2 at %3 in the %4 %5, %6.")
					.arg(planet->Name())
					.arg(strLongLat)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(ship->NameWithEmpire());
		}
		else if(IsInWater())
		{
			if(dynamic_cast<const DaWorldGasGiant*>(planet))
			{
				if(HullStreamline() == SLINE_Submersible)
				{
					switch(planet->AtmosphereType())
					{
					default:
					case ATMTYPE_Standard:
					case ATMTYPE_Tainted:
					case ATMTYPE_Exotic:
						return tr("We are submerged in the icy soup on %2 at %3 in the %4 %5, %6.")
							.arg(planet->Name())
							.arg(strLongLat)
							.arg(sector->Name()).arg(sector->Description())
							.arg(galaxy->Name());
						break;
					case ATMTYPE_Corrosive:
						return tr("We are submerged in the corrosive soup on %2 at %3 in the %4 %5, %6.")
							.arg(planet->Name())
							.arg(strLongLat)
							.arg(sector->Name()).arg(sector->Description())
							.arg(galaxy->Name());
						break;
					case ATMTYPE_Insidious:
						return tr("We are submerged in the insidious soup on %2 at %3 in the %4 %5, %6.")
							.arg(planet->Name())
							.arg(strLongLat)
							.arg(sector->Name()).arg(sector->Description())
							.arg(galaxy->Name());
						break;
					}
				}
				else
				{
					switch(planet->AtmosphereType())
					{
					default:
					case ATMTYPE_Standard:
					case ATMTYPE_Tainted:
					case ATMTYPE_Exotic:
						return tr("We are freezing in the icy soup on %2 at %3 in the %4 %5, %6.")
							.arg(planet->Name())
							.arg(strLongLat)
							.arg(sector->Name()).arg(sector->Description())
							.arg(galaxy->Name());
						break;
					case ATMTYPE_Corrosive:
						return tr("We are dissolving in the corrosive soup on %2 at %3 in the %4 %5, %6.")
							.arg(planet->Name())
							.arg(strLongLat)
							.arg(sector->Name()).arg(sector->Description())
							.arg(galaxy->Name());
						break;
					case ATMTYPE_Insidious:
						return tr("We are disintegrating in the insidious soup on %2 at %3 in the %4 %5, %6.")
							.arg(planet->Name())
							.arg(strLongLat)
							.arg(sector->Name()).arg(sector->Description())
							.arg(galaxy->Name());
						break;
					}
				}
			}
			else
			{
				switch(OrbitZone())
				{
				case OZONE_Inferno:
					return tr("We are melting in the lava on %2 at %3 in the %4 %5, %6.")
						.arg(planet->Name())
						.arg(strLongLat)
						.arg(sector->Name()).arg(sector->Description())
						.arg(galaxy->Name());
				case OZONE_Inner:
					if(HullStreamline() == SLINE_Submersible)
					{
						return tr("We are submerged in hot water on %2 at %3 in the %4 %5, %6.")
							.arg(planet->Name())
							.arg(strLongLat)
							.arg(sector->Name()).arg(sector->Description())
							.arg(galaxy->Name());
					}
					else
					{
						return tr("We are rusting in hot water on %2 at %3 in the %4 %5, %6.")
							.arg(planet->Name())
							.arg(strLongLat)
							.arg(sector->Name()).arg(sector->Description())
							.arg(galaxy->Name());
					}
				case OZONE_Habitable:
				default:
					if(HullStreamline() == SLINE_Submersible)
					{
						return tr("We are submerged in the water on %2 at %3 in the %4 %5, %6.")
							.arg(planet->Name())
							.arg(strLongLat)
							.arg(sector->Name()).arg(sector->Description())
							.arg(galaxy->Name());
					}
					else
					{
						return tr("We are rusting in the water on %2 at %3 in the %4 %5, %6.")
							.arg(planet->Name())
							.arg(strLongLat)
							.arg(sector->Name()).arg(sector->Description())
							.arg(galaxy->Name());
					}
				case OZONE_Outer:
					if(HullStreamline() == SLINE_Submersible)
					{
						return tr("We are submerged in cold water on %2 at %3 in the %4 %5, %6.")
							.arg(planet->Name())
							.arg(strLongLat)
							.arg(sector->Name()).arg(sector->Description())
							.arg(galaxy->Name());
					}
					else
					{
						return tr("We are rusting in cold water on %2 at %3 in the %4 %5, %6.")
							.arg(planet->Name())
							.arg(strLongLat)
							.arg(sector->Name()).arg(sector->Description())
							.arg(galaxy->Name());
					}
				case OZONE_Frigid:
					if(HullStreamline() == SLINE_Submersible)
					{
						return tr("We are submerged in the icy soup on %2 at %3 in the %4 %5, %6.")
							.arg(planet->Name())
							.arg(strLongLat)
							.arg(sector->Name()).arg(sector->Description())
							.arg(galaxy->Name());
					}
					else
					{
						return tr("We are freezing in the icy soup on %2 at %3 in the %4 %5, %6.")
							.arg(planet->Name())
							.arg(strLongLat)
							.arg(sector->Name()).arg(sector->Description())
							.arg(galaxy->Name());
					}
				}
			}
		}
		else if(IsOnGround())
			return tr("We are on the ground on %2 at %3 in the %4 %5, %6.")
				.arg(planet->Name())
				.arg(strLongLat)
				.arg(sector->Name()).arg(sector->Description())
				.arg(galaxy->Name());
		else if(IsInAtmosphere())
			return tr("We are flying over %2 at %3 in the %4 %5, %6.")
				.arg(planet->Name())
				.arg(strLongLat)
				.arg(sector->Name()).arg(sector->Description())
				.arg(galaxy->Name());
		else if(!IsMoving())
			return tr("We are orbiting %2 at %3 in the %4 %5, %6.")
				.arg(planet->Name())
				.arg(strLongLat)
				.arg(sector->Name()).arg(sector->Description())
				.arg(galaxy->Name());
		else
			return tr("We are maneuvering near %2 at %3 in the %4 %5, %6.")
				.arg(planet->Name())
				.arg(strLongLat)
				.arg(sector->Name()).arg(sector->Description())
				.arg(galaxy->Name());
	}
	else if(FindParentStarInEnv())
	{
		if(ship)
		{
			if(!ship->IsMoving())
				return tr("This ship is aboard %7 holding position in %2 in the %3 %4, %6.")
					.arg(systemName)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(ship->NameWithEmpire());
			else
				return tr("This ship is aboard %7 traveling through %2 in the %3 %4, %6.")
					.arg(systemName)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(ship->NameWithEmpire());
		}
		else if(!IsMoving())
			return tr("We are holding position in %2 in the %3 %4, %6.")
				.arg(systemName)
				.arg(sector->Name()).arg(sector->Description())
				.arg(galaxy->Name());
		else
			return tr("We are traveling through %2 in the %3 %4, %6.")
				.arg(systemName)
				.arg(sector->Name()).arg(sector->Description())
				.arg(galaxy->Name());
	}
	else
	{
		if(!system->IsEmptySystem())
			systemName = tr("deep space in the %1 system").arg(system->Name());
		AuPoint galaxyLoc;
		if(ship)
		{
			ship->GetLocationRelativeToParent(galaxyLoc, galaxy);
			if(!ship->IsMoving())
				return tr("This ship is aboard %10 holding position in %2 in the %3 %4, %6 %7, %8, %9.")
					.arg(systemName)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(galaxyLoc.x,0,'f',4).arg(galaxyLoc.y,0,'f',4).arg(galaxyLoc.z,0,'f',4)
					.arg(ship->NameWithEmpire());
			else
				return tr("This ship is aboard %10 traveling through %2 in the %3 %4, %6 %7, %8, %9.")
					.arg(systemName)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(galaxyLoc.x,0,'f',4).arg(galaxyLoc.y,0,'f',4).arg(galaxyLoc.z,0,'f',4)
					.arg(ship->NameWithEmpire());
		}
		else
		{
			GetLocationRelativeToParent(galaxyLoc, galaxy);
			if(!IsMoving())
				return tr("We are holding position in %2 in the %3 %4, %6 %7, %8, %9.")
					.arg(systemName)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(galaxyLoc.x,0,'f',4).arg(galaxyLoc.y,0,'f',4).arg(galaxyLoc.z,0,'f',4);
			else
				return tr("We are traveling through %2 in the %3 %4, %6 %7, %8, %9.")
					.arg(systemName)
					.arg(sector->Name()).arg(sector->Description())
					.arg(galaxy->Name())
					.arg(galaxyLoc.x,0,'f',4).arg(galaxyLoc.y,0,'f',4).arg(galaxyLoc.z,0,'f',4);
		}
	}
}

// Used by AI to turn off one overclocked system that is not damaged, if any.
// Returns true if anything is changed.
bool DaVessel::TurnOffOneOverclock()
{
	//Q_ASSERT(!g_Au.IsSceneClient());	// Scene client does not rx enough m_ssaShipSystems data to do this.
	int systemIndex = 0;
	m_SystemsLock.lockForWrite();
	for(AuShipSystemArray::iterator si = m_ssaShipSystems.begin(); si != m_ssaShipSystems.end(); si++, systemIndex++)
	{
		if((*si).GetOverclocking() && (*si).GetHealthCurrent() == (*si).GetHealthMax())
		{
			// Found an undamaged system that is overclocked.
			(*si).OverclockSupplier(false);
			UpdateSystemsPowerSystemsLocked();
			m_SystemsLock.unlock();
			SetModified();
			return true;
		}
	}
	m_SystemsLock.unlock();
	return false;
}

bool DaVessel::TurnOffOneShunt()
{
	//Q_ASSERT(!g_Au.IsSceneClient());	// Scene client does not rx enough m_ssaShipSystems data to do this.
	quint8 shuntsOn = GetShunts();
	if(shuntsOn && !HitsDamageSystem(SYS_PowerPlant))
	{
		// A shunt is on but the power plant is not damaged. Turn off a shunt.
		for(int shuntIndex=0; shuntIndex<8; shuntIndex++)
		{
			int shuntBit = 1<<shuntIndex;
			if(shuntBit & shuntsOn)
			{
				PowerPlantShuntOff(shuntIndex);
				return true;
			}
		}
	}
	return false;
}

// Used by AI to reconnect one disconnected system at the power relay console.
// Returns true if anything is changed.
bool DaVessel::ConnectOneSystem()
{
	//Q_ASSERT(!g_Au.IsSceneClient());	// Scene client does not rx enough m_ssaShipSystems data to do this.
	int systemIndex = 0;
	m_SystemsLock.lockForWrite();
	for(AuShipSystemArray::iterator si = m_ssaShipSystems.begin(); si != m_ssaShipSystems.end(); si++, systemIndex++)
	{
		if((*si).GetConnectedConsumer() < 0)
		{
			// Found a system that is not connected to anything.
			(*si).SetConnectedConsumer(systemIndex);
			m_ssaShipSystems.RecalculatePowerConsumption(systemIndex);
			m_tmSystemsUpg.Set();
			m_SystemsLock.unlock();
			SetModified();
			return true;
		}
	}
	m_SystemsLock.unlock();
	return false;
}

bool DaVessel::EmergeFromWormhole(DaSolarSystem* inSystem)
{
	//qDebug() << "DaVessel::EmergeFromWormhole" << *this;
	if(inSystem->IsOnlyStarFieldLoaded())
	{
		if(g_Au.m_LogMessages & LOG_Error)
			qDebug("DaVessel::EmergeFromWormhole %d %s system is starfield only", Id().GetID(), (const char*)Name().toLatin1());
		return false;
	}
	else if(inSystem->IsEmptySystem())
		return false;	// No wormholes in empty solar system.

	// Ship may have emerged from a wormhole in the solar system. There might not be a star if this was a one way intergalactic
	// wormhole.
	DaStar* ps = inSystem->FindPrimaryStar();	// Could be a black hole.
	DaStar* star = inSystem->FindPrimaryStarNotBlackHole();

	// If the ship is not inside the primary star, it is not emerging from a wormhole and needs no further processing here.
	double dist = Location().Length();
	if(ps && dist > ps->Radius())
		return false;											// The ship is not inside the primary star.

	int exitType;		// 0 = normal wormhole, 1 = wormhole but distance is wrong, 2 = galaxy coord
	if(dist == 0 || FuzzyEquals(dist,6))
	{
		// When appearing at the center of the sun, this could be an agent teleport to a galaxy coord. Don't complain.
		exitType = 2;	// Galaxy coord
	}
	else if(!FuzzyEquals(dist,1) && !FuzzyEquals(dist,2) && !FuzzyEquals(dist,3) && !FuzzyEquals(dist,4) && !FuzzyEquals(dist,5))
	{
		// Ship distance is bad but we need to get it out of the sun.
		if(!ps)
			return false;	// No primary star
		exitType = 1;	// normal wormhole exit but distance is wrong
		//if((g_Au.m_LogMessages & LOG_Error))
			//qWarning() << "DaVessel::EmergeFromWormhole" << Id() << Name() << "at star" << ps->Id() << "distance"
				//<< dist << "is incorrect";
	}
	else
		exitType = 0;	// normal wormhole exit

	// Mark this system as explored by the empire owning the ship.
	inSystem->ExploredBy(EmpireId());

	// Notify the system of the ship's quality for pirate encounters.
	if(!IsPirate())
		inSystem->SetNonPirateShipPresent(HighestSystemQuality());

	// Determine exit distance from wormhole based on polarity.
	AuWormhole::POLARITY exitPolarity;
	double rdist = 0;
	if(FuzzyEquals(dist,1))
	{
		// ship entered positive wormhole
		if(!star)
			return false;
		exitPolarity = AuWormhole::Negative;
		rdist = star->Radius() * 2.f * 1.1f ;
		if(g_Au.m_LogMessages & LOG_Travel)
			qDebug("DaVessel::EmergeFromWormhole %d %s exiting negative wormhole", Id().GetID(), (const char*)Name().toLatin1());
	}
	else if(FuzzyEquals(dist,2))
	{
		// ship entered negative wormhole
		if(!star)
			return false;
		exitPolarity = AuWormhole::Positive;
		rdist = star->Radius() * 2.f;
		if(g_Au.m_LogMessages & LOG_Travel)
			qDebug("DaVessel::EmergeFromWormhole %d %s exiting positive wormhole", Id().GetID(), (const char*)Name().toLatin1());
	}
	else if(FuzzyEquals(dist,3))
	{
		// ship entered intergalactic wormhole
		if(!ps)
			return false;
		exitPolarity = AuWormhole::Intergalactic;
		rdist = ps->IntergalacticWormholeDistance();
		star = ps;	// Emergent wormhole is at the blackhole.
		if(g_Au.m_LogMessages & LOG_Travel)
			qDebug("DaVessel::EmergeFromWormhole %d %s exiting intergalactic wormhole",
					Id().GetID(), (const char*)Name().toLatin1());

		// No waypoints set in the last galaxy could have any relevance in this galaxy.
		RemoveAllWaypoints();
	}
	else if(FuzzyEquals(dist,4))
	{
		// ship entered positive stargate.
		exitPolarity = AuWormhole::StargateNegative;
		DaStar* stargateStar = inSystem->FindStargateStar(false);	// Look for star with destination negative stargate.
		if(stargateStar)
			star = stargateStar;
		if(!star)
			return false;
		rdist = star->Radius() * 1.1;
		if(g_Au.m_LogMessages & LOG_Travel)
			qDebug("DaVessel::EmergeFromWormhole %d %s exiting negative stargate", Id().GetID(), (const char*)Name().toLatin1());
	}
	else if(FuzzyEquals(dist,5))
	{
		// ship entered negative stargate.
		exitPolarity = AuWormhole::StargatePositive;
		DaStar* stargateStar = inSystem->FindStargateStar(true);	// Look for star with origin positive stargate.
		if(stargateStar)
			star = stargateStar;
		if(!star)
			return false;
		rdist = star->Radius() * 1.1;
		if(g_Au.m_LogMessages & LOG_Travel)
			qDebug("DaVessel::EmergeFromWormhole %d %s exiting positive stargate", Id().GetID(), (const char*)Name().toLatin1());
	}
	else if(FuzzyEquals(dist,6))
	{
		// ship entered one way intergalactic wormhole
		exitPolarity = AuWormhole::Invalid;
		rdist = FeetPerSolarSystem()/2;
		star = ps;	// could be 0
		if(g_Au.m_LogMessages & LOG_Travel)
			qDebug("DaVessel::EmergeFromWormhole %d %s exiting one way intergalactic wormhole",
					Id().GetID(), (const char*)Name().toLatin1());

		// No waypoints set in the last galaxy could have any relevance in this galaxy.
		RemoveAllWaypoints();
	}
	else
	{
		// ship teleported to a galaxy location
		exitPolarity = AuWormhole::Invalid;
		rdist = FeetPerSolarSystem()/2;
		star = ps;	// could be 0
		if(g_Au.m_LogMessages & LOG_Travel)
			qDebug("DaVessel::EmergeFromWormhole %d %s exiting teleport to galaxy location",
					Id().GetID(), (const char*)Name().toLatin1());

		// No waypoints set in the last galaxy could have any relevance in this galaxy.
		RemoveAllWaypoints();
	}

	// Position the ship outside the appropriate wormhole.
	// Locate the wormhole pointing in the opposite direction of the ship's position vector.
	// Find a safe place for the ship to emerge that does not hit a planet, star or ship.
	AuMatrix4x4 t;
	AuPoint vy(Location());										// Exit vector is relative to the solar system, even if black hole.
	if(!vy.IsZero())
		vy /= dist;												// Make vy into a unit vector.
	else
		vy = AuPoint(0,0,1);
	AuPoint fdest = vy * (rdist + 10.0);						// Start just beyond wormhole. Location relative to system&primary.
	if(star)
	{
		fdest += star->Location();								// Offset to companion primary location in black hole system.

		// Make a map of the spheres we need to avoid on the way out, sorted by distance from the center of the solar system.
		QMultiMap<double, AuSphere> avoidSpheres;				// Key is squared distance from center of solar system.
		const double rad = RoughRadius();						// The rough radius of this ship.

		// Avoid stars.
		if(ps)
		{
			DaAtomList stars;
			ps->ListStars(stars);
			for(DaAtomList::const_iterator si = stars.begin(); si != stars.end(); si++)
			{
				const DaStar* astar = dynamic_cast<const DaStar*>(*si);
				if(!astar)
					continue;

				// Companion location is relative to primary star.
				avoidSpheres.insert(astar->Location().Length2(), AuSphere(astar->Location(),
							astar->AtmosphereRadius()+FeetPerMoonOrbit()+rad));
			}
		}

		// Avoid planets.
		{
			DaAtomList planets;
			inSystem->FindPlanetsRecursive(planets);
			for(DaAtomList::const_iterator pi = planets.begin(); pi != planets.end(); pi++)
			{
				const DaWorld* planet = dynamic_cast<const DaWorld*>(*pi);
				if(!planet)
					continue;

				// We never hit rings or ringworlds when emerging from a wormhole. Just check for spheres and gas giants.
				const DaWorldSphere* llworld = dynamic_cast<const DaWorldSphere*>(planet);
				if(!llworld)
					continue;

				// Calculate location relative to the primary star.
				AuPoint sysLoc = planet->EnvLocation();
				avoidSpheres.insert(sysLoc.Length2(), AuSphere(sysLoc, planet->AtmosphereRadius() + FeetPerMoonOrbit() + rad));
			}
		}

		// Avoid spacecraft.
		{
			DaAtomList ships;
			inSystem->FindVesselsRecursive(ships);
			for(DaAtomList::const_iterator ssi = ships.begin(); ssi != ships.end(); ssi++)
			{
				const DaVessel* ship = dynamic_cast<const DaVessel*>(*ssi);
				if(!ship)
					continue;
				if(ship == this)
					continue;									// Avoid myself.
				if(ship->IsInAtmosphere())
					continue;									// We will be avoiding the whole planet.
				// Ships can hang out of other ships.
				//if(ship->IsInSpacecraft())
					//continue;									// We will be avoiding the parent spacecraft.

				// Calculate location relative to the solar system/primary star.
				AuPoint sysLoc = ship->EnvLocation();
				double shipDist2 = sysLoc.Length2();
				if(shipDist2 > FeetPerSolarSystem()/2 * FeetPerSolarSystem()/2)	// Zone-of-control of primary star.
					continue;									// Ship is in deep space.
				avoidSpheres.insert(shipDist2, AuSphere(sysLoc, ship->RoughRadius() + 100*FeetPerMeter() + rad));
			}
		}

		// Find a wormhole exit location that does not land inside a sphere.
		// The QMap sorted them by distance from the primary star for me. Now I can work my way outward, avoiding each nu
		// sphere that I might push the location into.
		AuPoint farSide = vy * FeetPerSolarSystem();			// Long segment so intersect1 point is not inside the sphere.
		farSide += star->Location();							// Offset to companion primary location in black hole system.
		for(QMultiMap<double, AuSphere>::const_iterator asi = avoidSpheres.cbegin(); asi != avoidSpheres.cend(); asi++)
		{
			// Test destination location inside sphere.
			// Compare squared distance from destination to center of sphere with the squared radius of the sphere.
			if(fdest.Distance2((*asi).Center()) > (*asi).Radius2())
				continue;										// Sphere is not a factor though we could hurtle into it.

			// Determine the point of intersection with the sphere that is farthest from the primary sun.
			AuPoint intersect0, intersect1;
			int result = (*asi).Intersect(star->Location(), farSide, intersect0, intersect1);
			if(result == 1)
			{
				// Intersect segment is tangent or inside the sphere at either end.
				// This result only matters if the starting end is inside the obstacle sphere, in which case the intersect point
				// is the exit point from the obstacle sphere.
				// The starting point was (0,0) so the distance from the starting point to the obstacle sphere is the length of the
				// obstacle sphere center point vector.
				if((*asi).Center().Length2() > (*asi).Radius2())
					continue;									// Starting end is not inside the obstacle sphere.
				fdest = intersect0;								// The only intersection is the exit point from the obstacle sphere.
			}
			else if(result == 2)
				fdest = intersect1;								// Second intersect is the far side of the obstacle sphere.
		}

		// Make the ship subordinate to the primary star or the black hole.
		// Calculate transformation for ship.
		fdest = star->Transform().UnProject(fdest);					// From system coords to star coords.
		AuPoint vyd = star->Transform().UnProjectVector(vy);
		AuPoint vx;
		vyd.ArbitraryAxis(vx);
		vx = -vx;
		t.Translate(fdest);
		t.Rotate(AuPoint::CalculateRotation(vx,vyd));
	}
	else
	{
		// The ship will be subordinate to the solar system.
		// Calculate transformation for ship.
		AuPoint vx;
		vy.ArbitraryAxis(vx);
		vx = -vx;
		t.Translate(fdest);
		t.Rotate(AuPoint::CalculateRotation(vx,vy));
	}

	if(exitType == 2)
	{
		// Exit at galaxy coord complete, no wormhole there. Could be exit from one-way intergalactic wormhole.
		// There might or might not be a star here.
		AuWormhole wh(QString(), AuWormhole::IntergalacticOneWay, inSystem->Id(), AuPoint());
		TraversedWormhole(inSystem, wh);
		if(star)
			ForcePosMove(AuAtomPos(star->Id(), t), AuPoint(0,0,0));
		else
			ForcePosMove(AuAtomPos(inSystem->Id(), t), AuPoint(0,0,0));
		InvalidateEnvironment();
		SetModified();
		return true;
	}

	// Remaining wormhole exits must have a star.
	if(!star)
		return false;

	// Calculate exit speed of vessel.
	// Exit speed is orbit number in meters per second.
	// That way the speed shown on the HUD is the distance from the wormhole to the sun, in orbits.
	// That's actually too slow so the final speed is x100.
	double exitSpeed = rdist / FeetPerOrbit() * FeetPerMeter();
	exitSpeed *= 100.;
	ForcePosMove(AuAtomPos(star->Id(), t), AuPoint(0,exitSpeed,0));
	InvalidateEnvironment();
	SetModified();

	// Exit stargate is complete. The wormhole is too transient to mark explored. We don't care if it doesn't exist.
	if(exitPolarity == AuWormhole::StargatePositive || exitPolarity == AuWormhole::StargateNegative)
		return true;

	// Give the captain credit for exploring the wormhole.
	AuWormhole wh = inSystem->FindWormholeDir(-vy, exitPolarity);
	if(exitType == 1 && !wh.IsValid())
	{
		// The distance is wrong. Try the opposite polarity; maybe we'll get lucky.
		if(exitPolarity == AuWormhole::Positive)
			exitPolarity = AuWormhole::Negative;
		else if(exitPolarity == AuWormhole::Negative)
			exitPolarity = AuWormhole::Positive;
		else if(exitPolarity == AuWormhole::StargatePositive)
			exitPolarity = AuWormhole::StargateNegative;
		else
			exitPolarity = AuWormhole::StargatePositive;
		wh = inSystem->FindWormholeDir(-vy, exitPolarity);
		if(wh.IsValid())
			qDebug() << "DaVessel::EmergeFromWormhole" << *this << "wrong distance" << dist << "succeeded using exitPolarity"
				<< exitPolarity;
	}

	if(!wh.IsValid())
	{
		// No valid wormhole found for wormhole emergence.
		// Determine the system id that the ship came from. The ship's exit vector points away from it. It is almost
		// certainly in a neighboring sector since this situation doesn't occur when jumping within the same sector.
		const DaGalaxy* pg = inSystem->FindParentGalaxy();
		if(!pg)
			return false;

		// Get the id of the destination solar system and the proper length destination vector.
		// Vector is input as a unit vector in the coordinate space of the sector; it is returned at the length to the destination,
		// in parsecs.
		AuMatrix4x4 sysRot;
		sysRot.Rotate(inSystem->Rotation());
		AuPoint vecToDest = sysRot.Project(-vy);
		vecToDest.Unit();

		// Create the emergent wormhole in this system that was missing.
		AuId systemId = pg->WormholeDestinationSolarSystem(inSystem, vecToDest);
		if(!systemId.IsNull())
		{
			if(g_Au.m_LogMessages & LOG_Error)
				qDebug("DaVessel::EmergeFromWormhole %d emergent %s wormhole not found, creating one to system %d, my sector is %d",
						inSystem->Id().GetID(), (exitPolarity == AuWormhole::Positive)?"Positive":"Negative",
						systemId.GetID(), inSystem->ParentId().GetID());
			inSystem->AddWormholeConnection(systemId, vecToDest, exitPolarity);
			wh = inSystem->FindWormholeDir(-vy, exitPolarity);
		}
		//else if(g_Au.m_LogMessages & LOG_Error)
			//qDebug("DaVessel::EmergeFromWormhole %d no destination system. Could not create emergent %s wormhole.",
					//inSystem->Id().GetID(), (exitPolarity == AuWormhole::Positive)?"Positive":"Negative");
	}

	if(wh.IsValid())
	{
		if(g_Au.m_LogMessages & LOG_Travel)
			qDebug("DaVessel::EmergeFromWormhole %d %s valid emergent wormhole found", Id().GetID(),
					(const char*)Name().toLatin1());
		inSystem->WormholeExplored(wh, this, true);

		// Update the ship's route if the next waypoint is the emergent wormhole.
		TraversedWormhole(inSystem, wh);
	}
	return true;
}

void DaVessel::FindRepairBds(DaAtomList& repairBds)
{
	if(InArena())
		return;
	if(InDesignEnvironment())
		return;
	if(IsInAtmosphere())
	{
		// Ship is in an atmosphere. Locate the city this ship is in.
		if(!IsOnGround())
			return;						// Ship is not on the ground.
		if(BdId().IsNull())
			return;						// Ship is not at a bd building site or geo road.
		// Ship is sitting on the ground at a building site or geo road.
		// Allow repairs at quality of lowest system.
		DaWorld* planet = FindParentPlanetInEnv();
		if(planet)
		{
			// The bd city must have an airport terminal and an airport repair shop.
			DaAtomList bdCity;
			planet->FindBdCity(bdCity, InBdCity());
			for(DaAtomList::const_iterator bi = bdCity.cbegin(); bi != bdCity.cend(); bi++)
			{
				DaBd* bd = dynamic_cast<DaBd*>(*bi);
				if(!bd)
					continue;
				if(bd->IndustryId() != IID_AirportRepairShop)
					continue;
				if(!bd->CanAccessByGround(this))
					continue;
				// This ship's current city can provide repairs. Is repair restricted for export to this ship?
				// CID_Sunlight = 'Repair' in empire commerce restrictions data.
				quint32 empireId = bd->EmpireId();
				const DgData& gd = g_Au.LockDataForRead();
				POLITICAL_STANCE stanceToShip = gd.EmpirePoliticalStance(empireId, EmpireId());
				bool isRestricted = gd.EmpireCommerceRestricted(empireId, CID_Sunlight, stanceToShip);
				g_Au.UnlockData();
				if(!isRestricted)
					repairBds.append(bd);						// Shop is ok with repairing this ship.
				break;
			}
		}
	}
	else
	{
		// Ship is in space.
		// Locate station within trade range of this ship.
		bool hasStation;
		DaVessel* station = StationInTradeRange(RoughRadius(), hasStation);
		if(station)
		{
			// Ship is in trade range of a station.
			DaSolarSystem* system = station->FindParentSolarSystem();
			DaAtomList bds;
			system->FindBdRecursive(bds);

			// Find buildings that can and will repair this ship.
			for(DaAtomList::const_iterator bi = bds.begin(); bi != bds.end(); bi++)
			{
				DaBd* bd = dynamic_cast<DaBd*>(*bi);
				if(!bd)
					continue;
				if(bd->IndustryId() != IID_AirportRepairShop)
					continue;
				if(!bd->HasAirportAccess())
					continue;	// Repair shop must have an airport to provide repairs via station.
				// The city can provide repairs. Is repair restricted for export to this ship?
				// CID_Sunlight = 'Repair' in empire commerce restrictions data.
				quint32 empireId = bd->EmpireId();
				const DgData& gd = g_Au.LockDataForRead();
				POLITICAL_STANCE stanceToShip = gd.EmpirePoliticalStance(empireId, EmpireId());
				bool isRestricted = gd.EmpireCommerceRestricted(empireId, CID_Sunlight, stanceToShip);
				g_Au.UnlockData();
				if(!isRestricted)
					repairBds.append(bd);						// Shop is ok with repairing this ship.
			}
		}
	}
}

// Lists all buildings that this ship can reach for trade, via ground, station, or transporter.
void DaVessel::FindReachableBds(DaAtomList& reachableBds) const
{
	if(InArena())
		return;
	if(InDesignEnvironment())
		return;

	// Ships can trade with cities at the same planet that are within transporter range.
	const DaWorld* planet = FindParentPlanetInEnv();
	float transporterRange = 0;
	AuPoint shipLoc;											// Ship's location relative to planet, if it has transporters.
	if(planet)
	{
		transporterRange = MaxTransporterRange();
		if(transporterRange > 0)
		{
			transporterRange *= transporterRange;				// squared for faster test against squared distance
			GetLocationRelativeToParent(shipLoc, planet);
		}
	}

	if(IsInAtmosphere())
	{
		// Ship is in an atmosphere.
		if(IsOnGround() && !BdId().IsNull())
		{
			// Ship is sitting on the ground at a bd.
			DaAtomList bdCity;
			planet->FindBdCity(bdCity, InBdCity());				// All vessels can load from cities.
			if(!IsPrivatelyOwned())
				planet->FindBdBase(bdCity);						// Fleet vessels can load from bases too.
			for(DaAtomList::const_iterator bi = bdCity.cbegin(); bi != bdCity.cend(); bi++)
			{
				DaBd* bd = dynamic_cast<DaBd*>(*bi);
				if(!bd)
					continue;
				if(!bd->CanAccessByGround(this))
					continue;
				reachableBds.append(bd);						// Building is accessible to the ship by ground.
			}
		}

		if(transporterRange > 0)
		{
			// The ship has transporters. Check for buildings at the planet that are in transporter range.
			DaAtomList bds;
			planet->FindBds(bds);
			for(DaAtomList::const_iterator bi = bds.begin(); bi != bds.end(); bi++)
			{
				DaBd* bd = dynamic_cast<DaBd*>(*bi);
				if(!bd)
					continue;
				// The building is at the same planet as this ship. Check transporter range.
				if(bd->Location().Distance2(shipLoc) > transporterRange)
					continue;	// Building is beyond transporter range.
				// Building is accessible to the ship by transporter.
				if(!reachableBds.contains(bd))
					reachableBds.append(bd);
			}
		}
	}
	else
	{
		// Ship is in space.
		bool hasStation;
		DaVessel* station = StationInTradeRange(RoughRadius(), hasStation);	// Locate station within trade range of this ship.
		if(station || transporterRange > 0)
		{
			// Ship is in trade range of a station or the ship has transporters.
			const DaSolarSystem* system = FindParentSolarSystem();
			DaAtomList bds;
			system->FindBdRecursive(bds);

			// Find buildings that can trade with this ship.
			for(DaAtomList::const_iterator bi = bds.begin(); bi != bds.end(); bi++)
			{
				DaBd* bd = dynamic_cast<DaBd*>(*bi);
				if(!bd)
					continue;
				// If there is no station or the building has no airport, the building must be in transporter range to trade.
				if(!station || !bd->HasAirportAccess())
				{
					// Not at a station or the building has no airport access. Must be in transporter range to trade.
					if(bd->FindParentPlanetInEnv() == planet)
					{
						// The building is at the same planet as this ship. Check transporter range.
						if(bd->Location().Distance2(shipLoc) > transporterRange)
							continue;	// Building is beyond transporter range.
					}
					else
						continue;		// Not at same planet so can't transport cargo.
				}

				// The building is accessible to the ship.
				reachableBds.append(bd);	// City is ok with exporting the commodity to this ship.
			}
		}
	}
}

// CID_Invalid = 'Citizens' in empire commerce restrictions data.
void DaVessel::FindTradeBds(DaAtomList& tradeBds, COMMODITYID tradeCID) const
{
	// Find buildings that can trade with this ship.
	DaAtomList reachableBds;
	FindReachableBds(reachableBds);
	if(reachableBds.empty())
		return;
	FindTradeBds(tradeBds, tradeCID, reachableBds);
}

// CID_Invalid = 'Citizens' in empire commerce restrictions data.
void DaVessel::FindTradeBds(DaAtomList& tradeBds, COMMODITYID tradeCID, const DaAtomList& reachableBds) const
{
	// Find buildings that will trade with this ship.
	quint32 empireId = EmpireId();
	for(DaAtomList::const_iterator bi = reachableBds.cbegin(); bi != reachableBds.cend(); bi++)
	{
		const DaBd* bd = dynamic_cast<const DaBd*>(*bi);
		if(!bd)
			continue;
		switch(bd->IndustryId())
		{
		case IID_AirportTerminal:
			// Airport terminals provide trade and spacecraft services to all spacecraft.
			break;
		case IID_BaseAirTerminal:
			// Base air terminals provide all trade and spacecraft services to fleet spacecraft.
			if(IsPrivatelyOwned())
				continue;
			break;
		case IID_Broker:
			// CID_Sunlight = 'Repair' in empire commerce restrictions data. This is used when looking for an exterminator.
			if(tradeCID == CID_Sunlight)
				continue;		// Brokers do not have exterminators. They trade everything else.
			break;
		default:
			continue;
		}

		// The building can trade. Is the trade commodity restricted for export to this ship?
		const DgData& gd = g_Au.LockDataForRead();
		POLITICAL_STANCE stanceToShip = gd.EmpirePoliticalStance(bd->EmpireId(), empireId);
		bool isRestricted = gd.EmpireCommerceRestricted(bd->EmpireId(), tradeCID, stanceToShip);
		g_Au.UnlockData();
		if(!isRestricted)
			tradeBds.append((DaBd*)bd);	// Cast away constness. Building is ok with exporting the commodity to this ship.
	}
}

void DaVessel::FindTroopBds(DaAtomList& troopBds)
{
	// Find buildings that can trade with this ship.
	DaAtomList bds;
	FindReachableBds(bds);
	if(bds.empty())
		return;

	// Find buildings that will trade with this ship.
	quint32 empireId = EmpireId();
	for(DaAtomList::const_iterator bi = bds.begin(); bi != bds.end(); bi++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*bi);
		if(!bd)
			continue;
		if(!bd->Residents())
			continue;
		switch(bd->IndustryId())
		{
		case IID_BaseMarineBarracks:
		case IID_PoliceAcademy:
			if(bd->EmpireId() != empireId)
				continue;								// Troops will only serve aboard a ship of their own empire.
			break;
		default:
			continue;
		}
		troopBds.append(bd); 							// Building is ok with providing troops to the ship.
	}
}

void DaVessel::FindCrewBds(DaAtomList& crewBds)
{
	// Find buildings that can trade with this ship.
	DaAtomList bds;
	FindReachableBds(bds);
	if(bds.empty())
		return;

	// Find buildings that will trade with this ship.
	quint32 empireId = EmpireId();
	for(DaAtomList::const_iterator bi = bds.begin(); bi != bds.end(); bi++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*bi);
		if(!bd)
			continue;
		if(!bd->Residents())
			continue;
		switch(bd->IndustryId())
		{
		case IID_School:
			{
				// CID_Invalid = 'Citizens' in empire commerce restrictions data.
				const DgData& gd = g_Au.LockDataForRead();
				POLITICAL_STANCE stanceToShip = gd.EmpirePoliticalStance(bd->EmpireId(), empireId);
				bool isRestricted = gd.EmpireCommerceRestricted(bd->EmpireId(), CID_Invalid, stanceToShip);
				g_Au.UnlockData();
				if(isRestricted)
					continue;							// City is restricted from exporting citizens to my empire.
			}
			break;
		case IID_BaseAcademy:
			if(bd->EmpireId() != empireId)
				continue;								// Fleet crew will only serve aboard a ship of their own empire.
			break;
		default:
			continue;
		}
		crewBds.append(bd); 							// Building is ok with providing crew to the ship.
	}
}

void DaVessel::FindPassengerLoadBds(DaAtomList& passBds)
{
	// Find buildings that can trade with this ship.
	DaAtomList bds;
	FindReachableBds(bds);
	if(bds.empty())
		return;

	// Find buildings that will trade with this ship.
	quint32 empireId = EmpireId();
	for(DaAtomList::const_iterator bi = bds.begin(); bi != bds.end(); bi++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*bi);
		if(!bd)
			continue;
		if(!bd->Residents())
			continue;
		switch(bd->IndustryId())
		{
		case IID_School:
		case IID_PoliceAcademy:
		case IID_BaseAcademy:
		case IID_BaseMarineBarracks:
			continue;	// These buildings do not provide passengers.
		default:
			{
				// CID_Invalid = 'Citizens' in empire commerce restrictions data.
				const DgData& gd = g_Au.LockDataForRead();
				POLITICAL_STANCE stanceToShip = gd.EmpirePoliticalStance(bd->EmpireId(), empireId);
				bool isRestricted = gd.EmpireCommerceRestricted(bd->EmpireId(), CID_Invalid, stanceToShip);
				g_Au.UnlockData();
				if(isRestricted)
					continue;								// Building is restricted from exporting citizens to my empire.
			}
			break;
		}
		passBds.append(bd);	 								// Building is ok with providing passengers to the ship.
	}
}

void DaVessel::FindPassengerUnloadBds(DaAtomList& passBds)
{
	// Find buildings that can trade with this ship.
	DaAtomList bds;
	FindReachableBds(bds);
	if(bds.empty())
		return;

	// Find buildings that will trade with this ship.
	quint32 empireId = EmpireId();
	for(DaAtomList::const_iterator bi = bds.begin(); bi != bds.end(); bi++)
	{
		DaBd* bd = dynamic_cast<DaBd*>(*bi);
		if(!bd)
			continue;
		switch(bd->IndustryId())
		{
		case IID_School:
		case IID_PoliceAcademy:
		case IID_BaseAcademy:
		case IID_BaseMarineBarracks:
			continue;	// These buildings do not take passengers.
		default:
			if(PoliticalStance(bd->EmpireId(), empireId) == POL_Enemy)
				continue;									// AI will not unload to an enemy city.
			break;
		}
		passBds.append(bd); 								// Building is ok with taking passengers from the ship.
	}
}

// Sets the posture of all officers, crew and troops who are aboard.
void DaVessel::SetCrewPosture(CITIZEN_POSTURE posture)
{
	DaAtomList allCrew;
	FindAIPeopleRecursive(Id(), allCrew);
	FindOfficersRecursive(Id(), allCrew);
	FindCrewRecursive(Id(), allCrew);
	FindTroopsRecursive(Id(), allCrew);
	for(DaAtomList::const_iterator ai = allCrew.begin(); ai != allCrew.end(); ai++)
	{
		DaSentient* crewman = (DaSentient*)(*ai);
		if(crewman && crewman->Posture() != posture)
		{
			crewman->SetPosture(posture);
			crewman->SetModified();
		}
	}
}

bool DaVessel::InChainOfCommand(const DaAtom* atom) const
{
	if(InChainOfCommand(atom->Id(), atom->EmpireId()))
		return true;
	return atom->HasAgentPower();	// Admins are in the chain-of-command of all ships.
}

bool DaVessel::InChainOfCommand(const AuId& personId, quint32 personEmpire) const
{
	// Ships in the arena are in the chain of command of everyone on their team.
	if(InArena())
		return EmpireId() == personEmpire;
	if(InDesignEnvironment())
		return true;
	if(personEmpire == 0)
		return false;											// Free avatars cannot command spacecraft.

	// Officers are always in the ship's chain-of-command.
	if(IsOfficer(personId))
		return true;											// The person is an officer of the ship, possibly the captain.

	// A ship with no berths is in everyone's chain-of-command, if nobody is occupying the one-and-only captain's slot.
	if(CaptainsBerthType() == AuRosterEntry::CaptainSlot && !CaptainsBerthOccupied())
		return true;											// Captains Slot not occupied so ship is in everyone's chain.

	// Find any officer assigned to this ship, even if they don't have the con.
	DaSentient* officer = FindOfficer();
	if(IsPrivatelyOwned())
	{
		// Privately owned ship and the person is not an officer of the ship.
		if(OwnerId() == personId)
			return true;										// The person is the owner of the ship.

		// Person is not an officer or owner of this privately owned ship.
		if(HasHumanOfficer())
			return false;										// Captain or officer berth is occupied by a human.

		// No human officers; this privately owned ship will answer to anyone if there is no AI officer.
		if(!officer)
			return true;										// No AI officer.
	}

	// If the ship has an AI-controlled officer aboard, the person might be in the officer's chain of command.
	// Privately owned ships only get this far when they have an officer.
	if(officer)
	{
		// Ship has an AI-controlled officer aboard who is assigned to the ship.
		// The origin world, system and sector of the officer determines the ship's chain-of-command.
		if(officer->InChainOfCommand(personId, personEmpire))
			return true;										// The officer is in the chain-of-command of the person.
	}
	else
	{
		// Ship is subsidized and it has no AI officers; the person is not currently an officer of the ship.
		// The person must be in the same empire as the ship.
		if(personEmpire != EmpireId())
			return false;										// Wrong empire.

		const DgData& gd = g_Au.LockDataForRead();
		if(gd.IsEmpireGovernor(personEmpire, personId))
		{
			// Get list of current places starting with my planet up to the universe.
			AuIdList placeChain;
			ChainOfPlaces(placeChain);
			if(gd.IsEmpirePlaceInDomain(personEmpire, placeChain, personId))
			{
				g_Au.UnlockData();
				return true;
			}
		}
		g_Au.UnlockData();
	}

	// The person is not in the ship's chain-of-command if we made it this far.
	return false;
}

quint8 DaVessel::HighestSystemQuality() const
{
	// Scene client does rx enough m_ssaShipSystems data to do this.
	m_SystemsLock.lockForRead();
	quint8 highestQ = m_ssaShipSystems.HighestQuality();
	m_SystemsLock.unlock();

	if(m_QMade > highestQ)
		highestQ = m_QMade;

	return highestQ;
}

quint8 DaVessel::LowestSystemQuality() const
{
	// Scene client does rx enough m_ssaShipSystems data to do this.
	m_SystemsLock.lockForRead();
	quint8 lowestQ = m_ssaShipSystems.LowestQuality();
	m_SystemsLock.unlock();

	if(m_QMade < lowestQ)
		lowestQ = m_QMade;

	return lowestQ;
}

// Removes crew from the roster that cannot be found aboard the ship.
void DaVessel::ReconcileCrew()
{
	if(HasHumanOfficer())
		return;

	// Remove all crew from the roster who are not aboard the ship.
	bool anyRemoved = false;
	m_RosterLock.lockForWrite();
	AuRoster::iterator ri = m_Roster.begin();
	while(ri != m_Roster.end())
	{
		if((*ri).IsCrew() && !FindIdRecursive((*ri).m_OccId))
		{
			// Passengers not found in the ship.
			if(g_Au.m_LogMessages & LOG_Debug)
				qDebug("DaVessel::ReconcileCrew %d %s kicked %d %s from the roster.",
						Id().GetID(), (const char*)Name().toLatin1(), (*ri).m_OccId.GetID(),
						(const char*)(*ri).m_OccName.toLatin1());
			ri = m_Roster.erase(ri);
			anyRemoved = true;
		}
		else
			ri++;	// Occupant was found aboard the ship or the berth type is not a passenger.
	}

	// Notify changes.
	if(anyRemoved)
	{
		m_tmRosterChange.Set();
		SetModified();
	}
	m_RosterLock.unlock();
}

// Removes passengers from the roster that cannot be found aboard the ship.
void DaVessel::ReconcilePassengers()
{
	if(HasHumanOfficer())
		return;

	// Remove all passengers from the roster who are not aboard the ship.
	bool anyRemoved = false;
	m_RosterLock.lockForWrite();
	AuRoster::iterator ri = m_Roster.begin();
	while(ri != m_Roster.end())
	{
		if((*ri).IsPassenger() && !FindIdRecursive((*ri).m_OccId))
		{
			// Passengers not found in the ship.
			if(g_Au.m_LogMessages & LOG_Debug)
				qDebug("DaVessel::ReconcilePassengers %d %s kicked %d %s from the roster.",
						Id().GetID(), (const char*)Name().toLatin1(), (*ri).m_OccId.GetID(),
						(const char*)(*ri).m_OccName.toLatin1());
			ri = m_Roster.erase(ri);
			anyRemoved = true;
		}
		else
			ri++;	// Occupant was found aboard the ship or the berth type is not a passenger.
	}

	// Notify changes.
	if(anyRemoved)
	{
		m_tmRosterChange.Set();
		SetModified();
	}
	m_RosterLock.unlock();
}

// Removes troops from the roster that cannot be found aboard the ship.
void DaVessel::ReconcileTroops()
{
	if(HasHumanOfficer())
		return;

	// Remove all passengers from the roster who are not aboard the ship.
	bool anyRemoved = false;
	m_RosterLock.lockForWrite();
	AuRoster::iterator ri = m_Roster.begin();
	while(ri != m_Roster.end())
	{
		if((*ri).m_Type == AuRosterEntry::Troop && !FindIdRecursive((*ri).m_OccId))
		{
			// Passengers not found in the ship.
			if(g_Au.m_LogMessages & LOG_Debug)
				qDebug("DaVessel::ReconcileTroop %d %s kicked %d %s from the roster.",
						Id().GetID(), (const char*)Name().toLatin1(), (*ri).m_OccId.GetID(),
						(const char*)(*ri).m_OccName.toLatin1());
			ri = m_Roster.erase(ri);
			anyRemoved = true;
		}
		else
			ri++;	// Occupant was found aboard the ship or the berth type is not a passenger.
	}

	// Notify changes.
	if(anyRemoved)
	{
		m_tmRosterChange.Set();
		SetModified();
	}
	m_RosterLock.unlock();
}

// Removes occupants from the roster that cannot be found aboard the ship.
// This is similar to the code that reconciles the roster at the player/officer's request but it is not the same.
// That code is in DtSceneServer::ReconcileRoster.
void DaVessel::ReconcileRoster()
{
	if(HasHumanOfficer())
		return;

	// Remove all occupants from the roster who are not aboard the ship.
	bool anyRemoved = false;
	m_RosterLock.lockForWrite();
	AuRoster::iterator ri = m_Roster.begin();
	while(ri != m_Roster.end())
	{
		if(!FindIdRecursive((*ri).m_OccId))
		{
			// Occupant not found in the ship.
			if(g_Au.m_LogMessages & LOG_Debug)
				qDebug("DaVessel::ReconcileRoster %d %s kicked %d %s from the roster.", Id().GetID(),
						(const char*)Name().toLatin1(), (*ri).m_OccId.GetID(), (const char*)(*ri).m_OccName.toLatin1());
			ri = m_Roster.erase(ri);
			anyRemoved = true;
		}
		else
			ri++;	// Occupant was found aboard the ship.
	}

	// Notify changes.
	if(anyRemoved)
	{
		m_tmRosterChange.Set();
		SetModified();
	}
	m_RosterLock.unlock();
}

bool DaVessel::SpawnTroop(DaBd* bd)
{
	AuEther* ether = Ether();
	if(!ether)
		return false;
	Q_ASSERT(TroopBerthsAvailable());	// Caller needs to insure there is a troop berth available.
	AuPoint spawnAt;
	if(!IsDesignDataCacheValid() || !TroopOrCrewVoidLocation(spawnAt))
		return false;
	const DaWorldLL* llworld = bd->FindParentWorldInEnv();
	if(!llworld)
		return false;

	// Create a citizen atom using the worker DNA at the city location. We can't use the DNA from this terrain's region because
	// it might be invalid. NOTE: The city's location is its longitude and latitude, as returned by DaWorld::GetRealLongLat.
	AuDNABase::CHROMATID chroma = (AuDNABase::CHROMATID)g_Au.Dice().Roll(1,2,-1);
	HANDEDNESS hand = (HANDEDNESS)g_Au.Dice().Roll(1,2,-1);
	int resourceZone = llworld->ResourceZone(bd->GlobeLL().Longitude());
	quint8 bodyQ = llworld->NaturalResourceQuality(CID_Animal, resourceZone);
	DaNpc* troop = new DaNpc(ROLE_Troop, bd->ResidentSpecies(), chroma, hand, g_Au.Dice().RandomPersonName());
	troop->ConnectSignals();
	troop->Create();

	// Occupy a troop berth.
	troop->OccupyBerth(this);
	AuPoint berthLoc = BerthLocation(troop->Id());

	// Position the atom in the environment.
	//berthLoc.z += troop->GroundHeightAboveCG();
	berthLoc.z += .1;
	AuMatrix4x4 t;
	t.Translate(berthLoc);	// Move to location in spacecraft.

	// Rotate the atom randomly on its z axis.
	double rot = g_Au.Dice().Roll(1,360,-1) * DEG2RAD;
	t.RotateZ(rot);
	troop->SetPosAndOrient(AuAtomPos(Id(), t));
	ether->AddAtom(troop);

	troop->SetHomeWorld(llworld);
	troop->SetBodyQ(bodyQ);
	troop->ResetHitPoints();
	troop->ResetHunger();
	troop->ResetStamina();
	troop->SetEmpireId(bd->EmpireId());
	troop->Stand();
	troop->InitTraining();

	AuItemHash items, itemsTaken;
	bd->CommunityInventoryAvailable(items, EmpireId());
	troop->InitGear(items, itemsTaken, true);
	bd->CommunityRemoveInventory(itemsTaken);
	troop->GoToWork();

	// Send them on their way with any souvenirs the city has to offer.
	//city->LockSituationsForRead().InflictCitizen(troop);
	//city->UnlockSituations();

	g_Au.AtomAdded(troop->Id());
	troop->SetNormal();	// New atom is ready for normal thread processing.
	troop->TickTock();
	return true;
}

// Returns 'true' if the turret is fired.
bool DaVessel::FireTurret(DaSentient& gunner, bool avoidFriends, const AuId& target)
{
	if(InArenaCannotFireWeapons())
		return false;
	AuEther* ether = Ether();
	if(!ether)
		return false;

	// Determine if the turret is operational.
	quint16 which = gunner.StationId();
	if(!IsConsoleOperational(AuSysId(SYS_Turret, which)) ||
			!GetPowerConsumption(SYS_Turret, which))
		return false;	// The turret is not operational.

	// Go all the way to the parent solar system.
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return false;	// This isn't too likely to happen.

	// Determine ammunition loaded into the turret.
	unordered_map<quint16, AuOrdnance>::iterator ai = LockTurretOrdnance().find(which);
	if(ai == m_Turrets.end())
	{
		UnlockTurretOrdnance();
		return false;
	}
	if(!(*ai).second.CanFire())
	{
		// Energy weapon automatically reloads using one unit of energy.
		if(!(*ai).second.RoundsCount() && CapacitorLevel() > 1)
		{
			(*ai).second.Reload(DbCommodity::MaxTurretRounds((*ai).second.CommodityId()));
			DischargeCapacitor(1);
			if(!(*ai).second.CanFire())
			{
				UnlockTurretOrdnance();
				return false;	// For some reason we still can't fire.
			}
		}
		else
		{
			UnlockTurretOrdnance();
			return false;
		}
	}

	// Determine weapon location and firing direction.
	PositionTurret(gunner);
	AuPoint emitLoc = gunner.WeaponMuzzleLoc();
	AuPoint emitDir = gunner.WeaponMuzzleVec();
	AuPoint muzzleLoc = emitLoc + emitDir*3;	// move farther out for sighting target
	if((*ai).second.IsEnergyWeapon())
	{
		// Determine appropriate weapon effects.
		AuCombatWeapon weapon((*ai).second.Ammo(), 1);
		weapon.m_CID = SystemModule(SYS_Turret, which);
		SOUND_EFFECT sound = (SOUND_EFFECT)(*ai).second.SoundFired();
		VISUAL_EFFECT visual = (*ai).second.VisualFired();

		// Select target(s) in the firing direction of the vehicle.
		// Initialize sight dist for target search at max range.
		AuPoint sightedLoc = emitLoc + emitDir * weapon.m_Range;

		// Get reverse transformation back up to the parent solar system
		AuMatrix4x4 tModel = gunner.EnvTransform().Inverse();

		// Ask the solar system to find the nearest solid atom that is in front of the weapon.
		unsigned int sightedPart = 0;
		bool sightedShield = false;
		DaAtom* sightedAtom = system->SightTarget(&gunner, muzzleLoc, tModel, 0, sightedLoc, sightedPart, sightedShield, 0);
		if(sightedAtom)
		{
			// Target may be protected from attack.
			if(sightedAtom->IsNoncombatant())
			{
				UnlockTurretOrdnance();
				return false;
			}

			// Sighted an atom in our line of fire.
			if(avoidFriends && !sightedAtom->Is(target))
			{
				// Don't shoot if the sighted atom is a friendly unit, unless it is specifically our target.
				DaUnit* sightedUnit = dynamic_cast<DaUnit*>(sightedAtom);
				if(sightedUnit)
				{
					switch(PoliticalStanceMakeContact(sightedUnit))
					{
					case POL_Friend:
					case POL_Empire:
					case POL_Vassal:
						UnlockTurretOrdnance();
						return false;
					default:
						break;
					}
				}
				else
				{
					// Don't shoot items at all.
					DaItem* item = dynamic_cast<DaItem*>(sightedAtom);
					if(item)
					{
						UnlockTurretOrdnance();
						return false;
					}
				}
			}

			// Target found.
			// Add weapon fire effect to weapon.
			if(sound != SOUND_None || visual != VISUAL_None)
			{
				AuEffect effect(emitLoc, AuPoint(0,0), AuPoint(0,0), sound, 1, 1, visual);
				effect.SetTargetLoc(sightedLoc);
				effect.SetEnergyBeamEffect(weapon.m_w1.m_Effect);
				gunner.AddEffect(effect);
			}

			double targDist = emitLoc.Distance(sightedLoc);

			AuCombatDefender defender(sightedAtom, sightedLoc, sightedShield, targDist);

			AuCombatResult result;

			// Apply damage to sighted atom.
			if(IsCrewOrTroop(gunner.Id()))
			{
				AuCombatAttacker attacker(Id(), NameWithEmpire(), EmpireId(), IsPirate());
				attacker.m_MuzzleLoc = muzzleLoc;

				gunner.AttackTarget(attacker, defender, weapon, true, result);

				// Send notification on the crew channel when a unit is destroyed.
				if(result.m_IsDestroyed && dynamic_cast<DaUnit*>(sightedAtom) && !IsPirateWithFreeWill())
				{
					QString targetName;
					if(sightedAtom->EmpireId())
					{
						QString empire;
						const DgData& gd = g_Au.LockDataForRead();
						gd.EmpireName(sightedAtom->EmpireId(), empire);
						g_Au.UnlockData();
						targetName = QString("%1 %2").arg(empire).arg(sightedAtom->Name());
					}
					else
						targetName = sightedAtom->Name();
					AuCommDamageInflicted msg(targetName, result.m_CriticalHits, result.m_IsDestroyed,
							weapon.m_w1.m_Effect, result.m_Damage1, result.Armor1(),
							weapon.m_w2.m_Effect, result.m_Damage2, result.Armor2(),
							AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::CrewMates);
					msg.SetScopeData(Id().GetID());					// Required for CrewMates message to be valid.
					g_Au.SendAtomChat(msg);
				}
			}
			else
			{
				AuCombatAttacker attacker(gunner.Id(), tr("%1 aboard %2").arg(gunner.Name()).arg(NameWithEmpire()),
						gunner.EmpireId(), gunner.IsPirate());
				attacker.m_MuzzleLoc = muzzleLoc;

				gunner.AttackTarget(attacker, defender, weapon, true, result);
			}

			if(result.m_IsDestroyed &&
					(MissionOrderType() == AuMissionOrder::Attack || MissionOrderType() == AuMissionOrder::Plunder) &&
					sightedAtom->Is(MissionOrderTarget()) &&
					IsCommandedByAI())
				MissionOrderComplete(FindCommandingOfficer(), 1);	// We have destroyed the target of our attack order.
		}
		else
		{
			// Add weapon fire effect to weapon.
			if(sound != SOUND_None || visual != VISUAL_None)
			{
				AuEffect effect(emitLoc, AuPoint(0,0), AuPoint(0,0), sound, 1, 1, visual);
				effect.SetTargetLoc(sightedLoc);
				effect.SetEnergyBeamEffect(weapon.m_w1.m_Effect);
				gunner.AddEffect(effect);
			}
		}

		// Gun fired.
		// Expend ammo and update fire time and sequencer.
		(*ai).second.Fired();

		// Energy weapon automatically reloads using one unit of energy.
		if(!(*ai).second.RoundsCount() && CapacitorLevel() > 1)
		{
			(*ai).second.Reload(DbCommodity::MaxTurretRounds((*ai).second.CommodityId()));
			DischargeCapacitor(1);
		}
	}
	else
	{
		// Turret fires a bullet, rocket, or missile if it's not an energy weapon.
		AuId gunnerWorldId;
		int gunnerDomain = GunnerDomain(gunnerWorldId);

		AuPoint bestSysLoc;
		double bestDistance = AttackRange();
		const DaAtom* target;
		if(DbCommodity::IsMissile((*ai).second.CommodityId()))
			target = system->FindClosestMissileTarget(Id(), gunnerDomain, gunnerWorldId, EmpireId(),
					EnvTransform().Project(muzzleLoc), 0, bestSysLoc, bestDistance);
		else
			target = 0;

		// Shoot ordnance at target.
		DaMissile* bullet;
		if(target)
			bullet = new DaMissile(*(*ai).second.Ammo(), 1, EmpireId(), target->Id(), target->EnvTransform().UnProject(bestSysLoc));
		else
			bullet = new DaMissile(*(*ai).second.Ammo(), 1, EmpireId());
		bullet->ConnectSignals();
		bullet->Create();

		// Create a transform to position the bullet at the gun muzzle location, pointing in the gun fire direction.
		AuMatrix4x4 aim = AuPoint(0,0,1).GetAlignmentTransform(emitDir);
		AuMatrix4x4 t(Transform());
		t *= gunner.Transform();
		t.Translate(muzzleLoc);
		t *= aim;
		bullet->SetPosAndOrient(AuAtomPos(ParentId(), t));
		ether->AddAtom(bullet);

		bullet->SetInScene();							// Don't decay too fast right away.
		bullet->SetVector(aim.UnProject(Vector()) + AuPoint(0,0,bullet->MuzzleExitVelocity()));	// Muzzle velocity plus my vec.
		bullet->SetInMotion();
		bullet->ArmWeapon(Id(), tr("%1 aboard %2").arg(gunner.Name()).arg(NameWithEmpire()), EmpireId());
		quint16 roomId;
		if(!InsideRoom(muzzleLoc, roomId))
			bullet->SetOwnerIgnored(true);	// Do not hit my owner/armedBy atom when fired outside a room.

		g_Au.AtomAdded(bullet->Id());
		bullet->SetNormal();	// New atom is ready for normal thread processing.
		bullet->TickTock();		// Immediately start tick processing.

		// Add weapon fire effect to weapon.
		SOUND_EFFECT sound = (SOUND_EFFECT)(*ai).second.SoundFired();
		VISUAL_EFFECT visual = (*ai).second.VisualFired();
		if(sound != SOUND_None || visual != VISUAL_None)
		{
			AuPoint effRot;
			if(visual == VISUAL_FireCannon)
				effRot = AuPoint(0,1,0).GetAlignmentTransform(emitDir).GetRotation();
			AuEffect effect(emitLoc, effRot, AuPoint(0,0), sound, 1, 1, visual);
			effect.SetTargetLoc(AuPoint(0, 1000, 0));
			gunner.AddEffect(effect);
		}

		// Gun fired.
		// Expend ammo and update fire time and sequencer.
		(*ai).second.Fired();
	}
	UnlockTurretOrdnance();
	SetModified();
	return true;
}

// 'unitsFired' is the number of weapon bay units to fire.
// 'harvestIndex' is an index into an array of commodities for harvesters
// Returns 'true' if the weapon is fired.
bool DaVessel::FireBayWeapon(DaSentient& gunner, quint16 whichBay, float unitsFired, quint8 harvestIndex,
		const DaAtom* targetAtom, const AuPoint& targetSysLoc)
{
	if(InArenaCannotFireWeapons())
		return false;

	// Insure the weapon bay is ready to fire again. It may still be waiting to fire since it takes a couple of seconds to rearm.
	if(!WeaponBayCanFire(whichBay) || !GetPowerConsumption(SYS_Weapon, whichBay) || IsInsideSpacecraft())
	{
		// A tractor beam bay should shut off now.
		COMMODITYID bayModule = SystemModule(SYS_Weapon, whichBay);
		if(bayModule == CID_TractorBeamBayModule)
			CancelWeaponBayFired(whichBay);
		return false;
	}

	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return false;	// No solar system? Not likely.

	if(!targetAtom)
		return false;	// Could not find the target atom in this system.
	if(Is(targetAtom->ParentId()))
		return false;	// Cannot target on atoms aboard this firing ship.

	float ammoConsumed = 0;	// Set to the number of units of ammo consumed from the hold.
	float powerConsumed = 0;		// Set to the number of units of energy consumed from the capacitor.
	COMMODITYID bayModule = SystemModule(SYS_Weapon, whichBay);
	const AuItem* ammoSel = 0;
	const AuItem* ammoInBay = LockBayWeaponAmmunition(whichBay);
	bool weaponFired = false;
	switch(bayModule)
	{
	case CID_BallisticWeaponBayModule:
	case CID_WeaponDeploymentBayModule:
	case CID_WeaponLauncherBayModule:
		// Cannot fire at all targets when I am a noncombatant.
		if(IsNoncombatant())
		{
			// As a noncombatant, I can only shoot at targets at my world.
			// Check for common parent world.
			if(FindParentWorldInEnv() != targetAtom->FindParentWorldInEnv())
			{
				// Target is not at this world. Target could be at a moon of my world.
				UnlockBayWeaponAmmunition();
				return false;
			}
		}

		// Check for appropriate ammo and consume it.
		// The ammo's count is set to the number of them available in the hold.
		ammoSel = ammoInBay;
		if(!ammoSel || !ammoSel->Count())
		{
			UnlockBayWeaponAmmunition();
			return false;	// Cannot fire unless ammo is selected.
		}

		// Calculate power consumption of weapon.
		ammoConsumed = unitsFired;
		if(ammoConsumed > ammoSel->Count())
		{
			ammoConsumed = ammoSel->Count();
			unitsFired = ammoSel->Count();
		}

		weaponFired = true;
		break;

	case CID_EnergyWeaponBayModule:
		// Cannot fire at all targets when I am a noncombatant.
		if(IsNoncombatant())
		{
			// As a noncombatant, I can only shoot at targets at my world.
			// Check for common parent world.
			if(FindParentWorldInEnv() != targetAtom->FindParentWorldInEnv())
			{
				// Target is not at this world. Target could be at a moon of my world.
				UnlockBayWeaponAmmunition();
				return false;
			}
		}

		// Check for appropriate ammo and consume it.
		// The ammo's count is set to the number of them available in the hold.
		ammoSel = ammoInBay;
		if(!ammoSel || !ammoSel->Count())
		{
			UnlockBayWeaponAmmunition();
			return false;	// Cannot fire unless ammo is selected.
		}

		// One weapon bay unit in a spacecraft fires 100 weapon power units, consuming one unit from the capacitor.
		powerConsumed = unitsFired;
		if(powerConsumed > ammoSel->Count())
		{
			powerConsumed = ammoSel->Count();
			unitsFired = ammoSel->Count();
		}
		if(powerConsumed >= CapacitorLevel())
		{
			UnlockBayWeaponAmmunition();
			return false;				// Don't use the last of the capacitor.
		}

		weaponFired = true;
		break;

	case CID_TractorBeamBayModule:
		// Cannot fire at all targets when I am a noncombatant.
		if(IsNoncombatant())
		{
			POLITICAL_STANCE stance = PoliticalStance(EmpireId(), targetAtom->EmpireId());
			if(stance == POL_Enemy || stance == POL_Neutral)
			{
				// Noncombatant cannot tractor beam an enemy or neutral unit.
				// As a noncombatant, I can only shoot at targets at my world.
				// Check for common parent world.
				if(FindParentWorldInEnv() != targetAtom->FindParentWorldInEnv())
				{
					// Target is not at this world. Target could be at a moon of my world.
					UnlockBayWeaponAmmunition();
					return false;
				}
			}
		}
		[[fallthrough]];

	case CID_AtmosphereCondenserBayModule:
	case CID_ParticleCollectorBayModule:
	case CID_ProspectorBayModule:
	case CID_SiphonBayModule:
	case CID_WildcatterBayModule:
		// Calculate power consumption of weapon.
		// One weapon bay unit in a spacecraft fires 100 weapon power units, consuming one unit from the capacitor.
		powerConsumed = unitsFired;		// power consumption is 1 per unit fired
		if(powerConsumed >= CapacitorLevel())
		{
			UnlockBayWeaponAmmunition();
			return false;				// Don't use the last of the capacitor.
		}

		weaponFired = false;
		break;

	default:
		UnlockBayWeaponAmmunition();
		return false;
	}

	// Stations and AI commanders broadcast a message when firing bay weapons at enemy units.
	if(weaponFired && !InArena() &&
			!InDesignEnvironment() &&
			(!LastAlert().isValid() || LastAlert().ElapsedSec() > 60) &&	// A minutes has passed since last report.
			(IsSpaceStation() || IsCommandedByAI()) &&				// Station or AI commander reports.
			!IsPirateWithFreeWill())								// Pirates have nobody to report to.
	{
		if(IsSpaceStation())
		{
			const DaWorld* planet = FindParentPlanetInEnv();
			if(planet)
			{
				// Station is orbiting a planet.
				// Send a message on the friend channel.
				const DaSector* sector = planet->FindParentSector();
				AuCommFiringAtTarget msg(0, targetAtom->Name(), planet->Name(), sector->Name() + " " + sector->Description(),
						AuCommStdChannel::FriendChannelName(), Id(), Name(), sector->Id(), Galaxy(),
						AuCommStdChannel::Sector, AuCommStdChannel::Friends);
				msg.SetScopeData(EmpireId());	// Friend channel needs my empire id to know the scope of the message.
				g_Au.SendAtomChat(msg);
			}
			else
			{
				// Station is not orbiting a planet.
				// Send a message on the friend channel.
				const DaSector* sector = system->FindParentSector();
				AuCommFiringAtTarget msg(1, targetAtom->Name(), system->Name(), sector->Name() + " " + sector->Description(),
						AuCommStdChannel::FriendChannelName(), Id(), Name(), sector->Id(), Galaxy(),
						AuCommStdChannel::Sector, AuCommStdChannel::Friends);
				msg.SetScopeData(EmpireId());	// Friend channel needs my empire id to know the scope of the message.
				g_Au.SendAtomChat(msg);
			}
		}
		else
		{
			// AI commanded spacecraft is firing bay weapon.
			// Send a message on the friend channel.
			const DaSector* sector = system->FindParentSector();
			AuCommFiringAtTarget msg(2, targetAtom->Name(), system->Name(), sector->Name() + " " + sector->Description(),
					AuCommStdChannel::FriendChannelName(), Id(), Name(), sector->Id(), Galaxy(),
					AuCommStdChannel::Sector, AuCommStdChannel::Friends);
			msg.SetScopeData(EmpireId());	// Friend channel needs my empire id to know the scope of the message.
			g_Au.SendAtomChat(msg);
		}
		m_CommLock.lockForWrite();
		m_tmAlert.restart();
		m_CommLock.unlock();
	}

	// Weapon has fired; power is consumed now.
	// Weapon actually fires in a couple of seconds, during DaVessel::CycleBayWeapons.
	AuPoint emitLoc = WeaponBayEmitLoc(whichBay, gunner.Location());

	// Get the lock location on the target based on ring geometry and harvesting vs attacking.
	//AuPoint fLockLoc = targetAtom->WeaponLockLocation(EnvTransform().Project(emitLoc), bayModule);
	AuPoint fLockLoc = EnvTransform().UnProject(targetSysLoc);					// into this spacecraft space
	AuPoint locOnTarg = targetAtom->EnvTransform().UnProject(targetSysLoc);		// into target space

	// Weapon has fired; power and/or ammo is consumed now.
	if(powerConsumed > 0)
		DischargeCapacitor(powerConsumed);						// Consume capacitor power.

	// Weapon actually fires in a couple of seconds, during CycleBayWeapons.
	WeaponBayFired(gunner.Id(), whichBay, emitLoc, unitsFired, harvestIndex, targetAtom->Id(), fLockLoc, locOnTarg, ammoSel);
	if(ammoSel && ammoConsumed)
		RemoveCargo(*ammoSel, ammoConsumed);				// Consume ammo.

	UnlockBayWeaponAmmunition();

	SetBayWeaponTarget(whichBay, targetAtom->Id());	// If a human grabs the console from AI, they'll see the target selected.

	return true;
}

void DaVessel::WeaponBayFired(const AuId& firer, quint16 which, const AuPoint& emitLoc, float unitsFired, quint8 harvestIndex,
		const AuId& target, const AuPoint& targetLoc, const AuPoint& locOnTarget, const AuItem* ammo)
{
	//qDebug() << "DaVessel::WeaponBayFired" << Id() << Name() << "which" << which << "emitLoc" << emitLoc << "unitsFired"
		//<< unitsFired;
	// Create a default AuBayFired object in the hash table. Using the reference is faster than reindexing every time.
	//m_BayLock.lockForWrite();
	if(m_BayFired.find(which) != m_BayFired.cend())			// Bay is already firing. Call WeaponBayCanFire first.
		return;

	AuBayFired bay;
	if(IsCrewOrTroop(firer))
		bay.m_Credit = Id();								// Crew shares the credit by supplying ship id.
	else
		bay.m_Credit = firer;								// Firer gets all the credit.

	bay.m_EmitLoc = emitLoc;
	bay.m_HarvestIndex = harvestIndex;
	bay.m_LocOnTarget = locOnTarget;
	bay.m_Target = target;
	bay.m_TargetLoc = targetLoc;

	COMMODITYID bayModule = SystemModule(SYS_Weapon, which);
	if(bayModule == CID_TractorBeamBayModule)
		bay.m_TractorState = 1;

	bay.m_UnitsFired = unitsFired;
	bay.m_When.Set();
	if(ammo)
	{
		bay.m_Ammo = ammo->Clone();
		bay.m_Ammo->SetCount(unitsFired);
	}
	m_BayFired.insert(pair<quint16, AuBayFired>(which, bay));
	m_tmBayFired.Set();
	//m_BayLock.unlock();
	SetModified();
}

// Returns the number of colonists aboard this ship.
int DaVessel::ColonistsAboard() const
{
	int colonists = 0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaCitizen* citizen = dynamic_cast<const DaCitizen*>(*it);
		if(!citizen)
			continue;
		if(!citizen->IsNpcPassenger())
			continue;
		colonists++;
	}
	return colonists;
}

int DaVessel::TroopsAboard() const
{
	int troops = 0;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaCitizen* citizen = dynamic_cast<DaCitizen*>(*it);
		if(!citizen || !citizen->IsNpcTroop())
			continue;
		troops++;
	}
	return troops;
}

DaNpc* DaVessel::FindColonist() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaNpc* citizen = dynamic_cast<DaNpc*>(*it);
		if(!citizen)
			continue;
		if(!citizen->IsNpcPassenger())
			continue;
		return citizen;
	}
	return 0;
}

// Locate a troop that is assigned to this ship.
DaNpc* DaVessel::FindTroop() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaNpc* citizen = dynamic_cast<DaNpc*>(*it);
		if(!citizen || !citizen->IsNpcTroop())
			continue;
		if(!Is(citizen->AssignedShip()))
			continue;	// The citizen is not assigned to this ship.
		return citizen;
	}
	return 0;
}

DaNpc* DaVessel::BoardTroop(quint32 empireId, const QList<AuSpecies>& species, AuItemHash& inventory, quint8 bodyQ)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	if(!TroopBerthsAvailable())
		return 0;
	if(species.empty())
		return 0;

	// Create a citizen atom using the city's worker DNA.
	AuDNABase::CHROMATID chroma = (AuDNABase::CHROMATID)g_Au.Dice().Roll(1,2,-1);
	HANDEDNESS hand = (HANDEDNESS)g_Au.Dice().Roll(1,2,-1);
	int di = g_Au.Dice().Roll(1, species.size(), -1);
	DaNpc* troop = new DaNpc(ROLE_Troop, species[di], chroma, hand, g_Au.Dice().RandomPersonName());
	troop->ConnectSignals();
	troop->Create();

	// Occupy a troop berth with the troop.
	troop->OccupyBerth(this);
	AuPoint berthLoc = BerthLocation(troop->Id());

	// Position the atom in the environment.
	//berthLoc.z += troop->GroundHeightAboveCG();
	berthLoc.z += .1;
	AuMatrix4x4 t;
	t.Translate(berthLoc);	// Move to location in spacecraft.

	// Rotate the atom randomly on its z axis.
	double rot = g_Au.Dice().Roll(1,360,-1) * DEG2RAD;
	t.RotateZ(rot);
	troop->SetPosAndOrient(AuAtomPos(Id(), t));
	ether->AddAtom(troop);

	const DaWorldLL* world = FindParentWorldInEnv();
	if(world)
		troop->SetHomeWorld(world);
	troop->SetBodyQ(bodyQ);
	troop->ResetHitPoints();
	troop->ResetHunger();
	troop->ResetStamina();
	troop->SetEmpireId(empireId);
	troop->Stand();
	troop->InitTraining();

	AuItemHash itemsTaken;
	troop->InitGear(inventory, itemsTaken, true);
	troop->GoToWork();

	/*
	// Send them on their way with any souvenirs the city has to offer.
	fromCity->LockSituationsForRead().InflictCitizen(troop);
	fromCity->UnlockSituations();
	*/

	g_Au.AtomAdded(troop->Id());
	troop->SetNormal();	// New atom is ready for normal thread processing.
	troop->TickTock();

	IncreaseMonster(1, MONSTER_Troop);
	return troop;
}

DaPerson* DaVessel::FindOfficerOnLineRecursive()
{
	AuId captainId = CaptainId();
	DaPerson* officer = 0;
	bool captFoundOffLine = false;	// true if we find the captain aboard but he is off line
	m_RosterLock.lockForRead();
	for(AuRoster::const_iterator ri = m_Roster.cbegin(); ri != m_Roster.cend(); ri++)
	{
		if((*ri).IsHumanOfficer())
		{
			// Found a human officer listed in the ship's roster. Is the person aboard the ship?
			DaPerson* person = dynamic_cast<DaPerson*>(FindIdRecursive((*ri).m_OccId));
			if(person)
			{
				// Found a human officer aboard the ship.
				if(person->IsOnLine())
				{
					// The human officer is on line.
					if(person->Is(captainId))
					{
						m_RosterLock.unlock();
						return person;			// The on-line human officer is the captain; stop searching.
					}
					else if(captFoundOffLine)
					{
						m_RosterLock.unlock();
						return person;			// Found an on-line human officer and the captain is known to be off line.
					}
					officer = person;			// Remember on-line human officer who is not the captain.
				}
				else if(person->Is(captainId))
				{
					// Found the captain off line.
					if(officer)
					{
						m_RosterLock.unlock();
						return officer;			// Already found a human officer and now the captain is known to be off line.
					}
					captFoundOffLine = true;
				}
			}
		}
	}
	m_RosterLock.unlock();

	// Return any on-line officer that we found.
	return officer;
}

DaPerson* DaVessel::FindPersonOnLine(quint16 roomId) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaPerson* person = dynamic_cast<DaPerson*>(*it);
		if(!person)
			continue;
		if(!person->IsOnLine())
			continue;
		if(!person->IsInSpacecraft())
			continue;
		if(person->ShipRoomId() != roomId)
			continue;
		return person;
	}
	return 0;
}

DaAnimal* DaVessel::FindWildAnimal(quint16 roomId) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaAnimal* animal = dynamic_cast<DaAnimal*>(*it);
		if(!animal)
			continue;
		if(!animal->IsWild())
			continue;
		Q_ASSERT(animal->IsInSpacecraft());
		if(!animal->IsInSpacecraft())
			continue;
		if(animal->ShipRoomId() != roomId)
			continue;
		return animal;
	}
	return 0;
}

void DaVessel::TransferColonists(DaVessel* fromShip, int howMany)
{
	for(int icol=0; icol<howMany; icol++)
	{
		// Transfer an npc from the other ship to this ship.
		DaNpc* npc = fromShip->FindColonist();
		if(!npc)
			break;	// No more colonists aboard.

		// Release the passenger berth the npc occupied on the origin ship.
		fromShip->RemoveBerthOccupant(npc->Id());

		// Occupy a passenger berth with the npc.
		npc->OccupyBerth(this);
		AuPoint berthLoc = BerthLocation(npc->Id());

		// A location was found inside the hull to place the passenger.
		npc->Stand();
		berthLoc.z += .1;
		AuMatrix4x4 t;
		t.Translate(berthLoc);
		double zRot = g_Au.Dice().Roll(1,360,-1) * DEG2RAD;
		t.RotateZ(zRot);
		npc->ChangePosAndOrient(AuAtomPos(Id(), t));	// Move the citizen atom to this ship.
		npc->InvalidateEnvironment();
		npc->SetModified();
		npc->GoToWork();
	}
}

void DaVessel::TransferCryo(DaVessel* fromShip, int howMany)
{
	if(howMany > (int)CryoBerthsAvailable())
		howMany = CryoBerthsAvailable();
	for(int icol=0; icol<howMany; icol++)
	{
		// Transfer a cryo traveller from the other ship to this ship.
		AuSpecies species = fromShip->RemoveAnyCryoBerthOccupant();
		if(species.IsNull())
			break;	// No more cryo travellers aboard.
		BoardCryoTraveller(species);
	}
}

void DaVessel::TransferTroops(DaVessel* fromShip, int howMany)
{
	for(int icol=0; icol<howMany; icol++)
	{
		// Transfer a troop from the other ship to this ship.
		// Locate a troop aboard the from ship that is assigned to that spacecraft.
		DaNpc* troop = fromShip->FindTroop();
		if(!troop || !troop->IsNpcTroop())
			break;	// No more troops aboard.

		// Release the passenger berth the troop occupied on the origin ship.
		fromShip->RemoveBerthOccupant(troop->Id());

		// Occupy a troop berth with the troop.
		troop->OccupyBerth(this);
		AuPoint berthLoc = BerthLocation(troop->Id());

		// A location was found inside the hull to place the passenger.
		troop->Stand();
		//berthLoc.z += troop->GroundHeightAboveCG();
		berthLoc.z += .1;
		AuMatrix4x4 t;
		t.Translate(berthLoc);
		double zRot = g_Au.Dice().Roll(1,360,-1) * DEG2RAD;
		t.RotateZ(zRot);
		troop->ChangePosAndOrient(AuAtomPos(Id(), t));		// Move the troop atom to this ship.
		troop->InvalidateEnvironment();
		troop->SetModified();
		troop->GoToWork();
	}
}

void DaVessel::KilledByPerson(DaPerson* killer)
{
	// This spacecraft was killed by a person.
	const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(FindEnvironmentAtom());
	if(system)
		killer->SpacecraftDestroyed(this, system->Id());
}

void DaVessel::KilledByUnit(const DaUnit* killer)
{
	if(!killer->EmpireId())
		return;							// Killer is probably a wild animal.

	if(InArena())
		return;
	if(InDesignEnvironment())
		return;

	if(EmpireId() == killer->EmpireId())
		return;
	// Victim is from a different empire as the killer.

	// Victim's empire logs this as a diplomatic incident.
	const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(FindEnvironmentAtom());
	if(!system)
		return;	// No parent solar system?

	GALAXY galaxy = Galaxy();
	const DaSector* sector = system->FindParentSector();
	const DaWorld* world = FindParentPlanetInEnv();
	QString sectorName = sector ? sector->Name() : "";
	QString systemName = system ? system->Name() : "";
	QString worldName = world ? world->Name() : "";

	switch(Structure())
	{
	case SC_Count:
	case SC_Deprecated:
		break;
	case SC_Hull:
	case SC_Spaceship:
		{
			AuDocSpaceshipDestroyed doc(Name(), killer->Name(), galaxy, sectorName, systemName, worldName);
			emit GovDossierAddDocument(EmpireId(), killer->EmpireId(), doc.Pack());
		}
		break;
	case SC_Starship:
		{
			AuDocStarshipDestroyed doc(Name(), killer->Name(), galaxy, sectorName, systemName, worldName);
			emit GovDossierAddDocument(EmpireId(), killer->EmpireId(), doc.Pack());
		}
		break;
	case SC_Station:
		{
			AuDocSpaceStationDestroyed doc(Name(), killer->Name(), galaxy, sectorName, systemName, worldName);
			emit GovDossierAddDocument(EmpireId(), killer->EmpireId(), doc.Pack());
		}
		break;
	}
}

void DaVessel::KillColonist()
{
	DaNpc* npc = FindColonist();
	if(npc)
		KillSubatom(npc->Id());
}

void DaVessel::KillTroop()
{
	// Locate a troop aboard this ship that is assigned to this spacecraft.
	DaNpc* troop = FindTroop();
	if(troop)
		KillSubatom(troop->Id());
}

DaNpc* DaVessel::BoardOfficer(const DaWorldLL& llworld, quint32 empireId, const AuSpecies& species, AuItemHash& inventory,
		quint8 bodyQ)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	if(!OfficerBerthsAvailable())
		return 0;

	// Create an officer atom using the city's worker DNA.
	QString name(llworld.CaptName());
	if(name.isEmpty())
		name = g_Au.Dice().RandomPersonName();
	AuDNABase::CHROMATID chroma = (AuDNABase::CHROMATID)g_Au.Dice().Roll(1,2,-1);
	HANDEDNESS hand = (HANDEDNESS)g_Au.Dice().Roll(1,2,-1);
	DaNpc* officer = new DaNpc(ROLE_Officer, species, chroma, hand, name);
	officer->SetHomeWorld(&llworld);
	officer->ConnectSignals();
	officer->Create();

	// Occupy an AI Officer berth with the officer.
	officer->OccupyBerth(this);		// Also opens comm channels that need monitoring.
	AuPoint berthLoc = BerthLocation(officer->Id());

	// Position the atom in the environment.
	//berthLoc.z += officer->GroundHeightAboveCG();
	berthLoc.z += .1;
	AuMatrix4x4 t;
	t.Translate(berthLoc);	// Move to location in spacecraft.

	// Rotate the atom randomly on its z axis.
	double rot = g_Au.Dice().Roll(1,360,-1) * DEG2RAD;
	t.RotateZ(rot);
	officer->SetPosAndOrient(AuAtomPos(Id(), t));
	ether->AddAtom(officer);

	officer->SetBodyQ(bodyQ);
	officer->ResetHitPoints();
	officer->ResetHunger();
	officer->ResetStamina();
	officer->SetEmpireId(empireId);
	officer->Stand();
	officer->InitTraining();

	AuItemHash itemsTaken;
	officer->InitGear(inventory, itemsTaken, true);
	officer->GoToWork();

	/*
	// Send them on their way with any souvenirs the city has to offer.
	fromCity->LockSituationsForRead().InflictCitizen(officer);
	fromCity->UnlockSituations();
	*/

	g_Au.AtomAdded(officer->Id());
	officer->SetNormal();	// New atom is ready for normal thread processing.
	officer->TickTock();

	IncreaseMonster(1, MONSTER_Officer);
	return officer;
}

DaNpc* DaVessel::BoardCrew(quint32 empireId, const QList<AuSpecies>& species, AuItemHash& inventory, quint8 bodyQ)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	if(!CrewBerthsAvailable())
		return 0;
	if(species.empty())
		return 0;

	// Create a citizen atom using the city's worker DNA.
	AuDNABase::CHROMATID chroma = (AuDNABase::CHROMATID)g_Au.Dice().Roll(1,2,-1);
	HANDEDNESS hand = (HANDEDNESS)g_Au.Dice().Roll(1,2,-1);
	int di = g_Au.Dice().Roll(1, species.size(), -1);
	DaNpc* crew = new DaNpc(ROLE_Crew, species[di], chroma, hand, g_Au.Dice().RandomPersonName());
	crew->ConnectSignals();
	crew->Create();

	// Occupy a crew berth with the crew.
	crew->OccupyBerth(this);
	AuPoint berthLoc = BerthLocation(crew->Id());

	// Position the atom in the environment.
	//berthLoc.z += crew->GroundHeightAboveCG();
	berthLoc.z += .1;
	AuMatrix4x4 t;
	t.Translate(berthLoc);	// Move to location in spacecraft.

	// Rotate the atom randomly on its z axis.
	double rot = g_Au.Dice().Roll(1,360,-1) * DEG2RAD;
	t.RotateZ(rot);
	crew->SetPosAndOrient(AuAtomPos(Id(), t));
	ether->AddAtom(crew);

	const DaWorldLL* world = FindParentWorldInEnv();
	if(world)
		crew->SetHomeWorld(world);
	crew->SetBodyQ(bodyQ);
	crew->ResetHitPoints();
	crew->ResetHunger();
	crew->ResetStamina();
	crew->SetEmpireId(empireId);
	crew->Stand();
	crew->InitTraining();

	AuItemHash itemsTaken;
	crew->InitGear(inventory, itemsTaken, true);
	crew->GoToWork();

	/*
	// Send them on their way with any souvenirs the city has to offer.
	fromCity->LockSituationsForRead().InflictCitizen(crew);
	fromCity->UnlockSituations();
	*/

	g_Au.AtomAdded(crew->Id());
	crew->SetNormal();	// New atom is ready for normal thread processing.
	crew->TickTock();

	IncreaseMonster(1, MONSTER_Crew);
	return crew;
}

DaNpc* DaVessel::BoardColonist(const AuId& fromAirport, const DaWorld* fromWorld, quint32 empireId, const QList<AuSpecies>& species,
		AuItemHash& inventory, quint8 bodyQ)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	if(!PassengerBerthsAvailable())
		return 0;
	if(species.empty())
		return 0;

	AuDNABase::CHROMATID chroma = (AuDNABase::CHROMATID)g_Au.Dice().Roll(1,2,-1);
	HANDEDNESS hand = (HANDEDNESS)g_Au.Dice().Roll(1,2,-1);
	int di = g_Au.Dice().Roll(1, species.size(), -1);
	DaNpc* colonist = new DaNpc(ROLE_Passenger, species[di], chroma, hand, g_Au.Dice().RandomPersonName());
	colonist->ConnectSignals();
	colonist->Create();

	GROWTHSTAGE stage = g_Au.Dice().Roll(1,100) < 15 ? Juvenile : Mature;
	if(stage == Juvenile)
	{
		quint8 fullGrowthTime = colonist->GrowthTime();
		quint32 minutesToFullGrown = (fullGrowthTime + 1) / 2;	// At least 1.
		colonist->SetMinutesToFullGrown(minutesToFullGrown);
	}
	if(fromWorld)
		colonist->SetHomeWorld(fromWorld);
	colonist->SetPortBd(fromAirport);

	// Occupy a passenger berth with the colonist.
	colonist->OccupyBerth(this);
	AuPoint berthLoc = BerthLocation(colonist->Id());

	// Position the atom in the environment.
	//berthLoc.z += colonist->GroundHeightAboveCG();
	berthLoc.z += .1;
	AuMatrix4x4 t;
	t.Translate(berthLoc);	// Move to location in spacecraft.

	// Rotate the atom randomly on its z axis.
	double rot = g_Au.Dice().Roll(1,360,-1) * DEG2RAD;
	t.RotateZ(rot);
	colonist->SetPosAndOrient(AuAtomPos(Id(), t));
	ether->AddAtom(colonist);

	colonist->SetBodyQ(bodyQ);
	colonist->ResetHitPoints();
	colonist->ResetHunger();
	colonist->ResetStamina();
	colonist->SetEmpireId(empireId);
	colonist->Stand();
	colonist->InitTraining();

	AuItemHash itemsTaken;
	colonist->InitGear(inventory, itemsTaken, true);
	colonist->GoToWork();

	// Send them on their way with any souvenirs the city has to offer.
	//DHH city fromCity->LockSituationsForRead().InflictCitizen(colonist);
	//DHH city fromCity->UnlockSituations();

	g_Au.AtomAdded(colonist->Id());
	colonist->SetNormal();	// New atom is ready for normal thread processing.
	colonist->TickTock();

	IncreaseMonster(1, MONSTER_Passenger);
	return colonist;
}

void DaVessel::ListRepairsNeeded(quint32& hitsNeeded, AuRepairParts& partsNeeded) const
{
	m_DamageLock.lockForRead();
	hitsNeeded = m_Damage.HitsDamageNoParts();
	m_Damage.PartsDamage(partsNeeded);
	m_DamageLock.unlock();
}

void DaVessel::ListRepairsNeeded(const AuSysId& sysId, quint32& hitsNeeded, AuRepairParts& partsNeeded) const
{
	m_DamageLock.lockForRead();
	hitsNeeded = m_Damage.HitsDamageNoParts(sysId);
	m_Damage.PartsDamage(sysId, partsNeeded);
	m_DamageLock.unlock();
}

quint32 DaVessel::RepairHitPoints(quint16 sightedPart, quint32 fixCount, AuSysId& sysIdRepaired)
{
	if(!IsDesignDataCacheValid())
	{
		sysIdRepaired = AuSysId();
		return 0;
	}
	sysIdRepaired = SightedSystem(sightedPart);
	if(sysIdRepaired.System() == SYS_Count)
		return 0;
	RepairConditions();

	quint32 hitsFixed = 0;
	m_DamageLock.lockForWrite();
	quint32 hitsNeeded = m_Damage.HitsDamageNoParts(sysIdRepaired);
	if(hitsNeeded)
	{
		if(fixCount > hitsNeeded)
			hitsFixed = hitsNeeded;
		else
			hitsFixed = fixCount;
		m_Damage.RepairHitPoints(sysIdRepaired, hitsFixed);
	}
	m_DamageLock.unlock();
	if(hitsFixed)
		DamageStateChanged();
	return hitsFixed;
}

quint32 DaVessel::RepairDamage(ROOMTYPE inRoomType, quint16 sightedPart, const AuItem& sparePart, quint32 fixCount,
		AuSysId& sysIdRepaired, bool& systemModuleChange, bool forceUpgrade)
{
	if(!IsDesignDataCacheValid())
	{
		sysIdRepaired = AuSysId();
		return 0;
	}
	RepairConditions();

	// Apply the commodity to the sighted system.
	quint32 fixedCount = ReplaceComponent(inRoomType, sightedPart, sparePart.CommodityId(), sparePart.Quality(), fixCount,
			sysIdRepaired, systemModuleChange, forceUpgrade);
	if(fixedCount)
	{
		// One or more spare parts were used to repair damage to the system.
		DamageStateChanged();
		return fixedCount;
	}

	sysIdRepaired = AuSysId();
	return 0;
}

// 'sightedPart' is 0 for the hull or 1..n for a detail id.
quint32 DaVessel::RepairDamage(ROOMTYPE inRoomType, quint16 sightedPart, COMMODITYID cid, quint32 fixCount, AuSysId& sysIdRepaired,
		bool& systemModuleChange)
{
	// Locate the spare part commodity in the hold.
	m_CargoLock.lockForWrite();
	AuItemHash::iterator cargoI = m_Cargo.FindBest(cid);
	if(cargoI == m_Cargo.end())
	{
		m_CargoLock.unlock();
		sysIdRepaired = AuSysId();
		return 0;
	}

	// Limit the amount we can fix to the amount in the stack.
	if(fixCount > (*cargoI).second->Count())
		fixCount = (*cargoI).second->Count();

	// Apply the commodity to the sighted system.
	// If the commodity is used, remove it from the hold.
	quint32 fixedCount = RepairDamage(inRoomType, sightedPart, *(*cargoI).second, fixCount, sysIdRepaired, systemModuleChange,
			false);
	if(fixedCount)
	{
		// The spare part was used to repair damage to the system. Remove one of them from the hold.
		RemoveCargoNoLock(*(*cargoI).second, fixedCount);
		m_CargoLock.unlock();
		return fixedCount;
	}

	m_CargoLock.unlock();
	sysIdRepaired = AuSysId();
	return 0;
}

// Pirate/agent version of RepairAll
quint16 DaVessel::RepairAll()
{
	RepairConditions();

	// Determine what is needed to repair all the systems of the ship.
	quint32 hitsNeeded;
	AuRepairParts partsNeeded;
	ListRepairsNeeded(hitsNeeded, partsNeeded);
	unsigned int totalRepairSecs = (hitsNeeded + REPAIR_PER_SEC - 1) / REPAIR_PER_SEC;
	if(hitsNeeded || !partsNeeded.empty())
	{
		// Calculate the total time to repair.
		for(QHash<COMMODITYID, quint32>::const_iterator pni = partsNeeded.cbegin(); pni != partsNeeded.cend(); pni++)
			totalRepairSecs += *pni;

		// Repair the spacecraft.
		m_DamageLock.lockForWrite();
		m_Damage.RepairAll();
		m_DamageLock.unlock();

		DamageStateChanged();
	}

	if(totalRepairSecs > 0xffff)
		return 0xffff;
	else
		return (quint16)totalRepairSecs;
}

// Repair all damaged systems using whatever parts are available from city's inventory.
void DaVessel::RepairAllPossible(DaBd* bd, quint8 repairQ)
{
	RepairConditions();

	// Determine what is needed to repair all the systems of the ship.
	quint32 hitsNeeded;
	AuRepairParts partsNeeded;
	ListRepairsNeeded(hitsNeeded, partsNeeded);
	if(hitsNeeded || !partsNeeded.empty())
	{
		// Consume as many of the spare parts from city's inventory as it still has on hand.
		// Repair as much of the spacecraft as possible using the parts on hand.
		QSet<AuSysId> systemsRepaired;
		m_DamageLock.lockForWrite();
		for(AuRepairParts::const_iterator pni = partsNeeded.cbegin(); pni != partsNeeded.cend(); pni++)
			m_Damage.RepairParts(pni.key(), bd->CommunityConsumeSpareParts(pni.key(), repairQ, *pni), systemsRepaired);
		m_Damage.RepairAllHitPoints();	// Does not affect system quality.
		m_DamageLock.unlock();

		DowngradeSystemModules(repairQ, systemsRepaired);
		DamageStateChanged();
		bd->SetModified();
	}
}

void DaVessel::RepairAllPossible(DaBd* bd, const AuSysId& sysId, quint8 repairQ)
{
	RepairConditions();

	// Determine what is needed to repair all the systems of the ship.
	quint32 hitsNeeded;
	AuRepairParts partsNeeded;
	ListRepairsNeeded(sysId, hitsNeeded, partsNeeded);
	if(hitsNeeded || !partsNeeded.empty())
	{
		// Consume as many of the spare parts from city's inventory as it still has on hand.
		// Repair as much of the spacecraft as possible using the parts on hand.
		QSet<AuSysId> systemsRepaired;
		m_DamageLock.lockForWrite();
		for(AuRepairParts::const_iterator pni = partsNeeded.cbegin(); pni != partsNeeded.cend(); pni++)
			m_Damage.RepairParts(sysId, pni.key(), bd->CommunityConsumeSpareParts(pni.key(), repairQ, *pni), systemsRepaired);
		m_Damage.RepairAllHitPoints(sysId);	// Does not affect system quality.
		m_DamageLock.unlock();

		DowngradeSystemModules(repairQ, systemsRepaired);
		DamageStateChanged();
		bd->SetModified();
	}
}

// Repair all damaged systems. Use whatever parts are needed from city's inventory.
// This is used in response to a bona fide repair offer. The city insured it had what was needed before it made the repair offer.
// If the stuff is missing too bad; we'll fake it and nobody will ever know.
void DaVessel::RepairAll(DaBd* bd)
{
	RepairConditions();

	// Determine what is needed to repair all the systems of the ship.
	quint32 hitsNeeded;
	AuRepairParts partsNeeded;
	ListRepairsNeeded(hitsNeeded, partsNeeded);
	if(hitsNeeded || !partsNeeded.empty())
	{
		// Consume as many of the spare parts from city's inventory as it still has on hand.
		// Scrap will be consumed if needed.
		quint8 repairQ = QualityMade();
		for(AuRepairParts::const_iterator pni = partsNeeded.cbegin(); pni != partsNeeded.cend(); pni++)
			bd->CommunityConsumeSpareParts(pni.key(), repairQ, *pni);
		bd->SetModified();

		// Repair the spacecraft.
		m_DamageLock.lockForWrite();
		m_Damage.RepairAll();
		m_DamageLock.unlock();

		DamageStateChanged();
	}
}

// Repair all damage to a certain system. Use whatever parts are needed from city's inventory.
// This is used in response to a bona fide repair offer. The city insured it had what was needed before it made the repair offer.
// If the stuff is missing too bad; we'll fake it and nobody will ever know.
void DaVessel::RepairSystem(DaBd* bd, const AuSysId& sysId, quint8 repairQ)
{
	RepairConditions();

	// Determine what is needed to repair the system of the ship.
	quint32 hitsNeeded;
	AuRepairParts partsNeeded;
	ListRepairsNeeded(sysId, hitsNeeded, partsNeeded);
	if(hitsNeeded || !partsNeeded.empty())
	{
		// Consume as many of the spare parts from city's inventory as it still has on hand.
		for(AuRepairParts::const_iterator pni = partsNeeded.cbegin(); pni != partsNeeded.cend(); pni++)
			bd->CommunityConsumeSpareParts(pni.key(), repairQ, *pni);
		bd->SetModified();

		// Repair the system of the spacecraft.
		QSet<AuSysId> systemsRepaired;
		m_DamageLock.lockForWrite();
		m_Damage.RepairAll(sysId, systemsRepaired);
		m_DamageLock.unlock();

		DowngradeSystemModules(repairQ, systemsRepaired);
		DamageStateChanged();
	}
}

void DaVessel::RepairSystemExcept(DaBd* bd, const AuSysId& sysId, QList<COMMODITYID> cidExceptions, quint8 repairQ)
{
	RepairConditions();

	// Determine what is needed to repair the system of the ship.
	quint32 hitsNeeded;
	AuRepairParts partsNeeded;
	ListRepairsNeeded(sysId, hitsNeeded, partsNeeded);
	if(hitsNeeded || !partsNeeded.empty())
	{
		// Consume as many of the spare parts from city's inventory as it still has on hand.
		for(QHash<COMMODITYID, quint32>::const_iterator pni = partsNeeded.cbegin(); pni != partsNeeded.cend(); pni++)
			if(!cidExceptions.contains(pni.key()))
				bd->CommunityConsumeSpareParts(pni.key(), repairQ, *pni);
		bd->SetModified();

		// Repair the system components we can of the spacecraft.
		QSet<AuSysId> systemsRepaired;
		m_DamageLock.lockForWrite();
		m_Damage.RepairAllExcept(sysId, cidExceptions, systemsRepaired);
		m_DamageLock.unlock();

		DowngradeSystemModules(repairQ, systemsRepaired);
		DamageStateChanged();
	}
}

void DaVessel::PowerPlantShuntOn(int shuntIndex)
{
	m_PowerPlantShunts |= (1<<shuntIndex);
	SetModified(NOTIFY_Database);
}

void DaVessel::PowerPlantShuntOff(int shuntIndex)
{
	m_PowerPlantShunts &= ~(1<<shuntIndex);
	SetModified(NOTIFY_Database);
}

// Return true on system needing updating
bool DaVessel::UpdateSystemsHealth()
{
	m_SystemsLock.lockForWrite();
	bool v = UpdateSystemsHealthSystemsLocked();
	m_SystemsLock.unlock();
	return v;
}

// Return true on system needing updating
bool DaVessel::UpdateSystemsHealthSystemsLocked()
{
	// Solo designer preview needs this. Q_ASSERT(!g_Au.IsSceneClient());	// Client does not rx enough m_ssaShipSystems for this.
	bool bReturn = false;
	for(AuShipSystemArray::iterator si = m_ssaShipSystems.begin(); si != m_ssaShipSystems.end(); si++)
	{
		unsigned int systemHealth = SystemHitsRemainingSystemsLocked((*si).SysId());
		if((*si).GetHealthCurrent() != systemHealth)
		{
			(*si).SetHealthCurrent(systemHealth);
			bReturn = true;
		}
	}
	return bReturn;
}

// Return true on system needing updating
void DaVessel::UpdateSystemsPower()
{
	m_SystemsLock.lockForWrite();
	UpdateSystemsPowerSystemsLocked();
	m_SystemsLock.unlock();
}

// Return true on system needing updating
void DaVessel::UpdateSystemsPowerSystemsLocked()
{
	// Solo designer preview needs this. Q_ASSERT(!g_Au.IsSceneClient());	// Client does not rx enough m_ssaShipSystems for this.
	// Recalculate power consumed by all systems.
	bool anyChange = false;
	for(int systemIndex = 0; systemIndex < m_ssaShipSystems.count(); systemIndex++)
		if(m_ssaShipSystems.RecalculatePowerConsumption(systemIndex))
			anyChange = true;
	if(anyChange)
	{
		m_tmSystemsUpg.Set();
		SetModified();
	}
}

quint32 DaVessel::AssessDamage(unsigned int sightedPart, COMMODITYID cid) const
{
	if(!IsDesignDataCacheValid())
		return 0;

	quint32 countNeeded = 0;
	if(cid == CID_Invalid)
	{
		// Return hits points of damage to the system, not including parts.
		AuSysId sysId = SightedSystem(sightedPart);
		AuRepairParts partsNeeded;
		if(sysId.System() == SYS_Hull || sysId.System() == SYS_Count)
		{
			// Hull is comprised of several systems.
			quint32 hitsNeeded;
			ListRepairsNeeded(SYS_Hull, hitsNeeded, partsNeeded);
			countNeeded += hitsNeeded;

			ListRepairsNeeded(SYS_Fuel, hitsNeeded, partsNeeded);
			countNeeded += hitsNeeded;

			ListRepairsNeeded(SYS_Hold, hitsNeeded, partsNeeded);
			countNeeded += hitsNeeded;
		}
		else
			ListRepairsNeeded(sysId, countNeeded, partsNeeded);
	}
	else
	{
		AuSysId sysId = SightedSystem(sightedPart);
		quint32 hitsNeeded;
		AuRepairParts partsNeeded;
		if(sysId.System() == SYS_Hull || sysId.System() == SYS_Count)
		{
			// Hull is comprised of several systems.
			quint32 hitsNeeded;
			ListRepairsNeeded(SYS_Hull, hitsNeeded, partsNeeded);
			AuRepairParts::const_iterator pi = partsNeeded.find(cid);
			if(pi != partsNeeded.cend())
				countNeeded += *pi;

			ListRepairsNeeded(SYS_Fuel, hitsNeeded, partsNeeded);
			pi = partsNeeded.find(cid);
			if(pi != partsNeeded.cend())
				countNeeded += *pi;

			ListRepairsNeeded(SYS_Hold, hitsNeeded, partsNeeded);
			pi = partsNeeded.find(cid);
			if(pi != partsNeeded.cend())
				countNeeded += *pi;
		}
		else
		{
			ListRepairsNeeded(sysId, hitsNeeded, partsNeeded);
			if(!partsNeeded.empty())
			{
				// We really only expect one entry in 'partsEntry'.
				AuRepairParts::const_iterator pi = partsNeeded.find(cid);
				if(pi != partsNeeded.cend())
					countNeeded = *pi;		// The system needs the part in question.
				else if(ScModuleSystem(cid) == sysId.System())
				{
					// The module is compatible with the system.
					// A module can change a system, when every previous module has been removed.
					quint32 sysMax = SystemComponents(sysId, cid);
					if(sysMax)
					{
						// Is the system completely out of modules?
						pi = partsNeeded.cbegin();
						if(*pi >= sysMax)
						{
							// The system is completely out of modules. A different compatible module can be installed.
							countNeeded = *pi;
						}
					}
				}
			}
		}
	}
	return countNeeded;
}

// NOTE: This function must not perform any actions that require the ship design data cache to be valid.
void DaVessel::Reactivate()
{
	// Moor the ship and shut off all systems. The owner may not show up for a long time, maybe never if they don't log back in.
	m_Moored = true;							// Moor to prevent abandoned private ships from crashing over and over.
	CloseAllHullDoors();						// Insure all hull doors are closed so the air doesn't leak out.
	m_bExtLightOn = false;
	m_bPowerPlantOn = false;
	m_bSensorsOn = false;
	m_ShieldState = SS_Off;

	m_VesselLock.lockForWrite();
	m_eGravityDriveState = GD_Off;
	m_InTow = false;
	m_wActions = 0;
	m_VesselLock.unlock();

	m_ControlLock.lockForWrite();
	m_Throttle = 0;
	m_ThrustLateral = 0;
	m_ThrustLong = 0;
	m_ThrustVertical = 0;
	m_ControlLock.unlock();

	// Clear bay fired data, so we don't hang onto things with our tractor beam if it was on.
	m_BayLock.lockForWrite();
	m_BayFired.clear();
	m_BayLock.unlock();

	MissionSetActive(false);			// Insure mission is no longer active.
	RemoveAllWaypoints();				// Clear the current route.
	SetAtRest();						// Stop moving.
	InvalidateEnvironmentRecursive();
	SetNormal();						// Ship atom becomes dormant until summoned by the captain or crew.
	SetModified();

	// Awaken my subatoms.
	DaAtomList shipAtoms;
	ListSubatomsRecursive(shipAtoms);
	for(DaAtomList::const_iterator ai = shipAtoms.begin(); ai != shipAtoms.end(); ai++)
		if((*ai)->IsDormant())
			(*ai)->SetNormal();

	TickTockRecursive();

	// Start monitoring comm channels.
	MonitorCommChannels();

	// Hail on the crew channel so the person knows I am here.
	if(HasHumanOfficer())
		AnswerHail(AuCommStdChannel::CrewChannelName(), AuCommStdChannel::SolarSystem, AuCommStdChannel::CrewMates, false, 0);
	else
		AnswerHail(AuCommStdChannel::CompanyChannelName(), AuCommStdChannel::SolarSystem, AuCommStdChannel::Company, false, 0);
}

void DaVessel::SelfDestruct()
{
	DamageAndDestroy(tr("Self destruct sequence complete."));
}

/*
// Test for Weapon console occupied by AI or a person.
bool DaVessel::IsWeaponManned() const
{
	if(m_pAtoms)
	{
		for(DaAtomList::const_iterator it = m_pAtoms->begin(); it != m_pAtoms->end(); it++)
		{
			DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);
			if(!pAtom)
				continue;
			if(!pAtom->IsNormal())
				continue;
			if(pAtom->IsFireControlStationPose())
				return true;
		}
	}
	return false;
}
*/

// Test for Weapon console occupied by AI.
bool DaVessel::IsWeaponMannedByAI() const
{
	// If the occupancy state of the console is known, return the cached value.
	if(m_ConWeaponAI)
		return m_ConWeaponAI == 2;

	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsFireControlStationPose() && pAtom->IsAIControlled())
		{
			m_ConWeaponAI = 2;	// yes
			return true;
		}
	}
	m_ConWeaponAI = 1;	// no
	return false;
}

// Test for Weapon console occupied by a person.
bool DaVessel::IsWeaponMannedByPerson() const
{
	// If the occupancy state of the console is known, return the cached value.
	if(m_ConWeaponPerson)
		return m_ConWeaponPerson - 1;

	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaPerson* pAtom = dynamic_cast<DaPerson*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsFireControlStationPose() && !pAtom->IsAIControlled())
		{
			m_ConWeaponPerson = 2;	// yes
			return true;
		}
	}
	m_ConWeaponPerson = 1;	// no
	return false;
}

// Test for Pilot console occupied by AI.
bool DaVessel::IsPowerRelayMannedByAI() const
{
	// If the occupancy state of the console is known, return the cached value.
	if(m_ConPowerRelayAI)
		return m_ConPowerRelayAI == 2;

	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsPowerStationPose() && pAtom->IsAIControlled())
		{
			m_ConPowerRelayAI = 2;	// yes
			return true;
		}
	}
	m_ConPowerRelayAI = 1;	// no
	return false;
}

// Test for Pilot console occupied by a person.
bool DaVessel::IsPowerRelayMannedByPerson() const
{
	// If the occupancy state of the console is known, return the cached value.
	if(m_ConPowerRelayPerson)
		return m_ConPowerRelayPerson - 1;

	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaPerson* pAtom = dynamic_cast<DaPerson*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsPowerStationPose() && !pAtom->IsAIControlled())
		{
			m_ConPowerRelayPerson = 2;	// yes
			return true;
		}
	}
	m_ConPowerRelayPerson = 1;	// no
	return false;
}

// Test for Sensor console occupied by AI.
bool DaVessel::IsSensorMannedByAI() const
{
	// If the occupancy state of the console is known, return the cached value.
	if(m_ConSensorAI)
		return m_ConSensorAI == 2;

	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsSensorStationPose() && pAtom->IsAIControlled())
		{
			m_ConSensorAI = 2;	// yes
			return true;
		}
	}
	m_ConSensorAI = 1;	// no
	return false;
}

// Test for Sensor console occupied by a person.
bool DaVessel::IsSensorMannedByPerson() const
{
	// If the occupancy state of the console is known, return the cached value.
	if(m_ConSensorPerson)
		return m_ConSensorPerson - 1;

	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaPerson* pAtom = dynamic_cast<DaPerson*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsSensorStationPose() && !pAtom->IsAIControlled())
		{
			m_ConSensorPerson = 2;	// yes
			return true;
		}
	}
	m_ConSensorPerson = 1;	// no
	return false;
}

// Test for Shield console occupied by AI.
bool DaVessel::IsShieldMannedByAI() const
{
	// If the occupancy state of the console is known, return the cached value.
	if(m_ConShieldAI)
		return m_ConShieldAI == 2;

	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsShieldStationPose() && pAtom->IsAIControlled())
		{
			m_ConShieldAI = 2;	// yes
			return true;
		}
	}
	m_ConShieldAI = 1;	// no
	return false;
}

// Test for Shield console occupied by a person.
bool DaVessel::IsShieldMannedByPerson() const
{
	// If the occupancy state of the console is known, return the cached value.
	if(m_ConShieldPerson)
		return m_ConShieldPerson - 1;

	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaPerson* pAtom = dynamic_cast<DaPerson*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsShieldStationPose() && !pAtom->IsAIControlled())
		{
			m_ConShieldPerson = 2;	// yes
			return true;
		}
	}
	m_ConShieldPerson = 1;	// no
	return false;
}

// Find first officer and determine if he has the con.
bool DaVessel::IsCommandedByAI() const
{
	// If the state of the first officer is known, return the cached value.
	if(m_ConCaptAI)
		return m_ConCaptAI == 2;

	AuId firstOfficerId;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* officer = dynamic_cast<DaSentient*>(*it);
		if(!officer)
			continue;
		if(!officer->IsAIControlled())
			continue;
		if(Is(officer->AssignedShip()))
		{
			// Demiavatars may hold non-officer berths to perform the duties of other crew positions.
			if(!AuRosterEntry::IsOfficer(officer->AssignedShipBerthType()))
				continue;

			// Found an officer assigned to this ship.
			if(officer->HasTheCon())
			{
				// First officer who has the con.
				AuId oid(officer->Id());

				m_VesselLock.lockForWrite();
				m_ConAICaptId = oid;
				m_ConCaptAI = 2;
				m_VesselLock.unlock();
				return true;
			}
			else if(firstOfficerId.IsNull())
			{
				if(dynamic_cast<DaNpc*>(officer) || dynamic_cast<DaPerson*>(officer))
					firstOfficerId = officer->Id();			// First officer (officer or demiavatar) who does not have the con.
			}
		}
	}

	m_VesselLock.lockForWrite();
	m_ConAICaptId = firstOfficerId;
	m_ConCaptAI = 1;	// no
	m_VesselLock.unlock();
	return false;
}

/*
// Test for Engineer console occupied by AI or a person.
bool DaVessel::IsEngineerManned() const
{
	if(m_pAtoms)
	{
		for(DaAtomList::const_iterator it = m_pAtoms->begin(); it != m_pAtoms->end(); it++)
		{
			DaCreature* pAtom = dynamic_cast<DaCreature*>(*it);
			if(!pAtom)
				continue;
			if(!pAtom->IsNormal())
				continue;
			if(pAtom->IsEngineStationPose())
				return true;
		}
	}
	return false;
}
*/

// Test for Engineer console occupied by AI.
bool DaVessel::IsEngineerMannedByAI() const
{
	// If the occupancy state of the console is known, return the cached value.
	if(m_ConEngineerAI)
		return m_ConEngineerAI == 2;

	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsEngineStationPose() && pAtom->IsAIControlled())
		{
			m_ConEngineerAI = 2;	// yes
			return true;
		}
	}
	m_ConEngineerAI = 1;	// no
	return false;
}

// Test for Engineer console occupied by a person.
bool DaVessel::IsEngineerMannedByPerson() const
{
	// If the occupancy state of the console is known, return the cached value.
	if(m_ConEngineerPerson)
		return m_ConEngineerPerson - 1;

	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaPerson* pAtom = dynamic_cast<DaPerson*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsEngineStationPose() && !pAtom->IsAIControlled())
		{
			m_ConEngineerPerson = 2;	// yes
			return true;
		}
	}
	m_ConEngineerPerson = 1;	// no
	return false;
}

// Test for Navigator console occupied by AI.
bool DaVessel::IsNavigatorMannedByAI() const
{
	// If the occupancy state of the console is known, return the cached value.
	if(m_ConNavigatorAI)
		return m_ConNavigatorAI == 2;

	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsNavStationPose() && pAtom->IsAIControlled())
		{
			m_ConNavigatorAI = 2;	// yes
			return true;
		}
	}
	m_ConNavigatorAI = 1;	// no
	return false;
}

// Test for Navigator console occupied by a person.
bool DaVessel::IsNavigatorMannedByPerson() const
{
	// If the occupancy state of the console is known, return the cached value.
	if(m_ConNavigatorPerson)
		return m_ConNavigatorPerson - 1;

	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaPerson* pAtom = dynamic_cast<DaPerson*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsNavStationPose() && !pAtom->IsAIControlled())
		{
			m_ConNavigatorPerson = 2;	// yes
			return true;
		}
	}
	m_ConNavigatorPerson = 1;	// no
	return false;
}

// Test for Pilot console occupied by AI.
bool DaVessel::IsPilotMannedByAI() const
{
	// If the occupancy state of the console is known, return the cached value.
	if(m_ConPilotAI)
		return m_ConPilotAI == 2;

	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsHelmStationPose() && pAtom->IsAIControlled())
		{
			m_ConPilotAI = 2;	// yes
			return true;
		}
	}
	m_ConPilotAI = 1;	// no
	return false;
}

// Test for Pilot console occupied by a person.
bool DaVessel::IsPilotMannedByPerson() const
{
	// If the occupancy state of the console is known, return the cached value.
	if(m_ConPilotPerson)
		return m_ConPilotPerson - 1;

	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaPerson* pAtom = dynamic_cast<DaPerson*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsHelmStationPose() && !pAtom->IsAIControlled())
		{
			m_ConPilotPerson = 2;	// yes
			return true;
		}
	}
	m_ConPilotPerson = 1;	// no
	return false;
}

DaSentient* DaVessel::Pilot() const
{
	// When we KNOW there is no pilot, just return 0.
	if(m_ConPilotPerson == 1 && m_ConPilotAI == 1)
		return 0;

	DaSentient* pilot = 0;										// Pilot if no human on line is found.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* creature = dynamic_cast<DaSentient*>(*it);
		if(!creature)
			continue;
		if(!creature->IsHelmStationPose())
			continue;

		DaPerson* person = dynamic_cast<DaPerson*>(creature);
		if(person && !person->IsAIControlled())
		{
			// Found a person aboard this ship at a pilot console.
			if(!person->IsInvisibleOffLine())
				return person;
		}
		else if(!pilot)
			pilot = creature;									// Remember crew pilot in case we find no human engineers.
	}
	return pilot;
}

DaSentient* DaVessel::Navigator() const
{
	// When we KNOW there is no navigator, just return 0.
	if(m_ConNavigatorPerson == 1 && m_ConNavigatorAI == 1)
		return 0;

	DaSentient* navigator = 0;									// Navigator if no human on line is found.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* creature = dynamic_cast<DaSentient*>(*it);
		if(!creature)
			continue;
		if(!creature->IsNavStationPose())
			continue;

		DaPerson* person = dynamic_cast<DaPerson*>(creature);
		if(person && !person->IsAIControlled())
		{
			// Found a person aboard this ship at a navigator console.
			if(!person->IsInvisibleOffLine())
				return person;
		}
		else if(!navigator)
			navigator = creature;								// Remember crew navigator in case we find no human engineers.
	}
	return navigator;
}

DaSentient* DaVessel::Engineer() const
{
	// When we KNOW there is no engineer, just return 0.
	if(m_ConEngineerPerson == 1 && m_ConEngineerAI == 1)
		return 0;

	DaSentient* engineer = 0;									// Engineer if no human on line is found.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* creature = dynamic_cast<DaSentient*>(*it);
		if(!creature)
			continue;
		if(!creature->IsEngineStationPose())
			continue;

		DaPerson* person = dynamic_cast<DaPerson*>(creature);
		if(person && !person->IsAIControlled())
		{
			// Found a person aboard this ship at a engineer console.
			if(!person->IsInvisibleOffLine())
				return person;
		}
		else if(!engineer)
			engineer = creature;								// Remember crew engineer in case we find no human engineers.
	}
	return engineer;
}

DaSentient* DaVessel::PowerOp() const
{
	// When we KNOW there is no power operator, just return 0.
	if(m_ConPilotPerson == 1 && m_ConPilotAI == 1)
		return 0;

	DaSentient* power = 0;										// Pilot if no human on line is found.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* creature = dynamic_cast<DaSentient*>(*it);
		if(!creature)
			continue;
		if(!creature->IsPowerStationPose())
			continue;

		DaPerson* person = dynamic_cast<DaPerson*>(creature);
		if(person && !person->IsAIControlled())
		{
			// Found a person aboard this ship at a power console.
			if(!person->IsInvisibleOffLine())
				return person;
		}
		else if(!power)
			power = creature;									// Remember crew power in case we find no human power.
	}
	return power;
}

DaSentient* DaVessel::SensorOp() const
{
	// When we KNOW there is no sensor, just return 0.
	if(m_ConSensorPerson == 1 && m_ConSensorAI == 1)
		return 0;

	DaSentient* sensor = 0;										// Sensor if no human on line is found.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* creature = dynamic_cast<DaSentient*>(*it);
		if(!creature)
			continue;
		if(!creature->IsSensorStationPose())
			continue;

		DaPerson* person = dynamic_cast<DaPerson*>(creature);
		if(person && !person->IsAIControlled())
		{
			// Found a person aboard this ship at a sensor console.
			if(!person->IsInvisibleOffLine())
				return person;
		}
		else if(!sensor)
			sensor = creature;									// Remember crew sensor in case we find no human sensors.
	}
	return sensor;
}

DaSentient* DaVessel::ShieldOp() const
{
	// When we KNOW there is no shield, just return 0.
	if(m_ConShieldPerson == 1 && m_ConShieldAI == 1)
		return 0;

	DaSentient* shield = 0;										// Shield if no human on line is found.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* creature = dynamic_cast<DaSentient*>(*it);
		if(!creature)
			continue;
		if(!creature->IsShieldStationPose())
			continue;

		DaPerson* person = dynamic_cast<DaPerson*>(creature);
		if(person && !person->IsAIControlled())
		{
			// Found a person aboard this ship at a shield console.
			if(!person->IsInvisibleOffLine())
				return person;
		}
		else if(!shield)
			shield = creature;									// Remember crew shield in case we find no human shields.
	}
	return shield;
}

DaSentient* DaVessel::AIEngineOperator() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsEngineStationPose() && pAtom->IsAIControlled())
			return pAtom;
	}
	return 0;
}

AuId DaVessel::AIMedic() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsMedicStationPose() && pAtom->IsAIControlled())
			return pAtom->Id();
	}
	return AuId(0);
}

AuId DaVessel::AIPatient() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsPatientStationPose() && pAtom->IsAIControlled())
			return pAtom->Id();
	}
	return AuId(0);
}

AuId DaVessel::AINavigator() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsNavStationPose() && pAtom->IsAIControlled())
			return pAtom->Id();
	}
	return AuId(0);
}

AuId DaVessel::AIPilot() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsHelmStationPose() && pAtom->IsAIControlled())
			return pAtom->Id();
	}
	return AuId(0);
}

AuId DaVessel::AIPowerRelay() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsPowerStationPose() && pAtom->IsAIControlled())
			return pAtom->Id();
	}
	return AuId(0);
}

DaSentient* DaVessel::AIPowerRelayOperator()
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsPowerStationPose() && pAtom->IsAIControlled())
			return pAtom;
	}
	return 0;
}

AuId DaVessel::AISensor() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsSensorStationPose() && pAtom->IsAIControlled())
			return pAtom->Id();
	}
	return AuId(0);
}

DaSentient* DaVessel::AISensorOperator()
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsSensorStationPose() && pAtom->IsAIControlled())
			return pAtom;
	}
	return 0;
}

AuId DaVessel::AIShield() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsShieldStationPose() && pAtom->IsAIControlled())
			return pAtom->Id();
	}
	return AuId(0);
}

AuId DaVessel::AITransporter() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsTransporterStationPose() && pAtom->IsAIControlled())
			return pAtom->Id();
	}
	return AuId(0);
}

AuId DaVessel::AITurretGunner() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsTurretPose() && pAtom->IsAIControlled())
			return pAtom->Id();
	}
	return AuId(0);
}

AuId DaVessel::AIFireControl() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsFireControlStationPose() && pAtom->IsAIControlled())
			return pAtom->Id();
	}
	return AuId(0);
}

AuId DaVessel::AIGamer() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsGameTablePose() && pAtom->IsAIControlled())
			return pAtom->Id();
	}
	return AuId(0);
}

AuId DaVessel::AITroopAtPost() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsPostPose() && pAtom->IsAIControlled())
			return pAtom->Id();
	}
	return AuId(0);
}

AuId DaVessel::AICitizenAtPost() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsPostPose() && pAtom->IsAIControlled())
			return pAtom->Id();
	}
	return AuId(0);
}

AuId DaVessel::AILivestockAtPost() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaAnimal* pAtom = dynamic_cast<DaAnimal*>(*it);
		if(!pAtom)
			continue;
		if(pAtom->IsPostPose())
			return pAtom->Id();
	}
	return AuId(0);
}

// Find the AI officer in charge of this ship.
DaSentient* DaVessel::FindCommandingOfficer() const
{
	if(IsCommandedByAI() && !AIFirstOfficer().IsNull())
	{
		// We know there is supposed to be an AI first officer aboard who is in command.
		const DaSentient* officer = dynamic_cast<const DaSentient*>(FindIdRecursive(AIFirstOfficer()));
		if(officer && officer->HasTheCon() && officer->IsAIControlled())
			return (DaSentient*)officer;						// Discard const
	}
	return 0;
}

// Locates any officer assigned to this ship. Prefers the officer with the con.
DaSentient* DaVessel::FindOfficer() const
{
	// If we already have the first officer id, return that guy.
	DaSentient* officer = FindCommandingOfficer();
	if(officer)
		return officer;

	if(!AIFirstOfficer().IsNull())
	{
		// We have found an officer aboard. They must not have the con.
		const DaSentient* officer = dynamic_cast<const DaSentient*>(FindIdRecursive(AIFirstOfficer()));
		if(officer)
			return (DaSentient*)officer;						// Discard const
	}

	return 0;
}

void DaVessel::KickAIStationOccupant(quint16 stationId, unsigned int seatIndex)
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaSentient* pAtom = dynamic_cast<DaSentient*>(*it);	// Only care about AI controlled citizens.
		if(!pAtom)
			continue;
		if(!pAtom->IsAIControlled())
			continue;
		if(pAtom->StationId() != stationId)
			continue;
		if(pAtom->ConsoleSeatIndex() != seatIndex)
			continue;	// On new surgery station, station index 0 is the doctor; 1 is the patient.

		// Kick this dude from the console.
		pAtom->ExitConsole();
		pAtom->WakeUp();									// Wake them so they don't hang around so long.
		return;
	}
}

bool DaVessel::CanAddCargo(const AuItem& iNew) const
{
	if(!IsDesignDataCacheValid())
		return false;	// Can't add cargo until design is loaded and valid.

	//if(iNew.Content() && !iNew.Content()->empty())
		//return false;	// Can't put non-empty containers into the hold.

	if(iNew.CommodityId() == CID_Hydrogen)
	{
		// Hydrogen must be stored in fuel cells.
		int fuelNeeded = FuelWanted();
		return ((int)iNew.Count() > fuelNeeded) ? false : true;
	}
	else if(iNew.CommodityId() == CID_Electricity)
	{
		// Electricity must be stored in capacitor.
		int powerNeeded = (int)ceil(CapacitorMax() - CapacitorLevel());
		return ((int)iNew.Count() > powerNeeded) ? false : true;
	}
	else
	{
		// Cargo capacity of ship must not be exceeded.
		m_CargoLock.lockForRead();
		int cargoRoom = CargoCapacityVolume() - m_Cargo.TotalCargoVolume();
		if((int)iNew.CargoVolume() > cargoRoom)
		{
			m_CargoLock.unlock();
			return false;
		}

		bool v = m_Cargo.CanAddItem(iNew);
		m_CargoLock.unlock();
		return v;
	}
}

bool DaVessel::CanAddCargoTrim(AuItem& iNew) const
{
	if(!IsDesignDataCacheValid())
		return false;	// Can't add cargo until design is loaded and valid.

	//if(iNew.Content() && !iNew.Content()->empty())
		//return false;	// Can't put non-empty containers into the hold.

	if(iNew.CommodityId() == CID_Hydrogen)
	{
		// Hydrogen must be stored in fuel cells.
		unsigned int fuelNeeded = FuelWanted();
		if(fuelNeeded == 0)
			return false;
		if(iNew.Count() > fuelNeeded)
			iNew.SetCount(fuelNeeded);
		return true;
	}
	else if(iNew.CommodityId() == CID_Electricity)
	{
		// Electricity must be stored in capacitor.
		unsigned int powerNeeded = (int)ceil(CapacitorMax() - CapacitorLevel());
		if(powerNeeded == 0)
			return false;
		if(iNew.Count() > powerNeeded)
			iNew.SetCount(powerNeeded);
		return true;
	}
	else
	{
		// Cargo capacity of ship must not be exceeded.
		m_CargoLock.lockForRead();
		unsigned int cargoRoom = CargoCapacityVolume() - m_Cargo.TotalCargoVolume();
		m_CargoLock.unlock();
		if(cargoRoom == 0)
			return false;
		if(iNew.CargoVolume() > cargoRoom)
			iNew.SetCount(cargoRoom);
		return true;
	}
}

// Be sure to call CanAddCargo first. This function doesn't check.
void DaVessel::AddCargo(const AuItem& iNew)
{
	if(!iNew.Count())
	{
		qDebug() << "DaVessel::AddCargo Skipping invalid iNew" << iNew.ListDesc();
		return;
	}
	//Q_ASSERT(IsDesignDataCacheValid());	// Is tested by CanAddCargo.
	if(iNew.CommodityId() == CID_Hydrogen)
	{
		// Calculate nu average quality.
		m_VesselLock.lockForWrite();
		double nuTotal = m_fFuelQuantity + iNew.Count();
		if(nuTotal >= 1)
		{
			int fuelQual = (int)round(((m_fFuelQuantity * m_FuelQuality) + ((double)iNew.Count() * iNew.Quality())) / nuTotal);
			if(fuelQual <= 0)
				m_FuelQuality = 1;
			else if(fuelQual > 255)
				m_FuelQuality = 255;
			else
				m_FuelQuality = fuelQual;
		}
		else
			m_FuelQuality = iNew.Quality();

		// Increase quantity.
		m_fFuelQuantity = nuTotal;
		if(m_fFuelQuantity > FuelCapacity())
			m_fFuelQuantity = FuelCapacity();
		m_VesselLock.unlock();
	}
	else if(iNew.CommodityId() == CID_Electricity)
	{
		// Increase quantity.
		float capMax = CapacitorMax();
		m_VesselLock.lockForWrite();
		m_CapacitorLevel += iNew.Count();
		if(m_CapacitorLevel > capMax)
			m_CapacitorLevel = capMax;
		m_VesselLock.unlock();
	}
	else
	{
		// Add the cargo.
		m_CargoLock.lockForWrite();
		m_Cargo.AddItem(iNew);
		m_tmCargoChange.Set();
		m_CargoLock.unlock();
	}
	IncreaseMonster(1, MONSTER_Cargo);
	SetModified();	// Trigger update due to inventory change.
}

bool DaVessel::RemoveFuel(COMMODITYID fuelCID)
{
#if !defined(QT_NO_DEBUG)
	if(fuelCID == CID_Invalid)
		qDebug() << "DaVessel::RemoveFuel fuelCID INVALID" << Id() << Name();
#endif
	bool hadTheFuel = false;
	if(fuelCID == CID_Hydrogen)
	{
		m_VesselLock.lockForWrite();
		hadTheFuel = (m_fFuelQuantity >= 1)?true:false;
		m_fFuelQuantity--;
		if(m_fFuelQuantity < 0)
			m_fFuelQuantity = 0;
		m_VesselLock.unlock();
		SetModified();	// Trigger update due to inventory change.
	}
	else if(fuelCID == CID_Electricity)
	{
		hadTheFuel = (CapacitorLevel() >= 1)?true:false;
		DischargeCapacitor(1);
		SetModified();	// Trigger update due to inventory change.
	}
	else
	{
		// Look in my cargo hold for the fuel commodity.
		m_CargoLock.lockForWrite();
		AuItemHash::iterator it = m_Cargo.FindBest(fuelCID);
		if(it != m_Cargo.end())
		{
			// The commodity was found in my hold.
			m_Cargo.TakeItem(*(*it).second, 1);
			hadTheFuel = true;
			m_tmCargoChange.Set();
			SetModified();	// Trigger update due to inventory change.
		}
		m_CargoLock.unlock();
	}
	return hadTheFuel;
}

bool DaVessel::RemoveCargo(const AuItem& iTake, quint32 howMany)
{
	m_CargoLock.lockForWrite();
	bool v = RemoveCargoNoLock(iTake, howMany);
	m_CargoLock.unlock();
	return v;
}

bool DaVessel::RemoveCargoNoLock(const AuItem& iTake, quint32 howMany)
{
	bool hadTheCargo = false;
	if(iTake.CommodityId() == CID_Hydrogen)
	{
		m_VesselLock.lockForWrite();
		hadTheCargo = (m_fFuelQuantity >= howMany)?true:false;
		m_fFuelQuantity -= howMany;
		if(m_fFuelQuantity < 0)
			m_fFuelQuantity = 0;
		m_VesselLock.unlock();
		SetModified();	// Trigger update due to inventory change.
	}
	else if(iTake.CommodityId() == CID_Electricity)
	{
		hadTheCargo = (CapacitorLevel() >= howMany)?true:false;
		DischargeCapacitor(howMany);
		SetModified();	// Trigger update due to inventory change.
	}
	else if(m_Cargo.TakeItem(iTake, howMany))
	{
		hadTheCargo = true;
		m_tmCargoChange.Set();
		SetModified();	// Trigger update due to inventory change.
	}
	return hadTheCargo;
}

void DaVessel::AddWaypoint(const DeWaypoint* wp)
{
	if(wp)
	{
		m_WaypointLock.lockForWrite();
		m_aWaypoints.AddDestination(*wp);
		m_tmWaypointChange.Set();
		m_WaypointLock.unlock();
		SetModified();
	}
}

void DaVessel::AddWaypoint(const AuId& idDestSolarSystem, const AuId& idDestAtom)
{
	m_WaypointLock.lockForWrite();
	m_aWaypoints.AddDestination(idDestAtom, idDestSolarSystem);
	m_tmWaypointChange.Set();
	m_WaypointLock.unlock();
	SetModified();
}

void DaVessel::AddWaypoint(const AuId& idDestSolarSystem, const AuId& idDestAtom, const AuPoint& vFromDestAtom)
{
	m_WaypointLock.lockForWrite();
	m_aWaypoints.AddDestination(idDestAtom, vFromDestAtom, idDestSolarSystem);
	m_tmWaypointChange.Set();
	m_WaypointLock.unlock();
	SetModified();
}

const DeWaypoint* DaVessel::LockWaypoint(int sequence) const
{
	m_WaypointLock.lockForRead();
	if(sequence >= 0 && sequence < m_aWaypoints.count())
		return &m_aWaypoints[sequence];
	else
		return 0;
}

void DaVessel::InsertWaypointAt(int sequence, const AuId& idDestSolarSystem,
		const AuId& idDestAtom)
{
	m_WaypointLock.lockForWrite();
	if(sequence >= 0 && sequence < m_aWaypoints.count())
	{
		DeWaypoint wp(idDestAtom, idDestSolarSystem);
		DeWaypointRoute::iterator it = m_aWaypoints.begin() + sequence;
		m_aWaypoints.insert(it, wp);
		m_tmWaypointChange.Set();
		SetModified();
	}
	m_WaypointLock.unlock();
}

void DaVessel::InsertWaypointAt(int sequence, const AuId& idDestSolarSystem, const AuId& idDestAtom,
		const AuPoint& vFromDestAtom)
{
	m_WaypointLock.lockForWrite();
	if(sequence >= 0 && sequence < m_aWaypoints.count())
	{
		DeWaypoint wp(idDestAtom, idDestSolarSystem, vFromDestAtom);
		DeWaypointRoute::iterator it = m_aWaypoints.begin() + sequence;
		m_aWaypoints.insert(it, wp);
		m_tmWaypointChange.Set();
		SetModified();
	}
	m_WaypointLock.unlock();
}

void DaVessel::RemoveWaypoint(int sequence)
{
	m_WaypointLock.lockForWrite();
	if(sequence >= 0 && sequence < m_aWaypoints.count())
	{
		m_aWaypoints.erase(m_aWaypoints.begin() + sequence);
		m_tmWaypointChange.Set();
		SetModified();
	}
	m_WaypointLock.unlock();
}

void DaVessel::RemoveAllWaypoints()
{
	m_WaypointLock.lockForWrite();
	m_aWaypoints.clear();
	m_tmWaypointChange.Set();
	m_WaypointLock.unlock();
	SetModified();
}

void DaVessel::PerformSensorConsoleScan(AuSensorBlipList& sensorBlips) const
{
	// Clear previous sensor scan results.
	sensorBlips.Clear();

	// Scanners don't detect things inside the photon sphere.
	if(InPhotonSphere())
		return;

	// Debug switches.
	switch(Structure())
	{
	case SC_Count:
	case SC_Deprecated:
		return;
	case SC_Hull:
	case SC_Starship:
	case SC_Spaceship:
		if(!g_Au.m_EnableSensorScanByShip)
			return;
		break;
	case SC_Station:
		if(!g_Au.m_EnableSensorScanByStation)
			return;
		break;
	}

	// Sensor scan is only performed if someone (person or citizen ai) is sitting at an appropriate console.
	// Sensor scan is performed even if sensors are not working because the AI troops need the information to fire turrets.
	// Sensor scan content varies based on what is needed.
	int scanContent = 0x0001;	// Bitmask of the scan content needed.
								//  b0 = sensor console blips
								//  b1 = fire control console blips
								//  b2 = transporter console blips
								//  b3 = locked-on blip
								//  b4 = AI pilot obstacle blips
								//  b5 = AI turret gunner target blips
								//   b6 = Scanning spacecraft wants its own subatoms to be scanned.

	//qDebug("DaVessel::Performing Sensor Scan");
	DaAtom* envAtom = FindEnvironmentAtom();
	if(!envAtom)
		return;

	AuId scannerWorldId;
	int scannerDomain = GunnerDomain(scannerWorldId);

	// Scan the solar system for anything that might show up on sensors.
	// This data is also used for selecting targets by troops manning battle stations.
	// Transform sensor blips so they are relative to me, not the environment.
	unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash> stanceCache;
	AuId lb(LockedBlip());
	if(!lb.IsNull())
		envAtom->SensorScan(sensorBlips, EnvLocation(), Id(), EmpireId(), scannerDomain, scannerWorldId, SensorRange(),
				MaxBayWeaponRange(), scanContent, lb, stanceCache);
	else
		envAtom->SensorScan(sensorBlips, EnvLocation(), Id(), EmpireId(), scannerDomain, scannerWorldId, SensorRange(),
				MaxBayWeaponRange(), scanContent, AuId(0), stanceCache);
}

void DaVessel::PerformFireControlConsoleScan(AuSensorBlipList& sensorBlips) const
{
	// Clear previous sensor scan results.
	sensorBlips.Clear();

	// Scanners don't detect things inside the photon sphere.
	if(InPhotonSphere())
		return;

	// Debug switches.
	switch(Structure())
	{
	case SC_Count:
	case SC_Deprecated:
		return;
	case SC_Hull:
	case SC_Starship:
	case SC_Spaceship:
		if(!g_Au.m_EnableSensorScanByShip)
			return;
		break;
	case SC_Station:
		if(!g_Au.m_EnableSensorScanByStation)
			return;
		break;
	}

	// Sensor scan is only performed if someone (person or citizen ai) is sitting at an appropriate console.
	// Sensor scan is performed even if sensors are not working because the AI troops need the information to fire turrets.
	// Sensor scan content varies based on what is needed.
	int scanContent = 0x0002;	// Bitmask of the scan content needed.
								//  b0 = sensor console blips
								//  b1 = fire control console blips
								//  b2 = transporter console blips
								//  b3 = locked-on blip
								//  b4 = AI pilot obstacle blips
								//  b5 = AI turret gunner target blips
								//   b6 = Scanning spacecraft wants its own subatoms to be scanned.

	//qDebug("DaVessel::Performing Sensor Scan");
	DaAtom* envAtom = FindEnvironmentAtom();
	if(!envAtom)
		return;

	AuId scannerWorldId;
	int scannerDomain = GunnerDomain(scannerWorldId);

	// Scan the solar system for anything that might show up on sensors.
	// This data is also used for selecting targets by troops manning battle stations.
	// Transform sensor blips so they are relative to me, not the environment.
	unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash> stanceCache;
	AuId lb(LockedBlip());
	if(!lb.IsNull())
		envAtom->SensorScan(sensorBlips, EnvLocation(), Id(), EmpireId(), scannerDomain, scannerWorldId, SensorRange(),
				MaxBayWeaponRange(), scanContent, lb, stanceCache);
	else
		envAtom->SensorScan(sensorBlips, EnvLocation(), Id(), EmpireId(), scannerDomain, scannerWorldId, SensorRange(),
				MaxBayWeaponRange(), scanContent, AuId(0), stanceCache);
}

void DaVessel::PerformTransporterConsoleScan(AuSensorBlipList& sensorBlips, bool showAboard) const
{
	// Clear previous sensor scan results.
	sensorBlips.Clear();

	// Scanners don't detect things inside the photon sphere.
	if(InPhotonSphere())
		return;

	// Debug switches.
	switch(Structure())
	{
	case SC_Count:
	case SC_Deprecated:
		return;
	case SC_Hull:
	case SC_Starship:
	case SC_Spaceship:
		if(!g_Au.m_EnableSensorScanByShip)
			return;
		break;
	case SC_Station:
		if(!g_Au.m_EnableSensorScanByStation)
			return;
		break;
	}

	// Sensor scan is only performed if someone (person or citizen ai) is sitting at an appropriate console.
	// Sensor scan is performed even if sensors are not working because the AI troops need the information to fire turrets.
	// Sensor scan content varies based on what is needed.
	int scanContent = 0x0004;	// Bitmask of the scan content needed.
								//  b0 = sensor console blips
								//  b1 = fire control console blips
								//  b2 = transporter console blips
								//  b3 = locked-on blip
								//  b4 = AI pilot obstacle blips
								//  b5 = AI turret gunner target blips
								//   b6 = Scanning spacecraft wants its own subatoms to be scanned.
	if(showAboard)
		scanContent |= 0x0040;

	//qDebug("DaVessel::Performing Sensor Scan");
	DaAtom* envAtom = FindEnvironmentAtom();
	if(!envAtom)
		return;

	AuId scannerWorldId;
	int scannerDomain = GunnerDomain(scannerWorldId);

	// Scan the solar system for anything that might show up on sensors.
	// This data is also used for selecting targets by troops manning battle stations.
	// Transform sensor blips so they are relative to me, not the environment.
	unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash> stanceCache;
	AuId lb(LockedBlip());
	if(!lb.IsNull())
		envAtom->SensorScan(sensorBlips, EnvLocation(), Id(), EmpireId(), scannerDomain, scannerWorldId, SensorRange(),
				MaxBayWeaponRange(), scanContent, lb, stanceCache);
	else
		envAtom->SensorScan(sensorBlips, EnvLocation(), Id(), EmpireId(), scannerDomain, scannerWorldId, SensorRange(),
				MaxBayWeaponRange(), scanContent, AuId(0), stanceCache);
}

quint8 DaVessel::FuelPercent() const
{
	float fuelCap = FuelCapacity();
	if(fuelCap <= 0)
		return 0;
	quint8 fper = (quint8)ceil(FuelQuantity() * 100.f / fuelCap);
	return fper;
}

char DaVessel::CapacitorPercent() const
{
#if !defined(QT_NO_DEBUG)
	if(!IsDesignDataCacheValid())
	{
		qDebug("DaVessel::CapacitorPercent - ship %d %s. Design id %d is not valid yet", Id().GetID(),
			(const char*)Name().toLatin1(), DesignId());
		return 100;	// Fake response until design is valid. Must not be loaded into local data yet.
	}
#endif
	float capMax = CapacitorMax();
	if(capMax <= 0)
		return 0;
	int fper = (char)ceil(CapacitorLevel() * 100.f / capMax);
	return fper;
}

void DaVessel::TraversedWormhole(const DaSolarSystem* system, const AuWormhole& wh)
{
	//qDebug("DaVessel::TraversedWormhole %d %s", Id().GetID(), (const char*)Name().toAscii());
	if(!IsInSpacecraft())
	{
		// Insure FTL drive discharges.
		DischargeFTLDrive(FTLWormholeDrive());

		// Kill throttles.
		SetThrottle(0);
		SetThrustLateral(0);
		SetThrustLongitudinal(0);
		SetThrustVertical(0);

		// Automatically park the ship when going through an intergalactic wormhole.
		if(wh.IsIntergalactic())
			SetGravityDriveState(GD_Hold);
	}

	// If the first waypoint is the wormhole, it can be discarded.
	AuPoint loc = wh.Location(system);
	const DeWaypoint* pw = LockWaypoint(0);
	bool v = pw && pw->DestinationOffset().FuzzyEquals(loc);
	UnlockWaypoint();
	if(v)
		RemoveWaypoint(0);

	// Let the subatoms know so the nav console can update, etc.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaVessel* pAtom = dynamic_cast<DaVessel*>(*it);
		if(!pAtom)
			continue;
		// This is recursive to get spacecraft aboard this one.
		pAtom->TraversedWormhole(system, wh);
	}
}

// Disconnects atoms that are not "inside" this ship and reparents them to my parent, with my movement vector.
void DaVessel::DetachRiders()
{
	if(!HasSubatoms())
		return;

	// Locate subordinate atoms that are not inside the ship.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	DaAtomList riders;
	for(DaAtomList::const_iterator ai = subs.cbegin(); ai != subs.cend(); ai++)
	{
		if(!(*ai)->DoesQueryEnvironment())
			continue;											// Design studio instance?
		if((*ai)->IsInsideSpacecraft())
			continue;											// Skip atoms that are inside the spacecraft.
		if((*ai)->IsClimbingLadder() || (*ai)->IsStaticDisplay())
			continue;											// Skip atoms clinging to ladders and static displayed items.

		// The atom is not inside the spacecraft or hanging onto a ladder. It must be disconnected and allowed to drift free.
		riders.append(*ai);
	}

	// Disconnect subordinate atoms that are not inside the ship.
	for(DaAtomList::const_iterator ri = riders.begin(); ri != riders.end(); ri++)
	{
		(*ri)->ForcePosMove(AuAtomPos(ParentId(), Transform() * (*ri)->Transform()), (*ri)->Transform().UnProjectVector(Vector()));
		(*ri)->InvalidateEnvironment();
		(*ri)->SetModified();
	}
}

void DaVessel::AnswerHail(const QString& channelName, AuCommStdChannel::RANGE range, AuCommStdChannel::SCOPE scope,
		bool contactedByCommand, const DaSentient* officer)
{
	const DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return;	// Not likely.
	const DaSector* sector = system->FindParentSector();
	if(!sector)
		return;	// Not likely.
	const DaGalaxy* galaxy = sector->FindParentGalaxy();
	if(!galaxy)
		return;	// Not likely.

	// Compose reply message text.
	// Identify what I am.
	QString text;
	if(IsDormant())
	{
		// Mooring distinguishes between hangared ship vs ship in stasis.
		if(!IsMoored())
		{
			// Ship in stasis.
			if(StasisReadyTime() < AuTime::Now())
				text = tr("%1 here. This is an automated response. "
						"The ship is in stasis until %2. "
						"Then it can be recovered by an airport repair shop.")
					.arg(NameWithEmpire())
					.arg(StasisReadyTime().UTC().toString());
			else
				text = tr("%1 here. This is an automated response. "
						"The ship is in stasis, ready to be recovered by an airport repair shop. ")
					.arg(NameWithEmpire());
		}
		else
		{
			// Ship in a hangar.
			const DaWorldLL* world = FindParentWorldInEnv();
			if(!world)
				return;	// wut?
			AuGlobeLLF ll;
			if(!world->LLOfLoc(ll, Location()))
				return;	// wut?
			text = tr("%1 here. This is an automated response. "
					"The ship is in a hangar at %2 on %3, in the %4 system, %5 sector, %6. ")
				.arg(NameWithEmpire())
				.arg(ll.ToString())
				.arg(world->Name())
				.arg(galaxy->Name());
		}
	}
	else
	{
		if(officer)
			text = tr("%1 aboard %2 here.").arg(officer->Name()).arg(NameWithEmpire());
		else
			text = tr("%1 here. This is an automated response as no officer is aboard.").arg(NameWithEmpire());

		// Say where I am.
		text = text + " " + CommMessageLocation();

		// Add repair estimate onto hail reply.
		if(scope == AuCommStdChannel::CrewMates || scope == AuCommStdChannel::Fleet)
		{
			if(RepairSecondsRemaining())
				text += tr(" Repairing damage, %1 seconds until finished.").arg(RepairSecondsRemaining());
			if(!LifeSupportWorking())
				text += tr(" Life support is off line so people cannot recall to berths.");
		}

		if(contactedByCommand)
		{
			if(officer)
			{
				switch(officer->Posture())
				{
				case POSTURE_Offensive:
					text += tr(" Our posture is offensive; we are watching for threats.");
					break;
				case POSTURE_DefendSelf:
					text += tr(" Our posture is self defense; we are holding fire unless attacked.");
					break;
				case POSTURE_DefendArea:
					text += tr(" Our posture is defensive; we are defending our area.");
					break;
				case POSTURE_Berserk:
					text += tr(" Our posture is berserk; we want to kill.");
					break;
				case POSTURE_HoldFire:
					text += tr(" Our posture is hold fire; we are defenseless.");
					break;
				}
			}

			// This is a response to contact from a commander.
			text += tr(" Hello commander.");
		}
	}

	// HailReply message type requires sender solar system id.
	AuCommHailReply repMsg(text, system->Id(), channelName, Id(), Name(), sector->Id(), Galaxy(), range, scope);
	if(repMsg.Scope() == AuCommStdChannel::CrewMates)
		repMsg.SetScopeData(Id().GetID());			// CrewMate scope needs ship id.
	else if(repMsg.Scope() == AuCommStdChannel::Company)
		repMsg.SetScopeData(OwnerId().GetID());
	else
		repMsg.SetScopeData(EmpireId());			// All other logical scopes need empire id.
	g_Au.SendAtomChat(repMsg);

	if(!IsDormant())
	{
		if(contactedByCommand && officer && !officer->DecideWhetherToTravel(this))
		{
			text += tr("Sir, we are holding our position. "
					"Mission orders take us into unexplored space. "
					"To go there, an avatar on line must be aboard.");
			AuCommHailReply repMsg(text, system->Id(), channelName, Id(), Name(), sector->Id(), Galaxy(), range,
					scope);
			if(repMsg.Scope() == AuCommStdChannel::CrewMates)
				repMsg.SetScopeData(Id().GetID());			// CrewMate scope needs ship id.
			else if(repMsg.Scope() == AuCommStdChannel::Company)
				repMsg.SetScopeData(OwnerId().GetID());
			else
				repMsg.SetScopeData(EmpireId());			// All other logical scopes need empire id.
			g_Au.SendAtomChat(repMsg);
		}
	}
}

void DaVessel::AnswerContactShip(AuCommStdChannel::RANGE range, bool contactedByCommand, const DaSentient* officer)
{
	const DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return;	// Not likely.
	const DaSector* sector = system->FindParentSector();
	if(!sector)
		return;	// Not likely.
	const DaGalaxy* galaxy = sector->FindParentGalaxy();
	if(!galaxy)
		return;	// Not likely.

	// Compose reply message text.
	// Identify what I am.
	QString text;
	if(IsDormant())
	{
		// Mooring distinguishes between hangared ship vs ship in stasis.
		if(!IsMoored())
		{
			// Ship in stasis.
			if(StasisReadyTime() < AuTime::Now())
				text = tr("%1 here. This is an automated message. "
						"The ship is in stasis until %2. "
						"Then it can be recovered by an airport repair shop.")
					.arg(NameWithEmpire())
					.arg(StasisReadyTime().UTC().toString());
			else
				text = tr("%1 here. This is an automated message. "
						"The ship is in stasis, ready to be recovered by an airport repair shop. ")
					.arg(NameWithEmpire());
		}
		else
		{
			// Ship in a hangar.
			const DaWorldLL* world = FindParentWorldInEnv();
			if(!world)
				return;	// wut?
			AuGlobeLLF ll;
			if(!world->LLOfLoc(ll, Location()))
				return;	// wut?
			text = tr("%1 here. This is an automated message. "
					"The ship is in a hangar at %2 on %3, in the %4 system, %5 sector, %6. ")
				.arg(NameWithEmpire())
				.arg(ll.ToString())
				.arg(world->Name())
				.arg(galaxy->Name());
		}
	}
	else
	{
		if(officer)
			text = tr("%1 aboard %2 here.").arg(officer->Name()).arg(NameWithEmpire());
		else
			text = tr("%1 here. This is an automated message as no officer is aboard.").arg(NameWithEmpire());

		// Say where I am.
		text = text + " " + CommMessageLocation();

		// Add repair estimate onto hail reply.
		if(RepairSecondsRemaining())
			text += tr(" Repairing damage, %1 seconds until finished.").arg(RepairSecondsRemaining());
		if(!LifeSupportWorking())
			text += tr(" Life support is off line so people cannot recall to berths.");

		if(contactedByCommand)
		{
			if(officer)
			{
				switch(officer->Posture())
				{
				case POSTURE_Offensive:
					text += tr(" Our posture is offensive; we are watching for threats.");
					break;
				case POSTURE_DefendSelf:
					text += tr(" Our posture is self defense; we are holding fire unless attacked.");
					break;
				case POSTURE_DefendArea:
					text += tr(" Our posture is defensive; we are defending our area.");
					break;
				case POSTURE_Berserk:
					text += tr(" Our posture is berserk; we want to kill.");
					break;
				case POSTURE_HoldFire:
					text += tr(" Our posture is hold fire; we are defenseless.");
					break;
				}
			}

			// This is a response to contact from a commander.
			text += tr(" Welcome back commander.");
		}
	}

	// ContactReply message type requires sender solar system id.
	AuCommContactReply repMsg(text, system->Id(), Id(), Name(), sector->Id(), Galaxy(), range);
	g_Au.SendAtomChat(repMsg);

	if(!IsDormant())
	{
		if(contactedByCommand && officer && !officer->DecideWhetherToTravel(this))
		{
			text += tr("Sir, we are holding our position. "
					"Mission orders take us into unexplored space. "
					"To go there, an avatar on line must be aboard.");
			AuCommContactReply repMsg(text, system->Id(), Id(), Name(), sector->Id(), Galaxy(), range);
			g_Au.SendAtomChat(repMsg);
		}
	}
}

void DaVessel::MonitorCommChannels()
{
	m_RosterLock.lockForRead();
	bool hasct = m_Roster.HasAnyCrewOrTroops();
	bool haso = m_Roster.HasAIOfficer() || m_Roster.HasHumanOfficer();
	m_RosterLock.unlock();

	//if(Name() == "Opsecy")
		//qDebug() << "DaVessel::MonitorCommChannels" << Id() << Name() << "crewOrTroops" << hasct << "officer" << haso
			//<< "priv" << IsPrivatelyOwned() << OwnerId() << "station" << IsSpaceStation();

	// Ships with crew monitor Crew channel.
	if(hasct)
		g_Au.MonitorChannel(AuCommStdChannel::CrewChannelName(), Id());

	if(haso)
	{
		// AI officers monitor the Fleet, Hail and ship name channels.
		g_Au.MonitorChannel(AuCommStdChannel::FleetChannelName(), Id());
		g_Au.MonitorChannel(AuCommStdChannel::HailChannelName(), Id());
		g_Au.MonitorChannel(Name(), Id());
	}
	else if(IsSpaceStation())
	{
		// All stations monitor the Hail and ship name channels.
		g_Au.MonitorChannel(AuCommStdChannel::HailChannelName(), Id());
		g_Au.MonitorChannel(Name(), Id());
	}

	// Privately owned ships monitor the Company and ship name channels.
	if(IsPrivatelyOwned() && !OwnerId().IsNull())
	{
		g_Au.MonitorChannel(Name(), Id());
		g_Au.MonitorChannel(AuCommStdChannel::CompanyChannelName(), Id());
	}
}

void DaVessel::CancelWeaponBayFired(quint16 which)
{
	bool canceled = false;
	m_BayLock.lockForWrite();
	unordered_map<quint16, AuBayFired>::iterator lfi = m_BayFired.find(which);
	if(lfi != m_BayFired.end())
	{
		m_BayFired.erase(lfi);
		m_tmBayFired.Set();
		canceled = true;
	}
	m_BayLock.unlock();

	if(canceled)
		SetModified();
}

// Returns true when 'which' bay is no longer attached.
// Calls CancelWeaponBayFired when no longer active.
bool DaVessel::DecayTractorBay(quint16 which)
{
	m_BayLock.lockForWrite();

	unordered_map<quint16, AuBayFired>::iterator lfi = m_BayFired.find(which);
	if(lfi == m_BayFired.end() || !(*lfi).second.IsTractorAttached())
	{
		// Which bay not firing or not an attached tractor.
		m_BayLock.unlock();
		return true;
	}

	// Reduce tractor attachment strength.
	(*lfi).second.m_TractorState--;

	// When strength reaches 1 it is no longer attached.
	bool canceled = false;
	if(!(*lfi).second.IsTractorAttached())
	{
		m_BayFired.erase(lfi);
		m_tmBayFired.Set();
		canceled = true;
	}

	m_BayLock.unlock();

	if(canceled)
		SetModified();

	return canceled;
}

// Returns the distance to the ship in orbit that is closest to loc.
// Also checks for other atoms that block a ship from spawning at 'loc'.
double DaVessel::ClosestShipInSystem(const DaAtomList& ships, const AuPoint& loc, const AuId& shipToIgnore) const
{
	double bestDist = FeetPerParsec();
	for(DaAtomList::const_iterator it = ships.cbegin(); it != ships.cend(); it++)
	{
		const DaAtom* atom = *it;
		if(!atom->IsNormal())
			continue;
		if(atom->Is(shipToIgnore))
			continue;

		// Calculate atom's location relative to me.
		AuPoint atomLoc = EnvTransform().UnProject(atom->EnvLocation());

		double dist;
		const DaVessel* ship = dynamic_cast<const DaVessel*>(atom);
		if(ship)
		{
			// Calculate distance to ship, considering its rough radius.
			dist = loc.Distance(atomLoc) - (ship->RoughRadius() + 10*FeetPerMeter());
		}
		else
		{
			// The non-spaceship atoms that we're concerned about are all less than 100' in radius.
			dist = loc.Distance(atomLoc) - 100;
		}
		if(dist < bestDist)
			bestDist = dist;
	}
	if(bestDist < 0)
		bestDist = 0;
	return bestDist;
}

// Returns the number of hits of damage to a system.
unsigned int DaVessel::HitsDamageSystem(SYSTEM system) const
{
	m_DamageLock.lockForRead();
	unsigned int v = m_Damage.HitsDamageAddParts(system);
	m_DamageLock.unlock();
	return v;
}

const DaBd* DaVessel::MissionDestGoToCity(const AuMissionOrder& order) const
{
	if(order.DestinationId().IsNull())
		return 0;

	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return 0;

	return dynamic_cast<const DaBd*>(system->FindIdRecursive(order.DestinationId()));
}

const DaVessel* DaVessel::MissionDestGoToStation(const AuMissionOrder& order) const
{
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return 0;

	DaVessel* destAtom = 0;
	if(!order.DestinationId().IsNull())
		destAtom = dynamic_cast<DaVessel*>(system->FindIdRecursive(order.DestinationId()));
	if(!destAtom)
	{
		// No station was specified or the specified station was not found.
		// Look for the specified station by name. When a station gets upgraded, the name is carried to its replacement.
		if(!order.DestinationName().isEmpty())
		{
			// We have a name of a station to look for.
			DaAtomList ships;
			system->FindVesselsRecursive(ships);
			for(DaAtomList::const_iterator si = ships.begin(); si != ships.end(); si++)
			{
				DaVessel* systemShip = dynamic_cast<DaVessel*>(*si);
				if(!systemShip)
					continue;
				if(!systemShip->IsSpaceStation())
					continue;				// We only look for stations.
				if(systemShip->Name() != order.DestinationName())
					continue;

				// Found the station with the specified name.
				return systemShip;
			}
		}
	}
	return destAtom;
}

const DaVessel* DaVessel::MissionDestGoToBestStation() const
{
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return 0;

	// Look for a suitable station.
	DaVessel* bestEmpire = 0;
	DaVessel* bestFriend = 0;
	DaVessel* bestNeutral = 0;
	DaAtomList ships;
	system->FindVesselsRecursive(ships);
	for(DaAtomList::const_iterator si = ships.begin(); si != ships.end(); si++)
	{
		DaVessel* systemShip = dynamic_cast<DaVessel*>(*si);
		if(!systemShip)
			continue;
		if(!systemShip->IsSpaceStation())
			continue;				// We only look for stations.
		DaVessel** bestToCompare = 0;
		switch(PoliticalStance(EmpireId(), systemShip->EmpireId()))
		{
		case POL_Empire:
		case POL_Vassal:
			bestToCompare = &bestEmpire;
			break;
		case POL_Friend:
			bestToCompare = &bestFriend;
			break;
		case POL_Neutral:
			bestToCompare = &bestNeutral;
			break;
		case POL_Enemy:
			continue;				// Skip enemy stations.
		}

		if(!(*bestToCompare))
			*bestToCompare = systemShip;	// This is the first station found in the political stance category.
		else
		{
			// Compare to the previous one in the political stance category that we found.
			DaVessel* previousShip = *bestToCompare;
			if(systemShip->QualityOfDesign() > previousShip->QualityOfDesign())
				*bestToCompare = systemShip;	// Found station with higher tech design.
			else if(systemShip->QualityOfDesign() == previousShip->QualityOfDesign() &&
					systemShip->LowestSystemQuality() > previousShip->LowestSystemQuality())
				*bestToCompare = systemShip;	// Found station with same design Q but higher Q construction.
		}
	}

	// Select the best station we found, prefering our own over our friends.
	DaVessel* destAtom;
	if(bestEmpire)
		destAtom = bestEmpire;
	else if(bestFriend)
		destAtom = bestFriend;
	else if(bestNeutral)
		destAtom = bestNeutral;
	else
		destAtom = 0;
	return destAtom;
}

const DaSolarSystem* DaVessel::MissionDestGoToSystem(const AuMissionOrder& order) const
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(FindEnvironmentAtom());
	if(!system)
		return 0;

	if(system->Is(order.DestinationId()))
		return system;	// The ship is in the destination system.

	// The ship is not in the destination system.
	const DaSolarSystem* destSys = dynamic_cast<const DaSolarSystem*>(ether->FindNormalAtom(order.DestinationId()));
	if(!destSys)
		return 0;

	// If the system is not in a neighboring sector, return 0 so the ship will go directly to the coordinate instead.
	AuPoint secVec = system->Parent()->Location() - destSys->Parent()->Location();
	if(fabs(secVec.x) > 10 || fabs(secVec.y) > 10 || fabs(secVec.z) > 10)
		return 0;
	return destSys;
}

const DaGalaxy* DaVessel::MissionDestGoToGalaxy(const AuMissionOrder& order) const
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	GALAXY destGalaxy = order.DestinationId().Galaxy();
	if(destGalaxy == GALAXY_Count)
		return 0;

	const DaGalaxy* destGal = dynamic_cast<const DaGalaxy*>(ether->FindNormalAtom(order.DestinationId()));
	return destGal;
}

const DaBd* DaVessel::MissionDestPatrolCities() const
{
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return 0;

	// Make a list of the bds that we will patrol.
	DaAtomList cities;
	system->FindCitiesBdRecursive(cities);
	DaAtomList::iterator ci = cities.begin();
	while(ci != cities.end())
	{
		DaBd* systemBd = dynamic_cast<DaBd*>(*ci);
		if(systemBd)
		{
			// We only patrol empire capitols and HQs with airports.
			if(!systemBd || !systemBd->HasAirportAccess() || systemBd->EmpireId() != EmpireId())
				ci = cities.erase(ci);	// Remove city that we won't be patrolling.
			else
				ci++;					// Skip over cities that we will be patrolling.
			continue;
		}

		ci = cities.erase(ci);	// Remove whatever else is in the list. There should be nothing. We should never get here.
	}
	if(cities.isEmpty())
		return 0;	// No cities to patrol.

	if(HasWaypoint())
	{
		// The ship already has a course set. Does it take me to any of the cities' planets?
		const DeWaypoint* wp = LockWaypoint(WaypointCount()-1);	// Final waypoint.
		for(DaAtomList::const_iterator ci = cities.begin(); ci != cities.end(); ci++)
		{
			const DaWorld* planet = (*ci)->FindParentPlanetInEnv();
			if(planet && planet->Is(wp->Destination()))
			{
				UnlockWaypoint();
				return (DaBd*)*ci;			// The final waypoint takes me to the city's planet.
			}
		}
		UnlockWaypoint();
	}

	// We aren't currently enroute to any of our patrol cities.
	// Look for the next one after the last destination atom we arrived at.
	AuId arid(m_Mission.LastArrivalId());
	if(!arid.IsNull())
	{
		// Ship has a previous arrival atom. If it is in my list, I'll go to the next city's planet.
		for(DaAtomList::const_iterator ci = cities.begin(); ci != cities.end(); ci++)
		{
			if((*ci)->Is(arid))
			{
				// Found the city that the ship last arrived at.
				// Set course for the next city in the list.
				ci++;
				if(ci != cities.end())
					return (DaBd*)*ci;
				break;
			}
		}
	}

	// Didn't find the next city.
	return (DaBd*)cities.first();	// Go to the first city in the list.
}

const DaVessel* DaVessel::MissionDestPatrolStations() const
{
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return 0;

	// Make a list of the stations that we will patrol.
	DaAtomList ships;
	system->FindVesselsRecursive(ships);
	DaAtomList::iterator ci = ships.begin();
	while(ci != ships.end())
	{
		DaVessel* systemShip = dynamic_cast<DaVessel*>(*ci);
		// We only patrol empire stations.
		if(!systemShip || !systemShip->IsSpaceStation() || systemShip->EmpireId() != EmpireId())
			ci = ships.erase(ci);	// Remove ships that we won't be patrolling.
		else
			ci++;					// Skip over ships that we will be patrolling.
	}
	if(ships.isEmpty())
		return 0;	// No ships to patrol.

	// Am I already following a route to one of those stations' planets?
	if(HasWaypoint())
	{
		// The ship already has a course set. Does it take me to any of the stations' planets?
		const DeWaypoint* wp = LockWaypoint(WaypointCount()-1);	// Final waypoint.
		for(DaAtomList::const_iterator ci = ships.begin(); ci != ships.end(); ci++)
		{
			DaVessel* systemShip = dynamic_cast<DaVessel*>(*ci);
			DaWorld* planet = systemShip->FindParentPlanetInEnv();
			if(planet)
			{
				if(planet->Is(wp->Destination()))
				{
					UnlockWaypoint();
					return systemShip;	// The final waypoint takes me to the station's planet.
				}
			}
			else
			{
				// Station is not at a planet.
				if(systemShip->Is(wp->Destination()))
				{
					UnlockWaypoint();
					return systemShip;	// The final waypoint takes me to the station.
				}
			}
		}
		UnlockWaypoint();
	}

	// We aren't currently enroute to any of our stations' planets.
	// Look for the next one after the last destination atom we arrived at.
	AuId arid(m_Mission.LastArrivalId());
	if(!arid.IsNull())
	{
		// Ship has a previous arrival atom. If it is in my list, I'll go to the next station.
		for(DaAtomList::const_iterator ci = ships.begin(); ci != ships.end(); ci++)
		{
			DaVessel* systemShip = dynamic_cast<DaVessel*>(*ci);
			if(systemShip->Is(arid))
			{
				// Found the station that the ship last arrived at.
				// Set course for the next station in the list.
				ci++;
				if(ci != ships.end())
					return dynamic_cast<DaVessel*>(*ci);
				break;
			}
		}
	}

	// Didn't find the next station.
	return dynamic_cast<DaVessel*>(ships.first());	// Go to the first station in the list.
}

const DaWorld* DaVessel::MissionDestPatrolPlanets(AuPoint& destOffset) const
{
	const DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return 0;

	// Make a list of the planets that we will patrol.
	DaAtomList planets;
	system->FindPlanetsRecursive(planets);
	DaAtomList::iterator ci = planets.begin();
	while(ci != planets.end())
	{
		DaWorld* systemPlanet = dynamic_cast<DaWorld*>(*ci);
		// We only patrol planets, not satellites.
		if(!systemPlanet || dynamic_cast<const DaWorld*>(systemPlanet->Parent()))
			ci = planets.erase(ci);	// Remove planets that we won't be patrolling.
		else
			ci++;					// Skip over planets that we will be patrolling.
	}
	if(planets.isEmpty())
		return 0; // No planets to patrol.

	DaWorld* destPlanet = 0;

	// Am I already following a route to one of those planets?
	if(HasWaypoint())
	{
		// The ship already has a course set. Does it take me to any of the planets?
		const DeWaypoint* wp = LockWaypoint(WaypointCount()-1);	// Final waypoint.
		for(DaAtomList::const_iterator ci = planets.begin(); ci != planets.end(); ci++)
		{
			DaWorld* systemPlanet = dynamic_cast<DaWorld*>(*ci);
			if(systemPlanet->Is(wp->Destination()))
			{
				destPlanet = systemPlanet;						// The final waypoint takes me to the planet.
				break;
			}
		}
		UnlockWaypoint();
	}

	if(!destPlanet)
	{
		// Decide which planet to visit next.
		const DaWorld* myPlanet = FindParentPlanetInEnv();
		if(myPlanet)
		{
			// If the ship is near a planet in my list, I'll go to the next planet.
			for(DaAtomList::const_iterator ci = planets.begin(); ci != planets.end(); ci++)
			{
				DaWorld* systemPlanet = dynamic_cast<DaWorld*>(*ci);
				if(systemPlanet == myPlanet)
				{
					// Found the planet that the ship is currently near.
					// Set course for the next planet in the list.
					ci++;
					if(ci != planets.end())
						destPlanet = dynamic_cast<DaWorld*>(*ci);
					break;
				}
			}
		}
	}

	if(!destPlanet)
	{
		// Didn't find the next planet.
		destPlanet = dynamic_cast<DaWorld*>(planets.first());	// Go to the first planet in the list.
	}

	if(destPlanet)
	{
		// Calculate the closest orbit point to the ship.
		const AuMatrix4x4& destT = destPlanet->EnvTransform();
		destOffset = destT.UnProject(EnvLocation());					// Ship loc in planet coords.

		// Calculate a location in the orbit zone of the ringworld, directly above/below the destOffset location.
		double orbitAlt = destPlanet->AtmosphereRadius() + OrbitAltAboveAtm();

		if(dynamic_cast<const DaWorldRingSection*>(destPlanet))
			destOffset.y = 0;									// Flattened to equatorial plane of ringworld section.
		destOffset = destPlanet->FindOrbitSpawnLoc(destOffset, Id(), RoughRadius(), orbitAlt, false);
	}

	// Return whatever we got, could be 0.
	return destPlanet;
}

const DaSolarSystem* DaVessel::MissionDestPatrolWormholes(AuPoint& destOffset) const
{
	if(InArena())
		return 0;
	if(InDesignEnvironment())
		return 0;
	const DaSolarSystem* system = FindParentSolarSystem();
	if(!system || system->IsEmptySystem())
		return 0;
	const AuWormholeArray& wha = system->LockWormholes();
	if(wha.isEmpty())
	{
		system->UnlockWormholes();
		return 0;
	}

	// Am I already following a route to one of those wormholes?
	if(HasWaypoint())
	{
		// The ship already has a course set. Does it take me to any of the wormholes?
		const DeWaypoint* wp = LockWaypoint(WaypointCount()-1);	// Final waypoint.
		for(AuWormholeArray::const_iterator wi = wha.cbegin(); wi != wha.cend(); wi++)
		{
			if(system->Is(wp->m_idSystem))
			{
				AuPoint whSysLoc = (*wi).Location(system);
				if(whSysLoc.FuzzyEquals(wp->DestinationOffset()))
				{
					UnlockWaypoint();

					// The final waypoint takes me to the wormhole.
					system->UnlockWormholes();
					destOffset = whSysLoc;
					//qDebug() << "DaVessel::MissionDestPatrolWormholes" << Id() << Name()
						//<< "current dest is wormhole" << system->Id() << system->Name() << destOffset;
					return system;
				}
			}
		}
		UnlockWaypoint();
	}

	// Decide which wormhole to visit next.
	if(!IsInAtmosphere())
	{
		// Ship is in space. If it is near a wormhole in my list, I'll go to the next wormhole.
		AuPoint shipLoc = EnvLocation();
		for(AuWormholeArray::const_iterator wi = wha.cbegin(); wi != wha.cend(); wi++)
		{
			AuPoint whSysLoc = (*wi).Location(system);
			if(shipLoc.Distance2(whSysLoc) < (1000*FeetPerMeter() * 1000*FeetPerMeter()))
			{
				// Found the wormhole that the ship is currently near.
				// Set course for the next wormhole in the list.
				wi++;
				if(wi != wha.cend())
				{
					system->UnlockWormholes();
					destOffset = whSysLoc;
					//qDebug() << "DaVessel::MissionDestPatrolWormholes" << Id() << Name()
						//<< "near wormhole, next dest" << system->Id() << system->Name() << destOffset;
					return system;
				}
				break;
			}
		}
	}

	// Didn't find the next wormhole and we're not at the last wormhole in the list.
	destOffset = wha.first().Location(system);	// Go to the first wormhole in the list.
	system->UnlockWormholes();
	//qDebug() << "DaVessel::MissionDestPatrolWormholes" << Id() << Name()
		//<< "first wormhole is dest" << system->Id() << system->Name() << destOffset;
	return system;
}

const DaBd* DaVessel::MissionDestGoToClosestCity() const
{
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return 0;

	AuPoint shipLoc = EnvLocation();

	// Look for the closest suitable city.
	DaBd* bestEmpire = 0;
	DaBd* bestFriend = 0;
	DaBd* bestNeutral = 0;
	double distEmpire = 0;
	double distFriend = 0;
	double distNeutral = 0;
	DaAtomList bds;
	system->FindBdRecursive(bds);
	for(DaAtomList::const_iterator ci = bds.begin(); ci != bds.end(); ci++)
	{
		// We only look for airports.
		DaBd* systemBd = dynamic_cast<DaBd*>(*ci);
		if(!systemBd)
			continue;
		if(IsPrivatelyOwned())
		{
			if(systemBd->IndustryId() != IID_AirportTerminal)
				continue;
		}
		else
		{
			if(systemBd->IndustryId() != IID_AirportTerminal && systemBd->IndustryId() != IID_BaseAirTerminal)
				continue;
		}

		DaBd** bestToCompare = 0;
		double* bestDist = 0;
		switch(PoliticalStance(EmpireId(), (*ci)->EmpireId()))
		{
		case POL_Empire:
		case POL_Vassal:
			bestToCompare = &bestEmpire;
			bestDist = &distEmpire;
			break;
		case POL_Friend:
			bestToCompare = &bestFriend;
			bestDist = &distFriend;
			break;
		case POL_Neutral:
			bestToCompare = &bestNeutral;
			bestDist = &distNeutral;
			break;
		case POL_Enemy:
			continue;				// Skip enemy cities.
		}

		AuPoint cityLoc = (*ci)->EnvLocation();
		double dist = cityLoc.Distance2(shipLoc);
		if(!(*bestToCompare) || dist < *bestDist)
		{
			*bestToCompare = (DaBd*)*ci;
			*bestDist = dist;
		}
	}

	// Select the best city we found, prefering our own over our friends.
	DaBd* destAtom;
	if(bestEmpire)
		destAtom = bestEmpire;
	else if(bestFriend)
		destAtom = bestFriend;
	else if(bestNeutral)
		destAtom = bestNeutral;
	else
		destAtom = 0;
	return destAtom;
}

const DaVessel* DaVessel::MissionDestGoToClosestStation() const
{
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return 0;

	AuPoint shipLoc = EnvLocation();

	// Look for a suitable station.
	DaVessel* bestEmpire = 0;
	DaVessel* bestFriend = 0;
	DaVessel* bestNeutral = 0;
	double distEmpire = 0;
	double distFriend = 0;
	double distNeutral = 0;
	DaAtomList ships;
	system->FindVesselsRecursive(ships);
	for(DaAtomList::const_iterator si = ships.begin(); si != ships.end(); si++)
	{
		DaVessel* systemShip = dynamic_cast<DaVessel*>(*si);
		if(!systemShip)
			continue;
		if(!systemShip->IsSpaceStation())
			continue;				// We only look for stations.
		DaVessel** bestToCompare = 0;
		double* bestDist = 0;
		switch(PoliticalStance(EmpireId(), systemShip->EmpireId()))
		{
		case POL_Empire:
		case POL_Vassal:
			bestToCompare = &bestEmpire;
			bestDist = &distEmpire;
			break;
		case POL_Friend:
			bestToCompare = &bestFriend;
			bestDist = &distFriend;
			break;
		case POL_Neutral:
			bestToCompare = &bestNeutral;
			bestDist = &distNeutral;
			break;
		case POL_Enemy:
			continue;				// Skip enemy stations.
		}

		AuPoint stationLoc = systemShip->EnvLocation();
		double dist = stationLoc.Distance2(shipLoc);
		if(!(*bestToCompare) || dist < *bestDist)
		{
			*bestToCompare = systemShip;
			*bestDist = dist;
		}
	}

	// Select the best station we found, prefering our own over our friends.
	DaVessel* destAtom;
	if(bestEmpire)
		destAtom = bestEmpire;
	else if(bestFriend)
		destAtom = bestFriend;
	else if(bestNeutral)
		destAtom = bestNeutral;
	else
		destAtom = 0;
	return destAtom;
}

const DaAtom* DaVessel::MissionDestGoToClosestStationOrCity() const
{
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return 0;

	AuPoint shipLoc = EnvLocation();

	// This order never specifies a specific id.
	DaVessel* bestEmpireStation = 0;
	DaVessel* bestFriendStation = 0;
	DaVessel* bestNeutralStation = 0;
	double distEmpire = 0;
	double distFriend = 0;
	double distNeutral = 0;
	{
		// Look for a suitable station.
		DaAtomList ships;
		system->FindVesselsRecursive(ships);
		for(DaAtomList::const_iterator si = ships.begin(); si != ships.end(); si++)
		{
			DaVessel* systemShip = dynamic_cast<DaVessel*>(*si);
			if(!systemShip)
				continue;
			if(!systemShip->IsSpaceStation())
				continue;				// We only look for stations.
			DaVessel** bestToCompare = 0;
			double* bestDist = 0;
			switch(PoliticalStance(EmpireId(), systemShip->EmpireId()))
			{
			case POL_Empire:
			case POL_Vassal:
				bestToCompare = &bestEmpireStation;
				bestDist = &distEmpire;
				break;
			case POL_Friend:
				bestToCompare = &bestFriendStation;
				bestDist = &distFriend;
				break;
			case POL_Neutral:
				bestToCompare = &bestNeutralStation;
				bestDist = &distNeutral;
				break;
			case POL_Enemy:
				continue;				// Skip enemy stations.
			}

			AuPoint stationLoc = systemShip->EnvLocation();
			double dist = stationLoc.Distance2(shipLoc);
			if(!(*bestToCompare) || dist < *bestDist)
			{
				*bestToCompare = systemShip;
				*bestDist = dist;
			}
		}
	}

	DaAtom* bestEmpireCity = 0;
	DaAtom* bestFriendCity = 0;
	DaAtom* bestNeutralCity = 0;
	if(!bestEmpireStation)
	{
		// Look for a suitable city.
		DaAtomList bds;
		system->FindBdRecursive(bds);
		for(DaAtomList::const_iterator ci = bds.begin(); ci != bds.end(); ci++)
		{
			// We only look for airports.
			DaBd* systemBd = dynamic_cast<DaBd*>(*ci);
			if(!systemBd)
				continue;
			if(IsPrivatelyOwned())
			{
				if(systemBd->IndustryId() != IID_AirportTerminal)
					continue;
			}
			else
			{
				if(systemBd->IndustryId() != IID_AirportTerminal && systemBd->IndustryId() != IID_BaseAirTerminal)
					continue;
			}

			DaAtom** bestToCompare = 0;
			double* bestDist = 0;
			switch(PoliticalStance(EmpireId(), (*ci)->EmpireId()))
			{
			case POL_Empire:
			case POL_Vassal:
				bestToCompare = &bestEmpireCity;
				bestDist = &distEmpire;
				break;
			case POL_Friend:
				bestToCompare = &bestFriendCity;
				bestDist = &distFriend;
				break;
			case POL_Neutral:
				bestToCompare = &bestNeutralCity;
				bestDist = &distNeutral;
				break;
			case POL_Enemy:
				continue;				// Skip enemy cities.
			}

			AuPoint cityLoc = (*ci)->EnvLocation();
			double dist = cityLoc.Distance2(shipLoc);
			if(!(*bestToCompare) || dist < *bestDist)
			{
				*bestToCompare = *ci;
				*bestDist = dist;
			}
		}
	}

	// Select the best station or city we found, prefering our own over our friends.
	DaAtom* destAtom;
	if(bestEmpireStation)
		destAtom = bestEmpireStation;
	else if(bestEmpireCity)
		destAtom = bestEmpireCity;
	else if(bestFriendStation)
		destAtom = bestFriendStation;
	else if(bestFriendCity)
		destAtom = bestFriendCity;
	else if(bestNeutralStation)
		destAtom = bestNeutralStation;
	else if(bestNeutralCity)
		destAtom = bestNeutralCity;
	else
		destAtom = 0;
	return destAtom;
}

const DaWorld* DaVessel::MissionDestGoToOrbitBestPlanet(AuPoint& destOffset) const
{
	const DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return 0;

	DaWorld* destWorld = 0;
	bool hasBreathableAtm = false;	// true if we find any planet with a breathable atmosphere
	DaAtomList systemAtoms;
	system->ListNormalSubatomsRecursive(systemAtoms);
	for(DaAtomList::const_iterator sai = systemAtoms.begin(); sai != systemAtoms.end(); sai++)
	{
		DaWorld* planet = dynamic_cast<DaWorld*>(*sai);
		if(!planet)
			continue;

		if(planet->IsHabitableOrbit() && planet->HasBreathableAtmosphere())
		{
			// Found a planet in a habitable orbit with a breathable atmosphere.
			// This could be a titan.
			destWorld = planet;
			break;	// Stop looking when we find a good one like this.
		}
		else if(planet->HasBreathableAtmosphere())
		{
			// Found a planet with a breathable atmosphere.
			// This could be a titan.
			// This must be in the inner, habitable, or outer zone.
			destWorld = planet;
			hasBreathableAtm = true;
		}
		else if(!hasBreathableAtm &&
				(dynamic_cast<const DaWorldSphere*>(planet) ||
				 dynamic_cast<const DaWorldRing*>(planet)))
		{
			// It's a planet, ring or gas giant and we have not found a planet with a breathable atmosphere yet.
			// This doesn't check ringworlds because they always have breathable atmospheres.
			destWorld = planet;
		}
	}

	if(destWorld)
	{
		// Calculate the closest orbit point to the ship.
		const AuMatrix4x4& destT = destWorld->EnvTransform();
		destOffset = destT.UnProject(EnvLocation());					// Ship loc in planet coords.

		// Calculate a location in the orbit zone of the ringworld, directly above/below the destOffset location.
		double orbitAlt;
		if(FindParentPlanetInEnv() == destWorld)
			orbitAlt = destWorld->AtmosphereRadius() + ParkAltAboveAtm();
		else
			orbitAlt = destWorld->AtmosphereRadius() + OrbitAltAboveAtm();	// Higher-up but inside waypoint-ok range.

		if(dynamic_cast<const DaWorldRingSection*>(destWorld))
			destOffset.y = 0;									// Flattened to equatorial plane of ringworld section.
		destOffset = destWorld->FindOrbitSpawnLoc(destOffset, Id(), RoughRadius(), orbitAlt, false);
	}
	return destWorld;
}

const DaAtom* DaVessel::MissionDestGoToOrbitHoverLand(const AuMissionOrder& order, AuPoint& destOffset) const
{
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return 0;

	const DaWorld* parentPlanet = FindParentPlanetInEnv();
	const DaWorld* destWorld;
	const DaAtom* destCityOrBd = 0;
	double ringRadius = 0;										// Only if destination is a ring, then it's the ring radius.
	if(order.DestinationId().IsNull())
	{
		// Orbit, hover or land at my current planet.
		destWorld = parentPlanet;
	}
	else if(order.Type() == AuMissionOrder::GoToOrbitCity)
	{
		// Id is a city.
		destCityOrBd = system->FindIdRecursive(order.DestinationId());	// sun sun planet moon city/bd
		if(destCityOrBd)
			destWorld = destCityOrBd->FindParentPlanetInEnv();
		else
			return 0;	// Not going to find a city at a star.
	}
	else
	{
		// Order specifies a planet.
		// Companion stars are always subordinate to the primary in our universe.
		destWorld = system->FindDestinationWorld(order.DestinationId(), order.DestinationName());
		if(dynamic_cast<const DaWorldRing*>(destWorld))
		{
			// When destination planet is a ring, go to the parent planet instead.
			ringRadius = destWorld->Location().Length();
			destWorld = dynamic_cast<const DaWorld*>(destWorld->Parent());
		}
	}

	// If no destination planet was found, try a star.
	if(!destWorld)
		return MissionDestGoToStar(order, destOffset);

	// Determine a specific location relative to the ringworld, based on the order.
	if(destWorld != parentPlanet &&
			!dynamic_cast<const DaWorldRingSection*>(destWorld))
	{
		// Not at the destination world, so treat it like a simple orbit order. That way the destination planet does not act as an
		// obstruction to our path when we are a long distance away, like if the destination coords are on the other side from me.
		// This is not done for ringworlds because we need to head for the proper relative location. The pilot has logic to avoid
		// ringworlds in its collision avoidance code.
		// Calculate the location of the ship relative to the planet.
		// Target altitude is below the altitude needed to call the waypoint good when coming in from the outside.
		const double orbitAlt = destWorld->AtmosphereRadius() + OrbitAltAboveAtm();
		const AuMatrix4x4& destT = destWorld->EnvTransform();
		destOffset = destT.UnProject(EnvLocation());				// Ship loc relative to the planet.

		// Calculate a location in the orbit zone of the planet, directly above/below the destOffset location.
		// We already know it's not a ringworld.
		destOffset = destWorld->FindOrbitSpawnLoc(destOffset, Id(), RoughRadius(), orbitAlt, false);
	}
	else if(order.Type() == AuMissionOrder::GoToHoverSite)
	{
		// Calculate the location of the coordinates specified in the order.
		AuGlobeLLF llf = order.GlobeLL();
		const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(destWorld);
		if(llworld)
		{
			// Location is at ground level, relative to the planet.
			destOffset = llworld->GetSurfaceLocation(llf);
		}
		else
			goto doOrbit;

		// Calculate hover location above landing site.
		double orbitAlt = destWorld->CGAltitude(destOffset);
		if(orbitAlt < destWorld->SeaLevelAltitude())
			orbitAlt = destWorld->SeaLevelAltitude();

		orbitAlt += order.Altitude()*FeetPerMeter() + GroundHeightAboveCG();
		if(destWorld == parentPlanet)
		{
			destWorld->ElevateLoc(destOffset, orbitAlt);
			AuPoint shipWorldLoc;
			GetLocationRelativeToParent(shipWorldLoc, destWorld);	// Ship loc relative to planet.
			if(destOffset.Distance(shipWorldLoc) < 100*FeetPerMeter())
				destOffset = shipWorldLoc;	// My current location is good enough.
			else
			{
				// Find unobstructed location near the loc.
				destOffset = destWorld->FindOrbitSpawnLoc(destOffset, Id(), RoughRadius(), orbitAlt, false);
				if(destOffset.Distance(shipWorldLoc) < 100*FeetPerMeter())
					destOffset = shipWorldLoc;	// My current location is good enough.
			}
		}
		else
			destOffset = destWorld->FindOrbitSpawnLoc(destOffset, Id(), RoughRadius(), orbitAlt, false);
	}
	else if(order.Type() == AuMissionOrder::GoToLandingSite)
	{
		// Calculate the location of the coordinates specified in the order.
		AuGlobeLLF llf = order.GlobeLL();
		const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(destWorld);
		if(llworld)
		{
			// Location is at ground level, relative to the planet.
			destOffset = llworld->GetSurfaceLocation(llf);
		}
		else
			goto doOrbit;
	}
	else if(order.Type() == AuMissionOrder::GoToOrbitCoords)
	{
		// Calculate the location of the coordinates specified in the order.
		AuGlobeLLF llf = order.GlobeLL();
		const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(destWorld);
		if(llworld)
		{
			// Location is at ground level, relative to the planet.
			destOffset = llworld->GetSurfaceLocation(llf);
		}
		else
			goto doOrbit;

		// Calculate orbit location at coordinates.
		if(ringRadius > 0)
		{
			destOffset.z = 0;									// Rings are at the Z=0 plane in the space of their parent planet.
			destOffset.Unit(ringRadius);						// Orbit above parent planet at center of ring.
		}
		else
		{
			double orbitAlt;
			if(destWorld == parentPlanet)
			{
				AuPoint shipWorldLoc;
				GetLocationRelativeToParent(shipWorldLoc, destWorld);	// Ship loc relative to planet.
				orbitAlt = destWorld->AtmosphereRadius() + ParkAltAboveAtm();
				destWorld->ElevateLoc(destOffset, orbitAlt);
				if(destOffset.Distance(shipWorldLoc) < 100*FeetPerMeter())
					destOffset = shipWorldLoc;	// My current location is good enough.
				else
				{
					// Find unobstructed location near the loc.
					destOffset = destWorld->FindOrbitSpawnLoc(destOffset, Id(), RoughRadius(), orbitAlt, false);
					if(destOffset.Distance(shipWorldLoc) < 100*FeetPerMeter())
						destOffset = shipWorldLoc;	// My current location is good enough.
				}
			}
			else
			{
				orbitAlt = destWorld->AtmosphereRadius() + OrbitAltAboveAtm();	// Higher-up but inside waypoint-ok range.
				destOffset = destWorld->FindOrbitSpawnLoc(destOffset, Id(), RoughRadius(), orbitAlt, false);
			}
		}
	}
	else if(order.Type() == AuMissionOrder::GoToLand)
	{
		// Any landing site, preferably in daylight.
		// Suggest the ship's current location; it might be just fine.
		// Calculate the location of the ship relative to the planet.
		if(destWorld == parentPlanet)
			GetLocationRelativeToParent(destOffset, destWorld);		// Ship loc relative to planet.
		else
		{
			const AuMatrix4x4& destT = destWorld->EnvTransform();
			destOffset = destT.UnProject(EnvLocation());				// Ship loc relative to the planet.
		}
		const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(destWorld);
		const DaWorldRingSection* rsworld = dynamic_cast<const DaWorldRingSection*>(destWorld);
		if(!llworld)
			goto doOrbit;

		AuPoint shipLoc(destOffset);

		if(!IsInAtmosphere())
		{
			// Not in the atmosphere so don't suggest my current location. Let the planet return the best one based on current
			// lighting conditions.
			destOffset = AuPoint(0,0);
		}

		// Find a location on land.
		// Select a random location, or the destOffset if it's over land.
		destOffset = llworld->RandomLandingSite(destOffset);	// At ground level, relative to the planet.
		if(destOffset.IsZero())
		{
			// No landing site was found.
			destOffset = shipLoc;
			if(rsworld)
				destOffset.y = 0;								// Flatten to equatorial plane of ringworld section.
		}
	}
	else if(order.Type() == AuMissionOrder::GoToOrbitCity)
	{
		// Head for the location of the city.
		Q_ASSERT(destCityOrBd);									// We shouldn't be here if this is null.
		destOffset = destCityOrBd->Location();

		// Calculate orbit location at coordinates.
		if(destWorld == parentPlanet)
		{
			double orbitAlt = destWorld->AtmosphereRadius() + ParkAltAboveAtm();
			destWorld->ElevateLoc(destOffset, orbitAlt);
			AuPoint shipWorldLoc;
			GetLocationRelativeToParent(shipWorldLoc, destWorld);	// Ship loc relative to planet.
			if(destOffset.Distance(shipWorldLoc) < 100*FeetPerMeter())
				destOffset = shipWorldLoc;	// My current location is good enough.
			else
			{
				// Find unobstructed location near the loc.
				destOffset = destWorld->FindOrbitSpawnLoc(destOffset, Id(), RoughRadius(), orbitAlt, false);
				if(destOffset.Distance(shipWorldLoc) < 100*FeetPerMeter())
					destOffset = shipWorldLoc;	// My current location is good enough.
			}
		}
		else
		{
			double orbitAlt = destWorld->AtmosphereRadius() + OrbitAltAboveAtm();	// Higher-up but inside waypoint-ok range.
			destOffset = destWorld->FindOrbitSpawnLoc(destOffset, Id(), RoughRadius(), orbitAlt, false);
		}
	}
	else //if(order.Type() == AuMissionOrder::GoToOrbit)
	{
		// Calculate the closest orbit point to the ship.
doOrbit:
		// Calculate the location of the ship relative to the planet.
		AuPoint shipWorldLoc;
		if(destWorld == parentPlanet)
		{
			GetLocationRelativeToParent(shipWorldLoc, destWorld);	// Ship loc relative to planet.
			destOffset = shipWorldLoc ;
		}
		else
		{
			const AuMatrix4x4& destT = destWorld->EnvTransform();
			destOffset = destT.UnProject(EnvLocation());			// Ship loc relative to the planet.
		}

		if(ringRadius)
		{
			destOffset.z = 0;									// Rings are at the Z=0 plane in the space of their parent planet.
			destOffset.Unit(ringRadius);						// Destination is the center of the ring.
		}
		else
		{
			// Calculate a location in the orbit zone of the planet, directly above/below the destOffset location.
			const DaWorldRingSection* rsworld = dynamic_cast<const DaWorldRingSection*>(destWorld);
			if(rsworld)
			{
				// Destination is a ringworld section.
				destOffset.y = 0;								// Flattened to equatorial plane of ringworld section.
				AuGlobeLL ll;
				if(!rsworld->LLOfLoc(ll, destOffset))
					destOffset = rsworld->LocOfLL(AuGlobeLL(0,0));
			}

			if(destWorld == parentPlanet)
			{
				double orbitAlt = destWorld->AtmosphereRadius() + ParkAltAboveAtm();
				destWorld->ElevateLoc(destOffset, orbitAlt);
				AuPoint shipWorldLoc;
				GetLocationRelativeToParent(shipWorldLoc, destWorld);	// Ship loc relative to planet.
				if(destOffset.Distance(shipWorldLoc) < 100*FeetPerMeter())
					destOffset = shipWorldLoc;	// My current location is good enough.
				else
				{
					// Find unobstructed location near the loc.
					destOffset = destWorld->FindOrbitSpawnLoc(destOffset, Id(), RoughRadius(), orbitAlt, false);
					if(destOffset.Distance(shipWorldLoc) < 100*FeetPerMeter())
						destOffset = shipWorldLoc;	// My current location is good enough.
				}
			}
			else
			{
				double orbitAlt = destWorld->AtmosphereRadius() + OrbitAltAboveAtm();	// Higher but inside waypoint-ok range.
				destOffset = destWorld->FindOrbitSpawnLoc(destOffset, Id(), RoughRadius(), orbitAlt, false);
			}
		}
	}

	if(destWorld != parentPlanet && destWorld->IsInAsteroidBelt())
	{
		// Asteroid belt objects orbit the sun very quickly. We have to aim for a point ahead of them.
		// Once we get into their zone, we'll change strategy.
		destOffset = destWorld->PredictLocation(destOffset, 10);
	}
	return destWorld;
}

const DaStar* DaVessel::MissionDestGoToStar(const AuMissionOrder& order, AuPoint& destOffset) const
{
	if(InArena())
		return 0;
	if(InDesignEnvironment())
		return 0;
	const DaSolarSystem* system = FindParentSolarSystem();
	if(!system || system->IsEmptySystem())
		return 0;

	const DaStar* parentStar = FindParentStarInEnv();
	const DaStar* destStar;
	if(order.DestinationId().IsNull())
	{
		// Orbit, hover or land at my current star.
		destStar = parentStar;
	}
	else
	{
		// Order specifies a star.
		// Companion stars are always subordinate to the primary in our universe.
		// Worst possible hierarchy from system to star: star, star
		destStar = system->FindDestinationStar(order.DestinationId(), order.DestinationName());
	}

	// If no destination star was found, give up.
	if(!destStar)
		return 0;

	// Determine a specific location relative to the ringworld, based on the order.
	if(destStar != parentStar)
	{
		// Not at the destination world, so treat it like a simple orbit order. That way the destination star does not act as an
		// obstruction to our path when we are a long distance away, like if the destination coords are on the other side from me.
		// Calculate the location of the ship relative to the star.
		// Target altitude is below the altitude needed to call the waypoint good when coming in from the outside.
		const double orbitAlt = destStar->AtmosphereRadius() + OrbitAltAboveSun();
		const AuMatrix4x4& destT = destStar->EnvTransform();
		destOffset = destT.UnProject(EnvLocation());				// Ship loc relative to the star.

		// Calculate a location in the orbit zone of the star, directly above/below the destOffset location.
		destOffset.Unit(orbitAlt);
	}
	else if(order.Type() == AuMissionOrder::GoToOrbitCoords)
	{
		// Calculate the location of the coordinates specified in the order.
		AuGlobeLLF llf = order.GlobeLL();
		double rxyangle = llf.Longitude();
		double rincline = llf.Latitude();							// Longitude is negative as we go south.

		// Calculate orbit location at coordinates.
		double orbitAlt = destStar->AtmosphereRadius() + ParkAltAboveSun();
		destOffset.PolarValue(orbitAlt, rxyangle, rincline);
	}
	else
	{
		// Calculate the closest orbit point to the ship.
		GetLocationRelativeToParent(destOffset, destStar);	// Ship loc relative to star.

		// Calculate a location in the orbit zone of the star, directly above/below the destOffset location.
		double orbitAlt = destStar->AtmosphereRadius() + ParkAltAboveSun();
		destOffset.Unit(orbitAlt);
	}
	return destStar;
}

const DaAtom* DaVessel::MissionDestGoToCoordinate(const AuMissionOrder& order, AuPoint& destOffset) const
{
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return 0;

	// Get the relative destination offset from the mission order.
	// This will be in parsecs for galaxy and sector relativity, astronomical units for system relativity.
	destOffset = order.DestinationCoordinate();

	// Convert destOffset to be in feet, relative to the solar system.
	switch(order.DestinationContext())
	{
	case 0:								// Coordinate is in parsecs, relative to the galaxy.
		// Convert offset from pc relative to the galaxy, to feet relative to the solar system.
		if(InArena())
			return 0;
		else if(InDesignEnvironment())
			return 0;
		else
		{
			const DaGalaxy* galaxy = FindParentGalaxy();
			if(!galaxy)
				return 0;
			AuMatrix4x4 tsys;
			system->GetTransformRelativeToParent(tsys, galaxy);
			destOffset = tsys.UnProject(destOffset);
		}
		break;
	case 1:								// Coordinate is in parsecs, relative to the sector.
		// Convert offset from pc relative to the sector, to feet relative to the solar system.
		if(InArena())
			return 0;
		else if(InDesignEnvironment())
			return 0;
		else
		{
			const DaSector* sector = FindParentSector();
			if(!sector)
				return 0;
			AuMatrix4x4 tsys;
			system->GetTransformRelativeToParent(tsys, sector);
			destOffset = tsys.UnProject(destOffset);
		}
		break;
	case 2:								// Coordinate is in astronomical units, relative to the solar system.
		// Convert offset from AU to feet.
		destOffset *= FeetPerAu();
		break;
	}

	return system;
}

const DaAtom* DaVessel::MissionDestAttackFollow(const AuMissionOrder& order) const
{
	const DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return 0;

	// Order specifies an atom.
	return system->FindIdRecursive(order.TargetId());
}

bool DaVessel::DestroyCargo(unsigned int unitsDamage)
{
	if(!unitsDamage)
		return false;

	// Determine the total amount of cargo aboard.
	const int cargoQty = CargoTotalVolume();
	if(!cargoQty)
		return false;	// No cargo to lose.

	// Lose 1 unit of cargo for every 5 units of damage.
	int amtToLose;
	if(unitsDamage > 5)
	{
		amtToLose = unitsDamage/5;
		if(amtToLose > cargoQty)
			amtToLose = cargoQty;
	}
	else
		amtToLose = 1;

	m_CargoLock.lockForWrite();
	while(amtToLose > 0)
	{
		// Remove random cargo.
		int randomIndex = g_Au.Dice().Roll(1, (unsigned int)m_Cargo.size(), -1);
		for(AuItemHash::iterator itemI = m_Cargo.begin(); itemI != m_Cargo.end(); itemI++)
		{
			if(!randomIndex)
			{
				amtToLose -= (*itemI).second->Consume(amtToLose);
				if((*itemI).second->Count() == 0)
					m_Cargo.Erase(itemI);
				break;
			}
			randomIndex--;
		}
	}

	m_tmCargoChange.Set();
	m_CargoLock.unlock();
	SetModified();
	return true;
}

bool DaVessel::DestroyFuel(unsigned int unitsDamage)
{
	if(!FuelQuantity() || !unitsDamage)
		return false;

	m_VesselLock.lockForWrite();
	// Lose 1 unit of fuel for every 5 units of damage.
	float amtToLose = unitsDamage/5;
	if(amtToLose > m_fFuelQuantity)
		amtToLose = m_fFuelQuantity;

	m_fFuelQuantity -= amtToLose;
	m_VesselLock.unlock();

	SetModified();
	return true;
}

COMMODITYID DaVessel::DestroyVehicle()
{
	DaAtomList vehicles;
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaVehicle* vehicle = dynamic_cast<DaVehicle*>(*it);
		if(!vehicle)
			continue;
		if(vehicle->IsOccupied())
			continue;										// Looking to destroy a vehicle from a bay, not one that is in use.
		
		// Found an unoccupied vehicle aboard.
		vehicles.append(vehicle);
	}

	if(!vehicles.isEmpty())
	{
		// Randomly select one of the vehicles we found.
		int vehicleIndex = g_Au.Dice().Roll(1,vehicles.count(),-1);
		DaAtomList::const_iterator it = vehicles.begin() + vehicleIndex;
		DaVehicle* vehicle = dynamic_cast<DaVehicle*>(*it);
		if(vehicle)
		{
			COMMODITYID vehicleCID = vehicle->CommodityId();
			vehicle->DestroyVehicle(tr("Critical hit on parent spacecraft."));
			return vehicleCID;
		}
	}
	return CID_Invalid;
}

// Returns true if iIndex is a valid system index and the system is not at 0% health.
// If a valid healthy system is selected, it is disconnected from any consumer it is currently connected to.
bool DaVessel::SystemSupplierSelection(int iIndex)
{
	//Q_ASSERT(!g_Au.IsSceneClient());	// Scene client does not rx enough m_ssaShipSystems data to do this.
	if(iIndex < 0)
		return false;

	m_SystemsLock.lockForWrite();
	if(iIndex >= m_ssaShipSystems.count())
	{
		m_SystemsLock.unlock();
		return false;
	}
	
	// Remove connections with other systems
	if(m_ssaShipSystems[iIndex].GetConnectedConsumer() != -1)
	{
		m_ssaShipSystems.DisconnectConsumer(iIndex);
		SetModified();
	}
	m_SystemsLock.unlock();
	return true;
}

void DaVessel::SystemConsumerSelection(int iConsumerIndex, int iSupplierIndex)
{
	//Q_ASSERT(!g_Au.IsSceneClient());	// Scene client does not rx enough m_ssaShipSystems data to do this.
	m_SystemsLock.lockForWrite();
	m_ssaShipSystems[iSupplierIndex].SetConnectedConsumer(iConsumerIndex);
	m_ssaShipSystems.RecalculatePowerConsumption(iConsumerIndex);
	m_tmSystemsUpg.Set();
	m_SystemsLock.unlock();

	SetModified();
}

void DaVessel::UpdateDisplayedSystems(const qint32 vSuppliers[MAX_DISPLAYED_SYSTEMS],
		SYSTEM* q8SupplierType,
		quint8* q8SupplierDamage,
		quint8* q8SupplierPower,
		quint8* q8SupplierConnectionConsumer,
		const qint32 vConsumers[MAX_DISPLAYED_SYSTEMS],
		SYSTEM* q8ConsumerType,
		quint8* q8ConsumerConnectionAndPower,
		quint8& q8PowerPlantShunts) const
{
	m_SystemsLock.lockForRead();
	{
		// Connection Information
		for(int iSupplierIndex = 0; iSupplierIndex < MAX_DISPLAYED_SYSTEMS; iSupplierIndex++)
		{
			int sysIdx = vSuppliers[iSupplierIndex];
			if(sysIdx < 0)
				continue;
			// HIGH 4 = Connection (off, Connect, D/C)
			if(m_ssaShipSystems.at(sysIdx).GetConnectedConsumer() == -1)
				q8SupplierConnectionConsumer[iSupplierIndex] = 0x10;	// Not connected to a consumer.
			else
				q8SupplierConnectionConsumer[iSupplierIndex] = 0x20;	// Connected to a consumer.
			if(m_ssaShipSystems.at(sysIdx).GetHealthCurrent() == 0)
				q8SupplierConnectionConsumer[iSupplierIndex] |= 0x40; 	// Too Damaged to provide power
			// LOW 4 = Consumer (MAX_DISPLAYED_SYSTEMS or higher means offscreen)
			quint8 q8ConnectionConsumer = MAX_DISPLAYED_SYSTEMS;
			for(int iConsumerIndex = 0; iConsumerIndex < MAX_DISPLAYED_SYSTEMS; iConsumerIndex++)
			{
				if(vConsumers[iConsumerIndex] == m_ssaShipSystems.at(sysIdx).GetConnectedConsumer())
				{
					q8ConnectionConsumer = iConsumerIndex;
					break;
				}
			}
			q8SupplierConnectionConsumer[iSupplierIndex] |= q8ConnectionConsumer;
		}
	}
	{
		// Shunt Information
		// 1 bit per possible shunt
		q8PowerPlantShunts = m_PowerPlantShunts;
	}
	{
		// Consumer Information
		for(int iConsumerIndex = 0; iConsumerIndex < MAX_DISPLAYED_SYSTEMS; iConsumerIndex++)
		{
			int sysIdx = vConsumers[iConsumerIndex];
			if(sysIdx < 0)
				continue;
			// HIGH 3 : 0x80 = a connected supplier is offscreen
			//          0x40 = a supplier is connected to this consumer
			//          0x20 = no supplier is connected to this consumer
			q8ConsumerConnectionAndPower[iConsumerIndex] = 0x20; // Init to disconnected value.
			int supplyIndex = 0;
			for(AuShipSystemArray::const_iterator si = m_ssaShipSystems.cbegin();
					si != m_ssaShipSystems.cend(); si++, supplyIndex++)
			{
				if((*si).GetConnectedConsumer() == sysIdx)
				{
					// Found a supplier connected to this consumer.
					q8ConsumerConnectionAndPower[iConsumerIndex] = 0x40;	// A supplier is connected to this consumer.

					// Is the supplier off-screen?
					bool supplyOffScreen = true;
					for(int iSupplierIndex = 0; iSupplierIndex < MAX_DISPLAYED_SYSTEMS; iSupplierIndex++)
					{
						if(vSuppliers[iSupplierIndex] == supplyIndex)
						{
							// The supplier is in our list of displayed suppliers so it is on-screen.
							supplyOffScreen = false;
							break;
						}
					}
					if(supplyOffScreen)
					{
						// The supplier is offscreen; we can stop looping now.
						q8ConsumerConnectionAndPower[iConsumerIndex] = 0xc0;	// Set both the offscreen and connected bits.
						break;
					}
				}
			}

			// LOW 5 = Power in pixels (max 31)
			if(m_ssaShipSystems.at(sysIdx).GetPowerConsumptionCurrent())
			{
				int powerVal = 31 * m_ssaShipSystems.at(sysIdx).GetPowerConsumptionCurrent() /
					m_ssaShipSystems.at(sysIdx).GetPowerConsumptionMax();
				if(!powerVal)
					powerVal = 1;	// Show minimum amount.
				q8ConsumerConnectionAndPower[iConsumerIndex] |= powerVal;
			}

			q8ConsumerType[iConsumerIndex] = m_ssaShipSystems.at(sysIdx).GetSystemType();
		}
	}
	{
		// Supplier Information
		for(int iSupplierIndex = 0; iSupplierIndex < MAX_DISPLAYED_SYSTEMS; iSupplierIndex++)
		{
			int sysIdx = vSuppliers[iSupplierIndex];
			if(sysIdx < 0)
				continue;
			// HIGH 3 = Overclocked (true/false)
			q8SupplierPower[iSupplierIndex] = m_ssaShipSystems.at(sysIdx).GetOverclocking() ? 0x20 : 0x00;
			// LOW 5 = Power in pixels (max 31)
			if(m_ssaShipSystems.at(sysIdx).GetPowerSupplyCurrent())
			{
				int powerVal = 31 * m_ssaShipSystems.at(sysIdx).GetPowerSupplyCurrent() /
					m_ssaShipSystems.at(sysIdx).GetPowerSupplyMax();
				if(!powerVal)
					powerVal = 1;	// Show minimum amount.
				q8SupplierPower[iSupplierIndex] |= powerVal;
			}

			q8SupplierType[iSupplierIndex] = m_ssaShipSystems.at(sysIdx).GetSystemType();
			if(m_ssaShipSystems.at(sysIdx).GetHealthCurrent())
			{
				int healthVal = 15 * m_ssaShipSystems.at(sysIdx).GetHealthCurrent() / m_ssaShipSystems.at(sysIdx).GetHealthMax();
				if(!healthVal)
					healthVal = 1;	// Show minimum amount.
				q8SupplierDamage[iSupplierIndex] = healthVal;
			}
			else
				q8SupplierDamage[iSupplierIndex] = 0;
		}
	}
	m_SystemsLock.unlock();
}

void DaVessel::TrimCargo()
{
	int cargoCap = CargoCapacityVolume();	// Adjusted for damage.
	int cargoQty = CargoTotalVolume();
	if(cargoQty > cargoCap)
	{
		int amtToLose = cargoQty - cargoCap;
		while(amtToLose > 0)
		{
			// Remove random cargo.
			m_CargoLock.lockForWrite();
			int randomIndex = g_Au.Dice().Roll(1, (unsigned int)m_Cargo.size(), -1);
			for(AuItemHash::iterator itemI = m_Cargo.begin(); itemI != m_Cargo.end(); itemI++)
			{
				if(!randomIndex)
				{
					amtToLose -= (*itemI).second->Consume(amtToLose);
					if((*itemI).second->Count() == 0)
						m_Cargo.Erase(itemI);
					break;
				}
				randomIndex--;
			}
			m_CargoLock.unlock();
		}
	}
}

void DaVessel::TrimFighters()
{
	//DHH TrimFighters
}

void DaVessel::TrimFuel()
{
	float fuelCap = FuelCapacity();	// Adjusted for damage.
	m_VesselLock.lockForWrite();
	if(m_fFuelQuantity > fuelCap)
	{
		m_fFuelQuantity = fuelCap;
		SetModified();
	}
	m_VesselLock.unlock();
}

// Called by stations and cities.
// Searches recursively for any trespassing cities or stations.
const DaAtom* DaVessel::FindTrespasserRecursive(quint32 empireId, const AuPoint& systemLoc, float sensorRange) const
{
	if(!IsSpaceStation())
		return 0;	// I am not a station.

	POLITICAL_STANCE stance = PoliticalStance(empireId, EmpireId());
	if(stance != POL_Neutral)
		return 0;	// I am not a neutral city to the scanner.

	if(EnvLocation().Distance2(systemLoc) > sensorRange*sensorRange)
		return 0;	// I am not in sensor range.

	// The scanning empire considers any station in sensor range to be a trespasser.
	return this;
}

void DaVessel::LogEvent(AuEventLogEntry::TYPE type, const QString& desc, const AuId& atomId)
{
	if(IsPirateWithFreeWill() || InArena() || InDesignEnvironment())
		return;
	DaSentient* firstOfficer = FindOfficer();					// Find any officer, even if they don't have the con.
	if(firstOfficer)
	{
		// The first officer reports the event in his log.
		firstOfficer->LogEvent(type, desc, atomId);
		return;
	}

	// Space station notifies the nearest airport, capitol or base, if any, so the event will show up in its report.
	if(!IsSpaceStation())
		return;
	DaSolarSystem* system = FindParentSolarSystemInEnv();
	if(!system)
		return;		// Am I in an arena?

	// Station uses m_ATCCity to cache the bd it notifies.
	m_CommLock.lockForRead();
	AuId cityId(m_ATCCity);
	m_CommLock.unlock();
	DaBd* bdNotify;
	if(!cityId.IsNull())
		bdNotify = dynamic_cast<DaBd*>(system->FindIdRecursive(cityId));	// Try to reuse the same building as last time.
	else
		bdNotify = 0;
	if(!bdNotify || !bdNotify->IsNormal() ||
			(bdNotify->IndustryId() != IID_AirportTerminal &&
			 bdNotify->IndustryId() != IID_BaseAirTerminal &&
			 bdNotify->IndustryId() != IID_Capitol &&
			 bdNotify->IndustryId() != IID_BaseHeadquarters))
	{
		// Must do the hard work of looking for the closest building.
		DaAtomList bds;
		system->FindBdRecursive(bds, EmpireId());	// Start with a list of all buildings of my empire.
		if(bds.empty())
		{
			// No buildings of my empire in the system.
			if(!cityId.IsNull())
			{
				// Clear the cached city id so we don't keep looking for it.
				m_CommLock.lockForWrite();
				m_ATCCity = AuId();
				m_CommLock.unlock();
			}
			return;
		}

		// Which empire bd is the closest that we want?
		AuPoint mySysLoc = EnvLocation();
		bdNotify = 0;
		double closestDist = 0;
		for(DaAtomList::const_iterator bi = bds.cbegin(); bi != bds.cend(); bi++)
		{
			if(!(*bi)->IsNormal())
				continue;
			DaBd* bd = dynamic_cast<DaBd*>(*bi);
			if(!bd)
				continue;
			if(bd->IndustryId() != IID_AirportTerminal &&
					bd->IndustryId() != IID_BaseAirTerminal &&
					bd->IndustryId() != IID_Capitol &&
					bd->IndustryId() != IID_BaseHeadquarters)
				continue;	// We don't report events to this type of building.
			AuPoint bdSysLoc = bd->EnvLocation();
			double dist = mySysLoc.Distance2(bdSysLoc);
			if(!bdNotify || dist < closestDist)
			{
				bdNotify = bd;
				closestDist = dist;
			}
		}

		if(bdNotify)
		{
			// Cache the result so we don't have to do all that work next time.
			m_CommLock.lockForWrite();
			m_ATCCity = bdNotify->Id();
			m_CommLock.unlock();
		}
	}
	if(bdNotify)
		bdNotify->LogEvent(type, desc, atomId);	// A building was found to notify.
}

// Acceleration/deceleration in ft/sec/sec due to gravity and other factors.
// Adjusted for damage.
AuPoint DaVessel::Acceleration() const
{
	// No acceleration inside the special zone inside a star.
	if(IsInSunSafeZone())
		return AuPoint(0,0);
	else if(InArenaCannotAccelerate())
		return AuPoint(0,0);

	if(!HasManeuverDrive())
		return AuPoint(0,0);

	float fAccel = ManeuverAcceleration();

	// Decay performance according to skew.
	const float rlongest = 40;
	float rdecay = (float)abs(m_chAccelSkew)/rlongest;
	if(rdecay < 0 || rdecay > 1)
	{
		qDebug("DaVessel::Acceleration %d accelSkew %d rlongest %g rdecay %g", Id().GetID(), m_chAccelSkew, rlongest, rdecay);
		rdecay = .5f;
	}
	fAccel = (float)(fAccel * (1.f - rdecay));

	// Gravity is not applied when gravity drives are operated outside the atmosphere, except in black hole systems.
	bool applyGravity = true;
	bool brakesNegateGravity = false;
	if(IsInTow())
	{
		applyGravity = false;									// No gravity while I am being towed by a tractor beam.
		if(fAccel > FeetPerMeter())
		{
			// Deduct power of tractor beams from my acceleration. Cheesy but close enough.
			DaAtom* envAtom = FindEnvironmentAtom();
			if(envAtom)
			{
				DaAtomList ships;
				envAtom->FindVesselsRecursive(ships);
				for(DaAtomList::const_iterator si = ships.cbegin(); si != ships.cend(); si++)
				{
					const DaVessel* ship = dynamic_cast<const DaVessel*>(*si);
					if(!ship)
						continue;
					if(!ship->IsNormal())
						continue;
					if(ship->Is(Id()))
						continue;
					double tractorRange;
					double tractorAccel;
					if(!ship->IsTowing(Id(), Mass(), tractorRange, tractorAccel))
						continue;
					fAccel -= tractorAccel;
					if(fAccel <= FeetPerMeter())
					{
						// Leave them a teensy bit of acceleration.
						fAccel = (float)FeetPerMeter();
						break;
					}
				}
			}
		}
	}
	else if(!IsInAtmosphere())
	{
		if(IsMoored())
			applyGravity = false;		// Mooring overcomes all gravity.
		else if(GravityDriveWorking() && (!HasRocketDrive() || GravityDriveState() == GD_Hold))
		{
			// Maneuver drive is on and working. It is not a rocket drive or it is holding.
			if(!InBlackHoleGravity())
				applyGravity = false;	// Normal gravity conditions, not black hole.
			else
				brakesNegateGravity = true;	// Black hole gravity affects me when my brakes are not on.
		}
	}

	// Engine thrust factor per throttle setting.
	// This progression makes the throttle more controllable at low speed.
	const double engineThrust[11] = {0, .0025, .005, .01, .02, .04, .08, .16, .32, .64, 1.};

	// Determine if brake is on.
	AuPoint accel;
	bool brake, brakeLateral, brakeVertical;
	if(GravityDriveWorking())
	{
		// Maneuver drive is working so brakes can be applied.
		if(IsSpaceStation())
		{
			// Stations hold their position with constant brakes in case any movement is induced.
			// Station brakes are inadequate when inside atmosphere range of a planet.
			if(!IsInAtmosphere() && GravityDriveState() != GD_Off)
			{
				// Station brakes are always on when outside of atmosphere range of a planet.
				brake = true;
				brakeLateral = true;
				brakeVertical = true;
			}
			else
			{
				brake = false;
				brakeLateral = false;
				brakeVertical = false;
			}
		}
		else if(!IsMoored())
		{
			// Maneuver drive is working and this ship is not moored.
			// Trigger brakes applied by the pilot.
			if(m_wActions & ACT_Brake)
				brake = true;
			else
				brake = false;
			if(m_wActions & ACT_BrakeLateral)
				brakeLateral = true;
			else
				brakeLateral = false;
			if(m_wActions & ACT_BrakeVertical)
				brakeVertical = true;
			else
				brakeVertical = false;

			if(GravityDriveState() == GD_On)
			{
				// When unpiloted in gravity with maneuver drive 'on', autobrake to hold position if no waypoint is set.
				if(applyGravity &&														// In an area where gravity is applied.
						!brake && !brakeLateral && !brakeVertical &&					// No brakes are on.
						!ThrustLateral() && !ThrustLongitudinal() && !ThrustVertical() &&	// No throttles on.
						!IsOnGround() &&												// Not on the ground or a ship deck.
						!IsOnWater() &&													// Not floating on water.
						!HasWaypoint() &&												// No waypoints
						!IsPilotManned())												// Helm not occupied.
				{
					brake = true;
					brakeLateral = true;
					brakeVertical = true;
				}

				// Lateral acceleration.
				if(!brakeLateral && ThrustLateral())
				{
					if(ThrustLateral() < 0)
						accel.x = -fAccel * engineThrust[-ThrustLateral()];
					else
						accel.x = fAccel * engineThrust[ThrustLateral()];
				}

				// Forward/reverse acceleration.
				if(!brake && ThrustLongitudinal())
				{
					if(ThrustLongitudinal() < 0)
						accel.y = -fAccel * engineThrust[-ThrustLongitudinal()];
					else
						accel.y = fAccel * engineThrust[ThrustLongitudinal()];
				}

				// Vertical acceleration.
				if(!brakeVertical && ThrustVertical())
				{
					if(ThrustVertical() < 0)
						accel.z = -fAccel * engineThrust[-ThrustVertical()];
					else
						accel.z = fAccel * engineThrust[ThrustVertical()];
				}
			}
			else if(GravityDriveState() == GD_Hold)
			{
				// When in gravity with engine on 'hold', autobrake to hold position.
				if(applyGravity &&														// In an area where gravity is applied.
						!IsOnGround() &&												// Not on the ground or a ship deck.
						!IsOnWater())													// Not floating on water.
				{
					brake = true;
					brakeLateral = true;
					brakeVertical = true;
				}
			}
		}
		else
		{
			brake = false;
			brakeLateral = false;
			brakeVertical = false;
		}
	}
	else
	{
		brake = false;
		brakeLateral = false;
		brakeVertical = false;
	}

	// Brakes need to negate gravity in black hole systems.
	if(brakesNegateGravity && brake && brakeLateral && brakeVertical)
		applyGravity = false;

	// Add acceleration due to gravity.
	if(applyGravity)
		accel += DaCapsule::Acceleration();

	// Braking by the spacecraft must be applied after gravity in order to counteract it properly.
	AuPoint vec(Vector());
	if(!HasRocketDrive())
	{
		// Gravity drive simply ignores gravity.
		// Lateral braking.
		if(brakeLateral)
		{
			// Use engine braking to stop.
			const float maxBrakeThrust = fAccel * engineThrust[10];
			accel.x = -vec.x*2;	// brake with opposite force
			if(accel.x < 0)
			{
				if(accel.x < -maxBrakeThrust)
					accel.x = -maxBrakeThrust;
			}
			else
			{
				if(accel.x > maxBrakeThrust)
					accel.x = maxBrakeThrust;
			}
		}

		// Forward/reverse braking.
		if(brake)
		{
			// Use engine braking to stop.
			const float maxBrakeThrust = fAccel * engineThrust[10];
			accel.y = -vec.y*2;	// brake with opposite force
			if(accel.y < 0)
			{
				if(accel.y < -maxBrakeThrust)
					accel.y = -maxBrakeThrust;
			}
			else
			{
				if(accel.y > maxBrakeThrust)
					accel.y = maxBrakeThrust;
			}
		}

		// Vertical braking.
		if(brakeVertical)
		{
			// Use engine braking to stop.
			const float maxBrakeThrust = fAccel * engineThrust[10];
			accel.z = -vec.z*2;	// brake with opposite force
			if(accel.z < 0)
			{
				if(accel.z < -maxBrakeThrust)
					accel.z = -maxBrakeThrust;
			}
			else
			{
				if(accel.z > maxBrakeThrust)
					accel.z = maxBrakeThrust;
			}
		}
	}
	else
	{
		// This ship is equipped with rocket drives. Braking must counterract gravity.
		// Lateral braking.
		if(brakeLateral)
		{
			// Use engine braking to stop.
			// Calculate maximum braking on this axis.
			float brakeForce = fAccel * engineThrust[10];

			// Calculate braking force needed to reduce axis acceleration to zero.
			float nuaccel = accel.x + vec.x;				// New acceleration after 'accel' added to current vector.
			if(brakeForce > fabs(nuaccel))
				brakeForce = fabs(nuaccel);

			// Brake with opposite force vs nu acceleration.
			if(nuaccel > 0)
				accel.x -= brakeForce;
			else
				accel.x += brakeForce;
		}

		// Forward/reverse braking.
		if(brake)
		{
			// Use engine braking to stop.
			// Calculate maximum braking on this axis.
			float brakeForce = fAccel * engineThrust[10];

			// Calculate braking force needed to reduce axis acceleration to zero.
			float nuaccel = accel.y + vec.y;				// New acceleration after 'accel' added to current vector.
			if(brakeForce > fabs(nuaccel))
				brakeForce = fabs(nuaccel);

			// Brake with opposite force vs nu acceleration.
			if(nuaccel > 0)
				accel.y -= brakeForce;
			else
				accel.y += brakeForce;
		}

		// Vertical braking.
		if(brakeVertical)
		{
			// Use engine braking to stop.
			// Calculate maximum braking on this axis.
			float brakeForce = fAccel * engineThrust[10];

			// Calculate braking force needed to reduce axis acceleration to zero.
			float nuaccel = accel.z + vec.z;				// New acceleration after 'accel' added to current vector.
			if(brakeForce > fabs(nuaccel))
				brakeForce = fabs(nuaccel);

			// Brake with opposite force vs nu acceleration.
			if(nuaccel > 0)
				accel.z -= brakeForce;
			else
				accel.z += brakeForce;
		}
	}

	return accel;
}

AuPoint DaVessel::Friction() const
{
	double fricFwd = 0, fricLat = 0, fricVrt = 0;
	if(IsInAtmosphere())
	{
		// Inside atmosphere range of a planet.
		if(IsInVisibleWater())
		{
			// In water induces some drag.
			fricFwd = 20;
			fricLat = 20;
			fricVrt = 20;
		}
		/*
		else
		{
			AuPoint fric = DaCapsule::Friction();
			fricFwd = fric.y;
			fricLat = fric.x;
			fricVrt = fric.z;
		}
		*/
	}
	else if(GravityDriveWorking() && !InBlackHoleGravity())
	{
		// Outside atmosphere range of a planet and maneuver drives are
		// operating. Induce some friction if the throttles aren't on so
		// the ship will grind to a halt.
		// I could be inside a spacecraft in space.
		AuPoint vec(Vector());
		if(!ThrustLongitudinal() && fabs(vec.y) < 2)
			fricFwd = .5;
		if(!ThrustLateral() && fabs(vec.x) < 2)
			fricLat = .5;
		if(!ThrustVertical() && fabs(vec.z) < 2)
			fricVrt = .5;
	}

	// Determine friction due to ground surface contact.
	// Assumes I'm oriented upright to the surface.
	if(LandingOrientation() == LORI_Tail)
	{
		switch(OnGround())
		{
		case OG_None:	// Not on the ground so no additional drag from environment.
		case OG_WaterSoundOnly:	// Not really returned by OnGround().
			break;
		case OG_Forest:
			fricLat += 120;
			fricVrt += 120;
			break;
		case OG_Snow:
			fricLat += 170;
			fricVrt += 170;
			break;
		case OG_HardFloor:
			fricLat += 70;
			fricVrt += 70;
			break;
		case OG_Metal:
			fricLat += 70;
			fricVrt += 70;
			break;
		case OG_Pavement:
			fricLat += 70;
			fricVrt += 70;
			break;
		case OG_WoodFloor:
			fricLat += 70;
			fricVrt += 70;
			break;
		case OG_Rock:
			fricLat += 95;
			fricVrt += 95;
			break;
		case OG_Sand:
			fricLat += 95;
			fricVrt += 95;
			break;
		case OG_Savanna:
			fricLat += 95;
			fricVrt += 95;
			break;
		}
	}
	else
	{
		switch(OnGround())
		{
		case OG_None:	// Not on the ground so no additional drag from environment.
		case OG_WaterSoundOnly:	// Not really returned by OnGround().
			break;
		case OG_Forest:
			fricFwd += 30;
			fricLat += 120;
			break;
		case OG_Snow:
			fricFwd += 40;
			fricLat += 170;
			break;
		case OG_HardFloor:
			fricFwd += 20;
			fricLat += 70;
			break;
		case OG_Metal:
			fricFwd += 20;
			fricLat += 70;
			break;
		case OG_Pavement:
			fricFwd += 20;
			fricLat += 70;
			break;
		case OG_WoodFloor:
			fricFwd += 20;
			fricLat += 70;
			break;
		case OG_Rock:
			fricFwd += 25;
			fricLat += 95;
			break;
		case OG_Sand:
			fricFwd += 25;
			fricLat += 95;
			break;
		case OG_Savanna:
			fricFwd += 25;
			fricLat += 95;
			break;
		}
	}

	return AuPoint(fricLat, fricFwd, fricVrt);
}

// Add a visual and/or audio effect to this atom.
// 'effect' may be modified to something appropriate. e.g. VISUAL_HitPierce (looks like blood) is changed to VISUAL_HitPierceBug
// when this atom is an exoskeletal creature, and something else when this atom is not even a creature.
void DaVessel::AddEffect(AuEffect& effect)
{
	// Don't add bleeding effects to spacecraft.
	if(effect.Visual() == VISUAL_HitBludgeon)
	{
		effect.SetVisual(VISUAL_Explosion);
		effect.SetExplosionEffect1(WE_Bludgeon, 1);
	}
	else if(effect.Visual() == VISUAL_HitPierce)
	{
		effect.SetVisual(VISUAL_Explosion);
		effect.SetExplosionEffect1(WE_Pierce, 1);
	}
	else if(effect.Visual() == VISUAL_HitSlash)
	{
		effect.SetVisual(VISUAL_Explosion);
		effect.SetExplosionEffect1(WE_Slash, 1);
	}

	DaCapsule::AddEffect(effect);
}

// Return the assessed alert condition of the ship, based on the worse condition of the structural and non-structural systems.
ALERT_CONDITION DaVessel::AlertCondition() const
{
	if(MissionOrderType() == AuMissionOrder::Scuttle)
		return ConditionRed;	// The ship is ordered to self destruct.

	// Any visible condition is cause for alarm. e.g. fire, acid
	if(IsAnyConditionVisible())
		return ConditionRed;

	// Determine alert condition of structural systems.
	ALERT_CONDITION strCond = AlertConditionOfStructure();
	if(strCond == ConditionRed)
		return ConditionRed;	// It doesn't get any worse than this.

	// Determine alert condition of non-structural systems.
	ALERT_CONDITION sysCond = AlertConditionOfSystems();
	if(sysCond == ConditionRed)
		return ConditionRed;	// It doesn't get any worse than this.

	// Is either condition in the yellow?
	if(strCond == ConditionYellow || sysCond == ConditionYellow)
		return ConditionYellow;

	// Everything looks good.
	return ConditionGreen;
}

ALERT_CONDITION DaVessel::AlertConditionOfStructure() const
{
	// Everything is ok until we get the design.
	if(!IsDesignDataCacheValid())
		return ConditionGreen;

	// Determine the percent of damage to the structure.
	int pctRemaining;
	unsigned int maxHits = DesignHitsStructural();
	if(maxHits)
	{
		unsigned int dmgHits = HitsDamageStructural();
		unsigned int curHits;
		if(dmgHits < maxHits)
			curHits = maxHits - dmgHits;
		else
			curHits = 0;
		if(curHits == maxHits)
			pctRemaining = 100;
		else
			pctRemaining = Percent(curHits, maxHits);		// percent of remaining hit points
	}
	else
		pctRemaining = 0;

	// Determine the alert condition based on structural damage percentage.
	if(pctRemaining <= 10)
		return ConditionRed;
	else if(pctRemaining < 50)
		return ConditionYellow;
	return ConditionGreen;
}

ALERT_CONDITION DaVessel::AlertConditionOfSystems() const
{
	// Everything is ok until we get the design.
	if(!IsDesignDataCacheValid())
		return ConditionGreen;

	// Unbreathable air warrants a red alert condition.
	if(!IsInsideAirBreathable())
		return ConditionRed;	// It doesn't get any worse than this.

	// Determine the alert condition based on the most damage to any one system.
	unsigned int worstDamage = WorstSystemDamage();
	if(worstDamage >= 100)
		return ConditionRed;
	else if(worstDamage > 50)
		return ConditionYellow;
	return ConditionGreen;
}

void DaVessel::CombatArmorEffectiveness(WEAPONMODE mode, WEAPONEFFECT effect, bool hitShield, const AuPoint& wpnLoc,
		const AuPoint& hitLoc, float& baseAbsorb, float& baseBlock, float& auxAbsorb, float& auxBlock) const
{
	DaCapsule::CombatArmorEffectiveness(mode, effect, hitShield, wpnLoc, hitLoc, baseAbsorb, baseBlock, auxAbsorb, auxBlock);

	// Get base armor values from the hull.
	// Block value of hull armor module is multipled by armor level, in the range 1-11, not 0-10.
	COMMODITYID hullMod = SystemModule(SYS_Hull);
	quint8 hullQ = SystemQuality(SYS_Hull);
	float hullQMult = QualityBoostF(hullQ);
	const DbCommodity& hullC = g_Au.Commodity(hullMod);
	baseAbsorb += (hullC.ArmorModeAbsorb(mode) + hullC.ArmorEffectAbsorb(effect));
	baseBlock += (hullC.ArmorModeBlock(mode) + hullC.ArmorEffectBlock(effect)) * hullQMult * (ArmorLevel()+1);
}

void DaVessel::UnitKilled(DaUnit* victim)
{
	// This ship killed a unit.
	int rewardXp = victim->ExperiencePointsForKilling();
	if(EmpireId() != victim->EmpireId())
	{
		// The crew is rewarded if the unit is an enemy troop or person.
		// Spacecraft and vehicles are not considered 'units' when referring to rewards. Rewards for killing 'units' specifically
		// refer to combat dudes.
		if(!victim->IsCivilian() && dynamic_cast<const DaSentient*>(victim))
		{
			// Empire of victim is different from mine and the victim is a combat unit, either citizen or person.
			// Determine result based on political stance of my empire toward the victim's empire.
			if(PoliticalStanceMakeContact(victim) == POL_Enemy)
			{
				// Killed an enemy combat unit.
				// Get information about my empire.
				RewardShip(rewardXp, REW_KillUnit, victim);
			}
			else
				RewardShip(rewardXp, REW_None, victim);	// Unit is not an enemy.
		}
		else
			RewardShip(rewardXp, REW_None, victim);	// Unit is not considered a 'combat unit' for reward purposes.
	}
	else
		RewardShip(rewardXp, REW_None, victim);	// Unit from my empire is not an enemy.

	// Base class declares mutual enemies appropriately, based on empire id of this ship.
	DaCapsule::UnitKilled(victim);
}

// Toggle state of nearest large hull door.
void DaVessel::ToggleHullDoor(int doorIndex)
{
	if(doorIndex < 0 || IsDoorJammed(doorIndex))
		return;

	if(IsDoorOpen(doorIndex))
		CloseDoor(doorIndex);
	else
		OpenDoor(doorIndex);
}

void DaVessel::TestHullStress()
{
	// Certain tests have already been made.
	Q_ASSERT(IsInAtmosphere());
	Q_ASSERT(!IsInVehicle());
	Q_ASSERT(!IsInsideSpacecraft());

	float hullStress = HullStress();
	if(hullStress > 1)
	{
		if(hullStress <= 1.2f)
		{
			// Stress is enough to induce damage but not outright destruction.
			if(!IsPirateWithFreeWill())
			{
				// I am not a pirate. Notify the crew.
				QString text = tr("Damaged by excess hull stress.");
				AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
						Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::CrewMates);
				msg.SetScopeData(Id().GetID());	// Crew channel needs ship's id to know the scope of the message.
				g_Au.SendAtomChat(msg);
			}

			// Determine hits of damage in one second at this speed.
			unsigned int hits = DesignHitsTotal()/5 + 1;		// damage per second

			AuCombatAttacker attacker(tr("excess hull stress"));

			AuCombatWeapon weapon(WM_Internal);
			weapon.m_w1.m_Effect = WE_Sonic;
			weapon.m_w2.m_Effect = WE_Heat;

			AuCombatResult result;
			result.m_Damage1 = g_Au.Dice().Roll(1,hits);
			result.m_Damage2 = g_Au.Dice().Roll(1,hits);

			CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
			//if(!IsNormal())
			//{
				//qDebug("DaVessel::TestHullStress %d destroyed by excess "
						//"hull stress %g.", Id().GetID(), hullStress);
			//}
		}
		else
		{
			// Stress is enough to flat-out destroy the ship.
			DamageAndDestroy(tr("Disintegrated by extreme hull stress."));
			//qDebug("DaVessel::TestHullStress %d destroyed by extreme hull stress %g.", Id().GetID(), hullStress);
		}
	}

	// Atmospheric hull is immune to buckling under atmospheric pressure.
	if(!IsCGSubmerged() && !IsUnderGround() && HullStreamline() < SLINE_Atmospheric && AirDensity() > .95f)
	{
		if(!IsPirateWithFreeWill())
		{
			// I am not a pirate. Notify the crew.
			QString text = tr("Hull buckling due to atmospheric pressure.");
			AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
					Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::CrewMates);
			msg.SetScopeData(Id().GetID());	// Crew channel needs ship's id to know the scope of the message.
			g_Au.SendAtomChat(msg);
		}

		// High atmospheric pressure. Induce 10% damage per second.
		AuCombatAttacker attacker(tr("atmospheric pressure"));

		AuCombatWeapon weapon(WM_Internal);
		weapon.m_w1.m_Effect = WE_Crush;

		AuCombatResult result;
		result.m_Damage1 = DesignHitsStructural()/10;

		CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
		SetModified();
	}
}

// Trade range for stations.
float DaVessel::StationTradeRange() const
{
	quint8 cncQ = SystemQuality(SYS_CommandAndControl);
	if(!cncQ)
		cncQ = 1;
	return RoughRadius() + TradeRange(cncQ);
}

// Trade range for stations.
float DaVessel::StationTradeRange(const DaVessel* otherShip) const
{
	return StationTradeRange() + otherShip->RoughRadius();
}

float DaVessel::StationParkingRange() const
{
	float tradeRange = StationTradeRange();					// This can be very big for a large high Q station.
	float parkRange = RoughRadius() + 100*FeetPerMeter();	// 100m wingtip to wingtip is a decent EVA range and safe for the pilot.
	if(tradeRange < parkRange)
		return tradeRange;									// Must at least be in trade range of the station.
	return parkRange;										// Return our cozy parking range for easy EVA.
}

float DaVessel::StationParkingRange(const DaVessel* otherShip) const
{
	return StationParkingRange() + otherShip->RoughRadius();
}

void DaVessel::SetAtRest()
{
	DaCapsule::SetAtRest();
	Orient(Location(), Transform().ProjectVector(TerrainNormal()));	// Instant orient when at rest.
}

void DaVessel::SpawnArenaCrewman(bool spawnOfficer, bool spawnCrew, bool spawnTroops, bool spawnPassengers)
{
	// Come up with some DNA for the nu dudes.
	//DHH animalia
	AuSpecies crewSpecies;
	const DaSentient* officer = FindOfficer();
	if(officer)
		crewSpecies = officer->Species();
	else
	{
		const DaPerson* person = FindPersonOnLineRecursive();
		if(person)
			crewSpecies = person->Species();
		else
		{
			AuDNA dna4;
			dna4.InitDefaultHuman();
			crewSpecies = dna4.Species();
		}
	}

	if(spawnOfficer && !HasFirstOfficer() && OfficerBerthsAvailable())
	{
		DaNpc* officer = LoadPirateOfficer(EmpireId(), crewSpecies);
		if(FindPersonOnLineRecursive())
			officer->ReleaseTheCon();
		officer->SetArenaTeam(EmpireId());
	}
	else if(spawnCrew && CrewBerthsAvailable())
	{
		DaUnit* unit = LoadPirateCrew(EmpireId(), crewSpecies);
		unit->SetArenaTeam(EmpireId());
	}
	else if(spawnTroops && TroopBerthsAvailable())
	{
		DaUnit* unit = LoadPirateTroop(EmpireId(), crewSpecies);
		unit->SetArenaTeam(EmpireId());
	}
	else if(spawnPassengers && PassengerBerthsAvailable())
	{
		DaUnit* unit = LoadPiratePassenger(EmpireId(), crewSpecies);
		unit->SetArenaTeam(EmpireId());
	}
}

double DaVessel::ShieldUnitsWorking() const
{
	if(!ShieldUnits())
		return 0;
	return GetPowerConsumption(SYS_Shield);
}

void DaVessel::SetName(const QString& name)
{
	if(AssignedName() != name)
	{
		DaCapsule::SetName(name);
		m_NameWithEmpireLock.lockForWrite();
		m_NameWithEmpire = "";
		m_NameWithEmpireLock.unlock();

		// Emit a signal to update people who hold berths aboard the ship. Only do this if any berths are taken.
		m_RosterLock.lockForRead();
		if(!m_Roster.isEmpty())
			emit SpacecraftNameChanged(Id(), name);
		m_RosterLock.unlock();
	}
}

// Splits a reward between all ship crew members that are aboard right now and on line.
// This includes the captain, officers, crew, and troops in the roster.
// This does not include passengers or other people aboard.
// The 'rewardXp' is split between crew members aboard, minimum 1 per dude if rewardXp is > 0.
// 'unitKilled' may be 0.
void DaVessel::RewardShip(int rewardXp, REWARDTYPE rewardType, DaUnit* unitKilled)
{
	if(rewardXp > 0 || unitKilled)
	{
		// Find people and troops aboard the ship.
		DaAtomList crew;
		FindPeopleAwakeRecursive(crew);					// List all people aboard that are online.

		// Player Statistics
		if(rewardType == REW_ExploreWormhole)
			for(DaAtomList::const_iterator pi = crew.begin(); pi != crew.end(); pi++)
				((DaPerson*)*pi)->Stats().WormholeExplored();

		// Remove people from the crew list who are not on the roster.
		DaAtomList::iterator itRoster = crew.begin();
		while(itRoster != crew.end())
		{
			// Found a person aboard who is on line.
			DaAtom* person = *itRoster;
			if(IsCrewOrTroop(person->Id()))
				itRoster++;									// The person is a crew member of this ship.
			else if(InChainOfCommand(person))
				itRoster++;									// The person is in the chain-of-command of this ship.
			else
				itRoster = crew.erase(itRoster);			// The person is not a crew member of this ship.
		}

		// Unit kills may trigger story/job events for people.
		if(unitKilled)
			for(DaAtomList::const_iterator itRoster = crew.begin(); itRoster != crew.end(); itRoster++)
			{
				DaPerson* person = dynamic_cast<DaPerson*>(*itRoster);
				if(person)
					unitKilled->KilledByPerson(person);
			}

		if(rewardXp > 0)
		{
			// Add officers, troops and crew members who are assigned to this ship to the crew list.
			// Other crew and troops don't get xp just because they're aboard; they will get xp for their individual kills.
			FindAIPeopleRecursive(Id(), crew);					// Add my officers aboard to the crew list.
			FindOfficersRecursive(Id(), crew);					// Add my officers aboard to the crew list.
			FindCrewRecursive(Id(), crew);						// Add my crew members aboard to the crew list.
			FindTroopsRecursive(Id(), crew);					// Add my troops aboard to the crew list.

			// Reward the crew of this ship
			if(crew.count())
			{
				// AI dudes do share in the xp.
				int shareXp;
				if(rewardXp)
				{
					if(rewardXp > crew.count())
						shareXp = rewardXp/crew.count();
					else
						shareXp = 1;							// minimum of 1 xp
				}
				else
					shareXp = 0;

				AuSkill::SKILL skill;
				switch(rewardType)
				{
				case REW_DamageStation:
				case REW_DamageSpaceship:
				case REW_DamageStarship:
					skill = AuSkill::SpacecraftCombat;
					break;

				case REW_ExploreWormhole:
					skill = AuSkill::SpaceExploration;
					break;

				case REW_KillUnit:
					skill = AuSkill::Combat;
					break;

				case REW_None:
					// I get this when I kill a non-enemy unit.
					skill = AuSkill::Invalid;
					break;

				case REW_CaptureBase:
				case REW_CaptureCapitalEmpire:
				case REW_CaptureCapitalGalaxy:
				case REW_CaptureCapitalSector:
				case REW_CaptureCity:
				case REW_CaptureSpaceship:
				case REW_CaptureStarship:
				case REW_CaptureStation:
				case REW_ColonizeHabitableWorld:
				case REW_ColonizeUninhabitableWorld:
				case REW_OrderComplete:
				case REW_Salary:
				case REW_SpotCity:
				case REW_SpotEnemy:
				default:
					// These reward types are not passed to this function.
					skill = AuSkill::Invalid;
					break;
				}

				// Divvy out the shares.
				for(DaAtomList::const_iterator itRoster = crew.begin(); itRoster != crew.end(); itRoster++)
				{
					DaPerson* person = dynamic_cast<DaPerson*>(*itRoster);
					if(person)
					{
						// Person who is on the roster.
						if(!person->IsAIControlled())
							person->Reward(0, rewardType);
						if(shareXp && skill != AuSkill::Invalid)
							person->IncreaseSkill(skill, shareXp);
					}
					else if(shareXp && skill != AuSkill::Invalid)
					{
						// Must be a citizen that is a member of my crew.
						// Crew only accumulate xp, not money.
						DaSentient* citizen = dynamic_cast<DaSentient*>(*itRoster);
						if(citizen)
							citizen->IncreaseSkill(skill, shareXp);
					}
				}
			}
		}
	}
}

// All stations with transporter rooms monitor the Hail channel and and a channel named after them so they can answer
// beam-up requests.
void DaVessel::RxChatMessage(const AuCommPacket* msg, quint32& /*countSupplied*/, bool /*firstInGroup*/)
{
	if(InDesignEnvironment())
		return;
	if(IsDormant())
	{
		if(IsMoored())
			return;	// Ships in hangars do not listen.
		else if(msg->Type() != AuCommPacket::HailRequest)
			return;	// Ships in stasis only listen for hail requests on the crew channel.
	}
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return;

	// All chat replies need my sector id. Insure I can get it.
	const DaSector* sector = system->FindParentSector();
	if(!sector)
		return;
	const DaGalaxy* galaxy = sector->FindParentGalaxy();
	if(!galaxy)
		return;

	// Locate the sender.
	// Sender atom will not be found when they are not in this solar system. That's ok.
	// SOS messages are sent on the Hail channel as Chat messages from ships who need fuel or power, just so I can log them now.
	DaAtom* sender = system->FindIdRecursive(msg->FromId());
	DaPerson* person = 0;
	DaVessel* senderShip = 0;
	if(sender)
	{
		person = dynamic_cast<DaPerson*>(sender);
		if(!person)
			senderShip = dynamic_cast<DaVessel*>(sender);
	}
	if(senderShip == this)
		return;													// Ignore messages from myself.

	if(msg->Scope() == AuCommStdChannel::CrewMates && !IsCrewOrTroop(msg->FromId()))
	{
		// Sender should not be transmitting on my crew channel.
		if(Is(AuId(msg->ScopeData())))
		{
			// The sender seems to think they have a berth aboard this ship. Correct them.
			emit ReleaseBerthSignal(msg->FromId(), AuId());		// Releases their berth from all ships.
		}
		else
			qWarning() << "error: DaVessel::RxChatMessage" << *this << "rx'd CrewMates msg with bad scope data" << msg->ScopeData();
	}

	// Determine stance toward message sender.
	POLITICAL_STANCE stanceToSender;							// My stance toward the sender.
	DaVessel* ship = 0;										// The sender person's ship, if any.
	if(person)
	{
		if(person->IsInsideSpacecraft() && !IsCrewOrTroop(msg->FromId()) && !InChainOfCommand(msg->FromId(), msg->FromEmpireId()))
		{
			// The sender is aboard a spacecraft and they are not one of my crew members and I am not in their personal c-o-c.
			// My stance is based on the owner of their ship.
			ship = person->FindParentVesselInEnv();
			if(!ship)
				return;
			stanceToSender = PoliticalStanceMakeContact(ship);
		}
		else
		{
			// The sender is not aboard a spacecraft or they are one of my crew members or I am in their chain of command.
			// My stance is based on the person.
			stanceToSender = PoliticalStanceMakeContact(person);
		}
	}
	else if(senderShip)
	{
		// Sender is a spacecraft.
		// Test political stance toward the ship.
		stanceToSender = PoliticalStanceMakeContact(senderShip);
	}
	else if(msg->Scope() == AuCommStdChannel::CrewMates)
	{
		// Crew message from a great distance because the person was not found in this solar system.
		// Crew members could be as low as neutral stance. That's good enough for now; this guess could be improved based on their
		// berth type.
		stanceToSender = POL_Neutral;
	}
	else if(msg->Scope() == AuCommStdChannel::Fleet && InChainOfCommand(msg->FromId(), msg->FromEmpireId()))
	{
		// Fleet message from a great distance because the person was not found in this solar system.
		stanceToSender = POL_Empire;
	}
	else if(msg->Scope() == AuCommStdChannel::Company && InChainOfCommand(msg->FromId(), EmpireId()))
	{
		// Fleet message from a great distance because the person was not found in this solar system.
		stanceToSender = POL_Empire;
	}
	else
	{
		// The sender is not in my solar system. I really only care about fleet messages and mission orders at this range.
		// Fleet scope messages and mission orders will have an empire id associated with them.
		quint32 fromEmpire = msg->FromEmpireId();
		if(fromEmpire == 0)
			return;												// The message has no empire id so ignore it.
		stanceToSender = PoliticalStance(EmpireId(), fromEmpire);
	}

	// Log the reception of all in-system messages from enemy units and log chat messages from all units.
	// Do not log messages received from distant solar systems. The sender atom is 0 if it's from another solar system.
	// SOS messages are sent on the Hail channel as Chat messages from ships who need fuel or power, just so I can log them now.
	if((person || senderShip) &&												// Sender is a person or ship in this system.
			(ship != this) &&													// Sender is not aboard this ship.
			(stanceToSender == POL_Enemy || msg->Type() == AuCommPacket::Chat) &&	// Enemy sender or Chat message.
			!IsPirateWithFreeWill() &&											// Pirates do not log events.
			!IsDormant())
	{
		// Log the reception of this message.
		AuEventLogEntry::TYPE type;
		switch(stanceToSender)
		{
		case POL_Friend:
			type = AuEventLogEntry::FriendComm;
			break;
#if defined(QT_NO_DEBUG)
		default:						// Eliminate compiler warning in release build.
#endif
		case POL_Neutral:
			type = AuEventLogEntry::NeutralComm;
			break;
		case POL_Enemy:
			type = AuEventLogEntry::EnemyComm;
			break;
		case POL_Empire:
		case POL_Vassal:
			type = AuEventLogEntry::EmpireComm;
			break;
		}

		QString text;
		if(senderShip)
		{
			text = tr("%1: %2")
				.arg(senderShip->NameWithEmpire())
				.arg(msg->Text());
		}
		else if(ship)
		{
			text = tr("%1 <i>(%2)</i> aboard %3: %4")
				.arg(person->Name())
				.arg(person->Id().StringId())
				.arg(ship->NameWithEmpire())
				.arg(msg->Text());
		}
		else
		{
			QString ename;
			const DgData& gd = g_Au.LockDataForRead();
			gd.EmpireName(person->EmpireId(), ename);
			g_Au.UnlockData();

			text = tr("%1 %2 <i>(%3)</i>: %4")
				.arg(ename)
				.arg(person->Name())
				.arg(person->Id().StringId())
				.arg(msg->Text());
		}

		LogEvent(type, text);
	}

	// Do not communicate with enemies.
	if(stanceToSender == POL_Enemy)
	{
		// Pirates simply ignore enemies. Pirates do not post assassination missions.
		if(IsPirateWithFreeWill())
			return;

		if(HasFirstOfficer() && g_Au.m_EnableJobPosting && !InArena())
		{
			// Post a job offer to assassinate enemy politicians.
			if(person && !person->m_AgentBits)
			{
				// Government officials are targeted for assassination.
				// Get empire record to determine the empire's posted rewards.
				const DgData& gd = g_Au.LockDataForRead();
				quint32 targetValue = gd.EmpireCitizenTargetValue(person->EmpireId(), person->Id());
				g_Au.UnlockData();
				if(targetValue > 1)
				{
					quint32 reward = targetValue;	// Scale reward based on value of the target.
					QString lastSeen = tr("%1, %2 %3, %4")
						.arg(system->Name())
						.arg(sector->Name()).arg(sector->Description())
						.arg(galaxy->Name());
					AuJobAssassin job(person->Id(), person->Name(), lastSeen, reward);
					emit PostJobOffer(AuJobOffer(Name(), EmpireId(), sector->Location(), galaxy->Galaxy(), &job));
				}
			}

			// Post a job to interdict enemy spacecraft in this system.
			if(ship)
			{
				AuJobInterdictSystem job(ship->EmpireId(), system->Id(),
						system->Name(), sector->Name() + " " + sector->Description() + ", " + galaxy->Name());
				emit PostJobOffer(AuJobOffer(Name(), EmpireId(), sector->Location(), galaxy->Galaxy(), &job));
			}
			else if(senderShip)
			{
				AuJobInterdictSystem job(senderShip->EmpireId(), system->Id(),
						system->Name(), sector->Name() + " " + sector->Description() + ", " + galaxy->Name());
				emit PostJobOffer(AuJobOffer(Name(), EmpireId(), sector->Location(), galaxy->Galaxy(), &job));
			}
		}

		// We are done with messages received from distant enemy units.
		if(!sender)
			return;

		// Allow enemy agent messages to get through.
		if(!person || !person->HasAgentPower())
			return;
	}

	// We only got this far listening to senderShip so we could log messages from spacecraft, particularly SOS messages from ships.
	if(senderShip)
		return;

	// Ships normally respond at the same range the message was sent.
	// Some responses are limited to solar system range.
	AuCommStdChannel::RANGE limitedRange = msg->ReceptionRange();		// Range that will be used for limited range responses.
	if(limitedRange > AuCommStdChannel::SolarSystem)
		limitedRange = AuCommStdChannel::SolarSystem;

	switch(msg->Type())
	{
	case AuCommPacket::RequestInventory:
		RxRequestInventory(dynamic_cast<const AuCommRequestInventory*>(msg), sector->Id(), msg->ReceptionRange(), person);
		break;

	case AuCommPacket::MissionNew:
		RxMissionNew(dynamic_cast<const AuCommMissionNew*>(msg));
		break;

	case AuCommPacket::MissionProperty:
		RxMissionProperty(dynamic_cast<const AuCommMissionProperty*>(msg));
		break;

	case AuCommPacket::MissionOrderInsert:
		RxMissionOrderInsert(dynamic_cast<const AuCommMissionOrderInsert*>(msg));
		break;

	case AuCommPacket::MissionOrderAdd:
		RxMissionOrderAdd(dynamic_cast<const AuCommMissionOrderAdd*>(msg));
		break;

	case AuCommPacket::MissionOrderNew:
		RxMissionOrderNew(dynamic_cast<const AuCommMissionOrderNew*>(msg));
		break;

	case AuCommPacket::BeamOver:
		RxBeamOver(dynamic_cast<const AuCommBeamOver*>(msg), person, stanceToSender, limitedRange);
		break;

	case AuCommPacket::BeamVehicleAtom:
		RxBeamVehicleAtom(dynamic_cast<const AuCommBeamVehicleAtom*>(msg), limitedRange);
		break;

	case AuCommPacket::BeamVehicleCoordinate:
		RxBeamVehicleCoordinate(dynamic_cast<const AuCommBeamVehicleCoordinate*>(msg), limitedRange);
		break;

	case AuCommPacket::BeamVehiclePlanet:
		RxBeamVehiclePlanet(dynamic_cast<const AuCommBeamVehiclePlanet*>(msg), limitedRange);
		break;

	case AuCommPacket::BeamVehicleShip:
		RxBeamVehicleShip(dynamic_cast<const AuCommBeamVehicleShip*>(msg), person, limitedRange);
		break;

	case AuCommPacket::BeamVehicleTransporter:
		RxBeamVehicleTransporter(dynamic_cast<const AuCommBeamVehicleTransporter*>(msg), person, limitedRange);
		break;

	case AuCommPacket::ContactShip:
		RxContactShip(dynamic_cast<const AuCommContactShip*>(msg));
		break;

	case AuCommPacket::HailRequest:
		RxHailRequest(dynamic_cast<const AuCommHailRequest*>(msg), person, limitedRange, sector->Id());
		break;

	case AuCommPacket::RequestBerthAny:
		RxRequestBerthAny(dynamic_cast<const AuCommRequestBerthAny*>(msg), person);
		break;

	case AuCommPacket::RequestBerthCaptain:
		RxRequestBerthCaptain(dynamic_cast<const AuCommRequestBerthCaptain*>(msg), person);
		break;

	case AuCommPacket::RequestBerthCaptainSlot:
		RxRequestBerthCaptainSlot(dynamic_cast<const AuCommRequestBerthCaptainSlot*>(msg), person);
		break;

	case AuCommPacket::RequestBerthOfficer:
		RxRequestBerthOfficer(dynamic_cast<const AuCommRequestBerthOfficer*>(msg), person);
		break;

	case AuCommPacket::RequestBerthCrew:
		RxRequestBerthCrew(dynamic_cast<const AuCommRequestBerthCrew*>(msg), person);
		break;

	case AuCommPacket::RequestBerthPassenger:
		RxRequestBerthPassenger(dynamic_cast<const AuCommRequestBerthPassenger*>(msg), person);
		break;

	case AuCommPacket::RequestBerthTroop:
		RxRequestBerthTroop(dynamic_cast<const AuCommRequestBerthTroop*>(msg), person);
		break;

	case AuCommPacket::Jettison:
		RxJettison(dynamic_cast<const AuCommJettison*>(msg), limitedRange, sector->Id(), person);
		break;

	case AuCommPacket::OpenTEU:
		RxOpenTEU(dynamic_cast<const AuCommOpenTEU*>(msg), limitedRange, sector->Id(), person);
		break;

	case AuCommPacket::Combine:
		RxCombine(dynamic_cast<const AuCommCombine*>(msg), limitedRange, sector->Id(), person);
		break;

	default:
		break;
	}
}

void DaVessel::RxBeamOver(const AuCommBeamOver* msg, DaPerson* person, POLITICAL_STANCE stanceToSender,
		AuCommStdChannel::RANGE limitedRange)
{
	if(!msg)
		return;

	if(!person)
		return;

	if(CheckAccess(person))
	{
		// The sender is a crew member on this ship.
		// Or this ship is in the person's chain-of-command.
		// Passengers must request beam over while ship is boarding.
		int requested = msg->Count();
		int result;
		if(TransporterSystems())
		{
			// Perform the beam up.
			// When the beam target is a person aboard a ship, we often get collisions. This is the only case that returns
			// a zero result. Let's try it more than once so the user doesn't have to ask us to every time.
			for(int itry=0; itry<100; itry++)
			{
				result = BeamUp(person, requested);
				if(result != 0)
					break;	// All non-zero results must be accepted. Keep trying when the result is zero.
			}
		}
		else
		{
			result = -20;	// Not equipped with transporters.
		}

		// Compose a reply, good or bad.
		// This reply is coming from the ship's computer, or Mr. Spock. Make it a bit mechanical.
		QString text;
		if(person->IsAboardVehicle())
		{
			if(result == 1)
				text = tr("Vehicle transporter energized. Welcome back sir.");
			else if(result == 0)
				text = tr("Sorry sir, vehicle transporter beam was blocked. Would you like me to try again?");
			else if(result == -1)
				text = tr("Sorry sir, vehicle transporters are not available.");
			else if(result == -2)
				text = tr("Sorry sir, you are beyond vehicle transporter range.");
			else if(result == -3)
				text = tr("Sorry sir, vehicle transporter cannot lock on at your present location.");
			else if(result == -4)
				text = tr("Sorry sir, insufficient power to energize vehicle transporter.");
			else if(result == -5)
				text = tr("Sorry sir, vehicle transporter has not completed its activation cycle.");
			else if(result == -6)
				text = tr("Sorry sir, vehicle transporter has no empty parking spots for your vehicle.");
			else if(result == -10)
				text = tr("Sorry sir, shields on your ship prevent vehicle transport.");
			else if(result == -11)
				text = tr("Sorry sir, your vehicle is not in an appropriate vehicle transporter parking spot.");
			else if(result == -12)
				text = tr("Sorry sir, vehicle transporter shows ambiguous life form reading, too dangerous to transport.");
			else if(result == -13)
				text = tr("Sorry sir, a shield generator in your area prevents vehicle transport.");
			else if(result == -14)
				text = tr("Sorry sir, we are not equipped with a vehicle transporter.");
			else if(result == -15)
				text = tr("Sorry sir, we have no vacant parking spots for your vehicle.");
			else if(result == -16)
				text = tr("Sorry sir, your ship is not equipped with a vehicle transporter.");
			else if(result == -17)
				text = "";	// Person is standing in the destination transporter room.
			else if(result == -20)
				text = tr("Sorry sir, we are not equipped with a transporter.");
			else
				text = tr("Sorry sir, unable to energize vehicle transporter.");
		}
		else
		{
			if(result == requested && requested == 1)
				text = tr("Transporter energized. Welcome back sir.");
			else if(result == requested)
				text = tr("Transporter energized for %1 to beam aboard. Welcome back sir.").arg(result);
			else if(result > 0)
				text = tr("Transporter could only be energized for %1 to beam aboard. Welcome back sir.").arg(result);
			else if(result == 0)
				text = tr("Sorry sir, transporter beam was blocked. Would you like me to try again?");
			else if(result == -1)
				text = tr("Sorry sir, transporters are not available.");
			else if(result == -2)
				text = tr("Sorry sir, you are beyond transporter range.");
			else if(result == -3)
				text = tr("Sorry sir, transporter cannot lock on at your present location.");
			else if(result == -4)
				text = tr("Sorry sir, insufficient power to energize transporter.");
			else if(result == -5)
				text = tr("Sorry sir, transporter has not completed its activation cycle.");
			else if(result == -10)
				text = tr("Sorry sir, shields on your ship prevent transport. Turn off the "
						"shields or enter a transporter room and turn it on.");
			else if(result == -11)
				text = tr("Sorry sir, your vehicle is not in an appropriate vehicle transporter parking spot.");
			else if(result == -12)
				text = tr("Sorry sir, transporter shows ambiguous life form reading, too dangerous to transport.");
			else if(result == -13)
				text = tr("Sorry sir, a shield generator in your area prevents transport. Turn off the "
						"shields or enter a transporter room and turn it on.");
			else if(result == -14)
				text = tr("Sorry sir, we are not equipped with a vehicle transporter.");
			else if(result == -17)
				text = "";	// Person is standing in the destination transporter room.
			else if(result == -20)
				text = tr("Sorry sir, we are not equipped with a transporter.");
			else
				text = tr("Sorry sir, unable to energize transporter.");
		}

		// Send the response. This could only be on the Crew channel.
		TxReplyChat(text, *msg, limitedRange);
	}
	else if(IsSpaceStation())
	{
		// The sender is not a crew member or in CoC of this station.
		int requested = msg->Count();
		int result;
		if(!AnyHullDoorUnlocked())
			result = -21;	// Do not beam non-crew/non-CoC aboard if hull doors are locked.
		else if(TransporterSystems())
		{
			// Perform the beam up.
			// When the beam target is a person aboard a ship, we often get collisions. This is the only case that returns
			// a zero result. Let's try it more than once so the user doesn't have to ask us to every time.
			for(int itry=0; itry<100; itry++)
			{
				result = BeamUp(person, requested);
				if(result != 0)
					break;	// All non-zero results must be accepted. Keep trying when the result is zero.
			}
		}
		else
		{
			result = -20;	// Not equipped with transporters.
		}

		// Compose a reply, good or bad.
		// The reply is coming from the transporter operator aboard this station. Give it a little personality.
		QString text;
		if(person->IsAboardVehicle())
		{
			if(result == 1)
				text = tr("I energized the station's vehicle transporter. Welcome aboard.");
			else if(result == 0)
				text = tr("The station's vehicle transporter beam was blocked. You must be in a cramped compartment.");
			else if(result == -1)
				text = tr("Sorry, no vehicle transporters on the station are available.");
			else if(result == -2)
			{
				// Out of range.
				// Don't even answer if we're not at the same planet. Nobody should expect beam over at this range.
				if(person->FindParentPlanetInEnv() == FindParentPlanetInEnv())
					text = tr("You are outside my vehicle transporter range.");
				else
					return;	// Send no reply.
			}
			else if(result == -3)
				text = tr("I can't get the station's vehicle transporter to lock on to you.");
			else if(result == -4)
				text = tr("The station doesn't have enough power to energize my vehicle transporter. "
						"We're having some trouble here.");
			else if(result == -5)
				text = tr("The station's vehicle transporter has not finished its activation cycle. "
						"Please try again in a few seconds.");
			else if(result == -6)
				text = tr("The station's vehicle transporter has no empty parking spots for your vehicle.");
			else if(result == -10)
				text = tr("The shields on your ship are blocking the station's vehicle transporter.");
			else if(result == -11)
				text = tr("Your vehicle is not in an appropriate vehicle transporter parking spot.");
			else if(result == -12)
				text = tr("Another life form is too close to transport your vehicle aboard the station. Are you "
						"riding an animal or did you just get eaten?");
			else if(result == -13)
				text = tr("A shield generator in your area is blocking the station's vehicle transporter.");
			else if(result == -14)
				text = tr("Sorry, this station is not equipped with a vehicle transporter.");
			else if(result == -15)
				text = tr("Sorry sir, we have no vacant parking spots for your vehicle.");
			else if(result == -16)
				text = tr("Sorry sir, your ship is not equipped with a vehicle transporter.");
			else if(result == -17)
				text = "";	// Person is standing in the destination transporter room.
			else if(result == -20)
				text = tr("Sorry, this station is not equipped with a transporter.");
			else if(result == -21)
				text = tr("I am not authorized to beam your vehicle aboard this station.");
			else
				text = tr("The station's vehicle transporter won't energize. I can't explain why; it's some kind of anomaly.");
		}
		else
		{
			if(result == requested && requested == 1)
				text = tr("I energized the station's transporter. Welcome aboard.");
			else if(result == requested)
				text = tr("I energized the station's transporter for %1 to beam aboard. Welcome aboard.").arg(result);
			else if(result > 0)
				text = tr("I could only energize the station's transporter for %1 to beam aboard. Welcome aboard.").arg(result);
			else if(result == 0)
				text = tr("The station's transporter beam was blocked. You must be in a cramped compartment.");
			else if(result == -1)
				text = tr("Sorry, no transporters on the station are available.");
			else if(result == -2)
			{
				// Out of range.
				// Don't even answer if we're not at the same planet. Nobody should expect beam over at this range.
				if(person->FindParentPlanetInEnv() == FindParentPlanetInEnv())
					text = tr("You are outside my transporter range.");
				else
					return;	// Send no reply.
			}
			else if(result == -3)
				text = tr("I can't get the station's transporter to lock on to you.");
			else if(result == -4)
				text = tr("The station doesn't have enough power to energize my transporter. We're having some trouble here.");
			else if(result == -5)
				text = tr("The station's transporter has not finished its activation cycle. Please try again in a few seconds.");
			else if(result == -10)
				text = tr("The shields on your ship are blocking the station's transporter. Turn off your "
						"shields or go to a transporter room and turn it on.");
			else if(result == -11)
				text = tr("Your vehicle is not in an appropriate vehicle transporter parking spot.");
			else if(result == -12)
				text = tr("Another life form is too close to transport you aboard the station. Are you "
						"riding an animal or did you just get eaten?");
			else if(result == -13)
				text = tr("A shield generator in your area is blocking the station's transporter. Turn off the "
						"shields or go to a transporter room and turn it on.");
			else if(result == -14)
				text = tr("Sorry, this station is not equipped with a vehicle transporter.");
			else if(result == -17)
				text = "";	// Person is standing in the destination transporter room.
			else if(result == -20)
				text = tr("Sorry, this station is not equipped with a transporter.");
			else if(result == -21)
				text = tr("I am not authorized to beam you aboard this station.");
			else
				text = tr("The station's transporter won't energize. I can't explain why; it's some kind of anomaly.");
		}
		TxReplyChat(text, *msg, limitedRange);
	}
	else if(IsCommandedByAI())
	{
		// This is not a station and the sender is not a crew member or in CoC.
		// AI commander must decide whether to beam them aboard.
		// The sender is neutral, friend, or empire; we've already skipped messages from enemy senders.
		bool okToBeamThemUp = false;	// Set true if it's ok to beam up the person requesting transport.
		if(MissionOrderType() == AuMissionOrder::CrewLoad || MissionOrderType() == AuMissionOrder::PassengersLoad)
			okToBeamThemUp = true;		// Ok when the mission order is to board passengers or crew.
		else if(MissionOrderType() == AuMissionOrder::TroopsLoad &&
			 (stanceToSender == POL_Empire || stanceToSender == POL_Vassal))
			okToBeamThemUp = true;		// Ok when the mission order is to board troops and the sender is my empire or overlord.
		//else if(!HasHumanOfficer() &&
				//(stanceToSender == POL_Empire || stanceToSender == POL_Friend || stanceToSender == POL_Vassal))
			//okToBeamThemUp = true;		// Ok when no human captain and sender is a friend, empire or overlord.
		if(okToBeamThemUp)
		{
			int requested = msg->Count();
			int result;
			if(TransporterSystems())
			{
				// Perform the beam up.
				// When the beam target is a person aboard a ship, we often get collisions. This is the only case that returns
				// a zero result. Let's try it more than once so the user doesn't have to ask us to every time.
				for(int itry=0; itry<100; itry++)
				{
					result = BeamUp(person, requested);
					if(result != 0)
						break;	// All non-zero results must be accepted. Keep trying when the result is zero.
				}
			}
			else
			{
				result = -20;	// Not equipped with transporters.
			}

			// Compose a reply, good or bad.
			// The reply is coming from the transporter operator aboard this station. Give it a little personality.
			QString text;
			if(person->IsAboardVehicle())
			{
				if(result == 1)
					text = tr("I energized the ship's vehicle transporter. Welcome aboard.");
				else if(result == 0)
					text = tr("The ship's vehicle transporter beam was blocked. You must be in a cramped compartment.");
				else if(result == -1)
					text = tr("Sorry, no vehicle transporters on the ship are available.");
				else if(result == -2)
				{
					// Out of range.
					// Don't even answer if we're not at the same planet. Nobody should expect beam over at this range.
					if(person->FindParentPlanetInEnv() == FindParentPlanetInEnv())
						text = tr("You are outside my vehicle transporter range.");
					else
						return;	// Send no reply.
				}
				else if(result == -3)
					text = tr("I can't get the ship's vehicle transporter to lock on to you.");
				else if(result == -4)
					text = tr("The ship doesn't have enough power to energize my vehicle transporter. "
							"We're having some trouble here.");
				else if(result == -5)
					text = tr("The ship's vehicle transporter has not finished its activation cycle. Try again in a few seconds.");
				else if(result == -6)
					text = tr("The ship's vehicle transporter has no empty parking spots for your vehicle.");
				else if(result == -10)
					text = tr("The shields on your ship are blocking the ship's vehicle transporter.");
				else if(result == -11)
					text = tr("Your vehicle is not in an appropriate vehicle transporter parking spot.");
				else if(result == -12)
					text = tr("Another life form is too close to transport your vehicle aboard the ship. Are you "
							"riding an animal or did you just get eaten?");
				else if(result == -13)
					text = tr("A shield generator in your area is blocking the ship's vehicle transporter.");
				else if(result == -14)
					text = tr("Sorry, this ship is not equipped with a vehicle transporter.");
				else if(result == -15)
					text = tr("Sorry sir, we have no vacant parking spots for your vehicle.");
				else if(result == -16)
					text = tr("Sorry sir, your ship is not equipped with a vehicle transporter.");
				else if(result == -17)
					text = "";	// Person is standing in the destination transporter room.
				else if(result == -20)
					text = tr("Sorry, this ship is not equipped with a transporter.");
				else
					text = tr("The ship's vehicle transporter won't energize. I can't explain why; it's some kind of anomaly.");
			}
			else
			{
				if(result == requested && requested == 1)
					text = tr("I energized the ship's transporter. Welcome aboard.");
				else if(result == requested)
					text = tr("I energized the ship's transporter for %1 to beam aboard. Welcome aboard.").arg(result);
				else if(result > 0)
					text = tr("I could only energize the ship's transporter for %1 to beam aboard. Welcome aboard.").arg(result);
				else if(result == 0)
					text = tr("The ship's transporter beam was blocked. You must be in a cramped compartment.");
				else if(result == -1)
					text = tr("Sorry, no transporters on the ship are available.");
				else if(result == -2)
				{
					// Out of range.
					// Don't even answer if we're not at the same planet. Nobody should expect beam over at this range.
					if(person->FindParentPlanetInEnv() == FindParentPlanetInEnv())
						text = tr("You are outside my transporter range.");
					else
						return;	// Send no reply.
				}
				else if(result == -3)
					text = tr("I can't get the ship's transporter to lock on to you.");
				else if(result == -4)
					text = tr("The ship doesn't have enough power to energize my transporter. We're having some trouble here.");
				else if(result == -5)
					text = tr("The ship's transporter has not finished its activation cycle. Try again in a few seconds.");
				else if(result == -6)
					text = tr("The ship's vehicle transporter has no empty parking spots for your vehicle.");
				else if(result == -10)
					text = tr("The shields on your ship are blocking the ship's transporter. Turn off your "
							"shields or go to a transporter room and turn it on.");
				else if(result == -11)
					text = tr("Your vehicle is not in an appropriate vehicle transporter parking spot.");
				else if(result == -12)
					text = tr("Another life form is too close to transport you aboard the ship. Are you "
							"riding an animal or did you just get eaten?");
				else if(result == -13)
					text = tr("A shield generator in your area is blocking the ship's transporter. Turn off the "
							"shields or go to a transporter room and turn it on.");
				else if(result == -14)
					text = tr("Sorry, this ship is not equipped with a vehicle transporter.");
				else if(result == -17)
					text = "";	// Person is standing in the destination transporter room.
				else if(result == -20)
					text = tr("Sorry, this ship is not equipped with a transporter.");
				else
					text = tr("The ship's transporter won't energize. I can't explain why; it's some kind of anomaly.");
			}
			TxReplyChat(text, *msg, limitedRange);
		}
	}
}

void DaVessel::RxBeamVehicleAtom(const AuCommBeamVehicleAtom* msg, AuCommStdChannel::RANGE limitedRange)
{
	DaAtom* targAtom = 0;
	AuPoint targLoc, targZ;
	int resultCode = FindTransporterTarget(msg->CityId(), msg->Target(), targAtom, targLoc, targZ);
	if(resultCode || !targAtom)
	{
		// An error occurred.
		if(resultCode == 2)
		{
			// Transporter operator announces the result at close range.
			QString text = QObject::tr("Vehicle transport to enemy building blocked by shield generator on the ground.");
			TxReplyChat(text, *msg, limitedRange);
		}
		else if(resultCode == 3)
		{
			// Transporter operator announces the result at close range.
			QString text = QObject::tr("Vehicle transport to enemy city blocked by shield generator on the ground.");
			TxReplyChat(text, *msg, limitedRange);
		}
		else if(resultCode == 4)
		{
			// Transporter operator announces the result at close range.
			QString text = QObject::tr("Vehicle transport blocked by shields on the target vessel.");
			TxReplyChat(text, *msg, limitedRange);
		}
		return;
	}

	bool isFrom = msg->BeamDirection() == 0;
	quint16 targWhich = 0;
	QList<AuMatrix4x4> targParkingSpots;
	EnergizeVehicleTransporter(msg, isFrom, targAtom, targLoc, targZ, targWhich, targParkingSpots, limitedRange);
}

void DaVessel::RxBeamVehicleCoordinate(const AuCommBeamVehicleCoordinate* msg, AuCommStdChannel::RANGE limitedRange)
{
	//qDebug() << "DaSentient::RxBeamVehicleCoordinate" << Id() << Name();
	// Calculate location relative to the parent of the ship.
	DaAtom* targAtom = Parent();
	AuPoint targLoc = Transform().Project(msg->Coordinate());
	AuPoint targZ = Transform().ProjectVector(AuPoint(0,0,1));
	targZ.Unit();

	// Also calculate X and Y vectors to find a random location nearby.
	AuPoint targX = AuPoint(0,0,1).Cross(targZ);
	targX.Unit();
	AuPoint targY = targZ.Cross(targX);
	targY.Unit();

	// Offset randomly from the target location.
	targLoc += targX * g_Au.Dice().Roll(1,41,-21) + targY * g_Au.Dice().Roll(1,41,-21);

	// Insure the target location is not inside the shield of a city.
	DaWorldLL* targWorld = dynamic_cast<DaWorldLL*>(targAtom);
	if(targWorld && targWorld->SubatomShieldsBlockTransporters(EmpireId(), targLoc))
	{
		QString text = QObject::tr("Vehicle transport to coordinates blocked by shield generator on the ground.");
		TxReplyChat(text, *msg, limitedRange);
		return;
	}

	// Insure target location is not inside the hull of any spacecraft with shields up.
	AuPoint sysTargLoc = targAtom->EnvTransform().Project(targLoc);	// Calculate targLoc relative to solar system.
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return;	// Not likely.
	DaAtomList ships;
	system->FindVesselsRecursive(ships);
	for(DaAtomList::const_iterator si = ships.begin(); si != ships.end(); si++)
	{
		DaCapsule* otherShip = (DaCapsule*)(*si);
		if(otherShip->Is(Id()))
			continue;					// Ignore the ship we are aboard. We can beam aboard it using coordinates.
		if(!otherShip->ShieldsBlockTransportAboard())
			continue;					// Ignore ships without shields. We are not blocked from beaming aboard them.

		// Calculate the target location in the space of the other ship.
		AuPoint otherShipLoc = otherShip->EnvTransform().UnProject(sysTargLoc);
		if(otherShip->IsInOrOnHull(otherShipLoc) != 1)
			continue;					// Ignore ship if location is not inside its hull.

		// The other ship blocks the transporter destination because its shields are up.
		QString text = QObject::tr("Vehicle transporter blocked by spacecraft shields at destination.");
		TxReplyChat(text, *msg, limitedRange);
		return;
	}

	bool isFrom = msg->BeamDirection() == 0;
	quint16 targWhich = 0;
	QList<AuMatrix4x4> targParkingSpots;
	EnergizeVehicleTransporter(msg, isFrom, targAtom, targLoc, targZ, targWhich, targParkingSpots, limitedRange);
}

void DaVessel::RxBeamVehiclePlanet(const AuCommBeamVehiclePlanet* msg, AuCommStdChannel::RANGE limitedRange)
{
	//qDebug() << "DaSentient::RxBeamVehiclePlanet" << Id() << Name();
	// Locate the target planet.
	DaWorld* planet = FindParentPlanetInEnv();
	if(!planet)
	{
		// We are not orbiting a planet.
		QString text = QObject::tr("Unable to transport vehicle to planet coordinates. We are not orbiting a planet.");
		TxReplyChat(text, *msg, limitedRange);
		return;
	}

	DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(planet);
	if(!llworld)
		return;

	// Insure the terrain patches exist at the planet location.
	DaAtom* targAtom = llworld;
	AuPoint targLoc = llworld->GetSurfaceLocation(AuGlobeLL(msg->Longitude(), msg->Latitude()));
	AuPoint targZ = -llworld->GravityDir(targLoc);

	if(llworld->SubatomShieldsBlockTransporters(EmpireId(), targLoc))
	{
		QString text = QObject::tr("Vehicle transporter blocked by shield generator on the ground.");
		TxReplyChat(text, *msg, limitedRange);
		return;
	}

	bool isFrom = msg->BeamDirection() == 0;
	quint16 targWhich = 0;
	QList<AuMatrix4x4> targParkingSpots;
	EnergizeVehicleTransporter(msg, isFrom, targAtom, targLoc, targZ, targWhich, targParkingSpots, limitedRange);
}

void DaVessel::RxBeamVehicleShip(const AuCommBeamVehicleShip* msg, DaPerson* person, AuCommStdChannel::RANGE limitedRange)
{
	//qDebug() << "DaSentient::RxBeamVehicleShip" << Id() << Name();
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return;	// Not likely.

	// Locate the ship that is the target of the order.
	DaCapsule* targAtom = dynamic_cast<DaCapsule*>(system->FindIdRecursive(msg->ShipId()));
	if(!targAtom)
		return;

	// Insure the person is permitted to beam aboard the other ship.
	bool okToBeam = false;
	{
		if(targAtom->CheckAccess(person))
			okToBeam = true;	// Person is crew member, in chain of command, or an agent with powers active.
		else if(targAtom->IsSpaceStation())
		{
			// Stations will beam any non-enemy aboard if their doors are not locked.
			if(!targAtom->AnyHullDoorUnlocked() && PoliticalStance(targAtom->EmpireId(), EmpireId()) != POL_Enemy)
				okToBeam = true;
		}
		else if(targAtom->IsCommandedByAI())
		{
			// Ship is not a station and the requestor is not a crew member or in CoC.
			// AI commander must decide whether to beam them aboard.
			// The sender is neutral, friend, or empire; we've already skipped messages from enemy senders.
			AuMissionOrder::TYPE t = targAtom->MissionOrderType();
			if(t == AuMissionOrder::CrewLoad ||
					t == AuMissionOrder::PassengersLoad)
				okToBeam = true;		// Ok when the mission order is to board passengers or crew.
			else if(t == AuMissionOrder::TroopsLoad)
			{
				// Determine target's stance toward message sender.
				POLITICAL_STANCE stanceToSender = targAtom->PoliticalStanceMakeContact(person);
				if(stanceToSender == POL_Empire || stanceToSender == POL_Vassal)
					okToBeam = true;	// Ok when the mission order is to board troops and the sender is my empire or overlord.
			}
		}

		if(!okToBeam && targAtom->ShieldsBlockTransportAboard())
		{
			// Shields will block the beam attempt when not permitted to make the transport.
			QString text = QObject::tr("Vehicle transporter blocked by shields on target vessel.");
			TxReplyChat(text, *msg, limitedRange);
			return;
		}
	}

	AuPoint targLoc;
	if(!targAtom->TransporterVoidLocation(targLoc))
	{
		QString text = QObject::tr("Unable to lock onto a safe void inside the hull.");
		TxReplyChat(text, *msg, limitedRange);
		return;
	}

	AuPoint targZ(0,0,1);
	bool isFrom = msg->BeamDirection() == 0;
	quint16 targWhich = 0;
	QList<AuMatrix4x4> targParkingSpots;
	EnergizeVehicleTransporter(msg, isFrom, targAtom, targLoc, targZ, targWhich, targParkingSpots, limitedRange);
}

void DaVessel::RxBeamVehicleTransporter(const AuCommBeamVehicleTransporter* msg, DaPerson* person,
		AuCommStdChannel::RANGE limitedRange)
{
	//qDebug() << "DaSentient::RxBeamVehicleTransporter";
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return;	// Not likely.

	// Locate the ship that is the target of the order.
	DaCapsule* targAtom = dynamic_cast<DaCapsule*>(system->FindIdRecursive(msg->ShipId()));
	if(!targAtom)
		return;

	// Insure the person is permitted to beam aboard the other ship.
	bool okToBeam = false;
	const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(person->Parent());
	if(!vehicle)
		return;
	VEHICLE_SPOT_TYPE spotType = vehicle->ParkingSpotType();
	if(targAtom->CheckAccess(person))
		okToBeam = true;	// Person is crew member, in chain of command, or an agent with powers active.
	else if(targAtom->IsSpaceStation())
	{
		// Stations will beam any non-enemy aboard if their doors are not locked.
		if(!targAtom->AnyHullDoorUnlocked() && PoliticalStance(targAtom->EmpireId(), EmpireId()) != POL_Enemy)
			okToBeam = true;
	}
	else if(targAtom->IsCommandedByAI())
	{
		// Ship is not a station and the requestor is not a crew member or in CoC.
		// AI commander must decide whether to beam them aboard.
		// The sender is neutral, friend, or empire; we've already skipped messages from enemy senders.
		AuMissionOrder::TYPE t = targAtom->MissionOrderType();
		if(t == AuMissionOrder::CrewLoad ||
				t == AuMissionOrder::PassengersLoad)
			okToBeam = true;		// Ok when the mission order is to board passengers or crew.
		else if(t == AuMissionOrder::TroopsLoad)
		{
			// Determine target's stance toward message sender.
			POLITICAL_STANCE stanceToSender = targAtom->PoliticalStanceMakeContact(person);
			if(stanceToSender == POL_Empire || stanceToSender == POL_Vassal)
				okToBeam = true;	// Ok when the mission order is to board troops and the sender is my empire or overlord.
		}
	}

	if(!okToBeam && targAtom->ShieldsBlockTransportAboard())
	{
		// Shields will block the beam attempt when not permitted to make the transport.
		QString text = QObject::tr("Vehicle transporter blocked by shields on target vessel.");
		TxReplyChat(text, *msg, limitedRange);
		return;
	}

	// When permitted to beam the person, the destination transporter room need not already be on.
	// When not permitted to beam the person, only look for transporter rooms that are on and inactive.
	quint16 targWhich = targAtom->VehicleTransporterSystemId(spotType, !okToBeam);
	if(targWhich == 0xffff)
	{
		QString text = QObject::tr("Target vessel has no vehicle transporter available.");
		TxReplyChat(text, *msg, limitedRange);
		return;
	}

	AuMatrix4x4 spotM;
	DaVehicle* parked = 0;
	if(!targAtom->VehicleTransporterSpot(spotType, spotM, parked))
	{
		QString text = QObject::tr("Target vehicle transporter has no vacant parking spots.");
		TxReplyChat(text, *msg, limitedRange);
		return;
	}
	QList<AuMatrix4x4> targParkingSpots;
	targParkingSpots.append(spotM);

	AuPoint targLoc;
	AuPoint targZ(0,0,1);
	bool isFrom = msg->BeamDirection() == 0;
	if(EnergizeVehicleTransporter(msg, isFrom, targAtom, targLoc, targZ, targWhich, targParkingSpots, limitedRange))
	{
		// Transport was successful.
		// Eliminate the parked vehicle.
		if(parked && parked->IsNormal() && !parked->HasSubatoms())
			parked->Kill();
	}
}

void DaVessel::RxHailRequest(const AuCommHailRequest* msg, DaPerson* person, AuCommStdChannel::RANGE limitedRange,
		const AuId& sectorId)
{
	if(!msg)
		return;

	if(AuCommStdChannel::IsHailChannel(msg->ChannelName()))
	{
		// Message received on the Hail channel.
		bool contactedByCommand = CheckForContactByCommander(*msg);
		if(person)
		{
			if(IsSpaceStation() && !InArena() && !InDesignPreview())
			{
				// Stations will answer hail requests as far as solar system range.
				QString channame = QString("%1-%2").arg(Name()).arg(msg->FromId().StringId());
				if(msg->ChannelName() != channame)
				{
					// Open a channel to the sender.
					g_Au.OpenChannel(channame, AuCommStdChannel::Everyone, Id(), Name(), sectorId, Galaxy(), msg->FromId(), false);

					// Notify the person on the current channel that we have opened a channel.
					QString text(tr("Channel opened from station to %1.").arg(person->TitleName()));
					AuCommChat mymsg(text, msg->ChannelName(), Id(), Name(), sectorId, Galaxy(), limitedRange, msg->Scope());
					g_Au.SendAtomChat(mymsg);
				}

				// Hail the person on the channel.
				QString text;
				const DaWorld* planet = FindParentPlanetInEnv();
				const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
				const DaStar* star = dynamic_cast<const DaStar*>(Parent());
				if(contactedByCommand)
				{
					if(llworld)
					{
						AuGlobeLLF ll;
						if(llworld->LLOfLoc(ll, Location()))
						{
							QString longLat = ll.ToString();
							text = tr("Hail commander. %4 orbiting %2 at %3 here.").arg(llworld->Name())
								.arg(longLat).arg(NameWithEmpire());
						}
					}
					else if(star)
					{
						AuGlobeLLF ll;
						ll.SetFromGlobeLocation(Location());
						QString longLat = ll.ToString();
						text = tr("Hail commander. %4 orbiting %2 at %3 here.").arg(star->Name())
							.arg(longLat).arg(NameWithEmpire());
					}
					else
						text = tr("Hail commander. %4 here.").arg(NameWithEmpire());
				}
				else
				{
					if(llworld)
					{
						AuGlobeLLF ll;
						if(llworld->LLOfLoc(ll, Location()))
						{
							QString longLat = ll.ToString();
							text = tr("Hail traveller. %4 orbiting %2 at %3 here.").arg(llworld->Name()).arg(longLat)
								.arg(NameWithEmpire());
						}
					}
					else if(star)
					{
						AuGlobeLLF ll;
						ll.SetFromGlobeLocation(Location());
						QString longLat = ll.ToString();
						text = tr("Hail traveller. %4 orbiting %2 at %3 here.").arg(star->Name()).arg(longLat)
							.arg(NameWithEmpire());
					}
					else
						text = tr("Hail traveller. %4 here.").arg(NameWithEmpire());
				}
				AuCommChat hailmsg(text, channame, Id(), Name(), sectorId, Galaxy(), limitedRange,
						AuCommStdChannel::Everyone);
				g_Au.SendAtomChat(hailmsg);
			}
			else
			{
				DaSentient* officer = FindOfficer();					// Find any officer, even if they don't have the con.
				if((officer || Is(person->SpawnShip())) && person->FindParentVesselInEnv() != this)
				{
					// Officer replies to all non-enemy spacecraft as far as solar system range.
					// We would not be here if our stance toward the sender was enemy.
					// Ship replies to person who is spawn tied aboard.
					// Do not reply to hail from someone who is aboard.
					QString channame = QString("%1-%2").arg(Name()).arg(msg->FromId().StringId());
					if(msg->ChannelName() != channame)
					{
						// Open a channel to the sender.
						g_Au.OpenChannel(channame, AuCommStdChannel::Everyone, Id(), Name(), sectorId, Galaxy(), msg->FromId(),
								false);

						// Notify the person on the current channel that we have opened a channel.
						QString text(tr("Channel opened to %1.").arg(person->TitleName()));
						AuCommChat mymsg(text, msg->ChannelName(), Id(), Name(), sectorId, Galaxy(), limitedRange, msg->Scope());
						g_Au.SendAtomChat(mymsg);
					}
					AnswerHail(channame, msg->ReceptionRange(), msg->Scope(), contactedByCommand, officer);
				}
			}
		}
	}
	else if(msg->Scope() == AuCommStdChannel::CrewMates)
	{
		// Ship answers on crew channel. Could only be a hail from a crew member.
		DaSentient* officer = FindOfficer();					// Find any officer, even if they don't have the con.
		bool contactedByCommand = CheckForContactByCommander(*msg);
		AnswerHail(msg->ChannelName(), msg->ReceptionRange(), AuCommStdChannel::CrewMates, contactedByCommand, officer);
	}
	else if(msg->Scope() == AuCommStdChannel::Company)
	{
		// Ship answers on company channel. Could only be a hail from the owner.
		DaSentient* officer = FindOfficer();					// Find any officer, even if they don't have the con.
		QString channame(Name());
		if(msg->ChannelName() == AuCommStdChannel::CompanyChannelName())
		{
			// Open a channel to the sender.
			g_Au.OpenChannel(channame, AuCommStdChannel::Company, Id(), Name(), sectorId, Galaxy(), msg->FromId(), true);

			// Notify the person on the current channel that we have opened a channel.
			QString text(tr("Channel opened from company spacecraft to %1.").arg(msg->FromName()));
			AuCommChat mymsg(text, msg->ChannelName(), Id(), Name(), sectorId, Galaxy(),
					msg->ReceptionRange(), msg->Scope());
			mymsg.SetScopeData(msg->ScopeData());			// Respond with same company.
			g_Au.SendAtomChat(mymsg);
		}
		AnswerHail(channame, msg->ReceptionRange(), AuCommStdChannel::Company, true, officer);
	}
	else if(msg->Scope() == AuCommStdChannel::Fleet)
	{
		// Ship answers on fleet channel. Could only be a hail from command.
		DaSentient* officer = FindOfficer();					// Find any officer, even if they don't have the con.
		QString channame(Name());
		if(msg->ChannelName() == AuCommStdChannel::FleetChannelName())
		{
			// Open a channel to the sender.
			g_Au.OpenChannel(channame, AuCommStdChannel::Fleet, Id(), Name(), sectorId, Galaxy(), msg->FromId(), true);

			// Notify the person on the current channel that we have opened a channel.
			QString text(tr("Channel opened from fleet spacecraft to %1.").arg(msg->FromName()));
			AuCommChat mymsg(text, msg->ChannelName(), Id(), Name(), sectorId, Galaxy(),
					msg->ReceptionRange(), msg->Scope());
			mymsg.SetScopeData(msg->ScopeData());			// Respond with same empire.
			g_Au.SendAtomChat(mymsg);
		}
		AnswerHail(channame, msg->ReceptionRange(), AuCommStdChannel::Fleet, true, officer);
	}
	else
	{
		// Hail request received, not on Hail, Fleet or Crew channels.
		// Check credentials of sender to insure I'm in their chain of command.
		// Ignore the message if the person does not have the authority to order me around.
		DaSentient* officer = FindOfficer();					// Find any officer, even if they don't have the con.
		if(!CheckForContactByCommander(*msg))
			return;												// This ship is not in the sender's chain-of-command.

		// Answer back using the channel and scope they used.
		AnswerHail(msg->ChannelName(), msg->ReceptionRange(), msg->Scope(), true, officer);
	}
}

void DaVessel::RxContactShip(const AuCommContactShip* msg)
{
	if(!msg)
		return;
	if(msg->Scope() != AuCommStdChannel::CrewMates)
		return;

	// Ship answers on crew channel. Could only be sent by a crew member.
	DaSentient* officer = FindOfficer();					// Find any officer, even if they don't have the con.
	bool contactedByCommand = CheckForContactByCommander(*msg);
	AnswerContactShip(msg->ReceptionRange(), contactedByCommand, officer);
}

void DaVessel::RxJettison(const AuCommJettison* msg, AuCommStdChannel::RANGE limitedRange, const AuId& sectorId,
		DaPerson* person)
{
	if(!msg)
		return;

	// Jettison order is only issued on the intercom.
	if(msg->Scope() != AuCommStdChannel::Intercom)
		return;

	// Check credentials of sender to insure I'm in their chain of command. Simply ignore the message if the person does
	// not have the authority to order me around.
	if(!person || !InChainOfCommand(person))
		return;	// This ship is not in the sender's chain-of-command.

	// Determine what is being jettisoned.
	const AuItem* jettisonItem = msg->Item();
	if(!jettisonItem)
		return;

	const AuItemTEUContainer* teuItem = dynamic_cast<const AuItemTEUContainer*>(msg->Item());
	if(teuItem && teuItem->Cargo() && teuItem->IsInsured())
	{
		// Calculate the value of the cargo and put a bounty on the shipper's head and the jettisoner's head.
		double bounty = teuItem->Cargo()->Value();
		if(bounty >= s_MinBounty)
		{
			if(person->Is(teuItem->CarrierId()))
			{
				// The opening person is the carrier responsible for the shipment.
				QString reason(tr("Jettisoned an insured shipment entrusted to their care."));
				person->AddBounty(AuId::InsurerId(), InsurerName(), reason, bounty, 1);
			}
			else
			{
				// The opening person is not the carrier responsible for the shipment.
				// Split the bounty 50:50.
				bounty /= 2;
				QString reason(tr("Jettisoned an insured shipment."));
				person->AddBounty(AuId::InsurerId(), InsurerName(), reason, bounty, 2);

				reason = tr("Allowed an insured shipment to be jettisoned.");
				emit BountyPosted(AuId::InsurerId(), InsurerName(), teuItem->CarrierId(), reason, bounty, 3);
			}
		}
	}

	// Remove the cargo and report the result.
	QString resultMsg;
	if(RemoveCargo(*jettisonItem, jettisonItem->Count()))
		resultMsg = tr("%1 jettisoned.").arg(jettisonItem->ListDesc());
	else
		resultMsg = tr("Failed to jettison %1.").arg(jettisonItem->ListDesc());

	// Notify the person of the result of the jettison, good or bad.
	AuCommChat mymsg(resultMsg, msg->ChannelName(), Id(), Name(), sectorId, Galaxy(), limitedRange,
			msg->Scope());
	g_Au.SendAtomChat(mymsg);
}

void DaVessel::RxCombine(const AuCommCombine* msg, AuCommStdChannel::RANGE limitedRange, const AuId& sectorId,
		DaPerson* person)
{
	if(!msg)
		return;

	// Combine order is only issued on the intercom.
	if(msg->Scope() != AuCommStdChannel::Intercom)
		return;

	// Check credentials of sender to insure I'm in their chain of command. Simply ignore the message if the person does
	// not have the authority to order me around.
	if(!person || !InChainOfCommand(person))
		return;	// This ship is not in the sender's chain-of-command.

	// Determine what is being jettisoned.
	const AuItem* item1 = msg->Item();
	const AuItem* item2 = msg->OfferItem();
	if(!item1 || !item2)
		return;
	const AuItem* highItem;
	const AuItem* lowItem;
	if(item1->Quality() > item2->Quality())
	{
		highItem = item1;
		lowItem = item2;
	}
	else
	{
		highItem = item2;
		lowItem = item1;
	}

	// Combine the cargo and report the result.
	QString resultMsg;
	if(CombineCargo(*lowItem, *highItem))
		resultMsg = tr("%1 combined into %2.")
			.arg(highItem->ListDesc())
			.arg(lowItem->ListDesc());
	else
		resultMsg = tr("Failed to combine %1 with %2.")
			.arg(lowItem->ListDesc())
			.arg(highItem->ListDesc());

	// Notify the person of the result of the jettison, good or bad.
	AuCommChat mymsg(resultMsg, msg->ChannelName(), Id(), Name(), sectorId, Galaxy(), limitedRange,
			msg->Scope());
	g_Au.SendAtomChat(mymsg);
}

bool DaVessel::CombineCargo(const AuItem& intoItem, const AuItem& combItem)
{
	if(!intoItem.CanCombine() || !combItem.CanCombine())
		return false;
	bool hadTheCargo = false;
	if(combItem.CommodityId() == CID_Hydrogen)
		hadTheCargo = false;	// How did they do this?
	else if(combItem.CommodityId() == CID_Electricity)
		hadTheCargo = false;	// How did they do this?
	else if(m_Cargo.TakeItem(combItem, combItem.Count()))
	{
		// The combine item was removed from our hold. We know we have room to put the same amount back right away.
		hadTheCargo = true;
		AuItem* nucomb = intoItem.Clone();	// Clone this item so they match exactly.
		nucomb->SetCount(combItem.Count());	// Set its count to the amount we removed.
		AddCargo(*nucomb);
		delete nucomb;
	}
	return hadTheCargo;
}

// Checks for officer aboard to accept a message. Officer takes the con if message is from an officer and nobody has the con.
bool DaVessel::CanOfficerAcceptMessage(const AuCommPacket* msg)
{
	if(!msg)
		return false;

	if(!HasOfficers())
		return false;	// A ship with no officers cannot accept mission orders.

	// On the crew channel, the sender's ship must accept the order so a human Captain can issue orders to the ship remotely.
	if(msg->Scope() != AuCommStdChannel::CrewMates && msg->Scope() != AuCommStdChannel::Intercom)
	{
		// On other channels, the ship must be AI controlled to accept orders.
		// On human-controlled ships the captain must accept the order comm message. The order is not accepted here.
		if(!IsCommandedByAI())
		{
			// If the sender is an officer of the ship, the AI officer can take the con automatically.
			if(!IsOfficer(msg->FromId()))
				return false;									// Sender is not an officer of the ship.
			DaSentient* officer = FindOfficer();				// Find any officer, even if they don't have the con.
			if(!officer)
				return false;									// No officer to accept the mission order.

			// Officer can take the con.
			officer->AmendOrder(ORDER_TakeCommand, Id(), AuPoint(0,0), 0);
			officer->SetModified();
			QString text = tr("Sir, %1 here. I am taking the con to carry out your orders.").arg(officer->Name());
			TxReplyChat(text, *msg, msg->ReceptionRange());
		}
	}

	// Check credentials of sender to insure I'm in their chain of command. Simply ignore the message if the person does
	// not have the authority to order me around.
	return CheckForContactByCommander(*msg);	// False if this ship is not in the sender's chain-of-command.
}

// Check for messages that trigger contact logic with AI officers, whether or not they are in command.
// Triggers that contact when the message is of the proper type.
// The only criteria is that the sender be in this ship's chain-of-command.
bool DaVessel::CheckForContactByCommander(const AuCommPacket& msg)
{
	// This test particularly excludes admins from the InChainOfCommand test so every ship in the universe does not answer
	// hails from admins.
	return InChainOfCommand(msg.FromId(), msg.FromEmpireId());
}

// Receive an entire mission.
void DaVessel::RxMissionNew(const AuCommMissionNew* msg)
{
	// Check for officer aboard to accept the message. Officer takes the con if message is from an officer and nobody has the con.
	if(!CanOfficerAcceptMessage(msg))
		return;

	// Configure the current mission.
	m_MissionLock.lockForWrite();
	m_Mission = msg->Mission();
	int c = m_Mission.Count();
	m_MissionLock.unlock();
	QString text;
	if(IsMissionActive())
		text = tr("Aye, mission with %1 steps received.").arg(c);
	else
		text = tr("Aye, mission with %1 steps received. Holding for mission activation.").arg(c);
	TxReplyChat(text, *msg, msg->ReceptionRange());
}

void DaVessel::RxMissionOrderAdd(const AuCommMissionOrderAdd* msg)
{
	// Check for officer aboard to accept the message. Officer takes the con if message is from an officer and nobody has the con.
	if(!CanOfficerAcceptMessage(msg))
		return;

	// Decode the order.
	QString replyText;
	const AuMissionOrder* order = msg->MissionOrder();
	if(order && order->IsValid())
	{
		MissionAppendOrder(*order);						// Set up the nu mission with the nu order.
		if(IsMissionActive())
			replyText = tr("Aye, add %1.").arg(order->ListName());
		else
			replyText = tr("Aye, add %1. Holding until mission activated.").arg(order->ListName());
	}
	else
		replyText = tr("Garbled mission order. Please repeat.");

	// Send a reply at the same range as the original message was sent.
	TxReplyChat(replyText, *msg, msg->ReceptionRange());
}

void DaVessel::RxMissionOrderInsert(const AuCommMissionOrderInsert* msg)
{
	// Check for officer aboard to accept the message. Officer takes the con if message is from an officer and nobody has the con.
	if(!CanOfficerAcceptMessage(msg))
		return;

	// Decode the order.
	QString replyText;
	const AuMissionOrder* order = msg->MissionOrder();
	if(order && order->IsValid())
	{
		MissionInsertOrder(0, *order);		// Set up the nu mission with the nu order.
		if(IsMissionActive())
			replyText = tr("Aye, %1 now current.").arg(order->ListName());
		else
			replyText = tr("Aye, %1 now current. Holding until mission activated.").arg(order->ListName());
	}
	else
		replyText = tr("Garbled mission order. Please repeat.");

	// Send a reply at the same range as the original message was sent.
	TxReplyChat(replyText, *msg, msg->ReceptionRange());
}

void DaVessel::RxMissionOrderNew(const AuCommMissionOrderNew* msg)
{
	// Check for officer aboard to accept the message. Officer takes the con if message is from an officer and nobody has the con.
	if(!CanOfficerAcceptMessage(msg))
		return;

	// Decode the order.
	QString replyText;
	const AuMissionOrder* order = msg->MissionOrder();
	if(order && order->IsValid())
	{
		MissionClear();					// Clear the previous mission, without notifying the pirate captain, if any.
		MissionAppendOrder(*order);		// Set up the nu mission with the nu order.
		if(IsMissionActive())
			replyText = tr("Aye, %1.").arg(order->ListName());
		else
			replyText = tr("Aye, %1. Holding until mission activated.").arg(order->ListName());
	}
	else
		replyText = tr("Garbled mission order. Please repeat.");

	// Send a reply at the same range as the original message was sent.
	TxReplyChat(replyText, *msg, msg->ReceptionRange());
}

void DaVessel::RxMissionProperty(const AuCommMissionProperty* msg)
{
	// Check for officer aboard to accept the message. Officer takes the con if message is from an officer and nobody has the con.
	if(!CanOfficerAcceptMessage(msg))
		return;

	int property = msg->Property();
	int value = msg->Value();
	switch(property)
	{
	case 0:	// Request for mission.
		m_MissionLock.lockForRead();
		if(m_Mission.IsEmpty())
		{
			QString text = tr("Mission is %1, %2. No orders.").arg(IsMissionActive()?tr("active"):tr("on hold"))
				.arg(m_Mission.Type() == AuMission::Single?tr("do once"):tr("cyclic"));
			m_MissionLock.unlock();

			TxReplyChat(text, *msg, msg->ReceptionRange());
		}
		else
		{
			QString text = tr("Mission is %1, %2.").arg(IsMissionActive()?tr("active"):tr("on hold"))
				.arg(m_Mission.Type() == AuMission::Single?tr("do once"):tr("cyclic"));
			QStringList missionOrders;
			m_Mission.ListDescriptions(missionOrders);
			m_MissionLock.unlock();

			TxReplyChat(text, *msg, msg->ReceptionRange());
			for(QStringList::const_iterator mi = missionOrders.cbegin(); mi != missionOrders.cend(); mi++)
				TxReplyChat(*mi, *msg, msg->ReceptionRange());
		}
		break;
	case 1:	// Active state
		{
			MissionSetActive(value);
			if(IsMissionActive())
				TxReplyChat(tr("Aye, activate mission."), *msg, msg->ReceptionRange());
			else
				TxReplyChat(tr("Aye, hold mission."), *msg, msg->ReceptionRange());
		}
		break;
	case 2:	// Cycle state
		{
			MissionSetType((AuMission::TYPE)value);
			if(value == AuMission::Single)
				TxReplyChat(tr("Aye, do mission once."), *msg, msg->ReceptionRange());
			else
				TxReplyChat(tr("Aye, cycle mission."), *msg, msg->ReceptionRange());
		}
		break;
	case 3:	// Request for mission disk.
		m_MissionLock.lockForRead();
		if(m_Mission.IsEmpty())
		{
			m_MissionLock.unlock();
			QString text = tr("Mission has no orders.");
			TxReplyChat(text, *msg, msg->ReceptionRange());
		}
		else
		{
			// Send my current mission.
			AuCommMissionNew mymsg(msg->FromId(), Name(), m_Mission, EmpireId(),
					msg->ChannelName(), Id(), Name(), FindParentSector()->Id(), Galaxy(), msg->ReceptionRange(),
					msg->Scope());
			m_MissionLock.unlock();

			if(mymsg.Scope() == AuCommStdChannel::CrewMates)
				mymsg.SetScopeData(Id().GetID());			// CrewMate scope needs ship id.
			else if(mymsg.Scope() == AuCommStdChannel::Company)
				mymsg.SetScopeData(msg->ScopeData());		// Respond with same company.
			else
				mymsg.SetScopeData(EmpireId());			// All other logical scopes need empire id.
			g_Au.SendAtomChat(mymsg);
		}
		break;
	}
}

void DaVessel::RxRequestBerthAny(const AuCommRequestBerthAny* msg, DaPerson* person)
{
	if(!msg)
		return;

	// Request can be accepted by the ship on the intercom channel if policy permits.
	if(person && msg->Scope() == AuCommStdChannel::Intercom)
	{
		// Try to add the requestor to the roster.
		// Captain's berth will take the CaptainSlot instead if that is appropriate.
		if(!CaptainsBerthOccupied() && (!HasOfficers() || InChainOfCommand(person)))
			RequestBerth(*msg, AuRosterEntry::Captain, person);
		else if(OfficerBerthsAvailable() && (!HasOfficers() || InChainOfCommand(person)))
			RequestBerth(*msg, AuRosterEntry::OfficerHuman, person);
		else if(CrewBerthsAvailable())
			RequestBerth(*msg, AuRosterEntry::Crew, person);
		else if(PassengerBerthsAvailable())
			RequestBerth(*msg, AuRosterEntry::PassengerHuman, person);
		else if(TroopBerthsAvailable())
			RequestBerth(*msg, AuRosterEntry::Troop, person);

		CheckForContactByCommander(*msg);
	}
	else if(IsCommandedByAI())
	{
		// Berth request received by AI controlled ship.
		// Check credentials of sender to insure I'm in their chain of command. Simply ignore the message if the person does
		// not have the authority to order me around.
		if(!InChainOfCommand(msg->FromId(), msg->FromEmpireId()))
			return;	// This ship is not in the sender's chain-of-command.

		// Try to add the requestor to the roster.
		// Captain's berth will take the CaptainSlot instead if that is appropriate.
		if(!CaptainsBerthOccupied())
			RequestBerth(*msg, AuRosterEntry::Captain, msg->FromName(), msg->FromId(), msg->FromEmpireId());
		else if(OfficerBerthsAvailable())
			RequestBerth(*msg, AuRosterEntry::OfficerHuman, msg->FromName(), msg->FromId(), msg->FromEmpireId());
		else if(CrewBerthsAvailable())
			RequestBerth(*msg, AuRosterEntry::Crew, msg->FromName(), msg->FromId(), msg->FromEmpireId());
		else if(PassengerBerthsAvailable())
			RequestBerth(*msg, AuRosterEntry::PassengerHuman, msg->FromName(), msg->FromId(), msg->FromEmpireId());
		else if(TroopBerthsAvailable())
			RequestBerth(*msg, AuRosterEntry::Troop, msg->FromName(), msg->FromId(), msg->FromEmpireId());
	}
}

void DaVessel::RxRequestBerthCaptain(const AuCommRequestBerthCaptain* msg, DaPerson* person)
{
	if(!msg)
		return;

	// Request can be accepted by the ship on the intercom channel if policy permits.
	if(person && msg->Scope() == AuCommStdChannel::Intercom)
	{
		// Ship with officers won't give up captaincy to requestor who is not in the ship's chain of command.
		// If the ship has officers, the captain's berth is only granted if the requestor is in the ship's chain-of-command.
		// This prevents rank-and-file players from taking over a ship captained by an AI-officer.
		// This allows a human officer to request the captain's berth if the captain releases it because that human officer
		// is in the chain-of-command of the ship.
		if(!HasOfficers() || InChainOfCommand(person))
			RequestBerth(*msg, AuRosterEntry::Captain, person);		// Try to add the requestor to the roster.

		CheckForContactByCommander(*msg);
	}
	else if(IsCommandedByAI())
	{
		// Berth request received by AI controlled ship.
		// Check credentials of sender to insure I'm in their chain of command. Simply ignore the message if the person does
		// not have the authority to order me around.
		if(!InChainOfCommand(msg->FromId(), msg->FromEmpireId()))
			return;	// This ship is not in the sender's chain-of-command.

		// Try to add the requestor to the roster.
		RequestBerth(*msg, AuRosterEntry::Captain, msg->FromName(), msg->FromId(), msg->FromEmpireId());
	}
}

void DaVessel::RxRequestBerthCaptainSlot(const AuCommRequestBerthCaptainSlot* msg, DaPerson* person)
{
	if(!msg)
		return;

	// Request can be accepted by the ship on the intercom channel if policy permits.
	if(person && msg->Scope() == AuCommStdChannel::Intercom)
	{
		// Ship with officers won't give up captaincy to requestor who is not in the ship's chain of command.
		// If the ship has officers, the captain's berth is only granted if the requestor is in the ship's chain-of-command.
		// This prevents rank-and-file players from taking over a ship captained by an AI-officer.
		// This allows a human officer to request the captain's berth if the captain releases it because that human officer
		// is in the chain-of-command of the ship.
		if(!HasOfficers() || InChainOfCommand(person))
			RequestBerth(*msg, AuRosterEntry::CaptainSlot, person);	// Try to add the requestor to the roster.

		CheckForContactByCommander(*msg);
	}
	else if(IsCommandedByAI())
	{
		// Berth request received by AI controlled ship.
		// Check credentials of sender to insure I'm in their chain of command. Simply ignore the message if the person does
		// not have the authority to order me around.
		if(!InChainOfCommand(msg->FromId(), msg->FromEmpireId()))
			return;	// This ship is not in the sender's chain-of-command.

		// Try to add the requestor to the roster.
		RequestBerth(*msg, AuRosterEntry::CaptainSlot, msg->FromName(), msg->FromId(), msg->FromEmpireId());
	}
}

void DaVessel::RxRequestBerthCrew(const AuCommRequestBerthCrew* msg, DaPerson* person)
{
	if(!msg)
		return;

	// Request can be accepted by the ship on the intercom channel if policy permits.
	if(person && (msg->Scope() == AuCommStdChannel::Intercom || MissionOrderType() == AuMissionOrder::CrewLoad))
	{
		RequestBerth(*msg, AuRosterEntry::Crew, person);			// Try to add the requestor to the roster.

		CheckForContactByCommander(*msg);
	}
	else if(IsCommandedByAI())
	{
		// Berth request received by AI controlled ship.
		// Check credentials of sender to insure I'm in their chain of command. Simply ignore the message if the person does
		// not have the authority to order me around.
		if(!InChainOfCommand(msg->FromId(), msg->FromEmpireId()))
			return;	// This ship is not in the sender's chain-of-command.

		// Try to add the requestor to the roster.
		RequestBerth(*msg, AuRosterEntry::Crew, msg->FromName(), msg->FromId(), msg->FromEmpireId());
	}
}

void DaVessel::RxRequestBerthOfficer(const AuCommRequestBerthOfficer* msg, DaPerson* person)
{
	if(!msg)
		return;

	// Request can be accepted by the ship on the intercom channel if policy permits.
	if(person && msg->Scope() == AuCommStdChannel::Intercom)
	{
		// Ship with officers won't give up officer berth to requestor who is not in the ship's chain of command.
		// If the ship has officers, officer berths are only granted if the requestor is in the ship's chain-of-command.
		// This prevents rank-and-file players from taking over a ship captained by an AI-officer.
		if(!HasOfficers() || InChainOfCommand(person))
			RequestBerth(*msg, AuRosterEntry::OfficerHuman, person);		// Try to add the requestor to the roster.

		CheckForContactByCommander(*msg);
	}
	else if(IsCommandedByAI())
	{
		// Berth request received by AI controlled ship.
		// Check credentials of sender to insure I'm in their chain of command. Simply ignore the message if the person does
		// not have the authority to order me around.
		if(!InChainOfCommand(msg->FromId(), msg->FromEmpireId()))
			return;	// This ship is not in the sender's chain-of-command.

		// Try to add the requestor to the roster.
		RequestBerth(*msg, AuRosterEntry::OfficerHuman, msg->FromName(), msg->FromId(), msg->FromEmpireId());
	}
}

void DaVessel::RxRequestBerthPassenger(const AuCommRequestBerthPassenger* msg, DaPerson* person)
{
	if(!msg)
		return;

	// Request can be accepted by the ship on the intercom channel if policy permits.
	// Request can be accepted on other channels if the current mission order is to load passengers.
	if(person && (msg->Scope() == AuCommStdChannel::Intercom || MissionOrderType() == AuMissionOrder::PassengersLoad))
	{
		RequestBerth(*msg, AuRosterEntry::PassengerHuman, person);		// Try to add the requestor to the roster.

		CheckForContactByCommander(*msg);
	}
	else if(IsCommandedByAI())
	{
		// Berth request received by AI controlled ship.
		// Check credentials of sender to insure I'm in their chain of command. Simply ignore the message if the person does
		// not have the authority to order me around.
		if(!InChainOfCommand(msg->FromId(), msg->FromEmpireId()))
			return;	// This ship is not in the sender's chain-of-command.

		// Try to add the requestor to the roster.
		RequestBerth(*msg, AuRosterEntry::PassengerHuman, msg->FromName(), msg->FromId(), msg->FromEmpireId());
	}
}

void DaVessel::RxRequestBerthTroop(const AuCommRequestBerthTroop* msg, DaPerson* person)
{
	if(!msg)
		return;

	// Request can be accepted by the ship on the intercom channel if policy permits.
	if(person && (msg->Scope() == AuCommStdChannel::Intercom || MissionOrderType() == AuMissionOrder::TroopsLoad))
	{
		RequestBerth(*msg, AuRosterEntry::Troop, person);			// Try to add the requestor to the roster.

		CheckForContactByCommander(*msg);
	}
	else if(IsCommandedByAI())
	{
		// Berth request received by AI controlled ship.
		// Check credentials of sender to insure I'm in their chain of command. Simply ignore the message if the person does
		// not have the authority to order me around.
		if(!InChainOfCommand(msg->FromId(), msg->FromEmpireId()))
			return;	// This ship is not in the sender's chain-of-command.

		// Try to add the requestor to the roster.
		RequestBerth(*msg, AuRosterEntry::Troop, msg->FromName(), msg->FromId(), msg->FromEmpireId());
	}
}

void DaVessel::RxRequestInventory(const AuCommRequestInventory* msg, const AuId& sectorId, AuCommStdChannel::RANGE range,
		DaPerson* person)
{
	// Check for officer aboard to accept the message. Officer takes the con if message is from an officer and nobody has the con.
	bool canDo;
	if(CanOfficerAcceptMessage(msg))
		canDo = true;
	else if(person && InChainOfCommand(person))
		canDo = true;
	else if(InChainOfCommand(msg->FromId(), EmpireId()))
		canDo = true;
	else
		canDo = false;
	if(!canDo)
		return;	// This ship is not in the sender's chain-of-command and no officer is here to respond.

	// Send every commodity as an InventoryItem message.
	// We'll accumulate an alphabetically sorted list so they look nice on the clients screen.
	QString typeName;
	QMap<QString, SellOffer> offers;	// List of offers sorted alphabetically. Key is commodity name.
	m_CargoLock.lockForRead();
	for(AuItemHash::const_iterator it = m_Cargo.begin(); it != m_Cargo.end(); it++)
	{
		// Prepare a sell offer.
		SellOffer offer(*(*it).second);

		// Insert the sell offer into our list in alphabetical order.
		QString name = (*it).second->ListName();
		offers.insert(name, offer);
	}
	m_CargoLock.unlock();

	// Add an item for fuel in the fuel cells.
	if(FuelQuantity() >= 1)
	{
		AuItem* fuelItem = AuItem::Create(CID_Hydrogen, FuelQuality(), EmpireId(), (int)FuelQuantity());

		// Prepare a sell offer.
		SellOffer offer(*fuelItem);

		// Insert the sell offer into our list in alphabetical order.
		QString name = fuelItem->ListName();
		offers.insert(name, offer);

		delete fuelItem;
	}

	// Add an item for electricity in the capacitor.
	if(CapacitorLevel() >= 1)
	{
		AuItem* powerItem = AuItem::Create(CID_Electricity, 255, 0, (int)CapacitorLevel());

		// Prepare a sell offer.
		SellOffer offer(*powerItem);

		// Insert the sell offer into our list in alphabetical order.
		QString name = powerItem->ListName();
		offers.insert(name, offer);

		delete powerItem;
	}

	// Lock atom chat queue mutex so we can access both the open
	// channels queue and the chat messages queue simultaneously.
	AuAtomChatList* chatqueue = g_Au.LockAtomChat();

	// Open a channel to the sender so my output, which could be considerable, will not be mixed with other messages on the channel.
	QString channame;
	AuCommStdChannel::SCOPE scope;
	quint32 scopeData;
	if(msg->ChannelName() != Name() &&
			msg->ChannelName() != AuCommStdChannel::CrewChannelName() &&
			msg->ChannelName() != AuCommStdChannel::IntercomChannelName())
	{
		// Open a channel to the sender.
		channame = Name();
		scope = AuCommStdChannel::Fleet;
		scopeData = EmpireId();
		g_Au.m_OpenChannels.append(AuOpenChannel(channame, scope, Id(), Name(), sectorId, Galaxy(), msg->FromId(), true));

		// Notify the person on the current channel that we have sent a response on a different channel.
		QString text = tr("%1 status report was sent on a private channel.").arg(SpacecraftStructureName(Structure()));
		AuCommChat mymsg(text, msg->ChannelName(), Id(), Name(), sectorId, Galaxy(), range, msg->Scope());
		if(mymsg.Scope() == AuCommStdChannel::CrewMates)
			mymsg.SetScopeData(Id().GetID());					// CrewMate scope needs ship id.
		else if(mymsg.Scope() == AuCommStdChannel::Company)
			mymsg.SetScopeData(msg->ScopeData());		// Respond with same company.
		else
			mymsg.SetScopeData(EmpireId());						// All other logical scopes need empire id.
		chatqueue->append(AuAtomChat(mymsg));
	}
	else
	{
		channame = msg->ChannelName();
		scope = msg->Scope();
		scopeData = msg->ScopeData();
	}

	// Damage summary
	{
		QString text;
		unsigned int maxHits = DesignHitsStructural();
		unsigned int dmgHits = HitsDamageStructural();
		unsigned int curHits;
		if(dmgHits < maxHits)
			curHits = maxHits - dmgHits;
		else
			curHits = 0;
		QString worst = WorstDamagedSystem();
		if(curHits < maxHits && !worst.isEmpty())
		{
			quint8 healthPerc = Percent(curHits, maxHits);
			unsigned int worstPerc = 100 - WorstSystemDamage();
			text = QObject::tr("Structure is %1%; %2 is at %3%.")
				.arg(healthPerc)
				.arg(worst)
				.arg(worstPerc);
		}
		else if(curHits < maxHits)
		{
			quint8 healthPerc = Percent(curHits, maxHits);
			text = QObject::tr("Structure is %1%; all systems are nominal.")
				.arg(healthPerc);
		}
		else if(!worst.isEmpty())
		{
			unsigned int worstPerc = 100 - WorstSystemDamage();
			text = QObject::tr("Structure is 100%; %2 is at %3%.")
				.arg(worst)
				.arg(worstPerc);
		}
		else
			text = QObject::tr("Structure is 100%; all systems are nominal.");

		AuCommChat mymsg(text, channame, Id(), Name(), sectorId, Galaxy(), range, scope);
		if(mymsg.Scope() == AuCommStdChannel::CrewMates)
			mymsg.SetScopeData(Id().GetID());					// CrewMate scope needs ship id.
		else if(mymsg.Scope() == AuCommStdChannel::Company)
			mymsg.SetScopeData(scopeData);						// Respond with same company.
		else
			mymsg.SetScopeData(EmpireId());						// All other logical scopes need empire id.
		chatqueue->append(AuAtomChat(mymsg));
	}

	bool captain = CaptainsBerthOccupied();
	unsigned int officers = OfficerBerthsOccupied();
	if(captain && officers)
		officers--;			// When officer berths are occupied and there is a captain, one of those officer berths is the captain.
	unsigned int crew = CrewBerthsOccupied();
	unsigned int passengers = PassengerBerthsOccupied();
	unsigned int troops = TroopBerthsOccupied();
	unsigned int cryo = CryoBerthsOccupied();

	// Personnel
	if(officers || crew || troops || passengers || cryo)
	{
		QString text = tr("Occupied berths: ");
		if(captain)
			text += tr(" Captain");
		if(officers)
		{
			if(captain)
				text += ", ";
			text += tr(" %L1 officers").arg(officers);
		}
		if(crew)
		{
			if(captain || officers)
				text += ", ";
			text += tr(" %L1 crew").arg(crew);
		}
		if(troops)
		{
			if(captain || officers || crew)
				text += ", ";
			text += tr(" %L1 troops").arg(troops);
		}
		if(passengers)
		{
			if(captain || officers || crew || troops)
				text += ", ";
			text += tr(" %L1 passengers").arg(passengers);
		}
		if(cryo)
		{
			if(captain || officers || crew || troops || passengers)
				text += ", ";
			text += tr(" %L1 cryo travellers").arg(cryo);
		}

		AuCommChat mymsg(text, channame, Id(), Name(), sectorId, Galaxy(), range, scope);
		if(mymsg.Scope() == AuCommStdChannel::CrewMates)
			mymsg.SetScopeData(Id().GetID());					// CrewMate scope needs ship id.
		else if(mymsg.Scope() == AuCommStdChannel::Company)
			mymsg.SetScopeData(scopeData);						// Respond with same company.
		else
			mymsg.SetScopeData(EmpireId());						// All other logical scopes need empire id.
		chatqueue->append(AuAtomChat(mymsg));
	}
	else
	{
		QString text = tr("No personnel on the roster.");
		AuCommChat mymsg(text, channame, Id(), Name(), sectorId, Galaxy(), range, scope);
		if(mymsg.Scope() == AuCommStdChannel::CrewMates)
			mymsg.SetScopeData(Id().GetID());					// CrewMate scope needs ship id.
		else if(mymsg.Scope() == AuCommStdChannel::Company)
			mymsg.SetScopeData(scopeData);						// Respond with same company.
		else
			mymsg.SetScopeData(EmpireId());						// All other logical scopes need empire id.
		chatqueue->append(AuAtomChat(mymsg));
	}

	// Account
	{
		QString text(tr("Ship's fund %L1%2.")
				.arg(AccountBalance(), 0,'f',0).arg(g_Au.CurrencySymbol()));
		AuCommChat mymsg(text, channame, Id(), Name(), sectorId, Galaxy(), range, scope);
		if(mymsg.Scope() == AuCommStdChannel::CrewMates)
			mymsg.SetScopeData(Id().GetID());					// CrewMate scope needs ship id.
		else if(mymsg.Scope() == AuCommStdChannel::Company)
			mymsg.SetScopeData(scopeData);						// Respond with same company.
		else
			mymsg.SetScopeData(EmpireId());						// All other logical scopes need empire id.
		chatqueue->append(AuAtomChat(mymsg));
	}

	if(!offers.isEmpty())
	{
		// Construct a chat message to send on the nu channel.
		QString text(tr("Inventory of hold, capacitor and fuel cells follows."));
		AuCommChat mymsg(text, channame, Id(), Name(), sectorId, Galaxy(), range, scope);
		if(mymsg.Scope() == AuCommStdChannel::CrewMates)
			mymsg.SetScopeData(Id().GetID());					// CrewMate scope needs ship id.
		else if(mymsg.Scope() == AuCommStdChannel::Company)
			mymsg.SetScopeData(scopeData);						// Respond with same company.
		else
			mymsg.SetScopeData(EmpireId());						// All other logical scopes need empire id.
		chatqueue->append(AuAtomChat(mymsg));

		// Now transmit the sorted list of offers as InventoryItem messages.
		for(QMap<QString, SellOffer>::const_iterator it = offers.cbegin(); it != offers.cend(); it++)
		{
			// Format the InventoryItem message text.
			AuCommInventoryItem mymsg(*(*it).m_Item, (*it).m_EmpireOfOrigin, channame, Id(), Name(), sectorId,
					Galaxy(), range, scope);
			if(mymsg.Scope() == AuCommStdChannel::CrewMates)
				mymsg.SetScopeData(Id().GetID());					// CrewMate scope needs ship id.
			else if(mymsg.Scope() == AuCommStdChannel::Company)
				mymsg.SetScopeData(scopeData);						// Respond with same company.
			else
				mymsg.SetScopeData(EmpireId());						// All other logical scopes need empire id.
			chatqueue->append(AuAtomChat(mymsg));
		}
	}
	else
	{
		// Construct a chat message to send on the nu channel.
		QString text(tr("Hold, capacitor and fuel cells are empty."));
		AuCommChat mymsg(text, channame, Id(), Name(), sectorId, Galaxy(), range, scope);
		if(mymsg.Scope() == AuCommStdChannel::CrewMates)
			mymsg.SetScopeData(Id().GetID());					// CrewMate scope needs ship id.
		else if(mymsg.Scope() == AuCommStdChannel::Company)
			mymsg.SetScopeData(scopeData);						// Respond with same company.
		else
			mymsg.SetScopeData(EmpireId());						// All other logical scopes need empire id.
		chatqueue->append(AuAtomChat(mymsg));
	}
	g_Au.UnlockAtomChat();
}

// Send a reply at the same range as the original message was sent.
void DaVessel::TxReplyChat(const QString& replyText, const AuCommPacket& msg, AuCommStdChannel::RANGE range)
{
	if(replyText.isEmpty())
		return;
	const DaSector* sector = FindParentSector();
	if(!sector)
		return;

	// Mission orders can be sent to a private id. If the sender did that, reply to them privately.
	//DHH if(replyType == AuCommPacket::Chat && !msg.ToId().IsNull())
		//DHH replyType = AuCommPacket::ChatPrivate;

	AuCommChat mymsg(replyText, msg.ChannelName(), Id(), Name(), sector->Id(), Galaxy(), range,
			msg.Scope());

	/*
	if(replyType == AuCommPacket::ChatPrivate)
	{
		// Private reply requires recipient id to be attached as binary data.
		QByteArray binaryData;
		QDataStream out(&binaryData, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		msg.FromId() >> out;
		mymsg.SetBinaryData(binaryData);
	}
	*/

	// Scope data is required for certain message to be valid. It is ignored on Hail and named channels.
	if(mymsg.Scope() == AuCommStdChannel::CrewMates)
		mymsg.SetScopeData(Id().GetID());			// CrewMate scope needs ship id.
	else if(mymsg.Scope() == AuCommStdChannel::Company)
		mymsg.SetScopeData(msg.ScopeData());		// Respond with same company.
	else
		mymsg.SetScopeData(EmpireId());			// All other logical scopes need empire id.
	g_Au.SendAtomChat(mymsg);
}

void DaVessel::RequestBerth(const AuCommPacket& msg, AuRosterEntry::TYPE type, DaPerson* requestor)
{
	RequestBerth(msg, type, requestor->Name(), requestor->Id(), requestor->EmpireId());
}

// Request for berths can be accepted by the ship on the intercom channel if policy permits.
void DaVessel::RequestBerth(const AuCommPacket& msg, AuRosterEntry::TYPE type, const QString& reqName, const AuId& reqUnitId,
		quint32 reqEmpire)
{
	if(InArena())
		return;
	if(InDesignEnvironment())
		return;

	// Prepare to send a reply.
	AuCommPacket* replyMsg = 0;			// Reply message, if we decide to reply at all.
	AuId fromSectorId;
	const DaSector* sector = FindParentSector();
	if(sector)
		fromSectorId = sector->Id();

	if(!reqEmpire && !AuRosterEntry::IsPassenger(type))
	{
		// The requestor is asking for a crew berth of some kind and they are not affiliated with any empire.
		replyMsg = new AuCommMessage(AuCommMessage::RequestorHasNoEmpire, msg.ChannelName(), Id(), Name(), fromSectorId,
				Galaxy(), msg.ReceptionRange(), msg.Scope());
	}
	else if(BerthTypeOccupied(reqUnitId) == type)
	{
		// The requestor is already in that kind of berth.
		replyMsg = new AuCommMessage(AuCommMessage::RequestorAlreadyHasBerthType, msg.ChannelName(), Id(), Name(), fromSectorId,
				Galaxy(), msg.ReceptionRange(), msg.Scope());
	}
	else
	{
		// Check the ship's automatic acceptance policy.
		// Passenger berths are accepted when the ship's mission is to load passengers, overriding the automatic acceptance policy.
		// Troop berths are accepted when the ship's mission is to load troops, overriding the automatic acceptance policy.
		// Crew berths are accepted when the ship's mission is to load crew, overriding the automatic acceptance policy.
		AuMissionOrder::TYPE t = MissionOrderType();
		bool isBoarding = ((type == AuRosterEntry::PassengerHuman && t == AuMissionOrder::PassengersLoad) ||
				(type == AuRosterEntry::Troop && t == AuMissionOrder::TroopsLoad) ||
				(type == AuRosterEntry::Crew && t == AuMissionOrder::CrewLoad)) ? true:false;
		if(!isBoarding && !CanAcceptBerthType(type))
		{
			// The ship cannot accept that berth type automatically.
			replyMsg = new AuCommMessage(AuCommMessage::CannotAcceptBerthType, msg.ChannelName(), Id(), Name(), fromSectorId,
					Galaxy(), msg.ReceptionRange(), msg.Scope());
		}
		else
		{
			// Check political stance of ship's empire toward the requestor's empire.
			POLITICAL_STANCE stance = PoliticalStance(EmpireId(), reqEmpire);
			switch(type)
			{
			case AuRosterEntry::Captain:
			case AuRosterEntry::CaptainSlot:
				// Stations only grant captain's berth to members of its own empire.
				if(stance != POL_Empire && IsSpaceStation())
					replyMsg = new AuCommMessage(AuCommMessage::CaptBerthRestrictedStation, msg.ChannelName(), Id(), Name(),
							fromSectorId, Galaxy(), msg.ReceptionRange(), msg.Scope());
				else if(CaptainsBerthOccupied())
					replyMsg = new AuCommMessage(AuCommMessage::CaptBerthOccupied, msg.ChannelName(), Id(), Name(),
							fromSectorId, Galaxy(), msg.ReceptionRange(), msg.Scope());
				else
				{
					// The ship must reply appropriately, depending on the type of captain berth that is available. The 'slot' is a
					// place-holder for ships that have no berths.
					bool restricted = false;
					if(!IsPrivatelyOwned())
					{
						// Captaincy of subsidized vessels is restricted by export policies.
						const DgData& gd = g_Au.LockDataForRead();
						restricted = gd.EmpireCommerceRestricted(EmpireId(), CID_InfernoAtmosphere, stance);
						g_Au.UnlockData();
					}

					if(restricted)
						replyMsg = new AuCommMessage(AuCommMessage::CaptBerthRestricted, msg.ChannelName(), Id(), Name(),
								fromSectorId, Galaxy(), msg.ReceptionRange(), msg.Scope());
					else
					{
						bool reqIsNewOwner = false;
						if(IsPrivatelyOwned() && OwnerId().IsNull())
						{
							// Title to private ship with no owner is claimed by the new captain.
							SetPrivateOwner(reqUnitId, reqName);
							reqIsNewOwner = true;
						}

						if(CaptainsBerthType() == AuRosterEntry::CaptainSlot)
							replyMsg = new AuCommAcceptBerthCaptainSlot(reqUnitId, reqName, reqIsNewOwner, Id(), Name(),
									IsPrivatelyOwned(), msg.ChannelName(), Id(),
									Name(), fromSectorId, Galaxy(), msg.ReceptionRange(), msg.Scope());
						else
							replyMsg = new AuCommAcceptBerthCaptain(reqUnitId, reqName, reqIsNewOwner, Id(), Name(),
									IsPrivatelyOwned(), msg.ChannelName(), Id(),
									Name(), fromSectorId, Galaxy(), msg.ReceptionRange(), msg.Scope());

						// When the captaincy is granted, the empire of the ship is changed to the nu captain.
						SetEmpireId(reqEmpire);

						SetModified();
					}
				}
				break;

			case AuRosterEntry::OfficerHuman:
				// Ships only grant officer berths to members of its own empire.
				if(stance != POL_Empire)
					replyMsg = new AuCommMessage(AuCommMessage::OfficerBerthRestrictedEmpire, msg.ChannelName(), Id(), Name(),
							fromSectorId, Galaxy(), msg.ReceptionRange(), msg.Scope());
				else if(OfficerBerthsAvailable())
					replyMsg = new AuCommAcceptBerthOfficer(reqUnitId, reqName, Id(), Name(), IsPrivatelyOwned(),
							msg.ChannelName(), Id(), Name(),
							fromSectorId, Galaxy(), msg.ReceptionRange(), msg.Scope());
				else
					replyMsg = new AuCommMessage(AuCommMessage::NoOfficerBerth, msg.ChannelName(), Id(), Name(),
							fromSectorId, Galaxy(), msg.ReceptionRange(), msg.Scope());
				break;

			case AuRosterEntry::Crew:
				// Do not grant crew berths to enemies.
				if(stance == POL_Enemy)
					replyMsg = new AuCommMessage(AuCommMessage::OfficerBerthRestrictedEmpire, msg.ChannelName(), Id(), Name(),
							fromSectorId, Galaxy(), msg.ReceptionRange(), msg.Scope());
				else if(CrewBerthsAvailable())
					replyMsg = new AuCommAcceptBerthCrew(reqUnitId, reqName, Id(), Name(), IsPrivatelyOwned(),
							msg.ChannelName(), Id(), Name(),
							fromSectorId, Galaxy(), msg.ReceptionRange(), msg.Scope());
				else
					replyMsg = new AuCommMessage(AuCommMessage::NoCrewBerth, msg.ChannelName(), Id(), Name(),
							fromSectorId, Galaxy(), msg.ReceptionRange(), msg.Scope());
				break;

			case AuRosterEntry::PassengerHuman:
				// Do not grant passenger berths to enemies.
				if(stance == POL_Enemy)
					replyMsg = new AuCommMessage(AuCommMessage::OfficerBerthRestrictedEmpire, msg.ChannelName(), Id(), Name(),
							fromSectorId, Galaxy(), msg.ReceptionRange(), msg.Scope());
				else if(PassengerBerthsAvailable())
					replyMsg = new AuCommAcceptBerthPassenger(reqUnitId, reqName, Id(), Name(), IsPrivatelyOwned(),
							msg.ChannelName(), Id(), Name(),
							fromSectorId, Galaxy(), msg.ReceptionRange(), msg.Scope());
				else
					replyMsg = new AuCommMessage(AuCommMessage::NoPassengerBerth, msg.ChannelName(), Id(), Name(),
							fromSectorId, Galaxy(), msg.ReceptionRange(), msg.Scope());
				break;

			case AuRosterEntry::Troop:
				// Do not grant troop berths to enemies.
				if(stance == POL_Enemy)
					replyMsg = new AuCommMessage(AuCommMessage::OfficerBerthRestrictedEmpire, msg.ChannelName(), Id(), Name(),
							fromSectorId, Galaxy(), msg.ReceptionRange(), msg.Scope());
				else if(TroopBerthsAvailable())
					replyMsg = new AuCommAcceptBerthTroop(reqUnitId, reqName, Id(), Name(), IsPrivatelyOwned(),
							msg.ChannelName(), Id(), Name(),
							fromSectorId, Galaxy(), msg.ReceptionRange(), msg.Scope());
				else
					replyMsg = new AuCommMessage(AuCommMessage::NoTroopBerth, msg.ChannelName(), Id(), Name(),
							fromSectorId, Galaxy(), msg.ReceptionRange(), msg.Scope());
				break;

			case AuRosterEntry::Invalid:
			case AuRosterEntry::OfficerAI:
			case AuRosterEntry::PassengerAI:
				// Invalid type of berth to request. No need to reply to this.
				qWarning() << "warning: DaVessel::RequestBerth" << Id() << Name() << "invalid berth type requested" << type;
				return;
			}
		}
	}

	if(replyMsg)
	{
		// The ship has an empty berth.
		// The ship responds with a berth accepted message.
		// Ships normally respond at the same range the message was sent.
		if(replyMsg->Scope() == AuCommStdChannel::CrewMates)
			replyMsg->SetScopeData(Id().GetID());	// Crew channel needs ship's id to know the scope of the message.
		else if(replyMsg->Scope() == AuCommStdChannel::Company)
			replyMsg->SetScopeData(msg.ScopeData());		// Respond with same company.
		else
			replyMsg->SetScopeData(EmpireId());		// Other scopes need Empire id or nothing at all.
		g_Au.SendAtomChat(*replyMsg);
		delete replyMsg;
	}
}

double DaVessel::PilotAvoidRadius() const
{
	double avoidRad = RoughRadius();
	if(ShieldsWorkingAndUp())
	{
		// Allow for shields.
		AuPoint shield = ShieldRadii();
		if(avoidRad < shield.x)
			avoidRad = shield.x;
		if(avoidRad < shield.y)
			avoidRad = shield.y;
		if(avoidRad < shield.z)
			avoidRad = shield.z;
	}
	avoidRad += 50*FeetPerMeter();
	return avoidRad;
}

bool DaVessel::OkToOrient() const
{
	// Base class checks to see if my parent is an animal or vehicle. No need to check that.
	return IsOnGround();
}

bool DaVessel::Orient(const AuPoint& atLoc, const AuPoint& terNormal)
{
	AuAtomPos pos(Pos());
	if(!OkToOrient() || !IsDesignDataCacheValid())
	{
		SetOrientRotation(pos.Rotation());
		return false;	// Rotation was not changed.
	}

	bool oriented = false;
	AuPoint oldVec;
	bool rotvec = false;
	if(!GravityDriveWorking() || HasRocketDrive())
	{
		oldVec = pos.Transform().ProjectVector(Vector());
		rotvec = true;
	}
	AuPoint otn = pos.Transform().ProjectVector(TerrainNormal());
	AuPoint ogn = pos.Transform().ProjectVector(GravityNormal());

	if(LandingOrientation() == LORI_Tail)
	{
		// Orientation stands on its tail.
		AuPoint nuRot;
		if(!IsOnGround() ||
				IsOnHardFloor() ||	// Prevents creeping on building floors.
				/*IsOnMetal() ||*/
				IsOnWoodFloor())	// Prevents creeping on building floors.
		{
			// Orient level with gravity.
			// Calculate rotation to orient pointing along the up vector.
			AuPoint vz = pos.Transform().ProjectVector(AuPoint(0,0,1));
			vz.Unit();
			AuPoint vx = ogn.Cross(vz);
			vx.Unit();
			AuPoint vy = ogn;
			nuRot = AuPoint::CalculateRotation(vx,vy);
		}
		else
		{
			// Orient level with the terrain.
			// Calculate rotation to orient pointing along the up vector.
			AuPoint vz = pos.Transform().ProjectVector(AuPoint(0,0,1));
			vz.Unit();
			AuPoint vx = terNormal.Cross(vz);
			vx.Unit();
			AuPoint vy = terNormal;
			nuRot = AuPoint::CalculateRotation(vx,vy);
		}

		SetOrientRotation(nuRot);
		if(IsAtRest() && !nuRot.FuzzyEquals(pos.Rotation()))
		{
			AuMatrix4x4 tOrient;
			tOrient.Translate(atLoc);
			tOrient.Rotate(nuRot);
			pos.SetTransform(tOrient);
			ChangePos(pos);
			oriented = true;
		}
	}
	else if(IsOnHardFloor() || /*IsOnMetal() ||*/ IsOnWoodFloor())
	{
		// Orient level with gravity;
		oriented = DaCapsule::Orient(atLoc, ogn);
	}
	else
	{
		// Orient level with the terrain.
		oriented = DaCapsule::Orient(atLoc, terNormal);
	}

	if(!oriented)
		return false;

	// This atom's movement vector must be held in the direction it was going, sort of like real inertia.
	if(rotvec)
	{
		AuPoint nuVec = pos.Transform().UnProjectVector(oldVec);
		SetVector(nuVec);
	}
	AuPoint ngn = pos.Transform().UnProjectVector(ogn);
	SetGravityNormal(ngn);
	AuPoint ntn = pos.Transform().UnProjectVector(otn);
	SetTerrainNormal(ntn);
	return true;
}

double DaVessel::ObstacleRadius() const
{
	return RoughRadius();
}

QString DaVessel::Name() const
{
	QString s(AssignedName());
	if(!s.isEmpty())
		return s;
	else
		return Id().StringId();
}

QString DaVessel::NameWithEmpire() const
{
	// Wait for good design analysis to cache m_NameWithEmpire.
	if(!IsDesignDataCacheValid())
	{
		// Return only empire name and ship name.
		QString empire;
		const DgData& gd = g_Au.LockDataForRead();
		gd.EmpireName(EmpireId(), empire);
		g_Au.UnlockData();

		return empire + " " + Name();
	}

	m_NameWithEmpireLock.lockForRead();
	QString ename(m_NameWithEmpire);
	m_NameWithEmpireLock.unlock();

	if(ename.isEmpty())
	{
		// Cached name with empire is empty so acquire it now.
		QString empire;
		const DgData& gd = g_Au.LockDataForRead();
		gd.EmpireName(EmpireId(), empire);
		g_Au.UnlockData();

		// Construct a name that identifies my empire structure and name, e.g. Imperium Starship Atlatl.
		ename = empire + " " + SpacecraftStructureName(Structure()) + " " + Name();

		m_NameWithEmpireLock.lockForWrite();
		m_NameWithEmpire = ename;
		m_NameWithEmpireLock.unlock();
	}

	return ename;
}

// 'hit' is the atom that was hit.
// 'hitNormal' is the surface normal at the point of collision, relative to this atom.
// Returns the undampened bounce vector. The length is the speed in feet per second that this atom was moving when the hit
// occurred. It is simply the reflected movement vector when the hit occurred.
AuPoint DaVessel::MovementBlocked(DaAtom* hit, const AuPoint& hitLoc, const AuPoint& hitNormal, bool hitShield)
{
	if(hitNormal.IsZero())
	{
		ForceVector(AuPoint(0,0));
		return AuPoint(0,0);
	}
	return DaCapsule::MovementBlocked(hit, hitLoc, hitNormal, hitShield);
}

// Test for life support functioning.
bool DaVessel::LifeSupportWorking() const
{
	if(!IsDesignDataCacheValid())
		return true;	// Give the benefit of the doubt until the design is loaded.

	// 100% damage to life support will make it stop working.
	if(!GetPowerConsumption(SYS_LifeSupport))
		return false;

	// Everything looks good.
	return true;
}

// Returns an atom that is in 'gunRange' of 'gunLoc', which is relative to my parent world.
// 'gunnerEmpire' is the empire id of the gunner, who is at my parent world.
// 'gunnerPosture' is the posture of the gunner. Caller checks POSTURE_HoldFire before calling to save lots of redundant checks.
// 'targetLoc' is an output parameter relative to my parent world.
bool DaVessel::IsGuardTowerTarget(quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunLoc, float gunRange,
		AuPoint& targetLoc) const
{
	Q_UNUSED(gunnerPosture);
	if(IsNew() && !HasSubatoms())
		return false;
	if(!IsInAtmosphere())
		return false;	// Turrets don't reach out of the atmosphere.

	double range2 = gunLoc.Distance2(Location());
	if(range2 > gunRange*gunRange)
		return false;	// I am out of range.

	// Check for ship hiding in ground clutter.
	if(range2 > HideInGroundClutterRange2() && IsHidingInGroundClutter())
		return false;											// Out of sight range and hiding.

	if(PoliticalStance(gunnerEmpire, EmpireId()) != POL_Enemy)
		return false;	// I am not an enemy.

	if(IsNoncombatant())
		return false;	// I am a noncombatant unit.

	// I am a target.
	targetLoc = Transform().Project(TargetLocation());
	return true;
}

// Returns an atom that is in 'gunRange' of 'gunSysLoc', which is relative to the solar system.
// 'gunnerEmpire' is the empire id of the gunner, who is operating the weapon.
// 'targetAtom' and 'targetRange' are output parameters.
void DaVessel::FindClosestDefenseBaseTargetRecursive(int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
		const AuPoint& gunSysLoc, float gunRange, DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc)
{
	if(!IsGunnerDomainTarget(gunnerDomain, gunnerWorldId, AuId()))
		return;
	if(IsNew() && !HasSubatoms())
		return;
	if(IsNoncombatant())
		return;
	AuPoint mysysloc = EnvTransform().Project(TargetLocation());
	double range2 = gunSysLoc.Distance2(mysysloc);
	if(range2 > gunRange*gunRange)
		return;		// I am out of range of the weapon.

	// Check for ship hiding in ground clutter.
	if(range2 > HideInGroundClutterRange2() && IsHidingInGroundClutter())
		return;													// Out of sight range and hiding.

	if(PoliticalStance(gunnerEmpire, EmpireId()) != POL_Enemy)
		return;		// I am not an enemy of the gunner.

	SetInEnemyRange();			// We're in range of the bay weapon of a base that considers us an enemy.
	//qDebug() << "DaVessel::FindClosestDefenseBaseTarget" << Id() << Name() << "in range";

	if(targetAtom && range2 > targetRange*targetRange)
		return;		// I am not closer than the current target.

	//DHH Check for obstructions between the gun and me?

	// I am the closest target.
	targetAtom = this;
	targetRange = sqrt(range2);
	targetSysLoc = mysysloc;
}

// Return true if this atom can be targetted by a turret on a spacecraft.
// 'gunnerEmpire' is the empire id of the gunner. It is 0 when political stance is to be ignored.
// 'gunnerPosture' is the posture of the gunner. Caller checks POSTURE_HoldFire before calling to save lots of redundant checks.
// 'gunSysLoc' and 'gunSysVec' are relative to the solar system.
bool DaVessel::IsShipTurretTarget(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
		CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
		AuPoint& targetSysLoc) const
{
	Q_UNUSED(gunnerPosture);
	if(Is(shipId))
		return false;
	if(IsNew() && !HasSubatoms())
		return false;
	if(!IsGunnerDomainTarget(gunnerDomain, gunnerWorldId, shipId))
		return false;

	// Perform range test.
	AuPoint mysysloc = EnvTransform().Project(TargetLocation());
	AuPoint targetVec = mysysloc - gunSysLoc;		// Vector to target from gun's location.
	double range2 = targetVec.Length2();
	if(range2 > gunRange*gunRange)
		return false;

	// Check for ship hiding in ground clutter.
	if(range2 > HideInGroundClutterRange2() && IsHidingInGroundClutter())
		return false;											// Out of sight range and hiding.

	if(targetVec.Dot(gunSysVec) < 0)						// Test field of fire.
		return false;

	if(gunnerEmpire && PoliticalStance(gunnerEmpire, EmpireId()) != POL_Enemy)
		return false;	// I am not an enemy.

	targetSysLoc = mysysloc;
	return true;
}

// Returns an atom that is in 'gunRange' of 'gunSysLoc', which is relative to the solar system.
// 'gunnerEmpire' is the empire id of the gunner, who is operating the weapon.
// 'targetAtom' and 'targetRange' are output parameters.
void DaVessel::FindClosestShipTurretTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
		quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
		const DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const
{
	AuPoint myTargLoc;
	if(!IsShipTurretTarget(shipId, gunnerDomain, gunnerWorldId, gunnerEmpire, gunnerPosture, gunSysLoc, gunRange, gunSysVec,
				myTargLoc))
		return;

	//DHH Check for obstructions between the gun and me?

	// I am the closest target.
	targetAtom = this;
	targetRange = myTargLoc.Distance(gunSysLoc);
	targetSysLoc = myTargLoc;
}

bool DaVessel::IsShipWeaponTarget(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
		CITIZEN_POSTURE gunnerPosture, const AuPoint& shipSysLoc, float gunRange, AuPoint& targetSysLoc) const
{
	Q_UNUSED(gunnerPosture);
	if(Is(shipId))
		return false;
	if(IsNew() && !HasSubatoms())
		return false;
	if(IsNoncombatant())
		return false;
	if(!IsGunnerDomainTarget(gunnerDomain, gunnerWorldId, shipId))
		return false;

	// Perform range test.
	AuPoint mysysloc = EnvTransform().Project(TargetLocation());
	AuPoint targetVec = mysysloc - shipSysLoc;					// Vector to target from ship's location.
	double range2 = targetVec.Length2();
	if(range2 > gunRange*gunRange)
		return false;

	// Check for ship hiding in ground clutter.
	if(range2 > HideInGroundClutterRange2() && IsHidingInGroundClutter())
		return false;											// Out of sight range and hiding.

	if(gunnerEmpire)
	{
		if(PoliticalStance(gunnerEmpire, EmpireId()) != POL_Enemy)
			return false;	// I am not an enemy.

		if(gunnerPosture != POSTURE_Berserk && IsCoveredByATC())
		{
			// Pirates do not attack targets covered by ATC, unless they are berserk. Pirates are berserk when defending their
			// home base.
			bool isPirateAttacker = DgEmpire::IsPirate(gunnerEmpire);
			if(isPirateAttacker)
				return false;
		}
	}
	SetInEnemyRange();			// We're in range of the bay weapon of a spacecraft that considers us an enemy.
	//qDebug() << "DaVessel::IsShipWeaponTarget" << Id() << Name() << "in range";
	targetSysLoc = mysysloc;
	return true;
}

void DaVessel::FindClosestShipWeaponTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
		quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& shipSysLoc, float gunRange, const DaAtom*& targetAtom,
		double& targetRange, AuPoint& targetSysLoc) const
{
	AuPoint mysysloc;
	if(!IsShipWeaponTarget(shipId, gunnerDomain, gunnerWorldId, gunnerEmpire, gunnerPosture, shipSysLoc, gunRange, mysysloc))
		return;

	double range2 = mysysloc.Distance2(shipSysLoc);		// Vector to target from ship's location.
	if(targetAtom && range2 > targetRange*targetRange)
		return;		// I am not closer than the current target.

	//DHH Check for obstructions between the gun and me?

	// I am the closest target.
	SetInEnemyRange();			// We're in range of the bay weapon of a spacecraft that considers us an enemy.
	//qDebug() << "DaVessel::FindClosestShipWeaponTarget" << Id() << Name() << "in range";
	targetAtom = this;
	targetRange = sqrt(range2);
	targetSysLoc = mysysloc;
}

void DaVessel::FindClosestShipLockOnRecursive(int scannerDomain, const AuId& scannerWorldId, quint32 scannerEmpire,
		const AuId& scannerCapsuleId, const AuPoint& scannerSysLoc, float sensorRange,
		const DaVessel*& enemyVessel, double& vesselRange,
		const DaVessel*& nonEnemyStation, double& stationRange,
		const DaBd*& enemyMilBd, double& milRange,
		const DaBd*& anyBd, double& bdRange) const
{
	Q_UNUSED(enemyMilBd);
	Q_UNUSED(milRange);
	Q_UNUSED(anyBd);
	Q_UNUSED(bdRange);
	if(!IsGunnerDomainTarget(scannerDomain, scannerWorldId, scannerCapsuleId))
		return;
	if(IsNew() && !HasSubatoms())
		return;

	// Perform range test.
	AuPoint targetVec = EnvLocation() - scannerSysLoc;		// Vector to target from ship's location.
	double range2 = targetVec.Length2();
	if(range2 > sensorRange*sensorRange)
		return;

	// Check for ship hiding in ground clutter.
	if(range2 > HideInGroundClutterRange2() && IsHidingInGroundClutter())
		return;													// Out of sight range and hiding.

	if(PoliticalStance(scannerEmpire, EmpireId()) == POL_Enemy)
	{
		// I am an enemy spacecraft to the scanner.
		if(enemyVessel && range2 > vesselRange*vesselRange)
			return;		// I am not closer than the current enemy spacecraft.

		// I am the closest enemy spacecraft.
		enemyVessel = this;
		vesselRange = sqrt(range2);
	}
	else if(IsSpaceStation())
	{
		// I am a non-enemy station to the scanner.
		if(nonEnemyStation && range2 > stationRange*stationRange)
			return;		// I am not closer than the current non enemy station.

		// I am the closest non enemy station.
		nonEnemyStation = this;
		stationRange = sqrt(range2);
	}
}

const DaAtom* DaVessel::FindClosestMissileTarget(const AuId& launcherId, int missileDomain, const AuId& missileWorldId,
		quint32 missileEmpireId, const AuPoint& missileSysLoc, const DaAtom* bestAtom, AuPoint& bestSysLoc,
		double& bestDistance) const
{
	if(!IsGunnerDomainTarget(missileDomain, missileWorldId, launcherId))
		return bestAtom;
	if(Is(launcherId))
		return bestAtom;

	// Missiles only lock on to neutral and enemy targets.
	POLITICAL_STANCE stance = PoliticalStance(missileEmpireId, EmpireId());
	if(stance != POL_Enemy && stance != POL_Neutral)
		return bestAtom;

	double dist = EnvLocation().Distance(missileSysLoc);
	if(bestDistance < 0. || dist < bestDistance)
	{
		bestDistance = dist;
		bestSysLoc = EnvLocation();
		return this;
	}

	// Don't check my subatoms; they can't be targetted.
	return bestAtom;
}

void DaVessel::FilterFinalMoveVector(AuPoint& vector) const
{
	bool warping = false;
	quint16 warpDrive = 0;
	if(HasWarpFTLDrive())
	{
		warpDrive = FTLWarpDrive();
		warping = IsFTLDriveHot(warpDrive);
	}
	if(warping)
	{
		// Warp drive controls speed like gravity drive controls inertia.
		quint8 warpSetting = FTLDriveSetting(warpDrive) + 1;
		if(InArena() || InDesignEnvironment())
		{
			// Calculate current speed of the ship. This is the movement vector.
			// Light speed is 10 minutes per parsec.
			// Warp one is 10x light speed; warp 9 is 30x light speed.
			double speed = (double)warpSetting * SpeedOfWarp();
			vector = AuPoint(0, speed, 0);
			return;
		}

		/*
		// Speed is directly proportional to the distance from the photosphere of the nearest star.
		AuPoint starLoc;
		const DaStar* star = FindParentStarInEnv();
		if(star)
		{
			// I am subordinate to a star.
			GetLocationRelativeToParent(starLoc, star);
		}
		else
		{
			// In deep space I am not subordinate to any star.
			starLoc = EnvLocation();	// Primary star location corresponds to center of solar system.
			//const DaSolarSystem* system = FindParentSolarSystem();
			//star = system->FindPrimaryStar();
		}
		*/

		// Calculate current speed of the ship. This is the movement vector.
		// Light speed is 10 minutes per parsec.
		// Warp one is 10x light speed; warp 9 is 30x light speed.
		double speed = warpSetting * SpeedOfWarp();
		vector = AuPoint(0, speed, 0);

		/*
		// Distance in feet to the photosphere of the star, photon sphere of a black hole.
		double starDist = starLoc.Length() - star->AtmosphereRadius() - RoughRadius();
		if(starDist > 0)
		{
			// Warp factors equate to speed settings.
			// Warp 1 speed is achieved at 1/5th parsec.
			// Warp speed maximum is limited by the warp factor setting.
			double warpFactor = starDist / (FeetPerSolarSystem()/2);
			if(warpFactor > warpSetting)
				warpFactor = warpSetting;
			//qDebug() << "DaVessel::FilterFinalMoveVector warpFactor" << warpFactor;

			// Calculate current speed of the ship. This is the movement vector.
			// Light speed is 10 minutes per parsec.
			// Warp one is 10x light speed; warp 9 is 30x light speed.
			double speed = warpFactor * SpeedOfWarp();
			vector = AuPoint(0, speed, 0);
		}
		else
		{
			// Warp drive is motionless inside the photosphere of the star.
			vector = AuPoint(0, 0, 0);
		}
		*/
	}
	else
	{
		// When sitting on the ground, eliminate the downward portion of the movement vector or we creep on hills.
		if(IsInGravity() && IsTouchingGround())
		{
			if(LandingOrientation() == LORI_Tail)
			{
				// Spacecraft orients on its tail with +Y up.
				if(vector.y < 0 && fabs(vector.x) < 1 && fabs(vector.z) < 1)
					vector.y = 0;
			}
			else
			{
				// When sitting on the ground, eliminate the downward portion of the movement vector or we creep on hills.
				if(vector.z < 0 && vector.Length2_2D() <= 1)
					vector.z = 0;
			}
		}

		// Limit movement vector to the speed of light.
		// This causes a ship with warp drives going faster than light to suddenly drop to light speed when the warp drives expire.
		if(!vector.IsZero() && vector.Length2() > SpeedOfLight2())
			vector.Unit(SpeedOfLight());
	}
}

void DaVessel::FilterPostMoveVector(AuPoint& vector) const
{
	if(!IsInGravity())
		return;
	// When sitting on the ground, eliminate the downward portion of the movement vector.
	// Eliminate gravity vector when blocked to help get free from the side of buildings.
	if(IsTouchingGround())
	{
		if(LandingOrientation() == LORI_Tail)
		{
			// Spacecraft orients on its tail with +Y up.
			if(vector.y < 0)
				vector.y = 0;
		}
		else
		{
			// Everything else orients with +Z up.
			if(vector.z < 0)
				vector.z = 0;
		}
	}
}

bool DaVessel::IsReadyToWormhole() const
{
	if(!HasWormholeFTLDrive())
		return false;

	quint16 wormFTL = FTLWormholeDrive();
	if(FTLDriveState(wormFTL) != FTL_DRIVE_On)
		return false;

	FTL_COMMIT commit = FTLDriveCommitState(wormFTL);
	return commit >= FTL_COMMIT_ChargedBegin && commit <= FTL_COMMIT_ChargedEnd;
}

// Test for gravity drive on and functioning.
bool DaVessel::GravityDriveWorking() const
{
	// Must be ON to work.
	GRAVITY_DRIVE_STATE v = GravityDriveState();
	if(v != GD_On && v != GD_Hold)
		return false;

	// Capacitor must have some power for system to work.
	if(CapacitorLevel() <= 0)
		return false;

	// 100% damage to gravity drive will make it stop working.
	if(IsDesignDataCacheValid() && !GetPowerConsumption(SYS_ManeuverDrive))
		return false;	// Gravity drive is inop.

	// Everything looks good. Gravity drive must be working.
	return true;
}

// A false return results in destruction if this ship actually does depart the solar system. See DepartingSolarSystem.
// A true return enables this ship to trigger pirate encounters.
bool DaVessel::IsSafeToLeaveSolarSystem() const
{
	if(IsInTow())
		return true;

	if(IsPirateWithFreeWill())
	{
		// Pirate ship acts like it is safe to leave the solar system but actually gets destroyed if it does so.
		// Ship may leave solar system if there is a person on board who is online.
		// This does not apply when someone is the captain, which could only be true if they were an agent. ;)
		if(FindPersonOnLineRecursive())
			return true;									// Person aboard who is online is ok to leave the system.

		// Pirates can never leave the system without a person aboard who is online. They just get destroyed when they do it.
		return false;
	}
	else
	{
		// Ship may leave the solar system if it has an AI officer aboard. The officer does not have to have the con.
		if(HasFirstOfficer())
			return true;									// Officer aboard is ok to leave the system.

		// Ship may leave solar system if there is a person on board.
		if(FindPersonRecursive())
			return true;									// Person aboard is ok to leave the system.

		// A ship must not leave the solar system with no first officer or person aboard.
		return false;										// Must not leave the solar system.
	}
}

bool DaVessel::DepartingSolarSystem()
{
	if(IsTraveling())
	{
		// This should not happen.
		if(g_Au.m_LogMessages & LOG_Travel)
			qDebug("DaVessel::DepartingSolarSystem %d %s is already traveling.", Id().GetID(), (const char*)Name().toLatin1());
		return true;
	}

	if(!IsSafeToLeaveSolarSystem())
	{
		// Unmanned spacecraft do not leave the solar system.
		DamageAndDestroy("Collided with dark matter object.");
		return false;
	}
	return true;
}

unsigned int DaVessel::IsTocker() const
{
	unsigned int ms = DaCapsule::IsTocker();
	if(!IsNormal())
		return ms;
	// Here we know ms is at least d_MSecPerMin.

	if(ms > 10*d_MSecPerSec)
		ms = 10*d_MSecPerSec;

	if(ms > d_MSecPerSec)
	{
		if(FTLDriveCommitState(0) != FTL_COMMIT_Dormant)
			ms = d_MSecPerSec;
		else
		{
			FTL_DRIVE_STATE fs = FTLDriveState(0);
			if(fs != FTL_DRIVE_Off && fs != FTL_DRIVE_Invalid && fs != FTL_DRIVE_New)
				ms = d_MSecPerSec;
		}
	}

	if(ms > d_MSecPerSec && (!IsNew() || IsPowerPlantOn()))
		ms = d_MSecPerSec;

	if(ms > d_MSecPerSec && IsMissionActive())
	{
		AuMissionOrder::TYPE curOrder = MissionOrderType();
		// Execute the scuttle order, if the current order is Scuttle.
		if(curOrder == AuMissionOrder::Scuttle)
			ms = d_MSecPerSec;
		// Configure harvester bays when that is the current mission order.
		else if(curOrder == AuMissionOrder::HarvestAtmosphere ||
				curOrder == AuMissionOrder::HarvestMinerals ||
				curOrder == AuMissionOrder::HarvestSea ||
				curOrder == AuMissionOrder::HarvestWell ||
				curOrder == AuMissionOrder::HarvestParticles)
			ms = d_MSecPerSec;
	}

	if(ms > d_MSecPerSec && ShieldScrambled())
		ms = d_MSecPerSec;
	return ms;
}

void DaVessel::ShieldStabilize()
{
	if(!ShieldScrambled())
		return;
	m_ShieldsLock.lockForWrite();
	for(int side=0; side<6; side++)
		if(m_ShieldScram[side])
			m_ShieldScram[side]--;
	m_ShieldsLock.unlock();
	SetModified();
}

void DaVessel::Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans)
{
	// Tock base class and all subatoms.
	DaCapsule::Tock(last, now, scripts, tockTrans);
	if(!IsNormal())
		return;

	// Decay console occupancy states.
	m_ConCaptAI = 0;			// 0=unknown, 1=no, 2=yes if AI has the con
	m_ConEngineerAI = 0;		// 0=unknown, 1=no, 2=yes
	m_ConEngineerPerson = 0;	// 0=unknown, 1=no, 2=yes
	m_ConNavigatorAI = 0;		// 0=unknown, 1=no, 2=yes
	m_ConNavigatorPerson = 0;	// 0=unknown, 1=no, 2=yes
	m_ConPilotAI = 0;			// 0=unknown, 1=no, 2=yes
	m_ConPilotPerson = 0;		// 0=unknown, 1=no, 2=yes
	m_ConPowerRelayAI = 0;		// 0=unknown, 1=no, 2=yes
	m_ConPowerRelayPerson = 0;	// 0=unknown, 1=no, 2=yes
	m_ConSensorAI = 0;			// 0=unknown, 1=no, 2=yes
	m_ConSensorPerson = 0;		// 0=unknown, 1=no, 2=yes
	m_ConShieldAI = 0;			// 0=unknown, 1=no, 2=yes
	m_ConShieldPerson = 0;		// 0=unknown, 1=no, 2=yes
	m_ConWeaponAI = 0;			// 0=unknown, 1=no, 2=yes
	m_ConWeaponPerson = 0;		// 0=unknown, 1=no, 2=yes

	// None of this stuff needs to be done in the first tock cycle.
	if(last.IsNull())
		return;

	// Close automatic doors that are open when nobody is near them.
	// Every other second is often enough.
	CloseAutomaticDoors();
	tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));

	// Kick loose from 'at rest' state when appropriate.
	if(StartMoving())
		SetModified();

	// Conditions are ticked so they can periodically affect this unit.
	TickConditions();
	tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));

	// The capacitor needs to be cycled to generate power, even on nu ships. When a person sits at the helm of a 'nu' ship,
	// turning on the power plant does not unset the 'nu' state. The capacitor must start charging though.
	int secCycles = last.ElapsedSecTicks(now);
	if(secCycles && (m_bPowerPlantOn || !IsNew()))
	{
		// CycleCapacitor is called once per second.
		CycleCapacitor();
		if(last.ElapsedHourTicks(now))
			SetModified();				// Trigger db save once in a while to save CycleCapacitor result.

		if(!IsNew())
		{
			// FTL drive operation.
			if(FTLDriveCommitState(0) != FTL_COMMIT_Dormant)
				CycleFTLDrive();
			else
			{
				FTL_DRIVE_STATE fs = FTLDriveState(0);
				if(fs != FTL_DRIVE_Off && fs != FTL_DRIVE_Invalid && fs != FTL_DRIVE_New)
					CycleFTLDrive();
			}

			m_ControlLock.lockForRead();
			bool c = m_ThrustLateral ||
					m_ThrustLong ||
					m_ThrustVertical ||
					(m_wActions & (ACT_TurnMask|ACT_PitchMask|ACT_RollMask|ACT_BrakeMask));
			m_ControlLock.unlock();
			if(c)
			{
				// AI pilot does not depend on control decay.
				if(IsPilotMannedByPerson() || !IsPilotMannedByAI())
					DecayMoveActions();
			}
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	// Report my position to the master server every 10 minutes, for the star map.
	if(last.IsNewPeriod(now, d_MSecPerSec*10*60, Id().GetID()))
	{
		ReportPosition();
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	if(IsNew())
	{
		// Post a request for a captain once per hour.
		if(CaptainId().IsNull() && !InArena() && !InDesignPreview())
		{
			// Report every 2 hours.
			if(last.IsNewPeriod(now, d_MSecPerHr*2, Id().GetID()))
			{
				DaSolarSystem* system = FindParentSolarSystem();
				Q_ASSERT(system);
				const DaSector* sector = dynamic_cast<const DaSector*>(system->Parent());
				Q_ASSERT(sector);
				const DaGalaxy* galaxy = sector->FindParentGalaxy();
				Q_ASSERT(galaxy);
				const DaWorld* planet = FindParentPlanetInEnv();
				if(!planet)
				{
					if(!FindPersonRecursive())
					{
						qDebug() << "DaVessel::Tock" << Id() << Name() << "nu ship not at planet, nobody aboard, KILLED!";
						Kill();
						return;
					}
				}

				if(IsInAtmosphere())
				{
					// New ship is inside the atmosphere of a planet.
					DaAtom* city;
					if(!BdId().IsNull())
						city = planet->FindBd(BdId());
					else
						city = 0;

					if(city)
					{
						// Bonus! This ship is in a city.
						if(g_Au.m_EnableJobPosting)
						{
							QString desc = tr("%1 class %2").arg(DesignName()).arg(Description());
							AuJobCommandShipOnGround job(Id(), Name(), desc, city->Name(), system->Id(), system->Name(),
									sector->Name() + " " + sector->Description() + ", " + galaxy->Name());
							emit PostJobOffer(AuJobOffer(Name(), EmpireId(), sector->Location(), galaxy->Galaxy(), &job));
						}
					}
					else
					{
						// This ship should no longer be treated as nu.
						SetNotNew();
						SetModified();
					}
				}
				else
				{
					// New ship is in orbit.
					if(g_Au.m_EnableJobPosting)
					{
						QString desc = tr("%1 class %2").arg(DesignName()).arg(Description());
						AuJobCommandShipInOrbit job(Id(), Name(), desc, planet->Name(), system->Id(), system->Name(),
								sector->Name() + " " + sector->Description() + ", " + galaxy->Name());
						emit PostJobOffer(AuJobOffer(Name(), EmpireId(), sector->Location(), galaxy->Galaxy(), &job));
					}
				}
			}
		}

		// Remaining actions are not performed on "nu" spacecraft.
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
		return;
	}

	if(secCycles)
	{
		// Deactivate active transporter rooms.
		if(HasTransporter())
			DeactivateTransporterSystems();

		// Reduce shield destabilization values, if any.
		ShieldStabilize();

		// Test for solar radiation damage if my parent is a sun.
		const DaStar* parentStar = dynamic_cast<const DaStar*>(Parent());
		if(parentStar)
			ApplySolarRadiation(parentStar, DesignHitsStructural());
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	// Remaining actions are not performed in the design environment.
	if(InDesignEnvironment())
		return;

	// If this ship is positioned at an arriving wormhole, we must reposition to the proper location.
	DaSolarSystem* pmyss = dynamic_cast<DaSolarSystem*>(Parent());
	if(pmyss)
		EmergeFromWormhole(pmyss);

	// Test for proximity to next waypoint.
	if(HasWaypoint())
	{
		if(!pmyss)
			pmyss = dynamic_cast<DaSolarSystem*>(FindEnvironmentAtom());
		if(pmyss)
		{
			const DaAtom* destAtom = 0;
			AuPoint fRelLoc;
			m_WaypointLock.lockForRead();
			if(!m_aWaypoints.empty())
			{
				const DeWaypoint& wp = m_aWaypoints[0];
				destAtom = wp.Location(pmyss, fRelLoc);
			}
			m_WaypointLock.unlock();
			if(destAtom)
			{
				m_MissionLock.lockForRead();
				bool isPatrol = m_Mission.CurrentOrder().IsPatrol();
				m_MissionLock.unlock();
				if(isPatrol || !pmyss->IsWormholeAt(fRelLoc))
				{
					// The waypoint is in my solar system and it is not a wormhole or I am just patroling.
					// Way points to go through wormholes are completed in other code, when the ship enters/exits the wormhole.
					// See TraversedWormhole.
					bool waypointReached = false;	// Set true if we decide the next waypoint has been reached.
					const DaWorld* planet = dynamic_cast<const DaWorld*>(destAtom);
					if(planet && WaypointCount() == 1)
					{
						// Next waypoint is the final waypoint at a planet; we are there when it is my parent planet and we are
						// close to its orbit zone.
						if(planet == FindParentPlanetInEnv())
						{
							AuPoint fMyLoc;
							GetLocationRelativeToParent(fMyLoc, planet);
							if(fMyLoc.Distance(fRelLoc) < 100*FeetPerMeter())
								waypointReached = true;				// I'm close enough to the waypoint at the planet.
						}
					}
					else
					{
						// Waypoint is not a planet or not my final waypoint.
						AuPoint fMyLoc = EnvLocation();
						double closeEnough;
						if(WaypointCount() == 1)
							closeEnough = 1000 * FeetPerMeter();	// Stop close to the final waypoint.
						else
							closeEnough = FeetPerOrbit()/2;	// Intermediate waypoints like enough coarseness to avoid planets.
						closeEnough *= closeEnough;	// squared
						if(fMyLoc.Distance2(fRelLoc) < closeEnough)
							waypointReached = true;			// I'm close enough to the way point to call it good.
					}
					if(waypointReached)
						RemoveWaypoint(0);	// Remove the first waypoint in the route.
				}
			}
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	if(last.ElapsedHourTicks(now) > 0 && !IsPirate() && !InArena())
	{
		// An hour has ticked over.
		// Check for monster encounter once per hour.
		MonsterEncounter();

		// Notify system of my presense.
		if(!pmyss)
			pmyss = dynamic_cast<DaSolarSystem*>(FindEnvironmentAtom());
		if(pmyss)
			pmyss->SetNonPirateShipPresent(HighestSystemQuality());
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	if(last.ElapsedMinTicks(now) > 0)
	{
		// Poop an updated position report into the commander's log.
		if(!IsPirateWithFreeWill() && !InArena())
		{
			// This is not a free roaming pirate. Those guys don't maintain an event log.
			DaSentient* firstOfficer = FindOfficer();			// Find any officer, even if they don't have the con.
			if(firstOfficer)
			{
				// Found an officer to keep a log.
				// Create a concise statement of location information for the log.
				int type = 0;	// 0 = Lost in space
								// 1 = On the ground at a planet
								// 2 = Above planet inside atmosphere
								// 3 = Above planet outside atmosphere, not moving
								// 4 = Above planet outside atmosphere, moving
								// 5 = In system, not moving
								// 6 = In system, moving
								// 7 = Deep space, not moving
								// 8 = Deep space, moving
				QString cityName, strLongLat, planetName;

				const DaWorld* planet = FindParentPlanetInEnv();
				if(planet)
				{
					planetName = planet->HtmlName();

					if(IsOnGround())
					{
						// Determine my longitude/latitude string on this world. We don't report longitude and latitude when
						// orbiting/moving or we create a lot of useless position log entries that vary only by lat/long.
						type = 1;
						const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(planet);
						if(llworld)
							strLongLat = GlobeLL().ToString();

						if(!BdId().IsNull())
						{
							const DaBd* bd = planet->FindBd(BdId());
							if(bd)
								cityName = HtmlName(bd->CityOrBuildingName());
						}
					}
					else if(IsInAtmosphere())
						type = 2;
					else if(!IsMoving())
						type = 3;
					else
						type = 4;
				}
				else if(FindParentStarInEnv())
				{
					if(!IsMoving())
						type = 5;
					else
						type = 6;
				}
				else if(!IsMoving())
					type = 7;
				else
					type = 8;

				QString systemName;
				if(!pmyss)
					pmyss = dynamic_cast<DaSolarSystem*>(FindEnvironmentAtom());
				if(pmyss && !pmyss->IsEmptySystem())
					systemName = tr("%1 %2").arg(pmyss->HtmlNameWithSystem()).arg(pmyss->Description());

				QString sectorName;
				const DaSector* sector = FindParentSector();
				if(sector)
					sectorName = tr("%1 %2").arg(sector->HtmlName()).arg(sector->Description());

				QString locText = QString("%1|%2|%3|%4|%5|%6")
					.arg(type)
					.arg(cityName)
					.arg(strLongLat)
					.arg(planetName)
					.arg(systemName)
					.arg(sectorName);
				firstOfficer->LogEvent(AuEventLogEntry::Position, locText);
			}
		}

		if(IsSpaceStation())
		{
			if(IsOnGround() && dynamic_cast<const DaWorld*>(Parent()))
			{
				// Stations that touch the terrain are destroyed instantly.
				//qDebug("DaVessel::Tock %d destroying station on the ground", Id().GetID());
				DamageAndDestroy("Crashed into terrain.");
			}
		}

		if(!IsInSpacecraft())
		{
			// People in cities notice the presense of enemy units. This causes workers to stop spawning which relieves a genocide
			// exploit. They are safer if they stay indoors.
			if(!InBdCity().IsNull() && InCityEmpire() != EmpireId())
			{
				// This unit is in a city.
				DaWorld* planet = FindParentPlanetInEnv();
				if(planet)
				{
					DaBd* bd = planet->FindBd(BdId());
					if(bd && bd->Residents() > 0)
					{
						// City with people might see something.
						if(bd->PoliticalStanceMakeContact(this) == POL_Enemy)
							bd->LogEvent(AuEventLogEntry::EnemySpacecraftSpotted, NameWithEmpire());
					}
				}
			}
		}

		// Spacecraft replenish their vehicle bays with vehicles from the solar system.
		if(m_AutoFetchVehicles)
			FetchVehicle();

		// Monitor Damage Chance for Overclocked systems in use.
		if(TestPowerSystem())
			return;	// Ship was destroyed.

		// The scene server needs to know what channels are being monitored by atoms. This is done using the global data object.
		// All ships with crew or troops monitor the Crew channel so they can answer beam-up requests and debark troops.
		// All stations with transporter rooms monitor the Hail channel and and a channel named after them so they can answer
		// beam-up requests.
		// Channels decay after 20 minutes. I'll refresh mine every 15 minutes.
		if(now.MinsSinceEpoch()%15 == 0)
			MonitorCommChannels();

		if(!IsPirate() && IsSpaceStation() && SensorsWorking() && !InArena())
		{
			float sensorRange = SensorRange();

			// Determine if we need to report neutral cities and stations.
			CheckTrespass(sensorRange);

			// Notify cities on the planet if there are any enemy units on my sensors.
			// This is not done for stations with AI officers because AI officers send their own ship's log that includes an enemy
			// contact log.
			// Pirate stations don't do this.
			m_RosterLock.lockForRead();
			bool hasaio = m_Roster.HasAIOfficer();
			m_RosterLock.unlock();
			if(!hasaio)
			{
				DaWorld* planet = FindParentPlanetInEnv();
				if(planet)
				{
					if(!pmyss)
						pmyss = dynamic_cast<DaSolarSystem*>(planet->FindEnvironmentAtom());
					if(pmyss)
					{
						AuId gunnerWorldId;
						int gunnerDomain = GunnerDomain(gunnerWorldId);

						const DaAtom* targetAtom = 0;
						double targetRange = 0;
						AuPoint targetSysLoc;
						pmyss->FindClosestShipWeaponTargetRecursive(Id(), gunnerDomain, gunnerWorldId, EmpireId(),
								POSTURE_Offensive, EnvLocation(), sensorRange, targetAtom, targetRange, targetSysLoc);
						if(targetAtom)
						{
							AuSensorBlipList sensorBlips;
							PerformSensorConsoleScan(sensorBlips);	// Get updated sensor scan data
							planet->ReportEnemySensorContact(pmyss, EmpireId(), sensorBlips, sensorRange);
						}
					}
				}
			}
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	// Check for pirate encounter.
	if(!IsInSpacecraft() &&											// I must not be aboard another spacecraft.
			!IsCoveredByATC() &&									// I must be outside radar coverage.
			!IsPirateCommander() &&									// I must not be commanded by a pirate.
			IsInAnyScene() &&										// Quick test to avoid FindPersonOnLine when it isn't needed.
			DaAtom::FindPersonOnLine())								// Non-recursive check for anybody aboard who is on line.
	{
		// Determine a period of time at which to check for a new pirate encounter.
		int period;
		if(IsMoving())
			period = d_MSecPerMin*12;				// Make this check more often while moving.
		else
			period = d_MSecPerMin*36;				// Make this check rarely while not moving.
		if(last.IsNewPeriod(now, period, Id().GetID()))
		{
			// Don't do pirate encounters when immediately subordinate to the solar system. This indicates deep space.
			if(!pmyss)
				pmyss = dynamic_cast<DaSolarSystem*>(FindEnvironmentAtom());
			if(pmyss && !pmyss->Is(ParentId()))
			{
				// My environment is a solar system, it is not my immediate parent, and I am not in a designer.
				pmyss->PirateEncounter(this, false);
			}
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	if(IsSpaceStation())
	{
		// Pirate stations do not get repaired or resupplied.
		if(!IsPirate() && !InArena())
		{
			if(last.IsNewPeriod(now, d_MSecPerHr, Id().GetID()))
			{
				// Hour period.
				if(g_Au.m_EnableUnitDecay && ScuttleExcessStation())
					return;	// This station was scuttled.
				if(g_Au.m_EnableStationResupply)
					RepairAndResupplyStation();
				FillShipments(true);			// Fill pending shipments and ship the ones that are full.
			}
			else if(last.IsNewPeriod(now, 6*d_MSecPerMin, Id().GetID()))
			{
				// Six minute period.
				if(g_Au.m_EnableStationResupply && g_Au.IsSceneSolo())
					RepairAndResupplyStation();	// Every 6 minutes in solo play or it might not happen.
				FillShipments(false);			// Fill pending shipments.
			}
		}

		if(IsCommandedByAI() || IsPilotMannedByAI())
		{
			if((!LastSOS().isValid() || LastSOS().ElapsedSec() > 180) && !IsMoored())
			{
				if(!GravityDriveWorking())
					SendSOS(0, "", 0);							// Station Maneuver Drive
				else if(FuelQuantity() < 1)
					SendSOS(1, "", (int)CapacitorPercent());	// Station Fuel
			}
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}
	else if(IsCommandedByAI() || IsPilotMannedByAI())
	{
		if((!LastSOS().isValid() || LastSOS().ElapsedSec() > 180) && !IsMoored())
		{
			if(!GetPowerConsumption(SYS_ManeuverDrive))
			{
				// Gravity drive is broken.
				if(!IsMoving())
				{
					if(BdId().IsNull())
						SendSOS(2, "", 0);						// Ship Maneuver Drive, Marooned
				}
				else
					SendSOS(3, "", 0);							// Ship Maneuver Drive, Adrift
			}
			else if(FuelQuantity() < 1)
			{
				if(IsOnGround() && !IsMoving())
				{
					if(BdId().IsNull())
						SendSOS(4, "", 0);						// Ship Fuel, Marooned
				}
				else
					SendSOS(5, "", (int)CapacitorPercent());	// Ship Fuel
			}
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	// Reconcile the roster of AI-controlled ships once per day.
	if(last.ElapsedDayTicks(now) > 0)
		ReconcileRoster();

	if(!IsSpaceStation())
	{
		if(!pmyss)
			pmyss = dynamic_cast<DaSolarSystem*>(FindEnvironmentAtom());
		if(pmyss)
		{
			// I am in a solar system.
			if(!IsAtRest())
			{
				// The solar system needs to keep ticking if I am in motion.
				// This is not done for stations just to avoid extra work.
				pmyss->SetLastFlagInfluence();	// This keeps the system ticking without other consequences (like officer visit).
			}

			if(!InArena())
			{
				// I am not a station and I am not in an arena.
				AuElapsedTimer t(LastATCContact());
				if(!t.isValid() || t.ElapsedSec() > 30)
					ATCContact(pmyss);
			}
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	// Decay for garbage collection.
	if(!IsInAnyScene() && !InArena())
	{
		// Execute orders that are handled in Tick when I am in a scene. Otherwise they may not be done until I am in a scene again.
		// Execute the scuttle order, if the current order is Scuttle.
		AuMissionOrder::TYPE curOrder = MissionOrderType();
		if(curOrder == AuMissionOrder::Scuttle)
			DoScuttleOrder();								// Execute the scuttle order.
		// Configure harvester bays when that is the current mission order.
		else if(curOrder == AuMissionOrder::HarvestAtmosphere ||
				curOrder == AuMissionOrder::HarvestMinerals ||
				curOrder == AuMissionOrder::HarvestSea ||
				curOrder == AuMissionOrder::HarvestWell ||
				curOrder == AuMissionOrder::HarvestParticles)
			ConfigureHarvester();
		else if(IsPirateWithFreeWill())
		{
			// Pirate ships are destroyed after a certain amount of time to ensure they do not accumulate in unwanted areas.
			// This is not done when someone is the captain, which could only be true if they were an agent. ;)
			if(IsSpaceStation())
			{
				// Stations live for up to a week.
				if(CreatedTime().ElapsedMin() > 7*24*60)
				{
					// This station has been alive for a week. This life limit insures no pirate station survives forever.
					qDebug() << "DaVessel::Tock" << Id() << Name() << "Pirate station design" << DesignId()
						<< "alive for over a week, scuttle in 3 min.." << SubatomCount();
					MissionScuttle(3 * 60);
				}
			}
			else
			{
				// Ships live for up to an hour, only 15 minutes if they haven't moved.
				if(PosChangeTime().ElapsedMin() > 15)
				{
					// Ship has not moved in 15 minutes.
					qDebug() << "DaVessel::Tock" << Id() << Name() << "Pirate ship design" << DesignId()
						<< "hasn't moved in 15 minutes, scuttle in 3 min." << SubatomCount();
					MissionScuttle(3 * 60);
				}
				else if(CreatedTime().ElapsedMin() > 60)
				{
					// Ship has been alive for an hour. This life limit insures no pirate ship survives forever.
					qDebug() << "DaVessel::Tock" << Id() << Name() << "Pirate ship design" << DesignId()
						<< "alive for an hour, scuttle in 3 min." << SubatomCount();
					MissionScuttle(3 * 60);
				}
			}
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	// Decay is not applied while in space and under power. If this guy runs out of power, it will crash somewhere, which will
	// clean it up. That could take a long time.
	// Decay is not applied when inside a spacecraft. The host spacecraft will be subject to its own decay rules, which will
	// destroy this guy when it goes.
	// Decay is not applied when sitting on a road slab if the ship has any officer, AI or human.
	bool canDecay = true;
	if(!g_Au.m_EnableUnitDecay)
		canDecay = false;
	else if(InArena())
	{
		canDecay = false;

		// Spacecraft in arena are destroyed if they depart the bounds.
		if(InArenaOutsideBounds())
			DamageAndDestroy(tr("Departed Arena Bounds."));
	}
	else if(IsInsideCapsule())
	{
		// Ships do not decay when inside other ships.
		canDecay = false;

		// Spacecraft in spacecraft are destroyed if they end up in a fuel cell.
		if(CapsuleRoomType() == ROOM_Fuel)
			DamageAndDestroy(tr("Invalid location inside fuel cell."));
	}
	else if(IsInAtmosphere())
	{
		// Ships in atmosphere do not decay on a road slab if they have a human or AI officer on the roster.
		if(IsOnGround())
		{
			INDUSTRYID iid = InIndustry();
			if(iid == IID_AirportTerminal || iid == IID_BaseAirTerminal || iid == IID_AirportRepairShop)
			{
				if(HasOfficers())
					canDecay = false;
			}
		}
	}
	else
	{
		// Ships in space do not decay if they have capacitor power.
		if(CapacitorLevel())
			canDecay = false;
	}

	if(canDecay)
	{
		// Ship is allowed to decay.
		quint32 maxHits = DesignHitsTotal();
		STREAMLINE hull = HullStreamline();
		QString decayReason;

		// Environmental decay.
		WEAPONEFFECT decayEnv = WE_None;	// Decay due to environment contact.
		COMMODITYID decayEnvCID = CID_Invalid;
		int hitsEnv = 0;					// Hits environmental damage.
		if(IsInVisibleWater())
		{
			// Spacecraft is sitting in lava/water/liquid.
			if(IsInLava())
			{
				// Sitting in lava.
				int factor;
				if(hull >= SLINE_Submersible)
					factor = 20;
				else if(hull >= SLINE_SeaSurface)
					factor = 15;
				else if(hull >= SLINE_Atmospheric)
					factor = 10;
				else
					factor = 5;
				decayEnv = WE_Heat;
				hitsEnv = maxHits/factor + 1;	// 10% per second is pretty bad
				decayReason = tr("lava");
				decayEnvCID = CID_Magma;
			}
			else if(IsInSupercooledLiquid())
			{
				// Sitting in supercooled liquid.
				if(hull >= SLINE_Submersible)
				{
					// Hull is streamlined for full submersion in liquids.
					if(last.IsNewPeriod(now, d_MSecPerHr, Id().GetID()))
					{
						if(InAtmosphereType() == ATMTYPE_Insidious)
							decayEnv = WE_Acid;
						else if(InAtmosphereType() == ATMTYPE_Corrosive)
							decayEnv = WE_Corrosion;
						else
							decayEnv = WE_Cold;
						hitsEnv = maxHits/40 + 1;
						decayReason = tr("supercooled liquid");
						decayEnvCID = CID_SupercooledLiquid;
					}
				}
				else if(hull >= SLINE_SeaSurface)
				{
					// Hull is streamlined for surface contact with liquids.
					if(WaterDepth() > 3*FeetPerMeter())
					{
						// Hull is submerged.
						if(last.ElapsedMinTicks(now) > 0)
						{
							if(InAtmosphereType() == ATMTYPE_Insidious)
								decayEnv = WE_Acid;
							else if(InAtmosphereType() == ATMTYPE_Corrosive)
								decayEnv = WE_Corrosion;
							else
								decayEnv = WE_Cold;
							hitsEnv = maxHits/20 + 1;
							decayReason = tr("supercooled liquid");
							decayEnvCID = CID_SupercooledLiquid;
						}
					}
					else
					{
						// Hull is on the surface.
						if(last.IsNewPeriod(now, d_MSecPerHr, Id().GetID()))
						{
							if(InAtmosphereType() == ATMTYPE_Insidious)
								decayEnv = WE_Acid;
							else if(InAtmosphereType() == ATMTYPE_Corrosive)
								decayEnv = WE_Corrosion;
							else
								decayEnv = WE_Cold;
							hitsEnv = maxHits/40 + 1;
							decayReason = tr("supercooled liquid");
							decayEnvCID = CID_SupercooledLiquid;
						}
					}
				}
				else if(hull >= SLINE_Atmospheric)
				{
					// Hull is streamlined against contact with moisture.
					if(last.ElapsedMinTicks(now) > 0)
					{
						if(InAtmosphereType() == ATMTYPE_Insidious)
							decayEnv = WE_Acid;
						else if(InAtmosphereType() == ATMTYPE_Corrosive)
							decayEnv = WE_Corrosion;
						else
							decayEnv = WE_Cold;
						hitsEnv = maxHits/20 + 1;	// Not quite as bad as lava.
						decayReason = tr("supercooled liquid");
						decayEnvCID = CID_SupercooledLiquid;
					}
				}
				else
				{
					// Hull is not streamlined against moisture.
					if(InAtmosphereType() == ATMTYPE_Insidious)
						decayEnv = WE_Acid;
					else if(InAtmosphereType() == ATMTYPE_Corrosive)
						decayEnv = WE_Corrosion;
					else
						decayEnv = WE_Cold;
					hitsEnv = maxHits/20 + 1;	// Not quite as bad as lava.
					decayReason = tr("supercooled liquid");
					decayEnvCID = CID_SupercooledLiquid;
				}
			}
			else
			{
				// Sitting in water.
				if(hull >= SLINE_Submersible)
				{
					// Hull is streamlined for full submersion in liquids.
					if(last.IsNewPeriod(now, d_MSecPerHr, Id().GetID()))
					{
						decayEnv = WE_Corrosion;
						hitsEnv = maxHits/100 + 1;
						decayReason = tr("water");
						decayEnvCID = CID_Hydrographics;
					}
				}
				else if(hull >= SLINE_SeaSurface)
				{
					// Hull is streamlined for surface contact with liquids.
					if(WaterDepth() > 3*FeetPerMeter())
					{
						// Hull is submerged.
						if(last.ElapsedMinTicks(now) > 0)
						{
							decayEnv = WE_Corrosion;
							hitsEnv = maxHits/50 + 1;
							decayReason = tr("water");
							decayEnvCID = CID_Hydrographics;
						}
					}
					else
					{
						// Hull is on the surface.
						if(last.IsNewPeriod(now, d_MSecPerHr, Id().GetID()))
						{
							decayEnv = WE_Corrosion;
							hitsEnv = maxHits/100 + 1;
							decayReason = tr("water");
							decayEnvCID = CID_Hydrographics;
						}
					}
				}
				else if(hull >= SLINE_Atmospheric)
				{
					// Hull is streamlined against contact with moisture.
					if(last.ElapsedMinTicks(now) > 0)
					{
						decayEnv = WE_Corrosion;
						hitsEnv = maxHits/50 + 1;
						decayReason = tr("water");
						decayEnvCID = CID_Hydrographics;
					}
				}
				else
				{
					// Hull is not streamlined against moisture.
					decayEnv = WE_Corrosion;
					hitsEnv = maxHits/50 + 1;
					decayReason = tr("water");
					decayEnvCID = CID_Hydrographics;
				}
			}
		}

		// Atmospheric decay.
		WEAPONEFFECT decayAtm = WE_None;	// Decay due to atmosphere.
		COMMODITYID decayAtmCID = CID_Invalid;
		int hitsAtm = 0;					// Hits atmospheric damage.
		if(!IsCGSubmerged())
		{
			if(IsInVacuum())
			{
				// Decay every two hours in vacuum.
				if(last.IsNewPeriod(now, d_MSecPerHr*2, Id().GetID()))
				{
					int factor;
					if(hull >= SLINE_Submersible)
						factor = 200;
					else if(hull >= SLINE_SeaSurface)
						factor = 150;
					else if(hull >= SLINE_Atmospheric)
						factor = 100;
					else
						factor = 50;
					decayAtm = WE_Bludgeon;	// meteor impact?
					hitsAtm = maxHits/factor + 1;
					if(!decayReason.isEmpty())
						decayReason = tr("%1 and meteor").arg(decayReason);
					else
						decayReason = tr("meteor");
					decayAtmCID = CID_Ore;	// meteors are often high in metal content
				}
			}
			else
			{
				switch(InAtmosphereType())
				{
				case ATMTYPE_Count:
					break;
				case ATMTYPE_Standard:
				case ATMTYPE_Tainted:
				case ATMTYPE_Exotic:
					// Decay every hour in reasonable atmosphere.
					if(last.IsNewPeriod(now, d_MSecPerHr, Id().GetID()))
					{
						int factor;
						if(hull >= SLINE_Submersible)
							factor = 200;
						else if(hull >= SLINE_SeaSurface)
							factor = 150;
						else if(hull >= SLINE_Atmospheric)
							factor = 100;
						else
							factor = 50;
						decayAtm = WE_Corrosion;
						hitsAtm = (int)(maxHits/factor * AirDensity()) + 1;
						if(!decayReason.isEmpty())
							decayReason = tr("%1 and weather").arg(decayReason);
						else
							decayReason = tr("weather");
						if(OrbitZone() == OZONE_Inferno)
							decayAtmCID = CID_InfernoAtmosphere;
						else if(OrbitZone() == OZONE_Frigid)
							decayAtmCID = CID_FrigidAtmosphere;
						else
							decayAtmCID = CID_Atmosphere;
					}
					break;
				case ATMTYPE_Corrosive:
					// Decay every minute in corrosive atmosphere.
					if(last.ElapsedMinTicks(now) > 0)
					{
						int factor;
						if(hull >= SLINE_Submersible)
							factor = 100;
						else if(hull >= SLINE_SeaSurface)
							factor = 75;
						else if(hull >= SLINE_Atmospheric)
							factor = 50;
						else
							factor = 25;
						decayAtm = WE_Acid;
						hitsAtm = (int)(maxHits/factor * AirDensity()) + 1;
						if(!decayReason.isEmpty())
							decayReason = tr("%1 and corrosive atmosphere").arg(decayReason);
						else
							decayReason = tr("corrosive atmosphere");
						if(OrbitZone() == OZONE_Inferno)
							decayAtmCID = CID_InfernoAtmosphere;
						else if(OrbitZone() == OZONE_Frigid)
							decayAtmCID = CID_FrigidAtmosphere;
						else
							decayAtmCID = CID_Atmosphere;
					}
					break;
				case ATMTYPE_Insidious:
					// Decay every second in insidius atmosphere.
					{
						int factor;
						if(hull >= SLINE_Submersible)
							factor = 400;
						else if(hull >= SLINE_SeaSurface)
							factor = 300;
						else if(hull >= SLINE_Atmospheric)
							factor = 200;
						else
							factor = 100;
						decayAtm = WE_Acid;
						hitsAtm = (int)(maxHits/factor * AirDensity()) + 1;
						if(!decayReason.isEmpty())
							decayReason = tr("%1 and insidious atmosphere").arg(decayReason);
						else
							decayReason = tr("insidious atmosphere");
						if(OrbitZone() == OZONE_Inferno)
							decayAtmCID = CID_InfernoAtmosphere;
						else if(OrbitZone() == OZONE_Frigid)
							decayAtmCID = CID_FrigidAtmosphere;
						else
							decayAtmCID = CID_Atmosphere;
					}
					break;
				}
			}
		}

		// Apply damage due to decay, if any.
		if(decayAtm != WE_None)
		{
			// This applies both decayAtm and decayEnv, if they are both set.
			if(HasOfficers())
			{
				QString text;
				if(IsInAtmosphere())
					text = tr("%1 damage from %2. Airport terminals and repair shops provide shelter.")
						.arg(EffectName(decayAtm)).arg(decayReason);
				else
					text = tr("%1 damage from %2. Electromagnetic hull plating is ineffective without capacitor power.")
						.arg(EffectName(decayAtm)).arg(decayReason);
				SendEnvDamage(text);
			}

			AuCombatAttacker attacker(decayReason);

			AuCombatWeapon weapon(WM_Internal);
			weapon.m_CID = decayAtmCID;
			weapon.m_w1.m_Effect = decayAtm;
			weapon.m_w2.m_Effect = decayEnv;

			AuCombatResult result;
			result.m_Damage1 = hitsAtm;
			result.m_Damage2 = hitsEnv;

			CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
		}
		else if(decayEnv != WE_None)
		{
			// This applies only decayEnv.
			if(HasOfficers())
			{
				QString text = tr("%1 damage from %2.").arg(EffectName(decayEnv)).arg(decayReason);
				SendEnvDamage(text);
			}

			AuCombatAttacker attacker(decayReason);

			AuCombatWeapon weapon(WM_Internal);
			weapon.m_CID = decayEnvCID;
			weapon.m_w1.m_Effect = decayEnv;

			AuCombatResult result;
			result.m_Damage1 = hitsEnv;

			CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
		}
	}
	tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));

	// Remainder from Think
	if(secCycles == 0)
		return;

	// Insure the system is explored.
	if(!pmyss)
		pmyss = dynamic_cast<DaSolarSystem*>(FindEnvironmentAtom());
	if(pmyss)
		pmyss->ExploredBy(EmpireId());

	if(IsMissionActive())
	{
		AuMissionOrder::TYPE curOrder = MissionOrderType();
		// Execute the scuttle order, if the current order is Scuttle.
		if(curOrder == AuMissionOrder::Scuttle)
			DoScuttleOrder();
		// Configure harvester bays when that is the current mission order.
		else if(curOrder == AuMissionOrder::HarvestAtmosphere ||
				curOrder == AuMissionOrder::HarvestMinerals ||
				curOrder == AuMissionOrder::HarvestSea ||
				curOrder == AuMissionOrder::HarvestWell ||
				curOrder == AuMissionOrder::HarvestParticles)
			ConfigureHarvester();
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	// Repairs. Count down seconds until repairs are completed.
	if(RepairSecondsRemaining())
	{
		m_VesselLock.lockForWrite();
		m_RepairSecs--;
		m_VesselLock.unlock();
		SetModified();
	}

	// Fire weapons that are waiting to fire.
	m_BayLock.lockForRead();
	bool v = !m_BayFired.empty();
	m_BayLock.unlock();
	if(v)
		CycleBayWeapons();
	tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));

	// Tick games.
	if(IsInAnyScene())
	{
		if(last.ElapsedMinTicks(now) > 0)
			GamesTick(true);			// Once per minute we validate the players are still present.
		else if(last.SecIsEven())
			GamesTick(false);			// Every even second is often enough for the game to tick.
	}
	tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
}

void DaVessel::CheckTrespass(float sensorRange)
{
	const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(FindEnvironmentAtom());
	if(!system)
		return;	// No parent solar system?

	quint32 empireId = EmpireId();
	if(!system->IsNeutralEmpirePresent(empireId))
		return;	// No neutral empire unit or city present.

	// Search the system for a trespasser.
	const DaAtom* trespasser = system->FindTrespasserRecursive(empireId, EnvLocation(), sensorRange);
	if(!trespasser)
		return;

	GALAXY galaxy = Galaxy();
	const DaSector* sector = system->FindParentSector();
	const DaWorld* world = trespasser->FindParentPlanetInEnv();
	QString sectorName = sector ? sector->Name() : "";
	QString systemName = system ? system->Name() : "";
	QString worldName = world ? world->Name() : "";

	// A trespassing station or city was detected. Declare them to be enemy.
	if(dynamic_cast<const DaBd*>(trespasser))
	{
		AuDocStationTrespassCity doc(Name(), trespasser->Name(), galaxy, sectorName, systemName, worldName);
		emit GovDossierAddDocument(empireId, trespasser->EmpireId(), doc.Pack());
	}
	else
	{
		AuDocStationTrespassStation doc(Name(), trespasser->Name(), galaxy, sectorName, systemName, worldName);
		emit GovDossierAddDocument(empireId, trespasser->EmpireId(), doc.Pack());
	}
}

bool DaVessel::ScuttleExcessStation()
{
	// Don't scuttle privately owned stations that have a human officer because the station will just respawn anyway.
	Q_ASSERT(IsSpaceStation());
	if(IsPrivatelyOwned() && HasHumanOfficer())
		return false;
	if(IsPirate())
		return false;	// Don't do this for pirate empires.
	if(InArena())
		return false;
	if(IsInTow())
		return false;	// Don't scuttle stations that are in tow.
	if(InDesignEnvironment())
		return false;
	if(FindOfficer())	// Find any officer, even if they don't have the con.
		return false;	// Don't scuttle stations that have any officer aboard.

	// Check to see if this station must be removed.
	const DaWorld* planet = FindParentPlanetInEnv();
	if(planet)
	{
		// Station is at a planet.
		// Scuttle if there's no officer and no people aboard who are online and if there is any other station of my empire at
		// the same planet that does not have an officer.
		if(FindPersonOnLineRecursive())
			return false;

		// No officer and no people on line. This station will be scuttled if there is another station here with no officer.
		DaAtomList subs;
		planet->ListNormalSubatoms(subs);
		for(DaAtomList::const_iterator si = subs.cbegin(); si != subs.cend(); si++)
		{
			const DaVessel* ship = dynamic_cast<const DaVessel*>(*si);
			if(!ship)
				continue;	// Skip non-spaceship atoms.
			if(ship == this)
				continue;	// Skip this spaceship.

			// If we find any ship with an invalid cache, just quit.
			// We'll do this again later when everybody knows their own structure type.
			if(!ship->IsDesignDataCacheValid())
				break;

			if(ship->IsInTow())
				continue;	// Ignore ships that are in tow.
			if(ship->IsPrivatelyOwned())
				continue;
			if(ship->EmpireId() != EmpireId())
				continue;	// Skip other empire's ship.
			if(!ship->IsSpaceStation())
				continue;	// Skip other ship if it's not a station.
			if(ship->FindOfficer())
				continue;	// Skip other station if it has any officer aboard.
			// No officer aboard that other fleet station of my empire that is not being towed.
			// Scuttle this station.
			DestroyShip(tr("Station at %1 decomissioned.").arg(planet->Name()));
			return true;
		}
	}
	else
	{
		// System station is one that is not at a planet.
		// Scuttle system station if there's no officer and it is not in any scene.
		if(IsInAnyScene())
			return false;

		// Scuttle this station.
		DestroyShip(tr("System station decomissioned."));
		return true;
	}
	return false;
}

// Breaks this ship free from AtRest state when appropriate.
bool DaVessel::StartMoving()
{
	if(IsNew())
		return false;

	// Check to see if this ship is already in motion.
	if(!IsAtRest())
		return false;					// Already in motion.

	if(IsInSunSafeZone())
		return false;

	if(IsInTow() && !IsMoored())
	{
		ForceMotion();
		return true;
	}

	// Ship is currently at rest. Look for reasons to kick it into motion.
	if(!GravityDriveWorking())
	{
		// Maneuver drive is not working at all.
		if(IsOnGround() || IsOnWater() || IsMoored())
			return false;				// Mooring or contact with surfaces/water prevents me from wandering.

		// Not moored, touching ground or floating on water.
		ForceMotion();
		return true;
	}

	// Maneuver drive is "working" and functioning in either 'On' or 'Hold' state.
	if(GravityDriveState() == GD_Hold)
	{
		// Maneuver drive is on 'Hold' but that only works when in space, not against strong gravity, like when in an atmosphere
		// or aboard another ship.
		if(!IsInGravity())
			return false;				// Maneuver drive hold is effective here.

		// Ship is affected by gravity; it must be inside another ship or in planet atmosphere range.
		if(IsOnGround() || IsOnWater() || IsMoored())
			return false;				// Mooring or contact with surfaces/water prevents me from wandering.

		// Not moored, touching ground or floating on water.
		ForceMotion();
		return true;
	}

	// Maneuver drive is working and functioning in 'On' state.
	if(m_Throttle || m_ThrustLateral || m_ThrustLong || m_ThrustVertical)
	{
		// Throttles are active so start moving.
		ForceMotion();
		return true;
	}

	if(RollPitchYawTime().elapsed() < 1000)
	{
		// Pilot is maneuvering the ship.
		ForceMotion();
		return true;
	}

	// Drive module specific checks.
	if(HasRocketDrive())
	{
		// Rocket drive does not hold my position in space when not explicitly on 'Hold'.
		if(IsOnGround() || IsOnWater() || IsMoored())
			return false;				// Mooring or contact with surfaces/water prevents me from wandering.

		// Under these conditions, an active AI pilot can easily hold my position.
		if(IsPilotMannedByAI())
			return false;			// AI pilot prevents me from wandering.

		// Not moored, touching ground or floating on water.
		ForceMotion();
		return true;
	}
	else
	{
		// Gravity drive is working and functioning in 'On' state.
		if(InBlackHoleGravity())
		{
			// Gravity drive cannot hold position against black hole gravity.
			if(IsOnGround() || IsOnWater() || IsMoored())
				return false;			// Mooring or contact with surfaces/water prevents me from wandering.

			// Under these conditions, an active AI pilot can easily hold my position.
			if(IsPilotMannedByAI())
				return false;			// AI pilot prevents me from wandering.

			// Not moored, touching ground or floating on water.
			ForceMotion();
			return true;
		}

		// In normal gravity conditions, position in space is held easily by a functioning gravity drive.
		return false;
	}
}

void DaVessel::RepairAndResupplyStation()
{
	if(InArena())
		return;
	if(IsInAtmosphere())
		return;		// Stations that have crashed to the ground do not get supplies or repairs.
	if(IsPrivatelyOwned())
		return;		// Privately owned stations do not get free supplies.
	if(IsPirate())
		return;		// Pirate empires do not have cities from which to get supplies.
	if(InDesignEnvironment())
		return;

	DaSolarSystem* system = FindParentSolarSystemInEnv();
	if(!system || system->IsEmptySystem())
		return;

	// Allow repairs at quality of lowest system.
	DaAtomList supplyCities;
	DaAtomList repairCities;
	DaAtomList troopCities;
	system->FindCitiesBdRecursive(supplyCities, repairCities, troopCities, EmpireId());	// Finds DaBd atoms.

	if(!repairCities.isEmpty())
	{
		// Select a bd or city at random from which to get repairs.
		DaAtomList::const_iterator rci = repairCities.begin() + g_Au.Dice().Roll(1,repairCities.count(),-1);
		DaBd* repairBd = dynamic_cast<DaBd*>(*rci);
		if(repairBd)
			RepairAllPossible(repairBd, QualityMade());			// Perform all repairs possible using community inventory.
	}

	// Import crew and supplies from a bd or city.
	if(!supplyCities.isEmpty())
	{
		// Select a bd or city at random from which to import supplies.
		DaAtomList::const_iterator sci = supplyCities.begin() + g_Au.Dice().Roll(1,supplyCities.count(),-1);
		DaBd* supplyBd = dynamic_cast<DaBd*>(*sci);
		if(supplyBd)
			supplyBd->ExportToStation(this);
	}

	// Deploy troops from a bd or city.
	if(!troopCities.isEmpty())
	{
		// Select a bd or city at random from which to import supplies.
		DaAtomList::const_iterator tci = troopCities.begin() + g_Au.Dice().Roll(1,troopCities.count(),-1);
		DaBd* troopBd = dynamic_cast<DaBd*>(*tci);
		if(troopBd)
			troopBd->DeployTroopsToStation(this);
	}
}

void DaVessel::DoScuttleOrder()
{
	// Self destruct when the prescribed time has passed, in seconds.
	bool isExpired;
	m_MissionLock.lockForRead();
	//qDebug() << "DaVessel::DoScuttleOrder" << m_Mission.OrderStart().ElapsedSec() << ":" << m_Mission.CurrentOrder().Info();
	if(m_Mission.CurrentOrder().Type() == AuMissionOrder::Scuttle)
		isExpired = m_Mission.CurrentOrder().IsExpired(m_Mission.OrderStart());
	else
		isExpired = false;
	m_MissionLock.unlock();
	if(isExpired)
	{
		MissionOrderComplete(FindCommandingOfficer(), 3);
		SelfDestruct();				// Initiate ship self destruct sequence.
	}
}

void DaVessel::ApplyRotation(unsigned int msElapsed, CONTROL_CONTEXT cc)
{
	// Ship does not rotate while moored.
	if(IsMoored())
		return;

	if(GravityDriveState() != GD_On || !GravityDriveWorking())
		return;

	float turnRate = TurnRate();		// Must acquire turn rate before locking control lock.
	AuMatrix4x4 tRotate;
	m_ControlLock.lockForWrite();
	if(IsTurning())
	{
		// When animating local rotation, limit the time span so occasional network lags won't cause huge turns.
		//if(cc != CONTROL_Server)
		{
			if(msElapsed > 150)
				msElapsed = 150;
		}
		//qDebug("DaVessel::ApplyRotation %d msElapsed %d Capacitor "
				//"%g turn rate %g rot speeds %d,%d,%d",
				//Id().GetID(),
				//msElapsed, CapacitorLevel(),
				//turnRate*RAD2DEG,
				//m_byRotSpeedX, m_byRotSpeedY, m_byRotSpeedZ);

		// Calculate turn amounts based on turn rate and time span.
		float rsec = (float)msElapsed/1000;	// Real world secs that passed.
		float rMaxTurn = turnRate * rsec;	// Max turn angle in this time period.
		float rTurnX = m_byRotSpeedX * rsec / 100;
		if(rTurnX > rMaxTurn)
			rTurnX = rMaxTurn;
		float rTurnY = m_byRotSpeedY * rsec / 100;
		if(rTurnY > rMaxTurn)
			rTurnY = rMaxTurn;
		float rTurnZ = m_byRotSpeedZ * rsec / 100;
		if(rTurnZ > rMaxTurn)
			rTurnZ = rMaxTurn;

		// Accumulate the rotation matrix to apply to the ship.
		// Then we can change the relative location of the sensor blips using the same matrix.
		if(m_wActions & ACT_TurnLeft)
			tRotate.RotateZ(rTurnZ);
		else if(m_wActions & ACT_TurnRight)
			tRotate.RotateZ(-rTurnZ);
		if(m_wActions & ACT_PitchUp)
			tRotate.RotateX(rTurnX);
		else if(m_wActions & ACT_PitchDown)
			tRotate.RotateX(-rTurnX);
		if(m_wActions & ACT_RollLeft)
			tRotate.RotateY(-rTurnY);
		else if(m_wActions & ACT_RollRight)
			tRotate.RotateY(rTurnY);
		m_wActions &= ~ACT_Run;	// Indicate not turning by AI, for the IsTicker function.
		m_Pitch = 0;
		m_Roll = 0;
		m_Yaw = 0;
	}
	else if(m_Pitch != 0 || m_Roll != 0 || m_Yaw != 0)
	{
		// AI pilot is turning this ship, has specified precise turn-to angles.
		//qDebug() << "DaVessel::ApplyRotation" << Id() << Name() << "msElapsed" << msElapsed;
		// Calculate turn amounts based on turn rate and time span.
		if(msElapsed > 150)
			msElapsed = 150;
		float rsec = (float)msElapsed/1000;	// Real world secs that passed.
		float rMaxTurn = turnRate * rsec;	// Max turn angle in this time period.

		// Limit desired turn amounts to the max that I can turn in the time period.
		float roll;
		if(m_Roll > rMaxTurn)
		{
			roll = rMaxTurn;
			m_Roll -= rMaxTurn;
		}
		else if(m_Roll < -rMaxTurn)
		{
			roll = -rMaxTurn;
			m_Roll += rMaxTurn;
		}
		else
		{
			roll = m_Roll;
			m_Roll = 0;
		}

		float pitch;
		if(m_Pitch > rMaxTurn)
		{
			pitch = rMaxTurn;
			m_Pitch -= rMaxTurn;
		}
		else if(m_Pitch < -rMaxTurn)
		{
			pitch = -rMaxTurn;
			m_Pitch += rMaxTurn;
		}
		else
		{
			pitch = m_Pitch;
			m_Pitch = 0;
		}

		float yaw;
		if(m_Yaw > rMaxTurn)
		{
			yaw = rMaxTurn;
			m_Yaw -= rMaxTurn;
		}
		else if(m_Yaw < -rMaxTurn)
		{
			yaw = -rMaxTurn;
			m_Yaw += rMaxTurn;
		}
		else
		{
			yaw = m_Yaw;
			m_Yaw = 0;
		}

		if(m_Roll == 0 && m_Pitch == 0 && m_Yaw == 0)
			m_wActions &= ~ACT_Run;	// Indicate not turning by AI, for the IsTicker function.

		// Accumulate the rotation matrix to apply to the ship.
		// Then we can change the relative location of the sensor blips using the same matrix.
		if(roll)
			tRotate.RotateY(roll);
		if(pitch)
			tRotate.RotateX(pitch);
		if(yaw)
			tRotate.RotateZ(yaw);
	}
	else
		m_wActions &= ~ACT_Run;		// Insure this bit is off when not needed.
	m_ControlLock.unlock();

	// Apply the rotation to the ship.
	if(!tRotate.IsIdentity())
	{
		AuAtomPos pos(Pos());
		AuPoint oldVec;
		bool rotvec = false;
		if(!GravityDriveWorking() || HasRocketDrive())
		{
			oldVec = pos.Transform().ProjectVector(Vector());
			rotvec = true;
		}
		AuPoint otn = pos.Transform().ProjectVector(TerrainNormal());
		AuPoint ogn = pos.Transform().ProjectVector(GravityNormal());

		// Rotate the ship.
		pos.TransformBy(tRotate);
		ChangePosAndOrient(pos);

		// This atom's movement vector must be held in the direction it was going, sort of like real inertia.
		if(rotvec)
		{
			AuPoint nuVec = Transform().UnProjectVector(oldVec);
			SetVector(nuVec);
		}
		AuPoint ngn = Transform().UnProjectVector(ogn);
		SetGravityNormal(ngn);
		AuPoint ntn = Transform().UnProjectVector(otn);
		SetTerrainNormal(ntn);
		if(cc != CONTROL_Client)
			SetModified(NOTIFY_Scene);
	}

	if(Orient(Location(), Transform().ProjectVector(TerrainNormal())))
	{
		if(cc != CONTROL_Client)
			SetModified(NOTIFY_Scene);
	}
}

void DaVessel::TickMovement(const AuElapsedTimer& last)
{
	// Kick loose from 'at rest' state when appropriate.
	StartMoving();

	DaCapsule::TickMovement(last);

	// On the client, TickMovement is only called on the scene atom and the atom controlled by the scene atom, which could only be
	// a vehicle, spacecraft or animal.
	qint64 msElapsed = last.elapsed();
	CONTROL_CONTEXT cc;
	if(g_Au.IsSceneClient())
		cc = CONTROL_Client;
	else if(g_Au.IsSceneSolo())
		cc = CONTROL_Solo;
	else
		cc = CONTROL_Server;
	ApplyRotation(msElapsed, cc);

	DecayMoveActions();
}

bool DaVessel::IsTicker() const
{
	if(DaCapsule::IsTicker())
		return true;
	if(IsTurning() || (m_wActions & ACT_Run))	// ACT_Run vessel indicates turning by AI.
		return true;
	if(IsTraveling() || IsNew() || InDesignEnvironment())
		return false;
	if(IsInTow())
		return true;
	return false;
}

void DaVessel::Tick(const AuElapsedTimer& last, const AuElapsedTimer& now)
{
	// Ticking this ship and its subatoms depends on this ship's design being analyzed. Tick will be delayed until the database
	// thread has loaded the design from the database.
	if(!IsDesignDataCacheValid())
		return;

	// Tick base class and all subatoms.
	//SetMassAboard(0);											// Subordinate spacecraft will add to this using AddMassAboard.
	DaCapsule::Tick(last, now);
	if(IsDead())
		return;

	// Don't do anything else if this ship entered a wormhole.
	if(IsTraveling())
		return;

	// Remaining actions are not performed on "nu" spacecraft.
	if(IsNew())
		return;

	// Remaining actions are not performed in the design environment.
	if(InDesignEnvironment())
		return;

	/*DHH Unpredictable tick sequence broke this.
	if(IsInSpacecraft())
	{
		// Add my mass to my parent spacecraft to slow down those motherships.
		DaVessel* parent = dynamic_cast<DaVessel*>(Parent());
		if(parent)
			parent->AddMassAboard(Mass());
	}
	*/

	if(IsInTow())
	{
		bool foundTug = false;
		if(!ShieldsBlockWeapons())
		{
			// Shields are not blocking the tractor beam.
			// Move toward or from the ships that are towing me.
			DaAtom* envAtom = FindEnvironmentAtom();
			if(envAtom)
			{
				DaAtomList ships;
				envAtom->FindVesselsRecursive(ships);
				for(DaAtomList::const_iterator si = ships.cbegin(); si != ships.cend(); si++)
				{
					const DaVessel* ship = dynamic_cast<const DaVessel*>(*si);
					if(!ship)
						continue;
					if(!ship->IsNormal() && !ship->IsTraveling())
						continue;
					if(ship->Is(Id()))
						continue;
					double tractorRange;
					double tractorAccel;
					if(!ship->IsTowing(Id(), Mass(), tractorRange, tractorAccel))
						continue;

					// Found a ship that is towing me.
					foundTug = true;
					if(!ship->IsNormal())
						continue;
					if(ship->IsInSunSafeZone())
						continue;

					// Am I resisting?
					if((m_Throttle || m_ThrustLateral || m_ThrustLong || m_ThrustVertical || (m_wActions & ACT_BrakeMask)) &&
							GravityDriveWorking() &&
							ManeuverAcceleration() >= tractorAccel)
					{
						// I can overpower the beam. Don't let it drag me.
						continue;
					}

					AuPoint vtow = EnvTransform().UnProject(ship->EnvLocation());	// Vector from me to towing vessel, in my space.
					double dist = vtow.Length();
					if(dist == 0)
						continue;	// Not likely.

					// Calculate towing range.
					double towRange = RoughRadius() + ship->RoughRadius() + 500*FeetPerMeter();
					if(towRange > tractorRange - 100*FeetPerMeter())
						towRange = tractorRange - 100*FeetPerMeter();

					if(dist < towRange - 50*FeetPerMeter())
					{
						// Distance to towing vessel is too close; move farther away.
						//qDebug() << "DaVessel::Tick" << Id() << Name()
							//<< "moving away from tug" << ship->Id() << ship->Name()
							//<< "accel" << tractorAccel / FeetPerMeter() << "mps"
							//<< "range" << dist / FeetPerMeter() << "m towRange" << towRange / FeetPerMeter() << "m";
						vtow.Unit((towRange - 50*FeetPerMeter()) - dist);
						Move(-vtow, last.elapsed());
					}
					else if(dist > towRange + 50*FeetPerMeter())
					{
						// Distance to towing vessel is too far away; move closer.
						//qDebug() << "DaVessel::Tick" << Id() << Name()
							//<< "moving toward tug" << ship->Id() << ship->Name()
							//<< "accel" << tractorAccel / FeetPerMeter() << "mps"
							//<< "range" << dist / FeetPerMeter() << "m towRange" << towRange / FeetPerMeter() << "m";
						vtow.Unit(dist - (towRange + 50*FeetPerMeter()));
						Move(vtow, last.elapsed());
					}
				}
			}
		}

		// Clear my tow state if I did not find the tug.
		if(!foundTug)
			SetInTow(false);
	}
}

// Complete the firing sequence of weapon bays.
void DaVessel::CycleBayWeapons()
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	DaAtom* system = FindEnvironmentAtom();
	if(!system)
		return;							// No solar system? Not likely.

	AuBayFired bayFired;
	quint16 which;
	quint16 lastWhich = 0xffff;			// Weapon bay which is the room number. Highest valid room number is 0x00ff.
	while(GetNextWeaponBayFired(bayFired, which) && which != lastWhich)
	{
		lastWhich = which;

		// Locate the target atom.
		DaAtom* target = system->FindIdRecursive(bayFired.m_Target);
		if(!target)
		{
			if(bayFired.IsTractor())
				CancelWeaponBayFired(which);	// Tractor beam breaks when target cannot be found.
			continue;	// Could not find the target atom in this system. It may have escaped before the weapon fired.
		}
		if(Is(target->ParentId()))
		{
			if(bayFired.IsTractor())
				CancelWeaponBayFired(which);	// Tractor beam breaks when target is aboard the firing ship.
			continue;	// Cannot target on atoms aboard the firing ship.
		}

		// Target may be protected from attack.
		if(target->IsNoncombatant())
		{
			// Target is protected from attack.
			if(bayFired.IsTractor())
			{
				// Tractor beam is allowed to grab ships of friendly empires.
				POLITICAL_STANCE targStanceToMe = PoliticalStance(target->EmpireId(), EmpireId());
				if(targStanceToMe == POL_Neutral || targStanceToMe == POL_Enemy)
				{
					// Neutral and enemy ships cannot be grabbed with tractor.
					CancelWeaponBayFired(which);
					continue;
				}
			}
			else
				continue;	 // Skip the target with all other kinds of bay weapons.
		}

		// Determine what kind of module has fired.
		COMMODITYID bayModule = SystemModule(SYS_Weapon, which);

		// Create visual effect of the weapon firing if it has any functioning weapon units.
		// Get reverse transformation back up to the parent solar system
		AuMatrix4x4 tModel = EnvTransform().Inverse();

		// Ask the solar system to find the nearest solid atom that is in front of the weapon.
		// Harvesters do not hit anything they are not supposed to.
		// Harvesters always hit at the target location, which is the calculated harvest point within the atmosphere or whatever.
		AuPoint sightedLoc = bayFired.m_TargetLoc;			// Sighted location relative to this ship.
		bool sightedShield = false;
		if(bayModule == CID_TractorBeamBayModule)
		{
			//qDebug() << "DaVessel::CycleBayWeapons" << Id() << Name() << "tractor target" << target->Id() << target->Name();
			sightedLoc = target->WeaponLockLocation(EnvLocation(), bayModule);
			sightedLoc = EnvTransform().UnProject(sightedLoc);
		}
		else if(DbCommodity::IsHarvesterModule(bayModule))
		{
			sightedLoc = target->WeaponLockLocation(EnvLocation(), bayModule);
			sightedLoc = EnvTransform().UnProject(sightedLoc);
		}
		else
		{
			unsigned int sightedPart = 0;
			DaAtom* sightedAtom = system->SightTarget(this, bayFired.m_EmitLoc, tModel, 0, sightedLoc, sightedPart, sightedShield,
					0);
			if(sightedAtom && !Is(sightedAtom->Id()))
				target = sightedAtom;				// A target was hit along the ray that is not this ship.
			else
				sightedLoc = bayFired.m_TargetLoc;	// Restore to targetted loc.
		}

		// Convert sightedLoc to be a vector from the weapon emit loc to the target hit loc.
		AuPoint wpnSightLoc = sightedLoc - bayFired.m_EmitLoc;
		double targDist = wpnSightLoc.Length();					// On rings this is the closest point on the ring, for harvesters.

		// Determine appropriate special effects based on weapon installed in weapon system.
		AuCombatWeapon weapon(WM_Projectile);
		SOUND_EFFECT sound = SOUND_None;
		VISUAL_EFFECT visual = VISUAL_None;
		WEAPONEFFECT energyBeamEffect = WE_None;
		switch(bayModule)
		{
		case CID_BallisticWeaponBayModule:
			if(!bayFired.m_Ammo)
				continue;
			weapon = AuCombatWeapon(bayFired.m_Ammo, bayFired.m_UnitsFired);
			sound = (SOUND_EFFECT)AuOrdnance::SoundFired(bayFired.m_Ammo->CommodityId());
			visual = AuOrdnance::VisualFired(bayFired.m_Ammo->CommodityId());
			break;

		case CID_EnergyWeaponBayModule:
			if(!bayFired.m_Ammo)
				continue;
			weapon = AuCombatWeapon(bayFired.m_Ammo, bayFired.m_UnitsFired);
			sound = (SOUND_EFFECT)AuOrdnance::SoundFired(bayFired.m_Ammo->CommodityId());
			visual = AuOrdnance::VisualFired(bayFired.m_Ammo->CommodityId());
			energyBeamEffect = weapon.m_w1.m_Effect;
			weapon.m_Range = BayWeaponRange(which);
			break;

		case CID_WeaponDeploymentBayModule:
		case CID_WeaponLauncherBayModule:
			if(!bayFired.m_Ammo)
				continue;
			weapon = AuCombatWeapon(bayFired.m_Ammo, bayFired.m_UnitsFired);
			sound = (SOUND_EFFECT)AuOrdnance::SoundFired(bayFired.m_Ammo->CommodityId());
			visual = AuOrdnance::VisualFired(bayFired.m_Ammo->CommodityId());
			break;

		case CID_TractorBeamBayModule:
			// Visual/sound effect of tractor beam is handled elsewhere, as a continuous thing, not as a gun-fire effect.
			// We can use this opportunity to emit a noise when the beam fails.
			if(CapacitorLevel() > 2)
			{
				// My ship has some capacitor power.
				DaVessel* targetShip = dynamic_cast<DaVessel*>(target);
				if(targetShip && !targetShip->IsMoored() && !targetShip->ShieldsBlockWeapons())
				{
					// Target is a spacecraft that is not moored and shields are down.
					weapon.m_Range = TractorBeamRange(bayFired.m_UnitsFired, which);
					if(weapon.m_Range < targDist && DecayTractorBay(which))
					{
						// Tractor beam fails when target is too far away. DecayTractor is used to allow for Tick jumpiness.
						// Can't cancel tow state on target because others could be towing it also. It manages that.
						sound = SOUND_ServoWeak;	// Tractor range exceeded.
					}
					else
						targetShip->SetInTow(true);	// Refresh tow state.
				}
				else
					sound = SOUND_ServoStopped;		// Invalid tractor target or it is moored.
			}
			else
				sound = SOUND_ServoWeak;			// Capacitor is too low to maintain a tractor beam.
			break;

		case CID_AtmosphereCondenserBayModule:
		case CID_ParticleCollectorBayModule:
		case CID_ProspectorBayModule:
		case CID_SiphonBayModule:
		case CID_WildcatterBayModule:
			{
				sound = SOUND_Harvest;
				visual = VISUAL_FireEnergyBeam;
				energyBeamEffect = WE_Sonic;
				weapon.m_Range = HarvesterBayRange(bayFired.m_UnitsFired, which);
			}
			break;

		default:
			break;
		}

		// Add weapon fire effect to weapon.
		if(sound != SOUND_None || visual != VISUAL_None)
		{
			//qDebug() << "DaVessel::CycleBayWeapons" << Id() << Name() << "visual" << visual << "emitLoc" << bayFired.m_EmitLoc;
			AuPoint effRot;
			if(visual == VISUAL_FireCannon)
				effRot = AuPoint(0,1,0).GetAlignmentTransform(wpnSightLoc).GetRotation();
			float gain = .9f + sqrtf(bayFired.m_UnitsFired)/10.f;	// 1 unit = 1 gain, 100u = 1.9g, 1000u = 4g, 10000u = 10.9g
			AuEffect fireeffect(bayFired.m_EmitLoc, effRot, AuPoint(0,0), sound, gain, 1, visual);
			fireeffect.SetTargetLoc(sightedLoc);
			fireeffect.SetEnergyBeamEffect(energyBeamEffect);
			AddEffect(fireeffect);
		}

		// Calculate amount of energy to reach the target.
		if(bayModule == CID_TractorBeamBayModule)
		{
		}
		else if(bayModule == CID_AtmosphereCondenserBayModule)
		{
			// Atmosphere Harvester
			if(weapon.m_Range > 0)
			{
				const DaWorld* targetPlanet = dynamic_cast<const DaWorld*>(target);
				if(targetPlanet)
				{
					// Determine probability of harvesting the commodity.
					COMMODITYID cid = HarvestAtmosphereCommodity(bayFired.m_HarvestIndex);
					AuZoneResource res;
					targetPlanet->QueryNaturalResources(res, 0);
					Harvest(which, bayModule, weapon.m_Range, targDist, res.FetchProbability(cid), cid, res.Quality(cid));
				}
			}
		}
		else if(bayModule == CID_ProspectorBayModule)
		{
			// Mineral Harvester
			if(weapon.m_Range > 0)
			{
				// Determine probability of harvesting the commodity, which comes from the zone resource info.
				bool doharvest = false;
				AuZoneResource res;

				const DaWorldRing* targetRing = dynamic_cast<const DaWorldRing*>(target);
				if(targetRing)
				{
					// Determine probability of harvesting the commodity.
					targetRing->QueryNaturalResources(res, 0);					// Rings have only one resource zone.
					doharvest = true;
				}
				else
				{
					const DaWorldLL* targetWorld = dynamic_cast<const DaWorldLL*>(target);
					if(targetWorld)
					{
						AuPoint loc;
						int resourceZone = 0;
						if(GetLocationRelativeToParent(loc, targetWorld))
						{
							// I am subordinate to the world.
							resourceZone = targetWorld->ResourceZone(loc);
							doharvest = true;
						}
						targetWorld->QueryNaturalResources(res, resourceZone);
					}
				}

				if(doharvest)
				{
					// Attempt to harvest the commodity.
					COMMODITYID cid = HarvestProspectorCommodity(bayFired.m_HarvestIndex);
					Harvest(which, bayModule, weapon.m_Range, targDist, res.FetchProbability(cid), cid, res.Quality(cid));
				}
			}
		}
		else if(bayModule == CID_SiphonBayModule)
		{
			// Sea Harvester
			if(weapon.m_Range > 0)
			{
				// Determine probability of harvesting the commodity, which comes from the zone resource info.
				bool doharvest = false;
				AuZoneResource res;

				const DaWorldLL* targetWorld = dynamic_cast<const DaWorldLL*>(target);
				if(targetWorld)
				{
					AuPoint loc;
					int resourceZone = 0;
					if(GetLocationRelativeToParent(loc, targetWorld))
					{
						// I am subordinate to the world.
						resourceZone = targetWorld->ResourceZone(loc);
						doharvest = true;
					}
					targetWorld->QueryNaturalResources(res, resourceZone);
				}

				if(doharvest)
				{
					// Attempt to harvest the commodity.
					COMMODITYID cid = HarvestSiphonCommodity(bayFired.m_HarvestIndex);
					Harvest(which, bayModule, weapon.m_Range, targDist, res.FetchProbability(cid), cid, res.Quality(cid));
				}
			}
		}
		else if(bayModule == CID_WildcatterBayModule)
		{
			// Mineral Harvester
			if(weapon.m_Range > 0)
			{
				// Determine probability of harvesting the commodity, which comes from the zone resource info.
				bool doharvest = false;
				AuZoneResource res;

				const DaWorldLL* targetWorld = dynamic_cast<const DaWorldLL*>(target);
				if(targetWorld)
				{
					AuPoint loc;
					int resourceZone = 0;
					if(GetLocationRelativeToParent(loc, targetWorld))
					{
						// I am subordinate to the world.
						resourceZone = targetWorld->ResourceZone(loc);
						doharvest = true;
					}
					targetWorld->QueryNaturalResources(res, resourceZone);
				}

				if(doharvest)
				{
					// Attempt to harvest the commodity.
					COMMODITYID cid = HarvestWildcatterCommodity(bayFired.m_HarvestIndex);
					Harvest(which, bayModule, weapon.m_Range, targDist, res.FetchProbability(cid), cid, res.Quality(cid));
				}
			}
		}
		else if(bayModule == CID_ParticleCollectorBayModule)
		{
			// Particle Harvester
			if(weapon.m_Range > 0)
			{
				const DaWorld* targetPlanet = dynamic_cast<const DaWorld*>(target);
				if(targetPlanet)
				{
					// Determine probability of harvesting the commodity.
					COMMODITYID cid = HarvestParticleCommodity(bayFired.m_HarvestIndex);
					AuZoneResource res;
					targetPlanet->QueryNaturalResources(res, 0);
					Harvest(which, bayModule, weapon.m_Range, targDist, res.FetchProbability(cid), cid, res.Quality(cid));
				}
				else
				{
					const DaStar* targetStar = dynamic_cast<const DaStar*>(target);
					if(targetStar)
					{
						// Determine probability of harvesting the commodity.
						COMMODITYID cid = HarvestParticleCommodity(bayFired.m_HarvestIndex);
						cid = targetStar->CorrespondingHarvestResource(cid);
						Harvest(which, bayModule, weapon.m_Range, targDist, targetStar->FetchProbability(cid), cid,
								targetStar->ResourceQuality(cid));
					}
				}
			}
		}
		else if(bayFired.m_Ammo)
		{
			// Weapon bay module
			if(DbCommodity::IsEnergyWeapon(bayFired.m_Ammo->CommodityId()))
			{
				//int hit = g_Au.Dice().Roll(1,3);	// Randomly miss so we're not perfectly accurate.
				//if(hit == 1)
				{
					// Determine weapon effects based on module installed.
					// Apply net damage to target atom.
					bool creditCrew = Is(bayFired.m_Credit);				// Last fired id is ship id when crew gets the credit.
					DaAtom* byWho;
					if(!creditCrew)
						byWho = system->FindIdRecursive(bayFired.m_Credit);	// Can be null.
					else
						byWho = 0;

					AuCombatDefender defender(target, sightedLoc, sightedShield, targDist);

					AuCombatResult result;

					if(creditCrew || !byWho)
					{
						// Credit crew when a crew member fired the bay weapon.
						// Credit the crew when the gunner is no longer in the system.
						AuCombatAttacker attacker(Id(), NameWithEmpire(), EmpireId(), IsPirate());
						attacker.m_MuzzleLoc = bayFired.m_EmitLoc;

						AttackTarget(attacker, defender, weapon, true, result);

						// Send notification on the crew channel when a unit is hit.
						if(dynamic_cast<DaUnit*>(target) && !IsPirateWithFreeWill())
						{
							QString targetName;
							if(target->EmpireId())
							{
								QString empire;
								const DgData& gd = g_Au.LockDataForRead();
								gd.EmpireName(target->EmpireId(), empire);
								g_Au.UnlockData();
								targetName = QString("%1 %2").arg(empire).arg(target->Name());
							}
							else
								targetName = target->Name();
							AuCommDamageInflicted msg(targetName, result.m_CriticalHits, result.m_IsDestroyed,
									weapon.m_w1.m_Effect, result.m_Damage1, result.Armor1(),
									weapon.m_w2.m_Effect, result.m_Damage2, result.Armor2(),
									AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
									Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::CrewMates);
							msg.SetScopeData(Id().GetID());				// Required for CrewMates message to be valid.
							g_Au.SendAtomChat(msg);
						}
					}
					else
					{
						AuCombatAttacker attacker(byWho->Id(), tr("%1 aboard %2").arg(byWho->Name()).arg(NameWithEmpire()),
								byWho->EmpireId(), byWho->IsPirate());
						attacker.m_MuzzleLoc = bayFired.m_EmitLoc;

						AttackTarget(attacker, defender, weapon, true, result);
					}

					if(result.m_IsDestroyed && (IsSpaceStation() || HasFirstOfficer()))
					{
						AuMissionOrder::TYPE t = MissionOrderType();
						if((t == AuMissionOrder::Attack || t == AuMissionOrder::Plunder) &&
								target->Is(MissionOrderTarget()) &&
								IsCommandedByAI())
							MissionOrderComplete(FindCommandingOfficer(), 1);	// We destroyed the target of our attack order.

						if(!IsPirateWithFreeWill() && !InArena() && !InDesignPreview())
						{
							// Send a message on the friend channel.
							// Do this when someone is the captain, which could only be true if they were an agent. ;)
							QString text = tr("%1 destroyed.").arg(target->Name());
							AuCommChat msg(text, AuCommStdChannel::FriendChannelName(), Id(), Name(), FindParentSector()->Id(),
									Galaxy(), AuCommStdChannel::Sector, AuCommStdChannel::Friends);
							msg.SetScopeData(EmpireId());	// Friend channel needs my empire id to know the scope of the message.
							g_Au.SendAtomChat(msg);
						}
					}
				}
			}
			else
			{
				// Launch a projectile at the target.
				DaMissile* bullet;
				if(DbCommodity::IsMissile(bayFired.m_Ammo->CommodityId()))
					bullet = new DaMissile(*bayFired.m_Ammo, bayFired.m_UnitsFired, EmpireId(), bayFired.m_Target,
							bayFired.m_LocOnTarget);
				else
					bullet = new DaMissile(*bayFired.m_Ammo, bayFired.m_UnitsFired, EmpireId());
				bullet->ConnectSignals();
				bullet->Create();

				double muzzleVel = bullet->MuzzleExitVelocity();

				// Create a transform to position the projectile at the bay emitter, pointing at the target.
				AuMatrix4x4 aim;
				if(DbCommodity::IsDeployableWeapon(bayFired.m_Ammo->CommodityId()))
					aim = AuPoint(1,0,0).GetAlignmentTransform(AuPoint(0,1,0));
				else
				{
					// Calculate a firing solution that considers our movement vectors.
					AuPoint targSysLoc = target->EnvTransform().Project(target->TargetLocation());
					AuPoint emitDir = EnvTransform().UnProject(targSysLoc) - bayFired.m_EmitLoc;	// Relative to me.
					if(muzzleVel > 0)
					{
						double targdist = emitDir.Length();
						double targtime = targdist / muzzleVel;	// Time to stationary target at initial muzzle velocity, in seconds.

						// Compensate for my movement vector during that time.
						if(!IsAtRest())
							emitDir -= Vector() * targtime;

						// Compensate for target's movement vector during that time.
						if(!target->IsAtRest())
						{
							AuPoint targSysVec = target->EnvTransform().Project(target->Vector());
							AuPoint targVec = EnvTransform().UnProjectVector(targSysVec);
							emitDir += targVec * targtime;
						}
					}

					emitDir.Unit();
					aim = AuPoint(0,0,1).GetAlignmentTransform(emitDir);
				}
				AuMatrix4x4 t(Transform());
				t.Translate(bayFired.m_EmitLoc);
				t *= aim;
				bullet->SetPosAndOrient(AuAtomPos(ParentId(), t));
				ether->AddAtom(bullet);

				bullet->SetInScene();							// Don't decay too fast right away.
				bullet->SetVector(aim.UnProject(Vector()) + AuPoint(0,0,muzzleVel));	// Muzzle velocity plus my vec.
				bullet->SetInMotion();
				bullet->ArmWeapon(Id(), Name(), EmpireId());
				bullet->SetOwnerIgnored(true);	// Do not hit my owner/armedBy atom.

				g_Au.AtomAdded(bullet->Id());
				bullet->SetNormal();	// New atom is ready for normal thread processing.
				bullet->TickTock();		// Immediately start tick processing.

				if(!IsPirateWithFreeWill())
				{
					// Send a message on the crew channel.
					// Do this when someone is the captain, which could only be true if they were an agent. ;)
					QString text;
					if(DbCommodity::IsMissile(bayFired.m_Ammo->CommodityId()))
					{
						if(bayFired.m_UnitsFired > 1)
							text = tr("%2 %3 away, locked on %1.")
								.arg(target->Name()).arg(bayFired.m_UnitsFired).arg(bullet->Name());
						else
							text = tr("%2 away, locked on %1.")
								.arg(target->Name()).arg(bullet->Name());
					}
					else if(DbCommodity::IsDeployableWeapon(bayFired.m_Ammo->CommodityId()))
					{
						if(bayFired.m_UnitsFired > 1)
							text = tr("%2 %3 away.").arg(target->Name()).arg(bayFired.m_UnitsFired);
						else
							text = tr("%2 away.").arg(target->Name());
					}
					else if(DbCommodity::IsRocket(bayFired.m_Ammo->CommodityId()))
					{
						if(bayFired.m_UnitsFired > 1)
							text = tr("%2 %3 away, vectors to %1.")
								.arg(target->Name()).arg(bayFired.m_UnitsFired).arg(bullet->Name());
						else
							text = tr("%2 away, vectors to %1.")
								.arg(target->Name()).arg(bullet->Name());
					}
					else
					{
						if(bayFired.m_UnitsFired > 1)
							text = tr("%2 %3 fired at %1.")
								.arg(target->Name()).arg(bayFired.m_UnitsFired).arg(bullet->Name());
						else
							text = tr("%2 fired at %1.")
								.arg(target->Name()).arg(bullet->Name());
					}
					AuCommChat msg(text, AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
							Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::CrewMates);
					msg.SetScopeData(Id().GetID());	// Crew channel needs ship's id to know the scope of the message.
					g_Au.SendAtomChat(msg);
				}
			}
		}
	}
}

void DaVessel::DecayMoveActions()
{
	m_ControlLock.lockForRead();
	bool v = m_ThrustLateral ||
			m_ThrustLong ||
			m_ThrustVertical ||
			(m_wActions & (ACT_TurnMask|ACT_PitchMask|ACT_RollMask|ACT_BrakeMask));
	m_ControlLock.unlock();
	if(v)
	{
		// AI pilot does not depend on control decay.
		bool humanPilot = IsPilotMannedByPerson();
		if(!humanPilot && IsPilotMannedByAI())
			return;

		// Set control decay to work best with the pilot based on their ping time.
		int controlDecayMs;
		if(humanPilot)
		{
			DaPerson* humanPilotOnLine = dynamic_cast<DaPerson*>(Pilot());
			if(humanPilotOnLine)
				controlDecayMs = humanPilotOnLine->ControlDecayMs();	// Control decay is based on human pilot on line.
			else
				controlDecayMs = 250;
		}
		else
			controlDecayMs = 250;

		m_ControlLock.lockForWrite();

		// Shut off lateral and vertical throttles after a short time.
		if(m_ThrustLateral)
		{
			if(!m_tmLastThrottleLControl.isValid() || m_tmLastThrottleLControl.elapsed() > controlDecayMs)
			{
				if(m_ThrustLateral > 0)
					m_ThrustLateral--;
				else
					m_ThrustLateral++;
				SetModified();
			}
		}
		if(m_ThrustLong)
		{
			if(!m_tmLastThrottleFControl.isValid() || m_tmLastThrottleFControl.elapsed() > controlDecayMs)
			{
				if(m_ThrustLong > 0)
					m_ThrustLong--;
				else
					m_ThrustLong++;
				SetModified();
			}
		}
		if(m_ThrustVertical)
		{
			if(!m_tmLastThrottleVControl.isValid() || m_tmLastThrottleVControl.elapsed() > controlDecayMs)
			{
				if(m_ThrustVertical > 0)
					m_ThrustVertical--;
				else
					m_ThrustVertical++;
				SetModified();
			}
		}

		// Shut off turn after a short time to stop rotating.
		if(m_wActions & ACT_TurnMask)
		{
			if(!m_tmLastTurnControl.isValid() || m_tmLastTurnControl.elapsed() > controlDecayMs)
			{
				m_wActions &= ~ACT_TurnMask;
				SetModified(NOTIFY_Database);
			}
		}

		// Shut off pitch after a short time to stop rotating.
		if(m_wActions & ACT_PitchMask)
		{
			if(!m_tmLastPitchControl.isValid() || m_tmLastPitchControl.elapsed() > controlDecayMs)
			{
				m_wActions &= ~ACT_PitchMask;
				SetModified(NOTIFY_Database);
			}
		}

		// Shut off roll after a short time to stop rotating.
		if(m_wActions & ACT_RollMask)
		{
			if(!m_tmLastRollControl.isValid() || m_tmLastRollControl.elapsed() > controlDecayMs)
			{
				m_wActions &= ~ACT_RollMask;
				SetModified(NOTIFY_Database);
			}
		}

		// Shut off brakes after a short time to stop braking.
		if(m_wActions & ACT_Brake)
		{
			if(!m_tmLastBrake.isValid() || m_tmLastBrake.elapsed() > controlDecayMs)
			{
				m_wActions &= ~ACT_Brake;
				SetModified();
			}
		}

		// Shut off lateral/vertical brakes after a short time to stop braking.
		if(m_wActions & (ACT_BrakeLateral|ACT_BrakeVertical))
		{
			if(!m_tmLastBrakeLV.isValid() || m_tmLastBrakeLV.elapsed() > controlDecayMs)
			{
				m_wActions &= ~(ACT_BrakeLateral|ACT_BrakeVertical);
				SetModified();
			}
		}

		m_ControlLock.unlock();
	}
}

void DaVessel::Harvest(quint16 which, COMMODITYID bayModule, double wpnRange, double targDist, int probabilityToHarvest,
		COMMODITYID harvestCID, quint8 harvestQ)
{
	if(InArena())
		return;
	if(InDesignEnvironment())
		return;
	if(probabilityToHarvest <= 0)
		return;

	// Harvesters always fire at max power.
	float targetE;			// Energy to reach the target.
	if(wpnRange > 0 && targDist < wpnRange)
	{
		int weaponPower = GetPowerConsumption(SYS_Weapon, which);
		float rangeFactor = targDist/wpnRange;
		float wpnPower = BayWeaponPower(weaponPower, which);
		targetE = round(wpnPower * (1.f - rangeFactor));
	}
	else
		targetE = 0;

	// Harvest up to 1 unit for every fixed quantity of energy that hits target.
	int harvestUnits = (int)(targetE / HarvestUnitsPerEnergyUnit());
	if(harvestUnits <= 0)
		return;

	// Harvested units is modulated by probability of getting any at all.
	int harvestedUnits = harvestUnits * g_Au.Dice().Roll(2, probabilityToHarvest, -1) / 100;
	if(harvestedUnits <= 0)
		harvestedUnits = 1;

	// We harvested some.
	AuItem* harvested = AuItem::Create(harvestCID, harvestQ, EmpireId(), harvestedUnits);
	if(!harvested)
		return;

	// Preons store their solar system coordinates in the galaxy, when they are harvested. This is the stargate
	// destination when they are used to activate a ringworld stargate.
	const DaSolarSystem* system = FindParentSolarSystem();
	const DaGalaxy* galaxy = FindParentGalaxy();
	if(system && galaxy)
	{
		AuPoint galaxyLoc;
		system->GetLocationRelativeToParent(galaxyLoc, galaxy);
		harvested->SetGalaxyLoc(galaxy->Id(), galaxy->Name(), galaxyLoc);
	}

	if(CanAddCargoTrim(*harvested))
	{
		AddCargo(*harvested);
		AuCommHarvest msg(bayModule, *harvested,
				AuCommStdChannel::CrewChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Hail, AuCommStdChannel::CrewMates);
		msg.SetScopeData(Id().GetID());	// Crew channel needs ship id to resolve scope of message.
		g_Au.SendAtomChat(msg);
	}

	delete harvested;
}

bool DaVessel::GetNextWeaponBayFired(AuBayFired& bayFired, quint16& which)
{
	bool tractorStateChanged = false;	// Tractor state is visual on client so they need to be updated whenever it changes.
	bool fired = false;

	m_BayLock.lockForWrite();

	for(unordered_map<quint16, AuBayFired>::iterator lfi = m_BayFired.begin(); lfi != m_BayFired.end(); lfi++)
	{
		// Waiting for two seconds to pass since fired.
		// A tractor beam will return true continuously until it is detached.
		if((*lfi).second.IsReadyToEmit())
		{
			// Time to finish firing the weapon bay.
			bayFired = (*lfi).second;							// Last fired info.
			which = (*lfi).first;								// System which identifier.

			quint8 tractorState = (*lfi).second.m_TractorState;
			bool started = (*lfi).second.StartEmitting();
			if(tractorState != (*lfi).second.m_TractorState)
				tractorStateChanged = true;
			if(started)
				m_BayFired.erase(lfi);							// Remove the last fired info from the hash so it can fire again.
			fired = true;
			break;
		}
	}
	if(tractorStateChanged)
		m_tmBayFired.Set();										// Tractor state changed.

	m_BayLock.unlock();

	if(tractorStateChanged)
		SetModified();											// Client needs to be updated.
	return fired;
}

bool DaVessel::CanDisengageFTLNow(quint16 whichFTL) const
{
	return IsFTLDriveHot(whichFTL);
}

// Returns true if all systems are go for FTL commit at setting 0.
bool DaVessel::CanEngageFTLAt0(quint16 whichFTL) const
{
	if(!FTLDriveWorking(whichFTL))
		return false;

	if(FTLDriveCommitState(whichFTL) != FTL_COMMIT_Dormant)
		return false;

	// Check current adequacy of FTL drives.
	//qDebug() << "DaVessel::CanEngageFTLNow" << Id() << Name()
		//<< "systemHits" << SystemHitsRemaining(SYS_FTLDrive, whichFTL)
		//<< "powerCon" << GetPowerConsumption(SYS_FTLDrive, whichFTL)
		//<< "unitToEng" << FTLDriveUnitsToEngage();
	unsigned int pc = GetPowerConsumption(SYS_FTLDrive, whichFTL);
	if(pc < FTLDriveUnitsToEngage())
		return false;	// Insufficient functioning FTL drive units to engage.

	// Check capacitor power level.
	float ftlPowerNeeded = PowerNeededToEngageFTLDrive(whichFTL, 0);
	if(CapacitorLevel() < ftlPowerNeeded)
		return false;	// Insufficient capacitor power to FTL.

	return true;
}

// Returns true if all systems are go for FTL commit.
bool DaVessel::CanEngageFTLNow(quint16 whichFTL) const
{
	if(!FTLDriveWorking(whichFTL))
		return false;

	if(FTLDriveCommitState(whichFTL) != FTL_COMMIT_Dormant)
		return false;

	// Check current adequacy of FTL drives.
	//qDebug() << "DaVessel::CanEngageFTLNow" << Id() << Name()
		//<< "systemHits" << SystemHitsRemaining(SYS_FTLDrive, whichFTL)
		//<< "powerCon" << GetPowerConsumption(SYS_FTLDrive, whichFTL)
		//<< "unitToEng" << FTLDriveUnitsToEngage();
	unsigned int pc = GetPowerConsumption(SYS_FTLDrive, whichFTL);
	if(pc < FTLDriveUnitsToEngage())
		return false;	// Insufficient functioning FTL drive units to engage.

	// Check capacitor power level.
	float ftlPowerNeeded = PowerNeededToEngageFTLDrive(whichFTL, FTLDriveSetting(whichFTL));
	if(CapacitorLevel() < ftlPowerNeeded)
		return false;	// Insufficient capacitor power to FTL.

	return true;
}

// Returns true if all systems are go for FTL commit.
bool DaVessel::CanReengageFTLNow(quint16 whichFTL) const
{
	if(!FTLDriveWorking(whichFTL))
		return false;

	if(!IsFTLDriveHot(whichFTL))
		return false;

	// Check current adequacy of FTL drives.
	unsigned int pc = GetPowerConsumption(SYS_FTLDrive, whichFTL);
	if(pc < FTLDriveUnitsToEngage())
		return false;	// Insufficient functioning FTL drive units to engage.

	// Check capacitor power level.
	float ftlPowerNeeded = PowerNeededToEngageFTLDrive(whichFTL, FTLDriveSetting(whichFTL));
	if(CapacitorLevel() < ftlPowerNeeded)
		return false;	// Insufficient capacitor power to FTL.

	return true;
}

AuRosterEntry::TYPE DaVessel::CaptainsBerthType() const
{
	if(CaptainsBerths())
		return AuRosterEntry::Captain;
	else if(OfficerBerths())
		return AuRosterEntry::OfficerHuman;
	else if(PassengerBerths())
		return AuRosterEntry::PassengerHuman;
	else if(CrewBerths())
		return AuRosterEntry::Crew;
	else if(TroopBerths())
		return AuRosterEntry::Troop;
	else
		return AuRosterEntry::CaptainSlot;	// No berths, just a slot.
}

void DaVessel::DestroyShip(const QString& reason)
{
	// Create an effect atom to carry the explosion effect.
	DaAtom* parent = Parent();
	if(parent)
	{
		DaEffect* explode = parent->CreateEffect();

		// Add the explosion effect to the effect atom.
		// Loudness/size based on my size.
		if(LandingOrientation() == LORI_Tail)
		{
			AuMatrix4x4 myT(Transform());
			myT.Translate(TargetLocation());
			myT.RotateX(-PI/2);
			AuPoint myRot(myT.GetRotation());
			float gain = 1 + DesignHitsTotal();
			AuEffect effect(Transform().Project(DesignCenterOffset()), myRot, myT.ProjectVector(Vector()),
					SOUND_ExplodeSpacecraft, gain, 1, VISUAL_ExplodeSpacecraft);
			explode->AddEffect(effect);
		}
		else
		{
			AuMatrix4x4 myT(Transform());
			myT.Translate(TargetLocation());
			AuPoint myRot(Rotation());
			float gain = 1 + DesignHitsTotal();
			AuEffect effect(Transform().Project(DesignCenterOffset()), myRot, myT.ProjectVector(Vector()),
					SOUND_ExplodeSpacecraft, gain, 1, VISUAL_ExplodeSpacecraft);
			explode->AddEffect(effect);
		}
	}

	if(InDesignEnvironment())
	{
		// Turn off preview mode in designer. Don't kill anybody.
		DaDesigner* de = FindParentDesignerInEnv();
		if(de)
			de->SetPreview(0, 0, 0);
		return;
	}

	// Set to a non-normal atom state to prevent things from getting back aboard before we have decided to go dormant or die.
	SetTraveling();

	g_Au.StopMonitoringChannels(Id());				// Stop monitoring all comm channels.

	// Ships with a human officer will go dormant rather than being destroyed.
	bool goDormant = HasHumanOfficer();

	DaAtom* killer = 0;
	AuId killerId = KillerId();	// Award the kill to the atom that did the most damage; this could be a spaceship.
	if(!killerId.IsNull())
	{
		DaAtom* system = FindEnvironmentAtom();
		if(system)
		{
			// Reset last pirate station spawn time in the system when a pirate station is destroyed to hold them off for a while.
			//if(IsSpaceStation() && IsPirate())
				//system->SetLastPirateStation();

			// Locate the unit that killed me.
			killer = system->FindIdRecursive(killerId);
			if(killer)
				killer->UnitKilled(this);
		}
	}

	// Take people and citizens off the ship and kill them at my location.
	// Spacecraft and vehicles aboard are also destroyed.
	if(HasSubatoms())
	{
		// Award the kill(s) to the atom that did the most damage; this could be a spaceship.
		COMMODITYID weaponCID = DamagedUsingCID(killerId);
		quint8 weaponQ = DamagedUsingQ(killerId);
		QString killerName;
		if(killer)
			killerName = killer->Name();

		// Check for an officer aboard.
		AuIdList notified;										// Ids of people who are sent mail in ShipFinalLog.
		if(!IsPirateWithFreeWill())
		{
			// Non-pirate officer sends final ship log by mail.
			DaSentient* officer = FindOfficer();				// Find any officer, even if they don't have the con.
			// ShipLogFinal fills the 'notified' list with atoms who are sent mail.
			if(officer)
				officer->ShipLogFinal(this, reason, killerName, weaponCID, weaponQ, notified);
		}

		// This server's login credentials are needed to send mail.
		const DgData& gd = g_Au.LockDataForRead();
		unsigned int idPlayer = gd.PlayerId();
		unsigned int confirmNum = gd.ConfirmationNumber();
		g_Au.UnlockData();

		// Collect a list of the subatoms that must be destroyed. People and citizens are listed first because they could be aboard
		// subordinate vehicles or spacecraft. This way they get killed before the sub spacecraft or vehicle is destroyed.
		DaAtomList killSubAtoms;
		DaAtomList subs;
		ListSubatoms(subs);
		subs.FindLivePeopleRecursive(killSubAtoms);		// People atoms who are IsNormal or IsTraveling (spawning).
		subs.FindCitizensRecursive(killSubAtoms);			// Citizens are killed for scoring purposes.
		subs.FindItemsRecursive(killSubAtoms, false);		// Non-static sub-items are destroyed before vehicles and ships.
		subs.FindVehiclesRecursive(killSubAtoms);			// Vehicles are destroyed before sub spacecraft.
		subs.FindVesselsRecursive(killSubAtoms);		// Subordinate spacecraft are destroyed last, bottom to top.

		bool makeDebris = true;
		if(killSubAtoms.count() > 100)
		{
			makeDebris = false;
			qDebug() << "DaVessel::DestroyShip" << Id() << Name() << "reason" << reason
				<< "NOT MAKING DEBRIS for all" << killSubAtoms.count() << "subatoms";
			for(DaAtomList::const_iterator it = killSubAtoms.begin(); it != killSubAtoms.end(); it++)
				qDebug() << "  " << (*it)->Id() << (*it)->Name() << (*it)->Description();
		}

		// Destroy subatoms.
		AuIdList notifySubs;									// Ids of people who will be sent mail.
		for(DaAtomList::const_iterator it = killSubAtoms.begin(); it != killSubAtoms.end(); it++)
		{
			DaAtom* atom = *it;
			DaPerson* person = dynamic_cast<DaPerson*>(atom);
			if(person)
			{
				// Found a person aboard.
				if(person->IsNormal() && !notified.contains(person->Id()))
					notifySubs.append(person->Id());			// The person was not notified of the destruction of the ship.
			}

			// The atom is going to be killed here and now at the ship's location.
			// Select an appropriate weapon effect with which to destroy the atom.
			WEAPONEFFECT killType[] =
			{
				//WE_Acid,		// Acid sticks to players and they die repeatedly.
				WE_Suffocate,
				//WE_Stun,
				//WE_Poison,
				WE_Radiation,
				WE_Crush,
				WE_Sonic, 
				WE_Bludgeon,	// 8
				WE_Cold,
				//WE_Disease,
				WE_Electric,	// 11
				WE_Heat,
				WE_Pierce,		// 13
				//WE_Implant,
				WE_Slash,		// 15
			};
			int killTypes = sizeof(killType)/sizeof(WEAPONEFFECT);
			WEAPONEFFECT killEffect = killType[g_Au.Dice().Roll(1,killTypes,-1)];	// Random weapon effect for kill.

			// Destroy the atom.
			if(person)
			{
				if(person->IsNormal())
				{
					// Person is not awaiting spawn and they are not already a dead atom (killed atom vs dead body).
					// Throw them from the explosion in prone pose.
					person->SetGeneralPose(AuPose::Prone);
					person->CloseHelmetVisor();	// Automatically close their helmet for them.

					AuCombatAttacker attacker(killerId, killerName, killer ? killer->EmpireId() : 0);

					AuCombatWeapon weapon(WM_Internal);
					weapon.m_CID = weaponCID;
					weapon.m_Quality = weaponQ;
					weapon.m_w1.m_Effect = killEffect;

					AuCombatResult result;
					result.m_Damage1 = g_Au.Dice().Roll(1, person->CurrentHitPoints());	// Don't necessarily kill them.

					person->CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
				}
			}
			else
			{
				// The atom is not a person.
				DaNpc* officer = dynamic_cast<DaNpc*>(atom);
				if(officer && officer->IsNpcOfficer())
				{
					// Atom is an officer.
					// If this ship will go dormant, be sure the dying officer does not hold onto a berth.
					if(goDormant)
						RemoveBerthOccupant(officer->Id());

					// Kill the officer immediately, with no bleed-out time.
					// This is needed so the officer's BodyHasDied function gets called while the ship atom can still be found.
					// The BodyHasDied function needs to find the ship so it can store the event log onto the officer log dropped
					// by the dead officer.
					AuCombatAttacker attacker(killerId, killerName, killer ? killer->EmpireId() : 0);

					AuCombatWeapon weapon(WM_Internal);
					weapon.m_CID = weaponCID;
					weapon.m_Quality = weaponQ;
					weapon.m_w1.m_Effect = killEffect;

					AuCombatResult result;
					result.m_Damage1 = officer->CurrentHitPoints() - DpBody::DeadHitPoints();

					officer->CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
				}
				else
				{
					DaCitizen* citizen = dynamic_cast<DaCitizen*>(atom);
					if(citizen)
					{
						// Atom is a citizen.
						// If this ship will go dormant, be sure the dying citizen does not hold onto a berth.
						if(goDormant)
							RemoveBerthOccupant(citizen->Id());

						// Kill the citizen immediately, with random bleed-out time.
						int killPts = citizen->CurrentHitPoints() + g_Au.Dice().Roll(1,-DpBody::DeadHitPoints()/2,-1);
						if(!makeDebris)
						{
							// Too much junk aboard to make debris. Remove the citizen atom quietly.
							// Award the kill to the atom that did the most damage; this could be a spaceship.
							// We don't do this for officers because of the DeathLog they send in BodyHasDied.
							AuId killerId = KillerId();
							if(!killerId.IsNull())
							{
								DaAtom* system = FindEnvironmentAtom();
								if(system)
								{
									// Locate the atom that killed me.
									DaAtom* killer = system->FindIdRecursive(killerId);
									if(killer)
										killer->UnitKilled(citizen);
								}
							}
							citizen->Kill();
						}
						else
						{
							// This will change its pose to something appropriate.
							AuCombatAttacker attacker(killerId, killerName, killer ? killer->EmpireId() : 0);

							AuCombatWeapon weapon(WM_Internal);
							weapon.m_CID = weaponCID;
							weapon.m_Quality = weaponQ;
							weapon.m_w1.m_Effect = killEffect;

							AuCombatResult result;
							result.m_Damage1 = killPts;

							citizen->CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
						}
					}
					else
					{
						// Atom is not a citizen or a person.
						DaVessel* ship = dynamic_cast<DaVessel*>(atom);
						if(ship)
						{
							// Atom is a spaceship. We have already removed/killed all ships, vehicles, people, and citizens aboard
							// due to the order the list of subatoms was collected. Ships and vehicles are last in the list.
							ship->DamageAndDestroy(tr("Parent spacecraft %1 destroyed.").arg(Name()));
						}
						else
						{
							// Atom is not a citizen, person or spaceship.
							DaVehicle* vehicle = dynamic_cast<DaVehicle*>(atom);
							if(vehicle)
							{
								// Atom is a vehicle. We have already removed/killed everybody aboard it
								// due to the order the list of subatoms was collected. Ships and vehicles are last in the list.
								vehicle->DestroyVehicle(tr("Parent spacecraft %1 destroyed.").arg(Name()));
							}
							else
							{
								// Atom is not a citizen, person, spaceship, or vehicle.
								DaItem* item = dynamic_cast<DaItem*>(atom);
								if(item)
									item->Kill();	// Atom is an item that is not static displayed. We did not list static items.
							}
						}
					}
				}
			}

			// If the atom did not end up getting Kill'd in the process, move it from its parent and place it in the universe
			// subordinate to my parent atom. This tests for atomic death, not the death of a person's body.
			if(!atom->IsDead())
			{
				// Remove the atom from its parent, which could be a spacecraft or vehicle subordinate to me.
				// This is done after the atom is destroyed so officers can determine who their captain is in CombatApplyDamage.
				AuMatrix4x4 atomT;
				atom->GetTransformRelativeToParent(atomT, this);

				// Set the atom's location relative to my parent atom.
				AuPoint spreadVec = atomT.Project(AuPoint(0,0));	// Location relative to this ship is their spread vector.
				spreadVec.Unit(g_Au.Dice().Roll(1,50));				// Random vector length up to 50 feet per second
				atom->ForcePosMove(AuAtomPos(ParentId(), Transform() * atomT), Vector() + spreadVec);
				atom->SetModified();
			}
			else
				atom->TickTock();									// Get moving.
		}

		// Send mail to every avatar who was not already notified so they know what happened.
		if(!notifySubs.isEmpty())
			g_Au.SendBulkMail(idPlayer, confirmNum, Id(), Name(), notifySubs, AuMailMsg::MSG_Destroyed,
					tr("Spacecraft Destroyed"),
					tr("%1<p>%2").arg(g_Au.EncodeUTCString()).arg(reason), AuId(0), "", AuId(0), "", GALAXY_Count,
					AuPoint(0,0,0));
	}

	if(goDormant)
		GoDormant();
	else
		Kill();										// Destroy this ship and everything still aboard.
}

void DaVessel::GoDormant()
{
	// Remove all cargo.
	m_CargoLock.lockForWrite();
	m_Cargo.Clear();
	m_tmCargoChange.Set();
	m_CargoLock.unlock();

	DestroyCryoBerthOccupants();

	// Remove all destination teleporters from the ship so someone can't finish beaming aboard a dormant ship.
	DaAtomList teleporters;
	FindTeleportersRecursive(teleporters);
	for(DaAtomList::const_iterator ti = teleporters.begin(); ti != teleporters.end(); ti++)
	{
		DaTeleporter* teleporter = dynamic_cast<DaTeleporter*>(*ti);
		if(teleporter && !teleporter->IsOrigin())
			teleporter->Kill();					// Destination teleporters never host subatoms so this is safe.
	}

	// Repair systems and recharge the capacitor now so the ship is ready to reactivate.
	if(IsDesignDataCacheValid())
	{
		RepairCriticalSystems();

		// We depend on the damage change time to delay the recovery of ships in stasis.
		DamageStateChanged();

		m_SystemsLock.lockForWrite();
		m_PowerPlantShunts = 0;					// Disable all power plant shunts.
		m_ssaShipSystems.Reset();				// Reconnect all power relay connections and turn off overclocking.
		UpdateSystemsPowerSystemsLocked();	// Insure this is called since we fixed/changed shunts/connections/overclocking.
		m_SystemsLock.unlock();

		RechargeCapacitor();					// Recharge the capacitor in case there's no fuel.
	}
	else
	{
		// We depend on the damage change time to delay the recovery of ships in stasis.
		m_DamageLock.lockForWrite();
		m_tmDamageChange.Set();
		m_DamageLock.unlock();
	}

	// Ships go dormant for two reasons.
	// Ships with a human officer on the roster go dormant when destroyed. That results in this function call. Unmoor the ship so
	// we know that is the reason for its dormancy.
	// Ships go dormant when mothballed in a hangar. That case does not call this function. A hangared ship is moored so we know
	// it is in a hangar and not a ship waiting in stasis.
	Unmoor();

	// Set some values on this ship as part of going dormant.
	SetDormant();								// Ship atom becomes dormant until summoned by the captain or crew.
	SetAtRest();								// Stop moving.
	SetModified();
}

// Fill up with fuel from a city.
void DaVessel::FillUpFuel(DaBd* bd)
{
	if(!bd)
		return;
	AuItemHash inventory;
	bd->CommunityInventoryAvailable(inventory, EmpireId());
	quint32 fuelWanted;	// Amount of fuel wanted, recalculated in each iteration of the loop.
	while((fuelWanted = FuelWanted()) != 0)
	{
		// This ship wants fuel.
		// Locate fuel in the city's inventory.
		AuItemHash::iterator fuelI = inventory.find(CID_Hydrogen);
		if(fuelI == inventory.end())
			break;	// The city has no fuel.

		// Transfer hydrogen from the city to this ship, up to the amount we need to fill our fuel cells.
		AuItem* fuel = (*fuelI).second;
		if(fuel->Count() >= fuelWanted)
		{
			fuel->SetCount(fuelWanted);
			AddCargo(*fuel);
			bd->CommunityRemoveInventory(*fuel);
			break;
		}
		else
		{
			// We can take all of the fuel item.
			AddCargo(*fuel);
			bd->CommunityRemoveInventory(*fuel);
			inventory.Erase(fuelI);
		}
	}
}

void DaVessel::CreateDeviceObjects(AuAtomRenderInfo& ari)
{
	DaCapsule::CreateDeviceObjects(ari);

	if(!s_GameChair)
	{
		// Start loading the 3ds model using a worker thread.
		QString modelFile(":data/mGameChair.3dsN");
		if(!QFile::exists(modelFile))
			modelFile = ":data/BoltCube.3dsX";			// The model file does not exist so substitute with the bold cube.
		// This does not create the open gl display list.
		s_GameChair = dynamic_cast<Au3dsGLSLModelInstance*>(ari.Models().LoadModelThreaded(modelFile, true, 1));
		if(s_GameChair && s_GameChair->HasGLModel())
		{
			s_GameChair->GetExtents(s_GameChairMin, s_GameChairMax);
			s_GameChairBase = AuPoint((s_GameChairMin.x + s_GameChairMax.x)/2, (s_GameChairMin.y + s_GameChairMax.y)/2,
					s_GameChairMin.z - .083f);
			s_GameChairExt = true;
		}
	}
	else if(!s_GameChair->HasGLModel())
	{
		// The 3ds model load was started but the GL model hasn't been created yet.
		// When the model is ready, I can get its extents.
		// GLModel actually creates the open GL models after the worker thread has loaded the necessary data.
		// GLModel returns true when the open GL model is ready.
		if(s_GameChair->GLModel(*ari.re))
		{
			s_GameChair->GetExtents(s_GameChairMin, s_GameChairMax);
			s_GameChairBase = AuPoint((s_GameChairMin.x + s_GameChairMax.x)/2, (s_GameChairMin.y + s_GameChairMax.y)/2,
					s_GameChairMin.z - .083f);
			s_GameChairExt = true;
		}
	}
	else if(!s_GameChairExt)
	{
		s_GameChair->GetExtents(s_GameChairMin, s_GameChairMax);
		s_GameChairBase = AuPoint((s_GameChairMin.x + s_GameChairMax.x)/2, (s_GameChairMin.y + s_GameChairMax.y)/2,
				s_GameChairMin.z - .083f);
		s_GameChairExt = true;
	}

	if(!s_GameTable)
	{
		// Start loading the 3ds model using a worker thread.
		QString modelFile(":data/mGameTable.3dsN");
		if(!QFile::exists(modelFile))
			modelFile = ":data/BoltCube.3dsX";			// The model file does not exist so substitute with the bold cube.
		// This does not create the open gl display list.
		s_GameTable = dynamic_cast<Au3dsGLSLModelInstance*>(ari.Models().LoadModelThreaded(modelFile, true, 1));
		if(s_GameTable && s_GameTable->HasGLModel())
		{
			s_GameTable->GetExtents(s_GameTableMin, s_GameTableMax);
			s_GameTableBase = AuPoint((s_GameTableMin.x + s_GameTableMax.x)/2, (s_GameTableMin.y + s_GameTableMax.y)/2,
					s_GameTableMin.z - .083f);
			s_GameTableExt = true;
		}
	}
	else if(!s_GameTable->HasGLModel())
	{
		// The 3ds model load was started but the GL model hasn't been created yet.
		// When the model is ready, I can get its extents.
		// GLModel actually creates the open GL models after the worker thread has loaded the necessary data.
		// GLModel returns true when the open GL model is ready.
		if(s_GameTable->GLModel(*ari.re))
		{
			s_GameTable->GetExtents(s_GameTableMin, s_GameTableMax);
			s_GameTableBase = AuPoint((s_GameTableMin.x + s_GameTableMax.x)/2, (s_GameTableMin.y + s_GameTableMax.y)/2,
					s_GameTableMin.z - .083f);
			s_GameTableExt = true;
		}
	}
	else if(!s_GameTableExt)
	{
		s_GameTable->GetExtents(s_GameTableMin, s_GameTableMax);
		s_GameTableBase = AuPoint((s_GameTableMin.x + s_GameTableMax.x)/2, (s_GameTableMin.y + s_GameTableMax.y)/2,
				s_GameTableMin.z - .083f);
		s_GameTableExt = true;
	}
}

void DaVessel::AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim)
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	DaCapsule::AnimateDeviceObjects(ari, sceneAtom, lastAnim);

	if(lastAnim.ElapsedSecTicks())
	{
		// Decay console occupancy states.
		m_ConCaptAI = 0;			// 0=unknown, 1=no, 2=yes if AI has the con
		m_ConEngineerAI = 0;		// 0=unknown, 1=no, 2=yes
		m_ConEngineerPerson = 0;	// 0=unknown, 1=no, 2=yes
		m_ConNavigatorAI = 0;		// 0=unknown, 1=no, 2=yes
		m_ConNavigatorPerson = 0;	// 0=unknown, 1=no, 2=yes
		m_ConPilotAI = 0;			// 0=unknown, 1=no, 2=yes
		m_ConPilotPerson = 0;		// 0=unknown, 1=no, 2=yes
		m_ConPowerRelayAI = 0;		// 0=unknown, 1=no, 2=yes
		m_ConPowerRelayPerson = 0;	// 0=unknown, 1=no, 2=yes
		m_ConSensorAI = 0;			// 0=unknown, 1=no, 2=yes
		m_ConSensorPerson = 0;		// 0=unknown, 1=no, 2=yes
		m_ConShieldAI = 0;			// 0=unknown, 1=no, 2=yes
		m_ConShieldPerson = 0;		// 0=unknown, 1=no, 2=yes
		m_ConWeaponAI = 0;			// 0=unknown, 1=no, 2=yes
		m_ConWeaponPerson = 0;		// 0=unknown, 1=no, 2=yes
	}

	// Calculate my ETE for display screens and HUDs.
	double rdist=0.0;
	if(HasWaypoint())
	{
		const DeWaypoint* wp = LockWaypoint(0);
		if(wp)
		{
			const DaAtom* pwaydest = ether->FindNormalAtom(wp->m_idSystem);
			const DaSolarSystem* pwayss = dynamic_cast<const DaSolarSystem*>(pwaydest);
			if(pwaydest)
			{
				//qDebug("Found waypoint destination solarsystem, destid %s ssid %s", 
						//(const char*)wp->Destination().StringId(),
						//(const char*)pwaydest->Id().StringId());
				AuPoint fwp;
				const DaAtom* pdest = wp->Location(pwaydest, fwp);
				if(pdest)
				{
					DaAtom* mysys = FindEnvironmentAtom();
					if(mysys && mysys->Is(pwaydest->Id()))
					{
						// The next waypoint is in this system.
						if(wp->DestinationOffset().IsZero())
						{
							// The atom is the destination.
							AuPoint fship = EnvLocation();
							AuPoint fdest = pdest->EnvLocation();
							rdist = fship.Distance(fdest);
						}
						else
						{
							// The atom is a nav point.
							AuPoint fship = EnvLocation();
							rdist = fship.Distance(fwp);	// Offset is always relative to solar system.
						}
					}
					else if(pwayss)
					{
						//qDebug("Found waypoint destination atom is in another system");
						AuPoint fship = EnvLocation();
						AuMatrix4x4 mysysRot;
						mysysRot.Rotate(mysys->Rotation());
						fship = mysysRot.Project(fship);	// Relative to my sector.

						AuPoint fdest = pdest->EnvLocation();
						AuMatrix4x4 waysysRot;
						waysysRot.Rotate(pwayss->Rotation());
						fdest = waysysRot.Project(fdest);	// Relative to destination sector.

						const DaSector* pmysec = FindParentSector();
						const DaSector* pdsec = pdest->FindParentSector();
						if(pmysec == pdsec)
						{
							// These locations are in parsec units.
							AuPoint fmyss = mysys->Location();
							AuPoint fdss = pwayss->Location();
							fship += fmyss*FeetPerParsec();
							fdest += fdss*FeetPerParsec();
							rdist = fship.Distance(fdest);
						}
						else
						{
							// Sectors differ.
							DaGalaxy* pg = mysys->FindParentGalaxy();
							if(pg)
							{
								// These locations are in parsec units.
								AuPoint fmyss;
								mysys->GetLocationRelativeToParent(fmyss, pg);
								AuPoint fdss;
								pwayss->GetLocationRelativeToParent(fdss, pg);
								fship += fmyss*FeetPerParsec();
								fdest += fdss*FeetPerParsec();
								rdist = fship.Distance(fdest);
							}
						}
					}
				}
			}
		}
		UnlockWaypoint();
	}

	// Calc ete in seconds.
	float etesecs = 0;
	if(rdist > 0.0)
	{
		float speed = fabs(Speed());	// Can be negative when in reverse.
		if(speed != 0.0)
			etesecs = (float)rdist/speed;
	}
	else
		etesecs = 0;
	SetETE((int)ceil(etesecs));

	if(m_TractorBeam)
		m_TractorBeam->Update();
}

// Render circular nav beacon for stations.
void DaVessel::RenderStationNavBeacon(const AuMatrix4x4& projection, const AuMatrix4x4& model, const DaPerson* sceneAtom,
		float textBrightness, AuGLEffects& re)
{
	// This is a station that is not crashed on the ground.
	if(!sceneAtom->CanSeeBridgeIndicators())
		return;

	// Display the beacon around the station.
	// Calculate color of beacon.
	QColor color = PoliticalStanceColor(sceneAtom->EmpireId(), EmpireId(), IsCivilian());
	float r = color.redF() * textBrightness;
	float g = color.greenF() * textBrightness;
	float b = color.blueF() * textBrightness;

	// Flashing Beacon - Upright circle rotating around the Z axis.
	const float cycleSec = AuTime::CurrentMinCycle() * 60.f;	// 0 to 60 seconds
	const float zrot = cycleSec * (360.f / 5.f);				// angle per sec (5 seconds per cycle)
	AuMatrix4x4 bm(model);
	bm.RotateZ(zrot*DEG2RAD);			// Rotate arount Z

	// Select vertex shader and configure its parameters.
	re.GLSL().LineShader().Select();
	re.GLSL().LineShader().SetProjectionModelView(projection * bm);
	re.GLSL().LineShader().SetColor(r,g,b);

	const float beaconRadius = StationTradeRange();
	const int iarcstrokes = 100;
	AuPoint fa;

	// Compile a list of vertex locations.
	AuPointArray verts;

	// Draw XY plane circle.
	for(int iarc=0; iarc<=iarcstrokes; iarc++)
	{
		float ang = PIf*2.f * iarc / iarcstrokes;
		fa.x = cosf(ang) * beaconRadius;
		fa.y = sinf(ang) * beaconRadius;
		verts.append(AuPoint(fa.x, fa.y, 0));	// iarc=0 -> 1,0,0
	}

	// Draw XZ plane circle, the one that appears to rotate.
	for(int iarc=0; iarc<iarcstrokes; iarc++)
	{
		float ang = PIf*2.f * iarc / iarcstrokes;
		fa.x = cosf(ang) * beaconRadius;
		fa.y = sinf(ang) * beaconRadius;
		verts.append(AuPoint(fa.x, 0, fa.y));	// iarc=0 -> 1,0,0
	}

	// Make a VAO of the vertex locations.
	AuGLVAO vao;
	vao.CreateForLines(re, verts, true, GL_STREAM_DRAW);

	// Display the VAO once and clean it up.
	vao.SelectGL();
	vao.DrawGL(GL_LINE_STRIP);
	vao.DeselectGL();
	vao.DeleteGL();

	re.GLSL().LineShader().Deselect();
}

bool DaVessel::MissionHarvestOperationByBay(quint16 whichBay, AuHarvestOp& op) const
{
	bool opok = false;
	m_MissionLock.lockForRead();
	AuHarvestOps::const_iterator oi = m_Mission.CurrentOrder().HarvestOps().find(whichBay);
	if(oi != m_Mission.CurrentOrder().HarvestOps().cend())
	{
		op = *oi;
		opok = true;
	}
	m_MissionLock.unlock();
	return opok;
}

bool DaVessel::MissionHarvestOperationByModule(COMMODITYID bayModule, AuHarvestOp& op) const
{
	bool opok = false;
	m_MissionLock.lockForRead();
	AuHarvestOps::const_iterator oi = m_Mission.CurrentOrder().HarvestOps().find((quint16)bayModule);
	if(oi != m_Mission.CurrentOrder().HarvestOps().cend())
	{
		op = *oi;
		opok = true;
	}
	m_MissionLock.unlock();
	return opok;
}

int DaVessel::MissionIsHarvestOperationComplete() const
{
	quint8 fuelPerc = FuelPercent();
	quint32 fuelQty = FuelQuantity();
	quint32 fuelVolume = FuelCapacity();
	quint32 holdVolume = CargoCapacityVolume();

	int result = 0;		// 0=complete
	int holdFill = 0;	// 0=not filling, 1=filling, 2=full
	int fuelFill = 0;	// 0=not filling, 1=filling, 2=full
	m_MissionLock.lockForRead();
	// Ensure current order is a harvest operation.
	if(m_Mission.CurrentOrder().Type() == AuMissionOrder::HarvestOperationByBay ||
			m_Mission.CurrentOrder().Type() == AuMissionOrder::HarvestOperationByModule)
	{
		// Check fuel threshold.
		if(fuelPerc > m_Mission.CurrentOrder().FuelThreshold())
		{
			// Check each operation to see if it is satisfied.
			for(AuHarvestOps::const_iterator oi = m_Mission.CurrentOrder().HarvestOps().cbegin();
					oi != m_Mission.CurrentOrder().HarvestOps().cend(); oi++)
			{
				if((*oi).CommodityId() == CID_Hydrogen)
				{
					// Hydrogen needs fuel cell space.
					// fuelFill is 0 if this is the first hydrogen harvest op in the list.
					if(!fuelFill)
						fuelFill = (fuelQty >= fuelVolume) ? 2 : 1;	// 2 if full, 1 if filling
					if(fuelFill < 2)
					{
						// Fuel cells are not full.
						// How much does the operation want?
						quint32 amountWanted;
						if((*oi).AmountType() == 0)
							amountWanted = fuelVolume * (*oi).Amount() / 100;
						else
							amountWanted = (*oi).Amount();
						if(fuelQty < amountWanted)
						{
							// Found an unsatisfied harvest process.
							result = 4;
							break;
						}
					}
				}
				else
				{
					// Harvested materials need hold space.
					// holdFill is 0 if this is the first materials harvest op in the list.
					if(!holdFill)
						holdFill = (CargoTotalVolume() >= holdVolume) ? 2 : 1;	// 2 if full, 1 if filling
					if(holdFill < 2)
					{
						// Hold is not full.
						// How much does the operation want?
						quint32 amountWanted;
						if((*oi).AmountType() == 0)
							amountWanted = holdVolume * (*oi).Amount() / 100;
						else
							amountWanted = (*oi).Amount();
						if(CargoRawCount((*oi).CommodityId()) < amountWanted)
						{
							// Found an unsatisfied harvest process.
							result = 4;
							break;
						}
					}
				}
			}

			// Are we done?
			// result is still 0 if no unsatisfied harvest operations were found.
			if(!result)
			{
				// Check to see if we filled the hold before satisfying all the harvest operations.
				if((holdFill == 0 || holdFill == 2) &&		// True if not filling the hold or if the hold is full.
						(fuelFill == 0 || fuelFill == 2))	// True if not filling the fuel cell or if the fuel cell is full.
					result = 1;	// Whatever we are gathering is filled.
			}
		}
		else
			result = 3;	// Fuel is at or below threshold.
	}
	else
		result = 2;	// Current order is not a harvest operation.
	m_MissionLock.unlock();
	return result;
}

void DaVessel::ATCContact(const DaSolarSystem* system)
{
	// Search less often when at rest.
	if(m_ATCAtRest && !IsMoving() && LastATCContact().ElapsedSec() < 180)
		return;

	// Caller has insured I am not at rest, not a station, am in a scene, not in an arena, and not a pirate.
	const DaSector* sector = system->FindParentSector();
	if(!sector)
		return;	// wut?
	const DaGalaxy* galaxy = sector->FindParentGalaxy();
	if(!galaxy)
		return;	// wut?

	// Check for stations in the system whose sensor range we are inside of. It doesn't have to be the closest one.
	const DaVessel* atcStation = 0;
	const DaAtom* atcCity = 0;
	const DaVessel* enemyStation = 0;
	const DaAtom* enemyCity = 0;
	system->FindATC(EnvLocation(), EmpireId(), atcStation, atcCity, enemyStation, enemyCity);

	m_CommLock.lockForWrite();
	bool wasATCAtRest = m_ATCAtRest;
	m_ATCAtRest = !IsMoving();
	m_ATCContact.restart();

	AuId atcCityOld;
	if(!wasATCAtRest)
		atcCityOld = m_ATCCity;
	if(atcCity)
		m_ATCCity = atcCity->Id();
	else
		m_ATCCity = AuId();

	AuId enemyCityOld;
	if(!wasATCAtRest)
		enemyCityOld = m_ATCCityEnemy;
	if(enemyCity)
		m_ATCCityEnemy = enemyCity->Id();
	else
		m_ATCCityEnemy = AuId();

	AuId atcStationOld;
	if(!wasATCAtRest)
		atcStationOld = m_ATCStation;
	if(atcStation)
		m_ATCStation = atcStation->Id();
	else
		m_ATCStation = AuId();

	AuId enemyStationOld;
	if(!wasATCAtRest)
		enemyStationOld = m_ATCStationEnemy;
	if(enemyStation)
		m_ATCStationEnemy = enemyStation->Id();
	else
		m_ATCStationEnemy = AuId();

	// Nobody says anything if I was stopped last time and I am still stopped now.
	if(m_ATCAtRest && wasATCAtRest)
	{
		m_CommLock.unlock();
		return;
	}

	// ATC contacts.
	// Whose control area was I in?
	AuId oldController(atcStationOld);
	if(oldController.IsNull())
		oldController = atcCityOld;	// Could still be null.

	// ATC does not control ships at rest.
	AuId nuController;
	if(!m_ATCAtRest)
	{
		nuController = m_ATCStation;
		if(nuController.IsNull())
			nuController = m_ATCCity;	// could be null
	}

	if(oldController != nuController)
	{
		// There has been a change in controllers.
		if(!oldController.IsNull() && !nuController.IsNull())
		{
			// Handoff from one atc coverage area to another.
			if(oldController == atcStationOld)
			{
				// Departed atc station coverage area.
				const DaVessel* station = dynamic_cast<const DaVessel*>(system->FindIdRecursive(atcStationOld));
				if(station)
				{
					QString text;
					if(atcStation)
						text = QObject::tr("%1, you are departing my coverage area. Handing you off to %2 control.")
							.arg(Name())
							.arg(atcStation->Name());
					else
						text = QObject::tr("%1, you have departed my coverage area. Handing you off to %2 control.")
							.arg(Name())
							.arg(atcCity->CommName());
					AuCommHailReply msg(text, system->Id(), AuCommStdChannel::HailChannelName(), station->Id(), station->Name(),
							sector->Id(), galaxy->Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
					g_Au.SendAtomChat(msg);
				}
			}
			else
			{
				// Departed atc city coverage area.
				const DaAtom* city = system->FindIdRecursive(atcCityOld);
				if(city)
				{
					QString text;
					if(atcStation)
						text = QObject::tr("%1, you have departed my airspace. Handing you off to %2 control.")
							.arg(Name())
							.arg(atcStation->Name());
					else
						text = QObject::tr("%1, you are departing my airspace. Handing you off to %2 control.")
							.arg(Name())
							.arg(atcCity->CommName());
					AuCommHailReply msg(text, system->Id(), AuCommStdChannel::HailChannelName(), city->Id(), city->CommName(),
							sector->Id(), galaxy->Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
					g_Au.SendAtomChat(msg);
				}
			}
		}
		else if(!oldController.IsNull())
		{
			// Departed atc coverage area.
			if(oldController == atcStationOld)
			{
				// Departed atc station coverage area.
				const DaVessel* station = dynamic_cast<const DaVessel*>(system->FindIdRecursive(atcStationOld));
				if(station)
				{
					QString text;
					if(m_ATCAtRest)
						text = QObject::tr("%1, my sensors show you have stopped. Services terminated.").arg(Name());
					else
						text = QObject::tr("%1, sensor contact lost. Proceed VFR. Have a safe journey.").arg(Name());
					AuCommHailReply msg(text, system->Id(), AuCommStdChannel::HailChannelName(), station->Id(), station->Name(),
							sector->Id(), galaxy->Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
					g_Au.SendAtomChat(msg);
				}
			}
			else
			{
				// Departed atc city coverage area.
				const DaAtom* city = system->FindIdRecursive(atcCityOld);
				if(city)
				{
					QString text;
					if(m_ATCAtRest)
						text = QObject::tr("%1, radar services terminated. Enjoy your stay.").arg(Name());
					else
						text = QObject::tr("%1, radar contact lost. Proceed VFR. Have a good flight.").arg(Name());
					AuCommHailReply msg(text, system->Id(), AuCommStdChannel::HailChannelName(), city->Id(), city->CommName(),
							sector->Id(), galaxy->Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
					g_Au.SendAtomChat(msg);
				}
			}
		}
		else if(!nuController.IsNull())
		{
			// Entering atc coverage area.
			if(!m_ATCStation.IsNull())
			{
				// Entered coverage area of an atc station.
				QString text =
					QObject::tr("%1, sensor contact. Traffic separation and terrain avoidance services not available. Maintain VFR.") .arg(NameWithEmpire());
				AuCommHailReply msg(text, system->Id(), AuCommStdChannel::HailChannelName(), atcStation->Id(),
						atcStation->Name(), sector->Id(), galaxy->Galaxy(), AuCommStdChannel::SolarSystem,
						AuCommStdChannel::Everyone);
				g_Au.SendAtomChat(msg);
			}
			else
			{
				// Entered coverage area of an atc city.
				QString text =
					QObject::tr("%1, radar contact. No traffic separation or terrain avoidance services available. Maintain VFR.").arg(NameWithEmpire());
				AuCommHailReply msg(text, system->Id(), AuCommStdChannel::HailChannelName(), atcCity->Id(), atcCity->CommName(),
						sector->Id(), galaxy->Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
				g_Au.SendAtomChat(msg);
			}
		}
	}

	// Enemy contacts.
	// Whose control area was I in?
	AuId oldEnemy(enemyStationOld);
	if(oldEnemy.IsNull())
		oldEnemy = enemyCityOld;	// Could still be null.

	// Enemy ATC does not control ships at rest.
	AuId nuEnemy;
	if(!m_ATCAtRest)
	{
		nuEnemy = m_ATCStationEnemy;
		if(nuEnemy.IsNull())
			nuEnemy = m_ATCCityEnemy;	// could be null
	}

	if(oldEnemy != nuEnemy)
	{
		// There has been a change in enemy tracking me.
		if(!oldEnemy.IsNull() && !nuEnemy.IsNull())
		{
			// Handoff from one enemy coverage area to another.
			if(oldEnemy == enemyStationOld)
			{
				// Departed enemy station coverage area.
				const DaVessel* station = dynamic_cast<const DaVessel*>(system->FindIdRecursive(enemyStationOld));
				if(station)
				{
					QString text;
					if(enemyStation)
						text = QObject::tr("%1, you are heading into the %2 ADIZ. Prepare to be destroyed.")
							.arg(Name())
							.arg(enemyStation->NameWithEmpire());
					else
					{
						QString empire;
						const DgData& gd = g_Au.LockDataForRead();
						gd.EmpireName(enemyCity->EmpireId(), empire);
						g_Au.UnlockData();
						text = QObject::tr("%1, you are heading straight for the %2 %3 ADIZ. Prepare to be destroyed.")
							.arg(Name())
							.arg(empire)
							.arg(enemyCity->CommName());
					}
					AuCommHailReply msg(text, system->Id(), AuCommStdChannel::HailChannelName(), station->Id(), station->Name(),
							sector->Id(), galaxy->Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
					g_Au.SendAtomChat(msg);
				}
			}
			else
			{
				// Departed enemy city coverage area.
				const DaAtom* city = system->FindIdRecursive(enemyCityOld);
				if(city)
				{
					QString text;
					if(enemyStation)
						text = QObject::tr("%1, you are entering the %2 ADIZ. Prepare to be destroyed.")
							.arg(Name())
							.arg(enemyStation->NameWithEmpire());
					else
					{
						QString empire;
						const DgData& gd = g_Au.LockDataForRead();
						gd.EmpireName(enemyCity->EmpireId(), empire);
						g_Au.UnlockData();
						text = QObject::tr("%1, you have entered the %2 %3 ADIZ. Prepare to be destroyed.")
							.arg(Name())
							.arg(empire)
							.arg(enemyCity->CommName());
					}
					AuCommHailReply msg(text, system->Id(), AuCommStdChannel::HailChannelName(), city->Id(), city->CommName(),
							sector->Id(), galaxy->Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
					g_Au.SendAtomChat(msg);
				}
			}
		}
		else if(!oldEnemy.IsNull())
		{
			// Departed enemy coverage area.
			if(oldEnemy == enemyStationOld)
			{
				// Departed enemy station coverage area.
				const DaVessel* station = dynamic_cast<const DaVessel*>(system->FindIdRecursive(enemyStationOld));
				if(station)
				{
					QString text;
					if(m_ATCAtRest && IsInAtmosphere())
						text = QObject::tr("%1, you can't hide from our sensors. Get out of the ADIZ or be destroyed.")
							.arg(Name());
					else if(m_ATCAtRest)
						text = QObject::tr("%1, you are still in our space. Get out of the ADIZ or be destroyed.")
							.arg(Name());
					else
						text = QObject::tr("%1, you are clear of our space. Remain outside the ADIZ or be destroyed.")
							.arg(Name());
					AuCommHailReply msg(text, system->Id(), AuCommStdChannel::HailChannelName(), station->Id(), station->Name(),
							sector->Id(), galaxy->Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
					g_Au.SendAtomChat(msg);
				}
			}
			else
			{
				// Departed enemy city coverage area.
				const DaAtom* city = system->FindIdRecursive(enemyCityOld);
				if(city)
				{
					QString text;
					if(m_ATCAtRest && IsInAtmosphere())
						text = QObject::tr("%1, you cannot hide. We have a fix on your position. "
								"Get out of the ADIZ or be destroyed.")
							.arg(Name());
					else if(m_ATCAtRest)
						text = QObject::tr("%1, you are still in our airspace. Get out of the ADIZ or be destroyed.")
							.arg(Name());
					else
						text = QObject::tr("%1, you are clear of our airspace. Remain outside the ADIZ or be destroyed.")
							.arg(Name());
					AuCommHailReply msg(text, system->Id(), AuCommStdChannel::HailChannelName(), city->Id(), city->CommName(),
							sector->Id(), galaxy->Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
					g_Au.SendAtomChat(msg);
				}
			}
		}
		else if(!nuEnemy.IsNull())
		{
			// Entering enemy coverage area.
			if(!m_ATCStationEnemy.IsNull())
			{
				// Entered coverage area of an enemy station.
				QString empire;
				const DgData& gd = g_Au.LockDataForRead();
				gd.EmpireName(enemyStation->EmpireId(), empire);
				g_Au.UnlockData();
				QString text = QObject::tr("%1, sensor contact. %2 ADIZ incursion! "
						"Leave this area immediately or you will be destroyed.")
					.arg(NameWithEmpire())
					.arg(empire);
				AuCommHailReply msg(text, system->Id(), AuCommStdChannel::HailChannelName(), enemyStation->Id(),
						enemyStation->Name(), sector->Id(), galaxy->Galaxy(), AuCommStdChannel::SolarSystem,
						AuCommStdChannel::Everyone);
				g_Au.SendAtomChat(msg);
			}
			else
			{
				// Entered coverage area of an enemy city.
				QString empire;
				const DgData& gd = g_Au.LockDataForRead();
				gd.EmpireName(enemyCity->EmpireId(), empire);
				g_Au.UnlockData();
				QString text = QObject::tr("%1, radar contact. %2 ADIZ incursion! "
						"Leave this area immediately or you will be destroyed.")
					.arg(NameWithEmpire())
					.arg(empire);
				AuCommHailReply msg(text, system->Id(), AuCommStdChannel::HailChannelName(), enemyCity->Id(), enemyCity->CommName(),
						sector->Id(), galaxy->Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
				g_Au.SendAtomChat(msg);
			}
		}
	}

	m_CommLock.unlock();
}

void DaVessel::RenderGameTable(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, const AuPoint& loc,
		float rotZ, quint16 whichTable)
{
	if(!s_GameTable || !s_GameChair)
		return;
	if(!s_GameTable->HasGLModel())
		return;
	if(!s_GameChair->HasGLModel())
		return;

	// Multiply by current transformation (Translation, rotation, and scale)
	AuPoint tloc(loc);
	tloc.z -= s_GameTableBase.z;
	AuMatrix4x4 tcon;
	tcon.Translate(tloc);
	tcon.RotateZ(rotZ);
	AuMatrix4x4 tThis = model * tcon;

	AuPoint ambientUp = tThis.ProjectVector(0,0,1);

	s_GameTable->RenderGLSLModel(projection, tThis, re, ambientUp, false);

	RenderGameChair(projection, model, re, loc, -135.f*DEG2RADf+rotZ, rotZ);
	RenderGameChair(projection, model, re, loc, -45+rotZ, rotZ + 90.f*DEG2RADf);
	RenderGameChair(projection, model, re, loc, 45.f*DEG2RADf+rotZ, rotZ + 180.f*DEG2RADf);
	RenderGameChair(projection, model, re, loc, 135.f*DEG2RADf+rotZ, rotZ + 270.f*DEG2RADf);

	// Render active game.
	unordered_map<quint16, AuGame*>::iterator gi = m_Games.find(whichTable);
	if(gi != m_Games.end())
	{
		// There is an active game at the table.
		// Move model space to be centered at the top of the table.
		AuPoint tloc(loc);
		tloc.z += (s_GameTableMax.z - s_GameTableMin.z) + .083;		// Move up to the surface of the table.
		AuMatrix4x4 tcon;
		tcon.Translate(tloc);
		tcon.RotateZ(rotZ);

		// Now rotate the Z axis so the dealer is always along the -Y axis.
		float seatRots[] =
		{
			135 * DEG2RADf,
			45 * DEG2RADf,
			-45 * DEG2RADf,
			-135 * DEG2RADf,
		};
		tcon.RotateZ(seatRots[(*gi).second->DealerSeat()]);
		AuMatrix4x4 tThis = model * tcon;

		// Draw the game effect.
		(*gi).second->Render(projection, tThis, re);
	}
}

void DaVessel::RenderGameChair(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, const AuPoint& loc,
		float rotZ, float rotAroundTable)
{
	// Multiply by current transformation (Translation, rotation, and scale)
	AuMatrix4x4 tcon;
	tcon.Translate(loc);					// Translate to table center.
	tcon.RotateZ(rotAroundTable);			// Rotate around the center.
	tcon.Translate(4.5,4.5,-s_GameChairBase.z);	// Translate to chair location.
	tcon.RotateZ(rotZ-rotAroundTable);		// Rotate chair to desired final rotation.
	AuMatrix4x4 tThis = model * tcon;

	AuPoint ambientUp = tThis.ProjectVector(0,0,1);

	s_GameChair->RenderGLSLModel(projection, tThis, re, ambientUp, false);
}

bool DaVessel::CheckAccess(const DaUnit* unit) const
{
	// Units in the chain of command of the ship are always granted access. This includes people with agent powers enabled.
	if(InChainOfCommand(unit))
		return true;

	switch(AccessPolicy())
	{
	case ACCESS_Command:
		return false;					// Policy prohibits access to people who are not in the chain of command.
	case ACCESS_Crew:
		if(!IsCrewOrTroop(unit->Id()))
			return false;				// Policy prohibits access to people who are not crew or troops.
		break;
	case ACCESS_CrewAndPassengers:
		if(BerthTypeOccupied(unit->Id()) == AuRosterEntry::Invalid)
			return false;				// Policy prohibits access to people who are not on the roster.
		break;
	case ACCESS_Empire:
		if(BerthTypeOccupied(unit->Id()) != AuRosterEntry::Invalid)
			break;						// Access granted to crew and passengers.
		else if(EmpireId() != unit->EmpireId())
			return false;				// Policy prohibits access to people who are not in my empire.
		break;
	case ACCESS_Friend:
		if(BerthTypeOccupied(unit->Id()) != AuRosterEntry::Invalid)
			break;						// Access granted to crew and passengers.
		else
		{
			POLITICAL_STANCE stanceToUnit = PoliticalStanceMakeContact(unit);
			if(stanceToUnit == POL_Enemy || stanceToUnit == POL_Neutral)
				return false;			// Policy prohibits access to members of enemy and neutral empires.
		}
		break;
	case ACCESS_Neutral:
		if(BerthTypeOccupied(unit->Id()) != AuRosterEntry::Invalid)
			break;						// Access granted to crew and passengers.
		else
		{
			POLITICAL_STANCE stanceToUnit = PoliticalStanceMakeContact(unit);
			if(stanceToUnit == POL_Enemy)
				return false;			// Policy prohibits access to members of enemy empires.
		}
		break;
	case ACCESS_Enemy:
	default:
		// Everybody is welcome.
		break;
	}
	return true;
}

bool DaVessel::IsSmoldering() const
{
	// Ships in the design studio do not smolder.
	if(InDesignEnvironment())
		return false;

	unsigned int maxHits = DesignHitsStructural();
	unsigned int dmgHits = HitsDamageStructural();
	return dmgHits > maxHits/2;
}

bool DaVessel::RenderTractorBeams(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re)
{
	bool hasTractorBeam = false;
	m_BayLock.lockForRead();
	for(unordered_map<quint16, AuBayFired>::const_iterator lfi = m_BayFired.cbegin(); lfi != m_BayFired.cend(); lfi++)
		if((*lfi).second.IsTractorAttached())
		{
			//qDebug() << "DaVessel::RenderTractorBeams" << Id() << Name() << "tractor attached to" << (*lfi).second.m_Target;
			// Found tractor attached. Locate the target.
			DaAtom* envAtom = FindEnvironmentAtom();
			if(!envAtom)
				continue;

			DaAtom* target = envAtom->FindIdRecursive((*lfi).second.m_Target);
			if(!target)
				continue;
			AuPoint tloc = target->EnvTransform().Project(target->TargetLocation());	// Target location in solar system space.
			tloc = EnvTransform().UnProject(tloc);	// Target in my space.
			//tloc -= m_DesignAnalysis.CG();

			// Draw the tractor beam effect.
			if(!m_TractorBeam)
			{
				GLuint texId = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "VTractorBeam", ":data/BeamTractor.png");
				GLuint texIdGlow = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "tBCM", ":data/particle.bmp");
				GLuint texIdGrab = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID,  "WSt", ":data/stunsprite.png");
				m_TractorBeam = new AuTractorBeam;
				m_TractorBeam->SetMaxParticles(100);
				m_TractorBeam->SetLifeCycle(1.0f);
				m_TractorBeam->SetNumToRelease(1);
				m_TractorBeam->SetReleaseInterval(0.0f);
				m_TractorBeam->SetSize(2000);
				m_TractorBeam->SetTargetSize(target->ObstacleRadius());
				m_TractorBeam->SetTextureID(texId);
				m_TractorBeam->SetTextureID2(texIdGlow);	// Gun end.
				m_TractorBeam->SetTextureID3(texIdGrab);	// Target end.
			}
			m_TractorBeam->SetVelocity((tloc-(*lfi).second.m_EmitLoc) * 2);
			m_TractorBeam->SetLength(tloc.Distance((*lfi).second.m_EmitLoc));
			m_TractorBeam->SetTargetLoc(tloc);

			// Render the tractor beam.
			re.glDepthMask(GL_FALSE);	// This way the particles do not have to be sorted
			re.glEnable(GL_BLEND);
			re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);	// An additive blend

			AuMatrix4x4 tThis(model);
			tThis.Translate((*lfi).second.m_EmitLoc);

			m_TractorBeam->Render(projection, tThis, re);

			re.glDisable(GL_BLEND);
			re.glDepthMask(GL_TRUE);

			hasTractorBeam = true;
		}
	m_BayLock.unlock();
	return hasTractorBeam;
}

void DaVessel::RenderShieldBubble(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
		const DaPerson* sceneAtom)
{
	double shieldMax = ShieldUnitsWorking();
	if(shieldMax <= 0)
		return;

	// Render shield bubble.
	if(m_shieldTime.isValid())
		m_fShieldTexOffset += m_shieldTime.elapsed()*(.005f);
	else
		m_shieldTime.restart();

	// Offset to center of ship hull, which may not be the current pivot point.
	AuPoint cen(DesignCenterOffset());
	AuMatrix4x4 shieldM(model);
	shieldM.Translate(cen);

	// Calculate shield shader parameters.
	float fDivisor = 20.0f;
	AuPoint eRadius = ShieldRadii();
	float sFront = ShieldCapacitor(SHIELD_Front)/shieldMax * 10;
	float sBack = ShieldCapacitor(SHIELD_Back)/shieldMax * 10;
	float sLeft = ShieldCapacitor(SHIELD_Left)/shieldMax * 10;
	float sRight = ShieldCapacitor(SHIELD_Right)/shieldMax * 10;
	float sTop = ShieldCapacitor(SHIELD_Top)/shieldMax * 10;
	float sBottom = ShieldCapacitor(SHIELD_Bottom)/shieldMax * 10;
	bool sfr = ShieldScram(SHIELD_Front);
	bool sba = ShieldScram(SHIELD_Back);
	bool sle = ShieldScram(SHIELD_Left);
	bool sri = ShieldScram(SHIELD_Right);
	bool sto = ShieldScram(SHIELD_Top);
	bool sbo = ShieldScram(SHIELD_Bottom);
	for(int iSDEffect=0; iSDEffect<MAX_SHIELD_HIT_EFFECTS; iSDEffect++)
		m_fShieldDuration[iSDEffect] = 0;
	int iShieldEffects = 0;	// will hold final count of effects on shields
	if(EffectsOnClient())
	{
		// My eff
		float fEffectDurationAmt = 0;
		int iEffect = 0;
		QList<AuEffect>::iterator it;
		for(it = EffectsOnClient()->m_Effects.begin(); it != EffectsOnClient()->m_Effects.end(); it++)
		{
			if(iShieldEffects >= MAX_SHIELD_HIT_EFFECTS || iEffect >= MAX_SHIELD_HIT_EFFECTS*4)
				break;	// Don't do more than allowed
			if(!(*it).IsShieldHit())
				continue;
			//if(!(*it).IsHitEffect())
				//continue;
			float durationLeft = (*it).VisualDuration()-(*it).SecondsOffset();
			if(durationLeft <= 0)
				continue;
			AuPoint pHitLoc = (*it).OriginLoc() - cen;	// Shield bubble wants these relative to the bubble.
			m_fShieldEffect[iEffect++] = (float)pHitLoc.x;
			m_fShieldEffect[iEffect++] = (float)pHitLoc.y;
			m_fShieldEffect[iEffect++] = (float)pHitLoc.z;
			float hitGain = 100.f + (*it).Gain() * 50.f;
			if(hitGain > 2000.f)
				hitGain = 2000.f;
			m_fShieldEffect[iEffect++] = hitGain;

			//qDebug() << "effect on shields, gain" << hitGain << "at" << pHitLoc << "dur" << durationLeft;
			m_fShieldDuration[iShieldEffects++] = durationLeft;
			fEffectDurationAmt += durationLeft * hitGain / 1000;
		}

		// Decrease divisor to Increase the shield color based on the number of effects
		// hitting the shield. This works out because the duration of both effects is 1
		// second so the max effect duration amount would be equal to MAX_SHIELD_HIT_EFFECTS.
		fDivisor = fDivisor - fDivisor*(fEffectDurationAmt/MAX_SHIELD_HIT_EFFECTS);
		if(fDivisor < 2)
			fDivisor = 2;
		//if(iEffect)
			//qDebug() << "effects on shields" << (iEffect/4) << "divisor" << fDivisor;
	}

	// Determine if viewer is inside or outside the bubble.
	DaAtom* pParent = sceneAtom->Parent();
	bool bInside = false;
	if(pParent == this || pParent->Parent() == this)
		bInside = true;
	else
	{
		AuPoint pScenePoint = EnvTransform().UnProject(sceneAtom->EnvLocation());
		// (x/a)² + (y/b)² + (z/c)² = 1
		double x = pScenePoint.x/eRadius.x;
		double y = pScenePoint.y/eRadius.y;
		double z = pScenePoint.z/eRadius.z;
		double dValue = x*x + y*y + z*z;
		if(dValue <= 1.01)	// Fudge the distance value a little to help in the transition
			bInside = true;
	}

	if(bInside)
	{
		ari.re->glDisable(GL_CULL_FACE);
		ari.re->RenderShieldBubble(projection, shieldM, eRadius.x, eRadius.y, eRadius.z, sFront, sfr, sBottom, sbo, sLeft, sle,
				sBack, sba, sTop, sto, sRight, sri, m_fShieldTexOffset, fDivisor, m_fShieldEffect, m_fShieldDuration,
				iShieldEffects);
		ari.re->glEnable(GL_CULL_FACE);
	}
	else
		ari.re->RenderShieldBubble(projection, shieldM, eRadius.x, eRadius.y, eRadius.z, sFront, sfr, sBottom, sbo, sLeft, sle,
				sBack, sba, sTop, sto, sRight, sri, m_fShieldTexOffset, fDivisor, m_fShieldEffect, m_fShieldDuration,
				iShieldEffects);
}

bool DaVessel::HasVehicleTransporter() const
{
	m_SystemsLock.lockForRead();
	bool hasIt = m_ssaShipSystems.HasVehicleTransporter();
	m_SystemsLock.unlock();
	return hasIt;
}

bool DaVessel::IsHidingInGroundClutter() const
{
	return !IsNew() &&
		IsOnGround() &&
		!IsMoving() &&
		!ShieldsWorkingAndUp() &&
		!SensorsWorking() &&
		!PowerPlantWorking() &&
		!GravityDriveWorking() &&
		!IsAnyFTLDriveWorking();
}

bool DaVessel::CanOpenDoor(const DaSentient& who, unsigned int doorIndex) const
{
	if(IsHullDoor(doorIndex))
	{
		// The door is a hull door.
		if(who.IsInsideSpacecraft() && who.FindParentVesselInEnv() == this)
			return true;				// Hull doors are never locked from the inside.
		// The person is outside this ship.
		if(!IsDoorLocked(doorIndex))
			return CheckAccess(&who);	// Hull door is not locked so access policy decides if someone can enter from the outside.
	}
	else
	{
		// The door is an interior door.
		if(!IsDoorLocked(doorIndex))
			return true;				// The interior door is not locked.
	}
	// The door is locked.
	if(IsCrewOrTroop(who.Id()))
		return true;					// Officers, crew and troops can open any door.
	// The person is not an officer, crew or troop.
	if(InChainOfCommand(&who))
		return true;					// People in the chain of command of the ship can open any door.
	return false;						// Door cannot be opened.
}

quint32 DaVessel::CryoBerthsAvailable() const
{
	return CryoBerths() - CryoBerthsOccupied();
}

quint32 DaVessel::CryoBerthsOccupied() const
{
	m_RosterLock.lockForRead();
	quint32 v = m_CryoBerths.CountOccupants();
	m_RosterLock.unlock();
	return v;
}

quint32 DaVessel::CryoBerthsOccupied(QStringList& cryoSpecies) const
{
	m_RosterLock.lockForRead();
	quint32 v = 0;
	for(AuCryoBerths::const_iterator ci = m_CryoBerths.cbegin(); ci != m_CryoBerths.cend(); ci++)
	{
		QString speciesName = ci.key().NameOrDesc();
		if(!cryoSpecies.contains(speciesName))
			cryoSpecies.append(speciesName);
		v += *ci;
	}
	m_RosterLock.unlock();
	return v;
}

void DaVessel::BoardCryoTraveller(const AuSpecies& species)
{
	m_RosterLock.lockForWrite();
	m_CryoBerths.AddOccupant(species);
	m_tmRosterChange.Set();
	m_RosterLock.unlock();
	SetModified();
}

AuSpecies DaVessel::RemoveAnyCryoBerthOccupant()
{
	m_RosterLock.lockForWrite();
	AuSpecies species = m_CryoBerths.RemoveAnyOccupant();
	if(!species.IsNull())
		m_tmRosterChange.Set();
	m_RosterLock.unlock();
	if(species.IsNull())
		return AuSpecies();
	SetModified();
	return species;
}

bool DaVessel::RemoveCryoBerthOccupant(const AuSpecies& species)
{
	m_RosterLock.lockForWrite();
	bool removed = m_CryoBerths.RemoveOccupant(species);
	if(removed)
		m_tmRosterChange.Set();
	m_RosterLock.unlock();
	if(removed)
		SetModified();
	return removed;
}

quint32 DaVessel::KickCryoBerthOccupants(const AuSpecies& species, quint32 count)
{
	m_RosterLock.lockForWrite();
	quint32 removed = m_CryoBerths.RemoveOccupants(species, count);
	if(removed)
		m_tmRosterChange.Set();
	m_RosterLock.unlock();
	if(removed)
		SetModified();
	return removed;
}

DaNpc* DaVessel::ReviveCryoBerthOccupant(const AuMatrix4x4& where, const AuSpecies& species, quint8 role)
{
	AuEther* ether = Ether();
	if(!ether)
		return 0;
	// Try to remove an occupant from the cryo berths.
	m_RosterLock.lockForWrite();
	bool removed = m_CryoBerths.RemoveOccupant(species);
	if(removed)
		m_tmRosterChange.Set();
	m_RosterLock.unlock();
	if(!removed)
		return 0;
	SetModified();

	QString name = g_Au.Dice().RandomPersonName();
	AuDNABase::CHROMATID chroma = (AuDNABase::CHROMATID)g_Au.Dice().Roll(1,2,-1);
	HANDEDNESS hand = (HANDEDNESS)g_Au.Dice().Roll(1,2,-1);

	DaNpc* npc;
	switch(role)
	{
	default:
	case 0:	// No role, make a worker.
		npc = new DaNpc(ROLE_Worker, species, chroma, hand, name);
		break;
	case 1:	// Crew
		npc = new DaNpc(ROLE_Crew, species, chroma, hand, name);
		break;
	case 2:	// Troop
		npc = new DaNpc(ROLE_Troop, species, chroma, hand, name);
		break;
	case 3:	// Passenger
		npc = new DaNpc(ROLE_Passenger, species, chroma, hand, name);
		break;
	}
	npc->ConnectSignals();
	npc->Create();
	npc->SetPosAndOrient(AuAtomPos(Id(), where));
	ether->AddAtom(npc);

	npc->ResetHitPoints();
	npc->ResetHunger();
	npc->ResetStamina();
	npc->SetEmpireId(EmpireId());
	npc->Stand();
	npc->InitTraining();
	npc->SetInScene();					// Only spawns when in a scene. Set this so it doesn't suicide immediately.
	g_Au.AtomAdded(npc->Id());
	npc->SetNormal();					// New atom is ready for normal thread processing.

	// Occupy a berth.
	switch(role)
	{
	case 0:	// No role, make a worker.
	default:
		break;
	case 1:	// Crew
		if(CrewBerthsAvailable())
			npc->OccupyBerth(this);
		break;
	case 2:	// Troop
		if(TroopBerthsAvailable())
			npc->OccupyBerth(this);
		break;
	case 3:	// Passenger
		if(PassengerBerthsAvailable())
			npc->OccupyBerth(this);
		break;
	}
	npc->TickTock();
	return npc;
}

quint32 DaVessel::DestroyCryoBerthOccupants()
{
	m_RosterLock.lockForWrite();
	quint32 removed = m_CryoBerths.CountOccupants();
	if(removed)
	{
		m_CryoBerths.clear();
		m_tmRosterChange.Set();
	}
	m_RosterLock.unlock();
	if(removed)
		SetModified();
	return removed;
}

COMMODITYID DaVessel::FuelType() const
{
	return CID_Hydrogen;
}

void DaVessel::AccelSkewWander()
{
	if(EnginesRequireEngineer())
	{
		const int skewMax = 40 - 10 * m_FuelQuality/255;	// Skew wander is restricted when fuel quality is better.
		char chWander = (char)g_Au.Dice().Roll(1,5,-3);		// -2, -1, 0, 1 or 2
		if(m_chAccelSkew > skewMax)
		{
			// Current skew is greater than max.
			// Only allow negative wander in this zone.
			if(chWander < 0)
			{
				m_chAccelSkew += chWander;
				if(m_chAccelSkew < -skewMax)
					m_chAccelSkew = -skewMax;
			}
		}
		else if(m_chAccelSkew < -skewMax)
		{
			// Current skew is less than min.
			// Only allow positive wander in this zone.
			if(chWander > 0)
			{
				m_chAccelSkew += chWander;
				if(m_chAccelSkew > skewMax)
					m_chAccelSkew = skewMax;
			}
		}
		else
		{
			// Positive or negative wander is allowed.
			m_chAccelSkew += chWander;
			if(m_chAccelSkew > skewMax)
				m_chAccelSkew = skewMax;
			else if(m_chAccelSkew < -skewMax)
				m_chAccelSkew = -skewMax;
		}
		m_byAccelSkewDur = 0;
	}
	else
	{
		m_chAccelSkew = 0;
		m_byAccelSkewDur = 255;
	}
}

void DaVessel::PowerPlantSkewWander()
{
	if(EnginesRequireEngineer())
	{
		const int skewMax = 16 - 4 * m_FuelQuality/255;	// Skew wander is restricted when fuel quality is better.
		char chWander = (char)g_Au.Dice().Roll(1,5,-3);		// -2, -1, 0, 1 or 2
		if(m_chPowerPlantSkew > skewMax)
		{
			// Current skew is greater than max.
			// Only allow negative wander in this zone.
			if(chWander < 0)
			{
				m_chPowerPlantSkew += chWander;
				if(m_chPowerPlantSkew < -skewMax)
					m_chPowerPlantSkew = -skewMax;
			}
		}
		else if(m_chPowerPlantSkew < -skewMax)
		{
			// Current skew is less than min.
			// Only allow positive wander in this zone.
			if(chWander > 0)
			{
				m_chPowerPlantSkew += chWander;
				if(m_chPowerPlantSkew > skewMax)
					m_chPowerPlantSkew = skewMax;
			}
		}
		else
		{
			// Positive or negative wander is allowed.
			m_chPowerPlantSkew += chWander;
			if(m_chPowerPlantSkew > skewMax)
				m_chPowerPlantSkew = skewMax;
			else if(m_chPowerPlantSkew < -skewMax)
				m_chPowerPlantSkew = -skewMax;
		}
		m_byPowerPlantSkewDur = 0;
	}
	else
	{
		m_chPowerPlantSkew = 0;
		m_byPowerPlantSkewDur = 255;
	}
}

void DaVessel::TurnRateSkewWander()
{
	if(EnginesRequireEngineer())
	{
		const int skewMax = 28 - 7 * m_FuelQuality/255;	// Skew wander is restricted when fuel quality is better.
		// -2, -1, 0, 1 or 2
		char chWanderX = (char)g_Au.Dice().Roll(1,5,-3);	
		// -2, -1, 0, 1 or 2
		char chWanderY = (char)g_Au.Dice().Roll(1,5,-3);	
		if(m_chTurnRateSkewX > skewMax)
		{
			// Current skew is greater than max.
			// Only allow negative wander in this zone.
			if(chWanderX < 0)
			{
				m_chTurnRateSkewX += chWanderX;
				if(m_chTurnRateSkewX < -skewMax)
					m_chTurnRateSkewX = -skewMax;
			}
		}
		else if(m_chTurnRateSkewX < -skewMax)
		{
			// Current skew is less than min.
			// Only allow positive wander in this zone.
			if(chWanderX > 0)
			{
				m_chTurnRateSkewX += chWanderX;
				if(m_chTurnRateSkewX > skewMax)
					m_chTurnRateSkewX = skewMax;
			}
		}
		else
		{
			// Positive or negative wander is allowed.
			m_chTurnRateSkewX += chWanderX;
			if(m_chTurnRateSkewX > skewMax)
				m_chTurnRateSkewX = skewMax;
			else if(m_chTurnRateSkewX < -skewMax)
				m_chTurnRateSkewX = -skewMax;
		}
		if(m_chTurnRateSkewY > skewMax)
		{
			// Current skew is greater than max.
			// Only allow negative wander in this zone.
			if(chWanderY < 0)
			{
				m_chTurnRateSkewY += chWanderY;
				if(m_chTurnRateSkewY < -skewMax)
					m_chTurnRateSkewY = -skewMax;
			}
		}
		else if(m_chTurnRateSkewY < -skewMax)
		{
			// Current skew is less than min.
			// Only allow positive wander in this zone.
			if(chWanderY > 0)
			{
				m_chTurnRateSkewY += chWanderY;
				if(m_chTurnRateSkewY > skewMax)
					m_chTurnRateSkewY = skewMax;
			}
		}
		else
		{
			// Positive or negative wander is allowed.
			m_chTurnRateSkewY += chWanderY;
			if(m_chTurnRateSkewY > skewMax)
				m_chTurnRateSkewY = skewMax;
			else if(m_chTurnRateSkewY < -skewMax)
				m_chTurnRateSkewY = -skewMax;
		}
		m_byTurnRateSkewDur = 0;
	}
	else
	{
		m_chTurnRateSkewX = 0;
		m_chTurnRateSkewY = 0;
		m_byTurnRateSkewDur = 255;
	}
}

// Returns true if 'targetShip' is being towed by my tractor beam.
// 'tractorRange' is set to the maximum range of the tractor that is towing the target ship.
// 'tractorAccel' is set to the maximum acceleration of the tractor that is towing the target ship.
bool DaVessel::IsTowing(const AuId& targetShip, float targetMass, double& tractorRange, double& tractorAccel) const
{
	bool v = false;
	m_BayLock.lockForRead();
	for(unordered_map<quint16, AuBayFired>::const_iterator lfi = m_BayFired.cbegin(); lfi != m_BayFired.cend(); lfi++)
		if((*lfi).second.IsTractorAttached() && (*lfi).second.m_Target == targetShip)
		{
			tractorRange = TractorBeamRange((*lfi).second.m_UnitsFired, (*lfi).first);
			const float tlBoost = QualityBoostF(SystemQuality(SYS_Weapon, (*lfi).first));
			tractorAccel = ((float)(*lfi).second.m_UnitsFired * GravityDriveForce()) / targetMass * tlBoost; 
			v = true;
			break;
		}
	m_BayLock.unlock();
	return v;
}

bool DaVessel::IsVisibleNearWorld() const
{
	return true;
}

// Sets state values to cause this ship to roll, pitch and yaw by the angles specified.
// Used by the AI to maneuver the ship.
void DaVessel::RollPitchYaw(float rollAngle, float pitchAngle, float yawAngle)
{
	m_ControlLock.lockForWrite();
	bool wasTicking = IsTicking();
	m_Roll = rollAngle;
	m_Pitch = pitchAngle;
	m_Yaw = yawAngle;
	bool isTicker = m_Roll != 0 || m_Pitch != 0 || m_Yaw != 0;
	if(isTicker)
		m_wActions |= ACT_Run;			// Indicate turning by AI.
	else
		m_wActions &= ~ACT_Run;			// Indicate not turning by AI.
	m_tmLastRPYControl.restart();
	m_ControlLock.unlock();
	if(!wasTicking && isTicker)
		TickTock();						// Was not ticking before. Start ticking asap.
}

int DaVessel::FireProbability() const
{
	quint32 maxHits = DesignHitsStructural();
	if(!maxHits)
		return 0;
	quint32 dmgHits = HitsDamageStructural();
	switch(SystemModule(SYS_Hull))
	{
	case CID_WoodHullModule:
		return 16 + 84 * dmgHits / maxHits;	// 16 to 100% probability as damage increases from 0 to 100%.
	case CID_LumberHullModule:
		return 8 + 72 * dmgHits / maxHits;	// 8 to 90% probability as damage increases from 0 to 100%.
	case CID_StoneHullModule:
		return 4 + 76 * dmgHits / maxHits;	// 4 to 80% probability as damage increases from 0 to 100%.
	case CID_MetalHullModule:
		return 2 + 68 * dmgHits / maxHits;	// 2 to 70% probability as damage increases from 0 to 100%.
	case CID_MagmiumHullModule:
		return 1 + 59 * dmgHits / maxHits;	// 1 to 60% probability as damage increases from 0 to 100%.
	case CID_VulciumHullModule:
		return 50 * dmgHits / maxHits;		// 0 to 50% probability as damage increases from 0 to 100%.
	case CID_AdamantineHullModule:
		return 40 * dmgHits / maxHits;		// 0 to 40% probability as damage increases from 0 to 100%.
	default:
		return 0;
	}
}

quint64 DaVessel::MemoryUsage() const
{
	quint64 mem = DaCapsule::MemoryUsage();
	mem += sizeof(DaVessel) - sizeof(DaCapsule);
	mem += StringMemoryUsage(m_WhereMade);
	mem += (sizeof(quint16) + sizeof(AuBayFired)) * m_BayFired.size();
	mem += (sizeof(quint16) + sizeof(AuBaySetting)) * m_BaySetting.size();
	mem += m_Cargo.MemoryUsage();
	mem += m_Damage.MemoryUsage();
	mem += (sizeof(quint16) + sizeof(AuGame*)) * m_Games.size();
	mem += StringMemoryUsage(m_NameWithEmpire);
	mem += StringMemoryUsage(m_OwnerName);
	mem += m_Roster.MemoryUsage();
	mem += m_ssaShipSystems.MemoryUsage();
	mem += (sizeof(quint16) + sizeof(AuOrdnance)) * m_Turrets.size();
	mem += sizeof(DeWaypoint) * m_aWaypoints.capacity();
	if(m_TractorBeam)
		mem += sizeof(AuTractorBeam);
	return mem;
}

void DaVessel::BuildingSurrendered(quint32 surrenderingEmpire, quint32 toEmpire)
{
	Q_UNUSED(surrenderingEmpire);
	if(!IsNew())
		return;
	SetEmpireId(toEmpire);
}

void DaVessel::WorldCaptured(quint32 byEmpire)
{
	if(!IsNew())
		return;
	if(EmpireId() == byEmpire)
		return;
	SetEmpireId(byEmpire);
	SetModified();
	DaCapsule::WorldCaptured(byEmpire);	// Notify my subatoms.
}

double DaVessel::FallSafeSpeed() const
{
	return 30 * FeetPerMeter();
}

float DaVessel::RelativeAudioGain(const DaPerson* sceneAtom) const
{
	if(sceneAtom->IsInSpacecraft() && sceneAtom->FindParentVesselInEnv() == this)
	{
		if(sceneAtom->HelmetClosed())
			return .5f;
		return 1;
	}
	return DaCapsule::RelativeAudioGain(sceneAtom);
}

// Find atoms that might cause shields to be raised.
void DaVessel::FindClosestShipThreatRecursive(const AuId& shipId, int shipDomain, const AuId& shipWorldId, quint32 shipEmpire,
		SHIELD_POLICY shieldPolicy, const AuPoint& shipSysLoc, float sensorRange, const DaAtom*& targetAtom,
		double& targetRange) const
{
	if(IsNew())
		return;
	if(Is(shipId))
		return;
	if(IsHidingInGroundClutter())
	{
		// Check for ship hiding in ground clutter.
		double range2 = shipSysLoc.Distance2(EnvLocation());
		if(range2 > HideInGroundClutterRange2())
			return;		// Out of sight range and hiding.
	}
	DaUnit::FindClosestShipThreatRecursive(shipId, shipDomain, shipWorldId, shipEmpire, shieldPolicy, shipSysLoc, sensorRange,
			targetAtom, targetRange);
}

void DaVessel::EmpireDeleted(quint32 empireId)
{
	if(EmpireId() != empireId)
		return;
	// The deleted empire has no avatars or cities.
	// Officers, crew, troops and passengers of the deleted empire Kill themselves.
	// This ship will change to a pirate empire.
	const DgData& gd = g_Au.LockDataForRead();
	quint32 nuEmpire = gd.RandomPirateEmpire(Galaxy());
	g_Au.UnlockData();
	SetEmpireId(nuEmpire);
}

DaVessel* DaVessel::FindParentVesselInEnv()
{
	return this;
}

const DaVessel* DaVessel::FindParentVesselInEnv() const
{
	return this;
}

void DaVessel::ReportPosition()
{
	if(InArena())
		return;		// Do not report ships in an arena.
	if(InDesignEnvironment())
		return;		// Do not report ships in the design studio.
	quint32 empireId = EmpireId();
	if(!empireId)
		return;		// Empire-less ships do not report their position.
	const DaSolarSystem* system = FindParentSolarSystem();
	if(!system)
		return;
	quint32 maxHits = DesignHitsStructural();
	if(!maxHits)
		return;
	quint32 dmgHits = HitsDamageStructural();
	double health = 1. - dmgHits / maxHits;	// 0=destroyed, 1=full health
	health *= 255;							// 0 to 255
	if(health < 0)
		health = 0;
	else if(health > 255)
		health = 255;

	AuStarMapSpacecraft pos(Name(), empireId);
	if(IsInSpacecraft())
	{
		const DaAtom* parent = Parent();
		const DaVessel* v = parent ? parent->FindParentVesselInEnv() : 0;
		if(v)
			pos.SetAboardSpacecraft(v->Name());
	}
	else if(IsInBd())
	{
		const DaBd* b = FindParentBdInEnv();
		if(b)
			pos.SetAtBuilding(b->CityOrBuildingName());
	}

	pos.SetHealth((unsigned int)round(health));
	if(IsSpaceStation())
		pos.SetSpaceStation();
	if(IsInAtmosphere())
		pos.SetInAtmosphere();
	if(IsNearPlanet())
		pos.SetOrbiting();
	if(IsMoving())
		pos.SetMoving();
	if(FindParentStarInEnv())
		pos.SetInSystem();
	if(IsNew())
		pos.SetNew();
	if(!CaptainId().IsNull())
		pos.SetCaptain();
	if(!AIFirstOfficer().IsNull())
		pos.SetOfficer();
	pos.SetAlertCondition(AlertCondition());
	if(HasWarpFTLDrive())
	{
		pos.SetWarpDrive();
		if(IsFTLDriveHot(FTLWarpDrive()))
			pos.SetWarpFTLHot();
	}
	if(HasWormholeFTLDrive())
	{
		pos.SetWormholeDrive();
		if(IsFTLDriveHot(FTLWormholeDrive()))
			pos.SetWormholeFTLHot();
	}
	if(HasSensors())
		pos.SetHasSensors();
	if(IsPrivatelyOwned())
		pos.SetCompany();

	emit SpacecraftPositionReport(Id(), system->Id(), pos);
}

void DaVessel::DetectWarpSignatureRecursive(const AuId& systemId, quint32 warpFactors, const AuPoint& galaxyLoc)
{
	// Ignore emissions of I have no sensors.
	if(!HasSensors())
		return;													// This building has no sensors.

	// Spacecraft without warp drive cannot detect warp signatures.
	if(!HasWarpFTLDrive())
		return;

	const DaGalaxy* galaxy = FindParentGalaxy();
	if(!galaxy)
		return;

	// Calculate vector to the location that emitted the signal.
	// I only receive signals emitted from within my own galaxy.
	AuPoint myGalaxyLoc;
	GetLocationRelativeToParent(myGalaxyLoc, galaxy);
	AuPoint vecToSystem = galaxyLoc - myGalaxyLoc;				// Vector to system, in parsecs.

	// Crossing zones dampens the signal.
	quint32 warpSignal = warpFactors * 10;	// Lets work with 10 units of signal for every warp factor engaged.
	int myZone;
	if(myGalaxyLoc.z > 10)
		myZone = 1;												// I am in the upper veil.
	else if(myGalaxyLoc.z < 0)
		myZone = -1;											// I am in the lower veil.
	else
		myZone = 0;												// I am in the neutral zone.
	int signalZone;
	if(galaxyLoc.z > 10)
		signalZone = 1;											// Signal emanated from the upper veil.
	else if(galaxyLoc.z < 0)
		signalZone = -1;										// Signal emanated from the lower veil.
	else
		signalZone = 0;											// Signal emanated from the neutral zone.
	int relZone = abs(myZone - signalZone);						// 0, 1 or 2
	if(relZone)
	{
		warpSignal /= (relZone * 2);							// 1/2 signal lost per zone
		if(!warpSignal)
			return;
	}

	// Calculate emitter detection range based on warp signal and the capability of my sensors.
	// A spaceport in an empire with warp technology will engage a warp drive at factor 1 once per hour.
	// A system emits its warp signature signal every hour.
	// At 10pc per warp factor, a drive engaged at warp 9 can be detected 90pc away, or 9 sectors.
	quint32 emissionRange = warpSignal;							// Range the emission can be detected in pc.
	quint32 sensorRange = SensorRangeDeepSpace();				// Range my sensors can detect warp signatures, in parsecs.
	quint32 effectiveRange;
	if(emissionRange < sensorRange)
		effectiveRange = emissionRange;							// Effective range is limited by the emitter's weak signal.
	else
		effectiveRange = sensorRange;							// Effective range is limited by my sensor range.

	//qDebug() << "DaVessel::DetectCityRecursive" << Id() << Name() << "warpSignal" << warpSignal << "emitRange"
		//<< cityEmissionRange << "sensorRange" << sensorRange << "effRange" << effectiveRange << "vecToSystem" << vecToSystem;

	// Quickly rule out cities that are way too far away.
	if(fabs(vecToSystem.x) > effectiveRange || fabs(vecToSystem.y) > effectiveRange || fabs(vecToSystem.z) > effectiveRange)
		return;							// Way out of range.

	// Do some math.
	double dist2 = vecToSystem.Length2();
	if(dist2 > effectiveRange*effectiveRange)
		return;							// Out of range.

	// The emitter is in range to detect.
	//qDebug() << "DaVessel::DetectCityRecursive" << Id() << Name() << "power use detected, system" << citySystemId << "my empire"
		//<< EmpireId(0);
	emit WarpSignatureDetected(systemId, warpSignal, EmpireId());
}

float DaVessel::SensorRangeDeepSpace() const
{
	return AuDesign::SensorRangeDeepSpace(SensorRange());
}

void DaVessel::SetSystemQuality(const AuSysId& sysId, quint8 quality)
{
	// Set module quality of system.
	m_SystemsLock.lockForWrite();
	if(m_ssaShipSystems.SetSystemQuality(sysId, quality))
		m_tmSystemsUpg.Set();
	m_SystemsLock.unlock();
}

void DaVessel::DowngradeSystemModules(quint8 repairQ, const QSet<AuSysId>& systemsRepaired)
{
	if(systemsRepaired.empty())
		return;

	// Reduce module quality of systems that were repaired using lower Q parts.
	bool anyDowngraded = false;
	m_SystemsLock.lockForWrite();
	for(QSet<AuSysId>::const_iterator ri = systemsRepaired.cbegin(); ri != systemsRepaired.cend(); ri++)
	{
		if(m_ssaShipSystems.DowngradeSystemQuality(*ri, repairQ))
			anyDowngraded = true;
	}
	if(anyDowngraded)
		m_tmSystemsUpg.Set();
	m_SystemsLock.unlock();
}

bool DaVessel::SystemWhiches(SYSTEM system, QSet<quint16>& whiches) const
{
	m_SystemsLock.lockForRead();
	bool v = m_ssaShipSystems.SystemWhiches(system, whiches);
	m_SystemsLock.unlock();
	return v;
}

void DaVessel::FillShipments(bool shipNow)
{
	// Only stations fill broker shipments.
	//if(!IsSpaceStation())
		//return;
	const DaSolarSystem* system = FindParentSolarSystem();
	if(!system)
		return;

	// Don't lock for write if there's nothing to do.
	bool anythingToDo = false;
	m_CargoLock.lockForRead();
	anythingToDo = m_Shipments.AnyFilling() || m_Shipments.AnyReadyToDeliver();
	if(!anythingToDo && shipNow)
		anythingToDo = m_Shipments.AnyReadyToShip();
	m_CargoLock.unlock();
	if(!anythingToDo)
		return;

	//AuItemHash inventory;
	//BrokerInventoryAvailable(inventory, EmpireId());
	//if(inventory.empty())
		//return;

	QSet<AuId> cityIds;
	if(shipNow)
	{
		// This happens only once per hour. Get a list of all cities so we can remove shipments to nonexistent cities.
		const DgData& gd = g_Au.LockDataForRead();
		gd.CityIds(cityIds);
		g_Au.UnlockData();
	}

	// We have work to do so lock for write.
	bool didAnything = false;
	AuShipmentList deliveries;
	AuTradeAgreementList tradesReady;	// Vessels do not enter into trade agreements.
	m_CargoLock.lockForWrite();

	// Deliver shipments that have arrived at their destinations by now.
	if(m_Shipments.Deliver(deliveries, tradesReady))
		didAnything = true;

	// Ship stuff that's ready to go before accumulating materials, just to keep things slow. Then we don't accumulate and ship
	// the same order in one cycle.
	// This duplicates shipments that are awaiting shipment and makes enroute shipments out of them, broker levels permitting.
	int brokerLevels = 2;
	if(shipNow && m_Shipments.Ship(AuShipmentBroker(brokerLevels), cityIds))
		didAnything = true;

	// Accumulate materials into shipments that are being filled.
	// This fills shipments that are filling and changes them to awaiting shipment when they get full.
	if(m_Shipments.Fill(m_Cargo))
		didAnything = true;

	m_CargoLock.unlock();

	// Emit deliveries.
	// This sends the actual container of stuff to the destination city.
	if(!deliveries.empty())
	{
		bool sectorHasPirateBase;
		quint32 systemBaseEmpire;
		bool systemHasPirateBase = system->HasPirateBase(systemBaseEmpire);
		if(systemHasPirateBase)
			sectorHasPirateBase = true;
		else
		{
			const DaSector* sector = system->FindParentSector();
			sectorHasPirateBase = sector && sector->HasPirateBase();
		}

		// This sends the actual container of stuff to the destination city.
		for(AuShipmentList::const_iterator di = deliveries.cbegin(); di != deliveries.cend(); di++)
		{
			if(systemHasPirateBase)
			{
				// Most shipments get plundered in the system with a pirate base.
				if(g_Au.Dice().Roll(1,100) <= 80)
				{
					// Shipment interdicted by pirates in the system.
					QString what;
					if((*di).CommodityId() != CID_Invalid)
						what = g_Au.CommodityName((*di).CommodityId());
					else
						what = CommodityFilterName((*di).CommodityFilter());
					LogEvent(AuEventLogEntry::ShipmentOutPlunderedSystem, what);
					continue;
				}
			}
			else if(sectorHasPirateBase)
			{
				// Some shipments get plundered in the system with a pirate base.
				if(g_Au.Dice().Roll(1,100) <= 20)
				{
					// Shipment interdicted by pirates in the sector.
					QString what;
					if((*di).CommodityId() != CID_Invalid)
						what = g_Au.CommodityName((*di).CommodityId());
					else
						what = CommodityFilterName((*di).CommodityFilter());
					LogEvent(AuEventLogEntry::ShipmentOutPlunderedSector, what);
					continue;
				}
			}
			// If we get here, send the shipment.
			emit ShipIt((*di).Container());
		}
	}

	if(didAnything)
		SetModified();
}

int DaVessel::ShipmentCreate(const AuShipment& shipment)
{
	// Only stations fill broker shipments.
	if(!IsSpaceStation())
		return 1;
	//if(FindParentSolarSystem()->Is(shipment.Container().DestinationSystem()))
		//return 6;

	// This was copied from DaBd.
	int brokerLevels = 2;

	// Check limit on existing shipments.
	m_CargoLock.lockForWrite();
	int shipments = m_Shipments.CountOffices();
	if(shipments >= brokerLevels)
	{
		m_CargoLock.unlock();
		return 4;
	}

	// Add the new shipment.
	bool added = m_Shipments.AddShipment(shipment);
	m_CargoLock.unlock();
	if(!added)
		return 5;

	SetModified();
	return 0;
}

int DaVessel::ShipmentDelete(const AuShipment& shipment)
{
	// Only stations fill broker shipments.
	if(!IsSpaceStation())
		return 1;

	// Delete the shipment.
	m_CargoLock.lockForWrite();
	bool deleted = m_Shipments.Remove(shipment);
	m_CargoLock.unlock();
	if(!deleted)
		return 4;

	SetModified();
	return 0;
}

bool DaVessel::ShieldsWorkingAndOn() const
{
	// Must be not be OFF.
	if(ShieldState() == SS_Off)
		return false;

	// Capacitor must have some power for system to work.
	if(CapacitorLevel() <= 0)
		return false;
	
	if(!IsDesignDataCacheValid())
		return true;	// Give the benefit of the doubt until the design is loaded.

	// 100% damage to shields will make them stop working.
	if(!GetPowerConsumption(SYS_Shield))
		return false;

	// Everything looks good.
	return true;
}

bool DaVessel::ShieldsWorkingAndUp() const
{
	// Must be UP.
	if(ShieldState() != SS_Up)
		return false;

	// Capacitor must have some power for system to work.
	if(CapacitorLevel() <= 0)
		return false;
	
	if(!IsDesignDataCacheValid())
		return true;	// Give the benefit of the doubt until the design is loaded.

	// 100% damage to shields will make them stop working.
	if(!GetPowerConsumption(SYS_Shield))
		return false;

	// Everything looks good.
	return true;
}

void DaVessel::FindVesselsRecursive(DaAtomList& spaceships) const
{
	//if(!IsNormal())
		//return;
	DaCapsule::FindVesselsRecursive(spaceships);
	spaceships.append((DaVessel*)this);	// Cast away constness.
}

void DaVessel::FindPirateStationsRecursive(DaAtomList& stations) const
{
	//if(!IsNormal())
		//return;
	DaCapsule::FindPirateStationsRecursive(stations);
	if(IsSpaceStation() && IsPirateCommander())
		stations.append((DaVessel*)this);	// Cast away constness.
}

const DaVessel* DaVessel::FindParentVessel() const
{
	return this;
}

DaVessel* DaVessel::FindParentVessel()
{
	return this;
}

TRANSPORTER_TARGET DaVessel::TransporterTarget() const
{
	if(ShieldsBlockTransportAboard())
		return TT_Invalid;
	switch(Structure())
	{
	case SC_Count:
	case SC_Deprecated:
		break;
	case SC_Hull:
	case SC_Spaceship:
		return TT_Spaceship;
	case SC_Starship:
		return TT_Starship;
	case SC_Station:
		return TT_SpaceStation;
	}
	return TT_Invalid;
}

void DaVessel::ClaimTitleRecursive(const AuId& propertyId, const QString& propertyName, const AuId& ownerId,
		const QString& ownerName, quint32 ownerEmpire)
{
	DaCapsule::ClaimTitleRecursive(propertyId, propertyName, ownerId, ownerName, ownerEmpire);
	if(IsDead())
		return;

	bool anyChange = false;
	m_CargoLock.lockForWrite();
	if(m_Cargo.VoidOwnershipTitle(propertyId, ownerId, ownerName))
		anyChange = true;
	if(anyChange)
		m_tmCargoChange.Set();
	m_CargoLock.unlock();
	if(anyChange)
		SetModified();

	if(Is(propertyId))
	{
		if(!IsPrivatelyOwned())
		{
			qWarning() << "warning: DaVessel::ClaimTitleRecursive" << *this << "property" << propertyId << propertyName
				<< "owner" << ownerId << ownerName << ownerEmpire << "ship is a fleet vessel";
			return;
		}

		if(!OwnerId().IsNull())
		{
			// Notify the previous owner.
			QString text = tr("Ownership of %3 %4 was claimed by %1 %2 using a title.")
				.arg(ownerName)
				.arg(ownerId.StringId())
				.arg(propertyName)
				.arg(propertyId.StringId());
			AuCommChat msg(text, AuCommStdChannel::CompanyChannelName(), Id(), Name(), FindParentSector()->Id(),
					Galaxy(), AuCommStdChannel::Universe, AuCommStdChannel::Company);
			msg.SetScopeData(OwnerId().GetID());	// Company channel needs owner's id to know the scope of the message.
			g_Au.SendAtomChat(msg);
		}

		SetPrivateOwner(ownerId, ownerName);
		SetEmpireId(ownerEmpire);
		MonitorCommChannels();

		// Announce this change by hailing on the company channel.
		QString text = tr("Hail to owner %1 %2. %3 %4 is yours, to do with as you will, sir.")
			.arg(ownerName)
			.arg(ownerId.StringId())
			.arg(NameWithEmpire())
			.arg(Id().StringId());
		AuCommChat msg(text, AuCommStdChannel::CompanyChannelName(), Id(), Name(), FindParentSector()->Id(),
				Galaxy(), AuCommStdChannel::Universe, AuCommStdChannel::Company);
		msg.SetScopeData(ownerId.GetID());	// Company channel needs owner's id to know the scope of the message.
		g_Au.SendAtomChat(msg);

		// Send a little information about where we are.
		AnswerHail(AuCommStdChannel::CompanyChannelName(), AuCommStdChannel::Universe, AuCommStdChannel::Company, false, 0);
	}
}

void DaVessel::RxOpenTEU(const AuCommOpenTEU* msg, AuCommStdChannel::RANGE limitedRange, const AuId& sectorId,
		DaPerson* person)
{
	if(!msg)
		return;

	// Open TEU order is only issued on the intercom.
	if(msg->Scope() != AuCommStdChannel::Intercom)
		return;

	// Check credentials of sender to insure I'm in their chain of command. Simply ignore the message if the person does
	// not have the authority to order me around.
	if(!person || !InChainOfCommand(person))
		return;	// This ship is not in the sender's chain-of-command.

	// Determine what is being opened.
	const AuItemTEUContainer* teuItem = dynamic_cast<const AuItemTEUContainer*>(msg->Item());
	if(!teuItem)
		return;

	// Remove the teu container from my hold.
	if(!RemoveCargo(*teuItem, 1))
		return;		// The cargo was not there?

	// Crack open the container.
	const AuItemHash* cargo = teuItem->Cargo();
	if(!cargo)
		return;

	// Calculate the value of the cargo and put a bounty on the shipper's head and the opener's head.
	if(teuItem->IsInsured())
	{
		double bounty = cargo->Value();
		if(bounty > s_MinBounty)
		{
			if(person->Is(teuItem->CarrierId()))
			{
				// The opening person is the carrier responsible for the shipment.
				QString reason(tr("Opened an insured shipment entrusted to their care."));
				person->AddBounty(AuId::InsurerId(), InsurerName(), reason, bounty, 4);
			}
			else
			{
				// The opening person is not the carrier responsible for the shipment.
				// Split the bounty 50:50.
				bounty /= 2;
				QString reason(tr("Opened an insured shipment."));
				person->AddBounty(AuId::InsurerId(), InsurerName(), reason, bounty, 5);

				reason = tr("Allowed an insured shipment to be opened.");
				emit BountyPosted(AuId::InsurerId(), InsurerName(), teuItem->CarrierId(), reason, bounty, 6);
			}
		}
	}

	// Put the individual cargo into my hold.
	int itemsLost = 0;	// true if any of the items cannot be added to my hold. This is extremely unlikely (I can't think of a
						// case) so I won't bother providing details.
	int itemsAdded = 0;
	for(AuItemHash::const_iterator ci = cargo->cbegin(); ci != cargo->cend(); ci++)
	{
		if(CanAddCargo(*(*ci).second))
		{
			AddCargo(*(*ci).second);
			itemsAdded++;
		}
		else
			itemsLost++;
	}

	int teus = teuItem->TEUCount();

	// Notify the person of the result.
	QString resultMsg;
	if(teus == 1)
	{
		if(!itemsAdded && !itemsLost)
			resultMsg = tr("The TEU was empty; it was jettisoned.");
		else if(!itemsLost)
			resultMsg = tr("%L1 packages were removed from the TEU to the hold. "
					"The empty container was jettisoned.")
				.arg(itemsAdded);
		else
			resultMsg = tr("%L1 packages were removed from the TEU. "
					"%L2 packages could not go into the hold so they were jettisoned with the container.")
				.arg(itemsAdded)
				.arg(itemsLost);
	}
	else
	{
		if(!itemsAdded && !itemsLost)
			resultMsg = tr("The TEUs were empty; they were jettisoned.");
		else if(!itemsLost)
			resultMsg = tr("%L1 packages were removed from the TEUs to the hold. "
					"%L2 empty containers were jettisoned.")
				.arg(itemsAdded)
				.arg(teus);
		else
			resultMsg = tr("%L1 packages were removed from the TEUs. "
					"%L2 packages could not go into the hold so they were jettisoned with the containers.")
				.arg(itemsAdded)
				.arg(itemsLost);
	}

	AuCommChat mymsg(resultMsg, msg->ChannelName(), Id(), Name(), sectorId, Galaxy(), limitedRange,
			msg->Scope());
	g_Au.SendAtomChat(mymsg);
}

void DaVessel::ShipmentDefaulted(const AuItemTEUContainer& container)
{
	bool anyChange = false;
	m_CargoLock.lockForWrite();
	if(m_Cargo.ShipmentDefaulted(container))
		anyChange = true;
	if(anyChange)
		m_tmCargoChange.Set();
	m_CargoLock.unlock();
	SetModified();
}

quint32 DaVessel::ExperiencePointsForKilling() const
{
	quint32 xp = DesignHitsTotal()/1000;
	if(!xp)
		return 1;
	return xp;
}

int DaVessel::CanSpawnHere(const AuId& id, quint32 empireId) const
{
	if(!IsNormal())
		return 4;

	// Does the person still occupy a berth?
	AuRosterEntry::TYPE berthType = BerthTypeOccupied(id);
	if(berthType == AuRosterEntry::Invalid)
		return 3;	// They do not have a berth aboard this ship.

	// They can respawn here unless they are enemy, passengers excluded. Enemy can recall to private ships.
	if(!IsPrivatelyOwned() && !AuRosterEntry::IsPassenger(berthType) && PoliticalStance(EmpireId(), empireId) == POL_Enemy)
		return 2;	// They are enemy non passenger.

	// Is the life support working
	if(!LifeSupportWorking())
		return 1;	// Life support is not working.

	// Spawn here is ok.
	return 0;
}

bool DaVessel::IsDamaged() const
{
	m_DamageLock.lockForRead();
	bool v = m_Damage.IsDamaged();
	m_DamageLock.unlock();
	return v;
}

float DaVessel::StructuralIntegrity() const
{
	return (float)HitsDamageStructural() / (float)DesignHitsStructural();
}

QString DaVessel::WorstDamagedSystem() const
{
	if(!IsDesignDataCacheValid())
		return "";
	if(!IsInsideAirBreathable())
		return QObject::tr("Air Pressure");

	// Get a list of the damaged systems.
	AuDamage dmg;
	DamageReportPercent(dmg, 6);

	// Determine most damaged system, based on percentage of damage.
	quint32 mostDmg = 0;
	AuSysId mostDmgId = dmg.MostDamagedSystem(mostDmg);
	return mostDmgId.ScName();
}

unsigned int DaVessel::WorstSystemDamage() const
{
	if(!IsDesignDataCacheValid())
		return 0;
	if(!IsInsideAirBreathable())
		return 100;

	// Get a list of the damaged systems.
	AuDamage dmg;
	DamageReportPercent(dmg, 6);

	// Determine most damaged system, based on percentage of damage.
	quint32 mostDmg = 0;
	dmg.MostDamagedSystem(mostDmg);
	return mostDmg;
}

int DaVessel::SystemModuleInstallTime(const AuSysId& sysId, COMMODITYID cid) const
{
	if(sysId.System() == SYS_Hull)
	{
		// Hull is upgraded using units of metal, magmium, etc. instead of modules.
		if(cid == CID_Adamantine ||
				cid == CID_Magmium ||
				cid == CID_Metal ||
				cid == CID_Vulcium)
			return 1;
		return 0;
	}

	if(ScModuleSystem(cid) != sysId.System())
		return 0;	// Module is not an upgrade for system.
	if(sysId.System() == SYS_FTLDrive)
	{
		if(DbCommodity::IsWormholeFTLDriveModule(cid))
		{
			// Do not install a wormhole drive if I already have a wormhole drive on a different 'which'.
			quint16 wormWhich = FTLWormholeDrive();
			if(wormWhich && wormWhich != sysId.Which())
				return -1;							// Wrong system for the wormhole drive module.
		}
		else if(DbCommodity::IsWarpFTLDriveModule(cid))
		{
			// Do not install a warp drive if I already have a warp drive on a different 'which'.
			quint16 warpWhich = FTLWarpDrive();
			if(warpWhich && warpWhich != sysId.Which())
				return -1;							// Wrong system for the warp drive module.
		}
	}
	return 1;
}

quint32 DaVessel::SystemComponents(const AuSysId& sysId, COMMODITYID cid) const
{
	// This list of cids is the same as the list in DbCommodity::IsSparePart.
	switch(cid)
	{
	case CID_Adamantine:
	case CID_Magmium:
	case CID_Metal:
	case CID_Vulcium:
		if(sysId.IsStructural())
			return SystemModuleCount(sysId);
		break;
	case CID_AtmosphereCondenserBayModule:
	case CID_BallisticWeaponBayModule:
	case CID_EnergyWeaponBayModule:
	case CID_ParticleCollectorBayModule:
	case CID_ProspectorBayModule:
	case CID_SiphonBayModule:
	case CID_TractorBeamBayModule:
	case CID_WeaponDeploymentBayModule:
	case CID_WeaponLauncherBayModule:
	case CID_WildcatterBayModule:
		if(sysId.System() == SYS_Weapon)
			return SystemModuleCount(sysId);
		break;
	case CID_BallisticWeaponTurretModule:
	case CID_EnergyWeaponTurretModule:
	case CID_WeaponLauncherTurretModule:
		if(sysId.System() == SYS_Turret)
			return SystemModuleCount(sysId);
		break;
	case CID_CapacitorModule:
		if(sysId.System() == SYS_Capacitor)
			return SystemModuleCount(sysId);
		break;
	case CID_CloakingShieldModule:
	case CID_EnergyShieldModule:
	case CID_KineticShieldModule:
		if(sysId.System() == SYS_Shield)
			return SystemModuleCount(sysId);
		break;
	case CID_CommandAndControlModule:
		if(sysId.System() == SYS_CommandAndControl)
			return SystemModuleCount(sysId);
		break;
	case CID_CyberneticsMedicalModule:
	case CID_GeneticsMedicalModule:
	case CID_SurgeryMedicalModule:
		if(sysId.System() == SYS_SurgeryUnit)
			return SystemModuleCount(sysId);
		break;
	case CID_GravityDriveModule:
	case CID_RocketDriveModule:
		if(sysId.System() == SYS_ManeuverDrive)
			return SystemModuleCount(sysId);
		break;
	case CID_HydrogenPowerPlantModule:
		if(sysId.System() == SYS_PowerPlant)
			return SystemModuleCount(sysId);
		break;
	case CID_LifeSupportGravityModule:
	case CID_LifeSupportModule:
		if(sysId.System() == SYS_LifeSupport)
			return SystemModuleCount(sysId);
		break;
	case CID_PersonnelTransporterModule:
	case CID_VehicleTransporterModule:
		if(sysId.System() == SYS_Transporter)
			return SystemModuleCount(sysId);
		break;
	case CID_SensorModule:
		if(sysId.System() == SYS_Sensor)
			return SystemModuleCount(sysId);
		break;
	case CID_Warp1FTLDriveModule:
	case CID_Warp2FTLDriveModule:
	case CID_Warp3FTLDriveModule:
	case CID_Warp4FTLDriveModule:
	case CID_Warp5FTLDriveModule:
	case CID_Warp6FTLDriveModule:
	case CID_Warp7FTLDriveModule:
	case CID_Warp8FTLDriveModule:
	case CID_Warp9FTLDriveModule:
	case CID_WormholePFTLDriveModule:
	case CID_WormholePNFTLDriveModule:
	case CID_WormholePNNFTLDriveModule:
		if(sysId.System() == SYS_FTLDrive)
			return SystemModuleCount(sysId);
		break;
	case CID_ElectronicPart:
		if(sysId.System() == SYS_VehicleLanding)
			return SystemModuleCount(sysId);
		break;
	case CID_MechanicalPart:
		if(sysId.System() == SYS_VehicleLaunch)
			return SystemModuleCount(sysId);
		break;
	case CID_IoTube:
	case CID_Plastic:
		break;
	default:
		break;
	}
	return 0;
}

bool DaVessel::IsConsoleOperational(const AuSysId& sysId) const
{
	return SystemHitsRemaining(sysId) ? true : false;
}

bool DaVessel::IsCivilian() const
{
	return IsPrivatelyOwned();
}

void DaVessel::operator>>(QDataStream& out) const
{
	DaCapsule::operator>>(out);
	out << m_AcceptCrew;
	out << m_AcceptPassengers;
	out << m_AcceptTroops;
	out << (quint8)m_AccessPolicy;
	out << m_AllowPassengerSeaChests;
	out << m_AutoFetchVehicles;
	out << m_bExtLightOn;
	out << m_bPowerPlantOn;
	out << m_bSensorsOn;
	out << m_byAccelSkewDur;
	out << m_byPowerPlantSkewDur;
	out << m_byTurnRateSkewDur;
	out << (qint8)m_chAccelSkew;
	out << (qint8)m_chPowerPlantSkew;
	out << (qint8)m_chTurnRateSkewX;
	out << (qint8)m_chTurnRateSkewY;
	out << m_ConCaptAI;
	out << m_ConEngineerAI;
	out << m_ConEngineerPerson;
	out << m_ConNavigatorAI;
	out << m_ConNavigatorPerson;
	out << m_ConPilotAI;
	out << m_ConPilotPerson;
	out << m_ConPowerRelayAI;
	out << m_ConPowerRelayPerson;
	out << m_ConSensorAI;
	out << m_ConSensorPerson;
	out << m_ConShieldAI;
	out << m_ConShieldPerson;
	out << m_ConWeaponAI;
	out << m_ConWeaponPerson;
	out << m_FuelQuality;
	out << m_InTow;
	out << m_IsHumanLock;
	out << m_IsPrivate;
	out << m_LockedIsCivilian;
	out << m_LockedStance;
	out << m_Moored;
	out << (quint8)m_PilotUpPolicy;
	out << m_PowerPlantShunts;
	out << (quint8)m_ShieldFuelPolicy;
	out << (quint8)m_ShieldPolicy;
	out << (quint8)m_ShieldState;
	out << m_QMade;

	m_VesselLock.lockForRead();
	out << m_CapacitorLevel;
	out << m_CapacitorTrend;
	out << m_ConAICaptId;
	out << (quint8)m_eGravityDriveState;
	out << m_ETE;
	out << m_fFuelQuantity;
	out << m_LockedBlip;
	out << m_LockedSysLoc;
	out << m_Monster;
	out << (quint8)m_MonsterOrigin;
	out << m_RatSpecies;
	out << m_RepairSecs;
	out << m_WhereMade;
	m_VesselLock.unlock();

	m_AccountLock.lockForRead();
	out << m_Bank;
	out << m_BankSubsidyTax;
	out << AuTime();	//m_tmLastBank;
	m_AccountLock.unlock();

	m_BayLock.lockForRead();
	out << (quint16)m_BayFired.size();
	for(unordered_map<quint16, AuBayFired>::const_iterator bi = m_BayFired.cbegin(); bi != m_BayFired.cend(); bi++)
	{
		out << (*bi).first;
		out << (*bi).second;
	}

	out << AuTime();	//m_tmBayFired;

	out << (quint16)m_BaySetting.size();
	for(unordered_map<quint16, AuBaySetting>::const_iterator si = m_BaySetting.cbegin(); si != m_BaySetting.cend(); si++)
	{
		out << (*si).first;
		out << (*si).second;
	}
	m_BayLock.unlock();

	m_CargoLock.lockForRead();
	out << m_Cargo;
	out << m_Shipments;
	out << AuTime();	//m_tmCargoChange;
	m_CargoLock.unlock();

	m_CommLock.lockForRead();
	out << m_ATCAtRest;
	out << m_ATCCity;
	out << m_ATCCityEnemy;
	AuTime aTCContact;
	out << aTCContact;
	out << m_ATCStation;
	out << m_ATCStationEnemy;
	AuTime tmAlert;
	out << tmAlert;
	AuTime tmEnvDamage;
	out << tmEnvDamage;
	AuTime tmSOS;
	out << tmSOS;
	m_CommLock.unlock();

	m_ControlLock.lockForRead();
	out << m_byRotSpeedX;
	out << m_byRotSpeedY;
	out << m_byRotSpeedZ;
	out << m_Pitch;
	out << m_Roll;
	out << m_Throttle;
	out << m_ThrustLateral;
	out << m_ThrustLong;
	out << m_ThrustVertical;
	AuTime tmLastBrake;
	out << tmLastBrake;
	AuTime tmLastBrakeLV;
	out << tmLastBrakeLV;
	AuTime tmLastPitchControl;
	out << tmLastPitchControl;
	AuTime tmLastRollControl;
	out << tmLastRollControl;
	AuTime tmLastRPYControl;
	out << tmLastRPYControl;
	AuTime tmLastThrottleControl;
	out << tmLastThrottleControl;
	AuTime tmLastThrottleFControl;
	out << tmLastThrottleFControl;
	AuTime tmLastThrottleLControl;
	out << tmLastThrottleLControl;
	AuTime tmLastThrottleVControl;
	out << tmLastThrottleVControl;
	AuTime tmLastTurnControl;
	out << tmLastTurnControl;
	out << m_Yaw;
	out << m_wActions;
	m_ControlLock.unlock();

	m_DamageLock.lockForRead();
	out << m_Damage;
	out << m_tmDamageChange;
	m_DamageLock.unlock();

	m_GamesLock.lockForRead();
	out << (quint16)m_Games.size();
	for(unordered_map<quint16, AuGame*>::const_iterator gi = m_Games.cbegin(); gi != m_Games.cend(); gi++)
	{
		out << (*gi).first;
		out << (*gi).second->Type();
		out << *(*gi).second;
	}
	out << AuTime();	//m_GamesChanged;
	m_GamesLock.unlock();

	m_LastInRangeLock.lockForRead();
	QTime lastInEnemyRange;
	out << lastInEnemyRange;
	m_LastInRangeLock.unlock();

	m_MissionLock.lockForRead();
	out << m_Mission;
	out << AuTime();	//m_tmMissionChange;
	m_MissionLock.unlock();

	m_NameWithEmpireLock.lockForRead();
	out << m_NameWithEmpire;
	m_NameWithEmpireLock.unlock();

	m_RosterLock.lockForRead();
	out << m_CryoBerths;
	out << m_OwnerId;
	out << m_OwnerName;
	out << m_Roster;
	out << AuTime();	// Used to be last time an officer was on line.
	out << AuTime();	//m_tmRosterChange;
	m_RosterLock.unlock();

	m_ShieldsLock.lockForRead();
	for(int s=0; s<6; s++)
	{
		out << m_ShieldCap[s];
		out << m_ShieldScram[s];
	}
	m_ShieldsLock.unlock();

	m_SystemsLock.lockForRead();
	out << m_ssaShipSystems;
	out << AuTime();	//m_tmSystemsUpg;
	m_SystemsLock.unlock();

	m_TurretLock.lockForRead();
	out << (quint16)m_Turrets.size();
	for(unordered_map<quint16, AuOrdnance>::const_iterator ti = m_Turrets.cbegin(); ti != m_Turrets.cend(); ti++)
	{
		out << (*ti).first;
		out << (*ti).second;
	}
	m_TurretLock.unlock();

	m_WaypointLock.lockForRead();
	out << m_aWaypoints;
	out << AuTime();	//m_tmWaypointChange;
	m_WaypointLock.unlock();
}

void DaVessel::operator<<(QDataStream& in)
{
	DaCapsule::operator<<(in);
	quint8 ui8;
	qint8 i8;

	in >> m_AcceptCrew;
	in >> m_AcceptPassengers;
	in >> m_AcceptTroops;
	in >> ui8;
	m_AccessPolicy = (ACCESS_POLICY)ui8;
	in >> m_AllowPassengerSeaChests;
	in >> m_AutoFetchVehicles;
	in >> m_bExtLightOn;
	in >> m_bPowerPlantOn;
	in >> m_bSensorsOn;
	in >> m_byAccelSkewDur;
	in >> m_byPowerPlantSkewDur;
	in >> m_byTurnRateSkewDur;
	in >> i8;
	m_chAccelSkew = i8;
	in >> i8;
	m_chPowerPlantSkew = i8;
	in >> i8;
	m_chTurnRateSkewX = i8;
	in >> i8;
	m_chTurnRateSkewY = i8;
	in >> m_ConCaptAI;
	in >> m_ConEngineerAI;
	in >> m_ConEngineerPerson;
	in >> m_ConNavigatorAI;
	in >> m_ConNavigatorPerson;
	in >> m_ConPilotAI;
	in >> m_ConPilotPerson;
	in >> m_ConPowerRelayAI;
	in >> m_ConPowerRelayPerson;
	in >> m_ConSensorAI;
	in >> m_ConSensorPerson;
	in >> m_ConShieldAI;
	in >> m_ConShieldPerson;
	in >> m_ConWeaponAI;
	in >> m_ConWeaponPerson;
	in >> m_FuelQuality;
	in >> m_InTow;
	in >> m_IsHumanLock;
	in >> m_IsPrivate;
	in >> m_LockedIsCivilian;
	in >> m_LockedStance;
	in >> m_Moored;
	in >> ui8;
	m_PilotUpPolicy = (PILOT_UP_POLICY)ui8;
	in >> m_PowerPlantShunts;
	in >> ui8;
	m_ShieldFuelPolicy = (SHIELDFUEL_POLICY)ui8;
	in >> ui8;
	m_ShieldPolicy = (SHIELD_POLICY)ui8;
	in >> ui8;
	m_ShieldState = (SHIELD_STATE)ui8;
	in >> m_QMade;

	m_VesselLock.lockForWrite();
	in >> m_CapacitorLevel;
	in >> m_CapacitorTrend;
	in >> m_ConAICaptId;
	in >> ui8;
	m_eGravityDriveState = (GRAVITY_DRIVE_STATE)ui8;
	in >> m_ETE;
	in >> m_fFuelQuantity;
	in >> m_LockedBlip;
	in >> m_LockedSysLoc;
	in >> m_Monster;
	in >> ui8;
	m_MonsterOrigin = (MONSTER_ORIGIN)ui8;
	in >> m_RatSpecies;
	in >> m_RepairSecs;
	in >> m_WhereMade;
	m_VesselLock.unlock();

	m_AccountLock.lockForWrite();
	in >> m_Bank;
	in >> m_BankSubsidyTax;
	AuTime _tmLastBank;
	in >> _tmLastBank;
	m_AccountLock.unlock();

	m_BayLock.lockForWrite();
	m_BayFired.clear();
	quint16 ui16;
	in >> ui16;
	while(ui16--)
	{
		quint16 id;
		AuBayFired bay;
		in >> id;
		in >> bay;
		m_BayFired.insert(pair<quint16, AuBayFired>(id, bay));
	}

	AuTime _tmBayFired;
	in >> _tmBayFired;

	m_BaySetting.clear();
	in >> ui16;
	while(ui16--)
	{
		quint16 id;
		AuBaySetting bay;
		in >> id;
		in >> bay;
		m_BaySetting.insert(pair<quint16, AuBaySetting>(id, bay));
	}
	m_BayLock.unlock();

	m_CargoLock.lockForWrite();
	in >> m_Cargo;
	in >> m_Shipments;
	AuTime _tmCargoChange;
	in >> _tmCargoChange;
	m_CargoLock.unlock();

	m_CommLock.lockForWrite();
	in >> m_ATCAtRest;
	in >> m_ATCCity;
	in >> m_ATCCityEnemy;
	AuTime aTCContact;
	in >> aTCContact;
	in >> m_ATCStation;
	in >> m_ATCStationEnemy;
	AuTime tmAlert;
	in >> tmAlert;
	AuTime tmEnvDamage;
	in >> tmEnvDamage;
	AuTime tmSOS;
	in >> tmSOS;
	m_CommLock.unlock();

	m_ControlLock.lockForWrite();
	in >> m_byRotSpeedX;
	in >> m_byRotSpeedY;
	in >> m_byRotSpeedZ;
	in >> m_Pitch;
	in >> m_Roll;
	in >> m_Throttle;
	in >> m_ThrustLateral;
	in >> m_ThrustLong;
	in >> m_ThrustVertical;
	AuTime tmLastBrake;
	in >> tmLastBrake;
	AuTime tmLastBrakeLV;
	in >> tmLastBrakeLV;
	AuTime tmLastPitchControl;
	in >> tmLastPitchControl;
	AuTime tmLastRollControl;
	in >> tmLastRollControl;
	AuTime tmLastRPYControl;
	in >> tmLastRPYControl;
	AuTime tmLastThrottleControl;
	in >> tmLastThrottleControl;
	AuTime tmLastThrottleFControl;
	in >> tmLastThrottleFControl;
	AuTime tmLastThrottleLControl;
	in >> tmLastThrottleLControl;
	AuTime tmLastThrottleVControl;
	in >> tmLastThrottleVControl;
	AuTime tmLastTurnControl;
	in >> tmLastTurnControl;
	in >> m_Yaw;
	in >> m_wActions;
	m_ControlLock.unlock();

	m_DamageLock.lockForWrite();
	in >> m_Damage;
	in >> m_tmDamageChange;
	m_DamageLock.unlock();

	m_GamesLock.lockForWrite();
	m_Games.clear();
	in >> ui16;
	while(ui16--)
	{
		quint16 id;
		quint8 type;
		AuGame* game;
		in >> id;
		in >> type;
		game = AuGame::CreateGame(type);
		in >> *game;
		m_Games.insert(pair<quint16, AuGame*>(id, game));
	}
	AuTime _GamesChanged;
	in >> _GamesChanged;
	m_GamesLock.unlock();

	m_LastInRangeLock.lockForWrite();
	QTime lastInEnemyRange;
	in >> lastInEnemyRange;
	m_LastInRangeLock.unlock();

	m_MissionLock.lockForWrite();
	in >> m_Mission;
	AuTime _tmMissionChange;
	in >> _tmMissionChange;
	m_MissionLock.unlock();

	m_NameWithEmpireLock.lockForWrite();
	in >> m_NameWithEmpire;
	m_NameWithEmpireLock.unlock();

	m_RosterLock.lockForWrite();
	in >> m_CryoBerths;
	in >> m_OwnerId;
	in >> m_OwnerName;
	in >> m_Roster;
	AuTime lastOfficerOnline;
	in >> lastOfficerOnline;
	AuTime _tmRosterChange;
	in >> _tmRosterChange;
	m_RosterLock.unlock();

	m_ShieldsLock.lockForWrite();
	for(int s=0; s<6; s++)
	{
		in >> m_ShieldCap[s];
		in >> m_ShieldScram[s];
	}
	m_ShieldsLock.unlock();

	m_SystemsLock.lockForWrite();
	in >> m_ssaShipSystems;
	AuTime _tmSystemsUpg;
	in >> _tmSystemsUpg;
	m_SystemsLock.unlock();

	m_TurretLock.lockForWrite();
	m_Turrets.clear();
	in >> ui16;
	while(ui16--)
	{
		quint16 id;
		AuOrdnance o;
		in >> id;
		in >> o;
		m_Turrets.insert(pair<quint16, AuOrdnance>(id, o));
	}
	m_TurretLock.unlock();

	m_WaypointLock.lockForWrite();
	in >> m_aWaypoints;
	AuTime _tmWaypointChange;
	in >> _tmWaypointChange;
	m_WaypointLock.unlock();
}

bool DaVessel::CommAcceptBerth(int berthType, const AuCommAcceptBerth& msg)
{
	// We trust that a ship accepting a berth request has already made all the proper checks.
	// The acceptor has the authority to accept the request.
	if(BerthTypeOccupied(msg.UnitId()) == berthType)
		return false;	// The requestor is already in that kind of berth.

	// Insure there is an empty berth of the type desired.
	switch(berthType)
	{
	case AuRosterEntry::Invalid:
	case AuRosterEntry::OfficerAI:
	case AuRosterEntry::PassengerAI:
		return false;		// Invalid type of berth to request.
	case AuRosterEntry::Captain:
	case AuRosterEntry::CaptainSlot:
		if(CaptainsBerthOccupied())
			return false;	// There are no empty berths.
		break;
	case AuRosterEntry::OfficerHuman:
		if(!OfficerBerthsAvailable())
			return false;	// There are no empty berths.
		break;
	case AuRosterEntry::Crew:
		if(!CrewBerthsAvailable())
			return false;	// There are no empty berths.
		break;
	case AuRosterEntry::PassengerHuman:
		if(!PassengerBerthsAvailable())
			return false;	// There are no empty berths.
		break;
	case AuRosterEntry::Troop:
		if(!TroopBerthsAvailable())
			return false;	// There are no empty berths.
		break;
	}

	// The ship has an empty berth.

	// Add the requestor to the roster of the ship.
	// OccupyBerth insures the requestor is only in the roster one time.
	OccupyBerth((AuRosterEntry::TYPE)berthType, msg.UnitId(), msg.UnitName());
	SetModified();
	return true;
}

AuId DaVessel::FindStationOccupant(int stationType) const
{
	switch(stationType)
	{
	default:
		qWarning() << "error: DaVessel::FindStationOccupant bad stationType " << stationType;
		break;
	case AuStation::Integrated_Command:
		return AIFirstOfficer();
	case AuStation::Integrated_Designer:
		{
			// Find a person who is subordinate to a designer that is aboard this vessel.
			DaAtomList atoms;
			FindPeopleRecursive(atoms);
			for(DaAtomList::const_iterator ai = atoms.cbegin(); ai != atoms.cend(); ai++)
			{
				const DaPerson* person = (const DaPerson*)*ai;
				if(!person)
					continue;
				if(person->InDesignEnvironment())
					return person->Id();
			}
		}
		break;
	case AuStation::Integrated_Engine:
		{
			DaSentient* sent = AIEngineOperator();
			if(sent)
				return sent->Id();
		}
		break;
	case AuStation::Integrated_FireControl:
		return AIFireControl();
	case AuStation::Integrated_Helm:
		return AIPilot();
	case AuStation::Integrated_Medic:
		return AIMedic();
	case AuStation::Integrated_Navigation:
		return AINavigator();
	case AuStation::Integrated_Patient:
		return AIPatient();
	case AuStation::Integrated_Power:
		return AIPowerRelay();
	case AuStation::Integrated_Sensor:
		return AISensor();
	case AuStation::Integrated_Shield:
		return AIShield();
	case AuStation::Integrated_Transporter:
		return AITransporter();
	case AuStation::Turret_Integrated:
		return AITurretGunner();
	case AuStation::Table_Game:
		return AIGamer();
	case AuStation::Troop:
		return AITroopAtPost();
	case AuStation::Citizen:
		return AICitizenAtPost();
	case AuStation::Livestock:
		return AILivestockAtPost();
	}
	return 0;
}

AuId DaVessel::FindStationOccupantInRoom(int stationType, const QString& room) const
{
	switch(stationType)
	{
	default:
		qWarning() << "error: DaVessel::FindStationOccupantInRoom bad stationType " << stationType << "in room" << room;
		break;
	case AuStation::Integrated_Command:
		{
			AuId firstOfficerId;
			DaAtomList subs;
			ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				DaSentient* officer = dynamic_cast<DaSentient*>(*it);
				if(!officer)
					continue;
				if(!officer->IsAIControlled())
					continue;
				if(officer->CapsuleRoomName() != room)
					continue;
				if(Is(officer->AssignedShip()))
				{
					// Demiavatars may hold non-officer berths to perform the duties of other crew positions.
					if(!AuRosterEntry::IsOfficer(officer->AssignedShipBerthType()))
						continue;

					// Found an officer assigned to this ship.
					if(officer->HasTheCon())
					{
						// First officer who has the con.
						return officer->Id();
					}
					else if(firstOfficerId.IsNull())
					{
						if(dynamic_cast<DaNpc*>(officer) || dynamic_cast<DaPerson*>(officer))
							firstOfficerId = officer->Id();		// First officer (officer or demiavatar) who does not have the con.
					}
				}
			}
			return firstOfficerId;
		}
	case AuStation::Integrated_Designer:
		{
			// Find a person who is subordinate to a designer that is aboard this vessel.
			DaAtomList atoms;
			FindPeopleRecursive(atoms);
			for(DaAtomList::const_iterator ai = atoms.cbegin(); ai != atoms.cend(); ai++)
			{
				const DaPerson* person = (const DaPerson*)*ai;
				if(!person)
					continue;
				if(!person->InDesignEnvironment())
					continue;
				const DaDesigner* designer = person->FindParentDesignerInEnv();
				if(!designer)
					continue;
				// Determine the room that contains the designer instance.
				ROOMTYPE roomType;
				QString roomName;
				quint16 droom = InsideRoomType(designer->Location(), roomType, roomName);
				if(!droom)
					continue;
				if(roomName == room)
					return person->Id();
			}
		}
		break;
	case AuStation::Integrated_Engine:
		{
			DaAtomList subs;
			ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				DaSentient* sent = dynamic_cast<DaSentient*>(*it);
				if(!sent)
					continue;
				if(sent->CapsuleRoomName() != room)
					continue;
				if(sent->IsEngineStationPose() && sent->IsAIControlled())
					return sent->Id();
			}
		}
		break;
	case AuStation::Integrated_FireControl:
		{
			DaAtomList subs;
			ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				DaSentient* sent = dynamic_cast<DaSentient*>(*it);
				if(!sent)
					continue;
				if(sent->CapsuleRoomName() != room)
					continue;
				if(sent->IsFireControlStationPose() && sent->IsAIControlled())
					return sent->Id();
			}
		}
		break;
	case AuStation::Integrated_Helm:
		{
			DaAtomList subs;
			ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				DaSentient* sent = dynamic_cast<DaSentient*>(*it);
				if(!sent)
					continue;
				if(sent->CapsuleRoomName() != room)
					continue;
				if(sent->IsHelmStationPose() && sent->IsAIControlled())
					return sent->Id();
			}
		}
		break;
	case AuStation::Integrated_Medic:
		{
			DaAtomList subs;
			ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				DaSentient* sent = dynamic_cast<DaSentient*>(*it);
				if(!sent)
					continue;
				if(sent->CapsuleRoomName() != room)
					continue;
				if(sent->IsMedicStationPose() && sent->IsAIControlled())
					return sent->Id();
			}
		}
		break;
	case AuStation::Integrated_Navigation:
		{
			DaAtomList subs;
			ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				DaSentient* sent = dynamic_cast<DaSentient*>(*it);
				if(!sent)
					continue;
				if(sent->CapsuleRoomName() != room)
					continue;
				if(sent->IsNavStationPose() && sent->IsAIControlled())
					return sent->Id();
			}
		}
		break;
	case AuStation::Integrated_Patient:
		{
			DaAtomList subs;
			ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				DaSentient* sent = dynamic_cast<DaSentient*>(*it);
				if(!sent)
					continue;
				if(sent->CapsuleRoomName() != room)
					continue;
				if(sent->IsPatientStationPose() && sent->IsAIControlled())
					return sent->Id();
			}
		}
		break;
	case AuStation::Integrated_Power:
		{
			DaAtomList subs;
			ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				DaSentient* sent = dynamic_cast<DaSentient*>(*it);
				if(!sent)
					continue;
				if(sent->CapsuleRoomName() != room)
					continue;
				if(sent->IsPowerStationPose() && sent->IsAIControlled())
					return sent->Id();
			}
		}
		break;
	case AuStation::Integrated_Sensor:
		{
			DaAtomList subs;
			ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				DaSentient* sent = dynamic_cast<DaSentient*>(*it);
				if(!sent)
					continue;
				if(sent->CapsuleRoomName() != room)
					continue;
				if(sent->IsSensorStationPose() && sent->IsAIControlled())
					return sent->Id();
			}
		}
		break;
	case AuStation::Integrated_Shield:
		{
			DaAtomList subs;
			ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				DaSentient* sent = dynamic_cast<DaSentient*>(*it);
				if(!sent)
					continue;
				if(sent->CapsuleRoomName() != room)
					continue;
				if(sent->IsShieldStationPose() && sent->IsAIControlled())
					return sent->Id();
			}
		}
		break;
	case AuStation::Integrated_Transporter:
		{
			DaAtomList subs;
			ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				DaSentient* sent = dynamic_cast<DaSentient*>(*it);
				if(!sent)
					continue;
				if(sent->CapsuleRoomName() != room)
					continue;
				if(sent->IsTransporterStationPose() && sent->IsAIControlled())
					return sent->Id();
			}
		}
		break;
	case AuStation::Turret_Integrated:
		{
			DaAtomList subs;
			ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				DaSentient* sent = dynamic_cast<DaSentient*>(*it);
				if(!sent)
					continue;
				if(sent->CapsuleRoomName() != room)
					continue;
				if(sent->IsTurretPose() && sent->IsAIControlled())
					return sent->Id();
			}
		}
		break;
	case AuStation::Table_Game:
		{
			DaAtomList subs;
			ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				DaSentient* sent = dynamic_cast<DaSentient*>(*it);
				if(!sent)
					continue;
				if(sent->CapsuleRoomName() != room)
					continue;
				if(sent->IsGameTablePose() && sent->IsAIControlled())
					return sent->Id();
			}
		}
		break;
	case AuStation::Troop:
		{
			DaAtomList subs;
			ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				DaSentient* sent = dynamic_cast<DaSentient*>(*it);
				if(!sent)
					continue;
				if(sent->CapsuleRoomName() != room)
					continue;
				if(sent->IsPostPose() && sent->IsAIControlled())
					return sent->Id();
			}
		}
		break;
	case AuStation::Citizen:
		{
			DaAtomList subs;
			ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				DaSentient* sent = dynamic_cast<DaSentient*>(*it);
				if(!sent)
					continue;
				if(sent->CapsuleRoomName() != room)
					continue;
				if(sent->IsPostPose() && sent->IsAIControlled())
					return sent->Id();
			}
		}
		break;
	case AuStation::Livestock:
		{
			DaAtomList subs;
			ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				DaAnimal* animal = dynamic_cast<DaAnimal*>(*it);
				if(!animal)
					continue;
				if(animal->CapsuleRoomName() != room)
					continue;
				if(animal->IsPostPose())
					return animal->Id();
			}
		}
		break;
	}
	return 0;
}

double DaVessel::ExploreRadius(const DaWorldLL& world) const
{
	// Under water gives worst exploration radius.
	if(IsCGSubmerged())
		return ObstacleRadius() + 50*FeetPerMeter();

	// Gas giant can only be explored while underwater.
	if(world.IsGasGiant())
		return 0;

	// Not a gas giant.
	if(!IsInAtmosphere())
	{
		// Spacecraft outside atmosphere can see the entire world.
		return -1;	// Explore all.
	}

	// Exploration radius improves with altitude above the ground.
	return ObstacleRadius() + 250*FeetPerMeter() + AltitudeAGL();
}

