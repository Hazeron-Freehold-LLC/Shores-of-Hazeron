//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumailserverapp.cpp,v 1.48 2022/04/07 21:54:33 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aumailserverapp.h"
//
#include "auglobal.h"
#include "aumath.h"
#include "dtmailserver.h"
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTextStream>
#include <QTimer>
#include <signal.h>

// Sometimes we get bursts of mail from the scene servers. Instead of querying
// the login server with each new mail message, this cache remembers each player
// id login and its success result within the immediate past.
// 'pIdUnit' must be null or it must point to an array large enough for six AuIds.
bool AuMailLogin::IsLoggedIn(unsigned int idPlayer, unsigned int confirmNum, AuId* pIdUnit)
{
	const int checkInterval = 60 * 5;	// seconds
	QDateTime now = g_Au.CurrentUTC();
	if(m_tmLastCheck.isNull() || m_tmLastCheck.secsTo(now) > checkInterval || confirmNum != m_confirmNumber)
	{
		// Actually query the login server.
		ACCOUNT acctType;
		AuTime acctExpiry;
		m_isLoggedIn = g_Au.IsPlayerLoggedIn(idPlayer, confirmNum, m_idUnit, acctType, acctExpiry);
		m_tmLastCheck = now;
	}
	if(pIdUnit)
	{
		// Provide the list of unit ids returned by the login server when we last checked.
		for(int iu=0; iu<6; iu++)
			pIdUnit[iu] = m_idUnit[iu];
	}
	return m_isLoggedIn;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

AuMailServerApp::AuMailServerApp(int& argc, char** argv) :
	QCoreApplication(argc, argv)
{
	// Get default values that may be overridden by command line options.
	bool isMaster = false;
	QString netName;

	// Parse command line.
	QStringList args = arguments();
	for(QStringList::const_iterator it = args.constBegin();
			it != args.constEnd(); it++)
	{
		if((*it).left(2) == "-n")
		{
			netName = (*it).mid(2);
		}
		else if((*it) == "-master")
		{
			isMaster = true;
		}
	}

	Q_ASSERT(gp_AuGlobal == 0);
	if(isMaster)
	{
		// Create 'MailMaster' global data object independent of app object.
		gp_AuGlobal = new AuGlobal("Hazeron Mail Master", "Hazeron Freehold", AuGlobal::MailMaster);
		g_Au.Log("Mail Master Started");
	}
	else
	{
		// Create 'MailServer' global data object independent of app object.
		gp_AuGlobal = new AuGlobal("Hazeron Mail Worker", "Hazeron Freehold", AuGlobal::MailWorker);
		g_Au.Log("Mail Worker Started");
	}

	// Configure internet name.
	if(!netName.isEmpty())
		g_Au.SetInternetName(netName);

	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), SLOT(UpdateStats()));

	m_tmUp = g_Au.CurrentUTC();

	StartServer();
	UpdateStats();
}

AuMailServerApp::~AuMailServerApp()
{
	// This must be done after the QSqlDatabase object has gone out of scope.
	// Refer to QSqlDatabase::removeDatabase documentation.
	QSqlDatabase::removeDatabase("AuMail");

	if(gp_AuGlobal)
	{
		delete gp_AuGlobal;
		gp_AuGlobal = 0;
	}
}

void AuMailServerApp::StartServer()
{
	// addDatabase must only be called once for each database connection we will need.
	QSqlDatabase dbmail = QSqlDatabase::addDatabase("QPSQL7", "AuMail");
	if(!dbmail.isValid())
	{
		qWarning("ERROR! Failed to add AuMail database.");
		return;
	}

	// Start listening for client mail connections.
	quint16 port = AuUtil::MailMasterPort();
	QString logmsg;
	if(m_TcpServer.listen(QHostAddress::Any, port))
		logmsg = QObject::tr("%1 Server listening on TCP port %2")
				.arg(g_Au.IsMailMaster() ? "Master" : "Worker").arg(port);
	else
		logmsg = QObject::tr("ERROR! %1 Server failed to listen on TCP port %2")
				.arg(g_Au.IsMailMaster() ? "Master" : "Worker").arg(port);
	g_Au.Log(logmsg);
}

