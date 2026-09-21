//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtclient.cpp,v 1.70 2022/04/15 17:01:23 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dtclient.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "augear.h"
#include "auglobal.h"
#include "aumission.h"
#include "daanimal.h"
#include "daarena.h"
#include "daasteroid.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dadesigner.h"
#include "dadesignplane.h"
#include "daeffect.h"
#include "dagalaxy.h"
#include "daitem.h"
#include "damissile.h"
#include "daperson.h"
#include "dasc.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dastar.h"
#include "dateleporter.h"
#include "dauniverse.h"
#include "davehicle.h"
#include "daworldgasgiant.h"
#include "daworldglobe.h"
#include "daworldring.h"
#include "daworldringsection.h"
#include "dgdata.h"
#include "dtsspacket.h"
#include <QDataStream>

DtClient::DtClient(QObject* parent) : QThread(parent), m_semAlive(1)
{
	m_BytesRx = 0;
	m_BytesTx = 0;
	m_ClientUnitEmpire = 0;
	m_EmpireDataIncoming = false;
	m_HasClientUnitEmpire = false;
	m_isConnected = false;
	m_PacketSize = 0;
	m_PacketSizeBytesLeft = 0;
	m_semAlive.acquire(1);			// I live...
}

DtClient::~DtClient()
{
}

void DtClient::run()
{
	// Creating the socket on the stack automatically associates it with the thread's
	// event loop. This ensures that Qt will not try to deliver events to our socket
	// from the main thread while we are accessing it from DtClient::run().
	QTcpSocket tcpSocket;
	//connect(&tcpSocket, SIGNAL(readyRead()), SLOT(RxData()));
	//connect(&tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
			//SLOT(NetworkError(QAbstractSocket::SocketError)));

	// Connect to server.
	tcpSocket.abort();
	tcpSocket.connectToHost(g_Au.SceneServer(), g_Au.ScenePort());
	tcpSocket.waitForConnected(10000);
	if(tcpSocket.state() == QAbstractSocket::UnconnectedState)
	{
		// The connection attempt failed. The server or the network is probably not up.
		//qWarning("DtClient::run - tcp connection attempt failed");
		return;
	}
	m_isConnected = true;
	emit Connected();
	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		g_Au.Log(g_Au.CacheApp() + "/soh.txt",
				QString("Client: Connected to %1 port %2").arg(g_Au.SceneServer()).arg(g_Au.ScenePort()));

	// Request basic game data.
	DgData& gd = g_Au.LockDataForWrite();
	bool needsData = gd.LoadBasicGameData();
	g_Au.UnlockData();
	if(needsData)
		auApp->RequestBasicGameData();

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

	// We don't like to disconnect.
	m_isConnected = false;
	if(Alive() && gp_AuGlobal)
	{
		// Defer back to the master.
		m_PacketSize = 0;
		m_PacketSizeBytesLeft = 0;

		// Clear local atom cache.
		g_Au.ClearEtherium();

		// Stop the scene synchronizer until we get the first full scene from the new scene server.
		emit SceneDisconnectedToDefer();

		// Reset connection values to identify the master server.
		g_Au.ResetSceneServer();

		// Attempt to reconnect to the master.
		tcpSocket.connectToHost(g_Au.SceneServer(), g_Au.ScenePort());
		tcpSocket.waitForConnected(10000);
		if(tcpSocket.state() != QAbstractSocket::UnconnectedState)
		{
			// Signal that scene was deferred to next server. This will clear the scene model and view data.
			m_isConnected = true;
			if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
				g_Au.Log(g_Au.CacheApp()+"/soh.txt", "Client: Scene deferred, connection lost");
			emit SceneDeferred(AuApp::DeferScene);
		}

		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("Client: Disconnected"));
	}
}

