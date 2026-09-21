//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: davehicle.h,v 1.396 2022/05/17 16:58:17 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaVehicle_H
#define DaVehicle_H

#include "augleffects.h"
#include "auordnance.h"
#include "aupose.h"
#include "auskill.h"
#include "daunit.h"

#define MAX_VEHICLE_CONTRAILS	2

class Au3dsModelInstance;
class AuItem;
class AuSensorBlipList;
class DaCreature;
class DaNpc;
class PsEngineTrail;

class AUATOM_DLL DaVehicle : public DaUnit  
{
protected:
	enum AUDIO_SOURCE
	{
		AS_Engine,
		AS_Ground,
		AS_MaxSources
	};

public:
	enum CONTROLINPUT
	{
		CI_AllStop,
		CI_DecreaseThrottle,
		CI_IncreaseThrottle,
		CI_NextSeat,
		CI_PreviousSeat,
		CI_Seat0,
		CI_Seat1,
		CI_Seat2,
		CI_Seat3,
		CI_Seat4,
		CI_Seat5,
		CI_Seat6,
		CI_Seat7,
		CI_Seat8,
		CI_Seat9,
		CI_TurnLeft,
		CI_TurnRight,
		CI_Brakes,
		CI_PitchDown,
		CI_PitchUp,
		CI_RollLeft,
		CI_RollRight,
		CI_DecreaseThrottleL,	// thrust lateral
		CI_IncreaseThrottleL,	// thrust lateral
		CI_DecreaseThrottleV,	// thrust vertical
		CI_IncreaseThrottleV,	// thrust vertical
		CI_BrakesLV,	// lateral and vertical together
		CI_EngineOnOff,
		CI_LightOnOff,
		CI_UNUSED0,
		CI_FireGun,
		CI_DoorRemote,	// open/close bay door(s), like garage door opener
		CI_MaxForwardThrottle,
		CI_MaxReverseThrottle,
		CI_SetThrottle,
		CI_LookBack,
		CI_LookForward,
		CI_LookLeft,
		CI_LookLeftBack,
		CI_LookLeftForward,
		CI_LookRight,
		CI_LookRightBack,
		CI_LookRightForward,
		CI_ThrustB,		// thrust backward
		CI_ThrustF,		// thrust forward
		CI_Service,		// refuel, repair and rearm from current location/context. e.g. hold, slab, gear
		CI_FireWeapon,
		CI_SelectWeapon,
		CI_OrbitRingsToggle,
		CI_Hold,
		CI_RequestLaunch,				// Requests launch from ship with space vehicle launcher.
		CI_RequestRecovery,				// Requests recovery by nearby ship with space vehicle recovery system.
		CI_ReleaseMotherShip,			// Release mothership assignment.
	};

	enum SWITCHES
	{
		SWITCH_Engine					= 0x01,					// Power on/off switch.
		SWITCH_Light					= 0x02,					// Light on/off switch.
		SWITCH_Used						= 0x04,					// true after occupied once
		SWITCH_Hold						= 0x08,					// Brakes hold on/off switch.
		SWITCH_OrbitRings				= 0x10,					// Orbit ring visibility on/off switch.
	};

	enum WEAPON_RACK
	{
		RACK_None,
		RACK_Gun,
		RACK_Ord0,
		RACK_Ord1,
	};

private:
	// Unguarded
	unsigned char						m_byRotSpeedX;			// Rotation speed about X (roll left/right).
	unsigned char						m_byRotSpeedY;			// Rotation speed about Y (pitch up/down).
	unsigned char						m_byRotSpeedZ;			// Rotation speed about Z (turn left/right).
	COMMODITYID							m_cid;
	bool								m_DriverOnLine;
	quint8								m_Fuel;					// 1 fuel unit is burned per minute
	bool								m_Parked;				// true after autoparked in vehicle bay, until disturbed
	quint8								m_Quality;				// Quality when manufactured.
	quint8								m_Switches;				// SWITCHES bits OR'd together
	qint8								m_Throttle;				// Throttle setting (constant forward/backward thrust)
	qint8								m_ThrustLateral;		// momentary lateral thrust throttle, - left, + right
	qint8								m_ThrustLong;			// momentary longitudinal thrust throttle, - backward, + forward
	qint8								m_ThrustVertical;		// momentary vertical thrust throttle, - down, + up

private:
	mutable QReadWriteLock				m_VehicleLock;
	int									m_wActions;				// ACT_ Bits representing current actions to process during Tick.
	AuId								m_MotherShip;			// Parent spaceship where I was fetched or last autoparked.
	AuElapsedTimer						m_MotherShipChg;
	QString								m_MotherShipName;
	AuElapsedTimer						m_tmFuelChanged;
	AuElapsedTimer						m_tmLastBrake;
	AuElapsedTimer						m_tmLastBrakeLV;
	AuElapsedTimer						m_tmLastPitchControl;	// Time last control received.
	AuElapsedTimer						m_tmLastRollControl;	// Time last control received.
	AuElapsedTimer						m_tmLastThrottleControl;	// Time last control received.
	AuElapsedTimer						m_tmLastThrottleFControl;	// Time last control received. Forward/back.
	AuElapsedTimer						m_tmLastThrottleLControl;	// Time last control received. Lateral.
	AuElapsedTimer						m_tmLastThrottleVControl;	// Time last control received. Vertical.
	AuElapsedTimer						m_tmLastTurnControl;	// Time last control received.

	quint16								m_Hits;					// current hit points
	AuElapsedTimer						m_tmHitsChanged;

	// Ammunition
	AuOrdnance							m_Gun;
	AuOrdnance							m_Ord[2];
	quint8								m_OrdSelect;			// Index of the weapon that is currently selected.

private:
	mutable QReadWriteLock				m_LaunchLock;
	AuTime								m_LaunchChg;
	AuPoint								m_LauncherExit;
	quint16								m_LauncherId;
	AuId								m_LauncherShip;
	float								m_LaunchSpeed;

private:
	// OpenAL
	unsigned int						m_AudioVehicle[AS_MaxSources];
	int									m_Sound[AS_MaxSources];	// current SOUND_EFFECT

