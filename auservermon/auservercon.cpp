//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auservercon.cpp,v 1.210 2022/06/09 23:01:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auservercon.h"
//
#include "auaccountmanagerdlg.h"
#include "auatommanager.h"
#include "audebugmsgbits.h"
#include "audef.h"
#include "augear.h"
#include "auglobal.h"
#include "auid.h"
#include "aulistwidget.h"
#include "aumemory.h"
#include "auplayerstable.h"
#include "aureportdialog.h"
#include "aureportwidget.h"
#include "auservermonapp.h"
#include "auservermonframe.h"
#include "aushutdowndlg.h"
#include "ausound.h"
#include "austubglobal.h"
#include "daatom.h"
#include "daitem.h"
#include "dgdata.h"
#include "dtmgrclient.h"
#include <QComboBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QResizeEvent>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTimer>
#include <QToolButton>
#include <QVariant>

#define d_AlertSecs		(60*4)
#define d_Prefix		""
#define d_PrefixStep	"&nbsp; "
#define d_IconSize		18

// The icon changes every 100ms so every 10 icons is 1 second.
static QString s_PingIcons[] =
{
	QString(":data/LinkActive0.png"),
	QString(":data/LinkActive0.png"),
	QString(":data/LinkActive0.png"),
	QString(":data/LinkActive0.png"),
	QString(":data/LinkActive0.png"),
	QString(":data/LinkActive0.png"),
	QString(":data/LinkActive0.png"),
	QString(":data/LinkActive0.png"),
	QString(":data/LinkActive0.png"),
	QString(":data/LinkActive0.png"),

	QString(":data/LinkActive0.png"),
	QString(":data/LinkActive0.png"),
	QString(":data/LinkActive0.png"),
	QString(":data/LinkActive0.png"),
	QString(":data/LinkActive0.png"),
	QString(":data/LinkActive0.png"),
	QString(":data/LinkActive0.png"),
	QString(":data/LinkActive0.png"),
	QString(":data/LinkActive0.png"),
	QString(":data/LinkActive0.png"),

	QString(":data/LinkActive0.png"),
	QString(":data/LinkActive0.png"),
	QString(":data/LinkActive0.png"),
	QString(":data/LinkActive1.png"),
	QString(":data/LinkActive2.png"),
	QString(":data/LinkActive3.png"),
	QString(":data/LinkActive4.png"),
	QString(":data/LinkActive5.png"),
	QString(":data/LinkActive6.png"),
	QString(":data/LinkActive7.png"),
};
static unsigned int s_PingIconCount = sizeof(s_PingIcons)/sizeof(QString);

AuServerCon::AuServerCon(AuReportWidget* report, AuPlayersTable* playersOn, QWidget* parent) :
	QWidget(parent), m_Fire(":data/Fire.png")
{
	m_DbFailed = false;
	m_Dice.TimeSeed();
	m_GdLoaded = true;	// until requested
	m_OperateOnSelectedOnly = false;
	m_PingIcon = 0;
	m_PlayerCount = 0;
	m_PlayersOn = playersOn;
	m_Report = report;

	int splitServers = true;	// true to split the 64 core computers into multiple server processes, false for one big process.
	m_PublicServers.append(AuServerChoice(AuGlobal::SceneMaster, "Master u5", "u5.Hazeron.net", 16762));
	m_PublicServers.append(AuServerChoice(AuGlobal::SceneManager, "Manager u6", "u6.Hazeron.net", 16762));
	//m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u1", "u1.Hazeron.net", 16763));
	//m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u2", "u2.Hazeron.net", 16763));
	//m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u3", "u3.Hazeron.net", 16763));
	//m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u4", "u4.Hazeron.net", 16763));
	//m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u5", "u5.Hazeron.net", 16763));
	//m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u6", "u6.Hazeron.net", 16763));
	m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u7", "u7.Hazeron.net", 16763));
	if(!splitServers)
	{
		m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u8", "u8.Hazeron.net", 16763));
		m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u9", "u9.Hazeron.net", 16763));
		m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u10", "u10.Hazeron.net", 16763));
		m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u11", "u11.Hazeron.net", 16763));
		m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u12", "u12.Hazeron.net", 16763));
		m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u13", "u13.Hazeron.net", 16763));
		m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u14", "u14.Hazeron.net", 16763));
		m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u15", "u15.Hazeron.net", 16763));
	}
	else
	{
		m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u8a", "u8.Hazeron.net", 16763));
		m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u8b", "u8b.Hazeron.net", 16767));
		m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u9a", "u9.Hazeron.net", 16763));
		m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u9b", "u9b.Hazeron.net", 16767));
		m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u10a", "u10.Hazeron.net", 16763));
		m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u10b", "u10b.Hazeron.net", 16767));
		m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u11a", "u11.Hazeron.net", 16763));
		m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u11b", "u11b.Hazeron.net", 16767));
		m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u12a", "u12.Hazeron.net", 16763));
		m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u12b", "u12b.Hazeron.net", 16767));
		m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u13a", "u13.Hazeron.net", 16763));
		m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u13b", "u13b.Hazeron.net", 16767));
		m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u14a", "u14.Hazeron.net", 16763));
		m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u14b", "u14b.Hazeron.net", 16767));
		m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u15a", "u15.Hazeron.net", 16763));
		m_PublicServers.append(AuServerChoice(AuGlobal::SceneWorker, "u15b", "u15b.Hazeron.net", 16767));
	}
	m_DebugServers.append(AuServerChoice(AuGlobal::SceneMaster, "Master Huey", "u0d.Hazeron.net", 16762));
	m_DebugServers.append(AuServerChoice(AuGlobal::SceneManager, "Manager Dewey", "u1d.Hazeron.net", 16762));
	m_DebugServers.append(AuServerChoice(AuGlobal::SceneWorker, "Dewey", "u1d.Hazeron.net", 16763));
	m_DebugServers.append(AuServerChoice(AuGlobal::SceneWorker, "Louie", "u2d.Hazeron.net", 16763));

	/* I let this run for almost an hour and it still wasn't done. Takes 30 seconds on the server.
	QPushButton* pblimbo = new QPushButton("Limbo Check");
	connect(pblimbo, SIGNAL(clicked()), SLOT(LimboCheck()));
	pblimbo->setToolTip("<html>Reports atoms lost in limbo for various causes. "
			"<b>Warning: This takes a long time.</b></html>");
			*/

	m_ConnectionsList = new AuListWidget;
	m_ConnectionsList->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_ConnectionsList->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_ConnectionsList->setSortingEnabled(true);
	connect(m_ConnectionsList, SIGNAL(itemPressed(QListWidgetItem*)), SLOT(ConnectionClicked(QListWidgetItem*)));
	m_ConnectionsList->setToolTip("<html><p>Right-click to perform actions on servers that are selected.</p>"
			"<p>"
			"When monitoring:<br>"
			"⚙ Load %<br>"
			"🌡️ Heavy Load %<br>"
			"⏱ Average Cycle ms<br>"
			"🔓 Etherium Lock ms<br>"
			"🔒 Etherium Locked<br>"
			"⚛ Busy Atoms<br>"
			"🌎 Global Data Locked<br>"
			"🏠 Building Designs Locked<br>"
			"🕹 Item Designs Locked<br>"
			"🟊 Spacecraft Designs Locked<br>"
			"🗑️ Trash Locked"
			"</p>"
			"</html>");

	m_PlayersInfo = new QLabel(this);
	m_PlayersInfo->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	QLabel* lbServers = new QLabel(this);
	lbServers->setPixmap(QPixmap(":data/Server.png").scaled(d_IconSize,d_IconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));

	m_ConnectionsInfo = new QLabel(this);

	QLabel* lbAv = new QLabel(this);
	lbAv->setPixmap(QPixmap(":data/c_VacSuitHelmet.png").scaled(d_IconSize,d_IconSize, Qt::KeepAspectRatio,
				Qt::SmoothTransformation));

	QHBoxLayout* ilay = new QHBoxLayout;
	ilay->addWidget(lbServers);
	ilay->addWidget(m_ConnectionsInfo, 1);
	ilay->addWidget(lbAv);
	ilay->addWidget(m_PlayersInfo);

	QVBoxLayout* conlay = new QVBoxLayout;
	conlay->addLayout(ilay);
	conlay->addWidget(m_ConnectionsList);
	conlay->setMargin(2);
	conlay->setSpacing(2);
	setLayout(conlay);

	// Get an update timer started.
	QTimer* timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), SLOT(TimerUpdate()));
	timer->start(100);
}

AuServerCon::~AuServerCon()
{
}

