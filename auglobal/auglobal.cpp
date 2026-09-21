//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auglobal.cpp,v 1.311 2022/05/19 16:13:30 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auglobal.h"
//
#include "ausound.h"
#include "dtloginserver.h"
#include "dtmailserver.h"
#if !defined(Q_OS_WIN) && !defined(Q_OS_MACOS)
#include "malloc.h"						// for mallinfo
#endif
#include <QDir>
#include <QFile>
#include <QString>
#include <QTextStream>

AUGLOBAL_DLL AuGlobal*					gp_AuGlobal = 0;		// This is the one and only global data pointer.

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuHengeDestination::operator>>(QDataStream& out) const
{
	quint8 ver = 1;
	out << ver;
	out << m_HengeLL;
	out << m_SectorId;
	out << m_SystemId;
	out << m_SystemName;
	out << m_Sun;
	out << m_WorldCatName;
	out << m_WorldName;
	out << m_WorldType;
	out << m_Breathable;
	out << m_OrbitZone;
	out << m_OutsideTown;
}

void AuHengeDestination::operator<<(QDataStream& in)
{
	quint8 ver;
	in >> ver;
	in >> m_HengeLL;
	in >> m_SectorId;
	in >> m_SystemId;
	in >> m_SystemName;
	in >> m_Sun;
	if(ver < 1)
	{
		AuTime updated;
		in >> updated;
	}
	in >> m_WorldCatName;
	in >> m_WorldName;
	in >> m_WorldType;
	in >> m_Breathable;
	in >> m_OrbitZone;
	in >> m_OutsideTown;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuHengeDestinations::Update(const AuHengeDestination& hd)
{
	// Replace existing matching entry.
	bool updated = false;
	iterator hdi = begin();
	while(hdi != end())
	{
		if((*hdi).Is(hd))
		{
			// Replace matching entry.
			*hdi = hd;
			hdi++;
			updated = true;
		}
		else
			hdi++;
	}

	// Add nu entry.
	if(!updated)
		append(hd);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGlobal::AuGlobal(const QString& appName, const QString& company, MODE mode, DgData* data/*=0*/, AuEther* etherium/*=0*/,
		QSettings::Format format) :
	m_Settings(format, company, appName)
{
	Init(mode, appName, company, data, etherium);
}

AuGlobal::AuGlobal(const QString& appName, const QString& settingsAppName, const QString& company, MODE mode, DgData* data/*=0*/,
		AuEther* etherium/*=0*/, QSettings::Format format) :
	m_Settings(format, company, settingsAppName)
{
	Init(mode, appName, company, data, etherium);
}

AuGlobal::~AuGlobal()
{
	//qDebug() << "AuGlobal::~AuGlobal";
	if(m_LogFile)
		fclose(m_LogFile);
	if(m_LogConnectFile)
		fclose(m_LogConnectFile);
}

void AuGlobal::Init(MODE mode, const QString& appName, const QString& company, DgData* data, AuEther* etherium)
{
	m_AppName = appName;
	m_AtomManagerDispatch = 0;
	m_AtomTickerReleased = false;
	m_AtomTockerReleased = false;

	m_BodyStatSize = m_Settings.readNumEntry("/Settings/BodyStatSize", 100);
	if(m_BodyStatSize < 1)
		m_BodyStatSize = 1;
	else if(m_BodyStatSize > 200)
		m_BodyStatSize = 200;

	m_BuildingViewRange = m_Settings.readNumEntry("/Settings/BuildingViewRange", 12000);
	if(m_BuildingViewRange < 200)
		m_BuildingViewRange = 200;
	else if(m_BuildingViewRange > 20000)
		m_BuildingViewRange = 20000;
	m_BuildingViewRangeFt = m_BuildingViewRange * FeetPerMeter();
	m_BuildingViewRangeFt2 = (quint64)m_BuildingViewRangeFt * (quint64)m_BuildingViewRangeFt;

	m_BumpMapSeas = m_Settings.readBoolEntry("/Settings/BumpMapSeas", true);

	m_Company = company;

	m_ControlAverageSpanMs = m_Settings.readUInt("/Settings/ControlSmoothing", 0);
	if(m_ControlAverageSpanMs > 500)
		m_ControlAverageSpanMs = 500;

	m_ControlDecayMargin = m_Settings.readUInt("/Settings/ControlDecayMs", 500);
	if(m_ControlDecayMargin > 2000)
		m_ControlDecayMargin = 2000;

	m_DesignerBlockPaths = Settings().readBoolEntry("Designer/BlockPaths", false);

	m_DesignerFindEdges = Settings().readBoolEntry("Designer/FindEdges", true);

	m_DesignerFindFaces = Settings().readBoolEntry("Designer/FindFaces", true);

	m_DesignerFindVertices = Settings().readBoolEntry("Designer/FindVertices", true);

	m_DesignerGravityOn = Settings().readBoolEntry("Designer/GravityOn", false);

	m_Data = data;

	m_DetailedTerrainRange = m_Settings.readNumEntry("/Settings/DetailedTerrainRange", 800);
	if(m_DetailedTerrainRange < 200)
		m_DetailedTerrainRange = 200;
	else if(m_DetailedTerrainRange > 2000)
		m_DetailedTerrainRange = 2000;
	m_DetailedTerrainRangeFt = m_DetailedTerrainRange * FeetPerMeter();
	m_DetailedTerrainRangeFt2 = m_DetailedTerrainRangeFt * m_DetailedTerrainRangeFt;

	QByteArray ba;
	Settings().readEntry("/Settings/Emojis", ba);
	LoadDefaultEmojis();
	LoadEmojis(ba);			// Load user overrides

	m_FieldOfViewY = Settings().readNumEntry("/Settings/FieldOfViewY", 60.) * DEG2RAD;
	if(m_FieldOfViewY < 40*DEG2RAD || m_FieldOfViewY > 90*DEG2RAD)
		m_FieldOfViewY = 60*DEG2RAD;

	m_FieldOfViewXCos = 0;

	m_GrassViewRange = m_Settings.readNumEntry("/Settings/GrassViewRange", 100);
	if(m_GrassViewRange < 10)
		m_GrassViewRange = 10;
	else if(m_GrassViewRange > 200)
		m_GrassViewRange = 200;
	m_GrassViewRangeFt = m_GrassViewRange * FeetPerMeter();
	m_GrassViewRangeFt2 = m_GrassViewRangeFt * m_GrassViewRangeFt;

	m_LeftRightCursor = m_Settings.readNumEntry("/Settings/LeftRightCursor", 0);
	if(m_LeftRightCursor < 0 || m_LeftRightCursor > 2)
		m_LeftRightCursor = 0;

	m_LeftRightOrientation = m_Settings.readNumEntry("/Settings/LeftRightOrientation", 0);
	if(m_LeftRightOrientation < 0 || m_LeftRightOrientation > 2)
		m_LeftRightOrientation = 0;

	m_MailCityReportAutoDelete = m_Settings.readBoolEntry("/Settings/MailCityReportAutoDelete", true);
	m_MailCityReportOldAge = m_Settings.readNumEntry("/Settings/MailCityReportOldAge", 4);
	if(m_MailCityReportOldAge < 1 || m_MailCityReportOldAge > 100)
		m_MailCityReportOldAge = 4;

	m_MailFleetReportAutoDelete = m_Settings.readBoolEntry("/Settings/MailFleetReportAutoDelete", true);
	m_MailFleetReportOldAge = m_Settings.readNumEntry("/Settings/MailFleetReportOldAge", 6);
	if(m_MailFleetReportOldAge < 1 || m_MailFleetReportOldAge > 100)
		m_MailFleetReportOldAge = 6;

	m_MailNotificationMode = m_Settings.readNumEntry("/Settings/MailNotificationMode", 1);
	if(m_MailNotificationMode < 0 || m_MailNotificationMode > 2)
		m_MailNotificationMode = 1;

	m_MailOfficerReportAutoDelete = m_Settings.readBoolEntry("/Settings/MailOfficerReportAutoDelete", true);
	m_MailOfficerReportOldAge = m_Settings.readNumEntry("/Settings/MailOfficerReportOldAge", 1);
	if(m_MailOfficerReportOldAge < 1 || m_MailOfficerReportOldAge > 100)
		m_MailOfficerReportOldAge = 1;

	m_Net1Downloading = 0;
	m_Net1Uploading = 0;
	m_Net2Downloading = 0;
	m_Net2Uploading = 0;
	m_Net3Downloading = 0;
	m_Net3Uploading = 0;
	m_Net4Downloading = 0;
	m_Net4Uploading = 0;

	m_RockViewRange = m_Settings.readNumEntry("/Settings/RockViewRange", 150);
	if(m_RockViewRange < 10)
		m_RockViewRange = 10;
	else if(m_RockViewRange > 500)
		m_RockViewRange = 500;
	m_RockViewRangeFt = m_RockViewRange * FeetPerMeter();
	m_RockViewRangeFt2 = m_RockViewRangeFt * m_RockViewRangeFt;

	m_ShrubViewRange = m_Settings.readNumEntry("/Settings/ShrubViewRange", 200);
	if(m_ShrubViewRange < 50)
		m_ShrubViewRange = 50;
	else if(m_ShrubViewRange > 500)
		m_ShrubViewRange = 500;
	m_ShrubViewRangeFt = m_ShrubViewRange * FeetPerMeter();
	m_ShrubViewRangeFt2 = m_ShrubViewRangeFt * m_ShrubViewRangeFt;

	m_TreeViewRange = m_Settings.readNumEntry("/Settings/TreeViewRange", 400);
	if(m_TreeViewRange < 100)
		m_TreeViewRange = 100;
	else if(m_TreeViewRange > 1000)
		m_TreeViewRange = 1000;
	m_TreeViewRangeFt = m_TreeViewRange * FeetPerMeter();
	m_TreeViewRangeFt2 = m_TreeViewRangeFt * m_TreeViewRangeFt;

	m_DivTexels = m_Settings.readNumEntry("/Settings/DivTexels", 2);
	if(m_DivTexels < 1)
		m_DivTexels = 1;
	else if(m_DivTexels > 3)
		m_DivTexels = 3;

	m_DrawBody = m_Settings.readNumEntry("/Settings/DrawBodyModel", 1);
	m_DrawClouds = m_Settings.readNumEntry("/Settings/DrawClouds", 3);
	if(m_DrawClouds < 0 || m_DrawClouds > 3)
		m_DrawClouds = 3;
	m_DrawFramedPictures = m_Settings.readBoolEntry("/Settings/DrawFramedPictures", true);
	m_DrawGun = m_Settings.readBoolEntry("/Settings/DrawGun", true);
	m_DrawHelmetHUDAlways = m_Settings.readBoolEntry("/Settings/DrawHelmetHUDAlways", true);
	m_DrawMiniMe = m_Settings.readBoolEntry("/Settings/DrawMiniMe", true);
	m_DrawNebulaCloud = m_Settings.readBoolEntry("/Settings/DrawNebulaCloud", true);
	m_DrawSpecks = m_Settings.readBoolEntry("/Settings/DrawSpecks", true);
	m_DrawSkyBox = m_Settings.readBoolEntry("/Settings/DrawSkyBox", true);
	m_SkyBoxMipMap = m_Settings.readBoolEntry("/Settings/SkyBoxMipMap", true);
	m_SkyBoxPaneSize = m_Settings.readNumEntry("/Settings/SkyBoxSize", 1024);
	if(m_SkyBoxPaneSize < 256 || m_SkyBoxPaneSize > 4096)
		m_SkyBoxPaneSize = 1024;
	m_DrawWeatherEffects = m_Settings.readBoolEntry("/Settings/DrawWeatherEffects", true);

	m_FramedPictureSize = m_Settings.readNumEntry("/Settings/FramedPictureSize", 512);
	if(m_FramedPictureSize < 64 || m_FramedPictureSize > 2048)
		m_FramedPictureSize = 512;

	m_ResourceIconSize = m_Settings.readNumEntry("/Settings/ResourceIconSize", 100);
	if(m_ResourceIconSize < 1 || m_ResourceIconSize > 200)
		m_ResourceIconSize = 100;

	for(int joy=0; joy<6; joy++)
		m_EnableJoystick[joy] = m_Settings.readBoolEntry(QString("/Settings/EnableJoystick%1").arg(joy), true);

	m_LoginMasterName = AuUtil::LoginMasterName();
	m_MailMasterName = AuUtil::MailMasterName();
	m_Mode = mode;
	m_ModelsGL = 0;
	m_SceneMasterName = AuUtil::SceneMasterName();

	if(IsSceneSolo())
		m_Rating = Everyone;
	else
	{
		m_Rating = (RATING)m_Settings.readNumEntry("/Settings/Rating", Mature);
		if(m_Rating < Everyone)
			m_Rating = Everyone;
		else if(m_Rating > Adult)
			m_Rating = Adult;
	}

	m_BaseDir = QDir::home().absoluteFilePath(QString("%1").arg(ApplicationName()));
	m_BaseDir = m_Settings.readEntry("/Settings/BaseDir", m_BaseDir);

	m_CacheDir = m_BaseDir + "/Temporary";
	m_CacheDir = m_Settings.readEntry("/Settings/CacheDir", m_CacheDir);

	m_DesignerBackColor = m_Settings.readColorEntry("/Settings/DesignerBackColor", DesignerDefaultBackColor());

	m_DesignerGridColor = m_Settings.readColorEntry("/Settings/DesignerGridColor", DesignerDefaultGridColor());

	m_DesignerGridXColor = m_Settings.readColorEntry("/Settings/DesignerGridXColor", DesignerDefaultGridXColor());

	m_DesignerGridYColor = m_Settings.readColorEntry("/Settings/DesignerGridYColor", DesignerDefaultGridYColor());

	m_DesignerGridZColor = m_Settings.readColorEntry("/Settings/DesignerGridZColor", DesignerDefaultGridZColor());

	m_DesignerLadderColor = m_Settings.readColorEntry("/Settings/DesignerLadderPathColor", DesignerDefaultLadderPathColor());

	m_DesignerLineColor = m_Settings.readColorEntry("/Settings/DesignerLineColor", DesignerDefaultLineColor());

	m_DesignerSelColor = m_Settings.readColorEntry("/Settings/DesignerSelColor", DesignerDefaultSelColor());

	m_DesignerWalkColor = m_Settings.readColorEntry("/Settings/DesignerWalkPathColor", DesignerDefaultWalkPathColor());

	m_DesignerVertexSize = m_Settings.readNumEntry("/Settings/DesignerVertexSize", 20);
	if(m_DesignerVertexSize < 6)
		m_DesignerVertexSize = 6;
	else if(m_DesignerVertexSize > 60)
		m_DesignerVertexSize = 60;

	m_GameSaveDir = m_BaseDir + "/Saved Games";
	m_GameSaveDir = m_Settings.readEntry("/Settings/GameSaveDir", m_GameSaveDir);

	m_PictureDir = m_BaseDir + "/Pictures";
	m_PictureDir = m_Settings.readEntry("/Settings/PictureDir", m_PictureDir);

	m_FontAtom = QFont(TypefaceSans(), 16),
	m_FontAtom = Settings().readFontEntry("Settings/FontAtom", m_FontAtom);

	m_FontDesigner = QFont(TypefaceSans(), 12),
	m_FontDesigner = Settings().readFontEntry("Settings/FontDesigner", m_FontDesigner);

	m_OpacityActive = m_Settings.readNumEntry("/Settings/OpacityActive", 100);
	if(m_OpacityActive < 5 || m_OpacityActive > 100)
		m_OpacityActive = 100;

	m_OpacityInactive = m_Settings.readNumEntry("/Settings/OpacityInactive", 100);
	if(m_OpacityInactive < 5 || m_OpacityInactive > 100)
		m_OpacityInactive = 100;

	m_GroundCoverQty = m_Settings.readNumEntry("/Settings/GroundCoverQty", 200);
	if(m_GroundCoverQty < 0)
		m_GroundCoverQty = 0;
	else if(m_GroundCoverQty > 1000)
		m_GroundCoverQty = 1000;

	m_MaxLights = m_Settings.readNumEntry("/Settings/MaxLights", 8);
	if(m_MaxLights < 3)
		m_MaxLights = 3;
	else if(m_MaxLights > MAX_POINT_LIGHTS)
		m_MaxLights = MAX_POINT_LIGHTS;

	m_GroundTextureSize = m_Settings.readNumEntry("/Settings/GroundTextureSize", 128);
	if(m_GroundTextureSize < 64)
		m_GroundTextureSize = 64;
	else if(m_GroundTextureSize > 256)
		m_GroundTextureSize = 256;

	m_PlantBranchMaxSegs = m_Settings.readNumEntry("/Settings/PlantBranchMaxSegs", 10);
	if(m_PlantBranchMaxSegs < 1)
		m_PlantBranchMaxSegs = 1;

	m_PlantTextureSize = m_Settings.readNumEntry("/Settings/PlantTextureSize", 256);
	if(m_PlantTextureSize < 64)
		m_PlantTextureSize = 64;
	else if(m_PlantTextureSize > 512)
		m_PlantTextureSize = 512;

	m_TreeTrunkMinDia = m_Settings.readNumEntry("/Settings/TreeTrunkMinDia", .0415);	// 1/2"
	if(m_TreeTrunkMinDia < 0)
		m_TreeTrunkMinDia = 0;

	m_TreeTrunkFaces = m_Settings.readNumEntry("/Settings/TreeTrunkFaces", 7);
	if(m_TreeTrunkFaces < 3)
		m_TreeTrunkFaces = 3;

	m_TreeTrunkMaxSegs = m_Settings.readNumEntry("/Settings/TreeTrunkMaxSegs", 4);
	if(m_TreeTrunkMaxSegs < 1)
		m_TreeTrunkMaxSegs = 1;

	m_UdpDecayMargin = m_Settings.readUInt("/Settings/UdpDecayMs", 1000);
	if(m_UdpDecayMargin > 1000)
		m_UdpDecayMargin = 1000;

	m_UseNativeColorDialog = m_Settings.readBoolEntry("/Settings/UseNativeColorDialog", false);
	m_UseNativeFileDialog = m_Settings.readBoolEntry("/Settings/UseNativeFileDialog", false);
	m_UseNativeFontDialog = m_Settings.readBoolEntry("/Settings/UseNativeFontDialog", false);

	m_WorkerThreadCount = m_Settings.readNumEntry("/Settings/WorkerThreadCount", 0);	// 0=automatic
	if(m_WorkerThreadCount < 0 || m_WorkerThreadCount > QThread::idealThreadCount())
		m_WorkerThreadCount = 0;		// 0=automatic

	// Sound gain and overrides.
	m_SoundEffectsGain = m_Settings.readNumEntry(QString("/Sound/Volume"), 1.f);
	for(int s=SOUND_None+1; s < SOUND_Count; s++)
	{
		SOUND_EFFECT sound = (SOUND_EFFECT)s;

		// Gain
		float defGain = AuSound::DefaultSoundGain(sound);
		float gain = m_Settings.readNumEntry(QString("/Sound/Sound%1/Volume").arg(sound), defGain);
		if(gain != 1.f)
			m_SoundGain.insert(sound, gain);

		// Override
		double defRefDist = AuSound::DefaultSoundReferenceDistance(sound)/FeetPerMeter();	// Setting is stored in meters
		QString fileName = m_Settings.readEntry(QString("/Sound/Sound%1/File").arg(sound), "");	// Empty file name gets default wav.
		double refDist = m_Settings.readNumEntry(QString("/Sound/Sound%1/RefDist").arg(sound), defRefDist);
		if(refDist <= 0)
			refDist = 1;
		refDist *= FeetPerMeter();
		float defRolloff = AuSound::DefaultSoundRolloffFactor(sound);
		float rolloff = m_Settings.readNumEntry(QString("/Sound/Sound%1/Rolloff").arg(sound), defRolloff);
		if(rolloff <= 0)
			rolloff = 1;
		AuSoundOverride sndOver(fileName, refDist, rolloff);
		if(sndOver != AuSound::DefaultSoundOverride(sound))
			m_SoundOverrides.insert(sound, sndOver);
	}

	// Configure the number of workers in the global thread pool.
	QThreadPool::globalInstance()->setMaxThreadCount(WorkerThreadPoolCount());

	m_Etherium = etherium;

	m_ServerMSecSkew = 0;	// milliseconds skew = serverTime - localTime.

	// Server communications occur on the internal network. Don't forward
	// this port through the router to the outside world!
	m_ServerPort = AuUtil::LoginMasterPort() - 1;		// Scene subserver channel

	ResetClientChannelServer();
	ResetSceneServer();

	// Debug switches.
	m_LogConnectFile = 0;
	m_LogFile = 0;
	if(IsSceneApp() || IsMailClient())
	{
#if defined(QT_NO_DEBUG)
		m_LogMessages = LOG_Error;							// Client release builds have all log messages turned off.
#else
		m_LogMessages = LOG_Debug|LOG_Warning|LOG_Error;
#endif
	}
	else if(IsSceneWorker())
		m_LogMessages = LOG_Log|LOG_Debug|LOG_Warning|LOG_Error|LOG_LoadBalance|LOG_Admin;
	else
		m_LogMessages = LOG_Log|LOG_Debug|LOG_Warning|LOG_Error|LOG_LoadBalance|LOG_Admin|LOG_Status;

	//m_LogMessages |= LOG_Database;

	// Current bit usage of m_DebugSwitches:
	// 0x00000001 = Show frame rate
	// 0x00000002 = Show frame time
	// 0x00000004 = Log empire requests
	// 0x00000008 = No render near globe
	// 0x00000010 = No render sc
	// 0x00000020 = No render vehicle
	// 0x00000040 = No render at globe
	// 0x00000080 = No render far globe
	// 0x00000100 = No render globe atmosphere
	// 0x00000200 = No render flora
	// 0x00000400 = No render sea globe
	// 0x00000800 = No create globe device objects
	// 0x00001000 = No render globe
	// 0x00002000 = No render bd
	// 0x00004000 = No render bd design
	// 0x00008000 = Atom received or updated on client, also empire requests
	// 0x00020000 = Show paths
	// 0x00040000 = Enable AuEther Thorough Change Parent Check
	// 0x00080000 = Enable AuEther Thorough Trash Check
	// 0x00100000 = Enable AuEther Thorough Remove Check
	// 0x01000000 = Disable DtTockTransactions
	// 0x02000000 =
	// 0x04000000 = No UpdateTarget
	// 0x08000000 = No render item
	// 0x10000000 = No render globe rocks
	// 0x20000000 = No render roads
	// 0x40000000 = No render smoke on unit
	// 0x80000000 = 
	if(IsSceneApp())
		m_DebugSwitches = 0x000c0000;	// Client still does some thorough checking until I'm sure there are no problems.
	else
		m_DebugSwitches = 0x00000000;

	m_EnableAnimalAI = true;
	m_EnableAnimalEncounters = true;
	m_EnableCharacterAI = true;
	m_EnableCitizenPathing = true;
	m_EnableCopAI = true;
	m_EnableCreatureDecay = true;
	m_EnableCrewAI = true;
	m_EnableIndigenousEncounters = true;
	m_EnableIndigenousAI = true;
	m_EnableJobPosting = true;
	m_EnableMovement = true;
	m_EnableOfficerAI = true;
	m_EnablePassengerAI = true;
	m_EnablePirateEncounters = true;
	m_EnableReportOfficers = true;
	m_EnableSensorScanByCity = true;
	m_EnableSensorScanByShip = true;
	m_EnableSensorScanByStation = true;
	m_EnableSetModified = true;
	m_EnableStationResupply = true;
	m_EnableSystemEmission = true;
	m_EnableTick = true;
	m_EnableTock = true;
	m_EnableTroopAI = true;
	m_EnableTroopResupply = true;
	m_EnableUnitDecay = true;
	m_EnableWorkerAI = true;
}

// Adds a timestamped message to a log file.
//static
void AuGlobal::LogFile(FILE* fp, const QString& message)
{
#if defined(Q_OS_WIN)
	// Send the message out to text file on Windows.
	QFile file;
	if(fp == stdout)
		file.setFileName("SoH stdout.txt");
	else
		file.setFileName("SoH stderr.txt");
	if(file.open(QIODevice::Append))
#else
	// Send the message out to the file*.
	QFile file;
	if(file.open(fp, QIODevice::WriteOnly))
#endif
	{
		// Format a log message that includes a timestamp.
		QTextStream ts(&file);
#if defined(Q_OS_WIN)
		ts << AuTime::Now().UTC().toString(Qt::ISODate) << ' ' << message << "\r\n";
#else
		ts << AuTime::Now().UTC().toString(Qt::ISODate) << ' ' << message << '\n';
#endif
		file.close();
	}
}

// Adds a timestamped message to a log file.
//static
void AuGlobal::Log(const QString& fileName, const QString& message)
{
	// Append the message to the log file.
	QFile file(fileName);
	if(file.open(QIODevice::Append))
	{
		// Format a log message that includes a timestamp.
		QTextStream ts(&file);
#if defined(Q_OS_WIN)
		ts << AuTime::Now().UTC().toString(Qt::ISODate) << ' ' << message << "\r\n";
#else
		ts << AuTime::Now().UTC().toString(Qt::ISODate) << ' ' << message << '\n';
#endif
		file.close();
	}
}

void AuGlobal::SendMail(unsigned int idPlayer, unsigned int confirmNum, const AuId& idFromAtom, const QString& nameFrom,
		const AuId& idToAtom, AuMailMsg::MSGTYPE msgType, const QString& subject, const QString& bodyText,
		const AuId& fromSystemId, const QString& fromSystem, const AuId& fromWorldId, const QString& fromWorld,
		GALAXY fromGalaxy, const AuPoint& fromGalaxyLoc)
{
	//qDebug("AuGlobal::SendMail from %s %d to %d msgType %d subj '%s' "
			//"body '%s'",
			//(const char*)nameFrom.toAscii(),
			//idFromAtom.GetID(), idToAtom.GetID(),
			//msgType,
			//(const char*)subject.toAscii(),
			//(const char*)bodyText.toAscii());
	if(!IsSceneSolo() && !TestConfirmationValidity(confirmNum))
	{
		if(m_LogMessages & LOG_Warning)
		{
			QString logmsg;
			logmsg = QObject::tr("AuGlobal::SendMail - Invalid "
					"confirmation number %1, player %2, fromAtom %3, fromName %4, "
					"toAtom %5, msgType %6, subject %7, body size %8").arg(confirmNum)
					.arg(idPlayer).arg(idFromAtom.GetID()).arg(nameFrom).arg(idToAtom.GetID())
					.arg(msgType).arg(subject).arg(bodyText.length());
			Warning(logmsg);
		}
		return;
	}

	// Use mail message record to format output data.
	AuMailMsg msg;
	msg.m_id = 0;
	msg.m_tmSent.Set();
	msg.m_idPlayer = idPlayer;
	msg.m_nameFrom = nameFrom;
	msg.m_idFrom = idFromAtom;
	msg.m_idTo = idToAtom;
	msg.m_msgType = msgType;
	msg.m_subject = subject;
	msg.m_body = bodyText;
	msg.m_deleted = false;
	msg.m_fromSystem = fromSystem;
	msg.m_fromSystemId = fromSystemId;
	msg.m_fromWorld = fromWorld;
	msg.m_fromWorldId = fromWorldId;
	msg.m_fromGalaxy = fromGalaxy;
	msg.m_fromGalaxyX = (float)fromGalaxyLoc.x;
	msg.m_fromGalaxyY = (float)fromGalaxyLoc.y;
	msg.m_fromGalaxyZ = (float)fromGalaxyLoc.z;
	if(!msg.ApplyFieldLimits())
	{
		//qDebug("AuGlobal::SendMail ApplyFieldLimits failed");
		return;
	}

	// Format the mail message packet.
	// Mail message must include login account id and confirmation number.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtMailServer::P_Message;	// mail message packet type
	QString ver(AuUtil::LoginVersion());			// NOTE: Only client inserts version
	out << ver;								// into P_Message packet.
	out << (quint32)idPlayer;				// NOTE: Only client inserts login
	out << (quint32)confirmNum;				// info into P_Message packet.
	msg.Tx(out);
	// Add the message packet to the transmit data stream.
	// The DtMailClient thread will send it eventually.
	//qDebug("AuGlobal::SendMail LockMailBuffer");
	QByteArray& txbuf = LockMailBuffer("AuGlobal::SendMail");
	AuUtil::WritePacketSize(txbuf, ba.size());
	txbuf.append(ba);
	UnlockMailBuffer("AuGlobal::SendMail");
	//qDebug("AuGlobal::SendMail UnlockMailBuffer and exit nicely.");
}

void AuGlobal::SendBulkMail(unsigned int idPlayer, unsigned int confirmNum, const AuId& idFromAtom, const QString& nameFrom,
		const AuIdList& recipients, AuMailMsg::MSGTYPE msgType, const QString& subject, const QString& bodyText,
		const AuId& fromSystemId, const QString& fromSystem, const AuId& fromWorldId, const QString& fromWorld,
		GALAXY fromGalaxy, const AuPoint& fromGalaxyLoc)
{
	//qDebug("AuGlobal::SendMail from %s %d to %d msgType %d subj '%s' "
			//"body '%s'",
			//(const char*)nameFrom.toAscii(),
			//idFromAtom.GetID(), idToAtom.GetID(),
			//msgType,
			//(const char*)subject.toAscii(),
			//(const char*)bodyText.toAscii());

	// If there is only one recipient, only call SendMail - less data to transfer
	if(recipients.count() == 1)
	{
		SendMail(idPlayer, confirmNum, idFromAtom, nameFrom, recipients[0], msgType, subject, bodyText, fromSystemId, fromSystem,
				fromWorldId, fromWorld, fromGalaxy, fromGalaxyLoc);
		return;
	}

	if(!IsSceneSolo() && !TestConfirmationValidity(confirmNum))
	{
		if(m_LogMessages & LOG_Warning)
		{
			QString logmsg;
			logmsg = QObject::tr("AuGlobal::SendBulkMail - Invalid "
					"confirmation number %1, player %2, fromAtom %3, fromName %4, "
					"toAtomCount %5, msgType %6, subject %7, body size %8").arg(confirmNum)
					.arg(idPlayer).arg(idFromAtom.GetID()).arg(nameFrom).arg(recipients.count())
					.arg(msgType).arg(subject).arg(bodyText.length());
			Warning(logmsg);
		}
		return;
	}

	// Use mail message record to format output data.
	AuMailMsg msg;
	msg.m_id = 0;
	msg.m_tmSent.Set();
	msg.m_idPlayer = idPlayer;
	msg.m_nameFrom = nameFrom;
	msg.m_idFrom = idFromAtom;
	msg.m_idTo = -1;
	msg.m_msgType = msgType;
	msg.m_subject = subject;
	msg.m_body = bodyText;
	msg.m_deleted = false;
	msg.m_fromSystem = fromSystem;
	msg.m_fromSystemId = fromSystemId;
	msg.m_fromWorld = fromWorld;
	msg.m_fromWorldId = fromWorldId;
	msg.m_fromGalaxy = fromGalaxy;
	msg.m_fromGalaxyX = (float)fromGalaxyLoc.x;
	msg.m_fromGalaxyY = (float)fromGalaxyLoc.y;
	msg.m_fromGalaxyZ = (float)fromGalaxyLoc.z;
	if(!msg.ApplyFieldLimits())
	{
		//qDebug("AuGlobal::SendMail ApplyFieldLimits failed");
		return;
	}

	// Format the mail message packet.
	// Mail message must include login account id and confirmation number.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)DtMailServer::P_BulkMessage;	// bulk mail message packet type (multiple recipients)
	QString ver(AuUtil::LoginVersion());			// NOTE: Only client inserts version
	out << ver;								// into P_Message packet.
	out << (quint32)idPlayer;				// NOTE: Only client inserts login
	out << (quint32)confirmNum;				// info into P_Message packet.
	msg.Tx(out);							// the message
	recipients >> out;						// list of recipients
	// Add the message packet to the transmit data stream.
	// The DtMailClient thread will send it eventually.
	//qDebug("AuGlobal::SendMail LockMailBuffer");
	QByteArray& txbuf = LockMailBuffer("AuGlobal::SendBulkMail");
	AuUtil::WritePacketSize(txbuf, ba.size());
	txbuf.append(ba);
	UnlockMailBuffer("AuGlobal::SendMail");
}

// static
// Only called by servers.
bool AuGlobal::IsPlayerLoggedIn(unsigned int idPlayer, unsigned int confirmNum, AuId* pIdUnit, ACCOUNT* acctType, AuTime* expiry)
{
	if(!idPlayer)
		return false;
	if(!TestConfirmationValidity(confirmNum))
	{
		QString logmsg = QObject::tr("warning: AuGlobal::IsPlayerLoggedIn - Invalid confirmation number %1, player %2")
			.arg(confirmNum).arg(idPlayer);
		AuGlobal::LogFile(stderr, logmsg);
		return false;
	}

	// We always start at the login master for deferral to a login worker. If we cache the worker name we get stuck if it crashes.
	QString loginServerName = g_Au.LoginMasterName();
	quint16 loginPort = AuUtil::LoginMasterPort();

	// This will be our return result.
	bool isLoggedIn = false;
	bool deferred;
	do
	{
		// Connect to the login server.
		deferred = false;
		QTcpSocket tcpSocket;
		tcpSocket.connectToHost(loginServerName, loginPort);
		tcpSocket.waitForConnected(10000);
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			// The connection attempt succeeded.
			// Transmit the name and password to the server and wait for a reply.
			// We also send the version so it can be verified right off.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			if(pIdUnit && acctType && expiry)
				out << (quint8)DtLoginServer::P_LoginQueryUnit;	// Want unit ids also
			else
				out << (quint8)DtLoginServer::P_LoginQuery;	// Want only login state
			out << idPlayer;
			out << confirmNum;
			AuUtil::WritePacketSize(tcpSocket, ba.size());
			tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten(10000);

			// The server will send back a reply, good or bad.
			quint32 packetSize = 0;
			int packetSizeBytesLeft = 0;
			tcpSocket.waitForReadyRead(10000);
			while(tcpSocket.state() == QAbstractSocket::ConnectedState || tcpSocket.bytesAvailable())
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
					case DtLoginServer::P_LoginQuery:
						// This reply is only valid if we don't want a unit id.
						if(!pIdUnit || !acctType || !expiry)
						{
							quint8 loggedIn;
							block >> loggedIn;
							isLoggedIn = loggedIn ? true : false;
						}
						break;

					case DtLoginServer::P_LoginQueryUnit:
						// This reply is only valid if we want a unit id.
						if(pIdUnit && acctType && expiry)
						{
							quint8 loggedIn;
							block >> loggedIn;
							isLoggedIn = loggedIn ? true : false;
							if(isLoggedIn)
							{
								for(int iu=0; iu<6; iu++)
									pIdUnit[iu] << block;
								quint8 at;
								block >> at;
								*acctType = (ACCOUNT)at;
								block >> *expiry;
							}
						}
						break;
					case DtLoginServer::P_Deferred:
						{
							block >> loginServerName;
							block >> loginPort;
							deferred = true;
						}
						break;

					case DtLoginServer::P_DeferFailed:
						// Server tried to defer us but failed. We're done now.
						break;

					default:
						qWarning("HACKER WARNING! AuGlobal::IsPlayerLoggedIn "
								"Unexpected packet type %d", ptype);
						break;
					}
				}

				if(tcpSocket.state() == QAbstractSocket::ConnectedState)
					tcpSocket.waitForDisconnected(100);
			}

			// Close the tcp connection.
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
			{
				tcpSocket.disconnectFromHost();
				if(tcpSocket.state() == QAbstractSocket::ConnectedState)
					tcpSocket.waitForDisconnected(10000);
			}
		}
	}
	while(deferred);
	
	// Return the final result.
	return isLoggedIn;
}