	// OpenGL
	mutable Au3dsGLSLModelInstance*		m_Model;
	PsEngineTrail*						m_pEngineTrail[MAX_VEHICLE_CONTRAILS];
	AuGLVAO								m_WeaponVAO;			// Guns/Weapons icon.

public:
	DaVehicle(COMMODITYID cid, quint8 quality);
	DaVehicle();
	virtual ~DaVehicle();

protected:
	DaVehicle(const DaVehicle &rhs);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	// Add a visual and/or audio effect to this atom.
	// 'effect' may be modified to something appropriate. e.g. VISUAL_HitPierce (looks like blood) is changed to
	// VISUAL_HitPierceBug when this atom is an exoskeletal creature, and something else when this atom is not even a creature.
	virtual void AddEffect(AuEffect& effect);

	virtual void BuildingSurrendered(quint32 surrenderingEmpire, quint32 toEmpire);

	// Returns true for a new vehicle that can be fetched by a spacecraft filling its 'spotType' parking spot. Excludes things
	// like farm machinery.
	bool CanFetch(VEHICLE_SPOT_TYPE spotType) const;

	bool CanBdFetch(VEHICLE_SPOT_TYPE spotType, const DaBd* fetcher) const;

	WEAPON_RACK CanLoadAmmunition(const AuItem& item) const;

	// Returns true if any changes are made.
	inline bool ClearMotherShip()
	{
		bool cleared = false;
		m_VehicleLock.lockForWrite();
		if(!m_MotherShip.IsNull() || !m_MotherShipName.isEmpty())
		{
			m_MotherShip = AuId::NullId();
			m_MotherShipName = "";
			m_MotherShipChg.Set();
			cleared = true;
		}
		m_VehicleLock.unlock();
		return cleared;
	}

	virtual DaAtom *Clone() const;

	int ColonistsAboard() const;

	// 'attacker' is the atom that is causing the damage. The attacker id should be NULL when the attacker is not a DaUnit, like
	//   when it is the environment or a sun or something.
	// 'weapon' describes the weapon that was used to inflict the damage.
	// 'wpnLoc' is the location of the weapon that is attacking this atom, relative to this atom.
	// 'hitLoc' is the location of the hit on this atom, relative to this atom.
	// 'result' contains the results of the attack, except the IsDestroyed state; this function call sets that.
	// Sets the IsDestroyed result in 'result', if this atom is killed. This is temporal death, not necessarily atomic death.
	// IsDestroyed result is not modified if this atom is already dead.
	// The only thing this base class function does is sends a log message to 'whoId', if it's not NULL.
	virtual void CombatApplyDamage(const AuCombatAttacker& attacker, const AuCombatWeapon& weapon, const AuPoint& wpnLoc,
			const AuPoint& hitLoc, AuCombatResult& result);

	virtual void CombatArmorEffectiveness(WEAPONMODE mode, WEAPONEFFECT effect, bool hitShield, const AuPoint& wpnLoc,
			const AuPoint& hitLoc, float& baseAbsorb, float& baseBlock, float& auxAbsorb, float& auxBlock) const;

	void Create();

	short int CurrentHitPoints() const
	{
		m_VehicleLock.lockForRead();
		short int v = m_Hits;
		m_VehicleLock.unlock();
		return v;
	}

	// Looks for the driver, if there is one.
	// Returns a person on line who is in a control seat, preferably.
	// Returns a citizen in a control seat if no person on line is found in a control seat.
	DaCreature* Driver() const;

	// Returns the skill earned by the driver for operating this type of vehicle.
	AuSkill::SKILL DriverSkill() const;

	virtual bool DepartingSolarSystem();

	virtual QString Description() const;

	void DestroyVehicle(const QString& reason);

	virtual void EmpireDeleted(quint32 empireId);

	virtual quint32 ExperiencePointsForKilling() const;

	double ExploreRadius(const DaWorldLL& world) const;

	virtual double FallSafeSpeed() const;

	inline int Fuel() const
	{
		// Returns -1 if this vehicle does not use fuel.
		if(m_cid == CID_SailBoat)
			return -1;
		return m_Fuel;
	}

	// Used for determination of color highlighting based on political stance toward this unit.
	// Civilian units use a different color palette than military units.
	virtual bool IsCivilian() const
	{
		// Vehicles are considered civilian until they are no longer nu.
		return IsNew() ? true : false;
	}

	inline bool IsDriverOnLine() const
	{
		return m_DriverOnLine;
	}

	virtual bool IsEthereal() const;

	inline bool IsGroundVehicle() const
	{
		return DbCommodity::IsGroundVehicle(m_cid);
	}

	// Returns an atom that is in 'gunRange' of 'gunLoc', which is relative to my parent world.
	// 'gunnerEmpire' is the empire id of the gunner, who is at my parent world.
	// 'gunnerPosture' is the posture of the gunner. Caller checks POSTURE_HoldFire before calling to save lots of redundant checks.
	// 'targetLoc' is an output parameter relative to my parent world.
	virtual bool IsGuardTowerTarget(quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunLoc, float gunRange,
			AuPoint& targetLoc) const;

	virtual bool IsInScene(const DaPerson* sceneAtom, const AuPoint& cameraEnvLoc) const;

	// Note that a space rocket is also a space vehicle.
	inline bool IsSpaceRocket() const
	{
		return DbCommodity::IsSpaceRocket(m_cid);
	}

	inline bool IsSpaceVehicle() const
	{
		return DbCommodity::IsSpaceVehicle(m_cid);
	}

	inline bool IsWaterVehicle() const
	{
		return DbCommodity::IsWaterVehicle(m_cid);
	}

	inline void ClearLauncher()
	{
		m_LaunchLock.lockForWrite();
		m_LauncherId = 0;
		m_LaunchChg.Set();
		m_LaunchLock.unlock();
	}

	inline bool IsLaunching() const
	{
		m_LaunchLock.lockForRead();
		bool v = m_LauncherId;
		m_LaunchLock.unlock();
		return v;
	}

	// Return true if this atom can be targetted by a turret on a spacecraft.
	// 'gunnerEmpire' is the empire id of the gunner. It is 0 when political stance is to be ignored.
	// 'gunnerPosture' is the posture of the gunner. Caller checks POSTURE_HoldFire before calling to save lots of redundant checks.
	// 'gunSysLoc' and 'gunSysVec' are relative to the solar system.
	// 'targetSysLoc' is the location to target, relative to the solar system.
	virtual bool IsShipTurretTarget(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
			CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
			AuPoint& targetSysLoc) const;

	inline bool IsSmoldering() const
	{
		return CurrentHitPoints() < MaxHitPoints()/2;
	}

	// Return true for vehicles that can fly using a gravity drive.
	inline bool HasGravityDrive() const
	{
		switch(m_cid)
		{
		case CID_AdvSpaceFighter:
		case CID_SpaceFighter:
		case CID_SpaceTransport:
			return true;
		default:
			return false;
		}
	}

