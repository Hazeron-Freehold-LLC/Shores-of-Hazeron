//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtclientchannel.cpp,v 1.283 2022/04/15 20:40:31 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dtclientchannel.h"
//
#include "audbglobal.h"
#include "aujoboffer.h"
#include "auplayeronlinestatus.h"
#include "ausceneserverapp.h"
#include "daatom.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dauniverse.h"
#include "dgdata.h"
#include "dtserverchannel.h"
#include <QDataStream>

DtClientChannel::DtClientChannel(QObject* parent) : QThread(parent), m_semAlive(1)
{
	m_AwaitingAtomIdRestock = false;
	m_AwaitingBuildingDesignIdRestock = false;
	m_AwaitingItemDesignIdRestock = false;
	m_AwaitingShipDesignIdRestock = false;
	m_AwaitingStoryIdRestock = false;
	m_BytesRx = 0;
	m_BytesTx = 0;
	m_PacketSize = 0;
	m_PacketSizeBytesLeft = 0;
	m_semAlive.acquire(1);			// I live...
}

DtClientChannel::~DtClientChannel()
{
}

void DtClientChannel::run()
{
	//qDebug("DtClientChannel::run");
	// Creating the socket on the stack automatically associates it with the thread's
	// event loop. This ensures that Qt will not try to deliver events to our socket
	// from the main thread while we are accessing it from DtClientChannel::run().
	QTcpSocket tcpSocket;
	//connect(&tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
			//SLOT(NetworkError(QAbstractSocket::SocketError)));

	// Connect to server.
	//tcpSocket.abort();
	tcpSocket.connectToHost(g_Au.ClientChannelServer(), g_Au.ServerPort());
	tcpSocket.waitForConnected(10000);
	if(tcpSocket.state() != QAbstractSocket::ConnectedState)
	{
		// The connection attempt failed. The server or the network is probably not up.
		g_Au.Warning("Client Channel TCP connection to server attempt failed.");
		return;
	}
	emit Connected();

	// Send an initial status packet so the server has a chance to defer me right off.
	SendStatus();

	const int secsToCheckIn = 60;	// Worker checks in every minute.
	AuElapsedTimer tmStatus;
	while(Alive() && tcpSocket.state() == QAbstractSocket::ConnectedState &&
			!tcpSocket.waitForDisconnected(10))
	{
		// Send anything that's ready to go.
		TxData(tcpSocket);

		// Pull in any data that is waiting for us.
		// Note: If we wait for the readyRead signal, it comes from the gui thread,
		// which causes us grief. It's better for us to get the data when it's there.
		RxData(tcpSocket);

		// Restock local id values from boss.
		RestockNextIds();

		// Send a status packet to the server periodically so it knows I'm alive, not
		// just "connected".
		if(!tmStatus.isValid() || tmStatus.elapsed()/d_MSecPerSec >= secsToCheckIn)
		{
			SendStatus();
			tmStatus.restart();
		}
	}
	//qDebug("DtClientChannel::run - Disconnected.");
}

void DtClientChannel::NetworkError(QAbstractSocket::SocketError socketError)
{
	switch(socketError)
	{
	case QAbstractSocket::RemoteHostClosedError:
		g_Au.Warning("DtClientChannel::NetworkError - RemoteHostClosedError");
		break;
	case QAbstractSocket::HostNotFoundError:
		g_Au.Warning("DtClientChannel::NetworkError - HostNotFoundError");
		break;
	case QAbstractSocket::ConnectionRefusedError:
		g_Au.Warning("DtClientChannel::NetworkError - ConnectionRefusedError");
		break;
	default:
		//if(m_Tcp)
			//g_Au.Warning("DtClientChannel::NetworkError - Error %s",
					//(const char*)m_Tcp->errorString());
		//else
			g_Au.Warning("DtClientChannel::NetworkError - Error");
		break;
	}
}

void DtClientChannel::RxData(QTcpSocket& tcpSocket)
{
	//qDebug("DtClientChannel::RxData");
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
		if(!ReadPacket(block, tcpSocket))
		{
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.disconnectFromHost();
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.waitForDisconnected(10000);
		}
	}
}

void DtClientChannel::TxData(QTcpSocket& tcpSocket)
{
	// Check to see if we have anything to send in the tcp stream.
	int written = 0;
	//if(!m_TxBuffer.isEmpty())
		//qDebug("DtClientChannel::TxData");
	m_muTxBuffer.lock();
	if(!m_TxBuffer.isEmpty())
	{
		// There is data to be sent.
		// It is already formatted into packets with block sizes.
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			written = tcpSocket.write(m_TxBuffer);
			m_BytesTx += written;
			m_TxBuffer.clear();
		}
	}
	m_muTxBuffer.unlock();
	//if(written)
		//qDebug("DtClientChannel::TxData - %d bytes written", written);
}

