//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dtchatchannel.h,v 1.373 2022/04/15 20:40:27 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DtChatChannel_H
#define DtChatChannel_H

#include "auarenaready.h"
#include "aubddesignexchange.h"
#include "aucityreport.h"
#include "aucommstdchannel.h"
#include "audossier.h"
#include "auemoji.h"
#include "auexchangerating.h"
#include "augear.h"
#include "auglobell.h"
#include "auhair.h"
#include "auid.h"
#include "aujoboffer.h"
#include "auoffice.h"
#include "auroster.h"
#include "auscdesignexchange.h"
#include "aushipment.h"
#include "autime.h"
#include "deconstructionprocess.h"
#include <QHostAddress>
#include <QImage>
#include <QMutex>
#include <QReadWriteLock>
#include <QSemaphore>
#include <QTcpSocket>
#include <QThread>
#include <QUdpSocket>

class AuBdDesign;
class AuCityReport;
class AuCityReportHistory;
class AuCommPacket;
class AuDesign;
class AuDesignPosition;
class AuDesignTex;
class AuDNA;
class AuGearPos;
class AuItDesign;
class AuItem;
class AuItemHash;
class AuMission;
class AuMissionOrder;
class AuScDesign;
class AuScFaceList;
class AuScIdSet;
class AuScPart;
class AuScParts;
class AuScript;
class AuScSelection;
class AuServerInterface;
class DaPerson;
class DaSolarSystem;
class DaVessel;
class DiControlInputArray;
class DpBody;

class DtChatChannel : public QThread
{
	Q_OBJECT;
public:
	DtChatChannel(int socketDescriptor, QObject* parent=0);
	~DtChatChannel();

	/*
	bool Alive() const
	{
		return !m_semAlive.available();
	}
	*/

	inline int BadPacketCount() const
	{
		return m_BadPackets;
	}

	inline int BytesRx() const
	{
		return m_BytesRx;
	}

	inline int BytesTx() const
	{
		return m_BytesTx;
	}

	inline const AuTime& ConnectTime() const
	{
		return m_tmConnect;
	}

	void DesignStudioEnter(quint16 panelIndex, const QString& password);

	bool IsCommChannelOpen(const QString& channelName);

	void ListDesignsInUse(QSet<quint32>& bdDesigns, QSet<quint32>& scDesigns);

	// 'narrDesc' is the brief description for a narrator channel. Must be non-blank for narrator channel, blank for non-narrator.
	// Returns true if a channel is opened.
	bool OpenCommChannel(const QString& channelName, AuCommStdChannel::SCOPE scope, const QString& narrDesc);

	inline quint32 PlayerId() const
	{
		return m_idPlayer;
	}

	// 'chgWhat' is
	//     0=ground cover DNA regenerated in scene atom's current resource zone
	//     1=shrubbery DNA regenerated in scene atom's current resource zone
	//     2=trees DNA regenerated in scene atom's current resource zone
	//     3=reset workerDNA in scene atom's current worker zone to match admin's dude
	//     4=clear workerDNA in scene atom's current worker zone
	// 'floraZone' is 0=polar, 1=temperate, 2=tropical
	void RegenDNA(int chgWhat, int floraZone);

	void SendPacket(const QByteArray& ba, bool flush=true, bool usemutex=true);	// Adds a packet to the transmit buffer.

	void SendBdDesign(quint32 idDesign);

	void SendItDesign(quint32 idDesign);

	void SendScDesign(quint32 idDesign);

	// Responds to client request for all TRANSPORTER_TARGET places in range of the transporter room in which they are standing.
	void SendTransporterPlaces();

	// Responds to client request for all spacecraft that are in range of the transporter room in which they are standing.
	void SendTransporterSpacecraft();

	// Responds to client request for all transporters that are in range of the transporter room in which they are standing.
	void SendTransporterTransporters();

	void TransferListCitizens(const AuId& shipId);

	const AuId& UnitId() const;

signals:
	// move 'person' to the atom 'dest'.
	void AdminGoTo(AuId person, AuId dest, quint8 rideAlso, bool limboRescue, AuId returnOnFail);
	void AdminGoToGalaxyLoc(AuId personOrRide, AuId origSystem, AuId destGalaxyId, AuPoint destGalaxyLoc);
	void AdminSummon(AuId person, AuId adminId, quint8 rideAlso);				// move 'person' to the atom 'adminId'.

