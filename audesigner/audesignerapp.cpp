//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audesignerapp.cpp,v 1.23 2022/04/15 16:39:21 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audesignerapp.h"
//
#include "auclientglobal.h"
#include "auframe.h"
#include "auglmodel.h"
#include "dgdata.h"
#include "dtsspacket.h"
#include "dttrash.h"
#if defined(Q_OS_LINUX)
#include "../../linuxPlatform.h"
#endif

AuDesignerApp::AuDesignerApp(int& argc, char** argv, const QString& appName, const QString& settingsAppName, bool isSolo) :
	AuApp(argc, argv)
{
	// Parse command line.
	QString altMaster;
	bool logClient = false;
	QSettings::Format settingsFormat = QSettings::NativeFormat;
	QStringList args = arguments();
	for(QStringList::const_iterator it = args.constBegin(); it != args.constEnd(); it++)
	{
		if(*it == "-log")
			logClient = true;
		else if(*it == "-ini")
			settingsFormat = QSettings::IniFormat;
		else if((*it).left(2) == "-a")
			altMaster = (*it).mid(2);
	}

	gp_AuGlobal = new AuClientGlobal(appName, settingsAppName, "Hazeron Freehold", isSolo, m_Data, m_Etherium, settingsFormat);
	if(logClient)
		g_Au.m_LogMessages |= LOG_ConnectDisconnect;	// Log what happens while starting up and logging in.
//#if !defined(QT_NO_DEBUG)
	//g_Au.m_DebugSwitches |= 0x00000001;	// Frame rate on screen
	//g_Au.m_DebugSwitches |= 0x00000002;	// Frame time on screen
//#endif

	g_Au.Dice().TimeSeed();

	// Create cache for 3ds models.
	m_ModelCache = new AuGLModelCache;
	g_Au.SetModelCache(m_ModelCache);

	// Set alternate servers.
	if(!altMaster.isEmpty())
	{
		g_Au.SetLoginMaster(altMaster);
		g_Au.SetMailMaster(altMaster);
		g_Au.SetSceneMaster(altMaster);
	}

	g_Au.CreateCacheApp();
	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
	{
		QFile::remove(g_Au.CacheApp() + "/soh.txt");
		g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: CPU cores %1").arg(QThread::idealThreadCount()));
	}

	m_tTrash = new DtTrash(this);
	m_tTrash->start();

	InitializeSound();
}

AuDesignerApp::~AuDesignerApp()
{
	if(m_tTrash)
	{
#if !defined(QT_NO_DEBUG)
		qDebug("D AuDesignerApp::~AuDesignerApp - Stopping Trash");
#endif

        m_tTrash->deleteLater();
        m_tTrash->Kill();

		AuElapsedTimer t;
		t.restart();
		while(m_tTrash->isRunning())
		{
			if(t.elapsed() > 1000)
			{
#if !defined(QT_NO_DEBUG)
				qDebug() << "D Waiting for the Trash thread to die.";
#endif
				t.restart();
			}
		}
#if !defined(QT_NO_DEBUG)
		qDebug("D AuDesignerApp::~AuDesignerApp - Trash stopped");
#endif
	}
}

void AuDesignerApp::SceneLogin()
{
	const DgData& gd = g_Au.LockDataForRead();
	bool isLoggedIn = gd.IsClientLoggedIn();
	quint32 idPlayer = gd.PlayerId();
	quint32 confirmNum = gd.ConfirmationNumber();
	AuId idUnit = gd.ClientUnitId();	// Id of unit the player will be controlling.
	g_Au.UnlockData();
	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: Scene login, player %1, logged in %2, sceneConnected %3")
				.arg(idPlayer)
				.arg(isLoggedIn?"yes":"no")
				.arg(SceneConnectedOnce()?"yes":"no"));
	Q_ASSERT(isLoggedIn);
	if(!isLoggedIn)
		return;

	quint8 status = 0;
	AuFrame* pFrame = Frame();
	if(pFrame->IsAFK())
		status |= 0x01;
	if(!SceneConnectedOnce())
		status |= 0x04;	// First scene connection.

	// Include scene version.
	QString sver(AuUtil::SceneVersion());

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtSceneServerPacket::PL_Login;
	out << sver;
	out << idPlayer;
	out << confirmNum;
	out << idUnit;

	// Client OS Information
	quint8 osType;
	quint16 osVer;
#if defined(Q_OS_WIN)
	osType = 1;
	osVer = QSysInfo::windowsVersion();
#elif defined(Q_OS_MACOS)
	osType = 2;
	osVer = QSysInfo::macVersion();
#else	// Q_OS_LINUX
	osType = 0;
	osVer = SEiLINUXPLATFORM;
#endif
	quint8 wordSize = QSysInfo::WordSize;
	out << osType;
	out << osVer;
	out << wordSize;
	out << status;

	if(g_Au.m_LogMessages & LOG_ConnectDisconnect)
		g_Au.Log(g_Au.CacheApp() + "/soh.txt", QString("App: Sending login TCP packet. %1 %2 %3")
				.arg(osType).arg(osVer).arg(wordSize));
	QueueScenePacket(ba);
}
