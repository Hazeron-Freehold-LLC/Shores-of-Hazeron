//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daunit.h,v 1.218 2022/05/17 16:47:06 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaUnit_H
#define DaUnit_H

#include "aucondition.h"
#include "auunitdamage.h"
#include "daatom.h"
#include <QDataStream>

class AuFireParticles;
class AuSmokeParticles;
class DeWaypointRoute;
class PsCloudAcid;
class PsCloudDisease;
class PsCloudPoison;
class PsCloudStun;
class PsNebula;
class PsSpeck;

// Actions that can be posted to a person during ProcessControlInput.
// These flags trigger actions that require a time interval to perform, such as walking.
// These actions are processed during the Tick function.
#define ACT_None			0x00000000
#define ACT_MoveForward		0x00000001
#define ACT_MoveBackward	0x00000002
#define ACT_MoveLeft		0x00000004
#define ACT_MoveRight		0x00000008
#define ACT_MoveMask		0x0003000f
#define ACT_LookUp			0x00000010
#define ACT_LookDown		0x00000020
#define ACT_LookLeft		0x00000040
#define ACT_LookRight		0x00000080
#define ACT_LookMask		0x000000f0
#define ACT_TurnLeft		0x00000100
#define ACT_TurnRight		0x00000200
#define ACT_TurnMask		0x00000300
#define ACT_PitchUp			0x00000400
#define ACT_PitchDown		0x00000800
#define ACT_PitchMask		0x00000c00
#define ACT_RollLeft		0x00001000
#define ACT_RollRight		0x00002000
#define ACT_RollMask		0x00003000
#define ACT_Run				0x00004000
#define ACT_SpawnRecall		0x00008000
#define ACT_MoveDown		0x00010000
#define ACT_MoveUp			0x00020000
#define ACT_Brake			0x00040000
#define ACT_BrakeLateral	0x00080000
#define ACT_BrakeVertical	0x00100000
#define ACT_BrakeMask		0x001c0000
#define ACT_SpawnTie		0x00200000	// Initiated spawn tie
#define ACT_Jump			0x00400000
#define ACT_Continuous		0x00800000	// Only in conjunction with ACT_MoveForward.
#define ACT_SpawnLocal		0x01000000	// Respawn at next Tick with forceLocal = true. See DaPerson::LoadSubatoms for usage.
#define ACT_SpawnAnywhere	0x02000000	// Respawn at next Tick with forceLocal = false. See DaPerson::LoadSubatoms for usage.
#define ACT_SuitBrake		0x08000000	// Set while suit thruster is on.
#define ACT_SuitThruster	0x10000000	// Set while suit brake is on.
#define ACT_SuitMask		0x18000000

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL DaUnit : public DaAtom  
{
private:
	// Unguarded
	quint8								m_ArenaTeam;			// Arena DgEmpire id who owns this unit.
	mutable quint8						m_IsPirateEmpire;		// 0=unknown, 1=no, 2=yes

private:
	mutable QReadWriteLock				m_UnitLock;
	quint32								m_idAllegiance;			// The DgEmpire id who owns this unit.
	QString								m_sName;				// Every unit can have a name. It doesn't have to be unique.
	AuElapsedTimer						m_tmChange;				// i.e. allegiance, hit points, name, arena team
	AuUnitDamageHash					m_UnitDamage;			// History of the atoms that did damage to this unit recently.

protected:
	mutable QReadWriteLock				m_ConditionsLock;
	AuConditionList						m_Conditions;			// Conditions affecting this unit. i.e. fire, disease, etc.
	AuElapsedTimer						m_tmConditionChange;	// m_Conditions change
	AuElapsedTimer						m_tmConditionTick;

private:
	// OpenGL
	PsCloudAcid*						m_PSAcid;
	PsCloudDisease*						m_PSDisease;
	AuFireParticles*					m_PSFire;				// Fire particle system.
	PsNebula*							m_PSNebula;				// Smoky environmental nebula effect.
	PsCloudPoison*						m_PSPoison;
	AuSmokeParticles*					m_PSSmoke;				// Smoke particle system.
	PsSpeck*							m_PSSpeck;				// specks moving in space
	PsCloudStun*						m_PSStun;
	AuGLVAO								m_WarpVAO;				// Warp tunnel.

public:
	DaUnit(const QString& name, int idEmpire);
	DaUnit();
	virtual ~DaUnit();

protected:
	DaUnit(const DaUnit &rhs);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	// Adds a bounty to this unit.
	virtual void AddBounty(const AuId& posterId, const QString& posterName, const QString& reason, double reward, int why);

	inline void AddCondition(AuCondition* condition)
	{
		m_ConditionsLock.lockForWrite();
		m_Conditions.append(condition);
		m_tmConditionChange.Set();
		m_ConditionsLock.unlock();
	}

	// Used for building waypoint routes for ships.
	// Adds waypoints to 'route', via known wormholes, to go to a destination solar system that is/contains 'idDest'.
	// If no wormhole route exists, no waypoints are added.
	// 'vFromDestAtom' is used for locating wormholes in my current solar system.
	// 'origSystem' is used if the route is empty to determine the starting point. If NULL, this atom's system is used.
	// 'negativeOk' is true if it is ok to enter negative wormholes.
	// 'neutralOk' is true if it is ok to enter neutral wormholes.
	bool AddWaypointRouteTo(DeWaypointRoute& route, const AuId& origSystem, const AuId& idDest, const AuPoint& vFromDestAtom,
			bool negativeOk, bool neutralOk) const;

	// Returns my empire of allegiance, unlike EmpireId() which returns ArenaTeam() in an arena.
	inline quint32 Allegiance() const
	{
		m_UnitLock.lockForRead();
		quint32 v = m_idAllegiance;
		m_UnitLock.unlock();
		return v;
	}

	inline quint8 ArenaTeam() const
	{
		return m_ArenaTeam;
	}

	// Returns the name that has been assigned, unlike Name() which substitutes my string id if I haven't been named.
	QString AssignedName() const
	{
		m_UnitLock.lockForRead();
		QString v(m_sName);
		m_UnitLock.unlock();
		return v;
	}

	// Returns the total current bounty on this unit.
	double Bounty() const;

	// Collects a list of bounties.
	void Bounties(QList<AuBountyOnItem>& bounties) const;

	// Keyed by when the bounty was created.
	void Bounties(QMultiMap<AuTime, AuBountyOnItem>& bounties) const;

	// Clears damage history caused by a particular atom.
	inline void ClearUnitDamage(const AuId& byWho)
	{
		m_UnitLock.lockForWrite();
		m_UnitDamage.Clear(byWho);
		m_UnitLock.unlock();
	}

	inline void CloneConditions(AuConditionList& conditions) const
	{
		m_ConditionsLock.lockForRead();
		for(AuConditionList::const_iterator ci = m_Conditions.constBegin(); ci != m_Conditions.constEnd(); ci++)
			conditions.append((*ci)->Clone());
		m_ConditionsLock.unlock();
	}

	inline void ConditionsChanged()
	{
		m_tmConditionChange.Set();
	}

	// Returns true if a unit is inside this unit whose empire considers 'empireId' to be an enemy.
	// Not recursive.
	// Used to test for enemy units whose presence blocks construction on a world. That is why DaBd overrides this but DaVessel
	// does not.
	virtual bool ContainsEnemyUnit(quint32 empireId) const
	{
		Q_UNUSED(empireId);
		return false;
	}

	virtual void DeleteDeviceObjects(AuAtomRenderInfo& ari);

	// Returns due date of my baby, if I am pregnant; otherwise returns null time.
	AuTime DueDate() const;

	// Returns id of the father of my baby, if I am pregnant; otherwise returns null id.
	AuId FatherOfBaby() const;

	// Returns the radius of the fire effect when this unit is on fire.
	virtual double FireRadius() const
	{
		return ObstacleRadius() / 3;
	}

	virtual void GetRenderLightsRecursive(const DaPerson& person, const AuMatrix4x4& model, QList<AuGLSun>& suns,
			QList<AuGLLight>& lights) const;

	inline const AuConditionList& LockConditions() const
	{
		m_ConditionsLock.lockForRead();
		return m_Conditions;
	}

	inline AuConditionList& LockConditionsForWrite()
	{
		m_ConditionsLock.lockForWrite();
		return m_Conditions;
	}

	inline void UnlockConditions() const
	{
		m_ConditionsLock.unlock();
	}

	inline const AuUnitDamageHash& LockUnitDamage() const
	{
		m_UnitLock.lockForRead();
		return m_UnitDamage;
	}

	inline AuUnitDamageHash& LockUnitDamageForWrite()
	{
		m_UnitLock.lockForWrite();
		return m_UnitDamage;
	}

	inline void UnlockUnitDamage() const
	{
		m_UnitLock.unlock();
	}

	// Return true for atoms that require QueryEnvironment information.
	// Atoms that can have a movement vector must return true.
	virtual bool DoesQueryEnvironment() const
	{
		return true;
	}

	// Find atoms that might cause shields to be raised.
	virtual void FindClosestShipThreatRecursive(const AuId& shipId, int shipDomain, const AuId& shipWorldId, quint32 shipEmpire,
			SHIELD_POLICY shieldPolicy, const AuPoint& shipSysLoc, float sensorRange, const DaAtom*& targetAtom,
			double& targetRange) const;

	// Returns an atom that is in 'gunRange' of 'gunSysLoc' and 'gunSysVec', which are relative to the solar system.
	// 'gunnerEmpire' is the empire id of the gunner, who is operating the weapon.
	// 'targetAtom' and 'targetRange' are output parameters.
	// 'targetSysLoc' is the location to target, relative to the solar system.
	virtual void FindClosestShipTurretTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
			quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
			const DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const;

	// Unlock using UnlockConditions.
	inline const AuCondition* FindConditionLock(AuCondition::CONDITION_TYPE conditionType) const
	{
		m_ConditionsLock.lockForRead();
		for(AuConditionList::const_iterator ci = m_Conditions.constBegin(); ci != m_Conditions.constEnd(); ci++)
			if((*ci)->CType() == conditionType)
				return *ci;
		return 0;
	}

	inline AuCondition* FindConditionClone(AuCondition::CONDITION_TYPE conditionType) const
	{
		m_ConditionsLock.lockForRead();
		for(AuConditionList::const_iterator ci = m_Conditions.constBegin(); ci != m_Conditions.constEnd(); ci++)
			if((*ci)->CType() == conditionType)
			{
				AuCondition* c = (*ci)->Clone();
				m_ConditionsLock.unlock();
				return c;
			}
		m_ConditionsLock.unlock();
		return 0;
	}

	// Returns:
	// 0 = good return
	// 1 = error
	// 2 = blocked by shields on target bd
	// 3 = blocked by shields in target city
	// 4 = blocked by shields on target vessel
	int FindTransporterTarget(const AuId& targetId, TRANSPORTER_TARGET target, DaAtom*& targAtom, AuPoint& targLoc,
			AuPoint& targZ) const;

	inline bool HasAnyCondition() const
	{
		m_ConditionsLock.lockForRead();
		bool anyCondition = !m_Conditions.empty();
		m_ConditionsLock.unlock();
		return anyCondition;
	}

	inline bool HasCondition(AuCondition::CONDITION_TYPE conditionType) const
	{
		m_ConditionsLock.lockForRead();
		for(AuConditionList::const_iterator ci = m_Conditions.constBegin(); ci != m_Conditions.constEnd(); ci++)
			if((*ci)->CType() == conditionType)
			{
				m_ConditionsLock.unlock();
				return true;
			}
		m_ConditionsLock.unlock();
		return false;
	}

	bool HasInsuranceBounty() const;

	inline bool HasTreatableConditions() const
	{
		m_ConditionsLock.lockForRead();
		bool v = m_Conditions.HasTreatableConditions();
		m_ConditionsLock.unlock();
		return v;
	}

	// True if this unit has any acid condition.
	bool HasAcid() const;

	// True if this unit has any disease condition.
	bool HasDisease() const;

	bool HasDisease(quint8 strength, quint8 contagion, quint8 quality) const;

	// True if this unit has any implant condition.
	bool HasImplant() const;

	double InsuranceBounty() const;

	// Test for me sitting in a vehicle that is aboard a capsule in the appropriate parking spot to beam by transporter.
	// Returns the DaVehicle::ParkingSpotType on success, SPOT_Invalid on failure.
	VEHICLE_SPOT_TYPE InVehicleTransporter() const;

	inline bool IsAnyConditionVisible() const
	{
		m_ConditionsLock.lockForRead();
		bool v = m_Conditions.IsVisible();
		m_ConditionsLock.unlock();
		return v;
	}

	// Returns true if insurance bounty exceeds a threshold (1,000,000c?)
	bool IsBlackListedByInsurer() const;

	// True if this unit has a visible fire burning.
	// Base class returns true if any fire condition exists.
	bool IsBurning() const;

	// Returns an atom that is in 'gunRange' of 'gunLoc', which is relative to my parent world.
	// 'gunnerEmpire' is the empire id of the gunner, who is at my parent world.
	// 'gunnerPosture' is the posture of the gunner. Caller checks POSTURE_HoldFire before calling to save lots of redundant checks.
	// 'targetLoc' is an output parameter relative to my parent world.
	virtual bool IsGuardTowerTarget(quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunLoc, float gunRange,
			AuPoint& targetLoc) const;

	virtual bool IsInvisibleOffLine() const
	{
		return false;
	}

	virtual bool IsNoncombatant() const;

	// True if this unit has any paralysis condition.
	bool IsParalyzed() const;

	// True if this unit's empire is a "pirate" empire.
	virtual bool IsPirate() const;

	// True if this unit has any poison condition.
	bool IsPoisoned() const;

	// True if this unit has a pregnancy condition.
	bool IsPregnant() const;

	// Return true if this atom can be targetted by a turret on a spacecraft.
	// 'gunnerEmpire' is the empire id of the gunner. It is 0 when political stance is to be ignored.
	// 'gunnerPosture' is the posture of the gunner. Caller checks POSTURE_HoldFire before calling to save lots of redundant checks.
	// 'gunSysLoc' and 'gunSysVec' are relative to the solar system.
	// 'targetSysLoc' is the location to target, relative to the solar system.
	virtual bool IsShipTurretTarget(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
			CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
			AuPoint& targetSysLoc) const;

	// Called when this unit is killed by a person.
	virtual void KilledByPerson(DaPerson* /*killer*/)
	{
	}

	// Called when this unit is killed by another unit.
	virtual void KilledByUnit(const DaUnit* /*killer*/)
	{
	}

	// Lists conditions that can be treated by a medic.
	void ListTreatableConditions(QStringList& conditions) const;

	void NameTheBaby(const QString& babyName);

	// Returns a valid Npc role while AI controlled. Returns ROLE_Count if player controlled or otherwise not npc AI controlled.
	virtual NPC_ROLE NpcRole() const
	{
		return ROLE_Count;
	}

	// Pays an amount toward insurance bounties. They are removed when paid off.
	void PayInsuranceBounty(double amount);

	POLITICAL_STANCE PoliticalStanceMakeContact(const DaUnit* unit) const;

	// Reduces the quality of a condition by the 'qualityReduction'.
	// Returns true if the condition is reduced.
	// Returns false if the condition is not found, which is the only reason it would not be reduced.
	bool ReduceCondition(const AuCondition* condition, quint8 qualityReduction);

	virtual bool Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari);

	// Returns true if the condition is removed.
	// Returns false if the condition is not found, which is the only reason it would not be removed.
	bool RemoveCondition(const AuCondition* condition);

	inline void SetArenaTeam(quint8 team)
	{
		m_UnitLock.lockForWrite();
		m_ArenaTeam = team;
		m_tmChange.Set();
		m_UnitLock.unlock();
	}

	// Takes all standing bounties and returns the sum. The bounties are removed from this unit's conditions list.
	double TakeBounty();

	// Takes the largest standing bounty and returns the sum. The bounty is removed from this unit's conditions list.
	virtual bool TakeItemBounty(const QString& bountyHunter, const AuBountyOnItem& bountyOnItem);

	// Called when this atom is credited with killing a unit.
	// All unit kills go through here.
	// This is where empires declare mutual enemies based on unit kills. See DaUnit::UnitKilled.
	virtual void UnitKilled(DaUnit* victim);

	// Return the number of experience points gained by killing this unit.
	virtual quint32 ExperiencePointsForKilling() const = 0;

	// Returns true if a name has been assigned.
	inline bool HasName() const
	{
		m_UnitLock.lockForRead();
		bool v = m_sName.isEmpty() ? false : true;
		m_UnitLock.unlock();
		return v;
	}

	// Returns the atom id who is responsible for the most damage dealt on this unit.
	AuId KillerId() const;

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const
	{
		QString s(AssignedName());
		if(s.isEmpty())
			return "Unit";
		else
			return s;
	}

	// Record damage dealt by an atom into this unit's damage log.
	// 'damageDealt' must be non-zero.
	// byWho id can be null or cidUsed can be CID_Invalid but not both.
	void RecordDamage(const AuId& byWho, quint32 damageDealt, COMMODITYID cidUsed, quint8 cidQuality);

	// Returns station pointer if this unit is in trade range of a station that is neutral, friend, or empire.
	// 'myRoughRadius' is used by ships to allow a small portion of them to be inside the area, not necessarily the center
	// 'hasStation' is an output parameter, set true if a station is found in the system whether or not it is in range.
	DaVessel* StationInTradeRange(double myRoughRadius, bool& hasStation) const;

	virtual unsigned int IsTocker() const;

	virtual void Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans);

	virtual void RxUpdate(QDataStream& fromServer, bool isFirstUpdate);
	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
			const DaPerson& sceneAtom);
	virtual ATOM_TYPE AtomType() const;
	virtual void AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim);

	virtual float CGHeightStanding() const
	{
		// Return height of this atom when "standing".
		return 0;
	}
	
	inline ON_GROUND OnGroundSound() const
	{
		if(IsInVisibleWater())
			return OG_WaterSoundOnly;
		else
			return OnGround();
	}

	// Fills 'empires' with my empire id and all the empires that consider my empire to be a friend.
	// Result could be empty.
	void EmpireAndFriendlies(QList<quint32>& empires) const;

	// Fills 'empires' with my empire id and all the empires that my empire considers to be friends.
	// Result could be empty.
	void EmpireAndFriends(QList<quint32>& empires) const;

	// Returns m_idAllegiance when I am not in an arena instance.
	// Returns m_ArenaTeam when I am in an arena instance.
	virtual quint32 EmpireId() const;

	// Removes all conditions that could be fixed at a repair shop, or by a repairman. e.g. fire, acid, paralysis.
	// Returns true if anything changes.
	bool RepairConditions();

	inline void SetAllegiance(quint32 idEmpire)
	{
		m_UnitLock.lockForWrite();
		m_idAllegiance = idEmpire;
		m_tmChange.Set();
		m_UnitLock.unlock();
	}

	virtual void SetEmpireId(quint32 idEmpire);

	virtual void SetName(const QString& name)
	{
		if(AssignedName() != name)
		{
			m_UnitLock.lockForWrite();
			m_sName = name;
			m_tmChange.Set();
			m_UnitLock.unlock();
		}
	}

	// Used for determination of color highlighting based on political stance toward this unit.
	// Civilian units use a different color palette than military units.
	virtual bool IsCivilian() const
	{
		return false;	// Most units are not civilians so that is the default.
	}

	inline bool IsDamagedBy(const AuId& id) const
	{
		m_UnitLock.lockForRead();
		bool v = m_UnitDamage.find(id) != m_UnitDamage.end();
		m_UnitLock.unlock();
		return v;
	}

	// Returns the commodity that an atom used to inflict the last damage.
	// CID_Invalid = not damaged by atom 'id'
	inline COMMODITYID DamagedUsingCID(const AuId& id) const
	{
		m_UnitLock.lockForRead();
		AuUnitDamageHash::const_iterator udi = m_UnitDamage.find(id);
		if(udi != m_UnitDamage.end())
		{
			COMMODITYID v = (*udi).second.CommodityId();
			m_UnitLock.unlock();
			return v;
		}
		m_UnitLock.unlock();
		return CID_Invalid;
	}

	// Returns the quality of the commodity that an atom used to inflict the last damage.
	// 0 returned if not damaged by atom 'id'
	inline quint8 DamagedUsingQ(const AuId& id) const
	{
		m_UnitLock.lockForRead();
		AuUnitDamageHash::const_iterator udi = m_UnitDamage.find(id);
		if(udi != m_UnitDamage.end())
		{
			quint8 v = (*udi).second.m_Quality;
			m_UnitLock.unlock();
			return v;
		}
		m_UnitLock.unlock();
		return CID_Invalid;
	}

	virtual bool IsSightTarget(const DaAtom* /*toWho*/) const
	{
		return true;	// This atom can be sighted when targeting a weapon or tool.
	}

	// Returns true if this unit can be selected by the scene atom to be given orders.
	virtual bool UnitCanBeSelected(const DaPerson* /*sceneAtom*/, bool /*topDownView*/) const
	{
		return false;	// Most units cannot be selected.
	}

