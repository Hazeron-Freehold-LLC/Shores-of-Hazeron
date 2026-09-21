//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dacreature.h,v 1.493 2022/05/17 16:15:14 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaCreature_H
#define DaCreature_H

#include "auanimalpart.h"
#include "audnabase.h"
#include "auhair.h"
#include "austation.h"
#include "aupose.h"
#include "daunit.h"
#include "dpbody.h"

class AuAnimalia;
class AuGear;
class AuItem;
class AuOrdnance;
class AuQuad;
class DaItem;
class DaVehicle;
class DpStation;

#define c_CreatureSightRange			(1000*FeetPerMeter())	// Maximum distance targets can be sighted.
#define CREATURE_SOUNDS					3

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL DaCreature : public DaUnit
{
protected:
	quint8								m_AirInLungs;			// Seconds of air in lungs before suffocating in vacuum.
	quint8								m_BodyQ;				// Current quality of my body. Changes based on foods I eat.
	quint16								m_ControlId;			// Subsystem controlled by my current console. e.g. Fire control bay
	quint8								m_MinutesToFullGrown;	// Minutes until fully grown when born from implant.
	quint8								m_SeatIndex;			// Vehicle seat index when in a vehicle.
	quint8								m_Stamina;				// Amount of stamina remaining.

private:
	mutable QReadWriteLock				m_BodyLock;				// Guards m_Body vars.
	AuAnimalia*							m_Animalia;				// Constructed from DNA. Only DNA is serialized.
	DpBody*								m_Body;					// Constructed from DNA. Only DNA is serialized.
	AuHairStyle::STYLE					m_Haircut;
	AuHairStyle::SHAVE					m_Shave;							
	quint16								m_StationId;			// Part id in AuScDesign.
	AuElapsedTimer						m_tmBodyChange;
	AuElapsedTimer						m_tmHairChange;			// Hair or shave change time.
	AuElapsedTimer						m_tmHeadChange;
	AuElapsedTimer						m_tmHPChange;
	unsigned int						m_MouthAnimDur;			// Mouth animation duration. 0 if this is not a factor.
	AuElapsedTimer						m_MouthAnimStart;		// When last mouth animation started or restarted.
	AuVisage							m_MouthFinish;			// Pose to put the mouth in after it finished animating.
	AuPose								m_MouthStationary;		// Mouth pose after animating. This is the last stationary pose set.
	AuElapsedTimer						m_tmPoseChange;

private:
	mutable QReadWriteLock				m_HungerLock;			// Guards m_Hunger vars.
	quint16								m_Hunger;				// Amount of hunger remaining.
	AuElapsedTimer						m_tmHungerChange;

private:
	mutable	QReadWriteLock				m_ConsoleLock;
	DpStation*							m_Console;				// Console at which this creature is sitting when in console pose.

private:
	mutable	QReadWriteLock				m_ItemInHandLock;		// This gets used on the server when setting my pose.
	AuItem*								m_ItemInHand;			// Information about the current item in hand, hvy wpn in turret.

protected:
	// Temporary, not stored.
	AuItem*								m_Ordnance;				// Info about ordnance|bay, if I am operating one. Scene atom only.

private:
	// OpenAL
	unsigned int						m_AudioCreature[CREATURE_SOUNDS];	// sound sources
	double								m_RenderedHeadTurn;			// non-zero if rendered at a stationary station.
	int									m_Sound[CREATURE_SOUNDS];	// current SOUND_EFFECT

public:
	// Avatar
	DaCreature(const AuDNABase& dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand, const QString& name,
			quint32 idEmpire, GROWTHSTAGE stage);
	// Citizen
	DaCreature(const AuDNABase& dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand, const QString& name,
			GROWTHSTAGE stage);
	DaCreature(const AuSpecies& species, AuDNABase::CHROMATID chroma, HANDEDNESS hand, const QString& name,
			GROWTHSTAGE stage);
	// Animal
	DaCreature(const AuSpecies& species, AuDNABase::CHROMATID chroma, HANDEDNESS hand, GROWTHSTAGE stage);
	DaCreature();
	virtual ~DaCreature();

protected:
	DaCreature(const DaCreature &rhs);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	// Add a visual and/or audio effect to this atom.
	// 'effect' may be modified to something appropriate. e.g. VISUAL_HitPierce (looks like blood) is changed to VISUAL_HitPierceBug
	// when this atom is an exoskeletal creature, and something else when this atom is not even a creature.
	virtual void AddEffect(AuEffect& effect);

	// Queues any damage using CombatApplyDamageLater.
	void ApplyAtmosphericDamage();

	// Queues any damage using CombatApplyDamageLater.
	void ApplyLiquidDamage();

	// Return the commodity id of the armor or clothing that is outermost for rendering of appearance.
	virtual COMMODITYID Armor() const
	{
		return CID_Invalid;
	}

	float BiteRange() const;

	inline quint8 BodyQ() const
	{
		return m_BodyQ;
	}

	// Called when my body has been restored to consciousness by defibrillation.
	virtual void Defibrillated(quint32 byEmpireId);

	// Returns false if I have no air left in my lungs.
	inline bool HoldBreath()
	{
		if(!m_AirInLungs)
			return false;
		m_AirInLungs--;
		return true;
	}

	// Refills my lungs.
	inline void Inhale()
	{
		if(m_AirInLungs < LungCapacity())
			m_AirInLungs++;
	}

	static inline quint8 LungCapacity()
	{
		return 3;
	}

	// Returns current weight of burden in lbs.
	// Includes weight of riders' burden when mounted.
	// As creatures do not carry inventory, this base class only returns the combined burdens of riders, if any.
	virtual float Burden() const;

	// Returns maximum burden this person can carry in lbs.
	float BurdenMax() const;

	virtual bool DoGetRoomUp() const;

	// Returns the vector from my atom origin to the eye camera location of my body model.
	// Beware that on the server the HeadOffset is returned, which differs from the client.
	// This works for the server for aiming purposes, since the server does not load head geometry, but the client often needs the
	// exact eye camera location.
	virtual AuPoint CameraOffset() const;

	bool CanGrabLadderOnCapsule(DaCapsule* ship) const;

	virtual bool CanGrabLadderOnMyBd() const;

	virtual bool CanGrabLadderOnMyShip() const;

	// Returns the vector from my atom location to the CG of my body model.
	// In a typical standing pose, this would be the reverse of the CameraOffset.
	// In a straddle pose, the CG is at my body's pivot point so I don't move wierdly when looking up and down on a mount.
	virtual AuPoint CGOffset() const;

	// Chooses a haircut and shave style for myself.
	virtual void ChooseHairStyle()
	{
		// This base class sets them to Unkempt and Unshaven.
		SetHairStyle(AuHairStyle::Unkempt, AuHairStyle::Unshaven);
	}

	AuDNABase::CHROMATID Chromatid() const;

	// Returns true if my current console is an integrated station.
	inline bool ConsoleIsIntegrated() const
	{
		return CurrentGeneralPose().IsIntegratedStation();
	}

	// Returns console occupant position based on my current pose.
	// Returns AuStation::Stand if not at a station.
	AuStation::OCCUPANT_POSITION ConsoleOccupantPosition() const;

	// Returns seat offset based on current pose.
	inline AuPoint ConsolePoseOffset() const
	{
		return ConsolePoseOffset(ConsoleType(), ConsoleOccupantPosition(), CGHeightStanding(), TorsoThickness(),
				ConsoleSeatIndex());
	}

	// Returns seat offset required when entering a station. For example, standing stations must adjust upward so this creature
	// lands there on its feet, instead of its cg.
	static AuPoint ConsolePoseOffset(AuStation::TYPE consoleType, AuStation::OCCUPANT_POSITION occPos, float cgHeightStanding,
			float torsoThickness, int seatIndex=0);

	// Needed for Sc type stations. Returns true on success.
	bool ConsoleScreenQuad(AuQuad& screen) const;

	// Returns console seat index based on my current pose.
	// Returns 0 if not at a station.
	inline unsigned int ConsoleSeatIndex() const
	{
		return CurrentGeneralPose().ConsoleSeatIndex();
	}

	//----- Station Identification
	// Station identification pertains to the station itself, at which I am actually sitting, not viewing via a command chair.
	// Station identification does not change when changing the station viewed at a command station.

	// Returns 0 if not at a console.
	inline quint16 StationId() const
	{
		m_BodyLock.lockForRead();
		quint16 stationId = m_StationId;
		m_BodyLock.unlock();
		return stationId;
	}

	// This is the system id of my console.
	inline AuSysId StationSysId() const
	{
		return AuSysId(StationSystem(), StationId());
	}

	// Returns only SYS_CommandAndControl, SYS_Turret, or SYS_Count when not at a console.
	SYSTEM StationSystem() const;

	//----- Station Control Identification

	// Stations sometimes control other systems.
	// e.g. Fire control console can operate different weapon systems.
	inline quint16 ControlId() const
	{
		return m_ControlId;
	}

	// This is the system id of a console controlling ControlId, not the subsystem itself, such as with a weapon system.
	// This is used when testing if the station running the controlled system is operational.
	inline AuSysId ControlSysId() const
	{
		return AuSysId(StationSystem(), ControlId());
	}

	//-----

	// Adjust camera location for a subatom.
	// 'subatom' is one of my subatoms.
	// 'camera' is the matrix to the camera relative to the subatom.
	// Returns true if 'camera' is modified.
	virtual bool ConstrainCamera(const DaAtom* subatom, AuMatrix4x4& camera) const;

	// Body cycle speed based on current pose, body speed, and encumberance.
	float CycleSpeed() const;

	// Dismounts this atom from the creature that it is riding.
	void DismountCreature();

	// Called when I have been dismounted by a creature.
	virtual void Dismounted();

	// Dismounts this atom from the vehicle that it is riding.
	// Returns false if unable to dismount vehicle due to obstructions or because my parent atom is not a vehicle.
	bool DismountVehicle();

	void DismountAllRiders();

	//DaItem* DropItem(const AuItem& item);

	DaItem* DropItem(const AuItem& item, const AuPoint& offsetVec=AuPoint(0,0));

	virtual void EmpireDeleted(quint32 empireId);

	virtual double FallSafeSpeed() const;

	virtual AuId Father() const
	{
		return AuId::NullId();
	}

	// Field of view angle is 1/2 the full field of view.
	float FieldOfViewAngle() const;

	int FightReflex() const;

	// Find atoms that might cause shields to be raised.
	virtual void FindClosestShipThreatRecursive(const AuId& shipId, int shipDomain, const AuId& shipWorldId, quint32 shipEmpire,
			SHIELD_POLICY shieldPolicy, const AuPoint& shipSysLoc, float sensorRange, const DaAtom*& targetAtom,
			double& targetRange) const;

	virtual DaCreature* FindCreatureRecursive();

	virtual double FireRadius() const;

	int FlightReflex() const;

	// Grabs the ladder at my location.
	// Tries to grab any ladder within reach. 1) On my ship, 2) On my world. 3) On ships in my environment.
	// Returns true on success, changes my body pose, and moves me to the ladder.
	bool GrabLadder();

	// Returns true on success.
	// Returns false if I am not on a bd.
	// Returns false if my bd's CanGrabLadder function returns false.
	// Returns false if the path from my current eye position to my nu eye position on the ladder is blocked.
	bool GrabLadderOnMyBd();

	// Returns true on success.
	// Returns false if I am not on a ship.
	// Returns false if my ship's CanGrabLadder function returns false.
	// Returns false if the path from my current eye position to my nu eye position on the ladder is blocked.
	bool GrabLadderOnMyShip();
	bool GrabLadderOnMyShipAt(const AuPoint& ladderLoc);

	// I am not aboard 'ship'. It could be anywhere in the environment.
	// Returns true on success.
	// Returns false if ship->CanGrabLadder returns false.
	// Returns false if the path from my current eye position to my nu eye position on the ladder is blocked.
	bool GrabLadderOnCapsule(DaCapsule* ship);

	// Creatures grow by reducing their MinutesToFullGrown time. Called once per minute.
	// Return false if this creature should not grow this minute.
	virtual bool GrowNaturally() const
	{
		if(IsFullGrown())
			return false;	// Creatures stop growing when they are fully grown.
		if(!IsInAnyScene())
			return false;	// Creatures don't grow when nobody is around.
		return true;
	}

	AuPoint GrowthScaleBody() const;

	// Returns a non-zero number for scaling this creature when it has emerged from an implant and is in its "growth" state.
	inline float GrowthScale() const
	{
		return GrowthScale(m_MinutesToFullGrown, GrowthTime());
	}

	// Returns a non-zero number for scaling this creature when it has emerged from an implant and is in its "growth" state.
	static inline float GrowthScale(quint8 minutesToFullGrown, quint8 growthTime)
	{
		// Calculate non-zero scale according to growth factor.
		if(minutesToFullGrown && growthTime)
		{
			float scale = (float)(growthTime - (minutesToFullGrown - 1)) / (float)growthTime;
			//if(scale < 1.f/5.f)
				//scale = 1.f/5.f;
			//return scale;

			// Scale ranges from .2 to 1, so bodies of babies aren't tiny. They will be heavy, though.
			return .2f + (scale * .8f);
		}
		return 1;
	}

	inline GROWTHSTAGE GrowthStage() const
	{
		return IsFullGrown() ? Mature : Juvenile;
	}

	// Returns the number of minutes this creature needs to grow when it has emerged from an implant or been born and is in its
	// "growth" state.
	// The return value is in the range [7..255].
	quint8 GrowthTime() const;

	inline AuHairStyle::STYLE Haircut() const
	{
		return m_Haircut;
	}

	HANDEDNESS Handedness() const;

	// Returns true if this creature is using a generation 3 DpBody/DhDNA body.
	inline bool HasDNA3() const
	{
		m_BodyLock.lockForRead();
		bool v = m_Body;
		m_BodyLock.unlock();
		return v;
	}

	// Returns true if this creature is using a generation 4 AuAnimalia/AuDNA body.
	inline bool HasDNA4() const
	{
		m_BodyLock.lockForRead();
		bool v = m_Animalia;
		m_BodyLock.unlock();
		return v;
	}

	inline bool HasItemInHand() const
	{
		return m_ItemInHand ? true : false;
	}

	// Checks for an item in my dominant hand.
	bool HasItemInHand(COMMODITYID cid) const;

	// Checks for a specific item in my dominant hand.
	// 'inscription' is only compared if 'inscription' is not blank.
	bool HasItemInHandSpecific(COMMODITYID cid, quint8 quality, quint32 empireOfOrigin, const QString& inscription) const;

	// Returns the vector from my atom origin to the base of my skull.
	virtual AuPoint HeadOffset() const;

	float HearingRange() const;

	// Returns the commodity id of the helmet only if the visor is closed. This is used to test for current helmet that is
	// protecting from the environment.
	// Gen3 DNA creatures did not have a visor that opened and closed so CID_Invalid was returned when the visor was open. Thus this
	// became the test for which head to render in the scene, helmet (visor closed) or biological (visor open).
	// Npcs don't really don/doff their helmet nor do they open/close the visor. This function returns the current "apparent" helmet
	// based on various conditions, e.g. mating, surgery unit, unbreathable air, etc.
	virtual COMMODITYID Helmet() const
	{
		return CID_Invalid;
	}

	// 'destArc' is n/a when the destination system is not a ringworld.
	void HengeTeleport(const AuId& fromSysId, quint8 fromSun, quint8 fromArc, quint8 shadowPanels, const AuId& destSysId,
			quint8 destSun, quint8 destArc);

	inline AuElapsedTimer HungerChange() const
	{
		m_HungerLock.lockForRead();
		AuElapsedTimer t(m_tmHungerChange);
		m_HungerLock.unlock();
		return t;
	}

	inline void InitTimeToFullGrown()
	{
		m_MinutesToFullGrown = GrowthTime();
	}

	virtual bool IsAIControlled() const
	{
		return true;
	}

	virtual bool IsClimbingLadder() const
	{
		return IsLadderPose();
	}

	inline bool IsCommandStationPose() const
	{
		return CurrentGeneralPose().IsCommandStation();
	}

	bool IsEatingPose() const;

	inline bool IsEngineStationPose() const
	{
		return CurrentGeneralPose().IsEngineStation();
	}

	virtual bool IsEthereal() const
	{
		return false;
	}

	inline bool IsFireControlStationPose() const
	{
		return CurrentGeneralPose().IsFireControlStation();
	}

	// True if I am a flyer, regardless if current conditions permit flying.
	bool IsFlyer() const;

	inline bool IsFullGrown() const
	{
		return !m_MinutesToFullGrown;
	}

	virtual bool IsGuardTowerTarget(quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunLoc, float gunRange,
			AuPoint& targetLoc) const;

	inline bool IsHelmStationPose() const
	{
		return CurrentGeneralPose().IsHelmStation();
	}

	bool IsInBerth() const;

	virtual bool IsInScene(const DaPerson* sceneAtom, const AuPoint& cameraEnvLoc) const;

	virtual bool IsMarried() const
	{
		return false;
	}

	// true if this creature is currently boinking another creature, either on top or on the bottom, base on my body pose.
	bool IsMating() const;

	inline bool IsMedicStationPose() const
	{
		return CurrentGeneralPose().IsMedicStation();
	}

	inline bool IsMovingFast() const
	{
		return CurrentGeneralPose().IsFastMoving();
	}

	bool IsMovingPose() const;

	inline bool IsNavStationPose() const
	{
		return CurrentGeneralPose().IsNavStation();
	}

	virtual bool IsObstacle(const DaAtom* toWho) const;

	virtual bool IsPathBlocked(const AuPoint& oloc, const AuPoint& nloc, AuPoint& hitLoc, AuPoint& hitNormal, const DaAtom* toWho);

	inline bool IsPowerStationPose() const
	{
		return CurrentGeneralPose().IsPowerStation();
	}

	virtual bool IsReadyForMating() const;

	inline bool IsSensorStationPose() const
	{
		return CurrentGeneralPose().IsSensorStation();
	}

	bool IsSexuallyCompatible(const DaCreature& creature) const;

	inline bool IsShieldStationPose() const
	{
		return CurrentGeneralPose().IsShieldStation();
	}

	virtual bool IsShipTurretTarget(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
			CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
			AuPoint& targetSysLoc) const;

	inline bool IsTransporterStationPose() const
	{
		return CurrentGeneralPose().IsTransporterStation();
	}

	virtual bool IsSightTarget(const DaAtom* toWho) const;

	inline bool IsPatientPronePose() const
	{
		return CurrentGeneralPose().IsPatientProne();
	}

	inline bool IsPatientStationPose() const
	{
		return CurrentGeneralPose().IsPatientStation();
	}

	// Returns true if this creature is fully suited up for harsh environments, including helmet visor closed.
	// Equipment condition must not be leaking.
	virtual bool IsProtectedByEnvironmentSuit() const
	{
		return false;
	}

	// Returns true if this creature appears to be fully suited up for harsh environments, including helmet visor closed.
	// Equipment condition is not tested, could be leaking.
	inline bool IsWearingEnvironmentSuit() const
	{
		// My Helmet and Armor functions return what I "appear" to be wearing, based on environmental conditions and visor state.
		// They will return appropriate gear when necessary, if I have it.
		// Condition of the equipment is ignored.
		if(!DbCommodity::IsEnvironmentHelmet(Helmet()))
			return false;										// I don't appear to be wearing an environment helmet.
		if(!DbCommodity::IsEnvironmentSuit(Armor()))
			return false;										// I don't appear to be wearing an environment suit.
		return true;
	}

	// Creatures with no empire id are considered to be wild.
	inline bool IsWild() const
	{
		return !EmpireId();
	}

	// Returns the axis down which the item in hand is aimed. 0=positive X, 1=positive Y
	inline int ItemInHandAimAxis() const
	{
		int v;
		m_ItemInHandLock.lockForRead();
		if(m_ItemInHand)
			v = m_ItemInHand->AimAxis();
		else
			v = 0;
		m_ItemInHandLock.unlock();
		return v;
	}

	inline COMMODITYID ItemInHandCID() const
	{
		COMMODITYID v;
		m_ItemInHandLock.lockForRead();
		if(m_ItemInHand)
			v = m_ItemInHand->CommodityId();
		else
			v = CID_Invalid;
		m_ItemInHandLock.unlock();
		return v;
	}

	float JumpForce() const;

	bool LivesInAir() const;

	bool LivesOnLand() const;

	// True if this creature looks like it could be a rat.
	bool LooksLikeVermin() const;

	// Mounts a creature for mating.
	void MateWithCreature(DaCreature* creature);

	inline quint8 MinutesToFullGrown() const
	{
		return m_MinutesToFullGrown;
	}

	virtual AuId Mother() const
	{
		return AuId::NullId();
	}

	// Mounts a creature for riding.
	void MountCreature(DaCreature* creature);
	void ForceMountCreature(DaCreature* creature);	// Result forced upon client.

	// Mounts a vehicle for riding.
	// Returns seat index on success, -1 on failure because all seats are occupied.
	int MountVehicle(DaVehicle* vehicle);
	int ForceMountVehicle(DaVehicle* vehicle);		// Result forced upon client.

	inline AuPose MouthStationaryPose() const
	{
		m_BodyLock.lockForRead();
		AuPose v(m_MouthStationary);
		m_BodyLock.unlock();
		return v;
	}

	// Returns info about my current ordnance, if I am operating a turret or other heavy weapon.
	// CAVEAT: This is transient data that is only available on the scene atom on the client for screen icon display and ordnance
	//         loading UI. It is only present when the scene atom is in a turret or operating some kind of heavy weapon.
	inline const AuItem* Ordnance() const
	{
		return m_Ordnance;
	}

	// Orient and OrientationSpeed have no effect unless this function returns true.
	virtual bool OkToOrient() const;

	virtual bool KeepsAutomaticDoorsOpen() const;

	virtual bool OpensAutomaticDoors() const;

	// If this atom is "at rest", its rotation is changed immediately and true is returned.
	// If this atom is not "at rest", its m_OrientRotation is set and false is returned.
	// Always returns false if parent is a vehicle or an animal.
	virtual bool Orient(const AuPoint& atLoc, const AuPoint& terNormal);

	virtual double OrientationSpeed() const;

	bool PoseAttackUnderGround();

	bool PoseAttackUnderWater();

	bool PoseAttackHovering();

	bool PoseAttackFlying();

	bool PoseAttackStanding();

	// 'speedEnc' is a multiplier based on encumberance, to slow body movements. 1=natural speed, .5=half speed, 0=motionless.
	bool PoseBurrow(float speedEnc);

	bool PoseEatOffGround();

	inline AuElapsedTimer PoseTime() const
	{
		m_BodyLock.lockForRead();
		AuElapsedTimer v(m_tmPoseChange);
		m_BodyLock.unlock();
		return v;
	}

	// Sets 'start' visage on the face. When visage duration is 0, animated poses will expire on their cycle time.
	// A visage that expires is changed to 'finish' when that happens.
	bool PoseVisage(const AuVisage& start, const AuVisage& finish);

	virtual bool Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari);

	// Returns camera id, if this creature is viewing through a remote camera, e.g. at a command station.
	quint16 RemoteCameraId() const;

	virtual AuPoint SceneCameraLocation() const;

	inline bool SetBodyQ(quint8 q)
	{
		if(q == 0)
			return false;	// Q0 is not valid.
		if(m_BodyQ == q)
			return false;
		m_BodyQ = q;
		return true;
	}

	// Changes the empire of the parent vehicle if this creature is the driver.
	virtual void SetEmpireId(quint32 idEmpire);

	// Render VAOs must be recreated after calling this.
	void SetHairStyle(AuHairStyle::STYLE cut, AuHairStyle::SHAVE shave);

	// Sets my current minutes before full grown.
	// Sets my current growth stage to Mature if 'minutes' is 0.
	// Sets my current growth stage to Juvenile if 'minutes' is not 0.
	void SetMinutesToFullGrown(quint8 minutes);

	inline AuHairStyle::SHAVE Shave() const
	{
		return m_Shave;
	}

	float SightRange() const;

	// Finds the atom that is nearest to this atom in this atom's look direction and in front of this atom's eye location.
	// 'shotgunRange' is the maximum range of the shotgun, whether or not it is loaded.
	// 'patternAngle' is the angle of spread of the pattern off the centerline. Full spread = 2x patternAngle.
	// Returns sighted atom, or 0 if none sighted.
	// When an atom is sighted:
	//   'sightedLoc' set to the 3D location on the atom that was sighted. Relative to this atom.
	//   'sightedPart' set to the atom-specific part that was sighted, as returned by SightTargetDetails.
	DaAtom* SightShotgunTarget(AuPoint& sightedLoc, unsigned int& sightedPart, bool& sightedShield, float shotgunRange,
			float patternAngle) const;

	float SmellRange() const;

	AuTime SpecialWeaponUseTime() const;

	AuSpecies Species() const;

	QString SpeciesName() const;

	virtual QString SpotDescription(quint32 spotterEmpire) const;

	// Returns the location to use when targeting this atom with a weapon.
	virtual AuPoint TargetLocation() const;

	// Returns throwing force, in foot pounds per second.
	float ThrowForce() const;

	// Sets my current seat index, applicable to my current vehicle.
	virtual void SetVehicleSeatIndex(quint8 seatIndex)
	{
		m_SeatIndex = seatIndex;
	}

	// Sends 'text' to this creature as a thought message.
	void Thought(const QString& text) const;

	virtual quint16 AtTransporterWhich() const;

	virtual bool IsTicker() const;

	virtual void Tick(const AuElapsedTimer& last, const AuElapsedTimer& now);

	virtual void TickMovement(const AuElapsedTimer& last);

	virtual unsigned int IsTocker() const;

	virtual void Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans);

	// Returns my seat index aboard my current vehicle, if I am aboard a vehicle.
	inline int VehicleSeatIndex() const
	{
		return m_SeatIndex;
	}

	// The name used when addressed by a superior.
	virtual QString CrewName() const
	{
		if(AssignedName().isEmpty())
			return("Creature");
		else
			return(AssignedName());
	}

	inline QString FemaleMale(bool upper) const
	{
		if(upper)
		{
			QString names[] =
			{
				tr("Female"),
				tr("Male"),
				tr("Asexual"),
				tr("Hermaphrodite"),
			};
			return names[Gender()];
		}
		else
		{
			QString names[] =
			{
				tr("female"),
				tr("male"),
				tr("asexual"),
				tr("hermaphrodite"),
			};
			return names[Gender()];
		}
	}

	inline QString MomDad(bool upper) const
	{
		if(upper)
		{
			QString names[] =
			{
				tr("Mom"),
				tr("Dad"),
				tr("Parent"),
				tr("Parent"),
			};
			return names[Gender()];
		}
		else
		{
			QString names[] =
			{
				tr("mom"),
				tr("dad"),
				tr("parent"),
				tr("parent"),
			};
			return names[Gender()];
		}
	}

	inline QString MotherFather(bool upper) const
	{
		if(upper)
		{
			QString names[] =
			{
				tr("Mother"),
				tr("Father"),
				tr("Parent"),
				tr("Parent"),
			};
			return names[Gender()];
		}
		else
		{
			QString names[] =
			{
				tr("mother"),
				tr("father"),
				tr("parent"),
				tr("parent"),
			};
			return names[Gender()];
		}
	}

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const
	{
		if(AssignedName().isEmpty())
			return("Creature");
		else
			return(AssignedName());
	}

	inline QString NieceNephew(bool upper) const
	{
		if(upper)
		{
			QString names[] =
			{
				tr("Niece"),
				tr("Nephew"),
				tr("Kinsperson"),
				tr("Kinsperson"),
			};
			return names[Gender()];
		}
		else
		{
			QString names[] =
			{
				tr("niece"),
				tr("nephew"),
				tr("kinsperson"),
				tr("kinsperson"),
			};
			return names[Gender()];
		}
	}

	inline QString HerHim(bool upper) const
	{
		if(upper)
		{
			QString names[] =
			{
				tr("Her"),
				tr("Him"),
				tr("Xer"),
				tr("Xim"),
			};
			return names[Gender()];
		}
		else
		{
			QString names[] =
			{
				tr("her"),
				tr("him"),
				tr("xer"),
				tr("xim"),
			};
			return names[Gender()];
		}
	}

	inline QString HerHis(bool upper) const
	{
		if(upper)
		{
			QString names[] =
			{
				tr("Her"),
				tr("His"),
				tr("Xer"),
				tr("Xis"),
			};
			return names[Gender()];
		}
		else
		{
			QString names[] =
			{
				tr("her"),
				tr("his"),
				tr("xer"),
				tr("xis"),
			};
			return names[Gender()];
		}
	}

	inline QString HersHis(bool upper) const
	{
		if(upper)
		{
			QString names[] =
			{
				tr("Hers"),
				tr("His"),
				tr("Xers"),
				tr("Xis"),
			};
			return names[Gender()];
		}
		else
		{
			QString names[] =
			{
				tr("hers"),
				tr("his"),
				tr("xers"),
				tr("xis"),
			};
			return names[Gender()];
		}
	}

	inline QString MaamSir(bool upper) const
	{
		if(upper)
		{
			QString names[] =
			{
				tr("Ma'am"),
				tr("Sir"),
				tr("Lord"),
				tr("Lord"),
			};
			return names[Gender()];
		}
		else
		{
			QString names[] =
			{
				tr("ma'am"),
				tr("sir"),
				tr("lord"),
				tr("lord"),
			};
			return names[Gender()];
		}
	}

	inline QString MissMister(bool upper) const
	{
		if(upper)
		{
			if(IsMarried())
			{
				QString names[] =
				{
					tr("Missess"),
					tr("Mister"),
					tr("Master"),
					tr("Master"),
				};
				return names[Gender()];
			}
			else
			{
				QString names[] =
				{
					tr("Miss"),
					tr("Mister"),
					tr("Master"),
					tr("Master"),
				};
				return names[Gender()];
			}
		}
		else
		{
			if(IsMarried())
			{
				QString names[] =
				{
					tr("mistress"),
					tr("mister"),
					tr("master"),
					tr("master"),
				};
				return names[Gender()];
			}
			else
			{
				QString names[] =
				{
					tr("miss"),
					tr("mister"),
					tr("master"),
					tr("master"),
				};
				return names[Gender()];
			}
		}
	}

	inline QString MsMr() const
	{
		if(IsMarried())
		{
			QString names[] =
			{
				tr("Mrs"),
				tr("Mr"),
				tr("Mx"),
				tr("Mx"),
			};
			return names[Gender()];
		}
		else
		{
			QString names[] =
			{
				tr("Miss"),
				tr("Mr"),
				tr("Mx"),
				tr("Mx"),
			};
			return names[Gender()];
		}
	}

	inline QString SheHe(bool upper) const
	{
		if(upper)
		{
			QString names[] =
			{
				tr("She"),
				tr("He"),
				tr("Xe"),
				tr("Xe"),
			};
			return names[Gender()];
		}
		else
		{
			QString names[] =
			{
				tr("she"),
				tr("he"),
				tr("xe"),
				tr("xe"),
			};
			return names[Gender()];
		}
	}

	// true if this creature can control its acceleration/deceleration
	inline bool CanControlMovement() const
	{
		if(IsBodyDyingOrDead() || IsInBerth() || IsPatientPronePose())
			return false;
		if(IsOnGround() ||				// Touching the ground. false when standing over a hatch that is open.
			 IsInWater() ||				// Wet, touching water.
			 IsInGravityWell() ||		// Gravity Well.
			 IsLadderPose() ||			// Clinging to a ladder.
			 InLadderWell() ||			// Close enough to a ladder to grab it.
			 CanFly() ||				// Can fly and air density permits it.
			 InDesignEnvironment())		// In a design environment.
			return true;
		return false;
	}

	bool CanFly() const;	// Considers atmosphere density and DNA.

	bool CanHover() const;	// Considers atmosphere density and DNA.

	// Jumper must
	// - not be at a console or in a turret
	// - not already be jumping
	// - not be moving too fast
	// - be touching something for friction
	bool CanJump() const;

	// Returns true if this creature is able to turn to a different facing.
	inline bool CanTurn() const
	{
		if(IsBodyDyingOrDead() || IsInBerth() || IsPatientPronePose())
			return false;
		if(IsLadderPose())
			return false;
		return true;
	}

	// Returns true if this creature is in liquid that is deep enough to swim or tread water instead of standing and walking.
	inline bool InDeepEnoughLiquidToSwim() const
	{
		return WaterDepth() >= CGHeightStanding();
	}

	bool IsExoskeletal() const;	// based on DNA

	bool IsLeaper() const;	// based on DNA

	bool LivesInWater() const;	// based on DNA

	bool LivesUnderGround() const;	// based on DNA

	// Return CG height of this atom in current pose.
	float CGHeight() const;

	// Return CG height of this atom in current pose, when fully grown. Ignores minutes to full grown.
	float CGHeightFullGrown() const;

	// Return CG height of a body in its current pose.
	// This is the Z distance from the base of the ground to the atom location.
	static float CGHeight(const AuAnimalia& body, quint8 minutesToFullGrown, quint8 growthTime, bool isAboardVehicle);

	static inline float CGHeight(const DpBody& body, quint8 minutesToFullGrown, quint8 growthTime, bool isAboardVehicle)
	{
		// Scale height according to growth factor.
		if(minutesToFullGrown)
			return body.CGHeight(true) * GrowthScale(minutesToFullGrown, growthTime) -
					CGOffset(body, minutesToFullGrown, growthTime, isAboardVehicle).z;
		return body.CGHeight(true) - CGOffset(body, minutesToFullGrown, growthTime, isAboardVehicle).z;
	}

	// Return CG height of this atom when "standing".
	virtual float CGHeightStanding() const;

	AuDNABase::ECOSTRATEGY EcoStrategy() const;

	ECOROLE EcoRole() const;

	GENDER Gender() const;

	bool IsAffectedByGravity() const;

	bool IsGameTablePose() const;

	// 'power' is a measure of how much of its power the creature is putting into the jump, in the range [0..1].
	// 'jumpVector' is the movement vector, based on the pose from which the jump was initiated.
	void Jump(float power, const AuPoint& jumpVector);

	virtual void RxUpdate(QDataStream& fromServer, bool isFirstUpdate);

	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
			const DaPerson& sceneAtom);

	void RxConsoleSettings(QDataStream& fromClient);

	virtual ATOM_TYPE AtomType() const;

	virtual QString Description() const;
	virtual float GroundHeightAboveCG() const;

	// Caller must delete the pointer that is returned, if it isn't zero.
	AuDNABase* DNAnew() const;

	inline DhDNA DNA3() const
	{
		m_BodyLock.lockForRead();
		DhDNA dna;
		if(m_Body)
			dna = m_Body->DNA();
		else
		{
			dna.InitDefaultHuman();
			qWarning() << "error: DaCreature::DNA3 has" << (m_Animalia ? "DNA4" : "No DNA");
		}
		m_BodyLock.unlock();
		return dna;
	}

	// Returns 3 for old style DhDNA, 4 for new style AuDNA.
	int DNAVersion() const;

	bool IsCrouched() const;

	// Returns true if my body is dead or dying. Call IsBodyDead to find out if I'm really dead or only mostly dead.
	inline bool IsBodyDyingOrDead() const
	{
		// The body pose is not used for this test because it may be carried by someone. In that case, the body is not in a dying
		// pose but in a straddle pose.
		return CurrentHitPoints() <= 0;
	}

	bool IsBodyDead() const;

	bool IsBodyDying() const;

	bool IsHerbivore() const;

	/*
	inline void SetDNA3(const DhDNA& dna)
	{
		m_BodyLock.lockForWrite();
		if(m_Body)
		{
			m_Body->SetDNA(dna, 1);
			m_tmBodyChange.Set();
		}
		m_BodyLock.unlock();
	}

	// This will change a DNA3 creature into a DNA4 creature.
	void SetDNA4(const AuDNA& dna);
	*/
	void SetDNA(const AuDNABase& dna);

	// Finds the atom that is nearest to this atom in this atom's look direction and in front of this atom's eye location.
	// Returns sighted atom, or 0 if none sighted.
	// When an atom is sighted:
	//   'sightedLoc' set to the 3D location on the atom that was sighted. Relative to this atom.
	//   'sightedPart' set to the atom-specific part that was sighted, as returned by SightTargetDetails.
	virtual DaAtom* SightTargetInSight(AuPoint& sightedLoc, unsigned int& sightedPart, bool& sightedShield) const;

	virtual DaAtom* SightTarget(const DaAtom* sightingAtom, const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, DaAtom* pBest,
			AuPoint& sightedLoc, unsigned int& sightedPart, bool& sightedShield, AuPoint* sightedSurfaceNormal) const;

	bool SightTargetDetails(const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, AuPoint& sightedLoc, unsigned int& sightedPart,
			bool& sightedShield) const;

	// Melee weapons are affected by strength and strength increases damage.
	int StrengthMod() const;

	/*
	inline float TorsoLength() const
	{
		m_BodyLock.lockForRead();
		float tl = m_Body.TorsoLength();
		m_BodyLock.unlock();
		return tl;
	}
	*/

	virtual void AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim);

	void AnimatePose(bool bendDominantArm, float speedEnc);

	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari);

	virtual void DeleteDeviceObjects(AuAtomRenderInfo& ari);

	virtual void RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
			const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass,
			DaAtomList& shadSc,
			DaAtomList& shadScFog,
			DaAtomList& shadScGlobe,
			DaAtomList& shadScGlobeFog,
			DaAtomList& shadScRingworld,
			DaAtomList& shadScRingworldFog);

	// 'pass' is 0 to render everything, 1 for first pass only, 2 for second pass only
	// 'renderLast' is only used when pass is 1.
	virtual void RenderItemInHand(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari, int pass,
			const DaAtom* sceneAtom, AuRenderLastMap* renderLast);

	virtual void RenderLast(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
			const DaPerson* sceneAtom,
			DaAtomList& shadSc,
			DaAtomList& shadScFog,
			DaAtomList& shadScGlobe,
			DaAtomList& shadScGlobeFog,
			DaAtomList& shadScRingworld,
			DaAtomList& shadScRingworldFog);

	virtual bool RoughExtents(AuBox &fExt) const;

	void SetupCamera(AuMatrix4x4& camera, double& nearClip, double& farClip, AuAtomRenderInfo& ari) const;

	// Physics
	// Acceleration in ft/sec/sec.
	virtual AuPoint Acceleration() const;

	virtual float Buoyancy(const AuQueryEnvironment& qe) const;

	// True if this atom is permitted to go below the surface of the terrain.
	// This could be any hard surface, such as ground terrain or the floor of
	// a spacecraft or building.
	virtual bool CanBeUnderGround() const;

	virtual AuPoint CrossSectionalArea() const;

	// Density is mass (m) per unit volume (V).
	// Density is the ratio of the amount of matter in an object compared to
	// its volume.
	//  p = m / V
	// p is the density of the substance, measured in kg/m^3
	// e.g.
	//  Water ranges from 999.8425 at 0C to 958.3665 at 100C
	//  Air ranges from 1.342 at -10C to 1.293 at 0C to 1.164 at 30C
	// Return density in lbs/ft^3
	virtual float Density() const;

	virtual void FilterFinalMoveVector(AuPoint& vector) const;

	virtual void FilterPostMoveVector(AuPoint& vector) const;

	// Returns friction in ft/sec/sec, typically due to surface contact.
	virtual AuPoint Friction() const;

	// Returns the mass of this atom, in pounds.
	virtual float Mass() const
	{
		return Volume() * DensityOfWater();
	}

	virtual void SetAtRest();

