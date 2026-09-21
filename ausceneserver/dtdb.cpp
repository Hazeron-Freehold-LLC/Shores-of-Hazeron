//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtdb.cpp,v 1.457 2022/05/02 22:16:10 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dtdb.h"
//
#include "auatommanager.h"
#include "aubddesign.h"
#include "aubddesignexchange.h"
#include "aubdreport.h"
#include "aucity.h"
#include "audbglobal.h"
#include "auitdesign.h"
#include "auscdesign.h"
#include "auscdesignexchange.h"
#include "ausceneserverapp.h"
#include "daanimal.h"
#include "daarena.h"
#include "daasteroid.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dacitizen.h"
#include "dadesignbase.h"
#include "dadesignplane.h"
#include "daeffect.h"
#include "dagalaxy.h"
#include "daitem.h"
#include "damissile.h"
#include "danpc.h"
#include "daperson.h"
#include "dasc.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dastar.h"
#include "dateleporter.h"
#include "dauniverse.h"
#include "davehicle.h"
#include "daworld.h"
#include <dgdata.h>
#include <qsettings.h>
#include <qsqldatabase.h>
#include <QSqlError>
#include <QSqlQuery>

// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
// The Q_DECLARE_METATYPE macro is also required.
static int s_AuHengeTeleportAtomMetaType = qRegisterMetaType<AuHengeTeleportAtom>();

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DtDb::DtDb(QObject* parent) : QThread(parent), m_semAlive(1)
{
	m_DbNextBuildingDesignId = 0;
	m_DbNextEmpireId = 0;
	m_DbNextItemDesignId = 0;
	m_DbNextSpaceShipDesignId = 0;
	m_DbNextStoryId = 0;
	m_eDbStatus = NotLoaded;
	m_FirstSave = true;
	m_semAlive.acquire(1);			// I live...
	m_ThreadId = 0;

	m_bDatabaseUpdated = false;
}

DtDb::~DtDb()
{
	//qDebug("DtDb terminated.");
}

bool DtDb::OpenDatabase(QSqlDatabase& db)
{
	//qDebug("DtDb::OpenDatabase");
	// auUniverse database
	//   Owner: aumaster
	//   Password: Light
	//   User: audog
	//   Password: Gold
	db.setDatabaseName("auUniverse");			// NOTE: Name is case sensitive.
	db.setHostName("UniverseDb.Hazeron.net");
	if(!db.open("audog","Gold"))
	{
		QSqlError er = db.lastError();
		QString sdv = er.driverText();
		QString sdb = er.databaseText();
		qWarning("Failed to open database connection:\n%s\n%s\n",
			(const char *)sdv.toLatin1(),
			(const char *)sdb.toLatin1());
		return false;
	}
	//qDebug("DtDb::OpenDatabase - ok");
	return true;
}

void DtDb::run()
{
	m_eDbStatus = NotLoaded;
	m_ThreadId = currentThreadId();
	m_bDatabaseUpdated = false;

	//qDebug("DtDb started.");

	// Start a scope in which the QSqlDatabase object can exist so that it
	// will be destructed before calling removeDatabase.
	{
		QSqlDatabase::addDatabase("QPSQL7", "AuLogin");	// Needed by UpdateEmpires to reconcile deleted person atoms.
		QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL7", "AuUniverse");
		if(db.isValid())
		{
			if(OpenDatabase(db))
			{
				// Load master game data.
				DgData& gd = g_Au.LockDataForWrite();
				gd.Load();
				if(g_Au.IsSceneMaster())
				{
					InitEmpireCities(gd, db);	// Tally cities, planets, systems, and sectors data for the empire records.
					UpdateEmpires(gd, db);		// Validate people data in empire records.
				}
				g_Au.UnlockData();

				// Load active stories.
				if(g_Au.IsSceneWorker())
				{
					AuStories* stories = g_AuDb.LockStoriesForWrite();
					stories->Load(db);
					g_AuDb.UnlockStories();
				}

				db.close();	// We'll open it when we need it. We don't keep it open.

				// Interval on which database updates are posted.
				int secsInterval;		// seconds between database saves
				if(g_Au.IsSceneMaster())
				{
					// Scene Master posts updates often. There is usually a Scene 
					// Manager waiting for the records to appear in the database.
					// There's only one Scene Master in the network so it can be a
					// hog.
					// Updates are primarily sectors, solar systems, and wormholes
					// as they are created. Updates also include wormhole names as
					// they are explored for the first time, which aren't that
					// crucial.
					secsInterval = 1;
				}
				else if(g_Au.IsSceneManager())
				{
					// Scene Manager posts updates frequently. There is usually a
					// Scene Worker waiting for the records to appear in the db.
					// There are relatively few Scene Managers in the network.
					// Updates are primarily the contents of solar systems as they
					// are created.
					secsInterval = 3;
				}
				else
				{
					// Scene Worker posts updates infrequently. Nobody is waiting 
					// for these updates. Changes that affect the universe outside
					// the solar systems managed by this server get propagated
					// upward to higher level servers and saved by them.
					// If the save interval is too frequent, the sql server will be
					// deluged by update queries from a potentially large number of
					// Scene Workers.
					// If the save interfal is too infrequent, the updates will be
					// bigger and more demanding on the scene worker.
#if !defined(QT_NO_DEBUG)
					secsInterval = 5 * 60;		// Shorter save interval for testing.
#else
					secsInterval = 24* 60 * 60;	// Once per day might be enough.
#endif
				}
				//qDebug("DtDb::run secsInterval is %d secs", secsInterval);

				bool loadPublished = true;	// Until we load the published designs.
				m_LastSave.restart();
				AuElapsedTimer lastTime;
				while(Alive())
				{
					// Poop in the log so I know when the last time this thread stopped responding.
					if(g_Au.m_LogMessages & LOG_Database)
					{
						if(!lastTime.isValid())
							lastTime.start();
						else if(lastTime.elapsed()/d_MSecPerMin >= 1)
						{
							g_Au.Log("Database Thread Alive");
							lastTime.restart();
						}
					}

					// NOTE: Saving changes must precede loading functions. Saving changes may involve departing atoms and the final
					//       save of revoked solar systems. This must happen before we load assigned systems or there can be overlap
					//       between departing data and arriving data when a revoked system is immediately reassigned to me or a
					//       departing atom is moving to a system that has now been assigned to me.

					// Save the database every now and again.
					// We also monitor for critical updates, such as departing atoms.
					// This requires an immediate save of our db data so the next server can load the proper data.
					// During DtDb::StoreAtoms we monitor for atom departures and notify the scene manager so the handoff can be
					// performed.
					if(m_eDbStatus == Loaded)
					{
						if(g_Au.IsSceneMaster())
						{
							// The scene master is responsible for updating empire records. Once per day the master calls
							// UpdateEmpires to get a fresh tally of reputation. This also does some verification of
							// data that could potentially get out of sync.
							if(m_LastSave.ElapsedDayTicks() > 0)
							{
								if(OpenDatabase(db))
								{
									DgData& gd = g_Au.LockDataForWrite();
									UpdateEmpires(gd, db);
									g_Au.UnlockData();
									db.close();	// We'll open it when we need it. We don't keep it open.
								}
							}
						}

						g_AuDb.m_muAtomTransactions.lock();
						bool hasCrit = g_AuDb.m_HasCriticalUpdates;
						g_AuDb.m_HasCriticalUpdates = false;
						g_AuDb.m_muAtomTransactions.unlock();
						if(hasCrit || m_LastSave.ElapsedSec() > secsInterval)
						{
							StoreDatabase();
							m_LastSave.restart();	// Time DONE.
						}
					}

					// Attempt to load any empire records that have changed.
					// NOTE: This is longer be needed because changed empire records are sent to the subs over the network.
					//LoadChangedEmpires();

					// Attempt to load any specie records that have changed.
					// NOTE: This is longer be needed because changed specie records are sent to the subs over the network.
					//LoadChangedSpecies();

					// Attempt to load any story records that have changed.
					LoadChangedStories();

					// Attempt to load any spacecraft designs that are needed.
					if(g_Au.IsSceneWorker())
					{
						if(loadPublished)
						{
							// Load all published designs. This is only done once, when we first wake up.
							LoadScExchange();
							LoadBdExchange();
							loadPublished = false;
						}
						LoadScDesigns();
						LoadItDesigns();
						LoadBdDesigns();
					}

					// Attempt to load any atoms that have been assigned to me.
					LoadAssignedAtoms();

					ProvideCityReports();

					msleep(1000);
				}

				// Post all remaining updates before we exit.
				if(m_eDbStatus == Loaded)
					StoreDatabase();
				m_eDbStatus = NotLoaded;
			}
			else
			{
				m_eDbStatus = OpenError;
			}
		}
		else
		{
			/*
			QStringList list = QSqlDatabase::drivers();
			QStringList::Iterator it = list.begin();
			while( it != list.end() )
			{
				qDebug("DtDb::driver - %s", (const char*)(*it));
				++it;
			}
			*/
			m_eDbStatus = DriverError;
		}
	}

	// Remove the database.
	// This must be done after the QSqlDatabase object has gone out of scope.
	// Refer to QSqlDatabase::removeDatabase documentation.
	QSqlDatabase::removeDatabase("AuUniverse");
	QSqlDatabase::removeDatabase("AuLogin");

	//qDebug("DtDb::run - finished");
}

void DtDb::StoreDatabase()
{
	QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
	if(!OpenDatabase(db))
		return;	// Failed to open database?

	// Lists of things that need to be signaled after the changes are committed.
	QList<quint32> empiresChanged;
	QList<AuSpecies> speciesChanged;
	QList<quint32> bdDesignsDeactivated;
	QList<quint32> itDesignsDeactivated;
	QList<quint32> scDesignsDeactivated;
	AuIdList sectorsAdded;
	QList<quint32> storiesChanged;
	QList<quint32> storiesDeleted;
	QList<AuTravelAtom> atomsDeparted;
	QList<AuSpawnAtom> atomsSpawned;
	QList<AuHengeTeleportAtom> atomsHengeTeleported;

	// Begin an SQL batch.
	// PostgreSQL documentation says this insures that a failure during this save (such as a server crash) will not result in
	// partially saved data. The entire batch must be committed or none of it gets saved.
	// This also insures that none of the data becomes visible until all of the data in the batch is saved.
	if(!db.transaction())
	{
		QSqlError err = db.lastError();
		qWarning("DtDb::StoreDatabase BEGIN TRANSACTION FAILED. '%s'", (const char*)err.text().toLatin1());
	}

	// Make the changes.
	UpdateNextIds(db);									// Next db Id values (on scene master anyway).
	if(g_Au.IsSceneWorker())
	{
		StoreScDesigns(db, scDesignsDeactivated);
		StoreScExchange(db);
		StoreScRatings(db);
		StoreItDesigns(db, itDesignsDeactivated);
		StoreBdDesigns(db, bdDesignsDeactivated);
		StoreBdExchange(db);
		StoreBdRatings(db);
	}
	StoreEmpires(db, empiresChanged);					// Insert/Update empires.
	StoreSpecies(db, speciesChanged);					// Insert/Update species.
	StoreStories(db, storiesChanged, storiesDeleted);	// Insert/Update stories.
	StoreAtoms(db, sectorsAdded, atomsDeparted, atomsSpawned, atomsHengeTeleported);		// Insert/Update/Delete atoms.

	// Commit our SQL batch.
	if(!db.commit())
	{
		QSqlError err = db.lastError();
		qWarning("DtDb::StoreDatabase COMMIT FAILED. '%s'", (const char*)err.text().toLatin1());
	}

	db.close();

	// Notify subordinate servers that a story was added/changed. It doesn't matter which, the server needs to load the current
	// story record.
	for(QList<quint32>::const_iterator it = storiesChanged.cbegin(); it != storiesChanged.cend(); it++)
		emit StoryRecordChanged(*it);
	for(QList<quint32>::const_iterator it = storiesDeleted.cbegin(); it != storiesDeleted.cend(); it++)
		emit StoryRecordDeleted(*it);

	// Notify app of any nu sectors that have been stored. In some cases, these sectors need to get loaded by a manager server
	// before the remaining notifications are relevant.
	for(AuIdList::const_iterator it = sectorsAdded.cbegin(); it != sectorsAdded.cend(); it++)
		emit NewSectorStored(*it);

	// Notify subordinate servers that an empire was added/changed. It doesn't matter which, the server needs to load the current
	// empire record. This is done after the db is closed in hopes the sql server finishes the commit before the sub servers get
	// the notification and attempt to load the record.
	for(QList<quint32>::const_iterator it = empiresChanged.cbegin(); it != empiresChanged.cend(); it++)
		emit EmpireRecordChanged(*it);

	// Notify subordinate servers that a specie was added/changed. It doesn't matter which, the server needs to load the current
	// specie record. This is done after the db is closed in hopes the sql server finishes the commit before the sub servers get
	// the notification and attempt to load the record.
	for(QList<AuSpecies>::const_iterator it = speciesChanged.cbegin(); it != speciesChanged.cend(); it++)
		emit SpeciesRecordChanged(*it);

	// Notify manager server of atoms that have departed our sphere of influence due to travel.
	for(QList<AuTravelAtom>::const_iterator it = atomsDeparted.cbegin(); it != atomsDeparted.cend(); it++)
	{
		//qDebug("DtDb::StoreDatabase signaling departure of traveler %d destSystem %d destSector %d",
				//(*it).m_Traveler.GetID(), (*it).m_DestSystem.GetID(), (*it).m_DestSector.GetID());
		if((*it).IsStargate())
		{
			// When traveling via stargate, 'destSystem' is the galaxy id.
			emit AtomTraveledStargate((*it).m_Traveler, (*it).m_DestSystem, (*it).m_DestGalaxyLoc);
		}
		else if((*it).IsGalaxyLoc())
		{
			// When traveling to a galaxy loc, 'destSystem' is the galaxy id.
			emit AtomTraveledGalaxyLoc((*it).m_Traveler, (*it).m_DestSystem, (*it).m_DestGalaxyLoc);
		}
		else
		{
			// When traveling via intergalactic wormhole, 'destSystem' and 'destSector' are the destination galaxy id.
			emit AtomTraveled((*it).m_Traveler, (*it).m_DestSystem, (*it).m_DestSector);
		}
	}

	// Notify manager server of atoms that have departed our sphere of influence due to spawn.
	for(QList<AuSpawnAtom>::const_iterator it = atomsSpawned.cbegin(); it != atomsSpawned.cend(); it++)
		emit AtomSpawned((*it).m_Person, (*it).m_EmpireId, (*it).m_SpawnCity, (*it).m_SpawnShip, (*it).m_Arena, (*it).Galaxy());

	// Notify manager server of atoms that have departed our sphere of influence due to spawn.
	for(QList<AuHengeTeleportAtom>::const_iterator it = atomsHengeTeleported.cbegin(); it != atomsHengeTeleported.cend(); it++)
		emit AtomHengeTeleported(*it);

	// Notify other servers of sc designs that appear to have been deactivated.
	if(!scDesignsDeactivated.isEmpty())
	{
		for(QList<quint32>::const_iterator dp = scDesignsDeactivated.cbegin(); dp != scDesignsDeactivated.cend(); dp++)
			emit ScDesignDeactivated(*dp);

		// Leave a log entry.
		if(g_Au.m_LogMessages & LOG_SpacecraftDesign)
		{
			QString logmsg;
			if(scDesignsDeactivated.count() == 1)
				logmsg = tr("Update Sc Designs: message sent for 1 design that was deactivated");
			else
				logmsg = tr("Update Sc Designs: messages sent for %1 designs that were deactivated")
					.arg(scDesignsDeactivated.count());
			g_Au.Log(logmsg);
		}
	}
	if(!itDesignsDeactivated.isEmpty())
	{
		for(QList<quint32>::const_iterator dp = itDesignsDeactivated.cbegin(); dp != itDesignsDeactivated.cend(); dp++)
			emit ItDesignDeactivated(*dp);

		// Leave a log entry.
		if(g_Au.m_LogMessages & LOG_SpacecraftDesign)
		{
			QString logmsg;
			if(itDesignsDeactivated.count() == 1)
				logmsg = tr("Update It Designs: message sent for 1 design that was deactivated");
			else
				logmsg = tr("Update It Designs: messages sent for %1 designs that were deactivated")
					.arg(itDesignsDeactivated.count());
			g_Au.Log(logmsg);
		}
	}
	if(!bdDesignsDeactivated.isEmpty())
	{
		for(QList<quint32>::const_iterator dp = bdDesignsDeactivated.cbegin(); dp != bdDesignsDeactivated.cend(); dp++)
			emit BdDesignDeactivated(*dp);

		// Leave a log entry.
		if(g_Au.m_LogMessages & LOG_SpacecraftDesign)
		{
			QString logmsg;
			if(bdDesignsDeactivated.count() == 1)
				logmsg = tr("Update Bd Designs: message sent for 1 design that was deactivated");
			else
				logmsg = tr("Update Bd Designs: messages sent for %1 designs that were deactivated")
					.arg(bdDesignsDeactivated.count());
			g_Au.Log(logmsg);
		}
	}

	// Save complete. Next save is not the first one anymore.
	m_FirstSave = false;
}

void DtDb::LoadScDesigns()
{
	g_AuDb.LockScDesignsNeeded();
	QList<quint32> designsNeeded(g_AuDb.m_DesignsNeededSc);
	g_AuDb.m_DesignsNeededSc.clear();
	QList<AuId> archDesignsNeeded(g_AuDb.m_ArchDesignsNeededSc);
	g_AuDb.m_ArchDesignsNeededSc.clear();
	g_AuDb.UnlockScDesignsNeeded();

	// Insure the designs used by pirates are loaded.
	const DgData& gd = g_Au.LockDataForRead();
	gd.PirateScBlueprints(designsNeeded);
	g_Au.UnlockData();

	if(designsNeeded.isEmpty() && archDesignsNeeded.isEmpty())
		return;	// Nothing to load.

	// Open the database before we lock the mutex.
	QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
	if(!OpenDatabase(db))
		return;	// Failed to open database?

	// Load the spacecraft designs into local data before locking any mutexes.
	AuScDesignHash designs;

	// Load spacecraft designs by specified architects.
	QList<quint32> justLoaded;
	for(QList<AuId>::const_iterator it = archDesignsNeeded.cbegin(); it != archDesignsNeeded.cend(); it++)
	{
		// Load all the spaceship design records created by the architect from the database.
		AuScDesignHash archdesigns;
		archdesigns.LoadArchitect(*it);
		for(AuScDesignHash::const_iterator dit = archdesigns.cbegin(); dit != archdesigns.cend(); dit++)
			justLoaded.append((*dit).second.Id());

		// Collect designs that were loaded.
		AuScDesignHash::iterator desit = archdesigns.begin();
		while(desit != archdesigns.end())
		{
			designs.insert(pair<quint32, AuScDesign>((*desit).first, (*desit).second));
			desit = archdesigns.erase(desit);
		}
	}

	// Load spacecraft designs that were specified by id.
	for(QList<quint32>::const_iterator it = designsNeeded.cbegin(); it != designsNeeded.cend(); it++)
	{
		// These are only requested in DaSc::LoadSubatoms and DaSc to insure the design is loaded.
		if(justLoaded.contains(*it))
			continue;	// We just loaded the design into our temporary data so we don't need to load it again right now.

		// We can avoid repeatedly reloading ship designs here because we know that if the design is loaded at all, it's ok.
		const DgData& sdd = g_Au.LockScDesignsForRead();
		bool alreadyLoaded = sdd.ScDesignIsLoaded(*it);
		g_Au.UnlockScDesigns();
		if(alreadyLoaded)
			continue;	// The design is already in the global data.

		// Load the individual design.
		designs.Load(*it);
	}

	db.close();
	if(designs.empty())
		return;

	if(g_Au.m_LogMessages & LOG_Database)
	{
		QString logmsg;
		logmsg = tr("Load Sc Designs: Need %1 individual designs and all designs by %2 people. Loading %3 designs.")
				.arg(designsNeeded.count()).arg(archDesignsNeeded.count()).arg(designs.size());
		g_Au.Log(logmsg);
	}

	// Now the designs are all loaded. Add/replace them to the global data en-masse to minimize the mutex lock time.
	DgData& sdd = g_Au.LockScDesignsForWrite();
	sdd.ScDesignAddDesigns(designs);							// Replaces existing design with the same id.
	g_Au.UnlockScDesigns();

	if(g_Au.m_LogMessages & LOG_Database)
		g_Au.Log(tr("Load Sc Designs: finished loading %1 designs").arg(designs.size()));
}

void DtDb::LoadItDesigns()
{
	g_AuDb.LockItDesignsNeeded();
	QList<quint32> designsNeeded(g_AuDb.m_DesignsNeededIt);
	g_AuDb.m_DesignsNeededIt.clear();
	QList<AuId> archDesignsNeeded(g_AuDb.m_ArchDesignsNeededIt);
	g_AuDb.m_ArchDesignsNeededIt.clear();
	g_AuDb.UnlockItDesignsNeeded();

	if(designsNeeded.isEmpty() && archDesignsNeeded.isEmpty())
		return;	// Nothing to load.

	// Open the database before we lock the mutex.
	QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
	if(!OpenDatabase(db))
		return;	// Failed to open database?

	if(g_Au.m_LogMessages & LOG_Database)
	{
		QString logmsg;
		logmsg = tr("Load It Designs: %1 individual designs, and all designs by %3 people")
				.arg(designsNeeded.count()).arg(archDesignsNeeded.count());
		g_Au.Log(logmsg);
	}

	// Load the spacecraft designs into local data before locking any mutexes.
	AuItDesignHash designs;

	// Load spacecraft designs by specified architects.
	QList<quint32> justLoaded;
	for(QList<AuId>::const_iterator it = archDesignsNeeded.cbegin(); it != archDesignsNeeded.cend(); it++)
	{
		// Load all the spaceship design records created by the architect from the database.
		AuItDesignHash archdesigns;
		archdesigns.LoadArchitect(*it);
		for(AuItDesignHash::const_iterator dit = archdesigns.cbegin(); dit != archdesigns.cend(); dit++)
			justLoaded.append((*dit).second.Id());

		// Collect designs that were loaded.
		AuItDesignHash::iterator desit = archdesigns.begin();
		while(desit != archdesigns.end())
		{
			designs.insert(pair<quint32, AuItDesign>((*desit).first, (*desit).second));
			desit = archdesigns.erase(desit);
		}
	}

	// Load spacecraft designs that were specified by id.
	for(QList<quint32>::const_iterator it = designsNeeded.cbegin(); it != designsNeeded.cend(); it++)
	{
		// These are only requested in DaItem to insure the design is loaded.
		if(justLoaded.contains(*it))
			continue;	// We just loaded the design into our temporary data so we don't need to load it again right now.

		// We can avoid repeatedly reloading ship designs here because we know that if the design is loaded at all, it's ok.
		const DgData& sdd = g_Au.LockItDesignsForRead();
		bool alreadyLoaded = sdd.ItDesignIsLoaded(*it);
		g_Au.UnlockItDesigns();
		if(alreadyLoaded)
			continue;	// The design is already in the global data.

		// Load the individual design.
		designs.Load(*it);
	}

	db.close();

	// Now the designs are all loaded. Add/replace them to the global data en-masse to minimize the mutex lock time.
	DgData& sdd = g_Au.LockItDesignsForWrite();
	sdd.ItDesignAddDesigns(designs);							// Replaces existing design with the same id.
	g_Au.UnlockItDesigns();

	if(g_Au.m_LogMessages & LOG_Database)
		g_Au.Log("Load It Designs: finished");
}

