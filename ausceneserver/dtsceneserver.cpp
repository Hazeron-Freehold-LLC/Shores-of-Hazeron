//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtsceneserver.cpp,v 1.894 2022/05/17 19:01:56 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dtsceneserver.h"
//
#include "auatominterface.h"
#include "audbglobal.h"
#include "aulogincomm.h"
#include "auplayeronlinestatus.h"
#include "ausceneserverapp.h"
#include "austarmapdata.h"
#include "daasteroid.h"
#include "daatomlist.h"
#include "dagalaxy.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dastar.h"
#include "dateleporter.h"
#include "dauniverse.h"
#include "davehicle.h"
#include "davessel.h"
#include "daworldglobe.h"
#include "dgdata.h"
#include "dicontrolinput.h"
#include "dtsspacket.h"
#include <QDataStream>
#include <QUdpSocket>
#include <QVariant>

DtSceneServer::DtSceneServer(unsigned int id, int socketDesc, QObject* parent) : QThread(parent)//, m_semAlive(1)
{
	m_AccountType = ACCT_Count;
	m_BadPackets = 0;
	m_BytesRx = 0;
	m_BytesTx = 0;
	m_Id = id;
	m_idPlayer = 0;
	m_isUnitNewbie = false;
	m_AFK = false;
	m_ChatClient = false;
	m_LocalClient = false;
	m_OsType = 0;
	m_OsVer = 0;
	m_PacketSize = 0;
	m_PacketSizeBytesLeft = 0;
	m_QueryReason = QUERY_None;
	m_socketDescriptor = socketDesc;
	m_SyncCount = 0;
	m_SyncGalaxy = GALAXY_Count;
	m_SyncLastCount = 0;
	memset(m_SyncLastAtoms, 0, sizeof(m_SyncLastAtoms));
	memset(m_SyncLastBytes, 0, sizeof(m_SyncLastBytes));
	memset(m_SyncLastPing, 0, sizeof(m_SyncLastPing));
	m_SyncTotalAtoms = 0;
	m_SyncTotalBytes = 0;
	m_ThreadId = 0;
	m_TxBufferFlush = false;
	m_UdpSequence = 0;
	m_WordSize = 0;
	m_confirmNum = 0;
	m_IdEmpire = 0;
	m_EmpireQuery = 0;
	m_idUnitEmpire = 0;		// Id of player's person unit atom's empire.
	//m_semAlive.acquire(1);			// I live...

	m_PosAtRest = false;
	m_PosHeadTilt = 0;
	m_PosParentAlso = false;
	m_PosParentAtRest = false;
	for(int ps=0; ps<DpBody::PS_Count; ps++)
	{
		m_PosParentPose[ps] = AuPose::Init;
		m_PosPose[ps] = AuPose::Init;
	}
	m_PosParentStationId = 0;
	m_PosParentWhichConsole = 0;
	m_PosSeatIndex = 0;
	m_PosStationId = 0;
	m_PosWhichConsole = 0;

	connect(this, SIGNAL(finished()), this, SLOT(ThreadFinished()));
}

void DtSceneServer::ThreadFinished()
{
	//qDebug() << "DtSceneServer::ThreadFinished id" << m_Id;
	// Signal thread completion with my id.
	emit Finished(m_Id);
}

void DtSceneServer::run()
{
	m_ThreadId = currentThreadId();

	// Creating the socket on the stack automatically associates it with the
	// thread's event loop. This ensures that Qt will not try to deliver events
	// to our socket from the main thread while we are accessing it from
	// DtSceneServer::run().
	QTcpSocket tcpSocket;
	if(!tcpSocket.setSocketDescriptor(m_socketDescriptor))
	{
		emit error(tcpSocket.error());
		return;
	}

	m_tmConnect.Set();
	QString peerAddr = tcpSocket.peerAddress().toString();
	//if(peerAddr.left(8) == "192.168." || peerAddr == "127.0.0.1" || peerAddr.right(9) == "127.0.0.1" ||
			//peerAddr.left(15) == "::ffff:192.168.")
		//m_LocalClient = true;	// This is cheesy but it works for my purposes.
	QString logmsg = tr("SCENE:%99 connected %1").arg(peerAddr).arg(m_Id);
	g_Au.LogConnect(auSceneServerApp->LoginName(), logmsg);

	const int nominalSyncTime = 1000/30;	// 30 frames/sec :)
	AuElapsedTimer syncTimer;
	syncTimer.start();

	// Login check interval must be longer than the cache decay time in the login server.
	const int loginCheckInterval = 20 * 60;	// seconds between login checks
	AuElapsedTimer tmLoginCheck;
	while(tcpSocket.state() == QAbstractSocket::ConnectedState && !tcpSocket.waitForDisconnected(10))
	{
		// Still connected so check to see if we have anything to send.
		m_muTxBuffer.lock();
		if(!m_TxBuffer.isEmpty())
		{
			// There is data to be sent.
			// It is already formatted into packets with block sizes.
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
			{
				int written = tcpSocket.write(m_TxBuffer);
				m_BytesTx += written;
				m_TxBuffer.clear();

				// Sometimes we must flush the output stream.
				if(m_TxBufferFlush)
				{
					tcpSocket.flush();
					m_TxBufferFlush = false;
				}
			}
		}
		m_muTxBuffer.unlock();
		if(tcpSocket.state() != QAbstractSocket::ConnectedState)
			break;

		// Pull in any data that is waiting for us.
		// Note: If we wait for the signal, it appears to come from the gui
		// thread, which causes us grief. It's better for us to get the data
		// when it's there.
		RxData(tcpSocket);
		if(tcpSocket.state() != QAbstractSocket::ConnectedState)
			break;

		// Check to see if the login server still thinks this guy is logged in.
		if(!gp_AuGlobal)
			break;
		if(!tmLoginCheck.isValid() || tmLoginCheck.elapsed()/d_MSecPerSec > loginCheckInterval)
		{
			CheckLoginStatus();
			tmLoginCheck.restart();
			if(tcpSocket.state() != QAbstractSocket::ConnectedState)
				break;
		}

		// When a disconnect time is established, we have to eventually disconnect.
		if(m_tmDisconnect.isValid())
		{
			// We allow a brief time so the client can get the reason packet before we disconnect the socket.
			if(m_tmDisconnect.elapsed()/d_MSecPerSec > 1)
			{
				if(tcpSocket.state() == QAbstractSocket::ConnectedState)
					tcpSocket.disconnectFromHost();
				if(tcpSocket.state() == QAbstractSocket::ConnectedState)
					tcpSocket.waitForDisconnected(10000);
			}
		}
		else
		{
			// We are not scheduled to disconnect.
			bool timeToSync = false;
			if(syncTimer.elapsed() >= nominalSyncTime)
			{
				syncTimer.restart();
				timeToSync = true;
			}

			// We are not scheduled to disconnect. If the transmit buffer is empty, send a scene refresh to the client.
			if(tcpSocket.bytesToWrite() == 0)
			{
				// Transmit buffer is empty.
				if(!m_IdDeferEstTo.IsNull())
				{
					// Keep trying to defer to the nu server until we succeed.
					// Success will disconnect the connection.
					DeferEst();
				}
				else
				{
					// Until the client has a unit id, there is no scene to refresh and no unit to defer.
					// Do not defer the client until they have a non-zero unit id.
					if(!m_idUnit.IsNull())
					{
						if(g_Au.IsSceneWorker())
						{
							// Scene worker syncronizes the client scene.
							// Try to achieve smooth timing between calls to SyncClient.
							if(timeToSync)
							{
								SyncClient();
								timeToSync = false;	// No need to call IdleClient. SyncClient does that right off the bat.
							}
						}
						else
						{
							// The scene master and scene manager defer the client to the subordinate server controlling the region
							// of space that contains the person's unit.
							DeferClient();
						}
					}
				}
			}

			// Don't let their unit think it is off line because we haven't sent a scene sync in a while.
			if(timeToSync && !m_idUnit.IsNull() && g_Au.IsSceneWorker())
				IdleClient();
		}
	}

	//qDebug("DtSceneServer::run - Player %d disconnected.", m_idPlayer);
	if(!m_idUnit.IsNull())
	{
		// Release client control of their avatar and vehicle/ship.
		ReleaseClientControl();
		emit Disconnecting(m_idUnit);
	}
	logmsg = tr("SCENE:%99 thread exit %1").arg(peerAddr).arg(m_Id);
	if(gp_AuGlobal)
		g_Au.LogConnect(auSceneServerApp->LoginName(), logmsg);
}

void DtSceneServer::RxData(QTcpSocket& tcpSocket)
{
	//qDebug("DtSceneServer::RxData");
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

		// It is now safe to read our data packet from the socket buffer.
		QByteArray block = tcpSocket.read(m_PacketSize);
		m_BytesRx += m_PacketSize;
		m_PacketSize = 0;
		ReadPacket(block, tcpSocket);	// Read the contents of the packet.
	}
}

void DtSceneServer::UdpPacketReceived(quint32 playerId, QByteArray packet)
{
	if(m_idPlayer == playerId)
		ReadUdpPacket(packet);
}

void DtSceneServer::ReadUdpPacket(const QByteArray& ba)
{
	if(m_tmDisconnect.isValid())
		return;							// Ignore packets when preparing to disconnect.

	m_BytesRx += ba.count();
	QDataStream block(ba);
	block.setVersion(QDataStream::Qt_4_1);

	// The first byte is always the packet type.
	unsigned char type;
	block >> type;

	// Datagrams may arrive out of order.
	quint16 seq;
	AuTime sent;
	block >> seq;
	block >> sent;
	//qDebug() << "DtSceneServer::ReadUdpPacket type" << type << "seq" << seq << "sent" << sent;

	// Milliseconds from time sent by client to time recieved by me.
	qint64 msecs = sent.ElapsedMSec();

	// Milliseconds until the packet is considered too late to process.
	int udpDecayMs = SyncAveragePing() + g_Au.UdpDecayMargin();
	if(udpDecayMs > 2000)
		udpDecayMs = 2000;

	// We really don't want old udp packets so they just get ignored.
	if(msecs > udpDecayMs)
	{
		//qDebug() << "DtSceneServer::ReadUdpPacket player" << m_idPlayer << "unit" << m_idUnit << "msecs" << msecs
			//<< "udpDecayMs" << udpDecayMs;
		return;													// Packet is too old. This is not logged as a "bad" packet.
	}

	int diff = (int)seq - (int)m_UdpSequence;
	if((diff > 0 && diff < 2000) ||						// later without rollover (unlikely) OR
			(diff < -63000))							// probably acceptable rollover
	{
		// It doesn't look like we got an earlier packet.
		// Remember this one's sequence number for the next datagram.
		m_UdpSequence = seq;
	}
	else
	{
		if(seq != 0 || m_UdpSequence != 0)
		{
			g_Au.LogConnect(auSceneServerApp->LoginName(),
					QString("SCENE:%99 ERROR! Player %1 unit %2 %3: UDP packet type %4 sequence mismatch "
						"packet=%5 server=%6")
				.arg(m_idPlayer).arg(m_idUnit.GetID()).arg(m_UnitName)
				.arg(type).arg(seq).arg(m_UdpSequence).arg(m_Id));
			m_BadPackets++;
		}
		m_UdpSequence = seq;	// Try to recover.
		return;
	}

	// The udp packet can be processed.
	switch(type)
	{
	case DtSceneServerPacket::PI_Control:
		{
			// Control input comes to us as a datagram.
			AuId idto;
			idto << block;
			quint8 hasWhat;
			block >> hasWhat;

			AuGearPos pos;
			if(hasWhat & 0x01)
				block >> pos;

			qint8 selValue;
			if(hasWhat & 0x04)
				block >> selValue;
			else
				selValue = 0;

			DiControlInputArray ac;
			ac.Rx(block);
			ProcessControlInput(idto, ac, pos, selValue);
		}
		break;

	default:
		g_Au.LogConnect(auSceneServerApp->LoginName(),
				QString("SCENE:%99 ERROR! Player %1 unit %2 %3: Unknown UDP packet type %4 size %5. Hacker?")
				.arg(m_idPlayer).arg(m_idUnit.GetID()).arg(m_UnitName).arg(type).arg(ba.count()).arg(m_Id));
		m_BadPackets++;
		break;
	}
}

