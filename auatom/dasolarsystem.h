//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dasolarsystem.h,v 1.302 2022/05/17 16:41:40 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DaSolarSystem_H
#define DaSolarSystem_H

#include "aucommodityexchange.h"
#include "daatom.h"
#include <QReadWriteLock>
#include <QVector>

class AuDice;
class AuItem;
class DaWorld;
class DaStar;
class DaVehicle;
class DaWorldGlobe;
class AuMatrix4x4;
class AuSegmentArray;

class AuSysDet_
{
public:
	quint32								m_SignalStrength;
	AuTime								m_When;

public:
	inline AuSysDet_(quint32 signalStrength)
	{
		m_SignalStrength = signalStrength;
		m_When.Set();
	}

	inline AuSysDet_()
	{
		m_SignalStrength = 0;
	}

public:
	inline void operator>>(QDataStream& out) const
	{
		out << m_SignalStrength;
		out << m_When;
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_SignalStrength;
		in >> m_When;
	}

public:
	inline void UpdateSignalStrength(quint32 signalStrength)
	{
		// Remember the most powerful signal strength.
		if(m_SignalStrength < signalStrength)
			m_SignalStrength = signalStrength;
		m_When.Set();
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuSysDet_& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuSysDet_& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuHengeDest_
{
public:
	quint8								m_FromArcSegment;
	quint8								m_FromShadowPanels;
	quint8								m_FromSun;
	AuId								m_FromSysId;
	bool								m_HasBreathableAtm;
	AuGlobeLLF							m_HengeLL;
	AuId								m_MySectorId;
	ORBITZONE							m_OrbitZone;
	bool								m_OutsideCity;
	quint8								m_ToArcSegment;
	quint8								m_ToSun;
	DaAtom::ATOM_TYPE					m_WorldAtomType;
	QString								m_WorldCatName;
	QString								m_WorldName;

public:
	inline AuHengeDest_(const AuId& mySectorId,
			const AuId& fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			DaAtom::ATOM_TYPE worldAtomType, const QString& worldCatName, const QString& worldName,
			const AuGlobeLLF& hengeLL, bool hasBreathableAtm, ORBITZONE orbitZone, bool outsideCity,
			quint8 toSun, quint8 toArcSegment)
	{
		m_FromArcSegment = fromArcSegment;
		m_FromShadowPanels = fromShadowPanels;
		m_FromSun = fromSun;
		m_FromSysId = fromSysId;
		m_HasBreathableAtm = hasBreathableAtm;
		m_HengeLL = hengeLL;
		m_MySectorId = mySectorId;
		m_OrbitZone = orbitZone;
		m_OutsideCity = outsideCity;
		m_ToArcSegment = toArcSegment;
		m_ToSun = toSun;
		m_WorldAtomType = worldAtomType;
		m_WorldCatName = worldCatName;
		m_WorldName = worldName;
	}

	inline AuHengeDest_()
	{
		m_FromArcSegment = 0;
		m_FromShadowPanels = 0;
		m_FromSun = 0;
		m_HasBreathableAtm = false;
		m_OrbitZone = OZONE_Inferno;
		m_OutsideCity = false;
		m_ToArcSegment = 0;
		m_ToSun = 0;
		m_WorldAtomType = DaAtom::ATOM_Invalid;
	}

	inline AuHengeDest_(const AuHengeDest_& c)
	{
		m_FromArcSegment = c.m_FromArcSegment;
		m_FromShadowPanels = c.m_FromShadowPanels;
		m_FromSun = c.m_FromSun;
		m_FromSysId = c.m_FromSysId;
		m_HasBreathableAtm = c.m_HasBreathableAtm;
		m_HengeLL = c.m_HengeLL;
		m_MySectorId = c.m_MySectorId;
		m_OrbitZone = c.m_OrbitZone;
		m_OutsideCity = c.m_OutsideCity;
		m_ToArcSegment = c.m_ToArcSegment;
		m_ToSun = c.m_ToSun;
		m_WorldAtomType = c.m_WorldAtomType;
		m_WorldCatName = c.m_WorldCatName;
		m_WorldName = c.m_WorldName;
	}

public:
	inline void operator>>(QDataStream& out) const
	{
		quint8 bitsOzone = m_OrbitZone;
		if(m_HasBreathableAtm)
			bitsOzone |= 0x10;
		if(m_OutsideCity)
			bitsOzone |= 0x20;
		out << bitsOzone;

		out << m_FromArcSegment;
		out << m_FromShadowPanels;
		out << m_FromSun;
		out << m_FromSysId;
		out << m_HengeLL;
		out << m_MySectorId;
		out << m_ToArcSegment;
		out << m_ToSun;
		out << (quint8)m_WorldAtomType;
		out << m_WorldCatName;
		out << m_WorldName;
	}

	inline void operator<<(QDataStream& in)
	{
		quint8 bitsOzone;
		in >> bitsOzone;
		m_OrbitZone = (ORBITZONE)(bitsOzone & 0x0f);
		m_HasBreathableAtm = bitsOzone & 0x10;
		m_OutsideCity = bitsOzone & 0x20;

		in >> m_FromArcSegment;
		in >> m_FromShadowPanels;
		in >> m_FromSun;
		in >> m_FromSysId;
		in >> m_HengeLL;
		in >> m_MySectorId;
		in >> m_ToArcSegment;
		in >> m_ToSun;
		quint8 ui8;
		in >> ui8;
		m_WorldAtomType = (DaAtom::ATOM_TYPE)ui8;
		in >> m_WorldCatName;
		in >> m_WorldName;
	}

public:
	inline bool IsHenge(const AuId& mySectorId, const AuId& fromSysId, quint8 fromSun, quint8 fromShadowPanels,
			quint8 fromArcSegment, quint8 toSun, quint8 toArcSegment) const
	{
		if(m_MySectorId != mySectorId)
			return false;
		if(m_FromSysId != fromSysId)
			return false;
		if(m_FromSun != fromSun)
			return false;
		if(m_FromShadowPanels != fromShadowPanels)
			return false;
		if(m_FromArcSegment != fromArcSegment)
			return false;
		if(m_ToSun != toSun)
			return false;
		if(m_ToArcSegment != toArcSegment)
			return false;
		return true;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuHengeDest_& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuHengeDest_& o)
{
	o << in;
	return in;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUATOM_DLL DaSolarSystem : public DaAtom
{
private:
	// Unguarded
	bool								m_Final;				// true if this solar system is final. Reset when it decays.
	HABITABILITY						m_GenHab;				// HABITABILITY of this system. Never changes.
	quint32								m_GenSeed;				// Random seed value used to generate the system contents.
	quint8								m_HighestQShip;			// Highest Q non-pirate ship to visit this system.
	bool								m_isOnlyStarFieldLoaded;
	mutable quint8						m_StrongGravity;		// 0=unknown, 1=no, 2=supermassive, 3=stellar, 4=neutron star
	bool								m_WormholesFinalized;	// True if wormholes were repaired, until db updated.
	bool								m_WormholesPositioned;	// False until positioned in Tock.

private:
	mutable QMutex						m_LastAvatarAccountActiveLock;
	AuTime								m_LastAvatarAccountActive;		// When last avatar active signal received

private:
	mutable QReadWriteLock				m_CatalogNameLock;
	mutable QString						m_CatalogName;			// My catalog name, cached when queried.
	mutable bool						m_CatalogNameValid;

private:
	mutable QReadWriteLock				m_DetectedByLock;
	unordered_map<quint32, AuSysDet_>	m_DetectedBy;			// Empires that have detected emanations from this system.
																// ONLY SCENE MASTER accumulates and stores this info in the db.

private:
	mutable QReadWriteLock				m_EmpiresPresentLock;
	unordered_map<quint32, AuElapsedTimer>	m_EmpiresPresent;		// Empires of units and cities in this system. Entries decay if not
																// refreshed.

private:
	mutable QReadWriteLock				m_ExploredByLock;
	QVector<quint32>					m_ExploredBy;			// Empires that have explored this system.

private:
	mutable QReadWriteLock				m_ScannedByLock;
	QSet<quint32>						m_ScannedBy;			// Empires that have scanned this system.

private:
	mutable QReadWriteLock				m_LastFlagInfluenceLock;
	AuTime								m_LastFlagInfluence;	// Last date a flag was influenced by an enemy in this system.

private:
	mutable QReadWriteLock				m_LastOfficerVisitLock;
	AuElapsedTimer						m_LastOfficerMod;		// Last time I called SetModified due to an officer visit.
	AuTime								m_LastOfficerVisit;		// Last date an AI officer in command of a ship was in this system.

private:
	mutable QReadWriteLock				m_LastPersonVisitLock;
	AuElapsedTimer						m_LastPersonMod;		// Last time I called SetModified due to a person visit.
	AuElapsedTimer						m_LastPersonScene;		// Last date a person generated a scene in this system.
	AuTime								m_LastPersonVisit;		// Last date a person on-line was in this system.

private:
	mutable QReadWriteLock				m_LastPirateLock;
	AuElapsedTimer						m_LastPirateShip;		// Last date a pirate ship was in this system. Not stored.
	AuElapsedTimer						m_LastPirateStation;	// Last date a pirate station spawned in this system.

private:
	mutable QReadWriteLock				m_NameLock;
	QString								m_sName;				// The assigned name of this system.

private:
	mutable QMutex						m_WarpSignatureLock;	// Guards m_WarpSignature during Think.
	AuElapsedTimer						m_LastWarpMod;			// Last time I called SetModified due to warp drives.
	quint32								m_WarpSignature;		// Traces of FTL drives being engaged. Reset to 0 in Tock.

private:
	mutable QReadWriteLock				m_StarLightLock;
	mutable float						m_StarLight;			// Ambient starlight here from this sector and neighboring sectors.
																// -1 if not set.

private:
	mutable QReadWriteLock				m_WormholesLock;
	AuWormholeArray						m_Wormholes;			// Wormholes and who knows of them.

private:
	mutable QReadWriteLock				m_CommodityExchangeLock;
	AuCommodityExchange					m_CommodityExchange;	// Not stored in db.

private:
	mutable QMutex						m_HengeDestLock;
	QList<AuHengeDest_>					m_HengeDest;			// Cached results of calls to HengeCreated. Only when !IsFinal.

public:
	DaSolarSystem();
	virtual ~DaSolarSystem();

protected:
	DaSolarSystem(const DaSolarSystem &rhs);

public:	// Operators
	virtual void operator>>(QDataStream& out) const;

	virtual void operator<<(QDataStream& in);

public:
	// Returns the radius of this solar system based on the farthest orbiting body from the sun at this moment.
	// Radius returned is from center of primary star to center of farthest orbiting body.
	double ActualRadius() const;

	// Adjusts 'navLoc' so it is not inside the atmosphere/photosphere of any planets/stars or other objects a navigator would
	// perceive.
	// Result will be a close orbit radius outside of those.
	// 'shipLoc' is the reference loc for deciding where to move 'navLoc'. It is probably the location of a navigator's ship.
	// 'shipLoc' and 'navLoc' are in solar system coordinates.
	// Recursive.
	virtual void AdjustNavLocation(const AuPoint& shipLoc, AuPoint& navLoc) const;

	void AdminGoTo(DaAtom* moveAtom, DaAtom* destAtom);

	// Returns asteroid belt density at 'envLoc', for particle effects.
	float AsteroidBeltDensity(const AuPoint& envLoc) const;

	// Looks for a DaAsteroid in this system that is targetted at 'targetId'.
	// Returns the asteroids target impact time.
	// Returns AuTime() if none is found.
	// 'asteroidLoc' is a return parameter, set to the EnvLocation of the asteroid.
	AuTime AsteroidImpactTime(const AuId& targetId, AuPoint& asteroidLoc) const;

	virtual void AvatarAccountActiveRecursive(const AuIdList& land);

	// This atom cannot have a movement vector.
	virtual bool CanSetInMotion() const
	{
		return false;
	}

	// Returns the default name of this solar system, based on galaxy location.
	// e.g. WOU'WWK'JL'a
	// Returns blank for IsEmptySystem.
	QString CatalogName() const;

	// 'galaxyLoc' is in pc.
	static QString CatalogName(const AuPoint& galaxyLoc);

	void CommodityAirBaseSupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const;

	// Called by a building when it has a commodity available for export, even if restricted to its own empire.
	void CommodityAvailable(COMMODITYID cid, const AuId& bdId, quint32 empireId);

	void CommodityCantinaSupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const;

	void CommodityDockSupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const;

	// 'cidsAvailable' is filled with cids from 'cidsSought' that are available.
	void CommodityFindSupplies(QList<COMMODITYID>& cidsAvailable, const QList<COMMODITYID>& cidsSought,
			const QHash<AuId, int>& tradeConnections) const;

	void CommodityGrocerySupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const;

	void CommodityHospitalSupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const;

	void CommodityHQSupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const;

	void CommodityMotorPoolSupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const;

	// Called when a building no longer has a commodity available for export.
	void CommodityNotAvailable(COMMODITYID cid, const AuId& bdId);
	
	void CommodityRetailSupplies(QList<COMMODITYID>& cidsAvailable, const QHash<AuId, int>& tradeConnections) const;

	// Query for suppliers when a building in an empire wants to import a commodity.
	void CommoditySuppliers(COMMODITYID cid, quint32 importEmpireId, AuIdList& suppliers);

	// Query all buildings for supplies of commodities.
	void CommoditySupplierUpdate();

	void CreateHengeDestinations();

	// 'departAtom' is an atom departing from this system through deep space.
	// 'departLoc' is the location of the departing atom when it failed CheckChildExit, relative to this system.
	// Atoms in tow will be taken with the departing atom.
	bool DepartedOnDeadHeading(DaAtom* departAtom, const AuPoint& departLoc);

	// Power consumed per parsec range at which a system's emission can be detected. Detection range is 1 pc per
	// DetectionRangeFactor power units consumed by a system in an emission period. Raise this value to reduce the detection range
	// of solar system power emissions.
	static inline int DetectionRangeFactor()
	{
		return EmissionPeriod()*5000;
	}

	virtual void DetectWarpSignatureRecursive(const AuId& systemId, quint32 warpFactors, const AuPoint& galaxyLoc);

	// Hours between solar system warp signature emissions.
	static inline int EmissionPeriod()
	{
		return 1;
	}

	// Milliseconds between solar system warp signature emissions.
	static inline int EmissionPeriodMs()
	{
		return EmissionPeriod()*d_MSecPerHr;
	}

	// Returns my location relative to my parent galaxy, in parsecs.
	AuPoint GalaxyLoc() const;

	bool HasBlackHole() const;

	inline bool HasHengeDestination() const
	{
		m_HengeDestLock.lock();
		bool hengeDestEmpty = m_HengeDest.empty();
		m_HengeDestLock.unlock();
		return !hengeDestEmpty;
	}

	// 'empireId' is the pirate empire who has the base, if one is found.
	bool HasPirateBase(quint32& empireId) const;

	// Returns true if the portal exists, regardless of whether we created it just now.
	bool HengeActivated(const AuId& fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			quint8 toSun, quint8 destArcSegment);

	void HengeCreated(const AuId& fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			quint8 toSun, quint8 destArcSegment);

	void HengeDeactivated(const AuId& fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			quint8 toSun, quint8 destArcSegment);

	void HengeTeleportArrived(DaAtom* arrived, quint8 destSun, quint8 destArc, const AuId& fromSysId, quint8 fromSun,
			quint8 fromArc);

	// Returns true if this solar system is the black hole at the core of a galaxy.
	bool IsCoreBlackHole() const;

	inline bool IsEmptySystem() const
	{
		return m_GenHab == HABITABLE_EmptySystem;
	}

	virtual bool IsEthereal() const
	{
		return true;
	}

	// Solar system contents are only ticked when their contents are active. i.e. needed by players on line or officers.
	// On false return, all system contents stop getting Ticked.
	// Essential functions of cities and other atoms are performed in the Tock cycle.
	bool IsTickingContents() const;

	inline bool IsTockingContents() const
	{
		return !IsOnlyStarFieldLoaded();
	}

	// Returns the atom blocking the location.
	// 'systemLoc' is the solar system location of the atom who is checking.
	// 'pilotAvoidRadius' is the radius of the atom who is checking.
	const DaAtom* LocationBlocked(const AuPoint& systemLoc, double pilotAvoidRadius) const;

	void RememberHengeDestination(const AuId& mySectorId,
			const AuId& fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			ATOM_TYPE worldAtomType, const QString& worldCatName, const QString& worldName,
			const AuGlobeLLF& hengLL, bool hasBreathableAtm, ORBITZONE orbitZone, bool outsideCity,
			quint8 toSun, quint8 toArcSegment);

	inline void ResetWormholesFinalized()
	{
		m_WormholesFinalized = false;
	}

	// Returns a value that indicates the source of irresistable gravitational forces.
	// Returns: 0=no, 1=supermassive, 2=stellar, 3=neutron star
	int StrongGravity() const;

	// Returns the number of minutes remaining on any positive stargate that is active.
	AuTime PositiveStargateExpiry() const;

	// Call when warp FTL drives are engaged, with 'warpFactor' set to its warp factor.
	// Accumulated warp factor usage is emitted in Tock as a signal that can be detected by cities in other solar systems.
	void WarpEngaged(quint32 warpFactor);

	// Returns the DaStar or DaWorld atom that is blocking entry to the wormhole at 'loc'.
	const DaAtom* WormholeBlocked(const AuPoint& loc) const;

	// Locates atc and enemy radar within range of 'loc'. No effort is made to find the closest.
	// 'atcCity' and 'enemyCity' is a DaBd
	void FindATC(const AuPoint& shipLoc, quint32 shipEmpire, const DaVessel*& atcStation, const DaAtom*& atcCity,
			const DaVessel*& enemyStation, const DaAtom*& enemyCity) const;

	const DaStar* FindDestinationStar(const AuId& id, const QString& name) const;

	virtual const DaWorld* FindDestinationWorld(const AuId& id, const QString& name) const;

	// Recursive attempt to locate a 'nu' vehicle that has been manufactured.
	DaVehicle* FindNewVehicle(COMMODITYID vehicleCID, unsigned int empireId);

	// Recursive attempt to locate a 'nu' vehicle that has been manufactured.
	DaVehicle* FindNewVehicleType(VEHICLE_SPOT_TYPE spotType, unsigned int empireId);
	DaVehicle* FindNewVehicleType(VEHICLE_SPOT_TYPE spotType, const DaBd* fetcher);

	virtual DaBd* FindParentBdInEnv();

	virtual const DaBd* FindParentBdInEnv() const;

	virtual DaCapsule* FindParentCapsuleInEnv();

	virtual const DaCapsule* FindParentCapsuleInEnv() const;

	virtual DaDesignBase* FindParentDesignBaseInEnv();

	virtual const DaDesignBase* FindParentDesignBaseInEnv() const;

	virtual DaDesigner* FindParentDesignerInEnv();

	virtual const DaDesigner* FindParentDesignerInEnv() const;

	virtual DaWorld* FindParentPlanetInEnv();

	virtual const DaWorld* FindParentPlanetInEnv() const;

	virtual DaSolarSystem* FindParentSolarSystem();

	virtual const DaSolarSystem* FindParentSolarSystem() const;

	virtual DaSolarSystem* FindParentSolarSystemInEnv();

	virtual const DaSolarSystem* FindParentSolarSystemInEnv() const;

	virtual DaStar* FindParentStarInEnv();

	virtual const DaStar* FindParentStarInEnv() const;

	virtual DaVessel* FindParentVesselInEnv();

	virtual const DaVessel* FindParentVesselInEnv() const;

	virtual DaWorldLL* FindParentWorldInEnv();

	virtual const DaWorldLL* FindParentWorldInEnv() const;

	// Finds what should be the one-and-only DaStar atom subordinate to this solar system.
	DaStar* FindPrimaryStar() const;

	// Finds the DaStar that is considered the primary star. In a black hole system, this will actually be the closest companion
	// star, per StarNumber.
	DaStar* FindPrimaryStarNotBlackHole() const;

	// Returns the star with a stargate that is either an origin or destination, depending on 'origin'.
	// Returns 0 if none found.
	DaStar* FindStargateStar(bool origin) const;

	// Returns the habitibility value used when CreatePlanets was called.
	inline HABITABILITY GenHab() const
	{
		return m_GenHab;
	}

	// This is only called by DaSector::FixSystemGenHabs when fixing old solar system created before the genHab was unchangeable.
	inline void SetGenHab(HABITABILITY genHab)
	{
		Q_ASSERT(m_GenHab == HABITABLE_Unspecified);	// Cannot change gen hab after it has been established.
		// Only these gen hab values are allowed.
		Q_ASSERT(genHab == HABITABLE_HomeWorld || genHab == HABITABLE_System || genHab == HABITABLE_EmptySystem);
		m_GenHab = genHab;
	}

	inline void SetFinal(bool isFinal)
	{
		m_Final = isFinal;
	}

	// Returns a seed value for generating this system that is based on its position in the galaxy. That way this system will regen
	// the same if it is removed and recreated due to decay or any other reason. We can't use the atom id because it will not be
	// the same when the system is regenerated.
	inline quint32 GenSeed() const
	{
		return m_GenSeed;
	}

	// Returns the force of gravity in ft/sec/sec projected by this atom on
	// another atom that is 'cgDistance' feet away.
	virtual double Gravity(double cgDistance) const;

	inline quint8 HighestQNonPirateShip() const
	{
		return m_HighestQShip;
	}

	// Returns star map info about this solar system, not its contents.
	void StarMapInfo(quint32 empireId, AuStarMapSystem& sm) const;

	// Repairs all positive wormholes in this solar system.
	// Preserves their names and explored status if possible.
	// Does not touch wormholes in other solar systems, leaving other-ends of wormholes to be cleaned up by their
	// own parent solar systems if they are left disconnected.
	// 'notifyDb' is passed to SetModified calls to signal updates to the database.
	// If 'notifyDb' is true, all systems in this sector and the 3x3x3 cube of neighboring sectors must be listed in 'systems'.
	// 'notifyDb' is false when fixing positive wormholes in starfield-only perimeter sectors, in preparation for fixing negative
	// wormholes in the sectors adjacent to those starfield-only sectors.
	void RepairPositiveWormholes(const DqAtomLocArray& systems, bool notifyDb);

	// Repairs all negative wormholes in this solar system.
	// Preserves their names and explored status if possible.
	// Does not touch wormholes in other solar systems, leaving other-ends of wormholes to be cleaned up by their
	// own parent solar systems if they are left disconnected.
	// Requires that all systems in this sector and the 3x3x3 cube of neighboring sectors be listed in 'systems'.
	void RepairNegativeWormholes(const DqAtomLocArray& systems, bool notifyDb);

	virtual bool Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari);

	// Returns true if this system fits the requirements of the 'search' criteria.
	// The range test and name matching parts of the search were already performed and they passed.
	// This system was created using PrepareForResourceScan to complete the search.
	bool StarMapSearch(const AuStarMapSystemSearch& search) const;

	virtual QString SystemScanReport(int scanEx) const;

	void UpdateTradeConnections();

	virtual unsigned int IsTocker() const;

	virtual void Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans);

	virtual DaAtom *Clone() const;

	// Returns true if this system will generate a ringworld if finalized.
	bool CouldHaveRingworld() const;

	void Create(HABITABILITY genHab, quint32 genSeed);

	void DecayContents();

	// Only valid on scene master. Only used to provide 'detected systems' in starfield information.
	void DetectedBy(quint32 empire, quint32 signalStrength);

	// Returns signal strength if emissions from this system were detected by 'empire' within the last 48 hours.
	// NOTE: Only valid on the scene master.
	inline quint32 DetectedSignalStrength(quint32 empire) const
	{
		//Q_ASSERT(g_Au.IsSceneMaster());
		float s;
		m_DetectedByLock.lockForRead();
		unordered_map<quint32, AuSysDet_>::const_iterator dbi = m_DetectedBy.find(empire);
		if(dbi != m_DetectedBy.end())
		{
			if((*dbi).second.m_When.ElapsedSec() < 60*60*48)	// Check for detection within the past 48 hours.
				s = (*dbi).second.m_SignalStrength;
			else
				s = 0;
		}
		else
			s = 0;
		m_DetectedByLock.unlock();
		return s;
	}

	// Does nothing if already explored by 'empire'.
	void ExploredBy(quint32 empire);

	void ScannedBy(quint32 empire, HABITABILITY genHab, quint32 genSeed);

	// Returns true if any units or cities are present that 'empireId' considers to be an enemy.
	bool IsEnemyEmpirePresent(quint32 empireId) const;

	// Returns true if any units or cities are present that 'empireId' considers to be an enemy.
	bool IsNeutralEmpirePresent(quint32 empireId) const;

	// Returns true if 'loc' is in the domain of this solar system atom.
	// false if 'loc' is more than a sector away.
	// false if CheckChildExit says no.
	bool IsInSystem(const AuPoint& loc) const;
	
	// A solar system with no cities is ready for decay one week after it was last visited by a human player or AI officer.
	bool IsReadyToDecay() const;

	inline bool IsExploredBy(quint32 empire) const
	{
		m_ExploredByLock.lockForRead();
		bool v = m_ExploredBy.contains(empire);
		m_ExploredByLock.unlock();
		return v;
	}

	inline bool IsScannedBy(quint32 empire) const
	{
		m_ScannedByLock.lockForRead();
		bool v = m_ScannedBy.contains(empire);
		m_ScannedByLock.unlock();
		return v;
	}

	inline AuTime LastAvatarAccountActive() const
	{
		m_LastAvatarAccountActiveLock.lock();
		AuTime v(m_LastAvatarAccountActive);
		m_LastAvatarAccountActiveLock.unlock();
		return v;
	}

	inline const QVector<quint32>& LockExploredBy() const
	{
		m_ExploredByLock.lockForRead();
		return m_ExploredBy;
	}

	inline void UnlockExploredBy() const
	{
		m_ExploredByLock.unlock();
	}

	inline const QSet<quint32>& LockScannedBy() const
	{
		m_ScannedByLock.lockForRead();
		return m_ScannedBy;
	}

	inline void UnlockScannedBy() const
	{
		m_ScannedByLock.unlock();
	}

	inline const AuWormholeArray& LockWormholes() const
	{
		m_WormholesLock.lockForRead();
		return m_Wormholes;
	}

	inline void UnlockWormholes() const
	{
		m_WormholesLock.unlock();
	}

	virtual quint64 MemoryUsage() const;

	// Returns blank for IsEmptySystem.
	virtual QString Name() const;

	inline QString NameKnownTo(quint32 empireId) const
	{
		if(!IsExploredBy(empireId))
			return CatalogName();
		return Name();
	}

	// Returns true if all the planets in this system that have cities returns true to OkToRename.
	// At least one planet in the system must have a city controlled by 'empireId'.
	// A system with no planets is always ok to rename.
	bool OkToRename(quint32 empireId) const;

	// Creates an initial pirate base in this system if possible.
	void PirateBaseEncounter();

	// 'victim' is the ship that triggered this encounter.
	// Returns:
	//  0=pirate encountered, base in system
	//  1=pirate encountered, base in sector, not in system
	//  1000+=probability of encounter (+1000)
	// -1=no pirate empires ready
	// -2=probability of encounter is 0 or less
	// -11=no pirate base in sector yet, encounter triggered
	// -12=pirate base not expected to be in this system but it is
	// -13=encounter location landed outside the system
	// -14=no pirate ship design
	int PirateEncounter(DaVessel* victim, bool agentTriggered);

	// Returns:
	//  0=pirate encountered, base in system
	//  1=pirate encountered, base in sector, not in system
	// -11=no pirate base in sector yet, encounter triggered
	// -12=pirate base not expected to be in this system but it is
	// -13=encounter location landed outside the system
	// -14=no pirate ship design
	int PirateEncounterBase(DaVessel* victim, bool agentTriggered);

	virtual void RxUpdate(QDataStream& fromServer, bool isFirstUpdate);

	// Returns the transformation matrix relative to this atom's parent solar system.
	virtual AuMatrix4x4 EnvTransform() const;

	virtual void TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx,
			const DaPerson& sceneAtom);
	virtual ATOM_TYPE AtomType() const;
	virtual QString Description() const;
	virtual DaAtom *FindClosestAtom(const QPoint &pxFind,
			const AuMatrix4x4 &tDevicePersp, const AuMatrix4x4 &tDeviceOrtho,
			const AuMatrix4x4 &tModel) const;

	virtual bool InStarField() const	// This atom is in the star field.
	{
		return true;
	}

	inline bool IsOnlyStarFieldLoaded() const
	{
		return m_isOnlyStarFieldLoaded;
	}

	inline AuTime LastOfficerVisited() const
	{
		m_LastOfficerVisitLock.lockForRead();
		AuTime v(m_LastOfficerVisit);
		m_LastOfficerVisitLock.unlock();
		return v;
	}

	inline AuTime LastPersonVisited() const
	{
		m_LastPersonVisitLock.lockForRead();
		AuTime v(m_LastPersonVisit);
		m_LastPersonVisitLock.unlock();
		return v;
	}

	void ProcessDemandTally(const AuItem& item,
			unsigned int& stock, unsigned int& population, unsigned int& needCount, unsigned int& produceCount) const;

	quint32 DemandCost(const AuItem& item) const;

	// Refreshes the last time an empire was present in this system.
	inline void SetEmpirePresent(quint32 empireId)
	{
		m_EmpiresPresentLock.lockForWrite();
		bool wasEmpty = m_EmpiresPresent.empty();
		unordered_map<quint32, AuElapsedTimer>::iterator ei = m_EmpiresPresent.find(empireId);
		if(ei == m_EmpiresPresent.end())
			m_EmpiresPresent.insert(pair<quint32, AuElapsedTimer>(empireId, AuElapsedTimer::Now()));
		else
			(*ei).second.restart();
		m_EmpiresPresentLock.unlock();
		if(wasEmpty)
			TickTock();
	}

	inline void SetLastAvatarAccountActive()
	{
		m_LastAvatarAccountActiveLock.lock();
		m_LastAvatarAccountActive.Set();
		m_LastAvatarAccountActiveLock.unlock();
		SetModified(NOTIFY_Database);
	}

	inline void SetLastFlagInfluence()
	{
		m_LastFlagInfluenceLock.lockForWrite();
		m_LastFlagInfluence.Set();
		m_LastFlagInfluenceLock.unlock();
		SetModified(NOTIFY_Database);
	}

	void SetLastOfficerVisited();

	void SetLastPersonScene();

	void SetLastPersonVisited();

	inline void SetLastPirateShip()
	{
		m_LastPirateLock.lockForWrite();
		m_LastPirateShip.restart();
		m_LastPirateLock.unlock();
	}

	inline void SetLastPirateStation()
	{
		m_LastPirateLock.lockForWrite();
		m_LastPirateStation.restart();
		m_LastPirateLock.unlock();
	}

	// 'shipQ' is used to push up the m_HighestQShip value, used to limit the quality of pirate encounters.
	inline void SetNonPirateShipPresent(quint8 shipQ)
	{
		Q_ASSERT(shipQ);
		if(shipQ > m_HighestQShip)
			m_HighestQShip = shipQ;
	}

	inline void SetStarFieldFlag(bool isOnlyStarField)
	{
		m_isOnlyStarFieldLoaded = isOnlyStarField;
	}

	void FinalizeContents();

	// Prepares a default constructed DaSolarSystem object for use in a client-side resource scan.
	// Set it up like this.
	//   AuEther tempEther;
	//   DaSolarSystem* sys = new DaSolarSystem;
	//   sys->SetId(someId);	// Id needed for AddAtom
	//   tempEther.AddAtom(sys);
	//   sys->PrepareForResourceScan
	// 
	// Atom ids in the result are bogus but they are unique among the resulting atoms.
	// This solar system atom must have no subatoms and it must not be a subatom when this function is called.
	// This function should not be called on a real solar system atom.
	// 'catalogName' is the catalog name of the solar sytem.
	// 'systemName' is the name of the solar sytem.
	// 'galaxy' is the GALAXY that contains this solar system.
	// 'genHab' is the HABITABILITY mode to use when calling DaStar::CreatePlanetsForScan. It is assigned to my m_GenHab variable.
	// 'genSeed' is assigned to my m_GenSeed variable.
	// 'isCoreBlackHole' is true if this solar system is the core solar system that contains the black hole.
	// 'systemLoc' is the Location() of this system relative to its parent sector
	// 'starLight' is this system's StarLight value. 
	void PrepareForResourceScan(const QString& catalogName, const QString& systemName, GALAXY galaxy, HABITABILITY genHab,
			quint32 genSeed, bool isCoreBlackHole, const AuMatrix4x4& systemT, float starLight);

	inline bool IsFinal() const		// True if contents creation is finished.
	{
		return m_Final;
	}

	// Used to verify this is the scanned system prepared with the passed arguments.
	bool IsSystem(const QString& systemName, GALAXY galaxy, HABITABILITY genHab, quint32 genSeed, bool isCoreBlackHole,
			const AuPoint& systemLoc, float starLight);

	// Returns a habitable planet within this solar system, if there is one. NULL if not.
	DaWorldLL* FindHabitableWorld(HABITABILITY hab) const;

	virtual bool CanRenderEnvironment() const
	{
		return true;
	}

	// Instance universe's prevent some top-down recursive searches from entering the instance.
	virtual bool IsInstanceUniverse() const
	{
		return false;
	}

	virtual void RenderEnvironment(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& model,
			AuAtomRenderInfo& ari, DaPerson* sceneAtom);

	// Returns true for atoms that persist unconditionally in the scene between scene syncs e.g. sectors, wormholes, etc.
	virtual bool IsPermanentInScene() const
	{
		// Once added to the scene, this atom is not removed between updates.
		// This way the server doesn't need to keep reminding us that this object is in the scene.
		return true;
	}

	// Wormholes.
	void AddWormholeConnection(const AuId &idSolarSystem, const AuPoint &vToThere, AuWormhole::POLARITY polarity);

	AuWormhole CheckWormholeEntry(const AuPoint& oldloc, const AuPoint& nuloc, float& entryAngle) const;

	// Returns the actual number of positive wormholes. Used by wormhole creation logic.
	inline int CountPositiveWormholes() const
	{
		int posWorms = 0;
		m_WormholesLock.lockForRead();
		for(AuWormholeArray::const_iterator it = m_Wormholes.constBegin(); it != m_Wormholes.constEnd(); it++)
			if((*it).Polarity() == AuWormhole::Positive)
				posWorms++;
		m_WormholesLock.unlock();
		return posWorms;
	}

	// Returns the number of positive wormholes this solar system is supposed to have.
	int PositiveWormholes() const;

	bool IsWormholeAt(const AuPoint& loc) const;

	void ListLocationAndWormholes(DqAtomLocArray &aSystems, const AuPoint &fOffset, AuSegmentArray& exploredWormholes,
			AuSegmentArray& knownWormholes, quint32 empire);

	AuWormhole FindClosestWormhole(const AuPoint& systemLoc) const;
	AuWormhole FindWormhole(const AuPoint& systemLoc) const;
	AuWormhole FindWormhole(const AuId &system, AuWormhole::POLARITY polarity) const;
	AuWormhole FindWormholeDir(const AuPoint &dir, AuWormhole::POLARITY polarity) const;
	QString WormholeName(const AuWormhole& wh) const;
	QString WormholeNameAt(const AuPoint& loc) const;

	virtual bool GetTransformRelativeToParent(AuMatrix4x4 &tModel,
			const DaAtom *pParent) const;

	inline bool HasName() const
	{
		// Returns true if a real name has been assigned, not just a number.
		m_NameLock.lockForRead();
		bool v = !m_sName.isEmpty() && m_sName != CatalogName();
		m_NameLock.unlock();
		return v;
	}

	// Returns true if a ringworld exists in this solar system.
	bool HasRingworld() const;

	inline QString HtmlNameWithSystem() const
	{
		return HtmlName(NameWithSystem());
	}

	virtual bool MoveSubatom(DaAtom* pMove, const AuPoint& vMove, int elapsedMs);

	// Returns a name for this solar system with the word 'System' in it.
	// The default name when a system is not named already includes the word System, e.g. System HGBSSD
	// The name assigned to a system almost never has the word System in it.
	inline QString NameWithSystem() const
	{
		if(HasName() && Name().indexOf("System") < 0)
			return Name() + " System";							// Add the 'System' tag.
		else
			return Name();										// Already includes the word System.
	}

	// Returns true if intergalactic wormholes are added.
	bool PositionWormholes();

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

	void SetNameIncludePlanets(const QString& sName, bool notifyDb);

	float StarLight() const;

	void WormholeExplored(const AuWormhole& wh, DaVessel* ship, bool isEmerging);

	bool WormholeExplored(const AuId& toSystem, AuWormhole::POLARITY polarity, quint32 empire, const QString& nuName);

protected:
	void AddFinalPrimary(DaStar* primary);

	// Finalizes contents of this solar system in preparation for generating resource scan data.
	// Atom ids in the result are bogus but they are unique among the resulting atoms.
	// This solar system atom must have no subatoms and it must not be a subatom when this function is called.
	// This function should not be called on a real solar system atom.
	// 'galaxy' is the GALAXY that contains this solar system.
	// 'genHab' is the HABITABILITY mode to use when calling DaStar::CreatePlanetsForScan. It is assigned to my m_GenHab variable.
	// 'genSeed' is assigned to my m_GenSeed variable.
	// 'isCoreBlackHole' is true if this solar system is the core solar system that contains the black hole.
	void CreateContentsForScan(GALAXY galaxy, HABITABILITY genHab, quint32 genSeed, bool isCoreBlackHole);

	DaWorldGlobe* CreateHomeWorld();

	DaStar* CreateStars() const;	// Returns the primary.

	// See CreateContentsForScan
	DaStar* CreateStarsForScan(GALAXY galaxy) const;

	// DepartedOnDeadHeading calls this for each individual atom. There can be more than one when using a tractor beam.
	bool DepartedOnDeadHeading(DaAtom* departAtom, const DaGalaxy* galaxy, const DaSector* ps, const DaSolarSystem* pcss);

	DaStar* HengeSun(quint8 sun);

	void RenderWormholes(const AuMatrix4x4& projection, const AuMatrix4x4& model, RENDERPASS pass, AuGLEffects& re,
			AuGLModelCache& mc, const AuModelMatrixHash& rmod);

	void SetupLights(DaPerson* sceneAtom, const AuMatrix4x4& model, AuGLEffects& re);

	virtual bool CheckChildExit(const AuPoint &foldloc, const AuPoint &fnuloc, const DaAtom* who) const;
	virtual bool CheckPeerJoin(const AuPoint &foldloc, const AuPoint &fnuloc, const DaAtom* who);
	virtual void ListSceneSubatoms(const DaPerson *pSceneAtom, const AuPoint& cameraEnvLoc, DaAtomList &sceneAtoms);

	// Returns false if a known component of the starfield is missing. i.e. a sector isn't loaded yet that we need.
	virtual bool ListStarFieldSubatoms(const DaAtom *pSceneAtom, DaAtomList& sceneAtoms) const;

public:
	static DaSolarSystem* LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, LOADSUBATOMS loadWhat,
			const AuId& idToLoad=0);
	virtual void Insert(QSqlDatabase& db) const;	// Insert first time database record.
	virtual void StoredInDb();
	virtual void Update(QSqlDatabase& db) const;	// Update existing database record.

	// Returns the destination galaxy given the 'fromGalaxy' and a direction in 'vecToDest'.
	// Returns GALAXY_Count if no match found.
	static GALAXY WormholeDestGalaxy(GALAXY fromGalaxy, const AuPoint& vecToDest);
};

#endif // DaSolarSystem_H
