//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtmailserver.cpp,v 1.82 2022/04/07 21:51:57 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dtmailserver.h"
//
#include <auglobal.h>
#include "aumailserverapp.h"
#include <QHostAddress>

DtMailServer::DtMailServer(int socketDesc, QObject* parent) : QThread(parent)
{
	m_BadPackets = 0;
	m_BytesRx = 0;
	m_BytesTx = 0;
	m_idPlayer = 0;
	m_LocalClient = false;
	m_MailMsgsReady = false;
	m_PacketSize = 0;
	m_PacketSizeBytesLeft = 0;
	m_socketDescriptor = socketDesc;
}

void DtMailServer::run()
{
	// Creating the socket on the stack automatically associates it with the
	// thread's event loop. This ensures that Qt will not try to deliver events
	// to our socket from the main thread while we are accessing it from
	// DtMailServer::run().
	QTcpSocket tcpSocket;
	if(!tcpSocket.setSocketDescriptor(m_socketDescriptor))
	{
		emit error(tcpSocket.error());
		return;
	}
	//connect(&tcpSocket, SIGNAL(readyRead()), SLOT(RxData()));
	//connect(&tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
			//SLOT(NetworkError(QAbstractSocket::SocketError)));

	//qDebug("DtMailServer::run - Connection established.");
	QString peerAddr = tcpSocket.peerAddress().toString();
	if(peerAddr.left(8) == "192.168." || peerAddr == "127.0.0.1" || peerAddr.left(15) == "::ffff:192.168.")
		m_LocalClient = true;	// This is cheesy but it works for my internal purposes.
	QString logmsg;
	logmsg = QObject::tr("%1 Incoming connection from %2.")
			.arg((unsigned long)currentThreadId()).arg(peerAddr);
	g_Au.Log(logmsg);

	tcpSocket.waitForReadyRead(10000);
	while(tcpSocket.state() == QAbstractSocket::ConnectedState ||
			tcpSocket.bytesAvailable())
	{
		// Pull in any data that is waiting for us.
		// Note: If we wait for the signal, it appears to come from the gui
		// thread, which causes us grief. It's better for us to get the data
		// when it's there.
		RxData(tcpSocket);
		TxData(tcpSocket);
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
			tcpSocket.waitForDisconnected(10);
	}

	logmsg = QObject::tr("%1 Connection with %2 terminated.")
			.arg((unsigned long)currentThreadId()).arg(peerAddr);
	g_Au.Log(logmsg);
}

void DtMailServer::RxData(QTcpSocket& tcpSocket)
{
	while(tcpSocket.bytesAvailable())
	{
		if(!m_PacketSize || m_PacketSizeBytesLeft)
		{
			int packetSizeBytesRead = AuUtil::ReadPacketSize(tcpSocket, m_PacketSize, m_PacketSizeBytesLeft);
			if(m_PacketSizeBytesLeft)
				break;					// We don't have all of the packet size bytes yet.
			m_BytesRx += packetSizeBytesRead;
		}

		// Wait until all of the bytes arrive.
		if(tcpSocket.bytesAvailable() < m_PacketSize)
			break;

		// Accumulate data until the entire packet arrives.
		QByteArray block = tcpSocket.read(m_PacketSize);
		m_BytesRx += m_PacketSize;
		m_PacketSize = 0;
		ReadPacket(block, tcpSocket);
	}
	//qDebug("DtMailServer::RxData - done");
}

