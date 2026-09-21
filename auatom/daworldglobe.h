//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daworldglobe.h,v 1.291 2022/05/17 18:30:28 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaWorldGlobe_H
#define DaWorldGlobe_H

#include "auweather.h"
#include "daworldsphere.h"

class Au3dsGLSLModelInstance;
class AuFogOfWar;
class AuZoneResource;
class DaBd;
class DePlanetFauna;
class DePlanetFlora;

class AUATOM_DLL DaWorldGlobe : public DaWorldSphere
{
protected:
	enum CONTROLINPUT
	{
		CI_Forage,
		CI_ForageCID,
		CI_ForageSeed,
	};

private:
	// Unguarded
	quint8								m_byAtmDensity;			// Atmosphere density (0-100)
	ATMTYPE								m_AtmType;				// Atmosphere type.
	qint8								m_byWater;				// Percentage of surface water (0-100)
	quint8								m_OrbitIndex;			// For multiple planets in the same orbit.
	quint8								m_Radius100km;			// Radius in 100s km.
	float								m_WindDir;				// Current wind direction in the render scene.
	float								m_WindForce;			// Current wind force in the render scene.

private:
	mutable QReadWriteLock				m_GlobeLock;
	mutable QColor						m_AtmColor;				// Cached here to save work.
	AuFogOfWar*							m_FogOfWar;				// FogOfWar shroud.
	mutable quint32						m_Population;			// Of buildings and cities. Cached here, not stored.
	AuSpecies*							m_pSentient;			// Indigenous sentient race species.
	double								m_RotationAngle;		// Angle of rotation about the axis.
	double								m_SpinAngle;
	AuPoint								m_SpinAxis;
	mutable double						m_ZoneRadius;			// Calculated, not stored.

	// Animal Encounters Data
	DePlanetFauna*						m_pFauna;				// Animals when CanHaveFauna is true, one entry for each zone.
	DePlanetFlora*						m_pFlora;				// Plant DNA, one entry for each resource zone.
	AuTime								m_tmFloraChange;		// When flora DNA last changed.
	
	// Terrain generation factors unique to this world.
	float								m_TerrainFrequency;		// Noise frequency.
	float								m_TerrainPersFactor;	// Noise scaling factor due to persistence.
	float								m_TerrainPersistence;	// Persistence causes jaggier terrain boundaries as it goes up.
	float								m_TerrainXOff;			// Perlin noise X offset.
	float								m_TerrainYOff;			// Perlin noise Y offset.
	float								m_TerrainWaterFactor;	// Modifier due to water coverage.

	// Think variables.
	double								m_ThinkRotation;		// My nu m_RotationAngle calculated in Think.
	AuMatrix4x4							m_ThinkTransform;		// My nu transform calculated in Think.

private:
	mutable QReadWriteLock				m_ResourcesLock;
	mutable AuZoneResource*				m_Resources;			// analysis of resources on this planet, in each zone

private:
	// OpenGL
	QMap<AuPoint, Au3dsGLSLModelInstance*>	m_PlanetoidModels;	// Key is location relative to my parent, value is model to use.
	QMultiMap<GLuint, QMap<AuPoint, Au3dsGLSLModelInstance*>::iterator> m_PlanetoidsSorted;
	AuAtGlobeVAO						m_AtGlobe;				// Vertex array objects of the globe as seen from the ground.
	AuGLVAO								m_Atmosphere;			// Vertex array object of the atmosphere unit globe for clouds, etc.
	GLuint								m_glTexFarSurface;		// Generated planet surface texture for distant render/surface map.
	GLuint								m_glTexGrass;			// Grass texture generated from local flora.
	GLuint								m_glTexNearSurface;		// Generated planet surface texture for nearby render.
	bool								m_glTexNearValid;		// Set false when the surface texture needs to be regenerated.
	GLuint								m_glTexWx;				// OpenGL weather texture id.
	Au3dsGLSLModelInstance*				m_HengeColumn;
	Au3dsGLSLModelInstance*				m_HengeFirePit;
	Au3dsGLSLModelInstance*				m_HengeRock;
	AuGLVAO								m_NearGlobe;			// Vertex array object of the globe model as seen from orbit.
	AuGLVAO								m_SeaGlobe;				// Vertex array object of the sea surface.
	AuElapsedTimer						m_TerrainCleanupTime;
	AuGLVAO								m_UnderseaGlobe;		// Vertex array object of the sea subsurface.
	bool								m_ViewerNear;			// Near surface texture is updated while this is true.
	AuWeather							m_Wx;					// weather model
	float								m_WxMaxSP;
	float								m_WxMaxVP;
	float								m_WxMinSP;
	float								m_WxMinVP;
	float								m_WxSPOffset;
	float								m_WxVPOffset;
	mutable QImage*						m_Surface;				// Created on client for consoles and map. -1 = requested.

	// Transient data used to manage display of flora.
	QMap<AuResKey, AuGlobeFlora>		m_AquaticGrass;			// Ground cover near m_FloraViewer location.
	AuGLVAO**							m_FloraVAOsHigh;		// High tessellation PLANT_VAOS entries for every species of plant.
	mutable AuPoint						m_FloraViewer;			// Ground location under viewer when flora was made.
	QMap<AuResKey, AuGlobeFlora>		m_Grass;				// Ground cover near m_FloraViewer location.

	// Transient data used to manage display of rocks.
	AuGLVAO**							m_RockVAOs;				// 1 entry for every type of rock.
	mutable AuPoint						m_RockViewer;			// Ground location under viewer when rocks were made.

	// Water offsets for animation of waves.
	float								m_WaterOffX;
	float								m_WaterOffY;

public:
	DaWorldGlobe();
	virtual ~DaWorldGlobe();

protected:
	DaWorldGlobe(const DaWorldGlobe &rhs);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

	//-----------------
	// DaAtom Functions
public:
	// Adjusts 'navLoc' so it is not inside the atmosphere/photosphere of any planets/stars or other objects a navigator would
	// perceive.
	// Result will be a close orbit radius outside of those.
	// 'shipLoc' is the reference loc for deciding where to move 'navLoc'. It is probably the location of a navigator's ship.
	// 'shipLoc' and 'navLoc' are in solar system coordinates.
	// Recursive.
	virtual void AdjustNavLocation(const AuPoint& shipLoc, AuPoint& navLoc) const;

	virtual double AngleToPlanetAt(qint64 msecsSinceEpoch, double& rUYears) const;