void DtClient::NetworkError(QAbstractSocket::SocketError socketError)
{
	switch(socketError)
	{
	case QAbstractSocket::RemoteHostClosedError:
		qWarning("DtClient::NetworkError - RemoteHostClosedError");
		break;
	case QAbstractSocket::HostNotFoundError:
		qWarning("DtClient::NetworkError - HostNotFoundError");
		break;
	case QAbstractSocket::ConnectionRefusedError:
		qWarning("DtClient::NetworkError - ConnectionRefusedError");
		break;
	default:
		//if(m_Tcp)
			//qWarning("DtClient::NetworkError - Error %s",
					//(const char*)m_Tcp->errorString());
		//else
			qWarning("DtClient::NetworkError - Error");
		break;
	}
}

void DtClient::RxData(QTcpSocket& tcpSocket)
{
	while(tcpSocket.bytesAvailable())
	{
		if(!gp_AuGlobal)
			return;

		g_Au.SetNet2Downloading(100);
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

void DtClient::TxData(QTcpSocket& tcpSocket)
{
	int written = auApp->TransmitSceneBuffer(tcpSocket);
	if(written)
	{
		m_BytesTx += written;
		m_LastComm.restart();
	}

	if(!written && m_LastComm.isValid() && m_LastComm.elapsed()/d_MSecPerSec > 60)
	{
		// Send a keep-alive packet when no data has been sent within the last minute.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PL_KeepAlive;
		int written = AuUtil::WritePacketSize(tcpSocket, ba.size());
		written += tcpSocket.write(ba);
		m_BytesTx += written;
		m_LastComm.restart();
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("Client: Keep alive packet sent."));
	}
}

bool DtClient::ReadPacket(const QByteArray& ba, QTcpSocket& tcpSocket)
{
	QDataStream block(ba);
	block.setVersion(QDataStream::Qt_4_1);

	// The first byte is always the packet type.
	DaAtom* atom = 0;	// This will be allocated if an atom is created.
	quint8 ptype;
	block >> ptype;
	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
#if defined(QT_NO_DEBUG)
		// Don't reveal packet type names in release build. Could be useful to hackers.
		g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("Client: Read packet type %1 size %L2.").arg(ptype).arg(ba.count()));
#else
		g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("Client: Read packet '%1' size %L2.")
				.arg(DtSceneServerPacket::PacketName((DtSceneServerPacket::PACKET)ptype)).arg(ba.count()));