void AuMailServerApp::UpdateStats()
{
	if(m_timer)
		m_timer->stop();

	QDateTime tmNow = g_Au.CurrentUTC();
	const int secsToCheckIn = 60*3;	// Worker check-in interval.
	if(g_Au.IsMailWorker())
	{
		// Check in with master server.
		if(m_tmCheckIn.isNull() || m_tmCheckIn.secsTo(tmNow) > secsToCheckIn)
		{
			DtMailServer::CheckIn();
			m_tmCheckIn = tmNow;
		}
	}
	else
	{
		// The master server periodically discards old city reports.
		// Messages are too old after four hours. Discard old messages hourly.
		if(tmNow.time().hour() != m_tmCheckIn.time().hour() || m_tmCheckIn.isNull())
		{
			DeleteOldMail();
			m_tmCheckIn = tmNow;
		}

		// On the master, decay the server check-in list.
		const int secsToDrop = secsToCheckIn + 30;	// CheckIn can be up to 30 secs late
		QMap<QString,DhMailServer>& serverMap = LockServerMap();
		QMap<QString,DhMailServer>::iterator it = serverMap.begin();
		while(it != serverMap.end())
		{
			if((*it).m_CheckIn.isNull() || (*it).m_CheckIn.secsTo(tmNow) > secsToDrop)
				it = serverMap.erase(it);	// Too long since we've heard from it.
			else
				it++;
		}
		UnlockServerMap();
	}

	// Send the stats report to a file.
	QFile file("aumail.txt");
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
			stat = QString("Up %1 days %2:%3:%4\n").arg(uday).arg(uhr).arg(umin, 2, 10, QChar('0'))
				.arg(usec, 2, 10, QChar('0'));
			//stat.sprintf("Up %d days %d:%02d:%02d\n", uday, uhr, umin, usec);
		else if(uhr > 0)
			stat = QString("Up %1:%2:%3\n").arg(uhr).arg(umin, 2, 10, QChar('0'))
				.arg(usec, 2, 10, QChar('0'));
			//stat.sprintf("Up %d:%02d:%02d\n", uhr, umin, usec);
		else
			stat = QString("Up %1:%2\n").arg(umin).arg(usec, 2, 10, QChar('0'));
			//stat.sprintf("Up %d:%02d\n", umin, usec);
		out << stat;

		// Server defer stats.
		if(!g_Au.IsMailWorker())
		{
			QMap<QString,DhMailServer>& serverMap = LockServerMap();
			if(!serverMap.empty())
			{
				out << tr("Server Map\n");
				for(QMap<QString,DhMailServer>::const_iterator it = serverMap.cbegin(); it != serverMap.cend(); it++)
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
				out << tr("Server Map - EMPTY!\n");
			UnlockServerMap();
		}

		// TCP Server Status
		const unsigned int bpMb = 1048576;	// bytes per megabyte
		if(m_TcpServer.m_Servers.count())
		{
			// Start with headers
			const char* colhdr[10] = {	"Client",
										"Bytes Tx",
										"Bytes Rx",
										"Bad"};
			for(int ic=0; ic<4; ic++)
			{
				out << colhdr[ic];
				out << " ";
			}
			out << '\n';

			int idx = 1;
			QString stat;
			for(QVector<DtMailServer*>::const_iterator it =
					m_TcpServer.m_Servers.constBegin();
					it != m_TcpServer.m_Servers.constEnd(); it++, idx++)
			{
				stat = QString("%1 ").arg(idx++, 6, 10);
				//stat.sprintf("%6d ", idx++);
				out << stat;

				unsigned int btx = (*it)->BytesTx();
				if(btx < bpMb)
					stat = QString("%1B ").arg(btx, 7, 10);
					//stat.sprintf("%7dB ", btx);
				else
				{
					double mb = (double)btx/(double)bpMb;
					mb = FROUND(mb, .01);
					stat = QString("%1MB ").arg(mb, 6, 'g');
					//stat.sprintf("%6gMB ", mb);
				}
				out << stat;

				unsigned int brx = (*it)->BytesRx();
				if(brx < bpMb)
					stat = QString("%1B ").arg(brx, 7, 10);
					//stat.sprintf("%7dB ", brx);
				else
				{
					double mb = (double)brx/(double)bpMb;
					mb = FROUND(mb, .01);
					stat = QString("%1MB ").arg(mb, 6, 'g');
					//stat.sprintf("%6gMB ", mb);
				}
				out << stat;

				stat = QString("%1 ").arg((*it)->BadPacketCount(), 3, 10);
				//stat.sprintf("%3d ", (*it)->BadPacketCount());
				out << stat;

				out << '\n';
			}
		}

		file.close();
	}

	if(m_timer)
		m_timer->start(10000);
}