public:
	// Returns the location to put the button click sound effect when I operate a console.
	AuPoint ButtonSoundLoc() const;

	bool CanSeeBridgeIndicators() const;

	bool CanSeeOrbitRings() const;

	bool CanSeeMoonOrbitRings() const;

	// Returns 0=no, 1=named, 2=all
	int CanSeeSystemNames() const;

	bool CanSeeWormholes() const;

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

	// Returns the type of console at which this creature is seated. AuStation::TYPE_Count is returned if not at a console.
	AuStation::TYPE ConsoleType() const;

	// Returns console name, blank if not at a console.
	QString ConsoleName() const;

	/*
	inline POSE CurrentBonePose(DhDNA::BONES bone) const
	{
		m_BodyLock.lockForRead();
		POSE pose = m_Body.CurrentBonePose(bone);
		m_BodyLock.unlock();
		return pose;
	}
	*/

	AuPose CurrentGeneralPose() const;

	float CurrentHeadTilt() const;

	AuPose CurrentMouthPose() const;

	// Returns current hit points of body, not scaled for growth size.
	short int CurrentBodyHitPoints() const;

	inline short int CurrentHitPoints() const
	{
		short int hp = CurrentBodyHitPoints();
		if(!IsFullGrown() && hp > 0)
		{
			// Scale current hit points according to growth factor.
			int growTime = GrowthTime();
			hp = hp * (growTime - (m_MinutesToFullGrown - 1)) / growTime;
			if(!hp)
				return 1;	// Return at least 1 hit point since the body has 1.
		}

		return hp;
	}

	inline unsigned short int CurrentHunger() const
	{
		m_HungerLock.lockForRead();
		unsigned short int h = m_Hunger;
		m_HungerLock.unlock();
		return h;
	}

	inline void CurrentPoses3(AuPose pose[DpBody::PS_Count]) const
	{
		Q_ASSERT(m_Body);
		m_BodyLock.lockForRead();
		for(int p=0; p<DpBody::PS_Count; p++)
			pose[p] = m_Body->CurrentPoses()[p];
		m_BodyLock.unlock();
	}

	void CurrentPoses4(AuPose pose[AuAnimalPart::PS_Count]) const;

	inline int CurrentStamina() const
	{
		return m_Stamina;
	}

	void EatOffGround();

	virtual void ExitConsole();

	virtual quint32 ExperiencePointsForKilling() const;

	inline DpStation* LockConsoleForRead() const
	{
		m_ConsoleLock.lockForRead();
		return m_Console;
	}

	inline DpStation* LockConsoleForWrite()
	{
		m_ConsoleLock.lockForWrite();
		return m_Console;
	}

	inline void UnlockConsole() const
	{
		m_ConsoleLock.unlock();
	}

	inline bool HasConsole() const
	{
		return m_Console ? true : false;
	}

	float HullStress() const;

	virtual bool IsAlert() const
	{
		return false;
	}

	bool IsConsolePose() const;

	// True if a troop, citizen, or livestock in the current pose is manning a post.
	bool IsPostPose() const;

	// True if a troop in the current pose is manning a station.
	bool IsTroopStationPose() const;

	bool IsLockedPose() const;

	bool IsFlying() const;

	bool IsLadderPose() const;

	bool IsLeftHanded() const;

	bool IsJumping() const;

	bool IsMonitoringSystem(const AuId& idSolarSystem) const;

	// True if the creature is monitoring a solar system but it is not the same as 'systemId'.
	bool IsMonitoringDifferentSystem(const AuId& systemId) const;

	inline bool IsMounted() const
	{
		return HasSubatoms();
	}

	// Returns true for any horizontal pose, including prone, bunks, dying, dead and mating.
	// This is based on current pose. See AuPose::IsProne.
	bool IsProne() const;

	bool IsSwimming() const;

	// True if my current body pose is an integrated turret station, for new style AuScDesigns.
	bool IsTurretIntegratedPose() const;

	// True if my current body pose is operating a turret.
	bool IsTurretPose() const;

	short int MaxHitPoints() const;

	inline unsigned short int MaxHunger() const
	{
		// Two hunger points per hit point.
		return MaxHitPoints() * 2;
	}

	unsigned short int MaxStamina() const;

	bool IsAwaitingSpawn() const;

	AuPoint JumpVectorFromCurrentPose() const;

	// Speed modifier due to current burden.
	// 1=fast, 0=motionless
	inline float BurdenSpeedMod() const
	{
		float maxBurden = BurdenMax();
		if(Burden() >= maxBurden)
			return 0;
		return (maxBurden - Burden()) / maxBurden;
	}

	inline void MateOnBottom()
	{
		SetGeneralPose(AuPose::MateOnBottom);
		SetAtRest();
	}
	
	inline void MateOnTop()
	{
		SetGeneralPose(AuPose::MateOnTop);
		SetAtRest();
	}
	
	// 'power' is percent power applied, [0..1].
	// Returns movement speed in feet per second.
	float MovementSpeed(float power) const;

	// The result of this modifier shows up as a reduction of the maximum muscle strength.
	inline float MovementSpeedMod() const
	{
		// Adjust speed based on health.
		short int curhp = CurrentHitPoints();
		if(curhp <= 0)
			return 1;			// No speed reduction when body is dead or dying, for body animation.

		// Adjust speed based on burden.
		float burdenSpeedMod = BurdenSpeedMod();
		if(burdenSpeedMod <= 0)
			return 0;

		short int maxhp = MaxHitPoints();
		curhp *= 2;							// Speed reduction due to health starts at 50% damage.
		if(curhp < maxhp)
			return (float)curhp/(float)maxhp * burdenSpeedMod;
		else
			return burdenSpeedMod;
	}

	bool NeedsInit() const;

	float NoiseLevel() const;	// 0..1

	// NOTE: Rendering states must already be configured. Check actual usage of this function.
	virtual void RenderBodyStats(float left, float bottom, float wid, float ht, bool isTarget, unsigned int targetPart,
			POLITICAL_STANCE stance, AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuMatrix4x4& tDevPersp,
			const AuMatrix4x4& tDevOrtho, float hudIntensity);

	void ResetHitPoints();

	inline void ResetHunger()
	{
		SetHunger(MaxHunger());
	}

	inline void ResetStamina()
	{
		m_Stamina = MaxStamina();
	}

	// Finds a normal subatom that is a unit.
	DaUnit* Rider() const;

	void SetChromatid(AuDNABase::CHROMATID chroma);

	void SetHitPoints(short int hitPoints);

	// Sets the current hunger value without regard to the max.
	inline void SetHunger(short int hunger)
	{
		m_HungerLock.lockForWrite();
		if(m_Hunger != hunger)
		{
			// Don't allow negative hunger.
			if(hunger > 0)
				m_Hunger = hunger;
			else
				m_Hunger = 0;
			m_tmHungerChange.Set();
		}
		m_HungerLock.unlock();
	}

	inline void SetHungerChanged()
	{
		m_HungerLock.lockForWrite();
		m_tmHungerChange.Set();
		m_HungerLock.unlock();
	}

	// Sets the hunger value without permitting it to exceed the max.
	inline void SetHungerLimitMax(short int hunger)
	{
		// Don't allow too much hunger value.
		if(hunger > MaxHunger())
			ResetHunger();									// Rest to max.
		else
			SetHunger(hunger);								// Set current hunger level.
	}

	void SetLeftHanded(bool isLefty);

	AuPose SetDeadPose();

	AuPose SetDyingPose();

	bool SetGeneralPose(const AuPose& pose, bool setModified=true);

	virtual bool SetGeneralPose(const AuPose& pose, quint16 stationId, bool setModified=true);

	// 'stationId' and 'controlId' are passed to SitAtConsole, if a console is needed for the pose.
	void SetPoses3(const AuPose pose[DpBody::PS_Count], quint16 stationId, quint16 controlId);
	void SetPoses4(const AuPose pose[AuAnimalPart::PS_Count], quint16 stationId, quint16 controlId);

	// Returns the stationary pose that corresponds to my current body pose.
	AuPose StationaryPose() const;

	quint8 StaminaToJump() const;

	virtual void TestHullStress();

	bool SpecialWeaponReady() const;

	void UseSpecialWeapon();

	virtual COMMODITYID WornArmor() const
	{
		return CID_Invalid;
	}

	virtual COMMODITYID WornClothing() const
	{
		return CID_Invalid;
	}

	virtual COMMODITYID WornHat() const
	{
		return CID_Invalid;
	}

