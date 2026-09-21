//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auloginserverapp.cpp,v 1.54 2022/04/07 21:55:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auloginserverapp.h"
//
#include "audnabase.h"
#include "auglobal.h"
#include "dtlogindb.h"
#include "dtloginserver.h"
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QTextStream>
#include <QTimer>
#include <signal.h>
#if defined(AUSTEAM)
#include <steamencryptedappticket.h>
#endif	// AUSTEAM

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

QString AuLoginAttempt::AccountTypeName() const
{
	return AcctTypeName(m_AccountType);
}

AuLoginAttempt::~AuLoginAttempt()
{
	for(int u=0; u<6; u++)
		if(m_UnitDNA[u])
			delete m_UnitDNA[u];
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#if defined(AUSTEAM)
AuLoginSteamAttempt::AuLoginSteamAttempt(unsigned int serverId, const QString& clientName, const QByteArray& ticket,
		bool testCredentialsOnly)
{
	m_AccountType = ACCT_Active;
	m_ClientName = clientName;
	m_ConfirmNum = 0;
	m_PlayerId = 0;
	m_Result = Pending;
	m_ServerId = serverId;
	m_TestCredentialsOnly = testCredentialsOnly;
	for(int u=0; u<6; u++)
		m_UnitDNA[u] = 0;

	// Decipher the steam ticket.
	quint8 rgubTicketDecrypted[1024];
	quint32 cubTicketDecrypted = sizeof(rgubTicketDecrypted);
	quint8 rgubKey[k_nSteamEncryptedAppTicketSymmetricKeyLen] =
	{
		// My secret encrypted app key must never be given to clients.
		0xa4, 0xc4, 0x45, 0x10, 0x49, 0x0d, 0x80, 0x89,
		0x5b, 0xa2, 0xe8, 0x0f, 0x23, 0x55, 0xda, 0x0b,
		0xf1, 0x40, 0xcf, 0x8c, 0x69, 0xda, 0x5b, 0x4c,
		0x5d, 0x3e, 0xb5, 0x09, 0x6f, 0x0e, 0x1e, 0x33
	};
	int cubKey = sizeof(rgubKey);
	if(SteamEncryptedAppTicket_BDecryptTicket((const quint8*)ticket.data(), ticket.size(), rgubTicketDecrypted, &cubTicketDecrypted,
				rgubKey, cubKey))
	{
		// Key decrypted ok.
		AppId_t SoHAppId = 1040380;
		if(SteamEncryptedAppTicket_BIsTicketForApp(rgubTicketDecrypted, cubTicketDecrypted, SoHAppId))
		{
			RTime32 rt(SteamEncryptedAppTicket_GetTicketIssueTime(rgubTicketDecrypted, cubTicketDecrypted));	// Secs since epoch
			AuTime t((quint64)rt * 1000);
			if(t.ElapsedSec() < 120)
				SteamEncryptedAppTicket_GetTicketSteamID(rgubTicketDecrypted, cubTicketDecrypted, &m_SteamId);
			else
				m_Result = BadKeyOld;
		}
		else
			m_Result = BadKeyApp;
	}
	else
		m_Result = BadKey;
}

AuLoginSteamAttempt::~AuLoginSteamAttempt()
{
	for(int u=0; u<6; u++)
		if(m_UnitDNA[u])
			delete m_UnitDNA[u];
}

QString AuLoginSteamAttempt::AccountTypeName() const
{
	return AcctTypeName(m_AccountType);
}

QString AuLoginSteamAttempt::Name() const
{
	QString name;
	if(m_SteamId.IsValid())
		name = QString("%1:%2")
			.arg(m_SteamId.GetAccountID())
			.arg(m_SteamId.GetEUniverse());
	return name;
}
#endif	// AUSTEAM

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuLoginServerApp::AuLoginServerApp(int& argc, char** argv) : QCoreApplication(argc, argv)
{
	// Get default values that may be overridden by command line options.
	bool isMaster = false;
	QString netName;

	// Parse command line.
	QStringList args = arguments();
	for(QStringList::const_iterator it = args.constBegin(); it != args.constEnd(); it++)
	{
		if((*it).left(2) == "-n")
		{
			// -n parm enables worker server supplies it's network resolvable name.
			// e.g. -nu2.Hazeron.net
			netName = (*it).mid(2);
		}
		else if((*it) == "-master")
		{
			// -master parm identifies the master login server
			isMaster = true;
		}
	}

	Q_ASSERT(gp_AuGlobal == 0);
	if(isMaster)
	{
		// Create 'LoginMaster' global data object independent of app object.
		gp_AuGlobal = new AuGlobal("Hazeron Login Master", "Hazeron Freehold", AuGlobal::LoginMaster);
		g_Au.Log("Login Master Started");
	}
	else
	{
		// Create 'LoginServer' global data object independent of app object.
		gp_AuGlobal = new AuGlobal("Hazeron Login Worker", "Hazeron Freehold", AuGlobal::LoginWorker);
		g_Au.Log("Login Worker Started");
	}

	// Configure internet name.
	if(!netName.isEmpty())
		g_Au.SetInternetName(netName);

	m_tLoginDb = 0;

	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), SLOT(UpdateStats()));
	m_timer->start(10000);

	m_tmUp = g_Au.CurrentUTC();

	// Seed random number generator for login confirmation numbers.
	g_Au.Dice().TimeSeed();

	StartServer();
	UpdateStats();
}