void AuMailServerApp::HandleSignal(int value)
{
	switch(value)
	{
#if !defined(Q_OS_WIN)
	case SIGHUP:
		g_Au.Log(tr("SIGHUP ignored."));
		return;	// Ignore hangup signal.

	case SIGQUIT:
		g_Au.Log(tr("SIGQUIT caught. Shutting down..."));
		break;
#endif

	case SIGINT:
		g_Au.Log(tr("SIGINT caught. Shutting down..."));
		break;

	case SIGTERM:
		g_Au.Log(tr("SIGTERM caught. Shutting down..."));
		break;
	}
	quit();
}

bool AuMailServerApp::IsLoggedIn(unsigned int idPlayer, unsigned int confirmNum, AuId* pIdUnit)
{
	bool isLoggedIn = false;
	QMap<unsigned int, AuMailLogin>& loginCache = LockLoginMap();
	if(loginCache.contains(idPlayer))
	{
		// The player is in our login cache. Get a quick answer unless it's been too
		// long; then we need to really query the login server.
		isLoggedIn = loginCache[idPlayer].IsLoggedIn(idPlayer, confirmNum, pIdUnit);
	}
	else
	{
		// The player is not in our login cache. Add them to the cache and query the
		// login server.
		AuMailLogin login(confirmNum);
		loginCache.insert(idPlayer, login);
		isLoggedIn = login.IsLoggedIn(idPlayer, confirmNum, pIdUnit);
	}
	UnlockLoginMap();
	return isLoggedIn;
}

// Returns the number of worker servers.
int AuMailServerApp::WorkerCount()
{
	QMap<QString,DhMailServer>& serverMap = LockServerMap();
	int workers = serverMap.size();
	UnlockServerMap();
	return workers;
}

// Deletes city reports that have gotten too old to be relevant.
// static
void AuMailServerApp::DeleteOldMail()
{
	QString logmsg;
	logmsg = QString("DeleteOldMail Started");
	g_Au.Log(logmsg);

	// Get the list of mail message types that should be deleted at various intervals.
	// This should comprise all the kinds of mail we store. No messages get to stay around forever.
	QList<AuMailMsg::MSGTYPE> deleteEveryHour;
	QList<AuMailMsg::MSGTYPE> deleteEvery2Days;
	QList<AuMailMsg::MSGTYPE> deleteEveryWeek;
	QList<AuMailMsg::MSGTYPE> deleteEveryMonth;
	QList<AuMailMsg::MSGTYPE> deleteEvery2Months;
	AuMailMsg::ListAutoDeletedTypes(deleteEveryHour, deleteEvery2Days, deleteEveryWeek, deleteEveryMonth, deleteEvery2Months);

	const int oldHour = 1 * 60 * 60;							// number of seconds in an hour
	QString delHour = CreateDeleteQuery(oldHour, deleteEveryHour);

	const int old2Days = 2 * 24 * 60 * 60;						// number of seconds in two days
	QString del2Days = CreateDeleteQuery(old2Days, deleteEvery2Days);

	const int oldWeek = 7 * 24 * 60 * 60;						// number of seconds in a week
	QString delWeek = CreateDeleteQuery(oldWeek, deleteEveryWeek);

	const int oldMonth = 30 * 24 * 60 * 60;						// number of seconds in a month
	QString delMonth = CreateDeleteQuery(oldMonth, deleteEveryMonth);

	const int old2Months = 2 * 30 * 24 * 60 * 60;				// number of seconds in two months
	QString del2Months = CreateDeleteQuery(old2Months, deleteEvery2Months);

	// Form an sql query to delete the old city report messages.
	// Refer to aumailmsg.h for the MSGTYPE enum, which is the value stored in msgType.
	QString sq = QString("UPDATE g_Mail SET deleted = 't' WHERE deleted = 'f' AND (%1 OR %2 OR %3 OR %4 OR %5);")
		.arg(delHour)
		.arg(del2Days)
		.arg(delWeek)
		.arg(delMonth)
		.arg(del2Months);

	// Execute the query.
	int countAffected = 0;
	QSqlDatabase db = QSqlDatabase::database("AuMail", false);
	if(OpenDatabase(db))
	{
		// This scope is needed so the QSqlQuery gets destructed before we close the database.
		// Otherwise we get an error 'Unable to free statement: connection pointer is NULL'.
		{
			QSqlQuery qdel(QString(), db);
			qdel.prepare(sq);
			if(!qdel.exec())
			{
				QSqlError err = qdel.lastError();
				qWarning() << "ERROR: DtMailServer::DeleteOldMail failed" << sq << err.text();
			}
			countAffected = qdel.numRowsAffected();
		}
		db.close();
	}

	logmsg = QString("DeleteOldMail Finished: %1").arg(countAffected);
	g_Au.Log(logmsg);
}

