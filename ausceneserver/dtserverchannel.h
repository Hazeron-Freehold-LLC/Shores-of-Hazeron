//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtserverchannel.h,v 1.241 2022/02/19 18:39:34 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DtServerChannel_H
#define DtServerChannel_H

#include "auid.h"
#include "auplayeronlinestatus.h"
#include "ausceneserverapp.h"
#include "autime.h"
#include <QHostAddress>
#include <QMap>
#include <QMutex>
#include <QSemaphore>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QThread>
#include <unordered_map>

class DiControlInputArray;
class QImage;

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

class DtServerChannel : public QThread
{
	Q_OBJECT;
public:
	// Atoms are only sent to the client from the server, never back the other
	// way.
	// Other packets vary in usage: client-to-server or server-to-client or
	// both.
	enum PACKET
	{
		P_Invalid,
		P_AddCitizen,
		P_AdminSummon,		// Propagates admin summon request up to master.
		P_ArenaReady,
		P_AtomIds,			// Sent by boss to sub with stock of atom ids.
		P_AtomPresence,		// Sent by sub to boss with query result 1=yes, 0=no
		P_AtomSpawning,		// Sent by sub to boss with atom to spawn elsewhere. Sent by boss to sub with atom to spawn and loc.
		P_AtomTraveling,	// Sent by sub with atom id and dest system and sector ids.
		P_AtomTravelingGalaxyLoc,	// Sent by sub with atom id and dest galaxy and galaxy loc.
		P_AtomTravelingStargate,	// Sent by sub with atom id and dest galaxy and galaxy loc.
		P_BdDesignDeactivated,
		P_BdDesignExchangePublished,
		P_BdDesignExchangeUnpublished,
		P_BlueprintRatedBd,
		P_BlueprintRatedSc,
		P_BountyPosted,					// Sent by sub to boss to propagate bounty posting
		P_BuildingDesignIds,	// Sent by boss to sub with stock of building design ids.
		P_Chat,				// Sent by sub to boss to propagate a chat message. Sent by boss to sub to propagate a chat message.
		P_CityCaptured,
		P_CityCreated,
		P_CityDecayed,
		P_CityFacilities,
		P_CityMailRequested,
		P_CollectBounty,
		P_CreditCityBank,		// Credit city bank treasury with money.
		P_Deferred,			// Sent by boss to subordinate with nu boss name.
		P_EmpireChanged,	// Sent by boss to subs with empire record added/changed.
		P_EmpireChg,
		P_EmpireChgFlag,	// Sent by sub to boss with empire id and nu flag.
		P_EmpireChgName,	// Sent by sub to boss with empire id and nu name.
		P_EmpireChgRole,	// Sent by sub to boss with empire id and nu role.
		P_EmpireCityWorkerDNAChanged,
		P_EmpireDeleted,	// Sent by boss to subs with empire id deleted.
		P_EmpireRecordPatent,
		P_EstNewEmpire,
		P_GovBossChanged,
		P_GovCitySurrendered,
		P_GovDossierAddDocument,
		P_GovDossierChangeState,
		P_GovSetAnnouncements,			// Sent by sub to boss with empire and announcements.
		P_GovSetCapital,	// Sent by sub to boss with empire and city ids.
		P_GovSetCommerce,	// Sent by sub to boss with empire and commodity com
		P_GovSetContrailColor,	// Sent by sub to boss with empire and hue
		P_GovSetDefStance,	// Sent by sub to boss with empire and pol stance.
		P_GovSetTax,		// Sent by sub to boss with empire and tax percents.
		P_GovSetThemeColor,			// Sent by sub to boss with empire and announcements.
		P_GovSetWarCriteria,
		P_HandoffElsewhere,
		P_HengeActivated,
		P_HengeCreated,
		P_HengeDeactivated,
		P_HengeDestination,
		P_HengeTeleportAtom,
		P_HengeTeleporterPresence,		// Sent by sub to boss with query result 0=no, 1=yes, 2=not ready, 3=not possible
		P_JobOffer,						// Sent by sub to boss to propagate job offer.
		P_LeaveEmpire,
		P_NameSpecies,		// Sent by sub to boss.
		P_OfficeAssigned,	// Sent by sub to boss to notify empire record on manager of assignment chg.
		P_OfficeRevoked,
		P_OfficerKilled,		// Propagates a message to the officer's home world.
		P_OpenChannelTo,	// Sent by sub to boss to propagate open channel request. Sent by boss to sub to propagate open request.
		P_PayBounty,
		P_PersonDeleted,
		P_AvatarOnLine,
		P_QueryAtomPresence,	// Sent by boss to sub with atom and empire ids.
		P_QueryHengeTeleporterPresence,	// Sent by boss to sub
		P_RedeemShipTitle,
		P_ReleaseBerth,			// Propagates person releasing berth to entire farm.
		P_ReloadStory,		// Sent by boss to subs with storyInstance added/changed.
		P_RemoveStory,		// Sent by boss to subs with storyInstance deleted.
		P_RenameAvatar,
		P_RenameCity,
		P_RenamePlanet,
		P_RenameSector,
		P_RenameSystem,
		P_RenameWormhole,	// Sent by boss to subs to change wormhole name. Passed down to entire farm when a wormhole is explored.
		P_RequestAtomIds,	// Sent by sub to boss to get more atom ids.
		P_RequestBuildingDesignIds,	// Sent by sub to boss to get more building design ids.
		P_RequestShipDesignIds,	// Sent by sub to boss to get more ship design ids.
		P_RequestStoryIds,	// Sent by sub to boss to get more story ids.
		P_ScDesignDeactivated,
		P_ScDesignExchangePublished,
		P_ScDesignExchangeUnpublished,
		P_SectorAssigned,	// Sent by boss to sub with sector id assigned.
		P_SectorDecayed,				// Sent by boss to subs with sector id that has decayed.
		P_SectorPirateBase,
		P_SectorRevoked,	// Sent by boss to sub with sector id assignment revoked.
		P_SectorVisited,
		P_SendFunds,			// Propagates a funds transfer to the recipient. Sender has already been taken care of.
		P_SetPoliticalStance,
		P_ShipDesignIds,	// Sent by boss to sub with stock of ship design ids.
		P_ShipStasisRecover,
		P_SpacecraftNameChanged,
		P_SpacecraftPosition,
		P_SpawnPersonOnPlanet,
		P_SpeciesChanged,	// Sent by boss to subs with species added/changed.
		P_StargateClosed,
		P_StargateOpened,
		P_Status,			// Sent by subordinate to boss. Send down to the sub with status of all players online.
		P_StoryChanged,		// Sent by sub to boss when story added/changed.
		P_StoryDeleted,		// Sent by sub to boss when story deleted.
		P_StoryIds,			// Sent by boss to sub with stock of story ids.
		P_SummonDormantShip,
		P_SystemAssigned,	// Sent by boss to sub with system and sector ids.
		P_SystemWarpSignature,
		P_SystemDecayed,	// Sent by sub to boss with system id that is ready to decay its stars and planets.
		P_WarpSignatureDetected,
		P_SystemExplored,	// Solar system was "explored" for the first time by an empire.
		P_SystemFinal,		// Sent by manager to master when system contents finalized.
		P_SystemNameChanged,	// The name of a solar system and possibly its parent sector have changed.
		P_SystemRevoked,	// Sent by boss to sub with system id assignment revoked.
		P_SystemScanned,		// Solar system was thoroughly scanned by an empire.
		P_Trade,
		P_WormholeExplored,	// Sent by sub to boss when a wormhole is explored.
		P_CityStatus,
		P_AvatarAccountActive,
		P_ItemDesignIds,	// Sent by boss to sub with stock of item design ids.
		P_RequestItemDesignIds,	// Sent by sub to boss to get more item design ids.
		P_ItDesignDeactivated,
		P_GovSetJoinPass,	// Sent by sub to boss with empire and passCrypt
	};

public:
	DtServerChannel(unsigned int id, int socketDescriptor, QObject* parent=0);