int AuServerCon::ConnectToServer(int serverMode, const QString& listName, const QString& netName, quint16 netPort, int sortOrder,
		int msConnectDelay)
{
	if(netName.isEmpty())
		return 1;	// Can't continue without a server name.

	m_ConnectionsLock.lock();
	for(QList<DtMgrClient*>::const_iterator cit = m_Connections.constBegin(); cit != m_Connections.constEnd(); cit++)
	{
		if(!(*cit)->IsConnected())
			continue;
		if((*cit)->ConnectionName() == netName && (*cit)->ConnectionPort() == netPort)
		{
			m_ConnectionsLock.unlock();
			return 2;
		}
	}

	// Add a new connection to our list of connection.
	DtMgrClient* mgrConnection = new DtMgrClient(this);
	int maxHistory = width()/2;	// History can be limited to half the frame width because the histograms are side-by-side.
	mgrConnection->SetMaxHistory(maxHistory);
	if(mgrConnection)
	{
		// Configure/start the connection.
		mgrConnection->SetConnectionInfo(netName, netPort, serverMode);
		mgrConnection->SetConnectDelay(msConnectDelay);
		connect(mgrConnection, SIGNAL(finished()), SLOT(ConnectionClosed()));
		connect(this, SIGNAL(Broadcast(QString, quint16, QString)),
				mgrConnection, SLOT(Broadcast(QString, quint16, QString)));
		connect(this, SIGNAL(CreateSectorAt(QString, quint16, quint8, double,double,double)),
				mgrConnection, SLOT(CreateSectorAt(QString, quint16, quint8, double,double,double)));
		connect(this, SIGNAL(DebugMessages(QString, quint16, quint32)),
				mgrConnection, SLOT(DebugMessages(QString, quint16, quint32)));
		connect(this, SIGNAL(DebugSwitch(QString, quint16, quint8, quint8, quint32)),
				mgrConnection, SLOT(DebugSwitch(QString, quint16, quint8, quint8, quint32)));
		connect(this, SIGNAL(Disconnect(QString, quint16)), mgrConnection, SLOT(Disconnect(QString, quint16)));
		connect(this, SIGNAL(FinalizeSystem(QString, quint16, quint32)),
				mgrConnection, SLOT(FinalizeSystem(QString, quint16, quint32)));
		connect(this, SIGNAL(FindUnitId(QString, quint16, quint32)), mgrConnection, SLOT(FindUnitId(QString, quint16, quint32)));
		connect(this, SIGNAL(FindUnitName(QString, quint16, QString)),
				mgrConnection, SLOT(FindUnitName(QString, quint16, QString)));
		connect(this, SIGNAL(SetMaxHistory(int)), mgrConnection, SLOT(SetMaxHistory(int)));
		connect(m_Report, SIGNAL(KickUnit(QString, quint16, quint32)), mgrConnection, SLOT(KickUnit(QString, quint16, quint32)));
		connect(this, SIGNAL(RestoreEmpire(QString, quint16, DgEmpire)),
				mgrConnection, SLOT(RestoreEmpire(QString, quint16, DgEmpire)));
		connect(this, SIGNAL(LimboRescue(QString, quint16, quint32)), mgrConnection, SLOT(LimboRescue(QString, quint16, quint32)));
		connect(this, SIGNAL(LimboRescueList(QString, quint16, QList<quint32>)),
				mgrConnection, SLOT(LimboRescueList(QString, quint16, QList<quint32>)));
		connect(this, SIGNAL(Probe(QString, quint16)), mgrConnection, SLOT(Probe(QString, quint16)));
		connect(this, SIGNAL(Shutdown(QString, quint16, QString, QString, qint16)),
				mgrConnection, SLOT(Shutdown(QString, quint16, QString, QString, qint16)));
		connect(this, SIGNAL(ShutdownHalt(QString, quint16, QString)),
				mgrConnection, SLOT(ShutdownHalt(QString, quint16, QString)));
		connect(this, SIGNAL(StartPolling(int, QString, quint16)), mgrConnection, SLOT(StartPolling(int, QString, quint16)));
		connect(this, SIGNAL(StopPolling(QString, quint16)), mgrConnection, SLOT(StopPolling(QString, quint16)));
		connect(this, SIGNAL(ReassignAtom(QString, quint16, quint32, QString, quint16)),
				mgrConnection, SLOT(ReassignAtom(QString, quint16, quint32, QString, quint16)));
		connect(this, SIGNAL(RequestResourceReport(QString, quint16, quint8, quint32)),
				mgrConnection, SLOT(RequestResourceReport(QString, quint16, quint8, quint32)));
		connect(this, SIGNAL(ReloadScript(QString, quint16, quint32)),
				mgrConnection, SLOT(ReloadScript(QString, quint16, quint32)));
		connect(this, SIGNAL(RequestMemoryUsage(QString, quint16)),
				mgrConnection, SLOT(RequestMemoryUsage(QString, quint16)));
		connect(this, SIGNAL(RequestReloadData(QString, quint16)), mgrConnection, SLOT(RequestReloadData(QString, quint16)));
		connect(this, SIGNAL(RevokeAtom(QString, quint16, quint32)), mgrConnection, SLOT(RevokeAtom(QString, quint16, quint32)));
		connect(this, SIGNAL(SetAtomModified(QString, quint16, quint32, bool)),
				mgrConnection, SLOT(SetAtomModified(QString, quint16, quint32, bool)));
		connect(this, SIGNAL(SetControlDecay(QString, quint16, quint16)),
				mgrConnection, SLOT(SetControlDecay(QString, quint16, quint16)));
		connect(this, SIGNAL(SetUdpDecay(QString, quint16, quint16)), mgrConnection, SLOT(SetUdpDecay(QString, quint16, quint16)));
		connect(mgrConnection, SIGNAL(MemoryUsageReport(QString, quint16, QStringList)),
				SLOT(MemoryUsageReport(QString, quint16, QStringList)));
		connect(mgrConnection, SIGNAL(ResourceReport(AuResourceReport)), SLOT(ResourceReport(AuResourceReport)));
		connect(mgrConnection, SIGNAL(ResourceReportAtom(AuResourceReportAtom)), SLOT(ResourceReportAtom(AuResourceReportAtom)));
		connect(mgrConnection, SIGNAL(MonitorAnswered(QString, quint16, unsigned int, float, int,unsigned int,bool,bool,bool,bool, bool)),
				SLOT(MonitorAnswered(QString, quint16, unsigned int, float, int, unsigned int, bool,bool,bool,bool, bool)));
		connect(mgrConnection, SIGNAL(ProbeAnswered(QString, quint16, unsigned int, float, int, unsigned int, bool,bool,bool,bool, bool)),
				SLOT(ProbeAnswered(QString, quint16, unsigned int, float, int, unsigned int, bool,bool,bool,bool, bool)));
		connect(mgrConnection, SIGNAL(UnitSearchResult(QString, quint16, quint32, QString)),
				SLOT(UnitSearchResult(QString, quint16, quint32, QString)));
		mgrConnection->start();

		// Add the connection to our list of connections.
		m_Connections.append(mgrConnection);

		// Add an item to the list widget.
		AuListWidgetItem* col0 = new AuListWidgetItem(
				QIcon(QPixmap(":data/Server.png").scaled(d_IconSize,d_IconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)),
				listName);
		col0->SetSortOrder(sortOrder);
		m_ConnectionsList->addItem(col0);
		col0->setData(Qt::UserRole, netName);
		col0->setData(Qt::UserRole+5, serverMode);
		col0->setData(Qt::UserRole+7, netPort);
		col0->setData(Qt::UserRole+8, listName);
	}
	int connections = m_Connections.count();
	m_ConnectionsLock.unlock();
	if(connections)
		m_ConnectionsInfo->setText(QString("%1 Servers").arg(connections));
	else
		m_ConnectionsInfo->setText(QString("No Servers"));
	return 0;
}

void AuServerCon::ConnectionClosed()
{
	//qDebug("AuServerCon::ConnectionClosed");
	// Remove all finished connections from our list of connections.
	m_ConnectionsLock.lock();
	QList<DtMgrClient*>::iterator cit = m_Connections.begin();
	while(cit != m_Connections.end())
	{
		if((*cit)->isFinished())
		{
			// Remove the item from our list widget.
			for(int row=0; row<m_ConnectionsList->count(); row++)
			{
				QListWidgetItem* item = m_ConnectionsList->item(row);
				if(item)
				{
					QString netName = item->data(Qt::UserRole).toString();
					quint16 netPort = item->data(Qt::UserRole+7).toUInt();
					if((*cit)->ConnectionName() == netName && (*cit)->ConnectionPort() == netPort)
					{
						delete m_ConnectionsList->takeItem(row);
						break;
					}
				}
			}

			// Remove the connection from our list of connections.
			delete *cit;
			cit = m_Connections.erase(cit);
		}
		else
			cit++;
	}
	int connections = m_Connections.count();
	m_ConnectionsLock.unlock();
	if(connections)
		m_ConnectionsInfo->setText(QString("%1 Servers").arg(connections));
	else
		m_ConnectionsInfo->setText(QString("No Servers"));
}

void AuServerCon::ConnectionClicked(QListWidgetItem* item)
{
	if(m_ConnectionsList->LastButtonDown() == Qt::RightButton)
	{
		// Get info about the connected servers that are selected.
		m_OperateOnSelectedOnly = true;
		QList<int> rows;
		QStringList serverTitles;
		QStringList serverNames;
		QList<quint16> serverPorts;
		Servers(rows, serverTitles, serverNames, serverPorts);
		if(rows.isEmpty())
			return;
		ShowMenu(rows);
		m_OperateOnSelectedOnly = false;	// Restore to normal 'all' state.
	}
	else
	{
		// Server mode could change lower pane display.
		int serverMode = item->data(Qt::UserRole+5).toInt();
		emit ServerSelected(serverMode);
	}
}

void AuServerCon::AllServers()
{
	// Get info about all connected servers.
	m_OperateOnSelectedOnly = false;
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;
	ShowMenu(rows);
}

void AuServerCon::ShowMenu(const QList<int>& rows)
{
	// Show a pop-up menu of options.
	QMenu tickMenu(tr("Tick"));
	tickMenu.addAction(tr("Enable Tick"), this, SLOT(TickEnable()));
	tickMenu.addAction(tr("Disable Tick"), this, SLOT(TickDisable()));
	tickMenu.addSeparator();
	tickMenu.addAction(tr("Include Atom Id..."), this, SLOT(TickEnableAtomId()));
	tickMenu.addAction(tr("Include Atom Type..."), this, SLOT(TickEnableAtomType()));
	tickMenu.addSeparator();
	tickMenu.addAction(tr("Exclude Atom Id..."), this, SLOT(TickExcludeAtomId()));
	tickMenu.addAction(tr("Exclude Atom Type..."), this, SLOT(TickExcludeAtomType()));
	tickMenu.addSeparator();
	tickMenu.addAction(tr("Clear Atom Id Exclusions"), this, SLOT(TickClearAtomIdExclusions()));
	tickMenu.addAction(tr("Clear Atom Type Exclusions"), this, SLOT(TickClearAtomTypeExclusions()));

	QMenu tockMenu(tr("Tock"));
	tockMenu.addAction(tr("Enable Tock"), this, SLOT(TockEnable()));
	tockMenu.addAction(tr("Disable Tock"), this, SLOT(TockDisable()));
	tockMenu.addSeparator();
	tockMenu.addAction(tr("Include Atom Id..."), this, SLOT(TockEnableAtomId()));
	tockMenu.addAction(tr("Include Atom Type..."), this, SLOT(TockEnableAtomType()));
	tockMenu.addSeparator();
	tockMenu.addAction(tr("Exclude Atom Id..."), this, SLOT(TockExcludeAtomId()));
	tockMenu.addAction(tr("Exclude Atom Type..."), this, SLOT(TockExcludeAtomType()));
	tockMenu.addSeparator();
	tockMenu.addAction(tr("Clear Atom Id Exclusions"), this, SLOT(TockClearAtomIdExclusions()));
	tockMenu.addAction(tr("Clear Atom Type Exclusions"), this, SLOT(TockClearAtomTypeExclusions()));

	QMenu storyMenu(tr("Story"));
	storyMenu.addAction(tr("Reload All Scripts"), this, SLOT(StoryReloadAllScripts()));
	storyMenu.addAction(tr("Reload Script..."), this, SLOT(StoryReloadScript()));

	// Enable debug switches submenu.
	QMenu enableMenu(tr("Enable Server Code"));
	enableMenu.addAction(tr("All"), this, SLOT(ServerEnableAll()));
	enableMenu.addSeparator();
	enableMenu.addAction(tr("Animal Encounters"), this, SLOT(ServerEnableAnimalEnc()));
	enableMenu.addAction(tr("Creature Decay"), this, SLOT(ServerEnableCreatureDecay()));
	enableMenu.addAction(tr("Job Posting"), this, SLOT(ServerEnableJobPosting()));
	enableMenu.addAction(tr("Pirate Encounters"), this, SLOT(ServerEnablePirateEnc()));
	enableMenu.addAction(tr("Report Officers"), this, SLOT(ServerEnableReportOff()));
	enableMenu.addAction(tr("Station Resupply"), this, SLOT(ServerEnableStationResupply()));
	enableMenu.addAction(tr("System Emission"), this, SLOT(ServerEnableSystemEmission()));
	enableMenu.addAction(tr("Troop Resupply"), this, SLOT(ServerEnableTroopResupply()));
	enableMenu.addAction(tr("Unit Decay"), this, SLOT(ServerEnableUnitDecay()));
	enableMenu.addSeparator();
	enableMenu.addAction(tr("Animal AI"), this, SLOT(ServerEnableAnimalAI()));
	enableMenu.addAction(tr("Character AI"), this, SLOT(ServerEnableCharAI()));
	enableMenu.addAction(tr("Citizen Pathing in Spacecraft"), this,SLOT(ServerEnableCitizenPath()));
	enableMenu.addAction(tr("Cop AI"), this, SLOT(ServerEnableCopAI()));
	enableMenu.addAction(tr("Crew AI"), this, SLOT(ServerEnableCrewAI()));
	enableMenu.addAction(tr("Indigenous AI"), this, SLOT(ServerEnableIndiAI()));
	enableMenu.addAction(tr("Officer AI"), this, SLOT(ServerEnableOffAI()));
	enableMenu.addAction(tr("Passenger AI"), this, SLOT(ServerEnablePassAI()));
	enableMenu.addAction(tr("Troop AI"), this, SLOT(ServerEnableTroopAI()));
	enableMenu.addAction(tr("Worker AI"), this, SLOT(ServerEnableWorkerAI()));
	enableMenu.addSeparator();
	enableMenu.addAction(tr("City Sensor Scans"), this, SLOT(ServerEnableCityScan()));
	enableMenu.addAction(tr("Ship Sensor Scans"), this, SLOT(ServerEnableShipScan()));
	enableMenu.addAction(tr("Station Sensor Scans"), this, SLOT(ServerEnableStationScan()));
	enableMenu.addSeparator();
	enableMenu.addAction(tr("Movement"), this, SLOT(ServerEnableMovement()));
	enableMenu.addAction(tr("SetModified Signal"), this, SLOT(ServerEnableSetModified()));
	enableMenu.addAction(tr("Tock Transactions Logger"), this, SLOT(ServerEnableTockTransactions()));

	// Disable debug switches submenu.
	QMenu disableMenu(tr("Disable Server Code"));
	disableMenu.addAction(tr("All"), this, SLOT(ServerDisableAll()));
	disableMenu.addSeparator();
	disableMenu.addAction(tr("Animal Encounters"), this, SLOT(ServerDisableAnimalEnc()));
	disableMenu.addAction(tr("Creature Decay"), this, SLOT(ServerDisableCreatureDecay()));
	disableMenu.addAction(tr("Job Posting"), this, SLOT(ServerDisableJobPosting()));
	disableMenu.addAction(tr("Pirate Encounters"), this, SLOT(ServerDisablePirateEnc()));
	disableMenu.addAction(tr("Report Officers"), this, SLOT(ServerDisableReportOff()));
	disableMenu.addAction(tr("Station Resupply"), this, SLOT(ServerDisableStationResupply()));
	disableMenu.addAction(tr("System Emission"), this, SLOT(ServerDisableSystemEmission()));
	disableMenu.addAction(tr("Troop Resupply"), this, SLOT(ServerDisableTroopResupply()));
	disableMenu.addAction(tr("Unit Decay"), this, SLOT(ServerDisableUnitDecay()));
	disableMenu.addSeparator();
	disableMenu.addAction(tr("Animal AI"), this, SLOT(ServerDisableAnimalAI()));
	disableMenu.addAction(tr("Character AI"), this, SLOT(ServerDisableCharAI()));
	disableMenu.addAction(tr("Citizen Pathing in Spacecraft"), this, SLOT(ServerDisableCitizenPath()));
	disableMenu.addAction(tr("Cop AI"), this, SLOT(ServerDisableCopAI()));
	disableMenu.addAction(tr("Crew AI"), this, SLOT(ServerDisableCrewAI()));
	disableMenu.addAction(tr("Indigenous AI"), this, SLOT(ServerDisableIndiAI()));
	disableMenu.addAction(tr("Officer AI"), this, SLOT(ServerDisableOffAI()));
	disableMenu.addAction(tr("Passenger AI"), this, SLOT(ServerDisablePassAI()));
	disableMenu.addAction(tr("Troop AI"), this, SLOT(ServerDisableTroopAI()));
	disableMenu.addAction(tr("Worker AI"), this, SLOT(ServerDisableWorkerAI()));
	disableMenu.addSeparator();
	disableMenu.addAction(tr("City Sensor Scans"), this, SLOT(ServerDisableCityScan()));
	disableMenu.addAction(tr("Ship Sensor Scans"), this, SLOT(ServerDisableShipScan()));
	disableMenu.addAction(tr("Station Sensor Scans"), this, SLOT(ServerDisableStationScan()));
	disableMenu.addSeparator();
	disableMenu.addAction(tr("Movement"), this, SLOT(ServerDisableMovement()));
	disableMenu.addAction(tr("SetModified Signal"), this, SLOT(ServerDisableSetModified()));
	disableMenu.addAction(tr("Tock Transactions Logger"), this, SLOT(ServerDisableTockTransactions()));

	// Monitor intervals submenu.
	QMenu monitorMenu(tr("Start Monitoring"));
	monitorMenu.addAction(tr("60 second intervals"), this, SLOT(Start60Sec()));
	monitorMenu.addAction(tr("30 second intervals"), this, SLOT(Start30Sec()));
	monitorMenu.addAction(tr("15 second intervals"), this, SLOT(Start15Sec()));
	monitorMenu.addAction(tr("10 second intervals"), this, SLOT(Start10Sec()));
	monitorMenu.addAction(tr("5 second intervals"), this, SLOT(Start5Sec()));
	monitorMenu.addAction(tr("3 second intervals"), this, SLOT(Start3Sec()));
	monitorMenu.addAction(tr("2 second intervals"), this, SLOT(Start2Sec()));
	monitorMenu.addAction(tr("1 second intervals"), this, SLOT(Start1Sec()));

	// Main menu.
	QMenu menu(this);
	menu.addMenu(&monitorMenu);
	menu.addAction(tr("Stop Monitoring"), this, SLOT(StopMonitoring()));
	menu.addAction(tr("Probe %1 Servers").arg(rows.count()), this, SLOT(ProbeServers()));
	menu.addAction(tr("Disconnect"), this, SLOT(Disconnect()));
	menu.addSeparator();
	menu.addAction(tr("Log Messages..."), this, SLOT(LogMessages()));
	menu.addSeparator();
	menu.addAction(tr("Report Memory Usage"), this, SLOT(ReportMemory()));
	menu.addAction(tr("Report Resource Usage"), this, SLOT(ReportResources()));
	menu.addAction(tr("Report Resource Usage Of Atom..."), this, SLOT(ReportAtom()));
	menu.addAction(tr("Find Unit Id..."), this, SLOT(FindId()));
	menu.addAction(tr("Find Unit Name..."), this, SLOT(FindName()));
	menu.addSeparator();
	menu.addMenu(&enableMenu);
	menu.addMenu(&disableMenu);
	menu.addMenu(&storyMenu);
	menu.addMenu(&tickMenu);
	menu.addMenu(&tockMenu);
	menu.addSeparator();
	menu.addAction(tr("Create Sector At..."), this, SLOT(CreateSectorAt()));
	menu.addAction(tr("Finalize Sector/Solar System..."), this, SLOT(FinalizeSectorOrSystem()));
	menu.addAction(tr("Reassign Solar System..."), this, SLOT(ReassignSystem()));
	menu.addAction(tr("Revoke Sector/Solar System Assignment..."), this, SLOT(RevokeSectorOrSystem()));
	menu.addSeparator();
	menu.addAction(tr("Restore Empire..."), this, SLOT(RestoreEmpire()));
	menu.addAction(tr("Limbo Rescue Atom..."), this, SLOT(LimboRescue()));
	menu.addAction(tr("Limbo Rescue Atom List..."), this, SLOT(LimboRescueList()));
	//menu.addSeparator();
	//menu.addAction(tr("Start Monitoring All Servers"), this, SLOT(StartMonitoringAll()));
	//menu.addAction(tr("Stop Monitoring All Servers"), this, SLOT(StopMonitoringAll()));
	//menu.addAction(tr("Disconnect All Servers"), this, SIGNAL(DisconnectAll()));
	menu.addSeparator();
	menu.addAction(tr("Reload Static Data"), this, SLOT(ReloadStatic()));
	menu.addAction(tr("Set Atom Modified..."), this, SLOT(SetAtomModified()));
	menu.addAction(tr("Set Control Decay..."), this, SLOT(SetControlDecay()));
	menu.addAction(tr("Set UDP Packet Decay..."), this, SLOT(SetUDPDecay()));
	menu.addSeparator();
	menu.addAction(tr("Server Announcement..."), this, SLOT(ServerBroadcast()));
	menu.addAction(tr("Shutdown Server..."), this, SLOT(ShutdownServer()));
	menu.addAction(tr("Halt Server Shutdown..."), this, SLOT(HaltShutdown()));

	menu.exec(QCursor::pos());
}