void DtSceneServer::ReadPacket(const QByteArray& ba, QTcpSocket& tcpSocket)
{
	if(m_tmDisconnect.isValid())
		return;							// Ignore packets when preparing to disconnect.
	QDataStream block(ba);
	block.setVersion(QDataStream::Qt_4_1);

	QString logmsg;
	quint8 result = 0;

	// The first byte is always the packet type.
	quint8 type;
	block >> type;
	//qDebug() << "DtSceneServer::ReadPacket type" << DtSceneServerPacket::PacketName((DtSceneServerPacket::PACKET)type);

	switch((DtSceneServerPacket::PACKET)type)	// Generate warning when enum forgotten.
	{
	case DtSceneServerPacket::PL_KeepAlive:
		break;

	case DtSceneServerPacket::PI_Deadhead:
		{
			AuId departingSystemId, travId;
			AuPoint departureLoc;

			block >> departingSystemId;
			block >> travId;
			block >> departureLoc;

			if(departingSystemId == m_idSystem)
				ClientDeadheaded(travId, departureLoc);

			// Disconnect from the client when they deadhead to another system.
			QString logmsg = QString("SCENE:%99 Disconnect. Player %1 %2: Atom %3 deadheaded from system %4 at (%L5, %L6, %L7).")
					.arg(m_idPlayer)
					.arg(tcpSocket.peerAddress().toString())
					.arg(travId.GetID())
					.arg(departingSystemId.GetID())
					.arg(departureLoc.x)
					.arg(departureLoc.y)
					.arg(departureLoc.z)
					.arg(m_Id);
			g_Au.LogConnect(auSceneServerApp->LoginName(), logmsg);
			Disconnect();
		}
		break;

	case DtSceneServerPacket::PI_Collision:
		{
			AuId myId, hitId;
			AuPoint myChange, myHitLoc, hitChange, hitFromLoc, hitResultVec;
			bool hitShield;

			block >> myId;
			block >> myChange;
			block >> myHitLoc;
			block >> hitId;
			block >> hitChange;
			block >> hitFromLoc;
			block >> hitResultVec;
			block >> hitShield;

			ClientCollision(myId, myChange, myHitLoc, hitId, hitChange, hitFromLoc, hitResultVec, hitShield);
		}
		break;

	case DtSceneServerPacket::PI_Fell:
		{
			AuId fellId;
			quint32 hitsDamage;
			int stopReason;

			block >> fellId;
			block >> hitsDamage;
			block >> stopReason;

			ClientFell(fellId, hitsDamage, stopReason);
		}
		break;

	case DtSceneServerPacket::PI_EnteredWormhole:
		{
			AuId travId;
			AuId systemId;
			AuWormhole wh;
			float entryAngle;

			block >> travId;
			block >> systemId;
			block >> wh;
			block >> entryAngle;

			if(systemId == m_idSystem)
				ClientEnteredWormhole(travId, wh, entryAngle);

			// Disconnect from the client when they enter a wormhole.
			QString logmsg = QString("SCENE:%99 Disconnect. Player %1 %2: Atom %3 entered wormhole %4 %5 from system %6 "
					"to system %7.")
					.arg(m_idPlayer)
					.arg(tcpSocket.peerAddress().toString())
					.arg(travId.GetID())
					.arg(wh.Name())
					.arg(wh.Polarity())
					.arg(systemId.GetID())
					.arg(wh.ToSystem().GetID())
					.arg(m_Id);
			g_Au.LogConnect(auSceneServerApp->LoginName(), logmsg);
			Disconnect();
		}
		break;

	case DtSceneServerPacket::PI_Position:
		if(g_Au.IsSceneWorker())
		{
			quint16 hasWhat;
			block >> hasWhat;

			float headTilt;
			if(hasWhat & 0x0001)
				block >> headTilt;
			else
				headTilt = m_PosHeadTilt;

			AuAtomPos pos;
			if(hasWhat & 0x0002)
				pos.Rx(block);
			else
				pos = m_Pos;

			AuPose pose[DpBody::PS_Count];
			quint16 whichConsole;
			quint16 stationId;
			if(hasWhat & 0x0008)
			{
				for(int ps=0; ps<DpBody::PS_Count; ps++)
					block >> pose[ps];
				block >> whichConsole;
				block >> stationId;
			}
			else
			{
				for(int ps=0; ps<DpBody::PS_Count; ps++)
					pose[ps] = m_PosPose[ps];
				whichConsole = m_PosWhichConsole;
				stationId = m_PosStationId;
			}

			AuPoint vec;
			if(hasWhat & 0x0010)
				block >> vec;
			else
				vec = m_PosVec;

			bool atRest = hasWhat & 0x0020;

			quint8 seatIndex;
			if(hasWhat & 0x0040)
				block >> seatIndex;
			else
				seatIndex = m_PosSeatIndex;

			bool parentAlso = hasWhat & 0x0100;
			AuAtomPos parentPos;
			AuPose parentPose[DpBody::PS_Count];
			for(int ps=0; ps<DpBody::PS_Count; ps++)
				parentPose[ps] = AuPose::Init;
			quint16 parentWhichConsole = 0;
			quint16 parentStationId = 0;
			AuPoint parentVec;
			bool parentAtRest = false;
			if(parentAlso)
			{
				if(hasWhat & 0x0200)
					parentPos.Rx(block);
				else
					parentPos = m_PosParent;

				if(hasWhat & 0x0800)
				{
					for(int ps=0; ps<DpBody::PS_Count; ps++)
						block >> parentPose[ps];
					block >> parentWhichConsole;
					block >> parentStationId;
				}
				else
				{
					for(int ps=0; ps<DpBody::PS_Count; ps++)
						parentPose[ps] = m_PosParentPose[ps];
					parentWhichConsole = m_PosParentWhichConsole;
					parentStationId = m_PosParentStationId;
				}

				if(hasWhat & 0x1000)
					block >> parentVec;
				else
					parentVec = m_PosParentVec;

				parentAtRest = hasWhat & 0x2000;
			}

			ClientPositionUpdate(atRest, headTilt, pos, pose, whichConsole, stationId, seatIndex, vec, parentAlso, parentAtRest,
					parentPos, parentPose, parentWhichConsole, parentStationId, parentVec);
		}
		break;

	case DtSceneServerPacket::PG_StarMap:
		// Sent to master to get data for galaxy viewer specific to a particular empire.
		if(g_Au.IsSceneMaster())
		{
			quint32 idPlayer, confirmNum, empireId;
			AuId personId, sectorId, personSystemId;
			quint8 query;	// 0=universe, 1=galaxy, 2=sector
			quint8 galaxy = GALAXY_Count;
			quint16 range = 0;	// in sectors
			quint16 empireCount;
			QList<quint32> empires;	// empire records the client already has

			block >> idPlayer;
			block >> confirmNum;
			block >> personId;
			block >> empireId;
			block >> personSystemId;
			block >> query;
			if(query == 1)
				block >> galaxy;
			else if(query == 2)
			{
				block >> sectorId;
				block >> range;
			}
			block >> empireCount;
			while(empireCount--)
			{
				quint32 empireId;
				block >> empireId;
				empires.append(empireId);
			}

			if(AuGlobal::TestConfirmationValidity(confirmNum))
			{
				DgEmpire empire;
				empire.m_id = empireId;
				const DgData& gd = g_Au.LockDataForRead();
				bool hasEmpire = gd.Empire(empire);
				g_Au.UnlockData();
				if(!hasEmpire)
				{
					// No such empire.
					g_Au.LogConnect(auSceneServerApp->LoginName(), QString("SCENE:%99 DISCONNECT! Attempted PG_StarMap. "
								"INVALID EMPIRE! %1 %2 %3 %4")
							.arg(idPlayer).arg(personId.GetID()).arg(empireId).arg(tcpSocket.peerAddress().toString()).arg(m_Id));
					Disconnect();
				}
				else if(!empire.HasCitizen(personId))
				{
					// Person is not a citizen of that empire.
					g_Au.LogConnect(auSceneServerApp->LoginName(), QString("SCENE:%99 DISCONNECT! Attempted PG_StarMap. "
								"INVALID CITIZEN! %1 %2 %3 %4")
							.arg(idPlayer).arg(personId.GetID()).arg(empireId).arg(tcpSocket.peerAddress().toString()).arg(m_Id));
					Disconnect();
				}
				else
				{
					// Collect galaxy viewer data.
					AuStarMapData gvdata;
					AuAtomInterface::CollectStarMapData(empireId, gvdata, query, (GALAXY)galaxy, sectorId, range, personSystemId);

					// Send the client any empire records that are in the star map that they do not already have.
					// We send them before the map so they already have them when the map data arrives.
					QSet<quint32> empiresInMap;
					gvdata.ListEmpires(empiresInMap);
					QSet<quint32>::iterator ei = empiresInMap.begin();
					while(ei != empiresInMap.end())
					{
						if(empires.contains(*ei))
							ei = empiresInMap.erase(ei);	// They already have this one.
						else
						{
							// They do not have this one. Do I have it?
							const DgData& gd = g_Au.LockDataForRead();
							bool hasEmpire = gd.HasEmpire(*ei);
							g_Au.UnlockData();
							if(!hasEmpire)
								ei = empiresInMap.erase(ei);	// I do not have that empire.
							else
								ei++;			// I have the empire record.
						}
					}
					//qDebug() << "PG_StarMap sending" << empiresInMap.size() << "empire records before star map";
					for(QSet<quint32>::iterator ei = empiresInMap.begin(); ei != empiresInMap.end(); ei++)
					{
						// Send the empire's data.
						DgEmpire empire;
						empire.m_id = *ei;
						const DgData& gd = g_Au.LockDataForRead();
						gd.Empire(empire);
						g_Au.UnlockData();

						QByteArray ba;
						QDataStream out(&ba, QIODevice::WriteOnly);
						out.setVersion(QDataStream::Qt_4_1);
						out << (quint8)DtSceneServerPacket::PG_Empire;
						empire.Tx(out, m_idUnitEmpire == empire.m_id);	// When sending the person's empire, include announcements.

						m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
						m_BytesTx += tcpSocket.write(ba);
					}

					// Send the star map data.
					QByteArray ba;
					QDataStream out(&ba, QIODevice::WriteOnly);
					out.setVersion(QDataStream::Qt_4_1);
					out << (quint8)DtSceneServerPacket::PG_StarMap;
					gvdata >> out;
					if(query == 0)	// Log the big queries to see how big they get.
						g_Au.Log(QString("PG_StarMap player %1 avatar %2 empire %3 sectors %4 packetSize %5 bytes")
								.arg(idPlayer)
								.arg(personId.GetID())
								.arg(empireId)
								.arg(gvdata.m_Sectors.size())
								.arg(ba.size()));
					m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
					m_BytesTx += tcpSocket.write(ba);
					tcpSocket.waitForBytesWritten();
				}
			}
			else
			{
				// Bogus confirmation number.
				g_Au.LogConnect(auSceneServerApp->LoginName(), QString("SCENE:%99 DISCONNECT! Attempted PG_StarMap. INVALID CONFIRMATION NUMBER! "
							"%1 %2 %3 %4")
						.arg(idPlayer).arg(personId.GetID()).arg(empireId).arg(tcpSocket.peerAddress().toString()).arg(m_Id));
				Disconnect();
			}
		}
		else
		{
			g_Au.LogConnect(auSceneServerApp->LoginName(), QString("SCENE:%99 DISCONNECT! Attempted PG_StarMap but I'M NOT THE MASTER SERVER! "
						"%1 unit %2 %3 %4")
					.arg(m_idPlayer).arg(m_idUnit.GetID()).arg(m_UnitName).arg(tcpSocket.peerAddress().toString()).arg(m_Id));
			Disconnect();
		}
		break;

	case DtSceneServerPacket::PZ_PlayersQuery:
		//qDebug("PZ_PlayersQuery:");
		// Sent to master to get a list of all players currently on-line.
		if(g_Au.IsSceneMaster())
		{
			// Client doesn't have to be logged in to this server to make this request. We'll expect at least a valid
			// confirmation number.
			quint32 idPlayer, confirmNum;
			block >> idPlayer;
			block >> confirmNum;
			if(AuGlobal::TestConfirmationValidity(confirmNum))
			{
				// Collect players list.
				AuPlayerOnlineStatusVector players;
				auSceneServerApp->PlayersOnlineStatus(players);

				// Reply with the status packet.
				QByteArray ba;
				QDataStream out(&ba, QIODevice::WriteOnly);
				out.setVersion(QDataStream::Qt_4_1);
				out << (quint8)DtSceneServerPacket::PZ_PlayersQuery;
				players >> out;
				if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				{
					m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
					m_BytesTx += tcpSocket.write(ba);
					tcpSocket.waitForBytesWritten();
				}
			}
			else
			{
				g_Au.LogConnect(auSceneServerApp->LoginName(), QString("SCENE:%99 DISCONNECT! Attempted PZ_PlayersQuery. INVALID CONFIRMATION NUMBER! %1 %2")
						.arg(idPlayer).arg(tcpSocket.peerAddress().toString()).arg(m_Id));
				Disconnect();
			}
		}
		else
		{
			g_Au.LogConnect(auSceneServerApp->LoginName(), QString("SCENE:%99 DISCONNECT! Attempted PZ_PlayersQuery but I'M NOT THE MASTER SERVER! %1 unit %2 %3 %4")
						.arg(m_idPlayer).arg(m_idUnit.GetID()).arg(m_UnitName).arg(tcpSocket.peerAddress().toString()).arg(m_Id));
			Disconnect();
		}
		break;

	case DtSceneServerPacket::PZ_StatusQuery:
		//qDebug("PZ_StatusQuery:");
		// Sent by AuServerStatus to get my current status. An immediate reply is expected.
		{
			// Collect status values.
			quint8 status = (quint8)auSceneServerApp->DbStatus();	// Status type.
			AuTime tmUp = auSceneServerApp->TimeUp();
			quint16 managers = auSceneServerApp->ManagerCount();
			quint16 workers = auSceneServerApp->WorkerCount();
			quint32 players = auSceneServerApp->PlayerCount();
			quint16 workerCores = auSceneServerApp->WorkerCoreCount();

			// Send recent galactic chat for the web site.
			QList<AuTime> posted;
			QList<quint8> galaxies;
			QStringList senders;
			QStringList msgs;
			const AuAtomChatList* chat = g_AuDb.LockRecentAtomChat();
			for(QList<AuAtomChat>::const_iterator it = chat->cbegin(); it != chat->cend(); it++)
				if((*it).CommPacket() &&
						(*it).CommPacket()->TransmissionRange() == AuCommStdChannel::Universe &&	// Universe range.
						(*it).CommPacket()->ToId().IsNull() &&										// Publicly addressed.
						(AuCommStdChannel::IsGalacticChannel((*it).CommPacket()->ChannelName()) ||	// Galactic channel.
						 (*it).CommPacket()->Scope() == AuCommStdChannel::Thoughts))				// Broadcast at universe range.
				{
					posted.append((*it).TimeStamp());
					galaxies.append((*it).CommPacket()->FromGalaxy());
					senders.append((*it).CommPacket()->FromName());
					msgs.append((*it).CommPacket()->Text());
				}
			g_AuDb.UnlockRecentAtomChat();

			// Reply with the status packet.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtSceneServerPacket::PZ_StatusQuery;
			out << status;
			out << tmUp.UTC();
			out << managers;
			out << workers;
			out << players;
			out << workerCores;
			out << posted;
			out << galaxies;
			out << senders;
			out << msgs;
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
			{
				m_BytesTx += AuUtil::WritePacketSize(tcpSocket, ba.size());
				m_BytesTx += tcpSocket.write(ba);
				tcpSocket.waitForBytesWritten();
			}
		}
		break;

	case DtSceneServerPacket::P_SyncComplete:
		//qDebug("P_SyncComplete:");
		if(g_Au.IsSceneWorker())
		{
			// Only the scene worker cares about sync complete packets.
			//DHH The master sometimes gets a sync complete right away during a handoff, perhaps due to a sync complete packet
			//DHH queued for transmission on the client right when the handoff occurs. We don't want to disconnect them for this.
			// Client sent back the sync complete packet so I can calculate ping time.
			AuTime syncUTC;
			syncUTC.Rx32(block);

			qint64 ping = syncUTC.ElapsedMSec();
			if(ping < 0)
				ping = 1;

			//m_rwSync.lockForRead();
			//int syncCount = m_SyncCount;
			//m_rwSync.unlock();

			m_rwSyncLast.lockForWrite();
			m_SyncLastCount++;
			//if(ping > 200)
				//qDebug() << "P_SyncComplete ping" << ping/2 << "ms" << syncCount << "syncs" << m_SyncLastCount << "complete";
			quint32 syncLastIndex = m_SyncLastCount%c_SyncHistory;
			m_SyncLastPing[syncLastIndex] = ping/2;				// Store one-way ping time.
			if(m_SyncLastCount > m_SyncCount)
			{
				g_Au.LogConnect(auSceneServerApp->LoginName(), QString("SCENE:%99 DISCONNECT! SyncComplete packets exceed Syncs! Player %1 unit %2 %3 TCP %4")
						.arg(m_idPlayer).arg(m_idUnit.GetID()).arg(m_UnitName).arg(tcpSocket.peerAddress().toString()).arg(m_Id));
				Disconnect();
			}
			m_rwSyncLast.unlock();
		}
		break;

	case DtSceneServerPacket::PL_SwitchAvatar:
		{
			AuId personId;

			block >> personId;

			SwitchToCharacter(personId);
		}
		break;

	case DtSceneServerPacket::PL_Login:	// Login information.
		//qDebug("PL_Login:");
		// Client sent us their login player id and confirmation number issued by the login server.
		{
			// Read player id and login confirmation number from packet.
			QString sver;
			quint32 idPlayer, confirmNum;
			AuId idUnit;

			block >> sver;
			block >> idPlayer;
			block >> confirmNum;
			block >> idUnit;

			// Client OS Information
			quint8 status;
			block >> m_OsType;
			block >> m_OsVer;
			block >> m_WordSize;
			block >> status;

			m_AFK = status & 0x01;
			m_ChatClient = status & 0x02;	// true if they are using the chat client "Hazeron Commander".
			bool initLogin = status & 0x04;	// true if this is their first worker server login, so their ship can contact them.

			if(sver != AuUtil::SceneVersion())
			{
				// Disconnect wrong version without logging it.
				m_AccountType = ACCT_Count;
				m_confirmNum = 0;
				m_idPlayer = 0;
				m_idUnit = AuId(0);
				// Clear the unit name.
				if(g_Au.IsSceneWorker())
				{
					m_muUnitName.lock();
					m_UnitName = "";
					m_muUnitName.unlock();
				}

				// Send a message to the client indicating login failure.
				QByteArray ba;
				QDataStream out(&ba, QIODevice::WriteOnly);
				out.setVersion(QDataStream::Qt_4_1);
				out << (quint8)DtSceneServerPacket::PL_LoginFailure;
				out << (quint8)1;	// 1=bad version
				SendPacket(ba);
				Disconnect();
				break;
			}

			g_Au.LogConnect(auSceneServerApp->LoginName(), QString("SCENE:%99 Login Player %1 Confirmation Number %2 Unit %3")
					.arg(idPlayer).arg(confirmNum).arg(idUnit.GetID()).arg(m_Id));

			AuId idUnits[6];
			if(g_Au.IsPlayerLoggedIn(idPlayer, confirmNum, idUnits, m_AccountType, m_AccountExpiry))
			{
				// Emit a signal to insure no other channels are open to the player.
				emit PlayerConnecting(idPlayer, (unsigned long)m_ThreadId);

				m_confirmNum = confirmNum;
				m_idPlayer = idPlayer;
				//qDebug("PL_Login playerId %d unitId %d", idPlayer, idUnit.GetID());

				// Verify the client passed us a valid unit id. It must be listed in the array of six ids that belong to the
				// player, as returned by the login server.
				if(!idUnit.IsNull())
				{
					for(int iu=0; iu<6; iu++)
						if(idUnit == idUnits[iu])
						{
							m_idUnit = idUnit;
							break;
						}
				}

				QString motd;
				if(g_Au.IsSceneMaster())
				{
					// Scene master sends message of the day.
					QFile motdFile("motds");
					if(motdFile.open(QIODevice::ReadOnly))
					{
						// Read in the messages-of-the-day file.
						QTextStream in(&motdFile);
						QStringList motds;
						while(!in.atEnd())
							motds.append(in.readLine());
						motdFile.close();
						if(!motds.isEmpty())
						{
							// We read some motds. Pick one of them semi-randomly, based on today's Julian day.
							int day = g_Au.CurrentUTC().date().toJulianDay();
							int msg = day%motds.count();
							motd = motds[msg].replace("\\n", "\n");
						}
						else
							qWarning("warning: Empty 'motds' file. No message-of-the-day will be sent.");
					}
					else
						qWarning("warning: Failed to read 'motds' file. No message-of-the-day will be sent.");
					//qDebug("MOTD motds %d day %d msg %d", motds, day, msg);
				}
				else if(g_Au.IsSceneWorker())
				{
					// Pick up the person's name ASAP.
					QString name;
					if(!m_idUnit.IsNull())
					{
						AuEther& ether = g_Au.LockEtheriumDelete();
						const DaPerson* person = dynamic_cast<const DaPerson*>(ether.FindNormalAtom(m_idUnit));
						if(person)
						{
							name = person->AssignedName();	// Get the real name of agents, instead of 'Entity' or 'Animal';
							if(initLogin)
								m_ContactShip = person->SpawnShip();
						}
						g_Au.UnlockEtheriumDelete();
					}
					m_muUnitName.lock();
					if(name.isEmpty())
						m_UnitName = g_Au.MiddleDotSymbol();
					else
						m_UnitName = name;
					m_muUnitName.unlock();
				}

				QString osTypeName = AuPlayerOnlineStatus::OsTypeName(m_OsType);
				QString osVerName = AuPlayerOnlineStatus::OsVersionName(m_OsType, m_OsVer);
				if(g_Au.IsSceneWorker())
					g_Au.LogConnect(auSceneServerApp->LoginName(),
							QString("SCENE:%99 Login player %2 confirm %3. Unit %4 %5 type %6. %7 %8 ×%9 bits")
							.arg(idPlayer)
							.arg(confirmNum)
							.arg(m_idUnit.GetID())
							.arg(m_UnitName)
							.arg(AcctTypeName(m_AccountType))
							.arg(osTypeName)
							.arg(osVerName)
							.arg(m_WordSize)
							.arg(m_Id));
				else
					g_Au.LogConnect(auSceneServerApp->LoginName(),
							QString("SCENE:%99 Login player %2 confirm %3. Unit %4 type %6. %7 %8 ×%9 bits")
							.arg(idPlayer)
							.arg(confirmNum)
							.arg(m_idUnit.GetID())
							.arg(AcctTypeName(m_AccountType))
							.arg(osTypeName)
							.arg(osVerName)
							.arg(m_WordSize)
							.arg(m_Id));

				// Send a message to the client indicating login success.
				QByteArray ba;
				QDataStream out(&ba, QIODevice::WriteOnly);
				out.setVersion(QDataStream::Qt_4_1);
				out << (quint8)DtSceneServerPacket::PL_Login;
				out << motd;
				out << g_Au.ChatPort();			// Tell the client what port to connect for chat.
				SendPacket(ba);
			}
			else
			{
				m_AccountType = ACCT_Count;
				m_confirmNum = 0;
				m_idPlayer = 0;
				m_idUnit = AuId(0);
				// Clear the unit name.
				if(g_Au.IsSceneWorker())
				{
					m_muUnitName.lock();
					m_UnitName = "";
					m_muUnitName.unlock();
				}

				// Send a message to the client indicating login failure.
				QByteArray ba;
				QDataStream out(&ba, QIODevice::WriteOnly);
				out.setVersion(QDataStream::Qt_4_1);
				out << (quint8)DtSceneServerPacket::PL_LoginFailure;
				out << (quint8)0;	// 0=player not logged in
				SendPacket(ba);
				Disconnect();

				g_Au.LogConnect(auSceneServerApp->LoginName(), tr("SCENE:%99 DISCONNECT! Login failed: player %2 confirm %3 TCP %4")
						.arg(idPlayer).arg(confirmNum).arg(tcpSocket.peerAddress().toString()).arg(m_Id));
			}
		}
		break;

	case DtSceneServerPacket::PL_Logout:	// Logout client player
		//qDebug("PL_Logout:");
		// Client notification that they are logging out.
		{
			g_Au.LogConnect(auSceneServerApp->LoginName(), QString("SCENE:%99 Disconnect. Logout player %2. TCP %3")
					.arg(m_idPlayer).arg(tcpSocket.peerAddress().toString()).arg(m_Id));
			Disconnect();
		}
		break;

	case DtSceneServerPacket::PG_Data:	// Client req for basic game data from server.
		//qDebug("PG_Data:");
		SendGameData();
		break;

	case DtSceneServerPacket::PE_StartNewAvatar:	// Create first empire data sent to server.
		//qDebug("PE_StartNewAvatar:");
		result = 0;
		if(g_Au.IsSceneMaster())
		{
			// SceneMaster is the first step in establishing a nu empire.
			// If all goes well, the client will be deferred to the scene manager in charge of their nu home sector.
			quint8 proximity;	// 0=near, 1=far
			quint8 galaxy;		// GALAXY
			quint8 scenario;
			AuId joinSectorId;

			block >> proximity;
			block >> galaxy;
			block >> scenario;
			block >> joinSectorId;

			result = EstablishSector(proximity, (GALAXY)galaxy, (SCENARIO)scenario, joinSectorId);
		}
		else if(g_Au.IsSceneManager())
		{
			// SceneManager is the second step in establishing a nu empire.
			// If all goes well, the client will be deferred to the Scene Worker in charge of their nu home solar system.
			AuId idSector;

			block >> idSector;

			result = EstablishHomeWorld(idSector);
		}
		else
		{
			// SceneWorker is the last step in establishing a nu empire.
			// If all goes well, we will return to the player their nu atom's id ready for play.
			AuId idHomeWorld;
			QByteArray baAvatar;
			block >> idHomeWorld;
			block >> baAvatar;

			// Extract avatar data from the byte array.
			QString charname;
			quint8 chroma;
			quint8 hand;
			AuDNABase* dna;
			quint8 scenario;
			AuTime asteroidImpact;
			quint32 joinEmpire;
			QByteArray joinPassCrypt;
			AuId joinSectorId;

			{
				QDataStream in(baAvatar);
				in.setVersion(QDataStream::Qt_5_7);
				in >> charname;
				in >> chroma;
				in >> hand;
				dna = AuDNABase::ReadDynamic(in);
				in >> scenario;
				in >> asteroidImpact;
				in >> joinEmpire;
				in >> joinPassCrypt;
				in >> joinSectorId;
			}

			if(dna)
			{
				result = CreateAvatar(charname, dna, (AuDNABase::CHROMATID)chroma, (HANDEDNESS)hand, idHomeWorld,
						(SCENARIO)scenario, asteroidImpact, joinEmpire, joinPassCrypt);
				delete dna;
			}
			else
				result = 21;
		}
		if(result)
		{
			// Failure.
			if(result != 15 && result != 16)
				qWarning() << "warning: DtSceneServer::ReadPacket PE_StartNewAvatar failed result" << result;
			// Send a message to the client indicating failure.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtSceneServerPacket::PE_EstError;
			out << result;
			SendPacket(ba);
		}
		break;

	case DtSceneServerPacket::PE_DeletePerson:
		//qDebug("PE_DeletePerson:");
		if(g_Au.IsSceneMaster())
		{
			// SceneMaster is the only server that gets to do this. Helps to reduce the possibility of hackers.
			AuId personId;
			personId << block;
			DeletePerson(personId);
		}
		break;

	case DtSceneServerPacket::PG_Empire:	// Client req for a specific empire record.
		//qDebug("PG_Empire:");
		if(g_Au.IsSceneMaster() || g_Au.IsSceneWorker())
		{
			quint32 empireId;
			block >> empireId;
			if(empireId)
				SendEmpireData(empireId);
		}
		else
		{
			g_Au.LogConnect(auSceneServerApp->LoginName(),
					QString("SCENE:%99 ERROR! Player %1 unit %2 %3: Unexpected TCP packet type %4 size %5 from %6. Hacker?")
					.arg(m_idPlayer)
					.arg(m_idUnit.GetID())
					.arg(m_UnitName)
					.arg(type)
					.arg(ba.count())
					.arg(tcpSocket.peerAddress().toString())
					.arg(m_Id));
			m_BadPackets++;
		}
		break;

	case DtSceneServerPacket::PG_EmpiresAll:	// Client req for all empire records.
		//qDebug("PG_Empire:");
		if(g_Au.IsSceneMaster() || g_Au.IsSceneWorker())
		{
			SendAllEmpires();
		}
		else
		{
			g_Au.LogConnect(auSceneServerApp->LoginName(),
					QString("SCENE:%99 ERROR! Player %1 unit %2 %3: Unexpected TCP packet type %4 size %5 from %6. Hacker?")
					.arg(m_idPlayer)
					.arg(m_idUnit.GetID())
					.arg(m_UnitName)
					.arg(type)
					.arg(ba.count())
					.arg(tcpSocket.peerAddress().toString())
					.arg(m_Id));
			m_BadPackets++;
		}
		break;

	case DtSceneServerPacket::PG_EmpiresJoinable:	// Client req for all joinable empire records.
		//qDebug("PG_Empire:");
		if(g_Au.IsSceneMaster() || g_Au.IsSceneWorker())
		{
			SendJoinableEmpires();
		}
		else
		{
			g_Au.LogConnect(auSceneServerApp->LoginName(),
					QString("SCENE:%99 ERROR! Player %1 unit %2 %3: Unexpected TCP packet type %4 size %5 from %6. Hacker?")
					.arg(m_idPlayer)
					.arg(m_idUnit.GetID())
					.arg(m_UnitName)
					.arg(type)
					.arg(ba.count())
					.arg(tcpSocket.peerAddress().toString())
					.arg(m_Id));
			m_BadPackets++;
		}
		break;

	case DtSceneServerPacket::PL_Setting:
		{
			quint8 whichSetting;
			block >> whichSetting;
			switch(whichSetting)
			{
			case 0:	// afk
				block >> m_AFK;
				break;
			}
		}
		break;

	case DtSceneServerPacket::P_Invalid:
	case DtSceneServerPacket::PE_EstDeferred:
	case DtSceneServerPacket::PE_EstError:
	case DtSceneServerPacket::PE_SceneDeferred:
	case DtSceneServerPacket::PE_Success:
	case DtSceneServerPacket::PG_Commodity:
	case DtSceneServerPacket::PG_Industry:
	case DtSceneServerPacket::PG_ManufacturingComp:
	case DtSceneServerPacket::PG_ManufacturingProc:
	case DtSceneServerPacket::PI_Control:
	case DtSceneServerPacket::PL_LoginFailure:
#if defined(QT_NO_DEBUG)
	default:				// No default in debug build for compiler warnings. Compiler warnings wanted in release build.
#endif
		g_Au.LogConnect(auSceneServerApp->LoginName(), QString("SCENE:%99 DISCONNECT! Player %1 unit %2 %3: Unknown TCP packet type %4 size %5 from %6. Hacker?")
			.arg(m_idPlayer).arg(m_idUnit.GetID()).arg(m_UnitName).arg(type).arg(ba.count())
			.arg(tcpSocket.peerAddress().toString()).arg(m_Id));
		m_BadPackets++;
		Disconnect();
		break;
	}
}