AuLoginServerApp::~AuLoginServerApp()
{
	if(gp_AuGlobal)
	{
		delete gp_AuGlobal;
		gp_AuGlobal = 0;
	}

	if(m_tLoginDb)
	{
		m_tLoginDb->Kill();
		m_tLoginDb->wait();	// Wait indefinitely for db thread to finish.
		m_tLoginDb->deleteLater();
		m_tLoginDb = 0;
	}
}

void AuLoginServerApp::StartServer()
{
	if(g_Au.IsLoginWorker())
	{
		// Start database thread. He's the only guy who gets to communicate
		// with the db. The login master does not communicate with the database.
		m_tLoginDb = new DtLoginDb(this);
		m_tLoginDb->start();
	}

	// Start listening for client login connections.
	quint16 port = AuUtil::LoginMasterPort();
	QString logmsg;
	if(m_TcpServer.listen(QHostAddress::Any, port))
		logmsg = QObject::tr("%1 Server listening on TCP port %2")
				.arg(g_Au.IsLoginMaster() ? "Master" : "Worker").arg(port);
	else
		logmsg = QObject::tr("ERROR! %1 Server failed to listen on TCP port %2")
				.arg(g_Au.IsLoginMaster() ? "Master" : "Worker").arg(port);
	g_Au.Log(logmsg);
}