void AuServerCon::TimerUpdate()
{
	if(!m_GdLoaded)
	{
		//qDebug() << "AuServerCon::TimerUpdate loading game data";
		UniverseDbQuery(Q_GameData, AuId(0), "");
		//qDebug() << "AuServerCon::TimerUpdate game data loaded";
	}

	m_PingIcon = (m_PingIcon + 1)%s_PingIconCount;

	// Get info about the item that is selected.
	QListWidgetItem* item = m_ConnectionsList->currentItem();
	if(item)
	{
		QString netName = item->data(Qt::UserRole).toString();
		quint16 netPort = item->data(Qt::UserRole+7).toUInt();

		m_ConnectionsLock.lock();
		for(QList<DtMgrClient*>::const_iterator cit = m_Connections.constBegin(); cit != m_Connections.constEnd(); cit++)
		{
			if(!(*cit)->IsConnected())
				continue;
			if((*cit)->ConnectionName() == netName && (*cit)->ConnectionPort() == netPort)
			{
				// Found the selected server and it is still running.
				switch((AuGlobal::MODE)(*cit)->ServerMode())
				{
				case AuGlobal::SceneWorker:
					if(m_Report)
					{
						const QList<AuStatusReport>& reports = (*cit)->LockStatusReports();
						m_Report->SetStatus((*cit)->ConnectionName(), (*cit)->ConnectionPort(), reports);
						(*cit)->UnlockStatusReports();
					}
					break;
				case AuGlobal::SceneMaster:
					if(m_PlayersOn)
					{
						const AuPlayerOnlineStatusVector& users = (*cit)->LockPlayersOn();
						int result = m_PlayersOn->Update(users, true);
						int playerCount = m_PlayersOn->AvatarsOnline();
						(*cit)->UnlockPlayersOn();
						if(m_PlayerCount != playerCount)
						{
							m_PlayerCount = playerCount;
							if(playerCount == 0)
								m_PlayersInfo->setText(tr("No Avatars Online"));
							else
								m_PlayersInfo->setText(tr("%L1 Avatars Online").arg(playerCount));
						}
						if(result == 2)
							ReloadGameData();
					}
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
					m_PlayersOn->hide();
					m_Report->hide();
					break;
				}
			}
		}
		m_ConnectionsLock.unlock();
	}

	// Change color of polling entries that are taking too long.
	QSet<int> alert;	// row indexes of connections that are alerting.
	bool onFire = false;
	for(int row=0; row<m_ConnectionsList->count(); row++)
	{
		QListWidgetItem* item = m_ConnectionsList->item(row);
		if(item)
		{
			QDateTime started = item->data(Qt::UserRole+1).toDateTime();
			int interval = item->data(Qt::UserRole+2).toUInt();
			if(!started.isNull() && interval)
			{
				int elapsed = started.msecsTo(QDateTime::currentDateTime());
				if(elapsed < interval*1000+5000)
				{
					// Color according to atom tick time.
					unsigned int avgCycleMs = item->data(Qt::UserRole+3).toUInt();
					float atomLoad = item->data(Qt::UserRole+4).toFloat();
					int etheriumLock = item->data(Qt::UserRole+6).toInt();
					unsigned int systemsDeadlocked = item->data(Qt::UserRole+9).toInt();
					bool bdLock = item->data(Qt::UserRole+11).toBool();
					bool dataLock = item->data(Qt::UserRole+12).toBool();
					bool itLock = item->data(Qt::UserRole+13).toBool();
					bool scLock = item->data(Qt::UserRole+14).toBool();
					bool trashLock = item->data(Qt::UserRole+15).toBool();
					item->setForeground(AuAtomManager::Color(atomLoad, etheriumLock, avgCycleMs, systemsDeadlocked, bdLock,
								dataLock, itLock, scLock, trashLock));
					item->setIcon(QIcon(QPixmap(PingIcon(started.time().msec())).scaled(d_IconSize,d_IconSize, Qt::KeepAspectRatio,
									Qt::SmoothTransformation)));

					//if(systemsDeadlocked)
						//alert.insert(row);
				}
				else if(elapsed < interval*1000+10000)
				{
					item->setForeground(QColor(230,230,0));
					item->setIcon(QIcon(QPixmap(":data/SlowReply.png").scaled(d_IconSize,d_IconSize, Qt::KeepAspectRatio,
									Qt::SmoothTransformation)));
				}
				else
				{
					item->setForeground(QColor(230,0,0));
					item->setIcon(FireIcon());
					alert.insert(row);
					onFire = true;
				}
			}
		}
	}

	// Determine what sounds to play, if any.
	bool siren = false;
	if(!alert.empty())
	{
		QHash<int, AuElapsedTimer> oldAlert(m_Alert);
		m_Alert.clear();

		for(QSet<int>::const_iterator ai = alert.cbegin(); ai != alert.cend(); ai++)
		{
			QHash<int, AuElapsedTimer>::iterator oi = oldAlert.find(*ai);
			if(oi == oldAlert.end())
			{
				AuElapsedTimer now;
				now.start();
				m_Alert.insert(*ai, now);	// Init start time on rows that were not alert.
			}
			else
			{
				m_Alert.insert(*ai, *oi);			// Preserve start time on rows that were already alert.
				if((*oi).elapsed()/d_MSecPerSec > d_AlertSecs)
					siren = true;					// This one has been alerting long enough to siren.
				oldAlert.erase(oi);
			}
		}
	}
	else
		m_Alert.clear();

	// Reduce volume when my window is visible, loud when minimized.
	float gain = 1;
	QTime now = QTime::currentTime();
	QTime dayStart(9,0,0);
	QTime dayEnd(21,0,0);
	if(now < dayStart || now > dayEnd)
	{
		// Remain silent while we're not working.
		onFire = false;
		siren = false;
	}
	else
	{
		// Time is during working hours, not while we're asleep.
		if(!auServerMonApp->MainWindow()->isMinimized())
		{
			// Main window is visible. Turn down the blast in the face.
			//gain = .25f;
			siren = false;
		}
		else if(!siren)
			onFire = false;	// No fire when minimized and no siren.
	}

	// Play the appropriate sounds.
	SOUND_EFFECT soundFire = SOUND_CracklingFire;
	if(onFire)
	{
		if(!auServerMonApp->IsClientSoundPlaying(soundFire))
			auServerMonApp->PlayClientSound(soundFire, gain);
	}
	else
	{
		if(auServerMonApp->IsClientSoundPlaying(soundFire))
			auServerMonApp->StopClientSound(soundFire);
	}

	SOUND_EFFECT soundSiren = SOUND_AlertRed;
	if(siren)
	{
		if(!auServerMonApp->IsClientSoundPlaying(soundSiren))
			auServerMonApp->PlayClientSound(soundSiren, gain);
	}
	else
	{
		if(auServerMonApp->IsClientSoundPlaying(soundSiren))
			auServerMonApp->StopClientSound(soundSiren);
	}
}

