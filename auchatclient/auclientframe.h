//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auclientframe.h,v 1.28 2022/04/15 20:35:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuClientFrame_H
#define AuClientFrame_H

#include "auarenaready.h"
#include "aucityreport.h"
#include "aucommpacket.h"
#include "audockwidget.h"
#include "audossier.h"
#include "auframe.h"
#include "auid.h"
#include "aujoboffer.h"
#include "aumailmsg.h"
#include "auoffice.h"
#include "aushipment.h"
#include "autime.h"
#include "autransportertarget.h"
#include <QComboBox>
#include <QMainWindow>
#include <QMutex>

class AuAgentDialog;
class AuBdExchangeDock;
class AuBioDock;
class AuBuildDock;
class AuBuildingDock;
class AuCargoDock;
class AuCargoDockFree;
class AuCargoHoldDock;
class AuCargoJettisonDock;
class AuCargoOrdnanceDock;
class AuCargoTransferDock;
class AuCommDock;
class AuCommPacket;
class AuConstructionDock;
class AuDesignerBar;
class AuDesignerBar2;
class AuDesignerPartsDock;
class AuDesignerTextureWrapDock;
class AuDialog;
class AuGearPos;
class AuGearDock;
class AuGovAnnouncementsDock;
class AuGovCitiesDock;
class AuGovCitizensDock;
class AuGovCommerceDock;
class AuGovDiplomacyDock;
class AuGovPatentsDock;
class AuGovPolicyDock;
class AuGovTaxesDock;
class AuGovernDock;
class AuHandCraftDialog;
class AuIntro;
class AuJoystick;
class AuLocatorCompassDock;
class AuLocatorDock;
class AuLocatorGPSDock;
class AuLocatorStarMapDock;
class AuLocatorSystemScanDock;
class AuLocatorWorldMapDock;
class AuLogin;
class AuMailDock;
class AuPlayersOnlineDock;
class AuScExchangeDock;
class AuSettingsDialog;
class AuSpacecraftAccountDock;
class AuSpacecraftBountiesDock;
class AuSpacecraftDamageDock;
class AuSpacecraftDock;
class AuSpacecraftDockFree;
class AuSpacecraftMissionDock;
class AuSpacecraftPolicyDock;
class AuSpacecraftRosterDock;
class AuSpacecraftShipmentsDock;
class AuUnitOrdersDock;
class AuView;
class AuClientView;
class DaAtom;
class DaSector;
class DdCommand;
class DiControlInputArray;
class DzSpecies;
class QKeyEvent;
class QMessageBox;
class QNetworkAccessManager;
class QNetworkDiskCache;
class QNetworkReply;
class QOpenGLWidget;
class QTimer;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuClientFrame : public AuFrame
{
	Q_OBJECT;
private:
	AuAgentDialog*						m_Agent;
	QList<AuCommPacket*>				m_EarlyArrivals;		// Messages that arrive before the comm is ready for them.
	AuIntro*							m_Intro;				// Create avatar and go startup.
	QNetworkAccessManager*				m_Net;
	QNetworkDiskCache*					m_NetCache;
	bool								m_SelectGameUp;
	bool								m_SelectPlayerUp;
	AuSettingsDialog*					m_Settings;
	QTimer*								m_Timer;
	AuClientView*						m_View;
	AuElapsedTimer						m_WaitTime;				// The amount of time waiting for basic game data
																// After 30 seconds we will shut down.

private:
	mutable QMutex						m_muEmpireDataState;
	int									m_EmpireDataState;		// For interface dialogs that use empire records. 0=ok, 1=incoming,
																// 2=received

private:
	mutable QMutex						m_ControlLock;
	unordered_map<Qt::Key, AuElapsedTimer, AuKey_Hash>	m_ActiveKeys;	// Keys currently down. Time is used by DiControlInput; starts null.
	Qt::KeyboardModifiers				m_ActiveKeyMods;		// Keyboard modifiers active at last keyboard event.
    DiControlInputArray*				m_pControls;			// Control inputs.

private:
	// Dock Bars
	AuFrameDockList						m_DockBars;
	AuBdExchangeDock*					m_BdExchangeDock;
	AuBioDock*							m_BioDock;
	AuBuildingDock*						m_BuildingDock;
	AuCargoHoldDock*					m_CargoHoldDock;
	AuCargoJettisonDock*				m_CargoManifestDock;
	AuCargoOrdnanceDock*				m_CargoOrdnanceDock;
	AuCargoTransferDock*				m_CargoTransferDock;
	AuCommDock*							m_CommDock;
	AuGearDock*							m_GearDock;
	AuGovernDock*						m_GovernDock;
	AuGovAnnouncementsDock*				m_GovernAnnouncementsDock;
	AuGovCitiesDock*					m_GovernCitiesDock;
	AuGovCitizensDock*					m_GovernCitizensDock;
	AuGovCommerceDock*					m_GovernCommerceDock;
	AuGovDiplomacyDock*					m_GovernDiplomacyDock;
	AuGovPatentsDock*					m_GovernPatentsDock;
	AuGovPolicyDock*					m_GovernPolicyDock;
	AuGovTaxesDock*						m_GovernTaxesDock;
	AuHandCraftDialog*					m_HandCraftDlg;
	AuCargoDock*						m_HoldDock;				// Tabs for accessing hold and transferring cargo between ships.
	AuCargoDockFree*					m_HoldDockFree;			// Tabs for accessing hold and transferring cargo between ships.
	AuJoystick*							m_Joystick;				// NULL on failure to initialize.
	AuLocatorDock*						m_LocatorDock;
	AuLocatorCompassDock*				m_LocatorCompassDock;
	AuLocatorGPSDock*					m_LocatorGPSDock;
	AuLocatorStarMapDock*				m_LocatorStarMapDock;
	AuLocatorSystemScanDock*			m_LocatorSystemScanDock;
	AuLocatorWorldMapDock*				m_LocatorWorldMapDock;
	AuMailDock*							m_MailDock;
	AuUnitOrdersDock*					m_OrdersDock;
	AuPlayersOnlineDock*				m_PlayersOnlineDock;
	AuSpacecraftDock*					m_SpacecraftDock;
	AuSpacecraftDockFree*				m_SpacecraftDockFree;
	AuSpacecraftAccountDock*			m_SpacecraftAccountDock;
	AuSpacecraftBountiesDock*			m_SpacecraftBountiesDock;
	AuSpacecraftDamageDock*				m_SpacecraftDamageDock;
	AuScExchangeDock*					m_SpacecraftDesignExchangeDock;
	AuSpacecraftMissionDock*			m_SpacecraftMissionDock;
	AuSpacecraftPolicyDock*				m_SpacecraftPolicyDock;
	AuSpacecraftRosterDock*				m_SpacecraftRosterDock;
	AuSpacecraftShipmentsDock*			m_SpacecraftShipmentsDock;

public:
	AuClientFrame(QWidget *parent=0);
	virtual ~AuClientFrame();

