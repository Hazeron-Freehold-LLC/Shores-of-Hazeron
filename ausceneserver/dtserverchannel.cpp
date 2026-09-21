//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtserverchannel.cpp,v 1.348 2022/04/15 00:33:40 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dtserverchannel.h"
//
#include "audbglobal.h"
#include "dadesignplane.h"
#include "dagalaxy.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dauniverse.h"
#include "dgdata.h"
#include "dicontrolinput.h"
#include "dttick.h"
#include <QDataStream>
#include <QVariant>

DtServerChannel::DtServerChannel(unsigned int id, int socketDesc,
		QObject* parent) : QThread(parent), m_semAlive(1)
{
	m_BadPackets = 0;
	m_BytesRx = 0;
	m_BytesTx = 0;
	m_Id = id;
	m_PacketSize = 0;
	m_PacketSizeBytesLeft = 0;
	m_DoleAtomIds = 0;
	m_DoleBuildingDesignIds = 0;
	m_DoleItemDesignIds = 0;
	m_DoleSpaceShipDesignIds = 0;
	m_DoleStoryIds = 0;
	m_socketDescriptor = socketDesc;
	m_semAlive.acquire(1);			// I live...

	m_SubAtomLoad = 0;
	m_SubAtomMgrs = 1;
	m_SubNetPort = 0;
	m_SubServerMode = 0;				// subordinate server mode: 0=Master, 1=Manager, 2=Worker
	m_SubTickTime = 0xffffffff;			// tick time
	m_SubWorkers = 0;		// number of scene worker attached to manager
	m_SubWorkerCores = 0;

	connect(this, SIGNAL(finished()), this, SLOT(ThreadFinished()));
}

void DtServerChannel::ThreadFinished()
{
	//qDebug() << "DtServerChannel::ThreadFinished id" << m_Id;
	// Signal thread completion with my id.
	emit Finished(m_Id);
}

void DtServerChannel::run()
{
	// Creating the socket on the stack automatically associates it with the
	// thread's event loop. This ensures that Qt will not try to deliver events
	// to our socket from the main thread while we are accessing it from
	// DtServerChannel::run().
	QTcpSocket tcpSocket;
	if(!tcpSocket.setSocketDescriptor(m_socketDescriptor))
	{
		emit error(tcpSocket.error());
		return;
	}

	m_tmConnect = AuTime::Now();
	m_peerAddress = tcpSocket.peerAddress();
	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
	{
		QString logmsg = tr("Server Channel Connected %1 %2").arg(m_Id).arg(PeerAddress());
		g_Au.Log(logmsg);
	}

	m_SubStatusLock.lockForWrite();
	m_SubCheckInTime = m_tmConnect;
	m_SubStatusLock.unlock();

	//const int secsToCheckIn = 60 + 60;	// Manager and Worker checks in every minute.
	while(Alive() && tcpSocket.state() == QAbstractSocket::ConnectedState &&
			!tcpSocket.waitForDisconnected(10))
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
				//qDebug("DtServerChannel::run - %d bytes written out of %d", written,
						//m_TxBuffer.size());
				m_TxBuffer.clear();
			}
		}
		m_muTxBuffer.unlock();

		// Pull in any data that is waiting for us.
		// Note: If we wait for the signal, it appears to come from the gui thread,
		// which causes us grief. It's better for us to get the data when it's there.
		RxData(tcpSocket);

		// Dole out ids that have been requested. We have to do it like this because a subordinate might request ids before
		// I have received any from my boss. I'm not sure if we can ever get here on the master before we've loaded them from
		// the database.
		if(m_DoleAtomIds)
			DoleAtomIds();
		if(m_DoleBuildingDesignIds)
			DoleBuildingDesignIds();
		if(m_DoleItemDesignIds)
			DoleItemDesignIds();
		if(m_DoleSpaceShipDesignIds)
			DoleSpaceShipDesignIds();
		if(m_DoleStoryIds)
			DoleStoryIds();
	}

	//qDebug("DtServerChannel::run - Player %d disconnected.", m_idPlayer);
	if(gp_AuGlobal && (g_Au.m_LogMessages & LOG_ConnectDisconnect))
	{
		QString logmsg = tr("Server Channel Disconnected %1 %2").arg(m_Id).arg(PeerAddress());
		g_Au.Log(logmsg);
	}
}