void AuServerCon::ConnectGrid()
{
	int server = 0;
	for(QList<AuServerChoice>::const_iterator si = m_PublicServers.cbegin(); si != m_PublicServers.cend(); si++, server++)
		ConnectToServer((*si).ServerMode(), (*si).ListName(), (*si).NetName(), (*si).NetPort(), server, server*50);
}

void AuServerCon::ConnectSceneWorkers()
{
	int server = 0;
	for(QList<AuServerChoice>::const_iterator si = m_PublicServers.cbegin(); si != m_PublicServers.cend(); si++, server++)
	{
		if((*si).ServerMode() != AuGlobal::SceneWorker)
			continue;	// Skip non-workers
		ConnectToServer((*si).ServerMode(), (*si).ListName(), (*si).NetName(), (*si).NetPort(), server, server*50);
	}
}

void AuServerCon::ConnectDebugGrid()
{
	int server = m_PublicServers.size();
	for(QList<AuServerChoice>::const_iterator si = m_DebugServers.cbegin(); si != m_DebugServers.cend(); si++, server++)
		ConnectToServer((*si).ServerMode(), (*si).ListName(), (*si).NetName(), (*si).NetPort(), server, server*50);
}

void AuServerCon::ConnectDebugSceneWorkers()
{
	int server = m_PublicServers.size();
	for(QList<AuServerChoice>::const_iterator si = m_DebugServers.cbegin(); si != m_DebugServers.cend(); si++, server++)
	{
		if((*si).ServerMode() != AuGlobal::SceneWorker)
			continue;	// Skip non-workers
		ConnectToServer((*si).ServerMode(), (*si).ListName(), (*si).NetName(), (*si).NetPort(), server, server*50);
	}
}

void AuServerCon::DisconnectAll()
{
	emit Disconnect("", 0);
}

/*
void AuServerCon::StartMonitoringAll()
{
	QString pollInterval = QInputDialog::getText(this, auServerMonApp->Name(),
			"Enter seconds between polls.", QLineEdit::Normal, "1");
	int interval = QVariant(pollInterval).toUInt();
	if(!interval)
		interval = 1;

	// Start polling all connections.
	emit StartPolling(interval*1000, "", 0);

	// Update text on connections in list.
	for(int row=0; row<m_ConnectionsList->count(); row++)
	{
		QListWidgetItem* item = m_ConnectionsList->item(row);
		if(item)
		{
			QString listName = item->text();
			int monIdx = listName.indexOf(" (Monitoring");
			bool isMonitoring = (monIdx >= 0);
			if(!isMonitoring)
			{
				listName += QString(" (Monitoring %L1sec)").arg(interval);
				item->setText(listName);
			}
		}
	}
}

void AuServerCon::StopMonitoringAll()
{
	// Stop polling all connections.
	emit StopPolling("", 0);

	// Update text on connections in list.
	for(int row=0; row<m_ConnectionsList->count(); row++)
	{
		QListWidgetItem* item = m_ConnectionsList->item(row);
		if(item)
		{
			QString listName = item->text();
			int monIdx = listName.indexOf(" (Monitoring");
			bool isMonitoring = (monIdx >= 0);
			if(isMonitoring)
			{
				listName = listName.left(monIdx);
				item->setText(listName);
			}
		}
	}
}
*/

void AuServerCon::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);

	// History can be limited to half the frame width because the histograms are side-by-side.
	int maxHistory = event->size().width()/2;
	emit SetMaxHistory(maxHistory);
}

void AuServerCon::ResourceReportAtom(AuResourceReportAtom report)
{
	QString listName;
	for(int row=0; row<m_ConnectionsList->count(); row++)
	{
		QListWidgetItem* item = m_ConnectionsList->item(row);
		if(item->data(Qt::UserRole).toString() == report.m_ServerName && item->data(Qt::UserRole+7).toUInt() == report.m_ServerPort)
			listName = item->text();
	}

	QString rpt("<html>");

	rpt += "<b>Resource Usage</b><div style=\"margin-left: 10px;\">";
	rpt += QString("%1<br>%2<br>Port %3").arg(listName).arg(report.m_ServerName).arg(report.m_ServerPort);
	rpt += "</div><br>";

	if(report.m_AtomType)
	{
		rpt += "<b>Atom Requested</b><div style=\"margin-left: 10px;\">";
		rpt += QString("%1 %2<br>").arg(report.m_AtomId).arg(report.m_Name);
		rpt += QString("Atom Type: %1<br>").arg(report.m_AtomType);
		rpt += QString("Description: %1<br>").arg(report.m_Desc);
		rpt += QString("Spot Description: %1<br>").arg(report.m_SpotDesc);
		rpt += QString("Time Created: %1<br>").arg(report.m_TimeCreated.toString());
		rpt += QString("Time Modified: %1<br>").arg(report.m_TimeModified.toString());
		rpt += QString("Time Ticked: %1<br>").arg(report.m_TimeTicked.toString());
		rpt += QString("Time Tocked: %1<br>").arg(report.m_TimeTocked.toString());
		rpt += QString("In Scene: %1, Last Scene %2<br>").arg(report.m_InScene).arg(report.m_TimeScene.toString());
		rpt += QString("Movement: At Rest %1, Vector (%2, %3, %4)")
			.arg(report.m_AtRest).arg(report.m_VecX).arg(report.m_VecY).arg(report.m_VecZ);
		rpt += "</div><br>";

		rpt += ResourceReportCommon(report);
	}
	else
	{
		// The server did not find the atom.
		rpt += "<b>Atom Requested</b><div style=\"margin-left: 10px;\">";
		rpt += QString("%1 not found on server.").arg(report.m_AtomId);
		rpt += "</div><br>";
	}

	rpt += "</html>";

	AuReportDialog* reportDlg = new AuReportDialog(rpt, this);
	reportDlg->show();
}

void AuServerCon::MemoryUsageReport(QString serverName, quint16 serverPort, QStringList report)
{
	QString listName;
	for(int row=0; row<m_ConnectionsList->count(); row++)
	{
		QListWidgetItem* item = m_ConnectionsList->item(row);
		if(item->data(Qt::UserRole).toString() == serverName && item->data(Qt::UserRole+7).toUInt() == serverPort)
			listName = item->text();
	}

	QString rpt("<html>");

	rpt += "<b>Resource Usage</b><div style=\"margin-left: 10px;\">";
	rpt += QString("%1<br>%2<br>Port %3").arg(listName).arg(serverName).arg(serverPort);
	rpt += "</div><br>";

	for(QStringList::const_iterator si = report.cbegin(); si != report.cend(); si++)
	{
		if((*si).left(1) == ' ')
			rpt += "&nbsp;";
		rpt += *si + "<br>";
	}

	rpt += "</html>";

	AuReportDialog* reportDlg = new AuReportDialog(rpt, this);
	reportDlg->show();
}

void AuServerCon::ResourceReport(AuResourceReport report)
{
	QString listName;
	for(int row=0; row<m_ConnectionsList->count(); row++)
	{
		QListWidgetItem* item = m_ConnectionsList->item(row);
		if(item->data(Qt::UserRole).toString() == report.m_ServerName && item->data(Qt::UserRole+7).toUInt() == report.m_ServerPort)
			listName = item->text();
	}

	QString rpt("<html>");

	rpt += "<b>Resource Usage</b><div style=\"margin-left: 10px;\">";
	rpt += QString("%1<br>%2<br>Port %3").arg(listName).arg(report.m_ServerName).arg(report.m_ServerPort);
	rpt += "</div><br>";

	rpt += ResourceReportCommon(report);

	rpt += "</html>";

	AuReportDialog* reportDlg = new AuReportDialog(rpt, this);
	reportDlg->show();
}