// Used by the client to send an array of bytes to the server.
void DtSceneServer::SendPacket(const QByteArray& ba, bool flush, bool usemutex)
{
	// The packet is already formatted. Add it to the transmit data stream.
	//qDebug("DtSceneServer::SendPacket - tx %d bytes", ba.count());
	if(usemutex)
	{
		// Most callers will use the mutex.
		m_muTxBuffer.lock();
		AuUtil::WritePacketSize(m_TxBuffer, ba.size());
		m_TxBuffer.append(ba);
		if(flush)
			m_TxBufferFlush = true;
		m_muTxBuffer.unlock();
	}
	else
	{
		// Don't use the mutex only if you've already locked it. SyncClient does
		// this so its stuff will be an uninterrupted stream in the buffer.
		AuUtil::WritePacketSize(m_TxBuffer, ba.size());
		m_TxBuffer.append(ba);
		if(flush)
			m_TxBufferFlush = true;
	}
}

void DtSceneServer::IdleClient()
{
	if(m_idSystem.IsNull())
		return;
	AuId systemId(m_idSystem);
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSolarSystem* system = auSceneServerApp->AssignedSolarSystem(systemId, ether);
	if(system)
	{
		DaPerson* person = dynamic_cast<DaPerson*>(system->FindIdRecursive(m_idUnit));
		if(person)
			person->SetOnLine(m_AccountType, SyncAveragePing(), AuTime::Now());
	}
	g_Au.UnlockEtheriumDelete();
}