bool AuMailServerApp::OpenDatabase(QSqlDatabase& db)
{
	//qDebug("DtMailServer::OpenDatabase");
	// auMail database
	//   Owner: aumaster
	//   Password: Light
	//   User: audog
	//   Password: Gold
	db.setDatabaseName("auMail");			// NOTE: Name is case sensitive.
	db.setHostName("MailDb.Hazeron.net");
	if(!db.open("audog","Gold"))
	{
		QSqlError er = db.lastError();
		QString sdv = er.driverText();
		QString sdb = er.databaseText();
		QString logmsg;
		logmsg = QObject::tr("error: Failed to open database connection:\n%2\n%3\n")
			.arg(sdv).arg(sdb);
		g_Au.Log(logmsg);
		return false;
	}
	//qDebug("DtMailServer::OpenDatabase - ok");
	return true;
}

unsigned int AuMailServerApp::CreateMessageId(QSqlDatabase& db)
{
	unsigned int idret = 0;
	if(db.isOpen())
	{
		QSqlQuery qid("SELECT nextval('g_Mail_id_seq');", db);
		if(qid.first())
			idret = qid.value(0).toInt();
	}
	Q_ASSERT(idret != 0);
	return idret;
}

void AuMailServerApp::MailMessageReceived(quint32 idPlayer, AuMailMsg msg)
{
	// Open the mail database.
	QSqlDatabase db = QSqlDatabase::database("AuMail", false);
	if(OpenDatabase(db))
	{
		// Store the mail message into the database.
		// Server sets sender and time.
		msg.m_id = CreateMessageId(db);
		msg.m_idPlayer = idPlayer;
		msg.m_tmSent.Set();
		bool stored = false;
		if(msg.ApplyFieldLimits())
		{
			msg.Store(db);
			stored = true;
		}
		db.close();

		QString logmsg;
		if(stored)
			logmsg = QObject::tr("Message %2: Player %3, %4 %5 -> %6 '%7'")
				.arg(msg.m_id)
				.arg(idPlayer)
				.arg(msg.m_nameFrom)
				.arg(msg.m_idFrom.GetID())
				.arg(msg.m_idTo.GetID())
				.arg(msg.m_subject);
		else
			logmsg = QObject::tr("error: Invalid Message %2: Player %3, %4 %5 -> %6 '%7' NOT STORED")
				.arg(msg.m_id)
				.arg(idPlayer)
				.arg(msg.m_nameFrom)
				.arg(msg.m_idFrom.GetID())
				.arg(msg.m_idTo.GetID())
				.arg(msg.m_subject);
		g_Au.Log(logmsg);
	}
}

