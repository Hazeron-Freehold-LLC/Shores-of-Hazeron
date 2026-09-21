//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumailserverapp.h,v 1.26 2022/02/19 18:37:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AUMAILSERVERAPP_H
#define AUMAILSERVERAPP_H

#include "auid.h"
#include "aumailmsg.h"
#include "aumailserver.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QMap>
#include <QMutex>

class QSqlDatabase;
class QTimer;

class DhMailServer
{
public:
	QDateTime	m_CheckIn;	// Last checkin time.
	QString		m_NetName;	// Internet resolvable name.
	quint16		m_NetPort;	// Internet port on which the server is listening.
    QString     m_PeerAddr; // Address on LAN

public:
	DhMailServer()
	{
		m_NetPort = 0;
	}

	DhMailServer(const DhMailServer& c)
	{
		m_CheckIn = c.m_CheckIn;
		m_NetName = c.m_NetName;
		m_NetPort = c.m_NetPort;
		m_PeerAddr = c.m_PeerAddr;
	}

	const DhMailServer& operator=(const DhMailServer& c)
	{
		m_CheckIn = c.m_CheckIn;
		m_NetName = c.m_NetName;
		m_NetPort = c.m_NetPort;
		m_PeerAddr = c.m_PeerAddr;
		return *this;
	}
};

class AuMailLogin
{
private:
	unsigned int		m_confirmNumber;	// Confirmation number.
	AuId				m_idUnit[6];		// Result of last real check.
	bool				m_isLoggedIn;		// Result of last real check.
	QDateTime			m_tmLastCheck;		// Time of last real check.

public:
	AuMailLogin()
	{
		m_confirmNumber = 0;
		m_isLoggedIn = false;
	}
	AuMailLogin(unsigned int confirmNum)
	{
		m_confirmNumber = confirmNum;
		m_isLoggedIn = false;
	}
	bool IsLoggedIn(unsigned int idPlayer, unsigned int confirmNum, AuId* pIdUnit=0);
};

class AuMailServerApp : public QCoreApplication
{
	Q_OBJECT;
public:
	AuMailServerApp(int& argc, char** argv);
	~AuMailServerApp();

	static void DeleteOldMail();

	void HandleSignal(int value);	// Called to handle a signal caught in main.cpp.

	bool IsLoggedIn(unsigned int idPlayer, unsigned int confirmNum, AuId* pIdUnit=0);

	QMap<unsigned int,AuMailLogin>& LockLoginMap()
	{
		m_muLoginMap.lock();
		return m_LoginMap;
	}

	QMap<QString,DhMailServer>& LockServerMap()
	{
		m_muServerMap.lock();
		return m_ServerMap;
	}

	inline const QDateTime& TimeUp() const
	{
		return m_tmUp;
	}

	void UnlockLoginMap()
	{
		m_muLoginMap.unlock();
	}

	void UnlockServerMap()
	{
		m_muServerMap.unlock();
	}

	// Returns the number of worker servers.
	int WorkerCount();

protected:
	static QString CreateDeleteQuery(int secondsTooOld, const QList<AuMailMsg::MSGTYPE>& typesToDelete);
	unsigned int CreateMessageId(QSqlDatabase& db);
	static bool OpenDatabase(QSqlDatabase& db);
	void StartServer();

public slots:
	void DeleteMailMessages(quint32 idPlayer, AuId idAvatar, AuMailMsgIdList delids);
	void MailMessageReceived(quint32 idPlayer, AuMailMsg msg);
	void MailMessageReceivedBulk(quint32 idPlayer, AuMailMsg msg, AuIdList recipients);
	void RequestMailMessages(quint32 idPlayer, AuId idAvatar, quint32 maxMsgId, AuIdList squelchIds, quint8 cityRptAutoDel,
			quint32 cityRptOldAge, quint8 fleetRptAutoDel, quint32 fleetRptOldAge, quint8 officerRptAutoDel,
			quint32 officerRptOldAge);

signals:
	void PlayerMailMessages(quint32 idPlayer, AuMailMsgArray msgs);

protected slots:
	void UpdateStats();

private:
	QMap<unsigned int,AuMailLogin>	m_LoginMap;		// Used when running as MailWorker.
	QMutex				m_muLoginMap;
	QMutex				m_muServerMap;
	QMap<QString,DhMailServer>	m_ServerMap;	// Used when running as MailMaster.
	AuMailServer		m_TcpServer;
	QTimer*				m_timer;
	QDateTime			m_tmCheckIn;	// Last time this Server checked in with Master.
										// On master last time old city reports removed.
	QDateTime			m_tmUp;			// Time this object was constructed. For uptime.
};

#define auMailServerApp	((AuMailServerApp*)QCoreApplication::instance())

#endif // AUMAILSERVERAPP_H