public:
	void RenderAcid(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float size);

	void RenderDisease(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float size);

	void RenderFire(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float size);

	void RenderParalysis(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float size);

	void RenderPoison(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float size);

	void RenderSmoke(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float size);

	// Renders warp tunnel if going fast enough, otherwise renders nebula cloud if there is one.
	// This is called before RenderLast. See DaSolarSystem::RenderEnvironment
	void RenderWarpOrNebula(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari);

protected:
	// Clears all damage history.
	inline void ClearUnitDamage()
	{
		m_UnitLock.lockForWrite();
		m_UnitDamage.clear();
		m_UnitLock.unlock();
	}

	// Returns a number from 0..1 based on the current affect of paralyzing conditions.
	inline float ConditionSpeedEffect() const
	{
		float moveSpeed = 1;
		m_ConditionsLock.lockForRead();
		if(!m_Conditions.isEmpty())
		{
			// Reduce speed with paralysis.
			int paralysis = 0;
			for(AuConditionList::const_iterator ci = m_Conditions.constBegin(); ci != m_Conditions.constEnd(); ci++)
			{
				const AuConParalysis* paralyzeCon = dynamic_cast<const AuConParalysis*>(*ci);
				if(paralyzeCon)
					paralysis += paralyzeCon->SpeedReduction();	// This time is close enough for this calc.
			}
			if(paralysis >= 100)
				moveSpeed = 0;
			else if(paralysis > 0)
				moveSpeed = (100.f - (float)paralysis) / 100.f;
		}
		m_ConditionsLock.unlock();
		return moveSpeed;
	}

	void CreateWarpTunnelVAO(AuGLEffects& re);

	inline AuTime LastUnitDamage(const AuId& byUnitId) const
	{
		m_UnitLock.lockForRead();
		AuTime v(m_UnitDamage.LastDamage(byUnitId));
		m_UnitLock.unlock();
		return v;
	}

	// Render gravel moving in space around the unit.
	void RenderGravel(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari);

	// Render smoky nebula effect when inside a nebula.
	void RenderNebula(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari);

	// Renders FTL effect when moving near the speed of light.
	bool RenderWarpTunnel(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari);

	void TickConditions();
};

#endif // DaUnit_H
