//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daworld.h,v 1.198 2022/05/17 18:42:46 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaWorld_H
#define DaWorld_H

#include "auspecies.h"
#include "daatom.h"

#define WORLD_TEXTURE_FAR_VER			0x16					// WTF12345 far world texture files in temporary cache

class AuCityIndustry;
class AuStation;
class AuTerrainInfo;
class AuZoneResource;
class DaPerson;
class DaSentient;
class DaWorldGlobe;
class DaWorldRing;
class DePlanetFlora;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

struct AuWorldRes_
{
	COMMODITYID	cid;
	quint8		qual[4];				// Up to 4 resource zones ResourceZones() on a world
	quint8		prob[4];				// Up to 4 resource zones ResourceZones() on a world
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL DaWorld : public DaAtom
{
public:
	enum VEG_DENSITY_TYPE
	{
		VEG_NonePolar=0,				// No vegetation in the area.
		VEG_LightPolar,
		VEG_ModeratePolar,
		VEG_DensePolar,
		VEG_NoneTemperate,				// No vegetation in the area.
		VEG_LightTemperate,
		VEG_ModerateTemperate,
		VEG_DenseTemperate,
		VEG_NoneTropical,				// No vegetation in the area.
		VEG_LightTropical,
		VEG_ModerateTropical,
		VEG_DenseTropical,
		VEG_Types
	};

protected:
	// Unguarded
	ORBITZONE							m_OrbitZone;			// Orbit Zone (OZONE_)
	mutable quint8						m_InDesigner;			// 0=unknown, 1=no, 2=yes

private:
	mutable QReadWriteLock				m_WorldLock;
	mutable quint32						m_GenSeed;				// Cached result of GenSeed function, which is a lot of work.
	AuElapsedTimer						m_LastAirTravellerArr;	// When the last air traveller was taken by a building.
	AuElapsedTimer						m_LastAirTravellerDep;	// When the last air traveller was taken by a building.
	AuElapsedTimer						m_LastIndigenousArr;	// When the last indigenous was taken by a building.
	AuElapsedTimer						m_LastIndigenousDep;	// When the last indigenous was taken by a building.
	AuElapsedTimer						m_LastNewborn;			// When the last baby was taken by a building.
	double								m_OrbitRadius;			// Orbit radius in astronomical units (au).
	AuElapsedTimer						m_OrbitChanged;
	QString								m_sName;

public:
	DaWorld();
	virtual ~DaWorld();

protected:
	DaWorld(const DaWorld& c);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	// Add a visual and/or audio effect to this atom.
	// 'effect' may be modified to something appropriate. e.g. VISUAL_HitPierce (looks like blood) is changed to VISUAL_HitPierceBug
	// when this atom is an exoskeletal creature, and something else when this atom is not even a creature.
	virtual void AddEffect(AuEffect& effect);

	// Returns an approximation of 'airDensity' in bars.
	static float AirDensityToBar(float airDensity);

	// Returns count of all normal buildings on the world that are not street lights or fences.
	quint32 BuildingCount() const;

	// Returns count of all normal fences on the world.
	quint32 FenceCount() const;

	// Returns count of all normal street lights on the world.
	quint32 StreetLightCount() const;

	// Returns cubic meters of all normal buildings on the world, including street lights.
	quint32 BuildingVolume() const;

	virtual quint32 BuildingVolumeLimit() const;

	// This atom cannot have a movement vector.
	virtual bool CanSetInMotion() const
	{
		return false;
	}

	virtual AuId ClaimedByPersonId() const
	{
		return AuId(0);
	}

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

	virtual DaAtom *FindClosestAtom(const QPoint &pxFind, const AuMatrix4x4 &tDevicePersp, const AuMatrix4x4 &tDeviceOrtho,
			const AuMatrix4x4 &tModel) const;

	// Called when this world is hit by an asteroid of size 'asteroidSize', in the range 1 to 4.
	// Returns true if the asteroid should be Kill'd on return.
	// Returns false if the asteroid should Explode on return.
	virtual bool HitByAsteroid(quint8 asteroidSize, const AuPoint& asteroidLoc)
	{
		Q_UNUSED(asteroidSize);
		Q_UNUSED(asteroidLoc);
		return false;
	}

	// 'orbitIndex' is my new orbit index so I can become a secondary asteroid of a new belt.
	virtual void ParentDestroyedByAsteroid(DaAtom* nuParent, quint8 orbitIndex, double orbitRadiusAu)
	{
		Q_UNUSED(nuParent);
		Q_UNUSED(orbitIndex);
		Q_UNUSED(orbitRadiusAu);
	}

	// Attempts to predict the location of 'loc' in 'secondsFromNow' seconds.
	// Used by sentients when plotting a course to a planetoid moving in an asteroid belt. Super precision is not required.
	virtual AuPoint PredictLocation(const AuPoint& loc, double secondsFromNow) const
	{
		Q_UNUSED(secondsFromNow);
		return loc;
	}

	// Returns true if 'person' is in this world's chain of command.
	// EmpireId of this world is based on who holds the current claim. See DaWorldLL.
	bool InChainOfCommand(const AuId& personId) const;

	bool InDesignerPreview() const;

	inline AuElapsedTimer LastAirTravellerArrival() const
	{
		m_WorldLock.lockForRead();
		AuElapsedTimer v(m_LastAirTravellerArr);
		m_WorldLock.unlock();
		return v;
	}

	inline AuElapsedTimer LastAirTravellerDeparture() const
	{
		m_WorldLock.lockForRead();
		AuElapsedTimer v(m_LastAirTravellerDep);
		m_WorldLock.unlock();
		return v;
	}

	inline AuElapsedTimer LastIndigenousArrival() const
	{
		m_WorldLock.lockForRead();
		AuElapsedTimer v(m_LastIndigenousArr);
		m_WorldLock.unlock();
		return v;
	}

	inline AuElapsedTimer LastIndigenousDeparture() const
	{
		m_WorldLock.lockForRead();
		AuElapsedTimer v(m_LastIndigenousDep);
		m_WorldLock.unlock();
		return v;
	}

	inline AuElapsedTimer LastNewbornTaken() const
	{
		m_WorldLock.lockForRead();
		AuElapsedTimer v(m_LastNewborn);
		m_WorldLock.unlock();
		return v;
	}

	virtual quint64 MemoryUsage() const;

	virtual QString Name() const
	{
		m_WorldLock.lockForRead();
		QString v(m_sName);
		m_WorldLock.unlock();
		return v;
	}

	virtual void PrepareForScan();

	virtual bool Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari);