unsigned int AuGlobal::GenConfirmation()
{
	// Assign a random confirmation number, max 10m
	unsigned int confirmNum = m_Dice.Roll(1,10000000);

	// Do some things to this random number so we can readily identify a fake.
	confirmNum |= 0x00000010;	// Bit 4 is always 1.
	confirmNum &= 0xffffffbf;	// Bit 6 is always 0.

	// Bit 11 is always the complement of bit 1.
	if(confirmNum & 0x00000002)
		confirmNum &= 0xfffff7ff;	// Bit 1=1 so bit 11 is set to 0.
	else
		confirmNum |= 0x00000800;	// Bit 1=0 so bit 11 is set to 1.

	// Insure top bit isn't set. (Can't be with max of 10m).
	confirmNum &= 0x7fffffff;

	return confirmNum;
}

//static
bool AuGlobal::TestConfirmationValidity(unsigned int confirmNum)
{
	// Confirmation numbers are never greater than 10m.
	if(confirmNum > 10000000)
		return false;	// Invalid max

	// Bit 4 is always 1.
	if((confirmNum & 0x00000010) == 0)
		return false;	// Invalid bit 4

	// Bit 6 is always 0.
	if(confirmNum & 0x00000040)
		return false;	// Invalid bit 6

	// Bit 11 is always the complement of bit 1.
	if(confirmNum & 0x00000002)
	{
		// Bit 1 is 1 so bit 11 needs to be 0.
		if(confirmNum & 0x00000800)
			return false;	// Invalid bit 11
	}
	else
	{
		// Bit 1 is 0 so bit 11 needs to be 1.
		if((confirmNum & 0x00000800) == 0)
			return false;	// Invalid bit 11
	}

	// The confirmation number passed our validity tests.
	return true;
}