	virtual void AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim);

	virtual float AsteroidBeltDensityRecursive(const AuPoint& envLoc) const;

	virtual void BuildingChanged(const AuMatrix4x4& bldM, double radius, AuGLModelMaker& mm);

	virtual bool CanBeHarvested(COMMODITYID moduleInUse) const
	{
		if(moduleInUse == CID_ProspectorBayModule || moduleInUse == CID_WildcatterBayModule)
			return true;
		if(moduleInUse == CID_AtmosphereCondenserBayModule && m_byAtmDensity)
			return true;
		if(moduleInUse == CID_SiphonBayModule && m_byWater)
			return true;
		return false;
	}

	// Returns true if my size permits me to orbit another globe. Larger sizes must orbit a gas giant or a sun.
	inline bool CanSizeOrbitGlobe() const
	{
		return m_Radius100km <= 29;
	}

	virtual bool CheckChildExit(const AuPoint &foldloc, const AuPoint &fnuloc, const DaAtom* who) const;

	virtual bool CheckPeerJoin(const AuPoint &foldloc, const AuPoint &fnuloc, const DaAtom* who);

	virtual DaAtom* Clone() const;

	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari);

	virtual void DeleteDeviceObjects(AuAtomRenderInfo& ari);

	virtual QString Description() const;

	virtual void EmpireDeleted(quint32 empireId);

	virtual void EnumControlPanels(AuQuadArray &aControlPanels, const AuMatrix4x4 &tModel, QVector<qint16>& idPanels,
			const DaAtom* sceneAtom) const;

	virtual void EnumViewPanelControls(qint16 panelId, QList<AuControlButton>& controls, AuAtomRenderInfo& ari) const;

	virtual void Explore(const AuId& avatarId, quint32 empireId, const AuPoint& seaLevelLoc, double radius);

	virtual void ExploreAll(const AuId& avatarId, quint32 empireId);

	virtual void FindClosestHarvesterTargetRecursive(COMMODITYID harvesterModule, const AuPoint& gunSysLoc, float gunRange,
			const DaAtom*& targetAtom, double& targetRange) const;

	virtual DaWorldLL* FindParentWorldNotSatelliteInEnv();

	virtual const DaWorldLL* FindParentWorldNotSatelliteInEnv() const;

	virtual bool FogOfWar(const AuId& avatarId, quint32 empireId, AuFogOfWarData& fow) const;

	virtual bool FogOfWarTest(const AuId& avatarId, quint32 empireId, const AuGlobeLL& ll) const;

	virtual AuPoint WeaponLockLocation(const AuPoint& shipSysLoc, COMMODITYID harvesterModule) const;

	virtual double GlobeRadius() const
	{
		return ::Radius100km(m_Radius100km);
	}

	// Returns acceleration due to gravity in ft/sec^2 at a given cg distance
	// from this planet.
	// The basic formula is...
	//   G = K * V / a^2
	// where...
	//   G is gravity acceleration in ft/sec/sec
	//   K is universal mass constant
	//   V is volume of this planet in cubic feet
	//   a is altitude of object above this planet's center of gravity in feet
	virtual double Gravity(double cgDistance) const
	{
		// Gravity is infinite at zero distance? Let's just return zero to avoid dividing by zero.
		if(cgDistance == 0)
			return 0;
		double cgDist2 = cgDistance * cgDistance;	// CG distance squared
		return MassK() * Volume() / cgDist2;
	}

	// Returns a unit vector pointing down with respect to gravity at a specific location.
	// This is the direction of the gravitational force, if this atom exerts any.
	virtual AuPoint GravityDir(const AuPoint& loc) const
	{
		AuGlobeLL ll;
		ll.SetFromGlobeLocation(loc);
		return ll.GlobeLocation(-1);
	}

	virtual bool HitByAsteroid(quint8 asteroidSize, const AuPoint& asteroidLoc);

	// 'orbitIndex' is my new orbit index so I can become a secondary asteroid of a new belt.
	virtual void ParentDestroyedByAsteroid(DaAtom* nuParent, quint8 orbitIndex, double orbitRadiusAu);

	virtual void InitAnimalEncounterTable(const AuDice& dice);

	virtual void InitSystemPosition();

	virtual void Insert(QSqlDatabase& db) const;	// Insert first time database record.

	virtual bool IsAsteroidBeltPrimary() const
	{
		return m_Radius100km < 10 && !m_OrbitIndex;
	}

	virtual bool IsPlanetoidSize() const
	{
		return m_Radius100km < 10;
	}

	// Returns true if 'loc' is in the teleport area of an active henge.
	// 'from', 'dest', and 'shadowPanels' variables are all output parameters.
	virtual bool IsHengeTeleportLocation(const AuPoint& loc, AuId& fromSysId, quint8& fromSun, quint8& fromArc,
			quint8& shadowPanels, AuId& destSysId, quint8& destSun, quint8& destArc) const;

	virtual bool IsInAsteroidBelt() const
	{
		return BodyType() == BODY_Planetoid;
	}

	// Moons are globes of size 10 to 28. Ommitting 29 allows it to be the smallest titan.
	inline bool IsMoonSize() const
	{
		return m_Radius100km >= 10 && m_Radius100km <= 28;
	}

	virtual bool IsMoonOrPlanetoidSize() const
	{
		return m_Radius100km <= 28;
	}

	virtual quint8 OrbitIndex() const
	{
		return m_OrbitIndex;
	}

	virtual bool IsSightTarget(const DaAtom* toWho) const;

	inline bool IsTwinWorld() const
	{
		return m_Radius100km >= 30 && m_OrbitIndex;
	}

	virtual bool IsVisibleFarWorld() const
	{
		return true;
	}

	virtual bool IsVisibleNearWorld() const
	{
		return true;
	}

	virtual void ListSceneSubatoms(const DaPerson *pSceneAtom, const AuPoint& cameraEnvLoc, DaAtomList &sceneAtoms);

	// Returns the mass of this atom, in pounds.
	// Needed for when things crash into my terrain. Returns a big number.
	virtual float Mass() const
	{
		return 200000000000.f;			// 100,000,000 tons
	}

	virtual quint64 MemoryUsage() const;

	virtual bool MoveSubatom(DaAtom* pMove, const AuPoint& vMove, int elapsedMs);

	// Land distance from pole to pole on a sphere, wall to wall on a ringworld.
	virtual double NorthSouthDistance() const
	{
		return ::Radius100km(m_Radius100km) * PI;
	}

	// Returns actual radius of this object for avoidance when navigating and bay weapon fire obstruction.
	virtual double ObstacleRadius() const
	{
		return AtmosphereRadius();
	}

	// Called by spacecraft to avoid objects.
	// Returns safe radius with a safety margin to avoid this object when navigating.
	// A return value of 0 means this atom is not avoided by spacecraft.
	virtual double PilotAvoidRadius() const
	{
		return AtmosphereRadius() + OrbitAltAboveAtm();
	}

	// Range within which a pilot maintains a slow safe speed limit.
	// Returning non-zero here has an effect on AI pilot. See usage. Obstacle that returns non-zero here is not ignored even if
	// too close. Returning 0 causes AI pilot to ignore this obstacle when too close.
	virtual float PilotSpeedLimitRange() const
	{
		return ZoneRadius();
	}

	double ZoneRadius() const;

	virtual AuPoint PredictLocation(const AuPoint& loc, double secondsFromNow) const;

	virtual void PrepareForScan();

	virtual void ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson *pSceneAtom, const AuGearPos& gearPos,
			qint8 chSelValue, CONTROL_CONTEXT cc, int msElapsed);

	virtual bool Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari);

	virtual void RenderScWorld(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
			const DaPerson* sceneAtom);

	virtual void RenderSc_Shader(AuGLSLScBase* shader, const AuMatrix4x4& projection, const AuMatrix4x4& model,
			AuAtomRenderInfo& ari, const DaPerson* sceneAtom);

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

	virtual void GeologicalFeatureChanged(const AuGeologicalFeature* feat, AuGLModelMaker& mm);

	virtual void RxUpdate(QDataStream& fromServer, bool isFirstUpdate);

	virtual void SampleControlPanelInput(qint16 idPanel, const AuPoint &fPanelLoc, DiControlInputArray &aActiveControls,
			bool bContinuous);

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

	virtual DaAtom* SightTarget(const DaAtom* sightingAtom, const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, DaAtom* pBest,
			AuPoint& sightedLoc, unsigned int& sightedPart, bool& sightedShield, AuPoint* sightedSurfaceNormal) const;

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

	// Only atoms that return true to IsTicker will have their Tick function called.
	virtual bool IsTicker() const;

	virtual void Tick(const AuElapsedTimer& last, const AuElapsedTimer& now);

	virtual void TickOnClient(const AuElapsedTimer& last);

	virtual unsigned int IsTocker() const;

	virtual void Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans);

	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
			const DaPerson& sceneAtom);

	virtual ATOM_TYPE AtomType() const;

	virtual void Update(QSqlDatabase& db) const;	// Update existing database record.

	virtual bool UpdateEnclosureCache() const;

	//------------------
	// DaWorld Functions
