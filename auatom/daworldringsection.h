//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daworldringsection.h,v 1.224 2022/05/17 18:44:25 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaWorldRingSection_H
#define DaWorldRingSection_H

#include "ausphere.h"
#include "auweather.h"
#include "auzoneresource.h"
#include "daworldsphere.h"
#include "deflora.h"

class Au3dsGLSLModelInstance;
class AuFogOfWar;
class AuZoneResource;
class DaBd;
class DePlanetFauna;
class DePlanetFlora;

// DaWorldRingSection is a section of a ringworld.
// 
// A ring section is a section of a cylinder. The coordinate system of a ring section places the origin at the center of the
// section hull wall. That point is exactly 'OrbitRadius' distance from the center of the parent star. That is the "sea level"
// radius of the section. The center of the sun in the section's coordinate space is always (0, 0, OrbitRadius()). The hull is
// parallel to the Y axis and it curves away from the X axis toward the center as distance from center increases.
//
//                       +Z
//                        * Sun
//                        |
//                        |
//                        |
//                        |
//            180W        |        180E
//                =       |       =
//                 ==     |     == Hull
//                   ===  |  ===
//        -X  ----------==o==---------  +X
//                   Atom Origin
//
// AuGlobeLL objects are used to reference surface locations on ringworld sections. The entire latitude range describes the Y axis
// of the terrain, where 90 degrees north is at the north wall of the ringworld and 90 degrees south is at the south wall of the
// ringworld. The longitude range describes the curving hull of the ringworld, where 180 degrees west is the edge of the ringworld
// section terrain that is farthest along the -X axis and 180 east is the edge of the ringworld section terrain that is farthest
// along the +X axis.

class DaWorldRing;

class AUATOM_DLL DaWorldRingSection : public DaWorldLL
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
	bool								m_ControlCenterActive;	// true if control center is active. Not stored in db.
	quint8								m_SectionIndex;			// Section index among all ringworld sections.
	float								m_WindDir;				// Current wind direction in the render scene.
	float								m_WindForce;			// Current wind force in the render scene.

private:
	mutable QReadWriteLock				m_RingSectionLock;
	mutable QColor						m_AtmColor;				// Cached here to save work.
	mutable quint32						m_Population;			// Of buildings and cities. Cached here, not stored.
	AuSpecies*							m_pSentient;			// Indigenous sentient race species.
	double								m_RotationAngle;		// Shadow square rotation relative to the ring.
	double								m_RotationAngleOffset;	// Used by designer preview.
	mutable double						m_SectionAngle;			// Sweep angle of section; all sections of a ringworld are the same.

	// Animal Encounters Data
	DePlanetFauna*						m_pFauna;				// Animals when CanHaveFauna is true, one entry for each zone.
	DePlanetFlora*						m_pFlora;				// Plant DNA, one entry for each resource zone.
	AuTime								m_tmFloraChange;		// When flora DNA last changed.

	// Terrain generation factors unique to this world.
	float								m_TerrainFrequency;		// Noise frequency in X.
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
	// Terrain texturing
	AuAtGlobeVAO						m_AtGlobe;				// Vertex array objects of the globe as seen from the ground.
	AuGLVAO								m_Atmosphere;			// Vertex array object for the atmosphere.
	AuGLVAO								m_Clouds;				// Vertex array object for clouds.
	GLuint								m_glTexFarSurface;		// Generated planet surface texture for distant render/surface map.
	GLuint								m_glTexGrass;			// Grass texture generated from local flora.
	GLuint								m_glTexNearSurface;		// Generated planet surface texture for nearby render.
	bool								m_glTexNearValid;		// Set false when the surface texture needs to be regenerated.
	Au3dsGLSLModelInstance*				m_HengeColumn;
	Au3dsGLSLModelInstance*				m_HengeFirePit;
	Au3dsGLSLModelInstance*				m_HengeRock;
	AuGLVAO								m_Hull;					// Vertex array object of my hull
	AuGLVAO								m_NearGlobe;			// Vertex array object of the globe model as seen from orbit.
	AuGLVAO								m_Obelisk;				// Obelisk used at control center, only on ring arc 0.
	AuGLVAO								m_SeaGlobe;				// Vertex array object of the sea surface.
	AuGLVAO								m_ShadowSquare;			// Vertex array object of my shadow square.
	AuElapsedTimer						m_TerrainCleanupTime;
	AuGLVAO								m_UnderseaGlobe;		// Vertex array object of the sea subsurface.
	GLuint								m_glTexWx;				// OpenGL weather texture id.
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
	DaWorldRingSection();
	virtual ~DaWorldRingSection();

