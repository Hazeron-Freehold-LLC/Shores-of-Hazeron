//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtmgrclient.h,v 1.58 2022/02/19 18:40:44 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DtMgrClient_H
#define DtMgrClient_H

#include "aupoint.h"
#include "auplayeronlinestatus.h"
#include "daatom.h"
#include "dgempire.h"
#include <QMetaType>
#include <QMutex>
#include <QSemaphore>
#include <QStringList>
#include <QTcpSocket>
#include <QTime>
#include <QThread>

class AuThinkerReport
{
public:
	quint32			atomId;
	quint32			avgThinkTime;
	quint32			maxThinkTime;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Report item about a design in the server's sc or bd design caches.
class AuResourceDesign
{
public:
	quint32								m_Id;
	QString								m_Name;
	quint8								m_State;
	quint32								m_Triangles;

public:
	AuResourceDesign()
	{
		m_Id = 0;
		m_State = 0;
		m_Triangles = 0;
	}

	AuResourceDesign(const AuResourceDesign& c)
	{
		m_Id = c.m_Id;
		m_Name = c.m_Name;
		m_State = c.m_State;
		m_Triangles = c.m_Triangles;
	}

	void operator<<(QDataStream& in)
	{
		in >> m_Id;
		in >> m_Name;
		in >> m_State;
		in >> m_Triangles;
	}

