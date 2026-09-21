//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daasteroid.h,v 1.30 2022/05/17 15:20:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaAsteroid_H
#define DaAsteroid_H

#include "auatom.h"
#include "daatom.h"

class Au3dsGLSLModelInstance;
class AuSmokeParticles;

class AUATOM_DLL DaAsteroid : public DaAtom  
{
private:
	enum ASTEROID_OBSERVATION
	{
		ASTEROID_HitSun,
		ASTEROID_HitBlackHole,
		ASTEROID_HitWorld,
		ASTEROID_DestroyedWorld,
		ASTEROID_DepartedDeepSpace,
	};

private:
	mutable QReadWriteLock				m_AsteroidLock;
	quint16								m_Hits;					// Current hit points.
	AuElapsedTimer						m_HitsChange;
	quint8								m_Size;					// 6-45
	AuPoint								m_SpinAxis1;			// May be 0,0.
	float								m_SpinSpeed1;			// In radians per second.
	AuPoint								m_SpinAxis2;			// May be 0,0.
	float								m_SpinSpeed2;			// In radians per second.
	AuId								m_Target;				// Sometimes an asteroid is targetted at a specific atom.
	AuTime								m_TargetTime;			// Arrival time at the target.
	AuTime								m_TargetReleaseTime;	// Time of release when waiting to approach the target.

	// Render values.
	Au3dsGLSLModelInstance*				m_Model;
	AuSmokeParticles*					m_PSSmoke;
	AuMatrix4x4							m_Spin;					// Current spin.

public:
	DaAsteroid(const AuId& target, const AuTime& timeOnTarget, const AuTime& timeOfRelease);
	DaAsteroid(quint8 size);
	DaAsteroid();
	virtual ~DaAsteroid();
protected:
	DaAsteroid(const DaAsteroid &c);

public:
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	virtual void AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim);

	// Returns asteroid size in the range 6 to 45.
	inline quint32 AsteroidSize() const
	{
		m_AsteroidLock.lockForRead();
		int s = m_Size;
		m_AsteroidLock.unlock();
		return s;
	}

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

	virtual void CombatArmorEffectiveness(WEAPONMODE mode, WEAPONEFFECT effect, bool hitShield, const AuPoint& wpnLoc,
			const AuPoint& hitLoc, float& baseAbsorb, float& baseBlock, float& auxAbsorb, float& auxBlock) const;

	void Create();

	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari);

	virtual void DeleteDeviceObjects(AuAtomRenderInfo& ari);

	// Return true for atoms that require QueryEnvironment information.
	// Atoms that can have a movement vector must return true.
	virtual bool DoesQueryEnvironment() const
	{
		return true;
	}

	virtual const DaAsteroid* FindAsteroidRecursive(const AuId& targetId) const;

	virtual void FindAsteroidsRecursive(DaAtomList& asteroids) const;

	virtual void FindClosestDefenseBaseTargetRecursive(int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
			const AuPoint& gunSysLoc, float gunRange, DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc);

	virtual const DaAtom* FindClosestMissileTarget(const AuId& launcherId, int missileDomain, const AuId& missileWorldId,
			quint32 missileEmpireId, const AuPoint& missileSysLoc, const DaAtom* bestAtom, AuPoint& bestSysLoc,
			double& bestDistance) const;

	virtual void FindClosestShipTurretTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
			quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
			const DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const;

	virtual void FindClosestShipWeaponTargetRecursive(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId,
			quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& shipSysLoc, float gunRange,
			const DaAtom*& targetAtom, double& targetRange, AuPoint& targetSysLoc) const;

	virtual float GroundHeightAboveCG() const
	{
		return ObstacleRadius();
	}

	inline quint16 Hits() const
	{
		m_AsteroidLock.lockForRead();
		quint16 v = m_Hits;					// Current hit points.
		m_AsteroidLock.unlock();
		return v;
	}

	virtual void Insert(QSqlDatabase& db) const;

	// Returns true if this asteroid is targetted on an atom and it is waiting for its designated release time.
	bool IsAwaitingRelease() const;

	virtual bool IsEthereal() const;

	virtual bool IsInScene(const DaPerson* sceneAtom, const AuPoint& cameraEnvLoc) const;

	virtual bool IsObstacle(const DaAtom* toWho) const;

	virtual bool IsShipTurretTarget(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
			CITIZEN_POSTURE gunnerPosture, const AuPoint& gunSysLoc, float gunRange, const AuPoint& gunSysVec,
			AuPoint& targetSysLoc) const;

	virtual bool IsShipWeaponTarget(const AuId& shipId, int gunnerDomain, const AuId& gunnerWorldId, quint32 gunnerEmpire,
			CITIZEN_POSTURE gunnerPosture, const AuPoint& shipSysLoc, float gunRange, AuPoint& targetSysLoc) const;

	// Return true if this atom can be sighted when targeted with a weapon or tool by 'toWho'.
	// If false is returned, this atom will not act as an obstacle to gunfire from 'toWho'.
	// 'toWho' may be null.
	virtual bool IsSightTarget(const DaAtom* toWho) const;

	virtual bool IsTicker() const;

	virtual unsigned int IsTocker() const;

	static DaAsteroid* LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad=0);

	// 'hit' is the atom that was hit.
	// 'hitNormal' is the surface normal at the point of collision, relative to this atom.
	// Returns the undampened bounce vector. The length is the speed in feet per second that this atom was moving when the hit
	// occurred. It is simply the reflected movement vector when the hit occurred.
	virtual AuPoint MovementBlocked(DaAtom* hit, const AuPoint& hitLoc, const AuPoint& hitNormal, bool hitShield);

	virtual QString Name() const;

	// Returns actual radius of this object for avoidance when navigating and bay weapon fire obstruction.
	virtual double ObstacleRadius() const
	{
		return ObstacleRadius(m_Size);
	}

	static double ObstacleRadius(quint8 size)
	{
		// 'size' < 6 is not valid.
		return (size-5) * 250;
	}

	virtual bool Orient(const AuPoint& atLoc, const AuPoint& terNormal)
	{
		Q_UNUSED(atLoc);
		Q_UNUSED(terNormal);
		return false;
	}

	// Called by spacecraft to avoid objects.
	// Returns safe radius with a safety margin to avoid this object when navigating.
	// A return value of 0 means this atom is not avoided by spacecraft.
	virtual double PilotAvoidRadius() const;

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

	virtual bool RoughExtents(AuBox &fExt) const;

	virtual void RxUpdate(QDataStream& fromServer, bool isFirstUpdate);

	virtual void SensorScan(AuSensorBlipList& blips, const AuPoint& scannerLoc, const AuId& scannerId, quint32 scannerEmpire,
			int scannerDomain, const AuId& scannerWorld, float scannerSensorRange, float scannerWeaponRange, int scanContent,
			const AuId& lockedBlip, unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash>& stanceCache) const;

	virtual DaAtom* SightTarget(const DaAtom* sightingAtom, const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, DaAtom* pBest,
			AuPoint& sightedLoc, unsigned int& sightedPart, bool& sightedShield, AuPoint* sightedSurfaceNormal) const;

	virtual QString SpotDescription(quint32 spotterEmpire) const;

	virtual QString SystemScanReport(int scanEx) const;

	inline AuId TargetId() const
	{
		m_AsteroidLock.lockForRead();
		AuId targetId(m_Target);
		m_AsteroidLock.unlock();
		return targetId;
	}

	// Speed at which a targetted asteroid pursues its target when there is no impact time specified or the impact time has passed.
	static double TargetPursueSpeed();

	inline AuTime TargetTime() const
	{
		m_AsteroidLock.lockForRead();
		AuTime targetTime(m_TargetTime);
		m_AsteroidLock.unlock();
		return targetTime;
	}

	virtual void Tick(const AuElapsedTimer& last, const AuElapsedTimer& now);

	virtual void Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans);

	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
			const DaPerson& sceneAtom);

	virtual ATOM_TYPE AtomType() const;

	virtual void Update(QSqlDatabase& db) const;