protected:
	DaWorldRingSection(const DaWorldRingSection &rhs);

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

	virtual void AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim);

	virtual quint32 BuildingVolumeLimit() const;

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

	virtual bool CheckChildExit(const AuPoint &foldloc, const AuPoint &fnuloc, const DaAtom* who) const;

	virtual bool CheckPeerJoin(const AuPoint &foldloc, const AuPoint &fnuloc, const DaAtom* who);

	virtual DaAtom* Clone() const;

	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari);

	virtual void DeleteDeviceObjects(AuAtomRenderInfo& ari);

	virtual QString Description() const;

	virtual void EnumControlPanels(AuQuadArray &aControlPanels, const AuMatrix4x4 &tModel, QVector<qint16>& idPanels,
			const DaAtom* sceneAtom) const;

	virtual void EnumViewPanelControls(qint16 panelId, QList<AuControlButton>& controls, AuAtomRenderInfo& ari) const;

	virtual void Explore(const AuId& avatarId, quint32 empireId, const AuPoint& seaLevelLoc, double radius);

	virtual void ExploreAll(const AuId& avatarId, quint32 empireId);

	virtual bool FetchConstructionMaterials(DeConstructionProcess& cp, const DaBd* forBuilding);

	virtual bool FetchManufacturingMaterials(DeManufacturingProcess &mp, const AuPoint& loc, bool isDark, INDUSTRYID iid,
			bool isUndersea, bool inEnclosure, quint8& elecSubQ, quint8& waterSubQ);

	virtual void FindClosestHarvesterTargetRecursive(COMMODITYID harvesterModule, const AuPoint& gunSysLoc, float gunRange,
			const DaAtom*& targetAtom, double& targetRange) const;

	virtual DaWorldRing* FindRing(const AuPoint& searchLoc) const;

	virtual bool FogOfWar(const AuId& avatarId, quint32 empireId, AuFogOfWarData& fow) const;

	virtual bool FogOfWarTest(const AuId& avatarId, quint32 empireId, const AuGlobeLL& ll) const;

	virtual AuPoint WeaponLockLocation(const AuPoint& shipSysLoc, COMMODITYID harvesterModule) const;

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
		// CG distance on a ringworld is relative to sea level, which is the ring's orbit radius.
		cgDistance += RadiusStandard();	// Offset sea level radius for proper gravity calculation.

		// Gravity is infinite at zero distance? Let's just return zero to avoid dividing by zero.
		if(cgDistance == 0)
			return 0;

		// Code this so the preprocessor calculates the Volume part of the return value ahead of time.
		double cgDist2 = cgDistance * cgDistance;	// CG distance squared
		return MassK() * DaWorldSphere::Volume100km(RadiusStandard100km()) / cgDist2;
	}

	// Returns a unit vector pointing down with respect to gravity at a specific location.
	// This is the direction of the gravitational force, if this atom exerts any.
	virtual AuPoint GravityDir(const AuPoint& loc) const
	{
		// Sun is always at (0,0,OrbitRadius()) relative to a ringworld section, which is a slice of a cylinder.
		AuPoint pvec(loc.x, loc.z - OrbitRadius());				// 2D vector from ringworld center to loc; this is a cylinder.
		pvec.Unit_2D();
		return AuPoint(pvec.x, 0, pvec.y);						// Swizzle to proper UP direction.
	}

	virtual bool HitByAsteroid(quint8 asteroidSize, const AuPoint& asteroidLoc);

	virtual void InitAnimalEncounterTable(const AuDice& dice);

	virtual void InitSystemPosition();

	virtual void Insert(QSqlDatabase& db) const;	// Insert first time database record.

	// Returns true if 'loc' is in the activation area of the control center, for testing preon location for activation.
	bool IsActivationLocation(const AuPoint& loc) const;

	// Note that only the section with the control center is ever active, in this sense.
	inline bool IsControlCenterActive() const
	{
		return m_ControlCenterActive;
	}

	// Returns true if 'loc' is in the teleport area of an active henge.
	// 'from', 'dest', and 'shadowPanels' variables are all output parameters.
	virtual bool IsHengeTeleportLocation(const AuPoint& loc, AuId& fromSysId, quint8& fromSun, quint8& fromArc,
			quint8& shadowPanels, AuId& destSysId, quint8& destSun, quint8& destArc) const;

	virtual bool IsPathBlocked(const AuPoint& oloc, const AuPoint& nloc, AuPoint& hitLoc, AuPoint& hitNormal, const DaAtom* toWho);

	virtual bool IsSightTarget(const DaAtom* toWho) const;

	// Returns true if the shadow panels are currently blocking the beams from the cooling towers to the sun.
	inline bool IsStargateBeamBlocked() const
	{
		return AngleOfRotation() < -SectionAngle()/4. || AngleOfRotation() > SectionAngle()/4.;
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
		return TerrainYSize();
	}

	// Range within which a pilot maintains a slow safe speed limit.
	// Returning non-zero here has an effect on AI pilot. See usage. Obstacle that returns non-zero here is not ignored even if
	// too close. Returning 0 causes AI pilot to ignore this obstacle when too close.
	virtual float PilotSpeedLimitRange() const
	{
		return FeetPerOrbit() / 2;
	}

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
		return CGAltitude(loc);
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
		return AtmosphereTopMSL();							// Atmosphere depth is nominally 1000 miles above sea.
	}

	virtual ATMTYPE AtmosphereType() const
	{
		return m_AtmType;
	}

	// Test whether this planet can support any animal life.
	// Query CanHaveFauna using a terrain elevation to be more specific.
	virtual bool CanHaveFauna() const
	{
		return true;					// Ringworlds always have animals.
	}

	// Test whether this planet can support any plant life.
	// Query CanHaveFlora using a terrain elevation to be more specific.
	virtual bool CanHaveFlora() const
	{
		return true;					// Ringworlds always have plants.
	}

	// Returns true if control center is not active and shadow panels do not block activation.
	inline bool CanOpenStargate() const
	{
		return !IsControlCenterActive() && !IsStargateBeamBlocked();
	}

	virtual double CGAltitude(const AuPoint& loc) const
	{
		double ringworldRadius = OrbitRadius();
		AuPoint pvec(loc.x, loc.z - ringworldRadius);			// 2D vector from ringworld center to loc; this is a cylinder.
		return ringworldRadius - pvec.Length_2D();
	}

	// Returns true if a compass can provide directional guidance on this world at 'loc', which is relative to this world.
	// Sets 'north' and 'east' to unit vectors from loc that point to the north and east respectively, parallel with the surface.
	// Sets 'up' to a unit vector pointing up from loc with respect to gravity.
	virtual bool CompassVectors(const AuPoint& loc, AuPoint& north, AuPoint& east, AuPoint& up) const;

	// NOTE: Don't call this function unless you understand the implications.
	virtual void CopyFlora(const DePlanetFlora* flora);

	virtual void CreateFloraTextures(AuGLEffects& re, int resourceZone);

	virtual QString DefaultName(const QString& solarSystemName) const;

	// Changes the altitude of a location to a specific 'cgaltitude' by raising/lowering it relative to gravity.
	virtual void ElevateLoc(AuPoint& loc, double cgaltitude) const
	{
		double ringworldRadius = OrbitRadius();
		AuPoint pvec(loc.x, loc.z - ringworldRadius);			// 2D vector from ringworld center to loc; this is a cylinder.
		pvec.Unit_2D(ringworldRadius - cgaltitude);				// Calculate loc at altitude.
		loc.x = pvec.x;
		// loc.y remains unchanged
		loc.z = pvec.y + ringworldRadius;						// Swizzle to proper UP direction.
	}

	// Changes the altitude of a location to a specific 'altitudeMSL' by raising/lowering it relative to gravity.
	virtual void ElevateLocMSL(AuPoint& loc, double altitudeMSL) const
	{
		double ringworldRadius = OrbitRadius();
		AuPoint pvec(loc.x, loc.z - ringworldRadius);			// 2D vector from ringworld center to loc; this is a cylinder.
		pvec.Unit_2D(ringworldRadius - altitudeMSL);			// Calculate loc at altitude.
		loc.x = pvec.x;
		// loc.y remains unchanged
		loc.z = pvec.y + ringworldRadius;						// Swizzle to proper UP direction.
	}

	virtual AuPoint FindOrbitSpawnLoc(const AuPoint& startLoc, const AuId& shipId, double shipRadius, double orbitAlt,
			bool avoidCreatures) const;

	// Recursive search for the resources needed by starting players.
	virtual void FindStartupResources(int& eludium, int& lumenite);

	// Returns the GL texture id for a given resource zone. Will create the texture if it hasn't been created yet.
	virtual GLuint FloraTexture(int resourceZone) const;

	// Ringworlds always have atmosphere.
	virtual bool HasAtmosphere() const
	{
		return true;
	}

	// Ringworlds always have breathable atmosphere.
	virtual bool HasBreathableAtmosphere() const
	{
		return true;
	}

	virtual unsigned char Hydrographics() const
	{
		return m_byWater;
	}

	// Returns the altitude of a location inside the atmosphere of this planet or ringworld.
	// Returns -100000 if the location is not inside the atmosphere.
	virtual double InAtmosphereCGAltitude(const AuPoint& loc) const
	{
		// Insure loc is inside the atmosphere envelope on the sides.
		if(fabs(loc.y) > TerrainYSize()/2.)
			return -100000;				// Loc is outside the sides in Y.

		// Insure loc is inside the atmosphere envelope above and below.
		double alt = CGAltitude(loc);
		if(alt > AtmosphereRadius() || alt < MinRadius())
			return -100000;

		// Loc is inside the atmosphere envelope. We ignore the ends because we expect these sections to form a full ringworld
		// end to end. If any are missing, we will need an end cap or force field of some kind.
		return alt;
	}

	virtual bool IsHabitable(HABITABILITY hab) const;

	// Returns depth below sea level for ringworld as a negative number.
	inline double MinRadius() const
	{
		return -4. * AltitudeIncrement();
	}

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

	// Returns the maximum population allowed in each resource zone before cities apply an overpopulation penalty to morale.
	// This is fairly aggressive so people will need to spread into the galaxy rather than infest the same planet.
	// This is necessary to help prevent a single solar system from taking too long in the tick cycle.
	// 'cityId' is used to grandfather in old cities before population max was reduced. It can be null to treat it as a nu city.
	virtual int PopulationMax() const;

	virtual quint32 WorldPopulation() const
	{
		m_RingSectionLock.lockForRead();
		quint32 v = m_Population;
		m_RingSectionLock.unlock();
		if(v == (quint32)-1)
		{
			v = CalcWorldPopulation();
			m_RingSectionLock.lockForWrite();
			m_Population = v;
			m_RingSectionLock.unlock();
		}
		return v;
	}

	virtual void WorldPopulationChanged()
	{
		m_RingSectionLock.lockForWrite();
		m_Population = (quint32)-1;
		m_RingSectionLock.unlock();
	}

	virtual quint32 WorldPopulationMax() const;

	virtual BODY_TYPE BodyType() const
	{
		return BODY_Ringworld;
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
		return 4;
	}

	// Spherical worlds return their radius. Ringworlds return 0.
	virtual double SeaLevelAltitude() const
	{
		return 0;
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
	// Returns the amount of altitude change in feet per whole terrain data altitude increment.
	virtual double AltitudeIncrement() const
	{
		return RadiusStandard()/100.;	// Based on Earth sized globe.
	}

	virtual int GroundLevelEquatorDivisions() const;

	virtual int SeaLevelEquatorDivisions() const;

	// Animals when CanHaveFauna is true, one entry for each zone.
	virtual DePlanetFauna* LockFaunaForWrite()
	{
		m_RingSectionLock.lockForWrite();
		return m_pFauna;
	}

	virtual void UnlockFauna()
	{
		m_RingSectionLock.unlock();
	}

	// Plant DNA, one entry for each resource zone.
	virtual DePlanetFlora* LockFloraForWrite()
	{
		m_RingSectionLock.lockForWrite();
		return m_pFlora;
	}

	// Plant DNA, one entry for each resource zone.
	virtual const DePlanetFlora* LockFlora() const
	{
		m_RingSectionLock.lockForRead();
		return m_pFlora;
	}

	virtual void UnlockFlora() const
	{
		m_RingSectionLock.unlock();
	}

	// Returns the base geological altitude of terrain at 'll'.
	// The altitude is not adjusted for city developments, geological features or ringworld structural elements.
	virtual double GeologicalTerrainAltitudeMSL(const AuGlobeLL& ll) const;

	// Returns true if the build location at 'buildLL' and 'buildAltMSL' falls within 'radius' of hitting a zone where construction
	// is not permitted.
	virtual bool IsForbiddenZone(const AuGlobeLLF& buildLL, double buildAltMSL, double radius) const;

	// Lists all possible commodities that a particular mine can extract.
	virtual void QueryMineResources(const DaBd* mine, QList<COMMODITYID>& rockCIDs) const;

	// Lists all possible commodities that a particular well can extract.
	virtual void QueryWellResources(const DaBd* well, QList<COMMODITYID>& wellCIDs) const;

	virtual AuGlobeLLF RandomLL(const AuDice& dice) const;

	virtual double RotationAngleOffset() const
	{
		m_RingSectionLock.lockForRead();
		double v = m_RotationAngleOffset;
		m_RingSectionLock.unlock();
		return v;
	}

	// Sets the rotation angle offset for only this arc section.
	void SetArcRotationAngleOffset(double rotAngleOffset)
	{
		m_RingSectionLock.lockForWrite();
		m_RotationAngleOffset = rotAngleOffset;
		m_RingSectionLock.unlock();
	}

	// Sets rotation angle offset for all arc sections subordinate to my parent star, just me if no parent star.
	virtual void SetRotationAngleOffset(double rotAngleOffset);

	//------------------
	// DaWorldRingSection Functions
public:
	// Only called on the section that has the control center, the reference section.
	void ActivateControlCenter();

	virtual void ActivateHengeContaining(const AuPoint& containingLoc);

	// Activates the henge portal(s) on this arc segment. Creates a henge portal if none exists.
	// This signals each interconnecting system to create/activate the destination henge portal.
	void ActivateHenges();

	virtual void BuildingChanged(const AuMatrix4x4& bldM, double radius, AuGLModelMaker& mm);

	// Creats the henge portal on this arc segment if none exists.
	// This signals the interconnecting system to create the destination henge portal even if mine already exists.
	void CreateHenges();

	// Only called on the section that has the control center, the reference section.
	void DeactivateControlCenter();

	// Deactivates all henges on this arc section.
	void DeactivateHenges();

	// Adjust camera location for a subatom.
	// 'subatom' is one of my subatoms.
	// 'camera' is the matrix to the camera relative to the subatom.
	// Returns true if 'camera' is modified.
	virtual bool ConstrainCamera(const DaAtom* subatom, AuMatrix4x4& camera) const;

	virtual void CreateAtGlobeSurfaceTriangles(QList<AuGlobeTriAltVegN>& tris, float& maxAltMSL, float& minAltMSL) const;

	// Used when creating the at-globe model.
	virtual void CreateAtSurfaceTriangles(QList<AuGlobeTerrainTriCen>& tricens) const;

	// 'killed' becomes true when the thread who called me is killed, so I can stop midstream.
	virtual void CreateAtTerrainTriangles(QList<AuGlobeTriAltVegNPave>& tris, const AuGlobeLLTri& terTri, const bool& killed) const;

	void CreateFirstRingworldSection(const AuDice& dice, double orbitRadiusAu, ORBITZONE orbitZone);

	void CreateFirstRingworldSectionForScan(const AuDice& dice, double orbitRadiusAu, ORBITZONE orbitZone);

	void CreateFlatGlobeVAOTriangle(QList<AuGlobeTriAltVeg>& tris, const AuGlobeLL& ll0, const AuGlobeLL& ll1,
			const AuGlobeLL& ll2) const;

	virtual void CreateFlora(QMap<AuResKey, AuGlobeFlora>& grass, QMap<AuResKey, AuGlobeFlora>& shrubs,
			QMap<AuResKey, AuGlobeTree>& trees, QMap<AuResKey, AuGlobeFlora>& aquaticgrass,
			QMap<AuResKey, AuGlobeFlora>& aquaticshrubs, const AuPoint& groundLoc, quint32 worldGenSeed, int floraGlow) const;

	virtual void CreateNearGlobeSurfaceTriangles(QList<AuGlobeTriTexNShineSnow>& tris) const;

	void CreateRingworldSection(const AuDice& dice, int sectionIndex, const DaWorldRingSection& firstSection);

	void CreateRingworldSectionForScan(const AuDice& dice, int sectionIndex, const DaWorldRingSection& firstSection);

	virtual void CreateRocks(QMap<AuResKey, AuGlobeRock>& rocks, const AuPoint& groundLoc, quint32 worldGenSeed, float range,
			bool visibleOnly) const;

	// Tessellates the surface of a globe fairly uniformly, with 'eqdivs' vertices at the equator and a single vertex at each pole.
	// 'maxAltMSL' and 'minAltMSL' are return values.
	// Triangles are added to 'tris' such that the first and last vertices are never at the same latitude. The second vertex will be
	// at the latitude of either the first vertex or the last vertex.
	void CreateSurfaceTriangles(AuGlobeLLTriList& tris, int eqdivs, float& maxAltMSL, float& minAltMSL) const;

	// Returns the location on the surface of the planet at ll.
	// Checks for permanent terrain-like features such as the control center dais on a ringworld.
	virtual AuPoint GetSurfaceLocation(const AuGlobeLLF& ll) const
	{
		return LocOfLL(ll, SurfaceAltitudeMSL(ll));
	}

	virtual AuPoint GetSurfaceLocation(const AuGlobeLLF& ll, float altAGL) const
	{
		return LocOfLL(ll, SurfaceAltitudeMSL(ll) + altAGL);
	}

	// Returns the location on the surface of the planet at ll.
	// Checks for permanent terrain-like features such as the control center dais on a ringworld.
	virtual AuPoint GetSurfaceLocation(const AuGlobeLL& ll) const
	{
		return LocOfLL(ll, SurfaceAltitudeMSL(ll));
	}

	virtual AuPoint GetSurfaceLocation(const AuGlobeLL& ll, double altAGL) const
	{
		return LocOfLL(ll, SurfaceAltitudeMSL(ll) + altAGL);
	}

	// Returns the location on the surface of the planet at ll.
	// Does not check for permanent terrain-like features such as the control center dais on a ringworld.
	virtual AuPoint GetTerrainLocation(const AuGlobeLLF& ll) const
	{
		return LocOfLL(ll, TerrainAltitudeMSL(ll));
	}

	virtual AuPoint GetTerrainLocation(const AuGlobeLLF& ll, float altAGL) const
	{
		return LocOfLL(ll, TerrainAltitudeMSL(ll) + altAGL);
	}

	// Returns the location on the surface of the planet at ll.
	// Does not check for permanent terrain-like features such as the control center dais on a ringworld.
	virtual AuPoint GetTerrainLocation(const AuGlobeLL& ll) const
	{
		return LocOfLL(ll, TerrainAltitudeMSL(ll));
	}

	virtual AuPoint GetTerrainLocation(const AuGlobeLL& ll, double altAGL) const
	{
		return LocOfLL(ll, TerrainAltitudeMSL(ll) + altAGL);
	}

	// Call when something has changed that affects the appearance of the globe. e.g. City lights go on/off.
	virtual void InvalidateNearGlobeTexture()
	{
		m_glTexNearValid = false;
	}

	// Shape for AuGlobeLL linear surface calculations.
	virtual LLSHAPE LLShape() const
	{
		return LL_Cylinder;
	}

	static DaWorldRingSection* LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, LOADSUBATOMS loadWhat,
			const AuId& idToLoad=0);

	virtual AuPoint GlobeLocOfLL(const AuGlobeLL& ll) const
	{
		return LocOfLL(ll);
	}

	virtual AuPoint GlobeLocOfLL(const AuGlobeLLF& ll) const
	{
		return LocOfLL(ll);
	}

	// Location returned is at sea level.
	virtual AuPoint LocOfLL(const AuGlobeLL& ll) const
	{
		double xang = (ll.Longitude()/PI) * SectionAngle()/2.;					// Sweep angle to X location.
		AuPoint loc = AuPoint(0, 0, -OrbitRadius()).RotateY(-xang);				// Sea level center location relative to the sun.
		loc.y = (ll.Latitude() / (PI/2.)) * TerrainYSize()/2.;					// Y is an easy linear calculation.
		loc.z += OrbitRadius();													// Flip Z to be relative to the ring section.
		return loc;
	}

	// Location returned is at sea level.
	virtual AuPoint LocOfLL(const AuGlobeLLF& ll) const
	{
		double xang = (double)(ll.Longitude()/PIf) * SectionAngle()/2.;			// Sweep angle to X location.
		AuPoint loc = AuPoint(0, 0, -OrbitRadius()).RotateY(-xang);				// Sea level center location relative to the sun.
		loc.y = (double)(ll.Latitude() / (PIf/2.f)) * TerrainYSize()/2.;		// Y is an easy linear calculation.
		loc.z += OrbitRadius();													// Flip Z to be relative to the ring section.
		return loc;
	}

	// Location returned is at 'altitude' above/below sea level.
	virtual AuPoint LocOfLL(const AuGlobeLL& ll, double altitude) const
	{
		double xang = (ll.Longitude()/PI) * SectionAngle()/2.;					// Sweep angle to X location.
		AuPoint loc = AuPoint(0, 0, -OrbitRadius()).RotateY(-xang);				// Sea level center location relative to the sun.
		if(altitude != 0)
			loc.Unit(OrbitRadius() - altitude);									// Raise to desired altitude.
		loc.y = (ll.Latitude() / (PI/2.)) * TerrainYSize()/2.;					// Y is an easy linear calculation.
		loc.z += OrbitRadius();													// Flip Z to be relative to the ring section.
		return loc;
	}

	// Location returned is at 'altitude' above/below sea level.
	virtual AuPoint LocOfLL(const AuGlobeLLF& ll, double altitude) const
	{
		double xang = (double)(ll.Longitude()/PIf) * SectionAngle()/2.;			// Sweep angle to X location.
		AuPoint loc = AuPoint(0, 0, -OrbitRadius()).RotateY(-xang);				// Sea level center location relative to the sun.
		if(altitude != 0)
			loc.Unit(OrbitRadius() - altitude);									// Raise to desired altitude.
		loc.y = (double)(ll.Latitude() / (PIf/2.f)) * TerrainYSize()/2.;		// Y is an easy linear calculation.
		loc.z += OrbitRadius();													// Flip Z to be relative to the ring section.
		return loc;
	}

	// Calculates 'll' from 'loc', which is in the space of this ringworld section.
	// Returns false if 'loc' is outside the bounds of the ringworld section.
	virtual bool LLOfLoc(AuGlobeLL& ll, const AuPoint& loc) const
	{
		// Calculate latitude of loc as a linear fraction of terrain Y size, in the range -1 to 1.
		double latFrac = loc.y / (TerrainYSize()/2.);
		if(fabs(latFrac) > 1.)
			return false;										// loc is outside Y bounds of ringworld section.

		// Calculate longitude of loc as a fraction of ringworld section angle, in the range -1 to 1.
		double lonFrac = AuPoint(OrbitRadius() - loc.z, loc.x).ZRotation() / (SectionAngle()/2.);
		if(fabs(lonFrac) > 1.)
			return false;										// loc is outside sweep angle of ringworld section.

		// Calculate latitude and longitude into normal ranges.
		ll.SetLatitude(latFrac * (PI/2.));						// PI/2 to -PI/2
		ll.SetLongitude(lonFrac * PI);							// PI to -PI
		return true;
	}

	// Calculates 'll' from 'loc', which is in the space of this ringworld section.
	// Returns false if 'loc' is outside the bounds of the ringworld section.
	virtual bool LLOfLoc(AuGlobeLLF& ll, const AuPoint& loc) const
	{
		// Calculate latitude of loc as a linear fraction of terrain Y size, in the range -1 to 1.
		float latFrac = (float)(loc.y / (TerrainYSize()/2.));
		if(fabsf(latFrac) > 1.f)
			return false;										// loc is outside Y bounds of ringworld section.

		// Calculate longitude of loc as a fraction of ringworld section angle, in the range -1 to 1.
		float lonFrac = (float)(AuPoint(OrbitRadius() - loc.z, loc.x).ZRotation() / (SectionAngle()/2.));
		if(fabsf(lonFrac) > 1.f)
			return false;										// loc is outside sweep angle of ringworld section.

		// Calculate latitude and longitude into normal ranges.
		ll.SetLatitude(latFrac * (PIf/2.f));					// PI/2 to -PI/2
		ll.SetLongitude(lonFrac * PIf);							// PI to -PI
		return true;
	}

	// 'fromLoc' and 'toLoc' are relative to this terrain patch.
	// Returns next location in direction of path that will avoid obstructions to get to 'toLoc' from 'fromLoc'.
	// Returns 'toLoc' if the path is not obstructed.
	// Returns 0,0 if no path could be found.
	virtual AuPoint NextPathLocation(const AuPoint& fromLoc, const AuPoint& toLoc) const;

	// Returns the number of map increments in the north/south direction corresponding to 'EquatorDivisions' equator divisions.
	inline int NorthSouthDivisions() const
	{
		return NorthSouthDivisions(SeaLevelEquatorDivisions());
	}

	// Lists cities that have a development that falls within 'radius' of 'll'.
	// 'cityIds' and 'cityNames' are output parameters, appended with information about cities found in the area.
	void QueryCities(const AuGlobeLLF& ll, double radius, AuIdList& cityIds, QStringList& cityNames) const;

	void RenderHullStatue(const AuMatrix4x4& projection, const AuMatrix4x4& model, double statueScale, AuGLEffects& re,
			float ambientLight, float fogDensity, float fogR, float fogG, float fogB);

	void RenderShadowSquareStatue(const AuMatrix4x4& projection, const AuMatrix4x4& model, double statueScale, AuGLEffects& re,
			float ambientLight, float fogDensity, float fogR, float fogG, float fogB);

	void RenderTerrainStatue(const AuMatrix4x4& projection, const AuMatrix4x4& model, double statueScale, AuGLEffects& re,
			float ambientLight, float fogDensity, float fogR, float fogG, float fogB, bool isActive);

	virtual void ResourceDistributionAt(const AuMatrix4x4& mBld, const AuPoint& siteCen, const AuPoint& siteBoxRadius,
			unsigned int& rocks, unsigned int& shrubs, unsigned int& trees, float& waterCoverage, unsigned int maxICareAbout) const;

	// Returns a zone index for flora, fauna, and resources.
	// 'longitude' is an angle in radians.
	virtual int ResourceZone(float longitude) const
	{
		longitude += PIf;				// Offset to remain consistent with the old style DaWorldGlobe resource zone longitudes.
		while(longitude < 0)
			longitude += PIf*2;
		const int zones = ResourceZones();
		return (int)(longitude*zones/(PIf*2)) % zones;
	}

	// Returns a zone index for flora, fauna, and resources.
	// 'longitude' is an angle in radians.
	virtual int ResourceZone(double longitude) const
	{
		longitude += PI;				// Offset to remain consistent with the old style DaWorldGlobe resource zone longitudes.
		while(longitude < 0)
			longitude += PI*2;
		const int zones = ResourceZones();
		return (int)(longitude*zones/(PI*2)) % zones;
	}

	// Returns 0,0 if no landing site is found in the zone.
	virtual AuPoint ResourceZoneLandingSite(int zone) const;

	// Returns the number of sections in a complete ringworld based on my current orbit radius.
	inline int RingworldSections() const
	{
		return RingworldSections(OrbitRadiusAu());
	}

	// Returns the number of sections in a complete ringworld of radius 'radOrbitAu'.
	inline static int RingworldSections(double radOrbitAu)
	{
		double rsC = radOrbitAu * FeetPerAu() * 2. * PI;		// Circumference of the ringworld.
		return (int)ceil(rsC / MinimumSectionLength());			// Number of sections so each has minimum length.
	}

	virtual void GeologicalFeatureChanged(const AuGeologicalFeature* feat, AuGLModelMaker& mm);

	// Rounds a location to the nearest globe longitude latitude vertex.
	// Nearest vertex is the one that would result from tessellating the globe using CreateSurfaceTriangles, passing it 'eqdivs'.
	virtual AuGlobeLL RoundedGlobeLoc(const AuGlobeLL& locLL, int eqdivs) const;

	// Rounds a location to the nearest globe longitude latitude vertex.
	// Nearest vertex is the one that would result from tessellating the globe using CreateSurfaceTriangles, passing it 'eqdivs'.
	virtual AuGlobeLLF RoundedGlobeLoc(const AuGlobeLLF& locLL, int eqdivs) const;

	// Returns the arc sweep angle of this section. All sections of a ringworld have the same arc angle.
	inline double SectionAngle() const
	{
		// This value is used so much and it never changes so it is cached into a mutable variable.
		m_RingSectionLock.lockForRead();
		if(m_SectionAngle == 0)
		{
			m_RingSectionLock.unlock();

			m_RingSectionLock.lockForWrite();
			m_SectionAngle = (PI*2) / RingworldSections();
		}
		double v = m_SectionAngle;
		m_RingSectionLock.unlock();
		return v;
	}

	virtual quint8 OrbitIndex() const
	{
		return m_SectionIndex;
	}

	inline quint8 SectionIndex() const
	{
		return m_SectionIndex;
	}

	// Returns the sun angle when configuring shaders, based on the sun's current 'sunShadowAngle'.
	inline float ShadowPanelSunAngle(float sunShadowAngle) const
	{
		return sunShadowAngle + (float)(AngleToRingSection() + SectionAngle()/4.);
	}

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

	// Returns the altitude of the surface at ll relative to MSL.
	// Checks for permanent terrain-like features such as the control center dais on a ringworld.
	virtual double SurfaceAltitudeMSL(const AuGlobeLL& ll) const;

	// Returns the altitude of the surface at ll relative to MSL.
	// Checks for permanent terrain-like features such as the control center dais on a ringworld.
	virtual double SurfaceAltitudeMSL(const AuGlobeLLF& ll) const
	{
		return SurfaceAltitudeMSL(AuGlobeLL(ll.Longitude(), ll.Latitude()));
	}

	// Only valid on the client.
	virtual const QImage* SurfaceImage() const;

	virtual double SurfaceImageHeight() const
	{
		return TerrainYSize();
	}

	// Returns a surface texture for display into a world map status display screen.
	virtual GLuint SurfaceTexture() const;

	// Size of the terrain area of this ring section, end-to-end.
	// X axis follows the curve of the section. This is the E/W direction on the ringworld.
	// This value is the length of that curve based on the sweep angle and orbit radius of the entire ringworld.
	inline double TerrainXSize() const
	{
		//double ringC = OrbitRadius() * 2 * PI;					// Circumference of ringworld.
		//return ringC * SectionAngle() / (PI*2);					// My slice of the circumference at sea level.
		//return OrbitRadius() * (PI*2) * SectionAngle() / (PI*2);
		return OrbitRadius() * SectionAngle();
	}

	// Size of the terrain area of this ring section, between the walls.
	// Y axis is straight across the band of the ringworld. This is the N/S direction on the ringworld.
	static double TerrainYSize()
	{
		// TerrainXSize is based on this value so a wider Y size results in fewer sections needed to make an entire ringworld.
		return RadiusStandard() * PI;
	}

	virtual bool TestForLiquid(const AuMatrix4x4& mBld, const AuPoint& siteCen, const AuPoint& siteBoxRadius) const;

	virtual bool TestRockResource(const AuMatrix4x4& mBld, const AuPoint& siteCen, const AuPoint& siteBoxRadius,
			COMMODITYID idRes) const;

	// Calculates m_ThinkRotation and m_ThinkTransform for Tick.
	void ThinkPerformAxialRotation();

	virtual bool HasWorkerSpecies() const;

	virtual void SetWorkerSpecies(const AuSpecies& species);

	virtual AuSpecies WorkerSpecies() const;

	// Test whether animals can exist at a specific terrain level on this
	// planet. Inner and outer worlds restrict animals to the seas and coasts.
	virtual bool CanHaveFaunaAtElev(TER_ELEV_TYPE /*terType*/) const
	{
		return true;
	}

	// Test whether plants can exist at a specific terrain level on this planet.
	// Inner and outer worlds restrict plants to the coasts.
	// Returns 0=no, 1=yes, 2=no unless in enclosure
	int CanHaveFloraAtAltitude(float altitudeMSL) const;

	QColor ChartColor() const;

	static QColor ChartColor(unsigned char byWater);

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
		// The tree line is between the cloud layers.
		// This keeps the mountains a little shorter on moons.
		return (AuWeather::LayerAltitude(*this, AuWeather::LowClouds) +
				AuWeather::LayerAltitude(*this, AuWeather::MiddleClouds))/2;
	}

	// Spawns a spacecraft in orbit, if possible.
	// Returns:
	//   0 = ok
	//   1 = blocked by unanalyzed ship
	//   2 = Cannot spawn another station in orbit or replace an existing station.
	//   3 = Cannot spawn another nu ship in orbit or replace an existing nu ship.
	//   4 = Orbit spawn location could not be found.
	//   5 = No station at which to spawn.
	virtual int SpawnSc(const AuScReport& shipDesign, quint32 empireId, quint8 quality, const AuId& ownerId,
			const QString& ownerName, quint32 ownerEmpire, const AuId& cityOrBdId);

	static int TextureTileSize()
	{
		return 1024/VEG_Types;
	}

