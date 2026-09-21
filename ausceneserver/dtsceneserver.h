//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtsceneserver.h,v 1.272 2022/05/17 19:03:42 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DtSceneServer_H
#define DtSceneServer_H

#include "auatompos.h"
#include "auid.h"
#include "aujoboffer.h"
#include "aumatrix4x4.h"
#include "auoffice.h"
#include "autime.h"
#include "dpbody.h"
#include <QHostAddress>
#include <QMutex>
#include <QReadWriteLock>
#include <QSemaphore>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QThread>

class AuDNABase;
class AuGearPos;
class AuMissionOrder;
class AuPlayerOnlineStatus;
class AuStarMapData;
class AuWormhole;
class DgEmpire;
class DiControlInputArray;
class DpBody;
class QImage;

#define c_SyncHistory					100						// History of syncs maintained.

class AuAtomSync
{
public:
	AuTime								m_Time;
	AuElapsedTimer						m_Timer;

public:
	inline AuAtomSync()
	{
	}

	inline AuAtomSync(const AuTime& time, const AuElapsedTimer& timer)
	{
		m_Time = time;
		m_Timer = timer;
	}

	inline AuAtomSync(const AuAtomSync& c) : m_Time(c.m_Time), m_Timer(c.m_Timer)
	{
	}

public:
	inline const AuAtomSync& operator=(const AuAtomSync& c)
	{
		m_Time = c.m_Time;
		m_Timer = c.m_Timer;
		return *this;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class DtSceneServer : public QThread
{
	Q_OBJECT;
public:
	enum QUERY_REASON
	{
		QUERY_None,
		QUERY_Defer,
	};

public:
	DtSceneServer(unsigned int id, int socketDescriptor, QObject* parent=0);

	inline ACCOUNT AccountType() const
	{
		return m_AccountType;
	}

	void AtomPresence(const QString& netName, quint16 port, const QString& peerName);

	inline int BadPacketCount() const
	{
		return m_BadPackets;
	}

	inline int BytesRx() const
	{
		return m_BytesRx;
	}

	inline int BytesTx() const
	{
		return m_BytesTx;
	}

	inline const AuId& ClientSolarSystem() const
	{
		return m_idSystem;
	}

	inline const AuTime& ConnectTime() const
	{
		return m_tmConnect;
	}

	inline unsigned int EstEmpireId() const
	{
		return m_IdEmpire;
	}

	inline bool HasEmpireStance(quint32 idEmpire) const
	{
		m_rwAtomEmpires.lockForRead();
		bool hasEmpire = m_AtomStances.find(idEmpire) != m_AtomStances.cend();
		m_rwAtomEmpires.unlock();
		return hasEmpire;
	}

	inline unsigned int Id() const
	{
		return m_Id;
	}

	inline bool IsAwaitingAtomQuery(const AuId& atom, unsigned int empire) const
	{
		if(m_QueryReason == QUERY_None)
			return false;
		if(empire != m_EmpireQuery)
			return false;
		if(atom != m_AtomQuery)
			return false;
		return true;
	}

	inline bool IsEmpireInScene(quint32 idEmpire) const
	{
		m_rwAtomEmpires.lockForRead();
		bool hasEmpire = m_AtomEmpires.find(idEmpire) != m_AtomEmpires.cend();
		m_rwAtomEmpires.unlock();
		return hasEmpire;
	}

	inline unsigned int PlayerId() const
	{
		return m_idPlayer;
	}

	void PlayerOnlineStatus(AuPlayerOnlineStatus& status) const;

	void ReadUdpPacket(const QByteArray& ba);

	void SendAllEmpires();

	void SendEmpireData(quint32 idEmpire);

	void SendGameData();

	void SendJoinableEmpires();

	// Adds a packet to the transmit buffer.
	// 'flush' is true if the caller wants the socket flushed when this data is sent.
	// 'usemutex' is false when the caller has already locked m_muTxBuffer.
	void SendPacket(const QByteArray& ba, bool flush=true, bool usemutex=true);

	inline quint32 SyncCount() const		// Number of refreshes sent to client.
	{
		m_rwSync.lockForRead();
		quint32 syncCount = m_SyncCount;
		m_rwSync.unlock();
		return syncCount;
	}

	inline unsigned int SyncLastAtoms() const		// Number of atoms sent in last refresh.
	{
		m_rwSync.lockForRead();
		quint32 syncLastIndex = m_SyncCount%c_SyncHistory;
		quint32 lastAtoms = m_SyncLastAtoms[syncLastIndex];
		m_rwSync.unlock();
		return lastAtoms;
	}

	inline unsigned int SyncLastBytes() const		// Number of bytes sent in last refresh.
	{
		m_rwSync.lockForRead();
		quint32 syncLastIndex = m_SyncCount%c_SyncHistory;
		quint32 lastBytes = m_SyncLastBytes[syncLastIndex];
		m_rwSync.unlock();
		return lastBytes;
	}

	inline unsigned int SyncLastPing() const		// Ping time of last refresh
	{
		m_rwSyncLast.lockForRead();
		quint32 syncLastIndex = m_SyncLastCount%c_SyncHistory;
		quint32 lastPing =  m_SyncLastPing[syncLastIndex];
		m_rwSyncLast.unlock();
		return lastPing;
	}

	unsigned int SyncAverageAtoms() const;	// Rolling avg of atoms sent in refreshes.

	unsigned int SyncAverageBytes() const;	// Rolling avg of bytes sent in refreshes.

	unsigned int SyncAveragePing() const;	// Rolling avg of ping times.

	inline quint32 UnitEmpireId() const
	{
		return m_idUnitEmpire;
	}

	inline const AuId& UnitId() const
	{
		return m_idUnit;
	}

	inline const QString& UnitName() const
	{
		return m_UnitName;
	}

signals:
	void AddCitizenToEmpire(quint32 oldEmpireId, quint32 nuEmpireId, AuId personId, QString personName, quint8 personGender);

	void AtomTraveled(AuId travelId, AuId travelOrigSystem, AuId travelDestSystem, AuId travelDestSector);

	// Used to send initial login hail to my ship.
	void ChatReceived(QByteArray msg);

	void DeferClient(AuId unit);

	void Disconnecting(AuId);

	void EmpireAdded(quint32);

	void error(QTcpSocket::SocketError socketError);

	// emitted when QThread emits finished or terminated
	void Finished(unsigned int serverId);

	// 'reason' is an AuDocument::DocumentType().
	void GovSetPoliticalStance(quint32 idToEmpire, quint32 idFromEmpire, quint8 stance, quint8 vassalTributePercent,
			AuId fromUnit, QString fromUnitName, quint8 toStance, AuId toUnit, QString toUnitName,
			quint8 reason, quint8 galaxy, QString sector, QString system, QString world);

	void OfficeAssigned(AuId person, quint32 empire, AuOffice office);

	void PersonSystemChanged(AuId systemId, quint32 playerId);

	void PlayerConnecting(quint32 playerId, unsigned long threadId);	// Player login account id, not an atom id.

	void SceneAtomChanged(AuId oldPersonId, AuId nuPersonId);

public slots:
	void AtomPresenceResolved(AuId atom, quint32 empire, QString netname, quint16 port, QString peerName);
	void DisconnectPlayer(quint32 playerId, unsigned long exceptThreadId);
	void DisconnectUnit(AuId);

	void DisconnectUnitTransferred(AuId);

	void EmpireRecordAvailable(quint32 empireId);
	void SolarSystemChanged(AuId systemId, AuId sectorId);
	void UdpPacketReceived(quint32 playerId, QByteArray packet);

protected slots:
	void ThreadFinished();

protected:
	virtual void run();

private:
	void CheckLoginStatus();

	void ClientCollision(const AuId& myId, const AuPoint& myChange, const AuPoint& myHitLoc, const AuId& hitId,
			const AuPoint& hitChange, const AuPoint& hitFromLoc, const AuPoint& hitResultVec, bool hitShield);

	void ClientDeadheaded(const AuId& travId, const AuPoint& departureLoc);

	void ClientEnteredWormhole(const AuId& travId, const AuWormhole& wh, float entryAngle);

	void ClientFell(const AuId& fellId, quint32 hitsDamage, int stopReason);

	void ClientPositionUpdate(bool atRest, float headTilt, AuAtomPos& pos, AuPose pose[DpBody::PS_Count], quint16 whichConsole,
			quint16 stationId, quint8 seatIndex, AuPoint& vec, bool parentAlso, bool parentAtRest, AuAtomPos& parentPos,
			AuPose parentPose[DpBody::PS_Count], quint16 parentWhichConsole, quint16 parentStationId, AuPoint& parentVec);

	quint8 CreateAvatar(const QString& charname, const AuDNABase* dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand,
			const AuId& idHomeWorld, SCENARIO scenario, const AuTime& asteroidImpact, quint32 joinEmpire,
			const QByteArray& joinPassCrypt);

	//void DeferSceneServer(QTcpSocket& tcpSocket);
	void DeferClient();
	void DeferEst();
	void DeferToMaster();
	void DeletePerson(const AuId& personId);
	void Disconnect();

	unsigned char EstablishSector(unsigned char proximity, GALAXY galaxy, SCENARIO scenario, const AuId& joinSectorId);

	unsigned char EstablishHomeWorld(const AuId& idSector);

	void EstDeferred(const AuId& idTo, const AuId& idInfo);

	void IdleClient();

	void ProcessControlInput(const AuId& toAtom, const DiControlInputArray& aActiveControls, const AuGearPos& pos, qint8 selValue);

	void ReadPacket(const QByteArray& ba, QTcpSocket& tcpSocket);

	void ReleaseClientControl();

	void RxData(QTcpSocket& tcpSocket);

	void SwitchToCharacter(const AuId& personId);

	void SyncClient();

private:
	unsigned int	m_Id;				// Id of this server instance.
	// User
	ACCOUNT			m_AccountType;
	AuTime			m_AccountExpiry;
	unsigned int	m_confirmNum;		// Confirmation number assigned by login server.
	unsigned int	m_idPlayer;			// Id of player in login database.
	AuId			m_idUnit;			// Id of player's person unit atom.
	quint32			m_idUnitEmpire;		// Id of player's person unit atom's empire.
	quint8			m_UnitAgentBits;

	QString			m_UnitName;			// Name of player's person unit atom.
	mutable QMutex	m_muUnitName;		// Guards m_UnitName.

	bool			m_AFK;				// true if away from keyboard.
	bool			m_ChatClient;		// true if client is using the chat client "Hazeron Commander".
	AuId			m_ContactShip;		// Non-null to contact ship; set only at initial login, not when deferred
	bool			m_isUnitNewbie;
	AuId			m_idSystem;			// Id of unit's solar system atom.
	quint8			m_OsType;
	quint16			m_OsVer;
	quint8			m_WordSize;

	unordered_map<quint32, bool> m_AtomEmpires;	// List of all empire ids the client has in their scene.
	unordered_map<quint32, bool> m_AtomStances;	// List of all empire ids the client has in their scene.
	mutable QReadWriteLock	m_rwAtomEmpires;	// Guards m_AtomEmpires.

	mutable QMutex	m_EmpiresSentLock;	// Guards m_EmpiresSent.
	QHash<quint32, QByteArray>	m_EmpiresSent;	// Empires sent to the client with their SHA-1 key, so we avoid duplicate resends.

	Qt::HANDLE		m_ThreadId;

	// Guarded by m_rwSync.
	QHash<AuId, AuAtomSync>				m_AtomSync;				// Time of last sync per atom. (Access controlled by m_muTxBuffer)
	unordered_map<AuId,bool, AuIdHash>	m_PermanentAtoms;		// Atoms in the scene that returned 'true' to IsPermanentInScene
	quint32								m_SyncCount;			// Number of refreshes sent to client.
	GALAXY								m_SyncGalaxy;			// Scene atom's parent galaxy at last sync.
	unsigned int						m_SyncLastAtoms[c_SyncHistory];	// Qty of atoms sent in last c_SyncHistory refreshes.
	unordered_map<AuId,bool, AuIdHash>	m_SyncLastAtomIds;		// Hash of non-permanent atoms ids sent with the previous sync.
	unsigned int						m_SyncLastBytes[c_SyncHistory];	// Qty of bytes sent in last c_SyncHistory refreshes.
	AuId								m_SyncSector;			// Scene atom's sector at last sync. (Locked by m_muTxBuffer)
	unsigned int						m_SyncTotalAtoms;		// Total atoms sent in all refreshes.
	unsigned int						m_SyncTotalBytes;		// Total bytes sent in all refreshes.
	mutable QReadWriteLock				m_rwSync;				// Guards all 'Sync' section variables.

	// Guarded by m_rwSyncLast
	quint32			m_SyncLastCount;		// Number of sync completes received from the client
	unsigned int	m_SyncLastPing[c_SyncHistory];	// Ping times of last c_SyncHistory refreshes.
	mutable QReadWriteLock	m_rwSyncLast;			// Guards all 'Sync' section variables.

	// Comm
	unsigned int	m_BadPackets;		// # of packets rx'd with unknown packet type
	unsigned int	m_BytesRx;			// Total bytes received from client
	unsigned int	m_BytesTx;			// Total bytes sent to client
	bool			m_LocalClient;		// true if the client appears to be on our LAN
	quint32			m_PacketSize;		// Data packet size
	int				m_PacketSizeBytesLeft;	// Number of packet size bytes we are expecting.
	int				m_socketDescriptor;	// Provided in the constructor.
	AuTime			m_tmConnect;		// Time connection is established.
	AuElapsedTimer	m_tmDisconnect;		// Null until time to disconnect.

	QByteArray		m_TxBuffer;			// On the client, the tx buffer is a property of AuApp.
	bool			m_TxBufferFlush;	// true to flush the data when it is next transmitted.
	QMutex			m_muTxBuffer;		// Guards m_TxBuffer and m_TxBufferFlush.

	quint16			m_UdpSequence;		// Highest Udp sequence received so far.

	// Est defer data.
	AuId			m_IdDeferEstTo;
	AuId			m_IdInfo;
	unsigned int	m_IdEmpire;

	// Atom query values.
	QUERY_REASON	m_QueryReason;		// Reason we are waiting for atom query.
	AuElapsedTimer	m_tmAtomQuery;		// We don't wait forever for atom query.
	AuId			m_AtomQuery;
	unsigned int	m_EmpireQuery;

	// Client position updates.
	AuAtomPos		m_Pos;
	bool			m_PosAtRest;
	float			m_PosHeadTilt;
	AuAtomPos		m_PosParent;
	bool			m_PosParentAlso;
	bool			m_PosParentAtRest;
	AuPose			m_PosParentPose[DpBody::PS_Count];
	quint16			m_PosParentStationId;
	AuPoint			m_PosParentVec;
	quint16			m_PosParentWhichConsole;
	AuPose			m_PosPose[DpBody::PS_Count];
	quint8			m_PosSeatIndex;
	quint16			m_PosStationId;
	AuPoint			m_PosVec;
	quint16			m_PosWhichConsole;
};

#endif // DtSceneServer_H