protected:
	void RenderAudioSources(const DaPerson* sceneAtom, const AuMatrix4x4& model, AuSound* al, float fade);

	void LoadClientConsoleSettings();

	void StoreClientConsoleSettings() const;

public:
	// 'stationId' is the part id in an AuScDesign.
	// 'seatIndex' is 0..3 for seating at multi-seat console such as game table.
	// 'controlId' is the system id of the subsystem controlled by the console.
	// Returns my general body pose sitting at the console.
	AuPose SitAtConsole(quint16 stationId, AuStation::TYPE stationType, AuStation::OCCUPANT_POSITION occPos, unsigned int seatIndex,
			quint16 controlId);

	inline void Stand()
	{
		if(SetGeneralPose(AuPose::Stand))
			SetInMotion();
	}

	inline void Straddle()
	{
		SetGeneralPose(AuPose::Straddle);
		SetAtRest();
	}
	
	// Head tilts at base of head. 0 = level
	// Restricts head tilt if in a turret.
	// Returns true if head tilt actually changes.
	virtual bool TiltHeadTo(float angle);

	float TorsoThickness() const;

	float TurnRate() const;

	// Approximate volume of body in cubic feet.
	float Volume() const;

	virtual bool BestWeaponAtRange(float targetRange, AuDNABase::WEAPONLOC& loc, AuCombatWeapon& weapon, int& ammoCur,
			int& ammoFull) const;

	// NOTE: This function is only ever called on the scene atom, which is a person, and the creature on which they are mounted.
	virtual void WeaponAtUsePos(const AuGearPos& pos, INVENTORYSUB sub, AuCombatWeapon& weapon, int& ammoCur, int& ammoFull) const;

	AuPoint WeaponLoc() const;		// Location of weapon model relative to me.
	AuMatrix4x4 WeaponMatrix() const;		// Matrix of weapon model relative to me. +X is firing direction.
	AuPoint WeaponMuzzleLoc() const;	// Emit location of my gun muzzle, relative to me. Location to add muzzle flash effect.
	AuPoint WeaponMuzzleVec() const;	// Emit vector of my gun muzzle, relative to me.