public:
	// Return the altitude above mean sea level at a 'loc'.
	virtual double AltitudeMSL(const AuPoint& loc) const
	{
		return CGAltitude(loc) - SeaLevelRadius();
	}

	virtual QColor AtmosphereColor() const;

	// Returns the height of the top of the atmosphere above sea level, whether or not there is any actual atmosphere density there.
	virtual double AtmosphereHeightMSL() const
	{
		return AtmosphereTopMSL();
	}

	// Not really a radius on a ringworld. Returns the height of the atmosphere above sea level instead, which is the ring surface.
	virtual double AtmosphereRadius() const
	{
		return SeaLevelRadius() + AtmosphereTopMSL();
	}

	virtual ATMTYPE AtmosphereType() const
	{
		return m_AtmType;
	}

	// Test whether this planet's size can support an atmosphere.
	inline bool CanHaveAtmosphere() const
	{
		return m_Radius100km >= 29 && m_Radius100km <= 128;
	}

	// Test whether this planet can support any animal life.
	// Query CanHaveFauna using a terrain elevation to be more specific.
	virtual bool CanHaveFauna() const
	{
		// Test for planet size, not moon.
		if(!CanHaveAtmosphere())
			return false;

		// Test atmosphere type, including orbit zone.
		if(!HasBreathableAtmosphere())
			return false;	// Atmosphere does not support animals.

		// Passed all the tests.
		return true;
	}

	// Test whether this planet can support any plant life.
	// Query CanHaveFlora using a terrain elevation to be more specific.
	virtual bool CanHaveFlora() const
	{
		// Test for planet size, not moon or gas giant.
		if(!CanHaveAtmosphere())
			return false;

		// Test atmosphere type.
		if(m_byAtmDensity < 10 || m_AtmType > ATMTYPE_Exotic)
			return false;

		// Test orbit zone.
		if(OrbitZone() < OZONE_Inner || OrbitZone() > OZONE_Outer)
			return false;

		// Passed all the tests.
		return true;
	}

	virtual double CGAltitude(const AuPoint& loc) const
	{
		// Planet only needs to test vector length to know if a location is inside the atmosphere.
		return loc.Length();
	}

	// Returns true if a compass can provide directional guidance on this world at 'loc', which is relative to this world.
	// Sets 'north' and 'east' to unit vectors from loc that point to the north and east respectively, parallel with the surface.
	// Sets 'up' to a unit vector pointing up from loc with respect to gravity.
	virtual bool CompassVectors(const AuPoint& loc, AuPoint& north, AuPoint& east, AuPoint& up) const;

	// NOTE: Don't call this function unless you understand the implications.
	virtual void CopyFlora(const DePlanetFlora* flora);

	virtual void CreateFloraTextures(AuGLEffects& re, int resourceZone);

	virtual QString DefaultName(const QString& solarSystemName) const;

	// Direction this world rotates on its axis.
	// Return 1 or -1.
	virtual int DirectionOfRotation() const
	{
		// Use the GenSeed to determine if this body rotates clockwise or counter-clockwise.
		if(GenSeed()&0x01)
			return -1;
		else
			return 1;
	}

	// Changes the altitude of a location to a specific 'cgaltitude' by raising/lowering it relative to gravity.
	virtual void ElevateLoc(AuPoint& loc, double cgaltitude) const
	{
		loc.Unit(cgaltitude);
	}

	// Changes the altitude of a location to a specific 'altitudeMSL' by raising/lowering it relative to gravity.
	virtual void ElevateLocMSL(AuPoint& loc, double altitudeMSL) const
	{
		loc.Unit(SeaLevelRadius() + altitudeMSL);
	}

	virtual AuPoint FindOrbitSpawnLoc(const AuPoint& startLoc, const AuId& shipId, double shipRadius, double orbitAlt,
			bool avoidCreatures) const;

	// Recursive search for the resources needed by starting players.
	virtual void FindStartupResources(int& eludium, int& lumenite);

	// Returns glow strength added to all flora on this world.
	virtual int FloraGlow() const;

	// Returns the GL texture id for a given resource zone. Will create the texture if it hasn't been created yet.
	virtual GLuint FloraTexture(int resourceZone) const;

	virtual bool HasAtmosphere() const
	{
		return m_byAtmDensity ? true : false;
	}

	// Returns true if the orbit zone, atmosphere type and atmosphere density are good enough to breath without apparatus.
	virtual bool HasBreathableAtmosphere() const
	{
		if((m_AtmType == ATMTYPE_Standard || m_AtmType == ATMTYPE_Tainted) &&
				(m_byAtmDensity >= 15 && m_byAtmDensity <= 85) &&
				(OrbitZone() >= OZONE_Inner && OrbitZone() <= OZONE_Outer))
			return true;
		return false;
	}

	virtual unsigned char Hydrographics() const
	{
		return m_byWater;
	}

	// Returns the altitude of a location inside the atmosphere of this planet or ringworld.
	// On a planet, the altitude is above the center of the planet and will never be negative.
	// Returns -100000 if the location is not inside the atmosphere.
	virtual double InAtmosphereCGAltitude(const AuPoint& loc) const
	{
		// Planet only needs to test vector length to know if a location is inside the atmosphere.
		double alt = loc.Length();
		if(alt <= AtmosphereRadius())
			return alt;
		return -100000;
	}

	virtual bool IsHabitable(HABITABILITY hab) const;

	// Slightly pickier criteria than IsHabitablePlanet.
	virtual bool IsSuitableHomePlanet() const;

	// Returns atmosphere density at sea level in the range 0..1.
	virtual float MSLAtmosphereDensity() const
	{
		return (float)m_byAtmDensity / 100.0f;
	}

	// Resource quality varies by resource zone across the planet.
	// So far this is only used when foraging from the environment.
	virtual quint8 NaturalResourceQuality(COMMODITYID cid, int resourceZone) const;

	// Test if all the flags in all the cities on this planet are controlled by the 'empireId' and if none of those
	// cities is an occupied city. This planet must have at least one city controlled by 'empireId'.
	// Returns:
	//   0 = ok to rename
	//   1 = no cities on the planet
	//   2 = occupied city is present on this world or a moon of this world if this is a gas giant
	//   3 = flag of other empire is present on this world or a moon of this world if this is a gas giant
	virtual int OkToRename(quint32 empireId) const;

	virtual double Period() const;

	// Returns the maximum population allowed in each resource zone before cities apply an overpopulation penalty to morale.
	// This is fairly aggressive so people will need to spread into the galaxy rather than infest the same planet.
	// This is necessary to help prevent a single solar system from taking too long in the tick cycle.
	// 'cityId' is used to grandfather in old cities before population max was reduced. It can be null to treat it as a nu city.
	virtual int PopulationMax() const;

	virtual bool TestForLiquid(const AuMatrix4x4& mBld, const AuPoint& siteCen, const AuPoint& siteBoxRadius) const;

	virtual quint32 WorldPopulation() const
	{
		m_GlobeLock.lockForRead();
		quint32 v = m_Population;
		m_GlobeLock.unlock();
		if(v == (quint32)-1)
		{
			v = CalcWorldPopulation();
			m_GlobeLock.lockForWrite();
			m_Population = v;
			m_GlobeLock.unlock();
		}
		return v;
	}

	virtual void WorldPopulationChanged()
	{
		m_GlobeLock.lockForWrite();
		m_Population = (quint32)-1;
		m_GlobeLock.unlock();
	}

	virtual void QueryNaturalResources(AuZoneResource& res, int resourceZone) const;

	// Determine the intensity of sunlight striking the surface location from each sun.
	// Light level is adjusted for water depth.
	// This does not consider sloping terrain or shadows.
	// In habitable zone, noon sun intensity is 1.0.
	virtual float QuerySunlight(const AuPoint& fSurfLoc) const;

	// Queries for information about the terrain at a specific longitude and latitude.
	virtual void QueryTerrain(AuTerrainInfo& ter, const AuGlobeLL& ll) const;

	// Queries for information about the terrain at a specific 3D location.
	virtual void QueryTerrainAtLoc(AuTerrainInfo& ter, const AuPoint& loc) const;

	// Returns the number of resource zones on this world.
	virtual int ResourceZones() const
	{
		if(m_Radius100km >= 100)		// This threshold is above the largest titan size.
			return 4;
		else if(m_Radius100km >= 50)	// This threshold is below the smallest home world size.
			return 3;
		else if(m_Radius100km >= 30)	// This threshold is above the largest moon size.
			return 2;
		else
			return 1;					// Moons and planetoids.
	}

	// Spherical worlds return their radius. Ringworlds return 0.
	virtual double SeaLevelAltitude() const
	{
		return SeaLevelRadius();
	}

	// Returns the temperature of this world at sea level, in kelvin.
	// 'longLat' holds longitude and latitude in x and y, in radians.
	virtual float SeaLevelTempAt(const AuGlobeLLF& longLat, float solarOrbitRadius, float sunLightAtten) const;	// in kelvin

	// Returns the precipitation conditions present.
	// 'll' is the location relative to this world.
	// 'altitudeMSL' is the MSL altitude of the desired weather reading.
	// 'relHumidity' is an output parameter set to the relative humidity.
	// Relative humidity is the ratio of vapor pressure to saturation pressure. <=1 is not precipitating. >1 is precipitating.
	// 0-1 = No precip
	// 1-1.25 = Light precip
	// 1.25-1.5 = Moderate precip
	// 1.5-1.75 = Heavy precip
	// 1.75+ = Very Heavy precip
	// 'oat' is an output parameter indicating outside air temperature in celsius.
	virtual void WeatherAt(const AuGlobeLLF& ll, float altitudeMSL, float& relHumidity, float& oat, float& bar) const;

	// 'longLat' is longitude latitude angles, in radians
	virtual QPointF WindAt(const AuGlobeLLF& longLat) const;

	//------------------
	// DaWorldLL Functions
