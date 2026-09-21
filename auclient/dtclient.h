//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtclient.h,v 1.18 2022/04/15 17:04:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DtClient_H
#define DtClient_H

#include "auclientlib.h"
#include "aucommpacket.h"
#include "auelapsedtimer.h"
#include "auid.h"
#include "auspecies.h"
#include <QImage>
#include <QSemaphore>
#include <QTcpSocket>
#include <QThread>

class AuMission;

class AUCLIENT_DLL DtClient : public QThread
{
	Q_OBJECT;

public:
	DtClient(QObject* parent=0);
	~DtClient();

	bool Alive() const
	{
		return !m_semAlive.available();
	}

	bool IsConnected() const
	{
		return m_isConnected;
	}

	void Kill()
	{
		if(Alive())
			m_semAlive.release(1);
	}

public slots:
	void TxData(QTcpSocket& tcpSocket);

signals:
	void ClientUnitEmpireReceived(quint32 empireId);

	void Connected();

	void EmpireDataBegin();

	void EmpireDataComplete();

	void EmpireRequested(quint32 empireId);

	void EstDeferred(AuId idSectorOrWorld);
	void InitSucceeded(AuId idUnit);
	void LoggedOut();
	void LoginFailed(quint8 errCode);	// 0=not logged in, 1=bad version

	void BdDesignNeeded(quint32 designId);
	void ItDesignNeeded(quint32 designId);
	void ScDesignNeeded(quint32 designId);

	void RestartNewAvatar();

	void SceneDisconnectedToDefer();
	void SceneDeferred(int);
	void SceneLoginSucceeded();

	void StarMapErased();

	void StarMapAddSector(AuId sectorId);

	void StartNewAvatarError(unsigned char reason);

	void SwitchAvatarTransferred(AuId transferredId);

protected:
	bool ReadPacket(const QByteArray& ba, QTcpSocket& tcpSocket);
	void ReconcileAtomsInScene();
	virtual void run();

private slots:
	void NetworkError(QAbstractSocket::SocketError socketError);
	void RxData(QTcpSocket& tcpSocket);

private:
	int			m_BytesRx;				// Total bytes received from client
	int			m_BytesTx;				// Total bytes sent to client
	AuId		m_idDeferAtom;			// Atom id in last defer packet received.
										// It will be a sector when deferred to a manager, a homeworld when deferred to a worker.
	bool		m_isConnected;			// true while connected to a scene server
	AuElapsedTimer	m_LastComm;				// Last reception or transmission of any bytes. Used for keep-alives.
	quint32		m_PacketSize;			// TCP block size
	int			m_PacketSizeBytesLeft;	// Number of packet size bytes we are expecting.
	QSemaphore	m_semAlive;				// Used to know when I'm supposed to die.
	unordered_map<AuId, bool, AuIdHash>	m_SyncLastAtomIds;	// Ids of non-permanent atoms that belong in the scene.
	bool		m_EmpireDataIncoming;	// true while we are receiving a batch of empire data.
	quint32		m_ClientUnitEmpire;		// client unit empire id
	bool		m_HasClientUnitEmpire;	// true if we have received the client unit's empire record

	QSet<quint32>						m_EmpiresRequested;		// Empires for which we have requested an empire record.
	QMap<AuSpecies, AuElapsedTimer>		m_SpeciesRequested;		// When we last requested each specie record.
};

#endif // DtClient_H
