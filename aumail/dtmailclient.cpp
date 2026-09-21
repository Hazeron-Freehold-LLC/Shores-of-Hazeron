//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtmailclient.cpp,v 1.55 2022/04/12 20:49:11 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dtmailclient.h"
//
#include "aufilever.h"
#include "auglobal.h"
#include "dgdata.h"
#include "dtmailserver.h"

DtMailClient::DtMailClient(QObject* parent) : QThread(parent), m_semAlive(1)
{
	m_BytesRx = 0;
	m_BytesTx = 0;
	m_DeferredToWorker = false;
	m_MailRequested = false;
	m_MailServerPort = 0;
	m_MailVisible = false;
	m_UpLongEnough = false;

	m_semAlive.acquire(1);	// I live...
}

DtMailClient::~DtMailClient()
{
}

void DtMailClient::run()
{
	//qDebug("DtMailClient::run");
	m_UpTime.start();
	const int rxInterval = 10 * 60;	// seconds between mail receive polls
	while(Alive())
	{
		const DgData& gd = g_Au.LockDataForRead();
		bool isLoggedIn = gd.IsClientLoggedIn();
		g_Au.UnlockData();
		if(isLoggedIn)
		{
			// When alert notifications are enabled, we may have to check mail regardless of the visibility of the mail window.
			bool mustFetchForAlerts;
			if(g_Au.IsSceneApp() || g_Au.IsMailClient())
			{
				if(g_Au.MailNotificationMode() == 0)
					mustFetchForAlerts = false;						// Notifications disabled.
				else if(g_Au.MailNotificationMode() == 2)
					mustFetchForAlerts = true;						// Periodic notifications desired.
				else if(!m_LastRx.isValid())
					mustFetchForAlerts = true;						// First time notifications desired.
				else
					mustFetchForAlerts = false;						// First time notifications already fetched.
			}
			else
				mustFetchForAlerts = false;

			if(m_MailVisible || mustFetchForAlerts)
			{
				if(!m_UpLongEnough)
				{
					if(m_UpTime.elapsed()/d_MSecPerMin >= 1)
					{
						// I have been running for a minute. It's ok to do something now.
						m_UpLongEnough = true;
						//qDebug() << "DtMailClient::run up long enough";
					}
				}
				else if(!m_idPerson.IsNull())
				{
					// Periodically pull in any mail that is waiting for the person.
					// Scene server does not currently get any mail
					// However, requesting mail may get us deferred to a different server which we need to use for transmissions.
					if(!m_LastRx.isValid() || m_LastRx.elapsed()/d_MSecPerSec > rxInterval || m_MailRequested)
					{
						//qDebug() << "DtMailClient::run RxMail" << m_idPerson;
						RxMail(m_idPerson);
						m_MailRequested = false;
						m_LastRx.restart();
					}
				}
			}
			else if(!m_DeferredToWorker)
			{
				// Must contact server for handoff to appropriate worker or TxMail won't work.
				//qDebug() << "DtMailClient::run RxMail to defer";
				RxMail(AuId(0));
			}

			// Send any mail that's ready to go.
			TxMail();

			// Send deletion requests.
			TxDeletions();
		}

		if(g_Au.IsMailClient())
			msleep(1000);	// Running as mail client so be fairly responsive.
		else
			msleep(5000);	// Running in game or server mode so hang back a bit; no server mail is all that critical.
	}
	//qDebug("DtMailClient::run - Exiting.");
}

void DtMailClient::NetworkError(QAbstractSocket::SocketError socketError)
{
	switch(socketError)
	{
	case QAbstractSocket::RemoteHostClosedError:
		qWarning("NetworkError - RemoteHostClosedError");
		break;
	case QAbstractSocket::HostNotFoundError:
		qWarning("NetworkError - HostNotFoundError");
		break;
	case QAbstractSocket::ConnectionRefusedError:
		qWarning("NetworkError - ConnectionRefusedError");
		break;
	default:
		//if(m_Tcp)
			//qWarning("DtMailClient::NetworkError - Error %s",
					//(const char*)m_Tcp->errorString());
		//else
			qWarning("NetworkError - Error");
		break;
	}
}