bool DtClientChannel::ReadPacket(const QByteArray& ba, QTcpSocket& tcpSocket)
{
	// NOTE: The scene master does not create a client channel.
	//Q_ASSERT(!g_Au.IsSceneMaster());

	QDataStream block(ba);
	block.setVersion(QDataStream::Qt_4_1);

	// The first byte is always the packet type.
	quint8 ptype;
	block >> ptype;

	// Tally count of packets by type.
	QHash<quint32, quint32>::iterator pri = m_PacketsRx.find(ptype);
	if(pri == m_PacketsRx.end())
		m_PacketsRx.insert(ptype, 1);
	else
		(*pri)++;

	switch((DtServerChannel::PACKET)ptype)	// The cast generates a warning when we forget to handle a packet value.
	{
	case DtServerChannel::P_AvatarAccountActive:
		{
			AuIdList land;
			block >> land;
			emit AvatarAccountActiveSignal(land);
		}
		break;

	case DtServerChannel::P_RedeemShipTitle:
		{
			AuId propertyId, ownerId;
			QString propertyName, ownerName;
			quint32 ownerEmpire;
			block >> propertyId;
			block >> propertyName;
			block >> ownerId;
			block >> ownerName;
			block >> ownerEmpire;
			emit ClaimTitleSignal(propertyId, propertyName, ownerId, ownerName, ownerEmpire);
		}
		break;

	case DtServerChannel::P_SectorPirateBase:
		{
			AuId inSystemId;
			block >> inSystemId;
			emit PirateBaseEncountered(inSystemId);
		}
		break;

	case DtServerChannel::P_BlueprintRatedBd:
		{
			AuExchangeRatingRec rating;
			block >> rating;
			emit BlueprintRatedBd(rating);
		}
		break;

	case DtServerChannel::P_BlueprintRatedSc:
		{
			AuExchangeRatingRec rating;
			block >> rating;
			emit BlueprintRatedSc(rating);
		}
		break;

	case DtServerChannel::P_CollectBounty:
		{
			AuId bountyHunter;
			QString bountyHunterName;
			AuGearPos gearPos;
			AuBountyOnItem bountyOnItem;
			block >> bountyHunter;
			block >> bountyHunterName;
			block >> gearPos;
			block >> bountyOnItem;
			emit CollectBounty(bountyHunter, bountyHunterName, gearPos, bountyOnItem);
		}
		break;

	case DtServerChannel::P_PayBounty:
		{
			AuId bountyHunter;
			AuGearPos gearPos;
			AuBountyOnItem bountyOnItem;
			bool victimHadBounty;
			block >> bountyHunter;
			block >> gearPos;
			block >> bountyOnItem;
			block >> victimHadBounty;
			emit PayBounty(bountyHunter, gearPos, bountyOnItem, victimHadBounty);
		}
		break;

	case DtServerChannel::P_BdDesignExchangePublished:
		{
			AuBdDesignExchangeEntry item;

			block >> item;

			emit BdDesignExchangePublishedSignal(item);
		}
		break;

	case DtServerChannel::P_BdDesignExchangeUnpublished:
		{
			quint32 designId;

			block >> designId;

			emit BdDesignExchangeUnpublishedSignal(designId);
		}
		break;

	case DtServerChannel::P_ItDesignDeactivated:
		{
			quint32 designId;

			block >> designId;

			emit ItDesignDeactivatedSignal(designId);
		}
		break;

	case DtServerChannel::P_BdDesignDeactivated:
		{
			quint32 designId;

			block >> designId;

			emit BdDesignDeactivatedSignal(designId);
		}
		break;

	case DtServerChannel::P_ScDesignExchangePublished:
		{
			AuScDesignExchangeEntry item;

			block >> item;

			emit ScDesignExchangePublishedSignal(item);
		}
		break;

	case DtServerChannel::P_ScDesignExchangeUnpublished:
		{
			quint32 designId;

			block >> designId;

			emit ScDesignExchangeUnpublishedSignal(designId);
		}
		break;

	case DtServerChannel::P_ScDesignDeactivated:
		{
			quint32 designId;

			block >> designId;

			emit ScDesignDeactivatedSignal(designId);
		}
		break;

	case DtServerChannel::P_ShipStasisRecover:
		{
			AuId shipId;
			AuId personId;
			quint8 personGalaxy;
			bool agentRecover;

			block >> shipId;
			block >> personId;
			block >> personGalaxy;
			block >> agentRecover;

			emit SpacecraftStasisRecover(shipId, personId, personGalaxy, agentRecover);
		}
		break;

	case DtServerChannel::P_HengeDestination:
		{
			AuId sectorId;
			AuId systemId;
			QString systemName;
			quint8 sun;
			QString worldCatName;
			QString worldName;
			quint8 worldType;
			AuGlobeLLF hengeLL;
			bool breathable;
			quint8 orbitZone;
			bool outsideTown;

			block >> sectorId;
			block >> systemId;
			block >> systemName;
			block >> sun;
			block >> worldCatName;
			block >> worldName;
			block >> worldType;
			block >> hengeLL;
			block >> breathable;
			block >> orbitZone;
			block >> outsideTown;

			emit HengeDestinationConfirmed(sectorId, systemId, systemName, sun, worldCatName, worldName, worldType, hengeLL,
					breathable, orbitZone, outsideTown);
		}
		break;

	case DtServerChannel::P_HengeActivated:
		{
			AuId fromSysId;
			quint8 fromSun;
			quint8 fromShadowPanels;
			quint8 fromArcSegment;
			AuId destSysId;
			quint8 destSun;
			quint8 destArcSegment;

			block >> fromSysId;
			block >> fromSun;
			block >> fromShadowPanels;
			block >> fromArcSegment;
			block >> destSysId;
			block >> destSun;
			block >> destArcSegment;

			emit HengeActivatedSignal(fromSysId, fromSun, fromShadowPanels, fromArcSegment, destSysId, destSun, destArcSegment);
		}
		break;

	case DtServerChannel::P_HengeCreated:
		{
			AuId fromSysId;
			quint8 fromSun;
			quint8 fromShadowPanels;
			quint8 fromArcSegment;
			AuId destSysId;
			quint8 destSun;
			quint8 destArcSegment;

			block >> fromSysId;
			block >> fromSun;
			block >> fromShadowPanels;
			block >> fromArcSegment;
			block >> destSysId;
			block >> destSun;
			block >> destArcSegment;

			emit HengeCreatedSignal(fromSysId, fromSun, fromShadowPanels, fromArcSegment, destSysId, destSun, destArcSegment);
		}
		break;

	case DtServerChannel::P_HengeDeactivated:
		{
			AuId fromSysId;
			quint8 fromSun;
			quint8 fromShadowPanels;
			quint8 fromArcSegment;
			AuId destSysId;
			quint8 destSun;
			quint8 destArcSegment;

			block >> fromSysId;
			block >> fromSun;
			block >> fromShadowPanels;
			block >> fromArcSegment;
			block >> destSysId;
			block >> destSun;
			block >> destArcSegment;

			emit HengeDeactivatedSignal(fromSysId, fromSun, fromShadowPanels, fromArcSegment, destSysId, destSun, destArcSegment);
		}
		break;

	case DtServerChannel::P_ArenaReady:
		{
			AuArenaReady arena;

			block >> arena;

			emit ArenaReady(arena);
		}
		break;

	case DtServerChannel::P_SpacecraftNameChanged:
		{
			AuId shipId;
			QString nuName;

			block >> shipId;
			block >> nuName;

			emit SpacecraftNameChangedDown(shipId, nuName);
		}
		break;

	case DtServerChannel::P_StargateClosed:
		{
			AuId fromGalaxy;
			AuPoint fromGalaxyLoc;

			block >> fromGalaxy;
			block >> fromGalaxyLoc;

			emit CloseStargateDown(fromGalaxy, fromGalaxyLoc);
		}
		break;

	case DtServerChannel::P_StargateOpened:
		{
			AuId fromGalaxy, toGalaxy, toSector, toSystem;
			AuPoint fromGalaxyLoc, toGalaxyLoc;
			AuTime expire;

			block >> fromGalaxy;
			block >> fromGalaxyLoc;
			block >> toGalaxy;
			block >> toGalaxyLoc;
			block >> toSector;
			block >> toSystem;
			expire << block;

			emit OpenStargateDown(fromGalaxy, fromGalaxyLoc, toGalaxy, toGalaxyLoc, toSector, toSystem, expire);
		}
		break;

	case DtServerChannel::P_CityMailRequested:
		{
			AuId cityId, unitId;
			quint32 unitEmpire;

			block >> cityId;
			block >> unitId;
			block >> unitEmpire;

			emit CityMailRequestedSignal(cityId, unitId, unitEmpire);
		}
		break;

	case DtServerChannel::P_PersonDeleted:
		{
			AuId personId;

			block >> personId;

			emit PersonDeleted(personId);
		}
		break;

	case DtServerChannel::P_SystemWarpSignature:
		{
			AuId systemId;
			quint32 warpFactors;
			quint8 galaxy;	// GALAXY enum value
			float galaxyLocX;
			float galaxyLocY;
			float galaxyLocZ;

			block >> systemId;
			block >> warpFactors;
			block >> galaxy;
			block >> galaxyLocX;
			block >> galaxyLocY;
			block >> galaxyLocZ;

			emit SystemWarpSignatureDown(systemId, warpFactors, galaxy, galaxyLocX, galaxyLocY, galaxyLocZ);
		}
		break;

	case DtServerChannel::P_SectorDecayed:
		{
			AuId sectorId;

			block >> sectorId;

			emit SectorDecayed(sectorId);
		}
		break;

	case DtServerChannel::P_SpawnPersonOnPlanet:
		{
			AuId personId;
			AuId sectorOrWorldId;	// Sector id is sent to manager; world id is sent to worker.
			quint8 galaxy;			// GALAXY enum value

			block >> personId;
			block >> sectorOrWorldId;
			block >> galaxy;

			emit SpawnPersonOnPlanet(personId, sectorOrWorldId, galaxy);
		}
		break;

	case DtServerChannel::P_SectorVisited:
		{
			/*
			AuId sectorId;
			quint32 empireId;

			block >> sectorId;
			block >> empireId;

			emit SectorVisited(sectorId, empireId);
			*/
		}
		break;

	case DtServerChannel::P_AvatarOnLine:
		{
			AuId personId;
			quint32 empireId;

			block >> personId;
			block >> empireId;

			emit AvatarOnLine(personId, empireId);
		}
		break;

	case DtServerChannel::P_RenamePlanet:
		{
			AuId planetId, systemId, sectorId;
			QString nuName, systemName, sectorName;

			block >> planetId;
			block >> nuName;
			block >> systemId;
			block >> systemName;
			block >> sectorId;
			block >> sectorName;

			emit RenamePlanet(planetId, nuName, systemId, systemName, sectorId, sectorName);
		}
		break;

	case DtServerChannel::P_RenameSector:
		{
			AuId sectorId;
			QString nuName;

			block >> sectorId;
			block >> nuName;

			emit RenameSector(sectorId, nuName);
		}
		break;

	case DtServerChannel::P_RenameSystem:
		{
			AuId systemId;
			QString nuName;

			block >> systemId;
			block >> nuName;

			emit RenameSystem(systemId, nuName);
		}
		break;

	case DtServerChannel::P_BountyPosted:
		{
			AuId posterId;
			QString posterName;
			AuId victimId;
			QString reason;
			double reward;
			quint8 why;

			block >> posterId;
			block >> posterName;
			block >> victimId;
			block >> reason;
			block >> reward;
			block >> why;

			emit BountyPosted(posterId, posterName, victimId, reason, reward, why);
		}
		break;

	case DtServerChannel::P_JobOffer:
		{
			AuJobOffer jobOffer;

			block >> jobOffer;

			// Only propagate if we got a valid job.
			if(jobOffer.Job())
			{
				emit PostJobOffer(jobOffer);
			}
		}
		break;

	case DtServerChannel::P_GovCitySurrendered:
		{
			AuId byWho;
			AuId cityId;
			quint32 fromEmpire;
			quint32 toEmpire;

			block >> byWho;
			block >> cityId;
			block >> fromEmpire;
			block >> toEmpire;

			emit GovCitySurrendered(byWho, cityId, fromEmpire, toEmpire);
		}
		break;

	case DtServerChannel::P_AtomSpawning:
		{
			AuId personOrShip, spawnCity, spawnShip, spawnAt, arena;
			quint32 empire;
			quint8 galaxy;	// GALAXY enum value

			block >> personOrShip;
			block >> empire;
			block >> spawnCity;
			block >> spawnShip;
			block >> spawnAt;
			block >> arena;
			block >> galaxy;

			emit AtomSpawning(personOrShip, empire, spawnCity, spawnShip, spawnAt, arena, galaxy);
		}
		break;

	case DtServerChannel::P_HengeTeleportAtom:
		{
			AuHengeTeleportAtom hengeTel;

			block >> hengeTel;

			emit HengeTeleportAtom(hengeTel);
		}
		break;

	case DtServerChannel::P_AtomTraveling:
		{
			AuId traveler, destSystem, destSector;
			traveler << block;
			destSystem << block;
			destSector << block;
			//qDebug("DtClientChannel::ReadPacket P_AtomTraveling traveler %d destSystem %d destSector %d",
					//traveler.GetID(), destSystem.GetID(), destSector.GetID());
			emit AtomTraveling(traveler, destSystem, destSector);
		}
		break;
	case DtServerChannel::P_HandoffElsewhere:
		{
			AuId person, dest, returnOnFail;
			quint8 rideAlso;
			bool limboRescue;
			QByteArray traveller;

			block >> person;
			block >> dest;
			block >> rideAlso;
			block >> limboRescue;
			block >> traveller;
			block >> returnOnFail;

			emit HandoffElsewhere(person, dest, rideAlso, limboRescue, traveller, returnOnFail);
		}
		break;
		/*
	case DtServerChannel::P_SpawnPersonElsewhere:
		{
			AuId person, spawnCity, spawnShip, spawnAt;
			quint32 empire;
			quint8 isAlternate;
			QByteArray traveller;

			person << block;
			block >> empire;
			spawnCity << block;
			spawnShip << block;
			spawnAt << block;
			block >> isAlternate;
			block >> traveller;

			emit SpawnPersonElsewhere(person, empire, spawnCity, spawnShip, spawnAt, isAlternate, traveller);
		}
		break;
	case DtServerChannel::P_SpawnShipElsewhere:
		{
			AuId ship, spawnCity, spawnAt;
			quint32 empire;
			quint8 isAlternate;
			QByteArray traveller;

			ship << block;
			block >> empire;
			spawnCity << block;
			spawnAt << block;
			block >> isAlternate;
			block >> traveller;

			emit SpawnShipElsewhere(ship, empire, spawnCity, spawnAt, isAlternate, traveller);
		}
		break;
	case DtServerChannel::P_TransferElsewhere:
		{
			AuId id, destSystem, destSector;
			QByteArray traveller;

			id << block;
			destSystem << block;
			destSector << block;
			block >> traveller;

			//qDebug("DtClientChannel::ReadPacket P_TransferElsewhere id %d system %d sector %d data %d bytes",
					//id.GetID(), destSystem.GetID(), destSector.GetID(), traveller.count());
			emit TravelElsewhere(id, destSystem, destSector, traveller);
		}
		break;
		*/

	case DtServerChannel::P_SummonDormantShip:
		{
			AuId shipId, crewId;
			block >> shipId;
			block >> crewId;
			emit SummonDormantShip(shipId, crewId);	// Propagate down when received on this client channel.
		}
		break;

	case DtServerChannel::P_AdminSummon:
		{
			AuId person, adminId;
			quint8 rideAlso;
			block >> person;
			block >> adminId;
			block >> rideAlso;
			emit AdminSummon(person, adminId, rideAlso);	// Propagate down when received on this client channel.
		}
		break;

	case DtServerChannel::P_CreditCityBank:
		{
			float fAmount;
			AuId cityId;
			block >> fAmount;
			cityId << block;

			emit CreditCityBank(fAmount, cityId);
		}
		break;

	case DtServerChannel::P_OfficerKilled:
		{
			AuId homeWorldId;
			homeWorldId << block;
			emit OfficerKilled(homeWorldId);
		}
		break;
	case DtServerChannel::P_SendFunds:
		{
			AuId fromId;
			QString fromName;
			AuId toId;
			double amount;
			quint8 fromGalaxy;
			quint8 reason;
			quint32 designId;

			block >> fromId;
			block >> fromName;
			block >> toId;
			block >> amount;
			block >> fromGalaxy;
			block >> reason;
			block >> designId;

			emit SendFunds(fromId, fromName, toId, amount, fromGalaxy, reason, designId);
		}
		break;

	case DtServerChannel::P_AtomIds:	// Sent by boss to sub with atom ids.
		{
			AuId nextId;
			quint16 howMany;
			nextId << block;
			block >> howMany;
			g_AuDb.RestockAtomIds(nextId, howMany);
			m_AwaitingAtomIdRestock = false;

			if(g_Au.m_LogMessages & LOG_AtomCreate)
			{
				QString logmsg;
				logmsg = tr("Stock of %1 atom ids received starting at %2.").arg(howMany).arg(nextId.GetID());
				g_Au.Log(logmsg);
			}
		}
		break;

		/*
	case DtServerChannel::P_AtomSpawn:	// Sent by boss to sub with person or ship id and spawn location id.
		{
			AuId personOrShip, spawnAt;
			quint8 isAlternate;
			personOrShip << block;
			spawnAt << block;
			block >> isAlternate;	// true if this is the alternate capitol of a different empire

			//qDebug("DtClientChannel::ReadPacket P_AtomSpawn %d at %d", personOrShip.GetID(), spawnAt.GetID());
			if(g_Au.IsSceneWorker())
			{
				// The worker must load the atom to spawn and set its
				// spawn location. This requires the db thread to be involved.
				g_AuDb.SpawnAtom(personOrShip, spawnAt, isAlternate);
			}
			else
			{
				// This is a scene manager.
				// Emit a signal that will cause the spawn message to propagate
				// down to the worker that has the spawnAt atom.
				emit AtomSpawn(personOrShip, spawnAt, isAlternate);
			}
		}
		break;
		*/

	case DtServerChannel::P_Chat:	// Chat message coming down from my boss.
		{
			quint8 msgType;
			block >> msgType;
			AuCommPacket* msg = AuCommPacket::CreateCommPacket((AuCommPacket::TYPE)msgType);
			block >> *msg;

			// Bundle the message into a byte array so it can be emitted as a signal.
			QByteArray bamsg;
			QDataStream out(&bamsg, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << msgType;
			out << *msg;
			delete msg;

			// This message is on the way down the server chain since it arrived on the client channel.
			emit ChatDown(bamsg);
		}
		break;

	case DtServerChannel::P_OpenChannelTo:	// Open-channel request coming down from my boss.
		{
			AuId fromUnitId, fromSector, toUnitId;
			QString channelName, fromName;
			quint8 fromGalaxy, scope;

			block >> fromUnitId;
			block >> fromName;
			block >> fromSector;
			block >> fromGalaxy;
			block >> toUnitId;
			block >> channelName;
			block >> scope;

			// This message is on the way down the server chain since it arrived on the client channel.
			emit OpenChannelDown(fromUnitId, fromName, fromSector, fromGalaxy, toUnitId, channelName, scope);
		}
		break;

	case DtServerChannel::P_ReleaseBerth:
		{
			// My boss is propagating a berth release down the server tree.
			AuId personId, shipId;
			personId << block;		// Person releasing their berth.
			shipId << block;		// The ship they are moving to, may be 0.
			emit ReleaseBerth(personId, shipId);
		}
		break;

	case DtServerChannel::P_Deferred:
		{
			QString toServer;
			block >> toServer;

			// Disconnect from current server.
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.disconnectFromHost();
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
				tcpSocket.waitForDisconnected(10000);

			// Change our global scene server name.
			g_Au.SetClientChannelServer(toServer);

			// Connect to the scene server to which we were deferred.
			tcpSocket.connectToHost(g_Au.ClientChannelServer(), g_Au.ServerPort());
			tcpSocket.waitForConnected(10000);
			if(tcpSocket.state() != QAbstractSocket::ConnectedState)
			{
				// The connection attempt failed. The server or the network is probably not up.
				g_Au.Warning(QString("ERROR! Client Channel deferred to %1. Connection failed.").arg(g_Au.ClientChannelServer()));
				return false;
			}
			
			emit Connected();

			// Send an initial status packet so the server knows what mode I'm in.
			SendStatus();

			if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
				g_Au.Log("Client channel deferred to " + g_Au.ClientChannelServer());
		}
		break;

	case DtServerChannel::P_SpeciesChanged:		// Sent by boss to sub.
		{
			AuSpecies species;
			QString specieName;
			AuId discoverer;

			block >> species;
			block >> specieName;
			block >> discoverer;

			emit SpeciesChanged(species, specieName, discoverer);
		}
		break;

	case DtServerChannel::P_EmpireChanged:		// Sent by boss to sub.
		{
			DgEmpire empire;

			empire << block;

			emit EmpireChanged(empire);
		}
		break;

	case DtServerChannel::P_EmpireDeleted:		// Sent by boss to sub.
		{
			quint32 idEmpire;

			block >> idEmpire;

			emit EmpireRecordDeleted(idEmpire);
		}
		break;

	case DtServerChannel::P_OfficeAssigned:
		{
			AuId person, spawnCity, spawnSystem;
			quint32 empire;
			AuOffice office;
			QString spawnCityName, spawnSystemName;

			block >> person;
			block >> empire;
			block >> office;
			block >> spawnCity;
			block >> spawnCityName;
			block >> spawnSystem;
			block >> spawnSystemName;

			//qDebug("DtClientChannel::ReadPacket emits AssignOffice");
			emit AssignOffice(person, empire, office, spawnCity, spawnCityName, spawnSystem, spawnSystemName);
		}
		break;


	case DtServerChannel::P_QueryAtomPresence:
		{
			AuId idAtom;
			idAtom << block;
			quint32 empire;
			block >> empire;
			AuId atomToSpawn;		// DaPerson or DaVessel atom
			atomToSpawn << block;

			emit QueryAtomServer(idAtom, empire, atomToSpawn);
		}
		break;

	case DtServerChannel::P_QueryHengeTeleporterPresence:
		{
			AuHengeTeleportAtom hengeTel;

			block >> hengeTel;

			emit QueryHengeTeleporterServer(hengeTel);
		}
		break;

	case DtServerChannel::P_RenameWormhole:
		{
			AuId systemId, toSystem;
			qint8 polarity;
			quint32 empire;
			QString nuName;
			systemId << block;
			toSystem << block;
			block >> polarity;
			block >> empire;
			block >> nuName;
			
			emit RenameWormhole(systemId, toSystem, polarity, empire, nuName);
		}
		break;

	case DtServerChannel::P_SectorAssigned:
		if(g_Au.IsSceneManager())
		{
			// Only scene manager is assigned sectors.
			AuId idSector;
			quint8 systemsInSector;
			quint8 galaxy;	// GALAXY enum value

			block >> idSector;
			block >> systemsInSector;
			block >> galaxy;

			emit SectorAssigned(idSector, systemsInSector, galaxy);
		}
		else
		{
			g_Au.Warning(QString("HACKER WARNING! Client Channel received unexpected TCP packet type %1 size %2.")
					.arg(ptype).arg(ba.count()));
		}
		break;

	case DtServerChannel::P_SectorRevoked:
		if(g_Au.IsSceneManager())
		{
			// Only scene manager is assigned sectors.
			AuId idSector;
			idSector << block;
			emit SectorRevoked(idSector);
		}
		else
		{
			g_Au.Warning(QString("HACKER WARNING! Client Channel received unexpected TCP packet type %1 size %2.")
					.arg(ptype).arg(ba.count()));
		}
		break;

	case DtServerChannel::P_BuildingDesignIds:	// Sent by boss to sub with design ids.
		{
			quint32 nextId;
			quint16 howMany;
			block >> nextId;
			block >> howMany;
			g_AuDb.RestockBuildingDesignIds(nextId, howMany);
			m_AwaitingBuildingDesignIdRestock = false;

			if(g_Au.m_LogMessages & LOG_SpacecraftDesign)
			{
				QString logmsg;
				logmsg = tr("Stock of %1 building design ids received starting at %2.").arg(howMany).arg(nextId);
				g_Au.Log(logmsg);
			}
		}
		break;

	case DtServerChannel::P_ItemDesignIds:	// Sent by boss to sub with design ids.
		{
			quint32 nextId;
			quint16 howMany;
			block >> nextId;
			block >> howMany;
			g_AuDb.RestockItemDesignIds(nextId, howMany);
			m_AwaitingItemDesignIdRestock = false;

			if(g_Au.m_LogMessages & LOG_SpacecraftDesign)
			{
				QString logmsg;
				logmsg = tr("Stock of %1 item design ids received starting at %2.").arg(howMany).arg(nextId);
				g_Au.Log(logmsg);
			}
		}
		break;

	case DtServerChannel::P_ShipDesignIds:	// Sent by boss to sub with design ids.
		{
			quint32 nextId;
			quint16 howMany;
			block >> nextId;
			block >> howMany;
			g_AuDb.RestockSpaceShipDesignIds(nextId, howMany);
			m_AwaitingShipDesignIdRestock = false;

			if(g_Au.m_LogMessages & LOG_SpacecraftDesign)
			{
				QString logmsg;
				logmsg = tr("Stock of %1 space ship design ids received starting at %2.").arg(howMany).arg(nextId);
				g_Au.Log(logmsg);
			}
		}
		break;

	case DtServerChannel::P_StoryIds:	// Sent by boss to sub with story ids.
		{
			quint32 nextId;
			quint16 howMany;
			block >> nextId;
			block >> howMany;
			g_AuDb.RestockStoryIds(nextId, howMany);
			m_AwaitingStoryIdRestock = false;

			if(g_Au.m_LogMessages & LOG_Story)
			{
				QString logmsg;
				logmsg = tr("Stock of %1 story ids received starting at %2.").arg(howMany).arg(nextId);
				g_Au.Log(logmsg);
			}
		}
		break;

	case DtServerChannel::P_SystemAssigned:
		if(g_Au.IsSceneWorker())
		{
			// Only scene worker is assigned solar systems.
			AuId idSector;
			AuId idSystem;
			quint8 galaxy;	// GALAXY enum value

			block >> idSector;
			block >> idSystem;
			block >> galaxy;

			emit SystemAssigned(idSector, idSystem, galaxy);
		}
		else
		{
			g_Au.Warning(QString("HACKER WARNING! Client Channel received unexpected TCP packet type %d size %d.")
					.arg(ptype).arg(ba.count()));
		}
		break;

	case DtServerChannel::P_SystemRevoked:
		if(g_Au.IsSceneWorker())
		{
			// Only scene worker is assigned solar systems.
			AuId idSystem;
			idSystem << block;
			emit SystemRevoked(idSystem);
		}
		else
		{
			g_Au.Warning(QString("HACKER WARNING! Client Channel received unexpected TCP packet type %d size %d.")
					.arg(ptype).arg(ba.count()));
		}
		break;

	case DtServerChannel::P_ReloadStory:
		{
			quint32 storyInstance;
			block >> storyInstance;
			emit ReloadStory(storyInstance);
		}
		break;

	case DtServerChannel::P_RemoveStory:
		{
			quint32 storyInstance;
			block >> storyInstance;
			emit RemoveStory(storyInstance);
		}
		break;

	case DtServerChannel::P_SystemNameChanged:
		{
			AuId systemId, sectorId;
			QString systemName, sectorName;
			systemId << block;
			block >> systemName;
			sectorId << block;
			block >> sectorName;
			emit SystemNameChanged(systemId, systemName, sectorId, sectorName);
		}
		break;

	case DtServerChannel::P_SystemExplored:
		{
			AuId systemId;
			quint32 empireId;

			block >> systemId;
			block >> empireId;

			emit SystemExplored(systemId, empireId);
		}
		break;

	case DtServerChannel::P_SystemScanned:
		{
			AuId systemId;
			quint32 empireId;
			quint8 genHab;
			quint32 genSeed;

			block >> systemId;
			block >> empireId;
			block >> genHab;
			block >> genSeed;

			emit SystemScanned(systemId, empireId, genHab, genSeed);
		}
		break;

	case DtServerChannel::P_Trade:
		{
			quint8 doWhat;
			block >> doWhat;
			if(doWhat == 0)
			{
				// Deliver a TEU container.
				AuItemTEUContainer container;
				block >> container;
				emit ShipIt(container);
			}
			else if(doWhat == 1)
			{
				// Deliver trade proposal.
				AuTradeAgreement ta;
				block >> ta;
				emit ProposeTrade(ta);
			}
			else if(doWhat == 2)
			{
				// Deliver trade proposal.
				AuTradeAgreement ta;
				quint8 nuState;
				bool atDest;
				block >> ta;
				block >> nuState;
				block >> atDest;
				emit DeactivateTrade(ta, nuState, atDest);
			}
			else if(doWhat == 3)
			{
				// Accept trade proposal.
				AuTradeAgreement ta;
				block >> ta;
				emit AcceptTrade(ta);
			}
			else if(doWhat == 4)
			{
				// Trade ready to ship
				AuId readyCapitolId;
				AuTradeAgreement ta;
				block >> readyCapitolId;
				block >> ta;
				emit TradeReadyToShip(readyCapitolId, ta);
			}
			else if(doWhat == 5)
			{
				// Trade shipped
				AuId shipperCapitolId;
				AuTradeAgreement ta;
				block >> shipperCapitolId;
				block >> ta;
				emit TradeShipped(shipperCapitolId, ta);
			}
		}
		break;

	case DtServerChannel::P_Status:
		{
			AuPlayerOnlineStatusVector players;
			players << block;
			emit PlayerOnlineStatus(players);
		}
		break;

	case DtServerChannel::P_AddCitizen:
	case DtServerChannel::P_AtomPresence:
	case DtServerChannel::P_AtomTravelingGalaxyLoc:
	case DtServerChannel::P_AtomTravelingStargate:
	case DtServerChannel::P_CityCaptured:
	case DtServerChannel::P_CityCreated:
	case DtServerChannel::P_CityDecayed:
	case DtServerChannel::P_CityFacilities:
	case DtServerChannel::P_CityStatus:
	case DtServerChannel::P_EmpireChg:
	case DtServerChannel::P_EmpireChgFlag:
	case DtServerChannel::P_EmpireChgName:
	case DtServerChannel::P_EmpireChgRole:
	case DtServerChannel::P_EmpireCityWorkerDNAChanged:
	case DtServerChannel::P_EmpireRecordPatent:
	case DtServerChannel::P_EstNewEmpire:
	case DtServerChannel::P_GovBossChanged:
	case DtServerChannel::P_GovDossierAddDocument:
	case DtServerChannel::P_GovDossierChangeState:
	case DtServerChannel::P_GovSetAnnouncements:
	case DtServerChannel::P_GovSetCapital:
	case DtServerChannel::P_GovSetJoinPass:
	case DtServerChannel::P_GovSetCommerce:
	case DtServerChannel::P_GovSetContrailColor:
	case DtServerChannel::P_GovSetDefStance:
	case DtServerChannel::P_GovSetTax:
	case DtServerChannel::P_GovSetThemeColor:
	case DtServerChannel::P_GovSetWarCriteria:
	case DtServerChannel::P_HengeTeleporterPresence:
	case DtServerChannel::P_Invalid:
	case DtServerChannel::P_LeaveEmpire:
	case DtServerChannel::P_NameSpecies:
	case DtServerChannel::P_OfficeRevoked:
	case DtServerChannel::P_RenameAvatar:
	case DtServerChannel::P_RenameCity:
	case DtServerChannel::P_RequestAtomIds:
	case DtServerChannel::P_RequestBuildingDesignIds:
	case DtServerChannel::P_RequestItemDesignIds:
	case DtServerChannel::P_RequestShipDesignIds:
	case DtServerChannel::P_RequestStoryIds:
	case DtServerChannel::P_SetPoliticalStance:
	case DtServerChannel::P_SpacecraftPosition:
	case DtServerChannel::P_StoryChanged:
	case DtServerChannel::P_StoryDeleted:
	case DtServerChannel::P_SystemDecayed:
	case DtServerChannel::P_WarpSignatureDetected:
	case DtServerChannel::P_SystemFinal:
	case DtServerChannel::P_WormholeExplored:
		// These packet types are only sent by the sub server to the boss so
		// we should never receive them here.
		g_Au.Warning(QString("HACKER WARNING! Client Channel received unexpected TCP packet type %d size %d.")
				.arg(ptype).arg(ba.count()));
		break;

#if defined(QT_NO_DEBUG)	// allow warning for unhandled switch condition
	default:
		g_Au.Warning(QString("HACKER WARNING! Client Channel received UNKNOWN TCP packet type %d size %d.")
			.arg(ptype).arg(ba.count()));
		break;
#endif
	}
	return true;
}

// Used by the client to send an array of bytes to the server.
void DtClientChannel::SendPacket(const QByteArray& ba)
{
	// The packet is already formatted. Add it to the transmit data stream.
	//qDebug("DtClientChannel::SendPacket - tx %d bytes", ba.count());
	m_muTxBuffer.lock();
	AuUtil::WritePacketSize(m_TxBuffer, ba.size());
	m_TxBuffer.append(ba);
	m_muTxBuffer.unlock();
	//qDebug("DtClientChannel::SendPacket done");
}

void DtClientChannel::SendStatus()
{
	// NOTE: The scene master does not create a client channel.
	//Q_ASSERT(!g_Au.IsSceneMaster());

	quint8 serverMode;
	quint32 tickTime;
	unordered_map<AuId, AuSystemStatus, AuIdHash> systemStatus;
	quint16 workerCores;
	float atomLoad = 0;
	if(g_Au.IsSceneManager())
	{
		// Manager reports the average tick time of its workers. Its own tick time is not relevant since it doesn't tick anything.
		serverMode = 1;	// Manager
		tickTime = auSceneServerApp->AverageWorkerTickTime();	// This value is in 1/2 millisecond increments. i.e. 100 = 50ms
		workerCores = auSceneServerApp->WorkerCoreCount();		// Sum of cores available between all workers.
		atomLoad = auSceneServerApp->AverageWorkerLoadAverage();
	}
	else
	{
		// Worker reports its own tick time.
		serverMode = 2;	// Worker
		tickTime = auSceneServerApp->TotalTickTime();			// This value is in 1/2 millisecond increments. i.e. 100 = 50ms
		workerCores = auSceneServerApp->AtomManagerCount();
		atomLoad = auSceneServerApp->AtomManagerLoadAverage();

		// Worker reports info about clients in systems and average tick times so manager can balance loads intelligently.
		auSceneServerApp->CollectSystemStatus(systemStatus);
	}

	quint16 workers = auSceneServerApp->WorkerCount();			// Will be 0 on scene worker.
	AuPlayerOnlineStatusVector players;
	auSceneServerApp->PlayersOnlineStatus(players);				// Propagates the results up to master for complete list.

	QString netName = g_Au.InternetName();

	quint16 netPort = g_Au.ScenePort();

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtServerChannel::P_Status;
	out << serverMode;
	out << atomLoad;
	out << tickTime;
	out << workers;
	out << workerCores;
	players >> out;
	out << netName;
	out << netPort;
	quint16 atomMgrs = auSceneServerApp->AtomManagerCount();
	out << atomMgrs;
	out << (quint16)systemStatus.size();
	for(unordered_map<AuId, AuSystemStatus, AuIdHash>::const_iterator it = systemStatus.cbegin(); it != systemStatus.cend(); it++)
	{
		out << (*it).first;
		out << (*it).second.m_Clients;
		out << (*it).second.m_Tick;	// This value is in 1/2 millisecond increments. i.e. 100 = 50ms
	}
	SendPacket(ba);
}

// Test for need to restock back stock of next ids.
void DtClientChannel::RestockNextIds()
{
	// Scene worker must not request ids from the scene master.
	// NOTE: This prevents the scene manager from running at the same IP address as the scene master.
	if(g_Au.IsSceneWorker() && g_Au.ClientChannelServer() == g_Au.SceneMasterName())
		return;

	//qDebug("DtClientChannel::RestockNextIds");
	if(!m_AwaitingAtomIdRestock && g_AuDb.AtomIdsNeedRestock())
	{
		quint16 howMany = g_AuDb.AtomIdsRestockQuantity();

		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_RequestAtomIds;
		out << howMany;
		SendPacket(ba);
		m_AwaitingAtomIdRestock = true;

		if(g_Au.m_LogMessages & LOG_AtomCreate)
		{
			QString logmsg;
			logmsg = tr("Restock of %1 atom ids requested.").arg(howMany);
			g_Au.Log(logmsg);
		}
	}

	if(!m_AwaitingBuildingDesignIdRestock && g_AuDb.BuildingDesignIdsNeedRestock())
	{
		quint16 howMany = g_AuDb.BuildingDesignIdsRestockQuantity();

		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_RequestBuildingDesignIds;
		out << howMany;
		SendPacket(ba);
		m_AwaitingBuildingDesignIdRestock = true;

		if(g_Au.m_LogMessages & LOG_SpacecraftDesign)
		{
			QString logmsg;
			logmsg = tr("Restock of %1 building design ids requested.").arg(howMany);
			g_Au.Log(logmsg);
		}
	}

	if(!m_AwaitingItemDesignIdRestock && g_AuDb.ItemDesignIdsNeedRestock())
	{
		quint16 howMany = g_AuDb.ItemDesignIdsRestockQuantity();

		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_RequestItemDesignIds;
		out << howMany;
		SendPacket(ba);
		m_AwaitingItemDesignIdRestock = true;

		if(g_Au.m_LogMessages & LOG_SpacecraftDesign)
		{
			QString logmsg;
			logmsg = tr("Restock of %1 item design ids requested.").arg(howMany);
			g_Au.Log(logmsg);
		}
	}

	if(!m_AwaitingShipDesignIdRestock && g_AuDb.SpaceShipDesignIdsNeedRestock())
	{
		quint16 howMany = g_AuDb.SpaceShipDesignIdsRestockQuantity();

		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_RequestShipDesignIds;
		out << howMany;
		SendPacket(ba);
		m_AwaitingShipDesignIdRestock = true;

		if(g_Au.m_LogMessages & LOG_SpacecraftDesign)
		{
			QString logmsg;
			logmsg = tr("Restock of %1 space ship design ids requested.").arg(howMany);
			g_Au.Log(logmsg);
		}
	}

	if(!m_AwaitingStoryIdRestock && g_AuDb.StoryIdsNeedRestock())
	{
		quint16 howMany = g_AuDb.StoryIdsRestockQuantity();

		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_RequestStoryIds;
		out << howMany;
		SendPacket(ba);
		m_AwaitingStoryIdRestock = true;

		if(g_Au.m_LogMessages & LOG_Story)
		{
			QString logmsg;
			logmsg = tr("Restock of %1 story ids requested.").arg(howMany);
			g_Au.Log(logmsg);
		}
	}
}

// Send a QueryAtomPresence result packet.
void DtClientChannel::SendAtomPresence(const AuId& atom, unsigned int empire, int result)
{
#if !defined(QT_NO_DEBUG)
	if(result && g_Au.IsSceneWorker() && (g_Au.m_LogMessages & LOG_AtomQuery))
	{
		QString res;
		switch(result)
		{
		case 0:
			res = tr("Not here.");
			break;
		case 1:
			res = tr("Found it!");
			break;
		case 2:
			res = tr("Found but can't spawn there.");
			break;
		}
		g_Au.Log(tr("DtClientChannel::SendAtomPresence atom %1 empire %2 result %3").arg(atom.GetID()).arg(empire).arg(res));
	}
#endif
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtServerChannel::P_AtomPresence;
	atom >> out;
	out << (quint32)empire;
	out << (quint8)result;	// 0=no, 1=yes, 2=empire not available
	SendPacket(ba);
}

// Send a QueryHengeTeleporterPresence result packet.
void DtClientChannel::SendHengeTeleportDestPresence(const AuId& teleportId, int result)
{
#if !defined(QT_NO_DEBUG)
	if(result && g_Au.IsSceneWorker() && (g_Au.m_LogMessages & LOG_AtomQuery))
		g_Au.Log(tr("DtClientChannel::SendHengeTeleportDestPresence teleport %1 result %2")
				.arg(teleportId.GetID())
				.arg(result));
#endif
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtServerChannel::P_HengeTeleporterPresence;
	out << teleportId;
	out << (quint8)result;	// 0=no, 1=yes, 2=not ready, 3=not possible
	SendPacket(ba);
}

void DtClientChannel::SendJobOffer(const AuJobOffer& jobOffer)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtServerChannel::P_JobOffer;
	out << jobOffer;
	SendPacket(ba);
}