public:
	virtual void CreateAtGlobeSurfaceTriangles(QList<AuGlobeTriAltVegN>& tris, float& maxAltMSL, float& minAltMSL) const;

	virtual void CreateFlora(QMap<AuResKey, AuGlobeFlora>& grass, QMap<AuResKey, AuGlobeFlora>& shrubs,
			QMap<AuResKey, AuGlobeTree>& trees, QMap<AuResKey, AuGlobeFlora>& aquaticgrass,
			QMap<AuResKey, AuGlobeFlora>& aquaticshrubs, const AuPoint& groundLoc, quint32 worldGenSeed, int floraGlow) const;

	virtual void CreateRocks(QMap<AuResKey, AuGlobeRock>& rocks, const AuPoint& groundLoc, quint32 worldGenSeed, float range,
			bool visibleOnly) const;

	// 'killed' becomes true when the thread who called me is killed, so I can stop midstream.
	virtual void CreateAtTerrainTriangles(QList<AuGlobeTriAltVegNPave>& tris, const AuGlobeLLTri& terTri, const bool& killed) const;

	// Animals when CanHaveFauna is true, one entry for each zone.
	virtual DePlanetFauna* LockFaunaForWrite()
	{
		m_GlobeLock.lockForWrite();
		return m_pFauna;
	}

	virtual void UnlockFauna()
	{
		m_GlobeLock.unlock();
	}

	// Plant DNA, one entry for each resource zone.
	virtual DePlanetFlora* LockFloraForWrite()
	{
		m_GlobeLock.lockForWrite();
		return m_pFlora;
	}

	// Plant DNA, one entry for each resource zone.
	virtual const DePlanetFlora* LockFlora() const
	{
		m_GlobeLock.lockForRead();
		return m_pFlora;
	}

	virtual void UnlockFlora() const
	{
		m_GlobeLock.unlock();
	}

	// Returns the base geological altitude of terrain at 'll'.
	// The altitude is not adjusted for city developments or geological features.
	virtual double GeologicalTerrainAltitudeMSL(const AuGlobeLL& ll) const;

	// Lists all possible commodities that a particular mine can extract.
	virtual void QueryMineResources(const DaBd* mine, QList<COMMODITYID>& rockCIDs) const;

	// Lists all possible commodities that a particular well can extract.
	virtual void QueryWellResources(const DaBd* well, QList<COMMODITYID>& wellCIDs) const;

	//------------------
	// DaWorldGlobe Functions