QString AuServerCon::ResourceReportCommon(const AuResourceReport& report)
{
	QString rpt;
	rpt += "<b>Atoms</b><div style=\"margin-left: 10px;\">";
	rpt += "<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" style=\"background-color: rgb(0, 0, 0);\">";
	rpt += "<tbody>";
	rpt += QString("<tr><td></td><td><b>Root</b></td><td><b>Trash</b></td></tr>");
	rpt += QString("<tr><td>Total</td><td>%L1</td><td>%L2</td></tr>").arg(report.TotalAtoms()).arg(report.TotalTrash());
	rpt += QString("<tr><td>Dead</td><td>%L1</td><td>%L2</td></tr>").arg(report.m_dead).arg(report.m_deadTrash);
	rpt += QString("<tr><td>Dying</td><td>%L1</td><td>%L2</td></tr>").arg(report.m_dying).arg(report.m_dyingTrash);
	rpt += QString("<tr><td>Traveling</td><td>%L1</td><td>%L2</td></tr>").arg(report.m_travel).arg(report.m_travelTrash);

	for(int a=0; a < DaAtom::ATOM_Count; a++)
	{
		DaAtom::ATOM_TYPE atomType = (DaAtom::ATOM_TYPE)a;
		QHash<DaAtom::ATOM_TYPE, quint32>::const_iterator ai = report.m_Atoms.find(atomType);
		QHash<DaAtom::ATOM_TYPE, quint32>::const_iterator ti = report.m_Trash.find(atomType);
		if(ai == report.m_Atoms.cend() && ti == report.m_Trash.cend())
			continue;	// None of that kind of atom.
		if(ti == report.m_Trash.cend())
			rpt += QString("<tr><td>%1</td><td>%L2</td><td></td></tr>")
				.arg(DaAtom::AtomTypeName(atomType))
				.arg(*ai);
		else if(ai == report.m_Atoms.cend())
			rpt += QString("<tr><td>%1</td><td></td><td>%L2</td></tr>")
				.arg(DaAtom::AtomTypeName(atomType))
				.arg(*ti);
		else
			rpt += QString("<tr><td>%1</td><td>%L2</td><td>%L3</td></tr>")
				.arg(DaAtom::AtomTypeName(atomType))
				.arg(*ai)
				.arg(*ti);
	}

	rpt += "</tbody>";
	rpt += "</table>";
	rpt += "</div><br>";

	rpt += QString("<b>%L1 People Online</b><div style=\"margin-left: 10px;\">").arg(report.m_peopleOnLine.count());
	rpt += "<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" style=\"background-color: rgb(0, 0, 0);\">";
	rpt += "<tbody>";
	for(QStringList::const_iterator aimi = report.m_peopleOnLine.constBegin(); aimi != report.m_peopleOnLine.constEnd(); aimi++)
	{
		QStringList fields = (*aimi).split('|');
		rpt += "<tr>";
		for(QStringList::const_iterator fi = fields.constBegin(); fi != fields.constEnd(); fi++)
			rpt += QString("<td>%1</td>").arg(*fi);
		rpt += "</tr>";
	}
	rpt += "</tbody>";
	rpt += "</table>";
	rpt += "</div><br>";

	rpt += QString("<b>%1 Atoms In Motion</b><div style=\"margin-left: 10px;\">").arg(report.m_atomsInMotion.count());
	rpt += "<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" style=\"background-color: rgb(0, 0, 0);\">";
	rpt += "<tbody>";
	for(QStringList::const_iterator aimi = report.m_atomsInMotion.constBegin(); aimi != report.m_atomsInMotion.constEnd(); aimi++)
	{
		QStringList fields = (*aimi).split('|');
		rpt += "<tr>";
		for(QStringList::const_iterator fi = fields.constBegin(); fi != fields.constEnd(); fi++)
			rpt += QString("<td>%1</td>").arg(*fi);
		rpt += "</tr>";
	}
	rpt += "</tbody>";
	rpt += "</table>";
	rpt += "</div><br>";

	{
		rpt += QString("<b>%L1 Tickers</b><div style=\"margin-left: 10px;\">").arg(report.m_tickers.count());
		rpt += "<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" style=\"background-color: rgb(0, 0, 0);\">";
		rpt += "<tbody>";
		rpt += QString("<tr><td><b>Ticker</b></td><td><b>Average Tick</b></td><td><b>Max Tick</b></td></tr>");

		for(QList<AuThinkerReport>::const_iterator mti = report.m_tickers.constBegin();
				mti != report.m_tickers.constEnd(); mti++)
		{
			QString avg;
			if((*mti).avgThinkTime > 4)
				avg = QString("<big><b>%1</b></big>").arg((float)(*mti).avgThinkTime/2.f);
			else if((*mti).avgThinkTime > 2)
				avg = QString("<b>%1</b>").arg((float)(*mti).avgThinkTime/2.f);
			else if((*mti).avgThinkTime <= 1)
				avg = QString("<small>%1</small>").arg((float)(*mti).avgThinkTime/2.f);
			else
				avg = QString("%1").arg((float)(*mti).avgThinkTime/2.f);

			QString max;
			if((*mti).maxThinkTime > 20)
				max = QString("<big><b>%1</b></big>").arg((float)(*mti).maxThinkTime/2.f);
			else if((*mti).maxThinkTime > 10)
				max = QString("<b>%1</b>").arg((float)(*mti).maxThinkTime/2.f);
			else if((*mti).maxThinkTime <= 2)
				max = QString("<small>%1</small>").arg((float)(*mti).maxThinkTime/2.f);
			else
				max = QString("%1").arg((float)(*mti).maxThinkTime/2.f);

			rpt += QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>")
				.arg((*mti).atomId)
				.arg(avg)
				.arg(max);
		}

		rpt += "</tbody>";
		rpt += "</table>";
		rpt += "</div><br>";
	}

	{
		rpt += QString("<b>%L1 Tockers</b><div style=\"margin-left: 10px;\">").arg(report.m_tockers.count());

		bool anyToReport = false;
		for(QList<AuThinkerReport>::const_iterator mti = report.m_tockers.constBegin();
				mti != report.m_tockers.constEnd(); mti++)
		{
			if((*mti).avgThinkTime || (*mti).maxThinkTime)
			{
				anyToReport = true;
				break;
			}
		}

		if(anyToReport)
		{
			// Sort the report data by max tock time.
			QMultiMap<quint32, QList<AuThinkerReport>::const_iterator> sorted;
			for(QList<AuThinkerReport>::const_iterator mti = report.m_tockers.constBegin();
					mti != report.m_tockers.constEnd(); mti++)
			{
				if(!(*mti).avgThinkTime && !(*mti).maxThinkTime)
					continue;
				sorted.insert((*mti).maxThinkTime, mti);
			}

			rpt += "<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" style=\"background-color: rgb(0, 0, 0);\">";
			rpt += "<tbody>";
			rpt += QString("<tr><td><b>Tocker</b></td><td><b>Average Tock</b></td><td><b>Max Tock</b></td></tr>");
			for(QMultiMap<quint32, QList<AuThinkerReport>::const_iterator>::const_iterator si = sorted.cend()-1;
					si != sorted.cbegin()-1; si--)
			{
				QList<AuThinkerReport>::const_iterator mti = *si;

				QString avg;
				if((*mti).avgThinkTime > 4)
					avg = QString("<big><b>%1</b></big>").arg((float)(*mti).avgThinkTime/2.f);
				else if((*mti).avgThinkTime > 2)
					avg = QString("<b>%1</b>").arg((float)(*mti).avgThinkTime/2.f);
				else if((*mti).avgThinkTime <= 1)
					avg = QString("<small>%1</small>").arg((float)(*mti).avgThinkTime/2.f);
				else
					avg = QString("%1").arg((float)(*mti).avgThinkTime/2.f);

				QString max;
				if((*mti).maxThinkTime > 20)
					max = QString("<big><b>%1</b></big>").arg((float)(*mti).maxThinkTime/2.f);
				else if((*mti).maxThinkTime > 10)
					max = QString("<b>%1</b>").arg((float)(*mti).maxThinkTime/2.f);
				else if((*mti).maxThinkTime <= 2)
					max = QString("<small>%1</small>").arg((float)(*mti).maxThinkTime/2.f);
				else
					max = QString("%1").arg((float)(*mti).maxThinkTime/2.f);

				rpt += QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>")
					.arg((*mti).atomId)
					.arg(avg)
					.arg(max);
			}
			rpt += "</tbody>";
			rpt += "</table>";
		}
		else
		{
			rpt += "<p>No measurable Tock times.</p>";
		}

		rpt += "</div><br>";
	}

	if(report.m_hasCache)
	{
		rpt += QString("<b>%1 Models In Cache</b><div style=\"margin-left: 10px;\">").arg(report.m_modelCache.count());
		rpt += "<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" style=\"background-color: rgb(0, 0, 0);\">";
		rpt += "<tbody>";
		rpt += QString("<tr><td><b>File Name</b></td><td><b>Users</b></td><td><b>Building<br>Etc</b></td><td><b>Building<br>Inside<br>Level 1</b></td><td><b>Building<br>Inside<br>Level 2</b></td><td><b>Building<br>Obstacles<br>Level 1</b></td><td><b>Building<br>Obstacles<br>Level 2</b></td><td><b>Building<br>Ramp</b></td><td><b>Building<br>Weapon</b></td><td><b>Screens</b></td><td><b>Stations</b></td><td><b>Weapons</b></td><td><b>Materials</b></td><td><b>Objects</b></td><td><b>Vertex Groups</b></td></tr>");
		for(QList<AuResourceModel>::const_iterator mci = report.m_modelCache.constBegin();
				mci != report.m_modelCache.constEnd(); mci++)
			rpt += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td><td>%7</td><td>%8</td><td>%9</td><td>%10</td><td>%11</td><td>%12</td><td>%13</td><td>%14</td><td>%15</td></tr>")
				.arg((*mci).m_FileName)
				.arg((*mci).m_Users)
				.arg((*mci).m_BuildingEtc?"Y":"")
				.arg((*mci).m_BuildingIns1?"Y":"")
				.arg((*mci).m_BuildingIns2?"Y":"")
				.arg((*mci).m_BuildingObs1?"Y":"")
				.arg((*mci).m_BuildingObs2?"Y":"")
				.arg((*mci).m_BuildingRamp?"Y":"")
				.arg((*mci).m_BuildingWpn?"Y":"")
				.arg((*mci).m_Screens?"Y":"")
				.arg((*mci).m_Stations?"Y":"")
				.arg((*mci).m_Weapons?"Y":"")
				.arg((*mci).m_Materials)
				.arg((*mci).m_Objects)
				.arg((*mci).m_VertexGroups);
		rpt += "</tbody>";
		rpt += "</table>";
		rpt += "</div><br>";
	}

	if(!report.m_BdDesigns.empty())
	{
		rpt += QString("<b>%1 Building Models In Bd Cache</b><div style=\"margin-left: 10px;\">").arg(report.m_BdDesigns.count());
		rpt += "<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" style=\"background-color: rgb(0, 0, 0);\">";
		rpt += "<tbody>";
		rpt += QString("<tr><td><b>Design Id</b></td>"
				"<td><b>Design Name</b></td>"
				"<td><b>Design State</b></td>"
				"<td><b>Design Triangles</b></td>"
				"</tr>");
		for(QList<AuResourceDesign>::const_iterator di = report.m_BdDesigns.constBegin(); di != report.m_BdDesigns.constEnd(); di++)
			rpt += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>")
				.arg((*di).m_Id)
				.arg((*di).m_Name)
				.arg((*di).StateName())
				.arg((*di).m_Triangles);
		rpt += "</tbody>";
		rpt += "</table>";
		rpt += "</div><br>";
	}

	if(!report.m_ScDesigns.empty())
	{
		rpt += QString("<b>%1 Spacecraft Models In Sc Cache</b><div style=\"margin-left: 10px;\">").arg(report.m_ScDesigns.count());
		rpt += "<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" style=\"background-color: rgb(0, 0, 0);\">";
		rpt += "<tbody>";
		rpt += QString("<tr><td><b>Design Id</b></td>"
				"<td><b>Design Name</b></td>"
				"<td><b>Design State</b></td>"
				"<td><b>Design Triangles</b></td>"
				"</tr>");
		for(QList<AuResourceDesign>::const_iterator di = report.m_ScDesigns.constBegin(); di != report.m_ScDesigns.constEnd(); di++)
			rpt += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>")
				.arg((*di).m_Id)
				.arg((*di).m_Name)
				.arg((*di).StateName())
				.arg((*di).m_Triangles);
		rpt += "</tbody>";
		rpt += "</table>";
		rpt += "</div><br>";
	}
	return rpt;
}

QString AuServerCon::MakeServerTitle(const QString& curTitle, const QString& suffix) const
{
	// Build a server title.
	QString newTitle(curTitle);

	// Remove any previous suffix, which always begins with " (".
	int suffixIdx = newTitle.indexOf(" (");
	if(suffixIdx >= 0)
		newTitle = newTitle.left(suffixIdx);
	//suffixIdx = newTitle.indexOf(tr(" 🩺"));
	suffixIdx = newTitle.indexOf(tr(" ⚙"));
	if(suffixIdx >= 0)
		newTitle = newTitle.left(suffixIdx);
	suffixIdx = newTitle.indexOf(tr(" 🌡️"));
	if(suffixIdx >= 0)
		newTitle = newTitle.left(suffixIdx);

	// Add the new suffix, if any.
	newTitle += suffix;

	// Return the final result.
	return newTitle;
}

QListWidgetItem* AuServerCon::Update(QString serverName, quint16 serverPort, unsigned int avgCycleMs, float atomLoad,
		int etheriumLock, unsigned int systemsDeadlocked, bool bdLock, bool dataLock, bool itLock, bool scLock, bool trashLock)
{
	for(int row=0; row<m_ConnectionsList->count(); row++)
	{
		QListWidgetItem* item = m_ConnectionsList->item(row);
		if(item->data(Qt::UserRole).toString() == serverName && item->data(Qt::UserRole+7).toUInt() == serverPort)
		{
			int doingWhat = item->data(Qt::UserRole+10).toInt();	// 0=idle, 1=monitoring, 2=probing, 3=searching
			int secs = item->data(Qt::UserRole+2).toUInt();
			QColor color(AuAtomManager::Color(atomLoad, etheriumLock, avgCycleMs, systemsDeadlocked, bdLock, dataLock, itLock,
						scLock, trashLock));

			QString suffix;
			switch(doingWhat)
			{
			case 0:
				// No suffix.
				break;
			case 1:	// Monitoring
				suffix = tr(" (%1sec)").arg(secs);
				[[fallthrough]];
			case 2:	// Probing
				if(atomLoad)
				{
					if(color.green() != 255)
						suffix += tr(" 🌡️%1")
							.arg(atomLoad*100, 0,'f',0);
					else
						//suffix += tr(" 🩺 %1")
						suffix += tr(" ⚙ %1")
							.arg(atomLoad*100, 0,'f',0);
				}

				if(avgCycleMs)
					suffix += tr(" ⏱%1ms")
						.arg((avgCycleMs+1)/2);

				if(etheriumLock > 0)
					suffix += tr(" 🔓%2ms").arg(etheriumLock);
				else if(etheriumLock < 0)
					suffix += tr(" 🔒Locked");

				if(systemsDeadlocked == 1)
					suffix += tr(" ⚛ Busy");
				else if(systemsDeadlocked)
					suffix += tr(" ⚛ %1 Busy").arg(systemsDeadlocked);

				if(dataLock)
					suffix += tr(" 🌎");	//tr(" 🏴");
				if(bdLock)
					suffix += tr(" 🏠");
				if(itLock)
					suffix += tr(" 🕹");	//tr(" 🔨");
				if(scLock)
					suffix += tr(" 🟊");	//tr(" 🚀");
				if(trashLock)
					suffix += tr(" 🗑️");
				break;
			case 3:	// Searching
				break;
			}

			QString curTitle = item->text();
			item->setText(MakeServerTitle(curTitle, suffix));
			item->setData(Qt::UserRole+1, QDateTime::currentDateTime());
			item->setData(Qt::UserRole+3, avgCycleMs);
			item->setData(Qt::UserRole+4, atomLoad);
			item->setData(Qt::UserRole+6, etheriumLock);
			item->setData(Qt::UserRole+9, systemsDeadlocked);
			item->setData(Qt::UserRole+11, bdLock);
			item->setData(Qt::UserRole+12, dataLock);
			item->setData(Qt::UserRole+13, itLock);
			item->setData(Qt::UserRole+14, scLock);
			item->setData(Qt::UserRole+15, trashLock);
			item->setForeground(color);
			return item;
		}
	}
	return 0;
}

