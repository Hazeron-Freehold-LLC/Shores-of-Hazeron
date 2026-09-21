//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtclientchannel.h,v 1.144 2022/02/19 18:39:30 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DTCLIENTCHANNEL_H
#define DTCLIENTCHANNEL_H

#include "auarenaready.h"
#include "audomain.h"
#include "auexchangerating.h"
#include "augear.h"
#include "auid.h"
#include "aujoboffer.h"
#include "auplayeronlinestatus.h"
#include "auscdesignexchange.h"
#include "aushipment.h"
#include "dgempire.h"
#include "dtdb.h"
#include <QMutex>
#include <QSemaphore>
#include <QTcpSocket>
#include <QThread>

class DtClientChannel : public QThread
{
	Q_OBJECT;

public:
	DtClientChannel(QObject* parent=0);
	~DtClientChannel();

	bool Alive() const
	{
		return !m_semAlive.available();
	}

	void Kill()
	{
		m_semAlive.release(1);
	}

	void HengeActivated(const AuId& fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			const AuId& destSysId, quint8 destSun, quint8 destArcSegment);

	void HengeCreated(const AuId& fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			const AuId& destSysId, quint8 destSun, quint8 destArcSegment);

	void HengeDeactivated(const AuId& fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			const AuId& destSysId, quint8 destSun, quint8 destArcSegment);

	void SendAtomPresence(const AuId& atom, unsigned int empire, int result);

	void SendHengeDestinationConfirmed(const AuId& sectorId, const AuId& systemId, const QString& systemName, quint8 sun,
			const QString& worldCatName, const QString& worldName, quint8 worldType, const AuGlobeLLF& hengeLL, bool breathable,
			quint8 orbitZone, bool outsideTown);

	void SendHengeTeleportDestPresence(const AuId& teleportId, int result);

	void SendBountyPosted(const AuId& posterId, const QString& posterName, const AuId& victim, const QString& reason, double reward,
			int why);

	void SendJobOffer(const AuJobOffer& jobOffer);

	void SendPacket(const QByteArray& ba);

public slots:
	void SendStatus();
	void TxData(QTcpSocket& tcpSocket);

signals:
	void AdminSummon(AuId person, AuId adminId, quint8 rideAlso);

	void ArenaReady(AuArenaReady arena);

	void AssignOffice(AuId person, quint32 empire, AuOffice office, AuId spawnCity, QString spawnCityName, AuId spawnSystem,
			QString spawnSystemName);

	// 'galaxy' is a GALAXY enum value.
	void AtomSpawning(AuId personOrShip, quint32 empire, AuId spawnCity, AuId spawnShip, AuId spawnAt, AuId arena, quint8 galaxy);

	void AtomTraveling(AuId traveler, AuId destSystem, AuId destSector);

	void AvatarAccountActiveSignal(AuIdList land);

	void BlueprintRatedBd(AuExchangeRatingRec rating);

	void BlueprintRatedSc(AuExchangeRatingRec rating);

	void BountyPosted(AuId posterId, QString posterName, AuId victimId, QString reason, double reward, int why);

	void ChatDown(QByteArray msg);

	// City report mail request on its way down the server farm
	void CityMailRequestedSignal(AuId cityId, AuId unitId, quint32 unitEmpire);

	void CloseStargateDown(AuId fromGalaxyId, AuPoint fromGalaxyLoc);

	void CollectBounty(AuId bountyHunter, QString bountyHunterName, AuGearPos gearPos, AuBountyOnItem bountyOnItem);

	void EmpireChanged(DgEmpire empire);

	void EmpireRecordDeleted(quint32 empireId);

	void HengeActivatedSignal(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			AuId destSysId, quint8 destSun, quint8 destArcSegment);

	void HengeCreatedSignal(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			AuId destSysId, quint8 destSun, quint8 destArcSegment);

	void HengeDeactivatedSignal(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			AuId destSysId, quint8 destSun, quint8 destArcSegment);

	void HengeDestinationConfirmed(AuId sectorId, AuId systemId, QString systemName, quint8 sun, QString worldCatName, QString worldName,
			quint8 worldType, AuGlobeLLF hengeLL, bool breathable, quint8 orbitZone, bool outsideTown);

	void HengeTeleportAtom(AuHengeTeleportAtom hengeTel);

	// 'scope' is a AuCommStdChannel::SCOPE
	void OpenChannelDown(AuId fromUnitId, QString fromName, AuId fromSector, quint8 fromGalaxy, AuId toUnitId, QString channelName,
			quint8 scope);

	void OpenStargateDown(AuId fromGalaxyId, AuPoint fromGalaxyLoc, AuId toGalaxyId, AuPoint toGalaxyLoc, AuId toSector,
			AuId toSystem, AuTime expire);

	void PayBounty(AuId bountyHunter, AuGearPos gearPos, AuBountyOnItem bountyOnItem, bool victimHadBounty);

	void PlayerOnlineStatus(AuPlayerOnlineStatusVector players);

	void BdDesignDeactivatedSignal(quint32 designId);

	void BdDesignExchangePublishedSignal(AuBdDesignExchangeEntry item);