	bool HasWeapon() const;

	inline quint8 FuelMax() const
	{
		quint8 fuelMax = 0;
		switch(m_cid)
		{
		case CID_DirtBike:
			fuelMax = 30;
			break;
		case CID_AdvSpaceFighter:
		case CID_SpaceFighter:
			fuelMax = 45;
			break;
		case CID_SpaceRocket:
			fuelMax = 20;
			break;
		case CID_Helicopter:
			fuelMax = 25;
			break;
		case CID_SpaceTransport:
			fuelMax = 60;
			break;
		case CID_Submarine:
			fuelMax = 100;
			break;
		case CID_SUV:
		case CID_APC:
		case CID_FarmMachinery:
			fuelMax = 60;
			break;
		case CID_SailBoat:
		default:
			fuelMax = 0;	// no fuel required
			break;
		}
		return fuelMax;
	}

	inline COMMODITYID FuelType() const
	{
		switch(m_cid)
		{
		case CID_DirtBike:
		case CID_Submarine:
		case CID_SUV:
		case CID_APC:
		case CID_FarmMachinery:
		case CID_Helicopter:
			return CID_Gasoline;
		case CID_SailBoat:
		default:
			return CID_Invalid;	// Doesn't require fuel.
		case CID_AdvSpaceFighter:
		case CID_SpaceFighter:
		case CID_SpaceRocket:
		case CID_SpaceTransport:
			return CID_Hydrogen;
		}
	}

	float HullStress() const;

	// Returns true if this vehicle is parked in the appropriate type of parking spot, so it can be beamed by a vehicle transporter.
	bool InParkingSpot() const;

	// Returns true if a control seat is occupied by a person on line.
	bool IsControllerOnLine() const;

	// 'hitNormal' is an output parameter. It is the normal to the surface or detail that is hit.
	// 'toWho' is the atom attempting to move so we can automatically open doors when appropriate.
	virtual bool IsPathBlocked(const AuPoint& oloc, const AuPoint& nloc, AuPoint& hitLoc, AuPoint& hitNormal, const DaAtom* toWho);

	// Called when this unit is killed by a person.
	virtual void KilledByPerson(DaPerson* killer);

	inline LANDING_ORIENT LandingOrientation() const
	{
		if(IsSpaceRocket())
			return LORI_Tail;
		return LORI_Belly;
	}

	void Launch(const AuId& shipId, quint16 launcherId, AuMatrix4x4& launchPadM, AuPoint& launchExit);

	inline AuTime LaunchChange() const
	{
		m_LaunchLock.lockForRead();
		AuTime v(m_LaunchChg);
		m_LaunchLock.unlock();
		return v;
	}

	inline AuPoint LauncherExit() const
	{
		m_LaunchLock.lockForRead();
		AuPoint v(m_LauncherExit);
		m_LaunchLock.unlock();
		return v;
	}

	inline quint16 LauncherId() const
	{
		m_LaunchLock.lockForRead();
		quint16 v = m_LauncherId;
		m_LaunchLock.unlock();
		return v;
	}

	inline AuId LaunchingVessel() const
	{
		m_LaunchLock.lockForRead();
		AuId v(m_LauncherShip);
		m_LaunchLock.unlock();
		return v;
	}

	inline float LaunchSpeed() const
	{
		m_LaunchLock.lockForRead();
		float v = m_LaunchSpeed;
		m_LaunchLock.unlock();
		return v;
	}

	void LoadAmmunition(const AuItem& item, WEAPON_RACK rack);

	virtual AuPoint MovementBlocked(DaAtom* hit, const AuPoint& hitLoc, const AuPoint& hitNormal, bool hitShield);

	// Subatoms of vehicles never move freely.
	virtual bool MoveSubatom(DaAtom* pMove, const AuPoint& vMove, int elapsedMs);

	// Called to park this vehicle in a parking spot by a recovery system. Shuts everything down and stops moving.
	// 'parkingSpot' is the ground level position of the parking spot.
	void Park(DaAtom* parent, AuMatrix4x4& parkingSpot, VEHICLE_SPOT_TYPE spotType);

	// Returns the type of the parking spot primarily used by this vehicle.
	// Returns SPOT_Invalid on failure.
	inline VEHICLE_SPOT_TYPE ParkingSpotType() const
	{
		return ParkingSpotType(CommodityId());
	}

	// Returns the type of the parking spot primarily used by a vehicle commodity.
	// Returns SPOT_Invalid on failure.
	static VEHICLE_SPOT_TYPE ParkingSpotType(COMMODITYID vehicleCID);

	virtual float RelativeAudioGain(const DaPerson* sceneAtom) const;

