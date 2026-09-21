//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auservermonframe.cpp,v 1.38 2022/02/19 18:40:49 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auservermonframe.h"
//
#include "auglobal.h"
#include "auplayerstable.h"
#include "aureportwidget.h"
#include "auservercon.h"
#include "auservermonapp.h"
#include <QGridLayout>
#include <QHeaderView>
#include <QIcon>
#include <QMenu>
#include <QMenuBar>
#include <QSplitter>
#include <QVariant>

AuServerMonFrame::AuServerMonFrame(QWidget* parent) : QMainWindow(parent)
{
	setWindowIcon(QIcon(":data/ServerMon.png"));
	setWindowTitle(auServerMonApp->Name());

	m_Report = new AuReportWidget;

	m_Players = new AuPlayersTable(true, this);
	m_Players->hide();
	connect(m_Players->horizontalHeader(), SIGNAL(sectionResized(int, int, int)), SLOT(SavePlayerColumns()));

	m_ServerCon = new AuServerCon(m_Report, m_Players, this);
	connect(m_ServerCon, SIGNAL(ServerSelected(int)), SLOT(ServerSelected(int)));

	// Assemble the connections list and histograms into a splitter.
	m_Splitter = new QSplitter;
	m_Splitter->setOrientation(Qt::Horizontal);
	m_Splitter->addWidget(m_ServerCon);
	m_Splitter->addWidget(m_Report);
	m_Splitter->addWidget(m_Players);
	m_Splitter->setStretchFactor(1, 1);
	connect(m_Splitter, SIGNAL(splitterMoved(int, int)), SLOT(SplitterMoved(int, int)));

	CreateMenuBar();

	setCentralWidget(m_Splitter);

	// Restore window size and position.
	restoreGeometry(auServerMonApp->Settings().value("/MainFrame/Geometry").toByteArray());
	m_Report->RestoreSettings("/Report", auServerMonApp->Settings());

	QVariant splitterState = auServerMonApp->Settings().value("/MainFrame/Splitter");
	m_Splitter->restoreState(splitterState.toByteArray());

	RestorePlayerColumns();
}

AuServerMonFrame::~AuServerMonFrame()
{
}

void AuServerMonFrame::SplitterMoved(int /*pos*/, int /*index*/)
{
	QVariant var(m_Splitter->saveState());
	auServerMonApp->Settings().setValue("/MainFrame/Splitter", var);
}

void AuServerMonFrame::closeEvent(QCloseEvent* event)
{
	auServerMonApp->Settings().setValue("/MainFrame/Geometry", saveGeometry());
	m_Report->SaveSettings("/Report", auServerMonApp->Settings());

	// Disconnect all connections.
	m_ServerCon->DisconnectAll();

	QMainWindow::closeEvent(event);
}

void AuServerMonFrame::ServerSelected(int serverMode)
{
	//qDebug() << "AuServerMonFrame::ServerSelected" << serverMode;
	switch((AuGlobal::MODE)serverMode)
	{
	case AuGlobal::SceneWorker:
		m_Players->hide();
		m_Report->show();
		break;
	case AuGlobal::SceneMaster:
		m_Report->hide();
		m_Players->show();
		break;
	case AuGlobal::LoginMaster:
	case AuGlobal::LoginWorker:
	case AuGlobal::MailClient:
	case AuGlobal::MailMaster:
	case AuGlobal::MailWorker:
	case AuGlobal::SceneClient:
	case AuGlobal::SceneManager:
	case AuGlobal::WebCGI:
	case AuGlobal::SceneSolo:
		m_Players->hide();
		m_Report->hide();
		break;
	}
}

void AuServerMonFrame::SavePlayerColumns()
{
	// Save the sizes of the table widget's columns.
	for(int col=0; col<m_Players->columnCount(); col++)
		g_Au.Settings().writeEntry(QString("Widget/Players/ColumnWid%1").arg(col), m_Players->columnWidth(col));
}