	void BlueprintRatedBd(AuExchangeRatingRec rating);
	void BlueprintRatedSc(AuExchangeRatingRec rating);

	void BountyPosted(AuId posterId, QString posterName, AuId victim, QString reason, double reward, int why);

	void ChatReceived(QByteArray msg);

	// 'cityType' is a CITYTYPE enum value.
	// 'galaxy' is a GALAXY enum value.
	void CityCreated(quint32 empireId,
			quint8 cityType, AuId cityId, QString cityName,
			AuId founderId, QString founderName,
			AuId planetId, QString planetName,
			AuId systemId, QString systemName, AuPoint systemLoc,
			AuId sectorId, QString sectorName,
			AuId galaxyId, quint8 galaxy);

	// Request city mail report starts here if city is not found locally.
	void CityMailRequestedSignal(AuId cityId, AuId unitId, quint32 unitEmpire);

	// 'reportWhat': 0=last city report, 1=city history
	void CityReportRequested(AuId cityId, quint8 reportWhat, AuId unitId, quint32 unitEmpire);

	void CollectBounty(AuId bountyHunter, QString bountyHunterName, AuGearPos gearPos, AuBountyOnItem bountyOnItem);

	void BdDesignExchangePublished(AuBdDesignExchangeEntry item);

	void BdDesignExchangeUnpublished(quint32 designId);

	void ScDesignExchangePublished(AuScDesignExchangeEntry item);

	void ScDesignExchangeUnpublished(quint32 designId);

	// 'industry' is an INDUSTRYID
	void EmpireChgBdPref(quint32 empireId, int industry, quint32 designId);

	// 'scrole' is an AuScPrefs::SCROLE
	void EmpireChgScPref(quint32 empireId, int scrole, quint32 designId);

	void EmpireChgFlag(quint32 empireId, QImage flag);

	void EmpireChgName(quint32 empireId, QString name);

	void EmpireChgRole(quint32 empireId, quint8 gameRole);

	void error(QTcpSocket::SocketError socketError);

	void EstablishNewEmpire(AuId personId, const QString& personName, quint8 personGender, quint32 idOldEmpire, const QString& name,
			const QImage& imFlag);

	void GovBossChanged(quint32 empire, AuId governorId, AuId bossId);

	void GovDossierAddDocument(quint32 empire, quint32 dosEmpire, QByteArray packedDoc);

	void GovDossierChangeState(quint32 empire, quint32 dosEmpire, AuTime docTime, quint8 docType, quint16 docIndex,
			quint8 docState);

	void GovSetAnnouncements(quint32 empire, QString announcements);

	// 'capitalType' 0=sector, 1=empire, 3=galaxy
	void GovSetCapital(quint32 idEmpire, AuId city, AuSpecies workerSpecies, AuId domain, int capitalType);
	void GovSetCommerce(quint32 idEmpire, QByteArray commerce);
	void GovSetContrailColor(quint32 idEmpire, quint8 hue, quint8 sat);
	void GovSetDefaultPoliticalStance(quint32 empire, quint8 stance);
	void GovSetJoinPassword(quint32 idEmpire, QByteArray passCrypt);

	void GovSetPoliticalStance(quint32 idToEmpire, quint32 idFromEmpire, quint8 stance, quint8 vassalTributePercent,
			AuId fromUnit, QString fromUnitName, quint8 toStance, AuId toUnit, QString toUnitName,
			quint8 reason, quint8 galaxy, QString sector, QString system, QString world);

	void GovSetTax(quint32 empire, quint8 income, quint8 sales, quint8 subsidy);

	void GovSetThemeColor(quint32 empire, quint8 theme, QColor color);

	void GovSetWarCriteria(quint32 empire, AuWarCriteria crit);

	void LeaveEmpire(AuId personId, QString personName, quint32 idOldEmpire);

	void NameSpecies(AuSpecies species, QString specieName, AuId discoverer);

	void OfficeAssigned(AuId person, quint32 empire, AuOffice office);

	void OfficeRevoked(AuId person, quint32 empire);

