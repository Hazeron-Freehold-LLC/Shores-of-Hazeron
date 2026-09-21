//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daworldring.h,v 1.92 2022/05/17 18:43:42 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaWorldRing_H
#define DaWorldRing_H

#include "daworld.h"

class AuZoneResource;

class AUATOM_DLL DaWorldRing : public DaWorld
{
private:
	mutable QReadWriteLock				m_RingLock;
	mutable double						m_RadInside;			// Not stored
	mutable double						m_RadOutside;			// Not stored
	AuMatrix4x4							m_ThinkTransform;		// My nu transform calculated in Think.

private:
	mutable QReadWriteLock				m_ResourcesLock;
	mutable AuZoneResource*				m_Resources;			// analysis of resources on this planet, in each zone

private:
	// OpenGL and other transient data that is not serialized.
	GLuint								m_glTexRing;			// OpenGL texture id if I'm a ring.
	AuGLVAO								m_Ring;					// Vertex array object of the ring model.

public:
	DaWorldRing();
	virtual ~DaWorldRing();

protected:
	DaWorldRing(const DaWorldRing &rhs);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	//-----------------
	// DaAtom Functions
public:
	virtual void AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim);

	virtual float AsteroidBeltDensityRecursive(const AuPoint& envLoc) const;

	virtual bool CanBeHarvested(COMMODITYID moduleInUse) const
	{
		return moduleInUse == CID_ProspectorBayModule || moduleInUse == CID_ParticleCollectorBayModule;
	}

	virtual bool CheckChildExit(const AuPoint &, const AuPoint &, const DaAtom*) const
	{
		// parameters: old location, nu location, atom that is being checked
		// Override to decide when to release control of a subatom that is moving.
		return true;	// Rings do not hold onto subatoms.
	}

	virtual DaAtom *Clone() const;

	void Create(double orbitRadius, ORBITZONE orbitZone);

	void CreateForScan(double orbitRadius, ORBITZONE orbitZone);

	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari);

	virtual void DeleteDeviceObjects(AuAtomRenderInfo& ari);

	virtual QString Description() const;

	virtual void FindClosestHarvesterTargetRecursive(COMMODITYID harvesterModule, const AuPoint& gunSysLoc, float gunRange,
			const DaAtom*& targetAtom, double& targetRange) const;

	virtual DaWorldRing* FindRing(const AuPoint& searchLoc) const;

	virtual void InitSystemPosition();

	virtual void ParentDestroyedByAsteroid(DaAtom* nuParent, quint8 orbitIndex, double orbitRadiusAu);

	virtual AuPoint WeaponLockLocation(const AuPoint& shipSysLoc, COMMODITYID harvesterModule) const;

	virtual void Insert(QSqlDatabase& db) const;	// Insert first time database record.

	virtual bool IsEthereal() const
	{
		return true;
	}

	virtual bool IsObstacle(const DaAtom* toWho) const
	{
		Q_UNUSED(toWho);
		return false;
	}

	virtual bool IsVisibleFarWorld() const
	{
		return true;
	}

	virtual bool IsVisibleNearWorld() const
	{
		return true;
	}

	virtual quint64 MemoryUsage() const;

	// Land distance from pole to pole on a sphere, wall to wall on a ringworld.
	virtual double NorthSouthDistance() const
	{
		return 0;
	}

	virtual void PrepareForScan();

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

	virtual QString SystemScanReport(int scanEx) const;

	// Used on client to produce the scan report information available by selecting stars, planets and solar systems on the
	// star map scan report xml export.
	virtual void SystemScanReportXML(QXmlStreamWriter& stream, int scanEx) const;

	// Used by system survey scan report to make a list of the best resources on a star or world.
	// 'atmRes', 'hydRes', 'bioRes' and 'geoRes' are filled with all the natural resources this atom (world or star) has to
	// offer. Key is quality, value is COMMODITYID.
	virtual void SystemScanResources(QMultiMap<quint8, COMMODITYID>& atmRes,
			QMultiMap<quint8, COMMODITYID>& hydRes,
			QMultiMap<quint8, COMMODITYID>& geoRes,
			QMultiMap<quint8, COMMODITYID>& bioRes) const;

	virtual bool IsTicker() const;

	virtual void Tick(const AuElapsedTimer& last, const AuElapsedTimer& now);

	virtual void TickOnClient(const AuElapsedTimer& last);

	virtual unsigned int IsTocker() const;

	virtual void Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans);

	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
			const DaPerson& sceneAtom);

	virtual ATOM_TYPE AtomType() const;

	virtual void Update(QSqlDatabase& db) const;	// Update existing database record.

	//------------------
	// DaWorld Functions