	bool Alive() const
	{
		return !m_semAlive.available();
	}

	inline float AtomLoad() const
	{
		m_SubStatusLock.lockForRead();
		float l = m_SubAtomLoad;
		m_SubStatusLock.unlock();
		return l;
	}

	int BadPacketCount() const
	{
		return m_BadPackets;
	}
	int BytesRx() const
	{
		return m_BytesRx;
	}
	int BytesTx() const
	{
		return m_BytesTx;
	}

	inline AuTime CheckInTime() const
	{
		m_SubStatusLock.lockForRead();
		AuTime tmCheckIn = m_SubCheckInTime;
		m_SubStatusLock.unlock();
		return tmCheckIn;
	}

	inline AuTime ConnectTime() const
	{
		return m_tmConnect;
	}

	// Load expressed as a number for comparison against a peer server's load.
	// Returning 0xffffffff indicates the server is not ready for assignments.
	// Locks m_SubStatusLock and returns the value returned by CurrentSubLoad().
	// Call CurrentSubLoad if m_SubStatusLock is already locked.
	inline quint32 CurrentLoad() const
	{
		m_SubStatusLock.lockForRead();
		quint32 subLoad = CurrentSubLoad();
		m_SubStatusLock.unlock();
		return subLoad;
	}

	void Disconnect();