void AuGlobal::MonitorChannel(const QString& channelName, const AuId& idMonitoringAtom)
{
	m_muAtomChannels.lock();

	bool foundIt = false;
	for(unordered_multimap<QString, AuCommChannelConnection, AuStringHash>::iterator ci = m_CommChannels.find(channelName);
			ci != m_CommChannels.end() && (*ci).first == channelName; ci++)
	{
		if((*ci).second.IsMonitoredBy(idMonitoringAtom))
		{
			(*ci).second.m_LastUse.restart();
			foundIt = true;
			//qDebug("AuGlobal::MonitorChannel Atom %d still monitoring %s channel.",
					//idMonitoringAtom.GetID(), (const char*)channelName.toAscii());
			break;
		}
	}

	if(!foundIt)
	{
		AuCommChannelConnection con;
		con.m_idAtom = idMonitoringAtom;
		con.m_LastUse.start();
		m_CommChannels.insert(pair<QString, AuCommChannelConnection>(channelName, con));
		//qDebug("AuGlobal::MonitorChannel Atom %d now monitoring %s channel.",
				//idMonitoringAtom.GetID(), (const char*)channelName.toAscii());
	}

	m_muAtomChannels.unlock();
}

// NOTE: Root mutex needs to be locked already.
void AuGlobal::StopMonitoringChannel(const QString& channelName, const AuId& idMonitoringAtom)
{
	m_muAtomChannels.lock();
	for(unordered_multimap<QString, AuCommChannelConnection, AuStringHash>::iterator ci = m_CommChannels.find(channelName);
			ci != m_CommChannels.end() && (*ci).first == channelName; ci++)
	{
		if((*ci).second.IsMonitoredBy(idMonitoringAtom))
		{
			m_CommChannels.erase(ci);
			//qDebug("AuGlobal::StopMonitoringChannel Atom %d stopped monitoring %s channel.",
					//idMonitoringAtom.GetID(), (const char*)channelName.toAscii());
			break;
		}
	}
	m_muAtomChannels.unlock();
}

void AuGlobal::StopMonitoringChannels(const AuId& idMonitoringAtom)
{
	//qDebug("AuGlobal::StopMonitoringChannels Atom %d stopped monitoring ALL channel.", idMonitoringAtom.GetID());
	m_muAtomChannels.lock();
	unordered_multimap<QString, AuCommChannelConnection, AuStringHash>::iterator ci = m_CommChannels.begin();
	while(ci != m_CommChannels.end())
	{
		if((*ci).second.IsMonitoredBy(idMonitoringAtom))
			ci = m_CommChannels.erase(ci);
		else
			ci++;
	}
	m_muAtomChannels.unlock();
}