void AuServerCon::MonitorAnswered(QString serverName, quint16 serverPort, unsigned int avgCycleMs, float atomLoad, int etheriumLock,
		unsigned int systemsDeadlocked, bool bdLock, bool dataLock, bool itLock, bool scLock, bool trashLock)
{
	Update(serverName, serverPort, avgCycleMs, atomLoad, etheriumLock, systemsDeadlocked, bdLock, dataLock, itLock, scLock,
			trashLock);
}

void AuServerCon::ProbeAnswered(QString serverName, quint16 serverPort, unsigned int avgCycleMs, float atomLoad, int etheriumLock,
		unsigned int systemsDeadlocked, bool bdLock, bool dataLock, bool itLock, bool scLock, bool trashLock)
{
	QListWidgetItem* item = Update(serverName, serverPort, avgCycleMs, atomLoad, etheriumLock, systemsDeadlocked, bdLock, dataLock,
			itLock, scLock, trashLock);
	if(item)
	{
		item->setIcon(QIcon(QPixmap(":data/Server.png").scaled(d_IconSize,d_IconSize, Qt::KeepAspectRatio,
						Qt::SmoothTransformation)));
		item->setData(Qt::UserRole+2, 0);
	}
}

void AuServerCon::UnitSearchResult(QString serverName, quint16 serverPort, quint32 unitId, QString unitName)
{
	// Only leave a suffix on the one that finds the unit. Others will lose their suffix.
	QString suffix;
	if(unitId)
		suffix = QString(" (Found %1 %2)").arg(unitId).arg(unitName);

	QPalette pal;
	for(int row=0; row<m_ConnectionsList->count(); row++)
	{
		QListWidgetItem* item = m_ConnectionsList->item(row);
		if(item->data(Qt::UserRole).toString() == serverName && item->data(Qt::UserRole+7).toUInt() == serverPort)
		{
			QString curTitle = item->text();
			item->setText(MakeServerTitle(curTitle, suffix));
			item->setData(Qt::UserRole+1, QDateTime());
			item->setData(Qt::UserRole+2, 0);
			item->setForeground(pal.text().color());
		}
	}
}

quint32 AuServerCon::AtomId(const QString& atomIdS) const
{
	quint32 atomId = QVariant(atomIdS).toUInt();
	if(!atomId)
	{
		// Try deciphering a string atom id.
		QString upperAtomIdS = atomIdS.toUpper().trimmed();

		// Parse the number from a backwards base 26 (A-Z) number, just like AuId::SetStringId.
		int imag = 1;
		for(int chr=0; chr<upperAtomIdS.count(); chr++)
		{
			int ich = upperAtomIdS[chr].toLatin1() - 'A';
			if(ich)
				atomId += ich * imag;
			imag *= 26;
		}
	}
	return atomId;
}

void AuServerCon::ManageAccounts()
{
	AuAccountManagerDlg mgr(this);
	mgr.exec();
}

QString AuServerCon::PingIcon(unsigned int offset) const
{
	unsigned int pingIcon = (m_PingIcon + offset) % s_PingIconCount;
	return s_PingIcons[pingIcon];
}

void AuServerCon::DeselectAll()
{
	if(m_ConnectionsList->selectionModel())
		m_ConnectionsList->selectionModel()->clear();
}

QIcon AuServerCon::FireIcon()
{
	// Determine the size of our fire image, which is a horizontal strip of flames.
	QSize fsz = m_Fire.size();
	int randwid = fsz.width() - fsz.height();	// Position possibilities for a square that is as wide as the strip is tall.
	int randx = m_Dice.Roll(1,randwid,-1);		// Random position in the allowable movement range.
	QHash<int, QIcon>::const_iterator ii = m_FireIcons.find(randx);	// Look in our cache.
	if(ii != m_FireIcons.cend())
		return *ii;								// Return the icon we already created.

	// Extract a square fire image from the randx position in the fire image strip.
	int sz = fsz.height();
	QPixmap fpix(sz,sz);
	QPainter p(&fpix);
	p.drawImage(QRect(0,0,sz,sz), m_Fire, QRect(randx,0,sz,sz));
	p.end();

	// Create a new icon from the fire image.
	QIcon fireIcon = QIcon(fpix.scaled(d_IconSize,d_IconSize,Qt::KeepAspectRatio,Qt::SmoothTransformation));
	m_FireIcons.insert(randx, fireIcon);		// Save the new fire icon in our cache.
	return fireIcon;
}

QStringList AuServerCon::UniverseDbQuery(UNIVERSE_DB_QUERY q, const AuId& atomId, const QString& name)
{
	//qDebug() << "AuServerCon::UniverseDbQuery" << q;
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	QStringList textLines;
	if(!m_DbFailed)
	{
		QSqlDatabase dbUniverse = QSqlDatabase::addDatabase("QPSQL7", "AuUniverse");
		if(!dbUniverse.isValid())
		{
			textLines << "error: AuServerCon::UniverseDbQuery unable to addDatabase";
			QApplication::restoreOverrideCursor();
			m_DbFailed = true;
			return textLines;
		}

		dbUniverse.setDatabaseName("auUniverse");				// NOTE: Name is case sensitive.
		dbUniverse.setHostName("u1.Hazeron.net");

		if(!dbUniverse.open("aumaster","Light"))
		{
			QSqlError er = dbUniverse.lastError();
			QString sdv = er.driverText();
			QString sdb = er.databaseText();
			textLines << "error: AuServerCon::UniverseDbQuery Failed to open database" << sdv << sdb;
			QApplication::restoreOverrideCursor();
			m_DbFailed = true;
			qDebug() << "error:" << textLines;
			return textLines;
		}
		//qDebug() << "AuServerCon::UniverseDbQuery Universe database opened";

		if(!m_GdLoaded)
		{
			// Load game data.
			DgData& gd = g_Au.LockDataForWrite();
			gd.Load();
			g_Au.UnlockData();
			m_GdLoaded = true;
			//qDebug() << "AuServerCon::UniverseDbQuery basic data loaded";
		}
		//else
			//qDebug() << "AuServerCon::UniverseDbQuery basic data already loaded";

		switch(q)
		{
		case Q_GameData:
			break;
		case Q_LimboCheck:
			textLines = DaAtom::DbLimboCheck(dbUniverse, d_PrefixStep);
			break;
		case Q_AtomId:
			DaAtom::DbFindAtom(textLines, dbUniverse, atomId, true, true, d_PrefixStep);
			break;
		case Q_AvatarName:
			DaAtom::DbFindPerson(textLines, dbUniverse, name, true, true, d_PrefixStep);
			break;
		case Q_SpacecraftName:
			DaAtom::DbFindSpacecraft(textLines, dbUniverse, name, true, true, d_PrefixStep);
			break;
		case Q_SystemId:
			DaAtom::DbQuerySystem(textLines, dbUniverse, atomId, d_PrefixStep);
			break;
		case Q_SystemName:
			DaAtom::DbQuerySystem(textLines, dbUniverse, name, d_PrefixStep);
			break;
		case Q_WorldId:
			DaAtom::DbQueryWorld(textLines, dbUniverse, atomId, d_PrefixStep);
			break;
		case Q_WorldName:
			DaAtom::DbQueryWorld(textLines, dbUniverse, name, d_PrefixStep);
			break;
		}
		//qDebug() << "AuServerCon::UniverseDbQuery query complete";
	}
	QSqlDatabase::removeDatabase("AuUniverse");
	QApplication::restoreOverrideCursor();
	//qDebug() << "AuServerCon::UniverseDbQuery" << q << "done";
	return textLines;
}

void AuServerCon::AvatarQuery()
{
	QString name = QInputDialog::getText(this, auServerMonApp->Name(),
			"Enter name of avatar to report. The search is case sensitive.", QLineEdit::Normal, "");
	if(name.isEmpty())
		return;

	QStringList textLines = UniverseDbQuery(Q_AvatarName, AuId(0), name);
	AuReportDialog* reportDlg = new AuReportDialog(textLines, this);
	reportDlg->show();
}

void AuServerCon::SpacecraftQuery()
{
	QString name = QInputDialog::getText(this, auServerMonApp->Name(),
			"Enter name of spacecraft to report. The search is case sensitive.", QLineEdit::Normal, "");
	if(name.isEmpty())
		return;

	QStringList textLines = UniverseDbQuery(Q_SpacecraftName, AuId(0), name);
	AuReportDialog* reportDlg = new AuReportDialog(textLines, this);
	reportDlg->show();
}

void AuServerCon::AtomQuery()
{
	QString atomIdS = QInputDialog::getText(this, auServerMonApp->Name(),
			"Enter atom id to report. The id can be string or numeric.", QLineEdit::Normal, "");
	AuId atomId(atomIdS);
	if(atomId.IsNull())
		return;

	QStringList textLines = UniverseDbQuery(Q_AtomId, atomId, "");
	AuReportDialog* reportDlg = new AuReportDialog(textLines, this);
	reportDlg->show();
}

void AuServerCon::WorldIdQuery()
{
	QString atomIdS = QInputDialog::getText(this, auServerMonApp->Name(),
			"Enter world id to report. The id can be string or numeric.", QLineEdit::Normal, "");
	AuId atomId(atomIdS);
	if(atomId.IsNull())
		return;

	QStringList textLines = UniverseDbQuery(Q_WorldId, atomId, "");
	AuReportDialog* reportDlg = new AuReportDialog(textLines, this);
	reportDlg->show();
}

void AuServerCon::WorldNameQuery()
{
	QString name = QInputDialog::getText(this, auServerMonApp->Name(),
			"Enter name of world to report. The search is case sensitive.", QLineEdit::Normal, "");
	if(name.isEmpty())
		return;

	QStringList textLines = UniverseDbQuery(Q_WorldName, AuId(0), name);
	AuReportDialog* reportDlg = new AuReportDialog(textLines, this);
	reportDlg->show();
}

void AuServerCon::SystemIdQuery()
{
	QString atomIdS = QInputDialog::getText(this, auServerMonApp->Name(),
			"Enter solar system id to report. The id can be string or numeric.", QLineEdit::Normal, "");
	AuId atomId(atomIdS);
	if(atomId.IsNull())
		return;

	QStringList textLines = UniverseDbQuery(Q_SystemId, atomId, "");
	AuReportDialog* reportDlg = new AuReportDialog(textLines, this);
	reportDlg->show();
}

void AuServerCon::SystemNameQuery()
{
	QString name = QInputDialog::getText(this, auServerMonApp->Name(),
			"Enter name of solar system to report. The search is case sensitive.", QLineEdit::Normal, "");
	if(name.isEmpty())
		return;

	QStringList textLines = UniverseDbQuery(Q_WorldName, AuId(0), name);
	AuReportDialog* reportDlg = new AuReportDialog(textLines, this);
	reportDlg->show();
}

void AuServerCon::LimboCheck()
{
	QStringList textLines = UniverseDbQuery(Q_LimboCheck, AuId(0), "");
	AuReportDialog* reportDlg = new AuReportDialog(textLines, this);
	reportDlg->show();
}

void AuServerCon::ConnectDebugWorker()
{
	qDebug() << "AuServerCon::ConnectDebugWorker";
}

void AuServerCon::ConnectPublicWorker()
{
	qDebug() << "AuServerCon::ConnectPublicWorker";
}

void AuServerCon::ReloadGameData()
{
	//qDebug() << "AuServerCon::ReloadGameData";
	m_GdLoaded = false;
}

void AuServerCon::Servers(QList<int>& rows, QStringList& serverTitles, QStringList& serverNames, QList<quint16>& serverPorts) const
{
	for(int row=0; row<m_ConnectionsList->model()->rowCount(); row++)
	{
		if(m_OperateOnSelectedOnly && !m_ConnectionsList->item(row)->isSelected())
			continue;

		rows.append(row);

		QString listName = m_ConnectionsList->item(row)->text();
		serverTitles.append(listName);

		QString netName = m_ConnectionsList->item(row)->data(Qt::UserRole).toString();
		quint16 netPort = m_ConnectionsList->item(row)->data(Qt::UserRole+7).toUInt();
		serverNames.append(netName);
		serverPorts.append(netPort);
	}
}