	void HandoffAtom(const AuId& person, const AuId& dest, quint8 rideAlso, bool limboRescue, const QByteArray& atomData,
			const AuId& returnOnFail);

	// Returns true if the number of systems assigned to my subordinate worker server (on this channel) exceeds the number
	// of CPUs/cores available for tick processing on the sub.
	inline bool HasExcessLoad() const
	{
		m_SubStatusLock.lockForRead();
		bool hasExcess = (m_SubSystemClients.size() > m_SubAtomMgrs) ? true : false;
		m_SubStatusLock.unlock();
		return hasExcess;
	}

	inline unsigned int Id() const
	{
		return m_Id;
	}

	inline QString InternetName() const
	{
		m_SubStatusLock.lockForRead();
		QString netName = m_SubNetName;
		m_SubStatusLock.unlock();
		return netName;
	}

	inline quint16 InternetPort() const
	{
		m_SubStatusLock.lockForRead();
		quint16 netPort = m_SubNetPort;
		m_SubStatusLock.unlock();
		return netPort;
	}

	bool IsAwaitingAtomQuery(const AuId& atom, unsigned int empire) const;

	inline bool IsSceneManager() const
	{
		return ServerMode() == 1;
	}

	inline bool IsSceneWorker() const
	{
		return ServerMode() == 2;
	}

	inline void Kill()
	{
		m_semAlive.release(1);
	}

	void ListSystemsToRevoke(float loadToRevoke, AuIdList& revokeSystems);

	inline QString PeerAddress() const
	{
		return m_peerAddress.toString();
	}

	// This is used to accumulate all player info from several server threads so we don't clear the vector before appending.
	inline void PlayersOnlineStatus(AuPlayerOnlineStatusVector& players) const
	{
		m_SubStatusLock.lockForRead();
		players.AddUnique(m_SubPlayers);
		m_SubStatusLock.unlock();
	}

	void SendAtomPresence(const AuId& atom, unsigned int empire, int result);

	void SendPacket(const QByteArray& ba);	// Adds a packet to the tx data stream.

	inline int ServerMode() const
	{
		m_SubStatusLock.lockForRead();
		int mode = m_SubServerMode;
		m_SubStatusLock.unlock();
		return mode;
	}

