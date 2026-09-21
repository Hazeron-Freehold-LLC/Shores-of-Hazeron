//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audesignerframe.cpp,v 1.97 2022/05/19 15:59:38 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audesignerframe.h"
//
#include "auabout.h"
#include "auaccesshold.h"
#include "auagentdialog.h"
#include "auapp.h"
#include "auatommanager.h"
#include "aubdexchange.h"
#include "aubdpreviewinit.h"
#include "aubeamoptions.h"
#include "aubeamvehicleoptions.h"
#include "aubio.h"
#include "aubiojobs.h"
#include "aubld.h"
#include "aubldhangar.h"
#include "aucargodialog.h"
#include "aucargojettison.h"
#include "aucargotransfer.h"
#include "aucity.h"
#include "aucityhistorydlg.h"
#include "aucityreportdlg.h"
#include "auclientglobal.h"
#include "aucomm.h"
#include "auconstruction.h"
#include "audamagereport.h"
#include "audesignerbar.h"
#include "audesignerbar2.h"
#include "audesignerparts.h"
#include "audesignerview.h"
#include "audesignertexturewrap.h"
#include "audocktitle.h"
#include "auempirecitizens.h"
#include "augameselect.h"
#include "augearwidget.h"
#include "augovannouncements.h"
#include "augovcities.h"
#include "augovcommerce.h"
#include "augovdiplomacy.h"
#include "augovern.h"
#include "augovpatents.h"
#include "augovpolicy.h"
#include "augovpolicy.h"
#include "augovtaxes.h"
#include "auindustrydlg.h"
#include "auintrobase.h"
#include "aujobsquerydialog.h"
#include "aujoystick.h"
#include "aulaborhandcraft.h"
#include "aulocator.h"
#include "aulocatorgps.h"
#include "aulocatormap.h"
#include "aulocatorpos.h"
#include "aulocatorstarmap.h"
#include "aumaildock.h"
#include "AuMissionTool.h"
#include "AuMsgDialog.h"
#include "auordnancetransfer.h"
#include "auplayersonline.h"
#include "ausolodesignerdlg.h"
#include "auscexchange.h"
#include "auscpreviewinit.h"
#include "ausetcontrols.h"
#include "ausettingsdialog.h"
#include "auspacecraftaccount.h"
#include "auspacecraftberths.h"
#include "auspacecraftberthsdialog.h"
#include "auspacecraftbounties.h"
#include "auspacecraftpages.h"
#include "auspacecraftpolicy.h"
#include "auspacecraftshipments.h"
#include "auspacecraftspecs.h"
#include "austorydialog.h"
#include "auswitchavatar.h"
#include "ausystemscan.h"
#include "ausystemspacecraft.h"
#include "auunitorders.h"
#include "daatomlist.h"
#include "dagalaxy.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dgdata.h"
#include "dicontrolinput.h"
#include <QDesktopServices>
#include <QGridLayout>
#include <QInputDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QNetworkReply>
#include <QTimer>
#include <QUrl>

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Table of control inputs.
DiControlInputArray s_FrameControls;		// Controls that may have bindings modified by the user.
const DiControlInput s_FrameDefaults[] =
{
	// Commands that do not toggle the visibility of a window.
	DiControlInput(QObject::tr("All Windows Toggle"),
			AuFrame::CI_ToggleAllUIs, Qt::Key_F1, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Exit %1").arg("Hazeron"),
			QObject::tr("The operating system may signal program exit in response to specific key combinations, "
				"regardless of the binding of this key."),
			AuFrame::CI_ExitGame, Qt::Key_F4, Qt::AltModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Full Screen Toggle"),
			AuFrame::CI_FullScreenToggle, Qt::Key_Return, Qt::AltModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),

	// Commands that toggle the visibility of a single window.
	DiControlInput(QObject::tr("About %1").arg("Hazeron"),
			QObject::tr("Displays a window with information about <i>%1</i>.").arg("Hazeron"),
			AuFrame::CI_About, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Agent Window Toggle, <i>Agent Only</i>"),		// Agent only
			AuFrame::CI_ToggleAgentUI, Qt::Key_F9, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Avatars Online Window Toggle"),
			AuFrame::CI_TogglePlayersOnlineUI, Qt::Key_F8, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Bio Window Toggle"),
			AuFrame::CI_ToggleCharacterUI, Qt::Key_F8, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Building Window Toggle"),
			QObject::tr("The building window interacts with buildings in cities."),
			AuFrame::CI_ToggleBuildingUI, Qt::Key_F10, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Building Blueprint Exchange Window Toggle"),
			AuFrame::CI_ToggleBdExchangeUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Cargo Window Toggle"),
			AuFrame::CI_ToggleCargoUI, Qt::Key_F5, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Cargo Hold Window Toggle"),
			AuFrame::CI_ToggleCargoHoldUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Cargo Manifest Window Toggle"),
			AuFrame::CI_ToggleCargoManifestUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Cargo Ordnance Window Toggle"),
			AuFrame::CI_ToggleCargoOrdnanceUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Cargo Transfer Window Toggle"),
			AuFrame::CI_ToggleCargoTransferUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Comm Window Toggle"),
			AuFrame::CI_ToggleCommUI, Qt::Key_F3, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Construction Window Toggle"),
			AuFrame::CI_ToggleConstructionUI, Qt::Key_F11, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Designer Parts Window Toggle"),
			QObject::tr("The parts window can only be opened when you are inside a design studio."),
			AuFrame::CI_ToggleDesignPartsUI, Qt::Key_F6, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Designer Texture Wrap Window Toggle"),
			QObject::tr("The texture wrap window can only be opened when you are inside a design studio."),
			AuFrame::CI_ToggleDesignTextureWrapUI, Qt::Key_F6, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Gear Window Toggle"),
			AuFrame::CI_ToggleGearUI, Qt::Key_F4, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Governance Window Toggle"),
			AuFrame::CI_ToggleGovernUI, Qt::Key_F12, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Govern Announcements Window Toggle"),
			AuFrame::CI_ToggleGovernAnnouncementsUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Govern Cities and Bases Window Toggle"),
			AuFrame::CI_ToggleGovernCitiesUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Govern Citizens Window Toggle"),
			AuFrame::CI_ToggleGovernCitizensUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Govern Commerce Window Toggle"),
			AuFrame::CI_ToggleGovernCommerceUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Govern Diplomacy Window Toggle"),
			AuFrame::CI_ToggleGovernDiplomacyUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Govern Policy Window Toggle"),
			AuFrame::CI_ToggleGovernPolicyUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Govern Taxes Window Toggle"),
			AuFrame::CI_ToggleGovernTaxesUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Hand Craft Window Toggle"),
			AuFrame::CI_ToggleHandCraftUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Locator Window Toggle"),
			AuFrame::CI_ToggleLocatorUI, Qt::Key_F7, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Locator Compass Window Toggle"),
			AuFrame::CI_ToggleLocatorCompassUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Locator GPS Window Toggle"),
			AuFrame::CI_ToggleLocatorGPSUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Locator Star Map Window Toggle"),
			AuFrame::CI_ToggleLocatorStarMapUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Locator System Map Window Toggle"),
			AuFrame::CI_ToggleLocatorSystemScanUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Locator World Map Window Toggle"),
			AuFrame::CI_ToggleLocatorWorldMapUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Mail Window Toggle"),
			AuFrame::CI_ToggleMailUI, Qt::Key_F2, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Settings Window Toggle"),
			AuFrame::CI_ToggleSettingsUI, Qt::Key_F12, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Spacecraft Window Toggle"),
			AuFrame::CI_ToggleSpacecraftUI, Qt::Key_F9, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Spacecraft Account Window Toggle"),
			AuFrame::CI_ToggleSpacecraftAccountUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Spacecraft Bounties Window Toggle"),
			AuFrame::CI_ToggleSpacecraftBountiesUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Spacecraft Damage Window Toggle"),
			AuFrame::CI_ToggleSpacecraftDamageUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Spacecraft Blueprint Exchange Window Toggle"),
			AuFrame::CI_ToggleScExchangeUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Spacecraft Mission Window Toggle"),
			AuFrame::CI_ToggleSpacecraftMissionUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Spacecraft Policy Window Toggle"),
			AuFrame::CI_ToggleSpacecraftPolicyUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Spacecraft Roster Window Toggle"),
			AuFrame::CI_ToggleSpacecraftRosterUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Spacecraft Shipments Window Toggle"),
			AuFrame::CI_ToggleSpacecraftShipmentsUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Spacecraft Specifications Window Toggle"),
			AuFrame::CI_ToggleSpacecraftSpecsUI, Qt::Key_unknown, Qt::NoModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Unit Orders Window Toggle"),
			AuFrame::CI_ToggleOrdersUI, Qt::Key_F3, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
};
int s_NumFrameDefaults = sizeof(s_FrameDefaults)/sizeof(DiControlInput);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDesignerFrame::AuDesignerFrame(QWidget *parent) : AuFrame(parent)
{
	setBackgroundRole(QPalette::Background);
	setWindowIcon(QIcon(auApp->Icon32()));
	setWindowTitle(g_Au.ApplicationName());

	// App filters for function keys so I get them no matter who has focus.
	connect(auApp, SIGNAL(KeyPressed(QKeyEvent*)), SLOT(KeyPressed(QKeyEvent*)));
	connect(auApp, SIGNAL(KeyReleased(QKeyEvent*)), SLOT(KeyReleased(QKeyEvent*)));

	m_ActiveKeyMods = Qt::NoModifier;
	m_Agent = 0;
	m_BioDock = 0;
	m_BuildingDock = 0;
	m_CargoHoldDock = 0;
	m_CargoManifestDock = 0;
	m_CargoOrdnanceDock = 0;
	m_CargoTransferDock = 0;
	m_CommDock = 0;
	m_ConstructionDock = 0;
	m_DesignerBar = 0;
	m_DesignerBar2 = 0;
	m_DesignerPartsDock = 0;
	m_DesignerTextureWrapDock = 0;
	m_GearDock = 0;
	m_GovernAnnouncementsDock = 0;
	m_GovernCitiesDock = 0;
	m_GovernCitizensDock = 0;
	m_GovernCommerceDock = 0;
	m_GovernDiplomacyDock = 0;
	m_GovernDock = 0;
	m_GovernPatentsDock = 0;
	m_GovernPolicyDock = 0;
	m_GovernTaxesDock = 0;
	m_HandCraftDlg = 0;
	m_HoldDock = 0;
	m_HoldDockFree = 0;
	m_Intro = 0;
	m_LocatorCompassDock = 0;
	m_LocatorDock = 0;
	m_LocatorGPSDock = 0;
	m_LocatorStarMapDock = 0;
	m_LocatorSystemScanDock = 0;
	m_LocatorWorldMapDock = 0;
	m_MailDock = 0;
	m_Net = 0;
	m_NetCache = 0;
	m_OrdersDock = 0;

	m_pControls = new DiControlInputArray;
	EnumControlInputs(*m_pControls);							// Grab current controls right away.

	m_PlayersOnlineDock = 0;
	m_SelectGameUp = false;
	m_SelectPlayerUp = false;
	m_Settings = 0;
	m_SpacecraftAccountDock = 0;
	m_SpacecraftBountiesDock = 0;
	m_SpacecraftDamageDock = 0;
	m_BdExchangeDock = 0;
	m_SpacecraftDesignExchangeDock = 0;
	m_SpacecraftDock = 0;
	m_SpacecraftDockFree = 0;
	m_SpacecraftMissionDock = 0;
	m_SpacecraftPolicyDock = 0;
	m_SpacecraftRosterDock = 0;
	m_SpacecraftShipmentsDock = 0;
	m_SpacecraftSpecsDock = 0;
	m_View = 0;
	m_WaitTime.start();

	// Designer
	m_Designer_About = 0;
	m_Designer_Align = 0;
	m_Designer_AlignLiftStop = 0;
	m_Designer_Associate = 0;
	m_Designer_Barrier = 0;
	m_Designer_BdHullMax = 0;
	m_Designer_BdLibraryDelete = 0;
	m_Designer_BdLibraryOpen = 0;
	m_Designer_BerthCapt = 0;
	m_Designer_BerthCrew = 0;
	m_Designer_BerthName = 0;
	m_Designer_BerthOfficer = 0;
	m_Designer_BerthPassenger = 0;
	m_Designer_BerthTroop = 0;
	m_Designer_BoundGround = 0;
	m_Designer_BoundMax = 0;
	m_Designer_BuildZone = 0;
	m_Designer_BuildingCodes = 0;
	m_Designer_CaptName = 0;
	m_Designer_ChangeDNA = 0;
	m_Designer_CitizenPost = 0;
	m_Designer_ConsoleCommand = 0;
	m_Designer_ConsoleEngineer = 0;
	m_Designer_ConsoleFireControl = 0;
	m_Designer_ConsoleHelm = 0;
	m_Designer_ConsoleMedic = 0;
	m_Designer_ConsoleNavigator = 0;
	m_Designer_ConsolePowerRelay = 0;
	m_Designer_ConsoleSensor = 0;
	m_Designer_ConsoleShield = 0;
	m_Designer_ConsoleTransporter = 0;
	m_Designer_ContrailEmitter = 0;
	m_Designer_Copy = 0;
	m_Designer_CopySymmetrical = 0;
	m_Designer_Cut = 0;
	m_Designer_CutFace = 0;
	m_Designer_CuttingJig = 0;
	m_Designer_DecalJig = 0;
	m_Designer_Delete = 0;
	m_Designer_Depart = 0;
	m_Designer_DeselectAll = 0;
	m_Designer_DesignFinalize = 0;
	m_Designer_DesignFinalizeReport = 0;
	m_Designer_DesignMenu = 0;
	m_Designer_DesignProperties = 0;
	m_Designer_DetailsMenu = 0;
	m_Designer_Dissociate = 0;
	m_Designer_DivideFaces = 0;
	m_Designer_DoorJig = 0;
	m_Designer_DoorProperties = 0;
	m_Designer_EditMenu = 0;
	m_Designer_EffectIntensity = 0;
	m_Designer_EmpireFlag = 0;
	m_Designer_EmpireName = 0;
	m_Designer_Exit = 0;
	m_Designer_ExportSTL = 0;
	m_Designer_FaceMenu = 0;
	m_Designer_Fence = 0;
	m_Designer_FindEdges = 0;
	m_Designer_FindFaces = 0;
	m_Designer_FindVertices = 0;
	m_Designer_Flatten = 0;
	m_Designer_FractureFaces = 0;
	m_Designer_FullScreen = 0;
	m_Designer_FuseFaces = 0;
	m_Designer_GameTable = 0;
	m_Designer_GearOpen = 0;
	m_Designer_GearSaveBd = 0;
	m_Designer_GearSaveSc = 0;
	m_Designer_Glow = 0;
	m_Designer_GoToSelection = 0;
	m_Designer_Grab = 0;
	m_Designer_GravityToggle = 0;
	m_Designer_Grid3pt = 0;
	m_Designer_GridFollowsWork = 0;
	m_Designer_GridMove = 0;
	m_Designer_GridPositionLoad1 = 0;
	m_Designer_GridPositionLoad2 = 0;
	m_Designer_GridPositionLoad3 = 0;
	m_Designer_GridPositionLoad4 = 0;
	m_Designer_GridPositionLoad5 = 0;
	m_Designer_GridPositionLoad6 = 0;
	m_Designer_GridPositionLoad7 = 0;
	m_Designer_GridPositionLoad8 = 0;
	m_Designer_GridPositionLoad9 = 0;
	m_Designer_GridPositionOrigin = 0;
	m_Designer_GridPositionSave1 = 0;
	m_Designer_GridPositionSave2 = 0;
	m_Designer_GridPositionSave3 = 0;
	m_Designer_GridPositionSave4 = 0;
	m_Designer_GridPositionSave5 = 0;
	m_Designer_GridPositionSave6 = 0;
	m_Designer_GridPositionSave7 = 0;
	m_Designer_GridPositionSave8 = 0;
	m_Designer_GridPositionSave9 = 0;
	m_Designer_GridXY = 0;
	m_Designer_GridXZ = 0;
	m_Designer_GridYZ = 0;
	m_Designer_HullExtrusion = 0;
	m_Designer_HullMenu = 0;
	m_Designer_HullSphereLL = 0;
	m_Designer_HullSphereUni = 0;
	m_Designer_HullSpindle = 0;
	m_Designer_HullTubing = 0;
	m_Designer_Import3DS = 0;
	m_Designer_ImportSTL = 0;
	m_Designer_InteriorExtrusion = 0;
	m_Designer_InteriorSphereLL = 0;
	m_Designer_InteriorSphereUni = 0;
	m_Designer_InteriorSpindle = 0;
	m_Designer_InteriorTubing = 0;
	m_Designer_InvertSelection = 0;
	m_Designer_JigCut = 0;
	m_Designer_Join = 0;
	m_Designer_LadderPath = 0;
	m_Designer_LandingGearExtrusion = 0;
	m_Designer_LandingGearMenu = 0;
	m_Designer_LandingGearSphereLL = 0;
	m_Designer_LandingGearSphereUni = 0;
	m_Designer_LandingGearSpindle = 0;
	m_Designer_LandingGearTubing = 0;
	m_Designer_LandingSystem = 0;
	m_Designer_LandscapeMenu = 0;
	m_Designer_LaunchTube = 0;
	m_Designer_LaunchTubeJig = 0;
	m_Designer_LibrarySave = 0;
	m_Designer_LightBulb = 0;
	m_Designer_LightBulbProps = 0;
	m_Designer_LightLensProps = 0;
	m_Designer_LivestockPost = 0;
	m_Designer_LoopCut = 0;
	m_Designer_MakeDecal = 0;
	m_Designer_MakeDoor = 0;
	m_Designer_MakeHull = 0;
	m_Designer_MakeLandingGear = 0;
	m_Designer_MakeLightLens = 0;
	m_Designer_MakeMesh = 0;
	m_Designer_MakeNavLightLens = 0;
	m_Designer_MakeRoom = 0;
	m_Designer_MakeRotatingBeaconLens = 0;
	m_Designer_MakeStrobeLightLens = 0;
	m_Designer_MakeTurboLift = 0;
	m_Designer_MakeTurret = 0;
	m_Designer_MakeWindow = 0;
	m_Designer_MoveDoorSwitches = 0;
	m_Designer_NewBuilding = 0;
	m_Designer_NewItem = 0;
	m_Designer_NewSpacecraft = 0;
	m_Designer_NewVehicle = 0;
	m_Designer_ObjectExtrusion = 0;
	m_Designer_ObjectFace = 0;
	m_Designer_ObjectIsolate = 0;
	m_Designer_ObjectMode = 0;
	m_Designer_ObjectQuad = 0;
	m_Designer_ObjectSphereLL = 0;
	m_Designer_ObjectSphereUni = 0;
	m_Designer_ObjectSpindle = 0;
	m_Designer_ObjectTriangle = 0;
	m_Designer_ObjectTubing = 0;
	m_Designer_Opacity = 0;
	m_Designer_Open = 0;
	m_Designer_Paint = 0;
	m_Designer_PanelCapacitor = 0;
	m_Designer_PanelFTLDrive = 0;
	m_Designer_PanelLifeSupport = 0;
	m_Designer_PanelManeuverDrive = 0;
	m_Designer_PanelPowerPlant = 0;
	m_Designer_PanelSensor = 0;
	m_Designer_PanelShield = 0;
	m_Designer_PanelWeapon = 0;
	m_Designer_PanelsMenu = 0;
	m_Designer_ParkGroundVehicle = 0;
	m_Designer_ParkRocket = 0;
	m_Designer_ParkSpaceVehicle = 0;
	m_Designer_ParkSpacecraft = 0;
	m_Designer_ParkWaterVehicle = 0;
	m_Designer_PartMenu = 0;
	m_Designer_PartPropsMenu = 0;
	m_Designer_Paste = 0;
	m_Designer_PasteAt = 0;
	m_Designer_PasteRadial = 0;
	m_Designer_PathsMenu = 0;
	m_Designer_PodBay = 0;
	m_Designer_Polish = 0;
	m_Designer_PreviewBd = 0;
	m_Designer_PreviewPeople = 0;
	m_Designer_PreviewSettings = 0;
	m_Designer_PreviewToggle = 0;
	m_Designer_PunchFace = 0;
	m_Designer_PushPull = 0;
	m_Designer_RefreshObstructions = 0;
	m_Designer_ReverseFaces = 0;
	m_Designer_RoomExtrusion = 0;
	m_Designer_RoomMenu = 0;
	m_Designer_RoomSphereLL = 0;
	m_Designer_RoomSphereUni = 0;
	m_Designer_RoomSpindle = 0;
	m_Designer_RoomTubing = 0;
	m_Designer_Rotate = 0;
	m_Designer_Save = 0;
	m_Designer_SaveAs = 0;
	m_Designer_ScLibraryDelete = 0;
	m_Designer_ScLibraryOpen = 0;
	m_Designer_SelectAll = 0;
	m_Designer_SelectContiguous = 0;
	m_Designer_SelectEdges = 0;
	m_Designer_SelectFaces = 0;
	m_Designer_SelectVertices = 0;
	m_Designer_SelfIllumination = 0;
	m_Designer_SetRoomProperties = 0;
	m_Designer_SetVoidProperties = 0;
	m_Designer_Settings = 0;
	m_Designer_ShipName = 0;
	m_Designer_ShowBacks = 0;
	m_Designer_ShowEdges = 0;
	m_Designer_ShowFaces = 0;
	m_Designer_ShowGrid = 0;
	m_Designer_ShowGround = 0;
	m_Designer_ShowSkyBox = 0;
	m_Designer_Shrub = 0;
	m_Designer_Site = 0;
	m_Designer_SiteMax = 0;
	m_Designer_SiteProperties = 0;
	m_Designer_Sketch = 0;
	m_Designer_Smooth = 0;
	m_Designer_SnapToEdges = 0;
	m_Designer_SnapToGrid = 0;
	m_Designer_SnapToVertices = 0;
	m_Designer_SolidsBlockPath = 0;
	m_Designer_SplitFaces = 0;
	m_Designer_StationCapt = 0;
	m_Designer_StationDesigner = 0;
	m_Designer_StationEngineer = 0;
	m_Designer_StationFireControl = 0;
	m_Designer_StationGunner = 0;
	m_Designer_StationHelm = 0;
	m_Designer_StationMedic = 0;
	m_Designer_StationNavigator = 0;
	m_Designer_StationPowerRelay = 0;
	m_Designer_StationSensor = 0;
	m_Designer_StationShield = 0;
	m_Designer_StationTransporter = 0;
	m_Designer_StationsMenu = 0;
	m_Designer_StatusAlert = 0;
	m_Designer_StatusLifeSupport = 0;
	m_Designer_StatusMission = 0;
	m_Designer_StatusWorldMap = 0;
	m_Designer_Stretch = 0;
	m_Designer_TransporterPadLocs = 0;
	m_Designer_Tree = 0;
	m_Designer_TroopPost = 0;
	m_Designer_TurretBase = 0;
	m_Designer_TurretGun = 0;
	m_Designer_VehiclesMenu = 0;
	m_Designer_Videos = 0;
	m_Designer_ViewPositionGrid = 0;
	m_Designer_ViewPositionLoad1 = 0;
	m_Designer_ViewPositionLoad2 = 0;
	m_Designer_ViewPositionLoad3 = 0;
	m_Designer_ViewPositionLoad4 = 0;
	m_Designer_ViewPositionLoad5 = 0;
	m_Designer_ViewPositionLoad6 = 0;
	m_Designer_ViewPositionLoad7 = 0;
	m_Designer_ViewPositionLoad8 = 0;
	m_Designer_ViewPositionLoad9 = 0;
	m_Designer_ViewPositionOrigin = 0;
	m_Designer_ViewPositionSave1 = 0;
	m_Designer_ViewPositionSave2 = 0;
	m_Designer_ViewPositionSave3 = 0;
	m_Designer_ViewPositionSave4 = 0;
	m_Designer_ViewPositionSave5 = 0;
	m_Designer_ViewPositionSave6 = 0;
	m_Designer_ViewPositionSave7 = 0;
	m_Designer_ViewPositionSave8 = 0;
	m_Designer_ViewPositionSave9 = 0;
	m_Designer_VoidEnvironmentExtrusion = 0;
	m_Designer_VoidEnvironmentSpindle = 0;
	m_Designer_VoidEnvMenu = 0;
	m_Designer_VoidHullExtrusion = 0;
	m_Designer_VoidHullMenu = 0;
	m_Designer_VoidHullSpindle = 0;
	m_Designer_VoidRoomExtrusion = 0;
	m_Designer_VoidRoomMenu = 0;
	m_Designer_VoidRoomSpindle = 0;
	m_Designer_WalkPath = 0;
	m_Designer_WeaponEmitLoc = 0;
	m_Designer_Wiki = 0;
	m_Designer_WindowJig = 0;

	m_EmpireDataState = 0;

	m_Timer = 0;

	// Create and initialize the joystick.
	m_Joystick = new AuJoystick;
	if(m_Joystick->Initialize())
	{
		// Delete the object on failure to initialize.
		delete m_Joystick;
		m_Joystick = 0;
	}
}

AuDesignerFrame::~AuDesignerFrame()
{
	//qDebug() << "AuDesignerFrame::~AuDesignerFrame m_Joystick";
	// Deleting the joystick here crashes in SDL.
	//if(m_Joystick)
		//delete m_Joystick;

	//qDebug() << "AuDesignerFrame::~AuDesignerFrame m_pControls";
	if(m_pControls)
		delete m_pControls;

	//qDebug() << "AuDesignerFrame::~AuDesignerFrame earlyArrivals";
	for(QList<AuCommPacket*>::iterator ci = m_EarlyArrivals.begin(); ci != m_EarlyArrivals.end(); ci++)
		delete *ci;
	m_EarlyArrivals.clear();
	//qDebug() << "AuDesignerFrame::~AuDesignerFrame done";
}

void AuDesignerFrame::StopThreads()
{
	if(m_MailDock)
		m_MailDock->StopThreads();

	if(m_View)
		m_View->StopThreads();
}

QSize AuDesignerFrame::sizeHint() const
{
	return QSize(1280, 720);
}

void AuDesignerFrame::Timer()
{
	//qDebug() << "AuDesignerFrame::Timer" << auApp->RunState();
	switch(auApp->RunState())
	{
	case AuApp::NotLoggedIn:
		m_WaitTime.restart();
		return;

	case AuApp::LoggedIn:
		// App is logged in and the login account's player is valid.
		//qDebug() << "AuDesignerFrame::TimerUpdate LoggedIn, intro" << (bool)m_Intro;
		if(m_Intro)
		{
			m_View = CreateViewWidget();

			// Get player unit id from application data. DtClient puts it there when we login.
			const DgData& gd = g_Au.LockDataForRead();
			AuId idPerson = gd.ClientUnitId();
			g_Au.UnlockData();
			g_Au.Settings().writeEntry("/Account/LastPlayerId", idPerson.StringId());

			// Select the client unit as the scene atom. This may cause us to continue waiting until the scene atom arrives.
			m_View->SetSceneAtom(idPerson);
		}
		else if(!m_CommDock && !m_View->SceneSector().IsNull())
		{
			// Splash screen is gone. Button bar has not been created. View is no longer waiting for a scene atom.
			CreateDockWidgets();

			// Slow timer to one second intervals, enough for posting net requests, testing client connection, and view design mode.
			m_Timer->stop();
			m_Timer->start(1000);
		}
		else
		{
			// Check/queue Internet downloads wanted by the scene.
			PostNetRequests();

			// Test the connection to the server to insure we haven't lost the link.
			auApp->TestClientConnection();
		}
		break;

	case AuApp::LoggedInInit:
		// App is logged in but the login account's player has not been initialized yet.
		// This must be the very first time this person has logged in.
		//qDebug() << "AuDesignerFrame::TimerUpdate LoggedInInit";
		{
			// We can't show the accounts player's info until the basic game data has been received.
			const DgData& gd = g_Au.LockDataForRead();
			bool hasData = gd.BasicGameDataReceived();
			g_Au.UnlockData();
			// We only wait 30 seconds to get the basic game data then it is assumed that the
			// connection to the server can not be made and the app shuts down.
			if(!hasData && m_WaitTime.elapsed()/d_MSecPerMin >= 5)
			{
				QString msg = tr("A connection to the game server could not be made. Try again later.");
				QMessageBox::warning(this, g_Au.ApplicationName(), msg);
				exit(0);
			}
			if(hasData)
			{
				m_WaitTime.restart();
				if(m_Intro)
				{
					if(!m_Intro->HasAccountInfo())
						m_Intro->SetAccountInfo();	// Setup the accounts info
				}
			}
			else
			{
				if(m_Intro)
					m_Intro->ShowWaitInfoMsg();
			}
		}
		return;

	case AuApp::LoggedInWait:
		// App is logged in and the player has chosen to join an existing empire or establish their own
		// Note: When logging in with an existing account the app goes straight to the LoggedIn state.
		if(m_Intro)
			m_Intro->EnterNewAvatar();						// Changes app state to InitWait
		return;

	case AuApp::InitWait:
		// App is waiting for a reply to the init information transmitted.
		// Do nothing; keep waiting.
		return;

	case AuApp::Designer:
		if(m_Intro)
		{
			auApp->setOverrideCursor(QCursor(Qt::WaitCursor));
			//qDebug("AuDesignerFrame::Timer creating AuDesignerView for designer");
			m_View = CreateViewWidget();

			// Create a design studio and player unit for local use.
			auApp->InitDesignerMode();

			// Get player unit id from application data. The model puts it there after it creates our scene person unit.
			const DgData& gd = g_Au.LockDataForRead();
			AuId idPerson = gd.ClientUnitId();
			g_Au.UnlockData();

			//qDebug("AuDesignerFrame::Timer - idPerson %d", idPerson.GetID());
			// Select the client unit as the scene atom. This may cause us to continue waiting until the scene atom arrives
			// from the server.
			m_View->SetSceneAtom(idPerson);
		}
		else if(!m_CommDock && !m_View->SceneSector().IsNull())
		{
			// Splash screen is gone. Button bar has not been created. View is no longer waiting for a scene atom.
			CreateDockWidgets();
			DesignerEnableControls();

			// Slow timer to one second intervals, enough for view design mode.
			m_Timer->stop();
			m_Timer->start(1000);
			auApp->restoreOverrideCursor();
		}
		else
		{
			// Check/queue Internet downloads wanted by the scene.
			PostNetRequests();

			// Activate and deactivate avatars
			auApp->SoloDeactivateAvatars();

			// Process solo atom chat message queue.
			auApp->SoloAtomChat();
			g_AuClient.StoryMaintenance();
		}
		break;
	case AuApp::Solitaire:
		if(m_Intro)
		{
			auApp->setOverrideCursor(QCursor(Qt::WaitCursor));
			m_View = CreateViewWidget();

			// Get player unit id from application data. The model puts it there after it creates our scene person unit.
			const DgData& gd = g_Au.LockDataForRead();
			AuId idPerson = gd.ClientUnitId();
			g_Au.UnlockData();
			g_Au.Settings().writeEntry("/Account/LastSoloPlayerId", idPerson.StringId());

			// Select the client unit as the scene atom.
			m_View->SetSceneAtom(idPerson);

			auApp->SceneSyncComplete();
		}
		else if(!m_CommDock && !m_View->SceneSector().IsNull())
		{
			// Splash screen is gone. Button bar has not been created. View is no longer waiting for a scene atom.
			CreateDockWidgets();

			// Slow timer to one second intervals, enough for posting net requests, testing client connection, and view design mode.
			m_Timer->stop();
			m_Timer->start(1000);
			auApp->restoreOverrideCursor();
		}
		else
		{
			// Check/queue Internet downloads wanted by the scene.
			PostNetRequests();

			// Activate and deactivate avatars
			auApp->SoloDeactivateAvatars();

			// Process solo atom chat message queue.
			auApp->SoloAtomChat();
			g_AuClient.StoryMaintenance();
		}
		break;
	}

	// Hide toolbars that should not be visible in the current state.
	if(m_View && !m_View->IsInDesignEnvironment())
		m_View->SetDesignMode(false);	// Turn off design mode in the view.
}

AuGearPos AuDesignerFrame::GetGearSelectionPos() const
{
	if(!m_GearDock || !m_GearDock->isVisible())
		return AuGearPos();
	return m_GearDock->m_Gear->GetSelectionPos();
}

AuView* AuDesignerFrame::View() const
{
	return m_View;
}

AuDesignerView* AuDesignerFrame::ViewWidget() const
{
	return m_View;
}

AuDesignerView* AuDesignerFrame::DesignerView() const
{
	return m_View;
}

void AuDesignerFrame::ShowForum()
{
	// Thanks Qt for making this one so easy.
	QUrl url("https://www.hazeron.com/mybb/");
	QDesktopServices::openUrl(url);
}

void AuDesignerFrame::ShowVideos()
{
	// Thanks Qt for making this one so easy.
	QUrl url("https://www.hazeron.com/videos.php");
	QDesktopServices::openUrl(url);
}

void AuDesignerFrame::ShowWikiDesigner()
{
	ShowWikiTopic("Designer");
}

void AuDesignerFrame::ShowAvatars()
{
	// Thanks Qt for making this one so easy.
	QUrl url("https://www.hazeron.com/EmpireStandings/Avatars.php");
	QDesktopServices::openUrl(url);
}

void AuDesignerFrame::ShowWarDimensionStandings()
{
	// Thanks Qt for making this one so easy.
	QUrl url("https://www.hazeron.com/EmpireStandings/EmpireStandings.php");
	QDesktopServices::openUrl(url);
}

void AuDesignerFrame::ShowPeaceDimensionStandings()
{
	// Thanks Qt for making this one so easy.
	QUrl url("https://www.hazeron.com/EmpireStandingsNZ/EmpireStandings.php");
	QDesktopServices::openUrl(url);
}

void AuDesignerFrame::ToggleConstructionUi()
{
	if(m_ConstructionDock)
	{
		if(m_View && m_View->SceneContext() != SCENE_Universe)
			m_ConstructionDock->hide();
		else if(m_ConstructionDock->isHidden())
			m_ConstructionDock->show();
		else
			m_ConstructionDock->hide();
	}
}

void AuDesignerFrame::ToggleBuildingUi()
{
	if(m_BuildingDock)
	{
		if(m_View && m_View->SceneContext() != SCENE_Universe)
			m_BuildingDock->hide();
		else if(m_BuildingDock->isHidden())
			m_BuildingDock->show();
		else
			m_BuildingDock->hide();
	}
}

void AuDesignerFrame::ToggleSpacecraftUi()
{
	if(m_SpacecraftDock && m_SpacecraftDockFree)
	{
		if(m_View && m_View->SceneContext() == SCENE_Designer)
		{
			m_SpacecraftDock->hide();
			m_SpacecraftDockFree->hide();
		}
		else if(m_SpacecraftDock->isHidden() && m_SpacecraftDockFree->isHidden())
		{
			if(m_View->SceneAtomEmpire() || m_View->SceneAtomIsOfficer())
			{
				m_SpacecraftDock->show();
				m_SpacecraftDockFree->hide();
			}
			else
			{
				m_SpacecraftDock->hide();
				m_SpacecraftDockFree->show();
			}
		}
		else
		{
			m_SpacecraftDock->hide();
			m_SpacecraftDockFree->hide();
		}
	}
}

void AuDesignerFrame::ToggleSpacecraftAccountUi()
{
	if(m_SpacecraftAccountDock)
	{
		if(m_View && m_View->SceneContext() == SCENE_Designer)
			m_SpacecraftAccountDock->hide();
		else if(m_SpacecraftAccountDock->isHidden() && (m_View->SceneAtomEmpire() || m_View->SceneAtomIsOfficer()))
			m_SpacecraftAccountDock->show();
		else
			m_SpacecraftAccountDock->hide();
	}
}

void AuDesignerFrame::ToggleSpacecraftBountiesUi()
{
	if(m_SpacecraftBountiesDock)
	{
		if(m_View && m_View->SceneContext() == SCENE_Designer)
			m_SpacecraftBountiesDock->hide();
		else if(m_SpacecraftBountiesDock->isHidden())
			m_SpacecraftBountiesDock->show();
		else
			m_SpacecraftBountiesDock->hide();
	}
}

void AuDesignerFrame::ToggleSpacecraftDamageUi()
{
	if(m_SpacecraftDamageDock)
	{
		if(m_View && m_View->SceneContext() == SCENE_Designer)
			m_SpacecraftDamageDock->hide();
		else if(m_SpacecraftDamageDock->isHidden())
			m_SpacecraftDamageDock->show();
		else
			m_SpacecraftDamageDock->hide();
	}
}

void AuDesignerFrame::ToggleBdExchangeUi()
{
	if(m_BdExchangeDock)
	{
		if(m_BdExchangeDock->isHidden())
			m_BdExchangeDock->show();
		else
			m_BdExchangeDock->hide();
	}
}

void AuDesignerFrame::ToggleSpacecraftDesignExchangeUi()
{
	if(m_SpacecraftDesignExchangeDock)
	{
		if(m_SpacecraftDesignExchangeDock->isHidden())
			m_SpacecraftDesignExchangeDock->show();
		else
			m_SpacecraftDesignExchangeDock->hide();
	}
}

void AuDesignerFrame::ToggleSpacecraftMissionUi()
{
	if(m_SpacecraftMissionDock)
	{
		if(m_View && m_View->SceneContext() == SCENE_Designer)
			m_SpacecraftMissionDock->hide();
		else if(m_SpacecraftMissionDock->isHidden() && (m_View->SceneAtomEmpire() || m_View->SceneAtomIsOfficer()))
			m_SpacecraftMissionDock->show();
		else
			m_SpacecraftMissionDock->hide();
	}
}

void AuDesignerFrame::ToggleSpacecraftPolicyUi()
{
	if(m_SpacecraftPolicyDock)
	{
		if(m_View && m_View->SceneContext() == SCENE_Designer)
			m_SpacecraftPolicyDock->hide();
		else if(m_SpacecraftPolicyDock->isHidden() && (m_View->SceneAtomEmpire() || m_View->SceneAtomIsOfficer()))
			m_SpacecraftPolicyDock->show();
		else
			m_SpacecraftPolicyDock->hide();
	}
}

void AuDesignerFrame::ToggleSpacecraftRosterUi()
{
	if(m_SpacecraftRosterDock)
	{
		if(m_View && m_View->SceneContext() == SCENE_Designer)
			m_SpacecraftRosterDock->hide();
		else if(m_SpacecraftRosterDock->isHidden())
			m_SpacecraftRosterDock->show();
		else
			m_SpacecraftRosterDock->hide();
	}
}

void AuDesignerFrame::ToggleSpacecraftShipmentsUi()
{
	if(m_SpacecraftShipmentsDock)
	{
		if(m_View && m_View->SceneContext() == SCENE_Designer)
			m_SpacecraftShipmentsDock->hide();
		else if(m_SpacecraftShipmentsDock->isHidden())
			m_SpacecraftShipmentsDock->show();
		else
			m_SpacecraftShipmentsDock->hide();
	}
}

void AuDesignerFrame::ToggleSpacecraftSpecsUi()
{
	if(m_SpacecraftSpecsDock)
	{
		if(m_View && m_View->SceneContext() == SCENE_Designer)
			m_SpacecraftSpecsDock->hide();
		else if(m_SpacecraftSpecsDock->isHidden())
			m_SpacecraftSpecsDock->show();
		else
			m_SpacecraftSpecsDock->hide();
	}
}

void AuDesignerFrame::ToggleLocatorUi()
{
	if(m_LocatorDock)
	{
		if(m_View && m_View->SceneContext() != SCENE_Universe)
			m_LocatorDock->hide();
		else if(m_LocatorDock->isHidden())
			m_LocatorDock->show();
		else
			m_LocatorDock->hide();
	}
}

void AuDesignerFrame::ToggleLocatorCompassUi()
{
	if(m_LocatorCompassDock)
	{
		if(m_View && m_View->SceneContext() != SCENE_Universe)
			m_LocatorCompassDock->hide();
		else if(m_LocatorCompassDock->isHidden())
			m_LocatorCompassDock->show();
		else
			m_LocatorCompassDock->hide();
	}
}

void AuDesignerFrame::ToggleLocatorGPSUi()
{
	if(m_LocatorGPSDock)
	{
		if(m_View && m_View->SceneContext() != SCENE_Universe)
			m_LocatorGPSDock->hide();
		else if(m_LocatorGPSDock->isHidden())
			m_LocatorGPSDock->show();
		else
			m_LocatorGPSDock->hide();
	}
}

void AuDesignerFrame::ToggleLocatorStarMapUi()
{
	if(m_LocatorStarMapDock)
	{
		if(m_View && m_View->SceneContext() != SCENE_Universe)
			m_LocatorStarMapDock->hide();
		else if(m_LocatorStarMapDock->isHidden())
			m_LocatorStarMapDock->show();
		else
			m_LocatorStarMapDock->hide();
	}
}

void AuDesignerFrame::ToggleLocatorSystemScanUi()
{
	if(m_LocatorSystemScanDock)
	{
		if(m_View && m_View->SceneContext() != SCENE_Universe)
			m_LocatorSystemScanDock->hide();
		else if(m_LocatorSystemScanDock->isHidden())
			m_LocatorSystemScanDock->show();
		else
			m_LocatorSystemScanDock->hide();
	}
}

void AuDesignerFrame::ToggleLocatorWorldMapUi()
{
	if(m_LocatorWorldMapDock)
	{
		if(m_View && m_View->SceneContext() != SCENE_Universe)
			m_LocatorWorldMapDock->hide();
		else if(m_LocatorWorldMapDock->isHidden())
			m_LocatorWorldMapDock->show();
		else
			m_LocatorWorldMapDock->hide();
	}
}

void AuDesignerFrame::ToggleCommUi()
{
	if(m_CommDock)
	{
		if(m_CommDock->isHidden())
			m_CommDock->show();
		else
			m_CommDock->hide();
	}
}

void AuDesignerFrame::ToggleOrdersUi()
{
	if(m_OrdersDock)
	{
		if(m_View && m_View->SceneContext() == SCENE_Designer)
			m_OrdersDock->hide();
		else if(m_OrdersDock->isHidden() && (m_View->SceneAtomEmpire() || m_View->SceneAtomIsOfficer()))
			m_OrdersDock->show();
		else
			m_OrdersDock->hide();
	}
}

void AuDesignerFrame::ToggleGovernUi()
{
	if(m_GovernDock)
	{
		if(m_GovernDock->isHidden() && m_View->SceneAtomEmpire())
			m_GovernDock->show();
		else
			m_GovernDock->hide();
	}
}

void AuDesignerFrame::ToggleGovernAnnouncementsUi()
{
	if(m_GovernAnnouncementsDock)
	{
		if(m_GovernAnnouncementsDock->isHidden() && m_View->SceneAtomEmpire())
			m_GovernAnnouncementsDock->show();
		else
			m_GovernAnnouncementsDock->hide();
	}
}

void AuDesignerFrame::ToggleGovernCitiesUi()
{
	if(m_GovernCitiesDock)
	{
		if(m_GovernCitiesDock->isHidden() && m_View->SceneAtomEmpire())
			m_GovernCitiesDock->show();
		else
			m_GovernCitiesDock->hide();
	}
}

void AuDesignerFrame::ToggleGovernCitizensUi()
{
	if(m_GovernCitizensDock)
	{
		if(m_GovernCitizensDock->isHidden() && m_View->SceneAtomEmpire())
			m_GovernCitizensDock->show();
		else
			m_GovernCitizensDock->hide();
	}
}

void AuDesignerFrame::ToggleGovernCommerceUi()
{
	if(m_GovernCommerceDock)
	{
		if(m_GovernCommerceDock->isHidden() && m_View->SceneAtomEmpire())
			m_GovernCommerceDock->show();
		else
			m_GovernCommerceDock->hide();
	}
}

void AuDesignerFrame::ToggleGovernDiplomacyUi()
{
	if(m_GovernDiplomacyDock)
	{
		if(m_GovernDiplomacyDock->isHidden() && m_View->SceneAtomEmpire())
			m_GovernDiplomacyDock->show();
		else
			m_GovernDiplomacyDock->hide();
	}
}

void AuDesignerFrame::ToggleGovernPatentsUi()
{
	if(m_GovernPatentsDock)
	{
		if(m_GovernPatentsDock->isHidden() && m_View->SceneAtomEmpire())
			m_GovernPatentsDock->show();
		else
			m_GovernPatentsDock->hide();
	}
}

void AuDesignerFrame::ToggleGovernPolicyUi()
{
	if(m_GovernPolicyDock)
	{
		if(m_GovernPolicyDock->isHidden() && m_View->SceneAtomEmpire())
			m_GovernPolicyDock->show();
		else
			m_GovernPolicyDock->hide();
	}
}

void AuDesignerFrame::ToggleGovernTaxesUi()
{
	if(m_GovernTaxesDock)
	{
		if(m_GovernTaxesDock->isHidden() && m_View->SceneAtomEmpire())
			m_GovernTaxesDock->show();
		else
			m_GovernTaxesDock->hide();
	}
}

void AuDesignerFrame::ToggleSettingsUi()
{
	// Settings window can be created at login before view window exists. Watch for the change. We have to recreate the dialog then.
	if(m_Settings && m_Settings->isVisible())
	{
		m_Settings->hide();
	}
	else
	{
		// If settings exists here, it was created with the wrong parent. Close it and create a new one.
		if(m_Settings)
			m_Settings->close();

		if(m_View)
		{
			m_Settings = new AuSettingsDialog(true, this);	// Show Shift+F12 tip on title.
			m_Settings->SetOpenGLInfo(m_View->OpenGLTextureUnits(), m_View->OpenGLUniformLocs());
			connect(m_Settings, SIGNAL(ControlsChanged()), SLOT(ControlsChanged()));
			connect(m_Settings, SIGNAL(ControlsChanged()), m_View, SLOT(ControlsChanged()));
			connect(m_Settings, SIGNAL(MessageLimitChanged(int)), m_View, SLOT(SetNumberOfMessages(int)));
			connect(m_Settings, SIGNAL(SoundGainChanged(int)), auApp, SLOT(SoundGainChanged(int)));
			connect(m_Settings, SIGNAL(SoundOverrideChanged(int)), auApp, SLOT(SoundOverrideChanged(int)));
		}
		else
		{
			m_Settings = new AuSettingsDialog(false, this);		// Do not show Shift+F12 tip on title.
			connect(m_Settings, SIGNAL(ControlsChanged()), SLOT(ControlsChanged()));
			connect(m_Settings, SIGNAL(SoundGainChanged(int)), auApp, SLOT(SoundGainChanged(int)));
			connect(m_Settings, SIGNAL(SoundOverrideChanged(int)), auApp, SLOT(SoundOverrideChanged(int)));
		}
		m_Settings->show();
	}
}

void AuDesignerFrame::CloseSettingsUi()
{
	if(m_Settings && !m_Settings->isHidden())
		m_Settings->hide();
}

void AuDesignerFrame::ToggleCharacterUi()
{
	if(m_BioDock)
	{
		if(m_BioDock->isHidden())
			m_BioDock->show();
		else
			m_BioDock->hide();
	}
}

void AuDesignerFrame::ToggleDesignPartsUi()
{
	if(m_View && m_View->SceneContext() == SCENE_Designer)
	{
		if(m_DesignerPartsDock)
		{
			if(m_DesignerPartsDock->isHidden())
				m_DesignerPartsDock->show();
			else
				m_DesignerPartsDock->hide();
		}
	}
	else
	{
		if(m_DesignerPartsDock)
			m_DesignerPartsDock->hide();
	}
}

void AuDesignerFrame::ToggleDesignTextureWrapUi()
{
	if(m_View && m_View->SceneContext() == SCENE_Designer)
	{
		if(m_DesignerTextureWrapDock)
		{
			if(m_DesignerTextureWrapDock->isHidden())
				m_DesignerTextureWrapDock->show();
			else
				m_DesignerTextureWrapDock->hide();
		}
	}
	else
	{
		if(m_DesignerTextureWrapDock)
			m_DesignerTextureWrapDock->hide();
	}
}

void AuDesignerFrame::ToggleHoldUi()
{
	if(m_HoldDock && m_HoldDockFree)
	{
		if(m_View && m_View->SceneContext() == SCENE_Designer)
		{
			m_HoldDock->hide();
			m_HoldDockFree->hide();
		}
		else if(m_HoldDock->isHidden() && m_HoldDockFree->isHidden())
		{
			if(m_View->SceneAtomEmpire() || m_View->SceneAtomIsOfficer())
			{
				m_HoldDock->show();
				m_HoldDockFree->hide();
			}
			else
			{
				m_HoldDock->hide();
				m_HoldDockFree->show();
			}
		}
		else
		{
			m_HoldDock->hide();
			m_HoldDockFree->hide();
		}
	}
}

void AuDesignerFrame::ToggleCargoHoldUi()
{
	if(m_CargoHoldDock)
	{
		if(m_View && m_View->SceneContext() == SCENE_Designer)
			m_CargoHoldDock->hide();
		else if(m_CargoHoldDock->isHidden())
			m_CargoHoldDock->show();
		else
			m_CargoHoldDock->hide();
	}
}

void AuDesignerFrame::ToggleCargoManifestUi()
{
	if(m_CargoManifestDock)
	{
		if(m_View && m_View->SceneContext() == SCENE_Designer)
			m_CargoManifestDock->hide();
		else if(m_CargoManifestDock->isHidden() && (m_View->SceneAtomEmpire() || m_View->SceneAtomIsOfficer()))
			m_CargoManifestDock->show();
		else
			m_CargoManifestDock->hide();
	}
}

void AuDesignerFrame::ToggleCargoOrdnanceUi()
{
	if(m_CargoOrdnanceDock)
	{
		if(m_View && m_View->SceneContext() == SCENE_Designer)
			m_CargoOrdnanceDock->hide();
		else if(m_CargoOrdnanceDock->isHidden())
			m_CargoOrdnanceDock->show();
		else
			m_CargoOrdnanceDock->hide();
	}
}

void AuDesignerFrame::ToggleCargoTransferUi()
{
	if(m_CargoTransferDock)
	{
		if(m_View && m_View->SceneContext() == SCENE_Designer)
			m_CargoTransferDock->hide();
		else if(m_CargoTransferDock->isHidden())
			m_CargoTransferDock->show();
		else
			m_CargoTransferDock->hide();
	}
}

void AuDesignerFrame::ToggleGearUi()
{
	if(m_GearDock)
	{
		if(m_GearDock->isHidden())
			m_GearDock->show();
		else
			m_GearDock->hide();
	}
}

void AuDesignerFrame::ToggleHandCraftUi()
{
	if(m_HandCraftDlg)
	{
		if(m_View && m_View->SceneContext() != SCENE_Universe)
			m_HandCraftDlg->hide();
		else if(m_HandCraftDlg->isHidden())
			m_HandCraftDlg->show();
		else
			m_HandCraftDlg->hide();
	}
	else if(m_View)
	{
		if(m_View->SceneContext() == SCENE_Universe)
		{
			m_HandCraftDlg = new AuHandCraftDialog(this);
			m_HandCraftDlg->show();
		}
	}
}

void AuDesignerFrame::ToggleAllUis()
{
	// Determine what state we want them to be in.
	if(m_DockBars.AnyVisible())
	{
		// At least one toolbar is visible.
		m_DockBars.SaveVisibilityStates();

		// Remember their visibility states so we can turn them all back on.
		m_DockBars.UpdateVisibilityStates();

		// Hide all the ones that are up.
		m_DockBars.HideAll();
	}
	else
	{
		// No toolbar is visible so we want them back the way they were.
		m_DockBars.RestoreVisibilityStates();

		// Hide all the ones that were up.
		m_DockBars.ShowWereVisible();
	}
}

void AuDesignerFrame::ToggleAgentUi()
{
	const DgData& gd = g_Au.LockDataForRead();
	bool isAgent = gd.ClientPlayerIsAgent();
	bool isMinion = gd.ClientPlayerIsMinion();
	g_Au.UnlockData();

	if(m_Agent)
	{
		if(m_Agent->isHidden() && (isAgent || isMinion))
			m_Agent->show();
		else
			m_Agent->hide();
	}
	else if(m_View && (isAgent || isMinion))
	{
		m_Agent = new AuAgentDialog(this);
		connect(m_View, SIGNAL(SceneAtomChanged()), m_Agent, SLOT(SceneAtomChanged()));
		m_Agent->show();
	}
}

void AuDesignerFrame::TogglePlayersOnlineUi()
{
	if(m_PlayersOnlineDock)
	{
		if(m_View && m_View->SceneContext() != SCENE_Universe)
			m_PlayersOnlineDock->hide();
		else if(m_PlayersOnlineDock->isHidden())
			m_PlayersOnlineDock->show();
		else
			m_PlayersOnlineDock->hide();
	}
}

void AuDesignerFrame::ToggleMailUi()
{
	// Single user designer mode does not get to see/use the mail ui.
	if(auApp->IsSingleUserDesignerMode())
		return;

	if(m_MailDock)
	{
		if(m_MailDock->isHidden())
			m_MailDock->show();
		else
			m_MailDock->hide();
	}
}

// A channel open request has arrived from the server.
void AuDesignerFrame::ChannelOpened(const QString& channelName, AuCommStdChannel::SCOPE scope, const QString& narrDesc)
{
	// The new channel tab is not pushed to the top so creating a new channel
	// can't be used as an offensive tactic against someone. The tab just
	// appears quietly.
	if(m_CommDock)
		m_CommDock->m_Comm->CreateChannel(channelName, scope, false, narrDesc);
}

void AuDesignerFrame::ChatReceived(AuCommPacket& msg)
{
	if(m_CommDock)
		m_CommDock->m_Comm->ChatReceived(msg);
	else
	{
		//qDebug() << "AuDesignerFrame::ChatReceived early arrival" << msg.Text();
		m_EarlyArrivals.append(msg.Clone());
		return;
	}

	if(m_View)
	{
		// Trap "Spot Target" messages from the local scene atom. They were
		// already sent to the view in AuCommChannel::SendChat.
		if(msg.Type() == AuCommPacket::SpotTargetOrTerrain)
		{
			const DgData& gd = g_Au.LockDataForRead();
			AuId sceneAtomId = gd.ClientUnitId();
			g_Au.UnlockData();
			if(sceneAtomId != msg.FromId())
				m_View->ChatReceived(&msg);
		}
		else
			m_View->ChatReceived(&msg);
	}

	if(msg.StoryInstance() > 1 && g_AuClient.ShowStoryDialog())
	{
		// Story messages appear on a story dialog.
		QHash<quint32, AuStoryDialog*>::iterator di = m_StoryDialogs.find(msg.StoryInstance());
		if(di == m_StoryDialogs.end())
		{
			AuStoryDialog* sdlg = new AuStoryDialog(this);
			connect(sdlg, SIGNAL(ChoicePicked(quint32,qint8,AuId)), auApp, SLOT(ChoicePicked(quint32,qint8,AuId)));
			connect(sdlg, SIGNAL(HintRequested(quint32)), auApp, SLOT(StoryHint(quint32)));
			connect(this, SIGNAL(StoryChoicePickedCleanup(quint32,qint8,AuId)),
					sdlg, SLOT(ChoicePickedCleanup(quint32,qint8,AuId)));

			di = m_StoryDialogs.insert(msg.StoryInstance(), sdlg);
		}

		if(!(*di)->isVisible())
			(*di)->show();

		(*di)->MessageReceived(msg);
	}
}

void AuDesignerFrame::ListOpenChannels(QStringList& channelNames) const
{
	if(m_CommDock)
		m_CommDock->m_Comm->ListOpenChannels(channelNames);
}

void AuDesignerFrame::closeEvent(QCloseEvent* event)
{
	// Qt example code does not depict them calling this base class implementation. I am guessing it just accepts the event.
	if(m_View && !m_View->DesignerConfirmLoseChanges())
	{
		event->ignore();
		return;
	}

	//qDebug() << "AuDesignerFrame::closeEvent";
	event->accept();

	auApp->TerminateSound();
	StopThreads();

	g_Au.Settings().setValue("/Widget/MainFrame/Geometry", saveGeometry());
	if(auApp->RunState() == AuApp::Designer)
	{
		if(m_CommDock)
			g_Au.Settings().setValue("/Widget/MainFrame/DesignerWindowState", saveState());
		g_Au.Settings().setValue("/Widget/MainFrame/DesignerFullScreen", isFullScreen());
	}
	else if(auApp->RunState() == AuApp::Solitaire)
	{
		if(m_CommDock)
			g_Au.Settings().setValue("/Widget/MainFrame/SoloWindowState", saveState());
		g_Au.Settings().setValue("/Widget/MainFrame/SoloFullScreen", isFullScreen());
	}
	else
	{
		if(m_CommDock)
			g_Au.Settings().setValue("/Widget/MainFrame/WindowState1", saveState());
		g_Au.Settings().setValue("/Widget/MainFrame/FullScreen", isFullScreen());
	}

	if(m_Agent)
		m_Agent->close();
	if(m_Settings)
		m_Settings->close();

	// Store the visibility states of all our docking toolbars if any are currently visible.
	if(m_DockBars.AnyVisible())
	{
		// At least one toolbar is visible.
		m_DockBars.SaveVisibilityStates();
		m_DockBars.CloseAll();
	}

	AuFrame::closeEvent(event);
}

void AuDesignerFrame::JobAvailable(AuJobOffer job)
{
	// Locate the jobs dialog if it is up. This findChild call will find it, no matter which child widget has displayed it.
	AuJobsQueryDialog* jobs = findChild<AuJobsQueryDialog*>("Jobs");
	if(jobs)
		jobs->JobAvailable(job);
}

void AuDesignerFrame::ToggleFullScreen()
{
	//qDebug() << "AuDesignerFrame::ToggleFullScreen isFullScreen" << isFullScreen();
	if(m_View)
		m_View->PauseRendering();

	if(isFullScreen())
		showNormal();
	else
		showFullScreen();
}

void AuDesignerFrame::EmpireDataBegin()
{
	//qDebug() << "AuDesignerFrame::EmpireDataBegin";
	m_muEmpireDataState.lock();
	m_EmpireDataState = 1;
	m_muEmpireDataState.unlock();
}

void AuDesignerFrame::EmpireDataComplete()
{
	//qDebug() << "AuDesignerFrame::EmpireDataComplete";
	m_muEmpireDataState.lock();
	m_EmpireDataState = 2;
	m_muEmpireDataState.unlock();
	if(m_Agent)
		m_Agent->EmpireDataComplete();
	if(m_Intro)
		m_Intro->EmpireDataReceived();
	if(m_LocatorDock)
		m_LocatorDock->m_Locator->EmpireDataChanged();
	if(m_LocatorStarMapDock)
		m_LocatorStarMapDock->m_Content->EmpireDataChanged();
	if(m_GovernDock)
		m_GovernDock->m_Govern->UpdateDatabase();
	if(m_GovernAnnouncementsDock)
		m_GovernAnnouncementsDock->m_Content->UpdateEmpireRecord();
	if(m_GovernCitiesDock)
		m_GovernCitiesDock->m_Content->UpdateEmpireRecord();
	if(m_GovernCitizensDock)
		m_GovernCitizensDock->m_Content->UpdateEmpireRecord();
	if(m_GovernCommerceDock)
		m_GovernCommerceDock->m_Content->UpdateEmpireRecord();
	if(m_GovernDiplomacyDock)
		m_GovernDiplomacyDock->m_Content->EmpireDataComplete();
	if(m_GovernPatentsDock)
		m_GovernPatentsDock->m_Content->UpdateEmpireRecord();
	if(m_GovernPolicyDock)
		m_GovernPolicyDock->m_Content->UpdateEmpireRecord();
	if(m_GovernTaxesDock)
		m_GovernTaxesDock->m_Content->UpdateEmpireRecord();
}

bool AuDesignerFrame::EmpireDataIncoming() const
{
	m_muEmpireDataState.lock();
	bool empireDataIncoming = (m_EmpireDataState == 1);
	m_muEmpireDataState.unlock();
	return empireDataIncoming;
}

bool AuDesignerFrame::GetAndResetEmpireDataReceived()
{
	m_muEmpireDataState.lock();
	bool empireDataReceived = (m_EmpireDataState == 2);
	if(empireDataReceived)
		m_EmpireDataState = 0;
	m_muEmpireDataState.unlock();
	return empireDataReceived;
}

void AuDesignerFrame::SendChat(AuCommPacket& msg)
{
	if(m_CommDock)
		m_CommDock->m_Comm->SendChat(msg);
}

QString AuDesignerFrame::OrdersChannel() const
{
	if(m_OrdersDock)
		return m_OrdersDock->m_UnitOrders->OrderChannel();
	else
		return AuCommStdChannel::CrewChannelName();
}

void AuDesignerFrame::HelpRequestAccepted(AuId reqId, QString reqName)
{
	const DgData& gd = g_Au.LockDataForRead();
	bool isAgent = gd.ClientPlayerIsAgent();
	g_Au.UnlockData();
	if(isAgent)
	{
		// Display the admin ui so the admin can decide about teleporting to the requestor with all options available.
		if(!m_Agent || m_Agent->isHidden())
			ToggleAgentUi();

		if(m_Agent)
			m_Agent->SetGoToDestination(reqId, reqName, 0);	// setup go to character destination
	}
}

void AuDesignerFrame::GameSelect(quint8 gameType, bool gameSelected)
{
	if(m_View)
		m_View->GameSelect(gameType, gameSelected);
}

void AuDesignerFrame::GameTurn(AuId personsTurn)
{
	if(m_View)
		m_View->GameTurn(personsTurn);
}

void AuDesignerFrame::GameOver(quint8 gameType, QByteArray gameResult)
{
	if(m_View)
		m_View->GameOver(gameType, gameResult);
}

void AuDesignerFrame::ShowCitiesOnStarMap(AuIdList cityIds)
{
	if(m_LocatorStarMapDock)
		m_LocatorStarMapDock->m_Content->StarMapShowCities(cityIds);

	if(m_LocatorDock)
		m_LocatorDock->m_Locator->StarMapShowCities(cityIds);
}

void AuDesignerFrame::ShowGalaxyLocOnStarMap(QString galaxyName, AuPoint loc)
{
	if(m_LocatorStarMapDock)
		m_LocatorStarMapDock->m_Content->StarMapShowGalaxyLocation(galaxyName, loc);

	if(m_LocatorDock)
		m_LocatorDock->m_Locator->StarMapShowGalaxyLocation(galaxyName, loc);
}

void AuDesignerFrame::SurrenderCities(AuIdList cityIds, QStringList cityNames, quint32 toEmpire)
{
	if(!m_View)
		return;
	if(!m_View->SceneAtomEmpire())
		return;

	AuCommCitySurrenderOffered msg(m_View->SceneAtomEmpire(), toEmpire, cityIds, cityNames,
			AuCommStdChannel::DiplomacyChannelName(), m_View->SceneAtomId(), m_View->SceneAtomName(), m_View->SceneSector(),
			m_View->SceneGalaxy(), AuCommStdChannel::Universe, AuCommStdChannel::Diplomacy);
	SendChat(msg);
}

void AuDesignerFrame::HangarQueryAnswered(QString shipDesc, quint32 shipDesign, quint32 shipsInHangar)
{
	AuBldHangar* bdhangar = findChild<AuBldHangar*>("BldHangar");
	if(bdhangar)
		bdhangar->QueryAnswered(shipDesc, shipDesign, shipsInHangar);
}

void AuDesignerFrame::TransporterPlace(AuTransporterTarget tt)
{
	// Locate the transporter options dialog if it is up. This findChild call will find it, no matter which child widget has
	// displayed it.
	AuBeamOptions* beamOptions = findChild<AuBeamOptions*>("BeamOptions");
	if(beamOptions)
		beamOptions->TransporterPlace(tt);
	else
	{
		AuBeamVehicleOptions* beamVehicleOptions = findChild<AuBeamVehicleOptions*>("BeamVehicleOptions");
		if(beamVehicleOptions)
			beamVehicleOptions->TransporterPlace(tt);
	}
}

void AuDesignerFrame::TransporterShip(AuId id, QString name, QString desc)
{
	// Locate the transporter options dialog if it is up. This findChild call will find it, no matter which child widget has
	// displayed it.
	AuBeamOptions* beamOptions = findChild<AuBeamOptions*>("BeamOptions");
	if(beamOptions)
		beamOptions->TransporterShip(id, name, desc);
	else
	{
		AuBeamVehicleOptions* beamVehicleOptions = findChild<AuBeamVehicleOptions*>("BeamVehicleOptions");
		if(beamVehicleOptions)
			beamVehicleOptions->TransporterShip(id, name, desc);
	}
}

void AuDesignerFrame::TransporterTransporter(AuId id, QString name, QString desc, quint16 whichTransporter)
{
	// Locate the transporter options dialog if it is up. This findChild call will find it, no matter which child widget has
	// displayed it.
	AuBeamOptions* beamOptions = findChild<AuBeamOptions*>("BeamOptions");
	if(beamOptions)
		beamOptions->TransporterTransporter(id, name, desc, whichTransporter);
	else
	{
		AuBeamVehicleOptions* beamVehicleOptions = findChild<AuBeamVehicleOptions*>("BeamVehicleOptions");
		if(beamVehicleOptions)
			beamVehicleOptions->TransporterTransporter(id, name, desc, whichTransporter);
	}
}

void AuDesignerFrame::StarMapUpdateAllComplete()
{
	if(m_LocatorDock)
		m_LocatorDock->m_Locator->StarMapUpdateAllComplete();
	if(m_LocatorStarMapDock)
		m_LocatorStarMapDock->m_Content->StarMapUpdateAllComplete();
}

void AuDesignerFrame::StarMapUpdateGalaxyComplete(int galaxy)
{
	if(m_LocatorDock)
		m_LocatorDock->m_Locator->StarMapUpdateGalaxyComplete((GALAXY)galaxy);
	if(m_LocatorStarMapDock)
		m_LocatorStarMapDock->m_Content->StarMapUpdateGalaxyComplete((GALAXY)galaxy);
}

void AuDesignerFrame::StarMapUpdateSectorComplete(int galaxy, AuIdList sectorIds)
{
	if(m_LocatorDock)
		m_LocatorDock->m_Locator->StarMapUpdateSectorComplete((GALAXY)galaxy, sectorIds);
	if(m_LocatorStarMapDock)
		m_LocatorStarMapDock->m_Content->StarMapUpdateSectorComplete((GALAXY)galaxy, sectorIds);
}

void AuDesignerFrame::StarMapUpdateAllStarted()
{
	if(m_LocatorDock)
		m_LocatorDock->m_Locator->StarMapUpdateAllStarted();
	if(m_LocatorStarMapDock)
		m_LocatorStarMapDock->m_Content->StarMapUpdateAllStarted();
}

void AuDesignerFrame::StarMapUpdateGalaxyStarted(int galaxy)
{
	if(m_LocatorDock)
		m_LocatorDock->m_Locator->StarMapUpdateGalaxyStarted((GALAXY)galaxy);
	if(m_LocatorStarMapDock)
		m_LocatorStarMapDock->m_Content->StarMapUpdateGalaxyStarted((GALAXY)galaxy);
}

void AuDesignerFrame::StarMapUpdateSectorStarted(AuId sector, unsigned int range)
{
	if(m_LocatorDock)
		m_LocatorDock->m_Locator->StarMapUpdateSectorStarted(sector, range);
	if(m_LocatorStarMapDock)
		m_LocatorStarMapDock->m_Content->StarMapUpdateSectorStarted(sector, range);
}

void AuDesignerFrame::StarMapErased()
{
	if(m_LocatorDock)
		m_LocatorDock->m_Locator->StarMapErased();
	if(m_LocatorStarMapDock)
		m_LocatorStarMapDock->m_Content->StarMapErased();
}

void AuDesignerFrame::StarMapAddSector(AuId sectorId)
{
	// This function does not replace the sector if it already exists.
	if(g_AuClient.StarMapHasSector(sectorId))
		return;

	quint32 empireId = g_AuClient.StarMapEmpire();

	AuEther& ether = g_Au.LockEtheriumDelete();
	const DaSector* sector = dynamic_cast<const DaSector*>(ether.FindNormalAtom(sectorId));
	if(sector)
		StarMapAddSector(sector, empireId);
	g_Au.UnlockEtheriumDelete();
}

void AuDesignerFrame::StarMapInit(quint32 empireId)
{
	g_AuClient.StarMapInit(empireId);

	DaAtomList galaxies;
	AuEther& ether = g_Au.LockEtheriumDelete();
	ether.ListGalaxies(galaxies);
	for(DaAtomList::const_iterator gi = galaxies.cbegin(); gi != galaxies.cend(); gi++)
	{
		const DaGalaxy* galaxy = dynamic_cast<const DaGalaxy*>(*gi);
		if(!galaxy)
			continue;

		DaAtomList subs;
		galaxy->ListSubatoms(subs);
		for(DaAtomList::const_iterator si = subs.cbegin(); si != subs.cend(); si++)
		{
			const DaSector* sector = dynamic_cast<const DaSector*>(*si);
			if(sector)
				StarMapAddSector(sector, empireId);
		}
	}
	g_Au.UnlockEtheriumDelete();
}

void AuDesignerFrame::StarMapAddSector(const DaSector* sector, quint32 empireId)
{
	if(g_AuClient.StarMapHasSector(sector->Id()))
		return;

	// We have received a sector and it is not in the star map.
	// Build a sector object for the star map.
	AuStarMapSector gvsector;
	sector->StarMapInfo(empireId, gvsector);

	// Fill the sector record with information about its solar systems.
	DaAtomList subs;
	sector->ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator si = subs.cbegin(); si != subs.cend(); si++)
	{
		const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(*si);
		if(!system)
			continue;

		// Create a system record for the viewer.
		AuStarMapSystem gvsystem;
		system->StarMapInfo(empireId, gvsystem);

		if(gvsystem.IsExplored())
		{
			// Fill the system record with information about all cities and spacecraft in explored systems.
			// We may suppress this information if there is no way to get current intel on the system.
			// Master server does not load city atoms but it does have empire records. It's painful but we can search them.
			bool ecity = false;     // true if any empire city in the system
			bool eradar = false;    // true if any empire radar in the system
			bool enearby = false;   // true if any empire city with an airport or air base is near the system
			const DgData& gd = g_Au.LockDataForRead();
			for(DgEmpireHash::const_iterator ei = gd.m_gEmpires.begin(); ei != gd.m_gEmpires.end(); ei++)
			{
				for(AuCityHash::const_iterator ci = (*ei).second.m_Cities->cbegin(); ci != (*ei).second.m_Cities->cend(); ci++)
				{
					if((*ci).second.SystemId() == system->Id())
					{
						// City is in the system.
						// Create a city record for the viewer.
						AuStarMapCity gvcity;
						gvcity.m_Name = (*ci).second.CityName();
						gvcity.m_Empire = (*ei).second.m_id;
						if((*ci).second.Facilities().HasAirBase())
							gvcity.SetAirBase();
						if((*ci).second.Facilities().HasAirport())
							gvcity.SetAirport();
						if((*ci).second.Facilities().HasBaseHQ())
							gvcity.SetBaseHeadquarters();
						if((*ci).second.Facilities().HasCapitol())
							gvcity.SetCapitolBuilding();
						if((*ci).first == (*ei).second.m_capitol)
							gvcity.SetEmpireCapital();
						if((*ci).second.IsGalaxyCapital())
							gvcity.SetGalaxyCapital();
						if((*ci).second.IsSectorCapital())
							gvcity.SetSectorCapital();
						gvsystem.m_Cities.insert(pair<AuId, AuStarMapCity>((*ci).first, gvcity));

						if((*ei).first == empireId)
						{
							ecity = true;
							if(!eradar &&
									((*ci).second.Facilities().HasAirportRadar() || (*ci).second.Facilities().HasBaseRadar()))
								eradar = true;
						}
					}
					else if(!enearby)
					{
						// City is not in the system and we haven't found a city nearby yet.
						if((*ei).first != empireId)
							continue;	// City is not friendly enough to provide us with information.
						if(!(*ci).second.Facilities().HasAirBase() && !(*ci).second.Facilities().HasAirport())
							continue;	// City has no facilities for space travellers.
						// Check distance to the city.
						if((*ci).second.Galaxy() != system->Galaxy())
							continue;	// City is in a distant galaxy.
						double dist = (*ci).second.SystemLoc().Distance(sector->Location() + system->Location());	// in pc
						// The diagonal distance across a sector is 17.32 = √(10² + 10² + 10²) parsecs.
						if(dist <= 17.32)
							enearby = true;
					}
				}
			}
			g_Au.UnlockData();

			if(!eradar && !ecity && !enearby)
			{
				// No radars, cities, or non-new ships in the system; no spaceports nearby; scene atom is not in the system.
				// We cannot know about cities that are there, just because the system was explored once.
				gvsystem.m_Cities.clear();
			}
		}

		gvsector.m_Systems.insert(pair<AuId, AuStarMapSystem>(system->Id(), gvsystem));
	}
	g_AuClient.StarMapAddSector(sector->Id(), gvsector);
	if(m_LocatorDock)
		m_LocatorDock->m_Locator->StarMapSectorAdded(sector->Id());
	if(m_LocatorStarMapDock)
		m_LocatorStarMapDock->m_Content->StarMapSectorAdded(sector->Id());
}

