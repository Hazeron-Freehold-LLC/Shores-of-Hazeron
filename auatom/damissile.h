//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: damissile.h,v 1.75 2022/05/17 16:08:11 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaMissile_H
#define DaMissile_H

#include "daitem.h"

class AuItem;
class PsEngineTrail;

class AUATOM_DLL DaMissile : public DaItem  
{
private:
	quint32								m_EmpireId;				// Empire id who launched this missile.
	bool								m_OwnerIgnored;			// True if owner atom is ignored in collision checks.
	AuId								m_TargetId;				// Target when locked on. Non-null implies seeking missile.
	AuPoint								m_TargetLoc;			// Location targetted on target atom, relative to target.

private:
	// Missile guidance.
	mutable QReadWriteLock				m_MissileLock;
	AuPoint								m_ExplodeLoc;			// Location() when I exploded in proximity to m_ExplodeTarget.
	bool								m_ExplodeShield;		// Exploded due to impact with shield.
	AuId								m_ExplodeTarget;		// Non null when I have decided to explode.
	AuElapsedTimer						m_LastTurn;				// Last time I thought about turning to seek a target.
	AuMatrix4x4							m_TurnM;				// TransformBy matrix to turn toward my seek target in Tick.

private:
	// OpenAL
	unsigned int						m_AudioEngine;
	int									m_SoundEngine;			// SOUND_EFFECT

	PsEngineTrail*						m_pEngineTrail;

public:
	// Seeking missile constructor.
	DaMissile(const AuItem& item, quint32 count, quint32 empireId, const AuId& targetId, const AuPoint& targetLoc);

	// Dumb rockets and bombs constructor.
	DaMissile(const AuItem& item, quint32 count, quint32 empireId);

	DaMissile();
	virtual ~DaMissile();

protected:
	DaMissile(const DaMissile &c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual void AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim);

	// True if this atom is permitted to go below the surface of the terrain.
	// This could be any hard surface, such as ground terrain or the floor of a spacecraft or building.
	virtual bool CanBeUnderGround() const
	{
		return true;
	}

	double MuzzleExitVelocity() const;

