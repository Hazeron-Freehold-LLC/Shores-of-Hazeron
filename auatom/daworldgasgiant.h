//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daworldgasgiant.h,v 1.147 2022/05/17 17:11:58 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaWorldGasGiant_H
#define DaWorldGasGiant_H

#include "auweather.h"
#include "daworldsphere.h"

class Au3dsGLSLModelInstance;
class AuFogOfWar;
class AuZoneResource;
class DaBd;

class AUATOM_DLL DaWorldGasGiant : public DaWorldSphere
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
	quint8								m_byCoreRadius;			// Radius in 1000's of kilometers. Will be in the range 1-6.
	quint8								m_Radius1000km;			// Radius in 1000's of kilometers. Will be in the range 20-40.
	float								m_WindDir;				// Current wind direction in the render scene.
	float								m_WindForce;			// Current wind force in the render scene.

private:
	mutable QReadWriteLock				m_GasGiantLock;
	mutable QColor						m_AtmColor;				// Cached here to save work.
	AuFogOfWar*							m_FogOfWar;				// FogOfWar shroud.
	mutable quint32						m_Population;			// Of buildings and cities. Cached here, not stored.
	AuSpecies*							m_pSentient;			// Indigenous sentient race species.
	double								m_RotationAngle;		// Angle of rotation about the axis.

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

	// Think variables.
	double								m_ThinkRotation;		// My nu m_RotationAngle calculated in Think.
	AuMatrix4x4							m_ThinkTransform;		// My nu transform calculated in Think.

private:
	mutable QReadWriteLock				m_ResourcesLock;
	mutable AuZoneResource*				m_Resources;			// analysis of resources on this planet, in each zone
	
private:
	// OpenGL and other transient data that is not serialized.
	AuAtGlobeVAO						m_AtGlobe;				// Vertex array objects of the globe as seen from the ground.
	AuGLVAO								m_AtmosphereLayer[3];	// Vertex array objects of the atmosphere cloud layers.
	GLuint								m_glTexFarSurface;		// Generated planet surface texture for distant render/surface map.
	GLuint								m_glTexGrass;			// Grass texture generated from local flora.
	GLuint								m_glTexNearSurface;		// Generated planet surface texture for nearby render.
	bool								m_glTexNearValid;		// Set false when the surface texture needs to be regenerated.
	Au3dsGLSLModelInstance*				m_HengeColumn;
	Au3dsGLSLModelInstance*				m_HengeFirePit;
	Au3dsGLSLModelInstance*				m_HengeRock;
	AuGLVAO								m_NearGlobe;			// Vertex array object of the globe model as seen from orbit.
	AuGLVAO								m_SeaGlobe;				// Vertex array object of the sea surface.
	AuElapsedTimer						m_TerrainCleanupTime;
	AuGLVAO								m_UnderseaGlobe;		// Vertex array object of the sea subsurface.
	bool								m_ViewerNear;			// Near surface texture is updated while this is true.
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

public:	// Constructors
	DaWorldGasGiant();
	virtual ~DaWorldGasGiant();