void AuDesignerFrame::ClientUnitEmpireReceived(quint32 empireId)
{
	// Now that we have the scene atom's empire, we can init the star map using local sector atoms.
	if(g_AuClient.StarMapIsEmpty())
		StarMapInit(empireId);
}

void AuDesignerFrame::CreateDockWidgets()
{
	//qDebug() << "AuDesignerFrame::CreateDockWidgets";
	Q_ASSERT(m_View);

	// Designer input bar
	m_DesignerBar = new AuDesignerBar(this);
	addToolBar(Qt::TopToolBarArea, m_DesignerBar);
	connect(m_DesignerBar, SIGNAL(CoordinateEntered(QString, int)), m_View, SLOT(CoordinateEntered(QString, int)));
	connect(m_View, SIGNAL(DesignerCommandPrompt(QString)), m_DesignerBar, SLOT(SetInputPrompt(QString)));

	// Designer helper bar
	m_DesignerBar2 = new AuDesignerBar2(this);
	addToolBar(Qt::TopToolBarArea, m_DesignerBar2);
	connect(m_View, SIGNAL(DesignerAnchorChanged(QString)), m_DesignerBar2, SLOT(SetAnchorDesc(QString)));

	// Communicator opens all previously opened channels.
	m_CommDock = new AuCommDock(this);
	connect(m_CommDock->m_Comm, SIGNAL(ChoicePicked(quint32, qint8, AuId)), auApp, SLOT(ChoicePicked(quint32, qint8, AuId)));
	connect(m_View, SIGNAL(ChatReleased()), m_CommDock->m_Comm, SLOT(FocusChatInput()));
	connect(m_View, SIGNAL(SpotTargetOrTerrain(QString, int, bool, bool)),
			m_CommDock->m_Comm, SLOT(SpotTargetOrTerrain(QString, int, bool, bool)));
	addDockWidget(Qt::BottomDockWidgetArea, m_CommDock, Qt::Horizontal);
	m_CommDock->hide();
	m_DockBars.m_DockWidgets.append(AuFrameDock(m_CommDock));

	if(auApp->RunState() == AuApp::Designer)
	{
		// Single user designer.
		CreateDesignerMenu();
		ShowDesignerMenu();

		// Designer parts list.
		m_DesignerPartsDock = new AuDesignerPartsDock(this);
		connect(m_DesignerPartsDock, SIGNAL(GoToSelection()),
				m_View, SLOT(DesignerGoToSelection()),
				Qt::QueuedConnection);
		connect(m_DesignerPartsDock, SIGNAL(PartsSelectionChanged(AuScIdSet)),
				m_View, SLOT(DesignerSetPartSelection(AuScIdSet)),
				Qt::QueuedConnection);
		addDockWidget(Qt::RightDockWidgetArea, m_DesignerPartsDock, Qt::Vertical);
		m_DesignerPartsDock->hide();
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_DesignerPartsDock));

		// Designer texture wrapper.
		m_DesignerTextureWrapDock = new AuDesignerTextureWrapDock(this);
		connect(m_View, SIGNAL(DesignerSelectionChanged()), m_DesignerTextureWrapDock, SLOT(DesignerSelectionChanged()));
		addDockWidget(Qt::RightDockWidgetArea, m_DesignerTextureWrapDock, Qt::Vertical);
		m_DesignerTextureWrapDock->hide();
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_DesignerTextureWrapDock));

		// Restore positions and visibility of dock widgets.
		QByteArray windowState = g_Au.Settings().value("/Widget/MainFrame/DesignerWindowState").toByteArray();
		if(!windowState.isEmpty())
			restoreState(windowState);

		m_DesignerBar->show();
		m_DesignerBar2->show();
		m_DesignerPartsDock->show();
		m_DesignerTextureWrapDock->show();
	}
	else
	{
		bool solo = auApp->RunState() == AuApp::Solitaire;
		bool soloDesigner = auApp->IsSingleUserDesignerMode();

		// Character Bio
		m_BioDock = new AuBioDock(this);
		connect(m_BioDock->m_Bio->m_JobsPage, SIGNAL(CloseStoryRequested(AuJobOffer)),
				auApp, SLOT(CloseStory(AuJobOffer)));
		connect(m_BioDock->m_Bio->m_JobsPage, SIGNAL(StoryBlockRequested(quint32)),
				auApp, SLOT(StoryResendBlock(quint32)));
		connect(m_BioDock->m_Bio->m_JobsPage, SIGNAL(StoryHintRequested(quint32)),
				auApp, SLOT(StoryHint(quint32)));
		addDockWidget(Qt::RightDockWidgetArea, m_BioDock, Qt::Vertical);
		m_BioDock->hide();
		m_BioDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_BioDock));

		// Governance tabbed pages
		m_GovernDock = new AuGovernDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_GovernDock, Qt::Horizontal);
		m_GovernDock->hide();
		m_GovernDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_GovernDock));

		// Governance announcements
		m_GovernAnnouncementsDock = new AuGovAnnouncementsDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_GovernAnnouncementsDock, Qt::Horizontal);
		m_GovernAnnouncementsDock->hide();
		m_GovernAnnouncementsDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_GovernAnnouncementsDock));

		// Governance cities
		m_GovernCitiesDock = new AuGovCitiesDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_GovernCitiesDock, Qt::Horizontal);
		m_GovernCitiesDock->hide();
		m_GovernCitiesDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_GovernCitiesDock));

		// Governance citizens
		m_GovernCitizensDock = new AuGovCitizensDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_GovernCitizensDock, Qt::Horizontal);
		m_GovernCitizensDock->hide();
		m_GovernCitizensDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_GovernCitizensDock));

		// Governance commerce
		m_GovernCommerceDock = new AuGovCommerceDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_GovernCommerceDock, Qt::Horizontal);
		m_GovernCommerceDock->hide();
		m_GovernCommerceDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_GovernCommerceDock));

		// Governance diplomacy
		m_GovernDiplomacyDock = new AuGovDiplomacyDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_GovernDiplomacyDock, Qt::Horizontal);
		m_GovernDiplomacyDock->hide();
		m_GovernDiplomacyDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_GovernDiplomacyDock));

		// Governance patents
		m_GovernPatentsDock = new AuGovPatentsDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_GovernPatentsDock, Qt::Horizontal);
		m_GovernPatentsDock->hide();
		m_GovernPatentsDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_GovernPatentsDock));

		// Governance policy
		m_GovernPolicyDock = new AuGovPolicyDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_GovernPolicyDock, Qt::Horizontal);
		m_GovernPolicyDock->hide();
		m_GovernPolicyDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_GovernPolicyDock));

		// Governance taxes
		m_GovernTaxesDock = new AuGovTaxesDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_GovernTaxesDock, Qt::Horizontal);
		m_GovernTaxesDock->hide();
		m_GovernTaxesDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_GovernTaxesDock));

		// Cargo tabs
		m_HoldDock = new AuCargoDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_HoldDock, Qt::Horizontal);
		m_HoldDock->hide();
		m_HoldDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_HoldDock));

		m_HoldDockFree = new AuCargoDockFree(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_HoldDockFree, Qt::Horizontal);
		m_HoldDockFree->hide();
		m_HoldDockFree->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_HoldDockFree));

		// Cargo Hold
		m_CargoHoldDock = new AuCargoHoldDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_CargoHoldDock, Qt::Horizontal);
		m_CargoHoldDock->hide();
		m_CargoHoldDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_CargoHoldDock));

		// Cargo Manifest
		m_CargoManifestDock = new AuCargoJettisonDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_CargoManifestDock, Qt::Horizontal);
		m_CargoManifestDock->hide();
		m_CargoManifestDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_CargoManifestDock));

		// Cargo Ordnance
		m_CargoOrdnanceDock = new AuCargoOrdnanceDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_CargoOrdnanceDock, Qt::Horizontal);
		m_CargoOrdnanceDock->hide();
		m_CargoOrdnanceDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_CargoOrdnanceDock));

		// Cargo Transfer
		m_CargoTransferDock = new AuCargoTransferDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_CargoTransferDock, Qt::Horizontal);
		m_CargoTransferDock->hide();
		m_CargoTransferDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_CargoTransferDock));

		// Personal gear
		m_GearDock = new AuGearDock(this);
		connect(m_GearDock->m_Gear, SIGNAL(DropGearItem(AuGearPos)),
				auApp, SLOT(DropGearItem(AuGearPos)));
		connect(m_GearDock->m_Gear, SIGNAL(InscribeGearItem(AuGearPos, QString)),
				auApp, SLOT(InscribeGearItem(AuGearPos, QString)));
		connect(m_GearDock->m_Gear, SIGNAL(RedeemGearItem(AuGearPos)),
				auApp, SLOT(RedeemGearItem(AuGearPos)));
		connect(m_GearDock->m_Gear, SIGNAL(SetMessage(AuGearPos, QString)),
				auApp, SLOT(SetMessage(AuGearPos, QString)));
		connect(m_GearDock->m_Gear, SIGNAL(MoveGearItemToItem(AuGearPos, AuGearPos)),
				auApp, SLOT(MoveGearItemToItem(AuGearPos, AuGearPos)));
		connect(m_GearDock->m_Gear, SIGNAL(MoveGearItemToSlot(AuGearPos, quint8)),
				auApp, SLOT(MoveGearItemToSlot(AuGearPos, quint8)));
		connect(m_GearDock->m_Gear, SIGNAL(SetFlowerPot(AuGearPos, quint8)),
				auApp, SLOT(SetFlowerPot(AuGearPos, quint8)));
		connect(m_GearDock->m_Gear, SIGNAL(SetPictureFrame(AuGearPos, QString, bool, bool)),
				auApp, SLOT(SetPictureFrame(AuGearPos, QString, bool, bool)));
		connect(m_GearDock->m_Gear, SIGNAL(UseGearItem(AuGearPos)),
				auApp, SLOT(UseGearItem(AuGearPos)));
		connect(m_GearDock->m_Gear, SIGNAL(ShowGalaxyLocOnStarMap(QString, AuPoint)),
				SLOT(ShowGalaxyLocOnStarMap(QString, AuPoint)));
		connect(m_GearDock->m_Gear, SIGNAL(ToggleHandCraftUi()),
				SLOT(ToggleHandCraftUi()));
		addDockWidget(Qt::RightDockWidgetArea, m_GearDock, Qt::Vertical);
		m_GearDock->hide();
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_GearDock));

		// Locator
		m_LocatorDock = new AuLocatorDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_LocatorDock, Qt::Horizontal);
		m_LocatorDock->hide();
		m_LocatorDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_LocatorDock));

		// Locator Compass
		m_LocatorCompassDock = new AuLocatorCompassDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_LocatorCompassDock, Qt::Horizontal);
		m_LocatorCompassDock->hide();
		m_LocatorCompassDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_LocatorCompassDock));

		// Locator GPS
		m_LocatorGPSDock = new AuLocatorGPSDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_LocatorGPSDock, Qt::Horizontal);
		m_LocatorGPSDock->hide();
		m_LocatorGPSDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_LocatorGPSDock));

		// Locator Star Map
		m_LocatorStarMapDock = new AuLocatorStarMapDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_LocatorStarMapDock, Qt::Horizontal);
		m_LocatorStarMapDock->hide();
		m_LocatorStarMapDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_LocatorStarMapDock));

		// Locator World Map
		m_LocatorWorldMapDock = new AuLocatorWorldMapDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_LocatorWorldMapDock, Qt::Horizontal);
		m_LocatorWorldMapDock->hide();
		m_LocatorWorldMapDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_LocatorWorldMapDock));

		if(soloDesigner)
			m_MailDock = 0;
		else
		{
			m_MailDock = new AuMailDock(m_View->SceneAtomId(), this);
			connect(m_MailDock, SIGNAL(AlertMessageReceived(AuMailMsg)), SLOT(AlertMessageReceived(AuMailMsg)));
			connect(m_MailDock, SIGNAL(GoDirectToSystem(AuId, QString, AuPoint)),
					auApp, SLOT(GoDirectToSystem(AuId, QString, AuPoint)));
			connect(m_MailDock, SIGNAL(GoDirectToCoord(AuPoint)), auApp, SLOT(GoDirectToCoord(AuPoint)));
			connect(m_MailDock, SIGNAL(GoLandAtCity(AuId, QString)), auApp, SLOT(GoLandAtCity(AuId, QString)));
			connect(m_MailDock, SIGNAL(GoOrbit(AuId, QString)), auApp, SLOT(GoOrbit(AuId, QString)));
			connect(m_MailDock, SIGNAL(GoOrbitCity(AuId, QString)), auApp, SLOT(GoOrbitCity(AuId, QString)));
			connect(m_MailDock, SIGNAL(GoViaWormholes(AuId, QString)), auApp, SLOT(GoViaWormholes(AuId, QString)));
			connect(m_MailDock, SIGNAL(UpdateSquelch(AuId, bool)), auApp, SLOT(UpdateSquelch(AuId, bool)));
			connect(m_MailDock, SIGNAL(UpdateSquelches(AuIdList, bool)), auApp, SLOT(UpdateSquelches(AuIdList, bool)));
			addDockWidget(Qt::LeftDockWidgetArea, m_MailDock, Qt::Vertical);
			m_MailDock->hide();
			m_MailDock->setFloating(true);
			m_DockBars.m_DockWidgets.append(AuFrameDock(m_MailDock));
		}

		m_SpacecraftDock = new AuSpacecraftDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_SpacecraftDock, Qt::Horizontal);
		m_SpacecraftDock->hide();
		m_SpacecraftDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_SpacecraftDock));

		m_SpacecraftDockFree = new AuSpacecraftDockFree(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_SpacecraftDockFree, Qt::Horizontal);
		m_SpacecraftDockFree->hide();
		m_SpacecraftDockFree->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_SpacecraftDockFree));

		m_SpacecraftAccountDock = new AuSpacecraftAccountDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_SpacecraftAccountDock, Qt::Horizontal);
		m_SpacecraftAccountDock->hide();
		m_SpacecraftAccountDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_SpacecraftAccountDock));

		m_SpacecraftBountiesDock = new AuSpacecraftBountiesDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_SpacecraftBountiesDock, Qt::Horizontal);
		m_SpacecraftBountiesDock->hide();
		m_SpacecraftBountiesDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_SpacecraftBountiesDock));

		m_SpacecraftDamageDock = new AuSpacecraftDamageDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_SpacecraftDamageDock, Qt::Horizontal);
		m_SpacecraftDamageDock->hide();
		m_SpacecraftDamageDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_SpacecraftDamageDock));

		m_SpacecraftDesignExchangeDock = new AuScExchangeDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_SpacecraftDesignExchangeDock, Qt::Horizontal);
		m_SpacecraftDesignExchangeDock->hide();
		m_SpacecraftDesignExchangeDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_SpacecraftDesignExchangeDock));

		m_SpacecraftMissionDock = new AuSpacecraftMissionDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_SpacecraftMissionDock, Qt::Horizontal);
		m_SpacecraftMissionDock->hide();
		m_SpacecraftMissionDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_SpacecraftMissionDock));

		m_SpacecraftPolicyDock = new AuSpacecraftPolicyDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_SpacecraftPolicyDock, Qt::Horizontal);
		m_SpacecraftPolicyDock->hide();
		m_SpacecraftPolicyDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_SpacecraftPolicyDock));

		m_SpacecraftRosterDock = new AuSpacecraftRosterDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_SpacecraftRosterDock, Qt::Horizontal);
		m_SpacecraftRosterDock->hide();
		m_SpacecraftRosterDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_SpacecraftRosterDock));

		m_SpacecraftShipmentsDock = new AuSpacecraftShipmentsDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_SpacecraftShipmentsDock, Qt::Horizontal);
		m_SpacecraftShipmentsDock->hide();
		m_SpacecraftShipmentsDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_SpacecraftShipmentsDock));

		m_OrdersDock = new AuUnitOrdersDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_OrdersDock, Qt::Horizontal);
		m_OrdersDock->hide();
		m_OrdersDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_OrdersDock));

		// Designer parts list.
		m_DesignerPartsDock = new AuDesignerPartsDock(this);
		connect(m_DesignerPartsDock, SIGNAL(GoToSelection()),
				m_View, SLOT(DesignerGoToSelection()),
				Qt::QueuedConnection);
		connect(m_DesignerPartsDock, SIGNAL(PartsSelectionChanged(AuScIdSet)),
				m_View, SLOT(DesignerSetPartSelection(AuScIdSet)),
				Qt::QueuedConnection);
		addDockWidget(Qt::RightDockWidgetArea, m_DesignerPartsDock, Qt::Vertical);
		m_DesignerPartsDock->hide();
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_DesignerPartsDock));

		// Designer texture wrapper.
		m_DesignerTextureWrapDock = new AuDesignerTextureWrapDock(this);
		connect(m_View, SIGNAL(DesignerSelectionChanged()), m_DesignerTextureWrapDock, SLOT(DesignerSelectionChanged()));
		addDockWidget(Qt::RightDockWidgetArea, m_DesignerTextureWrapDock, Qt::Vertical);
		m_DesignerTextureWrapDock->hide();
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_DesignerTextureWrapDock));

		// Players Online
		if(solo)
			m_PlayersOnlineDock = 0;
		else
		{
			m_PlayersOnlineDock = new AuPlayersOnlineDock(this);
			connect(m_PlayersOnlineDock->m_Content, SIGNAL(AddBuddy(AuId, QString)),
					m_CommDock->m_Comm, SLOT(AddBuddy(AuId, QString)));
			connect(m_PlayersOnlineDock->m_Content, SIGNAL(AddSquelch(AuId, QString)),
					m_CommDock->m_Comm, SLOT(AddSquelch(AuId, QString)));
			connect(m_PlayersOnlineDock->m_Content, SIGNAL(AvatarOnLine(AuId)), m_View, SLOT(AvatarOnLine(AuId)));
			connect(auApp, SIGNAL(PlayersOnlineStatus(AuPlayerOnlineStatusVector)),
					m_PlayersOnlineDock->m_Content, SLOT(PlayersOnlineStatus(AuPlayerOnlineStatusVector)));
			connect(m_PlayersOnlineDock->m_Content, SIGNAL(OpenChannelToAvatar(AuId)),
					m_CommDock->m_Comm, SLOT(OpenChannelTo(AuId)));
			addDockWidget(Qt::BottomDockWidgetArea, m_PlayersOnlineDock, Qt::Horizontal);
			m_PlayersOnlineDock->hide();
			m_PlayersOnlineDock->setFloating(true);
			m_DockBars.m_DockWidgets.append(AuFrameDock(m_PlayersOnlineDock));
		}

		// Construction
		m_ConstructionDock = new AuConstructionDock(this);
		connect(m_ConstructionDock->m_Content, SIGNAL(PlanBuilding(INDUSTRYID, const AuBdReport&, COMMODITYID, quint16, bool)),
				m_View, SLOT(PlanBd(INDUSTRYID, const AuBdReport&, COMMODITYID, quint16, bool)));
		connect(m_ConstructionDock->m_Content, SIGNAL(PlanFence(int, quint32, COMMODITYID)),
				m_View, SLOT(PlanBdFence(int, quint32, COMMODITYID)));
		connect(m_ConstructionDock->m_Content, SIGNAL(PlanBuildingMaterial(COMMODITYID)),
				m_View, SLOT(PlanBdMaterial(COMMODITYID)));
		connect(m_ConstructionDock->m_Content, SIGNAL(PlanConstructionProcess(const AuBdReport&)),
				m_View, SLOT(PlanBdConstructionProcess(const AuBdReport&)));
		connect(m_ConstructionDock->m_Content, SIGNAL(PlanRoadAsphalt()),
				m_View, SLOT(PlanGeoRoadAsphalt()));
		connect(m_ConstructionDock->m_Content, SIGNAL(PlanRoadConcrete()),
				m_View, SLOT(PlanGeoRoadConcrete()));
		connect(m_ConstructionDock->m_Content, SIGNAL(PlanRoadDirt()),
				m_View, SLOT(PlanGeoRoadDirt()));
		connect(m_ConstructionDock->m_Content, SIGNAL(PlanRoadGrade()),
				m_View, SLOT(PlanGeoRoadGrade()));
		connect(m_ConstructionDock->m_Content, SIGNAL(PlanLandscapeClearing()),
				m_View, SLOT(PlanGeoLandscapeClearing()));
		connect(m_ConstructionDock->m_Content, SIGNAL(PlanLandscapeDefoliate()),
				m_View, SLOT(PlanGeoLandscapeDefoliate()));
		connect(m_ConstructionDock->m_Content, SIGNAL(PlanLandscapeIrrigate()),
				m_View, SLOT(PlanGeoLandscapeIrrigate()));
		connect(m_ConstructionDock->m_Content, SIGNAL(PlanManufacturingProcess(int)),
				m_View, SLOT(PlanManufacturingProcess(int)));
		connect(m_ConstructionDock->m_Content, SIGNAL(PlanManufacturingWorkerOk(bool)),
				m_View, SLOT(PlanManufacturingWorkerOk(bool)));
		connect(m_ConstructionDock->m_Content, SIGNAL(WindowHidden()), m_View, SLOT(EndCommands()));
		addDockWidget(Qt::RightDockWidgetArea, m_ConstructionDock, Qt::Vertical);
		m_ConstructionDock->hide();
		m_ConstructionDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_ConstructionDock));

		m_BdExchangeDock = new AuBdExchangeDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_BdExchangeDock, Qt::Horizontal);
		m_BdExchangeDock->hide();
		m_BdExchangeDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_BdExchangeDock));

		// Bd Building window
		m_BuildingDock = new AuBuildingDock(this);
		addDockWidget(Qt::RightDockWidgetArea, m_BuildingDock, Qt::Vertical);
		m_BuildingDock->hide();
		m_BuildingDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_BuildingDock));

		// Locator System Map
		m_LocatorSystemScanDock = new AuLocatorSystemScanDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_LocatorSystemScanDock, Qt::Horizontal);
		m_LocatorSystemScanDock->hide();
		m_LocatorSystemScanDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_LocatorSystemScanDock));

		m_SpacecraftSpecsDock = new AuSpacecraftSpecsDock(this);
		addDockWidget(Qt::BottomDockWidgetArea, m_SpacecraftSpecsDock, Qt::Horizontal);
		m_SpacecraftSpecsDock->hide();
		m_SpacecraftSpecsDock->setFloating(true);
		m_DockBars.m_DockWidgets.append(AuFrameDock(m_SpacecraftSpecsDock));

		// Restore positions and visibility of dock widgets.
		QByteArray windowState;
		if(solo)
			windowState = g_Au.Settings().value("/Widget/MainFrame/SoloWindowState").toByteArray();
		else
			windowState = g_Au.Settings().value("/Widget/MainFrame/WindowState1").toByteArray();
		if(!windowState.isEmpty())
			restoreState(windowState);

		for(QList<AuCommPacket*>::iterator ci = m_EarlyArrivals.begin(); ci != m_EarlyArrivals.end(); ci++)
		{
			ChatReceived(**ci);
			delete *ci;
		}
		m_EarlyArrivals.clear();

		m_DesignerBar->hide();
		m_DesignerBar2->hide();
		m_DesignerPartsDock->hide();
		m_DesignerTextureWrapDock->hide();
	}

	m_DockBars.UpdateTitleShortcuts();

	// Show/hide things appropriately.
	SceneContextChanged(m_View->SceneContext());
}

