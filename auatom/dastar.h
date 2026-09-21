//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dastar.h,v 1.235 2022/05/17 16:42:59 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaStar_H
#define DaStar_H

#include "augl.h"
#include "austargate.h"
#include "daatom.h"

class AuDice;
class AuMatrix4x4;
class DaVehicle;
class DaWorldGasGiant;
class DaWorldLL;
class DaWorldRingSection;
class PsWeaponEffect;
class QXmlStreamWriter;

#define TSWID 100
#define TSHT  50

class AUATOM_DLL DaStar : public DaAtom  
{
public:
	enum TYPE
	{
		TYPE_O,
		TYPE_B,
		TYPE_A,
		TYPE_F,
		TYPE_G,
		TYPE_K,
		TYPE_M,
		TYPE_SupermassiveBlackHole,
		TYPE_StellarBlackHole,
		TYPE_NeutronStar
		// This must not exceed 15.
	};
	enum SIZE
	{
		SIZE_Ia,		// Bright Supergiant
		SIZE_Ib,		// Weaker Supergiant
		SIZE_II,		// Bright Giant
		SIZE_III,		// Giant
		SIZE_IV,		// Subgiant
		SIZE_V,			// Main Sequence
		SIZE_VI,		// Sub Dwarf
		SIZE_D			// White Dwarf
	};
	enum ORBIT_ZONE
	{
		ZONE_InsideStar,
		ZONE_Unavailable,
		ZONE_Inferno,
		ZONE_Inner,
		ZONE_Habitable,
		ZONE_Outer,
		ZONE_Frigid,
	};

private:
	// Unguarded
	unsigned char						m_bySpecDec;			// spectral decimal classification (0-9) (N/A for type D)
	char								m_chHabitable;			// Orbit of the habitable zone. -1 if none.
	char								m_chOrbit;				// orbit (-1 for primary, 0 for close companions)
	char								m_chRadius;				// Radius of this star, in orbits. -1 for really small.
	mutable quint8						m_InDesigner;			// 0=unknown, 1=no, 2=yes
	mutable qint8						m_StarNumber;			// Cached result of calling StarNumber(). Not stored. -3=unknown,
																// -2=black hole, -1=Solitaire, 0=Primary, 1=Secondary, 2=Tertiary
	double								m_ZoneOfControl;		// Squared radius of this star's zone-of-control. Not stored.

private:
	mutable QReadWriteLock				m_StarLock;
	mutable double						m_AxialTilt;
	unsigned char						m_byZones[16];			// orbit zones
	SIZE								m_eSize;				// spectral size
	TYPE								m_eType;				// spectral type, or black hole

	AuStargateList						m_Stargates;			// Active stargates. Too transient to store in db.
	AuElapsedTimer						m_StargatesChange;

	AuId								m_RingworldRefSection;	// Ringworld section that is the reference section. Not serialized.
	mutable QColor						m_SunColor;
	quint16								m_SurfaceTempK;			// Surface temperature in Kelvin. 1300 to 65000.

private:
	// OpenGL
	AuGLVAO								m_CoreSphere;			// Vertex array object of the core sphere.
	AuGLVAO								m_CoronaSphere;			// Vertex array object of the corona sphere.
	float								m_fRotation[4];			// Texture rotation. DUAL PURPOSE! Holds a temp value during create.
	mutable float						m_NeutronStarOutput;
	PsWeaponEffect*						m_PSStargateBeam;		// Beam effect from cooling towers to the sun.
	AuGLVAO								m_SolarDisk;			// Vertex array object of the solar disk.
	AuPoint								m_SphereRot;			// Rotation of black hole and neutron star effects.
	GLuint								m_TexCorona[4];			// Cloudy corona sprite textures.
	GLuint								m_TexLightning;			// Lightning texture.
	GLuint								m_TexNoise;				// Noise texture.
	GLuint								m_TexSun;				// Texture for the sun VAO.

public:
	DaStar();
	virtual ~DaStar();

protected:
	DaStar(const DaStar &rhs);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	// Adjusts 'navLoc' so it is not inside the atmosphere/photosphere of any planets/stars or other objects a navigator would
	// perceive.
	// Result will be a close orbit radius outside of those.
	// 'shipLoc' is the reference loc for deciding where to move 'navLoc'. It is probably the location of a navigator's ship.
	// 'shipLoc' and 'navLoc' are in solar system coordinates.
	// Recursive.
	virtual void AdjustNavLocation(const AuPoint& shipLoc, AuPoint& navLoc) const;