public:
	// Returns true if a compass can provide directional guidance on this world at 'loc', which is relative to this world.
	// Sets 'north' and 'east' to unit vectors from loc that point to the north and east respectively, parallel with the surface.
	// Sets 'up' to a unit vector pointing up from loc with respect to gravity.
	virtual bool CompassVectors(const AuPoint& /*loc*/, AuPoint& /*north*/, AuPoint& /*east*/, AuPoint& /*up*/) const
	{
		return false;
	}

	virtual QString DefaultName(const QString& solarSystemName) const;

	// Resource quality varies by resource zone across the planet.
	// So far this is only used when foraging from the environment.
	virtual quint8 NaturalResourceQuality(COMMODITYID cid, int resourceZone) const;

	virtual int OkToRename(quint32 empireId) const;

	virtual double Period() const;

	virtual void QueryNaturalResources(AuZoneResource& res, int resourceZone) const;

	// Returns the resource zone number of a given location.
	virtual int ResourceZone(const AuPoint& /*loc*/) const
	{
		return 0;
	}

	// Returns the number of resource zones on this world.
	virtual int ResourceZones() const
	{
		return 1;
	}

	virtual quint32 WorldPopulation() const
	{
		return 0;
	}

	virtual quint32 WorldPopulationMax() const
	{
		return 0;
	}

	virtual bool HasWorkerSpecies() const
	{
		return false;
	}

	virtual void SetWorkerSpecies(const AuSpecies& species)
	{
		Q_UNUSED(species);
	}

	virtual AuSpecies WorkerSpecies() const
	{
		return AuSpecies();
	}

	//------------------
	// DaWorldRing Functions
public:
	void CreateRingModel(AuOpenGLFunctions& GLf);

	// Returns true if 'loc' is within the inner and outer radii of this ring, and within the maximum possible ring thickness in Z.
	// 'loc' is in the space of my parent world.
	bool IsLocInRing(const AuPoint& loc) const;

	static DaWorldRing* LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, LOADSUBATOMS loadWhat,
			const AuId& idToLoad=0);

	// Calculates m_ThinkTransform for Tick.
	void ThinkPerformAxialRotation();

	void RenderRing(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool viewerIsClose);

	// Calculates radii of inside and outside edges of ring.
	// Radii are positive values relative to the center line of the ring atom.
	// If the ring can spread 100 feet inside and 200 feet outside, 100 and 200 are returned respectively.
	void Radii(double& inside, double& outside) const;

	inline double RotationSpeedFactor() const
	{
		// Other worlds can range from .1 to 3 times normal speed.
		// This produces 50:50 results above or below 1.
		if(Id().GetID()&1)
			return 1. + 2. * (Id().GetID() % 1001) / 1000.;		// Result range is 1 to 3
		else
			return .1 + .9 * (Id().GetID() % 1001) / 1000.;		// Result range is .1 to 1

		//return (double)((Id().GetID() % 291) + 10) / 100.;		// Result range is .1 to 3, >2/3 of results are above 1.

		//return 3.1 - (double)((Id().GetID() % 291) + 10) / 100.;	// Result range is .1 to 3, >2/3 of results are below 1.
	}

	virtual BODY_TYPE BodyType() const
	{
		return BODY_Ring;
	}

protected:
	virtual int AirTravellerInterval(qint8 morale) const;

	virtual int IndigenousPopulationInterval(qint8 morale) const;
};

#endif // DaWorldRing_H