void DtDb::LoadBdDesigns()
{
	g_AuDb.LockBdDesignsNeeded();
	QList<quint32> designsNeeded(g_AuDb.m_DesignsNeededBd);
	g_AuDb.m_DesignsNeededBd.clear();
	QList<AuId> archDesignsNeeded(g_AuDb.m_ArchDesignsNeededBd);
	g_AuDb.m_ArchDesignsNeededBd.clear();
	g_AuDb.UnlockBdDesignsNeeded();

	// Insure the designs used by pirates are loaded.
	const DgData& gd = g_Au.LockDataForRead();
	gd.PirateBdBlueprints(designsNeeded);
	g_Au.UnlockData();

	if(designsNeeded.isEmpty() && archDesignsNeeded.isEmpty())
		return;	// Nothing to load.

	// Open the database before we lock the mutex.
	QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
	if(!OpenDatabase(db))
		return;	// Failed to open database?

	// Load the spacecraft designs into local data before locking any mutexes.
	AuBdDesignHash designs;

	// Load spacecraft designs by specified architects.
	QList<quint32> justLoaded;
	for(QList<AuId>::const_iterator it = archDesignsNeeded.cbegin(); it != archDesignsNeeded.cend(); it++)
	{
		// Load all the spaceship design records created by the architect from the database.
		AuBdDesignHash archdesigns;
		archdesigns.LoadArchitect(*it);
		for(AuBdDesignHash::const_iterator dit = archdesigns.cbegin(); dit != archdesigns.cend(); dit++)
			justLoaded.append((*dit).second.Id());

		// Collect designs that were loaded.
		AuBdDesignHash::iterator desit = archdesigns.begin();
		while(desit != archdesigns.end())
		{
			designs.insert(pair<quint32, AuBdDesign>((*desit).first, (*desit).second));
			desit = archdesigns.erase(desit);
		}
	}

	// Load bd designs that were specified by id.
	for(QList<quint32>::const_iterator it = designsNeeded.cbegin(); it != designsNeeded.cend(); it++)
	{
		// These are only requested in DaBd::LoadSubatoms and DaBd to insure the design is loaded.
		if(justLoaded.contains(*it))
			continue;	// We just loaded the design into our temporary data so we don't need to load it again right now.

		// We can avoid repeatedly reloading ship designs here because we know that if the design is loaded at all, it's ok.
		const DgData& sdd = g_Au.LockBdDesignsForRead();
		bool alreadyLoaded = sdd.BdDesignIsLoaded(*it);
		g_Au.UnlockBdDesigns();
		if(alreadyLoaded)
			continue;	// The design is already in the global data.

		// Load the individual design.
		designs.Load(*it);
	}

	db.close();
	if(designs.empty())
		return;

	if(g_Au.m_LogMessages & LOG_Database)
	{
		QString logmsg;
		logmsg = tr("Load Bd Designs: Need %1 individual designs and all designs by %2 people. Loading %3 designs.")
				.arg(designsNeeded.count()).arg(archDesignsNeeded.count()).arg(designs.size());
		g_Au.Log(logmsg);
	}

	// Now the designs are all loaded. Add/replace them to the global data en-masse to minimize the mutex lock time.
	DgData& sdd = g_Au.LockBdDesignsForWrite();
	sdd.BdDesignAddDesigns(designs);							// Replaces existing design with the same id.
	g_Au.UnlockBdDesigns();

	if(g_Au.m_LogMessages & LOG_Database)
		g_Au.Log(tr("Load Sc Designs: finished loading %1 designs").arg(designs.size()));
}

void DtDb::LoadScExchange()
{
	// Now we can proceed with loading designs from the database.
	if(g_Au.m_LogMessages & LOG_Database)
	{
		QString logmsg;
		logmsg = tr("Load Sc Library: started");
		g_Au.Log(logmsg);
	}

	// Open the database before we lock the mutex.
	QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
	if(OpenDatabase(db))
	{
		DgData& scl = g_Au.LockScExchangeForWrite();
		scl.m_ScExchange->SqlSelectAll(db);

		// Load ratings for all designs in the exchange.
		QList<quint32> designIds = scl.m_ScExchange->keys();
		for(QList<quint32>::const_iterator di = designIds.cbegin(); di != designIds.cend(); di++)
			scl.m_ScExchangeRatings->SqlSelectSc(db, *di);

		g_Au.UnlockScExchange();
		db.close();
	}

	if(g_Au.m_LogMessages & LOG_Database)
		g_Au.Log("Load Sc Library: finished");
}

void DtDb::LoadBdExchange()
{
	// Now we can proceed with loading designs from the database.
	if(g_Au.m_LogMessages & LOG_Database)
	{
		QString logmsg;
		logmsg = tr("Load Bd Library: started");
		g_Au.Log(logmsg);
	}

	// Open the database before we lock the mutex.
	QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
	if(OpenDatabase(db))
	{
		DgData& scl = g_Au.LockBdExchangeForWrite();
		scl.m_BdExchange->SqlSelectAll(db);

		// Load ratings for all designs in the exchange.
		QList<quint32> designIds = scl.m_BdExchange->keys();
		for(QList<quint32>::const_iterator di = designIds.cbegin(); di != designIds.cend(); di++)
			scl.m_BdExchangeRatings->SqlSelectBd(db, *di);

		g_Au.UnlockBdExchange();
		db.close();
	}

	if(g_Au.m_LogMessages & LOG_Database)
		g_Au.Log("Load Bd Library: finished");
}

DaUniverse* DtDb::LoadGalaxies(QSqlDatabase& db, AuEther& ether)
{
	// All database accesses must be done from the same thread.
	Q_ASSERT(ThreadId() == QThread::currentThreadId());

	// Load the Universe from the database.
	DaUniverse::LoadSubatoms(ether, db, AuId(0));	// Universe but no subordinates.
	DaUniverse* universe = ether.FindUniverse();
	if(!universe)
	{
		if(g_Au.m_LogMessages & LOG_Error)
			qWarning("******************** NOTICE! ********************\n"
					 "    A Universe was not found in the database.\n"
					 "   A Universe and %d galaxies will be created.\n"
					 "*************************************************", GALAXY_Count);
		return 0;
	}

	// Load the galaxies from the database.
	DaGalaxy::LoadSubatoms(ether, db, universe->Id());					// Galaxies but no subordinates.

	// Validate the static galaxy ids in AuId. They could change if the database was restarted.
	for(int gal=0; gal<GALAXY_Count; gal++)
	{
		const DaGalaxy* galaxy = universe->FindGalaxy((GALAXY)gal);
		if(!galaxy)
		{
			qWarning() << "error: Missing galaxy" << GalaxyName((GALAXY)gal);
			continue;
		}

		if(!galaxy->Is(AuId::GalaxyId((GALAXY)gal)))
			qWarning() << "error: Table of galaxy atom ids is incorrect for galaxy" << galaxy->Id() << galaxy->Name();
	}

	return universe;
}

void DtDb::InitNextIds(QSqlDatabase& db)
{
	//qDebug("DtDb::InitNextIds");
	//Q_ASSERT(g_Au.IsSceneMaster());

	// Init next ids to default values.
	m_DbNextAtomId = AuId::FirstUnreserved();	// Leave room at the beginning for reserved atom ids. See auid.h.
	m_DbNextBuildingDesignId = 1000;			// Leave room at the beginning for publishing from the command line.
	m_DbNextEmpireId = 100;						// Leave room at the beginning for pirate empires, 15 of them.
	m_DbNextItemDesignId = 20000;				// Leave room at the beginning for publishing from the command line.
	m_DbNextSpaceShipDesignId = 1000;			// Leave room at the beginning for publishing from the command line.
	m_DbNextStoryId = 100;						// Leave room at the beginning for who knows what.

	// Attempt to load the next id values from the database.
	QString sq("SELECT atom,empire,spaceShipDesign,story,buildingDesign,itemDesign FROM g_NextIds;");
	QSqlQuery qnextids(db);
	qnextids.prepare(sq);
	if(qnextids.exec() && qnextids.first())
	{
		// We found a record of saved values.
		//qDebug("DtDb::InitNextIds - Record found.");
		int qv=0;
		m_DbNextAtomId = AuId(qnextids.value(qv++).toUInt());
		m_DbNextEmpireId = qnextids.value(qv++).toUInt();
		m_DbNextSpaceShipDesignId = qnextids.value(qv++).toUInt();
		m_DbNextStoryId = qnextids.value(qv++).toUInt();
		m_DbNextBuildingDesignId = qnextids.value(qv++).toUInt();
		m_DbNextItemDesignId = qnextids.value(qv++).toUInt();
	}
	else
	{
		// No record was loaded. Attempt to initialize.
		//qDebug("DtDb::InitNextIds - No Record found...init.");
		QString sq = QString("INSERT INTO g_NextIds (atom,empire,spaceShipDesign,story,buildingDesign,itemDesign) "
				"VALUES (%1,%2,%3,%4,%5,%6);")
			.arg(m_DbNextAtomId.GetID())
			.arg(m_DbNextEmpireId)
			.arg(m_DbNextSpaceShipDesignId)
			.arg(m_DbNextStoryId)
			.arg(m_DbNextBuildingDesignId)
			.arg(m_DbNextItemDesignId);
		QSqlQuery qinitnextids(db);
		qinitnextids.prepare(sq);
		if(!qinitnextids.exec())
		{
			// This is a critical error. We can't continue.
			qCritical("CRITICAL ERROR! Failed to initialize next ids.");
			auSceneServerApp->quit();
		}
	}

	// Update the global data object with our next id values.
	g_AuDb.InitNextIds(m_DbNextAtomId,
			m_DbNextEmpireId,
			m_DbNextSpaceShipDesignId,
			m_DbNextStoryId,
			m_DbNextBuildingDesignId,
			m_DbNextItemDesignId);
}

void DtDb::UpdateNextIds(QSqlDatabase& db)
{
	// Only the scene master does this.
	if(!g_Au.IsSceneMaster())
		return;

	// Get the next ids without incrementing them.
	const AuId& lastAtomId = g_AuDb.CurrentAtomId();
	quint32 lastEmpireId = g_AuDb.CurrentEmpireId();
	quint32 lastSpaceShipDesignId = g_AuDb.CurrentSpaceShipDesignId();
	quint32 lastStoryId = g_AuDb.CurrentStoryId();
	quint32 lastBuildingDesignId = g_AuDb.CurrentBuildingDesignId();
	quint32 lastItemDesignId = g_AuDb.CurrentItemDesignId();
	if(lastAtomId == m_DbNextAtomId &&
			lastEmpireId == m_DbNextEmpireId &&
			lastSpaceShipDesignId == m_DbNextSpaceShipDesignId &&
			lastStoryId == m_DbNextStoryId &&
			lastBuildingDesignId == m_DbNextBuildingDesignId &&
			lastItemDesignId == m_DbNextItemDesignId)
		return;	// No change.

	QString sq;
	sq = QString("UPDATE g_NextIds SET atom=%1, empire=%2, spaceShipDesign=%3, story=%4, buildingDesign=%5, itemDesign=%6;")
			.arg(lastAtomId.GetID()).arg(lastEmpireId).arg(lastSpaceShipDesignId).arg(lastStoryId).arg(lastBuildingDesignId)
			.arg(lastItemDesignId);

	QSqlQuery qsaveids(db);
	qsaveids.prepare(sq);
	if(qsaveids.exec())
	{
		// The database record was updated.
		m_DbNextAtomId = lastAtomId;
		m_DbNextBuildingDesignId = lastBuildingDesignId;
		m_DbNextEmpireId = lastEmpireId;
		m_DbNextItemDesignId = lastItemDesignId;
		m_DbNextSpaceShipDesignId = lastSpaceShipDesignId;
		m_DbNextStoryId = lastStoryId;
	}
	else
		qWarning("ERROR! Failed to update next ids.");
}

void DtDb::StoreScDesigns(QSqlDatabase& db, QList<quint32>& designsDeactivated)
{
	// Test the lists of added, changed, and deleted design records.
	g_AuDb.LockScDesignTransactions();
	if(g_AuDb.m_DesignTransactionsSc.isEmpty())
	{
		// No sc design database transactions.
		g_AuDb.UnlockScDesignTransactions();
		return;
	}

	// Grab a copy of the transaction list so we can release the mutex quickly.
	QList<AuDesignTransaction> transactions(g_AuDb.m_DesignTransactionsSc);
	g_AuDb.m_DesignTransactionsSc.clear();
	g_AuDb.m_RecentSc.clear();
	for(QList<AuDesignTransaction>::const_iterator ti = transactions.cbegin(); ti != transactions.cend(); ti++)
		g_AuDb.m_RecentSc.insert((*ti).m_idDesign);
	g_AuDb.UnlockScDesignTransactions();

	const DgData& sdd = g_Au.LockScDesignsForRead();

	if(g_Au.m_LogMessages & LOG_Database)
	{
		QString logmsg;
		logmsg = tr("Update Sc Designs %1 transactions").arg(transactions.count());
		g_Au.Log(logmsg);
	}

	// We grab copies of the affected records so we can release the mutex as quickly as
	// possible. Then we can take our time about saving the data, which takes a long
	// time.
	QList<AuScDesign> designs;
	for(QList<AuDesignTransaction>::const_iterator tit = transactions.cbegin(); tit != transactions.cend(); tit++)
	{
		switch((*tit).m_Action)
		{
		case D_Invalid:
		case D_DesignDeactivate:
			continue;

		case D_DesignAdd:
		case D_DesignChange:
			if(sdd.ScDesignIsLoaded((*tit).m_idDesign))
			{
				AuScDesign design;
				if(sdd.ScDesign((*tit).m_idDesign, design))
					designs.append(design);		// Found the design record.
			}
			break;
		case D_DesignDelete:
			continue;	// Don't need the record for deletes.
		}
	}

	// We have copies of the records we need so release the global data mutex.
	g_Au.UnlockScDesigns();

	// Perform the transactions in sequence using our local copies of the database records.
	// This stack of ints is used to accumulate add,change,delete statistics for the final log entry.
	int a=0;
	int c=0;
	int d=0;
	int del=0;
	designsDeactivated.clear();	// Unique list of read-only designs that have been deactivated so other servers can be notified.
	for(QList<AuDesignTransaction>::const_iterator tit = transactions.cbegin(); tit != transactions.cend(); tit++)
	{
		switch((*tit).m_Action)
		{
		case D_Invalid:
			continue;
		case D_DesignDeactivate:
			if(!designsDeactivated.contains((*tit).m_idDesign))
			{
				designsDeactivated.append((*tit).m_idDesign);
				d++;
			}
			break;
		case D_DesignAdd:
			for(QList<AuScDesign>::const_iterator it = designs.cbegin(); it != designs.cend(); it++)
				if((*it).Id() == (*tit).m_idDesign)
				{
					// Insert the added record into the database.
					(*it).Insert(db);
					a++;
					break;
				}
			break;
		case D_DesignChange:
			for(QList<AuScDesign>::const_iterator it = designs.cbegin(); it != designs.cend(); it++)
				if((*it).Id() == (*tit).m_idDesign)
				{
					// Update the changed record into the database.
					(*it).Update(db);
					c++;
					break;
				}
			break;
		case D_DesignDelete:
			{
				QString sq = QString("DELETE FROM n_ScDesign WHERE id=%1;").arg((*tit).m_idDesign);

				QSqlQuery qdel(db);
				qdel.prepare(sq);
				if(!qdel.exec())
					qWarning() << "DtDb::StoreScDesigns - DELETE FAILED" << sq;
				del++;
			}
			break;
		}
	}

	int tot = a + c + d + del;
	if(tot != transactions.count())
	{
		if(g_Au.m_LogMessages & LOG_Error)
		{
			QString logmsg = tr("error: Update Sc Designs: a%1 c%2 d%3 del%4 total %5 != %6 transactions")
				.arg(a)
				.arg(c)
				.arg(d)
				.arg(del)
				.arg(tot)
				.arg(transactions.count());
			g_Au.Error(logmsg);
		}
	}
	else
	{
		if(g_Au.m_LogMessages & LOG_Database)
		{
			QString logmsg = tr("Update Sc Designs: a%1 c%2 d%3 v%4")
				.arg(a)
				.arg(c)
				.arg(d)
				.arg(del);
			g_Au.Log(logmsg);
		}
	}
}

void DtDb::StoreScExchange(QSqlDatabase& db)
{
	// Test the lists of added, changed, and deleted design records.
	g_AuDb.LockScExchangeTransactions();
	if(g_AuDb.m_ExchangeTransactionsSc.isEmpty())
	{
		// No sc design database transactions.
		g_AuDb.UnlockScExchangeTransactions();
		return;
	}

	// Grab a copy of the transaction list so we can release the mutex quickly.
	QList<AuDesignTransaction> transactions(g_AuDb.m_ExchangeTransactionsSc);
	g_AuDb.m_ExchangeTransactionsSc.clear();
	g_AuDb.UnlockScExchangeTransactions();

	const DgData& sdd = g_Au.LockScExchangeForRead();

	if(g_Au.m_LogMessages & LOG_Database)
	{
		QString logmsg;
		logmsg = tr("Update Sc Exchange %1 transactions").arg(transactions.count());
		g_Au.Log(logmsg);
	}

	// We grab copies of the affected records so we can release the mutex as quickly as
	// possible. Then we can take our time about saving the data, which takes a long
	// time.
	QList<AuScDesignExchangeEntry> exchange;
	for(QList<AuDesignTransaction>::const_iterator tit = transactions.cbegin(); tit != transactions.cend(); tit++)
	{
		switch((*tit).m_Action)
		{
		case D_Invalid:
		case D_DesignDeactivate:
			continue;

		case D_DesignAdd:
		case D_DesignChange:
			{
				AuScDesignExchange::const_iterator ei = sdd.m_ScExchange->find((*tit).m_idDesign);
				if(ei != sdd.m_ScExchange->end())
					exchange.append(*ei);
			}
			break;
		case D_DesignDelete:
			continue;	// Don't need the record for deletes.
		}
	}

	// We have copies of the records we need so release the global data mutex.
	g_Au.UnlockScExchange();

	// Perform the transactions in sequence using our local copies of the database records.
	// This stack of ints is used to accumulate add,change,delete statistics for the final log entry.
	int a=0;
	int c=0;
	int del=0;
	for(QList<AuDesignTransaction>::const_iterator tit = transactions.cbegin(); tit != transactions.cend(); tit++)
	{
		switch((*tit).m_Action)
		{
		case D_Invalid:
			continue;
		case D_DesignDeactivate:
			continue;
		case D_DesignAdd:
			for(QList<AuScDesignExchangeEntry>::const_iterator it = exchange.cbegin(); it != exchange.cend(); it++)
				if((*it).DesignId() == (*tit).m_idDesign)
				{
					// Insert the added record into the database.
					(*it).SqlInsert(db);
					a++;
					break;
				}
			break;
		case D_DesignChange:
			for(QList<AuScDesignExchangeEntry>::const_iterator it = exchange.cbegin(); it != exchange.cend(); it++)
				if((*it).DesignId() == (*tit).m_idDesign)
				{
					// Update the changed record into the database.
					(*it).SqlUpdate(db);
					c++;
					break;
				}
			break;
		case D_DesignDelete:
			{
				QString sq = QString("DELETE FROM n_ScExchange WHERE id=%1;").arg((*tit).m_idDesign);

				QSqlQuery qdel(db);
				qdel.prepare(sq);
				if(!qdel.exec())
					qWarning() << "DtDb::StoreScExchange - DELETE FAILED" << sq;
				del++;
			}
			break;
		}
	}

	int tot = a + c + del;
	if(tot != transactions.count())
	{
		if(g_Au.m_LogMessages & LOG_Error)
		{
			QString logmsg = tr("error: Update Sc Exchange: a%1 c%2 del%4 total %5 != %6 transactions")
				.arg(a)
				.arg(c)
				.arg(del)
				.arg(tot)
				.arg(transactions.count());
			g_Au.Error(logmsg);
		}
	}
	else
	{
		if(g_Au.m_LogMessages & LOG_Database)
		{
			QString logmsg = tr("Update Sc Exchange: a%1 c%2 v%4")
				.arg(a)
				.arg(c)
				.arg(del);
			g_Au.Log(logmsg);
		}
	}
}

