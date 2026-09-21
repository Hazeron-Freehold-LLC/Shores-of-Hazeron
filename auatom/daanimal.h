//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daanimal.h,v 1.173 2022/05/17 15:15:07 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaAnimal_H
#define DaAnimal_H

#include "dacreature.h"

class AuGLEffects;

class AUATOM_DLL DaAnimal : public DaCreature  
{
public:
	enum ACTIVITY
	{
		Attacking,		// Inflicting damage on prey or defending against attacker.
		Charging,		// Closing range with threat to start Attacking.
		Chasing,		// Closing range with prey to start Attacking.
		Eating,			// Consuming food.
		Fleeing,		// Moving away from danger.
		Gathering,		// Closing range with food to start Eating.
		Searching,		// Routine movement in search of food.
		Waiting,		// Routine stop in search of food.
	};

private:
	// Unguarded
	quint8								m_ActivityTick;			// Tick frame counter until decision is made
	bool								m_IsAlert;				// true when this animal's senses are alert

private:
	mutable QReadWriteLock				m_ThinkLock;			// Guards m_Activity, m_IsAlert, m_TurnZ, etc.
	ACTIVITY							m_Activity;
	int									m_AssumePose;			// >0 = POSE, <0 = special
	AuCombatWeapon						m_AttackWeapon;			// Weapon to use when attacking.
	quint8								m_Decision;
	AuElapsedTimer						m_FFCheck;
	bool								m_HasAttacked;			// For attack logic
	float								m_HeadTilt;
	AuId								m_TargetId;
	AuPoint								m_TargetDir;
	double								m_TargetRange;
	float								m_TurnZ;				// Amount of left/right turn remaining. Set this to the angle to
																// turn to and this animal will turn as fast as it can, based on
																// its dna.
	bool								m_UseSpecWpn;

public:
	DaAnimal(const AuSpecies& species, AuDNABase::CHROMATID chroma, HANDEDNESS hand, GROWTHSTAGE stage);
	DaAnimal();
	virtual ~DaAnimal();

protected:
	DaAnimal(const DaAnimal &rhs);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual void AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim);

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

	// Wild animal becomes domesticated when defibrillated.
	virtual void Defibrillated(quint32 byEmpireId);

	virtual void Dismounted();

	void EatTarget(DaAtom* target);

	virtual void EmpireDeleted(quint32 empireId);

	// Decide to attack or run from a threat.
	// 'reactMod' higher results in greater change to attack or flee.
	void FightOrFlightCheck(float threatRange, int reactMod);

	virtual void FindAnimalsRecursive(DaAtomList& animals) const;

	// Decide to run from danger.
	void FlightCheck(float dangerRange);

	// Returns an atom that is in 'gunRange' of 'gunSysLoc' and 'gunSysVec', which are relative to the solar system.
	// 'shipId' is the ship the gunner is aboard firing a turret.
	// 'gunnerEmpire' is the empire id of the gunner, who is operating the weapon.
	// 'targetAtom' and 'targetRange' are output parameters.
	virtual void FindClosestShipTurretTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
			quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
			const DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const;
	
	// To be interested in eating, the creature must have this level of hunger or below.
	inline unsigned short int HungryThreshold() const
	{
		return MaxHunger() / 2;
	}

	// To be interested in hunting, the creature must have this level of hunger or below.
	inline unsigned short int HuntThreshold() const
	{
		return MaxHunger() * 3 / 4;
	}

	// Used for determination of color highlighting based on political stance toward this unit.
	// Civilian units use a different color palette than military units.
	virtual bool IsCivilian() const;

	// Returns an atom that is in 'gunRange' of 'gunLoc', which is relative to my parent world.
	// 'gunnerEmpire' is the empire id of the gunner, who is at my parent world.
	// 'gunnerPosture' is the posture of the gunner. Caller checks POSTURE_HoldFire before calling to save lots of redundant checks.
	// 'targetLoc' is an output parameter relative to my parent world.
	virtual bool IsGuardTowerTarget(quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunLoc, float gunRange,
			AuPoint& targetLoc) const;

	// We do not permit bestiality and animals do not mate between themselves.
	virtual bool IsReadyForMating() const
	{
		return false;
	}

	// Return true if this atom can be targetted by a turret on a spacecraft.
	// 'gunnerEmpire' is the empire id of the gunner. It is 0 when political stance is to be ignored.
	// 'gunnerPosture' is the posture of the gunner. Caller checks POSTURE_HoldFire before calling to save lots of redundant checks.
	// 'gunSysLoc' and 'gunSysVec' are relative to the solar system.
	// 'targetSysLoc' is the location to target, relative to the solar system.
	virtual bool IsShipTurretTarget(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
			CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
			AuPoint& targetSysLoc) const;

	virtual quint64 MemoryUsage() const;

	virtual DaAtom *Clone() const;

	virtual QString Name() const;

	virtual bool Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari);

	virtual bool IsTicker() const;

	virtual void Tick(const AuElapsedTimer& last, const AuElapsedTimer& now);

	virtual unsigned int IsTocker() const;

	virtual void Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans);

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

	virtual AuPoint MovementBlocked(DaAtom* hit, const AuPoint& hitLoc, const AuPoint& hitNormal, bool hitShield);

	virtual void RxUpdate(QDataStream& fromServer, bool isFirstUpdate);
	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
			const DaPerson& sceneAtom);
	virtual ATOM_TYPE AtomType() const;

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

	// Returns true if this unit can be selected by the scene atom.
	virtual bool UnitCanBeSelected(const DaPerson* sceneAtom, bool topDownView) const
	{
		if(!IsMounted())
			return false;
		return DaCreature::UnitCanBeSelected(sceneAtom, topDownView);
	}

