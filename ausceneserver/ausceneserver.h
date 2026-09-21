//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausceneserver.h,v 1.35 2022/02/19 18:39:26 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AUSCENESERVER_H
#define AUSCENESERVER_H

#include "auhash.h"
#include <QMap>
#include <QReadWriteLock>
#include <QTcpServer>
#include <QList>
#include <unordered_map>

class AuId;
class AuPlayerOnlineStatusVector;
class DtSceneServer;

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Used by workers to report status of each assigned system to manager.

class AuSystemStatus
{
public:
	quint16		m_Clients;	// Number of clients attached to the system.
	quint32		m_Tick;		// Average tick time of the system for the past 100 tick cycles. 1/2 millisecond incs. i.e. 100 = 50ms

public:
	AuSystemStatus()
	{
		m_Clients = 0;
		m_Tick = 0;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuSceneServer : public QTcpServer
{
	Q_OBJECT;

public:
	AuSceneServer(QObject* parent=0);

	// The lock guards the vector itself, not the contents.
	// The 'public' cannot change the contents of the vector.
	//const QList<DtSceneServer*>& LockServerList(const char* file, int line) const
	const unordered_map<unsigned int, DtSceneServer*>& LockServerList(const char* /*file*/, int /*line*/) const
	{
		//qDebug("AuSceneServer::LockServerList in %s at line %d", file, line);
		m_ServersLock.lockForRead();
		return m_Servers;
	}

	void PlayersOnlineStatus(AuPlayerOnlineStatusVector& players) const;

	void SystemClients(unordered_map<AuId, AuSystemStatus, AuIdHash>& systems) const;

	//void UnlockServerList(const char* file, int line) const
	void UnlockServerList(const char* /*file*/, int /*line*/) const
	{
		//qDebug("AuSceneServer::UnlockServerList in %s at line %d", file, line);
		m_ServersLock.unlock();
	}

protected:
	virtual void incomingConnection(qintptr socketDescriptor);

protected slots:
	void DeferClient(AuId unit);
	void ServerThreadFinished(unsigned int serverId);

private:
	unsigned int						m_NextId;
	unordered_map<unsigned int, DtSceneServer*>	m_Servers;				// key is server id
	mutable QReadWriteLock				m_ServersLock;			// Guards the hash itself, not the contents.
};

#endif // AUSCENESERVER_H