void AuServerCon::StartMonitor(int secs)
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	QPalette pal;
	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
	{
		QString listName = MakeServerTitle(serverTitles[selIdx], QString(" (%1sec)").arg(secs));

		QDateTime now = QDateTime::currentDateTime();
		QListWidgetItem* item = m_ConnectionsList->item(rows[selIdx]);
		item->setText(listName);
		item->setData(Qt::UserRole+1, now);
		item->setData(Qt::UserRole+2, secs);
		item->setData(Qt::UserRole+3, 0);
		item->setData(Qt::UserRole+4, 0);
		item->setData(Qt::UserRole+6, 0);
		item->setData(Qt::UserRole+10, 1);	// 1=monitoring
		item->setForeground(pal.text().color());
		item->setIcon(QIcon(QPixmap(PingIcon(0)).scaled(d_IconSize,d_IconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));

		emit StartPolling(secs*1000, serverNames[selIdx], serverPorts[selIdx]);
	}
}

void AuServerCon::Start60Sec()
{
	StartMonitor(60);
}

void AuServerCon::Start30Sec()
{
	StartMonitor(30);
}

void AuServerCon::Start15Sec()
{
	StartMonitor(15);
}

void AuServerCon::Start10Sec()
{
	StartMonitor(10);
}

void AuServerCon::Start5Sec()
{
	StartMonitor(5);
}

void AuServerCon::Start3Sec()
{
	StartMonitor(3);
}

void AuServerCon::Start2Sec()
{
	StartMonitor(2);
}

void AuServerCon::Start1Sec()
{
	StartMonitor(1);
}

void AuServerCon::StopMonitoring()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	QPalette pal;
	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
	{
		QString listName = MakeServerTitle(serverTitles[selIdx], "");

		QListWidgetItem* item = m_ConnectionsList->item(rows[selIdx]);
		item->setText(listName);
		item->setData(Qt::UserRole+1, QDateTime());
		item->setData(Qt::UserRole+2, 0);
		item->setData(Qt::UserRole+3, 0);
		item->setData(Qt::UserRole+4, 0);
		item->setData(Qt::UserRole+6, 0);
		item->setData(Qt::UserRole+10, 0);	// 0=idle
		item->setForeground(pal.text().color());
		item->setIcon(QIcon(QPixmap(":data/Server.png").scaled(d_IconSize,d_IconSize, Qt::KeepAspectRatio,
						Qt::SmoothTransformation)));

		emit StopPolling(serverNames[selIdx], serverPorts[selIdx]);
	}
}

void AuServerCon::ProbeServers()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	QPalette pal;
	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
	{
		QString listName = MakeServerTitle(serverTitles[selIdx], " (Probing)");

		QDateTime now = QDateTime::currentDateTime();
		QListWidgetItem* item = m_ConnectionsList->item(rows[selIdx]);
		item->setText(listName);
		item->setData(Qt::UserRole+1, now);
		item->setData(Qt::UserRole+2, 1);
		item->setData(Qt::UserRole+3, 0);
		item->setData(Qt::UserRole+4, 0);
		item->setData(Qt::UserRole+6, 0);
		item->setData(Qt::UserRole+10, 2);	// 2=probing
		item->setForeground(pal.text().color());
		item->setIcon(QIcon(QPixmap(PingIcon(0)).scaled(d_IconSize,d_IconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));

		emit Probe(serverNames[selIdx], serverPorts[selIdx]);
	}
}

void AuServerCon::Disconnect()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit Disconnect(serverNames[selIdx], serverPorts[selIdx]);
}

void AuServerCon::LogMessages()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	AuDebugMsgBits bitsDlg(this);
	if(bitsDlg.exec() == QDialog::Accepted)
	{
		quint32 debugMsgBits = bitsDlg.DebugMsgBits();
		for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
			emit DebugMessages(serverNames[selIdx], serverPorts[selIdx], debugMsgBits);
	}
}

void AuServerCon::ReportMemory()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit RequestMemoryUsage(serverNames[selIdx], serverPorts[selIdx]);
}

void AuServerCon::ReportResources()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit RequestResourceReport(serverNames[selIdx], serverPorts[selIdx], 1, 0);				// 1=full report
}

void AuServerCon::ReportAtom()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	QString atomIdS = QInputDialog::getText(this, auServerMonApp->Name(),
			"Enter atom id to report.", QLineEdit::Normal, "");
	quint32 atomId = AtomId(atomIdS);
	if(atomId)
	{
		for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
			emit RequestResourceReport(serverNames[selIdx], serverPorts[selIdx], 2, atomId);	// 2=atom report
	}
}

void AuServerCon::FindId()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	QString atomIdS = QInputDialog::getText(this, auServerMonApp->Name(),
			"Enter unit id to find online.", QLineEdit::Normal, "");
	quint32 atomId = AtomId(atomIdS);
	if(atomId)
	{
		QPalette pal;
		for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		{
			QListWidgetItem* item = m_ConnectionsList->item(rows[selIdx]);
			item->setData(Qt::UserRole+1, QDateTime::currentDateTime());
			item->setData(Qt::UserRole+2, 1);
			item->setData(Qt::UserRole+3, 0);
			item->setData(Qt::UserRole+4, 0);
			item->setData(Qt::UserRole+6, 0);
			item->setData(Qt::UserRole+10, 3);	// 3=searching
			item->setForeground(pal.text().color());
			item->setIcon(QIcon(QPixmap(":data/Server.png").scaled(d_IconSize,d_IconSize, Qt::KeepAspectRatio,
							Qt::SmoothTransformation)));

			emit FindUnitId(serverNames[selIdx], serverPorts[selIdx], atomId);
		}
	}
}

void AuServerCon::FindName()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	QString unitName = QInputDialog::getText(this, auServerMonApp->Name(),
			"Enter unit name to find online.", QLineEdit::Normal, "");
	if(!unitName.isEmpty())
	{
		QPalette pal;
		for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		{
			QListWidgetItem* item = m_ConnectionsList->item(rows[selIdx]);
			item->setData(Qt::UserRole+1, QDateTime::currentDateTime());
			item->setData(Qt::UserRole+2, 1);
			item->setData(Qt::UserRole+3, 0);
			item->setData(Qt::UserRole+4, 0);
			item->setData(Qt::UserRole+6, 0);
			item->setData(Qt::UserRole+10, 3);	// 3=searching
			item->setForeground(pal.text().color());
			item->setIcon(QIcon(QPixmap(":data/Server.png").scaled(d_IconSize,d_IconSize, Qt::KeepAspectRatio,
							Qt::SmoothTransformation)));

			emit FindUnitName(serverNames[selIdx], serverPorts[selIdx], unitName);
		}
	}
}

void AuServerCon::ServerEnableAll()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 0, 1, 0);
}

void AuServerCon::ServerEnableAnimalEnc()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 2, 1, 0);
}

void AuServerCon::ServerEnableCreatureDecay()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 28, 1, 0);
}

void AuServerCon::ServerEnableJobPosting()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 23, 1, 0);
}

void AuServerCon::ServerEnablePirateEnc()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 24, 1, 0);
}

void AuServerCon::ServerEnableReportOff()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 25, 1, 0);
}

void AuServerCon::ServerEnableStationResupply()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 26, 1, 0);
}

void AuServerCon::ServerEnableSystemEmission()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 27, 1, 0);
}

void AuServerCon::ServerEnableTroopResupply()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 29, 1, 0);
}

void AuServerCon::ServerEnableUnitDecay()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 30, 1, 0);
}

void AuServerCon::ServerEnableAnimalAI()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 1, 1, 0);
}

void AuServerCon::ServerEnableCharAI()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 14, 1, 0);
}

void AuServerCon::ServerEnableCitizenPath()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 20, 1, 0);
}

void AuServerCon::ServerEnableCopAI()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 15, 1, 0);
}

void AuServerCon::ServerEnableCrewAI()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 3, 1, 0);
}

void AuServerCon::ServerEnableIndiAI()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 32, 1, 0);
}

void AuServerCon::ServerEnableOffAI()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 4, 1, 0);
}

void AuServerCon::ServerEnablePassAI()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 5, 1, 0);
}

void AuServerCon::ServerEnableTroopAI()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 11, 1, 0);
}

void AuServerCon::ServerEnableWorkerAI()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 16, 1, 0);
}

void AuServerCon::ServerEnableCityScan()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 6, 1, 0);
}

void AuServerCon::ServerEnableShipScan()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 7, 1, 0);
}

void AuServerCon::ServerEnableStationScan()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 8, 1, 0);
}

void AuServerCon::ServerEnableMovement()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 19, 1, 0);
}

void AuServerCon::ServerEnableSetModified()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 18, 1, 0);
}

void AuServerCon::ServerDisableAll()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 0, 0, 0);
}

void AuServerCon::ServerDisableAnimalEnc()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 2, 0, 0);
}

void AuServerCon::ServerDisableCreatureDecay()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 28, 0, 0);
}

void AuServerCon::ServerDisableJobPosting()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 23, 0, 0);
}

void AuServerCon::ServerDisablePirateEnc()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 24, 0, 0);
}

void AuServerCon::ServerDisableReportOff()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 25, 0, 0);
}

void AuServerCon::ServerDisableStationResupply()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 26, 0, 0);
}

void AuServerCon::ServerDisableSystemEmission()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 27, 0, 0);
}

void AuServerCon::ServerDisableTroopResupply()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 29, 0, 0);
}

void AuServerCon::ServerDisableUnitDecay()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 30, 0, 0);
}

void AuServerCon::ServerDisableAnimalAI()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 1, 0, 0);
}

void AuServerCon::ServerDisableCharAI()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 14, 0, 0);
}

void AuServerCon::ServerDisableCitizenPath()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 20, 0, 0);
}

void AuServerCon::ServerDisableCopAI()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 15, 0, 0);
}

void AuServerCon::ServerDisableCrewAI()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 3, 0, 0);
}

void AuServerCon::ServerDisableIndiAI()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 32, 0, 0);
}

void AuServerCon::ServerDisableOffAI()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 4, 0, 0);
}

void AuServerCon::ServerDisablePassAI()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 5, 0, 0);
}

void AuServerCon::ServerDisableTroopAI()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 11, 0, 0);
}

void AuServerCon::ServerDisableWorkerAI()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 16, 0, 0);
}

void AuServerCon::ServerDisableCityScan()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 6, 0, 0);
}

void AuServerCon::ServerDisableShipScan()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 7, 0, 0);
}

void AuServerCon::ServerDisableStationScan()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 8, 0, 0);
}

void AuServerCon::ServerDisableMovement()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 19, 0, 0);
}

void AuServerCon::ServerDisableSetModified()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 18, 0, 0);
}

void AuServerCon::StoryReloadAllScripts()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit ReloadScript(serverNames[selIdx], serverPorts[selIdx], 0);
}

void AuServerCon::StoryReloadScript()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	QString script = QInputDialog::getText(this, auServerMonApp->Name(),
			"Enter script id to reload.", QLineEdit::Normal, "");
	int scriptId = QVariant(script).toUInt();
	if(scriptId)
	{
		for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
			emit ReloadScript(serverNames[selIdx], serverPorts[selIdx], scriptId);
	}
}

void AuServerCon::TickEnable()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 9, 1, 0);
}

void AuServerCon::TickDisable()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 9, 0, 0);
}

void AuServerCon::TickEnableAtomId()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	QString atomIdS = QInputDialog::getText(this, auServerMonApp->Name(),
			"Enter atom id (numeric or string) to resume ticking.", QLineEdit::Normal, "");
	quint32 atomId = AtomId(atomIdS);
	if(atomId)
		for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
			emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 12, 1, atomId);
}

void AuServerCon::TickEnableAtomType()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	QString atomTypeS = QInputDialog::getText(this, auServerMonApp->Name(),
			"Enter the type a atom to resume ticking.", QLineEdit::Normal, "");
	quint32 atomType = QVariant(atomTypeS).toUInt();
	if(atomType)
		for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
			emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 21, 1, atomType);
}

void AuServerCon::TickExcludeAtomId()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	QString atomIdS = QInputDialog::getText(this, auServerMonApp->Name(),
			"Enter atom id (numeric or string) to suspend ticking.", QLineEdit::Normal, "");
	quint32 atomId = AtomId(atomIdS);
	if(atomId)
		for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
			emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 12, 0, atomId);
}

void AuServerCon::TickExcludeAtomType()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	QString atomTypeS = QInputDialog::getText(this, auServerMonApp->Name(),
			"Enter the type of atom to suspend ticking.", QLineEdit::Normal, "");
	quint32 atomType = QVariant(atomTypeS).toUInt();
	if(atomType)
		for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
			emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 21, 0, atomType);
}