	void BdDesignExchangeUnpublishedSignal(quint32 designId);

	void ClaimTitleSignal(AuId propertyId, QString propertyName, AuId ownerId, QString ownerName, quint32 ownerEmpire);

	void ItDesignDeactivatedSignal(quint32 designId);

	void AcceptTrade(AuTradeAgreement ta);

	void DeactivateTrade(AuTradeAgreement ta, quint8 nuState, bool atDest);

	void ProposeTrade(AuTradeAgreement ta);

	void ScDesignDeactivatedSignal(quint32 designId);

	void ScDesignExchangePublishedSignal(AuScDesignExchangeEntry item);

	void ScDesignExchangeUnpublishedSignal(quint32 designId);

	void ShipIt(AuItemTEUContainer shipment);

	void SpacecraftNameChangedDown(AuId shipId, QString nuName);

	void SpacecraftStasisRecover(AuId shipId, AuId personId, quint8 personGalaxy, bool agentRecover);

	// 'galaxy' is a GALAXY enum value.
	void SystemWarpSignatureDown(AuId systemId, quint32 warpFactors, quint8 galaxy, float galaxyLocX, float galaxyLocY,
			float galaxyLocZ);

	void TradeReadyToShip(AuId readyCapitolId, AuTradeAgreement ta);

	void TradeShipped(AuId shipperCapitolId, AuTradeAgreement ta);

	void Connected();
	void CreditCityBank(float fAmount, AuId cityId);
	void GovCitySurrendered(AuId byWho, AuId cityId, quint32 fromEmpire, quint32 toEmpire);
	void HandoffElsewhere(AuId person, AuId dest, quint8 rideAlso, bool limboRescue, QByteArray atomData, AuId returnOnFail);
	void OfficerKilled(const AuId);

	void PersonDeleted(AuId personId);

	void AvatarOnLine(AuId personId, quint32 empireId);

	void PirateBaseEncountered(AuId inSystemId);

	void PostJobOffer(AuJobOffer jobOffer);

	void QueryAtomServer(AuId idAtom, quint32 empire, AuId personOrShipToSpawn);
	void QueryHengeTeleporterServer(AuHengeTeleportAtom hengeTel);

	void RenamePlanet(AuId planetId, QString nuName, AuId systemId, QString systemName, AuId sectorId, QString sectorName);
	void RenameSector(AuId sectorId, QString nuName);
	void RenameSystem(AuId systemId, QString nuName);

	void RenameWormhole(AuId systemId, AuId toSystem,
			qint8 polarity, quint32 empire, QString nuName);
	void ReleaseBerth(AuId personId, AuId shipId);
	void ReloadStory(quint32 storyInstance);
	void RemoveStory(quint32 storyInstance);

	// 'galaxy' is a GALAXY enum value
	void SectorAssigned(AuId idSector, int systemsInSector, quint8 galaxy);

	void SectorDecayed(AuId sectorId);

	void SectorRevoked(AuId idSector);

	//void SectorVisited(AuId sectorId, quint32 empireId);

	// 'fromGalaxy' is a GALAXY value.
	void SendFunds(AuId fromId, QString fromName, AuId toId, double amount, quint8 fromGalaxy, quint8 reason, quint32 designId);

	// 'galaxy' is a GALAXY enum value
	void SpawnPersonOnPlanet(AuId personId, AuId sectorOrWorldId, quint8 galaxy);

	void SpeciesChanged(AuSpecies species, QString specieName, AuId discoverer);

	void SummonDormantShip(AuId shipId, AuId crewId);

	// 'galaxy' is a GALAXY enum value
	void SystemAssigned(AuId idSector, AuId idSystem, quint8 galaxy);

	void SystemExplored(AuId systemId, quint32);
	void SystemScanned(AuId systemId, quint32, quint8 genHab, quint32 genSeed);
	void SystemNameChanged(AuId systemId, QString systemName, AuId sectorId, QString sectorName);

	void SystemRevoked(AuId idSystem);

protected:
	virtual void run();

private:
	void RestockNextIds();

private slots:
	void NetworkError(QAbstractSocket::SocketError socketError);
	bool ReadPacket(const QByteArray& ba, QTcpSocket& tcpSocket);
	void RxData(QTcpSocket& tcpSocket);

private:
	bool			m_AwaitingAtomIdRestock;
	bool			m_AwaitingBuildingDesignIdRestock;
	bool			m_AwaitingItemDesignIdRestock;
	bool			m_AwaitingShipDesignIdRestock;
	bool			m_AwaitingStoryIdRestock;

	int				m_BytesRx;			// Total bytes received from client
	int				m_BytesTx;			// Total bytes sent to client
	QMutex			m_muTxBuffer;		// TCP tx buffer mutex
	quint32			m_PacketSize;		// Data packet size.
	int				m_PacketSizeBytesLeft;	// Number of packet size bytes we are expecting.
	QHash<quint32, quint32>	m_PacketsRx;	// Count of packets received, keyed by packet type.
	QSemaphore		m_semAlive;			// Used to know when I'm supposed to die.
	QByteArray		m_TxBuffer;			// TCP tx buffer
};

#endif // DTCLIENTCHANNEL_H
