//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtclientchat.cpp,v 1.48 2022/04/12 20:35:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dtclientchat.h"
//
#include "auapp.h"
#include "aubdreport.h"
#include "auclientglobal.h"
#include "auglobal.h"
#include "auitreport.h"
#include "aumission.h"
#include "auscreport.h"
#include "ausensorblip.h"
#include "daanimal.h"
#include "dadesignplane.h"
#include "daeffect.h"
#include "dagalaxy.h"
#include "daitem.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dastar.h"
#include "dateleporter.h"
#include "dauniverse.h"
#include "davehicle.h"
#include "dgdata.h"
#include "dtccpacket.h"
#include "hashpack.h"
#include <QDataStream>

DtClientChat::DtClientChat(QObject* parent) : QThread(parent), m_semAlive(1)
{
	m_BytesRx = 0;
	m_BytesTx = 0;
	m_isConnected = false;
	m_PacketSize = 0;
	m_PacketSizeBytesLeft = 0;
	m_semAlive.acquire(1);			// I live...
}

DtClientChat::~DtClientChat()
{
}

void DtClientChat::run()
{
	//qDebug("DtClientChat::run");
	// Creating the socket on the stack automatically associates it with the thread's
	// event loop. This ensures that Qt will not try to deliver events to our socket
	// from the main thread while we are accessing it from DtClientChat::run().
	QTcpSocket tcpSocket;
	//connect(&tcpSocket, SIGNAL(readyRead()), SLOT(RxData()));
	//connect(&tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
			//SLOT(NetworkError(QAbstractSocket::SocketError)));

	// Connect to server.
	tcpSocket.abort();
	tcpSocket.connectToHost(g_Au.SceneServer(), g_Au.ChatPort());
	tcpSocket.waitForConnected(10000);
	if(tcpSocket.state() == QAbstractSocket::UnconnectedState)
	{
		// The connection attempt failed. The server or the network is probably not up.
		//qWarning("DtClientChat::run - tcp connection attempt failed");
		return;
	}
	m_isConnected = true;
	emit Connected();
	//qDebug("DtClientChat::run - connected to %s:%d", (const char*)g_Au.SceneServer().toAscii(), g_Au.ScenePort());
	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		g_Au.Log(g_Au.CacheApp() + "/soh.txt",
				QString("Chat: Connected to %1 port %2").arg(g_Au.SceneServer()).arg(g_Au.ChatPort()));

	while(Alive() && tcpSocket.state() != QAbstractSocket::UnconnectedState)
	{
		if(!tcpSocket.waitForDisconnected(10))
		{
			// Send anything that's ready to go.
			TxData(tcpSocket);

			// Pull in any data that is waiting for us.
			// Note: If we wait for the signal, it appears to come from the gui
			// thread, which causes us grief. It's better for us to get the data
			// when it's there.
			RxData(tcpSocket);
		}
	}

	m_isConnected = false;
	if(gp_AuGlobal)
	{
		// Emit my disconnected signal so the app will try to reconnect me when the connection is lost unexpectedly.
		// This also causes the app to log the time the chat channel was disconnect. That way we can get the chat channels we
		// missed if we reconnect within the next few minutes. This is the purpose of AuDbGlobal::RecentAtomChat.
		emit Disconnected();

		//qDebug("DtClientChat::run - Disconnected.");
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("Chat: Disconnected"));
	}
}

void DtClientChat::NetworkError(QAbstractSocket::SocketError socketError)
{
	switch(socketError)
	{
	case QAbstractSocket::RemoteHostClosedError:
		qWarning("DtClientChat::NetworkError - RemoteHostClosedError");
		break;
	case QAbstractSocket::HostNotFoundError:
		qWarning("DtClientChat::NetworkError - HostNotFoundError");
		break;
	case QAbstractSocket::ConnectionRefusedError:
		qWarning("DtClientChat::NetworkError - ConnectionRefusedError");
		break;
	default:
		//if(m_Tcp)
			//qWarning("DtClientChat::NetworkError - Error %s",
					//(const char*)m_Tcp->errorString());
		//else
			qWarning("DtClientChat::NetworkError - Error");
		break;
	}
}