	virtual bool RoughExtents(AuBox &fExt) const;

	virtual void RxUpdate(QDataStream& fromServer, bool isFirstUpdate);

	inline void SetLastAirTravellerArrival()
	{
		m_WorldLock.lockForWrite();
		m_LastAirTravellerArr.restart();
		m_WorldLock.unlock();
	}

	inline void SetLastAirTravellerDeparture()
	{
		m_WorldLock.lockForWrite();
		m_LastAirTravellerDep.restart();
		m_WorldLock.unlock();
	}

	inline void SetLastIndigenousArrival()
	{
		m_WorldLock.lockForWrite();
		m_LastIndigenousArr.restart();
		m_WorldLock.unlock();
	}

	inline void SetLastIndigenousDeparture()
	{
		m_WorldLock.lockForWrite();
		m_LastIndigenousDep.restart();
		m_WorldLock.unlock();
	}

	inline void SetLastNewbornTaken()
	{
		m_WorldLock.lockForWrite();
		m_LastNewborn.restart();
		m_WorldLock.unlock();
	}

	virtual void SetName(const QString& nuName)
	{
		m_WorldLock.lockForWrite();
		m_sName = nuName;
		m_WorldLock.unlock();
	}

	// Returns a surface texture for display into a world map status display screen.
	virtual GLuint SurfaceTexture() const
	{
		return 0;
	}

	// 'hasHome' is true if the caller will give a home to the resident on 1 return.
	// 'hasResident' is true if the caller will give up a resident to the depart via the airport on -1 return.
	// Returns 1 if an air travelling citizen has arrived at this world who can be taken. Used to populate DaBd buildings that
	// are known to be connected to an airport and thus have access to space travellers. This is a citizen who is presumed to have
	// arrived at this world by air or space travel.
	// Returns -1 if a citizen departs this world.
	// This function imposes a pace to arrivals and departures that is based on morale and the hospitality of this world, assuming
	// nicer worlds attract more travellers.
	int TakeAirTraveller(qint8 morale, bool hasHome, bool hasResident);