	void OpenChannelTo(AuId fromUnitId, QString fromName, AuId fromSector, quint8 fromGalaxy, AuId toUnitId, QString channelName,
			quint8 scope, bool propagateUp);

	void PlayerConnecting(quint32 playerId, unsigned long threadId);	// Player login account id, not an atom id.

	void AcceptTrade(AuTradeAgreement ta);

	void DeactivateTrade(AuTradeAgreement ta, quint8 nuState, bool atDest);

	void ProposeTrade(AuTradeAgreement ta);

	void ReleaseBerth(AuId personId, AuId shipId);
	
	void RenameAvatar(AuId avatarId, QString nuName);

	void RenameCity(AuId cityId, QString nuName);

	void RenamePlanet(AuId planetId, QString nuName, AuId systemId, QString systemName, AuId sectorId, QString sectorName);

	void RenameSector(AuId sectorId, QString nuName);

	void RenameSystem(AuId systemId, QString nuName);

	// 'fromGalaxy' is a GALAXY value.
	void SendFunds(AuId fromId, QString fromName, AuId toId, double amount, quint8 fromGalaxy, quint8 reason, quint32 designId);

	void SpacecraftStasisRecover(AuId shipId, AuId personId, quint8 personGalaxy, bool agentRecover);

	void TradeReadyToShip(AuId readyCapitolId, AuTradeAgreement ta);

	void TradeShipped(AuId shipperCapitolId, AuTradeAgreement ta);

public slots:
	void ChatToClients(QByteArray msg);

	void CityHistoryReportReady(AuId unitId, AuId cityId, AuCityReportHistory report);

	void CityLastReportReady(AuId unitId, AuId cityId, AuCityReport report, bool havePrevReport, AuCityReport prevReport);

	void Disconnect(AuId idAtom);
	void DisconnectPlayer(quint32 playerId, unsigned long exceptThreadId);

	void EnteredDesigner(AuId personId, int designStudioMode);

	// Notification from an atom that a game has ended.
	void GameOver(AuId shipId, quint16 whichTable, quint8 gameType, QByteArray gameResult);

	// Notification from an atom that it is someone's turn in a game. This is send periodically as a constant reminder to the
	// person that it is their turn. That way the client can fixup user interfaces after a wormhole or client crash.
	void GameTurn(AuId shipId, quint16 whichTable, AuId personsTurn);

	void OpenChannelToUnit(AuId fromUnitId, QString fromName, AuId fromSector, quint8 fromGalaxy, AuId toUnitId,
			QString channelname, quint8 scope);

	void ReportCityLast(const AuId& cityId);

	void ReportCityHistory(const AuId& cityId);

	void SceneAtomChanged(AuId oldPersonId, AuId nuPersonId);

	void SendSpeciesRecord(AuSpecies species);
	void SetPersonSystem(AuId systemId, quint32 playerId);

protected:
	virtual void run();

private:
	void AgentAssignEmperor(quint32 empireId, const AuId& toWho, const QString& toName);

	void AgentGoToGalaxyLoc(GALAXY destGalaxy, const AuPoint& destGalaxyLoc, bool rideAlso);

	void AgentMakeCommodity(int makeWhat, COMMODITYID cid, quint8 quality, quint32 count, GALAXY destGalaxy,
			const AuPoint& destGalaxyLoc);
	
	void AgentRefuel(quint8 quality);
	
	void AgentRepair();
	
	void ArenaExit();

	void ArenaRecall();

	void ArenaSendList();

	void ArenaStatus();

	// 'fenceStyle' is an AuScFence::STYLE.
	// 'mat' is a COMMODITYID.
	void BdFence(const AuPoint& startLoc, const AuPoint& endLoc, int fenceStyle, const AuId& onPlatform, quint32 customDesignId,
			int mat);

	void BdManufactureSpacecraftProcessDefaultSelected(quint32 designId, const QString& designName, quint8 designQ,
			const AuId& opBd);

	void BdStoreBuyItem(const AuItem& buyItem, const AuItem& offerItem, const AuId& opBd);

	void BdTakeItem(const AuItem& buyItem, const AuId& opBd);

	void ChatReceivedFromClient(AuCommPacket& msg);	// 'msg' not const so values can be overridden.