	void SpawnAtom(const AuId& personOrShip, quint32 empire, const AuId& spawnCity, const AuId& spawnShip, const AuId& spawnAt,
			const AuId& arena, GALAXY galaxy);

	void HengeTeleportAtom(const AuHengeTeleportAtom& hengeTel);

	//void SpawnPerson(const AuId& person, quint32 empire, const AuId& spawnCity, const AuId& spawnShip, const AuId& spawnAt,
			//bool isAlternate, const QByteArray& atomData);

	//void SpawnShip(const AuId& ship, quint32 empire, const AuId& spawnCity, const AuId& spawnAt,
			//bool isAlternate, const QByteArray& atomData);

	inline unsigned int SubPlayers() const
	{
		m_SubStatusLock.lockForRead();
		unsigned int subPlayers = m_SubPlayers.size();
		m_SubStatusLock.unlock();
		return subPlayers;
	}

	inline unsigned int SubWorkerCores() const
	{
		m_SubStatusLock.lockForRead();
		unsigned int subcores = m_SubWorkerCores;
		m_SubStatusLock.unlock();
		return subcores;
	}

	inline unsigned int SubWorkers() const
	{
		m_SubStatusLock.lockForRead();
		unsigned int subworkers = m_SubWorkers;
		m_SubStatusLock.unlock();
		return subworkers;
	}

	// Returns tick time of connected server. 1/2 millisecond increments. i.e. 100 = 50ms
	inline quint32 TickTime() const
	{
		m_SubStatusLock.lockForRead();
		quint32 tickTime = m_SubTickTime;
		m_SubStatusLock.unlock();
		return tickTime;
	}

public slots:
	void ArenaReadyDown(AuArenaReady arena);

	void AvatarAccountActive(AuIdList land);

	void BlueprintRatedBdDown(AuExchangeRatingRec rating);

	void BlueprintRatedScDown(AuExchangeRatingRec rating);

	// City mail request working its way down the server farm.
	void CityMailRequested(AuId cityId, AuId unitId, quint32 unitEmpire);

	void CloseStargateDown(AuId fromGalaxyId, AuPoint fromGalaxyLoc);

	void CollectBountyDown(AuId bountyHunter, QString bountyHunterName, AuGearPos gearPos, AuBountyOnItem bountyOnItem);

	void EmpireChangedDown(DgEmpire empire);		// Empire record sent down to subordinate servers.

	void HengeDestinationDown(AuId sectorId, AuId systemId, QString systemName, quint8 sun, QString worldCatName, QString worldName,
			quint8 worldType, AuGlobeLLF hengeLL, bool breathable, quint8 orbitZone, bool outsideTown);

	void PersonDeleted(AuId personId);

	void OpenStargateDown(AuId fromGalaxyId, AuPoint fromGalaxyLoc, AuId toGalaxyId, AuPoint toGalaxyLoc, AuId toSector,
			AuId toSystem, AuTime expire);

	void PayBountyDown(AuId bountyHunter, AuGearPos gearPos, AuBountyOnItem bountyOnItem, bool victimHadBounty);

	void PirateBaseEncountered(AuId inSystemId);

	void ClaimTitle(AuId propertyId, QString propertyName, AuId ownerId, QString ownerName, quint32 ownerEmpire);

	void QueryAtomPresence(AuId idAtom, quint32 empire, AuId personToSpawn);

	void QueryHengeTeleporterPresence(AuHengeTeleportAtom hengeTel);

	void RenamePlanetDown(AuId planetId, QString nuName, AuId systemId, QString systemName, AuId sectorId, QString sectorName);

	void RenameSectorDown(AuId sectorId, QString nuName);

	void RenameSystemDown(AuId systemId, QString nuName);

	void BdDesignDeactivated(quint32 designId);

	void BdDesignExchangePublished(AuBdDesignExchangeEntry item);

	void BdDesignExchangeUnpublished(quint32 designId);