public:
	static double MinimumSectionLength();						// Calculates minimum length of a ringworld section.

	// Shadow square rotation for ringworlds.
	inline double AngleOfRotation() const
	{
		m_RingSectionLock.lockForRead();
		double v = m_RotationAngle;
		m_RingSectionLock.unlock();
		return v;
	}

protected:
	double AngleToRingSection() const;

	void CreateWeatherRenderData(AuGLEffects& re, AuGLModelMaker& mm);

	double CalcAngleOfRotation() const;

	// Creates a texture for use in globe rendering when viewer is at this planet. All terrain is textured; water/lava/liquid are
	// rendered separately.
	// 'maxAltMSL' is the highest MSL altitude in the triangle data that will be sent to the shader.
	// 'minAltMSL' is the lowest MSL altitude in the triangle data that will be sent to the shader.
	GLuint CreateAtLookupTable(AuGLEffects &re, float maxAltMSL, float minAltMSL, GLuint terElevTex[5]) const;

	QImage CreateAtVegetationLookupTable(float maxAltMSL, float minAltMSL, GLuint terElevTex[5]) const;

	// Creates a texture for use in far globe rendering. Sea surfaces turn to water/lava/liquid color.
	// 'maxAltMSL' is the highest MSL altitude in the triangle data that will be sent to the shader.
	// 'minAltMSL' is the lowest MSL altitude in the triangle data that will be sent to the shader.
	GLuint CreateFarLookupTable(AuGLEffects &re, float maxAltMSL, float minAltMSL, GLuint terElevTex[5]) const;

	QImage CreateFarVegetationLookupTable(float maxAltMSL, float minAltMSL, GLuint terElevTex[5]) const;

	// Creates the map of the planet that is shown on the transporter console and the locator map window.
	void CreateSurfaceMap(AuGLEffects& re);

	void CreateTerrainData(const AuDice& dice);

