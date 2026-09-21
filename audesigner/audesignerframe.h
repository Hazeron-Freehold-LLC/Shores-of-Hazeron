//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audesignerframe.h,v 1.48 2022/04/15 21:02:19 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDesignerFrame_H
#define AuDesignerFrame_H

#include "auarenaready.h"
#include "aucityreport.h"
#include "aucommpacket.h"
#include "audesigner.h"
#include "audockwidget.h"
#include "audossier.h"
#include "auelapsedtimer.h"
#include "auframe.h"
#include "auid.h"
#include "aujoboffer.h"
#include "aumailmsg.h"
#include "auoffice.h"
#include "aushipment.h"
#include "autransportertarget.h"
#include <QComboBox>
#include <QMainWindow>
#include <QMutex>

class AuAgentDialog;
class AuBdExchangeDock;
class AuBioDock;
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
class AuDesignerView;
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
class AuIntroBase;
class AuJoystick;
class AuLaborDock;
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
class AuSpacecraftSpecsDock;
class AuStoryDialog;
class AuUnitOrdersDock;
class AuView;
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

class AUDESIGNER_DLL AuDesignerFrame : public AuFrame
{
	Q_OBJECT;
private:
	AuAgentDialog*						m_Agent;
	QList<AuCommPacket*>				m_EarlyArrivals;		// Messages that arrive before the comm is ready for them.
	QNetworkAccessManager*				m_Net;
	QNetworkDiskCache*					m_NetCache;
	bool								m_SelectGameUp;
	bool								m_SelectPlayerUp;
	AuSettingsDialog*					m_Settings;
	QTimer*								m_Timer;
	AuDesignerView*						m_View;
	AuElapsedTimer						m_WaitTime;				// The amount of time waiting for basic game data
																// After 30 seconds we will shut down.

protected:
	AuIntroBase*						m_Intro;				// Login and startup page.

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
	AuConstructionDock*					m_ConstructionDock;
	AuDesignerBar*						m_DesignerBar;
	AuDesignerBar2*						m_DesignerBar2;
	AuDesignerPartsDock*				m_DesignerPartsDock;
	AuDesignerTextureWrapDock*			m_DesignerTextureWrapDock;
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
	AuSpacecraftSpecsDock*				m_SpacecraftSpecsDock;
	QHash<quint32, AuStoryDialog*>		m_StoryDialogs;			// key is story instance