void DtClientChat::RxData(QTcpSocket& tcpSocket)
{
	while(tcpSocket.bytesAvailable())
	{
		if(!gp_AuGlobal)
			return;

		g_Au.SetNet1Downloading(100);
		if(!m_PacketSize || m_PacketSizeBytesLeft)
		{
			int packetSizeBytesRead = AuUtil::ReadPacketSize(tcpSocket, m_PacketSize, m_PacketSizeBytesLeft);
			if(m_PacketSizeBytesLeft)
				break;					// We don't have all of the packet size bytes yet.
			m_BytesRx += packetSizeBytesRead;
			m_LastComm.restart();
		}

		// Wait until all of the bytes arrive.
		if(tcpSocket.bytesAvailable() < m_PacketSize)
			break;

		// It is now safe to read our data from the socket buffer.
		QByteArray block = tcpSocket.read(m_PacketSize);
		m_BytesRx += m_PacketSize;
		m_LastComm.restart();
		m_PacketSize = 0;
		if(!ReadPacket(block, tcpSocket))
		{
			if(tcpSocket.state() != QAbstractSocket::UnconnectedState)
				tcpSocket.disconnectFromHost();
			if(tcpSocket.state() != QAbstractSocket::UnconnectedState)
				tcpSocket.waitForDisconnected(10000);
			m_isConnected = false;
			break;
		}
	}
}

void DtClientChat::TxData(QTcpSocket& tcpSocket)
{
	int written = auApp->TransmitChatBuffer(tcpSocket);
	if(written)
	{
		m_BytesTx += written;
		m_LastComm.restart();
	}

	if(!written && m_LastComm.isValid() && m_LastComm.elapsed()/d_MSecPerSec > 60)
	{
		// Send a keep-alive packet when no data has been sent within the last minute.
		// Client keep alive packets were squeezed in without restarting the servers. So they became hangar action 3.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_Hangar;
		out << (quint8)3;
		int written = AuUtil::WritePacketSize(tcpSocket, ba.size());
		written += tcpSocket.write(ba);
		m_BytesTx += written;
		m_LastComm.restart();
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("Chat: Keep alive packet sent."));
	}
}