void AuMailServerApp::MailMessageReceivedBulk(quint32 idPlayer, AuMailMsg msg, AuIdList recipients)
{
	// Open the mail database.
	QSqlDatabase db = QSqlDatabase::database("AuMail", false);
	if(OpenDatabase(db))
	{
		// Store the mail message into the database.
		// Server sets sender and time.
		for(AuIdList::const_iterator it = recipients.constBegin(); it != recipients.constEnd(); it++)
		{
			msg.m_idTo = (*it);
			msg.m_id = CreateMessageId(db);
			msg.m_idPlayer = idPlayer;
			msg.m_tmSent.Set();
			bool stored = false;
			if(msg.ApplyFieldLimits())
			{
				msg.Store(db);
				stored = true;
			}

			QString logmsg;
			if(stored)
				logmsg = QObject::tr("Message %2: Player %3, %4 %5 -> %6 '%7'")
					.arg(msg.m_id)
					.arg(idPlayer)
					.arg(msg.m_nameFrom)
					.arg(msg.m_idFrom.GetID())
					.arg(msg.m_idTo.GetID())
					.arg(msg.m_subject);
			else
				logmsg = QObject::tr("error: Invalid Message %2: Player %3, %4 %5 -> %6 '%7' NOT STORED")
					.arg(msg.m_id)
					.arg(idPlayer)
					.arg(msg.m_nameFrom)
					.arg(msg.m_idFrom.GetID())
					.arg(msg.m_idTo.GetID())
					.arg(msg.m_subject);
			g_Au.Log(logmsg);
		}
		db.close();
	}
}

void AuMailServerApp::DeleteMailMessages(quint32 idPlayer, AuId idAvatar, AuMailMsgIdList delids)
{
	int numids = delids.size();
	QSqlDatabase db = QSqlDatabase::database("AuMail", false);
	if(OpenDatabase(db))
	{
		// Form an sql query to mark the messages deleted that are addressed to idUnit. This query can get
		// too big for the SQL server, so break it into manageable bites.
		while(!delids.isEmpty())
		{
			QString sdel;
			unsigned int maxdels = 100;
			AuMailMsgIdList::iterator di = delids.begin();
			while(di != delids.end() && maxdels)
			{
				if(sdel.isEmpty())
					sdel = QString("id = %1").arg(*di);
				else
					sdel += QString(" OR id = %1").arg(*di);
				di = delids.erase(di);
				maxdels--;
			}

			QString sq = QString("UPDATE g_Mail SET deleted = 't' WHERE idTo = %1 AND (%2);")
				.arg(idAvatar.GetID())
				.arg(sdel);

			QSqlQuery qdel(QString(), db);
			qdel.prepare(sq);
			qdel.exec();
		}
		db.close();
	}
	if(numids)
	{
		QString logmsg;
		logmsg = QObject::tr("Request: Player %2 deleted %3 messages.")
			.arg(idPlayer)
			.arg(numids);
		g_Au.Log(logmsg);
	}
}