	// 'hasHome' is true if the caller will give a home to the resident on 1 return.
	// 'hasResident' is true if the caller will give up a resident to the wilderness on -1 return.
	// 'result' is a return parameter:
	//   1 if an indigenous citizen is taken from the wilderness. Used to populate DaBd buildings.
	//   -1 if a citizen returns to the wilderness.
	// Indigenous citizens can only move to|from from the environment of habitable worlds.
	// Returns the species.
	AuSpecies TakeIndigenous(qint8 morale, bool hasHome, bool hasResident, int& result);

	// Returns true if a citizen is born from a DaBd building.
	bool TakeNewborn(qint8 morale);

	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
			const DaPerson& sceneAtom);

	//------------------
	// DaWorld Functions
public:
	// 'errcode' is an output parameter. It will be set to one of the following values:
	// 0 = success
	// 1 = an unlikely error occurred
	// 2 = design not loaded, try again
	// 6 = spawn location was blocked
	// 27 = design not ok to manufacture
	DaVessel* AgentSpawnSpacecraft(DaPerson* person, int designId, quint8& errcode);

	// Return the altitude above mean sea level at a 'loc'.
	virtual double AltitudeMSL(const AuPoint& /*loc*/) const
	{
		return 0;
	}

	// 'msecsSinceEpoch' is the time at which the angle is calculated.
	// 'rUYears' is an output parameter, set to the number of years passed in the game Universe, in that time span.
	virtual double AngleToPlanetAt(qint64 msecsSinceEpoch, double& rUYears) const;

	double AngleToPlanet() const;

	virtual QColor AtmosphereColor() const
	{
		return QColor(255,255,255);
	}

	static QColor AtmosphereColor(unsigned char byAtmType, unsigned char byAtmDensity, quint32 genSeed);

	// Returns density of atmosphere at a specific altitude above sea level.
	float AtmosphereDensity(float altitudeMSL) const
	{
		// Return 0 if this world has no atmosphere at all.
		if(!HasAtmosphere())
			return 0;					// This world has no atmosphere.

		// Calculate MSL values.
		float atmoHtMSL = (float)AtmosphereHeightMSL();
		if(altitudeMSL >= atmoHtMSL)
			return 0;					// altitude is above the height of the atmosphere.

		// Atmosphere density drops off with altitude.
		// This should come from the weather model but the server doesn't make a weather model. A linear drop off works fine for
		// the purposes that atmosphere density is used.
		float altitudeFactor = 1.f - altitudeMSL/atmoHtMSL;		// altitude factor is linearly 1 at sea level to 0 at space.
		return MSLAtmosphereDensity() * altitudeFactor;
	}

	// Returns the height of the top of the atmosphere above sea level, whether or not there is any actual atmosphere density there.
	virtual double AtmosphereHeightMSL() const
	{
		return 0;
	}

	// Not really a radius on a ringworld. Returns the height of the atmosphere above sea level instead, which is the ring surface.
	virtual double AtmosphereRadius() const
	{
		return 0;
	}

	virtual ATMTYPE AtmosphereType() const
	{
		return ATMTYPE_Standard;
	}

	virtual bool BdHere() const
	{
		return false;
	}

	// Returns true if a DaBd capitol is here with the name.
	bool CityHereNamed(const QString& cityName) const;

	// Returns true if a DaBd capitol or hq is found.
	bool CityOrBaseHere() const;

	// Test whether this planet can support any animal life.
	virtual bool CanHaveFauna() const
	{
		return false;
	}

	// Test whether this planet can support any plant life.
	// Query CanHaveFlora using a terrain elevation to be more specific.
	virtual bool CanHaveFlora() const
	{
		return false;
	}

	// Looks for an existing capitol building that would be in the jurisdiction of a police station or capitol placed at
	// 'proposedPoliceStation'.
	// 'jr' is the JurisdictionRadius of the proposed building.
	AuId CapitolInProposedJurisdiction(const AuPoint& proposedBdLoc, double jr, quint32 empireId) const;

	QString CatalogName(const DaSolarSystem* system) const;

	virtual double CGAltitude(const AuPoint& /*loc*/) const
	{
		return 0;
	}

	static QColor ChartColor(ORBITZONE eZone, unsigned char byWater)
	{
		int r,g,b;
		switch (eZone)
		{
		case OZONE_Inferno:
			r = 128 + byWater * 127/100;	// Lava level determines the amount of red.
			g = 0;
			b = 0;
			break;
		case OZONE_Inner:
			r = 192;
			g = 64;
			b = byWater * 255/100;	// Water level determines the amount of blue.
			break;
		case OZONE_Habitable:
			r = 64;
			g = 192;
			b = byWater * 255/100;	// Water level determines the amount of blue.
			break;
		default:
		case OZONE_Outer:
			r = 128;
			g = 64;
			b = byWater * 255/100;	// Water level determines the amount of blue.
			break;
		case OZONE_Frigid:
			r = 192;
			g = 128 + byWater * 128/100;
			b = byWater * 255/100;	// Water level determines the amount of blue.
			break;
		}
		if(r < 0)
			r = 0;
		if(r > 255)
			r = 255;
		if(g < 0)
			g = 0;
		if(g > 255)
			g = 255;
		if(b < 0)
			b = 0;
		if(b > 255)
			b = 255;
		return QColor(r,g,b);
	}

	// Returns distance to closest ship in orbit from a location.
	// Ignores 'shipToIgnore'.
	double ClosestShipInOrbit(const AuPoint& loc, const AuId& shipToIgnore, bool avoidCreatures) const;

	// Returns true if a compass can provide directional guidance on this world at 'loc', which is relative to this world.
	// Sets 'north' and 'east' to unit vectors from loc that point to the north and east respectively, parallel with the surface.
	// Sets 'up' to a unit vector pointing up from loc with respect to gravity.
	virtual bool CompassVectors(const AuPoint& loc, AuPoint& north, AuPoint& east, AuPoint& up) const = 0;

	// NOTE: Don't call this function unless you understand the implications.
	// Worlds that can have flora must override this function.
	virtual void CopyFlora(const DePlanetFlora* /*flora*/)
	{
	}

	void Create(double orbitRadius, ORBITZONE orbitZone);

	void CreateForScan(double orbitRadius, ORBITZONE orbitZone);

	virtual void CreateFloraTextures(AuGLEffects& /*re*/, int /*resourceZone*/)
	{
	}

	virtual QString DefaultName(const QString& solarSystemName) const = 0;

	quint32 DemandCost(const AuItem& item) const;

	// Direction this world rotates on its axis.
	// Return 1 or -1.
	virtual int DirectionOfRotation() const
	{
		return 1;
	}

	// Changes the altitude of a location to a specific 'cgaltitude' by raising/lowering it relative to gravity.
	virtual void ElevateLoc(AuPoint& /*loc*/, double /*cgaltitude*/) const
	{
	}

	// Changes the altitude of a location to a specific 'altitudeMSL' by raising/lowering it relative to gravity.
	virtual void ElevateLocMSL(AuPoint& /*loc*/, double /*altitudeMSL*/) const
	{
	}

	// Returns an atom that is in 'gunRange' of 'gunLoc', which is relative to this world.
	// 'gunnerEmpire' is the empire id of the gunner, who is at this world.
	// 'gunnerPosture' is the posture of the gunner.
	// 'targetLoc' is an output parameter relative to this world.
	DaAtom* FindGuardTowerTarget(quint32 gunnerEmpire, CITIZEN_POSTURE gunnerPosture, const AuPoint& gunLoc, float gunRange,
			AuPoint& targetLoc) const;

	DaWorldGlobe* FindHabitableMoon(HABITABILITY hab) const;

	// Find a star or world that has natural resource commodity 'resCID' at minimum quality 'minQ' in the environment.
	virtual bool FindNaturalResource(COMMODITYID resCID, quint8 minQ) const;

	// Recursive attempt to locate a 'nu' vehicle that has been manufactured.
	// 'onSlabInCity' is the id of the city in which the requestor is sitting on a road slab. May be null.
	virtual DaVehicle* FindNewVehicle(COMMODITYID vehicleCID, unsigned int empireId, const AuId& onSlabInCity)
	{
		Q_UNUSED(vehicleCID);
		Q_UNUSED(empireId);
		Q_UNUSED(onSlabInCity);
		return 0;
	}

	// Recursive attempt to locate a 'nu' vehicle that has been manufactured.
	// 'onSlabInCity' is the id of the city in which the requestor is sitting on a road slab. May be null.
	virtual DaVehicle* FindNewVehicleType(VEHICLE_SPOT_TYPE spotType, unsigned int empireId, const AuId& onSlabInCity)
	{
		Q_UNUSED(spotType);
		Q_UNUSED(empireId);
		Q_UNUSED(onSlabInCity);
		return 0;
	}

	virtual DaVehicle* FindNewVehicleType(VEHICLE_SPOT_TYPE spotType, const DaBd* fetcher)
	{
		Q_UNUSED(spotType);
		Q_UNUSED(fetcher);
		return 0;
	}

	virtual AuPoint FindOrbitSpawnLoc(const AuPoint& startLoc, const AuId& shipId, double shipRadius, double orbitAlt,
			bool avoidCreatures) const
	{
		Q_UNUSED(shipId);
		Q_UNUSED(shipRadius);
		Q_UNUSED(avoidCreatures);
		AuPoint orbitLoc(startLoc);
		ElevateLoc(orbitLoc, orbitAlt);
		return orbitLoc;
	}

	virtual DaWorld* FindParentPlanetInEnv();

	virtual const DaWorld* FindParentPlanetInEnv() const;

	virtual void FindPlanetsRecursive(DaAtomList& planets) const;

	// Find a ring orbiting this world that contains 'searchLoc', which is in the space of this world.
	virtual DaWorldRing* FindRing(const AuPoint& searchLoc) const = 0;

	// Recursive search for the resources needed by starting players.
	// This base class function just searches subordinate world atoms.
	virtual void FindStartupResources(int& eludium, int& lumenite);

	bool HasAlienCapitol(quint32 empireId) const;

	bool HasAlienMilitaryBuilding(quint32 empireId) const;

	bool HasAlienMilitaryHQ(quint32 empireId) const;

	bool HasAnyCapitol() const;

	bool HasAnyMilitaryHQ() const;

	// Returns true if any building is a military building that is not a headquarters.
	bool HasAnyMilitaryNotHQ() const;

	// Returns true if any police stations exist that belong to 'capitolId'.
	bool HasCityPolice(const AuId& capitolId) const;

	bool HasEmpireMilitaryHQ(quint32 empireId, bool underConstructionOk) const;

	// Returns true if a Bd hq is here.
	bool HQHereNamed(const QString& cityName) const;

	bool InAnyJurisdiction(const AuPoint& loc) const;

	bool InAlienJurisdiction(const AuPoint& loc, quint32 empireId) const;

	bool InEmpireJurisdiction(const AuPoint& loc, quint32 empireId) const;

	virtual bool IsAsteroidBeltPrimary() const
	{
		return false;
	}

	virtual bool IsEthereal() const
	{
		return false;
	}

	virtual bool IsPlanetoidSize() const
	{
		return false;
	}

	virtual bool IsGasGiant() const
	{
		return false;
	}

	// Returns true for worlds that are planetoids in an asteroid belt in a primary orbit, not a ring around a planet like Saturn.
	// Can include moon sized worlds that were once satellites.
	virtual bool IsInAsteroidBelt() const
	{
		return false;
	}

	virtual bool IsInVacuum() const
	{
		return !HasAtmosphere();
	}

	// Returns true for worlds that are moon-sized or smaller.
	virtual bool IsMoonOrPlanetoidSize() const
	{
		return false;
	}

	virtual quint8 OrbitIndex() const
	{
		return 0;
	}

	// Returns the id of the capitol bd if 'loc' falls inside any jurisdiction.
	// 'cityEmpireId' is set to the empire id of the jurisdiction, if any, otherwise untouched.
	AuId JurisdictionAt(const AuPoint& loc, quint32& cityEmpireId) const;

	// Returns true if a police station or capitol placed at 'proposedBdLoc' would create an overlap between two city jurisdictions.
	// 'jr' is the JurisdictionRadius of the proposed building.
	// 'isCapitol' is true if the proposed building is a capitol building.
	bool WouldCreateOverlappingJurisdictions(const AuPoint& proposedBdLoc, double jr, bool isCapitol) const;

	// Queries for a sorted list of moon sizes orbiting this world.
	// Map key is orbit radius of the moon; value is AtmosphereRadius of the moon, 0 if it's a ring.
	void ListMoons(QMap<double, double>& moons) const;

	// Plant DNA, one entry for each resource zone.
	// Worlds that can have flora must override this function.
	virtual const DePlanetFlora* LockFlora() const
	{
		return 0;
	}

	// Plant DNA, one entry for each resource zone.
	virtual DePlanetFlora* LockFloraForWrite()
	{
		return 0;
	}

	// Worlds that can have flora must override this function.
	virtual void UnlockFlora() const
	{
	}

	// Returns the GL texture id for a given resource zone. Will create the texture if it hasn't been created yet.
	// Worlds that can have terrain and flora must override this function.
	virtual GLuint FloraTexture(int /*resourceZone*/) const
	{
		return 0;
	}

	// Returns a seed value that can be used for generating things about this world pseudo-randomly.
	// This seed value is derived from the seed value of my parent star and this planet's position in the sub atom hierarchy. It
	// is only valid after the solar system contents are final.
	// This is not the seed used when the world was originally Create'd. That sequence is seeded in DaSolarSystem when it creates
	// its contents.
	virtual quint32 GenSeed() const;

	virtual AuPoint GravityDir(const AuPoint& /*loc*/) const
	{
		return AuPoint(0,0,1);
	}

	virtual bool HasAtmosphere() const
	{
		return false;
	}

	virtual bool HasBreathableAtmosphere() const
	{
		return false;
	}

	// True if stance of empire is stance or less toward all cities on the planet.
	// True if no cities on the planet.
	// 'Or less' is in the sense of Enemy being less than Friend or Neutral. This is not based on the actual values of the
	// POLITICAL_STANCE enum.
	// Used when determining if a citizen has RIGHT_TO_FOUND authority.
	bool HasCityStanceOrLess(quint32 empireId, POLITICAL_STANCE stanceOrLess);

	inline HYDROGRAPHIC_LIQUID HydrographicLiquid() const
	{
		if(IsGasGiant())
			return HYDRO_SupercooledLiquid;
		if(Hydrographics() == 0)
			return HYDRO_None;
		switch(OrbitZone())
		{
		case OZONE_Inferno:
			return HYDRO_Lava;
		case OZONE_Inner:
		case OZONE_Habitable:
		case OZONE_Outer:
			return HYDRO_Water;
		case OZONE_Frigid:
			return HYDRO_SupercooledLiquid;
		case OZONE_DeepSpace:
			break;
		}
		return HYDRO_None;
	}

	// Return a value from 0 to 100. This is the percentage of surface covered by water, magma, or supercooled liquid.
	// Worlds with seas should override this function.
	virtual quint8 Hydrographics() const
	{
		return 0;
	}

	// Returns the CG altitude of 'loc' if it is inside the atmosphere envelope of this world.
	// Returns -100000 if 'loc' is not inside the atmosphere envelope.
	virtual double InAtmosphereCGAltitude(const AuPoint& /*loc*/) const
	{
		return -100000;
	}

	void InitSpawnLocation(DaPerson* pu);

	virtual bool IsHabitable(HABITABILITY hab) const
	{
		Q_UNUSED(hab);
		return false;
	}

	inline bool IsHabitableOrbit() const
	{
		return m_OrbitZone == OZONE_Habitable;
	}

	virtual bool IsObstacle(const DaAtom* toWho) const
	{
		Q_UNUSED(toWho);
		return true;
	}

	inline bool IsSatellite() const
	{
		// Returns true if this body orbits another body.
		return dynamic_cast<const DaWorld*>(Parent()) ? true : false;
	}

	// Slightly pickier criteria than IsHabitablePlanet.
	virtual bool IsSuitableHomePlanet() const
	{
		return false;
	}

	// Lists troops of 'empireId' that are in city 'cityId'. Troops that are listed are not in vehicles, spacecraft, or on mounts.
	void ListTroopsInCity(const AuId& cityId, quint32 empireId, DaAtomList& troopsInCity) const;

	// Lists people online and troops that are in city 'cityId'.
	// Troops and people that are listed are not in vehicles, spacecraft, or on mounts.
	void ListTroopsAndPeopleOnlineInCity(const AuId& cityId, DaAtomList& troopsAndPeopleInCity) const;

	// Returns atmosphere density at sea level in the range 0..1.
	// Worlds with atmospheres should override this function.
	virtual float MSLAtmosphereDensity() const
	{
		return 0;
	}

	// Resource quality varies by resource zone across the planet.
	virtual quint8 NaturalResourceQuality(COMMODITYID cid, int resourceZone) const = 0;

	// 'fromLoc' and 'toLoc' are relative to this world.
	// Returns next location in direction of path that will avoid obstructions to get to 'toLoc' from 'fromLoc'.
	// Returns 'toLoc' if the path is not obstructed.
	// Returns 0,0 if no path could be found.
	virtual AuPoint NextPathLocation(const AuPoint& /*fromLoc*/, const AuPoint& /*toLoc*/) const
	{
		return AuPoint(0,0);
	}

	// Land distance from pole to pole on a sphere, wall to wall on a ringworld.
	virtual double NorthSouthDistance() const = 0;

	// Test if empireId has authority to assign a nu name to this world.
	// Returns:
	//   0 = ok to rename
	//   1 = no cities on this world
	//   2 = occupied city is present on this world or a moon of this world if this is a gas giant
	//   3 = flag of other empire is present on this world or a moon of this world if this is a gas giant
	virtual int OkToRename(quint32 empireId) const = 0;

	// Returns orbit radius in feet.
	inline double OrbitRadius() const
	{
		return OrbitRadiusAu() * FeetPerAu();
	}

	// Returns orbit radius in astronomical units (au).
	inline double OrbitRadiusAu() const
	{
		m_WorldLock.lockForRead();
		double v = m_OrbitRadius;
		m_WorldLock.unlock();
		return v;
	}

	inline void SetOrbitRadiusAu(double radau)
	{
		m_WorldLock.lockForWrite();
		m_OrbitRadius = radau;
		m_OrbitChanged.Set();
		m_WorldLock.unlock();
	}

	inline AuElapsedTimer OrbitChangeTime() const
	{
		m_WorldLock.lockForRead();
		AuElapsedTimer v(m_OrbitChanged);
		m_WorldLock.unlock();
		return v;
	}

	inline ORBITZONE OrbitZone() const
	{
		return m_OrbitZone;
	}

	// This is an approximation of the period based roughly on Keppler's formula:
	//	t^2 = r^3 where t is time and r is radius.
	// The return result is in earth years.
	// The earth standard is considered OrbitRadius=1au
	virtual double Period() const;

	// Returns the maximum population allowed in each resource zone before cities apply an overpopulation penalty to morale.
	// This is fairly aggressive so people will need to spread into the galaxy rather than infest the same planet.
	// This is necessary to help prevent a single solar system from taking too long in the tick cycle.
	virtual int PopulationMax() const
	{
		return 0;
	}

	virtual quint32 WorldPopulation() const = 0;

	// Called to trigger recalc of cached population value(s).
	virtual void WorldPopulationChanged()
	{
	}

	// Returns the maximum population allowed before indigenous stop reproducing and air travellers stop coming here.
	virtual quint32 WorldPopulationMax() const = 0;

	virtual void ProcessDemandTally(const AuId& /*idCity*/, const AuItem& /*item*/,
			unsigned int& stock, unsigned int& population, unsigned int& needCount, unsigned int& produceCount) const
	{
		stock = 0;
		population = 0;
		needCount = 0;
		produceCount = 0;
	}

	// Returns information about resources available in one of the resource zones of this world.
	virtual void QueryNaturalResources(AuZoneResource& res, int resourceZone) const = 0;

	// Determine the intensity of sunlight striking the surface location from each sun.
	// Light level is adjusted for water depth.
	// This does not consider sloping terrain or shadows.
	// In habitable zone, noon sun intensity is 1.0.
	virtual float QuerySunlight(const AuPoint& /*fSurfLoc*/) const
	{
		return 0.f;
	}

	// Determine the intensity of sunlight striking the sea level location from each sun.
	// 'fSurfLoc' is adjusted to 1' MSL altitude.
	// This does not consider sloping terrain or shadows.
	// In habitable zone, noon sun intensity is 1.0.
	inline float QuerySunlightAtSeaLevel(const AuPoint& loc) const
	{
		AuPoint surfLoc(loc);
		ElevateLocMSL(surfLoc, 1);
		return QuerySunlight(surfLoc);
	}

	// This is just a stub that returns a default constructed 'ter'. Must override for real info.
	virtual void QueryTerrainAtLoc(AuTerrainInfo& ter, const AuPoint& fSurfLoc) const;

	virtual void QueryTreeTrunkSize(COMMODITYID /*treeCID*/, int /*resourceZone*/, AuFloraDNA::ZONE /*latRegion*/,
			float& trunkLength, float& trunkRadius) const
	{
		trunkLength = 0;
		trunkRadius = 0;
	}

	// This function prefers locations on land in daylight.
	virtual AuPoint RandomAvatarLocation() const
	{
		return AuPoint(0,0);
	}

	// Notifies cities of enemy sensor contact.
	void ReportEnemySensorContact(const DaSolarSystem* system, quint32 empireId, const AuSensorBlipList& sensorBlips,
			float sensorRange);

	// Returns the resource zone number of a given location.
	virtual int ResourceZone(const AuPoint& loc) const = 0;

	// Returns the number of resource zones on this world.
	virtual int ResourceZones() const
	{
		return 0;
	}

	// Blue for water, red for magma, etc.
	virtual QColor SeaColor() const;

	virtual float SeaFogDensity() const;

	// Spherical worlds return the radius of their ball of dirt. Ringworlds return 0.
	// This differs from the sea level radius on gas giants.
	virtual double GlobeRadius() const
	{
		return 0;
	}

	// Spherical worlds return their radius. Ringworlds return 0.
	virtual double SeaLevelAltitude() const
	{
		return 0;
	}

	// Returns the atmosphere pressure at sea level, in bar.
	// We consider Earth to be about 50% atmosphere density.
	// Earth standard atmospheric pressure at sea leval is one bar.
	float SeaLevelPressure() const;

	// Returns the temperature of this world at sea level, in kelvin.
	// 'longLat' holds longitude and latitude angles in x and y, in radians.
	// Worlds that can have weather should override this function.
	virtual float SeaLevelTempAt(const AuGlobeLLF& /*longLat*/, float /*solarOrbitRadius*/, float /*sunLightAtten*/) const
	{
		return 0;
	}

	virtual bool SolarSystemNameChanged(const QString& oldName, const QString& nuName, bool notifyDb);

	// Finds vehicles that are outside on the ground in a city on this planet that can be used for production processes.
	// Only one of each vehicle type is wanted/needed.
	virtual void TallyVehicles(const AuId& /*cityId*/, quint32 /*empireId*/, QList<COMMODITYID>& /*vehicles*/) const
	{
	}

	// Sea surface is slightly transparent in bright sun.
	// 'illumination' is light level at viewer. 1 is noonday sun.
	// Returns the opacity value that will be used by the Water shader for rendering water surface at a given illumination level.
	// Range is 0..1.
	inline float WaterOpacity(float illumination) const
	{
		float illumFactor = 1.f - illumination;
		if(illumFactor < 0.f)
			illumFactor = 0.f;
		return illumFactor * .05f + .95f;
	}

	// Returns true if worker dna is valid on this planet.
	virtual bool HasWorkerSpecies() const = 0;

	// Returns true if 'dna' is not sexually compatible with the indigenous population.
	// Returns false if there is no indigenous population.
	bool IsAlienSpecies(const AuSpecies& species) const;

	// Sets/replaces the current worker DNA.
	virtual void SetWorkerSpecies(const AuSpecies& species) = 0;

	// The caller must delete the pointer that is returned.
	virtual AuSpecies WorkerSpecies() const = 0;

	QString WorkerSpeciesDesc() const;

	QString WorkerSpeciesName() const;