void DtDb::StoreItDesigns(QSqlDatabase& db, QList<quint32>& designsDeactivated)
{
	// Test the lists of added, changed, and deleted design records.
	g_AuDb.LockItDesignTransactions();
	if(g_AuDb.m_DesignTransactionsIt.isEmpty())
	{
		// No sc design database transactions.
		g_AuDb.UnlockItDesignTransactions();
		return;
	}

	// Grab a copy of the transaction list so we can release the mutex quickly.
	QList<AuDesignTransaction> transactions(g_AuDb.m_DesignTransactionsIt);
	g_AuDb.m_DesignTransactionsIt.clear();
	g_AuDb.m_RecentIt.clear();
	for(QList<AuDesignTransaction>::const_iterator ti = transactions.cbegin(); ti != transactions.cend(); ti++)
		g_AuDb.m_RecentIt.insert((*ti).m_idDesign);
	g_AuDb.UnlockItDesignTransactions();

	const DgData& sdd = g_Au.LockItDesignsForRead();

	if(g_Au.m_LogMessages & LOG_Database)
	{
		QString logmsg;
		logmsg = tr("Update It Designs %1 transactions").arg(transactions.count());
		g_Au.Log(logmsg);
	}

	// We grab copies of the affected records so we can release the mutex as quickly as
	// possible. Then we can take our time about saving the data, which takes a long
	// time.
	QList<AuItDesign> designs;
	for(QList<AuDesignTransaction>::const_iterator tit = transactions.cbegin(); tit != transactions.cend(); tit++)
	{
		switch((*tit).m_Action)
		{
		case D_Invalid:
		case D_DesignDeactivate:
			continue;

		case D_DesignAdd:
		case D_DesignChange:
			if(sdd.ItDesignIsLoaded((*tit).m_idDesign))
			{
				AuItDesign design;
				if(sdd.ItDesign((*tit).m_idDesign, design))
					designs.append(design);		// Found the design record.
			}
			break;
		case D_DesignDelete:
			continue;	// Don't need the record for deletes.
		}
	}

	// We have copies of the records we need so release the global data mutex.
	g_Au.UnlockItDesigns();

	// Perform the transactions in sequence using our local copies of the database records.
	// This stack of ints is used to accumulate add,change,delete statistics for the final log entry.
	int a=0;
	int c=0;
	int d=0;
	int del=0;
	designsDeactivated.clear();	// Unique list of read-only designs that have been deactivated so other servers can be notified.
	for(QList<AuDesignTransaction>::const_iterator tit = transactions.cbegin(); tit != transactions.cend(); tit++)
	{
		switch((*tit).m_Action)
		{
		case D_Invalid:
			continue;
		case D_DesignDeactivate:
			if(!designsDeactivated.contains((*tit).m_idDesign))
			{
				designsDeactivated.append((*tit).m_idDesign);
				d++;
			}
			break;
		case D_DesignAdd:
			for(QList<AuItDesign>::const_iterator it = designs.cbegin(); it != designs.cend(); it++)
				if((*it).Id() == (*tit).m_idDesign)
				{
					// Insert the added record into the database.
					(*it).Insert(db);
					a++;
					break;
				}
			break;
		case D_DesignChange:
			for(QList<AuItDesign>::const_iterator it = designs.cbegin(); it != designs.cend(); it++)
				if((*it).Id() == (*tit).m_idDesign)
				{
					// Update the changed record into the database.
					(*it).Update(db);
					c++;
					break;
				}
			break;
		case D_DesignDelete:
			{
				QString sq = QString("DELETE FROM n_ItDesign WHERE id=%1;").arg((*tit).m_idDesign);

				QSqlQuery qdel(db);
				qdel.prepare(sq);
				if(!qdel.exec())
					qWarning() << "DtDb::StoreItDesigns - DELETE FAILED" << sq;
				del++;
			}
			break;
		}
	}

	int tot = a + c + d + del;
	if(tot != transactions.count())
	{
		if(g_Au.m_LogMessages & LOG_Error)
		{
			QString logmsg = tr("error: Update It Designs: a%1 c%2 d%3 del%4 total %5 != %6 transactions")
				.arg(a)
				.arg(c)
				.arg(d)
				.arg(del)
				.arg(tot)
				.arg(transactions.count());
			g_Au.Error(logmsg);
		}
	}
	else
	{
		if(g_Au.m_LogMessages & LOG_Database)
		{
			QString logmsg = tr("Update It Designs: a%1 c%2 d%3 v%4")
				.arg(a)
				.arg(c)
				.arg(d)
				.arg(del);
			g_Au.Log(logmsg);
		}
	}
}

void DtDb::StoreBdDesigns(QSqlDatabase& db, QList<quint32>& designsDeactivated)
{
	// Test the lists of added, changed, and deleted design records.
	g_AuDb.LockBdDesignTransactions();
	if(g_AuDb.m_DesignTransactionsBd.isEmpty())
	{
		// No sc design database transactions.
		g_AuDb.UnlockBdDesignTransactions();
		return;
	}

	// Grab a copy of the transaction list so we can release the mutex quickly.
	QList<AuDesignTransaction> transactions(g_AuDb.m_DesignTransactionsBd);
	g_AuDb.m_DesignTransactionsBd.clear();
	g_AuDb.m_RecentBd.clear();
	for(QList<AuDesignTransaction>::const_iterator ti = transactions.cbegin(); ti != transactions.cend(); ti++)
		g_AuDb.m_RecentBd.insert((*ti).m_idDesign);
	g_AuDb.UnlockBdDesignTransactions();

	const DgData& sdd = g_Au.LockBdDesignsForRead();

	if(g_Au.m_LogMessages & LOG_Database)
	{
		QString logmsg;
		logmsg = tr("Update Bd Designs %1 transactions").arg(transactions.count());
		g_Au.Log(logmsg);
	}

	// We grab copies of the affected records so we can release the mutex as quickly as
	// possible. Then we can take our time about saving the data, which takes a long
	// time.
	QList<AuBdDesign> designs;
	for(QList<AuDesignTransaction>::const_iterator tit = transactions.cbegin(); tit != transactions.cend(); tit++)
	{
		switch((*tit).m_Action)
		{
		case D_Invalid:
		case D_DesignDeactivate:
			continue;

		case D_DesignAdd:
		case D_DesignChange:
			if(sdd.BdDesignIsLoaded((*tit).m_idDesign))
			{
				AuBdDesign design;
				if(sdd.BdDesign((*tit).m_idDesign, design))
					designs.append(design);		// Found the design record.
			}
			break;
		case D_DesignDelete:
			continue;	// Don't need the record for deletes.
		}
	}

	// We have copies of the records we need so release the global data mutex.
	g_Au.UnlockBdDesigns();

	// Perform the transactions in sequence using our local copies of the database records.
	// This stack of ints is used to accumulate add,change,delete statistics for the final log entry.
	int a=0;
	int c=0;
	int d=0;
	int del=0;
	designsDeactivated.clear();	// Unique list of read-only designs that have been deactivated so other servers can be notified.
	for(QList<AuDesignTransaction>::const_iterator tit = transactions.cbegin(); tit != transactions.cend(); tit++)
	{
		switch((*tit).m_Action)
		{
		case D_Invalid:
			continue;
		case D_DesignDeactivate:
			if(!designsDeactivated.contains((*tit).m_idDesign))
			{
				designsDeactivated.append((*tit).m_idDesign);
				d++;
			}
			break;
		case D_DesignAdd:
			for(QList<AuBdDesign>::const_iterator it = designs.cbegin(); it != designs.cend(); it++)
				if((*it).Id() == (*tit).m_idDesign)
				{
					// Insert the added record into the database.
					(*it).Insert(db);
					a++;
					break;
				}
			break;
		case D_DesignChange:
			for(QList<AuBdDesign>::const_iterator it = designs.cbegin(); it != designs.cend(); it++)
				if((*it).Id() == (*tit).m_idDesign)
				{
					// Update the changed record into the database.
					(*it).Update(db);
					c++;
					break;
				}
			break;
		case D_DesignDelete:
			{
				QString sq = QString("DELETE FROM n_BdDesign WHERE id=%1;").arg((*tit).m_idDesign);

				QSqlQuery qdel(db);
				qdel.prepare(sq);
				if(!qdel.exec())
					qWarning() << "DtDb::StoreBdDesigns - DELETE FAILED" << sq;
				del++;
			}
			break;
		}
	}

	int tot = a + c + d + del;
	if(tot != transactions.count())
	{
		if(g_Au.m_LogMessages & LOG_Error)
		{
			QString logmsg = tr("error: Update Bd Designs: a%1 c%2 d%3 del%4 total %5 != %6 transactions")
				.arg(a)
				.arg(c)
				.arg(d)
				.arg(del)
				.arg(tot)
				.arg(transactions.count());
			g_Au.Error(logmsg);
		}
	}
	else
	{
		if(g_Au.m_LogMessages & LOG_Database)
		{
			QString logmsg = tr("Update Bd Designs: a%1 c%2 d%3 v%4")
				.arg(a)
				.arg(c)
				.arg(d)
				.arg(del);
			g_Au.Log(logmsg);
		}
	}
}

void DtDb::StoreBdExchange(QSqlDatabase& db)
{
	// Test the lists of added, changed, and deleted design records.
	g_AuDb.LockBdExchangeTransactions();
	if(g_AuDb.m_ExchangeTransactionsBd.isEmpty())
	{
		// No sc design database transactions.
		g_AuDb.UnlockBdExchangeTransactions();
		return;
	}

	// Grab a copy of the transaction list so we can release the mutex quickly.
	QList<AuDesignTransaction> transactions(g_AuDb.m_ExchangeTransactionsBd);
	g_AuDb.m_ExchangeTransactionsBd.clear();
	g_AuDb.UnlockBdExchangeTransactions();

	const DgData& sdd = g_Au.LockBdExchangeForRead();

	if(g_Au.m_LogMessages & LOG_Database)
	{
		QString logmsg;
		logmsg = tr("Update Bd Exchange %1 transactions").arg(transactions.count());
		g_Au.Log(logmsg);
	}

	// We grab copies of the affected records so we can release the mutex as quickly as
	// possible. Then we can take our time about saving the data, which takes a long
	// time.
	QList<AuBdDesignExchangeEntry> exchange;
	for(QList<AuDesignTransaction>::const_iterator tit = transactions.cbegin(); tit != transactions.cend(); tit++)
	{
		switch((*tit).m_Action)
		{
		case D_Invalid:
		case D_DesignDeactivate:
			continue;

		case D_DesignAdd:
		case D_DesignChange:
			{
				AuBdDesignExchange::const_iterator ei = sdd.m_BdExchange->find((*tit).m_idDesign);
				if(ei != sdd.m_BdExchange->end())
					exchange.append(*ei);
			}
			break;
		case D_DesignDelete:
			continue;	// Don't need the record for deletes.
		}
	}

	// We have copies of the records we need so release the global data mutex.
	g_Au.UnlockBdExchange();

	// Perform the transactions in sequence using our local copies of the database records.
	// This stack of ints is used to accumulate add,change,delete statistics for the final log entry.
	int a=0;
	int c=0;
	int del=0;
	for(QList<AuDesignTransaction>::const_iterator tit = transactions.cbegin(); tit != transactions.cend(); tit++)
	{
		switch((*tit).m_Action)
		{
		case D_Invalid:
			continue;
		case D_DesignDeactivate:
			continue;
		case D_DesignAdd:
			for(QList<AuBdDesignExchangeEntry>::const_iterator it = exchange.cbegin(); it != exchange.cend(); it++)
				if((*it).DesignId() == (*tit).m_idDesign)
				{
					// Insert the added record into the database.
					(*it).SqlInsert(db);
					a++;
					break;
				}
			break;
		case D_DesignChange:
			for(QList<AuBdDesignExchangeEntry>::const_iterator it = exchange.cbegin(); it != exchange.cend(); it++)
				if((*it).DesignId() == (*tit).m_idDesign)
				{
					// Update the changed record into the database.
					(*it).SqlUpdate(db);
					c++;
					break;
				}
			break;
		case D_DesignDelete:
			{
				QString sq = QString("DELETE FROM n_BdExchange WHERE id=%1;").arg((*tit).m_idDesign);

				QSqlQuery qdel(db);
				qdel.prepare(sq);
				if(!qdel.exec())
					qWarning() << "DtDb::StoreBdExchange - DELETE FAILED" << sq;
				del++;
			}
			break;
		}
	}

	int tot = a + c + del;
	if(tot != transactions.count())
	{
		if(g_Au.m_LogMessages & LOG_Error)
		{
			QString logmsg = tr("error: Update Bd Exchange: a%1 c%2 del%4 total %5 != %6 transactions")
				.arg(a)
				.arg(c)
				.arg(del)
				.arg(tot)
				.arg(transactions.count());
			g_Au.Error(logmsg);
		}
	}
	else
	{
		if(g_Au.m_LogMessages & LOG_Database)
		{
			QString logmsg = tr("Update Bd Exchange: a%1 c%2 v%4")
				.arg(a)
				.arg(c)
				.arg(del);
			g_Au.Log(logmsg);
		}
	}
}