void DtServerChannel::RxData(QTcpSocket& tcpSocket)
{
	//qDebug("DtServerChannel::RxData");
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

void DtServerChannel::ReadPacket(const QByteArray& ba, QTcpSocket& /*tcpSocket*/)
{
	//qDebug("DtServerChannel::ReadPacket - %d bytes rx'd", ba.size());
	QDataStream block(ba);
	block.setVersion(QDataStream::Qt_4_1);

	QString logmsg;

	// The first byte is always the packet type.
	quint8 type;
	block >> type;

	// Tally count of packets by type.
	QHash<quint32, quint32>::iterator pri = m_PacketsRx.find(type);
	if(pri == m_PacketsRx.end())
		m_PacketsRx.insert(type, 1);
	else
		(*pri)++;

	switch((PACKET)type)		// The cast generates a warning when we forget to handle a packet value.
	{
	case P_AvatarAccountActive:
		{
			AuIdList land;
			block >> land;
			emit AvatarAccountActiveSignal(land);
		}
		break;

	case P_RedeemShipTitle:
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

	case P_SectorPirateBase:
		{
			AuId encSectorId, encSystemId;
			block >> encSectorId;
			block >> encSystemId;
			emit SectorPirateBaseEncounter(encSectorId, encSystemId);
		}
		break;
	case P_BlueprintRatedBd:
		{
			AuExchangeRatingRec rating;
			block >> rating;
			emit BlueprintRatedBd(rating);
		}
		break;

	case P_BlueprintRatedSc:
		{
			AuExchangeRatingRec rating;
			block >> rating;
			emit BlueprintRatedSc(rating);
		}
		break;

	case P_CollectBounty:
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

	case P_PayBounty:
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

	case P_SpacecraftPosition:
		{
			AuId shipId, systemId;
			AuStarMapSpacecraft pos;

			block >> shipId;
			block >> systemId;
			block >> pos;

			emit SpacecraftPositionReport(shipId, systemId, pos);
		}
		break;

	case P_EmpireRecordPatent:
		{
			quint32 empireId;
			quint16 patentCID;
			quint8 patentQ;

			block >> empireId;
			block >> patentCID;
			block >> patentQ;

			emit EmpireRecordPatent(empireId, patentCID, patentQ);
		}
		break;

	case P_Trade:
		{
			quint8 doWhat;
			block >> doWhat;
			if(doWhat == 0)
			{
				// Deliver shipment of a TEU container.
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

	case P_CityFacilities:
		{
			AuId cityId;
			AuCityIndustryHash facil;

			block >> cityId;
			block >> facil;

			emit CityFacilitiesChanged(cityId, facil);
		}
		break;

	case P_CityStatus:
		{
			AuId cityId;
			quint8 alertCondition;
			qint8 morale;

			block >> cityId;
			block >> alertCondition;
			block >> morale;

			emit CityStatusChanged(cityId, alertCondition, morale);
		}
		break;

	case P_BdDesignExchangePublished:
		{
			AuBdDesignExchangeEntry item;

			block >> item;

			emit BdDesignExchangePublishedSignal(item);
		}
		break;

	case P_BdDesignExchangeUnpublished:
		{
			quint32 designId;

			block >> designId;

			emit BdDesignExchangeUnpublishedSignal(designId);
		}
		break;

	case P_BdDesignDeactivated:
		{
			quint32 designId;

			block >> designId;

			emit BdDesignDeactivatedSignal(designId);
		}
		break;

	case P_ItDesignDeactivated:
		{
			quint32 designId;

			block >> designId;

			emit ItDesignDeactivatedSignal(designId);
		}
		break;

	case P_ScDesignExchangePublished:
		{
			AuScDesignExchangeEntry item;

			block >> item;

			emit ScDesignExchangePublishedSignal(item);
		}
		break;

	case P_ScDesignExchangeUnpublished:
		{
			quint32 designId;

			block >> designId;

			emit ScDesignExchangeUnpublishedSignal(designId);
		}
		break;

	case P_ScDesignDeactivated:
		{
			quint32 designId;

			block >> designId;

			emit ScDesignDeactivatedSignal(designId);
		}
		break;

	case P_ShipStasisRecover:
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

	case P_HengeDestination:
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

	case P_HengeActivated:
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

			emit HengeActivated(fromSysId, fromSun, fromShadowPanels, fromArcSegment, destSysId, destSun, destArcSegment);
		}
		break;

	case P_HengeCreated:
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

			emit HengeCreated(fromSysId, fromSun, fromShadowPanels, fromArcSegment, destSysId, destSun, destArcSegment);
		}
		break;

	case P_HengeDeactivated:
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

			emit HengeDeactivated(fromSysId, fromSun, fromShadowPanels, fromArcSegment, destSysId, destSun, destArcSegment);
		}
		break;

	case P_HengeTeleporterPresence:
		{
			// We have received a response to QueryHengeTeleporterPresence.
			// This message is on the way up the server chain since it arrived on the server channel.
			AuId teleportId;
			qint8 result;

			block >> teleportId;
			block >> result;	// 0=no, 1=yes, 2=not ready, 3=not possible

			// Remove the query from my queue.
			m_AtomQueryMutex.lock();
			QVector<AuHengeTeleportDestQuery>::iterator it = m_HengeTeleporterQueries.begin();
			while(it != m_HengeTeleporterQueries.end())
			{
				if((*it).m_TeleportId == teleportId)
				{
					it = m_HengeTeleporterQueries.erase(it);
					break;
				}
				else
					it++;
			}
			m_AtomQueryMutex.unlock();

			emit HengeTeleporterPresence(teleportId, result, m_Id);
		}
		break;


	case P_SpacecraftNameChanged:
		{
			AuId shipId;
			QString nuName;

			block >> shipId;
			block >> nuName;

			emit SpacecraftNameChanged(shipId, nuName);
		}
		break;

	case P_ArenaReady:
		{
			AuArenaReady arena;

			block >> arena;

			emit ArenaReady(arena);
		}
		break;

	case P_StargateClosed:
		{
			AuId fromGalaxyId;
			AuPoint fromGalaxyLoc;

			block >> fromGalaxyId;
			block >> fromGalaxyLoc;

			emit StargateClosed(fromGalaxyId, fromGalaxyLoc);
		}
		break;

	case P_StargateOpened:
		{
			AuId fromGalaxyId, toGalaxyId;
			AuPoint fromGalaxyLoc, toGalaxyLoc;
			AuTime expire;

			block >> fromGalaxyId;
			block >> fromGalaxyLoc;
			block >> toGalaxyId;
			block >> toGalaxyLoc;
			expire << block;

			emit StargateOpened(fromGalaxyId, fromGalaxyLoc, toGalaxyId, toGalaxyLoc, expire);
		}
		break;

	case P_CityMailRequested:
		{
			AuId cityId, unitId;
			quint32 unitEmpire;

			block >> cityId;
			block >> unitId;
			block >> unitEmpire;

			emit CityMailRequestedSignal(cityId, unitId, unitEmpire);
		}
		break;

	case P_WarpSignatureDetected:
		{
			AuId systemId;
			quint32 signalStrength;
			quint32 detectingEmpireId;

			block >> systemId;
			block >> signalStrength;
			block >> detectingEmpireId;

			emit WarpSignatureDetected(systemId, signalStrength, detectingEmpireId);
		}
		break;

	case P_SystemWarpSignature:
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

			emit SystemWarpSignature(systemId, warpFactors, galaxy, galaxyLocX, galaxyLocY, galaxyLocZ);
		}
		break;

	case P_SectorVisited:
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

	case P_AvatarOnLine:
		{
			AuId personId;
			quint32 empireId;

			block >> personId;
			block >> empireId;

			emit AvatarOnLine(personId, empireId);
		}
		break;

	case P_RenameAvatar:
		{
			AuId avatarId;
			QString nuName;

			block >> avatarId;
			block >> nuName;

			emit RenameAvatar(avatarId, nuName);
		}
		break;

	case P_RenameCity:
		{
			AuId cityId;
			QString nuName;

			block >> cityId;
			block >> nuName;

			emit RenameCity(cityId, nuName);
		}
		break;

	case P_RenamePlanet:
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

	case P_RenameSector:
		{
			AuId sectorId;
			QString nuName;

			block >> sectorId;
			block >> nuName;

			emit RenameSector(sectorId, nuName);
		}
		break;

	case P_RenameSystem:
		{
			AuId systemId;
			QString nuName;

			block >> systemId;
			block >> nuName;

			emit RenameSystem(systemId, nuName);
		}
		break;

	case P_BountyPosted:
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

	case P_JobOffer:
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

	case P_AtomSpawning:
		{
			AuId personOrShip, spawnCity, spawnShip, arena;
			quint32 empireId;
			quint8 galaxy;	// GALAXY enum value

			block >> personOrShip;
			block >> empireId;
			block >> spawnCity;
			block >> spawnShip;
			block >> arena;
			block >> galaxy;

			//qDebug("DtServerChannel::ReadPacket P_AtomSpawning personOrShip %d empire %d spawnCity %d spawnShip %d",
					//personOrShip.GetID(), empireId, spawnCity.GetID(), spawnShip.GetID());
			emit AtomSpawning(personOrShip, empireId, spawnCity, spawnShip, arena, galaxy);
		}
		break;

	case P_HengeTeleportAtom:
		{
			AuHengeTeleportAtom hengeTel;

			block >> hengeTel;

			emit AtomHengeTeleporting(hengeTel);
		}
		break;

	case P_AtomTravelingGalaxyLoc:
		{
			AuId traveler, destGalaxy;
			AuPoint destGalaxyLoc;

			block >> traveler;
			block >> destGalaxy;
			block >> destGalaxyLoc;

			emit AtomTravelingGalaxyLoc(traveler, destGalaxy, destGalaxyLoc);
		}
		break;

	case P_AtomTravelingStargate:
		{
			AuId traveler, destGalaxy;
			AuPoint destGalaxyLoc;

			block >> traveler;
			block >> destGalaxy;
			block >> destGalaxyLoc;

			emit AtomTravelingStargate(traveler, destGalaxy, destGalaxyLoc);
		}
		break;

	case P_AtomTraveling:
		{
			AuId traveler, destSystem, destSector;
			traveler << block;
			destSystem << block;
			destSector << block;
			//qDebug("DtServerChannel::ReadPacket P_AtomTraveling traveler %d destSystem %d destSector %d",
					//traveler.GetID(), destSystem.GetID(), destSector.GetID());
			emit AtomTraveling(traveler, destSystem, destSector);
		}
		break;

	case P_SystemDecayed:
		{
			AuId systemId;
			block >> systemId;

			emit SolarSystemReadyToDecay(systemId);
		}
		break;

	case P_HandoffElsewhere:
		{
			AuId person, dest, returnOnFail;
			QByteArray traveller;
			quint8 rideAlso;
			bool limboRescue;

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
	case P_SpawnPersonElsewhere:
		{
			AuId person, spawnCity, spawnShip;
			quint32 empire;
			QByteArray traveller;
			person << block;
			block >> empire;
			spawnCity << block;
			spawnShip << block;
			block >> traveller;
			emit SpawnPersonElsewhere(person, empire, spawnCity, spawnShip, traveller);
		}
		break;
	case P_SpawnShipElsewhere:
		{
			AuId ship, spawnCity;
			quint32 empire;
			QByteArray traveller;
			ship << block;
			block >> empire;
			spawnCity << block;
			block >> traveller;
			emit SpawnShipElsewhere(ship, empire, spawnCity, traveller);
		}
		break;
	case P_TransferElsewhere:
		{
			AuId id, destSystem, destSector;
			QByteArray traveller;

			id << block;
			destSystem << block;
			destSector << block;
			block >> traveller;

			//qDebug("DtServerChannel::ReadPacket P_TransferElsewhere id %d system %d sector %d data %d bytes",
					//id.GetID(), destSystem.GetID(), destSector.GetID(), traveller.size());
			emit TravelElsewhere(id, destSystem, destSector, traveller);
		}
		break;
		*/

	case P_SummonDormantShip:
		{
			AuId shipId, crewId;
			block >> shipId;
			block >> crewId;
			emit SummonDormantShip(shipId, crewId);	// Propagate up when received on this server channel.
		}
		break;

	case P_AdminSummon:
		{
			AuId person, adminId;
			quint8 rideAlso;
			block >> person;
			block >> adminId;
			block >> rideAlso;
			emit AdminSummon(person, adminId, rideAlso);	// Propagate up when received on this server channel.
		}
		break;

	case P_OfficerKilled:
		{
			AuId homeWorldId;
			homeWorldId << block;
			emit OfficerKilled(homeWorldId);
		}
		break;
	case P_SendFunds:
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
	case P_NameSpecies:
		{
			AuSpecies species;
			QString specieName;
			AuId discoverer;

			block >> species;
			block >> specieName;
			block >> discoverer;

			emit NameSpecies(species, specieName, discoverer);
		}
		break;

	case P_EmpireChg:
		{
			quint8 chgWhat;
			block >> chgWhat;
			switch(chgWhat)
			{
			case 0:
				{
					quint32 empireId;
					quint16 industry;
					quint32 designId;
					block >> empireId;
					block >> industry;
					block >> designId;
					emit EmpireChgBdPref(empireId, industry, designId);
				}
				break;
			case 1:
				{
					quint32 empireId;
					quint8 scrole;
					quint32 designId;
					block >> empireId;
					block >> scrole;
					block >> designId;
					emit EmpireChgScPref(empireId, scrole, designId);
				}
				break;
			}
		}
		break;

	case P_EmpireChgFlag:
		{
			quint32 empireId;
			QImage flag;
			block >> empireId;
			block >> flag;
			emit EmpireChgFlag(empireId, flag);
		}
		break;

	case P_EmpireChgName:
		{
			quint32 empireId;
			QString name;

			block >> empireId;
			block >> name;

			emit EmpireChgName(empireId, name);
		}
		break;

	case P_EmpireChgRole:
		{
			quint32 empireId;
			quint8 gameRole;

			block >> empireId;
			block >> gameRole;

			emit EmpireChgRole(empireId, gameRole);
		}
		break;

	case P_AtomPresence:
		{
			// We have received a response to QueryAtomPresence.
			// This message is on the way up the server chain since it arrived
			// on the server channel.
			AuId atom;
			atom << block;
			quint32 empire;
			block >> empire;
			qint8 result;
			block >> result;	// 0=no, 1=yes, 2=empire not friendly

			// Remove the query from my queue.
			m_AtomQueryMutex.lock();
			//QString logmsg;
			//logmsg.sprintf("DtServerChannel::ReadPacket %d AtomQuery "
					//"mutex locked", m_Id);
			//g_Au.Log(logmsg);
			QVector<AuAtomQuery>::iterator it = m_AtomQueries.begin();
			while(it != m_AtomQueries.end())
			{
				if((*it).m_Atom == atom && (*it).m_Empire == empire)
				{
					it = m_AtomQueries.erase(it);
					break;
				}
				else
					it++;
			}
			m_AtomQueryMutex.unlock();

			// Emit a signal.
			//qDebug("DtServerChannel::ReadPacket %d peer %s P_AtomPresence emit "
					//"AtomPresence", m_Id,
					//(const char*)PeerAddress().toAscii());
			emit AtomPresence(atom, empire, result, m_Id);
			//qDebug("DtServerChannel::ReadPacket %d peer %s P_AtomPresence done",
					//m_Id, (const char*)PeerAddress().toAscii());
		}
		break;

	case P_Chat:			// Sent by sub to boss to propagate a chat message.
		{
			quint8 msgType;
			block >> msgType;
			AuCommPacket* msg = AuCommPacket::CreateCommPacket((AuCommPacket::TYPE)msgType);
			block >> *msg;

			// This message is on the way up the server chain since it arrived on the server channel.
			auSceneServerApp->ChatUp(*msg, this);
			delete msg;
		}
		break;

	case P_OpenChannelTo:	// Sent by sub to boss to propagate a channel-open request.
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

			// This message is on the way up the server chain since it arrived on the server channel.
			auSceneServerApp->OpenChannelUp(fromUnitId, fromName, fromSector, (GALAXY)fromGalaxy, toUnitId, channelName,
					(AuCommStdChannel::SCOPE)scope, this);
		}
		break;

	case P_ReleaseBerth:
		{
			// My subordinate server is propagating a berth release up to the
			// master server.
			AuId personId, shipId;
			personId << block;		// Person releasing their berth.
			shipId << block;		// The ship they are moving to, may be 0.
			emit ReleaseBerth(personId, shipId);
		}
		break;

	case P_GovDossierAddDocument:
		{
			quint32 empireId;
			quint32 dosEmpire;
			QByteArray packedDoc;

			block >> empireId;
			block >> dosEmpire;
			block >> packedDoc;

			emit GovDossierAddDocument(empireId, dosEmpire, packedDoc);
		}
		break;

	case P_GovBossChanged:
		{
			quint32 empireId;
			AuId governorId, bossId;

			block >> empireId;
			block >> governorId;
			block >> bossId;

			emit GovBossChanged(empireId, governorId, bossId);
		}
		break;

	case P_OfficeAssigned:
		{
			AuId person;
			quint32 empire;
			AuOffice office;

			block >> person;
			block >> empire;
			block >> office;

			//qDebug("DtServerChannel::ReadPacket, emits OfficeAssigned");
			emit OfficeAssigned(person, empire, office);
		}
		break;

	case P_OfficeRevoked:
		{
			AuId person;
			quint32 empire;

			block >> person;
			block >> empire;

			emit OfficeRevoked(person, empire); 
		}
		break;

	case P_RequestAtomIds:	// Sent by sub to boss to get more atom ids.
		block >> m_DoleAtomIds;
		break;

	case P_RequestBuildingDesignIds:	// Sent by sub to boss to get more design ids.
		block >> m_DoleBuildingDesignIds;
		break;

	case P_RequestItemDesignIds:	// Sent by sub to boss to get more design ids.
		block >> m_DoleItemDesignIds;
		break;

	case P_RequestShipDesignIds:	// Sent by sub to boss to get more design ids.
		block >> m_DoleSpaceShipDesignIds;
		break;

	case P_RequestStoryIds:	// Sent by sub to boss to get more atom ids.
		block >> m_DoleStoryIds;
		break;

	case P_SystemFinal:
		if(g_Au.IsSceneMaster())
		{
			AuId systemId;

			block >> systemId;

			emit SolarSystemFinal(systemId);
		}
		else
		{
			if(g_Au.m_LogMessages & LOG_Warning)
				g_Au.Warning(tr("%1 P_SystemFinal Unexpected TCP packet type %2 size %3.")
						.arg(m_Id).arg(type).arg(ba.size()));
			m_BadPackets++;
		}
		break;

	case P_Status:			// Sent by subordinate to boss.
		{
			quint8 serverMode;
			block >> serverMode;

			m_SubStatusLock.lockForWrite();
			m_SubServerMode = serverMode;
			block >> m_SubAtomLoad;
			block >> m_SubTickTime;
			block >> m_SubWorkers;	// number of workers.
			block >> m_SubWorkerCores;
			m_SubPlayers << block;	// players
			block >> m_SubNetName;
			block >> m_SubNetPort;
			block >> m_SubAtomMgrs;
			quint16 sysClients;
			block >> sysClients;
			m_SubSystemClients.clear();
			for(;sysClients;sysClients--)
			{
				AuId idSystem;
				block >> idSystem;
				AuSystemStatus sstat;
				block >> sstat.m_Clients;
				block >> sstat.m_Tick;
				m_SubSystemClients.insert(pair<AuId, AuSystemStatus>(idSystem, sstat));
			}
			m_SubCheckInTime = AuTime::Now();
			m_SubStatusLock.unlock();

			if(g_Au.IsSceneMaster() && serverMode == 2)
			{
				// SceneMaster defers SceneWorkers to a SceneManager.
				DeferSceneWorker();
			}
			else if(g_Au.IsSceneMaster() && serverMode == 1)
			{
				// SceneMaster keeps connection with live SceneManagers.
				if(g_Au.m_LogMessages & LOG_Status)
				{
					m_SubStatusLock.lockForRead();
					if(m_SubTickTime == 0xffffffff)
						logmsg = tr("%1 %2 %3:%4. %5 players. %6 workers. %7 atom managers. Not ready for assignments.")
							.arg(serverMode == 0 ? "Master" : (serverMode == 1 ? "Manager" : "Worker"))
							.arg(m_Id)
							.arg(m_SubNetName)
							.arg(m_SubNetPort)
							.arg(m_SubPlayers.size())
							.arg(m_SubWorkers)
							.arg(m_SubWorkerCores);
					else
						logmsg = tr("%1 %2 %3:%4. %5 players. %6 workers. %7 atom managers. Load %8")
							.arg(serverMode == 0 ? "Master" : (serverMode == 1 ? "Manager" : "Worker"))
							.arg(m_Id)
							.arg(m_SubNetName)
							.arg(m_SubNetPort)
							.arg(m_SubPlayers.size())
							.arg(m_SubWorkers)
							.arg(m_SubWorkerCores)
							.arg(m_SubAtomLoad, 0,'f',2);
					m_SubStatusLock.unlock();
					g_Au.Log(logmsg);
				}
			}
			else if((g_Au.IsSceneManager() && serverMode == 2))
			{
				// SceneManager keeps connection with live SceneWorkers.
				if(g_Au.m_LogMessages & LOG_Status)
				{
					m_SubStatusLock.lockForRead();
					if(m_SubTickTime == 0xffffffff)
					{
						logmsg = tr("%1 %2 %3:%4. %5 players. %6 systems. %7 atom managers. Not ready for assignments.")
							.arg(serverMode == 0 ? "Master" : (serverMode == 1 ? "Manager" : "Worker"))
							.arg(m_Id)
							.arg(m_SubNetName)
							.arg(m_SubNetPort)
							.arg(m_SubPlayers.size())
							.arg(m_SubSystemClients.size())
							.arg(m_SubWorkerCores);
						g_Au.Log(logmsg);
					}
					else
					{
						logmsg = tr("%1 %2 %3:%4. %5 players. %6 systems. %7 atom managers. Load %8")
							.arg(serverMode == 0 ? "Master" : (serverMode == 1 ? "Manager" : "Worker"))
							.arg(m_Id)
							.arg(m_SubNetName)
							.arg(m_SubNetPort)
							.arg(m_SubPlayers.size())
							.arg(m_SubSystemClients.size())
							.arg(m_SubWorkerCores)
							.arg(m_SubAtomLoad, 0,'f',2);
						g_Au.Log(logmsg);
					}
					m_SubStatusLock.unlock();
				}
			}
			else
			{
				if(g_Au.m_LogMessages & LOG_Warning)
				{
					QString logmsg = tr("%1 Received TCP packet type P_Status size %2. Scene %3 should never send this to me.")
						.arg(m_Id)
						.arg(ba.size())
						.arg(serverMode == 0 ? "Master" : (serverMode == 1 ? "Manager" : "Worker"));
					g_Au.Warning(logmsg);
				}
				m_BadPackets++;
			}
		}
		break;

	case P_StoryChanged:
		{
			quint32 storyInstance;
			block >> storyInstance;
			if(g_Au.IsSceneMaster())
				emit ReloadStory(storyInstance);	// propagate down
			else
				emit StoryChanged(storyInstance);	// propagate up
		}
		break;

	case P_StoryDeleted:
		{
			quint32 storyInstance;
			block >> storyInstance;
			if(g_Au.IsSceneMaster())
				emit RemoveStory(storyInstance);	// propagate down
			else
				emit StoryDeleted(storyInstance);	// propagate up
		}
		break;

	case P_WormholeExplored:
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
			if(g_Au.IsSceneMaster())
			{
				// The scene master notifies all scene managers of the change.
				emit RenameWormhole(systemId, toSystem, polarity, empire, nuName);
			}
			else if(g_Au.IsSceneManager())
			{
				// Scene manager propagates message up to scene master.
				emit WormholeExploredSignal(systemId, toSystem, polarity, empire, nuName);
			}
			else
			{
				if(g_Au.m_LogMessages & LOG_Warning)
					g_Au.Warning(tr("%1 P_WormholeExplored Unexpected TCP packet type %2 size %3.")
							.arg(m_Id).arg(type).arg(ba.size()));
				m_BadPackets++;
			}
		}
		break;

	case P_GovCitySurrendered:
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

	case P_GovSetCapital:
		{
			quint32 empire;
			AuId city, domain;
			AuSpecies species;
			quint8 capitalType;
			block >> empire;
			block >> city;
			block >> species;
			block >> domain;
			block >> capitalType;
			if(g_Au.IsSceneMaster())
			{
				// Master changes the empire and notifies the subs.
				//qDebug("P_GovSetCapital master sets empire %d capitol %d", empire, city.GetID());
				DgData& gd = g_Au.LockDataForWrite();
				switch(capitalType)
				{
				case 0:	// sector
					if(!domain.IsNull())
						gd.SetEmpireSectorCapital(empire, city, domain);
					break;
				case 1:	// empire
					gd.SetEmpireCapital(empire, city, species);
					break;
				case 3:	// galaxy
					if(!domain.IsNull())
						gd.SetEmpireGalaxyCapital(empire, city, domain);
					break;
				}
				g_Au.UnlockData();
				emit ChangeEmpireInDb(empire);	// Trigger db store which notifies the subordinates when done.
			}
			else
			{
				// Manager propagates the message up to the master.
				//qDebug("P_GovSetCapital manager propagates empire %d city %d",
						//empire, city.GetID());
				emit GovSetCapital(empire, city, species, domain, capitalType);
			}
		}
		break;

	case P_GovSetJoinPass:
		{
			quint32 empire;
			QByteArray passCrypt;
			block >> empire;
			block >> passCrypt;
			if(g_Au.IsSceneMaster())
			{
				// Master changes the empire and notifies the subs.
				DgData& gd = g_Au.LockDataForWrite();
				gd.EmpireSetJoinPassCrypt(empire, passCrypt);
				g_Au.UnlockData();
				emit ChangeEmpireInDb(empire);	// Trigger db store which notifies the subordinates when done.
			}
			else
			{
				// Manager propagates the message up to the master.
				emit GovSetJoinPassword(empire, passCrypt);
			}
		}
		break;

	case P_GovSetCommerce:
		{
			quint32 empire;
			QByteArray commerce;
			block >> empire;
			block >> commerce;
			if(g_Au.IsSceneMaster())
			{
				// Master changes the empire and notifies the subs.
				//qDebug("P_GovSetCommerce - SetEmpireCommerce %d items",
						//commerce.size());
				DgData& gd = g_Au.LockDataForWrite();
				gd.SetEmpireCommerce(empire, commerce);
				g_Au.UnlockData();
				emit ChangeEmpireInDb(empire);	// Trigger db store which notifies
											// the subordinates when done.
			}
			else
			{
				// Manager propagates the message up to the master.
				//qDebug("P_GovSetCommerce - emit GovSetCommerce %d items",
						//commerce.size());
				auSceneServerApp->GovSetCommerceUp(empire, commerce);
				//emit GovSetCommerce(empire, commerce);
			}
		}
		break;

	case P_GovSetDefStance:
		{
			quint32 empire;
			quint8 stance;
			block >> empire;
			block >> stance;
			if(g_Au.IsSceneMaster())
			{
				// Master changes the empire and notifies the subs.
				//qDebug("P_GovSetDefStance master sets empire %d capitol %d",
						//empire, city.GetID());
				if(stance != POL_Empire)
				{
					DgData& gd = g_Au.LockDataForWrite();
					gd.SetEmpireDefaultPoliticalStance(empire, (POLITICAL_STANCE)stance);
					g_Au.UnlockData();
					emit ChangeEmpireInDb(empire);	// Trigger db store which notifies the subordinates when done.
				}
			}
			else
			{
				// Manager propagates the message up to the master.
				emit GovSetDefaultPoliticalStance(empire, stance);
			}
		}
		break;

	case P_GovSetContrailColor:
		{
			quint32 empire;
			quint8 hue, sat;
			block >> empire;
			block >> hue;
			block >> sat;
			if(g_Au.IsSceneMaster())
			{
				// Master changes the empire and notifies the subs.
				DgData& gd = g_Au.LockDataForWrite();
				gd.SetEmpireContrailColor(empire, hue, sat);
				g_Au.UnlockData();
				emit ChangeEmpireInDb(empire);	// Trigger db store which notifies the subordinates when done.
			}
			else
			{
				// Manager propagates the message up to the master.
				emit GovSetContrailColor(empire, hue, sat);
			}
		}
		break;

	case P_GovDossierChangeState:
		{
			quint32 empire;
			quint32 dosEmpire;
			AuTime docTime;
			quint8 docType;
			quint16 docIndex;
			quint8 docState;

			block >> empire;
			block >> dosEmpire;
			block >> docTime;
			block >> docType;
			block >> docIndex;
			block >> docState;

			if(g_Au.IsSceneMaster())
			{
				// Master changes the empire and notifies the subs.
				DgData& gd = g_Au.LockDataForWrite();
				gd.SetEmpireDossierChangeState(empire, dosEmpire, docTime, docType, docIndex, (AuDocument::DOC_STATE)docState);
				g_Au.UnlockData();
				emit ChangeEmpireInDb(empire);	// Trigger db store which notifies the subordinates when done.
			}
			else
			{
				// Manager propagates the message up to the master.
				emit GovDossierChangeState(empire, dosEmpire, docTime, docType, docIndex, docState);
			}
		}
		break;

	case P_GovSetAnnouncements:
		{
			quint32 empire;
			QString announcements;

			block >> empire;
			block >> announcements;

			if(g_Au.IsSceneMaster())
			{
				// Master changes the empire and notifies the subs.
				DgData& gd = g_Au.LockDataForWrite();
				gd.SetEmpireAnnouncements(empire, announcements);
				g_Au.UnlockData();
				emit ChangeEmpireInDb(empire);	// Trigger db store which notifies the subordinates when done.
			}
			else
			{
				// Manager propagates the message up to the master.
				emit GovSetAnnouncements(empire, announcements);
			}
		}
		break;

	case P_GovSetThemeColor:
		{
			quint32 empire;
			quint8 theme;
			QColor color;

			block >> empire;
			block >> theme;
			block >> color;

			if(g_Au.IsSceneMaster())
			{
				// Master changes the empire and notifies the subs.
				DgData& gd = g_Au.LockDataForWrite();
				gd.SetEmpireThemeColor(empire, (AuScFace::COLOR_THEME)theme, color);
				g_Au.UnlockData();
				emit ChangeEmpireInDb(empire);	// Trigger db store which notifies the subordinates when done.
			}
			else
			{
				// Manager propagates the message up to the master.
				emit GovSetThemeColor(empire, theme, color);
			}
		}
		break;

	case P_GovSetWarCriteria:
		{
			quint32 empire;
			AuWarCriteria crit;

			block >> empire;
			block >> crit;

			emit GovSetWarCriteria(empire, crit);
		}
		break;

	case P_GovSetTax:
		{
			quint32 empire;
			quint8 income, sales, subsidy;
			block >> empire;
			block >> income;
			block >> sales;
			block >> subsidy;
			if(g_Au.IsSceneMaster())
			{
				// Master changes the empire and notifies the subs.
				DgData& gd = g_Au.LockDataForWrite();
				gd.SetEmpireTaxes(empire, income, sales, subsidy);
				g_Au.UnlockData();
				emit ChangeEmpireInDb(empire);	// Trigger db store which notifies
											// the subordinates when done.
			}
			else
			{
				// Manager propagates the message up to the master.
				emit GovSetTax(empire, income, sales, subsidy);
			}
		}
		break;

	case P_CityDecayed:
		{
			quint32 empireId;
			AuId cityId;
			block >> empireId;
			block >> cityId;
			if(g_Au.IsSceneMaster())
			{
				// The scene master notifies all scene managers of the change.
				// Master changes the empire and notifies the subs.
				DgData& gd = g_Au.LockDataForWrite();
				gd.EmpireCityDecayed(empireId, cityId);
				g_Au.UnlockData();
				if(empireId > 0)
				{
					emit ChangeEmpireInDb(empireId);	// Trigger db store for empire
				}
				//else
					//qDebug("DtServerChannel::P_CityDecayed empireId=0");
			}
			else if(g_Au.IsSceneManager())
			{
				// Scene manager propagates message up to scene master.
				emit CityDecayed(empireId, cityId);
			}
			else
			{
				if(g_Au.m_LogMessages & LOG_Warning)
					g_Au.Warning(tr("%1 P_CityDecayed Unexpected TCP packet type %2 size %3.")
							.arg(m_Id).arg(type).arg(ba.size()));
				m_BadPackets++;
			}
		}
		break;

	case P_CityCaptured:
		{
			quint32 oldEmpireId, nuEmpireId;
			AuId cityId;
			QString sCityName;
			block >> oldEmpireId;
			block >> nuEmpireId;
			block >> cityId;
			if(g_Au.IsSceneMaster())
			{
				//qDebug("DtServerChannel::ReadPacket Master is calling EmpireCityCaptured OldEmpire=%d, NewEmpire=%d, CityId=%d, CityName=%s",oldEmpireId,nuEmpireId,cityId.GetID());
				// The scene master notifies all scene managers of the change.
				// Master changes the empire and notifies the subs.
				DgData& gd = g_Au.LockDataForWrite();
				gd.EmpireCityCaptured(oldEmpireId, nuEmpireId, cityId);
				g_Au.UnlockData();
				if(oldEmpireId)
				{
					emit ChangeEmpireInDb(oldEmpireId);	// Trigger db store for old empire
				}
				//else
					//qDebug("DtServerChannel::P_CityCaptured OldEmpireChanged=0");
				if(nuEmpireId)
				{
					emit ChangeEmpireInDb(nuEmpireId);	// Trigger db store for nu empire
				}
				//else
					//qDebug("DtServerChannel::P_CityCaptured NewEmpireChanged=0");
			}
			else if(g_Au.IsSceneManager())
			{
				//qDebug("DtServerChannel::ReadPacket, emits CityCaptured");
				// Scene manager propagates message up to scene master.
				emit CityCaptured(oldEmpireId, nuEmpireId, cityId);
			}
			else
			{
				if(g_Au.m_LogMessages & LOG_Warning)
					g_Au.Warning(tr("%1 P_CityCaptured Unexpected TCP packet type %2 size %3.")
							.arg(m_Id).arg(type).arg(ba.size()));
				m_BadPackets++;
			}
		}
		break;

	case P_EmpireCityWorkerDNAChanged:
		{
			quint32 empireId;
			AuId cityId;
			AuSpecies workerSpecies;

			block >> empireId;
			block >> cityId;
			block >> workerSpecies;

			emit EmpireCityWorkerSpeciesChanged(empireId, cityId, workerSpecies);
		}
		break;

	case P_CityCreated:
		{
			quint32 empireId;
			quint8 cityType;
			AuId cityId, founderId, planetId, systemId, sectorId, galaxyId;
			QString cityName, founderName, planetName, systemName, sectorName;
			AuPoint systemLoc;
			quint8 galaxy;

			block >> empireId;
			block >> cityType;
			block >> cityId;
			block >> cityName;
			block >> founderId;
			block >> founderName;
			block >> planetId;
			block >> planetName;
			block >> systemId;
			block >> systemName;
			block >> systemLoc;
			block >> sectorId;
			block >> sectorName;
			block >> galaxyId;
			block >> galaxy;

			emit CityCreated(empireId,
					cityType, cityId, cityName,
					founderId, founderName,
					planetId, planetName,
					systemId, systemName, systemLoc,
					sectorId, sectorName,
					galaxyId, galaxy);
		}
		break;

	case P_SystemExplored:
		{
			AuId systemId;
			quint32 empireId;

			block >> systemId;
			block >> empireId;

			// This is a manager or master server receiving the SystemExplored message.
			//qDebug("DtServerChannel::ReadPacket SystemExplored %d empire %d", systemId.GetID(), empireId);
			emit SystemExplored(systemId, empireId);
		}
		break;

	case P_SystemScanned:
		{
			AuId systemId;
			quint32 empireId;
			quint8 genHab;
			quint32 genSeed;

			block >> systemId;
			block >> empireId;
			block >> genHab;
			block >> genSeed;

			// This is a manager or master server receiving the SystemScanned message.
			//qDebug("DtServerChannel::ReadPacket SystemScanned %d empire %d", systemId.GetID(), empireId);
			emit SystemScanned(systemId, empireId, genHab, genSeed);
		}
		break;

	case P_SystemNameChanged:
		{
			AuId systemId, sectorId;
			QString systemName, sectorName;
			systemId << block;
			block >> systemName;
			sectorId << block;
			block >> sectorName;

			// This is the master server receiving the SystemNameChanged message. From here it will propagate
			// back down to every server in the farm so they can update their local data.
			emit SystemNameChanged(systemId, systemName, sectorId, sectorName);
		}
		break;

	case P_SetPoliticalStance:
		{
			quint32 idToEmpire, idFromEmpire;
			quint8 stance, vassalTributePercent, toStance, reason, galaxy;
			AuId fromUnit, toUnit;
			QString fromUnitName, toUnitName, sector, system, world;

			block >> idToEmpire;	// Empire toward whom the stance is set.
			block >> idFromEmpire;	// Empire whose stance is being set.
			block >> stance;		// Stance idFromEmpire is taking toward idToEmpire.
			block >> vassalTributePercent;
			block >> fromUnit;
			block >> fromUnitName;
			block >> toStance;
			block >> toUnit;
			block >> toUnitName;
			block >> reason;
			block >> galaxy;
			block >> sector;
			block >> system;
			block >> world;

			emit SetPoliticalStance(idToEmpire, idFromEmpire, stance, vassalTributePercent,
					fromUnit, fromUnitName, toStance, toUnit, toUnitName, reason, galaxy, sector, system, world);
		}
		break;

	case P_CreditCityBank:
		{
			float fAmount;
			AuId cityId;
			block >> fAmount;
			cityId << block;

			emit CreditCityBank(fAmount, cityId);
		}
		break;

	case P_AddCitizen:
		{
			quint32 oldEmpireId, nuEmpireId;
			AuId personId;
			QString personName;
			quint8 personGender;

			block >> oldEmpireId;
			block >> nuEmpireId;
			block >> personId;
			block >> personName;
			block >> personGender;

			emit AddCitizenToEmpire(oldEmpireId, nuEmpireId, personId, personName, personGender);
		}
		break;

	case P_LeaveEmpire:
		{
			quint32 idOldEmpire;
			AuId personId;
			QString personName;

			block >> personId;
			block >> personName;
			block >> idOldEmpire;

			emit LeaveEmpire(personId, personName, idOldEmpire);
		}
		break;

	case P_EstNewEmpire:
		{
			QString personName, empireName;
			AuId personId;
			quint32 idOldEmpire;
			quint8 personGender;
			QImage imFlag;

			block >> personId;
			block >> personName;
			block >> personGender;
			block >> idOldEmpire;
			block >> empireName;
			block >> imFlag;

			//qDebug("DtServerChannel::ReadPacket, emits EstablishNewEmpire");
			emit EstablishNewEmpire(personId, personName, personGender, idOldEmpire, empireName, imFlag);
		}
		break;

	case P_AtomIds:
	case P_Deferred:
	case P_EmpireChanged:
	case P_EmpireDeleted:
	case P_Invalid:
	case P_PersonDeleted:
	case P_QueryAtomPresence:
	case P_QueryHengeTeleporterPresence:
	case P_ReloadStory:
	case P_RemoveStory:
	case P_SectorAssigned:
	case P_SectorDecayed:
	case P_SectorRevoked:
	case P_BuildingDesignIds:
	case P_ItemDesignIds:
	case P_ShipDesignIds:
	case P_SpawnPersonOnPlanet:
	case P_StoryIds:
	case P_SystemAssigned:
	case P_SystemRevoked:
	case P_RenameWormhole:
	case P_SpeciesChanged:
		// These packet types are only sent by the boss to the sub server so we should never receive them here.
		if(g_Au.m_LogMessages & LOG_Warning)
			g_Au.Warning(tr("%1 Unexpected TCP packet type %2 size %3.")
					.arg(m_Id).arg(type).arg(ba.size()));
		m_BadPackets++;
		break;

		/*
#if defined(QT_NO_DEBUG)	// allow warning for unhandled switch condition
	default:
		if(g_Au.m_LogMessages & LOG_Error)
			g_Au.Error(tr("%1 Unknown TCP packet type %4 size %5 from %6. Hacker?")
				.arg(m_Id).arg(type).arg(ba.size()).arg(tcpSocket.peerAddress().toString()));
		m_BadPackets++;
		break;
#endif
*/
	}
}