void DtMailClient::TxMail()
{
	// In solo play, the RxMail function gets mail directly out of the mail buffer.
	if(g_Au.IsSceneSolo())
		return;

	//qDebug("DtMailClient::TxMail");
	// Check to see if we have anything to send in the mail stream.
	QByteArray& tba = g_Au.LockMailBuffer("DtMailClient::TxMail");
	if(!tba.isEmpty())
	{
		//qDebug("DtMailClient::TxMail - %d bytes", tba.count());
		// There is mail to be sent.
		// It is already formatted into packets with block sizes.
		// Connect to the mail server.
		QTcpSocket tcpSocket;
		//tcpSocket.abort();
		tcpSocket.connectToHost(m_MailServerName, m_MailServerPort);
		tcpSocket.waitForConnected(5000);
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			g_Au.SetNet4Uploading(100);
			// The connection attempt succeeded.
			int written = tcpSocket.write(tba);
			tba.clear();
			g_Au.UnlockMailBuffer("DtMailClient::TxMail");
			m_BytesTx += written;

			// Loop until all the data has been sent.
			while(tcpSocket.state() == QAbstractSocket::ConnectedState &&
					tcpSocket.waitForBytesWritten(10000))
			{}
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.disconnectFromHost();
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.waitForDisconnected(10000);
		}
		else
		{
			g_Au.UnlockMailBuffer("DtMailClient::TxMail");
			qWarning("ERROR! Failed to connect to mail server.");
		}
	}
	else
		g_Au.UnlockMailBuffer("DtMailClient::TxMail");
	//qDebug("DtMailClient::TxMail - done %d", m_BytesTx);
}

void DtMailClient::TxDeletions()
{
	// In solo play, no server needs to be notified of deleted mail.
	if(g_Au.IsSceneSolo())
	{
		m_DeleteQueue.clear();
		return;
	}

	// Notify server of mail the client wants to delete.
	// This is also how we notify the server to delete messages that have been successfully cached locally.
	if(m_DeleteQueue.count())
	{
		// Get player id and confirmation number for identity.
		const DgData& gd = g_Au.LockDataForRead();
		quint32 idPlayer = gd.PlayerId();
		quint32 confirmNum = gd.ConfirmationNumber();
		g_Au.UnlockData();

		// Make a packet of the message id's to delete.
		// Connect to the mail server.
		QTcpSocket tcpSocket;
		tcpSocket.connectToHost(m_MailServerName, m_MailServerPort);
		tcpSocket.waitForConnected(5000);
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			g_Au.SetNet4Uploading(100);
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtMailServer::P_Delete;
			QString ver(AuUtil::LoginVersion());
			out << ver;
			out << idPlayer;
			out << confirmNum;
			out << m_idPerson;
			quint32 numids = m_DeleteQueue.count();
			out << numids;
			for(unsigned int id=0; id<numids; id++)
				out << m_DeleteQueue[id];
			m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
			m_BytesTx += tcpSocket.write(ba);
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.waitForBytesWritten(10000);
			tcpSocket.disconnectFromHost();
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.waitForDisconnected(10000);
			m_DeleteQueue.clear();
		}
	}
}

