//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtdb.h,v 1.98 2022/05/02 22:13:24 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DtDb_H
#define DtDb_H

#include "aucityreport.h"
#include "auelapsedtimer.h"
#include "auhash.h"
#include "auid.h"
#include <map>
#include <QList>
#include <QMutex>
#include <QReadWriteLock>
#include <QSemaphore>
#include <QThread>

class AuEther;
class DaUniverse;
class DgData;
class QSqlDatabase;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuCityReportRequest
{
public:
	quint8								m_ReportWhat;
	quint32								m_UnitEmpire;
	AuId								m_UnitId;

public:
	AuCityReportRequest(quint8 reportWhat, const AuId& unitId, quint32 unitEmpire)
	{
		m_ReportWhat = reportWhat;
		m_UnitEmpire = unitEmpire;
		m_UnitId = unitId;
	}

	AuCityReportRequest()
	{
		m_ReportWhat = 0;
		m_UnitEmpire = 0;
	}

	AuCityReportRequest(const AuCityReportRequest& c)
	{
		m_ReportWhat = c.m_ReportWhat;
		m_UnitEmpire = c.m_UnitEmpire;
		m_UnitId = c.m_UnitId;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSpawnAtom
{
public:
	AuId		m_Arena;				// Parent atom of arena person entered.
	quint32		m_EmpireId;				// Empire id of spawning person.
	quint8		m_Galaxy;				// GALAXY enum value
	AuId		m_Person;				// Id of spawning person.
	AuId		m_SpawnCity;			// Spawn city id of spawning person
	AuId		m_SpawnShip;			// Person's spawn ship id.

public:
	AuSpawnAtom()
	{
		m_EmpireId = 0;
		m_Galaxy = ShoresOfHazeron;
	}

	AuSpawnAtom(const AuId& person, quint32 empireId, const AuId& spawnCity, const AuId& spawnShip, const AuId& arena,
			GALAXY galaxy)
	{
		m_Arena = arena;
		m_EmpireId = empireId;
		m_Galaxy = galaxy;
		m_Person = person;
		m_SpawnCity = spawnCity;
		m_SpawnShip = spawnShip;
	}

	GALAXY Galaxy() const
	{
		return (GALAXY)m_Galaxy;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuHengeTeleportAtom
{
public:
	quint8		m_DestArc;
	quint8		m_DestSun;
	AuId		m_DestSysId;
	quint8		m_FromArc;
	quint8		m_FromSun;
	AuId		m_FromSysId;
	quint8		m_ShadowPanels;
	AuId		m_TeleportId;			// Atom teleporting.

public:
	AuHengeTeleportAtom()
	{
		m_DestArc = 0;
		m_DestSun = 0;
		m_FromArc = 0;
		m_FromSun = 0;
		m_ShadowPanels = 0;
	}

	AuHengeTeleportAtom(const AuId& teleportId, const AuId& fromSysId, quint8 fromSun, quint8 fromArc, quint8 shadowPanels,
			const AuId& destSysId, quint8 destSun, quint8 destArc)
	{
		m_DestArc = destArc;
		m_DestSun = destSun;
		m_DestSysId = destSysId;
		m_FromArc = fromArc;
		m_FromSun = fromSun;
		m_FromSysId = fromSysId;
		m_ShadowPanels = shadowPanels;
		m_TeleportId = teleportId;
	}

	void operator>>(QDataStream& out) const
	{
		out << m_DestArc;
		out << m_DestSun;
		out << m_DestSysId;
		out << m_FromArc;
		out << m_FromSun;
		out << m_FromSysId;
		out << m_ShadowPanels;
		out << m_TeleportId;
	}

	void operator<<(QDataStream& in)
	{
		in >> m_DestArc;
		in >> m_DestSun;
		in >> m_DestSysId;
		in >> m_FromArc;
		in >> m_FromSun;
		in >> m_FromSysId;
		in >> m_ShadowPanels;
		in >> m_TeleportId;
	}

	void GoBack(const AuHengeTeleportAtom& c)
	{
		m_DestArc = c.m_FromArc;
		m_DestSun = c.m_FromSun;
		m_DestSysId = c.m_FromSysId;
		m_FromArc = c.m_DestArc;
		m_FromSun = c.m_DestSun;
		m_FromSysId = c.m_DestSysId;
		m_ShadowPanels = c.m_ShadowPanels;	// This always matches at each end.
		m_TeleportId = c.m_TeleportId;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuHengeTeleportAtom& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuHengeTeleportAtom& o)
{
	o << in;
	return in;
}

// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.
Q_DECLARE_METATYPE(AuHengeTeleportAtom)

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuTravelAtom
{
public:
	AuPoint		m_DestGalaxyLoc;		// Galaxy location of destination for stargates.
	AuId		m_DestSector;			// Destination sector id that contains m_DestSystem. N/A for stargate.
	AuId		m_DestSystem;			// Destination system id of traveling atom. Dest galaxy id for stargates and galaxyLoc.
	AuId		m_OrigSystem;			// Origin system id of traveling atom. Only valid for departures. 0 for arrivals.
	AuId		m_Traveler;				// Id of traveling atom.
	int			m_TravelType;			// 0=regular traveller, 1=stargate, 2=galaxyLoc

public:	// Constructors
	inline AuTravelAtom()
	{
		m_TravelType = 0;
	}

	// Wormhole and deadhead constructor.
	inline AuTravelAtom(const AuId& traveler, const AuId& origSystem, const AuId& destSystem, const AuId& destSector)
	{
		m_DestSector = destSector;
		m_DestSystem = destSystem;
		m_OrigSystem = origSystem;
		m_Traveler = traveler;
		m_TravelType = 0;
	}

	// Stargate and GalaxyLoc
	inline AuTravelAtom(const AuId& traveler, const AuId& origSystem, const AuId& destGalaxy, const AuPoint& destGalaxyLoc,
			int travelType)
	{
		m_DestGalaxyLoc = destGalaxyLoc;
		m_DestSystem = destGalaxy;
		m_OrigSystem = origSystem;
		m_Traveler = traveler;
		m_TravelType = travelType;
	}

	inline AuTravelAtom(const AuTravelAtom& c)
	{
		m_DestGalaxyLoc = c.m_DestGalaxyLoc;
		m_DestSector = c.m_DestSector;
		m_DestSystem = c.m_DestSystem;
		m_OrigSystem = c.m_OrigSystem;
		m_Traveler = c.m_Traveler;
		m_TravelType = c.m_TravelType;
	}

public:	// Methods
	inline bool IsGalaxyLoc() const
	{
		return m_TravelType == 2;
	}

	inline bool IsStargate() const
	{
		return m_TravelType == 1;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class DtDb : public QThread
{
	Q_OBJECT;
public:
	enum DBSTATUS
	{
		NotLoaded,		// Database has not been loaded yet.
		Loaded,			// Database loaded ok.
		Storing,		// Database loaded ok and is currently storing.
		OpenError,		// Database did not open ok.
		DriverError,	// Database driver did not init ok.
	};

public:
	DtDb(QObject* parent=0);
	~DtDb();

	bool Alive() const
	{
		return !m_semAlive.available();
	}

	DBSTATUS DbStatus() const
	{
		return m_eDbStatus;
	}

	void Kill()
	{
		m_semAlive.release(1);
	}

	const AuElapsedTimer& LastSaved() const
	{
		return m_LastSave;
	}

	Qt::HANDLE ThreadId() const
	{
		return m_ThreadId;
	}

public slots:
	void CityReportRequested(AuId cityId, quint8 reportWhat, AuId unitId, quint32 unitEmpire);

	void InsertEmpire(quint32);
	void InsertSpecies(AuSpecies species);
	void RecoverAtomFromLimbo(AuId);
	void ReloadData();

	void SectorAssignmentRevoked(AuId sectorId);

	void HengeTeleportAtom(AuHengeTeleportAtom hengeTel);

	void HengeTeleportArrived(AuHengeTeleportAtom hengeTel);

	// 'galaxy' is a GALAXY enum value
	void SpawnAtom(AuId personOrShip, quint32 empireId, AuId spawnCity, AuId spawnShip, AuId arena, quint8 galaxy);

	void SystemAssignmentRevoked(AuId systemId);

	void TravelerArrived(AuId traveler, AuId destSystem, AuId destSector);

	// When traveling via intergalactic wormhole, 'destSystem' and 'destSector' are the destination galaxy id.
	void TravelerDeparted(AuId traveler, AuId origSystem, AuId destSystem, AuId destSector);

	void TravelerDepartedGalaxyLoc(AuId traveler, AuId origSystem, AuId destGalaxy, AuPoint destGalaxyLoc);

	void TravelerDepartedStargate(AuId traveler, AuId origSystem, AuId destGalaxy, AuPoint destGalaxyLoc);

	void UpdateEmpire(quint32);
	void UpdateSpecies(AuSpecies species);

signals:
	void AssignedSectorLoaded(AuId sectorId);
	void AssignedSolarSystemLoaded(AuId systemId);

	// 'galaxy' is a GALAXY enum value
	void AtomSpawned(AuId personOrShip, quint32 empireId, AuId spawnCity, AuId spawnShip, AuId arena, quint8 galaxy);

	void AtomHengeTeleported(AuHengeTeleportAtom hengeTel);

	void AtomTraveled(AuId traveler, AuId destSystem, AuId destSector);

	void AtomTraveledGalaxyLoc(AuId traveler, AuId destGalaxy, AuPoint destGalaxyLoc);

	void AtomTraveledStargate(AuId traveler, AuId destGalaxy, AuPoint destGalaxyLoc);

	void CityHistoryReportReady(AuId unitId, AuId cityId, AuCityReportHistory report);

	void CityLastReportReady(AuId unitId, AuId cityId, AuCityReport report, bool havePrevReport, AuCityReport prevReport);

	void EmpireRecordChanged(quint32 idEmpire);
	void EmpireRecordDeleted(quint32 idEmpire);
	void EmpireRecordLoaded(quint32 idEmpire);
	void FinishedInit();
	void HandoffElsewhere(AuId person, AuId dest, quint8 rideAlso, bool limboRescue, QByteArray atomData, AuId returnOnFail);
	void OfficerKilled(const AuId);
	void NewSectorStored(AuId idSector);
	void PersonDeleted(AuId personId);	// only emitted on master scene server

	void BdDesignDeactivated(quint32 designId);
	void ItDesignDeactivated(quint32 designId);
	void ScDesignDeactivated(quint32 designId);

	void SpeciesRecordChanged(AuSpecies species);
	void SpeciesRecordLoaded(AuSpecies species);
	void StoryRecordChanged(quint32 storyInstance);
	void StoryRecordDeleted(quint32 storyInstance);

protected:
	virtual void run();

private:
	void InitEmpireCities(DgData& gd, QSqlDatabase& db);
	void InitNextIds(QSqlDatabase& db);
	void LoadAssignedAtoms();
	void LoadChangedEmpires();
	void LoadChangedSpecies();
	void LoadChangedStories();
	DaUniverse* LoadGalaxies(QSqlDatabase& db, AuEther& ether);
	void LoadBdDesigns();
	void LoadBdExchange();
	void LoadItDesigns();
	void LoadScDesigns();
	void LoadScExchange();
	bool OpenDatabase(QSqlDatabase& db);
	void ProvideCityReports();
	void StoreAtoms(QSqlDatabase& db, AuIdList& sectorsAdded, QList<AuTravelAtom>& atomsDeparted, QList<AuSpawnAtom>& atomsSpawned,
			QList<AuHengeTeleportAtom>& atomsHengeTeleported);
	void StoreDatabase();
	void StoreEmpires(QSqlDatabase& db, QList<quint32>& empiresChanged);
	void StoreBdDesigns(QSqlDatabase& db, QList<quint32>& designsDeactivated);
	void StoreBdExchange(QSqlDatabase& db);
	void StoreBdRatings(QSqlDatabase& db);
	void StoreItDesigns(QSqlDatabase& db, QList<quint32>& designsDeactivated);
	void StoreScDesigns(QSqlDatabase& db, QList<quint32>& designsDeactivated);
	void StoreScExchange(QSqlDatabase& db);
	void StoreScRatings(QSqlDatabase& db);
	void StoreSpecies(QSqlDatabase& db, QList<AuSpecies>& speciesChanged);
	void StoreStories(QSqlDatabase& db, QList<quint32>& storiesChanged, QList<quint32>& storiesDeleted);

	// Reconcile/validate/update information in empire records.
	// This has grown to fix other stuff also.
	// This gets run once per day, at the day change in Z.
	void UpdateEmpires(DgData& gd, QSqlDatabase& db);

	void UpdateNextIds(QSqlDatabase& db);

private:
	DBSTATUS				m_eDbStatus;
	bool					m_FirstSave;	// true until first save is completed.
	AuElapsedTimer			m_LastSave;
	QSemaphore				m_semAlive;		// To know when I'm supposed to die.
	Qt::HANDLE				m_ThreadId;
	bool					m_bDatabaseUpdated;	// Used on Fridays to determine if the database has been updated

	// Only valid on scene master.
	AuId					m_DbNextAtomId;
	quint32					m_DbNextBuildingDesignId;
	quint32					m_DbNextEmpireId;
	quint32					m_DbNextItemDesignId;
	quint32					m_DbNextSpaceShipDesignId;
	quint32					m_DbNextStoryId;

private:
	// On scene worker, list of solar systems that I have fully loaded.
	// On scene manager, list of sectors that I have fully loaded.
	QMutex					m_LoadedLock;
	AuIdList				m_Loaded;

private:
	// Update notifications.
	mutable QMutex			m_muAtomTransactions;	// Guards m_ArrivingTravelers, m_DepartingTravelers, and m_SpawningAtoms.
	QList<AuHengeTeleportAtom>	m_ArrivingHengeTeleport;	// Atoms that are arriving to henge teleporters on this server.
	QList<AuTravelAtom>		m_ArrivingTravelers;	// Atoms that are arriving to this server.
	QList<AuHengeTeleportAtom>	m_DepartingHengeTeleport;	// Atoms that are departing from this server to henge teleporters.
	QList<AuTravelAtom>		m_DepartingTravelers;	// Atoms that are departing from this server.
	QList<AuSpawnAtom>		m_SpawningAtoms;		// Atoms that are departing from this server to spawn elsewhere.

private:
	QMutex					m_muAtomsInLimbo;		// Guards m_AtomsInLimbo.
	AuIdList				m_AtomsInLimbo;			// Atoms requested to be recovered from limbo. (Only on scene master).

private:
	mutable QMutex			m_muEmpireTransactions;	// Guards m_AddedEmpireIds and m_ChangedEmpireIds.
	QList<quint32>			m_AddedEmpireIds;		// Empires that have been added.
	QList<quint32>			m_ChangedEmpireIds;		// Empires that have been changed.

private:
	mutable QMutex			m_muSpecieTransactions;	// Guards m_AddedSpecies
	QList<AuSpecies>		m_AddedSpecies;			// Species that have been added.
	QList<AuSpecies>		m_ChangedSpecies;		// Species that have been changed.

private:
	mutable QMutex			m_muCityReportRequests;
	unordered_multimap<AuId, AuCityReportRequest, AuIdHash>	m_CityReportRequests;	// key is city id
};

#endif // DtDb_H