protected:
	DaWorldGasGiant(const DaWorldGasGiant &rhs);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:	// Methods
	// Adjusts 'navLoc' so it is not inside the atmosphere/photosphere of any planets/stars or other objects a navigator would
	// perceive.
	// Result will be a close orbit radius outside of those.
	// 'shipLoc' is the reference loc for deciding where to move 'navLoc'. It is probably the location of a navigator's ship.
	// 'shipLoc' and 'navLoc' are in solar system coordinates.
	// Recursive.
	virtual void AdjustNavLocation(const AuPoint& shipLoc, AuPoint& navLoc) const;

	virtual void AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim);

	virtual void BuildingChanged(const AuMatrix4x4& bldM, double radius, AuGLModelMaker& mm);

	// Test whether this planet can support any animal life.
	// Gas giants with exotic atmosphere have creatures near the core.
	virtual bool CanHaveFauna() const;

	// Test whether animals can exist at a specific terrain level on this
	// planet. Inner and outer worlds restrict animals to the seas and coasts.
	virtual bool CanHaveFaunaAtElev(TER_ELEV_TYPE terType) const;

	virtual bool CheckChildExit(const AuPoint &foldloc, const AuPoint &fnuloc, const DaAtom* who) const;

	virtual bool CheckPeerJoin(const AuPoint &foldloc, const AuPoint &fnuloc, const DaAtom* who);

	virtual DaAtom *Clone() const;

	void Create(const AuDice& dice, double orbitRadius, ORBITZONE orbitZone);

	virtual void CreateAtGlobeSurfaceTriangles(QList<AuGlobeTriAltVegN>& tris, float& maxAltMSL, float& minAltMSL) const;

	// Used when creating the at-globe model.
	virtual void CreateAtSurfaceTriangles(QList<AuGlobeTerrainTriCen>& tricens) const;

	// 'killed' becomes true when the thread who called me is killed, so I can stop midstream.
	virtual void CreateAtTerrainTriangles(QList<AuGlobeTriAltVegNPave>& tris, const AuGlobeLLTri& terTri, const bool& killed) const;

	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari);

	virtual void CreateFlatGlobeVAOTriangle(QList<AuGlobeTriAltVeg>& tris, const AuGlobeLL& ll0, const AuGlobeLL& ll1,
			const AuGlobeLL& ll2) const;

	virtual void CreateFlora(QMap<AuResKey, AuGlobeFlora>& grass, QMap<AuResKey, AuGlobeFlora>& shrubs,
			QMap<AuResKey, AuGlobeTree>& trees, QMap<AuResKey, AuGlobeFlora>& aquaticgrass,
			QMap<AuResKey, AuGlobeFlora>& aquaticshrubs, const AuPoint& groundLoc, quint32 worldGenSeed, int floraGlow) const;

	void CreateForScan(const AuDice& dice, double orbitRadius, ORBITZONE orbitZone);

	virtual void CreateNearGlobeSurfaceTriangles(QList<AuGlobeTriTexNShineSnow>& tris) const;

	virtual void CreateRocks(QMap<AuResKey, AuGlobeRock>& rocks, const AuPoint& groundLoc, quint32 worldGenSeed, float range,
			bool visibleOnly) const;

	// Like CreateSurfaceTriangles except the poles have more triangles for making a flat projection of the surface.
	virtual void CreateSurfaceTrianglesFlat(AuGlobeLLTriList& tris, int eqdivs, float& maxAltMSL, float& minAltMSL) const;

	virtual void DeleteDeviceObjects(AuAtomRenderInfo& ari);

	virtual QString Description() const;

	virtual void EmpireDeleted(quint32 empireId);

	virtual void EnumControlPanels(AuQuadArray &aControlPanels, const AuMatrix4x4 &tModel, QVector<qint16>& idPanels,
			const DaAtom* sceneAtom) const;

	virtual void EnumViewPanelControls(qint16 panelId, QList<AuControlButton>& controls, AuAtomRenderInfo& ari) const;

	virtual void Explore(const AuId& avatarId, quint32 empireId, const AuPoint& seaLevelLoc, double radius);

	virtual void ExploreAll(const AuId& avatarId, quint32 empireId);

	virtual bool FogOfWar(const AuId& avatarId, quint32 empireId, AuFogOfWarData& fow) const;

	virtual bool FogOfWarTest(const AuId& avatarId, quint32 empireId, const AuGlobeLL& ll) const;

	virtual int SeaLevelEquatorDivisions() const;

	// Returns the base geological altitude of terrain at 'll'.
	// The altitude is not adjusted for city developments or geological features.
	virtual double GeologicalTerrainAltitudeMSL(const AuGlobeLL& ll) const;

	virtual double GlobeRadius() const
	{
		return ::Radius100km(CoreRadius1000km()*10);
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
		AuPoint gvec(-loc);
		gvec.Unit();
		return gvec;
	}

	virtual bool HitByAsteroid(quint8 asteroidSize, const AuPoint& asteroidLoc);

	virtual quint8 Hydrographics() const
	{
		return 100;
	}

	virtual void InitAnimalEncounterTable(const AuDice& dice);

	virtual void InitSystemPosition();

	virtual void Insert(QSqlDatabase& db) const;	// Insert first time database record.

	// Call when something has changed that affects the appearance of the globe. e.g. City lights go on/off.
	virtual void InvalidateNearGlobeTexture()
	{
		m_glTexNearValid = false;
	}

	virtual bool IsGasGiant() const
	{
		return true;
	}

	virtual bool IsSightTarget(const DaAtom* toWho) const;

	virtual void ListSceneSubatoms(const DaPerson *pSceneAtom, const AuPoint& cameraEnvLoc, DaAtomList &sceneAtoms);

	// Animals when CanHaveFauna is true, one entry for each zone.
	virtual DePlanetFauna* LockFaunaForWrite()
	{
		m_GasGiantLock.lockForWrite();
		return m_pFauna;
	}

	// Plant DNA, one entry for each resource zone.
	virtual const DePlanetFlora* LockFlora() const
	{
		m_GasGiantLock.lockForRead();
		return m_pFlora;
	}

	// Plant DNA, one entry for each resource zone.
	virtual DePlanetFlora* LockFloraForWrite()
	{
		m_GasGiantLock.lockForWrite();
		return m_pFlora;
	}

	virtual quint64 MemoryUsage() const;

	virtual bool MoveSubatom(DaAtom* pMove, const AuPoint& vMove, int elapsedMs);

	// Land distance from pole to pole on a sphere, wall to wall on a ringworld.
	virtual double NorthSouthDistance() const
	{
		return ::Radius100km(CoreRadius1000km()*10) * PI;
	}

	// Returns the maximum population allowed in each resource zone before cities apply an overpopulation penalty to morale.
	// This is fairly aggressive so people will need to spread into the galaxy rather than infest the same planet.
	// This is necessary to help prevent a single solar system from taking too long in the tick cycle.
	// 'cityId' is used to grandfather in old cities before population max was reduced. It can be null to treat it as a nu city.
	virtual int PopulationMax() const;

	virtual bool TestForLiquid(const AuMatrix4x4& mBld, const AuPoint& siteCen, const AuPoint& siteBoxRadius) const
	{
		Q_UNUSED(mBld);
		Q_UNUSED(siteCen);
		Q_UNUSED(siteBoxRadius);
		return true;
	}

	virtual quint32 WorldPopulation() const
	{
		m_GasGiantLock.lockForRead();
		quint32 v = m_Population;
		m_GasGiantLock.unlock();
		if(v == (quint32)-1)
		{
			v = CalcWorldPopulation();
			m_GasGiantLock.lockForWrite();
			m_Population = v;
			m_GasGiantLock.unlock();
		}
		return v;
	}

	virtual void WorldPopulationChanged()
	{
		m_GasGiantLock.lockForWrite();
		m_Population = (quint32)-1;
		m_GasGiantLock.unlock();
	}

	virtual BODY_TYPE BodyType() const
	{
		return BODY_GasGiant;
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

	// Only valid on the client.
	virtual const QImage* SurfaceImage() const;

	virtual double SurfaceImageHeight() const;

	// Returns a surface texture for display into a world map status display screen.
	virtual GLuint SurfaceTexture() const;

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

	// Lists all possible commodities that a particular mine can extract.
	virtual void QueryMineResources(const DaBd* mine, QList<COMMODITYID>& rockCIDs) const;

	// Queries for information about the terrain at a specific longitude and latitude.
	virtual void QueryTerrain(AuTerrainInfo& ter, const AuGlobeLL& ll) const;

	// Queries for information about the terrain at a specific 3D location.
	virtual void QueryTerrainAtLoc(AuTerrainInfo& ter, const AuPoint& loc) const;

	// Lists all possible commodities that a particular well can extract.
	virtual void QueryWellResources(const DaBd* well, QList<COMMODITYID>& wellCIDs) const;

	virtual void ResourceDistributionAt(const AuMatrix4x4& mBld, const AuPoint& siteCen, const AuPoint& siteBoxRadius,
			unsigned int& rocks, unsigned int& shrubs, unsigned int& trees, float& waterCoverage, unsigned int maxICareAbout) const;

	// Returns a zone index for flora, fauna, and resources.
	// 'longitude' is an angle in radians.
	virtual int ResourceZone(float longitude) const;

	// Returns a zone index for flora, fauna, and resources.
	// 'longitude' is an angle in radians.
	virtual int ResourceZone(double longitude) const;

	// Returns 0,0 if no landing site is found in the zone.
	virtual AuPoint ResourceZoneLandingSite(int zone) const;

	virtual bool HasWorkerSpecies() const;

	void SetFogOfWar(const AuFogOfWar& fow);

	virtual void SetWorkerSpecies(const AuSpecies& species);

	virtual AuSpecies WorkerSpecies() const;

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

	virtual TER_ELEV_TYPE TerrainElevationType(float altitudeMSL) const;

	virtual bool IsTicker() const;

	virtual void Tick(const AuElapsedTimer& last, const AuElapsedTimer& now);

	virtual void TickOnClient(const AuElapsedTimer& last);

	virtual unsigned int IsTocker() const;

	virtual void Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans);

	// Returns an altitude above which there are no plants. This is the target altitude when making terrain for the tree line of
	// mountains. It is also used in the CanHaveFlora test.
	// The altitude returned does not depend on the presense of trees or plants. It is solely based on the weather layer altitudes.
	virtual float TreeLineMSL() const;

	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
			const DaPerson& sceneAtom);

	virtual ATOM_TYPE AtomType() const;

	virtual double UndevelopedVirginTerrainAltitudeMSL(const AuGlobeLL& ll) const;

	virtual void UnlockFauna()
	{
		m_GasGiantLock.unlock();
	}

	virtual void UnlockFlora() const
	{
		m_GasGiantLock.unlock();
	}

	virtual void Update(QSqlDatabase& db) const;

	virtual bool UpdateEnclosureCache() const;

	virtual double VirginTerrainAltitudeMSL(const AuGlobeLL& ll, WORLDTEX_TYPE& pavementType, AuTime& newest) const;

	virtual double VirginTerrainAltitudeMSL(const AuGlobeLL& ll, bool& hideRocks, int& hideGrass, bool& hideShrubs, bool& hideTrees,
			WORLDTEX_TYPE& pavementType, AuTime& newest) const;

	virtual void WeatherAt(const AuGlobeLLF& ll, float altitudeMSL, float& relHumidity, float& oat, float& bar) const;

	virtual QPointF WindAt(const AuGlobeLLF& longLat) const;

	//------------------
	// DaWorld Functions
