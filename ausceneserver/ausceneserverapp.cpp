//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausceneserverapp.cpp,v 1.1253 2022/05/17 20:39:44 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausceneserverapp.h"
//
#include "auatominterface.h"
#include "auatommanager.h"
#include "aubddesign.h"
#include "audbglobal.h"
#include "auether.h"
#include "aulogincomm.h"
#include "auplayeronlinestatus.h"
#include "auscdesign.h"
#include "auscparts.h"
#include "daanimal.h"
#include "daarena.h"
#include "daatom.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dadesignbase.h"
#include "dadesignplane.h"
#include "daeffect.h"
#include "dagalaxy.h"
#include "daitem.h"
#include "damissile.h"
#include "danpc.h"
#include "daperson.h"
#include "dasc.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dastar.h"
#include "dateleporter.h"
#include "daunit.h"
#include "dauniverse.h"
#include "davehicle.h"
#include "daworldll.h"
#include "dgdata.h"
#include "dpconsoletransporter.h"
#include "dtchatchannel.h"
#include "dtclientchannel.h"
#include "dtdb.h"
#include "dtmailclient.h"
#include "dtmcpacket.h"
#include "dtsceneserver.h"
#include "dtserverchannel.h"
#include "dttick.h"
#include "dttock.h"
#include "dttrash.h"
#include <QCryptographicHash>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <signal.h>
#include <typeinfo>			// for bad_cast

#if defined(MEM_TRACE_GLOBAL)
quint64 s_AllocatedGlobal = 0;
quint64 s_FreedGlobal = 0;
#endif

AuSceneServerApp::AuSceneServerApp(int& argc, char** argv) : QCoreApplication(argc, argv)
{
	m_AtomManagerCount = 0;
	m_AtomMgrLoadIndex = 0;
	for(int l=0; l<60; l++)
		m_AtomMgrLoads[l] = -1;
	m_AtomMgrLoadsValid = 0;
	m_Data = 0;
	m_Etherium = 0;
	m_JobOfferLast.start();				// Start the clock on job offer acceptance.
	m_ModelCache = 0;
	m_ShutdownSeconds = 0;	// 0 until we are shutting down.
	m_tClientChannel = 0;
	m_tDb = 0;			// Keeps local data in sync with the database.
	m_timer = 0;
	m_tMailClient = 0;
	m_tmUp.Set();
	m_tTrash = 0;
	m_UdpBytesRx = 0;	// count of udp bytes received
	m_UdpBytesTx = 0;	// count of udp bytes sent
	m_UdpPacketsRx = 0;	// count of udp packets received
	m_UdpPacketsTx = 0;	// count of udp packets sent

	// Get default values that may be overridden by command line options.
	quint16 chatPort = 0;
	quint16 mgrPort = 0;
	QString netName;
	int role = 2;			// default 2=worker role. 0=master, 1=manager
	quint16 scenePort = AuUtil::SceneMasterPort();
	QString settingsAlt;

	// Parse command line.
	QStringList args = arguments();
	for(QStringList::const_iterator it = args.cbegin(); it != args.cend(); it++)
	{
		if((*it).left(1) == "=")
		{
			settingsAlt = QVariant((*it).mid(1)).toString();
		}
		else if((*it).left(2) == "-p")
		{
			scenePort = QVariant((*it).mid(2)).toInt();
		}
		else if((*it).left(2) == "-c")
		{
			chatPort = QVariant((*it).mid(2)).toInt();
		}
		else if((*it).left(2) == "-n")
		{
			netName = (*it).mid(2);
		}
		else if((*it).left(2) == "-l")
		{
			m_LoginName = (*it).mid(2);
		}
		else if((*it).left(2) == "-w")
		{
			QString loginPass = (*it).mid(2);

			// Create a SHA-1 hash key from the password.
			QByteArray passCrypt;
			passCrypt.append(loginPass.toUtf8());
			QCryptographicHash passCryptHash(QCryptographicHash::Sha1);
			passCryptHash.addData(passCrypt);
			m_LoginPassCrypt = passCryptHash.result();
		}
		else if((*it).left(2) == "-s")
		{
			// A scene worker can be directed to report directly to a specific manager, rather than reporting to the master to get
			// deferred to a manager. Likewise, a scene manager can be directed to report directly to a specific master.
			// This is convenient when a worker shares a subnet with a manager or when running the manager on the debug server farm.
			// An anticipated configuration involves putting each manager and its workers on their own subnet, routed through a
			// high-speed internet gateway.
			m_ClientChannelServer = (*it).mid(2);
		}
		else if((*it).left(2) == "-t")
		{
			m_AtomManagerCount = QVariant((*it).mid(2)).toInt();
		}
		else if((*it) == "-master")
		{
			role = 0;
		}
		else if((*it) == "-manager")
		{
			role = 1;
		}
		else if((*it).left(2) == "-m")
		{
			// MUST test for this option after -master and -manager due to spelling. 
			mgrPort = QVariant((*it).mid(2)).toInt();
		}
	}

	// Get settings from a configuration file.
	QString settingsFile;
	if(settingsAlt.isEmpty())
		settingsFile = QString("au.ini");
	else
	{
		settingsFile = QString("%1.ini").arg(settingsAlt);
		if(!QFileInfo::exists(settingsFile))
		{
			// They specified a settings file. It doesn't exist.
			qWarning() << "error: No settings file, terminating." << settingsFile;
			QTimer::singleShot(0, [=](){quit();});
			return;
		}
	}
	QSettings settings(settingsFile, QSettings::IniFormat);
	if(settings.contains("Settings/atomManagers"))								// atomManagers
		m_AtomManagerCount = settings.value("Settings/atomManagers").toUInt();
	if(settings.contains("Settings/bossName"))									// bossName
		m_ClientChannelServer = settings.value("Settings/bossName").toString();
	if(settings.contains("Settings/chatPort"))									// chatPort
		chatPort = settings.value("Settings/chatPort").toUInt();
	if(settings.contains("Settings/loginName"))									// loginName
		m_LoginName = settings.value("Settings/loginName").toString();
	if(settings.contains("Settings/loginPass"))									// loginPass
	{
		QString loginPass = settings.value("Settings/loginPass").toString();

		// Create a SHA-1 hash key from the password.
		QByteArray passCrypt;
		passCrypt.append(loginPass.toUtf8());
		QCryptographicHash passCryptHash(QCryptographicHash::Sha1);
		passCryptHash.addData(passCrypt);
		m_LoginPassCrypt = passCryptHash.result();
	}
	if(settings.contains("Settings/mgrPort"))									// mgrPort
		mgrPort = settings.value("Settings/mgrPort").toUInt();
	if(settings.contains("Settings/netName"))									// netName
		netName = settings.value("Settings/netName").toString();
	if(settings.contains("Settings/scenePort"))									// scenePort
		scenePort = settings.value("Settings/scenePort").toUInt();
	if(settings.contains("Settings/serverRole"))								// serverRole
	{
		QString srole = settings.value("Settings/serverRole").toString();
		if(srole.compare("master", Qt::CaseInsensitive) == 0)
			role = 0;
		else if(srole.compare("manager", Qt::CaseInsensitive) == 0)
			role = 1;
		else if(srole.compare("worker", Qt::CaseInsensitive) == 0)
			role = 2;
	}

	// Invalid atom manager count results in the default count.
	int idealThreadCount = QThread::idealThreadCount();
	if(m_AtomManagerCount <= 0 || m_AtomManagerCount > idealThreadCount)
	{
		// Each atom manager is a bundle of threads so the ideal number of atom managers is probably close to 1/2 the number of
		// cores available.
		m_AtomManagerCount = idealThreadCount / 2;
		if(!m_AtomManagerCount)
			m_AtomManagerCount = 1;
	}

	// Create 'server mode' global data object independent of app object.
	{
		QString copy;
		if(QString(AuUtil::CopyrightYear()) == QString("2022"))
			copy = QString("Copyright %1 2022 Hazeron Freehold LLC. All rights reserved.")
				.arg(AuGlobal::CopyrightSymbol());
		else
			copy = QString("Copyright %1 2022-%2 Hazeron Freehold LLC. All rights reserved.")
				.arg(AuGlobal::CopyrightSymbol())
				.arg(AuUtil::CopyrightYear());
		if(role == 0)
		{
			// Create 'SceneMaster' global data object independent of app object.
			m_Data = new DgData;
			m_Etherium = new AuEther("Hazeron");
			gp_AuGlobal = new AuDbGlobal("Hazeron Scene Master", "Hazeron Freehold", AuGlobal::SceneMaster, m_Data, m_Etherium);
			if(settings.contains("Settings/logFile"))									// logFile
			{
				QString logFile = settings.value("Settings/logFile").toString();
				if(!g_Au.OpenLogFile(logFile))
					qWarning() << "error: Failed to open logFile" << logFile;
			}
			if(settings.contains("Settings/connectFile"))								// connectFile
			{
				QString connectFile = settings.value("Settings/connectFile").toString();
				if(!g_Au.OpenLogConnectFile(connectFile))
					qWarning() << "error: Failed to open connectFile" << connectFile;
			}
			if(g_Au.m_LogMessages & LOG_Log)
			{
				QString title = QString("%1 %2").arg(g_Au.ApplicationName()).arg(AuUtil::VersionFull());
				g_Au.Log(title);
				g_Au.Log(copy);
			}
		}
		else if(role == 1)
		{
			// Create 'SceneManager' global data object independent of app object.
			m_Data = new DgData;
			m_Etherium = new AuEther("Hazeron");
			gp_AuGlobal = new AuDbGlobal("Hazeron Scene Manager", "Hazeron Freehold", AuGlobal::SceneManager,m_Data,m_Etherium);
			if(settings.contains("Settings/logFile"))									// logFile
			{
				QString logFile = settings.value("Settings/logFile").toString();
				if(!g_Au.OpenLogFile(logFile))
					qWarning() << "error: Failed to open logFile" << logFile;
			}
			if(settings.contains("Settings/connectFile"))								// connectFile
			{
				QString connectFile = settings.value("Settings/connectFile").toString();
				if(!g_Au.OpenLogConnectFile(connectFile))
					qWarning() << "error: Failed to open connectFile" << connectFile;
			}
			if(g_Au.m_LogMessages & LOG_Log)
			{
				QString title = QString("%1 %2").arg(g_Au.ApplicationName()).arg(AuUtil::VersionFull());
				g_Au.Log(title);
				g_Au.Log(copy);
			}

			// Configure manager for specific master.
			if(!m_ClientChannelServer.isEmpty())
				g_Au.SetClientChannelServer(m_ClientChannelServer);
		}
		else
		{
			// Create 'SceneWorker' global data object independent of app object.
			m_Data = new DgData;
			m_Etherium = new AuEther("Hazeron");
			gp_AuGlobal = new AuDbGlobal("Hazeron Scene Worker", "Hazeron Freehold", AuGlobal::SceneWorker, m_Data, m_Etherium);
			if(settings.contains("Settings/logFile"))									// logFile
			{
				QString logFile = settings.value("Settings/logFile").toString();
				if(!g_Au.OpenLogFile(logFile))
					qWarning() << "error: Failed to open logFile" << logFile;
			}
			if(settings.contains("Settings/connectFile"))								// connectFile
			{
				QString connectFile = settings.value("Settings/connectFile").toString();
				if(!g_Au.OpenLogConnectFile(connectFile))
					qWarning() << "error: Failed to open connectFile" << connectFile;
			}
			if(g_Au.m_LogMessages & LOG_Log)
			{
				QString title = QString("%1 %2").arg(g_Au.ApplicationName()).arg(AuUtil::VersionFull());
				g_Au.Log(title);
				g_Au.Log(copy);
			}

			// Configure worker for specific manager.
			if(!m_ClientChannelServer.isEmpty())
				g_Au.SetClientChannelServer(m_ClientChannelServer);

			// Create a model cache. The manager and master don't need this.
			m_ModelCache = new AuGLModelCache;
			g_Au.SetModelCache(m_ModelCache);

			// Warn if chat port not set.
			if(chatPort)
				g_Au.SetChatPort(chatPort);
			else
				g_Au.Error("Chat server port not set. -cPORTNUMBER option needed.");
		}

		// Warn if server manager port not set.
		if(mgrPort)
			g_Au.SetManagerPort(mgrPort);
		else
			g_Au.Error("Server manager port not set. -mPORTNUMBER option needed.");

		// Configure internet name.
		if(!netName.isEmpty())
			g_Au.SetInternetName(netName);

		// Server clears all cache files whenever it comes up.
		g_Au.ClearAllCacheFiles();

		//g_Au.m_LogMessages |= LOG_Spawn|LOG_Travel;
		//g_Au.m_LogMessages |= LOG_Thread;
		//g_Au.m_LogMessages |= LOG_Database;
		//g_Au.m_LogMessages |= LOG_AtomKillAndDestruct|LOG_Etherium;

		// Configure scene listen port for scene worker and manager.
		// NOTE: Scene master server never listens on a non-standard port.
		g_Au.SetSceneServer(g_Au.SceneMasterName(), scenePort, 0);

		// Seed the random number generator so we don't get the same results every time.
		g_Au.Dice().TimeSeed();

		// Start threads.
		StartThreads();

		// Start the server channel listener for connections from sub servers.
		StartServerChannel();	// Does nothing if I'm a scene worker, i.e. no subs.
	}
}

AuSceneServerApp::~AuSceneServerApp()
{
	// Must delete atoms before the AuGlobal object and the ModelCache.
	if(m_Etherium)
	{
		delete m_Etherium;
		m_Etherium = 0;
	}

	if(gp_AuGlobal)
	{
		delete gp_AuGlobal;
		gp_AuGlobal = 0;
	}

	if(m_Data)
	{
		delete m_Data;
		m_Data = 0;
	}

	if(m_ModelCache)
	{
		delete m_ModelCache;
		m_ModelCache = 0;
	}

	for(unordered_map<AuId, AuAtomManaged*, AuIdHash>::iterator msi = m_MySystems.begin(); msi != m_MySystems.end(); msi++)
		delete (*msi).second;
	m_MySystems.clear();
	//qDebug() << "AuSceneServerApp::~AuSceneServerApp";
}

void AuSceneServerApp::StartThreads()
{
	m_timer = new QTimer(this);
	connect(m_timer, SIGNAL(timeout()), SLOT(UpdateStats()));
	m_timer->start(d_MSecPerSec);

	// Login to the login server.
	AuLoginComm login;
	int result = login.Login(m_LoginName, m_LoginPassCrypt);
	switch(result)
	{
	case 0:
		if(g_Au.m_LogMessages & LOG_Log)
			g_Au.Log(QString("Login Successful as '%1'").arg(m_LoginName));
		break;
	case 1:
		if(g_Au.m_LogMessages & LOG_Error)
			g_Au.Error(QString("Login Error, terminating."));
		QTimer::singleShot(0, [=](){quit();});
		return;
	case 2:
		if(g_Au.m_LogMessages & LOG_Warning)
			g_Au.Error(QString("Login Failed - Account Name '%1', terminating.").arg(m_LoginName));
		QTimer::singleShot(0, [=](){quit();});
		return;
	case 3:
		if(g_Au.m_LogMessages & LOG_Warning)
			g_Au.Error(QString("Version Error, terminating."));
		QTimer::singleShot(0, [=](){quit();});
		return;
	}

	if(!g_Au.IsSceneMaster())
	{
		m_tClientChannel = new DtClientChannel(this);
		connect(m_tClientChannel, SIGNAL(finished()), SLOT(ClientChannelLost()),
				Qt::QueuedConnection);
		connect(m_tClientChannel, SIGNAL(AdminSummon(AuId, AuId, quint8)), SLOT(AdminSummonDown(AuId, AuId, quint8)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(ArenaReady(AuArenaReady)), SLOT(ArenaReadyDown(AuArenaReady)), Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(AssignOffice(AuId, quint32, AuOffice, AuId, QString, AuId, QString)),
				SLOT(AssignOffice(AuId, quint32, AuOffice, AuId, QString, AuId, QString)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(AtomSpawning(AuId, quint32, AuId, AuId, AuId, AuId, quint8)),
				SLOT(SpawnAtomDown(AuId, quint32, AuId, AuId, AuId, AuId, quint8)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(HengeTeleportAtom(AuHengeTeleportAtom)),
				SLOT(HengeTeleportAtomDown(AuHengeTeleportAtom)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(AtomTraveling(AuId, AuId, AuId)), SLOT(AtomTraveledDown(AuId, AuId, AuId)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(BlueprintRatedBd(AuExchangeRatingRec)),
				SLOT(BlueprintRatedBdDown(AuExchangeRatingRec)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(BlueprintRatedSc(AuExchangeRatingRec)),
				SLOT(BlueprintRatedScDown(AuExchangeRatingRec)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(BountyPosted(AuId, QString, AuId, QString, double, int)),
				SLOT(BountyPostedDown(AuId, QString, AuId, QString, double, int)),
				Qt::QueuedConnection);
		connect(m_tClientChannel, SIGNAL(SystemWarpSignatureDown(AuId, quint32, quint8, float, float, float)),
				SLOT(SystemWarpSignatureBroadcast(AuId, quint32, quint8, float, float, float)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(ChatDown(QByteArray)), SLOT(ChatReceivedDown(QByteArray)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(AvatarAccountActiveSignal(AuIdList)),
				SLOT(AvatarAccountActiveDown(AuIdList)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(ClaimTitleSignal(AuId, QString, AuId, QString, quint32)),
				SLOT(ClaimTitleDown(AuId, QString, AuId, QString, quint32)),
				Qt::QueuedConnection);

		// City mail request works its way down the server farm this way.
		connect(m_tClientChannel, SIGNAL(CityMailRequestedSignal(AuId, AuId, quint32)),
				SLOT(CityMailRequestedDown(AuId, AuId, quint32)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(CreditCityBank(float, AuId)), SLOT(CreditCityBankDown(float, AuId)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(CloseStargateDown(AuId, AuPoint)),
				SLOT(CloseStargateDown(AuId, AuPoint)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(CollectBounty(AuId, QString, AuGearPos, AuBountyOnItem)),
				SLOT(CollectBountyDown(AuId, QString, AuGearPos, AuBountyOnItem)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(PayBounty(AuId, AuGearPos, AuBountyOnItem, bool)),
				SLOT(PayBountyDown(AuId, AuGearPos, AuBountyOnItem, bool)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(EmpireChanged(DgEmpire)), SLOT(EmpireChanged(DgEmpire)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(EmpireRecordDeleted(quint32)), SLOT(EmpireRecordDeleted(quint32)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(HengeActivatedSignal(AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
				SLOT(HengeActivatedDown(AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(HengeCreatedSignal(AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
				SLOT(HengeCreatedDown(AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(HengeDeactivatedSignal(AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
				SLOT(HengeDeactivatedDown(AuId, quint8, quint8, quint8, AuId, quint8, quint8)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(HengeDestinationConfirmed(AuId, AuId, QString, quint8, QString, QString, quint8, AuGlobeLLF,
						bool, quint8, bool)),
				SLOT(HengeDestinationDown(AuId, AuId, QString, quint8, QString, QString, quint8, AuGlobeLLF,
						bool, quint8, bool)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(OpenChannelDown(AuId, QString, AuId, quint8, AuId, QString, quint8)),
				SLOT(OpenChannelDown(AuId, QString, AuId, quint8, AuId, QString, quint8)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(OpenStargateDown(AuId, AuPoint, AuId, AuPoint, AuId, AuId, AuTime)),
				SLOT(OpenStargateDown(AuId, AuPoint, AuId, AuPoint, AuId, AuId, AuTime)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(PlayerOnlineStatus(AuPlayerOnlineStatusVector)),
				SLOT(PlayersOnlineStatusDown(AuPlayerOnlineStatusVector)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(DeactivateTrade(AuTradeAgreement, quint8, bool)),
				SLOT(DeactivateTradeDown(AuTradeAgreement, quint8, bool)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(AcceptTrade(AuTradeAgreement)),
				SLOT(AcceptTradeDown(AuTradeAgreement)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(ProposeTrade(AuTradeAgreement)),
				SLOT(ProposeTradeDown(AuTradeAgreement)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(TradeReadyToShip(AuId, AuTradeAgreement)),
				SLOT(TradeReadyToShipDown(AuId, AuTradeAgreement)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(TradeShipped(AuId, AuTradeAgreement)),
				SLOT(TradeShippedDown(AuId, AuTradeAgreement)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(ShipIt(AuItemTEUContainer)),
				SLOT(ShipItDown(AuItemTEUContainer)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(SpacecraftNameChangedDown(AuId, QString)),
				SLOT(SpacecraftNameChangedDown(AuId, QString)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(SpacecraftStasisRecover(AuId, AuId, quint8, bool)),
				SLOT(SpacecraftStasisRecoverDown(AuId, AuId, quint8, bool)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(SpeciesChanged(AuSpecies, QString, AuId)),
				SLOT(SpeciesChanged(AuSpecies, QString, AuId)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(GovCitySurrendered(AuId, AuId, quint32, quint32)),
				SLOT(GovCitySurrenderedDown(AuId, AuId, quint32, quint32)),
				Qt::QueuedConnection);
		connect(m_tClientChannel, SIGNAL(HandoffElsewhere(AuId, AuId, quint8, bool, QByteArray, AuId)),
				SLOT(HandoffAtomDown(AuId, AuId, quint8, bool, QByteArray, AuId)),
				Qt::QueuedConnection);
		connect(m_tClientChannel, SIGNAL(OfficerKilled(const AuId)), SLOT(OfficerKilledDown(const AuId)),
				Qt::QueuedConnection);
		connect(m_tClientChannel, SIGNAL(PersonDeleted(AuId)), SLOT(PersonDeletedDown(AuId)),
				Qt::QueuedConnection);
		connect(m_tClientChannel, SIGNAL(AvatarOnLine(AuId, quint32)),
				SLOT(AvatarOnLineDown(AuId, quint32)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(PirateBaseEncountered(AuId)),
				SLOT(PirateBaseEncounteredDown(AuId)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(PostJobOffer(AuJobOffer)), SLOT(PostJobOfferDown(AuJobOffer)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(QueryAtomServer(AuId, quint32, AuId)), SLOT(QueryAtomServer(AuId, quint32, AuId)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(QueryHengeTeleporterServer(AuHengeTeleportAtom)),
				SLOT(QueryHengeTeleporterServer(AuHengeTeleportAtom)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(ReloadStory(quint32)), SLOT(StoryRecordChangedDown(quint32)),
				Qt::QueuedConnection);
		connect(m_tClientChannel, SIGNAL(RemoveStory(quint32)), SLOT(StoryRecordDeletedDown(quint32)),
				Qt::QueuedConnection);
		connect(m_tClientChannel, SIGNAL(RenamePlanet(AuId, QString, AuId, QString, AuId, QString)),
				SLOT(RenamePlanetDown(AuId, QString, AuId, QString, AuId, QString)),
				Qt::QueuedConnection);
		connect(m_tClientChannel, SIGNAL(RenameSector(AuId, QString)), SLOT(RenameSectorDown(AuId, QString)),
				Qt::QueuedConnection);
		connect(m_tClientChannel, SIGNAL(RenameSystem(AuId, QString)), SLOT(RenameSystemDown(AuId, QString)),
				Qt::QueuedConnection);
		connect(m_tClientChannel, SIGNAL(RenameWormhole(AuId, AuId, qint8, quint32, QString)),
				SLOT(RenameWormhole(AuId, AuId, qint8, quint32, QString)),
				Qt::QueuedConnection);
		connect(m_tClientChannel, SIGNAL(SectorAssigned(AuId, int, quint8)), SLOT(SectorAssigned(AuId, int, quint8)),
				Qt::QueuedConnection);
		connect(m_tClientChannel, SIGNAL(SectorDecayed(AuId)), SLOT(SectorDecayedDown(AuId)),
				Qt::QueuedConnection);
		connect(m_tClientChannel, SIGNAL(SectorRevoked(AuId)), SLOT(SectorRevoked(AuId)),
				Qt::QueuedConnection);
		//connect(m_tClientChannel, SIGNAL(SectorVisited(AuId, quint32)), SLOT(SectorVisitedDown(AuId, quint32)),
				//Qt::QueuedConnection);
		connect(m_tClientChannel, SIGNAL(SendFunds(AuId, QString, AuId, double, quint8, quint8, quint32)),
				SLOT(SendFundsDown(AuId, QString, AuId, double, quint8, quint8, quint32)),
				Qt::QueuedConnection);
		connect(m_tClientChannel, SIGNAL(SpawnPersonOnPlanet(AuId, AuId, quint8)), SLOT(SpawnPersonOnPlanet(AuId, AuId, quint8)),
				Qt::QueuedConnection);
		connect(m_tClientChannel, SIGNAL(SummonDormantShip(AuId, AuId)), SLOT(SummonDormantShipDown(AuId, AuId)),
				Qt::QueuedConnection);
		connect(m_tClientChannel, SIGNAL(SystemAssigned(AuId, AuId, quint8)), SLOT(SystemAssigned(AuId, AuId, quint8)),
				Qt::QueuedConnection);
		connect(m_tClientChannel, SIGNAL(SystemRevoked(AuId)), SLOT(SystemRevoked(AuId)),
				Qt::QueuedConnection);
		connect(m_tClientChannel, SIGNAL(ReleaseBerth(AuId, AuId)), SLOT(ReleaseBerthDown(AuId, AuId)),
				Qt::QueuedConnection);
		connect(m_tClientChannel, SIGNAL(SystemExplored(AuId, quint32)), SLOT(SolarSystemExploredDown(AuId, quint32)),
				Qt::QueuedConnection);
		connect(m_tClientChannel, SIGNAL(SystemScanned(AuId, quint32, quint8, quint32)),
				SLOT(SolarSystemScannedDown(AuId, quint32, quint8, quint32)),
				Qt::QueuedConnection);
		connect(m_tClientChannel, SIGNAL(SystemNameChanged(AuId, QString, AuId, QString)),
				SLOT(SystemNameChangedDown(AuId, QString, AuId, QString)),
				Qt::QueuedConnection);
		//connect(m_tClientChannel, SIGNAL(TravelElsewhere(AuId, AuId, AuId, QByteArray)),
				//SLOT(TravelElsewhere(AuId, AuId, AuId, QByteArray)),
				//Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(ScDesignDeactivatedSignal(quint32)),
				SLOT(ScDesignDeactivatedDown(quint32)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(ItDesignDeactivatedSignal(quint32)),
				SLOT(ItDesignDeactivatedDown(quint32)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(ScDesignExchangePublishedSignal(AuScDesignExchangeEntry)),
				SLOT(ScDesignExchangePublishedDown(AuScDesignExchangeEntry)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(ScDesignExchangeUnpublishedSignal(quint32)),
				SLOT(ScDesignExchangeUnpublishedDown(quint32)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(BdDesignDeactivatedSignal(quint32)),
				SLOT(BdDesignDeactivatedDown(quint32)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(BdDesignExchangePublishedSignal(AuBdDesignExchangeEntry)),
				SLOT(BdDesignExchangePublishedDown(AuBdDesignExchangeEntry)),
				Qt::QueuedConnection);

		connect(m_tClientChannel, SIGNAL(BdDesignExchangeUnpublishedSignal(quint32)),
				SLOT(BdDesignExchangeUnpublishedDown(quint32)),
				Qt::QueuedConnection);

		m_tClientChannel->start();
	}

	// Start server manager listener so we can get status right away.
	StartServerManager();

	m_tDb = new DtDb(this);
	if(m_tDb)
	{
		// Connect db thread signals to my slots.
		connect(m_tDb, SIGNAL(AssignedSectorLoaded(AuId)), SLOT(AssignedSectorLoaded(AuId)),
				Qt::QueuedConnection);
		connect(m_tDb, SIGNAL(AssignedSolarSystemLoaded(AuId)), SLOT(AssignedSolarSystemLoaded(AuId)),
				Qt::QueuedConnection);

		connect(m_tDb, SIGNAL(AtomHengeTeleported(AuHengeTeleportAtom)),
				SLOT(HengeTeleportAtomUp(AuHengeTeleportAtom)),
				Qt::QueuedConnection);

		connect(m_tDb, SIGNAL(AtomSpawned(AuId, quint32, AuId, AuId, AuId, quint8)),
				SLOT(SpawnAtomUp(AuId, quint32, AuId, AuId, AuId, quint8)),
				Qt::QueuedConnection);

		connect(m_tDb, SIGNAL(AtomTraveled(AuId, AuId, AuId)), SLOT(AtomTraveledUp(AuId, AuId, AuId)),
				Qt::QueuedConnection);
		connect(m_tDb, SIGNAL(AtomTraveledGalaxyLoc(AuId, AuId, AuPoint)), SLOT(AtomTraveledGalaxyLocUp(AuId, AuId, AuPoint)),
				Qt::QueuedConnection);
		connect(m_tDb, SIGNAL(AtomTraveledStargate(AuId, AuId, AuPoint)), SLOT(AtomTraveledStargateUp(AuId, AuId, AuPoint)),
				Qt::QueuedConnection);
		connect(m_tDb, SIGNAL(CityLastReportReady(AuId, AuId, AuCityReport, bool, AuCityReport)),
				SIGNAL(CityLastReportReady(AuId, AuId, AuCityReport, bool, AuCityReport)),
				Qt::QueuedConnection);
		connect(m_tDb, SIGNAL(CityHistoryReportReady(AuId, AuId, AuCityReportHistory)),
				SIGNAL(CityHistoryReportReady(AuId, AuId, AuCityReportHistory)),
				Qt::QueuedConnection);
		connect(m_tDb, SIGNAL(EmpireRecordChanged(quint32)), SLOT(EmpireRecordChanged(quint32)),
				Qt::QueuedConnection);
		connect(m_tDb, SIGNAL(EmpireRecordDeleted(quint32)), SLOT(EmpireRecordDeleted(quint32)),
				Qt::QueuedConnection);
		connect(m_tDb, SIGNAL(EmpireRecordLoaded(quint32)), SLOT(EmpireRecordLoaded(quint32)),
				Qt::QueuedConnection);
		connect(m_tDb, SIGNAL(finished()), SLOT(DbFinishedInit()),
				Qt::QueuedConnection);
		connect(m_tDb, SIGNAL(FinishedInit()), SLOT(DbFinishedInit()),
				Qt::QueuedConnection);
		connect(m_tDb, SIGNAL(HandoffElsewhere(AuId, AuId, quint8, bool, QByteArray, AuId)),
				SLOT(HandoffAtomUp(AuId, AuId, quint8, bool, QByteArray, AuId)),
				Qt::QueuedConnection);

		connect(this, SIGNAL(HengeTeleportAtomViaDb(AuHengeTeleportAtom)),
				m_tDb, SLOT(HengeTeleportAtom(AuHengeTeleportAtom)),
				Qt::QueuedConnection);

		connect(m_tDb, SIGNAL(NewSectorStored(AuId)), SLOT(NewSectorStored(AuId)),
				Qt::QueuedConnection);
		connect(m_tDb, SIGNAL(OfficerKilled(const AuId)), SLOT(OfficerKilledDown(const AuId)),
				Qt::QueuedConnection);
		if(g_Au.IsSceneMaster())
			connect(m_tDb, SIGNAL(PersonDeleted(AuId)), SLOT(PersonDeletedDown(AuId)),
					Qt::QueuedConnection);
		//connect(m_tDb, SIGNAL(SpeciesRecordChanged(AuSpecies)), SLOT(SpeciesRecordChanged(AuSpecies)));
		connect(m_tDb, SIGNAL(SpeciesRecordLoaded(AuSpecies)), SLOT(SpeciesRecordLoaded(AuSpecies)),
				Qt::QueuedConnection);
		connect(m_tDb, SIGNAL(StoryRecordChanged(quint32)), SLOT(StoryRecordChangedUp(quint32)),
				Qt::QueuedConnection);
		connect(m_tDb, SIGNAL(StoryRecordDeleted(quint32)), SLOT(StoryRecordDeletedUp(quint32)),
				Qt::QueuedConnection);
		connect(this, SIGNAL(AddEmpireToDb(quint32)), m_tDb, SLOT(InsertEmpire(quint32)),
				Qt::QueuedConnection);
		connect(this, SIGNAL(AddSpeciesToDb(AuSpecies)), m_tDb, SLOT(InsertSpecies(AuSpecies)),
				Qt::QueuedConnection);
		connect(this, SIGNAL(ChangeEmpireInDb(quint32)), m_tDb, SLOT(UpdateEmpire(quint32)),
				Qt::QueuedConnection);
		connect(this, SIGNAL(ChangeSpeciesInDb(AuSpecies)), m_tDb, SLOT(UpdateSpecies(AuSpecies)),
				Qt::QueuedConnection);
		connect(this, SIGNAL(CityReportRequested(AuId, quint8, AuId, quint32)),
				m_tDb, SLOT(CityReportRequested(AuId, quint8, AuId, quint32)),
				Qt::QueuedConnection);

		connect(this, SIGNAL(HengeTeleportArrivedViaDb(AuHengeTeleportAtom)),
				m_tDb, SLOT(HengeTeleportArrived(AuHengeTeleportAtom)),
				Qt::QueuedConnection);

		connect(this, SIGNAL(RecoverAtomFromLimbo(AuId)), m_tDb, SLOT(RecoverAtomFromLimbo(AuId)),
				Qt::QueuedConnection);
		connect(this, SIGNAL(ReloadData()), m_tDb, SLOT(ReloadData()),
				Qt::QueuedConnection);
		connect(this, SIGNAL(SectorAssignmentRevoked(AuId)), m_tDb, SLOT(SectorAssignmentRevoked(AuId)),
				Qt::QueuedConnection);
		connect(this, SIGNAL(SpawnAtomViaDb(AuId, quint32, AuId, AuId, AuId, quint8)),
				m_tDb, SLOT(SpawnAtom(AuId, quint32, AuId, AuId, AuId, quint8)));
		connect(this, SIGNAL(SystemAssignmentRevoked(AuId)), m_tDb, SLOT(SystemAssignmentRevoked(AuId)),
				Qt::QueuedConnection);
		connect(this, SIGNAL(TravelerArrivedViaDb(AuId, AuId, AuId)), m_tDb, SLOT(TravelerArrived(AuId, AuId, AuId)),
				Qt::QueuedConnection);
		connect(this, SIGNAL(TravelerDepartedViaDb(AuId, AuId, AuId, AuId)), m_tDb, SLOT(TravelerDeparted(AuId, AuId, AuId, AuId)),
				Qt::QueuedConnection);

		connect(this, SIGNAL(TravelerDepartedGalaxyLocViaDb(AuId, AuId, AuId, AuPoint)),
				m_tDb, SLOT(TravelerDepartedGalaxyLoc(AuId, AuId, AuId, AuPoint)),
				Qt::QueuedConnection);

		connect(this, SIGNAL(TravelerDepartedStargateViaDb(AuId, AuId, AuId, AuPoint)),
				m_tDb, SLOT(TravelerDepartedStargate(AuId, AuId, AuId, AuPoint)),
				Qt::QueuedConnection);

		connect(m_tDb, SIGNAL(ScDesignDeactivated(quint32)),
				SLOT(ScDesignDeactivatedUp(quint32)),
				Qt::QueuedConnection);

		connect(m_tDb, SIGNAL(ItDesignDeactivated(quint32)),
				SLOT(ItDesignDeactivatedUp(quint32)),
				Qt::QueuedConnection);

		connect(m_tDb, SIGNAL(BdDesignDeactivated(quint32)),
				SLOT(BdDesignDeactivatedUp(quint32)),
				Qt::QueuedConnection);

		m_tDb->start();
	}

	m_tMailClient = new DtMailClient(this);
	if(m_tMailClient)
		m_tMailClient->start();

	m_tTrash = new DtTrash(this);
	if(m_tTrash)
		m_tTrash->start();

	// Start atom managers. Tick/tock threads only needed on the scene worker.
	if(g_Au.IsSceneWorker())
	{
		if(!m_AtomManagerCount)
			m_AtomManagerCount = QThread::idealThreadCount();
		g_Au.SetAtomManagerDispatch(new AuAtomManagerDispatch(m_AtomManagerCount, this));
		m_tmBalance.start();
	}
}

void AuSceneServerApp::StopThreads()
{
	m_timer->stop();
	m_timer->deleteLater();

	// Shut down database first. We may need every remaining moment to save our data.
	if(m_tDb)
	{
		if(g_Au.m_LogMessages & LOG_Thread)
			qDebug("AuSceneServerApp::StopThreads - Stopping Db");

		// Disconnect 'finished' signal so DbFinishedInit doesn't get called.
		disconnect(m_tDb, SIGNAL(finished()));

		m_tDb->deleteLater();
		m_tDb->Kill();
		qDebug() << "Waiting indefinitely for the Database thread to finish." << m_tDb;
		m_tDb->wait();			// Wait indefinitely for the database thread to finish saving.
		m_tDb = 0;
		qDebug() << "Database thread finished.";
	}

	// Stop server channel threads.
	m_ServerChannel.StopThreads();

	// Closing the connection with our boss tells him we're gone. Our work can be delegated to someone else now its been saved.
	if(m_tClientChannel)
	{
		if(g_Au.m_LogMessages & LOG_Thread)
			qDebug("AuSceneServerApp::StopThreads - Stopping Client Channel");

		// Disconnect 'finished' signal so ClientChannelLost doesn't get called.
		disconnect(m_tClientChannel, SIGNAL(finished()));

		m_tClientChannel->deleteLater();
		m_tClientChannel->Kill();
	}

	// We won't be sending any more mail.
	if(m_tMailClient)
	{
		if(g_Au.m_LogMessages & LOG_Thread)
			qDebug("AuSceneServerApp::StopThreads - Stopping Mail Client");

		m_tMailClient->deleteLater();
		m_tMailClient->Kill();
	}

	if(m_tTrash)
	{
		if(g_Au.m_LogMessages & LOG_Thread)
			qDebug("AuSceneServerApp::StopThreads - Stopping Trash");

		m_tTrash->deleteLater();
		m_tTrash->Kill();
	}

	// Wait for threads to die.
	{
		AuElapsedTimer t;
		t.start();
		if(m_tClientChannel)
		{
			while(m_tClientChannel->isRunning())
			{
				if(t.elapsed() > 1000)
				{
					qDebug() << "Waiting for the Client Channel thread to die.";
					t.restart();
				}
			}
			m_tClientChannel = 0;
			if(g_Au.m_LogMessages & LOG_Thread)
				qDebug("AuSceneServerApp::StopThreads - Client Channel stopped");
		}

		if(m_tMailClient)
		{
			t.restart();
			while(m_tMailClient->isRunning())
			{
				if(t.elapsed() > 1000)
				{
					qDebug() << "Waiting for the Mail thread to die.";
					t.restart();
				}
			}
			m_tMailClient = 0;
			if(g_Au.m_LogMessages & LOG_Thread)
				qDebug("AuSceneServerApp::StopThreads - Mail Client stopped");
		}

		if(m_tTrash)
		{
			t.restart();
			while(m_tTrash->isRunning())
			{
				if(t.elapsed() > 1000)
				{
					qDebug() << "Waiting for the Trash thread to die.";
					t.restart();
				}
			}
			m_tTrash = 0;
			if(g_Au.m_LogMessages & LOG_Thread)
				qDebug("AuSceneServerApp::StopThreads - Trash stopped");
		}
	}

	// Discard my assignments and stop their threads.
	if(g_Au.AtomManagerDispatch())
	{
		if(g_Au.m_LogMessages & LOG_Thread)
			qDebug("AuSceneServerApp::StopThreads - Discarding assignments");

		delete g_Au.AtomManagerDispatch();
		g_Au.SetAtomManagerDispatch(0);

		if(g_Au.m_LogMessages & LOG_Thread)
			qDebug("AuSceneServerApp::StopThreads - Assignments discarded");
	}
}

void AuSceneServerApp::DbFinishedInit()
{
	switch(DbStatus())
	{
	case DtDb::NotLoaded:			// Database has not been opened yet.
		break;
	case DtDb::Loaded:				// Database opened ok.
	case DtDb::Storing:				// Database opened ok and currently storing.
		if(g_Au.m_LogMessages & LOG_Database)
			g_Au.Log(tr("Database Loaded"));
		if(g_Au.IsSceneWorker())
			g_AuDb.LoadScripts();	// Load and validate story scripts.
		StartSceneServer();
		break;
	case DtDb::OpenError:			// Database did not open ok.
		if(g_Au.m_LogMessages & LOG_Error)
			g_Au.Error(tr("Failed to Open Database"));
		break;
	case DtDb::DriverError:			// Database driver did not init ok.
		if(g_Au.m_LogMessages & LOG_Error)
			g_Au.Error(tr("Failed to Load Database Driver"));
		break;
	}
}

// Returns sum of average tick times of solar systems on worker server.
// Returns 0xffffffff when not ready to receive system assignments.
// Return value is in 1/2 millisecond increments. i.e. 100 = 50ms
quint32 AuSceneServerApp::TotalTickTime() const
{
	switch(DbStatus())
	{
	case DtDb::Loaded:				// Database opened ok.
	case DtDb::Storing:				// Database opened ok and currently storing.
		break;
	default:
		return 0xffffffff;			// Not ready for assignments.
	}

	unsigned int totalTime = g_Au.SumOfAverageTickTimes();
	return totalTime;
}

quint32 AuSceneServerApp::TotalTockTime() const
{
	switch(DbStatus())
	{
	case DtDb::Loaded:				// Database opened ok.
	case DtDb::Storing:				// Database opened ok and currently storing.
		break;
	default:
		return 0xffffffff;			// Not ready for assignments.
	}

	unsigned int totalTime = g_Au.SumOfAverageTockTimes();
	return totalTime;
}

// Returns the longest average tick time of the managed atoms.
// Return value is in 1/2 millisecond increments. i.e. 100 = 50ms
unsigned int AuSceneServerApp::MaxTickTime() const
{
	QHash<AuId, unsigned int> systemTotals;
	QHash<AuId, unsigned int> systemMax;
	QHash<AuId, unsigned int> systemCycle;
	QHash<AuId, unsigned int> systemCycleMax;
	TallySystemTickTotals(systemTotals, systemMax, systemCycle, systemCycleMax);

	unsigned int maxtime = 0;
	for(QHash<AuId, unsigned int>::iterator si = systemTotals.begin(); si != systemTotals.end(); si++)
		if(*si > maxtime)
			maxtime = *si;

	return maxtime;
}

Qt::HANDLE AuSceneServerApp::DbThreadId() const
{
	if(m_tDb)
		return m_tDb->ThreadId();
	Q_ASSERT(false);	// No db thread?
	return 0;
}

void AuSceneServerApp::StartSceneServer()
{
	QString logmsg;

	// TCP channel used to communicate with scene clients.
	quint16 scenePort = g_Au.ScenePort();
	if(m_TcpServer.listen(QHostAddress::Any, scenePort))
	{
		logmsg = tr("APP: %1 Server Scene channel listening on TCP port %2")
			.arg(g_Au.IsSceneMaster() ? "Master" : g_Au.IsSceneManager() ? "Manager" : "Worker").arg(scenePort);
		g_Au.LogConnect(LoginName(), logmsg);
	}
	else
	{
		if(g_Au.m_LogMessages & LOG_Error)
		{
			logmsg = tr("APP: %1 Server Scene channel failed to listen on TCP port %2")
				.arg(g_Au.IsSceneMaster() ? "Master" : g_Au.IsSceneManager() ? "Manager" : "Worker").arg(scenePort);
			g_Au.Error(logmsg);
		}
	}

	if(g_Au.IsSceneWorker())
	{
		// TCP channel used for chat communications is only created on the scene worker.
		quint16 chatPort = g_Au.ChatPort();
		if(m_ChatServer.listen(QHostAddress::Any, chatPort))
		{
			logmsg = tr("APP: Worker Server Chat channel listening on TCP port %2").arg(chatPort);
			g_Au.LogConnect(LoginName(), logmsg);
		}
		else
		{
			if(g_Au.m_LogMessages & LOG_Error)
			{
				logmsg = tr("APP: Worker Server Chat channel failed to listen on TCP port %2").arg(chatPort);
				g_Au.Error(logmsg);
			}
		}

		// UDP channel used to communicate with scene clients.
		// Only the worker listens for udp packets.
		if(m_UdpSocket.bind(QHostAddress::Any, scenePort))
		{
			connect(&m_UdpSocket, SIGNAL(readyRead()), SLOT(ReadPendingDatagrams()));
			logmsg = tr("APP: UDP Socket bound to port %1").arg(scenePort);
			g_Au.LogConnect(LoginName(), logmsg);
		}
		else
		{
			if(g_Au.m_LogMessages & LOG_Error)
			{
				logmsg = tr("APP: UDP Socket failed to bind to port %1").arg(scenePort);
				g_Au.Error(logmsg);
			}
		}
	}
}

void AuSceneServerApp::StartServerManager()
{
	// TCP channel used for server manager connections.
	quint16 mgrPort = g_Au.ManagerPort();
	if(m_MgrServer.listen(QHostAddress::Any, mgrPort))
	{
		QString logmsg = tr("APP: Server Manager channel listening on TCP port %2").arg(mgrPort);
		g_Au.LogConnect(LoginName(), logmsg);
	}
	else
	{
		if(g_Au.m_LogMessages & LOG_Error)
		{
			QString logmsg = tr("APP: Server Manager channel failed to listen on TCP port %2").arg(mgrPort);
			g_Au.Error(logmsg);
		}
	}
}

void AuSceneServerApp::StopSceneServer()
{
	// TCP channel used to communicate with scene clients.
	if(m_TcpServer.isListening())
	{
		m_TcpServer.close();
		QString logmsg = tr("APP: %1 Server Scene channel stopped listening on TCP port %2")
			.arg(g_Au.IsSceneMaster() ? "Master" : g_Au.IsSceneManager() ? "Manager" : "Worker").arg(g_Au.ScenePort());
		g_Au.LogConnect(LoginName(), logmsg);
	}

	// TCP channel used to communicate with scene clients.
	if(m_ChatServer.isListening())
	{
		m_ChatServer.close();
		QString logmsg = tr("APP: %1 Server Chat channel stopped listening on TCP port %2")
			.arg(g_Au.IsSceneMaster() ? "Master" : g_Au.IsSceneManager() ? "Manager" : "Worker").arg(g_Au.ChatPort());
		g_Au.LogConnect(LoginName(), logmsg);
	}

	// UDP channel used to communicate with scene clients.
	if(g_Au.IsSceneWorker())
	{
		m_UdpSocket.close();
		QString logmsg = tr("APP: UDP Socket unbound from port %1").arg(g_Au.ScenePort());
		g_Au.LogConnect(LoginName(), logmsg);
	}
}

void AuSceneServerApp::StopServerManager()
{
	// TCP channel used to communicate with scene clients.
	if(m_MgrServer.isListening())
	{
		m_MgrServer.close();
		QString logmsg = tr("APP: Server Manager channel stopped listening on TCP port %2").arg(g_Au.ManagerPort());
		g_Au.LogConnect(LoginName(), logmsg);
	}
}

// Server channel is used to communicate with subordinate worker servers.
// Subordinates will connect to boss via this comm channel.
void AuSceneServerApp::StartServerChannel()
{
	// The SceneWorker has no subordinates so it doesn't need this.
	if(g_Au.IsSceneMaster() || g_Au.IsSceneManager())
	{
		connect(&m_ServerChannel, SIGNAL(ConnectionClosed(unsigned int)), SLOT(ServerChannelClosed(unsigned int)));
		QString logmsg;
		if(m_ServerChannel.listen(QHostAddress::Any, g_Au.ServerPort()))
		{
			if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			{
				logmsg = tr("APP: Server Channel listening on TCP port %1").arg(g_Au.ServerPort());
				g_Au.LogConnect(LoginName(), logmsg);
			}
		}
		else
		{
			if(g_Au.m_LogMessages & LOG_Error)
			{
				logmsg = tr("APP: Server Channel failed to listen on TCP port %1").arg(g_Au.ServerPort());
				g_Au.Error(logmsg);
			}
		}
	}

	// NOTICE: AuServerStatus keys off the word "Preparing " as a marker to clear/restart the web page galactic chat.
	ServerBroadcast(tr("Preparing the depths of space."));
}

void AuSceneServerApp::StopServerChannel()
{
	if(m_ServerChannel.isListening())
	{
		m_ServerChannel.close();

		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		{
			QString logmsg = tr("APP: Server Channel stopped listening on TCP port %1").arg(g_Au.ServerPort());
			g_Au.LogConnect(LoginName(), logmsg);
		}
	}
}

void AuSceneServerApp::ReadPendingDatagrams()
{
	//qDebug("AuSceneServerFrame::ReadPendingDatagrams");
	while(m_UdpSocket.hasPendingDatagrams())
	{
		// Read the pending datagram.
		QByteArray datagram;
		datagram.resize(m_UdpSocket.pendingDatagramSize());
		QHostAddress sender;
		quint16 senderPort;
		unsigned int bytesrx = m_UdpSocket.readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

		// Peel off the outer envelope to determine which player should receive the packet.
		unsigned int idPlayer;
		QByteArray packet;
		QDataStream in(datagram);
		in.setVersion(QDataStream::Qt_4_1);
		in >> idPlayer;
		in >> packet;

		// Maintain comm statistics.
		m_UdpBytesRx += bytesrx;	// count of udp bytes received
		m_UdpPacketsRx++;			// count of udp packets received

		//emit UdpPacketReceived(idPlayer, packet);
		const unordered_map<unsigned int, DtSceneServer*>& servers = m_TcpServer.LockServerList(__FILE__, __LINE__);
		unordered_map<unsigned int, DtSceneServer*>::const_iterator it = servers.cbegin();
		while(it != servers.cend())
		{
			if((*it).second->isRunning() && (*it).second->PlayerId() == idPlayer)
			{
				// Found the scene server that is connected to the unit.
				(*it).second->UdpPacketReceived(idPlayer, packet);
				break;
			}
			it++;
		}
		m_TcpServer.UnlockServerList(__FILE__, __LINE__);
	}
}

void AuSceneServerApp::Shutdown()
{
	//if(g_Au.m_LogMessages & LOG_Thread)
		//qDebug("AuSceneServerApp::Shutdown - Stopping threads.");
	StopThreads();
	//qDebug("AuSceneServerApp::Shutdown - Quitting.");
	quit();
	//qDebug("AuSceneServerApp::Shutdown - Bye.");
}

void AuSceneServerApp::UpdateStats()
{
	//if(g_Au.IsSceneMaster())
	//{
		//if(m_ShutdownSeconds)
			//qDebug("AuSceneServerApp::UpdateStats shutdownSeconds %d", m_ShutdownSeconds);
		//else
			//qDebug("AuSceneServerApp::UpdateStats");
	//}
	if(m_ShutdownSeconds > 0 && g_Au.IsSceneMaster())
	{
		// This is the scene master and it has been ordered to shut down.
		// Broadcast a message to the server farm periodically then shutdown on the last second.
		m_ShutdownSeconds--;
		if(m_ShutdownSeconds > 0)
		{
			if(m_ShutdownSeconds > 60)
			{
				// Over a minute so announce every minute.
				if(m_ShutdownSeconds%60 == 0)
				{
					QString text = QString("%1 %2 MINUTES").arg(m_ShutdownWarn).arg(m_ShutdownSeconds/60);
					ServerBroadcast(text);
				}
			}
			else if(m_ShutdownSeconds < 10 || (m_ShutdownSeconds < 30 && m_ShutdownSeconds%5 == 0) || (m_ShutdownSeconds%10 == 0))
			{
				QString text = QString("%1 %2 SECONDS").arg(m_ShutdownWarn).arg(m_ShutdownSeconds);
				ServerBroadcast(text);
			}
		}
		else
		{
			ServerBroadcast(m_ShutdownHalt);
			Shutdown();
			return;
		}
	}

	// Release sector and solar system assignments after the connection with their assigned server has been lost.
	DeassignLostSectors();
	DeassignLostSolarSystems();

	// Assign sectors and solar systems that have been created and those that need reassignment.
	AssignSectors();
	AssignSolarSystems();

	// Open destination stargates that are waiting.
	OpenDelayedStargates();

	g_Au.DecayArenas();

	if(g_Au.IsSceneMaster())
	{
		// Scene master does nothing more here if no sectors have been assigned to managers. This prevents excess work during the
		// loading phase while starting up.
		m_AssignmentsLock.lock();
		bool noneAssigned = m_SectorAssignments.empty();
		m_AssignmentsLock.unlock();
		if(noneAssigned)
			return;
		PlayerStatusBroadcast();
	}
	else if(g_Au.IsSceneManager())
	{
		// Scene manager does nothing more here if no solar systems have been assigned to workers. This prevents excess work
		// during the loading phase while starting up.
		m_AssignmentsLock.lock();
		bool noneAssigned = m_SystemAssignments.empty();
		m_AssignmentsLock.unlock();
		if(noneAssigned)
			return;
	}
	else if(g_Au.IsSceneWorker())
	{
		if(DbStatus() == DtDb::Loaded)
		{
			// We would expect to be listening on the udp socket now.
			if(m_UdpSocket.state() == QAbstractSocket::BoundState)
			{
				if(m_UdpSocket.hasPendingDatagrams())
				{
					//qDebug() << "AuSceneServerApp::UpdateStats hasPendingDatagrams, calling ReadPendingDatagrams";
					ReadPendingDatagrams();
				}
			}
			else
				qDebug() << "AuSceneServerApp::UpdateStats UDP socket not bound" << m_UdpSocket.state();
		}

		DeactivateAvatars();
	}

	// Signal atom departures.
	SendDepartingAtoms();

	//if(g_Au.IsSceneMaster())
		//qDebug("AuSceneServerApp::UpdateStats Chat Maintenance");
	// Open channels. Send atom chat messages. Decay atom chat messages.
	AtomChat();

	// Decay atom comm connections.
	g_Au.DecayChannelMonitors();

	// Handoff and spawn atoms that have received atom query responses about desired destination atoms.
	HandoffAtoms();
	SpawnAtoms();
	HengeTeleportAtoms();

	if(!g_Au.IsSceneWorker())
	{
		// Balance the load between subordinate servers.
		const int balanceInterval = 60 * 10;
		if(!m_tmBalance.isValid() || m_tmBalance.elapsed()/d_MSecPerSec > balanceInterval)
		{
			//if(g_Au.IsSceneMaster())
				//qDebug("AuSceneServerApp::UpdateStats Balance Loads");
			BalanceLoads();
			m_tmBalance.restart();
		}
	}
	else
	{
		if(!m_tmBalance.isValid() || m_tmBalance.elapsed() >= d_MSecPerSec)
			AtomManagerLoadReset();

		if(!m_tmUnload.isValid() || m_tmUnload.elapsed()/d_MSecPerMin >= 5)
		{
			UnloadUnusedData();
			m_tmUnload.restart();
		}
	}
}

void AuSceneServerApp::HandleSignal(int value)
{
	switch(value)
	{
#if !defined(Q_OS_WIN)
	case SIGHUP:
		g_Au.Log("Hangup signal ignored.");
		return;	// Ignore hangup signal.

	case SIGPIPE:
		g_Au.Log("Broken pipe signal ignored.");
		return;	// Ignore broken pipe signal.

	case SIGQUIT:
		g_Au.Log("Terminal quit signal caught. Shutting down...");
		break;
#endif

	case SIGINT:
		g_Au.Log("Terminal interrupt signal caught. Shutting down...");
		break;

	case SIGTERM:
		g_Au.Log("Termination signal caught. Shutting down...");
		break;
	}

	if(g_Au.IsSceneMaster())
	{
		if(m_ShutdownSeconds)
		{
			// We are currently shutting down.
			// Take the second signal to mean we are not supposed to shut down.
			Shutdown("", "SERVER SHUTDOWN HALTED!", -1);

			// Resume listening for incoming connections since we are no longer shutting down.
			//StartSceneServer();
			return;
		}

		// Contents of shutdown.conf file should look like this.
		// The file should be in the same folder as the application.
		// 180
		// ATTENTION! SERVERS ARE SHUTTING DOWN.
		// <><><><><> SERVERS ARE SHUTTING DOWN NOW! <><><><><>
		bool storeSettings = false;
		QString warnMsg, haltMsg;
		qint16 shutdownSecs;
		QFile confFile("shutdown.conf");
		if(confFile.open(QIODevice::ReadOnly))
		{
			QTextStream in(&confFile);
			QString time;
			shutdownSecs = QVariant(in.readLine()).toInt();
			warnMsg = in.readLine();
			haltMsg = in.readLine();
			confFile.close();

			// Set defaults for blank shutdown messages.
			if(shutdownSecs < 0)
			{
				g_Au.Warning("Invalid seconds read from shutdown.conf file. Using default seconds.");
				shutdownSecs = 180;
				storeSettings = true;
			}

			if(warnMsg.isEmpty())
			{
				g_Au.Warning("Failed to read warning message from shutdown.conf file. Using default warning message.");
				warnMsg = "ATTENTION! SERVERS ARE SHUTTING DOWN.";
				storeSettings = true;
			}

			if(haltMsg.isEmpty())
			{
				g_Au.Warning("Failed to read halt message from shutdown.conf file. Using default halt message.");
				haltMsg = "<><><><><> SERVERS ARE SHUTTING DOWN NOW! <><><><><>";
				storeSettings = true;
			}
		}
		else
		{
			// No conf file so use default shutdown time.
			g_Au.Warning("Failed to read shutdown.conf file. Using default shutdown parameters.");
			shutdownSecs = 180;
			warnMsg = "ATTENTION! SERVERS ARE SHUTTING DOWN.";
			haltMsg = "<><><><><> SERVERS ARE SHUTTING DOWN NOW! <><><><><>";
			storeSettings = true;
		}

		if(storeSettings)
		{
			// Store values to the shutdown.conf file.
			if(confFile.open(QIODevice::WriteOnly))
			{
				g_Au.Warning("Storing updated shutdown parameters to the shutdown.conf file.");
				QTextStream out(&confFile);
				out << shutdownSecs << '\n';
				out << warnMsg << '\n';
				out << haltMsg << '\n';
				confFile.close();
			}
			else
				g_Au.Warning("Failed to open shutdown.conf file. Unable to store updated shutdown parameters.");
		}

		if(shutdownSecs >= 0)
		{
			// Initiate timed shutdown sequence.
			Shutdown(warnMsg, haltMsg, shutdownSecs);

			// Stop listening for incoming connections waiting to shut down.
			//StopSceneServer();
		}
	}

	if(m_ShutdownSeconds <= 0)
		Shutdown();												// Shutdown time is immediate.

	// NOTE: Debug output reveals an occasional segmentation fault right now. Everything
	//       in my world is clean so it's not a critical problem but it would be nice to
	//       know the real cause. Destructor in an object?
}

// Scene worker returns the number of client connections.
unsigned int AuSceneServerApp::CountSubServers() const
{
	if(g_Au.IsSceneWorker())
	{
		const unordered_map<unsigned int, DtSceneServer*>& servers = m_TcpServer.LockServerList(__FILE__, __LINE__);
		int count = servers.size();
		m_TcpServer.UnlockServerList(__FILE__, __LINE__);
		return count;
	}
	else
	{
		const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
		int count = servers.size();
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
		return count;
	}
}

// Called on scene worker to get count of clients by system id.
// system id, # of clients
void AuSceneServerApp::CollectSystemStatus(unordered_map<AuId, AuSystemStatus, AuIdHash>& systems)
{
	if(g_Au.IsSceneWorker())
	{
		systems.clear();

		// Init the map with all of the system ids that are assigned to me, setting the client count to 0.
		QHash<AuId, unsigned int> systemTotals;
		g_Au.TallyAtomTockerSystemStatus(systemTotals);	// Status only, no lock checks.

		for(QHash<AuId, unsigned int>::iterator si = systemTotals.begin(); si != systemTotals.end(); si++)
		{
			AuSystemStatus status;
			status.m_Tick = *si;	// This value is in 1/2 millisecond increments.
			systems.insert(pair<AuId, AuSystemStatus>(si.key(), status));
		}

		// Bump the counts for the systems that have clients.
		m_TcpServer.SystemClients(systems);
	}
}

DhSceneServer AuSceneServerApp::DeferSceneToManager() const
{
	// Select my subordinate manager server with the lowest number of
	// subordinate scene workers.
	DhSceneServer toServer;
	quint16 leastWorkers = 0xffff;
	const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
	for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = servers.cbegin(); it != servers.cend(); it++)
	{
		if((*it).second->isFinished())
			continue;
		if((*it).second->IsSceneManager() && (*it).second->SubWorkers() < leastWorkers)
		{
			toServer.m_Id = (*it).second->Id();
			toServer.m_NetName = (*it).second->InternetName();
			toServer.m_NetPort = (*it).second->InternetPort();
			toServer.m_PeerName = (*it).second->PeerAddress();
			toServer.m_CheckIn = (*it).second->CheckInTime();
			leastWorkers = (*it).second->SubWorkers();
			if(!leastWorkers)
				break;
		}
	}
	m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
	return toServer;
}

DhSceneServer AuSceneServerApp::AssignSectorToManager(int which, unsigned int avoidServerId)
{
	//Q_ASSERT(g_Au.IsSceneMaster());
	// Assignments often occur in bursts. Rather than assign all of them to the
	// one least loaded server, distribute them across several servers.
	DhSceneServer toServer;
	m_ServerChannel.LockServerList(__FILE__, __LINE__);
	DtServerChannel* server = m_ServerChannel.LeastLoadedServer(1, which, avoidServerId);
	if(server)
	{
		toServer.m_Id = server->Id();
		toServer.m_NetName = server->InternetName();
		toServer.m_NetPort = server->InternetPort();
		toServer.m_PeerName = server->PeerAddress();
		toServer.m_CheckIn = server->CheckInTime();
	}
	m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
	return toServer;
}

DhSceneServer AuSceneServerApp::AssignSolarSystemToWorker(int which, unsigned int avoidServerId, unsigned int specificServerId)
{
	//Q_ASSERT(g_Au.IsSceneManager());
	// Assignments often occur in bursts. Rather than assign all of them to the
	// one least loaded server, distribute them across several servers.
	DhSceneServer toServer;
	m_ServerChannel.LockServerList(__FILE__, __LINE__);
	DtServerChannel* server = 0;
	if(specificServerId)
		server = m_ServerChannel.ServerChannel(specificServerId);
	if(!server)
		server = m_ServerChannel.LeastLoadedServer(2, which, avoidServerId);
	if(server)
	{
		toServer.m_Id = server->Id();
		toServer.m_NetName = server->InternetName();
		toServer.m_NetPort = server->InternetPort();
		toServer.m_PeerName = server->PeerAddress();
		toServer.m_CheckIn = server->CheckInTime();
	}
	m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
	return toServer;
}

bool AuSceneServerApp::SendPacketTo(const QByteArray& ba, unsigned int serverId)
{
	m_ServerChannel.LockServerList(__FILE__, __LINE__);
	DtServerChannel* channel = m_ServerChannel.ServerChannel(serverId);
	if(channel)
	{
		channel->SendPacket(ba);
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
		return true;
	}
	m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
	return false;
}

// Transfer departing atoms after their destination sector has been assigned.
void AuSceneServerApp::SendDepartingAtoms()
{
	if(g_Au.IsSceneMaster())
	{
		// Scene master propagates atom transfers down to the manager responsible for the destination sector.
		m_muPendingDep.lock();
		bool sendingAny = false;
		if(!m_PendingDepartures.isEmpty())
		{
			if(g_Au.m_LogMessages & LOG_Travel)
				qDebug("AuSceneServerApp::SendDepartingAtoms %d atoms departing", m_PendingDepartures.count());
			sendingAny = true;
		}
		QMap<AuId, AuAtomDeparture>::iterator it = m_PendingDepartures.begin();
		while(it != m_PendingDepartures.end())
		{
			DhSceneServer toServer = SectorServer((*it).m_toSector);
			if(toServer.IsValid())
			{
				AuEther& ether = g_Au.LockEtheriumDelete();
				DaUniverse* universe = ether.FindUniverse();
				if(universe)
				{
					// toServer is valid so we are assured TransferAtomDown2 will send a packet to that server.
					// NOTE: TransferAtomDown2 will not lock m_muPendingDep because we have already tested toServer.IsValid().
					AtomTraveledDown2(it.key(), (*it).m_toSolarSystem, (*it).m_toSector, universe);
					//TransferAtomDown2(it.key(), (*it).m_toSolarSystem, (*it).m_toSector, (*it).m_AtomData, universe);
					it = m_PendingDepartures.erase(it);
				}
				else
					it++;
				g_Au.UnlockEtheriumDelete();
			}
			else
				it++;
		}
		if(sendingAny && (g_Au.m_LogMessages & LOG_Travel))
			qDebug("AuSceneServerApp::SendDepartingAtoms done, %d atoms still departing", m_PendingDepartures.count());
		m_muPendingDep.unlock();
	}
	else if(g_Au.IsSceneManager())
	{
		// Scene manager propagates atom transfers down to the worker responsible for the destination solar system.
		m_muPendingDep.lock();
		QMap<AuId, AuAtomDeparture>::iterator it = m_PendingDepartures.begin();
		while(it != m_PendingDepartures.end())
		{
			if(IsSectorAssignedToMe((*it).m_toSector))
			{
				DhSceneServer toServer = SolarSystemServer((*it).m_toSolarSystem);
				if(toServer.IsValid())
				{
					AuEther& ether = g_Au.LockEtheriumDelete();
					DaSector* sector = dynamic_cast<DaSector*>(ether.FindNormalAtom((*it).m_toSector));
					if(sector)
					{
						// toServer is valid so we are assured TransferAtomDown2 will send a packet to that server.
						// NOTE: TransferAtomDown2 will not lock m_muPendingDep because we have already tested toServer.IsValid().
						AtomTraveledDown2(it.key(), (*it).m_toSolarSystem, (*it).m_toSector, sector);
						//TransferAtomDown2(it.key(), (*it).m_toSolarSystem, (*it).m_toSector, (*it).m_AtomData, sector);
						it = m_PendingDepartures.erase(it);
					}
					else
						it++;
					g_Au.UnlockEtheriumDelete();
				}
				else
					it++;
			}
			else
			{
				// The destination sector is no longer assigned to me. Send this notification back up to the master.
				AtomTraveledUp2(it.key(), (*it).m_toSolarSystem, (*it).m_toSector, 0);
				//TransferAtomUp2(it.key(), (*it).m_toSolarSystem, (*it).m_toSector, (*it).m_AtomData, 0);
				it = m_PendingDepartures.erase(it);
			}
		}
		m_muPendingDep.unlock();
	}
}

void AuSceneServerApp::AssignSectors()
{
	// Only scene master does this.
	if(!g_Au.IsSceneMaster())
		return;

	// Master waits for database to be completely loaded before assigning any sectors.
	// Master does not assign sectors while database is Storing. It might be storing the sectors we are assigning.
	if(DbStatus() != DtDb::Loaded)
		return;

	// Can't delegate sectors unless we have subordinate managers.
	if(m_ServerChannel.CountManagerServers() == 0)
		return;

	// Do not assign sectors while we have atoms in the add or change list.
	// Without more searching, this is the best way to be sure all relevant
	// adds and changes are complete before assigning a sector.
	bool hasAddOrChg = g_AuDb.HasAtomAddsOrChanges();
	if(hasAddOrChg)
		return;

	m_AssignmentsLock.lock();
	bool noneAssigned = m_SectorAssignments.empty();
	m_AssignmentsLock.unlock();

	// Get a list of sectors that can be assigned. We do not assign sectors that form the perimeter of explored space. In other
	// words, we do not assign sectors unless all 26 neighboring sectors exist. Perimeter sectors do not contain any people or
	// facilities so they don't need to be ticked or otherwise managed by anyone but the scene master. This also enables the scene
	// worker to expect the neighboring sectors of all solar systems assigned to exist in the database for the star field.
	unordered_map<AuId, AuMySector, AuIdHash> idSectorsToAssign;
	DaAtomList galaxies;
	AuEther& ether = g_Au.LockEtheriumDelete();
	ether.ListGalaxies(galaxies);
	for(DaAtomList::const_iterator gi = galaxies.cbegin(); gi != galaxies.cend(); gi++)
	{
		const DaGalaxy* galaxy = dynamic_cast<const DaGalaxy*>(*gi);
		if(galaxy)
		{
			unordered_map<AuId, int, AuIdHash> galaxySectorsToAssign;
			galaxy->ListAssignableSectorIds(galaxySectorsToAssign);

			for(unordered_map<AuId, int, AuIdHash>::const_iterator gsi = galaxySectorsToAssign.cbegin();
					gsi != galaxySectorsToAssign.cend(); gsi++)
				idSectorsToAssign.insert(pair<AuId, AuMySector>((*gsi).first, AuMySector(galaxy->Galaxy(), (*gsi).second)));
		}
	}
	g_Au.UnlockEtheriumDelete();
	if(idSectorsToAssign.empty())
		return;			// No sectors have been loaded or created yet.

	// Revoke sectors that are assigned but are no longer in the assignable sectors list.
	// This could happen as a result of sector decay.
	AuIdList revokeSectorIds;
	m_AssignmentsLock.lock();
	for(unordered_map<AuId, AuSectorAssignment, AuIdHash>::const_iterator it = m_SectorAssignments.cbegin();
			it != m_SectorAssignments.cend(); it++)
		if(idSectorsToAssign.find((*it).first) == idSectorsToAssign.cend())
			revokeSectorIds.append((*it).first);
	m_AssignmentsLock.unlock();
	if(!revokeSectorIds.isEmpty())
	{
		if(g_Au.m_LogMessages & LOG_LoadBalance)
			qDebug() << "AuSceneServerApp::AssignSectors" << revokeSectorIds.count() << "sectors are no longer assignable.";
		for(AuIdList::const_iterator ri = revokeSectorIds.cbegin(); ri != revokeSectorIds.cend(); ri++)
			RevokeAssignment(*ri);
	}

	// Remove sector ids that have already been assigned or that are nu and haven't been stored in the database yet.
	m_rwNewSectors.lockForRead();
	unordered_map<AuId, AuMySector, AuIdHash>::iterator stai = idSectorsToAssign.begin();
	while(stai != idSectorsToAssign.end())
	{
		if(IsSectorAssigned((*stai).first) || m_NewSectors.contains((*stai).first))
			stai = idSectorsToAssign.erase(stai);
		else
			stai++;
	}
	m_rwNewSectors.unlock();

	// Assign sector ids remaining in list, the ones that can be assigned and haven't been assigned yet.
	int which = 0;
	for(unordered_map<AuId, AuMySector, AuIdHash>::const_iterator it = idSectorsToAssign.cbegin(); it != idSectorsToAssign.cend();
			it++)
	{
		// Determine if it was revoked from a server.
		unsigned int avoidServerId = 0;
		unordered_map<AuId, unsigned int, AuIdHash>::iterator ri = m_Revokations.find((*it).first);
		if(ri != m_Revokations.end())
		{
			avoidServerId = (*ri).second;
			m_Revokations.erase(ri);
		}

		// Create the assignment record.
		AuSectorAssignment assign;
		assign.m_toServer = AssignSectorToManager(which, avoidServerId);
		which++;
		if(assign.m_toServer.IsValid())
		{
			assign.m_solarSystems = (*it).second.m_Systems;
			m_AssignmentsLock.lock();
			m_SectorAssignments.insert(pair<AuId, AuSectorAssignment>((*it).first, assign));
			m_AssignmentsLock.unlock();

			if(g_Au.m_LogMessages & LOG_LoadBalance)
			{
				QString logmsg;
				logmsg = tr("Assigned sector %1 with %3 solar systems to manager %2 %4:%5")
					.arg((*it).first.GetID()).arg(assign.m_toServer.m_Id).arg(assign.m_solarSystems)
					.arg(assign.m_toServer.m_NetName).arg(assign.m_toServer.m_NetPort);
				g_Au.Log(logmsg);
			}

			// Notify the scene manager of the assignment.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_SectorAssigned;
			out << (*it).first;
			out << (quint8)(*it).second.m_Systems;						// Number of systems in the sector.
			out << (quint8)(*it).second.m_Galaxy;						// Which galaxy.
			SendPacketTo(ba, assign.m_toServer.m_Id);

			// Check for spawns on planets that are waiting for this nu sector to be assigned.
			m_SpawnMutex.lock();
			unordered_map<AuId, AuSpawnDeparture, AuIdHash>::iterator dsi = m_DepartingSpawnsOnPlanets.begin();
			while(dsi != m_DepartingSpawnsOnPlanets.end())
			{
				if((*dsi).second.m_SectorOrSystemId == (*it).first)
				{
					// A spawning atom is waiting for the sector to be assigned.
					SpawnPersonOnPlanet3((*dsi).first, (*dsi).second.m_SectorOrSystemId, (*dsi).second.Galaxy());
					dsi = m_DepartingSpawnsOnPlanets.erase(dsi);
				}
				else
					dsi++;
			}
			m_SpawnMutex.unlock();
		}
	}
	//qDebug("AuSceneServer::AssignSectors finished");

	m_AssignmentsLock.lock();
	bool anyAssigned = !m_SectorAssignments.empty();
	m_AssignmentsLock.unlock();

	if(noneAssigned && anyAssigned)
	{
		// No sectors were assigned on entry. Now sectors are assigned. Post a message that will appear on the web site.
		ServerBroadcast(tr("Commencing the course of time."));
	}
}

void AuSceneServerApp::AssignSolarSystems()
{
	// Only scene manager does this.
	if(!g_Au.IsSceneManager())
		return;

	// Manager waits for database to be completely loaded before assigning any solar system.
	// Manager does not assign solar system while database is Storing. It might be storing the solar system we are assigning.
	if(DbStatus() != DtDb::Loaded)
		return;

	// Can't delegate solar systems unless we have subordinate workers.
	int workerServers = m_ServerChannel.CountWorkerServers();
	if(!workerServers)
		return;

	// Do not assign systems while we have atoms in the add or change list.
	// Without more searching, this is the best way to be sure all relevant
	// adds and changes are complete before assigning a solar system.
	bool hasAddOrChg = g_AuDb.HasAtomAddsOrChanges();
	if(hasAddOrChg)
		return;

	DaAtomList galaxies;
	AuEther& ether = g_Au.LockEtheriumDelete();
	ether.ListGalaxies(galaxies);
	if(galaxies.empty())
	{
		g_Au.UnlockEtheriumDelete();
		return;
	}

	QMap<AuId,AuMySystem> idSystemsToAssign;							// Key = systemids to assign, value = sectorids
	for(DaAtomList::const_iterator gi = galaxies.cbegin(); gi != galaxies.cend(); gi++)
	{
		// Get a list of all the sectors stored locally. This will only include the sectors that have been assigned to this server.
		// The scene manager does not load any sectors except those assigned to it.
		const DaGalaxy* galaxy = dynamic_cast<const DaGalaxy*>(*gi);
		if(!galaxy)
			continue;

		DaAtomList sectors;
		galaxy->ListSectors(sectors);

		// Assign solar systems that haven't been assigned yet.
		for(DaAtomList::const_iterator it = sectors.cbegin(); it != sectors.cend(); it++)
		{
			const DaSector* sector = (const DaSector*)(*it);
			DaAtomList systems;
			sector->ListAssignableSolarSystems(systems);
			for(DaAtomList::const_iterator sit = systems.cbegin(); sit != systems.cend(); sit++)
			{
				AuId idSystem = (*sit)->Id();
				if(!IsSolarSystemAssigned(idSystem))
				{
					// The solar system hasn't been assigned to anyone.
					idSystemsToAssign.insert(idSystem, AuMySystem(galaxy->Galaxy(), sector->Id()));
				}
			}
		}
	}
	g_Au.UnlockEtheriumDelete();

	// Create the assignment records.
	for(QMap<AuId,AuMySystem>::const_iterator it = idSystemsToAssign.cbegin(); it != idSystemsToAssign.cend(); it++)
	{
		// Determine if it was revoked from a server.
		unsigned int avoidServerId = 0;
		unordered_map<AuId, unsigned int, AuIdHash>::iterator ri = m_Revokations.find(it.key());
		if(ri != m_Revokations.end())
		{
			avoidServerId = (*ri).second;
			m_Revokations.erase(ri);
		}
		unsigned int assignServerId = 0;
		unordered_map<AuId, unsigned int, AuIdHash>::iterator ai = m_Reassignments.find(it.key());
		if(ai != m_Reassignments.end())
		{
			assignServerId = (*ai).second;
			m_Reassignments.erase(ai);
		}

		int which = g_Au.Dice().Roll(1, workerServers, -1);		// Select a server index at random for better distribution.
		AuSystemAssignment assign;
		assign.m_toServer = AssignSolarSystemToWorker(which, avoidServerId, assignServerId);
		if(assign.m_toServer.IsValid())
		{
			m_AssignmentsLock.lock();
			m_SystemAssignments.insert(pair<AuId, AuSystemAssignment>(it.key(), assign));
			m_AssignmentsLock.unlock();

			if(g_Au.m_LogMessages & LOG_LoadBalance)
			{
				QString logmsg;
				logmsg = tr("Assigned solar system %1 to worker %2 %3:%4").arg(it.key().GetID()).arg(assign.m_toServer.m_Id)
					.arg(assign.m_toServer.m_NetName).arg(assign.m_toServer.m_NetPort);
				g_Au.Log(logmsg);
			}

			// Notify the scene worker of the assignment.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_SystemAssigned;
			out << (*it).m_Sector;			// Sector id
			out << it.key();				// Solar system id
			out << (quint8)(*it).Galaxy();
			SendPacketTo(ba, assign.m_toServer.m_Id);

			// Check for spawns on planets that are waiting for this nu solar system to be assigned.
			m_SpawnMutex.lock();
			unordered_map<AuId, AuSpawnDeparture, AuIdHash>::iterator dsi = m_DepartingSpawnsOnPlanets.begin();
			while(dsi != m_DepartingSpawnsOnPlanets.end())
			{
				if((*dsi).second.m_SectorOrSystemId == it.key())
				{
					// A spawning atom is waiting for the solar system to be assigned.
					SpawnPersonOnPlanet3((*dsi).first, (*dsi).second.m_SectorOrSystemId, (*dsi).second.Galaxy());
					dsi = m_DepartingSpawnsOnPlanets.erase(dsi);
				}
				else
					dsi++;
			}
			m_SpawnMutex.unlock();
		}
	}
}

// Returns true if a solar system is assigned.
bool AuSceneServerApp::IsSolarSystemAssigned(const AuId& idSolarSystem)
{
	m_AssignmentsLock.lock();
	bool isAssigned = m_SystemAssignments.find(idSolarSystem) != m_SystemAssignments.cend();
	m_AssignmentsLock.unlock();
	return isAssigned;
}

// Returns true if a sector is assigned.
bool AuSceneServerApp::IsSectorAssigned(const AuId& idSector)
{
	m_AssignmentsLock.lock();
	bool isAssigned = m_SectorAssignments.find(idSector) != m_SectorAssignments.cend();
	m_AssignmentsLock.unlock();
	return isAssigned;
}

// Scene master returns the number of sectors assigned to a sub server.
// Scene manager returns the number of solar systems assigned to a sub server.
unsigned int AuSceneServerApp::CountAssignedTo(unsigned int serverId)
{
	unsigned int countAssigned = 0;
	if(g_Au.IsSceneMaster())
	{
		m_AssignmentsLock.lock();
		for(unordered_map<AuId, AuSectorAssignment, AuIdHash>::const_iterator it = m_SectorAssignments.cbegin();
				it != m_SectorAssignments.cend(); it++)
			if((*it).second.m_toServer.m_Id == serverId)
				countAssigned++;
		m_AssignmentsLock.unlock();
	}
	else if(g_Au.IsSceneManager())
	{
		m_AssignmentsLock.lock();
		for(unordered_map<AuId, AuSystemAssignment, AuIdHash>::const_iterator it = m_SystemAssignments.cbegin();
				it != m_SystemAssignments.cend(); it++)
			if((*it).second.m_toServer.m_Id == serverId)
				countAssigned++;
		m_AssignmentsLock.unlock();
	}
	return countAssigned;
}

// Returns server of sector.
DhSceneServer AuSceneServerApp::SectorServer(const AuId& idSector)
{
	DhSceneServer toServer;
	m_AssignmentsLock.lock();
	if(m_SectorAssignments.find(idSector) != m_SectorAssignments.cend())
	{
		toServer = m_SectorAssignments[idSector].m_toServer;
		//qDebug("AuSceneServerApp::SectorServerName - "
				//"Sector %d assigned to %s",
				//idSector.GetID(), (const char*)toServer);
	}
	//else
	//{
		//qDebug("AuSceneServerApp::SectorServerName - Sector %d not assigned",
				//idSector.GetID());
	//}
	m_AssignmentsLock.unlock();
	return toServer;
}

// Returns server of solar system.
DhSceneServer AuSceneServerApp::SolarSystemServer(const AuId& idSolarSystem)
{
	DhSceneServer toServer;
	m_AssignmentsLock.lock();
	if(m_SystemAssignments.find(idSolarSystem) != m_SystemAssignments.cend())
	{
		toServer = m_SystemAssignments[idSolarSystem].m_toServer;
		//qDebug("AuSceneServerApp::SolarSystemServerName - SolarSystem %d assigned to "
				//"%s", idSolarSystem.GetID(), (const char*)toServer);
	}
	//else
	//{
		//qDebug("AuSceneServerApp::SolarSystemServerName - Solar System %d not assigned",
				//idSolarSystem.GetID());
	//}
	m_AssignmentsLock.unlock();
	return toServer;
}

// A sector has been assigned to me by my boss server.
void AuSceneServerApp::SectorAssigned(AuId idSector, int systemsInSector, quint8 galaxy)
{
	// Sectors are only assigned to the SceneManager.
	if(!g_Au.IsSceneManager())
		return;

	if(g_Au.m_LogMessages & LOG_LoadBalance)
	{
		QString logmsg;
		logmsg = tr("Received assignment, Sector %1 with %2 systems.").arg(idSector.GetID()).arg(systemsInSector);
		g_Au.Log(logmsg);
	}

	m_rwAtomManagers.lockForWrite();
	if(m_MySectors.find(idSector) == m_MySectors.cend())
		m_MySectors.insert(pair<AuId, AuMySector>(idSector, AuMySector((GALAXY)galaxy, systemsInSector)));
	m_rwAtomManagers.unlock();
}

void AuSceneServerApp::SectorRevoked(AuId idSector)
{
	// Sectors are only assigned to the SceneManager.
	if(!g_Au.IsSceneManager())
		return;

	// Remove the assignment from my list.
	m_rwAtomManagers.lockForWrite();
	unordered_map<AuId, AuMySector, AuIdHash>::iterator si = m_MySectors.find(idSector);
	if(si != m_MySectors.end())
		m_MySectors.erase(si);
	m_rwAtomManagers.unlock();

	// Trigger updates asap by the database thread.
	// The database thread will remove sectors that are no longer assigned after the next updates are posted.
	emit SectorAssignmentRevoked(idSector);
	g_AuDb.PostCriticalUpdates();

	if(g_Au.m_LogMessages & LOG_LoadBalance)
	{
		QString logmsg;
		logmsg = tr("Received revokation of assignment, Sector %1").arg(idSector.GetID());
		g_Au.Log(logmsg);
	}
}

// A solar system has been assigned to me by my boss server.
void AuSceneServerApp::SystemAssigned(AuId idSector, AuId idSystem, quint8 galaxy)
{
	// Solar systems are only assigned to the SceneWorker.
	if(!g_Au.IsSceneWorker())
		return;

	if(g_Au.m_LogMessages & LOG_LoadBalance)
	{
		QString logmsg;
		logmsg = tr("Received assignment, Solar System %1").arg(idSystem.GetID());
		g_Au.Log(logmsg);
	}

	m_rwAtomManagers.lockForWrite();
	if(m_MySystems.find(idSystem) == m_MySystems.end())			// true if the system is already assigned to me.
		m_MySystems.insert(pair<AuId, AuAtomManaged*>(idSystem, new AuAtomManaged(idSystem, idSector, (GALAXY)galaxy)));
	m_rwAtomManagers.unlock();
}

void AuSceneServerApp::SystemRevoked(AuId idSystem)
{
	// Solar systems are only assigned to the SceneWorker.
	if(!g_Au.IsSceneWorker())
		return;

	// Remove the assignment from my list. This will stop it from being ticked/tocked.
	m_rwAtomManagers.lockForWrite();
	unordered_map<AuId, AuAtomManaged*, AuIdHash>::iterator msi = m_MySystems.find(idSystem);
	if(msi != m_MySystems.end())
	{
		delete (*msi).second;
		m_MySystems.erase(msi);
	}
	m_rwAtomManagers.unlock();

	// Mark the solar system as loaded for star field purposes only.
	// After the next save of updates, the database thread will remove the atoms that don't belong in the star field.
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(idSystem));
	if(system)
	{
		system->SetStarFieldFlag(true);
		system->TickTockRecursive();	// Release Tick and Tock queue objects.
	}
	g_Au.UnlockEtheriumDelete();

	// Trigger updates asap by the database thread.
	// The database thread will remove atoms that don't belong in the star field after the next updates are posted.
	emit SystemAssignmentRevoked(idSystem);
	g_AuDb.PostCriticalUpdates();

	if(g_Au.m_LogMessages & LOG_LoadBalance)
	{
		QString logmsg;
		logmsg = tr("Received revokation of assignment, Solar System %1").arg(idSystem.GetID());
		g_Au.Log(logmsg);
	}
}

// Posts a query to locate the subordinate server who manages a region of space that contains the atom idAtom.
void AuSceneServerApp::QueryAtomServer(AuId idAtom, quint32 empire, AuId atomToSpawn)
{
	if(g_Au.m_LogMessages & LOG_AtomQuery)
	{
		QString logmsg = QString("AuSceneServerApp::QueryAtomServer atom %d empire %d atomToSpawn %d")
			.arg(idAtom.GetID())
			.arg(empire)
			.arg(atomToSpawn.GetID());
		g_Au.Log(logmsg);
	}
	if(g_Au.IsSceneWorker())
	{
		// Scene Worker checks its local atoms for the atom.
		quint8 result = 0;
		AuEther& ether = g_Au.LockEtheriumDelete();
		const DaAtom* pAtom = ether.FindNormalAtom(idAtom);
		if(pAtom)
		{
			// Ignore atom if it's in a solar system that is loaded for star field only.
			const DaSolarSystem* system = pAtom->FindParentSolarSystem();
			if(!system || system->IsOnlyStarFieldLoaded() || !IsSolarSystemAssignedToMe(system->Id()))
			{
				// Parent solar system is not found or it is not assigned to this worker server.
				pAtom = 0;
			}
			else
			{
				// Ignore person if they're waiting to spawn.
				const DaPerson* person = dynamic_cast<const DaPerson*>(pAtom);
				if(person && person->IsAwaitingSpawn())
					pAtom = 0;
			}
		}

		result = pAtom ? 1 : 0;	// 1=yes, 0=no, 2=not available
		if(pAtom && !atomToSpawn.IsNull())
		{
			// A spawn point is sought, either a spacecraft or a building.
			// 'empire' may be zero when a free avatar is attempting to respawn at their tied building.
			const DaVessel* ship = dynamic_cast<const DaVessel*>(pAtom);
			if(ship)
			{
				// A spacecraft spawn point is sought.
				switch(ship->CanSpawnHere(atomToSpawn, empire))
				{
				case 0:					// Ok
					if(g_Au.m_LogMessages & (LOG_AtomQuery|LOG_Spawn))
						qDebug() << "AuSceneServerApp::QueryAtomServer found ship" << *ship << "ok";
					break;
				case 1:
					result = 2;			// Not available
					if(g_Au.m_LogMessages & (LOG_AtomQuery|LOG_Spawn))
						qDebug() << "AuSceneServerApp::QueryAtomServer found ship" << *ship << "not ok, life support down";
					break;
				case 2:
					result = 2;			// Not available
					if(g_Au.m_LogMessages & (LOG_AtomQuery|LOG_Spawn))
						qDebug() << "AuSceneServerApp::QueryAtomServer found ship" << *ship << "not ok, enemy";
					break;
				case 3:
					result = 2;			// Not available
					if(g_Au.m_LogMessages & (LOG_AtomQuery|LOG_Spawn))
						qDebug() << "AuSceneServerApp::QueryAtomServer found ship" << *ship << "not ok, no berth aboard";
					break;
				case 4:
					result = 0;			// Vessel deleted and almost gone or on the way elsewhere.
					if(g_Au.m_LogMessages & (LOG_AtomQuery|LOG_Spawn))
						qDebug() << "AuSceneServerApp::QueryAtomServer found ship" << *ship << "bad state" << ship->AtomState();
					break;
				}
			}
			else
			{
				const DaBd* bd = dynamic_cast<const DaBd*>(pAtom);
				if(bd)
				{
					// A bd spawn point is sought.
					switch(bd->CanSpawnHere(empire))
					{
					case 0:				// Ok
						if(g_Au.m_LogMessages & (LOG_AtomQuery|LOG_Spawn))
							qDebug() << "AuSceneServerApp::QueryAtomServer found bd" << *bd << "ok";
						break;
					case 1:
						result = 2;		// Not available
						if(g_Au.m_LogMessages & (LOG_AtomQuery|LOG_Spawn))
							qDebug() << "AuSceneServerApp::QueryAtomServer found bd" << *bd << "not ok";
						break;
					case 2:
						result = 0;		// Bd deleted and almost gone or on the way elsewhere.
						if(g_Au.m_LogMessages & (LOG_AtomQuery|LOG_Spawn))
							qDebug() << "AuSceneServerApp::QueryAtomServer found bd" << *bd << "bad state" << ship->AtomState();
						break;
					}
				}
				else
					result = 2;			// Unrecognized type of spawn atom.
			}
		}
		g_Au.UnlockEtheriumDelete();

		// Send the result, good or bad.
		if(m_tClientChannel)
			m_tClientChannel->SendAtomPresence(idAtom, empire, result);
	}
	else
	{
		// Scene Manager and Master must defer the query to its subordinates.
		AuAtomQuery query;
		query.m_Atom = idAtom;
		query.m_Empire = empire;
		const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
		query.m_RepliesPending = servers.size();	// The number of reponses we are expecting from the query.
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);

		// Remember the atom queries that are unresolved.
		m_QueryMutex.lock();
		m_AtomQueries.append(query);
		m_QueryMutex.unlock();

		// This signal should be sent to each server channel thread in a nice thread-safe Qt way; cross your fingers.
		emit QueryAtomServerDown(idAtom, empire, atomToSpawn);
	}

	if(g_Au.m_LogMessages & LOG_AtomQuery)
	{
		QString logmsg("AuSceneServerApp::QueryAtomServer done");
		g_Au.Log(logmsg);
	}
}

// Called when a subordinate server answers an atom presence query.
void AuSceneServerApp::AtomPresence(AuId atom, unsigned int empire, int presence, unsigned int serverId)
{
	//if(presence)
	//qDebug("AuSceneServerApp::AtomPresence atom %d empire %d presence %d serverId %d", atom.GetID(), empire, presence, serverId);
	//Q_ASSERT(!g_Au.IsSceneWorker());	// Never has subordinate servers.
	// Insure we're still waiting to resolve the query. This could be a
	// late response or one received after we got a definitive answer.
	bool queryActive = false;
	bool isLastReply = false;
	m_QueryMutex.lock();
	for(QList<AuAtomQuery>::iterator it = m_AtomQueries.begin(); it != m_AtomQueries.end(); it++)
	{
		if((*it).m_Atom == atom && (*it).m_Empire == empire)
		{
			// The atom query is still waiting for a definite result.
			queryActive = true;
			(*it).m_RepliesPending--;
			if(!(*it).m_RepliesPending)
				isLastReply = true;
			break;
		}
	}
	m_QueryMutex.unlock();

	// Ignore the call if we're not waiting for the information any more.
	if(!queryActive)
	{
		//qDebug("AuSceneServerApp::AtomPresence atom query no longer active");
		return;
	}

	QString netName, peerName;
	quint16 port = 0;
	if(presence)
	{
		// The atom was found; send notifications immediately.
		if(presence == 1)
		{
			// The atom was found and it is friendly/viable.
			// Get info about the server that claims to have found the atom.
			m_ServerChannel.LockServerList(__FILE__, __LINE__);
			DtServerChannel* server = m_ServerChannel.ServerChannel(serverId);
			if(server && !server->isFinished())
			{
				netName = server->InternetName();
				port = server->InternetPort();
				peerName = server->PeerAddress();
				if(g_Au.m_LogMessages & LOG_AtomQuery)
					qDebug() << "AuSceneServerApp::AtomPresence atom" << atom << "empire" << empire << "server" << serverId
						<< netName << port << peerName;
			}
			m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
		}
		else
		{
			// The atom was found but it is not friendly/viable.
			// Configure for failed result.
			netName = g_Au.SceneMasterName();
			port = AuUtil::SceneMasterPort();
			peerName = g_Au.SceneMasterName();
			serverId = 1;
			//qDebug("AuSceneServerApp::AtomPresence NOT VIABLE found by "
					//"%s:%d %s", (const char*)netName.toAscii(), port,
					//(const char*)peerName.toAscii());
		}

		// Signal that the query is no longer active so it will be removed from the pending queries array.
		queryActive = false;
	}
	else
	{
		// A sub server reported the atom is definitely not present.
		// If we have heard from all the subServers, send the final result.
		if(isLastReply)
		{
			// Configure for failed result.
			netName = g_Au.SceneMasterName();
			port = AuUtil::SceneMasterPort();
			peerName = g_Au.SceneMasterName();
			serverId = 1;

			// Signal that the query is no longer active so it will be removed from the pending queries array.
			queryActive = false;
		}
	}

	if(!queryActive)
	{
		// The atom query is resolved so remove it from our pending query results array.
		//qDebug("AuSceneServerApp::AtomPresence remove pending query");
		m_QueryMutex.lock();
		for(QList<AuAtomQuery>::iterator itQuery = m_AtomQueries.begin(); itQuery != m_AtomQueries.end(); itQuery++)
		{
			if((*itQuery).m_Atom == atom && (*itQuery).m_Empire == empire)
			{
				// This is the first atom query that matches so remove it.
				m_AtomQueries.erase(itQuery);
				break;
			}
		}
		m_QueryMutex.unlock();

		// Notify all sub server threads that are looking for the atom.
		// If it wasn't found, they'll receive the master server data and defer to there.
		//qDebug("AuSceneServerApp::AtomPresence notify scene servers");
		emit AtomPresenceResolved(atom, empire, netName, port, peerName);

		// Notify the scene master also, in case he cares.
		// He gets the actual result: 0=no, 1=yes, or 2=empire not friendly.
		if(!g_Au.IsSceneMaster() && m_tClientChannel)
			m_tClientChannel->SendAtomPresence(atom, empire, presence);

		// Scene master and manager look for multiple atoms when filling spawn requests.
		// If the serverId = 1 for a given atom, they'll know the search for that atom found it but it was not viable.
		m_SpawnMutex.lock();
		for(QList<AuSpawnRequest>::iterator itSpawn = m_Spawns.begin(); itSpawn != m_Spawns.end(); itSpawn++)
		{
			if((*itSpawn).m_Arena == atom && (*itSpawn).m_ArenaServerId == 0)
				(*itSpawn).m_ArenaServerId = serverId;
			else if((*itSpawn).m_Ship == atom && (*itSpawn).m_ShipServerId == 0)
				(*itSpawn).m_ShipServerId = serverId;
			else if((*itSpawn).m_City == atom && (*itSpawn).m_CityServerId == 0)
				(*itSpawn).m_CityServerId = serverId;
			else if((*itSpawn).m_CapitolG == atom && (*itSpawn).m_CapitolGServerId == 0)
				(*itSpawn).m_CapitolGServerId = serverId;
			else if((*itSpawn).m_CapitolU == atom && (*itSpawn).m_CapitolUServerId == 0)
				(*itSpawn).m_CapitolUServerId = serverId;
			else if((*itSpawn).m_RandomCity == atom && (*itSpawn).m_RandomCityServerId == 0)
				(*itSpawn).m_RandomCityServerId = serverId;
		}
		m_SpawnMutex.unlock();

		// Scene master and manager look for a destination atom when filling handoff requests.
		// If the serverId = 1 for a given atom, they'll know the search for that atom found it but it was not viable.
		m_HandoffMutex.lock();
		for(QList<AuHandoffRequest>::iterator itHandoff = m_Handoffs.begin(); itHandoff != m_Handoffs.end(); itHandoff++)
			if((*itHandoff).m_Dest == atom)
				(*itHandoff).m_ServerId = serverId;
		m_HandoffMutex.unlock();
	}
	//qDebug("AuSceneServerApp::AtomPresence done");
}

// Deassign sectors after the connection with the assigned Scene Manager is lost.
void AuSceneServerApp::DeassignLostSectors()
{
	// Only scene master does this.
	if(!g_Au.IsSceneMaster())
		return;

	// This must be locked before the SectorAssignments lock.
	m_ServerChannel.LockServerList(__FILE__, __LINE__);

	// Check each system assignment to see if we are still connected to its server.
	m_AssignmentsLock.lock();
	unordered_map<AuId, AuSectorAssignment, AuIdHash>::iterator it = m_SectorAssignments.begin();
	while(it != m_SectorAssignments.end())
	{
		if(!m_ServerChannel.ServerChannel((*it).second.m_toServer.m_Id))
		{
			// Remove the assignment record.
			it = m_SectorAssignments.erase(it);
		}
		else
		{
			// Still connected.
			it++;
		}
	}
	m_AssignmentsLock.unlock();

	m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
}

// Deassign solar systems after the connection with the assigned Scene Worker
// is lost.
void AuSceneServerApp::DeassignLostSolarSystems()
{
	// Only scene manager does this.
	if(!g_Au.IsSceneManager())
		return;

	// This must be locked before the SystemAssignments lock.
	m_ServerChannel.LockServerList(__FILE__, __LINE__);

	// Check each system assignment to see if we are still connected to its server.
	m_AssignmentsLock.lock();
	unordered_map<AuId, AuSystemAssignment, AuIdHash>::iterator it = m_SystemAssignments.begin();
	while(it != m_SystemAssignments.end())
	{
		if(!m_ServerChannel.ServerChannel((*it).second.m_toServer.m_Id))
		{
			// Remove the assignment record.
			it = m_SystemAssignments.erase(it);
		}
		else
		{
			// Still connected.
			it++;
		}
	}
	m_AssignmentsLock.unlock();

	m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
}

void AuSceneServerApp::ClientChannelLost()
{
	// The connection to my boss server has been lost.
	//Q_ASSERT(!g_Au.IsSceneMaster());	// Scene Master doesn't have a boss.
	if(!m_tDb)
		return;							// We are probably in the midst of shutting down.

	// Shutdown is preferable to trying to recover. This way I can shut down the entire server farm by shutting down the master.
	if(g_Au.m_LogMessages & LOG_Error)
		g_Au.Error("Client channel lost. Shutting down.");
	Shutdown();
}

// Called on scene worker when a wormhole has been explored for the
// first time to propagate a notification to all servers.
// The message propagates back down as a RenameWormhole call.
void AuSceneServerApp::WormholeExplored(AuId systemId, AuId toSystem, qint8 polarity, quint32 empire, QString nuName)
{
	if(m_tClientChannel)
	{
		//qDebug("AuSceneServerApp::WormholeExplored - systemId %d toSystem %d polarity %d empire %d name %s",
				//systemId.GetID(), toSystem.GetID(), polarity, empire, (const char*)nuName.toAscii());
		// Queue a message for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_WormholeExplored;
		out << systemId;
		out << toSystem;
		out << polarity;
		out << empire;
		out << nuName;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::CityCaptured(quint32 oldEmpireId, quint32 nuEmpireId, AuId cityId)
{
	if(m_tClientChannel)
	{
		//qDebug("AuSceneServerApp::CityCaptured - %s:%d was captured by Empire %d from Empire %d", 
			//cityId.GetID(), nuEmpireId.GetID(), oldEmpireId.GetID());
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_CityCaptured;
		out << oldEmpireId;
		out << nuEmpireId;
		out << cityId;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::CityDecayed(quint32 empireId, AuId cityId)
{
	if(m_tClientChannel)
	{
		//qDebug("AuSceneServerApp::CityDecayed - %s:%d decayed and will be removed from Empire %d", 
			//cityId.GetID(), empireId.GetID());
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_CityDecayed;
		out << empireId;
		out << cityId;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::EmpireCityWorkerSpeciesChanged(quint32 empireId, AuId cityId, AuSpecies workerSpecies)
{
	if(g_Au.IsSceneMaster())
	{
		// Scene master updates the empire record.
		// Since a city is only owned by one empire at a time, this change can only affect one empire.
		DgData& gd = g_Au.LockDataForWrite();
		bool anyChange = gd.SetEmpireCityWorkerSpecies(empireId, cityId, workerSpecies);	// Returns empire id if one is changed.
		g_Au.UnlockData();
		if(anyChange)
		{
			emit ChangeEmpireInDb(empireId);					// Notify the server farm that an empire record changed.
		}
	}
	else if(m_tClientChannel)
	{
		// Subordinate non-master servers propagate the notification up to the master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_EmpireCityWorkerDNAChanged;
		out << empireId;
		out << cityId;
		out << workerSpecies;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::CityCreated(quint32 empireId,
		quint8 cityType, AuId cityId, QString cityName,
		AuId founderId, QString founderName,
		AuId planetId, QString planetName,
		AuId systemId, QString systemName, AuPoint systemLoc,
		AuId sectorId, QString sectorName,
		AuId galaxyId, quint8 galaxy)
{
	if(!empireId)
		return;

	if(g_Au.IsSceneMaster())
	{
		// Master changes the empire and notifies the subs.
		DgData& gd = g_Au.LockDataForWrite();
		gd.AddCityToEmpire(empireId,
				(CITYTYPE)cityType, cityId, cityName,
				founderId, founderName,
				planetId, planetName,
				systemId, systemName, systemLoc,
				sectorId, sectorName,
				galaxyId, (GALAXY)galaxy, false, false);	// Newly created city does not start out as a sector capital.
		g_Au.UnlockData();

		// The scene master notifies all scene managers of the change.
		emit ChangeEmpireInDb(empireId);	// Trigger db store which notifies the subordinates when done.

		// End of the line for the CityCreated activity.
		if(g_Au.m_LogMessages & LOG_Construction)
			qDebug() << "AuSceneServerApp::CityCreated empire" << empireId
				<< "city" << cityId << cityName << cityType
				<< "founder" << founderId << founderName
				<< "planet" << planetId << planetName
				<< "system" << systemId << systemName
				<< "sector" << sectorId << sectorName
				<< "galaxy" << galaxyId << GalaxyName((GALAXY)galaxy);
		return;
	}
	else
	{
		if(g_Au.IsSceneManager())
		{
			// This manager server is responsible for updating and saving its sector records.
			// Solar system records are updated here but the responsible worker takes care of updating the database.
			// Any name change is propagated so the entire server farm gets notified.
			// Locate the sector so we can see if its name has changed.
			bool systemNameChanged = false;
			AuEther& ether = g_Au.LockEtheriumDelete();
			DaSector* psect = dynamic_cast<DaSector*>(ether.FindNormalAtom(sectorId));
			if(psect)
			{
				if(!psect->HasName())
				{
					// Sector has not been named.
					psect->SetName(sectorName);
					psect->SetModified();
				}

				// Locate the solar system in the sector so we can see if its name has changed.
				DaSolarSystem* pss = (DaSolarSystem*)psect->FindNormalSubatomId(systemId);
				if(pss && !pss->HasName())
				{
					// Solar system has not been named.
					pss->SetNameIncludePlanets(systemName, false);	// The worker server updates the database, not me.
					systemNameChanged = true;
				}
			}
			else if(g_Au.m_LogMessages & LOG_Error)
				qWarning("ERROR! DtServerChannel::ReadPacket P_CityCreated sector %d not found to rename", sectorId.GetID());
			g_Au.UnlockEtheriumDelete();

			// This is the beginning of the trail for the SystemNameChanged message. From here it will propagate up to
			// the master and then back down to every server in the farm so they can update their local data.
			// None of them will update the database since the sector and solar system records have been updated if
			// needed by now.
			if(systemNameChanged)
				SystemNameChangedUp(systemId, systemName, sectorId, sectorName);
		}

		// Scene worker and manager propagates message up to boss server.
		if(m_tClientChannel)
		{
			if(g_Au.m_LogMessages & LOG_Governance)
				qDebug() << "AuSceneServerApp::CityCreated empire" << empireId
					<< "city" << cityId << cityName << cityType
					<< "founder" << founderId << founderName
					<< "planet" << planetId << planetName
					<< "system" << systemId << systemName << systemLoc
					<< "sector" << sectorId << sectorName
					<< "galaxy" << galaxyId << GalaxyName((GALAXY)galaxy);

			// Queue a message for my boss server, it could be a manager or a master.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_CityCreated;
			out << empireId;
			out << cityType;
			out << cityId;
			out << cityName;
			out << founderId;
			out << founderName;
			out << planetId;
			out << planetName;
			out << systemId;
			out << systemName;
			out << systemLoc;
			out << sectorId;
			out << sectorName;
			out << galaxyId;
			out << galaxy;
			m_tClientChannel->SendPacket(ba);
		}
	}
}

// Called after a manager server has changed the name of a solar system and possibly its parent sector.
void AuSceneServerApp::SystemNameChangedDown(AuId systemId, QString systemName, AuId sectorId, QString sectorName)
{
	// Change the names in my local data.
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSector* psect = dynamic_cast<DaSector*>(ether.FindNormalAtom(sectorId));
	if(psect)
	{
		// Set the name of the sector.
		// Flag the sector atom as modified but don't update the database.
		// That was done by the manager that initiated the name change.
		//if(g_Au.m_LogMessages & LOG_Debug)
			//qDebug() << "AuSceneServerApp::SystemNameChangedDown sector name set" << sectorId << sectorName;
		psect->SetName(sectorName);
		psect->SetModified(DaAtom::NOTIFY_Scene);

		// Locate the solar system in the sector so it can be named.
		DaSolarSystem* pss = dynamic_cast<DaSolarSystem*>(psect->FindNormalSubatomId(systemId));
		if(pss)
		{
			// Flag the solar system atom as modified but don't update the database.
			// That was done by the worker that initiated the name change.
			// It may also have been done when the contents of the system decayed, thus blanking the name.
			//if(g_Au.m_LogMessages & LOG_Debug)
				//qDebug() << "AuSceneServerApp::SystemNameChangedDown solar system name set" << systemId << systemName;
			pss->SetNameIncludePlanets(systemName, false);
			pss->SetModified(DaAtom::NOTIFY_Scene);
		}
		g_Au.UnlockEtheriumDelete();

		// Update names in empire records.
		// Multiple empire records could change as a result of a nu sector name.
		// We only have to worry about one empire changing as a result of a nu system name, because the system cannot be renamed
		// unless only one empire owns all the cities there.
		QList<quint32> empireIds;
		DgData& gd = g_Au.LockDataForWrite();
		gd.SetEmpireSectorName(sectorId, sectorName, empireIds);
		quint32 empireId = gd.SetEmpireSystemName(systemId, systemName);
		g_Au.UnlockData();
		if(!empireIds.contains(empireId))
			empireIds.append(empireId);

		// Insure scene clients get updated with fresh data.
		// This does not send a packet. It simply sets a flag so the scene clients get updated stars at the next sync.
		if(g_Au.IsSceneWorker())
		{
			//qDebug("AuSceneServerApp::SystemNameChangedDown worker notifying scene clients");
			emit SolarSystemChanged(systemId, sectorId);	// So clients will get fresh data.
		}
		else if(g_Au.IsSceneMaster())
		{
			// Master maintains empire records in the database.
			for(QList<quint32>::const_iterator ei = empireIds.cbegin(); ei != empireIds.cend(); ei++)
				emit ChangeEmpireInDb(*ei);						// Notify the database thread of the empire record change.
		}
	}
	else
		g_Au.UnlockEtheriumDelete();

	// Propagate the message down to my subordinate servers so they can update their local data and notify their subs the same way.
	if(!g_Au.IsSceneWorker())
	{
		// Worker servers don't have subordinates.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_SystemNameChanged;
		out << systemId;
		out << systemName;
		out << sectorId;
		out << sectorName;
		const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
		for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = servers.cbegin(); it != servers.cend(); it++)
			if((*it).second->isRunning())
				(*it).second->SendPacket(ba);
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
	}
}

// Called after a manager server has changed the name of a solar system and possibly its parent sector.
void AuSceneServerApp::SystemNameChangedUp(AuId systemId, QString systemName, AuId sectorId, QString sectorName)
{
	if(m_tClientChannel)
	{
		// Queue a message for my boss server, it could only be a master because a worker doesn't generate this message. I am not
		// the master because the master does not create a client channel.
		//Q_ASSERT(g_Au.IsSceneManager());
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_SystemNameChanged;
		out << systemId;
		out << systemName;
		out << sectorId;
		out << sectorName;
		m_tClientChannel->SendPacket(ba);
	}
	else
	{
		// Master must change the names in its own data and propagate the change to the rest of the server farm.
		SystemNameChangedDown(systemId, systemName, sectorId, sectorName);
	}
}

void AuSceneServerApp::DeclareMutualEnemies(quint32 empireId1, quint32 empireId2, quint8 reason, AuId fromUnit,
		QString fromUnitName, AuId toUnit, QString toUnitName, quint8 galaxy, QString sector, QString system, QString world)
{
	SetPoliticalStance(empireId1, empireId2, POL_Enemy, 0, fromUnit, fromUnitName, POL_Enemy, toUnit, toUnitName, reason,
			galaxy, sector, system, world);
}

void AuSceneServerApp::SetPoliticalStance(quint32 toEmpire, quint32 fromEmpire, quint8 fromStance, quint8 vassalTributePercent,
		AuId fromUnit, QString fromUnitName, quint8 toStance, AuId toUnit, QString toUnitName,
		quint8 reason, quint8 galaxy, QString sector, QString system, QString world)
{
	// It is not valid to set the stance toward another empire to be "empire".
	if((POLITICAL_STANCE)fromStance == POL_Empire)
	{
		//qDebug("AuSceneServerApp::SetPoliticalStance FromEmpire:%d ToEmpire:%d Stance:%d CAN'T SET POL_Empire STANCE!",
				//fromEmpire, toEmpire, fromStance);
		return;
	}
	if(toEmpire == fromEmpire)
	{
		//qDebug("AuSceneServerApp::SetPoliticalStance FromEmpire:%d ToEmpire:%d Stance:%d CAN'T DO THIS FOR THE SAME EMPIRES!",
				//fromEmpire, toEmpire, fromStance);
		return;
	}

	if(g_Au.IsSceneMaster())
	{
		//qDebug("AuSceneServerApp::SetPoliticalStance FromEmpire:%d  ToEmpire:%d  Stance:%d", fromEmpire, toEmpire, stance);
		DgData& gd = g_Au.LockDataForWrite();

		// Downgrades in stance are automatically reciprocated by the 'to' empire.
		// Downgrade Order:
		//  Empire - Cannot be downgraded.
		//  Vassal - Cannot be a downgrade. Cannot drop to Vassal from anything higher.
		//  Friend
		//  Neutral
		//  Enemy
		// 'Vassal' is never a downgrade so it is not reciprocated by the 'to' empire.
		bool toChanged = false;
		bool fromChanged = false;
		DgEmpireHash::iterator ief = gd.m_gEmpires.find(fromEmpire);
		if(ief == gd.m_gEmpires.end())
		{
			// From empire was not found.
			g_Au.UnlockData();
			//qWarning() << "error: AuSceneServerApp::SetPoliticalStance from empire not found" << fromEmpire << "fromUnit"
				//<< fromUnit << fromUnitName;
			return;
		}

		DgEmpireHash::iterator iet = gd.m_gEmpires.find(toEmpire);
		if(iet == gd.m_gEmpires.end())
		{
			// To empire was not found.
			g_Au.UnlockData();
			//qWarning() << "error: AuSceneServerApp::SetPoliticalStance to empire not found" << toEmpire << "toUnit"
				//<< toUnit << toUnitName;
			return;
		}

		// Found both empires in the hash table.
		// Get stance toward empire setting the stance.
		POLITICAL_STANCE stanceOfToEmpire = (*iet).second.PoliticalStance(fromEmpire);
		POLITICAL_STANCE oldFromStance = (*ief).second.PoliticalStance(toEmpire);	// Old stance, default if unknown.

		// The 'to' empire will now know about the 'from' empire, starting with its own default political stance.
		if(!(*iet).second.IsEmpireKnown(fromEmpire))
		{
			// The 'from' empire is not known to the 'to' empire so stanceOfToEmpire is 'to's default political stance.
			(*iet).second.SetPoliticalStance(fromEmpire, (*ief).second.Name(), stanceOfToEmpire, 0,
					AuDocUnitContact::DocumentType(), toUnit, toUnitName, (POLITICAL_STANCE)oldFromStance, fromUnit,
					fromUnitName, (GALAXY)galaxy, sector, system, world);
			toChanged = true;
		}

		if((POLITICAL_STANCE)fromStance != POL_Vassal)
		{
			// Insure the 'to' empire's stance toward the 'from' empire is not higher than the nu stance the 'from' empire
			// has set toward the 'to' empire.
			// Get the names of the two empires involved.
			QString sToEmpire = (*iet).second.Name();		// Name of empire toward whom the stance is set.
			QString sFromEmpire = (*ief).second.Name();	// Name of empire setting the stance.

			(*ief).second.SetPoliticalStance(toEmpire, sToEmpire, (POLITICAL_STANCE)fromStance, 0, reason, fromUnit,
					fromUnitName, stanceOfToEmpire, toUnit, toUnitName, (GALAXY)galaxy, sector, system, world);
			fromChanged = true;

			// We are done when this was an initial contact.
			if(reason != AuDocUnitContact::DocumentType())
			{
				// Stance change toward known empire.
				bool toReciprocated = false;
				if((POLITICAL_STANCE)fromStance == POL_Friend)
				{
					// 'from' empire has set stance to Friend.
				}
				else if((POLITICAL_STANCE)fromStance == POL_Neutral)
				{
					// 'from' empire has set stance to Neutral.
					if(stanceOfToEmpire == POL_Friend)
					{
						// Downgrade stance of 'to' empire toward 'from' empire to reciprocate Neutral declaration.
						(*iet).second.SetPoliticalStance(fromEmpire, sFromEmpire, POL_Neutral, 0,
								AuDocReciprocalStance::DocumentType(), toUnit, toUnitName, (POLITICAL_STANCE)fromStance,
								fromUnit, fromUnitName, (GALAXY)galaxy, sector, system, world);
						toChanged = true;
						toReciprocated = true;
					}
				}
				else
				{
					// 'from' empire has set stance to Enemy.
					if(stanceOfToEmpire != POL_Enemy)
					{
						// Downgrade stance of 'to' empire toward 'from' empire to reciprocate Enemy declaration.
						(*iet).second.SetPoliticalStance(fromEmpire, sFromEmpire, POL_Enemy, 0,
								AuDocReciprocalStance::DocumentType(), toUnit, toUnitName, (POLITICAL_STANCE)fromStance,
								fromUnit, fromUnitName, (GALAXY)galaxy, sector, system, world);
						toChanged = true;
						toReciprocated = true;
					}
				}

				// Notify the to empire of from's stance change, if no automatic reciprocation was triggered.
				if(!toReciprocated && fromStance != oldFromStance)
				{
					bool isUpgrade = true;
					switch(oldFromStance)
					{
					case POL_Empire:			// Not really logical; cannot set this stance toward another empire.
					case POL_Friend:
						isUpgrade = false;		// Not going vassal so any other change from Friend could only be a downgrade.
						break;
					case POL_Neutral:
						if(fromStance == POL_Enemy)
							isUpgrade = false;	// New stance of enemy is the only downgrade from neutral.
						break;
					case POL_Enemy:
						break;					// Any change from enemy is an upgrade.
					case POL_Vassal:
						isUpgrade = false;		// Any change from vassal is a downgrade.
						break;
					}

					if(isUpgrade)
						(*iet).second.DossierAddDocument(fromEmpire, AuDocStanceUpgraded(oldFromStance,
									(POLITICAL_STANCE)fromStance, vassalTributePercent, fromUnit, fromUnitName));
					else
						gd.EmpireDossierAddDocument(toEmpire, fromEmpire, AuDocStanceDowngraded(oldFromStance,
									(POLITICAL_STANCE)fromStance, fromUnit, fromUnitName));	// This call could trigger war.
					toChanged = true;
				}
			}
		}
		else
		{
			// Setting Vassal stance toward another empire.
			// Found the empire whose stance is being set.
			// Prevent setting vassal stance toward an empire that considers the vassal an enemy.
			if(stanceOfToEmpire != POL_Enemy)
			{
				// Get the name of the 'to' empire.
				QString sToEmpire = (*iet).second.Name();		// Name of empire toward whom the stance is set.

				POLITICAL_STANCE oldFromStance = (*ief).second.PoliticalStance(toEmpire);	// Old stance, default if unk

				// Turn off vassal stance toward any other empires (should only be one at most).
				for(unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::const_iterator itEmpire =
						(*ief).second.m_Stance.cbegin(); itEmpire != (*ief).second.m_Stance.cend(); itEmpire++)
				{
					if((*itEmpire).second.m_stance != POL_Vassal)
						continue;	// Not a vassal of that empire.
					if((*itEmpire).second.m_id == toEmpire)
						continue;	// To empire was already our overlord. Skip it.

					// Downgrade stance toward old overlord empire to Friend from Vassal.
					POLITICAL_STANCE overToMe = gd.EmpirePoliticalStance((*itEmpire).second.m_id, (*ief).second.m_id);
					(*ief).second.SetPoliticalStance((*itEmpire).second.m_id, (*itEmpire).second.m_name, overToMe, 0,
							AuDocNewOverlord::DocumentType(), fromUnit, fromUnitName, stanceOfToEmpire, toUnit,
							toUnitName, (GALAXY)galaxy, sector, system, world);
					fromChanged = true;
				}

				// Set stance of 'from' empire toward the 'to' empire.
				//qDebug("AuSceneServerApp::SetPoliticalStance empire %d declaring stance POL_Vassal toward %d %s",
						//fromEmpire, toEmpire, (const char*)sToEmpire.toAscii());
				(*ief).second.SetPoliticalStance(toEmpire, sToEmpire, POL_Vassal, vassalTributePercent, reason, fromUnit,
						fromUnitName, stanceOfToEmpire, toUnit, toUnitName, (GALAXY)galaxy, sector, system, world);
				fromChanged = true;

				// Notify the to empire.
				(*iet).second.DossierAddDocument(fromEmpire, AuDocStanceUpgraded(oldFromStance, (POLITICAL_STANCE)fromStance,
							vassalTributePercent, fromUnit, fromUnitName));
				toChanged = true;
			}
		}

		// Perform a little maintenance on the empire records that changed, since we will be updating the database anyway.
		if(fromChanged)
		{
			// Remove empires that no longer exist. These show up as Unknown Empires in the diplomacy list.
			unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::iterator si = (*ief).second.m_Stance.begin();
			while(si != (*ief).second.m_Stance.end())
			{
				// Check to see if the empire still exists, to whom the from empire has a stance.
				if(!gd.HasEmpire((*ief).first))
					si = (*ief).second.m_Stance.erase(si);
				else
					si++;
			}
		}
		if(toChanged)
		{
			// Remove empires that no longer exist. These show up as Unknown Empires in the diplomacy list.
			unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::iterator si = (*iet).second.m_Stance.begin();
			while(si != (*iet).second.m_Stance.end())
			{
				// Check to see if the empire still exists, to whom the to empire has a stance.
				if(!gd.HasEmpire((*iet).first))
					si = (*iet).second.m_Stance.erase(si);
				else
					si++;
			}
		}

		g_Au.UnlockData();

		if(fromChanged)
			emit ChangeEmpireInDb(fromEmpire);

		if(toChanged)
			emit ChangeEmpireInDb(toEmpire);
	}
	else if(m_tClientChannel)
	{
		// Queue a message for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_SetPoliticalStance;
		out << toEmpire;
		out << fromEmpire;
		out << fromStance;
		out << vassalTributePercent;
		out << fromUnit;
		out << fromUnitName;
		out << toStance;
		out << toUnit;
		out << toUnitName;
		out << reason;
		out << galaxy;
		out << sector;
		out << system;
		out << world;
		m_tClientChannel->SendPacket(ba);
	}
}

// On the way up the server chain.
void AuSceneServerApp::CreditCityBank(float fAmount, AuId cityId)
{
	if(g_Au.IsSceneMaster())
	{
		// Master server broadcasts the notification down to the server farm.
		CreditCityBankDown(fAmount, cityId);
	}
	else if(m_tClientChannel)
	{
		// Queue a message for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_CreditCityBank;
		out << fAmount;
		cityId >> out;
		m_tClientChannel->SendPacket(ba);
	}
}

// On the way down the server chain.
void AuSceneServerApp::CreditCityBankDown(float fAmount, AuId cityId)
{
	if(g_Au.IsSceneWorker())
	{
		// Look for the city in my local data.
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaAtom* atom = ether.FindNormalAtom(cityId);
		if(atom)
		{
			DaBd* bd = dynamic_cast<DaBd*>(atom);
			if(bd)
				bd->CreditAccountGovtTaxLevy(fAmount);
		}
		g_Au.UnlockEtheriumDelete();
	}
	else
	{
		// Propagate the message down to my subordinate servers.
		// Worker servers don't have subordinates.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_CreditCityBank;
		out << fAmount;
		cityId >> out;
		const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
		for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = servers.cbegin(); it != servers.cend(); it++)
			if((*it).second->isRunning())
				(*it).second->SendPacket(ba);
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
	}
}

void AuSceneServerApp::AddCitizenToEmpire(quint32 oldEmpireId, quint32 nuEmpireId, AuId personId, QString personName,
		quint8 personGender)
{
	if(g_Au.IsSceneMaster())
	{
		// The scene master notifies all scene managers of the change.
		// Master changes the empire and notifies the subs.
		//qDebug("DtServerChannel::ReadPacket, Master is calling AddCitizenToEmpire");
		DgData& gd = g_Au.LockDataForWrite();
		gd.AddCitizenToEmpire(oldEmpireId, nuEmpireId, personId, personName, personGender);
		g_Au.UnlockData();
		if(oldEmpireId > 0)
			emit ChangeEmpireInDb(oldEmpireId);		// Trigger db store
		//else
			//qDebug("DtServerChannel::P_AddCitizen OldEmpireChanged=0");
		if(nuEmpireId > 0)
			emit ChangeEmpireInDb(nuEmpireId);		// Trigger db store
		//else
			//qDebug("DtServerChannel::P_AddCitizen NewEmpireChanged=0");
	}
	if(m_tClientChannel)
	{
		//qDebug("AuSceneServerApp::AddCitizenToEmpire - PersonId: %d, Empire: %d",
			//personId.GetID(),nuEmpireId.GetID());
		// Queue a message for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_AddCitizen;
		out << oldEmpireId;
		out << nuEmpireId;
		out << personId;
		out << personName;
		out << personGender;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::RenameWormhole(const AuId systemId, const AuId toSystem,
		qint8 polarity, quint32 empire, const QString nuName)
{
	//qDebug("AuSceneServerApp::RenameWormhole - system %d toSystem %d polarity %d empire %d name %s", systemId.GetID(),
			//toSystem.GetID(), polarity, empire, (const char*)nuName.toAscii());

	// Modify the wormhole if I have it in my local data.
	// I don't need to update its db data with the call to SetModified
	// because the server responsible for the wormhole takes care of that.
	// SceneMaster and SceneManager would not be allowed to do so.
	// The clients who have the system in their scene do need an update.
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSolarSystem* pss = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(systemId));
	if(pss)
	{
		pss->WormholeExplored(toSystem, (AuWormhole::POLARITY)polarity, empire, nuName);
		pss->SetModified(DaAtom::NOTIFY_Scene);					// No db update but scenes need update.
		AuId parId(pss->ParentId());
		g_Au.UnlockEtheriumDelete();
		emit SolarSystemChanged(systemId, parId);			// So clients will get fresh wormhole data.
	}
	else
	{
		//qDebug("AuSceneServerApp::RenameWormhole - system not found");
		g_Au.UnlockEtheriumDelete();
	}

	if(!g_Au.IsSceneWorker())
	{
		// Scene master broadcasts name change to all scene managers.
		// Scene managers broadcast name change to all scene workers.
		emit WormholeExploredSignal(systemId, toSystem, polarity, empire, nuName);
	}
}

// Called on the scene worker when a solar system has been explored for the first time by an empire.
// Propagates a notification to the entire server farm so everybody can update their starfield data.
void AuSceneServerApp::SolarSystemExplored(const AuId systemId, quint32 empireId)
{
	//qDebug("AuSceneServerApp::SolarSystemExplored %d empire %d", systemId.GetID(), empireId);
	if(g_Au.IsSceneMaster())
	{
		// Scene master needs to propagate a notification down to the entire farm.
		SolarSystemExploredDown(systemId, empireId);
	}
	else if(m_tClientChannel)
	{
		// Scene Manager and Worker just send the message up so it reaches the master.
		// Queue a message for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_SystemExplored;
		systemId >> out;
		out << empireId;
		m_tClientChannel->SendPacket(ba);
	}
}

// On the way down the server chain.
void AuSceneServerApp::SolarSystemExploredDown(const AuId systemId, quint32 empireId)
{
	// Set the explored status in my local data.
	// This does not update any database records. That was done by the worker in charge of the solar system.
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(systemId));
	if(system)
		system->ExploredBy(empireId);
	g_Au.UnlockEtheriumDelete();

	// Propagate the message down to my subordinate servers so they can update their local data and notify their subs the same way.
	if(!g_Au.IsSceneWorker())
	{
		// Worker servers don't have subordinates.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_SystemExplored;
		systemId >> out;
		out << empireId;
		const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
		for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = servers.cbegin(); it != servers.cend(); it++)
			if((*it).second->isRunning())
				(*it).second->SendPacket(ba);
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
	}
}

// Called on the scene worker when a solar system has been explored for the first time by an empire.
// Propagates a notification to the entire server farm so everybody can update their starfield data.
void AuSceneServerApp::SolarSystemScanned(const AuId systemId, quint32 empireId, quint8 genHab, quint32 genSeed)
{
	//qDebug("AuSceneServerApp::SolarSystemScanned %d empire %d", systemId.GetID(), empireId);
	if(g_Au.IsSceneMaster())
	{
		// Scene master needs to propagate a notification down to the entire farm.
		SolarSystemScannedDown(systemId, empireId, genHab, genSeed);
	}
	else if(m_tClientChannel)
	{
		// Scene Manager and Worker just send the message up so it reaches the master.
		// Queue a message for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_SystemScanned;
		out << systemId;
		out << empireId;
		out << genHab;
		out << genSeed;
		m_tClientChannel->SendPacket(ba);
	}
}

// On the way down the server chain.
void AuSceneServerApp::SolarSystemScannedDown(const AuId systemId, quint32 empireId, quint8 genHab, quint32 genSeed)
{
	// Set the explored status in my local data.
	// This does not update any database records. That was done by the worker in charge of the solar system.
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(systemId));
	if(system && (!system->IsScannedBy(empireId) || system->GenHab() != genHab || system->GenSeed() != genSeed))
	{
		system->ScannedBy(empireId, (HABITABILITY)genHab, genSeed);
		//qDebug() << "AuSceneServerApp::SolarSystemScannedDown" << systemId << system->Name() << "empire" << empireId
			//<< "genHab" << genHab << "genSeed" << genSeed;
	}
	g_Au.UnlockEtheriumDelete();

	// Propagate the message down to my subordinate servers so they can update their local data and notify their subs the same way.
	if(!g_Au.IsSceneWorker())
	{
		// Worker servers don't have subordinates.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_SystemScanned;
		out << systemId;
		out << empireId;
		out << genHab;
		out << genSeed;
		const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
		for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = servers.cbegin(); it != servers.cend(); it++)
			if((*it).second->isRunning())
				(*it).second->SendPacket(ba);
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
	}
}

// Called on the scene manager when the contents of a solar system have been finalized.
void AuSceneServerApp::SolarSystemFinal(AuId systemId)
{
	if(g_Au.IsSceneMaster())
	{
		// Scene master needs to flag the system as finalized.
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(systemId));
		if(system)
			system->SetFinal(true);
		g_Au.UnlockEtheriumDelete();
	}
	else if(m_tClientChannel)
	{
		// Scene manager notifies scene master.
		//qDebug("AuSceneServerApp::SolarSystemFinal - %d", system.GetID());
		// Queue a message for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_SystemFinal;
		out << systemId;
		m_tClientChannel->SendPacket(ba);
	}
}

// Called on the scene worker when a solar system thinks it is ready for the last stage of decay, removing its planets and stars.
void AuSceneServerApp::SolarSystemReadyToDecay(AuId systemId)
{
	if(g_Au.IsSceneWorker())
	{
		// Decay of solar system contents starts on the scene worker when a system signals it is ready to decay.
		// First verify the system is still ready, that nobody showed up at the last instant.
		bool revokeIt = false;
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(systemId));
		if(system && system->IsReadyToDecay())
		{
			system->DecayContents();		// Delete everything in the system.
			system->SetFinal(false);
			system->SetModified(DaAtom::NOTIFY_Database);
			revokeIt = true;
		}
		g_Au.UnlockEtheriumDelete();

		if(revokeIt)
		{
			// Queue a message for my manager server right away so it does not reassign the solar system when I revoke it.
			if(m_tClientChannel)
			{
				QByteArray ba;
				QDataStream out(&ba, QIODevice::WriteOnly);
				out.setVersion(QDataStream::Qt_4_1);
				out << (quint8)DtServerChannel::P_SystemDecayed;
				systemId >> out;
				m_tClientChannel->SendPacket(ba);
			}

			// Revoke my own system assignment.
			if(g_Au.m_LogMessages & LOG_LoadBalance)
				g_Au.Log(QString("AuSceneServerApp::SolarSystemReadyToDecay systemId %1").arg(systemId.GetID()));
			SystemRevoked(systemId);
		}
	}
	else if(g_Au.IsSceneManager())
	{
		// Scene manager revokes the assignment from its own data. The worker has already done this in its data.
		m_AssignmentsLock.lock();
		unordered_map<AuId, AuSystemAssignment, AuIdHash>::iterator rit = m_SystemAssignments.find(systemId);
		if(rit != m_SystemAssignments.end())
			m_SystemAssignments.erase(rit);		// Remove the assignment record.
		m_AssignmentsLock.unlock();

		// Clear the contents of the system that has decayed.
		AuId sectorId;
		QString sectorName;
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(systemId));
		if(system)
		{
			system->TrashSubatoms();
			system->SetNameIncludePlanets("", true);		// Manager clears solar system name.
			system->SetFinal(false);

			const DaSector* sector = system->FindParentSector();
			if(sector)
			{
				// Get current sector id and name for name change signal.
				sectorId = sector->Id();
				if(sector->HasName())
					sectorName = sector->SectorName();			// Just the name part, without the 'Sector' suffix.
			}
		}
		g_Au.UnlockEtheriumDelete();

		// Propagate the name change to blank.
		SystemNameChangedUp(systemId, "", sectorId, sectorName);

		// Queue a message for my master server.
		if(m_tClientChannel)
		{
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_SystemDecayed;
			out << systemId;
			m_tClientChannel->SendPacket(ba);
		}
	}
	else if(g_Au.IsSceneMaster())
	{
		// Clear the contents of the system that has decayed.
		AuIdList deletedSectors;
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(systemId));
		if(system)
		{
			system->TrashSubatoms();
			system->SetFinal(false);

			// When the parent sector of the system no longer thinks it IsExplored, there are no more solar systems in the sector
			// that have any contents. At this time, the sector and one or more of its 26 neighboring sectors may be eligible for
			// decay. A sector decays when it is !IsExplored and none of its neighboring sectors IsExplored. Only the master server
			// can make this determination because no sub server loads enough of the sector data.
			/*DHH Sector decay may be causing severe problems. Oct 16, 2012
			DaSector* sector = system->FindParentSector();
			Q_ASSERT(sector);
			if(sector && sector->IsReadyToDecay())
				DecaySector(sector, deletedSectors);
				*/
		}
		g_Au.UnlockEtheriumDelete();

		// Notify subordinate servers of sectors that decayed.
		for(AuIdList::const_iterator di = deletedSectors.cbegin(); di != deletedSectors.cend(); di++)
		{
			// Revoke assignment of the sector, if it is assigned to anyone.
			RevokeAssignment(*di);
			emit SectorDecayed(*di);
		}
	}
}

// Examines a sector and its 26 neighbors and deletes the ones that are no longer needed.
// 'deletedSectors' is filled with the ids of sectors that get deleted.
void AuSceneServerApp::DecaySector(DaSector* sector, AuIdList& deletedSectors)
{
	DaGalaxy* galaxy = sector->FindParentGalaxy();
	Q_ASSERT(galaxy);
	if(!galaxy)
		return;

	// Delete sectors that are not needed.
	AuIdList sectorsToDecay;
	galaxy->ListSectorsReadyToDecay(sector, sectorsToDecay);
	if(!sectorsToDecay.isEmpty())
		galaxy->DecaySectors(sectorsToDecay, deletedSectors);
}

// Examines all sectors in the galaxy and deletes the ones that are no longer needed.
// 'deletedSectors' is filled with the ids of sectors that get deleted.
void AuSceneServerApp::DecaySectors(DaGalaxy* galaxy, AuIdList& deletedSectors)
{
	Q_ASSERT(galaxy);
	if(!galaxy)
		return;

	// Delete sectors that are not needed.
	AuIdList sectorsToDecay;
	galaxy->ListSectorsReadyToDecay(sectorsToDecay);
	if(!sectorsToDecay.isEmpty())
		galaxy->DecaySectors(sectorsToDecay, deletedSectors);
}

void AuSceneServerApp::SectorDecayedDown(AuId sectorId)
{
	// Remove the sector that has decayed.
	AuEther& ether = g_Au.LockEtheriumDelete();
	ether.TrashAtom(sectorId);
	g_Au.UnlockEtheriumDelete();

	// Manager notifies subordinate workers.
	if(g_Au.IsSceneManager())
		emit SectorDecayed(sectorId);
}

// The goal of load balancing is to have the smoothest tick times throughout the galaxy.
// Employs a simple highest-to-lowest strategy.
// One object is deassigned from the most loaded server. The next time we assign objects in AssignSectors or AssignSolarSystems,
// it will be assigned to the least loaded server.
void AuSceneServerApp::BalanceLoads()
{
	m_ServerChannel.LockServerList(__FILE__, __LINE__);
	DtServerChannel* mostLoaded = m_ServerChannel.MostLoadedServer();
	if(!mostLoaded)
	{
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
		//QString logmsg;
		//logmsg = QString("AuSceneServerApp::BalanceLoads no 'most loaded' server to balance");
		//g_Au.Log(logmsg);
		return;		// Nobody suitable to give up some load?
	}

	// Nothing needs to be done if the most loaded server isn't working hard.
	if(mostLoaded->AtomLoad() < .4f)
	{
		//QString logmsg;
		//logmsg = QString("AuSceneServerApp::BalanceLoads most loaded server %1 reports only %2 load")
				//.arg(mostLoaded->Id()).arg(mostLoaded->CurrentLoad());
		//g_Au.Log(logmsg);
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
		return;		// Most loaded server isn't working that hard.
	}

	int subServerMode = g_Au.IsSceneMaster() ? 1 : 2;	// 1=manager, 2=worker
	DtServerChannel* leastLoaded = m_ServerChannel.LeastLoadedServer(subServerMode);
	if(!leastLoaded)
	{
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
		//QString logmsg;
		//logmsg = QString("AuSceneServerApp::BalanceLoads no 'least loaded' server to balance");
		//g_Au.Log(logmsg);
		return;		// Nobody suitable to accept some load?
	}

	if(mostLoaded == leastLoaded)
	{
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
		//QString logmsg;
		//logmsg = QString("AuSceneServerApp::BalanceLoads 'most loaded' server is 'least loaded' server");
		//g_Au.Log(logmsg);
		return;		// Only one server in the barrel?
	}

	// Compare the load difference between the most and least loaded servers.
	// Don't do anything if the difference is less than the balance threshold.
	const float atomBalanceThreshold = .2f;
	float atomLoadDiff = mostLoaded->AtomLoad() - leastLoaded->AtomLoad();
	if(atomLoadDiff < atomBalanceThreshold)
	{
		if(g_Au.m_LogMessages & LOG_LoadBalance)
		{
			QString logmsg;
			logmsg = QString("BalanceLoads nothing to do, atomLoadDiff %1 mostAtomLoad %2 leastAtomLoad %3")
				.arg(atomLoadDiff, 0,'f',2)
				.arg(mostLoaded->AtomLoad(), 0,'f',2)
				.arg(leastLoaded->AtomLoad(), 0,'f',2);
			g_Au.Log(logmsg);
		}
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
		return;
	}

	// Select an object to take from the most loaded server.
	unsigned int mostId = mostLoaded->Id();
	if(g_Au.IsSceneMaster())
	{
		m_AssignmentsLock.lock();
		for(unordered_map<AuId, AuSectorAssignment, AuIdHash>::iterator it = m_SectorAssignments.begin();
				it != m_SectorAssignments.end(); it++)
		{
			if((*it).second.m_toServer.m_Id == mostId)
			{
				// Cancel the assignment of the sector to the most loaded server.
				// Deassigned sectors will be reassigned in the next call to AssignSectors.
				RevokeSector((*it).first);
				break;											// Only one sector is deassigned.
			}
		}
		m_AssignmentsLock.unlock();
	}
	else if(g_Au.IsSceneManager())
	{
		// Determine how many systems to deassign from the most loaded server.
		float loadShift = atomLoadDiff/2;		// Shift half the difference in system load.
		loadShift = loadShift / mostLoaded->AtomLoad();	// Calculate fraction of current overall load. (0..1).
		AuIdList revokeSystems;
		ListSystemsToRevoke(mostId, loadShift, revokeSystems);
		if(g_Au.m_LogMessages & LOG_LoadBalance)
		{
			QString logmsg;
			logmsg = QString("Deassigning %3 systems from worker %1 to relieve %2 load")
				.arg(mostId).arg(loadShift, 0,'f',2).arg(revokeSystems.count());
			g_Au.Log(logmsg);
		}
		if(revokeSystems.isEmpty())
		{
			m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
			return;	// Unlikely that most loaded server has no systems to revoke.
		}

		// Deassigned systems will be reassigned in the next call to AssignSolarSystems.
		m_AssignmentsLock.lock();
		for(AuIdList::const_iterator rsi = revokeSystems.cbegin(); rsi != revokeSystems.cend(); rsi++)
			RevokeSolarSystem(*rsi);
		m_AssignmentsLock.unlock();
	}
	m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
}

void AuSceneServerApp::ReassignSystem(AuId systemId, QString wrkServerName, quint16 wrkServerPort)
{
	m_ServerChannel.LockServerList(__FILE__, __LINE__);

	m_AssignmentsLock.lock();

	unordered_map<AuId, AuSystemAssignment, AuIdHash>::iterator it = m_SystemAssignments.find(systemId);
	if(it != m_SystemAssignments.end())
	{
		DtServerChannel* server = m_ServerChannel.ServerChannel((*it).second.m_toServer.m_Id);
		DtServerChannel* nuServer = m_ServerChannel.FindServer(wrkServerName, wrkServerPort);
		if(server && !server->isFinished() && nuServer && !nuServer->isFinished())
		{
			if(server != nuServer)
			{
				if(g_Au.m_LogMessages & LOG_LoadBalance)
				{
					QString logmsg;
					logmsg = tr("Reassigned solar system %1 assignment from worker %2")
						.arg((*it).first.GetID()).arg((*it).second.m_toServer.m_Id);
					g_Au.Log(logmsg);
				}

				// Remember recent revokation so we don't reassign back to the same server.
				m_Reassignments.insert(pair<AuId, unsigned int>(systemId, nuServer->Id()));

				// Notify the sub server that the assignment has been revoked.
				QByteArray ba;
				QDataStream out(&ba, QIODevice::WriteOnly);
				out.setVersion(QDataStream::Qt_4_1);
				out << (quint8)DtServerChannel::P_SystemRevoked;
				out << systemId;
				server->SendPacket(ba);

				// Remove the assignment record.
				// Deassigned systems will be reassigned in the next call to AssignSolarSystems.
				m_SystemAssignments.erase(it);
			}
			else
				qDebug() << "AuSceneServerApp::ReassignSystem" << systemId << "already assigned to worker"
					<< wrkServerName << wrkServerPort;
		}
	}

	m_AssignmentsLock.unlock();

	m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
}

// Revokes the assignment of 'atomId' from whatever subserver it is currently assigned to.
// It only makes sense to call this with sector ids on the scene master.
// It only makes sense to call this with solar system ids on the scene manager.
void AuSceneServerApp::RevokeAssignment(AuId atomId)
{
	m_ServerChannel.LockServerList(__FILE__, __LINE__);

	m_AssignmentsLock.lock();

	if(g_Au.IsSceneMaster())
		RevokeSector(atomId);
	else if(g_Au.IsSceneManager())
		RevokeSolarSystem(atomId);

	m_AssignmentsLock.unlock();

	m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
}

// Revokes the assignment of 'sectorId' from whatever sub manager server it is currently assigned to.
void AuSceneServerApp::RevokeSector(const AuId& sectorId)
{
	//Q_ASSERT(g_Au.IsSceneMaster());

	unordered_map<AuId, AuSectorAssignment, AuIdHash>::iterator it = m_SectorAssignments.find(sectorId);
	if(it != m_SectorAssignments.end())
	{
		DtServerChannel* server = m_ServerChannel.ServerChannel((*it).second.m_toServer.m_Id);
		if(server && !server->isFinished())
		{
			if(g_Au.m_LogMessages & LOG_LoadBalance)
			{
				QString logmsg;
				logmsg = tr("Revoked sector %1 assignment from manager %2")
					.arg((*it).first.GetID()).arg((*it).second.m_toServer.m_Id);
				g_Au.Log(logmsg);
			}

			// Remember recent revokation so we don't reassign back to the same server.
			m_Revokations.insert(pair<AuId, unsigned int>(sectorId, server->Id()));

			// Notify the sub server that the assignment has been revoked.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_SectorRevoked;
			out << sectorId;
			server->SendPacket(ba);
		}

		// Remove the assignment record.
		// Deassigned sectors will be reassigned in the next call to AssignSectors.
		m_SectorAssignments.erase(it);
	}
}

// Revokes the assignment of 'systemId' from whatever sub worker server it is currently assigned to.
void AuSceneServerApp::RevokeSolarSystem(const AuId& systemId)
{
	//Q_ASSERT(g_Au.IsSceneManager());

	unordered_map<AuId, AuSystemAssignment, AuIdHash>::iterator it = m_SystemAssignments.find(systemId);
	if(it != m_SystemAssignments.end())
	{
		DtServerChannel* server = m_ServerChannel.ServerChannel((*it).second.m_toServer.m_Id);
		if(server && !server->isFinished())
		{
			if(g_Au.m_LogMessages & LOG_LoadBalance)
			{
				QString logmsg;
				logmsg = tr("Revoked solar system %1 assignment from worker %2")
					.arg((*it).first.GetID()).arg((*it).second.m_toServer.m_Id);
				g_Au.Log(logmsg);
			}

			// Remember recent revokation so we don't reassign back to the same server.
			m_Revokations.insert(pair<AuId, unsigned int>(systemId, server->Id()));

			// Notify the sub server that the assignment has been revoked.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_SystemRevoked;
			out << systemId;
			server->SendPacket(ba);
		}

		// Remove the assignment record.
		// Deassigned systems will be reassigned in the next call to AssignSolarSystems.
		m_SystemAssignments.erase(it);
	}
}

// Returns average tick time of all subordinate servers.
// Returns 0xffffffff if there are no worker servers or none of them has reported a tick time.
// Return value is in 1/2 millisecond increments. i.e. 100 = 50ms
quint32 AuSceneServerApp::AverageWorkerTickTime() const
{
	// Accumulate number of ready servers and their total tick time.
	unsigned int serverCount = 0;
	unsigned int totaltime = 0;
	const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
	for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = servers.cbegin(); it != servers.cend(); it++)
	{
		if((*it).second->isFinished())
			continue;
		// We do not include servers reporting 0xffffffff tick time, which indicates they are not ready for work,
		// probably still loading initial data. They will report in once they are ready.
		if((*it).second->TickTime() != 0xffffffff)
		{
			serverCount++;
			totaltime += (*it).second->CurrentLoad();
		}
	}
	m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
	if(!serverCount)
		return 0xffffffff;

	// Calculate average tick time from totals.
	// The average can be slightly less than the nominal tick time imposed in
	// DtTick::run due to integer truncation; it is not significant.
	return totaltime/serverCount;
}

float AuSceneServerApp::AverageWorkerLoadAverage() const
{
	// Accumulate number of ready servers and their total load.
	unsigned int serverCount = 0;
	float totalLoad = 0;
	const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
	for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = servers.cbegin(); it != servers.cend(); it++)
	{
		if((*it).second->isFinished())
			continue;
		// We do not include servers reporting 0xffffffff tick time, which indicates they are not ready for work,
		// probably still loading initial data. They will report in once they are ready.
		if((*it).second->TickTime() != 0xffffffff)
		{
			serverCount++;
			totalLoad += (*it).second->AtomLoad();
		}
	}
	m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
	if(!serverCount)
		return 0;
	return totalLoad/serverCount;
}

// Called when a channel to a subordinate server has closed.
// All work assigned to that server needs to be reassigned.
void AuSceneServerApp::ServerChannelClosed(unsigned int serverId)
{
	QString logmsg;
	if(g_Au.IsSceneMaster())
	{
		// Scene master deletes all sector assignments to the manager server.
		// The sectors will be reassigned by the usual assigner.
		m_AssignmentsLock.lock();
		unordered_map<AuId, AuSectorAssignment, AuIdHash>::iterator it = m_SectorAssignments.begin();
		while(it != m_SectorAssignments.end())
		{
			if((*it).second.m_toServer.m_Id == serverId)
			{
				// Cancel the assignment of this object to the server.
				if(g_Au.m_LogMessages & LOG_LoadBalance)
				{
					QString logmsg = tr("Canceled sector %1 assignment to manager %2")
						.arg((*it).first.GetID()).arg((*it).second.m_toServer.m_Id);
					g_Au.Log(logmsg);
				}

				// Remove the assignment record.
				it = m_SectorAssignments.erase(it);
			}
			else
				it++;
		}
		m_AssignmentsLock.unlock();
	}
	else if(g_Au.IsSceneManager())
	{
		// Scene manager deletes all solar system assignments to the worker
		// server. The systems will be reassigned by the usual assigner.
		m_AssignmentsLock.lock();
		unordered_map<AuId, AuSystemAssignment, AuIdHash>::iterator it = m_SystemAssignments.begin();
		while(it != m_SystemAssignments.end())
		{
			if((*it).second.m_toServer.m_Id == serverId)
			{
				// Cancel the assignment of this object to the server.
				if(g_Au.m_LogMessages & LOG_LoadBalance)
				{
					QString logmsg = tr("Canceled solar system %1 assignment to worker %2")
						.arg((*it).first.GetID()).arg((*it).second.m_toServer.m_Id);
					g_Au.Log(logmsg);
				}

				// Remove the assignment record.
				it = m_SystemAssignments.erase(it);
			}
			else
				it++;
		}
		m_AssignmentsLock.unlock();
	}
}

void AuSceneServerApp::EmpireRecordLoaded(quint32 idEmpire)
{
	if(g_Au.IsSceneWorker())
	{
		//qDebug("AuSceneServerApp::EmpireRecordLoaded empire %d", idEmpire);
		// Look through each workers client servers and see if any client
		// has an atom unit with that empire id in their scene
		emit EmpireRecordAvailable(idEmpire);
	}
}

void AuSceneServerApp::SpeciesChanged(AuSpecies species, QString specieName, AuId whoNamed)
{
	//qDebug() << "AuSceneServerApp::SpeciesChanged" << species << specieName << whoNamed;
	//Q_ASSERT(!g_Au.IsSceneMaster());

	// Update my local data.
	DgData& gd = g_Au.LockDataForWrite();
	gd.NameSpecies(species, specieName, whoNamed);
	g_Au.UnlockData();

	if(g_Au.IsSceneWorker())
	{
		// Notify the clients.
		emit SpeciesRecordAvailable(species);
	}
	else
	{
		// Propagate the notification down to subordinate servers.
		emit SpeciesNameChanged(species, specieName, whoNamed);
	}
}

void AuSceneServerApp::SpeciesRecordLoaded(AuSpecies species)
{
	if(g_Au.IsSceneWorker())
		emit SpeciesRecordAvailable(species);
}

// Database thread notifies me of a saved empire record using this slot. This is where the trail begins at the master to notify
// the subordinates.
void AuSceneServerApp::EmpireRecordChanged(quint32 idEmpire)
{
	//qDebug() << "AuSceneServerApp::EmpireRecordChanged" << idEmpire;
	if(!g_Au.IsSceneWorker())
	{
		// Boss servers propagate this notification by sending a message to all their subordinates.
		DgEmpire empire;
		empire.m_id = idEmpire;
		const DgData& gd = g_Au.LockDataForRead();
		bool hasEmpire = gd.Empire(empire);
		g_Au.UnlockData();
		if(hasEmpire)
		{
			emit EmpireDoneChanging(empire);	// Propagate down to subordinate servers.
		}
		else
			qDebug() << "AuSceneServerApp::EmpireRecordChanged empire" << idEmpire << "record not found to send to subordinates";
	}
}

// The empire change notification is propagated down to the subs with the entire empire record so they can update their data asap.
void AuSceneServerApp::EmpireChanged(DgEmpire empire)
{
	//qDebug() << "AuSceneServerApp::EmpireChanged" << empire.m_id << empire.Name();
	//Q_ASSERT(!g_Au.IsSceneMaster());

	// Update my local empire record.
	DgData& gd = g_Au.LockDataForWrite();
	gd.ReplaceEmpire(empire);					// Add/replace an empire record.
	g_Au.UnlockData();

	if(g_Au.IsSceneWorker())
	{
		// Notify the clients.
		emit EmpireRecordAvailable(empire.m_id);
	}
	else
	{
		// Propagate the notification down to subordinate servers.
		emit EmpireDoneChanging(empire);
	}
}

void AuSceneServerApp::EmpireRecordDeleted(quint32 idEmpire)
{
	if(!idEmpire)
		return;
	// The master initiates this broadcast to all subordinate servers after an empire record has been deleted from the database.
	if(g_Au.IsSceneWorker())
	{
		// Destroy/reassign all things that belong to 'idEmpire'.
		AuEther& ether = g_Au.LockEtheriumDelete();
		ether.EmpireDeleted(idEmpire);
		g_Au.UnlockEtheriumDelete();

		// Worker servers remove the empire record from their local data.
		DgData& gd = g_Au.LockDataForWrite();
		DgEmpireHash::iterator ei = gd.m_gEmpires.find(idEmpire);
		if(ei != gd.m_gEmpires.end())
			gd.m_gEmpires.erase(ei);
		g_Au.UnlockData();
	}
	else
	{
		// Boss servers propagate this notification by sending a message to all their subordinates.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_EmpireDeleted;
		out << idEmpire;

		const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
		for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = servers.cbegin(); it != servers.cend(); it++)
			if((*it).second->isRunning())
				(*it).second->SendPacket(ba);
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
	}
}

void AuSceneServerApp::StoryRecordChangedDown(quint32 storyInstance)
{
	if(g_Au.IsSceneWorker())
	{
		// This is the end of the line for a signal propagated when a story changed. If the person in the story is on this server
		// we really don't want to reload the story or it could cause a slight rollback of the script, which is bad.
		AuIdList peopleInStory;
		AuStory* story = g_AuDb.LockStory(storyInstance);
		if(story)
			story->People(peopleInStory);	// We have the story instance locally. Get a list of the people who are in it.
		g_AuDb.UnlockStory(storyInstance);

		// Check to see if any of the people in the story are in my local data.
		bool okToLoadStory = true;
		if(!peopleInStory.isEmpty())
		{
			AuEther& ether = g_Au.LockEtheriumDelete();
			for(AuIdList::const_iterator ai = peopleInStory.cbegin(); ai != peopleInStory.cend(); ai++)
			{
				DaAtom* atom = ether.FindNormalAtom(*ai);
				if(atom)
				{
					okToLoadStory = false;
					break;
				}
			}
			g_Au.UnlockEtheriumDelete();
		}

		// Worker server signals db thread to reload story instance record.
		if(okToLoadStory)
			g_Au.ReloadStory(storyInstance);
	}
	else
	{
		// Broadcast a notification to all sub servers.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_ReloadStory;
		out << storyInstance;
		const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
		for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = servers.cbegin(); it != servers.cend(); it++)
			if((*it).second->isRunning())
				(*it).second->SendPacket(ba);
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
	}
}

void AuSceneServerApp::StoryRecordChangedUp(quint32 storyInstance)
{
	if(!m_tClientChannel)
		return;
	// Sub servers send this message up to their boss.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtServerChannel::P_StoryChanged;
	out << storyInstance;
	m_tClientChannel->SendPacket(ba);
}

void AuSceneServerApp::StoryRecordDeletedDown(quint32 storyInstance)
{
	if(g_Au.IsSceneWorker())
	{
		// This chain of communications started when the db thread deleted the record on a worker server.
		// It emitted a signal that ends up here on every worker server, to clean up local data.
		// Worker server signals db thread to remove story instance record from local data; the record was already deleted.
		g_Au.RemoveStory(storyInstance);

		// What happens to the atoms and inventory items?
		// Those things self destruct when their associated story instance no longer exists.
	}
	else
	{
		// Broadcast a notification to all sub servers.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_RemoveStory;
		out << storyInstance;
		const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
		for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = servers.cbegin(); it != servers.cend(); it++)
			if((*it).second->isRunning())
				(*it).second->SendPacket(ba);
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
	}
}

void AuSceneServerApp::StoryRecordDeletedUp(quint32 storyInstance)
{
	if(!m_tClientChannel)
		return;
	// Sub servers send this message up to their boss.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtServerChannel::P_StoryDeleted;
	out << storyInstance;
	m_tClientChannel->SendPacket(ba);
}

void AuSceneServerApp::ChatReceived(QByteArray bamsg)
{
	// Extract the packet from the byte array.
	QDataStream in(bamsg);
	in.setVersion(QDataStream::Qt_4_1);
	quint8 msgType;
	in >> msgType;
	AuCommPacket* msgPacket = AuCommPacket::CreateCommPacket((AuCommPacket::TYPE)msgType);
	in >> *msgPacket;
	if(msgPacket)
	{
		// Ok to propagate up to the manager server based on range.
		Chat(*msgPacket, true);
		delete msgPacket;
		msgPacket = 0;
	}
}

// Called on a worker to send a chat message to all relevant clients.
// 'propagateUp' is true when it's ok to send messages to the manager for
// further propagation. false is passed when the message is propagating back
// down the server tree.
void AuSceneServerApp::Chat(const AuCommPacket& msg, bool propagateUp)
{
	if(!g_Au.IsSceneWorker())
		return;
	//qDebug() << "AuSceneServerApp::Chat" << msg.Type() << "from" << msg.FromId() << msg.FromName()
		//<< "range" << msg.TransmissionRange() << msg.Text() << "propagateUp" << propagateUp;

	// Do not propagate the detonate message to players or to other servers.
	if(msg.Type() != AuCommPacket::Detonate)
	{
		// Send the message to chat channels who are connected right now.
		// Bundle it up into a byte array so it can be emitted as a signal.
		QByteArray bamsg;
		QDataStream out(&bamsg, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)msg.Type();
		out << msg;
		emit ChatToClients(bamsg);

		// Remember the message for people who are traveling right now.
		g_AuDb.RecentAtomChat(msg);

		// Range determines whether the chat message is propagated up to my manager.
		// Chat messages are not propagated up when they are on their way down from my manager.
		if(propagateUp && m_tClientChannel &&
				(msg.TransmissionRange() == AuCommStdChannel::Sector ||
				 msg.TransmissionRange() == AuCommStdChannel::Galaxy ||
				 msg.TransmissionRange() == AuCommStdChannel::Universe))
		{
			// Queue a message for my manager server.
			Q_ASSERT(!msg.IsDataLarge());	// No large binary chat messages are expected to be propagated beyond the solar system.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_Chat;
			out << (quint8)msg.Type();
			out << msg;
			m_tClientChannel->SendPacket(ba);
		}
	}

	// Process messages that affect local atoms merely by receiving them.
	// For example, a berth accepted message must cause the berth requestor to update its spawn ship id.
	// Another example, contacting a ship that has gone dormant after destruction.
	AuId toId = msg.ToId();		// Some messages are addressed to a specific recipient.
	switch(msg.Type())
	{
	default:
		// Most message types do not need any special processing here.
		break;

	case AuCommPacket::CitySurrenderOffered:
		try
		{
			const AuCommCitySurrenderOffered& msgSurrender = dynamic_cast<const AuCommCitySurrenderOffered&>(msg);
			bool isPirate = DgEmpire::IsPirate(msgSurrender.ToEmpireId());
			if(isPirate && msg.FromEmpireId() != msg.ToEmpireId())
			{
				// Pirate empire accepts all surrendered cities immediately.
				AuIdList cityIds;
				msgSurrender.CityIds(cityIds);

				// Get crecentials for fake emperor of the pirate empire.
				AuId pirateEmperorId;
				QString pirateEmperorName;
				const DgData& gd = g_Au.LockDataForRead();
				gd.PirateEmperor(msgSurrender.ToEmpireId(), pirateEmperorId, pirateEmperorName);
				g_Au.UnlockData();

				AuCommCitySurrenderAccepted msgAccept(msgSurrender.ToEmpireId(), msg.FromId(), msgSurrender.FromEmpireId(), cityIds,
						msgSurrender.ChannelName(), pirateEmperorId, pirateEmperorName, msgSurrender.FromSectorId(),
						msgSurrender.FromGalaxy(), AuCommStdChannel::SolarSystem, msgSurrender.Scope());
				g_Au.SendAtomChat(msgAccept);

				// Surrender the cities.
				for(AuIdList::const_iterator ci = cityIds.cbegin(); ci != cityIds.cend(); ci++)
					GovCitySurrenderedUp(msg.FromId(), *ci, msg.FromEmpireId(), msg.ToEmpireId());
			}
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
		break;

	case AuCommPacket::Chat:
		try
		{
			AuEther& ether = g_Au.LockEtheriumDelete();
			const AuCommChat& msgChat = dynamic_cast<const AuCommChat&>(msg);
			if(AuCommStdChannel::IsVoiceChannel(msgChat.ChannelName()) &&
					(msgChat.Text().toUpper().left(5) == "HELLO" || DaSentient::ContainsEuphemismForSex(msgChat.Text())))
			{
				// Hello on voice channel wakes up AI in the area.
				const DaPerson* person = dynamic_cast<const DaPerson*>(ether.FindNormalAtom(msgChat.FromId()));
				if(person)
				{
					// Locate all AI within 10' that are in front of the sender.
					const DaAtom* envAtom = person->FindEnvironmentAtom();
					if(envAtom)
						envAtom->StartVoiceConversationRecursive(*person);	// Causes AI dudes to listen on the Voice channel.
				}
			}
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
		g_Au.UnlockEtheriumDelete();
		break;

	case AuCommPacket::HailRequest:
		try
		{
			AuEther& ether = g_Au.LockEtheriumDelete();
			const AuCommHailRequest& msgHail = dynamic_cast<const AuCommHailRequest&>(msg);
			if(AuCommStdChannel::IsVoiceChannel(msgHail.ChannelName()))
			{
				// Hail on voice channel wakes up AI in the area.
				const DaPerson* person = dynamic_cast<const DaPerson*>(ether.FindNormalAtom(msgHail.FromId()));
				if(person)
				{
					// Locate all AI within 10' that are in front of the sender.
					const DaAtom* envAtom = person->FindEnvironmentAtom();
					if(envAtom)
						envAtom->StartVoiceConversationRecursive(*person);	// Causes AI dudes to listen on the Voice channel.
				}
			}
			else if(msgHail.Scope() == AuCommStdChannel::CrewMates)
			{
				// Hail on crew channel might get a response from a dormant ship.
				DaVessel* ship = dynamic_cast<DaVessel*>(ether.FindDormantAtom(msgHail.FromShipId()));
				if(ship)
				{
					quint32 countSupplied = 0;
					bool firstInGroup = true;
					ship->RxChatMessage(&msg, countSupplied, firstInGroup);
				}
			}
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
		g_Au.UnlockEtheriumDelete();
		break;

	case AuCommPacket::ContactShip:
		try
		{
			AuEther& ether = g_Au.LockEtheriumDelete();
			const AuCommContactShip& msgContact = dynamic_cast<const AuCommContactShip&>(msg);
			if(msgContact.Scope() == AuCommStdChannel::CrewMates)
			{
				// Hail on crew channel might get a response from a dormant ship.
				DaVessel* ship = dynamic_cast<DaVessel*>(ether.FindDormantAtom(msgContact.FromShipId()));
				if(ship)
				{
					quint32 countSupplied = 0;
					bool firstInGroup = true;
					ship->RxChatMessage(&msg, countSupplied, firstInGroup);
				}
			}
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
		g_Au.UnlockEtheriumDelete();
		break;

	case AuCommPacket::AcceptBerthCaptain:
		// Berth acceptance message. See if the target of the message is in my local atom data.
		// The target of the message is the person who requested a berth in the first place.
		try
		{
			AuEther& ether = g_Au.LockEtheriumDelete();
			const AuCommAcceptBerthCaptain& msgAccept = dynamic_cast<const AuCommAcceptBerthCaptain&>(msg);
			DaPerson* person = dynamic_cast<DaPerson*>(ether.FindNormalAtom(msgAccept.UnitId()));
			if(person)
			{
				person->SetSpawnShip(msgAccept.ShipId(), msgAccept.ShipName(), AuRosterEntry::Captain,
						msgAccept.ShipIsPrivate());

				// A person who has spawn tied to a spacecraft must not have a null spawn city id any longer, which is our
				// indicator of a nubie emperor who hasn't planted their flag yet. They will now be treated like a regular
				// player rather than a nu emperor when they die.
				if(person->IsNewbieEmperor())
					person->ClearSpawnCity();

				person->SpacecraftCommanded(msgAccept.ShipId());

				person->SetModified();
			}
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
		g_Au.UnlockEtheriumDelete();
		break;

	case AuCommPacket::AcceptBerthCaptainSlot:
		// Berth acceptance message. See if the target of the message is in my local atom data.
		// The target of the message is the person who requested a berth in the first place.
		try
		{
			AuEther& ether = g_Au.LockEtheriumDelete();
			const AuCommAcceptBerthCaptainSlot& msgAccept = dynamic_cast<const AuCommAcceptBerthCaptainSlot&>(msg);
			DaPerson* person = dynamic_cast<DaPerson*>(ether.FindNormalAtom(msgAccept.UnitId()));
			if(person)
			{
				person->SetSpawnShip(msgAccept.ShipId(), msgAccept.ShipName(), AuRosterEntry::CaptainSlot,
						msgAccept.ShipIsPrivate());

				// A person who has spawn tied to a spacecraft must not have a null spawn city id any longer, which is our
				// indicator of a nubie emperor who hasn't planted their flag yet. They will now be treated like a regular
				// player rather than a nu emperor when they die.
				if(person->IsNewbieEmperor())
					person->ClearSpawnCity();

				person->SpacecraftCommanded(msgAccept.ShipId());

				person->SetModified();
			}
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
		g_Au.UnlockEtheriumDelete();
		break;

	case AuCommPacket::AcceptBerthOfficer:
		// Berth acceptance message. See if the target of the message is in my local atom data.
		// The target of the message is the person who requested a berth in the first place.
		try
		{
			AuEther& ether = g_Au.LockEtheriumDelete();
			const AuCommAcceptBerthOfficer& msgAccept = dynamic_cast<const AuCommAcceptBerthOfficer&>(msg);
			DaPerson* person = dynamic_cast<DaPerson*>(ether.FindNormalAtom(msgAccept.UnitId()));
			if(person)
			{
				person->SetSpawnShip(msgAccept.ShipId(), msgAccept.ShipName(), AuRosterEntry::OfficerHuman,
						msgAccept.ShipIsPrivate());

				// A person who has spawn tied to a spacecraft must not have a null spawn city id any longer, which is our
				// indicator of a nubie emperor who hasn't planted their flag yet. They will now be treated like a regular
				// player rather than a nu emperor when they die.
				if(person->IsNewbieEmperor())
					person->ClearSpawnCity();

				person->SpacecraftCommanded(msgAccept.ShipId());

				person->SetModified();
			}
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
		g_Au.UnlockEtheriumDelete();
		break;

	case AuCommPacket::AcceptBerthCrew:
		// Berth acceptance message. See if the target of the message is in my local atom data.
		// The target of the message is the person who requested a berth in the first place.
		try
		{
			AuEther& ether = g_Au.LockEtheriumDelete();
			const AuCommAcceptBerthCrew& msgAccept = dynamic_cast<const AuCommAcceptBerthCrew&>(msg);
			DaPerson* person = dynamic_cast<DaPerson*>(ether.FindNormalAtom(msgAccept.UnitId()));
			if(person)
			{
				person->SetSpawnShip(msgAccept.ShipId(), msgAccept.ShipName(), AuRosterEntry::Crew,
						msgAccept.ShipIsPrivate());

				// A person who has spawn tied to a spacecraft must not have a null spawn city id any longer, which is our
				// indicator of a nubie emperor who hasn't planted their flag yet. They will now be treated like a regular
				// player rather than a nu emperor when they die.
				if(person->IsNewbieEmperor())
					person->ClearSpawnCity();

				person->SetModified();
			}
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
		g_Au.UnlockEtheriumDelete();
		break;

	case AuCommPacket::AcceptBerthPassenger:
		// Berth acceptance message. See if the target of the message is in my local atom data.
		// The target of the message is the person who requested a berth in the first place.
		try
		{
			AuEther& ether = g_Au.LockEtheriumDelete();
			const AuCommAcceptBerthPassenger& msgAccept = dynamic_cast<const AuCommAcceptBerthPassenger&>(msg);
			DaPerson* person = dynamic_cast<DaPerson*>(ether.FindNormalAtom(msgAccept.UnitId()));
			if(person)
			{
				person->SetSpawnShip(msgAccept.ShipId(), msgAccept.ShipName(), AuRosterEntry::PassengerHuman,
						msgAccept.ShipIsPrivate());

				// A person who has spawn tied to a spacecraft must not have a null spawn city id any longer, which is our
				// indicator of a nubie emperor who hasn't planted their flag yet. They will now be treated like a regular
				// player rather than a nu emperor when they die.
				if(person->IsNewbieEmperor())
					person->ClearSpawnCity();

				person->SetModified();
			}
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
		g_Au.UnlockEtheriumDelete();
		break;

	case AuCommPacket::AcceptBerthTroop:
		// Berth acceptance message. See if the target of the message is in my local atom data.
		// The target of the message is the person who requested a berth in the first place.
		try
		{
			AuEther& ether = g_Au.LockEtheriumDelete();
			const AuCommAcceptBerthTroop& msgAccept = dynamic_cast<const AuCommAcceptBerthTroop&>(msg);
			DaPerson* person = dynamic_cast<DaPerson*>(ether.FindNormalAtom(msgAccept.UnitId()));
			if(person)
			{
				person->SetSpawnShip(msgAccept.ShipId(), msgAccept.ShipName(), AuRosterEntry::Troop,
						msgAccept.ShipIsPrivate());

				// A person who has spawn tied to a spacecraft must not have a null spawn city id any longer, which is our
				// indicator of a nubie emperor who hasn't planted their flag yet. They will now be treated like a regular
				// player rather than a nu emperor when they die.
				if(person->IsNewbieEmperor())
					person->ClearSpawnCity();

				person->SetModified();
			}
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
		g_Au.UnlockEtheriumDelete();
		break;
	}

	// Process server activities specific to the message type before allowing local atoms to see the message.
	// This includes bid and berth acceptance type activities.
	// This is so a nu bid placed can be accepted and removed from the bid memory properly.
	// To prevent abuse, we remember the last bid placed on each channel by each person.
	// These are considered "active" until they decay after some time.
	// A RevokeBid message will remove the active bid from the sender on that channel.
	// An Accept message will remove the active bid when completing the transaction.
	// A city will remove the active bid if it accepts the offer right away.
	DaAtom* pfrom = 0;
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaAtom* fromSector = ether.FindNormalAtom(msg.FromSectorId());
	if(fromSector && !msg.FromId().IsNull())
		pfrom = fromSector->FindIdRecursive(msg.FromId());
	DaPerson* fromPerson = dynamic_cast<DaPerson*>(pfrom);	// May be null
	if(pfrom)
	{
		DaVessel* fromShip = dynamic_cast<DaVessel*>(pfrom);
		switch(msg.Type())
		{
		case AuCommPacket::CitySurrenderAccepted:
			if(fromPerson)
			{
				try
				{
					const AuCommCitySurrenderAccepted& msgAccept = dynamic_cast<const AuCommCitySurrenderAccepted&>(msg);
					CommAcceptCitySurrender(msgAccept);
				}
				catch(const bad_cast&)
				{
					// dynamic_cast failed
				}
			}
			break;

		case AuCommPacket::Accept:	// Bid accepted, perform the transaction.
			// This Accept code only handles transactions accepted by people.
			// Cities Accept bids in DaBd::RxChatMessage when the bid arrives.
			if(fromPerson)
			{
				try
				{
					const AuCommAccept& msgAccept = dynamic_cast<const AuCommAccept&>(msg);
					fromPerson->CommAccept(msgAccept);
				}
				catch(const bad_cast&)
				{
					// dynamic_cast failed
				}
			}
			break;

		case AuCommPacket::AcceptBarter:	// Bid accepted, perform the transaction.
			// This Accept code only handles transactions accepted by people.
			// Cities Accept bids in DaBd::RxChatMessage when the bid arrives.
			if(fromPerson)
			{
				try
				{
					const AuCommAcceptBarter& msgAccept = dynamic_cast<const AuCommAcceptBarter&>(msg);
					fromPerson->CommAcceptBarter(msgAccept);
				}
				catch(const bad_cast&)
				{
					// dynamic_cast failed
				}
			}
			break;
		case AuCommPacket::AcceptRepairAll:
		case AuCommPacket::AcceptRepairSystem:
			// This AcceptRepairXXX code only handles repair offers accepted by people.
			// Cities and spacecraft do not accept repair offers.
			if(fromPerson)
			{
				try
				{
					const AuCommAcceptRepair& msgAccept = dynamic_cast<const AuCommAcceptRepair&>(msg);
					fromPerson->CommAcceptRepair(msgAccept);
				}
				catch(const bad_cast&)
				{
					// dynamic_cast failed
				}
			}
			break;
		case AuCommPacket::Bid:									// Bid placed, money's on the table.
		case AuCommPacket::BidBarter:							// BidBarter placed, money's on the table.
		case AuCommPacket::BidPickupShipment:
		case AuCommPacket::BoardPassengers:
		case AuCommPacket::BoardCrew:
		case AuCommPacket::BoardCryo:
		case AuCommPacket::BoardOfficers:
		case AuCommPacket::BoardTroops:
		case AuCommPacket::DebarkCryo:
		case AuCommPacket::DebarkPassengers:
		case AuCommPacket::OfferRepairAll:
		case AuCommPacket::OfferRepairSystem:
			// - Bids are monitored so they can decay and so we can insure that they only get accepted once. They can also be
			// revoked.
			// - BoardPassengers and BoardTroops requests are treated like a bid because they represents an offer that can only be
			// accepted once. In this case, the offer is for a certain number of empty berths and we don't want multiple people to
			// be able to accept the message.
			// - DebarkPassengers announcement is treated like a bid because it represents an offer that can only be accepted once.
			// In this case, the offer is for a certain number of free passengers and we don't want multiple people to be able to
			// accept the message.
			// - OfferRepairXXX because it is an offer that can only be accepted once.
			g_Au.BidPlaced(&msg);
			break;
		case AuCommPacket::RevokeBid:	// Bid revoked, offer's no longer valid.
			// This also revokes LoadPassengersoffers, which are treated like bids.
			g_Au.BidRevoked(msg.FromId(), msg.ChannelName());
			break;
		case AuCommPacket::AcceptInviteToEmpire:
			if(fromPerson)
				fromPerson->CommAcceptInvitation(msg);
			break;
		case AuCommPacket::AcceptTeleport:
			if(fromPerson)
				CommAcceptTeleport(fromPerson, msg);
			break;
		case AuCommPacket::AcceptAvatar:
			try
			{
				// Get the avatar information from the message.
				const AuCommAcceptAvatar& msgAccept = dynamic_cast<const AuCommAcceptAvatar&>(msg);
				AuId avatarId = msgAccept.AvatarId();
				if(fromPerson)
				{
					// A local person accepted the offer of an avatar.
					QString avatarName = msgAccept.AvatarName();

					// Transfer the avatar to the accepter's account.
					AvatarAccepted(avatarId, avatarName, fromPerson->Id(), fromPerson->Name());
				}

				// Disconnect local avatar whose transfer offer was accepted. The accepting avatar could be on another server.
				emit DisconnectUnitTransferred(avatarId);
			}
			catch(const bad_cast&)
			{
				// dynamic_cast failed
			}
			break;

		case AuCommPacket::AcceptBerthCaptain:
			// A local atom accepted a request for the captain's berth.
			// The acceptor can only be a ship. Ships check for same galaxy before accepting; I don't have to do that here.
			// People do not accept captain's berths.
			if(fromShip)
			{
				try
				{
					const AuCommAcceptBerth& msgAccept = dynamic_cast<const AuCommAcceptBerth&>(msg);
					if(pfrom->CommAcceptBerth(AuRosterEntry::Captain, msgAccept))
					{
						// Remove the requestor from the roster of their previous ship. The ship may be on another server.
						// ReleaseBerth gets the id of the nu ship, not the ship being released. This helps insure this data
						// doesn't get out of sync. The logic causes all ships that are not the ship id to remove the requestor's
						// id from their roster, if it's there.
						ReleaseBerthUp(msgAccept.UnitId(), fromShip->Id());
					}
				}
				catch(const bad_cast&)
				{
					// dynamic_cast failed
				}
			}
			break;
		case AuCommPacket::AcceptBerthCaptainSlot:
			// A local atom accepted a request for the captain's slot.
			// The acceptor can only be a ship. Ships check for same galaxy before accepting; I don't have to do that here.
			// People do not accept captain's berths.
			if(fromShip)
			{
				try
				{
					const AuCommAcceptBerth& msgAccept = dynamic_cast<const AuCommAcceptBerth&>(msg);
					if(pfrom->CommAcceptBerth(AuRosterEntry::CaptainSlot, msgAccept))
					{
						// Remove the requestor from the roster of their previous ship. The ship may be on another server.
						// ReleaseBerth gets the id of the nu ship, not the ship being released. This helps insure this data
						// doesn't get out of sync. The logic causes all ships that are not the ship id to remove the requestor's
						// id from their roster, if it's there.
						ReleaseBerthUp(msgAccept.UnitId(), fromShip->Id());
					}
				}
				catch(const bad_cast&)
				{
					// dynamic_cast failed
				}
			}
			break;
		case AuCommPacket::AcceptBerthOfficer:
			// A local atom accepted a request for a berth.
			// The acceptor is either the ship or is aboard the ship.
			if(fromPerson || fromShip)
			{
				try
				{
					const AuCommAcceptBerth& msgAccept = dynamic_cast<const AuCommAcceptBerth&>(msg);
					if(pfrom->CommAcceptBerth(AuRosterEntry::OfficerHuman, msgAccept))
					{
						// Remove the requestor from the roster of their previous ship. The ship may be on another server.
						// ReleaseBerth gets the id of the nu ship, not the ship being released. This helps insure this data
						// doesn't get out of sync. The logic causes all ships that are not the ship id to remove the requestor's
						// id from their roster, if it's there.
						if(!fromShip)
							fromShip = pfrom->FindParentVesselInEnv();
						if(fromShip)
							ReleaseBerthUp(msgAccept.UnitId(), fromShip->Id());
					}
				}
				catch(const bad_cast&)
				{
					// dynamic_cast failed
				}
			}
			break;
		case AuCommPacket::AcceptBerthCrew:
			// A local atom accepted a request for a berth.
			// The acceptor is either the ship or is aboard the ship.
			if(fromPerson || fromShip)
			{
				try
				{
					const AuCommAcceptBerth& msgAccept = dynamic_cast<const AuCommAcceptBerth&>(msg);
					if(pfrom->CommAcceptBerth(AuRosterEntry::Crew, msgAccept))
					{
						// Remove the requestor from the roster of their previous ship. The ship may be on another server.
						// ReleaseBerth gets the id of the nu ship, not the ship being released. This helps insure this data
						// doesn't get out of sync. The logic causes all ships that are not the ship id to remove the requestor's
						// id from their roster, if it's there.
						if(!fromShip)
							fromShip = pfrom->FindParentVesselInEnv();
						if(fromShip)
							ReleaseBerthUp(msgAccept.UnitId(), fromShip->Id());
					}
				}
				catch(const bad_cast&)
				{
					// dynamic_cast failed
				}
			}
			break;
		case AuCommPacket::AcceptBerthPassenger:
			// A local atom accepted a request for a berth.
			// The acceptor is either the ship or is aboard the ship.
			if(fromPerson || fromShip)
			{
				try
				{
					const AuCommAcceptBerth& msgAccept = dynamic_cast<const AuCommAcceptBerth&>(msg);
					if(pfrom->CommAcceptBerth(AuRosterEntry::PassengerHuman, msgAccept))
					{
						// Remove the requestor from the roster of their previous ship. The ship may be on another server.
						// ReleaseBerth gets the id of the nu ship, not the ship being released. This helps insure this data
						// doesn't get out of sync. The logic causes all ships that are not the ship id to remove the requestor's
						// id from their roster, if it's there.
						if(!fromShip)
							fromShip = pfrom->FindParentVesselInEnv();
						if(fromShip)
							ReleaseBerthUp(msgAccept.UnitId(), fromShip->Id());
					}
				}
				catch(const bad_cast&)
				{
					// dynamic_cast failed
				}
			}
			break;
		case AuCommPacket::AcceptBerthTroop:
			// A local atom accepted a request for a berth.
			// The acceptor is either the ship or is aboard the ship.
			if(fromPerson || fromShip)
			{
				try
				{
					const AuCommAcceptBerth& msgAccept = dynamic_cast<const AuCommAcceptBerth&>(msg);
					if(pfrom->CommAcceptBerth(AuRosterEntry::Troop, msgAccept))
					{
						// Remove the requestor from the roster of their previous ship. The ship may be on another server.
						// ReleaseBerth gets the id of the nu ship, not the ship being released. This helps insure this data
						// doesn't get out of sync. The logic causes all ships that are not the ship id to remove the requestor's
						// id from their roster, if it's there.
						if(!fromShip)
							fromShip = pfrom->FindParentVesselInEnv();
						if(fromShip)
							ReleaseBerthUp(msgAccept.UnitId(), fromShip->Id());
					}
				}
				catch(const bad_cast&)
				{
					// dynamic_cast failed
				}
			}
			break;
		case AuCommPacket::CryoBoarded:
			// This code only handles BoardCryo requests accepted by
			// people, which generates a CryoBoarded message. The person
			// who sends the CryoBoarded message is willing to unload
			// passengers to the person id identified in the message text.
			// Cities load passengers in DaBd::RxChatMessage when the
			// BoardCryo message arrives.
			if(fromPerson)
			{
				try
				{
					const AuCommCryoBoarded& msgAccept = dynamic_cast<const AuCommCryoBoarded&>(msg);
					CommAcceptCryoBoarded(fromPerson, msgAccept);
				}
				catch(const bad_cast&)
				{
					// dynamic_cast failed
				}
			}
			break;
		case AuCommPacket::CryoDebarked:
			// This code only handles DebarkCryo notices accepted by
			// people, which generates a CryoDebarked message. The person
			// who sends the CryoDebarked message is willing to load
			// passengers from the person id identified in the message text.
			// Cities accept passengers in DaBd::RxChatMessage when the
			// DebarkCryo message arrives.
			if(fromPerson)
			{
				try
				{
					const AuCommCryoDebarked& msgAccept = dynamic_cast<const AuCommCryoDebarked&>(msg);
					CommAcceptCryoDebarked(fromPerson, msgAccept);
				}
				catch(const bad_cast&)
				{
					// dynamic_cast failed
				}
			}
			break;
		case AuCommPacket::PassengersBoarded:
			// This code only handles BoardPassengers requests accepted by
			// people, which generates a PassengersBoarded message. The person
			// who sends the PassengersBoarded message is willing to unload
			// passengers to the person id identified in the message text.
			// Cities load passengers in DaBd::RxChatMessage when the
			// BoardPassengers message arrives.
			if(fromPerson)
			{
				try
				{
					const AuCommPassengersBoarded& msgAccept = dynamic_cast<const AuCommPassengersBoarded&>(msg);
					CommAcceptColonistsBoarded(fromPerson, msgAccept);
				}
				catch(const bad_cast&)
				{
					// dynamic_cast failed
				}
			}
			break;
		case AuCommPacket::PassengersDebarked:
			// This code only handles DebarkPassengers notices accepted by
			// people, which generates a PassengersDebarked message. The person
			// who sends the PassengersDebarked message is willing to load
			// passengers from the person id identified in the message text.
			// Cities accept passengers in DaBd::RxChatMessage when the
			// DebarkPassengers message arrives.
			if(fromPerson)
			{
				try
				{
					const AuCommPassengersDebarked& msgAccept = dynamic_cast<const AuCommPassengersDebarked&>(msg);
					CommAcceptColonistsDebarked(fromPerson, msgAccept);
				}
				catch(const bad_cast&)
				{
					// dynamic_cast failed
				}
			}
			break;
		case AuCommPacket::TroopsBoarded:
			// This code only handles BoardTroops requests accepted by
			// people, which generates a TroopsBoarded message. The person
			// who sends the TroopsBoarded message is willing to unload
			// troops to the person id identified in the message text.
			// Cities load troops in DaBd::RxChatMessage when the
			// BoardTroops message arrives.
			if(fromPerson)
			{
				try
				{
					const AuCommTroopsBoarded& msgAccept = dynamic_cast<const AuCommTroopsBoarded&>(msg);
					CommAcceptTroopsBoarded(fromPerson, msgAccept);
				}
				catch(const bad_cast&)
				{
					// dynamic_cast failed
				}
			}
			break;

		case AuCommPacket::AcceptPickupShipment:
		case AuCommPacket::AcceptRefit:
		case AuCommPacket::AcceptUpgradeInstall:
		case AuCommPacket::Announcement:
		case AuCommPacket::Auction:
		case AuCommPacket::BeamAtom:
		case AuCommPacket::BeamCoordinate:
		case AuCommPacket::BeamOver:
		case AuCommPacket::BeamPlanet:
		case AuCommPacket::BeamShip:
		case AuCommPacket::BeamTransporter:
		case AuCommPacket::BeamVehicleAtom:
		case AuCommPacket::BeamVehicleCoordinate:
		case AuCommPacket::BeamVehicleOver:
		case AuCommPacket::BeamVehiclePlanet:
		case AuCommPacket::BeamVehicleShip:
		case AuCommPacket::BeamVehicleTransporter:
		case AuCommPacket::ChannelOpened:
		case AuCommPacket::Chat:
		case AuCommPacket::ChatAudio:
		case AuCommPacket::ChatChoice:
		case AuCommPacket::ChatContinued:
		case AuCommPacket::ChatImage:
		case AuCommPacket::ChatStory:
		case AuCommPacket::ChoicePicked:
		case AuCommPacket::CitizenOrder:
		case AuCommPacket::CitizenPosture:
		case AuCommPacket::CityChat:
		case AuCommPacket::CityMessage:
		case AuCommPacket::CitySurrenderOffered:
		case AuCommPacket::Combine:
		case AuCommPacket::ContactReply:
		case AuCommPacket::ContactShip:
		case AuCommPacket::CrewBoarded:
		case AuCommPacket::CrewBoardingComplete:
		case AuCommPacket::CryoBoardingComplete:
		case AuCommPacket::DamageInflicted:
		case AuCommPacket::Detonate:
		case AuCommPacket::DiplomaticMessage:
		case AuCommPacket::FiringAtTarget:
		case AuCommPacket::ForSale:
		case AuCommPacket::HailReply:
		case AuCommPacket::HailRequest:
		case AuCommPacket::Harvest:
		case AuCommPacket::InventoryItem:
		case AuCommPacket::InviteTeleport:
		case AuCommPacket::InviteToEmpire:
		case AuCommPacket::Jettison:
		case AuCommPacket::Message:
		case AuCommPacket::MissionNew:
		case AuCommPacket::MissionOrderAdd:
		case AuCommPacket::MissionOrderInsert:
		case AuCommPacket::MissionOrderNew:
		case AuCommPacket::MissionProperty:
		case AuCommPacket::Narrate:
		case AuCommPacket::NarrateAudio:
		case AuCommPacket::NarrateChoice:
		case AuCommPacket::NarrateImage:
		case AuCommPacket::NarrateTitle:
		case AuCommPacket::NarrateVideo:
		case AuCommPacket::OfferAvatar:
		case AuCommPacket::OfferRefit:
		case AuCommPacket::OfferTEU:
		case AuCommPacket::OfferUpgradeInstall:
		case AuCommPacket::OfficerBoarded:
		case AuCommPacket::OpenTEU:
		case AuCommPacket::PassengerBoardingComplete:
		case AuCommPacket::PreBoard:
		case AuCommPacket::RepairPartsMissing:
		case AuCommPacket::RequestBerthAny:
		case AuCommPacket::RequestBerthCaptain:
		case AuCommPacket::RequestBerthCaptainSlot:
		case AuCommPacket::RequestBerthCrew:
		case AuCommPacket::RequestBerthOfficer:
		case AuCommPacket::RequestBerthPassenger:
		case AuCommPacket::RequestBerthTroop:
		case AuCommPacket::RequestExterminator:
		case AuCommPacket::RequestHelp:
		case AuCommPacket::RequestHouseKeeping:
		case AuCommPacket::RequestInventory:
		case AuCommPacket::RequestRefit:
		case AuCommPacket::RequestRepairAll:
		case AuCommPacket::RequestRepairSystem:
		case AuCommPacket::RequestUpgradeSystem:
		case AuCommPacket::SOSAttack:
		case AuCommPacket::SOSFuelMarooned:
		case AuCommPacket::SOSFuelShip:
		case AuCommPacket::SOSFuelStation:
		case AuCommPacket::SOSManeuverDriveAdrift:
		case AuCommPacket::SOSManeuverDriveMarooned:
		case AuCommPacket::SOSManeuverDriveStation:
		case AuCommPacket::ShipStasisWait:
		case AuCommPacket::ShipStasisDimension:
		case AuCommPacket::ShipmentItem:
		case AuCommPacket::Silverwill:
		case AuCommPacket::SolicitAllSales:
		case AuCommPacket::SolicitAmmoSales:
		case AuCommPacket::SolicitArmorSales:
		case AuCommPacket::SolicitConstructionMaterialSales:
		case AuCommPacket::SolicitFoodSales:
		case AuCommPacket::SolicitFuelSales:
		case AuCommPacket::SolicitItemSales:
		case AuCommPacket::SolicitMedSales:
		case AuCommPacket::SolicitMiscSales:
		case AuCommPacket::SolicitNaturalResourceSales:
		case AuCommPacket::SolicitPartsSales:
		case AuCommPacket::SolicitPatentSales:
		case AuCommPacket::SolicitShipments:
		case AuCommPacket::SolicitToolSales:
		case AuCommPacket::SolicitWeaponSales:
		case AuCommPacket::SpotTargetOrTerrain:
		case AuCommPacket::Thought:
		case AuCommPacket::ThoughtMailAlert:
		case AuCommPacket::ThoughtMessage:
		case AuCommPacket::TroopBoardingComplete:
		case AuCommPacket::CommTypes:
			break;
		}
	}

	// Process the message by local non-person atoms monitoring the channel regardless of range.
	// Ships need to be able to receive mission orders from beyond the solar system.
	// Person atoms were handled earlier when sending to all open tcp channels, which are the currently connected people.
	const AuCommStdChannel::SCOPE scope = msg.Scope();
	if(msg.RecipientCount())
	{
		// Citizen orders contain a list of the recipients in the message. Those are the specific atoms to locate,
		// regardless of what channels they might be listening on. This relieves passengers and troops from having to
		// monitor the Voice or Crew channels for orders.
		if(fromPerson)
		{
			// Orders are only processed when the sending person is in our local data because we only look for dudes in the
			// same solar system.
			DaSolarSystem* system = fromPerson->FindParentSolarSystem();
			if(system)
			{
				bool firstCrewToGetOrder = true;			// So only the first to get the order sends a reply.
				bool firstTroopToGetOrder = true;			// So only the first to get the order sends a reply.
				quint32 countSupplied = 0;						// n/a for these types of orders.
				AuIdList recipients = msg.Recipients();		// Grab a copy of the list of recipient ids.
				for(AuIdList::const_iterator doi = recipients.cbegin(); doi != recipients.cend(); doi++)
				{
					// AI controlled atoms don't receive messages beyond solar system range.
					if(*doi == msg.FromId())
						continue;	// Don't receive my own messages.
					// Search for the destination atom in the same solar system.
					DaAtom* pto = system->FindIdRecursive(*doi);
					if(!pto)
						continue;	// destination atom is not in the same system.
					if(!pto->IsNormal())
						continue;

					// We get here if its ok for the non-person atom to receive the message.
					DaNpc* npc = dynamic_cast<DaNpc*>(pto);
					if(npc)
					{
						if(npc->IsNpcTroop() && !npc->AssignedShip().IsNull())
						{
							// Only the first troop that is assigned to a ship responds to the order.
							pto->RxChatMessage(&msg, countSupplied, firstTroopToGetOrder);
							firstTroopToGetOrder = false;
							continue;
						}
						else if(npc->IsNpcCrew() && !npc->AssignedShip().IsNull())
						{
							// Only the first crew that is assigned to a ship responds to the order.
							// Since we get them in rank order, an officer should be the first to respond, ahead of this guy.
							pto->RxChatMessage(&msg, countSupplied, firstCrewToGetOrder);
							firstCrewToGetOrder = false;
							continue;
						}
						else if(npc->IsNpcOfficer() && !npc->AssignedShip().IsNull())
						{
							// Only the first officer that is assigned to a ship responds to the order.
							// Since we get them in rank order, the officer should be the the first to respond, ahead of
							// other crew members.
							pto->RxChatMessage(&msg, countSupplied, firstCrewToGetOrder);
							firstCrewToGetOrder = false;
							continue;
						}
					}

					// Each individual other unit responds to the order.
					pto->RxChatMessage(&msg, countSupplied, true);
				}

				// Player Statistics
				fromPerson->Stats().OrderSquad();
				fromPerson->IncreaseSkill(AuSkill::SquadLeader, 1);
			}
		}
	}
	else if(scope == AuCommStdChannel::Intercom)
	{
		// Messages sent on the intercom channel may need to be processed by the ship or vehicle itself.
		// These are handled here because the intercom channel is not listed as "monitored" like other channels.
		// The intercom channel is implicitly monitored by spacecraft and vehicles.
		if(fromPerson)
		{
			quint32 countSupplied = 0;								// n/a for these types of orders.
			DaVehicle* invehicle = dynamic_cast<DaVehicle*>(fromPerson->Parent());
			if(invehicle)
				invehicle->RxChatMessage(&msg, countSupplied);		// Intercom goes to parent vehicle first.
			else
			{
				DaCapsule* ship = fromPerson->FindParentCapsuleInEnv();
				if(ship)
					ship->RxChatMessage(&msg, countSupplied);		// Intercom goes to parent space ship.
			}
		}
	}
	else
	{
		// Search for local non-person message recipients that might be present in our data.
		if(msg.Type() == AuCommPacket::Detonate)
		{
			// Detonate message is processed by items in range. Items don't listen to comm channels so we have to proactively
			// search for the items that were armed by the message sender.
			if(fromPerson)
			{
				// The fromPerson is in our local data. We don't propagate detonate message beyond the system so this should
				// always be true.
				// Detonators only reach solar system range so limit searches to solar system.
				DaAtomList atoms;
				DaAtom* fromSystem = fromPerson->FindEnvironmentAtom();	// Limit detonation search to solar system.
				Q_ASSERT(fromSystem);
				fromSystem->ListNormalSubatomsRecursive(atoms);
				DaAtomList detatoms;
				for(DaAtomList::const_iterator ai = atoms.begin(); ai != atoms.end(); ai++)
				{
					DaItem* item = dynamic_cast<DaItem*>(*ai);
					if(!item)
						continue;
					if(!item->IsNormal())
						continue;	// ListNormalSubatomsRecursive lists normal atoms but those atoms could be killed in this loop.
					if(!item->IsDetonatorWeapon())
						continue;
					if(!item->IsArmedBy(msg.FromId()))
						continue;

					// Determine if item is capable of receiving the message based on range.
					if(msg.TransmissionRange() == AuCommStdChannel::Hail || msg.TransmissionRange() == AuCommStdChannel::Close)
					{
						// Calculate precise distance between the sender and the item.
						AuPoint fromloc = fromPerson->EnvLocation();
						AuPoint toloc = item->EnvLocation();
						double dist = fromloc.Distance(toloc);
						if(!msg.IsWithinRange(dist))
							continue;						// Range to atom test failed.
					}

					// Detonate the item.
					item->DetonateInitiate();
				}
			}
		}
		else
		{
			// Grab a copy of the open atom channels so we can unlock the mutex right away.
			// Otherwise, an atom cannot open a channel in response to receiving a message or we lock up.
			unordered_multimap<QString, AuCommChannelConnection, AuStringHash>& commChannels = g_Au.LockAtomCommChannels();
			int channelCount = commChannels.size();
			g_Au.UnlockAtomCommChannels();

			// 'countSupplied' is used when loading and unloading crew and passengers so multiple cities can take a crack at
			// taking/supplying some citizens until the request is filled.
			quint32 countSupplied = 0;
			for(int channelIndex=0; channelIndex < channelCount; channelIndex++)
			{
				AuId channelAtomId;	// Atom listening on the channel.
				unordered_multimap<QString, AuCommChannelConnection, AuStringHash>& commChannels = g_Au.LockAtomCommChannels();
				channelCount = commChannels.size();		// Update channel count.
				if(channelIndex < channelCount)
				{
					// Channel index is still within the limits of the open channels array.
					// Position the iterator at the index.
					unordered_multimap<QString, AuCommChannelConnection, AuStringHash>::const_iterator ci = commChannels.begin();
					for(int chani=0; chani<channelIndex; chani++, ci++);

					if((*ci).first == msg.ChannelName())
						channelAtomId = (*ci).second.m_idAtom;
				}
				g_Au.UnlockAtomCommChannels();
				if(channelAtomId.IsNull())
					continue;
				if(channelAtomId == msg.FromId())
					continue;	// Don't receive my own messages.

				// Search for the destination atom.
				DaAtom* pto = ether.FindNormalAtom(channelAtomId);
				if(!pto)
					continue;									// Destination atom is not in the local atom data.

				// People were handled in ChatToClients; don't send it to people again or those who are off-line.
				DaSentient* tocitizen = dynamic_cast<DaSentient*>(pto);
				if(tocitizen && !tocitizen->IsAIControlled())
					continue;

				// Determine if the atom is capable of receiving the message based on standard channel criteria.
				DaVessel* toship = dynamic_cast<DaVessel*>(pto);
				switch(scope)
				{
				case AuCommStdChannel::Everyone:	// Hail channel is limited only by range
				case AuCommStdChannel::Audible:		// Voice channel is limited by range and audibility.
				default:
					break;

				case AuCommStdChannel::Agent:
				case AuCommStdChannel::Architect:
				case AuCommStdChannel::Programmer:
					continue;					// No non-person atom is an agent.

				case AuCommStdChannel::CrewMates:	// Crew channel requires crew fellowship
					if(toship)
					{
						// A ship is monitoring the channel. Is the sender a member of the crew or a passenger?
						if(!toship->Is(msg.FromShipId()))
							continue;	// Sender is not spawn-tied to the ship.
					}
					else if(tocitizen)
					{
						// A citizen is monitoring the channel, probably a troop. Are they members of the same crew?
						if(tocitizen->AssignedShip() != msg.FromShipId())
							continue;	// Sender is not a crew member on the same ship as the citizen.
					}
					break;

				case AuCommStdChannel::Intercom:	// Intercom channel handled above.
					continue;

				case AuCommStdChannel::Company:		// Company channel goes to ships owned by someone.
					if(!toship || toship->OwnerId() != msg.FromOwnerId())
						continue;	// Company messages only go to ships of the same owner as the sender (besides people).
					break;

				case AuCommStdChannel::Fleet:		// Fleet channel goes to units in the same empire.
					if(!toship || toship->EmpireId() != msg.FromEmpireId())
						continue;	// Fleet messages only go to ships of the same empire as the sender (besides people).
					break;

				case AuCommStdChannel::Empire:		// Empire channel requires same empire.
					{
						bool empireOk = false;
						unsigned int empireId = 0;
						DaUnit* unit = dynamic_cast<DaUnit*>(pto);
						if(unit)
							empireId = unit->EmpireId();
						if(empireId == msg.FromEmpireId())
							empireOk = true;
						if(!empireOk)
							continue;	// Empire allegiance did not match exactly.
					}
					break;

				case AuCommStdChannel::Friends:	// Friend channel requires same empire or friendly empire.
					{
						bool empireOk = false;
						unsigned int empireId = 0;
						DaUnit* unit = dynamic_cast<DaUnit*>(pto);
						if(unit)
							empireId = unit->EmpireId();

						// Check political stance of sender's empire toward the unit's empire.
						POLITICAL_STANCE stanceTo = PoliticalStance(msg.FromEmpireId(), empireId);
						POLITICAL_STANCE stanceFrom = PoliticalStance(empireId, msg.FromEmpireId());
						if((stanceTo == POL_Friend || stanceTo == POL_Empire || stanceTo == POL_Vassal) &&
								(stanceFrom == POL_Friend || stanceFrom == POL_Empire || stanceFrom == POL_Vassal))
							empireOk = true;
						if(!empireOk)
							continue;	// political stance not friend
					}
					break;

				case AuCommStdChannel::Narrator:
					// Local non-person atoms do not receive Narrator scope messages.
					// People can't send narrator scope messages.
					continue;
				}

				// Determine if atom is capable of receiving the message based on range.
				if(msg.TransmissionRange() == AuCommStdChannel::Hail || msg.TransmissionRange() == AuCommStdChannel::Close)
				{
					// At this range, the sender person must be in our local data.
					if(fromPerson)
					{
						// Calculate precise distance between the sender and the atom.
						const DaAtom* env = fromPerson->FindEnvironmentAtom();
						if(!env)
							continue;
						AuPoint fromloc = fromPerson->EnvLocation();
						AuPoint toloc = pto->EnvLocation();
						double dist = fromloc.Distance(toloc);
						if(!msg.IsWithinRange(dist) && pto != env)
							continue;	// Range to atom test failed.
					}
					else
					{
						DaNpc* fromChr = dynamic_cast<DaNpc*>(pfrom);
						if(fromChr)
						{
							// Calculate precise distance between the sender and the atom.
							const DaAtom* env = fromChr->FindEnvironmentAtom();
							if(!env)
								continue;
							AuPoint fromloc = fromChr->EnvLocation();
							AuPoint toloc = pto->EnvLocation();
							double dist = fromloc.Distance(toloc);
							if(!msg.IsWithinRange(dist) && pto != env)
								continue;	// Range to atom test failed.
						}
						else
							continue;
					}
				}
				else if(msg.TransmissionRange() == AuCommStdChannel::SolarSystem)
				{
					// Solar system range messages must originate in the same solar system as the receiver.
					// At this range, the sender must be in our local data.
					if(pfrom)
					{
						const DaAtom* env = pfrom->FindEnvironmentAtom();
						if(pto->FindEnvironmentAtom() != env && pto != env)
							continue;
					}
					else
						continue;
				}
				else if(msg.TransmissionRange() == AuCommStdChannel::Sector)
				{
					// Sector range requires the sender sector and receiver sector to be adjacent.
					if(!fromSector)
						continue;								// From atom sector not on server. Too far away.

					// At this range, the sender person may not be in our local data.
					const DaSector* toSector = pto->FindParentSector();
					if(!toSector)
						continue;								// To atom not in a sector? Not likely.

					if(fromSector != toSector)
					{
						if(toSector->Parent() != fromSector->Parent())
							continue;							// The sectors are in different galaxies.

						// Check distance between receiving atom's sector and the sector that originated the message.
						// An adjacent sector will be 10 parsecs away; I'll check 11 in case of fuzzy errors.
						AuPoint vecToOrig = toSector->Location() - fromSector->Location();
						if(fabs(vecToOrig.x) > 11 || fabs(vecToOrig.y) > 11 || fabs(vecToOrig.z) > 11)
							continue;							// Origin sector is not adjacent to the receiving atom's sector.
					}
				}

				// We get here if its ok for the non-person atom to receive the message.
				// The sender may or may not be a person atom.
				pto->RxChatMessage(&msg, countSupplied);
			}

			// Revoke the bid if it was filled by any of the atoms we offered it to.
			// This only applies to certain types of bids, namely attempts to board and debark crew and passengers, so each city
			// can fill a portion of the request.
			if(countSupplied)
				g_Au.BidRevoked(msg.FromId(), msg.ChannelName());
		}
	}
	g_Au.UnlockEtheriumDelete();
}

// Called when a chat message is propagated up to this master or manager from a sub.
void AuSceneServerApp::ChatUp(const AuCommPacket& msg, DtServerChannel* fromChannel)
{
	//qDebug() << "AuSceneServerApp::ChatUp" << msg.Type() << "from" << msg.FromId() << msg.FromName()
		//<< "range" << msg.TransmissionRange() << msg.Text();

	// Scene manager propagates the message by sending it to the scene master
	// and to all its workers, except the worker who sent up the chat message.
	Q_ASSERT(!msg.IsDataLarge());	// No large binary chat messages are expected to be propagated beyond the solar system.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtServerChannel::P_Chat;
	out << (quint8)msg.Type();
	out << msg;

	// Queue the message for my master server if the range is galaxy-wide or sector-wide.
	if(g_Au.IsSceneMaster())
	{
		// Scene master maintains Galactic channel chat history, for appearance on the web site.
		if(msg.TransmissionRange() == AuCommStdChannel::Universe &&
				(AuCommStdChannel::IsGalacticChannel(msg.ChannelName()) ||
				 msg.Scope() == AuCommStdChannel::Thoughts))
			g_AuDb.RecentAtomChat(msg);
	}
	else if(g_Au.IsSceneManager() && m_tClientChannel &&
			(msg.TransmissionRange() == AuCommStdChannel::Sector ||
			 msg.TransmissionRange() == AuCommStdChannel::Galaxy ||
			 msg.TransmissionRange() == AuCommStdChannel::Universe))
		m_tClientChannel->SendPacket(ba);

	// Queue the message for my sub servers, except the one I got the message from.
	const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
	for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = servers.cbegin(); it != servers.cend(); it++)
	{
		if((*it).second->isFinished())
			continue;
		if((*it).second != fromChannel)
			(*it).second->SendPacket(ba);
	}
	m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
}

void AuSceneServerApp::ChatReceivedDown(QByteArray bamsg)
{
	// Extract the packet from the byte array.
	QDataStream in(bamsg);
	in.setVersion(QDataStream::Qt_4_1);
	quint8 msgType;
	in >> msgType;
	AuCommPacket* msgPacket = AuCommPacket::CreateCommPacket((AuCommPacket::TYPE)msgType);
	in >> *msgPacket;
	if(msgPacket)
	{
		// Ok to propagate up to the manager server based on range.
		ChatDown(*msgPacket);
		delete msgPacket;
		msgPacket = 0;
	}
}

// Called when a chat message is propagated down to this manager or worker from a boss.
void AuSceneServerApp::ChatDown(const AuCommPacket& msg)
{
	// Queue the message for my sub servers.
	if(g_Au.IsSceneManager())
	{
		//qDebug() << "AuSceneServerApp::ChatDown" << msg.Type() << "from" << msg.FromId() << msg.FromName()
			//<< "range" << msg.TransmissionRange() << msg.Text();
		// Manager server propagates the message by sending it to all its workers.
		Q_ASSERT(!msg.IsDataLarge());	// No large binary chat messages are expected to be propagated beyond the solar system.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_Chat;
		out << (quint8)msg.Type();
		out << msg;
		const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
		for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = servers.cbegin(); it != servers.cend(); it++)
			if((*it).second->isRunning())
				(*it).second->SendPacket(ba);
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
	}
	else
	{
		// Worker server propagates the message by sending it to the clients that are supposed to receive the
		// message. No need to propagate it back up because its currently on the way down from above.
		Chat(msg, false);
	}
}

void AuSceneServerApp::OpenChannelUp(const AuId& fromUnitId, const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy,
		const AuId& toUnitId, const QString& channelName, AuCommStdChannel::SCOPE scope, DtServerChannel* fromChannel)
{
	// Scene manager propagates the channel-open by sending it to the scene master
	// and to all its workers, except the worker who sent up the channel-open message.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtServerChannel::P_OpenChannelTo;
	out << fromUnitId;
	out << fromName;
	out << fromSector;
	out << (quint8)fromGalaxy;
	out << toUnitId;
	out << channelName;
	out << (quint8)scope;

	// Queue the message for my master server.
	if(g_Au.IsSceneManager() && m_tClientChannel)
		m_tClientChannel->SendPacket(ba);

	// Queue the message for my sub servers, except the one I got the message from.
	const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
	for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = servers.cbegin(); it != servers.cend(); it++)
		if((*it).second != fromChannel)
			if((*it).second->isRunning())
				(*it).second->SendPacket(ba);
	m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
}

// Called when a channel-open request is propagated down to this manager or worker from a boss.
void AuSceneServerApp::OpenChannelDown(AuId fromUnitId, QString fromName, AuId fromSector, quint8 fromGalaxy, AuId toUnitId,
		QString channelName, quint8 scope)
{
	// Queue the message for my sub servers.
	if(g_Au.IsSceneManager())
	{
		// Manager server propagates the message by sending it to all its workers.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_OpenChannelTo;
		out << fromUnitId;
		out << fromName;
		out << fromSector;
		out << fromGalaxy;
		out << toUnitId;
		out << channelName;
		out << scope;
		const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
		for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = servers.cbegin(); it != servers.cend(); it++)
			if((*it).second->isRunning())
				(*it).second->SendPacket(ba);
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
	}
	else
	{
		// Worker server propagates the message by sending it to the clients and local atoms that are supposed to receive the
		// message. No need to propagate it back up because its currently on the way down from above.
		OpenChannelTo(fromUnitId, fromName, fromSector, fromGalaxy, toUnitId, channelName, (AuCommStdChannel::SCOPE)scope, false);
	}
}

void AuSceneServerApp::OpenChannelTo(AuId fromUnitId, QString fromName, AuId fromSector, quint8 fromGalaxy, AuId toUnitId,
		QString channelName, quint8 scope, bool propagateUp)
{
	//qDebug("OpenChannelTo channel '%s' target %d propagate %c", (const char*)channelName.toAscii(), idTargetAtom.GetID(),
			//propagateUp?'Y':'N');
	if(!g_Au.IsSceneWorker())
		return;

	// First try to locate the target atom among the clients that are present.
	emit OpenChannelToUnit(fromUnitId, fromName, fromSector, fromGalaxy, toUnitId, channelName, scope);

	// We get here if there is no active open client connection to the target atom.
	// Try to locate the target atom in my local data.
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaAtom* ptarget = ether.FindNormalAtom(toUnitId);
	if(ptarget && ptarget->IsNormal())
	{
		// The target is an atom in my local data.
		// This test insures that we only open channels to people and spacecraft across the galaxy, not cities or other atoms.
		// If the target atom is not a person or spacecraft, propagateUp must be true for us to open the channel.
		// It is only true when the worker receives the open channel request from one of its clients or from a spacecraft atom.
		// When the open message is propagated galaxy wide, propagateUp is false by the time it gets down to this worker.
		if(!dynamic_cast<DaPerson*>(ptarget))
		{
			// Do not call MonitorChannel for person atoms. DtChatChannel does this by catching the signal OpenChannelToUnit.
			//qDebug("AuSceneServerApp::OpenChannelTo found target person atom %s", (const char*)ptarget->Name().toAscii());
			if(propagateUp || dynamic_cast<DaVessel*>(ptarget))
			{
				//qDebug("AuSceneServerApp::OpenChannelTo found target atom %s", (const char*)ptarget->Name().toAscii());
				// 'propagateUp' true here catches city atoms on the way up but not on the way back down.
				// The space ship test catches spacecraft up and down the propagation.
				g_Au.MonitorChannel(channelName, toUnitId);
			}
		}

		// No need to propagate farther; we found the atom.
		propagateUp = false;
	}
	g_Au.UnlockEtheriumDelete();

	if(propagateUp && m_tClientChannel)
	{
		//qDebug("OpenChannelTo propagating open channel up to my manager");
		// We get here if the target atom was not found in my local data.
		// The channel open must be propagated up to my manager to locate the target atom elsewhere in the server farm.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_OpenChannelTo;
		out << fromUnitId;
		out << fromName;
		out << fromSector;
		out << fromGalaxy;
		out << toUnitId;
		out << channelName;
		out << scope;
		m_tClientChannel->SendPacket(ba);
	}
}

// Open channels. Send atom chat messages. Decay atom chat messages.
void AuSceneServerApp::AtomChat()
{
	// Get a copy of the array of channels we are supposed to open.
	// This mutex controls both the open channels queue and the chat messages queue.
	g_Au.LockAtomChat();
	QList<AuOpenChannel> channelsToOpen(g_Au.m_OpenChannels);
	g_Au.m_OpenChannels.clear();
	g_Au.UnlockAtomChat();

	// Open the channels that my local atoms want to open.
	for(QList<AuOpenChannel>::const_iterator it = channelsToOpen.cbegin(); it != channelsToOpen.cend(); it++)
	{
		//qDebug("AuSceneServerApp::AtomChat open channel '%s' from %d to %d propagate %c",
				//(const char*)(*it).m_ChannelName.toAscii(), (*it).m_idFrom.GetID(), (*it).m_idTo.GetID(),
				//(*it).m_ConnectBeyondSystem?'Y':'N');
		// First notify server that the local atom is monitoring a channel.
		g_Au.MonitorChannel((*it).m_ChannelName, (*it).m_idFrom);

		// Now notify the target of the opened channel.
		OpenChannelTo((*it).m_idFrom, (*it).m_FromName, (*it).m_FromSector, (*it).m_FromGalaxy, (*it).m_idTo, (*it).m_ChannelName,
				(*it).m_Scope, (*it).m_ConnectBeyondSystem);
	}

	// Send atom chat messages that have not yet been sent.
	bool sentMsg;
	do
	{
		// Locate a chat message to send.
		AuCommPacket* msg = 0;
		AuAtomChatList* chat = g_Au.LockAtomChat();
		for(AuAtomChatList::iterator it = chat->begin(); it != chat->end(); it++)
		{
			msg = (*it).CommPacket()->Clone();
			chat->erase(it);
			break;	// Break out to send this message. We'll come back for another one on the go-around.
		}
		g_Au.UnlockAtomChat();

		// Send the message without the atom chat mutex locked or we'll eventually deadlock.
		if(msg)
		{
			Chat(*msg, true);	// Send the chat message. It's allowed to propagate.
			delete msg;
			sentMsg = true;
		}
		else
			sentMsg = false;
	}
	while(sentMsg);		// Keep going until there are no messages left to send.

	// Decay atom chat messages that have gotten too old. This allows local atoms to store a short term memory of messages they
	// have sent.
	g_Au.DecayAtomChat();

	// Decay active bids that have gotten too old. This cleans the bid queue of old data.
	g_Au.DecayActiveBids();

	// Decay atom chat messages received recently. This allows warp travelers to pick up messages they might have missed.
	g_AuDb.DecayRecentAtomChat();
}

void AuSceneServerApp::NewSectorStored(AuId idSector)
{
	m_rwNewSectors.lockForWrite();
	int rem = m_NewSectors.indexOf(idSector);
	if(rem >= 0)
		m_NewSectors.removeAt(rem);
	m_rwNewSectors.unlock();
}

bool AuSceneServerApp::IsSectorNew(const AuId& idSector) const
{
	m_rwNewSectors.lockForRead();
	bool isNew = m_NewSectors.contains(idSector);
	m_rwNewSectors.unlock();
	return isNew;
}

// Handoff atoms that have accumulated information about desired destinations.
// This is called periodically.
void AuSceneServerApp::HandoffAtoms()
{
	// Scene worker doesn't do anything with spawn requests.
	if(g_Au.IsSceneWorker())
		return;

	// Array of spawn requests that must be reposted due to timeout.
	QList<AuHandoffRequest> repost;

	m_HandoffMutex.lock();
	if((g_Au.m_LogMessages & LOG_Travel) && !m_Handoffs.isEmpty())
		qDebug("AuSceneServerApp::HandoffAtoms %d atoms waiting for handoff", m_Handoffs.count());
	QList<AuHandoffRequest>::iterator it = m_Handoffs.begin();
	while(it != m_Handoffs.end())
	{
		if((*it).IsTimedOut())
		{
			// We get here if the handoff request timed out without finding our destination atom. This could happen if the
			// server assigned our capital city was still loading the database when the request arrived. It could also happen if
			// the assignment was revoked while we were searching. Because of that, scene managers send the spawn request back up
			// to the master to start the spawn point search all over again.
			(*it).m_TimeOuts++;

			if(g_Au.m_LogMessages & LOG_Travel)
			{
				QString logmsg;
				logmsg = tr("HandoffAtoms: Query for destination atom %1 timed out %2 times...retrying.")
					.arg((*it).m_Dest.GetID()).arg((*it).m_TimeOuts);
				g_Au.Log(logmsg);
			}

			if(g_Au.IsSceneMaster())
			{
				// Only the scene master does this.
				if((*it).m_LimboRescue)
				{
					// After 1 timeouts (1 minutes) without finding the destination atom, assume it is lost in limbo.
					// 4 minutes should be enough time to recover the atom from the database and transfer it somewhere viable.
					if((*it).m_TimeOuts == 1)
					{
						if(g_Au.m_LogMessages & LOG_Admin)
						{
							QString logmsg = QString("HandoffAtoms: Initiating limbo rescue for destination atom %2")
								.arg((*it).m_Dest.GetID());
							g_Au.Log(logmsg);
						}

						emit RecoverAtomFromLimbo((*it).m_Dest);	// Tell the database thread to recover the destination atom.

						// When the server manager requests a limbo rescue, there is no atom actually travelling.
						if((*it).m_PersonOrShip.IsNull())
						{
							// Remove the handoff request now and don't repost it.
							it = m_Handoffs.erase(it);
							continue;
						}
					}
					else if((*it).m_TimeOuts >= 3)
					{
						if((*it).m_ReturnOnFail.IsNull() || (*it).m_ReturnOnFail == (*it).m_Dest)
						{
							// Destination atom hasn't been found. Change handoff destination to a random capital city.
							// Find a random empire that has a capital.
							const DgData& gd = g_Au.LockDataForRead();
							quint32 empireId = gd.JoinRandomEmpire(0);
							if(empireId)
								gd.EmpireCapital(empireId, (*it).m_Dest);
							g_Au.UnlockData();

							if(g_Au.m_LogMessages & LOG_Admin)
							{
								QString logmsg = QString("HandoffAtoms: Changing limbo rescuer destination to capital city %1 of "
										"random empire %2")
									.arg((*it).m_Dest.GetID()).arg(empireId);
								g_Au.Log(logmsg);
							}
						}
						else
							(*it).m_Dest = (*it).m_ReturnOnFail;
					}
				}
				else
				{
					// 3 minutes should be enough time to wait to find an admin teleport destination.
					if((*it).m_TimeOuts >= 3)
					{
						if((*it).m_ReturnOnFail.IsNull() || (*it).m_ReturnOnFail == (*it).m_Dest)
						{
							// Destination atom hasn't been found. Change handoff destination to a random capital city.
							// Find a random empire that has a capital.
							const DgData& gd = g_Au.LockDataForRead();
							quint32 empireId = gd.JoinRandomEmpire(0);
							if(empireId)
								gd.EmpireCapital(empireId, (*it).m_Dest);
							g_Au.UnlockData();

							if(g_Au.m_LogMessages & LOG_Admin)
							{
								QString logmsg = QString("HandoffAtoms: Changing destination to capital city %1 of random "
										"empire %2")
									.arg((*it).m_Dest.GetID()).arg(empireId);
								g_Au.Log(logmsg);
							}
						}
						else
							(*it).m_Dest = (*it).m_ReturnOnFail;
					}
				}
			}

			repost.append(*it);

			// Remove the handoff request now. It will be reposted.
			it = m_Handoffs.erase(it);
		}
		else if((*it).IsReadyToHandoff())
		{
			// We found a handoff request that is ready.
			// Notify the sub thread who claims to have our destination atom.
			m_ServerChannel.LockServerList(__FILE__, __LINE__);
			DtServerChannel* server = m_ServerChannel.ServerChannel((*it).m_ServerId);
			if(server && !server->isFinished())
			{
				if((*it).m_PersonOrShip.IsNull())
				{
					if(g_Au.m_LogMessages & LOG_Admin)
						g_Au.Log(tr("HandoffAtoms: Limbo rescue atom %1 was found safe. Rescue canceled.")
								.arg((*it).m_Dest.GetID()));
				}
				else
					server->HandoffAtom((*it).m_PersonOrShip, (*it).m_Dest, (*it).m_RideAlso, (*it).m_LimboRescue, (*it).m_AtomData,
							(*it).m_ReturnOnFail);
			}
			m_ServerChannel.UnlockServerList(__FILE__, __LINE__);

			// Remove the handoff request now that we've dealt with it.
			it = m_Handoffs.erase(it);
		}
		else
			it++;
	}
	m_HandoffMutex.unlock();

	// Repost spawn requests that failed.
	if(g_Au.IsSceneMaster())
	{
		for(QList<AuHandoffRequest>::iterator rit = repost.begin(); rit != repost.end(); rit++)
			HandoffAtomDown2((*rit).m_PersonOrShip, (*rit).m_Dest, (*rit).m_RideAlso, (*rit).m_LimboRescue, (*rit).m_AtomData,
					(*rit).m_TimeOuts, (*rit).m_ReturnOnFail);
	}
	else
	{
		for(QList<AuHandoffRequest>::iterator rit = repost.begin(); rit != repost.end(); rit++)
			HandoffAtomUp2((*rit).m_PersonOrShip, (*rit).m_Dest, (*rit).m_RideAlso, (*rit).m_LimboRescue, (*rit).m_AtomData,
					(*rit).m_ReturnOnFail);
	}
}

// Spawn atoms that have accumulated information about desired spawn points.
// This is called periodically.
void AuSceneServerApp::SpawnAtoms()
{
	// Scene worker doesn't do anything with spawn requests.
	if(g_Au.IsSceneWorker())
		return;

	// Array of spawn requests that must be reposted due to timeout.
	QList<AuSpawnRequest> repost;

	m_SpawnMutex.lock();
	if((g_Au.m_LogMessages & LOG_Spawn) && !m_Spawns.isEmpty())
		qDebug("AuSceneServerApp::SpawnAtoms %d atoms waiting to spawn", m_Spawns.count());
	QList<AuSpawnRequest>::iterator it = m_Spawns.begin();
	while(it != m_Spawns.end())
	{
		if(g_Au.m_LogMessages & LOG_Spawn)
			qDebug() << "AuSceneServerApp::SpawnAtoms atom" << (*it).m_Person
				<< "empire" << (*it).m_Empire
				<< "capU" << (*it).m_CapitolU << (*it).m_CapitolUServerId
				<< "capG" << (*it).m_CapitolG << (*it).m_CapitolGServerId
				<< "city" << (*it).m_City << (*it).m_CityServerId
				<< "randCity" << (*it).m_RandomCity << (*it).m_RandomCityServerId
				<< "ship" << (*it).m_Ship << (*it).m_ShipServerId
				<< "arena" << (*it).m_Arena << (*it).m_ArenaServerId
				<< "gal" << GalaxyName((*it).Galaxy())
				<< "timeO" << (*it).IsTimedOut()
				<< "ready" << (*it).IsReadyToSpawn();

		if((*it).IsReadyToSpawn())
		{
			// We found a spawn request that is ready.
			// Inform the appropriate manager of the spawn.
			unsigned int spawnServerId = 0;
			AuId spawnAt;
			if((*it).m_ArenaServerId > 1 && !(*it).m_Arena.IsNull())
			{
				spawnAt = (*it).m_Arena;
				spawnServerId = (*it).m_ArenaServerId;
			}
			else if((*it).m_ShipServerId > 1 && !(*it).m_Ship.IsNull())
			{
				spawnAt = (*it).m_Ship;
				spawnServerId = (*it).m_ShipServerId;
			}
			else if((*it).m_CityServerId > 1 && !(*it).m_City.IsNull())
			{
				spawnAt = (*it).m_City;
				spawnServerId = (*it).m_CityServerId;
			}
			else if((*it).m_CapitolGServerId > 1 && !(*it).m_CapitolG.IsNull())
			{
				spawnAt = (*it).m_CapitolG;
				spawnServerId = (*it).m_CapitolGServerId;
			}
			else if((*it).m_CapitolUServerId > 1 && !(*it).m_CapitolU.IsNull())
			{
				spawnAt = (*it).m_CapitolU;
				spawnServerId = (*it).m_CapitolUServerId;
			}
			else if((*it).m_RandomCityServerId > 1 && !(*it).m_RandomCity.IsNull())
			{
				spawnAt = (*it).m_RandomCity;
				spawnServerId = (*it).m_RandomCityServerId;
			}
			else
			{
				// Trigger a special type of respawn that involves the creation of a nu homeworld for the spawning atom.
				SpawnPersonOnPlanet2((*it).m_Person, AuId(0), (*it).Galaxy());	// spawn mutex is already locked so use this version
			}

			if(spawnServerId)
			{
				// Notify the sub thread who claims to have our spawn point atom.
				m_ServerChannel.LockServerList(__FILE__, __LINE__);
				DtServerChannel* server = m_ServerChannel.ServerChannel(spawnServerId);
				if(server && !server->isFinished())
					server->SpawnAtom((*it).m_Person, (*it).m_Empire, (*it).m_City, (*it).m_Ship, spawnAt, (*it).m_Arena,
							(*it).Galaxy());
				m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
			}

			// Remove the spawn request now that we've dealt with it.
			it = m_Spawns.erase(it);
		}
		else if((*it).IsTimedOut())
		{
			// We get here if the spawn request timed out without finding all of our spawn points. This could happen if the
			// server assigned our capital city was still loading the database when the request arrived. It could also happen if
			// the assignment was revoked while we were searching. Because of that, scene managers send the spawn request back up
			// to the master to start the spawn point search all over again.
			if(g_Au.m_LogMessages & LOG_Spawn)
			{
				QString logmsg = tr("AuSceneServerApp::SpawnAtoms retrying timed-out atom %1 empire %2 city %3 %4 ship %5 %6 "
						"capitalU %7 %8 G %13 %14 randomCity %9 %10 arena %11 %12")
					.arg((*it).m_Person.GetID()).arg((*it).m_Empire)
					.arg((*it).m_City.GetID()).arg((*it).m_CityServerId)
					.arg((*it).m_Ship.GetID()).arg((*it).m_ShipServerId)
					.arg((*it).m_CapitolU.GetID()).arg((*it).m_CapitolUServerId)
					.arg((*it).m_RandomCity.GetID()).arg((*it).m_RandomCityServerId)
					.arg((*it).m_Arena.GetID()).arg((*it).m_ArenaServerId)
					.arg((*it).m_CapitolG.GetID()).arg((*it).m_CapitolGServerId);
				g_Au.Warning(logmsg);
			}

			// When reposting a spawn request, we disable specific spawn locations one-by-one until the person finally has none.
			// That causes them to eventually respawn on an empty world somewhere instead of being stuck in the spawn queue forever
			// waiting for a ship or city that does not exist.
			if(!(*it).m_Arena.IsNull())
				(*it).m_Arena = AuId(0);						// Disable the arena spawn location.
			else if(!(*it).m_Ship.IsNull())
				(*it).m_Ship = AuId(0);							// Disable the ship spawn location.
			else if(!(*it).m_City.IsNull())
				(*it).m_City = AuId(0);							// Disable the home city spawn location.
			else if(!(*it).m_CapitolG.IsNull())
				(*it).m_CapitolG = AuId(0);						// Disable the capital city spawn location.
			else if(!(*it).m_CapitolU.IsNull())
				(*it).m_CapitolU = AuId(0);						// Disable the capital city spawn location.
			else if(!(*it).m_RandomCity.IsNull())
				(*it).m_RandomCity = AuId(0);					// Disable the random city spawn location.
			repost.append(*it);

			// Remove the spawn request now. It will be reposted.
			it = m_Spawns.erase(it);
		}
		else
			it++;
	}
	m_SpawnMutex.unlock();

	// Repost spawn requests that failed.
	//if(!repost.isEmpty())
		//qDebug("AuSceneServerApp::SpawnAtoms reposting %d atom spawn requests", repost.count());
	for(QList<AuSpawnRequest>::iterator rit = repost.begin(); rit != repost.end(); rit++)
		SpawnAtomUp2((*rit).m_Person, (*rit).m_Empire, (*rit).m_City, (*rit).m_Ship, (*rit).m_Arena, (*rit).Galaxy());
}

void AuSceneServerApp::GovBossChanged(quint32 empireId, AuId governorId, AuId bossId)
{
	if(g_Au.IsSceneMaster())
	{
		// I am the master so I need to make the change in the empire data and notify my subordinate servers.
		DgData& gd = g_Au.LockDataForWrite();
		bool empireChanged = gd.EmpireChangeBoss(empireId, governorId, bossId);
		g_Au.UnlockData();

		if(empireChanged)
			emit ChangeEmpireInDb(empireId);	// Trigger db store which notifies the subordinates when done.
	}
	else if(m_tClientChannel)
	{
		// Notify my boss. The master will notify everyone. We'll pick up the notification on the way back down.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_GovBossChanged;
		out << empireId;
		out << governorId;
		out << bossId;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::OfficeAssigned(AuId person, quint32 empire, AuOffice office)
{
	if(g_Au.IsSceneMaster())
	{
		// I am the master so I need to make the change in the empire data and notify my subordinate servers.
		AssignOffice(person, empire, office, AuId(0), "", AuId(0), "");
	}
	else if(m_tClientChannel)
	{
		// Notify my boss. The master will notify everyone. We'll pick up the notification on the way back down.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_OfficeAssigned;
		out << person;
		out << empire;
		out << office;
		m_tClientChannel->SendPacket(ba);
	}
}

// 'officeChanged' is true if the type of office of the person has changed.
void AuSceneServerApp::AssignOffice(AuId person, quint32 empire, AuOffice office, AuId spawnCity, QString spawnCityName,
		AuId spawnSystem, QString spawnSystemName)
{
	// Make the change in my local atom data.
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaUniverse* universe = ether.FindUniverse();
	if(universe)
	{
		if(g_Au.IsSceneMaster())
		{
			// Master changes the empire and notifies the subs.
			if(empire)
			{
				// I am the master so I need to make the change in the empire data and notify my subordinate servers.
				DgData& gd = g_Au.LockDataForWrite();
				gd.EmpireAssignOffice(empire, person, office);
				g_Au.UnlockData();

				emit ChangeEmpireInDb(empire);	// Trigger db store which notifies the subordinates when done.
			}
		}
		else if(g_Au.IsSceneWorker())
		{
			// Try to find the person that will receive the office position.
			DaPerson* client = dynamic_cast<DaPerson*>(ether.FindNormalAtom(person));
			if(client)	// This is the person that holds this office now
			{
				if(client->EmpireId() != empire)
				{
					// Player's assignment is in a different empire than their current one. This player is changing empires.
					client->WillfullyChangedEmpire(empire);	// emits AddCitizenToEmpire message.
					if(!spawnCity.IsNull())
						client->SetSpawnCity(spawnCity, spawnCityName, AuGlobeLLF(0,0), spawnSystem, spawnSystemName);
					client->SetModified();
				}
			}
		}
	}
	g_Au.UnlockEtheriumDelete();

	// Notify my subordinates of the assignment.
	if(!g_Au.IsSceneWorker())
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_OfficeAssigned;
		out << person;
		out << empire;
		out << office;
		out << spawnCity;
		out << spawnCityName;
		out << spawnSystem;
		out << spawnSystemName;
		const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
		for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = servers.cbegin(); it != servers.cend(); it++)
			if((*it).second->isRunning())
				(*it).second->SendPacket(ba);
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
	}
}

void AuSceneServerApp::OfficeRevoked(AuId person, quint32 empire)
{
	if(g_Au.IsSceneMaster())
	{
		// Master changes the empire and notifies subordinates.
		if(empire)
		{
			DgData& gd = g_Au.LockDataForWrite();
			gd.EmpireRevokeOffice(empire, person);
			g_Au.UnlockData();

			emit ChangeEmpireInDb(empire);	// Trigger db store which notifies the subordinates when done.
		}
	}
	else if(m_tClientChannel)
	{
		// Notify my boss. The master will notify everyone. We'll pick up the notification on the way back down.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_OfficeRevoked;
		out << person;
		out << empire;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::JoinNewEmpire(const AuId& personId, const QString& personName, quint8 personGender, quint32 idOldEmpire,
		quint32 idNewEmpire)
{
	if(g_Au.IsSceneMaster())
	{
		// Move the person from their old empire to their nu empire.
		DgData& gd = g_Au.LockDataForWrite();
		gd.AddCitizenToEmpire(idOldEmpire, idNewEmpire, personId, personName, personGender);
		g_Au.UnlockData();

		// Trigger db store which notifies the subordinates when done.
		emit ChangeEmpireInDb(idNewEmpire);
		if(idOldEmpire)
			emit ChangeEmpireInDb(idOldEmpire);

		// AssignOffice will notify the master's subordinates of the change.
		AuOffice office;
		office.SetupEmperor();
		AssignOffice(personId, idNewEmpire, office, AuId(0), "", AuId(0), "");
	}
}

void AuSceneServerApp::LeaveEmpire(AuId personId, QString personName, quint32 idOldEmpire)
{
	if(g_Au.IsSceneMaster())
	{
		if(idOldEmpire)
		{
			DgData& gd = g_Au.LockDataForWrite();
			gd.RemoveCitizenFromEmpire(idOldEmpire, personId);
			g_Au.UnlockData();

			// Trigger db store which notifies the subordinates when done.
			emit ChangeEmpireInDb(idOldEmpire);
		}

		// Send down the office assigned packet with a bogus office. That will change the avatar.
		AuOffice office;
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_OfficeAssigned;
		out << personId;
		out << 0;
		out << office;
		out << AuId(0);
		out << "";
		out << AuId(0);
		out << "";
		const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
		for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = servers.cbegin(); it != servers.cend(); it++)
			if((*it).second->isRunning())
				(*it).second->SendPacket(ba);
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
	}
	else
	{
		if(m_tClientChannel)
		{
			// Queue a message for my boss server, it could be a manager or a master.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_LeaveEmpire;
			out << personId;
			out << personName;
			out << idOldEmpire;
			m_tClientChannel->SendPacket(ba);
		}
	}
}

void AuSceneServerApp::EstablishNewEmpire(AuId personId, const QString& personName, quint8 personGender, quint32 idOldEmpire,
		const QString& name, const QImage& imFlag)
{
	if(g_Au.IsSceneMaster())
	{
		// Insure the empire name is unique.
		if(!IsNameValid(name))
			return;						// The name is not valid.

		DgData& gd = g_Au.LockDataForWrite();
		unsigned int idEmpire = gd.EmpireId(name);
		if(idEmpire)
		{
			// An empire by that name already exists.
			g_Au.UnlockData();
			return;
		}

		// Create the nu empire.
		DgEmpire emp(name);
		emp.m_id = g_Au.NextEmpireId();	// Get a nu id.
		emp.m_flag = imFlag;
		//qDebug("AuSceneServerApp::EstablishNewEmpire, Master is Adding nu Empire=%d", emp.m_id);
		gd.m_gEmpires.insert(pair<quint32, DgEmpire>(emp.m_id, emp));
		g_Au.UnlockData();

		// Signal db thread to post updates.
		emit AddEmpireToDb(emp.m_id);
		
		// Add Citizen to nu empire
		//qDebug("AuSceneServerApp::EstablishNewEmpire, Master is calling JoinNewEmpire Player=%s:%d", 
					//(const char*)personName.toAscii(), personId.GetID());
		JoinNewEmpire(personId, personName, personGender, idOldEmpire, emp.m_id);
	}
	else
	{
		if(m_tClientChannel)
		{
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_EstNewEmpire;
			out << personId;
			out << personName;
			out << personGender;
			out << idOldEmpire;
			out << name;
			out << imFlag;
			m_tClientChannel->SendPacket(ba);
		}
	}
}

void AuSceneServerApp::GovSetCapitalUp(quint32 empire, AuId city, AuSpecies species, AuId domain, int capitalType)
{
	//qDebug("GovSetCapitalUp empire %d city %d", empire, city.GetID());
	if(!m_tClientChannel)
		return;
	// Sub servers send this message up to their boss.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtServerChannel::P_GovSetCapital;
	out << empire;
	out << city;
	out << species;
	out << domain;
	out << (quint8)capitalType;
	m_tClientChannel->SendPacket(ba);
}

void AuSceneServerApp::GovSetJoinPasswordUp(quint32 empire, QByteArray passCrypt)
{
	if(!m_tClientChannel)
		return;
	// Sub servers send this message up to their boss.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtServerChannel::P_GovSetJoinPass;
	out << empire;
	out << passCrypt;
	m_tClientChannel->SendPacket(ba);
}

void AuSceneServerApp::GovCitySurrenderedUp(AuId byWho, AuId cityId, quint32 fromEmpire, quint32 toEmpire)
{
	if(g_Au.IsSceneMaster())
	{
		// Master echoes this back down to the farm.
		GovCitySurrenderedDown(byWho, cityId, fromEmpire, toEmpire);
	}
	else if(m_tClientChannel)
	{
		// Sub servers send this message up to their boss.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_GovCitySurrendered;
		out << byWho;
		out << cityId;
		out << fromEmpire;
		out << toEmpire;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::GovCitySurrenderedDown(AuId byWho, AuId cityId, quint32 fromEmpire, quint32 toEmpire)
{
	if(g_Au.IsSceneWorker())
	{
		// Search for the city/cities in my local data.
		AuEther& ether = g_Au.LockEtheriumDelete();
		if(cityId == AuId(1))
		{
			// Surrender all cities to another empire. The surrendering empire could own a flag in any of them.
			DaAtomList cities;
			DaAtomList atoms;
			ether.ListAtoms(atoms);
			for(DaAtomList::const_iterator ci = atoms.cbegin(); ci != atoms.cend(); ci++)
			{
				DaBd* bd = dynamic_cast<DaBd*>(*ci);
				if(!bd)
					continue;	// wut?
				bd->Surrender(byWho, fromEmpire, toEmpire);
			}
		}
		else
		{
			// Surrender a single city, capitol or hq to another empire.
			DaBd* bd = dynamic_cast<DaBd*>(ether.FindNormalAtom(cityId));
			if(bd)
				bd->Surrender(byWho, fromEmpire, toEmpire);		// The city is here in my local data.
		}
		g_Au.UnlockEtheriumDelete();
	}
	else
	{
		// Echoes this down to the farm.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_GovCitySurrendered;
		out << byWho;
		out << cityId;
		out << fromEmpire;
		out << toEmpire;
		const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
		for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = servers.cbegin(); it != servers.cend(); it++)
			if((*it).second->isRunning())
				(*it).second->SendPacket(ba);
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
	}
}

void AuSceneServerApp::GovSetCommerceUp(quint32 empire,	QByteArray commerce)
{
	//qDebug("GovSetCommerceUp empire %d city %d", empire, city.GetID());
	if(!m_tClientChannel)
		return;
	// Sub servers send this message up to their boss.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtServerChannel::P_GovSetCommerce;
	out << empire;
	out << commerce;
	m_tClientChannel->SendPacket(ba);
}

void AuSceneServerApp::GovSetDefaultPoliticalStanceUp(quint32 empire,
		quint8 stance)
{
	if(!m_tClientChannel)
		return;
	// Sub servers send this message up to their boss.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtServerChannel::P_GovSetDefStance;
	out << empire;
	out << stance;
	m_tClientChannel->SendPacket(ba);
}

void AuSceneServerApp::GovSetContrailColorUp(quint32 empire, quint8 hue, quint8 sat)
{
	if(!m_tClientChannel)
		return;
	// Sub servers send this message up to their boss.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtServerChannel::P_GovSetContrailColor;
	out << empire;
	out << hue;
	out << sat;
	m_tClientChannel->SendPacket(ba);
}

void AuSceneServerApp::GovDossierChangeStateUp(quint32 empire, quint32 dosEmpire, AuTime docTime, quint8 docType, quint16 docIndex,
		quint8 docState)
{
	if(!m_tClientChannel)
		return;
	// Sub servers send this message up to their boss.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtServerChannel::P_GovDossierChangeState;
	out << empire;
	out << dosEmpire;
	out << docTime;
	out << docType;
	out << docIndex;
	out << docState;
	m_tClientChannel->SendPacket(ba);
}

void AuSceneServerApp::GovSetAnnouncementsUp(quint32 empire, QString announcements)
{
	if(!m_tClientChannel)
		return;
	// Sub servers send this message up to their boss.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtServerChannel::P_GovSetAnnouncements;
	out << empire;
	out << announcements;
	m_tClientChannel->SendPacket(ba);
}

void AuSceneServerApp::GovSetThemeColorUp(quint32 empire, quint8 theme, QColor color)
{
	if(!m_tClientChannel)
		return;
	// Sub servers send this message up to their boss.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtServerChannel::P_GovSetThemeColor;
	out << empire;
	out << theme;
	out << color;
	m_tClientChannel->SendPacket(ba);
}

void AuSceneServerApp::GovSetWarCriteria(quint32 empire, AuWarCriteria crit)
{
	if(g_Au.IsSceneMaster())
	{
		// Master changes the empire and notifies the subs.
		DgData& gd = g_Au.LockDataForWrite();
		gd.SetEmpireWarCriteria(empire, crit);
		g_Au.UnlockData();

		emit ChangeEmpireInDb(empire);							// Trigger db store which notifies the subordinates when done.
	}
	else if(m_tClientChannel)
	{
		// Sub servers send this message up to their boss.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_GovSetWarCriteria;
		out << empire;
		out << crit;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::GovSetTaxUp(quint32 empire, quint8 income, quint8 sales, quint8 subsidy)
{
	if(!m_tClientChannel)
		return;
	// Sub servers send this message up to their boss.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtServerChannel::P_GovSetTax;
	out << empire;
	out << income;
	out << sales;
	out << subsidy;
	m_tClientChannel->SendPacket(ba);
}

// ReleaseBerthUp gets the id of the nu ship to which the person is assigned,
// which could be 0 if they are simply releasing their berth.
// This helps insure the berth/roster data doesn't get out of sync.
// The logic causes all ships that are not the ship id to remove the person's
// id from their roster, if it's there. That way a person can't accidentally
// end up consuming berths on multiple different ships.
void AuSceneServerApp::ReleaseBerthUp(AuId personId, AuId shipId)
{
	if(m_tClientChannel)
	{
		// This must be a worker or manager; propagate the message up to
		// the master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_ReleaseBerth;
		personId >> out;	// The person who has released their berth.
		shipId >> out;		// The ship the person is now berthed to.
		m_tClientChannel->SendPacket(ba);
	}
	else
	{
		// Master server propagates the message by sending it down to all its
		// managers.
		ReleaseBerthDown(personId, shipId);
	}
}

// 'personId' is the person who has released their berth.
// 'shipId' is the ship that the person is berthing with instead of their old
// one. This could be 0.
void AuSceneServerApp::ReleaseBerthDown(const AuId personId, const AuId shipId)
{
	if(g_Au.IsSceneWorker())
	{
		// On the way down, the worker server processes the berth release.
		// This is the end of the road for the P_ReleaseBerth message.
		AuAtomInterface::ReleaseBerthDown(personId, shipId);
	}
	else
	{
		// Master and manager servers propagates the message by sending it to
		// all their subordinate servers.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_ReleaseBerth;
		personId >> out;	// The person who has released their berth.
		shipId >> out;		// The ship the person is now berthed to.
		const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
		for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = servers.cbegin(); it != servers.cend(); it++)
			if((*it).second->isRunning())
				(*it).second->SendPacket(ba);
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
	}
}

void AuSceneServerApp::EmpireChgFlagUp(quint32 empireId, QImage flag)
{
	if(g_Au.IsSceneMaster())
	{
		DgData& gd = g_Au.LockDataForWrite();
		gd.SetEmpireFlag(empireId, flag);
		g_Au.UnlockData();
		emit ChangeEmpireInDb(empireId);	// Propagate a change notification to the rest of the server farm.
	}
	else if(m_tClientChannel)
	{
		// Queue a message for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_EmpireChgFlag;
		out << empireId;
		out << flag;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::EmpireChgNameUp(quint32 empireId, QString name)
{
	if(g_Au.IsSceneMaster())
	{
		DgData& gd = g_Au.LockDataForWrite();
		QList<quint32> chgEmpireIds;
		gd.SetEmpireName(empireId, name, chgEmpireIds);
		g_Au.UnlockData();
		for(QList<quint32>::const_iterator cei = chgEmpireIds.cbegin(); cei != chgEmpireIds.cend(); cei++)
		{
			emit ChangeEmpireInDb(*cei);
		}
	}
	else if(m_tClientChannel)
	{
		// Queue a message for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_EmpireChgName;
		out << empireId;
		out << name;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::EmpireChgRoleUp(quint32 empireId, quint8 gameRole)
{
	if(g_Au.IsSceneMaster())
	{
		DgData& gd = g_Au.LockDataForWrite();
		gd.SetEmpireRole(empireId, (EMPIRE_ROLE)gameRole);
		g_Au.UnlockData();

		emit ChangeEmpireInDb(empireId);
	}
	else if(m_tClientChannel)
	{
		// Queue a message for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_EmpireChgRole;
		out << empireId;
		out << gameRole;
		m_tClientChannel->SendPacket(ba);
	}
}

// This is the beginning of the trail when an admin summons a unit.
// The unit is admin teleported locally if possible.
// Otherwise, the summon is broadcast to the entire server farm and the server managing the summoned unit admin teleports them
// to the destination adminId, with or without their vehicle.
void AuSceneServerApp::AdminSummon(AuId portUnitId, AuId adminId, quint8 rideAlso)
{
	if(portUnitId.IsNull())
		return;
	if(adminId.IsNull())
		return;

	if(m_tClientChannel)
	{
		if(g_Au.IsSceneWorker())
		{
			// Attempt to perform an AdminGoTo locally.
			AuEther& ether = g_Au.LockEtheriumDelete();
			bool didMove = AdminGoTo2(portUnitId, adminId, rideAlso, ether);
			g_Au.UnlockEtheriumDelete();
			if(didMove)
				return;	// We were able to perform the admin summon in our local data.
		}

		// The summon must be propagated up the server chain.
		// Queue a message for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_AdminSummon;
		out << portUnitId;
		out << adminId;
		out << rideAlso;
		m_tClientChannel->SendPacket(ba);
	}
	else
	{
		// This is a master server because it has no client channel.
		// The master and manager broadcast the summon to their subordinate servers.
		// The server that has the summoned unit will admin teleport them to the admin person.
		AdminSummonDown(portUnitId, adminId, rideAlso);
	}
}

void AuSceneServerApp::AdminSummonDown(AuId portUnitId, AuId adminId, quint8 rideAlso)
{
	if(g_Au.IsSceneWorker())
	{
		// If the summoned unit is found locally, tell them to AdminGoTo to the adminId person.
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaUnit* moveUnit = dynamic_cast<DaUnit*>(ether.FindNormalAtom(portUnitId));
		if(moveUnit)
		{
			// The summoned unit was found locally; admin teleport them.
			AuId parId(moveUnit->ParentId());
			g_Au.UnlockEtheriumDelete();
			AdminGoTo(portUnitId, adminId, rideAlso, false, parId);
		}
		else
			g_Au.UnlockEtheriumDelete();
	}
	else
	{
		// Broadcast the summon to my sub servers. Whoever has the moving unit will set them in motion as if
		// that unit admin teleported to the admin unit.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_AdminSummon;
		out << portUnitId;
		out << adminId;
		out << rideAlso;

		const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
		for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = servers.cbegin(); it != servers.cend(); it++)
			if((*it).second->isRunning())
				(*it).second->SendPacket(ba);
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
	}
}

// Initiates the sequence of an admin person moving to the location of another atom, with or without their vehicle.
// 'person' is the person atom who is moving
// 'dest' is the destination atom, this could be various kinds of atoms. It will be an admin person when summoning an atom.
// 'rideAlso' is 1 when the person wants their parent vehicle or mount taken with them. 2 allows spacecraft also.
void AuSceneServerApp::AdminGoTo(AuId person, AuId dest, quint8 rideAlso, bool limboRescue, AuId returnOnFail)
{
	//Q_ASSERT(g_Au.IsSceneWorker());
	if(person.IsNull())
		return;
	if(dest.IsNull())
		return;

	// Attempt to perform the AdminGoTo locally.
	//qDebug("AuSceneServerApp::AdminGoTo person %d dest %d rideAlso %d", person.GetID(), dest.GetID(), rideAlso);
	AuEther& ether = g_Au.LockEtheriumDelete();
	if(!AdminGoTo2(person, dest, rideAlso, ether))
		HandoffElsewhere2(person, dest, rideAlso, limboRescue, returnOnFail, ether);	// A handoff to another server.
	g_Au.UnlockEtheriumDelete();
}

bool AuSceneServerApp::AdminGoTo2(const AuId& portUnitId, const AuId& dest, quint8 rideAlso, AuEther& ether)
{
	//qDebug("AuSceneServerApp::AdminGoTo2 person %d dest %d rideAlso %d", person.GetID(), dest.GetID(), rideAlso);
	DaUnit* portUnit = dynamic_cast<DaUnit*>(ether.FindNormalAtom(portUnitId));
	if(!portUnit)
	{
		bool r = SummonGoTo2(portUnitId, dest, ether);	// Try summoning a dormant ship instead.
		return r;
	}

	// The person atom was found in my local data.
	if(rideAlso == 2 && portUnit->FindParentDesignBaseInEnv())
		rideAlso = 1;	// Don't allow the spacecraft in the design studio to be admin teleported.

	// Determine which atom is actually moving.
	// It could be the person or their mount/vehicle/spacecraft if 'rideAlso' is true.
	DaCreature* moveCreature = 0;
	DaVessel* moveShip = 0;
	DaVehicle* moveVehicle = 0;
	if(rideAlso)
	{
		moveCreature = dynamic_cast<DaCreature*>(portUnit->Parent());
		if(!moveCreature)
			moveVehicle = dynamic_cast<DaVehicle*>(portUnit->Parent());
		if(!moveVehicle)
			moveShip = portUnit->FindParentVesselInEnv();
	}
	if(!moveCreature && !moveVehicle && !moveShip)
	{
		// Did not get the parent of the porting unit. Try the porting unit itself.
		moveCreature = dynamic_cast<DaCreature*>(portUnit);
		if(!moveCreature)
			moveVehicle = dynamic_cast<DaVehicle*>(portUnit);
		if(!moveVehicle)
			moveShip = dynamic_cast<DaVessel*>(portUnit);

		// Insure ported dude is standing for the teleport.
		// It looks strange when they show up in town with their console still attached.
		if(moveCreature)
			moveCreature->Stand();
	}

	// We now know which atom is moving.
	// If the destination atom is in my local data, I can perform the move right here.
	DaAtom* destAtom = ether.FindNormalAtom(dest);
	if(destAtom)
	{
		// The destination atom is in my local data.
		DaSolarSystem* system = destAtom->FindParentSolarSystem();
		if(system && !system->IsOnlyStarFieldLoaded() && IsSolarSystemAssignedToMe(system->Id()))
		{
			// The destination atom's solar system is not a star field only system. Perform the move.
			DaAtom* movingAtom = moveCreature;
			if(!movingAtom)
				movingAtom = moveVehicle;
			if(!movingAtom)
				movingAtom = moveShip;
			if(movingAtom)
				system->AdminGoTo(movingAtom, destAtom);
			return true;	// We got this far so call it good. If we didn't port the atom, something is wrong.
		}
	}
	return false;
}

// This is the beginning of the trail when a crewman summons their dormant spacecraft.
// The ship is teleported locally if possible.
// Otherwise, the summon is broadcast to the entire server farm and the server managing the summoned dormant ship teleports it
// to the destination adminId.
// 'crewId' is AuId(1) when summoned by an agent.
void AuSceneServerApp::SummonDormantShip(AuId shipId, AuId crewId)
{
	if(shipId.IsNull())
		return;
	if(crewId.IsNull())
		return;

	if(m_tClientChannel)
	{
		if(g_Au.IsSceneWorker())
		{
			// Attempt to perform an AdminGoTo locally.
			AuEther& ether = g_Au.LockEtheriumDelete();
			bool didMove = SummonGoTo2(shipId, crewId, ether);
			g_Au.UnlockEtheriumDelete();
			if(didMove)
				return;	// We were able to perform the summon in our local data.
		}

		// The summon must be propagated up the server chain.
		// Queue a message for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_SummonDormantShip;
		out << shipId;
		out << crewId;
		m_tClientChannel->SendPacket(ba);
	}
	else
	{
		// This is a master server because it has no client channel.
		// The master and manager broadcast the summon to their subordinate servers.
		// The server that has the summoned ship will teleport them to the crew person.
		SummonDormantShipDown(shipId, crewId);
	}
}

void AuSceneServerApp::SummonDormantShipDown(AuId shipId, AuId crewId)
{
	if(g_Au.IsSceneWorker())
	{
		// If the summoned ship is found locally, tell them to teleport to the crewId person.
		AuEther& ether = g_Au.LockEtheriumDelete();
		bool moveShip = dynamic_cast<DaVessel*>(ether.FindDormantAtom(shipId));
		g_Au.UnlockEtheriumDelete();
		if(moveShip)
			SummonGoTo(shipId, crewId);	// The summoned ship was found locally; admin teleport them.
	}
	else
	{
		// Broadcast the summon to my sub servers. Whoever has the moving person will set them in motion as if
		// that person admin teleported to the admin person.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_SummonDormantShip;
		out << shipId;
		out << crewId;

		const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
		for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = servers.cbegin(); it != servers.cend(); it++)
			if((*it).second->isRunning())
				(*it).second->SendPacket(ba);
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
	}
}

// Initiates the sequence of a dormant privately owned spacecraft moving to the location of a crew member who summoned it.
// 'crewId' is AuId(1) when summoned by an agent.
void AuSceneServerApp::SummonGoTo(AuId shipId, AuId crewId)
{
	//Q_ASSERT(g_Au.IsSceneWorker());
	if(shipId.IsNull())
		return;
	if(crewId.IsNull())
		return;

	// Attempt to perform the AdminGoTo locally.
	AuEther& ether = g_Au.LockEtheriumDelete();
	if(!SummonGoTo2(shipId, crewId, ether))
		HandoffElsewhere2(shipId, crewId, false, false, AuId(0), ether);		// A handoff is required to another server.
	g_Au.UnlockEtheriumDelete();
}

// 'crewId' is AuId(1) when summoned by an agent.
bool AuSceneServerApp::SummonGoTo2(const AuId& shipId, const AuId& crewId, AuEther& ether)
{
	DaVessel* moveShip = dynamic_cast<DaVessel*>(ether.FindDormantAtom(shipId));
	if(!moveShip)
		return false;

	// If the destination atom is in my local data, I can perform the move right here.
	DaAtom* destAtom = ether.FindNormalAtom(crewId);
	if(destAtom)
	{
		// The destination atom is in my local data.
		DaSolarSystem* system = destAtom->FindParentSolarSystem();
		if(system && !system->IsOnlyStarFieldLoaded() && IsSolarSystemAssignedToMe(system->Id()))
		{
			// The destination atom's solar system is not a star field only system. Perform the move.
			system->AdminGoTo(moveShip, destAtom);
			moveShip->Reactivate();
			return true;
		}
	}
	return false;
}

void AuSceneServerApp::SendFundsUp(AuId fromId, QString fromName, AuId toId, double amount, quint8 fromGalaxy, quint8 reason,
		quint32 designId)
{
	if(g_Au.IsSceneWorker())
	{
		// The receipient atom might be on this worker server.
		if(SendFunds(fromId, fromName, toId, amount, (GALAXY)fromGalaxy, reason, designId))
		{
			// The recipient atom is here so there is no need to propagate this up to the master.
			return;
		}
	}

	// We get here if the message needs to be propagated to the entire server farm.
	if(g_Au.IsSceneMaster())
	{
		SendFundsDown(fromId, fromName, toId, amount, fromGalaxy, reason, designId);
	}
	else if(m_tClientChannel)
	{
		// Queue a message for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_SendFunds;
		out << fromId;
		out << fromName;
		out << toId;
		out << amount;
		out << fromGalaxy;
		out << reason;
		out << designId;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::SendFundsDown(AuId fromId, QString fromName, AuId toId, double amount, quint8 fromGalaxy, quint8 reason,
		quint32 designId)
{
	if(g_Au.IsSceneWorker())
	{
		// All workers attempt to complete the transfer. Only one will succeed, if any.
		SendFunds(fromId, fromName, toId, amount, (GALAXY)fromGalaxy, reason, designId);
	}
	else
	{
		// Master and manager servers propagate the message by sending it to all their subordinate servers.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_SendFunds;
		out << fromId;
		out << fromName;
		out << toId;
		out << amount;
		out << fromGalaxy;
		out << reason;
		out << designId;
		const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
		for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = servers.cbegin(); it != servers.cend(); it++)
			if((*it).second->isRunning())
				(*it).second->SendPacket(ba);
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
	}
}

// 'reason' is 0=avatar transfer of funds, 1=funds return to avatar, 2=funds to seller of design sold at exchange
// Returns true if the recipient atom is found in the local data.
bool AuSceneServerApp::SendFunds(const AuId& fromId, const QString& fromName, const AuId& toId, double amount, GALAXY fromGalaxy,
		quint8 reason, quint32 designId)
{
	// The receipient atom might be on this worker.
	bool transferComplete = false;
	double oldBalance = 0;
	double nuBalance = 0;
	bool sendReceipt = false;
	bool returnIt = false;

	AuEther& ether = g_Au.LockEtheriumDelete();
	DaAtom* receiver = ether.FindNormalAtom(toId);
	if(receiver)
	{
		if(reason == 0)
		{
			// Avatar transfer of funds to someone or something.
			// Agent sends GALAXY_Count so money can be sent anywhere.
			if(fromGalaxy == GALAXY_Count || receiver->Galaxy() == fromGalaxy)
			{
				// What kind of atom has received the funds?
				DaPerson* person = dynamic_cast<DaPerson*>(receiver);
				DaVessel* ship = dynamic_cast<DaVessel*>(receiver);
				if(person)
				{
					// Add the money to the receiver's account.
					oldBalance = person->AccountBalance();
					person->CreditAccount(amount);
					nuBalance = person->AccountBalance();

					// A person receives a receipt of the transfer of funds to them.
					sendReceipt = true;
				}
				else if(ship)
				{
					// No receipt is sent to the ship.
					ship->CreditAccount(amount);
				}
			}
			else
			{
				// Cannot accept the money. Return it.
				returnIt = true;
			}
		}
		else if(reason == 1)
		{
			// Funds returned to sender.
			DaPerson* person = dynamic_cast<DaPerson*>(receiver);
			if(person)
			{
				// Add the money to the receiver's account.
				oldBalance = person->AccountBalance();
				person->CreditAccount(amount);
				nuBalance = person->AccountBalance();

				// A person receives a receipt of the return of funds back to them.
				sendReceipt = true;
			}
		}
		else if(reason == 2)
		{
			// Funds from sale of design at the exchange.
			DaPerson* person = dynamic_cast<DaPerson*>(receiver);
			if(person)
			{
				// Add the money to the receiver's account. They are the seller of the design.
				oldBalance = person->AccountBalance();
				person->CreditAccount(amount);
				nuBalance = person->AccountBalance();

				// Person receives a receipt of the sale.
				sendReceipt = true;
			}
		}
		else if(reason == 3)
		{
			// Funds from refund of canceled company ship in production.
			DaPerson* person = dynamic_cast<DaPerson*>(receiver);
			if(person)
			{
				// Add the money to the receiver's account. They are the seller of the design.
				oldBalance = person->AccountBalance();
				person->CreditAccount(amount);
				nuBalance = person->AccountBalance();

				// Person receives a receipt of the sale.
				sendReceipt = true;
			}
		}
		transferComplete = true;	// Found the recipient atom, even if they don't get the money.
	}
	g_Au.UnlockEtheriumDelete();

	if(sendReceipt)
	{
		// Send a receipt to the person who received the money.
		QString sAmount = QString("%L1%2").arg(amount,0,'f',0).arg(g_Au.CurrencySymbol());
		QString sOldBalance = QString("%L1%2").arg(oldBalance,0,'f',0).arg(g_Au.CurrencySymbol());
		QString sNewBalance = QString("%L1%2").arg(nuBalance,0,'f',0).arg(g_Au.CurrencySymbol());
		if(reason == 0)
		{
			// The money was delivered to the designated recipient.
			QString sReceipt = tr("<p>Funds transferred.</p>"
					"Old account balance %1<br>%2 received from %3<br>New account balance %4.")
				.arg(sOldBalance).arg(sAmount).arg(fromName).arg(sNewBalance);
			SendMail(AuMailMsg::MSG_FundsTransfer, AuId::BankMail(), tr("Bank"), toId, tr("Funds Transfer Receipt"),
					sReceipt);
		}
		else if(reason == 1)
		{
			// The money was returned to the sender.
			QString sReceipt = tr("<p>Funds could not be transferred.</p>"
					"Old account balance %1<br>%2 returned because recipient was in another galaxy.<br>"
					"New account balance %3.")
				.arg(sOldBalance).arg(sAmount).arg(sNewBalance);
			SendMail(AuMailMsg::MSG_FundsTransfer, AuId::BankMail(), tr("Bank"), toId, tr("Funds Transfer Return"),
					sReceipt);
		}
		else if(reason == 2)
		{
			QString sReceipt = tr("<p>Blueprint %1 was sold to %2 <i>%3</i> at the exchange.</p>"
					"Old account balance %4<br>Sale price %5<br>New account balance %6.")
				.arg(designId)
				.arg(fromName)
				.arg(fromId.StringId())
				.arg(sOldBalance)
				.arg(sAmount)
				.arg(sNewBalance);
			SendMail(AuMailMsg::MSG_ArchitectCommission, AuId::BankMail(), tr("Bank"), toId,
					tr("Design %1 Sold at Exchange").arg(designId), sReceipt);
		}
		else if(reason == 3)
		{
			// We do not get a design id with this.
			QString sReceipt = tr("<p>Your production order for a company spacecraft was canceled. "
					"Your money was refunded. "
					"</p>"
					"Old account balance %4<br>Refund %5<br>New account balance %6.")
				.arg(sOldBalance)
				.arg(sAmount)
				.arg(sNewBalance);
			SendMail(AuMailMsg::MSG_ArchitectCommission, AuId::BankMail(), tr("Bank"), toId,
					tr("Refund, Spacecraft Order Canceled"), sReceipt);
		}
	}
	else if(returnIt)
	{
		// Send money back to the person who sent it. '*' affects receipt sent a few lines above here.
		// GALAXY_Count insures the refund won't be rejected for the same reason, in case they change galaxies in between.
		SendFundsUp(toId, "*", fromId, amount, GALAXY_Count, 1, 0);
	}

	return transferComplete;
}

void AuSceneServerApp::SendMail(AuMailMsg::MSGTYPE msgType, const AuId& fromId, const QString& fromName, const AuId& toId,
		const QString& subject, const QString& bodyText) const
{
	// Get this server's login credentials to send mail.
	const DgData& gd = g_Au.LockDataForRead();
	unsigned int acctId = gd.PlayerId();	// The local server must be "logged in" as someone to send mail.
	unsigned int confirmNum = gd.ConfirmationNumber();
	g_Au.UnlockData();

	// Send the mail message.
	g_Au.SendMail(acctId, confirmNum, fromId, fromName, toId, msgType, subject, bodyText, AuId(0), "", AuId(0), "",
			GALAXY_Count, AuPoint(0,0,0));
}

int AuSceneServerApp::ManagerCount() const
{
	if(g_Au.IsSceneMaster())
	{
		// Return count of master's server channel connections.
		const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
		int count = servers.size();
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
		return count;
	}
	else
		return 0;	// Only scene master has managers.
}

int AuSceneServerApp::PlayerCount() const
{
	if(g_Au.IsSceneMaster() || g_Au.IsSceneManager())
		return m_ServerChannel.PlayerCount();		// Return count of players tallied from status updates.
	else
	{
		// Return count of worker's client connections.
		const unordered_map<unsigned int, DtSceneServer*>& servers = m_TcpServer.LockServerList(__FILE__, __LINE__);
		int count = servers.size();
		m_TcpServer.UnlockServerList(__FILE__, __LINE__);
		return count;
	}
}

void AuSceneServerApp::PlayersOnlineStatus(AuPlayerOnlineStatusVector& players) const
{
	if(g_Au.IsSceneMaster() || g_Au.IsSceneManager())
		return m_ServerChannel.PlayersOnlineStatus(players);	// Return status of players accumulated from status updates.
	else
		return m_TcpServer.PlayersOnlineStatus(players);		// Return status of worker's player connections.
}

int AuSceneServerApp::WorkerCoreCount() const
{
	return m_ServerChannel.WorkerCoreCount();		// Return count of worker CPU cores tallied from status updates.
}

int AuSceneServerApp::WorkerCount() const
{
	return m_ServerChannel.WorkerCount();		// Return count of workers tallied from status updates.
}

// Called on the scene worker when an officer has been killed.
// Propagates a notification to the entire server farm so the officer's homeworld can be updated.
void AuSceneServerApp::OfficerKilledUp(const AuId homePlanetId)
{
	if(g_Au.IsSceneMaster())
	{
		// Scene master needs to propagate a notification down to the entire farm.
		OfficerKilledDown(homePlanetId);
	}
	else if(m_tClientChannel)
	{
		// Scene Manager or Scene Worker
		// Scene worker will attempt to find the home planet in its local data before propagating a message up to the manager.
		if(g_Au.IsSceneManager() || !NotifyOfficerKilled(homePlanetId))
		{
			// Queue a message for my boss server, it could be a manager or a master.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_OfficerKilled;
			homePlanetId >> out;
			m_tClientChannel->SendPacket(ba);
		}
	}
}

void AuSceneServerApp::OfficerKilledDown(const AuId homePlanetId)
{
	if(g_Au.IsSceneWorker())
	{
		// I am a worker server at the bottom of the pile.
		// Notify the home planet if it is in my local data.
		NotifyOfficerKilled(homePlanetId);
	}
	else
	{
		// Master and manager servers propagate the message by sending it to all their subordinate servers.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_OfficerKilled;
		homePlanetId >> out;
		const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
		for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = servers.cbegin(); it != servers.cend(); it++)
			if((*it).second->isRunning())
				(*it).second->SendPacket(ba);
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
	}
}

// Returns true if the 'homePlanetId' planet is found in my local data.
bool AuSceneServerApp::NotifyOfficerKilled(const AuId& homePlanetId)
{
	// Check to see if I have the planet in question.
	bool planetNotified = false;
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaWorld* homePlanet = dynamic_cast<DaWorld*>(ether.FindNormalAtom(homePlanetId));
	if(homePlanet)
	{
		// The home planet was found in my local data.
		DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(homePlanet);
		if(llworld)
			llworld->SetCapt(AuId(0), QString(""));
		homePlanet->SetModified();
		planetNotified = true;
	}
	g_Au.UnlockEtheriumDelete();
	return planetNotified;
}

// This code only handles BoardCryo requests accepted by people, which generates a CryoBoarded message.
// The person who sends the CryoBoarded message is willing to unload passengers to the person identified in the message text.
// Cities load passengers in DaBd::RxChatMessage when the BoardCryo message arrives.
void AuSceneServerApp::CommAcceptCryoBoarded(DaPerson* unloadPerson, const AuCommCryoBoarded& msg)
{
	// Get the number of passengers to transfer from the message.
	unsigned int count = msg.Count();
	if(!count)
		return;	// Invalid data.

	// Locate the unloader person.
	DaAtom* system = unloadPerson->FindEnvironmentAtom();
	if(!system)
		return;	// Unloader is not in a solar system? Problem!

	// The unloader must be aboard a ship.
	DaVessel* unloadShip = 0;
	if(unloadPerson->IsInSpacecraft())
		unloadShip = unloadPerson->FindParentVesselInEnv();
	if(!unloadShip)
		return;	// Unloader is not on a ship.
	if(!unloadShip->InChainOfCommand(unloadPerson))
		return;	// Unloader is not an officer of the ship.

	// Insure unloading ship actually has the passengers.
	unsigned int passengers = unloadShip->CryoBerthsOccupied();
	if(passengers < count)
	{
		QString text(tr("Your ship does not have %1 cryo travellers aboard.").arg(count));
		AuCommCityChat mymsg(text, msg.ChannelName(), unloadPerson->Id(), unloadPerson->Name(), msg.FromSectorId(),
				msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;
	}

	// Validate the transaction based on the loader.
	AuId loaderId = msg.LoaderId();	// person loading the passengers
	DaAtom* loader = (DaAtom*)system->FindIdRecursive(loaderId);
	if(!loader)
	{
		AuCommCityMessage mymsg(AuCommCityMessage::CryoSameSystem, msg.ChannelName(), unloadPerson->Id(),
				unloadPerson->Name(), msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;	// Loader not found in the same solar system; moved away?
	}
	DaPerson* loadPerson = dynamic_cast<DaPerson*>(loader);
	if(!loadPerson)
		return;	// Loader must be a person.

	// Must locate the exact bid in the list of active bids.
	// The count may not match because the accepter may offer less
	// passengers that the loader has offered as open.
	if(!g_Au.HasPendingCryo(&msg))
	{
		AuCommCityMessage mymsg(AuCommCityMessage::CryoBoardingNotActive, msg.ChannelName(), loadPerson->Id(),
				loadPerson->Name(), msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;	// Ignore; load request is no longer active.
	}

	// Insure physical locations permit acceptance of the offer.
	// - Loader in a space ship, and
	// - Loader ship near seller's space ship.
	// - Loader ship not the same as seller ship.
	DaVessel* loadShip = 0;
	if(loadPerson->IsInSpacecraft())
		loadShip = loadPerson->FindParentVesselInEnv();
	if(!loadShip)
		return;	// Loader is not on a ship.
	if(loadShip->Id() == unloadShip->Id())
		return;	// They are in the same ship.
	if(!loadShip->InChainOfCommand(loadPerson))
		return;	// Loader is not an officer of the ship.

	// Loader and unloader are aboard different ships.
	AuPoint unloadLoc = unloadShip->EnvLocation();
	AuPoint loadLoc = loadShip->EnvLocation();
	float tradeRange = unloadShip->TransferRange(loadShip);
	float distApart = unloadLoc.Distance(loadLoc);
	if(distApart > tradeRange)
	{
		AuCommCityMessage mymsg(AuCommCityMessage::TooFarToTransactCryo, msg.ChannelName(), unloadPerson->Id(),
				unloadPerson->Name(), msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;
	}

	// Insure loading ship has enough room.
	unsigned int openBerths = loadShip->CryoBerthsAvailable();
	if(openBerths < count)
	{
		QString text(tr("Your ship does not have %1 empty cryo berths.").arg(count));
		AuCommCityChat mymsg(text, msg.ChannelName(), loadPerson->Id(), loadPerson->Name(), msg.FromSectorId(), msg.FromGalaxy(),
				msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;
	}

	// Transfer the passengers.
	loadShip->TransferCryo(unloadShip, count);

	// Revoke the bid so it cannot be accepted again.
	g_Au.BidRevoked(loaderId, msg.ChannelName());
}

// This code only handles DebarkCryo notices accepted by
// people, which generates a CryoDebarked message. The person
// who sends the CryoDebarked message is willing to load
// passengers from the person id identified in the message text.
// Cities accept passengers in DaBd::RxChatMessage when the
// DebarkCryo message arrives.
void AuSceneServerApp::CommAcceptCryoDebarked(DaPerson* loadPerson, const AuCommCryoDebarked& msg)
{
	// Get the number of passengers to transfer from the message.
	unsigned int count = msg.Count();
	if(!count)
		return;	// Invalid data.

	// Locate the accepter person.
	const DaAtom* system = loadPerson->FindEnvironmentAtom();
	if(!system)
		return;	// accepter is not in a solar system? Problem!

	// The accepter must be aboard a ship.
	DaVessel* loadShip = 0;
	if(loadPerson->IsInSpacecraft())
		loadShip = loadPerson->FindParentVesselInEnv();
	if(!loadShip)
		return;	// accepter is not on a ship.
	if(!loadShip->InChainOfCommand(loadPerson))
		return;	// accepter is not an officer of the ship.

	// Insure accepting ship actually has the berths.
	unsigned int berths = loadShip->CryoBerthsAvailable();
	if(berths < count)
	{
		QString text(tr("Your ship does not have %1 empty cryo berths.").arg(count));
		AuCommCityChat mymsg(text, msg.ChannelName(), loadPerson->Id(), loadPerson->Name(), msg.FromSectorId(), msg.FromGalaxy(),
				msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;
	}

	// Validate the transaction based on the unloader.
	AuId unloaderId = msg.UnloaderId();	// person unloading them
	DaAtom* unloader = (DaAtom*)system->FindIdRecursive(unloaderId);
	if(!unloader)
	{
		AuCommCityMessage mymsg(AuCommCityMessage::CryoSameSystem, msg.ChannelName(), loadPerson->Id(), loadPerson->Name(),
				msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;	// unloader not found in the same solar system;
				// moved away?
	}
	DaPerson* unloadPerson = dynamic_cast<DaPerson*>(unloader);
	if(!unloadPerson)
		return;	// unloader must be a person.

	// Must locate the exact bid in the list of active bids.
	// The count may not match because the accepter may take less
	// passengers that the unloader has offered.
	if(!g_Au.HasPendingCryo(&msg))
	{
		AuCommCityMessage mymsg(AuCommCityMessage::CryoDebarkingNotActive, msg.ChannelName(), unloader->Id(), unloader->Name(),
				msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;	// Ignore; unload notice is no longer active.
	}

	// Insure physical locations permit acceptance of the offer.
	// - unloader in a space ship, and
	// - unloader ship near seller's space ship.
	// - unloader ship not the same as seller ship.
	DaVessel* unloadShip = 0;
	if(unloadPerson->IsInSpacecraft())
		unloadShip = unloadPerson->FindParentVesselInEnv();
	if(!unloadShip)
		return;	// unloader is not on a ship.
	if(unloadShip->Id() == loadShip->Id())
		return;	// They are in the same ship.
	if(!unloadShip->InChainOfCommand(unloadPerson))
		return;	// unloader is not an officer of the ship.

	// unloader and accepter are aboard different ships.
	AuPoint acceptLoc = loadShip->EnvLocation();
	AuPoint unloadLoc = unloadShip->EnvLocation();
	float tradeRange = unloadShip->TransferRange(loadShip);
	float distApart = unloadLoc.Distance(acceptLoc);
	if(distApart > tradeRange)
	{
		AuCommCityMessage mymsg(AuCommCityMessage::TooFarToTransactPassengers, msg.ChannelName(), loadPerson->Id(),
				loadPerson->Name(), msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;
	}

	// Insure unloading ship has the passengers.
	unsigned int passengers = unloadShip->CryoBerthsOccupied();
	if(passengers < count)
	{
		QString text(tr("Your ship does not have %1 cryo travellers aboard.").arg(count));
		AuCommCityChat mymsg(text, msg.ChannelName(), unloader->Id(), unloader->Name(), msg.FromSectorId(), msg.FromGalaxy(),
				msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;
	}

	// Transfer the passengers.
	loadShip->TransferCryo(unloadShip, count);

	// Revoke the bid so it cannot be accepted again.
	g_Au.BidRevoked(unloaderId, msg.ChannelName());
}

// This code only handles BoardPassengers requests accepted by people, which generates a PassengersBoarded message.
// The person who sends the PassengersBoarded message is willing to unload passengers to the person identified in the message text.
// Cities load passengers in DaBd::RxChatMessage when the BoardPassengers message arrives.
void AuSceneServerApp::CommAcceptColonistsBoarded(DaPerson* unloadPerson, const AuCommPassengersBoarded& msg)
{
	// Get the number of passengers to transfer from the message.
	unsigned int count = msg.Count();
	if(!count)
		return;	// Invalid data.

	// Locate the unloader person.
	DaAtom* system = unloadPerson->FindEnvironmentAtom();
	if(!system)
		return;	// Unloader is not in a solar system? Problem!

	// The unloader must be aboard a ship.
	DaVessel* unloadShip = 0;
	if(unloadPerson->IsInSpacecraft())
		unloadShip = unloadPerson->FindParentVesselInEnv();
	if(!unloadShip)
		return;	// Unloader is not on a ship.
	if(!unloadShip->InChainOfCommand(unloadPerson))
		return;	// Unloader is not an officer of the ship.

	// Insure unloading ship actually has the passengers.
	unsigned int passengers = unloadShip->ColonistsAboard();
	if(passengers < count)
	{
		QString text(tr("Your ship does not have %1 passengers aboard.").arg(count));
		AuCommCityChat mymsg(text, msg.ChannelName(), unloadPerson->Id(), unloadPerson->Name(), msg.FromSectorId(),
				msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;
	}

	// Validate the transaction based on the loader.
	AuId loaderId = msg.LoaderId();	// person loading the passengers
	DaAtom* loader = (DaAtom*)system->FindIdRecursive(loaderId);
	if(!loader)
	{
		AuCommCityMessage mymsg(AuCommCityMessage::PassengersSameSystem, msg.ChannelName(), unloadPerson->Id(),
				unloadPerson->Name(), msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;	// Loader not found in the same solar system; moved away?
	}
	DaPerson* loadPerson = dynamic_cast<DaPerson*>(loader);
	if(!loadPerson)
		return;	// Loader must be a person.

	// Must locate the exact bid in the list of active bids.
	// The count may not match because the accepter may offer less
	// passengers that the loader has offered as open.
	if(!g_Au.HasPendingColonists(&msg))
	{
		AuCommCityMessage mymsg(AuCommCityMessage::PassengerBoardingNotActive, msg.ChannelName(), loadPerson->Id(),
				loadPerson->Name(), msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;	// Ignore; load request is no longer active.
	}

	// Insure physical locations permit acceptance of the offer.
	// - Loader in a space ship, and
	// - Loader ship near seller's space ship.
	// - Loader ship not the same as seller ship.
	DaVessel* loadShip = 0;
	if(loadPerson->IsInSpacecraft())
		loadShip = loadPerson->FindParentVesselInEnv();
	if(!loadShip)
		return;	// Loader is not on a ship.
	if(loadShip->Id() == unloadShip->Id())
		return;	// They are in the same ship.
	if(!loadShip->InChainOfCommand(loadPerson))
		return;	// Loader is not an officer of the ship.

	// Loader and unloader are aboard different ships.
	AuPoint unloadLoc = unloadShip->EnvLocation();
	AuPoint loadLoc = loadShip->EnvLocation();
	float tradeRange = unloadShip->TransferRange(loadShip);
	float distApart = unloadLoc.Distance(loadLoc);
	if(distApart > tradeRange)
	{
		AuCommCityMessage mymsg(AuCommCityMessage::TooFarToTransactPassengers, msg.ChannelName(), unloadPerson->Id(),
				unloadPerson->Name(), msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;
	}

	// Insure loading ship has enough room.
	unsigned int openBerths = loadShip->PassengerBerthsAvailable();
	if(openBerths < count)
	{
		QString text(tr("Your ship does not have %1 empty stateroom berths.").arg(count));
		AuCommCityChat mymsg(text, msg.ChannelName(), loadPerson->Id(), loadPerson->Name(), msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(),
				msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;
	}

	// Transfer the passengers.
	loadShip->TransferColonists(unloadShip, count);

	// Revoke the bid so it cannot be accepted again.
	g_Au.BidRevoked(loaderId, msg.ChannelName());
}

// This code only handles DebarkColonists notices accepted by
// people, which generates a ColonistsDebarked message. The person
// who sends the ColonistsDebarked message is willing to load
// passengers from the person id identified in the message text.
// Cities accept passengers in DaBd::RxChatMessage when the
// DebarkColonists message arrives.
void AuSceneServerApp::CommAcceptColonistsDebarked(DaPerson* loadPerson, const AuCommPassengersDebarked& msg)
{
	// Get the number of passengers to transfer from the message.
	unsigned int count = msg.Count();
	if(!count)
		return;	// Invalid data.

	// Locate the accepter person.
	const DaAtom* system = loadPerson->FindEnvironmentAtom();
	if(!system)
		return;	// accepter is not in a solar system? Problem!

	// The accepter must be aboard a ship.
	DaVessel* loadShip = 0;
	if(loadPerson->IsInSpacecraft())
		loadShip = loadPerson->FindParentVesselInEnv();
	if(!loadShip)
		return;	// accepter is not on a ship.
	if(!loadShip->InChainOfCommand(loadPerson))
		return;	// accepter is not an officer of the ship.

	// Insure accepting ship actually has the berths.
	unsigned int berths = loadShip->PassengerBerthsAvailable();
	if(berths < count)
	{
		QString text(tr("Your ship does not have %1 empty stateroom berths.").arg(count));
		AuCommCityChat mymsg(text, msg.ChannelName(), loadPerson->Id(), loadPerson->Name(), msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(),
				msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;
	}

	// Validate the transaction based on the unloader.
	AuId unloaderId = msg.UnloaderId();	// person unloading them
	DaAtom* unloader = (DaAtom*)system->FindIdRecursive(unloaderId);
	if(!unloader)
	{
		AuCommCityMessage mymsg(AuCommCityMessage::PassengersSameSystem, msg.ChannelName(), loadPerson->Id(), loadPerson->Name(),
				msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;	// unloader not found in the same solar system;
				// moved away?
	}
	DaPerson* unloadPerson = dynamic_cast<DaPerson*>(unloader);
	if(!unloadPerson)
		return;	// unloader must be a person.

	// Must locate the exact bid in the list of active bids.
	// The count may not match because the accepter may take less
	// passengers that the unloader has offered.
	if(!g_Au.HasPendingColonists(&msg))
	{
		AuCommCityMessage mymsg(AuCommCityMessage::PassengerDebarkingNotActive, msg.ChannelName(), unloader->Id(), unloader->Name(),
				msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;	// Ignore; unload notice is no longer active.
	}

	// Insure physical locations permit acceptance of the offer.
	// - unloader in a space ship, and
	// - unloader ship near seller's space ship.
	// - unloader ship not the same as seller ship.
	DaVessel* unloadShip = 0;
	if(unloadPerson->IsInSpacecraft())
		unloadShip = unloadPerson->FindParentVesselInEnv();
	if(!unloadShip)
		return;	// unloader is not on a ship.
	if(unloadShip->Id() == loadShip->Id())
		return;	// They are in the same ship.
	if(!unloadShip->InChainOfCommand(unloadPerson))
		return;	// unloader is not an officer of the ship.

	// unloader and accepter are aboard different ships.
	AuPoint acceptLoc = loadShip->EnvLocation();
	AuPoint unloadLoc = unloadShip->EnvLocation();
	float tradeRange = unloadShip->TransferRange(loadShip);
	float distApart = unloadLoc.Distance(acceptLoc);
	if(distApart > tradeRange)
	{
		AuCommCityMessage mymsg(AuCommCityMessage::TooFarToTransactPassengers, msg.ChannelName(), loadPerson->Id(),
				loadPerson->Name(), msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;
	}

	// Insure unloading ship has the passengers.
	unsigned int passengers = unloadShip->ColonistsAboard();
	if(passengers < count)
	{
		QString text(tr("Your ship does not have %1 passengers aboard.").arg(count));
		AuCommCityChat mymsg(text, msg.ChannelName(), unloader->Id(), unloader->Name(), msg.FromSectorId(), msg.FromGalaxy(),
				msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;
	}

	// Transfer the passengers.
	loadShip->TransferColonists(unloadShip, count);

	// Revoke the bid so it cannot be accepted again.
	g_Au.BidRevoked(unloaderId, msg.ChannelName());
}

// This code only handles BoardTroops requests accepted by
// people, which generates a TroopsBoarded message. The person
// who sends the TroopsBoarded message is willing to unload
// troops to the person id identified in the message text.
// Cities load troops in DaBd::RxChatMessage when the
// BoardTroops message arrives.
void AuSceneServerApp::CommAcceptTroopsBoarded(DaPerson* unloadPerson, const AuCommTroopsBoarded& msg)
{
	// Get the number of troops to transfer from the message.
	unsigned int count = msg.Count();
	if(!count)
		return;	// Invalid data.

	// Locate the unloader person.
	const DaAtom* system = unloadPerson->FindEnvironmentAtom();
	if(!system)
		return;	// Unloader is not in a solar system? Problem!

	// The unloader must be aboard a ship.
	DaVessel* unloadShip = 0;
	if(unloadPerson->IsInSpacecraft())
		unloadShip = unloadPerson->FindParentVesselInEnv();
	if(!unloadShip)
		return;	// Unloader is not on a ship.
	if(!unloadShip->InChainOfCommand(unloadPerson))
		return;	// Unloader is not an officer of the ship.

	// Insure unloading ship actually has the troops.
	unsigned int troops = unloadShip->TroopBerthsOccupied();
	if(troops < count)
	{
		QString text(tr("Your ship does not have %1 troops aboard.").arg(count));
		AuCommCityChat mymsg(text, msg.ChannelName(), unloadPerson->Id(), unloadPerson->Name(), msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(),
				msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;
	}

	// Validate the transaction based on the loader.
	AuId loaderId = msg.LoaderId();	// person loading the troops
	DaAtom* loader = (DaAtom*)system->FindIdRecursive(loaderId);
	if(!loader)
	{
		AuCommCityMessage mymsg(AuCommCityMessage::TroopsSameSystem, msg.ChannelName(), unloadPerson->Id(), unloadPerson->Name(),
				msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;	// Loader not found in the same solar system; moved
				// away?
	}
	DaPerson* loadPerson = dynamic_cast<DaPerson*>(loader);
	if(!loadPerson)
		return;	// Loader must be a person.

	// Must locate the exact bid in the list of active bids.
	// The count may not match because the accepter may offer less
	// troops that the loader has offered as open.
	if(!g_Au.HasPendingTroops(&msg))
	{
		AuCommCityMessage mymsg(AuCommCityMessage::TroopBoardingNotActive, msg.ChannelName(), loader->Id(), loader->Name(),
				msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;	// Ignore; load request is no longer active.
	}

	// Insure physical locations permit acceptance of the offer.
	// - Loader in a space ship, and
	// - Loader ship near seller's space ship.
	// - Loader ship not the same as seller ship.
	DaVessel* loadShip = 0;
	if(loadPerson->IsInSpacecraft())
		loadShip = loadPerson->FindParentVesselInEnv();
	if(!loadShip)
		return;	// Loader is not on a ship.
	if(loadShip->Id() == unloadShip->Id())
		return;	// They are in the same ship.
	if(!loadShip->InChainOfCommand(loadPerson))
		return;	// Loader is not an officer of the ship.

	// Loader and unloader are aboard different ships.
	AuPoint unloadLoc = unloadShip->EnvLocation();
	AuPoint loadLoc = loadShip->EnvLocation();
	float tradeRange = unloadShip->TransferRange(loadShip);
	float distApart = unloadLoc.Distance(loadLoc);
	if(distApart > tradeRange)
	{
		AuCommCityMessage mymsg(AuCommCityMessage::TooFarToTransactTroops, msg.ChannelName(), unloadPerson->Id(),
				unloadPerson->Name(), msg.FromSectorId(), msg.FromGalaxy(), msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;
	}

	// Insure loading ship has enough room.
	unsigned int openBerths = loadShip->TroopBerthsAvailable();
	if(openBerths < count)
	{
		QString text(tr("Your ship does not have %1 empty troop berths.").arg(count));
		AuCommCityChat mymsg(text, msg.ChannelName(), loader->Id(), loader->Name(), msg.FromSectorId(), msg.FromGalaxy(),
				msg.ReceptionRange(), msg.Scope());
		g_Au.SendAtomChat(mymsg);
		return;
	}

	// Transfer the troops.
	loadShip->TransferTroops(unloadShip, count);

	// Revoke the bid so it cannot be accepted again.
	g_Au.BidRevoked(loaderId, msg.ChannelName());
}

void AuSceneServerApp::CommAcceptTeleport(DaPerson* fromPerson, const AuCommPacket& msg)
{
	if(!fromPerson)
		return;
	// Teleport the person to the sender of the message.
	//AdminGoTo(fromPerson->Id(), msg.ToId(), 0, false);		// This used to deadlock on root.
	HandoffElsewhere(fromPerson->Id(), msg.ToId(), 0, false, fromPerson->ParentId());
}

void AuSceneServerApp::CommAcceptCitySurrender(const AuCommCitySurrenderAccepted& msg)
{
	// Message validity check insures city ids and empire ids are valid.
	if(!msg.IsValid())
		return;

	quint32 acceptEmpire = msg.FromEmpireId();		// Empire accepting the city.
	const DgData& gd = g_Au.LockDataForRead();
	bool canAcceptCity = gd.EmpireCitizenCanAcceptCitySurrender(acceptEmpire, msg.FromId());
	g_Au.UnlockData();
	if(!canAcceptCity)
		return;		// Recipient is not eligible to accept the message for the empire.

	quint32 surrenderEmpire = msg.ToEmpireId();		// Empire surrendering the cities.
	if(surrenderEmpire == acceptEmpire)
		return;
	AuIdList cityIds;
	msg.CityIds(cityIds);
	for(AuIdList::const_iterator ci = cityIds.cbegin(); ci != cityIds.cend(); ci++)
		GovCitySurrenderedUp(msg.SurrenderingAvatarId(), *ci, surrenderEmpire, acceptEmpire);
}

/*
void AuSceneServerApp::AtomArrived(AuId idAtom, AuId idToSolarSystem)
{
	//qDebug("AuSceneServerApp::AtomArrived %d toSystem %d", idAtom.GetID(), idToSolarSystem.GetID());
	// Signal global data so database thread will load the nuly arrived atom.
	g_Au.AtomArrived(idAtom, idToSolarSystem);
	//qDebug("AuSceneServerApp::AtomArrived %d toSystem %d done", idAtom.GetID(), idToSolarSystem.GetID());
}
*/

void AuSceneServerApp::PlayerConnectingChat(quint32 playerId, unsigned long threadId)
{
	// Send a signal to each chat channel thread.
	emit DisconnectExcessChatChannels(playerId, threadId);
}

void AuSceneServerApp::PersonSystemChanged(AuId systemId, quint32 playerId)
{
	// Send a signal to each chat channel thread.
	emit SetPersonSystem(systemId, playerId);
}

void AuSceneServerApp::HandoffElsewhere(AuId personOrShip, AuId dest, quint8 rideAlso, bool limboRescue, AuId returnOnFail)
{
	if(g_Au.m_LogMessages & LOG_Travel)
		qDebug() << "AuSceneServerApp::HandoffElsewhere personOrShip" << personOrShip << "rideToo" << rideAlso << "dest" << dest
			<< "limboRescue" << limboRescue << "returnOnFail" << returnOnFail;
	AuEther& ether = g_Au.LockEtheriumDelete();
	HandoffElsewhere2(personOrShip, dest, rideAlso, limboRescue, returnOnFail, ether);
	g_Au.UnlockEtheriumDelete();
}

// Etherium delete lock is already locked.
void AuSceneServerApp::HandoffElsewhere2(AuId personOrShip, AuId dest, quint8 rideAlso, bool limboRescue, AuId returnOnFail,
		AuEther& ether)
{
	//Q_ASSERT(g_Au.IsSceneWorker());
	AuId atomId;	// Id of departing atom.
	QByteArray atomData;
	DaAtom* traveller = ether.FindNormalAtom(personOrShip);
	if(!traveller)
		traveller = ether.FindDormantAtom(personOrShip);	// Attempt to find dormant private ship that has been summoned.
	if(traveller)
	{
		// Determine which atom is actually moving.
		// It could be the person or their mount/vehicle/spacecraft if 'rideAlso' is true.
		DaAtom* moveAtom = 0;
		if(rideAlso)
		{
			if(dynamic_cast<DaCreature*>(traveller->Parent()) ||
					dynamic_cast<DaVehicle*>(traveller->Parent()))
				moveAtom = traveller->Parent();					// Parent is a creature or a vehicle.
			else if(rideAlso == 2)
				moveAtom = traveller->FindParentVesselInEnv();	// Parent spacecraft.
		}
		if(!moveAtom)
			moveAtom = traveller;
		atomId = moveAtom->Id();

		// Bundle the departing atom into a byte array of its own.
		// It is put into an atom list because the atom list knows how to deserialize the atoms properly.
		DaAtomList list;
		ether.RemoveAtom(moveAtom->Id(), list);	// Removes the atom and returns the list of moveAtom and its subatoms.
		QDataStream dst(&atomData, QIODevice::WriteOnly);
		dst.setVersion(QDataStream::Qt_4_1);
		dst << list;

		// Move the local atoms into the trash.
		ether.TrashAtoms(list);
	}
	if(!atomData.isEmpty())
	{
		HandoffAtomUp2(personOrShip, dest, rideAlso, limboRescue, atomData, returnOnFail);

		// Notify database thread so the atom parent id can be updated in the database.
		g_AuDb.AtomDeparted(atomId);
	}
}

void AuSceneServerApp::HandoffAtomUp(AuId personOrShip, AuId dest, quint8 rideAlso, bool limboRescue, QByteArray atomData,
		AuId returnOnFail)
{
	HandoffAtomUp2(personOrShip, dest, rideAlso, limboRescue, atomData, returnOnFail);
}

void AuSceneServerApp::HandoffAtomUp2(const AuId& personOrShip, const AuId& dest, quint8 rideAlso, bool limboRescue,
		const QByteArray& atomData, const AuId& returnOnFail)
{
	if(g_Au.IsSceneMaster())
	{
		// The scene master sends the atom handoff request down the chain by broadcasting a handoff query.
		if(limboRescue && (g_Au.m_LogMessages & LOG_Admin))
			g_Au.Log(tr("Limbo Rescue initiated for %1.").arg(dest.GetID()));
		HandoffAtomDown2(personOrShip, dest, rideAlso, limboRescue, atomData, 0, returnOnFail);
	}
	else if(m_tClientChannel)
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_HandoffElsewhere;
		out << personOrShip;
		out << dest;
		out << rideAlso;
		out << limboRescue;
		out << atomData;
		out << returnOnFail;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::HandoffAtomDown(AuId personOrShip, AuId dest, quint8 rideAlso, bool limboRescue, QByteArray atomData,
		AuId returnOnFail)
{
	if(g_Au.IsSceneWorker())
	{
		bool didSpawn = false;
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaUniverse* universe = ether.FindUniverse();
		if(universe)
		{
			// Locate the spawn point atom.
			DaAtom* destA = ether.FindNormalAtom(dest);
			if(destA)
			{
				// Deserialize the arriving atom data.
				// It is always sent in a DaAtomList because the list knows how to deserialize the atoms.
				DaAtomList list;
				QDataStream in(atomData);
				in.setVersion(QDataStream::Qt_4_1);
				in >> list;

				DaAtom* handoffA = list.first();
				if(handoffA)
				{
					if(g_Au.m_LogMessages & LOG_Travel)
					{
						QString logmsg = QString("Handoff of atom %1 %2 received by universe containing personOrShip %3")
							.arg(handoffA->Id().GetID()).arg(handoffA->Name()).arg(personOrShip.GetID());
						g_Au.Log(logmsg);
					}

					// Remove any previous copy of the traveller atom from my local data. It shouldn't be here anyway.
					// Also insure subatoms that showed up subordinate to the traveling atom are not already in my data.
					for(DaAtomList::const_iterator pi = list.cbegin(); pi != list.cend(); pi++)
					{
						// Search for any other copies of the atom in the local data.
						DaAtom* dup = ether.FindNormalAtom((*pi)->Id());
						if(dup)
						{
							if(g_Au.m_LogMessages & LOG_Error)
								g_Au.Error(QString("AuSceneServerApp::HandoffAtomDown - Atom %1 %2 handoff arrived and its already "
										"here, parent is %3 %4. Replacing the old atom with the nu data.")
										.arg(handoffA->Id().GetID()).arg(handoffA->Name())
										.arg(dup->ParentId().GetID()).arg(dup->Parent()->Name()));
							//DHH What if the dup has subatoms that must not be destroyed?
							ether.TrashAtom(dup->Id());
						}
					}

					// The arriving atoms are added to the ether. AdminGoTo will find the moving atom and move it to the dest atom.
					// Call SetModified on arriving atoms so they can be saved ASAP. It is possible they were just created on
					// the origin server and teleported here without ever being saved in the database yet.
					for(DaAtomList::const_iterator pi = list.cbegin(); pi != list.cend(); pi++)
					{
						(*pi)->ConnectSignals();
						ether.AddAtom(*pi);
						(*pi)->SetModified();
						(*pi)->TickTock();
					}

					// This MUST work. We've already verified the necessary criteria.
					didSpawn = AdminGoTo2(personOrShip, dest, rideAlso, ether);

					// Kick database to save now to minimize the chance of losing the travelling atom if this server crashes.
					g_AuDb.PostCriticalUpdates();
				}
				else
				{
					// Spawn failed due to bad data. Don't repost this one.
					didSpawn = true;
					g_Au.Error(QString("AuSceneServerApp::HandoffAtomDown - Failed to handoff personOrShip %1 to dest atom %2.")
							.arg(personOrShip.GetID()).arg(dest.GetID()));
				}
			}
			else
			{
				// Failed to locate handoff destination atom. This causes the handoff request to be reposted.
				g_Au.Error(QString("AuSceneServerApp::HandoffAtomDown - Atom %1 handoff. Dest personOrShip %2 is not here.")
						.arg(personOrShip.GetID()).arg(dest.GetID()));
			}
		}
		else
			g_Au.Error(QString("AuSceneServerApp::HandoffAtomDown - NO UNIVERSE for personOrShip %1 dest %2 rideAlso %3 data %4 "
						"bytes")
					.arg(personOrShip.GetID()).arg(dest.GetID()).arg(rideAlso).arg(atomData.count()));
		g_Au.UnlockEtheriumDelete();

		if(!didSpawn)
		{
			// Repost the handoff request.
			HandoffAtomUp2(personOrShip, dest, rideAlso, limboRescue, atomData, returnOnFail);
		}
	}
	else
	{
		// Propagate the atom handoff request down to the workers.
		// When I find out who has the handoff destination atom, the worker will be told to handoff the atom.
		//Q_ASSERT(g_Au.IsSceneManager());
		HandoffAtomDown2(personOrShip, dest, rideAlso, limboRescue, atomData, 0, returnOnFail);
	}
}

void AuSceneServerApp::HandoffAtomDown2(const AuId& personOrShip, const AuId& dest, quint8 rideAlso, bool limboRescue,
		const QByteArray& atomData, int timeOuts, const AuId& returnOnFail)
{
	//Q_ASSERT(!g_Au.IsSceneWorker());
	Q_ASSERT(!dest.IsNull());

	// Prepare to wait for the information we need to handoff the atom.
	AuHandoffRequest handoff;
	handoff.m_Start.Set();
	handoff.m_PersonOrShip = personOrShip;
	handoff.m_Dest = dest;
	handoff.m_RideAlso = rideAlso;
	handoff.m_LimboRescue = limboRescue;
	handoff.m_AtomData = atomData;
	handoff.m_TimeOuts = timeOuts;
	handoff.m_ReturnOnFail = returnOnFail;

	// This could be a repeat request so remove any pending request for the same atom.
	m_HandoffMutex.lock();
	QList<AuHandoffRequest>::iterator it = m_Handoffs.begin();
	while(it != m_Handoffs.end())
	{
		if((*it).m_PersonOrShip.IsNull() && personOrShip.IsNull())
		{
			// Limbo rescue requested by server manager.
			if((*it).m_Dest == dest)
				it = m_Handoffs.erase(it);						// Duplicate limbo rescue request.
			else
				it++;
		}
		else if((*it).m_PersonOrShip == personOrShip)
			it = m_Handoffs.erase(it);							// Duplicate request.
		else
			it++;
	}

	// Append the nu handoff request.
	m_Handoffs.append(handoff);
	m_HandoffMutex.unlock();

	// Broadcast atom query to subordinates to locate the destination atom.
	QueryAtomServer(dest, 0, AuId(0));
}

DaSolarSystem* AuSceneServerApp::AssignedSolarSystem(const AuId& systemId, AuEther& ether)
{
	if(systemId.IsNull())
		return 0;
	if(!IsSolarSystemAssignedToMe(systemId))
		return 0;						// Must return with root locked due to non-null system id.
	return dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(systemId));
}

void AuSceneServerApp::ListSystemsToRevoke(unsigned int serverId, float loadToRevoke, AuIdList& revokeSystems)
{
	//Q_ASSERT(g_Au.IsSceneManager());	// This is only called on the manager server.
	m_ServerChannel.LockServerList(__FILE__, __LINE__);
	DtServerChannel* channel = m_ServerChannel.ServerChannel(serverId);
	if(channel)
		channel->ListSystemsToRevoke(loadToRevoke, revokeSystems);
	m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
}

// A local atom is travelling from one solar system to another via stargate.
// This is where the trail begins when an atom departs a solar system through a stargate.
// This is not where spawning happens.
// If the origin and destination solar systems are in my locally managed data, the transfer happens immediately right here.
void AuSceneServerApp::AtomTraveledStargate(AuId traveler, AuId origSystem, AuId destGalaxy, AuPoint destGalaxyLoc)
{
	if(g_Au.m_LogMessages & LOG_Travel)
		qDebug() << "AuSceneServerApp::AtomTraveledStargate traveler" << traveler << "origSystem" << origSystem << "destGalaxy"
			<< destGalaxy << "destGalaxyLoc" << destGalaxyLoc;
	// Insure the originating system hasn't been revoked from me already.
	DaSolarSystem* orgSys;
	AuEther& ether = g_Au.LockEtheriumDelete();
	if(IsSolarSystemAssignedToMe(origSystem))
		orgSys = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(origSystem));
	else
		orgSys = 0;
	if(!orgSys || orgSys->IsOnlyStarFieldLoaded())
	{
		// Ignore this travel signal. The departing atom will be loaded by the next server. There the atom will have to decide to
		// do the handoff again.
		g_Au.UnlockEtheriumDelete();
		if(g_Au.m_LogMessages & LOG_Error)
			qWarning() << "AuSceneServerApp::AtomTraveledStargate traveler" << traveler << "origSystem not found" << origSystem
				<< "destGalaxy" << destGalaxy << "destGalaxyLoc" << destGalaxyLoc;
		return;
	}

	// See if we can identify the destination system locally.
	DaGalaxy* dGalaxy = dynamic_cast<DaGalaxy*>(ether.FindNormalAtom(destGalaxy));
	if(!dGalaxy)
	{
		g_Au.UnlockEtheriumDelete();
		if(g_Au.m_LogMessages & LOG_Error)
			qWarning() << "AuSceneServerApp::AtomTraveledStargate traveler" << traveler << "origSystem" << origSystem
				<< "destGalaxy not found" << destGalaxy << "destGalaxyLoc" << destGalaxyLoc;
		return;
	}

	AuId destSystem;
	DaSolarSystem* destSys = 0;
	{
		AuPoint destSectorLoc(destGalaxyLoc);
		destSectorLoc.Round(10);
		DaSector* sector = dGalaxy->FindSectorAt(destSectorLoc);
		if(sector)
		{
			AuPoint destSystemLoc = destGalaxyLoc - destSectorLoc;
			destSys = sector->FindSolarSystemAt(destSystemLoc);
			if(!destSys)
				destSys = sector->FindClosestSolarSystem(destSystemLoc);	// We MUST find a solar system. Agents use this.
			if(destSys)
				destSystem = destSys->Id();
		}
	}

	// If the destination solar system is in my data, I can perform the transfer right here and now.
	bool atomTraveled = false;
	if(!IsSolarSystemAssignedToMe(destSystem))	// This will always fail when traveling through intergalactic wormhole.
		destSys = 0;
	if(destSys && !destSys->IsOnlyStarFieldLoaded())
	{
		DaAtom* trav = orgSys->FindLiveIdRecursive(traveler);
		if(trav && trav->IsTraveling())
		{
			if(g_Au.m_LogMessages & LOG_Travel)
			{
				QString logmsg = QString("Atom %1 %2 traveled from system %3 %4 to system %5 %6 via stargate")
					.arg(trav->Id().GetID()).arg(trav->Name())
					.arg(orgSys->Id().GetID()).arg(orgSys->Name())
					.arg(destSys->Id().GetID()).arg(destSys->Name());
				g_Au.Log(logmsg);
			}

			// Add the traveling atom to its destination solar system.
			trav->ForcePos(AuAtomPos(destSystem, trav->Transform()));
			if(destSys->FindPersonOnLineRecursive())
				destSys->SetLastPersonVisited();
			if(destSys->FindOfficerRecursive() || destSys->FindAIPersonRecursive())
				destSys->SetLastOfficerVisited();
			destSys->SetModified();

			// Kick the travelling atom into motion.
			trav->SetNormal();						// Return to 'Normal' non-traveling state.
			trav->SetModified();
			trav->TickTockRecursive();

			// Mark the destination solar system as explored by the empire owning the travelling unit.
			DaUnit* unit = dynamic_cast<DaUnit*>(trav);
			if(unit)
				destSys->ExploredBy(unit->EmpireId());

			// Notify the system of the quality of non-pirate ships. We will use QualityMade because the ship design may
			// not be analyzed yet; HighestSystemQuality would be slightly preferable.
			const DaVessel* ship = dynamic_cast<const DaVessel*>(trav);
			if(ship && !ship->IsPirate())
				destSys->SetNonPirateShipPresent(ship->QualityMade());

			atomTraveled = true;
		}
		else
		{
			if(g_Au.m_LogMessages & LOG_Error)
			{
				if(trav)
					g_Au.Error(QString("AuSceneServerApp::AtomTraveledStargate traveler %1 is not traveling")
							.arg(traveler.GetID()));
				else
					g_Au.Error(QString("AuSceneServerApp::AtomTraveledStargate traveler %1 not found")
							.arg(traveler.GetID()));
			}
		}
	}
	g_Au.UnlockEtheriumDelete();

	if(!atomTraveled)
	{
		// The traveling atom could not be handed off locally. It will need to be saved in the database and then the
		// destination server notified.
		if(g_Au.m_LogMessages & LOG_Travel)
			qDebug("AuSceneServerApp::AtomTraveledStargate traveler %d departed via db", traveler.GetID());
		emit TravelerDepartedStargateViaDb(traveler, origSystem, destGalaxy, destGalaxyLoc);
	}
}

// A local atom is travelling from one solar system to another.
// This is where the trail begins when an atom departs a solar system, either through a wormhole or by dead-head through space.
// This is not where spawning happens.
// If the origin and destination solar systems are in my locally managed data, the transfer happens immediately right here.
// When traveling via intergalactic wormhole, 'destSystem' and 'destSector' are the destination galaxy id.
void AuSceneServerApp::AtomTraveled(AuId traveler, AuId origSystem, AuId destSystem, AuId destSector)
{
	if(g_Au.m_LogMessages & LOG_Travel)
		qDebug() << "AuSceneServerApp::AtomTraveled traveler" << traveler << "origSystem" << origSystem << "destSystem"
			<< destSystem << "destSector" << destSector;
	//Q_ASSERT(g_Au.IsSceneWorker());	// This should only be called on the worker server.

	// Insure the originating system hasn't been revoked from me already.
	DaSolarSystem* orgSys;
	AuEther& ether = g_Au.LockEtheriumDelete();
	if(IsSolarSystemAssignedToMe(origSystem))
		orgSys = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(origSystem));
	else
		orgSys = 0;
	if(!orgSys || orgSys->IsOnlyStarFieldLoaded())
	{
		// Ignore this travel signal. The departing atom will be loaded by the next server. There the atom will have to decide to
		// do the handoff again.
		g_Au.UnlockEtheriumDelete();
		if(g_Au.m_LogMessages & LOG_Error)
			g_Au.Warning(QString("AuSceneServerApp::AtomTraveled traveler %1 origSystem %2 not found. destSystem %3 destSect %4")
					.arg(traveler.GetID()).arg(origSystem.GetID()).arg(destSystem.GetID()).arg(destSector.GetID()));
		return;
	}

	// If the destination solar system is in my data, I can perform the transfer right here and now.
	bool atomTraveled = false;
	DaSolarSystem* destSys;
	if(IsSolarSystemAssignedToMe(destSystem))	// This will always fail when traveling through intergalactic wormhole.
		destSys = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(destSystem));
	else
		destSys = 0;
	if(destSys && !destSys->IsOnlyStarFieldLoaded())
	{
		DaAtom* trav = orgSys->FindLiveIdRecursive(traveler);
		if(trav && trav->IsTraveling())
		{
			if(g_Au.m_LogMessages & LOG_Travel)
			{
				QString logmsg = QString("Atom %1 %2 traveled from system %3 %4 to system %5 %6")
					.arg(trav->Id().GetID()).arg(trav->Name())
					.arg(orgSys->Id().GetID()).arg(orgSys->Name())
					.arg(destSys->Id().GetID()).arg(destSys->Name());
				g_Au.Log(logmsg);
			}

			// Add the traveling atom to its destination solar system.
			trav->ForcePos(AuAtomPos(destSystem, trav->Transform()));
			if(destSys->FindPersonOnLineRecursive())
				destSys->SetLastPersonVisited();
			if(destSys->FindOfficerRecursive() || destSys->FindAIPersonRecursive())
				destSys->SetLastOfficerVisited();
			destSys->SetModified();

			// Kick the travelling atom into motion.
			trav->SetNormal();						// Return to 'Normal' non-traveling state.
			trav->SetModified();
			trav->TickTockRecursive();

			// Mark the destination solar system as explored by the empire owning the travelling unit.
			DaUnit* unit = dynamic_cast<DaUnit*>(trav);
			if(unit)
				destSys->ExploredBy(unit->EmpireId());

			// Notify the system of the quality of non-pirate ships. We will use QualityMade because the ship design may
			// not be analyzed yet; HighestSystemQuality would be slightly preferable.
			const DaVessel* ship = dynamic_cast<const DaVessel*>(trav);
			if(ship && !ship->IsPirate())
				destSys->SetNonPirateShipPresent(ship->QualityMade());

			atomTraveled = true;
		}
		else
		{
			if(g_Au.m_LogMessages & LOG_Error)
			{
				if(trav)
					g_Au.Error(QString("AuSceneServerApp::AtomTraveled traveler %1 is not traveling").arg(traveler.GetID()));
				else
					g_Au.Error(QString("AuSceneServerApp::AtomTraveled traveler %1 not found").arg(traveler.GetID()));
			}
		}
	}
	g_Au.UnlockEtheriumDelete();

	if(!atomTraveled)
	{
		// The traveling atom could not be handed off locally. It will need to be saved in the database and then the
		// destination server notified.
		if(g_Au.m_LogMessages & LOG_Travel)
			qDebug("AuSceneServerApp::AtomTraveled traveler %d departed via db", traveler.GetID());
		emit TravelerDepartedViaDb(traveler, origSystem, destSystem, destSector);
	}
}

void AuSceneServerApp::AtomTraveledUp(AuId traveler, AuId destSystem, AuId destSector)
{
	if(g_Au.m_LogMessages & LOG_Travel)
		qDebug() << "AuSceneServerApp::AtomTraveledUp traveler" << traveler << "destSystem" << destSystem
			<< "destSector" << destSector;
	if(g_Au.IsSceneMaster())
	{
		// Master server manages universe atom.
		m_muPendingDep.lock();
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaUniverse* universe = ether.FindUniverse();
		if(universe)
			AtomTraveledUp2(traveler, destSystem, destSector, universe);
		g_Au.UnlockEtheriumDelete();
		m_muPendingDep.unlock();
	}
	else if(g_Au.IsSceneManager())
	{
		// Manager server manages sector atoms.
		m_muPendingDep.lock();
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaSector* sector;
		if(IsSectorAssignedToMe(destSector))
			sector = dynamic_cast<DaSector*>(ether.FindNormalAtom(destSector));
		else
			sector = 0;
		AtomTraveledUp2(traveler, destSystem, destSector, sector);	// 'sector' allows for optional shortcut down to worker
		g_Au.UnlockEtheriumDelete();
		m_muPendingDep.unlock();
	}
	else
	{
		// Worker server sends the notification up to its manager.
		AtomTraveledUp2(traveler, destSystem, destSector, 0);
	}
}

// m_muPendingDep must already be locked on master and manager
void AuSceneServerApp::AtomTraveledUp2(const AuId& id, const AuId& destSystem, const AuId& destSector, DaAtom* managedAtom)
{
	if(g_Au.m_LogMessages & LOG_Travel)
		qDebug() << "AuSceneServerApp::AtomTraveledUp2 id" << id << "destSystem" << destSystem << "destSector" << destSector
			<< "managedAtom" << *managedAtom;
	if(g_Au.IsSceneMaster())
	{
		// I only get this message when an atom has departed a worker server and its manager is not assigned the destination sector.
		// The manager propagates the departure message up to me and I need to pass it back down so it reaches the scene manager
		// responsible for the destination sector.
		// In the case of intergalactic wormhole travel, the destSector is actually a dest galaxy.
		AtomTraveledDown2(id, destSystem, destSector, managedAtom);	// Notify appropriate sector manager.
	}
	else
	{
		// If the destination system is in my sphere of control, I can simply notify the worker server that is receiving the atom.
		if(g_Au.IsSceneManager() && managedAtom)
		{
			if(AtomTraveledDown2(id, destSystem, destSector, managedAtom))
			{
				//qDebug("AuSceneServerApp::AtomTraveledUp2 traveler %d destSystem %d destSector %d took shortcut to worker",
						//id.GetID(), destSystem.GetID(), destSector.GetID());
				return;		// Transferred the atom to a subordinate worker server.
			}
		}

		// The destination solar system is not in my sphere of control.
		if(m_tClientChannel)
		{
			// Propagate the atom traveled message up to my boss server.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_AtomTraveling;
			id >> out;
			destSystem >> out;
			destSector >> out;
			m_tClientChannel->SendPacket(ba);
		}
	}
}

void AuSceneServerApp::AtomTraveledDown(AuId traveler, AuId destSystem, AuId destSector)
{
	if(g_Au.m_LogMessages & LOG_Travel)
		qDebug() << "AuSceneServerApp::AtomTraveledDown traveler" << traveler << "destSystem" << destSystem << "destSector"
			<< destSector;
	if(g_Au.IsSceneMaster())
	{
		// Master doesn't really ever receive this signal.
		Q_ASSERT(false);
	}
	else if(g_Au.IsSceneManager())
	{
		// Manager server manages sector atoms.
		m_muPendingDep.lock();
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaSector* sector;
		if(IsSectorAssignedToMe(destSector))
			sector = dynamic_cast<DaSector*>(ether.FindNormalAtom(destSector));
		else
			sector = 0;
		AtomTraveledUp2(traveler, destSystem, destSector, sector);	// Bounces up to master if can't send down to worker.
		g_Au.UnlockEtheriumDelete();
		m_muPendingDep.unlock();
	}
	else
	{
		// Worker server sends the notification up to its manager.
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaUniverse* universe = ether.FindUniverse();
		if(!universe || !AtomTraveledDown2(traveler, destSystem, destSector, universe))
			AtomTraveledUp2(traveler, destSystem, destSector, 0);	// Bounce back up to manager.
		g_Au.UnlockEtheriumDelete();
	}
}

// m_muPendingDep must already be locked on master and manager
bool AuSceneServerApp::AtomTraveledDown2(const AuId& id, const AuId& destSystem, const AuId& destSector, DaAtom* managedAtom)
{
	if(g_Au.m_LogMessages & LOG_Travel)
		qDebug() << "AuSceneServerApp::AtomTraveledDown2 id" << id << "destSystem" << destSystem << "destSector" << destSector
			<< "managedAtom" << *managedAtom;
	if(g_Au.IsSceneMaster())
	{
		DhSceneServer toServer = SectorServer(destSector);
		if(toServer.IsValid())
		{
			// The destination sector is already assigned, which means its contents have been finalized.
			// Notify the responsible server that an atom is destined for one of its solar systems.
			// Send the atom data down to the sub server.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_AtomTraveling;
			id >> out;
			destSystem >> out;
			destSector >> out;
			SendPacketTo(ba, toServer.m_Id);
		}
		else
		{
			// The sector is not assigned. It must be a perimeter sector.
			// The managedAtom on the master server is a DaUniverse.
			bool atomTransferred = false;
			DaAtom* destatom = managedAtom->FindIdRecursive(destSector);
			DaSector* atom = dynamic_cast<DaSector*>(destatom);
			if(atom)
			{
				DaGalaxy* galaxy = atom->FindParentGalaxy();
				Q_ASSERT(galaxy);

				// Generate the sector cube at the destination sector's location. This will cause it to get
				// assigned to a scene manager. Eventually, the responsible worker server will load the data
				// with the nuly arrived atom. All the while, the scene client is bouncing scene defer messages
				// off me until his atom shows up in the universe.
				// The m_NewSectors list is appended with the ids of any nu sectors that are created. The ids are
				// removed by DtDb after the sectors are stored. Sectors will not be assigned to a manager while
				// their id remains in the list. This way sectors do not get assigned to managers until they have
				// been stored in the database.
				if(g_Au.m_LogMessages & LOG_Travel)
					qDebug() << "AuSceneServerApp::AtomTraveledDown2 creating sector cube for traveler" << id
						<< "destSystem" << destSystem << "destSector" << destSector << "galaxy" << galaxy->Id() << galaxy->Name();
				m_rwNewSectors.lockForWrite();
				galaxy->CreateSectorCubeAt(atom->Location(), m_NewSectors);
				m_rwNewSectors.unlock();
				g_AuDb.PostCriticalUpdates();

				// Queue an atom departure for notification to the manager that gets assigned the sector after it
				// has been stored in the database.
				// This has already been done when we are being called from SendDepartingAtoms.
				// NOTE: The m_muPendingDep mutex must be locked already, before any atom locks.
				if(!m_PendingDepartures.contains(id))
				{
					AuAtomDeparture dep;
					dep.m_toSolarSystem = destSystem;
					dep.m_toSector = destSector;
					m_PendingDepartures.insert(id, dep);
				}
				atomTransferred = true;
			}
			else
			{
				// In the case of intergalactic wormhole travel, 'destSector' is actually a galaxy.
				DaGalaxy* destGalaxy = dynamic_cast<DaGalaxy*>(destatom);
				if(destGalaxy)
				{
					// Destination is the core sector of a galaxy.
					// Generate the sector cube at the destination sector's location. This will cause it to get
					// assigned to a scene manager. Eventually, the responsible worker server will load the data
					// with the nuly arrived atom. All the while, the scene client is bouncing scene defer messages
					// off me until his atom shows up in the universe.
					// The m_NewSectors list is appended with the ids of any nu sectors that are created. The ids are
					// removed by DtDb after the sectors are stored. Sectors will not be assigned to a manager while
					// their id remains in the list. This way sectors do not get assigned to managers until they have
					// been stored in the database.
					if(g_Au.m_LogMessages & LOG_Travel)
						qDebug() << "AuSceneServerApp::AtomTraveledDown2 creating sector cube for intergalactic traveler" << id
							<< "destGalaxy" << destGalaxy->Id() << destGalaxy->Name();
					m_rwNewSectors.lockForWrite();
					DaSector* sector = destGalaxy->CreateSectorCubeAt(DaGalaxy::CoreSectorLoc(), m_NewSectors);
					m_rwNewSectors.unlock();
					g_AuDb.PostCriticalUpdates();
					Q_ASSERT(sector);
					DaSolarSystem* coreSystem = sector->FindCoreSolarSystem();
					Q_ASSERT(coreSystem);

					// Queue an atom departure for notification to the manager that gets assigned the sector after it
					// has been stored in the database.
					// This has already been done when we are being called from SendDepartingAtoms.
					// NOTE: The m_muPendingDep mutex must be locked already, before any atom locks.
					if(!m_PendingDepartures.contains(id))
					{
						AuAtomDeparture dep;
						dep.m_toSolarSystem = coreSystem->Id();
						dep.m_toSector = sector->Id();
						m_PendingDepartures.insert(id, dep);
					}
					atomTransferred = true;
				}
				else
					g_Au.Error(QString("AuSceneServerApp::AtomTraveledDown2 dest sector %1 does not exist or it is not a sector.")
							.arg(destSector.GetID()));
			}
			if(!atomTransferred)
				return false;
		}
	}
	else if(g_Au.IsSceneManager())
	{
		// If the destination system is in my sphere of control, I can simply notify the worker server that is receiving the atom.
		// This manager might not have the worker in charge of the system, in which case the propagation stops for this manager.
		if(!IsSectorAssignedToMe(destSector))
		{
			if(g_Au.m_LogMessages & LOG_Travel)
				qDebug() << "AuSceneServerApp::AtomTraveledDown2 destSector" << destSector << "not assigned to me.";
			return false;	// The system is in a sector that is not in my domain.
		}
		DhSceneServer toServer = SolarSystemServer(destSystem);
		if(toServer.IsValid())
		{
			// The destination solar system is already assigned, which means its contents have been finalized.
			if(g_Au.m_LogMessages & LOG_Travel)
				qDebug() << "AuSceneServerApp::AtomTraveledDown2 notifying destSystem" << destSystem << "destSector" << destSector;
			// Notify the responsible worker server that an atom has arrived in one of its solar systems.
			// Send the atom data down to the sub server.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_AtomTraveling;
			id >> out;
			destSystem >> out;
			destSector >> out;
			SendPacketTo(ba, toServer.m_Id);
		}
		else
		{
			// The solar system is not assigned.
			// The managedAtom on a manager server is a DaSector.
			DaSolarSystem* atom = dynamic_cast<DaSolarSystem*>(managedAtom->FindIdRecursive(destSystem));
			if(atom)
			{
				// The destination atom exists in my local data. The destination must be in my sphere of control. The scene
				// manager only loads sectors that are assigned to it. All the solar systems in the sector are guaranteed to
				// exist but I am responsible for my systems' contents.
				if(g_Au.m_LogMessages & LOG_Travel)
					qDebug() << "AuSceneServerApp::AtomTraveledDown2 traveler" << id << "destSystem" << destSystem << "destSector"
							<< destSector << "finalizing system contents";

				// Finalize the solar system contents. This will cause it to be assigned to a worker server who will load all
				// the data, including the nuly arrived atom.
				atom->FinalizeContents();
				g_AuDb.PostCriticalUpdates();

				// Queue an atom departure for notification to the worker that gets assigned the system after it
				// has been stored in the database.
				// This has already been done when we are being called from SendDepartingAtoms.
				// NOTE: The m_muPendingDep mutex must be locked already, before any atom locks.
				if(!m_PendingDepartures.contains(id))
				{
					AuAtomDeparture dep;
					dep.m_toSolarSystem = destSystem;
					dep.m_toSector = destSector;
					m_PendingDepartures.insert(id, dep);
				}
			}
			else
			{
				if(g_Au.m_LogMessages & LOG_Travel)
					qDebug() << "AuSceneServerApp::AtomTraveledDown2 traveler" << id << "destSystem" << destSystem << "destSector"
							<< destSector << "dest system not found";
			}
		}
	}
	else if(g_Au.IsSceneWorker())
	{
		// This worker received the transfer down so it must be in charge of the destination solar system.
		DaSolarSystem* system;
		if(IsSolarSystemAssignedToMe(destSystem))
			system = dynamic_cast<DaSolarSystem*>(managedAtom->FindIdRecursive(destSystem));	// 'managedAtom' is universe
		else
			system = 0;
		if(system && !system->IsOnlyStarFieldLoaded())
		{
			emit TravelerArrivedViaDb(id, destSystem, destSector);
			if(g_Au.m_LogMessages & LOG_Travel)
				qDebug() << "AuSceneServerApp::AtomTraveledDown2 traveler" << id << "destSystem" << *system << "destSector"
					<< "emitting TravelerArrivedViaDb";
		}
		else
		{
			// The managed atom appears to be loaded only for the starfield.
			// Perhaps it was deassigned during the handoff.
			// It may also be assigned to me but the db thread is not done loading it yet.
			// Send it back for another round.
			AtomTraveledUp2(id, destSystem, destSector, 0);
			if(g_Au.m_LogMessages & LOG_Travel)
				qDebug() << "AuSceneServerApp::AtomTraveledDown2 traveler" << id << "destSystem" << destSystem << "destSector"
					<< "system loaded for starfield only";
		}
	}
	return true;
}

void AuSceneServerApp::AtomTraveledStargateUp(AuId traveler, AuId destGalaxy, AuPoint destGalaxyLoc)
{
	if(g_Au.m_LogMessages & LOG_Travel)
		qDebug() << "AuSceneServerApp::AtomTraveledStargateUp traveler" << traveler << "destGalaxy" << destGalaxy
			<< "destGalaxyLoc" << destGalaxyLoc;
	if(g_Au.IsSceneMaster())
	{
		// Master server manages universe atom.
		m_muPendingDep.lock();
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaUniverse* universe = ether.FindUniverse();
		if(universe)
			AtomTraveledStargateUp2(traveler, destGalaxy, destGalaxyLoc, universe);
		g_Au.UnlockEtheriumDelete();
		m_muPendingDep.unlock();
	}
	else if(g_Au.IsSceneManager())
	{
		// Manager server manages sector atoms.
		m_muPendingDep.lock();
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaGalaxy* galaxy = dynamic_cast<DaGalaxy*>(ether.FindNormalAtom(destGalaxy));
		AuId destSector;
		if(galaxy)
		{
			AuPoint sectorLoc(destGalaxyLoc);
			sectorLoc.Round(10);
			DaSector* sector = galaxy->FindSectorAt(sectorLoc);
			if(sector)
				destSector = sector->Id();
		}
		DaSector* sector;
		if(!destSector.IsNull() && IsSectorAssignedToMe(destSector))
			sector = dynamic_cast<DaSector*>(ether.FindNormalAtom(destSector));
		else
			sector = 0;
		AtomTraveledStargateUp2(traveler, destGalaxy, destGalaxyLoc, sector);	// 'sector' allows for shortcut down to worker
		g_Au.UnlockEtheriumDelete();
		m_muPendingDep.unlock();
	}
	else
	{
		// Worker server sends the notification up to its manager.
		AtomTraveledStargateUp2(traveler, destGalaxy, destGalaxyLoc, 0);
	}
}

// m_muPendingDep must already be locked on master and manager
void AuSceneServerApp::AtomTraveledStargateUp2(const AuId& id, const AuId& destGalaxy, const AuPoint& destGalaxyLoc,
		DaAtom* managedAtom)
{
	if(g_Au.m_LogMessages & LOG_Travel)
		qDebug() << "AuSceneServerApp::AtomTraveledStargateUp2 id" << id << "destGalaxy" << destGalaxy
			<< "destGalaxyLoc" << destGalaxyLoc << "managedAtom" << *managedAtom;
	if(g_Au.IsSceneMaster())
	{
		// I only get this message when an atom has departed a worker server and its manager is not assigned the destination sector.
		// The manager propagates the departure message up to me and I need to pass it back down so it reaches the scene manager
		// responsible for the destination sector.
		// In the case of intergalactic wormhole travel, the destSector is actually a dest galaxy.

		// Find the destination sector.
		DaGalaxy* galaxy = dynamic_cast<DaGalaxy*>(managedAtom->FindSubatomId(destGalaxy));
		if(galaxy)
		{
			AuPoint sectorLoc(destGalaxyLoc);
			sectorLoc.Round(10);

			// Make sure the cube is there in case the sector decayed or never existed. Agents use this.
			m_rwNewSectors.lockForWrite();
			DaSector* sector = galaxy->CreateSectorCubeAt(sectorLoc, m_NewSectors);
			m_rwNewSectors.unlock();
			g_AuDb.PostCriticalUpdates();

			if(sector)
			{
				AuPoint systemLoc = destGalaxyLoc - sectorLoc;
				DaSolarSystem* system = sector->FindSolarSystemAt(systemLoc);
				if(!system)
					system = sector->FindClosestSolarSystem(systemLoc);	// We MUST find a solar system. Agents use this.
				if(system)
					AtomTraveledDown2(id, system->Id(), sector->Id(), managedAtom);
			}
		}
	}
	else
	{
		// If the destination system is in my sphere of control, I can simply notify the worker server that is receiving the atom.
		if(g_Au.IsSceneManager() && managedAtom)
		{
			// Manager server manages sector atoms.
			AuPoint sectorLoc(destGalaxyLoc);
			sectorLoc.Round(10);
			AuPoint systemLoc = destGalaxyLoc - sectorLoc;
			DaSolarSystem* system = ((DaSector*)managedAtom)->FindSolarSystemAt(systemLoc);
			if(!system)
				system = ((DaSector*)managedAtom)->FindClosestSolarSystem(systemLoc);	// We MUST find a system. Agents use this.
			if(system)
			{
				if(AtomTraveledDown2(id, system->Id(), managedAtom->Id(), managedAtom))
				{
					//qDebug("AuSceneServerApp::AtomTraveledUp2 traveler %d destSystem %d destSector %d "
							//"took shortcut to worker",
							//id.GetID(), destSystem.GetID(), destSector.GetID());
					return;		// Transferred the atom to a subordinate worker server.
				}
			}
		}

		// The destination solar system is not in my sphere of control.
		if(m_tClientChannel)
		{
			// Propagate the atom traveled message up to my boss server.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_AtomTravelingStargate;
			out << id;
			out << destGalaxy;
			out << destGalaxyLoc;
			m_tClientChannel->SendPacket(ba);
		}
	}
}

void AuSceneServerApp::FinalizeSystem(AuId systemId)
{
	if(g_Au.IsSceneMaster())
	{
		// The scene master finalizes sector contents.
		// Make sure I have not already assigned the sector to somebody. That would imply the sector cube was already finalized.
		DhSceneServer toServer = SectorServer(systemId);
		if(toServer.IsValid())
		{
			qDebug() << "AuSceneServerApp::FinalizeSystem Sector" << systemId << "already finalized. Assigned to manager"
				<< toServer.m_Id << toServer.m_NetName << ":" << toServer.m_NetPort;
			return;
		}

		AuEther& ether = g_Au.LockEtheriumDelete();
		DaSector* sector = dynamic_cast<DaSector*>(ether.FindNormalAtom(systemId));
		if(sector)
		{
			DaGalaxy* galaxy = sector->FindParentGalaxy();
			Q_ASSERT(galaxy);

			// Generate the sector cube at the destination sector's location. This will cause it to get
			// assigned to a scene manager.
			m_rwNewSectors.lockForWrite();
			galaxy->CreateSectorCubeAt(sector->Location(), m_NewSectors);
			m_rwNewSectors.unlock();
			g_AuDb.PostCriticalUpdates();
			qDebug() << "AuSceneServerApp::FinalizeSystem Sector" << sector->Id() << sector->Name()
				<< "finalized, ready for assignment.";
		}
		else
			qDebug() << "AuSceneServerApp::FinalizeSystem sector not found" << systemId;
		g_Au.UnlockEtheriumDelete();
	}
	else if(g_Au.IsSceneManager())
	{
		// The scene manager finalizes solar system contents.
		// Make sure I have not already assigned the solar system to somebody. That would imply the contents were already finalized.
		DhSceneServer toServer = SolarSystemServer(systemId);
		if(toServer.IsValid())
		{
			qDebug() << "AuSceneServerApp::FinalizeSystem Solar system" << systemId << "already finalized. Assigned to worker"
				<< toServer.m_Id << toServer.m_NetName << ":" << toServer.m_NetPort;
			return;
		}

		AuEther& ether = g_Au.LockEtheriumDelete();
		DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(systemId));
		if(system && IsSectorAssignedToMe(system->ParentId()))
		{
			// The solar system's parent sector is assigned to me which means I have authority over the solar system.
			// Finalize the solar system contents. This will cause it to be assigned to a worker server who will load all
			// the data, including the nuly arrived atom.
			system->FinalizeContents();
			g_AuDb.PostCriticalUpdates();
			qDebug() << "AuSceneServerApp::FinalizeSystem Solar System" << system->Id() << system->Name()
				<< "finalized, ready for assignment.";
		}
		else if(!system)
			qDebug() << "AuSceneServerApp::FinalizeSystem system not found" << systemId;
		else
			qDebug() << "AuSceneServerApp::FinalizeSystem parent sector of system" << systemId << "not assigned to me";
		g_Au.UnlockEtheriumDelete();
	}
	else
		qWarning() << "AuSceneServerApp::FinalizeSystem Has no effect on this server.";
}

// This is the beginning of the trail when a person atom needs to respawn on a different server.
// This method uses the database to handoff the person atom.
void AuSceneServerApp::SpawnAtom(AuId personOrShip, quint32 empireId, AuId spawnCity, AuId spawnShip, AuId arena, quint8 galaxy)
{
	//Q_ASSERT(g_Au.IsSceneWorker());
	if(g_Au.m_LogMessages & LOG_Spawn)
		qDebug() << "AuSceneServerApp::SpawnAtom personOrShip" << personOrShip << "empire" << empireId << "spawnCity" << spawnCity
			<< "spawnShip" << spawnShip << "arena" << arena << "galaxy" << GalaxyName((GALAXY)galaxy);
	emit SpawnAtomViaDb(personOrShip, empireId, spawnCity, spawnShip, arena, galaxy);
}

void AuSceneServerApp::SpawnAtomUp(AuId personOrShip, quint32 empire, AuId spawnCity, AuId spawnShip, AuId arena, quint8 galaxy)
{
	SpawnAtomUp2(personOrShip, empire, spawnCity, spawnShip, arena, (GALAXY)galaxy);
}

void AuSceneServerApp::SpawnAtomUp2(const AuId& personOrShip, quint32 empire, const AuId& spawnCity, const AuId& spawnShip,
		const AuId& arena, GALAXY galaxy)
{
	if(g_Au.IsSceneMaster())
	{
		// This is part of the chain of events resulting from an atom, either a person or a spacecraft, initiating a respawn
		// request. The hosting worker server failed to find the spawn location in its local data so it propagated the request
		// up to the master server.
		// The scene master sends the atom spawn request down the chain by broadcasting a spawn query.
		// We also get here when limbo rescuing an atom who has no empire city to be sent to.
		AuId capitolU, capitolG, randomCity;
		if(empire)
		{
			const DgData& gd = g_Au.LockDataForRead();
			// Determine the capitol of the empire, if any. This could be null.
			gd.EmpireCapital(empire, capitolU);
			capitolG = gd.EmpireGalaxyCapital(empire, galaxy);

			// Find a random city in case none of the predetermined spawn locations are available.
			QSet<AuId> exceptIds;
			exceptIds.insert(capitolU);
			exceptIds.insert(capitolG);
			exceptIds.insert(spawnCity);
			gd.RandomEmpireCity(empire, randomCity, exceptIds);
			g_Au.UnlockData();
		}

		//qDebug() << "Scene Master is spawning the atom down: " << personOrShip << " " << empire << " " 
			//<< spawnCity << " " << spawnShip << " " << capitol << " " << randomCity;
		SpawnAtomDown2(personOrShip, empire, spawnCity, spawnShip, capitolU, capitolG, randomCity, arena, galaxy);
	}
	else if(m_tClientChannel)
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_AtomSpawning;
		out << personOrShip;
		out << empire;
		out << spawnCity;
		out << spawnShip;
		out << arena;
		out << (quint8)galaxy;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::SpawnAtomDown(AuId personOrShip, quint32 empire, AuId spawnCity, AuId spawnShip, AuId spawnAt,
		AuId arena, quint8 galaxy)
{
	if(g_Au.IsSceneWorker())
	{
		// This worker received the spawn atom down so it must be in charge of the spawnAt atom.
		bool atomTransferred = false;

		// Locate the spawn point atom.
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaAtom* spawnA = ether.FindNormalAtom(spawnAt);
		if(spawnA)
		{
			const DaSolarSystem* system = spawnA->FindParentSolarSystem();
			if(system && !system->IsOnlyStarFieldLoaded())
			{
				const DaSector* sector = system->FindParentSector();
				if(sector)
				{
					// At this point, the atom can be treated like an arriving traveler.
					// The DaAtom::LoadSubatoms code triggers the necessary actions when a spawning atom is loaded.
					emit TravelerArrivedViaDb(personOrShip, system->Id(), sector->Id());
					atomTransferred = true;
				}
			}
		}
		g_Au.UnlockEtheriumDelete();

		if(!atomTransferred)
			SpawnAtomUp2(personOrShip, empire, spawnCity, spawnShip, arena, (GALAXY)galaxy);		// Repost the spawn request.
	}
	else
	{
		// Propagate the atom spawn request down to the workers.
		// When I find out who has the spawn point, the worker will be told to spawn the atom.
		// At this stage of the game we must ignore the empire. If the spawn point changed ownership in the last second or two
		// we don't care too much.
		//Q_ASSERT(g_Au.IsSceneManager());
		SpawnAtomDown2(personOrShip, empire, spawnAt, AuId(0), AuId(0), AuId(0), AuId(0), AuId(0), (GALAXY)galaxy);	// Look only for the spawnAt atom.
	}
}

void AuSceneServerApp::SpawnAtomDown2(const AuId& person, quint32 empire, const AuId& spawnCity, const AuId& spawnShip,
		const AuId& capitolU, const AuId& capitolG, const AuId& randomCity, const AuId& arena, GALAXY galaxy)
{
	//Q_ASSERT(!g_Au.IsSceneWorker());

	// Prepare to wait for the information we need to make a proper decision about where to spawn the person.
	AuSpawnRequest spawn;
	spawn.m_Start.Set();
	spawn.m_Person = person;
	spawn.m_Empire = empire;
	spawn.m_CapitolU = capitolU;
	spawn.m_CapitolG = capitolG;
	spawn.m_City = spawnCity;
	spawn.m_Galaxy = galaxy;
	spawn.m_Ship = spawnShip;
	spawn.m_Arena = arena;
	spawn.m_RandomCity = randomCity;

	// This could be a repeat request so remove any pending request for the same person or ship.
	m_SpawnMutex.lock();
	QList<AuSpawnRequest>::iterator it = m_Spawns.begin();
	while(it != m_Spawns.end())
	{
		if((*it).m_Person == person)
			it = m_Spawns.erase(it);
		else
			it++;
	}

	// Append the nu spawn request.
	m_Spawns.append(spawn);
	//qDebug() << "SpawnAtomDown2" << person << "spawn request count" << m_Spawns.count();
	m_SpawnMutex.unlock();

	// Broadcast atom queries to subordinates to locate the spawn points.
	// We prefer to find the arena first because that could terminate our search.
	if(!arena.IsNull())
		QueryAtomServer(arena, empire, person);
	if(!spawnShip.IsNull())
		QueryAtomServer(spawnShip, empire, person);
	if(!spawnCity.IsNull())
		QueryAtomServer(spawnCity, empire, person);
	if(!capitolG.IsNull())
		QueryAtomServer(capitolG, empire, person);
	if(!capitolU.IsNull())
		QueryAtomServer(capitolU, empire, person);
	if(!randomCity.IsNull())
		QueryAtomServer(randomCity, empire, person);
}

void AuSceneServerApp::NameSpeciesUp(AuSpecies species, QString specieName, AuId whoNamed)
{
	if(g_Au.IsSceneMaster())
	{
		DgData& gd = g_Au.LockDataForWrite();
		bool existingSpecies = gd.HasSpecies(species);
		gd.NameSpecies(species, specieName, whoNamed);
		g_Au.UnlockData();

		if(existingSpecies)
			emit ChangeSpeciesInDb(species);
		else
			emit AddSpeciesToDb(species);

		// Notify the subs. We don't have to wait for the db to save the record.
		emit SpeciesNameChanged(species, specieName, whoNamed);
	}
	else if(m_tClientChannel)
	{
		// Queue a message for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_NameSpecies;
		out << species;
		out << specieName;
		out << whoNamed;
		m_tClientChannel->SendPacket(ba);
	}
}

// Statistics about ticking atoms, by system. Return values are in 1/2 millisecond increments.
// 'systemTotal' is sum of average tick times.
// 'systemMax' is sum of max tick times.
// 'systemCycle' is average of average cycle times.
// 'systemCycleMax' is average of max cycle times.
void AuSceneServerApp::TallySystemTickTotals(QHash<AuId, unsigned int>& systemTotals, QHash<AuId, unsigned int>& systemMax,
		QHash<AuId, unsigned int>& systemCycle, QHash<AuId, unsigned int>& systemCycleMax) const
{
	QHash<AuId, unsigned int> systemTotalsCount;
	QHash<AuId, unsigned int> systemMaxCount;
	QHash<AuId, unsigned int> systemCycleCount;
	QHash<AuId, unsigned int> systemCycleMaxCount;

	systemTotals.clear();
	systemMax.clear();
	systemCycle.clear();
	systemCycleMax.clear();

	g_Au.TallyAtomTickerSystemTimes(systemTotals, systemTotalsCount, systemMax, systemMaxCount, systemCycle, systemCycleCount,
			systemCycleMax, systemCycleMaxCount);

	/*
	// Divide the totals by the counts to determine the averages.
	unordered_map<AuId, unsigned int, AuIdHash>::iterator si = systemTotals.begin();
	unordered_map<AuId, unsigned int, AuIdHash>::iterator sic = systemTotalsCount.begin();
	while(si != systemTotals.end() && sic != systemTotalsCount.end())
	{
		if(*si && *sic > 1)
		{
			// Total is non-zero and there was more than one.
			*si /= *sic;
			if(!*si)
				*si = 1;	// Minimum of 1.
		}
		si++;
		sic++;
	}

	unordered_map<AuId, unsigned int, AuIdHash>::iterator mi = systemMax.begin();
	unordered_map<AuId, unsigned int, AuIdHash>::iterator mic = systemMaxCount.begin();
	while(mi != systemMax.end() && mic != systemMaxCount.end())
	{
		if(*mi && *mic > 1)
		{
			// Total is non-zero and there was more than one.
			*mi /= *mic;
			if(!*mi)
				*mi = 1;	// Minimum of 1.
		}
		mi++;
		mic++;
	}
	*/

	QHash<AuId, unsigned int>::iterator ci = systemCycle.begin();
	QHash<AuId, unsigned int>::iterator cic = systemCycleCount.begin();
	while(ci != systemCycle.end() && cic != systemCycleCount.end())
	{
		if(*ci && *cic > 1)
		{
			// Total is non-zero and there was more than one.
			*ci /= *cic;
			if(!*ci)
				*ci = 1;	// Minimum of 1.
		}
		ci++;
		cic++;
	}

	QHash<AuId, unsigned int>::iterator yi = systemCycleMax.begin();
	QHash<AuId, unsigned int>::iterator yic = systemCycleMaxCount.begin();
	while(yi != systemCycleMax.end() && yic != systemCycleMaxCount.end())
	{
		if(*yi && *yic > 1)
		{
			// Total is non-zero and there was more than one.
			*yi /= *yic;
			if(!*yi)
				*yi = 1;	// Minimum of 1.
		}
		yi++;
		yic++;
	}

	/*
	// Atoms Managed Status
	m_rwAtomManagers.lockForRead();

	for(unordered_map<AuId, AuAtomManaged*, AuIdHash>::const_iterator msi = m_MySystems.cbegin(); msi != m_MySystems.cend(); msi++)
	{
		if(!g_Au.m_ExcludeFromTick.contains((*msi)->AtomId()))
		{
			systemTotals.insert((*msi)->AtomId(), (*msi)->AverageTickTime());
			systemMax.insert((*msi)->AtomId(), (*msi)->MaxTickTime());

			systemCycle.insert((*msi)->AtomId(), (*msi)->AverageCycleTime());
			systemCycleMax.insert((*msi)->AtomId(), (*msi)->MaxCycleTime());
		}
		else
		{
			// Report 0's on atoms that are excluded from the tick cycle.
			systemTotals.insert((*msi)->AtomId(), 0);
			systemMax.insert((*msi)->AtomId(), 0);

			systemCycle.insert((*msi)->AtomId(), 0);
			systemCycleMax.insert((*msi)->AtomId(), 0);
		}
	}

	m_rwAtomManagers.unlock();
	*/
}

void AuSceneServerApp::StatusReportUsers(QDataStream& out)
{
	AuPlayerOnlineStatusVector players;
	PlayersOnlineStatus(players);							// Collect current list of players who are on line.
	players >> out;
}

void AuSceneServerApp::StatusReportLoads(QDataStream& out)
{
	QHash<AuId, unsigned int> systemTotals;
	QHash<AuId, unsigned int> systemMax;
	g_Au.TallyAtomTockerSystemTimes(systemTotals, systemMax);	// Checks for deadlocks, which may take a long time.

	QHash<AuId, unsigned int> sysTickTotals;
	QHash<AuId, unsigned int> sysTickMax;
	QHash<AuId, unsigned int> sysTickCycle;
	QHash<AuId, unsigned int> sysTickCycleMax;
	TallySystemTickTotals(sysTickTotals, sysTickMax, sysTickCycle, sysTickCycleMax);

	out << (quint8)m_AtomManagerCount;
	out << (quint16)systemTotals.size();

	for(QHash<AuId, unsigned int>::iterator si = systemTotals.begin(); si != systemTotals.end(); si++)
	{
		out << si.key();
		// Tick values. If a system is ticking it is usually tocking too.
		QHash<AuId, unsigned int>::const_iterator tsi = sysTickTotals.find(si.key());
		if(tsi != sysTickTotals.cend())
		{
			out << *tsi;										// This value is in 1/2 millisecond increments.
			out << sysTickMax[tsi.key()];						// This value is in 1/2 millisecond increments.
			out << sysTickCycle[tsi.key()];						// This value is in 1/2 millisecond increments.
			out << sysTickCycleMax[tsi.key()];					// This value is in 1/2 millisecond increments.
		}
		else
		{
			out << (unsigned int)0;
			out << (unsigned int)0;
			out << (unsigned int)0;
			out << (unsigned int)0;
		}

		// Tock values.
		out << *si;												// This value is in 1/2 millisecond increments.
		out << systemMax[si.key()];								// This value is in 1/2 millisecond increments.
		//out << (*msi)->LockTime();
	}

	bool bdLock = !g_Au.TestBdLock();
	bool dataLock = !g_Au.TestDataLock();
	bool itLock = !g_Au.TestItLock();
	bool scLock = !g_Au.TestScLock();
	bool trashLock = !g_Au.TestEtheriumTrashLock();

	// Connected Clients Status
	if(g_Au.IsSceneMaster())
	{
		// Master sends etherium lock.
		int etheriumLock = g_Au.TestEtheriumLock(10000);	// 10 seconds is a ridiculous amount of time to wait.
		out << (quint16)0;
		out << (float)0;
		out << etheriumLock;
		out << bdLock;
		out << dataLock;
		out << itLock;
		out << scLock;
		out << trashLock;
	}
	else if(g_Au.IsSceneManager())
	{
		// Manager sends the list of subordinate servers and their current load values.
		int etheriumLock = g_Au.TestEtheriumLock(10000);	// 10 seconds is a ridiculous amount of time to wait.
		const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
		out << (quint16)servers.size();
		for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = servers.cbegin(); it != servers.cend(); it++)
		{
			unsigned int systemsAssigned = CountAssignedTo((*it).second->Id());

			// This is formatted exactly like the server status below, to keep the stream from changing.
			out << (quint32)(*it).second->Id();
			out << (quint8)2;	// 2=server
			out << (*it).second->InternetName();
			out << (quint32)(*it).second->InternetPort();
			out << (quint32)0;
			out << AuId(systemsAssigned);					// System count instead of system id.
			out << (*it).second->ConnectTime().UTC();
			out << (quint32)(*it).second->BytesTx();
			out << (quint32)(*it).second->BytesRx();
			out << (quint16)(*it).second->BadPacketCount();
			out << (quint32)(*it).second->TickTime()/2;		// Tick in SyncCount column.
			out << (quint16)((*it).second->AtomLoad()*100);
			out << (quint32)(*it).second->CurrentLoad()/2;
			out << (quint16)0;
			out << (quint16)0;
			out << (quint32)0;
			out << (quint16)0;
		}
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
		out << (float)0;
		out << etheriumLock;
		out << bdLock;
		out << dataLock;
		out << itLock;
		out << scLock;
		out << trashLock;
	}
	else if(g_Au.IsSceneWorker())
	{
		float atomLoad = AtomManagerLoadAverage();
		int etheriumLock = g_Au.TestEtheriumLock(10000);	// 10 seconds is a ridiculous amount of time to wait.
		const unordered_map<unsigned int, DtSceneServer*>& servers = m_TcpServer.LockServerList(__FILE__, __LINE__);
		out << (quint16)servers.size();
		for(unordered_map<unsigned int, DtSceneServer*>::const_iterator it = servers.cbegin(); it != servers.cend(); it++)
		{
			out << (quint32)(*it).second->PlayerId();
			out << (quint8)(*it).second->AccountType();
			out << (*it).second->UnitName();
			out << (*it).second->UnitId();
			out << (quint32)(*it).second->UnitEmpireId();
			out << (*it).second->ClientSolarSystem();
			out << (*it).second->ConnectTime().UTC();
			out << (quint32)(*it).second->BytesTx();
			out << (quint32)(*it).second->BytesRx();
			out << (quint16)(*it).second->BadPacketCount();
			out << (quint32)(*it).second->SyncCount();
			out << (quint16)(*it).second->SyncAverageAtoms();
			out << (quint32)(*it).second->SyncAverageBytes();
			out << (quint16)(*it).second->SyncAveragePing();
			out << (quint16)(*it).second->SyncLastAtoms();
			out << (quint32)(*it).second->SyncLastBytes();
			out << (quint16)(*it).second->SyncLastPing();
		}
		m_TcpServer.UnlockServerList(__FILE__, __LINE__);
		out << atomLoad;
		out << etheriumLock;
		out << bdLock;
		out << dataLock;
		out << itLock;
		out << scLock;
		out << trashLock;
	}
}

// Report resource usage into a data stream.
void AuSceneServerApp::ResourceReport(QDataStream& out)
{
	ResourceReportAtoms(out, AuId(0));
	ResourceReportModels(out);
}

// Report resource usage of a specific atom and its subatoms into a data stream.
void AuSceneServerApp::ResourceReport(QDataStream& out, const AuId& atomId)
{
	out << atomId;
	ResourceReportAtoms(out, atomId);
}

// Report resource usage into a data stream.
void AuSceneServerApp::ResourceReportAtoms(QDataStream& out, const AuId& atomId)
{
	// Atom stats
	quint32 dead = 0;
	quint32 dying = 0;
	quint32 travel = 0;
	QHash<DaAtom::ATOM_TYPE, quint32> atomsByType;
	QStringList atomsInMotion;
	QStringList peopleOnLine;

	// List the atoms we will report.
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaAtomList allatoms;
	if(atomId.IsNull())
		ether.ListAtoms(allatoms);
	else
	{
		// We are supposed to report a specific atom and its subatoms.
		DaAtom* atom = ether.FindLiveAtom(atomId);
		if(atom)
		{
			// Report information about the specific atom.
			out << (quint8)atom->AtomType();		// All atoms have a non-zero type.
			out << atom->Name();
			out << atom->Description();
			out << atom->SpotDescription(0);
			out << atom->CreatedTime().UTC();
			out << atom->ModifyTime().UTC();
			out << QDateTime();	// atom->TickTime().UTC();
			out << atom->TockTime().UTC();
			out << QDateTime();	// atom->SceneTime();
			out << atom->IsInAnyScene();
			out << atom->IsAtRest();
			out << atom->Vector().x;
			out << atom->Vector().y;
			out << atom->Vector().z;

			allatoms.append(atom);
			atom->ListSubatomsRecursive(allatoms);
		}
		else
			out << (unsigned char)0;			// signal that the atom was not found. It could be dead.
	}

	for(DaAtomList::const_iterator it = allatoms.cbegin(); it != allatoms.cend(); it++)
	{
		DaAtom* atom = *it;
		if(!atom)
			continue;
		if(atom->IsDead())
			dead++;
		else if(atom->IsDying())
			dying++;
		if(atom->IsTraveling())
			travel++;

		QHash<DaAtom::ATOM_TYPE, quint32>::iterator ai = atomsByType.find(atom->AtomType());
		if(ai == atomsByType.end())
			atomsByType.insert(atom->AtomType(), 1);
		else
			(*ai)++;

		if(!atom->IsAtRest())
		{
			DaSolarSystem* system = atom->FindParentSolarSystem();
			if(system && system->IsTickingContents())
			{
				QString desc = atom->Description();
				desc += QString(", system %1 %2")
					.arg(system->Id().GetID())
					.arg(system->Name());
				if(atom->MustNotTickTock())
					desc += ", must not tick tock";
				QString stat = tr("%1|%2|%3|%4").arg(atom->Id().GetID()).arg(atom->AtomType()).arg(atom->Name()).arg(desc);
				atomsInMotion << stat;
			}
		}
	}

	out << dead;
	out << dying;
	out << travel;
	out << atomsInMotion;
	out << peopleOnLine;

	out << (quint8)atomsByType.size();
	for(QHash<DaAtom::ATOM_TYPE, quint32>::const_iterator ai = atomsByType.cbegin(); ai != atomsByType.cend(); ai++)
	{
		out << (quint8)ai.key();
		out << ai.value();
	}

	// Trash stats
	quint32 deadTrash = 0;
	quint32 dyingTrash = 0;
	quint32 travelTrash = 0;
	QHash<DaAtom::ATOM_TYPE, quint32> trashByType;

	QHash<DaAtom*, AuElapsedTimer>& trash = ether.LockTrash();
	for(QHash<DaAtom*, AuElapsedTimer>::const_iterator it = trash.cbegin(); it != trash.cend(); it++)
	{
		DaAtom* atom = it.key();
		if(!atom)
			continue;
		if(atom->IsDead())
			deadTrash++;
		else if(atom->IsDying())
			dyingTrash++;
		if(atom->IsTraveling())
			travelTrash++;

		QHash<DaAtom::ATOM_TYPE, quint32>::iterator ai = trashByType.find(atom->AtomType());
		if(ai == trashByType.end())
			trashByType.insert(atom->AtomType(), 1);
		else
			(*ai)++;
	}
	ether.UnlockTrash();

	g_Au.UnlockEtheriumDelete();

	out << deadTrash;
	out << dyingTrash;
	out << travelTrash;

	out << (quint8)trashByType.size();
	for(QHash<DaAtom::ATOM_TYPE, quint32>::const_iterator ai = trashByType.cbegin(); ai != trashByType.cend(); ai++)
	{
		out << (quint8)ai.key();
		out << ai.value();
	}

	if(atomId.IsNull())
	{
		const AuAtomTickerHash& ati = g_Au.LockAtomTickers();
		out << (quint16)ati.size();
		for(AuAtomTickerHash::const_iterator mti = ati.cbegin(); mti != ati.cend(); mti++)
		{
			out << (*mti).AtomId();
			if(!g_Au.m_ExcludeFromTick.contains((*mti).AtomId()))
			{
				out << (*mti).AverageProcessTime();	// This value is in 1/2 millisecond increments.
				out << (*mti).MaxProcessTime();		// This value is in 1/2 millisecond increments.
			}
			else
			{
				// Report 0's on atoms that are excluded from the tick cycle.
				unsigned int zero = 0;
				out << zero;
				out << zero;
			}
		}
		g_Au.UnlockAtomTickers();

		const AuAtomTockerHash& ato = g_Au.LockAtomTockers();
		out << (quint16)ato.size();
		for(AuAtomTockerHash::const_iterator mti = ato.cbegin(); mti != ato.cend(); mti++)
		{
			out << (*mti).AtomId();
			if(!g_Au.m_ExcludeFromTock.contains((*mti).AtomId()))
			{
				out << (*mti).AverageProcessTime();	// This value is in 1/2 millisecond increments.
				out << (*mti).MaxProcessTime();		// This value is in 1/2 millisecond increments.
			}
			else
			{
				// Report 0's on atoms that are excluded from the tock cycle.
				unsigned int zero = 0;
				out << zero;
				out << zero;
			}
		}
		g_Au.UnlockAtomTockers();
	}
	else
	{
		quint16 tickers = 0;
		const AuAtomTickerHash& ati = g_Au.LockAtomTickers();
		for(AuAtomTickerHash::const_iterator mti = ati.cbegin(); mti != ati.cend(); mti++)
			if((*mti).AtomId() == atomId || (*mti).SystemId() == atomId)
				tickers++;

		out << tickers;
		for(AuAtomTickerHash::const_iterator mti = ati.cbegin(); mti != ati.cend(); mti++)
		{
			if((*mti).AtomId() == atomId || (*mti).SystemId() == atomId)
			{
				out << (*mti).AtomId();
				if(!g_Au.m_ExcludeFromTick.contains((*mti).AtomId()))
				{
					out << (*mti).AverageProcessTime();	// This value is in 1/2 millisecond increments.
					out << (*mti).MaxProcessTime();		// This value is in 1/2 millisecond increments.
				}
				else
				{
					// Report 0's on atoms that are excluded from the tick cycle.
					unsigned int zero = 0;
					out << zero;
					out << zero;
				}
			}
		}
		g_Au.UnlockAtomTickers();

		quint16 tockers = 0;
		const AuAtomTockerHash& ato = g_Au.LockAtomTockers();
		for(AuAtomTockerHash::const_iterator mti = ato.cbegin(); mti != ato.cend(); mti++)
			if((*mti).AtomId() == atomId || (*mti).SystemId() == atomId)
				tockers++;

		out << tockers;
		for(AuAtomTockerHash::const_iterator mti = ato.cbegin(); mti != ato.cend(); mti++)
		{
			if((*mti).AtomId() == atomId || (*mti).SystemId() == atomId)
			{
				out << (*mti).AtomId();
				if(!g_Au.m_ExcludeFromTock.contains((*mti).AtomId()))
				{
					out << (*mti).AverageProcessTime();	// This value is in 1/2 millisecond increments.
					out << (*mti).MaxProcessTime();		// This value is in 1/2 millisecond increments.
				}
				else
				{
					// Report 0's on atoms that are excluded from the tock cycle.
					unsigned int zero = 0;
					out << zero;
					out << zero;
				}
			}
		}
		g_Au.UnlockAtomTockers();
	}
}

// Report resource usage into a data stream.
void AuSceneServerApp::ResourceReportModels(QDataStream& out)
{
	if(m_ModelCache)
	{
		out << (quint8)1;
		m_ModelCache->ResourceReport(out);
	}
	else
		out << (quint8)0;

	// Report sc spacecraft designs.
	const DgData& scd = g_Au.LockScDesignsForRead();
	out << (quint16)scd.m_ScDesigns->size();
	for(AuScDesignHash::const_iterator di = scd.m_ScDesigns->cbegin(); di != scd.m_ScDesigns->cend(); di++)
	{
		out << (*di).second.Id();
		out << (*di).second.Name();
		out << (quint8)(*di).second.State();
		out << (*di).second.Parts().CountTriangles();
	}
	g_Au.UnlockScDesigns();

	// Report bd building designs.
	const DgData& bdd = g_Au.LockBdDesignsForRead();
	out << (quint16)bdd.m_BdDesigns->size();
	for(AuBdDesignHash::const_iterator di = bdd.m_BdDesigns->cbegin(); di != bdd.m_BdDesigns->cend(); di++)
	{
		out << (*di).second.Id();
		out << (*di).second.Name();
		out << (quint8)(*di).second.State();
		out << (*di).second.Parts().CountTriangles();
	}
	g_Au.UnlockBdDesigns();
}

// Initiated by server manager via DtMgrChannel
void AuSceneServerApp::LimboRescue(AuId atomId)
{
	QByteArray atomData;
	HandoffAtomUp2(AuId(0), atomId, false, true, atomData, AuId(0));
}

// Initiated by server manager via DtMgrChannel
void AuSceneServerApp::RestoreEmpire(DgEmpire empire)
{
	if(!g_Au.IsSceneMaster())
	{
		qWarning() << "warning: AuSceneServerApp::RestoreEmpire" << empire.m_id << empire.Name() << "only the master can do this";
		return;
	}
	else if(!empire.m_id)
	{
		qWarning() << "warning: AuSceneServerApp::RestoreEmpire" << empire.m_id << empire.Name() << "invalid empire id";
		return;
	}
	qDebug() << "AuSceneServerApp::RestoreEmpire" << empire.m_id << empire.Name();

	// Update my local empire record.
	DgData& gd = g_Au.LockDataForWrite();
	gd.ReplaceEmpire(empire);					// Add/replace an empire record.
	g_Au.UnlockData();
	emit ChangeEmpireInDb(empire.m_id);			// Trigger db store which notifies the subordinates when done.
}

void AuSceneServerApp::Shutdown(QString warnMsg, QString haltMsg, qint16 shutdownSecs)
{
	if(shutdownSecs < 0)
	{
		// Take the second signal to mean we are not supposed to shut down.
		m_ShutdownSeconds = 0;
		m_ShutdownWarn = "";
		m_ShutdownHalt = "";

		// Notify the farm that the shutdown was halted.
		AuCommAnnouncement msg(haltMsg);
		if(g_Au.IsSceneMaster())
			ChatUp(msg, 0);
		else
			ChatDown(msg);
		if(g_Au.m_LogMessages & LOG_Log)
			g_Au.Log(haltMsg);
	}
	else
	{
		m_ShutdownSeconds = shutdownSecs + 1;
		m_ShutdownWarn = warnMsg;
		m_ShutdownHalt = haltMsg;

		if(!g_Au.IsSceneMaster())
		{
			// Master will go through timed shutdown sequence then shutdown.
			// Manager and workers shutdown immediately; they must notify the subordinates about the shutdown.
			AuCommAnnouncement msg(haltMsg);
			ChatDown(msg);
			if(g_Au.m_LogMessages & LOG_Log)
				g_Au.Log(haltMsg);

			// Shutdown now.
			Shutdown();
		}
	}
}

void AuSceneServerApp::PostJobOffer(AuJobOffer jobOffer)
{
	if(g_Au.IsSceneMaster())
	{
		// Scene master sends the job offer down to the entire server farm.
		PostJobOfferDown(jobOffer);
	}
	else if(m_tClientChannel)
	{
		// Scene manager and worker servers send this message up their chain-of-command to the master server.
		m_tClientChannel->SendJobOffer(jobOffer);
	}
}

void AuSceneServerApp::PostJobOfferDown(AuJobOffer jobOffer)
{
	if(g_Au.IsSceneWorker())
	{
		// Worker server posts the job offer to the local queue.
		// This turns out to be a flood of job offers. We will accept a nu job every 15 seconds. The rest are simply ignored.
		if(m_JobOfferLast.elapsed()/d_MSecPerSec > 15 || jobOffer.IsCritical())
		{
			// It's been long enough. We can accept a nu job now.
			AuJobOffers* jobOffers = g_Au.LockJobOffersForWrite();

			// Since we have the mutex locked for write, take this opportunity to discard job offers that have expired.
			AuTime now = AuTime::Now();
			AuJobOffers::iterator joi = jobOffers->begin();
			while(joi != jobOffers->end())
				if((*joi).IsExpired(now) || jobOffer.DoesReplace(*joi))
					joi = jobOffers->erase(joi);
				else
					joi++;

			jobOffers->append(jobOffer);
			while(jobOffers->count() > 1000)
				jobOffers->removeFirst();

			//qDebug() << "AuSceneServerApp::PostJobOfferDown" << jobOffer.FromName() << jobOffer.Title()
				//<< "jobs queue contains" << jobOffers->size() << "job offers";

			g_Au.UnlockJobOffers();

			// Restart the timer for accepting the next job offer.
			m_JobOfferLast.restart();
		}
	}
	else
	{
		// Broadcast the notification to all sub servers.
		emit BroadcastJobOffer(jobOffer);
	}
}

void AuSceneServerApp::BountyPosted(AuId posterId, QString posterName, AuId victimId, QString reason, double reward, int why)
{
	if(g_Au.IsSceneMaster())
	{
		// Scene master sends the job offer down to the entire server farm.
		BountyPostedDown(posterId, posterName, victimId, reason, reward, why);
	}
	else if(m_tClientChannel)
	{
		// Scene manager and worker servers send this message up their chain-of-command to the master server.
		m_tClientChannel->SendBountyPosted(posterId, posterName, victimId, reason, reward, why);
	}
}

void AuSceneServerApp::BountyPostedDown(AuId posterId, QString posterName, AuId victimId, QString reason, double reward, int why)
{
	if(g_Au.IsSceneWorker())
	{
		// Worker server puts the bounty on the atom.
		// Locate the victim atom.
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaAtom* victim = ether.FindNormalAtom(victimId);
		if(victim)
			victim->AddBounty(posterId, posterName, reason, reward, why);
		g_Au.UnlockEtheriumDelete();
	}
	else
	{
		// Broadcast the notification to all sub servers.
		emit BroadcastBountyPosted(posterId, posterName, victimId, reason, reward, why);
	}
}

void AuSceneServerApp::RenameAvatarUp(AuId avatarId, QString nuName)
{
	if(g_Au.IsSceneMaster())
	{
		// Empire record stores avatar names.
		DgData& gd = g_Au.LockDataForWrite();
		quint32 empireId = gd.SetEmpireAvatarName(avatarId, nuName);
		g_Au.UnlockData();
		if(empireId)
			emit ChangeEmpireInDb(empireId);					// Propagate a change notification to the rest of the server farm.

		// Servers will load changed empire record with nu avatar name.
		// Original worker server renamed the avatar atom in the database.
		// Avatar name change is at the end of the road here.
	}
	else if(m_tClientChannel)
	{
		// Queue a message for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_RenameAvatar;
		out << avatarId;
		out << nuName;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::RenameCityUp(AuId cityId, QString nuName)
{
	if(g_Au.IsSceneMaster())
	{
		// Empire record stores city names.
		DgData& gd = g_Au.LockDataForWrite();
		quint32 empireId = gd.SetEmpireCityName(cityId, nuName);
		g_Au.UnlockData();
		if(empireId)
			emit ChangeEmpireInDb(empireId);					// Propagate a change notification to the rest of the server farm.

		// Servers will load changed empire record with nu city name.
		// Original worker server renamed the city atom in the database.
		// City name change is at the end of the road here.
	}
	else if(m_tClientChannel)
	{
		// Queue a message for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_RenameCity;
		out << cityId;
		out << nuName;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::RenamePlanetUp(AuId planetId, QString nuName, AuId systemId, QString systemName, AuId sectorId,
		QString sectorName)
{
	if(g_Au.IsSceneMaster())
	{
		// Empire record stores planet names.
		// We only have to worry about one empire changing because the planet cannot be renamed unless only one empire owns all
		// the cities there.
		DgData& gd = g_Au.LockDataForWrite();
		quint32 empireId = gd.SetEmpirePlanetName(planetId, nuName);
		g_Au.UnlockData();
		if(empireId)
			emit ChangeEmpireInDb(empireId);					// Propagate a change notification to the rest of the server farm.

		// Servers will load changed empire record with nu planet name.
		// Original worker server renamed the planet atom in the database.
		// Planet name change is propagated to the farm to finish the process.
		RenamePlanetDown(planetId, nuName, systemId, systemName, sectorId, sectorName);
	}
	else if(m_tClientChannel)
	{
		// Queue a message for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_RenamePlanet;
		out << planetId;
		out << nuName;
		out << systemId;
		out << systemName;
		out << sectorId;
		out << sectorName;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::RenameSystemUp(AuId systemId, QString nuName)
{
	if(g_Au.IsSceneMaster())
	{
		// Empire record stores system names.
		// We only have to worry about one empire changing because the system cannot be renamed unless only one empire owns all
		// the cities there.
		DgData& gd = g_Au.LockDataForWrite();
		quint32 empireId = gd.SetEmpireSystemName(systemId, nuName);
		g_Au.UnlockData();
		if(empireId)
		{
			emit ChangeEmpireInDb(empireId);					// Propagate a change notification to the rest of the server farm.
		}

		// Servers will load changed empire record with nu system name.
		// Original worker server renamed the system atom in the database.
		// System name change is propagated to the farm to finish the process.
		RenameSystemDown(systemId, nuName);
	}
	else if(m_tClientChannel)
	{
		// Queue a message for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_RenameSystem;
		out << systemId;
		out << nuName;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::RenameSectorUp(AuId sectorId, QString nuName)
{
	if(g_Au.IsSceneMaster())
	{
		// Empire record stores sector names.
		// Multiple empire records could change as a result.
		QList<quint32> empireIds;
		DgData& gd = g_Au.LockDataForWrite();
		gd.SetEmpireSectorName(sectorId, nuName, empireIds);
		g_Au.UnlockData();
		for(QList<quint32>::const_iterator ei = empireIds.cbegin(); ei != empireIds.cend(); ei++)
		{
			emit ChangeEmpireInDb(*ei);							// Propagate a change notification to the rest of the server farm.
		}

		// Servers will load changed empire record(s) with nu sector name.
		// Original manager server renamed the sector atom in the database as the message propagated up to me.
		// Sector name change is propagated to the farm to finish the process.
		RenameSectorDown(sectorId, nuName);
	}
	else if(m_tClientChannel)
	{
		if(g_Au.IsSceneManager())
		{
			// Original scene manager is responsible for changing the sector atom in the database.
			AuEther& ether = g_Au.LockEtheriumDelete();
			DaAtom* sector = ether.FindNormalAtom(sectorId);
			if(sector)
			{
				sector->SetName(nuName);
				sector->SetModified();
			}
			else
				qWarning() << "AuSceneServerApp::RenameSectorUp sector not found" << sectorId;
			g_Au.UnlockEtheriumDelete();
		}

		// Queue a message for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_RenameSector;
		out << sectorId;
		out << nuName;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::RenamePlanetDown(AuId planetId, QString nuName, AuId systemId, QString systemName, AuId sectorId,
		QString sectorName)
{
	// Locate the planet in my local data.
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaWorld* planet = dynamic_cast<DaWorld*>(ether.FindNormalAtom(planetId));
	BODY_TYPE bodyType = BODY_TYPE_Count;
	if(planet)
	{
		planet->SetName(nuName);
		bodyType = planet->BodyType();
	}
	if(g_Au.IsSceneWorker())
	{
		// Locate npc(s) from that planet.
		DaAtomList npcs;
		ether.FindNpcsFromWorld(planetId, npcs);
		for(DaAtomList::const_iterator oi = npcs.begin(); oi != npcs.end(); oi++)
		{
			DaNpc* npc = (DaNpc*)*oi;
			npc->SetHomeWorld(planetId, nuName, bodyType, systemId, systemName, sectorId, sectorName, npc->HomeGalaxy());
			npc->SetModified();
		}
	}
	g_Au.UnlockEtheriumDelete();

	// Propagate the message down to my subordinate servers.
	if(!g_Au.IsSceneWorker())
		emit RenamePlanet(planetId, nuName, systemId, systemName, sectorId, sectorName);
}

void AuSceneServerApp::RenameSectorDown(AuId sectorId, QString nuName)
{
	// Locate the sector in my local data.
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSector* sector = dynamic_cast<DaSector*>(ether.FindNormalAtom(sectorId));
	if(sector)
		sector->SetName(nuName);
	if(g_Au.IsSceneWorker())
	{
		// Locate officer(s) from that sector.
		DaAtomList npcs;
		ether.FindNpcsFromSector(sectorId, npcs);
		for(DaAtomList::const_iterator oi = npcs.begin(); oi != npcs.end(); oi++)
		{
			DaNpc* npc = (DaNpc*)*oi;
			if(npc->SetHomeSectorName(nuName))
				npc->SetModified();
		}
	}
	g_Au.UnlockEtheriumDelete();

	// Propagate the message down to my subordinate servers.
	if(!g_Au.IsSceneWorker())
		emit RenameSector(sectorId, nuName);
}

void AuSceneServerApp::RenameSystemDown(AuId systemId, QString nuName)
{
	// Locate the solar system in my local data.
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(systemId));
	if(system)
		system->SetNameIncludePlanets(nuName, false);		// Db not notified, was done when rename started in DtChatChannel.

	DaAtomList people;
	ether.FindPeopleFromSystem(systemId, people);
	for(DaAtomList::const_iterator pi = people.begin(); pi != people.end(); pi++)
	{
		DaPerson* p = (DaPerson*)*pi;
		if(p->SetSpawnCitySystemName(nuName))
			p->SetModified();
	}
	g_Au.UnlockEtheriumDelete();

	// Propagate the message down to my subordinate servers.
	if(!g_Au.IsSceneWorker())
		emit RenameSystem(systemId, nuName);
}

void AuSceneServerApp::AvatarOnLineUp(AuId personId, quint32 empireId)
{
	if(g_Au.IsSceneMaster())
	{
		// Master echoes this back down to the farm.
		AvatarOnLineDown(personId, empireId);
	}
	else if(m_tClientChannel)
	{
		// Sub servers send this message up to their boss.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_AvatarOnLine;
		out << personId;
		out << empireId;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::AvatarOnLineDown(AuId personId, quint32 empireId)
{
	// Every server gets this message so they all update their own empire records.
	if(empireId)
	{
		// Empire stores the change time in whole minutes so this doesn't trigger a change too often.
		DgData& gd = g_Au.LockDataForWrite();
		bool chg = gd.EmpireCitizenOnLine(empireId);	// Empire doesn't care which citizen was on line.
		g_Au.UnlockData();
		if(chg && g_Au.IsSceneMaster())
			emit ChangeEmpireInDb(empireId);	// Trigger db store which notifies the subordinates when done.
	}

	if(g_Au.IsSceneWorker())
	{
		// Look for the capitol.
		AuEther& ether = g_Au.LockEtheriumDelete();
		if(empireId)
		{
			// Capital could be a DaBd.
			AuId empireCapitol;
			const DgData& gd = g_Au.LockDataForRead();
			gd.EmpireCapital(empireId, empireCapitol);
			g_Au.UnlockData();
			if(!empireCapitol.IsNull())
			{
				DaAtom* capitol = (DaAtom*)ether.FindNormalAtom(empireCapitol);	// Cast away constness.
				if(capitol)
				{
					DaSolarSystem* system = capitol->FindParentSolarSystem();
					if(system)
						system->SetLastPersonVisited();
				}
			}
		}
		g_Au.UnlockEtheriumDelete();
	}
	else
	{
		// Echo this down to the farm.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_AvatarOnLine;
		out << personId;
		out << empireId;
		const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
		for(unordered_map<unsigned int, DtServerChannel*>::const_iterator it = servers.cbegin(); it != servers.cend(); it++)
			if((*it).second->isRunning())
				(*it).second->SendPacket(ba);
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
	}
}

// Slot version must lock the spawns mutex.
void AuSceneServerApp::SpawnPersonOnPlanet(AuId person, AuId sectorOrSystemId, quint8 galaxy)
{
	m_SpawnMutex.lock();
	SpawnPersonOnPlanet2(person, sectorOrSystemId, (GALAXY)galaxy);
	m_SpawnMutex.unlock();
}

// NOTE: m_SpawnMutex must be locked before calling this function.
void AuSceneServerApp::SpawnPersonOnPlanet2(const AuId& person, const AuId& sectorOrSystemId, GALAXY galaxy)
{
	// A person needs to spawn at a nu empty planet suitable for a homeworld.
	if(g_Au.IsSceneMaster())
	{
		// This process starts at the master server, with the creation of a nu sector.
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaUniverse* au = ether.FindUniverse();
		if(au)
		{
			// 'galaxy' tells us which galaxy contained the person when they respawned.
			galaxy = ShoresOfHazeron;

			DaGalaxy* ag = au->FindGalaxy(galaxy);
			if(ag)
			{
				// Create a nu unexplored 3x3x3 cloud of sectors.
				AuIdList& nuSectors = LockNewSectors();
				DaSector* asect = ag->CreateNewHomeWorldSector(8, nuSectors);
				UnlockNewSectors();
				if(asect)
				{
					// The nu sector must be assigned to a manager before we can continue.
					AuId sectorId = asect->Id();
					if(IsSectorAssigned(sectorId))
					{
						// Sector is assigned so we can continue right away.
						SpawnPersonOnPlanet3(person, sectorId, galaxy);
						g_Au.UnlockEtheriumDelete();
						return;
					}
					else
					{
						// Sector is not assigned so we must wait for it.
						g_AuDb.PostCriticalUpdates();
						// Guarded by m_SpawnMutex.
						m_DepartingSpawnsOnPlanets.insert(pair<AuId, AuSpawnDeparture>(person, AuSpawnDeparture(sectorId, galaxy)));
					}
				}
			}
			else
			{
				qWarning() << "error: AuSceneServerApp::SpawnPersonOnPlanet2 galaxy" << galaxy << "not found";
			}
		}
		g_Au.UnlockEtheriumDelete();
	}
	else if(g_Au.IsSceneManager())
	{
		// This process progresses to the manager server, with the creation of a nu solar system.
		// Locate the sector. It should have been assigned to me already.
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaSector* asect = dynamic_cast<DaSector*>(ether.FindNormalAtom(sectorOrSystemId));
		if(asect)
		{
			// Claim a system in the sector. This fetch guarantees we receive a solar system that contains a habitable planet.
			DaSolarSystem* system = asect->FindGoodHome();
			if(system)
			{
				// Locate a homeworld within the system.
				system->FinalizeContents();
				Q_ASSERT(system->FindHabitableWorld(HABITABLE_HomeWorld));
				// The nu solar system must be assigned to a worker before we can continue.
				AuId systemId = system->Id();
				if(IsSolarSystemAssigned(systemId))
				{
					// Solar system is assigned so we can continue right away.
					SpawnPersonOnPlanet3(person, systemId, asect->Galaxy());
					g_Au.UnlockEtheriumDelete();
					return;
				}
				else
				{
					// Solar system is not assigned so we must wait for it.
					g_AuDb.PostCriticalUpdates();

					// Guarded by m_SpawnMutex.
					m_DepartingSpawnsOnPlanets.insert(pair<AuId, AuSpawnDeparture>(person, AuSpawnDeparture(systemId,
									asect->Galaxy())));
				}
			}
			else
			{
				if(asect->HasSubatoms())
					qWarning() << "ERROR! AuSceneServerApp::SpawnPersonOnPlanet2 sector" << asect->Id() << asect->Name()
						<< "FindGoodHome failed." << asect->SubatomCount() << "atoms in sector. person" << person
						<< "sectorOrSystemId" << sectorOrSystemId;
				else
					qWarning() << "ERROR! AuSceneServerApp::SpawnPersonOnPlanet2 sector" << asect->Id() << asect->Name()
						<< "FindGoodHome failed." << "NO atoms in sector. person" << person
						<< "sectorOrSystemId" << sectorOrSystemId;
			}
		}
		else
		{
			// Sector is not loaded yet. Must wait for it to be loaded.
			m_ArrivingSpawnsOnPlanets.insert(pair<AuId, AuSpawnArrival>(person, AuSpawnArrival(sectorOrSystemId, galaxy)));
		}
		g_Au.UnlockEtheriumDelete();
	}
	else
	{
		// This process completes at the worker server where the database thread is instructed to spawn the person on the planet.
		// Locate the spawn point atom.
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(sectorOrSystemId));
		if(system && !system->IsOnlyStarFieldLoaded())
		{
			const DaSector* sector = system->FindParentSector();
			Q_ASSERT(sector);
			if(sector)
			{
				// At this point, the atom can be treated like an arriving traveler.
				// The DaAtom::LoadSubatoms code triggers the necessary actions when a spawning atom is loaded.
				emit TravelerArrivedViaDb(person, system->Id(), sector->Id());
			}
		}
		else
		{
			// Solar system is not loaded yet. Must wait for it to be loaded.
			m_ArrivingSpawnsOnPlanets.insert(pair<AuId, AuSpawnArrival>(person, AuSpawnArrival(sectorOrSystemId, galaxy)));
		}
		g_Au.UnlockEtheriumDelete();
	}
}

void AuSceneServerApp::SpawnPersonOnPlanet3(const AuId& person, const AuId& sectorOrSystemId, GALAXY galaxy)
{
	//Q_ASSERT(!g_Au.IsSceneWorker());
	if(g_Au.IsSceneMaster())
	{
		// Notify the manager of the sector 'sectorOrSystemId' to create a nu homeworld.
		DhSceneServer toServer = SectorServer(sectorOrSystemId);
		if(toServer.IsValid())
		{
			// Notify the responsible server that an atom is destined for one of its solar systems.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_SpawnPersonOnPlanet;
			out << person;
			out << sectorOrSystemId;
			out << galaxy;
			SendPacketTo(ba, toServer.m_Id);
		}
	}
	else if(g_Au.IsSceneManager())
	{
		// Notify the worker assigned the solar system to spawn the person.
		DhSceneServer toServer = SolarSystemServer(sectorOrSystemId);
		if(toServer.IsValid())
		{
			// Notify the responsible server that an atom is destined for one of its solar systems.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_SpawnPersonOnPlanet;
			out << person;
			out << sectorOrSystemId;
			out << galaxy;
			SendPacketTo(ba, toServer.m_Id);
		}
	}
}

void AuSceneServerApp::AssignedSectorLoaded(AuId sectorId)
{
	// Check for arriving spawns who are waiting for the sector to be loaded to proceed with creating a habitable world.
	m_SpawnMutex.lock();
	unordered_map<AuId, AuSpawnArrival, AuIdHash>::iterator asi = m_ArrivingSpawnsOnPlanets.begin();
	while(asi != m_ArrivingSpawnsOnPlanets.end())
	{
		if((*asi).second.m_SectorOrSystemId == sectorId)
		{
			// A spawning atom is waiting for the sector to be loaded to proceed with creating a habitable world.
			SpawnPersonOnPlanet2((*asi).first, (*asi).second.m_SectorOrSystemId, (*asi).second.Galaxy());
			asi = m_ArrivingSpawnsOnPlanets.erase(asi);
		}
		else
			asi++;
	}
	m_SpawnMutex.unlock();
}

void AuSceneServerApp::AssignedSolarSystemLoaded(AuId systemId)
{
	// Check for arriving spawns who are waiting for the solar system to be loaded to proceed with creating a habitable world.
	m_SpawnMutex.lock();
	unordered_map<AuId, AuSpawnArrival, AuIdHash>::iterator asi = m_ArrivingSpawnsOnPlanets.begin();
	while(asi != m_ArrivingSpawnsOnPlanets.end())
	{
		if((*asi).second.m_SectorOrSystemId == systemId)
		{
			// A spawning atom is waiting for the solar system to be loaded to proceed with creating a habitable world.
			SpawnPersonOnPlanet2((*asi).first, (*asi).second.m_SectorOrSystemId, (*asi).second.Galaxy());
			asi = m_ArrivingSpawnsOnPlanets.erase(asi);
		}
		else
			asi++;
	}
	m_SpawnMutex.unlock();
}

// This is the receiver of the signal emitted by solar systems.
void AuSceneServerApp::SystemWarpSignature(AuId systemId, quint32 warpFactors, quint8 galaxy, float galaxyLocX, float galaxyLocY,
		float galaxyLocZ)
{
	if(g_Au.IsSceneMaster())
	{
		// Echo the message down to the entire server farm.
		SystemWarpSignatureBroadcast(systemId, warpFactors, galaxy, galaxyLocX, galaxyLocY, galaxyLocZ);
	}
	else if(m_tClientChannel)
	{
		// Propagate the notification up to the master server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_SystemWarpSignature;
		out << systemId;
		out << warpFactors;
		out << galaxy;
		out << galaxyLocX;
		out << galaxyLocY;
		out << galaxyLocZ;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::SystemWarpSignatureBroadcast(AuId systemId, quint32 warpFactors, quint8 galaxy, float galaxyLocX,
		float galaxyLocY, float galaxyLocZ)
{
	if(g_Au.IsSceneWorker())
	{
		// Worker processes the notification by sending it to every local city.
		// Buildings and spacecraft that detect the emission will in turn emit a WarpSignatureDetected signal.
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaUniverse* universe = ether.FindUniverse();
		if(universe)
		{
			DaGalaxy* pg = universe->FindGalaxy((GALAXY)galaxy);
			if(pg)
				pg->DetectWarpSignatureRecursive(systemId, warpFactors, AuPoint(galaxyLocX, galaxyLocY, galaxyLocZ));
		}
		g_Au.UnlockEtheriumDelete();
	}
	else
	{
		// Echo the message down to the entire server farm.
		emit SystemWarpSignatureDown(systemId, warpFactors, galaxy, galaxyLocX, galaxyLocY, galaxyLocZ);
	}
}

// This is the receiver of the signal emitted by cities and buildings when they receive a power emission from a solar system.
void AuSceneServerApp::WarpSignatureDetected(AuId systemId, quint32 signalStrength, quint32 detectingEmpireId)
{
	if(g_Au.IsSceneMaster())
	{
		// Master processes this information so it can provide it in star map data. It is not needed by workers and we don't store
		// the information in the database so there is no need to notify the farm.
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(systemId));
		if(system)
			system->DetectedBy(detectingEmpireId, signalStrength);
		g_Au.UnlockEtheriumDelete();
	}
	else if(m_tClientChannel)
	{
		// Propagate the notification up to the master server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_WarpSignatureDetected;
		out << systemId;
		out << signalStrength;
		out << detectingEmpireId;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::SetAtomModified(AuId atomId, bool recursive)
{
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaAtom* atom = ether.FindNormalAtom(atomId);
	if(atom)
	{   
		atom->SetModified();

		if(recursive)
		{
			DaAtomList subAtoms;
			atom->ListNormalSubatomsRecursive(subAtoms);
			for(DaAtomList::const_iterator ai = subAtoms.begin(); ai != subAtoms.end(); ai++)
				(*ai)->SetModified();
		}
	}   
	g_Au.UnlockEtheriumDelete();
}

void AuSceneServerApp::PersonDeletedDown(AuId personId)
{
	if(g_Au.IsSceneMaster())
	{
		// Remove the person from all empire records, in case they somehow ended up in more than one.
		QList<quint32> empiresChanged;
		DgData& gd = g_Au.LockDataForWrite();
		for(DgEmpireHash::iterator ei = gd.m_gEmpires.begin(); ei != gd.m_gEmpires.end(); ei++)
			if((*ei).second.RemoveCitizen(personId))
				empiresChanged.append((*ei).second.m_id);
		g_Au.UnlockData();

		// Notify the server farm of any empire records that changed.
		for(QList<quint32>::const_iterator ci = empiresChanged.cbegin(); ci != empiresChanged.cend(); ci++)
			emit ChangeEmpireInDb(*ci);

		// Propagate the notification down to my manager(s).
		emit PersonDeleted(personId);
	}
	else if(g_Au.IsSceneManager())
	{
		// Propagate the notification down to my workers.
		emit PersonDeleted(personId);
	}
	else if(g_Au.IsSceneWorker())
	{
		// Remove the person from all spacecraft berths.
		DaAtomList spacecraft;
		AuEther& ether = g_Au.LockEtheriumDelete();
		ether.ListAtoms(spacecraft);
		if(!spacecraft.isEmpty())
		{
			for(DaAtomList::const_iterator it = spacecraft.begin(); it != spacecraft.end(); it++)
			{
				DaVessel* ship = dynamic_cast<DaVessel*>(*it);
				if(!ship)
					continue;
				ship->RemoveBerthOccupant(personId);
				ship->KillDormantAbandoned();					// Dormant ship could be deleted when a person leaves the roster.
			}
		}
		g_Au.UnlockEtheriumDelete();
	}
}

// Initiated by server manager to create a sector cube at a specific location.
void AuSceneServerApp::CreateSectorAt(quint8 galaxyEnum, double sectorX, double sectorY, double sectorZ)
{
	// Only the scene master does this.
	if(!g_Au.IsSceneMaster())
		return;

	AuPoint sectorLocPc(sectorX, sectorY, sectorZ);
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaUniverse* universe = ether.FindUniverse();
	if(universe)
	{
		DaGalaxy* galaxy = universe->FindGalaxy((GALAXY)galaxyEnum);
		if(galaxy)
		{
			// Generate the sector cube at the destination sector's location. This will cause it to get
			// assigned to a scene manager.
			m_rwNewSectors.lockForWrite();
			DaSector* sector = galaxy->CreateSectorCubeAt(sectorLocPc, m_NewSectors);
			m_rwNewSectors.unlock();
			g_AuDb.PostCriticalUpdates();
			if(sector)
			{
				// Dump contents of the nu sector for the agent that requested it, presuming they have access to the log.
				qDebug() << "AuSceneServerApp::CreateSectorAt sector" << sector->Id() << sector->Name() << sector->Description();
				DaAtomList subs;
				sector->ListSubatoms(subs);
				for(DaAtomList::const_iterator si = subs.cbegin(); si != subs.cend(); si++)
					qDebug() << " " << (*si)->Id() << (*si)->Name() << (*si)->Description();
			}
			else
				qDebug() << "AuSceneServerApp::CreateSectorAt" << sectorLocPc << "FAILED";
		}
	}
	g_Au.UnlockEtheriumDelete();
}

void AuSceneServerApp::CityMailRequested(AuId cityId, AuId unitId, quint32 unitEmpire)
{
	if(g_Au.IsSceneMaster())
	{
		// The scene master sends the notification down the chain so all workers receive the message.
		CityMailRequestedDown(cityId, unitId, unitEmpire);
	}
	else if(m_tClientChannel)
	{
		// Workers and Managers propagate the message up the server chain to the master.
		// Queue a msg for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_CityMailRequested;
		out << cityId;
		out << unitId;
		out << unitEmpire;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::CityMailRequestedDown(AuId cityId, AuId unitId, quint32 unitEmpire)
{
	if(g_Au.IsSceneWorker())
	{
		AuEther& ether = g_Au.LockEtheriumDelete();
		const DaAtom* atom = ether.FindNormalAtom(cityId);
		const DaBd* bd = dynamic_cast<const DaBd*>(atom);
		if(bd && bd->EmpireId() == unitEmpire)
			bd->MailLastReport(unitId);
		g_Au.UnlockEtheriumDelete();
	}
	else
	{
		// Boss servers broadcast the request to all sub servers.
		emit CityMailRequestedSignal(cityId, unitId, unitEmpire);
	}
}

void AuSceneServerApp::MgrStatusRequest(quint8 specificStatus)
{
	// Reply with the status packet.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtMgrChannelPacket::P_Status;
	out << specificStatus;

	// Fill the status packet with data.
	switch(specificStatus)
	{
	case 0:
	default:
		// Default is an empty status packet.
		break;
	case 1:
		break;
	case 2:
		// report load values
		StatusReportLoads(out);
		break;
	case 3:
		// report connected users
		StatusReportUsers(out);
		break;
	}

	emit MgrSendPacket(ba);
}

void AuSceneServerApp::MgrMemoryUsageRequest()
{
	QStringList mem = ReportMemoryUsage(false);

	// Reply with the status packet.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtMgrChannelPacket::P_MemoryUsage;
	out << mem;
	emit MgrSendPacket(ba);
}

void AuSceneServerApp::MgrResourceRequest(quint8 specificResources, AuId atomId)
{
	// Reply with the status packet.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtMgrChannelPacket::P_Resources;
	out << specificResources;

	// Fill the status packet with data.
	switch(specificResources)
	{
	case 0:
	default:
		// Default is an empty resource packet.
		break;
	case 1:
		// Report all resources.
		ResourceReport(out);
		break;
	case 2:
		// Report atom resources, including subatoms.
		ResourceReport(out, atomId);
		break;
	}

	emit MgrSendPacket(ba);
}

// This is where the beginning of the trail begins for opening a stargate connection to a distant galaxy/sector/solar system.
void AuSceneServerApp::StargateOpened(AuId fromGalaxy, AuPoint fromGalaxyLoc, AuId toGalaxy, AuPoint toGalaxyLoc, AuTime expire)
{
	//qDebug() << "AuSceneServerApp::StargateOpened from" << fromGalaxy << fromGalaxyLoc << "to" << toGalaxy << toGalaxyLoc;
	if(g_Au.IsSceneMaster())
	{
		OpenStargateDown(fromGalaxy, fromGalaxyLoc, toGalaxy, toGalaxyLoc, AuId(0), AuId(0), expire);
	}
	else if(m_tClientChannel)
	{
		// Workers and Managers propagate the message up the server chain to the master.
		// Queue a msg for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_StargateOpened;
		out << fromGalaxy;
		out << fromGalaxyLoc;
		out << toGalaxy;
		out << toGalaxyLoc;
		expire >> out;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::OpenStargateDown(AuId fromGalaxy, AuPoint fromGalaxyLoc, AuId toGalaxy, AuPoint toGalaxyLoc,
		AuId toSector, AuId toSystem, AuTime expire)
{
	//qDebug() << "AuSceneServerApp::OpenStargateDown" << fromGalaxy << fromGalaxyLoc << toGalaxy << toGalaxyLoc << toSector
		//<< toSystem << "expire" << expire;
	if(g_Au.IsSceneMaster())
	{
		// Master server must insure the sector exists and has been assigned to a manager.
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaGalaxy* galaxy = dynamic_cast<DaGalaxy*>(ether.FindNormalAtom(toGalaxy));
		if(galaxy)
		{
			AuPoint sectorLoc(toGalaxyLoc);
			sectorLoc.Round(10);			// Round to an even sector location.

			m_rwNewSectors.lockForWrite();
			DaSector* sector = galaxy->CreateSectorCubeAt(sectorLoc, m_NewSectors);
			m_rwNewSectors.unlock();
			g_AuDb.PostCriticalUpdates();

			if(IsSectorAssigned(sector->Id()))
			{
				//qDebug() << "AuSceneServer::OpenStargateDown sector assigned" << sector->Id() << "signaling OpenStargateDown";
				emit OpenStargateDownSignal(fromGalaxy, fromGalaxyLoc, toGalaxy, toGalaxyLoc, sector->Id(), AuId(0), expire);
			}
			else if(expire > AuTime::Now())
			{
				// Must wait until the sector is assigned. Queue emptied in OpenDelayedStargates.
				m_muPendingStargates.lock();
				m_PendingStargates.insert(pair<AuId, AuOpenStargate>(sector->Id(), AuOpenStargate(fromGalaxy, fromGalaxyLoc,
								toGalaxy, toGalaxyLoc, sector->Id(), AuId(0), expire)));
				m_muPendingStargates.unlock();
			}
		}
		g_Au.UnlockEtheriumDelete();
	}
	else if(g_Au.IsSceneManager())
	{
		// Manager server must insure the solar system exists and has been assigned to a worker.
		if(IsSectorAssignedToMe(toSector))
		{
			// The destination sector is assigned to me. The destination system must be in my sphere of control. The scene
			// manager only loads sectors that are assigned to it. All the solar systems in the sector are guaranteed to
			// exist but I am responsible for my systems' contents.
			AuEther& ether = g_Au.LockEtheriumDelete();
			bool foundDest = false;
			DaGalaxy* galaxy = dynamic_cast<DaGalaxy*>(ether.FindNormalAtom(toGalaxy));
			if(galaxy)
			{
				DaSector* sector = dynamic_cast<DaSector*>(galaxy->FindSubatomId(toSector));
				if(sector)
				{
					AuPoint systemLoc = toGalaxyLoc - sector->Location();
					DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(sector->FindSolarSystemAt(systemLoc));
					if(!system)
						system = dynamic_cast<DaSolarSystem*>(sector->FindClosestSolarSystem(systemLoc));
					if(system)
					{
						foundDest = true;
						if(IsSolarSystemAssigned(system->Id()))
						{
							//qDebug() << "AuSceneServer::OpenStargateDown system assigned" << system->Id()
								//<< "signaling OpenStargateDown";
							emit OpenStargateDownSignal(fromGalaxy, fromGalaxyLoc, toGalaxy, toGalaxyLoc, toSector, system->Id(),
									expire);
						}
						else
						{
							// Finalize the solar system contents. This will cause it to be assigned to a worker server who will
							// load all the data. Does nothing if the system is already finalized.
							//qDebug() << "AuSceneServer::OpenStargateDown finalizing system" << system->Id() << system->Name();
							system->FinalizeContents();
							g_AuDb.PostCriticalUpdates();

							// Must wait until the system is assigned.
							if(expire > AuTime::Now())
							{
								m_muPendingStargates.lock();
								m_PendingStargates.insert(pair<AuId, AuOpenStargate>(system->Id(), AuOpenStargate(fromGalaxy,
												fromGalaxyLoc, toGalaxy, toGalaxyLoc, toSector, system->Id(), expire)));
								m_muPendingStargates.unlock();
							}
						}
					}
				}
			}
			g_Au.UnlockEtheriumDelete();
			
			if(!foundDest)
			{
				// Sometimes the manager (me) receives the stargate notification so fast I have barely been assigned the sector
				// and DtDb has not had time to read the database yet.
				// I know the sector is assigned to me; queue it in a way that I can watch for it in OpenDelayedStargates.
				//qDebug() << "AuSceneServer::OpenStargateDown queuing null id stargate open request" << toSector;
				m_muPendingStargates.lock();
				m_PendingStargates.insert(pair<AuId, AuOpenStargate>(AuId(0), AuOpenStargate(fromGalaxy, fromGalaxyLoc, toGalaxy,
								toGalaxyLoc, toSector, toSystem, expire)));
				m_muPendingStargates.unlock();
			}
		}
		//else
			//qDebug() << "AuSceneServer::OpenStargateDown sector not assigned to me" << toSector;
	}
	else
	{
		// Worker server must notify the destination solar system to create the negative stargate.
		// If, like the manager, the worker (me) receives the stargate notification so fast I have barely been assigned the system
		// and DtDb has not had time to read the database yet, it's not a big deal. The ship emits a StargateOpened notification
		// when it enters the stargate; that will cause the final exit stargate wormhole to be created.
		if(IsSolarSystemAssignedToMe(toSystem))
		{
			bool foundDest = false;
			AuId idSystem(toSystem);
			AuEther& ether = g_Au.LockEtheriumDelete();
			DaSolarSystem* system = AssignedSolarSystem(idSystem, ether);
			if(system)
			{
				// This is used to open the destination negative stargate.
				// We create destination stargates at the primary star.
				DaStar* star = system->FindPrimaryStarNotBlackHole();
				if(star)
				{
					foundDest = true;
					star->OpenStargate(fromGalaxy, fromGalaxyLoc, false, expire);
				}
			}
			g_Au.UnlockEtheriumDelete();

			if(!foundDest)
			{
				// Queue the request in case the solar system gets assigned to me.
				// Sometimes the worker (me) receives the stargate notification so fast I have barely been assigned the system
				// and DtDb has not had time to read the database yet.
				// I know the system is assigned to me; queue it so I can watch for it in OpenDelayedStargates.
				m_muPendingStargates.lock();
				m_PendingStargates.insert(pair<AuId, AuOpenStargate>(toSystem, AuOpenStargate(fromGalaxy, fromGalaxyLoc, toGalaxy,
								toGalaxyLoc, toSector, toSystem, expire)));
				m_muPendingStargates.unlock();
			}
		}
	}
}

void AuSceneServerApp::OpenDelayedStargates()
{
	if(g_Au.IsSceneMaster())
	{
		// Notify any subordinates of stargates waiting to open.
		m_muPendingStargates.lock();
		unordered_multimap<AuId, AuOpenStargate, AuIdHash>::iterator gi = m_PendingStargates.begin();	// Key is sector id awaiting notification.
		while(gi != m_PendingStargates.end())
		{
			if(IsSectorAssigned((*gi).first))
			{
				//qDebug() << "AuSceneServer::OpenDelayedStargates sector assigned" << (*gi).first << "signaling OpenStargateDown";
				emit OpenStargateDownSignal((*gi).second.m_FromGalaxy, (*gi).second.m_FromGalaxyLoc, (*gi).second.m_ToGalaxy, (*gi).second.m_ToGalaxyLoc,
						(*gi).second.m_ToSector, (*gi).second.m_ToSystem, (*gi).second.m_Expire);
				gi = m_PendingStargates.erase(gi);
			}
			else if(AuTime::Now() > (*gi).second.m_Expire)
				gi = m_PendingStargates.erase(gi);	// Throw them away when it's too late.
			else
				gi++;
		}
		m_muPendingStargates.unlock();
	}
	else if(g_Au.IsSceneManager())
	{
		// Notify any subordinates of stargates waiting to open.
		// Quick check before locking root.
		m_muPendingStargates.lock();
		bool anyPending = !m_PendingStargates.empty();
		m_muPendingStargates.unlock();
		if(!anyPending)
			return;

		// Sometimes the manager (me) receives the stargate notification so fast I have barely been assigned the sector
		// and DtDb has not had time to read the database yet.
		// This must be done with root locked so we can find unresolved destination solar systems.
		m_muPendingStargates.lock();
		unordered_multimap<AuId, AuOpenStargate, AuIdHash>::iterator gi = m_PendingStargates.begin();	// Key is system id awaiting notification.
		while(gi != m_PendingStargates.end())
		{
			if(AuTime::Now() > (*gi).second.m_Expire)
				gi = m_PendingStargates.erase(gi);	// Throw them away when it's too late.
			else if((*gi).first.IsNull())
			{
				// The solar system id has not been resolved. Do it now if we can.
				if(IsSectorAssignedToMe((*gi).second.m_ToSector))
				{
					AuEther& ether = g_Au.LockEtheriumDelete();
					const DaGalaxy* galaxy = dynamic_cast<const DaGalaxy*>(ether.FindNormalAtom((*gi).second.m_ToGalaxy));
					Q_ASSERT(galaxy);
					const DaSector* sector = dynamic_cast<const DaSector*>(galaxy->FindSubatomId((*gi).second.m_ToSector));
					if(sector)
					{
						AuPoint systemLoc = (*gi).second.m_ToGalaxyLoc - sector->Location();
						DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(sector->FindSolarSystemAt(systemLoc));
						if(!system)
							system = dynamic_cast<DaSolarSystem*>(sector->FindClosestSolarSystem(systemLoc));
						if(system)
						{
							if(IsSolarSystemAssigned(system->Id()))
							{
								//qDebug() << "AuSceneServer::OpenDelayedStargates system assigned" << system->Id()
									//<< "signaling OpenStargateDown";
								emit OpenStargateDownSignal((*gi).second.m_FromGalaxy, (*gi).second.m_FromGalaxyLoc,
										(*gi).second.m_ToGalaxy, (*gi).second.m_ToGalaxyLoc, (*gi).second.m_ToSector, system->Id(),
										(*gi).second.m_Expire);

								gi = m_PendingStargates.erase(gi);
								g_Au.UnlockEtheriumDelete();
								continue;	// skip the loop iterator.
							}
							else
							{
								// Finalize the solar system contents. This will cause it to be assigned to a worker server who will
								// load all the data. Does nothing if the system is already finalized.
								//qDebug() << "AuSceneServer::OpenDelayedStargates finalizing system" << system->Id()
									//<< system->Name();
								system->FinalizeContents();
								g_AuDb.PostCriticalUpdates();
							}
						}
					}
					g_Au.UnlockEtheriumDelete();
				}

				gi++;
			}
			else
			{
				if(IsSolarSystemAssigned((*gi).first))
				{
					//qDebug() << "AuSceneServer::OpenDelayedStargates system assigned" << (*gi).first << "signaling OpenStargateDown";
					emit OpenStargateDownSignal((*gi).second.m_FromGalaxy, (*gi).second.m_FromGalaxyLoc, (*gi).second.m_ToGalaxy, (*gi).second.m_ToGalaxyLoc,
							(*gi).second.m_ToSector, (*gi).second.m_ToSystem, (*gi).second.m_Expire);
					gi = m_PendingStargates.erase(gi);
				}
				else
					gi++;
			}
		}
		m_muPendingStargates.unlock();
	}
	else
	{
		// Quick check before locking root.
		m_muPendingStargates.lock();
		bool anyPending = !m_PendingStargates.empty();
		m_muPendingStargates.unlock();
		if(!anyPending)
			return;

		// Pending stargates typically result from receiving the notification before DtDb has time to load the assigned system.
		m_muPendingStargates.lock();
		unordered_multimap<AuId, AuOpenStargate, AuIdHash>::iterator gi = m_PendingStargates.begin();	// Key is system id awaiting notification.
		while(gi != m_PendingStargates.end())
		{
			if(IsSolarSystemAssignedToMe((*gi).first))
			{
				AuEther& ether = g_Au.LockEtheriumDelete();
				DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom((*gi).first));
				if(system)
				{
					// This is used to open the destination negative stargate.
					// We create destination stargates at the primary star.
					DaStar* star = system->FindPrimaryStarNotBlackHole();
					if(star)
					{
						//qDebug() << "AuSceneServer::OpenDelayedStargates opening stargate at" << star->Id() << star->Name()
							//<< "in" << system->Id() << system->Name();
						star->OpenStargate((*gi).second.m_FromGalaxy, (*gi).second.m_FromGalaxyLoc, false, (*gi).second.m_Expire);
						gi = m_PendingStargates.erase(gi);
						g_Au.UnlockEtheriumDelete();
						continue;	// Skip loop iterator
					}
				}
				g_Au.UnlockEtheriumDelete();
			}

			gi++;
		}
		m_muPendingStargates.unlock();
	}
}

void AuSceneServerApp::StargateClosed(AuId fromGalaxy, AuPoint fromGalaxyLoc)
{
	//qDebug() << "AuSceneServerApp::StargateClosed from" << fromGalaxy << fromGalaxyLoc;
	if(g_Au.IsSceneMaster())
	{
		CloseStargateDown(fromGalaxy, fromGalaxyLoc);
	}
	else if(m_tClientChannel)
	{
		// Workers and Managers propagate the message up the server chain to the master.
		// Queue a msg for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_StargateClosed;
		out << fromGalaxy;
		out << fromGalaxyLoc;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::CloseStargateDown(AuId fromGalaxy, AuPoint fromGalaxyLoc)
{
	//qDebug() << "AuSceneServerApp::CloseStargateDown from" << fromGalaxy << fromGalaxyLoc;
	if(g_Au.IsSceneWorker())
	{
		// Master server must insure the sector exists and has been assigned to a manager.
		m_rwAtomManagers.lockForRead();
		QSet<AuId> mySystems;
		for(unordered_map<AuId, AuAtomManaged*, AuIdHash>::const_iterator si = m_MySystems.cbegin(); si != m_MySystems.cend(); si++)
			mySystems.insert((*si).first);
		m_rwAtomManagers.unlock();

		for(QSet<AuId>::const_iterator si = mySystems.cbegin(); si != mySystems.cend(); si++)
		{
			AuEther& ether = g_Au.LockEtheriumDelete();
			DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(*si));
			if(system)
			{
				// This is used to close the destination negative stargate.
				// We create destination stargates at the primary star so that's where we remove them from.
				DaStar* star = system->FindPrimaryStarNotBlackHole();
				if(star)
					star->CloseStargate(fromGalaxy, fromGalaxyLoc);
			}
			g_Au.UnlockEtheriumDelete();
		}
	}
	else
	{
		// Notify subordinate servers so the appropriate worker gets the message.
		emit CloseStargateDownSignal(fromGalaxy, fromGalaxyLoc);
	}
}

// This is the beginning of the road for signals emitted by arenas that are ready for players to join.
void AuSceneServerApp::ArenaReady(AuArenaReady arena)
{
	if(g_Au.IsSceneMaster())
	{
		ArenaReadyDown(arena);
	}
	else if(m_tClientChannel)
	{
		// Workers and Managers propagate the message up the server chain to the master.
		// Queue a msg for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_ArenaReady;
		out << arena;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::ArenaReadyDown(AuArenaReady arena)
{
	if(g_Au.IsSceneWorker())
	{
		// Worker server keeps a queue of arenas that are ready for players.
		arena.m_When.Set();
		unordered_map<AuId, AuArenaReady, AuIdHash>& arenas = g_Au.LockArenasForWrite();
		unordered_map<AuId, AuArenaReady, AuIdHash>::iterator ui = arenas.find(arena.m_Id);
		if(ui == arenas.end())
			arenas.insert(pair<AuId, AuArenaReady>(arena.m_Id, arena));
		else
			(*ui).second = arena;
		g_Au.UnlockArenas();
	}
	else
	{
		// Notify subordinate servers so the appropriate worker gets the message.
		emit ArenaReadyDownSignal(arena);
	}
}

void AuSceneServerApp::PlayerStatusBroadcast()
{
	if(!m_PlayerStatusTime.isValid())
	{
		// We must have just come up; don't bother yet.
		m_PlayerStatusTime.start();
		return;
	}

	// We do this at regular intervals.
	// We only receive updates during the one-minute status from our subs. No need to broadcast more often than that.
	if(m_PlayerStatusTime.elapsed()/d_MSecPerSec > 60)
	{
		m_PlayerStatusTime.restart();
		AuPlayerOnlineStatusVector players;
		PlayersOnlineStatus(players);							// Collect current list of players who are on line.
		PlayersOnlineStatusDown(players);						// Transmit it to my sub servers.
	}
}

void AuSceneServerApp::DeactivateAvatars()
{
	if(!m_AvatarDeactivateTime.isValid())
	{
		// We must have just come up. We won't have any atoms loaded yet anyway so don't bother.
		m_AvatarDeactivateTime.start();
		return;
	}

	// We do this once per day.
	if(m_AvatarDeactivateTime.ElapsedSec() < 60*60*24)
		return;
	m_AvatarDeactivateTime.restart();

	// Get lists of all avatars from the login server.
	AuIdList activeAvatars;
	AuIdList inactiveAvatars;
	AuLoginComm::QueryAvatars(activeAvatars, inactiveAvatars);
	if(activeAvatars.isEmpty() && inactiveAvatars.isEmpty())
		return;

	// Deactivate any live person atoms we have in our local data.
	qDebug() << "AuSceneServerApp::DeactivateAvatars" << activeAvatars.count() << "active" << inactiveAvatars.count() << "inactive";
	DaAtomList people;
	AuEther& ether = g_Au.LockEtheriumDelete();
	ether.ListNormalPeople(people);	// Lists all IsNormal people. It's ok if we skip over people in other atom states.
	for(DaAtomList::const_iterator pi = people.begin(); pi != people.end(); pi++)
	{
		DaPerson* person = (DaPerson*)*pi;
		if(activeAvatars.contains(person->Id()))
		{
			// The person is in the list of active avatars. Make sure the atom knows its account is active.
			if(person->IsAccountInactive())
				qDebug() << " Activating" << *person << (person->IsLineageDemiavatar()?"Demiavatar":"Avatar");
			person->SetAccountActive();
		}
		else if(inactiveAvatars.contains(person->Id()))
		{
			// The person is in the list of inactive avatars. Make sure the atom is deactivated so it will stop its AI.
			if(person->IsAccountActive())
				qDebug() << " Deactivating" << *person << (person->IsLineageDemiavatar()?"Demiavatar":"Avatar");
			person->SetAccountInactive();
		}
		else
		{
			// The avatar is not associated with any login account.
			if(person->CreatedTime().ElapsedDay() > 0)
			{
				// The avatar is more than one day old. Delete the avatar.
				qDebug() << (person->IsLineageDemiavatar()?" Demiavatar":" Avatar") << *person
					<< "Kill'ed because they are not associated with a login account.";
				person->Kill();
				person->SetModified();
			}
		}
	}
	g_Au.UnlockEtheriumDelete();
}

// 'motherAvatar' is actually the dad when the mother was an AI citizen.
void AuSceneServerApp::AvatarBorn(AuId motherAvatar, AuId babyAvatar)
{
	if(g_Au.IsSceneWorker())
	{
		// Create a birth announcement for dad and add it as a thought condition. Dad will get it later if he's off line right now.
		// It's not crucial that he get this.
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaPerson* dad = dynamic_cast<DaPerson*>(ether.FindNormalAtom(motherAvatar));
		if(dad && (dad->Gender() == Male || dad->Gender() == Hermaphrodite))
		{
			DaPerson* babyP = dynamic_cast<DaPerson*>(ether.FindNormalAtom(babyAvatar));
			if(babyP)
			{
				float mass = babyP->Mass() * KilogramsPerPound();
				QString birthAnnounce = QObject::tr("%1 was born at %2. %3 weighed %4kg.")
					.arg(babyP->Name())
					.arg(g_Au.CurrentUTC().toString())
					.arg(babyP->SheHe(true))
					.arg(mass, 0, 'f', 2);
				dad->AddCondition(new AuConThought(birthAnnounce, AuTime::Now()));
			}
		}
		g_Au.UnlockEtheriumDelete();
	}
	AuLoginComm login;
	login.PlayerUnitBorn(motherAvatar, babyAvatar);
}

void AuSceneServerApp::DemiavatarDied(AuId avatar)
{
	AuLoginComm login;
	login.PlayerUnitDied(avatar);
}

void AuSceneServerApp::AvatarAccepted(const AuId& avatarId, const QString& avatarName, const AuId& accepterId,
		const QString& accepterName)
{
	Q_ASSERT(!avatarId.IsNull());

	// Insure the accepter has an empty avatar slot.
	// Uh oh! This could block if there's trouble with the login servers.
	QString resultMsg;
	AuId accepterUnits[6];
	if(g_Au.ListPlayerUnits(accepterId, accepterUnits))
	{
		// Got the the accepter's list of units from their account.
		if(accepterUnits[0].IsNull() ||
				accepterUnits[1].IsNull() ||
				accepterUnits[2].IsNull() ||
				accepterUnits[3].IsNull() ||
				accepterUnits[4].IsNull() ||
				accepterUnits[5].IsNull())
		{
			// At least one of the accepter's unit slots is empty.
			AuLoginComm login;
			login.TransferPlayerUnit(avatarId, accepterId);
			resultMsg = tr("%1 was transferred to my account. %1 won't show up on the avatar switch UI until I re login.")
				.arg(avatarName);
		}
		else
			resultMsg = tr("My account has no open avatar slots.");
	}
	else
		resultMsg = tr("Information about my account is unavailable.");

	// Tell the accepter what happened.
	AuCommThought msg(resultMsg, accepterId, accepterName);
	g_Au.SendAtomChat(msg);
}

void AuSceneServerApp::SpacecraftNameChanged(AuId shipId, QString nuName)
{
	//qDebug() << "AuSceneServerApp::SpacecraftNameChanged from" << fromGalaxy << fromGalaxyLoc;
	if(g_Au.IsSceneMaster())
	{
		SpacecraftNameChangedDown(shipId, nuName);
	}
	else if(m_tClientChannel)
	{
		// Workers and Managers propagate the message up the server chain to the master.
		// Queue a msg for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_SpacecraftNameChanged;
		out << shipId;
		out << nuName;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::SpacecraftNameChangedDown(AuId shipId, QString nuName)
{
	//qDebug() << "AuSceneServerApp::SpacecraftNameChangedDown" << shipId << nuName;
	if(g_Au.IsSceneWorker())
	{
		// Update all avatars who are berthed aboard the ship.
		DaAtomList people;
		AuEther& ether = g_Au.LockEtheriumDelete();
		ether.ListNormalPeople(people);
		for(DaAtomList::const_iterator pi = people.begin(); pi != people.end(); pi++)
		{
			DaPerson* person = (DaPerson*)*pi;
			person->SpacecraftNameChanged(shipId, nuName);
		}
		g_Au.UnlockEtheriumDelete();
	}
	else
	{
		// Notify subordinate servers so the appropriate worker gets the message.
		emit SpacecraftNameChangedSignal(shipId, nuName);
	}
}

// An atom signaled activation of a henge.
void AuSceneServerApp::HengeActivated(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
		AuId destSysId, quint8 destSun, quint8 destArcSegment)
{
	// Henges only reach their current sector so we don't have to propagate them up to the master.
	if(g_Au.IsSceneManager())
	{
		// I can determine the exact server to send this notification to, without bothering the others.
		DhSceneServer toServer = SolarSystemServer(destSysId);
		if(toServer.IsValid())
		{
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_HengeActivated;
			out << fromSysId;
			out << fromSun;
			out << fromShadowPanels;
			out << fromArcSegment;
			out << destSysId;
			out << destSun;
			out << destArcSegment;
			SendPacketTo(ba, toServer.m_Id);
		}
	}
	else if(m_tClientChannel)
	{
		// Notify my boss server.
		m_tClientChannel->HengeActivated(fromSysId, fromSun, fromShadowPanels, fromArcSegment, destSysId, destSun, destArcSegment);
	}
}

void AuSceneServerApp::HengeActivatedDown(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
		AuId destSysId, quint8 destSun, quint8 destArcSegment)
{
	if(!IsSolarSystemAssignedToMe(destSysId))
		return;
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(destSysId));
	if(system)
		system->HengeActivated(fromSysId, fromSun, fromShadowPanels, fromArcSegment, destSun, destArcSegment);
	g_Au.UnlockEtheriumDelete();
}

// An atom signaled activation of a henge.
void AuSceneServerApp::HengeCreated(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
		AuId destSysId, quint8 destSun, quint8 destArcSegment)
{
	// Henges only reach their current sector so we don't have to propagate them up to the master.
	if(g_Au.IsSceneManager())
	{
		// I can determine the exact server to send this notification to, without bothering the others.
		DhSceneServer toServer = SolarSystemServer(destSysId);
		if(toServer.IsValid())
		{
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_HengeCreated;
			out << fromSysId;
			out << fromSun;
			out << fromShadowPanels;
			out << fromArcSegment;
			out << destSysId;
			out << destSun;
			out << destArcSegment;
			SendPacketTo(ba, toServer.m_Id);
		}
	}
	else if(m_tClientChannel)
	{
		// Notify my boss server.
		m_tClientChannel->HengeCreated(fromSysId, fromSun, fromShadowPanels, fromArcSegment, destSysId, destSun, destArcSegment);
	}
}

void AuSceneServerApp::HengeCreatedDown(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
		AuId destSysId, quint8 destSun, quint8 destArcSegment)
{
	if(!IsSolarSystemAssignedToMe(destSysId))
		return;
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(destSysId));
	if(system)
		system->HengeCreated(fromSysId, fromSun, fromShadowPanels, fromArcSegment, destSun, destArcSegment);
	g_Au.UnlockEtheriumDelete();
}

// An atom has signaled deactivation of a henge.
void AuSceneServerApp::HengeDeactivated(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
		AuId destSysId, quint8 destSun, quint8 destArcSegment)
{
	// Henges only reach their current sector so we don't have to propagate them up to the master.
	if(g_Au.IsSceneManager())
	{
		// I can determine the exact server to send this notification to, without bothering the others.
		DhSceneServer toServer = SolarSystemServer(destSysId);
		if(toServer.IsValid())
		{
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_HengeDeactivated;
			out << fromSysId;
			out << fromSun;
			out << fromShadowPanels;
			out << fromArcSegment;
			out << destSysId;
			out << destSun;
			out << destArcSegment;
			SendPacketTo(ba, toServer.m_Id);
		}
	}
	else if(m_tClientChannel)
	{
		// Notify my boss server.
		m_tClientChannel->HengeDeactivated(fromSysId, fromSun, fromShadowPanels, fromArcSegment, destSysId, destSun,
				destArcSegment);
	}
}

void AuSceneServerApp::HengeDeactivatedDown(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
		AuId destSysId, quint8 destSun, quint8 destArcSegment)
{
	if(!IsSolarSystemAssignedToMe(destSysId))
		return;
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(destSysId));
	if(system)
		system->HengeDeactivated(fromSysId, fromSun, fromShadowPanels, fromArcSegment, destSun, destArcSegment);
	g_Au.UnlockEtheriumDelete();
}

// This trail of logic was copied from SpawnAtom.
// This uses the database to handoff the teleporting atom.
void AuSceneServerApp::HengeTeleporterEntered(AuId teleportId, AuId fromSysId, quint8 fromSun, quint8 fromArc, quint8 shadowPanels,
		AuId destSysId, quint8 destSun, quint8 destArc)
{
	//Q_ASSERT(g_Au.IsSceneWorker());
	if(g_Au.m_LogMessages & LOG_Travel)
		qDebug() << "AuSceneServerApp::HengeTeleporterEntered tele" << teleportId << "from sys" << fromSysId << "sun" << fromSun
			<< "arc" << fromArc << "panels" << shadowPanels << "to sys" << destSysId << "sun" << destSun << "arc" << destArc;

	AuHengeTeleportAtom hengeTel(teleportId, fromSysId, fromSun, fromArc, shadowPanels, destSysId, destSun, destArc);
	emit HengeTeleportAtomViaDb(hengeTel);
}

void AuSceneServerApp::HengeTeleportAtomUp(AuHengeTeleportAtom hengeTel)
{
	HengeTeleportAtomUp2(hengeTel);
}

void AuSceneServerApp::HengeTeleportAtomUp2(const AuHengeTeleportAtom& hengeTel)
{
	if(g_Au.IsSceneMaster())
	{
		// This is part of the chain of events resulting from a creature entering a henge portal.
		HengeTeleportAtomDown2(hengeTel);
	}
	else if(m_tClientChannel)
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_HengeTeleportAtom;
		out << hengeTel;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::HengeTeleportAtomDown2(const AuHengeTeleportAtom& hengeTel)
{
	//Q_ASSERT(!g_Au.IsSceneWorker());

	// Prepare to wait for the information we need to make a proper decision about where to spawn the person.
	AuHengeTeleportRequest hengeReq;
	hengeReq.m_Start.Set();
	hengeReq.m_HengeTel = hengeTel;

	// This could be a repeat request so remove any pending request for the same person or ship.
	m_SpawnMutex.lock();
	QList<AuHengeTeleportRequest>::iterator it = m_HengeTeleports.begin();
	while(it != m_HengeTeleports.end())
	{
		if((*it).m_HengeTel.m_TeleportId == hengeTel.m_TeleportId)
			it = m_HengeTeleports.erase(it);
		else
			it++;
	}

	// Append the nu spawn request.
	m_HengeTeleports.append(hengeReq);
	m_SpawnMutex.unlock();

	// Broadcast atom queries to subordinates to locate the spawn points.
	// We prefer to find the arena first because that could terminate our search.
	QueryHengeTeleporterServer(hengeTel);
}

// Posts a query to locate the subordinate server who manages a region of space that contains the atom destSysId.
void AuSceneServerApp::QueryHengeTeleporterServer(AuHengeTeleportAtom hengeTel)
{
	if(g_Au.m_LogMessages & LOG_AtomQuery)
	{
		QString logmsg = QString("AuSceneServerApp::QueryHengeTeleporterServer atom %d sun %d arc %d teleportId %d")
				.arg(hengeTel.m_DestSysId.GetID())
				.arg(hengeTel.m_DestSun)
				.arg(hengeTel.m_DestArc)
				.arg(hengeTel.m_TeleportId.GetID());
		g_Au.Log(logmsg);
	}

	if(g_Au.IsSceneMaster())
	{
		// Scene master must defer the query to its subordinate manager(s).
		AuHengeTeleportDestQuery query;
		query.m_TeleportId = hengeTel.m_TeleportId;

		const unordered_map<unsigned int, DtServerChannel*>& servers = m_ServerChannel.LockServerList(__FILE__, __LINE__);
		query.m_RepliesPending = servers.size();	// The number of reponses we are expecting from the query.
		m_ServerChannel.UnlockServerList(__FILE__, __LINE__);

		// Remember the atom queries that are unresolved.
		m_QueryMutex.lock();
		m_HengeTeleportDestQueries.append(query);
		m_QueryMutex.unlock();

		emit QueryHengeTeleporterServerDown(hengeTel);
	}
	else if(g_Au.IsSceneManager())
	{
		// The scene manager finalizes solar system contents. Make sure the destination is final.
		// If the destination has a server assigned, then it is implicitly finalized.
		DhSceneServer toServer = SolarSystemServer(hengeTel.m_DestSysId);
		if(!toServer.IsValid())
		{
			// The destination system is not assigned to any of my workers.
			// If it is in a sector that is assigned to me, I may be able to finalize it.
			AuEther& ether = g_Au.LockEtheriumDelete();
			DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(hengeTel.m_DestSysId));
			if(system && IsSectorAssignedToMe(system->ParentId()))
			{
				// The solar system's parent sector is assigned to me which means I have authority over the solar system.
				// Finalize the solar system contents. This will cause it to be assigned to a worker server who will load all
				// the data, including the nuly arrived atom.
				system->FinalizeContents();
				g_AuDb.PostCriticalUpdates();
				if(g_Au.m_LogMessages & LOG_AtomQuery)
					qDebug() << "AuSceneServerApp::QueryHengeTeleporterServer Solar System" << system->Id() << system->Name()
						<< "finalized, ready for assignment.";
			}
			else if(!system)
			{
				if(g_Au.m_LogMessages & LOG_AtomQuery)
					qDebug() << "AuSceneServerApp::QueryHengeTeleporterServer system not found" << hengeTel.m_DestSysId;
			}
			else
			{
				if(g_Au.m_LogMessages & LOG_AtomQuery)
					qDebug() << "AuSceneServerApp::QueryHengeTeleporterServer parent sector of system" << hengeTel.m_DestSysId
						<< "not assigned to me";
			}
			g_Au.UnlockEtheriumDelete();

			// Respond to server that they should ask again. It will take a moment for the assignment to go through and for the
			// worker server to be ready.
			if(m_tClientChannel)
				m_tClientChannel->SendHengeTeleportDestPresence(hengeTel.m_TeleportId, 2);
		}
		else
		{
			// Destination system is assigned to one of my workers.
			AuHengeTeleportDestQuery query;
			query.m_TeleportId = hengeTel.m_TeleportId;
			query.m_RepliesPending = 1;		// The number of reponses we are expecting from the query.

			// Remember the atom queries that are unresolved.
			m_QueryMutex.lock();
			m_HengeTeleportDestQueries.append(query);
			m_QueryMutex.unlock();

			// Post the query to the specific worker that is assigned the destination solar system.
			m_ServerChannel.LockServerList(__FILE__, __LINE__);
			DtServerChannel* server = m_ServerChannel.ServerChannel(toServer.m_Id);
			if(server && !server->isFinished())
				server->QueryHengeTeleporterPresence(hengeTel);
			m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
		}
	}
	else
	{
		// Scene Worker checks its local atoms for the destination henge portal.
		quint8 result = 0;
		if(IsSolarSystemAssignedToMe(hengeTel.m_DestSysId))
		{
			// Destination system is assigned to me.
			AuEther& ether = g_Au.LockEtheriumDelete();
			DaSolarSystem* destSys = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(hengeTel.m_DestSysId));
			if(destSys)
			{
				// Solar system was found. Is it ready?
				if(destSys->IsOnlyStarFieldLoaded())
					result = 2;		// Destination system is not ready yet. It must not be loaded from the db yet.
				else
				{
					// Does the system have the appropriate destination world and henge portal?
					// HengeActivated creates and activates the portal if possible.
					if(destSys->HengeActivated(hengeTel.m_FromSysId, hengeTel.m_FromSun, hengeTel.m_ShadowPanels,
								hengeTel.m_FromArc, hengeTel.m_DestSun, hengeTel.m_DestArc))
						result = 1;
					else
						result = 3;	// No place for a portal?
				}
			}
			g_Au.UnlockEtheriumDelete();
		}
		else
			result = 2;	// Manager only sends it to me if I'm assigned the system. I may not have received the assignement it yet.

		// Send the result, good or bad.
		if(m_tClientChannel)
			m_tClientChannel->SendHengeTeleportDestPresence(hengeTel.m_TeleportId, result);
	}

	if(g_Au.m_LogMessages & LOG_AtomQuery)
	{
		QString logmsg("AuSceneServerApp::QueryHengeTeleporterServer done");
		g_Au.Log(logmsg);
	}
}

// Called when a subordinate server answers a henge teleporter presence query.
void AuSceneServerApp::HengeTeleporterPresence(AuId teleportId, int presence, unsigned int serverId)
{
	//Q_ASSERT(!g_Au.IsSceneWorker());	// Never has subordinate servers.
	// Insure we're still waiting to resolve the query. This could be a
	// late response or one received after we got a definitive answer.
	bool queryActive = false;
	bool isLastReply = false;
	m_QueryMutex.lock();
	for(QList<AuHengeTeleportDestQuery>::iterator it = m_HengeTeleportDestQueries.begin(); it != m_HengeTeleportDestQueries.end();
			it++)
	{
		if((*it).m_TeleportId == teleportId)
		{
			// The atom query is still waiting for a definite result.
			queryActive = true;
			(*it).m_RepliesPending--;
			if(!(*it).m_RepliesPending)
				isLastReply = true;
			break;
		}
	}
	m_QueryMutex.unlock();

	// Ignore the call if we're not waiting for the information any more.
	if(!queryActive)
		return;

	if(presence)
	{
		// The atom was found; send notifications immediately.
		// Signal that the query is no longer active so it will be removed from the pending queries array.
		queryActive = false;
	}
	else
	{
		// A sub server reported the atom is definitely not present.
		// If we have heard from all the subServers, send the final result.
		if(isLastReply)
		{
			// Signal that the query is no longer active so it will be removed from the pending queries array.
			queryActive = false;
		}
	}

	// We're done if the query is still active.
	if(queryActive)
		return;

	// The atom query is resolved so remove it from our pending query results array.
	m_QueryMutex.lock();
	for(QList<AuHengeTeleportDestQuery>::iterator itQuery = m_HengeTeleportDestQueries.begin();
			itQuery != m_HengeTeleportDestQueries.end(); itQuery++)
	{
		if((*itQuery).m_TeleportId == teleportId)
		{
			// This is the first query that matches so remove it.
			m_HengeTeleportDestQueries.erase(itQuery);
			break;
		}
	}
	m_QueryMutex.unlock();

	// Notify the scene master also, in case he cares.
	// He gets the actual result: 0=no, 1=yes, 2=not now, 3=not possible
	if(!g_Au.IsSceneMaster() && m_tClientChannel)
		m_tClientChannel->SendHengeTeleportDestPresence(teleportId, presence);

	// Scene master and manager look for multiple atoms when filling spawn requests.
	// If the serverId = 1 for a given atom, they'll know the search for that atom found it but it was not viable.
	m_SpawnMutex.lock();
	for(QList<AuHengeTeleportRequest>::iterator it = m_HengeTeleports.begin(); it != m_HengeTeleports.end(); it++)
		if((*it).m_HengeTel.m_TeleportId == teleportId)
		{
			(*it).m_DestServerId = serverId;
			(*it).m_Result = presence;
		}
	m_SpawnMutex.unlock();
}

// Teleport atoms that have accumulated information about the destination henge portal.
// This is called periodically.
void AuSceneServerApp::HengeTeleportAtoms()
{
	// Scene worker doesn't do anything with spawn requests.
	if(g_Au.IsSceneWorker())
		return;

	// Array of spawn requests that must be reposted due to timeout.
	QList<AuHengeTeleportAtom> repost;

	m_SpawnMutex.lock();
	if((g_Au.m_LogMessages & LOG_Spawn) && !m_HengeTeleports.isEmpty())
		qDebug("AuSceneServerApp::HengeTeleportAtoms %d atoms waiting to spawn", m_HengeTeleports.count());
	QList<AuHengeTeleportRequest>::iterator it = m_HengeTeleports.begin();
	while(it != m_HengeTeleports.end())
	{
		if(g_Au.m_LogMessages & LOG_Spawn)
			qDebug() << "AuSceneServerApp::HengeTeleportAtoms atom" << (*it).m_HengeTel.m_TeleportId
				<< "timeO" << (*it).IsTimedOut()
				<< "ready" << (*it).IsReadyToTeleport();

		if((*it).IsReadyToTeleport())
		{
			// We found a henge teleport request that is ready.
			// Inform the appropriate manager of the henge teleport.
			if((*it).m_Result == 1)
			{
				// Good result from sub server.
				// Notify the sub thread who claims to have our spawn point atom.
				m_ServerChannel.LockServerList(__FILE__, __LINE__);
				DtServerChannel* server = m_ServerChannel.ServerChannel((*it).m_DestServerId);
				if(server && !server->isFinished())
					server->HengeTeleportAtom((*it).m_HengeTel);
				m_ServerChannel.UnlockServerList(__FILE__, __LINE__);
			}
			else if((*it).m_Result == 2)
			{
				// Destination system is not yet available; the server must be finalizing or loading it.
				// Sub server says to try again.
				repost.append((*it).m_HengeTel);
			}
			else
			{
				// The destination system cannot work because there must be no world on which to create a destination henge portal.
				// Switch the request around to send them back where they came from.
				AuHengeTeleportAtom goback;
				goback.GoBack((*it).m_HengeTel);
				repost.append(goback);
			}

			// Remove the spawn request now that we've dealt with it.
			it = m_HengeTeleports.erase(it);
		}
		else if((*it).IsTimedOut())
		{
			// We get here if the spawn request timed out without finding all of our henge portal.
			if(g_Au.m_LogMessages & LOG_Spawn)
			{
				QString logmsg = tr("AuSceneServerApp::HengeTeleportAtoms retrying timed-out teleport %1")
					.arg((*it).m_HengeTel.m_TeleportId.GetID());
				g_Au.Warning(logmsg);
			}

			// Send them back where they came from.
			AuHengeTeleportAtom goback;
			goback.GoBack((*it).m_HengeTel);
			repost.append(goback);

			// Remove the spawn request now. It will be reposted.
			it = m_HengeTeleports.erase(it);
		}
		else
			it++;
	}
	m_SpawnMutex.unlock();

	// Repost spawn requests that failed.
	for(QList<AuHengeTeleportAtom>::iterator rit = repost.begin(); rit != repost.end(); rit++)
		HengeTeleportAtomUp2(*rit);
}

void AuSceneServerApp::HengeTeleportAtomDown(AuHengeTeleportAtom hengeTel)
{
	if(g_Au.IsSceneWorker())
	{
		// This worker received the spawn atom down so it must be in charge of the spawnAt atom.
		bool atomTransferred = false;

		// Locate the spawn point atom.
		AuEther& ether = g_Au.LockEtheriumDelete();
		const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(ether.FindNormalAtom(hengeTel.m_DestSysId));
		if(system && !system->IsOnlyStarFieldLoaded())
		{
			const DaSector* sector = system->FindParentSector();
			if(sector)
			{
				emit HengeTeleportArrivedViaDb(hengeTel);
				atomTransferred = true;
			}
		}
		g_Au.UnlockEtheriumDelete();

		if(!atomTransferred)
			HengeTeleportAtomUp2(hengeTel);	// Repost the henge teleport request.
	}
	else
	{
		// Propagate the henge teleport request down to the workers.
		//Q_ASSERT(g_Au.IsSceneManager());
		HengeTeleportAtomDown2(hengeTel);
	}
}

// A local world has signaled that it is the destination of a henge portal; its HengeCreated function was called.
// All the workers need to know where these are for story purposes.
void AuSceneServerApp::HengeDestinationConfirmed(AuId sectorId, AuId systemId, QString systemName, quint8 sun, QString worldCatName,
		QString worldName, quint8 worldType, AuGlobeLLF hengeLL, bool breathable, quint8 orbitZone, bool outsideTown)
{
	// Send the notification up the server chain until it reaches the master.
	if(!g_Au.IsSceneMaster())
	{
		// Send the message to my boss server.
		if(m_tClientChannel)
			m_tClientChannel->SendHengeDestinationConfirmed(sectorId, systemId, systemName, sun, worldCatName, worldName, worldType,
					hengeLL, breathable, orbitZone, outsideTown);
		return;
	}

	// Master broadcasts the message back down to all the servers.
	HengeDestinationDown(sectorId, systemId, systemName, sun, worldCatName, worldName, worldType, hengeLL, breathable, orbitZone,
			outsideTown);
}

void AuSceneServerApp::HengeDestinationDown(AuId sectorId, AuId systemId, QString systemName, quint8 sun, QString worldCatName,
		QString worldName, quint8 worldType, AuGlobeLLF hengeLL, bool breathable, quint8 orbitZone, bool outsideTown)
{
	// Send the notification down the server chain until it reaches the master.
	if(!g_Au.IsSceneWorker())
	{
		// Broadcast the message to all my sub servers.
		emit HengeDestinationDownSignal(sectorId, systemId, systemName, sun, worldCatName, worldName, worldType, hengeLL, breathable,
				orbitZone, outsideTown);
		return;
	}

	// Worker caches the information about henge destination worlds locally.
	AuHengeDestination dest(sectorId, systemId, systemName, sun, worldCatName, worldName, worldType, hengeLL, breathable,
			(ORBITZONE)orbitZone, outsideTown);
	AuHengeDestinations& hd = g_Au.LockHengeDestinationsForWrite();
	hd.Update(dest);
	//int hdcount = hd.count();
	g_Au.UnlockHengeDestinations();
	//qDebug() << "AuSceneServerApp::HengeDestinationDown sector" << sectorId << "sys" << systemId << systemName
		//<< "world" << worldId << hengeLL << "hengeDestinations" << hdcount;
}

void AuSceneServerApp::GovDossierAddDocument(quint32 empireId, quint32 dosEmpire, QByteArray packedDoc)
{
	if(g_Au.IsSceneMaster())
	{
		// Add the document to the empire's dossier.
		AuDocument* doc = AuDocument::Unpack(packedDoc);
		if(doc)
		{
			DgData& gd = g_Au.LockDataForWrite();
			int result = gd.EmpireDossierAddDocument(empireId, dosEmpire, *doc);
			g_Au.UnlockData();
			delete doc;

			if(result)
				emit ChangeEmpireInDb(empireId);			// Trigger db store, which notifies the subordinates when done.
			if(result == 2)
				emit ChangeEmpireInDb(dosEmpire);			// Trigger db store, due to war/stance change.
		}
	}
	else if(m_tClientChannel)
	{
		// Send it up to the master server via my boss server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_GovDossierAddDocument;
		out << empireId;
		out << dosEmpire;
		out << packedDoc;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::SpacecraftStasisRecoverUp(AuId shipId, AuId personId, quint8 personGalaxy, bool agentRecover)
{
	if(g_Au.IsSceneMaster())
	{
		SpacecraftStasisRecoverDown(shipId, personId, personGalaxy, agentRecover);
	}
	else if(m_tClientChannel)
	{
		// Send it up to the master server via my boss server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_ShipStasisRecover;
		out << shipId;
		out << personId;
		out << personGalaxy;
		out << agentRecover;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::SpacecraftStasisRecoverDown(AuId shipId, AuId personId, quint8 personGalaxy, bool agentRecover)
{
	if(!g_Au.IsSceneWorker())
	{
		// Broadcast the message to all my sub servers.
		emit SpacecraftStasisRecoverSignal(shipId, personId, personGalaxy, agentRecover);
		return;
	}

	// Worker servers look for the ship to restore. This is where the restoration from stasis finally happens.

	// Recovering ships requires the root mutex. Make a hash table of them and kick them off at the end. They happen rarely.
	unordered_map<AuId, AuId, AuIdHash> recoverShips;

	// An unmoored dormant ship is a destroyed ship in stasis that can be reactivated by summoning.
	// Ships can only be in stasis if they have a human officer on the roster; this is how they recover their lost ship.
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaVessel* ship = dynamic_cast<DaVessel*>(ether.FindDormantAtom(shipId));
	if(ship && !ship->IsMoored())
	{
		// The dormant unmoored ship was found here.
		// A moored dormant ship is a ship stored in a hanger; that cannot be summoned here.
		// Ships in stasis cannot be restored immediately.
		AuTime stasisTime = ship->StasisReadyTime();
		GALAXY stasisGalaxy = ship->Galaxy();
		if(AuTime::Now() >= stasisTime || agentRecover)
		{
			// Use the agent teleport method to send the ship to the crewman, as if they summoned it.
			recoverShips.insert(pair<AuId, AuId>(shipId, personId));

			// Notify the sender that the ship was recovered.
			AuCommMessage mymsg(AuCommMessage::ShipSummoned, AuCommStdChannel::CrewChannelName(), shipId, ship->Name(),
					ship->FindParentSector()->Id(), stasisGalaxy, AuCommStdChannel::Universe,
					AuCommStdChannel::CrewMates);
			mymsg.SetScopeData(shipId.GetID());	// For CrewMates scope.
			g_Au.SendAtomChat(mymsg);
		}
		else
		{
			// Notify the sender that the ship is not ready to be recovered..
			AuCommShipStasisWait mymsg(stasisTime, AuCommStdChannel::CrewChannelName(), shipId, ship->Name(),
					ship->FindParentSector()->Id(), stasisGalaxy, AuCommStdChannel::Universe,
					AuCommStdChannel::CrewMates);
			mymsg.SetScopeData(shipId.GetID());	// For CrewMates scope.
			g_Au.SendAtomChat(mymsg);
		}
	}
	g_Au.UnlockEtheriumDelete();

	for(unordered_map<AuId, AuId, AuIdHash>::const_iterator sdi = recoverShips.cbegin(); sdi != recoverShips.cend(); sdi++)
		SummonDormantShip((*sdi).first, (*sdi).second);
}

void AuSceneServerApp::ScDesignDeactivatedDown(quint32 designId)
{
	//QString logmsg;
	//logmsg.sprintf("AuSceneServerApp::ScDesignDeactivatedDown design %d", designId);
	//g_Au.Log(logmsg);
	if(g_Au.IsSceneWorker())
	{
		// Worker server signals db thread to mark the ship design Deleted if it is already loaded.
		DgData& gd = g_Au.LockScDesignsForWrite();
		gd.ScDesignDeactivated(designId);
		g_Au.UnlockScDesigns();
	}
	else
	{
		// Broadcast a notification to all sub servers.
		emit ScDesignDeactivatedSignal(designId);
	}
}

void AuSceneServerApp::ScDesignDeactivatedUp(quint32 designId)
{
	//QString logmsg;
	//logmsg.sprintf("AuSceneServerApp::ScDesignDeactivatedUp design %d", designId);
	//g_Au.Log(logmsg);
	if(g_Au.IsSceneMaster())
	{
		// The scene master sends the notification down the chain so all workers receive the message.
		ScDesignDeactivatedDown(designId);
	}
	else if(m_tClientChannel)
	{
		// Workers and Managers propagate the message up the server chain to the master.
		// Queue a msg for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_ScDesignDeactivated;
		out << designId;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::ScDesignExchangePublishedUp(AuScDesignExchangeEntry item)
{
	if(g_Au.IsSceneMaster())
	{
		// The scene master sends the notification down the chain so all workers receive the message.
		ScDesignExchangePublishedDown(item);
	}
	else if(m_tClientChannel)
	{
		// Workers and Managers propagate the message up the server chain to the master.
		// Queue a msg for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_ScDesignExchangePublished;
		out << item;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::ScDesignExchangePublishedDown(AuScDesignExchangeEntry item)
{
	if(g_Au.IsSceneWorker())
	{
		// Worker server signals db thread to mark the ship design Deleted if it is already loaded.
		DgData& gd = g_Au.LockScExchangeForWrite();
		gd.m_ScExchange->insert(item.DesignId(), item);
		g_Au.UnlockScExchange();
	}
	else
	{
		// Broadcast a notification to all sub servers.
		emit ScDesignExchangePublishedSignal(item);
	}
}

void AuSceneServerApp::ScDesignExchangeUnpublishedUp(quint32 designId)
{
	if(g_Au.IsSceneMaster())
	{
		// The scene master sends the notification down the chain so all workers receive the message.
		ScDesignExchangeUnpublishedDown(designId);
	}
	else if(m_tClientChannel)
	{
		// Workers and Managers propagate the message up the server chain to the master.
		// Queue a msg for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_ScDesignExchangeUnpublished;
		out << designId;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::ScDesignExchangeUnpublishedDown(quint32 designId)
{
	if(g_Au.IsSceneWorker())
	{
		// Worker server signals db thread to mark the ship design Deleted if it is already loaded.
		DgData& gd = g_Au.LockScExchangeForWrite();
		gd.m_ScExchange->remove(designId);
		g_Au.UnlockScExchange();
	}
	else
	{
		// Broadcast a notification to all sub servers.
		emit ScDesignExchangeUnpublishedSignal(designId);
	}
}

void AuSceneServerApp::CityFacilitiesChanged(AuId cityId, AuCityIndustryHash facil)
{
	if(g_Au.IsSceneMaster())
	{
		// Update the appropriate empire record.
		DgData& gd = g_Au.LockDataForWrite();
		quint32 cityEmpireId = gd.CityFacilitiesChanged(cityId, facil);
		g_Au.UnlockData();
		if(cityEmpireId)
			emit ChangeEmpireInDb(cityEmpireId);				// Trigger db store which notifies the subordinates when done.
	}
	else if(m_tClientChannel)
	{
		// Workers and Managers propagate the message up the server chain to the master.
		// Queue a msg for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_CityFacilities;
		out << cityId;
		out << facil;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::CityStatusChanged(AuId cityId, int alertCondition, qint8 morale)
{
	if(g_Au.IsSceneMaster())
	{
		// Update the appropriate empire record.
		DgData& gd = g_Au.LockDataForWrite();
		quint32 cityEmpireId = gd.CityStatusChanged(cityId, (ALERT_CONDITION)alertCondition, morale);
		g_Au.UnlockData();
		if(cityEmpireId)
			emit ChangeEmpireInDb(cityEmpireId);				// Trigger db store which notifies the subordinates when done.
	}
	else if(m_tClientChannel)
	{
		// Workers and Managers propagate the message up the server chain to the master.
		// Queue a msg for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_CityStatus;
		out << cityId;
		out << (quint8)alertCondition;
		out << morale;
		m_tClientChannel->SendPacket(ba);
	}
}

// Initial start of the trail of shipping a container. Atom signal to ship a container lands here.
void AuSceneServerApp::ShipIt(AuItemTEUContainer container)
{
	//qDebug() << "AuSceneServerApp::ShipIt" << container.DestinationCity() << container.DestinationSystem()
		//<< container.DestinationSector() << container.DestinationDesc();
	if(g_Au.IsSceneMaster())
	{
		// Scene master sends the container down to the appropriate manager.
		ShipItDown(container);
		return;
	}
	else if(g_Au.IsSceneManager())
	{
		// Scene master sends the container down to the appropriate worker if the destination sector is assigned to me.
		if(IsSectorAssignedToMe(container.DestinationSector()))
		{
			ShipItDown(container);
			return;
		}
	}
	else
	{
		// Scene worker sends the container down to the appropriate city atom if the destination system is assigned to me.
		if(IsSolarSystemAssignedToMe(container.DestinationSystem()))
		{
			ShipItDown(container);
			return;
		}
	}

	// Send the container up to my boss, either a manager or master.
	if(m_tClientChannel)
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_Trade;
		out << (quint8)0;
		out << container;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::ShipItDown(AuItemTEUContainer container)
{
	//qDebug() << "AuSceneServerApp::ShipItDown" << container.DestinationCity() << container.DestinationSystem()
		//<< container.DestinationSector() << container.DestinationDesc();
	if(g_Au.IsSceneMaster())
	{
		// Master server.
		DhSceneServer toServer = SectorServer(container.DestinationSector());
		if(toServer.IsValid())
		{
			// Send the container to the manager who hosts the destination sector.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_Trade;
			out << (quint8)0;
			out << container;
			SendPacketTo(ba, toServer.m_Id);
		}
	}
	else if(g_Au.IsSceneManager())
	{
		// Manager server.
		DhSceneServer toServer = SolarSystemServer(container.DestinationSystem());
		if(toServer.IsValid())
		{
			// Send the container to the worker who hosts the destination system.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_Trade;
			out << (quint8)0;
			out << container;
			SendPacketTo(ba, toServer.m_Id);
		}
	}
	else
	{
		// Worker server.
		// I expect to find the destination in my local data. If it's not here, the container is lost. We don't need these
		// things bouncing around forever looking for a city that is gone.
		AuId idSystem(container.DestinationSystem());
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaSolarSystem* system = AssignedSolarSystem(idSystem, ether);
		if(system)
		{
			DaAtom* recipient = system->FindIdRecursive(container.DestinationCity());
			if(recipient)
				recipient->ShipmentDeliver(container);			// Shipment finally reaches the destination here.
		}
		g_Au.UnlockEtheriumDelete();
	}
}

void AuSceneServerApp::PlayersOnlineStatusDown(AuPlayerOnlineStatusVector players)
{
	if(g_Au.IsSceneMaster() || g_Au.IsSceneManager())
	{
		emit PlayersOnlineStatusSignal(players);
	}
	else
	{
		// Store the updated player status.
		m_PlayerStatusLock.lockForWrite();
		m_PlayerStatus = players;
		m_PlayerStatusLock.unlock();
	}
}

void AuSceneServerApp::AllPlayersOnlineStatus(AuPlayerOnlineStatusVector& players, bool forAgent) const
{
	m_PlayerStatusLock.lockForRead();
	m_PlayerStatus.CopyForClient(players, forAgent);
	m_PlayerStatusLock.unlock();
}

void AuSceneServerApp::BdDesignDeactivatedDown(quint32 designId)
{
	//QString logmsg;
	//logmsg.sprintf("AuSceneServerApp::BdDesignDeactivatedDown design %d", designId);
	//g_Au.Log(logmsg);
	if(g_Au.IsSceneWorker())
	{
		// Worker server signals db thread to mark the ship design Deleted if it is already loaded.
		DgData& gd = g_Au.LockBdDesignsForWrite();
		gd.BdDesignDeactivated(designId);
		g_Au.UnlockBdDesigns();
	}
	else
	{
		// Broadcast a notification to all sub servers.
		emit BdDesignDeactivatedSignal(designId);
	}
}

void AuSceneServerApp::BdDesignDeactivatedUp(quint32 designId)
{
	//QString logmsg;
	//logmsg.sprintf("AuSceneServerApp::BdDesignDeactivatedUp design %d", designId);
	//g_Au.Log(logmsg);
	if(g_Au.IsSceneMaster())
	{
		// The scene master sends the notification down the chain so all workers receive the message.
		BdDesignDeactivatedDown(designId);
	}
	else if(m_tClientChannel)
	{
		// Workers and Managers propagate the message up the server chain to the master.
		// Queue a msg for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_BdDesignDeactivated;
		out << designId;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::BdDesignExchangePublishedUp(AuBdDesignExchangeEntry item)
{
	if(g_Au.IsSceneMaster())
	{
		// The scene master sends the notification down the chain so all workers receive the message.
		BdDesignExchangePublishedDown(item);
	}
	else if(m_tClientChannel)
	{
		// Workers and Managers propagate the message up the server chain to the master.
		// Queue a msg for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_BdDesignExchangePublished;
		out << item;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::BdDesignExchangePublishedDown(AuBdDesignExchangeEntry item)
{
	if(g_Au.IsSceneWorker())
	{
		// Worker server signals db thread to mark the ship design Deleted if it is already loaded.
		DgData& gd = g_Au.LockBdExchangeForWrite();
		gd.m_BdExchange->insert(item.DesignId(), item);
		g_Au.UnlockBdExchange();
	}
	else
	{
		// Broadcast a notification to all sub servers.
		emit BdDesignExchangePublishedSignal(item);
	}
}

void AuSceneServerApp::BdDesignExchangeUnpublishedUp(quint32 designId)
{
	if(g_Au.IsSceneMaster())
	{
		// The scene master sends the notification down the chain so all workers receive the message.
		BdDesignExchangeUnpublishedDown(designId);
	}
	else if(m_tClientChannel)
	{
		// Workers and Managers propagate the message up the server chain to the master.
		// Queue a msg for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_BdDesignExchangeUnpublished;
		out << designId;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::BdDesignExchangeUnpublishedDown(quint32 designId)
{
	if(g_Au.IsSceneWorker())
	{
		// Worker server signals db thread to mark the ship design Deleted if it is already loaded.
		DgData& gd = g_Au.LockBdExchangeForWrite();
		gd.m_BdExchange->remove(designId);
		g_Au.UnlockBdExchange();
	}
	else
	{
		// Broadcast a notification to all sub servers.
		emit BdDesignExchangeUnpublishedSignal(designId);
	}
}

void AuSceneServerApp::EmpireRecordPatent(quint32 empireId, quint16 patentCID, quint8 patentQ)
{
	if(g_Au.IsSceneMaster())
	{
		// Scene master updates the empire record.
		DgData& gd = g_Au.LockDataForWrite();
		bool anyChange = gd.EmpireRecordPatent(empireId, (COMMODITYID)patentCID, patentQ);
		g_Au.UnlockData();
		if(anyChange)
			emit ChangeEmpireInDb(empireId);					// Notify the server farm that an empire record changed.
	}
	else if(m_tClientChannel)
	{
		// Subordinate non-master servers propagate the notification up to the master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_EmpireRecordPatent;
		out << empireId;
		out << patentCID;
		out << patentQ;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::AtomManagerLoadReset()
{
	if(!g_Au.AtomManagerDispatch())
		return;

	// Tally sum of all milliseconds used by atom manager threads since the last reset.
	quint64 usedMs = g_Au.AtomManagerDispatch()->UsedMsReset();

	// Elapsed time since the last reset.
	qint64 elapsedMs = m_tmBalance.elapsed();
	m_tmBalance.restart();

	// Calculate load per manager since the last reset.
	double load = (double)usedMs / (double)elapsedMs;
	load /= m_AtomManagerCount;

	// Store recent loads for averages.
	m_AtomMgrLoads[m_AtomMgrLoadIndex] = load;
	m_AtomMgrLoadIndex = (m_AtomMgrLoadIndex + 1) % 60;
	if(!m_AtomMgrLoadsValid && m_AtomMgrLoads[m_AtomMgrLoadIndex] >= 0)
		m_AtomMgrLoadsValid = true;	// All entries in the array are now valid.
}

float AuSceneServerApp::AtomManagerLoadAverage() const
{
	if(!m_AtomMgrLoadsValid)
		return 0;
	float avg = 0;
	for(int l=0; l<60; l++)
		avg += m_AtomMgrLoads[l];
	avg /= 60;
	return avg;
}

void AuSceneServerApp::UnloadUnusedData()
{
	QSet<quint32> bdDesigns;			// Bd building designs.
	QSet<quint32> scDesigns;			// Sc spacecraft designs.
	AuEther& ether = g_Au.LockEtheriumDelete();
	const DaUniverse* universe = ether.FindUniverse();
	if(universe)
		universe->ListDesignsRecursive(bdDesigns, scDesigns);
	g_Au.UnlockEtheriumDelete();
	if(!universe)
		return;	// No universe yet. DtDb is likely still loading ship designs and we need to wait, not hang on a mutex.
	g_AuDb.ListDesignsInUse(bdDesigns, scDesigns);

	// Insure the designs used by pirates remain loaded.
	QList<quint32> pirateBd, pirateSc;
	const DgData& gd = g_Au.LockDataForRead();
	gd.PirateBdBlueprints(pirateBd);
	gd.PirateScBlueprints(pirateSc);
	g_Au.UnlockData();
	for(QList<quint32>::const_iterator pi = pirateBd.cbegin(); pi != pirateBd.cend(); pi++)
		bdDesigns.insert(*pi);
	for(QList<quint32>::const_iterator pi = pirateSc.cbegin(); pi != pirateSc.cend(); pi++)
		scDesigns.insert(*pi);

	// Insure designs used by stories remain loaded.
	const AuScripts* scripts = g_Au.LockScripts();
	if(scripts)
	{
		scripts->ListDesignsNeeded(bdDesigns, scDesigns);

		// Debug test
		QSet<quint32> dbgbdDesigns;			// Bd building designs.
		QSet<quint32> dbgscDesigns;			// Sc spacecraft designs.
		scripts->ListDesignsNeeded(dbgbdDesigns, dbgscDesigns);
		//if(!dbgbdDesigns.empty())
			//qDebug() << "Stories need bdDesigns" << dbgbdDesigns;
		//if(!dbgscDesigns.empty())
			//qDebug() << "Stories need scDesigns" << dbgscDesigns;
	}
	g_Au.UnlockScripts();

	// Publicly available exchange designs can remain loaded so we don't beat ourself up loading and unloading them.
	// Without this code, exchange designs get unloaded as soon as the person logs off who asked for them. That behavior might be
	// wanted some day?
	{
		const DgData& bdx = g_Au.LockBdExchangeForRead();
		AuBdDesignExchange bdlib;
		bdx.m_BdExchange->ListPublic(bdlib);	// List designs available to most everybody (neutral or better).
		g_Au.UnlockBdExchange();
		for(AuBdDesignExchange::const_iterator bli = bdlib.cbegin(); bli != bdlib.cend(); bli++)
			bdDesigns.insert((*bli).DesignId());

		const DgData& sdx = g_Au.LockScExchangeForRead();
		AuScDesignExchange sdlib;
		sdx.m_ScExchange->ListPublic(sdlib);	// List designs available to most everybody (neutral or better).
		g_Au.UnlockScExchange();
		for(AuScDesignExchange::const_iterator sli = sdlib.cbegin(); sli != sdlib.cend(); sli++)
			scDesigns.insert((*sli).DesignId());
	}

	// Scene clients know what designs they are using.
	m_ChatServer.ListDesignsInUse(bdDesigns, scDesigns);

	// See if there is anything to be deleted before locking things for write.
	bool bdToDelete = false;
	const DgData& bdd = g_Au.LockBdDesignsForRead();
	if(bdd.m_BdDesigns && bdd.m_BdDesigns->size() > (quint32)bdDesigns.size())
		bdToDelete = true;				// We have more designs than those in use.
	g_Au.UnlockBdDesigns();

	bool scToDelete = false;
	const DgData& sdd = g_Au.LockScDesignsForRead();
	if(sdd.m_ScDesigns && sdd.m_ScDesigns->size() > (quint32)scDesigns.size())
		scToDelete = true;				// We have more designs than those in use.
	g_Au.UnlockScDesigns();

	if(!bdToDelete && !scToDelete)
		return;							// No designs need deleting.

	if(bdToDelete)
	{
		DgData& bdd = g_Au.LockBdDesignsForWrite();
		AuBdDesignHash::iterator di = bdd.m_BdDesigns->begin();
		while(di != bdd.m_BdDesigns->end())
		{
			if(bdDesigns.contains((*di).first))
				di++;
			else
			{
				// Unload the design.
				//qDebug() << "AuSceneServerApp::UnloadUnusedData Unloading bd design" << (*di).first << (*di).second.Name();
				di = bdd.m_BdDesigns->erase(di);
			}
		}
		g_Au.UnlockBdDesigns();
	}

	if(scToDelete)
	{
		DgData& sdd = g_Au.LockScDesignsForWrite();
		AuScDesignHash::iterator di = sdd.m_ScDesigns->begin();
		while(di != sdd.m_ScDesigns->end())
		{
			if(scDesigns.contains((*di).first))
				di++;
			else
			{
				// Unload the design.
				//qDebug() << "AuSceneServerApp::UnloadUnusedData Unloading sc design" << (*di).first << (*di).second.Name();
				di = sdd.m_ScDesigns->erase(di);
			}
		}
		g_Au.UnlockScDesigns();
	}
}

void AuSceneServerApp::ReportMemoryUsage()
{
	QStringList mem = ReportMemoryUsage(false);
	for(QStringList::const_iterator si = mem.cbegin(); si != mem.cend(); si++)
		qDebug() << *si;
}

QStringList AuSceneServerApp::ReportMemoryUsage(bool noLocks)
{
	QStringList mem;
	mem << QString("App %L1 bytes").arg(sizeof(AuSceneServerApp));

	//AuGLModelCache*	m_ModelCache;
	quint64 mgr = 0;
	quint64 mgrCount = 0;
	if(!noLocks)
		m_rwAtomManagers.lockForRead();
	mgr += (sizeof(AuId) + sizeof(AuMySector)) * m_MySectors.size();
	mgrCount = m_MySectors.size() + m_MySystems.size();
	mgr += (sizeof(AuId) + sizeof(AuAtomManaged*) + sizeof(AuAtomManaged)) * m_MySystems.size();
	if(!noLocks)
		m_rwAtomManagers.unlock();
	mem << QString("Managed %L1 atoms using %L2 bytes").arg(mgrCount).arg(mgr);

	if(!noLocks)
		m_muPendingStargates.lock();
	quint64 starg = (sizeof(AuId) + sizeof(AuOpenStargate)) * m_PendingStargates.size();
	mem << QString("Stargates %L1 gates using %L2 bytes").arg(m_PendingStargates.size()).arg(starg);
	if(!noLocks)
		m_muPendingStargates.unlock();

	if(!noLocks)
		m_muPendingDep.lock();
	quint64 dep = (sizeof(AuId) + sizeof(AuAtomDeparture)) * m_PendingDepartures.size();
	mem << QString("Departures %L1 atoms using %L2 bytes").arg(m_PendingDepartures.size()).arg(dep);
	if(!noLocks)
		m_muPendingDep.unlock();

	quint64 ass = 0;
	if(!noLocks)
		m_AssignmentsLock.lock();
	ass += (sizeof(AuId) + sizeof(unsigned int)) * m_Reassignments.size();
	ass += (sizeof(AuId) + sizeof(unsigned int)) * m_Revokations.size();
	ass += (sizeof(AuId) + sizeof(AuSectorAssignment)) * m_SectorAssignments.size();
	ass += (sizeof(AuId) + sizeof(AuSystemAssignment)) * m_SystemAssignments.size();
	if(!noLocks)
		m_AssignmentsLock.unlock();
	mem << QString("Assignments %L1 bytes").arg(ass);

	if(!noLocks)
		m_HandoffMutex.lock();
	quint64 hand = sizeof(AuHandoffRequest) * m_Handoffs.size();
	if(!noLocks)
		m_HandoffMutex.unlock();
	mem << QString("Handoffs %L1 bytes").arg(hand);

	quint64 spawn = 0;
	if(!noLocks)
		m_SpawnMutex.lock();
	spawn += sizeof(AuSpawnRequest) * m_Spawns.size();
	spawn += sizeof(AuHengeTeleportRequest) * m_HengeTeleports.size();
	spawn += (sizeof(AuId) + sizeof(AuSpawnArrival)) * m_ArrivingSpawnsOnPlanets.size();
	spawn += (sizeof(AuId) + sizeof(AuSpawnDeparture)) * m_DepartingSpawnsOnPlanets.size();
	if(!noLocks)
		m_SpawnMutex.unlock();
	mem << QString("Spawns %L1 bytes").arg(spawn);

	quint64 aq = 0;
	if(!noLocks)
		m_QueryMutex.lock();
	aq += sizeof(AuAtomQuery) * m_AtomQueries.size();
	aq += sizeof(AuHengeTeleportDestQuery) * m_HengeTeleportDestQueries.size();
	if(!noLocks)
		m_QueryMutex.unlock();
	mem << QString("Atom Queries %L1 bytes").arg(aq);

	quint64 arena = 0;
	/*
	if(!noLocks)
		m_ArenaLock.lockForRead();
	arena = (sizeof(AuId) + sizeof(AuArenaReady)) * m_Arenas.size();
	if(!noLocks)
		m_ArenaLock.unlock();
	mem << QString("Arenas %L1 bytes").arg(arena);
	*/

	if(!noLocks)
		m_PlayerStatusLock.lockForRead();
	quint64 plon = sizeof(AuPlayerOnlineStatus) * m_PlayerStatus.capacity();
	if(!noLocks)
		m_PlayerStatusLock.unlock();
	mem << QString("Player Status %L1 bytes").arg(plon);

	mem << QString("DgData");
	if(!noLocks)
		g_Au.LockDataForRead();
	quint64 data = m_Data->MemoryUsage(mem);
	if(!noLocks)
		g_Au.UnlockData();

	AuEther& ether = g_Au.LockEtheriumDelete();
	quint64 atomMem = 0;
	quint64 atoms = 0;
	QHash<DaAtom::ATOM_TYPE, quint64> memByType;
	QHash<DaAtom::ATOM_TYPE, quint64> atomsByType;
	QList<AuId> atomIds = ether.AtomIds();
	for(QList<AuId>::const_iterator ai = atomIds.cbegin(); ai != atomIds.cend(); ai++)
	{
		DaAtom* atom = ether.FindAtom(*ai);
		if(atom)
		{
			quint64 mem = atom->MemoryUsage();
			atomMem += mem;
			atoms++;
			DaAtom::ATOM_TYPE atomType = atom->AtomType();
			QHash<DaAtom::ATOM_TYPE, quint64>::iterator ti = memByType.find(atomType);
			if(ti == memByType.end())
				memByType.insert(atomType, mem);
			else
				(*ti) += mem;
			ti = atomsByType.find(atomType);
			if(ti == atomsByType.end())
				atomsByType.insert(atomType, 1);
			else
				(*ti)++;
		}
	}
	g_Au.UnlockEtheriumDelete();
	mem << QString("Atoms %L1 using %L2 bytes").arg(atoms).arg(atomMem);
	for(QHash<DaAtom::ATOM_TYPE, quint64>::const_iterator ti = memByType.cbegin(); ti != memByType.cend(); ti++)
	{
		QHash<DaAtom::ATOM_TYPE, quint64>::iterator ai = atomsByType.find(ti.key());
		mem << QString("  Type %1 %2 %L3 atoms using %L4 bytes")
			.arg(ti.key())
			.arg(DaAtom::AtomTypeName(ti.key()))
			.arg(*ai)
			.arg(*ti);
	}

	quint64 models = 0;
	quint64 modelTotal = 0;
	if(m_ModelCache)
	{
		models = m_ModelCache->capacity();
		modelTotal = m_ModelCache->MemoryUsage();
	}
	mem << QString("3D Models %L1 using %L2 bytes").arg(models).arg(modelTotal);

	quint64 total = data + mgr + starg + dep + ass + hand + spawn + aq + arena + plon + atomMem + modelTotal;
	mem << QString("Total %L1 bytes").arg(total);
	return mem;
}

void AuSceneServerApp::SpacecraftPositionReport(AuId shipId, AuId systemId, AuStarMapSpacecraft pos)
{
	if(g_Au.IsSceneMaster())
	{
		QMultiHash<AuId, AuStarMapScPos>& scpos = g_Au.LockScPositionReports();
		QMultiHash<AuId, AuStarMapScPos>::iterator si = scpos.begin();
		while(si != scpos.end())
		{
			if((*si).IsExpired() || (*si).Id() == shipId)
				si = scpos.erase(si);	// Remove the one I am updating and expired ones as we find them.
			else
				si++;
		}
		scpos.insert(systemId, AuStarMapScPos(shipId, pos));	// Add a new one.
		g_Au.UnlockScPositionReports();
	}
	else
	{
		// Subordinate non-master servers propagate the notification up to the master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_SpacecraftPosition;
		out << shipId;
		out << systemId;
		out << pos;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::ReloadScript(quint32 scriptId)
{
	if(!g_Au.IsSceneWorker())
		return;
	if(scriptId == 0)
		g_AuDb.LoadScripts();	// Load and validate story scripts.
	else
		g_AuDb.LoadScript(scriptId);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
#if defined(MEM_TRACE_GLOBAL)
//static
quint64 AuSceneServerApp::AllocatedCount()
{
	return s_AllocatedGlobal;
}

//static
quint64 AuSceneServerApp::FreedCount()
{
	return s_FreedGlobal;
}

void* operator new(size_t size)
{
	void* p = malloc(size);
	if(p)
		s_AllocatedGlobal++;
	return p;
}
 
void operator delete(void* p)
{
	if(p)
		s_FreedGlobal++;
	free(p);
}
#endif
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// This begins the trail, when an avatar elects to collect the bounty on an item in their possession, i.e. an avatar head.
// This comes to us via a signal from DtChatChannel. We can be reasonably certain the bounty hunter is on line atm, and ok.
void AuSceneServerApp::CollectBounty(AuId bountyHunter, QString bountyHunterName, AuGearPos gearPos, AuBountyOnItem bountyOnItem)
{
	if(!g_Au.IsSceneWorker())
		return;
	// Locate the victim to take the bounty from them. Check to see if they are in my local data.
	bool victimFound = false;
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaPerson* person = dynamic_cast<DaPerson*>(ether.FindNormalAtom(bountyHunter));
	DaUnit* unit = dynamic_cast<DaUnit*>(ether.FindNormalAtom(bountyOnItem.Id()));
	if(person && unit)
	{
		// The victim unit was found. Do they have any bounties?
		victimFound = true;
		if(unit->TakeItemBounty(bountyHunterName, bountyOnItem))	// Take the bounty.
			person->BountyCollected(gearPos, bountyOnItem);			// Give it to the bounty hunter.
		else
			person->BountyExpired(gearPos, bountyOnItem);			// Invalidate the bounty on the item.
	}
	g_Au.UnlockEtheriumDelete();
	if(!victimFound)
		CollectBountyUp(bountyHunter, bountyHunterName, gearPos, bountyOnItem);
}

void AuSceneServerApp::CollectBountyUp(AuId bountyHunter, QString bountyHunterName, AuGearPos gearPos, AuBountyOnItem bountyOnItem)
{
	if(m_tClientChannel)
	{
		// Queue a message for my boss manager server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_CollectBounty;
		out << bountyHunter;
		out << bountyHunterName;
		out << gearPos;
		out << bountyOnItem;
		m_tClientChannel->SendPacket(ba);
	}
	else
		emit CollectBountySignal(bountyHunter, bountyHunterName, gearPos, bountyOnItem);	// Master propagates the message down.
}

void AuSceneServerApp::CollectBountyDown(AuId bountyHunter, QString bountyHunterName, AuGearPos gearPos,
		AuBountyOnItem bountyOnItem)
{
	if(g_Au.IsSceneWorker())
	{
		// Locate the victim to take a bounty from them. Check to see if they are in my local data.
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaUnit* unit = dynamic_cast<DaUnit*>(ether.FindNormalAtom(bountyOnItem.Id()));
		if(unit)
		{
			// The victim unit was found. Do they have any bounties?
			if(unit->TakeItemBounty(bountyHunterName, bountyOnItem))		// Take the bounty.
				PayBountyUp(bountyHunter, gearPos, bountyOnItem, true);		// Send payment to bounty hunter.
			else
				PayBountyUp(bountyHunter, gearPos, bountyOnItem, false);	// Bounty hunter must invalidate the bounty on the item.
		}
		g_Au.UnlockEtheriumDelete();
	}
	else
		emit CollectBountySignal(bountyHunter, bountyHunterName, gearPos, bountyOnItem);	// Propagate the message down.
}

void AuSceneServerApp::PayBountyUp(AuId bountyHunter, AuGearPos gearPos, AuBountyOnItem bountyOnItem, bool victimHadBounty)
{
	if(m_tClientChannel)
	{
		// Queue a message for my boss manager server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_PayBounty;
		out << bountyHunter;
		out << gearPos;
		out << bountyOnItem;
		out << victimHadBounty;
		m_tClientChannel->SendPacket(ba);
	}
	else
		PayBountyDown(bountyHunter, gearPos, bountyOnItem, victimHadBounty);
}

void AuSceneServerApp::PayBountyDown(AuId bountyHunter, AuGearPos gearPos, AuBountyOnItem bountyOnItem, bool victimHadBounty)
{
	if(g_Au.IsSceneWorker())
	{
		// Locate the victim to take a bounty from them. Check to see if they are in my local data.
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaPerson* person = dynamic_cast<DaPerson*>(ether.FindNormalAtom(bountyHunter));
		if(person)
		{
			if(victimHadBounty)
				person->BountyCollected(gearPos, bountyOnItem);	// Give it to the bounty hunter.
			else
				person->BountyExpired(gearPos, bountyOnItem);	// Invalidate the bounty on the item.
		}
		g_Au.UnlockEtheriumDelete();
	}
	else
		emit PayBountySignal(bountyHunter, gearPos, bountyOnItem, victimHadBounty);	// Propagate the message down.
}

void AuSceneServerApp::BlueprintRatedBdUp(AuExchangeRatingRec rating)
{
	if(g_Au.IsSceneMaster())
		BlueprintRatedBdDown(rating);
	else
	{
		// Queue a message for my boss manager server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_BlueprintRatedBd;
		out << rating;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::BlueprintRatedScUp(AuExchangeRatingRec rating)
{
	if(g_Au.IsSceneMaster())
		BlueprintRatedScDown(rating);
	else
	{
		// Queue a message for my boss manager server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_BlueprintRatedSc;
		out << rating;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::BlueprintRatedBdDown(AuExchangeRatingRec rating)
{
	if(g_Au.IsSceneWorker())
	{
		// Rate my local Bd blueprint. This is for my local cache. It has already been saved to the db.
		DgData& sdi = g_Au.LockBdExchangeForWrite();
		sdi.m_BdExchangeRatings->AddReview(rating);
		g_Au.UnlockBdExchange();
	}
	else
		emit BlueprintRatedBdSignal(rating);
}

void AuSceneServerApp::BlueprintRatedScDown(AuExchangeRatingRec rating)
{
	if(g_Au.IsSceneWorker())
	{
		// Rate my local Sc blueprint. This is for my local cache. It has already been saved to the db.
		DgData& sdi = g_Au.LockScExchangeForWrite();
		sdi.m_ScExchangeRatings->AddReview(rating);
		g_Au.UnlockScExchange();
	}
	else
		emit BlueprintRatedScSignal(rating);
}

// A ship has triggered a potential pirate base encounter in their sector by emitting a signal that connects here.
void AuSceneServerApp::SectorPirateBaseEncounter(AuId encSectorId, AuId encSystemId)
{
	if(g_Au.IsSceneManager())
	{
		// The manager is in charge of the sector.
		// Insure the sector wasn't revoked from me before I received this message from my sub worker.
		if(IsSectorAssignedToMe(encSectorId))
		{
			// I still have the sector in my domain.
			AuId baseSystem;
			AuEther& ether = g_Au.LockEtheriumDelete();
			DaSector* sector = dynamic_cast<DaSector*>(ether.FindNormalAtom(encSectorId));
			if(sector)
				baseSystem = sector->PirateBaseEncounter(encSystemId);	// Returns id of system that will create a new pirate base.
			g_Au.UnlockEtheriumDelete();
			if(!baseSystem.IsNull())
				emit PirateBaseEncountered(baseSystem);
		}
	}
	else if(m_tClientChannel)
	{
		// Queue a message for my boss manager server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_SectorPirateBase;
		out << encSectorId;
		out << encSystemId;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::PirateBaseEncounteredDown(AuId inSystemId)
{
	Q_ASSERT(g_Au.IsSceneWorker());
	if(!IsSolarSystemAssignedToMe(inSystemId))
		return;	// All workers subordinate to a manager get this message. The one with the system is expected to do the work.
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(inSystemId));
	if(system)
		system->PirateBaseEncounter();
	g_Au.UnlockEtheriumDelete();
}

void AuSceneServerApp::EmpireChgBdPrefUp(quint32 empireId, int industry, quint32 designId)
{
	if(g_Au.IsSceneMaster())
	{
		AuBlueprintPref bp(designId, BENV_All);	//DHH so far all pirate building designs work in all environments.
		AuBdPref pref;
		pref.AddBlueprint(bp);
		DgData& gd = g_Au.LockDataForWrite();
		gd.SetEmpirePreferredBlueprints(empireId, (INDUSTRYID)industry, pref);
		g_Au.UnlockData();
		emit ChangeEmpireInDb(empireId);	// Propagate a change notification to the rest of the server farm.
	}
	else if(m_tClientChannel)
	{
		// Queue a message for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_EmpireChg;
		out << (quint8)0;
		out << empireId;
		out << (quint16)industry;
		out << designId;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::EmpireChgScPrefUp(quint32 empireId, int scrole, quint32 designId)
{
	if(g_Au.IsSceneMaster())
	{
		AuScPref pref;
		pref.AddBlueprint(designId);
		DgData& gd = g_Au.LockDataForWrite();
		gd.SetEmpirePreferredBlueprints(empireId, (AuScPrefs::SCROLE)scrole, pref);
		g_Au.UnlockData();
		emit ChangeEmpireInDb(empireId);	// Propagate a change notification to the rest of the server farm.
	}
	else if(m_tClientChannel)
	{
		// Queue a message for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_EmpireChg;
		out << (quint8)1;
		out << empireId;
		out << (quint8)scrole;
		out << designId;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::ClaimTitleUp(AuId propertyId, QString propertyName, AuId ownerId, QString ownerName, quint32 ownerEmpire)
{
	if(g_Au.IsSceneMaster())
	{
		ClaimTitleDown(propertyId, propertyName, ownerId, ownerName, ownerEmpire);
	}
	else
	{
		// Queue a message for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_RedeemShipTitle;
		out << propertyId;
		out << propertyName;
		out << ownerId;
		out << ownerName;
		out << ownerEmpire;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::ClaimTitleDown(AuId propertyId, QString propertyName, AuId ownerId, QString ownerName, quint32 ownerEmpire)
{
	if(!g_Au.IsSceneWorker())
	{
		emit ClaimTitleSignal(propertyId, propertyName, ownerId, ownerName, ownerEmpire);
	}
	else
	{
		//qDebug() << "AuSceneServerApp::ClaimTitleDown property" << propertyId << "owner" << ownerId << ownerName << ownerEmpire;
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaUniverse* universe = ether.FindUniverse();
		if(universe)
			universe->ClaimTitleRecursive(propertyId, propertyName, ownerId, ownerName, ownerEmpire);
		g_Au.UnlockEtheriumDelete();
	}
}

// This is where the trail begins when an agent teleports to a galaxy location or when someone enters a one way intergalactic
// wormhole.
// This is not where spawning happens.
// If the origin and destination solar systems are in my locally managed data, the transfer happens immediately right here.
void AuSceneServerApp::AtomTraveledGalaxyLoc(AuId traveler, AuId origSystem, AuId destGalaxyId, AuPoint destGalaxyLoc)
{
	if(g_Au.m_LogMessages & LOG_Travel)
		qDebug() << "AuSceneServerApp::AtomTraveledGalaxyLoc traveler" << traveler << "origSystem" << origSystem << "destGalaxy"
			<< destGalaxyId << "destGalaxyLoc" << destGalaxyLoc;
	// Insure the originating system hasn't been revoked from me already.
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaAtom* trav = ether.FindLiveAtom(traveler);
	if(!trav)
	{
		if(g_Au.m_LogMessages & LOG_Error)
			qWarning() << "AuSceneServerApp::AtomTraveledGalaxyLoc traveler" << traveler << "atom not found"
				<< "destGalaxyId" << destGalaxyId << "destGalaxyLoc" << destGalaxyLoc;
		g_Au.UnlockEtheriumDelete();
		return;
	}
	if(trav->IsTraveling())
	{
		if(g_Au.m_LogMessages & LOG_Error)
			qWarning() << "AuSceneServerApp::AtomTraveledGalaxyLoc traveler" << *trav << "is already traveling"
				<< "destGalaxyId" << destGalaxyId << "destGalaxyLoc" << destGalaxyLoc;
		g_Au.UnlockEtheriumDelete();
		return;
	}

	// See if we can identify the destination system locally.
	DaGalaxy* dGalaxy = dynamic_cast<DaGalaxy*>(ether.FindNormalAtom(destGalaxyId));
	if(!dGalaxy)
	{
		if(g_Au.m_LogMessages & LOG_Error)
			qWarning() << "AuSceneServerApp::AtomTraveledGalaxyLoc traveler" << *trav << "origSystem" << origSystem
				<< "destGalaxyId not found" << destGalaxyId << "destGalaxyLoc" << destGalaxyLoc;
		g_Au.UnlockEtheriumDelete();
		return;
	}

	// The traveller is on their way at this point, either locally or via the database.
	trav->SetTraveling();

	AuId destSystem;
	DaSolarSystem* destSys = 0;
	AuPoint destSystemLoc;
	{
		AuPoint destSectorLoc(destGalaxyLoc);
		destSectorLoc.Round(10);
		DaSector* sector = dGalaxy->FindSectorAt(destSectorLoc);
		if(sector)
		{
			destSystemLoc = destGalaxyLoc - destSectorLoc;	// Location relative to the sector.
			destSys = sector->FindClosestSolarSystem(destSystemLoc);
			if(destSys)
				destSystem = destSys->Id();
		}
	}

	// If the destination solar system is in my data, I can perform the transfer right here and now.
	bool atomTraveled = false;
	if(!IsSolarSystemAssignedToMe(destSystem))
		destSys = 0;
	if(destSys && !destSys->IsOnlyStarFieldLoaded())
	{
		if(g_Au.m_LogMessages & LOG_Travel)
		{
			QString logmsg = QString("Atom %1 %2 traveled from system %3 to system %5 %6 to reach galaxy loc (%7, %8, %9)")
				.arg(trav->Id().GetID()).arg(trav->Name())
				.arg(origSystem.GetID())
				.arg(destSys->Id().GetID()).arg(destSys->Name())
				.arg(destGalaxyLoc.x, 0,'f',3)
				.arg(destGalaxyLoc.y, 0,'f',3)
				.arg(destGalaxyLoc.z, 0,'f',3);
			g_Au.Log(logmsg);
		}

		// Add the traveling atom to its destination solar system.
		AuMatrix4x4 t;
		t.Translate(destSys->Transform().UnProject(destSystemLoc));	// Location relative to the solar system.
		trav->ForcePosStop(AuAtomPos(destSystem, t));
		if(destSys->FindPersonOnLineRecursive())
			destSys->SetLastPersonVisited();
		if(destSys->FindOfficerRecursive() || destSys->FindAIPersonRecursive())
			destSys->SetLastOfficerVisited();
		destSys->SetModified();

		// Kick the travelling atom into motion.
		trav->SetNormal();						// Return to 'Normal' non-traveling state.
		trav->SetModified();

		// Mark the destination solar system as explored by the empire owning the travelling unit.
		DaUnit* unit = dynamic_cast<DaUnit*>(trav);
		if(unit)
			destSys->ExploredBy(unit->EmpireId());

		// Notify the system of the quality of non-pirate ships. We will use QualityMade because the ship design may
		// not be analyzed yet; HighestSystemQuality would be slightly preferable.
		const DaVessel* ship = dynamic_cast<const DaVessel*>(trav);
		if(ship && !ship->IsPirate())
			destSys->SetNonPirateShipPresent(ship->QualityMade());

		atomTraveled = true;
	}
	g_Au.UnlockEtheriumDelete();
	if(atomTraveled)
		return;	// The traveling atom was handed off locally. We're done.

	// The traveling atom could not be handed off locally. It will need to be saved in the database and then the
	// destination server notified.
	if(g_Au.m_LogMessages & LOG_Travel)
		qDebug("AuSceneServerApp::AtomTraveledGalaxyLoc traveler %d departed via db", traveler.GetID());
	emit TravelerDepartedGalaxyLocViaDb(traveler, origSystem, destGalaxyId, destGalaxyLoc);
}

// Db thread has finished saving the 'traveler'.
void AuSceneServerApp::AtomTraveledGalaxyLocUp(AuId traveler, AuId destGalaxy, AuPoint destGalaxyLoc)
{
	if(g_Au.m_LogMessages & LOG_Travel)
		qDebug() << "AuSceneServerApp::AtomTraveledGalaxyLocUp traveler" << traveler << "destGalaxy" << destGalaxy
			<< "destGalaxyLoc" << destGalaxyLoc;
	if(g_Au.IsSceneMaster())
	{
		// Master server manages universe atom.
		m_muPendingDep.lock();
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaUniverse* universe = ether.FindUniverse();
		if(universe)
			AtomTraveledGalaxyLocUp2(traveler, destGalaxy, destGalaxyLoc, universe);
		g_Au.UnlockEtheriumDelete();
		m_muPendingDep.unlock();
	}
	else if(g_Au.IsSceneManager())
	{
		// Manager server manages sector atoms.
		m_muPendingDep.lock();
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaGalaxy* galaxy = dynamic_cast<DaGalaxy*>(ether.FindNormalAtom(destGalaxy));
		AuId destSector;
		if(galaxy)
		{
			AuPoint sectorLoc(destGalaxyLoc);
			sectorLoc.Round(10);
			DaSector* sector = galaxy->FindSectorAt(sectorLoc);
			if(sector)
				destSector = sector->Id();
		}
		DaSector* sector;
		if(!destSector.IsNull() && IsSectorAssignedToMe(destSector))
			sector = dynamic_cast<DaSector*>(ether.FindNormalAtom(destSector));
		else
			sector = 0;
		AtomTraveledGalaxyLocUp2(traveler, destGalaxy, destGalaxyLoc, sector);	// 'sector' allows for shortcut down to worker
		g_Au.UnlockEtheriumDelete();
		m_muPendingDep.unlock();
	}
	else
	{
		// Worker server sends the notification up to its manager.
		AtomTraveledGalaxyLocUp2(traveler, destGalaxy, destGalaxyLoc, 0);
	}
}

// m_muPendingDep must already be locked on master and manager
void AuSceneServerApp::AtomTraveledGalaxyLocUp2(const AuId& id, const AuId& destGalaxy, const AuPoint& destGalaxyLoc,
		DaAtom* managedAtom)
{
	if(g_Au.m_LogMessages & LOG_Travel)
		qDebug() << "AuSceneServerApp::AtomTraveledGalaxyLocUp2 id" << id << "destGalaxy" << destGalaxy << "destGalaxyLoc"
			<< destGalaxyLoc << "managedAtom" << *managedAtom;
	if(g_Au.IsSceneMaster())
	{
		// I only get this message when an atom has departed a worker server and its manager is not assigned the destination sector.
		// The manager propagates the departure message up to me and I need to pass it back down so it reaches the scene manager
		// responsible for the destination sector.
		// In the case of intergalactic wormhole travel, the destSector is actually a dest galaxy.

		// Find the destination sector.
		DaGalaxy* galaxy = dynamic_cast<DaGalaxy*>(managedAtom->FindSubatomId(destGalaxy));
		if(galaxy)
		{
			AuPoint sectorLoc(destGalaxyLoc);
			sectorLoc.Round(10);

			// Make sure the cube is there in case the sector decayed or never existed. Agents use this.
			m_rwNewSectors.lockForWrite();
			DaSector* sector = galaxy->CreateSectorCubeAt(sectorLoc, m_NewSectors);
			m_rwNewSectors.unlock();
			g_AuDb.PostCriticalUpdates();

			if(sector)
			{
				AuPoint systemLoc = destGalaxyLoc - sectorLoc;
				DaSolarSystem* system = sector->FindClosestSolarSystem(systemLoc);
				if(system)
					AtomTraveledDown2(id, system->Id(), sector->Id(), managedAtom);
			}
		}
	}
	else
	{
		// If the destination system is in my sphere of control, I can simply notify the worker server that is receiving the atom.
		if(g_Au.IsSceneManager() && managedAtom)
		{
			// Manager server manages sector atoms.
			AuPoint sectorLoc(destGalaxyLoc);
			sectorLoc.Round(10);
			AuPoint systemLoc = destGalaxyLoc - sectorLoc;
			DaSolarSystem* system = ((DaSector*)managedAtom)->FindClosestSolarSystem(systemLoc);
			if(system && system->IsFinal())
			{
				if(AtomTraveledDown2(id, system->Id(), managedAtom->Id(), managedAtom))
				{
					//qDebug("AuSceneServerApp::AtomTraveledUp2 traveler %d destSystem %d destSector %d "
							//"took shortcut to worker",
							//id.GetID(), destSystem.GetID(), destSector.GetID());
					return;		// Transferred the atom to a subordinate worker server.
				}
			}
		}

		// The destination solar system is not in my sphere of control.
		if(m_tClientChannel)
		{
			// Propagate the atom traveled message up to my boss server.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_AtomTravelingGalaxyLoc;
			out << id;
			out << destGalaxy;
			out << destGalaxyLoc;
			m_tClientChannel->SendPacket(ba);
		}
	}
}

void AuSceneServerApp::AvatarAccountActive(AuIdList land)
{
	if(g_Au.IsSceneMaster())
	{
		AvatarAccountActiveDown(land);
	}
	else
	{
		// Queue a message for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_AvatarAccountActive;
		out << land;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::AvatarAccountActiveDown(AuIdList land)
{
	if(!g_Au.IsSceneWorker())
	{
		emit AvatarAccountActiveSignal(land);
	}
	else
	{
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaUniverse* universe = ether.FindUniverse();
		if(universe)
			universe->AvatarAccountActiveRecursive(land);
		g_Au.UnlockEtheriumDelete();
	}
}

void AuSceneServerApp::ItDesignDeactivatedDown(quint32 designId)
{
	//QString logmsg;
	//logmsg.sprintf("AuSceneServerApp::ItDesignDeactivatedDown design %d", designId);
	//g_Au.Log(logmsg);
	if(g_Au.IsSceneWorker())
	{
		// Worker server signals db thread to mark the ship design Deleted if it is already loaded.
		DgData& gd = g_Au.LockItDesignsForWrite();
		gd.ItDesignDeactivated(designId);
		g_Au.UnlockItDesigns();
	}
	else
	{
		// Broadcast a notification to all sub servers.
		emit ItDesignDeactivatedSignal(designId);
	}
}

void AuSceneServerApp::ItDesignDeactivatedUp(quint32 designId)
{
	//QString logmsg;
	//logmsg.sprintf("AuSceneServerApp::ItDesignDeactivatedUp design %d", designId);
	//g_Au.Log(logmsg);
	if(g_Au.IsSceneMaster())
	{
		// The scene master sends the notification down the chain so all workers receive the message.
		ItDesignDeactivatedDown(designId);
	}
	else if(m_tClientChannel)
	{
		// Workers and Managers propagate the message up the server chain to the master.
		// Queue a msg for my boss server, it could be a manager or a master.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_ItDesignDeactivated;
		out << designId;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::ProposeTrade(AuTradeAgreement ta)
{
	if(g_Au.IsSceneMaster())
	{
		// Scene master sends the proposal down to the appropriate manager.
		ProposeTradeDown(ta);
		return;
	}
	else if(g_Au.IsSceneManager())
	{
		// Scene master sends the proposal down to the appropriate worker if the destination sector is assigned to me.
		if(IsSectorAssignedToMe(ta.DestinationSector()))
		{
			ProposeTradeDown(ta);
			return;
		}
	}
	else
	{
		// Scene worker sends the proposal down to the appropriate city atom if the destination system is assigned to me.
		if(IsSolarSystemAssignedToMe(ta.DestinationSystem()))
		{
			ProposeTradeDown(ta);
			return;
		}
	}

	// Send the proposal up to my boss, either a manager or master.
	if(m_tClientChannel)
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_Trade;
		out << (quint8)1;
		out << ta;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::ProposeTradeDown(AuTradeAgreement ta)
{
	if(g_Au.IsSceneMaster())
	{
		// Master server.
		DhSceneServer toServer = SectorServer(ta.DestinationSector());
		if(toServer.IsValid())
		{
			// Send the proposal to the manager who hosts the destination sector.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_Trade;
			out << (quint8)1;
			out << ta;
			SendPacketTo(ba, toServer.m_Id);
		}
	}
	else if(g_Au.IsSceneManager())
	{
		// Manager server.
		DhSceneServer toServer = SolarSystemServer(ta.DestinationSystem());
		if(toServer.IsValid())
		{
			// Send the proposal to the worker who hosts the destination system.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_Trade;
			out << (quint8)1;
			out << ta;
			SendPacketTo(ba, toServer.m_Id);
		}
	}
	else
	{
		// Worker server.
		// I expect to find the destination in my local data. If it's not here, the proposal is lost. We don't need these
		// things bouncing around forever looking for a city that is gone.
		int result = 1;
		AuId idSystem(ta.DestinationSystem());
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaSolarSystem* system = AssignedSolarSystem(idSystem, ether);
		if(system)
		{
			DaBd* recipient = dynamic_cast<DaBd*>(system->FindIdRecursive(ta.DestinationCity()));
			if(recipient)
				result = recipient->TradeAgreementDeliver(ta);				// Proposal finally reaches the destination here.
		}
		g_Au.UnlockEtheriumDelete();
		if(result)
			DeactivateTrade(ta, AuTradeAgreement::Undeliverable, false);	// Deactivate the proposal at its origin.
	}
}

void AuSceneServerApp::DeactivateTrade(AuTradeAgreement ta, quint8 nuState, bool atDest)
{
	if(g_Au.IsSceneMaster())
	{
		// Scene master sends the proposal down to the appropriate manager.
		DeactivateTradeDown(ta, nuState, atDest);
		return;
	}
	else if(g_Au.IsSceneManager())
	{
		// Scene master sends the proposal down to the appropriate worker if the destination sector is assigned to me.
		if(atDest)
		{
			if(IsSectorAssignedToMe(ta.DestinationSector()))
			{
				DeactivateTradeDown(ta, nuState, atDest);
				return;
			}
		}
		else
		{
			if(IsSectorAssignedToMe(ta.OriginSector()))
			{
				DeactivateTradeDown(ta, nuState, atDest);
				return;
			}
		}
	}
	else
	{
		// Scene worker sends the proposal down to the appropriate city atom if the destination system is assigned to me.
		if(atDest)
		{
			if(IsSolarSystemAssignedToMe(ta.DestinationSystem()))
			{
				DeactivateTradeDown(ta, nuState, atDest);
				return;
			}
		}
		else
		{
			if(IsSolarSystemAssignedToMe(ta.OriginSystem()))
			{
				DeactivateTradeDown(ta, nuState, atDest);
				return;
			}
		}
	}

	// Send the proposal up to my boss, either a manager or master.
	if(m_tClientChannel)
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_Trade;
		out << (quint8)2;
		out << ta;
		out << nuState;
		out << atDest;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::DeactivateTradeDown(AuTradeAgreement ta, quint8 nuState, bool atDest)
{
	if(g_Au.IsSceneMaster())
	{
		// Master server.
		DhSceneServer toServer;
		if(atDest)
			toServer = SectorServer(ta.DestinationSector());
		else
			toServer = SectorServer(ta.OriginSector());
		if(toServer.IsValid())
		{
			// Send the proposal to the manager who hosts the destination sector.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_Trade;
			out << (quint8)2;
			out << ta;
			out << nuState;
			out << atDest;
			SendPacketTo(ba, toServer.m_Id);
		}
	}
	else if(g_Au.IsSceneManager())
	{
		// Manager server.
		DhSceneServer toServer;
		if(atDest)
			toServer = SolarSystemServer(ta.DestinationSystem());
		else
			toServer = SolarSystemServer(ta.OriginSystem());
		if(toServer.IsValid())
		{
			// Send the proposal to the worker who hosts the destination system.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_Trade;
			out << (quint8)2;
			out << ta;
			out << nuState;
			out << atDest;
			SendPacketTo(ba, toServer.m_Id);
		}
	}
	else
	{
		// Worker server.
		// I expect to find the destination in my local data. If it's not here, the proposal is lost. We don't need these
		// things bouncing around forever looking for a city that is gone.
		AuId idSystem;
		if(atDest)
			idSystem = ta.DestinationSystem();
		else
			idSystem = ta.OriginSystem();
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaSolarSystem* system = AssignedSolarSystem(idSystem, ether);
		if(system)
		{
			DaBd* recipient;
			if(atDest)
				recipient = dynamic_cast<DaBd*>(system->FindIdRecursive(ta.DestinationCity()));
			else
				recipient = dynamic_cast<DaBd*>(system->FindIdRecursive(ta.OriginCity()));
			if(recipient)
				recipient->TradeAgreementDeactivate(ta, (AuTradeAgreement::STATE)nuState);	// Deactivate with inactive status.
		}
		g_Au.UnlockEtheriumDelete();
	}
}

void AuSceneServerApp::AcceptTrade(AuTradeAgreement ta)
{
	if(g_Au.IsSceneMaster())
	{
		// Scene master sends the proposal down to the appropriate manager.
		AcceptTradeDown(ta);
		return;
	}
	else if(g_Au.IsSceneManager())
	{
		// Scene master sends the proposal down to the appropriate worker if the destination sector is assigned to me.
		if(IsSectorAssignedToMe(ta.OriginSector()))
		{
			AcceptTradeDown(ta);
			return;
		}
	}
	else
	{
		// Scene worker sends the proposal down to the appropriate city atom if the destination system is assigned to me.
		if(IsSolarSystemAssignedToMe(ta.OriginSystem()))
		{
			AcceptTradeDown(ta);
			return;
		}
	}

	// Send the proposal up to my boss, either a manager or master.
	if(m_tClientChannel)
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_Trade;
		out << (quint8)3;
		out << ta;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::AcceptTradeDown(AuTradeAgreement ta)
{
	if(g_Au.IsSceneMaster())
	{
		// Master server.
		DhSceneServer toServer = SectorServer(ta.OriginSector());
		if(toServer.IsValid())
		{
			// Send the proposal to the manager who hosts the destination sector.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_Trade;
			out << (quint8)3;
			out << ta;
			SendPacketTo(ba, toServer.m_Id);
		}
	}
	else if(g_Au.IsSceneManager())
	{
		// Manager server.
		DhSceneServer toServer = SolarSystemServer(ta.OriginSystem());
		if(toServer.IsValid())
		{
			// Send the proposal to the worker who hosts the destination system.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_Trade;
			out << (quint8)3;
			out << ta;
			SendPacketTo(ba, toServer.m_Id);
		}
	}
	else
	{
		// Worker server.
		// I expect to find the destination in my local data. If it's not here, the proposal is lost. We don't need these
		// things bouncing around forever looking for a city that is gone.
		AuId idSystem(ta.OriginSystem());
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaSolarSystem* system = AssignedSolarSystem(idSystem, ether);
		if(system)
		{
			DaBd* recipient = dynamic_cast<DaBd*>(system->FindIdRecursive(ta.OriginCity()));
			if(recipient)
				recipient->TradeAgreementAccept(ta, false);			// Destination acceptance finally reaches the origin here.
		}
		g_Au.UnlockEtheriumDelete();
	}
}

void AuSceneServerApp::TradeReadyToShip(AuId readyCapitolId, AuTradeAgreement ta)
{
	if(g_Au.IsSceneMaster())
	{
		// Scene master sends the proposal down to the appropriate manager.
		TradeReadyToShipDown(readyCapitolId, ta);
		return;
	}
	else if(g_Au.IsSceneManager())
	{
		// Scene master sends the proposal down to the appropriate worker if the destination sector is assigned to me.
		if(readyCapitolId == ta.OriginCity())
		{
			if(IsSectorAssignedToMe(ta.DestinationSector()))
			{
				TradeReadyToShipDown(readyCapitolId, ta);
				return;
			}
		}
		else if(readyCapitolId == ta.DestinationCity())
		{
			if(IsSectorAssignedToMe(ta.OriginSector()))
			{
				TradeReadyToShipDown(readyCapitolId, ta);
				return;
			}
		}
	}
	else
	{
		// Scene worker sends the proposal down to the appropriate city atom if the destination system is assigned to me.
		if(readyCapitolId == ta.OriginCity())
		{
			if(IsSolarSystemAssignedToMe(ta.DestinationSystem()))
			{
				TradeReadyToShipDown(readyCapitolId, ta);
				return;
			}
		}
		else if(readyCapitolId == ta.DestinationCity())
		{
			if(IsSolarSystemAssignedToMe(ta.OriginSystem()))
			{
				TradeReadyToShipDown(readyCapitolId, ta);
				return;
			}
		}
	}

	// Send the proposal up to my boss, either a manager or master.
	if(m_tClientChannel)
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_Trade;
		out << (quint8)4;
		out << readyCapitolId;
		out << ta;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::TradeReadyToShipDown(AuId readyCapitolId, AuTradeAgreement ta)
{
	if(g_Au.IsSceneMaster())
	{
		// Master server.
		DhSceneServer toServer;
		if(readyCapitolId == ta.OriginCity())
			toServer = SectorServer(ta.DestinationSector());
		else if(readyCapitolId == ta.DestinationCity())
			toServer = SectorServer(ta.OriginSector());
		if(toServer.IsValid())
		{
			// Send the proposal to the manager who hosts the destination sector.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_Trade;
			out << (quint8)4;
			out << readyCapitolId;
			out << ta;
			SendPacketTo(ba, toServer.m_Id);
		}
	}
	else if(g_Au.IsSceneManager())
	{
		// Manager server.
		DhSceneServer toServer = SolarSystemServer(ta.DestinationSystem());
		if(toServer.IsValid())
		{
			// Send the proposal to the worker who hosts the destination system.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_Trade;
			out << (quint8)4;
			out << readyCapitolId;
			out << ta;
			SendPacketTo(ba, toServer.m_Id);
		}
	}
	else
	{
		// Worker server.
		// I expect to find the destination in my local data. If it's not here, the proposal is lost. We don't need these
		// things bouncing around forever looking for a city that is gone.
		AuId idSystem;
		if(readyCapitolId == ta.OriginCity())
			idSystem = ta.DestinationSystem();
		else if(readyCapitolId == ta.DestinationCity())
			idSystem = ta.OriginSystem();
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaSolarSystem* system = AssignedSolarSystem(idSystem, ether);
		if(system)
		{
			DaBd* recipient;
			if(readyCapitolId == ta.OriginCity())
				recipient = dynamic_cast<DaBd*>(system->FindIdRecursive(ta.DestinationCity()));
			else if(readyCapitolId == ta.DestinationCity())
				recipient = dynamic_cast<DaBd*>(system->FindIdRecursive(ta.OriginCity()));
			else
				recipient = 0;
			if(recipient)
			{
				// Signal finally reaches the destination here.
				// It will ship any corresponding trade shipments that are ready and emit the TradeShipped signal when appropriate.
				recipient->TradeReadyToShipNow(ta);
			}
		}
		g_Au.UnlockEtheriumDelete();
	}
}

void AuSceneServerApp::TradeShipped(AuId shipperCapitolId, AuTradeAgreement ta)
{
	if(g_Au.IsSceneMaster())
	{
		// Scene master sends the proposal down to the appropriate manager.
		TradeShippedDown(shipperCapitolId, ta);
		return;
	}
	else if(g_Au.IsSceneManager())
	{
		// Scene master sends the proposal down to the appropriate worker if the destination sector is assigned to me.
		if(shipperCapitolId == ta.OriginCity())
		{
			if(IsSectorAssignedToMe(ta.DestinationSector()))
			{
				TradeShippedDown(shipperCapitolId, ta);
				return;
			}
		}
		else if(shipperCapitolId == ta.DestinationCity())
		{
			if(IsSectorAssignedToMe(ta.OriginSector()))
			{
				TradeShippedDown(shipperCapitolId, ta);
				return;
			}
		}
	}
	else
	{
		// Scene worker sends the proposal down to the appropriate city atom if the destination system is assigned to me.
		if(shipperCapitolId == ta.OriginCity())
		{
			if(IsSolarSystemAssignedToMe(ta.DestinationSystem()))
			{
				TradeShippedDown(shipperCapitolId, ta);
				return;
			}
		}
		else if(shipperCapitolId == ta.DestinationCity())
		{
			if(IsSolarSystemAssignedToMe(ta.OriginSystem()))
			{
				TradeShippedDown(shipperCapitolId, ta);
				return;
			}
		}
	}

	// Send the proposal up to my boss, either a manager or master.
	if(m_tClientChannel)
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtServerChannel::P_Trade;
		out << (quint8)5;
		out << shipperCapitolId;
		out << ta;
		m_tClientChannel->SendPacket(ba);
	}
}

void AuSceneServerApp::TradeShippedDown(AuId shipperCapitolId, AuTradeAgreement ta)
{
	if(g_Au.IsSceneMaster())
	{
		// Master server.
		DhSceneServer toServer;
		if(shipperCapitolId == ta.OriginCity())
			toServer = SectorServer(ta.DestinationSector());
		else if(shipperCapitolId == ta.DestinationCity())
			toServer = SectorServer(ta.OriginSector());
		if(toServer.IsValid())
		{
			// Send the proposal to the manager who hosts the destination sector.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_Trade;
			out << (quint8)5;
			out << shipperCapitolId;
			out << ta;
			SendPacketTo(ba, toServer.m_Id);
		}
	}
	else if(g_Au.IsSceneManager())
	{
		// Manager server.
		DhSceneServer toServer = SolarSystemServer(ta.DestinationSystem());
		if(toServer.IsValid())
		{
			// Send the proposal to the worker who hosts the destination system.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtServerChannel::P_Trade;
			out << (quint8)5;
			out << shipperCapitolId;
			out << ta;
			SendPacketTo(ba, toServer.m_Id);
		}
	}
	else
	{
		// Worker server.
		// I expect to find the destination in my local data. If it's not here, the proposal is lost. We don't need these
		// things bouncing around forever looking for a city that is gone.
		AuId idSystem;
		if(shipperCapitolId == ta.OriginCity())
			idSystem = ta.DestinationSystem();
		else if(shipperCapitolId == ta.DestinationCity())
			idSystem = ta.OriginSystem();
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaSolarSystem* system = AssignedSolarSystem(idSystem, ether);
		if(system)
		{
			DaBd* recipient;
			if(shipperCapitolId == ta.OriginCity())
				recipient = dynamic_cast<DaBd*>(system->FindIdRecursive(ta.DestinationCity()));
			else if(shipperCapitolId == ta.DestinationCity())
				recipient = dynamic_cast<DaBd*>(system->FindIdRecursive(ta.OriginCity()));
			else
				recipient = 0;
			if(recipient)
			{
				// Signal finally reaches the destination here.
				// It will ship any corresponding trade shipments that are ready and emit the TradeShipped signal when appropriate.
				recipient->TradeShippedNow(ta);
			}
		}
		g_Au.UnlockEtheriumDelete();
	}
}

void AuSceneServerApp::ServerBroadcast(QString text)
{
	if(g_Au.IsSceneMaster())
	{
		AuCommAnnouncement msg(text);
		ChatUp(msg, 0);
		if(g_Au.m_LogMessages & LOG_Log)
			g_Au.Log(text);
	}
}