// 'personId' is null to make contact with servers but not really fetch any mail.
void DtMailClient::RxMail(const AuId& personId)
{
	if(g_Au.IsSceneSolo())
	{
		RxMailSolo(personId);
		return;
	}

	// We always start at the login master for deferral to a login worker. If we cache the worker name we get stuck if it crashes.
	m_MailServerName = g_Au.MailMasterName();
	m_MailServerPort = AuUtil::MailMasterPort();

	//qDebug() << "DtMailClient::RxMail" << personId;
	bool deferred;
	do
	{
		//qDebug("DtMailClient::RxMail");
		deferred = false;
		QTcpSocket tcpSocket;
		tcpSocket.connectToHost(m_MailServerName, m_MailServerPort);
		tcpSocket.waitForConnected(5000);
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			g_Au.SetNet4Uploading(100);
			// Get player id and confirmation number for identity.
			const DgData& gd = g_Au.LockDataForRead();
			quint32 idPlayer = gd.PlayerId();
			quint32 confirmNum = gd.ConfirmationNumber();
			g_Au.UnlockData();
			//qDebug() << "idPlayer" << idPlayer << "personId" << personId;

			quint32 unused = 0;

			// Build the list of squelch ids
			AuIdList SquelchIds;
			if(!personId.IsNull())
			{
				const DgData& gd1 = g_Au.LockDataForRead();
				for(QList<AuContact>::const_iterator it = gd1.GetContacts().constBegin(); it != gd1.GetContacts().constEnd(); it++)
				{
					if((*it).m_bIgnore)
						SquelchIds.AddIfUnique((*it).m_id);
				}
				g_Au.UnlockData();
			}

			// Send a mail request packet.
			//qDebug("DtMailClient::RxMail - send Request");
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtMailServer::P_Request;	// mail request packet type
			QString ver(AuUtil::LoginVersion());
			out << ver;
			out << idPlayer;
			out << confirmNum;
			out << personId;
			out << unused;
			SquelchIds >> out;
			out << (quint8)g_Au.MailCityReportAutoDelete();
			if(g_Au.MailCityReportAutoDelete())
				out << (quint32)g_Au.MailCityReportOldAge();
			out << (quint8)g_Au.MailFleetReportAutoDelete();
			if(g_Au.MailFleetReportAutoDelete())
				out << (quint32)g_Au.MailFleetReportOldAge();
			out << (quint8)g_Au.MailOfficerReportAutoDelete();
			if(g_Au.MailOfficerReportAutoDelete())
				out << (quint32)g_Au.MailOfficerReportOldAge();
			m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
			m_BytesTx += tcpSocket.write(ba);
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.waitForBytesWritten(10000);
			//qDebug("DtMailClient::RxMail - Request sent");

			// Expect zero to many mail message packets now.
			// We will get a "done" packet at the end.
			// Either way, the server will respond with something.
			tcpSocket.waitForReadyRead(10000);
			//qDebug("DtMailClient::RxMail - ready to read");

			// Read data packets until we get the done packet.
			quint32 packetSize = 0;
			int packetSizeBytesLeft = 0;
			bool done = false;
			while(!done)
			{
				while(tcpSocket.bytesAvailable())
				{
					g_Au.SetNet4Downloading(100);
					if(!packetSize || packetSizeBytesLeft)
					{
						int packetSizeBytesRead = AuUtil::ReadPacketSize(tcpSocket, packetSize, packetSizeBytesLeft);
						if(packetSizeBytesLeft)
							break;					// We don't have all of the packet size bytes yet.
						m_BytesRx += packetSizeBytesRead;
					}

					// Wait until all of the bytes arrive.
					if(tcpSocket.bytesAvailable() < packetSize)
						break;

					// It is now safe to read our data from the socket buffer.
					QByteArray block = tcpSocket.read(packetSize);
					m_BytesRx += packetSize;
					packetSize = 0;
					int result = ReadPacket(block);
					if(result)
					{
						done = true;
						if(result == 2)
						{
							deferred = true;
							m_DeferredToWorker = true;
						}
						break;
					}
				}

				// Wait for more data if we haven't received the done packet.
				if(!done && tcpSocket.state() == QAbstractSocket::ConnectedState)
					tcpSocket.waitForReadyRead(1000);
			}

			// Close the socket.
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.disconnectFromHost();
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.waitForDisconnected(10000);
		}
	}
	while(deferred);
	//qDebug("DtMailClient::RxMail - done");
}

void DtMailClient::RxMailSolo(const AuId& personId)
{
	if(personId.IsNull())
		return;
	// Take mail from the mail queue and treat it like incoming packets.
	QByteArray& tba = g_Au.LockMailBuffer("DtMailClient::TxMail");
	if(!tba.isEmpty())
	{
		QDataStream readSocket(tba);	// Our fake TCP socket.
		readSocket.setVersion(QDataStream::Qt_4_1);

		QByteArray leaveMail;	// Mail to be left in the queue.

		// Read data packets until we run out of packets.
		while(!readSocket.atEnd())
		{
			quint32 packetSize = 0;
			int packetSizeBytesLeft = 0;
			AuUtil::ReadPacketSize(readSocket, packetSize, packetSizeBytesLeft);
			if(packetSizeBytesLeft)
				AuUtil::ReadPacketSize(readSocket, packetSize, packetSizeBytesLeft);
			Q_ASSERT(!packetSizeBytesLeft);

			// Read our data from the socket buffer.
			char* rawBytes = new char[packetSize];
			int len = readSocket.readRawData(rawBytes, packetSize);
			Q_ASSERT(len > 0);
			if(len > 0)
			{
				QByteArray block(rawBytes, len);
				delete [] rawBytes;

				int result = ReadPacket(block);
				if(result == 3)
				{
					// The mail message must be put back into the mail queue.
					AuUtil::WritePacketSize(leaveMail, block.size());
					leaveMail.append(block);
				}
			}
		}

		// Leave the mail in the queue that we don't wan't. Could be none.
		tba = leaveMail;
	}
	g_Au.UnlockMailBuffer("DtMailClient::TxMail");
}

