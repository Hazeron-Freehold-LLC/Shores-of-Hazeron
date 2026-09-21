//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausceneserverapp.h,v 1.427 2022/04/15 20:40:21 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSceneServerApp_H
#define AuSceneServerApp_H

#include "auarenaready.h"
#include "auchatchannel.h"
#include "aucommpacket.h"
#include "audomain.h"
#include "auexchangerating.h"
#include "augear.h"
#include "auglobal.h"
#include "auglobell.h"
#include "auid.h"
#include "aujoboffer.h"
#include "aumgrchannel.h"
#include "auplayeronlinestatus.h"
#include "auroster.h"
#include "auscdesignexchange.h"
#include "ausceneserver.h"
#include "auserverchannel.h"
#include "aushipment.h"
#include "auspecies.h"
#include "austarmapdata.h"
#include "autime.h"
#include "dgempire.h"
#include "dtdb.h"
#include <unordered_map>
#include <QCoreApplication>
#include <QMap>
#include <QMutex>
#include <QThread>
#include <QUdpSocket>

// Enable the MEM_TRACE_GLOBAL define to override new and delete for everyone.
// Enable the MEM_TRACE_ATOM define in daatom.h to override new and delete for atoms.
//#define MEM_TRACE_GLOBAL

class AuCommPacket;
class AuEther;
class DaAtom;
class DaAtomList;
class DaGalaxy;
class DaPerson;
class DaSector;
class DaSolarSystem;
class DgData;
class DtClientChannel;
class DtDb;
class DtMailClient;
class DtTrash;
class QTimer;
class AuGLModelCache;
class DgEmpire;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuOpenStargate
{
public:
	AuId								m_FromGalaxy;
	AuPoint								m_FromGalaxyLoc;
	AuId								m_ToGalaxy;
	AuPoint								m_ToGalaxyLoc;
	AuId								m_ToSector;
	AuId								m_ToSystem;
	AuTime								m_Expire;

public:
	inline AuOpenStargate(const AuId& fromGalaxy, const AuPoint& fromGalaxyLoc, const AuId& toGalaxy, const AuPoint& toGalaxyLoc,
			const AuId& toSector, const AuId& toSystem, AuTime expire)
	{
		m_Expire = expire;
		m_FromGalaxy = fromGalaxy;
		m_FromGalaxyLoc = fromGalaxyLoc;
		m_ToGalaxy = toGalaxy;
		m_ToGalaxyLoc = toGalaxyLoc;
		m_ToSector = toSector;
		m_ToSystem = toSystem;
	}