void AuGlobal::DecayChannelMonitors()
{
	m_muAtomChannels.lock();
	unordered_map<QString, AuCommChannelConnection, AuStringHash>::iterator ci = m_CommChannels.begin();
	while(ci != m_CommChannels.end())
	{
		if((*ci).second.IsTooOld())
		{
			//qDebug("AuGlobal::DecayChannelMonitors Atom %d channel %s decayed.",
					//(*it).m_idAtom.GetID(), (const char*)(*it).m_ChannelName.toAscii());
			ci = m_CommChannels.erase(ci);
		}
		else
			ci++;
	}
	m_muAtomChannels.unlock();
}

// Needed on the scene client to insure time sensitive calculations are as
// close to the clock on the scene server as possible. i.e. Stellar and
// planetary rotation and orbit angles.
void AuGlobal::ServerTimeUpdated(const AuTime& serverUTC)
{
	m_ServerMSecSkew = AuTime::CurrentMSecsTo(serverUTC);
	//qDebug() << "AuGlobal::ServerTimeUpdated offset" << m_ServerMSecSkew << "curMSecTo" << AuTime::CurrentMSecsTo(serverUTC)
		//<< "now" << AuTime::Now() << "serverUTC" << serverUTC;
}

COMMODITYID AuGlobal::CommodityId(const QString& name) const
{
	for(DbCommodityArray::const_iterator it = m_CommodityArray.constBegin(); it != m_CommodityArray.constEnd(); it++)
		if((*it).m_id != CID_Invalid)
			if((*it).m_name.compare(name, Qt::CaseInsensitive) == 0)
				return (*it).m_id;	// This is the matching commodity.

	// The commodity wasn't found.
	return CID_Invalid;
}

void AuGlobal::SetModelCache(AuGLModelCache* modelsGLSL)
{
	m_ModelsGL = modelsGLSL;
}

QString AuGlobal::DecodeUTCString(const QString& encodedUTC) const
{
	uint utcSecs = encodedUTC.mid(4).toUInt(0, 16);
	QDateTime utcDateTime;
	utcDateTime.setTime_t(utcSecs);
	utcDateTime.setTimeSpec(Qt::UTC);
	utcDateTime.toLocalTime();
	return utcDateTime.toString(Qt::SystemLocaleShortDate);
}

QString AuGlobal::DecodeUTCStrings(const QString& encodedUTCs) const
{
	// Quickie test to see if there are any encoded UTCs in the string at all.
	const int encLen = 12;										// e.g. UTC:01234567, always 8 hex digits
	int utcpos = encodedUTCs.indexOf("UTC:");
	if(utcpos < 0 || utcpos > encodedUTCs.count() - encLen)
		return encodedUTCs;										// Not enough string left to contain an encoded UTC.

	QString decodedUTCs(encodedUTCs);
	do
	{
		QString encodedUTC = decodedUTCs.mid(utcpos, encLen);
		decodedUTCs.replace(encodedUTC, DecodeUTCString(encodedUTC));
		utcpos = decodedUTCs.indexOf("UTC:");					// Locate the next encoded UTC in the string.
	}
	while(utcpos >= 0 && utcpos <= decodedUTCs.count() - encLen);
	return decodedUTCs;
}

bool AuGlobal::CreateCacheApp() const
{
	// Check to see if the cache directory already exists.
	QDir cacheDir(CacheApp());
	if(cacheDir.exists())
		return true;					// Cache directory already exists.

	// The cache directory does not exist. Attempt to create it.
	if(!cacheDir.mkpath("."))
		return false;					// Failed to create the cache directory.

	// Create a file in the cache directory to explain why it is here.
	QFile whatsThis(cacheDir.absoluteFilePath("About This Folder.txt"));
	if(whatsThis.open(QFile::WriteOnly))
	{
		QTextStream ts(&whatsThis);
		ts << "Folder Name: " << cacheDir.absolutePath() << "\n";
		ts << "Created: " << AuTime::Now().UTC().toString() << "\n";
		ts << "Created By: " << ApplicationName() << "\n";
		ts << "Purpose: Data Cache\n";
		ts << "\n";
		ts << "This folder contains transient files created by the " << ApplicationName() << " program. "
			"The data contained in these files enables the program to save precious network "
			"bandwidth and CPU processing time. "
			"Removal of these files is discouraged if you are actively using " << ApplicationName() << "; "
			"the program will be forced to download large amounts of data and it may consume a lot of CPU time to "
			"recreate these files when it needs them.\n";
		ts << "\n";
		ts << "The files in this folder contain only data. "
			"They are not executable programs. "
			"Viruses cannot spread by infecting any of these files.\n";
		ts << "\n";
		ts << "It is safe to remove this folder or any of the files contained herein. "
			"No harm to the " << ApplicationName() << " program installation will occur if any of these files "
			"are removed. "
			<< ApplicationName() << " will not have to be reinstalled as a result of removing any of these files.\n";
		whatsThis.close();
	}

	// Cache directory created successfully.
	return true;
}

bool AuGlobal::CreateGameSaveDir() const
{
	// Check to see if the cache directory already exists.
	QDir cacheDir(GameSaveDir());
	if(cacheDir.exists())
		return true;					// Cache directory already exists.

	// The cache directory does not exist. Attempt to create it.
	if(!cacheDir.mkpath("."))
		return false;					// Failed to create the cache directory.

	// Create a file in the cache directory to explain why it is here.
	QFile whatsThis(cacheDir.absoluteFilePath("About This Folder.txt"));
	if(whatsThis.open(QFile::WriteOnly))
	{
		QTextStream ts(&whatsThis);
		ts << "Folder Name: " << cacheDir.absolutePath() << "\n";
		ts << "Created: " << AuTime::Now().UTC().toString() << "\n";
		ts << "Created By: " << ApplicationName() << "\n";
		ts << "Purpose: Data Cache\n";
		ts << "\n";
		ts << "This folder contains saved game files created by the " << ApplicationName() << " program. "
			"Files include the current game in progress and checkpoint saves.\n";
		ts << "\n";
		ts << "The files in this folder contain only data. "
			"They are not executable programs. "
			"Viruses cannot spread by infecting any of these files.\n";
		ts << "\n";
		ts << "It is safe to remove this folder or any of the files contained herein. "
			"No harm to the " << ApplicationName() << " program installation will occur if any of these files "
			"are removed. "
			<< ApplicationName() << " will not have to be reinstalled as a result of removing any of these files.\n";
		whatsThis.close();
	}

	// Cache directory created successfully.
	return true;
}

bool AuGlobal::CreateCacheGame() const
{
	// Check to see if the cache directory already exists.
	QDir cacheDir(CacheGame());
	if(cacheDir.exists())
		return true;					// Cache directory already exists.

	// The cache directory does not exist. Attempt to create it.
	if(!cacheDir.mkpath("."))
		return false;					// Failed to create the cache directory.

	// Create a file in the cache directory to explain why it is here.
	QFile whatsThis(cacheDir.absoluteFilePath("About This Folder.txt"));
	if(whatsThis.open(QFile::WriteOnly))
	{
		QTextStream ts(&whatsThis);
		ts << "Folder Name: " << cacheDir.absolutePath() << "\n";
		ts << "Created: " << AuTime::Now().UTC().toString() << "\n";
		ts << "Created By: " << ApplicationName() << "\n";
		ts << "Purpose: Data Cache\n";
		ts << "\n";
		ts << "This folder contains transient files created by the " << ApplicationName() << " program. "
			"The data contained in these files enables the program to save precious network "
			"bandwidth and CPU processing time. "
			"Removal of these files is discouraged if you are actively using " << ApplicationName() << "; "
			"the program will be forced to download large amounts of data and it may consume a lot of CPU time to "
			"recreate these files when it needs them.\n";
		ts << "\n";
		ts << "The files in this folder contain only data. "
			"They are not executable programs. "
			"Viruses cannot spread by infecting any of these files.\n";
		ts << "\n";
		ts << "It is safe to remove this folder or any of the files contained herein. "
			"No harm to the " << ApplicationName() << " program installation will occur if any of these files "
			"are removed. "
			<< ApplicationName() << " will not have to be reinstalled as a result of removing any of these files.\n";
		whatsThis.close();
	}

	// Cache directory created successfully.
	return true;
}

bool AuGlobal::CreateCacheSolo() const
{
	// Check to see if the cache directory already exists.
	QDir cacheDir(CacheSolo());
	if(cacheDir.exists())
		return true;					// Cache directory already exists.

	// The cache directory does not exist. Attempt to create it.
	if(!cacheDir.mkpath("."))
		return false;					// Failed to create the cache directory.

	// Create a file in the cache directory to explain why it is here.
	QFile whatsThis(cacheDir.absoluteFilePath("About This Folder.txt"));
	if(whatsThis.open(QFile::WriteOnly))
	{
		QTextStream ts(&whatsThis);
		ts << "Folder Name: " << cacheDir.absolutePath() << "\n";
		ts << "Created: " << AuTime::Now().UTC().toString() << "\n";
		ts << "Created By: " << ApplicationName() << "\n";
		ts << "Purpose: Data Cache\n";
		ts << "\n";
		ts << "This folder contains transient files created by the " << ApplicationName() << " program. "
			"The data contained in these files enables the program to save precious network "
			"bandwidth and CPU processing time. "
			"Removal of these files is discouraged if you are actively using " << ApplicationName() << "; "
			"the program will be forced to download large amounts of data and it may consume a lot of CPU time to "
			"recreate these files when it needs them.\n";
		ts << "\n";
		ts << "The files in this folder contain only data. "
			"They are not executable programs. "
			"Viruses cannot spread by infecting any of these files.\n";
		ts << "\n";
		ts << "It is safe to remove this folder or any of the files contained herein. "
			"No harm to the " << ApplicationName() << " program installation will occur if any of these files "
			"are removed. "
			<< ApplicationName() << " will not have to be reinstalled as a result of removing any of these files.\n";
		whatsThis.close();
	}

	// Cache directory created successfully.
	return true;
}

// 'unitId' is any unit owned by a player.
// Fills the array idUnits with the player's current unit ids.
// Queries the login server to do this.
// Returns true if the player record was found and unit ids returned.
//static
bool AuGlobal::ListPlayerUnits(const AuId& unitId, AuId* idUnits)
{
	// We always start at the login master for deferral to a login worker. If we cache the worker name we get stuck if it crashes.
	QString loginServerName = g_Au.LoginMasterName();
	quint16 loginPort = AuUtil::LoginMasterPort();

	// This will be our return result.
	bool unitsFound = false;
	bool deferred;
	do
	{
		// Connect to the login server.
		deferred = false;
		QTcpSocket tcpSocket;
		tcpSocket.connectToHost(loginServerName, loginPort);
		tcpSocket.waitForConnected(10000);
		if(tcpSocket.state() == QAbstractSocket::ConnectedState)
		{
			// The connection attempt succeeded.
			// Transmit the name and password to the server and wait for a reply.
			// We also send the version so it can be verified right off.
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			out << (quint8)DtLoginServer::P_UnitQuery;
			out << unitId;
			AuUtil::WritePacketSize(tcpSocket, ba.size());
			tcpSocket.write(ba);
			tcpSocket.waitForBytesWritten(10000);

			// The server will send back a reply, good or bad.
			quint32 packetSize = 0;
			int packetSizeBytesLeft = 0;
			tcpSocket.waitForReadyRead(10000);
			while(tcpSocket.state() == QAbstractSocket::ConnectedState || tcpSocket.bytesAvailable())
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
					case DtLoginServer::P_UnitQuery:
						{
							for(int iu=0; iu<6; iu++)
								block >> idUnits[iu];

							unitsFound = true;
						}
						break;

					case DtLoginServer::P_Deferred:
						{
							block >> loginServerName;
							block >> loginPort;
							deferred = true;
						}
						break;

					case DtLoginServer::P_DeferFailed:
						// Server tried to defer us but failed. We're done now.
						break;

					default:
						qWarning("HACKER WARNING! AuGlobal::ListPlayerUnits Unexpected packet type %d", ptype);
						break;
					}
				}

				if(tcpSocket.state() == QAbstractSocket::ConnectedState)
					tcpSocket.waitForDisconnected(100);
			}

			// Close the tcp connection.
			if(tcpSocket.state() == QAbstractSocket::ConnectedState)
			{
				tcpSocket.disconnectFromHost();
				if(tcpSocket.state() == QAbstractSocket::ConnectedState)
					tcpSocket.waitForDisconnected(10000);
			}
		}
	}
	while(deferred);
	
	// Return the final result.
	return unitsFound;
}