	// Designer actions. The only ones listed here tend to be the ones that need to be dis/enabled or un/checked.
	QAction*							m_Designer_About;
	QAction*							m_Designer_Align;
	QAction*							m_Designer_AlignLiftStop;
	QAction*							m_Designer_Associate;
	QAction*							m_Designer_Barrier;
	QAction*							m_Designer_BdHullMax;
	QAction*							m_Designer_BerthCapt;
	QAction*							m_Designer_BerthCrew;
	QAction*							m_Designer_BerthName;
	QAction*							m_Designer_BerthOfficer;
	QAction*							m_Designer_BerthPassenger;
	QAction*							m_Designer_BerthTroop;
	QAction*							m_Designer_BoundGround;
	QAction*							m_Designer_BoundMax;
	QAction*							m_Designer_BuildingCodes;
	QAction*							m_Designer_BuildZone;
	QAction*							m_Designer_CaptName;
	QAction*							m_Designer_ChangeDNA;
	QAction*							m_Designer_CitizenPost;
	QAction*							m_Designer_ConsoleCommand;
	QAction*							m_Designer_ConsoleEngineer;
	QAction*							m_Designer_ConsoleFireControl;
	QAction*							m_Designer_ConsoleHelm;
	QAction*							m_Designer_ConsoleMedic;
	QAction*							m_Designer_ConsoleNavigator;
	QAction*							m_Designer_ConsolePowerRelay;
	QAction*							m_Designer_ConsoleSensor;
	QAction*							m_Designer_ConsoleShield;
	QAction*							m_Designer_ConsoleTransporter;
	QAction*							m_Designer_ContrailEmitter;
	QAction*							m_Designer_Copy;
	QAction*							m_Designer_CopySymmetrical;
	QAction*							m_Designer_Cut;
	QAction*							m_Designer_CutFace;
	QAction*							m_Designer_CuttingJig;
	QAction*							m_Designer_DecalJig;
	QAction*							m_Designer_Delete;
	QAction*							m_Designer_Depart;
	QAction*							m_Designer_DeselectAll;
	QAction*							m_Designer_DesignFinalize;
	QAction*							m_Designer_DesignFinalizeReport;
	QMenu*								m_Designer_DesignMenu;
	QAction*							m_Designer_DesignProperties;
	QMenu*								m_Designer_DetailsMenu;
	QAction*							m_Designer_Dissociate;
	QAction*							m_Designer_DivideFaces;
	QAction*							m_Designer_DoorJig;
	QAction*							m_Designer_DoorProperties;
	QMenu*								m_Designer_EditMenu;
	QAction*							m_Designer_EffectIntensity;
	QAction*							m_Designer_EmpireFlag;
	QAction*							m_Designer_EmpireName;
	QAction*							m_Designer_Exit;
	QAction*							m_Designer_ExportSTL;
	QMenu*								m_Designer_FaceMenu;
	QAction*							m_Designer_Fence;
	QAction*							m_Designer_FindEdges;
	QAction*							m_Designer_FindFaces;
	QAction*							m_Designer_FindVertices;
	QAction*							m_Designer_Flatten;
	QAction*							m_Designer_FractureFaces;
	QAction*							m_Designer_FullScreen;
	QAction*							m_Designer_FuseFaces;
	QAction*							m_Designer_GameTable;
	QAction*							m_Designer_GearOpen;
	QAction*							m_Designer_GearSaveBd;
	QAction*							m_Designer_GearSaveSc;
	QAction*							m_Designer_Glow;
	QAction*							m_Designer_GoToSelection;
	QAction*							m_Designer_Grab;
	QAction*							m_Designer_GravityToggle;
	QAction*							m_Designer_Grid3pt;
	QAction*							m_Designer_GridFollowsWork;
	QAction*							m_Designer_GridMove;
	QAction*							m_Designer_GridPositionLoad1;
	QAction*							m_Designer_GridPositionLoad2;
	QAction*							m_Designer_GridPositionLoad3;
	QAction*							m_Designer_GridPositionLoad4;
	QAction*							m_Designer_GridPositionLoad5;
	QAction*							m_Designer_GridPositionLoad6;
	QAction*							m_Designer_GridPositionLoad7;
	QAction*							m_Designer_GridPositionLoad8;
	QAction*							m_Designer_GridPositionLoad9;
	QAction*							m_Designer_GridPositionOrigin;
	QAction*							m_Designer_GridPositionSave1;
	QAction*							m_Designer_GridPositionSave2;
	QAction*							m_Designer_GridPositionSave3;
	QAction*							m_Designer_GridPositionSave4;
	QAction*							m_Designer_GridPositionSave5;
	QAction*							m_Designer_GridPositionSave6;
	QAction*							m_Designer_GridPositionSave7;
	QAction*							m_Designer_GridPositionSave8;
	QAction*							m_Designer_GridPositionSave9;
	QAction*							m_Designer_GridXY;
	QAction*							m_Designer_GridXZ;
	QAction*							m_Designer_GridYZ;
	QAction*							m_Designer_HullExtrusion;
	QMenu*								m_Designer_HullMenu;
	QAction*							m_Designer_HullSphereLL;
	QAction*							m_Designer_HullSphereUni;
	QAction*							m_Designer_HullSpindle;
	QAction*							m_Designer_HullTubing;
	QAction*							m_Designer_Import3DS;
	QAction*							m_Designer_ImportSTL;
	QAction*							m_Designer_InteriorExtrusion;
	QAction*							m_Designer_InteriorSphereLL;
	QAction*							m_Designer_InteriorSphereUni;
	QAction*							m_Designer_InteriorSpindle;
	QAction*							m_Designer_InteriorTubing;
	QAction*							m_Designer_InvertSelection;
	QAction*							m_Designer_JigCut;
	QAction*							m_Designer_Join;
	QAction*							m_Designer_LadderPath;
	QAction*							m_Designer_LandingGearExtrusion;
	QMenu*								m_Designer_LandingGearMenu;
	QAction*							m_Designer_LandingGearSphereLL;
	QAction*							m_Designer_LandingGearSphereUni;
	QAction*							m_Designer_LandingGearSpindle;
	QAction*							m_Designer_LandingGearTubing;
	QMenu*								m_Designer_LandscapeMenu;
	QAction*							m_Designer_LandingSystem;
	QAction*							m_Designer_LaunchTube;
	QAction*							m_Designer_LaunchTubeJig;
	QAction*							m_Designer_BdLibraryDelete;
	QAction*							m_Designer_ScLibraryDelete;
	QAction*							m_Designer_BdLibraryOpen;
	QAction*							m_Designer_ScLibraryOpen;
	QAction*							m_Designer_LibrarySave;
	QAction*							m_Designer_LightBulb;
	QAction*							m_Designer_LightBulbProps;
	QAction*							m_Designer_LightLensProps;
	QAction*							m_Designer_LivestockPost;
	QAction*							m_Designer_LoopCut;
	QAction*							m_Designer_MakeDecal;
	QAction*							m_Designer_MakeDoor;
	QAction*							m_Designer_MakeHull;
	QAction*							m_Designer_MakeLandingGear;
	QAction*							m_Designer_MakeLightLens;
	QAction*							m_Designer_MakeMesh;
	QAction*							m_Designer_MakeNavLightLens;
	QAction*							m_Designer_MakeRoom;
	QAction*							m_Designer_MakeRotatingBeaconLens;
	QAction*							m_Designer_MakeStrobeLightLens;
	QAction*							m_Designer_MakeTurboLift;
	QAction*							m_Designer_MakeTurret;
	QAction*							m_Designer_MakeWindow;
	QAction*							m_Designer_MoveDoorSwitches;
	QAction*							m_Designer_NewBuilding;
	QAction*							m_Designer_NewItem;
	QAction*							m_Designer_NewSpacecraft;
	QAction*							m_Designer_NewVehicle;
	QAction*							m_Designer_ObjectExtrusion;
	QAction*							m_Designer_ObjectFace;
	QAction*							m_Designer_ObjectIsolate;
	QAction*							m_Designer_ObjectMode;
	QAction*							m_Designer_ObjectQuad;
	QAction*							m_Designer_ObjectSphereLL;
	QAction*							m_Designer_ObjectSphereUni;
	QAction*							m_Designer_ObjectSpindle;
	QAction*							m_Designer_ObjectTriangle;
	QAction*							m_Designer_ObjectTubing;
	QAction*							m_Designer_Opacity;
	QAction*							m_Designer_Open;
	QAction*							m_Designer_Paint;
	QAction*							m_Designer_PanelCapacitor;
	QAction*							m_Designer_PanelFTLDrive;
	QAction*							m_Designer_PanelLifeSupport;
	QAction*							m_Designer_PanelManeuverDrive;
	QAction*							m_Designer_PanelPowerPlant;
	QAction*							m_Designer_PanelSensor;
	QAction*							m_Designer_PanelShield;
	QMenu*								m_Designer_PanelsMenu;
	QAction*							m_Designer_PanelWeapon;
	QAction*							m_Designer_ParkGroundVehicle;
	QAction*							m_Designer_ParkRocket;
	QAction*							m_Designer_ParkSpacecraft;
	QAction*							m_Designer_ParkSpaceVehicle;
	QAction*							m_Designer_ParkWaterVehicle;
	QMenu*								m_Designer_PartMenu;
	QMenu*								m_Designer_PartPropsMenu;
	QAction*							m_Designer_Paste;
	QAction*							m_Designer_PasteAt;
	QAction*							m_Designer_PasteRadial;
	QMenu*								m_Designer_PathsMenu;
	QAction*							m_Designer_PodBay;
	QAction*							m_Designer_Polish;
	QAction*							m_Designer_PreviewBd;
	QAction*							m_Designer_PreviewPeople;
	QAction*							m_Designer_PreviewSettings;
	QAction*							m_Designer_PreviewToggle;
	QAction*							m_Designer_PunchFace;
	QAction*							m_Designer_PushPull;
	QAction*							m_Designer_Redo;
	QAction*							m_Designer_RefreshObstructions;
	QAction*							m_Designer_RemoteCamera;
	QAction*							m_Designer_ReverseFaces;
	QAction*							m_Designer_RoomExtrusion;
	QMenu*								m_Designer_RoomMenu;
	QAction*							m_Designer_RoomSphereLL;
	QAction*							m_Designer_RoomSphereUni;
	QAction*							m_Designer_RoomSpindle;
	QAction*							m_Designer_RoomTubing;
	QAction*							m_Designer_Rotate;
	QAction*							m_Designer_Save;
	QAction*							m_Designer_SaveAs;
	QAction*							m_Designer_SelectAll;
	QAction*							m_Designer_SelectContiguous;
	QAction*							m_Designer_SelectEdges;
	QAction*							m_Designer_SelectFaces;
	QAction*							m_Designer_SelectVertices;
	QAction*							m_Designer_SelfIllumination;
	QAction*							m_Designer_Settings;
	QAction*							m_Designer_SetRoomProperties;
	QAction*							m_Designer_SetVoidProperties;
	QAction*							m_Designer_ShipName;
	QAction*							m_Designer_Shrub;
	QAction*							m_Designer_ShowBacks;
	QAction*							m_Designer_ShowEdges;
	QAction*							m_Designer_ShowFaces;
	QAction*							m_Designer_ShowGrid;
	QAction*							m_Designer_ShowGround;
	QAction*							m_Designer_ShowSkyBox;
	QAction*							m_Designer_Site;
	QAction*							m_Designer_SiteMax;
	QAction*							m_Designer_SiteProperties;
	QAction*							m_Designer_Sketch;
	QAction*							m_Designer_Smooth;
	QAction*							m_Designer_SnapToEdges;
	QAction*							m_Designer_SnapToGrid;
	QAction*							m_Designer_SnapToVertices;
	QAction*							m_Designer_SolidsBlockPath;
	QAction*							m_Designer_SplitFaces;
	QAction*							m_Designer_StationCapt;
	QAction*							m_Designer_StationDesigner;
	QAction*							m_Designer_StationEngineer;
	QAction*							m_Designer_StationFireControl;
	QAction*							m_Designer_StationGunner;
	QAction*							m_Designer_StationHelm;
	QAction*							m_Designer_StationMedic;
	QAction*							m_Designer_StationNavigator;
	QAction*							m_Designer_StationPowerRelay;
	QAction*							m_Designer_StationSensor;
	QAction*							m_Designer_StationShield;
	QMenu*								m_Designer_StationsMenu;
	QAction*							m_Designer_StationTransporter;
	QAction*							m_Designer_StatusAlert;
	QAction*							m_Designer_StatusLifeSupport;
	QAction*							m_Designer_StatusMission;
	QAction*							m_Designer_StatusWorldMap;
	QAction*							m_Designer_Stretch;
	QAction*							m_Designer_Tree;
	QAction*							m_Designer_TransporterPadLocs;
	QAction*							m_Designer_TroopPost;
	QAction*							m_Designer_TurretBase;
	QAction*							m_Designer_TurretGun;
	QAction*							m_Designer_Undo;
	QMenu*								m_Designer_VehiclesMenu;
	QAction*							m_Designer_Videos;
	QAction*							m_Designer_ViewPositionGrid;
	QAction*							m_Designer_ViewPositionLoad1;
	QAction*							m_Designer_ViewPositionLoad2;
	QAction*							m_Designer_ViewPositionLoad3;
	QAction*							m_Designer_ViewPositionLoad4;
	QAction*							m_Designer_ViewPositionLoad5;
	QAction*							m_Designer_ViewPositionLoad6;
	QAction*							m_Designer_ViewPositionLoad7;
	QAction*							m_Designer_ViewPositionLoad8;
	QAction*							m_Designer_ViewPositionLoad9;
	QAction*							m_Designer_ViewPositionOrigin;
	QAction*							m_Designer_ViewPositionSave1;
	QAction*							m_Designer_ViewPositionSave2;
	QAction*							m_Designer_ViewPositionSave3;
	QAction*							m_Designer_ViewPositionSave4;
	QAction*							m_Designer_ViewPositionSave5;
	QAction*							m_Designer_ViewPositionSave6;
	QAction*							m_Designer_ViewPositionSave7;
	QAction*							m_Designer_ViewPositionSave8;
	QAction*							m_Designer_ViewPositionSave9;
	QAction*							m_Designer_VoidEnvironmentExtrusion;
	QAction*							m_Designer_VoidEnvironmentSpindle;
	QMenu*								m_Designer_VoidEnvMenu;
	QAction*							m_Designer_VoidHullExtrusion;
	QMenu*								m_Designer_VoidHullMenu;
	QAction*							m_Designer_VoidHullSpindle;
	QAction*							m_Designer_VoidRoomExtrusion;
	QMenu*								m_Designer_VoidRoomMenu;
	QAction*							m_Designer_VoidRoomSpindle;
	QAction*							m_Designer_WalkPath;
	QAction*							m_Designer_WeaponEmitLoc;
	QAction*							m_Designer_Wiki;
	QAction*							m_Designer_WindowJig;

public:
	AuDesignerFrame(QWidget *parent=0);
	virtual ~AuDesignerFrame();

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