// 'sceneContext' is an SCENE_CONTEXT value.
void AuDesignerFrame::SceneContextChanged(int sceneContext)
{
	switch((SCENE_CONTEXT)sceneContext)
	{
	case SCENE_Unknown:
		break;

	case SCENE_Universe:
		if(!menuBar()->isHidden())
			menuBar()->hide();
		if(m_DesignerBar && !m_DesignerBar->isHidden())
			m_DesignerBar->hide();
		if(m_DesignerBar2 && !m_DesignerBar2->isHidden())
			m_DesignerBar2->hide();
		if(m_DesignerPartsDock && !m_DesignerPartsDock->isHidden())
			m_DesignerPartsDock->hide();
		if(m_DesignerTextureWrapDock && !m_DesignerTextureWrapDock->isHidden())
			m_DesignerTextureWrapDock->hide();
		break;

	case SCENE_Designer:
		CreateDesignerMenu();			// Does nothing if the menu has already been created.
		if(menuBar()->isHidden())
			menuBar()->show();
		if(m_ConstructionDock && !m_ConstructionDock->isHidden())
			m_ConstructionDock->hide();
		if(m_CargoHoldDock && !m_CargoHoldDock->isHidden())
			m_CargoHoldDock->hide();
		if(m_CargoManifestDock && !m_CargoManifestDock->isHidden())
			m_CargoManifestDock->hide();
		if(m_CargoOrdnanceDock && !m_CargoOrdnanceDock->isHidden())
			m_CargoOrdnanceDock->hide();
		if(m_CargoTransferDock && !m_CargoTransferDock->isHidden())
			m_CargoTransferDock->hide();
		if(m_HoldDock && !m_HoldDock->isHidden())
			m_HoldDock->hide();
		if(m_HoldDockFree && !m_HoldDockFree->isHidden())
			m_HoldDockFree->hide();
		if(m_BuildingDock && !m_BuildingDock->isHidden())
			m_BuildingDock->hide();
		if(m_LocatorDock && !m_LocatorDock->isHidden())
			m_LocatorDock->hide();
		if(m_LocatorCompassDock && !m_LocatorCompassDock->isHidden())
			m_LocatorCompassDock->hide();
		if(m_LocatorStarMapDock && !m_LocatorStarMapDock->isHidden())
			m_LocatorStarMapDock->hide();
		if(m_LocatorSystemScanDock && !m_LocatorSystemScanDock->isHidden())
			m_LocatorSystemScanDock->hide();
		if(m_LocatorWorldMapDock && !m_LocatorWorldMapDock->isHidden())
			m_LocatorWorldMapDock->hide();
		if(m_SpacecraftDock && !m_SpacecraftDock->isHidden())
			m_SpacecraftDock->hide();
		if(m_SpacecraftDockFree && !m_SpacecraftDockFree->isHidden())
			m_SpacecraftDockFree->hide();
		if(m_SpacecraftAccountDock && !m_SpacecraftAccountDock->isHidden())
			m_SpacecraftAccountDock->hide();
		if(m_SpacecraftBountiesDock && !m_SpacecraftBountiesDock->isHidden())
			m_SpacecraftBountiesDock->hide();
		if(m_SpacecraftDamageDock && !m_SpacecraftDamageDock->isHidden())
			m_SpacecraftDamageDock->hide();
		if(m_SpacecraftDesignExchangeDock && !m_SpacecraftDesignExchangeDock->isHidden())
			m_SpacecraftDesignExchangeDock->hide();
		if(m_SpacecraftMissionDock && !m_SpacecraftMissionDock->isHidden())
			m_SpacecraftMissionDock->hide();
		if(m_SpacecraftPolicyDock && !m_SpacecraftPolicyDock->isHidden())
			m_SpacecraftPolicyDock->hide();
		if(m_SpacecraftRosterDock && !m_SpacecraftRosterDock->isHidden())
			m_SpacecraftRosterDock->hide();
		if(m_SpacecraftShipmentsDock && !m_SpacecraftShipmentsDock->isHidden())
			m_SpacecraftShipmentsDock->hide();
		if(m_SpacecraftSpecsDock && !m_SpacecraftSpecsDock->isHidden())
			m_SpacecraftSpecsDock->hide();
		if(m_OrdersDock && !m_OrdersDock->isHidden())
			m_OrdersDock->hide();
		if(m_BdExchangeDock && !m_BdExchangeDock->isHidden())
			m_BdExchangeDock->hide();
		if(m_DesignerBar)
			m_DesignerBar->show();
		if(m_DesignerBar2)
			m_DesignerBar2->show();
		if(m_DesignerPartsDock)
			m_DesignerPartsDock->show();
		if(m_DesignerTextureWrapDock)
			m_DesignerTextureWrapDock->show();
		break;

	case SCENE_Arena:
		if(!menuBar()->isHidden())
			menuBar()->hide();
		if(m_ConstructionDock && !m_ConstructionDock->isHidden())
			m_ConstructionDock->hide();
		if(m_BuildingDock && !m_BuildingDock->isHidden())
			m_BuildingDock->hide();
		if(m_LocatorDock && !m_LocatorDock->isHidden())
			m_LocatorDock->hide();
		if(m_LocatorCompassDock && !m_LocatorCompassDock->isHidden())
			m_LocatorCompassDock->hide();
		if(m_LocatorStarMapDock && !m_LocatorStarMapDock->isHidden())
			m_LocatorStarMapDock->hide();
		if(m_LocatorSystemScanDock && !m_LocatorSystemScanDock->isHidden())
			m_LocatorSystemScanDock->hide();
		if(m_LocatorWorldMapDock && !m_LocatorWorldMapDock->isHidden())
			m_LocatorWorldMapDock->hide();
		if(m_DesignerBar && !m_DesignerBar->isHidden())
			m_DesignerBar->hide();
		if(m_DesignerBar2 && !m_DesignerBar2->isHidden())
			m_DesignerBar2->hide();
		if(m_DesignerPartsDock && !m_DesignerPartsDock->isHidden())
			m_DesignerPartsDock->hide();
		if(m_DesignerTextureWrapDock && !m_DesignerTextureWrapDock->isHidden())
			m_DesignerTextureWrapDock->hide();
		break;
	}
}