// Used by the client to send an array of bytes to the server.
void DtServerChannel::SendPacket(const QByteArray& ba)
{
	// The packet is already formatted. Add it to the transmit data stream.
	//qDebug("DtServerChannel::SendPacket - tx %d bytes", ba.size());
	m_muTxBuffer.lock();
	AuUtil::WritePacketSize(m_TxBuffer, ba.size());
	m_TxBuffer.append(ba);
	m_muTxBuffer.unlock();
}

void DtServerChannel::Disconnect()
{
	// Signal the run loop to disconnect.
	m_tmDisconnect.Set();
}

// Defer the connecting SceneWorker to a subordinate SceneManager of this
// SceneMaster.
void DtServerChannel::DeferSceneWorker()
{
	// I am a SceneMaster redirecting a SceneWorker to one of my SceneManagers.
	//Q_ASSERT(g_Au.IsSceneMaster());
	DhSceneServer toServer = auSceneServerApp->DeferSceneToManager();
	if(toServer.IsValid())
	{
		bool isPeer = PeerAddress().left(8) == "192.168." ||
			PeerAddress() == "127.0.0.1" || PeerAddress().right(9) == "127.0.0.1" ||
			PeerAddress().left(15) == "::ffff:192.168.";
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)P_Deferred;
		if(isPeer)
			out << toServer.m_PeerName;
		else
			out << toServer.m_NetName;
		SendPacket(ba);
		Disconnect();

		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		{
			QString logmsg = tr("Deferred Scene Worker %1 to Scene Manager %2 %3.").arg(m_Id).arg(toServer.m_Id)
				.arg(isPeer ? toServer.m_PeerName : toServer.m_NetName);
			g_Au.Log(logmsg);
		}
	}
	else
	{
		if(g_Au.m_LogMessages & LOG_Error)
			qWarning("ERROR! %d Failed to defer Scene Worker.", m_Id);
	}
}

