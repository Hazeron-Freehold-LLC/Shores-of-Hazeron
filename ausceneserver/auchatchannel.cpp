//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auchatchannel.cpp,v 1.118 2022/02/19 18:39:35 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auchatchannel.h"
//
#include "auplayeronlinestatus.h"
#include "ausceneserverapp.h"
#include "dtchatchannel.h"

AuChatChannel::AuChatChannel(QObject* parent) : QTcpServer(parent)
{
}

void AuChatChannel::incomingConnection(qintptr socketDescriptor)
{
	//qDebug("AuChatChannel::incomingConnection socketDesc %d", socketDescriptor);

	// Create a thread to handle the client connection.
	DtChatChannel* tserver = new DtChatChannel(socketDescriptor, this);
	connect(tserver, SIGNAL(AdminGoTo(AuId, AuId, quint8, bool, AuId)),
			auSceneServerApp, SLOT(AdminGoTo(AuId, AuId, quint8, bool, AuId)));
	connect(tserver, SIGNAL(AdminGoToGalaxyLoc(AuId, AuId, AuId, AuPoint)),
			auSceneServerApp, SLOT(AtomTraveledGalaxyLoc(AuId, AuId, AuId, AuPoint)));
	connect(tserver, SIGNAL(AdminSummon(AuId, AuId, quint8)), auSceneServerApp, SLOT(AdminSummon(AuId, AuId, quint8)));
	connect(tserver, SIGNAL(BountyPosted(AuId, QString, AuId, QString, double, int)),
			auSceneServerApp, SLOT(BountyPosted(AuId, QString, AuId, QString, double, int)));
	connect(tserver, SIGNAL(ChatReceived(QByteArray)), auSceneServerApp, SLOT(ChatReceived(QByteArray)));

	connect(tserver, SIGNAL(CollectBounty(AuId, QString, AuGearPos, AuBountyOnItem)),
			auSceneServerApp, SLOT(CollectBounty(AuId, QString, AuGearPos, AuBountyOnItem)));

	connect(tserver, SIGNAL(BdDesignExchangePublished(AuBdDesignExchangeEntry)),
			auSceneServerApp, SLOT(BdDesignExchangePublishedUp(AuBdDesignExchangeEntry)));

	connect(tserver, SIGNAL(BdDesignExchangeUnpublished(quint32)),
			auSceneServerApp, SLOT(BdDesignExchangeUnpublishedUp(quint32)));

	connect(tserver, SIGNAL(BlueprintRatedBd(AuExchangeRatingRec)),
			auSceneServerApp, SLOT(BlueprintRatedBdUp(AuExchangeRatingRec)));

	connect(tserver, SIGNAL(BlueprintRatedSc(AuExchangeRatingRec)),
			auSceneServerApp, SLOT(BlueprintRatedScUp(AuExchangeRatingRec)));

	connect(tserver, SIGNAL(ScDesignExchangePublished(AuScDesignExchangeEntry)),
			auSceneServerApp, SLOT(ScDesignExchangePublishedUp(AuScDesignExchangeEntry)));

	connect(tserver, SIGNAL(ScDesignExchangeUnpublished(quint32)),
			auSceneServerApp, SLOT(ScDesignExchangeUnpublishedUp(quint32)));

	// Chat channel initiates request for city mail report.
	connect(tserver, SIGNAL(CityMailRequestedSignal(AuId, AuId, quint32)),
			auSceneServerApp, SLOT(CityMailRequested(AuId, AuId, quint32)));

	connect(tserver, SIGNAL(CityReportRequested(AuId, quint8, AuId, quint32)),
			auSceneServerApp, SIGNAL(CityReportRequested(AuId, quint8, AuId, quint32)));

	connect(tserver, SIGNAL(EmpireChgBdPref(quint32, int, quint32)),
			auSceneServerApp, SLOT(EmpireChgBdPrefUp(quint32, int, quint32)));

	connect(tserver, SIGNAL(EmpireChgScPref(quint32, int, quint32)),
			auSceneServerApp, SLOT(EmpireChgScPrefUp(quint32, int, quint32)));

	connect(tserver, SIGNAL(EmpireChgFlag(quint32, QImage)), auSceneServerApp, SLOT(EmpireChgFlagUp(quint32, QImage)));

	connect(tserver, SIGNAL(EmpireChgName(quint32, QString)), auSceneServerApp, SLOT(EmpireChgNameUp(quint32, QString)));

	connect(tserver, SIGNAL(EmpireChgRole(quint32, quint8)), auSceneServerApp, SLOT(EmpireChgRoleUp(quint32, quint8)));

	connect(tserver, SIGNAL(EstablishNewEmpire(AuId, QString, quint8, quint32, QString, QImage)),
			auSceneServerApp, SLOT(EstablishNewEmpire(AuId, QString, quint8, quint32, QString, QImage)),
			Qt::QueuedConnection);

	connect(tserver, SIGNAL(GovBossChanged(quint32, AuId, AuId)),
			auSceneServerApp, SLOT(GovBossChanged(quint32, AuId, AuId)));

	connect(tserver, SIGNAL(GovDossierAddDocument(quint32, quint32, QByteArray)),
			auSceneServerApp, SLOT(GovDossierAddDocument(quint32, quint32, QByteArray)));

	connect(tserver, SIGNAL(GovDossierChangeState(quint32, quint32, AuTime, quint8, quint16, quint8)),
			auSceneServerApp, SLOT(GovDossierChangeStateUp(quint32, quint32, AuTime, quint8, quint16, quint8)));

	connect(tserver, SIGNAL(GovSetAnnouncements(quint32, QString)),
			auSceneServerApp, SLOT(GovSetAnnouncementsUp(quint32, QString)));

	connect(tserver, SIGNAL(GovSetWarCriteria(quint32, AuWarCriteria)),
			auSceneServerApp, SLOT(GovSetWarCriteria(quint32, AuWarCriteria)));

	connect(tserver, SIGNAL(GovSetCapital(quint32, AuId, AuSpecies, AuId, int)),
			auSceneServerApp, SLOT(GovSetCapitalUp(quint32, AuId, AuSpecies, AuId, int)));

	connect(tserver, SIGNAL(GovSetJoinPassword(quint32, QByteArray)),
			auSceneServerApp, SLOT(GovSetJoinPasswordUp(quint32, QByteArray)));

	connect(tserver, SIGNAL(GovSetCommerce(quint32, QByteArray)),
			auSceneServerApp, SLOT(GovSetCommerceUp(quint32, QByteArray)));
	connect(tserver, SIGNAL(GovSetContrailColor(quint32, quint8, quint8)),
			auSceneServerApp, SLOT(GovSetContrailColorUp(quint32, quint8, quint8)));
	connect(tserver, SIGNAL(GovSetDefaultPoliticalStance(quint32, quint8)),
			auSceneServerApp, SLOT(GovSetDefaultPoliticalStanceUp(quint32, quint8)));

	connect(tserver, SIGNAL(GovSetPoliticalStance(quint32, quint32, quint8, quint8, AuId, QString, quint8, AuId, QString,
					quint8, quint8, QString, QString, QString)),
			auSceneServerApp, SLOT(SetPoliticalStance(quint32, quint32, quint8, quint8, AuId, QString, quint8, AuId, QString,
					quint8, quint8, QString, QString, QString)));

	connect(tserver, SIGNAL(GovSetTax(quint32, quint8, quint8, quint8)),
			auSceneServerApp, SLOT(GovSetTaxUp(quint32, quint8, quint8, quint8)));

	connect(tserver, SIGNAL(GovSetThemeColor(quint32, quint8, QColor)),
			auSceneServerApp, SLOT(GovSetThemeColorUp(quint32, quint8, QColor)));

	connect(tserver, SIGNAL(LeaveEmpire(AuId, QString, quint32)),
			auSceneServerApp, SLOT(LeaveEmpire(AuId, QString, quint32)));

	connect(tserver, SIGNAL(NameSpecies(AuSpecies, QString, AuId)),
			auSceneServerApp, SLOT(NameSpeciesUp(AuSpecies, QString, AuId)));

	connect(tserver,
			SIGNAL(CityCreated(quint32, quint8, AuId, QString, AuId, QString, AuId, QString, AuId, QString, AuPoint, AuId, QString,
					AuId, quint8)),
			auSceneServerApp,
			SLOT(CityCreated(quint32, quint8, AuId, QString, AuId, QString, AuId, QString, AuId, QString, AuPoint, AuId, QString,
					AuId, quint8)));

	connect(tserver, SIGNAL(OfficeAssigned(AuId, quint32, AuOffice)),
			auSceneServerApp, SLOT(OfficeAssigned(AuId, quint32, AuOffice)));

	connect(tserver, SIGNAL(OfficeRevoked(AuId, quint32)),
			auSceneServerApp, SLOT(OfficeRevoked(AuId, quint32)));

	connect(tserver, SIGNAL(OpenChannelTo(AuId, QString, AuId, quint8, AuId, QString, quint8, bool)),
			auSceneServerApp, SLOT(OpenChannelTo(AuId, QString, AuId, quint8, AuId, QString, quint8, bool)));
	connect(tserver, SIGNAL(PlayerConnecting(quint32, unsigned long)),
			auSceneServerApp, SLOT(PlayerConnectingChat(quint32, unsigned long)));

	connect(tserver, SIGNAL(AcceptTrade(AuTradeAgreement)), auSceneServerApp, SLOT(AcceptTrade(AuTradeAgreement)));
	connect(tserver, SIGNAL(DeactivateTrade(AuTradeAgreement, quint8, bool)),
			auSceneServerApp, SLOT(DeactivateTrade(AuTradeAgreement, quint8, bool)));
	connect(tserver, SIGNAL(ProposeTrade(AuTradeAgreement)), auSceneServerApp, SLOT(ProposeTrade(AuTradeAgreement)));
	connect(tserver, SIGNAL(TradeReadyToShip(AuId, AuTradeAgreement)),
			auSceneServerApp, SLOT(TradeReadyToShip(AuId, AuTradeAgreement)));
	connect(tserver, SIGNAL(TradeShipped(AuId, AuTradeAgreement)),
			auSceneServerApp, SLOT(TradeShipped(AuId, AuTradeAgreement)));

	connect(tserver, SIGNAL(ReleaseBerth(AuId,AuId)), auSceneServerApp, SLOT(ReleaseBerthUp(AuId,AuId)));

	connect(tserver, SIGNAL(RenameAvatar(AuId, QString)), auSceneServerApp, SLOT(RenameAvatarUp(AuId, QString)));
	connect(tserver, SIGNAL(RenameCity(AuId, QString)), auSceneServerApp, SLOT(RenameCityUp(AuId, QString)));
	connect(tserver, SIGNAL(RenamePlanet(AuId, QString, AuId, QString, AuId, QString)),
			auSceneServerApp, SLOT(RenamePlanetUp(AuId, QString, AuId, QString, AuId, QString)));
	connect(tserver, SIGNAL(RenameSector(AuId, QString)), auSceneServerApp, SLOT(RenameSectorUp(AuId, QString)));
	connect(tserver, SIGNAL(RenameSystem(AuId, QString)), auSceneServerApp, SLOT(RenameSystemUp(AuId, QString)));

	connect(tserver, SIGNAL(SendFunds(AuId, QString, AuId, double, quint8, quint8, quint32)),
			auSceneServerApp, SLOT(SendFundsUp(AuId, QString, AuId, double, quint8, quint8, quint32)));

	connect(tserver, SIGNAL(SpacecraftStasisRecover(AuId, AuId, quint8, bool)),
			auSceneServerApp, SLOT(SpacecraftStasisRecoverUp(AuId, AuId, quint8, bool)));

	// This enables a single signal emitted by the app to propagate to all
	// client servers via Qt's nice thread-safe signal/slot mechanism.
	connect(auSceneServerApp, SIGNAL(UnitDisconnected(AuId)), tserver, SLOT(Disconnect(AuId)));

	connect(auSceneServerApp, SIGNAL(EnteredDesigner(AuId, int)), tserver, SLOT(EnteredDesigner(AuId, int)));

	connect(auSceneServerApp, SIGNAL(SceneAtomChanged(AuId, AuId)), tserver, SLOT(SceneAtomChanged(AuId, AuId)));

	// This signal closes all excess chat channels when a nu one is opened from a particular login account.
	connect(auSceneServerApp, SIGNAL(DisconnectExcessChatChannels(quint32, unsigned long)),
			tserver, SLOT(DisconnectPlayer(quint32, unsigned long)));

	connect(auSceneServerApp, SIGNAL(GameOver(AuId, quint16, quint8, QByteArray)),
			tserver, SLOT(GameOver(AuId, quint16, quint8, QByteArray)));
	connect(auSceneServerApp, SIGNAL(GameTurn(AuId, quint16, AuId)),
			tserver, SLOT(GameTurn(AuId, quint16, AuId)));

	// This signal sets the system id in the appropriate chat channel after the scene server thread signals it has changed.
	connect(auSceneServerApp, SIGNAL(SetPersonSystem(AuId, quint32)), tserver, SLOT(SetPersonSystem(AuId, quint32)));

	connect(auSceneServerApp, SIGNAL(ChatToClients(QByteArray)), tserver, SLOT(ChatToClients(QByteArray)));

	connect(auSceneServerApp, SIGNAL(OpenChannelToUnit(AuId, QString, AuId, quint8, AuId, QString, quint8)),
			tserver, SLOT(OpenChannelToUnit(AuId, QString, AuId, quint8, AuId, QString, quint8)));

	connect(auSceneServerApp, SIGNAL(SpeciesRecordAvailable(AuSpecies)), tserver, SLOT(SendSpeciesRecord(AuSpecies)));

	connect(auSceneServerApp, SIGNAL(CityHistoryReportReady(AuId, AuId, AuCityReportHistory)),
			tserver, SLOT(CityHistoryReportReady(AuId, AuId, AuCityReportHistory)));
	connect(auSceneServerApp, SIGNAL(CityLastReportReady(AuId, AuId, AuCityReport, bool, AuCityReport)),
			tserver, SLOT(CityLastReportReady(AuId, AuId, AuCityReport, bool, AuCityReport)));

	// By connecting the thread's finished() signal to QObject::deleteLater(),
	// we ensure that the thread gets deleted once it has finished.
	connect(tserver, SIGNAL(finished()), SLOT(ServerThreadFinished()));
	//connect(tserver, SIGNAL(finished()), tserver, SLOT(deleteLater()));
	//qDebug("AuChatChannel::incomingConnection DtChatChannel thread created");

	// Start the thread.
	tserver->start();
	//qDebug("AuChatChannel::incomingConnection DtChatChannel thread started");

	// Instead, let's keep track of these server threads so we can monitor their status.
	m_ServersLock.lockForWrite();
	m_Servers.append(tserver);
	m_ServersLock.unlock();
	//qDebug("AuChatChannel::incomingConnection %d server threads.", m_Servers.count());
}

void AuChatChannel::ServerThreadFinished()
{
	//qDebug("AuChatChannel::ServerThreadFinished");
	m_ServersLock.lockForWrite();
	QList<DtChatChannel*>::iterator it = m_Servers.begin();
	while(it != m_Servers.end())
	{
		if((*it)->isFinished())
		{
			// We found a server thread that is supposed to die and that has finished.
			//qDebug("AuChatChannel::ServerThreadFinished - Found the server thread.");
			(*it)->deleteLater();
			it = m_Servers.erase(it);
		}
		else
			it++;
	}
	m_ServersLock.unlock();
}

void AuChatChannel::DeferClient(AuId unit)
{
	auSceneServerApp->QueryAtomServer(unit, 0, AuId(0));
}

void AuChatChannel::ListDesignsInUse(QSet<quint32>& bdDesigns, QSet<quint32>& scDesigns)
{
	m_ServersLock.lockForRead();
	QList<DtChatChannel*>::iterator it = m_Servers.begin();
	while(it != m_Servers.end())
	{
		if((*it)->isRunning())
			(*it)->ListDesignsInUse(bdDesigns, scDesigns);
		it++;
	}
	m_ServersLock.unlock();
}