	// Not really an atmosphere. AI uses this to decide how close to orbit/avoid this star.
	inline double AtmosphereRadius() const
	{
		if(IsAnyBlackHole())
			return PhotonSphereRadius();
		else if(IsNeutronStar())
			return NeutronSphereRadius();
		return Radius();
	}

	double AxialTilt() const;

	virtual BODY_TYPE BodyType() const;

	virtual bool CanBeHarvested(COMMODITYID moduleInUse) const
	{
		return moduleInUse == CID_ParticleCollectorBayModule;
	}

	// This atom cannot have a movement vector.
	virtual bool CanSetInMotion() const
	{
		return false;
	}

	// Called to calculate the m_ZoneOfControl radius value.
	void CalculateZoneOfControl();

	// Closes the destination stargate that was created from a certain galaxy.
	void CloseStargate(const AuId& fromGalaxy, const AuPoint& fromGalaxyLoc);

	// Sets 'north' and 'east' to unit vectors from loc that point to the north and east respectively, parallel with the surface.
	// Sets 'up' to a unit vector pointing up from loc with respect to gravity.
	void CompassVectors(const AuPoint& loc, AuPoint& north, AuPoint& east, AuPoint& up) const;

	// Harvest order may specify the wrong type of preon.
	// 'cidToHarvest' is the resource commodity desired to be harvested from here. If it is a preon, the return value is the
	// appropriate type of preon for this star, otherwise 'cidToHarvest' is returned unchanged.
	COMMODITYID CorrespondingHarvestResource(COMMODITYID cidToHarvest) const;

	virtual QString Description() const;

	int FetchProbability(COMMODITYID cid) const;	// Probability of fetching a resource from the environment. (0..100).

	virtual void FindClosestHarvesterTargetRecursive(COMMODITYID harvesterModule, const AuPoint& shipSysLoc, float gunRange,
			const DaAtom*& targetAtom, double& targetRange) const;

	// Find a star or world that has natural resource commodity 'resCID' at minimum quality 'minQ' in the environment.
	virtual bool FindNaturalResource(COMMODITYID resCID, quint8 minQ) const;

	virtual void InitSystemPosition();

	virtual AuPoint WeaponLockLocation(const AuPoint& shipSysLoc, COMMODITYID harvesterModule) const;

	// Recursive attempt to locate a 'nu' vehicle that has been manufactured.
	DaVehicle* FindNewVehicle(COMMODITYID vehicleCID, unsigned int empireId);

	// Recursive attempt to locate a 'nu' vehicle that has been manufactured.
	DaVehicle* FindNewVehicleType(VEHICLE_SPOT_TYPE spotType, unsigned int empireId);
	DaVehicle* FindNewVehicleType(VEHICLE_SPOT_TYPE spotType, const DaBd* fetcher);

	virtual DaStar* FindParentStarInEnv();

	virtual const DaStar* FindParentStarInEnv() const;

	// Locates the m_RingworldRefSection section of a ringworld, used for coordinated rotation of all sections.
	// This is also the section responsible for the shadow squares object.
	const DaWorldRingSection* FindRingworldReferenceSection() const;

	DaWorldRingSection* FindRingworldReferenceSection();

	DaWorldRingSection* FindRingworldArc(int segmentIndex);

	// Locates the m_RingworldRefSection section of a ringworld.
	// This is also the section responsible for the shadow squares object.
	DaWorldRingSection* FindRingworldControlSection();