void DtSceneServer::SyncClient()
{
	// Don't flood the client with syncs if they are falling behind.
	m_rwSync.lockForRead();
	quint32 syncCount = m_SyncCount;
	m_rwSync.unlock();
	m_rwSyncLast.lockForRead();
	quint32 syncLastCount = m_SyncLastCount;
	m_rwSyncLast.unlock();

	// The time we started doing the sync is good enough to track updates. At worst, we might unnecessarily send an atom twice
	// because it got updated after we grabbed this time but before the sync loop actually tx'd the atom. That is preferable to
	// hitting the system clock just before sending each and every atom.
	AuTime syncStartTime(AuTime::Now());
	AuElapsedTimer syncStartTimer(AuElapsedTimer::Now());

	// Locate the scene atom and their solar system in the atom data.
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaPerson* pSceneAtom = 0;
	DaSolarSystem* system = 0;
	if(!m_idSystem.IsNull())
	{
		// After first time through, look for the last solar system the scene atom was known to occupy.
		system = auSceneServerApp->AssignedSolarSystem(m_idSystem, ether);
		if(system)
			pSceneAtom = dynamic_cast<DaPerson*>(system->FindIdRecursive(m_idUnit));
		if(!pSceneAtom)
			m_idSystem = AuId();	// Person is not in the solar system any longer.
	}
	else
	{
		// First time through.
		pSceneAtom = dynamic_cast<DaPerson*>(ether.FindNormalAtom(m_idUnit));
		if(pSceneAtom)
		{
			// The scene atom was found in the local atom data.
			system = pSceneAtom->FindParentSolarSystem();	// Locate their current solar system.
			if(system && !system->IsOnlyStarFieldLoaded())
				m_idSystem = system->Id();					// The system is usable if it has loaded more than just star field data.
		}
	}

	if(m_idSystem.IsNull())
	{
		// The scene solar system changed or it could not be found.
		if(!pSceneAtom)
		{
			QString logmsg = QString("SCENE:%99 DtSceneServer::SyncClient person %1 atom not found")
				.arg(m_idUnit.GetID()).arg(m_Id);
			g_Au.LogConnect(auSceneServerApp->LoginName(), logmsg);
		}
		else
		{
			// Update on line status of scene atom one last time before we send them on their way.
			pSceneAtom->SetOnLine(m_AccountType, SyncAveragePing(), syncStartTime);	// On-line status decays in DaPerson::Tick.

			if(!system)
			{
				QString logmsg = QString("SCENE:%99 DtSceneServer::SyncClient person %1 %2 parent solar system not found..."
						"deferring").arg(pSceneAtom->Id().GetID()).arg(pSceneAtom->AssignedName()).arg(m_Id);
				g_Au.LogConnect(auSceneServerApp->LoginName(), logmsg);
			}
			else
			{
				QString logmsg = QString("SCENE:%99 DtSceneServer::SyncClient person %1 %2 parent solar system changed..."
						"deferring").arg(pSceneAtom->Id().GetID()).arg(pSceneAtom->AssignedName()).arg(m_Id);
				g_Au.LogConnect(auSceneServerApp->LoginName(), logmsg);
			}
		}

		g_Au.UnlockEtheriumDelete();
		m_idSystem = AuId(0);	// Current system id is 0.
		DeferToMaster();
		return;
	}

	Q_ASSERT(pSceneAtom);
	Q_ASSERT(system);

	// Update on line status of scene atom right away in case we decide to ret for some reason.
	pSceneAtom->SetOnLine(m_AccountType, SyncAveragePing(), syncStartTime);	// On-line status decays in DaPerson::Tick.
	system->SetLastPersonScene();								// System needs the last time it was visited by an on-line person.

	// Update information about the current scene atom.
	m_idUnitEmpire = pSceneAtom->EmpireId();
	m_UnitAgentBits = pSceneAtom->m_AgentBits;
	const DgData& gd = g_Au.LockDataForRead();
	AuId emperorId = gd.EmpireEmperor(m_idUnitEmpire);
	g_Au.UnlockData();
	m_isUnitNewbie = pSceneAtom->Is(emperorId) && pSceneAtom->IsNewbieEmperor();

	// We wait until now to ret for this reason so we can set the on line time on the scene atom.
	if(syncCount - syncLastCount >= 2)
	{
		// Client has fallen behind sending sync complete's.
		g_Au.UnlockEtheriumDelete();
		return;
	}

	if(pSceneAtom->IsAwaitingSpawn())
	{
		// The scene atom is waiting to spawn elsewhere.
		QString logmsg = QString("SCENE:%99 DtSceneServer::SyncClient person %1 %2 awaiting spawn...deferring")
			.arg(pSceneAtom->Id().GetID()).arg(pSceneAtom->AssignedName()).arg(m_Id);
		g_Au.LogConnect(auSceneServerApp->LoginName(), logmsg);
		g_Au.UnlockEtheriumDelete();
		DeferToMaster();
		return;
	}

	// Don't send scene sync when parent atom is a unit in the sun safe zone. It will be repositioned eventually.
	const DaVessel* parentUnit = dynamic_cast<const DaVessel*>(pSceneAtom->Parent());
	if(parentUnit && parentUnit->IsInSunSafeZone())
	{
		//qDebug() << "DtSceneServer::SyncClient parentUnit" << parentUnit->Id() << parentUnit->Name() << "is in sun safe zone"
		//<< parentUnit->EnvLocation().Length();
		g_Au.UnlockEtheriumDelete();
		return;
	}

	// Locate scene atom's sector in the assigned data.
	DaSector* personSector = (DaSector*)system->FindParentSector();
	if(!personSector)
	{
		// Solar system does not have a parent sector? Extremely unlikely.
		QString logmsg = QString("SCENE:%99 DtSceneServer::SyncClient person %1 %2 parent sector not found...deferring")
			.arg(pSceneAtom->Id().GetID()).arg(pSceneAtom->AssignedName()).arg(m_Id);
		g_Au.LogConnect(auSceneServerApp->LoginName(), logmsg);
		g_Au.UnlockEtheriumDelete();
		DeferToMaster();
		return;
	}

	// Get a list of the atoms that we need to send to the client.
	DaAtomList sceneAtoms;
	if(!pSceneAtom->ListSceneAtoms(sceneAtoms))
	{
		// No scene path for scene atom.
		QString logmsg = QString("SCENE:%99 DtSceneServer::SyncClient person %1 %2 no scene path...deferring")
			.arg(pSceneAtom->Id().GetID()).arg(pSceneAtom->AssignedName()).arg(m_Id);
		g_Au.LogConnect(auSceneServerApp->LoginName(), logmsg);
		g_Au.UnlockEtheriumDelete();
		DeferToMaster();
		return;
	}

	// We remember the parent sector of the scene atom for reference when we sync the next scene.
	// This way we can tell if the parent sector has changed between refreshes.
	m_rwSync.lockForWrite();
	if(personSector->Id() != m_SyncSector)
	{
		// The current sector is not the same sector as the last scene sync.
		// The star field is needed by the client.
		m_SyncSector = personSector->Id();
		m_SyncGalaxy = personSector->Galaxy();
		personSector->ListStarFieldSubatoms(pSceneAtom, sceneAtoms);
		//qDebug("DtSceneServer::SyncClient person %d %s scene with starfield, %d total atoms",
				//pSceneAtom->Id().GetID(), (const char*)pSceneAtom->AssignedName().toAscii(), sceneAtoms.count());
	}

	// We will lock the transmit buffer now and hold on to it until we're done.
	// That way the sync is completed in one uninterrupted stream.
	m_muTxBuffer.lock();
	int bufstartsz = m_TxBuffer.count();

	// Transmit the atoms to the client.
	unordered_map<AuId,bool, AuIdHash> atomsInScene;			// Non-permanent atoms that belong in the scene.
	unsigned int numsync = 0;
	for(DaAtomList::const_iterator it = sceneAtoms.begin(); it != sceneAtoms.end(); it++)
	{
		DaAtom* pa = *it;

		// Reset atom scene flag when it appears in a scene.
		pa->SetInScene();

		quint32 empireId = pa->EmpireId();
		if(empireId)
		{
			// Monitor for units of other empires so they can become "known" to the scene atom's empire.
			DaUnit* unit = dynamic_cast<DaUnit*>(pa);
			if(unit)
			{
				// The atom is a unit with an empire id.
				if(!IsEmpireInScene(empireId))
				{
					// Empire id does not exist in the list of empires in our scene.
					// Test political stance at least once with all empires we encounter, to make contact.
					pSceneAtom->PoliticalStanceMakeContact(unit);

					// Add the unit's empire id to the list of empires in the scene and the list of empires we've made contact with.
					m_rwAtomEmpires.lockForWrite();
					m_AtomEmpires.insert(pair<quint32, bool>(empireId, true));
					m_AtomStances.insert(pair<quint32, bool>(empireId, true));
					m_rwAtomEmpires.unlock();
				}
				else if(!HasEmpireStance(empireId))
				{
					// Test political stance at least once with all empires we encounter, to make contact.
					pSceneAtom->PoliticalStanceMakeContact(unit);

					// Add the unit's empire id to the list of empires we've made contact with.
					m_rwAtomEmpires.lockForWrite();
					m_AtomStances.insert(pair<quint32, bool>(empireId, true));
					m_rwAtomEmpires.unlock();
				}
			}
			else
			{
				// The atom is not a unit but it has an empire id, most likely a world that was claimed. Client needs the empire.
				if(!IsEmpireInScene(empireId))
				{
					// Add the world's empire id to the list of empires in the scene.
					m_rwAtomEmpires.lockForWrite();
					m_AtomEmpires.insert(pair<quint32, bool>(empireId, true));
					m_rwAtomEmpires.unlock();
				}
			}
		}

		// Determine when we last updated the atom, if ever.
		AuAtomSync atomSync;
		QHash<AuId, AuAtomSync>::const_iterator si = m_AtomSync.find(pa->Id());
		if(si != m_AtomSync.cend())
		{
			// We sync'd the atom once.
			// Check modify time from atom.
			atomSync = *si;
			if(atomSync.m_Time > pa->ModifyTime())
			{
				// The atom hasn't changed so no need to sync it.
				// It still needs to be listed among the atoms that are supposed to be in the scene.
				if(!pa->IsPermanentInScene())
					atomsInScene.insert(pair<AuId, bool>(pa->Id(), true));
				continue;
			}
		}
		if(pa->IsPermanentInScene())
			m_PermanentAtoms.insert(pair<AuId, bool>(pa->Id(), true));
		else
			atomsInScene.insert(pair<AuId, bool>(pa->Id(), true));

		// Make a packet out of each atom.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)pa->AtomType();							// ATOM_TYPE overlaps initial range of packet types.
		pa->TxUpdate(out, atomSync.m_Time, atomSync.m_Timer, *pSceneAtom);	// Atom does the rest.
		SendPacket(ba, false, false);							// No mutex lock because it's already locked. This insures the
																// entire sync is not interrupted by other packets.
		numsync++;

		// Update the atom's last sync time in the hash with the time we started this sync. That is the time sent to its TxUpdate
		// function as the last sync time.
		QHash<AuId, AuAtomSync>::iterator asi = m_AtomSync.find(pa->Id());
		if(asi == m_AtomSync.end())
			m_AtomSync.insert(pa->Id(), AuAtomSync(syncStartTime, syncStartTimer));
		else
			*asi = AuAtomSync(syncStartTime, syncStartTimer);
	}

	// We're done referencing atoms by pointer at this point.
	g_Au.UnlockEtheriumDelete();

	// Remove non-permanent atoms from our atom sync hash that are no longer in the scene.
	QHash<AuId, AuAtomSync>::iterator asi = m_AtomSync.begin();
	while(asi != m_AtomSync.end())
	{
		if(m_PermanentAtoms.find(asi.key()) == m_PermanentAtoms.end() &&	// Atom is not permanent in the scene.
				atomsInScene.find(asi.key()) == atomsInScene.end())		// Atom is not non-permanent in the scene.
			asi = m_AtomSync.erase(asi);	// Remove the non-permanent atom from the scene sync. Next time will be a fresh send.
		else
			asi++;
	}

	// Send a sync termination packet.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtSceneServerPacket::P_SyncComplete;	// Initial sync is complete
	syncStartTime.Tx32(out);		// Send my time to the client.
	if(atomsInScene.empty())
		out << (quint8)0;			// No non-permanent atoms in the scene?
	else if(atomsInScene == m_SyncLastAtomIds)
		out << (quint8)2;			// Reuse previously sent list of non-permanent atom ids.
	else
	{
		// Send non-permanent atom ids that belong in the scene.
		out << (quint8)1;			// Non-permanent scene atom ids list follows.
		out << (quint16)atomsInScene.size();
		for(unordered_map<AuId,bool, AuIdHash>::const_iterator uai = atomsInScene.cbegin(); uai != atomsInScene.cend(); uai++)
			out << (*uai).first;
	}
	SendPacket(ba, false, false);		// No mutex lock because it's already locked. This insures that the entire sync is not
										// interrupted by other packets.

	// This is the end of the scene sync data stream.
	int bufendsz = m_TxBuffer.count();
	m_muTxBuffer.unlock();

	// Remember statistics
	m_SyncLastAtomIds = atomsInScene;
	unsigned int syncsize = bufendsz - bufstartsz;				// bytes in this refresh
	m_SyncCount++;												// total refreshes so far
	quint32 syncLastIndex = m_SyncCount%c_SyncHistory;
	m_SyncLastAtoms[syncLastIndex] = numsync;					// atoms in this refresh
	m_SyncLastBytes[syncLastIndex] = syncsize;					// size of this refresh in bytes
	m_SyncTotalAtoms += numsync;								// total atoms sent so far
	m_SyncTotalBytes += syncsize;								// total bytes sent so far

	if(!m_ContactShip.IsNull() && m_SyncCount >= 100)
	{
		// Have their ship contact them.
		AuCommContactShip msg(m_idUnitEmpire, m_ContactShip, m_idUnit, m_UnitName, m_SyncSector, m_SyncGalaxy);
		m_ContactShip = AuId();	// We only do this once.

		// Bundle the message into a byte array so it can be emitted as a signal.
		QByteArray bamsg;
		QDataStream out(&bamsg, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)msg.Type();
		out << msg;
		emit ChatReceived(bamsg);
	}
	m_rwSync.unlock();
}

