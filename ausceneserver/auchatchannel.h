//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auchatchannel.h,v 1.15 2022/02/19 18:39:23 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuChatChannel_H
#define AuChatChannel_H

#include <auid.h>
#include <QTcpServer>
#include <QList>

class DtChatChannel;

class AuChatChannel : public QTcpServer
{
	Q_OBJECT;

public:
	AuChatChannel(QObject* parent=0);

	void ListDesignsInUse(QSet<quint32>& bdDesigns, QSet<quint32>& scDesigns);

	/*
	// The lock guards the vector itself, not the contents.
	// The 'public' cannot change the contents of the vector.
	//const QList<DtChatChannel*>& LockServerList(const char* file, int line) const
	const QList<DtChatChannel*>& LockServerList(const char* file, int line) const
	{
		//qDebug("AuChatChannel::LockServerList in %s at line %d", file, line);
		m_ServersLock.lockForRead();
		return m_Servers;
	}

	//void UnlockServerList(const char* file, int line) const
	void UnlockServerList(const char* file, int line) const
	{
		//qDebug("AuChatChannel::UnlockServerList in %s at line %d", file, line);
		m_ServersLock.unlock();
	}
	*/

protected:
	virtual void incomingConnection(qintptr socketDescriptor);

protected slots:
	void DeferClient(AuId unit);

	void ServerThreadFinished();

private:
	QList<DtChatChannel*>		m_Servers;
	mutable QReadWriteLock		m_ServersLock;	// Guards the vector itself, not the contents.
};

#endif // AuChatChannel_H