	AuDesignerView* DesignerView() const;

	AuDesignerView* ViewWidget() const;

public slots:
	void AlertMessageReceived(AuMailMsg msg);

	void AvatarsUpdated();

	void BrokerShipment(AuShipment shipment);

	void BrokerShipmentsClear();

	void BrokerTradeAgreement(AuTradeAgreement ta);

	void BrokerTradeAgreementsClear();

	void BuildingCodes();

	void CityHistoryReportReady(AuId cityId, AuCityReportHistory report);

	void CityLastReportReady(AuId cityId, AuCityReport report);

	void ClientUnitEmpireReceived(quint32 empireId);

	virtual void CloseSettingsUi();

	void ControlsChanged();

	void DesignerBlipCoordinate(AuPoint coord);

	void DesignerEnableControls();

	virtual void DesignerExited();

	void DesignerFindEdgesToggle();

	void DesignerFindFacesToggle();

	void DesignerFindVerticesToggle();

	void DesignerGridToggle();

	// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
	bool DesignerKeyReceived(QKeyEvent* e);

	void DesignerMenuObjectIsolate();

	void DesignerMenuObjectMode();

	void DesignerMenuSetFindEdges();

	void DesignerMenuSetFindFaces();

	void DesignerMenuSetFindVertices();

	void DesignerMenuSetGravityOn();