	// Recursive search for the resources needed by starting players.
	void FindStartupResources(int& eludium, int& lumenite);

	// Returns a seed value that is based on the solar system's GenSeed and this star's StarNumber.
	quint32 GenSeed() const;
	quint32 GenSeed(quint32 systemGenSeed, int starNumber) const;

	// 'origin' is true to look for an origin, false to look for a destination.
	inline bool HasStargate(bool origin) const
	{
		bool v;
		m_StarLock.lockForRead();
		if(origin)
			v = m_Stargates.HasOrigin();
		else
			v = m_Stargates.HasDestination();
		m_StarLock.unlock();
		return v;
	}

	// Returns true if the portal exists, regardless of whether we created it just now.
	bool HengeActivated(const AuId& fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			quint8 destArcSegment);

	// 'sys' is used to cache the result.
	void HengeCreated(DaSolarSystem* sys, const AuId& mySectorId, const AuId& fromSysId, quint8 fromSun, quint8 fromShadowPanels,
			quint8 fromArcSegment, quint8 destSun, quint8 destArcSegment);

	void HengeDeactivated(const AuId& fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			quint8 destArcSegment);

	void HengeTeleportArrived(DaAtom* arrived, quint8 destArc, const AuId& fromSysId, quint8 fromSun, quint8 fromArc);

	bool InDesignerPreview() const;

	virtual bool IsEthereal() const
	{
		return false;
	}

	virtual bool IsObstacle(const DaAtom* toWho) const
	{
		Q_UNUSED(toWho);
		return true;
	}

	// Returns actual radius of this object for avoidance when navigating and bay weapon fire obstruction.
	virtual double ObstacleRadius() const
	{
		if(IsAnyBlackHole())
			return PhotonSphereRadius();
		else if(IsNeutronStar())
			return NeutronSphereRadius();
		else
			return Radius();
	}

	// Called by spacecraft to avoid objects.
	// Returns safe radius with a safety margin to avoid this object when navigating.
	// A return value of 0 means this atom is not avoided by spacecraft.
	virtual double PilotAvoidRadius() const;

	// Range within which a pilot maintains a slow safe speed limit.
	// Returning 1 here has a desired effect on AI pilot. See usage. Obstacle that returns non-zero here is not ignored even if
	// too close. Returning 0 causes AI pilot to ignore this obstacle when too close.
	virtual float PilotSpeedLimitRange() const
	{
		return 1;
	}

	inline AuTime PositiveStargateExpiry() const
	{
		m_StarLock.lockForRead();
		AuTime v(m_Stargates.PositiveStargateExpiry());
		m_StarLock.unlock();
		return v;
	}

	inline bool IsRingworldStar() const
	{
		return !RingworldRefSection().IsNull();
	}

	inline AuId RingworldRefSection() const
	{
		m_StarLock.lockForRead();
		AuId v(m_RingworldRefSection);
		m_StarLock.unlock();
		return v;
	}

	inline bool IsRingworldStargateOpen() const
	{
		m_StarLock.lockForRead();
		bool v = !m_RingworldRefSection.IsNull() && m_Stargates.HasOrigin();
		m_StarLock.unlock();
		return v;
	}

	virtual void ListSceneSubatoms(const DaPerson *pSceneAtom, const AuPoint& cameraEnvLoc, DaAtomList &sceneAtoms);

	// 'origin' is true at the starting end, false at the destination.
	// 'expire' is the time to go away in case we never get the close notification.
	void OpenStargate(const AuId& toGalaxy, const AuPoint& toGalaxyLoc, bool origin, const AuTime& expire);

