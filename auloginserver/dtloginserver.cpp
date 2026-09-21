//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtloginserver.cpp,v 1.90 2022/04/07 22:42:44 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dtloginserver.h"
//
#include "audef.h"
#include "aucrypt.h"
#include "audnabase.h"
#include "auglobal.h"
#include "auloginserverapp.h"
#include "hashpack.h"
#include <QHostAddress>

DtLoginServer::DtLoginServer(unsigned int id, int socketDesc, QObject* parent) : QThread(parent)
{
	m_Id = id;
	m_BadPackets = 0;
	m_BytesRx = 0;
	m_BytesTx = 0;
	m_LocalClient = false;
	m_PacketSize = 0;
	m_PacketSizeBytesLeft = 0;
	m_socketDescriptor = socketDesc;
}

void DtLoginServer::run()
{
	// Creating the socket on the stack automatically associates it with the
	// thread's event loop. This ensures that Qt will not try to deliver events
	// to our socket from the main thread while we are accessing it from
	// DtLoginServer::run().
	QTcpSocket tcpSocket;
	if(!tcpSocket.setSocketDescriptor(m_socketDescriptor))
	{
		emit error(tcpSocket.error());
		return;
	}
	//connect(&tcpSocket, SIGNAL(readyRead()), SLOT(RxData()));
	//connect(&tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
			//SLOT(NetworkError(QAbstractSocket::SocketError)));

	//qDebug("DtLoginServer::run - Connection established.");
	QString peerAddr = tcpSocket.peerAddress().toString();
	if(peerAddr.left(8) == "192.168." || peerAddr == "127.0.0.1" || peerAddr.left(15) == "::ffff:192.168.")
		m_LocalClient = true;	// This is cheesy but it works for my purposes.
	QString logmsg;
	logmsg = QObject::tr("%1 Incoming connection from %2.").arg((unsigned long)currentThreadId()).arg(peerAddr);
	//logmsg.sprintf("%x Incoming connection from %s.",
	//		(unsigned int)currentThreadId(), (const char*)peerAddr.toAscii());
	g_Au.Log(logmsg);

	// Allow the connection to persist for up to 30 seconds.
	m_PacketSize = 0;
	m_PacketSizeBytesLeft = 0;
	bool done = false;
	bool packetRead = false;
	QDateTime tmNow = g_Au.CurrentUTC();
	while(!done && tmNow.secsTo(g_Au.CurrentUTC()) < 30)
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
			{
				//logmsg = QString("bytes available %1 less than packet size %2.").arg(tcpSocket.bytesAvailable()).arg(m_PacketSize);
				//g_Au.Log(logmsg);
				break;
			}

			// It is now safe to read our data packet from the socket buffer.
			QByteArray block = tcpSocket.read(m_PacketSize);
			m_BytesRx += m_PacketSize;
			m_PacketSize = 0;
			ReadPacket(block, tcpSocket);	// Read the contents of the packet.
			packetRead = true;
		}

		// Check for any operations that have been resolved by the database
		// thread for me.
		bool anyUnresolved = false;	// true if any of my ops are unresolved
		if(g_Au.IsLoginWorker())
		{
			// Only the login worker manipulated the database data.
			auLoginServerApp->m_LoginMutex.lock();

			// Answer login queries.
			for(QVector<AuLoginQuery>::iterator it = auLoginServerApp->m_LoginQueries.begin();
					it != auLoginServerApp->m_LoginQueries.end();)
			{
				if((*it).m_ServerId == m_Id)
				{
					// The operation belongs to this server thread.
					if(!(*it).IsResolved())
					{
						// Skip unresolved operations.
						anyUnresolved = true;
						it++;
					}
					else
					{
						// Transmit the result of the operation.
						QByteArray ba;
						QDataStream out(&ba, QIODevice::WriteOnly);
						out.setVersion(QDataStream::Qt_4_1);
						if((*it).m_IsUnitQuery)
						{
							out << (quint8)P_LoginQueryUnit;
							out << (quint8)(*it).IsLoggedIn();
							for(int iu=0; iu<6; iu++)
								(*it).m_UnitId[iu] >> out;
							out << (quint8)(*it).m_AccountType;
							out << (*it).m_AccountExpiry;
						}
						else
						{
							out << (quint8)P_LoginQuery;
							out << (quint8)(*it).IsLoggedIn();
						}
						m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
						m_BytesTx += tcpSocket.write(ba);
						tcpSocket.waitForBytesWritten();

						if((*it).m_IsUnitQuery)
						{
							logmsg = QObject::tr("%1 LoginQueryUnit player %2 confirm %3 -> %4. Units %5, %6, %7, %8, %9, %10")
									.arg((unsigned long)currentThreadId())
									.arg((*it).m_PlayerId).arg((*it).m_ConfirmNum)
									.arg((*it).IsLoggedIn() ? 'Y' : 'N')
									.arg((*it).m_UnitId[0].GetID())
									.arg((*it).m_UnitId[1].GetID())
									.arg((*it).m_UnitId[2].GetID())
									.arg((*it).m_UnitId[3].GetID())
									.arg((*it).m_UnitId[4].GetID())
									.arg((*it).m_UnitId[5].GetID());
							//logmsg.sprintf("%x LoginQueryUnit player %d "
							//		"confirm %d -> %c unit %d",
							//		(unsigned int)currentThreadId(),
							//		(*it).m_PlayerId, (*it).m_ConfirmNum,
							//		(*it).IsLoggedIn() ? 'Y' : 'N',
							//		(*it).m_UnitId.GetID());
						}
						else
						{
							logmsg = QObject::tr("%1 LoginQuery player %2 "
									"confirm %3 -> %4").arg((unsigned long)currentThreadId())
									.arg((*it).m_PlayerId).arg((*it).m_ConfirmNum)
									.arg((*it).IsLoggedIn() ? 'Y' : 'N');
							//logmsg.sprintf("%x LoginQuery player %d "
							//		"confirm %d -> %c",
							//		(unsigned int)currentThreadId(),
							//		(*it).m_PlayerId, (*it).m_ConfirmNum,
							//		(*it).IsLoggedIn() ? 'Y' : 'N');
						}
						g_Au.Log(logmsg);

						// Remove the operation from the queue.
						it = auLoginServerApp->m_LoginQueries.erase(it);
					}
				}
				else
				{
					// Skip operations that belong to other threads.
					it++;
				}
			}

			// Answer unit queries.
			for(QVector<AuUnitQuery>::iterator it = auLoginServerApp->m_UnitQueries.begin();
					it != auLoginServerApp->m_UnitQueries.end();)
			{
				if((*it).m_ServerId == m_Id)
				{
					// The operation belongs to this server thread.
					if(!(*it).IsResolved())
					{
						// Skip unresolved operations.
						anyUnresolved = true;
						it++;
					}
					else
					{
						// Transmit the result of the operation.
						QByteArray ba;
						QDataStream out(&ba, QIODevice::WriteOnly);
						out.setVersion(QDataStream::Qt_4_1);
						out << (quint8)P_UnitQuery;
						for(int iu=0; iu<6; iu++)
							out << (*it).m_UnitId[iu];
						m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
						m_BytesTx += tcpSocket.write(ba);
						tcpSocket.waitForBytesWritten();

						logmsg = QObject::tr("%1 UnitQuery %2 units %5, %6, %7, %8, %9, %10")
								.arg((unsigned long)currentThreadId())
								.arg((*it).m_QueryUnit.GetID())
								.arg((*it).m_UnitId[0].GetID())
								.arg((*it).m_UnitId[1].GetID())
								.arg((*it).m_UnitId[2].GetID())
								.arg((*it).m_UnitId[3].GetID())
								.arg((*it).m_UnitId[4].GetID())
								.arg((*it).m_UnitId[5].GetID());
						g_Au.Log(logmsg);

						// Remove the operation from the queue.
						it = auLoginServerApp->m_UnitQueries.erase(it);
					}
				}
				else
				{
					// Skip operations that belong to other threads.
					it++;
				}
			}

			// Answer avatar queries.
			for(QVector<AuQueryAvatars>::iterator it = auLoginServerApp->m_QueryAvatars.begin();
					it != auLoginServerApp->m_QueryAvatars.end();)
			{
				if((*it).m_ServerId == m_Id)
				{
					// The operation belongs to this server thread.
					if(!(*it).IsResolved())
					{
						// Skip unresolved operations.
						anyUnresolved = true;
						it++;
					}
					else
					{
						// Transmit the result of the operation in chunks so we don't overrun a 64k packet.
						int idsPerPacket = 0xf000 / sizeof(AuId);	// Stay safely under 64k. We'll probably never hit it anyway.

						// Send the active ids.
						{
							int numPkts = ((*it).m_ActiveIds.count() + idsPerPacket - 1) / idsPerPacket;	// # needed for all ids
							int pktIndex = 0;	// Index into m_ActiveIds at the beginning of each packet.
							for(int pkt=0; pkt<numPkts; pkt++, pktIndex += idsPerPacket)
							{
								int nextI = pktIndex + idsPerPacket;	// Index to next packet if we filled this one up.
								int idsInThisPacket;
								if(nextI <= (*it).m_ActiveIds.count())
									idsInThisPacket = idsPerPacket;							// Full packet.
								else
									idsInThisPacket = (*it).m_ActiveIds.count() - pktIndex;	// Final packet is partially full.
								
								// Fill a packet with ids and send it.
								QByteArray ba;
								QDataStream out(&ba, QIODevice::WriteOnly);
								out.setVersion(QDataStream::Qt_4_1);
								out << (quint8)P_QueryAvatars;
								out << (quint16)idsInThisPacket;
								for(int iu = 0; iu < idsInThisPacket; iu++)
									out << (*it).m_ActiveIds[pktIndex + iu];
								m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
								m_BytesTx += tcpSocket.write(ba);
								tcpSocket.waitForBytesWritten();
							}
						}

						// Send the inactive ids.
						{
							int numPkts = ((*it).m_InactiveIds.count() + idsPerPacket - 1) / idsPerPacket;  // # needed for all ids
							int pktIndex = 0;   // Index into m_InactiveIds at the beginning of each packet.
							for(int pkt=0; pkt<numPkts; pkt++, pktIndex += idsPerPacket)
							{
								int nextI = pktIndex + idsPerPacket;    // Index to next packet if we filled this one up.
								int idsInThisPacket;
								if(nextI <= (*it).m_InactiveIds.count())
									idsInThisPacket = idsPerPacket;                         // Full packet.
								else
									idsInThisPacket = (*it).m_InactiveIds.count() - pktIndex;   // Final packet is partially full.

								// Fill a packet with ids and send it.
								QByteArray ba;
								QDataStream out(&ba, QIODevice::WriteOnly);
								out.setVersion(QDataStream::Qt_4_1);
								out << (quint8)P_InactiveUnitQuery;
								out << (quint16)idsInThisPacket;
								for(int iu = 0; iu < idsInThisPacket; iu++)
									out << (*it).m_InactiveIds[pktIndex + iu];
								m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
								m_BytesTx += tcpSocket.write(ba);
								tcpSocket.waitForBytesWritten();
							}
						}

						logmsg = QObject::tr("%1 QueryAvatars %2 active and %3 inactive avatars found")
								.arg((unsigned long)currentThreadId())
								.arg((*it).m_ActiveIds.count())
								.arg((*it).m_InactiveIds.count());
						g_Au.Log(logmsg);

						// Remove the operation from the queue.
						it = auLoginServerApp->m_QueryAvatars.erase(it);
					}
				}
				else
				{
					// Skip operations that belong to other threads.
					it++;
				}
			}

			// Answer avatar queries.
			for(QVector<AuQueryAccountAvatars>::iterator it = auLoginServerApp->m_QueryAccountAvatars.begin();
					it != auLoginServerApp->m_QueryAccountAvatars.end();)
			{
				if((*it).m_ServerId == m_Id)
				{
					// The operation belongs to this server thread.
					if(!(*it).IsResolved())
					{
						// Skip unresolved operations.
						anyUnresolved = true;
						it++;
					}
					else
					{
						// Transmit the result of the operation.
						QByteArray ba;
						QDataStream out(&ba, QIODevice::WriteOnly);
						out.setVersion(QDataStream::Qt_4_1);
						out << (quint8)P_QueryAccountAvatars;
						for(int ia=0; ia<6; ia++)
						{
							out << (*it).m_AvatarId[ia];
							out << (*it).m_AvatarName[ia];
						}
						m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
						m_BytesTx += tcpSocket.write(ba);
						tcpSocket.waitForBytesWritten();

						//logmsg = QObject::tr("%1 QueryAccountAvatars %2 active and %3 inactive avatars found")
								//.arg((unsigned long)currentThreadId())
								//.arg((*it).m_ActiveIds.count());
						//g_Au.Log(logmsg);

						// Remove the operation from the queue.
						it = auLoginServerApp->m_QueryAccountAvatars.erase(it);
					}
				}
				else
				{
					// Skip operations that belong to other threads.
					it++;
				}
			}

			// Answer attempts to create new player accounts.
			for(QVector<AuNewPlayer>::iterator it = auLoginServerApp->m_NewPlayers.begin();
					it != auLoginServerApp->m_NewPlayers.end();)
			{
				if((*it).m_ServerId == m_Id)
				{
					// The operation belongs to this server thread.
					if(!(*it).IsResolved())
					{
						// Skip unresolved operations.
						anyUnresolved = true;
						it++;
					}
					else
					{
						// Transmit the result of the operation.
						QByteArray ba;
						QDataStream out(&ba, QIODevice::WriteOnly);
						out.setVersion(QDataStream::Qt_4_1);
						out << (quint8)P_AccountNew;
						out << (quint8)(*it).m_Result;
						out << (*it).m_AcctId;
						m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
						m_BytesTx += tcpSocket.write(ba);
						tcpSocket.waitForBytesWritten();

						// Log the result.
						const char* resultStr;
						switch((*it).m_Result)
						{
						case AuNewPlayer::Success:
							resultStr = "Ok";
							break;
						case AuNewPlayer::BadBlankName:
							resultStr = "ERROR! Blank Name";
							break;
						case AuNewPlayer::BadBlankPassword:
							resultStr = "ERROR! Blank Password";
							break;
						case AuNewPlayer::BadBlankEmail:
							resultStr = "ERROR! Blank Email Address";
							break;
						case AuNewPlayer::BadEmail:
							resultStr = "ERROR! Bogus Email Address";
							break;
						case AuNewPlayer::BadAccountExists:
							resultStr = "ERROR! Account Already Exists";
							break;
						case AuNewPlayer::BadAccountExistsPending:
							resultStr = "ERROR! Account Exists Pending Database Insert";
							break;
						case AuNewPlayer::BadDbErr:
							resultStr = "ERROR! Database Error";
							break;
						default:
							resultStr = "ERROR! UNDEFINED RESULT";
							break;
						}
						QString logmsg;
						logmsg = QObject::tr("%1 CreateAccount name '%2' email '%3' "
								"result %4").arg((unsigned long)currentThreadId()).arg((*it).m_Name)
								.arg((*it).m_Email).arg(resultStr);
						g_Au.Log(logmsg);

						// Remove the operation from the queue.
						it = auLoginServerApp->m_NewPlayers.erase(it);
					}
				}
				else
				{
					// Skip operations that belong to other threads.
					it++;
				}
			}

			// Answer attempts to login.
			for(QVector<AuLoginAttempt>::iterator it = auLoginServerApp->m_LoginAttempts.begin();
					it != auLoginServerApp->m_LoginAttempts.end();)
			{
				if((*it).m_ServerId == m_Id)
				{
					// The operation belongs to this server thread.
					if(!(*it).IsResolved())
					{
						// Skip unresolved operations.
						anyUnresolved = true;
						it++;
					}
					else
					{
						// Transmit the result of the operation.
						if((*it).m_Result == AuLoginAttempt::Success)
						{
							QByteArray ba;
							QDataStream out(&ba, QIODevice::WriteOnly);
							out.setVersion(QDataStream::Qt_4_1);
							if((*it).m_TestCredentialsOnly)
							{
								out << (quint8)P_LoginTest;
								out << (*it).m_ConfirmNum;
								out << (*it).m_PlayerId;
							}
							else
							{
								out << (quint8)P_Login4;
								out << (*it).m_ConfirmNum;
								out << (*it).m_PlayerId;
								out << (quint8)(*it).m_AccountType;
								for(int iu=0; iu<6; iu++)
								{
									(*it).m_UnitId[iu] >> out;
									out << (*it).m_UnitName[iu];

									if((*it).m_UnitDNA[iu])
										(*it).m_UnitDNA[iu]->WriteDynamic(out);
									else
										out << (quint8)0;

									out << (quint8)(*it).m_UnitGender[iu];
									out << (quint8)(*it).m_UnitHand[iu];
									out << (quint8)(*it).m_UnitHair[iu];
									out << (quint8)(*it).m_UnitShave[iu];
									out << (quint16)(*it).m_UnitClothes[iu];
									out << (quint16)(*it).m_UnitArmor[iu];
									out << (quint16)(*it).m_UnitHat[iu];
									out << (quint8)(*it).m_UnitVisor[iu];
								}
								out << (*it).m_AccountExpiry;
							}
							m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
							m_BytesTx += tcpSocket.write(ba);
							tcpSocket.waitForBytesWritten();

							QString logmsg;
							logmsg = QObject::tr("%1 Login %2 %3 confirm %4 type %5. "
									"Units %6 %7, %8 %9, %10 %11, %12 %13, %14 %15, %16 %17")
									.arg((unsigned long)currentThreadId())
									.arg((*it).m_PlayerId)
									.arg((*it).m_Name)
									.arg((*it).m_ConfirmNum)
									.arg((*it).AccountTypeName())
									.arg((*it).m_UnitId[0].GetID()).arg((*it).m_UnitName[0])
									.arg((*it).m_UnitId[1].GetID()).arg((*it).m_UnitName[1])
									.arg((*it).m_UnitId[2].GetID()).arg((*it).m_UnitName[2])
									.arg((*it).m_UnitId[3].GetID()).arg((*it).m_UnitName[3])
									.arg((*it).m_UnitId[4].GetID()).arg((*it).m_UnitName[4])
									.arg((*it).m_UnitId[5].GetID()).arg((*it).m_UnitName[5]);
							g_Au.Log(logmsg);
						}
						else if((*it).m_Result == AuLoginAttempt::LockedOut)
						{
							// Send a message to the client indicating the
							// failure of the login attempt.
							QByteArray ba;
							QDataStream out(&ba, QIODevice::WriteOnly);
							out.setVersion(QDataStream::Qt_4_1);
							out << (quint8)P_LoginLockedOut;
							m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
							m_BytesTx += tcpSocket.write(ba);
							tcpSocket.waitForBytesWritten();

							QString logmsg;
							logmsg = QObject::tr("ERROR - Login disabled: name '%2'").arg((*it).m_Name);
							g_Au.Log(logmsg);
						}
						else
						{
							// Send a message to the client indicating the failure of the login attempt.
							QByteArray ba;
							QDataStream out(&ba, QIODevice::WriteOnly);
							out.setVersion(QDataStream::Qt_4_1);
							out << (quint8)P_LoginFailure;
							m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
							m_BytesTx += tcpSocket.write(ba);
							tcpSocket.waitForBytesWritten();

							QString logmsg;
							logmsg = QObject::tr("ERROR - Login failed: name '%2'").arg((*it).m_Name);
							g_Au.Log(logmsg);
						}

						// Remove the operation from the queue.
						it = auLoginServerApp->m_LoginAttempts.erase(it);
					}
				}
				else
				{
					// Skip operations that belong to other threads.
					it++;
				}
			}

#if defined(AUSTEAM)
			// Answer attempts to login using a steam ticket.
			for(QVector<AuLoginSteamAttempt>::iterator it = auLoginServerApp->m_LoginSteamAttempts.begin();
					it != auLoginServerApp->m_LoginSteamAttempts.end();)
			{
				if((*it).m_ServerId == m_Id)
				{
					// The operation belongs to this server thread.
					if(!(*it).IsResolved())
					{
						// Skip unresolved operations.
						anyUnresolved = true;
						it++;
					}
					else
					{
						// Transmit the result of the operation.
						if((*it).m_Result == AuLoginSteamAttempt::Success)
						{
							QByteArray ba;
							QDataStream out(&ba, QIODevice::WriteOnly);
							out.setVersion(QDataStream::Qt_4_1);
							out << (quint8)P_Login4;
							out << (*it).m_ConfirmNum;
							out << (*it).m_PlayerId;
							out << (quint8)(*it).m_AccountType;
							for(int iu=0; iu<6; iu++)
							{
								(*it).m_UnitId[iu] >> out;
								out << (*it).m_UnitName[iu];

								if((*it).m_UnitDNA[iu])
									(*it).m_UnitDNA[iu]->WriteDynamic(out);
								else
									out << (quint8)0;

								out << (quint8)(*it).m_UnitGender[iu];
								out << (quint8)(*it).m_UnitHand[iu];
								out << (quint8)(*it).m_UnitHair[iu];
								out << (quint8)(*it).m_UnitShave[iu];
								out << (quint16)(*it).m_UnitClothes[iu];
								out << (quint16)(*it).m_UnitArmor[iu];
								out << (quint16)(*it).m_UnitHat[iu];
								out << (quint8)(*it).m_UnitVisor[iu];
							}
							out << (*it).m_AccountExpiry;
							m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
							m_BytesTx += tcpSocket.write(ba);
							tcpSocket.waitForBytesWritten();

							QString logmsg;
							logmsg = QObject::tr("%1 LoginSteam %2 confirm %4 type %5. "
									"Units %6 %7, %8 %9, %10 %11, %12 %13, %14 %15, %16 %17")
									.arg((unsigned long)currentThreadId())
									.arg((*it).m_PlayerId)
									.arg((*it).m_ConfirmNum)
									.arg((*it).AccountTypeName())
									.arg((*it).m_UnitId[0].GetID()).arg((*it).m_UnitName[0])
									.arg((*it).m_UnitId[1].GetID()).arg((*it).m_UnitName[1])
									.arg((*it).m_UnitId[2].GetID()).arg((*it).m_UnitName[2])
									.arg((*it).m_UnitId[3].GetID()).arg((*it).m_UnitName[3])
									.arg((*it).m_UnitId[4].GetID()).arg((*it).m_UnitName[4])
									.arg((*it).m_UnitId[5].GetID()).arg((*it).m_UnitName[5]);
							g_Au.Log(logmsg);
						}
						else
						{
							// Send a message to the client indicating the failure of the login attempt.
							QByteArray ba;
							QDataStream out(&ba, QIODevice::WriteOnly);
							out.setVersion(QDataStream::Qt_4_1);
							out << (quint8)P_LoginFailure;
							m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
							m_BytesTx += tcpSocket.write(ba);
							tcpSocket.waitForBytesWritten();

							QString logmsg;
							logmsg = QObject::tr("ERROR - LoginSteam failed");
							g_Au.Log(logmsg);
						}

						// Remove the operation from the queue.
						it = auLoginServerApp->m_LoginSteamAttempts.erase(it);
					}
				}
				else
				{
					// Skip operations that belong to other threads.
					it++;
				}
			}
#endif	// AUSTEAM
			auLoginServerApp->m_LoginMutex.unlock();

			// Answer requests for Contacts list.
			auLoginServerApp->m_LoginContactsMutex.lock();
			for(QVector<AuLoginContacts>::iterator it = auLoginServerApp->m_ContactsQueries.begin();
					it != auLoginServerApp->m_ContactsQueries.end();)
			{
				if((*it).m_ServerId == m_Id)
				{
					// The operation belongs to this server thread.
					if(!(*it).IsResolved())
					{
						// Skip unresolved operations.
						anyUnresolved = true;
						it++;
					}
					else
					{
						// Transmit the result of the operation.
						//  Contacts List
						QByteArray ba;
						QDataStream out(&ba, QIODevice::WriteOnly);
						out.setVersion(QDataStream::Qt_4_1);
						out << (quint8)P_ContactsQuery;
			
						// Compress the list
						{
							QByteArray bac;
							QDataStream ds(&bac, QIODevice::WriteOnly);
							ds.setVersion(QDataStream::Qt_4_1);
							quint16 count = (*it).m_Contacts.count();
							ds << count;
							for(QList<AuContact>::const_iterator cit = (*it).m_Contacts.constBegin();
									cit != (*it).m_Contacts.constEnd(); cit++)
								(*cit) >> ds;
							QByteArray bcomp = qCompress(bac);
							out << bcomp;
						}
						m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
						m_BytesTx += tcpSocket.write(ba);
						tcpSocket.waitForBytesWritten();

						// Remove the operation from the queue.
						it = auLoginServerApp->m_ContactsQueries.erase(it);
					}
				}
				else
				{
					// Skip operations that belong to other threads.
					it++;
				}
			}
			auLoginServerApp->m_LoginContactsMutex.unlock();
		}
		if(!anyUnresolved && packetRead)
			done = true;

		// Wait for more data.
		if(!packetRead && tcpSocket.state() == QAbstractSocket::ConnectedState)
			tcpSocket.waitForReadyRead(1000);
	}

	// Close the socket.
	if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		tcpSocket.disconnectFromHost();
	if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		tcpSocket.waitForDisconnected();

	if(g_Au.IsLoginWorker())
	{
		// Remove any remaining db operations that belong to me.
		// We only remove the ones that require a response. Others can be
		// processed after this thread is gone.
		auLoginServerApp->m_LoginMutex.lock();
		for(QVector<AuLoginQuery>::iterator it = auLoginServerApp->m_LoginQueries.begin();
				it != auLoginServerApp->m_LoginQueries.end();)
		{
			if((*it).m_ServerId == m_Id)
				it = auLoginServerApp->m_LoginQueries.erase(it);
			else
				it++;
		}
		for(QVector<AuNewPlayer>::iterator it = auLoginServerApp->m_NewPlayers.begin();
				it != auLoginServerApp->m_NewPlayers.end();)
		{
			if((*it).m_ServerId == m_Id)
				it = auLoginServerApp->m_NewPlayers.erase(it);
			else
				it++;
		}
		for(QVector<AuLoginAttempt>::iterator it = auLoginServerApp->m_LoginAttempts.begin();
				it != auLoginServerApp->m_LoginAttempts.end();)
		{
			if((*it).m_ServerId == m_Id)
				it = auLoginServerApp->m_LoginAttempts.erase(it);
			else
				it++;
		}
#if defined(AUSTEAM)
		for(QVector<AuLoginSteamAttempt>::iterator it = auLoginServerApp->m_LoginSteamAttempts.begin();
				it != auLoginServerApp->m_LoginSteamAttempts.end();)
		{
			if((*it).m_ServerId == m_Id)
				it = auLoginServerApp->m_LoginSteamAttempts.erase(it);
			else
				it++;
		}
#endif	// AUSTEAM
		auLoginServerApp->m_LoginMutex.unlock();

		auLoginServerApp->m_LoginContactsMutex.lock();
		for(QVector<AuLoginContacts>::iterator it = auLoginServerApp->m_ContactsQueries.begin();
				it != auLoginServerApp->m_ContactsQueries.end();)
		{
			if((*it).m_ServerId == m_Id)
				it = auLoginServerApp->m_ContactsQueries.erase(it);
			else
				it++;
		}
		auLoginServerApp->m_LoginContactsMutex.unlock();
	}

	if(m_PacketSize || !done)
		logmsg = QObject::tr("%1 ERROR! Connection with %2 terminated unexpectedly. "
				"pktSize %3 done %4").arg((unsigned long)currentThreadId()).arg(peerAddr)
				.arg(m_PacketSize).arg(done?'Y':'N');
	else
		logmsg = QObject::tr("%1 Connection with %2 terminated.")
				.arg((unsigned long)currentThreadId()).arg(peerAddr);
	g_Au.Log(logmsg);
}