protected:
	AuPoint WeaponMuzzleRot() const;	// Rotation of muzzle flash effect at muzzle loc.
	AuPoint WeaponRot() const;

protected:
	// Modifies tBody if needed for body model pose adjustment.
	void BodyPoseAdjustTransform(AuMatrix4x4& tBody) const;

	static AuPoint CGOffset(const AuAnimalia& body, quint8 minutesToFullGrown, quint8 growthTime, bool isAboardVehicle);

	static AuPoint CGOffset(const DpBody& body, quint8 minutesToFullGrown, quint8 growthTime, bool isAboardVehicle);

	// 'attachThought' is true to attach a thought condition reporting the lost item when the person recovers.
	// Thoughts should only be attached to DaPerson atoms.
	bool DropRandomItem(AuGear& gear, bool attachThought);

	AuPoint LeftHandLoc() const;

	AuPoint HeadLocation() const;

	// Called when stamina reaches zero while mating.
	void Orgasm();

	// Reposition riders to proper locations based on my current body pose.
	void PositionRiders();
	void ForcePositionRiders();

	// Used for mini me display on screen.
	void RenderMiniMe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari, double scale);

	void RenderScene(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& model,
			AuAtomRenderInfo& ari);

	// Call to restore console after loading from database, after adding this atom to its parent atom.
	void RestorePose(const AuPose& pos, quint16 stationId);

	AuPoint RightHandLoc() const;

	// Cleans up render objects of previous item.
	void SetItemInHand(AuGLEffects& re, AuSound* al, const AuItem* item);

	// Just replaces the item.
	void SetItemInHand(const AuItem* item);

	bool SetStationPose(const AuPose& pose, quint16 stationId, quint16 controlId);

	// Returns my station. Returns false if not at a station.
	bool Station(AuStation& station) const;

	// Returns true if I can mate with 'creature' due to our sexual compatibility and my IsReadyForMating test.
	bool TryBecomingReceptiveToMating(const DaCreature& creature) const;

	// Returns true if I can mate with 'creature' due to our sexual compatibility and my IsReadyForMating test.
	bool TryMating(const DaCreature& creature) const;

	// 'explodeEffect' and 'explodeRadius' used when 'cid' is a personal body weapon and 'wMode' is WM_Area.
	void UseItemEffect(COMMODITYID cid, WEAPONMODE wMode, WEAPONEFFECT explodeEffect, double explodeRadius, INVENTORYSUB subMode,
			const AuPoint& targetLoc);
};

#endif // DaCreature_H