// Dole ids to a subordinate in response to their request.
void DtServerChannel::DoleAtomIds()
{
	Q_ASSERT(m_DoleAtomIds);
	if(!g_AuDb.HasAtomIds())
	{
		// I don't have any atom ids yet so I can't dole them to the sub server yet.
		// The subordinate server waits patiently for the response.
		//qDebug("%d Dole %d atom ids pending", m_Id, m_DoleAtomIds);
		return;
	}

	AuId nextId = g_Au.NextAtomId(m_DoleAtomIds);

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_AtomIds;
	nextId >> out;
	out << m_DoleAtomIds;
	SendPacket(ba);

	if(g_Au.m_LogMessages & LOG_AtomCreate)
	{
		QString logmsg = tr("%1 Doled %2 atom ids starting at %3.").arg(m_Id).arg(m_DoleAtomIds).arg(nextId.GetID());
		g_Au.Log(logmsg);
	}

	m_DoleAtomIds = 0;
}

// Dole ids to a subordinate in response to their request.
void DtServerChannel::DoleBuildingDesignIds()
{
	Q_ASSERT(m_DoleBuildingDesignIds);
	if(!g_AuDb.HasBuildingDesignIds())
	{
		// I don't have any design ids yet so I can't dole them to the sub server yet.
		// The subordinate server waits patiently for the response.
		//qDebug("%d Dole %d building design ids pending", m_Id, m_DoleBuildingDesignIds);
		return;
	}

	quint32 nextId = g_Au.NextBuildingDesignId(m_DoleBuildingDesignIds);

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_BuildingDesignIds;
	out << nextId;
	out << m_DoleBuildingDesignIds;
	SendPacket(ba);

	if(g_Au.m_LogMessages & LOG_SpacecraftDesign)
	{
		QString logmsg = tr("%1 Doled %2 building design ids starting at %3.")
			.arg(m_Id).arg(m_DoleBuildingDesignIds).arg(nextId);
		g_Au.Log(logmsg);
	}

	m_DoleBuildingDesignIds = 0;
}