public:
	// Adjust camera location for a subatom.
	// 'subatom' is one of my subatoms.
	// 'camera' is the matrix to the camera relative to the subatom.
	// Returns true if 'camera' is modified.
	virtual bool ConstrainCamera(const DaAtom* subatom, AuMatrix4x4& camera) const;

	// 'opposite' is true when this world is to orbit opposite its normal position. This is used when placing an extra world in the
	// habitable zone opposite the first habitable world.
	void Create(BODY_TYPE type, HABITABILITY hab, const AuDice& dice, double orbitRadius, ORBITZONE orbitZone, quint8 orbitIndex);

	// Used when creating the at-globe model.
	virtual void CreateAtSurfaceTriangles(QList<AuGlobeTerrainTriCen>& tricens) const;

	void CreateForScan(int type, HABITABILITY hab, const AuDice& dice, double orbitRadius, ORBITZONE orbitZone, quint8 orbitIndex);

	virtual void CreateFlatGlobeVAOTriangle(QList<AuGlobeTriAltVeg>& tris, const AuGlobeLL& ll0, const AuGlobeLL& ll1,
			const AuGlobeLL& ll2) const;

	virtual void CreateNearGlobeSurfaceTriangles(QList<AuGlobeTriTexNShineSnow>& tris) const;

	// Like CreateSurfaceTriangles except the poles have more triangles for making a flat projection of the surface.
	virtual void CreateSurfaceTrianglesFlat(AuGlobeLLTriList& tris, int eqdivs, float& maxAltMSL, float& minAltMSL) const;

	virtual int GroundLevelEquatorDivisions() const
	{
		return EquatorDivisions100km(m_Radius100km);
	}

	virtual int SeaLevelEquatorDivisions() const
	{
		return EquatorDivisions100km(m_Radius100km);
	}

	// Called during creation of this planet to determine how many moons it has. Not called on moons.
	int GenMoonCount(const AuDice& dice) const;

	// Call when something has changed that affects the appearance of the globe. e.g. City lights go on/off.
	virtual void InvalidateNearGlobeTexture()
	{
		m_glTexNearValid = false;
	}

	static DaWorldGlobe* LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, LOADSUBATOMS loadWhat,
			const AuId& idToLoad=0);

	// 'fromLoc' and 'toLoc' are relative to this terrain patch.
	// Returns next location in direction of path that will avoid obstructions to get to 'toLoc' from 'fromLoc'.
	// Returns 'toLoc' if the path is not obstructed.
	// Returns 0,0 if no path could be found.
	virtual AuPoint NextPathLocation(const AuPoint& fromLoc, const AuPoint& toLoc) const;

	virtual void ResourceDistributionAt(const AuMatrix4x4& mBld, const AuPoint& siteCen, const AuPoint& siteBoxRadius,
			unsigned int& rocks, unsigned int& shrubs, unsigned int& trees, float& waterCoverage, unsigned int maxICareAbout) const;

	// Returns a zone index for flora, fauna, and resources.
	// 'longitude' is an angle in radians.
	virtual int ResourceZone(float longitude) const
	{
		longitude += PIf;
		while(longitude < 0)
			longitude += PIf*2;
		const int zones = ResourceZones();
		return (int)(longitude*zones/(PIf*2)) % zones;
	}

	// Returns a zone index for flora, fauna, and resources.
	// 'longitude' is an angle in radians.
	virtual int ResourceZone(double longitude) const
	{
		longitude += PI;
		while(longitude < 0)
			longitude += PI*2;
		const int zones = ResourceZones();
		return (int)(longitude*zones/(PI*2)) % zones;
	}

	// Returns 0,0 if no landing site is found in the zone.
	virtual AuPoint ResourceZoneLandingSite(int zone) const;

	// Returns the commodity id of significant rock outcroppings at 'll'.
	// This is used for spotting and foraging. 'll' is rounded to the nearest actual rock grid location and that is the rock that
	// is returned, using RockAt.
	// Returns CID_Invalid if no significant rocks are present.
	virtual COMMODITYID SpotRockAt(const AuGlobeLL& ll, quint32 worldGenSeed) const;

	// Returns the commodity id of significant shrubs at 'll'.
	// This is used for spotting and foraging. 'll' is rounded to the nearest actual shrub grid location and that is the shrub that
	// is returned, using ShrubAt.
	// 'aquaticPlant' is set true if the plant is such.
	// Returns CID_Invalid if no significant shrubs are present.
	virtual COMMODITYID SpotShrubAt(const AuGlobeLL& ll, quint32 worldGenSeed, bool& aquaticPlant) const;

	// Returns the commodity id of significant trees outcroppings at 'll'.
	// This is used for spotting and foraging. 'll' is rounded to the nearest actual tree grid location and that is the tree that
	// is returned, using TreeAt.
	// Returns CID_Invalid if no significant trees are present.
	virtual COMMODITYID SpotTreeAt(const AuGlobeLL& ll, quint32 worldGenSeed) const;

	// Only valid on the client.
	virtual const QImage* SurfaceImage() const;

	virtual double SurfaceImageHeight() const;

	// Returns a surface texture for display into a world map status display screen.
	virtual GLuint SurfaceTexture() const;

	// Calculates m_ThinkRotation and m_ThinkTransform for Tick.
	void ThinkPerformAxialRotation();

	virtual bool HasWorkerSpecies() const;

	void SetFogOfWar(const AuFogOfWar& fow);

	virtual void SetWorkerSpecies(const AuSpecies& species);

	virtual AuSpecies WorkerSpecies() const;

	inline bool IsHabitablePlanetOrMoon() const
	{
		return IsHabitableOrbit() && HasBreathableAtmosphere();
	}
	
	// Test whether animals can exist at a specific terrain level on this
	// planet. Inner and outer worlds restrict animals to the seas and coasts.
	virtual bool CanHaveFaunaAtElev(TER_ELEV_TYPE terType) const
	{
		// Test for planet size, not moon or gas giant.
		if(!CanHaveAtmosphere())
			return false;

		// Test atmosphere type, including orbit zone.
		if(!HasBreathableAtmosphere())
			return false;	// Atmosphere does not support animals.

		// Orbit-zone specific tests.
		switch(OrbitZone())
		{
		case OZONE_Inner:
			// Animals exist in the sea and on the low lands on inner worlds.
			if(terType < TER_Lowlands)
				return false;
			break;
		case OZONE_Habitable:
			// No further restrictions in habitable zone.
			break;
		case OZONE_Outer:
			// Animals exist in the sea and on the shore on outer worlds.
			if(terType < TER_Coastal)
				return false;
			break;
		case OZONE_Inferno:
		case OZONE_Frigid:
		case OZONE_DeepSpace:
			return false;
		}

		// Passed all the tests.
		return true;
	}

	// Test whether plants can exist at a specific terrain level on this planet.
	// Inner and outer worlds restrict plants to the coasts.
	// Returns 0=no, 1=yes, 2=no unless in enclosure
	int CanHaveFloraAtAltitude(float altitudeMSL) const;

	virtual quint16 Radius100km() const
	{
		return m_Radius100km;
	}

	virtual double SeaLevelRadius() const	// Returns the sea-level radius in feet.
	{
		return ::Radius100km(m_Radius100km);
	}

	// Returns the volume of this planet in cubic feet.
	inline double Volume() const
	{
		return Volume100km(m_Radius100km);
	}

	virtual BODY_TYPE BodyType() const;

	QColor ChartColor() const;

	static QColor ChartColor(ORBITZONE eZone, unsigned char byWater);

	// Returns angle orbit is inclined off XY plane.
	//AuPoint OrbitInclination() const;

	// Altitude at which TER_Alpine terrain starts.
	// This is only valid on worlds where CanHaveSnow is true, since TER_Peak terrain is snow.
	inline float AlpineAltitudeMSL() const
	{
		return TreeLineMSL() + AltitudeIncrement()*.4f;
	}

	// Altitude at which TER_Peak terrain starts.
	// This is only valid on worlds where CanHaveSnow is true, since TER_Peak terrain is snow.
	inline float PeakAltitudeMSL() const
	{
		return TreeLineMSL() + AltitudeIncrement()*.75f;
	}

	// Returns an altitude above which there are no plants. This is the target altitude when making terrain for the tree line of
	// mountains. It is also used in the CanHaveFlora test.
	// The altitude returned does not depend on the presense of trees or plants. It is solely based on the weather layer altitudes.
	virtual float TreeLineMSL() const
	{
		// The tree line is between the cloud layers on worlds larger than 2.
		// This keeps the mountains a little shorter on moons.
		if(m_Radius100km >= 30)
			return (AuWeather::LayerAltitude(*this, AuWeather::LowClouds) +
					AuWeather::LayerAltitude(*this, AuWeather::MiddleClouds))/2;
		else
			return AuWeather::LayerAltitude(*this, AuWeather::LowClouds);
	}

	static int TextureTileSize()
	{
		return 1024/VEG_Types;
	}