//static
void AuGlobal::MemoryInfo()
{
#if !defined(Q_OS_WIN) && !defined(Q_OS_MACOS)
	qDebug() << "======================================================================";
	qDebug() << "MemoryInfo" << AuTime::Now().UTC();
	qDebug() << "======================================================================";
	/*
	struct mallinfo mi = mallinfo();
	qDebug() << "mallinfo()";
	qDebug() << " Total non-mmapped bytes (arena)" << mi.arena;
	qDebug() << " # of free chunks (ordblks)" << mi.ordblks;
	qDebug() << " # of free fastbin blocks (smblks)" << mi.smblks;
	qDebug() << " # of mapped regions (hblks)" << mi.hblks;
	qDebug() << " Bytes in mapped regions (hblkhd)" << mi.hblkhd;
	qDebug() << " Max total allocated space (usmblks)" << mi.usmblks;
	qDebug() << " Free bytes held in fastbins (fsmblks)" << mi.fsmblks;
	qDebug() << " Total allocated space (uordblks)" << mi.uordblks;
	qDebug() << " Total free space (fordblks)" << mi.fordblks;
	qDebug() << " Topmost releasable block (keepcost)" << mi.keepcost;
	qDebug() << "----------------------------------------------------------------------";
	*/
	qDebug() << "malloc_stats()";
	malloc_stats();
	qDebug() << "----------------------------------------------------------------------";
	qDebug() << "malloc_info()";
	malloc_info(0, stderr);
	qDebug() << "----------------------------------------------------------------------";
#endif
}

QString AuGlobal::CacheApp() const
{
#if !defined(QT_NO_DEBUG)
	 if(SceneMasterName() != AuUtil::SceneMasterName())
		 return m_CacheDir + "Alt";	// Non standard scene master must be the debug servers.
#endif
	return m_CacheDir;
}

QString AuGlobal::CacheGame() const
{
	// Cache during solo play must be kept separate due to atom ids.
	if(IsSceneSolo())
		return CacheSolo();	// We don't need separate solo caches in both Temporary and TemporaryAlt caches.
	return CacheApp();
}

QString AuGlobal::CacheSolo() const
{
	// Cache during solo play must be kept separate due to atom ids.
	return m_CacheDir + "/Solo";	// We don't need separate solo caches in both Temporary and TemporaryAlt caches.
}

// Reset scene server values to return to communication with the master scene server.
void AuGlobal::ResetSceneServer()
{
	m_ChatPort = 0;						// There is no default chat port.
	m_ManagerPort = 0;					// There is no default server manager port.
	m_ScenePort = AuUtil::SceneMasterPort();
	m_SceneServerMode = 0;
	m_SceneServerName = SceneMasterName();
}

QColor AuGlobal::DesignerDefaultBackColor() const
{
	return QColor(128,128,0);
}

QColor AuGlobal::DesignerDefaultGridColor() const
{
	return QColor(255,255,255);
}

QColor AuGlobal::DesignerDefaultGridXColor() const
{
	return QColor(255,64,64);
}

QColor AuGlobal::DesignerDefaultGridYColor() const
{
	return QColor(64,255,64);
}

QColor AuGlobal::DesignerDefaultGridZColor() const
{
	return QColor(64,64,255);
}

QColor AuGlobal::DesignerDefaultLineColor() const
{
	return QColor(142,204,142);
}

QColor AuGlobal::DesignerDefaultSelColor() const
{
	return QColor(255,154,51);
}

QColor AuGlobal::DesignerDefaultLadderPathColor() const
{
	return QColor(142,192,204);
}

QColor AuGlobal::DesignerDefaultWalkPathColor() const
{
	return QColor(142,152,204);
}

void AuGlobal::DecayNet1Downloading(int howMuch)
{
	m_Net1Lock.lock();
	if(m_Net1Downloading > howMuch)
		m_Net1Downloading -= howMuch;
	else
		m_Net1Downloading = 0;
	m_Net1Lock.unlock();
}

void AuGlobal::DecayNet1Uploading(int howMuch)
{
	m_Net1Lock.lock();
	if(m_Net1Uploading > howMuch)
		m_Net1Uploading -= howMuch;
	else
		m_Net1Uploading = 0;
	m_Net1Lock.unlock();
}

void AuGlobal::DecayNet2Downloading(int howMuch)
{
	m_Net2Lock.lock();
	if(m_Net2Downloading > howMuch)
		m_Net2Downloading -= howMuch;
	else
		m_Net2Downloading = 0;
	m_Net2Lock.unlock();
}

void AuGlobal::DecayNet2Uploading(int howMuch)
{
	m_Net2Lock.lock();
	if(m_Net2Uploading > howMuch)
		m_Net2Uploading -= howMuch;
	else
		m_Net2Uploading = 0;
	m_Net2Lock.unlock();
}

void AuGlobal::DecayNet3Downloading(int howMuch)
{
	m_Net3Lock.lock();
	if(m_Net3Downloading > howMuch)
		m_Net3Downloading -= howMuch;
	else
		m_Net3Downloading = 0;
	m_Net3Lock.unlock();
}

void AuGlobal::DecayNet3Uploading(int howMuch)
{
	m_Net3Lock.lock();
	if(m_Net3Uploading > howMuch)
		m_Net3Uploading -= howMuch;
	else
		m_Net3Uploading = 0;
	m_Net3Lock.unlock();
}

void AuGlobal::DecayNet4Downloading(int howMuch)
{
	m_Net4Lock.lock();
	if(m_Net4Downloading > howMuch)
		m_Net4Downloading -= howMuch;
	else
		m_Net4Downloading = 0;
	m_Net4Lock.unlock();
}

void AuGlobal::DecayNet4Uploading(int howMuch)
{
	m_Net4Lock.lock();
	if(m_Net4Uploading > howMuch)
		m_Net4Uploading -= howMuch;
	else
		m_Net4Uploading = 0;
	m_Net4Lock.unlock();
}

void AuGlobal::SetNet1Downloading(int downloading)
{
	m_Net1Lock.lock();
	m_Net1Downloading = downloading;
	m_Net1Lock.unlock();
}

void AuGlobal::SetNet1Uploading(int uploading)
{
	m_Net1Lock.lock();
	m_Net1Uploading = uploading;
	m_Net1Lock.unlock();
}

void AuGlobal::SetNet2Downloading(int downloading)
{
	m_Net2Lock.lock();
	m_Net2Downloading = downloading;
	m_Net2Lock.unlock();
}

void AuGlobal::SetNet2Uploading(int uploading)
{
	m_Net2Lock.lock();
	m_Net2Uploading = uploading;
	m_Net2Lock.unlock();
}

void AuGlobal::SetNet3Downloading(int downloading)
{
	m_Net3Lock.lock();
	m_Net3Downloading = downloading;
	m_Net3Lock.unlock();
}

void AuGlobal::SetNet3Uploading(int uploading)
{
	m_Net3Lock.lock();
	m_Net3Uploading = uploading;
	m_Net3Lock.unlock();
}

void AuGlobal::SetNet4Downloading(int downloading)
{
	m_Net4Lock.lock();
	m_Net4Downloading = downloading;
	m_Net4Lock.unlock();
}

void AuGlobal::SetNet4Uploading(int uploading)
{
	m_Net4Lock.lock();
	m_Net4Uploading = uploading;
	m_Net4Lock.unlock();
}

bool AuGlobal::InTickTockQueue(const AuId& atomId)
{
	m_AtomTickerLock.lockForRead();
	bool inQ = m_AtomTickers.find(atomId) != m_AtomTickers.cend();
	m_AtomTickerLock.unlock();
	if(inQ)
		return true;

	m_AtomTockerLock.lockForRead();
	inQ = m_AtomTockers.find(atomId) != m_AtomTockers.cend();
	m_AtomTockerLock.unlock();
	return inQ;
}

void AuGlobal::AddTicker(const AuId& atomId, quint8 atomType, const AuId& systemId)
{
	m_AtomTickerLock.lockForWrite();
	m_AtomTickers.Add(atomId, atomType, systemId);
	if(m_AtomTickerReleased)
	{
		m_AtomTickers.RemoveReleased();
		m_AtomTickerReleased = false;
	}
	m_AtomTickerLock.unlock();
}

void AuGlobal::ReleaseTicker(const AuId& atomId) const
{
	m_AtomTickerLock.lockForRead();
	if(m_AtomTickers.Release(atomId))
		m_AtomTickerReleased = true;
	m_AtomTickerLock.unlock();
}

void AuGlobal::RemoveReleasedTickers()
{
	if(!m_AtomTickerReleased)
		return;
	m_AtomTickerLock.lockForWrite();
	m_AtomTickers.RemoveReleased();
	m_AtomTickerReleased = false;
	m_AtomTickerLock.unlock();
}

/*
void AuGlobal::RemoveTicker(const AuId& atomId)
{
	// Check for presence of atom before locking for write.
	m_AtomTickerLock.lockForRead();
	bool hasAtom = m_AtomTickers.contains(atomId);
	m_AtomTickerLock.unlock();
	if(!hasAtom)
		return;

	m_AtomTickerLock.lockForWrite();
	m_AtomTickers.Remove(atomId);
	if(m_LogMessages & LOG_TickTock)
		qDebug() << "AuGlobal::RemoveTicker atom" << atomId << "tickers" << m_AtomTickers.size();
	m_AtomTickerLock.unlock();
}
*/

bool AuGlobal::UpdateTicker(const AuId& atomId, const AuId& systemId) const
{
	m_AtomTickerLock.lockForRead();
	bool updated = m_AtomTickers.Update(atomId, systemId);
	m_AtomTickerLock.unlock();
	return updated;
}

void AuGlobal::UpdateOrAddTicker(const AuId& atomId, quint8 atomType, const AuId& systemId)
{
	if(!UpdateTicker(atomId, systemId))
		AddTicker(atomId, atomType, systemId);
}

void AuGlobal::AddTocker(const AuId& atomId, quint8 atomType, const AuId& systemId, unsigned int desiredCycleMs)
{
	m_AtomTockerLock.lockForWrite();
	m_AtomTockers.Add(atomId, atomType, systemId, desiredCycleMs);
	if(m_AtomTockerReleased)
	{
		m_AtomTockers.RemoveReleased();
		m_AtomTockerReleased = false;
	}
	m_AtomTockerLock.unlock();
}

void AuGlobal::ReleaseTocker(const AuId& atomId) const
{
	m_AtomTockerLock.lockForRead();
	if(m_AtomTockers.Release(atomId))
		m_AtomTockerReleased = true;
	m_AtomTockerLock.unlock();
}

/*
void AuGlobal::RemoveReleasedTockers()
{
	if(!m_AtomTockerReleased)
		return;
	m_AtomTockerLock.lockForWrite();
	m_AtomTockers.RemoveReleased();
	m_AtomTockerReleased = false;
	m_AtomTockerLock.unlock();
}

void AuGlobal::RemoveTocker(const AuId& atomId)
{
	// Check for presence of atom before locking for write.
	m_AtomTockerLock.lockForRead();
	bool hasAtom = m_AtomTockers.contains(atomId);
	m_AtomTockerLock.unlock();
	if(!hasAtom)
		return;

	m_AtomTockerLock.lockForWrite();
	m_AtomTockers.Remove(atomId);
	if(m_LogMessages & LOG_TickTock)
		qDebug() << "AuGlobal::RemoveTocker atom" << atomId << "tockers" << m_AtomTockers.size();
	m_AtomTockerLock.unlock();
}
*/

bool AuGlobal::UpdateTocker(const AuId& atomId, const AuId& systemId, unsigned int desiredCycleMs) const
{
	m_AtomTockerLock.lockForRead();
	bool updated = m_AtomTockers.Update(atomId, systemId, desiredCycleMs);
	m_AtomTockerLock.unlock();
	return updated;
}

void AuGlobal::UpdateOrAddTocker(const AuId& atomId, quint8 atomType, const AuId& systemId, unsigned int desiredCycleMs)
{
	if(!UpdateTocker(atomId, systemId, desiredCycleMs))
		AddTocker(atomId, atomType, systemId, desiredCycleMs);
}

unsigned int AuGlobal::SumOfAverageTickTimes() const
{
	m_AtomTickerLock.lockForRead();
	unsigned int totalTime = m_AtomTickers.SumOfAverageTimes();
	m_AtomTickerLock.unlock();
	return totalTime;
}

unsigned int AuGlobal::SumOfAverageTockTimes() const
{
	m_AtomTockerLock.lockForRead();
	unsigned int totalTime = m_AtomTockers.SumOfAverageTimes();
	m_AtomTockerLock.unlock();
	return totalTime;
}

bool AuGlobal::LockTicker(const AuId& atomId)
{
	m_AtomTickerLock.lockForRead();
	bool anyLocked = m_AtomTickers.LockAtom(atomId);
	m_AtomTickerLock.unlock();
	return anyLocked;
}

bool AuGlobal::LockNextTicker(const AuId& prevAtomId, AuId& atomId, AuId& systemId)
{
	if(!m_EnableTick)
	{
		atomId = AuId(0);
		systemId = AuId(0);
		return false;
	}
	m_AtomTickerLock.lockForRead();
	bool anyLocked = m_AtomTickers.LockNextAtom(prevAtomId, atomId, systemId);
	if((m_LogMessages & LOG_TickTock) && anyLocked)
		qDebug() << "AuGlobal::LockNextTicker locked" << anyLocked << "atom" << atomId << "system" << systemId
			<< "prev" << prevAtomId << "tickers" << m_AtomTickers.size();
	m_AtomTickerLock.unlock();
	return anyLocked;
}

void AuGlobal::UnlockTicker(const AuId& atomId)
{
	/*
	m_AtomTickerLock.lockForRead();
	bool atomReleased = m_AtomTickers.UnlockAtom(atomId);
	m_AtomTickerLock.unlock();
	if(atomReleased)
		RemoveTicker(atomId);
		*/
	m_AtomTickerLock.lockForRead();
	m_AtomTickers.UnlockAtom(atomId);
	m_AtomTickerLock.unlock();
}