public:
	static DaAnimal* LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad=0);
	virtual void Insert(QSqlDatabase& db) const;	// Insert first time database record.
	virtual void Update(QSqlDatabase& db) const;	// Update existing database record.

	// Physics
	// Acceleration in ft/sec/sec.
	virtual AuPoint Acceleration() const;

	// True if this atom is permitted to go below the surface of the terrain.
	// This could be any hard surface, such as ground terrain or the floor of
	// a spacecraft or building.
	virtual bool CanBeUnderGround() const;

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

	// Returns friction in ft/sec/sec, typically due to surface contact.
	virtual AuPoint Friction() const;

	inline ACTIVITY Activity() const
	{
		m_ThinkLock.lockForRead();
		ACTIVITY act = m_Activity;
		m_ThinkLock.unlock();
		return act;
	}

	inline void SetActivity(ACTIVITY act)
	{
		m_ThinkLock.lockForWrite();
		m_Activity = act;
		m_ThinkLock.unlock();
	}

	inline bool IsActivityThreatening() const
	{
		ACTIVITY act = Activity();
		return act == Attacking || act == Charging || act == Chasing;
	}

protected:
	void DoAI(const AuTime& last, const AuTime& now);

	// AI decision making and execution happens in two steps.
	// 1. The Think function calls MakeDecision to decide what to do. It does nothing except set state values on this atom.
	// 2. ExecuteDecision is called during Tick to carry out the decisions that have been made. It must work quickly with minimal
	//    processing.
	void ExecuteDecision();

	virtual bool IsAlert() const
	{
		// When alert, the AI logic is invoked more often to be responsive
		// enough for combat interactions.
		return m_IsAlert;
	}

	void Look(DaAtom*& foodId, AuPoint& foodDir, float& foodRange,
			DaAtom*& preyId, AuPoint& preyDir, float& preyRange,
			DaAtom*& threatId, AuPoint& threatDir, float& threatRange,
			DaAtom*& dangerId, AuPoint& dangerDir, float& dangerRange);

	void MakeDecision(const AuTime& last, const AuTime& now);

	void Search();

	inline void SetAlert(bool isAlert)
	{
		m_ThinkLock.lockForWrite();
		m_IsAlert = isAlert;
		m_ThinkLock.unlock();
	}

	void Wait();
};

#endif // DaAnimal_H
