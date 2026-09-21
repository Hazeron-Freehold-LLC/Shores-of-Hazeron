//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auserverchannel.h,v 1.21 2022/02/19 18:39:27 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AUSERVERCHANNEL_H
#define AUSERVERCHANNEL_H

#include <QReadWriteLock>
#include <QTcpServer>
#include <QList>

using namespace std;
#include <unordered_map>

class AuPlayerOnlineStatusVector;
class DtServerChannel;

class AuServerChannel : public QTcpServer
{
	Q_OBJECT;

public:
	AuServerChannel(QObject* parent=0);

	int CountManagerServers() const;

	int CountWorkerServers() const;

	DtServerChannel* FindServer(const QString& networkName, quint16 networkPort);

	// mode 1=mgr, 2=worker
	// 'avoidServerId' is the server id of a sub server that should be avoided if possible. This happens when selecting a sub
	// server for a system or sector that has been revoked. The avoidServerId is the server it was revoked from.
	DtServerChannel* LeastLoadedServer(int mode, int which=0, unsigned int avoidServerId=0);

	// The lock guards the vector itself, not the contents.
	// The 'public' cannot change the contents of the vector.
	//const QList<DtServerChannel*>& LockServerList(const char* file, int line) const
	const unordered_map<unsigned int, DtServerChannel*>& LockServerList(const char* /*file*/, int /*line*/) const
	{
		//qDebug("AuServerChannel::LockServerList in %s at line %d", file, line);
		m_ServersLock.lockForRead();
		return m_Servers;
	}

	DtServerChannel* MostLoadedServer();

	int PlayerCount() const;

	void PlayersOnlineStatus(AuPlayerOnlineStatusVector& players) const;

	DtServerChannel* ServerChannel(unsigned int id) const;

	void StopThreads();

	//void UnlockServerList(const char* file, int line) const
	void UnlockServerList(const char* /*file*/, int /*line*/) const
	{
		//qDebug("AuServerChannel::UnlockServerList in %s at line %d", file, line);
		m_ServersLock.unlock();
	}

	int WorkerCoreCount() const;

	int WorkerCount() const;

signals:
	void ConnectionClosed(unsigned int serverId);

protected:
	virtual void incomingConnection(qintptr socketDescriptor);

protected slots:
	void ServerThreadFinished(unsigned int serverId);

private:
	unsigned int						m_NextId;
	unordered_map<unsigned int, DtServerChannel*>	m_Servers;			// key is server id
	mutable QReadWriteLock				m_ServersLock;			// Guards the hash itself, not the contents.
};

#endif // AUSERVERCHANNEL_H