public:
	// Physics
	// Acceleration in ft/sec/sec.
	virtual AuPoint Acceleration() const;

	// Physics
	// Density is mass (m) per unit volume (V).
	// Density is the ratio of the amount of matter in an object compared to
	// its volume.
	//  p = m / V
	// p is the density of the substance, measured in kg/m^3
	// e.g.
	//  Water ranges from 999.8425 at 0C to 958.3665 at 100C
	//  Air ranges from 1.342 at -10C to 1.293 at 0C to 1.164 at 30C
	virtual float Density() const
	{
		return DensityOfWater()*2;
	}

	// Returns friction in ft/sec/sec, typically due to surface contact.
	virtual AuPoint Friction() const;

	// Returns the mass of this atom, in pounds.
	virtual float Mass() const
	{
		// From: http://www.delawarequarries.com/landscape/boulders/boulderwt.html
		// The average weight of a sandstone boulder is approximately 150 pounds per cubic foot. Limestone boulders and granite
		// boulders in most cases weigh more. They average about 175 pounds per cubic foot.
		float r = (float)ObstacleRadius();
		float vol = 4.f/3.f * PIf * r * r * r;
		return vol * 175;	// Assume asteroids are on the denser side, not made of sandstone.
	}

protected:
	void Explode(const AuPoint& explodeLoc);

	void HitPlanet();

	QString ModelName() const;

	// Notifies observatories of an event they may have seen, so they can report me hitting the sun or other things.
	// 'name' is the DaAtom::Name of the observed atom.
	void NotifyObservers(ASTEROID_OBSERVATION obs, BODY_TYPE bodyType, const QString& name) const;
};

#endif // DaAsteroid_H