void DtDb::StoreAtoms(QSqlDatabase& db, AuIdList& sectorsAdded, QList<AuTravelAtom>& atomsDeparted,
		QList<AuSpawnAtom>& atomsSpawned, QList<AuHengeTeleportAtom>& atomsHengeTeleported)
{
	// Test the lists of added, changed, and deleted atoms.
	g_AuDb.m_muAtomTransactions.lock();
	m_muAtomTransactions.lock();
	if(g_AuDb.m_AddedAtomIds.isEmpty() &&
			g_AuDb.m_ChangedAtomIds.isEmpty() &&
			g_AuDb.m_DeletedAtomIds.isEmpty() &&
			g_AuDb.m_DepartedAtoms.isEmpty() &&
			m_DepartingTravelers.isEmpty() &&
			m_SpawningAtoms.isEmpty() &&
			m_DepartingHengeTeleport.isEmpty())
	{
		m_muAtomTransactions.unlock();
		g_AuDb.m_muAtomTransactions.unlock();
		return;		// No atom database updates.
	}

	// Grab copies of the lists of atom ids that need to be updated.
	AuIdList addedAtomIds(g_AuDb.m_AddedAtomIds);
	g_AuDb.m_AddedAtomIds.clear();
	AuIdList changedAtomIds(g_AuDb.m_ChangedAtomIds);
	g_AuDb.m_ChangedAtomIds.clear();
	AuIdList deletedAtomIds(g_AuDb.m_DeletedAtomIds);
	g_AuDb.m_DeletedAtomIds.clear();
	AuIdList departedAtoms(g_AuDb.m_DepartedAtoms);
	g_AuDb.m_DepartedAtoms.clear();
	QList<AuTravelAtom> departTravelers(m_DepartingTravelers);
	m_DepartingTravelers.clear();
	QList<AuSpawnAtom> spawnAtoms(m_SpawningAtoms);
	m_SpawningAtoms.clear();
	QList<AuHengeTeleportAtom> departingHengeTeleport(m_DepartingHengeTeleport);
	m_DepartingHengeTeleport.clear();
	m_muAtomTransactions.unlock();
	g_AuDb.m_muAtomTransactions.unlock();

	// Atoms are needed for Inserts and Updates.
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaUniverse* pUniverse = ether.FindUniverse();
	if(!pUniverse)
	{
		g_Au.UnlockEtheriumDelete();
		qWarning("DtDb::StoreAtoms - No Universe!");
		return;
	}

	// We found the universe.
	if(g_Au.m_LogMessages & LOG_Database)
	{
		QString logmsg = QString("Update Universe Database - %1 added and %2 changed atoms.")
				.arg(addedAtomIds.count())
				.arg(changedAtomIds.count());
		g_Au.Log(logmsg);
	}

	// Insert each added atom into the database.
	if(g_Au.m_LogMessages & LOG_Database)
		g_Au.Log(QString("INSERT %1 atoms").arg(addedAtomIds.count()));
	sectorsAdded.clear();	// List of nu sectors that get stored.
	for(AuIdList::const_iterator it = addedAtomIds.cbegin(); it != addedAtomIds.cend(); it++)
	{
		DaAtom* atom = pUniverse->FindLiveIdRecursive(*it);
		if(!atom)
			continue;
		atom->Insert(db);	// Insert the added atom record into the database.
		atom->StoredInDb();

		const DaSector* sector = dynamic_cast<const DaSector*>(atom);
		if(sector)
			sectorsAdded.append(sector->Id());
	}
	if(g_Au.m_LogMessages & LOG_Database)
		g_Au.Log("INSERT atoms done");

	// Update each changed atom in the database.
#if !defined(QT_NO_DEBUG)
	int chganimal = 0;
	int chgarena = 0;
	int chgAsteroid = 0;
	int chgbd = 0;
	int chgcitizen = 0;
	int chgdesignenv = 0;
	int chgdesignplane = 0;
	int chgeffect = 0;
	int chggalaxy = 0;
	int chgitem = 0;
	int chgmissile = 0;
	int chgperson = 0;
	int chgplanet = 0;
	int chgroot = 0;
	int chgsector = 0;
	int chgship = 0;
	int chgsystem = 0;
	int chgstar = 0;
	int chgtel = 0;
	int chgter = 0;
	int chguniverse = 0;
	int chgvehicle = 0;
	int chgother = 0;
#endif
	// Postgres says the best defense against UPDATE deadlocks is consistent order so we must sort them.
	QMap<AuId,AuId> sortedIds;
	for(AuIdList::const_iterator it = changedAtomIds.cbegin(); it != changedAtomIds.cend(); it++)
		sortedIds.insert(*it, *it);

	if(g_Au.m_LogMessages & LOG_Database)
		g_Au.Log(QString("UPDATE %1 atoms").arg(sortedIds.count()));
	for(QMap<AuId,AuId>::const_iterator it = sortedIds.cbegin(); it != sortedIds.cend(); it++)
	{
		DaAtom* atom = pUniverse->FindLiveIdRecursive(*it);
		if(!atom)
			continue;
		atom->Update(db);	// Update the changed atom record in the database.
		atom->StoredInDb();
#if !defined(QT_NO_DEBUG)
		if(dynamic_cast<DaAnimal*>(atom))
			chganimal++;
		else if(dynamic_cast<DaAsteroid*>(atom))
			chgAsteroid++;
		else if(dynamic_cast<DaBd*>(atom))
			chgbd++;
		else if(dynamic_cast<DaArena*>(atom))
			chgarena++;
		else if(dynamic_cast<DaCitizen*>(atom))
			chgcitizen++;
		else if(dynamic_cast<DaDesignBase*>(atom))
			chgdesignenv++;
		else if(dynamic_cast<DaDesignPlane*>(atom))
			chgdesignplane++;
		else if(dynamic_cast<DaEffect*>(atom))
			chgeffect++;
		else if(dynamic_cast<DaGalaxy*>(atom))
			chggalaxy++;
		else if(dynamic_cast<DaItem*>(atom))
			chgitem++;
		else if(dynamic_cast<DaMissile*>(atom))
			chgmissile++;
		else if(dynamic_cast<DaPerson*>(atom))
			chgperson++;
		else if(dynamic_cast<DaWorld*>(atom))
			chgplanet++;
		else if(dynamic_cast<DaSector*>(atom))
			chgsector++;
		else if(dynamic_cast<DaVessel*>(atom))
			chgship++;
		else if(dynamic_cast<DaSolarSystem*>(atom))
			chgsystem++;
		else if(dynamic_cast<DaStar*>(atom))
			chgstar++;
		else if(dynamic_cast<DaTeleporter*>(atom))
			chgtel++;
		else if(dynamic_cast<DaUniverse*>(atom))
			chguniverse++;
		else if(dynamic_cast<DaVehicle*>(atom))
			chgvehicle++;
		else
			chgother++;
#endif
	}

	if(g_Au.m_LogMessages & LOG_Database)
	{
		QString logmsg = tr("UPDATE atoms done - Add %1, Change %2, Delete %3, Depart %4, Spawn %5, Travel %6")
			.arg(addedAtomIds.count()).arg(sortedIds.count()).arg(deletedAtomIds.count()).arg(departedAtoms.count())
			.arg(atomsSpawned.count()).arg(atomsDeparted.count());
		g_Au.Log(logmsg);

#if !defined(QT_NO_DEBUG)
		if(chganimal)
			qDebug(" %d animals changed", chganimal);
		if(chgarena)
			qDebug(" %d arenas changed", chgarena);
		if(chgAsteroid)
			qDebug(" %d asteroids changed", chgAsteroid);
		if(chgbd)
			qDebug(" %d bds changed", chgbd);
		if(chgcitizen)
			qDebug(" %d citizens changed", chgcitizen);
		if(chgdesignenv)
			qDebug(" %d design environments changed", chgdesignenv);
		if(chgdesignplane)
			qDebug(" %d design planes changed", chgdesignplane);
		if(chgeffect)
			qDebug(" %d effects changed", chgeffect);
		if(chggalaxy)
			qDebug(" %d galaxies changed", chggalaxy);
		if(chgitem)
			qDebug(" %d items changed", chgitem);
		if(chgmissile)
			qDebug(" %d missiles changed", chgmissile);
		if(chgperson)
			qDebug(" %d people changed", chgperson);
		if(chgplanet)
			qDebug(" %d planets changed", chgplanet);
		if(chgroot)
			qDebug(" %d roots changed", chgroot);
		if(chgsector)
			qDebug(" %d sectors changed", chgsector);
		if(chgsystem)
			qDebug(" %d solar systems changed", chgsystem);
		if(chgship)
			qDebug(" %d spacecraft changed", chgship);
		if(chgstar)
			qDebug(" %d stars changed", chgstar);
		if(chgtel)
			qDebug(" %d teleporters changed", chgtel);
		if(chgter)
			qDebug(" %d terrain patches changed", chgter);
		if(chguniverse)
			qDebug(" %d universes changed", chguniverse);
		if(chgvehicle)
			qDebug(" %d vehicles changed", chgvehicle);
		if(chgother)
			qDebug(" %d other atoms changed", chgother);
#endif
	}

	if(g_Au.IsSceneWorker())
	{
		// Atoms changes have been saved.
		// Remove atoms from this worker's local data that are traveling to other servers.
		// This is the where we end up next after a traveling atom cannot be handled locally.
		if(g_Au.m_LogMessages & LOG_Travel)
			if(departTravelers.count())
				qDebug() << "DtDb::StoreAtoms departTravelers" << departTravelers.count();
		for(QList<AuTravelAtom>::const_iterator it = departTravelers.cbegin(); it != departTravelers.cend(); it++)
		{
			DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom((*it).m_OrigSystem));
			if(system)
			{
				// Found the origin solar system. Look for the traveling atom there.
				DaAtom* trav = system->FindLiveIdRecursive((*it).m_Traveler);
				if(trav && trav->IsTraveling())
				{
					// Remove the traveler from my local data. It is not deleted from the database.
					ether.TrashAtom(trav->Id());

					// List the travelers we actually find and remove, so the caller can signal their departures after we
					// have finished saving changes to the database.
					atomsDeparted.append(*it);
					if(g_Au.m_LogMessages & LOG_Travel)
						qDebug() << "DtDb::StoreAtoms traveler" << (*it).m_Traveler << "atomsDeparted" << atomsDeparted.count();
				}
				else if(g_Au.m_LogMessages & LOG_Error)
				{
					if(trav)
						qWarning("ERROR! DtDb::StoreAtoms traveler %d %s not traveling", trav->Id().GetID(),
								(const char*)trav->Name().toLatin1());
					else
						qWarning() << "ERROR! DtDb::StoreAtoms traveler" << (*it).m_Traveler << "not found in system"
							<< system->Id() << system->Name();
				}
			}
			else if(g_Au.m_LogMessages & LOG_Error)
				qWarning("ERROR! DtDb::StoreAtoms traveler %d orig system %d not found", (*it).m_Traveler.GetID(),
						(*it).m_OrigSystem.GetID());
		}

		// Remove people from this worker's local data that are spawning on other servers.
		if(g_Au.m_LogMessages & LOG_Travel)
			if(spawnAtoms.count() || atomsDeparted.count())
				qDebug() << "DtDb::StoreAtoms spawnAtoms" << spawnAtoms.count() << "atomsDeparted" << atomsDeparted.count();
		for(QList<AuSpawnAtom>::const_iterator it = spawnAtoms.cbegin(); it != spawnAtoms.cend(); it++)
		{
			// Look for the spawning atom.
			DaAtom* trav = pUniverse->FindLiveIdRecursive((*it).m_Person);
			if(trav && trav->IsTraveling())
			{
				// Remove the traveler from my local data. It is not deleted from the database.
				ether.TrashAtom(trav->Id());

				// List the atom spawns we actually find and remove, so the caller can signal their departures after we
				// have finished saving changes to the database.
				atomsSpawned.append(*it);
				if(g_Au.m_LogMessages & LOG_Travel)
					qDebug() << "DtDb::StoreAtoms spawnAtom" << (*it).m_Person << "atomsSpawned" << atomsSpawned.count();
			}
			else if(g_Au.m_LogMessages & LOG_Error)
			{
				if(trav)
					qWarning("ERROR! DtDb::StoreAtoms spawn atom %d %s not traveling", trav->Id().GetID(),
							(const char*)trav->Name().toLatin1());
				else
					qWarning("ERROR! DtDb::StoreAtoms spawn atom %d not found", (*it).m_Person.GetID());
			}
		}

		if(g_Au.m_LogMessages & LOG_Travel)
			if(departingHengeTeleport.count() || atomsSpawned.count())
				qDebug() << "DtDb::StoreAtoms departingHengeTeleport" << departingHengeTeleport.count()
					<< "atomsSpawned" << atomsSpawned.count();
		for(QList<AuHengeTeleportAtom>::const_iterator it = departingHengeTeleport.cbegin(); it != departingHengeTeleport.cend();
				it++)
		{
			// Look for the spawning atom.
			DaAtom* trav = pUniverse->FindLiveIdRecursive((*it).m_TeleportId);
			if(trav && trav->IsTraveling())
			{
				// Remove the traveler from my local data. It is not deleted from the database.
				ether.TrashAtom(trav->Id());

				// List the atom spawns we actually find and remove, so the caller can signal their departures after we
				// have finished saving changes to the database.
				atomsHengeTeleported.append(*it);
				if(g_Au.m_LogMessages & LOG_Travel)
					qDebug() << "DtDb::StoreAtoms henge teleported atom" << (*it).m_TeleportId << "atomsHengeTeleported"
						<< atomsHengeTeleported.count();
			}
			else if(g_Au.m_LogMessages & LOG_Error)
			{
				if(trav)
					qWarning("ERROR! DtDb::StoreAtoms henge teleport atom %d %s not traveling", trav->Id().GetID(),
							(const char*)trav->Name().toLatin1());
				else
					qWarning("ERROR! DtDb::StoreAtoms henge teleport atom %d not found", (*it).m_TeleportId.GetID());
			}
		}

		// The scene worker needs to remove game piece atoms that exist in solar systems that are loaded for star field only.
		// This can occur when a solar system assignment is revoked by the manager.
		if(g_Au.m_LogMessages & LOG_Database)
			g_Au.Log("DtDb::StoreAtoms - RemoveGamePiecesFromStarField");
		pUniverse->RemoveGamePiecesFromStarField();
		if(g_Au.m_LogMessages & LOG_Database)
			g_Au.Log("DtDb::StoreAtoms - RemoveGamePiecesFromStarField done");
	}
	else if(g_Au.IsSceneManager())
	{
		// The scene manager deletes sectors from the local atom data that are no longer assigned.
		unordered_map<AuId, AuMySector, AuIdHash> idSectors;							// Sectors assigned to me.
		if(g_Au.IsSceneManager())
			auSceneServerApp->ListSectorsAssignedToMe(idSectors);

		DaAtomList subs;
		pUniverse->ListSubatoms(subs);
		for(DaAtomList::const_iterator gi = subs.cbegin(); gi != subs.cend(); gi++)
		{
			DaGalaxy* galaxy = dynamic_cast<DaGalaxy*>(*gi);
			if(!galaxy)
				continue;

			// Get a list of all the sectors in the galaxy.
			DaAtomList sectors;
			galaxy->ListSectors(sectors);

			// Remove sectors that are not in the assigned list.
			for(DaAtomList::const_iterator it=sectors.begin(); it != sectors.end(); it++)
				if(idSectors.find((*it)->Id()) == idSectors.end())
				{
					// Sector is not in assigment list
					// Move the atom to delete to the trash so it can be deleted quietly during another thread.
					galaxy->RemoveSectorLoc((*it)->Id(), (*it)->Location());
					ether.TrashAtom((*it)->Id());
				}
		}
	}
	g_Au.UnlockEtheriumDelete();

	// Traveling atoms know the parent id of their destination. We must update them now because their current transform is
	// relative to the nu parent atom but they were still subordinate to their origin atom when they were last updated.
	if(g_Au.m_LogMessages & LOG_Database)
		qDebug() << "DtDb::StoreAtoms departed travelers" << atomsDeparted.count();
	for(QList<AuTravelAtom>::const_iterator tpi = atomsDeparted.cbegin(); tpi != atomsDeparted.cend(); tpi++)
	{
		if((*tpi).IsGalaxyLoc())
		{
			//qDebug() << "DtDb::StoreAtoms atom departed" << (*tpi).m_Traveler << "sysId" << (*tpi).m_DestSystem;
			QString stp = QString("UPDATE a_Atom SET idp=?, locx=?,locy=?,locz=?, rotx=?,roty=?,rotz=? WHERE id=%1;")
				.arg((*tpi).m_Traveler.GetID());
			QSqlQuery qtp(db);
			qtp.prepare(stp);
			qtp.addBindValue((*tpi).m_DestSystem.GetID());
			qtp.addBindValue(0);
			qtp.addBindValue(0);
			qtp.addBindValue(0);
			qtp.addBindValue(0);
			qtp.addBindValue(0);
			qtp.addBindValue(FeetPerSolarSystem()/2);	// Put me above the solar system at a safe distance from the largest star.
			if(!qtp.exec())
				qWarning() << "DtDb::StoreAtoms - UPDATE galaxyLoc traveler atom FAILED" << stp;
		}
		else
		{
			QString stp = QString("UPDATE a_Atom SET idp=%2 WHERE id=%1;")
				.arg((*tpi).m_Traveler.GetID())
				.arg((*tpi).m_DestSystem.GetID());
			QSqlQuery qtp(db);
			qtp.prepare(stp);
			if(!qtp.exec())
				qWarning() << "DtDb::StoreAtoms - UPDATE traveler atom FAILED" << stp;
		}
	}

	// Update parent ids of atoms that have departed. The nu parent id is zero when admin teleporting and spawning.
	if(g_Au.m_LogMessages & LOG_Database)
		qDebug() << "DtDb::StoreAtoms departedAtoms" << departedAtoms.count();
	for(AuIdList::const_iterator dai = departedAtoms.cbegin(); dai != departedAtoms.cend(); dai++)
	{
		QString sqdep = QString("UPDATE a_Atom SET idp=0 WHERE id=%1;").arg((*dai).GetID());
		QSqlQuery qdep(db);
		qdep.prepare(sqdep);
		if(!qdep.exec())
			qWarning("DtDb::StoreAtoms - UPDATE FAILED '%s'", (const char*)sqdep.toLatin1());
	}

	// Delete atom records from the database. This only requires id's.
	if(!deletedAtomIds.isEmpty())
	{
		if(g_Au.m_LogMessages & LOG_Database)
			g_Au.Log(QString("DELETE %1 atoms").arg(deletedAtomIds.count()));
		while(!deletedAtomIds.isEmpty())
		{
			// Build a query for a batch of the atoms that we need to delete, up to 1000 per batch.
			QString sqa("DELETE FROM a_Atom WHERE");
			QString delid;
			int delCount = 1000;									// We will delete in batches of 1000 max.
			while(!deletedAtomIds.isEmpty() && delCount--)
			{
				AuId id = deletedAtomIds.takeFirst();

				if(!delid.isEmpty())
				{
					// OR additional Id's after the first.
					sqa += " OR";
				}

				delid = QVariant(id.GetID()).toString();
				sqa += " id=" + delid;
			}
			sqa += ";";

			// Run the delete atom query.
			QSqlQuery qdela(db);
			qdela.prepare(sqa);
			if(!qdela.exec())
				qWarning("DtDb::StoreAtoms - DELETE FAILED '%s'", (const char*)sqa.toLatin1());
		}
		if(g_Au.m_LogMessages & LOG_Database)
			g_Au.Log("DELETE atoms done");
	}

	// Master keeps other atoms cleaned up too.
	// This is a slow process so we only do it the first time the master saves atoms.
	if(g_Au.IsSceneMaster() && m_FirstSave)
	{
		if(g_Au.m_LogMessages & LOG_Database)
			g_Au.Log("Delete unneeded atoms.");
		// Delete orphan solar systems. These are left behind by sectors that decay.
		QSqlQuery qdelsystems(db);
		qdelsystems.prepare("DELETE FROM a_SolarSystem WHERE idp NOT IN (SELECT id FROM a_Sector);");
		if(!qdelsystems.exec())
			qWarning("DtDb::StoreAtoms - DELETE orphan solar systems FAILED");
		int numAffected = qdelsystems.numRowsAffected();
		if(g_Au.m_LogMessages & LOG_Database)
			g_Au.Log(QString("Deleted %1 unneeded solar systems.").arg(numAffected));

		// Delete orphan stars. These are left behind by solar systems and stars that decay.
		// This can leave more orphan stars. We'll just get them next time through rather than doing this query again.
		QSqlQuery qdelstars(db);
		qdelstars.prepare("DELETE FROM a_Star WHERE idp NOT IN (SELECT id FROM a_Star) AND "
				"idp NOT IN (SELECT id FROM a_SolarSystem) AND "
				"idp NOT IN (SELECT id FROM a_Designer) AND "
				"idp NOT IN (SELECT id FROM a_Arena);");
		if(!qdelstars.exec())
			qWarning("DtDb::StoreAtoms - DELETE orphan stars FAILED");
		numAffected = qdelstars.numRowsAffected();
		if(g_Au.m_LogMessages & LOG_Database)
			g_Au.Log(QString("Deleted %1 unneeded stars.").arg(numAffected));

		// Delete orphan planets. These are left behind by stars and planets that decay.
		// This can leave more orphan planets. We'll just get them next time through rather than doing this query again.
		QSqlQuery qdelplanets(db);
		qdelplanets.prepare("DELETE FROM a_World WHERE idp NOT IN (SELECT id FROM a_World) AND "
				"idp NOT IN (SELECT id FROM a_Star);");
		if(!qdelplanets.exec())
			qWarning("DtDb::StoreAtoms - DELETE orphan planets FAILED");
		numAffected = qdelplanets.numRowsAffected();
		if(g_Au.m_LogMessages & LOG_Database)
			g_Au.Log(QString("Deleted %1 unneeded planets.").arg(numAffected));

		// Delete people that were deleted from their account.
		// Must release their berth, if they are holding one.
		{
			QString sq("SELECT id FROM a_Person WHERE idp=-1;");
			QSqlQuery qa(QString(), db);
			qa.prepare(sq);
			qa.exec();
			while(qa.next())
			{
				AuId personId = AuId(qa.value(0).toUInt());
				emit PersonDeleted(personId);
			}
		}
		/*
		QSqlQuery qdelpeople(db);
		qdelpeople.prepare("DELETE FROM a_Person WHERE idp=-1;");
		if(!qdelpeople.exec())
			qWarning("DtDb::StoreAtoms - DELETE people FAILED");
			*/
		QSqlQuery qdelpeople(db);
		qdelpeople.prepare("UPDATE a_Person SET idp=-2 WHERE idp=-1;");
		if(!qdelpeople.exec())
			//qWarning("DtDb::StoreAtoms - DELETE people FAILED");
			qWarning("DtDb::StoreAtoms - Reparent people to -2 FAILED");

		numAffected = qdelpeople.numRowsAffected();
		if(g_Au.m_LogMessages & LOG_Database)
			//g_Au.Log(QString("Deleted %1 unneeded people.").arg(numAffected));
			g_Au.Log(QString("Reparented %1 unneeded people to -2, rather than delete them.").arg(numAffected));

		if(g_Au.m_LogMessages & LOG_Database)
			g_Au.Log("Delete unneeded atoms done.");
	}

	if(g_Au.m_LogMessages & LOG_Database)
		g_Au.Log(tr("Update Universe Database - Finished. %1 departed. %2 spawned. %3 sectors added.")
				.arg(atomsDeparted.count())
				.arg(atomsSpawned.count())
				.arg(sectorsAdded.count()));
}

void DtDb::StoreEmpires(QSqlDatabase& db, QList<quint32>& empiresChanged)
{
	// Only scene master stores empires.
	if(!g_Au.IsSceneMaster())
		return;

	// Acquire the list of added and changed empires.
	m_muEmpireTransactions.lock();
	if(m_AddedEmpireIds.isEmpty() && m_ChangedEmpireIds.isEmpty())
	{
		m_muEmpireTransactions.unlock();
		return;	// No empire database updates.
	}

	// Make copies of the id lists so we can clear them and release the mutex.
	QList<quint32> addIds(m_AddedEmpireIds);
	QList<quint32> chgIds(m_ChangedEmpireIds);
	m_AddedEmpireIds.clear();
	m_ChangedEmpireIds.clear();
	m_muEmpireTransactions.unlock();

	//qDebug("DtDb::StoreEmpires - add %d change %d", addIds.count(), chgIds.count());

	empiresChanged.clear();	// List of ids of empires added/changed.
	const DgData& gd = g_Au.LockDataForRead();

	// Insert each added empire record into the database.
	for(QList<quint32>::const_iterator it = addIds.cbegin(); it != addIds.cend(); it++)
	{
		DgEmpire emp;
		emp.m_id = *it;
		if(gd.Empire(emp))
		{
			emp.Insert(db);
			empiresChanged.append(*it);
		}
	}

	// Update each changed empire record in the database.
	for(QList<quint32>::const_iterator it = chgIds.cbegin(); it != chgIds.cend(); it++)
	{
		DgEmpire emp;
		emp.m_id = *it;
		if(gd.Empire(emp))
		{
			emp.Update(db);
			empiresChanged.append(*it);
		}
	}

	g_Au.UnlockData();
}

void DtDb::StoreSpecies(QSqlDatabase& db, QList<AuSpecies>& speciesChanged)
{
	// Only scene master stores empires.
	if(!g_Au.IsSceneMaster())
		return;

	// Acquire the list of added and changed empires.
	m_muSpecieTransactions.lock();
	if(m_AddedSpecies.isEmpty() && m_ChangedSpecies.isEmpty())
	{
		m_muSpecieTransactions.unlock();
		return;	// No empire database updates.
	}

	// Make copies of the id lists so we can clear them and release the mutex.
	QList<AuSpecies> addIds(m_AddedSpecies);
	QList<AuSpecies> chgIds(m_ChangedSpecies);
	m_AddedSpecies.clear();
	m_ChangedSpecies.clear();
	m_muSpecieTransactions.unlock();

	//qDebug("DtDb::StoreSpecies - add %d", addIds.count());

	speciesChanged.clear();		// List of ids of species added.
	const DgData& gd = g_Au.LockDataForRead();

	// Insert each added specie record into the database.
	for(QList<AuSpecies>::const_iterator it = addIds.cbegin(); it != addIds.cend(); it++)
	{
		DzSpeciesHash::const_iterator si = gd.m_Species.find(*it);
		if(si != gd.m_Species.end())
		{
			(*si).second.Insert(db);
			speciesChanged.append(*it);
		}
	}

	// Update each changed specie record in the database.
	for(QList<AuSpecies>::const_iterator it = chgIds.cbegin(); it != chgIds.cend(); it++)
	{
		DzSpeciesHash::const_iterator si = gd.m_Species.find(*it);
		if(si != gd.m_Species.end())
		{
			(*si).second.Update(db);
			speciesChanged.append(*it);
		}
	}

	g_Au.UnlockData();
}

void DtDb::StoreStories(QSqlDatabase& db, QList<quint32>& storiesChanged, QList<quint32>& storiesDeleted)
{
	// Only scene worker stores stories.
	if(!g_Au.IsSceneWorker())
		return;

	// Acquire the list of added and changed stories.
	g_AuDb.LockDbStoryIds();
	if(g_AuDb.m_AddedStoryIds.isEmpty() &&
			g_AuDb.m_ChangedStoryIds.isEmpty() &&
			g_AuDb.m_DeletedStoryIds.isEmpty())
	{
		g_AuDb.UnlockDbStoryIds();
		return;	// No story database updates.
	}

	// Make copies of the id lists so we can clear them and release the mutex.
	QList<quint32> addIds(g_AuDb.m_AddedStoryIds);
	g_AuDb.m_AddedStoryIds.clear();
	QList<quint32> chgIds(g_AuDb.m_ChangedStoryIds);
	g_AuDb.m_ChangedStoryIds.clear();
	QList<quint32> delIds(g_AuDb.m_DeletedStoryIds);
	g_AuDb.m_DeletedStoryIds.clear();
	g_AuDb.UnlockDbStoryIds();

	//qDebug("DtDb::StoreStories - add %d change %d delete %d", addIds.count(), chgIds.count(), delIds.count());

	// Insert each added record into the database.
	storiesChanged.clear();
	for(QList<quint32>::const_iterator it = addIds.cbegin(); it != addIds.cend(); it++)
	{
		AuStory* story = g_AuDb.LockStory(*it);
		if(story)
		{
			AuStory storyCopy(*story);		// Use a copy so we don't have to keep the mutex locked during the sql query.
			g_AuDb.UnlockStory(*it);

			storyCopy.Insert(db);
			storiesChanged.append(*it);
		}
		else
			g_AuDb.UnlockStory(*it);
	}

	// Update each changed record in the database.
	for(QList<quint32>::const_iterator it = chgIds.cbegin(); it != chgIds.cend(); it++)
	{
		AuStory* story = g_AuDb.LockStory(*it);
		if(story)
		{
			AuStory storyCopy(*story);		// Use a copy so we don't have to keep the mutex locked during the sql query.
			g_AuDb.UnlockStory(*it);

			storyCopy.Update(db);
			storiesChanged.append(*it);
		}
		else
			g_AuDb.UnlockStory(*it);
	}

	// Delete story records from the database. This only requires id's.
	if(!delIds.isEmpty())
	{
		// Build a query for all the stories that we need to delete.
		storiesDeleted.clear();
		QString sqa("DELETE FROM g_Story WHERE");
		QString delid;
		for(QList<quint32>::const_iterator it = delIds.cbegin(); it != delIds.cend(); it++)
		{
			storiesDeleted.append(*it);
			if(!delid.isEmpty())
			{
				// OR additional Id's after the first.
				sqa += " OR";
			}

			delid = QVariant(*it).toString();
			sqa += " id=" + delid;
		}
		sqa += ";";

		// Run the delete query.
		QSqlQuery qdela(db);
		qdela.prepare(sqa);
		if(!qdela.exec())
			qWarning("DtDb::StoreStories - DELETE FAILED '%s'",
					(const char*)sqa.toLatin1());
	}
}