void AuServerMonFrame::RestorePlayerColumns()
{
	// Restore the sizes of the table widget's columns 1 through 4, not 0 which is the automatically sized column.
	m_Players->horizontalHeader()->blockSignals(true);
	for(int col=0; col<m_Players->columnCount(); col++)
	{
		if(col == 0)
			continue;	// skip the automatically sized column
		int colwid = g_Au.Settings().readNumEntry(QString("Widget/Players/ColumnWid%1").arg(col),-123);
		if(colwid != -123)
			m_Players->setColumnWidth(col, colwid);
	}
	m_Players->horizontalHeader()->blockSignals(false);
}

void AuServerMonFrame::CreateMenuBar()
{
	QMenuBar* mbar = menuBar();
	if(!mbar)
		return;

	QMenu* mConnect = mbar->addMenu(tr("&Connect"));
	mConnect->addAction(tr("&Public"), m_ServerCon, SLOT(ConnectGrid()));
	mConnect->addAction(tr("P&ublic Workers"), m_ServerCon, SLOT(ConnectSceneWorkers()));
	mConnect->addAction(tr("Pu&blic Worker..."), m_ServerCon, SLOT(ConnectPublicWorker()));
	mConnect->addSeparator();
	mConnect->addAction(tr("&Debug"), m_ServerCon, SLOT(ConnectDebugGrid()));
	mConnect->addAction(tr("D&ebug Workers"), m_ServerCon, SLOT(ConnectDebugSceneWorkers()));
	mConnect->addAction(tr("Debug &Worker..."), m_ServerCon, SLOT(ConnectDebugWorker()));

	QMenu* mConnections = mbar->addMenu(tr("&Servers"));

	// Monitor intervals submenu.
	QMenu* monitorMenu = mConnections->addMenu(tr("Start Monitoring"));
	monitorMenu->addAction(tr("60 second intervals"), m_ServerCon, SLOT(Start60Sec()));
	monitorMenu->addAction(tr("30 second intervals"), m_ServerCon, SLOT(Start30Sec()));
	monitorMenu->addAction(tr("15 second intervals"), m_ServerCon, SLOT(Start15Sec()));
	monitorMenu->addAction(tr("10 second intervals"), m_ServerCon, SLOT(Start10Sec()));
	monitorMenu->addAction(tr("5 second intervals"), m_ServerCon, SLOT(Start5Sec()));
	monitorMenu->addAction(tr("3 second intervals"), m_ServerCon, SLOT(Start3Sec()));
	monitorMenu->addAction(tr("2 second intervals"), m_ServerCon, SLOT(Start2Sec()));
	monitorMenu->addAction(tr("1 second intervals"), m_ServerCon, SLOT(Start1Sec()));

	mConnections->addAction(tr("Stop Monitoring"), m_ServerCon, SLOT(StopMonitoring()));
	mConnections->addAction(tr("Probe Servers"), m_ServerCon, SLOT(ProbeServers()));
	mConnections->addAction(tr("Disconnect"), m_ServerCon, SLOT(Disconnect()));
	mConnections->addSeparator();
	mConnections->addAction(tr("Log Messages..."), m_ServerCon, SLOT(LogMessages()));
	mConnections->addSeparator();
	mConnections->addAction(tr("Report Memory Usage"), m_ServerCon, SLOT(ReportMemory()));
	mConnections->addAction(tr("Report Resource Usage"), m_ServerCon, SLOT(ReportResources()));
	mConnections->addAction(tr("Report Resource Usage Of Atom..."), m_ServerCon, SLOT(ReportAtom()));
	mConnections->addAction(tr("Find Unit Id..."), m_ServerCon, SLOT(FindId()));
	mConnections->addAction(tr("Find Unit Name..."), m_ServerCon, SLOT(FindName()));
	mConnections->addSeparator();

	// Enable debug switches submenu.
	QMenu* enableMenu = mConnections->addMenu(tr("Enable Server Code"));
	enableMenu->addAction(tr("All"), m_ServerCon, SLOT(ServerEnableAll()));
	enableMenu->addSeparator();
	enableMenu->addAction(tr("Animal Encounters"), m_ServerCon, SLOT(ServerEnableAnimalEnc()));
	enableMenu->addAction(tr("Creature Decay"), m_ServerCon, SLOT(ServerEnableCreatureDecay()));
	enableMenu->addAction(tr("Job Posting"), m_ServerCon, SLOT(ServerEnableJobPosting()));
	enableMenu->addAction(tr("Pirate Encounters"), m_ServerCon, SLOT(ServerEnablePirateEnc()));
	enableMenu->addAction(tr("Report Officers"), m_ServerCon, SLOT(ServerEnableReportOff()));
	enableMenu->addAction(tr("Station Resupply"), m_ServerCon, SLOT(ServerEnableStationResupply()));
	enableMenu->addAction(tr("System Emission"), m_ServerCon, SLOT(ServerEnableSystemEmission()));
	enableMenu->addAction(tr("Troop Resupply"), m_ServerCon, SLOT(ServerEnableTroopResupply()));
	enableMenu->addAction(tr("Unit Decay"), m_ServerCon, SLOT(ServerEnableUnitDecay()));
	enableMenu->addSeparator();
	enableMenu->addAction(tr("Animal AI"), m_ServerCon, SLOT(ServerEnableAnimalAI()));
	enableMenu->addAction(tr("Character AI"), m_ServerCon, SLOT(ServerEnableCharAI()));
	enableMenu->addAction(tr("Citizen Pathing in Spacecraft"), m_ServerCon, SLOT(ServerEnableCitizenPath()));
	enableMenu->addAction(tr("Cop AI"), m_ServerCon, SLOT(ServerEnableCopAI()));
	enableMenu->addAction(tr("Crew AI"), m_ServerCon, SLOT(ServerEnableCrewAI()));
	enableMenu->addAction(tr("Indigenous AI"), m_ServerCon, SLOT(ServerEnableIndiAI()));
	enableMenu->addAction(tr("Officer AI"), m_ServerCon, SLOT(ServerEnableOffAI()));
	enableMenu->addAction(tr("Passenger AI"), m_ServerCon, SLOT(ServerEnablePassAI()));
	enableMenu->addAction(tr("Troop AI"), m_ServerCon, SLOT(ServerEnableTroopAI()));
	enableMenu->addAction(tr("Worker AI"), m_ServerCon, SLOT(ServerEnableWorkerAI()));
	enableMenu->addSeparator();
	enableMenu->addAction(tr("City Sensor Scans"), m_ServerCon, SLOT(ServerEnableCityScan()));
	enableMenu->addAction(tr("Ship Sensor Scans"), m_ServerCon, SLOT(ServerEnableShipScan()));
	enableMenu->addAction(tr("Station Sensor Scans"), m_ServerCon, SLOT(ServerEnableStationScan()));
	enableMenu->addSeparator();
	enableMenu->addAction(tr("Movement"), m_ServerCon, SLOT(ServerEnableMovement()));
	enableMenu->addAction(tr("SetModified Signal"), m_ServerCon, SLOT(ServerEnableSetModified()));

	// Disable debug switches submenu.
	QMenu* disableMenu = mConnections->addMenu(tr("Disable Server Code"));
	disableMenu->addAction(tr("All"), m_ServerCon, SLOT(ServerDisableAll()));
	disableMenu->addSeparator();
	disableMenu->addAction(tr("Animal Encounters"), m_ServerCon, SLOT(ServerDisableAnimalEnc()));
	disableMenu->addAction(tr("Creature Decay"), m_ServerCon, SLOT(ServerDisableCreatureDecay()));
	disableMenu->addAction(tr("Job Posting"), m_ServerCon, SLOT(ServerDisableJobPosting()));
	disableMenu->addAction(tr("Pirate Encounters"), m_ServerCon, SLOT(ServerDisablePirateEnc()));
	disableMenu->addAction(tr("Report Officers"), m_ServerCon, SLOT(ServerDisableReportOff()));
	disableMenu->addAction(tr("Station Resupply"), m_ServerCon, SLOT(ServerDisableStationResupply()));
	disableMenu->addAction(tr("System Emission"), m_ServerCon, SLOT(ServerDisableSystemEmission()));
	disableMenu->addAction(tr("Troop Resupply"), m_ServerCon, SLOT(ServerDisableTroopResupply()));
	disableMenu->addAction(tr("Unit Decay"), m_ServerCon, SLOT(ServerDisableUnitDecay()));
	disableMenu->addSeparator();
	disableMenu->addAction(tr("Animal AI"), m_ServerCon, SLOT(ServerDisableAnimalAI()));
	disableMenu->addAction(tr("Character AI"), m_ServerCon, SLOT(ServerDisableCharAI()));
	disableMenu->addAction(tr("Citizen Pathing in Spacecraft"), m_ServerCon, SLOT(ServerDisableCitizenPath()));
	disableMenu->addAction(tr("Cop AI"), m_ServerCon, SLOT(ServerDisableCopAI()));
	disableMenu->addAction(tr("Crew AI"), m_ServerCon, SLOT(ServerDisableCrewAI()));
	disableMenu->addAction(tr("Indigenous AI"), m_ServerCon, SLOT(ServerDisableIndiAI()));
	disableMenu->addAction(tr("Officer AI"), m_ServerCon, SLOT(ServerDisableOffAI()));
	disableMenu->addAction(tr("Passenger AI"), m_ServerCon, SLOT(ServerDisablePassAI()));
	disableMenu->addAction(tr("Troop AI"), m_ServerCon, SLOT(ServerDisableTroopAI()));
	disableMenu->addAction(tr("Worker AI"), m_ServerCon, SLOT(ServerDisableWorkerAI()));
	disableMenu->addSeparator();
	disableMenu->addAction(tr("City Sensor Scans"), m_ServerCon, SLOT(ServerDisableCityScan()));
	disableMenu->addAction(tr("Ship Sensor Scans"), m_ServerCon, SLOT(ServerDisableShipScan()));
	disableMenu->addAction(tr("Station Sensor Scans"), m_ServerCon, SLOT(ServerDisableStationScan()));
	disableMenu->addSeparator();
	disableMenu->addAction(tr("Movement"), m_ServerCon, SLOT(ServerDisableMovement()));
	disableMenu->addAction(tr("SetModified Signal"), m_ServerCon, SLOT(ServerDisableSetModified()));

	mConnections->addSeparator();

	QMenu* storyMenu = mConnections->addMenu(tr("Story"));
	storyMenu->addAction(tr("Reload All Scripts"), m_ServerCon, SLOT(StoryReloadAllScripts()));
	storyMenu->addAction(tr("Reload Script..."), m_ServerCon, SLOT(StoryReloadScript()));

	QMenu* tickMenu = mConnections->addMenu(tr("Tick"));
	tickMenu->addAction(tr("Enable Tick"), m_ServerCon, SLOT(TickEnable()));
	tickMenu->addAction(tr("Disable Tick"), m_ServerCon, SLOT(TickDisable()));
	tickMenu->addSeparator();
	tickMenu->addAction(tr("Include Atom Id..."), m_ServerCon, SLOT(TickEnableAtomId()));
	tickMenu->addAction(tr("Include Atom Type..."), m_ServerCon, SLOT(TickEnableAtomType()));
	tickMenu->addSeparator();
	tickMenu->addAction(tr("Exclude Atom Id..."), m_ServerCon, SLOT(TickExcludeAtomId()));
	tickMenu->addAction(tr("Exclude Atom Type..."), m_ServerCon, SLOT(TickExcludeAtomType()));
	tickMenu->addSeparator();
	tickMenu->addAction(tr("Clear Atom Id Exclusions"), m_ServerCon, SLOT(TickClearAtomIdExclusions()));
	tickMenu->addAction(tr("Clear Atom Type Exclusions"), m_ServerCon, SLOT(TickClearAtomTypeExclusions()));

	QMenu* tockMenu = mConnections->addMenu(tr("Tock"));
	tockMenu->addAction(tr("Enable Tock"), m_ServerCon, SLOT(TockEnable()));
	tockMenu->addAction(tr("Disable Tock"), m_ServerCon, SLOT(TockDisable()));
	tockMenu->addSeparator();
	tockMenu->addAction(tr("Include Atom Id..."), m_ServerCon, SLOT(TockEnableAtomId()));
	tockMenu->addAction(tr("Include Atom Type..."), m_ServerCon, SLOT(TockEnableAtomType()));
	tockMenu->addSeparator();
	tockMenu->addAction(tr("Exclude Atom Id..."), m_ServerCon, SLOT(TockExcludeAtomId()));
	tockMenu->addAction(tr("Exclude Atom Type..."), m_ServerCon, SLOT(TockExcludeAtomType()));
	tockMenu->addSeparator();
	tockMenu->addAction(tr("Clear Atom Id Exclusions"), m_ServerCon, SLOT(TockClearAtomIdExclusions()));
	tockMenu->addAction(tr("Clear Atom Type Exclusions"), m_ServerCon, SLOT(TockClearAtomTypeExclusions()));

	mConnections->addSeparator();

	mConnections->addAction(tr("Create Sector At..."), m_ServerCon, SLOT(CreateSectorAt()));
	mConnections->addAction(tr("Finalize Sector/Solar System..."), m_ServerCon, SLOT(FinalizeSectorOrSystem()));
	mConnections->addAction(tr("Reassign Solar System..."), m_ServerCon, SLOT(ReassignSystem()));
	mConnections->addAction(tr("Revoke Sector/Solar System Assignment..."), m_ServerCon, SLOT(RevokeSectorOrSystem()));
	mConnections->addSeparator();
	mConnections->addAction(tr("Restore Empire..."), m_ServerCon, SLOT(RestoreEmpire()));
	mConnections->addAction(tr("Limbo Rescue Atom..."), m_ServerCon, SLOT(LimboRescue()));
	mConnections->addAction(tr("Limbo Rescue Atom List..."), m_ServerCon, SLOT(LimboRescueList()));
	mConnections->addSeparator();
	mConnections->addAction(tr("Reload Static Data"), m_ServerCon, SLOT(ReloadStatic()));
	mConnections->addAction(tr("Set Atom Modified..."), m_ServerCon, SLOT(SetAtomModified()));
	mConnections->addAction(tr("Set Control Decay..."), m_ServerCon, SLOT(SetControlDecay()));
	mConnections->addAction(tr("Set UDP Packet Decay..."), m_ServerCon, SLOT(SetUDPDecay()));
	mConnections->addSeparator();
	mConnections->addAction(tr("Shutdown Server..."), m_ServerCon, SLOT(ShutdownServer()));
	mConnections->addAction(tr("Halt Server Shutdown..."), m_ServerCon, SLOT(HaltShutdown()));

	QMenu* mUniverse = mbar->addMenu(tr("&Database"));
	mUniverse->addAction(tr("&Player Account Manager..."), m_ServerCon, SLOT(ManageAccounts()));
	mUniverse->addSeparator();
	mUniverse->addAction(tr("Query Atom &Id..."), m_ServerCon, SLOT(AtomQuery()));
	mUniverse->addAction(tr("Query Avatar &Name..."), m_ServerCon, SLOT(AvatarQuery()));
	mUniverse->addAction(tr("Query Spacecraf&t Name..."), m_ServerCon, SLOT(SpacecraftQuery()));
	mUniverse->addAction(tr("Query &World Id..."), m_ServerCon, SLOT(WorldIdQuery()));
	mUniverse->addAction(tr("Query W&orld Name..."), m_ServerCon, SLOT(WorldNameQuery()));
	mUniverse->addAction(tr("Query &System Id..."), m_ServerCon, SLOT(SystemIdQuery()));
	mUniverse->addAction(tr("Query S&ystem Name..."), m_ServerCon, SLOT(SystemNameQuery()));
	mUniverse->addSeparator();
	mUniverse->addAction(tr("&Reload Empire Data"), m_ServerCon, SLOT(ReloadGameData()));
}

