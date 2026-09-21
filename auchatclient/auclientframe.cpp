//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auclientframe.cpp,v 1.61 2022/04/15 20:35:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auclientframe.h"
//
#include "auabout.h"
#include "auaccesshold.h"
#include "auagentdialog.h"
#include "auapp.h"
#include "aubdexchange.h"
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
#include "auclientview.h"
#include "aucomm.h"
#include "audamagereport.h"
#include "audocktitle.h"
#include "auempirecitizens.h"
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
#include "auintro.h"
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
#include "auscexchange.h"
#include "ausettingsdialog.h"
#include "auspacecraftaccount.h"
#include "auspacecraftberths.h"
#include "auspacecraftberthsdialog.h"
#include "auspacecraftbounties.h"
#include "auspacecraftpages.h"
#include "auspacecraftpolicy.h"
#include "auspacecraftshipments.h"
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
	DiControlInput(QObject::tr("Exit Hazeron Commander"),
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
	DiControlInput(QObject::tr("About Hazeron Commander"),
			QObject::tr("Displays a window with information about <i>Hazeron Commander</i>."),
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
			QObject::tr("The building window interacts with buildings in modern cities."),
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
	DiControlInput(QObject::tr("Unit Orders Window Toggle"),
			AuFrame::CI_ToggleOrdersUI, Qt::Key_F3, Qt::ShiftModifier,
			DiControlInput::MT_None, Qt::NoModifier,
			DiControlInput::JT_None, 0, 100, false),
};
int s_NumFrameDefaults = sizeof(s_FrameDefaults)/sizeof(DiControlInput);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuClientFrame::AuClientFrame(QWidget *parent) : AuFrame(parent)
{
	setBackgroundRole(QPalette::Background);
	setWindowIcon(QIcon(":data/SoHLogo32.png"));
	setWindowTitle(g_Au.ApplicationName());

	// App filters for function keys so I get them no matter who has focus.
	connect(auApp, SIGNAL(KeyPressed(QKeyEvent*)), SLOT(KeyPressed(QKeyEvent*)));
	connect(auApp, SIGNAL(KeyReleased(QKeyEvent*)), SLOT(KeyReleased(QKeyEvent*)));

	m_Net = 0;
	m_NetCache = 0;

	// Create intro screen.
	m_Intro = new AuIntro();
	connect(m_Intro, SIGNAL(LoginOk()), auApp, SLOT(LoginSucceeded()));
	connect(m_Intro, SIGNAL(AvatarLogin(AuId)), auApp, SLOT(StartSceneCommunications(AuId)));
	connect(m_Intro, SIGNAL(HelpAbout()), SLOT(HelpAbout()));
	connect(m_Intro, SIGNAL(ToggleSettingsUI()), SLOT(ToggleSettingsUi()));
	connect(m_Intro, SIGNAL(ToggleFullScreen()), SLOT(ToggleFullScreen()));
	connect(m_Intro, SIGNAL(ExitGame()), auApp, SLOT(closeAllWindows()));
	setCentralWidget(m_Intro);
	if(auApp->IsSingleUserMode())
		m_Intro->hide();

	m_ActiveKeyMods = Qt::NoModifier;
	m_Agent = 0;
	m_BioDock = 0;
	m_BuildingDock = 0;
	m_CargoHoldDock = 0;
	m_CargoManifestDock = 0;
	m_CargoOrdnanceDock = 0;
	m_CargoTransferDock = 0;
	m_CommDock = 0;
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
	m_LocatorCompassDock = 0;
	m_LocatorDock = 0;
	m_LocatorGPSDock = 0;
	m_LocatorStarMapDock = 0;
	m_LocatorSystemScanDock = 0;
	m_LocatorWorldMapDock = 0;
	m_MailDock = 0;
	m_OrdersDock = 0;
	m_PlayersOnlineDock = 0;

	m_pControls = new DiControlInputArray;
	EnumControlInputs(*m_pControls);							// Grab current controls right away.

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
	m_View = 0;
	m_WaitTime.start();

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

AuClientFrame::~AuClientFrame()
{
	//qDebug() << "AuClientFrame::~AuClientFrame m_Joystick";
	// Deleting the joystick here crashes in SDL.
	//if(m_Joystick)
		//delete m_Joystick;

	//qDebug() << "AuClientFrame::~AuClientFrame m_pControls";
	if(m_pControls)
		delete m_pControls;

	//qDebug() << "AuClientFrame::~AuClientFrame earlyArrivals";
	for(QList<AuCommPacket*>::iterator ci = m_EarlyArrivals.begin(); ci != m_EarlyArrivals.end(); ci++)
		delete *ci;
	m_EarlyArrivals.clear();
	//qDebug() << "AuClientFrame::~AuClientFrame done";
}

void AuClientFrame::StopThreads()
{
	if(m_MailDock)
		m_MailDock->StopThreads();
}

QSize AuClientFrame::sizeHint() const
{
	return QSize(1280, 720);
}

void AuClientFrame::Timer()
{
	switch(auApp->RunState())
	{
	case AuApp::NotLoggedIn:
		m_WaitTime.restart();
		return;
	case AuApp::LoggedIn:
		// App is logged in and the login account's player is valid.
		//qDebug() << "AuClientFrame::TimerUpdate LoggedIn, intro" << (bool)m_Intro;
		if(m_Intro)
		{
			// Get player unit id from application data.
			// The run state indicates it has been received from the server.
			const DgData& gd = g_Au.LockDataForRead();
			AuId idPerson = gd.ClientUnitId();
			g_Au.UnlockData();
			g_Au.Settings().writeEntry("/Account/LastPlayerId", idPerson.StringId());

			//qDebug("AuClientFrame::Timer creating AuClientView for player");
			CreateViewWidget();

			//qDebug("AuClientFrame::Timer - idPerson %d", idPerson.GetID());
			// Select the client unit as the scene atom. This may cause us to continue waiting until the scene atom arrives
			// from the server.
			m_View->SetSceneAtom(idPerson);
		}
		else if(!m_CommDock && !m_View->SceneSector().IsNull())
		{
			// Splash screen is gone. Button bar has not been created. View is no longer waiting for a scene atom.
			CreateDockWidgets();
		}
		else
		{
			// Check/queue Internet downloads wanted by the scene.
			PostNetRequests();

			// This is a decent interval to test the connection to the server to insure we haven't lost the link.
			auApp->TestClientConnection();
		}
		break;
	case AuApp::LoggedInInit:
		// App is logged in but the login account's player has not been initialized yet.
		// This must be the very first time this person has logged in.
		//qDebug() << "AuClientFrame::TimerUpdate LoggedInInit";
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
		//qDebug() << "AuClientFrame::TimerUpdate LoggedInWait";
		return;
	case AuApp::InitWait:
		// App is waiting for a reply to the init information transmitted.
		// Do nothing; keep waiting.
		//qDebug() << "AuClientFrame::TimerUpdate InitWait";
		return;
	case AuApp::Designer:
	case AuApp::Solitaire:
		break;
	}
}

AuGearPos AuClientFrame::GetGearSelectionPos() const
{
	if(!m_GearDock || !m_GearDock->isVisible())
		return AuGearPos();
	return m_GearDock->m_Gear->GetSelectionPos();
}

AuView* AuClientFrame::View() const
{
	return m_View;
}

AuClientView* AuClientFrame::ClientView() const
{
	return m_View;
}

void AuClientFrame::ShowForum()
{
	// Thanks Qt for making this one so easy.
	QUrl url("https://www.hazeron.com/mybb/");
	QDesktopServices::openUrl(url);
}

void AuClientFrame::ShowVideos()
{
	// Thanks Qt for making this one so easy.
	QUrl url("https://www.hazeron.com/videos.php");
	QDesktopServices::openUrl(url);
}

void AuClientFrame::ShowWiki()
{
	// Thanks Qt for making this one so easy.
	QUrl url("https://www.hazeron.com/wiki");
	QDesktopServices::openUrl(url);
}

void AuClientFrame::ShowAvatars()
{
	// Thanks Qt for making this one so easy.
	QUrl url("https://www.hazeron.com/EmpireStandings/Avatars.php");
	QDesktopServices::openUrl(url);
}

void AuClientFrame::ShowWarDimensionStandings()
{
	// Thanks Qt for making this one so easy.
	QUrl url("https://www.hazeron.com/EmpireStandings/EmpireStandings.php");
	QDesktopServices::openUrl(url);
}

void AuClientFrame::ShowPeaceDimensionStandings()
{
	// Thanks Qt for making this one so easy.
	QUrl url("https://www.hazeron.com/EmpireStandingsNZ/EmpireStandings.php");
	QDesktopServices::openUrl(url);
}

void AuClientFrame::ToggleBuildingUi()
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

void AuClientFrame::ToggleSpacecraftUi()
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

void AuClientFrame::ToggleSpacecraftAccountUi()
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

void AuClientFrame::ToggleSpacecraftBountiesUi()
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

void AuClientFrame::ToggleSpacecraftDamageUi()
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

void AuClientFrame::ToggleBdExchangeUi()
{
	if(m_BdExchangeDock)
	{
		if(m_BdExchangeDock->isHidden())
			m_BdExchangeDock->show();
		else
			m_BdExchangeDock->hide();
	}
}

void AuClientFrame::ToggleScExchangeUi()
{
	if(m_SpacecraftDesignExchangeDock)
	{
		if(m_SpacecraftDesignExchangeDock->isHidden())
			m_SpacecraftDesignExchangeDock->show();
		else
			m_SpacecraftDesignExchangeDock->hide();
	}
}

void AuClientFrame::ToggleSpacecraftMissionUi()
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

void AuClientFrame::ToggleSpacecraftPolicyUi()
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

void AuClientFrame::ToggleSpacecraftRosterUi()
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

void AuClientFrame::ToggleSpacecraftShipmentsUi()
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

void AuClientFrame::ToggleLocatorUi()
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

void AuClientFrame::ToggleLocatorCompassUi()
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

void AuClientFrame::ToggleLocatorGPSUi()
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

void AuClientFrame::ToggleLocatorStarMapUi()
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

void AuClientFrame::ToggleLocatorSystemScanUi()
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

void AuClientFrame::ToggleLocatorWorldMapUi()
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

void AuClientFrame::ToggleCommUi()
{
	if(m_CommDock)
	{
		if(m_CommDock->isHidden())
			m_CommDock->show();
		else
			m_CommDock->hide();
	}
}

void AuClientFrame::ToggleOrdersUi()
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

void AuClientFrame::ToggleGovernUi()
{
	if(m_GovernDock)
	{
		if(m_GovernDock->isHidden() && m_View->SceneAtomEmpire())
			m_GovernDock->show();
		else
			m_GovernDock->hide();
	}
}

void AuClientFrame::ToggleGovernAnnouncementsUi()
{
	if(m_GovernAnnouncementsDock)
	{
		if(m_GovernAnnouncementsDock->isHidden() && m_View->SceneAtomEmpire())
			m_GovernAnnouncementsDock->show();
		else
			m_GovernAnnouncementsDock->hide();
	}
}

void AuClientFrame::ToggleGovernCitiesUi()
{
	if(m_GovernCitiesDock)
	{
		if(m_GovernCitiesDock->isHidden() && m_View->SceneAtomEmpire())
			m_GovernCitiesDock->show();
		else
			m_GovernCitiesDock->hide();
	}
}

void AuClientFrame::ToggleGovernCitizensUi()
{
	if(m_GovernCitizensDock)
	{
		if(m_GovernCitizensDock->isHidden() && m_View->SceneAtomEmpire())
			m_GovernCitizensDock->show();
		else
			m_GovernCitizensDock->hide();
	}
}

void AuClientFrame::ToggleGovernCommerceUi()
{
	if(m_GovernCommerceDock)
	{
		if(m_GovernCommerceDock->isHidden() && m_View->SceneAtomEmpire())
			m_GovernCommerceDock->show();
		else
			m_GovernCommerceDock->hide();
	}
}

void AuClientFrame::ToggleGovernDiplomacyUi()
{
	if(m_GovernDiplomacyDock)
	{
		if(m_GovernDiplomacyDock->isHidden() && m_View->SceneAtomEmpire())
			m_GovernDiplomacyDock->show();
		else
			m_GovernDiplomacyDock->hide();
	}
}

void AuClientFrame::ToggleGovernPatentsUi()
{
	if(m_GovernPatentsDock)
	{
		if(m_GovernPatentsDock->isHidden() && m_View->SceneAtomEmpire())
			m_GovernPatentsDock->show();
		else
			m_GovernPatentsDock->hide();
	}
}

void AuClientFrame::ToggleGovernPolicyUi()
{
	if(m_GovernPolicyDock)
	{
		if(m_GovernPolicyDock->isHidden() && m_View->SceneAtomEmpire())
			m_GovernPolicyDock->show();
		else
			m_GovernPolicyDock->hide();
	}
}

void AuClientFrame::ToggleGovernTaxesUi()
{
	if(m_GovernTaxesDock)
	{
		if(m_GovernTaxesDock->isHidden() && m_View->SceneAtomEmpire())
			m_GovernTaxesDock->show();
		else
			m_GovernTaxesDock->hide();
	}
}

void AuClientFrame::ToggleSettingsUi()
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
			//m_Settings->SetOpenGLTextureUnits(m_View->OpenGLTextureUnits());
			connect(m_Settings, SIGNAL(ControlsChanged()), SLOT(ControlsChanged()));
			//connect(m_Settings, SIGNAL(ControlsChanged()), m_View, SLOT(ControlsChanged()));
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

void AuClientFrame::CloseSettingsUi()
{
	if(m_Settings && !m_Settings->isHidden())
		m_Settings->hide();
}

void AuClientFrame::ToggleCharacterUi()
{
	if(m_BioDock)
	{
		if(m_BioDock->isHidden())
			m_BioDock->show();
		else
			m_BioDock->hide();
	}
}

void AuClientFrame::ToggleHoldUi()
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

void AuClientFrame::ToggleCargoHoldUi()
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

void AuClientFrame::ToggleCargoManifestUi()
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

void AuClientFrame::ToggleCargoOrdnanceUi()
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

void AuClientFrame::ToggleCargoTransferUi()
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

void AuClientFrame::ToggleGearUi()
{
	if(m_GearDock)
	{
		if(m_GearDock->isHidden())
			m_GearDock->show();
		else
			m_GearDock->hide();
	}
}

void AuClientFrame::ToggleHandCraftUi()
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

void AuClientFrame::ToggleAllUis()
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

void AuClientFrame::ToggleAgentUi()
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

void AuClientFrame::TogglePlayersOnlineUi()
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

void AuClientFrame::ToggleMailUi()
{
	// Single user mode does not get to see/use the mail ui.
	if(auApp->IsSingleUserMode())
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
void AuClientFrame::ChannelOpened(const QString& channelName, AuCommStdChannel::SCOPE scope, const QString& narrDesc)
{
	// The new channel tab is not pushed to the top so creating a new channel
	// can't be used as an offensive tactic against someone. The tab just
	// appears quietly.
	if(m_CommDock)
		m_CommDock->m_Comm->CreateChannel(channelName, scope, false, narrDesc);
}

void AuClientFrame::ChatReceived(AuCommPacket& msg)
{
	if(m_CommDock)
		m_CommDock->m_Comm->ChatReceived(msg);
	else
	{
		//qDebug() << "AuClientFrame::ChatReceived early arrival" << msg.Text();
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
}

void AuClientFrame::ListOpenChannels(QStringList& channelNames) const
{
	if(m_CommDock)
		m_CommDock->m_Comm->ListOpenChannels(channelNames);
}

void AuClientFrame::closeEvent(QCloseEvent* event)
{
	// Qt example code does not depict them calling this base class implementation. I am guessing it just accepts the event.
	//qDebug() << "AuClientFrame::closeEvent";
	event->accept();

	StopThreads();

	g_Au.Settings().setValue("/Widget/MainFrame/Geometry", saveGeometry());
	if(m_CommDock)
		g_Au.Settings().setValue("/Widget/MainFrame/WindowState1", saveState());
	g_Au.Settings().setValue("/Widget/MainFrame/FullScreen", isFullScreen());

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

void AuClientFrame::JobAvailable(AuJobOffer job)
{
	// Locate the jobs dialog if it is up. This findChild call will find it, no matter which child widget has displayed it.
	AuJobsQueryDialog* jobs = findChild<AuJobsQueryDialog*>("Jobs");
	if(jobs)
		jobs->JobAvailable(job);
}

void AuClientFrame::ChoicePicked(quint32 storyInstance, qint8 value, AuId offeredBy)
{
	auApp->ChoicePicked(storyInstance, value, offeredBy);
}

void AuClientFrame::GovDossierAddNote(quint32 dosEmpire, QString docNote)
{
	auApp->GovDossierAddNote(dosEmpire, docNote);
}

void AuClientFrame::GovDossierDispatchCourier(quint32 toEmpire, QString message)
{
	auApp->GovDossierDispatchCourier(toEmpire, message);
}

void AuClientFrame::GovDossierDocState(quint32 dosEmpire, AuTime docTime, quint8 docType, quint16 docIndex, quint8 docState)
{
	auApp->GovDossierDocState(dosEmpire, docTime, docType, docIndex, docState);
}

void AuClientFrame::GovSetAnnouncements(QString announcements)
{
	auApp->GovSetAnnouncements(announcements);
}

void AuClientFrame::GovSetWarCriteria(AuWarCriteria crit)
{
	auApp->GovSetWarCriteria(crit);
}

void AuClientFrame::GovSetDefaultPoliticalStance(int stance)
{
	auApp->GovSetDefaultPoliticalStance(stance);
}

void AuClientFrame::GovSetTaxes(double income, double sales, double subsidy)
{
	auApp->GovSetTaxes(income, sales, subsidy);
}

void AuClientFrame::GovSetCommerce(QByteArray commerce)
{
	auApp->GovSetCommerce(commerce);
}

void AuClientFrame::ToggleFullScreen()
{
	//qDebug() << "AuClientFrame::ToggleFullScreen isFullScreen" << isFullScreen();
	if(isFullScreen())
		showNormal();
	else
		showFullScreen();
}

void AuClientFrame::EmpireDataBegin()
{
	//qDebug() << "AuClientFrame::EmpireDataBegin";
	m_muEmpireDataState.lock();
	m_EmpireDataState = 1;
	m_muEmpireDataState.unlock();
}

void AuClientFrame::EmpireDataComplete()
{
	//qDebug() << "AuClientFrame::EmpireDataComplete";
	m_muEmpireDataState.lock();
	m_EmpireDataState = 2;
	m_muEmpireDataState.unlock();
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
		m_GovernDiplomacyDock->m_Content->UpdateEmpireRecord();
	if(m_GovernPatentsDock)
		m_GovernPatentsDock->m_Content->UpdateEmpireRecord();
	if(m_GovernPolicyDock)
		m_GovernPolicyDock->m_Content->UpdateEmpireRecord();
	if(m_GovernTaxesDock)
		m_GovernTaxesDock->m_Content->UpdateEmpireRecord();
}

bool AuClientFrame::EmpireDataIncoming() const
{
	m_muEmpireDataState.lock();
	bool empireDataIncoming = (m_EmpireDataState == 1);
	m_muEmpireDataState.unlock();
	return empireDataIncoming;
}

bool AuClientFrame::GetAndResetEmpireDataReceived()
{
	m_muEmpireDataState.lock();
	bool empireDataReceived = (m_EmpireDataState == 2);
	if(empireDataReceived)
		m_EmpireDataState = 0;
	m_muEmpireDataState.unlock();
	return empireDataReceived;
}

void AuClientFrame::AcceptCrew(bool accept)
{
	auApp->AcceptCrew(accept);
}

void AuClientFrame::AcceptPassengers(bool accept)
{
	auApp->AcceptPassengers(accept);
}

void AuClientFrame::AcceptTroops(bool accept)
{
	auApp->AcceptTroops(accept);
}

void AuClientFrame::AutoFetchVehicles(bool autoFetch)
{
	auApp->AutoFetchVehicles(autoFetch);
}

void AuClientFrame::AllowPassengerSeaChests(bool allow)
{
	auApp->AllowPassengerSeaChests(allow);
}

void AuClientFrame::KickBerthOccupant(AuId occupant)
{
	auApp->KickBerthOccupant(occupant);
}

void AuClientFrame::KickStowaways()
{
	auApp->KickStowaways();
}

void AuClientFrame::ReconcileRoster()
{
	auApp->ReconcileRoster();
}

void AuClientFrame::ReleaseBerth()
{
	auApp->ReleaseBerth();
}

void AuClientFrame::SendShipFunds(double amount, AuId idShip)
{
	auApp->SendShipFunds(amount, idShip);
}

void AuClientFrame::ReceiveShipFunds(double amount, AuId idShip)
{
	auApp->ReceiveShipFunds(amount, idShip);
}

void AuClientFrame::SetPoliticalStance(quint32 towardEmpire, quint8 stance, quint8 vassalTributePercent)
{
	auApp->SetPoliticalStance(towardEmpire, stance, vassalTributePercent);
}

void AuClientFrame::SendChat(AuCommPacket& msg)
{
	if(m_CommDock)
		m_CommDock->m_Comm->SendChat(msg);
}

QString AuClientFrame::OrdersChannel() const
{
	if(m_OrdersDock)
		return m_OrdersDock->m_UnitOrders->OrderChannel();
	else
		return AuCommStdChannel::CrewChannelName();
}

void AuClientFrame::SetManufacturingProcessForConstruction(quint16 mpid)
{
	Q_UNUSED(mpid);
}

void AuClientFrame::SetWorkerEnabledForConstruction(bool enableWorker)
{
	Q_UNUSED(enableWorker);
}

void AuClientFrame::HelpRequestAccepted(AuId reqId, QString reqName)
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

void AuClientFrame::EnterStudio(int panelIndex, QString password)
{
	auApp->EnterStudio(panelIndex, password);
}

void AuClientFrame::ShowCitiesOnStarMap(AuIdList cityIds)
{
	if(m_LocatorStarMapDock)
		m_LocatorStarMapDock->m_Content->StarMapShowCities(cityIds);

	if(m_LocatorDock)
		m_LocatorDock->m_Locator->StarMapShowCities(cityIds);
}

void AuClientFrame::ShowGalaxyLocOnStarMap(QString galaxyName, AuPoint loc)
{
	if(m_LocatorStarMapDock)
		m_LocatorStarMapDock->m_Content->StarMapShowGalaxyLocation(galaxyName, loc);

	if(m_LocatorDock)
		m_LocatorDock->m_Locator->StarMapShowGalaxyLocation(galaxyName, loc);
}

void AuClientFrame::SurrenderCities(AuIdList cityIds, QStringList cityNames, quint32 toEmpire)
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

void AuClientFrame::HangarQueryAnswered(QString shipDesc, quint32 shipDesign, quint32 shipsInHangar)
{
	AuBldHangar* bdhangar = findChild<AuBldHangar*>("BldHangar");
	if(bdhangar)
		bdhangar->QueryAnswered(shipDesc, shipDesign, shipsInHangar);
}

void AuClientFrame::TransporterPlace(AuTransporterTarget tt)
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

void AuClientFrame::TransporterShip(AuId id, QString name, QString desc)
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

void AuClientFrame::TransporterTransporter(AuId id, QString name, QString desc, quint16 whichTransporter)
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

void AuClientFrame::StarMapUpdateAllComplete()
{
	if(m_LocatorDock)
		m_LocatorDock->m_Locator->StarMapUpdateAllComplete();
	if(m_LocatorStarMapDock)
		m_LocatorStarMapDock->m_Content->StarMapUpdateAllComplete();
}

void AuClientFrame::StarMapUpdateGalaxyComplete(int galaxy)
{
	if(m_LocatorDock)
		m_LocatorDock->m_Locator->StarMapUpdateGalaxyComplete((GALAXY)galaxy);
	if(m_LocatorStarMapDock)
		m_LocatorStarMapDock->m_Content->StarMapUpdateGalaxyComplete((GALAXY)galaxy);
}

void AuClientFrame::StarMapUpdateSectorComplete(int galaxy, AuIdList sectorIds)
{
	if(m_LocatorDock)
		m_LocatorDock->m_Locator->StarMapUpdateSectorComplete((GALAXY)galaxy, sectorIds);
	if(m_LocatorStarMapDock)
		m_LocatorStarMapDock->m_Content->StarMapUpdateSectorComplete((GALAXY)galaxy, sectorIds);
}

void AuClientFrame::StarMapUpdateAllStarted()
{
	if(m_LocatorDock)
		m_LocatorDock->m_Locator->StarMapUpdateAllStarted();
	if(m_LocatorStarMapDock)
		m_LocatorStarMapDock->m_Content->StarMapUpdateAllStarted();
}

void AuClientFrame::StarMapUpdateGalaxyStarted(int galaxy)
{
	if(m_LocatorDock)
		m_LocatorDock->m_Locator->StarMapUpdateGalaxyStarted((GALAXY)galaxy);
	if(m_LocatorStarMapDock)
		m_LocatorStarMapDock->m_Content->StarMapUpdateGalaxyStarted((GALAXY)galaxy);
}

void AuClientFrame::StarMapUpdateSectorStarted(AuId sector, unsigned int range)
{
	if(m_LocatorDock)
		m_LocatorDock->m_Locator->StarMapUpdateSectorStarted(sector, range);
	if(m_LocatorStarMapDock)
		m_LocatorStarMapDock->m_Content->StarMapUpdateSectorStarted(sector, range);
}

void AuClientFrame::StarMapErased()
{
	if(m_LocatorDock)
		m_LocatorDock->m_Locator->StarMapErased();
	if(m_LocatorStarMapDock)
		m_LocatorStarMapDock->m_Content->StarMapErased();
}

void AuClientFrame::StarMapAddSector(AuId sectorId)
{
	// This function does not replace the sector if it already exists.
	if(g_AuClient.StarMapHasSector(sectorId))
		return;

	quint32 empireId = g_AuClient.StarMapEmpire();

	AuEther& ether = g_AuClient.LockEtheriumDelete();
	const DaSector* sector = dynamic_cast<const DaSector*>(ether.FindNormalAtom(sectorId));
	if(sector)
		StarMapAddSector(sector, empireId);
	g_Au.UnlockEtheriumDelete();
}

void AuClientFrame::StarMapInit(quint32 empireId)
{
	g_AuClient.StarMapInit(empireId);

	DaAtomList galaxies;
	AuEther& ether = g_AuClient.LockEtheriumDelete();
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

void AuClientFrame::StarMapAddSector(const DaSector* sector, quint32 empireId)
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

void AuClientFrame::ClientUnitEmpireReceived(quint32 empireId)
{
	// Now that we have the scene atom's empire, we can init the star map using local sector atoms.
	if(g_AuClient.StarMapIsEmpty())
		StarMapInit(empireId);
}

void AuClientFrame::CreateViewWidget()
{
	//qDebug() << "AuClientFrame::CreateViewWidget";
	Q_ASSERT(!m_View);
	m_View = new AuClientView(this);
	connect(m_View, SIGNAL(SceneContextChanged(int)), SLOT(SceneContextChanged(int)));
	connect(m_View, SIGNAL(SceneAtomChanged()), SLOT(SceneAtomChanged()));
	connect(m_View, SIGNAL(ToggleScreenReady()), SLOT(ToggleFullScreen()), Qt::QueuedConnection);

	// Replace the splash screen with the game UI.
	setCentralWidget(m_View);

	// Close the splash screen widget now that it is no longer the central widget.
	if(m_Intro)
	{
		m_Intro->close();
		m_Intro = 0;
	}
}

void AuClientFrame::CreateDockWidgets()
{
	//qDebug() << "AuClientFrame::CreateDockWidgets";
	Q_ASSERT(m_View);

	// Communicator opens all previously opened channels.
	m_CommDock = new AuCommDock(this);
	connect(m_CommDock->m_Comm, SIGNAL(ChoicePicked(quint32, qint8, AuId)), SLOT(ChoicePicked(quint32, qint8, AuId)));
	connect(m_View, SIGNAL(ChatReleased()), m_CommDock->m_Comm, SLOT(FocusChatInput()));
	connect(m_View, SIGNAL(SpotTargetOrTerrain(QString, int, bool, bool)),
			m_CommDock->m_Comm, SLOT(SpotTargetOrTerrain(QString, int, bool, bool)));
	addDockWidget(Qt::BottomDockWidgetArea, m_CommDock, Qt::Horizontal);
	m_CommDock->hide();
	m_DockBars.m_DockWidgets.append(AuFrameDock(m_CommDock));

	// Multi user mode.
	// Character Bio
	m_BioDock = new AuBioDock(this);
	connect(m_BioDock->m_Bio->m_JobsPage, SIGNAL(CloseStoryRequested(AuJobOffer)), auApp, SLOT(CloseStory(AuJobOffer)));
	connect(m_BioDock->m_Bio->m_JobsPage, SIGNAL(StoryBlockRequested(quint32)), auApp, SLOT(StoryResendBlock(quint32)));
	connect(m_BioDock->m_Bio->m_JobsPage, SIGNAL(StoryHintRequested(quint32)), auApp, SLOT(StoryHint(quint32)));
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

	m_MailDock = new AuMailDock(m_View->SceneAtomId(), this);
	connect(m_MailDock, SIGNAL(AlertMessageReceived(AuMailMsg)), SLOT(AlertMessageReceived(AuMailMsg)));
	connect(m_MailDock, SIGNAL(GoDirectToSystem(AuId, QString, AuPoint)), auApp, SLOT(GoDirectToSystem(AuId, QString, AuPoint)));
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

	// Players Online
	m_PlayersOnlineDock = new AuPlayersOnlineDock(this);
	connect(m_PlayersOnlineDock->m_Content, SIGNAL(AddBuddy(AuId, QString)), m_CommDock->m_Comm, SLOT(AddBuddy(AuId, QString)));
	connect(m_PlayersOnlineDock->m_Content, SIGNAL(AddSquelch(AuId, QString)),
			m_CommDock->m_Comm, SLOT(AddSquelch(AuId, QString)));
	connect(m_PlayersOnlineDock->m_Content, SIGNAL(AvatarOnLine(AuId)), m_View, SLOT(AvatarOnLine(AuId)));
	connect(auApp, SIGNAL(PlayersOnlineStatus(AuPlayerOnlineStatusVector)),
			m_PlayersOnlineDock->m_Content, SLOT(PlayersOnlineStatus(AuPlayerOnlineStatusVector)));
	connect(m_PlayersOnlineDock->m_Content, SIGNAL(OpenChannelToAvatar(AuId)), m_CommDock->m_Comm, SLOT(OpenChannelTo(AuId)));
	addDockWidget(Qt::BottomDockWidgetArea, m_PlayersOnlineDock, Qt::Horizontal);
	m_PlayersOnlineDock->hide();
	m_PlayersOnlineDock->setFloating(true);
	m_DockBars.m_DockWidgets.append(AuFrameDock(m_PlayersOnlineDock));

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

	// Restore positions and visibility of dock widgets.
	QByteArray windowState = g_Au.Settings().value("/Widget/MainFrame/WindowState1").toByteArray();
	if(!windowState.isEmpty())
		restoreState(windowState);

	for(QList<AuCommPacket*>::iterator ci = m_EarlyArrivals.begin(); ci != m_EarlyArrivals.end(); ci++)
	{
		ChatReceived(**ci);
		delete *ci;
	}
	m_EarlyArrivals.clear();

	m_DockBars.UpdateTitleShortcuts();

	// Show/hide things appropriately.
	SceneContextChanged(m_View->SceneContext());
}

// 'sceneContext' is an SCENE_CONTEXT value.
void AuClientFrame::SceneContextChanged(int sceneContext)
{
	switch((SCENE_CONTEXT)sceneContext)
	{
	case SCENE_Arena:
	case SCENE_Designer:
	case SCENE_Unknown:
		break;

	case SCENE_Universe:
		if(!menuBar()->isHidden())
			menuBar()->hide();
		break;
	}
}

void AuClientFrame::SetMailRecipient(const AuId& personId)
{
	if(m_MailDock)
		m_MailDock->SetRecipient(personId);
}

void AuClientFrame::AccessPolicy(int policy)
{
	auApp->AccessPolicy(policy);
}

void AuClientFrame::ShieldFuelPolicy(int policy)
{
	auApp->ShieldFuelPolicy(policy);
}

void AuClientFrame::AlertCondition(int condition)
{
	auApp->AlertCondition(condition);
}

void AuClientFrame::PilotUpPolicy(int policy)
{
	auApp->PilotUpPolicy(policy);
}

void AuClientFrame::ShieldUpPolicy(int policy)
{
	auApp->ShieldUpPolicy(policy);
}

void AuClientFrame::SwitchAvatar()
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

void AuClientFrame::SwitchToAvatar(AuId avatarId)
{
	// Set up to wait for a different scene atom to view.
	if(m_View)
		m_View->SetSceneAtom(avatarId);

	// Tell the scene server to make the switch.
	auApp->SwitchToAvatar(avatarId);
}

void AuClientFrame::SwitchToAvatarIndex(int avatarIndex)
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

void AuClientFrame::SwitchToAvatar1()
{
	SwitchToAvatarIndex(0);
}

void AuClientFrame::SwitchToAvatar2()
{
	SwitchToAvatarIndex(1);
}

void AuClientFrame::SwitchToAvatar3()
{
	SwitchToAvatarIndex(2);
}

void AuClientFrame::SwitchToAvatar4()
{
	SwitchToAvatarIndex(3);
}

void AuClientFrame::SwitchToAvatar5()
{
	SwitchToAvatarIndex(4);
}

void AuClientFrame::SwitchToAvatar6()
{
	SwitchToAvatarIndex(5);
}

void AuClientFrame::SwitchAvatarTransferred(AuId transferredId)
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

void AuClientFrame::ConfigureControls(const DiControlInputArray& aAllControls)
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

void AuClientFrame::ControlsChanged()
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

void AuClientFrame::RestoreDefaultControls()
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

void AuClientFrame::LoadControls() const
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

void AuClientFrame::SaveControls() const
{
	// Store my controls in the settings file.
	s_FrameControls.SaveSettings("Controls/Window");
}

// A scene atom should fill this array with control inputs it can accept.
void AuClientFrame::EnumControlInputs(DiControlInputArray& aAllControls) const
{
	// Load my static controls array if it hasn't been loaded yet. This data will only exist on the client since EnumControlInputs
	// is not called on the server.
	if(s_FrameControls.isEmpty())
		LoadControls();

	// Append my controls to the array. I must include titles because the settings UI calls this function to get started.
	for(DiControlInputArray::const_iterator ci = s_FrameControls.cbegin(); ci != s_FrameControls.cend(); ci++)
		aAllControls.append(*ci);
}

void AuClientFrame::KeyPressed(QKeyEvent* e)
{
	//qDebug() << "AuClientFrame::KeyPressed" << e->key() << e->modifiers() << m_ActiveKeys.contains((Qt::Key)e->key());
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

void AuClientFrame::KeyReleased(QKeyEvent* e)
{
	m_ControlLock.lock();
	//qDebug() << "AuClientFrame::KeyReleased" << e->key() << e->modifiers() << m_ActiveKeys.contains((Qt::Key)e->key());
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

void AuClientFrame::ProcessControlInput(const DiControlInputArray& aActiveControls)
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

		case CI_ToggleBuildUI_Deprecated:
		case CI_ToggleCityUI_Deprecated:
			break;

		case CI_ToggleConstructionUI:
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
		case CI_ToggleDesignTextureWrapUI:
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
			ToggleScExchangeUi();
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

		case CI_FullScreenToggle:
			ToggleFullScreen();
			break;
		}
	}
}

void AuClientFrame::MissionSensorContactsReceived(QByteArray bablips)
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

void AuClientFrame::HelpAbout()
{
	//qDebug() << "AuClientFrame::HelpAbout";
	AuAbout about(this);
	ExecDialog(about);
}

void AuClientFrame::ReportSystemResources(AuId systemId, QString systemName, quint8 galaxy, bool isCoreBlackHole, quint8 genHab,
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

void AuClientFrame::CityHistoryReportReady(AuId cityId, AuCityReportHistory report)
{
	AuCityHistoryDlg* dlg = new AuCityHistoryDlg(cityId, report, this);
	dlg->show();
}

void AuClientFrame::CityLastReportReady(AuId cityId, AuCityReport report)
{
	AuCityReportDlg* dlg = new AuCityReportDlg(cityId, report, this);
	dlg->show();
}

void AuClientFrame::setVisible(bool visible)
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
			m_Timer->start(1000);
		}
	}
}

void AuClientFrame::RestoreGeometry()
{
	restoreGeometry(g_Au.Settings().value("/Widget/MainFrame/Geometry").toByteArray());
}

void AuClientFrame::OrdnanceReceived(AuId loaderId, QByteArray ord)
{
	Q_UNUSED(loaderId);
	if(m_HoldDock)
		m_HoldDock->m_Cargo->OrdnanceReceived(ord);
	if(m_HoldDockFree)
		m_HoldDockFree->m_Cargo->OrdnanceReceived(ord);
	if(m_CargoOrdnanceDock)
		m_CargoOrdnanceDock->m_Content->OrdnanceReceived(ord);
}

void AuClientFrame::ArenaExit()
{
	auApp->ArenaExit();
}

void AuClientFrame::ArenaRecall()
{
	auApp->ArenaRecall();
}

void AuClientFrame::ArenaStatus()
{
	auApp->ArenaStatus();
}

void AuClientFrame::SceneAtomChanged()
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
void AuClientFrame::TransporterCoordUI(const AuPoint& personLoc)
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
void AuClientFrame::TransporterLongLatUI(const AuGlobeLLF& ll)
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

void AuClientFrame::PostNetRequests()
{
	//qDebug() << "AuClientFrame::PostNetRequests" << g_AuClient.ImageDownloads().count() << "pending";
	QHash<QUrl, AuImageDownload>& imgdowns = g_AuClient.LockImageDownloads();
	if(!imgdowns.isEmpty())
	{
		// Create the network disk cache if we haven't done so yet.
		if(!m_NetCache)
		{
			//qDebug() << "AuClientFrame::PostNetRequests creating net cache";
			m_NetCache = new QNetworkDiskCache(this);
			m_NetCache->setCacheDirectory(g_Au.CacheNet());
		}

		// Create the network manager if we haven't done so yet.
		if(!m_Net)
		{
			//qDebug() << "AuClientFrame::PostNetRequests creating net access manager";
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

			//qDebug() << "AuClientFrame::PostNetRequests" << di.key() << "requested";
			m_Net->get(QNetworkRequest(di.key()));
			(*di).SetNetRequestPosted();
		}
	}
	g_AuClient.UnlockImageDownloads();
}

void AuClientFrame::NetReplyFinished(QNetworkReply* reply)
{
	//qDebug() << "AuClientFrame::NetReplyFinished" << reply->url().path()
		//<< "from" << (reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute).toBool() ? "cache" : "net");
	QHash<QUrl, AuImageDownload>& imgdowns = g_AuClient.LockImageDownloads();
	QHash<QUrl, AuImageDownload>::iterator di = imgdowns.find(reply->url());
	if(di != imgdowns.end())
		(*di).SetNetRequestFinished();
	g_AuClient.UnlockImageDownloads();
}

QIODevice* AuClientFrame::NetCacheData(const QUrl& url)
{
	if(!m_NetCache)
		return 0;
	return m_NetCache->data(url);
}

void AuClientFrame::ClearNetCache()
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

void AuClientFrame::AlertMessageReceived(AuMailMsg msg)
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

QString AuClientFrame::ControlBinding(CONTROLINPUT control) const
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
			return AuKeyFilter::ModifierName(keymod) + AuKeyFilter::KeyName(key);
		}

	return "";
}

void AuClientFrame::ReplaceSpecies(const DzSpecies& species)
{
	if(m_GearDock)
		m_GearDock->m_Gear->ReplaceSpecies(species);
}

void AuClientFrame::BrokerShipment(AuShipment shipment)
{
	if(m_BuildingDock)
		m_BuildingDock->m_Labor->BrokerShipment(shipment);
	if(m_SpacecraftDock)
		m_SpacecraftDock->m_Content->BrokerShipment(shipment);
	if(m_SpacecraftShipmentsDock)
		m_SpacecraftShipmentsDock->m_Content->Shipment(shipment);
}

void AuClientFrame::BrokerShipmentsClear()
{
	if(m_BuildingDock)
		m_BuildingDock->m_Labor->BrokerShipmentsClear();
	if(m_SpacecraftDock)
		m_SpacecraftDock->m_Content->BrokerShipmentsClear();
	if(m_SpacecraftShipmentsDock)
		m_SpacecraftShipmentsDock->m_Content->ShipmentsClear();
}

void AuClientFrame::BrokerTradeAgreement(AuTradeAgreement ta)
{
	if(m_BuildingDock)
		m_BuildingDock->m_Labor->BrokerTradeAgreement(ta);
}

void AuClientFrame::BrokerTradeAgreementsClear()
{
	if(m_BuildingDock)
		m_BuildingDock->m_Labor->BrokerTradeAgreementsClear();
}

void AuClientFrame::OperateTurboLift(const AuId& shipId, const AuId& personId, quint16 liftId)
{
	Q_UNUSED(shipId);
	Q_UNUSED(personId);
	Q_UNUSED(liftId);
}

void AuClientFrame::KickCryoBerthOccupants(AuSpecies species, int count)
{
	auApp->KickCryoBerthOccupants(species, count);
}

void AuClientFrame::ReviveCryoBerthOccupant(AuSpecies species, quint8 role)
{
	auApp->ReviveCryoBerthOccupant(species, role);
}

bool AuClientFrame::IsAFK() const
{
	if(!m_PlayersOnlineDock)
		return false;
	return m_PlayersOnlineDock->m_Content->IsAFK();
}

void AuClientFrame::DisableAFK()
{
	if(!m_PlayersOnlineDock)
		return;
	if(m_PlayersOnlineDock->m_Content->IsAFK())
		m_PlayersOnlineDock->m_Content->AFKClicked();
}

void AuClientFrame::BuildingCodes()
{
	AuIndustryDlg* dlg = new AuIndustryDlg(this);
	dlg->show();
}

void AuClientFrame::ReportSystemSpacecraft(quint32 empireId, AuId sectorId, AuId systemId, QString systemName)
{
	// Display this thing modelessly.
	AuSystemSpacecraft* dlg = new AuSystemSpacecraft(empireId, sectorId, systemId, systemName, this);
	dlg->show();
}

void AuClientFrame::AvatarsUpdated()
{
	if(m_MailDock)
		m_MailDock->AvatarsUpdated();
}

void AuClientFrame::SpawnRecall()
{
	m_View->SpawnRecall();
}

void AuClientFrame::SpawnTie()
{
	m_View->SpawnTie();
}

void AuClientFrame::GovSetEmpireCapital()
{
	auApp->GovSetCapital(1);
}

void AuClientFrame::GovSetGalaxyCapital()
{
	auApp->GovSetCapital(3);
}

void AuClientFrame::GovSetSectorCapital()
{
	auApp->GovSetCapital(0);
}

void AuClientFrame::SpeciesRequested(AuSpecies species)
{
	auApp->SpeciesRequested(species);
}

void AuClientFrame::TransferCitizensAvailable(QByteArray ba)
{
	if(m_CargoTransferDock)
		m_CargoTransferDock->TransferCitizensAvailable(ba);
	if(m_HoldDock)
		m_HoldDock->TransferCitizensAvailable(ba);
	if(m_HoldDockFree)
		m_HoldDockFree->TransferCitizensAvailable(ba);
}