bool DtClientChat::ReadPacket(const QByteArray& ba, QTcpSocket& /*tcpSocket*/)
{
	QDataStream block(ba);
	block.setVersion(QDataStream::Qt_4_1);

	// The first byte is always the packet type.
	quint8 ptype;
	block >> ptype;
	//qDebug("DtClientChat::ReadPacket - %d bytes rx'd, PACKET Type %d",
			//ba.count(), ptype);
	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("Chat: Read packet type %1 size %L2.").arg(ptype).arg(ba.count()));

	// This cast generates a handy warning when an enum value is forgotten.
	switch((DtChatChannelPacket::PACKET)ptype)
	{
	case DtChatChannelPacket::PZ_QueryAvatars:
		{
			AuId unitIds[6];
			QString unitNames[6];

			for(int ia=0; ia<6; ia++)
			{
				block >> unitIds[ia];
				block >> unitNames[ia];
				//qDebug() << "DtChatChannelPacket::PL_QueryAvatars" << unitIds[ia] << unitNames[ia];
			}

			// Only change avatar ids and names. The other avatar stuff (DNA, handedness, gender) is not needed at this point. It
			// is only used at startup.
			DgData& gd = g_Au.LockDataForWrite();
			gd.SetClientAvatars(unitIds, unitNames);
			g_Au.UnlockData();
			emit AvatarsUpdated();
		}
		break;

	case DtChatChannelPacket::PTR_Place:
		{
			quint16 count;
			block >> count;
			while(count--)
			{
				AuTransporterTarget tt;
				block >> tt;
				emit TransporterPlace(tt);
			}
		}
		break;
	case DtChatChannelPacket::PTR_Ship:
		{
			quint16 count;
			block >> count;
			while(count--)
			{
				AuTransporterShip tt;
				block >> tt;
				emit TransporterShip(tt.Id(), tt.Name(), tt.SpotDesc());
			}
		}
		break;
	case DtChatChannelPacket::PTR_Transporter:
		{
			quint16 count;
			block >> count;
			while(count--)
			{
				AuTransporterSystem tt;
				block >> tt;
				emit TransporterTransporter(tt.Id(), tt.Name(), tt.SpotDesc(), tt.Which());
			}
		}
		break;

	// Chat
	case DtChatChannelPacket::PC_ChannelOpened:
		{
			QString channelName;
			quint8 scope;
			QString narrDesc;

			block >> channelName;
			block >> scope;
			block >> narrDesc;

			emit ChannelOpened(channelName, scope, narrDesc);
		}
		break;
	case DtChatChannelPacket::PC_Chat:				// Chat message.
		{
			// Create an encryption key based on this user's login credentials.
			const DgData& gd = g_Au.LockDataForRead();
			quint16 wKey = AuUtil::NetCryptKey((quint16)gd.ConfirmationNumber());
			g_Au.UnlockData();

			AuCommPacket* msg = AuCommPacket::RxDecrypt(block, wKey);
			if(msg)
			{
				// We have successfully decrypted a comm message packet.
				// Bundle it back up into a byte array so it can be emitted as a signal.
				QByteArray bamsg;
				QDataStream out(&bamsg, QIODevice::WriteOnly);
				out.setVersion(QDataStream::Qt_4_1);
				out << (quint8)msg->Type();
				out << *msg;

				emit ChatReceived(bamsg);

				delete msg;
			}
		}
		break;
	case DtChatChannelPacket::PC_OpenChannelTo:
		break;

	case DtChatChannelPacket::PM_Hangar:
		{
			QString shipDesc;
			quint32 shipDesign;
			quint32 shipsInHangar;

			block >> shipDesc;
			block >> shipDesign;
			block >> shipsInHangar;

			emit HangarQueryAnswered(shipDesc, shipDesign, shipsInHangar);
		}
		break;

	case DtChatChannelPacket::PJ_Query:			// Query for stories and jobs that can be started. Job offer back to client.
		{
			AuJobOffer job;

			block >> job;

			// If a new job type is added on the server that this client does not know about, the job pointer will safely be 0.
			if(job.Job())
			{
				emit JobAvailable(job);
			}
		}
		break;
	case DtChatChannelPacket::PV_Specie:
		{
			// Server sent a specie record to me.
			DzSpecies species;
			species << block;
			emit ReplaceSpecies(species);
		}
		break;

	case DtChatChannelPacket::PT_GameSelect:
		{
			// This is the response packet when the local client player has attempted to select a new game at a game table.
			quint8 gameType;		// AuGame::GameType selected.
			bool gameSelected;		// true if game selected, false if it could not be selected
			block >> gameType;
			block >> gameSelected;
			emit GameSelect(gameType, gameSelected);
		}
		break;

	case DtChatChannelPacket::PT_GameTurn:
		{
			// Sent periodically (during Tock about every 10 secs) to remind me of whose turn it is in my table game,
			// whether or not its my turn.
			AuId personsTurn;
			block >> personsTurn;
			emit GameTurn(personsTurn);
		}
		break;

	case DtChatChannelPacket::PT_GameOver:
		{
			// This is sent periodically (10 secs) to inform me of whose turn it is in my table game, whether or not its me.
			quint8 gameType;
			QByteArray gameResult;
			block >> gameType;
			block >> gameResult;
			emit GameOver(gameType, gameResult);
		}
		break;

	case DtChatChannelPacket::PD_ScDesignHere:
		{
			quint16 designCount;
			quint32 designId;
			quint8 designState;	// AuDesign::STATE

			block >> designCount;
			//qDebug() << "DtChatChannelPacket::PD_ScDesignHere designCount" << designCount;
			while(designCount--)
			{
				block >> designId;
				block >> designState;
				//qDebug() << " designId" << designId << "state" << designState;
				emit ScDesignNeeded(designId, designState);
			}
		}
		break;

	case DtChatChannelPacket::PD_ScDesign:			// Design rec from server.
		{
			AuScDesign rec;
			rec.Rx(block);

			emit ScDesign(rec);
		}
		break;

	case DtChatChannelPacket::PD_ScErase:		// Erase design from server. Precedes fresh send of entire design.
		{
			quint32 idDesign;
			block >> idDesign;

			emit ScDesignDelete(idDesign);
		}
		break;

	case DtChatChannelPacket::PD_ItDesignHere:
		{
			quint16 designCount;
			quint32 designId;
			quint8 designState;	// AuDesign::STATE

			block >> designCount;
			//qDebug() << "DtChatChannelPacket::PD_ItDesignHere designCount" << designCount;
			while(designCount--)
			{
				block >> designId;
				block >> designState;
				//qDebug() << " designId" << designId << "state" << designState;
				emit ItDesignNeeded(designId, designState);
			}
		}
		break;

	case DtChatChannelPacket::PD_ItDesign:			// Design rec from server.
		{
			AuItDesign rec;
			rec.Rx(block);

			emit ItDesign(rec);
		}
		break;

	case DtChatChannelPacket::PD_ItErase:		// Erase design from server. Precedes fresh send of entire design.
		{
			quint32 idDesign;
			block >> idDesign;

			emit ItDesignDelete(idDesign);
		}
		break;

	case DtChatChannelPacket::PD_BdDesignHere:
		{
			quint16 designCount;
			quint32 designId;
			quint8 designState;	// AuDesign::STATE

			block >> designCount;
			//qDebug() << "DtChatChannelPacket::PD_BdDesignHere designCount" << designCount;
			while(designCount--)
			{
				block >> designId;
				block >> designState;
				//qDebug() << " designId" << designId << "state" << designState;
				emit BdDesignNeeded(designId, designState);
			}
		}
		break;

	case DtChatChannelPacket::PD_BdDesign:			// Design rec from server.
		{
			AuBdDesign rec;
			rec.Rx(block);

			emit BdDesign(rec);
		}
		break;

	case DtChatChannelPacket::PD_BdErase:		// Erase design from server. Precedes fresh send of entire design.
		{
			quint32 idDesign;
			block >> idDesign;

			emit BdDesignDelete(idDesign);
		}
		break;

	case DtChatChannelPacket::PN_Mission:
		{
			quint8 command;
			block >> command;

			if(command == 0)
			{
				// Server has responded to my request for the current mission.
			}
			else if(command == 1)
			{
				// Server has responded to my request for sensor contacts for mission planning.
				AuSensorBlipList blips;
				block >> blips;

				// Bundle the sensor blips back up into a byte array so they can be emitted as a signal.
				QByteArray bablips;
				QDataStream out(&bablips, QIODevice::WriteOnly);
				out.setVersion(QDataStream::Qt_4_1);
				out << blips;

				emit MissionSensorContactsReceived(bablips);
			}
		}
		break;

	case DtChatChannelPacket::PE_Report:
		{
			quint8 reportWhat;

			block >> reportWhat;

			switch(reportWhat)
			{
			case 0:
				{
					AuId cityId;
					AuCityReport report;
					AuCityReport prevReport;
					bool hasPrevReport;

					block >> cityId;
					block >> report;
					block >> hasPrevReport;
					if(hasPrevReport)
					{
						block >> prevReport;

						AuCityReportHistory rcomb;
						rcomb.Append(prevReport);
						rcomb.Append(report);
						emit CityHistoryReportReady(cityId, rcomb);
					}
					else
						emit CityLastReportReady(cityId, report);
				}
				break;
			case 1:
				{
					AuId cityId;
					AuCityReportHistory report;
					block >> cityId;
					block >> report;
					emit CityHistoryReportReady(cityId, report);
				}
				break;
			case 2:	// Players online status
				{
					AuPlayerOnlineStatusVector players;
					players << block;
					emit PlayersOnlineStatus(players);
				}
				break;
			}
		}
		break;

	case DtChatChannelPacket::PI_Ordnance:
		{
			AuId loaderId;
			AuItemHash ord;

			block >> loaderId;
			block >> ord;

			// Bundle the ordnance back up into a byte array so they can be emitted as a signal.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << ord;
			emit OrdnanceReceived(loaderId, ba);
		}
		break;

	case DtChatChannelPacket::PM_QueryArenas:
		{
			quint16 arenas;

			block >> arenas;

			while(arenas--)
			{
				AuArenaReady arena;

				block >> arena;

				emit ArenaReady(arena);
			}
		}
		break;

	case DtChatChannelPacket::PD_ItLibrary:
		{
			quint8 doWhat;

			block >> doWhat;

			if(doWhat == 0)
			{
				// Receive a list of designs available for loading from the library.
				g_AuClient.DesignerClearItList();

				quint16 c;

				block >> c;
				//qDebug() << "DtClientChat::ReadPacket PD_ItLibrary receiving" << c << "designs";

				quint8 designState;
				QImage designThumb;
				while(c--)
				{
					AuItReport designReport;
					block >> designReport;
					block >> designState;
					block >> designThumb;

					//qDebug() << "DtClientChat::ReadPacket PD_ItLibrary received design" << designId << designName <<
						//designThumb.size();
					g_AuClient.DesignerAddItDesignListEntry(designReport, designState, designThumb);
				}

				emit ItLibraryUpdated();
			}
		}
		break;

	case DtChatChannelPacket::PD_ScLibrary:
		{
			quint8 doWhat;

			block >> doWhat;

			if(doWhat == 0)
			{
				// Receive a list of designs available for loading from the library.
				g_AuClient.DesignerClearScList();

				quint16 c;

				block >> c;
				//qDebug() << "DtClientChat::ReadPacket PD_ScLibrary receiving" << c << "designs";

				quint8 designState;
				QImage designThumb;
				while(c--)
				{
					AuScReport designReport;
					block >> designReport;
					block >> designState;
					block >> designThumb;

					//qDebug() << "DtClientChat::ReadPacket PD_ScLibrary received design" << designId << designName <<
						//designThumb.size();
					g_AuClient.DesignerAddScDesignListEntry(designReport, designState, designThumb);
				}

				emit ScLibraryUpdated();
			}
		}
		break;

	case DtChatChannelPacket::PD_ScExchange:
		{
			quint8 doWhat;

			block >> doWhat;

			if(doWhat == 0)
			{
				// Receive a list of designs available for purchase from the exchange.
				g_AuClient.DesignerClearScExchange();

				quint32 c;

				block >> c;
				//qDebug() << "DtClientChat::ReadPacket PD_ScExchange receiving" << c << "designs";

				QImage designThumb;
				while(c--)
				{
					AuScDesignExchangeEntry item;
					block >> item;
					block >> designThumb;
					//qDebug() << "DtClientChat::ReadPacket PD_ScExchange received design" << item.DesignId() << item.DesignName()
						//<< designThumb.size();

					// Ask the server for their empire data, for the name.
					const DgData& gd = g_Au.LockDataForRead();
					bool hasEmpire = gd.HasEmpire(item.PublisherEmpire());
					g_Au.UnlockData();
					if(!hasEmpire)
						emit EmpireRequested(item.PublisherEmpire());

					g_AuClient.DesignerAddScExchangeEntry(item, designThumb);
				}

				emit ScExchangeUpdated();
			}
			else if(doWhat == 1)
			{
				// Receive ratings for a design.
				quint32 designId;
				QString designName;
				AuExchangeBlueprintRating rating;
				block >> designId;
				block >> designName;
				block >> rating;
				emit ScExchangeBlueprintRating(designId, designName, rating);
			}
			else if(doWhat == 2)
			{
				// Receive a compressed list of designs available for purchase from the exchange.
				g_AuClient.DesignerClearScExchange();
				QByteArray bacomp;
				block >> bacomp;
				QByteArray ba = AuUncompress(bacomp, __FILE__, __LINE__,"");

				// Read from the uncompressed data.
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_5_7);

				quint32 c;

				in >> c;
				//qDebug() << "DtClientChat::ReadPacket PD_ScExchange receiving" << c << "designs";

				QImage designThumb;
				while(c--)
				{
					AuScDesignExchangeEntry item;
					in >> item;
					in >> designThumb;
					//qDebug() << "DtClientChat::ReadPacket PD_ScExchange received design" << item.DesignId() << item.DesignName()
						//<< designThumb.size();

					// Ask the server for their empire data, for the name.
					const DgData& gd = g_Au.LockDataForRead();
					bool hasEmpire = gd.HasEmpire(item.PublisherEmpire());
					g_Au.UnlockData();
					if(!hasEmpire)
						emit EmpireRequested(item.PublisherEmpire());

					g_AuClient.DesignerAddScExchangeEntry(item, designThumb);
				}

				emit ScExchangeUpdated();
			}
			else if(doWhat == 10)
			{
				// Receive ToC.
				quint32 count;
				block >> count;

				QHash<quint32, AuTime> toc;	// Designs we will specifically request from the server.
				quint32 designId;
				AuTime whenMod;
				while(count--)
				{
					block >> designId;
					block >> whenMod;
					toc.insert(designId, whenMod);
				}
				QList<quint32> designsNeeded;
				g_AuClient.ScExchangeToC(toc, designsNeeded);
				auApp->ScExchangeDesignsNeeded(designsNeeded);
			}
			else if(doWhat == 11)
			{
				// Receive a compressed partial list of designs available for purchase from the exchange.
				QByteArray bacomp;
				block >> bacomp;
				QByteArray ba = AuUncompress(bacomp, __FILE__, __LINE__,"");

				// Read from the uncompressed data.
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_5_7);

				quint32 c;

				in >> c;
				//qDebug() << "DtClientChat::ReadPacket PD_ScExchange receiving" << c << "designs";

				QImage designThumb;
				while(c--)
				{
					AuScDesignExchangeEntry item;
					in >> item;
					in >> designThumb;
					//qDebug() << "DtClientChat::ReadPacket PD_ScExchange received design" << item.DesignId() << item.DesignName()
						//<< designThumb.size();

					// Ask the server for their empire data, for the name.
					const DgData& gd = g_Au.LockDataForRead();
					bool hasEmpire = gd.HasEmpire(item.PublisherEmpire());
					g_Au.UnlockData();
					if(!hasEmpire)
						emit EmpireRequested(item.PublisherEmpire());

					g_AuClient.DesignerAddScExchangeEntry(item, designThumb);
				}

				emit ScExchangeUpdated();
			}
		}
		break;

	case DtChatChannelPacket::PD_BdLibrary:
		{
			quint8 doWhat;

			block >> doWhat;

			if(doWhat == 0)
			{
				// Receive a list of designs available for loading from the library.
				g_AuClient.DesignerClearBdList();

				quint16 c;

				block >> c;
				//qDebug() << "DtClientChat::ReadPacket PD_BdLibrary receiving" << c << "designs";

				quint8 designState;
				QImage designThumb;
				while(c--)
				{
					AuBdReport designReport;
					block >> designReport;
					block >> designState;
					block >> designThumb;

					//qDebug() << "DtClientChat::ReadPacket PD_BdLibrary received design" << designId << designName <<
						//designThumb.size();
					g_AuClient.DesignerAddBdDesignListEntry(designReport, designState, designThumb);
				}

				emit BdLibraryUpdated();
			}
		}
		break;

	case DtChatChannelPacket::PD_BdExchange:
		{
			quint8 doWhat;

			block >> doWhat;

			if(doWhat == 0)
			{
				// Receive a list of designs available for purchase from the exchange.
				g_AuClient.DesignerClearBdExchange();

				quint32 c;

				block >> c;
				//qDebug() << "DtClientChat::ReadPacket PD_BdExchange receiving" << c << "designs";

				QImage designThumb;
				while(c--)
				{
					AuBdDesignExchangeEntry item;
					block >> item;
					block >> designThumb;
					//qDebug() << "DtClientChat::ReadPacket PD_BdExchange received design" << item.DesignId() << item.DesignName()
						//<< designThumb.size();

					// Ask the server for their empire data, for the name.
					const DgData& gd = g_Au.LockDataForRead();
					bool hasEmpire = gd.HasEmpire(item.PublisherEmpire());
					g_Au.UnlockData();
					if(!hasEmpire)
						emit EmpireRequested(item.PublisherEmpire());

					g_AuClient.DesignerAddBdExchangeEntry(item, designThumb);
				}

				emit BdExchangeUpdated();
			}
			else if(doWhat == 1)
			{
				// Receive ratings for a design.
				quint32 designId;
				QString designName;
				AuExchangeBlueprintRating rating;
				block >> designId;
				block >> designName;
				block >> rating;
				emit BdExchangeBlueprintRating(designId, designName, rating);
			}
			else if(doWhat == 2)
			{
				// Receive a compressed list of designs available for purchase from the exchange.
				g_AuClient.DesignerClearBdExchange();
				QByteArray bacomp;
				block >> bacomp;
				QByteArray ba = AuUncompress(bacomp, __FILE__, __LINE__,"");

				// Read from the uncompressed data.
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_5_7);

				quint32 c;

				in >> c;
				//qDebug() << "DtClientChat::ReadPacket PD_BdExchange receiving" << c << "designs";

				QImage designThumb;
				while(c--)
				{
					AuBdDesignExchangeEntry item;
					in >> item;
					in >> designThumb;
					//qDebug() << "DtClientChat::ReadPacket PD_BdExchange received design" << item.DesignId() << item.DesignName()
						//<< designThumb.size();

					// Ask the server for their empire data, for the name.
					const DgData& gd = g_Au.LockDataForRead();
					bool hasEmpire = gd.HasEmpire(item.PublisherEmpire());
					g_Au.UnlockData();
					if(!hasEmpire)
						emit EmpireRequested(item.PublisherEmpire());

					g_AuClient.DesignerAddBdExchangeEntry(item, designThumb);
				}

				emit BdExchangeUpdated();
			}
			else if(doWhat == 10)
			{
				// Receive ToC.
				quint32 count;
				block >> count;

				QHash<quint32, AuTime> toc;	// Designs we will specifically request from the server.
				quint32 designId;
				AuTime whenMod;
				while(count--)
				{
					block >> designId;
					block >> whenMod;
					toc.insert(designId, whenMod);
				}
				QList<quint32> designsNeeded;
				g_AuClient.BdExchangeToC(toc, designsNeeded);
				auApp->BdExchangeDesignsNeeded(designsNeeded);
			}
			else if(doWhat == 11)
			{
				// Receive a compressed partial list of designs available for purchase from the exchange.
				QByteArray bacomp;
				block >> bacomp;
				QByteArray ba = AuUncompress(bacomp, __FILE__, __LINE__,"");

				// Read from the uncompressed data.
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_5_7);

				quint32 c;

				in >> c;
				//qDebug() << "DtClientChat::ReadPacket PD_BdExchange receiving" << c << "designs";

				QImage designThumb;
				while(c--)
				{
					AuBdDesignExchangeEntry item;
					in >> item;
					in >> designThumb;
					//qDebug() << "DtClientChat::ReadPacket PD_BdExchange received design" << item.DesignId() << item.DesignName()
						//<< designThumb.size();

					// Ask the server for their empire data, for the name.
					const DgData& gd = g_Au.LockDataForRead();
					bool hasEmpire = gd.HasEmpire(item.PublisherEmpire());
					g_Au.UnlockData();
					if(!hasEmpire)
						emit EmpireRequested(item.PublisherEmpire());

					g_AuClient.DesignerAddBdExchangeEntry(item, designThumb);
				}

				emit BdExchangeUpdated();
			}
		}
		break;

	case DtChatChannelPacket::PZ_Trade:
		{
			quint8 doWhat;

			block >> doWhat;

			if(doWhat == 0)
			{
				// Receive a list of shipments for a specific broker.
				AuShipmentList shipments;

				block >> shipments;

				emit BrokerShipmentsClear();
				for(AuShipmentList::const_iterator si = shipments.cbegin(); si != shipments.cend(); si++)
					emit BrokerShipment(*si);
			}
			else if(doWhat == 1)
			{
				// Receive a list of trade agreements for a specific broker.
				AuTradeAgreementList tas;

				block >> tas;

				emit BrokerTradeAgreementsClear();
				for(AuTradeAgreementList::const_iterator ti = tas.cbegin(); ti != tas.cend(); ti++)
					emit BrokerTradeAgreement(*ti);
			}
		}
		break;

	case DtChatChannelPacket::PI_TransferCargo:
		{
			quint8 doWhat;

			block >> doWhat;

			if(doWhat == 0)
			{
				// Received information about citizens available for transfer.
				QByteArray ba;
				block >> ba;
				emit TransferCitizensAvailable(ba);
			}
		}
		break;

	case DtChatChannelPacket::PB_BdBuild:
	case DtChatChannelPacket::PB_ChangeRoadConstruction:
	case DtChatChannelPacket::PB_ClearLand:
	case DtChatChannelPacket::PB_Demolish:
	case DtChatChannelPacket::PB_PlanFence:
	case DtChatChannelPacket::PB_PlanRoadGlobe:
	case DtChatChannelPacket::PB_RaiseFlag_Obsolete:
	case DtChatChannelPacket::PC_ChannelClosed:
	case DtChatChannelPacket::PC_Connect:
	case DtChatChannelPacket::PC_Emoji:
	case DtChatChannelPacket::PC_HailStore:
	case DtChatChannelPacket::PC_PostBounty:
	case DtChatChannelPacket::PC_SquelchAdd:
	case DtChatChannelPacket::PC_SquelchDelete:
	case DtChatChannelPacket::PC_SquelchUpdate:
	case DtChatChannelPacket::PD_BdLoaded:
	case DtChatChannelPacket::PD_Exit:
	case DtChatChannelPacket::PD_Gravity:
	case DtChatChannelPacket::PD_ItLoaded:
	case DtChatChannelPacket::PD_New:
	case DtChatChannelPacket::PD_Open:
	case DtChatChannelPacket::PD_Position:
	case DtChatChannelPacket::PD_Properties:
	case DtChatChannelPacket::PD_Redo:
	case DtChatChannelPacket::PD_ScLoaded:
	case DtChatChannelPacket::PD_ScPart:
	case DtChatChannelPacket::PD_Undo:
	case DtChatChannelPacket::PE_ChangeFlag:
	case DtChatChannelPacket::PE_GABalance:
	case DtChatChannelPacket::PE_GADeposit:
	case DtChatChannelPacket::PE_GAWithdraw:
	case DtChatChannelPacket::PE_LeaveEmpire:
	case DtChatChannelPacket::PE_Rename:
	case DtChatChannelPacket::PF_Station0:
	case DtChatChannelPacket::PF_Station1:
	case DtChatChannelPacket::PF_Station2:
	case DtChatChannelPacket::PF_Station3:
	case DtChatChannelPacket::PF_Station4:
	case DtChatChannelPacket::PF_Station5:
	case DtChatChannelPacket::PF_Station6:
	case DtChatChannelPacket::PF_Station7:
	case DtChatChannelPacket::PF_Station8:
	case DtChatChannelPacket::PF_Station9:
	case DtChatChannelPacket::PF_Station10:
	case DtChatChannelPacket::PF_Station11:
	case DtChatChannelPacket::PF_Station12:
	case DtChatChannelPacket::PF_Station13:
	case DtChatChannelPacket::PF_Station14:
	case DtChatChannelPacket::PF_Station15:
	case DtChatChannelPacket::PF_Station16:
	case DtChatChannelPacket::PF_Station17:
	case DtChatChannelPacket::PF_Station18:
	case DtChatChannelPacket::PF_Station19:
	case DtChatChannelPacket::PI_AvatarSetting:
	case DtChatChannelPacket::PI_BankBorrow:
	case DtChatChannelPacket::PI_BankPay:
	case DtChatChannelPacket::PI_Drop:
	case DtChatChannelPacket::PI_GearItem:
	case DtChatChannelPacket::PI_GetFromCargo:
	case DtChatChannelPacket::PI_Inscribe:
	case DtChatChannelPacket::PI_Insurer:
	case DtChatChannelPacket::PI_Manipulate:
	case DtChatChannelPacket::PI_Message:
	case DtChatChannelPacket::PI_PutIntoCargo:
	case DtChatChannelPacket::PI_Selfie:
	case DtChatChannelPacket::PI_SendFunds:
	case DtChatChannelPacket::PI_SetFlowerPot:
	case DtChatChannelPacket::PI_SetPictureFrame:
	case DtChatChannelPacket::PI_TurboLift:
	case DtChatChannelPacket::PI_Use:
	case DtChatChannelPacket::PJ_Accept:								// only sent to server
	case DtChatChannelPacket::PJ_QueryFilter:
	case DtChatChannelPacket::PM_ArenaExit:
	case DtChatChannelPacket::PM_ArenaRecall:
	case DtChatChannelPacket::PM_ArenaStatus:
	case DtChatChannelPacket::PM_CitizenEnabled:
	case DtChatChannelPacket::PM_CreateArena:
	case DtChatChannelPacket::PM_EnterStudio:
	case DtChatChannelPacket::PM_Fetch:
	case DtChatChannelPacket::PM_Go:
	case DtChatChannelPacket::PM_HandCraftGo:
	case DtChatChannelPacket::PM_JoinArena:
	case DtChatChannelPacket::PM_ProcessSelected:
	case DtChatChannelPacket::PM_ProcessSetMinQ:
	case DtChatChannelPacket::PM_ScFactory:
	case DtChatChannelPacket::PS_ChoiceMade:		// only sent to server
	case DtChatChannelPacket::PS_CloseStory:		// only sent to server
	case DtChatChannelPacket::PS_StoryHint:		// only sent to server
	case DtChatChannelPacket::PS_StoryResendBlock:		// only sent to server
	case DtChatChannelPacket::PS_StorySubmission:
	case DtChatChannelPacket::PTR_InputCoord:
	case DtChatChannelPacket::PTR_InputLongLat:
	case DtChatChannelPacket::PT_GameQuit:
	case DtChatChannelPacket::PT_GameStart:
	case DtChatChannelPacket::PU_SetAmmo:
	case DtChatChannelPacket::PU_SetAmmoCount:
	case DtChatChannelPacket::PU_SetArmor:
	case DtChatChannelPacket::PU_SetClothing:
	case DtChatChannelPacket::PU_SetHelmet:
	case DtChatChannelPacket::PU_SetWeapon:
	case DtChatChannelPacket::PV_AcceptCrew:
	case DtChatChannelPacket::PV_AcceptPassengers:
	case DtChatChannelPacket::PV_AcceptTroops:
	case DtChatChannelPacket::PV_AllowPassengerSeaChests:
	case DtChatChannelPacket::PV_AssignOffice:
	case DtChatChannelPacket::PV_ChangeTitle:
	case DtChatChannelPacket::PV_Christen:
	case DtChatChannelPacket::PV_ClaimReward:
	case DtChatChannelPacket::PV_ContrailColor:
	case DtChatChannelPacket::PV_Dossier:
	case DtChatChannelPacket::PV_KickBerthOccupant:
	case DtChatChannelPacket::PV_ReceiveShipFunds:
	case DtChatChannelPacket::PV_ReconcileRoster:
	case DtChatChannelPacket::PV_ReleaseBerth:
	case DtChatChannelPacket::PV_ResignOffice:
	case DtChatChannelPacket::PV_RevokeOffice:
	case DtChatChannelPacket::PV_SendShipFunds:
	case DtChatChannelPacket::PV_SetAnnouncements:
	case DtChatChannelPacket::PV_SetBoss:
	case DtChatChannelPacket::PV_SetCapital:
	case DtChatChannelPacket::PV_SetCommerce:
	case DtChatChannelPacket::PV_SetDefStance:
	case DtChatChannelPacket::PV_SetJoinPass:
	case DtChatChannelPacket::PV_SetPilotPolicy:
	case DtChatChannelPacket::PV_SetPoliticalStance:
	case DtChatChannelPacket::PV_SetShieldPolicy:
	case DtChatChannelPacket::PV_SetTax:
	case DtChatChannelPacket::PV_SetThemeColor:
	case DtChatChannelPacket::PV_SpeciesRequest:
	case DtChatChannelPacket::PV_WarCriteria:
	case DtChatChannelPacket::PZ_AgentBits:
	case DtChatChannelPacket::PZ_Capacitor:
	case DtChatChannelPacket::PZ_ChangeShip:
	case DtChatChannelPacket::PZ_EmpireChg:
	case DtChatChannelPacket::PZ_EmpireChgEmperor:
	case DtChatChannelPacket::PZ_EmpireChgFlag:
	case DtChatChannelPacket::PZ_EmpireChgName:
	case DtChatChannelPacket::PZ_EmpireChgRole:
	case DtChatChannelPacket::PZ_Encounter:
	case DtChatChannelPacket::PZ_EnvironmentChange:
	case DtChatChannelPacket::PZ_GoToAtom:
	case DtChatChannelPacket::PZ_GoToGalaxyLoc:
	case DtChatChannelPacket::PZ_GoToLongLat:
	case DtChatChannelPacket::PZ_KickStowaways:
	case DtChatChannelPacket::PZ_KillAnimals:
	case DtChatChannelPacket::PZ_MakeCommodity:
	case DtChatChannelPacket::PZ_Refuel:
	case DtChatChannelPacket::PZ_ResetStory:
	case DtChatChannelPacket::PZ_SetDNA:
	case DtChatChannelPacket::PZ_SummonPerson:
	case DtChatChannelPacket::PZ_TickTock:
	case DtChatChannelPacket::P_Invalid:
		qWarning("DtClientChat::ReadPacket - ERROR! UNEXPECTED PACKET TYPE %d", ptype);
		break;
	}
	return true;
}