	virtual void DisableAFK();

	virtual bool IsAFK() const;

	inline unordered_map<Qt::Key, AuElapsedTimer, AuKey_Hash>& LockActiveKeys()
	{
		m_ControlLock.lock();
		return m_ActiveKeys;
	}

	inline const unordered_map<Qt::Key, AuElapsedTimer, AuKey_Hash>& LockActiveKeys() const
	{
		m_ControlLock.lock();
		return m_ActiveKeys;
	}

	inline void UnlockActiveKeys() const
	{
		m_ControlLock.unlock();
	}

	inline void ClearActiveKeys()
	{
		m_ControlLock.lock();
		m_ActiveKeys.clear();
		m_ControlLock.unlock();
	}

	virtual void ChannelOpened(const QString& channelName, AuCommStdChannel::SCOPE scope, const QString& narrDesc);

	virtual void ChatReceived(AuCommPacket& msg);

	virtual void ClearNetCache();

	// 'control' is a CONTROLINPUT
	virtual QString ControlBinding(CONTROLINPUT control) const;

	void ConfigureControls(const DiControlInputArray &aAllControls);

	// Returns true if face mode should be enabled on the menu.
	bool DesignerMenuObjectModeEnabled();

	void EnumControlInputs(DiControlInputArray &aAllControls) const;

	// Returns true if m_EmpireStateChanged is currently 2 and resets it to 0 if so.
	// Returns false if m_EmipreStateChanged is not 2 and does nothing else.
	virtual bool GetAndResetEmpireDataReceived();

	// Returns true if m_EmpireStateChanged is currently 1, which means we are in the process of receiving empire data.
	bool EmpireDataIncoming() const;

	// Returns the selection position in the avatar's current gear, based on the gear window.
	AuGearPos GetGearSelectionPos() const;

	// Returns 0 if the joystick could not be initialized.
	inline AuJoystick* Joystick() const
	{
		return m_Joystick;
	}