// Load atoms that have been assigned to this server by its boss.
void DtDb::LoadAssignedAtoms()
{
	if(g_Au.IsSceneMaster())
	{
		//qDebug("DtDb::LoadAssignedAtoms - SceneMaster DbStatus %d",
				//m_eDbStatus);
		if(m_eDbStatus == NotLoaded)
		{
			// SceneMaster loads assigned atoms only once.
			QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
			if(!OpenDatabase(db))
				return;	// Failed to open database?

			// Init id values from the database.
			// Only the scene master does this. The scene manager and scene
			// worker must request ids from their boss.
			// We don't request ids from the database on demand using nextval
			// because we run into thread problems, i.e. the tick thread
			// creates a nu atom and it needs an id NOW. I might be talking
			// to the database at the time; multiple threads may not access
			// the database at the same time.
			// Also, we don't want to deluge the sql server with nu id
			// requests from a potentially large number of scene workers.
			InitNextIds(db);

			// SceneMaster is implicitly assigned the universe, sectors, solar
			// systems, and wormholes.
			// Load the known Universe and Galaxy from the database.
			g_Au.ClearEtherium();	// Start by clearing the current ether.
			AuEther& ether = g_Au.LockEtheriumDelete();
			DaUniverse* universe = LoadGalaxies(db, ether);
			if(universe)
			{
				// Universe loaded. Insure all galaxies are present, including late additions.
				universe->CreateMissingGalaxies();

				// Load all sectors in the galaxies and their primary stars.
				DaAtomList subs;
				universe->ListSubatoms(subs);
				for(DaAtomList::const_iterator gi = subs.cbegin(); gi != subs.cend(); gi++)
				{
					DaGalaxy* galaxy = dynamic_cast<DaGalaxy*>(*gi);
					if(!galaxy)
						continue;

					// Load all sectors into the etherium.
					DaSector::LoadSubatoms(ether, db, galaxy->Id(), LOAD_Systems);

					// Delete sectors that are not needed.
					AuIdList deletedSectors;	// output from DecaySectors. Nobody to notify.
					auSceneServerApp->DecaySectors(galaxy, deletedSectors);

					// Repair all the wormholes in the galaxy.
					galaxy->FinalizeAllWormholes();
				}
			}
			else
			{
				// No Universe?
				// Create a nu universe with a bunch of galaxies in it.
				DaUniverse* universe = new DaUniverse;
				universe->ConnectSignals();
				universe->Create();
				universe->SetPosAndOrient(AuAtomPos(AuId(0), AuMatrix4x4()));
				ether.AddAtom(universe);
				universe->InitGalaxies();	// Create the galaxies.

				g_AuDb.AtomAdded(universe->Id());
				universe->SetNormal();

				// Atom id is usually 2 for the Universe.
				// Atom id 1 is used by mail to identify server-sent message from the recipient's government.
				// Be sure this doesn't change on us unexpectedly.
				Q_ASSERT(universe->Id() != AuId::GovtMail());
			}
			g_Au.UnlockEtheriumDelete();

			db.close();	// We don't keep the real database open.
			m_eDbStatus = Loaded;
			//qDebug("DtDb::LoadAssignedAtoms - Master FinishedInit");
			emit FinishedInit();
		}
		else if(m_eDbStatus == Loaded)
		{
			// Scene master also rescues atoms from limbo. This occurs when an admin attempts to teleport to an atom but the atom
			// query times out after five consecutive attempts. It signals me to attempt to rescue the atom in limbo.
			m_muAtomsInLimbo.lock();
			AuIdList limboAtoms(m_AtomsInLimbo);
			m_AtomsInLimbo.clear();
			m_muAtomsInLimbo.unlock();

			if(limboAtoms.isEmpty())
				return;	// Nothing to do.

			// Open the database.
			QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
			if(!OpenDatabase(db))
				return;	// Failed to open database?

			// Rescue atoms from limbo.
			for(AuIdList::const_iterator lai = limboAtoms.cbegin(); lai != limboAtoms.cend(); lai++)
			{
				// This happens rarely enough that we will log every step of the process.
				// We are only looking to rescue person and officer atoms.
				// We will rescue the vehicle or spacecraft they are in also.
				// First locate the person atom in question.
				bool isPerson = false;
				quint32 idp = 0;			// Parent id of the person/officer atom. May be a vehicle or a ship. May be 0.
				QString name;
				quint32 empireId = 0;
				GALAXY galaxy = GALAXY_Count;
				QString sq = QString("SELECT idp,name,allegiance,spawnGalaxy FROM a_Person WHERE id=%1;").arg((*lai).GetID());
				QSqlQuery qa(QString(), db);
				qa.prepare(sq);
				qa.exec();
				if(qa.next())
				{
					// Get parent atom id of person.
					isPerson = true;
					idp = qa.value(0).toUInt();			// Parent id of the person atom. May be a vehicle or a ship.
					name = qa.value(1).toString();
					empireId = qa.value(2).toUInt();
					galaxy = (GALAXY)qa.value(3).toInt();
					qa.finish();
					if(g_Au.m_LogMessages & LOG_Admin)
					{
						QString logmsg = QString("Limbo Rescue: Person %1 %2 located. Empire %4. Parent %3.")
							.arg((*lai).GetID()).arg(name).arg(idp).arg(empireId);
						g_Au.Log(logmsg);
					}
				}
				else
				{
					// The person atom was not loaded. Try loading an officer.
					qa.finish();

					sq = QString("SELECT idp,name,allegiance FROM a_Npc WHERE id=%1 AND role=%2;")
						.arg((*lai).GetID())
						.arg(ROLE_Officer);
					QSqlQuery qoff(QString(), db);
					qoff.prepare(sq);
					qoff.exec();
					if(qoff.next())
					{
						// Get parent atom id of officer.
						isPerson = false;
						idp = qoff.value(0).toUInt();			// Parent id of the person atom. May be a vehicle or a ship.
						name = qoff.value(1).toString();
						empireId = qoff.value(2).toUInt();
						if(g_Au.m_LogMessages & LOG_Admin)
						{
							QString logmsg = QString("Limbo Rescue: Officer %1 %2 located. Empire %4. Parent %3.")
								.arg((*lai).GetID()).arg(name).arg(idp).arg(empireId);
							g_Au.Log(logmsg);
						}
					}
					else
					{
						// The limbo rescuee is not a person or an officer.
						if(g_Au.m_LogMessages & LOG_Admin)
						{
							QString logmsg = QString("Limbo Rescue: Officer was not found in the database; atom %1 may not be a "
									"person or an officer.").arg((*lai).GetID());
							g_Au.Log(logmsg);
						}
					}
					qoff.finish();
				}

				// Check to see if the parent atom is a vehicle or a spacecraft, or a vehicle in a spacecraft.
				quint32 scId = 0;
				quint32 vehicleId = 0;
				if(idp)
				{
					// Rescuee has a parent id.
					// Check for vehicle.
					QString sqv = QString("SELECT id,idp FROM a_Vehicle WHERE id=%1;").arg(idp);
					QSqlQuery qav(QString(), db);
					qav.prepare(sqv);
					qav.exec();
					if(qav.next())
					{
						vehicleId = qav.value(0).toUInt();
						idp = qav.value(1).toUInt();				// Update parent id so we get parent spacecraft of vehicle.
						if(g_Au.m_LogMessages & LOG_Admin)
						{
							QString logmsg = QString("Limbo Rescue: %4 %1 %2 parent vehicle %3 located.")
								.arg((*lai).GetID()).arg(name).arg(vehicleId).arg(isPerson?"Person":"Officer");
							g_Au.Log(logmsg);
						}
					}
					qav.finish();

					// Check for Sc spacecraft, which may be parent of vehicle.
					QString sqsc = QString("SELECT id FROM a_Sc WHERE id=%1;").arg(idp);
					QSqlQuery qasc(QString(), db);
					qasc.prepare(sqsc);
					qasc.exec();
					if(qasc.next())
					{
						scId = qasc.value(0).toUInt();			// Spacecraft was found.
						if(g_Au.m_LogMessages & LOG_Admin)
						{
							QString logmsg = QString("Limbo Rescue: %4 %1 %2 parent sc %3 located.")
								.arg((*lai).GetID()).arg(name).arg(scId).arg(isPerson?"Person":"Officer");
							g_Au.Log(logmsg);
						}
					}
					qasc.finish();
				}

				if(!name.isEmpty())
				{
					// We found a person or an officer.
					// Now load the actual atom we care about.
					quint8 rideAlso = 0;
					DaAtom* atomLoaded = 0;
					AuEther loadEther(__FILE__, __LINE__);			// Destructor will delete any atoms that we load.
					if(scId)
					{
						// Load the spacecraft and all its contents.
						if(g_Au.m_LogMessages & LOG_Admin)
						{
							QString logmsg = QString("Limbo Rescue: Loading sc %3 with %4 %1 %2 aboard.")
								.arg((*lai).GetID()).arg(name).arg(scId).arg(isPerson?"person":"officer");
							g_Au.Log(logmsg);
						}
						atomLoaded = DaSc::LoadSubatoms(loadEther, db, AuId(0), AuId(scId));
						if(atomLoaded)
							rideAlso = 2;
						else if(g_Au.m_LogMessages & LOG_Error)
							g_Au.Error("Limbo Rescue: FAILED TO LOAD SPACECRAFT.");
					}
					else if(vehicleId)
					{
						// Load the vehicle and all its contents.
						if(g_Au.m_LogMessages & LOG_Admin)
						{
							QString logmsg = QString("Limbo Rescue: Loading vehicle %3 with %4 %1 %2 aboard.")
								.arg((*lai).GetID()).arg(name).arg(vehicleId).arg(isPerson?"person":"officer");
							g_Au.Log(logmsg);
						}
						atomLoaded = DaVehicle::LoadSubatoms(loadEther, db, AuId(0), AuId(vehicleId));
						if(atomLoaded)
							rideAlso = 1;
						else if(g_Au.m_LogMessages & LOG_Error)
							g_Au.Error("Limbo Rescue: FAILED TO LOAD VEHICLE.");
					}

					if(!atomLoaded)
					{
						if(isPerson)
						{
							// Load the person.
							if(g_Au.m_LogMessages & LOG_Admin)
							{
								QString logmsg = QString("Limbo Rescue: Loading person %1 %2.").arg((*lai).GetID()).arg(name);
								g_Au.Log(logmsg);
							}
							atomLoaded = DaPerson::LoadSubatoms(loadEther, db, AuId(0), *lai);
							if(!atomLoaded && (g_Au.m_LogMessages & LOG_Error))
								g_Au.Error("Limbo Rescue: FAILED TO LOAD PERSON.");
						}
						else
						{
							// Load the officer.
							if(g_Au.m_LogMessages & LOG_Admin)
							{
								QString logmsg = QString("Limbo Rescue: Loading officer %1 %2.").arg((*lai).GetID()).arg(name);
								g_Au.Log(logmsg);
							}
							atomLoaded = DaNpc::LoadSubatoms(loadEther, db, AuId(0), *lai);
							if(!atomLoaded && (g_Au.m_LogMessages & LOG_Error))
								g_Au.Error("Limbo Rescue: FAILED TO LOAD OFFICER.");
						}
					}

					if(atomLoaded)
					{
						// We loaded the atom we will transfer. Decide where to send it.
						QString atomName = atomLoaded->Name();	// Cannot call Name while we have data locked.
						const DgData& gd = g_Au.LockDataForRead();
						AuId destCity;
						bool hasCapital = gd.EmpireCapital(empireId, destCity);
						if(hasCapital)
						{
							if(g_Au.m_LogMessages & LOG_Admin)
							{
								QString logmsg = QString("Limbo Rescue: Transferring %1 %2 to capital city %3 of %5's empire %4.")
									.arg(atomLoaded->Id().GetID()).arg(atomName).arg(destCity.GetID()).arg(empireId)
									.arg(isPerson?"person":"officer");
								g_Au.Log(logmsg);
							}
						}
						else
						{
							// Find a random city in their own empire.
							QSet<AuId> exceptIds;
							bool hasCity = gd.RandomEmpireCity(empireId, destCity, exceptIds);
							if(!hasCity)
								hasCity = gd.RandomEmpireCity(empireId, destCity, exceptIds);
							if(hasCity)
							{
								if(g_Au.m_LogMessages & LOG_Admin)
								{
									QString logmsg = QString("Limbo Rescue: Transferring %1 %2 to city %3 of %5's empire %4.")
										.arg(atomLoaded->Id().GetID()).arg(atomName).arg(destCity.GetID()).arg(empireId)
										.arg(isPerson?"person":"officer");
									g_Au.Log(logmsg);
								}
							}
							else
							{
								// Find a random empire with a capital.
								empireId = gd.JoinRandomEmpire(empireId);
								if(!empireId)
									empireId = gd.JoinRandomEmpire(empireId);
								gd.EmpireCapital(empireId, destCity);
								if(g_Au.m_LogMessages & LOG_Admin)
								{
									QString logmsg = QString("Limbo Rescue: Transferring %1 %2 to capital %3 of random empire %4.")
										.arg(atomLoaded->Id().GetID()).arg(atomName).arg(destCity.GetID()).arg(empireId);
									g_Au.Log(logmsg);
								}
							}
						}
						g_Au.UnlockData();

						if(!destCity.IsNull())
						{
							// Package up the atom and send it as if it was an admin teleport.
							if(g_Au.m_LogMessages & LOG_Admin)
							{
								QString logmsg = QString("Limbo Rescue: Handing off %1 %2 to server hosting capital city %3.")
									.arg(atomLoaded->Id().GetID()).arg(atomLoaded->Name()).arg(destCity.GetID());
								g_Au.Log(logmsg);
							}

							// Bundle the departing atom and all of its subatoms into a byte array of its own.
							// It is put into an atom list because the atom list knows how to deserialize the atom properly.
							QByteArray atomData;
							DaAtomList list;
							loadEther.RemoveAtom(atomLoaded->Id(), list);
							QDataStream dst(&atomData, QIODevice::WriteOnly);
							dst.setVersion(QDataStream::Qt_4_1);
							dst << list;
							loadEther.TrashAtoms(list);

							// Do not attempt to rescue the destCity from limbo.
							emit HandoffElsewhere(*lai, destCity, rideAlso, false, atomData, AuId(0));
						}
						else
						{
							if(g_Au.m_LogMessages & LOG_Admin)
							{
								QString logmsg = QString("Limbo Rescue: Failed to locate a capital city to restore %1 %2. "
										"Sending them to a new home world.")
									.arg(atomLoaded->Id().GetID()).arg(atomLoaded->Name());
								g_Au.Log(logmsg);
							}
							// This will make a new home world for them.
							emit AtomSpawned(*lai, empireId, AuId(), AuId(), AuId(), galaxy);
						}
					}
				}
				else
				{
					// The person atom was not loaded.
					qa.finish();
					if(g_Au.m_LogMessages & LOG_Admin)
					{
						QString logmsg = QString("Limbo Rescue: Person was not found in the database; atom %1 may not be a person.")
							.arg((*lai).GetID());
						g_Au.Log(logmsg);
					}
				}
			}
			db.close();	// We don't keep the real database open.
		}
	}
	else if(g_Au.IsSceneManager())
	{
		// Scene Manager loads assigned atoms as they are assigned.
		// Grab a copy of the current list of sector ids assigned to this server.
		unordered_map<AuId, AuMySector, AuIdHash> idSectors;						// Sectors assigned to me.
		auSceneServerApp->ListSectorsAssignedToMe(idSectors);

		// Test for wormhole reload.
		if(m_eDbStatus == Loaded && idSectors.empty())
			return;	// Nothing to do. When NotLoaded, must load Universe and Galaxy.

		// Open the database before locking the root mutex.
		QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
		if(!OpenDatabase(db))
			return;	// Failed to open database?

		// SceneMaster and SceneWorker load Universe and galaxies only once.
		// If they can't be loaded, we'll return and try again later.
		if(m_eDbStatus == NotLoaded)
		{
			// We cannot accept assigned atoms until we have a universe.
			// Load the known Universe and galaxies from the database.
			g_Au.ClearEtherium();	// Start by clearing the current ether.
			AuEther& ether = g_Au.LockEtheriumDelete();
			DaUniverse* universe = LoadGalaxies(db, ether);
			g_Au.UnlockEtheriumDelete();
			if(!universe)
			{
				// No Universe? Try again later.
				db.close();	// We don't keep the real database open.
				return;
			}

			// Go to 'Loaded' state since the universe loaded ok. All other atoms come and go as assigned/needed.
			m_eDbStatus = Loaded;
			emit FinishedInit();
		}

		// Check to see if anything has changed, as in solar system assigned/deassigned.
		m_LoadedLock.lock();
		AuIdList loaded(m_Loaded);
		m_LoadedLock.unlock();
		bool anyChange = false;
		if((unsigned int)loaded.size() != idSectors.size())
			anyChange = true;
		else
		{
			// The list of loaded systems and the list of assigned systems have the same number of entries.
			// Check each system id in case one left and another arrived in the same time period.
			for(AuIdList::const_iterator sli = loaded.cbegin(); sli != loaded.cend(); sli++)
				if(idSectors.find(*sli) == idSectors.end())
				{
					anyChange = true;
					break;
				}
		}

		// Load each individual sector that is assigned to me.
		AuIdList assignedSectorsLoaded;
		if(anyChange)
		{
			// There were changes to the list of sectors assigned to me.
			loaded.clear();		// We will update this entire list.
			for(unordered_map<AuId, AuMySector, AuIdHash>::const_iterator it = idSectors.cbegin();
					it != idSectors.cend() && Alive(); it++)
			{
				AuEther& ether = g_Au.LockEtheriumDelete();
				DaUniverse* universe = ether.FindUniverse();
				if(universe)
				{
					DaSector* sector = dynamic_cast<DaSector*>(ether.FindNormalAtom((*it).first));
					if(!sector)
					{
						// The sector has not been loaded yet.
						DaGalaxy* galaxy = universe->FindGalaxy((*it).second.Galaxy());
						if(galaxy)
						{
							// Try to load it. We may have to try again later if it hasn't shown up in the database yet. This
							// shouldn't happen since the master is careful not to assign nu sectors until they have been saved in
							// the database. See the usage of m_NewSectors.
							// When loading as SceneManager, the solar systems are loaded without any contents. That is enough to
							// determine if a system is final and know its genhab type.
							AuEther tempEther(__FILE__, __LINE__);	// Temporary ether so we can add atoms to the etherium en masse.
							sector = DaSector::LoadSubatoms(tempEther, db, galaxy->Id(), LOAD_Systems, (*it).first);
							if(sector)
							{
								// The sector was loaded.
								// Did we load the appropriate number of solar systems? For some unknown reason, even though the
								// master waits to notify us until the store transaction has been committed AND the database closed,
								// we load our assigned sector and don't get all of the solar systems, though they do show up in the
								// database at some point. D.H.H. 3/19/2013
								if(sector->SolarSystemCount() < (*it).second.m_Systems)
								{
									qWarning() << "DtDb::LoadAssignedAtoms Discarding sector" << sector->Id() << sector->Name()
										<< "loaded with only" << sector->SolarSystemCount() << "of" << (*it).second.m_Systems
										<< "systems. Trying again in 1 sec.";
									sector = 0;
								}
								else
								{
									// The assigned sector loaded with all of its solar systems.
									if(sector->SolarSystemCount() > (*it).second.m_Systems)
										qWarning() << "DtDb::LoadAssignedAtoms Sector" << sector->Id() << sector->Name()
											<< "loaded with" << sector->SolarSystemCount() << "of" << (*it).second.m_Systems
											<< "systems. That's too many but I'm going with it.";
									galaxy->AddSectorLoc(sector->Id(), sector->Location());

									// Transfer the sector and its subatoms from the temporary ether to the etherium.
									DaAtomList sectorSubs;
									tempEther.RemoveAtom(sector->Id(), sectorSubs);
									ether.AddAtoms(sectorSubs);
									assignedSectorsLoaded.append(sector->Id());
								}
							}
						}
					}
					if(sector)
						loaded.append(sector->Id());
				}
				g_Au.UnlockEtheriumDelete();
				msleep(1);				// Yield to allow solar systems to get assigned readily.
			}

			m_LoadedLock.lock();
			m_Loaded = loaded;
			m_LoadedLock.unlock();
		}
		db.close();	// We don't keep the real database open.

		// Notify the app of any assigned sectors that were just loaded, for arriving spawn atoms that may be waiting for a planet.
		for(AuIdList::const_iterator lsi = assignedSectorsLoaded.cbegin(); lsi != assignedSectorsLoaded.cend(); lsi++)
			emit AssignedSectorLoaded(*lsi);
	}
	else
	{
		// Scene Worker loads assigned atoms as they are assigned.
		// Grab a copy of the current list of system ids assigned to this server.
		QMap<AuId, AuMySystem> idSystems;	// Solar systems assigned to me. Key is system id, value is sector id.
		auSceneServerApp->ListSolarSystemsAssignedToMe(idSystems);

		// Test for atom assignments.
		// When dbStatus is NotLoaded, continue on to load Universe and Galaxy.
		if(m_eDbStatus == Loaded && idSystems.isEmpty())
			return;	// Database is loaded but no systems are assigned yet. Wait for a system assignment before doing anything.

		// Open the database before locking the root mutex.
		QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
		if(!OpenDatabase(db))
			return;	// Failed to open database?

		// SceneWorker loads Universe and galaxies only once.
		// If they can't be loaded, we'll return and try again later.
		if(m_eDbStatus == NotLoaded)
		{
			// We cannot accept assigned atoms until we have a universe.
			// Load the known Universe and galaxies from the database.
			g_Au.ClearEtherium();	// Start by clearing the current ether.
			AuEther& ether = g_Au.LockEtheriumDelete();
			DaUniverse* universe = LoadGalaxies(db, ether);
			g_Au.UnlockEtheriumDelete();
			if(!universe)
			{
				// No Universe? Try again later.
				db.close();	// We don't keep the real database open.
				return;
			}

			// Go to 'Loaded' state since the universe loaded ok. All other atoms come and go as assigned/needed.
			m_eDbStatus = Loaded;
			emit FinishedInit();
		}

		// Check to see if anything has changed, as in solar system assigned/deassigned.
		m_LoadedLock.lock();
		AuIdList loaded(m_Loaded);
		m_LoadedLock.unlock();
		bool anyChange = false;
		if(loaded.count() != idSystems.count())
			anyChange = true;
		else
		{
			// The list of loaded systems and the list of assigned systems have the same number of entries.
			// Check each system id in case one left and another arrived in the same time period.
			for(AuIdList::const_iterator sli = loaded.cbegin(); sli != loaded.cend(); sli++)
				if(!idSystems.contains(*sli))
				{
					anyChange = true;
					break;
				}
		}

		QMap<AuId, AuPoint> mySectors[GALAXY_Count];	// Sectors containing assigned systems. Key is sector; value is sector loc.
		AuEther tempEther(__FILE__, __LINE__);			// Temporary ether for solar systems that we load.
		AuIdList sectorsStarField;		// Sectors that are loaded for starfield only.
		AuIdList assignedSystemsLoaded;
		//if(g_Au.m_LogMessages & LOG_Database)
			//qDebug() << "DtDb::LoadAssignedAtoms anyChange" << anyChange << "systems" << idSystems.count();
		if(anyChange)
		{
			// There were changes to the list of systems assigned to me.
			// Load minimum contents of assigned sectors and starfield first.
			// This is reasonably quick; then we can release the root mutex.
			loaded.clear();		// We will update this entire list.
			for(QMap<AuId, AuMySystem>::const_iterator it = idSystems.cbegin(); it != idSystems.cend() && Alive(); it++)
			{
				// First insure we have the sector loaded locally.
				AuEther& ether = g_Au.LockEtheriumDelete();
				DaUniverse* universe = ether.FindUniverse();
				if(universe)
				{
					DaSector* sector = dynamic_cast<DaSector*>(ether.FindNormalAtom((*it).m_Sector));
					if(!sector)
					{
						// The sector has not been loaded yet.
						DaGalaxy* galaxy = universe->FindGalaxy((*it).Galaxy());
						if(galaxy)
						{
							// Try to load it. We may have to try again later if it hasn't shown up in the database yet.
							// Scene Worker starts by loading enough of the sector to form the star field.
							//qDebug("DtDb::LoadAssignedAtoms loading sector %d for system %d", it.value().GetID(),
									//it.key().GetID());
							AuId galaxyId(galaxy->Id());
							g_Au.UnlockEtheriumDelete();
							AuEther loadEther(__FILE__, __LINE__);			// Destructor will delete any atoms that we load.
							sector = DaSector::LoadSubatoms(loadEther, db, galaxyId,
									(LOADSUBATOMS)(LOAD_Systems|LOAD_StarField), (*it).m_Sector);
							if(!sector)
								continue;

							g_Au.LockEtheriumDelete();
							universe = ether.FindUniverse();
							if(universe)
							{
								galaxy = universe->FindGalaxy((*it).Galaxy());
								if(galaxy)
								{
									DaAtomList satoms;
									loadEther.RemoveAtom(sector->Id(), satoms);	// Take the nuly loaded sector from the temp ether.
									ether.AddAtoms(satoms);		// Add the nuly loaded sector to the real etherium.
									galaxy->AddSectorLoc(sector->Id(), sector->Location());
									satoms.TickTock();
								}
								else
									qDebug() << "DtDb::LoadAssignedAtoms no Galaxy" << (*it).Galaxy()
										<< "after loading sector" << sector->Id() << sector->Name();
							}
							else
								qDebug() << "DtDb::LoadAssignedAtoms no Universe after loading sector"
									<< sector->Id() << sector->Name();
						}
					}

					mySectors[(*it).Galaxy()].insert(sector->Id(), sector->Location());

					// Check to see if the assigned system is already loaded fully.
					DaSolarSystem* exSystem = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(it.key()));
					if(exSystem && !exSystem->IsOnlyStarFieldLoaded())
						loaded.append(exSystem->Id());
				}
				g_Au.UnlockEtheriumDelete();
			}

			if(g_Au.m_LogMessages & LOG_Database)
				qDebug() << "DtDb::LoadAssignedAtoms load star field";

			// Load Star Field for each galaxy on worker server.
			for(int galaxyIdx=0; galaxyIdx < GALAXY_Count; galaxyIdx++)
			{
				// Now we have the minimum sectors loaded that contain the systems we've been assigned. In order for us to provide a
				// star field to the scene client, we need to load the sectors that form the perimeter of the assigned sectors. The
				// neighboring sectors are guaranteed to exist in the database for my use here. See AuSceneServerApp::AssignSectors.
				// We load all sectors because we only load pieces of the sectors that are assigned to us.
				//qDebug("DtDb::LoadAssignedAtoms - Star Field, %d assigned sectors", mySectors.count());
				AuPointArray starfieldsectors;
				for(QMap<AuId, AuPoint>::const_iterator it = mySectors[galaxyIdx].cbegin();
						it != mySectors[galaxyIdx].cend() && Alive(); it++)
				{
					// Add 26 sector locations to the list needed for the star field.
					for(int ix=-10; ix<=10; ix+=10)
						for(int iy=-10; iy<=10; iy+=10)
							for(int iz=-10; iz<=10; iz+=10)
							{
								if(ix || iy || iz)
								{
									AuPoint fn = *it + AuPoint(ix, iy, iz);
									starfieldsectors.AddIfUnique(fn);
								}
							}
				}

				// Now we have the minimum list of star field sectors to load. Entries may overlap sectors we already have loaded.
				//qDebug("DtDb::LoadAssignedAtoms - Star Field, %d star field sectors", starfieldsectors.count());
				for(AuPointArray::const_iterator it = starfieldsectors.cbegin(); it != starfieldsectors.cend() && Alive(); it++)
				{
					AuEther& ether = g_Au.LockEtheriumDelete();
					DaUniverse* universe = ether.FindUniverse();
					if(universe)
					{
						DaGalaxy* galaxy = universe->FindGalaxy((GALAXY)galaxyIdx);
						if(galaxy)
						{
							DaSector* sector = galaxy->FindSectorAt(*it);
							if(!sector)
							{
								// The star field sector has not been loaded yet.
								//qDebug("DtDb::LoadAssignedAtoms - Loading sector at %g,%g,%g", (*it).x, (*it).y, (*it).z);
								AuId galaxyId(galaxy->Id());
								g_Au.UnlockEtheriumDelete();
								AuEther loadEther(__FILE__, __LINE__);			// Destructor will delete any atoms that we load.
								sector = DaSector::LoadSubatoms(loadEther, db, galaxyId,
										(LOADSUBATOMS)(LOAD_Systems|LOAD_StarField), galaxyId, &(*it));	// Load sector at coordinate
								if(!sector)
									continue;

								g_Au.LockEtheriumDelete();
								universe = ether.FindUniverse();
								if(universe)
								{
									galaxy = universe->FindGalaxy((GALAXY)galaxyIdx);
									if(galaxy)
									{
										DaAtomList satoms;
										loadEther.RemoveAtom(sector->Id(), satoms);	// Take the nuly loaded sector
										ether.AddAtoms(satoms);		// Add the nuly loaded sector to the real etherium.
										galaxy->AddSectorLoc(sector->Id(), sector->Location());
										satoms.TickTock();
									}
									else
										qDebug() << "DtDb::LoadAssignedAtoms no Galaxy" << galaxyIdx
											<< "after loading star field sector" << sector->Id() << sector->Name();
								}
								else
									qDebug() << "DtDb::LoadAssignedAtoms no Universe after loading star field sector"
										<< sector->Id() << sector->Name();
							}
							//else
								//qDebug("DtDb::LoadAssignedAtoms - Sector at %g,%g,%g already loaded", (*it).x, (*it).y, (*it).z);

							if(!mySectors[galaxyIdx].contains(sector->Id()))
								sectorsStarField.AddIfUnique(sector->Id());		// Not an assigned sector so it must be starfield.
						}
					}
					g_Au.UnlockEtheriumDelete();
				}
			}
			if(g_Au.m_LogMessages & LOG_Database)
				qDebug("DtDb::LoadAssignedAtoms - Star Field sectors loaded %d", sectorsStarField.count());

			// Now load assigned solar systems.
			// We have already loaded all the sectors that we need with their minimum contents.
			for(QMap<AuId, AuMySystem>::const_iterator it = idSystems.cbegin(); it != idSystems.cend() && Alive(); it++)
			{
				// Skip solar systems that are already loaded fully, not just for the star field.
				if(loaded.contains(it.key()))
					continue;

				// Load the solar system from the database without the root mutex locked.
				// Each solar system can take a long time to load.
				DaSolarSystem* system = DaSolarSystem::LoadSubatoms(tempEther, db, (*it).m_Sector, LOAD_All, it.key());
				if(system)
				{
					if(!system->IsFinal())
					{
						// The system loaded but it is not finalized yet.
						// Initial store must not be complete because the Scene Manager does not assign nonfinal systems to me.
						// Discard the loaded system and we'll try again next time around.
						if(g_Au.m_LogMessages & LOG_Travel)
							qDebug() << "DtDb::LoadAssignedAtoms discarded system" << system->Id() << system->Name()
								<< "not IsFinal";
						tempEther.TrashAtom(system->Id());	// Trash the system and its subatoms.
						system = 0;
					}
					else
					{
						loaded.append(system->Id());
						assignedSystemsLoaded.append(system->Id());
					}
				}
			}

			if(g_Au.m_LogMessages & LOG_Database)
				qDebug("DtDb::LoadAssignedAtoms - assigned systems loaded %d", loaded.count());
			m_LoadedLock.lock();
			m_Loaded = loaded;
			m_LoadedLock.unlock();
		}

		// Get the list of arriving traveler atoms.
		// Only the scene worker loads arriving atoms from the database.
		QList<AuTravelAtom> arrivals;
		QList<AuHengeTeleportAtom> arriveHengeTel;
		m_muAtomTransactions.lock();
		arrivals = m_ArrivingTravelers;
		m_ArrivingTravelers.clear();
		arriveHengeTel = m_ArrivingHengeTeleport;
		m_ArrivingHengeTeleport.clear();
		m_muAtomTransactions.unlock();

		if(!tempEther.Empty() || !arrivals.isEmpty() || !arriveHengeTel.isEmpty())
		{
			// Now that the systems are loaded, add them to the atom hierarchy.
			if(g_Au.m_LogMessages & LOG_Database)
				qDebug() << "DtDb::LoadAssignedAtoms add atoms" << idSystems.count();
			AuEther& ether = g_Au.LockEtheriumDelete();
			for(QMap<AuId, AuMySystem>::const_iterator it = idSystems.cbegin(); it != idSystems.cend() && Alive(); it++)
			{
				DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(tempEther.FindNormalAtom(it.key()));
				if(system)
				{
					DaSector* sector = dynamic_cast<DaSector*>(ether.FindNormalAtom((*it).m_Sector));
					if(sector)
					{
						DaSolarSystem* exSystem = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(it.key()));
						if(exSystem)
						{
							// The system is loaded but we only loaded enough for the star field.
							Q_ASSERT(exSystem->IsOnlyStarFieldLoaded());	// We already checked this a few lines above.
							// Delete the solar system because the data will be incomplete.
							// Then we'll replace it with the system we just loaded.
							sector->TrashSubatom(exSystem->Id());
						}
						DaAtomList ssubs;
						tempEther.RemoveAtom(system->Id(), ssubs);	// Take the system and all of its subatoms from the temp ether.
						if(ssubs.empty())
							continue;

						ether.AddAtoms(ssubs);		// Add the system and its atoms to the proper parent sector.

						// Update the last visited flag if there is a person online or an officer in the system.
						// People are stored with their online flag so we should pick it up here, before it decays.
						if(system->FindPersonOnLineRecursive())
							system->SetLastPersonVisited();
						if(system->FindOfficerRecursive() || system->FindAIPersonRecursive())
							system->SetLastOfficerVisited();

						ssubs.TickTock();
					}
					else
					{
						if(g_Au.m_LogMessages & LOG_Error)
							qWarning("ERROR! DtDb::LoadAssignedAtoms - Discarded %d...sector not loaded.", system->Id().GetID());
						tempEther.TrashAtom(system->Id());
					}
				}
			}

			// Load arriving atoms last.
			// These are atoms that have traveled from one system to another via wormhole or deadhead.
			// These are also atoms that are spawning here.
			if(g_Au.m_LogMessages & LOG_Travel)
				if(!arrivals.isEmpty())
					qDebug() << "DtDb::LoadAssignedAtoms " << arrivals.count() << "atoms arrived";

			for(QList<AuTravelAtom>::const_iterator it = arrivals.cbegin(); it != arrivals.cend(); it++)
			{
				// Locate the destination solar system of the arriving atom.
				DaSector* sector = dynamic_cast<DaSector*>(ether.FindNormalAtom((*it).m_DestSector));
				if(sector)
				{
					DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom((*it).m_DestSystem));
					if(system && !system->IsOnlyStarFieldLoaded())
					{
						// An atom arrived, destined for a solar system that I have loaded completely, which implies the
						// system is assigned to me.

						// The atom may already be here. This happens when a ship dead-heads to a nu solar system. It is set up in
						// the db with the destination system as its parent and then the travel signal is emitted. A problem occurs
						// when that nu system is loaded by a worker, since it was not already loaded being nu. The ship is
						// loaded with the system and continues on on its way. Then we receive the travel signal. By now the ship
						// could have moved enough to dead-head to another solar system and it's about to depart. So, if that atom
						// is already here and not dead, we will ignore this notification.
						const DaAtom* trav = ether.FindLiveAtom((*it).m_Traveler);
						if(trav)
						{
							// Traveler is already here and not dead.
							// What if their solar system is loaded for starfield only?
							DaSolarSystem* travSystem = trav->Parent()->FindParentSolarSystem();
							if(travSystem && travSystem->IsOnlyStarFieldLoaded())
							{
								// The traveler that is already here is in a starfield only solar system. It's probabably not
								// supposed to be there any more. Remove it to receive the inbound traveler.
								if(g_Au.m_LogMessages & LOG_Travel)
								{
									QString logmsg = QString("Atom arrived but its already here: %1 %2 in STARFIELD ONLY system "
											"%3 %4, arrived in system %5 %6. Removing existing atom to receive the traveler.")
										.arg(trav->Id().GetID()).arg(trav->Name())
										.arg(travSystem->Id().GetID()).arg(travSystem->Name())
										.arg(system->Id().GetID()).arg(system->Name());
									g_Au.Log(logmsg);
								}
								trav->Parent()->TrashSubatom(trav->Id());
								trav = 0;
							}
							else
							{
								if(g_Au.m_LogMessages & LOG_Travel)
								{
									QString logmsg = QString("Atom arrived but its already here: %1 %2 in system %3 %4, "
											"arrived in system %5 %6. Ignoring arrival notification.")
										.arg(trav->Id().GetID()).arg(trav->Name())
										.arg(trav->FindParentSolarSystem()->Id().GetID()).arg(trav->FindParentSolarSystem()->Name())
										.arg(system->Id().GetID()).arg(system->Name());
									g_Au.Log(logmsg);
								}

								// Check to see if that arriving traveler was a person on line or an officer.
								if(trav->FindPersonOnLineRecursive())
									system->SetLastPersonVisited();
								if(trav->FindOfficerRecursive() || trav->FindAIPersonRecursive())
									system->SetLastOfficerVisited();

								// Notify the system of the quality of non-pirate ships. We will use QualityMade because the
								// ship design may not be analyzed yet; HighestSystemQuality would be slightly preferable.
								const DaVessel* ship = dynamic_cast<const DaVessel*>(trav);
								if(ship && !ship->IsPirate())
									system->SetNonPirateShipPresent(ship->QualityMade());
								continue;
							}
						}

						// The only atoms at present that move from one solar system to another are people, vehicles and
						// space ships, far more likely the latter. People are usually in space ships or vehicles when the
						// move occurs, which means they don't come through here. A person would have to be at exactly the
						// boundary between two solar systems in deep space to traverse systems outside a ship.
						// Items and Npcs just get lost when traversing a solar system boundary.
						// This code also handles spacecraft and people who are supposed to spawn here. Their respective
						// LoadSubatoms functions handle the finishing touches there.
						AuEther tempEther(__FILE__, __LINE__);
						DaAtom* arrived = DaSc::LoadSubatoms(tempEther, db, system->Id(), (*it).m_Traveler);
						if(!arrived)
						{
							// The arrived atom was not a space ship. Try loading a vehicle with that id.
							arrived = DaVehicle::LoadSubatoms(tempEther, db, system->Id(), (*it).m_Traveler);
							if(!arrived)
							{
								// The arrived atom was not a vehicle. Try loading a person with that id.
								arrived = DaPerson::LoadSubatoms(tempEther, db, system->Id(), (*it).m_Traveler);
							}
						}

						if(arrived)
						{
							if(g_Au.m_LogMessages & LOG_Travel)
							{
								QString logmsg = QString("Atom arrived: %1 %2 in system %3 %4 at (%5, %6, %7)")
									.arg(arrived->Id().GetID()).arg(arrived->Name())
									.arg(system->Id().GetID()).arg(system->Name())
									.arg(arrived->Location().x, 0,'f',3)
									.arg(arrived->Location().y, 0,'f',3)
									.arg(arrived->Location().z, 0,'f',3);
								g_Au.Log(logmsg);
							}

							arrived->SetModified();					// Trigger save of arriving atom with nu parent id.
							DaAtomList arratoms;
							tempEther.RemoveAtom(arrived->Id(), arratoms);
							ether.AddAtoms(arratoms);

							if(g_Au.m_LogMessages & LOG_Travel)
							{
								QString logmsg = QString("Updating LastVisited for person %1 officer %2 demiavatar %3")
									.arg(system->FindPersonOnLineRecursive()?true:false)
									.arg(system->FindOfficerRecursive()?true:false)
									.arg(system->FindAIPersonRecursive()?true:false);
								g_Au.Log(logmsg);
							}

							// We must kick this if a bare person arrives so people can finish spawn recalling, especially when
							// they are auto recalled due to drifting in deep space.
							if(system->FindPersonOnLineRecursive() || dynamic_cast<const DaPerson*>(arrived))
								system->SetLastPersonVisited();
							if(system->FindOfficerRecursive() || system->FindAIPersonRecursive())
								system->SetLastOfficerVisited();

							// Notify the system of the quality of non-pirate ships. We will use QualityMade because the ship
							// design may not be analyzed yet; HighestSystemQuality would be slightly preferable.
							const DaVessel* ship = dynamic_cast<const DaVessel*>(arrived);
							if(ship && !ship->IsPirate())
								system->SetNonPirateShipPresent(ship->QualityMade());

							arratoms.TickTock();

							// This needs to trigger a database update right away. Otherwise a revokation of the system could lose
							// the data about the arrivals before I ever get around to saving them.
							if(g_Au.m_LogMessages & LOG_Travel)
							{
								QString logmsg = QString("Calling PostCriticalUpdates");
								g_Au.Log(logmsg);
							}
							g_AuDb.PostCriticalUpdates();
							if(g_Au.m_LogMessages & LOG_Travel)
							{
								QString logmsg = QString("Call to PostCriticalUpdates done");
								g_Au.Log(logmsg);
							}

							// Sleep between arrivals so maybe they won't arrive on top of each other.
							if(it != arrivals.cend()-1)
								msleep(2000);
						}
						else
						{
							if(g_Au.m_LogMessages & LOG_Error)
								qWarning("ERROR! DtDb::LoadAssignedAtoms - Ignoring arriving atom %d...NOT FOUND IN DB.",
										(*it).m_Traveler.GetID());
						}
					}
					else
					{
						// Repost the atom travel message. It may need to go to another server.
						if(g_Au.m_LogMessages & LOG_Travel)
						{
							if(system)
								qDebug("DtDb::LoadAssignedAtoms - Reposting arriving traveler %d...solar system %d is starfield "
										"only.", (*it).m_Traveler.GetID(), (*it).m_DestSystem.GetID());
							else
								qDebug("DtDb::LoadAssignedAtoms - Reposting arriving traveler %d...solar system %d not found.",
										(*it).m_Traveler.GetID(), (*it).m_DestSystem.GetID());
						}
						emit AtomTraveled((*it).m_Traveler, (*it).m_DestSystem, (*it).m_DestSector);
					}
				}
				else
				{
					// Repost the atom travel message. It may need to go to another server.
					if(g_Au.m_LogMessages & LOG_Travel)
						qDebug("DtDb::LoadAssignedAtoms - Reposting arriving traveler %d...sector %d not found.",
								(*it).m_Traveler.GetID(), (*it).m_DestSector.GetID());
					emit AtomTraveled((*it).m_Traveler, (*it).m_DestSystem, (*it).m_DestSector);
				}
			}

			if(g_Au.m_LogMessages & LOG_Travel)
				if(!arriveHengeTel.isEmpty())
					qDebug() << "DtDb::LoadAssignedAtoms " << arriveHengeTel.count() << "henge teleporters arrived";

			for(QList<AuHengeTeleportAtom>::const_iterator it = arriveHengeTel.cbegin(); it != arriveHengeTel.cend(); it++)
			{
				// Locate the destination solar system of the arriving atom.
				DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom((*it).m_DestSysId));
				if(system && !system->IsOnlyStarFieldLoaded())
				{
					// An atom arrived, destined for a solar system that I have loaded completely, which implies the
					// system is assigned to me.

					// The atom may already be here. This happens when a ship dead-heads to a nu solar system. It is set up in
					// the db with the destination system as its parent and then the travel signal is emitted. A problem occurs
					// when that nu system is loaded by a worker, since it was not already loaded being nu. The ship is
					// loaded with the system and continues on on its way. Then we receive the travel signal. By now the ship
					// could have moved enough to dead-head to another solar system and it's about to depart. So, if that atom
					// is already here and not dead, we will ignore this notification.
					const DaAtom* trav = ether.FindLiveAtom((*it).m_TeleportId);
					if(trav)
					{
						// Traveler is already here and not dead.
						// What if their solar system is loaded for starfield only?
						DaSolarSystem* travSystem = trav->Parent()->FindParentSolarSystem();
						if(travSystem && travSystem->IsOnlyStarFieldLoaded())
						{
							// The traveler that is already here is in a starfield only solar system. It's probabably not
							// supposed to be there any more. Remove it to receive the inbound traveler.
							if(g_Au.m_LogMessages & LOG_Travel)
							{
								QString logmsg = QString("Atom arrived by henge teleport but its already here: "
										"%1 %2 in STARFIELD ONLY system "
										"%3 %4, arrived in system %5 %6. Removing existing atom to receive the traveler.")
									.arg(trav->Id().GetID()).arg(trav->Name())
									.arg(travSystem->Id().GetID()).arg(travSystem->Name())
									.arg(system->Id().GetID()).arg(system->Name());
								g_Au.Log(logmsg);
							}
							trav->Parent()->TrashSubatom(trav->Id());
							trav = 0;
						}
						else
						{
							if(g_Au.m_LogMessages & LOG_Travel)
							{
								QString logmsg = QString("Atom arrived by henge teleport but its already here: "
										"%1 %2 in system %3 %4, arrived in system %5 %6. Ignoring arrival notification.")
									.arg(trav->Id().GetID()).arg(trav->Name())
									.arg(trav->FindParentSolarSystem()->Id().GetID()).arg(trav->FindParentSolarSystem()->Name())
									.arg(system->Id().GetID()).arg(system->Name());
								g_Au.Log(logmsg);
							}

							// Check to see if that arriving traveler was a person on line or an officer.
							if(trav->FindPersonOnLineRecursive())
								system->SetLastPersonVisited();
							if(trav->FindOfficerRecursive() || trav->FindAIPersonRecursive())
								system->SetLastOfficerVisited();

							continue;
						}
					}

					// Only creatures can use henge portals.
					AuEther tempEther(__FILE__, __LINE__);
					DaAtom* arrived = DaPerson::LoadSubatoms(tempEther, db, system->Id(), (*it).m_TeleportId);
					if(!arrived)
					{
						arrived = DaNpc::LoadSubatoms(tempEther, db, system->Id(), (*it).m_TeleportId);
						if(!arrived)
							arrived = DaAnimal::LoadSubatoms(tempEther, db, system->Id(), (*it).m_TeleportId);
					}

					if(arrived)
					{
						if(g_Au.m_LogMessages & LOG_Travel)
						{
							QString logmsg = QString("Henge teleport arrived: %1 %2 in system %3 %4")
								.arg(arrived->Id().GetID()).arg(arrived->Name())
								.arg(system->Id().GetID()).arg(system->Name());
							g_Au.Log(logmsg);
						}

						DaAtomList arratoms;
						tempEther.RemoveAtom(arrived->Id(), arratoms);
						ether.AddAtoms(arratoms);

						// Locate the destination henge portal.
						system->HengeTeleportArrived(arrived, (*it).m_DestSun, (*it).m_DestArc, (*it).m_FromSysId, (*it).m_FromSun,
								(*it).m_FromArc);

						arrived->SetModified();					// Trigger save of arriving atom with nu parent id.
						arrived->SetInScene();					// Keep wild animals alive a bit longer.

						// Insure subatoms that show up subordinate to the atom that arrived are not already in my data.
						DaAtomList subatoms;
						arrived->ListSubatomsRecursive(subatoms);
						for(DaAtomList::const_iterator pi = subatoms.cbegin(); pi != subatoms.cend(); pi++)
						{
							DaAtom* atom = *pi;

							// Delete old atom (if any) to replace it with fresh data.
							ether.TrashAtom(atom->Id());

							// Replace the nu atom subordinate to its proper parent.
							ether.AddAtom(atom);
							if(g_Au.m_LogMessages & LOG_Travel)
							{
								QString logmsg = QString(" Subatom arrived: %1 %2 aboard %3 %4")
									.arg(atom->Id().GetID()).arg(atom->Name())
									.arg(atom->ParentId().GetID()).arg(atom->Parent()->Name());
								g_Au.Log(logmsg);
							}
						}

						arratoms.TickTock();

						if(g_Au.m_LogMessages & LOG_Travel)
						{
							QString logmsg = QString("Updating LastVisited for person %1 officer %2 demiavatar %3")
								.arg(system->FindPersonOnLineRecursive()?true:false)
								.arg(system->FindOfficerRecursive()?true:false)
								.arg(system->FindAIPersonRecursive()?true:false);
							g_Au.Log(logmsg);
						}

						// We must kick this if a bare person arrives so people can finish spawn recalling, especially when
						// they are auto recalled due to drifting in deep space.
						if(system->FindPersonOnLineRecursive() || dynamic_cast<const DaPerson*>(arrived))
							system->SetLastPersonVisited();
						if(system->FindOfficerRecursive() || system->FindAIPersonRecursive())
							system->SetLastOfficerVisited();

						// This needs to trigger a database update right away. Otherwise a revokation of the system could lose
						// the data about the arrivals before I ever get around to saving them.
						if(g_Au.m_LogMessages & LOG_Travel)
						{
							QString logmsg = QString("Calling PostCriticalUpdates");
							g_Au.Log(logmsg);
						}
						g_AuDb.PostCriticalUpdates();
						if(g_Au.m_LogMessages & LOG_Travel)
						{
							QString logmsg = QString("Call to PostCriticalUpdates done");
							g_Au.Log(logmsg);
						}
					}
					else
					{
						if(g_Au.m_LogMessages & LOG_Error)
							qWarning("ERROR! DtDb::LoadAssignedAtoms - Ignoring arriving atom %d...NOT FOUND IN DB.",
									(*it).m_TeleportId.GetID());
					}
				}
				else
				{
					// Repost the henge teleport message. It may need to go to another server.
					if(g_Au.m_LogMessages & LOG_Travel)
					{
						if(system)
							qDebug("DtDb::LoadAssignedAtoms - Reposting henge teleport %d...solar system %d is starfield "
									"only.", (*it).m_TeleportId.GetID(), (*it).m_DestSysId.GetID());
						else
							qDebug("DtDb::LoadAssignedAtoms - Reposting henge teleport %d...solar system %d not found.",
									(*it).m_TeleportId.GetID(), (*it).m_DestSysId.GetID());
					}

					emit AtomHengeTeleported(*it);
				}
			}
			g_Au.UnlockEtheriumDelete();
		}
		db.close();	// We don't keep the real database open.

		//if(g_Au.m_LogMessages & LOG_Database)
			//qDebug() << "DtDb::LoadAssignedAtoms assignedSystemsLoaded" << assignedSystemsLoaded.count();
		// Notify the app of any assigned systems that were just loaded, for arriving spawn atoms that may be waiting for a planet.
		/*
		if((g_Au.m_LogMessages & LOG_Travel) && assignedSystemsLoaded.count())
		{
			QString logmsg = QString("emit AssignedSolarSystemLoaded signals for %1 assignedSystemsLoaded")
				.arg(assignedSystemsLoaded.count());
			g_Au.Log(logmsg);
		}
		*/
		for(AuIdList::const_iterator lsi = assignedSystemsLoaded.cbegin(); lsi != assignedSystemsLoaded.cend(); lsi++)
			emit AssignedSolarSystemLoaded(*lsi);
		/*
		if((g_Au.m_LogMessages & LOG_Travel) && assignedSystemsLoaded.count())
		{
			QString logmsg = QString("Done emitting AssignedSolarSystemLoaded signals");
			g_Au.Log(logmsg);
		}
		*/
		//if(g_Au.m_LogMessages & LOG_Database)
			//qDebug() << "DtDb::LoadAssignedAtoms done";
	}
}