	void ItDesignDeactivated(quint32 designId);

	void ScDesignDeactivated(quint32 designId);

	void ScDesignExchangePublished(AuScDesignExchangeEntry item);

	void ScDesignExchangeUnpublished(quint32 designId);

	void SectorDecayed(AuId sectorId);

	void SendBountyPosted(AuId posterId, QString posterName, AuId victimId, QString reason, double reward, int why);

	void SendJobOffer(AuJobOffer jobOffer);

	void SendPlayerStatus(AuPlayerOnlineStatusVector players);

	void SpacecraftNameChangedDown(AuId shipId, QString nuName);

	void SpacecraftStasisRecoverDown(AuId shipId, AuId personId, quint8 personGalaxy, bool agentRecover);

	// Sends the notification down to the sub servers.
	void SpeciesNameChanged(AuSpecies species, QString specieName, AuId whoNamed);

	// 'galaxy' is a GALAXY enum value.
	void SystemWarpSignatureBroadcast(AuId systemId, quint32 warpFactors, quint8 galaxy, float galaxyLocX, float galaxyLocY,
			float galaxyLocZ);

	void WormholeExplored(AuId systemId, AuId toSystem, qint8 polarity, quint32 empire, QString nuName);

protected slots:
	void ThreadFinished();

signals:
	void AddCitizenToEmpire(quint32 oldEmpireId, quint32 nuEmpireId, AuId personId, QString personName, quint8 personGender);

	void AdminSummon(AuId person, AuId adminId, quint8 rideAlso);

	void ArenaReady(AuArenaReady arena);

	void AtomHengeTeleporting(AuHengeTeleportAtom hengeTel);

	void AtomPresence(AuId atom, unsigned int empire, int presense, unsigned int myId);

	// 'galaxy' is a GALAXY enum value.
	void AtomSpawning(AuId traveler, quint32 empire, AuId spawnCity, AuId spawnShip, AuId arena, quint8 galaxy);

	void AtomTraveling(AuId traveler, AuId destSystem, AuId destSector);

	void AtomTravelingGalaxyLoc(AuId traveler, AuId destGalaxy, AuPoint destGalaxyLoc);

	void AtomTravelingStargate(AuId traveler, AuId destGalaxy, AuPoint destGalaxyLoc);

	void AvatarAccountActiveSignal(AuIdList land);

	void BlueprintRatedBd(AuExchangeRatingRec rating);

	void BlueprintRatedSc(AuExchangeRatingRec rating);

	void BountyPosted(AuId posterId, QString posterName, AuId victimId, QString reason, double reward, int why);

	void ChangeEmpireInDb(quint32 empireId);		// Emitted on master to notify the app to notify db and subordinates.

	void CityCaptured(quint32 oldEmpireId, quint32 nuEmpireId, AuId cityId);

	// 'cityType' is a CITYTYPE enum value.
	// 'galaxy' is a GALAXY enum value.
	void CityCreated(quint32 empireId,
			quint8 cityType, AuId cityId, QString cityName,
			AuId founderId, QString founderName,
			AuId planetId, QString planetName,
			AuId systemId, QString systemName, AuPoint systemLoc,
			AuId sectorId, QString sectorName,
			AuId galaxyId, quint8 galaxy);

	void CityDecayed(quint32 empireId, AuId cityId);

	void CityFacilitiesChanged(AuId cityId, AuCityIndustryHash facil);

	// City mail request working its way up the server farm.
	void CityMailRequestedSignal(AuId cityId, AuId unitId, quint32 unitEmpire);

	void CityStatusChanged(AuId cityId, int alertCondition, qint8 morale);

	void CollectBounty(AuId bountyHunter, QString bountyHunterName, AuGearPos gearPos, AuBountyOnItem bountyOnItem);

	void EmpireRecordPatent(quint32 empireId, quint16 patentCID, quint8 patentQ);