void AuMailServerApp::RequestMailMessages(quint32 idPlayer, AuId idAvatar, quint32 maxMsgId, AuIdList squelchIds,
		quint8 cityRptAutoDel, quint32 cityRptOldAge, quint8 fleetRptAutoDel, quint32 fleetRptOldAge, quint8 officerRptAutoDel,
		quint32 officerRptOldAge)
{
	AuMailMsgArray msgs;
	QSqlDatabase db = QSqlDatabase::database("AuMail", false);
	if(OpenDatabase(db))
	{
		// Load messages from the database that need to be sent to the client.
		msgs.Load(db, idAvatar, maxMsgId);
		db.close();
	}

	// See if any of the loaded messages needs to be squelched or auto deleted.
	AuMailMsgIdList delids;	// list of squelched or auto deleted message ids
	AuMailMsgArray::iterator it = msgs.begin();
	while(it != msgs.end())
	{
		if(squelchIds.contains((*it).m_idFrom) ||
				(*it).IsReadyToAutoDelete(cityRptAutoDel, cityRptOldAge, fleetRptAutoDel, fleetRptOldAge, officerRptAutoDel,
					officerRptOldAge))
		{
			// Remove the message and save its id for deletion.
			delids.append((*it).m_id);
			it = msgs.erase(it);
		}
		else
			it++;
	}

	if(!delids.empty())
		DeleteMailMessages(idPlayer, idAvatar, delids);

	// DtMailServer thread is expecting a response, even if it is empty.
	emit PlayerMailMessages(idPlayer, msgs);

	// Send delivery receipts for diplomatic messages.
	for(AuMailMsgArray::const_iterator it = msgs.cbegin(); it != msgs.cend(); it++)
	{
		if((*it).Type() == AuMailMsg::MSG_CityDiplomaticMessage)
		{
			// Send a delivery receipt for the diplomatic message.
			// Open the mail database.
			QSqlDatabase db = QSqlDatabase::database("AuMail", false);
			if(OpenDatabase(db))
			{
				int divi = (*it).m_body.indexOf("</div>");
				if(divi >= 0)
				{
					// Build a receipt message starting with the header from the diplomatic message,
					// which identifies the city.
					QString msgtext = (*it).m_body.left(divi + 6);

					// Replace date/time diplo msg originally sent with current date/time, for receipt.
					int dti = msgtext.indexOf("<");
					QString dts(msgtext.left(dti));
					msgtext = g_Au.EncodeUTCString() + msgtext.mid(dti);

					msgtext += tr("<div style=\"margin-left: 10px;\"><p>Excellency,</p>"
							"<p>You sent a diplomatic message at the city shown above on %1. "
							"A courier was dispatched to deliver your message to its leaders.</p>"
							"<p>The courier reports successful delivery of your diplomatic message.</p>"
							"<p>Best Regards,<br>"
							"<p>ParcX Courier Service<br>"
							"<small><i>The Universe On Time</i></small></p>"
							"</div>").arg(dts);

					AuMailMsg msg(*it);
					msg.m_body = msgtext;
					msg.m_idTo = (*it).m_idFrom;
					msg.m_idFrom = AuId::MailServer();
					msg.m_msgType = AuMailMsg::MSG_CityDiplomaticReceipt;
					msg.m_nameFrom = tr("ParcX Courier");
					msg.m_subject = tr("Diplomatic Message Delivery Receipt");

					// Server sets sender and time.
					msg.m_id = CreateMessageId(db);
					msg.m_idPlayer = idPlayer;
					msg.m_tmSent.Set();
					bool stored = false;
					if(msg.ApplyFieldLimits())
					{
						msg.Store(db);
						stored = true;
					}
					db.close();

					QString logmsg;
					if(stored)
						logmsg = QObject::tr("Delivery Receipt %2: Player %3, %4 %5 -> %6 '%7'")
							.arg(msg.m_id)
							.arg(idPlayer)
							.arg(msg.m_nameFrom)
							.arg(msg.m_idFrom.GetID())
							.arg(msg.m_idTo.GetID())
							.arg(msg.m_subject);
					else
						logmsg = QObject::tr("error: Invalid Message %2: Player %3, %4 %5 -> %6 '%7' NOT STORED")
							.arg(msg.m_id)
							.arg(idPlayer)
							.arg(msg.m_nameFrom)
							.arg(msg.m_idFrom.GetID())
							.arg(msg.m_idTo.GetID())
							.arg(msg.m_subject);
					g_Au.Log(logmsg);
				}
			}
		}
	}
}

// Creates a fragment of an sql query that will delete all messages of 'typesToDelete' that are older than 'secondsTooOld'.
//static
QString AuMailServerApp::CreateDeleteQuery(int secondsTooOld, const QList<AuMailMsg::MSGTYPE>& typesToDelete)
{
	// Determine the too old date.
	QDateTime tmTooOld = g_Au.CurrentUTC().addSecs(-secondsTooOld);
	QString sdate = tmTooOld.toString(Qt::ISODate);

	// Build the sql query fragment that selects the old messages of the types we want to delete.
	QString del = QString("(tmSent < '%1' AND (").arg(sdate);
	for(QList<AuMailMsg::MSGTYPE>::const_iterator di = typesToDelete.constBegin(); di != typesToDelete.constEnd(); di++)
	{
		if(di == typesToDelete.constBegin())
			del += QString("msgType = %1").arg(*di);
		else
			del += QString(" OR msgType = %1").arg(*di);
	}
	del += "))";

	// Return the sql query fragment.
	return del;
}