// Dole ids to a subordinate in response to their request.
void DtServerChannel::DoleSpaceShipDesignIds()
{
	Q_ASSERT(m_DoleSpaceShipDesignIds);
	if(!g_AuDb.HasSpaceShipDesignIds())
	{
		// I don't have any design ids yet so I can't dole them to the sub server yet.
		// The subordinate server waits patiently for the response.
		//qDebug("%d Dole %d space ship design ids pending", m_Id, m_DoleSpaceShipDesignIds);
		return;
	}

	quint32 nextId = g_Au.NextSpaceShipDesignId(m_DoleSpaceShipDesignIds);

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_ShipDesignIds;
	out << nextId;
	out << m_DoleSpaceShipDesignIds;
	SendPacket(ba);

	if(g_Au.m_LogMessages & LOG_SpacecraftDesign)
	{
		QString logmsg = tr("%1 Doled %2 space ship design ids starting at %3.")
			.arg(m_Id).arg(m_DoleSpaceShipDesignIds).arg(nextId);
		g_Au.Log(logmsg);
	}

	m_DoleSpaceShipDesignIds = 0;
}

// Dole ids to a subordinate in response to their request.
void DtServerChannel::DoleStoryIds()
{
	Q_ASSERT(m_DoleStoryIds);
	if(!g_AuDb.HasStoryIds())
	{
		// I don't have any story ids yet so I can't dole them to the sub server yet.
		// The subordinate server waits patiently for the response.
		//qDebug("%d Dole %d story ids pending", m_Id, m_DoleStoryIds);
		return;
	}

	quint32 nextId = g_Au.NextStoryId(m_DoleStoryIds);

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_StoryIds;
	out << nextId;
	out << m_DoleStoryIds;
	SendPacket(ba);

	if(g_Au.m_LogMessages & LOG_Story)
	{
		QString logmsg = tr("%1 Doled %2 story ids starting at %3.").arg(m_Id).arg(m_DoleStoryIds).arg(nextId);
		g_Au.Log(logmsg);
	}

	m_DoleStoryIds = 0;
}