// Load empire records that have changed.
void DtDb::LoadChangedEmpires()
{
	// The scene master does NOT do this because it is the only server that
	// changes empire records.
	if(g_Au.IsSceneMaster())
		return;

	m_muEmpireTransactions.lock();
	if(m_ChangedEmpireIds.isEmpty())
	{
		m_muEmpireTransactions.unlock();
		return;	// No empire record changes to load.
	}

	QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
	if(!OpenDatabase(db))
	{
		m_muEmpireTransactions.unlock();
		return;	// Failed to open database?
	}

	// Grab a copy of the changed ids so we can unlock the mutex.
	QList<quint32> chgIds(m_ChangedEmpireIds);
	m_ChangedEmpireIds.clear();	// Clear the list so we can repost failed loads.
	m_muEmpireTransactions.unlock();

	for(QList<quint32>::const_iterator it = chgIds.cbegin(); it != chgIds.cend(); it++)
	{
		DgEmpire emp;
		if(emp.Select(db, *it))
		{
			// Add/Update the in-memory copy of the empire record.
			//qDebug("DtDb::LoadChangedEmpires loaded empire %d", *it);
			DgData& gd = g_Au.LockDataForWrite();
			emp.ForgetForgottenEmpires(gd.m_gEmpires);			// Just in case some empire it knows about has been forgotten.
			gd.ReplaceEmpire(emp);
			g_Au.UnlockData();
			emit EmpireRecordLoaded(emp.m_id);
		}
		else
		{
			// Repost the change notification so we'll retry the load again
			// next time.
			// Perhaps the record hasn't landed in the database yet.
			// Empire records are never deleted so we don't have to worry
			// about that.
			qWarning("DtDb::LoadChangedEmpire - Failed to load id %d", *it);
			quint32 emp = *it;
			if(emp > 0 && emp != (quint32)-1)
				UpdateEmpire(emp);
		}
	}

	db.close();
}