void DtMailServer::ReadPacket(const QByteArray& ba, QTcpSocket& tcpSocket)
{
	//qDebug("DtMailServer::ReadPacket - %d bytes rx'd", ba.count());
	QDataStream block(ba);
	block.setVersion(QDataStream::Qt_4_1);

	// The first byte is always the packet type.
	QString logmsg;
	unsigned char type;
	block >> type;
	switch(type)
	{
	case P_StatusQuery:
		// Sent by AuServerStatus to get master's current status. An immediate reply is expected.
		{
			// Collect status values.
			quint8 status = 0;	// Status type.
			QDateTime tmUp = auMailServerApp->TimeUp();
			quint8 workers = auMailServerApp->WorkerCount();

			// Reply with the status packet.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)P_StatusQuery;
			out << status;
			out << tmUp;
			out << workers;
			m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
			m_BytesTx += tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten();
		}
		break;

	case P_Message:	// Client sent us a mail message.
		//qDebug("DtMailServer::ReadPacket - P_Message");
		{
			// Read client version from packet.
			// NOTE: Only client inserts version into P_Message packet.
			QString clientVer;
			block >> clientVer;
			if(clientVer == QString(AuUtil::LoginVersion()))
			{
				// Version matches.
				if(g_Au.IsMailWorker())
				{
					// Read player id and login confirmation number from packet.
					// NOTE: The P_Message packet only contains this information when
					// sent from client to server. The server does not put the id and
					// confirmation number into P_Message packets sent to the client.
					quint32 idPlayer, confirmNum;
					block >> idPlayer;
					block >> confirmNum;
					if(auMailServerApp->IsLoggedIn(idPlayer, confirmNum))
					{
						// User id and confirmation number must have matched.
						// Read the mail message record from the packet.
						AuMailMsg msg;
						msg.Rx(block);
						if(msg.m_id == 0)
						{
							emit MailMessageReceived(idPlayer, msg);
						}
						else
						{
							logmsg = QObject::tr("%1 ERROR! Message received with id %2. "
									"New messages submitted by clients should have id "
									"= 0. Hacker?").arg((unsigned long)currentThreadId()).arg(msg.m_id);
							g_Au.Log(logmsg);
						}
					}
					else
					{
						logmsg = QObject::tr("%1 ERROR! Message received with invalid login "
								"player %2 confirm %3. Hacker?").arg((unsigned long)currentThreadId())
								.arg(idPlayer).arg(confirmNum);
						g_Au.Log(logmsg);
					}
				}
				else
				{
					logmsg = QObject::tr("%1 ERROR! Message received by Master server. "
							"Hacker?").arg((unsigned long)currentThreadId());
					g_Au.Log(logmsg);
				}
			}
			else
			{
				// Version doesn't match.
				// The message is ignored.
				// The client should already know the version is bad due to our reply
				// to the P_Request. This is worth logging. It might indicate a hacker.
				logmsg = QObject::tr("%1 ERROR! Message received with invalid version '%2'. "
						"Version should have been rejected during the client's initial "
						"request for mail. Hacker?").arg((unsigned long)currentThreadId()).arg(clientVer);
				g_Au.Log(logmsg);
			}
		}
		//qDebug("DtMailServer::ReadPacket - P_Message - done");
		break;

	case P_BulkMessage:	// Client sent us a mail message.
		//qDebug("DtMailServer::ReadPacket - P_Message");
		{
			// Read client version from packet.
			// NOTE: Only client inserts version into P_Message packet.
			QString clientVer;
			block >> clientVer;
			if(clientVer == QString(AuUtil::LoginVersion()))
			{
				// Version matches.
				if(g_Au.IsMailWorker())
				{
					// Read player id and login confirmation number from packet.
					// NOTE: The P_Message packet only contains this information when
					// sent from client to server. The server does not put the id and
					// confirmation number into P_Message packets sent to the client.
					quint32 idPlayer, confirmNum;
					block >> idPlayer;
					block >> confirmNum;
					if(auMailServerApp->IsLoggedIn(idPlayer, confirmNum))
					{
						// User id and confirmation number must have matched.
						// Read the mail message record from the packet.
						AuMailMsg msg;
						msg.Rx(block);
						if(msg.m_id == 0)
						{
							AuIdList recipients;
							recipients << block;
							emit MailMessageReceivedBulk(idPlayer, msg, recipients);
						}
						else
						{
							logmsg = QObject::tr("%1 ERROR! Message received with id %2. "
									"New messages submitted by clients should have id "
									"= 0. Hacker?").arg((unsigned long)currentThreadId()).arg(msg.m_id);
							g_Au.Log(logmsg);
						}
					}
					else
					{
						logmsg = QObject::tr("%1 ERROR! Message received with invalid login "
								"player %2 confirm %3. Hacker?").arg((unsigned long)currentThreadId())
								.arg(idPlayer).arg(confirmNum);
						g_Au.Log(logmsg);
					}
				}
				else
				{
					logmsg = QObject::tr("%1 ERROR! Message received by Master server. "
							"Hacker?").arg((unsigned long)currentThreadId());
					g_Au.Log(logmsg);
				}
			}
			else
			{
				// Version doesn't match.
				// The message is ignored.
				// The client should already know the version is bad due to our reply
				// to the P_Request. This is worth logging. It might indicate a hacker.
				logmsg = QObject::tr("%1 ERROR! Message received with invalid version '%2'. "
						"Version should have been rejected during the client's initial "
						"request for mail. Hacker?").arg((unsigned long)currentThreadId()).arg(clientVer);
				g_Au.Log(logmsg);
			}
		}
		//qDebug("DtMailServer::ReadPacket - P_Message - done");
		break;

	case P_Request:	// Client wants their mail.
		//qDebug("DtMailServer::ReadPacket - P_Request");
		{
			// Read client version from packet.
			QString clientVer;
			block >> clientVer;
			if(clientVer == QString(AuUtil::LoginVersion()))
			{
				// Version matches.
				if(g_Au.IsMailWorker())
				{
					// Read player id and login confirmation number from packet.
					quint32 confirmNum;
					AuId idPerson;

					block >> m_idPlayer;
					block >> confirmNum;
					block >> idPerson;							// person whose mail is requested. 0 when only making contact.

					if(!idPerson.IsNull())
					{
						// They want to get mail for a specific person id.
						AuId idUnit[6];
						if(auMailServerApp->IsLoggedIn(m_idPlayer, confirmNum, idUnit))
						{
							bool idok = false;
							for(int iu=0; iu<6; iu++)
								if(idUnit[iu] == idPerson)
								{
									idok = true;
									break;
								}

							// Send all messages addressed to idPerson to the client.
							if(idok)
							{
								// User id and confirmation number must have matched.
								// Determine what they need.
								quint32 maxMsgId;
								block >> maxMsgId;	// max message id already cached by client

								AuIdList SquelchIds;
								SquelchIds << block;

								// Get their auto delete settings.
								quint8 cityRptAutoDel;
								quint32 cityRptOldAge;
								block >> cityRptAutoDel;
								if(cityRptAutoDel)
									block >> cityRptOldAge;
								else
									cityRptOldAge = 0;

								quint8 fleetRptAutoDel;
								quint32 fleetRptOldAge;
								block >> fleetRptAutoDel;
								if(fleetRptAutoDel)
									block >> fleetRptOldAge;
								else
									fleetRptOldAge = 0;

								quint8 officerRptAutoDel;
								quint32 officerRptOldAge;
								block >> officerRptAutoDel;
								if(officerRptAutoDel)
									block >> officerRptOldAge;
								else
									officerRptOldAge = 0;

								emit RequestMailMessages(m_idPlayer, idPerson, maxMsgId, SquelchIds, cityRptAutoDel, cityRptOldAge,
										fleetRptAutoDel, fleetRptOldAge, officerRptAutoDel, officerRptOldAge);
							}
							else
							{
								logmsg = QObject::tr("%1 ERROR! Request received with invalid unit id %4 "
										"player %2 confirm %3. Hacker?").arg((unsigned long)currentThreadId())
										.arg(m_idPlayer).arg(confirmNum).arg(idPerson.GetID());
								g_Au.Log(logmsg);
							}
						}
						else
						{
							logmsg = QObject::tr("%1 ERROR! Request received with invalid login "
									"player %2 confirm %3. Hacker?").arg((unsigned long)currentThreadId())
									.arg(m_idPlayer).arg(confirmNum);
							g_Au.Log(logmsg);

							// Send the client an error packet to terminate the transaction.
							QByteArray ba;
							QDataStream out(&ba, QIODevice::WriteOnly);
							out.setVersion(QDataStream::Qt_4_1);
							out << (quint8)P_LoginError;
							m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
							m_BytesTx += tcpSocket.write(ba);
							tcpSocket.waitForBytesWritten(10000);
						}
					}
					else
					{
						// Client needs a "done" packet to terminate the transaction.
						QByteArray ba;
						QDataStream out(&ba, QIODevice::WriteOnly);
						out.setVersion(QDataStream::Qt_4_1);
						out << (quint8)P_DoneOk;
						m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
						m_BytesTx += tcpSocket.write(ba);
						tcpSocket.waitForBytesWritten(10000);
					}
				}
				else
				{
					// Running as mail master; defer them to another server.
					Defer(tcpSocket);
				}
			}
			else
			{
				// Version doesn't match.
				// Send the client an error packet to terminate the transaction.
				// This is not a problem so it is not logged. The client will need to
				// get an update to their software.
				QByteArray ba;
				QDataStream out(&ba, QIODevice::WriteOnly);
				out.setVersion(QDataStream::Qt_4_1);
				out << (quint8)P_VersionError;
				m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
				m_BytesTx += tcpSocket.write(ba);

				logmsg = QObject::tr("%1 Request received with invalid version '%2'. "
						"VersionError sent to client.").arg((unsigned long)currentThreadId())
						.arg(clientVer);
				g_Au.Log(logmsg);
				tcpSocket.waitForBytesWritten(10000);
			}
		}
		break;

	case P_Delete:	// Client order to delete messages(s).
		//qDebug("DtMailServer::ReadPacket - P_Delete");
		{
			// Read client version from packet.
			QString clientVer;
			block >> clientVer;
			if(clientVer == QString(AuUtil::LoginVersion()))
			{
				// Version matches.
				if(g_Au.IsMailWorker())
				{
					// Read player id and login confirmation number from packet.
					quint32 idPlayer, confirmNum;
					block >> idPlayer;
					block >> confirmNum;
					AuId idUnit[6];
					if(auMailServerApp->IsLoggedIn(idPlayer, confirmNum, idUnit))
					{
						AuId idPerson;
						idPerson << block;	// person whose mail is requested
						bool idok = false;
						for(int iu=0; iu<6; iu++)
							if(idUnit[iu] == idPerson)
							{
								idok = true;
								break;
							}

						if(idok)
						{
							// User id and confirmation number must have matched.
							quint32 numids;
							block >> numids;
							AuMailMsgIdList delids;
							for(unsigned int d=0; d<numids; d++)
							{
								quint32 delid;
								block >> delid;
								delids.append(delid);
							}

							if(!delids.empty())
								emit DeleteMailMessages(idPlayer, idPerson, delids);
						}
						else
						{
							logmsg = QObject::tr("%1 ERROR! Delete received with invalid unit id %4 "
									"player %2 confirm %3. Hacker?").arg((unsigned long)currentThreadId())
									.arg(idPlayer).arg(confirmNum).arg(idPerson.GetID());
							g_Au.Log(logmsg);
						}
					}
					else
					{
						logmsg = QObject::tr("%1 ERROR! Delete received with invalid login "
								"player %2 confirm %3. Hacker?").arg((unsigned long)currentThreadId())
								.arg(idPlayer).arg(confirmNum);
						g_Au.Log(logmsg);
					}
				}
				else
				{
					logmsg = QObject::tr("%1 ERROR! Delete received by Master server. "
							"Hacker?").arg((unsigned long)currentThreadId());
					g_Au.Log(logmsg);
				}
			}
			else
			{
				// Version doesn't match.
				// The delete request is ignored.
				// The client should already know the version is bad due to our reply
				// to the P_Request. This is worth logging. It might indicate a hacker.
				logmsg = QObject::tr("%1 ERROR! Delete received with invalid version '%2'. "
						"Version should have been rejected during the client's initial "
						"request for mail. Hacker?").arg((unsigned long)currentThreadId()).arg(clientVer);
				g_Au.Log(logmsg);
			}
		}
		break;

	case P_DoneOk:
		logmsg = QObject::tr("%1 ERROR! Received mail packet type P_DoneOk size %2. Client "
				"should never send this to the server. Hacker?").arg((unsigned long)currentThreadId()).arg(ba.count());
		g_Au.Log(logmsg);
		m_BadPackets++;
		break;

	case P_LoginError:
		logmsg = QObject::tr("%1 ERROR! Received mail packet type P_LoginError "
				"size %2. Client should never send this to the server. Hacker?")
				.arg((unsigned long)currentThreadId()).arg(ba.count());
		g_Au.Log(logmsg);
		m_BadPackets++;
		break;

	case P_CheckIn:			// Send by MailServer to MailMaster. No reply needed.
		if(!g_Au.IsMailWorker())
		{
			// MailMaster app keeps track of live MailServers.
			QString serverName = tcpSocket.peerAddress().toString();
			if(!serverName.isEmpty())
			{
				DhMailServer srv;
				srv.m_PeerAddr = serverName;
				block >> srv.m_NetName;
				block >> srv.m_NetPort;
				srv.m_CheckIn = g_Au.CurrentUTC();

				QString serverKey = QString("%1:%2").arg(srv.m_PeerAddr).arg(srv.m_NetPort);
				QMap<QString,DhMailServer>& serverMap = auMailServerApp->LockServerMap();
				serverMap.insert(serverKey, srv);
				auMailServerApp->UnlockServerMap();

				logmsg = QObject::tr("%1 CheckIn %2 name %3:%4")
						.arg((unsigned long)currentThreadId()).arg(serverName)
						.arg(srv.m_NetName).arg(srv.m_NetPort);
				g_Au.Log(logmsg);
			}
			else
			{
				logmsg = QObject::tr("%1 ERROR! Received mail packet type "
						"P_CheckIn size %2 but peerAddress is empty. Hacker?")
						.arg((unsigned long)currentThreadId()).arg(ba.count());
				g_Au.Log(logmsg);
				m_BadPackets++;
			}
		}
		else
		{
			// MailServer should not get this message.
			logmsg = QObject::tr("%1 ERROR! Received mail packet type P_CheckIn "
					"size %2. MailServer should never receive this. Hacker?")
					.arg((unsigned long)currentThreadId()).arg(ba.count());
			g_Au.Log(logmsg);
			m_BadPackets++;
		}
		break;

	case P_VersionError:
		logmsg = QObject::tr("%1 ERROR! Received mail packet type P_VersionError "
				"size %2. Client should never send this to the server. Hacker?")
				.arg((unsigned long)currentThreadId()).arg(ba.count());
		g_Au.Log(logmsg);
		m_BadPackets++;
		break;

	default:
		logmsg = QObject::tr("%1 ERROR! Unknown mail packet type %2 size %3. Hacker?")
				.arg((unsigned long)currentThreadId()).arg(type).arg(ba.count());
		g_Au.Log(logmsg);
		m_BadPackets++;
		break;
	}
}