	void DesignerMenuSetGridFollowsWork();

	void DesignerMenuSetPreview();

	void DesignerMenuSetShowBacks();

	void DesignerMenuSetShowEdges();

	void DesignerMenuSetShowFaces();

	void DesignerMenuSetShowGround();

	void DesignerMenuSetShowSkyBox();

	void DesignerMenuSetSnapToEdges();

	void DesignerMenuSetSnapToGrid();

	void DesignerMenuSetSnapToVertices();

	void DesignerMenuSetSolidsBlockPath();

	void DesignerSetFindEdges(bool find);

	void DesignerSetFindFaces(bool find);

	void DesignerSetFindVertices(bool find);

	void DesignerSetGravityOn(bool grav);

	void DesignerSetGridFollowsWork(bool follow);

	void DesignerSetObjectIsolate(bool objectIsolate);

	void DesignerSetObjectMode(bool objectMode);

	void DesignerSetPreview(bool preview);

	void DesignerSetShowBacks(bool show);

	void DesignerSetShowEdges(bool show);

	void DesignerSetShowFaces(bool show);

	void DesignerSetShowGrid(bool show);

	void DesignerSetShowGround(bool show);

	void DesignerSetShowSkyBox(bool show);

	void DesignerSetSnapToEdges(bool snap);