void DtSceneServer::DeferClient()
{
	if(!m_tmAtomQuery.isValid())
	{
		// The defer time is null so start the defer process.
		//qDebug("DtSceneServer::DeferClient started for unitId %d", m_idUnit.GetID());
		m_tmAtomQuery.start();

		// Emit a signal to the app so it will try to locate our unit for us.
		m_QueryReason = QUERY_Defer;
		m_AtomQuery = m_idUnit;
		m_EmpireQuery = 0;
		emit DeferClient(m_idUnit);	// Defer to server with my unit.
	}
	else if(m_tmAtomQuery.elapsed()/d_MSecPerSec > 15)
	{
		// We should have received a result by now.
		QString logmsg = QString("SCENE:%99 DtSceneServer::DeferClient timed out for unitId %1 %2, deferring to master")
			.arg(m_idUnit.GetID()).arg(m_UnitName).arg(m_Id);
		g_Au.LogConnect(auSceneServerApp->LoginName(), logmsg);
		DeferToMaster();
	}

	// Still waiting for result of DeferClient signal, which should be a call to my AtomPresence function.
}

void DtSceneServer::DeferToMaster()
{
	//qDebug("DtSceneServer::DeferToMaster");
	m_QueryReason = QUERY_Defer;
	QString netName = g_Au.SceneMasterName();
	quint16 port = AuUtil::SceneMasterPort();
	QString peerName = g_Au.SceneMasterName();
	AtomPresence(netName, port, peerName);
}

void DtSceneServer::AtomPresence(const QString& netName, quint16 port, const QString& peerName)
{
	//qDebug("DtSceneServer::AtomPresence");
	if(m_QueryReason == QUERY_None)
	{
		// Nothing to do if we're not waiting for a query result.
		return;
	}
	else if(m_QueryReason == QUERY_Defer)
	{
		// Defer the client to the subordinate server controlling the region of
		// space that contains the person's unit, or to the scene master if no
		// subordinate server claimed to have the unit.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PE_SceneDeferred;
		if(m_LocalClient)
			out << peerName;
		else
			out << netName;
		out << port;
		quint8 srvmode;
		if(netName == g_Au.SceneMasterName() && port == AuUtil::SceneMasterPort())
			srvmode = 0;	// deferred up to master
		else if(g_Au.IsSceneMaster())
			srvmode = 1;	// master defers to managers
		else if(g_Au.IsSceneManager())
			srvmode = 2;	// manager defers to workers
		else
			srvmode = 1;	// I am a worker and the dest isn't the master so
		out << srvmode;		// the destination could only be a manager.
		SendPacket(ba);

		QString logmsg = QString("SCENE:%99 Disconnect. Player %2: Deferred to %3:%4 for scene sync, srvmode %5.")
				.arg(m_idPlayer).arg(m_LocalClient ? peerName : netName).arg(port).arg(srvmode).arg(m_Id);
		g_Au.LogConnect(auSceneServerApp->LoginName(), logmsg);
		Disconnect();
	}

	// No longer awaiting atom query.
	m_QueryReason = QUERY_None;
}

void DtSceneServer::SendGameData()
{
	//qDebug("DtSceneServer::SendGameData");
	const DgData& gd = g_Au.LockDataForRead();
	DbCommodityArray ctypes = g_Au.m_CommodityArray;
	DbIndustryArray inds = gd.m_bIndustryArray;
	DbManufacturingProcessArray mprocs = gd.m_bManufacturingProcessArray;
	DbManufacturingComponentArray mcomps = gd.m_bManufacturingComponentArray;
	g_Au.UnlockData();

	//qDebug("DtSceneServer::SendGameData - %d govt types", gtypes.count());

	m_muTxBuffer.lock();

	// Commodities
	for(DbCommodityArray::const_iterator cit = ctypes.cbegin(); cit != ctypes.cend(); cit++)
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PG_Commodity;
		(*cit).Tx(out);
		SendPacket(ba, false, false);
	}

	// Industries
	for(DbIndustryArray::const_iterator iit = inds.cbegin(); iit != inds.cend(); iit++)
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PG_Industry;
		(*iit).Tx(out);
		SendPacket(ba, false, false);
	}

	// Manufacturing processes
	for(DbManufacturingProcessArray::const_iterator mpit = mprocs.cbegin(); mpit != mprocs.cend(); mpit++)
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PG_ManufacturingProc;
		(*mpit).Tx(out);
		SendPacket(ba, false, false);
	}

	// Manufacturing processes
	for(DbManufacturingComponentArray::const_iterator mcit = mcomps.cbegin(); mcit != mcomps.cend(); mcit++)
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PG_ManufacturingComp;
		(*mcit).Tx(out);
		SendPacket(ba, false, false);
	}

	// Tag the end of the stream so client can proceed.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtSceneServerPacket::PG_Data;
	SendPacket(ba, true, false);

	m_muTxBuffer.unlock();
}

// 'idEmpire' identifies the empire data to send.
void DtSceneServer::SendEmpireData(quint32 idEmpire)
{
	//qDebug("DtSceneServer::SendEmpireData empireId %d", idEmpire);
	// Send a specific empire's data.
	DgEmpire empire;
	empire.m_id = idEmpire;
	const DgData& gd = g_Au.LockDataForRead();
	bool hasEmpire = gd.Empire(empire);
	g_Au.UnlockData();

	bool lastEmpire = true;
	if(hasEmpire)
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PG_Empire;
		empire.Tx(out, m_idUnitEmpire == empire.m_id);			// When sending the person's empire, include the announcements.

		// Make an SHA-1 key from the packet so we can remember if we sent it. They get big and repetitive, often with no changes.
		QCryptographicHash passCryptHash(QCryptographicHash::Sha1);
		passCryptHash.addData(ba);
		QByteArray bsha(passCryptHash.result());

		// This dup check code originated because of a bug that caused empire record change notifications frequently when no
		// change actually happened. I left it here just to let it notify of similar problems in the future.
		bool aDup = false;							// true if its a duplicate empire record to the last one we sent to them.
		m_EmpiresSentLock.lock();
		QHash<quint32, QByteArray>::iterator si = m_EmpiresSent.find(idEmpire);
		if(si != m_EmpiresSent.end())
		{
			// We already sent the empire to the player. Do the SHA-1 keys match?
			if(*si != bsha)
				*si = bsha;							// Keys don't match. Update the key and proceed with sending the record.
			else
				aDup = true;						// Key is a duplicate of the last one we sent. Don't send it again.
		}
		else
			m_EmpiresSent.insert(idEmpire, bsha);	// Save the SHA-1 key of the empire packet and proceed with sending it.
		m_EmpiresSentLock.unlock();

		// Send the empire record if it is not a duplicate of the last empire record we sent to them.
		if(!aDup)
		{
			out << lastEmpire;
			SendPacket(ba);
		}
		//else if(m_idPlayer == 98)	// Me
			//qDebug() << "DtSceneServer::SendEmpireData" << idEmpire << empire.Name()
				//<< "not sending duplicate empire" << ba.size() << "bytes";
	}
	else
	{
		// We must respond if the empire does not exist at all or the client will keep requesting it.
		DgEmpire bogusEmpire(QString("Forgotten Empire %1").arg(idEmpire));
		bogusEmpire.m_id = idEmpire;
		bogusEmpire.m_flag.load(":data/BogusEmpireFlag.png");

		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PG_Empire;
		bogusEmpire.Tx(out, false);
		out << lastEmpire;
		SendPacket(ba);
	}
}

void DtSceneServer::SendAllEmpires()
{
	//qDebug("DtSceneServer::SendAllEmpires");
	// Decide what empire records we are going to send them, skipping the ones we already sent.
	QHash<quint32, QByteArray> empiresToSend;	// Data is the PG_Empire packet, without the lastEmpire bool.
	//quint32 notSent = 0;
	m_EmpiresSentLock.lock();
	const DgData& gd = g_Au.LockDataForRead();
	for(DgEmpireHash::const_iterator empIt = gd.m_gEmpires.cbegin(); empIt != gd.m_gEmpires.cend(); empIt++)
	{
		// Make the PG_Empire packet, without the final lastEmpire byte.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PG_Empire;
		(*empIt).second.Tx(out, m_idUnitEmpire == (*empIt).second.m_id);	// When sending person's empire, include more.

		// Make an SHA-1 key so we can remember if we sent it. They get big and repetitive, often with no changes.
		QCryptographicHash passCryptHash(QCryptographicHash::Sha1);
		passCryptHash.addData(ba);
		QByteArray bsha(passCryptHash.result());

		// Check to see if the key matches the last one we sent.
		bool aDup = false;							// true if its a duplicate empire record to the last one we sent to them.
		QHash<quint32, QByteArray>::iterator si = m_EmpiresSent.find((*empIt).second.m_id);
		if(si != m_EmpiresSent.end())
		{
			// We already sent the empire to the player. Do the SHA-1 keys match?
			if(*si != bsha)
				*si = bsha;							// Keys don't match. Update the key and proceed with sending the record.
			else
				aDup = true;						// Key is a duplicate of the last one we sent. Don't send it again.
		}
		else
			m_EmpiresSent.insert((*empIt).first, bsha);	// Save the SHA-1 key of the empire packet and proceed with sending it.
		if(!aDup)
			empiresToSend.insert((*empIt).second.m_id, ba);	// Save the packet that we built, to be sent.
		else
		{
			//notSent += ba.size();
			//if(m_idPlayer == 98)	// Me
				//qDebug() << "DtSceneServer::SendAllEmpires" << (*empIt).first << (*empIt).second.Name()
					//<< "not sending duplicate empire" << ba.size() << "bytes";
		}
	}
	if(empiresToSend.empty())
	{
		// Nothing is different from what we already sent them.
		// Send them any empire record, so they can get the "last empire record" signal to clean up UIs.
		DgEmpireHash::const_iterator empIt = gd.m_gEmpires.find(m_idUnitEmpire);
		if(empIt == gd.m_gEmpires.cend())
			empIt = gd.m_gEmpires.find(PIRATE_Zuul);	// They don't have an empire? Send a pirate.
		if(empIt != gd.m_gEmpires.cend())
		{
			// Make the PG_Empire packet, without the final lastEmpire byte.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtSceneServerPacket::PG_Empire;
			(*empIt).second.Tx(out, m_idUnitEmpire == (*empIt).second.m_id);	// When sending person's empire, include more.
			empiresToSend.insert((*empIt).second.m_id, ba);	// Save the packet that we built, to be sent.
		}
	}
	g_Au.UnlockData();
	m_EmpiresSentLock.unlock();
	if(empiresToSend.empty())
		return;			// There was nothing new to send.

	//if(m_idPlayer == 98 && notSent)
		//qDebug() << "DtSceneServer::SendAllEmpires total duplicates not sent" << notSent << "bytes";

	// Send the packets we decided to send.
	int empireIdx = 0;
	int lastIdx = empiresToSend.size() - 1;	// Index of last item in the empires hash.
	m_muTxBuffer.lock();
	for(QHash<quint32, QByteArray>::const_iterator si = empiresToSend.cbegin(); si != empiresToSend.cend(); si++, empireIdx++)
	{
		bool lastEmpire = (empireIdx == lastIdx) ? true : false;

		QByteArray ba(*si);
		//qDebug() << "preparing to send empire" << si.key() << "ba size" << ba.size();
		QDataStream out(&ba, QIODevice::WriteOnly|QIODevice::Append);
		out.setVersion(QDataStream::Qt_4_1);
		out << lastEmpire;
		//qDebug() << "sending empire" << si.key() << "lastEmpire" << lastEmpire << "final ba size" << ba.size();
		SendPacket(ba, true, false);							// Flush when done. We've already got the mutex locked.
	}
	m_muTxBuffer.unlock();
}

void DtSceneServer::SendJoinableEmpires()
{
	//qDebug("DtSceneServer::SendJoinableEmpires");
	const DgData& gd = g_Au.LockDataForRead();
	DgEmpireHash empArray;
	gd.ListEmpiresToJoin(empArray);
	g_Au.UnlockData();

	// Determine which empire will be the last empire.
	int empireIdx = 0;
	int lastIdx = empArray.size() - 1;	// Index of last item in the empires hash.
	m_muTxBuffer.lock();
	for(DgEmpireHash::const_iterator empIt = empArray.cbegin(); empIt != empArray.cend(); empIt++, empireIdx++)
	{
		bool lastEmpire = (empireIdx == lastIdx) ? true : false;

		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PG_Empire;
		(*empIt).second.Tx(out, m_idUnitEmpire == (*empIt).second.m_id);	// When sending person's empire, include announcements.
		out << lastEmpire;
		SendPacket(ba, true, false);							// Flush when done. We've already got the mutex locked.
	}
	m_muTxBuffer.unlock();
}

void DtSceneServer::ProcessControlInput(const AuId& toAtom, const DiControlInputArray& aActiveControls, const AuGearPos& pos,
		qint8 selValue)
{
	if(m_idSystem.IsNull())
		return;
	AuId systemId(m_idSystem);
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSolarSystem* system = auSceneServerApp->AssignedSolarSystem(systemId, ether);
	if(!system)
	{
		g_Au.UnlockEtheriumDelete();
		return;
	}

	DaAtom* pInputTo = system->FindIdRecursive(toAtom);
	if(pInputTo)
	{
		DaPerson* pScene;
		if(m_idUnit == toAtom)
			pScene = dynamic_cast<DaPerson*>(pInputTo);
		else
			pScene = dynamic_cast<DaPerson*>(system->FindIdRecursive(m_idUnit));
		if(pScene)
		{
			//qDebug("DtSceneServer::ProcessControlInput - %d controls to %d %s by person %d %s",
					//aActiveControls.count(),
					//toAtom.GetID(), (const char*)pInputTo->Name().toAscii(),
					//pScene->Id().GetID(), (const char*)pScene->AssignedName().toAscii());
			pInputTo->ProcessControlInput(aActiveControls, pScene, pos, selValue, CONTROL_Server, 0);
		}
		//else
			//qWarning("%x 'scene' atom not found",
					//(unsigned int)currentThreadId());
	}
	//else
		//qWarning("%x 'to' atom not found", (unsigned int)currentThreadId());
	g_Au.UnlockEtheriumDelete();
	//qDebug("DtSceneServer::ProcessControlInput to %d done", toAtom.GetID());
}

