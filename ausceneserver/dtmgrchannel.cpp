//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtmgrchannel.cpp,v 1.54 2022/04/08 18:41:27 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dtmgrchannel.h"
//
#include "audbglobal.h"
#include "ausceneserverapp.h"
#include "dtmcpacket.h"
#include <QHostAddress>

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DtMgrChannel::DtMgrChannel(int socketDesc, QObject* parent) : QThread(parent)
{
	m_HasConnected = false;

	// Comm
	m_BadPackets = 0;
	m_BytesRx = 0;
	m_BytesTx = 0;
	m_Disconnect = false;
	m_PacketSize = 0;
	m_PacketSizeBytesLeft = 0;
	m_socketDescriptor = socketDesc;
}

void DtMgrChannel::run()
{
	m_ThreadId = currentThreadId();

	// Creating the socket on the stack automatically associates it with the
	// thread's event loop. This ensures that Qt will not try to deliver events
	// to our socket from the main thread while we are accessing it from
	// DtMgrChannel::run().
	QTcpSocket tcpSocket;
	if(!tcpSocket.setSocketDescriptor(m_socketDescriptor))
	{
		emit error(tcpSocket.error());
		return;
	}

	m_tmConnect.Set();
	//qDebug("DtMgrChannel::run - Connection established.");
	QString peerAddr = tcpSocket.peerAddress().toString();
	QString logmsg = tr("MGR: Connected %1").arg(peerAddr);
	g_Au.LogConnect(auSceneServerApp->LoginName(), logmsg);

	while(tcpSocket.state() == QAbstractSocket::ConnectedState && !tcpSocket.waitForDisconnected(20))
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
				//qDebug("DtMgrChannel::run - %d bytes written out of %d", written,
						//m_TxBuffer.count());
				m_TxBuffer.clear();
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

		// When a disconnect time is established, we have to eventually disconnect.
		if(m_Disconnect)
		{
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.disconnectFromHost();
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.waitForDisconnected(10000);
		}
	}

	//qDebug("DtMgrChannel::run - Player %d disconnected.", m_idPlayer);
	logmsg = tr("MGR: Disconnected %1").arg(peerAddr);
	if(gp_AuGlobal)
		g_Au.LogConnect(auSceneServerApp->LoginName(), logmsg);
}