	void ChatToClient(AuCommPacket* msg);

	void CheckLoginStatus();

	inline bool ClientHasBdDesign(quint32 designId) const	// True if the client has been sent the spacecraft design.
	{
		m_rwBdDesigns.lockForRead();
		bool hasDesign = m_BdDesigns.find(designId) != m_BdDesigns.cend();
		m_rwBdDesigns.unlock();
		return hasDesign;
	}

	inline bool ClientHasItDesign(quint32 designId) const	// True if the client has been sent the spacecraft design.
	{
		m_rwItDesigns.lockForRead();
		bool hasDesign = m_ItDesigns.find(designId) != m_ItDesigns.cend();
		m_rwItDesigns.unlock();
		return hasDesign;
	}

	inline bool ClientHasScDesign(quint32 designId) const	// True if the client has been sent the spacecraft design.
	{
		m_rwScDesigns.lockForRead();
		bool hasDesign = m_ScDesigns.find(designId) != m_ScDesigns.cend();
		m_rwScDesigns.unlock();
		return hasDesign;
	}

	void DesignExit();

	// 'designMode' is a DaDesignBase::DESIGNMODE
	void DesignOpen(quint32 designId, int designMode);

	// Lists designs made by this architect.
	void DesignerArchitectListBd();

	void DesignerArchitectListIt();

	void DesignerArchitectListSc();

	void DesignerGearOpen(quint32 designId, const AuGearPos& pos);

	// Lists published designs in the library.
	void BdDesignExchangeList();

	void BdDesignExchangeList(const QList<quint32>& designIds);

	void BdDesignExchangeReportRatings(quint32 designId);

	void BdDesignExchangeToC();

	// Lists published designs in the library.
	void ScDesignExchangeList();

	void ScDesignExchangeList(const QList<quint32>& designIds);

	void ScDesignExchangeReportRatings(quint32 designId);

	void ScDesignExchangeToC();

	void Disconnect();

	void FillOrDrainCapacitor(quint8 fillOrDrain, quint8 percent);

	void GeoLandscaper(const AuPoint& startLoc, const AuPoint& endLoc, float roadWidth, bool hideRocks, int hideGrass,
			bool hideShrubs, bool hideTrees, WORLDTEX_TYPE pavement);

	void GeoRoad(const AuPoint& startLoc, const AuPoint& endLoc, bool isSlab, float roadWidth, quint8 geoType);

	void HangarLaunch(bool takeCaptBerth);

	void HangarMothball(const AuId& shipId);

	void HangarQuery();

	// 0=domestic animals, 1=wild animals
	void KillAnimals(quint8 killWhat);

	// Mission commands affect the current ship. Scene person must have authority or the command is ignored.
	void MissionSend(QTcpSocket& tcpSocket);				// Sends the current ship's mission to the client.
	void MissionSendSensorContacts(QTcpSocket& tcpSocket);	// Sends the current ship's sensor contacts to the client.

	void OrdnanceList();

	void QueryAvatars();

	void QueryJobs();

	void QueryJobs(const QList<AuJob::TYPE>& filterTypes, int maxSectorsAway);

	void ReadPacket(const QByteArray& ba, QTcpSocket& tcpSocket);

	void RxData(QTcpSocket& tcpSocket);

	void SendCityLastReport(const AuId& cityId, const AuCityReport& report, const AuCityReport* prevReport);

	void SendCityReportHistory(const AuId& cityId, const AuCityReportHistory& report);

	void SendPlayerOnlineStatus();

	void SendSpeciesRecordIfNeeded(const AuSpecies& species);

	void SetAutoEat(bool autoEat);

	void SetCurrentPerson(const AuId& personId);

	void SetDNA(const AuDNA& dna);

	void SetHairStyle(AuHairStyle::STYLE haircut, AuHairStyle::SHAVE shave);

	// Unit inventory control.
	void SetUnitClothing(const AuId& unitId, const AuItem& invId);
	void SetUnitHelmet(const AuId& unitId, const AuItem& invId);
	void SetUnitWeapon(const AuId& unitId, const AuItem& invId);

	void ShipmentCreate(const AuShipment& shipment);					// on station
	void ShipmentCreate(const AuShipment& shipment, const AuId& opBd);	// at bd