	void HengeActivated(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			AuId destSysId, quint8 destSun, quint8 destArcSegment);

	void HengeCreated(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			AuId destSysId, quint8 destSun, quint8 destArcSegment);

	void HengeDeactivated(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
			AuId destSysId, quint8 destSun, quint8 destArcSegment);

	void HengeDestinationConfirmed(AuId sectorId, AuId systemId, QString systemName, quint8 sun, QString worldCatName, QString worldName,
			quint8 worldType, AuGlobeLLF hengeLL, bool breathable, quint8 orbitZone, bool outsideTown);

	void HengeTeleporterPresence(AuId teleportId, int presense, unsigned int myId);

	void BdDesignDeactivatedSignal(quint32 designId);

	void BdDesignExchangePublishedSignal(AuBdDesignExchangeEntry item);

	void BdDesignExchangeUnpublishedSignal(quint32 designId);

	void ItDesignDeactivatedSignal(quint32 designId);

	void PayBounty(AuId bountyHunter, AuGearPos gearPos, AuBountyOnItem bountyOnItem, bool victimHadBounty);

	void ClaimTitleSignal(AuId propertyId, QString propertyName, AuId ownerId, QString ownerName, quint32 ownerEmpire);

	void AcceptTrade(AuTradeAgreement ta);

	void DeactivateTrade(AuTradeAgreement ta, quint8 nuState, bool atDest);

	void ProposeTrade(AuTradeAgreement ta);

	void ScDesignDeactivatedSignal(quint32 designId);

	void ScDesignExchangePublishedSignal(AuScDesignExchangeEntry item);

	void ScDesignExchangeUnpublishedSignal(quint32 designId);

	void SectorPirateBaseEncounter(AuId encSectorId, AuId encSystemId);

	void ShipIt(AuItemTEUContainer shipment);

	void SpacecraftPositionReport(AuId shipId, AuId systemId, AuStarMapSpacecraft);

	void SpacecraftStasisRecover(AuId shipId, AuId personId, quint8 personGalaxy, bool agentRecover);

	// 'galaxy' is a GALAXY enum value.
	void SystemWarpSignature(AuId systemId, quint32 warpFactors, quint8 galaxy, float galaxyLocX, float galaxyLocY,
			float galaxyLocZ);

	void TradeReadyToShip(AuId readyCapitolId, AuTradeAgreement ta);

	void TradeShipped(AuId shipperCapitolId, AuTradeAgreement ta);

	void WarpSignatureDetected(AuId systemId, quint32 signalStrength, quint32 detectingEmpireId);

	// 'industry' is an INDUSTRYID
	void EmpireChgBdPref(quint32 empireId, int industry, quint32 designId);

	// 'scrole' is an AuScPrefs::SCROLE
	void EmpireChgScPref(quint32 empireId, int scrole, quint32 designId);

	void EmpireChgFlag(quint32 empireId, QImage flag);

	void EmpireChgName(quint32 empireId, QString name);

	void EmpireChgRole(quint32 empireId, quint8 gameRole);

	void EmpireCityWorkerSpeciesChanged(quint32 empireId, AuId cityId, AuSpecies workerSpecies);

	void error(QTcpSocket::SocketError socketError);

	// emitted when QThread emits finished or terminated
	void Finished(unsigned int serverId);

	void GovBossChanged(quint32 empireId, AuId governorId, AuId bossId);

	void GovCitySurrendered(AuId byWho, AuId cityId, quint32 fromEmpire, quint32 toEmpire);

	void GovDossierAddDocument(quint32 empire, quint32 dosEmpire, QByteArray doc);

	void GovDossierChangeState(quint32 empire, quint32 dosEmpire, AuTime docTime, quint8 docType, quint16 docIndex,
			quint8 docState);

	void GovSetAnnouncements(quint32 empire, QString announcements);