	virtual bool Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari);

	void RenderBlackHole(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, const DaPerson* sceneAtom);

	void RenderNeutronStar(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re);

	// 'brightness' is 0 to 1, used when dimmed due to stargate.
	void RenderSun(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float brightness);

	quint8 ResourceQuality(COMMODITYID cid) const;

	virtual DaAtom* SightTarget(const DaAtom* sightingAtom, const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, DaAtom* pBest,
			AuPoint& sightedLoc, unsigned int& sightedPart, bool& sightedShield, AuPoint* sightedSurfaceNormal) const;

	inline quint8 SpectralClass() const
	{
		return m_bySpecDec;
	}

	virtual DaAtom *Clone() const;

	// 'starsInSystem' contains all stars in the solar system. The first one is the primary. It could be me.
	bool CouldHaveRingworld(quint32 systemGenSeed, const DaAtomList& starsInSystem);

	// Used when creating a preview in the designer.
	// 'previewType' 0=none, 1=empty space, 2=sun, 3=sun and planet, 4=sun, planet and moon, 5=inferno, 6=frigid, 7=gas giant,
	//   8=ringworld, 9=sea bottom
	void CreateForPreview(quint8 previewType);

	// Called when building a solar system for scanning its resources. Does not assign a real atom id.
	void CreateForScan();

	virtual bool MoveSubatom(DaAtom* pMove, const AuPoint& vMove, int elapsedMs);

	virtual bool RoughExtents(AuBox &fExt) const;

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

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const;
	virtual void RxUpdate(QDataStream& fromServer, bool isFirstUpdate);
	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
			const DaPerson& sceneAtom);

	bool TraverseWormhole(DaVessel* ship, const AuWormhole& wh, float entryAngle);

	virtual ATOM_TYPE AtomType() const;

	static QString Name(DaStar::TYPE eType, DaStar::SIZE eSize, unsigned char bySpecDec);

	// This name is as short as possible, for sensor console screens.
	static QString ScreenName(DaStar::TYPE eType, DaStar::SIZE eSize, unsigned char bySpecDec);

	// This version is used by auempirereport to make the player stats pages.
	static QString Name(DaStar::TYPE eType, DaStar::SIZE eSize);

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

	// Returns acceleration due to gravity in ft/sec^2 at a given distance
	// from this star.
	// The basic formula is...
	//   G = K * V / a^2
	// where...
	//   G is gravity acceleration in ft/sec/sec
	//   K is universal mass constant
	//   V is volume of this star in cubic feet
	//   a is altitude of object above this star's center of gravity in feet
	virtual double Gravity(double cgDistance) const
	{
		// Gravity is infinite at zero distance? Let's just return zero to avoid dividing by zero.
		if(cgDistance == 0)
			return 0;

		if(IsAnyBlackHole())
			return MassKBlackHole() * Volume() / (cgDistance * cgDistance);
		else if(IsNeutronStar())
			return MassKNeutronStar() * Volume() / (cgDistance * cgDistance);
		else
			return MassKStar() * Volume() / (cgDistance * cgDistance);
	}

	// Returns a unit vector pointing down with respect to gravity at a specific location.
	// This is the direction of the gravitational force, if this atom exerts any.
	virtual AuPoint GravityDir(const AuPoint& loc) const
	{
		AuPoint gvec(-loc);
		gvec.Unit();
		return gvec;
	}

	// Returns the volume of this planet in cubic feet.
	// V = 4/3 * PI * r^3
	inline double Volume() const
	{
		double radius = Radius();
		return 4./3. * PI * radius * radius * radius;
	}

	char GenCompanionOrbit(const AuDice& dice, int lCompanion);

	// 'lSizeMod' is the value returned by GenPrimarySize, when creating the primary star.
	void GenCompanionSize(const AuDice& dice, int lSizeMod);

	// 'lTypeMod' is the value returned by GenPrimaryType, when creating the primary star.
	void GenCompanionType(const AuDice& dice, int lTypeMod);

	// Configures this star to be a neutron star.
	void GenNeutronStar(const AuDice& dice, int& typeMod, int& sizeMod);

	// Returns 'lSizeMod' needed to create companions using GenCompanionSize.
	int GenPrimarySize(const AuDice& dice);

	// Returns 'lTypeMod' needed to create companions using GenCompanionType.
	int GenPrimaryType(const AuDice& dice);

	// Configures this star to be a stellar black hole.
	void GenStellarBlackHole(const AuDice& dice, int& typeMod, int& sizeMod);

	// Configures this star to be a supermassive black hole.
	// Sets 'typeMod' and 'sizeMod' to the values needed to create companions using GenCompanionType and GenCompanionSize.
	void GenSupermassiveBlackHole(const AuDice& dice, GALAXY galaxy, int& typeMod, int& sizeMod);

	// Returns the orbit number of the habitable zone. -1 or 16 if none.
	// 16 indicates a weak star where the habitable zone is beyond the farthest
	// possible orbit, for light intensity calculations.
	inline char HabitableZone() const
	{
		return m_chHabitable;
	}

	void ProcessDemandTally(const AuItem& item,
			unsigned int& stock, unsigned int& population, unsigned int& needCount, unsigned int& produceCount) const;

	void CreatePlanets(HABITABILITY hab);

	void CreatePlanetsForScan(HABITABILITY hab, GALAXY galaxy);

	void InitBlackHoleZones(GALAXY galaxy);

	void InitNeutronStarZones();

	void InitZones();

	inline bool IsPrimary() const
	{
		return(m_chOrbit < 0 ? true : false);
	}

	inline bool IsAnyBlackHole() const
	{
		TYPE type = StarType();
		return type == TYPE_StellarBlackHole || type == TYPE_SupermassiveBlackHole;
	}

	inline bool IsNeutronStar() const
	{
		return StarType() == TYPE_NeutronStar;
	}

	inline bool IsStellarBlackHole() const
	{
		return StarType() == TYPE_StellarBlackHole;
	}

	inline bool IsSupermassiveBlackHole() const
	{
		return StarType() == TYPE_SupermassiveBlackHole;
	}

	inline int Orbit() const
	{
		return(m_chOrbit);
	}

	// Returns -2=Black Hole, -1=Solitaire, 0=Primary, 1=Secondary, 2=Tertiary
	// CAVEATE: In a black hole solar system, the black hole is -2, the first companion is -1 or 0, the second companion is 1.
	// Sets m_StarNumber.
	int StarNumber() const;
	int StarNumber(const DaAtomList& stars) const;

	// return the radius of an orbit number, in astonomical units (au).
	static inline float OrbitRadius(int orbit)
	{
		if(orbit < 0)
			return(0.125f);
		float radOrbitAu = (orbit + 1.f)/4.f;
		return radOrbitAu;
	}

	// 'radOrbitAu' is orbit radius in astronomical units (au).
	static inline char OrbitFromRadius(float radOrbitAu)
	{
		if(radOrbitAu <= 0.125f)
			return -1;

		char chOrbit = (char)(radOrbitAu * 4.f) - 1;
		return chOrbit;
	}

	inline void SetOrbit(char chOrbit)
	{
		m_chOrbit = chOrbit;
	}

	// Intergalactic wormholes are right at the photon sphere, draining into the black hole.
	double IntergalacticWormholeDistance() const;

	// Determine minimum orbit that can be used by a companion star.
	char MinOrbitAvailableFrom(char chCompanionOrbit) const;

	// Returns the neutron sphere radius for neutron stars.
	double NeutronSphereRadius() const;

	// Returns the photon sphere radius for black holes.
	double PhotonSphereRadius() const;

	// Returns the photon sphere radius of this black hole, in orbits, assuming it's a black hole.
	// This is based on the radius of the galaxy.
	// Return value range is 0 to 12 clamped.
	// Return value is always 1 for stellar black holes.
	// Return value range for supermassive black holes is 1 to 12, based on current table of s_Galaxy data in dagalaxy.cpp.
	inline static int PhotonSphereRadiusInOrbits(GALAXY galaxy)
	{
		if(galaxy == GALAXY_Count)
			return 0;											// Stellar black hole.
		int eh = SchwarzschildRadiusInOrbits(galaxy);			// Event horizon orbit radius.
		int ps = eh * 3 / 2;									// Photon sphere orbit radius.
		if(ps == eh)
			ps++;												// Insure it's at least one orbit larger than the event horizon.
		return ps;
	}

	// Returns the radius of the star in feet, event horizon radius for black holes.
	double Radius() const;

	inline char RadiusInOrbits() const
	{
		return m_chRadius;
	}

	void ReserveZones(char chMinOrb, char chMaxOrb);

	// 'galaxy' of GALAXY_Count returns the event horizone radius of a stellar black hole, which is always -1.
	// Returns the event horizon radius of this supermassive black hole, in orbits, assuming it's a black hole.
	// This is based on the radius of the 'galaxy'.
	// Return value is clamped to the range -1 to 8.
	// The range of return values is 0 to 8, based on current table of s_Galaxy data in dagalaxy.cpp.
	static int SchwarzschildRadiusInOrbits(GALAXY galaxy);

	inline void SetMaxOrbits(char chMaxOrbits)
	{
		// This is a cheesy place to store this value but it's only needed temporarily by companion stars during solar system
		// generation. That process is only run on the server, which does not create device objects of stars.
		Q_ASSERT(chMaxOrbits >= 0 && chMaxOrbits <= 16);
		Q_ASSERT(!IsPrimary());
		Q_ASSERT(m_fRotation[0] == 0);
		m_fRotation[0] = chMaxOrbits;
	}

	inline SIZE Size() const
	{
		m_StarLock.lockForRead();
		SIZE v = m_eSize;
		m_StarLock.unlock();
		return v;
	}

	inline TYPE StarType() const
	{
		m_StarLock.lockForRead();
		TYPE v = m_eType;
		m_StarLock.unlock();
		return v;
	}

	// Returns the surface temperature in Kelvin.
	inline quint16 SurfaceTempK() const
	{
		m_StarLock.lockForRead();
		quint16 k = m_SurfaceTempK;
		m_StarLock.unlock();
		return k;
	}

	DaWorldGasGiant* GetGasGiant() const;

	// Query functions.
	DaWorldLL* GetHabitableWorld(HABITABILITY hab) const;

	DaWorldLL* GetWorld(ORBITZONE ozone) const;

	// 'seaDepth' causes light level to drop off. Pass 0 for no dropoff. Depth is expected to be a positive number.
	float LightAttenuation(double seaDepth) const;

	AuWormhole CheckWormholeEntry(const AuPoint& oldloc, const AuPoint& nuloc, float& entryAngle) const;

	// Color of the sun on nav consoles and sensor-type consoles.
	inline QColor ChartColor() const
	{
		return ChartColor(StarType());
	}

	// Color of the sun on nav consoles and sensor-type consoles.
	static QColor ChartColor(TYPE eType);

	// Return the color of light emitted by this type of star.
	// Star colors are from 'http://www.vendian.org/mncharity/dir3/starcolor/'.
	inline QColor LightColor() const
	{
		/*
		// Star colors are from 'http://www.vendian.org/mncharity/dir3/starcolor/'.
		QColor starColors[] =
		{
			QColor(155, 176, 255),		// Light Blue
			QColor(170, 191, 255),		// Lighter Blue
			QColor(202, 215, 255),		// Even Lighter Blue
			QColor(248, 247, 255),		// Lightest Blue, Almost White
			QColor(255, 244, 234),		// Yellow
			QColor(255, 210, 161),		// Pale Orange
			QColor(255, 204, 111),		// Red Orange
		};
		Q_ASSERT(m_eType >= TYPE_O && m_eType <= TYPE_M);
		return starColors[m_eType];
		*/

		// The light color is richer and less subtle when we use SunColor, which increases the color saturation of these colors.
		return SunColor();
	}

	// Color used for the sun sprite.
	// Colors are based on the star colors from 'http://www.vendian.org/mncharity/dir3/starcolor/', with 2x saturation.
	QColor SunColor() const;