	void ShipmentDelete(const AuShipment& shipment);					// on station
	void ShipmentDelete(const AuShipment& shipment, const AuId& opBd);	// at bd

	void ShipmentsRefresh(bool sayResult);								// on station
	void ShipmentsRefresh(const AuId& opBd, bool sayResult);			// at bd

	void SpawnTieShip();

	void TradeAgreementAccept(const AuTradeAgreement& ta, const AuId& opBd);	// at bd

	void TradeAgreementCounter(const AuTradeAgreement& proposal, const AuTradeAgreement& counter, const AuId& opBd);	// at bd

	void TradeAgreementCreate(const AuTradeAgreement& ta, const AuId& opBd);	// at bd

	void TradeAgreementDelete(const AuTradeAgreement& ta, const AuId& opBd);	// at bd

	// Returns the current galaxy containing UnitId
	// Must lock root to acquire this.
	GALAXY UnitGalaxy() const;

	// Returns the current sector id containing UnitId
	// Must lock root to acquire this.
	AuId UnitSector() const;

private:
	// User
	ACCOUNT								m_AccountType;
	unsigned int						m_confirmNum;			// Confirmation number assigned by login server.
	AuEmojiList							m_Emojis;				// Emojis with player visage customizations.
	AuServerInterface*					m_I;
	quint32								m_idPlayer;				// Id of player in login database.
	unordered_map<AuId, bool, AuIdHash>	m_SquelchIds;			// List of contact Ids that are being squelched
	QString								m_UnitName;

	bool								m_BdDesignsRequested;	// Requested from db.
	quint8								m_DesignStudioMode;		// DaDesignBase::DESIGNMODE.
	bool								m_ItDesignsRequested;	// Requested from db.
	AuElapsedTimer						m_LastChat;				// Date/time of last PC_Chat packet.
	bool								m_ScDesignsRequested;	// Requested from db.
	Qt::HANDLE							m_ThreadId;

	// Comm
	unsigned int						m_BadPackets;			// # of packets rx'd with unknown packet type
	unsigned int						m_BytesRx;				// Total bytes received from client
	unsigned int						m_BytesTx;				// Total bytes sent to client
	bool								m_Disconnect;			// true when time to disconnect.
	quint32								m_PacketSize;			// Data packet size
	int									m_PacketSizeBytesLeft;	// Number of packet size bytes we are expecting.
	int									m_socketDescriptor;		// Provided in the constructor.
	AuTime								m_tmConnect;			// Time connection is established.

private:
	QMutex								m_muTxBuffer;			// Guards m_TxBuffer.
	QByteArray							m_TxBuffer;				// On the client, the tx buffer is a property of AuApp.
	bool								m_TxBufferFlush;		// true to flush the data when it is next transmitted.

private:
	QMutex								m_muChannels;			// Guards m_Channels.
	QList<QString>						m_Channels;				// Comm channels the client has opened.

private:
	mutable QReadWriteLock				m_rwBdDesigns;			// Guards m_BdDesigns.
	unordered_map<quint32, bool, AuDesignIdHash>	m_BdDesigns;	// AuBdDesign records the client has received.
	unordered_map<quint32, bool, AuDesignIdHash>	m_BdDesignsInUse;	// AuBdDesigns in use, may not have been sent to client.

private:
	mutable QReadWriteLock				m_rwItDesigns;			// Guards m_ItDesigns.
	unordered_map<quint32, bool, AuDesignIdHash>	m_ItDesigns;	// AuItDesign records the client has received.
	unordered_map<quint32, bool, AuDesignIdHash>	m_ItDesignsInUse;	// AuItDesigns in use, may not have been sent to client.

private:
	mutable QReadWriteLock				m_rwScDesigns;			// Guards m_ScDesigns.
	unordered_map<quint32, bool, AuDesignIdHash>	m_ScDesigns;	// AuScDesign records the client has received.
	unordered_map<quint32, bool, AuDesignIdHash>	m_ScDesignsInUse;	// AuScDesigns in use, may not have been sent to client.

private:
	QMutex								m_muSpecies;
	unordered_map<AuSpecies, bool, AuSpeciesHash>	m_Species;		// All species the client has requested.
};

#endif // DtChatChannel_H