	virtual DaAtom *Clone() const;

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

	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari);

	virtual void DeleteDeviceObjects(AuAtomRenderInfo& ari);

	virtual quint32 EmpireId() const
	{
		return m_EmpireId;
	}

	inline bool ExplodeHitShield() const
	{
		m_MissileLock.lockForRead();
		bool v = m_ExplodeShield;
		m_MissileLock.unlock();
		return v;
	}

	inline AuId ExplodeTarget() const
	{
		m_MissileLock.lockForRead();
		AuId v(m_ExplodeTarget);
		m_MissileLock.unlock();
		return v;
	}

	virtual void FallStopped(double oldCgAlt, double nuCgAlt, double stopMove, int elapsedMs, int stopReason);

	// Returns an atom that is in 'gunRange' of 'gunSysLoc' and 'gunSysVec', which are relative to the solar system.
	// 'gunnerEmpire' is the empire id of the gunner, who is operating the weapon.
	// 'targetAtom' and 'targetRange' are output parameters.
	// 'targetSysLoc' is the location to target, relative to the solar system.
	virtual void FindClosestShipTurretTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
			quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
			const DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const;

	virtual void Insert(QSqlDatabase& db) const;	// Just returns. Missiles are too short-lived to be stored in the database.

	virtual bool IsActiveMissile() const
	{
		return true;
	}

	// Returns an atom that is in 'gunRange' of 'gunLoc', which is relative to my parent world.
	// 'gunnerEmpire' is the empire id of the gunner, who is at my parent world.
	// 'gunnerPosture' is the posture of the gunner. Caller checks POSTURE_HoldFire before calling to save lots of redundant checks.
	// 'targetLoc' is an output parameter relative to my parent world.
	virtual bool IsGuardTowerTarget(quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunLoc, float gunRange,
			AuPoint& targetLoc) const;

	// Return true if this atom can be targetted by a turret on a spacecraft.
	// 'gunnerEmpire' is the empire id of the gunner. It is 0 when political stance is to be ignored.
	// 'gunnerPosture' is the posture of the gunner. Caller checks POSTURE_HoldFire before calling to save lots of redundant checks.
	// 'gunSysLoc' and 'gunSysVec' are relative to the solar system.
	// 'targetSysLoc' is the location to target, relative to the solar system.
	virtual bool IsShipTurretTarget(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
			CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
			AuPoint& targetSysLoc) const;

	virtual bool IsInScene(const DaPerson* sceneAtom, const AuPoint& cameraEnvLoc) const;

	// True if this atom's empire is a "pirate" empire.
	virtual bool IsPirate() const;

	virtual quint64 MemoryUsage() const;

	virtual AuPoint MovementBlocked(DaAtom* hit, const AuPoint& hitLoc, const AuPoint& hitNormal, bool hitShield);

	virtual QString Name() const;

	// If this atom is "at rest", its rotation is changed immediately and true is returned.
	// If this atom is not "at rest", its m_OrientRotation is set and false is returned.
	// Always returns false if parent is a vehicle or an animal.
	virtual bool Orient(const AuPoint& /*atLoc*/, const AuPoint& /*terNormal*/)
	{
		return false;
	}

	virtual bool Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari);

	virtual void RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
			const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass,
			DaAtomList& shadSc,
			DaAtomList& shadScFog,
			DaAtomList& shadScGlobe,
			DaAtomList& shadScGlobeFog,
			DaAtomList& shadScRingworld,
			DaAtomList& shadScRingworldFog);

	virtual void RenderLast(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
			const DaPerson* sceneAtom,
			DaAtomList& shadSc,
			DaAtomList& shadScFog,
			DaAtomList& shadScGlobe,
			DaAtomList& shadScGlobeFog,
			DaAtomList& shadScRingworld,
			DaAtomList& shadScRingworldFog);

	virtual void RxUpdate(QDataStream& fromServer, bool isFirstUpdate);

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

	// This atom is not stored in the database.
	virtual void SetModified(SET_MODIFIED_BITS notifyBits=NOTIFY_All)
	{
		notifyBits = (SET_MODIFIED_BITS)(notifyBits & ~NOTIFY_Database);		// Turn off database bit.
		DaItem::SetModified(notifyBits);
	}

	inline void SetOwnerIgnored(bool ownerIgnored)
	{
		m_OwnerIgnored = ownerIgnored;
	}

	virtual QString SpotDescription(quint32 spotterEmpire) const;

	virtual void TestHullStress();

	// Missiles search for targets. Even unguided munitions perform proximity checks for nearby targets.
	virtual bool IsTicker() const;

	virtual void Tick(const AuElapsedTimer& last, const AuElapsedTimer& now);

	virtual unsigned int IsTocker() const;

	virtual void Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans);

	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
			const DaPerson& sceneAtom);

	virtual ATOM_TYPE AtomType() const;

	virtual void Update(QSqlDatabase& db) const;	// Just returns. Missiles are too short-lived to be stored in the database.

	virtual bool WeaponOnCollision(AuCombatAttacker& attacker, AuCombatWeapon& weapon, int hitShield, double kineticEnergy) const;

public:
	// Physics
	// Acceleration in ft/sec/sec.
	virtual AuPoint Acceleration() const;

	// Returns friction in ft/sec/sec, typically due to surface contact.
	virtual AuPoint Friction() const;

protected:
	void Explode(DaAtom* target, const AuPoint& explodeLoc, bool hitShield);

	void RenderAudioSources(const DaPerson* sceneAtom, const AuMatrix4x4& model, AuSound* al, float fade);

private:
	// 'foldloc' and 'fnuloc' are relative to pParent.
	// 'fMoveExt' is relative to pParent. It is the rough extents box of the moving atom.
	// 'pHit' and 'hitNormal' are output parameters set if I hit another atom.
	virtual COLLISION_TYPE CheckCollisionRelativeToAtom(DaAtom* pParent, const AuPoint &foldloc, const AuPoint &fnuloc,
			DaAtom*& pHit, AuPoint& hitLoc, AuPoint& hitNormal, const AuBox& fMoveExt);
};

#endif // DaMissile_H