public:
	// Rendering functions.
	virtual void RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
			const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass,
			DaAtomList& shadSc,
			DaAtomList& shadScFog,
			DaAtomList& shadScGlobe,
			DaAtomList& shadScGlobeFog,
			DaAtomList& shadScRingworld,
			DaAtomList& shadScRingworldFog);

	virtual void CreateDeviceObjects(AuAtomRenderInfo& ari);
	virtual void AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim);
	virtual void DeleteDeviceObjects(AuAtomRenderInfo& ari);

	virtual void GetRenderLightsRecursive(const DaPerson& person, const AuMatrix4x4& model, QList<AuGLSun>& suns,
			QList<AuGLLight>& lights) const;

protected:
	void CreatePlanet(int orbit, HABITABILITY hab, const AuDice& dice);
	void CreatePlanetForScan(int orbit, HABITABILITY hab, const AuDice& dice);
	void CreateRingworld(int orbit, const AuDice& dice);
	void CreateRingworldForScan(int orbit, const AuDice& dice);
	void CreateStarModel(AuOpenGLFunctions& GLf);

	// 'radOrbitAu' is orbit radius in astronomical units.
	void CreateWorld(HABITABILITY hab, double radOrbitAu, ORBITZONE orbitZone, const AuDice& dice);
	void CreateWorldForScan(HABITABILITY hab, double radOrbitAu, ORBITZONE orbitZone, const AuDice& dice);

	double AngleToCompanionStar() const;
	double Period() const;
	void CreateRenderTexture(AuGLEffects& re);
	void InitMaxOrbits(const AuDice& dice);
	virtual bool CheckPeerJoin(const AuPoint &foldloc, const AuPoint &fnuloc, const DaAtom* who);