void AuGlobal::UnlockTicker(const AuId& atomId, unsigned int msElapsed)
{
	/*
	m_AtomTickerLock.lockForRead();
	bool atomReleased = m_AtomTickers.UnlockAtom(atomId, msElapsed);
	m_AtomTickerLock.unlock();
	if(atomReleased)
		RemoveTicker(atomId);
		*/
	m_AtomTickerLock.lockForRead();
	m_AtomTickers.UnlockAtom(atomId, msElapsed);
	m_AtomTickerLock.unlock();
}

bool AuGlobal::LockTocker(const AuId& atomId)
{
	m_AtomTockerLock.lockForRead();
	bool anyLocked = m_AtomTockers.LockAtom(atomId);
	m_AtomTockerLock.unlock();
	return anyLocked;
}

bool AuGlobal::LockNextTocker(const AuId& prevAtomId, AuId& atomId, AuId& systemId)
{
	if(!m_EnableTock)
	{
		atomId = AuId(0);
		systemId = AuId(0);
		return false;
	}
	m_AtomTockerLock.lockForRead();
	bool anyLocked = m_AtomTockers.LockNextAtom(prevAtomId, atomId, systemId);
	if((m_LogMessages & LOG_TickTock) && anyLocked)
		qDebug() << "AuGlobal::LockNextTocker locked" << anyLocked << "atom" << atomId << "system" << systemId
			<< "prev" << prevAtomId << "tockers" << m_AtomTockers.size();
	m_AtomTockerLock.unlock();
	return anyLocked;
}

void AuGlobal::UnlockTocker(const AuId& atomId)
{
	/*
	m_AtomTockerLock.lockForRead();
	bool atomReleased = m_AtomTockers.UnlockAtom(atomId);
	m_AtomTockerLock.unlock();
	if(atomReleased)
		RemoveTocker(atomId);
		*/
	m_AtomTockerLock.lockForRead();
	m_AtomTockers.UnlockAtom(atomId);
	m_AtomTockerLock.unlock();
}

void AuGlobal::UnlockTocker(const AuId& atomId, unsigned int msElapsed)
{
	/*
	m_AtomTockerLock.lockForRead();
	bool atomReleased = m_AtomTockers.UnlockAtom(atomId, msElapsed);
	m_AtomTockerLock.unlock();
	if(atomReleased)
		RemoveTocker(atomId);
		*/
	m_AtomTockerLock.lockForRead();
	m_AtomTockers.UnlockAtom(atomId, msElapsed);
	m_AtomTockerLock.unlock();
}

void AuGlobal::TallyAtomTickerSystemTimes(QHash<AuId, unsigned int>& systemTotals,
		QHash<AuId, unsigned int>& systemTotalsCount,
		QHash<AuId, unsigned int>& systemMax,
		QHash<AuId, unsigned int>& systemMaxCount,
		QHash<AuId, unsigned int>& systemCycle,
		QHash<AuId, unsigned int>& systemCycleCount,
		QHash<AuId, unsigned int>& systemCycleMax,
		QHash<AuId, unsigned int>& systemCycleMaxCount) const
{
	m_AtomTickerLock.lockForRead();
	m_AtomTickers.TallySystemTimes(systemTotals, systemTotalsCount, systemMax, systemMaxCount, systemCycle, systemCycleCount,
		systemCycleMax, systemCycleMaxCount);
	m_AtomTickerLock.unlock();
}

void AuGlobal::TallyAtomTockerSystemTimes(QHash<AuId, unsigned int>& systemTotals,
		QHash<AuId, unsigned int>& systemTotalsCount,
		QHash<AuId, unsigned int>& systemMax,
		QHash<AuId, unsigned int>& systemMaxCount) const
{
	m_AtomTockerLock.lockForRead();
	m_AtomTockers.TallySystemTimes(systemTotals, systemTotalsCount, systemMax, systemMaxCount);
	m_AtomTockerLock.unlock();
}

void AuGlobal::TallyAtomTockerSystemTimes(QHash<AuId, unsigned int>& systemTotals,
		QHash<AuId, unsigned int>& systemMax) const
{
	m_AtomTockerLock.lockForRead();
	m_AtomTockers.TallySystemTimes(systemTotals, systemMax);
	m_AtomTockerLock.unlock();
}

void AuGlobal::TallyAtomTockerSystemStatus(QHash<AuId, unsigned int>& systemTotals) const
{
	m_AtomTockerLock.lockForRead();
	m_AtomTockers.TallySystemStatus(systemTotals);
	m_AtomTockerLock.unlock();
}

const char* AuGlobal::Address() const
{
	return "Hazeron Freehold LLC\n"
		"3900 N. 37th St.\n"
		"Norfolk, NE 68701\n"
		"402-379-0181\n"
		"Shores@Hazeron.com";
}

QString AuGlobal::CommodityName(COMMODITYID cid) const
{
	if(cid >= CID_Invalid && cid < m_CommodityArray.count())
		return Commodity(cid).m_name;
	else
	{
		switch(cid)
		{
		default:
			return QObject::tr("Unknown:%1").arg(cid);
		case CID_GivingBirth:
			return QObject::tr("Giving Birth");
		case CID_Collision:
			return QObject::tr("Collision");
		case CID_Spaghettification:
			return QObject::tr("Spaghettification");
		case CID_Fire:
			return QObject::tr("Fire");
		case CID_Acid:
			return QObject::tr("Acid");
		case CID_MedicalProcedure:
			return QObject::tr("Medical Procedure");
		case CID_ImplantBirth:
			return QObject::tr("Parasite Birth");
		case CID_Poison:
			return QObject::tr("Poison");
		case CID_Disease:
			return QObject::tr("Disease");
		case CID_Eaten:
			return QObject::tr("Eaten By Animal");
		case CID_ShipCapture:
			return QObject::tr("Ship Capture");
		case CID_Wormhole:
			return QObject::tr("Wormhole Entry");
		case CID_Starvation:
			return QObject::tr("Starvation");
		case CID_InternalInjuries:
			return QObject::tr("Internal Injuries");
		case CID_BaseWeapon:
			return QObject::tr("Main Weapon at Base");
		case CID_BaseTurret:
			return QObject::tr("Turret at Base");
		case CID_SunBurn:
			return QObject::tr("Sun Burn");
		case CID_ShipTurret:
			return QObject::tr("Turret on Spacecraft");
		case CID_BayWeapon:
			return QObject::tr("Bay Weapon");
		case CID_Mount_Anus:
			return QObject::tr("Anus of Mount");
		case CID_Mount_Eyes:
			return QObject::tr("Eyes of Mount");
		case CID_Mount_Feet:
			return QObject::tr("Feet of Mount");
		case CID_Mount_Hands:
			return QObject::tr("Hands of Mount");
		case CID_Mount_Head:
			return QObject::tr("Head of Mount");
		case CID_Mount_Tail:
			return QObject::tr("Tail of Mount");
		case CID_Mount_Torso:
			return QObject::tr("Torso of Mount");
		case CID_My_Anus:
			return QObject::tr("Anus");
		case CID_My_Eyes:
			return QObject::tr("Eyes");
		case CID_My_Feet:
			return QObject::tr("Feet");
		case CID_My_Hands:
			return QObject::tr("Hands");
		case CID_My_Head:
			return QObject::tr("Head");
		case CID_My_Tail:
			return QObject::tr("Tail");
		case CID_My_Torso:
			return QObject::tr("Torso");
		}
	}
}

const QString& AuGlobal::TypefaceSans() const
{
	if(m_TypefaceSans.isEmpty())
	{
//#if !defined(QT_NO_DEBUG)
		//QFontDatabase fontDb;
		//QStringList fontFamilies = fontDb.families();
		//qDebug() << fontFamilies;
//#endif
		QString defaultFont;
#if defined(Q_OS_WIN)
		defaultFont = QString("Arial");
#elif defined(Q_OS_MACOS)
		defaultFont = QString("Helvetica");
#else
		defaultFont = QString("FreeSans");
#endif
		m_TypefaceSans = m_Settings.readEntry("/Settings/TypefaceSans", defaultFont);
	}
	return m_TypefaceSans;
}

const QString& AuGlobal::TypefaceSerif() const
{
	if(m_TypefaceSerif.isEmpty())
	{
		QString defaultFont;
#if defined(Q_OS_WIN)
		defaultFont = QString("Times New Roman");
#elif defined(Q_OS_MACOS)
		defaultFont = QString("Baskerville");
#else
		defaultFont = QString("FreeSerif");
#endif
		m_TypefaceSerif = m_Settings.readEntry("/Settings/TypefaceSerif", defaultFont);
	}
	return m_TypefaceSerif;
}

//static
QString AuGlobal::FormatArmorFactors(float absorb, float block)
{
	// 'absorb' value is a fraction from 0 to 1. Convert to an integer % value from 0 to 100, rounding up.
	int ai = (int)ceilf(absorb * 100.f);
	if(ai > 100)
		ai = 100;

	// 'block' value is a number of hit points from 0 to n. Convert to an integer, rounding up.
	int bi = (int)ceilf(block);

	// Return a string that looks like '5%+20hp', for absorb 5% block 20hp.
	return QVariant(ai).toString() + "%+" + QVariant(bi).toString() + "hp";
}

//static
QString AuGlobal::FormatMoney(qint32 amount)
{
	return QString("%L1%2").arg(amount).arg(CurrencySymbol());
}

//static
QString AuGlobal::FormatMoney(quint32 amount)
{
	return QString("%L1%2").arg(amount).arg(CurrencySymbol());
}

//static
QString AuGlobal::FormatMoney(double amount, int decimals)
{
	return QString("%L1%2").arg(amount, 0,'f',decimals).arg(CurrencySymbol());
}

bool AuGlobal::UseNativeColorDialog() const
{
	return m_UseNativeColorDialog;
}

bool AuGlobal::UseNativeFileDialog() const
{
	return m_UseNativeFileDialog;
}

bool AuGlobal::UseNativeFontDialog() const
{
	return m_UseNativeFontDialog;
}

void AuGlobal::SetBumpMapSeas(bool bumpMapSeas)
{
	m_BumpMapSeas = bumpMapSeas;
	m_Settings.writeEntry("/Settings/BumpMapSeas", m_BumpMapSeas);
}

void AuGlobal::SetRating(RATING rating)
{
	if(rating < Everyone || rating > Adult)
		return;
	m_Rating = rating;
	m_Settings.writeEntry("/Settings/Rating", m_Rating);
}

void AuGlobal::SetChatPort(quint16 chatPort)
{
	m_ChatPort = chatPort;
}

void AuGlobal::SetMode(MODE mode)
{
	// Mode change can affect my number of worker threads.
	int workerThreads = WorkerThreadPoolCount();
	m_Mode = mode;
	if(workerThreads != WorkerThreadPoolCount())
	{
		QThreadPool::globalInstance()->setMaxThreadCount(WorkerThreadPoolCount());
		//qDebug() << "AuGlobal::SetMode worker thread pool now" << WorkerThreadPoolCount();
	}
}

void AuGlobal::SetSoundEffectsGain(float gain)
{
	m_Settings.writeEntry(QString("/Sound/Volume"), gain);
	if(gain >= 0 && gain <= 1)
		m_SoundEffectsGain = gain;
}

void AuGlobal::SetSoundGain(int sound, float gain)
{
	m_Settings.writeEntry(QString("/Sound/Sound%1/Volume").arg(sound), gain);
	if(gain >= 0 && gain <= 2 && gain != 1)
		m_SoundGain.insert(sound, gain);
	else
		m_SoundGain.remove(sound);
}

void AuGlobal::SetSoundOverride(int sound, const AuSoundOverride& sndOver)
{
	m_Settings.writeEntry(QString("/Sound/Sound%1/File").arg(sound), sndOver.FileName());
	m_Settings.writeEntry(QString("/Sound/Sound%1/RefDist").arg(sound), sndOver.ReferenceDistance()/FeetPerMeter());
	m_Settings.writeEntry(QString("/Sound/Sound%1/Rolloff").arg(sound), sndOver.RolloffFactor());
	if(sndOver != AuSound::DefaultSoundOverride((SOUND_EFFECT)sound))
		m_SoundOverrides.insert(sound, sndOver);
	else
		m_SoundOverrides.remove(sound);
}

void AuGlobal::SetClientChannelServer(const QString& serverName)
{
	m_ClientChannelServer = serverName;
}

void AuGlobal::SetControlAverageSpanMs(quint32 controlAverageSpanMs)
{
	if(controlAverageSpanMs > 500)
		controlAverageSpanMs = 500;
	else
		m_ControlAverageSpanMs = controlAverageSpanMs;
	m_Settings.writeEntry("/Settings/ControlSmoothing", m_ControlAverageSpanMs);
}

void AuGlobal::SetControlDecayMargin(quint32 controlDecayMargin)
{
	if(controlDecayMargin > 2000)
		controlDecayMargin = 2000;
	else
		m_ControlDecayMargin = controlDecayMargin;
	m_Settings.writeEntry("/Settings/ControlDecayMs", m_ControlDecayMargin);
}

void AuGlobal::SetDetailedTerrainRange(int detailedTerrainRange)
{
	if(detailedTerrainRange < 200)
		m_DetailedTerrainRange = 200;
	else if(detailedTerrainRange > 2000)
		m_DetailedTerrainRange = 2000;
	else
		m_DetailedTerrainRange = detailedTerrainRange;

	// Calculate feet values.
	m_DetailedTerrainRangeFt = m_DetailedTerrainRange * FeetPerMeter();
	m_DetailedTerrainRangeFt2 = m_DetailedTerrainRangeFt * m_DetailedTerrainRangeFt;

	m_Settings.writeEntry("/Settings/DetailedTerrainRange", m_DetailedTerrainRange);
}