public:
	// Axis rotation for globes.
	// Shadow square rotation for ringworlds.
	inline double AngleOfRotation() const
	{
		m_GlobeLock.lockForRead();
		double v = m_RotationAngle;
		m_GlobeLock.unlock();
		return v;
	}

	virtual double RotationSpeedFactor() const;

protected:
	void CreateWeatherRenderData(AuGLEffects& re, AuGLModelMaker& mm);

	void RenderAtmosphere(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelMaker& mm,
			int viewerContext, double viewerAlt, bool clouds, float atmIllumination, float atmOpacity);

	QImage CreateAtFrigidLookupTable(float maxAltMSL, float minAltMSL, GLuint terElevTex[5]) const;

	QImage CreateAtInfernoLookupTable(float maxAltMSL, float minAltMSL, GLuint terElevTex[5]) const;

	// Creates a texture for use in globe rendering when viewer is at this planet. All terrain is textured; water/lava/liquid are
	// rendered separately.
	// 'maxAltMSL' is the highest MSL altitude in the triangle data that will be sent to the shader.
	// 'minAltMSL' is the lowest MSL altitude in the triangle data that will be sent to the shader.
	GLuint CreateAtLookupTable(AuGLEffects &re, float maxAltMSL, float minAltMSL, GLuint terElevTex[5]) const;

	QImage CreateAtMoonLookupTable(float maxAltMSL, float minAltMSL, GLuint terElevTex[5]) const;

	QImage CreateAtVegetationLookupTable(float maxAltMSL, float minAltMSL, GLuint terElevTex[5]) const;

	QImage CreateFarFrigidLookupTable(float maxAltMSL, float minAltMSL, GLuint terElevTex[5]) const;

	QImage CreateFarInfernoLookupTable(float maxAltMSL, float minAltMSL, GLuint terElevTex[5]) const;

	// Creates a texture for use in far globe rendering. Sea surfaces turn to water/lava/liquid color.
	// 'maxAltMSL' is the highest MSL altitude in the triangle data that will be sent to the shader.
	// 'minAltMSL' is the lowest MSL altitude in the triangle data that will be sent to the shader.
	GLuint CreateFarLookupTable(AuGLEffects &re, float maxAltMSL, float minAltMSL, GLuint terElevTex[5]) const;

	QImage CreateFarMoonLookupTable(float maxAltMSL, float minAltMSL, GLuint terElevTex[5]) const;

	QImage CreateFarVegetationLookupTable(float maxAltMSL, float minAltMSL, GLuint terElevTex[5]) const;

	// Creates the map of the planet that is shown on the transporter console and the locator map window.
	void CreateSurfaceMap(AuGLEffects& re);

	void CreateTerrainData(const AuDice& dice);