void AuServerCon::TickClearAtomIdExclusions()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 33, 0, 0);
}

void AuServerCon::TickClearAtomTypeExclusions()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 34, 0, 0);
}

void AuServerCon::TockEnable()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 10, 1, 0);
}

void AuServerCon::TockDisable()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 10, 0, 0);
}

void AuServerCon::TockEnableAtomId()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	QString atomIdS = QInputDialog::getText(this, auServerMonApp->Name(),
			"Enter atom id (numeric or string) to resume tocking.", QLineEdit::Normal, "");
	quint32 atomId = AtomId(atomIdS);
	if(atomId)
		for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
			emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 13, 1, atomId);
}

void AuServerCon::TockEnableAtomType()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	QString atomTypeS = QInputDialog::getText(this, auServerMonApp->Name(),
			"Enter the type of atom to resume tocking.", QLineEdit::Normal, "");
	quint32 atomType = QVariant(atomTypeS).toUInt();
	if(atomType)
		for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
			emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 22, 1, atomType);
}

void AuServerCon::TockExcludeAtomId()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	QString atomIdS = QInputDialog::getText(this, auServerMonApp->Name(),
			"Enter atom id (numeric or string) to suspend tocking.", QLineEdit::Normal, "");
	quint32 atomId = AtomId(atomIdS);
	if(atomId)
		for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
			emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 13, 0, atomId);
}

void AuServerCon::TockExcludeAtomType()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	QString atomTypeS = QInputDialog::getText(this, auServerMonApp->Name(),
			"Enter the type of atom to suspend tocking.", QLineEdit::Normal, "");
	quint32 atomType = QVariant(atomTypeS).toUInt();
	if(atomType)
		for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
			emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 22, 0, atomType);
}

void AuServerCon::TockClearAtomIdExclusions()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 35, 0, 0);
}

void AuServerCon::TockClearAtomTypeExclusions()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 36, 0, 0);
}

void AuServerCon::CreateSectorAt()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	QStringList galaxies;
	for(int galaxy=0; galaxy<GALAXY_Count; galaxy++)
		galaxies << GalaxyName((GALAXY)galaxy);
	QString galaxyName = QInputDialog::getItem(this, auServerMonApp->Name(),
			"Choose the galaxy in which to create a sector. "
			"This operation is only performed by the master server.", galaxies, VeilOfTargoss, false);
	quint8 galaxyEnum = GalaxyEnum(galaxyName);
	if(galaxyEnum != GALAXY_Count)
	{
		QString sectorLocS = QInputDialog::getText(this, auServerMonApp->Name(),
				"Enter sector coordinate to create, in pc (e.g. Core sector is 0,0,0). "
				"This operation is only performed by the master server.", QLineEdit::Normal, "");
		AuPoint sectorLocPc = AuPoint(sectorLocS);	// Sector loc in parsecs.
		if(!sectorLocS.isEmpty() && !sectorLocPc.IsZero())
		{
			AuPoint sectorLoc = AuPoint(sectorLocPc / 10).Rounded();
			if(sectorLocPc == sectorLoc * 10)
				for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
					emit CreateSectorAt(serverNames[selIdx], serverPorts[selIdx], galaxyEnum, sectorLocPc.x, sectorLocPc.y,
							sectorLocPc.z);
			else
				QMessageBox::warning(this, auServerMonApp->Name(),
						QString("Sector coordinates are in increments of 10pc and must end in a 0."));
		}
	}
}

void AuServerCon::FinalizeSectorOrSystem()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	QString atomIdS = QInputDialog::getText(this, auServerMonApp->Name(),
			"Enter atom id (numeric or string) to finalize. "
			"A sector id is expected by the master and a solar system id by a manager.", QLineEdit::Normal, "");
	quint32 atomId = AtomId(atomIdS);
	if(atomId)
		for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
			emit FinalizeSystem(serverNames[selIdx], serverPorts[selIdx], atomId);
}

void AuServerCon::ReassignSystem()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	QString lastAtomId = auServerMonApp->Settings().value("Settings/ReassignAtom", "").toString();
	QString atomIdS = QInputDialog::getText(this, auServerMonApp->Name(),
			"Do this on a Manager server. "
			"Enter atom id (numeric or string) of a solar system to reassign. ", QLineEdit::Normal, lastAtomId);
	quint32 atomId = AtomId(atomIdS);
	if(atomId)
	{
		auServerMonApp->Settings().setValue("Settings/ReassignAtom", atomIdS);
		QStringList serverChoices;
		for(int row=0; row<m_ConnectionsList->count(); row++)
		{
			QListWidgetItem* item = m_ConnectionsList->item(row);
			int serverMode = item->data(Qt::UserRole+5).toInt();
			if(serverMode != AuGlobal::SceneWorker)
				continue;
			QString listName = item->data(Qt::UserRole+8).toString();
			serverChoices << listName;
		}
		bool ok = false;
		QString listName = QInputDialog::getItem(this, auServerMonApp->Name(),
				"Choose a worker server to receive the solar system assignment. ",
				serverChoices, 0, false, &ok);
		if(ok && !listName.isEmpty())
		{
			// Figure out what server it is.
			QString netName;
			quint16 netPort = 0;
			bool foundServer = false;
			for(QList<AuServerChoice>::const_iterator si = m_PublicServers.cbegin(); si != m_PublicServers.cend(); si++)
				if((*si).ListName() == listName)
				{
					netName = (*si).NetName();
					netPort = (*si).NetPort();
					foundServer = true;
					break;
				}
			if(!foundServer)
			{
				// Check debug servers.
				for(QList<AuServerChoice>::const_iterator si = m_DebugServers.cbegin(); si != m_DebugServers.cend(); si++)
					if((*si).ListName() == listName)
					{
						netName = (*si).NetName();
						netPort = (*si).NetPort();
						foundServer = true;
						break;
					}
			}

			// Determine the worker server port based on its manager port.
			if(netPort == 16763)
				netPort = 16760;
			else if(netPort == 16766)
				netPort = 16764;
			else if(netPort == 16769)
				netPort = 16767;
			else if(netPort == 16772)
				netPort = 16770;
			for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
				emit ReassignAtom(serverNames[selIdx], serverPorts[selIdx], atomId, netName, netPort);
		}
	}
}

void AuServerCon::RevokeSectorOrSystem()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	QString atomIdS = QInputDialog::getText(this, auServerMonApp->Name(),
			"Enter atom id (numeric or string) of an assignment to revoke. "
			"A sector id is expected by the master and a solar system id by a manager.", QLineEdit::Normal, "");
	quint32 atomId = AtomId(atomIdS);
	if(atomId)
		for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
			emit RevokeAtom(serverNames[selIdx], serverPorts[selIdx], atomId);
}

void AuServerCon::RestoreEmpire()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	QString fileName = QFileDialog::getOpenFileName(this, tr("Select File Containing Empire"), "", "", 0,
			QFileDialog::DontUseNativeDialog);
	if(!fileName.isEmpty())
	{
		DgEmpire empire;
		QFile file(fileName);
		if(file.open(QFile::ReadOnly))
		{
			QDataStream ds(&file);
			ds.setVersion(QDataStream::Qt_5_8);
			empire << ds;
		}

		if(empire.m_id)
			for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
				emit RestoreEmpire(serverNames[selIdx], serverPorts[selIdx], empire);
	}
}

void AuServerCon::LimboRescue()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	QString atomIdS = QInputDialog::getText(this, auServerMonApp->Name(),
			"Enter atom id to limbo rescue.", QLineEdit::Normal, "");
	quint32 atomId = AtomId(atomIdS);
	if(atomId)
	{
		for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
			emit LimboRescue(serverNames[selIdx], serverPorts[selIdx], atomId);
	}
}

void AuServerCon::LimboRescueList()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	QString fileName = QFileDialog::getOpenFileName(this, tr("Select File Containing AuIdList"), "", "", 0,
			QFileDialog::DontUseNativeDialog);
	if(!fileName.isEmpty())
	{
		QList<quint32> atomIds;
		QFile file(fileName);
		if(file.open(QFile::ReadOnly))
		{
			QDataStream ds(&file);
			ds.setVersion(QDataStream::Qt_4_1);

			ds >> atomIds;
		}

		if(!atomIds.isEmpty())
			for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
				emit LimboRescueList(serverNames[selIdx], serverPorts[selIdx], atomIds);
	}
}

void AuServerCon::ReloadStatic()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit RequestReloadData(serverNames[selIdx], serverPorts[selIdx]);
}

void AuServerCon::SetAtomModified()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	QString atomIdS = QInputDialog::getText(this, auServerMonApp->Name(),
			"Enter atom id (numeric or string) on which to call SetModified. "
			"The server will assert/crash if it calls SetModified on an inappropriate atom.", QLineEdit::Normal, "");
	quint32 atomId = AtomId(atomIdS);
	if(atomId)
	{
		QMessageBox::StandardButton recurseButton = QMessageBox::question(this, auServerMonApp->Name(),
				tr("Recurse to all subatoms of %1 when calling SetModified.").arg(atomId),
				QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel, QMessageBox::No);
		if(recurseButton == QMessageBox::Yes || recurseButton == QMessageBox::No)
		{
			bool recursive = recurseButton == QMessageBox::Yes;
			for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
				emit SetAtomModified(serverNames[selIdx], serverPorts[selIdx], atomId, recursive);
		}
	}
}

void AuServerCon::SetControlDecay()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	QString decayS = QInputDialog::getText(this, auServerMonApp->Name(),
			"Enter number of milliseconds for Connection Decay Margin. Max is 2000ms.", QLineEdit::Normal, "");
	if(!decayS.isEmpty())
	{
		quint16 decayMs = QVariant(decayS).toUInt();
		if(decayMs <= 2000)
			for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
				emit SetControlDecay(serverNames[selIdx], serverPorts[selIdx], decayMs);
	}
}

void AuServerCon::SetUDPDecay()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	QString decayS = QInputDialog::getText(this, auServerMonApp->Name(),
			"Enter number of milliseconds for UDP Packet Decay Margin. Max is 2000ms.", QLineEdit::Normal, "");
	if(!decayS.isEmpty())
	{
		quint16 decayMs = QVariant(decayS).toUInt();
		if(decayMs <= 2000)
			for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
				emit SetUdpDecay(serverNames[selIdx], serverPorts[selIdx], decayMs);
	}
}

void AuServerCon::ShutdownServer()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	AuShutdownDlg shutdownDlg(this);
	if(shutdownDlg.exec() == QDialog::Accepted)
	{
		QString warnMsg = shutdownDlg.WarnMsg();
		QString haltMsg = shutdownDlg.HaltMsg();
		qint16 shutdownSecs = shutdownDlg.ShutdownSeconds();
		for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
			emit Shutdown(serverNames[selIdx], serverPorts[selIdx], warnMsg, haltMsg, shutdownSecs);
	}
}

void AuServerCon::HaltShutdown()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	AuShutdownHaltDlg haltDlg(this);
	if(haltDlg.exec() == QDialog::Accepted)
	{
		QString haltMsg = haltDlg.Message();
		for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
			emit ShutdownHalt(serverNames[selIdx], serverPorts[selIdx], haltMsg);
	}
}

void AuServerCon::ServerBroadcast()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	QString lastAnnouncement = auServerMonApp->Settings().value("Settings/Announcement", "").toString();
	QString text = QInputDialog::getText(this, auServerMonApp->Name(),
			tr("Announcement for Master Server to Broadcast"),
			QLineEdit::Normal, lastAnnouncement);
	if(text.isEmpty())
		return;
	auServerMonApp->Settings().setValue("Settings/Announcement", text);

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit Broadcast(serverNames[selIdx], serverPorts[selIdx], text);
}

void AuServerCon::ServerEnableTockTransactions()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 17, 1, 0);
}

void AuServerCon::ServerDisableTockTransactions()
{
	// Get info about the connected servers.
	QList<int> rows;
	QStringList serverTitles;
	QStringList serverNames;
	QList<quint16> serverPorts;
	Servers(rows, serverTitles, serverNames, serverPorts);
	if(rows.isEmpty())
		return;

	for(int selIdx=0; selIdx<serverTitles.count(); selIdx++)
		emit DebugSwitch(serverNames[selIdx], serverPorts[selIdx], 17, 0, 0);
}