void AuGlobal::SetGrassViewRange(int viewRange)
{
	if(viewRange < 10)
		m_GrassViewRange = 10;
	else if(viewRange > 200)
		m_GrassViewRange = 200;
	else
		m_GrassViewRange = viewRange;

	// Calculate feet values.
	m_GrassViewRangeFt = m_GrassViewRange * FeetPerMeter();
	m_GrassViewRangeFt2 = m_GrassViewRangeFt * m_GrassViewRangeFt;

	m_Settings.writeEntry("/Settings/GrassViewRange", m_GrassViewRange);
}

void AuGlobal::SetLeftRightCursor(int value)
{
	if(value < 0 || value > 2)
		value = 0;

	m_LeftRightCursor = value;
	m_Settings.writeEntry("/Settings/LeftRightCursor", m_LeftRightCursor);
}

void AuGlobal::SetLeftRightOrientation(int value)
{
	if(value < 0 || value > 2)
		value = 0;

	m_LeftRightOrientation = value;
	m_Settings.writeEntry("/Settings/LeftRightOrientation", m_LeftRightOrientation);
}

void AuGlobal::SetMailCityReportAutoDelete(bool value)
{
	m_MailCityReportAutoDelete = value;
	m_Settings.writeEntry("/Settings/MailCityReportAutoDelete", m_MailCityReportAutoDelete);
}

void AuGlobal::SetMailCityReportOldAge(int value)
{
	if(value < 1)
		m_MailCityReportOldAge = 1;
	else if(value > 100)
		m_MailCityReportOldAge = 100;
	else
		m_MailCityReportOldAge = value;
	m_Settings.writeEntry("/Settings/MailCityReportOldAge", m_MailCityReportOldAge);
}

void AuGlobal::SetMailFleetReportAutoDelete(bool value)
{
	m_MailFleetReportAutoDelete = value;
	m_Settings.writeEntry("/Settings/MailFleetReportAutoDelete", m_MailFleetReportAutoDelete);
}

void AuGlobal::SetMailFleetReportOldAge(int value)
{
	if(value < 1)
		m_MailFleetReportOldAge = 1;
	else if(value > 100)
		m_MailFleetReportOldAge = 100;
	else
		m_MailFleetReportOldAge = value;
	m_Settings.writeEntry("/Settings/MailFleetReportOldAge", m_MailFleetReportOldAge);
}

void AuGlobal::SetMailNotificationMode(int value)
{
	if(value < 0)
		m_MailNotificationMode = 0;
	else if(value > 2)
		m_MailNotificationMode = 2;
	else
		m_MailNotificationMode = value;
	m_Settings.writeEntry("/Settings/MailNotificationMode", m_MailNotificationMode);
}

void AuGlobal::SetMailOfficerReportAutoDelete(bool value)
{
	m_MailOfficerReportAutoDelete = value;
	m_Settings.writeEntry("/Settings/MailOfficerReportAutoDelete", m_MailOfficerReportAutoDelete);
}

void AuGlobal::SetMailOfficerReportOldAge(int value)
{
	if(value < 1)
		m_MailOfficerReportOldAge = 1;
	else if(value > 100)
		m_MailOfficerReportOldAge = 100;
	else
		m_MailOfficerReportOldAge = value;
	m_Settings.writeEntry("/Settings/MailOfficerReportOldAge", m_MailOfficerReportOldAge);
}

void AuGlobal::SetRockViewRange(int viewRange)
{
	if(viewRange < 10)
		m_RockViewRange = 10;
	else if(viewRange > 500)
		m_RockViewRange = 500;
	else
		m_RockViewRange = viewRange;

	// Calculate feet values.
	m_RockViewRangeFt = m_RockViewRange * FeetPerMeter();
	m_RockViewRangeFt2 = m_RockViewRangeFt * m_RockViewRangeFt;

	m_Settings.writeEntry("/Settings/RockViewRange", m_RockViewRange);
}

void AuGlobal::SetShrubViewRange(int viewRange)
{
	if(viewRange < 50)
		m_ShrubViewRange = 50;
	else if(viewRange > 500)
		m_ShrubViewRange = 500;
	else
		m_ShrubViewRange = viewRange;

	// Calculate feet values.
	m_ShrubViewRangeFt = m_ShrubViewRange * FeetPerMeter();
	m_ShrubViewRangeFt2 = m_ShrubViewRangeFt * m_ShrubViewRangeFt;

	m_Settings.writeEntry("/Settings/ShrubViewRange", m_ShrubViewRange);
}

void AuGlobal::SetBuildingViewRange(int viewRange)
{
	if(viewRange < 200)
		m_BuildingViewRange = 200;
	else if(viewRange > 20000)
		m_BuildingViewRange = 20000;
	else
		m_BuildingViewRange = viewRange;

	// Calculate feet values.
	m_BuildingViewRangeFt = m_BuildingViewRange * FeetPerMeter();
	m_BuildingViewRangeFt2 = (quint64)m_BuildingViewRangeFt * (quint64)m_BuildingViewRangeFt;

	m_Settings.writeEntry("/Settings/BuildingViewRange", m_BuildingViewRange);
}

void AuGlobal::SetTreeViewRange(int viewRange)
{
	if(viewRange < 100)
		m_TreeViewRange = 100;
	else if(viewRange > 1000)
		m_TreeViewRange = 1000;
	else
		m_TreeViewRange = viewRange;

	// Calculate feet values.
	m_TreeViewRangeFt = m_TreeViewRange * FeetPerMeter();
	m_TreeViewRangeFt2 = m_TreeViewRangeFt * m_TreeViewRangeFt;

	m_Settings.writeEntry("/Settings/TreeViewRange", m_TreeViewRange);
}

void AuGlobal::SetDivTexels(int divTexels)
{
	if(divTexels < 1)
		m_DivTexels = 1;
	else if(divTexels > 3)
		m_DivTexels = 3;
	else
		m_DivTexels = divTexels;
	m_Settings.writeEntry("/Settings/DivTexels", m_DivTexels);
}

void AuGlobal::SetEnableJoystick(int joyIndex, bool enableJoystick)
{
	Q_ASSERT(joyIndex >= 0);
	Q_ASSERT(joyIndex < 6);
	m_EnableJoystick[joyIndex] = enableJoystick;
	m_Settings.writeEntry(QString("/Settings/EnableJoystick%1").arg(joyIndex), m_EnableJoystick[joyIndex]);
}

void AuGlobal::SetFontAtom(const QFont& font)
{
	Q_ASSERT(font.pixelSize());
	m_FontAtom = font;
	Settings().writeEntry("Settings/FontAtom", m_FontAtom);
}

void AuGlobal::SetFontDesigner(const QFont& font)
{
	Q_ASSERT(font.pixelSize());
	m_FontDesigner = font;
	Settings().writeEntry("Settings/FontDesigner", m_FontDesigner);
}

void AuGlobal::SetInternetName(const QString& netName)
{
	m_NetName = netName;
}

void AuGlobal::SetManagerPort(quint16 mgrPort)
{
	m_ManagerPort = mgrPort;
}

void AuGlobal::SetTypefaceSans(const QString& fontName)
{
	m_TypefaceSans = fontName;
	Settings().writeEntry("Settings/TypefaceSans", m_TypefaceSans);
}

void AuGlobal::SetTypefaceSerif(const QString& fontName)
{
	m_TypefaceSerif = fontName;
	Settings().writeEntry("Settings/TypefaceSerif", m_TypefaceSerif);
}

void AuGlobal::SetUdpDecayMargin(quint32 udpDecayMargin)
{
	if(udpDecayMargin > 2000)
		udpDecayMargin = 2000;
	else
		m_UdpDecayMargin = udpDecayMargin;
	m_Settings.writeEntry("/Settings/UdpDecayMs", m_UdpDecayMargin);
}

void AuGlobal::SetLoginMaster(const QString& serverNameOrIP)
{
	Q_ASSERT(!serverNameOrIP.isEmpty());
	m_LoginMasterName = serverNameOrIP;
}

void AuGlobal::SetMailMaster(const QString& serverNameOrIP)
{
	Q_ASSERT(!serverNameOrIP.isEmpty());
	m_MailMasterName = serverNameOrIP;
}

void AuGlobal::SetSceneMaster(const QString& serverNameOrIP)
{
	Q_ASSERT(!serverNameOrIP.isEmpty());
	m_SceneMasterName = serverNameOrIP;
	m_SceneServerName = serverNameOrIP;
}

void AuGlobal::SetUseNativeColorDialog(bool value)
{
	m_UseNativeColorDialog = value;
	m_Settings.writeEntry("/Settings/UseNativeColorDialog", m_UseNativeColorDialog);
}

void AuGlobal::SetUseNativeFileDialog(bool value)
{
	m_UseNativeFileDialog = value;
	m_Settings.writeEntry("/Settings/UseNativeFileDialog", m_UseNativeFileDialog);
}

void AuGlobal::SetUseNativeFontDialog(bool value)
{
	m_UseNativeFontDialog = value;
	m_Settings.writeEntry("/Settings/UseNativeFontDialog", m_UseNativeFontDialog);
}

void AuGlobal::SetDesignerBackColor(const QColor& col)
{
	m_DesignerBackColor = col;
	m_Settings.writeEntry("/Settings/DesignerBackColor", m_DesignerBackColor);
}

void AuGlobal::SetDesignerGridColor(const QColor& col)
{
	m_DesignerGridColor = col;
	m_Settings.writeEntry("/Settings/DesignerGridColor", m_DesignerGridColor);
}

void AuGlobal::SetDesignerGridXColor(const QColor& col)
{
	m_DesignerGridXColor = col;
	m_Settings.writeEntry("/Settings/DesignerGridXColor", m_DesignerGridXColor);
}

void AuGlobal::SetDesignerGridYColor(const QColor& col)
{
	m_DesignerGridYColor = col;
	m_Settings.writeEntry("/Settings/DesignerGridYColor", m_DesignerGridYColor);
}

void AuGlobal::SetDesignerGridZColor(const QColor& col)
{
	m_DesignerGridZColor = col;
	m_Settings.writeEntry("/Settings/DesignerGridZColor", m_DesignerGridZColor);
}

void AuGlobal::SetDesignerLadderPathColor(const QColor& col)
{
	m_DesignerLadderColor = col;
	m_Settings.writeEntry("/Settings/DesignerLadderPathColor", m_DesignerLadderColor);
}

void AuGlobal::SetDesignerLineColor(const QColor& col)
{
	m_DesignerLineColor = col;
	m_Settings.writeEntry("/Settings/DesignerLineColor", m_DesignerLineColor);
}

void AuGlobal::SetDesignerSelColor(const QColor& col)
{
	m_DesignerSelColor = col;
	m_Settings.writeEntry("/Settings/DesignerSelColor", m_DesignerSelColor);
}

void AuGlobal::SetDesignerVertexSize(int vertexSize)
{
	if(vertexSize <= 6)
		m_DesignerVertexSize = 6;
	else if(vertexSize >= 60)
		m_DesignerVertexSize = 60;
	else
		m_DesignerVertexSize = vertexSize;
	m_Settings.writeEntry("/Settings/DesignerVertexSize", m_DesignerVertexSize);
}

void AuGlobal::SetDesignerWalkPathColor(const QColor& col)
{
	m_DesignerWalkColor = col;
	m_Settings.writeEntry("/Settings/DesignerWalkPathColor", m_DesignerWalkColor);
}

void AuGlobal::SetGroundCoverQty(int groundCoverQty)
{
	if(groundCoverQty <= 0)
		m_GroundCoverQty = 0;
	else if(groundCoverQty >= 1000)
		m_GroundCoverQty = 1000;
	else
		m_GroundCoverQty = groundCoverQty;
	m_Settings.writeEntry("/Settings/GroundCoverQty", m_GroundCoverQty);
}

void AuGlobal::SetMaxLights(unsigned int maxLights)
{
	if(maxLights < 3)
		m_MaxLights = 3;
	else if(maxLights > MAX_POINT_LIGHTS)
		m_MaxLights = MAX_POINT_LIGHTS;
	else
		m_MaxLights = maxLights;
	m_Settings.writeEntry("/Settings/MaxLights", m_MaxLights);
}

void AuGlobal::SetGroundTextureSize(int texSize)
{
	if(texSize <= 32)
		m_GroundTextureSize = 32;
	else if(texSize >= 256)
		m_GroundTextureSize = 256;
	else
		m_GroundTextureSize = texSize;
	m_Settings.writeEntry("/Settings/GroundTextureSize", m_GroundTextureSize);
}

void AuGlobal::SetPlantBranchMaxSegs(int maxSegs)
{
	if(maxSegs <= 1)
		m_PlantBranchMaxSegs = 1;
	else
		m_PlantBranchMaxSegs = maxSegs;
	m_Settings.writeEntry("/Settings/PlantBranchMaxSegs", m_PlantBranchMaxSegs);
}

void AuGlobal::SetPlantTextureSize(int texSize)
{
	if(texSize <= 32)
		m_PlantTextureSize = 32;
	else if(texSize >= 512)
		m_PlantTextureSize = 512;
	else
		m_PlantTextureSize = texSize;
	m_Settings.writeEntry("/Settings/PlantTextureSize", m_PlantTextureSize);
}