// Posts a query to my sub server in search of an atom with a certain id.
// If empire is non-zero, the atom must be friendly to the empire.
// Empire is ignored if it is zero.
// The result is emitted as the AtomPresence signal.
void DtServerChannel::QueryAtomPresence(AuId idAtom, quint32 empire, AuId personToSpawn)
{
	if(g_Au.m_LogMessages & LOG_AtomQuery)
	{
		QString logmsg = tr("DtServerChannel::QueryAtomPresence %1 peer %2 atom %3 empire %4 person %5")
			.arg(m_Id).arg(PeerAddress()).arg(idAtom.GetID()).arg(empire).arg(personToSpawn.GetID());
		g_Au.Log(logmsg);
	}

	// Remember that we have posted this query.
	AuAtomQuery query;
	query.m_Atom = idAtom;
	query.m_Empire = empire;
	m_AtomQueryMutex.lock();
	m_AtomQueries.append(query);
	m_AtomQueryMutex.unlock();

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_QueryAtomPresence;
	idAtom >> out;
	out << empire;
	personToSpawn >> out;
	SendPacket(ba);

	if(g_Au.m_LogMessages & LOG_AtomQuery)
	{
		QString logmsg = tr("DtServerChannel::QueryAtomPresence %d peer %s done")
			.arg(m_Id)
			.arg(PeerAddress());
		g_Au.Log(logmsg);
	}
}