	// Returns ord1 for unguided munitions, ord0 for guided munitions, even though they can be loaded alternately in either rack.
	static inline WEAPON_RACK WeaponRack(COMMODITYID ordCID)
	{
		switch(ordCID)
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
		case CID_HeavyArmsAmmoPi:
		case CID_HeavyArmsAmmoPiRa:
		case CID_HeavyArmsAmmoPiBl:
		case CID_HeavyArmsAmmoPiAc:
			return RACK_Gun;
		case CID_BombBl:
		case CID_BombBlRa:
		case CID_BombBlPi:
		case CID_BombElSt:
		case CID_BombSuPo:
		case CID_BombSlCr:
		case CID_LargeRocketBl:
		case CID_LargeRocketBlBu:
		case CID_LargeRocketBuDi:
		case CID_SmallRocketBl:
		case CID_SmallRocketBlBu:
		case CID_SmallRocketSlPi:
			return RACK_Ord1;	// Ord1 returned for unguided munitions.
		case CID_LargeMissileBl:
		case CID_LargeMissileBlBu:
		case CID_LargeMissileSlPo:
		case CID_SmallMissileBl:
		case CID_SmallMissileBlBu:
		case CID_SmallMissileCrFr:
			return RACK_Ord0;	// Ord0 returned for guided munitions.
		}
		return RACK_None;
	}

	// Troop Functions
	bool BoardTroop();

	bool BoardTroop(DaNpc* troop);

	virtual void FindClosestShipThreatRecursive(const AuId& shipId, int shipDomain, const AuId& shipWorldId, quint32 shipEmpire,
			SHIELD_POLICY shieldPolicy, const AuPoint& shipSysLoc, float sensorRange, const DaAtom*& targetAtom,
			double& targetRange) const;

	virtual void FindVehiclesRecursive(DaAtomList& vehicles) const;

	// Find vehicles assigned to a particular ship.
	virtual void FindVehiclesRecursive(DaAtomList& vehicles, const AuId& motherShipId) const;

	inline quint8 Quality() const
	{
		return m_Quality;
	}

	inline AuOrdnance WeaponLoad(WEAPON_RACK rack) const
	{
		AuOrdnance v;
		m_VehicleLock.lockForRead();
		switch(rack)
		{
		case RACK_None:
			break;
		case RACK_Gun:
			v = m_Gun;
			break;
		case RACK_Ord0:
			v = m_Ord[0];
			break;
		case RACK_Ord1:
			v = m_Ord[1];
			break;
		}
		m_VehicleLock.unlock();
		return v;
	}

	QString WeaponLoadDesc(WEAPON_RACK rack) const;

	inline QString WeaponLoadListName(WEAPON_RACK rack) const
	{
		QString v;
		m_VehicleLock.lockForRead();
		switch(rack)
		{
		case RACK_None:
			break;
		case RACK_Gun:
			v = m_Gun.ListName();
			break;
		case RACK_Ord0:
			v = m_Ord[0].ListName();
			break;
		case RACK_Ord1:
			v = m_Ord[1].ListName();
			break;
		}
		m_VehicleLock.unlock();
		return v;
	}

	inline QString WeaponLoadName(WEAPON_RACK rack) const
	{
		QString v;
		m_VehicleLock.lockForRead();
		switch(rack)
		{
		case RACK_None:
			break;
		case RACK_Gun:
			v = m_Gun.Name();
			break;
		case RACK_Ord0:
			v = m_Ord[0].Name();
			break;
		case RACK_Ord1:
			v = m_Ord[1].Name();
			break;
		}
		m_VehicleLock.unlock();
		return v;
	}

	inline QString WeaponLoadPic(WEAPON_RACK rack) const
	{
		QString v;
		m_VehicleLock.lockForRead();
		switch(rack)
		{
		case RACK_None:
			break;
		case RACK_Gun:
			v = m_Gun.BitmapName();
			break;
		case RACK_Ord0:
			v = m_Ord[0].BitmapName();
			break;
		case RACK_Ord1:
			v = m_Ord[1].BitmapName();
			break;
		}
		m_VehicleLock.unlock();
		return v;
	}

	virtual void WorldCaptured(quint32 byEmpire);

	void RepairConditionsTell(const DaPerson* person, int where, const QString& whereName);

	inline double RoughRadius() const
	{
		return ObstacleRadius();
	}

	// Returns closest seat index within 1' of 'passengerLoc', ignoring Z.
	// Returns -1 if we have no model or if the model has no seats.
	int SeatAt(const AuPoint& passengerLoc) const;

	// Refuel, repair and rearm this vehicle from its environment.
	// 'person' may provide fuel if it can't be acquired from the environment.
	void Service(DaPerson* person);

	// Returns current throttle value for lateral thrust.
	inline int ThrustLateral() const
	{
		return m_ThrustLateral;
	}

	// Returns current throttle value for longitudinal thrust.
	inline int ThrustLongitudinal() const
	{
		// Momentary longitudinal thrust overrides the throttle setting.
		if(m_ThrustLong || CommodityId() == CID_Helicopter)
			return m_ThrustLong;
		return m_Throttle;
	}

	// Returns current throttle value for vertical thrust.
	inline int ThrustVertical() const
	{
		return m_ThrustVertical;
	}

	int TroopsAboard() const;

	int EmptySeats() const;

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

	// Returns an atom that is in 'gunRange' of 'gunSysLoc' and 'gunSysVec', which are relative to the solar system.
	// 'gunnerEmpire' is the empire id of the gunner, who is operating the weapon.
	// 'targetAtom' and 'targetRange' are output parameters.
	// 'targetSysLoc' is the location to target, relative to the solar system.
	virtual void FindClosestShipTurretTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
			quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
			const DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const;

	// Returns index of empty seat that is found. -1 if no seats are empty.
	int FindEmptySeat(AuPoint& seatLoc, AuPose& pose) const;

	bool IsControlSeat(int seatIndex) const
	{
		if(seatIndex < 0)
			return false;
		switch(m_cid)
		{
		default:
			return false;	// Unrecognized cid has no controls.
		case CID_DirtBike:
		case CID_SUV:
		case CID_APC:
		case CID_FarmMachinery:
		case CID_SpaceFighter:
		case CID_AdvSpaceFighter:
		case CID_SpaceRocket:
		case CID_Helicopter:
		case CID_SpaceTransport:
			return (seatIndex == 0) ? true : false;
		case CID_SailBoat:
			if(IsSeatOccupied(0, true))
			{
				// Seat 0 is occupied by a person on line so it is the control
				// seat.
				return (seatIndex == 0) ? true : false;
			}
			else
			{
				// Seat 0 is not occupied so seat 1 is the control seat.
				return (seatIndex == 1) ? true : false;
			}
		case CID_Submarine:
			if(IsSeatOccupied(0, true))
			{
				// Seat 0 is occupied by a person on line so it is the control
				// seat.
				return (seatIndex == 0) ? true : false;
			}
			else
			{
				// Seat 0 is not occupied so seat 3 is the control seat.
				return (seatIndex == 3) ? true : false;
			}
		}
	}

	inline bool IsEngineOn() const
	{
		return m_Switches & SWITCH_Engine;
	}

	inline bool IsHolding() const
	{
		return m_Switches & SWITCH_Hold;
	}

	inline bool IsLightOn() const
	{
		return m_Switches & SWITCH_Light;
	}

	inline bool IsOrbitRingsOn() const
	{
		return m_Switches & SWITCH_OrbitRings;
	}

	// Returns true if any creature is aboard.
	bool IsOccupied() const;

	bool IsValidSeatPose(int seatIndex, const AuPose& pose) const;

	// Called when this unit is killed by another unit.
	virtual void KilledByUnit(const DaUnit* killer);

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const;

	int NextUnoccupiedSeat(int fromSeat) const;

	AuPoint OffLoc(int seat) const;						// Returns offload loc

	int PreviousUnoccupiedSeat(int fromSeat) const;

	// Refuels this vehicle to full.
	void Refuel();

	// Refuels this vehicle using 'fuel' if it is the appropriate type of fuel.
	bool Refuel(const AuItem& fuel);

	virtual bool Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari);

	// Returns true if any damage is repaired.
	bool RepairDamage(COMMODITYID tool);

	inline int RoundsCount(WEAPON_RACK rack) const
	{
		int v = 0;
		m_VehicleLock.lockForRead();
		switch(rack)
		{
		case RACK_None:
			break;
		case RACK_Gun:
			v = m_Gun.RoundsCount();
			break;
		case RACK_Ord0:
			v = m_Ord[0].RoundsCount();
			break;
		case RACK_Ord1:
			v = m_Ord[1].RoundsCount();
			break;
		}
		m_VehicleLock.unlock();
		return v;
	}

	AuPoint SeatLoc(int seat) const;

	AuPose SeatPose(int seat) const;

	virtual QString SpotDescription(quint32 spotterEmpire) const;

	virtual void TestHullStress();

	virtual int TestMissileHit(const AuPoint& missileLoc, float proximity) const;

	virtual bool IsTicker() const;

	virtual void Tick(const AuElapsedTimer& last, const AuElapsedTimer& now);

	virtual void TickMovement(const AuElapsedTimer& last);

	virtual void TickOnClient(const AuElapsedTimer& last);

	virtual unsigned int IsTocker() const;

	virtual void Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans);

	// Called when this atom is credited with killing a unit.
	// All unit kills go through here.
	// This is where empires declare mutual enemies based on unit kills. See DaUnit::UnitKilled.
	virtual void UnitKilled(DaUnit* victim);

	int WeaponCount() const;

	AuPoint WeaponDirection(int weaponIndex) const;

	AuPoint WeaponLocation(int weaponIndex) const;

	AuPoint WeaponRotation(int weaponIndex) const;

	AuPoint GunEmitLoc(int fireSeq) const;

	inline int MaxWeaponRounds(COMMODITYID ordCID) const
	{
		return MaxWeaponRounds(CommodityId(), ordCID);
	}

	static inline int MaxWeaponRounds(COMMODITYID vehicleCID, COMMODITYID ordCID)
	{
		switch(vehicleCID)
		{
		default:
			break;
		case CID_SpaceFighter:
			switch(ordCID)
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
				// Energy weapons use 100 units of ammo per fuel unit.
				return 100;
			case CID_BombBl:
			case CID_BombBlRa:
			case CID_BombBlPi:
			case CID_BombElSt:
			case CID_BombSuPo:
			case CID_BombSlCr:
				return 10;
			case CID_HeavyArmsAmmoPi:
			case CID_HeavyArmsAmmoPiRa:
			case CID_HeavyArmsAmmoPiBl:
			case CID_HeavyArmsAmmoPiAc:
				return 600;
			case CID_LargeMissileBl:
			case CID_LargeMissileBlBu:
			case CID_LargeMissileSlPo:
				return 30;
			case CID_LargeRocketBl:
			case CID_LargeRocketBlBu:
			case CID_LargeRocketBuDi:
				return 20;
			case CID_SmallMissileBl:
			case CID_SmallMissileBlBu:
			case CID_SmallMissileCrFr:
				return 60;
			case CID_SmallRocketBl:
			case CID_SmallRocketBlBu:
			case CID_SmallRocketSlPi:
				return 40;
			}
			break;
		case CID_AdvSpaceFighter:
			switch(ordCID)
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
				// Energy weapons use 100 units of ammo per fuel unit.
				return 100;
			case CID_BombBl:
			case CID_BombBlRa:
			case CID_BombBlPi:
			case CID_BombElSt:
			case CID_BombSuPo:
			case CID_BombSlCr:
				return 30;
			case CID_HeavyArmsAmmoPi:
			case CID_HeavyArmsAmmoPiRa:
			case CID_HeavyArmsAmmoPiBl:
			case CID_HeavyArmsAmmoPiAc:
				return 400;
			case CID_LargeMissileBl:
			case CID_LargeMissileBlBu:
			case CID_LargeMissileSlPo:
				return 20;
			case CID_LargeRocketBl:
			case CID_LargeRocketBlBu:
			case CID_LargeRocketBuDi:
				return 40;
			case CID_SmallMissileBl:
			case CID_SmallMissileBlBu:
			case CID_SmallMissileCrFr:
				return 40;
			case CID_SmallRocketBl:
			case CID_SmallRocketBlBu:
			case CID_SmallRocketSlPi:
				return 80;
			}
			break;
		case CID_SpaceTransport:
			switch(ordCID)
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
				// Energy weapons use 100 units of ammo per fuel unit.
				return 100;
			case CID_BombBl:
			case CID_BombBlRa:
			case CID_BombBlPi:
			case CID_BombElSt:
			case CID_BombSuPo:
			case CID_BombSlCr:
				return 50;
			case CID_HeavyArmsAmmoPi:
			case CID_HeavyArmsAmmoPiRa:
			case CID_HeavyArmsAmmoPiBl:
			case CID_HeavyArmsAmmoPiAc:
				return 200;
			case CID_LargeMissileBl:
			case CID_LargeMissileBlBu:
			case CID_LargeMissileSlPo:
				return 10;
			case CID_LargeRocketBl:
			case CID_LargeRocketBlBu:
			case CID_LargeRocketBuDi:
				return 30;
			case CID_SmallMissileBl:
			case CID_SmallMissileBlBu:
			case CID_SmallMissileCrFr:
				return 20;
			case CID_SmallRocketBl:
			case CID_SmallRocketBlBu:
			case CID_SmallRocketSlPi:
				return 60;
			}
			break;
		case CID_APC:
			switch(ordCID)
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
				// Energy weapons use 100 units of ammo per fuel unit.
				return 100;
			case CID_BombBl:
			case CID_BombBlRa:
			case CID_BombBlPi:
			case CID_BombElSt:
			case CID_BombSuPo:
			case CID_BombSlCr:
				return 0;
			case CID_HeavyArmsAmmoPi:
			case CID_HeavyArmsAmmoPiRa:
			case CID_HeavyArmsAmmoPiBl:
			case CID_HeavyArmsAmmoPiAc:
				return 800;
			case CID_LargeMissileBl:
			case CID_LargeMissileBlBu:
			case CID_LargeMissileSlPo:
				return 40;
			case CID_LargeRocketBl:
			case CID_LargeRocketBlBu:
			case CID_LargeRocketBuDi:
				return 50;
			case CID_SmallMissileBl:
			case CID_SmallMissileBlBu:
			case CID_SmallMissileCrFr:
				return 70;
			case CID_SmallRocketBl:
			case CID_SmallRocketBlBu:
			case CID_SmallRocketSlPi:
				return 90;
			}
			break;
		}
		return 0;
	}

	AuPoint WeaponEmitLoc(int fireSeq) const;

	// Total hit points when undamaged.
	int MaxHitPoints() const
	{
		switch(m_cid)
		{
		case CID_DirtBike:
			return 150;
		case CID_SailBoat:
			return 100;
		case CID_SpaceFighter:
			return 500;
		case CID_AdvSpaceFighter:
			return 600;
		case CID_SpaceTransport:
			return 750;
		case CID_SpaceRocket:
		case CID_Helicopter:
			return 200;
		case CID_Submarine:
		case CID_FarmMachinery:
			return 300;
		case CID_SUV:
			return 250;
		case CID_APC:
			return 1000;
		default:
			return 0;
		}
	}

	// Repairs hit points of damage if needed.
	// Returns false if no repairs are needed.
	bool RepairHitPoints(quint16 repairHits);

	// Repairs all damage instantly.
	void ResetHitPoints();

	virtual void RxUpdate(QDataStream& fromServer, bool isFirstUpdate);
	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
			const DaPerson& sceneAtom);
	virtual ATOM_TYPE AtomType() const;