#endif

	// Atom packets get special handling.
	if(ptype < DaAtom::ATOM_Count)
		atom = DaAtom::Create((DaAtom::ATOM_TYPE)ptype);	// The packet type is an atom.

	if(!atom)
	{
		// The packet was not an atom.
		// This cast generates a handy warning when an enum value is forgotten.
		switch((DtSceneServerPacket::PACKET)ptype)
		{
		case DtSceneServerPacket::PL_Login:	// Login success.
			{
				// Store message of the day.
				QString motd;
				quint16 chatPort;

				block >> motd;
				block >> chatPort;				// 0 in replies from scene master and scene manager.

				if(!motd.isEmpty())
					g_Au.Settings().writeEntry("Settings/MOTD", motd);
				g_Au.SetChatPort(chatPort);		// Scene worker server gives me the chat port number to connect to.

				emit SceneLoginSucceeded();

				// Forget the previous requests for empire and specie records when we have connected to a new server.
				m_EmpiresRequested.clear();
				m_SpeciesRequested.clear();
			}
			break;
		case DtSceneServerPacket::PL_LoginFailure:	// Login failure.
			{
				quint8 errCode;

				block >> errCode;

				emit LoginFailed(errCode);
			}
			break;
		case DtSceneServerPacket::PL_Logout:		// Server decision to log me out.
			emit LoggedOut();
			break;

		case DtSceneServerPacket::PE_EstDeferred:
			{
				QString toServer;
				block >> toServer;
				if(toServer == "127.0.0.1")
					toServer = g_Au.SceneServer();	// Stay with the same server ip.
				else if(toServer == AuUtil::SceneMasterName())
					toServer = g_Au.SceneMasterName();
				quint16 toPort;
				block >> toPort;
				quint8 srvmode;
				block >> srvmode;	// dest server mode
				block >> m_idDeferAtom;

				// Disconnect from current server.
				if(tcpSocket.state() != QAbstractSocket::UnconnectedState)
					tcpSocket.disconnectFromHost();
				if(tcpSocket.state() != QAbstractSocket::UnconnectedState)
					tcpSocket.waitForDisconnected(10000);
				m_isConnected = false;
				m_PacketSize = 0;
				m_PacketSizeBytesLeft = 0;

				// Stop the scene synchronizer until we get the first full scene from the new scene server.
				emit SceneDisconnectedToDefer();

				// Change our global scene server name.
				g_Au.SetSceneServer(toServer, toPort, srvmode);

				int tries = 0;
				while(tcpSocket.state() == QAbstractSocket::UnconnectedState && tries < 3)
				{
					// Connect to the scene server to which we were deferred.
					tcpSocket.connectToHost(g_Au.SceneServer(), g_Au.ScenePort());
					tcpSocket.waitForConnected(30000);
					tries++;
				}

				// Signal that init was deferred to next server.
				m_isConnected = true;
				emit EstDeferred(m_idDeferAtom);
			}
			break;

		case DtSceneServerPacket::PE_EstError:
			{
				unsigned char reason;
				block >> reason;
				if(reason == 13)
				{
					// Create new emperor failed to find a homeworld at the manager server. I need to go back to the master server
					// and send my new emperor info again.
					// Disconnect from current server.
					if(tcpSocket.state() != QAbstractSocket::UnconnectedState)
						tcpSocket.disconnectFromHost();
					if(tcpSocket.state() != QAbstractSocket::UnconnectedState)
						tcpSocket.waitForDisconnected(10000);
					m_isConnected = false;
					m_PacketSize = 0;
					m_PacketSizeBytesLeft = 0;

					// Stop the scene synchronizer until we get the first full scene from the new scene server.
					emit SceneDisconnectedToDefer();

					// Change our global scene server name.
					g_Au.ResetSceneServer();

					int tries = 0;
					while(tcpSocket.state() == QAbstractSocket::UnconnectedState && tries < 3)
					{
						// Connect to the scene server to which we were deferred.
						tcpSocket.connectToHost(g_Au.SceneServer(), g_Au.ScenePort());
						tcpSocket.waitForConnected(30000);
						tries++;
					}

					// Signal that init was deferred to next server.
					m_isConnected = true;
					emit RestartNewAvatar();
				}
				else if(reason == 15 || reason == 16)
				{
					// The reason indicates the assigned sector or planet was not
					// found on the server.  I am supposed to keep trying, expecting
					// the server to have the sector or planet loaded eventually.
					// Emitting the deferred signal gets the desired result.
					emit EstDeferred(m_idDeferAtom);
				}
				else
					emit StartNewAvatarError(reason);
			}
			break;
		case DtSceneServerPacket::PE_SceneDeferred:	// Deferred to another server for scene sync
			{
				QString toServer;
				block >> toServer;
				if(toServer == "127.0.0.1")
					toServer = g_Au.SceneServer();	// Stay with the same server ip.
				else if(toServer == AuUtil::SceneMasterName())
					toServer = g_Au.SceneMasterName();
				quint16 toPort;
				block >> toPort;
				quint8 srvmode;
				block >> srvmode;	// dest server mode

				// Clear local atom cache.
				g_Au.ClearEtherium();

				// Disconnect from current server.
				if(tcpSocket.state() != QAbstractSocket::UnconnectedState)
					tcpSocket.disconnectFromHost();
				if(tcpSocket.state() != QAbstractSocket::UnconnectedState)
					tcpSocket.waitForDisconnected(10000);
				m_isConnected = false;
				m_PacketSize = 0;
				m_PacketSizeBytesLeft = 0;

				// Stop the scene synchronizer until we get the first full scene from the new scene server.
				emit SceneDisconnectedToDefer();

				// Change our global scene server name.
				g_Au.SetSceneServer(toServer, toPort, srvmode);

				int tries = 0;
				while(tcpSocket.state() == QAbstractSocket::UnconnectedState && tries < 3)
				{
					// Connect to the scene server to which we were deferred.
					tcpSocket.connectToHost(g_Au.SceneServer(), g_Au.ScenePort());
					tcpSocket.waitForConnected(30000);
					tries++;
				}

				// Signal that scene was deferred to next server. This will clear the scene model and view data.
				m_isConnected = true;
				if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
					g_Au.Log(g_Au.CacheApp()+"/soh.txt", QString("Client: Scene deferred to %1:%2 mode %3")
							.arg(toServer).arg(toPort).arg(srvmode));
				emit SceneDeferred(AuApp::DeferScene);
			}
			break;
		case DtSceneServerPacket::PE_Success:
			{
				quint32 idUnit;
				block >> idUnit;
				emit InitSucceeded(idUnit);
			}
			break;

		case DtSceneServerPacket::PG_Data:	// end of game data
			{
				DgData& gd = g_Au.LockDataForWrite();
				gd.SetBasicGameDataReceived();
				g_Au.UnlockData();
			}
			break;
		case DtSceneServerPacket::PG_Empire:			// Empire rec from server.
			{
				// This is the beginning of a potentially large batch of empire records. Notify the user interface.
				if(!m_EmpireDataIncoming)
				{
					m_EmpireDataIncoming = true;
					emit EmpireDataBegin();
				}

				DgEmpire rec;
				rec.Rx(block);
				bool lastEmpire;			// true if this is the last empire record in this batch/burst
				block >> lastEmpire;
				DgData& gd = g_Au.LockDataForWrite();
				gd.ReplaceEmpire(rec);	// Replaces existing record if it exists.
				g_Au.UnlockData();

				if(rec.m_id == m_ClientUnitEmpire)
				{
					// We now have the empire record for the scene atom's unit.
					m_HasClientUnitEmpire = true;

					// Build starmap data when connected to scene worker.
					if(g_Au.SceneServerMode() == 2)
						emit ClientUnitEmpireReceived(m_ClientUnitEmpire);
				}

				if(lastEmpire)
				{
					// This is the last empire record in the batch, emit signal to change user interfaces.
					m_EmpireDataIncoming = false;
					emit EmpireDataComplete();
				}
			}
			break;
		case DtSceneServerPacket::PG_Commodity:	// Commodity rec from server.
			{
				DbCommodity rec;
				rec.Rx(block);
				g_Au.m_CommodityArray.Add(rec);
			}
			break;
		case DtSceneServerPacket::PG_Industry:	// Industry type rec from server.
			{
				DbIndustry rec;
				rec.Rx(block);
				DgData& gd = g_Au.LockDataForWrite();
				gd.m_bIndustryArray.Add(rec);
				g_Au.UnlockData();
			}
			break;
		case DtSceneServerPacket::PG_ManufacturingProc:	// Manufacturing process rec from server.
			{
				DbManufacturingProcess rec;
				rec.Rx(block);
				DgData& gd = g_Au.LockDataForWrite();
				gd.m_bManufacturingProcessArray.append(rec);
				g_Au.UnlockData();
			}
			break;
		case DtSceneServerPacket::PG_ManufacturingComp:	// Manufacturing process component rec from server.
			{
				DbManufacturingComponent rec;
				rec.Rx(block);
				DgData& gd = g_Au.LockDataForWrite();
				gd.m_bManufacturingComponentArray.append(rec);
				g_Au.UnlockData();
			}
			break;
		case DtSceneServerPacket::P_SyncComplete:
			{
				AuTime serverUTC;
				serverUTC.Rx32(block);
				quint8 hasAtomIds;
				block >> hasAtomIds;

				// Server sends the list of non-permanent atom ids that belong in the scene, those that are !PermanentInScene.
				// When 'hasAtomIds' is 2, we reuse the list from the last scene sync.
				if(hasAtomIds == 1)
				{
					// Receive a fresh list of atom ids.
					m_SyncLastAtomIds.clear();
					quint16 idCount;
					block >> idCount;
					AuId id;
					while(idCount)
					{
						id << block;
						m_SyncLastAtomIds.insert(pair<AuId, bool>(id, true));
						idCount--;
					}
				}
				else if(hasAtomIds == 0)
				{
					// The list of atoms is empty; clear my list too.
					m_SyncLastAtomIds.clear();
				}

				// When 'hasAtomIds' is 2, we are reusing the list from the last scene sync.
				// There is no need to reconcile the scene again since the list didn't change.
				if(hasAtomIds != 2)
					ReconcileAtomsInScene();

				// Send the SyncComplete packet back to the server for ping time calculation.
				g_Au.ServerTimeUpdated(serverUTC);
				auApp->SceneSyncComplete(serverUTC);

				/*DHH This is also done in aumodel. Why is it done here?
				// Keep the star map up-to-date with the current unit's empire id. If it changes, the star map is erased.
				bool mapCleared = g_AuClient.StarMapSetEmpire(m_ClientUnitEmpire);	// Erases local star map if the empire changes.
				if(mapCleared)
					emit StarMapErased();
				*/
			}
			break;

		case DtSceneServerPacket::PL_SwitchAvatar:
			// Server wants us to switch avatars because we handed off our current avatar to somebody else.
			{
				AuId transferredId;			// Avatar id that was transferred to a different account.

				block >> transferredId;

				emit SwitchAvatarTransferred(transferredId);
			}
			break;

		case DtSceneServerPacket::P_Invalid:
		case DtSceneServerPacket::PE_DeletePerson:
		case DtSceneServerPacket::PE_StartNewAvatar:
		case DtSceneServerPacket::PG_EmpiresAll:
		case DtSceneServerPacket::PG_EmpiresJoinable:
		case DtSceneServerPacket::PG_StarMap:
		case DtSceneServerPacket::PI_Collision:
		case DtSceneServerPacket::PI_Control:
		case DtSceneServerPacket::PI_Deadhead:
		case DtSceneServerPacket::PI_EnteredWormhole:
		case DtSceneServerPacket::PI_Fell:
		case DtSceneServerPacket::PI_Position:
		case DtSceneServerPacket::PL_Setting:
		case DtSceneServerPacket::PL_KeepAlive:
		case DtSceneServerPacket::PZ_PlayersQuery:
		case DtSceneServerPacket::PZ_StatusQuery:
			qWarning("DtClient::ReadPacket - ERROR! UNEXPECTED PACKET TYPE %d", ptype);
			Q_ASSERT(false);
			break;
		}
		return true;
	}

	// All atom packets take similar handling now that we have allocated the proper object.
	atom->RxUpdateId(block);	// Pull in only the id and modify time.

	AuEther& ether = g_Au.LockEtheriumDelete();
	DaAtom* exa = ether.FindAtom(atom->Id());
	if(exa)
	{
		// The atom exists in the local data.
		// Verify proper parenting.
		exa->SetModifyTime(atom->ModifyTime());
		exa->RxUpdateBase(block);							// May load new parent id.

		// The nu atom is no longer needed and is discarded.
		delete atom;

		// The remainder of the update is sent directly to the existing atom.
		atom = exa;
		if(g_Au.m_DebugSwitches & 0x00008000)
		{
			QString logmsg = tr("DtClient::ReadPacket Update atom %1 %2 %3 bytes %4.")
				.arg(atom->Id().GetID())
				.arg(atom->Name())
				.arg(atom->Description())
				.arg(ba.size());
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", logmsg);
		}
	}
	else
	{
		// The atom does not exist in the local data.
		atom->RxUpdateBase(block);	// May load new parent id.
		atom->ConnectSignals();
		ether.AddAtom(atom);

		if(g_Au.m_DebugSwitches & 0x00008000)
		{
			// We haven't read enough data about this new atom to know its name.
			QString logmsg = tr("DtClient::ReadPacket New atom %1 bytes %2.")
				.arg(atom->Id().GetID())
				.arg(ba.size());
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", logmsg);
		}
	}

	quint32 empireId = 0;
	AuId atomId;
	if(atom)
	{
		// Receive the remainder of the update data now that parent is correct.
		// 'exa' is non-zero if we are updating an existing atom; 'exa' will be 0 the first time we receive an atom.
		atom->RxUpdate(block, exa?false:true);

		atomId = atom->Id();
		empireId = atom->EmpireId();

		// Notify app to ensure space ship design is loaded.
		DaBd* bd = dynamic_cast<DaBd*>(atom);
		if(bd)
		{
			quint32 designId = bd->DesignId();
			const AuBdDesign* design = bd->LockDesign();
			if(!design)
			{
				bd->UnlockDesign();
				emit BdDesignNeeded(designId);
			}
			else
			{
				// Insure any externally referenced designs are here too.
				QSet<quint32> xrefs;
				design->ExternalReferences(xrefs);
				bd->UnlockDesign();

				QSet<quint32> xrefsNeeded;
				const DgData& sdd = g_Au.LockBdDesignsForRead();
				for(QSet<quint32>::const_iterator xi = xrefs.cbegin(); xi != xrefs.cend(); xi++)
					if(!sdd.BdDesignIsLoaded(*xi))
						xrefsNeeded.insert(*xi);
				g_Au.UnlockBdDesigns();
				for(QSet<quint32>::const_iterator xi = xrefsNeeded.cbegin(); xi != xrefsNeeded.cend(); xi++)
					emit BdDesignNeeded(*xi);
			}
			if(!exa && designId)
			{
				// New building arrived so clear building map.
				DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(bd->Parent());
				if(llworld)
					llworld->ClearMaps();
			}
		}
		else
		{
			DaSc* ship = dynamic_cast<DaSc*>(atom);
			if(ship)
			{
				if(ship->DesignId())
				{
					const DgData& sdd = g_Au.LockScDesignsForRead();
					bool hasDesign = sdd.ScDesignIsLoaded(ship->DesignId());
					g_Au.UnlockScDesigns();
					if(!hasDesign)
						emit ScDesignNeeded(ship->DesignId());
				}
			}
			else
			{
				// Ensure we have specie data for all creatures.
				const DaCreature* creature = dynamic_cast<const DaCreature*>(atom);
				if(creature)
				{
					AuSpecies species = creature->Species();
					const DgData& gd = g_Au.LockDataForRead();
					bool hasSpecies = gd.HasSpecies(species);
					g_Au.UnlockData();
					if(!hasSpecies)
					{
						// Don't ask for specie records too often.
						QMap<AuSpecies, AuElapsedTimer>::const_iterator sri = m_SpeciesRequested.constFind(species);
						if(sri == m_SpeciesRequested.constEnd() || (*sri).elapsed()/d_MSecPerSec)
						{
							// We haven't requested the species record or it's been a second or more since the last request.
							auApp->SpeciesRequested(species);
							AuElapsedTimer now;
							now.start();
							m_SpeciesRequested.insert(species, now);		// Update last request time.
						}
					}
				}
				else
				{
					// Galaxy sector locations cache must be dirtied when a new sector arrives.
					const DaSector* sector = dynamic_cast<const DaSector*>(atom);
					if(sector)
					{
						DaGalaxy* galaxy = dynamic_cast<DaGalaxy*>(sector->Parent());
						if(galaxy)
							galaxy->InvalidateSectorLocCache();			// Cache will be recreated when needed.
					}
					else
					{
						// When a world arrives, it tells its parent companion star to update its zone of control.
						const DaWorld* world = dynamic_cast<const DaWorld*>(atom);
						if(world)
						{
							DaStar* star = dynamic_cast<DaStar*>(world->Parent());
							if(star && !star->IsPrimary())
								star->CalculateZoneOfControl();
						}
						else
						{
							//DHH Item model
						}
					}
				}
			}
		}
	}
	g_Au.UnlockEtheriumDelete();

	// Insure empire data is available locally.
	// This will pick up all units (animals, people, spaceships) and cities.
	if(!atomId.IsNull() && empireId)
	{
		const DgData& gd = g_Au.LockDataForRead();
		bool hasEmpire = gd.HasEmpire(empireId);
		if(gd.ClientUnitId() == atomId)
		{
			m_ClientUnitEmpire = empireId;
			m_HasClientUnitEmpire = hasEmpire;			// Constantly updated because client unit can change.
		}
		g_Au.UnlockData();

		// Don't request the same empire more than once from a given server. The list is cleared when we change servers.
		// When requesting an empire by id, the server always replies, even if it's a bogus empire record named Forgotten
		// Empire NNN.
		if(!hasEmpire && !m_EmpiresRequested.contains(empireId))
		{
			if(g_Au.m_DebugSwitches & 0x00000004)
			{
				QString logmsg = tr("DtClient::ReadPacket Requesting empire %3.").arg(empireId);
				g_Au.Log(g_Au.CacheApp() + "/soh.txt", logmsg);
			}
			m_EmpiresRequested.insert(empireId);
			emit EmpireRequested(empireId);
		}
	}
	return true;
}