// Load empire records that have changed.
void DtDb::LoadChangedSpecies()
{
	// The scene master does NOT do this because it is the only server that changes specie records.
	if(g_Au.IsSceneMaster())
		return;

	m_muSpecieTransactions.lock();
	if(m_ChangedSpecies.isEmpty())
	{
		m_muSpecieTransactions.unlock();
		return;	// No specie record changes to load.
	}

	QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
	if(!OpenDatabase(db))
	{
		m_muSpecieTransactions.unlock();
		return;	// Failed to open database?
	}

	// Grab a copy of the changed ids so we can unlock the mutex.
	QList<AuSpecies> chgIds(m_ChangedSpecies);
	m_ChangedSpecies.clear();	// Clear the list so we can repost failed loads.
	m_muSpecieTransactions.unlock();

	for(QList<AuSpecies>::const_iterator it = chgIds.cbegin(); it != chgIds.cend(); it++)
	{
		DzSpecies species;
		if(species.Select(db, *it))
		{
			// Add/Update the in-memory copy of the specie record.
			DgData& gd = g_Au.LockDataForWrite();
			gd.ReplaceSpecies(species);
			g_Au.UnlockData();
			emit SpeciesRecordLoaded(*it);
		}
		else
		{
			// Repost the change notification so we'll retry the load again next time.
			// Perhaps the record hasn't landed in the database yet.
			// Specie records are never deleted so we don't have to worry about that.
			qWarning() << "DtDb::LoadChangedSpecies - Failed to load species" << (*it).NameOrDesc();
			UpdateSpecies(*it);
		}
	}

	db.close();
}

// Load story records that have changed.
void DtDb::LoadChangedStories()
{
	// The scene worker only needs to do this.
	if(!g_Au.IsSceneWorker())
		return;

	g_AuDb.LockDbStoryIds();
	if(g_AuDb.m_ReloadStoryIds.isEmpty() && g_AuDb.m_RemoveStoryIds.isEmpty())
	{
		g_AuDb.UnlockDbStoryIds();
		return;	// No story records to load or delete.
	}

	QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
	if(!OpenDatabase(db))
	{
		g_AuDb.UnlockDbStoryIds();
		return;	// Failed to open database?
	}

	// Grab a copy of the changed ids so we can unlock the mutex.
	QList<quint32> chgIds(g_AuDb.m_ReloadStoryIds);
	g_AuDb.m_ReloadStoryIds.clear();	// Clear the list so we can repost failed loads.
	QList<quint32> delIds(g_AuDb.m_RemoveStoryIds);
	g_AuDb.m_RemoveStoryIds.clear();
	g_AuDb.UnlockDbStoryIds();

	//if(!chgIds.isEmpty())
		//qDebug() << "DtDb::LoadChangedStories chgIds" << chgIds.count();
	for(QList<quint32>::const_iterator it = chgIds.cbegin(); it != chgIds.cend(); it++)
	{
		AuStory* story = new AuStory(*it);
		if(story->Select(db))
		{
			// Add/Update the in-memory copy of the story record.
			AuStories* stories = g_AuDb.LockStoriesForWrite();

			AuStories::iterator si = stories->find(*it);
			if(si != stories->end())
			{
				delete (*si).second;
				(*si).second = story;
			}
			else
				stories->insert(pair<unsigned int, AuStory*>(*it, story));

			g_AuDb.UnlockStories();
		}
		else
		{
			//DHH Failed to select story record. Has it been deleted?
			// Repost the change notification so we'll retry the load again
			// next time.
			// Perhaps the record hasn't landed in the database yet.
			delete story;
			qWarning("DtDb::LoadChangedStories - Failed to load story id %d", *it);
			//DHH g_Au.ReloadStory(*it);	// This is really annoying. Get rid of it instead.
		}
	}

	if(!delIds.isEmpty())
	{
		// Remove the in-memory copy of the deleted story records.
		AuStories* stories = g_AuDb.LockStoriesForWrite();
		for(QList<quint32>::const_iterator it = delIds.cbegin(); it != delIds.cend(); it++)
		{
			AuStories::iterator si = stories->find(*it);
			if(si != stories->end())
			{
				delete (*si).second;
				stories->erase(si);
			}
		}
		g_AuDb.UnlockStories();
	}

	db.close();
}

// Construct the AuCity arrays in all the empire records by querying the database.
// NOTE: This function must not be called on a running server farm.
void DtDb::InitEmpireCities(DgData& gd, QSqlDatabase& db)
{
	// Clear current cities data since this function reconstructs it entirely from scratch.
	unordered_map<AuId, quint32, AuIdHash> sectorCapitols;	// Preserve sector capital status. This is the only place it is stored.
	unordered_map<AuId, quint32, AuIdHash> galaxyCapitols;	// Preserve galaxy capital status. This is the only place it is stored.
	for(DgEmpireHash::iterator eit = gd.m_gEmpires.begin(); eit != gd.m_gEmpires.end(); eit++)
	{
		for(AuCityHash::const_iterator eci = (*eit).second.m_Cities->cbegin(); eci != (*eit).second.m_Cities->cend(); eci++)
		{
			if((*eci).second.IsGalaxyCapital())
				galaxyCapitols.insert(pair<AuId, quint32>((*eci).first, (*eit).second.m_id));
			if((*eci).second.IsSectorCapital())
				sectorCapitols.insert(pair<AuId, quint32>((*eci).first, (*eit).second.m_id));
		}
		(*eit).second.m_Cities->clear();
	}

	// Remember empires that change so we can propagate a notification down the server chain.
	QMap<quint32,bool> empiresChanged;

	// Note that a city is a DaBd that is an IID_Capitol or IID_BaseHeadquarters building.
	unordered_map<AuId, quint32, AuIdHash> allCities;							// All cities in the database; value is empire id.
	{
		// Prepare a query to select all of the capitol building records in the database so we can construct the AuCity data array.
		QString sq;
		sq = QString("SELECT id,idp,name,cityName,allegiance,industry,designRpt FROM a_Bd WHERE industry=%1 OR industry=%2;")
			.arg(IID_Capitol)
			.arg(IID_BaseHeadquarters);
		QSqlQuery qryBd(QString(), db);
		qryBd.prepare(sq);
		qryBd.exec();
		while(qryBd.next())
		{
			int qv=0;
			AuId idBd = AuId(qryBd.value(qv++).toUInt());
			AuId idPlanet = AuId(qryBd.value(qv++).toUInt());
			QString name = qryBd.value(qv++).toString();
			QString sCity = qryBd.value(qv++).toString();
			quint32 empireId = qryBd.value(qv++).toUInt();
			if(!empireId)
			{
				// This shouldn't happen.
				if(g_Au.m_LogMessages & LOG_Error)
					qWarning() << "error: DtDb::InitEmpireCities empireless Bd" << idBd << sCity << "planet" << idPlanet;
				continue;
			}
			INDUSTRYID iid = (INDUSTRYID)qryBd.value(qv++).toInt();

			AuId founderId;
			QString founderName;
			QByteArray ba = qryBd.value(qv++).toByteArray();
            if(!ba.isEmpty())
            {
                // Design report
                QDataStream in(ba);
                in.setVersion(QDataStream::Qt_4_1);
				AuBdReport design;
                in >> design;
                quint8 ver;
                in >> ver;
                if(ver >= 1)
                {
                    in >> founderId;
                    in >> founderName;
                }
            }

			CITYTYPE cityType;
			if(iid == IID_Capitol)
			{
				cityType = CITY_Capitol;
				if(sCity.isEmpty())
					sCity = QObject::tr("%1 City").arg(idBd.StringId());
			}
			else if(iid == IID_BaseHeadquarters)
			{
				cityType = CITY_MilitaryBase;
				if(sCity.isEmpty())
					sCity = QObject::tr("%1 Base").arg(idBd.StringId());
			}
			else
				continue;	// wut?


			// Restore sector capital flag if the city is in our hash table and the empire id matches.
			bool isGalaxyCapital = false;
			unordered_map<AuId, quint32, AuIdHash>::const_iterator gci = galaxyCapitols.find(idBd);
			if(gci != galaxyCapitols.end() && (*gci).second == empireId)
				isGalaxyCapital = true;

			// Restore sector capital flag if the city is in our hash table and the empire id matches.
			bool isSectorCapital = false;
			unordered_map<AuId, quint32, AuIdHash>::const_iterator sci = sectorCapitols.find(idBd);
			if(sci != sectorCapitols.end() && (*sci).second == empireId)
				isSectorCapital = true;

			// Perform a subquery to get information about the planet where the city resides.
			QString sPlanet, sSystem, sSector;
			AuId idSystem, idSector, idGalaxy;
			GALAXY galaxy = GALAXY_Count;
			double secX = 0;
			double secY = 0;
			double secZ = 0;
			double sysX = 0;
			double sysY = 0;
			double sysZ = 0;
			sq = QString("SELECT name,ids FROM a_World WHERE id=%1;").arg(idPlanet.GetID());
			QSqlQuery qryPlanet(QString(), db);
			qryPlanet.prepare(sq);
			qryPlanet.exec();
			if(qryPlanet.next())
			{
				int qv=0;
				sPlanet = qryPlanet.value(qv++).toString();
				idSystem = AuId(qryPlanet.value(qv++).toUInt());

				Q_ASSERT(!idSystem.IsNull());
				// Get information about the solar system.
				sq = QString("SELECT idp,name,locx,locy,locz from a_solarsystem WHERE id=%1;").arg(idSystem.GetID());
				QSqlQuery qrySystem(QString(), db);
				qrySystem.prepare(sq);
				qrySystem.exec();
				if(qrySystem.next())
				{
					int qv=0;
					idSector = AuId(qrySystem.value(qv++).toUInt());
					sSystem = qrySystem.value(qv++).toString();
					sysX = qrySystem.value(qv++).toDouble();
					sysY = qrySystem.value(qv++).toDouble();
					sysZ = qrySystem.value(qv++).toDouble();

					// Prepare a query to determine the name of the sector and id of the galaxy.
					sq = QString("SELECT idp,name,locx,locy,locz FROM a_Sector WHERE id=%1;").arg(idSector.GetID());
					QSqlQuery qrySector(QString(), db);
					qrySector.prepare(sq);
					qrySector.exec();
					if(qrySector.next())
					{
						int qv=0;
						idGalaxy = AuId(qrySector.value(qv++).toUInt());
						sSector = qrySector.value(qv++).toString();
						secX = qrySector.value(qv++).toDouble();
						secY = qrySector.value(qv++).toDouble();
						secZ = qrySector.value(qv++).toDouble();

						// Prepare the final query to determine the name of the galaxy.
						sq = QString("SELECT galaxy FROM a_Galaxy WHERE id=%1;").arg(idGalaxy.GetID());
						QSqlQuery qryGalaxy(QString(), db);
						qryGalaxy.prepare(sq);
						qryGalaxy.exec();
						if(qryGalaxy.next())
						{
							int qv=0;
							galaxy = (GALAXY)qryGalaxy.value(qv++).toInt();
						}
					}
				}
			}

			// We have a complete AuCity object now. Add it to the appropriate empire record.
			if(galaxy < GALAXY_Count)
			{
				allCities.insert(pair<AuId, quint32>(idBd, empireId));
				AuPoint systemLoc = AuPoint(secX + sysX, secY + sysY, secZ + sysZ);
				gd.AddCityToEmpire(empireId, AuCity(cityType, idBd, sCity,
							founderId, founderName,
							idPlanet, sPlanet,
							idSystem, sSystem, systemLoc,
							idSector, sSector,
							idGalaxy, galaxy, isSectorCapital, isGalaxyCapital));
				empiresChanged.insert(empireId, true);
			}
		}
	}

	// Insure capital cities of empires are still in existence.
	for(DgEmpireHash::iterator ei = gd.m_gEmpires.begin(); ei != gd.m_gEmpires.end(); ei++)
		if((*ei).second.ReconcileCities(allCities))
			empiresChanged.insert((*ei).first, true);

	// Trigger db store for all changed empires. This gets propagated down to the subs.
	for(QMap<quint32, bool>::const_iterator eci = empiresChanged.cbegin(); eci != empiresChanged.cend(); eci++)
		UpdateEmpire(eci.key());
}