void DtClientChannel::SendBountyPosted(const AuId& posterId, const QString& posterName, const AuId& victimId, const QString& reason,
		double reward, int why)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtServerChannel::P_BountyPosted;
	out << posterId;
	out << posterName;
	out << victimId;
	out << reason;
	out << reward;
	out << (quint8)why;
	SendPacket(ba);
}

void DtClientChannel::HengeActivated(const AuId& fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
		const AuId& destSysId, quint8 destSun, quint8 destArcSegment)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtServerChannel::P_HengeActivated;
	out << fromSysId;
	out << fromSun;
	out << fromShadowPanels;
	out << fromArcSegment;
	out << destSysId;
	out << destSun;
	out << destArcSegment;
	SendPacket(ba);
}

void DtClientChannel::HengeCreated(const AuId& fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
		const AuId& destSysId, quint8 destSun, quint8 destArcSegment)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtServerChannel::P_HengeCreated;
	out << fromSysId;
	out << fromSun;
	out << fromShadowPanels;
	out << fromArcSegment;
	out << destSysId;
	out << destSun;
	out << destArcSegment;
	SendPacket(ba);
}

void DtClientChannel::HengeDeactivated(const AuId& fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
		const AuId& destSysId, quint8 destSun, quint8 destArcSegment)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtServerChannel::P_HengeDeactivated;
	out << fromSysId;
	out << fromSun;
	out << fromShadowPanels;
	out << fromArcSegment;
	out << destSysId;
	out << destSun;
	out << destArcSegment;
	SendPacket(ba);
}

void DtClientChannel::SendHengeDestinationConfirmed(const AuId& sectorId, const AuId& systemId, const QString& systemName,
		quint8 sun, const QString& worldCatName, const QString& worldName, quint8 worldType, const AuGlobeLLF& hengeLL,
		bool breathable, quint8 orbitZone, bool outsideTown)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtServerChannel::P_HengeDestination;
	out << sectorId;
	out << systemId;
	out << systemName;
	out << sun;
	out << worldCatName;
	out << worldName;
	out << worldType;
	out << hengeLL;
	out << breathable;
	out << orbitZone;
	out << outsideTown;
	SendPacket(ba);
}
