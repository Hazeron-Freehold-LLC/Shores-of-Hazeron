//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auclientglobal.cpp,v 1.57 2022/04/08 16:36:15 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auclientglobal.h"
//
#include "aubdreport.h"
#include "auapp.h"
#include "audesign.h"
#include "audossier.h"
#include "aufilever.h"
#include "auframe.h"
#include "auitreport.h"
#include "auscreport.h"
#include "daatom.h"
#include "dgdata.h"
#include <QDataStream>
#include <QFile>

static quint32	s_NextBuildingDesignId = 1;							// Used in single-user client-only mode to provide design ids.
static quint32	s_NextEmpireId = 1;									// Used in single-user client-only mode to provide empire ids.
static quint32	s_NextItemDesignId = 1;								// Used in single-user client-only mode to provide design ids.
static quint32	s_NextMailMessageId = 1;							// Used in single-user client-only mode to provide mail msg ids.
static AuId		s_NextSingleUserAtomId = AuId::FirstUnreserved();	// Used in single-user client-only mode to provide atom ids.
static quint32	s_NextSpaceShipDesignId = 1;						// Used in single-user client-only mode to provide design ids.
static quint32	s_NextStoryId = 1;									// Used in single-user client-only mode to provide story ids.

AuSyncItem::AuSyncItem(TYPE type, const DaAtom* atom)
{
	Q_ASSERT(type == AtomAdded || type == AtomChanged || type == AtomParentChanged);
	m_Type = type;
	m_Atom = atom->Clone();
	//DHH do view atoms ever emit signals? m_Atom->ConnectSignals();
}

AuSyncItem::AuSyncItem(const AuId& atomId) : m_AtomId(atomId)
{
	m_Type = AtomDeparted;
	m_Atom = 0;
}

AuSyncItem::AuSyncItem()
{
	m_Type = AtomsCleared;
	m_Atom = 0;
}