	// 'capitalType' 0=sector, 1=empire, 3=galaxy
	void GovSetCapital(quint32 empire, AuId city, AuSpecies workerSpecies, AuId domain, int capitalType);
	void GovSetCommerce(quint32 empire, QByteArray commerce);
	void GovSetContrailColor(quint32 empire, quint8 hue, quint8 sat);
	void GovSetDefaultPoliticalStance(quint32 empire, quint8 stance);
	void GovSetJoinPassword(quint32 empire, QByteArray passCrypt);

	void GovSetTax(quint32 empire, quint8 income, quint8 sales, quint8 subsidy);

	void GovSetThemeColor(quint32 empire, quint8 theme, QColor color);

	void GovSetWarCriteria(quint32 empire, AuWarCriteria crit);

	void HandoffElsewhere(AuId person, AuId dest, quint8 rideAlso, bool limboRescue, QByteArray atomData, AuId returnOnFail);

	void LeaveEmpire(AuId personId, QString personName, quint32 idOldEmpire);

	void NameSpecies(AuSpecies species, QString specieName, AuId discoverer);

	void EstablishNewEmpire(AuId personId, const QString& personName, quint8 personGender, quint32 idOldEmpire, const QString& name,
			const QImage& imFlag);

	void OfficeAssigned(AuId person, quint32 empire, AuOffice office);

	void OfficeRevoked(AuId person, quint32 empire);

	void OfficerKilled(const AuId homeWorldId);

	void AvatarOnLine(AuId personId, quint32 empireId);

	void PostJobOffer(AuJobOffer jobOffer);

	void ReleaseBerth(AuId personId, AuId shipId);

	void ReloadStory(quint32 storyInstance);

	void RemoveStory(quint32 storyInstance);
	
	void RenameAvatar(AuId avatarId, QString nuName);

	void RenameCity(AuId cityId, QString nuName);

	void RenamePlanet(AuId planetId, QString nuName, AuId systemId, QString systemName, AuId sectorId, QString sectorName);

	void RenameSector(AuId sectorId, QString nuName);

	void RenameSystem(AuId systemId, QString nuName);

	void RenameWormhole(AuId systemId, AuId toSystem, qint8 polarity, quint32 empire, QString nuName);

	//void SectorVisited(AuId sectorId, quint32 empireId);

	// 'fromGalaxy' is a GALAXY value.
	void SendFunds(AuId fromId, QString fromName, AuId toId, double amount, quint8 fromGalaxy, quint8 reason, quint32 designId);

	void SolarSystemFinal(AuId systemId);
	void SolarSystemReadyToDecay(AuId systemId);
	//void SpawnPersonElsewhere(AuId person, quint32 empire, AuId spawnCity, AuId spawnShip, QByteArray atomData);
	//void SpawnShipElsewhere(AuId ship, quint32 empire, AuId spawnCity, QByteArray atomData);

	void SpacecraftNameChanged(AuId shipId, QString nuName);

	void StargateClosed(AuId fromGalaxyId, AuPoint fromGalaxyLoc);

	void StargateOpened(AuId fromGalaxyId, AuPoint fromGalaxyLoc, AuId toGalaxyId, AuPoint toGalaxyLoc, AuTime expire);

	void StoryChanged(quint32 storyInstance);
	void StoryDeleted(quint32 storyInstance);

	void SummonDormantShip(AuId shipId, AuId crewId);

	void SystemClientStatusUpdate(QByteArray systemClients);

	void SystemExplored(AuId systemId, quint32 empireId);

	void SystemScanned(AuId systemId, quint32 empireId, quint8 genHab, quint32 genSeed);

	void SystemNameChanged(AuId systemId, QString systemName, AuId sectorId, QString sectorName);

	//void TravelElsewhere(AuId id, AuId destSystem, AuId destSector, QByteArray atomData);
	void WormholeExploredSignal(AuId systemId, AuId toSystem, qint8 polarity, quint32 empire, QString nuName);