	virtual void ListOpenChannels(QStringList& channelNames) const;

	void LoadControls() const;

	virtual void OperateTurboLift(const AuId& shipId, const AuId& personId, quint16 liftId);

	virtual void ReplaceSpecies(const DzSpecies& species);

	void RestoreDefaultControls();

	void SaveControls() const;

	virtual void SendChat(AuCommPacket& msg);

	void SetMailRecipient(const AuId& personId);

	virtual QSize sizeHint() const;

	// Initializes the star map with data from the sector atoms we have locally.
	// Do not call this until the scene atom's empire record is available locally.
	virtual void StarMapInit(quint32 empireId);

	virtual void StopThreads();

	virtual AuView* View() const;

	AuClientView* ClientView() const;

public slots:
	void AcceptCrew(bool accept);
	void AcceptPassengers(bool accept);
	void AcceptTroops(bool accept);

	void AccessPolicy(int policy);

	// Configures current alert condition on a DaSc ship.
	void AlertCondition(int condition);

	void AlertMessageReceived(AuMailMsg msg);

	void AllowPassengerSeaChests(bool allow);

	void ArenaExit();

	void ArenaRecall();

	void ArenaStatus();

	void AutoFetchVehicles(bool autoFetch);

	void AvatarsUpdated();

	void BrokerShipment(AuShipment shipment);

	void BrokerShipmentsClear();

	void BrokerTradeAgreement(AuTradeAgreement ta);

	void BrokerTradeAgreementsClear();

	void BuildingCodes();

	void ChoicePicked(quint32 storyInstance, qint8 value, AuId offeredBy);

	void CityHistoryReportReady(AuId cityId, AuCityReportHistory report);

	void CityLastReportReady(AuId cityId, AuCityReport report);

	void ClientUnitEmpireReceived(quint32 empireId);

	virtual void CloseSettingsUi();

	void ControlsChanged();

	void EmpireDataBegin();

	void EmpireDataComplete();

	void EnterStudio(int panelIndex, QString password);

	void GovDossierAddNote(quint32 dosEmpire, QString docNote);

	void GovDossierDispatchCourier(quint32 toEmpire, QString message);

	void GovDossierDocState(quint32 dosEmpire, AuTime docTime, quint8 docType, quint16 docIndex, quint8 docState);

	void GovSetAnnouncements(QString announcements);

	void GovSetCommerce(QByteArray commerce);

	void GovSetDefaultPoliticalStance(int stance);

	virtual void GovSetEmpireCapital();

	virtual void GovSetGalaxyCapital();

	virtual void GovSetSectorCapital();

	void GovSetTaxes(double income, double sales, double subsidy);

	void GovSetWarCriteria(AuWarCriteria crit);

	void HangarQueryAnswered(QString shipDesc, quint32 shipDesign, quint32 shipsInHangar);

	void HelpAbout();

	void HelpRequestAccepted(AuId reqId, QString reqName);

	void JobAvailable(AuJobOffer job);

	void KeyPressed(QKeyEvent* e);

	void KeyReleased(QKeyEvent* e);

	void KickBerthOccupant(AuId occupant);

	void KickCryoBerthOccupants(AuSpecies species, int count);

	void KickStowaways();

	QString OrdersChannel() const;

	void OrdnanceReceived(AuId loaderId, QByteArray ord);

	void MissionSensorContactsReceived(QByteArray bablips);

	virtual QIODevice* NetCacheData(const QUrl& url);

	void NetReplyFinished(QNetworkReply* reply);

	void PostNetRequests();

	// 'role' is 0=none, 1=crew, 2=troop, 3=passenger
	void ReviveCryoBerthOccupant(AuSpecies species, quint8 role);

	void SetManufacturingProcessForConstruction(quint16 mpid);

	void SetWorkerEnabledForConstruction(bool enableWorker);

	void PilotUpPolicy(int policy);

	void ReceiveShipFunds(double amount, AuId idShip);

	void ReconcileRoster();

	void ReleaseBerth();

	void ReportSystemSpacecraft(quint32 empireId, AuId sectorId, AuId systemId, QString systemName);

	void ReportSystemResources(AuId systemId, QString systemName, quint8 galaxy, bool isCoreBlackHole, quint8 genHab,
			quint32 genSeed, AuPoint systemLoc, float starLight, int scanEx);

	void RestoreGeometry();

	void SceneAtomChanged();