// Defer the caller to a server on our server list.
void DtMailServer::Defer(QTcpSocket& tcpSocket)
{
	// Key value is calculated to defer to a mail server based on the peer
	// address.
	QString peerAddr = tcpSocket.peerAddress().toString();
	unsigned int key = 0;
	unsigned int plen = peerAddr.count();
	if(plen == 1)
		key = peerAddr[0].unicode();
	else if(plen > 1)
		key = (peerAddr[plen-2].unicode()<<16)|peerAddr[plen-1].unicode();

	// Select an address from the array.
	QMap<QString,DhMailServer>& serverMap = auMailServerApp->LockServerMap();
	if(serverMap.count())
	{
		// Select an address from the map.
		QMap<QString,DhMailServer>::const_iterator it = serverMap.cbegin();
		unsigned int server = key % serverMap.size();
		it += server;
		QString toServer;
		if(m_LocalClient)
			toServer = (*it).m_PeerAddr;	// use peer address on LAN
		else
			toServer = (*it).m_NetName;		// use net name on internet
		quint16 toPort = (*it).m_NetPort;
		auMailServerApp->UnlockServerMap();

		// Send the defer notification to the client.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)P_Deferred;
		out << toServer;
		out << toPort;
		m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
		m_BytesTx += tcpSocket.write(ba);
		tcpSocket.waitForBytesWritten(10000);

		QString logmsg;
		logmsg = QObject::tr("%1 Deferred to %2:%3")
				.arg((unsigned long)currentThreadId()).arg(toServer).arg(toPort);
		g_Au.Log(logmsg);
	}
	else
	{
		auMailServerApp->UnlockServerMap();

		// Notify the client of the defer failure.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)P_DeferFailed;
		m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
		m_BytesTx += tcpSocket.write(ba);
		tcpSocket.waitForBytesWritten(10000);

		QString logmsg;
		logmsg = QObject::tr("%1 ERROR - Defer failed: No servers.")
				.arg((unsigned long)currentThreadId());
		g_Au.Log(logmsg);
	}
}