	void DesignerSetSnapToGrid(bool snap);

	void DesignerSetSnapToVertices(bool snap);

	void DesignerSetSolidsBlockPath(bool block);

	void EmpireDataBegin();

	void EmpireDataComplete();

	virtual void GameOver(quint8 gameType, QByteArray gameResult);

	virtual void GameSelect(quint8 gameType, bool gameSelected);

	virtual void GameTurn(AuId personsTurn);

	void HangarQueryAnswered(QString shipDesc, quint32 shipDesign, quint32 shipsInHangar);

	void HelpAbout();

	void HelpRequestAccepted(AuId reqId, QString reqName);

	void JobAvailable(AuJobOffer job);

	void KeyPressed(QKeyEvent* e);

	void KeyReleased(QKeyEvent* e);

	void LaunchDesigner();

	QString OrdersChannel() const;

	void OrdnanceReceived(AuId loaderId, QByteArray ord);

	void MissionSensorContactsReceived(QByteArray bablips);

	virtual QIODevice* NetCacheData(const QUrl& url);

	void NetReplyFinished(QNetworkReply* reply);

	void PostNetRequests();

	void ReportSystemSpacecraft(quint32 empireId, AuId sectorId, AuId systemId, QString systemName);

	void ReportSystemResources(AuId systemId, QString systemName, quint8 galaxy, bool isCoreBlackHole, quint8 genHab,
			quint32 genSeed, AuPoint systemLoc, float starLight, int scanEx);