protected:
	virtual bool CheckChildExit(const AuPoint &foldloc, const AuPoint &fnuloc, const DaAtom* who) const;

public:
	void ListPlanets(DaAtomList &alPlanets) const;
	virtual bool SolarSystemNameChanged(const QString& oldName, const QString& nuName, bool notifyDb);

	virtual void PrepareForScan();

public:
	static DaStar* LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, LOADSUBATOMS loadWhat,
			const AuId& idToLoad=0);
	virtual void Insert(QSqlDatabase& db) const;	// Insert first time database record.
	virtual void Update(QSqlDatabase& db) const;	// Update existing database record.

protected:
	// This is the first step in the activation of henge portals. It happens whenever a stargate is opened at a ringworld.
	// A henge portal is created/activated on each ringworld arc segment.
	// This causes the interconnecting systems to create/activate the destination henge portals.
	// This can be called without ever calling CreateRingworldHenges; it does all the same work to create them when needed.
	void ActivateRingworldHenges();

	// Creates a henge portal on each ringworld arc segment that does not have one.
	// This causes the interconnecting systems to create the destination henge portals.
	void CreateRingworldHenges();

	// Sets m_RingworldRefSection to a section of a ringworld used for coordinated rotation of all sections.
	// This is also the section responsible for the shadow squares object.
	void ChooseRingworldRefSection();

	void CreateBlackHoleSphereModel(AuOpenGLFunctions& GLf);

	void CreateCoronaSphereModel(AuOpenGLFunctions& GLf);

	void CreateNeutronStarSphereModel(AuOpenGLFunctions& GLf);

	void CreateNeutronSphereModel(AuOpenGLFunctions& GLf);

	void CreatePhotonSphereModel(AuOpenGLFunctions& GLf);

	void CreateStarSphereModel(AuOpenGLFunctions& GLf);

	void DeactivateRingworldHenges();

	// Returns the orbit number of the first frigid orbit. Used when creating previews.
	unsigned int FirstFrigidOrbit() const;

	// Expects m_StarLock to be locked
	void GenSurfaceTempK(AuDice& dice);

	// Returns the orbit number of the last inferno orbit. Used when creating previews.
	unsigned int LastInfernoOrbit() const;

	// Returns a number from 0 to 1.
	float NeutronStarOutput() const;

	bool TraverseWormhole(DaVessel* ship, const AuWormhole& wh);
};

#endif // DaStar_H