void DtLoginServer::ReadPacket(const QByteArray& ba, QTcpSocket& tcpSocket)
{
	//qDebug("DtLoginServer::ReadPacket - %d bytes rx'd", ba.count());
	QDataStream block(ba);
	block.setVersion(QDataStream::Qt_4_1);

	// The first byte is always the packet type.
	QString logmsg;
	unsigned char type;
	block >> type;
	//logmsg = QString("ReadPacket type %1 size %2").arg(type).arg(ba.size());
	//g_Au.Log(logmsg);
	switch((LOGIN_PACKET)type)
	{
	case P_Login4:		// Login attempt.
	case P_LoginTest:	// Login test for name and password validity.
		// Client sent us their login name, password and software version.
		{
			QString uver;
			AuCrypt unameE;
			QByteArray upass;
			block >> uver;
			block >> unameE;
			block >> upass;
			QString uname = unameE.DecryptString(AuUtil::NetCryptKey(0x0101));
			if(uver == QString(AuUtil::LoginVersion()))
			{
				// Software version checks out.
				if(g_Au.IsLoginWorker())
				{
					// Running as login server; process the login request.
					QString serverName = tcpSocket.peerAddress().toString();
					AuLoginAttempt login(m_Id, serverName, uname, upass, (type == P_LoginTest)?true:false);
					auLoginServerApp->m_LoginMutex.lock();
					auLoginServerApp->m_LoginAttempts.append(login);
					auLoginServerApp->m_LoginMutex.unlock();
				}
				else
				{
					// Running as login master; defer them to another server.
					Defer(tcpSocket);
				}
			}
			else
			{
				QByteArray ba;
				QDataStream out(&ba, QIODevice::WriteOnly);
				out.setVersion(QDataStream::Qt_4_1);
				out << (quint8)P_VersionError;
				m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
				m_BytesTx += tcpSocket.write(ba);
				tcpSocket.waitForBytesWritten();

				logmsg = QObject::tr("%1 ERROR - Version mismatch: Server '%2', "
						"Client '%3' Login account name '%4'").arg((unsigned long)currentThreadId())
						.arg(AuUtil::LoginVersion()).arg(uver).arg(uname);
				g_Au.Log(logmsg);
			}
		}
		break;

	case P_LoginSteam:		// Login attempt using steam ticket.
	case P_LoginSteamTest:
#if defined(AUSTEAM)
		// Client sent us their encrypted steam ticket and software version.
		{
			QString uver;
			AuCrypt ticket;
			block >> uver;
			block >> ticket;
			if(uver == QString(AuUtil::LoginVersion()))
			{
				// Software version checks out.
				if(g_Au.IsLoginWorker())
				{
					// Running as login server; process the login request.
					ticket.Crypt(AuUtil::NetCryptKey(0x0101));	// Decrypt the ticket.
					QString serverName = tcpSocket.peerAddress().toString();
					AuLoginSteamAttempt login(m_Id, serverName, ticket, (type == P_LoginSteamTest)?true:false);
					auLoginServerApp->m_LoginMutex.lock();
					auLoginServerApp->m_LoginSteamAttempts.append(login);
					auLoginServerApp->m_LoginMutex.unlock();
				}
				else
				{
					// Running as login master; defer them to another server.
					Defer(tcpSocket);
				}
			}
			else
			{
				QByteArray ba;
				QDataStream out(&ba, QIODevice::WriteOnly);
				out.setVersion(QDataStream::Qt_4_1);
				out << (quint8)P_VersionError;
				m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
				m_BytesTx += tcpSocket.write(ba);
				tcpSocket.waitForBytesWritten();

				logmsg = QObject::tr("%1 ERROR - Version mismatch: Server '%2', "
						"Client '%3' Login steam key length '%4'").arg((unsigned long)currentThreadId())
						.arg(AuUtil::LoginVersion()).arg(uver).arg(ticket.size());
				g_Au.Log(logmsg);
			}
		}
#endif	// AUSTEAM
		break;

	case P_LoginQuery:	// Sent to server to validate login. Sent back to reply
		{
			quint32 idPlayer;
			block >> idPlayer;
			if(g_Au.IsLoginWorker())
			{
				quint32 confirmNum;
				block >> confirmNum;

				QString serverName = tcpSocket.peerAddress().toString();
				AuLoginQuery query(m_Id, idPlayer, confirmNum, false, serverName);
				auLoginServerApp->m_LoginMutex.lock();
				auLoginServerApp->m_LoginQueries.append(query);
				auLoginServerApp->m_LoginMutex.unlock();
			}
			else
			{
				Defer(tcpSocket);
			}
		}
		break;

	case P_LoginQueryUnit:	// Sent to server to validate login. Sent back to
		{					// reply. Sent periodically by scene clients and
			quint32 idPlayer;	// servers to insure status has not changed.
			block >> idPlayer;
			if(g_Au.IsLoginWorker())
			{
				quint32 confirmNum;
				block >> confirmNum;

				QString serverName = tcpSocket.peerAddress().toString();
				AuLoginQuery query(m_Id, idPlayer, confirmNum, true, serverName);
				auLoginServerApp->m_LoginMutex.lock();
				auLoginServerApp->m_LoginQueries.append(query);
				auLoginServerApp->m_LoginMutex.unlock();
			}
			else
			{
				Defer(tcpSocket);
			}
		}
		break;

	case P_Logout:			// Sent by client to server.
		if(g_Au.IsLoginWorker())
		{
			quint32 idPlayer;
			block >> idPlayer;
			quint32 confirmNum;
			block >> confirmNum;

			QString serverName = tcpSocket.peerAddress().toString();
			AuLogoutAttempt logout(idPlayer, confirmNum, serverName);
			auLoginServerApp->m_LoginMutex.lock();
			auLoginServerApp->m_LogoutAttempts.append(logout);
			auLoginServerApp->m_LoginMutex.unlock();

			// Ack
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)P_Ok;
			m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
			m_BytesTx += tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten();
		}
		else
		{
			// Running as login master; defer them to another server.
			Defer(tcpSocket);
		}
		break;

	case P_DeferFailed:	// Server reply to Login - defer failed.
		logmsg = QObject::tr("%1 HACKER WARNING! Received TCP packet type "
				"P_DeferFailed size %2. "
				"Client should never send this to the server.")
				.arg((unsigned long)currentThreadId()).arg(ba.count());
		g_Au.Log(logmsg);
		m_BadPackets++;
		break;

	case P_Deferred:	// Server reply to Login - login deferred.
		logmsg = QObject::tr("%1 HACKER WARNING! Received TCP packet type "
				"P_Deferred size %2. "
				"Client should never send this to the server.")
				.arg((unsigned long)currentThreadId()).arg(ba.count());
		g_Au.Log(logmsg);
		m_BadPackets++;
		break;

	case P_LoginLockedOut:	// Server reply to Login - login disabled.
		logmsg = QObject::tr("%1 HACKER WARNING! Received TCP packet type "
				"P_LoginLockedOut size %2. "
				"Client should never send this to the server.")
				.arg((unsigned long)currentThreadId()).arg(ba.count());
		g_Au.Log(logmsg);
		m_BadPackets++;
		break;

	case P_LoginFailure:	// Server reply to Login - login failure.
		logmsg = QObject::tr("%1 HACKER WARNING! Received TCP packet type "
				"P_LoginFailure size %2. "
				"Client should never send this to the server.")
				.arg((unsigned long)currentThreadId()).arg(ba.count());
		g_Au.Log(logmsg);
		m_BadPackets++;
		break;

	case P_StatusQuery:
		// Sent by AuServerStatus to get master's current status. An immediate reply is expected.
		{
			// Collect status values.
			quint8 status = 0;	// Status type.
			QDateTime tmUp = auLoginServerApp->TimeUp();
			quint8 workers = auLoginServerApp->WorkerCount();

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

	case P_ContactsQuery: // Asking for the complete list of contacts for this player
		{
			AuCrypt unameE;
			QByteArray upass;
			block >> unameE;
			block >> upass;
			if(g_Au.IsLoginWorker())
			{
				// Running as login server; process the contacts request.
				QString uname = unameE.DecryptString(AuUtil::NetCryptKey(0x0202));
				QString serverName = tcpSocket.peerAddress().toString();
				AuLoginContacts logincontacts(m_Id, serverName, uname, upass);
				auLoginServerApp->m_LoginContactsMutex.lock();
				auLoginServerApp->m_ContactsQueries.append(logincontacts);
				auLoginServerApp->m_LoginContactsMutex.unlock();
			}
			else
			{
				// Running as login master; defer them to another server.
				Defer(tcpSocket);
			}
			
		}
		break;

	case P_ContactsAdd:
		{
			AuCrypt unameE;
			QByteArray upass;
			block >> unameE;
			block >> upass;

			// Get all of the contacts out of the packet that was sent
			QList<AuContact> contacts;
			contacts.clear();// is it necessary to clear a new list?

			QByteArray bcomp;
			block >> bcomp;
			if(!bcomp.isEmpty())
			{
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__, unameE.DecryptString(AuUtil::NetCryptKey(0x0303)));
				QDataStream ds(&ba, QIODevice::ReadOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				quint16 count;
				ds >> count;
				for(;count ;count--)
				{
					AuContact contact;
					contact << ds;
					contacts.append(contact);
				}
			}

			// Software version checks out.
			if(g_Au.IsLoginWorker())
			{
				// Running as login server; process the contacts request.
				QString uname = unameE.DecryptString(AuUtil::NetCryptKey(0x0303));
				QString serverName = tcpSocket.peerAddress().toString();
				AuLoginContacts logincontacts(m_Id, serverName, uname, upass, &contacts);
				auLoginServerApp->m_LoginContactsMutex.lock();
				auLoginServerApp->m_ContactsAdds.append(logincontacts);
				auLoginServerApp->m_LoginContactsMutex.unlock();

				// Ack
				QByteArray ba;
				QDataStream out(&ba, QIODevice::WriteOnly);
				out.setVersion(QDataStream::Qt_4_1);
				out << (quint8)P_Ok;
				m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
				m_BytesTx += tcpSocket.write(ba);
				tcpSocket.waitForBytesWritten();
			}
			else
			{
				// Running as login master; defer them to another server.
				Defer(tcpSocket);
			}
		}
		break;
	
	case P_ContactsDelete:
		{
			AuCrypt unameE;
			QByteArray upass;
			block >> unameE;
			block >> upass;

			// Get all of the contacts out of the packet that was sent
			QList<AuContact> contacts;
			contacts.clear();// is it necessary to clear a new list?

			QByteArray bcomp;
			block >> bcomp;
			if(!bcomp.isEmpty())
			{
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__, unameE.DecryptString(AuUtil::NetCryptKey(0x0303)));
				QDataStream ds(&ba, QIODevice::ReadOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				quint16 count;
				ds >> count;
				for(;count ;count--)
				{
					AuContact contact;
					contact << ds;
					contacts.append(contact);
				}
			}

			// Software version checks out.
			if(g_Au.IsLoginWorker())
			{
				// Running as login server; process the contacts request.
				QString uname = unameE.DecryptString(AuUtil::NetCryptKey(0x0303));
				QString serverName = tcpSocket.peerAddress().toString();
				AuLoginContacts logincontacts(m_Id, serverName, uname, upass, &contacts);
				auLoginServerApp->m_LoginContactsMutex.lock();
				auLoginServerApp->m_ContactsDeletes.append(logincontacts);
				auLoginServerApp->m_LoginContactsMutex.unlock();

				// Ack
				QByteArray ba;
				QDataStream out(&ba, QIODevice::WriteOnly);
				out.setVersion(QDataStream::Qt_4_1);
				out << (quint8)P_Ok;
				m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
				m_BytesTx += tcpSocket.write(ba);
				tcpSocket.waitForBytesWritten();
			}
			else
			{
				// Running as login master; defer them to another server.
				Defer(tcpSocket);
			}
			
		}
		break;
	
	case P_ContactAdd:	// Add a single contact to the player's list of contacts
		{
			AuCrypt unameE;
			QByteArray upass;
			block >> unameE;
			block >> upass;

			AuContact contact;
			contact << block;

			// Software version checks out.
			if(g_Au.IsLoginWorker())
			{
				// Running as login server; process the contacts request.
				QString uname = unameE.DecryptString(AuUtil::NetCryptKey(0x0404));
				QString serverName = tcpSocket.peerAddress().toString();
				AuLoginContacts logincontacts(m_Id, serverName, uname, upass, &contact);
				auLoginServerApp->m_LoginContactsMutex.lock();
				auLoginServerApp->m_ContactAdds.append(logincontacts);
				auLoginServerApp->m_LoginContactsMutex.unlock();
			}
			else
			{
				// Running as login master; defer them to another server.
				Defer(tcpSocket);
			}
			
		}
		break;
	
	case P_ContactDelete:	// Delete a single contact from the player's list of contacts
		{
			AuCrypt unameE;
			QByteArray upass;
			AuId idContact;
			block >> unameE;
			block >> upass;
			block >> idContact;

			// Software version checks out.
			if(g_Au.IsLoginWorker())
			{
				// Running as login server; process the contacts request.
				QString uname = unameE.DecryptString(AuUtil::NetCryptKey(0x0404));
				QString serverName = tcpSocket.peerAddress().toString();
				AuLoginContacts logincontacts(m_Id, serverName, uname, upass, idContact);
				auLoginServerApp->m_LoginContactsMutex.lock();
				auLoginServerApp->m_ContactDeletes.append(logincontacts);
				auLoginServerApp->m_LoginContactsMutex.unlock();
			}
			else
			{
				// Running as login master; defer them to another server.
				Defer(tcpSocket);
			}
		}
		break;
	
	case P_CheckIn:		// Sent by LoginServer to LoginMaster. No reply needed.
		if(!g_Au.IsLoginWorker())
		{
			// LoginMaster app keeps track of live LoginServers.
			QString serverName = tcpSocket.peerAddress().toString();
			if(!serverName.isEmpty())
			{
				DhLoginServer srv;
				srv.m_PeerAddr = serverName;
				block >> srv.m_NetName;
				block >> srv.m_NetPort;
				srv.m_CheckIn = g_Au.CurrentUTC();

				QString serverKey = QString("%1:%2").arg(srv.m_PeerAddr).arg(srv.m_NetPort);
				QMap<QString, DhLoginServer>& serverMap = auLoginServerApp->LockServerMap();
				serverMap.insert(serverKey, srv);
				auLoginServerApp->UnlockServerMap();

				logmsg = QObject::tr("%1 CheckIn peer %2 name %3 port %4")
						.arg((unsigned long)currentThreadId()).arg(serverName)
						.arg(srv.m_NetName).arg(srv.m_NetPort);
				g_Au.Log(logmsg);
			}
			else
			{
				logmsg = QObject::tr("%1 HACKER WARNING! Received packet type "
						"P_CheckIn size %2 but peerAddress is empty.")
						.arg((unsigned long)currentThreadId()).arg(ba.count());
				g_Au.Log(logmsg);
				m_BadPackets++;
			}
		}
		else
		{
			// LoginServer should not get this message.
			logmsg = QObject::tr("%1 HACKER WARNING! Received packet type P_CheckIn "
					"size %2. LoginServer should never receive this.")
					.arg((unsigned long)currentThreadId()).arg(ba.count());
			g_Au.Log(logmsg);
			m_BadPackets++;
		}
		break;

	case P_Unit4:
		if(g_Au.IsLoginWorker())
		{
			quint32 idPlayer;
			quint32 confirmNum;
			AuId idUnit;

			block >> idPlayer;
			block >> confirmNum;
			block >> idUnit;

			QString serverName = tcpSocket.peerAddress().toString();
			AuSetUnit setUnit(serverName, idPlayer, confirmNum, idUnit, false);
			auLoginServerApp->m_LoginMutex.lock();
			auLoginServerApp->m_SetUnit.append(setUnit);
			auLoginServerApp->m_LoginMutex.unlock();

			// Ack
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)P_Ok;
			m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
			m_BytesTx += tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten();
		}
		else
		{
			// Running as login master; defer them to another server.
			Defer(tcpSocket);
		}
		break;

	case P_UnitDelete:
		if(g_Au.IsLoginWorker())
		{
			quint32 idPlayer;
			quint32 confirmNum;
			AuId idUnit;

			block >> idPlayer;
			block >> confirmNum;
			block >> idUnit;

			QString serverName = tcpSocket.peerAddress().toString();
			AuSetUnit setUnit(serverName, idPlayer, confirmNum, idUnit, true);
			auLoginServerApp->m_LoginMutex.lock();
			auLoginServerApp->m_SetUnit.append(setUnit);
			auLoginServerApp->m_LoginMutex.unlock();

			// Ack
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)P_Ok;
			m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
			m_BytesTx += tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten();
		}
		else
		{
			// Running as login master; defer them to another server.
			Defer(tcpSocket);
		}
		break;

	case P_UnitBorn4:
		if(g_Au.IsLoginWorker())
		{
			AuId motherUnit;
			AuId idUnit;

			block >> motherUnit;
			block >> idUnit;

			QString serverName = tcpSocket.peerAddress().toString();
			AuUnitBorn unitBorn(serverName, motherUnit, idUnit);
			auLoginServerApp->m_LoginMutex.lock();
			auLoginServerApp->m_UnitBorn.append(unitBorn);
			auLoginServerApp->m_LoginMutex.unlock();
		}
		else
		{
			logmsg = QObject::tr("%1 HACKER WARNING! Received TCP packet P_UnitBorn "
					"size %2. Client should never send this to the Master "
					"server.").arg((unsigned long)currentThreadId()).arg(ba.count());
			g_Au.Log(logmsg);
			m_BadPackets++;
		}
		break;

	case P_UnitDied:
		if(g_Au.IsLoginWorker())
		{
			AuId idUnit;
			AuId idHaxus;

			block >> idUnit;
			block >> idHaxus;

			if(idHaxus == AuId::Haxus())
			{
				QString serverName = tcpSocket.peerAddress().toString();
				AuUnitDied unitDied(serverName, idUnit);
				auLoginServerApp->m_LoginMutex.lock();
				auLoginServerApp->m_UnitDied.append(unitDied);
				auLoginServerApp->m_LoginMutex.unlock();
			}

			// Ack
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)P_Ok;
			m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
			m_BytesTx += tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten();
		}
		else
		{
			// Running as login master; defer them to another server.
			Defer(tcpSocket);
		}
		break;

	case P_UnitTransfer:
		if(g_Au.IsLoginWorker())
		{
			AuId avatar;
			AuId accepter;
			AuId idHaxus;

			block >> avatar;
			block >> accepter;
			block >> idHaxus;

			if(idHaxus == AuId::Haxus())
			{
				QString serverName = tcpSocket.peerAddress().toString();
				AuUnitTransfer unitTransfer(serverName, avatar, accepter);
				auLoginServerApp->m_LoginMutex.lock();
				auLoginServerApp->m_UnitTransfer.append(unitTransfer);
				auLoginServerApp->m_LoginMutex.unlock();
			}
		}
		else
		{
			logmsg = QObject::tr("%1 HACKER WARNING! Received TCP packet P_UnitTransfer "
					"size %2. Client should never send this to the Master "
					"server.").arg((unsigned long)currentThreadId()).arg(ba.count());
			g_Au.Log(logmsg);
			m_BadPackets++;
		}
		break;

	case P_VersionError:
		logmsg = QObject::tr("%1 HACKER WARNING! Received TCP packet P_VersionError "
				"size %2. Client should never send this to the server.")
				.arg((unsigned long)currentThreadId()).arg(ba.count());
		g_Au.Log(logmsg);
		m_BadPackets++;
		break;

	case P_AccountEmail:	// Sent to worker by client with new acct email.
		if(g_Au.IsLoginWorker())
		{
			QString acctName, newEmail;
			QByteArray password;
			block >> acctName;
			block >> password;
			block >> newEmail;

			AuSetEmail setEmail(acctName, password, newEmail);
			auLoginServerApp->m_LoginMutex.lock();
			auLoginServerApp->m_SetEmail.append(setEmail);
			auLoginServerApp->m_LoginMutex.unlock();
		}
		else
		{
			logmsg = QObject::tr("%1 HACKER WARNING! Received TCP packet "
					"P_AccountEmail size %2. "
					"Master server should never receive this.")
					.arg((unsigned long)currentThreadId()).arg(ba.count());
			g_Au.Log(logmsg);
			m_BadPackets++;
		}
		break;

	case P_AccountNew:					// Sent to worker by client with new account info.
										// Sent to client with new acct create result.
		if(g_Au.IsLoginWorker())
		{
			QString acctName, email;
			QByteArray password;
			quint8 acctType;

			block >> acctName;
			block >> password;
			block >> email;
			block >> acctType;

#if defined(Free2Play)
			// New accounts are only created in Active state.
			if(acctType == ACCT_Active)
#else
			// New accounts are only created in Locked Out or Trial states.
			if(acctType == ACCT_LockedOut || acctType == ACCT_Trial)
#endif
			{
				//logmsg = QString("P_AccountNew %1 %2 %3").arg(acctName).arg(password.size()).arg(email);
				//g_Au.Log(logmsg);
				// Queue new player create by db thread.
				AuNewPlayer player(m_Id, acctName, password, email, (ACCOUNT)acctType);
				auLoginServerApp->m_LoginMutex.lock();
				auLoginServerApp->m_NewPlayers.append(player);
				auLoginServerApp->m_LoginMutex.unlock();
			}
			else
			{
				logmsg = QObject::tr("%1 HACKER WARNING! Received TCP packet P_AccountNew size %2, "
						"name '%3' email '%5'. Invalid acctType %6.")
					.arg((unsigned long)currentThreadId())
					.arg(ba.count())
					.arg(acctName)
					.arg(email)
					.arg(acctType);
				g_Au.Log(logmsg);
				m_BadPackets++;
			}
		}
		else
		{
			logmsg = QObject::tr("%1 HACKER WARNING! Received TCP packet "
					"P_AccountNew size %2. "
					"Master server should never receive this.")
					.arg((unsigned long)currentThreadId()).arg(ba.count());
			g_Au.Log(logmsg);
			m_BadPackets++;
		}
		break;

	case P_AccountPassword:	// Sent to worker by client with new acct password.
		if(g_Au.IsLoginWorker())
		{
			QString acctName;
			QByteArray password, newPass;
			block >> acctName;
			block >> password;
			block >> newPass;

			AuSetPassword setPass(acctName, password, newPass);
			auLoginServerApp->m_LoginMutex.lock();
			auLoginServerApp->m_SetPassword.append(setPass);
			auLoginServerApp->m_LoginMutex.unlock();
		}
		else
		{
			logmsg = QObject::tr("%1 HACKER WARNING! Received TCP packet "
					"P_AccountPassword size %2. "
					"Master server should never receive this.")
					.arg((unsigned long)currentThreadId()).arg(ba.count());
			g_Au.Log(logmsg);
			m_BadPackets++;
		}
		break;

	case P_AccountResetPass:	// Sent to worker by client requesting password reset and result sent via e-mail to account address.
		if(g_Au.IsLoginWorker())
		{
			QString acctName;
			block >> acctName;

			AuResetPassword sendPass(acctName);
			auLoginServerApp->m_LoginMutex.lock();
			auLoginServerApp->m_ResetPassword.append(sendPass);
			auLoginServerApp->m_LoginMutex.unlock();
		}
		else
		{
			logmsg = QObject::tr("%1 HACKER WARNING! Received TCP packet "
					"P_AccountPassword size %2. "
					"Master server should never receive this.")
					.arg((unsigned long)currentThreadId()).arg(ba.count());
			g_Au.Log(logmsg);
			m_BadPackets++;
		}
		break;

	case P_UnitQuery:
		{
			AuId queryUnit;
			block >> queryUnit;
			if(g_Au.IsLoginWorker())
			{
				QString serverName = tcpSocket.peerAddress().toString();
				AuUnitQuery query(m_Id, queryUnit, serverName);
				auLoginServerApp->m_LoginMutex.lock();
				auLoginServerApp->m_UnitQueries.append(query);
				auLoginServerApp->m_LoginMutex.unlock();
			}
			else
			{
				Defer(tcpSocket);
			}
		}
		break;

	case P_InactiveUnitQuery:
		logmsg = QObject::tr("%1 HACKER WARNING! Received TCP packet type "
				"P_InactiveUnitQuery size %2. "
				"Client should never send this to the server.")
				.arg((unsigned long)currentThreadId()).arg(ba.count());
		g_Au.Log(logmsg);
		m_BadPackets++;
		break;

	case P_Ok:
		logmsg = QObject::tr("%1 HACKER WARNING! Received TCP packet type "
				"P_Ok size %2. "
				"Client should never send this to the server.")
				.arg((unsigned long)currentThreadId()).arg(ba.count());
		g_Au.Log(logmsg);
		m_BadPackets++;
		break;

	case P_QueryAvatars:
		{
			if(g_Au.IsLoginWorker())
			{
				QString serverName = tcpSocket.peerAddress().toString();
				AuQueryAvatars query(m_Id, serverName);
				auLoginServerApp->m_LoginMutex.lock();
				auLoginServerApp->m_QueryAvatars.append(query);
				auLoginServerApp->m_LoginMutex.unlock();
			}
			else
			{
				Defer(tcpSocket);
			}
		}
		break;

	case P_QueryAccountAvatars:
		{
			if(g_Au.IsLoginWorker())
			{
				quint32 accountId;
				quint32 confirmNum;

				block >> accountId;
				block >> confirmNum;

				QString serverName = tcpSocket.peerAddress().toString();
				AuQueryAccountAvatars query(m_Id, serverName, accountId, confirmNum);
				auLoginServerApp->m_LoginMutex.lock();
				auLoginServerApp->m_QueryAccountAvatars.append(query);
				auLoginServerApp->m_LoginMutex.unlock();
			}
			else
			{
				Defer(tcpSocket);
			}
		}
		break;
	}
}