void AuGlobal::SetTreeTrunkMinDia(float minDia)
{
	if(minDia <= 0)
		m_TreeTrunkMinDia = 0;
	else
		m_TreeTrunkMinDia = minDia;
	m_Settings.writeEntry("/Settings/TreeTrunkMinDia", m_TreeTrunkMinDia);
}

void AuGlobal::SetTreeTrunkFaces(int faces)
{
	if(faces <= 3)
		m_TreeTrunkFaces = 3;
	else
		m_TreeTrunkFaces = faces;
	m_Settings.writeEntry("/Settings/TreeTrunkFaces", m_TreeTrunkFaces);
}

void AuGlobal::SetTreeTrunkMaxSegs(int maxSegs)
{
	if(maxSegs <= 1)
		m_TreeTrunkMaxSegs = 1;
	else
		m_TreeTrunkMaxSegs = maxSegs;
	m_Settings.writeEntry("/Settings/TreeTrunkMaxSegs", m_TreeTrunkMaxSegs);
}

void AuGlobal::SetSceneServer(const QString& serverName, quint16 serverPort, unsigned int serverMode)
{
	m_SceneServerName = serverName;
	m_ScenePort = serverPort;
	m_SceneServerMode = serverMode;
}

void AuGlobal::SetCacheApp(const QString& dir)
{
	m_CacheDir = dir;
	m_Settings.writeEntry("/Settings/CacheDir", m_CacheDir);
}

void AuGlobal::SetGameSaveDir(const QString& dir)
{
	m_GameSaveDir = dir;
	m_Settings.writeEntry("/Settings/GameSaveDir", m_GameSaveDir);
}

void AuGlobal::SetPictureDir(const QString& dir)
{
	m_PictureDir = dir;
	m_Settings.writeEntry("/Settings/PictureDir", m_PictureDir);
}

void AuGlobal::SetBaseDir(const QString& dir)
{
	m_BaseDir = dir;
	m_Settings.writeEntry("/Settings/BaseDir", m_BaseDir);
}

void AuGlobal::SetBodyStatSize(int bodyStatSize)
{
	m_BodyStatSize = bodyStatSize;
	m_Settings.writeEntry("/Settings/BodyStatSize", m_BodyStatSize);
}

void AuGlobal::SetDrawBodyModel(int drawBody)
{
	m_DrawBody = drawBody;
	m_Settings.writeEntry("/Settings/DrawBodyModel", m_DrawBody);
}

void AuGlobal::SetDrawClouds(int drawClouds)
{
	m_DrawClouds = drawClouds;
	if(m_DrawClouds < 0 || m_DrawClouds > 3)
		m_DrawClouds = 3;
	m_Settings.writeEntry("/Settings/DrawClouds", m_DrawClouds);
}

void AuGlobal::SetDrawFramedPictures(bool drawFramedPictures)
{
	m_DrawFramedPictures = drawFramedPictures;
	m_Settings.writeEntry("/Settings/DrawFramedPictures", m_DrawFramedPictures);
}

void AuGlobal::SetDrawGun(bool drawGun)
{
	m_DrawGun = drawGun;
	m_Settings.writeEntry("/Settings/DrawGun", m_DrawGun);
}

void AuGlobal::SetDrawHelmetHUDAlways(bool drawHelmetHUDAlways)
{
	m_DrawHelmetHUDAlways = drawHelmetHUDAlways;
	m_Settings.writeEntry("/Settings/DrawHelmetHUDAlways", m_DrawHelmetHUDAlways);
}

void AuGlobal::SetDrawMiniMe(bool drawMiniMe)
{
	m_DrawMiniMe = drawMiniMe;
	m_Settings.writeEntry("/Settings/DrawMiniMe", m_DrawMiniMe);
}

void AuGlobal::SetDrawNebulaCloud(bool drawNebulaCloud)
{
	m_DrawNebulaCloud = drawNebulaCloud;
	m_Settings.writeEntry("/Settings/DrawNebulaCloud", m_DrawNebulaCloud);
}

void AuGlobal::SetDrawSpecks(bool drawNebulaCloud)
{
	m_DrawSpecks = drawNebulaCloud;
	m_Settings.writeEntry("/Settings/DrawSpecks", m_DrawSpecks);
}

void AuGlobal::SetDrawSkyBox(bool drawSkyBox)
{
	m_DrawSkyBox = drawSkyBox;
	m_Settings.writeEntry("/Settings/DrawSkyBox", m_DrawSkyBox);
}

void AuGlobal::SetSkyBoxMipMap(bool mipMap)
{
	m_SkyBoxMipMap = mipMap;
	m_Settings.writeEntry("/Settings/SkyBoxMipMap", m_SkyBoxMipMap);
}

void AuGlobal::SetSkyBoxPaneSize(unsigned int paneSize)
{
	m_SkyBoxPaneSize = paneSize;
	m_Settings.writeEntry("/Settings/SkyBoxSize", m_SkyBoxPaneSize);
}

void AuGlobal::SetDrawWeatherEffects(bool drawWeatherEffects)
{
	m_DrawWeatherEffects = drawWeatherEffects;
	m_Settings.writeEntry("/Settings/DrawWeatherEffects", m_DrawWeatherEffects);
}

void AuGlobal::SetFramedPictureSize(int framedPictureSize)
{
	m_FramedPictureSize = framedPictureSize;
	if(m_FramedPictureSize < 64 || m_FramedPictureSize > 2048)
		m_FramedPictureSize = 512;
	m_Settings.writeEntry("/Settings/FramedPictureSize", m_FramedPictureSize);
}

void AuGlobal::SetResourceIconSize(int resourceIconSize)
{
	m_ResourceIconSize = resourceIconSize;
	if(m_ResourceIconSize < 1 || m_ResourceIconSize > 200)
		m_ResourceIconSize = 100;
	m_Settings.writeEntry("/Settings/ResourceIconSize", m_ResourceIconSize);
}

void AuGlobal::SetWindowOpacity(bool active, unsigned int percent)
{
	if(percent > 100)
		percent = 100;
	if(active)
	{
		m_OpacityActive = percent;
		m_Settings.writeEntry("/Settings/OpacityActive", (int)percent);
	}
	else
	{
		m_OpacityInactive = percent;
		m_Settings.writeEntry("/Settings/OpacityInactive", (int)percent);
	}
}

void AuGlobal::SetWorkerThreadCount(int threadCount)
{
	m_WorkerThreadCount = threadCount;
	m_Settings.writeEntry("/Settings/WorkerThreadCount", threadCount);

	// Configure the number of workers in the global thread pool.
	QThreadPool::globalInstance()->setMaxThreadCount(WorkerThreadPoolCount());
	//if(m_LogMessages & LOG_Debug)
		//Log(QObject::tr("L AuGlobal::SetWorkerThreadCount %L1").arg(WorkerThreadPoolCount()));
}

void AuGlobal::SetFootOffset(const Au3dsGLSLDNA& dna, const AuPoint& footOffset)
{
	m_rwModels.lockForWrite();
	m_FootOffset.insert(dna, footOffset);
	m_rwModels.unlock();
}

void AuGlobal::SetHandOffset(Au3dsGLSLDNA dna, AuPoint handOffset)
{
	m_rwModels.lockForWrite();
	m_HandOffset.insert(dna, handOffset);
	m_rwModels.unlock();
}

void AuGlobal::ClearCacheFiles(int which)
{
	// Clear all the cache files.
	QStringList nameFilters;
	nameFilters << "SP[0-9]*";				// Spacecraft paths
	nameFilters << "F[0-9]*Z[0-9]*";		// Flora
	nameFilters << "FG[0-9]*Z[0-9]*";		// Grass
	nameFilters << "D[0-9]*";				// Spacecraft designs
	nameFilters << "S[0-9]*";				// Spacecraft models
	nameFilters << "T[0-9]*";				// Terrain patch models (obsolete)
	nameFilters << "WG[AFN][0-9]*[0-9]";	// World at, flat and near models.
	nameFilters << "WTF[0-9]*[0-9]";		// World texture far
	nameFilters << "B[0-9]*";				// Building designs
	nameFilters << "B[EI][0-9]*";			// Building models
	nameFilters << "BP[0-9]*";				// Building paths
	nameFilters << "BD[0-9]*";				// Bd designs
	nameFilters << "IT[0-9]*";				// IT designs
	nameFilters << "GD1";					// Game data
	nameFilters << "SS";					// Solar systems cache for star map.
	nameFilters << "BdX[0-9]*";				// Building blueprint exchange entries.
	nameFilters << "ScX[0-9]*";				// Spacecraft blueprint exchange entries.
	if(which == 1)
		nameFilters << "Transcript.*";		// Solo comm transcripts are trash.

	QDir tempDir;
	if(which == 1)
		tempDir = QDir(CacheSolo());
	else
		tempDir = QDir(CacheApp());

	QStringList delFiles = tempDir.entryList(nameFilters);
	for(QStringList::const_iterator dfi = delFiles.constBegin(); dfi != delFiles.constEnd(); dfi++)
		tempDir.remove(*dfi);
}

void AuGlobal::SetFieldOfViewY(double fovy)
{
	m_FieldOfViewY = fovy;
	if(m_FieldOfViewY < 40*DEG2RAD || m_FieldOfViewY > 90*DEG2RAD)
		m_FieldOfViewY = 60*DEG2RAD;
	m_Settings.writeEntry("/Settings/FieldOfViewY", m_FieldOfViewY*RAD2DEG);
}

bool AuGlobal::OpenLogFile(const QString& fileName)
{
	if(m_LogFile)
		fclose(m_LogFile);
	m_LogFile = fopen(fileName.toLocal8Bit().constData(), "w");
	return m_LogFile;
}

bool AuGlobal::OpenLogConnectFile(const QString& fileName)
{
	if(m_LogConnectFile)
		fclose(m_LogConnectFile);
	m_LogConnectFile = fopen(fileName.toLocal8Bit().constData(), "w");
	return m_LogConnectFile;
}

void AuGlobal::ReleaseTickTock(const AuId& atomId) const
{
	ReleaseTicker(atomId);
	ReleaseTocker(atomId);
}

void AuGlobal::LoadDefaultEmojis()
{
	m_Emojis.LoadStandardEmojis();
}

void AuGlobal::StoreEmojis(QByteArray& ba) const
{
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	m_Emojis.StoreTags(out);
	m_Emojis.StoreVisages(out);
}

void AuGlobal::LoadEmojis(const QByteArray& ba)
{
	QDataStream in(ba);
	in.setVersion(QDataStream::Qt_4_1);
	m_Emojis.LoadTags(in);
	m_Emojis.LoadVisages(in);
}

void AuGlobal::SetEmojis(const AuEmojiList& emojis)
{
	m_Emojis = emojis;

	QByteArray ba;
	StoreEmojis(ba);
	Settings().writeEntry("/Settings/Emojis", ba);
}

QString AuGlobal::ReplaceEmojiTags(const QString& raw) const
{
	return m_Emojis.ReplaceTags(raw);
}

QStringList AuGlobal::SplitWordsAndEmojis(const QString& textWithEmojis) const
{
	return m_Emojis.SplitWords(textWithEmojis);
}

void AuGlobal::DecayArenas()
{
	m_ArenaLock.lockForWrite();
	unordered_map<AuId, AuArenaReady, AuIdHash>::iterator ai = m_Arenas.begin();
	while(ai != m_Arenas.end())
	{
		if((*ai).second.IsExpired())
			ai = m_Arenas.erase(ai);
		else
			ai++;
	}
	m_ArenaLock.unlock();
}

QString AuGlobal::CacheMail() const
{
	QString mailPath;
    if(IsSceneSolo())
        mailPath = QDir::homePath() + "/Hazeron Starship/Mail";		// There is only one solo app that caches mail.
    else
	{
        mailPath = QDir::homePath() + "/Shores of Hazeron/Mail";	// Can't use ApplicationName() because SoH and SoH Mail apps.
		if(MailMasterName() != AuUtil::MailMasterName())
			mailPath += "Alt";										// Debug servers in use.
	}
	return mailPath;
}

int AuGlobal::AtomManagerCount() const
{
	// Each atom manager is a bundle of two threads so the ideal number of atom managers is probably close to 1/2 the number of
	// threads available.
	// This is the solo client so we must also allow for ModelMakerThreads. We can't use up all available threads on atom managers.
	if(!IsSceneSolo())
		return 0;	// Only scene solo uses atom managers. Server does not call this function.
	int workerThreads = WorkerThreadCount() / 3;
	if(workerThreads < 1)
		workerThreads = 1;
	return workerThreads;
}

int AuGlobal::WorkerThreadPoolCount() const
{
	int workerThreads = WorkerThreadCount();
	int atomManagerThreads = AtomManagerCount() * 2;	// Each atom manager is a bundle of two threads.
	if(atomManagerThreads >= workerThreads)
		return 1;
	return workerThreads - atomManagerThreads;
}

QString AuGlobal::GameName() const
{
	if(IsSceneApp())
		return m_AppName;
	else
		return QString("Shores of Hazeron");
}

//static
QString AuGlobal::RatingDescription(RATING rating)
{
	QString desc;
	switch(rating)
	{
	case Everyone:
		desc = QObject::tr("Everyone - No mating. No nudity.");
		break;
	case Teen:
		desc = QObject::tr("Teen - No mating. Nudity ok.");
		break;
	case Mature:
		desc = QObject::tr("Mature - Mating ok. Nudity only while mating.");
		break;
	case Adult:
		desc = QObject::tr("Adult - Mating ok. Nudity ok.");
		break;
	}
	return desc;
}