// First step of establishing a nu empire is performed on the scene master.
// Returns
//  0 = success, deferred to the appropriate scene manager
//  1 = player not authorized to create empires
//  4 = invalid proximity
//  5 = blank empire name
//  7 = invalid flag image
//  8 = invalid government
//  9 = empire with that name already exists
//  10 = universe missing
//  11 = galaxy missing
//  12 = failed to create nu sector
//  20 = invalid galaxy
unsigned char DtSceneServer::EstablishSector(unsigned char proximity, GALAXY galaxy, SCENARIO scenario, const AuId& joinSectorId)
{
	//Q_ASSERT(g_Au.IsSceneMaster());

	// Validate passed parameter values
	if(proximity > 8)
		return 4;		// invalid proximity value
	if(scenario != CitizenOfTheGalaxy && galaxy != ShoresOfHazeron)
		return 20;		// invalid start galaxy value

	quint8 result;
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaUniverse* au = ether.FindUniverse();
	if(au)
	{
		DaGalaxy* ag = au->FindGalaxy(galaxy);
		if(ag)
		{
			// Create a nu unexplored 3x3x3 cloud of sectors.
			AuIdList& nuSectors = auSceneServerApp->LockNewSectors();
			DaSector* asect;
			if(scenario == CitizenOfTheGalaxy)
				asect = ag->CreateJoinEmpireHomeWorldSector(joinSectorId, nuSectors);
			else
				asect = ag->CreateNewHomeWorldSector(proximity, nuSectors);
			auSceneServerApp->UnlockNewSectors();
			if(asect)
			{
				AuId idNewSector = asect->Id();
				QString sectorName(asect->Name());
				g_Au.UnlockEtheriumDelete();

				// Wait until the nu sector has been written to the database.
				int waiting = 0;
				bool isStillNew = true;
				do
				{
					if(!auSceneServerApp->IsSectorNew(idNewSector))
					{
						isStillNew = false;
					}
					else
					{
						msleep(500);
						waiting++;
						if(waiting%20 == 0 && (g_Au.m_LogMessages & LOG_Construction))
						{
							QString logmsg;
							logmsg = tr("%1 Player %2: Waiting for db thread to store nu home sector %3.")
								.arg((unsigned long)currentThreadId())
								.arg(m_idPlayer).arg(idNewSector.GetID());
							g_Au.Log(logmsg);
						}
					}
				}
				while(isStillNew);

				EstDeferred(idNewSector, idNewSector);

				if(g_Au.m_LogMessages & LOG_Construction)
				{
					QString logmsg;
					logmsg = tr("%1 Sector %2 established.").arg((unsigned long)currentThreadId()).arg(sectorName);
					g_Au.Log(logmsg);
				}
				return 0;	// root is already unlocked
			}
			else
				result = 12;
		}
		else
			result = 11;
	}
	else
		result = 10;
	g_Au.UnlockEtheriumDelete();
	return result;
}

// Second step in establishing a nu empire.
// Returns
//  0 = success
//  1 = player not authorized to create empires
//  2 = blank avatar name
//  3 = invalid sex
//  10 = universe missing
//  11 = galaxy missing
//  13 = failed to create a good home system
//  14 = failed to locate a habitable world
//  16 = failed to locate nu sector, client should try again
//  17 = idSector is not a sector
unsigned char DtSceneServer::EstablishHomeWorld(const AuId& idSector)
{
	//Q_ASSERT(g_Au.IsSceneManager());
	// Locate the sector. It should have been assigned to me already.
	quint8 result;
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSector* asect = dynamic_cast<DaSector*>(ether.FindNormalAtom(idSector));
	if(asect)
	{
		// Claim a system in the sector. This fetch guarantees we
		// receive a solar system that contains a habitable planet.
		DaSolarSystem* ass = asect->FindGoodHome();
		if(ass)
		{
			// Locate a homeworld within the system.
			ass->FinalizeContents();
			const DaWorld* pStartAt = ass->FindHabitableWorld(HABITABLE_HomeWorld);
			Q_ASSERT(pStartAt);
			if(pStartAt)
			{
				// Grab ids of atoms.
				AuId idSystem = ass->Id();
				AuId idHomeWorld = pStartAt->Id();
				EstDeferred(idSystem, idHomeWorld);
				if(g_Au.m_LogMessages & LOG_Construction)
				{
					QString logmsg;
					logmsg = tr("Homeworld established at %2 %3 system %4 %5 sector %6 %7. Player %1.")
						.arg(m_idPlayer)
						.arg(pStartAt->Id().GetID()).arg(pStartAt->Name())
						.arg(ass->Id().GetID()).arg(ass->Name())
						.arg(asect->Id().GetID()).arg(asect->Name());
					g_Au.Log(logmsg);
				}
				g_Au.UnlockEtheriumDelete();
				return 0;	// good return result
			}
			else
			{
				result = 14;
				if(g_Au.m_LogMessages & LOG_Error)
					qWarning() << "error: DtSceneServer::EstablishHomeWorld FindHabitableWorld failed. Player" << m_idPlayer
						<< "sector" << *asect << "system" << *ass;
			}
		}
		else
		{
			// A homeworld system was not found in the designated sector.
			result = 13;	// This result triggers the client to resend the new emperor info to the master.
			if(g_Au.m_LogMessages & LOG_Warning)
				qWarning() << "warning: DtSceneServer::EstablishHomeWorld FindGoodHome failed. Player" << m_idPlayer
					<< "sector" << *asect << "Deferring them to try again.";
		}
	}
	else
	{
		// The nu sector was not located. The client should try again when it
		// gets this response because we will have it loaded eventually.
		result = 16;
	}
	g_Au.UnlockEtheriumDelete();
	return result;
}

// Final step in establishing a first empire.
// Returns
//  0 = success
//  1 = player not authorized to create empires
//  2 = blank avatar name
//  3 = invalid sex
//  10 = universe missing
//  11 = galaxy missing
//  15 = failed to locate assigned homeworld
//  18 = idHomeWorld is not a planet
//  19 = dna is invalid for player avatar
//  21 = invalid overlord
//  22 = joinPassCrypt password incorrect
quint8 DtSceneServer::CreateAvatar(const QString& charname, const AuDNABase* dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand,
		const AuId& idHomeWorld, SCENARIO scenario, const AuTime& asteroidImpact, quint32 joinEmpire,
		const QByteArray& joinPassCrypt)
{
	if(!dna)
		return 19;

	m_idUnit = AuId(0);		// Init to null until successful.
	// Clear the unit name.
	m_muUnitName.lock();
	m_UnitName = "";
	m_muUnitName.unlock();

	// Validate passed parameter values
	if(!IsNameValid(charname))
		return 2;		// Name must be valid. Need not be unique.
	if(chroma != AuDNABase::Alpha && chroma != AuDNABase::Beta)
		return 3;

	if(scenario == CitizenOfTheGalaxy)
	{
		// Verify password for joining the empire.
		const DgData& gd = g_Au.LockDataForRead();
		bool restricted = gd.EmpireJoinRestricted(joinEmpire);
		bool passOk = gd.EmpireTestJoinPassCrypt(joinEmpire, joinPassCrypt);
		g_Au.UnlockData();
		if(restricted && !passOk)
			return 22;
	}

	// Check validity of body dna.
	if(dna->Version() == 3)
	{
		const DhDNA* dna3 = (const DhDNA*)dna;
		DpBody body(chroma == AuDNABase::Alpha ? Male : Female, hand == LeftHanded, *dna3);
		if(!body.IsValidForPerson())
		{
			qDebug() << "DtSceneServer::CreateEmperor player" << m_idPlayer
				<< "name" << charname << (chroma==AuDNABase::Alpha?"M":"F") << (hand == LeftHanded ?"Left-Handed":"Right-Handed")
				<< "DNA NOT VALID FOR PERSON BODY";
			qDebug() << " IsAbilityPointsValidForPerson" << body.IsAbilityPointsValidForPerson() << body.AbilityPointsUsed();
			qDebug() << " IsHitPointsValidForPerson" << body.IsHitPointsValidForPerson() << body.MaxHitPoints();

			if(dna3->BodyCarriage == 0 || dna3->BodyCarriage == 1)
				body.Pose(AuPose::Crouch, false, 1);
			else
				body.Pose(AuPose::Stand, false, 1);
			qDebug() << " IsCgHeightValidForPerson standing" << body.IsCgHeightValidForPerson()
				<< (body.CGHeight(true));
			qDebug() << " IsCameraHeightValidForPerson standing" << body.IsCameraHeightValidForPerson()
				<< (body.CGHeight(true) + body.CameraLocation().z);
			qDebug() << " IsHeadHeightValidForPerson standing" << body.IsHeadHeightValidForPerson()
				<< (body.CGHeight(true) + body.HeadLocation().z);

			body.Pose(AuPose::ConsoleCommand, false, 1);
			qDebug() << " IsCameraHeightValidForPerson at console" << body.IsCameraHeightValidForPerson()
				<< (body.CGHeight(true) + body.CameraLocation().z);
			qDebug() << " IsHeadHeightValidForPerson at console" << body.IsHeadHeightValidForPerson()
				<< (body.CGHeight(true) + body.HeadLocation().z);

			return 19;
		}
	}
	else
	{
		//DHH animalia
	}

	quint8 result = 0;
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaUniverse* au = ether.FindUniverse();
	if(au)
	{
		// Locate the homeworld. It should have been assigned to me already.
		DaWorldGlobe* pStartAt = dynamic_cast<DaWorldGlobe*>(ether.FindNormalAtom(idHomeWorld));
		if(pStartAt)
		{
			// Get id of solar system.
			AuTime now(AuTime::Now());
			DaSolarSystem* system = pStartAt->FindParentSolarSystem();
			if(system)
			{
				m_idSystem = system->Id();

				// Start the doomsday asteroid.
				AuPoint startAt = AuParticleSystem::GetRandomVector();

				// Calculate the distance so normal speed gets the asteroid to the target on time. The asteroid continuously
				// modulates its speed to arrive at the target on time. This calc will make it modulate to a speed close to
				// normal asteroid speed.
				quint32 minutesToImpact = now.MinsTo(asteroidImpact);
				double distance = minutesToImpact * 60 * DaAsteroid::TargetPursueSpeed();
				if(distance > FeetPerParsec()/2)
					distance = FeetPerParsec()/2;	// Don't fall into the next solar system.
				startAt.Unit(distance);
				AuMatrix4x4 t;
				t.Translate(startAt);

				// Calculate release time so asteroid comes screaming in at high speed, instead of crawling. Slow asteroids are
				// boring. Too fast and they strike too quickly to have any fun.
				qint64 secsTravel = (qint64)(distance / DaAsteroid::TargetPursueSpeed());	// Time to travel the distance at TPS.
				AuTime timeOfRelease = asteroidImpact.AddSecs(-secsTravel);	// If earlier than now, the asteroid will start now.

				DaAsteroid* roid = new DaAsteroid(pStartAt->Id(), asteroidImpact, timeOfRelease);
				roid->ConnectSignals();
				roid->Create();
				roid->SetPosAndOrient(AuAtomPos(system->Id(), t));
				ether.AddAtom(roid);

				g_Au.AtomAdded(roid->Id());
				roid->SetNormal();	// New atom is ready for normal thread processing.
				roid->TickTock();
			}

			// Grab name of starting planet for log message.
			QString startname = pStartAt->Name();

			// Create the nu person unit. Emperor.
			DaPerson* ape = new DaPerson(*dna, chroma, hand, charname, Mature, 0);
			ape->ConnectSignals();

			if(AcctIsArchitect(m_AccountType) && charname == "Haxus")
			{
				// When creating Haxus, assign a predefined atom id. It has been reserved for this purpose.
				ape->SetId(AuId::Haxus());
				ape->SetCreatedTime(AuTime::Now());
				ape->SetModifyTime(AuTime::Now());
			}
			else
				ape->Create();

			ape->SetInitScenario(scenario);
			ape->InitNewAvatar(pStartAt);

			m_idUnit = ape->Id();
			m_muUnitName.lock();
			m_UnitName = ape->Name();
			m_muUnitName.unlock();
			ape->SetOnLine(m_AccountType, SyncAveragePing(), now);

			if(AcctIsArchitect(m_AccountType))
				ape->SetAgentBits(AGENT_Incognito);
			ape->TickTock();
			g_Au.UnlockEtheriumDelete();

			// Signal db thread to post updates.
			g_AuDb.PostCriticalUpdates();	// Save the new avatar right away.

			// Success! Send the info the client needs to get the scene.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtSceneServerPacket::PE_Success;
			m_idUnit >> out;
			SendPacket(ba);

			// Login server needs to know player's nu unit assignment.
			AuLoginComm login;
			login.SetPlayerUnit(m_idPlayer, m_confirmNum, m_idUnit);

			emit PersonSystemChanged(m_idSystem, m_idPlayer);	// Notify our sibling DtChatChannel of the unit's system.

			if(scenario == CitizenOfTheGalaxy)
			{
				// Join the new avatar to its empire.
				ape->SetEmpireId(joinEmpire);
				emit AddCitizenToEmpire(0, joinEmpire, ape->Id(), ape->Name(), ape->Gender());
			}

			QString logmsg = tr("Avatar %1 %2 created on %3 %4.")
				.arg(m_idUnit.GetID())
				.arg(charname)
				.arg(idHomeWorld.GetID())
				.arg(startname);
			g_Au.Log(logmsg);
			return 0;	// good return result
		}
		else
		{
			// The nu homeworld was not located. The client should try again when it gets this response because we will have it
			// loaded eventually.
			result = 15;
		}
	}
	else
	{
		//qDebug() << "universe not found";
		result = 10;
	}
	g_Au.UnlockEtheriumDelete();
	return result;
}

void DtSceneServer::DeletePerson(const AuId& personId)
{
	//qDebug("DtSceneServer::DeletePerson");
	// Login server needs to know player's unit assignment.
	AuLoginComm login;
	login.DeletePlayerUnit(m_idPlayer, m_confirmNum, personId);
}

void DtSceneServer::SolarSystemChanged(AuId /*systemId*/, AuId sectorId)
{
	//qDebug("DtSceneServer::SolarSystemChanged");
	// If the sector is in my clients starfield, nullify m_SyncSector.
	m_rwSync.lockForRead();
	bool hasSector = m_AtomSync.find(sectorId) != m_AtomSync.cend();
	m_rwSync.unlock();
	if(hasSector)
	{
		// The sector is one we have sent to the client.
		// This will cause a fresh starfield to be sent to the client.
		m_rwSync.lockForWrite();
		m_SyncSector = AuId();
		m_rwSync.unlock();
	}
}

// Rolling avg of atoms sent in refreshes.
unsigned int DtSceneServer::SyncAverageAtoms() const
{
	m_rwSync.lockForRead();
	unsigned int total = 0;
	for(int it=0; it<c_SyncHistory; it++)
		total += m_SyncLastAtoms[it];
	unsigned int avg = total / c_SyncHistory;
	m_rwSync.unlock();
	return avg;
}

// Rolling avg of bytes sent in refreshes.
unsigned int DtSceneServer::SyncAverageBytes() const
{
	//qDebug("DtSceneServer::SyncAverageBytes");
	m_rwSync.lockForRead();
	unsigned int total = 0;
	for(int it=0; it<c_SyncHistory; it++)
		total += m_SyncLastBytes[it];
	unsigned int avg = total / c_SyncHistory;
	m_rwSync.unlock();
	return avg;
}