// Defer the caller to a server on our server list.
void DtLoginServer::Defer(QTcpSocket& tcpSocket)
{
	// Key value is calculated to defer to a login server based on the peer address.
	QString peerAddr = tcpSocket.peerAddress().toString();
	unsigned int key = 0;
	unsigned int plen = peerAddr.count();
	if(plen == 1)
		key = peerAddr[0].unicode();
	else if(plen > 1)
		key = (peerAddr[plen-2].unicode()<<16)|peerAddr[plen-1].unicode();

	QMap<QString, DhLoginServer>& serverMap = auLoginServerApp->LockServerMap();
	if(!serverMap.empty())
	{
		// Select an address from the map.
		QMap<QString, DhLoginServer>::const_iterator it = serverMap.cbegin();
		unsigned int server = key % serverMap.size();
		it += server;
		QString toServer;
		if(m_LocalClient)
			toServer = (*it).m_PeerAddr;	// use peer address on LAN
		else
			toServer = (*it).m_NetName;		// use net name on internet
		quint16 toPort = (*it).m_NetPort;
		auLoginServerApp->UnlockServerMap();

		// Send the defer notification to the client.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)P_Deferred;
		out << toServer;
		out << toPort;
		m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
		m_BytesTx += tcpSocket.write(ba);
		tcpSocket.waitForBytesWritten();

		QString logmsg;
		logmsg = QObject::tr("%1 Deferred to %2:%3")
				.arg((unsigned long)currentThreadId()).arg(toServer).arg(toPort);
		g_Au.Log(logmsg);
	}
	else
	{
		auLoginServerApp->UnlockServerMap();

		// Notify the client of the defer failure.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)P_DeferFailed;
		m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
		m_BytesTx += tcpSocket.write(ba);
		tcpSocket.waitForBytesWritten();

		QString logmsg;
		logmsg = QObject::tr("%1 ERROR - Defer failed: No servers.")
				.arg((unsigned long)currentThreadId());
		g_Au.Log(logmsg);
	}
}