void AuLoginServerApp::UpdateStats()
{
	QDateTime tmNow = g_Au.CurrentUTC();
	const int secsToCheckIn = 60*3; // Worker check-in interval.
	if(g_Au.IsLoginWorker())
	{
		// Check in with master server.
		if(m_tmCheckIn.isNull() || m_tmCheckIn.secsTo(tmNow) > secsToCheckIn)
		{
			DtLoginServer::CheckIn();
			m_tmCheckIn = tmNow;
		}
	}
	else
	{
		// On the master, decay the server check-in list.
		const int secsToDrop = secsToCheckIn + 30;  // CheckIn can be up to 30 seconds late
		QMap<QString, DhLoginServer>& serverMap = LockServerMap();
		QMap<QString, DhLoginServer>::iterator it = serverMap.begin();
		while(it != serverMap.end())
		{
			if((*it).m_CheckIn.isNull() || (*it).m_CheckIn.secsTo(tmNow) > secsToDrop)
				it = serverMap.erase(it);	// Too long since last checkin.
			else
				it++;
		}
		UnlockServerMap();
	}

	// Send the stats report to a file.
	QFile file("aulogin.txt");
	if(file.open(QIODevice::WriteOnly))
	{
		QTextStream out(&file);

		// Time Hack
		QDateTime tmNow = g_Au.CurrentUTC();
		out << tmNow.toString();
		out << '\n';

		// Up Time
		int uptime = m_tmUp.secsTo(tmNow);	// total seconds
		int umin = uptime/60;			// whole minutes
		int usec = uptime - umin*60;	// fractional seconds
		int uhr = umin/60;				// whole hours
		umin -= uhr*60;					// fractional minutes
		int uday = uhr/24;				// whole days
		uhr -= uday*24;					// fractional hours
		QString stat;
		if(uday > 0)
			//stat = QObject::tr("Up %1 days %2:%3:%4\n").arg(uday).arg(uhr).arg(umin, 2, 10, '0').arg(usec, 2, 10, '0');
			stat = QString("Up %1 days %2:%3:%4\n").arg(uday).arg(uhr, 2, 10, QChar('0')) 
				.arg(umin, 2, 10, QChar('0')).arg(usec, 2, 10, QChar('0'));
			//stat.sprintf("Up %d days %d:%02d:%02d\n", uday, uhr, umin, usec);
		else if(uhr > 0)
			stat = QString("Up %1:%2:%3\n").arg(uhr, 2, 10, QChar('0')).arg(umin, 2, 10, QChar('0'))
				.arg(usec, 2, 10, QChar('0'));
			//stat.sprintf("Up %d:%02d:%02d\n", uhr, umin, usec);
		else
			stat = QString("Up %1:%2\n").arg(umin).arg(usec, 2, 10, QChar('0'));
			//stat.sprintf("Up %d:%02d\n", umin, usec);
		out << stat;

		// Server defer stats.
		if(!g_Au.IsLoginWorker())
		{
			QMap<QString, DhLoginServer>& serverMap = LockServerMap();
			if(!serverMap.isEmpty())
			{
				out << QObject::tr("Server Map\n");
				for(QMap<QString, DhLoginServer>::const_iterator it = serverMap.cbegin(); it != serverMap.cend(); ++it)
				{
					out << ' ';
					out << (*it).m_PeerAddr;
					out << "  ";
					out << (*it).m_NetName;
					out << ", ";
					out << QVariant((*it).m_NetPort).toString();
					out << '\n';
				}
			}
			else
				out << QObject::tr("Server Map - EMPTY!\n");
			UnlockServerMap();
		}

		file.close();
	}
}

void AuLoginServerApp::HandleSignal(int value)
{
	switch(value)
	{
#if !defined(Q_OS_WIN)
	case SIGHUP:
		g_Au.Log("SIGHUP ignored.");
		return;	// Ignore hangup signal.

	case SIGQUIT:
		g_Au.Log("SIGQUIT caught. Shutting down...");
		break;
#endif

	case SIGINT:
		g_Au.Log("SIGINT caught. Shutting down...");
		break;

	case SIGTERM:
		g_Au.Log("SIGTERM caught. Shutting down...");
		break;
	}
	quit();
}

// Returns the number of worker servers.
int AuLoginServerApp::WorkerCount()
{
	QMap<QString, DhLoginServer>& serverMap = LockServerMap();
	int workers = serverMap.size();
	UnlockServerMap();
	return workers;
}

bool AuLoginServerApp::HasQueuedOperations() const
{
	m_LoginMutex.lock();
	bool hasOp =
		!m_LoginAttempts.empty() ||
		!m_LoginQueries.empty() ||
#if defined(AUSTEAM)
		!m_LoginSteamAttempts.empty() ||
#endif	// AUSTEAM
		!m_LogoutAttempts.empty() ||
		!m_NewPlayers.empty() ||
		!m_QueryAccountAvatars.empty() ||
		!m_QueryAvatars.empty() ||
		!m_ResetPassword.empty() ||
		!m_SetEmail.empty() ||
		!m_SetPassword.empty() ||
		!m_SetUnit.empty() ||
		!m_UnitBorn.empty() ||
		!m_UnitDied.empty() ||
		!m_UnitQueries.empty() ||
		!m_UnitTransfer.empty();
	m_LoginMutex.unlock();
	if(hasOp)
		return true;
	
	m_LoginContactsMutex.lock();	
	hasOp = !m_ContactsQueries.empty() ||
		!m_ContactsUpdates.empty() ||
		!m_ContactsAdds.empty() ||
		!m_ContactsDeletes.empty() ||
		!m_ContactAdds.empty() ||
		!m_ContactDeletes.empty();
	m_LoginContactsMutex.unlock();	
	return hasOp;
}