	inline AuOpenStargate(const AuOpenStargate& c)
	{
		m_Expire = c.m_Expire;
		m_FromGalaxy = c.m_FromGalaxy;
		m_FromGalaxyLoc = c.m_FromGalaxyLoc;
		m_ToGalaxy = c.m_ToGalaxy;
		m_ToGalaxyLoc = c.m_ToGalaxyLoc;
		m_ToSector = c.m_ToSector;
		m_ToSystem = c.m_ToSystem;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSpawnArrival
{
public:
	AuId								m_SectorOrSystemId;		// Sector or system id of spawn atom, depending on server context.
	quint8								m_Galaxy;				// GALAXY enum

public:
	inline AuSpawnArrival(const AuId& id, GALAXY galaxy)
	{
		m_SectorOrSystemId = id;
		m_Galaxy = galaxy;
	}

	inline GALAXY Galaxy() const
	{
		return (GALAXY)m_Galaxy;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSpawnDeparture
{
public:
	AuId								m_SectorOrSystemId;		// Sector or system id of spawn atom, depending on server context.
	quint8								m_Galaxy;				// GALAXY enum

public:
	inline AuSpawnDeparture(const AuId& id, GALAXY galaxy)
	{
		m_SectorOrSystemId = id;
		m_Galaxy = galaxy;
	}

	inline GALAXY Galaxy() const
	{
		return (GALAXY)m_Galaxy;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Store information about a system that is assigned to this worker server.

class AuAtomManaged
{
private:
	AuId								m_AtomId;				// Atom that is managed.
	GALAXY								m_Galaxy;				// Galaxy containing m_AtomId.
	AuId								m_SectorId;				// Sector containing m_AtomId.

public:
	inline AuAtomManaged(const AuId& atomId, const AuId& sectorId, GALAXY galaxy)
	{
		m_AtomId = atomId;
		m_Galaxy = galaxy;
		m_SectorId = sectorId;
	}

	inline AuAtomManaged()
	{
		m_Galaxy = ShoresOfHazeron;
	}

	inline ~AuAtomManaged()
	{
	}

	inline AuAtomManaged(const AuAtomManaged& c)
	{
		m_AtomId = c.m_AtomId;
		m_Galaxy = c.m_Galaxy;
	}

	inline const AuAtomManaged& operator=(const AuAtomManaged& c)
	{
		m_AtomId = c.m_AtomId;
		m_Galaxy = c.m_Galaxy;
		return *this;
	}

	inline const AuId& AtomId() const
	{
		return m_AtomId;
	}

	inline GALAXY Galaxy() const
	{
		return m_Galaxy;
	}

	inline const AuId& SectorId() const
	{
		return m_SectorId;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class DhSceneServer
{
public:
	AuTime			m_CheckIn;	// Last checkin time.
	unsigned int	m_Id;		// Unique id of thread.
	QString			m_NetName;	// Internet resolvable name/ip address.
	quint16			m_NetPort;	// Port on which the server is listening.
	QString			m_PeerName;	// Name on internal lan. PeerAddress of client channel.

	DhSceneServer()
	{
		m_Id = 0;
		m_NetPort = 0;
	}

	DhSceneServer(const DhSceneServer& c)
	{
		m_CheckIn = c.m_CheckIn;
		m_Id = c.m_Id;
		m_NetName = c.m_NetName;
		m_NetPort = c.m_NetPort;
		m_PeerName = c.m_PeerName;
	}

	DhSceneServer& operator=(const DhSceneServer& c)
	{
		m_CheckIn = c.m_CheckIn;
		m_Id = c.m_Id;
		m_NetName = c.m_NetName;
		m_NetPort = c.m_NetPort;
		m_PeerName = c.m_PeerName;

		return *this;
	}

	bool IsValid() const
	{
		return (m_NetName.isEmpty() || m_NetPort == 0) ? false : true;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class AuAtomQuery
{
public:
	AuId			m_Atom;
	unsigned int	m_Empire;
	unsigned int	m_RepliesPending;	// How many responses we are still waiting for on this query.

	AuAtomQuery()
	{
		m_Empire = 0;
		m_RepliesPending = 0;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class AuHengeTeleportDestQuery
{
public:
	AuId			m_TeleportId;		// Creature waiting for a destination.
	unsigned int	m_RepliesPending;	// How many responses we are still waiting for on this query.

	AuHengeTeleportDestQuery()
	{
		m_RepliesPending = 0;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class AuAtomDeparture
{
public:
	AuId			m_toSolarSystem;
	AuId			m_toSector;
	//QByteArray		m_AtomData;			// serialized atom data that is moving.

	AuAtomDeparture()
	{
	}

	AuAtomDeparture(const AuAtomDeparture& c) //: m_AtomData(c.m_AtomData)
	{
		m_toSolarSystem = c.m_toSolarSystem;
		m_toSector = c.m_toSector;
	}

	AuAtomDeparture& operator=(const AuAtomDeparture& c)
	{
		//m_AtomData = c.m_AtomData;
		m_toSolarSystem = c.m_toSolarSystem;
		m_toSector = c.m_toSector;

		return *this;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuMySector
{
public:
	quint8								m_Galaxy;				// Parent galaxy of the sector. GALAXY enum.
	quint8								m_Systems;				// Number of solar systems in the sector.

public:
	AuMySector(GALAXY galaxy, int systems)
	{
		m_Galaxy = galaxy;
		m_Systems = systems;
	}

	inline GALAXY Galaxy() const
	{
		return (GALAXY)m_Galaxy;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuMySystem
{
public:
	quint8								m_Galaxy;				// Parent galaxy of the system. GALAXY enum.
	AuId								m_Sector;				// Parent sector of the system.

public:
	inline AuMySystem(GALAXY galaxy, const AuId& sector)
	{
		m_Galaxy = galaxy;
		m_Sector = sector;
	}

	inline GALAXY Galaxy() const
	{
		return (GALAXY)m_Galaxy;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSectorAssignment
{
public:
	DhSceneServer		m_toServer;		// Server to which assigned.
	unsigned int		m_solarSystems;	// Solar systems contained within the sector.

public:
	AuSectorAssignment()
	{
		m_solarSystems = 0;
	}

	AuSectorAssignment(const AuSectorAssignment& c)
	{
		m_toServer = c.m_toServer;
		m_solarSystems = c.m_solarSystems;
	}

	AuSectorAssignment& operator=(const AuSectorAssignment& c)
	{
		m_toServer = c.m_toServer;
		m_solarSystems = c.m_solarSystems;

		return *this;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class AuSystemAssignment
{
public:
	DhSceneServer		m_toServer;		// Server to which assigned.

public:
	AuSystemAssignment()
	{
	}

	AuSystemAssignment(const AuSystemAssignment& c)
	{
		m_toServer = c.m_toServer;
	}

	AuSystemAssignment& operator=(const AuSystemAssignment& c)
	{
		m_toServer = c.m_toServer;
		return *this;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// Helper for accumulating spawn data during spawn request.

class AuSpawnRequest
{
public:
	AuTime			m_Start;			// Time spawn request was posted.
	AuId			m_Person;			// person who needs to respawn
	AuId			m_Arena;			// World id of arena when person entered.
	unsigned int	m_ArenaServerId;	// Client channel server that has arena.
	unsigned int	m_Empire;			// empire id of person
	AuId			m_CapitolG;			// galaxy capitol of spawning person's empire, in m_Galaxy
	unsigned int	m_CapitolGServerId;	// Client channel server that has capG.
	AuId			m_CapitolU;			// universe capitol of spawning person's empire
	unsigned int	m_CapitolUServerId;	// Client channel server that has capU.
	AuId			m_City;				// spawn city of person (Look at usage; this could be a ship atom id.)
	unsigned int	m_CityServerId;		// Client channel server id that has city.
	quint8			m_Galaxy;			// GALAXY enum value
	AuId			m_Ship;				// spawn spacecraft of person
	unsigned int	m_ShipServerId;		// Client channel server id that has ship
	AuId			m_RandomCity;		// Random city in empire
	unsigned int	m_RandomCityServerId;	// Client channel server id that has the random city.

public:
	AuSpawnRequest()
	{
		m_ArenaServerId = 0;
		m_CapitolGServerId = 0;
		m_CapitolUServerId = 0;
		m_CityServerId = 0;
		m_Empire = 0;
		m_Galaxy = ShoresOfHazeron;
		m_RandomCityServerId = 0;
		m_ShipServerId = 0;
	}

	AuSpawnRequest(const AuSpawnRequest& c) //: m_AtomData(c.m_AtomData)
	{
		m_Arena = c.m_Arena;
		m_ArenaServerId = c.m_ArenaServerId;
		m_Start = c.m_Start;
		m_Person = c.m_Person;
		m_Empire = c.m_Empire;
		m_CapitolG = c.m_CapitolG;
		m_CapitolGServerId = c.m_CapitolGServerId;
		m_CapitolU = c.m_CapitolU;
		m_CapitolUServerId = c.m_CapitolUServerId;
		m_City = c.m_City;
		m_CityServerId = c.m_CityServerId;
		m_Galaxy = c.m_Galaxy;
		m_Ship = c.m_Ship;
		m_ShipServerId = c.m_ShipServerId;
		m_RandomCity = c.m_RandomCity;
		m_RandomCityServerId = c.m_RandomCityServerId;
	}

	AuSpawnRequest& operator=(const AuSpawnRequest& c)
	{
		m_Arena = c.m_Arena;
		m_ArenaServerId = c.m_ArenaServerId;
		m_Start = c.m_Start;
		m_Person = c.m_Person;
		m_Empire = c.m_Empire;
		m_CapitolG = c.m_CapitolG;
		m_CapitolGServerId = c.m_CapitolGServerId;
		m_CapitolU = c.m_CapitolU;
		m_CapitolUServerId = c.m_CapitolUServerId;
		m_City = c.m_City;
		m_CityServerId = c.m_CityServerId;
		m_Galaxy = c.m_Galaxy;
		m_Ship = c.m_Ship;
		m_ShipServerId = c.m_ShipServerId;
		m_RandomCity = c.m_RandomCity;
		m_RandomCityServerId = c.m_RandomCityServerId;
		return *this;
	}

	inline GALAXY Galaxy() const
	{
		return (GALAXY)m_Galaxy;
	}

	inline bool IsReadyToSpawn() const
	{
		// Return true if we've found our number one priority spawn location.
		// Otherwise, we must wait until we timeout and then we'll figure out
		// the best spawn location from the atoms we did receive.
		if(!m_Arena.IsNull())
		{
			// Our first priority is to find the ship. Other spawn points are secondary.
			if(m_ArenaServerId > 1)
				return true;		// Arena found.
			else if(m_ArenaServerId == 0)
				return false;		// Arena server still waiting for definite response.
		}

		if(!m_Ship.IsNull())
		{
			// Our first priority is to find the ship. Other spawn points are secondary.
			if(m_ShipServerId > 1)
				return true;		// Ship found.
			else if(m_ShipServerId == 0)
				return false;		// Ship server still waiting for definite response.
		}

		if(!m_City.IsNull())
		{
			// Our first priority is to find the city. Other spawn points are secondary.
			if(m_CityServerId > 1)
				return true;		// City found.
			else if(m_CityServerId == 0)
				return false;		// City server still waiting for definite response.
		}

		if(!m_CapitolG.IsNull())
		{
			// Our first priority is to find the galaxy capitol city. Other spawn points are secondary.
			if(m_CapitolGServerId > 1)
				return true;		// Galaxy capitol city found.
			else if(m_CapitolGServerId == 0)
				return false;		// Galaxy capitol city still waiting for definite response.
		}
	
		if(!m_CapitolU.IsNull())
		{
			// Our first priority is to find the universe/empire capitol city. Other spawn points are secondary.
			if(m_CapitolUServerId > 1)
				return true;		// Universe capitol city found.
			else if(m_CapitolUServerId == 0)
				return false;		// Universe capitol city still waiting for definite response.
		}
	
		if(!m_RandomCity.IsNull())
		{
			// Is there anywhere left in the empire to spawn to?
			if(m_RandomCityServerId > 1)
				return true;		// we found a city holding out for the empire!
			else if(m_RandomCityServerId == 0)
				return false;		// still looking for a city in the empire.
		}

		// Here if we definitely know none of the spawn locations is available.
		// Spawn request is ready, even if it means initiating the creation of a nu homeworld.
		return true;
	}

	inline bool IsTimedOut() const
	{
		return (m_Start.ElapsedSec() > 60) ? true : false;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// Helper for accumulating spawn data during spawn request.

class AuHengeTeleportRequest
{
public:
	AuTime			m_Start;			// Time spawn request was posted.
	AuHengeTeleportAtom	m_HengeTel;
	unsigned int	m_DestServerId;		// Client channel server that has the destination.
	int				m_Result;

public:
	AuHengeTeleportRequest()
	{
		m_DestServerId = 0;
		m_Result = 0;
	}

	AuHengeTeleportRequest(const AuHengeTeleportRequest& c)
	{
		m_DestServerId = c.m_DestServerId;
		m_HengeTel = c.m_HengeTel;
		m_Result = c.m_Result;
		m_Start = c.m_Start;
	}

	AuHengeTeleportRequest& operator=(const AuHengeTeleportRequest& c)
	{
		m_DestServerId = c.m_DestServerId;
		m_HengeTel = c.m_HengeTel;
		m_Result = c.m_Result;
		m_Start = c.m_Start;
		return *this;
	}

	inline bool IsReadyToTeleport() const
	{
		// Return true if we've found our destination spawn location.
		if(m_Result > 0)
			return true;
		else if(m_DestServerId == 0)
			return false;		// Dest server still waiting for definite response.

		// Here if we definitely know the dest location is not available.
		return true;
	}

	// Give them ample time. When this times out, they get sent back where they came from. We could have to wait for a system to
	// be finalized.
	inline bool IsTimedOut() const
	{
		return (m_Start.ElapsedSec() > 180) ? true : false;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// Helper for accumulating handoff data during handoff request.

class AuHandoffRequest
{
public:
	AuTime			m_Start;			// Time spawn request was posted.
	AuId			m_PersonOrShip;		// person that needs to handoff
	AuId			m_Dest;				// destination atom of handoff
	AuId			m_ReturnOnFail;
	quint8			m_RideAlso;			// ride also flag
	bool			m_LimboRescue;		// true to initiate limbo rescue if destination is not found
	unsigned int	m_ServerId;			// Client channel server that has the destination atom.
	QByteArray		m_AtomData;			// Atom data of atom that wants to handoff.
	int				m_TimeOuts;			// Number of times this handoff request had timed out.

	AuHandoffRequest()
	{
		m_RideAlso = 0;
		m_LimboRescue = false;
		m_ServerId = 0;
		m_TimeOuts = 0;
	}

	AuHandoffRequest(const AuHandoffRequest& c) : m_AtomData(c.m_AtomData)
	{
		m_ReturnOnFail = c.m_ReturnOnFail;
		m_Start = c.m_Start;
		m_PersonOrShip = c.m_PersonOrShip;
		m_Dest = c.m_Dest;
		m_RideAlso = c.m_RideAlso;
		m_LimboRescue = c.m_LimboRescue;
		m_ServerId = c.m_ServerId;
		m_TimeOuts = c.m_TimeOuts;
	}

	AuHandoffRequest& operator=(const AuHandoffRequest& c)
	{
		m_ReturnOnFail = c.m_ReturnOnFail;
		m_AtomData = c.m_AtomData;
		m_Start = c.m_Start;
		m_PersonOrShip = c.m_PersonOrShip;
		m_Dest = c.m_Dest;
		m_RideAlso = c.m_RideAlso;
		m_LimboRescue = c.m_LimboRescue;
		m_ServerId = c.m_ServerId;
		m_TimeOuts = c.m_TimeOuts;
		return *this;
	}

	inline bool IsReadyToHandoff() const
	{
		return (m_ServerId > 1) ? true : false;
	}

	inline bool IsTimedOut() const
	{
		return (m_Start.ElapsedSec() > 60) ? true : false;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+


class AuSceneServerApp : public QCoreApplication
{
	Q_OBJECT;

public:
	AuSceneServerApp(int& argc, char** argv);
	~AuSceneServerApp();

#if defined(MEM_TRACE_GLOBAL)
	static quint64 AllocatedCount();
	static quint64 FreedCount();
#endif

	// Returns the desired number of atom managers, whether or not they have been created yet.
	inline int AtomManagerCount() const
	{
		return m_AtomManagerCount;
	}

	// Returns a number from 0 to 1+. 1 is 100%.
	float AtomManagerLoadAverage() const;

	float AverageWorkerLoadAverage() const;

	unsigned int AverageWorkerTickTime() const;

	inline DtDb::DBSTATUS DbStatus() const
	{
		if(!m_tDb)
			return DtDb::NotLoaded;
		return m_tDb->DbStatus();
	}

	void Chat(const AuCommPacket& msg, bool propagateUp);

	void ChatDown(const AuCommPacket& msg);

	void ChatUp(const AuCommPacket& msg, DtServerChannel* fromChannel);

	// Blocks until Tick and Tock have finished processing 'atomId'.
	void ClearTickTock(const AuId& atomId);

	int ClientCount() const;

	unsigned int CountAssignedTo(unsigned int serverId);

	unsigned int CountSubServers() const;

	Qt::HANDLE DbThreadId() const;

	// Examines a sector and its 26 neighbors and deletes the ones that are no longer needed.
	// 'deletedSectors' is filled with the ids of sectors that get deleted.
	void DecaySector(DaSector* sector, AuIdList& deletedSectors);

	// Examines all sectors in the galaxy and deletes the ones that are no longer needed.
	// 'deletedSectors' is filled with the ids of sectors that get deleted.
	void DecaySectors(DaGalaxy* galaxy, AuIdList& deletedSectors);

	DhSceneServer DeferSceneToManager() const;

	void HandleSignal(int value);	// Handles a signal caught in main.cpp.

	bool IsSectorAssigned(const AuId& idSector);

	// Only called on the scene manager. Asserts this to be true.
	inline bool IsSectorAssignedToMe(const AuId& idSector) const
	{
		//Q_ASSERT(g_Au.IsSceneManager());
		m_rwAtomManagers.lockForRead();
		bool isAssigned = m_MySectors.find(idSector) != m_MySectors.cend();
		m_rwAtomManagers.unlock();
		return isAssigned;
	}

	bool IsSectorNew(const AuId& idSector) const;

	bool IsSolarSystemAssigned(const AuId& idSolarSystem);

	// Only called on the scene worker. Asserts this to be true.
	inline bool IsSolarSystemAssignedToMe(const AuId& idSystem) const
	{
		//Q_ASSERT(g_Au.IsSceneWorker());
		m_rwAtomManagers.lockForRead();
		bool isAssigned = m_MySystems.find(idSystem) != m_MySystems.cend();
		m_rwAtomManagers.unlock();
		return isAssigned;
	}

	// Only called on the scene manager. Asserts this to be true.
	inline void ListSectorsAssignedToMe(unordered_map<AuId, AuMySector, AuIdHash>& idSectors) const
	{
		//Q_ASSERT(g_Au.IsSceneManager());
		m_rwAtomManagers.lockForRead();
		idSectors = m_MySectors;
		m_rwAtomManagers.unlock();
	}

	// Only called on the scene worker. Asserts this to be true.
	// Return map key is system id, value is sector id and galaxy.
	inline void ListSolarSystemsAssignedToMe(QMap<AuId, AuMySystem>& idSystems) const
	{
		//Q_ASSERT(g_Au.IsSceneWorker());
		m_rwAtomManagers.lockForRead();
		for(unordered_map<AuId, AuAtomManaged*, AuIdHash>::const_iterator mi = m_MySystems.cbegin(); mi != m_MySystems.cend(); mi++)
			idSystems.insert((*mi).second->AtomId(), AuMySystem((*mi).second->Galaxy(), (*mi).second->SectorId()));
		m_rwAtomManagers.unlock();
	}

	// Finds 'systemId' for the caller, dynamically casting the result to a solar system.
	// Returns 0 if 'systemId' is 0, or if 'systemId' is not assigned to me,  or if the dynamic cast fails.
	DaSolarSystem* AssignedSolarSystem(const AuId& systemId, AuEther& ether);

	inline AuIdList& LockNewSectors()
	{
		m_rwNewSectors.lockForWrite();
		return m_NewSectors;
	}

	inline const QString& LoginName() const
	{
		return m_LoginName;
	}

	int ManagerCount() const;

	// Returns the longest average tick time of the managed atoms.
	// Return value is in 1/2 millisecond increments. i.e. 100 = 50ms
	unsigned int MaxTickTime() const;

	void OpenChannelUp(const AuId& fromUnitId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
			const AuId& toUnitId, const QString& channelName, AuCommStdChannel::SCOPE scope, DtServerChannel* fromChannel);

	int PlayerCount() const;

	void PlayersOnlineStatus(AuPlayerOnlineStatusVector& players) const;

	// Returns the player online status of all players on line. Based on player online status sent up to the master, accumulated
	// there, then broadcast periodically to all workers using PlayerStatusBroadcast.
	void AllPlayersOnlineStatus(AuPlayerOnlineStatusVector& players, bool forAgent) const;

	void ReportMemoryUsage();

	// Report resource usage into a data stream.
	void ResourceReport(QDataStream& out);

	// Report resource usage of a specific atom and its subatoms into a data stream.
	void ResourceReport(QDataStream& out, const AuId& atomId);

	// Report resource usage into a data stream.
	void ResourceReportAtoms(QDataStream& out, const AuId& atomId);

	// Report resource usage into a data stream.
	void ResourceReportModels(QDataStream& out);

	// Report load status into a data stream.
	void StatusReportLoads(QDataStream& out);

	// Report user status into a data stream.
	void StatusReportUsers(QDataStream& out);

	inline const AuTime& TimeUp() const
	{
		return m_tmUp;
	}

	// Returns sum of average tick times of solar systems on worker server.
	// Returns 0xffffffff when not ready to receive system assignments.
	// The return value is in 1/2 millisecond increments. i.e. 100 = 50ms.
	quint32 TotalTickTime() const;

	// Returns sum of average tock times of solar systems on worker server.
	// Returns 0xffffffff when not ready to receive system assignments.
	// The return value is in 1/2 millisecond increments. i.e. 100 = 50ms.
	quint32 TotalTockTime() const;

	inline void UnlockNewSectors()
	{
		m_rwNewSectors.unlock();
	}

	int WorkerCoreCount() const;

	int WorkerCount() const;

	DhSceneServer SectorServer(const AuId& idSector);

	DhSceneServer SolarSystemServer(const AuId& idSolarSystem);

	void CollectSystemStatus(unordered_map<AuId, AuSystemStatus, AuIdHash>& systems);

public slots:
	void AddCitizenToEmpire(quint32 oldEmpireId, quint32 nuEmpireId, AuId personId, QString personName, quint8 personGender);

	// Initiates the sequence of an admin person moving to the location of another atom, with or without their vehicle.
	// 'person' is the person atom who is moving
	// 'dest' is the destination atom, this could be various kinds of atoms. It will be an admin person when summoning an atom.
	// 'rideAlso' is 1 when the person wants their parent vehicle or mount taken with them. 2 allows spacecraft also.
	// 'limboRescue' is true to initiate limbo rescue operation if the destination person cannot be found in 3 minutes.
	void AdminGoTo(AuId person, AuId dest, quint8 rideAlso, bool limboRescue, AuId returnOnFail);

	// Initiates the sequence of summoning a person to an admin, with or without their vehicle.
	// 'person' is the person atom who is moving
	// 'adminId' is the destination admin person atom.
	// 'rideAlso' is 1 when the person wants their parent vehicle or mount taken with them. 2 allows spacecraft also.
	void AdminSummon(AuId person, AuId adminId, quint8 rideAlso);

	void AdminSummonDown(AuId person, AuId adminId, quint8 rideAlso);

	void ArenaReady(AuArenaReady arena);

	void ArenaReadyDown(AuArenaReady arena);

	// Assigns 'office' to 'person'.
	// This can change the empire of 'person' to 'empire'.
	// Spawn information is only used when the office is in a new empire.
	void AssignOffice(AuId person, quint32 empire, AuOffice office, AuId spawnCity, QString spawnCityName, AuId spawnSystem,
			QString spawnSystemName);

	void AssignedSectorLoaded(AuId sectorId);

	void AssignedSolarSystemLoaded(AuId systemId);

	void AssignSectors();

	void AssignSolarSystems();

	void AtomPresence(AuId atom, unsigned int empire, int presence, unsigned int serverId);

	// Called daily by avatars whose account is active, to stave off decay.
	void AvatarAccountActive(AuIdList land);

	void AvatarAccountActiveDown(AuIdList land);

	void BlueprintRatedBdDown(AuExchangeRatingRec rating);

	void BlueprintRatedBdUp(AuExchangeRatingRec rating);

	void BlueprintRatedScDown(AuExchangeRatingRec rating);

	void BlueprintRatedScUp(AuExchangeRatingRec rating);

	// Propagate city industries up to master server to modify master empire record.
	void CityFacilitiesChanged(AuId cityId, AuCityIndustryHash facil);

	void CityStatusChanged(AuId cityId, int alertCondition, qint8 morale);

	void CollectBountyDown(AuId bountyHunter, QString bountyHunterName, AuGearPos gearPos, AuBountyOnItem bountyOnItem);

	void CollectBountyUp(AuId bountyHunter, QString bountyHunterName, AuGearPos gearPos, AuBountyOnItem bountyOnItem);

	void HengeDestinationConfirmed(AuId sectorId, AuId systemId, QString systemName, quint8 sun, QString worldCatName, QString worldName,
			quint8 worldType, AuGlobeLLF hengeLL, bool breathable, quint8 orbitZone, bool outsideTown);

	void HengeDestinationDown(AuId sectorId, AuId systemId, QString systemName, quint8 sun, QString worldCatName, QString worldName,
			quint8 worldType, AuGlobeLLF hengeLL, bool breathable, quint8 orbitZone, bool outsideTown);

	void HengeTeleporterPresence(AuId teleportId, int presence, unsigned int serverId);

	// A local atom is travelling from one solar system to another.
	// This is where the trail begins when an atom departs a solar system, either through a wormhole or by dead-head through space.
	// This is not where spawning happens.
	// If the origin and destination solar systems are in my locally managed data, the transfer happens immediately right here.
	// When traveling via intergalactic wormhole, 'destSystem' and 'destSector' are the destination galaxy id.
	void AtomTraveled(AuId traveler, AuId origSystem, AuId destSystem, AuId destSector);

	void AtomTraveledGalaxyLoc(AuId traveler, AuId origSystem, AuId destGalaxy, AuPoint destGalaxyLoc);

	void AtomTraveledStargate(AuId traveler, AuId origSystem, AuId destGalaxy, AuPoint destGalaxyLoc);

	void AtomTraveledDown(AuId traveler, AuId destSystem, AuId destSector);

	// When traveling via intergalactic wormhole, 'destSystem' and 'destSector' are the destination galaxy id.
	void AtomTraveledUp(AuId traveler, AuId destSystem, AuId destSector);

	void AtomTraveledGalaxyLocUp(AuId traveler, AuId destGalaxy, AuPoint destGalaxyLoc);

	void AtomTraveledStargateUp(AuId traveler, AuId destGalaxy, AuPoint destGalaxyLoc);

	// Emitted when a nu baby avatar is born. Adds the nu baby to the mother's login account, if there is an empty slot.
	void AvatarBorn(AuId motherAvatar, AuId babyAvatar);

	void AvatarAccepted(const AuId& avatarId, const QString& avatarName, const AuId& accepterId, const QString& accepterName);

	void BountyPosted(AuId posterId, QString posterName, AuId victimId, QString reason, double reward, int why);

	void BountyPostedDown(AuId posterId, QString posterName, AuId victimId, QString reason, double reward, int why);

	// Called when a chat packet is received by a DtChatChannel from a client connection.
	void ChatReceived(QByteArray bamsg);

	// Called when a chat packet is received by the client channel from my boss server, propagating a chat message down to me.
	void ChatReceivedDown(QByteArray bamsg);

	void CityCaptured(quint32 oldEmpireId, quint32 nuEmpireId, AuId cityId);

	// 'cityType' is a CITYTYPE enum value.
	// 'galaxy' is a GALAXY enum value.
	void CityCreated(quint32 empireId,
			quint8 cityType, AuId cityId, QString cityName,
			AuId founderId, QString founderName,
			AuId planetId, QString planetName,
			AuId systemId, QString systemName, AuPoint systemLoc,
			AuId sectorId, QString sectorName,
			AuId galaxyId, quint8 galaxy);

	void CityDecayed(quint32 empireId, AuId cityId);

	// City mail request on the way up the server farm.
	void CityMailRequested(AuId cityId, AuId unitId, quint32 unitEmpire);

	// City mail request on the way down the server farm.
	void CityMailRequestedDown(AuId cityId, AuId unitId, quint32 unitEmpire);

	void CloseStargateDown(AuId fromGalaxy, AuPoint fromGalaxyLoc);

	void CollectBounty(AuId bountyHunter, QString bountyHunterName, AuGearPos gearPos, AuBountyOnItem bountyOnItem);

	// Initiated by server manager to create a sector cube at a specific location.
	void CreateSectorAt(quint8 galaxyEnum, double sectorX, double sectorY, double sectorZ);

	// Emitted when a demiavatar is killed. Removes the avatar from the owner's login account.
	void DemiavatarDied(AuId avatar);

	// 'empireId' is the empire storing the document in their dossier.
	// 'dosEmpire' is the empire in the dossier.
	// 'packedDoc' is the document to store in the dossier, created using the Pack function of AuDocument.
	void GovDossierAddDocument(quint32 empireId, quint32 dosEmpire, QByteArray packedDoc);

	void HengeActivated(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			AuId destSysId, quint8 destSun, quint8 destArcSegment);

	void HengeActivatedDown(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			AuId destSysId, quint8 destSun, quint8 destArcSegment);

	void HengeCreated(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			AuId destSysId, quint8 destSun, quint8 destArcSegment);

	void HengeCreatedDown(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			AuId destSysId, quint8 destSun, quint8 destArcSegment);

	void HengeDeactivated(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			AuId destSysId, quint8 destSun, quint8 destArcSegment);

	void HengeDeactivatedDown(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			AuId destSysId, quint8 destSun, quint8 destArcSegment);

	void HengeTeleporterEntered(AuId teleportId, AuId fromSysId, quint8 fromSun, quint8 fromArc, quint8 shadowPanels,
			AuId destSysId, quint8 destSun, quint8 destArc);

	void MgrMemoryUsageRequest();

	void MgrResourceRequest(quint8 specificResources, AuId atomId);

	void MgrStatusRequest(quint8 specificStatus);

	// 'scope' is a AuCommStdChannel::SCOPE
	void OpenChannelDown(AuId fromUnitId, QString fromName, AuId fromSector, quint8 fromGalaxy, AuId toUnitId, QString channelName,
			quint8 scope);

	void OpenStargateDown(AuId fromGalaxy, AuPoint fromGalaxyLoc, AuId toGalaxy, AuPoint toGalaxyLoc, AuId toSector, AuId toSystem,
			AuTime expire);

	void PayBountyDown(AuId bountyHunter, AuGearPos gearPos, AuBountyOnItem bountyOnItem, bool victimHadBounty);

	void PayBountyUp(AuId bountyHunter, AuGearPos gearPos, AuBountyOnItem bountyOnItem, bool victimHadBounty);

	// This signal starts at the master server and works its way down to the rest of the grid.
	void PersonDeletedDown(AuId personId);

	void PirateBaseEncounteredDown(AuId inSystemId);

	void PlayersOnlineStatusDown(AuPlayerOnlineStatusVector players);

	void ClaimTitleDown(AuId propertyId, QString propertyName, AuId ownerId, QString ownerName, quint32 ownerEmpire);

	void ClaimTitleUp(AuId propertyId, QString propertyName, AuId ownerId, QString ownerName, quint32 ownerEmpire);

	void AcceptTrade(AuTradeAgreement ta);

	void AcceptTradeDown(AuTradeAgreement ta);

	// 'nuState' is an AuTradeAgreement::STATE.
	// 'atDest' is true when deactivating the destination of the agreement, false when deactivating the origin.
	void DeactivateTrade(AuTradeAgreement ta, quint8 nuState, bool atDest);

	// 'nuState' is an AuTradeAgreement::STATE.
	// 'atDest' is true when deactivating the destination of the agreement, false when deactivating the origin.
	void DeactivateTradeDown(AuTradeAgreement ta, quint8 nuState, bool atDest);

	void ProposeTrade(AuTradeAgreement ta);

	void ProposeTradeDown(AuTradeAgreement ta);

	void SectorPirateBaseEncounter(AuId encSectorId, AuId encSystemId);

	void ServerBroadcast(QString text);

	void SetAtomModified(AuId atomId, bool recursive);

	void ShipIt(AuItemTEUContainer container);

	void ShipItDown(AuItemTEUContainer container);

	void SpacecraftNameChanged(AuId shipId, QString nuName);

	void SpacecraftNameChangedDown(AuId shipId, QString nuName);

	void SpacecraftPositionReport(AuId shipId, AuId systemId, AuStarMapSpacecraft pos);

	void SpacecraftStasisRecoverDown(AuId shipId, AuId personId, quint8 personGalaxy, bool agentRecover);

	void SpacecraftStasisRecoverUp(AuId shipId, AuId personId, quint8 personGalaxy, bool agentRecover);

	void StargateClosed(AuId fromGalaxy, AuPoint fromGalaxyLoc);

	void StargateOpened(AuId fromGalaxy, AuPoint fromGalaxyLoc, AuId toGalaxy, AuPoint toGalaxyLoc, AuTime expire);

	void TradeReadyToShip(AuId readyCapitolId, AuTradeAgreement ta);

	void TradeReadyToShipDown(AuId readyCapitolId, AuTradeAgreement ta);

	void TradeShipped(AuId shipperCapitolId, AuTradeAgreement ta);

	void TradeShippedDown(AuId shipperCapitolId, AuTradeAgreement ta);

	void WarpSignatureDetected(AuId systemId, quint32 signalStrength, quint32 detectingEmpireId);

	// This one is on the way up the chain to the master.
	// 'galaxy' is a GALAXY enum value.
	void SystemWarpSignature(AuId systemId, quint32 warpFactors, quint8 galaxy, float galaxyLocX, float galaxyLocY,
			float galaxyLocZ);

	// This one is on the way down to the entire server farm.
	// 'galaxy' is a GALAXY enum value.
	void SystemWarpSignatureBroadcast(AuId systemId, quint32 warpFactors, quint8 galaxy, float galaxyLocX, float galaxyLocY,
			float galaxyLocZ);

	void CreditCityBank(float fAmount, AuId cityId);

	void CreditCityBankDown(float fAmount, AuId cityId);

	void DeclareMutualEnemies(quint32 empireId1, quint32 empireId2, quint8 reason, AuId fromUnit, QString fromUnitName,
			AuId toUnit, QString toUnitName, quint8 galaxy, QString sector, QString system, QString world);

	void BdDesignDeactivatedDown(quint32 designId);

	void BdDesignDeactivatedUp(quint32 designId);

	void BdDesignExchangePublishedDown(AuBdDesignExchangeEntry item);

	void BdDesignExchangePublishedUp(AuBdDesignExchangeEntry item);

	void BdDesignExchangeUnpublishedDown(quint32 designId);

	void BdDesignExchangeUnpublishedUp(quint32 designId);

	void ItDesignDeactivatedDown(quint32 designId);

	void ItDesignDeactivatedUp(quint32 designId);

	void ScDesignDeactivatedDown(quint32 designId);

	void ScDesignDeactivatedUp(quint32 designId);

	void ScDesignExchangePublishedDown(AuScDesignExchangeEntry item);

	void ScDesignExchangePublishedUp(AuScDesignExchangeEntry item);

	void ScDesignExchangeUnpublishedDown(quint32 designId);

	void ScDesignExchangeUnpublishedUp(quint32 designId);

	void FinalizeSystem(AuId systemId);

	void SectorDecayedDown(AuId sectorId);

	// Initiates the sequence of summoning a dormant spacecraft to a crew member.
	// 'shipId' is the dormant private ship atom that is moving
	// 'crewId' is the destination crew person atom.
	void SummonDormantShip(AuId shipId, AuId crewId);

	void SummonDormantShipDown(AuId shipId, AuId crewId);

	void SummonGoTo(AuId shipId, AuId crewId);

	void EmpireChanged(DgEmpire empire);

	// 'industry' is an INDUSTRYID
	void EmpireChgBdPrefUp(quint32 empireId, int industry, quint32 designId);

	// 'scrole' is an AuScPrefs::SCROLE
	void EmpireChgScPrefUp(quint32 empireId, int scrole, quint32 designId);

	void EmpireChgFlagUp(quint32 empireId, QImage flag);

	void EmpireChgNameUp(quint32 empireId, QString name);

	void EmpireChgRoleUp(quint32 empireId, quint8 gameRole);

	void EmpireCityWorkerSpeciesChanged(quint32 empireId, AuId cityId, AuSpecies workerSpecies);

	void EmpireRecordChanged(quint32 idEmpire);

	void EmpireRecordDeleted(quint32 idEmpire);

	void EmpireRecordLoaded(quint32 idEmpire);

	// Records a patent in an empire.
	void EmpireRecordPatent(quint32 empireId, quint16 patentCID, quint8 patentQ);

	void EstablishNewEmpire(AuId personId, const QString& personName, quint8 personGender, quint32 idOldEmpire, const QString& name,
			const QImage& imFlag);

	void GovBossChanged(quint32 empireId, AuId governorId, AuId bossId);

	void HandoffAtomDown(AuId personOrShip, AuId dest, quint8 rideAlso, bool limboRescue, QByteArray atomData, AuId returnOnFail);

	void HandoffAtomUp(AuId personOrShip, AuId dest, quint8 rideAlso, bool limboRescue, QByteArray atomData, AuId returnOnFail);

	void HandoffElsewhere(AuId personOrShip, AuId dest, quint8 rideAlso, bool limboRescue, AuId returnOnFail);

	void LeaveEmpire(AuId personId, QString personName, quint32 idOldEmpire);

	// Initiated by server manager via DtMgrChannel
	void LimboRescue(AuId atomId);

	void NameSpeciesUp(AuSpecies species, QString specieName, AuId discoverer);

	void NewSectorStored(AuId idSector);

	void OfficeAssigned(AuId person, quint32 empire, AuOffice office);

	void OfficeRevoked(AuId person, quint32 empire);

	// 'propagateUp' is true to allow the channel to span servers in its quest for the target atom.
	void OpenChannelTo(AuId fromUnitId, QString fromName, AuId fromSector, quint8 fromGalaxy, AuId toUnitId, QString channelName,
			quint8 scope, bool propagateUp);

	void AvatarOnLineDown(AuId personId, quint32 empireId);

	void AvatarOnLineUp(AuId personId, quint32 empireId);

	void PostJobOffer(AuJobOffer jobOffer);

	void PostJobOfferDown(AuJobOffer jobOffer);

	// Reloads a specific story 'scriptId'.
	// 'scriptId' 0 reloads all story scripts.
	void ReloadScript(quint32 scriptId);

	void RenameAvatarUp(AuId avatarId, QString nuName);

	void RenameCityUp(AuId cityId, QString nuName);

	void RenamePlanetDown(AuId planetId, QString nuName, AuId systemId, QString systemName, AuId sectorId, QString sectorName);

	void RenamePlanetUp(AuId planetId, QString nuName, AuId systemId, QString systemName, AuId sectorId, QString sectorName);

	void RenameSectorDown(AuId sectorId, QString nuName);

	void RenameSectorUp(AuId sectorId, QString nuName);

	void RenameSystemDown(AuId systemId, QString nuName);

	void RenameSystemUp(AuId systemId, QString nuName);

	// Initiated by server manager via DtMgrChannel
	void RestoreEmpire(DgEmpire empire);

	//void SectorVisitedDown(AuId sectorId, quint32 empireId);

	//void SectorVisitedUp(AuId sectorId, quint32 empireId);

	void SpeciesChanged(AuSpecies species, QString specieName, AuId whoNamed);

	void GovCitySurrenderedDown(AuId byWho, AuId cityId, quint32 fromEmpire, quint32 toEmpire);

	void GovCitySurrenderedUp(AuId byWho, AuId cityId, quint32 fromEmpire, quint32 toEmpire);

	// 'docTime' and 'docType' together are used to identify the exact document.
	void GovDossierChangeStateUp(quint32 empire, quint32 dosEmpire, AuTime docTime, quint8 docType, quint16 docIndex,
			quint8 docState);

	void GovSetAnnouncementsUp(quint32 empire, QString announcements);

	// 'sector' is NULL is setting empire capital
	// 'capitalType' 0=sector, 1=empire, 3=galaxy
	void GovSetCapitalUp(quint32 empire, AuId city, AuSpecies workerSpecies, AuId domain, int capitalType);

	void GovSetCommerceUp(quint32 empire, QByteArray commerce);

	void GovSetContrailColorUp(quint32 empire, quint8 hue, quint8 sat);

	void GovSetDefaultPoliticalStanceUp(quint32 empire, quint8 stance);

	void GovSetJoinPasswordUp(quint32 empire, QByteArray passCrypt);

	void GovSetTaxUp(quint32 empire, quint8 income, quint8 sales, quint8 subsidy);

	// 'theme' is a AuScFace::COLOR_THEME enum value.
	void GovSetThemeColorUp(quint32 empire, quint8 theme, QColor color);

	void GovSetWarCriteria(quint32 empire, AuWarCriteria crit);

	void HengeTeleportAtomDown(AuHengeTeleportAtom hengeTel);

	void OfficerKilledDown(const AuId homePlanetId);

	void OfficerKilledUp(const AuId homePlanetId);

	void PersonSystemChanged(AuId systemId, quint32 playerId);

	// 'playerId' is a login account id, not an atom id
	void PlayerConnectingChat(quint32 playerId, unsigned long threadId);

	// Used to locate spawn location atoms when a player dies, hence
	// the empire id of allegiance. The empire id is ignored if it is 0.
	// 'personToSpawn' is set when searching for a spawn point for a person,
	// This helps some spawn points (space ships) decide their suitability
	// as a spawn point. 'personToSpawn' is ignored if 'empire' is 0.
	void QueryAtomServer(AuId idAtom, quint32 empire, AuId personToSpawn);

	void QueryHengeTeleporterServer(AuHengeTeleportAtom hengeTel);

	// Revokes the assignment of 'systemId' from whatever subserver it is currently assigned to and assigns it to a specific
	// worker server. The worker server port is going to be the server manager port because that is who calls this.
	void ReassignSystem(AuId systemId, QString wrkServerName, quint16 wrkServerPort);

	// ReleaseBerthDown gets the id of the nu ship to which the person is
	// assigned, which could be 0 if they are simply releasing their berth.
	// This helps insure the berth/roster data doesn't get out of sync.
	// The logic causes all ships that are not the ship id to remove the
	// person's id from their roster, if it's there. That way a person can't
	// accidentally end up consuming berths on multiple different ships.
	// This slot is used when propagating the ReleaseBerth down to all the
	// worker servers.
	void ReleaseBerthDown(AuId personId, AuId shipId);

	// ReleaseBerthUp gets the id of the nu ship to which the person is
	// assigned, which could be 0 if they are simply releasing their berth.
	// This helps insure the berth/roster data doesn't get out of sync.
	// The logic causes all ships that are not the ship id to remove the
	// person's id from their roster, if it's there. That way a person can't
	// accidentally end up consuming berths on multiple different ships.
	// This slot is used when propagating the ReleaseBerth up to the master
	// server.
	void ReleaseBerthUp(AuId personId, AuId shipId);

	void RenameWormhole(const AuId systemId, const AuId toSystem, qint8 polarity, quint32 empire, const QString nuName);

	// Revokes the assignment of 'atomId' from whatever subserver it is currently assigned to.
	// It only makes sense to call this with sector ids on the scene master.
	// It only makes sense to call this with solar system ids on the scene manager.
	void RevokeAssignment(AuId atomId);

	void SectorAssigned(AuId idSector, int systemsInSector, quint8 galaxy);

	void SectorRevoked(AuId idSector);

	// 'fromGalaxy' is the GALAXY of the sender.
	void SendFundsDown(AuId fromId, QString fromName, AuId toId, double amount, quint8 fromGalaxy, quint8 reason, quint32 designId);

	// 'fromGalaxy' is the GALAXY of the sender.
	void SendFundsUp(AuId fromId, QString fromName, AuId toId, double amount, quint8 fromGalaxy, quint8 reason, quint32 designId);

	void Shutdown(QString warnMsg, QString haltMsg, qint16 shutdownSecs);

	void SolarSystemExplored(const AuId systemId, quint32 empireId);		// On the way up the server chain.

	void SolarSystemExploredDown(const AuId systemId, quint32 empireId);	// On the way down the server chain.

	// 'genHab' is a HABITABILITY enum
	void SolarSystemScanned(const AuId systemId, quint32 empireId, quint8 genHab, quint32 genSeed);		// up the server chain

	// 'genHab' is a HABITABILITY enum
	void SolarSystemScannedDown(const AuId systemId, quint32 empireId, quint8 genHab, quint32 genSeed);	// down the server chain

	void SolarSystemFinal(AuId systemId);

	void SolarSystemReadyToDecay(AuId systemId);

	// 'galaxy' is a GALAXY enum value
	void SpawnAtom(AuId person, quint32 empireId, AuId spawnCity, AuId spawnShip, AuId arena, quint8 galaxy);

	// 'galaxy' is a GALAXY enum value
	void SpawnAtomDown(AuId person, quint32 empire, AuId spawnCity, AuId spawnShip, AuId spawnAt, AuId arena, quint8 galaxy);

	// 'galaxy' is a GALAXY enum value
	void SpawnAtomUp(AuId person, quint32 empireId, AuId spawnCity, AuId spawnShip, AuId arena, quint8 galaxy);

	void HengeTeleportAtomUp(AuHengeTeleportAtom hengeTel);

	// 'sectorOrSystemId' is 0 on master, sector id on manager, and solar system id on worker.
	// 'galaxy' is a GALAXY enum value
	void SpawnPersonOnPlanet(AuId person, AuId sectorOrSystemId, quint8 galaxy);

	void SpeciesRecordLoaded(AuSpecies species);

	void StoryRecordChangedDown(quint32 storyInstance);
	void StoryRecordChangedUp(quint32 storyInstance);
	void StoryRecordDeletedDown(quint32 storyInstance);
	void StoryRecordDeletedUp(quint32 storyInstance);

	void SystemAssigned(AuId idSector, AuId idSystem, quint8 galaxy);

	void SystemRevoked(AuId idSystem);

	//void TravelElsewhere(AuId id, AuId destSystem, AuId destSector, QByteArray atomData);

	//void TransferAtomUp(AuId id, AuId destSystem, AuId destSector, QByteArray atomData);

	void WormholeExplored(AuId systemId, AuId toSystem, qint8 polarity, quint32 empire, QString nuName);

	// Sets political stance of 'fromEmpire' toward 'toEmpire' as a result of initial contact between units of two empires.
	// 'fromStance' is the stance taken by the fromEmpire.
	// 'vassalTributePercent' is relevant when becoming a vassal.
	// 'fromUnit' is the unit in 'fromEmpire' who triggered this stance setting.
	// 'toStance' is the stance taken by the toEmpire.
	// 'toUnit' is the unit in 'fromEmpire' who triggered this stance setting.
	// 'reason' is an AuDocument::Type() value, for the dossier.
	// 'galaxy', 'sector', 'system' and 'world' identify where this incident occurred. System and world may be blank.
	void SetPoliticalStance(quint32 toEmpire, quint32 fromEmpire, quint8 fromStance, quint8 vassalTributePercent,
			AuId fromUnit, QString fromUnitName, quint8 toStance, AuId toUnit, QString toUnitName,
			quint8 reason, quint8 galaxy, QString sector, QString system, QString world);

	void SystemNameChangedDown(AuId systemId, QString systemName, AuId sectorId, QString sectorName);

	void SystemNameChangedUp(AuId systemId, QString systemName, AuId sectorId, QString sectorName);

signals:
	void AddEmpireToDb(quint32);
	void AddSpeciesToDb(AuSpecies species);

	void ArenaReadyDownSignal(AuArenaReady arena);

	void AtomPresenceResolved(AuId atom, quint32 empire, QString netname, quint16 port, QString peerName);

	void AvatarAccountActiveSignal(AuIdList land);

	void BlueprintRatedBdSignal(AuExchangeRatingRec rating);

	void BlueprintRatedScSignal(AuExchangeRatingRec rating);

	void BroadcastBountyPosted(AuId posterId, QString posterName, AuId victimId, QString reason, double reward, int why);

	void BroadcastJobOffer(AuJobOffer jobOffer);

	void ChangeEmpireInDb(quint32);
	void ChangeSpeciesInDb(AuSpecies species);
	void ChatToClients(QByteArray msg);

	void CityHistoryReportReady(AuId unitId, AuId cityId, AuCityReportHistory report);

	void CityLastReportReady(AuId unitId, AuId cityId, AuCityReport report, bool havePrevReport, AuCityReport prevReport);

	// Propagate city mail request down the server farm.
	void CityMailRequestedSignal(AuId cityId, AuId unitId, quint32 unitEmpire);

	// 'cityId' is the id of the city for which a report is requested.
	// 'reportWhat': 0=last city report, 1=city report history
	// 'unitId' is the DtChatChannel m_idUnit who requested the report.
	// 'unitEmpire' must match the empire id of the city to get a report. 0 ignores this check.
	void CityReportRequested(AuId cityId, quint8 reportWhat, AuId unitId, quint32 unitEmpire);

	void CloseStargateDownSignal(AuId fromGalaxy, AuPoint fromGalaxyLoc);

	void CollectBountySignal(AuId bountyHunter, QString bountyHunterName, AuGearPos gearPos, AuBountyOnItem bountyOnItem);

	void EnteredDesigner(AuId personId, int designStudioMode);

	void HengeDestinationDownSignal(AuId sectorId, AuId systemId, QString systemName, quint8 sun, QString worldCatName, QString worldName,
			quint8 worldType, AuGlobeLLF hengeLL, bool breathable, quint8 orbitZone, bool outsideTown);

	void PirateBaseEncountered(AuId inSystemId);

	void SpacecraftNameChangedSignal(AuId shipId, QString nuName);

	// Notification down to all workers that a system consumed power. Sent by systems once per hour.
	// 'galaxy' is a GALAXY enum value.
	void SystemWarpSignatureDown(AuId systemId, quint32 warpFactors, quint8 galaxy, float galaxyLocX, float galaxyLocY,
			float galaxyLocZ);

	void DisconnectExcessChatChannels(quint32 playerId, unsigned long exceptThreadId);
	void DisconnectExcessSceneChannels(quint32 playerId, unsigned long exceptThreadId);

	void DisconnectUnit(AuId unitId);

	// Emitted when a unit is to be disconnected because it was transferred to another player's account.
	void DisconnectUnitTransferred(AuId unitId);

	void EmpireDeleted(quint32 empireId);

	// emitted to send the empire record to all subordinate servers.
	void EmpireDoneChanging(DgEmpire empire);

	void EmpireRecordAvailable(quint32 empireId);

	void GameOver(AuId ship, quint16 whichTable, quint8 gameType, QByteArray gameResult);

	void GameTurn(AuId ship, quint16 whichTable, AuId personsTurn);

	void MgrSendPacket(QByteArray ba);

	void PayBountySignal(AuId bountyHunter, AuGearPos gearPos, AuBountyOnItem bountyOnItem, bool victimHadBounty);

	// Notification to DtSceneServer threads.
	void SolarSystemChanged(AuId systemId, AuId sectorId);

	void OpenChannelToUnit(AuId fromUnitId, QString fromName, AuId fromSector, quint8 fromGalaxy, AuId toUnitId,
			QString channelname, quint8 scope);

	// Paves the way to the destination stargate solar system and creates it.
	void OpenStargateDownSignal(AuId fromGalaxy, AuPoint fromGalaxyLoc, AuId toGalaxy, AuPoint toGalaxyLoc, AuId toSector,
			AuId toSystem, AuTime expire);

	// Emitted to broadcast person emitted signal to all sub workers.
	void PersonDeleted(AuId personId);

	void QueryAtomServerDown(AuId idAtom, quint32 empire, AuId personToSpawn);

	void QueryHengeTeleporterServerDown(AuHengeTeleportAtom hengeTel);

	void RecoverAtomFromLimbo(AuId atomId);

	void ReloadData();

	void RenamePlanet(AuId planetId, QString nuName, AuId systemId, QString systemName, AuId sectorId, QString sectorName);

	void RenameSector(AuId sectorId, QString nuName);

	void RenameSystem(AuId systemId, QString nuName);

	void BdDesignDeactivatedSignal(quint32 designId);

	void BdDesignExchangePublishedSignal(AuBdDesignExchangeEntry item);

	void BdDesignExchangeUnpublishedSignal(quint32 designId);

	void ItDesignDeactivatedSignal(quint32 designId);

	void ScDesignDeactivatedSignal(quint32 designId);

	void ScDesignExchangePublishedSignal(AuScDesignExchangeEntry item);

	void ScDesignExchangeUnpublishedSignal(quint32 designId);

	void SceneAtomChanged(AuId oldPersonid, AuId nuPersonid);

	void SectorAssignmentReceived(AuId sectorId);
	void SectorAssignmentRevoked(AuId sectorId);
	void SectorAssignmentsCleared();

	void SectorDecayed(AuId sectorId);

	void PlayersOnlineStatusSignal(AuPlayerOnlineStatusVector players);

	void SetPersonSystem(AuId systemId, quint32 playerId);

	void SpacecraftStasisRecoverSignal(AuId shipId, AuId personId, quint8 personGalaxy, bool agentRecover);

	void HengeTeleportAtomViaDb(AuHengeTeleportAtom hengeTel);

	void HengeTeleportArrivedViaDb(AuHengeTeleportAtom hengeTel);

	void ClaimTitleSignal(AuId propertyId, QString propertyName, AuId ownerId, QString ownerName, quint32 ownerEmpire);

	// 'galaxy' is a GALAXY enum value
	void SpawnAtomViaDb(AuId person, quint32 empireId, AuId spawnCity, AuId spawnShip, AuId arena, quint8 galaxy);

	void SpeciesNameChanged(AuSpecies species, QString specieName, AuId discoverer);

	void SpeciesRecordAvailable(AuSpecies species);

	void SystemAssignmentReceived(AuId systemId);
	void SystemAssignmentRevoked(AuId systemId);
	void SystemAssignmentsCleared();
	void TravelerArrivedViaDb(AuId traveler, AuId destSystem, AuId destSector);

	// When traveling via intergalactic wormhole, 'destSystem' and 'destSector' are the destination galaxy id.
	void TravelerDepartedViaDb(AuId traveler, AuId origSystem, AuId destSystem, AuId destSector);

	void TravelerDepartedGalaxyLocViaDb(AuId traveler, AuId origSystem, AuId destGalaxy, AuPoint destGalaxyLoc);

	// When traveling via intergalactic wormhole, 'destSystem' and 'destSector' are the destination galaxy id.
	void TravelerDepartedStargateViaDb(AuId traveler, AuId origSystem, AuId destGalaxy, AuPoint destGalaxyLoc);

	void UdpPacketReceived(quint32 idPlayer, QByteArray packet);
	void UnitDisconnected(AuId unitId);

	void WormholeExploredSignal(AuId systemId, AuId toSystem, qint8 polarity, quint32 empire, QString nuName);

protected:
	DhSceneServer AssignSectorToManager(int which, unsigned int avoidServerId);

	DhSceneServer AssignSolarSystemToWorker(int which, unsigned int avoidServerId, unsigned int specificServerId);

	void AtomChat();

	void AtomManagerLoadReset();

	void BalanceAtomManagerLoads();

	void BalanceLoads();

	void CommAcceptCitySurrender(const AuCommCitySurrenderAccepted& msg);

	void CommAcceptColonistsBoarded(DaPerson* unloadPerson, const AuCommPassengersBoarded& msg);

	void CommAcceptColonistsDebarked(DaPerson* loadPerson, const AuCommPassengersDebarked& msg);

	void CommAcceptCryoBoarded(DaPerson* unloadPerson, const AuCommCryoBoarded& msg);

	void CommAcceptCryoDebarked(DaPerson* loadPerson, const AuCommCryoDebarked& msg);

	void CommAcceptTeleport(DaPerson* fromPerson, const AuCommPacket& msg);

	void CommAcceptTroopsBoarded(DaPerson* unloadPerson, const AuCommTroopsBoarded& msg);

	void DeactivateAvatars();

	void DeassignLostSectors();

	void DeassignLostSolarSystems();

	void HandoffAtoms();

	void HengeTeleportAtoms();

	void JoinNewEmpire(const AuId& personId, const QString& personName, quint8 personGender, quint32 idOldEmpire,
			quint32 idNewEmpire);

	void ListSystemsToRevoke(unsigned int serverId, float loadToRevoke, AuIdList& revokeSystems);

	bool NotifyOfficerKilled(const AuId& homePlanetId);

	void OpenDelayedStargates();

	void PlayerStatusBroadcast();

	QStringList ReportMemoryUsage(bool noLocks);

	// Revokes the assignment of 'sectorId' from whatever sub manager server it is currently assigned to.
	// Only call this on the master server.
	void RevokeSector(const AuId& sectorId);

	// Revokes the assignment of 'systemId' from whatever sub worker server it is currently assigned to.
	// Only call this on the manager server.
	void RevokeSolarSystem(const AuId& systemId);

	void SendDepartingAtoms();

	bool SendFunds(const AuId& fromId, const QString& fromName, const AuId& toId, double amount, GALAXY fromGalaxy, quint8 reason,
			quint32 designId);

	void SendMail(AuMailMsg::MSGTYPE msgType, const AuId& fromId, const QString& fromName, const AuId& toId,
			const QString& subject, const QString& bodyText) const;

	bool SendPacketTo(const QByteArray& ba, unsigned int serverId);

	void SpawnAtoms();

	// 'sectorOrSystemId' is 0 on master, sector id on manager, and solar system id on worker.
	// NOTE: m_SpawnMutex must be locked before calling this function.
	void SpawnPersonOnPlanet2(const AuId& person, const AuId& sectorOrSystemId, GALAXY galaxy);

	// 'sectorOrSystemId' is 0 on master, sector id on manager, and solar system id on worker.
	// NOTE: m_SpawnMutex must be locked before calling this function.
	void SpawnPersonOnPlanet3(const AuId& person, const AuId& sectorOrSystemId, GALAXY galaxy);

	void StartSceneServer();

	void StartServerChannel();

	void StartServerManager();

	void StartThreads();

	void StopSceneServer();

	void StopServerChannel();

	void StopServerManager();

	void StopThreads();

	// Statistics about ticking atoms, by system. Return values are in 1/2 millisecond increments.
	// 'systemTotal' is sum of average tick times.
	// 'systemMax' is sum of max tick times.
	// 'systemCycle' is average of average cycle times.
	// 'systemCycleMax' is average of max cycle times.
	void TallySystemTickTotals(QHash<AuId, unsigned int>& systemTotals, QHash<AuId, unsigned int>& systemMax,
			QHash<AuId, unsigned int>& systemCycle, QHash<AuId, unsigned int>& systemCycleMax) const;

	// The Etherium delete lock is expected to already be locked.
	bool AdminGoTo2(const AuId& personOrShip, const AuId& dest, quint8 rideAlso, AuEther& ether);

	bool AtomTraveledDown2(const AuId& id, const AuId& destSystem, const AuId& destSector, DaAtom* managedAtom);

	void AtomTraveledUp2(const AuId& id, const AuId& destSystem, const AuId& destSector, DaAtom* managedAtom);

	void AtomTraveledGalaxyLocUp2(const AuId& id, const AuId& destGalaxy, const AuPoint& destGalaxyLoc, DaAtom* managedAtom);

	void AtomTraveledStargateUp2(const AuId& id, const AuId& destGalaxy, const AuPoint& destGalaxyLoc, DaAtom* managedAtom);

	void HandoffAtomDown2(const AuId& personOrShip, const AuId& dest, quint8 rideAlso, bool limboRescue, const QByteArray& atomData,
			int timeOuts, const AuId& returnOnFail);

	void HandoffAtomUp2(const AuId& personOrShip, const AuId& dest, quint8 rideAlso, bool limboRescue, const QByteArray& atomData,
			const AuId& returnOnFail);

	// The Etherium delete lock is expected to already be locked.
	void HandoffElsewhere2(AuId personOrShip, AuId dest, quint8 rideAlso, bool limboRescue, AuId returnOnFail, AuEther& ether);

	void HengeTeleportAtomDown2(const AuHengeTeleportAtom& hengeTel);

	void HengeTeleportAtomUp2(const AuHengeTeleportAtom& hengeTel);

	void SpawnAtomUp2(const AuId& person, quint32 empire, const AuId& spawnCity, const AuId& spawnShip, const AuId& arena,
			GALAXY galaxy);

	void SpawnAtomDown2(const AuId& person, quint32 empire, const AuId& spawnCity, const AuId& spawnShip, const AuId& capitolU,
			const AuId& capitolG, const AuId& randomCity, const AuId& arena, GALAXY galaxy);

	bool SummonGoTo2(const AuId& shipId, const AuId& crewId, AuEther& ether);

	void UnloadUnusedData();

protected slots:
	void ClientChannelLost();
	void DbFinishedInit();
	void ReadPendingDatagrams();
	void ServerChannelClosed(unsigned int channelId);
	void Shutdown();
	void UpdateStats();

private:
	DgData*								m_Data;
	AuEther*							m_Etherium;				// The global etherium.
	AuGLModelCache*						m_ModelCache;
	QTimer*								m_timer;
	AuElapsedTimer						m_tmBalance;			// Time of last load balance.
	AuElapsedTimer						m_tmUnload;				// Time of last unload of unused data.
	AuTime								m_tmUp;					// Time this object was constructed. For uptime.

	QString								m_LoginName;			// For login to server farm.
	QByteArray							m_LoginPassCrypt;		// For login to server farm. SHA-1 hash of password.
	quint16								m_ScenePort;			// Port on which to listen for scene clients
	quint16								m_ChatPort;				// Port on which to listen for scene client chat communications.
	quint16								m_MgrPort;				// Port on which to listen for server manager connections.
	QString								m_ClientChannelServer;

	// This server.
	int									m_AtomManagerCount;		// Number of atom managers to create. Default is idealThreadCount()
																// but can be overridden with command line option.
	unsigned int						m_AtomMgrLoadIndex;		// Which load index we are timing, in the array.
	float								m_AtomMgrLoads[60];		// Load value measured at each of the last intervals.
	bool								m_AtomMgrLoadsValid;	// true after we have accumulated all mgr time interval values.

private:
	mutable QReadWriteLock				m_rwAtomManagers;		// Guards m_MySectors, m_MySystems.
	unordered_map<AuId, AuMySector, AuIdHash>	m_MySectors;	// Sectors assigned to me. Manager server only.
	unordered_map<AuId, AuAtomManaged*, AuIdHash>	m_MySystems;	// Solar systems assigned to me. Worker server only.

private:
	// Use the root mutex to access nu sector ids.
	AuIdList		m_NewSectors;	// Only used on master.
	mutable QReadWriteLock	m_rwNewSectors;	// Guards m_NewSectors

	unordered_multimap<AuId, AuOpenStargate, AuIdHash>	m_PendingStargates;	// Key is sector id on master, system id on manager and worker.
	QMutex			m_muPendingStargates;	// Guards m_PendingStargates.

	QMap<AuId, AuAtomDeparture>	m_PendingDepartures;	// Only used on master and manager.
	QMutex			m_muPendingDep;	// Guards m_PendingDepartures.

private:
	// Subordinate servers.
	QMutex								m_AssignmentsLock;
	unordered_map<AuId, unsigned int, AuIdHash>				m_Reassignments;		// Reassignments from/to subordinate servers.
	unordered_map<AuId, unsigned int, AuIdHash>				m_Revokations;			// Revokations from subordinate servers.
	unordered_map<AuId, AuSectorAssignment, AuIdHash>		m_SectorAssignments;	// Sectors assigned to subs of master server.
	unordered_map<AuId, AuSystemAssignment, AuIdHash>		m_SystemAssignments;	// Systems assigned to subs of manager server.

private:
	AuServerChannel	m_ServerChannel;	// Connections with worker servers.
	AuSceneServer	m_TcpServer;	// Connections with scene clients.
	AuChatChannel	m_ChatServer;	// Connections with scene clients for chat traffic, only on scene worker.
	AuMgrChannel	m_MgrServer;	// Connections with server managers.
	unsigned int	m_UdpBytesRx;	// count of udp bytes received
	unsigned int	m_UdpBytesTx;	// count of udp bytes sent
	unsigned int	m_UdpPacketsRx;	// count of udp packets received
	unsigned int	m_UdpPacketsTx;	// count of udp packets sent
	QUdpSocket		m_UdpSocket;

	// Threads
	DtClientChannel*	m_tClientChannel;	// Channel to boss if I'm not SceneMaster.
	DtDb*			m_tDb;			// Keeps local data in sync with the database.
	DtMailClient*	m_tMailClient;	// So tick processes can send mail to players.
	DtTrash*		m_tTrash;		// Deletes atoms from memory.

	// Handoff requests are only initiated on the scene master.
	// Scene manager tracks them as they are passed down to workers.
	QMutex			m_HandoffMutex;	// Controls m_Handoffs.
	QList<AuHandoffRequest>	m_Handoffs;	// Handoff requests awaiting atom queries.

	// Spawn requests are only initiated on the scene master.
	// Scene manager tracks them as they are passed down to workers.
	QMutex			m_SpawnMutex;	// Controls m_Spawns.
	QList<AuSpawnRequest>	m_Spawns;	// Spawn requests awaiting atom queries.
	QList<AuHengeTeleportRequest>	m_HengeTeleports;	// Henge teleport requests awaiting atom queries.
	unordered_map<AuId, AuSpawnArrival, AuIdHash>	m_ArrivingSpawnsOnPlanets;	// Spawns waiting for a planet to be loaded by the current server after handoff.
	unordered_map<AuId, AuSpawnDeparture, AuIdHash>	m_DepartingSpawnsOnPlanets;	// Spawns waiting for a planet to be created on a sub server for handoff.

	// Atom queries
	QMutex			m_QueryMutex;	// Controls m_AtomQueries
	QList<AuAtomQuery>	m_AtomQueries;	// Unresolved atom queries.
	QList<AuHengeTeleportDestQuery>	m_HengeTeleportDestQueries;	// Unresolved henge destination queries.

	// Shutdown
	QString			m_ShutdownHalt;		// Final message to send when shutting down.
	int				m_ShutdownSeconds;	// Seconds remaining to shutdown upon ordered to shutdown. 0 when not shutting down.
	QString			m_ShutdownWarn;		// Message to send during incremental warnings.

	// Job offers
	AuElapsedTimer						m_JobOfferLast;			// Our rate throttle for accepting job offers.

	AuElapsedTimer						m_AvatarDeactivateTime;	// Time of last query for inactive avatars and deactivation.

private:
	AuElapsedTimer						m_PlayerStatusTime;		// Time last player status was broadcast to sub servers.
	mutable QReadWriteLock				m_PlayerStatusLock;		// Controls m_PlayerStatus.
	AuPlayerOnlineStatusVector			m_PlayerStatus;			// Status of all players on line. Only maintained on worker.
};

#define auSceneServerApp	((AuSceneServerApp*)QCoreApplication::instance())

#endif // AuSceneServerApp_H