// 	// Functions used for just rendering the model, like in aumodelviewer
// 	bool CreateModelObject(AuGLEffects& re, QString sFile);
// 	void DeleteModelObject(AuGLEffects& re);
// 	void RenderModelObject(AuGLEffects& re, bool lightsOn);
	void GetExtents(AuPoint& pMin, AuPoint& pMax)
	{
		if(m_Model)
			m_Model->GetExtents(pMin, pMax);
	}

	
	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari);

	virtual void DeleteDeviceObjects(AuAtomRenderInfo& ari);

	// NOTE: Rendering states must already be configured. Check actual usage of this function.
	virtual void RenderBodyStats(float left, float bottom, float wid, float ht, bool isTarget, unsigned int targetPart,
			POLITICAL_STANCE stance, AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuMatrix4x4& tDevPersp,
			const AuMatrix4x4& tDevOrtho, float hudIntensity);

	virtual void RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
			const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass,
			DaAtomList& shadSc,
			DaAtomList& shadScFog,
			DaAtomList& shadScGlobe,
			DaAtomList& shadScGlobeFog,
			DaAtomList& shadScRingworld,
			DaAtomList& shadScRingworldFog);

	void RenderHud(float sightXpos, float sightYpos, int hudIntensity, AuAtomRenderInfo& ari);

	virtual void RenderLast(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
			const DaPerson* sceneAtom,
			DaAtomList& shadSc,
			DaAtomList& shadScFog,
			DaAtomList& shadScGlobe,
			DaAtomList& shadScGlobeFog,
			DaAtomList& shadScRingworld,
			DaAtomList& shadScRingworldFog);

	virtual bool CanBeSelected() const
	{
		return false;
	}

	virtual float GroundHeightAboveCG() const;

	virtual bool IsObstacle(const DaAtom* toWho) const;

	inline AuId MotherShip() const
	{
		m_VehicleLock.lockForRead();
		AuId v(m_MotherShip);
		m_VehicleLock.unlock();
		return v;
	}

	inline QString MotherShipName() const
	{
		m_VehicleLock.lockForRead();
		QString v(m_MotherShipName);
		m_VehicleLock.unlock();
		return v;
	}

	// Returns actual radius of this object for avoidance when navigating and bay weapon fire obstruction.
	virtual double ObstacleRadius() const
	{
		return 15.;
	}

	// Called by spacecraft to avoid objects.
	// Returns safe radius with a safety margin to avoid this object when navigating.
	// A return value of 0 means this atom is not avoided by spacecraft.
	virtual double PilotAvoidRadius() const
	{
		return 25. * FeetPerMeter();
	}

	virtual bool RoughExtents(AuBox &fExt) const;

	inline void SetDriverOnLine(bool isOnLine)
	{
		m_DriverOnLine = isOnLine;
	}

	virtual void SetEmpireId(quint32 idEmpire);

	virtual void GetRenderLightsRecursive(const DaPerson& person, const AuMatrix4x4& model, QList<AuGLSun>& suns,
			QList<AuGLLight>& lights) const;

	inline void SetParked(bool parked)
	{
		m_Parked = parked;
	}

	inline void SetMotherShip(const AuId& shipId, const QString& shipName)
	{
		m_VehicleLock.lockForWrite();
		m_MotherShip = shipId;
		m_MotherShipName = shipName;
		m_MotherShipChg.Set();
		m_VehicleLock.unlock();
	}

	inline bool CanPitchAndRoll() const
	{
		if(IsLaunching())
			return false;
		if(IsParalyzed())
			return false;
		switch(m_cid)
		{
		case CID_AdvSpaceFighter:
		case CID_SpaceFighter:
		case CID_SpaceTransport:
			if(!OnGround())
				return true;
			// On the ground.
			if(IsEngineOn() && Speed() > 50.*5280./3600.)
				return true;	// fast enough to fly away from the ground
			break;
		case CID_SpaceRocket:
		case CID_Helicopter:
			if(!OnGround())
				return true;
			break;
		case CID_Submarine:
			if(IsInWater() && !IsOnGround())
				return true;
			break;
		default:
			break;
		}
		return false;
	}

	inline bool CanTurn() const
	{
		if(IsLaunching())
			return false;
		if(IsParalyzed())
			return false;
		switch(m_cid)
		{
		case CID_DirtBike:
			return true;

		case CID_SailBoat:
			// Air required
			if(AirDensity() > 0)
				return true;
			break;

		case CID_SpaceRocket:
		case CID_Helicopter:
			// When not on ground, can turn with power or atmosphere density.
			if(!IsOnGround() && (IsEngineOn() || AirDensity() > .1f))
				return true;
			break;

		case CID_AdvSpaceFighter:
		case CID_SpaceFighter:
		case CID_SpaceTransport:
			// Can always turn with power.
			if(IsEngineOn())
				return true;
			// When not on ground, can turn with power or atmosphere density.
			if(!IsOnGround() && (IsEngineOn() || AirDensity() > .1f))
				return true;
			break;

		case CID_Submarine:
			// In water or on ground, can turn when moving or engine is on.
			if(IsInWater() || IsOnGround())
				if(IsEngineOn() || fabs(ForwardSpeed()) > 1)
					return true;
			break;

		case CID_APC:
			// Can always turn with power.
			if(IsEngineOn() || fabs(ForwardSpeed()) > 1)
				return true;
			break;

		case CID_SUV:
		case CID_FarmMachinery:
			// SUV requires forward or backward movement to turn
			if(fabs(ForwardSpeed()) > 1)
				return true;
			break;

		default:
			break;
		}
		return false;
	}
	inline COMMODITYID CommodityId() const
	{
		return m_cid;
	}

	inline bool HasLight() const
	{
		switch(m_cid)
		{
		case CID_SailBoat:
			return false;
		case CID_DirtBike:
		case CID_SpaceFighter:
		case CID_SpaceTransport:
		case CID_AdvSpaceFighter:
		case CID_SpaceRocket:
		case CID_Helicopter:
		case CID_Submarine:
		case CID_SUV:
		case CID_APC:
		case CID_FarmMachinery:
			return true;
		default:
			return false;
		}
	}

	inline bool IsEnclosed() const	// For audibility determination
	{
		switch(m_cid)
		{
		case CID_DirtBike:
		case CID_SailBoat:
		case CID_FarmMachinery:
			return false;
		case CID_AdvSpaceFighter:
		case CID_SpaceFighter:
		case CID_SpaceTransport:
		case CID_SpaceRocket:
		case CID_Helicopter:
		case CID_Submarine:
		case CID_SUV:
		case CID_APC:
			return true;
		default:
			return false;
		}
	}

	inline bool IsTwoWheeler() const
	{
		switch(m_cid)
		{
		case CID_DirtBike:
			return true;
		default:
			return false;
		}
	}

	virtual bool IsSightTarget(const DaAtom* toWho) const;

	inline bool MustMoveToTurn() const
	{
		switch(m_cid)
		{
		case CID_SUV:
		case CID_FarmMachinery:
			return true;
		case CID_APC:
		case CID_DirtBike:
		case CID_SailBoat:
		case CID_AdvSpaceFighter:
		case CID_SpaceFighter:
		case CID_SpaceTransport:
		case CID_SpaceRocket:
		case CID_Helicopter:
		case CID_Submarine:
		default:
			return false;
		}
	}

	// Used for animal perception of this vehicle.
	inline bool IsFlying() const
	{
		if(!IsOnGround() && !IsInWater())
			return true;
		return false;
	}

	inline bool IsNew() const
	{
		return (m_Switches & SWITCH_Used) ? false : true;
	}

	inline bool IsSubmersible() const
	{
		switch(m_cid)
		{
		case CID_Submarine:
			return true;
		case CID_APC:
		case CID_AdvSpaceFighter:
		case CID_DirtBike:
		case CID_FarmMachinery:
		case CID_Helicopter:
		case CID_SUV:
		case CID_SailBoat:
		case CID_SpaceFighter:
		case CID_SpaceRocket:
		case CID_SpaceTransport:
		default:
			return false;
		}
	}

	inline bool IsWaterCraft() const
	{
		switch(m_cid)
		{
		case CID_SailBoat:
		case CID_Submarine:
			return true;
		case CID_APC:
		case CID_AdvSpaceFighter:
		case CID_DirtBike:
		case CID_FarmMachinery:
		case CID_Helicopter:
		case CID_SUV:
		case CID_SpaceFighter:
		case CID_SpaceRocket:
		case CID_SpaceTransport:
		default:
			return false;
		}
	}

	// Used for animal perception.
	inline float NoiseLevel() const	// 0..1
	{
		if(IsEngineOn())
			return 1.f;	// Engine on
		else if(!Vector().IsFuzzyZero())
			return .5f;	// Moving
		return 0.f;
	}

	virtual void EnumControlInputs(DiControlInputArray &) const;

	void ConfigureControls(const DiControlInputArray &aAllControls);

	void LoadControls() const;

	void SaveControls() const;

	void RestoreDefaultControls();

	virtual DaAtom* SightTarget(const DaAtom* sightingAtom, const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, DaAtom* pBest,
			AuPoint& sightedLoc, unsigned int& sightedPart, bool& sightedShield, AuPoint* sightedSurfaceNormal) const;

	// User interface functions.
	virtual bool IsControl(const DaAtom* sceneAtom) const;

	virtual void EnumControlPanels(AuQuadArray &aControlPanels, const AuMatrix4x4 &tModel, QVector<qint16>& idPanels,
			const DaAtom* sceneAtom) const;

	virtual void EnumViewPanelControls(qint16 panelId, QList<AuControlButton>& controls, AuAtomRenderInfo& ari) const;

	virtual void ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson *pSceneAtom, const AuGearPos& gearPos,
			qint8 chSelValue, CONTROL_CONTEXT cc, int msElapsed);

	// 'blips' is the sensor blip array that is accumulating the scan data. Locations are relative to the solar system.
	// 'scannerLoc' is the location of the atom who is performing the scan, relative to the solar sytem.
	// 'scannerId' is the id of the atom who is performing the scan. Currently either a ship or a city.
	// 'scannerEmpire' is the empire id of the atom who is performing the scan.
	// 'scannerSensorRange' is the range in feet of the scanner atom's sensors.
	// 'scannerWeaponRange' is the range in feet of the scanner atom's bay/base weapons.
	// 'scanContent' is a bitmask of data that is sought in the scan
	//   b0 = sensor console blips
	//   b1 = fire control console blips
	//   b2 = transporter console blips
	//   b3 = locked-on blip
	//   b4 = AI pilot obstacle blips
	//   b5 = AI turret gunner target blips
	//   b6 = Scanning spacecraft wants its own subatoms to be scanned.
	// 'lockedBlip' is the id of the currently locked sensor blip
	virtual void SensorScan(AuSensorBlipList& blips, const AuPoint& scannerLoc, const AuId& scannerId, quint32 scannerEmpire,
			int scannerDomain, const AuId& scannerWorld,
			float scannerSensorRange, float scannerWeaponRange, int scanContent, const AuId& lockedBlip,
			unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash>& stanceCache) const;

	static DaVehicle* LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad=0);

	virtual void Insert(QSqlDatabase& db) const;	// Insert first time db record.

	virtual void Update(QSqlDatabase& db) const;	// Update existing db record.

	inline void UpdateDriverOnLine()
	{
		SetDriverOnLine(IsControllerOnLine());
	}

	// Physics
	// Acceleration in ft/sec/sec.
	virtual AuPoint Acceleration() const;

	// In square feet. Used by drag calculation.
	// X is lateral
	// Y is forward
	// Z is vertical
	// See http://www.newton.dep.anl.gov/askasci/phy99/phy99x88.htm
	virtual AuPoint CrossSectionalArea() const;

	// Density is mass (m) per unit volume (V).
	// Density is the ratio of the amount of matter in an object compared to
	// its volume.
	//  p = m / V
	// p is the density of the substance, measured in kg/m^3
	// e.g.
	//  Water ranges from 999.8425 at 0C to 958.3665 at 100C
	//  Air ranges from 1.342 at -10C to 1.293 at 0C to 1.164 at 30C
	virtual float Density() const;

	// Used by drag calculation. Returns 0 < DC <= 1
	// See http://www.newton.dep.anl.gov/askasci/phy99/phy99x88.htm
	virtual float DragCoefficient() const;

	virtual void FilterFinalMoveVector(AuPoint& vector) const;

	virtual void FilterPostMoveVector(AuPoint& vector) const;

	// Returns friction in ft/sec/sec, typically due to surface contact.
	virtual AuPoint Friction() const;

	// Returns the mass of this atom, in pounds.
	virtual float Mass() const
	{
		return Mass(m_cid);
	}

	// Returns the mass of a vehicle commodity, in pounds.
	inline static float Mass(COMMODITYID vehicleCID)
	{
		switch(vehicleCID)
		{
		case CID_DirtBike:
			return 230.f;
		case CID_SailBoat:
			return 320.f;
		case CID_AdvSpaceFighter:
			return 12000.f * (float)PoundsPerKilogram();
		case CID_SpaceFighter:
			return 10000.f * (float)PoundsPerKilogram();
		case CID_SpaceTransport:
			return 25000.f * (float)PoundsPerKilogram();
		case CID_SpaceRocket:
		case CID_Submarine:
			return 10000.f * (float)PoundsPerKilogram();	//DHH For now
		case CID_Helicopter:
			return 2500.f * (float)PoundsPerKilogram();
		case CID_SUV:
		case CID_FarmMachinery:
			// From Wikipedia weight and dimensions for H2.
			return 2903.f * (float)PoundsPerKilogram();

		case CID_APC:
			return 30000.f * (float)PoundsPerKilogram();

		default:
			break;
		}
		return 0;
	}

	// Orient and OrientationSpeed have no effect unless this function returns true.
	virtual bool OkToOrient() const;

	// If this atom is "at rest", its rotation is changed immediately and true is returned.
	// If this atom is not "at rest", its m_OrientRotation is set and false is returned.
	// Always returns false if parent is a vehicle or an animal.
	virtual bool Orient(const AuPoint& atLoc, const AuPoint& terNormal);

	virtual double OrientationSpeed() const;

	virtual void SetAtRest();

	inline void SetUsed()
	{
		m_Switches |= SWITCH_Used;
	}

	inline void SwitchEngineOff()
	{
		m_Switches &= ~SWITCH_Engine;
	}

	inline void SwitchEngineOn()
	{
		m_Switches |= SWITCH_Engine;
	}

	inline void SwitchEngineToggle()
	{
		m_Switches ^= SWITCH_Engine;
	}

	inline void SwitchHoldOff()
	{
		m_Switches &= ~SWITCH_Hold;
	}

	inline void SwitchHoldOn()
	{
		m_Switches |= SWITCH_Hold;
	}

	inline void SwitchHoldToggle()
	{
		m_Switches ^= SWITCH_Hold;
	}

	inline void SwitchLightOff()
	{
		m_Switches &= ~SWITCH_Light;
	}

	inline void SwitchLightOn()
	{
		m_Switches |= SWITCH_Light;
	}

	inline void SwitchLightToggle()
	{
		m_Switches ^= SWITCH_Light;
	}

	inline void SwitchOrbitRingsOff()
	{
		m_Switches &= ~SWITCH_OrbitRings;
	}

	inline void SwitchOrbitRingsOn()
	{
		m_Switches |= SWITCH_OrbitRings;
	}

	inline void SwitchOrbitRingsToggle()
	{
		m_Switches ^= SWITCH_OrbitRings;
	}