// Rolling avg of ping times.
unsigned int DtSceneServer::SyncAveragePing() const
{
	m_rwSyncLast.lockForRead();
	unsigned int total = 0;
	for(int it=0; it<c_SyncHistory; it++)
		total += m_SyncLastPing[it];
	unsigned int avg = total / c_SyncHistory;
	m_rwSyncLast.unlock();
	return avg;
}

void DtSceneServer::CheckLoginStatus()
{
	//qDebug("DtSceneServer::CheckLoginStatus");
	// The confirmation number is never zero after a player has successfully logged in to this server by sending a DtSceneServerPacket::PL_Login packet.
	// However, the client connects to the master server to get empire data and then has lots of settings to configure
	// before they finally send us the DtSceneServerPacket::PL_Login packet. During that time the confirmation number is still zero. We must not call
	// IsPlayerLoggedIn with a zero confirmation number or it will disconnect nu users while they are configuring their settings,
	// typically when setting up a nu empire because of the large number of options.
	if(!m_confirmNum)
		return;
	AuId pIdUnit[6];
	if(!g_Au.IsPlayerLoggedIn(m_idPlayer, m_confirmNum, pIdUnit, m_AccountType, m_AccountExpiry))
	{
		// This player is no longer logged in or the confirmation number is incorrect.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PL_Logout;
		SendPacket(ba);
		Disconnect();

		g_Au.LogConnect(auSceneServerApp->LoginName(), QString("SCENE:%99 DISCONNECT! CheckLoginStatus failed IsPlayerLoggedIn %1 unit %2 %3")
				.arg(m_idPlayer).arg(m_idUnit.GetID()).arg(m_UnitName).arg(m_Id));
	}
}

void DtSceneServer::Disconnect()
{
	//qDebug("DtSceneServer::Disconnect");
	// Signal the run loop to disconnect.
	m_tmDisconnect.start();
}

void DtSceneServer::ReleaseClientControl()
{
	if(m_idSystem.IsNull())
		return;
	AuId systemId(m_idSystem);
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSolarSystem* system = auSceneServerApp->AssignedSolarSystem(systemId, ether);
	if(system)
	{
		DaPerson* person = dynamic_cast<DaPerson*>(system->FindIdRecursive(m_idUnit));
		if(person)
		{
			// Immediately turn off the client-controlled flag on the avatar and the atom they control, e.g. vehicle or ship.
			// This enables those atoms to resume movement during this server's Tick thread with as little bump as possible.
			DaAtom* parentControl = person->ParentControlAtom();				// Parent atom if it's controlled by the person.
			person->SetControlled(AuId());
			if(parentControl)
				parentControl->SetControlled(AuId());
		}
	}
	g_Au.UnlockEtheriumDelete();
}

// Disconnect this channel if it is connected to 'playerId'.
void DtSceneServer::DisconnectPlayer(quint32 playerId, unsigned long exceptThreadId)
{
	if(m_idPlayer == playerId && exceptThreadId != (unsigned long)m_ThreadId)
	{
		g_Au.LogConnect(auSceneServerApp->LoginName(), tr("SCENE:%99 Disconnect. DisconnectPlayer player %1 unit %2 %3")
				.arg(m_idPlayer).arg(m_idUnit.GetID()).arg(m_UnitName).arg(m_Id));
		Disconnect();
	}
}

void DtSceneServer::DisconnectUnit(AuId unitId)
{
	if(m_idUnit == unitId)
	{
		g_Au.LogConnect(auSceneServerApp->LoginName(), tr("SCENE:%99 Disconnect. DisconnectUnit player %1 unit %2 %3")
				.arg(m_idPlayer).arg(m_idUnit.GetID()).arg(m_UnitName).arg(m_Id));
		Disconnect();
	}
}

void DtSceneServer::DisconnectUnitTransferred(AuId unitId)
{
	if(m_idUnit == unitId)
	{
		// Notify the client so they will not immediately try to reconnect.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PL_SwitchAvatar;
		out << unitId;					// Tell them which avatar id is no longer valid.
		SendPacket(ba);

		g_Au.LogConnect(auSceneServerApp->LoginName(), tr("SCENE:%99 Disconnect. DisconnectUnitTransferred player %1 unit %2 %3")
				.arg(m_idPlayer).arg(m_idUnit.GetID()).arg(m_UnitName).arg(m_Id));
		Disconnect();
	}
}

void DtSceneServer::EstDeferred(const AuId& idTo, const AuId& idInfo)
{
	//qDebug("DtSceneServer::EstDeferred");
	m_IdDeferEstTo = idTo;
	m_IdInfo = idInfo;
}

void DtSceneServer::DeferEst()
{
	Q_ASSERT(!m_IdDeferEstTo.IsNull());
	//Q_ASSERT(!g_Au.IsSceneWorker());

	// Check to see if the destination has been assigned yet.
	quint8 srvmode;
	DhSceneServer toServer;
	if(g_Au.IsSceneMaster())
	{
		toServer = auSceneServerApp->SectorServer(m_IdDeferEstTo);
		srvmode = 1;	// deferred to manager
	}
	else
	{
		toServer = auSceneServerApp->SolarSystemServer(m_IdDeferEstTo);
		srvmode = 2;	// deferred to worker
	}
	if(!toServer.IsValid())
		return;	// Assignment has not been completed yet.

	// Defer remainder of init to the server that got the assignment.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtSceneServerPacket::PE_EstDeferred;
	if(m_LocalClient)
		out << toServer.m_PeerName;
	else
		out << toServer.m_NetName;
	out << toServer.m_NetPort;
	out << srvmode;
	out << m_IdInfo;
	SendPacket(ba);
	Disconnect();

	QString logmsg;
	if(m_LocalClient)
		logmsg = tr("SCENE:%99 Disconnect. Est deferred to peer %2:%3.").arg(toServer.m_PeerName).arg(toServer.m_NetPort).arg(m_Id);
	else
		logmsg = tr("SCENE:%99 Disconnect. Est deferred to net %2:%3.").arg(toServer.m_NetName).arg(toServer.m_NetPort).arg(m_Id);
	g_Au.LogConnect(auSceneServerApp->LoginName(), logmsg);
}

void DtSceneServer::PlayerOnlineStatus(AuPlayerOnlineStatus& status) const
{
	status.m_Id = m_idUnit;
	m_muUnitName.lock();
	status.m_Name = m_UnitName;
	m_muUnitName.unlock();
	status.m_EmpireId = m_idUnitEmpire;
	if(m_isUnitNewbie)
		status.m_StatusBits |= 0x0001;	// b0=nubie emperor
	if(AcctIsAgent(m_AccountType))
		status.m_StatusBits |= 0x0002;	// b1=agent
	if(m_AFK)
		status.m_StatusBits |= 0x0004;	// b2=AFK
	if(m_ChatClient)
		status.m_StatusBits |= 0x0010;	// b4=chat client
	status.m_StatusBits |= (m_UnitAgentBits << 8);	// agent bits in top byte
	status.m_OsType = m_OsType;
	status.m_OsVer = m_OsVer;
	status.m_WordSize = m_WordSize;

	if(g_Au.m_LogMessages & LOG_Status)
	{
		QString logmsg = QString("Player Online Status: %1%2 %3 empire %4 in solar system %5. "
				"Connected %6. Average %7 atoms %8 bytes %9ms ping.")
			.arg(m_isUnitNewbie?"New emperor ":"")
			.arg(status.m_Id.GetID())
			.arg(status.m_Name)
			.arg(status.m_EmpireId)
			.arg(m_idSystem.GetID())
			.arg(m_tmConnect.UTC().toString())
			.arg(SyncAverageAtoms())
			.arg(SyncAverageBytes())
			.arg(SyncAveragePing());
		g_Au.Log(logmsg);
	}
}

void DtSceneServer::AtomPresenceResolved(AuId atom, quint32 empire, QString netName, quint16 port, QString peerName)
{
	if(IsAwaitingAtomQuery(atom, empire))
		AtomPresence(netName, port, peerName);
}

void DtSceneServer::EmpireRecordAvailable(quint32 empireId)
{
	//qDebug() << "DtSceneServer::EmpireRecordAvailable" << empireId << "IsEmpireInScene" << IsEmpireInScene(empireId);
	if(IsEmpireInScene(empireId))
		SendEmpireData(empireId);
}

void DtSceneServer::SwitchToCharacter(const AuId& personId)
{
	//qDebug() << "DtSceneServer::SwitchToCharacter" << personId << "current unit" << m_idUnit;
	if(!m_idPlayer)
		return;	// We get here on the master server sometimes.
	if(personId.IsNull())
	{
		qWarning() << "ERROR DtSceneServer::SwitchToCharacter current player" << m_idPlayer << "sent NULL personId";
		return;													// What?
	}
	if(personId == m_idUnit)
		return;													// That IS the current unit.

	AuId idUnits[6];
	if(g_Au.IsPlayerLoggedIn(m_idPlayer, m_confirmNum, idUnits, m_AccountType, m_AccountExpiry))
	{
		// Insure the unit is actually one of their dudes.
		for(int iu=0; iu<6; iu++)
			if(personId == idUnits[iu])
			{
				// Notify the chat channel of the change of identity.
				emit SceneAtomChanged(m_idUnit, personId);

				// Remove the old and nu scene atoms from the scene sync cache. That way they will each get fresh data.
				m_rwSync.lockForWrite();
				QHash<AuId, AuAtomSync>::iterator si = m_AtomSync.find(personId);
				if(si != m_AtomSync.end())
					m_AtomSync.erase(si);

				si = m_AtomSync.find(m_idUnit);
				if(si != m_AtomSync.end())
					m_AtomSync.erase(si);

				unordered_map<AuId,bool, AuIdHash>::iterator li = m_SyncLastAtomIds.find(personId);
				if(li != m_SyncLastAtomIds.end())
					m_SyncLastAtomIds.erase(li);

				li = m_SyncLastAtomIds.find(m_idUnit);
				if(li != m_SyncLastAtomIds.end())
					m_SyncLastAtomIds.erase(li);
				m_rwSync.unlock();

				// Change the current unit id of the scene atom.
				m_idUnit = personId;

				// Pick up the person's name ASAP.
				QString name;
				if(!m_idUnit.IsNull())
				{
					AuEther& ether = g_Au.LockEtheriumDelete();
					DaUnit* unit = dynamic_cast<DaUnit*>(ether.FindNormalAtom(m_idUnit));
					if(unit)
					{
						name = unit->AssignedName();	// Get the real name of agents, instead of 'Entity' or 'Animal';
						DaWorldLL* world = unit->FindParentWorldInEnv();
						if(world)
							world->SetModified(DaAtom::NOTIFY_Scene);	// Force fog of war update.
					}
					g_Au.UnlockEtheriumDelete();
				}
				m_muUnitName.lock();
				if(name.isEmpty())
					m_UnitName = g_Au.MiddleDotSymbol();
				else
					m_UnitName = name;
				m_muUnitName.unlock();
				break;
			}

		// If the unit id did not change, then the unit does not belong to the current player.
		if(m_idUnit != personId)
			qWarning() << "ERROR DtSceneServer::SwitchToCharacter current player" << m_idPlayer << "does not own unit" << personId;
	}
	else
		qWarning() << "ERROR DtSceneServer::SwitchToCharacter current player" << m_idPlayer << "is not logged in?";
}