	void CreditCityBank(float fAmount, AuId cityId);
	void SetPoliticalStance(quint32 idToEmpire, quint32 idFromEmpire, quint8 stance, quint8 vassalTributePercent,
			AuId fromUnit, QString fromUnitName, quint8 toStance, AuId toUnit, QString toUnitName,
			quint8 reason, quint8 galaxy, QString sector, QString system, QString world);

protected:
	virtual void run();

private:
	// Load expressed as a number for comparison against a peer server's load.
	// Returning 0xffffffff indicates the server is not ready for assignments.
	// Lock m_SubStatusLock before calling.
	quint32 CurrentSubLoad() const;

	void DeferSceneWorker();
	void DoleAtomIds();
	void DoleBuildingDesignIds();
	void DoleItemDesignIds();
	void DoleSpaceShipDesignIds();
	void DoleStoryIds();
	void ReadPacket(const QByteArray& ba, QTcpSocket& tcpSocket);
	void RxData(QTcpSocket& tcpSocket);

private:
	unsigned int	m_Id;				// Id of this server instance.
	QVector<AuAtomQuery>	m_AtomQueries;	// Used by QueryAtomPresence
	QVector<AuHengeTeleportDestQuery>	m_HengeTeleporterQueries;	// Used by QueryHengeTeleporterPresence
	mutable QMutex	m_AtomQueryMutex;

	unsigned int	m_BadPackets;		// # of packets rx'd with unknown packet
										// type
	unsigned int	m_BytesRx;			// Total bytes received from client
	unsigned int	m_BytesTx;			// Total bytes sent to client
	quint32			m_PacketSize;		// Data packet size
	int				m_PacketSizeBytesLeft;	// Number of packet size bytes we are expecting.
	QHash<quint32, quint32>	m_PacketsRx;	// Count of packets received, keyed by packet type.
	QHostAddress	m_peerAddress;
	int				m_socketDescriptor;	// Provided in the constructor.
	AuTime			m_tmConnect;		// Time connection is established.
	AuTime			m_tmDisconnect;		// Null until time to disconnect.
	QMutex			m_muTxBuffer;		// TCP tx buffer mutex
	QByteArray		m_TxBuffer;			// TCP tx buffer

	quint16			m_DoleAtomIds;				// number of atom ids requested from me
	quint16			m_DoleBuildingDesignIds;	// number of building design ids requested from me
	quint16			m_DoleItemDesignIds;		// number of item design ids requested from me
	quint16			m_DoleSpaceShipDesignIds;	// number of spacecraft design ids requested from me
	quint16			m_DoleStoryIds;				// number of story ids requested from me

	QSemaphore		m_semAlive;			// Used to know when I'm supposed to die.

	// Information provided by P_Status packets from the subordinate server to which I am connected.
	mutable QReadWriteLock				m_SubStatusLock;		// Guards sub server status variables updated by P_Status.
	float								m_SubAtomLoad;			// Atom load on sub worker server.
	quint16								m_SubAtomMgrs;			// number of atom managers reported by a worker to its manager
	AuTime								m_SubCheckInTime;		// checkin time
	QString								m_SubNetName;			// sub server name for internet client access
	quint16								m_SubNetPort;			// sub server port for internet client access
	AuPlayerOnlineStatusVector			m_SubPlayers;			// status about all players attached to subordinate worker(s)
	quint8								m_SubServerMode;		// subordinate server mode: 0=Master, 1=Manager, 2=Worker
	unordered_map<AuId, AuSystemStatus, AuIdHash> 		m_SubSystemClients;		// Status of individual systems assigned to system clients
	quint32								m_SubTickTime;			// tick time. 0xffffffff until at least one P_Status received.
	quint16								m_SubWorkerCores;		// number of scene worker processor cores attached to manager
	quint16								m_SubWorkers;			// number of scene workers attached to manager
};

#endif // DtServerChannel_H