	QString StateName() const
	{
		if(m_State > 3)
			return QString("Invalid state %1").arg(m_State);
		QString stateNames[] =
		{
			QObject::tr("Unfinished"),
			QObject::tr("Final"),
			QObject::tr("Deleted"),
			QObject::tr("Assembly"),
		};
		return stateNames[m_State];
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Report item about a 3ds model in the server's model cache.
class AuResourceModel
{
public:
	QString			m_FileName;
	quint16			m_Users;
	quint8			m_BuildingEtc;
	quint8			m_BuildingIns1;
	quint8			m_BuildingIns2;
	quint8			m_BuildingObs1;
	quint8			m_BuildingObs2;
	quint8			m_BuildingRamp;
	quint8			m_BuildingWpn;
	quint8			m_Screens;
	quint8			m_Stations;
	quint8			m_Weapons;
	quint16			m_Materials;
	quint16			m_Objects;
	quint16			m_VertexGroups;

public:
	AuResourceModel()
	{
		m_Users = 0;
		m_BuildingEtc = 0;
		m_BuildingIns1 = 0;
		m_BuildingIns2 = 0;
		m_BuildingObs1 = 0;
		m_BuildingObs2 = 0;
		m_BuildingRamp = 0;
		m_BuildingWpn = 0;
		m_Screens = 0;
		m_Stations = 0;
		m_Weapons = 0;
		m_Materials = 0;
		m_Objects = 0;
		m_VertexGroups = 0;
	}

	~AuResourceModel()
	{
	}

	AuResourceModel(const AuResourceModel& c)
	{
		m_FileName = c.m_FileName;
		m_Users = c.m_Users;
		m_BuildingEtc = c.m_BuildingEtc;
		m_BuildingIns1 = c.m_BuildingIns1;
		m_BuildingIns2 = c.m_BuildingIns2;
		m_BuildingObs1 = c.m_BuildingObs1;
		m_BuildingObs2 = c.m_BuildingObs2;
		m_BuildingRamp = c.m_BuildingRamp;
		m_BuildingWpn = c.m_BuildingWpn;
		m_Screens = c.m_Screens;
		m_Stations = c.m_Stations;
		m_Weapons = c.m_Weapons;
		m_Materials = c.m_Materials;
		m_Objects = c.m_Objects;
		m_VertexGroups = c.m_VertexGroups;
	}

	void operator<<(QDataStream& in)
	{
		in >> m_FileName;
		in >> m_Users;
		//in >> m_BuildingEtc;
		//in >> m_BuildingIns1;
		//in >> m_BuildingIns2;
		//in >> m_BuildingObs1;
		//in >> m_BuildingObs2;
		//in >> m_BuildingRamp;
		//in >> m_BuildingWpn;
		in >> m_Screens;
		in >> m_Stations;
		in >> m_Weapons;
		in >> m_Materials;
		in >> m_Objects;
		in >> m_VertexGroups;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Report about resource usage on a server.

class AuResourceReport
{
public:
	QString			m_ServerName;	// Server that generated this report.
	quint16			m_ServerPort;	// Server that generated this report.

	// Atoms
	QHash<DaAtom::ATOM_TYPE, quint32>	m_Atoms;
	quint32			m_dead;
	quint32			m_dying;
	quint32			m_travel;
	QStringList		m_atomsInMotion;
	QStringList		m_peopleOnLine;

	// Trash
	QHash<DaAtom::ATOM_TYPE, quint32>	m_Trash;
	quint32			m_deadTrash;
	quint32			m_dyingTrash;
	quint32			m_travelTrash;

	// Model Cache
	quint8			m_hasCache;
	QList<AuResourceModel>	m_modelCache;

	QList<AuResourceDesign>	m_BdDesigns;
	QList<AuResourceDesign>	m_ScDesigns;

	// Thinkers and Tickers
	QList<AuThinkerReport>	m_tickers;
	QList<AuThinkerReport>	m_tockers;

public:
	AuResourceReport()
	{
		m_ServerPort = 0;

		// Atoms
		m_dead = 0;
		m_dying = 0;
		m_travel = 0;
		
		// Trash atoms
		m_deadTrash = 0;
		m_dyingTrash = 0;
		m_travelTrash = 0;

		// Model Cache
		m_hasCache = 0;
	}

	~AuResourceReport()
	{
	}

	AuResourceReport(const AuResourceReport& c) :
		m_Atoms(c.m_Atoms),
		m_atomsInMotion(c.m_atomsInMotion),
		m_peopleOnLine(c.m_peopleOnLine),
		m_Trash(c.m_Trash),
		m_modelCache(c.m_modelCache),
		m_BdDesigns(c.m_BdDesigns),
		m_ScDesigns(c.m_ScDesigns),
		m_tickers(c.m_tickers),
		m_tockers(c.m_tockers)
	{
		m_ServerName = c.m_ServerName;
		m_ServerPort = c.m_ServerPort;
		m_dead = c.m_dead;
		m_dying = c.m_dying;
		m_travel = c.m_travel;
		m_hasCache = c.m_hasCache;

		m_deadTrash = c.m_deadTrash;
		m_dyingTrash = c.m_dyingTrash;
		m_travelTrash = c.m_travelTrash;
	}

	quint32 TotalAtoms() const;

	quint32 TotalTrash() const;
};

Q_DECLARE_METATYPE(AuResourceReport)	// Needed to use class in queued signal/slot connections. Must also call qRegisterMetaType.

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Report about resource usage on a server.

class AuResourceReportAtom : public AuResourceReport
{
public:
	quint32					m_AtomId;
	unsigned char			m_AtomType;			// DaAtom::Type(). 0=atom not found on server
	bool					m_AtRest;			// DaAtom::IsAtRest
	QString					m_Desc;				// DaAtom::Description()
	bool					m_InScene;			// DaAtom::IsInAnyScene
	QString					m_Name;				// DaAtom::Name()
	QString					m_SpotDesc;			// DaAtom::SpotDescription(0)
	QDateTime				m_TimeCreated;
	QDateTime				m_TimeModified;
	QDateTime				m_TimeScene;		// DaAtom::SceneTime
	QDateTime				m_TimeTicked;
	QDateTime				m_TimeTocked;
	double					m_VecX;
	double					m_VecY;
	double					m_VecZ;

public:
	AuResourceReportAtom()
	{
		m_AtomId = 0;
		m_AtomType = 0;
		m_AtRest = false;
		m_InScene = false;
		m_VecX = 0;
		m_VecY = 0;
		m_VecZ = 0;
	}

	~AuResourceReportAtom()
	{
	}

	AuResourceReportAtom(const AuResourceReportAtom& c) : AuResourceReport(c)
	{
		m_AtomId = c.m_AtomId;
		m_AtomType = c.m_AtomType;
		m_AtRest = c.m_AtRest;
		m_Desc = c.m_Desc;
		m_InScene = c.m_InScene;
		m_Name = c.m_Name;
		m_SpotDesc = c.m_SpotDesc;
		m_TimeCreated = c.m_TimeCreated;
		m_TimeModified = c.m_TimeModified;
		m_TimeScene = c.m_TimeScene;
		m_TimeTicked = c.m_TimeTicked;
		m_TimeTocked = c.m_TimeTocked;
		m_VecX = c.m_VecX;
		m_VecY = c.m_VecY;
		m_VecZ = c.m_VecZ;
	}
};

Q_DECLARE_METATYPE(AuResourceReportAtom)	// Needed to use in queued signal/slot connections. Must also call qRegisterMetaType.

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Status about an atom that is managed on a server.

class AuStatusAtom
{
public:
	quint32			m_AtomId;
	unsigned int	m_AvgTick;			// In 1/2 millisecond increments. i.e. 100 = 50ms
	unsigned int	m_MaxTick;			// In 1/2 millisecond increments. i.e. 100 = 50ms
	unsigned int	m_AvgTickCycle;		// In 1/2 millisecond increments. i.e. 100 = 50ms
	unsigned int	m_MaxTickCycle;		// In 1/2 millisecond increments. i.e. 100 = 50ms
	unsigned int	m_AvgTock;			// In 1/2 millisecond increments. i.e. 100 = 50ms
	unsigned int	m_MaxTock;			// In 1/2 millisecond increments. i.e. 100 = 50ms

public:
	// Format of this stream established in AuSceneServerApp::StatusReportLoads.
	inline void operator<<(QDataStream& in)
	{
		in >> m_AtomId;
		in >> m_AvgTick;
		in >> m_MaxTick;
		in >> m_AvgTickCycle;
		in >> m_MaxTickCycle;
		in >> m_AvgTock;
		in >> m_MaxTock;
	}

public:
	inline bool IsDeadlocked() const
	{
		return m_AvgTick == 0xffffffff || m_AvgTock == 0xffffffff;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Status about a client that is connected to a server.

class AuStatusClient
{
public:
	quint32		m_PlayerId;		// Id of player's login account.
	ACCOUNT		m_AccountType;	// ACCOUNT
	QString		m_UnitName;		// Name of person atom.
	quint32		m_UnitId;
	quint32		m_UnitEmpire;
	quint32		m_SolarSystem;	// Solar system that contains the unit.
	QDateTime	m_ConnectTime;	// When they connected.
	quint32		m_BytesTx;		// Tx'd to the client.
	quint32		m_BytesRx;		// Rx'd from the client.
	quint16		m_BadPackets;
	quint32		m_SyncCount;
	quint16		m_SyncAvgAtoms;
	quint32		m_SyncAvgBytes;
	quint16		m_SyncAvgPing;
	quint16		m_SyncLastAtoms;
	quint32		m_SyncLastBytes;
	quint16		m_SyncLastPing;

public:
	AuStatusClient()
	{
		m_AccountType = ACCT_Count;
		m_PlayerId = 0;
		m_UnitId = 0;
		m_UnitEmpire = 0;
		m_SolarSystem = 0;
		m_BytesTx = 0;
		m_BytesRx = 0;
		m_BadPackets = 0;
		m_SyncCount = 0;
		m_SyncAvgAtoms = 0;
		m_SyncAvgBytes = 0;
		m_SyncAvgPing = 0;
		m_SyncLastAtoms = 0;
		m_SyncLastBytes = 0;
		m_SyncLastPing = 0;
	}

	// Format of this stream established in AuSceneServerApp::StatusReportLoads.
	void operator<<(QDataStream& in)
	{
		in >> m_PlayerId;
		quint8 ui8;
		in >> ui8;
		m_AccountType = (ACCOUNT)ui8;
		in >> m_UnitName;
		in >> m_UnitId;
		in >> m_UnitEmpire;
		in >> m_SolarSystem;
		in >> m_ConnectTime;
		in >> m_BytesTx;
		in >> m_BytesRx;
		in >> m_BadPackets;
		in >> m_SyncCount;
		in >> m_SyncAvgAtoms;
		in >> m_SyncAvgBytes;
		in >> m_SyncAvgPing;
		in >> m_SyncLastAtoms;
		in >> m_SyncLastBytes;
		in >> m_SyncLastPing;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuStatusReport
{
public:
	// Atoms Managed Status
	float					m_AtomLoad;		// Load in the range of 0 to 1+.
	quint8					m_AtomManagers;	// Number of threads allocated to managing atoms.
	QList<AuStatusAtom>		m_Atoms;		// Atoms managed.
	int						m_EtheriumLock;	// Result of AuEther::TestLock.
	bool					m_BdLock;		// Result of AuGlobal::TestBdLock
	bool					m_DataLock;		// Result of AuGlobal::TestDataLock
	bool					m_ItLock;		// Result of AuGlobal::TestItLock
	bool					m_ScLock;		// Result of AuGlobal::TestScLock
	bool					m_TrashLock;	// Result of AuGlobal::TestTrashLock

	// Connected Clients Status
	QList<AuStatusClient>	m_Clients;		// Clients connected.

public:
	inline AuStatusReport()
	{
		m_BdLock = false;
		m_DataLock = false;
		m_EtheriumLock = 0;
		m_ItLock = false;
		m_ScLock = false;
		m_TrashLock = false;
	}

	inline float AtomLoad() const
	{
		return m_AtomLoad;
	}

	// The return value is in 1/2 ms increments, so a return value of 100 is 50ms. 3 is 1.5ms.
	unsigned int AverageCycleMs() const;

	inline bool BdLock() const
	{
		return m_BdLock;
	}

	inline bool DataLock() const
	{
		return m_DataLock;
	}

	inline int EtheriumLock() const
	{
		return m_EtheriumLock;
	}

	inline bool ItLock() const
	{
		return m_ItLock;
	}

	inline bool ScLock() const
	{
		return m_ScLock;
	}

	inline bool TrashLock() const
	{
		return m_TrashLock;
	}

	// Returns the number of solar systems that appear deadlocked.
	unsigned int SystemsDeadlocked() const;
};

class AuStatusReportList : public QList<AuStatusReport>
{
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class DtMgrClient : public QThread
{
	Q_OBJECT;
private:
	int				m_ConnectDelay;			// Delay before connecting, in ms.
	QString			m_ConnectionName;		// Host name of manager server.
	quint16			m_ConnectionPort;		// The port I will use to connect to the server.
	int				m_MaxHistory;
	bool			m_PollAwaitingReply;	// True when we are waiting for the reply to a poll, so we don't pile them on.
	int				m_PollInterval;			// ms between polls. 0 = no polling.
	int				m_PollMode;				// 0=continuous, 1=poll once and signal, 2=poll once to find unit id,
											// 3=poll once to find unit name
	QTime			m_PollTime;				// Time of last poll.
	quint32			m_PollUnitId;			// Unit id to find for poll mode 2.
	QString			m_PollUnitName;			// Unit name to find for poll mode 3.
	quint8			m_ServerMode;			// AuGlobal::MODE of the connected server.

private:
	QMutex			m_PlayersOnLock;		// Guards m_PlayersOn
	AuPlayerOnlineStatusVector m_PlayersOn;	// Result of status query 3.

private:
	QMutex			m_StatusReportsLock;	// Guards m_StatusReport
	AuStatusReportList	m_StatusReports;

private:
	// Comm
	int				m_BytesRx;				// Total bytes received from client
	int				m_BytesTx;				// Total bytes sent to client
	bool			m_isConnected;			// true while connected to a scene server
	quint32			m_PacketSize;			// TCP block size
	int				m_PacketSizeBytesLeft;	// Number of packet size bytes we are expecting.

	QMutex			m_TcpBufferLock;
	QByteArray		m_TcpBuffer;			// Outgoing tcp stream buffer.

public:
	DtMgrClient(QObject* parent=0);
	~DtMgrClient();

	inline bool IsConnected() const
	{
		return m_isConnected;
	}

	inline const QString& ConnectionName() const
	{
		return m_ConnectionName;
	}

	inline quint16 ConnectionPort() const
	{
		return m_ConnectionPort;
	}

	inline quint8 ServerMode() const
	{
		return m_ServerMode;
	}

	inline void SetConnectDelay(int msConnectDelay)
	{
		m_ConnectDelay = msConnectDelay;
	}

	inline void SetConnectionInfo(const QString& host, quint16 port, int serverMode)
	{
		m_ConnectionName = host;
		m_ConnectionPort = port;
		m_ServerMode = serverMode;
	}

	inline const AuPlayerOnlineStatusVector& LockPlayersOn()
	{
		m_PlayersOnLock.lock();
		return m_PlayersOn;
	}

	inline void UnlockPlayersOn()
	{
		m_PlayersOnLock.unlock();
	}

	inline const AuStatusReportList& LockStatusReports()
	{
		m_StatusReportsLock.lock();
		return m_StatusReports;
	}

	inline void UnlockStatusReports()
	{
		m_StatusReportsLock.unlock();
	}

public slots:
	void Broadcast(QString mgrServerName, quint16 mgrServerPort, QString message);

	// Calls CreateSectorCubeAt on the galaxy.
	// 'galaxy' is a GALAXY enum value.
	void CreateSectorAt(QString serverName, quint16 serverPort, quint8 galaxy, double sectorX, double sectorY, double sectorZ);

	void DebugMessages(QString serverName, quint16 serverPort, quint32 debugMsgBits);

	void DebugSwitch(QString serverName, quint16 serverPort, quint8 specificDebug, quint8 value, quint32 atomId);

	void Disconnect(QString serverName, quint16 serverPort);

	// Finalize system contents when sent to a manager.
	void FinalizeSystem(QString serverName, quint16 serverPort, quint32 atomId);

	// Sends a single poll to the server and then signals UnitSearchResult when a response is received.
	void FindUnitId(QString serverName, quint16 serverPort, quint32 unitId);

	// Sends a single poll to the server and then signals UnitSearchResult when a response is received.
	void FindUnitName(QString serverName, quint16 serverPort, QString unitName);

	void KickUnit(QString serverName, quint16 serverPort, quint32 unitId);
	void LimboRescue(QString serverName, quint16 serverPort, quint32 atomId);
	void LimboRescueList(QString serverName, quint16 serverPort, QList<quint32> atomIds);

	// Sends a single poll to the server and then signals PollAnswered when a response is received.
	void Probe(QString serverName, quint16 serverPort);

	// Reassign system to specific worker server. Sent to manager.
	void ReassignAtom(QString mgrServerName, quint16 mgrServerPort, quint32 atomId, QString wrkServerName, quint16 wrkServerPort);

	// 'scriptId' 0 reloads all scripts
	void ReloadScript(QString mgrServerName, quint16 mgrServerPort, quint32 scriptId);

	void RequestMemoryUsage(QString serverName, quint16 serverPort);
	void RequestReloadData(QString serverName, quint16 serverPort);
	void RequestResourceReport(QString serverName, quint16 serverPort, quint8 specificReport, quint32 atomId);

	void RestoreEmpire(QString serverName, quint16 serverPort, DgEmpire empire);

	// Revoke sector/system assignment when sent to master/manager respectively.
	void RevokeAtom(QString serverName, quint16 serverPort, quint32 atomId);

	// Calls SetModified on an atom. BEWARE! Calling SetModified on an inappropriate atom will cause the server to assert/crash.
	void SetAtomModified(QString serverName, quint16 serverPort, quint32 atomId, bool recursive);

	void SetControlDecay(QString serverName, quint16 serverPort, quint16 decayMs);
	void SetMaxHistory(int maxHistory);
	void SetUdpDecay(QString serverName, quint16 serverPort, quint16 decayMs);
	void Shutdown(QString serverName, quint16 serverPort, QString warnMsg, QString haltMsg, qint16 shutdownSecs);
	void ShutdownHalt(QString serverName, quint16 serverPort, QString haltMsg);

	// Begins continuous polling of the server.
	void StartPolling(int intervalMs, QString serverName, quint16 serverPort);

	// Stops any polling of the server.
	void StopPolling(QString serverName, quint16 serverPort);

signals:
	void Connected();
	void MemoryUsageReport(QString serverName, quint16 serverPort, QStringList report);
	void MonitorAnswered(QString serverName, quint16 serverPort, unsigned int avgCycleMs, float atomLoad, int etheriumLock,
			unsigned int systemsDeadlocked, bool bdLock, bool dataLock, bool itLock, bool scLock, bool trashLock);
	void ProbeAnswered(QString serverName, quint16 serverPort, unsigned int avgCycleMs, float atomLoad, int etheriumLock,
			unsigned int systemsDeadlocked, bool bdLock, bool dataLock, bool itLock, bool scLock, bool trashLock);
	void ResourceReport(AuResourceReport report);
	void ResourceReportAtom(AuResourceReportAtom report);

	// 'unitId' and 'unitName' are the atom id and name of the unit that was found.
	// unitId is 0 if the unit is not here.
	void UnitSearchResult(QString serverName, quint16 serverPort, quint32 unitId, QString unitName);

protected:
	void Poll();

	void QueuePacket(const QByteArray& ba);

	bool ReadPacket(const QByteArray& ba, QTcpSocket& tcpSocket);

	void ReadResourceReport(QDataStream& in);

	void ReadResourceReportAtom(QDataStream& in);

	void ReadStatusOfLoads(QDataStream& in);

	void ReadStatusOfUsers(QDataStream& in);

	virtual void run();

protected slots:
	void NetworkError(QAbstractSocket::SocketError socketError);

	void RxData(QTcpSocket& tcpSocket);

	void TxData(QTcpSocket& tcpSocket);
};

#endif // DtMgrClient_H