void DtSceneServer::ClientPositionUpdate(bool atRest, float headTilt, AuAtomPos& pos, AuPose pose[DpBody::PS_Count],
		quint16 whichConsole, quint16 stationId, quint8 seatIndex, AuPoint& vec, bool parentAlso, bool parentAtRest,
		AuAtomPos& parentPos, AuPose parentPose[DpBody::PS_Count], quint16 parentWhichConsole, quint16 parentStationId,
		AuPoint& parentVec)
{
	if(pos.ParentId().IsNull())
	{
		qDebug() << "DtSceneServer::ClientPositionUpdate null parent id, ignoring";
		return;
	}
	if(m_idSystem.IsNull())
		return;

	/*
	qDebug() << "DtSceneServer::ClientPositionUpdate rest" << atRest
		<< "head" << (headTilt*RAD2DEGf)
		<< "pos" << pos
		<< "pose" << pose[AuAnimalPart::PS_General]
		<< "w" << whichConsole
		<< "s" << stationId
		<< "t" << seatIndex
		<< "v" << vec
		<< "par2" << parentAlso
		<< "prest" << parentAtRest
		<< "ppos" << parentPos
		<< "ppose" << parentPose[AuAnimalPart::PS_General]
		<< "pw" << parentWhichConsole
		<< "ps" << parentStationId
		<< "pv" << parentVec;
		*/

	AuId systemId(m_idSystem);
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSolarSystem* system = auSceneServerApp->AssignedSolarSystem(systemId, ether);
	if(system)
	{
		// Locate the person unit.
		DaPerson* person = dynamic_cast<DaPerson*>(system->FindIdRecursive(m_idUnit));
		if(person && system->FindIdRecursive(pos.ParentId()))
		{
			person->SetControlled(m_idUnit);
			bool checkCollision = false;	// Set true if CheckCollision must be called
			AuPoint oloc, nloc;				// For CheckCollision, if needed.

			DaAtom* parent = person->Parent();
			bool updatePerson = true;							// False rejects the position update; reacquires pos vars instead.
			if(!parent)
				updatePerson = false;
			else if(person->NeedsInit())
				updatePerson = false;							// Wait for Tock to position them initially in the environment.
			else if(dynamic_cast<DaTeleporter*>(parent))
				updatePerson = false;							// Scene atom's parent is a teleporter.
			else if(!parent->IsNormal())
				updatePerson = false;

			if(updatePerson)
			{
				// Apply the position updates to the person.
				bool personChanged = false;
				bool changedParent = false;
				AuAtomPos curpos(person->Pos());
				if(curpos != pos)
				{
					personChanged = true;
					if(curpos.ParentId() != pos.ParentId())
					{
						// The person's parent atom is changing.
						changedParent = true;

						// Release control of old parent when appropriate so it can immediately be managed by a tick thread.
						DaVehicle* vehicle = dynamic_cast<DaVehicle*>(parent);
						if(vehicle)
						{
							if(vehicle->IsController(m_idUnit))
							{
								vehicle->SetParked(false);		// Autopark when the driver exits.
								vehicle->SetControlled(AuId());	// Release client control of vehicle.
							}
						}
						else
						{
							DaVessel* ship = dynamic_cast<DaVessel*>(parent);
							if(ship)
							{
								if(ship->IsController(m_idUnit))
									ship->SetControlled(AuId());	// Release client control of ship.
							}
							else
							{
								DaCreature* mount = dynamic_cast<DaCreature*>(parent);
								if(mount)
									mount->Dismounted();	// Person is dismounting from a creature.
							}
						}

						// Position the person.
						person->ChangePosAndOrient(pos);
						person->InvalidateEnvironment();
					}
					else
					{
						// Parent didn't change.
						// Call CheckCollision
						checkCollision = true;
						oloc = curpos.Translation();
						nloc = pos.Translation();

						// Position the person.
						person->ChangePosAndOrient(pos);
						person->InvalidateEnvironment();

						// Update turret position.
						if(person->IsTurretIntegratedPose())
						{
							DaCapsule* capsule = dynamic_cast<DaCapsule*>(person->Parent());
							if(capsule)
								capsule->PositionTurret(*person);
						}
					}
				}

				if(person->CurrentHeadTilt() != headTilt)
				{
					person->TiltHeadTo(headTilt);
					personChanged = true;
				}

				// The poses sent by the client are ignored when the body is dying or dead.
				if(!person->IsBodyDyingOrDead() && !person->NeedsInit())
				{
					// The client is not permitted to set certain poses, such as init, dying and dead poses.
					bool dopose = true;
					for(int ps=0; ps<DpBody::PS_Count; ps++)
					{
						AuPose p = pose[ps];
						if(p == AuPose::Init ||
								p == AuPose::WritheFaceDown || p == AuPose::WritheFaceLeft ||
								p == AuPose::WritheFaceRight || p == AuPose::WritheFaceUp ||
								p == AuPose::DeadFaceDown || p == AuPose::DeadFaceLeft ||
								p == AuPose::DeadFaceRight || p == AuPose::DeadFaceUp)
						{
							dopose = false;
							break;
						}
					}
					if(dopose)
					{
						// If there is no change, then don't set my pose information.
						AuPose curPose[DpBody::PS_Count];
						if(person->HasDNA3())
							person->CurrentPoses3(curPose);
						else
						{
							person->CurrentPoses4(curPose);
							// Server manages the mouth pose so preserve local mouth pose.
							pose[AuAnimalPart::PS_Mouth] = curPose[AuAnimalPart::PS_Mouth];
						}
						dopose = false;
						for(int ps=0; ps<DpBody::PS_Count; ps++)
							if(pose[ps] != curPose[ps])
							{
								// A body pose position changed.
								dopose = true;
								break;
							}
					}

					if(dopose)
					{
						AuPose curPose = person->CurrentGeneralPose();
						AuPose nuPose;
						if(person->HasDNA3())
							nuPose = pose[DpBody::PS_LowerTorso];
						else
							nuPose = pose[AuAnimalPart::PS_General];
						if(curPose.IsHelmStation() && nuPose != curPose)
						{
							// Departing pilot console, possibly just capt chair mode switch.
							DaVessel* ship = dynamic_cast<DaVessel*>(person->Parent());
							if(ship)
							{
								if(ship->IsClientControlled() && ship->Pilot() == person)
									ship->SetControlled(AuId());	// Release client control of ship.
							}
						}
						else if(curPose.IsGameTable() && !nuPose.IsGameTable())
						{
							// Person is exiting a game table.
							DaCapsule* capsule = dynamic_cast<DaCapsule*>(person->Parent());
							if(capsule)
								capsule->GameExitPlayer(person->StationId(), person->Id());
						}

						bool wasJumping = person->IsJumping();
						AuPoint jumpVector = person->JumpVectorFromCurrentPose();
						if(person->HasDNA3())
							person->SetPoses3(pose, stationId, whichConsole);
						else
							person->SetPoses4(pose, stationId, whichConsole);
						if(!wasJumping && person->IsJumping())
						{
							person->Jump(person->JumpPower(), jumpVector);		// The person jumped. Use up jump stamina.
							person->TickTock();
						}
						else if(curPose.IsMatingOrWaiting() && !nuPose.IsMatingOrWaiting())
							person->DismountAllRiders();						// Dismount all riders when leaving mating pose.
						else if(!curPose.IsMatingOrWaiting() && nuPose.IsMatingOrWaiting())
							person->DismountAllRiders();						// Dismount all riders when entering mating pose.
						personChanged = true;
					}
				}

				if(person->Vector() != vec)
				{
					person->SetVector(vec);
					personChanged = true;
				}

				if(atRest)
				{
					if(!person->IsAtRest())
					{
						person->SetAtRest();
						personChanged = true;
					}
				}
				else
				{
					if(person->IsAtRest())
					{
						person->SetInMotion();
						person->TickTock();
						personChanged = true;
					}
				}

				if(person->VehicleSeatIndex() != seatIndex)
				{
					person->SetVehicleSeatIndex(seatIndex);
					personChanged = true;

					// Release client control of vehicle when appropriate so it can immediately be taken over by a tick thread.
					DaVehicle* vehicle = dynamic_cast<DaVehicle*>(person->Parent());
					if(vehicle)
					{
						if(vehicle->IsController(m_idUnit))
							vehicle->SetControlled(AuId());		// Release client control of vehicle.
					}
				}
				if(personChanged)
					person->SetModified();

				if(changedParent)
				{
					// When entering a teleporter, we must force it to send us to our destination.
					DaTeleporter* tp = dynamic_cast<DaTeleporter*>(person->Parent());
					if(tp)
					{
						tp->MoveSubatom(person, AuPoint(0,0), 0);
						pos = person->Pos();
					}
				}

				// Check if client is controlling their parent atom also.
				if(parentAlso && system->FindIdRecursive(parentPos.ParentId()))
				{
					// Determine which atom the local person is controlling. This must be acquired after reparenting the person.
					DaUnit* parentControl = person->ParentControlAtom();
					bool updateControl = true;
					if(!parentControl)
						updateControl = false;					// Server returns no atom under their control.
					else if(dynamic_cast<DaTeleporter*>(parentControl->Parent()))
						updateControl = false;					// Controlled atom's parent is a teleporter.
					else if(!parentControl->IsClientControlledNoReset())
					{
						// First frame of control is ignored because it is sometimes wrong, like when emerging from a wormhole.
						updateControl = false;
						parentControl->SetControlled(m_idUnit);
					}

					if(updateControl)
					{
						bool parCheckCollision = false;	// Set true if CheckCollision must be called
						AuPoint poloc, pnloc;			// For CheckCollision, if needed.
						parentControl->SetControlled(m_idUnit);
						bool parentControlChanged = false;
						bool parentParentChanged = false;
						AuAtomPos curposc(parentControl->Pos());
						if(curposc != parentPos)
						{
							parentControlChanged = true;
							if(curposc.ParentId() != parentPos.ParentId())
							{
								// The control's parent atom is changing.
								parentParentChanged = true;

								// Position the control atom.
								parentControl->ChangePosAndOrient(parentPos);
								parentControl->InvalidateEnvironment();
							}
							else
							{
								// Control's parent didn't change.
								// Call CheckCollision
								parCheckCollision = true;
								poloc = curposc.Translation();
								pnloc = parentPos.Translation();

								// Position the parent atom.
								parentControl->ChangePosAndOrient(parentPos);
								parentControl->InvalidateEnvironment();
							}
						}

						if(parentControl->Vector() != parentVec)
						{
							parentControl->SetVector(parentVec);
							parentControlChanged = true;
						}

						if(parentAtRest)
						{
							if(!parentControl->IsAtRest())
							{
								parentControl->SetAtRest();
								parentControlChanged = true;
							}
						}
						else
						{
							if(parentControl->IsAtRest())
							{
								parentControl->SetInMotion();
								parentControl->TickTock();
								parentControlChanged = true;
							}
						}

						// The poses sent by the client are ignored when the mount's body is dying or dead.
						DaCreature* mount = dynamic_cast<DaCreature*>(parentControl);
						if(mount && !mount->IsBodyDyingOrDead() && !mount->NeedsInit())
						{
							// The client is not permitted to set certain poses, such as init, dying and dead poses.
							bool dopose = true;
							for(int ps=0; ps<DpBody::PS_Count; ps++)
							{
								AuPose p = parentPose[ps];
								if(p == AuPose::Init ||
										p == AuPose::WritheFaceDown || p == AuPose::WritheFaceLeft ||
										p == AuPose::WritheFaceRight || p == AuPose::WritheFaceUp ||
										p == AuPose::DeadFaceDown || p == AuPose::DeadFaceLeft ||
										p == AuPose::DeadFaceRight || p == AuPose::DeadFaceUp)
								{
									dopose = false;
									break;
								}
							}
							if(dopose)
							{
								bool wasJumping = mount->IsJumping();
								AuPoint jumpVector = mount->JumpVectorFromCurrentPose();
								if(mount->HasDNA3())
									mount->SetPoses3(parentPose, parentStationId, parentWhichConsole);
								else
									mount->SetPoses4(parentPose, parentStationId, parentWhichConsole);
								if(!wasJumping && mount->IsJumping())
								{
									mount->Jump(person->JumpPower(), jumpVector);	// The person jumped. Use up jump stamina.
									mount->TickTock();
								}
								parentControlChanged = true;
							}
						}
						if(parentControlChanged)
							parentControl->SetModified();

						if(parentParentChanged)
						{
							// When entering a teleporter, we must force it to send us to our destination.
							DaTeleporter* tp = dynamic_cast<DaTeleporter*>(parentControl->Parent());
							if(tp)
							{
								tp->MoveSubatom(parentControl, AuPoint(0,0), 0);
								parentPos = parentControl->Pos();
							}
						}

						if(parCheckCollision)
						{
							// Call CheckCollision to open automatic doors along the path.
							DaAtom* pHit;
							AuPoint hitLoc, hitNormal;
							parentControl->CheckCollision(poloc, pnloc, pHit, hitLoc, hitNormal);
						}
					}
				}
			}
			else
			{
				DaUnit* parentControl = person->ParentControlAtom();			// Parent atom if it's controlled by the person.
				atRest = person->IsAtRest();
				headTilt = person->CurrentHeadTilt();
				pos = person->Pos();
				if(person->HasDNA3())
					person->CurrentPoses3(pose);
				else
					person->CurrentPoses4(pose);	// This one takes fewer array entries so it is safe.
				whichConsole = person->ControlId();
				stationId = person->StationId();
				seatIndex = person->VehicleSeatIndex();
				vec = person->Vector();
				if(parentControl && parentControl->IsNormal() && system->FindIdRecursive(parentPos.ParentId()))
				{
					parentAlso = true;
					parentControl->SetControlled(m_idUnit);
					parentAtRest = parentControl->IsAtRest();
					parentPos = parentControl->Pos();
					const DaCreature* mount = dynamic_cast<const DaCreature*>(parentControl);
					if(mount)
					{
						if(mount->HasDNA3())
							mount->CurrentPoses3(parentPose);
						else
							mount->CurrentPoses4(parentPose);
						parentWhichConsole = mount->ControlId();
						parentStationId = mount->StationId();
					}
					else
					{
						for(int ps=0; ps<DpBody::PS_Count; ps++)
							parentPose[ps] = AuPose::Init;
						parentWhichConsole = 0;
						parentStationId = 0;
					}
					parentVec = parentControl->Vector();
				}
				else
					parentAlso = false;
			}
			if(checkCollision)
			{
				// Call CheckCollision to open automatic doors along the path.
				DaAtom* pHit;
				AuPoint hitLoc, hitNormal;
				person->CheckCollision(oloc, nloc, pHit, hitLoc, hitNormal);
			}
		}
	}
	g_Au.UnlockEtheriumDelete();

	// Update last-reported position values.
	m_PosAtRest = atRest;
	m_PosHeadTilt = headTilt;
	m_Pos = pos;
	for(int ps=0; ps<DpBody::PS_Count; ps++)
		m_PosPose[ps] = pose[ps];
	m_PosSeatIndex = seatIndex;
	m_PosVec = vec;
	m_PosWhichConsole = whichConsole;
	m_PosStationId = stationId;
	m_PosParentAlso = parentAlso;
	if(parentAlso)
	{
		m_PosParentAtRest = parentAtRest;
		m_PosParent = parentPos;
		for(int ps=0; ps<DpBody::PS_Count; ps++)
			m_PosParentPose[ps] = parentPose[ps];
		m_PosParentVec = parentVec;
		m_PosParentWhichConsole = parentWhichConsole;
		m_PosParentStationId = parentStationId;
	}
	else
	{
		m_PosParentAtRest = true;
		m_PosParent = AuAtomPos();
		for(int ps=0; ps<DpBody::PS_Count; ps++)
			m_PosParentPose[ps] = AuPose::Init;
		m_PosParentVec = AuPoint();
		m_PosParentWhichConsole = 0;
		m_PosParentStationId = 0;
	}
}

void DtSceneServer::ClientEnteredWormhole(const AuId& travId, const AuWormhole& wh, float entryAngle)
{
	if(m_idSystem.IsNull())
		return;

	AuId systemId(m_idSystem);
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSolarSystem* system = auSceneServerApp->AssignedSolarSystem(systemId, ether);
	if(system)
	{
		DaStar* star = system->FindPrimaryStar();
		if(star)
		{
			DaVessel* ship = dynamic_cast<DaVessel*>(star->FindIdRecursive(travId));
			if(ship)
				star->TraverseWormhole(ship, wh, entryAngle);
		}
	}
	g_Au.UnlockEtheriumDelete();
}

void DtSceneServer::ClientCollision(const AuId& myId, const AuPoint& myChange, const AuPoint& myHitLoc, const AuId& hitId,
		const AuPoint& hitChange, const AuPoint& hitFromLoc, const AuPoint& hitResultVec, bool hitShield)
{
	if(m_idSystem.IsNull())
		return;

	AuId systemId(m_idSystem);
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSolarSystem* system = auSceneServerApp->AssignedSolarSystem(systemId, ether);
	if(system)
	{
		DaAtom* me = system->FindIdRecursive(myId);
		DaAtom* hit = system->FindIdRecursive(hitId);
		if(me && hit)
			me->Collide(myChange, myHitLoc, hit, hitChange, hitFromLoc, hitResultVec, hitShield);
	}
	g_Au.UnlockEtheriumDelete();
}

void DtSceneServer::ClientFell(const AuId& fellId, quint32 hitsDamage, int stopReason)
{
	if(m_idSystem.IsNull())
		return;

	AuId systemId(m_idSystem);
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSolarSystem* system = auSceneServerApp->AssignedSolarSystem(systemId, ether);
	if(system)
	{
		DaAtom* fellAtom = system->FindIdRecursive(fellId);
		if(fellAtom)
			fellAtom->Fell(hitsDamage, stopReason);
	}
	g_Au.UnlockEtheriumDelete();
}

void DtSceneServer::ClientDeadheaded(const AuId& travId, const AuPoint& departureLoc)
{
	if(m_idSystem.IsNull())
		return;

	AuId systemId(m_idSystem);
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSolarSystem* system = auSceneServerApp->AssignedSolarSystem(systemId, ether);
	if(system)
	{
		DaAtom* travAtom = system->FindIdRecursive(travId);
		if(travAtom)
			system->DepartedOnDeadHeading(travAtom, departureLoc);
	}
	g_Au.UnlockEtheriumDelete();
}