// Reconcile/validate/update information in empire records.
// This is done daily on the master to collect reputation of everybody, for office assignments and voting.
// This also checks planets that have deployed AI officers to insure the officers still exist.
// This also deletes empires that no longer have any people or cities.
void DtDb::UpdateEmpires(DgData& gd, QSqlDatabase& db)
{
	//Q_ASSERT(g_Au.IsSceneMaster());
	// Open the login database so to make queries to insure each person still belongs to a player login record. When a person
	// deletes a character, its id is removed from the player record only. When this code notices that has happened, the person's
	// atom record is deleted here. This might result in an UPDATE error later if the person atom gets changed but that error will
	// be harmless. The person atom will eventually go away when it is handed-off to another server or the server farm is restarted.
	// NOTE: This is painful to do here because I didn't want the scene server to know about the login database. Grrr, it's the
	//       best solution for now without implementing more elaborate measures.
	// auLogin database
	//   Owner: aumaster
	//   Password: Light
	//   User: audog
	//   Password: Gold
	QSqlDatabase dbLogin = QSqlDatabase::database("AuLogin", false);
	dbLogin.setDatabaseName("auLogin");			// NOTE: Name is case sensitive.
	dbLogin.setHostName("LoginDb.Hazeron.net");
	if(!dbLogin.open("audog","Gold"))
	{
		QSqlError er = dbLogin.lastError();
		QString sdv = er.driverText();
		QString sdb = er.databaseText();
		qWarning("Failed to open login database connection:\n%s\n%s\n",
			(const char *)sdv.toLatin1(),
			(const char *)sdb.toLatin1());
		return;
	}

	// Prepare a query to select all of the people records in the database so we can verify empire data.
	AuIdList allPeople;		// All people records, except those we decide to delete in deletePeople.
	AuIdList deletePeople;	// People to delete.
	QList<AuSpawnAtom> atomsSpawned;	// Atoms that need to spawn somewhere.
	QMap<quint32,bool> empiresChanged;
	QString sq;
	sq = QString("SELECT id,name,allegiance,pose,spawnCity,spawnShip,arenaExitParent,timeModified,sex"
			" FROM a_Person WHERE idp > 0;");
	QSqlQuery qryPerson(QString(), db);
	qryPerson.prepare(sq);
	qryPerson.exec();
	while(qryPerson.next())
	{
		int qv=0;	// query value index
		AuId id = AuId(qryPerson.value(qv++).toUInt());
		QString name = qryPerson.value(qv++).toString();
		quint32 empireId = qryPerson.value(qv++).toUInt();
		int pose = qryPerson.value(qv++).toInt();
		AuId spawnCity = AuId(qryPerson.value(qv++).toUInt());
		AuId spawnShip = AuId(qryPerson.value(qv++).toUInt());
		AuId arena = AuId(qryPerson.value(qv++).toUInt());
		QDateTime timeMod = qryPerson.value(qv++).toDateTime();
		timeMod.setTimeSpec(Qt::UTC);
		int gender = qryPerson.value(qv++).toInt();

		// Check to see if the person is referenced by any account in the login database.
		bool deletePerson = false;
		{
			QString sq;
			sq = QString("SELECT id FROM g_Player WHERE idu1=%1 OR idu2=%1 OR idu3=%1 OR idu4=%1 OR idu5=%1 OR idu6=%1;")
				.arg(id.GetID());
			QSqlQuery qryLogin(sq, dbLogin);
			if(!qryLogin.first())
			{
				// The person is not referenced by any player record.
				deletePerson = true;
				deletePeople.append(id);
				empireId = 0;	// Set to zero so the person will be removed from any previous empire data.
			}
			else
				allPeople.append(id);	// Keep track of all people that are not deleted.
		}

		// Check all empire records to insure the person is not in the wrong ones and that the person is in the proper one with
		// the correct data.
		for(DgEmpireHash::iterator ei = gd.m_gEmpires.begin(); ei != gd.m_gEmpires.end(); ei++)
		{
			if((*ei).second.m_id == empireId)
			{
				// The empire record should contain information about the person in its list of citizens.
				bool hasPerson = false;
				unordered_map<AuId, AuCitizen, AuIdHash>::iterator ci = (*ei).second.m_Citizens.find(id);
				if(ci != (*ei).second.m_Citizens.end())
				{
					// Insure data about the person is valid.
					if((*ci).second.m_name != name)
					{
						(*ci).second.m_name = name;
						empiresChanged.insert(empireId, true);
					}
					if((*ci).second.Gender() != gender)
					{
						(*ci).second.m_Gender = gender;
						empiresChanged.insert(empireId, true);
					}
					hasPerson = true;
				}
				if(!hasPerson)
				{
					// The person needs to be added to the empire.
					gd.AddCitizenToEmpire(0, (*ei).second.m_id, id, name, gender);
					empiresChanged.insert((*ei).second.m_id, true);
				}
			}
			else
			{
				// The empire record should not contain info about the person.
				bool removePerson = false;
				unordered_map<AuId, AuCitizen, AuIdHash>::iterator ci = (*ei).second.m_Citizens.find(id);
				if(ci != (*ei).second.m_Citizens.end())
					removePerson = true;
				if(removePerson)
				{
					gd.RemoveCitizenFromEmpire((*ei).second.m_id, id);
					empiresChanged.insert((*ei).second.m_id, true);
				}
			}
		}

		// Some people may be stuck trying to spawn. Send them on their way if they were last modified over an hour ago.
		if(!deletePerson && pose == AuPose::Spawn && timeMod.secsTo(g_Au.CurrentUTC()) > 60*60)
		{
			if(g_Au.m_LogMessages & LOG_Spawn)
				qDebug() << "DtDb::UpdateEmpires spawning person:" << id << name << "empire" << empireId << "spawnCity" << spawnCity
					<< "spawnShip" << spawnShip << "arena" << arena << "last modified" << timeMod;
			atomsSpawned.append(AuSpawnAtom(id, empireId, spawnCity, spawnShip, arena, ShoresOfHazeron));
		}
	}

	// We're done with the login database now.
	dbLogin.close();

	// Remove people data from empire records whose id doesn't exist in the universe.
	for(DgEmpireHash::iterator ei = gd.m_gEmpires.begin(); ei != gd.m_gEmpires.end(); ei++)
	{
		// List people in the empire record who no longer exist in the universe.
		AuIdList removePeople;
		for(unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = (*ei).second.m_Citizens.cbegin(); ci != (*ei).second.m_Citizens.cend(); ci++)
		{
			if(!allPeople.contains((*ci).second.m_id))
				removePeople.append((*ci).second.m_id);	// Person in empire record was not found in list of all people ids.
		}

		if(!removePeople.isEmpty())
		{
			// The empire record should not contain info about the people in the removePeople list.
			for(AuIdList::const_iterator ri = removePeople.cbegin(); ri != removePeople.cend(); ri++)
			{
				gd.RemoveCitizenFromEmpire((*ei).second.m_id, *ri);
				empiresChanged.insert((*ei).second.m_id, true);
			}
		}
	}

	// Delete person atoms that are not referenced by any player record.
	// Mark person atoms that are not referenced by any player record so they don't get loaded but they still exist.
	if(!deletePeople.isEmpty())
	{
		QString sq("UPDATE a_Person SET idp=-1 WHERE");
		for(AuIdList::const_iterator di = deletePeople.cbegin(); di != deletePeople.cend(); di++)
		{
			if(g_Au.m_LogMessages & LOG_Database)
				qDebug("DtDb::UpdateEmpires Disabling person %d not referenced by any player record", (*di).GetID());
			if(di == deletePeople.cbegin())
				sq += QString(" id = %1").arg((*di).GetID());
			else
				sq += QString(" OR id = %1").arg((*di).GetID());
		}
		sq += ";";

		QSqlQuery qdel(db);
		qdel.prepare(sq);
		if(!qdel.exec())
			qWarning("DtDb::UpdateEmpires - UPDATE FAILED '%s'", (const char*)sq.toLatin1());
	}

	// Trigger db store for all changed empires. This gets propagated down to the subs.
	for(QMap<quint32,bool>::const_iterator eci = empiresChanged.cbegin(); eci != empiresChanged.cend(); eci++)
		UpdateEmpire(eci.key());

	// Insure officers still exist that have been deployed by planets.
	{
		// Query for planets whose captain id is non-zero but there is no corresponding captain record.
		QString sq = QString("SELECT id FROM a_WorldLL "
				"WHERE captId != 0 AND captId NOT IN (SELECT id FROM a_Npc WHERE id=captId AND role=%1);")
			.arg(ROLE_Officer);
		QSqlQuery qry(QString(), db);
		qry.prepare(sq);
		qry.exec();
		while(qry.next())
		{
			int qv=0;
			AuId planetId = AuId(qry.value(qv++).toUInt());
			if(g_Au.m_LogMessages & LOG_Database)
				qDebug("DtDb::UpdateEmpires Releasing captain slot from world %d. Captain record not found in db.",
						planetId.GetID());
			emit OfficerKilled(planetId);
		}
	}

	// Delete empires that no longer have any people or cities.
	{
		// Make a list of the empires that we will delete.
		QMap<quint32, QString> empiresDeleted;	// key is empire id. value is name for debug message
		for(DgEmpireHash::const_iterator ei = gd.m_gEmpires.cbegin(); ei != gd.m_gEmpires.cend(); ei++)
			if((*ei).second.m_Cities->empty() && (*ei).second.m_Citizens.empty() && (*ei).second.IsPlayerEmpire())
				empiresDeleted.insert((*ei).second.m_id, (*ei).second.Name());	// Empire has no cities or citizens.

		if(!empiresDeleted.isEmpty())
		{
			// Delete the empires from the global data and from the database.
			QList<quint32> chgEmpireIds;				// Accumulate a list of ids of other empire records that change as a result.
			QString sq("DELETE FROM g_Empire WHERE");
			for(QMap<quint32, QString>::const_iterator edi = empiresDeleted.cbegin(); edi != empiresDeleted.cend(); edi++)
			{
				//if(g_Au.m_LogMessages & LOG_Database)
				g_Au.Log(QString("UpdateEmpires empire %1 %2 deleted because it has no cities or avatars.")
						.arg(edi.key())
						.arg(*edi));
				gd.DeleteEmpire(edi.key(), chgEmpireIds);			// Delete global empire record. Also cleans up associations.
				if(edi == empiresDeleted.cbegin())
					sq += QString(" id = %1").arg(edi.key());
				else
					sq += QString(" OR id = %1").arg(edi.key());

				// Inform the server farm of the empires that will be deleted.
				emit EmpireRecordDeleted(edi.key());
			}
			sq += ";";

			QSqlQuery qdel(db);
			qdel.prepare(sq);
			if(!qdel.exec())
				qWarning("DtDb::UpdateEmpires - DELETE FAILED '%s'", (const char*)sq.toLatin1());
			else
				qDebug() << "DtDb::UpdateEmpires deleted" << empiresDeleted.size() << "empty empires. Check log for details.";

			// Update each changed empire record in the database. These are empires whose associations changed due to an empire
			// being deleted that they knu about.
			for(QList<quint32>::const_iterator it = chgEmpireIds.cbegin(); it != chgEmpireIds.cend(); it++)
			{
				if(!empiresDeleted.contains(*it))
				{
					// The empire was not deleted so update it.
					DgEmpire emp;
					emp.m_id = *it;
					if(gd.Empire(emp))
					{
						emp.Update(db);
						emit EmpireRecordChanged(*it);	// Notify subordinate servers that an empire record changed.
					}
				}
			}
		}
	}

	// Notify manager server of atoms that are supposed to spawn somewhere.
	for(QList<AuSpawnAtom>::const_iterator it = atomsSpawned.cbegin(); it != atomsSpawned.cend(); it++)
		emit AtomSpawned((*it).m_Person, (*it).m_EmpireId, (*it).m_SpawnCity, (*it).m_SpawnShip, (*it).m_Arena, (*it).Galaxy());
}

void DtDb::InsertSpecies(AuSpecies species)
{
	// Add the emire to the nuly added list so it will be saved by the db thread.
	m_muSpecieTransactions.lock();
	if(!m_AddedSpecies.contains(species))
		m_AddedSpecies.append(species);
	m_muSpecieTransactions.unlock();
}

void DtDb::UpdateSpecies(AuSpecies species)
{
	// No need to update the specie if it hasn't been inserted into the db yet.
	// The insert will save all its data.
	m_muSpecieTransactions.lock();
	bool addPending = m_AddedSpecies.contains(species);
	if(!addPending)
	{
		// Add the empire to the changed list so it will be updated by the db thread.
		if(!m_ChangedSpecies.contains(species))
			m_ChangedSpecies.append(species);
	}
	m_muSpecieTransactions.unlock();
}

void DtDb::InsertEmpire(quint32 idEmpire)
{
	// Add the emire to the nuly added list so it will be saved by the db thread.
	m_muEmpireTransactions.lock();
	if(!m_AddedEmpireIds.contains(idEmpire))
		m_AddedEmpireIds.append(idEmpire);
	m_muEmpireTransactions.unlock();
}

void DtDb::UpdateEmpire(quint32 idEmpire)
{
	// No need to update the empire if it hasn't been inserted into the db yet.
	// The insert will save all its data.
	m_muEmpireTransactions.lock();
	bool addPending = m_AddedEmpireIds.contains(idEmpire);
	if(!addPending)
	{
		// Add the empire to the changed list so it will be updated by the db thread.
		if(!m_ChangedEmpireIds.contains(idEmpire))
			m_ChangedEmpireIds.append(idEmpire);
	}
	m_muEmpireTransactions.unlock();
}

void DtDb::RecoverAtomFromLimbo(AuId atomId)
{
	//qDebug("DtDb::RecoverAtomFromLimbo %d", atomId.GetID());
	m_muAtomsInLimbo.lock();
	m_AtomsInLimbo.AddIfUnique(atomId);
	m_muAtomsInLimbo.unlock();
}

void DtDb::TravelerArrived(AuId traveler, AuId destSystem, AuId destSector)
{
	if(g_Au.m_LogMessages & LOG_Travel)
		qDebug("DtDb::TravelerArrived traveler %d destSystem %d destSector %d", traveler.GetID(),
				destSystem.GetID(), destSector.GetID());
	AuTravelAtom trav(traveler, AuId(0), destSystem, destSector);
	m_muAtomTransactions.lock();
	m_ArrivingTravelers.append(trav);
	m_muAtomTransactions.unlock();
}

// When traveling via intergalactic wormhole, 'destSystem' and 'destSector' are the destination galaxy id.
void DtDb::TravelerDeparted(AuId traveler, AuId origSystem, AuId destSystem, AuId destSector)
{
	if(g_Au.m_LogMessages & LOG_Travel)
		qDebug("DtDb::TravelerDeparted traveler %d origSystem %d destSystem %d destSector %d", traveler.GetID(),
				origSystem.GetID(), destSystem.GetID(), destSector.GetID());
	// The traveling atom needs to be saved in the database before the destination server is notified.
	AuTravelAtom trav(traveler, origSystem, destSystem, destSector);
	m_muAtomTransactions.lock();
	m_DepartingTravelers.append(trav);
	m_muAtomTransactions.unlock();
	g_AuDb.PostCriticalUpdates();		// This needs to trigger an update right away.
}

void DtDb::TravelerDepartedGalaxyLoc(AuId traveler, AuId origSystem, AuId destGalaxy, AuPoint destGalaxyLoc)
{
	if(g_Au.m_LogMessages & LOG_Travel)
		qDebug() << "DtDb::TravelerDepartedGalaxyLoc traveler" << traveler << "origSystem" << origSystem
			<< "destGalaxy" << destGalaxy << destGalaxyLoc;

	// The traveling atom needs to be saved in the database before the destination server is notified.
	AuTravelAtom trav(traveler, origSystem, destGalaxy, destGalaxyLoc, 2);	// GalaxyLoc
	m_muAtomTransactions.lock();
	m_DepartingTravelers.append(trav);
	m_muAtomTransactions.unlock();
	g_AuDb.PostCriticalUpdates();		// This needs to trigger an update right away.
}

void DtDb::TravelerDepartedStargate(AuId traveler, AuId origSystem, AuId destGalaxy, AuPoint destGalaxyLoc)
{
	if(g_Au.m_LogMessages & LOG_Travel)
		qDebug() << "DtDb::TravelerDepartedStargate traveler" << traveler << "origSystem" << origSystem << "destGalaxy"
			<< destGalaxy << destGalaxyLoc;

	// The traveling atom needs to be saved in the database before the destination server is notified.
	AuTravelAtom trav(traveler, origSystem, destGalaxy, destGalaxyLoc, 1);	// Stargate
	m_muAtomTransactions.lock();
	m_DepartingTravelers.append(trav);
	m_muAtomTransactions.unlock();
	g_AuDb.PostCriticalUpdates();		// This needs to trigger an update right away.
}

void DtDb::SpawnAtom(AuId personOrShip, quint32 empireId, AuId spawnCity, AuId spawnShip, AuId arena, quint8 galaxy)
{
	if(g_Au.m_LogMessages & LOG_Spawn)
		qDebug("DtDb::SpawnPerson personOrShip %d empireId %d spawnCity %d spawnShip %d arena %d", personOrShip.GetID(), empireId,
				spawnCity.GetID(), spawnShip.GetID(), arena.GetID());
	// The spawning person atom needs to be saved in the database before the destination server is notified.
	AuSpawnAtom spawn(personOrShip, empireId, spawnCity, spawnShip, arena, (GALAXY)galaxy);
	m_muAtomTransactions.lock();
	m_SpawningAtoms.append(spawn);
	m_muAtomTransactions.unlock();
	g_AuDb.PostCriticalUpdates();		// This needs to trigger an update right away.
}

void DtDb::HengeTeleportAtom(AuHengeTeleportAtom hengeTel)
{
	// The traveling atom needs to be saved in the database before the destination server is notified.
	m_muAtomTransactions.lock();
	m_DepartingHengeTeleport.append(hengeTel);
	m_muAtomTransactions.unlock();
	g_AuDb.PostCriticalUpdates();		// This needs to trigger an update right away.
}

void DtDb::HengeTeleportArrived(AuHengeTeleportAtom hengeTel)
{
	if(g_Au.m_LogMessages & LOG_Travel)
		qDebug("DtDb::HengeTeleportArrived traveler %d destSystem %d", hengeTel.m_TeleportId.GetID(),
				hengeTel.m_DestSysId.GetID());
	m_muAtomTransactions.lock();
	m_ArrivingHengeTeleport.append(hengeTel);
	m_muAtomTransactions.unlock();
}

void DtDb::ReloadData()
{
	QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
	if(OpenDatabase(db))
	{
		//qDebug() << "DtDb::ReloadData";
		DgData& gd = g_Au.LockDataForWrite();
		gd.Load();
		g_Au.UnlockData();

		db.close();	// We'll open it when we need it. We don't keep it open.
	}
}

void DtDb::CityReportRequested(AuId cityId, quint8 reportWhat, AuId unitId, quint32 unitEmpire)
{
	//qDebug() << "DtDb::CityReportRequested" << cityId << reportWhat << unitId << unitEmpire;
	m_muCityReportRequests.lock();

	// Insure there is not already a request for that exact same report in the queue.
	for(unordered_multimap<AuId, AuCityReportRequest, AuIdHash>::const_iterator ri = m_CityReportRequests.find(cityId);
			ri != m_CityReportRequests.end() && (*ri).first == cityId; ri++)
		if((*ri).second.m_ReportWhat == reportWhat && (*ri).second.m_UnitId == unitId && (*ri).second.m_UnitEmpire == unitEmpire)
		{
			// Ignore the request.
			m_muCityReportRequests.unlock();
			return;
		}

	// Queue the request.
	m_CityReportRequests.insert(pair<AuId, AuCityReportRequest>(cityId, AuCityReportRequest(reportWhat, unitId, unitEmpire)));

	m_muCityReportRequests.unlock();
}

void DtDb::ProvideCityReports()
{
	// Grab a copy of the list of city reports that have been requested.
	unordered_multimap<AuId, AuCityReportRequest, AuIdHash>	cityReportRequests;
	m_muCityReportRequests.lock();
	cityReportRequests = m_CityReportRequests;
	m_CityReportRequests.clear();
	m_muCityReportRequests.unlock();
	if(cityReportRequests.empty())
		return;

	QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
	if(!OpenDatabase(db))
		return;							// Failed to open database?

	// Build a query to select them all at once vs multiple selects.
	QString sq("SELECT id,allegiance,reportHist FROM a_Bd WHERE");
	for(unordered_multimap<AuId, AuCityReportRequest, AuIdHash>::const_iterator ri = cityReportRequests.cbegin();
			ri != cityReportRequests.cend(); ri++)
	{
		if(ri != cityReportRequests.cbegin())
			sq += QString(" OR id=%1").arg((*ri).first.GetID());
		else
			sq += QString(" id=%1").arg((*ri).first.GetID());
	}
	sq += ";";

	QSqlQuery qry(QString(), db);
	qry.prepare(sq);
	qry.exec();
	while(qry.next())
	{
		int qv=0;
		AuId cityId = AuId(qry.value(qv++).toUInt());
		quint32 empireId = qry.value(qv++).toUInt();

		// Locate the request so we know what to send them.
		unordered_multimap<AuId, AuCityReportRequest, AuIdHash>::iterator ri = cityReportRequests.find(cityId);
		if(ri == cityReportRequests.end())
			continue;			// The report request was not found?

		// Insure requestor is authorized to get the report. They must be in the same empire as the city.
		if((*ri).second.m_UnitEmpire && (*ri).second.m_UnitEmpire != empireId)
			continue;			// The unit empire does not match the city's. Empire 0 is used by agents to skip this test.

		int qvi = qv++;
		if(!qry.value(qvi).isNull())
		{
			QByteArray ba = qry.value(qvi).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_4_1);
			AuCityReportHistory reportHist;
			reportHist << in;

			// Locate the request so we know what to send them.
			unordered_multimap<AuId, AuCityReportRequest, AuIdHash>::iterator ri = cityReportRequests.find(cityId);
			if(ri != cityReportRequests.end())
			{
				// Send the report data.
				switch((*ri).second.m_ReportWhat)
				{
				case 0:
					if(reportHist.Count() == 1)
						emit CityLastReportReady((*ri).second.m_UnitId, (*ri).first, reportHist.Last(), false,
								reportHist.Last());
					else if(reportHist.Count() > 1)
						emit CityLastReportReady((*ri).second.m_UnitId, (*ri).first, reportHist.Last(), true,
								reportHist.NextToLast());
					break;
				case 1:
					emit CityHistoryReportReady((*ri).second.m_UnitId, (*ri).first, reportHist);
					break;
				}

				// Remove requests that we have processed in case of multiples for the same city.
				cityReportRequests.erase(ri);
			}
		}
	}
}

void DtDb::SectorAssignmentRevoked(AuId sectorId)
{
	Q_UNUSED(sectorId);
	// Clear loaded sectors list to force reeval of loaded sectors.
	m_LoadedLock.lock();
	m_Loaded.clear();
	m_LoadedLock.unlock();
}

void DtDb::SystemAssignmentRevoked(AuId systemId)
{
	Q_UNUSED(systemId);
	if(g_Au.m_LogMessages & LOG_Database)
		qDebug() << "DtDb::SystemAssignmentRevoked" << systemId;
	// Clear loaded systems list to force reeval of loaded systems.
	m_LoadedLock.lock();
	m_Loaded.clear();
	m_LoadedLock.unlock();
}

void DtDb::StoreBdRatings(QSqlDatabase& db)
{
	// Grab a copy of the ratings list.
	QList<AuExchangeRatingRec> ratings = g_AuDb.TakeBdRatings();
	if(ratings.empty())
		return;

	for(QList<AuExchangeRatingRec>::const_iterator ri = ratings.cbegin(); ri != ratings.cend(); ri++)
		(*ri).SqlStoreBd(db);
}

void DtDb::StoreScRatings(QSqlDatabase& db)
{
	// Grab a copy of the ratings list.
	QList<AuExchangeRatingRec> ratings = g_AuDb.TakeScRatings();
	if(ratings.empty())
		return;

	for(QList<AuExchangeRatingRec>::const_iterator ri = ratings.cbegin(); ri != ratings.cend(); ri++)
		(*ri).SqlStoreSc(db);
}