// Used to notify master that this worker is alive and ready for work.
// static
void DtMailServer::CheckIn()
{
	// Connect to the master server.
	QTcpSocket tcpSocket;
	tcpSocket.connectToHost(g_Au.MailMasterName(), AuUtil::MailMasterPort());
	tcpSocket.waitForConnected(10000);
	if(tcpSocket.state() == QAbstractSocket::ConnectedState)
	{
		// The connection attempt succeeded.
		QString netName = g_Au.InternetName();
		quint16 netPort = AuUtil::MailMasterPort();

		// Transmit the probe to the server and wait for a reply.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)P_CheckIn;
		out << netName;
		out << netPort;
		AuUtil::WritePacketSize(tcpSocket, ba.size());
		tcpSocket.write(ba);
		tcpSocket.waitForBytesWritten(10000);

		// Close the tcp connection.
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			tcpSocket.disconnectFromHost();
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.waitForDisconnected(10000);
		}

		QString logmsg;
		logmsg = QObject::tr("Checked in with master server %1.")
				.arg(g_Au.MailMasterName());
		g_Au.Log(logmsg);
	}
	else
	{
		QString logmsg;
		logmsg = QObject::tr("ERROR! Could not check in with master server %1.")
				.arg(g_Au.MailMasterName());
		g_Au.Log(logmsg);

		// Quit now so shutting down the master shuts down the workers too.
		auMailServerApp->quit();
	}
}