void DtMgrChannel::RxData(QTcpSocket& tcpSocket)
{
	//qDebug("DtMgrChannel::RxData");
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

void DtMgrChannel::ReadPacket(const QByteArray& ba, QTcpSocket& tcpSocket)
{
	//qDebug("DtMgrChannel::ReadPacket - %d bytes rx'd", ba.count());
	QDataStream block(ba);
	block.setVersion(QDataStream::Qt_4_1);

	QString logmsg;
	//quint8 result = 0;

	// The first byte is always the packet type.
	quint8 type;
	block >> type;

	if(!m_HasConnected && type != DtMgrChannelPacket::P_Connect)
	{
		if(g_Au.m_LogMessages & LOG_Warning)
		{
			logmsg = tr("DtMgrChannel::ReadPacket received packet type %1 before P_Connect").arg(type);
			g_Au.Warning(logmsg);
		}
		Disconnect();
		return;
	}

	switch((DtMgrChannelPacket::PACKET)type)	// Generate warning when enum forgotten.
	{
	case DtMgrChannelPacket::P_Announcement:
		if(g_Au.IsSceneMaster())
		{
			QString text;
			block >> text;
			emit ServerBroadcast(text);
		}
		break;
	case DtMgrChannelPacket::P_Connect:
		{
			quint32 checkId;

			block >> checkId;

			// AuId::Haxus() value used here for no reason that is actually associated with a particular atom id. The value used
			// just needs to be consistent with the AuServerManager program and this provided a handy definition.
			if(AuId(checkId) == AuId::Haxus())
			{
				m_HasConnected = true;

				logmsg = tr("MGR: Manager Channel Has Connected");
				g_Au.LogConnect(auSceneServerApp->LoginName(), logmsg);
			}
			else
			{
				m_HasConnected = false;
				Disconnect();

				logmsg = tr("MGR: DISCONNECT! Connect failed: checkId %2 does not match expected id %3")
					.arg(checkId).arg(AuId::Haxus().GetID());
				g_Au.LogConnect(auSceneServerApp->LoginName(), logmsg);
			}
		}
		break;

	case DtMgrChannelPacket::P_Status:
		{
			quint8 specificStatus;

			block >> specificStatus;

			emit SendStatus(specificStatus);
		}
		break;

	case DtMgrChannelPacket::P_KickUnit:
		{
			AuId unitId;

			block >> unitId;

			emit DisconnectUnit(unitId);
		}
		break;

	case DtMgrChannelPacket::P_DebugMessages:
		{
			quint32 debugMessageBits;

			block >> debugMessageBits;

			g_Au.m_LogMessages = debugMessageBits;
			if(debugMessageBits)
			{
				qDebug() << "Debug Messages Configured";
				for(int bit=0; bit<32; bit++)
				{
					int bitval = 1<<bit;
					if(debugMessageBits & bitval)
						qDebug(" %s messages enabled", LogMessageTypeName(bit));
				}
			}
			else
				qDebug() << "All Types of Debug Messages Disabled";
		}
		break;

	case DtMgrChannelPacket::P_MemoryUsage:
		emit SendMemoryUsage();
		break;

	case DtMgrChannelPacket::P_Debug:
		{
			quint8 specificDebug, ui8;
			AuId id;

			block >> specificDebug;
			block >> ui8;
			block >> id;

			switch(specificDebug)
			{
			case 0:	// All debugs.
			default:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s All", ui8?"Enable":"Disable");
				g_Au.m_EnableAnimalAI = ui8;
				g_Au.m_EnableAnimalEncounters = ui8;
				g_Au.m_EnableCharacterAI = ui8;
				g_Au.m_EnableCitizenPathing = ui8;
				g_Au.m_EnableCopAI = ui8;
				g_Au.m_EnableCrewAI = ui8;
				g_Au.m_EnableIndigenousEncounters = ui8;
				g_Au.m_EnableIndigenousAI = ui8;
				g_Au.m_EnableMovement = ui8;
				g_Au.m_EnableOfficerAI = ui8;
				g_Au.m_EnablePassengerAI = ui8;
				g_Au.m_EnableSensorScanByCity = ui8;
				g_Au.m_EnableSensorScanByShip = ui8;
				g_Au.m_EnableSensorScanByStation = ui8;
				g_Au.m_EnableSetModified = ui8;
				g_Au.m_EnableTick = ui8;
				g_Au.m_EnableTock = ui8;
				g_Au.m_EnableTroopAI = ui8;
				g_Au.m_EnableWorkerAI = ui8;
				break;
			case 1:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Animal AI", ui8?"Enable":"Disable");
				g_Au.m_EnableAnimalAI = ui8;
				break;
			case 2:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Animal Encounters", ui8?"Enable":"Disable");
				g_Au.m_EnableAnimalEncounters = ui8;
				break;
			case 3:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Crew AI", ui8?"Enable":"Disable");
				g_Au.m_EnableCrewAI = ui8;
				break;
			case 4:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Officer AI", ui8?"Enable":"Disable");
				g_Au.m_EnableOfficerAI = ui8;
				break;
			case 5:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Passenger AI", ui8?"Enable":"Disable");
				g_Au.m_EnablePassengerAI = ui8;
				break;
			case 6:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s City Sensor Scans", ui8?"Enable":"Disable");
				g_Au.m_EnableSensorScanByCity = ui8;
				break;
			case 7:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Ship Sensor Scans", ui8?"Enable":"Disable");
				g_Au.m_EnableSensorScanByShip = ui8;
				break;
			case 8:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Station Sensor Scans", ui8?"Enable":"Disable");
				g_Au.m_EnableSensorScanByStation = ui8;
				break;
			case 9:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Tick", ui8?"Enable":"Disable");
				g_Au.m_EnableTick = ui8;
				break;
			case 10:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Tock", ui8?"Enable":"Disable");
				g_Au.m_EnableTock = ui8;
				break;
			case 11:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Troop AI", ui8?"Enable":"Disable");
				g_Au.m_EnableTroopAI = ui8;
				break;
			case 12:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Tick of Atom %d", ui8?"Enable":"Disable", id.GetID());
				if(ui8)
					g_Au.m_ExcludeFromTick.removeAll(id);
				else
					g_Au.m_ExcludeFromTick.AddIfUnique(id);
				break;
			case 13:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Tock of Atom %d", ui8?"Enable":"Disable", id.GetID());
				if(ui8)
					g_Au.m_ExcludeFromTock.removeAll(id);
				else
					g_Au.m_ExcludeFromTock.AddIfUnique(id);
				break;
			case 14:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Character AI", ui8?"Enable":"Disable");
				g_Au.m_EnableCharacterAI = ui8;
				break;
			case 15:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Cop AI", ui8?"Enable":"Disable");
				g_Au.m_EnableCopAI = ui8;
				break;
			case 16:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Worker AI", ui8?"Enable":"Disable");
				g_Au.m_EnableWorkerAI = ui8;
				break;

			case 17:
				// Toggle the DtTockTransactions logger.
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Tock Transactions Logger", ui8?"Enable":"Disable");
				if(ui8)
					g_Au.m_DebugSwitches |= 0x01000000;
				else
					g_Au.m_DebugSwitches &= ~0x01000000;
				break;

			case 18:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s SetModified", ui8?"Enable":"Disable");
				g_Au.m_EnableSetModified = ui8;
				break;
			case 19:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Movement", ui8?"Enable":"Disable");
				g_Au.m_EnableMovement = ui8;
				break;
			case 20:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Citizen Pathing in Spacecraft", ui8?"Enable":"Disable");
				g_Au.m_EnableCitizenPathing = ui8;
				break;
			case 21:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Tick of Atom Type %d", ui8?"Enable":"Disable", id.GetID());
				if(ui8)
					g_Au.m_ExcludeFromTickType.removeAll(id.GetID());
				else if(!g_Au.m_ExcludeFromTickType.contains(id.GetID()))
					g_Au.m_ExcludeFromTickType.append(id.GetID());
				break;
			case 22:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Tock of Atom Type %d", ui8?"Enable":"Disable", id.GetID());
				if(ui8)
					g_Au.m_ExcludeFromTockType.removeAll(id.GetID());
				else if(!g_Au.m_ExcludeFromTockType.contains(id.GetID()))
					g_Au.m_ExcludeFromTockType.append(id.GetID());
				break;
			case 23:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Job Posting", ui8?"Enable":"Disable");
				g_Au.m_EnableJobPosting = ui8;
				break;
			case 24:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Pirate Encounters", ui8?"Enable":"Disable");
				g_Au.m_EnablePirateEncounters = ui8;
				break;
			case 25:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Report Officers", ui8?"Enable":"Disable");
				g_Au.m_EnableReportOfficers = ui8;
				break;
			case 26:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Station Resupply", ui8?"Enable":"Disable");
				g_Au.m_EnableStationResupply = ui8;
				break;
			case 27:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s System Emission", ui8?"Enable":"Disable");
				g_Au.m_EnableSystemEmission = ui8;
				break;
			case 28:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Creature Decay", ui8?"Enable":"Disable");
				g_Au.m_EnableCreatureDecay = ui8;
				break;
			case 29:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Troop Resupply", ui8?"Enable":"Disable");
				g_Au.m_EnableTroopResupply = ui8;
				break;
			case 30:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Unit Decay", ui8?"Enable":"Disable");
				g_Au.m_EnableUnitDecay = ui8;
				break;
			case 31:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Indigenous Encounters", ui8?"Enable":"Disable");
				g_Au.m_EnableIndigenousEncounters = ui8;
				break;
			case 32:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug("%s Indigenous AI", ui8?"Enable":"Disable");
				g_Au.m_EnableIndigenousAI = ui8;
				break;
			case 33:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug() << "Clearing Tick Atom Id Exclusions";
				g_Au.m_ExcludeFromTick.clear();
				break;
			case 34:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug() << "Clearing Tick Atom Type Exclusions";
				g_Au.m_ExcludeFromTickType.clear();
				break;
			case 35:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug() << "Clearing Tock Atom Id Exclusions";
				g_Au.m_ExcludeFromTock.clear();
				break;
			case 36:
				if(g_Au.m_LogMessages & LOG_Debug)
					qDebug() << "Clearing Tock Atom Type Exclusions";
				g_Au.m_ExcludeFromTockType.clear();
				break;
			}
		}
		break;

	case DtMgrChannelPacket::P_Resources:
		{
			quint8 specificResources;
			AuId atomId;

			block >> specificResources;
			block >> atomId;

			emit SendResources(specificResources, atomId);
		}
		break;

	case DtMgrChannelPacket::P_ReloadData:
		emit ReloadData();
		break;

	case DtMgrChannelPacket::P_RestoreEmpire:
		{
			DgEmpire empire;

			empire << block;

			emit RestoreEmpire(empire);
		}
		break;

	case DtMgrChannelPacket::P_LimboRescue:
		{
			AuId atomId;

			block >> atomId;

			emit LimboRescue(atomId);
		}
		break;

	case DtMgrChannelPacket::P_Shutdown:
		{
			QString warnMsg;
			QString haltMsg;
			qint16 shutdownSecs;

			block >> warnMsg;
			block >> haltMsg;
			block >> shutdownSecs;

			emit Shutdown(warnMsg, haltMsg, shutdownSecs);
		}
		break;

	case DtMgrChannelPacket::P_Reassign:
		{
			AuId reassignId;
			QString wrkServerName;
			quint16 wrkServerPort;

			block >> reassignId;
			block >> wrkServerName;
			block >> wrkServerPort;

			emit ReassignSystem(reassignId, wrkServerName, wrkServerPort);
		}
		break;

	case DtMgrChannelPacket::P_Revoke:
		{
			AuId revokeId;

			block >> revokeId;

			emit RevokeAssignment(revokeId);
		}
		break;

	case DtMgrChannelPacket::P_Setting:
		{
			quint8 setting;

			block >> setting;

			switch(setting)
			{
			case 0:						// Control decay margin.
				{
					quint16 decayMargin;
					block >> decayMargin;
					g_Au.SetControlDecayMargin(decayMargin);
					qDebug() << "DtMgrChannel::ReadPacket P_Setting ControlDecayMargin" << g_Au.ControlDecayMargin() << "ms";
				}
				break;

			case 1:						// Udp decay margin.
				{
					quint16 decayMargin;
					block >> decayMargin;
					g_Au.SetUdpDecayMargin(decayMargin);
					qDebug() << "DtMgrChannel::ReadPacket P_Setting UdpDecayMargin" << g_Au.UdpDecayMargin() << "ms";
				}
				break;

			default:
				qDebug() << "DtMgrChannel::ReadPacket P_Setting setting" << setting << "INVALID";
				break;
			}
		}
		break;

	case DtMgrChannelPacket::P_StatusQuery:
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
			out << (quint8)DtMgrChannelPacket::P_StatusQuery;
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

	case DtMgrChannelPacket::P_FinalizeSystem:
		{
			AuId systemId;

			block >> systemId;

			qDebug() << "Manager Finalize" << systemId;
			emit FinalizeSystem(systemId);
		}
		break;

	case DtMgrChannelPacket::P_SetModified:
		{
			AuId atomId;
			bool recursive;

			block >> atomId;
			block >> recursive;

			emit SetAtomModified(atomId, recursive);
		}
		break;

	case DtMgrChannelPacket::P_CreateSectorAt:
		if(g_Au.IsSceneMaster())
		{
			quint8 galaxyEnum;
			AuPoint sectorLocPc;

			block >> galaxyEnum;
			block >> sectorLocPc;

			emit CreateSectorAt(galaxyEnum, sectorLocPc.x, sectorLocPc.y, sectorLocPc.z);
		}
		break;

	case DtMgrChannelPacket::P_ReloadScript:
		{
			// scriptId 0 reloads all scripts.
			quint32 scriptId;
			block >> scriptId;
			emit ReloadScript(scriptId);
		}
		break;

	case DtMgrChannelPacket::P_Invalid:
#if defined(QT_NO_DEBUG)
	default:				// No default in debug build for compiler warnings. Compiler warnings wanted in release build.
#endif
		g_Au.LogConnect(auSceneServerApp->LoginName(), QString("MGR: ERROR! Manager Channel Unknown TCP packet type %2 size %3. Hacker?")
				.arg(type).arg(ba.count()));
		m_BadPackets++;
		break;
	}
}

// Used by the client to send an array of bytes to the server.
void DtMgrChannel::SendPacket(const QByteArray& ba)
{
	// The packet is already formatted. Add it to the transmit data stream.
	//qDebug("DtMgrChannel::SendPacket - tx %d bytes", ba.count());
	m_muTxBuffer.lock();
	AuUtil::WritePacketSize(m_TxBuffer, ba.size());
	m_TxBuffer.append(ba);
	m_muTxBuffer.unlock();
}

void DtMgrChannel::Disconnect()
{
	// Signal the run loop to disconnect.
	m_Disconnect = true;
}

void DtMgrChannel::MgrSendPacket(QByteArray ba)
{
	SendPacket(ba);
}