// return 0=ok, 1=done, 2=deferred, 3=solo not for me
int DtMailClient::ReadPacket(const QByteArray& ba)
{
	//qDebug("DtMailClient::ReadPacket");
	QDataStream block(ba);
	block.setVersion(QDataStream::Qt_4_1);

	// The first byte is always the packet type.
	unsigned char type;
	block >> type;
	switch(type)
	{
	case DtMailServer::P_DeferFailed:
		emit DeferFailed();
		break;

	case DtMailServer::P_Deferred:
		{
			block >> m_MailServerName;
			block >> m_MailServerPort;
		}
		return 2;

	case DtMailServer::P_DoneOk:	// Server says their's no more mail for me.
		break;

	case DtMailServer::P_LoginError:
		emit LoginError();
		break;

	case DtMailServer::P_Message:	// Mail message
		if(g_Au.IsSceneSolo())
		{
			// In solo we are pulling incoming messages directly from the outbound message queue.
			// Client inserts login version and player credentials. I don't need them, just skip them.
			QString ver;
			quint32 idPlayer, confirmNum;
			block >> ver;
			block >> idPlayer;
			block >> confirmNum;

			// Read the message record from the data stream.
			AuMailMsg msg;
			msg.Rx(block);

			if(msg.m_idTo != m_idPerson)
			{
				// In solo play, we put the message back into the local mail buffer when its not for current player.
				return 3;
			}
			else if(msg.ApplyFieldLimits())
			{
				// Watch for messages that the person considers too old to deliver.
				if(!msg.IsReadyToAutoDelete(g_Au.MailCityReportAutoDelete(), g_Au.MailCityReportOldAge(),
							g_Au.MailFleetReportAutoDelete(), g_Au.MailFleetReportOldAge(),
							g_Au.MailOfficerReportAutoDelete(), g_Au.MailOfficerReportOldAge()))
				{
					// Store the message in the local message cache folder.
					// The mail server assigns a unique id to each message as it arrives. In solo we must assign an id now, as each
					// message is delivered.
					msg.m_id = g_Au.NextMailMessageId();

					// This will notify the server to delete the message after the save is successful.
					StoreMessageInCache(msg);

					emit NewMessageReceived(msg);
				}
				else
				{
					// The message gets deleted simply by ignoring it.
				}
			}
			else
				qWarning("ERROR! Received invalid mail message size %d.", ba.count());
		}
		else
		{
			// Read the message record from the data stream.
			AuMailMsg msg;
			msg.Rx(block);

			if(msg.m_idTo != m_idPerson)
			{
				// The current person was probably changed before the mail request was completed.
				// We can safely ignore messages that are not addressed to the current person.
			}
			else if(msg.ApplyFieldLimits())
			{
				// Watch for messages that the person considers too old to deliver.
				if(!msg.IsReadyToAutoDelete(g_Au.MailCityReportAutoDelete(), g_Au.MailCityReportOldAge(),
							g_Au.MailFleetReportAutoDelete(), g_Au.MailFleetReportOldAge(),
							g_Au.MailOfficerReportAutoDelete(), g_Au.MailOfficerReportOldAge()))
				{
					// Store the message in the local message cache folder.
					// This will notify the server to delete the message after the save is successful.
					StoreMessageInCache(msg);

					emit NewMessageReceived(msg);
				}
				else
				{
					emit MessageDiscarded();

					// Tell the server to delete the message.
					m_DeleteQueue += msg.m_id;							// Queue the message id for deletion in a batch.

					// Don't let the deletion queue get too big.
					if(m_DeleteQueue.count() > 12)
						TxDeletions();
				}
			}
			else
				qWarning("ERROR! Received invalid mail message size %d.", ba.count());
		}
		return 0;	// ok, not done

	case DtMailServer::P_VersionError:
		emit VersionError();
		break;

	case DtMailServer::P_CheckIn:	// Client should never get this.
	case DtMailServer::P_Delete:	// Client should never get this.
	case DtMailServer::P_Request:	// Client should never get this.
	case DtMailServer::P_BulkMessage:	// Client should never get this.
	default:
		qWarning("ERROR! Unknown mail packet type %d size %d", type, ba.count());
		break;
	}
	return 1;	// done
}