	// 'sceneContext' is an AuView::SCENE_CONTEXT value.
	void SceneContextChanged(int sceneContext);

	void SendShipFunds(double amount, AuId idShip);

	void SetPoliticalStance(quint32 towardEmpire, quint8 stance, quint8 vassalTributePercent);

	virtual void setVisible(bool visible);

	void ShieldFuelPolicy(int policy);

	void ShieldUpPolicy(int policy);

	void ShowAvatars();

	void ShowCitiesOnStarMap(AuIdList cityIds);

	void ShowGalaxyLocOnStarMap(QString galaxyName, AuPoint loc);

	void ShowForum();

	void ShowPeaceDimensionStandings();

	void ShowVideos();

	void ShowWarDimensionStandings();

	void ShowWiki();

	void SpawnRecall();
	void SpawnTie();

	virtual void SpeciesRequested(AuSpecies species);

	void StarMapErased();

	void StarMapAddSector(AuId sectorId);

	virtual void StarMapAddSector(const DaSector* sector, quint32 empireId);

	void StarMapUpdateAllComplete();

	void StarMapUpdateGalaxyComplete(int galaxy);

	void StarMapUpdateSectorComplete(int galaxy, AuIdList sectorIds);

	void StarMapUpdateAllStarted();

	void StarMapUpdateGalaxyStarted(int galaxy);

	void StarMapUpdateSectorStarted(AuId sector, unsigned int range);

	void SurrenderCities(AuIdList cityIds, QStringList cityNames, quint32 toEmpire);

	void SwitchAvatar();

	void SwitchAvatarTransferred(AuId transferredId);

	virtual void SwitchToAvatar(AuId avatarId);

	void SwitchToAvatarIndex(int avatarIndex);

	void SwitchToAvatar1();

	void SwitchToAvatar2();

	void SwitchToAvatar3();

	void SwitchToAvatar4();

	void SwitchToAvatar5();

	void SwitchToAvatar6();

	void Timer();
	void ToggleAgentUi();
	void ToggleAllUis();
	void ToggleBdExchangeUi();
	void ToggleBuildingUi();
	void ToggleCargoHoldUi();
	void ToggleCargoManifestUi();
	void ToggleCargoOrdnanceUi();
	void ToggleCargoTransferUi();
	void ToggleCharacterUi();
	void ToggleCommUi();
	void ToggleFullScreen();
	void ToggleGearUi();
	void ToggleGovernAnnouncementsUi();
	void ToggleGovernCitiesUi();
	void ToggleGovernCitizensUi();
	void ToggleGovernCommerceUi();
	void ToggleGovernDiplomacyUi();
	void ToggleGovernPatentsUi();
	void ToggleGovernPolicyUi();
	void ToggleGovernTaxesUi();
	void ToggleGovernUi();
	void ToggleHandCraftUi();
	void ToggleHoldUi();
	void ToggleLocatorCompassUi();
	void ToggleLocatorGPSUi();
	void ToggleLocatorStarMapUi();
	void ToggleLocatorSystemScanUi();
	void ToggleLocatorUi();
	void ToggleLocatorWorldMapUi();
	void ToggleMailUi();
	void ToggleOrdersUi();
	void TogglePlayersOnlineUi();
	void ToggleScExchangeUi();
	void ToggleSettingsUi();
	void ToggleSpacecraftAccountUi();
	void ToggleSpacecraftBountiesUi();
	void ToggleSpacecraftDamageUi();
	void ToggleSpacecraftMissionUi();
	void ToggleSpacecraftPolicyUi();
	void ToggleSpacecraftRosterUi();
	void ToggleSpacecraftShipmentsUi();
	void ToggleSpacecraftUi();

	virtual void TransferCitizensAvailable(QByteArray ba);

	// Scene atom wants to enter a cartesian coordinate at a transporter console.
	virtual void TransporterCoordUI(const AuPoint& personLoc);

	// Scene atom wants to enter a longitude latitude coordinate at a transporter console.
	virtual void TransporterLongLatUI(const AuGlobeLLF& ll);

	void TransporterPlace(AuTransporterTarget tt);

	void TransporterShip(AuId id, QString name, QString desc);

	void TransporterTransporter(AuId id, QString name, QString desc, quint16 whichTransporter);

protected:
	virtual void closeEvent(QCloseEvent* event);

	void CreateDockWidgets();

	void CreateViewWidget();

	void ProcessControlInput(const DiControlInputArray& aActiveControls);
};

#endif // AuClientFrame_H