void DtMailServer::PlayerMailMessages(quint32 idPlayer, AuMailMsgArray msgs)
{
	if(idPlayer != m_idPlayer)
		return;
	m_MailMsgsLock.lock();
	m_MailMsgsToGo.append(msgs);
	m_MailMsgsReady = true;
	m_MailMsgsLock.unlock();
}

void DtMailServer::TxData(QTcpSocket& tcpSocket)
{
	if(tcpSocket.state() != QAbstractSocket::ConnectedState)
		return;
	m_MailMsgsLock.lock();
	if(!m_MailMsgsReady)
	{
		m_MailMsgsLock.unlock();
		return;
	}

	// Send the messages to the client, if any.
	int presend = m_BytesTx;
	int msgCount = m_MailMsgsToGo.size();
	m_BytesTx += m_MailMsgsToGo.Tx(tcpSocket);
	m_MailMsgsToGo.clear();
	m_MailMsgsReady = false;
	m_MailMsgsLock.unlock();

	// Client needs a "done" packet to terminate the transaction.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_DoneOk;
	m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
	m_BytesTx += tcpSocket.write(ba);
	tcpSocket.waitForBytesWritten(10000);

	QString logmsg = QObject::tr("%1 Request: Player %2 received %3 messages, %4 bytes.")
		.arg((unsigned long)currentThreadId())
		.arg(m_idPlayer)
		.arg(msgCount)
		.arg(m_BytesTx - presend);
	g_Au.Log(logmsg);
}