public:
	// Return the altitude above mean sea level at a 'loc'.
	virtual double AltitudeMSL(const AuPoint& loc) const
	{
		return CGAltitude(loc) - SeaLevelAltitude();
	}

	virtual QColor AtmosphereColor() const;

	// Returns the height of the top of the atmosphere above sea level, whether or not there is any actual atmosphere density there.
	virtual double AtmosphereHeightMSL() const
	{
		return AtmosphereTopGasGiantMSL();
	}

	// Not really a radius on a ringworld. Returns the height of the atmosphere above sea level instead, which is the ring surface.
	virtual double AtmosphereRadius() const
	{
		return SeaLevelRadius() + AtmosphereTopGasGiantMSL();
	}

	virtual ATMTYPE AtmosphereType() const
	{
		return m_AtmType;
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
		loc.Unit(SeaLevelAltitude() + altitudeMSL);
	}

	virtual bool HasAtmosphere() const
	{
		return true;
	}

	// Returns the altitude of a location inside the atmosphere of this planet or ringworld.
	// On a planet, the altitude is above the center of the planet and will never be negative.
	// On a ringworld, the altitude is below AtmosphereRadius and above MinRadius, which can be negative.
	// Returns -100000 if the location is not inside the atmosphere.
	virtual double InAtmosphereCGAltitude(const AuPoint& loc) const
	{
		// Planet only needs to test vector length to know if a location is inside the atmosphere.
		double alt = loc.Length();
		if(alt <= AtmosphereRadius())
			return alt;
		return -100000;
	}

	// Returns atmosphere density at sea level in the range 0..1.
	virtual float MSLAtmosphereDensity() const
	{
		return (float)m_byAtmDensity / 100.0f;
	}

	// Resource quality varies by resource zone across the planet.
	// So far this is only used when foraging from the environment.
	virtual quint8 NaturalResourceQuality(COMMODITYID cid, int resourceZone) const;

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
		return FeetPerOrbit()/2;
	}

	// Test if all the flags in all the cities on this planet are controlled by the 'empireId' and if none of those
	// cities is an occupied city. This planet must have at least one city controlled by 'empireId'.
	// Returns:
	//   0 = ok to rename
	//   1 = no cities on the planet
	//   2 = occupied city on the planet
	//   3 = flag of other empire on the planet
	virtual int OkToRename(quint32 empireId) const;

	virtual void QueryNaturalResources(AuZoneResource& res, int resourceZone) const;

	// Returns the intensity of sunlight striking the surface.
	virtual float QuerySunlight(const AuPoint& fSurfLoc) const;

	// Returns the number of resource zones on this world.
	virtual int ResourceZones() const
	{
		if(m_byCoreRadius >= 3)
			return 2;
		else
			return 1;
	}

	virtual double SeaLevelAltitude() const;

	// Returns the temperature of this world at sea level, in kelvin.
	// 'longLat' holds longitude and latitude in x and y, in radians.
	virtual float SeaLevelTempAt(const AuGlobeLLF& longLat, float solarOrbitRadius, float sunLightAtten) const;	// in kelvin

	//------------------
	// DaWorldGasGiant Functions