// Posts a query to my sub server in search of the destination of a henge teleporter.
// The result is emitted as the HengeTeleporterPresence signal.
void DtServerChannel::QueryHengeTeleporterPresence(AuHengeTeleportAtom hengeTel)
{
	if(g_Au.m_LogMessages & LOG_AtomQuery)
	{
		QString logmsg = tr("DtServerChannel::QueryHengeTeleporterPresence %1 peer %2 destSys %3 sun %4 arc %5 teleport %6")
			.arg(m_Id)
			.arg(PeerAddress())
			.arg(hengeTel.m_DestSysId.GetID())
			.arg(hengeTel.m_DestSun)
			.arg(hengeTel.m_DestArc)
			.arg(hengeTel.m_TeleportId.GetID());
		g_Au.Log(logmsg);
	}

	// Remember that we have posted this query.
	AuHengeTeleportDestQuery query;
	query.m_TeleportId = hengeTel.m_TeleportId;

	m_AtomQueryMutex.lock();
	m_HengeTeleporterQueries.append(query);
	m_AtomQueryMutex.unlock();

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_QueryHengeTeleporterPresence;
	out << hengeTel;
	SendPacket(ba);

	if(g_Au.m_LogMessages & LOG_AtomQuery)
	{
		QString logmsg = tr("DtServerChannel::QueryHengeTeleporterPresence %d peer %s done")
			.arg(m_Id)
			.arg(PeerAddress());
		g_Au.Log(logmsg);
	}
}

// Load expressed as a number for comparison against a peer server's load.
// Returning 0xffffffff indicates the server is not ready for assignments.
quint32 DtServerChannel::CurrentSubLoad() const
{
	// Test for 'not ready' status.
	if(m_SubTickTime == 0xffffffff)
		return m_SubTickTime;

	// On scene master, scene manager has reported the average tick time of its workers.
	// On scene manager, scene worker has reported its total tick time and the number of CPUs/cores it has.
	if(g_Au.IsSceneMaster() || m_SubAtomMgrs < 2)
		return m_SubTickTime;

	return m_SubTickTime/m_SubAtomMgrs;
}

bool DtServerChannel::IsAwaitingAtomQuery(const AuId& atom, unsigned int empire) const
{
	bool isWaiting = false;
	m_AtomQueryMutex.lock();
	for(QVector<AuAtomQuery>::const_iterator it = m_AtomQueries.cbegin();
			it != m_AtomQueries.cend(); it++)
	{
		if((*it).m_Atom == atom && (*it).m_Empire == empire)
		{
			isWaiting = true;
			break;
		}
	}
	m_AtomQueryMutex.unlock();
	return isWaiting;
}

void DtServerChannel::HandoffAtom(const AuId& person, const AuId& dest, quint8 rideAlso, bool limboRescue,
		const QByteArray& atomData, const AuId& returnOnFail)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_HandoffElsewhere;
	out << person;
	out << dest;
	out << rideAlso;
	out << limboRescue;
	out << atomData;
	out << returnOnFail;
	SendPacket(ba);
}

void DtServerChannel::SpawnAtom(const AuId& personOrShip, quint32 empire, const AuId& spawnCity, const AuId& spawnShip,
		const AuId& spawnAt, const AuId& arena, GALAXY galaxy)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_AtomSpawning;
	out << personOrShip;
	out << empire;
	out << spawnCity;
	out << spawnShip;
	out << spawnAt;
	out << arena;
	out << (quint8)galaxy;
	SendPacket(ba);
}

void DtServerChannel::HengeTeleportAtom(const AuHengeTeleportAtom& hengeTel)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_HengeTeleportAtom;
	out << hengeTel;
	SendPacket(ba);
}

/*
void DtServerChannel::SpawnPerson(const AuId& person, quint32 empire, const AuId& spawnCity, const AuId& spawnShip,
		const AuId& spawnAt, bool isAlternate, const QByteArray& atomData)
{
	// Data fits within a 64k packet size.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_SpawnPersonElsewhere;
	person >> out;
	out << empire;
	spawnCity >> out;
	spawnShip >> out;
	spawnAt >> out;
	out << (quint8)isAlternate;
	out << atomData;
	SendPacket(ba);
}

void DtServerChannel::SpawnShip(const AuId& person, quint32 empire, const AuId& spawnCity,
		const AuId& spawnAt, bool isAlternate, const QByteArray& atomData)
{
	// Data fits within a 64k packet size.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_SpawnShipElsewhere;
	person >> out;
	out << empire;
	spawnCity >> out;
	spawnAt >> out;
	out << (quint8)isAlternate;
	out << atomData;
	SendPacket(ba);
}
*/

void DtServerChannel::ListSystemsToRevoke(float loadToRevoke, AuIdList& revokeSystems)
{
	// Each worker should have a minimum number of assigned systems equal to the number of CPUs/cores they have.
	m_SubStatusLock.lockForRead();
	int subManagerAtoms = m_SubSystemClients.size();
	int revokable = subManagerAtoms - m_SubAtomMgrs;
	if(revokable <= 0)
	{
		m_SubStatusLock.unlock();
		return;
	}

	// Take a guess at loads based on tick/tock times.
	quint64 totalTick = 0;
	quint64 emptyTick = 0;
	quint64 peopleTick = 0;
	for(unordered_map<AuId, AuSystemStatus, AuIdHash>::const_iterator sci = m_SubSystemClients.cbegin();
			sci != m_SubSystemClients.cend(); sci++)
	{
		totalTick += (*sci).second.m_Tick;
		if((*sci).second.m_Clients)
			peopleTick += (*sci).second.m_Tick;
		else
			emptyTick += (*sci).second.m_Tick;
	}

	quint64 tickToRevoke = (quint64)ceilf(totalTick * loadToRevoke);
	if(emptyTick)
	{
		// Look for systems with no people in them.
		for(unordered_map<AuId, AuSystemStatus, AuIdHash>::const_iterator sci = m_SubSystemClients.cbegin();
				sci != m_SubSystemClients.cend(); sci++)
		{
			if((*sci).second.m_Clients)
				continue;	// Skip systems that have people logged into them in this pass.
			if(!(*sci).second.m_Tick)
				continue;	// Skip systems that are not ticking.
			if((*sci).second.m_Tick <= tickToRevoke)
			{
				revokeSystems.append((*sci).first);
				tickToRevoke -= (*sci).second.m_Tick;
			}
		}
	}
	if(peopleTick)
	{
		// Look for systems with people in them.
		for(unordered_map<AuId, AuSystemStatus, AuIdHash>::const_iterator sci = m_SubSystemClients.cbegin();
				sci != m_SubSystemClients.cend(); sci++)
		{
			if(!(*sci).second.m_Clients)
				continue;	// Skip systems that have no people logged into them in this pass.
			if(!(*sci).second.m_Tick)
				continue;	// Skip systems that are not ticking.
			if((*sci).second.m_Tick <= tickToRevoke)
			{
				revokeSystems.append((*sci).first);
				tickToRevoke -= (*sci).second.m_Tick;
			}
		}
	}
	m_SubStatusLock.unlock();
}

void DtServerChannel::SendJobOffer(AuJobOffer jobOffer)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_JobOffer;
	out << jobOffer;
	SendPacket(ba);
}