AuSyncItem::~AuSyncItem()
{
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuScDesignListEntry::AuScDesignListEntry(const AuScReport& report, quint8 state, const QImage& thumb) : m_Thumb(thumb)
{
	m_Report = new AuScReport(report);
	m_State = state;
}

AuScDesignListEntry::AuScDesignListEntry(const AuScDesignListEntry& c) : m_Thumb(c.m_Thumb)
{
	m_Report = new AuScReport(*c.m_Report);
	m_State = c.m_State;
}

AuScDesignListEntry::~AuScDesignListEntry()
{
	delete m_Report;
}

const AuScDesignListEntry& AuScDesignListEntry::operator=(const AuScDesignListEntry& c)
{
	m_Thumb = c.m_Thumb;
	if(m_Report)
		delete m_Report;
	m_Report = new AuScReport(*c.m_Report);
	m_State = c.m_State;
	return *this;
}

QString AuScDesignListEntry::StateName() const
{
	return AuDesign::StateName((AuDesign::STATE)m_State);
}

const AuId& AuScDesignListEntry::ArchitectId() const
{
	return m_Report->ArchitectId();
}

const QString& AuScDesignListEntry::ArchitectName() const
{
	return m_Report->ArchitectName();
}

const QString& AuScDesignListEntry::Name() const
{
	return m_Report->DesignName();
}

const AuScReport& AuScDesignListEntry::Report() const
{
	return *m_Report;
}

QString AuScDesignListEntry::ReportText() const
{
	return m_Report->ReportAnalysis(m_State);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBdDesignListEntry::AuBdDesignListEntry(const AuBdReport& report, quint8 state, const QImage& thumb) : m_Thumb(thumb)
{
	m_Report = new AuBdReport(report);
	m_State = state;
}

AuBdDesignListEntry::AuBdDesignListEntry(const AuBdDesignListEntry& c) : m_Thumb(c.m_Thumb)
{
	m_Report = new AuBdReport(*c.m_Report);
	m_State = c.m_State;
}

AuBdDesignListEntry::~AuBdDesignListEntry()
{
	delete m_Report;
}

QString AuBdDesignListEntry::StateName() const
{
	return AuDesign::StateName((AuDesign::STATE)m_State);
}

const AuBdDesignListEntry& AuBdDesignListEntry::operator=(const AuBdDesignListEntry& c)
{
	m_Thumb = c.m_Thumb;
	if(m_Report)
		delete m_Report;
	m_Report = new AuBdReport(*c.m_Report);
	m_State = c.m_State;
	return *this;
}

const AuId& AuBdDesignListEntry::ArchitectId() const
{
	return m_Report->ArchitectId();
}

const QString& AuBdDesignListEntry::ArchitectName() const
{
	return m_Report->ArchitectName();
}

const QString& AuBdDesignListEntry::Name() const
{
	return m_Report->DesignName();
}

const AuBdReport& AuBdDesignListEntry::Report() const
{
	return *m_Report;
}

QString AuBdDesignListEntry::ReportText() const
{
	return m_Report->ReportAnalysis(m_State);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuItDesignListEntry::AuItDesignListEntry(const AuItReport& report, quint8 state, const QImage& thumb) : m_Thumb(thumb)
{
	m_Report = new AuItReport(report);
	m_State = state;
}

AuItDesignListEntry::AuItDesignListEntry(const AuItDesignListEntry& c) : m_Thumb(c.m_Thumb)
{
	m_Report = new AuItReport(*c.m_Report);
	m_State = c.m_State;
}

AuItDesignListEntry::~AuItDesignListEntry()
{
	delete m_Report;
}

const AuItDesignListEntry& AuItDesignListEntry::operator=(const AuItDesignListEntry& c)
{
	m_Thumb = c.m_Thumb;
	if(m_Report)
		delete m_Report;
	m_Report = new AuItReport(*c.m_Report);
	m_State = c.m_State;
	return *this;
}

QString AuItDesignListEntry::StateName() const
{
	return AuDesign::StateName((AuDesign::STATE)m_State);
}

const AuId& AuItDesignListEntry::ArchitectId() const
{
	return m_Report->ArchitectId();
}

const QString& AuItDesignListEntry::ArchitectName() const
{
	return m_Report->ArchitectName();
}

const QString& AuItDesignListEntry::Name() const
{
	return m_Report->DesignName();
}

const AuItReport& AuItDesignListEntry::Report() const
{
	return *m_Report;
}

QString AuItDesignListEntry::ReportText() const
{
	return m_Report->ReportAnalysis(m_State);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuClientGlobal::AuClientGlobal(const QString& appName, const QString& settingsAppName, const QString& company, bool isSolo,
		DgData* data, AuEther* ether, QSettings::Format format) :
	AuGlobal(appName, settingsAppName, company, isSolo ? AuGlobal::SceneSolo : AuGlobal::SceneClient, data, ether, format)
{
	// Client is not supposed to call SetModified but it happens a lot now that movement is performed locally for less lag.
	m_EnableSetModified = false;

	m_DesignerGridFollowsWork = Settings().readBoolEntry("Designer/GridFollowsWork", true);

	m_DesignerGridSpacing = Settings().readNumEntry("Designer/GridSpacing", 1.);
	m_DesignerGridSpacingOm = Settings().readNumEntry("Designer/GridOm", 10);
	m_DesignerGridSpacingNoM = Settings().readNumEntry("Designer/GridNoM", 10);
	if(m_DesignerGridSpacing < 0 || m_DesignerGridSpacingOm <= 0 || m_DesignerGridSpacingNoM <= 0)
	{
		m_DesignerGridSpacing = 1.;		// 1 meter per increment.
		m_DesignerGridSpacingOm = 10;	// 10 increments per brighter increment.
		m_DesignerGridSpacingNoM = 10;	// 10 increments per next brighter increment.
	}

	// USS Enterprise (NCC-1701-D) is 642.5m (2,108 ft) long x 463.73m (1,521.4 ft) wide x 195.26m (640.6 ft) tall.
	// Imperial star destroyer is 1,600m long. D:
	// Death Star is 160,000m long. D:
	// Smallest moon is 1,894m diameter.
	m_DesignerGridRadius = (1600. * FeetPerMeter())/2.;

	m_DesignerSelectionAngleTolerance = Settings().readBoolEntry("Designer/SelectionAngleTolerance", 1);
	if(m_DesignerSelectionAngleTolerance < 0)
		m_DesignerSelectionAngleTolerance = 0;

	m_DesignerShowBacks = Settings().readBoolEntry("Designer/ShowBacks", true);

	m_DesignerShowEdges = Settings().readBoolEntry("Designer/ShowEdges", true);

	m_DesignerShowFaces = Settings().readBoolEntry("Designer/ShowFaces", true);

	m_DesignerShowGround = Settings().readBoolEntry("Designer/ShowGround", true);

	m_DesignerShowSkyBox = Settings().readBoolEntry("Designer/ShowSkyBox", true);

	m_DesignerShowGrid = Settings().readBoolEntry("Designer/ShowGrid", true);

	m_DesignerSnapToEdges = Settings().readBoolEntry("Designer/SnapToEdges", true);

	m_DesignerSnapToGrid = Settings().readBoolEntry("Designer/SnapToGrid", true);

	m_DesignerSnapToVertices = Settings().readBoolEntry("Designer/SnapToVertices", true);

	m_DesignerUnitOfMeasure = (UNIT_OF_MEASURE)Settings().readNumEntry("Designer/UnitOfMeasure", UOM_Meters);
	if(m_DesignerUnitOfMeasure < UOM_Meters || m_DesignerUnitOfMeasure > UOM_Inches)
		m_DesignerUnitOfMeasure = UOM_Meters;

	m_DossierDoubleClickStatus = Settings().readNumEntry("Dossier/DoubleClickStatus", AuDocument::DOC_Closed);
	if(m_DossierDoubleClickStatus < AuDocument::DOC_New || m_DossierDoubleClickStatus >= AuDocument::DOC_Count)
		m_DossierDoubleClickStatus = AuDocument::DOC_Closed;

	m_HUDIntensity = Settings().readNumEntry("Settings/HUDIntensity", 5);
	if(m_HUDIntensity < 0)
		m_HUDIntensity = 0;
	else if(m_HUDIntensity > 10)
		m_HUDIntensity = 10;

	// grabMouse causes cursor issues on the Mac.
#if defined(QT_NO_DEBUG) && !defined(Q_OS_MACOS)
	m_MouseGrab = Settings().readBoolEntry("Settings/MouseGrab", true);
#else
	m_MouseGrab = Settings().readBoolEntry("Settings/MouseGrab", false);	// When debugging, grab is obtrusive.
#endif

	m_MouseLookSensitivity = Settings().readNumEntry("Settings/MouseLookSensitivity", 256);
	if(m_MouseLookSensitivity < 1)
		m_MouseLookSensitivity = 1;
	else if(m_MouseLookSensitivity > 500)
		m_MouseLookSensitivity = 500;

	m_RoundingAngle = Settings().readNumEntry("Settings/RoundingAngle", 10.);
	if(m_RoundingAngle < 0)
		m_RoundingAngle = 0;
	else if(m_RoundingAngle > 90)
		m_RoundingAngle = 90;

	m_RoadType = Settings().readNumEntry("Settings/RoadType", 0);
	if(m_RoadType < 0)
		m_RoadType = 0;
	else if(m_RoadType > 1)
		m_RoadType = 1;

	m_RoadWidth = Settings().readNumEntry("Settings/RoadWidth", 16.);
	if(m_RoadWidth < 8)
		m_RoadWidth = 8;
	else if(m_RoadWidth > 80)
		m_RoadWidth = 80;

	m_SlabWidth = Settings().readNumEntry("Settings/SlabWidth", ceilf(MinSpacecraftSlabWidthMeters()));
	if(m_SlabWidth < 8)
		m_SlabWidth = 8;
	else if(m_SlabWidth > 80)
		m_SlabWidth = 80;

	m_LimitMessages = Settings().readBoolEntry("Settings/ChannelLimitMessages", true);

	m_MessageLimit = Settings().readNumEntry("Settings/ChannelMessageLimit", 50);
	if(m_MessageLimit < 0)
		m_MessageLimit = 0;

	m_PopupNotify = Settings().readNumEntry("Settings/PopupNotify", 2);

	m_ShowNet = Settings().readBoolEntry("Settings/ShowNet", true);

	m_ShowThreads = Settings().readBoolEntry("Settings/ShowThreads", true);

	m_ShowWindowSize = Settings().readBoolEntry("Settings/ShowWindowSize", true);

	m_StoryDialog = Settings().readBoolEntry("Settings/StoryDialog", true);

	// Fonts have been constructed with default values. Load the last saved values now.
	QString commTypeface(TypefaceSans());
	QFontDatabase fontDb;
	QStringList fontFamilies = fontDb.families();
	//qDebug() << fontFamilies;
	if(fontFamilies.contains("Laksaman"))
		commTypeface = "Laksaman";
	m_FontClose = QFont(commTypeface, 22);
	m_FontClose = Settings().readFontEntry("Settings/FontClose", m_FontClose);
	m_FontGalaxy = QFont(commTypeface, 18);
	m_FontGalaxy = Settings().readFontEntry("Settings/FontGalaxy", m_FontGalaxy);
	m_FontHail = QFont(commTypeface, 21);
	m_FontHail = Settings().readFontEntry("Settings/FontHail", m_FontHail);
	m_FontSector = QFont(commTypeface, 19);
	m_FontSector = Settings().readFontEntry("Settings/FontSector", m_FontSector);
	m_FontSystem = QFont(commTypeface, 20);
	m_FontSystem = Settings().readFontEntry("Settings/FontSystem", m_FontSystem);
	m_FontTitle = QFont(commTypeface, 23, QFont::Bold);
	m_FontTitle = Settings().readFontEntry("Settings/FontTitle", m_FontTitle);
	m_FontUniverse = QFont(commTypeface, 17);
	m_FontUniverse = Settings().readFontEntry("Settings/FontUniverse", m_FontUniverse);

	m_ViewButtonsHidden = Settings().readIntListEntry("Settings/ViewButtonsHidden");

	m_ViewActionButtons = Settings().readBoolEntry("Settings/ViewActionButtons", true);

	m_ViewButtonSize = Settings().readNumEntry("Settings/ViewButtonSize", 0);
	if(m_ViewButtonSize < 0)
		m_ViewButtonSize = 0;
	else if(m_ViewButtonSize > 100)
		m_ViewButtonSize = 100;

	m_ViewButtonStyle = Settings().readNumEntry("Settings/ViewButtonStyle", 1);
	if(m_ViewButtonStyle < 0 || m_ViewButtonStyle >= ViewButtonStyleCount())
		m_ViewButtonStyle = 0;

	m_ViewButtonStyleImage = Settings().readEntry("Settings/ViewButtonStyleImage", "");
	if(m_ViewButtonStyleImage.isEmpty() && m_ViewButtonStyle == 0)
		m_ViewButtonStyle = 1;

	m_ViewButtonTransparency = Settings().readNumEntry("Settings/ViewButtonTransparency", 15);
	if(m_ViewButtonTransparency < 0)
		m_ViewButtonTransparency = 0;
	else if(m_ViewButtonTransparency > 100)
		m_ViewButtonTransparency = 100;

	m_WordWrapCharacters = Settings().readNumEntry("Settings/WordWrapCharacters", 100);
	if(m_WordWrapCharacters < 40)
		m_WordWrapCharacters = 40;

	m_WordWrapState = Settings().readBoolEntry("Settings/WordWrap", false);
}

AuClientGlobal::~AuClientGlobal()
{
}

AuId AuClientGlobal::NextAtomId(unsigned int howMany)
{
	// Beware the id allocated by calling this on the client is unique for the session but it is not valid in the game universe.
	AuId retId = s_NextSingleUserAtomId;
	s_NextSingleUserAtomId = s_NextSingleUserAtomId + howMany;
	return retId;
}

quint32 AuClientGlobal::NextBuildingDesignId(unsigned int howMany)
{
	if(!IsSceneSolo())
		return AuGlobal::NextBuildingDesignId(howMany);

	quint32 retId = s_NextBuildingDesignId;
	//qDebug() << "AuClientGlobal::NextBuildingDesignId" << retId << "howMany" << howMany;
	s_NextBuildingDesignId = s_NextBuildingDesignId + howMany;
	return retId;
}

quint32 AuClientGlobal::NextEmpireId()
{
	if(!IsSceneSolo())
		return AuGlobal::NextEmpireId();

	quint32 retId = s_NextEmpireId;
	s_NextEmpireId = s_NextEmpireId + 1;
	return retId;
}

quint32 AuClientGlobal::NextMailMessageId()
{
	if(!IsSceneSolo())
		return AuGlobal::NextMailMessageId();

	quint32 retId = s_NextMailMessageId;
	s_NextMailMessageId = s_NextMailMessageId + 1;
	return retId;
}

quint32 AuClientGlobal::NextItemDesignId(unsigned int howMany)
{
	if(!IsSceneSolo())
		return AuGlobal::NextItemDesignId(howMany);

	quint32 retId = s_NextItemDesignId;
	//qDebug() << "AuClientGlobal::NextItemDesignId" << retId << "howMany" << howMany;
	s_NextItemDesignId = s_NextItemDesignId + howMany;
	return retId;
}

quint32 AuClientGlobal::NextStoryId(unsigned int howMany)
{
	if(!IsSceneSolo())
		return AuGlobal::NextStoryId(howMany);

	quint32 retId = s_NextStoryId;
	//qDebug() << "AuClientGlobal::NextStoryId" << retId << "howMany" << howMany;
	s_NextStoryId = s_NextStoryId + howMany;
	return retId;
}

quint32 AuClientGlobal::NextSpaceShipDesignId(unsigned int howMany)
{
	if(!IsSceneSolo())
		return AuGlobal::NextSpaceShipDesignId(howMany);

	quint32 retId = s_NextSpaceShipDesignId;
	//qDebug() << "AuClientGlobal::NextSpaceShipDesignId" << retId << "howMany" << howMany;
	s_NextSpaceShipDesignId = s_NextSpaceShipDesignId + howMany;
	return retId;
}

void AuClientGlobal::AtomAdded(const AuId& id)
{
	if(!IsSceneSolo())
		AuGlobal::AtomAdded(id);
	//qDebug() << "AuClientGlobal::AtomAdded" << id;
}

void AuClientGlobal::AtomChanged(const AuId& id)
{
	if(!IsSceneSolo())
		AuGlobal::AtomChanged(id);
	//qDebug() << "AuClientGlobal::AtomChanged" << id;
}

void AuClientGlobal::AtomDeleted(const AuId& id)
{
	if(!IsSceneSolo())
		AuGlobal::AtomDeleted(id);
	//qDebug() << "AuClientGlobal::AtomDeleted" << id;
}

void AuClientGlobal::AtomDeparted(const AuId& id)
{
	if(!IsSceneSolo())
		AuGlobal::AtomDeparted(id);
}

void AuClientGlobal::StarMapUpdate(const AuStarMapData& newMap)
{
	AuStarMapData& starMap = LockStarMapForWrite();
	for(unordered_map<AuId, AuStarMapSector, AuIdHash>::const_iterator si = newMap.m_Sectors.begin();
			si != newMap.m_Sectors.end(); si++)
	{
		unordered_map<AuId, AuStarMapSector, AuIdHash>::iterator ui = starMap.m_Sectors.find((*si).first);
		if(ui == starMap.m_Sectors.end())
			starMap.m_Sectors.insert(pair<AuId, AuStarMapSector>((*si).first, (*si).second));
		else
			(*ui).second = (*si).second;
	}
	UnlockStarMap();
}

bool AuClientGlobal::StarMapHasSector(const AuId& id) const
{
	const AuStarMapData& starMap = LockStarMapForRead();
	bool hasSector = starMap.m_Sectors.find(id) != starMap.m_Sectors.end();
	UnlockStarMap();
	return hasSector;
}

void AuClientGlobal::StarMapAddSector(const AuId& id, const AuStarMapSector& sector)
{
	//qDebug() << "AuClientGlobal::StarMapAddSector" << id << sector.m_Name;
	AuStarMapData& starMap = LockStarMapForWrite();
	starMap.m_Sectors.insert(pair<AuId, AuStarMapSector>(id, sector));
	UnlockStarMap();
}

void AuClientGlobal::StarMapInit(quint32 empireId)
{
	AuStarMapData& starMap = LockStarMapForWrite();
	starMap.m_Sectors.clear();
	m_StarMapEmpire = empireId;
	UnlockStarMap();
}

// Sets the empire id of the owner of the current star map.
// Clears the current star map when the owner empire changes.
bool AuClientGlobal::StarMapSetEmpire(quint32 empireId)
{
	if(StarMapEmpire() != empireId)
	{
		StarMapInit(empireId);
		return true;
	}
	return false;
}

quint32 AuClientGlobal::StarMapEmpire() const
{
	LockStarMapForRead();
	quint32 starMapEmpire = m_StarMapEmpire;
	UnlockStarMap();
	return starMapEmpire;
}

bool AuClientGlobal::StarMapIsEmpty() const
{
	const AuStarMapData& starMap = LockStarMapForRead();
	bool isEmpty = starMap.m_Sectors.size() == 0;
	UnlockStarMap();
	return isEmpty;
}

void AuClientGlobal::StarMapListAirportsInSystem(const AuId& systemId, AuIdList& cityIds, QStringList& cityNames,
		QList<quint32>& cityEmpires) const
{
	// Search through star map sectors to find the system.
	const AuStarMapData& starMap = LockStarMapForRead();
	for(unordered_map<AuId, AuStarMapSector, AuIdHash>::const_iterator sectI = starMap.m_Sectors.begin();
			sectI != starMap.m_Sectors.end(); sectI++)
	{
		unordered_map<AuId, AuStarMapSystem, AuIdHash>::const_iterator sysI = (*sectI).second.m_Systems.find(systemId);
		if(sysI != (*sectI).second.m_Systems.end())
		{
			// Found the system.
			for(unordered_map<AuId, AuStarMapCity, AuIdHash>::const_iterator cityI = (*sysI).second.m_Cities.begin();
					cityI != (*sysI).second.m_Cities.end(); cityI++)
			{
				if(!(*cityI).second.HasAirport() && !(*cityI).second.HasAirBase())
					continue;
				cityIds.append((*cityI).first);
				cityNames.append((*cityI).second.m_Name);
				cityEmpires.append((*cityI).second.m_Empire);
			}
			break;
		}
	}
	UnlockStarMap();
}

void AuClientGlobal::StarMapListCitiesInSystem(const AuId& systemId, AuIdList& cityIds, QStringList& cityNames,
		QList<quint32>& cityEmpires) const
{
	// Search through star map sectors to find the system.
	const AuStarMapData& starMap = LockStarMapForRead();
	for(unordered_map<AuId, AuStarMapSector, AuIdHash>::const_iterator sectI = starMap.m_Sectors.begin();
			sectI != starMap.m_Sectors.end(); sectI++)
	{
		unordered_map<AuId, AuStarMapSystem, AuIdHash>::const_iterator sysI = (*sectI).second.m_Systems.find(systemId);
		if(sysI != (*sectI).second.m_Systems.end())
		{
			// Found the system.
			for(unordered_map<AuId, AuStarMapCity, AuIdHash>::const_iterator cityI = (*sysI).second.m_Cities.begin();
					cityI != (*sysI).second.m_Cities.end(); cityI++)
			{
				cityIds.append((*cityI).first);
				cityNames.append((*cityI).second.m_Name);
				cityEmpires.append((*cityI).second.m_Empire);
			}
			break;
		}
	}
	UnlockStarMap();
}

void AuClientGlobal::StarMapListAirportsAtPlanet(const AuId& systemId, const AuId& /*planetId*/, AuIdList& cityIds,
		QStringList& cityNames, QList<quint32>& cityEmpires) const
{
	// Search through star map sectors to find the system.
	const AuStarMapData& starMap = LockStarMapForRead();
	for(unordered_map<AuId, AuStarMapSector, AuIdHash>::const_iterator sectI = starMap.m_Sectors.begin();
			sectI != starMap.m_Sectors.end(); sectI++)
	{
		unordered_map<AuId, AuStarMapSystem, AuIdHash>::const_iterator sysI = (*sectI).second.m_Systems.find(systemId);
		if(sysI != (*sectI).second.m_Systems.end())
		{
			// Found the system.
			for(unordered_map<AuId, AuStarMapCity, AuIdHash>::const_iterator cityI = (*sysI).second.m_Cities.begin();
					cityI != (*sysI).second.m_Cities.end(); cityI++)
			{
				if(!(*cityI).second.HasAirport() && !(*cityI).second.HasAirBase())
					continue;
				//DHH test planet id
				cityIds.append((*cityI).first);
				cityNames.append((*cityI).second.m_Name);
				cityEmpires.append((*cityI).second.m_Empire);
			}
			break;
		}
	}
	UnlockStarMap();
}

void AuClientGlobal::StarMapListCitiesAtPlanet(const AuId& systemId, const AuId& /*planetId*/, AuIdList& cityIds,
		QStringList& cityNames, QList<quint32>& cityEmpires) const
{
	// Search through star map sectors to find the system.
	const AuStarMapData& starMap = LockStarMapForRead();
	for(unordered_map<AuId, AuStarMapSector, AuIdHash>::const_iterator sectI = starMap.m_Sectors.begin();
			sectI != starMap.m_Sectors.end(); sectI++)
	{
		unordered_map<AuId, AuStarMapSystem, AuIdHash>::const_iterator sysI = (*sectI).second.m_Systems.find(systemId);
		if(sysI != (*sectI).second.m_Systems.end())
		{
			// Found the system.
			for(unordered_map<AuId, AuStarMapCity, AuIdHash>::const_iterator cityI = (*sysI).second.m_Cities.begin();
					cityI != (*sysI).second.m_Cities.end(); cityI++)
			{
				//DHH test planet id
				cityIds.append((*cityI).first);
				cityNames.append((*cityI).second.m_Name);
				cityEmpires.append((*cityI).second.m_Empire);
			}
			break;
		}
	}
	UnlockStarMap();
}

void AuClientGlobal::OpenChannel(const QString& channelName, AuCommStdChannel::SCOPE scope, const AuId& idFrom,
		const QString& fromName, const AuId& fromSector, GALAXY fromGalaxy, const AuId& idTo, bool connectBeyondSolarSystem)
{
	LockAtomChat();
	AuOpenChannel openchan(channelName, scope, idFrom, fromName, fromSector, fromGalaxy, idTo, connectBeyondSolarSystem);
	m_OpenChannels.append(openchan);
	UnlockAtomChat();
}

void AuClientGlobal::SendAtomChat(const AuCommPacket& msg)
{
	AuAtomChat chat(msg);
	LockAtomChat()->append(chat);
	UnlockAtomChat();
}

void AuClientGlobal::DecayAtomChat()
{
	LockAtomChat()->Decay();
	UnlockAtomChat();
}

// Overridden in the client to get images for picture frames off the internet.
QImage AuClientGlobal::DownloadImage(const QUrl& pictureURL)
{
	// Check local cache before making network request.
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return QImage();

	// Has this image already been requested?
	m_ImageDownloadsLock.lock();
	QHash<QUrl, AuImageDownload>::iterator di = m_ImageDownloads.find(pictureURL);
	if(di == m_ImageDownloads.end())
	{
		//qDebug() << "AuClientGlobal::DownloadImage" << pictureURL << "requested";
		m_ImageDownloads.insert(pictureURL, AuImageDownload());	// Create new entry in local request queue.
		m_ImageDownloadsLock.unlock();
		return QImage();
	}

	bool reqfin = (*di).NetRequestFinished();
	m_ImageDownloadsLock.unlock();
	if(!reqfin)
		return QImage();				// Return null image until the network manager says it's finished.

	// Get data from local cache.
	QIODevice* fileData = pFrame->NetCacheData(pictureURL);
	if(!fileData)
		return QImage();				// The file data is not in our network cache?

	// The file is in our network cache.
	QImage img;
	img.load(fileData, 0);
	//qDebug() << "AuClientGlobal::DownloadImage" << pictureURL << "loaded from cache" << img.size();
	fileData->close();
	delete fileData;
	return img;
}

// static
int AuClientGlobal::ViewButtonStyleCount()
{
	return 27;
}

//static
QString AuClientGlobal::ViewButtonStyleName(int vbs)
{
	QString buttonStyles[] =
	{
		QObject::tr("Custom..."),
		QObject::tr("Hazeron"),
		QObject::tr("Blue"),
		QObject::tr("Round"),
		QObject::tr("Rounded"),
		QObject::tr("Square"),
		QObject::tr("Black Hole"),
		QObject::tr("Blood"),
		QObject::tr("Cloud"),
		QObject::tr("Concrete"),
		QObject::tr("Flash"),
		QObject::tr("Freeze"),
		QObject::tr("Gas Giant"),
		QObject::tr("Heat"),
		QObject::tr("Leaf"),
		QObject::tr("Lightning"),
		QObject::tr("Matrix"),
		QObject::tr("Moon"),
		QObject::tr("Planet"),
		QObject::tr("Poison"),
		QObject::tr("Radiation"),
		QObject::tr("Rune"),
		QObject::tr("Shield"),
		QObject::tr("Camera"),
		QObject::tr("Eye"),
		QObject::tr("Titan"),
		QObject::tr("Asteroid"),
	};
	Q_ASSERT(sizeof(buttonStyles)/sizeof(QString) == ViewButtonStyleCount());
	return buttonStyles[vbs];
}

//static
QString AuClientGlobal::ViewButtonImageName(int vbs)
{
	QString buttonImages[] =
	{
		g_AuClient.m_ViewButtonStyleImage,
		":data/ButtonBackSoH.png",
		":data/ButtonBackBlue.png",
		":data/ButtonBackRound.png",
		":data/ButtonBackRounded.png",
		":data/ButtonBack.png",
		":data/StarBlackHole.png",
		":data/Blood.png",
		":data/poisonsprite.png",
		":data/Ringworld.png",
		":data/MuzzleFlash.png",
		":data/Freeze.png",
		":data/WorldGasGiant.png",
		":data/Heat.png",
		":data/ButtonBackLeaf.png",
		":data/Lightning.png",
		":data/Matrix2.png",		// This is a picture of the Matrix green threads of arcane symbols.
		":data/WorldMoon.png",
		":data/WorldPlanet.png",
		":data/Poison.png",
		":data/Radiation.png",
		":data/SpawnWait.png",
		":data/Shield.png",
		":data/ComputerEye.png",
		":data/Matrix.png",			// This is a picture of the Illuminati all seeing eye.
		":data/WorldTitan.png",
		":data/Asteroid.png",
	};
	Q_ASSERT(sizeof(buttonImages)/sizeof(QString) == ViewButtonStyleCount());
	return buttonImages[vbs];
}

// Adds a bid from the sender id on the named channel.
// Revokes all previous bids by the sender on the channel.
// NOTE: Does not verify the comm packet actually represents a bid. Caller does that.
void AuClientGlobal::BidPlaced(const AuCommPacket* msg)
{
	// Revoke any current bids by the sender on the channel.
	BidRevoked(msg->FromId(), msg->ChannelName());

	// Add the nu bid to the queue.
	m_muActiveBids.lock();
	m_ActiveBids.append(AuAtomChat(*msg));
	m_muActiveBids.unlock();
}

// Removes all bids from the sender id on the named channel.
// There should only be one at most but we'll scan the entire list to be thorough.
void AuClientGlobal::BidRevoked(const AuId& idFrom, const QString& channelName)
{
	m_muActiveBids.lock();
	QList<AuAtomChat>::iterator it = m_ActiveBids.begin();
	while(it != m_ActiveBids.end())
	{
		if(!(*it).HasCommPacket())
			continue;

		if((*it).CommPacket()->FromId() == idFrom &&
				(*it).CommPacket()->ChannelName() == channelName)
			it = m_ActiveBids.erase(it);
		else
			it++;
	}
	//qDebug("AuClientGlobal::BidRevoked - id %d channel %s, %d active bids",
			//idFrom.GetID(), (const char*)channelName.toAscii(), m_ActiveBids.count());
	m_muActiveBids.unlock();
}

void AuClientGlobal::DecayActiveBids()
{
	m_muActiveBids.lock();
	QList<AuAtomChat>::iterator it = m_ActiveBids.begin();
	while(it != m_ActiveBids.end())
	{
		if((*it).IsBidExpired())
			it = m_ActiveBids.erase(it);
		else
			it++;
	}
	m_muActiveBids.unlock();
}

// Checks for pending crew notices that corresponds to the crew message.
// Return false if there is no match or the requestor has no active crew notices.
bool AuClientGlobal::HasPendingCrew(const AuCommPacket* msg) const
{
	AuId idLoader;
	const AuCommBoardCrew* msgBoardCrew = dynamic_cast<const AuCommBoardCrew*>(msg);
	if(msgBoardCrew)
		idLoader = msgBoardCrew->LoaderId();
	else
	{
		const AuCommCrewBoarded* msgCrewBoarded = dynamic_cast<const AuCommCrewBoarded*>(msg);
		if(msgCrewBoarded)
			idLoader = msgCrewBoarded->LoaderId();
	}
	if(idLoader.IsNull())
		return false;

	// Look for the matching crew board/debark notice.
	bool foundMatchingBid = false;
	m_muActiveBids.lock();
	for(QList<AuAtomChat>::const_iterator it = m_ActiveBids.cbegin(); it != m_ActiveBids.cend(); it++)
	{
		if(!(*it).HasCommPacket())
			continue;

		// Different types of crew notices can be active. Check the message type.
		if((*it).CommPacket()->Type() != AuCommPacket::BoardCrew)
			continue;

		if((*it).CommPacket()->FromId() == idLoader &&
				(*it).CommPacket()->ChannelName() == msg->ChannelName())
		{
			// Found the active crew notice by the sender on the channel; there can be only one. See AuClientGlobal::BidPlaced.
			// Insure the crew notice has not expired.
			if(!(*it).IsBidExpired())
				foundMatchingBid = true;	// Accept matches the Bid.
			break;
		}
	}
	m_muActiveBids.unlock();
	return foundMatchingBid;
}

// Checks for pending crew notices that corresponds to the crew message.
// Return false if there is no match or the requestor has no active crew notices.
bool AuClientGlobal::HasPendingCryo(const AuCommPacket* msg) const
{
	AuId idLoader;
	const AuCommBoardCryo* msgBoardCryo = dynamic_cast<const AuCommBoardCryo*>(msg);
	if(msgBoardCryo)
		idLoader = msgBoardCryo->LoaderId();
	else
	{
		const AuCommCryoBoarded* msgCryoBoarded = dynamic_cast<const AuCommCryoBoarded*>(msg);
		if(msgCryoBoarded)
			idLoader = msgCryoBoarded->LoaderId();
		else
		{
			const AuCommCryoDebarked* msgCryoDebarked = dynamic_cast<const AuCommCryoDebarked*>(msg);
			if(msgCryoDebarked)
				idLoader = msgCryoDebarked->UnloaderId();
			else
			{
				const AuCommDebarkCryo* msgDebarkCryo = dynamic_cast<const AuCommDebarkCryo*>(msg);
				if(msgDebarkCryo)
					idLoader = msgDebarkCryo->UnloaderId();
			}
		}
	}
	if(idLoader.IsNull())
		return false;

	// Look for the matching crew board/debark notice.
	bool foundMatchingBid = false;
	m_muActiveBids.lock();
	for(QList<AuAtomChat>::const_iterator it = m_ActiveBids.cbegin(); it != m_ActiveBids.cend(); it++)
	{
		if(!(*it).HasCommPacket())
			continue;

		// Different types of crew notices can be active. Check the message type.
		if((*it).CommPacket()->Type() != AuCommPacket::BoardCryo &&
			(*it).CommPacket()->Type() != AuCommPacket::DebarkCryo)
			continue;

		if((*it).CommPacket()->FromId() == idLoader &&
				(*it).CommPacket()->ChannelName() == msg->ChannelName())
		{
			// Found the active crew notice by the sender on the channel; there can be only one. See AuClientGlobal::BidPlaced.
			// Insure the crew notice has not expired.
			if(!(*it).IsBidExpired())
				foundMatchingBid = true;	// Accept matches the Bid.
			break;
		}
	}
	m_muActiveBids.unlock();
	return foundMatchingBid;
}

// Checks for pending colonist notices that corresponds to the colonist message.
// Return false if there is no match or the requestor has no active colonist notices.
bool AuClientGlobal::HasPendingColonists(const AuCommPacket* msg) const
{
	AuId idLoader;
	const AuCommBoardPassengers* msgBoardPassengers = dynamic_cast<const AuCommBoardPassengers*>(msg);
	if(msgBoardPassengers)
		idLoader = msgBoardPassengers->LoaderId();
	else
	{
		const AuCommPassengersBoarded* msgPassengersBoarded = dynamic_cast<const AuCommPassengersBoarded*>(msg);
		if(msgPassengersBoarded)
			idLoader = msgPassengersBoarded->LoaderId();
		else
		{
			const AuCommPassengersDebarked* msgPassengersDebarked = dynamic_cast<const AuCommPassengersDebarked*>(msg);
			if(msgPassengersDebarked)
				idLoader = msgPassengersDebarked->UnloaderId();
			else
			{
				const AuCommDebarkPassengers* msgDebarkPassengers = dynamic_cast<const AuCommDebarkPassengers*>(msg);
				if(msgDebarkPassengers)
					idLoader = msgDebarkPassengers->UnloaderId();
			}
		}
	}
	if(idLoader.IsNull())
		return false;

	// Look for the matching colonist board/debark notice.
	bool foundMatchingBid = false;
	m_muActiveBids.lock();
	for(QList<AuAtomChat>::const_iterator it = m_ActiveBids.cbegin(); it != m_ActiveBids.cend(); it++)
	{
		if(!(*it).HasCommPacket())
			continue;

		// Different types of colonist notices can be active.
		// Check the message type.
		if((*it).CommPacket()->Type() != AuCommPacket::BoardPassengers &&
			(*it).CommPacket()->Type() != AuCommPacket::DebarkPassengers)
			continue;

		if((*it).CommPacket()->FromId() == idLoader &&
				(*it).CommPacket()->ChannelName() == msg->ChannelName())
		{
			// Found the active colonist notice by the sender on the channel;
			// there can be only one. See AuClientGlobal::BidPlaced.
			// Insure the colonist notice has not expired.
			if(!(*it).IsBidExpired())
				foundMatchingBid = true;	// Accept matches the Bid.
			break;
		}
	}
	m_muActiveBids.unlock();
	return foundMatchingBid;
}

// Checks for pending officer notices that corresponds to the officer message.
// Return false if there is no match or the requestor has no active officer notices.
bool AuClientGlobal::HasPendingOfficers(const AuCommPacket* msg) const
{
	AuId idLoader;
	const AuCommBoardOfficers* msgBoardOfficers = dynamic_cast<const AuCommBoardOfficers*>(msg);
	if(msgBoardOfficers)
		idLoader = msgBoardOfficers->LoaderId();
	else
	{
		const AuCommOfficerBoarded* msgOfficersBoarded = dynamic_cast<const AuCommOfficerBoarded*>(msg);
		if(msgOfficersBoarded)
			idLoader = msgOfficersBoarded->LoaderId();
	}
	if(idLoader.IsNull())
		return false;

	// Look for the matching troop board/debark notice.
	bool foundMatchingBid = false;
	m_muActiveBids.lock();
	for(QList<AuAtomChat>::const_iterator it = m_ActiveBids.cbegin(); it != m_ActiveBids.cend(); it++)
	{
		if(!(*it).HasCommPacket())
			continue;

		// Different types of troop notices can be active.
		// Check the message type.
		if((*it).CommPacket()->Type() != AuCommPacket::BoardOfficers)
			continue;

		if((*it).CommPacket()->FromId() == idLoader &&
				(*it).CommPacket()->ChannelName() == msg->ChannelName())
		{
			// Found the active officer notice by the sender on the channel;
			// there can be only one. See AuClientGlobal::BidPlaced.
			// Insure the officer notice has not expired.
			if(!(*it).IsBidExpired())
				foundMatchingBid = true;	// Accept matches the Bid.
			break;
		}
	}
	m_muActiveBids.unlock();
	return foundMatchingBid;
}

// Checks for pending troop notices that corresponds to the troop message.
// Return false if there is no match or the requestor has no active troop notices.
bool AuClientGlobal::HasPendingTroops(const AuCommPacket* msg) const
{
	AuId idLoader;
	const AuCommBoardTroops* msgBoardTroops = dynamic_cast<const AuCommBoardTroops*>(msg);
	if(msgBoardTroops)
		idLoader = msgBoardTroops->LoaderId();
	else
	{
		const AuCommTroopsBoarded* msgTroopsBoarded = dynamic_cast<const AuCommTroopsBoarded*>(msg);
		if(msgTroopsBoarded)
			idLoader = msgTroopsBoarded->LoaderId();
	}
	if(idLoader.IsNull())
		return false;	// Need a troop message.

	// Look for the matching troop board/debark notice.
	bool foundMatchingBid = false;
	m_muActiveBids.lock();
	for(QList<AuAtomChat>::const_iterator it = m_ActiveBids.cbegin(); it != m_ActiveBids.cend(); it++)
	{
		if(!(*it).HasCommPacket())
			continue;

		// Different types of troop notices can be active.
		// Check the message type.
		if((*it).CommPacket()->Type() != AuCommPacket::BoardTroops)
			continue;

		if((*it).CommPacket()->FromId() == idLoader &&
				(*it).CommPacket()->ChannelName() == msg->ChannelName())
		{
			// Found the active troop notice by the sender on the channel;
			// there can be only one. See AuClientGlobal::BidPlaced.
			// Insure the troop notice has not expired.
			if(!(*it).IsBidExpired())
				foundMatchingBid = true;	// Accept matches the Bid.
			break;
		}
	}
	m_muActiveBids.unlock();
	return foundMatchingBid;
}

// Checks for pending Bid that corresponds to the msg,
// expected to be an Accept or Bid message.
// Return false if there is no match or the buyer has no active bid.
bool AuClientGlobal::HasPendingBid(const AuCommPacket* msg) const
{
	AuId idBuyer;
	const AuCommAccept* msgAccept = dynamic_cast<const AuCommAccept*>(msg);
	if(msgAccept)
		idBuyer = msgAccept->BuyerId();
	else
	{
		const AuCommBid* msgBid = dynamic_cast<const AuCommBid*>(msg);
		if(msgBid)
			idBuyer = msgBid->FromId();
	}
	if(idBuyer.IsNull())
		return false;	// Need to pass an Accept or Bid message.

	// Look for the matching bid.
	bool foundMatchingBid = false;
	m_muActiveBids.lock();
	for(QList<AuAtomChat>::const_iterator it = m_ActiveBids.cbegin(); it != m_ActiveBids.cend(); it++)
	{
		if(!(*it).HasCommPacket())
			continue;

		// Different types of bids can be active.
		const AuCommBid* bid = dynamic_cast<const AuCommBid*>((*it).CommPacket());
		if(!bid)
			continue;

		if(bid->FromId() == idBuyer && bid->ChannelName() == msg->ChannelName())
		{
			// Found the active bid by the buyer on the channel; there can be only one. See AuClientGlobal::BidPlaced.
			//qDebug("AuClientGlobal::HasPendingBid found bid %s, bidText %s",
					//(const char*)(*it).m_CommPacket->RawText().toAscii(),
					//(const char*)bidText.toAscii());
			if(bid->CorrespondsTo(msg))
			{
				// Insure the bid has not expired.
				if(!(*it).IsBidExpired())
					foundMatchingBid = true;	// Accept or Bid matches the Bid.
				//else
					//qDebug("AuClientGlobal::HasPendingBid found expired bid");
				break;
			}
		}
	}
	m_muActiveBids.unlock();
	return foundMatchingBid;
}

// Checks for pending Bid that corresponds to the msg,
// expected to be an Accept or Bid message.
// Return false if there is no match or the buyer has no active bid.
bool AuClientGlobal::HasPendingBidBarter(const AuCommPacket* msg) const
{
	AuId idBuyer;
	const AuCommAcceptBarter* msgAccept = dynamic_cast<const AuCommAcceptBarter*>(msg);
	if(msgAccept)
		idBuyer = msgAccept->BuyerId();
	else
	{
		const AuCommBidBarter* msgBid = dynamic_cast<const AuCommBidBarter*>(msg);
		if(msgBid)
			idBuyer = msgBid->FromId();
	}
	if(idBuyer.IsNull())
		return false;	// Need to pass an Accept or Bid message.

	// Look for the matching bid.
	bool foundMatchingBid = false;
	m_muActiveBids.lock();
	for(QList<AuAtomChat>::const_iterator it = m_ActiveBids.cbegin(); it != m_ActiveBids.cend(); it++)
	{
		if(!(*it).HasCommPacket())
			continue;

		// Different types of bids can be active.
		const AuCommBidBarter* bid = dynamic_cast<const AuCommBidBarter*>((*it).CommPacket());
		if(!bid)
			continue;

		if(bid->FromId() == idBuyer && bid->ChannelName() == msg->ChannelName())
		{
			// Found the active bid by the buyer on the channel; there can be only one. See AuClientGlobal::BidPlaced.
			//qDebug("AuClientGlobal::HasPendingBid found bid %s, bidText %s",
					//(const char*)(*it).m_CommPacket->RawText().toAscii(),
					//(const char*)bidText.toAscii());
			if(bid->CorrespondsTo(msg))
			{
				// Insure the bid has not expired.
				if(!(*it).IsBidExpired())
					foundMatchingBid = true;	// Accept or Bid matches the Bid.
				//else
					//qDebug("AuClientGlobal::HasPendingBid found expired bid");
				break;
			}
		}
	}
	m_muActiveBids.unlock();
	return foundMatchingBid;
}

bool AuClientGlobal::HasPendingBidShipment(const AuCommPacket* msg) const
{
	AuId idBuyer;
	const AuCommAcceptPickupShipment* msgAccept = dynamic_cast<const AuCommAcceptPickupShipment*>(msg);
	if(msgAccept)
		idBuyer = msgAccept->BuyerId();
	else
	{
		const AuCommBidPickupShipment* msgBid = dynamic_cast<const AuCommBidPickupShipment*>(msg);
		if(msgBid)
			idBuyer = msgBid->FromId();
	}
	if(idBuyer.IsNull())
		return false;	// Need to pass an Accept or Bid message.

	// Look for the matching bid.
	bool foundMatchingBid = false;
	m_muActiveBids.lock();
	for(QList<AuAtomChat>::const_iterator it = m_ActiveBids.cbegin(); it != m_ActiveBids.cend(); it++)
	{
		if(!(*it).HasCommPacket())
			continue;

		// Different types of bids can be active.
		const AuCommBidPickupShipment* bid = dynamic_cast<const AuCommBidPickupShipment*>((*it).CommPacket());
		if(!bid)
			continue;

		if(bid->FromId() == idBuyer && bid->ChannelName() == msg->ChannelName())
		{
			// Found the active bid by the buyer on the channel; there can be only one. See AuClientGlobal::BidPlaced.
			if(bid->CorrespondsTo(msg))
			{
				// Insure the bid has not expired.
				if(!(*it).IsBidExpired())
					foundMatchingBid = true;	// Accept or Bid matches the Bid.
				break;
			}
		}
	}
	m_muActiveBids.unlock();
	return foundMatchingBid;
}

// Checks for pending OfferRepairXXX that corresponds to the msg, expected to be an AcceptRepair message.
// Return false if there is no match or the buyer has no active bid.
bool AuClientGlobal::HasPendingRepairOffer(const AuCommPacket* msg) const
{
	AuId repairShipId = msg->RepairShipId();
	if(repairShipId.IsNull())
		return false;

	// The message type must correspond to the offer type.
	AuCommPacket::TYPE offerType;
	AuId cityid;
	const AuCommAcceptRepairAll* msgAcceptRepairAll = dynamic_cast<const AuCommAcceptRepairAll*>(msg);
	if(msgAcceptRepairAll)
	{
		offerType = AuCommPacket::OfferRepairAll;
		cityid = msgAcceptRepairAll->CityId();
	}
	else
	{
		const AuCommAcceptRepairSystem* msgAcceptRepairSystem = dynamic_cast<const AuCommAcceptRepairSystem*>(msg);
		if(msgAcceptRepairSystem)
		{
			offerType = AuCommPacket::OfferRepairSystem;
			cityid = msgAcceptRepairSystem->CityId();
		}
		else
		{
			offerType = AuCommPacket::OfferRepairAll;	// Eliminates a compiler warning.
		}
	}
	if(cityid.IsNull())
		return false;

	// Look for the matching OfferRepairXXX notice.
	bool foundMatchingBid = false;
	m_muActiveBids.lock();
	for(QList<AuAtomChat>::const_iterator it = m_ActiveBids.cbegin(); it != m_ActiveBids.cend(); it++)
	{
		if(!(*it).HasCommPacket())
			continue;

		// Different types of repair offers can be active.
		// Check the message type.
		if((*it).CommPacket()->Type() != offerType)
			continue;

		if((*it).CommPacket()->FromId() == cityid &&
				(*it).CommPacket()->RepairShipId() == repairShipId &&
				(*it).CommPacket()->ChannelName() == msg->ChannelName())
		{
			// Found the active offer by the sender on the channel; there can be only one. See AuClientGlobal::BidPlaced.
			// Insure the offer has not expired.
			if(!(*it).IsBidExpired())
				foundMatchingBid = true;	// Accept matches the Bid.
			break;
		}
	}
	m_muActiveBids.unlock();
	return foundMatchingBid;
}

QString AuClientGlobal::FormatCoordinate(const AuPoint& c)
{
	// Convert to user's unit of measure.
	AuPoint vec = c / DesignerUnitScale();

	// Round
	AuPoint cround = vec.Rounded(.001);

	// Format
	QString sloc;
	if(cround.z != 0)
		sloc = QString("(%L1, %L2, %L3)%4")
			.arg(cround.x, 0,'g',3)
			.arg(cround.y, 0,'g',3)
			.arg(cround.z, 0,'g',3)
			.arg(DesignerUnitSymbol());
	else
		sloc = QString("(%L1, %L2)%3")
			.arg(cround.x, 0,'g',3)
			.arg(cround.y, 0,'g',3)
			.arg(DesignerUnitSymbol());

	return sloc;
}

QString AuClientGlobal::FormatLength(double l)
{
	// Convert to user's unit of measure.
	double len = l / DesignerUnitScale();

	// Round
	double cround = FROUND(len, .001);

	// Format
	QString sloc;
	if(cround != 0)
		sloc = QObject::tr("%L1%2")
			.arg(cround, 0,'g',3)
			.arg(DesignerUnitSymbol());
	else
		sloc = QObject::tr("0%1")
			.arg(DesignerUnitSymbol());

	return sloc;
}

// Adds/replaces a design in the list of Sc designs available for loading from the library.
void AuClientGlobal::DesignerAddScDesignListEntry(const AuScReport& report, quint8 state, const QImage& thumb)
{
	m_ScDesignListLock.lock();
	m_ScDesignList.insert(report.DesignId(), AuScDesignListEntry(report, state, thumb));
	m_ScDesignListLock.unlock();
}

// Clears the list of Sc designs available for loading from the library.
void AuClientGlobal::DesignerClearScList()
{
	m_ScDesignListLock.lock();
	m_ScDesignList.clear();
	m_ScDesignListLock.unlock();
}

void AuClientGlobal::DesignerAddScExchangeEntry(const AuScDesignExchangeEntry& libraryEntry, const QImage& thumb)
{
	m_ScExchangeLock.lock();
	m_ScExchange.insert(libraryEntry.DesignId(), AuScDesignExchangeEntryClient(libraryEntry, thumb));
	m_ScExchangeLock.unlock();

	// Cache the entry locally.
	// Insure the cache folder exists.
	if(CreateCacheGame())
	{
		// The cache folder exists.
		QDir cacheDir(CacheGame());
		QString cacheFileName = cacheDir.absoluteFilePath(QString("ScX%1").arg(libraryEntry.DesignId()));
		QFile cacheFile(cacheFileName);
		if(cacheFile.open(QFile::WriteOnly))
		{
			// Prepare to write to the file.
			QDataStream dsfile(&cacheFile);
			dsfile.setVersion(QDataStream::Qt_4_1);

			// Write a header to the file.
			const quint8 fileType = SC_EXCHANGE_CACHE;
			const quint8 fileVersion = SC_EXCHANGE_VER;
			dsfile << fileType;
			dsfile << fileVersion;

			// Write the data to the file.
			dsfile << libraryEntry;
			dsfile << thumb;

			// Close the file.
			cacheFile.close();
		}
	}
}

void AuClientGlobal::DesignerClearScExchange()
{
	m_ScExchangeLock.lock();
	m_ScExchange.clear();
	m_ScExchangeLock.unlock();
}

// Adds/replaces a design in the list of Bd designs available for loading from the library.
void AuClientGlobal::DesignerAddBdDesignListEntry(const AuBdReport& report, quint8 state, const QImage& thumb)
{
	m_BdDesignListLock.lock();
	m_BdDesignList.insert(report.DesignId(), AuBdDesignListEntry(report, state, thumb));
	m_BdDesignListLock.unlock();
}

// Clears the list of Bd designs available for loading from the library.
void AuClientGlobal::DesignerClearBdList()
{
	m_BdDesignListLock.lock();
	m_BdDesignList.clear();
	m_BdDesignListLock.unlock();
}

void AuClientGlobal::DesignerAddItDesignListEntry(const AuItReport& report, quint8 state, const QImage& thumb)
{
	m_ItDesignListLock.lock();
	m_ItDesignList.insert(report.DesignId(), AuItDesignListEntry(report, state, thumb));
	m_ItDesignListLock.unlock();
}

void AuClientGlobal::DesignerClearItList()
{
	m_ItDesignListLock.lock();
	m_ItDesignList.clear();
	m_ItDesignListLock.unlock();
}

void AuClientGlobal::DesignerAddBdExchangeEntry(const AuBdDesignExchangeEntry& libraryEntry, const QImage& thumb)
{
	m_BdExchangeLock.lock();
	m_BdExchange.insert(libraryEntry.DesignId(), AuBdDesignExchangeEntryClient(libraryEntry, thumb));
	m_BdExchangeLock.unlock();

	// Cache the entry locally.
	// Insure the cache folder exists.
	if(CreateCacheGame())
	{
		// The cache folder exists.
		QDir cacheDir(CacheGame());
		QString cacheFileName = cacheDir.absoluteFilePath(QString("BdX%1").arg(libraryEntry.DesignId()));
		QFile cacheFile(cacheFileName);
		if(cacheFile.open(QFile::WriteOnly))
		{
			// Prepare to write to the file.
			QDataStream dsfile(&cacheFile);
			dsfile.setVersion(QDataStream::Qt_4_1);

			// Write a header to the file.
			const quint8 fileType = BD_EXCHANGE_CACHE;
			const quint8 fileVersion = BD_EXCHANGE_VER;
			dsfile << fileType;
			dsfile << fileVersion;

			// Write the data to the file.
			dsfile << libraryEntry;
			dsfile << thumb;

			// Close the file.
			cacheFile.close();
		}
	}
}

void AuClientGlobal::DesignerClearBdExchange()
{
	m_BdExchangeLock.lock();
	m_BdExchange.clear();
	m_BdExchangeLock.unlock();
}

void AuClientGlobal::DossierSetDoubleClickStatus(int dcs)
{
	if(dcs < AuDocument::DOC_New || dcs >= AuDocument::DOC_Count)
		return;
	m_DossierDoubleClickStatus = dcs;
	Settings().writeEntry("Dossier/DoubleClickStatus", dcs);
}

void AuClientGlobal::ControlPacket(const QByteArray& ba)
{
	m_SceneControlLock.lock();
	m_ControlPackets.append(ba);
	m_SceneControlLock.unlock();
}

void AuClientGlobal::TakeControlQueue(QList<QByteArray>& controls)
{
	m_SceneControlLock.lock();
	controls = m_ControlPackets;
	m_ControlPackets.clear();
	m_SceneControlLock.unlock();
}

void AuClientGlobal::SyncAtomChanged(const DaAtom* atom)
{
	m_SceneSyncLock.lock();
	m_Sync.append(AuSyncItem(AuSyncItem::AtomChanged, atom));
	//qDebug() << "AuClientGlobal::AtomChanged queue" << m_Sync.size();
	m_SceneSyncLock.unlock();
}

void AuClientGlobal::SyncAtomAdded(const DaAtom* atom)
{
	m_SceneSyncLock.lock();
	m_Sync.append(AuSyncItem(AuSyncItem::AtomAdded, atom));
	//qDebug() << "AuClientGlobal::AtomAdded queue" << m_Sync.size();
	m_SceneSyncLock.unlock();
}

void AuClientGlobal::SyncAtomDeparted(const AuId& atomId)
{
	m_SceneSyncLock.lock();
	m_Sync.append(AuSyncItem(atomId));
	m_SceneSyncLock.unlock();
}

void AuClientGlobal::SyncAtomParentChanged(const DaAtom* atom)
{
	m_SceneSyncLock.lock();
	m_Sync.append(AuSyncItem(AuSyncItem::AtomParentChanged, atom));
	//qDebug() << "AuClientGlobal::AtomParentChanged queue" << m_Sync.size();
	m_SceneSyncLock.unlock();
}

void AuClientGlobal::SyncAtomsCleared()
{
	m_SceneSyncLock.lock();
	m_Sync.clear();
	m_Sync.append(AuSyncItem());
	m_SceneSyncLock.unlock();
}

unsigned int AuClientGlobal::SyncAtomCount() const
{
	m_SceneSyncLock.lock();
	unsigned int count = m_Sync.size();
	m_SceneSyncLock.unlock();
	return count;
}

void AuClientGlobal::TakeSyncQueue(QList<AuSyncItem>& sync)
{
	m_SceneSyncLock.lock();
	sync = m_Sync;
	m_Sync.clear();
	m_SceneSyncLock.unlock();
}

AuEther& AuClientGlobal::LockEtheriumDelete()
{
	if(IsSceneClient())
		m_EtheriumLock.lock();
	m_Etherium->LockDelete();
	//return AuGlobal::LockEtheriumDelete();
	return *m_Etherium;
}

void AuClientGlobal::UnlockEtheriumDelete()
{
	m_Etherium->UnlockDelete();
	//AuGlobal::UnlockEtheriumDelete();
	if(IsSceneClient())
		m_EtheriumLock.unlock();
}

size_t AuClientGlobal::DeleteEtheriumTrashAtom()
{
	if(IsSceneClient())
		m_EtheriumLock.lock();
	size_t atoms = m_Etherium->DeleteTrashAtom();
	if(IsSceneClient())
		m_EtheriumLock.unlock();
	return atoms;
}

void AuClientGlobal::ClearEtherium()
{
	if(IsSceneClient())
		m_EtheriumLock.lock();
	m_Etherium->DeleteEther();
	if(IsSceneClient())
		m_EtheriumLock.unlock();
}

bool AuClientGlobal::TestEtheriumTrashLock()
{
	if(IsSceneClient())
		m_EtheriumLock.lock();
	bool test = m_Etherium->TestTrashLock();
	if(IsSceneClient())
		m_EtheriumLock.unlock();
	return test;
}

int AuClientGlobal::TestEtheriumLock(int timeout)
{
	if(IsSceneClient())
		m_EtheriumLock.lock();
	int test = m_Etherium->TestLock(timeout);
	if(IsSceneClient())
		m_EtheriumLock.unlock();
	return test;
}

bool AuClientGlobal::BdExchangeHasEntry(quint32 designId, const AuTime& whenMod) const
{
	bool hasEntry = false;
	m_BdExchangeLock.lock();
	QHash<quint32, AuBdDesignExchangeEntryClient>::const_iterator ei = m_BdExchange.find(designId);
	if(ei != m_BdExchange.cend() && (*ei).LastMod() == whenMod)
		hasEntry = true;
	m_BdExchangeLock.unlock();
	return hasEntry;
}

bool AuClientGlobal::ScExchangeHasEntry(quint32 designId, const AuTime& whenMod) const
{
	bool hasEntry = false;
	m_ScExchangeLock.lock();
	QHash<quint32, AuScDesignExchangeEntryClient>::const_iterator ei = m_ScExchange.find(designId);
	if(ei != m_ScExchange.cend() && (*ei).LastMod() == whenMod)
		hasEntry = true;
	m_ScExchangeLock.unlock();
	return hasEntry;
}

void AuClientGlobal::BdExchangeToC(const QHash<quint32, AuTime>& designIds, QList<quint32>& designsNeeded)
{
	m_BdExchangeLock.lock();

	// Load entries from cache, if possible.
	for(QHash<quint32, AuTime>::const_iterator di = designIds.cbegin(); di != designIds.cend(); di++)
	{
		if(m_BdExchange.contains(di.key()))
			continue;	// We have that design loaded already.

		// Try loading exchange entry from cache.
		QDir cacheDir(CacheGame());
		QString cacheFileName = cacheDir.absoluteFilePath(QString("BdX%1").arg(di.key()));
		QFile cacheFile(cacheFileName);
		if(cacheFile.open(QIODevice::ReadOnly))
		{
			// Load from cache file.
			QDataStream dsfile(&cacheFile);
			dsfile.setVersion(QDataStream::Qt_4_1);

			// Read the header from the file.
			quint8 fileType;
			quint8 fileVersion;
			dsfile >> fileType;
			dsfile >> fileVersion;
			if(fileType == BD_EXCHANGE_CACHE && fileVersion == BD_EXCHANGE_VER)
			{
				// Cache file looks good.
				// Read the exchange entry from the file.
				AuBdDesignExchangeEntry item;
				QImage designThumb;
				dsfile >> item;
				dsfile >> designThumb;

				// Ask the server for their empire data, for the name.
				const DgData& gd = LockDataForRead();
				bool hasEmpire = gd.HasEmpire(item.PublisherEmpire());
				UnlockData();
				if(!hasEmpire)
					auApp->EmpireRequested(item.PublisherEmpire());

				m_BdExchange.insert(item.DesignId(), AuBdDesignExchangeEntryClient(item, designThumb));
			}

			// Close the file.
			cacheFile.close();
		}
	}

	// Remove unwanted entries.
	QHash<quint32, AuBdDesignExchangeEntryClient>::iterator ei = m_BdExchange.begin();
	while(ei != m_BdExchange.end())
	{
		QHash<quint32, AuTime>::const_iterator di = designIds.find(ei.key());
		if(di != designIds.cend() && (*ei).LastMod() == *di)
			ei++;							// The exchange entry is good.
		else
			ei = m_BdExchange.erase(ei);	// The exchange entry is unwanted. Discard it.
	}

	// List the design ids that are needed.
	for(QHash<quint32, AuTime>::const_iterator di = designIds.cbegin(); di != designIds.cend(); di++)
		if(!m_BdExchange.contains(di.key()))
			designsNeeded.append(di.key());

	m_BdExchangeLock.unlock();
}

void AuClientGlobal::ScExchangeToC(const QHash<quint32, AuTime>& designIds, QList<quint32>& designsNeeded)
{
	m_ScExchangeLock.lock();

	// Load entries from cache, if possible.
	for(QHash<quint32, AuTime>::const_iterator di = designIds.cbegin(); di != designIds.cend(); di++)
	{
		if(m_ScExchange.contains(di.key()))
			continue;	// We have that design loaded already.

		// Try loading exchange entry from cache.
		QDir cacheDir(CacheGame());
		QString cacheFileName = cacheDir.absoluteFilePath(QString("ScX%1").arg(di.key()));
		QFile cacheFile(cacheFileName);
		if(cacheFile.open(QIODevice::ReadOnly))
		{
			// Load from cache file.
			QDataStream dsfile(&cacheFile);
			dsfile.setVersion(QDataStream::Qt_4_1);

			// Read the header from the file.
			quint8 fileType;
			quint8 fileVersion;
			dsfile >> fileType;
			dsfile >> fileVersion;
			if(fileType == SC_EXCHANGE_CACHE && fileVersion == SC_EXCHANGE_VER)
			{
				// Cache file looks good.
				// Read the exchange entry from the file.
				AuScDesignExchangeEntry item;
				QImage designThumb;
				dsfile >> item;
				dsfile >> designThumb;

				// Ask the server for their empire data, for the name.
				const DgData& gd = LockDataForRead();
				bool hasEmpire = gd.HasEmpire(item.PublisherEmpire());
				UnlockData();
				if(!hasEmpire)
					auApp->EmpireRequested(item.PublisherEmpire());

				m_ScExchange.insert(item.DesignId(), AuScDesignExchangeEntryClient(item, designThumb));
			}

			// Close the file.
			cacheFile.close();
		}
	}

	// Remove unwanted entries.
	QHash<quint32, AuScDesignExchangeEntryClient>::iterator ei = m_ScExchange.begin();
	while(ei != m_ScExchange.end())
	{
		QHash<quint32, AuTime>::const_iterator di = designIds.find(ei.key());
		if(di != designIds.cend() && (*ei).LastMod() == *di)
			ei++;							// The exchange entry is good.
		else
			ei = m_ScExchange.erase(ei);	// The exchange entry is unwanted. Discard it.
	}

	// List the design ids that are needed.
	for(QHash<quint32, AuTime>::const_iterator di = designIds.cbegin(); di != designIds.cend(); di++)
		if(!m_ScExchange.contains(di.key()))
			designsNeeded.append(di.key());

	m_ScExchangeLock.unlock();
}

void AuClientGlobal::SaveSolitaire(QDataStream& out)
{
	quint8 ver = 4;
	out << ver;
	out << s_NextSingleUserAtomId;
	out << s_NextBuildingDesignId;
	out << s_NextEmpireId;
	out << s_NextItemDesignId;
	out << s_NextMailMessageId;
	out << s_NextSpaceShipDesignId;
	out << s_NextStoryId;
	out << m_Stories;

	out << LockMailBuffer("AuClientGlobal::SaveSolitaire");
	UnlockMailBuffer("AuClientGlobal::SaveSolitaire");

	LockJobOffersForRead();
	out << m_JobOffers;
	UnlockJobOffers();
}

void AuClientGlobal::RestoreSolitaire(QDataStream& in)
{
	quint8 ver;
	in >> ver;
	in >> s_NextSingleUserAtomId;
	in >> s_NextBuildingDesignId;
	if(ver >= 4)
		in >> s_NextEmpireId;
	else
		s_NextEmpireId = 1000;
	in >> s_NextItemDesignId;
	if(ver >= 2)
		in >> s_NextMailMessageId;
	in >> s_NextSpaceShipDesignId;
	in >> s_NextStoryId;
	in >> m_Stories;
	if(ver >= 1)
	{
		in >> LockMailBuffer("AuClientGlobal::RestoreSolitaire");
		UnlockMailBuffer("AuClientGlobal::RestoreSolitaire");

		if(ver < 3)
		{
			// Oops. Temporarily stored henge destinations but they aren't needed.
			AuHengeDestinations hd;
			in >> hd;
		}

		LockJobOffersForWrite();
		in >> m_JobOffers;
		UnlockJobOffers();
	}
}

void AuClientGlobal::LoadEther(QDataStream& in)
{
	Q_ASSERT(m_Etherium);
	in >> *m_Etherium;
}

void AuClientGlobal::RateBlueprintBd(const AuExchangeRatingRec& rating)
{
	m_RatingsLock.lock();
	m_BdRatings.append(rating);
	m_RatingsLock.unlock();
}

void AuClientGlobal::RateBlueprintSc(const AuExchangeRatingRec& rating)
{
	m_RatingsLock.lock();
	m_ScRatings.append(rating);
	m_RatingsLock.unlock();
}

AuStory* AuClientGlobal::LockStory(quint32 storyInstance)
{
	m_rwStories.lockForRead();					// Insure stories container does not change while the story is locked.
	return m_Stories.LockStory(storyInstance);	// Return the story pointer once the lock is acquired.
}

void AuClientGlobal::UnlockStory(quint32 storyInstance)
{
	m_Stories.UnlockStory(storyInstance);		// Unlock the story mutex.
	m_rwStories.unlock();						// Unlock the stories container.
}

bool AuClientGlobal::IsStoryActive(quint32 storyInstance)
{
	m_rwStories.lockForRead();
	bool isActive = (m_Stories.find(storyInstance) == m_Stories.cend()) ? false : true;
	m_rwStories.unlock();
	return isActive;
}

void AuClientGlobal::RemoveStory(quint32 storyInstance)
{
	qDebug() << "AuClientGlobal::RemoveStory" << storyInstance;
}

void AuClientGlobal::ReloadStory(quint32 storyInstance)
{
	qDebug() << "AuClientGlobal::ReloadStory" << storyInstance;
}

QString AuClientGlobal::StoryAuthor(quint32 storyInstance)
{
	// Determine the script id of the story instance.
	unsigned int scriptId;
	m_rwStories.lockForRead();
	AuStories::const_iterator it = m_Stories.find(storyInstance);
	if(it != m_Stories.cend())
		scriptId = (*it).second->ScriptId();
	else
		scriptId = 0;
	m_rwStories.unlock();

	// Get the story author from the script.
	QString author;
	if(scriptId)
	{
		m_rwScripts.lockForRead();
		AuScripts::const_iterator sit = m_Scripts.find(scriptId);
		if(sit != m_Scripts.cend())
			author = (*sit).second.Author();
		m_rwScripts.unlock();
	}
	return author;
}

QString AuClientGlobal::StoryBrief(quint32 storyInstance)
{
	// Determine the script id of the story instance.
	unsigned int scriptId;
	m_rwStories.lockForRead();
	AuStories::const_iterator it = m_Stories.find(storyInstance);
	if(it != m_Stories.cend())
		scriptId = (*it).second->ScriptId();
	else
		scriptId = 0;
	m_rwStories.unlock();

	// Get the story brief from the script.
	QString brief;
	if(scriptId)
	{
		m_rwScripts.lockForRead();
		AuScripts::const_iterator sit = m_Scripts.find(scriptId);
		if(sit != m_Scripts.cend())
			brief = (*sit).second.Brief();
		m_rwScripts.unlock();
	}
	return brief;
}

QString AuClientGlobal::StoryTitle(quint32 storyInstance)
{
	// Determine the script id of the story instance.
	unsigned int scriptId;
	m_rwStories.lockForRead();
	AuStories::const_iterator it = m_Stories.find(storyInstance);
	if(it != m_Stories.cend())
		scriptId = (*it).second->ScriptId();
	else
		scriptId = 0;
	m_rwStories.unlock();

	// Get the story title from the script.
	QString title;
	if(scriptId)
	{
		m_rwScripts.lockForRead();
		AuScripts::const_iterator sit = m_Scripts.find(scriptId);
		if(sit != m_Scripts.cend())
			title = (*sit).second.Title();
		m_rwScripts.unlock();
	}
	return title;
}

void AuClientGlobal::StoryAdded(quint32 storyInstance)
{
	Q_UNUSED(storyInstance);
	//qDebug() << "AuClientGlobal::StoryAdded" << storyInstance;
}

void AuClientGlobal::StoryChanged(quint32 storyInstance)
{
	Q_UNUSED(storyInstance);
	//qDebug() << "AuClientGlobal::StoryChanged" << storyInstance;
}

void AuClientGlobal::StoryDeleted(quint32 storyInstance)
{
	Q_UNUSED(storyInstance);
	//qDebug() << "AuClientGlobal::StoryDeleted" << storyInstance;
}

void AuClientGlobal::LoadScripts()
{
	m_rwScripts.lockForWrite();
	m_Scripts.LoadAllLib();
	m_rwScripts.unlock();
}

void AuClientGlobal::StoryMaintenance()
{
	m_rwStories.lockForWrite();					// Insure stories container does not change while the story is locked.
	m_Stories.ClearDefunct();
	m_rwStories.unlock();						// Unlock the stories container.
}

void AuClientGlobal::InitNextIds()
{
	const DgData& bdd = LockBdDesignsForRead();
	s_NextBuildingDesignId = bdd.m_BdDesigns->SoloInitNextId();
	UnlockBdDesigns();

	const DgData& gd = LockDataForRead();
	s_NextEmpireId = gd.m_gEmpires.HighestId() + 100;
	UnlockData();

	const DgData& idd = LockItDesignsForRead();
	s_NextItemDesignId = idd.m_ItDesigns->SoloInitNextId();
	UnlockItDesigns();

	s_NextMailMessageId = 1000000;

	s_NextSingleUserAtomId = AuId::FirstUnreserved();

	const DgData& sdd = LockScDesignsForRead();
	s_NextSpaceShipDesignId = sdd.m_ScDesigns->SoloInitNextId();
	UnlockScDesigns();

	s_NextStoryId = 1000;
#if !defined(QT_NO_DEBUG)
	qDebug() << "D AuClientGlobal::InitNextIds bd" << s_NextBuildingDesignId << "sc" << s_NextSpaceShipDesignId << "emp"
		<< s_NextEmpireId << "atom" << s_NextSingleUserAtomId << "story" << s_NextStoryId;
#endif
}

void AuClientGlobal::AddTicker(const AuId& atomId, quint8 atomType, const AuId& systemId)
{
	if(IsSceneSolo())
		AuGlobal::AddTicker(atomId, atomType, systemId);
}

void AuClientGlobal::UpdateOrAddTicker(const AuId& atomId, quint8 atomType, const AuId& systemId)
{
	if(IsSceneSolo())
		AuGlobal::UpdateOrAddTicker(atomId, atomType, systemId);
}

void AuClientGlobal::AddTocker(const AuId& atomId, quint8 atomType, const AuId& systemId, unsigned int desiredCycleMs)
{
	if(IsSceneSolo())
		AuGlobal::AddTocker(atomId, atomType, systemId, desiredCycleMs);
}

void AuClientGlobal::UpdateOrAddTocker(const AuId& atomId, quint8 atomType, const AuId& systemId, unsigned int desiredCycleMs)
{
	if(IsSceneSolo())
		AuGlobal::UpdateOrAddTocker(atomId, atomType, systemId, desiredCycleMs);
}