void DtMailClient::SelectPerson(AuId id)
{
	//qDebug() << "DtMailClient::SelectPerson" << id;
	// Set the atom id of the person whose mail we will manage now.
	m_idPerson = id;

	// Load messages that were cached locally.
	LoadMessageCache();

	// Request mail for the new player.
	m_MailRequested = true;
}

void DtMailClient::MailVisibilityChanged(bool isVisible)
{
	m_MailVisible = isVisible;
}

void DtMailClient::StoreMessageInCache(const AuMailMsg& msg)
{
	// Insure Mail folder exists.
	QString mailPath(g_Au.CacheMail());
	QDir mailDir(mailPath);
	if(!mailDir.exists())
		mailDir.mkpath(mailPath);

	// Construct a file name.
	mailPath += QString("/%1.%2.%3.m").arg(msg.m_idTo.StringId()).arg(msg.m_idFrom.StringId()).arg(msg.m_id);

	// Save the mail message file.
	QFile file(mailPath);
	if(file.open(QFile::WriteOnly))
	{
		QDataStream ds(&file);
		ds.setVersion(QDataStream::Qt_4_1);

		// Type/version stamp the file.
		ds << (quint8)MAIL_FILE_TYPE;
		ds << (quint8)MAIL_FILE_VER;

		// Store the message
		msg.Tx(ds);

		file.close();

		// File saved ok so tell the server to delete the message.
		m_DeleteQueue += msg.m_id;							// Queue the message id for deletion in a batch.

		// Don't let the deletion queue get too big.
		if(m_DeleteQueue.count() > 12)
			TxDeletions();
	}
}

void DtMailClient::LoadMessageCache()
{
	// Insure Mail folder exists.
	QString mailPath(g_Au.CacheMail());
	QDir mailDir(mailPath);
	if(!mailDir.exists())
		return;							// Nothing to load if there's no mail folder.

	QStringList nameFilters;
	nameFilters << QString("%1.*.*.m").arg(m_idPerson.StringId());				// Load all mail messages to the current player.
	QStringList mailFiles = mailDir.entryList(nameFilters);
	emit MailBatchStart(mailFiles.count());

	for(QStringList::const_iterator mfi = mailFiles.constBegin(); mfi != mailFiles.constEnd(); mfi++)
	{
		/*
		// Decode the message id from the file name.
		QStringList nameParts = (*mfi).split('.');
		quint32 msgId = QVariant(nameParts[2]).toUInt();
		emit CachedMessageReceived(msgId);
		*/

		// Load the mail message file.
		bool deleteFile = false;		// true if we determine the message should be deleted due to being too old.
		QFile file(mailPath + "/" + *mfi);
		if(file.open(QFile::ReadOnly))
		{
			QDataStream ds(&file);
			ds.setVersion(QDataStream::Qt_4_1);

			// Read type/version stamp of the file.
			quint8 fileType, fileVer;
			ds >> fileType;
			ds >> fileVer;

			if(fileType == MAIL_FILE_TYPE && fileVer == MAIL_FILE_VER)
			{
				// Read the message.
				AuMailMsg msg;
				msg.Rx(ds);

				// Watch for messages that the person considers too old to deliver.
				if(!msg.IsReadyToAutoDelete(g_Au.MailCityReportAutoDelete(), g_Au.MailCityReportOldAge(),
							g_Au.MailFleetReportAutoDelete(), g_Au.MailFleetReportOldAge(),
							g_Au.MailOfficerReportAutoDelete(), g_Au.MailOfficerReportOldAge()))
				{
					if(msg.m_idTo == m_idPerson && msg.ApplyFieldLimits())
						emit CachedMessageReceived(msg);
				}
				else
				{
					// The message is ready to be autodeleted.
					deleteFile = true;
				}
			}

			file.close();
		}

		// Delete messages that are too old from the local cache.
		if(deleteFile)
			mailDir.remove(*mfi);
	}

	emit MailBatchEnd();
}