void AuDesignerFrame::SetMailRecipient(const AuId& personId)
{
	if(m_MailDock)
		m_MailDock->SetRecipient(personId);
}

void AuDesignerFrame::SwitchAvatar()
{
	if(m_SelectPlayerUp)
		return;

	// Insure we have a good list of their avatars. They can change due to births, demi deaths, etc.
	auApp->RequestAvatars();

	AuSwitchAvatar switchAv(this);
	m_SelectPlayerUp = true;
	if(ExecDialog(switchAv) == QDialog::Accepted)
	{
		// Set up to wait for a different scene atom to view.
		SwitchToAvatar(switchAv.SwitchToId());
	}
	m_SelectPlayerUp = false;
}

void AuDesignerFrame::SwitchToAvatar(AuId avatarId)
{
	// Set up to wait for a different scene atom to view.
	if(m_View)
		m_View->SetSceneAtom(avatarId);

	// Tell the scene server to make the switch.
	auApp->SwitchToAvatar(avatarId);
}

void AuDesignerFrame::SwitchToAvatarIndex(int avatarIndex)
{
	const DgData& gd = g_Au.LockDataForRead();
	AuId avatarId = gd.ClientUnitId(avatarIndex);
	g_Au.UnlockData();
	if(!avatarId.IsNull())
	{
		if(m_View)
			m_View->SaveWhereabouts();
		SwitchToAvatar(avatarId);
	}
}

void AuDesignerFrame::SwitchToAvatar1()
{
	SwitchToAvatarIndex(0);
}

void AuDesignerFrame::SwitchToAvatar2()
{
	SwitchToAvatarIndex(1);
}

void AuDesignerFrame::SwitchToAvatar3()
{
	SwitchToAvatarIndex(2);
}

void AuDesignerFrame::SwitchToAvatar4()
{
	SwitchToAvatarIndex(3);
}

void AuDesignerFrame::SwitchToAvatar5()
{
	SwitchToAvatarIndex(4);
}

void AuDesignerFrame::SwitchToAvatar6()
{
	SwitchToAvatarIndex(5);
}

void AuDesignerFrame::SwitchAvatarTransferred(AuId transferredId)
{
	// Remove the transferred id from the current list of avatars.
	DgData& gd = g_Au.LockDataForWrite();
	gd.ClientUnitDeleted(transferredId);	// Clears the local id, which is what we want.
	bool hasAnyUnit = gd.HasClientUnit();
	g_Au.UnlockData();

	// Set up to wait for a different scene atom to view.
	if(m_View)
		m_View->SetSceneAtom(AuId(0));

	if(!hasAnyUnit)
	{
		// The person's account has no more unit.
		QString msg = tr("The last of your avatars was transferred. "
				"The game must be restarted to create a new avatar.");
		QMessageBox::warning(this, g_Au.ApplicationName(), msg);
		exit(0);
	}

	SwitchAvatar();
}

void AuDesignerFrame::SelectGame()
{
	if(m_SelectGameUp)
		return;

	AuGameSelect dlg(this);
	m_SelectGameUp = true;
	if(ExecDialog(dlg) == QDialog::Accepted)
		auApp->GameSelect(dlg.GameType());	// This will return a result code that calls GameSelect.
	m_SelectGameUp = false;
}

void AuDesignerFrame::ConfigureControls(const DiControlInputArray& aAllControls)
{
	// Update my controls with those passed to me.
	for(DiControlInputArray::const_iterator ci = aAllControls.cbegin(); ci != aAllControls.cend(); ci++)
	{
		// Locate the corresponding control in my controls array.
		for(DiControlInputArray::iterator myci = s_FrameControls.begin(); myci != s_FrameControls.end(); myci++)
		{
			if((*myci).GetId() == CI_Title)
				continue;	// Skip title entries.
			if((*myci).GetId() == (*ci).GetId())
			{
				// Found the corresponding control id. Copy in the new data.
				*myci = *ci;
				break;
			}
		}
	}
	SaveControls();
}

void AuDesignerFrame::ControlsChanged()
{
	m_ControlLock.lock();
	if(m_pControls)
		m_pControls->clear();
	else
		m_pControls = new DiControlInputArray;
	EnumControlInputs(*m_pControls);
	m_ControlLock.unlock();

	// Update title bars of dock windows with appropriate F-key shortcut.
	m_DockBars.UpdateTitleShortcuts();
}

void AuDesignerFrame::RestoreDefaultControls()
{
	// Some controls are reserved for programmers.
	const DgData& gd = g_Au.LockDataForRead();
	bool isAgent = gd.ClientPlayerIsAgent();
	bool isMinion = gd.ClientPlayerIsMinion();
	g_Au.UnlockData();

	s_FrameControls.clear();
	for(int ic=0; ic<s_NumFrameDefaults; ic++)
	{
		switch(s_FrameDefaults[ic].GetId())
		{
		default:
			break;
		case CI_ToggleAgentUI:
			// Command available to any agent.
			if(!isAgent && !isMinion)
				continue;
			break;
		}

		s_FrameControls.append(s_FrameDefaults[ic]);
	}

	SaveControls();
}

void AuDesignerFrame::LoadControls() const
{
	// Some controls are reserved for programmers.
	const DgData& gd = g_Au.LockDataForRead();
	bool isAgent = gd.ClientPlayerIsAgent();
	bool isMinion = gd.ClientPlayerIsMinion();
	g_Au.UnlockData();

	// Init with my default controls.
	s_FrameControls.clear();
	for(int ic=0; ic<s_NumFrameDefaults; ic++)
	{
		switch(s_FrameDefaults[ic].GetId())
		{
		default:
			break;
		case CI_ToggleAgentUI:
			// Command available to any agent.
			if(!isAgent && !isMinion)
				continue;
			break;
		}

		s_FrameControls.append(s_FrameDefaults[ic]);
	}

	// Load any overrides from the settings file.
	s_FrameControls.LoadSettings("Controls/Window");

	/*
	for(DiControlInputArray::const_iterator ci = s_FrameControls.cbegin(); ci != s_FrameControls.cend(); ci++)
	{
		qDebug() << (*ci).Name() << "keyboard" << (*ci).Key() << (*ci).KeyModifier()
			<< "joystick" << (*ci).Joystick() << (*ci).JoystickAxisExclusive() << (*ci).JoystickIndex() << (*ci).JoystickSubIndex()
			<< "mouse" << (*ci).Mouse() << (*ci).MouseModifier() << (*ci).MouseWheel();
	}
	*/
}

void AuDesignerFrame::SaveControls() const
{
	// Store my controls in the settings file.
	s_FrameControls.SaveSettings("Controls/Window");
}

// A scene atom should fill this array with control inputs it can accept.
void AuDesignerFrame::EnumControlInputs(DiControlInputArray& aAllControls) const
{
	// Load my static controls array if it hasn't been loaded yet. This data will only exist on the client since EnumControlInputs
	// is not called on the server.
	if(s_FrameControls.isEmpty())
		LoadControls();

	// Append my controls to the array. I must include titles because the settings UI calls this function to get started.
	for(DiControlInputArray::const_iterator ci = s_FrameControls.cbegin(); ci != s_FrameControls.cend(); ci++)
		aAllControls.append(*ci);
}

void AuDesignerFrame::KeyPressed(QKeyEvent* e)
{
	//qDebug() << "AuDesignerFrame::KeyPressed" << e->key() << e->modifiers() << m_ActiveKeys.contains((Qt::Key)e->key());
	// View does not get tab key press events so we'll forward this one.
	if(m_View && e->key() == Qt::Key_Tab)
	{
		m_ControlLock.lock();
		bool haskey = m_ActiveKeys.find((Qt::Key)e->key()) != m_ActiveKeys.end();
		m_ControlLock.unlock();
		if(!haskey)
			m_View->TabKeyPressed(e->modifiers());
	}

	m_ControlLock.lock();
	m_ActiveKeys.insert(pair<Qt::Key, AuElapsedTimer>((Qt::Key)e->key(), AuElapsedTimer()));
	m_ActiveKeyMods = e->modifiers();
	m_ControlLock.unlock();
}

void AuDesignerFrame::KeyReleased(QKeyEvent* e)
{
	m_ControlLock.lock();
	//qDebug() << "AuDesignerFrame::KeyReleased" << e->key() << e->modifiers() << m_ActiveKeys.contains((Qt::Key)e->key());
	Q_ASSERT(m_pControls);

	// Sample the controls requested by the user for activation.
	Qt::KeyboardModifiers keyMods = m_pControls->FilterModState(m_ActiveKeys, e->modifiers());
	DiControlInputArray aActiveControls;
	m_pControls->SampleKeyInput(aActiveControls, m_ActiveKeys, keyMods, AuId(0), 0, false, AuPoint(0,0), 0);

	unordered_map<Qt::Key, AuElapsedTimer, AuKey_Hash>::iterator ki = m_ActiveKeys.find((Qt::Key)e->key());
	if(ki != m_ActiveKeys.end())
		m_ActiveKeys.erase(ki);
	m_ActiveKeyMods = e->modifiers();
	m_ControlLock.unlock();

	// Process the active UI controls.
	if(!aActiveControls.isEmpty())
		ProcessControlInput(aActiveControls);
}

void AuDesignerFrame::ProcessControlInput(const DiControlInputArray& aActiveControls)
{
	for(DiControlInputArray::const_iterator ci = aActiveControls.cbegin(); ci != aActiveControls.cend(); ci++)
	{
		switch((*ci).GetId())
		{
		case CI_Title:
			// This one should never get activated. It is only used for titles in the settings UI.
			break;

		case CI_About:
			HelpAbout();
			break;

		case CI_ExitGame:
			auApp->closeAllWindows();
			break;

		case CI_ToggleAllUIs:
			ToggleAllUis();
			break;

		case CI_ToggleMailUI:
			ToggleMailUi();
			break;

		case CI_ToggleSettingsUI:
			ToggleSettingsUi();
			break;

		case CI_ToggleGovernUI:
			ToggleGovernUi();
			break;

		case CI_ToggleGovernAnnouncementsUI:
			ToggleGovernAnnouncementsUi();
			break;

		case CI_ToggleGovernCitiesUI:
			ToggleGovernCitiesUi();
			break;

		case CI_ToggleGovernCitizensUI:
			ToggleGovernCitizensUi();
			break;

		case CI_ToggleGovernCommerceUI:
			ToggleGovernCommerceUi();
			break;

		case CI_ToggleGovernDiplomacyUI:
			ToggleGovernDiplomacyUi();
			break;

		case CI_ToggleGovernPolicyUI:
			ToggleGovernPolicyUi();
			break;

		case CI_ToggleGovernRewardsUI_Deprecated:
			break;

		case CI_ToggleGovernTaxesUI:
			ToggleGovernUi();
			break;

		case CI_ToggleAgentUI:
			ToggleAgentUi();
			break;

		case CI_TogglePlayersOnlineUI:
			TogglePlayersOnlineUi();
			break;

		case CI_ToggleConstructionUI:
			ToggleConstructionUi();
			break;

		case CI_ToggleBuildingUI:
			ToggleBuildingUi();
			break;

		case CI_ToggleCharacterUI:
			ToggleCharacterUi();
			break;

		case CI_ToggleLocatorUI:
			ToggleLocatorUi();
			break;

		case CI_ToggleLocatorCompassUI:
			ToggleLocatorCompassUi();
			break;

		case CI_ToggleLocatorGPSUI:
			ToggleLocatorGPSUi();
			break;

		case CI_ToggleLocatorStarMapUI:
			ToggleLocatorStarMapUi();
			break;

		case CI_ToggleLocatorSystemScanUI:
			ToggleLocatorSystemScanUi();
			break;

		case CI_ToggleLocatorWorldMapUI:
			ToggleLocatorWorldMapUi();
			break;

		case CI_ToggleDesignPartsUI:
			ToggleDesignPartsUi();
			break;

		case CI_ToggleDesignTextureWrapUI:
			ToggleDesignTextureWrapUi();
			break;

		case CI_ToggleCargoUI:
			ToggleHoldUi();
			break;

		case CI_ToggleCargoHoldUI:
			ToggleCargoHoldUi();
			break;

		case CI_ToggleCargoManifestUI:
			ToggleCargoManifestUi();
			break;

		case CI_ToggleCargoOrdnanceUI:
			ToggleCargoOrdnanceUi();
			break;

		case CI_ToggleCargoTransferUI:
			ToggleCargoTransferUi();
			break;

		case CI_ToggleGearUI:
			ToggleGearUi();	// My gear.
			break;

		case CI_ToggleHandCraftUI:
			ToggleHandCraftUi();
			break;

		case CI_ToggleCommUI:
			ToggleCommUi();
			break;

		case CI_ToggleOrdersUI:
			ToggleOrdersUi();
			break;

		case CI_ToggleSpacecraftUI:
			ToggleSpacecraftUi();
			break;

		case CI_ToggleSpacecraftAccountUI:
			ToggleSpacecraftAccountUi();
			break;

		case CI_ToggleSpacecraftBountiesUI:
			ToggleSpacecraftBountiesUi();
			break;

		case CI_ToggleSpacecraftDamageUI:
			ToggleSpacecraftDamageUi();
			break;

		case CI_ToggleBdExchangeUI:
			ToggleBdExchangeUi();
			break;

		case CI_ToggleScExchangeUI:
			ToggleSpacecraftDesignExchangeUi();
			break;

		case CI_ToggleSpacecraftMissionUI:
			ToggleSpacecraftMissionUi();
			break;

		case CI_ToggleSpacecraftPolicyUI:
			ToggleSpacecraftPolicyUi();
			break;

		case CI_ToggleSpacecraftRosterUI:
			ToggleSpacecraftRosterUi();
			break;

		case CI_ToggleSpacecraftShipmentsUI:
			ToggleSpacecraftShipmentsUi();
			break;

		case CI_ToggleSpacecraftSpecsUI:
			ToggleSpacecraftSpecsUi();
			break;

		case CI_FullScreenToggle:
			ToggleFullScreen();
			break;

		case CI_ToggleBuildUI_Deprecated:
		case CI_ToggleCityUI_Deprecated:
			break;
		}
	}
}

void AuDesignerFrame::MissionSensorContactsReceived(QByteArray bablips)
{
	if(m_SpacecraftDock && m_SpacecraftDock->m_Content->ShipMission())
		m_SpacecraftDock->m_Content->ShipMission()->SensorContactsReceived(bablips);
	if(m_SpacecraftDockFree && m_SpacecraftDockFree->m_Content->ShipMission())
		m_SpacecraftDockFree->m_Content->ShipMission()->SensorContactsReceived(bablips);
	if(m_SpacecraftMissionDock)
		m_SpacecraftMissionDock->m_Mission->SensorContactsReceived(bablips);
	if(m_CommDock)
		m_CommDock->m_Comm->SensorContactsReceived(bablips);
}

void AuDesignerFrame::HelpAbout()
{
	AuAbout about(this);
	ExecDialog(about);
}

void AuDesignerFrame::ReportSystemResources(AuId systemId, QString systemName, quint8 galaxy, bool isCoreBlackHole, quint8 genHab,
		quint32 genSeed, AuPoint systemLoc, float starLight, int scanEx)
{
	if(genHab == HABITABLE_EmptySystem)
		return;
	if(!genSeed)
		return;

	const AuStarMapData& starMap = g_AuClient.LockStarMapForRead();
	AuId sectorId = starMap.SectorOfSystem(systemId);
	QString sectorName = starMap.SectorNameOfSystem(systemId);
	g_AuClient.UnlockStarMap();
	if(sectorId.IsNull())
		return;

	AuSystemScanDialog* dlg = new AuSystemScanDialog(sectorId, sectorName, systemId, systemName, (GALAXY)galaxy, isCoreBlackHole,
			(HABITABILITY)genHab, genSeed, systemLoc, starLight, scanEx, this);
	dlg->show();
}

void AuDesignerFrame::CityHistoryReportReady(AuId cityId, AuCityReportHistory report)
{
	AuCityHistoryDlg* dlg = new AuCityHistoryDlg(cityId, report, this);
	dlg->show();
}

void AuDesignerFrame::CityLastReportReady(AuId cityId, AuCityReport report)
{
	AuCityReportDlg* dlg = new AuCityReportDlg(cityId, report, this);
	dlg->show();
}

void AuDesignerFrame::setVisible(bool visible)
{
	AuFrame::setVisible(visible);
	if(visible)
	{
		if(!m_Timer)
		{
			// This is the first time we are becoming visible.
			QTimer::singleShot(0, [=](){RestoreGeometry();});

			// Start the timer.
			m_Timer = new QTimer(this);
			connect(m_Timer, SIGNAL(timeout()), this, SLOT(Timer()));
			m_Timer->start(100);
		}
	}
}

void AuDesignerFrame::RestoreGeometry()
{
	// Restore window geometry if not currently full screen.
	/*
	bool fullScreen = isFullScreen();
	if(!fullScreen)
	{
		// Full screen mode was not forced using a command line option. Restore last state.
		if(auApp->RunState() == AuApp::Solitaire)
			fullScreen = g_Au.Settings().readBoolEntry("/Widget/MainFrame/SoloFullScreen", fullScreen);
		else if(auApp->RunState() == AuApp::Designer)
			fullScreen = g_Au.Settings().readBoolEntry("/Widget/MainFrame/DesignerFullScreen", fullScreen);
		else
			fullScreen = g_Au.Settings().readBoolEntry("/Widget/MainFrame/FullScreen", fullScreen);
		if(fullScreen)
			showFullScreen();
	}
	*/
	restoreGeometry(g_Au.Settings().value("/Widget/MainFrame/Geometry").toByteArray());
}