protected:
	virtual int AirTravellerInterval(qint8 morale) const;

	virtual int IndigenousPopulationInterval(qint8 morale) const;

	double CalcTransform(AuMatrix4x4& tBody, AuPoint& spinAxis, double& spinAngle, qint64 msecsSinceEpoch) const;

	void CalcSpinAxis(AuPoint& spinAxis) const;

	void CreateAsteroidBelt(const AuDice& dice, quint8 orbitIndex);

	void CreateAtGlobe(AuGLEffects& re, AuGLModelMaker& mm);

	void CreateAtGlobeSurfaceTriangle(AuGlobeTriAltVegN& tri, const AuGlobeLL& ll0, const AuGlobeLL& ll1,
			const AuGlobeLL& ll2, QMap<AuGlobeLL, AuGlobeTerrainAltInfo>& terAltInfo) const;

	//void CreateAtGlobeVAO(float& maxAltMSL, float& minAltMSL, const AuPoint& viewer);

	void CreateAtTerrainSurfaceTriangle(QList<AuGlobeTriAltVegNPave>& tris, const AuGlobeLL& ll0, const AuGlobeLL& ll1,
			const AuGlobeLL& ll2, QMap<AuGlobeLL, AuGlobeTerrainAltInfo>& terAltInfo) const;

	void CreateEnvironment(HABITABILITY hab, const AuDice& dice);

	void CreateGeologicalFeatures(const AuDice& dice);

	void CreateGrass(QMap<AuResKey, AuGlobeFlora>& grass, QMap<AuResKey, AuGlobeFlora>& aquaticgrass, const AuPoint& groundLoc,
			const AuPoint& vx, const AuPoint& vy, quint32 worldGenSeed, int floraGlow) const;

	void CreateGrassTexture(AuGLEffects& re, AuGLModelMaker& mm);

	void CreateTitan(HABITABILITY hab, const AuDice& dice);

	// Finish creation of this body as a moon.
	// Note that the result may not reply true to IsMoonSize. A small percentage of moons become planets in this call, if they reach
	// their absolute maximum size of 29. That allows them to have atmosphere, fauna and flora.
	void CreateMoon(const AuDice& dice);

	void CreateNearGlobeSurfaceTriangle(QList<AuGlobeTriTexNShineSnow>& tris, const AuGlobeLL& ll0, const AuGlobeLL& ll1,
			const AuGlobeLL& ll2, QMap<AuGlobeLL, AuGlobeTerrainAltInfo>& terAltInfo) const;

	//void CreateNearGlobeVAO();

	// Finish creation of this body as a planet, includes ringworld type planets.
	// Does not create moons, gas giants, titans or asteroid belts. Refer to CreateMoon, CreateGasGiant, CreateTitan and
	// CreateAsteroidBelt for those.
	void CreatePlanet(HABITABILITY hab, const AuDice& dice);

	// iViewSizeX is the pixel width of the resulting texture. The height of the texture will be iViewSizeX/2.
	GLuint CreatePlanetTexture(AuAtomRenderInfo& ari, int iViewSizeX, bool drawCities);

	void CreateRocks(QMap<AuResKey, AuGlobeRock>& rocks, const AuPoint& groundLoc, const AuPoint& vx, const AuPoint& vy,
			quint32 worldGenSeed, float range, bool visibleOnly) const;

	void CreateShrubs(QMap<AuResKey, AuGlobeFlora>& shrubs, QMap<AuResKey, AuGlobeFlora>& aquaticshrubs, const AuPoint& groundLoc,
			const AuPoint& vx, const AuPoint& vy,
			quint32 worldGenSeed, int floraGlow) const;

	// Tessellates the surface of a globe fairly uniformly, with 'eqdivs' vertices at the equator and a single vertex at each pole.
	// 'maxAltMSL' and 'minAltMSL' are return values.
	// Triangles are added to 'tris' such that the first and last vertices are never at the same latitude. The second vertex will be
	// at the latitude of either the first vertex or the last vertex.
	void CreateSurfaceTriangles(AuGlobeLLTriList& tris, int eqdivs, float& maxAltMSL, float& minAltMSL) const;

	void CreateTrees(QMap<AuResKey, AuGlobeTree>& trees, const AuPoint& groundLoc, const AuPoint& vx, const AuPoint& vy,
			quint32 worldGenSeed, int floraGlow) const;

	void DeleteFloraDeviceObjects(AuGLEffects& re, AuGLModelMaker& mm);

	// Returns false if nuLoc or oldLoc is outside the atmosphere.
	bool FilterDestinationLoc(DaAtom* moveAtom, const AuPoint& oldLoc, AuPoint& nuLoc, int elapsedMs) const;

	// Calculates an index into the single dimensional m_FloraVAOsHigh array.
	// Each plant VAO array is treated like an array that is m_FloraVAOs[ResourceZones][LatitudeRegions][PlantType][PlantVAOIndex]
	// 'resourceZone' is the resource zone containing the plant. Range is 0 to ResourceZones()-1
	// 'latitudeRegion' is the AuFloraDNA::ZONE containing the plant.
	// 'plantType' is 0 = grass, 1 to ZONE_SHRUBS = shrubs, ZONE_SHRUBS+1 to ZONE_SHRUBS+ZONE_TREES = trees
	// 'plantVAOIndex' is a number from 0 to PLANT_VAOS-1 to allow for 3D model variations of each different plant specie.
	inline int FloraVAOIndex(int resourceZone, int latitudeRegion, int plantType, int plantVAOIndex) const
	{
		Q_ASSERT(resourceZone >= 0 && resourceZone < ResourceZones());
		Q_ASSERT(latitudeRegion >= 0 && latitudeRegion < (int)AuFloraDNA::Zones);
		Q_ASSERT(plantType >= 0 && plantType < 1 + ZONE_SHRUBS + ZONE_TREES);
		Q_ASSERT(plantVAOIndex >= 0 && plantVAOIndex < PLANT_VAOS);

		int index = plantVAOIndex +
			plantType * PLANT_VAOS +
			latitudeRegion * (1 + ZONE_SHRUBS + ZONE_TREES) * PLANT_VAOS +
			resourceZone * AuFloraDNA::Zones * (1 + ZONE_SHRUBS + ZONE_TREES) * PLANT_VAOS;

#if !defined(QT_NO_DEBUG)
		int maxFauna = PLANT_VAOS * AuFloraDNA::Zones * (1 + ZONE_TREES + ZONE_SHRUBS) * ResourceZones();
		Q_ASSERT(index >= 0 && index < maxFauna);
#endif
		return index;
	}

	// 'normal' is a return value
	inline AuPoint GetNearGlobeSurfaceLocation(const AuGlobeLL& ll, float altitudeMSL, AuPoint& normal) const
	{
		// Calculate above-water locations at their appropriate altitude. This makes the model bumpy. It is not the altitude
		// used by the shader to determine texturing. That is placed artificially on each vertex to get deep sea colors even though
		// the sea surface is flat.
		if(altitudeMSL > 0)
		{
			normal = TerrainNormal(ll);
			return ll.GlobeLocation(SeaLevelRadius() + altitudeMSL);
		}

		// Undersea locations are at the sea surface for smooth sea surface on far globe model.
		normal = ll.GlobeLocation(1);	// Normal is globe location at radius 1, for flat sea surface.
		return normal * SeaLevelRadius();		// Location is at sea level radius.
	}

	void GrabSurfaceTextureImage(AuGLEffects& re, QImage& fboImage, GLuint glTexture) const;

	// 'rockLL' is a rounded globe location.
	COMMODITYID GuaranteedRockAt(const AuGlobeLLF& rockLL, int resourceZone) const;

	// Returns a random rounded globe location for a guaranteed rock that is not already in use by another guaranteed rock.
	AuGlobeLLF GuaranteedRockLocationResLocked(const AuDice& dice, int eqDivs, int resourceZone) const;

	void LoadCachedSurfaceImage(QImage& surface, int iViewSizeX) const
	{
		LoadCachedSurfaceImage(Id(), surface, iViewSizeX, m_tmFloraChange);
	}

	void LoadCachedSurfaceImage(const AuId& worldId, QImage& surface, int iViewSizeX, const AuTime& floraChange) const;

	void QueryNaturalResources(AuZoneResource& res, int resourceZone, quint32 worldGenSeed) const;

	// Render the globe when the viewer is inside the atmosphere.
	void RenderAtGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
			const AuPoint& viewerPos, float ambientLight, float fogDensity, float fogR, float fogG, float fogB);

	// Render local plants relative to the viewer's position.
	void RenderFlora(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelMaker& mm,
			const AuPoint& viewerPos, float ambientLight, float fogDensity, float fogR, float fogG, float fogB);

	void RenderHenges(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
			float ambientR, float ambientG, float ambientB, float fogDensity, float fogR, float fogG, float fogB);

	// Render the globe when the viewer is outside the atmosphere and close enough to see terrain details.
	void RenderNearGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari, float ambientLight);

	// Render the globe when the viewer is inside the atmosphere and close enough to see terrain details but we don't have an 'at'
	// globe model to show them.
	void RenderNearGlobeFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
			float ambientLight, float fogDensity, float fogR, float fogG, float fogB);

	// Render local rocks relative to the viewer's position.
	void RenderRocks(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelMaker& mm,
			const AuPoint& viewerPos, float ambientLight, float fogDensity, float fogR, float fogG, float fogB);

	inline void RenderSea(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool eyeUnderWater,
			bool cgUnderWater, float illumination, float ambientLight, float fogDensity, float fogR, float fogG, float fogB,
			bool viewTopDown)
	{
		if(eyeUnderWater)
		{
			// Render only the transparent sea surface model.
			// Must reverse cull direction because eye is under the water.
			re.glFrontFace(GL_CCW);
			RenderSeaGlobe(projection, model, re, false, illumination, ambientLight, cgUnderWater, fogDensity, fogR, fogG, fogB,
					viewTopDown);
			re.glFrontFace(GL_CW);
		}
		else
		{
			// Render both sea surface models to hide the bottom.
			RenderSeaGlobe(projection, model, re, true, illumination, ambientLight, cgUnderWater, fogDensity, fogR, fogG, fogB,
					viewTopDown);
		}
	}

	void RenderSeaGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool bothLayers,
			float illumination, float ambientLight, bool showUndersideOfSurface, float fogDensity, float fogR, float fogG,
			float fogB, bool viewTopDown);

	// Returns the commodity id of significant rock outcroppings at 'll'.
	// Returns CID_Invalid if no significant rocks are present at 'll'.
	COMMODITYID RockAt(const AuGlobeLL& ll, quint32 worldGenSeed, int resourceZone) const;

	// NOTE: 'plantLL' has already been rounded to a proper location. See CreateShrubs.
	COMMODITYID ShrubAt(const AuGlobeLL& plantLL, quint32 worldGenSeed) const;

	// Returns snow coverage in the range of 0 to 1.
	inline float SnowCoverage(float altMSL, float latitude, int inEnclosure) const
	{
		// Enclosures are too warm for snow.
		if(inEnclosure)
			return 0;

		// Worlds must have proper water and atmosphere conditions to support snow.
		if(!CanHaveSnow())
			return 0;

		// Altitudes above the peak level are saturated with snow.
		float peakMSL = PeakAltitudeMSL();
		if(altMSL >= peakMSL)
			return 1;

		// Calculate snow coverage due to poles.
		float snow;
		if(fabs(latitude) + altMSL/100*DEG2RADf > PolarLatitude())
		{
			if(altMSL <= 0)
				snow = .3f;
			else
				snow = .8f + .2f * altMSL/peakMSL;
		}
		else
			snow = 0;

		// In alpine altitudes, increase snow as altitude increases.
		float alpineMSL = AlpineAltitudeMSL();
		if(altMSL > alpineMSL)
		{
			float alpineSnow = (altMSL - alpineMSL)/(peakMSL - alpineMSL);		// Snow increases with altitude here.
			if(alpineSnow > snow)
				snow = alpineSnow;
		}

		// Return the snow coverage.
		return snow;
	}

	// NOTE: 'plantLL' has already been rounded to a proper location. See CreateTrees.
	COMMODITYID TreeAt(const AuGlobeLL& plantLL, quint32 worldGenSeed) const;

	void UpdateNearSurfaceTexture(AuAtomRenderInfo& ari);

	quint8 VegetationAt(const AuGlobeLL& ll, float terAltMSL, int inEnclosure, int irrigated) const;

	virtual double UndevelopedVirginTerrainAltitudeMSL(const AuGlobeLL& ll) const;

	virtual double VirginTerrainAltitudeMSL(const AuGlobeLL& ll, WORLDTEX_TYPE& pavementType, AuTime& newest) const;

	virtual double VirginTerrainAltitudeMSL(const AuGlobeLL& ll, bool& hideRocks, int& hideGrass, bool& hideShrubs, bool& hideTrees,
			WORLDTEX_TYPE& pavementType, AuTime& newest) const;
};

#endif // DaWorldGlobe_H