protected:
	virtual int AirTravellerInterval(qint8 morale) const;

	virtual int IndigenousPopulationInterval(qint8 morale) const;

	// Atmosphere density (0-100)
	inline qint8 AtmosphereDensity() const
	{
		return m_byAtmDensity;
	}

	void CreateAtGlobe(AuGLEffects& re, AuGLModelMaker& mm);

	void CreateAtGlobeSurfaceTriangle(AuGlobeTriAltVegN& tri, const AuGlobeLL& ll0, const AuGlobeLL& ll1,
			const AuGlobeLL& ll2, QMap<AuGlobeLL, AuGlobeTerrainAltInfo>& terAltInfo) const;

	//void CreateAtGlobeVAO(float& maxAltMSL, float& minAltMSL, const AuPoint& viewer);

	void CreateAtmosphereVAO(AuOpenGLFunctions& GLf);

	void CreateCloudsVAO(AuOpenGLFunctions& GLf);

	void CreateAtTerrainSurfaceTriangle(QList<AuGlobeTriAltVegNPave>& tris, const AuGlobeLL& ll0, const AuGlobeLL& ll1,
			const AuGlobeLL& ll2, QMap<AuGlobeLL, AuGlobeTerrainAltInfo>& terAltInfo) const;

	// Tessellates a cylinder into a list of triangles that only contain latitude and longitude information, no altitude
	// information.
	void CreateCylinderTriangles(AuGlobeLLTriList& tris, int londivs, int latdivs) const;

	void CreateGeologicalFeatures(const AuDice& dice);

	void CreateGrass(QMap<AuResKey, AuGlobeFlora>& grass, QMap<AuResKey, AuGlobeFlora>& aquaticgrass, const AuPoint& groundLoc,
			const AuPoint& vx, const AuPoint& vy, quint32 worldGenSeed, int floraGlow) const;

	void CreateGrassTexture(AuGLEffects& re, AuGLModelMaker& mm);

	void CreateHullVAO(AuOpenGLFunctions& GLf);

	void CreateNearGlobeSurfaceTriangle(QList<AuGlobeTriTexNShineSnow>& tris, const AuGlobeLL& ll0, const AuGlobeLL& ll1,
			const AuGlobeLL& ll2, QMap<AuGlobeLL, AuGlobeTerrainAltInfo>& terAltInfo) const;

	// iViewSizeX is the pixel width of the resulting texture. The height of the texture will be iViewSizeX/2.
	GLuint CreatePlanetTexture(AuAtomRenderInfo& ari, int iViewSizeX, bool drawCities);

	void CreateRocks(QMap<AuResKey, AuGlobeRock>& rocks, const AuPoint& groundLoc, const AuPoint& vx, const AuPoint& vy,
			quint32 worldGenSeed, float range, bool visibleOnly) const;

	void CreateShadowSquareVAO(AuOpenGLFunctions& GLf);

	void CreateShrubs(QMap<AuResKey, AuGlobeFlora>& shrubs, QMap<AuResKey, AuGlobeFlora>& aquaticshrubs, const AuPoint& groundLoc,
			const AuPoint& vx, const AuPoint& vy, quint32 worldGenSeed, int floraGlow) const;

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

	// Returns a 2D coordinate for the longitude latitude location appropriate for creating the surface map of this world.
	// Return coordinate is in the range -1,-1 to 1,1.
	inline AuPoint GetFlatGlobeSurfaceLocation(const AuGlobeLL& ll) const
	{
		double x = (ll.Longitude() / (PIf*2)) * 2;
		double y = (ll.Latitude() / PIf) * 2;
		return AuPoint(x,y);
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
			return LocOfLL(ll, altitudeMSL);
		}

		// Undersea locations are at the sea surface for smooth sea surface on far globe model.
		AuPoint loc = LocOfLL(ll);		// Location at sea level radius.
		normal = -GravityDir(loc);
		return loc;
	}

	void GrabSurfaceTextureImage(AuGLEffects& re, QImage& imtex, GLuint glTexture) const;

	// 'rockLL' is a rounded globe location.
	COMMODITYID GuaranteedRockAt(const AuGlobeLLF& rockLL, int resourceZone) const;

	// Returns a random rounded globe location for a guaranteed rock that is not already in use by another guaranteed rock.
	AuGlobeLLF GuaranteedRockLocationResLocked(const AuDice& dice, int eqDivs, int resourceZone) const;

	void LoadCachedSurfaceImage(QImage& surface, int iViewSizeX) const;

	// Returns the number of map increments in the north/south direction corresponding to 'eqdivs' equator divisions.
	inline int NorthSouthDivisions(int eqdivs) const
	{
		return (int)ceil(eqdivs * TerrainYSize() / TerrainXSize());
	}

	void QueryNaturalResources(AuZoneResource& res, int resourceZone, quint32 worldGenSeed) const;

	// Render the globe when the viewer is inside the atmosphere.
	void RenderAtGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
			const AuPoint& viewerPos, float ambientLight, float fogDensity, float fogR, float fogG, float fogB);

	void RenderAtmosphere(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelMaker& mm,
			int viewerContext, double viewerAlt, bool clouds, float atmIllumination, float atmOpacity);

	void RenderControlCenter(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float ambientLight,
			float fogDensity, float fogR, float fogG, float fogB, bool isPreonPresent);

	// Render local plants relative to the viewer's position.
	void RenderFlora(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelMaker& mm,
			const AuPoint& viewerPos, float ambientLight, float fogDensity, float fogR, float fogG, float fogB);

	void RenderHenges(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
			float ambientR, float ambientG, float ambientB, float fogDensity, float fogR, float fogG, float fogB);

	void RenderHull(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float ambientLight,
			float fogDensity, float fogR, float fogG, float fogB);

	// Render the globe when the viewer is outside the atmosphere and close enough to see terrain details.
	void RenderNearGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari, float ambientLight);

	// Render the globe when the viewer is inside the atmosphere and close enough to see terrain details but we don't have an 'at'
	// globe model to show them.
	void RenderNearGlobeFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
			float ambientLight, float fogDensity, float fogR, float fogG, float fogB);

	// 'model' projection is to the location of the statue.
	// 'statueScale' is scaling applied to the statue at that location.
	void RenderRingworldStatue(const AuMatrix4x4& projection, const AuMatrix4x4& model, double statueScale, AuGLEffects& re,
			float ambientLight, float fogDensity, float fogR, float fogG, float fogB, bool isPreonPresent);

	// Render local rocks relative to the viewer's position.
	void RenderRocks(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelMaker& mm,
			const AuPoint& viewerPos, float ambientLight, float fogDensity, float fogR, float fogG, float fogB);

	inline void RenderSea(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool eyeUnderWater,
			bool cgUnderWater, float illumination, float ambientLight, float fogDensity, float fogR, float fogG, float fogB,
			bool topDownView)
	{
		if(eyeUnderWater)
		{
			// Render only the transparent sea surface model.
			// Must reverse cull direction because eye is under the water.
			re.glFrontFace(GL_CCW);
			RenderSeaGlobe(projection, model, re, false, illumination, ambientLight, cgUnderWater, fogDensity, fogR, fogG, fogB,
					topDownView);
			re.glFrontFace(GL_CW);
		}
		else
		{
			// Render both sea surface models to hide the bottom.
			RenderSeaGlobe(projection, model, re, true, illumination, ambientLight, cgUnderWater, fogDensity, fogR, fogG, fogB,
					topDownView);
		}
	}

	void RenderSeaGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool bothLayers,
			float illumination, float ambientLight, bool showUndersideOfSurface, float fogDensity, float fogR, float fogG,
			float fogB, bool topDownView);

	void RenderShadowSquare(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float ambientLight);

	// Returns the commodity id of significant rock outcroppings at 'll'.
	// Returns CID_Invalid if no significant rocks are present at 'll'.
	COMMODITYID RockAt(const AuGlobeLL& ll, quint32 worldGenSeed, int resourceZone) const;

	// NOTE: 'plantLL' has already been rounded to a proper location. See CreateShrubs.
	COMMODITYID ShrubAt(const AuGlobeLL& plantLL, quint32 worldGenSeed) const;

	virtual bool SightSeaSurface(const AuPoint& loc1, const AuMatrix4x4& model, AuPoint& hitLoc) const;

	// Returns true if this atom is hit and 'sightedLoc' and 'sightedPart' are output.
	// 'sightedLoc' and 'sightedPart' are not changed if false is returned.
	virtual bool SightTargetDetails(const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, AuPoint& sightedLoc,
			unsigned int& sightedPart, bool& sightedShield) const;

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

#endif // DaWorldRingSection_H