protected:
	// Returns the number of minutes between people arriving by air travel.
	virtual int AirTravellerInterval(qint8 morale) const = 0;

	// Calculates the current population based on calls to the Population() function of my subatoms.
	quint32 CalcWorldPopulation() const;

	// Returns the number of minutes between people being produced by the environment. Returns 0 if the environment cannot naturally
	// produce people or if no indigenous DNA has been established.
	virtual int IndigenousPopulationInterval(qint8 morale) const = 0;

	// Returns the number of minutes between people being born.
	unsigned int NewbornInterval(qint8 morale) const;

	inline quint32 RawGenSeed() const
	{
		m_WorldLock.lockForRead();
		quint32 v = m_GenSeed;
		m_WorldLock.unlock();
		return v;
	}

	inline void SetGenSeed(quint32 genSeed) const
	{
		m_WorldLock.lockForWrite();
		m_GenSeed = genSeed;
		m_WorldLock.unlock();
	}

	void ReportZoneBioResources(int rz, const AuZoneResource& res, COMMODITYID zoneres[], int zonerescount,
			unordered_map<COMMODITYID, int, COMMODITYID_Hash>& zonecount, QMap<QString, AuWorldRes_>& zonelist) const;

	void ReportZoneBioResources(const AuZoneResource& res, COMMODITYID zoneres[], int zonerescount,
			QMultiMap<quint8, COMMODITYID>& scanRes) const;

	void ReportZoneFetchResources(int rz, const AuZoneResource& res, COMMODITYID zoneres[], int zonerescount,
			unordered_map<COMMODITYID, int, COMMODITYID_Hash>& zonecount, QMap<QString, AuWorldRes_>& zonelist) const;

	void ReportZoneFetchResources(const AuZoneResource& res, COMMODITYID zoneres[], int zonerescount,
			QMultiMap<quint8, COMMODITYID>& scanRes) const;
};

#endif // DaWorld_H