public:
	// Axis rotation for globes.
	inline double AngleOfRotation() const
	{
		m_GasGiantLock.lockForRead();
		double v = m_RotationAngle;
		m_GasGiantLock.unlock();
		return v;
	}

	static QColor AtmosphereColor(unsigned char byAtmType, unsigned char byAtmDensity, quint32 genSeed);

	virtual bool CanBeHarvested(COMMODITYID moduleInUse) const
	{
		if(moduleInUse == CID_ProspectorBayModule || moduleInUse == CID_WildcatterBayModule)
			return true;
		if(moduleInUse == CID_AtmosphereCondenserBayModule || moduleInUse == CID_ParticleCollectorBayModule)
			return true;
		if(moduleInUse == CID_SiphonBayModule)
			return true;
		return false;
	}

	QColor ChartColor() const;

	static QColor ChartColor(ORBITZONE eZone);

	bool ClampLongLat(QPoint &pLongLat) const;	// Clamps pLongLat to be in the proper range. Returns false on latitude rollover.

	// context: 1=outside atm, and 2=distant
	AuGLVAO CreateGasGiantAtmosphereModel(AuOpenGLFunctions& GLf, int viewerContext);

	AuGLVAO CreateGasGiantCloudModel(AuOpenGLFunctions& GLf, AuWeather::LAYER layer);

	void CreateGasGiantModel(AuOpenGLFunctions& GLf);

	void CreateAtGlobeSurfaceTriangle(AuGlobeTriAltVegN& tri, const AuGlobeLL& ll0, const AuGlobeLL& ll1,
			const AuGlobeLL& ll2, QMap<AuGlobeLL, AuGlobeTerrainAltInfo>& terAltInfo) const;

	void CreateAtTerrainSurfaceTriangle(QList<AuGlobeTriAltVegNPave>& tris, const AuGlobeLL& ll0, const AuGlobeLL& ll1,
			const AuGlobeLL& ll2, QMap<AuGlobeLL, AuGlobeTerrainAltInfo>& terAltInfo) const;

	virtual void FindClosestHarvesterTargetRecursive(COMMODITYID harvesterModule, const AuPoint& gunSysLoc, float gunRange,
			const DaAtom*& targetAtom, double& targetRange) const;

	virtual AuPoint WeaponLockLocation(const AuPoint& shipSysLoc, COMMODITYID harvesterModule) const;

	// Called during creation of this gas giant to determine how many moons it has. Not called on moons.
	int GenMoonCount(const AuDice& dice) const;

	// Returns surface location at elevation of rough planet model.
	// 'longLat.x' is longitude as a fraction of longitude divisions, 0..1
	// 'longLat.y' is latitude as a fraction of latitude divisions, 0..1
	AuPoint GetLocation(const AuPoint &longLat) const;

	// Returns surface location at elevation of rough planet model.
	AuPoint GetLocation(const QPoint &pLongLat) const;

	// Returns surface location at elevation of rough planet model.
	// 'longitude' is the longitude angle in radians. + goes east, - goes west. 0 = 180 west, PI = prime meridian, PI*2 = 180 east
	// 'latitude' is the latitude angle in radians. + goes north, - goes south. 0 = equator
	inline AuPoint GetLocation(double longitude, double latitude) const
	{
		if(longitude < 0)
			longitude += PI*2;
		AuPoint longLat((longitude + PI) * LongitudeDivisions() / (PI*2), (PI/2 - latitude) * LatitudeDivisions() / PI);
		return GetLocation(longLat);
	}

	QPoint GetLongLat(const AuPoint &fSurfaceLoc) const;

	AuPoint GetRealLongLat(const AuPoint &fSurfaceLoc) const;

	int LatitudeDivisions() const
	{
		return LatitudeDivisions(m_Radius1000km);
	}

	// Longitude/latitude division calculations.
	static inline int LatitudeDivisions(quint8 radius1000km)
	{
		// Regular spherical planet.
		return LongitudeDivisions(radius1000km)/2;
	}

	static DaWorldGasGiant* LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, LOADSUBATOMS loadWhat,
			const AuId& idToLoad=0);

	int LongitudeDivisions() const
	{
		return LongitudeDivisions(m_Radius1000km);
	}

	static inline int LongitudeDivisions(quint8 radius1000km) 
	{
		// Regular spherical planet.
		int twid = 20 * radius1000km;
		if(twid > 240)
			twid = 240;	// Don't get carried away. The size consistency only matters for worlds we can build upon, sizes 1-12.
		return twid;
	}

	// Calculates m_ThinkRotation and m_ThinkTransform for Tick.
	void ThinkPerformAxialRotation();

	inline double SeaLevelRadius() const	// Returns the sea-level radius in feet.
	{
		return ::Radius100km(m_Radius1000km*10);
	}

	virtual quint16 Radius100km() const
	{
		return m_Radius1000km*10;
	}

	// Render the globe.
	// Lights have already been set up.
	// View and World transforms have already been set up.
	void RenderGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float ambientLight);

	virtual QColor SeaColor() const;

	virtual float SeaFogDensity() const;

	// Returns snow coverage in the range of 0 to 1.
	float SnowCoverage(float altMSL, float latitude, int inEnclosure) const;

	// Formats a "real" longitude/latitude coordinate (as returned by GetRealLongLat()) into a formatted string.
	inline QString StringLongLat(const AuPoint& longLat) const
	{
		float lats = LatitudeDivisions();
		float lons = LongitudeDivisions();

		// Y is reversed in GetRealLongLat return result so we must invert the sign here.
		double latang = longLat.y / lats;
		latang = (.5f - latang) * 2.f * PIf/2.f;				// Negative is south; positive is north.

		double lonang = longLat.x / lons;
		lonang = (lonang - .5f) * 2.f * PIf;					// Negative is west; positive is east.

		return AuGlobeLL::ToString(lonang, latang);
	}

	quint8 VegetationAt(const AuGlobeLL& ll, float terAltMSL, int inEnclosure, int irrigated) const;

	// Returns the volume of this planet in cubic feet.
	inline double Volume() const
	{
		return DaWorldSphere::Volume100km(m_Radius1000km*10);
	}