protected:
	void ApplyRotation(unsigned int msElapsed);

	void ChangeSeats(DaPerson* occupant, int fromSeat, int toSeat);

	void DecayMoveActions();

	// Returns density in pounds per cubic foot (lbs / ft^3)
	// Density is increased by armor and streamlining.
	static inline float Density(int armorLevel, bool isStreamlined)
	{
		// Density is increased by armor and streamlining.
		return DensityOfVehicle() * DensityFactor(armorLevel, isStreamlined);
	}

	// Density factor is increased by armor and streamlining.
	// Returns a density factor that is the actual density / nominal density.
	// The range of return values is [1 - 2.2].
	static inline float DensityFactor(int armorLevel, bool isStreamlined)
	{
		// Streamlining induces a 10% increase in denstiy.
		float densityFactor;
		if(isStreamlined)
			densityFactor = 1.1f;
		else
			densityFactor = 1;

		// The base density is linearly modified by armorLevel up to twice the
		// base density, assuming an armorLevel range of 0 to 10.
		return densityFactor + densityFactor * (float)armorLevel / 10.f;
	}

	void EnumSightSurfaces(AuQuadArray &solidSurfs, const AuMatrix4x4 &tModel) const;

	// Returns true if the gun fires.
	bool FireGun(const DaCreature* gunner);

	// Returns true if the weapon fires.
	bool FireWeapon(const DaCreature* gunner);

	bool IsSeatOccupied(int seat, bool onLineOnly=false) const;

	// Lists all occupants, no matter the atom type. i.e. people and items.
	// Lists indices of seats that are occupied by any atom.
	void ListSeatsOccupied(QList<int>& occupiedSeats) const;

	// Lists seat indices of people who are on line right now.
	// Ignores non-person atoms.
	void ListSeatsOccupiedByPeopleOnLine(QList<int>& occupiedSeats) const;

	void RemoteDoorSwitch(DaPerson* sceneAtom);

	void RenderAudioSources(const DaPerson* sceneAtom, const AuMatrix4x4& model, AuSound* al, float fade);

	void RequestLaunch(DaPerson* sceneAtom);

	void RequestRecovery(DaPerson* sceneAtom);

	// 'sightedPart' will be the index into the array of surfaces returned by EnumSightSurfaces.
	// Returns true if this atom is hit and 'sightedLoc' and 'sightedPart' are output.
	bool SightTargetDetails(const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, AuPoint& sightedLoc, unsigned int& sightedPart,
			bool& sightedShield) const;

	float TurnRate() const;			// Turn rate in radians per real world second. Adjusted for damage.
};

#endif // DaVehicle_H