// Remove atoms from my data that are not permanent in the scene and not listed in my atoms-in-scene list.
void DtClient::ReconcileAtomsInScene()
{
	// Remove unwanted atoms in reverse order so subordinates get removed before their parents.
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaUniverse* univ = ether.FindUniverse();
	if(!univ)
	{
		g_Au.UnlockEtheriumDelete();
		return;
	}
	DaAtomList myAtoms;
	univ->ListSubatomsRecursive(myAtoms);	// This gets them in descending order, top of tree to leaves.
	for(DaAtomList::iterator mai = myAtoms.end()-1; mai != myAtoms.begin()-1; mai--)
	{
		/*DHH This is also done in aumodel. Why is it done here?
		// Update star map with any sectors that are not in the star map already.
		// This must wait until we receive the client unit's empire record or city data will be missing.
		if(m_HasClientUnitEmpire)
		{
			const DaSector* sector = dynamic_cast<const DaSector*>(*mai);
			if(sector && !g_AuClient.StarMapHasSector(sector->Id()))
				emit StarMapAddSector(sector->Id());
		}
		*/

		// Skip atoms that are permanent in the scene. The server does not send us their ids over and over again.
		if((*mai)->IsPermanentInScene())
			continue;
		if(m_SyncLastAtomIds.find((*mai)->Id()) == m_SyncLastAtomIds.end())
		{
			// The non-permanent atom must be removed from the scene.
			if((*mai)->HasSubatoms())
				continue;	// Ignore this one. It's children could be later in the list and we'll die if it deletes them too.

			DaBd* bd = dynamic_cast<DaBd*>(*mai);
			if(bd && bd->HasEnclosedEnvironment())
			{
				// Enclosure updates parent world's cache when it departs.
				DaWorldLL* world = bd->FindParentWorldInEnv();
				ether.TrashAtom((*mai)->Id());
				if(world)
				{
					world->InvalidateEnclosureCache();
					world->InvalidateEnvironmentRecursive();
				}
			}
			else
				ether.TrashAtom((*mai)->Id());
		}
	}

#if !defined(QT_NO_DEBUG)
	//DHH Check for orphans. This check may be costly. Get rid of it after no messages for a while.
	DaAtomList orphans;
	ether.ListOrphans(orphans);
	if(!orphans.empty())
	{
		qDebug() << "DtClient::ReconcileAtomsInScene" << orphans.size() << "orphans";
		for(DaAtomList::const_iterator oi = orphans.cbegin(); oi != orphans.cend(); oi++)
			qDebug() << "    " << *(*oi) << "parent" << (*oi)->ParentId();
	}
#endif
	g_Au.UnlockEtheriumDelete();
}