void AuDesignerFrame::OrdnanceReceived(AuId loaderId, QByteArray ord)
{
	Q_UNUSED(loaderId);
	if(m_HoldDock)
		m_HoldDock->m_Cargo->OrdnanceReceived(ord);
	if(m_HoldDockFree)
		m_HoldDockFree->m_Cargo->OrdnanceReceived(ord);
	if(m_CargoOrdnanceDock)
		m_CargoOrdnanceDock->m_Content->OrdnanceReceived(ord);
}

void AuDesignerFrame::SceneAtomChanged()
{
	if(m_View->SceneAtomId().IsNull())
		return;

	if(m_MailDock)
	{
		bool mailAlso = g_Au.Settings().readBoolEntry("Widget/SwitchAvatar/MailAlso", true);
		if(mailAlso)
			m_MailDock->SetRecipient(m_View->SceneAtomId());
	}
}

// Scene atom entered a cartesian coordinate at a transporter console.
void AuDesignerFrame::TransporterCoordUI(const AuPoint& personLoc)
{
	AuPoint loc = personLoc / FeetPerMeter();	// Convert to meters.
	loc.Round(.1);								// Round to nearest .1 meters.
	QString sloc = QString("%1, %2, %3").arg(loc.x).arg(loc.y).arg(loc.z);
	bool ok;
	QString scoord = QInputDialog::getText(0, QObject::tr("Transporter Coordinates"),
			QObject::tr("<html>Enter Cartesian or polar coordinates relative to the pivot point of the spacecraft.<p>"
				"Distances are specified in meters.<p>"
				"Cartesian coordinate of the selected contact or your current location is shown.</html>"),
			QLineEdit::Normal, sloc, &ok);
	if(!ok || scoord.isEmpty())
		return;

	AuPoint coord(scoord);
	coord *= FeetPerMeter();									// Convert meters to feet units.
	auApp->TransporterCoord(coord);
}

// Scene atom entered a longitude latitude coordinate at a transporter console.
void AuDesignerFrame::TransporterLongLatUI(const AuGlobeLLF& ll)
{
	bool ok;
	QString slonglat = QInputDialog::getText(0, QObject::tr("Transporter Longitude/Latitude Coordinates"),
			QObject::tr("<html>Enter longitude/latitude coordinates on the current world.</html>"),
			QLineEdit::Normal, ll.ToString(), &ok);
	if(!ok)
		return;

	AuGlobeLLF llgo;
	if(!llgo.ParseLongitudeLatitude(slonglat))
		return;
	auApp->TransporterLongLat(llgo);
}

void AuDesignerFrame::PostNetRequests()
{
	//qDebug() << "AuDesignerFrame::PostNetRequests" << g_AuClient.ImageDownloads().count() << "pending";
	QHash<QUrl, AuImageDownload>& imgdowns = g_AuClient.LockImageDownloads();
	if(!imgdowns.isEmpty())
	{
		// Create the network disk cache if we haven't done so yet.
		if(!m_NetCache)
		{
			//qDebug() << "AuDesignerFrame::PostNetRequests creating net cache";
			m_NetCache = new QNetworkDiskCache(this);
			m_NetCache->setCacheDirectory(g_Au.CacheNet());
		}

		// Create the network manager if we haven't done so yet.
		if(!m_Net)
		{
			//qDebug() << "AuDesignerFrame::PostNetRequests creating net access manager";
			m_Net = new QNetworkAccessManager(this);
			connect(m_Net, SIGNAL(finished(QNetworkReply*)), SLOT(NetReplyFinished(QNetworkReply*)));
			m_Net->setCache(m_NetCache);
		}

		// Post network requests.
		for(QHash<QUrl, AuImageDownload>::iterator di = imgdowns.begin(); di != imgdowns.end(); di++)
		{
			// We only ask one time for each picture.
			if((*di).NetRequestPosted())
				continue;

			//qDebug() << "AuDesignerFrame::PostNetRequests" << di.key() << "requested";
			m_Net->get(QNetworkRequest(di.key()));
			(*di).SetNetRequestPosted();
		}
	}
	g_AuClient.UnlockImageDownloads();
}

void AuDesignerFrame::NetReplyFinished(QNetworkReply* reply)
{
	//qDebug() << "AuDesignerFrame::NetReplyFinished" << reply->url().path()
		//<< "from" << (reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute).toBool() ? "cache" : "net");
	QHash<QUrl, AuImageDownload>& imgdowns = g_AuClient.LockImageDownloads();
	QHash<QUrl, AuImageDownload>::iterator di = imgdowns.find(reply->url());
	if(di != imgdowns.end())
		(*di).SetNetRequestFinished();
	g_AuClient.UnlockImageDownloads();
}

QIODevice* AuDesignerFrame::NetCacheData(const QUrl& url)
{
	if(!m_NetCache)
		return 0;
	return m_NetCache->data(url);
}

void AuDesignerFrame::ClearNetCache()
{
	// Set up the network cache so we can clear it.
	if(!m_NetCache)
	{
		m_NetCache = new QNetworkDiskCache(this);
		m_NetCache->setCacheDirectory(g_Au.CacheNet());
	}

	// Clear the network cache.
	m_NetCache->clear();
}

void AuDesignerFrame::AlertMessageReceived(AuMailMsg msg)
{
	// Construct a thought message from the mail notification message.
	const DgData& gd = g_Au.LockDataForRead();
	AuId personId = gd.ClientUnitId();
	QString personName = gd.ClientUnitName(personId);
	g_Au.UnlockData();

	QString msgtext = QString("%1, sent %2").arg(msg.m_subject).arg(msg.m_tmSent.UTC().toLocalTime().toString());

	AuCommThoughtMailAlert tmsg(msg.Type(), msgtext, AuId(1), msg.m_nameFrom);
	g_Au.SendAtomChat(tmsg);
}

QString AuDesignerFrame::ControlBinding(CONTROLINPUT control) const
{
	if(s_FrameControls.isEmpty())
		LoadControls();

	for(DiControlInputArray::const_iterator ci = s_FrameControls.cbegin(); ci != s_FrameControls.cend(); ci++)
		if((*ci).GetId() == control)
		{
			Qt::Key key = (*ci).Key();
			if(key == Qt::Key_unknown)
				break;

			Qt::KeyboardModifiers keymod = (*ci).KeyModifier();
			return AuBindControls::ModifierName(keymod) + AuBindControls::KeyName(key);
		}

	return "";
}

void AuDesignerFrame::ReplaceSpecies(const DzSpecies& species)
{
	if(m_GearDock)
		m_GearDock->m_Gear->ReplaceSpecies(species);
}

void AuDesignerFrame::ShowDesignerMenu()
{
	menuBar()->show();
	if(m_DesignerBar)
		m_DesignerBar->show();
	if(m_DesignerBar2)
		m_DesignerBar2->show();
	if(m_DesignerPartsDock)
		m_DesignerPartsDock->show();
	if(m_DesignerTextureWrapDock)
		m_DesignerTextureWrapDock->show();
}

// Returns true if no other processing should be performed on the key by the view. e.g. Return, Backspace, coords.
bool AuDesignerFrame::DesignerKeyReceived(QKeyEvent* e)
{
	if(!m_DesignerBar || !m_DesignerBar->isVisible())
		return false;
	return m_DesignerBar->KeyReceived(e);
}

void AuDesignerFrame::DesignerExited()
{
	// Hide the main window menu bar, which is only used for the design studio.
	menuBar()->hide();

	if(m_DesignerBar)
		m_DesignerBar->hide();
	if(m_DesignerBar2)
		m_DesignerBar2->hide();
	if(m_DesignerPartsDock)
		m_DesignerPartsDock->hide();
	if(m_DesignerTextureWrapDock)
		m_DesignerTextureWrapDock->hide();
}