// Loads a mail message from the local cache.
// 'msgId' is an input parameter to specify which message to load.
// 'msg' is an output parameter, only valid if true is returned.
// Returns true on success.
//static
bool DtMailClient::LoadMessage(quint32 msgId, AuMailMsg& msg)
{
	// Insure Mail folder exists.
	QString mailPath(g_Au.CacheMail());
	QDir mailDir(mailPath);
	if(!mailDir.exists())
		return false;					// Nothing to load if there's no mail folder.

	QStringList nameFilters;
	nameFilters << QString("*.*.%1.m").arg(msgId);				// Load the message with the id.
	QStringList mailFiles = mailDir.entryList(nameFilters);
	if(mailFiles.isEmpty())
		return false;					// No matching message file found.

	// Load the mail message file.
	bool loadOk = false;
	QFile file(mailPath + "/" + mailFiles.first());
	if(file.open(QFile::ReadOnly))
	{
		QDataStream ds(&file);
		ds.setVersion(QDataStream::Qt_4_1);

		// Read type/version stamp of the file.
		quint8 fileType, fileVer;
		ds >> fileType;
		ds >> fileVer;

		if(fileType == MAIL_FILE_TYPE && fileVer == MAIL_FILE_VER)
		{
			// Read the message.
			msg.Rx(ds);
			loadOk = true;
		}

		file.close();
	}
	return loadOk;
}

//static
void DtMailClient::DeleteOldMail(const AuId& toId, QList<quint32>& oldMailIds)
{
	//g_Au.Log(QString("DtMailClient::DeleteOldMail to %1 started").arg(toId.GetID()));
	if(g_Au.MailCityReportAutoDelete() || g_Au.MailFleetReportAutoDelete() || g_Au.MailOfficerReportAutoDelete())
	{
		// Autodeletion is enabled.
		// Insure Mail folder exists.
		QString mailPath(g_Au.CacheMail());
		QDir mailDir(mailPath);
		if(mailDir.exists())
		{
			QStringList nameFilters;
			nameFilters << QString("%1.*.*.m").arg(toId.StringId());			// All mail messages to the player id.
			QStringList mailFiles = mailDir.entryList(nameFilters);
			//g_Au.Log(QString("DtMailClient::DeleteOldMail %1 mail files").arg(mailFiles.count()));

			for(QStringList::const_iterator mfi = mailFiles.constBegin(); mfi != mailFiles.constEnd(); mfi++)
			{
				// Load the mail message file.
				bool deleteFile = false;		// true if we determine the message should be deleted due to being too old.
				QFile file(mailPath + "/" + *mfi);
				if(file.open(QFile::ReadOnly))
				{
					QDataStream ds(&file);
					ds.setVersion(QDataStream::Qt_4_1);

					// Read type/version stamp of the file.
					quint8 fileType, fileVer;
					ds >> fileType;
					ds >> fileVer;

					if(fileType == MAIL_FILE_TYPE && fileVer == MAIL_FILE_VER)
					{
						// Read the message.
						AuMailMsg msg;
						msg.Rx(ds);

						// Watch for messages that the person considers too old to deliver.
						if(msg.IsReadyToAutoDelete(g_Au.MailCityReportAutoDelete(), g_Au.MailCityReportOldAge(),
									g_Au.MailFleetReportAutoDelete(), g_Au.MailFleetReportOldAge(),
									g_Au.MailOfficerReportAutoDelete(), g_Au.MailOfficerReportOldAge()))
						{
							// The message is ready to be autodeleted.
							oldMailIds.append(msg.m_id);
							deleteFile = true;
						}
					}

					file.close();
				}

				// Delete messages that are too old from the local cache.
				if(deleteFile)
					mailDir.remove(*mfi);
			}
		}
	}
	//g_Au.Log(QString("DtMailClient::DeleteOldMail to %1 done").arg(toId.GetID()));
}