void DtServerChannel::SendBountyPosted(AuId posterId, QString posterName, AuId victimId, QString reason, double reward, int why)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_BountyPosted;
	out << posterId;
	out << posterName;
	out << victimId;
	out << reason;
	out << reward;
	out << (quint8)why;
	SendPacket(ba);
}

void DtServerChannel::RenamePlanetDown(AuId planetId, QString nuName, AuId systemId, QString systemName, AuId sectorId,
		QString sectorName)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_RenamePlanet;
	out << planetId;
	out << nuName;
	out << systemId;
	out << systemName;
	out << sectorId;
	out << sectorName;
	SendPacket(ba);
}

void DtServerChannel::RenameSectorDown(AuId sectorId, QString nuName)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_RenameSector;
	out << sectorId;
	out << nuName;
	SendPacket(ba);
}

void DtServerChannel::RenameSystemDown(AuId systemId, QString nuName)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_RenameSystem;
	out << systemId;
	out << nuName;
	SendPacket(ba);
}

// Notify sub server that sector is decayed.
void DtServerChannel::SectorDecayed(AuId sectorId)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_SectorDecayed;
	out << sectorId;
	SendPacket(ba);
}

void DtServerChannel::SystemWarpSignatureBroadcast(AuId systemId, quint32 warpFactors, quint8 galaxy, float galaxyLocX,
		float galaxyLocY, float galaxyLocZ)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_SystemWarpSignature;
	out << systemId;
	out << warpFactors;
	out << galaxy;
	out << galaxyLocX;
	out << galaxyLocY;
	out << galaxyLocZ;
	SendPacket(ba);
}

// Notify sub servers that a person atom was deleted.
void DtServerChannel::PersonDeleted(AuId personId)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_PersonDeleted;
	out << personId;
	SendPacket(ba);
}

void DtServerChannel::WormholeExplored(AuId systemId, AuId toSystem, qint8 polarity, quint32 empire, QString nuName)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_RenameWormhole;
	out << systemId;
	out << toSystem;
	out << polarity;
	out << empire;
	out << nuName;
	SendPacket(ba);
}

void DtServerChannel::CityMailRequested(AuId cityId, AuId unitId, quint32 unitEmpire)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_CityMailRequested;
	out << cityId;
	out << unitId;
	out << unitEmpire;
	SendPacket(ba);
}

void DtServerChannel::EmpireChangedDown(DgEmpire empire)
{
	//qDebug() << "DtServerChannel::EmpireChangedDown" << empire.m_id << empire.Name();
	// Scene clients request empire records "as needed". A notification does not need to be sent to them.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_EmpireChanged;
	empire >> out;							// Send the empire record. Not sure if it could exceed 64k but it is dynamic.
	SendPacket(ba);
}

void DtServerChannel::SpeciesNameChanged(AuSpecies species, QString specieName, AuId whoNamed)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_SpeciesChanged;
	out << species;
	out << specieName;
	out << whoNamed;
	SendPacket(ba);
}

void DtServerChannel::CloseStargateDown(AuId fromGalaxy, AuPoint fromGalaxyLoc)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_StargateClosed;
	out << fromGalaxy;
	out << fromGalaxyLoc;
	SendPacket(ba);
}

void DtServerChannel::OpenStargateDown(AuId fromGalaxy, AuPoint fromGalaxyLoc, AuId toGalaxy, AuPoint toGalaxyLoc, AuId toSector,
		AuId toSystem, AuTime expire)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_StargateOpened;
	out << fromGalaxy;
	out << fromGalaxyLoc;
	out << toGalaxy;
	out << toGalaxyLoc;
	out << toSector;
	out << toSystem;
	expire >> out;
	SendPacket(ba);
}

void DtServerChannel::ArenaReadyDown(AuArenaReady arena)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_ArenaReady;
	out << arena;
	SendPacket(ba);
}

void DtServerChannel::SpacecraftNameChangedDown(AuId shipId, QString nuName)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_SpacecraftNameChanged;
	out << shipId;
	out << nuName;
	SendPacket(ba);
}

void DtServerChannel::HengeDestinationDown(AuId sectorId, AuId systemId, QString systemName, quint8 sun, QString worldCatName,
		QString worldName, quint8 worldType, AuGlobeLLF hengeLL, bool breathable, quint8 orbitZone, bool outsideTown)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_HengeDestination;
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

void DtServerChannel::SpacecraftStasisRecoverDown(AuId shipId, AuId personId, quint8 personGalaxy, bool agentRecover)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_ShipStasisRecover;
	out << shipId;
	out << personId;
	out << personGalaxy;
	out << agentRecover;
	SendPacket(ba);
}

void DtServerChannel::ScDesignDeactivated(quint32 designId)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_ScDesignDeactivated;
	out << designId;
	SendPacket(ba);
}

void DtServerChannel::ScDesignExchangePublished(AuScDesignExchangeEntry item)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_ScDesignExchangePublished;
	out << item;
	SendPacket(ba);
}

void DtServerChannel::ScDesignExchangeUnpublished(quint32 designId)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_ScDesignExchangeUnpublished;
	out << designId;
	SendPacket(ba);
}

void DtServerChannel::BdDesignDeactivated(quint32 designId)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_BdDesignDeactivated;
	out << designId;
	SendPacket(ba);
}

void DtServerChannel::BdDesignExchangePublished(AuBdDesignExchangeEntry item)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_BdDesignExchangePublished;
	out << item;
	SendPacket(ba);
}

void DtServerChannel::BdDesignExchangeUnpublished(quint32 designId)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_BdDesignExchangeUnpublished;
	out << designId;
	SendPacket(ba);
}

void DtServerChannel::SendPlayerStatus(AuPlayerOnlineStatusVector players)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_Status;
	players >> out;
	SendPacket(ba);
}

void DtServerChannel::CollectBountyDown(AuId bountyHunter, QString bountyHunterName, AuGearPos gearPos, AuBountyOnItem bountyOnItem)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_CollectBounty;
	out << bountyHunter;
	out << bountyHunterName;
	out << gearPos;
	out << bountyOnItem;
	SendPacket(ba);
}

void DtServerChannel::PayBountyDown(AuId bountyHunter, AuGearPos gearPos, AuBountyOnItem bountyOnItem, bool victimHadBounty)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_PayBounty;
	out << bountyHunter;
	out << gearPos;
	out << bountyOnItem;
	out << victimHadBounty;
	SendPacket(ba);
}

void DtServerChannel::BlueprintRatedBdDown(AuExchangeRatingRec rating)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_BlueprintRatedBd;
	out << rating;
	SendPacket(ba);
}

void DtServerChannel::BlueprintRatedScDown(AuExchangeRatingRec rating)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_BlueprintRatedSc;
	out << rating;
	SendPacket(ba);
}

void DtServerChannel::PirateBaseEncountered(AuId inSystemId)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_SectorPirateBase;
	out << inSystemId;
	SendPacket(ba);
}

void DtServerChannel::AvatarAccountActive(AuIdList land)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_AvatarAccountActive;
	out << land;
	SendPacket(ba);
}

void DtServerChannel::ClaimTitle(AuId propertyId, QString propertyName, AuId ownerId, QString ownerName, quint32 ownerEmpire)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_RedeemShipTitle;
	out << propertyId;
	out << propertyName;
	out << ownerId;
	out << ownerName;
	out << ownerEmpire;
	SendPacket(ba);
}

// Dole ids to a subordinate in response to their request.
void DtServerChannel::DoleItemDesignIds()
{
	Q_ASSERT(m_DoleItemDesignIds);
	if(!g_AuDb.HasItemDesignIds())
	{
		// I don't have any design ids yet so I can't dole them to the sub server yet.
		// The subordinate server waits patiently for the response.
		//qDebug("%d Dole %d item design ids pending", m_Id, m_DoleItemDesignIds);
		return;
	}

	quint32 nextId = g_Au.NextItemDesignId(m_DoleItemDesignIds);

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_ItemDesignIds;
	out << nextId;
	out << m_DoleItemDesignIds;
	SendPacket(ba);

	if(g_Au.m_LogMessages & LOG_SpacecraftDesign)
	{
		QString logmsg = tr("%1 Doled %2 item design ids starting at %3.")
			.arg(m_Id).arg(m_DoleItemDesignIds).arg(nextId);
		g_Au.Log(logmsg);
	}

	m_DoleItemDesignIds = 0;
}

void DtServerChannel::ItDesignDeactivated(quint32 designId)
{
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)P_ItDesignDeactivated;
	out << designId;
	SendPacket(ba);
}