void AuDesignerFrame::CreateDesignerMenu()
{
	Q_ASSERT(menuBar());
	if(m_Designer_SnapToGrid)
		return;							// The menu has already been created.

	QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
	if(fileMenu)
	{
		//QMenu* nuMenu = fileMenu->addMenu(QIcon(":data/new12x12.png"), tr("&New"));
		QMenu* nuMenu = fileMenu;
		if(nuMenu)
		{
			m_Designer_NewBuilding = nuMenu->addAction(tr("New &Building..."), m_View, SLOT(DesignerNewBuilding()));
			m_Designer_NewBuilding->setIcon(QIcon(":data/City.png"));

			m_Designer_NewItem = nuMenu->addAction(tr("New &Item..."), m_View, SLOT(DesignerNewItem()));
			m_Designer_NewItem->setIcon(QIcon(":data/Inventory.png"));

			m_Designer_NewSpacecraft = nuMenu->addAction(tr("New S&pacecraft"), m_View, SLOT(DesignerNewSpacecraft()));
			m_Designer_NewSpacecraft->setIcon(QIcon(":data/c_Spacecraft.png"));

			m_Designer_NewVehicle = nuMenu->addAction(tr("New &Vehicle..."), m_View, SLOT(DesignerNewVehicle()));
			m_Designer_NewVehicle->setIcon(QIcon(":data/c_SpaceFighter.png"));
		}

		m_Designer_Open = fileMenu->addAction(tr("&Open..."), m_View, SLOT(DesignerOpen()), QKeySequence::Open);
		m_Designer_Open->setIcon(QIcon(":data/dp_Load.png"));

		m_Designer_Save = fileMenu->addAction(tr("&Save"), m_View, SLOT(DesignerSave()), QKeySequence::Save);
		m_Designer_Save->setIcon(QIcon(":data/dp_Save.png"));

		m_Designer_SaveAs = fileMenu->addAction(tr("Save &As..."), m_View, SLOT(DesignerSaveAs()), QKeySequence::SaveAs);
		m_Designer_SaveAs->setIcon(QIcon(":data/dp_Save.png"));

		fileMenu->addSeparator();

		if(!auApp->IsSingleUserDesignerMode())
		{
			m_Designer_BdLibraryOpen = fileMenu->addAction(tr("Open Building &from Library..."), m_View,
					SLOT(DesignerBdLibraryOpen()));

			m_Designer_ScLibraryOpen = fileMenu->addAction(tr("Open Spacecraft &from Library..."), m_View,
					SLOT(DesignerScLibraryOpen()));

			m_Designer_LibrarySave = fileMenu->addAction(tr("Save &to Library..."), m_View, SLOT(DesignerLibrarySave()));

			m_Designer_BdLibraryDelete = fileMenu->addAction(tr("De&lete Building from Library..."), m_View,
					SLOT(DesignerBdLibraryDelete()));

			m_Designer_ScLibraryDelete = fileMenu->addAction(tr("De&lete Spacecraft from Library..."), m_View,
					SLOT(DesignerScLibraryDelete()));

			fileMenu->addSeparator();

			m_Designer_GearOpen = fileMenu->addAction(tr("O&pen from Gear..."), m_View, SLOT(DesignerGearOpen()));

			m_Designer_GearSaveBd = fileMenu->addAction(tr("Sa&ve Building to Gear..."), m_View, SLOT(DesignerGearSaveBd()));

			m_Designer_GearSaveSc = fileMenu->addAction(tr("Sa&ve Spacecraft to Gear..."), m_View, SLOT(DesignerGearSaveSc()));

			fileMenu->addSeparator();
		}

		QMenu* importMenu = fileMenu->addMenu(tr("I&mport"));
		if(importMenu)
		{
			m_Designer_Import3DS = importMenu->addAction(QIcon(":data/ScMesh.png"), tr("&3ds..."), m_View,
					SLOT(DesignerImport3DS()));

			m_Designer_ImportSTL = importMenu->addAction(QIcon(":data/ScMesh.png"), tr("&STL..."), m_View,
					SLOT(DesignerImportSTL()));
		}

		m_Designer_ExportSTL = fileMenu->addAction(QIcon(":data/ScMesh.png"), tr("&Export STL..."), m_View,
				SLOT(DesignerExportSTL()));

		fileMenu->addSeparator();

		if(!auApp->IsSingleUserDesignerMode())
		{
			m_Designer_Depart = fileMenu->addAction(tr("&Depart Studio"), m_View, SLOT(DesignerDepart()));
			m_Designer_Depart->setIcon(QIcon(":data/dp_Exit.png"));
		}

		m_Designer_Exit = fileMenu->addAction(tr("E&xit"), m_View, SLOT(DesignerExit()), QKeySequence::Quit);
		m_Designer_Exit->setIcon(QIcon(":data/ExitGame.png"));
	}

	m_Designer_EditMenu = menuBar()->addMenu(tr("&Edit"));
	if(m_Designer_EditMenu)
	{
		m_Designer_Undo = m_Designer_EditMenu->addAction(tr("&Undo"), m_View, SLOT(DesignerUndo()), QKeySequence::Undo);

		m_Designer_Redo = m_Designer_EditMenu->addAction(tr("Red&o"), m_View, SLOT(DesignerRedo()), QKeySequence::Redo);

		m_Designer_EditMenu->addSeparator();

		m_Designer_Cut = m_Designer_EditMenu->addAction(tr("Cu&t"), m_View, SLOT(DesignerCut()), QKeySequence::Cut);

		m_Designer_Copy = m_Designer_EditMenu->addAction(tr("&Copy"), m_View, SLOT(DesignerCopy()), QKeySequence::Copy);

		m_Designer_Paste = m_Designer_EditMenu->addAction(tr("&Paste"), m_View, SLOT(DesignerPaste()), QKeySequence::Paste);

		m_Designer_PasteAt = m_Designer_EditMenu->addAction(tr("Paste &At..."), m_View, SLOT(DesignerPasteAt()));

		m_Designer_PasteRadial = m_Designer_EditMenu->addAction(tr("Paste Radial..."), m_View, SLOT(DesignerPasteRadial()));

		m_Designer_Delete = m_Designer_EditMenu->addAction(tr("&Delete"), m_View, SLOT(DesignerDelete()), QKeySequence::Delete);

		m_Designer_EditMenu->addSeparator();

		m_Designer_SelectAll = m_Designer_EditMenu->addAction(tr("Se&lect All"), m_View, SLOT(DesignerSelectAll()),
				QKeySequence::SelectAll);

		m_Designer_SelectContiguous = m_Designer_EditMenu->addAction(tr("Select Contiguous Faces..."), m_View,
				SLOT(DesignerSelectContiguous()));

		m_Designer_SelectEdges = m_Designer_EditMenu->addAction(tr("Select Edges"), m_View, SLOT(DesignerSelectEdges()));

		m_Designer_SelectFaces = m_Designer_EditMenu->addAction(tr("Select Faces"), m_View, SLOT(DesignerSelectFaces()));

		m_Designer_SelectVertices = m_Designer_EditMenu->addAction(tr("Select Vertices"), m_View, SLOT(DesignerSelectVertices()));

		m_Designer_DeselectAll = m_Designer_EditMenu->addAction(tr("D&eselect All"), m_View, SLOT(DesignerSelectNone()),
				QKeySequence::Deselect);

		m_Designer_InvertSelection = m_Designer_EditMenu->addAction(tr("&Invert Selection"), m_View, SLOT(DesignerSelectInvert()),
				QKeySequence(Qt::Key_I|Qt::ControlModifier));

		m_Designer_EditMenu->addSeparator();

		m_Designer_Align = m_Designer_EditMenu->addAction(tr("Align Selected Objects"), m_View, SLOT(DesignerAlign()));

		m_Designer_Grab = m_Designer_EditMenu->addAction(tr("&Grab Selected Objects"), m_View, SLOT(DesignerMoveMenu()));

		m_Designer_Rotate = m_Designer_EditMenu->addAction(tr("&Rotate Selected Objects"), m_View, SLOT(DesignerRotate()));

		m_Designer_Stretch = m_Designer_EditMenu->addAction(tr("&Stretch Selected Objects"), m_View, SLOT(DesignerStretchMenu()));

		m_Designer_EditMenu->addSeparator();

		m_Designer_Settings = m_Designer_EditMenu->addAction(QIcon(":data/Settings.png"), tr("Setti&ngs..."), this, SLOT(ToggleSettingsUi()),
				QKeySequence::Preferences);
	}

	m_Designer_DesignMenu = menuBar()->addMenu(tr("&Blueprint"));
	if(m_Designer_DesignMenu)
	{
		m_Designer_DesignProperties = m_Designer_DesignMenu->addAction(QIcon(":data/CommManifest.png"), tr("&Properties..."),
				m_View, SLOT(DesignerProperties()));

		m_Designer_RefreshObstructions = m_Designer_DesignMenu->addAction(QIcon(":data/Cycle.png"), tr("Refresh &Obstructions"),
				auApp, SLOT(DesignerRefreshObstructions()));

		m_Designer_SolidsBlockPath = m_Designer_DesignMenu->addAction(QIcon(":data/ScMoveBlock.png"), tr("Solids Obstruct &Movement"),
				this, SLOT(DesignerMenuSetSolidsBlockPath()));
		m_Designer_SolidsBlockPath->setCheckable(true);
		m_Designer_SolidsBlockPath->setChecked(g_Au.DesignerBlockPaths());

		m_Designer_GravityToggle = m_Designer_DesignMenu->addAction(QIcon(":data/dp_Gravity.png"), tr("&Gravity On/Off"),
				this, SLOT(DesignerMenuSetGravityOn()));
		m_Designer_GravityToggle->setCheckable(true);
		m_Designer_GravityToggle->setChecked(g_Au.DesignerGravityOn());

		m_Designer_PreviewToggle = m_Designer_DesignMenu->addAction(QIcon(":data/MsgSolicitSales.png"), tr("Pre&view On/Off"),
				this, SLOT(DesignerMenuSetPreview()));
		m_Designer_PreviewToggle->setCheckable(true);
		m_Designer_PreviewToggle->setChecked(false);

		m_Designer_PreviewPeople = m_Designer_DesignMenu->addAction(QIcon(":data/CommCrew.png"), tr("Preview &People"),
				m_View, SLOT(DesignerPreviewPeople()));

		m_Designer_PreviewSettings = m_Designer_DesignMenu->addAction(QIcon(":data/CommChannel.png"), tr("Previe&w Settings..."),
				m_View, SLOT(DesignerPreviewSettings()));

		m_Designer_PreviewBd = m_Designer_DesignMenu->addAction(QIcon(":data/ConstructionSign.png"),
				tr("Preview Place &Building..."), m_View, SLOT(DesignerPreviewBd()));

		m_Designer_DesignMenu->addSeparator();

		m_Designer_DesignFinalize = m_Designer_DesignMenu->addAction(QIcon(":data/ScFinish.png"), tr("&Finalize..."),
				m_View, SLOT(DesignerFinalize()));

		m_Designer_DesignFinalizeReport = m_Designer_DesignMenu->addAction(QIcon(":data/ScFinish.png"),
				tr("Previous Finalize &Result..."), m_View, SLOT(DesignerFinalizeReport()));
	}

	m_Designer_PartMenu = menuBar()->addMenu(tr("&Part"));
	if(m_Designer_PartMenu)
	{
		m_Designer_PartPropsMenu = m_Designer_PartMenu->addMenu(QIcon(":data/CommManifest.png"), tr("Propert&ies"));
		if(m_Designer_PartPropsMenu)
		{
			m_Designer_DoorProperties = m_Designer_PartPropsMenu->addAction(QIcon(":data/dp_HullDoor.png"),
					tr("&Door..."), m_View, SLOT(DesignerDoorProperties()));

			m_Designer_MoveDoorSwitches = m_Designer_PartPropsMenu->addAction(QIcon(":data/ScDoorSwitches.png"),
					tr("Door &Switch Positions..."), m_View, SLOT(DesignerMoveDoorSwitches()));

			m_Designer_LightBulbProps = m_Designer_PartPropsMenu->addAction(QIcon(":data/Light.png"),
					tr("Light &Bulb..."), m_View, SLOT(DesignerLightBulbProps()));

			m_Designer_LightLensProps = m_Designer_PartPropsMenu->addAction(QIcon(":data/ScHeadlight.png"),
					tr("Light &Lens..."), m_View, SLOT(DesignerLightLensProps()));

			m_Designer_SetRoomProperties = m_Designer_PartPropsMenu->addAction(QIcon(":data/ScRoom.png"), tr("&Room Lighting..."),
					m_View, SLOT(DesignerSetRoomProperties()));

			m_Designer_SetVoidProperties = m_Designer_PartPropsMenu->addAction(QIcon(":data/ScRoomVoid.png"),
					tr("Room &Void Gravity and Hold Access..."), m_View, SLOT(DesignerSetVoidProperties()));

			m_Designer_SiteProperties = m_Designer_PartPropsMenu->addAction(QIcon(":data/ScSite.png"),
					tr("S&ite..."), m_View, SLOT(DesignerSiteProperties()));

			m_Designer_TransporterPadLocs = m_Designer_PartPropsMenu->addAction(QIcon(":data/dp_TransporterPad.png"),
					tr("&Transporter Station Pad Locations..."), m_View, SLOT(DesignerTransporterPadLocs()));

			m_Designer_TurretBase = m_Designer_PartPropsMenu->addAction(QIcon(":data/dp_TurretTopBottom.png"),
					tr("Turret Base &Position..."), m_View, SLOT(DesignerTurretBase()));

			m_Designer_TurretGun = m_Designer_PartPropsMenu->addAction(QIcon(":data/c_HeavyWeapon.png"),
					tr("Turret &Gun Position..."), m_View, SLOT(DesignerTurretGun()));

			m_Designer_WeaponEmitLoc = m_Designer_PartPropsMenu->addAction(QIcon(":data/dp_WeaponEmitter.png"),
					tr("&Weapon Emit Location..."), m_View, SLOT(DesignerWeaponEmitLoc()));
		}

		m_Designer_HullMenu = m_Designer_PartMenu->addMenu(QIcon(":data/ScHull.png"), tr("&Hull"));
		if(m_Designer_HullMenu)
		{
			m_Designer_HullExtrusion = m_Designer_HullMenu->addAction(tr("&Extrusion..."), m_View,
					SLOT(DesignerHullExtrusion()));

			m_Designer_HullSphereLL = m_Designer_HullMenu->addAction(tr("Sphere &Lon/Lat..."), m_View,
					SLOT(DesignerHullSphereLatLong()));

			m_Designer_HullSphereUni = m_Designer_HullMenu->addAction(tr("Sphere &Uniform..."), m_View,
					SLOT(DesignerHullSphereEqDivs()));

			m_Designer_HullSpindle = m_Designer_HullMenu->addAction(tr("&Spindle..."), m_View,
					SLOT(DesignerHullSpindle()));

			m_Designer_HullTubing = m_Designer_HullMenu->addAction(tr("&Tubing..."), m_View,
					SLOT(DesignerHullTubing()));
		}

		m_Designer_RoomMenu = m_Designer_PartMenu->addMenu(QIcon(":data/ScRoom.png"), tr("&Room"));
		if(m_Designer_RoomMenu)
		{
			m_Designer_RoomExtrusion = m_Designer_RoomMenu->addAction(tr("&Extrusion..."), m_View, SLOT(DesignerRoomExtrusion()));

			m_Designer_RoomSphereLL = m_Designer_RoomMenu->addAction(tr("Sphere &Lon/Lat..."), m_View,
					SLOT(DesignerRoomSphereLatLong()));

			m_Designer_RoomSphereUni = m_Designer_RoomMenu->addAction(tr("Sphere &Uniform..."), m_View,
					SLOT(DesignerRoomSphereEqDivs()));

			m_Designer_RoomSpindle = m_Designer_RoomMenu->addAction(tr("&Spindle..."), m_View, SLOT(DesignerRoomSpindle()));

			m_Designer_RoomTubing = m_Designer_RoomMenu->addAction(tr("&Tubing..."), m_View, SLOT(DesignerRoomTubing()));
		}

		m_Designer_LandingGearMenu = m_Designer_PartMenu->addMenu(QIcon(":data/ScLandingGear.png"), tr("Landing &Gear"));
		if(m_Designer_LandingGearMenu)
		{
			m_Designer_LandingGearExtrusion = m_Designer_LandingGearMenu->addAction(tr("&Extrusion..."),
					m_View, SLOT(DesignerLandingGearExtrusion()));

			m_Designer_LandingGearSphereLL = m_Designer_LandingGearMenu->addAction(tr("Sphere &Lon/Lat..."),
					m_View, SLOT(DesignerLandingGearSphereLatLong()));

			m_Designer_LandingGearSphereUni = m_Designer_LandingGearMenu->addAction(tr("Sphere &Uniform..."),
					m_View, SLOT(DesignerLandingGearSphereEqDivs()));

			m_Designer_LandingGearSpindle = m_Designer_LandingGearMenu->addAction(tr("&Spindle..."),
					m_View, SLOT(DesignerLandingGearSpindle()));

			m_Designer_LandingGearTubing = m_Designer_LandingGearMenu->addAction(tr("&Tubing..."),
					m_View, SLOT(DesignerLandingGearTubing()));
		}

		QMenu* intMenu = m_Designer_PartMenu->addMenu(QIcon(":data/ScMesh.png"), tr("&Mesh"));
		if(intMenu)
		{
			m_Designer_InteriorExtrusion = intMenu->addAction(tr("&Extrusion..."),
					m_View, SLOT(DesignerInteriorExtrusion()));

			m_Designer_InteriorSphereLL = intMenu->addAction(tr("Sphere &Lon/Lat..."),
					m_View, SLOT(DesignerInteriorSphereLatLong()));

			m_Designer_InteriorSphereUni = intMenu->addAction(tr("Sphere &Uniform..."),
					m_View, SLOT(DesignerInteriorSphereEqDivs()));

			m_Designer_InteriorSpindle = intMenu->addAction(tr("&Spindle..."),
					m_View, SLOT(DesignerInteriorSpindle()));

			m_Designer_InteriorTubing = intMenu->addAction(tr("&Tubing..."),
					m_View, SLOT(DesignerInteriorTubing()));
		}

		QMenu* jigMenu = m_Designer_PartMenu->addMenu(QIcon(":data/ScJig.png"), tr("&Jig"));
		if(jigMenu)
		{
			m_Designer_CuttingJig = jigMenu->addAction(tr("&Cutting..."), m_View, SLOT(DesignerCuttingJig()));

			m_Designer_DecalJig = jigMenu->addAction(tr("D&ecal..."), m_View, SLOT(DesignerDecalJig()));

			m_Designer_DoorJig = jigMenu->addAction(tr("&Door..."), m_View, SLOT(DesignerDoorJig()));

			m_Designer_LaunchTubeJig = jigMenu->addAction(tr("&Launch Tube..."), m_View, SLOT(DesignerLaunchTubeJig()));

			m_Designer_WindowJig = jigMenu->addAction(tr("&Window..."), m_View, SLOT(DesignerWindowJig()));
		}

		m_Designer_StationsMenu = m_Designer_PartMenu->addMenu(QIcon(":data/dp_CmdConsole.png"), tr("&Stations"));
		if(m_Designer_StationsMenu)
		{
			QMenu* intMenu = m_Designer_StationsMenu->addMenu(tr("&Integrated"));
			if(intMenu)
			{
				m_Designer_StationCapt = intMenu->addAction(QIcon(":data/dp_CmdConsole.png"), tr("&Command..."), m_View,
						SLOT(DesignerStationCapt()));

				m_Designer_StationDesigner = intMenu->addAction(QIcon(":data/dp_DesignerConsole.png"), tr("Designe&r..."), m_View,
						SLOT(DesignerStationDesigner()));

				m_Designer_StationEngineer = intMenu->addAction(QIcon(":data/dp_EngineerConsole.png"), tr("&Engineer..."),
						m_View, SLOT(DesignerStationEngineer()));

				m_Designer_StationFireControl = intMenu->addAction(QIcon(":data/dp_FireControlConsole.png"),
						tr("&Fire Control..."), m_View, SLOT(DesignerStationFireControl()));

				m_Designer_StationHelm = intMenu->addAction(QIcon(":data/dp_PilotConsole.png"), tr("&Helm..."), m_View,
						SLOT(DesignerStationHelm()));

				m_Designer_StationMedic = intMenu->addAction(QIcon(":data/dp_SurgeryConsole.png"), tr("&Medic..."), m_View,
						SLOT(DesignerStationMedic()));

				m_Designer_StationNavigator = intMenu->addAction(QIcon(":data/dp_NavConsole.png"), tr("&Navigator..."), m_View,
						SLOT(DesignerStationNavigator()));

				m_Designer_StationPowerRelay = intMenu->addAction(QIcon(":data/dp_PowerConsole.png"), tr("&Power Relay..."),
						m_View, SLOT(DesignerStationPowerRelay()));

				m_Designer_StationSensor = intMenu->addAction(QIcon(":data/dp_SensorConsole.png"), tr("&Sensor..."), m_View,
						SLOT(DesignerStationSensor()));

				m_Designer_StationShield = intMenu->addAction(QIcon(":data/dp_ShieldConsole.png"), tr("Shiel&d..."), m_View,
						SLOT(DesignerStationShield()));

				m_Designer_StationTransporter = intMenu->addAction(QIcon(":data/dp_TransporterConsole.png"),
						tr("&Transporter..."), m_View, SLOT(DesignerStationTransporter()));

				m_Designer_StationGunner = intMenu->addAction(QIcon(":data/dp_TurretTopBottom.png"), tr("Turret &Gunner..."),
						m_View, SLOT(DesignerStationGunner()));
			}

			QMenu* consolesMenu = m_Designer_StationsMenu->addMenu(tr("Swivel &Consoles w/Chairs"));
			if(consolesMenu)
			{
				m_Designer_ConsoleCommand = consolesMenu->addAction(QIcon(":data/dp_CmdConsole.png"), tr("&Command..."), m_View,
						SLOT(DesignerConsoleCapt()));

				m_Designer_ConsoleEngineer = consolesMenu->addAction(QIcon(":data/dp_EngineerConsole.png"), tr("&Engineer..."),
						m_View, SLOT(DesignerConsoleEngineer()));

				m_Designer_ConsoleFireControl = consolesMenu->addAction(QIcon(":data/dp_FireControlConsole.png"),
						tr("&Fire Control..."), m_View, SLOT(DesignerConsoleFireControl()));

				m_Designer_ConsoleHelm = consolesMenu->addAction(QIcon(":data/dp_PilotConsole.png"), tr("&Helm..."), m_View,
						SLOT(DesignerConsoleHelm()));

				m_Designer_ConsoleMedic = consolesMenu->addAction(QIcon(":data/dp_SurgeryConsole.png"), tr("&Medic..."), m_View,
						SLOT(DesignerConsoleMedic()));

				m_Designer_ConsoleNavigator = consolesMenu->addAction(QIcon(":data/dp_NavConsole.png"), tr("&Navigator..."), m_View,
						SLOT(DesignerConsoleNavigator()));

				m_Designer_ConsolePowerRelay = consolesMenu->addAction(QIcon(":data/dp_PowerConsole.png"), tr("&Power Relay..."),
						m_View, SLOT(DesignerConsolePowerRelay()));

				m_Designer_ConsoleSensor = consolesMenu->addAction(QIcon(":data/dp_SensorConsole.png"), tr("&Sensor..."), m_View,
						SLOT(DesignerConsoleSensor()));

				m_Designer_ConsoleShield = consolesMenu->addAction(QIcon(":data/dp_ShieldConsole.png"), tr("Shiel&d..."), m_View,
						SLOT(DesignerConsoleShield()));

				m_Designer_ConsoleTransporter = consolesMenu->addAction(QIcon(":data/dp_TransporterConsole.png"),
						tr("&Transporter..."), m_View, SLOT(DesignerConsoleTransporter()));
			}

			m_Designer_CitizenPost = m_Designer_StationsMenu->addAction(QIcon(":data/ScCitizen.png"), tr("Citi&zen Post..."),
					m_View, SLOT(DesignerStationCitizen()));

			m_Designer_LivestockPost = m_Designer_StationsMenu->addAction(QIcon(":data/ScLivestock.png"), tr("Li&vestock Post..."),
					m_View, SLOT(DesignerStationLivestock()));

			m_Designer_TroopPost = m_Designer_StationsMenu->addAction(QIcon(":data/ScTroop.png"), tr("T&roop Post..."),
					m_View, SLOT(DesignerStationTroop()));
		}

		m_Designer_PanelsMenu = m_Designer_PartMenu->addMenu(QIcon(":data/ScStatusLifeSupport.png"), tr("Pane&ls"));
		if(m_Designer_PanelsMenu)
		{
			m_Designer_StatusAlert = m_Designer_PanelsMenu->addAction(QIcon(":data/ScStatusAlert.png"), tr("&Alert Status..."),
					m_View, SLOT(DesignerStatusAlert()));

			m_Designer_StatusMission = m_Designer_PanelsMenu->addAction(QIcon(":data/ScStatusMission.png"), tr("Missio&n Status..."),
					m_View, SLOT(DesignerStatusMission()));

			m_Designer_StatusLifeSupport = m_Designer_PanelsMenu->addAction(QIcon(":data/ScStatusLifeSupport.png"),
					tr("&Room Life Support Status..."), m_View, SLOT(DesignerStatusLifeSupport()));

			m_Designer_StatusWorldMap = m_Designer_PanelsMenu->addAction(QIcon(":data/ScStatusWorldMap.png"), tr("W&orld Map..."),
					m_View, SLOT(DesignerStatusWorldMap()));

			m_Designer_PanelsMenu->addSeparator();

			m_Designer_PanelCapacitor = m_Designer_PanelsMenu->addAction(tr("&Capacitor Service Panel..."), m_View,
					SLOT(DesignerPanelCapacitor()));
			m_Designer_PanelCapacitor->setIcon(QIcon(":data/dp_Capacitor.png"));

			m_Designer_PanelFTLDrive = m_Designer_PanelsMenu->addAction(tr("F&TL Drive Service Panel..."), m_View, SLOT(DesignerPanelFTL()));
			m_Designer_PanelFTLDrive->setIcon(QIcon(":data/dp_WormholeDrive.png"));

			m_Designer_PanelLifeSupport = m_Designer_PanelsMenu->addAction(tr("&Life Support Service Panel..."), m_View,
					SLOT(DesignerPanelLifeSupport()));
			m_Designer_PanelLifeSupport->setIcon(QIcon(":data/dp_LifeSupport.png"));

			m_Designer_PanelManeuverDrive = m_Designer_PanelsMenu->addAction(tr("&Maneuver Drive Service Panel..."), m_View,
					SLOT(DesignerPanelManeuver()));
			m_Designer_PanelManeuverDrive->setIcon(QIcon(":data/dp_ManeuverDrive.png"));

			m_Designer_PanelPowerPlant = m_Designer_PanelsMenu->addAction(tr("&Power Plant Service Panel..."), m_View,
					SLOT(DesignerPanelPowerPlant()));
			m_Designer_PanelPowerPlant->setIcon(QIcon(":data/dp_PowerPlant.png"));

			m_Designer_PanelSensor = m_Designer_PanelsMenu->addAction(tr("&Sensor Service Panel..."), m_View,
					SLOT(DesignerPanelSensor()));
			m_Designer_PanelSensor->setIcon(QIcon(":data/dp_Sensor.png"));

			m_Designer_PanelShield = m_Designer_PanelsMenu->addAction(tr("Shiel&d Service Panel..."), m_View,
					SLOT(DesignerPanelShield()));
			m_Designer_PanelShield->setIcon(QIcon(":data/dp_Shield.png"));

			m_Designer_PanelWeapon = m_Designer_PanelsMenu->addAction(QIcon(":data/dp_Weapon.png"),
					tr("&Weapon System Service Panel..."), m_View, SLOT(DesignerPanelWeapon()));
		}

		m_Designer_VehiclesMenu = m_Designer_PartMenu->addMenu(QIcon(":data/ScParkSpaceVehicle.png"), tr("&Vehicles"));
		if(m_Designer_VehiclesMenu)
		{
			m_Designer_PodBay = m_Designer_VehiclesMenu->addAction(QIcon(":data/dp_DropPod.png"), tr("&Pod Bay..."), m_View,
					SLOT(DesignerPodBay()));

			m_Designer_ParkGroundVehicle = m_Designer_VehiclesMenu->addAction(QIcon(":data/ScParkGroundVehicle.png"),
					tr("&Ground Vehicle Parking Spot..."), m_View, SLOT(DesignerParkGroundVehicle()));

			m_Designer_ParkRocket = m_Designer_VehiclesMenu->addAction(QIcon(":data/ScParkRocket.png"),
					tr("Space Roc&ket Parking Spot..."), m_View, SLOT(DesignerParkRocket()));

			m_Designer_ParkSpaceVehicle = m_Designer_VehiclesMenu->addAction(QIcon(":data/ScParkSpaceVehicle.png"),
					tr("&Space Vehicle Parking Spot..."), m_View, SLOT(DesignerParkSpaceVehicle()));

			m_Designer_ParkWaterVehicle = m_Designer_VehiclesMenu->addAction(QIcon(":data/ScParkWaterVehicle.png"),
					tr("&Water Vehicle Parking Spot..."), m_View, SLOT(DesignerParkWaterVehicle()));

			m_Designer_VehiclesMenu->addSeparator();

			m_Designer_LandingSystem = m_Designer_VehiclesMenu->addAction(QIcon(":data/ScLandingSystem.png"),
					tr("Space Vehicle &Recovery System..."), m_View, SLOT(DesignerLandingSystem()));

			m_Designer_LaunchTube = m_Designer_VehiclesMenu->addAction(QIcon(":data/ScLaunchSpaceVehicle.png"),
					tr("Space Vehicle &Launcher..."), m_View, SLOT(DesignerLaunchTube()));

			m_Designer_VehiclesMenu->addSeparator();

			m_Designer_ParkSpacecraft = m_Designer_VehiclesMenu->addAction(QIcon(":data/ScParkSpacecraft.png"),
					tr("Space&craft Parking Spot..."), m_View, SLOT(DesignerParkSpacecraft()));
		}

		m_Designer_DetailsMenu = m_Designer_PartMenu->addMenu(QIcon(":data/Light.png"), tr("De&tails"));
		if(m_Designer_DetailsMenu)
		{
			QMenu* berthsMenu = m_Designer_DetailsMenu->addMenu(QIcon(":data/dp_Berth0.png"), tr("&Berths"));
			if(berthsMenu)
			{
				m_Designer_BerthCapt = berthsMenu->addAction(QIcon(":data/Captain.png"), tr("&Captain..."), m_View,
						SLOT(DesignerBerthCapt()));

				m_Designer_BerthOfficer = berthsMenu->addAction(QIcon(":data/Officer.png"), tr("&Officer..."), m_View,
						SLOT(DesignerBerthOfficer()));

				m_Designer_BerthCrew = berthsMenu->addAction(QIcon(":data/Crewman.png"), tr("&Crew..."), m_View,
						SLOT(DesignerBerthCrew()));

				m_Designer_BerthTroop = berthsMenu->addAction(QIcon(":data/Troop.png"), tr("&Troop..."), m_View,
						SLOT(DesignerBerthTroop()));

				m_Designer_BerthPassenger = berthsMenu->addAction(QIcon(":data/Passenger.png"), tr("&Passenger..."), m_View,
						SLOT(DesignerBerthPassenger()));
			}

			m_Designer_ContrailEmitter = m_Designer_DetailsMenu->addAction(QIcon(":data/dp_ContrailEmitter.png"),
					tr("&Contrail Emitter..."), m_View, SLOT(DesignerEmitterContrail()));

			QMenu* decalsMenu = m_Designer_DetailsMenu->addMenu(QIcon(":data/ScDecal.png"), tr("&Decals"));
			if(decalsMenu)
			{
				m_Designer_EmpireFlag = decalsMenu->addAction(QIcon(":data/ImperiumFlag.png"), tr("Empire &Flag..."), m_View,
						SLOT(DesignerEmpireFlag()));

				m_Designer_EmpireName = decalsMenu->addAction(QIcon(":data/ScName.png"), tr("&Empire Name..."), m_View,
						SLOT(DesignerEmpireName()));

				m_Designer_ShipName = decalsMenu->addAction(QIcon(":data/ScName.png"), tr("&Spacecraft Name..."), m_View,
						SLOT(DesignerShipName()));

				m_Designer_CaptName = decalsMenu->addAction(QIcon(":data/ScName.png"), tr("&Captain's Name..."), m_View,
						SLOT(DesignerCaptName()));

				m_Designer_BerthName = decalsMenu->addAction(QIcon(":data/ScName.png"), tr("&Berth Assignee's Name..."), m_View,
						SLOT(DesignerBerthName()));
			}

			m_Designer_GameTable = m_Designer_DetailsMenu->addAction(QIcon(":data/dp_GameTable.png"), tr("&Game Table..."), m_View,
					SLOT(DesignerGameTable()));

			m_Designer_LightBulb = m_Designer_DetailsMenu->addAction(QIcon(":data/Light.png"), tr("&Light Bulb..."),
					m_View, SLOT(DesignerLightBulb()));

			m_Designer_RemoteCamera = m_Designer_DetailsMenu->addAction(QIcon(":data/ScCamera.png"), tr("&Remote Camera..."),
					m_View, SLOT(DesignerRemoteCamera()));
		}

		m_Designer_LandscapeMenu = m_Designer_PartMenu->addMenu(QIcon(":data/c_Vegetation.png"), tr("Lan&dscape"));
		if(m_Designer_LandscapeMenu)
		{
			m_Designer_Fence = m_Designer_LandscapeMenu->addAction(QIcon(":data/ScFenceWire.png"), tr("&Fence/Wall..."), m_View,
					SLOT(DesignerFence()));

			m_Designer_Shrub = m_Designer_LandscapeMenu->addAction(QIcon(":data/ScShrub.png"), tr("&Shrub..."), m_View,
					SLOT(DesignerShrub()));

			m_Designer_Tree = m_Designer_LandscapeMenu->addAction(QIcon(":data/ScTree.png"), tr("&Tree..."), m_View,
					SLOT(DesignerTree()));

			m_Designer_LandscapeMenu->addSeparator();

			m_Designer_Site = m_Designer_LandscapeMenu->addAction(QIcon(":data/ScSite.png"), tr("S&ite..."), m_View,
					SLOT(DesignerSite()));

			m_Designer_BuildZone = m_Designer_LandscapeMenu->addAction(QIcon(":data/ScBuildZone.png"),
					tr("&Build Zone..."), m_View, SLOT(DesignerBuildZone()));
		}

		m_Designer_PathsMenu = m_Designer_PartMenu->addMenu(QIcon(":data/ScWalkPath.png"), tr("&Paths"));
		if(m_Designer_PathsMenu)
		{
			m_Designer_Barrier = m_Designer_PathsMenu->addAction(QIcon(":data/ScBarrier.png"), tr("&Barrier..."),
					m_View, SLOT(DesignerBarrier()));

			m_Designer_VoidHullMenu = m_Designer_PathsMenu->addMenu(QIcon(":data/ScHullVoid.png"), tr("&Hull Void"));

			m_Designer_VoidHullExtrusion = m_Designer_VoidHullMenu->addAction(tr("&Extrusion..."),
					m_View, SLOT(DesignerVoidHullExtrusion()));

			m_Designer_VoidHullSpindle = m_Designer_VoidHullMenu->addAction(tr("&Spindle..."),
					m_View, SLOT(DesignerVoidHullSpindle()));

			m_Designer_VoidRoomMenu = m_Designer_PathsMenu->addMenu(QIcon(":data/ScRoomVoid.png"), tr("&Room Void"));

			m_Designer_VoidRoomExtrusion = m_Designer_VoidRoomMenu->addAction(tr("&Extrusion..."),
					m_View, SLOT(DesignerVoidRoomExtrusion()));

			m_Designer_VoidRoomSpindle = m_Designer_VoidRoomMenu->addAction(tr("&Spindle..."),
					m_View, SLOT(DesignerVoidRoomSpindle()));

			m_Designer_VoidEnvMenu = m_Designer_PathsMenu->addMenu(QIcon(":data/ScEnvironmentVoid.png"), tr("&Environment Void"));

			m_Designer_VoidEnvironmentExtrusion = m_Designer_VoidEnvMenu->addAction(tr("&Extrusion..."),
					m_View, SLOT(DesignerVoidEnvironmentExtrusion()));

			m_Designer_VoidEnvironmentSpindle = m_Designer_VoidEnvMenu->addAction(tr("&Spindle..."),
					m_View, SLOT(DesignerVoidEnvironmentSpindle()));

			m_Designer_WalkPath = m_Designer_PathsMenu->addAction(QIcon(":data/ScWalkPath.png"), tr("&Walk Path..."),
					m_View, SLOT(DesignerWalkPath()));

			m_Designer_LadderPath = m_Designer_PathsMenu->addAction(QIcon(":data/ScLadderPath.png"), tr("&Ladder Path..."),
					m_View, SLOT(DesignerLadderPath()));
		}

		m_Designer_Sketch = m_Designer_PartMenu->addAction(QIcon(":data/ScSketch.png"), tr("Sk&etch..."),
				m_View, SLOT(DesignerSketch()));

		m_Designer_PartMenu->addSeparator();

		m_Designer_AlignLiftStop = m_Designer_PartMenu->addAction(QIcon(":data/ScLift.png"), tr("Align Turbo Li&ft Stop..."),
				m_View, SLOT(DesignerAlignLiftStop()));

		m_Designer_Associate = m_Designer_PartMenu->addAction(tr("&Associate Parts"),
				m_View, SLOT(DesignerAssociate()));

		m_Designer_Dissociate = m_Designer_PartMenu->addAction(tr("Diss&ociate Parts"),
				m_View, SLOT(DesignerDissociate()));

		m_Designer_JigCut = m_Designer_PartMenu->addAction(QIcon(":data/ScJigCut.png"), tr("Jig &Cut..."), m_View,
				SLOT(DesignerJigCut()));

		m_Designer_Join = m_Designer_PartMenu->addAction(QIcon(":data/ScJoin.png"), tr("Joi&n"), m_View, SLOT(DesignerJoinParts()));

		QMenu* makeMenu = m_Designer_PartMenu->addMenu(QIcon(":data/ScMake.png"), tr("Ma&ke"));
		if(makeMenu)
		{
			m_Designer_MakeDecal = makeMenu->addAction(QIcon(":data/ScDecal.png"), tr("D&ecal"),
					m_View, SLOT(DesignerMakeDecal()));

			m_Designer_MakeDoor = makeMenu->addAction(QIcon(":data/ScDoorHull.png"), tr("&Door"),
					m_View, SLOT(DesignerMakeDoor()));

			m_Designer_MakeHull = makeMenu->addAction(QIcon(":data/ScHull.png"), tr("&Hull"),
					m_View, SLOT(DesignerMakeHull()));

			m_Designer_MakeLandingGear = makeMenu->addAction(QIcon(":data/ScLandingGear.png"), tr("Landing &Gear"),
					m_View, SLOT(DesignerMakeLandingGear()));

			m_Designer_MakeMesh = makeMenu->addAction(QIcon(":data/ScMesh.png"), tr("&Mesh"),
					m_View, SLOT(DesignerMakeMesh()));

			m_Designer_MakeRoom = makeMenu->addAction(QIcon(":data/ScRoom.png"), tr("&Room"),
					m_View, SLOT(DesignerMakeRoom()));

			m_Designer_MakeTurboLift = makeMenu->addAction(QIcon(":data/ScLift.png"), tr("Turbo Li&ft"),
					m_View, SLOT(DesignerMakeTurboLift()));

			m_Designer_MakeTurret = makeMenu->addAction(QIcon(":data/dp_TurretTopBottom.png"), tr("&Turret..."),
					m_View, SLOT(DesignerMakeTurret()));

			m_Designer_MakeWindow = makeMenu->addAction(QIcon(":data/ScWindow.png"), tr("&Window..."),
					m_View, SLOT(DesignerMakeWindow()));

			makeMenu->addSeparator();

			m_Designer_MakeLightLens = makeMenu->addAction(QIcon(":data/ScHeadlight.png"), tr("&Light Lens..."),
					m_View, SLOT(DesignerMakeLightLens()));

			m_Designer_MakeNavLightLens = makeMenu->addAction(QIcon(":data/ScNavLight.png"), tr("&Navigation Light Lens..."),
					m_View, SLOT(DesignerMakeNavLightLens()));

			m_Designer_MakeRotatingBeaconLens = makeMenu->addAction(QIcon(":data/MsgSOS.png"), tr("Rotating &Beacon Lens"),
					m_View, SLOT(DesignerMakeRotatingBeaconLens()));

			m_Designer_MakeStrobeLightLens = makeMenu->addAction(QIcon(":data/SpotNeutral.png"), tr("&Strobe Lens"),
					m_View, SLOT(DesignerMakeStrobeLens()));
		}

		m_Designer_CopySymmetrical = m_Designer_PartMenu->addAction(QIcon(":data/ScSymmetry.png"), tr("S&ymmetrical Copy..."),
				m_View, SLOT(DesignerCopySymmetrical()));
	}

	m_Designer_FaceMenu = menuBar()->addMenu(tr("Fa&ce"));
	if(m_Designer_FaceMenu)
	{
		m_Designer_ObjectMode = m_Designer_FaceMenu->addAction(QIcon(":data/ObjectMode.png"), tr("Face &Mode"), this,
				SLOT(DesignerMenuObjectMode()));
		m_Designer_ObjectMode->setCheckable(true);
		m_Designer_ObjectMode->setChecked(m_View->DesignerObjectMode());

		m_Designer_ObjectIsolate = m_Designer_FaceMenu->addAction(QIcon(":data/ScIsolate.png"), tr("&Isolate Parts"), this,
				SLOT(DesignerMenuObjectIsolate()));
		m_Designer_ObjectIsolate->setCheckable(true);
		m_Designer_ObjectIsolate->setChecked(m_View->DesignerObjectIsolate());

		m_Designer_FaceMenu->addSeparator();

		QMenu* objectAddMenu = m_Designer_FaceMenu->addMenu(QIcon(":data/new12x12.png"), tr("&Add"));
		if(objectAddMenu)
		{
			m_Designer_ObjectTriangle = objectAddMenu->addAction(tr("T&riangle..."),
					m_View, SLOT(DesignerObjectTriangle()));

			m_Designer_ObjectQuad = objectAddMenu->addAction(tr("&Quad..."),
					m_View, SLOT(DesignerObjectQuad()));

			m_Designer_ObjectFace = objectAddMenu->addAction(tr("&Face..."),
					m_View, SLOT(DesignerObjectFace()));

			objectAddMenu->addSeparator();

			m_Designer_ObjectExtrusion = objectAddMenu->addAction(tr("&Extrusion..."),
					m_View, SLOT(DesignerObjectExtrusion()));

			m_Designer_ObjectSphereLL = objectAddMenu->addAction(tr("Sphere &Lon/Lat..."),
					m_View, SLOT(DesignerObjectSphereLatLong()));

			m_Designer_ObjectSphereUni = objectAddMenu->addAction(tr("Sphere &Uniform..."),
					m_View, SLOT(DesignerObjectSphereEqDivs()));

			m_Designer_ObjectSpindle = objectAddMenu->addAction(tr("&Spindle..."),
					m_View, SLOT(DesignerObjectSpindle()));

			m_Designer_ObjectTubing = objectAddMenu->addAction(tr("&Tubing..."),
					m_View, SLOT(DesignerObjectTubing()));
		}

		QMenu* finishMenu  = m_Designer_FaceMenu->addMenu(tr("&Finish"));
		if(finishMenu )
		{
			m_Designer_Paint = finishMenu->addAction(tr("&Paint..."),
					m_View, SLOT(DesignerPaint()));

			m_Designer_Polish = finishMenu->addAction(tr("P&olish..."),
					m_View, SLOT(DesignerPolish()));

			m_Designer_Glow = finishMenu->addAction(tr("&Glow..."),
					m_View, SLOT(DesignerGlow()));

			m_Designer_SelfIllumination = finishMenu->addAction(tr("Self &Illumination..."),
					m_View, SLOT(DesignerSelfIllumination()));

			m_Designer_Opacity = finishMenu->addAction(tr("&Transparency..."),
					m_View, SLOT(DesignerOpacity()));

			finishMenu->addSeparator();

			m_Designer_EffectIntensity = finishMenu->addAction(tr("&Effect Intensity..."),
					m_View, SLOT(DesignerEffectIntensity()));
		}

		m_Designer_FaceMenu->addSeparator();

		m_Designer_CutFace = m_Designer_FaceMenu->addAction(tr("&Cut Face..."), m_View, SLOT(DesignerCutFace()));

		m_Designer_DivideFaces = m_Designer_FaceMenu->addAction(tr("&Divide Faces..."), m_View, SLOT(DesignerDivideFaces()));

		m_Designer_Flatten = m_Designer_FaceMenu->addAction(tr("Flatte&n Vertices"), m_View, SLOT(DesignerFlatten()));

		m_Designer_FractureFaces = m_Designer_FaceMenu->addAction(tr("Frac&ture Faces"), m_View, SLOT(DesignerFractureFaces()));

		m_Designer_FuseFaces = m_Designer_FaceMenu->addAction(tr("F&use Faces"), m_View, SLOT(DesignerFuseFaces()));

		m_Designer_LoopCut = m_Designer_FaceMenu->addAction(tr("&Loop Cut..."), m_View, SLOT(DesignerLoopCut()));

		m_Designer_PunchFace = m_Designer_FaceMenu->addAction(tr("Punc&h Face..."), m_View, SLOT(DesignerPunchFace()));

		m_Designer_PushPull = m_Designer_FaceMenu->addAction(tr("&Push/Pull..."), m_View, SLOT(DesignerPushPull()));

		m_Designer_ReverseFaces = m_Designer_FaceMenu->addAction(tr("&Reverse Faces"), m_View, SLOT(DesignerReverseFaces()));

		m_Designer_Smooth = m_Designer_FaceMenu->addAction(tr("Sm&ooth Vertices"), m_View, SLOT(DesignerSmooth()));

		m_Designer_SplitFaces = m_Designer_FaceMenu->addAction(tr("&Split Faces"), m_View, SLOT(DesignerSplitFaces()));
	}

	QMenu* helperMenu = menuBar()->addMenu(tr("He&lpers"));
	if(helperMenu)
	{
		QMenu* gridSaveMenu = helperMenu->addMenu(tr("Save Grid Pos&ition"));
		if(gridSaveMenu)
		{
			m_Designer_GridPositionSave1 = gridSaveMenu->addAction(tr("Save Position &1"),
					m_View, SLOT(DesignerGridPositionSave1()));
			m_Designer_GridPositionSave2 = gridSaveMenu->addAction(tr("Save Position &2"),
					m_View, SLOT(DesignerGridPositionSave2()));
			m_Designer_GridPositionSave3 = gridSaveMenu->addAction(tr("Save Position &3"),
					m_View, SLOT(DesignerGridPositionSave3()));
			m_Designer_GridPositionSave4 = gridSaveMenu->addAction(tr("Save Position &4"),
					m_View, SLOT(DesignerGridPositionSave4()));
			m_Designer_GridPositionSave5 = gridSaveMenu->addAction(tr("Save Position &5"),
					m_View, SLOT(DesignerGridPositionSave5()));
			m_Designer_GridPositionSave6 = gridSaveMenu->addAction(tr("Save Position &6"),
					m_View, SLOT(DesignerGridPositionSave6()));
			m_Designer_GridPositionSave7 = gridSaveMenu->addAction(tr("Save Position &7"),
					m_View, SLOT(DesignerGridPositionSave7()));
			m_Designer_GridPositionSave8 = gridSaveMenu->addAction(tr("Save Position &8"),
					m_View, SLOT(DesignerGridPositionSave8()));
			m_Designer_GridPositionSave9 = gridSaveMenu->addAction(tr("Save Position &9"),
					m_View, SLOT(DesignerGridPositionSave9()));
		}

		QMenu* gridLoadMenu = helperMenu->addMenu(tr("&Restore Grid Position"));
		if(gridLoadMenu)
		{
			m_Designer_GridPositionOrigin = gridLoadMenu->addAction(tr("Position to &Origin"),
					m_View, SLOT(DesignerGridPositionOrigin()));

			gridLoadMenu->addSeparator();

			m_Designer_GridPositionLoad1 = gridLoadMenu->addAction(tr("Restore Position &1"),
					m_View, SLOT(DesignerGridPositionLoad1()));
			m_Designer_GridPositionLoad2 = gridLoadMenu->addAction(tr("Restore Position &2"),
					m_View, SLOT(DesignerGridPositionLoad2()));
			m_Designer_GridPositionLoad3 = gridLoadMenu->addAction(tr("Restore Position &3"),
					m_View, SLOT(DesignerGridPositionLoad3()));
			m_Designer_GridPositionLoad4 = gridLoadMenu->addAction(tr("Restore Position &4"),
					m_View, SLOT(DesignerGridPositionLoad4()));
			m_Designer_GridPositionLoad5 = gridLoadMenu->addAction(tr("Restore Position &5"),
					m_View, SLOT(DesignerGridPositionLoad5()));
			m_Designer_GridPositionLoad6 = gridLoadMenu->addAction(tr("Restore Position &6"),
					m_View, SLOT(DesignerGridPositionLoad6()));
			m_Designer_GridPositionLoad7 = gridLoadMenu->addAction(tr("Restore Position &7"),
					m_View, SLOT(DesignerGridPositionLoad7()));
			m_Designer_GridPositionLoad8 = gridLoadMenu->addAction(tr("Restore Position &8"),
					m_View, SLOT(DesignerGridPositionLoad8()));
			m_Designer_GridPositionLoad9 = gridLoadMenu->addAction(tr("Restore Position &9"),
					m_View, SLOT(DesignerGridPositionLoad9()));
		}

		helperMenu->addSeparator();

		m_Designer_FindVertices = helperMenu->addAction(QIcon(":data/DesignerSelVertices.png"), tr("Find &Vertices"), this,
				SLOT(DesignerMenuSetFindVertices()));
		m_Designer_FindVertices->setCheckable(true);
		m_Designer_FindVertices->setChecked(g_AuClient.DesignerFindVertices());

		m_Designer_FindEdges = helperMenu->addAction(QIcon(":data/DesignerSelEdges.png"), tr("Find &Edges"), this,
				SLOT(DesignerMenuSetFindEdges()));
		m_Designer_FindEdges->setCheckable(true);
		m_Designer_FindEdges->setChecked(g_AuClient.DesignerFindEdges());

		m_Designer_FindFaces = helperMenu->addAction(QIcon(":data/DesignerSelFaces.png"), tr("Find &Faces"), this,
				SLOT(DesignerMenuSetFindFaces()));
		m_Designer_FindFaces->setCheckable(true);
		m_Designer_FindFaces->setChecked(g_AuClient.DesignerFindFaces());

		helperMenu->addSeparator();

		m_Designer_GridMove = helperMenu->addAction(QIcon(":data/dp_PivotPoint.png"), tr("&Move Grid..."), m_View,
				SLOT(DesignerGridMove()));

		m_Designer_GridXY = helperMenu->addAction(QIcon(":data/ScGridXY.png"), tr("&XY Grid"), m_View,
				SLOT(DesignerGridXY()));

		m_Designer_GridXZ = helperMenu->addAction(QIcon(":data/ScGridXZ.png"), tr("XZ &Grid"), m_View,
				SLOT(DesignerGridXZ()));

		m_Designer_GridYZ = helperMenu->addAction(QIcon(":data/ScGridYZ.png"), tr("&YZ Grid"), m_View,
				SLOT(DesignerGridYZ()));

		m_Designer_Grid3pt = helperMenu->addAction(QIcon(":data/DesignGrid3pt.png"), tr("&Position Grid..."), m_View,
				SLOT(DesignerGrid3pt()));

		helperMenu->addSeparator();

		m_Designer_ShowGrid = helperMenu->addAction(QIcon(":data/dp_ShowDesignPlane.png"), tr("Sho&w Grid"), this,
				SLOT(DesignerGridToggle()));
		m_Designer_ShowGrid->setCheckable(true);
		m_Designer_ShowGrid->setChecked(g_AuClient.DesignerShowGrid());

		m_Designer_GridFollowsWork = helperMenu->addAction(QIcon(":data/DesignPlaneFollow.png"), tr("Grid F&ollows Work"), this,
				SLOT(DesignerMenuSetGridFollowsWork()));
		m_Designer_GridFollowsWork->setCheckable(true);
		m_Designer_GridFollowsWork->setChecked(g_AuClient.DesignerGridFollowsWork());

		helperMenu->addSeparator();

		m_Designer_SnapToGrid = helperMenu->addAction(QIcon(":data/DesignerSnapToGrid.png"), tr("&Snap to Grid"), this,
				SLOT(DesignerMenuSetSnapToGrid()));
		m_Designer_SnapToGrid->setCheckable(true);
		m_Designer_SnapToGrid->setChecked(g_AuClient.DesignerSnapToGrid());

		m_Designer_SnapToEdges = helperMenu->addAction(QIcon(":data/DesignerSnapToEdges.png"), tr("S&nap to Edges"), this,
				SLOT(DesignerMenuSetSnapToEdges()));
		m_Designer_SnapToEdges->setCheckable(true);
		m_Designer_SnapToEdges->setChecked(g_AuClient.DesignerSnapToEdges());

		m_Designer_SnapToVertices = helperMenu->addAction(QIcon(":data/DesignerSnapToVertices.png"), tr("Sn&ap to Vertices"), this,
				SLOT(DesignerMenuSetSnapToVertices()));
		m_Designer_SnapToVertices->setCheckable(true);
		m_Designer_SnapToVertices->setChecked(g_AuClient.DesignerSnapToVertices());

		helperMenu->addSeparator();

		QMenu* boxMenu = helperMenu->addMenu(QIcon(":data/ScBoundBox.png"), tr("&Bounding Box"));
		if(boxMenu)
		{
			m_Designer_BdHullMax = boxMenu->addAction(QIcon(":data/City.png"), tr("Building Structure &Design Limit"),
					m_View, SLOT(DesignerBdHullMax()));

			m_Designer_SiteMax = boxMenu->addAction(QIcon(":data/ScSite.png"), tr("Building Structure &Site Limit"),
					m_View, SLOT(DesignerSiteMax()));

			boxMenu->addSeparator();

			m_Designer_BoundGround = boxMenu->addAction(QIcon(":data/ScBoundBoxG.png"), tr("Spacecraft &Ground Build Limit"),
					m_View, SLOT(DesignerBoundGround()));

			m_Designer_BoundMax = boxMenu->addAction(QIcon(":data/ScBoundBox.png"), tr("Spacecraft &Max Design Limit"),
					m_View, SLOT(DesignerBoundMax()));
		}

		helperMenu->addSeparator();

		m_Designer_ChangeDNA = helperMenu->addAction(QIcon(":data/DNA.png"), tr("Change My &DNA..."),
				m_View, SLOT(DesignerChangeDNA()));
	}

	QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
	if(viewMenu)
	{
		m_Designer_GoToSelection = viewMenu->addAction(QIcon(":data/MsgTransport.png"), tr("&Go To Selection"), m_View,
				SLOT(DesignerGoToSelection()));

		viewMenu->addSeparator();

		QMenu* posSaveMenu = viewMenu->addMenu(QIcon(":data/OrderComeHereExactly.png"), tr("Save My Pos&ition"));
		if(posSaveMenu)
		{
			m_Designer_ViewPositionSave1 = posSaveMenu->addAction(tr("Save Position &1"),
					m_View, SLOT(DesignerViewPositionSave1()));
			m_Designer_ViewPositionSave2 = posSaveMenu->addAction(tr("Save Position &2"),
					m_View, SLOT(DesignerViewPositionSave2()));
			m_Designer_ViewPositionSave3 = posSaveMenu->addAction(tr("Save Position &3"),
					m_View, SLOT(DesignerViewPositionSave3()));
			m_Designer_ViewPositionSave4 = posSaveMenu->addAction(tr("Save Position &4"),
					m_View, SLOT(DesignerViewPositionSave4()));
			m_Designer_ViewPositionSave5 = posSaveMenu->addAction(tr("Save Position &5"),
					m_View, SLOT(DesignerViewPositionSave5()));
			m_Designer_ViewPositionSave6 = posSaveMenu->addAction(tr("Save Position &6"),
					m_View, SLOT(DesignerViewPositionSave6()));
			m_Designer_ViewPositionSave7 = posSaveMenu->addAction(tr("Save Position &7"),
					m_View, SLOT(DesignerViewPositionSave7()));
			m_Designer_ViewPositionSave8 = posSaveMenu->addAction(tr("Save Position &8"),
					m_View, SLOT(DesignerViewPositionSave8()));
			m_Designer_ViewPositionSave9 = posSaveMenu->addAction(tr("Save Position &9"),
					m_View, SLOT(DesignerViewPositionSave9()));
		}

		QMenu* posLoadMenu = viewMenu->addMenu(QIcon(":data/OrderFollowMe.png"), tr("&Restore My Position"));
		if(posLoadMenu)
		{
			m_Designer_ViewPositionOrigin = posLoadMenu->addAction(tr("Position to &Origin"),
					m_View, SLOT(DesignerViewPositionOrigin()));

			m_Designer_ViewPositionGrid = posLoadMenu->addAction(tr("Position to &Grid"),
					m_View, SLOT(DesignerViewPositionGrid()));

			posLoadMenu->addSeparator();

			m_Designer_ViewPositionLoad1 = posLoadMenu->addAction(tr("Restore Position &1"),
					m_View, SLOT(DesignerViewPositionLoad1()));
			m_Designer_ViewPositionLoad2 = posLoadMenu->addAction(tr("Restore Position &2"),
					m_View, SLOT(DesignerViewPositionLoad2()));
			m_Designer_ViewPositionLoad3 = posLoadMenu->addAction(tr("Restore Position &3"),
					m_View, SLOT(DesignerViewPositionLoad3()));
			m_Designer_ViewPositionLoad4 = posLoadMenu->addAction(tr("Restore Position &4"),
					m_View, SLOT(DesignerViewPositionLoad4()));
			m_Designer_ViewPositionLoad5 = posLoadMenu->addAction(tr("Restore Position &5"),
					m_View, SLOT(DesignerViewPositionLoad5()));
			m_Designer_ViewPositionLoad6 = posLoadMenu->addAction(tr("Restore Position &6"),
					m_View, SLOT(DesignerViewPositionLoad6()));
			m_Designer_ViewPositionLoad7 = posLoadMenu->addAction(tr("Restore Position &7"),
					m_View, SLOT(DesignerViewPositionLoad7()));
			m_Designer_ViewPositionLoad8 = posLoadMenu->addAction(tr("Restore Position &8"),
					m_View, SLOT(DesignerViewPositionLoad8()));
			m_Designer_ViewPositionLoad9 = posLoadMenu->addAction(tr("Restore Position &9"),
					m_View, SLOT(DesignerViewPositionLoad9()));
		}

		viewMenu->addSeparator();

		m_Designer_ShowBacks = viewMenu->addAction(tr("Show &Back Faces"), this, SLOT(DesignerMenuSetShowBacks()));
		m_Designer_ShowBacks->setCheckable(true);
		m_Designer_ShowBacks->setChecked(g_AuClient.DesignerShowBacks());

		m_Designer_ShowEdges = viewMenu->addAction(tr("Show &Edges"), this, SLOT(DesignerMenuSetShowEdges()));
		m_Designer_ShowEdges->setCheckable(true);
		m_Designer_ShowEdges->setChecked(g_AuClient.DesignerShowEdges());

		m_Designer_ShowFaces = viewMenu->addAction(tr("Show &Faces"), this, SLOT(DesignerMenuSetShowFaces()));
		m_Designer_ShowFaces->setCheckable(true);
		m_Designer_ShowFaces->setChecked(g_AuClient.DesignerShowFaces());

		m_Designer_ShowGround = viewMenu->addAction(tr("Show Grou&nd"), this, SLOT(DesignerMenuSetShowGround()));
		m_Designer_ShowGround->setCheckable(true);
		m_Designer_ShowGround->setChecked(g_AuClient.DesignerShowGround());

		m_Designer_ShowSkyBox = viewMenu->addAction(tr("Show S&ky Box"), this, SLOT(DesignerMenuSetShowSkyBox()));
		m_Designer_ShowSkyBox->setCheckable(true);
		m_Designer_ShowSkyBox->setChecked(g_AuClient.DesignerShowSkyBox());

		viewMenu->addSeparator();

		m_Designer_FullScreen = viewMenu->addAction(QIcon(":data/ToggleFull.png"), tr("Fu&ll Screen"),
				this, SLOT(ToggleFullScreen()));
		//, QKeySequence::FullScreen);
		m_Designer_FullScreen->setCheckable(true);
		m_Designer_FullScreen->setChecked(isFullScreen());

		viewMenu->addSeparator();

		viewMenu->addAction(QIcon(":data/SoHDesignerIcon.png"), tr("Parts"), this, SLOT(ToggleDesignPartsUi()));

		viewMenu->addAction(QIcon(":data/ScTexPalette.png"), tr("Texture &Wrapper"), this, SLOT(ToggleDesignTextureWrapUi()));
	}

	QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
	if(helpMenu)
	{
		m_Designer_Wiki = helpMenu->addAction(tr("Online &Wiki..."), this, SLOT(ShowWikiDesigner()),
				QKeySequence::HelpContents);
		m_Designer_Wiki->setIcon(QIcon(":data/SoHDesignerIcon.png"));

		m_Designer_Videos = helpMenu->addAction(tr("Online &Videos..."), this, SLOT(ShowVideos()));
		m_Designer_Videos->setIcon(QIcon(":data/FilmReel64.png"));

		helpMenu->addSeparator();

		m_Designer_BuildingCodes = helpMenu->addAction(tr("&Building Codes..."), this, SLOT(BuildingCodes()));
		m_Designer_BuildingCodes->setIcon(QIcon(":data/OrderRead.png"));

		helpMenu->addSeparator();

		m_Designer_About = helpMenu->addAction(tr("&About..."), this, SLOT(HelpAbout()));
		m_Designer_About->setIcon(QIcon(auApp->Icon32()));
	}

	menuBar()->hide();
}

