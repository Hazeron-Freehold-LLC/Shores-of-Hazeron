//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auapp.cpp,v 1.177 2022/05/05 16:08:17 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auapp.h"
//
#include "auatommanager.h"
#include "auclientglobal.h"
#include "auclientinterface.h"
#include "aucomm.h"
#include "aucreateempire.h"
#include "aufilever.h"
#include "auframe.h"
#include "auglmodelmaker.h"
#include "aulogincomm.h"
#include "aumodel.h"
#include "auParticleSystem.h"
#include "auscparts.h"
#include "auscreport.h"
#include "ausound.h"
#include "austylesheet.h"
#include "auview.h"
#include "daanimal.h"
#include "daarena.h"
#include "daasteroid.h"
#include "daatomlist.h"
#include "dadesigner.h"
#include "dagalaxy.h"
#include "daitem.h"
#include "danpc.h"
#include "daperson.h"
#include "dasc.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dauniverse.h"
#include "davehicle.h"
#include "dgdata.h"
#include "dicontrolinput.h"
#include "dpbody.h"
#include "dtccpacket.h"
#include "dtclient.h"
#include "dtclientchat.h"
#include "dtsspacket.h"
//#include <exception>
#if defined(Q_OS_WIN)
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#include <signal.h>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QSysInfo>

#define d_AtlantisDesignId				655903

#define SOLO_AVATAR_VER					0x01					// Av file containing solitaire avatar save.
#define SOLO_GAME_VER					0x02					// Db file containing solitaire game save.
static const QString				s_LoginFile("A1001100");	// 'A' followed by an ASCII L in 7 bit binary
static const QString				s_UniverseFile("A1010101");	// 'A' followed by an ASCII U in 7 bit binary
static const QString				s_LoginPrefix("sA.");		// Prefix for checkpoint save login file names.
static const QString				s_UniversePrefix("sU.");	// Prefix for checkpoint save universe file names.

AuApp::AuApp(int& argc, char** argv) : QApplication(argc, argv), m_KeyFilter(this)
{
	// Create 'client mode' global data object independent of app object.
	m_Data = new DgData;
	m_Etherium = new AuEther("Hazeron");

	m_DeferType = DeferNone;

	m_eState = NotLoggedIn;
	m_Frame = 0;
	m_SoloAvatarDeleted = false;

	m_AL = 0;
	m_AudioSource = 0;
	for(int cs=0; cs<CLIENT_SOUNDS; cs++)
	{
		m_Audio[cs] = 0;
		m_Sound[cs] = SOUND_None;
	}

	m_I = new AuClientInterface;
	connect(m_I, SIGNAL(AcceptTrade(AuTradeAgreement)),
			SLOT(AcceptTrade(AuTradeAgreement)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(AdminGoTo(AuId, AuId, quint8, bool, AuId)),
			SLOT(AdminGoTo(AuId, AuId, quint8, bool, AuId)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(AdminGoToGalaxyLoc(AuId, AuId, AuId, AuPoint)),
			SLOT(AtomTraveledGalaxyLoc(AuId, AuId, AuId, AuPoint)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(BdDesignExchangePublished(AuBdDesignExchangeEntry)),
			SLOT(BdDesignExchangePublished(AuBdDesignExchangeEntry)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(BdDesignExchangeUnpublished(quint32)),
			SLOT(BdDesignExchangeUnpublished(quint32)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(BlueprintRatedBd(AuExchangeRatingRec)),
			SLOT(BlueprintRatedBd(AuExchangeRatingRec)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(BlueprintRatedSc(AuExchangeRatingRec)),
			SLOT(BlueprintRatedSc(AuExchangeRatingRec)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(BountyPosted(AuId, QString, AuId, QString, double, int)),
			SLOT(BountyPosted(AuId, QString, AuId, QString, double, int)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(CityCreated(quint32, quint8, AuId, QString, AuId, QString, AuId, QString, AuId, QString, AuPoint, AuId,
					QString, AuId, quint8)),
			SLOT(CityCreated(quint32, quint8, AuId, QString, AuId, QString, AuId, QString, AuId, QString, AuPoint, AuId,
					QString, AuId, quint8)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(CityMailRequestedSignal(AuId, AuId, quint32)),
			SLOT(CityMailRequestedSignal(AuId, AuId, quint32)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(CollectBounty(AuId, QString, AuGearPos, AuBountyOnItem)),
			SLOT(CollectBounty(AuId, QString, AuGearPos, AuBountyOnItem)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(DeactivateTrade(AuTradeAgreement, quint8, bool)),
			SLOT(DeactivateTrade(AuTradeAgreement, quint8, bool)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(EmpireChgBdPref(quint32, int, quint32)),
			SLOT(EmpireChgBdPref(quint32, int, quint32)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(EmpireChgFlag(quint32, QImage)),
			SLOT(EmpireChgFlag(quint32, QImage)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(EmpireChgScPref(quint32, int, quint32)),
			SLOT(EmpireChgScPref(quint32, int, quint32)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(EstablishNewEmpire(AuId, QString, quint8, quint32, QString, QImage)),
			SLOT(EstablishNewEmpire(AuId, QString, quint8, quint32, QString, QImage)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(GovBossChanged(quint32, AuId, AuId)),
			SLOT(GovBossChanged(quint32, AuId, AuId)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(GovDossierAddDocument(quint32, quint32, QByteArray)),
			SLOT(GovDossierAddDocument(quint32, quint32, QByteArray)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(GovDossierChangeState(quint32, quint32, AuTime, quint8, quint16, quint8)),
			SLOT(GovDossierChangeState(quint32, quint32, AuTime, quint8, quint16, quint8)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(GovSetAnnouncements(quint32, QString)),
			SLOT(GovSetAnnouncements(quint32, QString)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(GovSetCapital(quint32, AuId, AuSpecies, AuId, int)),
			SLOT(GovSetCapitalUp(quint32, AuId, AuSpecies, AuId, int)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(GovSetCommerce(quint32, QByteArray)),
			SLOT(GovSetCommerce(quint32, QByteArray)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(GovSetContrailColor(quint32, quint8, quint8)),
			SLOT(GovSetContrailColor(quint32, quint8, quint8)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(GovSetDefaultPoliticalStance(quint32, quint8)),
			SLOT(GovSetDefaultPoliticalStance(quint32, quint8)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(GovSetJoinPassword(quint32, QByteArray)),
			SLOT(GovSetJoinPasswordUp(quint32, QByteArray)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(GovSetPoliticalStance(quint32, quint32, quint8, quint8, AuId, QString, quint8, AuId, QString, quint8,
					quint8, QString, QString, QString)),
			SLOT(SetPoliticalStance(quint32, quint32, quint8, quint8, AuId, QString, quint8, AuId, QString, quint8,
					quint8, QString, QString, QString)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(GovSetTax(quint32, quint8, quint8, quint8)),
			SLOT(GovSetTax(quint32, quint8, quint8, quint8)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(GovSetThemeColor(quint32, quint8, QColor)),
			SLOT(GovSetThemeColor(quint32, quint8, QColor)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(GovSetWarCriteria(quint32, AuWarCriteria)),
			SLOT(GovSetWarCriteria(quint32, AuWarCriteria)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(LeaveEmpire(AuId, QString, quint32)),
			SLOT(LeaveEmpire(AuId, QString, quint32)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(NameSpecies(AuSpecies, QString, AuId)),
			SLOT(NameSpecies(AuSpecies, QString, AuId)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(OfficeAssigned(AuId, quint32, AuOffice)),
			SLOT(OfficeAssigned(AuId, quint32, AuOffice)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(OfficeRevoked(AuId, quint32)),
			SLOT(OfficeRevoked(AuId, quint32)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(ProposeTrade(AuTradeAgreement)),
			SLOT(ProposeTrade(AuTradeAgreement)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(ReleaseBerth(AuId, AuId)),
			SLOT(ReleaseBerthUp(AuId, AuId)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(RenameAvatar(AuId, QString)),
			SLOT(RenameAvatar(AuId, QString)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(RenameCity(AuId, QString)),
			SLOT(RenameCity(AuId, QString)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(RenamePlanet(AuId, QString, AuId, QString, AuId, QString)),
			SLOT(RenamePlanet(AuId, QString, AuId, QString, AuId, QString)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(RenameSector(AuId, QString)),
			SLOT(RenameSector(AuId, QString)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(RenameSystem(AuId, QString)),
			SLOT(RenameSystem(AuId, QString)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(ScDesignExchangePublished(AuScDesignExchangeEntry)),
			SLOT(ScDesignExchangePublished(AuScDesignExchangeEntry)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(ScDesignExchangeUnpublished(quint32)),
			SLOT(ScDesignExchangeUnpublished(quint32)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(SendFunds(AuId, QString, AuId, double, quint8, quint8, quint32)),
			SLOT(SendFundsUp(AuId, QString, AuId, double, quint8, quint8, quint32)),
			Qt::QueuedConnection);
	connect(m_I, SIGNAL(SpacecraftStasisRecover(AuId, AuId, quint8, bool)),
			SLOT(SpacecraftStasisRecover(AuId, AuId, quint8, bool)),
			Qt::QueuedConnection);

	m_Model = new AuModel;
	//DHH can this go away? connect(m_Model, SIGNAL(ControlPacketReady()), SLOT(SendControlPackets()));

	m_HumanDNA.InitDefaultHuman();
	m_HumanGender = Male;
	m_HumanHand = RightHanded;
	m_HumanName = tr("Human");

	m_InitSyncDone = false;
	m_JobOfferLast.start();		// Start the clock on job offer acceptance.
	m_SyncDone = false;
	m_ModelCache = 0;
	m_NewGalaxy = ShoresOfHazeron;
	m_NewProximity = 0;
	m_SceneConnectedOnce = false;
	m_tChatChannel = 0;		// Client communication with server.
	m_tSceneChannel = 0;	// Client communication with server.
	m_UdpSequence = 0;
	m_wasConnected = false;	// Until our run state indicates we're connected.
	
	// Parse command line.
	bool setStyle = true;
	QStringList args = arguments();
	for(QStringList::const_iterator it = args.constBegin(); it != args.constEnd(); it++)
		if(*it == "-nostyle")
			setStyle = false;

	// Set style
	AuStyleSheet::SetStyle(!setStyle);

	// Install app-wide event filter.
	connect(&m_KeyFilter, SIGNAL(KeyPressed(QKeyEvent*)), SIGNAL(KeyPressed(QKeyEvent*)));
	connect(&m_KeyFilter, SIGNAL(KeyReleased(QKeyEvent*)), SIGNAL(KeyReleased(QKeyEvent*)));
	installEventFilter(&m_KeyFilter);

	// Initialize the Simple Directmedia Layer (SDL).
	if(SDL_Init(SDL_INIT_JOYSTICK) < 0)
		qWarning("SDL failed to initialize. Simple Directmedia Layer will not be available.");

	connect(this, SIGNAL(aboutToQuit()), SLOT(PrepareToQuit()));
	connect(this, SIGNAL(SoloChatReceived(QByteArray)), SLOT(ChatReceived(QByteArray)), Qt::QueuedConnection);
	connect(this, SIGNAL(SoloChannelOpened(QString, quint8, QString)), SLOT(ChannelOpened(QString, quint8, QString)));
}

AuApp::~AuApp()
{
	delete m_I;

	// Sound
	if(m_AL)
		delete m_AL;

	//qDebug() << "AuApp::~AuApp m_Model";
	if(m_Model)
		delete m_Model;

	//qDebug() << "AuApp::~AuAppm m_Etherium";
	if(m_Etherium)
		delete m_Etherium;

	//qDebug() << "AuApp::~AuApp m_ModelCache";
	if(m_ModelCache)
		delete m_ModelCache;

	//qDebug() << "AuApp::~AuApp m_Data";
	// Delete data and root atom, which were used by gp_AuGlobal.
	if(m_Data)
		delete m_Data;

	//qDebug() << "AuApp::~AuApp gp_AuGlobal";
	// Delete global object.
	if(gp_AuGlobal)
		delete gp_AuGlobal;

	//qDebug() << "AuApp::~AuApp SDL_Quit";
	// Shut down SDL.
	SDL_Quit();
	//qDebug() << "AuApp::~AuApp done";
}

#if !defined(Q_OS_WIN)
void AuApp::HandleSignal(int value)
#else
void AuApp::HandleSignal(int)
#endif
{
#if !defined(Q_OS_WIN)
	//qDebug() << "AuApp::HandleSignal - Caught signal" << value;
	if(value == SIGPIPE)
		return;			// Ignore broken pipe signal.
#endif

	AuFrame* frame = Frame();
	if(frame)
		frame->StopThreads();

	PrepareToQuit();

	closeAllWindows();	// Exit cleanly, letting things destruct properly.
}

void AuApp::SetFrame(AuFrame* pFrame)
{
	Q_ASSERT(pFrame);
	Q_ASSERT(m_Model);
	m_Frame = pFrame;
	connect(this, SIGNAL(AvatarsUpdated()),
			pFrame, SLOT(AvatarsUpdated()), Qt::QueuedConnection);
	connect(this, SIGNAL(BdExchangeBlueprintRating(quint32, QString, AuExchangeBlueprintRating)),
			pFrame, SLOT(BlueprintRating(quint32, QString, AuExchangeBlueprintRating)));
	connect(this, SIGNAL(BrokerShipment(AuShipment)),
			pFrame, SLOT(BrokerShipment(AuShipment)));
	connect(this, SIGNAL(BrokerShipmentsClear()),
			pFrame, SLOT(BrokerShipmentsClear()));
	connect(this, SIGNAL(BrokerTradeAgreement(AuTradeAgreement)),
			pFrame, SLOT(BrokerTradeAgreement(AuTradeAgreement)));
	connect(this, SIGNAL(BrokerTradeAgreementsClear()),
			pFrame, SLOT(BrokerTradeAgreementsClear()));
	connect(this, SIGNAL(CityHistoryReportReady(AuId, AuCityReportHistory)),
			pFrame, SLOT(CityHistoryReportReady(AuId, AuCityReportHistory)));
	connect(this, SIGNAL(CityLastReportReady(AuId, AuCityReport)),
			pFrame, SLOT(CityLastReportReady(AuId, AuCityReport)));
	connect(this, SIGNAL(HangarQueryAnswered(QString, quint32, quint32)),
			pFrame, SLOT(HangarQueryAnswered(QString, quint32, quint32)));
	connect(this, SIGNAL(JobAvailable(AuJobOffer)),
			pFrame, SLOT(JobAvailable(AuJobOffer)));
	connect(this, SIGNAL(MissionSensorContactsReceived(QByteArray)),
			pFrame, SLOT(MissionSensorContactsReceived(QByteArray)));
	connect(this, SIGNAL(OrdnanceReceived(AuId, QByteArray)),
			pFrame, SLOT(OrdnanceReceived(AuId, QByteArray)));
	connect(this, SIGNAL(ScExchangeBlueprintRating(quint32, QString, AuExchangeBlueprintRating)),
			pFrame, SLOT(BlueprintRating(quint32, QString, AuExchangeBlueprintRating)));
	connect(this, SIGNAL(TransferCitizensAvailable(QByteArray)),
			pFrame, SLOT(TransferCitizensAvailable(QByteArray)));
	connect(this, SIGNAL(TransporterPlace(AuTransporterTarget)),
			pFrame, SLOT(TransporterPlace(AuTransporterTarget)));
	connect(this, SIGNAL(TransporterShip(AuId, QString, QString)),
			pFrame, SLOT(TransporterShip(AuId, QString, QString)));
	connect(this, SIGNAL(TransporterTransporter(AuId, QString, QString, quint16)),
			pFrame, SLOT(TransporterTransporter(AuId, QString, QString, quint16)));

	connect(m_Model, SIGNAL(StarMapErased()),
			pFrame, SLOT(StarMapErased()), Qt::QueuedConnection);
	connect(m_Model, SIGNAL(StarMapAddSector(AuId)),
			pFrame, SLOT(StarMapAddSector(AuId)), Qt::QueuedConnection);
}

void AuApp::StopThreads()
{
	//qDebug() << "AuApp::StopThreads";
	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: Stop threads, disconnecting UDP and TCP connections."));
	if(m_SceneUdpSocket.state() == QAbstractSocket::ConnectedState)
	{
		m_SceneUdpSocket.disconnect(this, SLOT(ConnectUdp()));	// Let it die without trying to reconnect.
		m_SceneUdpSocket.close();
	}

	if(m_tChatChannel)
	{
		DtClientChat* chat = m_tChatChannel;
		m_tChatChannel = 0;
		chat->disconnect(this, SLOT(ConnectChat()));	// Let it die without trying to reconnect.
		chat->Kill();
		//chat->wait(1000);
		//chat->deleteLater();
	}

	if(m_tSceneChannel)
	{
		DtClient* scene = m_tSceneChannel;
		m_tSceneChannel = 0;
		scene->Kill();
		//scene->wait(1000);
		//scene->deleteLater();
	}
}

void AuApp::Logout()
{
	if(RunState() != LoggedIn &&
			RunState() != LoggedInInit &&
			RunState() != LoggedInWait &&
			RunState() != InitWait)
		return;

	// Notify scene server and change run state.
	SceneLogout();

	// Notify login server that we are logging out. This is a formality; it's not
	// really essential.
	AuLoginComm login;
	login.Logout();
}

void AuApp::StartSceneClientThread()
{
	//qDebug("AuApp::StartSceneClientThread");
	AuFrame* pFrame = Frame();
	Q_ASSERT(pFrame);

	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: Establishing client TCP connection to server %1 port %2...")
				.arg(g_Au.SceneServer()).arg(g_Au.ScenePort()));

	m_tSceneChannel = new DtClient(this);
	if(m_tSceneChannel)
	{
		connect(m_tSceneChannel, SIGNAL(ClientUnitEmpireReceived(quint32)), pFrame, SLOT(ClientUnitEmpireReceived(quint32)));
		connect(m_tSceneChannel, SIGNAL(Connected()), SLOT(ConnectionEstablished()));
		connect(m_tSceneChannel, SIGNAL(EmpireDataBegin()), pFrame, SLOT(EmpireDataBegin()));
		connect(m_tSceneChannel, SIGNAL(EmpireDataComplete()), pFrame, SLOT(EmpireDataComplete()));

		connect(m_tSceneChannel, SIGNAL(EmpireRequested(quint32)),
				SLOT(EmpireRequested(quint32)), Qt::QueuedConnection);

		connect(m_tSceneChannel, SIGNAL(StartNewAvatarError(unsigned char)), SLOT(StartNewAvatarError(unsigned char)));
		connect(m_tSceneChannel, SIGNAL(RestartNewAvatar()), SLOT(RestartNewAvatar()));
		connect(m_tSceneChannel, SIGNAL(EstDeferred(AuId)), SLOT(EstDeferred(AuId)));
		connect(m_tSceneChannel, SIGNAL(InitSucceeded(AuId)), SLOT(InitSucceeded(AuId)));
		connect(m_tSceneChannel, SIGNAL(LoggedOut()), SLOT(LoggedOut()));
		connect(m_tSceneChannel, SIGNAL(LoginFailed(quint8)), SLOT(LoginFailed(quint8)));
		connect(m_tSceneChannel, SIGNAL(BdDesignNeeded(quint32)), SLOT(BdDesignNeeded(quint32)), Qt::QueuedConnection);
		connect(m_tSceneChannel, SIGNAL(ItDesignNeeded(quint32)), SLOT(ItDesignNeeded(quint32)), Qt::QueuedConnection);
		connect(m_tSceneChannel, SIGNAL(ScDesignNeeded(quint32)), SLOT(ScDesignNeeded(quint32)), Qt::QueuedConnection);
		connect(m_tSceneChannel, SIGNAL(SceneDisconnectedToDefer()), SLOT(SceneDisconnected()));
		connect(m_tSceneChannel, SIGNAL(SceneDeferred(int)), SLOT(SceneDeferred(int)));
		connect(m_tSceneChannel, SIGNAL(SceneLoginSucceeded()), SLOT(SceneLoggedIn()));
		//connect(m_tSceneChannel, SIGNAL(StarMapErased()), pFrame, SLOT(StarMapErased()), Qt::QueuedConnection);
		//connect(m_tSceneChannel, SIGNAL(StarMapAddSector(AuId)), pFrame, SLOT(StarMapAddSector(AuId)), Qt::QueuedConnection);
		connect(m_tSceneChannel, SIGNAL(SwitchAvatarTransferred(AuId)), pFrame, SLOT(SwitchAvatarTransferred(AuId)),
				Qt::QueuedConnection);
		m_tSceneChannel->start(QThread::LowPriority);
	}
	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: Client TCP connection established to server %1 port %2.")
				.arg(g_Au.SceneServer()).arg(g_Au.ScenePort()));
}

void AuApp::TestClientConnection()
{
	// If the client thread has not been allocated yet, we've never been connected.
	if(!m_tSceneChannel)
		return;

	// If the client thread is no longer running, we've lost our connection.
	if(!m_tSceneChannel->isRunning())
	{
#if !defined(QT_NO_DEBUG)
		if(m_wasConnected)
			qDebug() << tr("The connection to the server was lost. Retrying...");
		else
			qDebug() << tr("Failed to connect to the server. Retrying...");
#endif

		StopThreads();
		g_Au.ResetSceneServer();
		StartSceneClientThread();
	}
}

// Used by the client to send an array of bytes to the server in the tcp stream.
void AuApp::QueueScenePacket(const QByteArray& ba)
{
	if(!m_tSceneChannel)
	{
		qWarning() << "warning: AuApp::QueueScenePacket size" << ba.size() << "bytes, no connection";
		return;
	}

	// The packet is already formatted. Add it to the transmit data stream.
	QByteArray& txbuf = LockSceneTcpBuffer("AuApp::QueueScenePacket");
	AuUtil::WritePacketSize(txbuf, ba.size());
	txbuf.append(ba);
	int txbufsize = txbuf.size();
	UnlockSceneTcpBuffer("AuApp::QueueScenePacket");
	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		g_Au.Log(g_Au.CacheApp() + "/soh.txt",
				QString("App: TCP packet sent on client channel, %1 bytes; tx buffer contains %2 bytes.")
				.arg(ba.size()).arg(txbufsize));
}

// Used by the client to send an array of bytes to the server in the tcp stream.
void AuApp::QueueChatPacket(const QByteArray& ba)
{
	if(!m_tChatChannel)
	{
		//qWarning() << "warning: AuApp::QueueChatPacket size" << ba.size() << "bytes, no connection";
		return;
	}

	// The packet is already formatted. Add it to the transmit data stream.
	QByteArray& txbuf = LockChatTcpBuffer("AuApp::QueueChatPacket");
	AuUtil::WritePacketSize(txbuf, ba.size());
	txbuf.append(ba);
	int txbufsize = txbuf.size();
	UnlockChatTcpBuffer("AuApp::QueueChatPacket");
	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		g_Au.Log(g_Au.CacheApp() + "/soh.txt",
				QString("App: TCP packet sent on chat channel, %1 bytes; tx buffer contains %2 bytes.")
				.arg(ba.size()).arg(txbufsize));
}

// Used by the client to send an array of bytes to the server as a datagram.
void AuApp::SendUdpPacket(const QByteArray& ba)
{
	m_muSceneUdpBuffer.lock();
	if(m_SceneUdpSocket.state() == QAbstractSocket::UnconnectedState)
	{
		// Attempt to make the connection.
		ConnectUdp();
		if(m_SceneUdpSocket.state() == QAbstractSocket::UnconnectedState)
		{
			m_muSceneUdpBuffer.unlock();
			if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
				g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: UDP socket failed to connect."));
			return;	// We are still unconnected.
		}
	}

	if(m_SceneUdpSocket.state() != QAbstractSocket::ConnectedState)
	{
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: UDP socket state %1 is not connected")
					.arg(m_SceneUdpSocket.state()));
	}

	// Wait for all bytes to be written before sending another packet, so we don't flood the channel.
	if(m_SceneUdpSocket.bytesToWrite() >= 1024)
	{
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt",
					QString("App: UDP socket waiting to send %1 bytes, flushing").arg(m_SceneUdpSocket.bytesToWrite()));
		m_SceneUdpSocket.flush();
		m_muSceneUdpBuffer.unlock();
		return;
	}

	// The packet is already formatted.
	// Package the packet into an outer envelope so the server knows which player it belongs to.
	const DgData& gd = g_Au.LockDataForRead();
	unsigned int idPlayer = gd.PlayerId();
	g_Au.UnlockData();

	QByteArray datagram;
	QDataStream out(&datagram, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << idPlayer;
	out << ba;

	m_SceneUdpSocket.write(datagram);
	m_muSceneUdpBuffer.unlock();
	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: UDP packet sent, %1 bytes").arg(datagram.count()));
}

void AuApp::ReadPendingDatagrams()
{
	//qDebug("AuApp::ReadPendingDatagrams");
	while(m_SceneUdpSocket.hasPendingDatagrams())
	{
		QByteArray datagram;
		datagram.resize(m_SceneUdpSocket.pendingDatagramSize());
		QHostAddress sender;
		quint16 senderPort;
		m_SceneUdpSocket.readDatagram(datagram.data(), datagram.size(),
				&sender, &senderPort);

		/*DHH I don't think we get datagrams any more.
		// The only datagram we get from the server is a person update for our dude.
		if(m_tSceneChannel)
		{
			//qDebug("AuApp::ReadPendingDatagrams - got one");
			m_tSceneChannel->ReadPacket(datagram);
		}
		*/
	}
}

int AuApp::TransmitChatBuffer(QTcpSocket& tcpSocket)
{
	// We cannot send anything unless the socket is connected.
	if(tcpSocket.state() != QAbstractSocket::ConnectedState)
		return 0;

	int written = 0;
	QByteArray& tba = LockChatTcpBuffer("AuApp::TransmitChatBuffer");
	if(!tba.isEmpty())
	{
		// There is data to be sent. It is already formatted into packets with block sizes.
		g_Au.SetNet1Uploading(100);
		written = tcpSocket.write(tba);
		tba.clear();
	}
	UnlockChatTcpBuffer("AuApp::TransmitChatBuffer");
	return written;
}

int AuApp::TransmitSceneBuffer(QTcpSocket& tcpSocket)
{
	// We cannot send anything unless the socket is connected.
	if(tcpSocket.state() != QAbstractSocket::ConnectedState)
		return 0;

	int written = 0;
	QByteArray& tba = LockSceneTcpBuffer("AuApp::TransmitSceneBuffer");
	if(!tba.isEmpty())
	{
		// There is data to be sent. It is already formatted into packets with block sizes.
		g_Au.SetNet2Uploading(100);
		written = tcpSocket.write(tba);
		tba.clear();
	}
	UnlockSceneTcpBuffer("AuApp::TransmitSceneBuffer");
	return written;
}

void AuApp::SetRunState(RUNSTATE state)
{
	//qDebug("AuApp::SetRunState - %d", state);
	m_eState = state;

	// This affects the mode of the global data.
	switch(m_eState)
	{
	case NotLoggedIn:
	case LoggedIn:
	case LoggedInInit:
	case LoggedInWait:
	case InitWait:
		g_Au.SetMode(AuGlobal::SceneClient);
		break;
	case Solitaire:
	case Designer:
		g_Au.SetMode(AuGlobal::SceneSolo);
		if(!g_Au.AtomManagerDispatch())
		{
			g_Au.SetAtomManagerDispatch(new AuAtomManagerDispatch(g_Au.AtomManagerCount(), this));
			AuEther& ether = g_Au.LockEtheriumDelete();
			if(!ether.Empty())
			{
				if(g_Au.m_LogMessages & LOG_Debug)
					g_Au.Log(QObject::tr("Commencing the course of time."));
				ether.TickTock();
			}
			g_Au.UnlockEtheriumDelete();
		}
		break;
	}
}

void AuApp::SetDefaultHuman(const QString& name, HANDEDNESS hand, GENDER gender, const AuDNA& dna)
{
	m_HumanDNA = dna;
	m_HumanGender = gender;
	m_HumanHand = hand;
	m_HumanName = name;
}

void AuApp::StartNewAvatarError(unsigned char reason)
{
	QString msg = tr("Attempt to establish a new empire failed for the following reason.\n\n");
	switch(reason)
	{
	case 0:
		msg += tr("Success");
		break;
	case 1:
		msg += tr("Player is not authorized to create empires.");
		break;
	case 2:
		msg += tr("The avatar name is invalid. A valid avatar name must be less than %1 characters and "
				"it must not contain certain reserved characters such as | and newline.").arg(MaxNameLength()+1);
		break;
	case 3:
		msg += tr("The choice of gender is invalid.");
		break;
	case 4:
		msg += tr("The proximity choice is invalid.");
		break;
	case 5:
		msg += tr("The empire name is invalid. A valid empire name must be less than %1 characters and "
				"it must not contain certain reserved characters such as | and newline.").arg(MaxNameLength()+1);
		break;
	case 6:
		msg += tr("The join policy is invalid.");
		break;
	case 7:
		msg += tr("The flag image is invalid.");
		break;
	case 9:
		msg += tr("An empire with that name already exists.");
		break;
	case 10:
		msg += tr("The universe is missing.");
		break;
	case 11:
		msg += tr("The galaxy is missing.");
		break;
	case 12:
		msg += tr("Failed to create or locate a new sector.");
		break;
	case 13:
		// Note we never get this error message here because it turns into a call to RestartNewAvatar.
		msg += tr("Failed to create a good home solar system.");
		break;
	case 14:
		msg += tr("Failed to locate a habitable world.");
		break;
	case 15:
		msg += tr("Failed to locate assigned homeworld.");
		break;
	case 16:
		msg += tr("Failed to locate a new sector. Player should try again.");
		break;
	case 17:
		msg += tr("Invalid sector.");
		break;
	case 18:
		msg += tr("Invalid planet.");
		break;
	case 19:
		msg += tr("Invalid avatar DNA.");
		break;
	case 20:
		msg += tr("Invalid galaxy.");
		break;
	case 21:
		msg += tr("Invalid DNA.");
		break;
	case 22:
		msg += tr("Join password is incorrect.");
		break;
	default:
		msg += tr("The server reported error number %1. "
				"I am unfamiliar with that error so I cannot provide an explanation of the problem. "
				"An update to this software may be needed.")
			.arg(reason);
		break;
	}
	QMessageBox::warning(Frame(), g_Au.ApplicationName(), msg);

	// This function would only be called if we were trying to establish our
	// first empire. Resetting this state will cause AuView to offer the init ui again.
	SetRunState(LoggedInInit);
}

// We receive this call from AuView after AuLogin has successfully logged in to the login server.
void AuApp::LoginSucceeded()
{
	//qDebug("AuApp::LoginSucceeded");
	Frame()->CloseSettingsUi();

	// When the login is successful, change to LoggedInInit state. This tells the frame to display the ui with the player's
	// list of avatars so they can select one or start a new one.
	SetRunState(LoggedInInit);

	StartSceneClientThread();
}

// Call this function with the unit id chosen for the scene. The id can be null when establishing a new empire.
// This starts the process of logging in to the scene servers, starting with the master server. The master will
// defer us to a manager who will then defer us to the worker who is hosting our active unit.
// The first packet in the stream when we connect to a scene server (master, manager, or worker) is expected to be PL_Login.
void AuApp::StartSceneCommunications(AuId personId)
{
	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: Start scene communications"));

	// Select the current active unit, which will be null for new avatars until we get the InitSucceeded signal.
	DgData& gd = g_Au.LockDataForWrite();
	gd.SetActiveUnit(personId);
	g_Au.UnlockData();

	if(!personId.IsNull())
		SetRunState(LoggedIn);
	else
		SetRunState(LoggedInWait);

	SceneLogin();				// First packet in stream will be the login.
}

void AuApp::StartNewAvatarSolo(int avatarIndex, const QString& charname, const AuDNABase& dna, AuDNABase::CHROMATID chroma,
		HANDEDNESS hand, quint8 proximity, GALAXY galaxy, SCENARIO scenario, AuTime asteroidImpact, quint32 joinEmpire,
		const AuId& joinSectorId, const QString& newEmpireName, const QImage& newEmpireFlag)
{
	// Create a new solitaire universe.
	SetRunState(Solitaire);		// Mode must be solo before we create any atoms.
	AuId personId = CreateSoloAvatar(avatarIndex, proximity, galaxy, charname, &dna, chroma, hand, scenario, asteroidImpact,
			joinEmpire, joinSectorId, newEmpireName, newEmpireFlag);
	//qDebug() << "AuApp::StartNewAvatarSolo" << personId;

	// Select the current active unit.
	DgData& gd = g_Au.LockDataForWrite();
	gd.SetActiveUnit(personId);
	g_Au.UnlockData();
}

// This completes the process of starting or resuming a solitaire game.
void AuApp::StartSolitaire(AuId personId)
{
	//qDebug() << "AuApp::StartSolitaire" << personId;
	Q_ASSERT(!personId.IsNull());
	SetRunState(Solitaire);

	// Select the current active unit.
	DgData& gd = g_Au.LockDataForWrite();
	gd.SetActiveUnit(personId);
	g_Au.UnlockData();

	// Clear the model and view.
	m_Model->ConnectionEstablished();
}

void AuApp::ForgetEmpireRequests()
{
	// Clear my list of empire ids that have been requested.
	m_EmpiresRequestedLock.lock();
	m_EmpiresRequested.clear();
	m_EmpiresRequestedLock.unlock();

	// Discard all 'Forgotten Empire NN' empires that may have been supplied in response to previous empire requests. Maybe we'll
	// get a better response from the next server.
	DgData& gd = g_Au.LockDataForWrite();
	gd.m_gEmpires.DiscardForgottenEmpires();
	g_Au.UnlockData();
}

// Configure this model for single-user spacecraft design mode.
void AuApp::InitDesignerMode()
{
	// Create an empire for the person.
	DgEmpire empire("Imperium");
	empire.m_id = 1;
	empire.m_flag = QImage(":data/ImperiumFlag.png")
		.scaled(DgEmpire::FlagImageMax(),DgEmpire::FlagImageMax(), Qt::KeepAspectRatio, Qt::SmoothTransformation)
		.convertToFormat(QImage::Format_Indexed8);

	AuEther& ether = g_Au.LockEtheriumDelete();

	// Create a universe;
	DaUniverse* univ = new DaUniverse;
	univ->ConnectSignals();
	univ->Create();
	ether.AddAtom(univ);
	univ->SetNormal();	// New atom is ready for normal thread processing.

	// Create a galaxy.
	DaGalaxy* galaxy = new DaGalaxy(ShoresOfHazeron);
	galaxy->ConnectSignals();
	galaxy->Create();
	galaxy->InitPos(univ->Id());
	ether.AddAtom(galaxy);
	galaxy->SetNormal();	// New atom is ready for normal thread processing.

	int proximity = 0;

	// Create a cube of sectors.
	AuIdList idNewSectors;
	DaSector* sector = galaxy->CreateNewHomeWorldSector(proximity, idNewSectors);
	sector->SetName(g_Au.Dice().RandomCityName());

	// Locate a solar system in that sector.
	DaSolarSystem* system = sector->FindGoodHome();
	Q_ASSERT(system);
	system->FinalizeContents();
	system->SetStarFieldFlag(false);
	DaWorld* planet = system->FindHabitableWorld(HABITABLE_HomeWorld);
	if(!planet)
	{
		g_Au.UnlockEtheriumDelete();
		return;
	}
	system->SetNameIncludePlanets(g_Au.Dice().RandomCityName(), false);
	system->ExploredBy(empire.m_id);

	// Create a design studio.
	DaDesigner* designer = new DaDesigner(0);
	designer->ConnectSignals();
	designer->Create();
	designer->SetPosAndOrient(AuAtomPos(planet->Id(), AuMatrix4x4()));
	ether.AddAtom(designer);
	designer->SetNormal();	// New atom is ready for normal thread processing.

	// Create a person.
	DaPerson* person = new DaPerson(m_HumanDNA, m_HumanGender == Male ? AuDNABase::Alpha : AuDNABase::Beta, m_HumanHand,
			m_HumanName, Mature, 0);
	person->ConnectSignals();
	{
		// When creating designer atom, assign a predefined atom id.
		//person->Create();
		person->SetId(AuId::Silverwill());
		person->SetCreatedTime(AuTime::Now());
		person->SetModifyTime(AuTime::Now());
		if(m_HumanGender == Male)
			person->SetHairStyle(AuHairStyle::Regulation, AuHairStyle::Clean);
		else
			person->SetHairStyle(AuHairStyle::Baltar, AuHairStyle::Clean);
	}
	person->Stand();
	AuMatrix4x4 t;
	t.Translate(2.f * FeetPerMeter(), -5.f * FeetPerMeter(), person->GroundHeightAboveCG());
	person->SetPosAndOrient(AuAtomPos(designer->Id(), t));
	ether.AddAtom(person);

	person->ResetHitPoints();
	person->ResetHunger();
	person->ResetStamina();
	person->ClearSpawnCity();	// So they won't be treated like a newbie emperor.
	person->SetEmpireId(empire.m_id);
	person->InitGearDesigner();
	person->SetNormal();	// New atom is ready for normal thread processing.

	// Establish owner of designer and move the person to a good starting location.
	AuScDesign design;
	design.SetStructure(SC_Starship);
	designer->NewDesign(person->Id(), person->Name(), design);

	// Update title on main frame window.
	QString windowTitle = QString("%1 %2 New Spacecraft")
		.arg(g_Au.ApplicationName())
		.arg(g_Au.MiddleDotSymbol());
	Frame()->setWindowTitle(windowTitle);

	// Create bogus login credentials as if our new architect just logged in.
	AuId idUnits[6];
	QString names[6];
	const AuDNABase* dna[6] = {0,0,0,0,0,0};
	AuDNABase::CHROMATID chroma[6];
	HANDEDNESS hand[6];
	COMMODITYID clothes[6];
	COMMODITYID armor[6];
	COMMODITYID hat[6];
	quint8 visor[6];
	ACCOUNT acctType;
	idUnits[0] = person->Id();
	names[0] = person->Name();
	AuHairStyle::STYLE hair[6];
	AuHairStyle::SHAVE shave[6];
	dna[0] = person->DNAnew();
	chroma[0] = person->Chromatid();
	hand[0] = person->IsLeftHanded() ? LeftHanded : RightHanded;
	clothes[0] = CID_RedUniform;
	armor[0] = CID_Invalid;
	hat[0] = CID_Invalid;
	visor[0] = 0;
	hair[0] = AuHairStyle::Regulation;
	shave[0] = AuHairStyle::Clean;
#if !defined(QT_NO_DEBUG)
	// Debug build plays in architect mode.
	acctType = ACCT_Architect;
#else
	// Release build plays in normal player mode.
	acctType = ACCT_Active;
#endif
	ether.TickTock();
	g_Au.UnlockEtheriumDelete();

	DgData& gd = g_Au.LockDataForWrite();
	gd.ReplaceEmpire(empire);
	gd.SetClientInfo(0, 1, idUnits, names, dna, chroma, hand, clothes, armor, hat, visor, hair, shave, acctType, AuTime());
	gd.SetActiveUnit(idUnits[0]);
	g_Au.UnlockData();

	delete dna[0];

	Frame()->StarMapInit(empire.m_id);

	// Tell the app that we have received the initial scene sync.
	SceneSyncComplete();
}

AuId AuApp::CreateSoloAvatar(int avatarIndex, unsigned char proximity, GALAXY galaxy, const QString& avatarName,
		const AuDNABase* dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand, SCENARIO scenario, const AuTime& asteroidImpact,
		quint32 joinEmpire, const AuId& joinSectorId, const QString& newEmpireName, const QImage& newEmpireFlag)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	AuEther& ether = g_Au.LockEtheriumDelete();

	// Insure the universe exists.
	if(!ether.FindUniverse())
	{
		// Create a universe.
		DaUniverse* universe = new DaUniverse;
		universe->ConnectSignals();
		universe->Create();
		universe->SetPosAndOrient(AuAtomPos(AuId(0), AuMatrix4x4()));
		ether.AddAtom(universe);
		universe->SetNormal();	// New atom is ready for normal thread processing.

		// Create all galaxies.
		universe->InitGalaxies();

		// Initialize empires, designs and exchanges.
		int result = m_Data->InitSolitaire();
		switch(result)
		{
		case 0:	// ok
			//qDebug() << "GS file loaded ok";
			break;
		case 1:
			qWarning() << "warning: GS file does not exist. Proceeding with no initial empires, designs or exchanges.";
			break;
		case 2:
			qWarning() << "warning: GS file could not be opened. Proceeding with no initial empires, designs or exchanges.";
			break;
		case 3:
			qWarning() << "warning: GS file is broken. Proceeding with no initial empires, designs or exchanges.";
			break;
		}

		// Story scripts need empires.
		g_AuClient.LoadScripts();

		// Basic data is loaded. Init next id values to start after what we already have.
		g_AuClient.InitNextIds();
	}

	// Create a person.
	DaPerson* person = new DaPerson(*dna, chroma, hand, avatarName, Mature, 0);
	person->ConnectSignals();
#if !defined(QT_NO_DEBUG)
	if(avatarName == "Haxus")
	{
		// When creating Haxus, assign a predefined atom id. It has been reserved for this purpose.
		person->SetId(AuId::Haxus());
		person->SetCreatedTime(AuTime::Now());
		person->SetModifyTime(AuTime::Now());
	}
	else
		person->Create();
#else
	person->Create();
#endif

	// Attempt to init using the specified scenario.
	bool scenarioOk = false;
	person->SetInitScenario(scenario);
	switch(scenario)
	{
	case ReachForTheStars:
		scenarioOk = InitReachForTheStars(ether, person, proximity, galaxy, asteroidImpact);
		break;
	case CitizenOfTheGalaxy:
		scenarioOk = InitCitizenOfTheGalaxy(ether, person, joinEmpire, galaxy, joinSectorId);
		break;
	case SavageState:
		scenarioOk = InitSavageState(ether, person, proximity, galaxy);
		break;
	case LostAtlantis:
		scenarioOk = InitLostAtlantis(ether, person, proximity, galaxy, newEmpireName, newEmpireFlag);
		break;
	case SCENARIO_Count:
		break;
	}

	if(!scenarioOk)
	{
		// Try a simple fallback state.
		person->SetInitScenario(SavageState);
		scenarioOk = InitSavageState(ether, person, proximity, galaxy);
	}

	AuId personId;
	if(scenarioOk)
	{
		personId = person->Id();

		// Get current credentials from the global data.
		AuId idUnits[6];
		QString names[6];
		AuDNABase* dnas[6];
		AuDNABase::CHROMATID chromas[6];
		HANDEDNESS hands[6];
		COMMODITYID clothes[6];
		COMMODITYID armor[6];
		COMMODITYID hat[6];
		quint8 visor[6];
		AuHairStyle::STYLE hair[6];
		AuHairStyle::SHAVE shave[6];
		DgData& gd = g_Au.LockDataForWrite();
		for(int ai=0; ai<6; ai++)
		{
			if(ai == avatarIndex)
			{
				idUnits[ai] = personId;
				names[ai] = avatarName;
				dnas[ai] = dna->Clone();
				chromas[ai] = chroma;
				hands[ai] = hand;
				clothes[ai] = CID_Invalid;
				armor[ai] = CID_Invalid;
				hat[ai] = CID_Invalid;
				visor[ai] = 0;
				hair[ai] = person->Haircut();
				shave[ai] = person->Shave();
			}
			else
			{
				AuId id = gd.ClientUnitId(ai);
				if(!id.IsNull())
				{
					idUnits[ai] = id;
					names[ai] = gd.ClientUnitName(id);
					dnas[ai] = gd.ClientUnitDNA(id)->Clone();
					chromas[ai] = gd.ClientUnitChromatid(id);
					hands[ai] = gd.ClientUnitHand(id);
					clothes[ai] = gd.ClientUnitClothes(id);
					armor[ai] = gd.ClientUnitArmor(id);
					hat[ai] = gd.ClientUnitHat(id);
					visor[ai] = gd.ClientUnitVisor(id);
					hair[ai] = gd.ClientUnitHaircut(id);
					shave[ai] = gd.ClientUnitShave(id);
				}
				else
				{
					idUnits[ai] = AuId();
					names[ai] = QString();
					dnas[ai] = 0;
					chromas[ai] = AuDNABase::Alpha;
					hands[ai] = LeftHanded;
					clothes[ai] = CID_Invalid;
					armor[ai] = CID_Invalid;
					hat[ai] = CID_Invalid;
					visor[ai] = 0;
					hair[ai] = AuHairStyle::Unkempt;
					shave[ai] = AuHairStyle::Unshaven;
				}
			}
		}

#if !defined(QT_NO_DEBUG)
		// Debug build plays in architect mode.
		ACCOUNT acctType = ACCT_Architect;
#else
		// Release build plays in normal player mode.
		ACCOUNT acctType = ACCT_Active;
#endif

		gd.SetClientInfo(1, 1, idUnits, names, (const AuDNABase**)dnas, chromas, hands, clothes, armor, hat, visor,
				hair, shave, acctType, AuTime());
		g_Au.UnlockData();

		for(int ai=0; ai<6; ai++)
			if(dnas[ai])
				delete dnas[ai];
	}
	else
	{
		delete person;
	}

	ether.TickTock();
	g_Au.UnlockEtheriumDelete();
	QApplication::restoreOverrideCursor();
	return personId;
}

bool AuApp::InitReachForTheStars(AuEther& ether, DaPerson* person, char proximity, GALAXY galaxy, const AuTime& asteroidImpact)
{
	DaGalaxy* ag = ether.FindGalaxy(galaxy);
	if(!ag)
		return false;

	// Create a cube of sectors.
	AuIdList idNewSectors;
	DaSector* sector = ag->CreateNewHomeWorldSector(proximity, idNewSectors);
	Q_ASSERT(sector);
	if(!sector)
		return false;

	// Locate a solar system in that sector.
	DaSolarSystem* system = sector->FindGoodHome();
	if(!system)
		return false;

	system->FinalizeContents();
	system->SetStarFieldFlag(false);

	DaWorldLL* planet = dynamic_cast<DaWorldLL*>(system->FindHabitableWorld(HABITABLE_HomeWorld));
	if(!planet)
		return false;

	// Start the doomsday asteroid.
	AuPoint startAt = AuParticleSystem::GetRandomVector();

	// Calculate the distance so normal speed gets the asteroid to the target on time. The asteroid continuously
	// modulates its speed to arrive at the target on time. This calc will make it modulate to a speed close to
	// normal asteroid speed.
	quint32 minutesToImpact = AuTime::Now().MinsTo(asteroidImpact);
	double distance = minutesToImpact * 60 * DaAsteroid::TargetPursueSpeed();
	if(distance > FeetPerParsec()/2)
		distance = FeetPerParsec()/2;	// Don't fall into the next solar system.
	startAt.Unit(distance);
	AuMatrix4x4 t;
	t.Translate(startAt);

	// Calculate release time so asteroid comes screaming in at high speed, instead of crawling. Slow asteroids
	// are boring. Too fast and they strike too quickly to have any fun.
	qint64 secsTravel = (qint64)(distance / DaAsteroid::TargetPursueSpeed());	// Time to get there at TPS.
	AuTime timeOfRelease = asteroidImpact.AddSecs(-secsTravel);	// If earlier than now, the asteroid starts now.

	DaAsteroid* roid = new DaAsteroid(planet->Id(), asteroidImpact, timeOfRelease);
	roid->ConnectSignals();
	roid->Create();
	roid->SetPosAndOrient(AuAtomPos(system->Id(), t));
	ether.AddAtom(roid);
	roid->SetNormal();	// New atom is ready for normal thread processing.

	person->InitNewAvatar(planet);
	return true;
}

bool AuApp::InitCitizenOfTheGalaxy(AuEther& ether, DaPerson* person, quint32 joinEmpire, GALAXY galaxy, const AuId& joinSectorId)
{
	if(!joinEmpire)
		return false;

	DaGalaxy* ag = ether.FindGalaxy(galaxy);
	if(!ag)
		return false;

	// Create a cube of sectors.
	AuIdList idNewSectors;
	DaSector* sector = ag->CreateJoinEmpireHomeWorldSector(joinSectorId, idNewSectors);
	Q_ASSERT(sector);
	if(!sector)
		return false;

	// Locate a solar system in that sector.
	DaSolarSystem* system = sector->FindGoodHome();
	if(!system)
		return false;

	system->FinalizeContents();
	system->SetStarFieldFlag(false);

	DaWorldLL* planet = dynamic_cast<DaWorldLL*>(system->FindHabitableWorld(HABITABLE_HomeWorld));
	if(!planet)
		return false;

	person->InitNewAvatar(planet);

	// Join the person to their new empire.
	person->SetEmpireId(joinEmpire);
	AddCitizenToEmpire(0, joinEmpire, person->Id(), person->Name(), person->Gender());
	return true;
}

bool AuApp::InitSavageState(AuEther& ether, DaPerson* person, unsigned char proximity, GALAXY galaxy)
{
	DaGalaxy* ag = ether.FindGalaxy(galaxy);
	if(!ag)
		return false;

	// Create a cube of sectors.
	AuIdList idNewSectors;
	DaSector* sector = ag->CreateNewHomeWorldSector(proximity, idNewSectors);
	Q_ASSERT(sector);
	if(!sector)
		return false;

	// Locate a solar system in that sector.
	DaSolarSystem* system = sector->FindGoodHome();
	if(!system)
		return false;

	system->FinalizeContents();
	system->SetStarFieldFlag(false);

	DaWorldLL* planet = dynamic_cast<DaWorldLL*>(system->FindHabitableWorld(HABITABLE_HomeWorld));
	if(!planet)
		return false;

	person->InitNewAvatar(planet);
	return true;
}

bool AuApp::InitLostAtlantis(AuEther& ether, DaPerson* person, unsigned char proximity, GALAXY galaxy, const QString& empireName,
		const QImage& empireFlag)
{
	if(!IsNameValid(empireName))
		return false;
	if(empireFlag.isNull())
		return false;

	DaGalaxy* ag = ether.FindGalaxy(galaxy);
	if(!ag)
		return false;

	// Create a cube of sectors.
	AuIdList idNewSectors;
	DaSector* sector = ag->CreateNewHomeWorldSector(proximity, idNewSectors);
	Q_ASSERT(sector);
	if(!sector)
		return false;

	// Locate a solar system in that sector, preferably a bad one.
	DaSolarSystem* system = sector->FindBadHome();
	if(!system)
		system = sector->FindGoodHome();
	if(!system)
		return false;

	// Locate the design for the Atlantis.
	AuScReport atlantis;
	const DgData& sdd = g_Au.LockScDesignsForRead();
	bool hasDesign = sdd.ScDesignReport(d_AtlantisDesignId, atlantis);
	g_Au.UnlockScDesigns();
	if(!hasDesign)
		return false;

	system->FinalizeContents();
	system->SetStarFieldFlag(false);

	// Create the ship.
	DaSc* ship = new DaSc();
	ship->ConnectSignals();
	ship->Create();
	ship->SetDesign(atlantis);

	// Position the ship in the solar system.
	AuPoint spawnAt = AuParticleSystem::GetRandomVector() * FeetPerSolarSystem() * 2;
	AuPoint spawnRot = AuParticleSystem::GetRandomVector() * PI;
	AuMatrix4x4 t;
	t.Translate(spawnAt);
	t.Rotate(spawnRot);
	ship->SetPosAndOrient(AuAtomPos(system->Id(), t));
	ether.AddAtom(ship);

	//ship->SetPrivateOwnership(true);
	//ship->SetPrivateOwner(person->Id(), person->Name());

	g_Au.AtomAdded(ship->Id());
	ship->SetNormal();	// New atom is ready for normal thread processing.

	EstablishNewEmpire(person->Id(), person->Name(), person->Gender(), 0, empireName, empireFlag);
	const DgData& gd = g_Au.LockDataForRead();
	quint32 empireId = gd.EmpireId(empireName);
	g_Au.UnlockData();

	AuSpecies species = person->Species();
	ship->SetupLostAtlantis(empireId, QualityBaseline(), species);
	ship->SetInMotion();

	person->InitNewAvatarAboardShip(ship);
	person->SetEmpireId(empireId);
	AddCitizenToEmpire(0, empireId, person->Id(), person->Name(), person->Gender());
	return true;
}

// This signal is triggered when creating a new dude. The worker server sends us the id of our newly created person unit.
void AuApp::InitSucceeded(AuId idUnit)
{
	// We finally know the id of our unit.
	//qDebug("AuApp::InitSucceeded unitId %d", idUnit.GetID());
	DgData& gd = g_Au.LockDataForWrite();
	gd.SetActiveUnit(idUnit);
	g_Au.UnlockData();

	// Resend our chat connect packet so the channel knows my unit id.
	SendChatConnect();

	// Let the scene display normally relative to my unit.
	SetRunState(LoggedIn);
}

// The server has initiated a logout.
void AuApp::LoggedOut()
{
	AuFrame* frame = Frame();
	if(frame)
		frame->StopThreads();

	SetRunState(NotLoggedIn);

	QString msg = tr("The server has logged off.\n\n%1 will now terminate.").arg(g_Au.ApplicationName());
	QMessageBox::critical(frame, g_Au.ApplicationName(), msg);

	closeAllWindows();
}

// We receive this call when DtClient verification of our login info fails.
void AuApp::LoginFailed(quint8 errCode)
{
	AuFrame* frame = Frame();
	if(frame)
		frame->StopThreads();

	SetRunState(NotLoggedIn);

	if(errCode == 0)
	{
		QString msg = tr("<html>"
				"Login failure. "
				"Please restart <i>%2</i> and try again.<p>"
				"<i>Tips:</i><p>"
				"%1 Be sure no one else is using your login account name and password.<br>"
				"%1 Verify that your account activation has not expired. "
				"It can be checked by logging in to the Pay to Play page at %3.<p>"
				"<i>%2</i> will now exit."
				"</html>")
			.arg(g_Au.MiddleDotSymbol())
			.arg(g_Au.ApplicationName())
			.arg(g_Au.WebAddress());
		QMessageBox::critical(frame, g_Au.ApplicationName(), msg);
	}
	else if(errCode == 1)
	{
		QString msg = tr("<html>"
				"Login failure. "
				"The version of this program is incompatible with the server.<p>"
				"<i>Tips:</i><p>"
				"%1 Run the <i>%2 Launcher</i> to update your local program files.<br>"
				"%1 Try again in a short while. We could be posting updates now.<p>"
				"<i>%2</i> will now exit."
				"</html>")
			.arg(g_Au.MiddleDotSymbol())
			.arg(g_Au.ApplicationName());
		QMessageBox::critical(frame, g_Au.ApplicationName(), msg);
	}

	closeAllWindows();
}

void AuApp::ConnectionEstablished()
{
	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: Connection established"));
	m_wasConnected = true;

	// We have established a new connection with a server.
	Model()->ConnectionEstablished();
}

void AuApp::SceneLogout()
{
	//qDebug("AuApp::SceneLogout");
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtSceneServerPacket::PL_Logout;
	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: Sending logout TCP packet."));
	QueueScenePacket(ba);
	SetRunState(NotLoggedIn);
}

void AuApp::StartNewAvatar(const QString& charname, const AuDNABase& dna, AuDNABase::CHROMATID chroma, HANDEDNESS hand,
		quint8 proximity, GALAXY galaxy, SCENARIO scenario, AuTime asteroidImpact, quint32 joinEmpire,
		const QByteArray& joinPassCrypt, const AuId& joinSectorId)
{
	m_NewGalaxy = galaxy;
	m_NewProximity = proximity;
	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		g_Au.Log(g_Au.CacheApp()+"/soh.txt", "App: Start New Avatar");
	SetRunState(InitWait);

	// Save these; we need them when we get deferred to the next server.
	// We send them in this first packet so the scene master can validate them right away.
	QDataStream ds(&m_NewDNA, QIODevice::WriteOnly);
	ds.setVersion(QDataStream::Qt_5_7);
	ds << charname.left(MaxNameLength()).trimmed();	// Name
	ds << (quint8)chroma;							// Chromatid
	ds << (quint8)hand;								// Handedness
	dna.WriteDynamic(ds);							// DNA
	ds << (quint8)scenario;							// Startup scenario
	ds << asteroidImpact;							// Asteroid impact time, for ReachForTheStars scenario
	ds << joinEmpire;								// Empire to join, for CitizenOfTheGalaxy scenario
	ds << joinPassCrypt;
	ds << joinSectorId;

	// Transmit the data to the server and wait for a reply.
	// Master creates my home sector and defers me to a manager. See EstDeferred.
	// Manager creates my home solar system and defers me to a worker. See EstDeferred.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtSceneServerPacket::PE_StartNewAvatar;
	out << proximity;
	out << (quint8)galaxy;
	out << (quint8)scenario;
	out << joinSectorId;
	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: Sending start new avatar TCP packet."));
	QueueScenePacket(ba);
}

void AuApp::RestartNewAvatar()
{
	// Some args we can retrieve from the m_NewDNA blob.
	QString charname;
	quint8 chroma;
	quint8 hand;
	AuDNABase* dna;
	quint8 scenario;
	AuTime asteroidImpact;
	quint32 joinEmpire;
	QByteArray joinPassCrypt;
	AuId joinSectorId;

	QDataStream in(m_NewDNA);
	in.setVersion(QDataStream::Qt_5_7);
	in >> charname;
	in >> chroma;
	in >> hand;
	dna = AuDNABase::ReadDynamic(in);
	in >> scenario;
	in >> asteroidImpact;
	in >> joinEmpire;
	in >> joinPassCrypt;
	in >> joinSectorId;

	qDebug() << "AuApp::RestartNewAvatar" << charname << "dna" << (dna?true:false);
	if(dna)
		StartNewAvatar(charname, *dna, (AuDNABase::CHROMATID)chroma, (HANDEDNESS)hand, m_NewProximity, m_NewGalaxy,
				(SCENARIO)scenario, asteroidImpact, joinEmpire, joinPassCrypt, joinSectorId);
}

void AuApp::ProcessControlInput(const AuId& toAtom, DiControlInputArray& aActiveControls, const AuGearPos& pos, qint8 chSelValue,
		int msElapsed)
{
	// If there is nothing but key modifiers, we can ignore the input altogether without bothering the server.
	// We must send changes in rotation, even if there are no other inputs.
	if(aActiveControls.isEmpty() || aActiveControls.ContainsOnlyModifiers())
		return;

	if(IsSingleUserMode())
	{
		// Process controls locally for smooth local response and also send controls to the local single-user server.
		ProcessControlInput(CONTROL_Solo, toAtom, aActiveControls, pos, chSelValue, msElapsed);
	}
	else
	{
		// Process controls locally for smooth local response.
		ProcessControlInput(CONTROL_Client, toAtom, aActiveControls, pos, chSelValue, msElapsed);

		// Send controls to server for processing.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PI_Control;
		out << NextUdpSequence();					// Helps the server discard packets received out of sequence.
		g_Au.CoordinatedTime() >> out;							// Helps server discard packets too old.

		out << toAtom;

		quint8 hasWhat = 0;
		if(pos.IsGear())
			hasWhat |= 0x01;
		if(chSelValue != 0)
			hasWhat |= 0x04;
		out << hasWhat;

		if(hasWhat & 0x01)
			out << pos;

		if(hasWhat & 0x04)
			out << (qint8)chSelValue;

		aActiveControls.Tx(out);

		// Signal the app to send the control packet for us. We can't call it directly due to the socket;
		// it doesn't like being touched by other threads.  DHH This is now called by the foreground thread.
		g_AuClient.ControlPacket(ba);
		//emit ControlPacketReady();	//DHH AuModel used to emit this to call my SendControlPackets function. Does it need queing?
		SendControlPackets();
	}
}

// This function was copied from DtSceneServerPacket::ProcessControlInput and cobbled to work for solo client-only input processing.
void AuApp::ProcessControlInput(CONTROL_CONTEXT localMotion, const AuId& toAtom, const DiControlInputArray& aActiveControls,
		const AuGearPos& pos, qint8 selValue, int msElapsed)
{
	// Locate the atom that is the target of the input.
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaAtom* pInputTo = ether.FindNormalAtom(toAtom);
	if(pInputTo)
	{
		// Locate the scene atom.
		DaPerson* pScene;
		if(AvatarId() == toAtom)
			pScene = dynamic_cast<DaPerson*>(pInputTo);
		else
			pScene = dynamic_cast<DaPerson*>(ether.FindNormalAtom(AvatarId()));
		if(pScene)
		{
			// Restore locally known position of scene atom and the atom it is controlling, if any.
			if(localMotion == CONTROL_Client)
				m_Model->RestoreScenePosition(pScene);

			// Process the input.
			pInputTo->ProcessControlInput(aActiveControls, pScene, pos, selValue, localMotion, msElapsed);

			// Preserve locally calculated position of scene atom and the atom it is controlling, if any.
			if(localMotion == CONTROL_Client)
				m_Model->PreserveScenePosition(pScene);
		}
	}
	g_Au.UnlockEtheriumDelete();
}

// 'idAtom' is the player's new sector when deferred to a scene manager.
// 'idAtom' is the player's new homeworld when deferred to a scene worker.
void AuApp::EstDeferred(AuId idAtom)
{
	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		g_Au.Log(g_Au.CacheApp()+"/soh.txt", QString("App: Est deferred - atom %1").arg(idAtom.StringId()));
	// Must send login info to new scene server.
	// It should be the first packet it receives from us.
	m_DeferAtom = idAtom;
	SceneDeferred(DeferEstFirstEmpire);

	// We will pick this up again in SceneLoggedIn, after we have received the login confirmation.
}

void AuApp::SceneDisconnected()
{
	//qDebug("AuApp::SceneDisconnected");
	// Stop the scene synchronizer until we get a full sync from the new server.
	m_InitSyncDone = false;
	m_SyncDone = false;

	// Disconnect udp socket from previous scene server. Next SceneLoggedIn signal received will connect it to the next server.
	if(m_SceneUdpSocket.state() != QAbstractSocket::UnconnectedState)
	{
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: Scene disconnected, disconnecting UDP connection, "
						"socket state %1.").arg(m_SceneUdpSocket.state()));
		m_SceneUdpSocket.disconnect(this, SLOT(ConnectUdp()));	// Let it die without trying to reconnect.
		m_SceneUdpSocket.close();
	}

	// Disconnect chat channel from previous scene server. Next SceneLoggedIn signal received will connect it to the next server.
	if(m_tChatChannel)
	{
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: Scene disconnected, disconnecting chat TCP connection"));
		DtClientChat* chat = m_tChatChannel;
		m_tChatChannel = 0;
		chat->disconnect(this, SLOT(ConnectChat()));	// Let it die without trying to reconnect.
		chat->Kill();
		m_ChatDisconnectTime.Set();
		//chat->wait(1000);
		//chat->deleteLater();
	}
}

// deferType is a DEFERTYPE
void AuApp::SceneDeferred(int deferType)
{
	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		g_Au.Log(g_Au.CacheApp()+"/soh.txt", "App: Scene Deferred");

	// Clear the model and view.
	m_Model->ConnectionEstablished();

	// Must send login info to new scene server.
	// It should be the first packet it receives from us.
	SceneLogin();

	// Defer data helps new player and emperor handoffs.
	m_DeferType = (DEFERTYPE)deferType;

	// Emit server connected signal so comm channels can reconnect.
	emit ServerConnected();
}

void AuApp::SceneLoggedIn()
{
	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: Client thread logged in, defer type %1.").arg(m_DeferType));

	// The current scene server has changed. Reset the model's list of empire records that have been requested. Maybe this new
	// server can resolve those requests.
	ForgetEmpireRequests();

	// We get a chat port number when we connect to a scene worker server.
	if(g_Au.ChatPort())
		m_SceneConnectedOnce = true;

	//qDebug("AuApp::SceneLoggedIn");

	// Send the join/establish data.
	if(m_DeferType == DeferEstFirstEmpire)
	{
		// Transmit the data to the next server and wait for a reply.
		// A good reply from the SceneManager will be deferral to the final SceneWorker.
		// A good reply from the SceneWorker will be a login packet identifying our new
		// person unit.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PE_StartNewAvatar;
		out << m_DeferAtom;		// Manager stops reading here. It doesn't use the dna data.
		out << m_NewDNA;
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: Deferred, sending start new avatar "
						"TCP packet."));
		QueueScenePacket(ba);
	}

	ConnectUdp();
	ConnectChat();
}

void AuApp::ConnectUdp()
{
	if(m_SceneUdpSocket.state() != QAbstractSocket::UnconnectedState)
	{
		// Disconnect the old connection since we have been told assertively to connect.
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: Connect UDP, disconnecting existing UDP connection, "
						"socket state %1.").arg(m_SceneUdpSocket.state()));
		m_SceneUdpSocket.disconnect(this, SLOT(ConnectUdp()));	// Let it die without trying to reconnect.
		m_SceneUdpSocket.close();
		if(m_SceneUdpSocket.state() != QAbstractSocket::UnconnectedState)
			return;	// The connection isn't done disconnecting.
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: UDP disconnected, socket state %1.")
					.arg(m_SceneUdpSocket.state()));
	}

	// Connect to server.
	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: Establishing client UDP connection to server %1 port %2...")
				.arg(g_Au.SceneServer()).arg(g_Au.ScenePort()));
	m_UdpSequence = 0;
	m_SceneUdpSocket.abort();
	m_SceneUdpSocket.connectToHost(g_Au.SceneServer(), g_Au.ScenePort());
	m_SceneUdpSocket.waitForConnected(10000);
	connect(&m_SceneUdpSocket, SIGNAL(readyRead()), SLOT(ReadPendingDatagrams()));
	connect(&m_SceneUdpSocket, SIGNAL(disconnected()), SLOT(ConnectUdp()));
	if(m_SceneUdpSocket.state() == QAbstractSocket::UnconnectedState)
	{
		// The connection attempt failed. The server or the network is probably not up.
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: Client UDP connection attempt failed to server %1 port %2.")
					.arg(g_Au.SceneServer()).arg(g_Au.ScenePort()));
		qWarning("AuApp::SceneLoggedIn - UDP connection attempt failed.");
	}
	else
	{
		//qDebug("AuApp::ConnectUdp - UDP connection established");
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: Client UDP connection established to server %1 port %2.")
					.arg(g_Au.SceneServer()).arg(g_Au.ScenePort()));
	}
}

void AuApp::ConnectChat()
{
	// Disconnect chat channel from previous scene server.
	if(m_tChatChannel)
	{
		//qDebug() << "AuApp::ConnectChat closing chat connection";
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: Connect chat, disconnecting existing chat TCP connection."));
		DtClientChat* chat = m_tChatChannel;
		m_tChatChannel = 0;
		chat->disconnect(this, SLOT(ConnectChat()));	// Let it die without trying to reconnect.
		chat->Kill();
		m_ChatDisconnectTime.Set();
		//chat->wait(1000);
		//chat->deleteLater();
	}

	if(g_Au.ChatPort())
	{
		//qDebug() << "AuApp::ConnectChat opening chat connection" << g_Au.SceneServer() << ":" << g_Au.ChatPort();
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: Establishing chat TCP connection to server %1 port %2...")
					.arg(g_Au.SceneServer()).arg(g_Au.ChatPort()));
		// Connect a chat channel to the next server.
		m_tChatChannel = new DtClientChat(this);
		if(m_tChatChannel)
		{
			AuFrame* pFrame = Frame();

			connect(m_tChatChannel, SIGNAL(ArenaReady(AuArenaReady)), SIGNAL(ArenaReadySignal(AuArenaReady)), Qt::QueuedConnection);
			connect(m_tChatChannel, SIGNAL(AvatarsUpdated()), pFrame, SLOT(AvatarsUpdated()), Qt::QueuedConnection);
			connect(m_tChatChannel, SIGNAL(ChannelOpened(QString, quint8, QString)), SLOT(ChannelOpened(QString, quint8, QString)));
			connect(m_tChatChannel, SIGNAL(ChatReceived(QByteArray)), SLOT(ChatReceived(QByteArray)));
			connect(m_tChatChannel, SIGNAL(CityHistoryReportReady(AuId, AuCityReportHistory)),
					pFrame, SLOT(CityHistoryReportReady(AuId, AuCityReportHistory)));
			connect(m_tChatChannel, SIGNAL(CityLastReportReady(AuId, AuCityReport)),
					pFrame, SLOT(CityLastReportReady(AuId, AuCityReport)));
			//connect(m_tChatChannel, SIGNAL(Connected()), SLOT(ConnectionEstablished()));
			connect(m_tChatChannel, SIGNAL(Disconnected()), SLOT(ConnectChat()));	// Reconnect if connection is lost.

			connect(m_tChatChannel, SIGNAL(PlayersOnlineStatus(AuPlayerOnlineStatusVector)),
					SIGNAL(PlayersOnlineStatus(AuPlayerOnlineStatusVector)));

			connect(m_tChatChannel, SIGNAL(BdExchangeBlueprintRating(quint32, QString, AuExchangeBlueprintRating)),
					pFrame, SLOT(BlueprintRating(quint32, QString, AuExchangeBlueprintRating)));
			connect(m_tChatChannel, SIGNAL(BdExchangeUpdated()), SIGNAL(BdExchangeUpdated()));
			connect(m_tChatChannel, SIGNAL(BdLibraryUpdated()), SIGNAL(BdLibraryUpdated()));

			connect(m_tChatChannel, SIGNAL(ScExchangeBlueprintRating(quint32, QString, AuExchangeBlueprintRating)),
					pFrame, SLOT(BlueprintRating(quint32, QString, AuExchangeBlueprintRating)));
			connect(m_tChatChannel, SIGNAL(ScExchangeUpdated()), SIGNAL(ScExchangeUpdated()));
			connect(m_tChatChannel, SIGNAL(ScLibraryUpdated()), SIGNAL(ScLibraryUpdated()));

			connect(m_tChatChannel, SIGNAL(EmpireRequested(quint32)),
					SLOT(EmpireRequested(quint32)), Qt::QueuedConnection);

			connect(m_tChatChannel, SIGNAL(GameOver(quint8, QByteArray)), pFrame, SLOT(GameOver(quint8, QByteArray)));
			connect(m_tChatChannel, SIGNAL(GameSelect(quint8, bool)), pFrame, SLOT(GameSelect(quint8, bool)));
			connect(m_tChatChannel, SIGNAL(GameTurn(AuId)), pFrame, SLOT(GameTurn(AuId)));
			connect(m_tChatChannel, SIGNAL(HangarQueryAnswered(QString, quint32, quint32)),
					pFrame, SLOT(HangarQueryAnswered(QString, quint32, quint32)));
			connect(m_tChatChannel, SIGNAL(JobAvailable(AuJobOffer)), pFrame, SLOT(JobAvailable(AuJobOffer)));
			connect(m_tChatChannel, SIGNAL(MissionSensorContactsReceived(QByteArray)),
					pFrame, SLOT(MissionSensorContactsReceived(QByteArray)));

			connect(m_tChatChannel, SIGNAL(OrdnanceReceived(AuId, QByteArray)),
					pFrame, SLOT(OrdnanceReceived(AuId, QByteArray)));

			connect(m_tChatChannel, SIGNAL(ReplaceSpecies(DzSpecies)), SLOT(ReplaceSpecies(DzSpecies)));

			connect(m_tChatChannel, SIGNAL(BdDesign(AuBdDesign)), SLOT(BdDesign(AuBdDesign)));
			connect(m_tChatChannel, SIGNAL(BdDesignDelete(quint32)), SLOT(BdDesignDelete(quint32)));
			connect(m_tChatChannel, SIGNAL(BdDesignNeeded(quint32, quint8)), SLOT(BdDesignNeeded(quint32, quint8)));

			connect(m_tChatChannel, SIGNAL(ItDesign(AuItDesign)), SLOT(ItDesign(AuItDesign)));
			connect(m_tChatChannel, SIGNAL(ItDesignDelete(quint32)), SLOT(ItDesignDelete(quint32)));
			connect(m_tChatChannel, SIGNAL(ItDesignNeeded(quint32, quint8)), SLOT(ItDesignNeeded(quint32, quint8)));

			connect(m_tChatChannel, SIGNAL(ScDesign(AuScDesign)), SLOT(ScDesign(AuScDesign)));
			connect(m_tChatChannel, SIGNAL(ScDesignDelete(quint32)), SLOT(ScDesignDelete(quint32)));
			connect(m_tChatChannel, SIGNAL(ScDesignNeeded(quint32, quint8)), SLOT(ScDesignNeeded(quint32, quint8)));

			connect(m_tChatChannel, SIGNAL(TransporterPlace(AuTransporterTarget)),
					pFrame, SLOT(TransporterPlace(AuTransporterTarget)));
			connect(m_tChatChannel, SIGNAL(TransporterShip(AuId, QString, QString)),
					pFrame, SLOT(TransporterShip(AuId, QString, QString)));
			connect(m_tChatChannel, SIGNAL(TransporterTransporter(AuId, QString, QString, quint16)),
					pFrame, SLOT(TransporterTransporter(AuId, QString, QString, quint16)));
			connect(m_tChatChannel, SIGNAL(BrokerShipment(AuShipment)),
					pFrame, SLOT(BrokerShipment(AuShipment)));
			connect(m_tChatChannel, SIGNAL(BrokerShipmentsClear()),
					pFrame, SLOT(BrokerShipmentsClear()));
			connect(m_tChatChannel, SIGNAL(BrokerTradeAgreement(AuTradeAgreement)),
					pFrame, SLOT(BrokerTradeAgreement(AuTradeAgreement)));
			connect(m_tChatChannel, SIGNAL(BrokerTradeAgreementsClear()),
					pFrame, SLOT(BrokerTradeAgreementsClear()));
			connect(m_tChatChannel, SIGNAL(TransferCitizensAvailable(QByteArray)),
					pFrame, SLOT(TransferCitizensAvailable(QByteArray)));
			m_tChatChannel->start(QThread::LowPriority);

			// Send a hello packet to the client channel.
			// When a new avatar has been created, our local unit id will still be null.
			SendChatConnect();
			if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
				g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: Chat TCP connection established to server %1 port %2.")
						.arg(g_Au.SceneServer()).arg(g_Au.ChatPort()));
		}
	}
}

void AuApp::SendChatConnect()
{
	//qDebug("AuApp::SendChatConnect");
	if(m_tChatChannel)
	{
		AuFrame* pFrame = Frame();

		// Send a hello packet to the client channel.
		const DgData& gd = g_Au.LockDataForRead();
		quint32 idPlayer = gd.PlayerId();
		quint32 confirmNum = gd.ConfirmationNumber();
		AuId idUnit = gd.ClientUnitId();	// Id of unit the player will be controlling.
		g_Au.UnlockData();
		//qDebug("AuApp::SendChatConnect unitId %d", idUnit.GetID());

		// Include a list of the comm channels we are monitoring.
		QStringList channelsOpen;
		pFrame->ListOpenChannels(channelsOpen);
		if(channelsOpen.isEmpty())
		{
			// Restore the list of last open channels. The comm window is going to reopen these. That way we will get the last
			// five minutes of chat.
			// Read open channels data from the local settings.
			QByteArray ba;
			g_Au.Settings().readEntry("Channels", ba);

			// Extract open channels array from the byte array.
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_4_1);
			quint16 chancount;
			in >> chancount;
			AuChannel channel;
			while(chancount--)
			{
				in >> channel;
				channelsOpen.append(channel.m_Name);
			}

			if(channelsOpen.isEmpty())
				AuCommStdChannel::ListDefaultChannels(channelsOpen);
		}

		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PC_Connect;
		out << idPlayer;
		out << confirmNum;
		out << idUnit;
		out << (quint8)channelsOpen.count();
		//qDebug() << "PC_Connect";
		for(int chan=0; chan<channelsOpen.count(); chan++)
		{
			out << channelsOpen[chan].left(MaxNameLength()).trimmed();
			//qDebug() << channelsOpen[chan].left(MaxNameLength()).trimmed();
		}
		m_ChatDisconnectTime >> out;		// So I will get chat messages received by the server since I disconnected.
		InsertSquelchIds(out);				// Send squelch ids so we don't rx an initial blast of messages from a squelched person.
		g_Au.Emojis().StoreVisages(out);	// Include my visage overrides because facial expression is controlled by the server.

		QueueChatPacket(ba);

		// Reset disconnect time for next disconnect.
		m_ChatDisconnectTime = AuTime();
	}
}

void AuApp::InsertSquelchIds(QDataStream& out)
{
	QString sAcctName;
	QByteArray sAcctPassword;
	bool bEmptyList = false;
	const DgData& gd = g_Au.LockDataForRead();
	sAcctName = gd.GetAcctName();
	sAcctPassword = gd.GetAcctPassCrypt();
	bEmptyList = gd.GetContacts().count();	// just to see if we have some
	g_Au.UnlockData();

	if(!bEmptyList)
	{
		AuLoginComm login;
		login.RequestContacts(sAcctName, sAcctPassword);
	}

	QHash<AuId, bool> hSquelchIds;
	const DgData& gd1 = g_Au.LockDataForRead();
	for(QList<AuContact>::const_iterator it = gd1.GetContacts().begin(); it != gd1.GetContacts().end(); it++)
	{
		if((*it).m_bIgnore)
			hSquelchIds.insert((*it).m_id, false);
	}
	g_Au.UnlockData();

	out << (quint16)hSquelchIds.size();
	for(QHash<AuId, bool>::const_iterator si = hSquelchIds.cbegin(); si != hSquelchIds.cend(); si++)
	{
		out << si.key();
		out << si.value();
	}
}

void AuApp::UpdateSquelch(AuId idContact, bool bDelete)
{
	if(bDelete)
		SendDeleteSquelch(idContact);
	else
		SendAddSquelch(idContact);
}

void AuApp::UpdateSquelches(AuIdList idContacts, bool bDelete)
{
	for(QList<AuId>::iterator it = idContacts.begin(); it != idContacts.end(); it++)
		UpdateSquelch(*it, bDelete);
}

void AuApp::SendAddSquelch(const AuId& idSquelch)
{
	if(m_tChatChannel)
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PC_SquelchAdd;
		out << idSquelch;
		QueueChatPacket(ba);
	}
}

void AuApp::SendDeleteSquelch(AuId &idSquelch)
{
	if(m_tChatChannel)
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PC_SquelchDelete;
		out << idSquelch;
		QueueChatPacket(ba);
	}
}

void AuApp::GoLandAtCity(AuId cityId, QString cityName)
{
	AuMissionOrderGoToCityAnywhere order(cityId, cityName);
	if(!order.IsValid())
		return;

	if(IsSingleUserMode())
	{
		m_I->MissionAppendOrder(order);
	}
	else
	{
		// Send the mission add command request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PN_Mission;
		out << (quint8)2;					// add mission order
		out << (quint8)order.Type();
		out << order;
		QueueChatPacket(ba);
	}
}

void AuApp::GoOrbit(AuId worldId, QString worldName)
{
	AuMissionOrderGoToOrbit order(worldId, worldName);
	if(!order.IsValid())
		return;

	if(IsSingleUserMode())
	{
		m_I->MissionAppendOrder(order);
	}
	else
	{
		// Send the mission add command request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PN_Mission;
		out << (quint8)2;					// add mission order
		out << (quint8)order.Type();
		out << order;
		QueueChatPacket(ba);
	}
}

void AuApp::GoOrbitCity(AuId cityId, QString cityName)
{
	AuMissionOrderGoToOrbitCity order(cityId, cityName);
	if(!order.IsValid())
		return;

	if(IsSingleUserMode())
	{
		m_I->MissionAppendOrder(order);
	}
	else
	{
		// Send the mission add command request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PN_Mission;
		out << (quint8)2;					// add mission order
		out << (quint8)order.Type();
		out << order;
		QueueChatPacket(ba);
	}
}

void AuApp::GoDirectToCoord(AuPoint galaxyLoc)
{
	AuMissionOrderGoToCoordinate order(AuMissionOrderGoToCoordinate::Galaxy, galaxyLoc);
	if(!order.IsValid())
		return;

	if(IsSingleUserMode())
	{
		m_I->MissionAppendOrder(order);
	}
	else
	{
		// Send the mission add command request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PN_Mission;
		out << (quint8)2;					// add mission order
		out << (quint8)order.Type();
		out << order;
		QueueChatPacket(ba);
	}
}

void AuApp::GoDirectToSystem(AuId systemId, QString systemName, AuPoint galaxyLoc)
{
	AuMissionOrderGoToSystemDirect order(systemId, systemName, galaxyLoc);
	if(!order.IsValid())
		return;

	if(IsSingleUserMode())
	{
		m_I->MissionAppendOrder(order);
	}
	else
	{
		// Send the mission add command request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PN_Mission;
		out << (quint8)2;					// add mission order
		out << (quint8)order.Type();
		out << order;
		QueueChatPacket(ba);
	}
}

void AuApp::GoViaWormholes(AuId systemId, QString systemName)
{
	AuMissionOrderGoToSystemViaWormholes order(systemId, systemName);
	if(!order.IsValid())
		return;

	if(IsSingleUserMode())
	{
		m_I->MissionAppendOrder(order);
	}
	else
	{
		// Send the mission add command request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PN_Mission;
		out << (quint8)2;					// add mission order
		out << (quint8)order.Type();
		out << order;
		QueueChatPacket(ba);
	}
}

bool AuApp::IsSceneConnected() const
{
	// In single-user mode, we pretend we have a good connection.
	if(IsSingleUserMode())
		return true;

	if(m_tSceneChannel)
		return m_tSceneChannel->IsConnected();
	return false;
}

void AuApp::ChatReceived(QByteArray msg)
{
	//qDebug() << "AuApp::ChatReceived bytes" << msg.count();
	AuFrame* pFrame = Frame();
	if(pFrame)
	{
		// Extract the packet from the byte array.
		QDataStream in(msg);
		in.setVersion(QDataStream::Qt_4_1);
		quint8 msgType;
		in >> msgType;
		AuCommPacket* msgPacket = AuCommPacket::CreateCommPacket((AuCommPacket::TYPE)msgType);
		in >> *msgPacket;

		pFrame->ChatReceived(*msgPacket);

		delete msgPacket;
	}
}

void AuApp::ChannelOpened(QString channelName, quint8 scope, QString narrDesc)
{
	AuFrame* pFrame = Frame();
	if(pFrame)
		pFrame->ChannelOpened(channelName, (AuCommStdChannel::SCOPE)scope, narrDesc);
}

void AuApp::EmpireRequested(quint32 empireId)
{
	if(!empireId)
		return;

	// Don't request the same empire more than once from a given server. The list is cleared when we change servers.
	// When requesting an empire by id, the server always replies, even if it's a bogus empire record named Forgotten Empire NN.
	m_EmpiresRequestedLock.lock();
	if(m_EmpiresRequested.contains(empireId))
	{
		// We have already requested that empire record.
		m_EmpiresRequestedLock.unlock();
		return;
	}
	m_EmpiresRequested.append(empireId);
	m_EmpiresRequestedLock.unlock();

	RequestEmpire(empireId);
}

void AuApp::RestoreConsoleSettings(quint8 stationType, QByteArray consoleSettings)
{
	quint8 packetType = DtChatChannelPacket::PF_Station0 + stationType;
	Q_ASSERT(packetType >= DtChatChannelPacket::PF_Station0 && packetType <= DtChatChannelPacket::PF_Station10);
	// Transmit the settings packet to the server.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << packetType;
	out << consoleSettings;
	QueueChatPacket(ba);
}

void AuApp::SendChatMessage(const AuCommPacket& msg)
{
	if(!msg.IsValid())
		return;

	if(IsSingleUserMode())
	{
		g_Au.SendAtomChat(msg);
	}
	else
	{
		// Create an encryption key based on this user's login credentials.
		const DgData& gd = g_Au.LockDataForRead();
		quint16 wKey = AuUtil::NetCryptKey((quint16)gd.ConfirmationNumber());
		g_Au.UnlockData();

		// The message appears valid. Send it.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PC_Chat;
		msg.TxEncrypt(out, wKey);
		QueueChatPacket(ba);
	}
}

void AuApp::SwitchToAvatar(const AuId& personId)
{
	// Change the active unit id.
	DgData& gd = g_Au.LockDataForWrite();
	gd.SetActiveUnit(personId);
	g_Au.UnlockData();

	if(g_Au.IsSceneClient())
	{
		// Tell the scene server to make the switch.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PL_SwitchAvatar;
		out << personId;
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: Sending switch character TCP packet."));
		QueueScenePacket(ba);
	}
}

void AuApp::ReplaceSpecies(DzSpecies species)
{
	DgData& gd = g_Au.LockDataForWrite();
	gd.ReplaceSpecies(species);									// Add/replace a specie record in my local data.
	g_Au.UnlockData();

	AuFrame* pFrame = Frame();
	if(pFrame)
		pFrame->ReplaceSpecies(species);
}

bool AuApp::notify(QObject* receiver, QEvent* event)
{
#if defined(AUCATCHEXCEPTIONS)
	bool retval;
	try
	{
		retval = QApplication::notify(receiver, event);
	}
	/*
	catch(const overflow_error& e)
	{
		qDebug() << "AuApp::notify caught exception: overflow_error";
		retval = false;
	}
	catch(const runtime_error& e)
	{
		qDebug() << "AuApp::notify caught exception: runtime_error";
		retval = false;
	}
	catch(const exception& e)
	{
		qDebug() << "AuApp::notify caught exception: exception";
		retval = false;
	}
	*/
	catch(...)
	{
		qDebug() << "AuApp::notify caught exception, event type" << event->type();
		retval = false;
	}
	return retval;
#else
	return QApplication::notify(receiver, event);
#endif
}

void AuApp::AtomEnteredWormhole(AuId travId, AuId systemId, AuWormhole wh, float entryAngle)
{
	if(IsSingleUserMode())
	{
		Q_ASSERT(false);	// This is not called in solo mode.
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PI_EnteredWormhole;
		out << travId;
		out << systemId;
		out << wh;
		out << entryAngle;
		QueueScenePacket(ba);								// Transmit on scene channel to DtSceneServer.
	}
}

void AuApp::AtomCollision(AuId myId, AuPoint myChange, AuPoint myHitLoc, AuId hitId, AuPoint hitChange, AuPoint hitFromLoc,
			AuPoint hitResultVec, bool hitShield)
{
	if(IsSingleUserMode())
	{
		Q_ASSERT(false);	// This is not called in solo mode.
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PI_Collision;
		out << myId;
		out << myChange;
		out << myHitLoc;
		out << hitId;
		out << hitChange;
		out << hitFromLoc;
		out << hitResultVec;
		out << hitShield;
		QueueScenePacket(ba);								// Transmit on scene channel to DtSceneServer.
	}
}

void AuApp::AtomDeadheaded(AuId departingSystem, AuId travId, AuPoint travLoc)
{
	if(IsSingleUserMode())
	{
		Q_ASSERT(false);	// This is not called in solo mode.
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PI_Deadhead;
		out << departingSystem;
		out << travId;
		out << travLoc;
		QueueScenePacket(ba);								// Transmit on scene channel to DtSceneServer.
	}
}

void AuApp::AtomFell(AuId fellId, quint32 hitsDamage, int stopReason)
{
	if(IsSingleUserMode())
	{
		Q_ASSERT(false);	// This is not called in solo mode.
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PI_Fell;
		out << fellId;
		out << hitsDamage;
		out << stopReason;
		QueueScenePacket(ba);								// Transmit on scene channel to DtSceneServer.
	}
}

// Scene atom entered a cartesian coordinate at a transporter console.
void AuApp::TransporterCoordUI(AuPoint coord)
{
	AuFrame* pFrame = Frame();
	if(pFrame)
		pFrame->TransporterCoordUI(coord);
}

// Scene atom entered a longitude latitude coordinate at a transporter console.
void AuApp::TransporterLongLatUI(AuGlobeLLF ll)
{
	AuFrame* pFrame = Frame();
	if(pFrame)
		pFrame->TransporterLongLatUI(ll);
}

void AuApp::RequestAvatars()
{
	if(IsSingleUserMode())
	{
		emit AvatarsUpdated();
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_QueryAvatars;
		QueueChatPacket(ba);
	}
}

// Scene says I need a design.
void AuApp::ScDesignNeeded(quint32 designId)
{
	// Test to see if we have the design.
	const DgData& sdd = g_Au.LockScDesignsForRead();
	bool hasDesign = sdd.ScDesignIsLoaded(designId);
	g_Au.UnlockScDesigns();
	if(hasDesign)
		return;													// We have the design now.

	// We do not have the design.
	// Try to load the design from a cache file.
	QDir cacheDir(g_Au.CacheGame());
	QString cacheFileName = cacheDir.absoluteFilePath(QString("D%1").arg(designId));
	AuDesign* d = AuDesign::LoadFromFile(cacheFileName, SHIP_DESIGN_CACHE);
	if(d)
	{
		//qDebug() << "AuApp::ScDesignNeeded" << designId << "File loaded" << cacheFileName << !rooms.isEmpty()
			//<< design.m_id << design.m_name;
		AuScDesign* scd = dynamic_cast<AuScDesign*>(d);
		if(scd)
		{
			// We appear to have loaded the proper type of design.
			// Insure no other thread slipped it into the data somehow.
			DgData& sdd = g_Au.LockScDesignsForWrite();
			if(!sdd.ScDesignIsLoaded(designId))
				sdd.ScDesignAdd(*scd);							// Add the design to our local data.
			g_Au.UnlockScDesigns();
			//qDebug() << "AuApp::ScDesignNeeded" << designId << "done";

			// Notify the server that we have loaded this design from local cache.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtChatChannelPacket::PD_ScDesignHere;
			out << designId;
			QueueChatPacket(ba);

			delete d;
			return;
		}
		else
			delete d;
	}

	// Design was not loaded from a cache file.
	// Ask the server to send me the design.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtChatChannelPacket::PD_ScDesign;
	out << designId;
	QueueChatPacket(ba);
}

// Scene says I need a design with a specific state.
void AuApp::ScDesignNeeded(quint32 designId, quint8 designState)
{
	// Test to see if we have the design.
	const DgData& sdd = g_Au.LockScDesignsForRead();
	bool hasDesign = sdd.ScDesignIsLoaded(designId);
	quint8 hasState = sdd.ScDesignState(designId);
	g_Au.UnlockScDesigns();

	if(!hasDesign)
	{
		// We do not have the design.
		// Try to load the design from a cache file.
		QDir cacheDir(g_Au.CacheGame());
		QString cacheFileName = cacheDir.absoluteFilePath(QString("D%1").arg(designId));
		AuDesign* d = AuDesign::LoadFromFile(cacheFileName, SHIP_DESIGN_CACHE);
		if(d)
		{
			//qDebug() << "AuApp::ScDesignNeeded" << designId << "File loaded" << cacheFileName << !rooms.isEmpty()
				//<< design.m_id << design.m_name;
			AuScDesign* scd = dynamic_cast<AuScDesign*>(d);
			if(scd && scd->State() == (AuDesign::STATE)designState)
			{
				// We appear to have loaded the design and the state is correct.
				// Insure no other thread slipped it into the data somehow.
				DgData& sdd = g_Au.LockScDesignsForWrite();
				if(!sdd.ScDesignIsLoaded(designId))
					sdd.ScDesignAdd(*scd);						// Add the design to our local data.
				g_Au.UnlockScDesigns();
				//qDebug() << "AuApp::ScDesignNeeded" << designId << "done";

				hasState = scd->State();
				hasDesign = true;
			}
			delete d;
		}
	}

	if(hasDesign && hasState == designState)
	{
		// We appear to have the correct design and the state matches the server.
		// Notify the server that we have loaded this design from local cache.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScDesignHere;
		out << designId;
		QueueChatPacket(ba);
	}
	else
	{
		// Design was not loaded from a cache file.
		// Ask the server to send me the design.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScDesign;
		out << designId;
		QueueChatPacket(ba);
	}
}

void AuApp::ScDesign(AuScDesign rec)
{
	DgData& sdd = g_Au.LockScDesignsForWrite();
	//bool hasDesign = sdd.ScDesignIsLoaded(rec.Id());
	sdd.ScDesignIsLoaded(rec.Id());
	sdd.ScDesignAdd(rec);						// Add the design record, possibly for the first time this session.
	g_Au.UnlockScDesigns();

	// If the spacecraft design record is not present, don't signal the change.
	// Spacecraft that have been waiting for the design to arrive will now successfully analyze their designs for the first
	// time, simply by finally finding the design record to be present. Then they will create the appropriate models.
	// This logic prevents us from erasing the cached paths data until the design really does change, not simply because
	// we received it for the first time during this session.
	//if(hasDesign)
		//ScDesignChanged(rec.Id());

	// This the final packet of a design received by the client.
	// If the design is read-only, we can cache it locally. Then we don't have to ask for it next time.
	// A design in progress in the design studio will not be read-only so it will not be cached, fortunately.
	if(rec.IsReadOnly())
	{
		// The design can be stored to a cache file.
		//qDebug() << "AuApp::ScDesign" << rec.m_id << rec.m_name << "storing into cache file";
		// Insure the cache folder exists.
		if(g_Au.CreateCacheGame())
		{
			// The cache folder exists.
			QDir cacheDir(g_Au.CacheGame());
			QString cacheFileName = cacheDir.absoluteFilePath(QString("D%1").arg(rec.Id()));
			rec.SaveToFile(cacheFileName, SHIP_DESIGN_CACHE);
		}
		//qDebug() << "AuApp::ScDesign" << rec.m_id << rec.m_name << "storing into cache file, done.";
	}
}

void AuApp::ScDesignDelete(quint32 designId)
{
	DgData& sdd = g_Au.LockScDesignsForWrite();
	sdd.ScDesignRemove(designId);								// Remove design and associated recs.
	g_Au.UnlockScDesigns();
}

// Scene says I need a design.
void AuApp::BdDesignNeeded(quint32 designId)
{
	// Test to see if we have the design.
	const DgData& sdd = g_Au.LockBdDesignsForRead();
	bool hasDesign = sdd.BdDesignIsLoaded(designId);
	g_Au.UnlockBdDesigns();
	if(hasDesign)
		return;													// We have the design now.

	// We do not have the design.
	// Try to load the design from a cache file.
	QDir cacheDir(g_Au.CacheGame());
	QString cacheFileName = cacheDir.absoluteFilePath(QString("BD%1").arg(designId));
	AuDesign* d = AuDesign::LoadFromFile(cacheFileName, SHIP_DESIGN_CACHE);
	if(d)
	{
		//qDebug() << "AuApp::BdDesignNeeded" << designId << "File loaded" << cacheFileName << !rooms.isEmpty()
			//<< design.m_id << design.m_name;
		AuBdDesign* scd = dynamic_cast<AuBdDesign*>(d);
		if(scd)
		{
			// We appear to have loaded the proper type of design.
			// Insure no other thread slipped it into the data somehow.
			DgData& sdd = g_Au.LockBdDesignsForWrite();
			if(!sdd.BdDesignIsLoaded(designId))
				sdd.BdDesignAdd(*scd);							// Add the design to our local data.
			g_Au.UnlockBdDesigns();
			//qDebug() << "AuApp::BdDesignNeeded" << designId << "done";

			// Notify the server that we have loaded this design from local cache.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtChatChannelPacket::PD_BdDesignHere;
			out << designId;
			QueueChatPacket(ba);

			delete d;
			return;
		}
		else
			delete d;
	}

	// Design was not loaded from a cache file.
	// Ask the server to send me the design.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtChatChannelPacket::PD_BdDesign;
	out << designId;
	QueueChatPacket(ba);
}

// Scene says I need a design with a specific state.
void AuApp::BdDesignNeeded(quint32 designId, quint8 designState)
{
	// Test to see if we have the design.
	const DgData& sdd = g_Au.LockBdDesignsForRead();
	bool hasDesign = sdd.BdDesignIsLoaded(designId);
	quint8 hasState = sdd.BdDesignState(designId);
	g_Au.UnlockBdDesigns();

	if(!hasDesign)
	{
		// We do not have the design.
		// Try to load the design from a cache file.
		QDir cacheDir(g_Au.CacheGame());
		QString cacheFileName = cacheDir.absoluteFilePath(QString("BD%1").arg(designId));
		AuDesign* d = AuDesign::LoadFromFile(cacheFileName, SHIP_DESIGN_CACHE);
		if(d)
		{
			//qDebug() << "AuApp::BdDesignNeeded" << designId << "File loaded" << cacheFileName << !rooms.isEmpty()
				//<< design.m_id << design.m_name;
			AuBdDesign* scd = dynamic_cast<AuBdDesign*>(d);
			if(scd && scd->State() == (AuDesign::STATE)designState)
			{
				// We appear to have loaded the design and the state is correct.
				// Insure no other thread slipped it into the data somehow.
				DgData& sdd = g_Au.LockBdDesignsForWrite();
				if(!sdd.BdDesignIsLoaded(designId))
					sdd.BdDesignAdd(*scd);						// Add the design to our local data.
				g_Au.UnlockBdDesigns();
				//qDebug() << "AuApp::BdDesignNeeded" << designId << "done";

				hasState = scd->State();
				hasDesign = true;
			}
			delete d;
		}
	}

	if(hasDesign && hasState == designState)
	{
		// We appear to have the correct design and the state matches the server.
		// Notify the server that we have loaded this design from local cache.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_BdDesignHere;
		out << designId;
		QueueChatPacket(ba);
	}
	else
	{
		// Design was not loaded from a cache file.
		// Ask the server to send me the design.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_BdDesign;
		out << designId;
		QueueChatPacket(ba);
	}
}

void AuApp::BdDesign(AuBdDesign rec)
{
	DgData& sdd = g_Au.LockBdDesignsForWrite();
	//bool hasDesign = sdd.BdDesignIsLoaded(rec.Id());
	sdd.BdDesignIsLoaded(rec.Id());
	sdd.BdDesignAdd(rec);						// Add the design record, possibly for the first time this session.
	g_Au.UnlockBdDesigns();

	// If the spacecraft design record is not present, don't signal the change.
	// Spacecraft that have been waiting for the design to arrive will now successfully analyze their designs for the first
	// time, simply by finally finding the design record to be present. Then they will create the appropriate models.
	// This logic prevents us from erasing the cached paths data until the design really does change, not simply because
	// we received it for the first time during this session.
	//if(hasDesign)
		//BdDesignChanged(rec.Id());

	// This the final packet of a design received by the client.
	// If the design is read-only, we can cache it locally. Then we don't have to ask for it next time.
	// A design in progress in the design studio will not be read-only so it will not be cached, fortunately.
	if(rec.IsReadOnly())
	{
		// The design can be stored to a cache file.
		//qDebug() << "AuApp::BdDesign" << rec.m_id << rec.m_name << "storing into cache file";
		// Insure the cache folder exists.
		if(g_Au.CreateCacheGame())
		{
			// The cache folder exists.
			QDir cacheDir(g_Au.CacheGame());
			QString cacheFileName = cacheDir.absoluteFilePath(QString("BD%1").arg(rec.Id()));
			rec.SaveToFile(cacheFileName, SHIP_DESIGN_CACHE);
		}
		//qDebug() << "AuApp::BdDesign" << rec.m_id << rec.m_name << "storing into cache file, done.";
	}
}

void AuApp::BdDesignDelete(quint32 designId)
{
	DgData& sdd = g_Au.LockBdDesignsForWrite();
	sdd.BdDesignRemove(designId);								// Remove design and associated recs.
	g_Au.UnlockBdDesigns();
}

void AuApp::OperateTurboLiftUI(AuId shipId, AuId personId, quint16 liftId)
{
	AuFrame* frame = Frame();
	frame->OperateTurboLift(shipId, personId, liftId);
}

void AuApp::BdExchangeRequestDesigns()
{
	if(IsSingleUserMode())
	{
		// Get my empire.
		quint32 empireId = 0;
		AuId personId;
		DaPerson* person = m_I->LockAvatar();
		if(person)
		{
			personId = person->Id();
			empireId = person->EmpireId();
		}
		m_I->UnlockAvatar();

		const DgData& sdl = g_Au.LockBdExchangeForRead();
		AuBdDesignExchange lib;
		sdl.m_BdExchange->ListAndRate(lib, personId, empireId, *sdl.m_BdExchangeRatings);   // List designs available to me.
		g_Au.UnlockBdExchange();

		// Send the list of design ids and last modified times. The client will then request the specific entries needed.
		QHash<quint32, AuTime> toc; // Designs we will specifically request from the server.
		for(AuBdDesignExchange::const_iterator li = lib.cbegin(); li != lib.cend(); li++)
			toc.insert((*li).DesignId(), (*li).LastMod());

		QList<quint32> designsNeeded;
		g_AuClient.BdExchangeToC(toc, designsNeeded);
		BdExchangeDesignsNeeded(designsNeeded);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_BdExchange;
		out << (quint8)10;											// List ToC of published blueprints available in the exchange.
		QueueChatPacket(ba);
	}
}

void AuApp::SceneSyncComplete()
{
	m_SyncDone = true;		// Set the sync done flag.
	if(!m_InitSyncDone)
	{
		m_InitSyncDone = true;
		if(!m_Model->isRunning())
			m_Model->start();
	}
}

bool AuApp::SceneSyncReset()
{
	if(!m_SyncDone)
		return false;
	m_SyncDone = false;		// Clear the sync done flag.
	return true;
}

void AuApp::SendControlPackets()
{
	QList<QByteArray> controls;
	g_AuClient.TakeControlQueue(controls);
	for(QList<QByteArray>::const_iterator ci = controls.cbegin(); ci != controls.cend(); ci++)
		SendUdpPacket(*ci);
}

void AuApp::PrepareToQuit()
{
	// Save the solo universe if an avatar was deleted or if they actually entered the solo universe.
	bool saveCheckPoint = RunState() == Solitaire;
	bool saveSolo = m_SoloAvatarDeleted || saveCheckPoint;
	
	TerminateSound();	// Kill sounds immediately or they linger on while we shut things down.

	// Logout from server(s)
	Logout();

	// Stop atom manager threads.
	if(g_Au.AtomManagerDispatch())
	{
		delete g_Au.AtomManagerDispatch();
		g_Au.SetAtomManagerDispatch(0);
	}

	// Give threads a chance to die before destroying them.
	StopThreads();

	m_Model->Kill();
	if(saveSolo)
	{
		// Wait for model thread to finish so we can save the game.
		AuElapsedTimer t;
		t.start();
		while(m_Model->isRunning())
		{
			if(t.elapsed() > 1000)
			{
				if(g_Au.m_LogMessages & LOG_Thread)
					qDebug() << "Waiting for the model thread to die.";
				t.restart();
			}
		}
		m_Model = 0;
		if(g_Au.m_LogMessages & LOG_Thread)
			qDebug("AuApp::PrepareToQuit - Model thread stopped");

		// Save the solitaire game universe.
		SaveSolitaire();

		if(saveCheckPoint)
			SaveCheckPointToday();
	}
}

void AuApp::ReportMemoryUsage()
{
	ReportMemoryUsage(false);
}

void AuApp::ReportMemoryUsage(bool noLocks)
{
	qDebug() << "App" << sizeof(AuApp) << "bytes";

	qDebug() << "DgData";
	if(!noLocks)
		g_Au.LockDataForRead();
	QStringList details;
	quint64 data = m_Data->MemoryUsage(details);
	if(!noLocks)
		g_Au.UnlockData();
	for(QStringList::const_iterator si = details.cbegin(); si != details.cend(); si++)
		qDebug() << *si;

	quint64 atomMem = 0;
	quint64 atoms = 0;
	QHash<DaAtom::ATOM_TYPE, quint64> memByType;
	AuEther& ether = g_Au.LockEtheriumDelete();
	QList<AuId> atomIds = ether.AtomIds();
	for(QList<AuId>::const_iterator ai = atomIds.cbegin(); ai != atomIds.cend(); ai++)
	{
		DaAtom* atom = ether.FindAtom(*ai);
		if(atom)
		{
			quint64 mem = atom->MemoryUsage();
			atomMem += mem;
			atoms++;
			QHash<DaAtom::ATOM_TYPE, quint64>::iterator ti = memByType.find(atom->AtomType());
			if(ti == memByType.end())
				memByType.insert(atom->AtomType(), 1);
			else
				(*ti) += mem;
		}
	}
	g_Au.UnlockEtheriumDelete();
	qDebug() << "Atoms" << atoms << "using" << atomMem << "bytes";
	for(QHash<DaAtom::ATOM_TYPE, quint64>::const_iterator ti = memByType.cbegin(); ti != memByType.cend(); ti++)
		qDebug() << "  Type" << ti.key() << DaAtom::AtomTypeName(ti.key()) << "using" << *ti << "bytes";

	quint64 total = data + atomMem;
	qDebug() << "Total" << total << "bytes";
}

void AuApp::CreateEmpireUI()
{
	AuFrame* pFrame = Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;
	if(!pView->SceneAtomEmpire())
	{
		// Scene atom has no empire.
		AuCreateEmpire dlg(pFrame);
		pFrame->ExecDialog(dlg);
	}
}

void AuApp::ItDesign(AuItDesign rec)
{
	DgData& sdd = g_Au.LockItDesignsForWrite();
	//bool hasDesign = sdd.ItDesignIsLoaded(rec.Id());
	sdd.ItDesignIsLoaded(rec.Id());
	sdd.ItDesignAdd(rec);						// Add the design record, possibly for the first time this session.
	g_Au.UnlockItDesigns();

	// If the spacecraft design record is not present, don't signal the change.
	// Spacecraft that have been waiting for the design to arrive will now successfully analyze their designs for the first
	// time, simply by finally finding the design record to be present. Then they will create the appropriate models.
	// This logic prevents us from erasing the cached paths data until the design really does change, not simply because
	// we received it for the first time during this session.
	//if(hasDesign)
		//ItDesignChanged(rec.Id());

	// This the final packet of a design received by the client.
	// If the design is read-only, we can cache it locally. Then we don't have to ask for it next time.
	// A design in progress in the design studio will not be read-only so it will not be cached, fortunately.
	if(rec.IsReadOnly())
	{
		// The design can be stored to a cache file.
		//qDebug() << "AuApp::ItDesign" << rec.m_id << rec.m_name << "storing into cache file";
		// Insure the cache folder exists.
		if(g_Au.CreateCacheGame())
		{
			// The cache folder exists.
			QDir cacheDir(g_Au.CacheGame());
			QString cacheFileName = cacheDir.absoluteFilePath(QString("IT%1").arg(rec.Id()));
			rec.SaveToFile(cacheFileName, SHIP_DESIGN_CACHE);
		}
		//qDebug() << "AuApp::ItDesign" << rec.m_id << rec.m_name << "storing into cache file, done.";
	}
}

void AuApp::ItDesignDelete(quint32 designId)
{
	DgData& sdd = g_Au.LockItDesignsForWrite();
	sdd.ItDesignRemove(designId);								// Remove design and associated recs.
	g_Au.UnlockItDesigns();
}

// Scene says I need a design.
void AuApp::ItDesignNeeded(quint32 designId)
{
	// Test to see if we have the design.
	const DgData& sdd = g_Au.LockItDesignsForRead();
	bool hasDesign = sdd.ItDesignIsLoaded(designId);
	g_Au.UnlockItDesigns();
	if(hasDesign)
		return;													// We have the design now.

	// We do not have the design.
	// Try to load the design from a cache file.
	QDir cacheDir(g_Au.CacheGame());
	QString cacheFileName = cacheDir.absoluteFilePath(QString("IT%1").arg(designId));
	AuDesign* d = AuDesign::LoadFromFile(cacheFileName, SHIP_DESIGN_CACHE);
	if(d)
	{
		//qDebug() << "AuApp::ItDesignNeeded" << designId << "File loaded" << cacheFileName << !rooms.isEmpty()
			//<< design.m_id << design.m_name;
		AuItDesign* scd = dynamic_cast<AuItDesign*>(d);
		if(scd)
		{
			// We appear to have loaded the proper type of design.
			// Insure no other thread slipped it into the data somehow.
			DgData& sdd = g_Au.LockItDesignsForWrite();
			if(!sdd.ItDesignIsLoaded(designId))
				sdd.ItDesignAdd(*scd);							// Add the design to our local data.
			g_Au.UnlockItDesigns();
			//qDebug() << "AuApp::ItDesignNeeded" << designId << "done";

			// Notify the server that we have loaded this design from local cache.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtChatChannelPacket::PD_ItDesignHere;
			out << designId;
			QueueChatPacket(ba);

			delete d;
			return;
		}
		else
			delete d;
	}

	// Design was not loaded from a cache file.
	// Ask the server to send me the design.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtChatChannelPacket::PD_ItDesign;
	out << designId;
	QueueChatPacket(ba);
}

// Scene says I need a design with a specific state.
void AuApp::ItDesignNeeded(quint32 designId, quint8 designState)
{
	// Test to see if we have the design.
	const DgData& sdd = g_Au.LockItDesignsForRead();
	bool hasDesign = sdd.ItDesignIsLoaded(designId);
	quint8 hasState = sdd.ItDesignState(designId);
	g_Au.UnlockItDesigns();

	if(!hasDesign)
	{
		// We do not have the design.
		// Try to load the design from a cache file.
		QDir cacheDir(g_Au.CacheGame());
		QString cacheFileName = cacheDir.absoluteFilePath(QString("IT%1").arg(designId));
		AuDesign* d = AuDesign::LoadFromFile(cacheFileName, SHIP_DESIGN_CACHE);
		if(d)
		{
			//qDebug() << "AuApp::ItDesignNeeded" << designId << "File loaded" << cacheFileName << !rooms.isEmpty()
				//<< design.m_id << design.m_name;
			AuItDesign* scd = dynamic_cast<AuItDesign*>(d);
			if(scd && scd->State() == (AuDesign::STATE)designState)
			{
				// We appear to have loaded the design and the state is correct.
				// Insure no other thread slipped it into the data somehow.
				DgData& sdd = g_Au.LockItDesignsForWrite();
				if(!sdd.ItDesignIsLoaded(designId))
					sdd.ItDesignAdd(*scd);						// Add the design to our local data.
				g_Au.UnlockItDesigns();
				//qDebug() << "AuApp::ItDesignNeeded" << designId << "done";

				hasState = scd->State();
				hasDesign = true;
			}
			delete d;
		}
	}

	if(hasDesign && hasState == designState)
	{
		// We appear to have the correct design and the state matches the server.
		// Notify the server that we have loaded this design from local cache.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ItDesignHere;
		out << designId;
		QueueChatPacket(ba);
	}
	else
	{
		// Design was not loaded from a cache file.
		// Ask the server to send me the design.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ItDesign;
		out << designId;
		QueueChatPacket(ba);
	}
}

void AuApp::GoLand(AuId worldId, QString worldName)
{
	AuMissionOrderGoToLand order(worldId, worldName);
	if(!order.IsValid())
		return;

	if(IsSingleUserMode())
	{
		m_I->MissionAppendOrder(order);
	}
	else
	{
		// Send the mission add command request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PN_Mission;
		out << (quint8)2;					// add mission order
		out << (quint8)order.Type();
		out << order;
		QueueChatPacket(ba);
	}
}

void AuApp::GoToCoord(AuPoint systemLoc)
{
	AuMissionOrderGoToCoordinate order(AuMissionOrderGoToCoordinate::SolarSystem, systemLoc);
	if(!order.IsValid())
		return;

	if(IsSingleUserMode())
	{
		m_I->MissionAppendOrder(order);
	}
	else
	{
		// Send the mission add command request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PN_Mission;
		out << (quint8)2;					// add mission order
		out << (quint8)order.Type();
		out << order;
		QueueChatPacket(ba);
	}
}

void AuApp::Attack(AuId targetId, QString targetName)
{
	AuMissionOrderAttack order(targetId, targetName);
	if(!order.IsValid())
		return;

	if(IsSingleUserMode())
	{
		m_I->MissionAppendOrder(order);
	}
	else
	{
		// Send the mission add command request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PN_Mission;
		out << (quint8)2;					// add mission order
		out << (quint8)order.Type();
		out << order;
		QueueChatPacket(ba);
	}
}

void AuApp::Follow(AuId targetId, QString targetName)
{
	AuMissionOrderFollow order(targetId, targetName);
	if(!order.IsValid())
		return;

	if(IsSingleUserMode())
	{
		m_I->MissionAppendOrder(order);
	}
	else
	{
		// Send the mission add command request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PN_Mission;
		out << (quint8)2;					// add mission order
		out << (quint8)order.Type();
		out << order;
		QueueChatPacket(ba);
	}
}

void AuApp::InitializeSound()
{
	Q_ASSERT(!m_AL);	// Don't call this twice.
	m_AudioSource = 0;
	for(int cs=0; cs<CLIENT_SOUNDS; cs++)
	{
		m_Audio[cs] = 0;
		m_Sound[cs] = SOUND_None;
	}

	m_AL = new AuSound;
	m_AL->InitializeAudio();
}

void AuApp::TerminateSound()
{
	if(!m_AL)
		return;
	//qDebug() << "AuApp::TerminateSound destroying OpenAL context";
	delete m_AL;
	m_AL = 0;
}

void AuApp::PlayClientSound(int sound, float gain, float pitch)
{
	if(!m_AL)
		return;
	gain *= g_Au.SoundEffectsGain();
	SOUND_EFFECT cur;
	for(int i = 0; i < CLIENT_SOUNDS; i++)
	{
		if(!m_AL->IsSoundPlaying(m_Audio[i]))
		{
			cur = (SOUND_EFFECT)m_Sound[i];
			m_AL->Play(m_Audio[i], cur, (SOUND_EFFECT)sound, 0,0,0, gain, pitch, 0, 1,0, 0);
			m_Sound[i] = cur;
			break;
		}
	}
}

// Render OpenAL audio sources
void AuApp::PlayQueuedSounds(float gain)
{
	//qDebug() << "AuApp::PlayQueuedSounds" << m_SoundQueue.count() << "gain" << gain;
	if(!m_AL)
		return;
	if(!m_SoundQueue.isEmpty())
	{
		// The sound queue has a sound to play.
		if(m_SoundQueue.first().IsPlaying())
		{
			// The first sound in the queue has been started already.
			if(m_AL->IsSoundPlaying(m_AudioSource))
			{
				// The sound source is still playing the sound.
				// Update the sound source parameters for the playing sound, such as position and gain.
				m_AL->Play(m_AudioSource, m_SoundQueue.first().AudioBuffer(), 0,0,0, gain*g_Au.SoundEffectsGain(), 1, 1,0);
			}
			else
			{
				// The current sound source has finished playing. Clean up.
				m_AL->DeleteSound(m_SoundQueue.first().AudioBuffer());	// Delete the OpenAL sound buffer.
				m_SoundQueue.pop_front();							// Erase the sound entry from the array.
			}
		}
		else
		{
			// Start playing the next sound in the sound source.
			m_SoundQueue.first().StartPlaying();
			m_AL->Play(m_AudioSource, m_SoundQueue.first().AudioBuffer(), 0,0,0, gain*g_Au.SoundEffectsGain(), 1, 1,0);
		}
	}
}

void AuApp::SoundGainChanged(int sound)
{
	if(!m_AL)
		return;

	float gain = g_Au.SoundGain((SOUND_EFFECT)sound);
	m_AL->SetSoundGain((SOUND_EFFECT)sound, gain);
}

void AuApp::SoundOverrideChanged(int sound)
{
	if(!m_AL)
		return;

	StopClientSound((SOUND_EFFECT)sound);

	// Get the new sound override, which might be no longer overridden from the default.
	AuSoundOverride sndOver;
	g_Au.SoundOverride((SOUND_EFFECT)sound, sndOver);
	m_AL->SetSoundOverride((SOUND_EFFECT)sound, sndOver);
}

void AuApp::QueueSound(const QByteArray& soundData)
{
	if(!m_AL)
		return;
	if(!soundData.isEmpty())
	{
		AuCitizenSound sound(soundData, *m_AL);
		if(sound.IsValid())
			m_SoundQueue.append(AuCitizenSound(soundData, *m_AL));
		else
		{
			// Sometimes the sound buffer creation fails with ALUT_ERROR_INVALID_OPERATION.
			// ALUT doc says this means, "ALUT has not been initialised." However, I know alutInit has been called by now.
			// The buffer creation usually works on the second try.
			AuCitizenSound sound(soundData, *m_AL);
			if(sound.IsValid())
				m_SoundQueue.append(AuCitizenSound(soundData, *m_AL));
			else
			{
				// I've never seen the buffer creation fail on the second try but it we'll try a third time just to be sure.
				AuCitizenSound sound(soundData, *m_AL);
				if(sound.IsValid())
					m_SoundQueue.append(AuCitizenSound(soundData, *m_AL));
			}
		}
	}
}

void AuApp::StopClientSound(int sound)
{
	if(!m_AL)
		return;
	for(int i = 0; i < CLIENT_SOUNDS; i++)
		if(m_Sound[i] == sound)
		{
			m_AL->StopSound(m_Audio[i]);
			m_Sound[i] = SOUND_None;
		}
}

void AuApp::StopClientSounds()
{
	if(!m_AL)
		return;
	for(int i = 0; i < CLIENT_SOUNDS; i++)
		if(m_Sound[i] != SOUND_None)
		{
			m_AL->StopSound(m_Audio[i]);
			m_Sound[i] = SOUND_None;
		}
}

bool AuApp::IsClientSoundPlaying(int sound) const
{
	if(!m_AL)
		return false;
	for(int i = 0; i < CLIENT_SOUNDS; i++)
		if(m_Sound[i] == sound && m_AL->IsSoundPlaying(m_Audio[i]))
			return true;
	return false;
}

void AuApp::SaveSolitaire() const
{
	// Insure the cache folder exists.
	if(!g_Au.CreateGameSaveDir())
		return;		// Game save folder does not exist?

	// Create temp files.
	QTemporaryFile avTmp("tmp.avXXXXXX");
	QTemporaryFile dbTmp("tmp.dbXXXXXX");
	if(!avTmp.open() || !dbTmp.open())
		return;

	// Save to temp files.
	if(!SaveSolitaireAvatarsFile(avTmp))
		return;	// Couldn't save avatars file so don't bother with universe.
	SaveSolitaireUniverseFile(dbTmp);

	// Replace the save files with the temp files.
	QDir cacheDir(g_Au.GameSaveDir());
	QString avFile = cacheDir.absoluteFilePath(s_LoginFile);		// Final login save file.
	QString dbFile = cacheDir.absoluteFilePath(s_UniverseFile);		// Final universe save file.
	QFile::remove(avFile);
	QFile::remove(dbFile);
	avTmp.copy(avFile);
	dbTmp.copy(dbFile);
}

void AuApp::SaveCheckPointToday() const
{
	// Insure the cache folder exists.
	if(!g_Au.CreateGameSaveDir())
		return;		// Game save folder does not exist?

	// Check to see if there is already a checkpoint with today's date.
	QDateTime now = QDateTime::currentDateTime();
	QDir cacheDir(g_Au.GameSaveDir());
	QStringList nameFilters;
	nameFilters << "sU.*";
	QStringList fileEntries = cacheDir.entryList(nameFilters);
	for(QStringList::const_iterator fi = fileEntries.constBegin(); fi != fileEntries.constEnd(); fi++)
	{
		QStringList fields = (*fi).split('.');
		if(fields.size() < 2)
			continue;
		QDateTime dt = QDateTime::fromString(fields[1], Qt::ISODate);
		if(dt.date() == now.date())
			return;	// Found a save with today's date.
	}

	// No save was found with today's date. Create a check point save.
	SaveCheckPoint(QString());
}

void AuApp::SaveCheckPoint(const QString& name) const
{
	QDateTime now = QDateTime::currentDateTime();
	QString avFileName = QString("%1%2")
		.arg(s_LoginPrefix)
		.arg(now.toString(Qt::ISODate));
	if(!name.isEmpty())
		avFileName += "." + name;
	if(!SaveSolitaireAvatars(avFileName))
		return;	// Couldn't save avatars file so don't bother with universe.

	QString uFileName = QString("%1%2")
		.arg(s_UniversePrefix)
		.arg(now.toString(Qt::ISODate));
	if(!name.isEmpty())
		uFileName += "." + name;
	SaveSolitaireUniverse(uFileName);

	// The empty name only comes from SaveCheckPointToday, during program shutdown. A thought acknowledgement is not wanted then.
	if(name.isEmpty())
		return;

	AuId personId;
	QString personName;
	m_I->Avatar(personId, personName);
	if(!personId.IsNull())
	{
		AuCommThought msg(tr("Game Saved"), personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuApp::SaveSolitaireUniverse(const QString& fileName) const
{
	// Insure the cache folder exists.
	if(!g_Au.CreateGameSaveDir())
		return;		// Game save folder does not exist?

	// The cache folder exists.
	QDir cacheDir(g_Au.GameSaveDir());
	QString dbFileName = cacheDir.absoluteFilePath(fileName);
	QFile dbFile(dbFileName);
	if(dbFile.open(QFile::WriteOnly))
	{
		SaveSolitaireUniverseFile(dbFile);
		dbFile.close();
	}
}

void AuApp::SaveSolitaireUniverseFile(QFile& file) const
{
	// Prepare to write to the db file.
	QDataStream dsfile(&file);
	dsfile.setVersion(QDataStream::Qt_5_0);

	// Write a header to the file.
	const quint8 fileType = SOLO_GAME_CACHE;
	const quint8 fileVersion = SOLO_GAME_VER;
	dsfile << fileType;
	dsfile << fileVersion;

	// Global values
	g_AuClient.SaveSolitaire(dsfile);

	// Data values
	m_Data->SaveSolitaire(dsfile);

	// Ether
	const AuEther& ether = g_Au.LockEtheriumDelete();
	dsfile << ether;
	g_Au.UnlockEtheriumDelete();
}

bool AuApp::SaveSolitaireAvatars(const QString& fileName) const
{
	// Insure the cache folder exists.
	if(!g_Au.CreateGameSaveDir())
		return false;		// Game save folder does not exist?

	// The cache folder exists.
	AuId avatarIds[6];
	bool allNull = true;
	const DgData& gd = g_Au.LockDataForRead();
	for(int ai=0; ai<6; ai++)
	{
		avatarIds[ai] = gd.ClientUnitId(ai);
		if(!avatarIds[ai].IsNull())
			allNull = false;
	}
	g_Au.UnlockData();
	if(allNull)
		return false;

	// The cache folder exists.
	bool success = false;
	QDir cacheDir(g_Au.GameSaveDir());
	QString avFileName = cacheDir.absoluteFilePath(fileName);
	QFile avFile(avFileName);
	if(avFile.open(QFile::WriteOnly))
	{
		success = SaveSolitaireAvatarsFile(avFile);
		avFile.close();
	}
	return success;
}

bool AuApp::SaveSolitaireAvatarsFile(QFile& file) const
{
	AuId avatarIds[6];
	bool allNull = true;
	const DgData& gd = g_Au.LockDataForRead();
	for(int ai=0; ai<6; ai++)
	{
		avatarIds[ai] = gd.ClientUnitId(ai);
		if(!avatarIds[ai].IsNull())
			allNull = false;
	}
	g_Au.UnlockData();
	if(allNull)
		return false;

	// Prepare to write to the av file.
	QDataStream dsfile(&file);
	dsfile.setVersion(QDataStream::Qt_5_0);

	// Write a header to the file.
	const quint8 fileType = SOLO_AVATAR_CACHE;
	const quint8 fileVersion = SOLO_AVATAR_VER;
	dsfile << fileType;
	dsfile << fileVersion;

	// Ether
	const AuEther& ether = g_Au.LockEtheriumDelete();
	// Count the number of avatars we will be saving.
	quint8 howMany = 0;
	for(int ai=0; ai<6; ai++)
		if(!avatarIds[ai].IsNull() && ether.FindAtom(avatarIds[ai]))
			howMany++;
	dsfile << howMany;

	// Save the avatar atom(s).
	for(int ai=0; ai<6; ai++)
	{
		if(avatarIds[ai].IsNull())
			continue;
		DaAtom* person = ether.FindAtom(avatarIds[ai]);
		if(!person)
			continue;
		dsfile << (quint8)person->AtomType();
		dsfile << *person;
	}
	g_Au.UnlockEtheriumDelete();

	if(!howMany)
		return false;	// failure
	return true;
}

void AuApp::ClearSavedGames()
{
	// Insure the game save folder exists.
	if(!g_Au.CreateGameSaveDir())
		return;		// Game save folder does not exist?

	// Make a filter to select the files.
	QStringList nameFilters;
	nameFilters << "sA.*";
	nameFilters << "sU.*";

	// Make a list of the files using the filter.
	QDir tempDir(g_Au.GameSaveDir());
	QStringList delFiles = tempDir.entryList(nameFilters);

	// Delete the files.
	for(QStringList::const_iterator dfi = delFiles.constBegin(); dfi != delFiles.constEnd(); dfi++)
		tempDir.remove(*dfi);
}

void AuApp::ClearSolitaire()
{
	// Clear the Etherium.
	g_Au.ClearEtherium();

	// Remove the solo avatars file of the current game.
	QDir gameSaveDir(g_Au.GameSaveDir());
	QString cacheFileName = gameSaveDir.absoluteFilePath(s_LoginFile);
	QFile::remove(cacheFileName);

	// Remove the solo universe file of the current game.
	cacheFileName = gameSaveDir.absoluteFilePath(s_UniverseFile);
	QFile::remove(cacheFileName);

	// Remove solo cache files.
	g_Au.ClearSoloCacheFiles();
}

bool AuApp::RestoreSolitaireUniverse()
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	// Create a temporary file.
	QTemporaryFile tmpFile("tmp.dbXXXXXX");
	tmpFile.open();
	tmpFile.close();
	QString dbTmp = tmpFile.fileName();

	// Build file names.
	QDir cacheDir(g_Au.GameSaveDir());
	QString dbFile = cacheDir.absoluteFilePath(s_UniverseFile);		// Actual universe save file.

	// So as not to monopolize the universe file, especially if my process hangs, we will make a copy of the file.
	tmpFile.remove();
	QFile::copy(dbFile, dbTmp);

	// Now we will load from the temp file.
	if(!tmpFile.open())
	{
		QApplication::restoreOverrideCursor();
		return false;	// Unable to open the file?
	}

	// Load from cache file.
	QDataStream dsfile(&tmpFile);
	dsfile.setVersion(QDataStream::Qt_5_0);

	// Read the header from the file.
	quint8 fileType;
	quint8 fileVersion;
	dsfile >> fileType;
	dsfile >> fileVersion;
	if(fileType != SOLO_GAME_CACHE || fileVersion != SOLO_GAME_VER)
	{
		QApplication::restoreOverrideCursor();
		return false;	// Bad file type or version.
	}

	// Global values
	g_AuClient.RestoreSolitaire(dsfile);

	// Data values
	m_Data->InitSolitaire();			// Init to pick up most recent changes to solo game data GS file, including designs added.
	m_Data->RestoreSolitaire(dsfile);	// Merges with existing data.

	// Story scripts need empires.
	g_AuClient.LoadScripts();

	// Ether
	g_AuClient.LoadEther(dsfile);

	AuEther& ether = g_Au.LockEtheriumDelete();
	ether.DecaySolo();
	g_Au.UnlockEtheriumDelete();

	QApplication::restoreOverrideCursor();
	return true;
}

int AuApp::RestoreSolitaireAvatar(DaPerson* avatars[6])
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	QDir cacheDir(g_Au.GameSaveDir());
	QString cacheFileName = cacheDir.absoluteFilePath(s_LoginFile);
	QFile cacheFile(cacheFileName);
	if(!cacheFile.open(QIODevice::ReadOnly))
	{
		QApplication::restoreOverrideCursor();
		return 1;	// Unable to open the file?
	}

	// Load from cache file.
	QDataStream dsfile(&cacheFile);
	dsfile.setVersion(QDataStream::Qt_5_0);

	// Read the header from the file.
	quint8 fileType;
	quint8 fileVersion;
	dsfile >> fileType;
	dsfile >> fileVersion;
	if(fileType != SOLO_AVATAR_CACHE || fileVersion != SOLO_AVATAR_VER)
	{
		QApplication::restoreOverrideCursor();
		return 2;	// Bad file type or version.
	}

	quint8 howMany;
	dsfile >> howMany;
	if(howMany == 0)
	{
		QApplication::restoreOverrideCursor();
		return 3;	// No avatars in file.
	}
	else if(howMany > 6)
	{
		QApplication::restoreOverrideCursor();
		return 4;	// Bad avatar count.
	}

	// Load the avatar(s) from the file.
	for(int ai=0; ai<howMany; ai++)
	{
		quint8 atomType;
		dsfile >> atomType;
		if(atomType != DaAtom::ATOM_Avatar)
		{
			//qDebug() << "AuApp::RestoreSolitaireAvatar bad atom type" << atomType;
			QApplication::restoreOverrideCursor();
			return 5;	// Bad atom type.
		}

		DaPerson* av = new DaPerson();
		dsfile >> *av;
		avatars[ai] = av;
		//qDebug() << "AuApp::RestoreSolitaireAvatar restored" << *av;
	}

	// Zero the remaining avatar pointers in the return array.
	for(int ai=howMany; ai<6; ai++)
		avatars[ai] = 0;
	QApplication::restoreOverrideCursor();
	return 0;	// Good return.
}

void AuApp::DeleteSolitaireAvatar(const AuId& avatarId)
{
	// Trash the avatar atom.
	if(!avatarId.IsNull())
	{
		AuEther& ether = g_Au.LockEtheriumDelete();
		ether.TrashAtom(avatarId);
		g_Au.UnlockEtheriumDelete();
	}

	// Determine if any solitaire avatars remain.
	bool anyRemain = false;
	const DgData& gd = g_Au.LockDataForRead();
	for(int ai=0; ai<6; ai++)
		if(!gd.ClientUnitId(ai).IsNull())
		{
			anyRemain = true;
			break;
		}
	g_Au.UnlockData();

	// When no solitaire avatars remain, delete the solitaire universe. This does not delete their saved games, just the current.
	if(!anyRemain)
		ClearSolitaire();
	else
		m_SoloAvatarDeleted = true;	// So we'll save the solo universe on exit, if they exit without playing.
}

const AuId& AuApp::AvatarId() const
{
	return m_I->AvatarId();
}


void AuApp::SetAvatarId(const AuId& id)
{
	m_I->SetAvatarId(id);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuApp::DesignExit()
{
	if(IsSingleUserMode())
	{
		m_I->DesignExit();
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_Exit;
		QueueChatPacket(ba);
	}

	Frame()->DesignerExited();
}

void AuApp::DesignOpen(quint32 designId, int designMode)
{
	if(!designId)
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignOpen(designId, designMode);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_Open;
		out << designId;
		out << (quint8)designMode;
		QueueChatPacket(ba);
	}
}

bool AuApp::DesignIsGravityOn() const
{
	return m_I->DesignIsGravityOn();
}

bool AuApp::DesignIsRenderShaded() const
{
	return m_I->DesignIsRenderShaded();
}

int AuApp::DesignMode() const
{
	return m_I->DesignMode();
}

QString AuApp::DesignName() const
{
	return m_I->DesignName();
}

void AuApp::DropGearItem(AuGearPos pos)
{
	if(IsSingleUserMode())
	{
		m_I->DropGearItem(pos);
	}
	else
	{
		// Transmit the data to the server.
		// The reply arrives as changes to my dude and/or the terrain atoms.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_Drop;
		out << pos;
		QueueChatPacket(ba);
	}
}

void AuApp::RedeemGearItem(AuGearPos pos)
{
	if(IsSingleUserMode())
	{
		m_I->CollectBounty(pos);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_GearItem;
		out << pos;
		out << (quint8)3;	// 3 = collect bounty
		QueueChatPacket(ba);
	}
}

void AuApp::InscribeGearItem(AuGearPos pos, QString inscription)
{
	if(IsSingleUserMode())
	{
		m_I->InscribeGearItem(pos, inscription);
	}
	else
	{
		// Transmit the data to the server.
		// The reply arrives as changes to my dude and/or the terrain atoms.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_Inscribe;
		out << pos;
		out << inscription;
		QueueChatPacket(ba);
	}
}

void AuApp::MoveGearItemToItem(AuGearPos fromPos, AuGearPos toPos)
{
	if(IsSingleUserMode())
	{
		m_I->MoveGearItemToItem(fromPos, toPos);
	}
	else
	{
		// Transmit the data to the server.
		// The reply arrives as changes to my dude's atom.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_GearItem;
		out << fromPos;
		out << (quint8)0;	// 0 = existing item pos
		out << toPos;
		QueueChatPacket(ba);
	}
}

void AuApp::MoveGearItemToSlot(AuGearPos fromPos, quint8 toSlotType)
{
	if(IsSingleUserMode())
	{
		m_I->MoveGearItemToSlot(fromPos, toSlotType);
	}
	else
	{
		// Transmit the data to the server.
		// The reply arrives as changes to my dude's atom.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_GearItem;
		out << fromPos;
		out << (quint8)1;	// 1 = new empty top level slot
		out << toSlotType;
		QueueChatPacket(ba);
	}
}

void AuApp::UseGearItem(AuGearPos pos)
{
	if(IsSingleUserMode())
	{
		m_I->UseGearItem(pos);
	}
	else
	{
		// Transmit the data to the server.
		// The reply arrives as changes to my dude and/or the terrain atoms.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_Use;
		out << pos;
		QueueChatPacket(ba);
	}
}

void AuApp::QueryJobs()
{
	if(IsSingleUserMode())
	{
		QMultiMap<int, AuJobOffer> jobsToStart;
		m_I->QueryJobs(jobsToStart);
		for(QMultiMap<int, AuJobOffer>::const_iterator si = jobsToStart.cbegin(); si != jobsToStart.cend(); si++)
			emit JobAvailable(*si);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PJ_Query;
		QueueChatPacket(ba);
	}
}

void AuApp::QueryJobs(quint16 maxSectorsAway, bool typeFilter[AuJob::JobTypes])
{
	// Make a list of only the job types that we want.
	QList<AuJob::TYPE> typesWanted;
	for(quint8 type=0; type<AuJob::JobTypes; type++)
		if(typeFilter[type])
			typesWanted.append((AuJob::TYPE)type);

	if(IsSingleUserMode())
	{
		QMultiMap<int, AuJobOffer> jobsToStart;
        m_I->QueryJobs(jobsToStart, typesWanted, maxSectorsAway);
        for(QMultiMap<int, AuJobOffer>::const_iterator si = jobsToStart.cbegin(); si != jobsToStart.cend(); si++)
            emit JobAvailable(*si);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PJ_QueryFilter;
		out << maxSectorsAway;
		out << (quint8)typesWanted.count();
		for(QList<AuJob::TYPE>::const_iterator twi = typesWanted.cbegin(); twi != typesWanted.cend(); twi++)
			out << (quint8)*twi;
		QueueChatPacket(ba);
	}
}

void AuApp::ChoicePicked(quint32 storyInstance, qint8 value, AuId offeredBy)
{
	if(IsSingleUserMode())
	{
		m_I->StoryChoicePicked(storyInstance, value, offeredBy);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PS_ChoiceMade;
		out << storyInstance;
		out << value;
		out << offeredBy;
		QueueChatPacket(ba);
	}

	// UIs need to disable or hide story choices after one has been picked, so they can't be picked again.
	Frame()->StoryChoicePickedCleanup(storyInstance, value, offeredBy);
}

void AuApp::GovDossierAddNote(quint32 dosEmpire, const QString& docNote)
{
	if(docNote.isEmpty())
		return;
	if(IsSingleUserMode())
	{
		m_I->GovDossierAddNote(dosEmpire, docNote);
	}
	else
	{
		quint8 doWhat = 0;	// Add Note

		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_Dossier;
		out << doWhat;
		out << dosEmpire;
		out << docNote;
		QueueChatPacket(ba);
	}
}

void AuApp::GovDossierDocState(quint32 dosEmpire, const AuTime& docTime, quint8 docType, quint16 docIndex, quint8 docState)
{
	if(docState >= AuDocument::DOC_Count)
		return;

	if(IsSingleUserMode())
	{
		m_I->GovDossierDocState(dosEmpire, docTime, docType, docIndex, docState);
	}
	else
	{
		quint8 doWhat = 1;	// Change state

		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_Dossier;
		out << doWhat;
		out << dosEmpire;
		out << docTime;
		out << docType;
		out << docIndex;
		out << docState;
		QueueChatPacket(ba);
	}
}

void AuApp::GovDossierDispatchCourier(quint32 toEmpire, const QString& message)
{
	if(message.isEmpty())
		return;
	if(IsSingleUserMode())
	{
		m_I->GovDossierDispatchCourier(toEmpire, message);
	}
	else
	{
		quint8 doWhat = 2;	// Diplomatic message

		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_Dossier;
		out << doWhat;
		out << toEmpire;
		out << message;
		QueueChatPacket(ba);
	}
}

void AuApp::GovSetThemeColor(quint8 theme, const QColor& color)
{
	if(theme >= AuScFace::THEME_Count)
		return;
	if(IsSingleUserMode())
	{
		m_I->GovSetThemeColor(theme, color);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_SetThemeColor;
		out << (quint8)theme;
		out << color;
		QueueChatPacket(ba);
	}
}

void AuApp::GovSetAnnouncements(const QString& announcements)
{
	if(IsSingleUserMode())
	{
		m_I->GovSetAnnouncements(announcements);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_SetAnnouncements;
		out << announcements;
		QueueChatPacket(ba);
	}
}

void AuApp::GovSetWarCriteria(const AuWarCriteria& crit)
{
	if(IsSingleUserMode())
	{
		m_I->GovSetWarCriteria(crit);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_WarCriteria;
		out << crit;
		QueueChatPacket(ba);
	}
}

void AuApp::GovSetCapital(int capitalType)
{
	if(IsSingleUserMode())
	{
		m_I->GovSetCapital(capitalType);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_SetCapital;
		out << (quint8)capitalType;
		QueueChatPacket(ba);
	}
}

void AuApp::GovSetEmpireCapital()
{
	GovSetCapital(1);
}

void AuApp::GovSetGalaxyCapital()
{
	GovSetCapital(3);
}

void AuApp::GovSetSectorCapital()
{
	GovSetCapital(0);
}

void AuApp::GovSetDefaultPoliticalStance(int stance)
{
	if(stance < POL_Friend || stance > POL_Vassal)
		return;
	if(IsSingleUserMode())
	{
		m_I->GovSetDefaultPoliticalStance(stance);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_SetDefStance;
		out << (quint8)stance;
		QueueChatPacket(ba);
	}
}

void AuApp::GovSetContrailColor(quint8 hue, quint8 sat)
{
	if(IsSingleUserMode())
	{
		m_I->GovSetContrailColor(hue, sat);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_ContrailColor;
		out << hue;
		out << sat;
		QueueChatPacket(ba);
	}
}

void AuApp::GovSetTaxes(double income, double sales, double subsidy)
{
	if(income < 0 || income > 100)
		return;
	if(sales < 0 || sales > 100)
		return;
	if(subsidy < 0 || subsidy > 100)
		return;

	quint8 incomeTax = (quint8)(income * 2);	// Truncates to 0-200
	quint8 salesTax = (quint8)(sales * 2);		// Truncates to 0-200
	quint8 subsidyTax = (quint8)(subsidy * 2);	// Truncates to 0-200

	if(IsSingleUserMode())
	{
		m_I->GovSetTax(income, sales, subsidy);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_SetTax;
		out << incomeTax;
		out << salesTax;
		out << subsidyTax;
		QueueChatPacket(ba);
	}
}

void AuApp::GovSetCommerce(QByteArray commerce)
{
	if(commerce.isEmpty())
		return;
	if(IsSingleUserMode())
	{
		m_I->GovSetCommerce(commerce);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_SetCommerce;
		out << commerce;
		QueueChatPacket(ba);
	}
}

// Current player assigns an office to another player.
void AuApp::AssignOffice(const AuId& toWho, const QString& toName, const AuOffice& office)
{
	if(IsSingleUserMode())
	{
		m_I->GovAssignOffice(toWho, toName, office);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_AssignOffice;
		out << toWho;
		out << toName;
		out << office;
		QueueChatPacket(ba);
	}
}

// Current player resigns from office.
void AuApp::ResignOffice()
{
	if(IsSingleUserMode())
	{
		m_I->GovResignOffice();
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_ResignOffice;
		QueueChatPacket(ba);
	}
}

// Current player revokes the office of another player.
void AuApp::RevokeOffice(const AuId& playerId, const QString& playerName)
{
	if(IsSingleUserMode())
	{
		m_I->GovRevokeOffice(playerId, playerName);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_RevokeOffice;
		out << playerId;
		out << playerName;
		QueueChatPacket(ba);
	}
}

void AuApp::KickBerthOccupant(const AuId& occupant)
{
	if(IsSingleUserMode())
	{
		m_I->KickBerthOccupant(occupant);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_KickBerthOccupant;
		out << (quint8)0;						// Kick berth occupant.
		out << occupant;
		QueueChatPacket(ba);
	}
}

void AuApp::ReconcileRoster()
{
	if(IsSingleUserMode())
	{
		m_I->ReconcileRoster();
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_ReconcileRoster;
		QueueChatPacket(ba);
	}
}

void AuApp::ReleaseBerth()
{
	if(IsSingleUserMode())
	{
		m_I->ReleaseBerth();
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_ReleaseBerth;
		QueueChatPacket(ba);
	}
}

void AuApp::ClaimReward(const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->RedeemGovtDebt(opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_ClaimReward;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::AcceptCrew(bool accept)
{
	if(IsSingleUserMode())
	{
		m_I->SetAcceptCrew(accept);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_AcceptCrew;
		out << (quint8)accept;
		QueueChatPacket(ba);
	}
}

void AuApp::AcceptPassengers(bool accept)
{
	if(IsSingleUserMode())
	{
		m_I->SetAcceptPassengers(accept);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_AcceptPassengers;
		out << (quint8)accept;
		QueueChatPacket(ba);
	}
}

void AuApp::AcceptTroops(bool accept)
{
	if(IsSingleUserMode())
	{
		m_I->SetAcceptTroops(accept);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_AcceptTroops;
		out << (quint8)accept;
		QueueChatPacket(ba);
	}
}

void AuApp::AllowPassengerSeaChests(bool allow)
{
	if(IsSingleUserMode())
	{
		m_I->SetAllowPassengerSeaChests(allow);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_AllowPassengerSeaChests;
		out << (quint8)allow;
		QueueChatPacket(ba);
	}
}

void AuApp::AutoFetchVehicles(bool autoFetch)
{
	if(IsSingleUserMode())
	{
		m_I->GovSetPilotPolicy(3, autoFetch);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_SetPilotPolicy;
		out << (quint8)3;
		out << (quint8)autoFetch;
		QueueChatPacket(ba);
	}
}

// Configures shield up policy for the ship the person is currently aboard.
void AuApp::AlertCondition(int condition)
{
	if(IsSingleUserMode())
	{
		m_I->AlertCondition(condition);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_SetPilotPolicy;
		out << (quint8)2;						// 2 = alert condition
		out << (quint8)condition;
		QueueChatPacket(ba);
	}
}

// Configures shield up policy for the ship the person is currently aboard.
void AuApp::PilotUpPolicy(int policy)
{
	if(IsSingleUserMode())
	{
		m_I->GovSetPilotPolicy(0, policy);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_SetPilotPolicy;
		out << (quint8)0;						// 0 = 'pilot up' policy
		out << (quint8)policy;
		QueueChatPacket(ba);
	}
}

// Configures shield fuel policy for the ship the person is currently aboard.
void AuApp::AccessPolicy(int policy)
{
	if(IsSingleUserMode())
	{
		m_I->GovSetPilotPolicy(1, policy);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_SetPilotPolicy;
		out << (quint8)1;						// 1 = access policy
		out << (quint8)policy;
		QueueChatPacket(ba);
	}
}

// Configures shield fuel policy for the ship the person is currently aboard.
void AuApp::ShieldFuelPolicy(int policy)
{
	if(IsSingleUserMode())
	{
		m_I->GovSetShieldPolicy(0, policy);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_SetShieldPolicy;
		out << (quint8)0;						// 0 = 'shield fuel' policy
		out << (quint8)policy;
		QueueChatPacket(ba);
	}
}

// Configures shield up policy for the ship the person is currently aboard.
void AuApp::ShieldUpPolicy(int policy)
{
	if(IsSingleUserMode())
	{
		m_I->GovSetShieldPolicy(1, policy);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_SetShieldPolicy;
		out << (quint8)1;						// 1 = 'shield up' policy
		out << (quint8)policy;
		QueueChatPacket(ba);
	}
}

void AuApp::SendShipFunds(double amount, AuId idShip)
{
	if(IsSingleUserMode())
	{
		m_I->DepositShipFunds(amount, idShip);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_SendShipFunds;
		out << amount;
		out << idShip;
		QueueChatPacket(ba);
	}
}

void AuApp::ReceiveShipFunds(double amount, AuId idShip)
{
	if(IsSingleUserMode())
	{
		m_I->WithdrawShipFunds(amount, idShip);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_ReceiveShipFunds;
		out << amount;
		out << idShip;
		QueueChatPacket(ba);
	}
}

void AuApp::SetPoliticalStance(quint32 towardEmpire, quint8 stance, quint8 vassalTributePercent)
{
	if(stance > POL_Vassal)
		return;
	if(IsSingleUserMode())
	{
		m_I->GovSetPoliticalStance(towardEmpire, stance, vassalTributePercent);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_SetPoliticalStance;
		out << towardEmpire;
		out << stance;
		out << vassalTributePercent;
		QueueChatPacket(ba);
	}
}

void AuApp::CreateArena(AuArenaReady arena)
{
	if(IsSingleUserMode())
	{
		m_I->ArenaCreateNew(arena);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_CreateArena;
		out << arena;
		QueueChatPacket(ba);
	}
}

void AuApp::JoinArena(AuId arenaId, QString password, quint32 shipDesign, quint8 team)
{
	if(IsSingleUserMode())
	{
		m_I->ArenaJoin(arenaId, password, shipDesign, team);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_JoinArena;
		out << arenaId;
		out << password;
		out << shipDesign;
		out << team;
		QueueChatPacket(ba);
	}
}

void AuApp::EnterStudio(int panelIndex, QString password)
{
	if(IsSingleUserMode())
	{
		m_I->DesignStudioEnter(panelIndex, password);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_EnterStudio;
		out << (quint16)panelIndex;
		out << password;
		QueueChatPacket(ba);
	}
}

void AuApp::HandCraftGo(int mpid)
{
	if(IsSingleUserMode())
	{
		m_I->HandcraftGo(mpid);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_HandCraftGo;
		out << (qint32)mpid;
		QueueChatPacket(ba);
	}
}

void AuApp::SetUnitAmmo(const AuId& unitId, COMMODITYID ammoCID)
{
	if(IsSingleUserMode())
	{
		m_I->SetUnitAmmo(unitId, ammoCID);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PU_SetAmmo;
		out << unitId;
		out << (quint16)ammoCID;
		QueueChatPacket(ba);
	}
}

void AuApp::SetUnitAmmoCount(const AuId& unitId, quint8 count)
{
	if(IsSingleUserMode())
	{
		m_I->SetUnitAmmoCount(unitId, count);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PU_SetAmmoCount;
		out << unitId;
		out << count;
		QueueChatPacket(ba);
	}
}

void AuApp::SetUnitArmor(const AuId& unitId, const AuItem& item)
{
	if(IsSingleUserMode())
	{
		m_I->SetUnitArmor(unitId, item);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PU_SetArmor;
		out << unitId;
		out << item.Type();
		out << item;
		QueueChatPacket(ba);
	}
}

void AuApp::SetUnitClothing(const AuId& unitId, const AuItem& item)
{
	if(IsSingleUserMode())
	{
		m_I->SetUnitClothing(unitId, item);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PU_SetClothing;
		out << unitId;
		out << item.Type();
		out << item;
		QueueChatPacket(ba);
	}
}

void AuApp::SetUnitHelmet(const AuId& unitId, const AuItem& item)
{
	if(IsSingleUserMode())
	{
		m_I->SetUnitHelmet(unitId, item);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PU_SetHelmet;
		out << unitId;
		out << item.Type();
		out << item;
		QueueChatPacket(ba);
	}
}

void AuApp::SetUnitWeapon(const AuId& unitId, const AuItem& item)
{
	if(IsSingleUserMode())
	{
		m_I->SetUnitWeapon(unitId, item);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PU_SetWeapon;
		out << unitId;
		out << item.Type();
		out << item;
		QueueChatPacket(ba);
	}
}

bool AuApp::SendFunds(const QString& toName, const AuId& toId, double amount)
{
	if(toId.IsNull())
		return false;
	if(amount <= 0)
		return false;
	if(IsSingleUserMode())
	{
		m_I->SendFunds(toId, toName, amount);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_SendFunds;
		out << toId;
		out << toName;
		out << amount;
		QueueChatPacket(ba);
	}
	return true;
}

void AuApp::NameSpecies(const AuSpecies& species, const QString& speciesName, bool isWildAnimal)
{
	if(species.IsNull())
		return;
	if(speciesName.isEmpty())
		return;
	if(IsSingleUserMode())
	{
		m_I->SetSpeciesName(species, speciesName, isWildAnimal);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_Specie;
		out << species;
		out << speciesName;
		out << isWildAnimal;
		QueueChatPacket(ba);
	}
}

void AuApp::SpeciesRequested(const AuSpecies& species)
{
	if(IsSingleUserMode())
	{
		DzSpecies spec;
		const DgData& gd = g_Au.LockDataForRead();
        DzSpeciesHash::const_iterator si = gd.m_Species.find(species);
        if(si != gd.m_Species.cend())
        {
            // The species has been named. Send the species record.
            spec = (*si).second;
        }
        else
        {
            // The species has not been named. Send an empty record.
            spec = DzSpecies(species, "", AuId());
        }
        g_Au.UnlockData();
		ReplaceSpecies(spec);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_SpeciesRequest;
		out << species;
		QueueChatPacket(ba);
	}
}

void AuApp::GameSelect(quint8 gameType)
{
	if(IsSingleUserMode())
	{
		m_I->GameSelected(gameType);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PT_GameSelect;
		out << gameType;
		QueueChatPacket(ba);
	}
}

void AuApp::GameStart(quint8 gameType, QByteArray gameSettings)
{
	if(IsSingleUserMode())
	{
		m_I->GameStart(gameType, gameSettings);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PT_GameStart;
		out << gameType;
		out << gameSettings;
		QueueChatPacket(ba);
	}
}

void AuApp::GameTurn(quint8 gameType, QByteArray gameTurn)
{
	if(IsSingleUserMode())
	{
		m_I->GameTurn(gameType, gameTurn);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PT_GameTurn;
		out << gameType;
		out << gameTurn;
		QueueChatPacket(ba);
	}
}

void AuApp::GameQuit()
{
	if(IsSingleUserMode())
	{
		m_I->GameQuit();
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PT_GameQuit;
		QueueChatPacket(ba);
	}
}

void AuApp::KickStowaways()
{
	if(IsSingleUserMode())
	{
		m_I->KickStowaways();
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_KickBerthOccupant;
		out << (quint8)1;						// Kick stowaways.
		QueueChatPacket(ba);
	}
}

void AuApp::ArenaExit()
{
	if(IsSingleUserMode())
	{
		m_I->ArenaExit();
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ArenaExit;
		QueueChatPacket(ba);
	}
}

void AuApp::ArenaRecall()
{
	if(IsSingleUserMode())
	{
		m_I->ArenaRecall();
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ArenaRecall;
		QueueChatPacket(ba);
	}
}

void AuApp::ArenaStatus()
{
	if(IsSingleUserMode())
	{
		m_I->ArenaStatus();
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ArenaStatus;
		QueueChatPacket(ba);
	}
}

void AuApp::SetPictureFrame(AuGearPos pos, QString url, bool hideURL, bool rotate)
{
	if(IsSingleUserMode())
	{
		m_I->SetPictureFrame(pos, url, hideURL, rotate);
	}
	else
	{
		// Transmit the data to the server.
		// The reply arrives as changes to my dude and/or the terrain atoms.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_SetPictureFrame;
		out << pos;
		out << url;
		out << rotate;
		out << hideURL;
		QueueChatPacket(ba);
	}
}

void AuApp::SetFlowerPot(AuGearPos pos, quint8 plantSize)
{
	if(IsSingleUserMode())
	{
		m_I->SetFlowerPot(pos, plantSize);
	}
	else
	{
		// Transmit the data to the server.
		// The reply arrives as changes to my dude and/or the terrain atoms.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_SetFlowerPot;
		out << pos;
		out << plantSize;
		QueueChatPacket(ba);
	}
}

void AuApp::GovSetBoss(const AuId& governorId, const AuId& newBossId)
{
	if(governorId.IsNull())
		return;
	if(newBossId.IsNull())
		return;

	if(IsSingleUserMode())
	{
		m_I->GovSetBoss(governorId, newBossId);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_SetBoss;
		out << governorId;
		out << newBossId;
		QueueChatPacket(ba);
	}
}

void AuApp::SetMessage(AuGearPos pos, QString message)
{
	if(IsSingleUserMode())
	{
		m_I->SetMessage(pos, message);
	}
	else
	{
		// Transmit the data to the server.
		// The reply arrives as changes to my dude and/or the terrain atoms.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_Message;
		out << pos;
		out << message;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerNew(const AuDesign& design)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerNew(design);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_New;
		out << design.Pack();
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerSetProperties(const AuDesign& design)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerSetProperties(design);
	}
	else
	{
		// Send a clone without the parts.
		AuDesign* clone = design.Clone();
		if(!clone)
			return;
		clone->ClearParts();

		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_Properties;
		out << clone->Pack();
		QueueChatPacket(ba);

		// Delete the clone.
		delete clone;
	}
}

void AuApp::DesignerPartAdd(const AuScPart& part)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerPartAdd(part);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)0;										// 0=add
		out << part.Pack();
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartUpdate(quint16 partId, const AuScPart& part)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerPartUpdate(partId, part);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)2;										// 2=update
		out << partId;
		out << part.Pack();
		QueueChatPacket(ba);
	}
}

bool AuApp::DesignerLoadedFromFile(AuDesign& design)
{
	bool sizeOk = true;
	if(IsSingleUserMode())
	{
		m_I->DesignerLoadedFromFile(design);
	}
	else
	{
		// Transmit the file data to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScLoaded;
		QByteArray bac(qCompress(design.Pack()));
		if(bac.size() > (int)design.CompressedSizeLimit())
		{
			bac = qCompress(QByteArray());	// Send nothing when the compressed data is too large.
			sizeOk = false;
		}
		out << bac;
		QueueChatPacket(ba);
	}
	return sizeOk;
}

void AuApp::DesignerCopyParts(const AuScSelection& partIds, const AuMatrix4x4& t)
{
	if(t.IsIdentity())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerCopyParts(partIds, t);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)3;										// Copy parts.
		out << partIds;
		out << t;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerCopyPartObjects(const AuScSelection& partIds, const AuMatrix4x4& t)
{
	if(t.IsIdentity())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerCopyPartObjects(partIds, t);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)7;										// Copy part objects.
		out << partIds;
		out << t;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerMoveLiftStops(const AuScSelection& partIds, const AuMatrix4x4& t)
{
	if(t.IsIdentity())
		return;
	if(partIds.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerMoveLiftStops(partIds, t);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)90;										// Move lift stops.
		out << partIds;
		out << t;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerMoveParts(const AuScSelection& partIds, const AuMatrix4x4& t)
{
	if(t.IsIdentity())
		return;
	if(partIds.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerMoveParts(partIds, t);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)4;										// Move parts.
		out << partIds;
		out << t;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerMovePartObjects(const AuScSelection& partIds, const AuMatrix4x4& t)
{
	if(t.IsIdentity())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerMovePartObjects(partIds, t);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)8;										// Move parts.
		out << partIds;
		out << t;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerDeleteParts(const AuScSelection& partIds)
{
	if(partIds.isEmpty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerDeleteParts(partIds);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)5;										// Delete parts
		out << partIds;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerDeletePartObjects(const AuScSelection& partIds)
{
	if(partIds.isEmpty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerDeletePartObjects(partIds);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)9;										// Delete part objects.
		out << partIds;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerAddParts(const AuScParts& parts)
{
	if(parts.isEmpty() || parts.count() > 0xffff)
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerAddParts(parts);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)6;										// Add parts
		out << parts;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerAddPartObjects(quint16 partId, const AuScParts& parts)
{
	if(parts.isEmpty() || parts.count() > 0xffff)
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerAddPartObjects(partId, parts);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)10;										// Add part objects.
		out << partId;
		out << parts;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerRedo()
{
	if(IsSingleUserMode())
	{
		m_I->DesignerRedo();
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_Redo;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerUndo()
{
	if(IsSingleUserMode())
	{
		m_I->DesignerUndo();
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_Undo;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerJoinParts(quint16 partId, const AuScSelection& sel)
{
	if(!partId)
		return;
	if(sel.size() < 2)
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerJoinParts(partId, sel);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)11;										// Join parts.
		out << partId;
		out << sel;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerReverseFaces(const AuScSelection& sel)
{
	if(!sel.HasObjectSelection())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerReverseFaces(sel);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)12;										// Reverse faces.
		out << sel;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerSplitFaces(const AuScSelection& sel)
{
	if(!sel.HasObjectSelection())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerSplitFaces(sel);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)13;										// Split faces.
		out << sel;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerFlatten(const AuScSelection& sel)
{
	if(!sel.HasObjectSelection())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerFlattenVertices(sel);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)14;										// Flatten selected vertices of selected faces.
		out << sel;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerSmooth(const AuScSelection& sel)
{
	if(!sel.HasObjectSelection())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerSmooth(sel);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)15;										// Smooth selected vertices of selected faces.
		out << sel;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerJigCut(const AuScSelection& sel)
{
	if(sel.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerJigCut(sel);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)16;										// Jig cut parts.
		out << sel;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartFillFace(const AuScSelection& sel, const AuPointArray& boundary, const AuMatrix4x4& gridPlane)
{
	if(sel.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartFillFace(sel, boundary, gridPlane);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)17;										// Flatten selected vertices of selected faces.
		out << sel;
		out << boundary;
		out << gridPlane;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartSetStopName(quint16 partId, const QString& name)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerPartSetStopName(partId, name);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)18;										// Flatten selected vertices of selected faces.
		out << partId;
		out << name;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartSetState(quint16 partId, quint8 partState)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerPartSetState(partId, partState);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)19;
		out << partId;
		out << partState;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsSetHidden(const AuScIdSet& parts, bool hide)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerPartsSetHidden(parts, hide);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)20;
		out << parts;
		out << hide;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsSetLocked(const AuScIdSet& parts, bool lock)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerPartsSetLocked(parts, lock);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)21;										// Flatten selected vertices of selected faces.
		out << parts;
		out << lock;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsSetOmitted(const AuScIdSet& parts, bool omit)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerPartsSetOmitted(parts, omit);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)22;
		out << parts;
		out << omit;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsSetSeal(const AuScIdSet& parts, bool seal)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerPartsSetSeal(parts, seal);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)1;
		out << parts;
		out << seal;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartAddState(quint16 partId, quint8 state)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerPartAddState(partId, state);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)23;
		out << partId;
		out << state;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartAddFaces(quint16 partId, const AuScFaceList& faces)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerPartAddFaces(partId, faces);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)24;
		out << partId;
		out << faces;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartDeleteState(quint16 partId, quint8 state)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerPartDeleteState(partId, state);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)25;
		out << partId;
		out << state;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartReplaceFaces(quint16 partId, const AuScSelection& sel, const AuScFaceList& faces)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerPartReplaceFaces(partId, sel, faces);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)26;
		out << partId;
		out << sel;
		out << faces;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerCutFaces(const AuScSelection& sel, const AuPointArray& cutLine)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerCutFaces(sel, cutLine);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)27;
		out << sel;
		out << cutLine;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerFractureFaces(const AuScSelection& sel)
{
	if(!sel.HasObjectSelection())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerFractureFaces(sel);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)28;
		out << sel;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerFuseFaces(const AuScSelection& sel)
{
	if(!sel.HasObjectSelection())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerFuseFaces(sel);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)29;
		out << sel;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerDivideFaces(const AuScSelection& sel, quint8 divisions)
{
	if(!sel.HasObjectSelection())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerDivideFaces(sel, divisions);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)30;
		out << sel;
		out << divisions;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerMakeNavLightLens(const AuScSelection& partIds, const QColor& color)
{
	if(partIds.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerMakeNavLightLens(partIds, color);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)31;
		out << partIds;
		out << color;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerMakeDoor(const AuScSelection& partIds)
{
	if(partIds.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerMakeDoor(partIds);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)32;
		out << partIds;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerMakeLightLens(const AuScSelection& partIds, const QColor& lightColor)
{
	if(partIds.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerMakeLightLens(partIds, lightColor);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)40;
		out << partIds;
		out << lightColor;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerMakeLandingGear(const AuScSelection& partIds)
{
	if(partIds.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerMakeLandingGear(partIds);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)34;
		out << partIds;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerMakeRotatingBeaconLens(const AuScSelection& partIds, const QColor& lightColor, const AuPoint& rotationAxis)
{
	if(partIds.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerMakeRotatingBeaconLens(partIds, lightColor, rotationAxis);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)35;
		out << partIds;
		out << lightColor;
		out << rotationAxis;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerMakeStrobeLens(const AuScSelection& partIds, const QColor& lightColor)
{
	if(partIds.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerMakeStrobeLens(partIds, lightColor);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)36;
		out << partIds;
		out << lightColor;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerMakeTurboLift(const AuScSelection& partIds)
{
	if(partIds.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerMakeTurboLift(partIds);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)37;
		out << partIds;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerMakeTurret(const AuScSelection& partIds, const AuPoint& basePivot, const AuPoint& baseAxis)
{
	if(partIds.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerMakeTurret(partIds, basePivot, baseAxis);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)38;
		out << partIds;
		out << basePivot;
		out << baseAxis;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerMakeWindow(const AuScSelection& partIds, float opacity)
{
	if(partIds.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerMakeWindow(partIds, opacity);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)39;
		out << partIds;
		out << opacity;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerTurretStation(const AuScSelection& sel, quint8 gunnerPos, const AuPoint& gunnerLoc,
		const AuPoint& gunnerFacing, const AuPoint& corner1, const AuPoint& corner2, const AuMatrix4x4& gridPlane,
		COMMODITYID module)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerTurretStation(sel, gunnerPos, gunnerLoc, gunnerFacing, corner1, corner2, gridPlane, module);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)42;
		out << sel;
		out << (quint8)gunnerPos;
		out << gunnerLoc;
		out << corner1;
		out << corner2;
		out << gridPlane;
		out << (quint16)module;
		out << gunnerFacing;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerTurretBase(const AuScSelection& sel, const AuPoint& basePivot, const AuPoint& baseAxis)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerTurretBase(sel, basePivot, baseAxis);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)56;
		out << sel;
		out << basePivot;
		out << baseAxis;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerTurretGun(const AuScSelection& sel, const AuPoint& gunPivot, const AuPoint& gunAxis,
		const AuPoint& gunEmitLoc, const AuPoint& gunEmitVec, const AuPoint& gunSightLoc)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerTurretGun(sel, gunPivot, gunAxis, gunEmitLoc, gunEmitVec, gunSightLoc);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)43;
		out << sel;
		out << gunPivot;
		out << gunAxis;
		out << gunEmitLoc;
		out << gunEmitVec;
		out << gunSightLoc;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartSetOpacity(quint16 partId, float opacity)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerPartSetOpacity(partId, opacity);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)44;
		out << partId;
		out << opacity;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerMakeHull(const AuScSelection& partIds)
{
	if(partIds.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerMakeHull(partIds);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)45;
		out << partIds;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerMakeMesh(const AuScSelection& partIds)
{
	if(partIds.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerMakeMesh(partIds);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)89;
		out << partIds;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerMakeRoom(const AuScSelection& partIds)
{
	if(partIds.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerMakeRoom(partIds);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)46;
		out << partIds;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartAddStop(quint16 partId, const QString& stop)
{
	if(stop.isEmpty() || !IsNameValid(stop))
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartAddStop(partId, stop);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)47;
		out << partId;
		out << stop;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartDeleteStop(quint16 partId, const QString& stop)
{
	if(stop.isEmpty() || !IsNameValid(stop))
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartDeleteStop(partId, stop);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)48;
		out << partId;
		out << stop;
		QueueChatPacket(ba);
	}
}

// Blank 'stop' is valid, to get back to the neutral state.
void AuApp::DesignerPartSetStop(quint16 partId, const QString& stop)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerPartSetStop(partId, stop);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)49;
		out << partId;
		out << stop;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPaintFaces(const AuScSelection& sel, const QColor& paint)
{
	if(!sel.HasObjectSelection())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPaintFaces(sel, paint);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)50;
		out << sel;
		out << paint;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPaintFaces(const AuScSelection& sel, quint8 theme, quint8 opacity)
{
	if(!sel.HasObjectSelection())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPaintFaces(sel, theme, opacity);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)51;
		out << sel;
		out << (quint8)theme;
		out << opacity;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerGlowVertices(const AuScSelection& sel, const QColor& glow)
{
	if(!sel.HasObjectSelection())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerGlowVertices(sel, glow);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)52;
		out << sel;
		out << glow;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPolishFaces(const AuScSelection& sel, float shine, float reflectivity)
{
	if(!sel.HasObjectSelection())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPolishFaces(sel, shine, reflectivity);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)53;
		out << sel;
		out << shine;
		out << reflectivity;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsSetFaceOpacity(const AuScSelection& sel, float opacity)
{
	if(!sel.HasObjectSelection())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsSetFaceOpacity(sel, opacity);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)54;
		out << sel;
		out << opacity;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsSetFaceSelfIllumination(const AuScSelection& sel, float selfIllum)
{
	if(!sel.HasObjectSelection())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsSetFaceSelfIllumination(sel, selfIllum);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)55;
		out << sel;
		out << selfIllum;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerSetTexturePalette(const AuDesignTex texturePalette[SCTEXI_Count])
{
	// Validate the palette.
	for(int tpi=0; tpi<SCTEXI_Count; tpi++)
		if(!texturePalette[tpi].IsValid())
			return;

	if(IsSingleUserMode())
	{
		m_I->DesignerSetTexturePalette(texturePalette);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)57;
		for(int tpi=0; tpi<SCTEXI_Count; tpi++)
			out << texturePalette[tpi];
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsTextureFaces(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex)
{
	if(!sel.HasObjectSelection())
		return;
	if(texIndex < 0 || texIndex >= SCTEXI_Count)
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsTextureFaces(sel, texIndex);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)58;
		out << sel;
		out << (quint8)texIndex;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsTextureFacesCylindrical(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
		double texSize, const AuMatrix4x4& gridPlane)
{
	if(!sel.HasObjectSelection())
		return;
	if(texIndex < 0 || texIndex >= SCTEXI_Count)
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsTextureFacesCylindrical(sel, texIndex, texRepsAtEquator, texSize, gridPlane);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)59;
		out << sel;
		out << (quint8)texIndex;
		out << texSize;
		out << gridPlane;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsTextureFacesProject(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, double texSize,
		const AuMatrix4x4& gridPlane)
{
	if(!sel.HasObjectSelection())
		return;
	if(texIndex < 0 || texIndex >= SCTEXI_Count)
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsTextureFacesProject(sel, texIndex, texSize, gridPlane);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)60;
		out << sel;
		out << (quint8)texIndex;
		out << texSize;
		out << gridPlane;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsTextureFacesSpherical(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, quint16 texRepsAtEquator,
		const AuMatrix4x4& gridPlane)
{
	if(!sel.HasObjectSelection())
		return;
	if(texIndex < 0 || texIndex >= SCTEXI_Count)
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsTextureFacesSpherical(sel, texIndex, texRepsAtEquator, gridPlane);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)61;
		out << sel;
		out << (quint8)texIndex;
		out << texRepsAtEquator;
		out << gridPlane;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsEffectFaces(const AuScSelection& sel, SURFACE_EFFECT effect)
{
	if(!sel.HasObjectSelection())
		return;
	if(effect < 0 || effect >= EFF_Count)
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsEffectFaces(sel, effect);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)79;
		out << sel;
		out << (quint8)effect;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsEffectFacesCylindrical(const AuScSelection& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
		double texSize, const AuMatrix4x4& gridPlane)
{
	if(!sel.HasObjectSelection())
		return;
	if(effect < 0 || effect >= EFF_Count)
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsEffectFacesCylindrical(sel, effect, texRepsAtEquator, texSize, gridPlane);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)80;
		out << sel;
		out << (quint8)effect;
		out << texSize;
		out << gridPlane;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsEffectFacesProject(const AuScSelection& sel, SURFACE_EFFECT effect, double texSize,
		const AuMatrix4x4& gridPlane)
{
	if(!sel.HasObjectSelection())
		return;
	if(effect < 0 || effect >= EFF_Count)
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsEffectFacesProject(sel, effect, texSize, gridPlane);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)81;
		out << sel;
		out << (quint8)effect;
		out << texSize;
		out << gridPlane;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsEffectFacesSpherical(const AuScSelection& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
		const AuMatrix4x4& gridPlane)
{
	if(!sel.HasObjectSelection())
		return;
	if(effect < 0 || effect >= EFF_Count)
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsEffectFacesSpherical(sel, effect, texRepsAtEquator, gridPlane);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)82;
		out << sel;
		out << (quint8)effect;
		out << texRepsAtEquator;
		out << gridPlane;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsEffectMove(AuScSelection sel, QPointF vec, quint8 undoSeq)
{
	if(vec == QPointF(0,0))
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsEffectMove(sel, vec, undoSeq);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)83;
		out << sel;
		out << vec;
		out << undoSeq;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsEffectRotate(AuScSelection sel, QPointF pivot, double angle, quint8 undoSeq)
{
	if(angle == 0 || angle == PI*2)
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsEffectRotate(sel, pivot, angle, undoSeq);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)84;
		out << sel;
		out << pivot;
		out << angle;
		out << undoSeq;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsEffectStretch(AuScSelection sel, QPointF base, double scaleX, double scaleY, quint8 undoSeq)
{
	if(scaleX == 1 && scaleY == 1)
		return;
	if(scaleX == 0 || scaleY == 0)
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsEffectStretch(sel, base, scaleX, scaleY, undoSeq);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)85;
		out << sel;
		out << base;
		out << scaleX;
		out << scaleY;
		out << undoSeq;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsEffectUnwrap(AuScSelection sel, SURFACE_EFFECT effect, double texSize,
		AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	if(!sel.HasObjectSelection())
		return;
	if(effect < 0 || effect >= EFF_Count)
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsEffectUnwrap(sel, effect, texSize, unwrap, angleTol, gridAlign, alignAxis, gridPlane);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)86;
		out << sel;
		out << (quint8)effect;
		out << texSize;
		out << (quint8)unwrap;
		out << angleTol;
		out << gridPlane;
		out << gridAlign;
		out << (quint8)alignAxis;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerMakeDecal(const AuScSelection& partIds)
{
	if(partIds.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerMakeDecal(partIds);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)62;
		out << partIds;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerCopyPartsSymmetrical(const AuScSelection& partIds, const AuMatrix4x4& gridPlane)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerCopyPartsSymmetrical(partIds, gridPlane);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)63;
		out << partIds;
		out << gridPlane;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsTextureMove(AuScSelection sel, QPointF vec, quint8 undoSeq)
{
	if(vec == QPointF(0,0))
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsTextureMove(sel, vec, undoSeq);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)64;
		out << sel;
		out << vec;
		out << undoSeq;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsTextureRotate(AuScSelection sel, QPointF pivot, double angle, quint8 undoSeq)
{
	if(angle == 0 || angle == PI*2)
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsTextureRotate(sel, pivot, angle, undoSeq);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)65;
		out << sel;
		out << pivot;
		out << angle;
		out << undoSeq;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsTextureStretch(AuScSelection sel, QPointF base, double scaleX, double scaleY, quint8 undoSeq)
{
	if(scaleX == 1 && scaleY == 1)
		return;
	if(scaleX == 0 || scaleY == 0)
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsTextureStretch(sel, base, scaleX, scaleY, undoSeq);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)66;
		out << sel;
		out << base;
		out << scaleX;
		out << scaleY;
		out << undoSeq;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsTextureUnwrap(AuScSelection sel, SC_TEXTURE_INDEX texIndex, double texSize,
		AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	if(!sel.HasObjectSelection())
		return;
	if(texIndex < 0 || texIndex >= SCTEXI_Count)
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsTextureUnwrap(sel, texIndex, texSize, unwrap, angleTol, gridAlign, alignAxis, gridPlane);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)67;
		out << sel;
		out << (quint8)texIndex;
		out << texSize;
		out << (quint8)unwrap;
		out << angleTol;
		out << gridPlane;
		out << gridAlign;
		out << (quint8)alignAxis;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerRefreshObstructions()
{
	// Path data is only present on the client side in the designer.
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaPerson* person = dynamic_cast<DaPerson*>(ether.FindNormalAtom(m_I->AvatarId()));
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
		{
			pde->RefreshObstructions();
			pde->SetModified();
		}
	}
	g_Au.UnlockEtheriumDelete();
	QApplication::restoreOverrideCursor();
}

void AuApp::DesignerAssociateParts(const AuScSelection& partIds)
{
	if(partIds.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerAssociateParts(partIds);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)68;
		out << partIds;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerSetGravity(bool gravOn)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerSetGravity(gravOn);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_Gravity;
		out << gravOn;
		QueueChatPacket(ba);
	}
}

bool AuApp::DesignerSetPreview(quint8 previewType, quint32 genSeed)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	// Path data is only present on the client side in the designer. The functions used cannot be run on the server so I have to
	// build the obstructions data and send it to the server to see a preview.
	AuDesign* design = 0;	// Clone of current design with obstruction model. Can be 0 when turning off the preview.
	if(previewType)
	{
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaPerson* person = dynamic_cast<DaPerson*>(ether.FindNormalAtom(m_I->AvatarId()));
		if(person)
		{
			DaDesigner* pde = person->FindParentDesignerInEnv();
			if(pde)
			{
				const AuDesign* des = pde->LockDesignForRead();
				if(des)
				{
					design = des->Clone();
					design->FinalizePreview();
				}
				pde->UnlockDesign();
			}
		}
		g_Au.UnlockEtheriumDelete();
	}

	bool sizeOk = true;
	if(IsSingleUserMode())
	{
		m_I->DesignerSetPreview(previewType, design, genSeed);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScLibrary;
		out << (quint8)6;
		out << previewType;
		if(previewType)
		{
			// I must send the design so the server does not have to call RefreshObstructions.
			QByteArray bac(qCompress(design->Pack()));
			if(bac.size() > (int)design->CompressedSizeLimit())
			{
				bac = qCompress(QByteArray());	// Send nothing when the compressed data is too large.
				sizeOk = false;
			}
			out << bac;
			out << genSeed;
		}
		QueueChatPacket(ba);
	}

	// Clean up the clone.
	if(design)
		delete design;
	QApplication::restoreOverrideCursor();
	return sizeOk;
}

void AuApp::DesignerPartsSetDoorProperties(const AuScSelection& partIds, Qt::CheckState isAutomatic, Qt::CheckState initLocked,
		Qt::CheckState remoteSpacecraft, Qt::CheckState remoteVehicle)
{
	if(partIds.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsSetDoorProperties(partIds, isAutomatic, initLocked, remoteSpacecraft, remoteVehicle);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)88;
		out << partIds;
		out << (quint8)isAutomatic;
		out << (quint8)initLocked;
		out << (quint8)remoteSpacecraft;
		out << (quint8)remoteVehicle;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsSetSiteProperties(const AuScSelection& partIds, quint8 grade, quint8 grass, quint8 pavement,
		quint8 rocks, quint8 shrubs, quint8 terBlend, quint8 trees)
{
	if(partIds.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsSetSiteProperties(partIds, grade, grass, pavement, rocks, shrubs, terBlend, trees);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)93;
		out << partIds;
		out << grade;
		out << grass;
		out << pavement;
		out << rocks;
		out << shrubs;
		out << terBlend;
		out << trees;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsSetVoidProperties(const AuScSelection& partIds, GRAVITY gravityType, const AuPoint& gravityUp,
		HOLD_ACCESS holdAccess, quint8 personnelAccess)
{
	if(partIds.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsSetVoidProperties(partIds, gravityType, gravityUp, holdAccess, personnelAccess);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)69;
		out << partIds;
		out << (quint8)gravityType;
		out << gravityUp;
		out << (quint8)holdAccess;
		out << personnelAccess;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsSetRoomProperties(const AuScSelection& partIds, bool isSunlit, INTERIOR_LIGHT initLight)
{
	if(partIds.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsSetRoomProperties(partIds, isSunlit, initLight);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)87;
		out << partIds;
		out << (quint8)isSunlit;
		out << (quint8)initLight;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartFillQuad(const AuScSelection& sel, const AuPointArray& boundary)
{
	if(sel.empty())
		return;
	if(boundary.size() < 4)
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartFillQuad(sel, boundary);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)70;
		out << sel;
		out << boundary[0];
		out << boundary[1];
		out << boundary[2];
		out << boundary[3];
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartFillTriangle(const AuScSelection& sel, const AuPointArray& boundary)
{
	if(sel.empty())
		return;
	if(boundary.size() < 3)
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartFillTriangle(sel, boundary);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)71;
		out << sel;
		out << boundary[0];
		out << boundary[1];
		out << boundary[2];
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPunchFaces(const AuScSelection& sel, const AuPointArray& cutBnd)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerPunchFaces(sel, cutBnd);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)72;
		out << sel;
		out << cutBnd;
		QueueChatPacket(ba);
	}
}

bool AuApp::DesignerLibrarySave(AuDesign* design)
{
	if(design->Name().isEmpty())
		return true;
	if(design->Thumbnail().width() > 256 || design->Thumbnail().height() > 256)
		return true;

	bool sizeOk = true;
	if(IsSingleUserMode())
	{
		AuScDesign* sc = dynamic_cast<AuScDesign*>(design);
		AuBdDesign* bd = dynamic_cast<AuBdDesign*>(design);
		if(sc)
			m_I->DesignerArchitectSaveSc(*sc);
		else if(bd)
			m_I->DesignerArchitectSaveBd(*bd);
	}
	else
	{
		if(dynamic_cast<AuScDesign*>(design))
		{
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtChatChannelPacket::PD_ScLibrary;
			out << (quint8)0;											// Storing a design.
			QByteArray bac(qCompress(design->Pack()));
			if(bac.size() > (int)design->CompressedSizeLimit())
			{
				bac = qCompress(QByteArray());	// Send nothing when the compressed data is too large.
				sizeOk = false;
			}
			out << bac;
			QueueChatPacket(ba);
		}
		else if(dynamic_cast<AuBdDesign*>(design))
		{
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtChatChannelPacket::PD_BdLibrary;
			out << (quint8)0;											// Storing a design.
			QByteArray bac(qCompress(design->Pack()));
			if(bac.size() > (int)design->CompressedSizeLimit())
			{
				bac = qCompress(QByteArray());	// Send nothing when the compressed data is too large.
				sizeOk = false;
			}
			out << bac;
			QueueChatPacket(ba);
		}
	}
	return sizeOk;
}

void AuApp::DesignerLibraryDeleteBd(quint32 designId)
{
	if(!designId)
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerArchitectDeleteBd(true, designId);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_BdLibrary;
		out << (quint8)2;											// Deleting a design.
		out << designId;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerLibraryDeleteIt(quint32 designId)
{
	if(!designId)
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerArchitectDeleteIt(true, designId);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ItLibrary;
		out << (quint8)2;											// Deleting a design.
		out << designId;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerLibraryDeleteSc(quint32 designId)
{
	if(!designId)
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerArchitectDeleteSc(true, designId);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScLibrary;
		out << (quint8)2;											// Deleting a design.
		out << designId;
		QueueChatPacket(ba);
	}
}

bool AuApp::DesignerLibraryFinalizeAssembly(AuDesign* design)
{
	if(design->Name().isEmpty())
		return true;
	if(design->Thumbnail().width() > 256 || design->Thumbnail().height() > 256)
		return true;

	bool sizeOk = true;
	if(IsSingleUserMode())
	{
		if(!IsSingleUserDesignerMode())
		{
			AuScDesign* sc = dynamic_cast<AuScDesign*>(design);
			AuBdDesign* bd = dynamic_cast<AuBdDesign*>(design);
			if(sc)
				m_I->DesignerArchitectFinalizeScAssembly(*sc);
			else if(bd)
			{
				QSet<quint32> xrefs;
				m_I->DesignerArchitectFinalizeBdAssembly(*bd, xrefs);
			}
		}
	}
	else
	{
		if(dynamic_cast<AuScDesign*>(design))
		{
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtChatChannelPacket::PD_ScLibrary;
			out << (quint8)3;											// Storing a finalized assembly.
			QByteArray bac(qCompress(design->Pack()));
			if(bac.size() > (int)design->CompressedSizeLimit())
			{
				bac = qCompress(QByteArray());	// Send nothing when the compressed data is too large.
				sizeOk = false;
			}
			out << bac;
			QueueChatPacket(ba);
		}
		else if(dynamic_cast<AuBdDesign*>(design))
		{
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtChatChannelPacket::PD_BdLibrary;
			out << (quint8)3;											// Storing a finalized assembly.
			QByteArray bac(qCompress(design->Pack()));
			if(bac.size() > (int)design->CompressedSizeLimit())
			{
				bac = qCompress(QByteArray());	// Send nothing when the compressed data is too large.
				sizeOk = false;
			}
			out << bac;
			QueueChatPacket(ba);
		}
	}
	return sizeOk;
}

bool AuApp::DesignerLibraryFinalizeDesign(AuDesign* design, quint32 designId)
{
	if(design->Name().isEmpty())
		return true;
	if(design->Thumbnail().width() > 256 || design->Thumbnail().height() > 256)
		return true;

	bool sizeOk = true;
	if(IsSingleUserMode())
	{
		if(!IsSingleUserDesignerMode())
		{
			AuScDesign* sc = dynamic_cast<AuScDesign*>(design);
			AuBdDesign* bd = dynamic_cast<AuBdDesign*>(design);
			if(sc)
				m_I->DesignerArchitectFinalizeScDesign(*sc);
			else if(bd)
			{
				QSet<quint32> xrefs;
				m_I->DesignerArchitectFinalizeBdDesign(*bd, xrefs);
			}
		}
	}
	else
	{
		if(designId)
		{
			// A specific design id is only specified by an agent.
			if(dynamic_cast<const AuScDesign*>(design))
			{
				QByteArray ba;
				QDataStream out(&ba, QIODevice::WriteOnly);
				out.setVersion(QDataStream::Qt_4_1);
				out << (quint8)DtChatChannelPacket::PD_ScLibrary;
				out << (quint8)7;											// Storing a finalized spacecraft design.
				out << designId;
				QByteArray bac(qCompress(design->Pack()));
				if(bac.size() > (int)design->CompressedSizeLimit())
				{
					bac = qCompress(QByteArray());	// Send nothing when the compressed data is too large.
					sizeOk = false;
				}
				out << bac;
				QueueChatPacket(ba);
			}
			else if(dynamic_cast<const AuBdDesign*>(design))
			{
				QByteArray ba;
				QDataStream out(&ba, QIODevice::WriteOnly);
				out.setVersion(QDataStream::Qt_4_1);
				out << (quint8)DtChatChannelPacket::PD_BdLibrary;
				out << (quint8)5;											// Storing a finalized bd design.
				out << designId;
				QByteArray bac(qCompress(design->Pack()));
				if(bac.size() > (int)design->CompressedSizeLimit())
				{
					bac = qCompress(QByteArray());	// Send nothing when the compressed data is too large.
					sizeOk = false;
				}
				out << bac;
				QueueChatPacket(ba);
			}
		}
		else
		{
			if(dynamic_cast<const AuScDesign*>(design))
			{
				QByteArray ba;
				QDataStream out(&ba, QIODevice::WriteOnly);
				out.setVersion(QDataStream::Qt_4_1);
				out << (quint8)DtChatChannelPacket::PD_ScLibrary;
				out << (quint8)4;											// Storing a finalized spacecraft design.
				QByteArray bac(qCompress(design->Pack()));
				if(bac.size() > (int)design->CompressedSizeLimit())
				{
					bac = qCompress(QByteArray());	// Send nothing when the compressed data is too large.
					sizeOk = false;
				}
				out << bac;
				QueueChatPacket(ba);
			}
			else if(dynamic_cast<const AuBdDesign*>(design))
			{
				QByteArray ba;
				QDataStream out(&ba, QIODevice::WriteOnly);
				out.setVersion(QDataStream::Qt_4_1);
				out << (quint8)DtChatChannelPacket::PD_BdLibrary;
				out << (quint8)4;											// Storing a finalized bd design.
				QByteArray bac(qCompress(design->Pack()));
				if(bac.size() > (int)design->CompressedSizeLimit())
				{
					bac = qCompress(QByteArray());	// Send nothing when the compressed data is too large.
					sizeOk = false;
				}
				out << bac;
				QueueChatPacket(ba);
			}
		}
	}
	return sizeOk;
}

void AuApp::DesignerCheckDoors(const AuScSelection& partIds)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerCheckDoors(partIds);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)73;
		out << partIds;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerDissociateParts(const AuScSelection& partIds)
{
	if(partIds.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerDissociateParts(partIds);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)74;
		out << partIds;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerGearOpen(quint32 designId, const AuGearPos& pos)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerGearOpen(designId, pos);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScLibrary;
		out << (quint8)5;
		out << designId;
		out << pos;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerGoToSelection(const AuScSelection& sel)
{
	if(sel.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerGoToSelection(sel);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)75;
		out << sel;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsMoveDoorSwitches(const AuScSelection& partIds, const AuPoint& switch1, const AuPoint& switch2)
{
	if(partIds.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsMoveDoorSwitches(partIds, switch1, switch2);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)76;
		out << partIds;
		out << switch1;
		out << switch2;
		QueueChatPacket(ba);
	}
}

void AuApp::CallTurboLift(const AuId& shipId, quint16 liftId, const QString& stop)
{
	if(IsSingleUserMode())
	{
		m_I->CallTurboLift(shipId, liftId, stop);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_TurboLift;
		out << (quint8)0;
		out << shipId;
		out << liftId;
		out << stop;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsTransporterPadLocs(const AuScSelection& partIds, const AuPointArray& padLocs)
{
	if(partIds.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsTransporterPadLocs(partIds, padLocs);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)77;
		out << partIds;
		out << padLocs;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsWeaponEmitLoc(const AuScSelection& partIds, const AuPoint& emitLoc)
{
	if(partIds.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsWeaponEmitLoc(partIds, emitLoc);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)78;
		out << partIds;
		out << emitLoc;
		QueueChatPacket(ba);
	}
}

void AuApp::KickCryoBerthOccupants(const AuSpecies& species, int count)
{
	if(IsSingleUserMode())
	{
		m_I->KickCryoBerthOccupants(species, count);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_KickBerthOccupant;
		out << (quint8)2;						// Kick cryo travellers.
		out << species;
		out << (quint32)count;
		QueueChatPacket(ba);
	}
}

void AuApp::ReviveCryoBerthOccupant(const AuSpecies& species, quint8 role)
{
	if(IsSingleUserMode())
	{
		m_I->ReviveCryoBerthOccupant(species, role);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_KickBerthOccupant;
		out << (quint8)3;						// Revive cryo traveller.
		out << species;
		out << role;
		QueueChatPacket(ba);
	}
}

void AuApp::BdFoundCity(INDUSTRYID industryId, quint32 designId, COMMODITYID material, const AuPoint& buildLoc, float zRotation,
		const AuId& onPlatform, quint16 mpid, bool workerOk, const QString& cityName, const QString& systemName,
		const QString& sectorName)
{
	if(IsSingleUserMode())
	{
		m_I->BdFoundCity((INDUSTRYID)industryId, designId, (COMMODITYID)material, buildLoc, zRotation, onPlatform, mpid, workerOk,
				cityName, systemName, sectorName);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PB_BdBuild;
		out << (quint8)industryId;
		out << designId;
		out << (quint16)material;
		out << buildLoc;
		out << zRotation;
		out << onPlatform;
		out << mpid;
		out << workerOk;
		out << (quint8)1;					// Founding a city.
		out << cityName;
		out << systemName;
		out << sectorName;
		QueueChatPacket(ba);
	}
}

void AuApp::BdBuild(INDUSTRYID industryId, quint32 designId, COMMODITYID material, const AuPoint& buildLoc, float zRotation,
		const AuId& onPlatform, quint16 mpid, bool workerOk)
{
	if(IsSingleUserMode())
	{
		m_I->BdBuild((INDUSTRYID)industryId, designId, (COMMODITYID)material, buildLoc, zRotation, onPlatform, mpid, workerOk);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PB_BdBuild;
		out << (quint8)industryId;
		out << designId;
		out << (quint16)material;
		out << buildLoc;
		out << zRotation;
		out << onPlatform;
		out << mpid;
		out << workerOk;
		out << (quint8)0;					// Not founding a city.
		QueueChatPacket(ba);
	}
}

void AuApp::BdBuildStorage(INDUSTRYID industryId, quint32 designId, COMMODITYID material, const AuPoint& buildLoc,
		float zRotation, const AuId& onPlatform, quint16 mpid, bool workerOk, COMMODITYID storeCID)
{
	if(IsSingleUserMode())
	{
		m_I->BdBuildStorage((INDUSTRYID)industryId, designId, (COMMODITYID)material, buildLoc, zRotation, onPlatform, mpid,
				workerOk, (COMMODITYID)storeCID);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PB_BdBuild;
		out << (quint8)industryId;
		out << designId;
		out << (quint16)material;
		out << buildLoc;
		out << zRotation;
		out << onPlatform;
		out << mpid;
		out << workerOk;
		out << (quint8)2;					// Building a warehouse or storage tank.
		out << (quint16)storeCID;
		QueueChatPacket(ba);
	}
}

void AuApp::BdPreview(const AuPoint& fBuildLoc, float zRotation)
{
	if(IsSingleUserMode())
	{
		m_I->BdPreview(fBuildLoc, zRotation);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PB_BdBuild;
		out << (quint8)IID_None;
		out << (quint8)0;	// building
		out << fBuildLoc;
		out << zRotation;
		QueueChatPacket(ba);
	}
}

void AuApp::BdPreviewFence(const AuPoint& startLoc, const AuPoint& endLoc)
{
	if(IsSingleUserMode())
	{
		m_I->BdPreviewFence(startLoc, endLoc);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PB_BdBuild;
		out << (quint8)IID_None;
		out << (quint8)1;	// fence
		out << startLoc;
		out << endLoc;
		QueueChatPacket(ba);
	}
}

void AuApp::BdConstructionEnabled(bool enabled, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdConstructionEnabled(enabled, opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_CitizenEnabled;
		out << (quint8)101;
		out << (quint8)enabled;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BdConstructionFetch(const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdConstructionFetch(opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_Fetch;
		out << (quint8)101;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BdConstructionGo(const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdConstructionGo(opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_Go;
		out << (quint8)101;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BdManufactureSpacecraftCitizenEnabled(quint32 enabled, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdManufactureSpacecraftCitizenEnabled(enabled, opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_CitizenEnabled;
		out << (quint8)102;
		out << enabled;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BdManufactureCitizenEnabledMPID(quint16 mpid, quint32 enabled, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdManufactureCitizenEnabledMPID(mpid, enabled, opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_CitizenEnabled;
		out << (quint8)103;
		out << mpid;
		out << enabled;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BdManufactureFetch(quint16 panelIndex, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdManufactureFetch(panelIndex, opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_Fetch;
		out << (quint8)102;
		out << panelIndex;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BdManufactureFetchMPID(quint16 mpid, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdManufactureFetchMPID(mpid, opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_Fetch;
		out << (quint8)103;
		out << mpid;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BdManufactureGo(quint16 panelIndex, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdManufactureGo(panelIndex, opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_Go;
		out << (quint8)102;
		out << panelIndex;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BdManufactureGoMPID(quint16 mpid, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdManufactureGoMPID(mpid, opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_Go;
		out << (quint8)103;
		out << mpid;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BdManufactureProcessesSetMinQ(quint8 minQ, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdManufactureSetMinQ(minQ, opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ProcessSetMinQ;
		out << (quint8)102;
		out << minQ;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BdManufactureProcessSetMinQMPID(quint16 mpid, quint8 minQ, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdManufactureProcessSetMinQMPID(mpid, minQ, opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ProcessSetMinQ;
		out << (quint8)103;
		out << mpid;
		out << minQ;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BdManufactureProcessSetStockLimitMPID(quint16 mpid, quint32 stockLimit, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdManufactureProcessSetStockLimitMPID(mpid, stockLimit, opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ProcessSetMinQ;
		out << (quint8)104;
		out << mpid;
		out << stockLimit;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BdPatentsRecord(const AuKnownPatents& patents, const AuId& opBd)
{
	Q_ASSERT(patents.size() < 0xffff);
	if(IsSingleUserMode())
	{
		m_I->BdManufactureRecordPatents(patents, opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ProcessSelected;
		out << (quint8)0xfb;	// Bd record patent(s).
		out << patents;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BdPatentsRequested(const QSet<COMMODITYID>& patentCIDs, quint8 maxQ, const AuGearPos& mediaPos, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdManufactureGetPatents(patentCIDs, maxQ, mediaPos, opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ProcessSelected;
		out << (quint8)0xfa;	// Bd request patents.
		out << (quint16)patentCIDs.size();
		for(QSet<COMMODITYID>::const_iterator pi = patentCIDs.cbegin(); pi != patentCIDs.cend(); pi++)
			out << (quint16)*pi;
		out << maxQ;
		out << mediaPos;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::GeoRoad(const AuPoint& roadStart, const AuPoint& roadEnd, bool isSlab, float roadWidth, quint8 geoType)
{
	if(IsSingleUserMode())
	{
		m_I->GeoRoad(roadStart, roadEnd, isSlab, roadWidth, geoType);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PB_PlanRoadGlobe;
		out << (quint8)0;	// 0 signals new style geo road
		out << roadStart;
		out << roadEnd;
		out << isSlab;
		out << roadWidth;
		out << geoType;
		QueueChatPacket(ba);
	}
}

void AuApp::GeoLandscape(const AuPoint& roadStart, const AuPoint& roadEnd, float roadWidth, bool hideRocks,
		int hideGrass, bool hideShrubs, bool hideTrees, WORLDTEX_TYPE pavement)
{
	if(IsSingleUserMode())
	{
		m_I->GeoLandscaper(roadStart, roadEnd, roadWidth, hideRocks, hideGrass, hideShrubs, hideTrees, pavement);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PB_PlanRoadGlobe;
		out << (quint8)1;	// 1 signals new style geo landscape
		out << roadStart;
		out << roadEnd;
		out << roadWidth;
		out << hideRocks;
		out << (quint8)hideGrass;
		out << hideShrubs;
		out << hideTrees;
		out << (quint8)pavement;
		QueueChatPacket(ba);
	}
}

void AuApp::BdSpacecraftOrderDelete(quint32 designId, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdManufactureSpacecraftOrderDelete(designId, opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ScFactory;
		out << (quint8)9;											// Delete ship order.
		out << designId;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BdSpacecraftProcessBuyCompanyShip(quint32 designId, AuGearPos pos, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdManufactureSpacecraftBuyCompanyShip(designId, pos, opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ScFactory;
		out << (quint8)10;											// Buy company ship.
		out << designId;
		out << pos;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BdSpacecraftProcessCanceled(const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdManufactureSpacecraftProcessCanceled(opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ScFactory;
		out << (quint8)11;											// Process canceled.
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BdSpacecraftProcessCommissionShip(quint32 designId, AuGearPos pos, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdManufactureSpacecraftCommissionShip(designId, pos, opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ScFactory;
		out << (quint8)12;											// Commission ship.
		out << designId;
		out << pos;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BdSpacecraftProcessDefaultSelected(quint32 designId, const QString& designName, quint8 designQ, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdManufactureSpacecraftProcessDefaultSelected(designId, designName, designQ, opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ScFactory;
		out << (quint8)13;											// Process selected.
		out << designId;
		out << designName;
		out << designQ;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BdSpacecraftProcessSetCompanyPolicy(bool companyOk, quint8 companyMaxQ, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdManufactureSpacecraftSetCompanyPolicy(companyOk, companyMaxQ, opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ScFactory;
		out << (quint8)14;											// Set company build policy.
		out << companyOk;
		out << companyMaxQ;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BdSpacecraftProcessSetMinQ(quint8 minQ, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdManufactureSpacecraftSetMinQ(minQ, opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ScFactory;
		out << (quint8)15;											// Set min quality.
		out << minQ;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::EmpirePatentsRecord(const AuKnownPatents& patents)
{
	Q_ASSERT(patents.size() < 0xffff);
	if(IsSingleUserMode())
	{
		m_I->EmpireRecordPatents(patents);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ProcessSelected;
		out << (quint8)0xf6;	// Empire record patent(s).
		out << patents;
		QueueChatPacket(ba);
	}
}

void AuApp::EmpirePatentsRequested(const QSet<COMMODITYID>& patentCIDs, quint8 maxQ, const AuGearPos& mediaPos)
{
	if(IsSingleUserMode())
	{
		m_I->EmpireGetPatents(patentCIDs, maxQ, mediaPos);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ProcessSelected;
		out << (quint8)0xf5;	// Empire request patents.
		out << (quint16)patentCIDs.size();
		for(QSet<COMMODITYID>::const_iterator pi = patentCIDs.cbegin(); pi != patentCIDs.cend(); pi++)
			out << (quint16)*pi;
		out << maxQ;
		out << mediaPos;
		QueueChatPacket(ba);
	}
}

void AuApp::BdManufactureProcessesClear(const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdManufactureClear(opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ProcessSelected;
		out << (quint8)0xff;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BdManufactureProcessLowerMPID(quint16 mpid, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdManufactureLowerMPID(mpid, opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ProcessSelected;
		out << (quint8)0xf3;
		out << mpid;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BdManufactureProcessRaiseMPID(quint16 mpid, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdManufactureRaiseMPID(mpid, opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ProcessSelected;
		out << (quint8)0xf2;
		out << mpid;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BdManufactureProcessMaxMPID(quint16 mpid, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdManufactureMaxMPID(mpid, opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ProcessSelected;
		out << (quint8)0xfd;
		out << mpid;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BdManufactureProcessMinMPID(quint16 mpid, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdManufactureMinMPID(mpid, opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ProcessSelected;
		out << (quint8)0xfe;
		out << mpid;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BdManufactureAddMPID(quint16 mpid, quint8 minQ, quint32 workshops, quint32 stockLimit, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdManufactureAddMPID(mpid, minQ, workshops, stockLimit, opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ProcessSelected;
		out << (quint8)0xf1;
		out << mpid;
		out << minQ;
		out << workshops;
		out << stockLimit;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BdManufactureDeleteMPID(quint16 mpid, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdManufactureDeleteMPID(mpid, opBd);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ProcessSelected;
		out << (quint8)0xf0;
		out << mpid;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::StartSalesDialog()
{
	if(IsSingleUserMode())
	{
		m_I->HailStore();
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PC_HailStore;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsSetLightBulbProperties(const AuScSelection& partIds, const QColor& color, float kL, float kQ,
		float cosOuterCone, float cosInnerCone)
{
	if(partIds.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsSetLightBulbProperties(partIds, color, kL, kQ, cosOuterCone, cosInnerCone);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)41;
		out << partIds;
		out << color;
		out << kL;
		out << kQ;
		out << cosOuterCone;
		out << cosInnerCone;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsSetLightLensProperties(const AuScSelection& partIds, const QColor& color)
{
	if(partIds.empty())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartsSetLightLensProperties(partIds, color);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)33;
		out << partIds;
		out << color;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartRenameStop(quint16 partId, const QString& oldStopName, const QString& nuStopName)
{
	if(oldStopName.isEmpty() || !IsNameValid(oldStopName))
		return;
	if(nuStopName.isEmpty() || !IsNameValid(nuStopName))
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerPartRenameStop(partId, oldStopName, nuStopName);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)91;
		out << partId;
		out << oldStopName;
		out << nuStopName;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerEffectIntensityVertices(const AuScSelection& sel, float intensity)
{
	if(intensity < 0 || intensity > 1)
		return;
	if(!sel.HasObjectSelection())
		return;

	if(IsSingleUserMode())
	{
		m_I->DesignerEffectIntensityVertices(sel, intensity);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)92;
		out << sel;
		out << intensity;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPreviewPeople(const AuSpecies& species)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerPreviewPeople(species);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)94;
		out << species;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerViewPositionLoad(const AuId& avatarId, quint8 number)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerViewPositionLoad(avatarId, number);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_Position;
		out << (quint8)1;		// Load
		out << avatarId;
		out << number;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerViewPositionGrid(const AuId& avatarId, const AuDesignPosition& pos)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerViewPositionGrid(avatarId, pos);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_Position;
		out << (quint8)7;		// Grid
		out << avatarId;
		out << pos;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerViewPositionOrigin(const AuId& avatarId)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerViewPositionOrigin(avatarId);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_Position;
		out << (quint8)3;		// Origin
		out << avatarId;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerViewPositionSave(const AuId& avatarId, quint8 number)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerViewPositionSave(avatarId, number);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_Position;
		out << (quint8)2;		// Save
		out << avatarId;
		out << number;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerGridPositionSave(quint8 number, const AuMatrix4x4& gridM)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerGridPositionSave(number, gridM);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_Position;
		out << (quint8)5;		// Save
		out << number;
		out << gridM;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerSetDNA(const AuDNABase* dna)
{
	if(!IsSingleUserDesignerMode())
		return;

	// Local client-only designer mode.
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaPerson* person = dynamic_cast<DaPerson*>(ether.FindNormalAtom(m_I->AvatarId()));
	if(person)
	{
		person->SetDNA(*dna);
		person->ResetHitPoints();
		person->ResetHunger();
		person->ResetStamina();
		person->SetModified();
	}
	g_Au.UnlockEtheriumDelete();
}

void AuApp::BdBuildFence(const AuPoint& startLoc, const AuPoint& endLoc, int fenceStyle, const AuId& onPlatform,
		quint32 customDesignId, COMMODITYID mat)
{
	if(IsSingleUserMode())
	{
		m_I->BdFence(startLoc, endLoc, fenceStyle, onPlatform, customDesignId, mat);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PB_PlanFence;
		out << startLoc;
		out << endLoc;
		out << (quint8)fenceStyle;
		out << onPlatform;
		out << customDesignId;
		out << (quint16)mat;
		QueueChatPacket(ba);
	}
}

void AuApp::DesignerPartsSetGroup(const AuScIdSet& parts, const QString& groupName)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerPartsSetGroup(parts, groupName);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScPart;
		out << (quint8)95;
		out << parts;
		out << groupName;
		QueueChatPacket(ba);
	}
}

void AuApp::AgentEncounterAsteroid(quint32 minutesFromNow)
{
	if(IsSingleUserMode())
	{
		m_I->AgentEncounterAsteroid(minutesFromNow);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Encounter;
		out << (quint8)2;											// Asteroid
		out << minutesFromNow;
		QueueChatPacket(ba);
	}
}

void AuApp::SetEmojis(const AuEmojiList& emojis)
{
	// Set emoji changes in local global data.
	g_Au.SetEmojis(emojis);

	// Send visage changes to server for facial expression control.
	if(!IsSingleUserMode())
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PC_Emoji;
		emojis.StoreVisages(out);
		QueueChatPacket(ba);
	}
}

// Insure we have the empire and specie records for this item in the local client data.
bool AuApp::QueryEmpireAndSpecie(const AuItem& item)
{
	quint32 empireOfOrigin = item.EmpireOfOrigin();

	const DgData& gd = g_Au.LockDataForRead();

	// Do we need to request an empire record?
	bool requestEmpire;
	if(empireOfOrigin)
		requestEmpire = !gd.HasEmpire(empireOfOrigin);
	else
		requestEmpire = false;	// Don't request empire.

	// Do we need to request a specie record?
	bool requestSpecie;
	if(item.DNA())
		requestSpecie = !gd.HasSpecies(item.DNASpecies());
	else
		requestSpecie = false;

	g_Au.UnlockData();

	// Perform the requests now that the mutex is unlocked.
	if(requestEmpire)
		EmpireRequested(empireOfOrigin);

	if(requestSpecie)
		SpeciesRequested(item.DNASpecies());

	// Recurse into containers.
	const AuItemContent* contents = item.Content();
	if(contents)
		for(AuItemContent::const_iterator ci = contents->cbegin(); ci != contents->cend(); ci++)
			if(QueryEmpireAndSpecie(*(*ci).second))
				requestEmpire = true;	// Not necessarily empire; could be specie. This will make us return true when finished.

	// Return true if we had to request anything.
	return requestEmpire || requestSpecie;
}

// Insure we have the empire and specie data loaded for all items in a gear, including items in containers.
bool AuApp::QueryEmpireAndSpecie(const AuGear& inventory)
{
	bool anyQueries = false;
	for(AuGear::const_iterator ii = inventory.cbegin(); ii != inventory.cend(); ii++)
		if(QueryEmpireAndSpecie((*ii).Item()))
			anyQueries = true;
	return anyQueries;
}

// Insure we have the empire and specie data loaded for all items in a cargo inventory, not including items in containers.
bool AuApp::QueryEmpireAndSpecie(const AuItemHash& cargo)
{
	bool anyQueries = false;
	for(AuItemHash::const_iterator ii = cargo.begin(); ii != cargo.end(); ii++)
		if(QueryEmpireAndSpecie(*(*ii).second))
			anyQueries = true;
	return anyQueries;
}

void AuApp::CloseStory(AuJobOffer jobToClose)
{
	if(IsSingleUserMode())
	{
		m_I->CloseJob(jobToClose.Job());
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PS_CloseStory;
		out << jobToClose;
		QueueChatPacket(ba);
	}
}

void AuApp::RequestEmpire(quint32 empireId)
{
	if(!empireId)
		return;

	if(IsSingleUserMode())
	{
		// Not needed in solo. All empires are already here.
		qDebug() << "AuApp::RequestEmpire" << empireId;
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PG_Empire;
		out << (quint32)empireId;
		QueueScenePacket(ba);								// Scene channel handles empire requests.
	}
}

void AuApp::AcceptJob(const AuJobOffer& jobOffer)
{
	if(IsSingleUserMode())
	{
		m_I->JobAccepted(jobOffer);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PJ_Accept;
		out << jobOffer;
		QueueChatPacket(ba);
	}
}

void AuApp::ChangeRoadConstruction(const AuId& cityId, const AuGlobeLLF& roadDevLL, CONSTRUCTIONPROCESSID cpid)
{
	if(IsSingleUserMode())
	{
		m_I->ChangeRoadConstruction(cityId, roadDevLL, cpid);
	}
	else
	{
		// Transmit the data to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PB_ChangeRoadConstruction;
		out << cityId;
		out << roadDevLL;
		out << (quint8)cpid;
		QueueChatPacket(ba);
	}
}

void AuApp::StoryHint(quint32 storyInstance)
{
	if(IsSingleUserMode())
	{
		m_I->StorySendHint(storyInstance);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PS_StoryHint;
		out << storyInstance;
		QueueChatPacket(ba);
	}
}

void AuApp::StoryResendBlock(quint32 storyInstance)
{
	if(IsSingleUserMode())
	{
		m_I->StorySendCurrentBlock(storyInstance);
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PS_StoryResendBlock;
		out << storyInstance;
		QueueChatPacket(ba);
	}
}

void AuApp::RequestEmpiresAll()
{
	if(IsSingleUserMode())
	{
		// Not needed in solo. All empires are already here.
		qDebug() << "AuApp::RequestEmpiresAll";
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PG_EmpiresAll;
		QueueScenePacket(ba);			// Must send on scene channel for master, who we do not connect with a chat channel.
	}
}

void AuApp::RequestEmpiresJoinable()
{
	if(IsSingleUserMode())
	{
		// Not needed in solo. All empires are already here.
		qDebug() << "AuApp::RequestEmpiresJoinable";
	}
	else
	{
		// Transmit the request to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PG_EmpiresJoinable;
		QueueScenePacket(ba);			// Must send on scene channel for master, who we do not connect with a chat channel.
	}
}

void AuApp::ClearBd(const AuId& bdId)
{
	if(IsSingleUserMode())
	{
		m_I->DemolishBd(bdId);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PB_ClearLand;
		out << (quint8)1;	// Clear bd
		out << bdId;
		QueueChatPacket(ba);
	}
}

void AuApp::ClearGeo(const AuGlobeLLF& geoLL)
{
	if(IsSingleUserMode())
	{
		m_I->ClearGeo(geoLL);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PB_ClearLand;
		out << (quint8)3;	// Clear geo
		out << geoLL;
		QueueChatPacket(ba);
	}
}

void AuApp::UpgradeDNA(const AuDNA& dna)
{
	if(IsSingleUserMode())
	{
		m_I->SetDNA(dna);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_AvatarSetting;
		out << (quint8)3;   // dna4
		out << dna;
		QueueChatPacket(ba);
	}
}

void AuApp::NameTheBaby(const QString& name)
{
	if(IsSingleUserMode())
	{
		m_I->NameTheBaby(name);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_AvatarSetting;
		out << (quint8)0;   // name
		out << name;
		QueueChatPacket(ba);
	}
}

void AuApp::SetAutoEat(int autoEat)
{
	if(IsSingleUserMode())
	{
		m_I->SetAutoEat(autoEat);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_AvatarSetting;
		out << (quint8)1;   // auto eat
		out << (quint8)autoEat;
		QueueChatPacket(ba);
	}
}

void AuApp::SetHairStyle(AuHairStyle::STYLE haircut, AuHairStyle::SHAVE shave)
{
	if(IsSingleUserMode())
	{
		m_I->SetHairStyle(haircut, shave);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_AvatarSetting;
		out << (quint8)2;   // hairstyle
		out << (quint8)haircut;
		out << (quint8)shave;
		QueueChatPacket(ba);
	}
}

void AuApp::PutIntoCargo(const AuItem& item)
{
	if(IsSingleUserMode())
	{
		m_I->PutIntoCargo(item);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_PutIntoCargo;
		out << item.Type();
		out << item;
		QueueChatPacket(ba);
	}
}

void AuApp::GetFromCargo(const AuItem& item)
{
	if(IsSingleUserMode())
	{
		m_I->GetFromCargo(item);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_GetFromCargo;
		out << item.Type();
		out << item;
		QueueChatPacket(ba);
	}
}

void AuApp::KickOrLeaveEmpire(const AuId& personId, const QString& personName, quint32 idOldEmpire)
{
	if(IsSingleUserMode())
	{
		if(personId == AvatarId())
			m_I->GovLeaveEmpire(personId, personName, idOldEmpire);
		else
			m_I->GovKick(personId, personName, idOldEmpire);
	}
	else
	{
		// Transmit the data to the server and wait for a reply.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PE_LeaveEmpire;
		out << personId;
		out << personName;
		out << idOldEmpire;
		QueueChatPacket(ba);
	}
}

void AuApp::DeletePerson(const AuId& personId)
{
	if(IsSingleUserMode())
	{
		DgData& gd = g_Au.LockDataForWrite();
		gd.ClientUnitDeleted(personId);
		g_Au.UnlockData();
	}
	else
	{
		// Transmit the data to the next server and wait for a reply.
		// A good reply from the SceneManager will be deferral to the final SceneWorker.
		// A good reply from the SceneWorker will be a login packet identifying our new
		// person unit.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PE_DeletePerson;
		personId >> out;
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
			g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: Sending delete person TCP packet, person %1.")
					.arg(personId.StringId()));
		QueueScenePacket(ba);
	}
}

void AuApp::SetAFK(bool afk)
{
	if(IsSingleUserMode())
	{
		// Not needed in solo.
		qDebug() << "AuApp::SetAFK" << afk;
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PL_Setting;
		out << (quint8)0;					// AFK setting
		out << afk;
		QueueScenePacket(ba);
	}
}

void AuApp::RequestBasicGameData()
{
	if(IsSingleUserMode())
	{
		// Not needed in solo.
		qDebug() << "AuApp::RequestBasicGameData";
	}
	else
	{
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
            g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: Sending game data request TCP packet."));
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PG_Data;
		QueueScenePacket(ba);
	}
}

void AuApp::SceneSyncComplete(const AuTime& serverUTC)
{
	if(IsSingleUserMode())
	{
		// Not needed in solo.
		qDebug() << "AuApp::SceneSyncComplete";
	}
	else
	{
		if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
            g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: Sending game data request TCP packet."));
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::P_SyncComplete;
		serverUTC.Tx32(out);
		QueueScenePacket(ba);
	}
	SceneSyncComplete();
}

void AuApp::SendPosition(quint16 hasWhat, float headTilt, const AuAtomPos& pos, const AuPose pose[DpBody::PS_Count],
		quint16 whichConsole, quint16 stationId, const AuPoint& vec, quint8 seatIndex, const AuAtomPos& parentPos,
		const AuPose parentPose[DpBody::PS_Count], quint16 parentWhichConsole, quint16 parentStationId, const AuPoint& parentVec)
{
	if(IsSingleUserMode())
	{
		// Not needed in solo.
		qDebug() << "AuApp::SendPosition";
	}
	else
	{
		// Transmit the position report to the server.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtSceneServerPacket::PI_Position;
		out << hasWhat;
		if(hasWhat & 0x0001)
			out << headTilt;
		if(hasWhat & 0x0002)
			pos.Tx(out);
		if(hasWhat & 0x0008)
		{
			for(int ps=0; ps<DpBody::PS_Count; ps++)
				out << pose[ps];
			out << whichConsole;
			out << stationId;
		}
		if(hasWhat & 0x0010)
			out << vec;
		if(hasWhat & 0x0040)
			out << seatIndex;

		if(hasWhat & 0x0200)
			parentPos.Tx(out);
		if(hasWhat & 0x0800)
		{
			for(int ps=0; ps<DpBody::PS_Count; ps++)
				out << parentPose[ps];
			out << parentWhichConsole;
			out << parentStationId;
		}
		if(hasWhat & 0x1000)
			out << parentVec;
		QueueScenePacket(ba);
	}
}

void AuApp::TransporterCoord(const AuPoint& personLoc)
{
	if(IsSingleUserMode())
	{
		m_I->TransporterCoord(personLoc);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PTR_InputCoord;
		out << personLoc;
		QueueChatPacket(ba);
	}
}

void AuApp::TransporterLongLat(const AuGlobeLLF& ll)
{
	if(IsSingleUserMode())
	{
		m_I->TransporterLongLat(ll);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PTR_InputLongLat;
		out << ll;
		QueueChatPacket(ba);
	}
}

void AuApp::EmpireChangeFlag(quint32 empireId, const QImage& imFlag)
{
	if(IsSingleUserMode())
	{
		//DHH solo agent
		qDebug() << "AuApp::EmpireChangeFlag" << empireId;
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_EmpireChgFlag;
		out << (quint32)empireId;
		out << imFlag;
		QueueChatPacket(ba);
	}
}

void AuApp::EmpireChangeFlag(const QImage& imFlag)
{
	if(IsSingleUserMode())
	{
		m_I->GovSetEmpireFlag(imFlag);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PE_ChangeFlag;
		out << imFlag;
		QueueChatPacket(ba);
	}
}

void AuApp::EmpireChangeName(quint32 empireId, const QString& name)
{
	if(IsSingleUserMode())
	{
		//DHH solo agent
		qDebug() << "AuApp::EmpireChangeName" << empireId << name;
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_EmpireChgName;
        out << (quint32)empireId;
        out << name;
		QueueChatPacket(ba);
	}
}

void AuApp::EmpireChangeEmperor(quint32 empireId, const AuId& avatarId, const QString& avatarName)
{
	if(IsSingleUserMode())
	{
		//DHH solo agent
		qDebug() << "AuApp::EmpireChangeEmperor" << empireId << avatarId << avatarName;
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_EmpireChgEmperor;
        out << (quint32)empireId;
        out << avatarId;
        out << avatarName;
		QueueChatPacket(ba);
	}
}

void AuApp::PirateSetIndustryBuilding(quint32 empireId, INDUSTRYID industry, quint32 blueprint)
{
	if(!DgEmpire::IsPirate(empireId))
		return;
	if(industry == IID_None)
		return;
	if(!blueprint)
		return;

	if(IsSingleUserMode())
	{
		//DHH solo agent
		qDebug() << "AuApp::PirateSetIndustryBuilding" << empireId << industry << blueprint;
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_EmpireChg;
		out << (quint8)0;
		out << empireId;
		out << (quint16)industry;
		out << blueprint;
		QueueChatPacket(ba);
	}
}

void AuApp::PirateSetSpacecraft(quint32 empireId, int scrole, quint32 designId)
{
	if(IsSingleUserMode())
	{
		//DHH solo agent
		qDebug() << "AuApp::PirateSetSpacecraft" << empireId << scrole << designId;
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_EmpireChg;
		out << (quint8)1;
		out << empireId;
		out << (quint8)scrole;
		out << designId;
		QueueChatPacket(ba);
	}
}

void AuApp::AgentKillDomesticAnimals()
{
	if(IsSingleUserMode())
	{
		DaPerson* person = m_I->LockAvatar();
		if(person)
		{
			DaAtomList sceneAtoms;
			person->ListSceneAtoms(sceneAtoms);
			for(DaAtomList::iterator sai = sceneAtoms.begin(); sai != sceneAtoms.end(); sai++)
			{
				DaAnimal* animal = dynamic_cast<DaAnimal*>(*sai);
				if(!animal)
					continue;
				if(animal->IsMounted())
					continue;	// Skip mounted animals.
				if(!animal->IsWild())
					animal->Kill();
			}
		}
		m_I->UnlockAvatar();
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_KillAnimals;
        out << (quint8)0;       // 0=domestic, 1=wild
		QueueChatPacket(ba);
	}
}

void AuApp::AgentKillWildAnimals()
{
	if(IsSingleUserMode())
	{
		DaPerson* person = m_I->LockAvatar();
		if(person)
		{
			DaAtomList sceneAtoms;
			person->ListSceneAtoms(sceneAtoms);
			for(DaAtomList::iterator sai = sceneAtoms.begin(); sai != sceneAtoms.end(); sai++)
			{
				DaAnimal* animal = dynamic_cast<DaAnimal*>(*sai);
				if(!animal)
					continue;
				if(animal->IsMounted())
					continue;	// Skip mounted animals.
				if(animal->IsWild())
					animal->Kill();
			}
		}
		m_I->UnlockAvatar();
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_KillAnimals;
        out << (quint8)1;       // 0=domestic, 1=wild
		QueueChatPacket(ba);
	}
}

void AuApp::AgentAerialEncounter()
{
	if(IsSingleUserMode())
	{
		m_I->AgentEncounter(3, 0);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Encounter;
		out << (quint8)3;                                           // Aerial animal encounter
		out << (quint32)0;
		QueueChatPacket(ba);
	}
}

void AuApp::AgentAnimalEncounter()
{
	if(IsSingleUserMode())
	{
		m_I->AgentEncounter(0, 0);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Encounter;
		out << (quint8)0;											// Animal encounter
		out << (quint32)0;
		QueueChatPacket(ba);
	}
}

void AuApp::AgentMonsterEncounter()
{
	if(IsSingleUserMode())
	{
		m_I->AgentEncounter(9, 0);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Encounter;
		out << (quint8)9;                                           // Monster encounter aboard a spacecraft
		out << (quint32)0;
		QueueChatPacket(ba);
	}
}

void AuApp::AgentIndigenousEncounter()
{
	if(IsSingleUserMode())
	{
		m_I->AgentEncounter(10, 0);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Encounter;
		out << (quint8)10;                                          // Indigenous encounter.
		out << (quint32)0;
		QueueChatPacket(ba);
	}
}

void AuApp::AgentIndigenousEncounterForceMove()
{
	if(IsSingleUserMode())
	{
		m_I->AgentEncounter(11, 0);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Encounter;
		out << (quint8)11;
		out << (quint32)0;
		QueueChatPacket(ba);
	}
}

void AuApp::AgentIndigenousEncounterForceMoveResetReputation()
{
	if(IsSingleUserMode())
	{
		m_I->AgentEncounter(12, 0);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Encounter;
		out << (quint8)12;
		out << (quint32)0;
		QueueChatPacket(ba);
	}
}

void AuApp::AgentEncounterPirateShip()
{
	if(IsSingleUserMode())
	{
		m_I->AgentEncounter(1, 0);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Encounter;
		out << (quint8)1;                                           // Pirate ship
		out << (quint32)0;
		QueueChatPacket(ba);
	}
}

void AuApp::AgentRegenPlantDNA(quint8 plantType, qint8 floraZone)
{
	if(IsSingleUserMode())
	{
		//DHH solo agent
		qDebug() << "AuApp::AgentRegenPlantDNA" << plantType << floraZone;
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_EnvironmentChange;
        out << plantType;       // 0=ground cover, 1=shrubbery, 2=trees, 3=set indigenous DNA to match agent,
                                // 4=clear indigenous DNA, 5=clear rat DNA, 6=set citizen DNA to match agent
        out << floraZone;       // 0=polar, 1=temperate, 2=tropical
		QueueChatPacket(ba);
	}
}

void AuApp::AgentSetCitizenDNA()
{
	if(IsSingleUserMode())
	{
		//DHH solo agent
		qDebug() << "AuApp::AgentSetCitizenDNA";
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_EnvironmentChange;
        out << (quint8)6;       // 0=ground cover, 1=shrubbery, 2=trees, 3=set indigenous DNA to match agent,
                                // 4=clear indigenous DNA, 5=clear rat DNA, 6=set citizen DNA to match agent
        out << (quint8)0;       // irrelevant to this operation
		QueueChatPacket(ba);
	}
}

void AuApp::AgentSetIndigenousDNA(qint8 floraZone)
{
	if(IsSingleUserMode())
	{
		//DHH solo agent
		qDebug() << "AuApp::AgentSetIndigenousDNA" << floraZone;
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_EnvironmentChange;
        out << (quint8)3;       // 0=ground cover, 1=shrubbery, 2=trees, 3=set indigenous DNA to match agent,
                                // 4=clear indigenous DNA, 5=clear rat DNA, 6=set citizen DNA to match agent
        out << floraZone;       // irrelevant to this operation
		QueueChatPacket(ba);
	}
}

void AuApp::AgentClearIndigenousDNA(qint8 floraZone)
{
	if(IsSingleUserMode())
	{
		//DHH solo agent
		qDebug() << "AuApp::AgentClearIndigenousDNA" << floraZone;
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_EnvironmentChange;
        out << (quint8)4;       // 0=ground cover, 1=shrubbery, 2=trees, 3=set indigenous DNA to match agent,
                                // 4=clear indigenous DNA, 5=clear rat DNA, 6=set citizen DNA to match agent
        out << floraZone;       // irrelevant to this operation
		QueueChatPacket(ba);
	}
}

void AuApp::ReportCityHistory(const AuId& cityId)
{
	if(IsSingleUserMode())
	{
		bool haveReport = false;
		bool personHasAgentPower = false;
		quint32 personEmpire = 0;
		AuCityReportHistory report;
		m_I->ReportCityHistory(cityId, haveReport, report, personHasAgentPower, personEmpire);
		if(haveReport)
			emit CityHistoryReportReady(cityId, report);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PE_Report;
		out << (quint8)1;   // 1=city report history
		out << cityId;
		QueueChatPacket(ba);
	}
}

void AuApp::ReportCityLast(const AuId& cityId)
{
	if(IsSingleUserMode())
	{
		bool haveReport = false;
		bool havePrevReport = false;
		bool personHasAgentPower = false;
		quint32 personEmpire = 0;
		AuCityReport report, prevReport;
		m_I->ReportCityLast(cityId, haveReport, report, havePrevReport, prevReport, personHasAgentPower, personEmpire);
		if(haveReport)
		{
			if(havePrevReport)
			{
				AuCityReportHistory rcomb;
				rcomb.Append(prevReport);
				rcomb.Append(report);
				emit CityHistoryReportReady(cityId, rcomb);
			}
			else
				emit CityLastReportReady(cityId, report);
		}
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PE_Report;
		out << (quint8)0;   // 0=last city history
		out << cityId;
		QueueChatPacket(ba);
	}
}

void AuApp::AgentSetBits(quint8 bits)
{
	if(IsSingleUserMode())
	{
		const DgData& gd = g_Au.LockDataForRead();
        AuId idUnit = gd.ClientUnitId();
        g_Au.UnlockData();

        // Local client-only mode.
        AuEther& ether = g_Au.LockEtheriumDelete();
        DaPerson* person = dynamic_cast<DaPerson*>(ether.FindAtom(idUnit));
        if(person)
            person->SetAgentBits(bits);
        g_Au.UnlockEtheriumDelete();
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_AgentBits;
        out << bits;
		QueueChatPacket(ba);
	}
}

void AuApp::AgentMakeItem(COMMODITYID cid, quint8 quality, quint32 count, quint8 galaxy, const AuPoint& galaxyCoord,
		AuAgentMakeItem::MAKEWHAT makeWhat)
{
	if(cid == CID_Invalid)
		return;
	if(IsSingleUserMode())
	{
		const DgData& gd = g_Au.LockDataForRead();
		AuId idUnit = gd.ClientUnitId();
		g_Au.UnlockData();

		// Local client-only mode.
		AuEther& ether = g_Au.LockEtheriumDelete();
		DaPerson* person = dynamic_cast<DaPerson*>(ether.FindAtom(idUnit));
		if(person)
			person->AgentMakeCommodity(makeWhat, cid, quality, count, (GALAXY)galaxy, galaxyCoord);
		g_Au.UnlockEtheriumDelete();
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_MakeCommodity;
		out << (quint16)cid;
		out << quality;
		out << count;
		out << galaxy;
		out << galaxyCoord;
		out << (quint8)makeWhat;
		QueueChatPacket(ba);
	}
}

void AuApp::StorySubmitScript(const AuId& avatarId, const AuScript& script)
{
	if(IsSingleUserMode())
	{
		m_I->StoryScriptSubmitted(avatarId, script);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PS_StorySubmission;
        out << (quint8)0;   // submit and launch
        out << avatarId;
        out << script;
		QueueChatPacket(ba);
	}
}

void AuApp::StoryCancelSubmission(const AuId& avatarId)
{
	if(IsSingleUserMode())
	{
		m_I->StoryClearScript(avatarId);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PS_StorySubmission;
        out << (quint8)1;   // clear
        out << avatarId;
		QueueChatPacket(ba);
	}
}

void AuApp::StoryReset(const AuId& avatarId, quint32 scriptId)
{
	if(IsSingleUserMode())
	{
		m_I->AgentResetStory(avatarId, scriptId);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_ResetStory;
		out << avatarId;
		out << scriptId;
		QueueChatPacket(ba);
	}
}

void AuApp::ChangeMyDNA(const AuDNABase& dna)
{
	if(IsSingleUserMode())
	{
		m_I->AgentSetDNA(AvatarId(), dna);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_SetDNA;
        out << AvatarId();
        dna.WriteDynamic(out);
		QueueChatPacket(ba);
	}
}

void AuApp::ShipToggleOwnership()
{
	if(IsSingleUserMode())
	{
		m_I->ToggleShipOwnership();
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_ChangeShip;
		out << (quint8)1;
		QueueChatPacket(ba);
	}
}

void AuApp::ShipScuttle(const QString& reason)
{
	if(IsSingleUserMode())
	{
		m_I->AgentScuttleShip(reason);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_ChangeShip;
        out << (quint8)0;                                       // scuttle
        out << reason;
		QueueChatPacket(ba);
	}
}

void AuApp::ShipRecover(const AuId& shipId)
{
	if(IsSingleUserMode())
	{
		m_I->AgentRecoverShip(shipId);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_Hangar;
		out << (quint8)11;
		out << shipId;
		QueueChatPacket(ba);
	}
}

void AuApp::ShipRefuel(quint8 fuelQ)
{
	if(!fuelQ)
		return;
	if(IsSingleUserMode())
	{
		DaPerson* person = m_I->LockAvatar();
		if(person)
		{
			// Refuel the person's parent vehicle or spacecraft.
			DaVehicle* vehicle = dynamic_cast<DaVehicle*>(person->Parent());
			if(vehicle)
			{
				// Refuel the vehicle.
				AuItem* fuel = AuItem::Create(vehicle->FuelType(), fuelQ, 0);
				if(fuel)
				{
					vehicle->Refuel(*fuel);
					delete fuel;
				}

				QString resultMsg = tr("%1 refueled with Q%2 %3.").arg(vehicle->Name())
					.arg(fuelQ)
					.arg(g_Au.CommodityName(vehicle->FuelType()));
				AuCommThought msg(resultMsg, person->Id(), person->Name());
				g_Au.SendAtomChat(msg);
			}
			else if(person->IsInSpacecraft())
			{
				// Refuel the spacecraft.
				DaVessel* ship = person->FindParentVesselInEnv();
				if(ship)
				{
					ship->Refuel(fuelQ);
					ship->SetModified();

					QString resultMsg = tr("%1 refueled with Q%2 Hydrogen.").arg(ship->NameWithEmpire()).arg(fuelQ);
					AuCommThought msg(resultMsg, person->Id(), person->Name());
					g_Au.SendAtomChat(msg);
				}
			}
			else
			{
				QString resultMsg = tr("Nothing refueled.");
				AuCommThought msg(resultMsg, person->Id(), person->Name());
				g_Au.SendAtomChat(msg);
			}
		}
		m_I->UnlockAvatar();
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Refuel;
        out << fuelQ;
		QueueChatPacket(ba);
	}
}

void AuApp::ShipRepair()
{
	if(IsSingleUserMode())
	{
		//DHH solo agent
		qDebug() << "AuApp::ShipRepair";
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Refuel;
		out << (quint8)0;
		QueueChatPacket(ba);
	}
}

void AuApp::ShipDrainCapacitor(quint8 percentCap)
{
	if(IsSingleUserMode())
	{
		//DHH solo agent
		qDebug() << "AuApp::ShipDrainCapacitor" << percentCap;
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Capacitor;
		out << (quint8)0;   // 0=drain, 1=fill
		out << percentCap;
		QueueChatPacket(ba);
	}
}

void AuApp::ShipFillCapacitor(quint8 percentCap)
{
	if(IsSingleUserMode())
	{
		//DHH solo agent
		qDebug() << "AuApp::ShipFillCapacitor" << percentCap;
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Capacitor;
		out << (quint8)1;   // 0=drain, 1=fill
		out << percentCap;
		QueueChatPacket(ba);
	}
}

void AuApp::ShipLoadOfficer()
{
	if(IsSingleUserMode())
	{
		m_I->AgentEncounter(4, 0);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Encounter;
		out << (quint8)4;                                           // Load Officer
		out << (quint32)0;
		QueueChatPacket(ba);
	}
}

void AuApp::ShipLoadCrew()
{
	if(IsSingleUserMode())
	{
		m_I->AgentEncounter(5, 0);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Encounter;
		out << (quint8)5;                                           // Load Crew
		out << (quint32)0;
		QueueChatPacket(ba);
	}
}

void AuApp::ShipLoadTroops()
{
	if(IsSingleUserMode())
	{
		m_I->AgentEncounter(6, 0);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Encounter;
		out << (quint8)6;                                           // Load Troops
		out << (quint32)0;
		QueueChatPacket(ba);
	}
}

void AuApp::ShipLoadPassengers()
{
	if(IsSingleUserMode())
	{
		m_I->AgentEncounter(7, 0);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Encounter;
		out << (quint8)7;                                           // Load Passengers
		out << (quint32)0;
		QueueChatPacket(ba);
	}
}

void AuApp::ShipPirateEquip()
{
	if(IsSingleUserMode())
	{
		m_I->AgentEncounter(8, 0);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Encounter;
		out << (quint8)8;                                           // Pirate equip
		out << (quint32)0;
		QueueChatPacket(ba);
	}
}

void AuApp::ShipClearRatDNA()
{
	if(IsSingleUserMode())
	{
		//DHH solo agent
		qDebug() << "AuApp::ShipClearRatDNA";
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_EnvironmentChange;
        out << (quint8)5;       // 0=ground cover, 1=shrubbery, 2=trees, 3=set indigenous DNA to match agent,
                                // 4=clear indigenous DNA, 5=clear rat DNA, 6=set citizen DNA to match agent
        out << (qint8)0;        // irrelevant to this operation
		QueueChatPacket(ba);
	}
}

void AuApp::AgentGoToLongLat(const AuGlobeLLF& ll, bool rideToo)
{
	if(IsSingleUserMode())
	{
		m_I->AgentGoToLongLat(ll, rideToo);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_GoToLongLat;
		out << ll;
		out << rideToo;
		QueueChatPacket(ba);
	}
}

void AuApp::AgentGoToDestination(const AuId& goToId, bool rideToo, bool limboRescue)
{
	if(goToId.IsNull())
        return;
	if(IsSingleUserMode())
	{
		if(goToId != m_I->AvatarId())
			m_I->AgentGoToAtom(goToId, rideToo, false);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_GoToAtom;
        out << goToId;
        out << rideToo;
        out << limboRescue;
		QueueChatPacket(ba);
	}
}

void AuApp::AgentSummonPerson(const AuId& summonId, bool rideToo)
{
	if(IsSingleUserMode())
	{
		//DHH solo agent
		qDebug() << "AuApp::AgentSummonPerson" << summonId << "rideToo" << rideToo;
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_SummonPerson;
		out << summonId;
		out << rideToo;
		QueueChatPacket(ba);
	}
}

void AuApp::AgentGoToGalaxy(GALAXY galaxy, const AuPoint& coord, bool rideToo)
{
	if(IsSingleUserMode())
	{
		m_I->AgentGoToGalaxyLoc(galaxy, coord, rideToo);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_GoToGalaxyLoc;
		out << (quint8)galaxy;
		out << coord;
		out << rideToo;
		QueueChatPacket(ba);
	}
}

void AuApp::BankBorrow(double amount, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BankLoan(amount, opBd);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_BankBorrow;
		out << amount;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BankPay(double amount, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BankPay(amount, opBd);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_BankPay;
		out << amount;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BdExchangeBuyDesign(quint32 designId, const AuGearPos& mediaPos)
{
	if(!mediaPos.IsValid())
        return;
	if(IsSingleUserMode())
	{
		m_I->BdDesignExchangePurchase(designId, mediaPos);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_BdExchange;
		out << (quint8)3;                                           // Purchase design from the exchange.
		out << designId;
		out << mediaPos;
		QueueChatPacket(ba);
	}
}

void AuApp::BdExchangePublish(AuBdDesignExchangeEntry& libEntry)
{
	if(IsSingleUserMode())
	{
		m_I->BdDesignExchangePublish(libEntry);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_BdExchange;
		out << (quint8)0;                                       // Publish design to the exchange.
		out << libEntry;
		QueueChatPacket(ba);
	}
}

void AuApp::BdExchangeUnpublish(quint32 designId)
{
	if(IsSingleUserMode())
	{
		m_I->BdDesignExchangeUnpublish(false, designId, false);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_BdExchange;
        out << (quint8)2;                                           // Unpublish blueprint.
        out << designId;
		QueueChatPacket(ba);
	}
}

void AuApp::BdExchangeRate(quint32 designId, quint8 rating, const QString& comment)
{
	if(IsSingleUserMode())
	{
		AuFrame* frame = Frame();
		if(!frame)
			return;
		AuView* view = frame->View();
		if(!view)
			return;
		AuId personId = view->SceneAtomId();
		QString personName = view->SceneAtomName();
		m_I->BdDesignExchangeRate(AuExchangeRatingRec(personId, personName, designId, rating, comment, AuTime::Now()));
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_BdExchange;
		out << (quint8)5;                                           // Rate design in the exchange.
		out << designId;
		out << rating;
		out << comment;
		QueueChatPacket(ba);
	}
}

void AuApp::BdExchangeViewRatings(quint32 designId)
{
	if(!designId)
        return;
	if(IsSingleUserMode())
	{
		AuExchangeBlueprintRating rating;
		const DgData& sdl = g_Au.LockBdExchangeForRead();
		bool hasRating = sdl.m_BdExchangeRatings->Rating(designId, rating);
		QString designName = sdl.m_BdExchange->DesignName(designId);
		g_Au.UnlockBdExchange();
		if(!hasRating)
			return;
		emit BdExchangeBlueprintRating(designId, designName, rating);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_BdExchange;
		out << (quint8)6;                                           // View design ratings.
		out << designId;
		QueueChatPacket(ba);
	}
}

void AuApp::BdLibraryRequestDesigns()
{
	if(IsSingleUserMode())
	{
		AuId personId;
		QString personName;
		m_I->Avatar(personId, personName);

		g_AuClient.DesignerClearBdList();
		const DgData& sdd = g_Au.LockBdDesignsForRead();
		for(AuBdDesignHash::const_iterator di = sdd.m_BdDesigns->cbegin(); di != sdd.m_BdDesigns->cend(); di++)
			if((*di).second.ArchitectId() == personId && !(*di).second.IsDeleted())
			{
				AuBdReport designReport;
				(*di).second.ReportDesign(designReport);
				g_AuClient.DesignerAddBdDesignListEntry(designReport, (*di).second.State(), (*di).second.Thumbnail());
			}
		g_Au.UnlockBdDesigns();
		emit BdLibraryUpdated();
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_BdLibrary;
		out << (quint8)1;                                           // List architects' designs available.
		QueueChatPacket(ba);
	}
}

void AuApp::TransporterPlaces()
{
	if(IsSingleUserMode())
	{
		QList<AuTransporterTarget> places;
		m_I->TransporterPlaces(places);
		for(QList<AuTransporterTarget>::const_iterator pi = places.cbegin(); pi != places.cend(); pi++)
			emit TransporterPlace(*pi);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PTR_Place;
		QueueChatPacket(ba);
	}
}

void AuApp::TransporterSpacecraft()
{
	if(IsSingleUserMode())
	{
		QList<AuTransporterShip> ships;
		m_I->TransporterSpacecraft(ships);
		for(QList<AuTransporterShip>::const_iterator si = ships.cbegin(); si != ships.cend(); si++)
			emit TransporterShip((*si).Id(), (*si).Name(), (*si).SpotDesc());
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PTR_Ship;
		QueueChatPacket(ba);
	}
}

void AuApp::TransporterTransporters()
{
	if(IsSingleUserMode())
	{
		QList<AuTransporterSystem> trans;
		m_I->TransporterTransporters(trans);
		for(QList<AuTransporterSystem>::const_iterator si = trans.cbegin(); si != trans.cend(); si++)
			emit TransporterTransporter((*si).Id(), (*si).Name(), (*si).SpotDesc(), (*si).Which());
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PTR_Transporter;
		QueueChatPacket(ba);
	}
}

void AuApp::PayInsurer(double amountToPay)
{
	if(IsSingleUserMode())
	{
		m_I->PayInsurer(amountToPay);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_Insurer;
		out << (quint8)0;   // Pay
		out << amountToPay; // amount is trimmed to exact insurance amount on server
		QueueChatPacket(ba);
	}
}

void AuApp::LandAddCurrentSystem()
{
	if(IsSingleUserMode())
	{
		m_I->DomainAddCurrent();
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_AvatarSetting;
        out << (quint8)6;   // add current system
		QueueChatPacket(ba);
	}
}

void AuApp::LandAddSystem(const AuId& systemId, const QString& systemName)
{
	if(IsSingleUserMode())
	{
		m_I->DomainAdd(systemId, systemName);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_AvatarSetting;
        out << (quint8)4;   // add domain system
        out << systemId;
        out << systemName;
		QueueChatPacket(ba);
	}
}

void AuApp::LandRemoveSystem(const AuId& systemId, const QString& systemName)
{
	if(IsSingleUserMode())
	{
		m_I->DomainRemove(systemId, systemName);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_AvatarSetting;
        out << (quint8)5;   // remove domain system
        out << systemId;
        out << systemName;
		QueueChatPacket(ba);
	}
}

void AuApp::QueryArenas()
{
	if(IsSingleUserMode())
	{
		unordered_map<AuId, AuArenaReady, AuIdHash> arenas;
		m_I->ArenaList(arenas);
		for(unordered_map<AuId, AuArenaReady, AuIdHash>::const_iterator ai = arenas.cbegin(); ai != arenas.cend(); ai++)
			emit ArenaReadySignal((*ai).second);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_QueryArenas;
		QueueChatPacket(ba);
	}
}

void AuApp::BankGARequestBalance(const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->GovtAcctRequestBalance(opBd);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PE_GABalance;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BrokerCreateShipment(const AuShipment& shipment, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		if(m_I->ShipmentCreate(shipment, opBd))
			BrokerRequestShipments(false, opBd);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Trade;
		out << (quint8)4;                                           // Create a new shipment.
		out << shipment;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BrokerDeleteShipment(const AuShipment& shipment, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		if(m_I->ShipmentDelete(shipment, opBd))
			BrokerRequestShipments(false, opBd);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Trade;
		out << (quint8)5;                                           // Delete a shipment.
		out << shipment;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BrokerAcceptTradeAgreement(const AuTradeAgreement& ta, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		if(m_I->TradeAgreementAccept(ta, opBd))
			BrokerRequestShipments(false, opBd);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Trade;
		out << (quint8)2;                                           // Accept a trade agreement.
		out << ta;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BrokerCounterTradeAgreement(const AuTradeAgreement& ta, const AuTradeAgreement& tac, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		if(m_I->TradeAgreementCounter(ta, tac, opBd))
			BrokerRequestShipments(false, opBd);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Trade;
		out << (quint8)3;                                           // Counter a trade proposal.
		out << ta;
		out << tac;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BrokerDeleteTradeAgreement(const AuTradeAgreement& ta, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		if(m_I->TradeAgreementDelete(ta, opBd))
			BrokerRequestShipments(false, opBd);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Trade;
		out << (quint8)1;                                           // Delete a trade agreement.
		out << ta;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BrokerRequestShipments(bool sayResult, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		AuShipmentList shipments;
		AuTradeAgreementList tas;
		int result = m_I->BrokerShipments(shipments, tas, opBd);

		emit BrokerShipmentsClear();
		for(AuShipmentList::const_iterator si = shipments.cbegin(); si != shipments.cend(); si++)
			emit BrokerShipment(*si);

		emit BrokerTradeAgreementsClear();
		for(AuTradeAgreementList::const_iterator ti = tas.cbegin(); ti != tas.cend(); ti++)
			emit BrokerTradeAgreement(*ti);

		if(!sayResult)
			return;

		// Notify the player of the result.
		QString resultMsg;
		if(result == 3)
			resultMsg = tr("Only authorized government officials can view shipments and trade agreements.");
		else if(shipments.empty() && tas.empty())
			resultMsg = tr("The broker has no trade agreements or shipments.");
		if(!resultMsg.isEmpty())
		{
			// Send the result on the Log channel.
			AuId personId;
			QString personName;
			m_I->Avatar(personId, personName);
			AuCommThought msg(resultMsg, personId, personName);
			g_Au.SendAtomChat(msg);
		}
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Trade;
		out << (quint8)6;                                           // List shipments.
		out << opBd;
		out << sayResult;
		QueueChatPacket(ba);
	}
}

void AuApp::BrokerCreateTradeAgreement(const AuTradeAgreement& ta, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		if(m_I->TradeAgreementCreate(ta, opBd))
			BrokerRequestShipments(false, opBd);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Trade;
		out << (quint8)0;                                           // Create a new trade proposal.
		out << ta;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BuildingTrashItem(const AuItem& item, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdDiscardItem(item, opBd);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ProcessSelected;
		out << (quint8)0xfc;
		out << item.Pack();
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BuildingTakeItem(const AuItem& item, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdTakeItem(item, opBd);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ProcessSelected;
		out << (quint8)0xf4;
		out << item.Pack();
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BuildingDemolish(const AuId& bldId)
{
	if(IsSingleUserMode())
	{
		m_I->DemolishBd(bldId);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PB_Demolish;
        out << (quint8)1;   // Sending bdid.
        out << bldId;
		QueueChatPacket(ba);
	}
}

void AuApp::FarmSetLivestock(const AuSpecies& species, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdManufactureSetLivestock(species, opBd);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ProcessSelected;
		out << (quint8)0xf9;
		out << species;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::HangarQuery()
{
	if(IsSingleUserMode())
	{
		QString shipDesc;
		quint32 shipDesign = 0;
		quint32 shipsInHangar = 0;
		m_I->HangarQuery(shipDesc, shipDesign, shipsInHangar);
		emit HangarQueryAnswered(shipDesc, shipDesign, shipsInHangar);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_Hangar;
		out << (quint8)0;           // 0=Query
		QueueChatPacket(ba);
	}
}

void AuApp::HangarLaunch(bool takeCommand)
{
	if(IsSingleUserMode())
	{
		if(m_I->HangarLaunch(takeCommand))
			HangarQuery();
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_Hangar;
		out << (quint8)2;           // 2=Launch
		out << takeCommand;
		QueueChatPacket(ba);
	}
}

void AuApp::HangarMothball(const AuId& shipId)
{
	if(IsSingleUserMode())
	{
		if(m_I->HangarMothball(shipId))
			HangarQuery();
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_Hangar;
		out << (quint8)1;           // 1=Mothball
		out << shipId;
		QueueChatPacket(ba);
	}
}

void AuApp::RepairShopRecover()
{
	if(IsSingleUserMode())
	{
		m_I->RepairShopRecover();
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_Hangar;  // Share hangar packet type so I can avoid some building.
		out << (quint8)10;          // 10=Recover from Stasis
		QueueChatPacket(ba);
	}
}

void AuApp::BuildingStoreCommodity(COMMODITYID storeCID, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdManufactureSetWarehouseStorage((COMMODITYID)storeCID, opBd);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ProcessSelected;
		out << (quint8)0xf8;
		out << (quint16)storeCID;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::StoreOfferToBuy(const AuItem& buyItem, const AuItem& offerItem, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->BdStoreBuyItem(buyItem, offerItem, opBd);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PM_ProcessSelected;
		out << (quint8)0xf7;
		out << buyItem.Pack();
		out << offerItem.Pack();
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::TransferSendCargo(const AuId& otherShipId, const AuItemHash& cargo)
{
	if(cargo.empty())
		return;
	if(IsSingleUserMode())
	{
		m_I->TransferSendCargo(otherShipId, cargo);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_TransferCargo;
        out << (quint8)5;                       // Send Cargo List
        out << otherShipId;
        out << cargo;
		QueueChatPacket(ba);
	}
}

void AuApp::TransferSendCargoItem(const AuId& otherShipId, const AuItem& item)
{
	if(IsSingleUserMode())
	{
		m_I->TransferSendCargo(otherShipId, item);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_TransferCargo;
        out << (quint8)3;                       // Send Cargo Item
        out << otherShipId;
        out << item.Type();
        out << item;
		QueueChatPacket(ba);
	}
}

void AuApp::TransferSendCitizen(const AuId& otherShipId, const AuId& citizenId, const AuId& swapId)
{
	if(IsSingleUserMode())
	{
		AuIdList citizenIds, swapIds;
		citizenIds.append(citizenId);
		if(!swapId.IsNull())
			swapIds.append(swapId);
		m_I->TransferSendCitizens(otherShipId, citizenIds, swapIds);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_TransferCargo;
		out << (quint8)7;                       // Send Citizen
		out << otherShipId;
		out << citizenId;
		out << swapId;
		QueueChatPacket(ba);
	}
}

void AuApp::TransferSendCitizens(const AuId& otherShipId, const AuIdList& citizenIds, const AuIdList& swapIds)
{
	if(citizenIds.empty())
		return;
	if(IsSingleUserMode())
	{
		m_I->TransferSendCitizens(otherShipId, citizenIds, swapIds);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_TransferCargo;
        out << (quint8)9;                       // Send Citizens List
        out << otherShipId;
        out << (quint16)citizenIds.count();
        for(int idx=0; idx<citizenIds.count(); idx++)
            out << citizenIds[idx];
        out << (quint16)swapIds.count();
        for(int idx=0; idx<swapIds.count(); idx++)
            out << swapIds[idx];
		QueueChatPacket(ba);
	}
}

void AuApp::TransferSendMoney(const AuId& otherShipId, double amountToTransfer)
{
	if(IsSingleUserMode())
	{
		m_I->TransferSendMoney(otherShipId, amountToTransfer);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_TransferCargo;
        out << (quint8)1;                       // Send Money
        out << otherShipId;
        out << amountToTransfer;
		QueueChatPacket(ba);
	}
}

void AuApp::TransferTakeCargo(const AuId& otherShipId, const AuItemHash& cargo)
{
	if(cargo.empty())
		return;
	if(IsSingleUserMode())
	{
		m_I->TransferTakeCargo(otherShipId, cargo);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_TransferCargo;
        out << (quint8)6;                       // Take Cargo List
        out << otherShipId;
        out << cargo;
		QueueChatPacket(ba);
	}
}

void AuApp::TransferTakeCargoItem(const AuId& otherShipId, const AuItem& item)
{
	if(IsSingleUserMode())
	{
		m_I->TransferTakeCargo(otherShipId, item);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_TransferCargo;
        out << (quint8)4;                       // Take Cargo Item
        out << otherShipId;
        out << item.Type();
        out << item;
		QueueChatPacket(ba);
	}
}

void AuApp::TransferTakeCitizen(const AuId& otherShipId, const AuId& citizenId, const AuId& swapId)
{
	if(IsSingleUserMode())
	{
		AuIdList citizenIds, swapIds;
		citizenIds.append(citizenId);
		if(!swapId.IsNull())
			swapIds.append(swapId);
		m_I->TransferTakeCitizens(otherShipId, citizenIds, swapIds);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_TransferCargo;
		out << (quint8)8;                       // Take Citizen
		out << otherShipId;
		out << citizenId;
		out << swapId;
		QueueChatPacket(ba);
	}
}

void AuApp::TransferTakeCitizens(const AuId& otherShipId, const AuIdList& citizenIds, const AuIdList& swapIds)
{
	if(citizenIds.empty())
		return;
	if(IsSingleUserMode())
	{
		m_I->TransferTakeCitizens(otherShipId, citizenIds, swapIds);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_TransferCargo;
        out << (quint8)10;                       // Take Citizens List
        out << otherShipId;
        out << (quint16)citizenIds.count();
        for(int idx=0; idx<citizenIds.count(); idx++)
            out << citizenIds[idx];
        out << (quint16)swapIds.count();
        for(int idx=0; idx<swapIds.count(); idx++)
            out << swapIds[idx];
		QueueChatPacket(ba);
	}
}

void AuApp::TransferTakeMoney(const AuId& otherShipId, double amountToTransfer)
{
	if(IsSingleUserMode())
	{
		m_I->TransferTakeMoney(otherShipId, amountToTransfer);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_TransferCargo;
        out << (quint8)2;                       // Take Money
        out << otherShipId;
        out << amountToTransfer;
		QueueChatPacket(ba);
	}
}

void AuApp::TransferSendItemsAll(const AuId& otherShipId)
{
	if(IsSingleUserMode())
	{
		m_I->TransferSendItemsLoose(otherShipId);
		m_I->TransferSendItemsStatic(otherShipId);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_TransferCargo;
		out << 11;
		out << otherShipId;
		QueueChatPacket(ba);
	}
}

void AuApp::TransferSendItemsLoose(const AuId& otherShipId)
{
	if(IsSingleUserMode())
	{
		m_I->TransferSendItemsLoose(otherShipId);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_TransferCargo;
		out << 13;
		out << otherShipId;
		QueueChatPacket(ba);
	}
}

void AuApp::TransferSendItemsStatic(const AuId& otherShipId)
{
	if(IsSingleUserMode())
	{
		m_I->TransferSendItemsStatic(otherShipId);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_TransferCargo;
		out << 12;
		out << otherShipId;
		QueueChatPacket(ba);
	}
}

void AuApp::TransferTakeItemsAll(const AuId& otherShipId)
{
	if(IsSingleUserMode())
	{
		m_I->TransferTakeItemsLoose(otherShipId);
		m_I->TransferTakeItemsStatic(otherShipId);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_TransferCargo;
		out << 14;
		out << otherShipId;
		QueueChatPacket(ba);
	}
}

void AuApp::TransferTakeItemsLoose(const AuId& otherShipId)
{
	if(IsSingleUserMode())
	{
		m_I->TransferTakeItemsLoose(otherShipId);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_TransferCargo;
		out << 16;
		out << otherShipId;
		QueueChatPacket(ba);
	}
}

void AuApp::TransferTakeItemsStatic(const AuId& otherShipId)
{
	if(IsSingleUserMode())
	{
		m_I->TransferTakeItemsStatic(otherShipId);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_TransferCargo;
		out << 15;
		out << otherShipId;
		QueueChatPacket(ba);
	}
}

void AuApp::TransferRequestCitizens(const AuId& shipId)
{
	if(shipId.IsNull())
		return;
	if(IsSingleUserMode())
	{
		QByteArray bacit;
		m_I->TransferListCitizens(shipId, bacit);
		emit TransferCitizensAvailable(bacit);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_TransferCargo;
		out << (quint8)17;                      // Request transferable citizens.
		out << shipId;
		QueueChatPacket(ba);
	}
}

void AuApp::CommCloseChannel(const QString& channelName)
{
	if(IsSingleUserMode())
	{
		// Not needed in solo.
		//qDebug() << "AuApp::CommCloseChannel" << channelName;
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PC_ChannelClosed;
		out << channelName;
		QueueChatPacket(ba);
	}
}

void AuApp::CommChannelOpened(const QString& channelName)
{
	if(IsSingleUserMode())
	{
		// Not needed in solo.
		//qDebug() << "AuApp::CommChannelOpened" << channelName;
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PC_ChannelOpened;
        out << channelName;
		QueueChatPacket(ba);
	}
}

void AuApp::MissionAppendOrder(const AuMissionOrder& order)
{
	if(!order.IsValid())
		return;
	if(IsSingleUserMode())
	{
		m_I->MissionAppendOrder(order);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PN_Mission;
        out << (quint8)2;                   // append mission order
        out << (quint8)order.Type();
        out << order;
		QueueChatPacket(ba);
	}
}

void AuApp::MissionInsertOrder(const AuMissionOrder& order)
{
	if(!order.IsValid())
		return;
	if(IsSingleUserMode())
	{
		m_I->MissionInsertOrder(0, order);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PN_Mission;
        out << (quint8)9;                   // insert mission order
        out << (quint8)order.Type();
        out << order;
		QueueChatPacket(ba);
	}
}

void AuApp::MissionInsertOrder(quint8 beforeOrderId, const AuMissionOrder& order)
{
	if(!order.IsValid())
		return;
	if(IsSingleUserMode())
	{
		m_I->MissionInsertOrder(beforeOrderId, order);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PN_Mission;
        out << (quint8)3;                   // insert mission order
		out << beforeOrderId;
        out << (quint8)order.Type();
        out << order;
		QueueChatPacket(ba);
	}
}

void AuApp::MissionNew(const AuMissionOrder& order)
{
	if(!order.IsValid())
		return;
	if(IsSingleUserMode())
	{
		m_I->MissionClear();
		m_I->MissionAppendOrder(order);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PN_Mission;
        out << (quint8)8;                   // new mission with attached order
        out << (quint8)order.Type();
        out << order;
		QueueChatPacket(ba);
	}
}

void AuApp::MissionSetActive(bool active)
{
	if(IsSingleUserMode())
	{
		m_I->MissionSetActive(active);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PN_Mission;
		out << (quint8)6;                   // set mission active state
		out << (quint8)active;
		QueueChatPacket(ba);
	}
}

void AuApp::MissionSetCycle(bool cycle)
{
	if(IsSingleUserMode())
	{
		m_I->MissionSetType(cycle);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PN_Mission;
		out << (quint8)5;                   // set mission cycle state
		out << (quint8)cycle;
		QueueChatPacket(ba);
	}
}

void AuApp::MissionNew(const AuMission& mission)
{
	if(IsSingleUserMode())
	{
		m_I->MissionNew(mission);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PN_Mission;
        out << (quint8)11;                  // new mission
        out << mission;
		QueueChatPacket(ba);
	}
}

void AuApp::PostBounty(const AuId& victimId, const QString& victimName, const QString& reason, double amount)
{
	if(IsSingleUserMode())
	{
		m_I->PostBounty(victimId, victimName, reason, amount);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PC_PostBounty;
        out << victimId;
        out << victimName;
        out << reason;
        out << amount;
		QueueChatPacket(ba);
	}
}

void AuApp::CommOpenChannelTo(const QString& channelName, const AuId& toId)
{
	if(IsSingleUserMode())
	{
		// Not needed in solo.
		qDebug() << "AuApp::CommOpenChannelTo" << channelName << "to" << toId;
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PC_OpenChannelTo;
		out << channelName;        // Channel name.
		out << toId;        // Target atom id.
		QueueChatPacket(ba);
	}
}

void AuApp::MissionRequestSensorContacts()
{
	if(IsSingleUserMode())
	{
		AuSensorBlipList sensorBlips;
		AuPoint sensorEnvLoc;
		double sensorRange = m_I->SensorContacts(sensorBlips, sensorEnvLoc);
		if(sensorRange > 0)
		{
			sensorBlips.RangeDiscard(sensorEnvLoc, sensorRange);

			// Bundle the sensor blips back up into a byte array so they can be emitted as a signal.
			QByteArray bablips;
			QDataStream out(&bablips, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << sensorBlips;

			emit MissionSensorContactsReceived(bablips);
		}
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PN_Mission;
		out << (quint8)17;                  // request sensor contacts for mission planning
		QueueChatPacket(ba);
	}
}

void AuApp::CreateNewEmpire(const QString& name, const QImage& flag)
{
	if(IsSingleUserMode())
	{
		m_I->CreateNewEmpire(name, flag);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_ChangeTitle;
		out << (quint8)3;
		out << name;
		out << flag;
		QueueChatPacket(ba);
	}
}

void AuApp::BankGADeposit(double amount, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->GovtAcctDepositFunds(amount, opBd);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PE_GADeposit;
		out << amount;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::BankGAWithdraw(double amount, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->GovtAcctWithdrawFunds(amount, opBd);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PE_GAWithdraw;
		out << amount;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::GovQuitClaim()
{
	if(IsSingleUserMode())
	{
		m_I->GovQuitClaim();
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_SetCapital;
		out << (quint8)2;
		QueueChatPacket(ba);
	}
}

void AuApp::ReportCitiesByMail(const AuIdList& cityIds)
{
	if(cityIds.isEmpty())
        return;
	if(IsSingleUserMode())
	{
		for(AuIdList::const_iterator ci = cityIds.cbegin(); ci != cityIds.cend(); ci++)
			m_I->MailCityLast(*ci);

		QString resultMsg;
		if(cityIds.count() == 1)
			resultMsg = tr("One city is sending me its last status report by mail.");
		else
			resultMsg = tr("%1 cities are sending me their last status report by mail.").arg(cityIds.count());
		AuId personId;
		QString personName;
		m_I->Avatar(personId, personName);
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PE_Report;
		out << (quint8)4;                   // 4=city report by mail batch
		out << cityIds;
		QueueChatPacket(ba);
	}
}

void AuApp::ReportCitiesHistoryByMail(const AuIdList& cityIds)
{
	if(cityIds.isEmpty())
        return;
	if(IsSingleUserMode())
	{
		for(AuIdList::const_iterator ci = cityIds.cbegin(); ci != cityIds.cend(); ci++)
			ReportCityHistory(*ci);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PE_Report;
		out << (quint8)3;                   // 3=city history batch
		out << cityIds;
		QueueChatPacket(ba);
	}
}

void AuApp::ReportCitiesLastByMail(const AuIdList& cityIds)
{
	if(cityIds.isEmpty())
        return;
	if(IsSingleUserMode())
	{
		for(AuIdList::const_iterator ci = cityIds.cbegin(); ci != cityIds.cend(); ci++)
			ReportCityLast(*ci);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PE_Report;
		out << (quint8)2;                   // 2=last city report batch
		out << cityIds;
		QueueChatPacket(ba);
	}
}

void AuApp::ItemMoveBackward(const AuId& itemId, double amount, bool relativeToItem)
{
	if(IsSingleUserMode())
	{
		m_I->MoveItemBackward(itemId, amount, relativeToItem);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_Manipulate;
		out << itemId;
		out << (quint8)3;
		out << amount;
		out << relativeToItem;
		QueueChatPacket(ba);
	}
}

void AuApp::ItemMoveDown(const AuId& itemId, double amount, bool relativeToItem)
{
	if(IsSingleUserMode())
	{
		m_I->MoveItemDown(itemId, amount, relativeToItem);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_Manipulate;
		out << itemId;
		out << (quint8)5;
		out << amount;
		out << relativeToItem;
		QueueChatPacket(ba);
	}
}

void AuApp::ItemMoveForward(const AuId& itemId, double amount, bool relativeToItem)
{
	if(IsSingleUserMode())
	{
		m_I->MoveItemForward(itemId, amount, relativeToItem);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_Manipulate;
		out << itemId;
		out << (quint8)2;
		out << amount;
		out << relativeToItem;
		QueueChatPacket(ba);
	}
}

void AuApp::ItemMoveLeft(const AuId& itemId, double amount, bool relativeToItem)
{
	if(IsSingleUserMode())
	{
		m_I->MoveItemLeft(itemId, amount, relativeToItem);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_Manipulate;
		out << itemId;
		out << (quint8)1;
		out << amount;
		out << relativeToItem;
		QueueChatPacket(ba);
	}
}

void AuApp::ItemMoveRight(const AuId& itemId, double amount, bool relativeToItem)
{
	if(IsSingleUserMode())
	{
		m_I->MoveItemRight(itemId, amount, relativeToItem);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_Manipulate;
		out << itemId;
		out << (quint8)0;
		out << amount;
		out << relativeToItem;
		QueueChatPacket(ba);
	}
}

void AuApp::ItemMoveUp(const AuId& itemId, double amount, bool relativeToItem)
{
	if(IsSingleUserMode())
	{
		m_I->MoveItemUp(itemId, amount, relativeToItem);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_Manipulate;
		out << itemId;
		out << (quint8)4;
		out << amount;
		out << relativeToItem;
		QueueChatPacket(ba);
	}
}

void AuApp::ItemReset(const AuId& itemId, const AuMatrix4x4& t, const QString& roomName)
{
	if(IsSingleUserMode())
	{
		m_I->ResetItemTransform(itemId, t, roomName);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_Manipulate;
		out << itemId;
		out << (quint8)12;
		out << t;
		out << roomName;
		QueueChatPacket(ba);
	}
}

void AuApp::ItemRotateXNeg(const AuId& itemId, double amount, bool relativeToItem)
{
	if(IsSingleUserMode())
	{
		m_I->RotateItemXNeg(itemId, amount, relativeToItem);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_Manipulate;
		out << itemId;
		out << (quint8)7;
		out << amount;
		out << relativeToItem;
		QueueChatPacket(ba);
	}
}

void AuApp::ItemRotateXPos(const AuId& itemId, double amount, bool relativeToItem)
{
	if(IsSingleUserMode())
	{
		m_I->RotateItemXPos(itemId, amount, relativeToItem);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_Manipulate;
		out << itemId;
		out << (quint8)6;
		out << amount;
		out << relativeToItem;
		QueueChatPacket(ba);
	}
}

void AuApp::ItemRotateYNeg(const AuId& itemId, double amount, bool relativeToItem)
{
	if(IsSingleUserMode())
	{
		m_I->RotateItemYNeg(itemId, amount, relativeToItem);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_Manipulate;
		out << itemId;
		out << (quint8)9;
		out << amount;
		out << relativeToItem;
		QueueChatPacket(ba);
	}
}

void AuApp::ItemRotateYPos(const AuId& itemId, double amount, bool relativeToItem)
{
	if(IsSingleUserMode())
	{
		m_I->RotateItemYPos(itemId, amount, relativeToItem);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_Manipulate;
		out << itemId;
		out << (quint8)8;
		out << amount;
		out << relativeToItem;
		QueueChatPacket(ba);
	}
}

void AuApp::ItemRotateZNeg(const AuId& itemId, double amount, bool relativeToItem)
{
	if(IsSingleUserMode())
	{
		m_I->RotateItemZNeg(itemId, amount, relativeToItem);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_Manipulate;
		out << itemId;
		out << (quint8)11;
		out << amount;
		out << relativeToItem;
		QueueChatPacket(ba);
	}
}

void AuApp::ItemRotateZPos(const AuId& itemId, double amount, bool relativeToItem)
{
	if(IsSingleUserMode())
	{
		m_I->RotateItemZPos(itemId, amount, relativeToItem);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_Manipulate;
		out << itemId;
		out << (quint8)10;
		out << amount;
		out << relativeToItem;
		QueueChatPacket(ba);
	}
}

void AuApp::ItemTake(const AuId& itemId)
{
	if(IsSingleUserMode())
	{
		m_I->TakeItem(itemId);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_Manipulate;
		out << itemId;
		out << (quint8)13;
		QueueChatPacket(ba);
	}
}

void AuApp::MissionAppendMission(const AuMission& mission)
{
	if(IsSingleUserMode())
	{
		m_I->MissionAppend(mission);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PN_Mission;
        out << (quint8)12;                  // mission append
        out << mission;
		QueueChatPacket(ba);
	}
}

void AuApp::MissionClear()
{
	if(IsSingleUserMode())
	{
		m_I->MissionClear();
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PN_Mission;
		out << (quint8)1;                   // clear mission
		QueueChatPacket(ba);
	}
}

void AuApp::MissionOrderDelete(quint8 orderId)
{
	if(IsSingleUserMode())
	{
		m_I->MissionDeleteOrder(orderId);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PN_Mission;
		out << (quint8)4;                   // delete mission order
		out << orderId;
		QueueChatPacket(ba);
	}
}

void AuApp::MissionOrderComplete()
{
	if(IsSingleUserMode())
	{
		m_I->MissionOrderComplete();
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PN_Mission;
		out << (quint8)7;                   // mission order complete
		QueueChatPacket(ba);
	}
}

void AuApp::MissionBurnCD(const QString& name, const AuGearPos& mediaPos)
{
	if(IsSingleUserMode())
	{
		m_I->MissionBurnCD(mediaPos, name);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PN_Mission;
        out << (quint8)10;                  // mission burn CD
        out << name;
        out << mediaPos;
		QueueChatPacket(ba);
	}
}

void AuApp::MissionLoad(const AuMission& mission)
{
	if(IsSingleUserMode())
	{
		m_I->MissionNew(mission);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PN_Mission;
        out << (quint8)11;                  // mission load
        out << mission;
		QueueChatPacket(ba);
	}
}

void AuApp::MissionOrderReplace(quint8 orderId, const AuMissionOrder& order)
{
	if(IsSingleUserMode())
	{
		m_I->MissionReplaceOrder(orderId, order);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PN_Mission;
		out << (quint8)18;                  // replace mission order
		out << orderId;
		out << (quint8)order.Type();
		out << order;
		QueueChatPacket(ba);
	}
}

void AuApp::OrdnanceRequest()
{
	if(IsSingleUserMode())
	{
		AuId loaderId;  // 'loaderId' is the id of the atom providing the ordnance.
		AuItemHash ordnance;
		m_I->OrdnanceList(loaderId, ordnance);

		// Bundle the ordnance back up into a byte array so they can be emitted as a signal.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << ordnance;
		emit OrdnanceReceived(loaderId, ba);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_Ordnance;
		out << (quint8)0;                       // Request list
		QueueChatPacket(ba);
	}
}

void AuApp::OrdnanceLoad(quint8 rack, const AuItem& item)
{
	if(rack < DaVehicle::RACK_Gun || rack > DaVehicle::RACK_Ord1)
		return;
	if(IsSingleUserMode())
	{
		m_I->OrdnanceLoad(rack, item);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_Ordnance;
		out << rack;                                        // Load ordnance to the specified rack.
		out << item.Pack();
		QueueChatPacket(ba);
	}
}

void AuApp::RequestPlayersOnline()
{
	if(IsSingleUserMode())
	{
		// Not needed in solo.
		qDebug() << "AuApp::RequestPlayersOnline";
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PE_Report;
		out << (quint8)5;                   // Players online status query.
		QueueChatPacket(ba);
	}
}

void AuApp::BuildingRename(const QString& nuName, const AuId& opBd)
{
	if(IsSingleUserMode())
	{
		m_I->Rename(5, nuName, opBd);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PE_Rename;
		out << (quint8)5;           // 5=building
		out << nuName;
		out << opBd;
		QueueChatPacket(ba);
	}
}

void AuApp::CityRename(const QString& nuName)
{
	if(IsSingleUserMode())
	{
		m_I->Rename(0, nuName, AuId());
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PE_Rename;
		out << (quint8)0;           // 0=City
		out << nuName;
		QueueChatPacket(ba);
	}
}

void AuApp::PlanetRename(const QString& nuName)
{
	if(IsSingleUserMode())
	{
		m_I->Rename(1, nuName, AuId());
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PE_Rename;
		out << (quint8)1;           // 1=planet
		out << nuName;
		QueueChatPacket(ba);
	}
}

void AuApp::SectorRename(const QString& nuName)
{
	if(IsSingleUserMode())
	{
		m_I->Rename(3, nuName, AuId());
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PE_Rename;
		out << (quint8)3;           // 3=sector
		out << nuName;
		QueueChatPacket(ba);
	}
}

void AuApp::SystemRename(const QString& nuName)
{
	if(IsSingleUserMode())
	{
		m_I->Rename(2, nuName, AuId());
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PE_Rename;
		out << (quint8)2;           // 2=system
		out << nuName;
		QueueChatPacket(ba);
	}
}

void AuApp::ScExchangeRequestDesigns()
{
	if(IsSingleUserMode())
	{
		// Get my empire.
		quint32 empireId = 0;
		AuId personId;
		DaPerson* person = m_I->LockAvatar();
		if(person)
		{
			personId = person->Id();
			empireId = person->EmpireId();
		}
		m_I->UnlockAvatar();

		const DgData& sdl = g_Au.LockScExchangeForRead();
		AuScDesignExchange lib;
		sdl.m_ScExchange->ListAndRate(lib, personId, empireId, *sdl.m_ScExchangeRatings);   // List designs available to me.
		g_Au.UnlockScExchange();

		// Send the list of design ids and last modified times. The client will then request the specific entries needed.
		QHash<quint32, AuTime> toc; // Designs we will specifically request from the server.
		for(AuScDesignExchange::const_iterator li = lib.cbegin(); li != lib.cend(); li++)
			toc.insert((*li).DesignId(), (*li).LastMod());

		QList<quint32> designsNeeded;
		g_AuClient.ScExchangeToC(toc, designsNeeded);
		ScExchangeDesignsNeeded(designsNeeded);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScExchange;
		out << (quint8)10;											// List ToC of published blueprints available in the exchange.
		QueueChatPacket(ba);
	}
}

void AuApp::ScExchangeBuyDesign(quint32 designId, const AuGearPos& mediaPos)
{
	if(!mediaPos.IsValid())
        return;
	if(IsSingleUserMode())
	{
		m_I->ScDesignExchangePurchase(designId, mediaPos);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScExchange;
		out << (quint8)3;                                           // Purchase design from the exchange.
		out << designId;
		out << mediaPos;
		QueueChatPacket(ba);
	}
}

void AuApp::ScExchangePublish(AuScDesignExchangeEntry& libEntry)
{
	if(IsSingleUserMode())
	{
		m_I->ScDesignExchangePublish(libEntry);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScExchange;
		out << (quint8)0;                                       // Publish design to the exchange.
		out << libEntry;
		QueueChatPacket(ba);
	}
}

void AuApp::ScExchangeUnpublish(quint32 designId)
{
	if(IsSingleUserMode())
	{
		m_I->ScDesignExchangeUnpublish(false, designId, false);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScExchange;
        out << (quint8)2;                                           // Unpublish blueprint.
        out << designId;
		QueueChatPacket(ba);
	}
}

void AuApp::ScExchangeRate(quint32 designId, quint8 rating, const QString& comment)
{
	if(IsSingleUserMode())
	{
		AuFrame* frame = Frame();
		if(!frame)
			return;
		AuView* view = frame->View();
		if(!view)
			return;
		AuId personId = view->SceneAtomId();
		QString personName = view->SceneAtomName();
		m_I->ScDesignExchangeRate(AuExchangeRatingRec(personId, personName, designId, rating, comment, AuTime::Now()));
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScExchange;
		out << (quint8)5;                                           // Rate design in the exchange.
		out << designId;
		out << rating;
		out << comment;
		QueueChatPacket(ba);
	}
}

void AuApp::ScExchangeViewRatings(quint32 designId)
{
	if(!designId)
        return;
	if(IsSingleUserMode())
	{
		AuExchangeBlueprintRating rating;
		const DgData& sdl = g_Au.LockScExchangeForRead();
		bool hasRating = sdl.m_ScExchangeRatings->Rating(designId, rating);
		QString designName = sdl.m_ScExchange->DesignName(designId);
		g_Au.UnlockScExchange();
		if(!hasRating)
			return;
		emit ScExchangeBlueprintRating(designId, designName, rating);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScExchange;
		out << (quint8)6;                                           // View design ratings.
		out << designId;
		QueueChatPacket(ba);
	}
}

void AuApp::ScLibraryRequestDesigns()
{
	if(IsSingleUserMode())
	{
		AuId personId;
		QString personName;
		m_I->Avatar(personId, personName);

		g_AuClient.DesignerClearScList();
		const DgData& sdd = g_Au.LockScDesignsForRead();
		for(AuScDesignHash::const_iterator di = sdd.m_ScDesigns->cbegin(); di != sdd.m_ScDesigns->cend(); di++)
			if((*di).second.ArchitectId() == personId && !(*di).second.IsDeleted())
			{
				AuScReport designReport;
				(*di).second.ReportDesign(designReport);
				g_AuClient.DesignerAddScDesignListEntry(designReport, (*di).second.State(), (*di).second.Thumbnail());
			}
		g_Au.UnlockScDesigns();
		emit ScLibraryUpdated();
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScLibrary;
		out << (quint8)1;                                           // List architects' designs available.
		QueueChatPacket(ba);
	}
}

void AuApp::PostSelfie(const QImage& image)
{
	if(image.isNull())
		return;
	if(IsSingleUserMode())
	{
		m_I->Selfie(image);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_Selfie;
		out << image;
		QueueChatPacket(ba);
	}
}

void AuApp::ShipChristen(const QString& name)
{
	if(!IsNameValid(name))
		return;
	if(IsSingleUserMode())
	{
		m_I->Christen(name);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_Christen;
		out << name;
		QueueChatPacket(ba);
	}
}

void AuApp::ShipClaimTitle()
{
	if(IsSingleUserMode())
	{
		m_I->ClaimSpacecraftTitle();
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_ChangeTitle;
		out << (quint8)1;                   // 1=claim
		QueueChatPacket(ba);
	}
}

void AuApp::ShipReleaseTitle()
{
	if(IsSingleUserMode())
	{
		m_I->ReleaseSpacecraftTitle();
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_ChangeTitle;
		out << (quint8)2;                   // 2=free
		QueueChatPacket(ba);
	}
}

void AuApp::ShipCopyTitle(const AuGearPos& mediaPos)
{
	if(IsSingleUserMode())
	{
		m_I->CopySpacecraftTitle(mediaPos);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_ChangeTitle;
		out << (quint8)4;                   // 4=copy title
		out << mediaPos;
		QueueChatPacket(ba);
	}
}

void AuApp::ShipCreateShipment(const AuShipment& shipment)
{
	if(IsSingleUserMode())
	{
		if(m_I->ShipmentCreate(shipment))
			ShipRequestShipments(false);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Trade;
		out << (quint8)7;                                           // Create a new shipment.
		out << shipment;
		QueueChatPacket(ba);
	}
}

void AuApp::ShipDeleteShipment(const AuShipment& shipment)
{
	if(IsSingleUserMode())
	{
		if(m_I->ShipmentDelete(shipment))
			ShipRequestShipments(false);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Trade;
		out << (quint8)8;                                           // Delete a shipment.
		out << shipment;
		QueueChatPacket(ba);
	}
}

void AuApp::ShipRequestShipments(bool sayResult)
{
	if(IsSingleUserMode())
	{
		AuShipmentList shipments;
		bool isStation = false;
		int result = m_I->Shipments(shipments, isStation);

		emit BrokerShipmentsClear();
		for(AuShipmentList::const_iterator si = shipments.cbegin(); si != shipments.cend(); si++)
			emit BrokerShipment(*si);

		if(!sayResult)
			return;

		QString resultMsg;
		if(result == 3)
			resultMsg = tr("I have to be in the chain of command to view shipments.");
		else if(shipments.empty())
		{
			if(isStation)
				resultMsg = tr("The space station has no shipments scheduled.");
			else
				resultMsg = tr("Only a space station can schedule shipments.");
		}

		// Notify the player of the result.
		if(!resultMsg.isEmpty())
		{
			// Send the result on the Log channel.
			AuId personId;
			QString personName;
			m_I->Avatar(personId, personName);
			AuCommThought msg(resultMsg, personId, personName);
			g_Au.SendAtomChat(msg);
		}
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_Trade;
		out << (quint8)9;                                           // List shipments.
		out << sayResult;
		QueueChatPacket(ba);
	}
}

void AuApp::GearDeleteContents(const AuGearPos& gearPos, const QSet<COMMODITYID>& patentCIDs, const QSet<quint32>& scDesigns,
		const QSet<quint32>& bdDesigns, const QSet<QString>& missions, const QSet<QString>& papers)
{
	if(patentCIDs.empty() && scDesigns.empty() && bdDesigns.empty() && missions.empty() && papers.empty())
        return;
	if(IsSingleUserMode())
	{
		m_I->GearDeleteStoredData(gearPos, patentCIDs, scDesigns, bdDesigns, missions, papers);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PI_GearItem;
		out << gearPos;
		out << (quint8)2;   // 2 = delete stored data
		out << (quint16)patentCIDs.size();
		for(QSet<COMMODITYID>::const_iterator pi = patentCIDs.cbegin(); pi != patentCIDs.cend(); pi++)
			out << (quint16)*pi;
		out << (quint16)scDesigns.size();
		for(QSet<quint32>::const_iterator di = scDesigns.cbegin(); di != scDesigns.cend(); di++)
			out << *di;
		out << (quint16)bdDesigns.size();
		for(QSet<quint32>::const_iterator di = bdDesigns.cbegin(); di != bdDesigns.cend(); di++)
			out << *di;
		out << (quint16)missions.size();
		for(QSet<QString>::const_iterator mi = missions.cbegin(); mi != missions.cend(); mi++)
			out << *mi;
		out << papers;
		QueueChatPacket(ba);
	}
}

void AuApp::AvatarRename(const QString& nuName)
{
	if(IsSingleUserMode())
	{
		m_I->Rename(4, nuName, AuId());
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PE_Rename;
		out << (quint8)4;           // 4=avatar
		out << nuName;
		QueueChatPacket(ba);
	}
}

void AuApp::ScExchangeDesignsNeeded(const QList<quint32>& designsNeeded)
{
	if(IsSingleUserMode())
	{
		// Get my empire.
		quint32 empireId = 0;
		DaPerson* person = m_I->LockAvatar();
		if(person)
			empireId = person->EmpireId();
		m_I->UnlockAvatar();

		const DgData& sdl = g_Au.LockScExchangeForRead();
		AuScDesignExchange lib;
		sdl.m_ScExchange->ListAndRate(lib, m_I->AvatarId(), empireId, *sdl.m_ScExchangeRatings);    // List designs available to me.
		g_Au.UnlockScExchange();

		g_AuClient.DesignerClearScExchange();
		const DgData& sdd = g_Au.LockScDesignsForRead();
		for(AuScDesignExchange::const_iterator li = lib.cbegin(); li != lib.cend(); li++)
		{
			AuScDesignHash::const_iterator di = sdd.m_ScDesigns->find((*li).DesignId());
			if(di != sdd.m_ScDesigns->cend())
				g_AuClient.DesignerAddScExchangeEntry(*li, (*di).second.Thumbnail());
			else
				g_AuClient.DesignerAddScExchangeEntry(*li, QImage());
		}
		g_Au.UnlockScDesigns();

		emit ScExchangeUpdated();
	}
	else
	{
		if(designsNeeded.empty())
		{
			emit ScExchangeUpdated();
			return;
		}
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScExchange;
		out << (quint8)11;
		out << designsNeeded;
		QueueChatPacket(ba);
	}
}

void AuApp::BdExchangeDesignsNeeded(const QList<quint32>& designsNeeded)
{
	if(IsSingleUserMode())
	{
		// Get my empire.
		quint32 empireId = 0;
		DaPerson* person = m_I->LockAvatar();
		if(person)
			empireId = person->EmpireId();
		m_I->UnlockAvatar();

		const DgData& sdl = g_Au.LockBdExchangeForRead();
		AuBdDesignExchange lib;
		sdl.m_BdExchange->ListAndRate(lib, m_I->AvatarId(), empireId, *sdl.m_BdExchangeRatings);    // List designs available to me.
		g_Au.UnlockBdExchange();

		g_AuClient.DesignerClearBdExchange();
		const DgData& sdd = g_Au.LockBdDesignsForRead();
		for(AuBdDesignExchange::const_iterator li = lib.cbegin(); li != lib.cend(); li++)
		{
			AuBdDesignHash::const_iterator di = sdd.m_BdDesigns->find((*li).DesignId());
			if(di != sdd.m_BdDesigns->cend())
				g_AuClient.DesignerAddBdExchangeEntry(*li, (*di).second.Thumbnail());
			else
				g_AuClient.DesignerAddBdExchangeEntry(*li, QImage());
		}
		g_Au.UnlockBdDesigns();

		emit BdExchangeUpdated();
	}
	else
	{
		if(designsNeeded.empty())
		{
			emit BdExchangeUpdated();
			return;
		}
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_BdExchange;
		out << (quint8)11;
		out << designsNeeded;
		QueueChatPacket(ba);
	}
}

void AuApp::BdExchangeSaveToGear(const AuBdDesignExchangeEntry& libEntry, const AuGearPos& mediaPos)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerGearSaveBd(libEntry, mediaPos);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_BdExchange;
        out << (quint8)4;                                       // Store design to media.
        out << libEntry;
        out << mediaPos;
		QueueChatPacket(ba);
	}
}

void AuApp::ScExchangeSaveToGear(const AuScDesignExchangeEntry& libEntry, const AuGearPos& mediaPos)
{
	if(IsSingleUserMode())
	{
		m_I->DesignerGearSaveSc(libEntry, mediaPos);
	}
	else
	{
		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ScExchange;
        out << (quint8)4;                                       // Store design to media.
        out << libEntry;
        out << mediaPos;
		QueueChatPacket(ba);
	}
}

void AuApp::QueryPlayersOnline(AuPlayerOnlineStatusVector& players)
{
	if(IsSingleUserMode())
	{
		// Not needed in solo.
		qDebug() << "AuApp::QueryPlayersOnline";
	}
	else
	{
		// This query must be made to the master scene server. By now, the client is talking to a worker.
		QTcpSocket tcpSocket;
		tcpSocket.connectToHost(g_Au.SceneMasterName(), AuUtil::SceneMasterPort());
		tcpSocket.waitForConnected(10000);
		if(tcpSocket.state() != QAbstractSocket::UnconnectedState)
		{
			// The connection attempt succeeded.
			// Transmit the players query packet to the master server and wait for a reply.
			// We also send the version so it can be verified right off.
			// We send the player id and confirmation number because we're not actually logged into the master server. The
			// confirmation number gives the master a tiny bit of validity checking to insure we're allowed to do this.
			const DgData& gd = g_Au.LockDataForRead();
			quint32 playerId = gd.PlayerId();
			quint32 confirmNum = gd.ConfirmationNumber();
			g_Au.UnlockData();

			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtSceneServerPacket::PZ_PlayersQuery;
			out << playerId;
			out << confirmNum;
			AuUtil::WritePacketSize(tcpSocket, ba.size());
			tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten(10000);

			// The server will send back a reply, good or bad.
			quint32 packetSize = 0;
			int packetSizeBytesLeft = 0;
			tcpSocket.waitForReadyRead(10000);
			while(tcpSocket.state() != QAbstractSocket::UnconnectedState || tcpSocket.bytesAvailable())
			{
				// Pull in any data that is waiting for us.
				while(tcpSocket.bytesAvailable())
				{
					if(!packetSize || packetSizeBytesLeft)
					{
						AuUtil::ReadPacketSize(tcpSocket, packetSize, packetSizeBytesLeft);
						if(packetSizeBytesLeft)
							break;					// We don't have all of the packet size bytes yet.
					}

					// Wait until all of the bytes arrive.
					if(tcpSocket.bytesAvailable() < packetSize)
						break;

					// It is now safe to read our data packet from the socket buffer.
					QByteArray ba = tcpSocket.read(packetSize);
					packetSize = 0;

					// Read the contents of the packet.
					QDataStream block(ba);
					block.setVersion(QDataStream::Qt_4_1);

					// The first byte is always the packet type.
					quint8 ptype;
					block >> ptype;
					switch(ptype)
					{
					case DtSceneServerPacket::PZ_PlayersQuery:
						players << block;
						break;

					default:
						qWarning("RefreshPlayersOnline received unexpected packet type %d reply", ptype);
						break;
					}

					// Close the socket.
					if(tcpSocket.state() != QAbstractSocket::UnconnectedState)
						tcpSocket.disconnectFromHost();
				}

				if(tcpSocket.state() != QAbstractSocket::UnconnectedState)
					tcpSocket.waitForDisconnected(100);
			}
		}
	}
}

void AuApp::QueryStarmap(quint8 query, GALAXY galaxy, const AuId& sectorId, quint16 range, const AuId& personId,
		quint32 personEmpireId, const AuId& personSystemId, AuIdList& sectorIds, GALAXY& sectorGalaxy)
{
	if(IsSingleUserMode())
	{
		AuStarMapData starMap;
		m_I->CollectStarMapData(personEmpireId, starMap, query, galaxy, sectorId, range, personSystemId);
		g_AuClient.StarMapUpdate(starMap);
		for(unordered_map<AuId, AuStarMapSector, AuIdHash>::const_iterator si = starMap.m_Sectors.cbegin();
				si != starMap.m_Sectors.cend(); si++)
		{
			sectorIds.append((*si).first);
			sectorGalaxy = (*si).second.Galaxy();	// They should all have the same galaxy.
		}
	}
	else
	{
		// This query must be made to the master scene server. By now, the client is talking to a worker.
		sectorGalaxy = GALAXY_Count;	// galaxy of returned sectors when query is sector
		QTcpSocket tcpSocket;
		tcpSocket.connectToHost(g_Au.SceneMasterName(), AuUtil::SceneMasterPort());
		tcpSocket.waitForConnected(10000);
		if(tcpSocket.state() != QAbstractSocket::UnconnectedState)
		{
			// The connection attempt succeeded.
			// Transmit the players query packet to the master server and wait for a reply.
			// We also send the version so it can be verified right off.
			// We send the player id and confirmation number because we're not actually logged into the master server. The
			// confirmation number gives the master a tiny bit of validity checking to insure we're allowed to do this.
			const DgData& gd = g_Au.LockDataForRead();
			quint32 playerId = gd.PlayerId();
			quint32 confirmNum = gd.ConfirmationNumber();
			QList<quint32> empireIds;
			gd.m_gEmpires.EmpireIds(empireIds);
			g_Au.UnlockData();

			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtSceneServerPacket::PG_StarMap;
			out << playerId;
			out << confirmNum;
			out << personId;
			out << personEmpireId;
			out << personSystemId;
			out << query;
			if(query == 1)
				out << (quint8)galaxy;
			else if(query == 2)
			{
				out << sectorId;
				out << range;
			}
			// Send the list of empire ids that I already have. The server will send any additional ones I need with the star map.
			out << (quint16)empireIds.size();
			for(QList<quint32>::const_iterator ei = empireIds.cbegin(); ei != empireIds.cend(); ei++)
				out << *ei;
			AuUtil::WritePacketSize(tcpSocket, ba.size());
			tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten(10000);

			// The server will send back a reply, good or bad.
			quint32 packetSize = 0;
			int packetSizeBytesLeft = 0;
			tcpSocket.waitForReadyRead(10000);
			while(tcpSocket.state() != QAbstractSocket::UnconnectedState || tcpSocket.bytesAvailable())
			{
				// Pull in any data that is waiting for us.
				while(tcpSocket.bytesAvailable())
				{
					if(!packetSize || packetSizeBytesLeft)
					{
						AuUtil::ReadPacketSize(tcpSocket, packetSize, packetSizeBytesLeft);
						if(packetSizeBytesLeft)
							break;					// We don't have all of the packet size bytes yet.
					}

					// Wait until all of the bytes arrive.
					if(tcpSocket.bytesAvailable() < packetSize)
						break;

					// It is now safe to read our data packet from the socket buffer.
					QByteArray ba = tcpSocket.read(packetSize);
					packetSize = 0;

					// Read the contents of the packet.
					QDataStream block(ba);
					block.setVersion(QDataStream::Qt_4_1);

					// The first byte is always the packet type.
					quint8 ptype;
					block >> ptype;
					bool weAreDone = false;
					switch(ptype)
					{
					case DtSceneServerPacket::PG_StarMap:
						{
							//qDebug() << "AuLocatorStarMap::PostQuery PG_StarMap";
							AuStarMapData starMap;
							starMap << block;
							g_AuClient.StarMapUpdate(starMap);
							for(unordered_map<AuId, AuStarMapSector, AuIdHash>::const_iterator si = starMap.m_Sectors.cbegin();
									si != starMap.m_Sectors.cend(); si++)
							{
								sectorIds.append((*si).first);
								sectorGalaxy = (*si).second.Galaxy();	// They should all have the same galaxy.
							}
							// The server sends the PG_Empire packets first. We are done when we get the PG_StarMap packet.
							weAreDone = true;
						}
						break;

					case DtSceneServerPacket::PG_Empire:
						{
							//qDebug() << "AuLocatorStarMap::PostQuery PG_Empire";
							DgEmpire rec;
							rec.Rx(block);
							DgData& gd = g_Au.LockDataForWrite();
							gd.ReplaceEmpire(rec);	// Replaces existing record if it exists.
							g_Au.UnlockData();
						}
						break;

					default:
						qWarning("RequestData received unexpected packet type %d reply", ptype);
						break;
					}

					// Close the socket.
					if(weAreDone && tcpSocket.state() != QAbstractSocket::UnconnectedState)
						tcpSocket.disconnectFromHost();
				}

				if(tcpSocket.state() != QAbstractSocket::UnconnectedState)
					tcpSocket.waitForDisconnected(100);
			}
		}
	}
}

void AuApp::ItLibraryRequestDesigns()
{
	if(IsSingleUserMode())
	{
		// Not needed in solo.
		qDebug() << "AuApp::ItLibraryRequestDesigns";
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PD_ItLibrary;
		out << (quint8)1;                                           // List architects' designs available.
		QueueChatPacket(ba);
	}
}

void AuApp::AddCitizenToEmpire(quint32 oldEmpireId, quint32 nuEmpireId, AuId personId, QString personName, quint8 personGender)
{
	DgData& gd = g_Au.LockDataForWrite();
	gd.AddCitizenToEmpire(oldEmpireId, nuEmpireId, personId, personName, personGender);
	g_Au.UnlockData();
}

void AuApp::AtomTraveled(AuId traveler, AuId origSystem, AuId destSystem, AuId destSector)
{
	if(g_Au.m_LogMessages & LOG_Travel)
		qDebug() << "AuApp::AtomTraveled traveler" << traveler << "origSystem" << origSystem << "destSystem" << destSystem
			<< "destSector" << destSector;

	// Insure the originating system hasn't been revoked from me already.
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSolarSystem* orgSys = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(origSystem));
	if(!orgSys)
	{
		// Ignore this travel signal. The departing atom will be loaded by the next server. There the atom will have to decide to
		// do the handoff again.
		g_Au.UnlockEtheriumDelete();
		if(g_Au.m_LogMessages & LOG_Error)
			g_Au.Warning(QString("AuApp::AtomTraveled traveler %1 origSystem %2 not found. destSystem %3 destSect %4")
					.arg(traveler.GetID()).arg(origSystem.GetID()).arg(destSystem.GetID()).arg(destSector.GetID()));
		return;
	}

	// If the destination solar system is in my data, I can perform the transfer right here and now.
	DaAtom* destatom = ether.FindNormalAtom(destSystem);	// Could be a solar system or a galaxy.
	DaSolarSystem* destSys = dynamic_cast<DaSolarSystem*>(destatom);
	if(!destSys)
	{
		// In the case of intergalactic wormholes, destSys and destSector are the galaxy id.
		DaGalaxy* destGalaxy = dynamic_cast<DaGalaxy*>(destatom);
		if(destGalaxy)
		{
			AuIdList nuSectors;
			DaSector* sector = destGalaxy->CreateSectorCubeAt(destGalaxy->CoreSectorLoc(), nuSectors);
			destSys = sector->FindCoreSolarSystem();
		}
		if(!destSys)
		{
			g_Au.UnlockEtheriumDelete();
			if(g_Au.m_LogMessages & LOG_Error)
				qWarning() << "AuApp::AtomTraveled traveler" << traveler << "destSystem not found" << destSystem
					<< "destSector" << destSector;
			return;
		}
	}

	if(destSys->IsOnlyStarFieldLoaded() || !destSys->IsFinal())
	{
		DaGalaxy* galaxy = destSys->FindParentGalaxy();
		DaSector* dparsect = destSys->FindParentSector();
		if(dparsect)
		{
			AuPoint sectorLoc = dparsect->Location();
			AuIdList nuSectors;
			DaSector* sector = galaxy->CreateSectorCubeAt(sectorLoc, nuSectors);
			if(sector)
			{
				if(!destSys->IsFinal())
					destSys->FinalizeContents();
				destSys->SetStarFieldFlag(false);
				destSys->TickTockRecursive();
			}
		}
	}

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
		trav->ForcePos(AuAtomPos(destSys->Id(), trav->Transform()));
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
	}
	else
	{
		if(g_Au.m_LogMessages & LOG_Error)
		{
			if(trav)
				g_Au.Error(QString("AuApp::AtomTraveled traveler %1 is not traveling").arg(traveler.GetID()));
			else
				g_Au.Error(QString("AuApp::AtomTraveled traveler %1 not found").arg(traveler.GetID()));
		}
	}
	g_Au.UnlockEtheriumDelete();
}

void AuApp::AtomTraveledStargate(AuId traveler, AuId origSystem, AuId destGalaxy, AuPoint destGalaxyLoc)
{
	if(g_Au.m_LogMessages & LOG_Travel)
		qDebug() << "AuApp::AtomTraveledStargate traveler" << traveler << "origSystem" << origSystem << "destGalaxy" << destGalaxy
			<< "destGalaxyLoc" << destGalaxyLoc;

	// Insure the originating system hasn't been revoked from me already.
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSolarSystem* orgSys = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(origSystem));
	if(!orgSys)
	{
		// Ignore this travel signal. The departing atom will be loaded by the next server. There the atom will have to decide to
		// do the handoff again.
		g_Au.UnlockEtheriumDelete();
		if(g_Au.m_LogMessages & LOG_Error)
			qWarning() << "AuApp::AtomTraveledStargate traveler" << traveler << "origSystem not found" << origSystem
				<< "destGalaxy" << destGalaxy << "destGalaxyLoc" << destGalaxyLoc;
		return;
	}

	// See if we can identify the destination system locally.
	DaGalaxy* dGalaxy = dynamic_cast<DaGalaxy*>(ether.FindNormalAtom(destGalaxy));
	if(!dGalaxy)
	{
		g_Au.UnlockEtheriumDelete();
		if(g_Au.m_LogMessages & LOG_Error)
			qWarning() << "AuApp::AtomTraveledStargate traveler" << traveler << "origSystem" << origSystem
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
	if(!destSys)
	{
		g_Au.UnlockEtheriumDelete();
		if(g_Au.m_LogMessages & LOG_Error)
			qWarning() << "AuApp::AtomTraveledStargate traveler" << traveler << "destSystem not found" << destSystem
				<< "destGalaxy" << destGalaxy << "destGalaxyLoc" << destGalaxyLoc;
		return;
	}

	if(destSys->IsOnlyStarFieldLoaded() || !destSys->IsFinal())
	{
		DaGalaxy* galaxy = destSys->FindParentGalaxy();
		DaSector* destSector = destSys->FindParentSector();
		if(destSector)
		{
			AuPoint sectorLoc = destSector->Location();
			AuIdList nuSectors;
			DaSector* sector = galaxy->CreateSectorCubeAt(sectorLoc, nuSectors);
			if(sector)
			{
				if(!destSys->IsFinal())
					destSys->FinalizeContents();
				destSys->SetStarFieldFlag(false);
				destSys->TickTockRecursive();
			}
		}
	}

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
	}
	else
	{
		if(g_Au.m_LogMessages & LOG_Error)
		{
			if(trav)
				g_Au.Error(QString("AuApp::AtomTraveledStargate traveler %1 is not traveling")
						.arg(traveler.GetID()));
			else
				g_Au.Error(QString("AuApp::AtomTraveledStargate traveler %1 not found")
						.arg(traveler.GetID()));
		}
	}
	g_Au.UnlockEtheriumDelete();
}

void AuApp::AvatarAccountActive(AuIdList land)
{
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaUniverse* universe = ether.FindUniverse();
	if(universe)
		universe->AvatarAccountActiveRecursive(land);
	g_Au.UnlockEtheriumDelete();
}

void AuApp::AvatarBorn(AuId motherAvatar, AuId babyAvatar)
{
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaPerson* babyP = dynamic_cast<DaPerson*>(ether.FindNormalAtom(babyAvatar));
	if(babyP)
	{
		// Create a birth announcement for dad and add it as a thought condition. Dad will get it later if he's off line right now.
		// It's not crucial that he get this.
		DaPerson* dad = dynamic_cast<DaPerson*>(ether.FindNormalAtom(motherAvatar));
		if(dad && (dad->Gender() == Male || dad->Gender() == Hermaphrodite))
		{
			float mass = babyP->Mass() * KilogramsPerPound();
			QString birthAnnounce = QObject::tr("%1 was born at %2. %3 weighed %4kg.")
				.arg(babyP->Name())
				.arg(g_Au.CurrentUTC().toString())
				.arg(babyP->SheHe(true))
				.arg(mass, 0, 'f', 2);
			dad->AddCondition(new AuConThought(birthAnnounce, AuTime::Now()));
		}

		// Add the avatar to the player's solo "account" if there is an empty slot.
		AuDNABase* dna = babyP->DNAnew();
		DgData& gd = g_Au.LockDataForWrite();
		gd.AddClientUnit(babyP->Id(), babyP->Name(), dna, babyP->Chromatid(), babyP->Handedness(), CID_Invalid, CID_Invalid,
			CID_Invalid, 0, babyP->Haircut(), babyP->Shave());
		g_Au.UnlockData();
		delete dna;
	}
	g_Au.UnlockEtheriumDelete();
}

void AuApp::BountyPosted(AuId posterId, QString posterName, AuId victimId, QString reason, double reward, int why)
{
	// Locate the victim atom.
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaAtom* victim = ether.FindNormalAtom(victimId);
	if(victim)
		victim->AddBounty(posterId, posterName, reason, reward, why);
	g_Au.UnlockEtheriumDelete();
}

void AuApp::CityCaptured(quint32 oldEmpireId, quint32 nuEmpireId, AuId cityId)
{
	DgData& gd = g_Au.LockDataForWrite();
	gd.EmpireCityCaptured(oldEmpireId, nuEmpireId, cityId);
	g_Au.UnlockData();
}

void AuApp::CityCreated(quint32 empireId,
		quint8 cityType, AuId cityId, QString cityName,
		AuId founderId, QString founderName,
		AuId planetId, QString planetName,
		AuId systemId, QString systemName, AuPoint systemLoc,
		AuId sectorId, QString sectorName,
		AuId galaxyId, quint8 galaxy)
{
	DgData& gd = g_Au.LockDataForWrite();
	gd.AddCityToEmpire(empireId,
			(CITYTYPE)cityType, cityId, cityName,
			founderId, founderName,
			planetId, planetName,
			systemId, systemName, systemLoc,
			sectorId, sectorName,
			galaxyId, (GALAXY)galaxy, false, false);    // Newly created city does not start out as a sector capital.
	g_Au.UnlockData();

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
			pss->SetNameIncludePlanets(systemName, false);  // The worker server updates the database, not me.
		}
	}
	else if(g_Au.m_LogMessages & LOG_Error)
		qWarning("ERROR! DtServerChannel::ReadPacket P_CityCreated sector %d not found to rename", sectorId.GetID());
	g_Au.UnlockEtheriumDelete();
}

void AuApp::CityDecayed(quint32 empireId, AuId cityId)
{
	DgData& gd = g_Au.LockDataForWrite();
	gd.EmpireCityDecayed(empireId, cityId);
	g_Au.UnlockData();
}

void AuApp::CityFacilitiesChanged(AuId cityId, AuCityIndustryHash facil)
{
	DgData& gd = g_Au.LockDataForWrite();
	gd.CityFacilitiesChanged(cityId, facil);
	g_Au.UnlockData();
}

void AuApp::CityStatusChanged(AuId cityId, int alertCondition, qint8 morale)
{
	DgData& gd = g_Au.LockDataForWrite();
	gd.CityStatusChanged(cityId, (ALERT_CONDITION)alertCondition, morale);
	g_Au.UnlockData();
}

void AuApp::ClaimTitleUp(AuId propertyId, QString propertyName, AuId ownerId, QString ownerName, quint32 ownerEmpire)
{
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaUniverse* universe = ether.FindUniverse();
	if(universe)
		universe->ClaimTitleRecursive(propertyId, propertyName, ownerId, ownerName, ownerEmpire);
	g_Au.UnlockEtheriumDelete();
}

void AuApp::CreditCityBank(float fAmount, AuId cityId)
{
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

void AuApp::DemiavatarDied(AuId avatar)
{
	// Remove the avatar from my solo "account".
	DgData& gd = g_Au.LockDataForWrite();
	gd.ClientUnitDeleted(avatar);
	g_Au.UnlockData();
	emit AvatarsUpdated();
}

void AuApp::EmpireCityWorkerSpeciesChanged(quint32 empireId, AuId cityId, AuSpecies workerSpecies)
{
	DgData& gd = g_Au.LockDataForWrite();
	gd.SetEmpireCityWorkerSpecies(empireId, cityId, workerSpecies);    // Returns empire id if one is changed.
	g_Au.UnlockData();
}

void AuApp::EmpireRecordPatent(quint32 empireId, quint16 patentCID, quint8 patentQ)
{
	DgData& gd = g_Au.LockDataForWrite();
	gd.EmpireRecordPatent(empireId, (COMMODITYID)patentCID, patentQ);
	g_Au.UnlockData();
}

void AuApp::GovDossierAddDocument(quint32 empireId, quint32 dosEmpire, QByteArray packedDoc)
{
	AuDocument* doc = AuDocument::Unpack(packedDoc);
	if(doc)
	{
		DgData& gd = g_Au.LockDataForWrite();
		gd.EmpireDossierAddDocument(empireId, dosEmpire, *doc);
		g_Au.UnlockData();
		delete doc;
	}
}

void AuApp::GovSetCapitalUp(quint32 empire, AuId city, AuSpecies workerSpecies, AuId domain, int capitalType)
{
	DgData& gd = g_Au.LockDataForWrite();
	switch(capitalType)
	{
	case 0: // sector
		if(!domain.IsNull())
			gd.SetEmpireSectorCapital(empire, city, domain);
		break;
	case 1: // empire
		gd.SetEmpireCapital(empire, city, workerSpecies);
		break;
	case 3: // galaxy
		if(!domain.IsNull())
			gd.SetEmpireGalaxyCapital(empire, city, domain);
		break;
	}
	g_Au.UnlockData();
}

void AuApp::HengeActivated(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
		AuId destSysId, quint8 destSun, quint8 destArcSegment)
{
	AuEther& ether = g_Au.LockEtheriumDelete();
    DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(destSysId));
    if(system)
        system->HengeActivated(fromSysId, fromSun, fromShadowPanels, fromArcSegment, destSun, destArcSegment);
    g_Au.UnlockEtheriumDelete();
}

void AuApp::HengeCreated(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
		AuId destSysId, quint8 destSun, quint8 destArcSegment)
{
	AuEther& ether = g_Au.LockEtheriumDelete();
    DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(destSysId));
    if(system)
        system->HengeCreated(fromSysId, fromSun, fromShadowPanels, fromArcSegment, destSun, destArcSegment);
    g_Au.UnlockEtheriumDelete();
}

void AuApp::HengeDeactivated(AuId fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
		AuId destSysId, quint8 destSun, quint8 destArcSegment)
{
	AuEther& ether = g_Au.LockEtheriumDelete();
    DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(destSysId));
    if(system)
        system->HengeDeactivated(fromSysId, fromSun, fromShadowPanels, fromArcSegment, destSun, destArcSegment);
    g_Au.UnlockEtheriumDelete();
}

void AuApp::HengeDestinationConfirmed(AuId sectorId, AuId systemId, QString systemName, quint8 sun, QString worldCatName,
		QString worldName, quint8 worldType, AuGlobeLLF hengeLL, bool breathable, quint8 orbitZone, bool outsideTown)
{
	AuHengeDestination dest(sectorId, systemId, systemName, sun, worldCatName, worldName, worldType, hengeLL, breathable,
            (ORBITZONE)orbitZone, outsideTown);
    AuHengeDestinations& hd = g_Au.LockHengeDestinationsForWrite();
    hd.Update(dest);
    g_Au.UnlockHengeDestinations();
}

void AuApp::HengeTeleporterEntered(AuId teleportId, AuId fromSysId, quint8 fromSun, quint8 fromArc, quint8 shadowPanels,
		AuId destSysId, quint8 destSun, quint8 destArc)
{
	//qDebug() << "AuApp::HengeTeleporterEntered" << teleportId;
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaAtom* teleportAtom = ether.FindLiveAtom(teleportId);
	if(teleportAtom)
	{
		//qDebug() << "AuApp::HengeTeleporterEntered teleportAtom" << *teleportAtom;
		DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(destSysId));
		if(system)
		{
			//qDebug() << "AuApp::HengeTeleporterEntered toSystem" << *system << system->IsFinal();
			if(!system->IsFinal())
			{
				system->FinalizeContents();
				system->SetStarFieldFlag(false);
				system->TickTockRecursive();
			}
			if(teleportAtom->FindPersonOnLineRecursive())
				system->SetLastPersonVisited();
			if(teleportAtom->FindOfficerRecursive() || teleportAtom->FindAIPersonRecursive())
				system->SetLastOfficerVisited();
			system->HengeActivated(fromSysId, fromSun, shadowPanels, fromArc, destSun, destArc);

			teleportAtom->SetNormal();
			system->HengeTeleportArrived(teleportAtom, destSun, destArc, fromSysId, fromSun, fromArc);
			teleportAtom->TickTockRecursive();
			teleportAtom->SetInScene();
		}
	}
	g_Au.UnlockEtheriumDelete();
}

void AuApp::OfficeAssigned(AuId person, quint32 empire, AuOffice office)
{
	//qDebug() << "AuApp::OfficeAssigned person" << person << "emp" << empire;
	AuEther& ether = g_Au.LockEtheriumDelete();
    DaUniverse* universe = ether.FindUniverse();
    if(universe)
    {
		if(empire)
		{
			// Change the empire data.
			DgData& gd = g_Au.LockDataForWrite();
			gd.EmpireAssignOffice(empire, person, office);
			g_Au.UnlockData();
		}

		// Try to find the person that will receive the office position.
		DaPerson* client = dynamic_cast<DaPerson*>(ether.FindNormalAtom(person));
		if(client)  // This is the person that holds this office now
		{
			if(client->EmpireId() != empire)
			{
				// Player's assignment is in a different empire than their current one. This player is changing empires.
				client->WillfullyChangedEmpire(empire); // emits AddCitizenToEmpire message.
				client->SetModified();
			}
		}
    }
    g_Au.UnlockEtheriumDelete();
}

void AuApp::OfficerKilledUp(const AuId homePlanetId)
{
	// Check to see if I have the planet in question.
    AuEther& ether = g_Au.LockEtheriumDelete();
    DaWorld* homePlanet = dynamic_cast<DaWorld*>(ether.FindNormalAtom(homePlanetId));
    if(homePlanet)
    {
        // The home planet was found in my local data.
        DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(homePlanet);
        if(llworld)
            llworld->SetCapt(AuId(), QString(""));
        homePlanet->SetModified();
    }
    g_Au.UnlockEtheriumDelete();
}

void AuApp::OpenChannelTo(AuId fromUnitId, QString fromName, AuId fromSector, quint8 fromGalaxy, AuId toUnitId, QString channelName,
		quint8 scope, bool propagateUp)
{
	// Not needed in solo.
	Q_UNUSED(propagateUp);
	//qDebug() << "AuApp::OpenChannelTo" << toUnitId << channelName << "from" << fromUnitId << fromName;
	if(AvatarId() == toUnitId)
    {
        // Check my squelch list. Don't let squelched people open channels to me.
        //DHH if(m_SquelchIds.find(fromUnitId) == m_SquelchIds.cend())
        {
			if(!Frame()->IsCommChannelOpen(channelName))
            {
                // Announce who opened the channel, on that channel.
            	ChannelOpened(channelName, (AuCommStdChannel::SCOPE)scope, "");
				AuId personId;
				QString personName;
				m_I->Avatar(personId, personName);
                AuCommChannelOpened msg(personId, personName, channelName, fromUnitId, fromName, fromSector, (GALAXY)fromGalaxy,
                        AuCommStdChannel::Universe, (AuCommStdChannel::SCOPE)scope);
				AuEther& ether = g_Au.LockEtheriumDelete();
                SoloChatToClient(&msg, ether);
				g_Au.UnlockEtheriumDelete();
            }
        }
    }
}

void AuApp::AvatarOnLineUp(AuId personId, quint32 empireId)
{
	// Every server gets this message so they all update their own empire records.
	if(empireId)
	{
		DgData& gd = g_Au.LockDataForWrite();
		gd.EmpireCitizenOnLine(empireId);		// Empire doesn't care which citizen was on line.
		g_Au.UnlockData();
	}

	Q_UNUSED(personId);
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
			DaAtom* capitol = (DaAtom*)ether.FindNormalAtom(empireCapitol); // Cast away constness.
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

void AuApp::PostJobOffer(AuJobOffer jobOffer)
{
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

		g_Au.UnlockJobOffers();

		// Restart the timer for accepting the next job offer.
		m_JobOfferLast.restart();
	}
}

void AuApp::ReleaseBerthUp(AuId personId, AuId shipId)
{
	AuAtomInterface::ReleaseBerthDown(personId, shipId);
}

void AuApp::SectorPirateBaseEncounter(AuId encSectorId, AuId encSystemId)
{
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSector* sector = dynamic_cast<DaSector*>(ether.FindNormalAtom(encSectorId));
	if(sector)
	{
		AuId baseSystem = sector->PirateBaseEncounter(encSystemId);  // Returns id of system that will create a new pirate base.
		if(!baseSystem.IsNull())
		{
			DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(baseSystem));
			if(system)
				system->PirateBaseEncounter();
		}
	}
    g_Au.UnlockEtheriumDelete();
}

void AuApp::SendFundsUp(AuId fromId, QString fromName, AuId toId, double amount, quint8 fromGalaxy, quint8 reason, quint32 designId)
{
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
}

void AuApp::SetPoliticalStance(quint32 toEmpire, quint32 fromEmpire, quint8 fromStance, quint8 vassalTributePercent,
		AuId fromUnit, QString fromUnitName, quint8 toStance, AuId toUnit, QString toUnitName,
		quint8 reason, quint8 galaxy, QString sector, QString system, QString world)
{
	Q_UNUSED(toStance);
	// It is not valid to set the stance toward another empire to be "empire".
	if((POLITICAL_STANCE)fromStance == POL_Empire)
		return;
	if(toEmpire == fromEmpire)
		return;

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
		return;
	}

	DgEmpireHash::iterator iet = gd.m_gEmpires.find(toEmpire);
	if(iet == gd.m_gEmpires.end())
	{
		// To empire was not found.
		g_Au.UnlockData();
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
}

void AuApp::ShipIt(AuItemTEUContainer container)
{
	AuId systemId(container.DestinationSystem());
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(systemId));
	if(system)
	{
		DaAtom* recipient = system->FindIdRecursive(container.DestinationCity());
		if(recipient)
			recipient->ShipmentDeliver(container);          // Shipment finally reaches the destination here.
	}
	g_Au.UnlockEtheriumDelete();
}

void AuApp::SolarSystemExplored(const AuId systemId, quint32 empireId)
{
	AuEther& ether = g_Au.LockEtheriumDelete();
    DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(systemId));
    if(system)
        system->ExploredBy(empireId);
    g_Au.UnlockEtheriumDelete();
}

void AuApp::SolarSystemFinal(AuId systemId)
{
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(systemId));
	if(system)
		system->SetFinal(true);
	g_Au.UnlockEtheriumDelete();
}

void AuApp::SolarSystemReadyToDecay(AuId systemId)
{
	//qDebug() << "AuApp::SolarSystemReadyToDecay" << systemId;
	// First verify the system is still ready, that nobody showed up at the last instant.
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(systemId));
	if(system && system->IsReadyToDecay())
	{
		//qDebug() << "AuApp::SolarSystemReadyToDecay" << *system << "IsReadyToDecay, doing so now.";
		system->DecayContents();        // Delete everything in the system.
		//system->SetNameIncludePlanets("", true);
		system->SetFinal(false);
		system->SetModified(DaAtom::NOTIFY_Database);
	}
	g_Au.UnlockEtheriumDelete();
}

void AuApp::SolarSystemScanned(const AuId systemId, quint32 empireId, quint8 genHab, quint32 genSeed)
{
	AuEther& ether = g_Au.LockEtheriumDelete();
    DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(systemId));
    if(system && (!system->IsScannedBy(empireId) || system->GenHab() != genHab || system->GenSeed() != genSeed))
        system->ScannedBy(empireId, (HABITABILITY)genHab, genSeed);
    g_Au.UnlockEtheriumDelete();
}

void AuApp::SpacecraftNameChanged(AuId shipId, QString nuName)
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

void AuApp::SpacecraftPositionReport(AuId shipId, AuId systemId, AuStarMapSpacecraft pos)
{
	QMultiHash<AuId, AuStarMapScPos>& scpos = g_Au.LockScPositionReports();
	QMultiHash<AuId, AuStarMapScPos>::iterator si = scpos.begin();
	while(si != scpos.end())
	{
		if((*si).IsExpired() || (*si).Id() == shipId)
			si = scpos.erase(si);   // Remove the one I am updating and expired ones as we find them.
		else
			si++;
	}
	scpos.insert(systemId, AuStarMapScPos(shipId, pos));    // Add a new one.
	g_Au.UnlockScPositionReports();
}

void AuApp::SpawnAtom(AuId personId, quint32 empireId, AuId spawnCity, AuId spawnShip, AuId arenaExitParent, quint8 galaxy)
{
	//qDebug() << "AuApp::SpawnAtom" << personId;
	AuId capitolU, capitolG, randomCity;
	if(empireId)
	{
		const DgData& gd = g_Au.LockDataForRead();
		// Determine the capitol of the empire, if any. This could be null.
		gd.EmpireCapital(empireId, capitolU);
		capitolG = gd.EmpireGalaxyCapital(empireId, (GALAXY)galaxy);

		// Find a random city in case none of the predetermined spawn locations are available.
		QSet<AuId> exceptIds;
		exceptIds.insert(capitolU);
		exceptIds.insert(capitolG);
		exceptIds.insert(spawnCity);
		gd.RandomEmpireCity(empireId, randomCity, exceptIds);
		g_Au.UnlockData();
	}

	AuEther& ether = g_Au.LockEtheriumDelete();
	DaPerson* person = dynamic_cast<DaPerson*>(ether.FindLiveAtom(personId));
	if(person)
	{
		DaAtom* spawnAt = 0;
		if(!arenaExitParent.IsNull())
			spawnAt = ether.FindNormalAtom(arenaExitParent);
		if(!spawnAt && !spawnShip.IsNull())
			spawnAt = ether.FindNormalAtom(spawnShip);
		if(!spawnAt && !spawnCity.IsNull())
			spawnAt = ether.FindNormalAtom(spawnCity);
		if(!spawnAt && !capitolG.IsNull())
			spawnAt = ether.FindNormalAtom(capitolG);
		if(!spawnAt && !capitolU.IsNull())
			spawnAt = ether.FindNormalAtom(capitolU);
		if(!spawnAt && !randomCity.IsNull())
			spawnAt = ether.FindNormalAtom(randomCity);

		if(!spawnAt)
		{
			// Create a new homeworld.
			DaGalaxy* ag = ether.FindGalaxy((GALAXY)galaxy);
			if(ag)
			{
				// Create a cube of sectors.
				AuIdList idNewSectors;
				DaSector* sector = ag->CreateNewHomeWorldSector(8, idNewSectors);
				Q_ASSERT(sector);
				if(sector)
				{
					// Locate a solar system in that sector.
					DaSolarSystem* system = sector->FindGoodHome();
					if(system)
					{
						system->FinalizeContents();
						system->SetStarFieldFlag(false);
						system->TickTockRecursive();
						spawnAt = system->FindHabitableWorld(HABITABLE_HomeWorld);
					}
				}
			}
		}

		if(spawnAt)
		{
			DaSolarSystem* system = spawnAt->FindParentSolarSystem();
			if(system)
			{
				system->SetLastPersonVisited();
				person->ForcePos(AuAtomPos(system->Id(), AuMatrix4x4()));
				person->SetNormal();		// No longer traveling.
				person->SetSpawnLocal();
				person->TickTock();
			}
		}
	}
	g_Au.UnlockEtheriumDelete();
}

void AuApp::StargateClosed(AuId fromGalaxy, AuPoint fromGalaxyLoc)
{
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaAtomList systems;
	ether.ListNormalSolarSystems(systems);
	for(DaAtomList::iterator si = systems.begin(); si != systems.end(); si++)
	{
		DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(*si);
		if(system)
		{
			// This is used to close the destination negative stargate.
			// We create destination stargates at the primary star so that's where we remove them from.
			DaStar* star = system->FindPrimaryStarNotBlackHole();
			if(star)
				star->CloseStargate(fromGalaxy, fromGalaxyLoc);
		}
	}
	g_Au.UnlockEtheriumDelete();
}

void AuApp::StargateOpened(AuId fromGalaxy, AuPoint fromGalaxyLoc, AuId toGalaxy, AuPoint toGalaxyLoc, AuTime expire)
{
	// Insure the sector exists.
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaGalaxy* galaxy = dynamic_cast<DaGalaxy*>(ether.FindNormalAtom(toGalaxy));
	if(galaxy)
	{
		AuPoint sectorLoc(toGalaxyLoc);
		sectorLoc.Round(10);            // Round to an even sector location.
		AuIdList nuSectors;
		DaSector* sector = galaxy->CreateSectorCubeAt(sectorLoc, nuSectors);
		if(sector)
		{
			AuPoint systemLoc = toGalaxyLoc - sector->Location();
			DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(sector->FindSolarSystemAt(systemLoc));
			if(!system)
				system = dynamic_cast<DaSolarSystem*>(sector->FindClosestSolarSystem(systemLoc));
			if(system)
			{
				if(!system->IsFinal())
				{
					system->FinalizeContents();
					system->SetStarFieldFlag(false);
					system->TickTockRecursive();
				}

				// This is used to open the destination negative stargate.
                // We create destination stargates at the primary star.
                DaStar* star = system->FindPrimaryStarNotBlackHole();
                if(star)
                    star->OpenStargate(fromGalaxy, fromGalaxyLoc, false, expire);
			}
		}
	}
	g_Au.UnlockEtheriumDelete();
}

void AuApp::SystemWarpSignature(AuId systemId, quint32 warpFactors, quint8 galaxy, float galaxyLocX, float galaxyLocY,
		float galaxyLocZ)
{
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

void AuApp::TradeReadyToShip(AuId readyCapitolId, AuTradeAgreement ta)
{
	AuId systemId;
	if(readyCapitolId == ta.OriginCity())
		systemId = ta.DestinationSystem();
	else if(readyCapitolId == ta.DestinationCity())
		systemId = ta.OriginSystem();
	AuEther& ether = g_Au.LockEtheriumDelete();
    DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(systemId));
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

void AuApp::TradeShipped(AuId shipperCapitolId, AuTradeAgreement ta)
{
	AuId systemId;
	if(shipperCapitolId == ta.OriginCity())
		systemId = ta.DestinationSystem();
	else if(shipperCapitolId == ta.DestinationCity())
		systemId = ta.OriginSystem();
	AuEther& ether = g_Au.LockEtheriumDelete();
    DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(systemId));
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

void AuApp::WarpSignatureDetected(AuId systemId, quint32 signalStrength, quint32 detectingEmpireId)
{
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(systemId));
	if(system)
		system->DetectedBy(detectingEmpireId, signalStrength);
	g_Au.UnlockEtheriumDelete();
}

void AuApp::WormholeExplored(AuId systemId, AuId toSystem, qint8 polarity, quint32 empire, QString nuName)
{
	AuEther& ether = g_Au.LockEtheriumDelete();
    DaSolarSystem* pss = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(systemId));
    if(pss)
    {
        pss->WormholeExplored(toSystem, (AuWormhole::POLARITY)polarity, empire, nuName);
        pss->SetModified(DaAtom::NOTIFY_Scene);                 // No db update but scenes need update.
    }
	g_Au.UnlockEtheriumDelete();
}

void AuApp::SendMail(AuMailMsg::MSGTYPE msgType, const AuId& fromId, const QString& fromName, const AuId& toId,
		const QString& subject, const QString& bodyText) const
{
	Q_ASSERT(g_Au.IsSceneSolo());
	// Get this server's login credentials to send mail.
	const DgData& gd = g_Au.LockDataForRead();
	unsigned int acctId = gd.PlayerId();	// The local server must be "logged in" as someone to send mail.
	unsigned int confirmNum = gd.ConfirmationNumber();
	g_Au.UnlockData();

	// Send the mail message.
	g_Au.SendMail(acctId, confirmNum, fromId, fromName, toId, msgType, subject, bodyText, AuId(0), "", AuId(0), "", GALAXY_Count,
			AuPoint(0,0,0));
}

void AuApp::AcceptTrade(AuTradeAgreement ta)
{
	AuId idSystem(ta.OriginSystem());
	AuEther& ether = g_Au.LockEtheriumDelete();
    DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(idSystem));
	if(system)
	{
		DaBd* recipient = dynamic_cast<DaBd*>(system->FindIdRecursive(ta.OriginCity()));
		if(recipient)
			recipient->TradeAgreementAccept(ta, false);         // Destination acceptance finally reaches the origin here.
	}
	g_Au.UnlockEtheriumDelete();
}

void AuApp::AdminGoTo(AuId personId, AuId dest, quint8 rideAlso, bool limboRescue, AuId returnOnFail)
{
	Q_UNUSED(limboRescue);
	Q_UNUSED(returnOnFail);
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaPerson* person = dynamic_cast<DaPerson*>(ether.FindNormalAtom(personId));
	if(person)
	{
		DaAtom* moveAtom = person;
		if(rideAlso && (dynamic_cast<DaCreature*>(person->Parent()) ||
					dynamic_cast<DaVehicle*>(person->Parent()) ||
					dynamic_cast<DaVessel*>(person->Parent())))
			moveAtom = moveAtom->Parent();

		DaAtom* destAtom = ether.FindNormalAtom(dest);
		if(destAtom)
		{
			DaSolarSystem* dsys = destAtom->FindParentSolarSystemInEnv();	// Will be destAtom if it is a system.
			dsys->AdminGoTo(moveAtom, destAtom);
		}
		else
			qDebug() << "error: AuApp::AdminGoTo dest atom not found" << dest;
	}
	g_Au.UnlockEtheriumDelete();
}

// This is where the trail begins when an agent teleports to a galaxy location or when someone enters a one way intergalactic
// wormhole.
// This is not where spawning happens.
// If the origin and destination solar systems are in my locally managed data, the transfer happens immediately right here.
void AuApp::AtomTraveledGalaxyLoc(AuId traveler, AuId origSystem, AuId destGalaxyId, AuPoint destGalaxyLoc)
{
	if(g_Au.m_LogMessages & LOG_Travel)
		qDebug() << "AuApp::AtomTraveledGalaxyLoc traveler" << traveler << "origSystem" << origSystem << "destGalaxy"
			<< destGalaxyId << "destGalaxyLoc" << destGalaxyLoc;
	// Insure the originating system hasn't been revoked from me already.
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaAtom* trav = ether.FindLiveAtom(traveler);
	if(!trav)
	{
		if(g_Au.m_LogMessages & LOG_Error)
			qWarning() << "AuApp::AtomTraveledGalaxyLoc traveler" << traveler << "atom not found"
				<< "destGalaxyId" << destGalaxyId << "destGalaxyLoc" << destGalaxyLoc;
		g_Au.UnlockEtheriumDelete();
		return;
	}
	if(!g_Au.IsSceneSolo() && trav->IsTraveling())
	{
		// In solo client our traveler has already been set to traveling.
		if(g_Au.m_LogMessages & LOG_Error)
			qWarning() << "AuApp::AtomTraveledGalaxyLoc traveler" << *trav << "is already traveling"
				<< "destGalaxyId" << destGalaxyId << "destGalaxyLoc" << destGalaxyLoc;
		g_Au.UnlockEtheriumDelete();
		return;
	}

	// See if we can identify the destination system locally.
	DaGalaxy* dGalaxy = dynamic_cast<DaGalaxy*>(ether.FindNormalAtom(destGalaxyId));
	if(!dGalaxy)
	{
		if(g_Au.m_LogMessages & LOG_Error)
			qWarning() << "AuApp::AtomTraveledGalaxyLoc traveler" << *trav << "origSystem" << origSystem
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
		destSectorLoc.Round(10);            // Round to an even sector location.
		AuIdList nuSectors;
		DaSector* sector = dGalaxy->CreateSectorCubeAt(destSectorLoc, nuSectors);
		if(sector)
		{
			destSystemLoc = destGalaxyLoc - destSectorLoc;	// Location relative to the sector.
			destSys = sector->FindClosestSolarSystem(destSystemLoc);
			if(destSys)
			{
				if(!destSys->IsFinal())
				{
					destSys->FinalizeContents();
					destSys->SetStarFieldFlag(false);
					destSys->TickTockRecursive();
				}

				destSystem = destSys->Id();
			}
		}
	}

	// I can perform the transfer right here and now.
	if(destSys)
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
	}
	g_Au.UnlockEtheriumDelete();
}

void AuApp::BdDesignExchangePublished(AuBdDesignExchangeEntry item)
{
	DgData& gd = g_Au.LockBdExchangeForWrite();
	gd.m_BdExchange->insert(item.DesignId(), item);
	g_Au.UnlockBdExchange();
}

void AuApp::BdDesignExchangeUnpublished(quint32 designId)
{
	DgData& gd = g_Au.LockBdExchangeForWrite();
	gd.m_BdExchange->remove(designId);
	g_Au.UnlockBdExchange();
}

void AuApp::BlueprintRatedBd(AuExchangeRatingRec rating)
{
	DgData& sdi = g_Au.LockBdExchangeForWrite();
	sdi.m_BdExchangeRatings->AddReview(rating);
	g_Au.UnlockBdExchange();
}

void AuApp::BlueprintRatedSc(AuExchangeRatingRec rating)
{
	DgData& sdi = g_Au.LockScExchangeForWrite();
	sdi.m_ScExchangeRatings->AddReview(rating);
	g_Au.UnlockScExchange();
}

void AuApp::CityMailRequestedSignal(AuId cityId, AuId unitId, quint32 unitEmpire)
{
	AuEther& ether = g_Au.LockEtheriumDelete();
	const DaAtom* atom = ether.FindNormalAtom(cityId);
	const DaBd* bd = dynamic_cast<const DaBd*>(atom);
	if(bd && bd->EmpireId() == unitEmpire)
		bd->MailLastReport(unitId);
	g_Au.UnlockEtheriumDelete();
}

void AuApp::CollectBounty(AuId bountyHunter, QString bountyHunterName, AuGearPos gearPos, AuBountyOnItem bountyOnItem)
{
	// Locate the victim to take the bounty from them. Check to see if they are in my local data.
    AuEther& ether = g_Au.LockEtheriumDelete();
    DaPerson* person = dynamic_cast<DaPerson*>(ether.FindNormalAtom(bountyHunter));
    DaUnit* unit = dynamic_cast<DaUnit*>(ether.FindNormalAtom(bountyOnItem.Id()));
    if(person && unit)
    {
        // The victim unit was found. Do they have any bounties?
        if(unit->TakeItemBounty(bountyHunterName, bountyOnItem))    // Take the bounty.
            person->BountyCollected(gearPos, bountyOnItem);         // Give it to the bounty hunter.
        else
            person->BountyExpired(gearPos, bountyOnItem);           // Invalidate the bounty on the item.
    }
    g_Au.UnlockEtheriumDelete();
}

void AuApp::DeactivateTrade(AuTradeAgreement ta, quint8 nuState, bool atDest)
{
	AuId idSystem;
	if(atDest)
		idSystem = ta.DestinationSystem();
	else
		idSystem = ta.OriginSystem();
	AuEther& ether = g_Au.LockEtheriumDelete();
    DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(idSystem));
	if(system)
	{
		DaBd* recipient;
		if(atDest)
			recipient = dynamic_cast<DaBd*>(system->FindIdRecursive(ta.DestinationCity()));
		else
			recipient = dynamic_cast<DaBd*>(system->FindIdRecursive(ta.OriginCity()));
		if(recipient)
			recipient->TradeAgreementDeactivate(ta, (AuTradeAgreement::STATE)nuState);  // Deactivate with inactive status.
	}
	g_Au.UnlockEtheriumDelete();
}

// 'industry' is an INDUSTRYID
void AuApp::EmpireChgBdPref(quint32 empireId, int industry, quint32 designId)
{
	AuBlueprintPref bp(designId, BENV_All); //DHH so far all pirate building designs work in all environments.
	AuBdPref pref;
	pref.AddBlueprint(bp);
	DgData& gd = g_Au.LockDataForWrite();
	gd.SetEmpirePreferredBlueprints(empireId, (INDUSTRYID)industry, pref);
	g_Au.UnlockData();
}

void AuApp::EmpireChgFlag(quint32 empireId, QImage flag)
{
	DgData& gd = g_Au.LockDataForWrite();
	gd.SetEmpireFlag(empireId, flag);
	g_Au.UnlockData();
}

// 'scrole' is an AuScPrefs::SCROLE
void AuApp::EmpireChgScPref(quint32 empireId, int scrole, quint32 designId)
{
	AuScPref pref;
	pref.AddBlueprint(designId);
	DgData& gd = g_Au.LockDataForWrite();
	gd.SetEmpirePreferredBlueprints(empireId, (AuScPrefs::SCROLE)scrole, pref);
	g_Au.UnlockData();
}

void AuApp::EstablishNewEmpire(AuId personId, QString personName, quint8 personGender, quint32 idOldEmpire, QString name,
		QImage imFlag)
{
	//qDebug() << "AuApp::EstablishNewEmpire" << personId << personName << "oldEmp" << idOldEmpire << "nuEmp" << name;

	// Insure the empire name is unique.
	if(!IsNameValid(name))
		return;                     // The name is not valid.

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
	emp.m_id = g_Au.NextEmpireId(); // Get a nu id.
	emp.m_flag = imFlag;
	//qDebug() << "AuSceneServerApp::EstablishNewEmpire, adding nu Empire" << emp.Name() << emp.m_id;
	gd.m_gEmpires.insert(pair<quint32, DgEmpire>(emp.m_id, emp));

	// Add Citizen to nu empire
	gd.AddCitizenToEmpire(idOldEmpire, emp.m_id, personId, personName, personGender);
	g_Au.UnlockData();

	// Assign office
	AuOffice office;
	office.SetupEmperor();
	OfficeAssigned(personId, emp.m_id, office);
}

void AuApp::GovBossChanged(quint32 empireId, AuId governorId, AuId bossId)
{
	DgData& gd = g_Au.LockDataForWrite();
	gd.EmpireChangeBoss(empireId, governorId, bossId);
	g_Au.UnlockData();
}

// 'docTime' and 'docType' together are used to identify the exact document.
void AuApp::GovDossierChangeState(quint32 empire, quint32 dosEmpire, AuTime docTime, quint8 docType, quint16 docIndex,
		quint8 docState)
{
	DgData& gd = g_Au.LockDataForWrite();
	gd.SetEmpireDossierChangeState(empire, dosEmpire, docTime, docType, docIndex, (AuDocument::DOC_STATE)docState);
	g_Au.UnlockData();
}

void AuApp::GovSetAnnouncements(quint32 empire, QString announcements)
{
	DgData& gd = g_Au.LockDataForWrite();
	gd.SetEmpireAnnouncements(empire, announcements);
	g_Au.UnlockData();
}

void AuApp::GovSetCommerce(quint32 empireId, QByteArray commerce)
{
	DgData& gd = g_Au.LockDataForWrite();
	gd.SetEmpireCommerce(empireId, commerce);
	g_Au.UnlockData();
}

void AuApp::GovSetContrailColor(quint32 empireId, quint8 hue, quint8 sat)
{
	DgData& gd = g_Au.LockDataForWrite();
	gd.SetEmpireContrailColor(empireId, hue, sat);
	g_Au.UnlockData();
}

void AuApp::GovSetDefaultPoliticalStance(quint32 empire, quint8 stance)
{
	if(stance == POL_Empire)
		return;
	DgData& gd = g_Au.LockDataForWrite();
	gd.SetEmpireDefaultPoliticalStance(empire, (POLITICAL_STANCE)stance);
	g_Au.UnlockData();
}

void AuApp::GovSetTax(quint32 empire, quint8 income, quint8 sales, quint8 subsidy)
{
	DgData& gd = g_Au.LockDataForWrite();
	gd.SetEmpireTaxes(empire, income, sales, subsidy);
	g_Au.UnlockData();
}

void AuApp::GovSetThemeColor(quint32 empire, quint8 theme, QColor color)
{
	DgData& gd = g_Au.LockDataForWrite();
	gd.SetEmpireThemeColor(empire, (AuScFace::COLOR_THEME)theme, color);
	g_Au.UnlockData();
}

void AuApp::GovSetWarCriteria(quint32 empire, AuWarCriteria crit)
{
	DgData& gd = g_Au.LockDataForWrite();
	gd.SetEmpireWarCriteria(empire, crit);
	g_Au.UnlockData();
}

void AuApp::LeaveEmpire(AuId personId, QString personName, quint32 idOldEmpire)
{
	Q_UNUSED(personName);
	if(idOldEmpire)
	{
		DgData& gd = g_Au.LockDataForWrite();
		gd.RemoveCitizenFromEmpire(idOldEmpire, personId);
		g_Au.UnlockData();
	}
}

void AuApp::NameSpecies(AuSpecies species, QString speciesName, AuId discoverer)
{
	DgData& gd = g_Au.LockDataForWrite();
	gd.NameSpecies(species, speciesName, discoverer);
	g_Au.UnlockData();
}

void AuApp::OfficeRevoked(AuId person, quint32 empire)
{
	if(!empire)
		return;
	DgData& gd = g_Au.LockDataForWrite();
	gd.EmpireRevokeOffice(empire, person);
	g_Au.UnlockData();
}

void AuApp::ProposeTrade(AuTradeAgreement ta)
{
	int result = 1;
	AuId idSystem(ta.DestinationSystem());
	AuEther& ether = g_Au.LockEtheriumDelete();
    DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(idSystem));
	if(system)
	{
		DaBd* recipient = dynamic_cast<DaBd*>(system->FindIdRecursive(ta.DestinationCity()));
		if(recipient)
			result = recipient->TradeAgreementDeliver(ta);              // Proposal finally reaches the destination here.
	}
	g_Au.UnlockEtheriumDelete();
	if(result)
		DeactivateTrade(ta, AuTradeAgreement::Undeliverable, false);    // Deactivate the proposal at its origin.
}

void AuApp::RenameAvatar(AuId avatarId, QString nuName)
{
	DgData& gd = g_Au.LockDataForWrite();
	gd.SetEmpireAvatarName(avatarId, nuName);
	gd.SetClientUnitName(avatarId, nuName);
	g_Au.UnlockData();
}

void AuApp::RenameCity(AuId cityId, QString nuName)
{
	DgData& gd = g_Au.LockDataForWrite();
	gd.SetEmpireCityName(cityId, nuName);
	g_Au.UnlockData();
}

void AuApp::RenamePlanet(AuId planetId, QString nuName, AuId systemId, QString systemName, AuId sectorId, QString sectorName)
{
	DgData& gd = g_Au.LockDataForWrite();
	gd.SetEmpirePlanetName(planetId, nuName);
	g_Au.UnlockData();

	AuEther& ether = g_Au.LockEtheriumDelete();
	DaWorld* planet = dynamic_cast<DaWorld*>(ether.FindNormalAtom(planetId));
	BODY_TYPE bodyType = BODY_TYPE_Count;
	if(planet)
	{
		planet->SetName(nuName);
		bodyType = planet->BodyType();
	}

	// Locate officer(s) from that planet.
	DaAtomList npcs;
	ether.FindNpcsFromWorld(planetId, npcs);
	for(DaAtomList::const_iterator oi = npcs.begin(); oi != npcs.end(); oi++)
	{
		DaNpc* npc = (DaNpc*)*oi;
		npc->SetHomeWorld(planetId, nuName, bodyType, systemId, systemName, sectorId, sectorName, npc->HomeGalaxy());
		npc->SetModified();
	}
	g_Au.UnlockEtheriumDelete();
}

void AuApp::RenameSector(AuId sectorId, QString nuName)
{
	QList<quint32> empireIds;
	DgData& gd = g_Au.LockDataForWrite();
	gd.SetEmpireSectorName(sectorId, nuName, empireIds);
	g_Au.UnlockData();

	AuEther& ether = g_Au.LockEtheriumDelete();
	DaAtom* sector = ether.FindNormalAtom(sectorId);
	if(sector)
	{
		sector->SetName(nuName);
		sector->SetModified();
	}
	else
		qWarning() << "AuApp::RenameSector sector not found" << sectorId;

	DaAtomList npcs;
	ether.FindNpcsFromSector(sectorId, npcs);
	for(DaAtomList::const_iterator oi = npcs.begin(); oi != npcs.end(); oi++)
	{
		DaNpc* npc = (DaNpc*)*oi;
		if(npc->SetHomeSectorName(nuName))
			npc->SetModified();
	}
	g_Au.UnlockEtheriumDelete();
}

void AuApp::RenameSystem(AuId systemId, QString nuName)
{
	DgData& gd = g_Au.LockDataForWrite();
	gd.SetEmpireSystemName(systemId, nuName);
	g_Au.UnlockData();

	AuEther& ether = g_Au.LockEtheriumDelete();
    DaSolarSystem* system = dynamic_cast<DaSolarSystem*>(ether.FindNormalAtom(systemId));
    if(system)
        system->SetNameIncludePlanets(nuName, false);       // Db not notified, was done when rename started in DtChatChannel.

    DaAtomList people;
    ether.FindPeopleFromSystem(systemId, people);
    for(DaAtomList::const_iterator pi = people.begin(); pi != people.end(); pi++)
    {
        DaPerson* p = (DaPerson*)*pi;
        if(p->SetSpawnCitySystemName(nuName))
            p->SetModified();
    }
    g_Au.UnlockEtheriumDelete();
}

void AuApp::ScDesignExchangePublished(AuScDesignExchangeEntry item)
{
	DgData& gd = g_Au.LockScExchangeForWrite();
	gd.m_ScExchange->insert(item.DesignId(), item);
	g_Au.UnlockScExchange();
}

void AuApp::ScDesignExchangeUnpublished(quint32 designId)
{
	DgData& gd = g_Au.LockScExchangeForWrite();
	gd.m_ScExchange->remove(designId);
	g_Au.UnlockScExchange();
}

void AuApp::SpacecraftStasisRecover(AuId shipId, AuId personId, quint8 personGalaxy, bool agentRecover)
{
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
			mymsg.SetScopeData(shipId.GetID()); // For CrewMates scope.
			g_Au.SendAtomChat(mymsg);
		}
		else
		{
			// Notify the sender that the ship is not ready to be recovered..
			AuCommShipStasisWait mymsg(stasisTime, AuCommStdChannel::CrewChannelName(), shipId, ship->Name(),
					ship->FindParentSector()->Id(), stasisGalaxy, AuCommStdChannel::Universe,
					AuCommStdChannel::CrewMates);
			mymsg.SetScopeData(shipId.GetID()); // For CrewMates scope.
			g_Au.SendAtomChat(mymsg);
		}
    }
    g_Au.UnlockEtheriumDelete();

	for(unordered_map<AuId, AuId, AuIdHash>::const_iterator sdi = recoverShips.cbegin(); sdi != recoverShips.cend(); sdi++)
        SummonDormantShip((*sdi).first, (*sdi).second, (GALAXY)personGalaxy);
}

void AuApp::SummonDormantShip(const AuId& shipId, const AuId& crewId, GALAXY toGalaxy)
{
	if(shipId.IsNull())
		return;
	if(crewId.IsNull())
		return;

	// Attempt to perform an AdminGoTo locally.
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaVessel* moveShip = dynamic_cast<DaVessel*>(ether.FindDormantAtom(shipId));
	if(moveShip)
	{
		// If the destination atom is in my local data, I can perform the move right here.
		DaAtom* destAtom = ether.FindNormalAtom(crewId);
		if(destAtom)
		{
			// The destination atom is in my local data.
			GALAXY moveGalaxy = moveShip->Galaxy();
			if(toGalaxy == GALAXY_Count || moveGalaxy == GALAXY_Count)
			{
				DaSolarSystem* system = destAtom->FindParentSolarSystem();
				if(system)
				{
					// The destination atom's solar system is not a star field only system. Perform the move.
					if(!system->IsFinal())
					{
						system->FinalizeContents();
						system->SetStarFieldFlag(false);
						system->TickTockRecursive();
					}
					system->AdminGoTo(moveShip, destAtom);
					moveShip->Reactivate();
				}
			}
		}
	}
	g_Au.UnlockEtheriumDelete();
}

void AuApp::ArenaReady(AuArenaReady arena)
{
	arena.m_When.Set();
	unordered_map<AuId, AuArenaReady, AuIdHash>& arenas = g_Au.LockArenasForWrite();
	unordered_map<AuId, AuArenaReady, AuIdHash>::iterator ui = arenas.find(arena.m_Id);
	if(ui == arenas.end())
		arenas.insert(pair<AuId, AuArenaReady>(arena.m_Id, arena));
	else
		(*ui).second = arena;
	g_Au.UnlockArenas();
}

void AuApp::EnteredDesigner(AuId personId, int designMode)
{
	Q_UNUSED(personId);
	Q_UNUSED(designMode);
	// Not needed in solo.
}

void AuApp::GameOver(AuId shipId, quint16 whichTable, quint8 gameType, QByteArray gameResult)
{
	if(!IsSingleUserMode())
		return;
	AuFrame* pFrame = Frame();
	if(!pFrame)
		return;
	bool notifyGameOver = false;
    DaPerson* person = m_I->LockAvatar();
    if(person && person->IsGameTablePose() && person->ParentId() == shipId && person->StationId() == whichTable)
        notifyGameOver = true;  // My guy is at the game table in question.
    m_I->UnlockAvatar();
    if(notifyGameOver)
		pFrame->GameOver(gameType, gameResult);
}

void AuApp::GameTurn(AuId shipId, quint16 whichTable, AuId personsTurn)
{
	if(!IsSingleUserMode())
		return;
	AuFrame* pFrame = Frame();
	if(!pFrame)
		return;
	bool notifyGameTurn = false;
	DaPerson* person = m_I->LockAvatar();
	if(person && person->IsGameTablePose() && person->ParentId() == shipId && person->StationId() == whichTable)
		notifyGameTurn = true;	// My guy is at the game table in question.
	m_I->UnlockAvatar();
	if(notifyGameTurn)
		pFrame->GameTurn(personsTurn);
}

// Cobbled from AuSceneServerApp::AtomChat
void AuApp::SoloAtomChat()
{
	if(!IsSingleUserMode())
		return;
	// Get a copy of the array of channels we are supposed to open.
    // This mutex controls both the open channels queue and the chat messages queue.
    g_Au.LockAtomChat();
    QList<AuOpenChannel> channelsToOpen(g_Au.m_OpenChannels);
    g_Au.m_OpenChannels.clear();
    g_Au.UnlockAtomChat();

    // Open the channels that my local atoms want to open.
    for(QList<AuOpenChannel>::const_iterator it = channelsToOpen.cbegin(); it != channelsToOpen.cend(); it++)
    {
        // First notify server that the local atom is monitoring a channel.
        g_Au.MonitorChannel((*it).m_ChannelName, (*it).m_idFrom);

        // Now notify the target of the opened channel.
        OpenChannelTo((*it).m_idFrom, (*it).m_FromName, (*it).m_FromSector, (*it).m_FromGalaxy, (*it).m_idTo, (*it).m_ChannelName,
                (*it).m_Scope, (*it).m_ConnectBeyondSystem);
    }

	// Post chat messages generated in the Tick/Tock/TickMovement cycles and ProcessControlInput.
	QList<AuCommPacket*> msgs;	// Clones so we can release the chat queue before calling SoloChat to deliver them.
	QList<AuAtomChat>* chatQueue = g_Au.LockAtomChat();
	QList<AuAtomChat>::iterator ci = chatQueue->begin();
	while(ci != chatQueue->end())
	{
		if((*ci).HasCommPacket())
		{
			AuCommPacket* comm = (*ci).TakeCommPacket();
			msgs.append(comm);
		}
		ci = chatQueue->erase(ci);
	}
	g_Au.UnlockAtomChat();

	// Send the messages to local atoms, including the player.
	while(!msgs.isEmpty())
	{
		AuCommPacket* comm = msgs.takeFirst();
		SoloChat(*comm);
		delete comm;
	}

	// Decay atom chat messages that have gotten too old. This allows local atoms to store a short term memory of messages they
    // have sent.
    g_Au.DecayAtomChat();

	// Decay active bids that have gotten too old. This cleans the bid queue of old data.
	g_Au.DecayActiveBids();
}

// Cobbled from AuSceneServerApp::Chat
void AuApp::SoloChat(const AuCommPacket& msg)
{
	if(!IsSingleUserMode())
		return;
	if(msg.Type() != AuCommPacket::Detonate)
	{
		// Send the message to the player, like when the server app emits ChatToClients.
		AuCommPacket* mc = msg.Clone();
		if(mc)
		{
			AuEther& ether = g_Au.LockEtheriumDelete();
			SoloChatToClient(mc, ether);
			g_Au.UnlockEtheriumDelete();
			delete mc;
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

	case AuCommPacket::Chat:
		try
		{
			AuEther& ether = g_Au.LockEtheriumDelete();
			const AuCommChat& msgChat = dynamic_cast<const AuCommChat&>(msg);
			if(AuCommStdChannel::IsVoiceChannel(msgChat.ChannelName()) && msgChat.Text().toUpper().left(5) == "HELLO")
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
	bool berthAccepted = false;
	AuId personId;
	AuId nuShipId;
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
		case AuCommPacket::Accept:  // Bid accepted, perform the transaction.
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

        case AuCommPacket::AcceptBarter:    // Bid accepted, perform the transaction.
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
				CommAcceptTeleport(fromPerson, msg, ether);
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
						berthAccepted = true;
						personId = msgAccept.UnitId();
						nuShipId = fromShip->Id();
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
						berthAccepted = true;
						personId = msgAccept.UnitId();
						nuShipId = fromShip->Id();
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
						berthAccepted = true;
						personId = msgAccept.UnitId();
						if(!fromShip)
							fromShip = pfrom->FindParentVesselInEnv();
						if(fromShip)
							nuShipId = fromShip->Id();
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
						berthAccepted = true;
						personId = msgAccept.UnitId();
						if(!fromShip)
							fromShip = pfrom->FindParentVesselInEnv();
						if(fromShip)
							nuShipId = fromShip->Id();
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
						berthAccepted = true;
						personId = msgAccept.UnitId();
						if(!fromShip)
							fromShip = pfrom->FindParentVesselInEnv();
						if(fromShip)
							nuShipId = fromShip->Id();
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
						berthAccepted = true;
						personId = msgAccept.UnitId();
						if(!fromShip)
							fromShip = pfrom->FindParentVesselInEnv();
						if(fromShip)
							nuShipId = fromShip->Id();
					}
				}
				catch(const bad_cast&)
				{
					// dynamic_cast failed
				}
			}
			break;

		case AuCommPacket::AcceptAvatar:
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
		case AuCommPacket::CitySurrenderAccepted:
		case AuCommPacket::CitySurrenderOffered:
		case AuCommPacket::Combine:
		case AuCommPacket::ContactReply:
		case AuCommPacket::ContactShip:
		case AuCommPacket::CrewBoarded:
		case AuCommPacket::CrewBoardingComplete:
		case AuCommPacket::CryoBoarded:
		case AuCommPacket::CryoBoardingComplete:
		case AuCommPacket::CryoDebarked:
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
		case AuCommPacket::PassengersBoarded:
		case AuCommPacket::PassengersDebarked:
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
		case AuCommPacket::TroopsBoarded:
		case AuCommPacket::CommTypes:
			break;
		}
	}

	// Process the message by local non-person atoms monitoring the channel regardless of range.
	// Ships need to be able to receive mission orders from beyond the solar system.
	// Person atoms were handled earlier when sending to all open tcp channels, which are the currently connected people.
	// Search for local non-person message recipients that might be present in our data.
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
			DaAtom* system = fromPerson->FindEnvironmentAtom();
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
	else if(msg.Type() == AuCommPacket::Detonate)
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
			for(DaAtomList::iterator ai = atoms.begin(); ai != atoms.end(); ai++)
			{
				DaItem* item = dynamic_cast<DaItem*>(*ai);
				if(!item)
					continue;
				if(!item->IsNormal())
					continue;	// ListNormalSubatomsRecursive only lists normal atoms but those atoms could be killed in this loop.
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
		int channelCount = (int)commChannels.size();
		g_Au.UnlockAtomCommChannels();

		// 'countSupplied' is used when loading and unloading crew and passengers so multiple cities can take a crack at
		// taking/supplying some citizens until the request is filled.
		quint32 countSupplied = 0;
		for(int channelIndex=0; channelIndex < channelCount; channelIndex++)
		{
			AuId channelAtomId;	// Atom listening on the channel.
			unordered_multimap<QString, AuCommChannelConnection, AuStringHash>& commChannels = g_Au.LockAtomCommChannels();
			channelCount = (int)commChannels.size();		// Update channel count.
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
				continue;   // Don't receive my own messages.

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
			switch(msg.Scope())
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

			case AuCommStdChannel::Company:
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
							continue;   // Range to atom test failed.
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
	if(fromPerson)
		fromPerson->PoseMouthForSaying(msg, 0);
	g_Au.UnlockEtheriumDelete();

	if(berthAccepted)
		AuAtomInterface::ReleaseBerthDown(personId, nuShipId);
}

// Cobbled from DtChatChannel::ChatToClient
void AuApp::SoloChatToClient(AuCommPacket* msg, AuEther& ether)
{
	if(!IsSingleUserMode())
		return;
	AuId avatarId(AvatarId());
	const DgData& gd = g_Au.LockDataForRead();
	ACCOUNT accountType = gd.ClientAccountType();
	g_Au.UnlockData();

	if(msg->Text() == "")
		return;	// Could be AuCommContactShip message, which is not visible to players.

	// Skip this client if the message has a specific recipient and this client is not the recipient or the sender.
	AuId toId = msg->ToId();									// Some messages are addressed to a specific recipient.
	if(!toId.IsNull() && toId != avatarId && msg->FromId() != avatarId)
		return;

	/*DHH solo
	// Check to see if the sender is in the recipient's squelch list.
	if(m_SquelchIds.find(msg->FromId()) != m_SquelchIds.cend())
		return;
		*/

	AuCommStdChannel::RANGE senderActualRange = msg->TransmissionRange();
	const AuCommStdChannel::SCOPE scope = msg->Scope();
	if(!msg->IsThought())
	{
		// Determine the actual range of the sender from me.
		// Log messages always appear to come from close range, which is the hard-coded range for the Log message type.
		// Pick up info about the sender while we're at it, for scope test.
		quint32 empireId = 0;
		bool aboardSame = false;
		bool crewOk = false;
		bool diplomatOk = false;
		bool empireOk = false;
		bool fleetOk = false;
		bool companyOk = false;
		bool friendOk = false;
		bool inStory = false;

		DaPerson* pto = dynamic_cast<DaPerson*>(ether.FindNormalAtom(avatarId));
		DaSolarSystem* system = pto ? pto->FindParentSolarSystem() : 0;
		if(pto && system)
		{
			// Found ME!
			empireId = pto->EmpireId();

			// Check for crew membership.
			crewOk = pto->SpawnShip() == msg->FromShipId();

			// Check for common empire.
			empireOk = pto->EmpireId() == msg->FromEmpireId();

			// Check for recipient being a diplomat.
			quint32 toEmpireId = msg->ToEmpireId();
			if(toEmpireId)
			{
				// City surrender messages are targeted at a particular empire on the diplomacy channel.
				if(empireId == toEmpireId || empireId == msg->FromEmpireId())
				{
					const DgData& gd = g_Au.LockDataForRead();
					diplomatOk = gd.EmpireCitizenCanAcceptCitySurrender(empireId, pto->Id());
					g_Au.UnlockData();
				}
			}
			else
			{
				const DgData& gd = g_Au.LockDataForRead();
				diplomatOk = gd.IsEmpireGovernor(empireId, pto->Id());
				g_Au.UnlockData();
			}

			// Check for recipient being eligible for fleet messages.
			// Fleet scope requires the recipient to be a spacecraft officer that is in the same empire as the sender.
			// The recipient may also be a government offical higher than the rank of mayor.
			if(pto->EmpireId() == msg->FromEmpireId())
			{
				if(AuRosterEntry::IsOfficer(pto->SpawnShipBerthType()))
					fleetOk = true;
				else
				{
					const DgData& gd = g_Au.LockDataForRead();
					fleetOk = gd.EmpireCitizenCanReceiveFleetMessages(empireId, pto->Id());
					g_Au.UnlockData();
				}
			}

			// Check for recipient being eligible for company messages.
			if(pto->Is(msg->FromOwnerId()))
				companyOk = true;			// Recipient is the owner.
			else if(pto->SpawnShipIsPrivate() && AuRosterEntry::IsOfficer(pto->SpawnShipBerthType()))
			{
				// Officer of a private ship. We must find the ship in the system.
				const DaAtom* envAtom = pto->FindEnvironmentAtom();
				const DaVessel* ship = dynamic_cast<const DaVessel*>(envAtom->FindIdRecursive(pto->SpawnShip()));
				if(ship && ship->OwnerId() == msg->FromOwnerId())
					companyOk = true;
			}

			// Check for recipent being eligible for friend messages.
			POLITICAL_STANCE stanceTo = PoliticalStance(msg->FromEmpireId(), pto->EmpireId());
			POLITICAL_STANCE stanceFrom = PoliticalStance(pto->EmpireId(), msg->FromEmpireId());
			if((stanceTo == POL_Friend || stanceTo == POL_Empire || stanceTo == POL_Vassal) &&
					(stanceFrom == POL_Friend || stanceFrom == POL_Empire || stanceFrom == POL_Vassal))
				friendOk = true;

			// Check to see if the person is in the story.
			// Everyone is considered to be in story instance 1. It is used for conversations with AI citizens. Those messages
			// have a ToId so the recipient is filtered on that.
			inStory = pto->IsInStoryInstance(msg->StoryInstance()) || msg->StoryInstance() == 1; 

			const DaSector* mySector = pto->FindParentSector();
			if(mySector)
			{
				// Found my sector.
				if(msg->FromId() == avatarId)
				{
					// Sender always receives their own messages at close range.
					senderActualRange = AuCommStdChannel::Close;
					if(pto->IsAboardVehicle() || pto->IsInsideSpacecraft())
						aboardSame = true;					// So intercom channel will work.
				}
				else if(mySector->Is(msg->FromSectorId()))
				{
					// The message originated in my sector.
					const DaAtom* pfrom = system->FindIdRecursive(msg->FromId());
					if(pfrom)
					{
						// Found the sender of the message in my solar system.
						bool calcActualDistance = false;
						if(dynamic_cast<const DaArena*>(pfrom))
						{
							// Message is from an arena. They don't communicate outside of themselves.
							// Client does not receive this message if they are not in the arena.
							if(pto->FindParentArena() == pfrom)
								senderActualRange = AuCommStdChannel::Hail;		// Always at hail range from the arena itself.
							else
								senderActualRange = AuCommStdChannel::InvalidRange;	// Not in the arena.
						}
						else
						{
							// Sender is not a city so it must be a unit. No other atoms send chat.
							// Sender has queried their environment.
							if(pto->IsAboardVehicle() && pfrom->IsAboardVehicle() && pto->Parent() == pfrom->Parent())
							{
								// Aboard same vehicle as sender.
								senderActualRange = AuCommStdChannel::Close;
								aboardSame = true;
							}
							else if(pto->IsAboardVehicle() && dynamic_cast<const DaVehicle*>(pfrom) &&
									pto->Parent() == pfrom)
							{
								// Aboard vehicle that is the sender.
								senderActualRange = AuCommStdChannel::Close;
								aboardSame = true;
							}
							else if(pto->IsInsideCapsule() && pfrom->IsInsideCapsule() &&
									pto->FindParentCapsuleInEnv() == pfrom->FindParentCapsuleInEnv())
							{
								// Aboard same capsule as sender.
								senderActualRange = AuCommStdChannel::Close;
								aboardSame = true;
							}
							else if(pto->IsInsideCapsule() && dynamic_cast<const DaCapsule*>(pfrom) &&
									pto->FindParentCapsuleInEnv() == pfrom)
							{
								// Aboard capsule that is the sender.
								senderActualRange = AuCommStdChannel::Close;
								aboardSame = true;
							}
							else
								calcActualDistance = true;
						}

						if(scope == AuCommStdChannel::Audible)
						{
							// Calculate precise distance between the sender and the client's unit.
							AuPoint fromloc = pfrom->EnvLocation();
							AuPoint toloc = pto->EnvLocation();
							double dist = fromloc.Distance(toloc);

							// Audible message range is limited by the listener's DNA, not regular close range.
							float hearingRange = pto->HearingRange();
							if(dist < hearingRange)
							{
								// Test audibility between sender and receiver, modulated over the audible distance.
								// SetSoundHeard multiplies aud by 20 to get an integer result, so less than .05 is pointless.
								float aud = pfrom->RelativeAudioGain(pto) * (1. - dist / hearingRange);
								if(aud >= .05f)
								{
									// Set sound heard so the listener gets a visual indication of direction and audibility.
									AuPoint soundDir = pto->EnvTransform().UnProject(fromloc);	// Relative to pto
									soundDir.Unit();
									pto->SetSoundHeard(soundDir, aud);

									// Louder than 20% is intelligible; quieter gets the audibility indicator but not the text.
									if(aud > .2f)
										senderActualRange = AuCommStdChannel::Close;		// Parties are within close range.
									else
										senderActualRange = AuCommStdChannel::InvalidRange;	// Cannot make out what I said.
								}
								else
									senderActualRange = AuCommStdChannel::InvalidRange;	// Cannot hear me at all.
							}
							else
								senderActualRange = AuCommStdChannel::InvalidRange;	// Cannot hear me.
						}
						else if(calcActualDistance)
						{
							// Calculate precise distance between the sender and the client's unit.
							AuPoint fromloc = pfrom->EnvLocation();
							AuPoint toloc = pto->EnvLocation();
							double dist = fromloc.Distance(toloc);
							if(AuCommStdChannel::IsWithinRange(dist, AuCommStdChannel::Close))
								senderActualRange = AuCommStdChannel::Close;		// Parties are within close range.
							else if(AuCommStdChannel::IsWithinRange(dist, AuCommStdChannel::Hail))
								senderActualRange = AuCommStdChannel::Hail;			// Parties are within hail range.
							else
								senderActualRange = AuCommStdChannel::SolarSystem;	// Parties are within system range.
						}
					}
					else if(msg->Type() == AuCommPacket::CitySurrenderAccepted)
					{
						// Sender of the surrender accepted message could be a pirate emperor who does not really exist.
						// This could result in a person in the wrong system receiving this response but it's ok if they are
						// otherwise eligible to receive the message.
						if(DgEmpire::IsPirate(msg->FromEmpireId()))
							senderActualRange = AuCommStdChannel::SolarSystem;	// Message originated in this solar system.
						else
							senderActualRange = AuCommStdChannel::Sector;	// Message originated in a different solar system.
					}
					else
						senderActualRange = AuCommStdChannel::Sector;	// Message originated in a different solar system.
				}
				else
				{
					// The message originated in a different sector.
					const DaGalaxy* galaxy = mySector->FindParentGalaxy();
					if(!galaxy)
						senderActualRange = AuCommStdChannel::InvalidRange;		// Could not find my galaxy?
					else if(galaxy->Galaxy() != msg->FromGalaxy())
						senderActualRange = AuCommStdChannel::Universe;		// Message originated in a distant galaxy.
					else
					{
						const DaSector* fromSector =
							dynamic_cast<const DaSector*>(galaxy->FindNormalSubatomId(msg->FromSectorId()));
						if(fromSector && fromSector->Parent() == mySector->Parent())
						{
							// Both sectors are in the same galaxy.
							// Check distance from my sector to the sector that originated the message.
							// An adjacent sector will be 10 parsecs away; I'll check 11 in case of fuzzy errors.
							AuPoint vecToOrig = mySector->Location() - fromSector->Location();
							if(fabs(vecToOrig.x) < 11 && fabs(vecToOrig.y) < 11 && fabs(vecToOrig.z) < 11)
								senderActualRange = AuCommStdChannel::Sector;	// Message originated within a sector.
							else
								senderActualRange = AuCommStdChannel::Galaxy;	// Message originated in a distant sector.
						}
						else
							senderActualRange = AuCommStdChannel::Galaxy;		// Message originated in a distant sector.
					}
				}
			}
			else
				senderActualRange = AuCommStdChannel::InvalidRange;		// Could not find my sector?
		}
		else
			senderActualRange = AuCommStdChannel::InvalidRange;				// Could not find me or I haven't queried env yet.
		m_I->UnlockAvatar();
		if(senderActualRange == AuCommStdChannel::InvalidRange)
			return;			// Sender range could not be determined, or I am ignoring them, or I cannot hear them.

		// Messages with an associated story instance are not received by anyone else.
		// These messages can arrive on any channel, regardless of scope.
		if(!inStory && msg->StoryInstance())
			return;

		// Test scope of message against this recipient.
		switch(scope)
		{
		case AuCommStdChannel::Agent:
			if(!AcctIsAgent(accountType))
				return;											// Agent scope requires agent, programmer or architect.
			break;
		case AuCommStdChannel::Programmer:
			if(!AcctIsProgrammer(accountType))
				return;											// Programmer scope requires programmer or architect.
			break;
		case AuCommStdChannel::Architect:
			if(!AcctIsArchitect(accountType))
				return;											// Architect scope requires architect.
			break;
		case AuCommStdChannel::Fleet:
		case AuCommStdChannel::Squadron:
			if(!fleetOk)
				return;
			break;

		case AuCommStdChannel::Company:
			if(!companyOk)
				return;
			break;

		case AuCommStdChannel::CrewMates:						// Crew channel requires crew fellowship
			if(!crewOk)
				return;
			break;
		case AuCommStdChannel::Diplomacy:						// Diplomacy requires the person to hold some office.
			if(!diplomatOk)
				return;											// Person is not a member of any government.
			break;
		case AuCommStdChannel::Empire:							// Empire channel requires same empire.
			if(!empireOk)
				return;											// Empire allegiance did not match exactly.
			break;
		case AuCommStdChannel::Government:						// Government requires person to hold office and be in same empire.
			if(!empireOk || !diplomatOk)
				return;
			break;
		case AuCommStdChannel::Audible:							// Already tested audibility.
		case AuCommStdChannel::Everyone:						// Everyone scope is limited only by range
		case AuCommStdChannel::Narrator:						// Narrative requires same story instance. Already checked.
		case AuCommStdChannel::Thoughts:
		case AuCommStdChannel::Help:
			break;
		case AuCommStdChannel::Friends:							// Friend channel requires same empire or friendly empire.
			if(!friendOk)
				return;
			break;
		case AuCommStdChannel::Intercom:						// Intercom channel requires same parent ship.
			if(!aboardSame)
				return;											// Scene client is not aboard the same ship or vehicle.
			break;
		case AuCommStdChannel::ScopeTypes:
			// msg->Scope() never returns this.
			break;
		}

		// Determine if client is capable of receiving the message based on range.
		// The Intercom channel has already checked based on common parent ship or vehicle only.
		// Log messages are sent directly to an atom id, which has already been checked. Range is irrelevant.
		// Narrative messages are received based on participation in the message's story instance. Range is irrelevant.
		if(scope != AuCommStdChannel::Intercom && !msg->IsNarratorMessage() && senderActualRange > msg->TransmissionRange())
			return;												// Sender range is beyond the transmission range of the message.
	}

	// Now test to see if the client is monitoring the channel.
	if(!Frame()->IsCommChannelOpen(msg->ChannelName()))
	{
		// The client does not have the channel open.
		if(scope == AuCommStdChannel::Agent || scope == AuCommStdChannel::Help)
		{
			// Agent channels and the Help channel are only opened automatically for agents, programmers and architects.
			if(AcctIsAgent(accountType))
				emit SoloChannelOpened(msg->ChannelName(), scope, "");	// Open the channel for the agent, programmer or architect.
			else
				return;											// The channel does not open automatically. Skip the client.
		}
		else if(scope == AuCommStdChannel::Programmer)
		{
			// Programmer channel are only opened automatically for programmers and architects.
			if(AcctIsProgrammer(accountType))
				emit SoloChannelOpened(msg->ChannelName(), scope, "");	// Open the channel for the programmer or architect.
			else
				return;											// The channel does not open automatically. Skip the client.
		}
		else if(scope == AuCommStdChannel::Architect)
		{
			// Architect channel are only opened automatically for architects.
			if(AcctIsArchitect(accountType))
				emit SoloChannelOpened(msg->ChannelName(), scope, "");	// Open the channel for the architect.
			else
				return;											// The channel does not open automatically. Skip the client.
		}
		else if(AuCommStdChannel::IsOpenedAutomatically(msg->ChannelName(), senderActualRange, AcctIsMinion(accountType)))
		{
			// We can open the channel automatically for them.
			// Automatically opened channels get the default scope for that channel.
			// Only standard channels automatically open.
			emit SoloChannelOpened(msg->ChannelName(), AuCommStdChannel::Scope(msg->ChannelName()), "");
		}
		else if(scope == AuCommStdChannel::Narrator)
		{
			// We can open the channel automatically for them.
			// Channels opened under due to narration are identified as narrator scope channels.
			emit SoloChannelOpened(msg->ChannelName(), scope, g_Au.StoryBrief(msg->StoryInstance()));
		}
		else
			return;												// The channel does not open automatically. Skip the client.
	}

	// Send specie information associated with message items, before the message.
	/*DHH solo
	if(msg->Item())
		SendSpeciesRecordIfNeeded(msg->Item()->DNASpecies());
	if(msg->OfferItem())
		SendSpeciesRecordIfNeeded(msg->OfferItem()->DNASpecies());
		*/

	// Set the reception range in the message with the actual range that we determined.
	// That helps the client show the message according to actual range. The transmission range is irrelevant to the client.
	// Announcements always appear to come from Close range so they get a big font.
	// Silverwill messages always appear to come from Universe range so they get a tiny font.
	if(msg->Type() == AuCommPacket::Announcement)
		msg->SetReceptionRange(AuCommStdChannel::Close);
	else if(msg->Type() == AuCommPacket::Silverwill)
		msg->SetReceptionRange(AuCommStdChannel::Universe);
	else
		msg->SetReceptionRange(senderActualRange);

	// At this point, anonymous messages must hide the sender's actual identity. This copy will be sent to the client who could
	// discover that information.
	if(msg->IsSenderAnonymous())
		msg->SetFromId(AuId(1));

	// Bundle the message into a byte array so it can be emitted as a signal to myself, as if it came from the DtClientChat thread.
	QByteArray bamsg;
	QDataStream out(&bamsg, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)msg->Type();
	out << *msg;
	emit SoloChatReceived(bamsg);
}

void AuApp::CommAcceptTeleport(DaPerson* fromPerson, const AuCommPacket& msg, AuEther& ether)
{
	if(!IsSingleUserMode())
		return;

	DaAtom* destAtom = ether.FindNormalAtom(msg.ToId());
	if(!destAtom)
		return;

	DaSolarSystem* system = destAtom->FindParentSolarSystemInEnv();
	if(!system)
		return;

	system->AdminGoTo(fromPerson, destAtom);
}

void AuApp::SteamEquipAvatar()
{
	if(!IsSingleUserMode())
		return;

	DaPerson* person = m_I->LockAvatar();
	if(person)
		person->AgentMakeCommodity(8, CID_Invalid, 192, 1, person->Galaxy(), AuPoint());
	m_I->UnlockAvatar();
}

void AuApp::SteamCreateStarship()
{
	if(!IsSingleUserMode())
		return;

	DaPerson* person = m_I->LockAvatar();
	if(person)
	{
		DaWorldLL* llworld = person->FindParentWorldInEnv();
		if(llworld)
		{
			if(!person->EmpireId())
			{
				// Create a new empire for the dweeb.
				QString ename("Steamworks");
				QImage imFlag(":data/Steamworks.png");
				EstablishNewEmpire(person->Id(), person->Name(), person->Gender(), 0, ename, imFlag);
				const DgData& gd = g_Au.LockDataForRead();
				quint32 empireId = gd.EmpireId(ename);
				g_Au.UnlockData();
				person->SetEmpireId(empireId);
			}

			quint32 designId = 655903;	// Atlantis
			quint8 errcode;
			DaVessel* ship = llworld->AgentSpawnSpacecraft(person, designId, errcode);	// Person gets moved aboard, near the helm.
			if(ship)
			{
				// Make the person the captain.
				ship->OccupyBerth(AuRosterEntry::Captain, person->Id(), person->Name());
				person->SetSpawnShip(ship->Id(), ship->Name(), AuRosterEntry::Captain, ship->IsPrivatelyOwned());

				// Pirate equip the ship: christen, officer, crew, troops, ammo, cargo.
				person->AgentEncounter(8, designId);

				// Upgrade the ship to warp 9.
				if(ship->HasWarpFTLDrive())
				{
					quint16 which = ship->FTLWarpDrive();
					AuItem module(CID_Warp9FTLDriveModule, 255, person->EmpireId());
					ship->InstallSystemModule(AuSysId(SYS_FTLDrive, which), module, 0);
				}
			}
		}
	}
	m_I->UnlockAvatar();
}

void AuApp::SteamCreateStation()
{
	if(!IsSingleUserMode())
		return;

	DaPerson* person = m_I->LockAvatar();
	if(person)
	{
		DaWorldLL* llworld = person->FindParentWorldInEnv();
		if(llworld)
		{
			if(!person->EmpireId())
			{
				// Create a new empire for the dweeb.
				QString ename("Steamworks");
				QImage imFlag(":data/Steamworks.png");
				EstablishNewEmpire(person->Id(), person->Name(), person->Gender(), 0, ename, imFlag);
				const DgData& gd = g_Au.LockDataForRead();
				quint32 empireId = gd.EmpireId(ename);
				g_Au.UnlockData();
				person->SetEmpireId(empireId);
			}

			quint32 designId = 655902;	// Del Rey Station - GDrive
			quint8 errcode;
			DaVessel* ship = llworld->AgentSpawnSpacecraft(person, designId, errcode);	// Person gets moved aboard, near the helm.
			if(ship)
			{
				// Pirate equip the ship: christen, officer, crew, troops, ammo, cargo.
				person->AgentEncounter(8, designId);
			}
		}
	}
	m_I->UnlockAvatar();
}

void AuApp::ListSolitaireSaves(QStringList& names, QStringList& files)
{
	QDir cacheDir(g_Au.GameSaveDir());
	QStringList nameFilters;
	nameFilters << "sU.*";
	QStringList fileEntries = cacheDir.entryList(nameFilters);
	QMultiMap<QDateTime, QString> sorted;
	for(QStringList::const_iterator fi = fileEntries.constBegin(); fi != fileEntries.constEnd(); fi++)
	{
		QStringList fields = (*fi).split('.');
		if(fields.size() < 2)
			continue;
		QDateTime dt = QDateTime::fromString(fields[1], Qt::ISODate);
		sorted.insert(dt, *fi);
	}

	for(QMultiMap<QDateTime, QString>::const_iterator fi = sorted.cend()-1; fi != sorted.cbegin()-1; fi--)
	{
		// Beautify the name.
		QStringList fields = (*fi).split('.');
		if(fields.size() < 2)
			continue;
		QDateTime dt = QDateTime::fromString(fields[1], Qt::ISODate);
		QString name = dt.toLocalTime().toString();
		if(fields.size() >= 3)
		{
			name += " " + fields[2];
			for(int fi=3; fi<fields.size(); fi++)
				name += ", " + fields[fi];
		}
		names.append(name);
		files.append(*fi);
	}
}

void AuApp::RestoreSolitaireSave(const QString& fileName)
{
	// Remove the solo avatars file.
	QDir cacheDir(g_Au.GameSaveDir());
	QString cacheFileName = cacheDir.absoluteFilePath(s_UniverseFile);
	QString restoreFileName = cacheDir.absoluteFilePath(fileName);
	QFile::remove(cacheFileName);
	QFile::copy(restoreFileName, cacheFileName);

	// Remove the solo universe file.
	QString avFileName(fileName);
	avFileName.replace("sU.","sA.");
	cacheFileName = cacheDir.absoluteFilePath(s_LoginFile);
	restoreFileName = cacheDir.absoluteFilePath(avFileName);
	QFile::remove(cacheFileName);
	QFile::copy(restoreFileName, cacheFileName);

	// Remove cached files that could represent incorrect data.
	g_Au.ClearSoloCacheFiles();
}

void AuApp::DeleteSolitaireSave(const QString& fileName)
{
	// Remove the solo avatars file.
	QDir cacheDir(g_Au.GameSaveDir());
	QString restoreFileName = cacheDir.absoluteFilePath(fileName);
	QFile::remove(restoreFileName);

	// Remove the solo universe file.
	QString avFileName(fileName);
	avFileName.replace("sU.","sA.");
	restoreFileName = cacheDir.absoluteFilePath(avFileName);
	QFile::remove(restoreFileName);
}

void AuApp::SoloDeactivateAvatars()
{
	// We do this once per day and also the first time we get called.
	if(m_AvatarDeactivateTime.isValid() && m_AvatarDeactivateTime.ElapsedSec() < 60*60*24)
		return;
	m_AvatarDeactivateTime.restart();

	// Get lists of all avatars from the login server.
	AuIdList activeAvatars;
	const DgData& gd = g_Au.LockDataForRead();
	for(int ii=0; ii<6; ii++)
	{
		AuId avid = gd.ClientUnitId(ii);
		if(avid.IsNull())
			continue;
		activeAvatars.append(avid);
	}
	g_Au.UnlockData();
	if(activeAvatars.isEmpty())
		return;

	// Deactivate any live person atoms we have in our local data.
	DaAtomList people;
	AuEther& ether = g_Au.LockEtheriumDelete();
	ether.ListNormalPeople(people);	// Lists all IsNormal people. It's ok if we skip over people in other atom states.
	//qDebug() << "AuApp::DeactivateAvatars" << activeAvatars.count() << "active" << people.size() << "avatars in universe";
	for(DaAtomList::const_iterator pi = people.begin(); pi != people.end(); pi++)
	{
		DaPerson* person = (DaPerson*)*pi;
		if(activeAvatars.contains(person->Id()))
		{
			// The person is in the list of active avatars. Make sure the atom knows its account is active.
			//if(person->IsAccountInactive())
				//qDebug() << " Activating" << *person << (person->IsLineageDemiavatar()?"Demiavatar":"Avatar");
			person->SetAccountActive();
		}
		else
		{
			// The avatar is not associated with any login account.
			if(person->CreatedTime().ElapsedDay() > 0)
			{
				// The avatar is more than one day old. Delete the avatar.
				//qDebug() << (person->IsLineageDemiavatar()?" Demiavatar":" Avatar") << *person
					//<< "Kill'ed because they are not associated with a login account.";
				person->Kill();
				person->SetModified();
			}
		}
	}
	g_Au.UnlockEtheriumDelete();
}

void AuApp::GovSetJoinPassword(const QString& password)
{
	QByteArray joinPassCrypt;
	if(!password.isEmpty())
	{
		// Create a SHA-1 hash key from the password.
		QByteArray passCrypt;
		passCrypt.append(password.toUtf8());
		QCryptographicHash passCryptHash(QCryptographicHash::Sha1);
		passCryptHash.addData(passCrypt);
		joinPassCrypt = passCryptHash.result();
	}

	if(IsSingleUserMode())
	{
		m_I->GovSetJoinPassword(joinPassCrypt);
	}
	else
	{
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PV_SetJoinPass;
		out << joinPassCrypt;
		QueueChatPacket(ba);
	}
}

void AuApp::GovSetJoinPasswordUp(quint32 empire, QByteArray passCrypt)
{
	DgData& gd = g_Au.LockDataForWrite();
	gd.EmpireSetJoinPassCrypt(empire, passCrypt);
	g_Au.UnlockData();
}

