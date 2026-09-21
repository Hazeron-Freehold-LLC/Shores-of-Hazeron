//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auserverchannel.cpp,v 1.238 2022/02/19 18:39:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auserverchannel.h"
//
#include "auplayeronlinestatus.h"
#include "ausceneserverapp.h"
#include "dtserverchannel.h"
#include "dttick.h"

AuServerChannel::AuServerChannel(QObject* parent) : QTcpServer(parent)
{
	m_NextId = 0;
}

void AuServerChannel::incomingConnection(qintptr socketDescriptor)
{
	//qDebug("AuServerChannel::incomingConnection");

	// Create a thread to handle the client connection.
	m_NextId++;
	while(!m_NextId || m_NextId == 1)
		m_NextId++;	// skip zero and one
	DtServerChannel* tserver = new DtServerChannel(m_NextId, socketDescriptor, this);

	connect(tserver, SIGNAL(AdminSummon(AuId, AuId, quint8)),
			auSceneServerApp, SLOT(AdminSummon(AuId, AuId, quint8)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(ArenaReady(AuArenaReady)),
			auSceneServerApp, SLOT(ArenaReady(AuArenaReady)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(ArenaReadyDownSignal(AuArenaReady)),
			tserver, SLOT(ArenaReadyDown(AuArenaReady)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(AtomHengeTeleporting(AuHengeTeleportAtom)),
			auSceneServerApp, SLOT(HengeTeleportAtomUp(AuHengeTeleportAtom)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(AtomPresence(AuId, unsigned int, int, unsigned int)),
			auSceneServerApp, SLOT(AtomPresence(AuId, unsigned int, int, unsigned int)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(CollectBounty(AuId, QString, AuGearPos, AuBountyOnItem)),
			auSceneServerApp, SLOT(CollectBountyUp(AuId, QString, AuGearPos, AuBountyOnItem)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(PayBounty(AuId, AuGearPos, AuBountyOnItem, bool)),
			auSceneServerApp, SLOT(PayBountyUp(AuId, AuGearPos, AuBountyOnItem, bool)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(HengeTeleporterPresence(AuId, int, unsigned int)),
			auSceneServerApp, SLOT(HengeTeleporterPresence(AuId, int, unsigned int)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(AtomSpawning(AuId, quint32, AuId, AuId, AuId, quint8)),
			auSceneServerApp, SLOT(SpawnAtomUp(AuId, quint32, AuId, AuId, AuId, quint8)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(AtomTraveling(AuId, AuId, AuId)),
			auSceneServerApp, SLOT(AtomTraveledUp(AuId, AuId, AuId)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(AtomTravelingGalaxyLoc(AuId, AuId, AuPoint)),
			auSceneServerApp, SLOT(AtomTraveledGalaxyLocUp(AuId, AuId, AuPoint)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(AtomTravelingStargate(AuId, AuId, AuPoint)),
			auSceneServerApp, SLOT(AtomTraveledStargateUp(AuId, AuId, AuPoint)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(BlueprintRatedBd(AuExchangeRatingRec)),
			auSceneServerApp, SLOT(BlueprintRatedBdUp(AuExchangeRatingRec)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(BlueprintRatedSc(AuExchangeRatingRec)),
			auSceneServerApp, SLOT(BlueprintRatedScUp(AuExchangeRatingRec)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(BountyPosted(AuId, QString, AuId, QString, double, int)),
			auSceneServerApp, SLOT(BountyPosted(AuId, QString, AuId, QString, double, int)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(CityFacilitiesChanged(AuId, AuCityIndustryHash)),
			auSceneServerApp, SLOT(CityFacilitiesChanged(AuId, AuCityIndustryHash)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(CityStatusChanged(AuId, int, qint8)),
			auSceneServerApp, SLOT(CityStatusChanged(AuId, int, qint8)),
			Qt::QueuedConnection);

	// City mail report request works its way up the server farm through here.
	connect(tserver, SIGNAL(CityMailRequestedSignal(AuId, AuId, quint32)),
			auSceneServerApp, SLOT(CityMailRequested(AuId, AuId, quint32)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(EmpireRecordPatent(quint32, quint16, quint8)),
			auSceneServerApp, SLOT(EmpireRecordPatent(quint32, quint16, quint8)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(HengeActivated(AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
			auSceneServerApp, SLOT(HengeActivated(AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(HengeCreated(AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
			auSceneServerApp, SLOT(HengeCreated(AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(HengeDeactivated(AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
			auSceneServerApp, SLOT(HengeDeactivated(AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(HengeDestinationConfirmed(AuId, AuId, QString, quint8, QString, QString, quint8, AuGlobeLLF,
					bool, quint8, bool)),
			auSceneServerApp, SLOT(HengeDestinationConfirmed(AuId, AuId, QString, quint8, QString, QString, quint8, AuGlobeLLF,
					bool, quint8, bool)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(HengeDestinationDownSignal(AuId, AuId, QString, quint8, QString, QString, quint8, AuGlobeLLF,
					bool, quint8, bool)),
			tserver, SLOT(HengeDestinationDown(AuId, AuId, QString, quint8, QString, QString, quint8, AuGlobeLLF,
					bool, quint8, bool)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(AvatarAccountActiveSignal(AuIdList)),
			auSceneServerApp, SLOT(AvatarAccountActive(AuIdList)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(AvatarAccountActiveSignal(AuIdList)),
			tserver, SLOT(AvatarAccountActive(AuIdList)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(ClaimTitleSignal(AuId, QString, AuId, QString, quint32)),
			auSceneServerApp, SLOT(ClaimTitleUp(AuId, QString, AuId, QString, quint32)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(ClaimTitleSignal(AuId, QString, AuId, QString, quint32)),
			tserver, SLOT(ClaimTitle(AuId, QString, AuId, QString, quint32)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(ScDesignDeactivatedSignal(quint32)),
			tserver, SLOT(ScDesignDeactivated(quint32)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(ScDesignDeactivatedSignal(quint32)),
			auSceneServerApp, SLOT(ScDesignDeactivatedUp(quint32)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(ScDesignExchangePublishedSignal(AuScDesignExchangeEntry)),
			tserver, SLOT(ScDesignExchangePublished(AuScDesignExchangeEntry)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(ScDesignExchangePublishedSignal(AuScDesignExchangeEntry)),
			auSceneServerApp, SLOT(ScDesignExchangePublishedUp(AuScDesignExchangeEntry)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(ScDesignExchangeUnpublishedSignal(quint32)),
			tserver, SLOT(ScDesignExchangeUnpublished(quint32)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(ScDesignExchangeUnpublishedSignal(quint32)),
			auSceneServerApp, SLOT(ScDesignExchangeUnpublishedUp(quint32)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(ItDesignDeactivatedSignal(quint32)),
			tserver, SLOT(ItDesignDeactivated(quint32)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(ItDesignDeactivatedSignal(quint32)),
			auSceneServerApp, SLOT(ItDesignDeactivatedUp(quint32)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(BdDesignDeactivatedSignal(quint32)),
			tserver, SLOT(BdDesignDeactivated(quint32)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(BdDesignDeactivatedSignal(quint32)),
			auSceneServerApp, SLOT(BdDesignDeactivatedUp(quint32)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(BdDesignExchangePublishedSignal(AuBdDesignExchangeEntry)),
			tserver, SLOT(BdDesignExchangePublished(AuBdDesignExchangeEntry)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(BdDesignExchangePublishedSignal(AuBdDesignExchangeEntry)),
			auSceneServerApp, SLOT(BdDesignExchangePublishedUp(AuBdDesignExchangeEntry)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(BdDesignExchangeUnpublishedSignal(quint32)),
			tserver, SLOT(BdDesignExchangeUnpublished(quint32)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(BdDesignExchangeUnpublishedSignal(quint32)),
			auSceneServerApp, SLOT(BdDesignExchangeUnpublishedUp(quint32)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(SectorPirateBaseEncounter(AuId, AuId)),
			auSceneServerApp, SLOT(SectorPirateBaseEncounter(AuId, AuId)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(AcceptTrade(AuTradeAgreement)),
			auSceneServerApp, SLOT(AcceptTrade(AuTradeAgreement)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(DeactivateTrade(AuTradeAgreement, quint8, bool)),
			auSceneServerApp, SLOT(DeactivateTrade(AuTradeAgreement, quint8, bool)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(TradeReadyToShip(AuId, AuTradeAgreement)),
			auSceneServerApp, SLOT(TradeReadyToShip(AuId, AuTradeAgreement)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(TradeShipped(AuId, AuTradeAgreement)),
			auSceneServerApp, SLOT(TradeShipped(AuId, AuTradeAgreement)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(ProposeTrade(AuTradeAgreement)),
			auSceneServerApp, SLOT(ProposeTrade(AuTradeAgreement)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(ShipIt(AuItemTEUContainer)),
			auSceneServerApp, SLOT(ShipIt(AuItemTEUContainer)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(SpacecraftStasisRecover(AuId, AuId, quint8, bool)),
			auSceneServerApp, SLOT(SpacecraftStasisRecoverUp(AuId, AuId, quint8, bool)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(SystemWarpSignature(AuId, quint32, quint8, float, float, float)),
			auSceneServerApp, SLOT(SystemWarpSignature(AuId, quint32, quint8, float, float, float)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(WarpSignatureDetected(AuId, quint32, quint32)),
			auSceneServerApp, SLOT(WarpSignatureDetected(AuId, quint32, quint32)),
			Qt::QueuedConnection);

	// On the way up to the boss.
	connect(tserver, SIGNAL(SpacecraftNameChanged(AuId, QString)),
			auSceneServerApp, SLOT(SpacecraftNameChanged(AuId, QString)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(SpacecraftPositionReport(AuId, AuId, AuStarMapSpacecraft)),
			auSceneServerApp, SLOT(SpacecraftPositionReport(AuId, AuId, AuStarMapSpacecraft)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(StargateClosed(AuId, AuPoint)),
			auSceneServerApp, SLOT(StargateClosed(AuId, AuPoint)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(StargateOpened(AuId, AuPoint, AuId, AuPoint, AuTime)),
			auSceneServerApp, SLOT(StargateOpened(AuId, AuPoint, AuId, AuPoint, AuTime)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(ChangeEmpireInDb(quint32)),
			auSceneServerApp, SIGNAL(ChangeEmpireInDb(quint32)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(EmpireChgBdPref(quint32, int, quint32)),
			auSceneServerApp, SLOT(EmpireChgBdPrefUp(quint32, int, quint32)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(EmpireChgScPref(quint32, int, quint32)),
			auSceneServerApp, SLOT(EmpireChgScPrefUp(quint32, int, quint32)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(EmpireChgFlag(quint32, QImage)),
			auSceneServerApp, SLOT(EmpireChgFlagUp(quint32, QImage)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(EmpireChgName(quint32, QString)),
			auSceneServerApp, SLOT(EmpireChgNameUp(quint32, QString)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(EmpireChgRole(quint32, quint8)),
			auSceneServerApp, SLOT(EmpireChgRoleUp(quint32, quint8)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(EmpireCityWorkerSpeciesChanged(quint32, AuId, AuSpecies)),
			auSceneServerApp, SLOT(EmpireCityWorkerSpeciesChanged(quint32, AuId, AuSpecies)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(Finished(unsigned int)),
			SLOT(ServerThreadFinished(unsigned int)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(HandoffElsewhere(AuId, AuId, quint8, bool, QByteArray, AuId)),
			auSceneServerApp, SLOT(HandoffAtomUp(AuId, AuId, quint8, bool, QByteArray, AuId)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(OfficeAssigned(AuId, quint32, AuOffice)),
			auSceneServerApp, SLOT(OfficeAssigned(AuId, quint32, AuOffice)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(OfficeRevoked(AuId, quint32)),
			auSceneServerApp, SLOT(OfficeRevoked(AuId, quint32)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(OfficerKilled(const AuId)),
			auSceneServerApp, SLOT(OfficerKilledUp(const AuId)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(AvatarOnLine(AuId, quint32)),
			auSceneServerApp, SLOT(AvatarOnLineUp(AuId, quint32)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(PostJobOffer(AuJobOffer)),
			auSceneServerApp, SLOT(PostJobOffer(AuJobOffer)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(ReloadStory(quint32)),
			auSceneServerApp, SLOT(StoryRecordChangedDown(quint32)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(RemoveStory(quint32)),
			auSceneServerApp, SLOT(StoryRecordDeletedDown(quint32)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(RenameAvatar(AuId, QString)),
			auSceneServerApp, SLOT(RenameAvatarUp(AuId, QString)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(RenameCity(AuId, QString)),
			auSceneServerApp, SLOT(RenameCityUp(AuId, QString)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(RenamePlanet(AuId, QString, AuId, QString, AuId, QString)),
			auSceneServerApp, SLOT(RenamePlanetUp(AuId, QString, AuId, QString, AuId, QString)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(RenameSector(AuId, QString)),
			auSceneServerApp, SLOT(RenameSectorUp(AuId, QString)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(RenameSystem(AuId, QString)),
			auSceneServerApp, SLOT(RenameSystemUp(AuId, QString)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(RenameWormhole(AuId, AuId, qint8, quint32, QString)),
			auSceneServerApp, SLOT(RenameWormhole(AuId, AuId, qint8, quint32, QString)),
			Qt::QueuedConnection);

	//connect(tserver, SIGNAL(SectorVisited(AuId, quint32)),
			//auSceneServerApp, SLOT(SectorVisitedUp(AuId, quint32)),
			//Qt::QueuedConnection);

	connect(tserver, SIGNAL(SendFunds(AuId, QString, AuId, double, quint8, quint8, quint32)),
			auSceneServerApp, SLOT(SendFundsUp(AuId, QString, AuId, double, quint8, quint8, quint32)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(SolarSystemFinal(AuId)),
			auSceneServerApp, SLOT(SolarSystemFinal(AuId)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(SolarSystemReadyToDecay(AuId)),
			auSceneServerApp, SLOT(SolarSystemReadyToDecay(AuId)),
			Qt::QueuedConnection);

	//connect(tserver, SIGNAL(SpawnPersonElsewhere(AuId, quint32, AuId, AuId, QByteArray)),
			//auSceneServerApp, SLOT(SpawnPersonUp(AuId, quint32, AuId, AuId, QByteArray)),
			//Qt::QueuedConnection);

	//connect(tserver, SIGNAL(SpawnShipElsewhere(AuId, quint32, AuId, QByteArray)),
			//auSceneServerApp, SLOT(SpawnShipUp(AuId, quint32, AuId, QByteArray)),
			//Qt::QueuedConnection);

	connect(tserver, SIGNAL(StoryChanged(quint32)),
			auSceneServerApp, SLOT(StoryRecordChangedUp(quint32)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(StoryDeleted(quint32)),
			auSceneServerApp, SLOT(StoryRecordDeletedUp(quint32)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(SummonDormantShip(AuId, AuId)),
			auSceneServerApp, SLOT(SummonDormantShip(AuId, AuId)),
			Qt::QueuedConnection);

	//connect(tserver, SIGNAL(TravelElsewhere(AuId, AuId, AuId, QByteArray)),
			//auSceneServerApp, SLOT(TransferAtomUp(AuId, AuId, AuId, QByteArray)),
			//Qt::QueuedConnection);

	connect(tserver, SIGNAL(WormholeExploredSignal(AuId, AuId, qint8, quint32, QString)),
			auSceneServerApp, SLOT(WormholeExplored(AuId, AuId, qint8, quint32, QString)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(GovBossChanged(quint32, AuId, AuId)),
			auSceneServerApp, SLOT(GovBossChanged(quint32, AuId, AuId)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(GovCitySurrendered(AuId, AuId, quint32, quint32)),
			auSceneServerApp, SLOT(GovCitySurrenderedUp(AuId, AuId, quint32, quint32)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(GovDossierAddDocument(quint32, quint32, QByteArray)),
			auSceneServerApp, SLOT(GovDossierAddDocument(quint32, quint32, QByteArray)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(GovDossierChangeState(quint32, quint32, AuTime, quint8, quint16, quint8)),
			auSceneServerApp, SLOT(GovDossierChangeStateUp(quint32, quint32, AuTime, quint8, quint16, quint8)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(GovSetAnnouncements(quint32, QString)),
			auSceneServerApp, SLOT(GovSetAnnouncementsUp(quint32, QString)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(GovSetWarCriteria(quint32, AuWarCriteria)),
			auSceneServerApp, SLOT(GovSetWarCriteria(quint32, AuWarCriteria)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(GovSetCapital(quint32, AuId, AuSpecies, AuId, int)),
			auSceneServerApp, SLOT(GovSetCapitalUp(quint32, AuId, AuSpecies, AuId, int)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(GovSetJoinPassword(quint32, QByteArray)),
			auSceneServerApp, SLOT(GovSetJoinPasswordUp(quint32, QByteArray)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(GovSetCommerce(quint32, QByteArray)),
			auSceneServerApp, SLOT(GovSetCommerceUp(quint32, QByteArray)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(GovSetContrailColor(quint32, quint8, quint8)),
			auSceneServerApp, SLOT(GovSetContrailColorUp(quint32, quint8, quint8)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(GovSetDefaultPoliticalStance(quint32, quint8)),
			auSceneServerApp, SLOT(GovSetDefaultPoliticalStanceUp(quint32, quint8)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(GovSetTax(quint32, quint8, quint8, quint8)),
			auSceneServerApp, SLOT(GovSetTaxUp(quint32, quint8, quint8, quint8)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(GovSetThemeColor(quint32, quint8, QColor)),
			auSceneServerApp, SLOT(GovSetThemeColorUp(quint32, quint8, QColor)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(LeaveEmpire(AuId, QString, quint32)),
			auSceneServerApp, SLOT(LeaveEmpire(AuId, QString, quint32)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(NameSpecies(AuSpecies, QString, AuId)),
			auSceneServerApp, SLOT(NameSpeciesUp(AuSpecies, QString, AuId)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(EstablishNewEmpire(AuId, QString, quint8, quint32, QString, QImage)),
			auSceneServerApp, SLOT(EstablishNewEmpire(AuId, QString, quint8, quint32, QString, QImage)),
			Qt::QueuedConnection);

	connect(tserver,
			SIGNAL(CityCreated(quint32, quint8, AuId, QString, AuId, QString, AuId, QString, AuId, QString, AuPoint, AuId, QString,
					AuId, quint8)),
			auSceneServerApp,
			SLOT(CityCreated(quint32, quint8, AuId, QString, AuId, QString, AuId, QString, AuId, QString, AuPoint, AuId, QString,
					AuId, quint8)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(CityCaptured(quint32, quint32, AuId)),
			auSceneServerApp, SLOT(CityCaptured(quint32, quint32, AuId)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(CityDecayed(quint32, AuId)),
			auSceneServerApp, SLOT(CityDecayed(quint32, AuId)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(AddCitizenToEmpire(quint32, quint32, AuId, QString, quint8)),
			auSceneServerApp, SLOT(AddCitizenToEmpire(quint32, quint32, AuId, QString, quint8)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(ReleaseBerth(AuId, AuId)),
			auSceneServerApp, SLOT(ReleaseBerthUp(AuId, AuId)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(CreditCityBank(float, AuId)),
			auSceneServerApp, SLOT(CreditCityBank(float, AuId)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(SetPoliticalStance(quint32, quint32, quint8, quint8, AuId, QString, quint8, AuId, QString,
					                    quint8, quint8, QString, QString, QString)),
		auSceneServerApp, SLOT(SetPoliticalStance(quint32, quint32, quint8, quint8, AuId, QString, quint8, AuId, QString,
				                    quint8, quint8, QString, QString, QString)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(SystemExplored(AuId, quint32)),
			auSceneServerApp, SLOT(SolarSystemExplored(AuId, quint32)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(SystemScanned(AuId, quint32, quint8, quint32)),
			auSceneServerApp, SLOT(SolarSystemScanned(AuId, quint32, quint8, quint32)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(SystemNameChanged(AuId, QString, AuId, QString)),
			auSceneServerApp, SLOT(SystemNameChangedUp(AuId, QString, AuId, QString)),
			Qt::QueuedConnection);

	// This enables a single signal emitted by the app to propagate to all client servers via Qt's signal/slot mechanism.
	connect(auSceneServerApp, SIGNAL(BroadcastBountyPosted(AuId, QString, AuId, QString, double, int)),
			tserver, SLOT(SendBountyPosted(AuId, QString, AuId, QString, double, int)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(BroadcastJobOffer(AuJobOffer)),
			tserver, SLOT(SendJobOffer(AuJobOffer)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(CityMailRequestedSignal(AuId, AuId, quint32)),
			tserver, SLOT(CityMailRequested(AuId, AuId, quint32)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(EmpireDoneChanging(DgEmpire)),
			tserver, SLOT(EmpireChangedDown(DgEmpire)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(PirateBaseEncountered(AuId)),
			tserver, SLOT(PirateBaseEncountered(AuId)),
			Qt::QueuedConnection);

	// On the way down to the subs.
	connect(auSceneServerApp, SIGNAL(BlueprintRatedBdSignal(AuExchangeRatingRec)),
			tserver, SLOT(BlueprintRatedBdDown(AuExchangeRatingRec)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(BlueprintRatedScSignal(AuExchangeRatingRec)),
			tserver, SLOT(BlueprintRatedScDown(AuExchangeRatingRec)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(CollectBountySignal(AuId, QString, AuGearPos, AuBountyOnItem)),
			tserver, SLOT(CollectBountyDown(AuId, QString, AuGearPos, AuBountyOnItem)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(PayBountySignal(AuId, AuGearPos, AuBountyOnItem, bool)),
			tserver, SLOT(PayBountyDown(AuId, AuGearPos, AuBountyOnItem, bool)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(CloseStargateDownSignal(AuId, AuPoint)),
			tserver, SLOT(CloseStargateDown(AuId, AuPoint)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(OpenStargateDownSignal(AuId, AuPoint, AuId, AuPoint, AuId, AuId, AuTime)),
			tserver, SLOT(OpenStargateDown(AuId, AuPoint, AuId, AuPoint, AuId, AuId, AuTime)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(PersonDeleted(AuId)),
			tserver, SLOT(PersonDeleted(AuId)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(QueryAtomServerDown(AuId, quint32, AuId)),
			tserver, SLOT(QueryAtomPresence(AuId, quint32, AuId)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(QueryHengeTeleporterServerDown(AuHengeTeleportAtom)),
			tserver, SLOT(QueryHengeTeleporterPresence(AuHengeTeleportAtom)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(RenamePlanet(AuId, QString, AuId, QString, AuId, QString)),
			tserver, SLOT(RenamePlanetDown(AuId, QString, AuId, QString, AuId, QString)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(RenameSector(AuId, QString)),
			tserver, SLOT(RenameSectorDown(AuId, QString)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(RenameSystem(AuId, QString)),
			tserver, SLOT(RenameSystemDown(AuId, QString)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(SectorDecayed(AuId)),
			tserver, SLOT(SectorDecayed(AuId)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(PlayersOnlineStatusSignal(AuPlayerOnlineStatusVector)),
			tserver, SLOT(SendPlayerStatus(AuPlayerOnlineStatusVector)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(SpacecraftNameChangedSignal(AuId, QString)),
			tserver, SLOT(SpacecraftNameChangedDown(AuId, QString)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(SpacecraftStasisRecoverSignal(AuId, AuId, quint8, bool)),
			tserver, SLOT(SpacecraftStasisRecoverDown(AuId, AuId, quint8, bool)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(SpeciesNameChanged(AuSpecies, QString, AuId)),
			tserver, SLOT(SpeciesNameChanged(AuSpecies, QString, AuId)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(SystemWarpSignatureDown(AuId, quint32, quint8, float, float, float)),
			tserver, SLOT(SystemWarpSignatureBroadcast(AuId, quint32, quint8, float, float, float)),
			Qt::QueuedConnection);

	connect(auSceneServerApp, SIGNAL(WormholeExploredSignal(AuId, AuId, qint8, quint32, QString)),
			tserver, SLOT(WormholeExplored(AuId, AuId, qint8, quint32, QString)),
			Qt::QueuedConnection);

	// Start the thread.
	tserver->start();

	// Instead, let's keep track of these server threads so we can monitor their status.
	m_ServersLock.lockForWrite();
	m_Servers.insert(pair<unsigned int, DtServerChannel*>(tserver->Id(), tserver));
	m_ServersLock.unlock();
}

void AuServerChannel::ServerThreadFinished(unsigned int serverId)
{
	//qDebug() << "AuServerChannel::ServerThreadFinished id" << serverId;
	m_ServersLock.lockForWrite();
	unordered_map<unsigned int, DtServerChannel*>::iterator it = m_Servers.find(serverId);
	if(it != m_Servers.end())
	{
		// We found the server thread that has finished.
		//qDebug() << "AuServerChannel::ServerThreadFinished found server id" << (*it).second->Id();
		emit ConnectionClosed((*it).second->Id());
		(*it).second->deleteLater();
		m_Servers.erase(it);
	}
	m_ServersLock.unlock();
}

DtServerChannel* AuServerChannel::ServerChannel(unsigned int id) const
{
	// Find the named subordinate server.
	unordered_map<unsigned int, DtServerChannel*>::const_iterator it = m_Servers.find(id);
	if(it != m_Servers.cend())
		return (*it).second;
	return 0;
}

// Returns peer address of least loaded subordinate server.
// mode=1 selects sub manager servers
// mode=2 selects sub worker servers
// which=0 selects the least loaded
// which=1 selects the second least loaded
// which=2 selects the third least loaded
// which=n selects the n'th least loaded
// Selection wraps around if which is >= the number of sub servers
DtServerChannel* AuServerChannel::LeastLoadedServer(int mode, int which, unsigned int avoidServerId)
{
	// If any subs are not ready, return 0 so we will wait until all connected subs become ready. This is
	// preferable to assigning everything to the first sub that becomes ready.
	for(unordered_map<unsigned int, DtServerChannel*>::iterator it = m_Servers.begin(); it != m_Servers.end(); it++)
		if((*it).second->CurrentLoad() == 0xffffffff && (*it).second->ServerMode() == mode)
			return 0;

	// Make a list of the subs that are ready.
	DtServerChannel* avoidSub = 0;
	QMultiMap<float, DtServerChannel*> readySubs;	// Map sorts by key, which is server load.
	for(unordered_map<unsigned int, DtServerChannel*>::iterator it = m_Servers.begin(); it != m_Servers.end(); it++)
		if((*it).second->CurrentLoad() != 0xffffffff && (*it).second->ServerMode() == mode)
		{
			if((*it).second->Id() == avoidServerId)
				avoidSub = (*it).second;
			else
				readySubs.insert((*it).second->AtomLoad(), (*it).second);
		}

	// We can't avoid the server if its the only sub that is ready.
	if(readySubs.isEmpty())
		return avoidSub;

	int assignTo = which % readySubs.count();	// wrap among ready subs using modulus
	QMultiMap<float, DtServerChannel*>::iterator si = readySubs.begin() + assignTo;
	return *si;
}

// Returns peer address of most loaded subordinate server.
// Manager server does not return servers that are assigned less than or equal to their number of sub worker CPUs/cores.
DtServerChannel* AuServerChannel::MostLoadedServer()
{
	DtServerChannel* mostLoadedChannel = 0;
	float mostLoad = 0;
	for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = m_Servers.cbegin(); it != m_Servers.cend(); it++)
	{
		quint32 load = (*it).second->CurrentLoad();
		if(load == 0xffffffff)
			continue;		// Sub server is not ready.
		if(g_Au.IsSceneManager() && !(*it).second->HasExcessLoad())
			continue;		// Skip sub worker server that does not have anything to revoke.
		float atomLoad = (*it).second->AtomLoad();
		if(!mostLoadedChannel || atomLoad > mostLoad)
		{
			mostLoadedChannel = (*it).second;
			mostLoad = atomLoad;
		}
	}
	return mostLoadedChannel;
}

int AuServerChannel::CountManagerServers() const
{
	int count = 0;
	m_ServersLock.lockForRead();
	for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = m_Servers.cbegin(); it != m_Servers.cend(); it++)
		if((*it).second->IsSceneManager())
			count++;
	m_ServersLock.unlock();
	return count;
}

int AuServerChannel::CountWorkerServers() const
{
	int count = 0;
	m_ServersLock.lockForRead();
	for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = m_Servers.cbegin(); it != m_Servers.cend(); it++)
		if((*it).second->IsSceneWorker())
			count++;
	m_ServersLock.unlock();
	return count;
}

int AuServerChannel::PlayerCount() const
{
	int count = 0;
	m_ServersLock.lockForRead();
	for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = m_Servers.cbegin(); it != m_Servers.cend(); it++)
		count += (*it).second->SubPlayers();
	m_ServersLock.unlock();
	return count;
}

int AuServerChannel::WorkerCount() const
{
	if(g_Au.IsSceneMaster())
	{
		// Return count of workers tallied from manager's status updates.
		int count = 0;
		m_ServersLock.lockForRead();
		for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = m_Servers.cbegin(); it != m_Servers.cend(); it++)
			count += (*it).second->SubWorkers();
		m_ServersLock.unlock();
		return count;
	}
	else if(g_Au.IsSceneManager())
	{
		// Return count of manager's server channel connections. They are workers.
		m_ServersLock.lockForRead();
		int count = m_Servers.size();
		m_ServersLock.unlock();
		return count;
	}
	else
		return 0;						// Scene worker has no workers.
}

int AuServerChannel::WorkerCoreCount() const
{
	int count = 0;
	m_ServersLock.lockForRead();
	for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = m_Servers.cbegin(); it != m_Servers.cend(); it++)
		count += (*it).second->SubWorkerCores();
	m_ServersLock.unlock();
	return count;
}

void AuServerChannel::PlayersOnlineStatus(AuPlayerOnlineStatusVector& players) const
{
	players.clear();
	m_ServersLock.lockForRead();
	for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = m_Servers.cbegin(); it != m_Servers.cend(); it++)
		(*it).second->PlayersOnlineStatus(players);
	m_ServersLock.unlock();
}

void AuServerChannel::StopThreads()
{
	m_ServersLock.lockForWrite();
	for(unordered_map<unsigned int, DtServerChannel*>::iterator it = m_Servers.begin(); it != m_Servers.end(); it++)
		(*it).second->Kill();

	// Wait up to 10 seconds for our server channels to shut down.
	bool anyAlive;
	QTime t;
	do
	{
		anyAlive = false;
		for(unordered_map<unsigned int, DtServerChannel*>::iterator it = m_Servers.begin(); it != m_Servers.end(); it++)
			if((*it).second->isRunning())
			{
				anyAlive = true;
				break;
			}
	}
	while(anyAlive && t.elapsed() < 10000);

	m_Servers.clear();
	m_ServersLock.unlock();
}

DtServerChannel* AuServerChannel::FindServer(const QString& networkName, quint16 networkPort)
{
	for(unordered_map<unsigned int, DtServerChannel*>::iterator it = m_Servers.begin(); it != m_Servers.end(); it++)
		if(networkName.compare((*it).second->InternetName(), Qt::CaseInsensitive) == 0 &&
				networkPort == (*it).second->InternetPort())
			return (*it).second;
	return 0;
}