void AuDesignerFrame::DesignerEnableControls()
{
	if(!m_Designer_BerthCapt)
		return;	// We're not initialized yet.

	// Preview state tracks view, on menu and designer bar.
	bool preview = m_View->IsDesignPreview();
	if(m_Designer_PreviewToggle->isChecked() != preview)
	{
		m_Designer_PreviewToggle->blockSignals(true);
		m_Designer_PreviewToggle->setChecked(preview);
		m_Designer_PreviewToggle->blockSignals(false);
	}

	if(m_DesignerBar)
	{
		m_DesignerBar->SetPreview(preview);
		m_DesignerBar->EnableControls();
	}
	if(m_DesignerBar2)
		m_DesignerBar2->EnableControls();
	if(m_DesignerPartsDock)
		m_DesignerPartsDock->EnableControls();

	DaDesignBase::DESIGNMODE designMode = (DaDesignBase::DESIGNMODE)m_View->DesignMode();
	BUILDING_STRUCTURE bdStructure = m_View->DesignBdStructure();
	bool bdpreviewok = preview && designMode == DaDesignBase::DESIGN_Building;	// Ok to place building design on preview terrain.
	bool dok = !preview && designMode > DaDesignBase::DESIGN_Unknown;			// True if mode is valid for designing things.
	bool copyOk = !m_View->DesignerIsViewOnly() || m_View->SceneAtomId() == AuId::Haxus();
	bool objectMode = m_View->DesignerObjectMode();
	bool partSelected = !m_View->DesignerPartObjectSelection().empty();			// any whole part
	bool objectSelected = m_View->DesignerPartObjectSelection().HasObjectSelection();	// face, edge or vertex
	bool faceSelected = m_View->DesignerPartObjectSelection().HasFaceSelection();
	bool vertexSelected = m_View->DesignerPartObjectSelection().HasVertexSelection();
	bool hasModeSelection = (!objectMode && partSelected) || (objectMode && objectSelected);
	bool hasSealedPartsSelected = m_View->HasSealedPartsSelected();
	QSet<quint8> viewIds = m_View->DesignerViewPositions();
	QSet<quint8> gridIds = m_View->DesignerGridPositions();

	//m_Designer_About;
	m_Designer_Align->setEnabled(dok && hasModeSelection);
	m_Designer_AlignLiftStop->setEnabled(dok && !objectMode && hasModeSelection);
	m_Designer_Associate->setEnabled(dok && !objectMode && hasModeSelection);
	m_Designer_Barrier->setEnabled(dok && !objectMode);
	m_Designer_BdHullMax->setEnabled(dok);
	m_Designer_BerthCapt->setEnabled(dok && !objectMode);
	m_Designer_BerthCrew->setEnabled(dok && !objectMode);
	m_Designer_BerthName->setEnabled(dok && !objectMode);
	m_Designer_BerthOfficer->setEnabled(dok && !objectMode);
	m_Designer_BerthPassenger->setEnabled(dok && !objectMode);
	m_Designer_BerthTroop->setEnabled(dok && !objectMode);
	m_Designer_BoundGround->setEnabled(dok);
	m_Designer_BoundMax->setEnabled(dok);
	//m_Designer_BuildingCodes
	m_Designer_BuildZone->setEnabled(dok && !objectMode);
	m_Designer_CaptName->setEnabled(dok && !objectMode);
	m_Designer_ChangeDNA->setEnabled(auApp->IsSingleUserDesignerMode());
	m_Designer_CitizenPost->setEnabled(dok && !objectMode);
	m_Designer_ConsoleCommand->setEnabled(dok && !objectMode);
	m_Designer_ConsoleEngineer->setEnabled(dok && !objectMode);
	m_Designer_ConsoleFireControl->setEnabled(dok && !objectMode);
	m_Designer_ConsoleHelm->setEnabled(dok && !objectMode);
	m_Designer_ConsoleMedic->setEnabled(dok && !objectMode);
	m_Designer_ConsoleNavigator->setEnabled(dok && !objectMode);
	m_Designer_ConsolePowerRelay->setEnabled(dok && !objectMode);
	m_Designer_ConsoleSensor->setEnabled(dok && !objectMode);
	m_Designer_ConsoleShield->setEnabled(dok && !objectMode);
	m_Designer_ConsoleTransporter->setEnabled(dok && !objectMode);
	m_Designer_ContrailEmitter->setEnabled(dok && !objectMode);
	m_Designer_Copy->setEnabled(dok && copyOk);
	m_Designer_CopySymmetrical->setEnabled(dok && !objectMode && hasModeSelection);
	m_Designer_Cut->setEnabled(dok && copyOk);
	m_Designer_CutFace->setEnabled(dok && objectMode);
	m_Designer_CuttingJig->setEnabled(dok && !objectMode);
	m_Designer_DecalJig->setEnabled(dok && !objectMode);
	m_Designer_Delete->setEnabled(dok);
	//m_Designer_Depart;
	m_Designer_DeselectAll->setEnabled(dok && hasModeSelection);
	m_Designer_DesignFinalize->setEnabled(dok);
	m_Designer_DesignFinalizeReport->setEnabled(dok);
	m_Designer_DesignProperties->setEnabled(dok);
	m_Designer_Dissociate->setEnabled(dok && !objectMode && hasModeSelection);
	m_Designer_DivideFaces->setEnabled(dok && objectMode && faceSelected);
	m_Designer_DoorJig->setEnabled(dok && !objectMode);
	m_Designer_DoorProperties->setEnabled(dok && !objectMode && hasModeSelection);
	m_Designer_EffectIntensity->setEnabled(dok && objectMode && vertexSelected);
	m_Designer_EmpireFlag->setEnabled(dok && !objectMode);
	m_Designer_EmpireName->setEnabled(dok && !objectMode);
	//m_Designer_Exit;
	m_Designer_ExportSTL->setEnabled(dok);
	m_Designer_Fence->setEnabled(dok && !objectMode);
	m_Designer_FindEdges->setEnabled(dok && objectMode);
	m_Designer_FindFaces->setEnabled(dok && objectMode);
	m_Designer_FindVertices->setEnabled(dok && objectMode);
	m_Designer_Flatten->setEnabled(dok && objectMode && faceSelected && vertexSelected);
	m_Designer_FractureFaces->setEnabled(dok && objectMode && faceSelected);
	//m_Designer_FullScreen;
	m_Designer_FuseFaces->setEnabled(dok && objectMode && faceSelected);
	m_Designer_GameTable->setEnabled(dok && !objectMode);
	if(m_Designer_GearOpen)
		m_Designer_GearOpen->setEnabled(true);
	if(m_Designer_GearSaveBd)
		m_Designer_GearSaveBd->setEnabled(dok);
	if(m_Designer_GearSaveSc)
		m_Designer_GearSaveSc->setEnabled(dok);
	m_Designer_Glow->setEnabled(dok && objectMode && vertexSelected);
	m_Designer_GoToSelection->setEnabled(!objectMode && partSelected);
	m_Designer_Grab->setEnabled(dok && hasModeSelection);
	m_Designer_GravityToggle->setEnabled(dok);
	m_Designer_Grid3pt->setEnabled(dok);
	m_Designer_GridFollowsWork->setEnabled(dok);
	m_Designer_GridMove->setEnabled(dok);
	m_Designer_GridPositionLoad1->setEnabled(dok && gridIds.contains(1));
	m_Designer_GridPositionLoad2->setEnabled(dok && gridIds.contains(2));
	m_Designer_GridPositionLoad3->setEnabled(dok && gridIds.contains(3));
	m_Designer_GridPositionLoad4->setEnabled(dok && gridIds.contains(4));
	m_Designer_GridPositionLoad5->setEnabled(dok && gridIds.contains(5));
	m_Designer_GridPositionLoad6->setEnabled(dok && gridIds.contains(6));
	m_Designer_GridPositionLoad7->setEnabled(dok && gridIds.contains(7));
	m_Designer_GridPositionLoad8->setEnabled(dok && gridIds.contains(8));
	m_Designer_GridPositionLoad9->setEnabled(dok && gridIds.contains(9));
	m_Designer_GridPositionOrigin->setEnabled(dok);
	m_Designer_GridPositionSave1->setEnabled(dok);
	m_Designer_GridPositionSave2->setEnabled(dok);
	m_Designer_GridPositionSave3->setEnabled(dok);
	m_Designer_GridPositionSave4->setEnabled(dok);
	m_Designer_GridPositionSave5->setEnabled(dok);
	m_Designer_GridPositionSave6->setEnabled(dok);
	m_Designer_GridPositionSave7->setEnabled(dok);
	m_Designer_GridPositionSave8->setEnabled(dok);
	m_Designer_GridPositionSave9->setEnabled(dok);
	m_Designer_GridXY->setEnabled(dok);
	m_Designer_GridXZ->setEnabled(dok);
	m_Designer_GridYZ->setEnabled(dok);
	m_Designer_HullExtrusion->setEnabled(dok && !objectMode);
	m_Designer_HullSphereLL->setEnabled(dok && !objectMode);
	m_Designer_HullSphereUni->setEnabled(dok && !objectMode);
	m_Designer_HullSpindle->setEnabled(dok && !objectMode);
	m_Designer_HullTubing->setEnabled(dok && !objectMode);
	m_Designer_Import3DS->setEnabled(dok);
	m_Designer_ImportSTL->setEnabled(dok);
	m_Designer_InteriorExtrusion->setEnabled(dok && !objectMode);
	m_Designer_InteriorSphereLL->setEnabled(dok && !objectMode);
	m_Designer_InteriorSphereUni->setEnabled(dok && !objectMode);
	m_Designer_InteriorSpindle->setEnabled(dok && !objectMode);
	m_Designer_InteriorTubing->setEnabled(dok && !objectMode);
	m_Designer_InvertSelection->setEnabled(dok);
	m_Designer_JigCut->setEnabled(dok && !objectMode && hasModeSelection);
	m_Designer_Join->setEnabled(dok && !objectMode && hasModeSelection && !hasSealedPartsSelected);
	m_Designer_LadderPath->setEnabled(dok && !objectMode);
	m_Designer_LandingGearExtrusion->setEnabled(dok && !objectMode);
	m_Designer_LandingGearSphereLL->setEnabled(dok && !objectMode);
	m_Designer_LandingGearSphereUni->setEnabled(dok && !objectMode);
	m_Designer_LandingGearSpindle->setEnabled(dok && !objectMode);
	m_Designer_LandingGearTubing->setEnabled(dok && !objectMode);
	m_Designer_LandingSystem->setEnabled(dok && !objectMode);
	m_Designer_LaunchTube->setEnabled(dok && !objectMode);
	m_Designer_LaunchTubeJig->setEnabled(dok && !objectMode);
	//m_Designer_BdLibraryDelete;
	//m_Designer_ScLibraryDelete;
	//m_Designer_BdLibraryOpen;
	//m_Designer_ScLibraryOpen;
	if(m_Designer_LibrarySave)
		m_Designer_LibrarySave->setEnabled(dok && copyOk);
	m_Designer_LightBulb->setEnabled(dok && !objectMode);
	m_Designer_LightBulbProps->setEnabled(dok && !objectMode && hasModeSelection);
	m_Designer_LightLensProps->setEnabled(dok && !objectMode && hasModeSelection);
	m_Designer_LivestockPost->setEnabled(dok && !objectMode);
	m_Designer_LoopCut->setEnabled(dok && objectMode);
	m_Designer_MakeDecal->setEnabled(dok && !objectMode && hasModeSelection && !hasSealedPartsSelected);
	m_Designer_MakeDoor->setEnabled(dok && !objectMode && hasModeSelection && !hasSealedPartsSelected);
	m_Designer_MakeHull->setEnabled(dok && !objectMode && hasModeSelection && !hasSealedPartsSelected);
	m_Designer_MakeLandingGear->setEnabled(dok && !objectMode && hasModeSelection && !hasSealedPartsSelected);
	m_Designer_MakeLightLens->setEnabled(dok && !objectMode && hasModeSelection && !hasSealedPartsSelected);
	m_Designer_MakeMesh->setEnabled(dok && !objectMode && hasModeSelection && !hasSealedPartsSelected);
	m_Designer_MakeNavLightLens->setEnabled(dok && !objectMode && hasModeSelection && !hasSealedPartsSelected);
	m_Designer_MakeRoom->setEnabled(dok && !objectMode && hasModeSelection && !hasSealedPartsSelected);
	m_Designer_MakeRotatingBeaconLens->setEnabled(dok && !objectMode && hasModeSelection && !hasSealedPartsSelected);
	m_Designer_MakeStrobeLightLens->setEnabled(dok && !objectMode && hasModeSelection && !hasSealedPartsSelected);
	m_Designer_MakeTurboLift->setEnabled(dok && !objectMode && hasModeSelection && !hasSealedPartsSelected);
	m_Designer_MakeTurret->setEnabled(dok && !objectMode && hasModeSelection && !hasSealedPartsSelected);
	m_Designer_MakeWindow->setEnabled(dok && !objectMode && hasModeSelection && !hasSealedPartsSelected);
	m_Designer_MoveDoorSwitches->setEnabled(dok && !objectMode && hasModeSelection);
	//m_Designer_NewBuilding->setEnabled(false);
#if defined(QT_NO_DEBUG)
	m_Designer_NewItem->setEnabled(false);	// Not available in release build.
#endif
	//m_Designer_NewSpacecraft;
	m_Designer_NewVehicle->setEnabled(false);
	m_Designer_ObjectExtrusion->setEnabled(dok && objectMode);
	m_Designer_ObjectFace->setEnabled(dok && objectMode);
	m_Designer_ObjectIsolate->setEnabled(dok && DesignerMenuObjectModeEnabled() && objectMode);
	m_Designer_ObjectMode->setEnabled(dok && DesignerMenuObjectModeEnabled());
	m_Designer_ObjectQuad->setEnabled(dok && objectMode);
	m_Designer_ObjectSphereLL->setEnabled(dok && objectMode);
	m_Designer_ObjectSphereUni->setEnabled(dok && objectMode);
	m_Designer_ObjectSpindle->setEnabled(dok && objectMode);
	m_Designer_ObjectTriangle->setEnabled(dok && objectMode);
	m_Designer_ObjectTubing->setEnabled(dok && objectMode);
	m_Designer_Opacity->setEnabled(dok && objectMode && faceSelected);
	//m_Designer_Open;
	m_Designer_Paint->setEnabled(dok && objectMode && faceSelected);
	m_Designer_PanelCapacitor->setEnabled(dok && !objectMode);
	m_Designer_PanelFTLDrive->setEnabled(dok && !objectMode);
	m_Designer_PanelLifeSupport->setEnabled(dok && !objectMode);
	m_Designer_PanelManeuverDrive->setEnabled(dok && !objectMode);
	m_Designer_PanelPowerPlant->setEnabled(dok && !objectMode);
	m_Designer_PanelSensor->setEnabled(dok && !objectMode);
	m_Designer_PanelShield->setEnabled(dok && !objectMode);
	m_Designer_PanelWeapon->setEnabled(dok && !objectMode);
	m_Designer_Paste->setEnabled(dok);
	m_Designer_ParkGroundVehicle->setEnabled(dok && !objectMode);
	m_Designer_ParkRocket->setEnabled(dok && !objectMode);
	m_Designer_ParkSpacecraft->setEnabled(dok && !objectMode);
	m_Designer_ParkSpaceVehicle->setEnabled(dok && !objectMode);
	m_Designer_ParkWaterVehicle->setEnabled(dok && !objectMode);
	m_Designer_PasteAt->setEnabled(dok);
	m_Designer_PasteRadial->setEnabled(dok);
	m_Designer_PodBay->setEnabled(dok && !objectMode);
	m_Designer_Polish->setEnabled(dok && objectMode && faceSelected);
	m_Designer_PreviewBd->setEnabled(bdpreviewok);
	m_Designer_PreviewPeople->setEnabled(preview);
	m_Designer_PreviewSettings->setEnabled(preview);
	m_Designer_PreviewToggle->setEnabled(true);
	m_Designer_PunchFace->setEnabled(dok && objectMode);
	m_Designer_PushPull->setEnabled(dok && objectMode && faceSelected);
	m_Designer_Redo->setEnabled(dok);
	m_Designer_RefreshObstructions->setEnabled(dok);
	m_Designer_RemoteCamera->setEnabled(dok && !objectMode);
	m_Designer_ReverseFaces->setEnabled(dok && objectMode && faceSelected);
	m_Designer_RoomExtrusion->setEnabled(dok && !objectMode);
	m_Designer_RoomSphereLL->setEnabled(dok && !objectMode);
	m_Designer_RoomSphereUni->setEnabled(dok && !objectMode);
	m_Designer_RoomSpindle->setEnabled(dok && !objectMode);
	m_Designer_RoomTubing->setEnabled(dok && !objectMode);
	m_Designer_Rotate->setEnabled(dok && hasModeSelection);
	m_Designer_Save->setEnabled(dok && copyOk);
	m_Designer_SaveAs->setEnabled(dok && copyOk);
	m_Designer_SelectAll->setEnabled(dok);
	m_Designer_SelectContiguous->setEnabled(dok && objectMode && faceSelected);
	m_Designer_SelectEdges->setEnabled(dok && objectMode);
	m_Designer_SelectFaces->setEnabled(dok && objectMode);
	m_Designer_SelectVertices->setEnabled(dok && objectMode);
	m_Designer_SelfIllumination->setEnabled(dok && objectMode && faceSelected);
	//m_Designer_Settings
	m_Designer_SetRoomProperties->setEnabled(dok && !objectMode && hasModeSelection);
	m_Designer_SetVoidProperties->setEnabled(dok && !objectMode && hasModeSelection);
	m_Designer_ShipName->setEnabled(dok && !objectMode);
	m_Designer_ShowBacks->setEnabled(dok);
	m_Designer_ShowEdges->setEnabled(dok);
	m_Designer_ShowFaces->setEnabled(dok);
	m_Designer_ShowGrid->setEnabled(dok);
	m_Designer_ShowGround->setEnabled(dok);
	m_Designer_ShowSkyBox->setEnabled(dok);
	m_Designer_Shrub->setEnabled(dok && !objectMode);
	m_Designer_Site->setEnabled(dok && !objectMode);
	m_Designer_SiteMax->setEnabled(dok);
	m_Designer_SiteProperties->setEnabled(dok && !objectMode && hasModeSelection);
	m_Designer_Sketch->setEnabled(dok && !objectMode);
	m_Designer_Smooth->setEnabled(dok && objectMode && faceSelected && vertexSelected);
	m_Designer_SnapToEdges->setEnabled(dok);
	m_Designer_SnapToGrid->setEnabled(dok);
	m_Designer_SnapToVertices->setEnabled(dok);
	m_Designer_SolidsBlockPath->setEnabled(dok);
	m_Designer_SplitFaces->setEnabled(dok && objectMode && faceSelected);
	m_Designer_StationCapt->setEnabled(dok && !objectMode);
	m_Designer_StationDesigner->setEnabled(dok && !objectMode);
	m_Designer_StationEngineer->setEnabled(dok && !objectMode);
	m_Designer_StationFireControl->setEnabled(dok && !objectMode);
	m_Designer_StationGunner->setEnabled(dok && !objectMode && hasModeSelection);
	m_Designer_StationHelm->setEnabled(dok && !objectMode);
	m_Designer_StationMedic->setEnabled(dok && !objectMode);
	m_Designer_StationNavigator->setEnabled(dok && !objectMode);
	m_Designer_StationPowerRelay->setEnabled(dok && !objectMode);
	m_Designer_StationSensor->setEnabled(dok && !objectMode);
	m_Designer_StationShield->setEnabled(dok && !objectMode);
	m_Designer_StationTransporter->setEnabled(dok && !objectMode);
	m_Designer_StatusAlert->setEnabled(dok && !objectMode);
	m_Designer_StatusLifeSupport->setEnabled(dok && !objectMode);
	m_Designer_StatusMission->setEnabled(dok && !objectMode);
	m_Designer_StatusWorldMap->setEnabled(dok && !objectMode);
	m_Designer_Stretch->setEnabled(dok && hasModeSelection);
	m_Designer_TransporterPadLocs->setEnabled(dok && !objectMode && hasModeSelection);
	m_Designer_Tree->setEnabled(dok && !objectMode);
	m_Designer_TroopPost->setEnabled(dok && !objectMode);
	m_Designer_TurretBase->setEnabled(dok && !objectMode && hasModeSelection);
	m_Designer_TurretGun->setEnabled(dok && !objectMode && hasModeSelection);
	m_Designer_Undo->setEnabled(dok);
	//m_Designer_Videos;
	m_Designer_ViewPositionGrid->setEnabled(dok);
	m_Designer_ViewPositionLoad1->setEnabled(dok && viewIds.contains(1));
	m_Designer_ViewPositionLoad2->setEnabled(dok && viewIds.contains(2));
	m_Designer_ViewPositionLoad3->setEnabled(dok && viewIds.contains(3));
	m_Designer_ViewPositionLoad4->setEnabled(dok && viewIds.contains(4));
	m_Designer_ViewPositionLoad5->setEnabled(dok && viewIds.contains(5));
	m_Designer_ViewPositionLoad6->setEnabled(dok && viewIds.contains(6));
	m_Designer_ViewPositionLoad7->setEnabled(dok && viewIds.contains(7));
	m_Designer_ViewPositionLoad8->setEnabled(dok && viewIds.contains(8));
	m_Designer_ViewPositionLoad9->setEnabled(dok && viewIds.contains(9));
	m_Designer_ViewPositionOrigin->setEnabled(dok);
	m_Designer_ViewPositionSave1->setEnabled(dok);
	m_Designer_ViewPositionSave2->setEnabled(dok);
	m_Designer_ViewPositionSave3->setEnabled(dok);
	m_Designer_ViewPositionSave4->setEnabled(dok);
	m_Designer_ViewPositionSave5->setEnabled(dok);
	m_Designer_ViewPositionSave6->setEnabled(dok);
	m_Designer_ViewPositionSave7->setEnabled(dok);
	m_Designer_ViewPositionSave8->setEnabled(dok);
	m_Designer_ViewPositionSave9->setEnabled(dok);
	m_Designer_VoidEnvironmentExtrusion->setEnabled(dok && !objectMode);
	m_Designer_VoidEnvironmentSpindle->setEnabled(dok && !objectMode);
	m_Designer_VoidHullExtrusion->setEnabled(dok && !objectMode);
	m_Designer_VoidHullSpindle->setEnabled(dok && !objectMode);
	m_Designer_VoidRoomExtrusion->setEnabled(dok && !objectMode);
	m_Designer_VoidRoomSpindle->setEnabled(dok && !objectMode);
	m_Designer_WalkPath->setEnabled(dok && !objectMode);
	m_Designer_WeaponEmitLoc->setEnabled(dok && !objectMode && hasModeSelection);
	//m_Designer_Wiki;
	m_Designer_WindowJig->setEnabled(dok && !objectMode);

	// Hide/show/disable actions that depend on design mode.
	// WARNING: You can hide menu items but you cannot hide menus. The menu system goes haywire.
	// For consistency, hide menu items according to design mode. Don't hide according to sub mode conditions; disable instead.
	// Menu items have already been enabled|disabled. Don't enable controls here based on design mode; only disable them.
	switch(designMode)
	{
	case DaDesignBase::InvalidMode:
	case DaDesignBase::DESIGN_Unknown:
		break;
	case DaDesignBase::DESIGN_Spacecraft:
		m_Designer_DetailsMenu->setEnabled(true);
		m_Designer_EditMenu->setEnabled(true);
		m_Designer_FaceMenu->setEnabled(true);
		m_Designer_HullMenu->setEnabled(true);
		m_Designer_LadderPath->setEnabled(true);
		m_Designer_LandingGearMenu->setEnabled(true);
		m_Designer_LandscapeMenu->setEnabled(false);
		m_Designer_MakeLandingGear->setVisible(true);
		m_Designer_PanelsMenu->setEnabled(true);
		m_Designer_PartMenu->setEnabled(true);
		m_Designer_PartPropsMenu->setEnabled(true);
		m_Designer_PathsMenu->setEnabled(true);
		m_Designer_RoomMenu->setEnabled(true);
		m_Designer_StationsMenu->setEnabled(true);
		m_Designer_VehiclesMenu->setEnabled(true);
		m_Designer_VoidEnvMenu->setEnabled(false);
		m_Designer_VoidHullMenu->setEnabled(true);
		m_Designer_VoidRoomMenu->setEnabled(true);
		m_Designer_WalkPath->setEnabled(true);
		break;
	case DaDesignBase::DESIGN_Building:
		m_Designer_BerthName->setEnabled(false);
		m_Designer_CaptName->setEnabled(false);
		m_Designer_ConsoleEngineer->setEnabled(false);
		m_Designer_ConsoleHelm->setEnabled(false);
		m_Designer_ConsolePowerRelay->setEnabled(false);
		m_Designer_ContrailEmitter->setEnabled(false);
		m_Designer_DetailsMenu->setEnabled(bdStructure != BD_Fence);
		m_Designer_EditMenu->setEnabled(true);
		m_Designer_FaceMenu->setEnabled(true);
		m_Designer_HullMenu->setEnabled(bdStructure != BD_Fence);
		m_Designer_LandingGearMenu->setEnabled(false);
		m_Designer_LandscapeMenu->setEnabled(bdStructure != BD_Fence);
		m_Designer_MakeLandingGear->setVisible(false);
		m_Designer_PanelCapacitor->setEnabled(false);
		m_Designer_PanelFTLDrive->setEnabled(false);
		m_Designer_PanelLifeSupport->setEnabled(false);
		m_Designer_PanelManeuverDrive->setEnabled(false);
		m_Designer_PanelPowerPlant->setEnabled(false);
		m_Designer_PanelSensor->setEnabled(false);
		m_Designer_PanelShield->setEnabled(false);
		m_Designer_PanelWeapon->setEnabled(false);
		m_Designer_PanelsMenu->setEnabled(bdStructure != BD_StreetLight && bdStructure != BD_Fence);
		m_Designer_PartMenu->setEnabled(true);
		m_Designer_PartPropsMenu->setEnabled(bdStructure != BD_Fence);
		m_Designer_PathsMenu->setEnabled(bdStructure != BD_StreetLight);
		m_Designer_RoomMenu->setEnabled(bdStructure != BD_StreetLight && bdStructure != BD_Fence);
		m_Designer_ShipName->setEnabled(false);
		m_Designer_StationDesigner->setEnabled(false);
		m_Designer_StationEngineer->setEnabled(false);
		m_Designer_StationHelm->setEnabled(false);
		m_Designer_StationPowerRelay->setEnabled(false);
		m_Designer_StationsMenu->setEnabled(bdStructure != BD_StreetLight && bdStructure != BD_Fence);
		m_Designer_StatusAlert->setEnabled(false);
		m_Designer_StatusMission->setEnabled(false);
		m_Designer_VehiclesMenu->setEnabled(bdStructure != BD_StreetLight && bdStructure != BD_Fence);
		m_Designer_VoidEnvMenu->setEnabled(bdStructure != BD_Fence);
		m_Designer_VoidHullMenu->setEnabled(bdStructure != BD_Fence);
		m_Designer_VoidRoomMenu->setEnabled(bdStructure != BD_Fence);

		// Disable items that may have been enabled above.
		if(bdStructure != BD_StreetLight)
		{
			m_Designer_LightBulb->setEnabled(false);
			m_Designer_MakeLightLens->setEnabled(false);
		}

		if(bdStructure == BD_Fence)
		{
			m_Designer_AlignLiftStop->setEnabled(false);
			m_Designer_Associate->setEnabled(false);
			m_Designer_DecalJig->setEnabled(false);
			m_Designer_Dissociate->setEnabled(false);
			m_Designer_DoorJig->setEnabled(false);
			m_Designer_LadderPath->setEnabled(false);
			m_Designer_LaunchTubeJig->setEnabled(false);
			m_Designer_MakeDecal->setEnabled(false);
			m_Designer_MakeDoor->setEnabled(false);
			m_Designer_MakeHull->setEnabled(false);
			m_Designer_MakeLandingGear->setEnabled(false);
			m_Designer_MakeRoom->setEnabled(false);
			m_Designer_MakeTurboLift->setEnabled(false);
			m_Designer_MakeTurret->setEnabled(false);
			m_Designer_MakeWindow->setEnabled(false);
			m_Designer_MakeLightLens->setEnabled(false);
			m_Designer_MakeNavLightLens->setEnabled(false);
			m_Designer_MakeRotatingBeaconLens->setEnabled(false);
			m_Designer_MakeStrobeLightLens->setEnabled(false);
			m_Designer_WalkPath->setEnabled(false);
			m_Designer_WindowJig->setEnabled(false);
		}

		if(bdStructure != BD_SeaPlatform &&
				bdStructure != BD_AerialPlatform &&
				bdStructure != BD_GroundPlatform)
			m_Designer_BuildZone->setEnabled(false);

		if(bdStructure != BD_Biodome &&
				bdStructure != BD_SeaPlatform &&
				bdStructure != BD_AerialPlatform &&
				bdStructure != BD_GroundPlatform)
			m_Designer_VoidEnvMenu->setEnabled(false);	// Only biodomes and platforms can have environment voids.
		break;
	case DaDesignBase::DESIGN_Item:
		m_Designer_DetailsMenu->setEnabled(false);
		m_Designer_EditMenu->setEnabled(true);
		m_Designer_FaceMenu->setEnabled(true);
		m_Designer_HullMenu->setEnabled(false);
		m_Designer_LandingGearMenu->setEnabled(false);
		m_Designer_LandscapeMenu->setEnabled(false);
		m_Designer_PanelsMenu->setEnabled(false);
		m_Designer_RoomMenu->setEnabled(false);
		m_Designer_StationsMenu->setEnabled(false);
		m_Designer_VehiclesMenu->setEnabled(false);
		m_Designer_PartMenu->setEnabled(true);
		m_Designer_PartPropsMenu->setEnabled(true);
		m_Designer_PathsMenu->setEnabled(false);

		m_Designer_GravityToggle->setEnabled(false);
		m_Designer_PreviewPeople->setEnabled(false);

		m_Designer_DecalJig->setEnabled(false);
		m_Designer_DoorJig->setEnabled(false);
		m_Designer_LaunchTubeJig->setEnabled(false);
		m_Designer_WindowJig->setEnabled(false);

		m_Designer_MakeDecal->setEnabled(false);
		m_Designer_MakeDoor->setEnabled(false);
		m_Designer_MakeHull->setEnabled(false);
		m_Designer_MakeLandingGear->setEnabled(false);
		m_Designer_MakeMesh->setEnabled(true);
		m_Designer_MakeRoom->setEnabled(false);
		m_Designer_MakeTurboLift->setEnabled(false);
		m_Designer_MakeTurret->setEnabled(false);
		m_Designer_MakeWindow->setEnabled(false);
		m_Designer_MakeLightLens->setEnabled(false);
		m_Designer_MakeNavLightLens->setEnabled(false);
		m_Designer_MakeRotatingBeaconLens->setEnabled(false);
		m_Designer_MakeStrobeLightLens->setEnabled(false);
		break;
	//case DaDesignBase::DESIGN_Vehicle:
		//break;
	}
}