// Notifies master that this worker is alive and ready for work.
// static
void DtLoginServer::CheckIn()
{
	// Connect to the master server.
	QTcpSocket tcpSocket;
	tcpSocket.connectToHost(g_Au.LoginMasterName(), AuUtil::LoginMasterPort());
	tcpSocket.waitForConnected();
	if(tcpSocket.state() == QAbstractSocket::ConnectedState)
	{
		// The connection attempt succeeded.
		QString netName = g_Au.InternetName();
		quint16 netPort = AuUtil::LoginMasterPort();

		// Transmit the probe to the server and wait for a reply.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)P_CheckIn;
		out << netName;
		out << netPort;
		AuUtil::WritePacketSize(tcpSocket, ba.size());
		tcpSocket.write(ba);
		tcpSocket.waitForBytesWritten();

		// Close the tcp connection.
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			tcpSocket.disconnectFromHost();
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.waitForDisconnected();
		}

		QString logmsg;
		logmsg = QObject::tr("Checked in with master server %1.")
				.arg(g_Au.LoginMasterName());
		g_Au.Log(logmsg);
	}
	else
	{
		QString logmsg;
		logmsg = QObject::tr("ERROR! Could not check in with master server %1.")
				.arg(g_Au.LoginMasterName());
		g_Au.Log(logmsg);

		// Quit now so shutting down the master shuts down the workers too.
		auLoginServerApp->quit();
	}
}