	void RestoreGeometry();

	void SceneAtomChanged();

	// 'sceneContext' is an AuView::SCENE_CONTEXT value.
	void SceneContextChanged(int sceneContext);

	void SelectGame();

	virtual void setVisible(bool visible);

	void ShowAvatars();

	void ShowCitiesOnStarMap(AuIdList cityIds);

	void ShowGalaxyLocOnStarMap(QString galaxyName, AuPoint loc);

	void ShowForum();

	void ShowPeaceDimensionStandings();

	void ShowVideos();

	void ShowWarDimensionStandings();

	// Opens wiki with browser at, "https://www.hazeron.com/wiki/index.php/Designer"
	void ShowWikiDesigner();

	void SpawnRecall();
	void SpawnTie();

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

	virtual void Timer();

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
	void ToggleConstructionUi();
	void ToggleDesignPartsUi();
	void ToggleDesignTextureWrapUi();
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
	void ToggleSettingsUi();
	void ToggleSpacecraftAccountUi();
	void ToggleSpacecraftBountiesUi();
	void ToggleSpacecraftDamageUi();
	void ToggleSpacecraftDesignExchangeUi();
	void ToggleSpacecraftMissionUi();
	void ToggleSpacecraftPolicyUi();
	void ToggleSpacecraftRosterUi();
	void ToggleSpacecraftShipmentsUi();
	void ToggleSpacecraftSpecsUi();
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

	void CreateDesignerMenu();

	virtual AuDesignerView* CreateViewWidget() = 0;

	void ProcessControlInput(const DiControlInputArray& aActiveControls);

	void ShowDesignerMenu();
};

#endif // AuDesignerFrame_H