void AuDesignerFrame::DesignerMenuSetFindEdges()
{
	DesignerSetFindEdges(m_Designer_FindEdges->isChecked());
}

void AuDesignerFrame::DesignerMenuSetFindFaces()
{
	DesignerSetFindFaces(m_Designer_FindFaces->isChecked());
}

void AuDesignerFrame::DesignerMenuSetFindVertices()
{
	DesignerSetFindVertices(m_Designer_FindVertices->isChecked());
}

void AuDesignerFrame::DesignerMenuSetGridFollowsWork()
{
	DesignerSetGridFollowsWork(m_Designer_GridFollowsWork->isChecked());
}

void AuDesignerFrame::DesignerMenuSetShowBacks()
{
	DesignerSetShowBacks(m_Designer_ShowBacks->isChecked());
}

void AuDesignerFrame::DesignerMenuSetShowEdges()
{
	DesignerSetShowEdges(m_Designer_ShowEdges->isChecked());
}

void AuDesignerFrame::DesignerMenuSetShowFaces()
{
	DesignerSetShowFaces(m_Designer_ShowFaces->isChecked());
}

void AuDesignerFrame::DesignerMenuSetShowGround()
{
	DesignerSetShowGround(m_Designer_ShowGround->isChecked());
}

void AuDesignerFrame::DesignerMenuSetShowSkyBox()
{
	DesignerSetShowSkyBox(m_Designer_ShowSkyBox->isChecked());
}

void AuDesignerFrame::DesignerMenuSetSnapToEdges()
{
	DesignerSetSnapToEdges(m_Designer_SnapToEdges->isChecked());
}

void AuDesignerFrame::DesignerMenuObjectIsolate()
{
	DesignerSetObjectIsolate(m_Designer_ObjectIsolate->isChecked());
}

void AuDesignerFrame::DesignerSetObjectIsolate(bool objectIsolate)
{
	m_View->SetDesignerObjectIsolate(objectIsolate);

	if(m_Designer_ObjectIsolate->isChecked() != objectIsolate)
	{
		m_Designer_ObjectIsolate->blockSignals(true);
		m_Designer_ObjectIsolate->setChecked(objectIsolate);
		m_Designer_ObjectIsolate->blockSignals(false);
	}

	if(m_DesignerBar2)
		m_DesignerBar2->SetObjectIsolate(objectIsolate);

	DesignerEnableControls();
}

void AuDesignerFrame::DesignerMenuObjectMode()
{
	DesignerSetObjectMode(m_Designer_ObjectMode->isChecked());
}

void AuDesignerFrame::DesignerSetObjectMode(bool objectMode)
{
	m_View->SetDesignerObjectMode(objectMode);

	if(m_Designer_ObjectMode->isChecked() != objectMode)
	{
		m_Designer_ObjectMode->blockSignals(true);
		m_Designer_ObjectMode->setChecked(objectMode);
		m_Designer_ObjectMode->blockSignals(false);
	}

	if(m_DesignerBar2)
		m_DesignerBar2->SetObjectMode(objectMode);

	DesignerEnableControls();
}

void AuDesignerFrame::DesignerMenuSetSnapToGrid()
{
	DesignerSetSnapToGrid(m_Designer_SnapToGrid->isChecked());
}

void AuDesignerFrame::DesignerMenuSetSnapToVertices()
{
	DesignerSetSnapToVertices(m_Designer_SnapToVertices->isChecked());
}

void AuDesignerFrame::DesignerSetGridFollowsWork(bool follow)
{
	g_AuClient.DesignerSetGridFollowsWork(follow);

	if(m_Designer_GridFollowsWork->isChecked() != follow)
	{
		m_Designer_GridFollowsWork->blockSignals(true);
		m_Designer_GridFollowsWork->setChecked(follow);
		m_Designer_GridFollowsWork->blockSignals(false);
	}

	if(m_DesignerBar2)
		m_DesignerBar2->SetGridFollowsWork(follow);
}

void AuDesignerFrame::DesignerSetShowBacks(bool show)
{
	g_AuClient.DesignerSetShowBacks(show);

	if(m_Designer_ShowBacks->isChecked() != show)
	{
		m_Designer_ShowBacks->blockSignals(true);
		m_Designer_ShowBacks->setChecked(show);
		m_Designer_ShowBacks->blockSignals(false);
	}

	if(m_DesignerBar2)
		m_DesignerBar2->SetShowBacks(show);
}

void AuDesignerFrame::DesignerSetShowEdges(bool show)
{
	g_AuClient.DesignerSetShowEdges(show);

	if(m_Designer_ShowEdges->isChecked() != show)
	{
		m_Designer_ShowEdges->blockSignals(true);
		m_Designer_ShowEdges->setChecked(show);
		m_Designer_ShowEdges->blockSignals(false);
	}

	if(m_DesignerBar2)
		m_DesignerBar2->SetShowEdges(show);
}

void AuDesignerFrame::DesignerSetShowFaces(bool show)
{
	g_AuClient.DesignerSetShowFaces(show);

	if(m_Designer_ShowFaces->isChecked() != show)
	{
		m_Designer_ShowFaces->blockSignals(true);
		m_Designer_ShowFaces->setChecked(show);
		m_Designer_ShowFaces->blockSignals(false);
	}

	if(m_DesignerBar2)
		m_DesignerBar2->SetShowFaces(show);
}

void AuDesignerFrame::DesignerMenuSetGravityOn()
{
	DesignerSetGravityOn(m_Designer_GravityToggle->isChecked());
}

void AuDesignerFrame::DesignerSetGravityOn(bool grav)
{
	g_Au.DesignerSetGravityOn(grav);

	if(m_Designer_GravityToggle->isChecked() != grav)
	{
		m_Designer_GravityToggle->blockSignals(true);
		m_Designer_GravityToggle->setChecked(grav);
		m_Designer_GravityToggle->blockSignals(false);
	}

	if(m_DesignerBar)
		m_DesignerBar->SetGravity(grav);

	// This might only be needed for the old style designer.
	auApp->DesignerSetGravity(grav);
}

void AuDesignerFrame::DesignerMenuSetPreview()
{
	DesignerSetPreview(m_Designer_PreviewToggle->isChecked());
}

void AuDesignerFrame::DesignerSetPreview(bool preview)
{
	quint32 genSeed = 0;
	quint8 previewType = 0;
	if(preview)
	{
		//qDebug() << "AuDesignerFrame::DesignerSetPreview";
		// Show preview options.
		DaDesignBase::DESIGNMODE designMode = (DaDesignBase::DESIGNMODE)m_View->DesignMode();
		switch(designMode)
		{
		case DaDesignBase::DESIGN_Building:
			{
				AuBdPreviewInit dlg(this);
				if(ExecDialog(dlg) == QDialog::Accepted)
				{
					genSeed = dlg.GenSeed();
					previewType = dlg.PreviewType();
				}
				else
					return;
			}
			break;
		case DaDesignBase::DESIGN_Spacecraft:
			{
				AuScPreviewInit dlg(this);
				if(ExecDialog(dlg) == QDialog::Accepted)
				{
					genSeed = dlg.GenSeed();
					previewType = dlg.PreviewType();
				}
				else
					return;
			}
			break;
		case DaDesignBase::DESIGN_Item:
		case DaDesignBase::DESIGN_Unknown:
		case DaDesignBase::InvalidMode:
			break;
		}
	}

	if(m_Designer_PreviewToggle->isChecked() != preview)
	{
		m_Designer_PreviewToggle->blockSignals(true);
		m_Designer_PreviewToggle->setChecked(preview);
		m_Designer_PreviewToggle->blockSignals(false);
	}

	if(m_DesignerBar)
		m_DesignerBar->SetPreview(preview);

	bool sizeOk = auApp->DesignerSetPreview(previewType, genSeed);
	if(!sizeOk)
	{
		QMessageBox mb(g_Au.ApplicationName(), tr("The blueprint data is too large. "
					"A preview of the design cannot be created. "),
				QMessageBox::Warning, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton,
				auApp->Frame());
		auApp->Frame()->ExecMessageBox(mb);
	}
}

void AuDesignerFrame::DesignerSetShowGround(bool show)
{
	g_AuClient.DesignerSetShowGround(show);

	if(m_Designer_ShowGround->isChecked() != show)
	{
		m_Designer_ShowGround->blockSignals(true);
		m_Designer_ShowGround->setChecked(show);
		m_Designer_ShowGround->blockSignals(false);
	}
}

void AuDesignerFrame::DesignerSetShowSkyBox(bool show)
{
	g_AuClient.DesignerSetShowSkyBox(show);

	if(m_Designer_ShowSkyBox->isChecked() != show)
	{
		m_Designer_ShowSkyBox->blockSignals(true);
		m_Designer_ShowSkyBox->setChecked(show);
		m_Designer_ShowSkyBox->blockSignals(false);
	}
}

void AuDesignerFrame::DesignerSetShowGrid(bool show)
{
	g_AuClient.DesignerSetShowGrid(show);

	if(m_Designer_ShowGrid->isChecked() != show)
	{
		m_Designer_ShowGrid->blockSignals(true);
		m_Designer_ShowGrid->setChecked(show);
		m_Designer_ShowGrid->blockSignals(false);
	}

	if(m_DesignerBar2)
		m_DesignerBar2->SetShowGrid(show);
}

void AuDesignerFrame::DesignerGridToggle()
{
	bool show = m_Designer_ShowGrid && m_Designer_ShowGrid->isChecked();
	DesignerSetShowGrid(show);
}

void AuDesignerFrame::DesignerSetSnapToEdges(bool snap)
{
	g_AuClient.DesignerSetSnapToEdges(snap);

	if(m_Designer_SnapToEdges->isChecked() != snap)
	{
		m_Designer_SnapToEdges->blockSignals(true);
		m_Designer_SnapToEdges->setChecked(snap);
		m_Designer_SnapToEdges->blockSignals(false);
	}

	if(m_DesignerBar2)
		m_DesignerBar2->SetSnapToEdges(snap);
}

void AuDesignerFrame::DesignerSetSnapToGrid(bool snap)
{
	g_AuClient.DesignerSetSnapToGrid(snap);

	if(m_Designer_SnapToGrid->isChecked() != snap)
	{
		m_Designer_SnapToGrid->blockSignals(true);
		m_Designer_SnapToGrid->setChecked(snap);
		m_Designer_SnapToGrid->blockSignals(false);
	}

	if(m_DesignerBar2)
		m_DesignerBar2->SetSnapToGrid(snap);
}

void AuDesignerFrame::DesignerSetSnapToVertices(bool snap)
{
	g_AuClient.DesignerSetSnapToVertices(snap);

	if(m_Designer_SnapToVertices->isChecked() != snap)
	{
		m_Designer_SnapToVertices->blockSignals(true);
		m_Designer_SnapToVertices->setChecked(snap);
		m_Designer_SnapToVertices->blockSignals(false);
	}

	if(m_DesignerBar2)
		m_DesignerBar2->SetSnapToVertices(snap);
}

void AuDesignerFrame::DesignerBlipCoordinate(AuPoint coord)
{
	if(!m_DesignerBar2)
		return;
	m_DesignerBar2->SetBlipCoordinate(coord);
}

bool AuDesignerFrame::DesignerMenuObjectModeEnabled()
{
	return m_View->DesignerObjectModeOk();
}

void AuDesignerFrame::DesignerSetFindEdges(bool find)
{
	g_AuClient.DesignerSetFindEdges(find);

	if(m_Designer_FindEdges->isChecked() != find)
	{
		m_Designer_FindEdges->blockSignals(true);
		m_Designer_FindEdges->setChecked(find);
		m_Designer_FindEdges->blockSignals(false);
	}

	if(m_DesignerBar2)
		m_DesignerBar2->SetFindEdges(find);
}

void AuDesignerFrame::DesignerFindEdgesToggle()
{
	bool find = m_Designer_FindEdges && m_Designer_FindEdges->isChecked();
	DesignerSetFindEdges(find);
}

void AuDesignerFrame::DesignerSetFindFaces(bool find)
{
	g_AuClient.DesignerSetFindFaces(find);

	if(m_Designer_FindFaces->isChecked() != find)
	{
		m_Designer_FindFaces->blockSignals(true);
		m_Designer_FindFaces->setChecked(find);
		m_Designer_FindFaces->blockSignals(false);
	}

	if(m_DesignerBar2)
		m_DesignerBar2->SetFindFaces(find);
}

void AuDesignerFrame::DesignerFindFacesToggle()
{
	bool find = m_Designer_FindFaces && m_Designer_FindFaces->isChecked();
	DesignerSetFindFaces(find);
}

void AuDesignerFrame::DesignerSetFindVertices(bool find)
{
	g_AuClient.DesignerSetFindVertices(find);

	if(m_Designer_FindVertices->isChecked() != find)
	{
		m_Designer_FindVertices->blockSignals(true);
		m_Designer_FindVertices->setChecked(find);
		m_Designer_FindVertices->blockSignals(false);
	}

	if(m_DesignerBar2)
		m_DesignerBar2->SetFindVertices(find);
}

void AuDesignerFrame::DesignerFindVerticesToggle()
{
	bool find = m_Designer_FindVertices && m_Designer_FindVertices->isChecked();
	DesignerSetFindVertices(find);
}

void AuDesignerFrame::DesignerMenuSetSolidsBlockPath()
{
	DesignerSetSolidsBlockPath(m_Designer_SolidsBlockPath->isChecked());
}

void AuDesignerFrame::DesignerSetSolidsBlockPath(bool block)
{
	g_Au.DesignerSetBlockPaths(block);

	if(m_Designer_SolidsBlockPath->isChecked() != block)
	{
		m_Designer_SolidsBlockPath->blockSignals(true);
		m_Designer_SolidsBlockPath->setChecked(block);
		m_Designer_SolidsBlockPath->blockSignals(false);
	}

	if(m_DesignerBar)
		m_DesignerBar->SetSolidsBlockPath(block);
}

void AuDesignerFrame::BrokerShipment(AuShipment shipment)
{
	if(m_BuildingDock)
		m_BuildingDock->m_Labor->BrokerShipment(shipment);
	if(m_SpacecraftDock)
		m_SpacecraftDock->m_Content->BrokerShipment(shipment);
	if(m_SpacecraftShipmentsDock)
		m_SpacecraftShipmentsDock->m_Content->Shipment(shipment);
}

void AuDesignerFrame::BrokerShipmentsClear()
{
	if(m_BuildingDock)
		m_BuildingDock->m_Labor->BrokerShipmentsClear();
	if(m_SpacecraftDock)
		m_SpacecraftDock->m_Content->BrokerShipmentsClear();
	if(m_SpacecraftShipmentsDock)
		m_SpacecraftShipmentsDock->m_Content->ShipmentsClear();
}

void AuDesignerFrame::BrokerTradeAgreement(AuTradeAgreement ta)
{
	if(m_BuildingDock)
		m_BuildingDock->m_Labor->BrokerTradeAgreement(ta);
}

void AuDesignerFrame::BrokerTradeAgreementsClear()
{
	if(m_BuildingDock)
		m_BuildingDock->m_Labor->BrokerTradeAgreementsClear();
}

void AuDesignerFrame::OperateTurboLift(const AuId& shipId, const AuId& personId, quint16 liftId)
{
	m_View->OperateTurboLift(shipId, personId, liftId);
}

bool AuDesignerFrame::IsAFK() const
{
	if(!m_PlayersOnlineDock)
		return false;
	return m_PlayersOnlineDock->m_Content->IsAFK();
}

void AuDesignerFrame::DisableAFK()
{
	if(!m_PlayersOnlineDock)
		return;
	if(m_PlayersOnlineDock->m_Content->IsAFK())
		m_PlayersOnlineDock->m_Content->AFKClicked();
}

void AuDesignerFrame::BuildingCodes()
{
	AuIndustryDlg* dlg = new AuIndustryDlg(this);
	dlg->show();
}

void AuDesignerFrame::ReportSystemSpacecraft(quint32 empireId, AuId sectorId, AuId systemId, QString systemName)
{
	// Display this thing modelessly.
	AuSystemSpacecraft* dlg = new AuSystemSpacecraft(empireId, sectorId, systemId, systemName, this);
	dlg->show();
}

void AuDesignerFrame::AvatarsUpdated()
{
	if(m_MailDock)
		m_MailDock->AvatarsUpdated();
}

void AuDesignerFrame::SpawnRecall()
{
	m_View->SpawnRecall();
}

void AuDesignerFrame::SpawnTie()
{
	m_View->SpawnTie();
}

void AuDesignerFrame::LaunchDesigner()
{
	// We can't show the designer until the basic game data has been loaded. It is stored in a local cache file whenever
	// the player logs into the server farm.
	DgData& gd = g_Au.LockDataForWrite();
	int result = gd.LoadBasicGameData();
	g_Au.UnlockData();
	switch(result)
	{
	case 0:
		break;	// Good result.
	case 1:
		{
			QString msg = tr("The file <b>%1/Programs/GD</b> is missing. "
					"Run the <i>Shores of Hazeron Launcher</i> to restore the file. "
					"Then try again.").arg(g_Au.ApplicationName());
			QMessageBox::warning(this, g_Au.ApplicationName(), msg);
		}
		return;
	case 2:
		{
			QString msg = tr("The file <b>%1/Programs/GD</b> exists but it could not be opened. "
					"Run the <i>Shores of Hazeron Launcher</i> to repair the file. "
					"This is probably a file permission problem so you may need to manually delete the file. "
					"Then try again.").arg(g_Au.ApplicationName());
			QMessageBox::warning(this, g_Au.ApplicationName(), msg);
		}
		return;
	case 3:
		{
			QString msg = tr("The file <b>%1/Programs/GD</b> is broken or obsolete. "
					"Run the <i>Shores of Hazeron Launcher</i> to repair the file. "
					"Then try again.").arg(g_Au.ApplicationName());
			QMessageBox::warning(this, g_Au.ApplicationName(), msg);
		}
		return;
	}

	AuSoloDesignerDlg dlg(this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	auApp->SetDefaultHuman(dlg.Name(), dlg.Handedness(), dlg.Gender(), dlg.DNA());
	auApp->SetRunState(AuApp::Designer);
}

void AuDesignerFrame::TransferCitizensAvailable(QByteArray ba)
{
	if(m_CargoTransferDock)
		m_CargoTransferDock->TransferCitizensAvailable(ba);
	if(m_HoldDock)
		m_HoldDock->TransferCitizensAvailable(ba);
	if(m_HoldDockFree)
		m_HoldDockFree->TransferCitizensAvailable(ba);
}