protected:
	virtual int AirTravellerInterval(qint8 morale) const;

	virtual int IndigenousPopulationInterval(qint8 morale) const;

	virtual int GroundLevelEquatorDivisions() const;

	// Returns radius of core.
	quint8 CoreRadius1000km() const;

	void CreateAtGlobe(AuGLEffects& re, AuGLModelMaker& mm);

	QImage CreateAtGasGiantLookupTable(float maxAltMSL, float minAltMSL, GLuint terElevTex[5]) const;

	// Creates a texture for use in globe rendering when viewer is at this planet. All terrain is textured; water/lava/liquid are
	// rendered separately.
	// 'maxAltMSL' is the highest MSL altitude in the triangle data that will be sent to the shader.
	// 'minAltMSL' is the lowest MSL altitude in the triangle data that will be sent to the shader.
	GLuint CreateAtLookupTable(AuGLEffects &re, float maxAltMSL, float minAltMSL, GLuint terElevTex[5]) const;

	virtual void CreateFloraTextures(AuGLEffects& re, int resourceZone);

	void CreateGeologicalFeatures(const AuDice& dice);

	void CreateGrass(QMap<AuResKey, AuGlobeFlora>& grass, QMap<AuResKey, AuGlobeFlora>& aquaticgrass, const AuPoint& groundLoc,
			const AuPoint& vx, const AuPoint& vy, quint32 worldGenSeed, int floraGlow) const;

	void CreateGrassTexture(AuGLEffects& re, AuGLModelMaker& mm);

	void CreateNearGlobeSurfaceTriangle(QList<AuGlobeTriTexNShineSnow>& tris, const AuGlobeLL& ll0, const AuGlobeLL& ll1,
			const AuGlobeLL& ll2, QMap<AuGlobeLL, AuGlobeTerrainAltInfo>& terAltInfo) const;

	// iViewSizeX is the pixel width of the resulting texture. The height of the texture will be iViewSizeX/2.
	GLuint CreatePlanetTexture(AuAtomRenderInfo& ari, int iViewSizeX, bool drawCities);

	void CreateRocks(QMap<AuResKey, AuGlobeRock>& rocks, const AuPoint& groundLoc, const AuPoint& vx, const AuPoint& vy,
			quint32 worldGenSeed, float range, bool visibleOnly) const;

	void CreateShrubs(QMap<AuResKey, AuGlobeFlora>& shrubs, QMap<AuResKey, AuGlobeFlora>& aquaticshrubs, const AuPoint& groundLoc,
			const AuPoint& vx, const AuPoint& vy, quint32 worldGenSeed, int floraGlow) const;

	// Creates the map of the planet that is shown on the transporter console and the locator map window.
	void CreateSurfaceMap(AuGLEffects& re);

	// Tessellates the surface of a globe fairly uniformly, with 'eqdivs' vertices at the equator and a single vertex at each pole.
	// 'maxAltMSL' and 'minAltMSL' are return values.
	// Triangles are added to 'tris' such that the first and last vertices are never at the same latitude. The second vertex will be
	// at the latitude of either the first vertex or the last vertex.
	void CreateSurfaceTriangles(AuGlobeLLTriList& tris, int eqdivs, float& maxAltMSL, float& minAltMSL) const;

	void CreateTerrainData(const AuDice& dice);

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
	AuPoint GetNearGlobeSurfaceLocation(const AuGlobeLL& ll, float altitudeMSL, AuPoint& normal) const;

	void GrabSurfaceTextureImage(AuGLEffects& re, QImage& fboImage, GLuint glTexture) const;

	// 'rockLL' is a rounded globe location.
	COMMODITYID GuaranteedRockAt(const AuGlobeLLF& rockLL, int resourceZone) const;

	// Returns a random rounded globe location for a guaranteed rock that is not already in use by another guaranteed rock.
	AuGlobeLLF GuaranteedRockLocationResLocked(const AuDice& dice, int eqDivs, int resourceZone) const;

	void InitCoreRadius(const AuDice& dice);

	void LoadCachedSurfaceImage(QImage& surface, int iViewSizeX) const
	{
		LoadCachedSurfaceImage(Id(), surface, iViewSizeX, m_tmFloraChange);
	}

	void LoadCachedSurfaceImage(const AuId& worldId, QImage& surface, int iViewSizeX, const AuTime& floraChange) const;

	void QueryNaturalResources(AuZoneResource& res, int resourceZone, quint32 worldGenSeed) const;

	// Render the atmosphere.
	// Lights have already been set up.
	// View and World transforms have already been set up.
	// viewerContext: 0=inside, 1=outside nearby, 2=outside at a distance
	void RenderAtmosphere(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
			int viewerContext, double viewerAlt, float atmIllumination);

	// Render the globe when the viewer is inside the atmosphere.
	void RenderAtGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
			const AuPoint& viewerPos, float ambientLight, float fogDensity, float fogR, float fogG, float fogB);

	// Render local plants relative to the viewer's position.
	void RenderFlora(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelMaker& mm,
			const AuPoint& viewerPos, float ambientLight, float fogDensity, float fogR, float fogG, float fogB);

	void RenderHenges(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
			float ambientR, float ambientG, float ambientB, float fogDensity, float fogR, float fogG, float fogB);

	// Render local rocks relative to the viewer's position.
	void RenderRocks(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, AuGLModelMaker& mm,
			const AuPoint& viewerPos, float ambientLight, float fogDensity, float fogR, float fogG, float fogB);

	void RenderSea(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool eyeUnderWater,
			bool cgUnderWater, float illumination, float ambientLight, float fogDensity, float fogR, float fogG, float fogB,
			bool viewTopDown);

	void RenderSeaGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool bothLayers,
			float illumination, float ambientLight, bool showUndersideOfSurface, float fogDensity, float fogR, float fogG,
			float fogB, bool viewTopDown);

	// Returns the commodity id of significant rock outcroppings at 'll'.
	// Returns CID_Invalid if no significant rocks are present at 'll'.
	COMMODITYID RockAt(const AuGlobeLL& ll, quint32 worldGenSeed, int resourceZone) const;

	// NOTE: 'plantLL' has already been rounded to a proper location. See CreateShrubs.
	COMMODITYID ShrubAt(const AuGlobeLL& plantLL, quint32 worldGenSeed) const;

	// NOTE: 'plantLL' has already been rounded to a proper location. See CreateTrees.
	COMMODITYID TreeAt(const AuGlobeLL& plantLL, quint32 worldGenSeed) const;

	void UpdateNearSurfaceTexture(AuAtomRenderInfo& ari);
};

#endif // DaWorldGasGiant_H
