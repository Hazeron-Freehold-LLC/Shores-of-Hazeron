//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: main.cpp,v 1.512 2022/05/06 16:10:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//
#include "aubddesign.h"
#include "aubdreport.h"
#include "auatomres.h"
#include "aubodypartsres.h"
#include "aubodyrenderwidget.h"
#include "aucity.h"
#include "auexchangerating.h"
#include "aufilever.h"
#include "augl.h"
#include "auglobal.h"
//#include "aupatron.h"
#include "auscdesign.h"
#include "auscparts.h"
#include "auscreport.h"
#include "austubglobal.h"
#include "daatomlist.h"
//#include "dacitcharacter.h"
//#include "dacitcrew.h"
//#include "dacitindigenous.h"
//#include "dacitofficer.h"
//#include "dacitpassenger.h"
//#include "dacittroop.h"
//#include "dacitworker.h"
#include "dagalaxy.h"
#include "daitem.h"
#include "danpc.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "davehicle.h"
#include "davessel.h"
#include "daworldgasgiant.h"
#include "daworldglobe.h"
#include "daworldring.h"
#include "daworldringsection.h"
#include "dgdata.h"
#include "dgempire.h"
#include "dtmgrchannel.h"
#include "dzspecies.h"
#include "hashpack.h"
#include <iostream>
#include <QCoreApplication>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTextStream>
#if defined(Q_OS_LINUX)
#include "../../linuxPlatform.h"
#endif

class AuEmpireReportItem;

#define d_Prefix				" "
#define d_PrefixStep			"  "

class AuTestCopy
{
public:
	int				m_Value;

	AuTestCopy()
	{
		m_Value = 0;
		qDebug() << "AuTestCopy()" << this;
	}

	AuTestCopy(int v)
	{
		m_Value = v;
		qDebug() << "AuTestCopy(v)" << this << m_Value;
	}

	AuTestCopy(const AuTestCopy& c)
	{
		m_Value = c.m_Value;
		qDebug() << "COPY AuTestCopy(copy) copied" << this << m_Value << "c" << &c;
	}

	const AuTestCopy& operator=(const AuTestCopy& c)
	{
		m_Value = c.m_Value;
		qDebug() << "AuTestCopy::operator=(copy) copied" << this << m_Value << "c" << &c;
		return *this;
	}

	void ValueArg(AuTestCopy c)
	{
		qDebug() << "AuTestCopy::ValueArg(c)" << this << m_Value << "c" << &c << "cv" << c.m_Value;
	}

	void ConstValueArg(const AuTestCopy c)
	{
		qDebug() << "AuTestCopy::ConstValueArg(c)" << this << m_Value << "c" << &c << "cv" << c.m_Value;
	}

	void RefArg(AuTestCopy& c)
	{
		qDebug() << "AuTestCopy::RefArg(c)" << this << m_Value << "c" << &c << "cv" << c.m_Value;
	}

	void ConstRefArg(const AuTestCopy& c)
	{
		qDebug() << "AuTestCopy::ConstRefArg(c)" << this << m_Value << "c" << &c << "cv" << c.m_Value;
	}
};

class AuCityFounder
{
public:
	AuId			m_CityId;
	QString			m_CityName;
	quint32			m_EmpireId;
	AuId			m_FounderId;
	QString			m_FounderName;
	FOUNDER_STATUS	m_FounderStatus;
	INDUSTRYID		m_IndustryId;

public:
	inline AuCityFounder()
	{
		m_EmpireId = 0;
		m_FounderStatus = FOUNDER_Unknown;
		m_IndustryId = IID_None;
	}

	inline AuCityFounder(const AuCityFounder& c)
	{
		m_CityId = c.m_CityId;
		m_CityName = c.m_CityName;
		m_EmpireId = c.m_EmpireId;
		m_FounderId = c.m_FounderId;
		m_FounderName = c.m_FounderName;
		m_FounderStatus = c.m_FounderStatus;
		m_IndustryId = c.m_IndustryId;
	}

public:
	inline const AuCityFounder& operator=(const AuCityFounder& c)
	{
		m_CityId = c.m_CityId;
		m_CityName = c.m_CityName;
		m_EmpireId = c.m_EmpireId;
		m_FounderId = c.m_FounderId;
		m_FounderName = c.m_FounderName;
		m_FounderStatus = c.m_FounderStatus;
		m_IndustryId = c.m_IndustryId;
		return *this;
	}

public:
	inline QString FounderStatus() const
	{
		int i = m_FounderStatus;
		if(i < FOUNDER_Unknown || i >= FOUNDER_BadStance)
			i = FOUNDER_Unknown;
		QString names[] =
		{
			QObject::tr("Unknown"),
			QObject::tr("<span style=\"color: #6060ff;\">Founder's Empire</span>"),
			QObject::tr("<span style=\"color: #40ff40;\">Aligned with Friend</span>"),
			QObject::tr("<span style=\"color: #ff8220;\">Possessed by Neutral</span>"),
			QObject::tr("<span style=\"color: #ff2020;\">Occupied by Enemy</span>"),
			QObject::tr("<span style=\"color: #6060ff;\">Property of Overlord</span>"),
			QObject::tr("Founder is Free"),
			QObject::tr("Held by Forgotten Empire"),
			QObject::tr("<span style=\"color: #ffff20;\">City Empire Error</span>"),
			QObject::tr("City Has No Founder"),
			QObject::tr("<span style=\"color: #ffff20;\">Political Stance Error</span>"),
		};
		return names[i];
	}

	inline bool IsMilitary() const
	{
		return DbIndustry::IsMilitary(m_IndustryId);
	}
};

class AuOldOffice
{
public:
	// Obsolete enumeration.
	enum OFFICE
	{
		President,				// Manager of the empire.
		VicePresident,			// Galaxy manager. Shares many powers with the President. Often succeeds the president.
		Senator,				// Sector manager.
		Congressman,			// Solar system manager.
		Governor,				// World manager (planet or moon).
		Mayor,					// City manager.
		Citizen					// Not a manager.
	};

	quint32 empireId;
	QHash<AuId, QString> governs;				// Places governed, all at m_Office gov level. key=id, value=name
	OFFICE office;
};

class AuEmpireReport
{
public:
	void DecaySystems(QSqlDatabase& dbUniverse);
	void DeleteAtoms(QSqlDatabase& dbUniverse, const AuIdList& deleteIds);
	void DoCalc();
	void EmpireReport(QSqlDatabase& dbUniverse, bool reportPlayers, bool reportExchange);
	void Emoji();
	void EmojiVal(const QString& emoji);
	void FindAtom(QSqlDatabase& dbUniverse, const AuId& atomId, bool reportGear, bool reportMemory);
	void FindBrokenHenges(QSqlDatabase& dbUniverse);
	void FixCompression(QSqlDatabase& dbUniverse);
	void FixConditions(QSqlDatabase& dbUniverse);
	void FixGeoFeatures(QSqlDatabase& dbUniverse);
	void FixPirateDNA(QSqlDatabase& dbUniverse);
	void FindPerson(QSqlDatabase& dbUniverse, const QString& personName, bool reportGear, bool reportMemory);
	void FixBdExchange(QSqlDatabase& dbUniverse);
	void FixCityItemStacks(QSqlDatabase& dbUniverse);
	void FixDb(QSqlDatabase& dbUniverse);
	void FixItems(QSqlDatabase& dbUniverse);
	/*
	void FixNpcCharacters(QSqlDatabase& dbUniverse);
	void FixNpcCrew(QSqlDatabase& dbUniverse);
	void FixNpcIndigenous(QSqlDatabase& dbUniverse);
	void FixNpcOfficers(QSqlDatabase& dbUniverse);
	void FixNpcPassengers(QSqlDatabase& dbUniverse);
	void FixNpcTroops(QSqlDatabase& dbUniverse);
	void FixNpcWorkers(QSqlDatabase& dbUniverse);
	*/
	void FixScExchange(QSqlDatabase& dbUniverse);
	void FixSpawnCitySystems(QSqlDatabase& dbUniverse);
	QString MakeSafeKey(const QString& key);
	//void PatreonCSV(const QString& fileName);
	void PirateUbiquity();
	GALAXY QueryParentGalaxyRecursive(QSqlDatabase& dbUniverse, const AuId& atomId);
	AuId QueryParentSectorRecursive(QSqlDatabase& dbUniverse, const AuId& atomId, QString& sectorName);
	AuId QueryParentSystemRecursive(QSqlDatabase& dbUniverse, const AuId& atomId, QString& systemName);
	void QueryWorldBds(QSqlDatabase& dbUniverse);
	void ReadCeruleas(QSqlDatabase& dbUniverse);
	void ReadCityReportFile();
	void ReadCityReportInventory(QSqlDatabase& dbUniverse);
	void ReadShipCargo(QSqlDatabase& dbUniverse);
	void ReportActiveAccounts(QSqlDatabase& dbLogin);
	void ReportArchitect(QSqlDatabase& dbUniverse, const AuId& archId);
	AuId ReportAtom(QSqlDatabase& dbUniverse, const AuId& atomId, bool includeGear, bool includeMemory, const char* prefix=d_Prefix,
			const char* prefixStep=d_PrefixStep);
	void ReportBdDesigns(QSqlDatabase& dbUniverse, bool sizeCheckOnly, bool fixEm);
	void ReportDesign(QSqlDatabase& dbUniverse, quint32 designId);
	void ReportDesignMedia(QSqlDatabase& dbUniverse, quint32 designId);
	void ReportItemPossession(QSqlDatabase& dbUniverse, COMMODITYID cid);
	void ReportShipMission(QSqlDatabase& dbUniverse, const AuId& shipId);
	void ReportCommodities();
	void ReportCommoditiesWiki();
	void ReportDecaySystems(QSqlDatabase& dbUniverse);
	void ReportDesignExchange(QSqlDatabase& dbUniverse);
	void ReportDuplicateAtomIds(QSqlDatabase& dbUniverse);
	void ReportDuplicateSectors(QSqlDatabase& dbUniverse);
	void ReportIndustries();
	void ReportIndustriesWiki();
	void ReportItem(QTextStream& so, const AuEmpireReportItem& item, const QList<AuEmpireReportItem>& reportItems, int treeDepth,
			int itemRank);
	void ReportLimboAtoms(QSqlDatabase& dbUniverse);
	void ReportLostShips(QSqlDatabase& dbUniverse, AuIdList& limboRescueMe);
	void ReportManufacturingProcesses(QSqlDatabase& dbUniverse);
	void ReportMultiplePrimaries(QSqlDatabase& dbUniverse);
	void ReportPlayers(QSqlDatabase& dbUniverse);
	void ReportBadSectorCoords(QSqlDatabase& dbUniverse);
	void ReportScDesigns(QSqlDatabase& dbUniverse, bool sizeCheckOnly, bool fixEm);
	void ReportScPref(QSqlDatabase& dbUniverse, const QString& empireName);
	void ReportSystem(QSqlDatabase& dbUniverse, const QString& systemName);
	void ReportSystem(QSqlDatabase& dbUniverse, const AuId& systemId);
	void ReportWorld(QSqlDatabase& dbUniverse, const QString& worldName);
	void ReportWorld(QSqlDatabase& dbUniverse, const AuId& worldId);
	void ReportTroops(QSqlDatabase& dbUniverse);
	void RestoreItems(QSqlDatabase& dbUniverse, const AuId& atomId, const QString& fileName);
	void RestoreSeaChests(QSqlDatabase& dbUniverse, const QString& fileName);
	void SaveEmpire(quint32 empireId);
	void SaveItems(QSqlDatabase& dbUniverse, const AuId& atomId);
	void SaveBdDesign(QSqlDatabase& dbUniverse, quint32 designId);
	void SaveScDesign(QSqlDatabase& dbUniverse, quint32 designId);
	void SaveSeaChests(QSqlDatabase& dbUniverse);
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Hazeron blue colors: "#193680" "#142b66" "#0f204d" "#0a1533" "#050a19"
const QString hazBlue5("#193680");
const QString hazBlue4("#142b66");
const QString hazBlue3("#0f204d");
const QString hazBlue2("#0a1533");
const QString hazBlue1("#050a19");

// Commonly used HTML fragments.
QString s_htmlHeader = "<html>\n"
	"<style>\n"
	".ll "
	"{ "
	" text-decoration: none; "
	"} "
	""
	".ll:hover "
	"{ "
	" -webkit-transition-duration: .4s; "
	" transition-duration: .4s; "
	" color: #ffffff; "
	"} "

	".a1 "
	"{ "
		"border-style: solid; "
		"border-color: #000000; "
		"border-radius: 8px; "
		"background-color: #000000; "
		"background-image: url(../images/hazBlackbutton.png); "
		"background-size: cover; "
		"padding: 4px 32px; "
		"text-align: center; "
		"text-decoration: none; "
		"display: inline-block; "
		"font-size: 12px; "
	"}\n"

	".a1:hover "
	"{ "
		"-webkit-transition-duration: .4s; "
		"transition-duration: .4s; "
		"border-style: solid; "
		"border-color: #fffcb9 #a3772f #7a4f0c #fedc69; "
	"}\n"
	"</style>\n";

QString s_bodyStart("<body style=\"color: rgb(112, 127, 255); background-color: rgb(0, 0, 0); font-family: Helvetica, Arial, "
"sans-serif;\" alink=\"#ffffff\" lang=\"EN-US\" link=\"#a0b3f9\" vlink=\"#a0a0a0\">\n"
"<?php include(\"../sohpageheader.html\"); ?><br>");

QString s_htmlFooter("</body></html>\n");
QString s_titleBar("<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"><title>Shores of Hazeron - %1</title></head>\n"
		"<link rel=\"shortcut icon\" href=\"https://hazeron.com/favicon.png\" type=\"image/png\">\n");
QString s_PageTitle("<div style=\"text-align: center; font-family: sans-serif;\"><big style=\"font-weight: bold; font-family: sans-serif\">"
	"<br><span style=\"color: #ffffff;\">Empire Standings</span></big><br><small><small>%1 &middot Updated Every 20 Minutes</small></small></div>");
QString s_UniTotals("<b>Universe Totals</b>"
	"<div style=\"margin-left: 10px;\">"
	"<table><tbody>"
	"<tr><td><small>"
	"Avatars<br>"
	"Officers<br>"
	"Crew<br>"
	"Troops<br>"
	"Passengers<br>"
	"Workers<br>"
	"Indigenous<br>"
	"Spacecraft<br>"
	"Vehicles<br>"
	"Cities<br>"
	"Buildings<br>"
	"Asteroids<br>"
	"Solar Systems<br>"
	"Sectors<br>"
	"Galaxies<br>"
	"Universe"
	"</small></td>"
	"<td style=\"text-align: right;\"><small>"
	"%L1<br>"
	"%L2<br>"
	"%L3<br>"
	"%L4<br>"
	"%L5<br>"
	"%L6<br>"
	"%L7<br>"
	"%L8<br>"
	"%L9<br>"
	"%L10<br>"
	"%L11<br>"
	"%L12<br>"
	"%L13<br>"
	"%L14<br>"
	"20<br>"
	"1"
	"</small></td></tr>"
	"</tbody></table>"
	"</div>");
QString s_RankSummary("<p><b>%L1 Ranked Empires<br>%L2 Total Empires</b><br>"
	"<small>"
	"To be ranked, an empire must have an <a class=\"ll\" href=\"https://hazeron.com/wiki/index.php/Capital_City\">empire capital city</a>. "
	"It must also have cities or bases in at least two solar systems. "
	"</p>"
	"<p>"
	"Allies (+) and vassals (-) are indented beneath their sworn empire to show their contribution. "
	"The score is not necessarily the sum of the system counts because systems can be shared by more than one empire. "
	"</small></p>");
QString s_tableStart = QString("<table style=\"text-align: left; width: 100%; background-color: %1; font-family: "
		"sans-serif;\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\n<tbody>").arg(hazBlue2);
QString s_tableHeader = QString("<tr>"
	"<td style=\"vertical-align: top; background-color: %1;\"><small><b></b></small><br></td>"
	"<td style=\"vertical-align: top; background-color: %1; width: 66px;\"><small><b>Flag</b></small><br></td>"
	"<td style=\"vertical-align: top; background-color: %1;\"><small><b>Name</b></small><br></td>"
	"<td style=\"vertical-align: top; background-color: %1;\"><small><b>Leadership</b></small><br></td>"
	"<td style=\"vertical-align: top; background-color: rgb(100,32,16);\"><small><b>Solar Systems</b></small><br></td>"
	"</tr>\n").arg(hazBlue5);
QString s_tableEnd("</tbody>\n</table>\n");
QString s_tableRow = "<tr>"
	"<td style=\"vertical-align: top;\"><small>%50</small><br></td>"	// rank
	"<td style=\"vertical-align: top; \"><img height=\"%5\" width=\"%6\" src=\"%1\"><br></td>"	// flag
	"<td style=\"vertical-align: top;\"><small>%2</small><br></td>"	// name
	"<td style=\"vertical-align: top;\"><small>%3<br>%4</small><br></td>"	// emperor and heir
	"<td style=\"vertical-align: top; background-color: %30;\"><small>%L15</small><br></td>"	// systems
	"</tr>\n";
QString s_tableRowVassal = "<tr>"
	"<td style=\"vertical-align: top;\"><small><small>%50</small></small><br></td>"	// rank
	"<td style=\"align: right; vertical-align: top; \"><img height=\"%5\" width=\"%6\" src=\"%1\"><br></td>"	// flag
	"<td style=\"vertical-align: top;\"><small><small>%2</small></small><br></td>"	// name
	"<td style=\"vertical-align: top;\"><small><small>%3<br>%4</small></small><br></td>"	// emperor and heir
	"<td style=\"vertical-align: top; background-color: %30;\"><small><small>%L15</small></small><br></td>"	// systems
	"</tr>\n";

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuCityData : public AuCity
{
public:
	quint32								m_AllegEmpire;			// empire of allegiance
	bool								m_IsContested;			// true if city owner does not own all the flags
	int									m_Population;			// value is population

	AuCityData()
	{
		m_AllegEmpire = 0;
		m_IsContested = false;
		m_Population = 0;
	}

	AuCityData(CITYTYPE cityType, const AuId& cityId, const QString& cityName,
			const AuId& planetId, const QString& planetName,
			const AuId& systemId, const QString& systemName, const AuPoint& systemLoc,
			const AuId& sectorId, const QString& sectorName,
			const AuId& galaxyId, GALAXY galaxy, bool isSectorCapital, bool isGalaxyCapital,
			quint32 allegEmpire, bool isContested, int population) :
		AuCity(cityType, cityId, cityName, AuId(), QString(), planetId, planetName, systemId, systemName, systemLoc, sectorId,
				sectorName, galaxyId, galaxy, isSectorCapital, isGalaxyCapital)	// founder not needed
	{
		m_AllegEmpire = allegEmpire;
		m_IsContested = isContested;
		m_Population = population;
	}
};

class AuGalaxyData
{
public:
	GALAXY								m_Galaxy;

	QString Name() const
	{
		return GalaxyName(m_Galaxy);
	}
};

class AuPlanetData
{
public:
	QString								m_Name;
	AuId								m_SystemId;
};

class AuSystemData
{
public:
	QString								m_Name;
	AuPoint								m_Loc;
	AuId								m_SectorId;
};

class AuSectorData
{
public:
	AuPoint								m_Loc;
	QString								m_Name;
	AuId								m_GalaxyId;

public:
	AuPoint SectorLoc() const
	{
		return (m_Loc - AuPoint(5,5,5))/10.0;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuCityItem
{
public:
	quint32								m_AllegEmpire;			// Empire the city is allegiant to.
	AuId								m_Id;
	bool								m_IsContested;			// True if the owner empire does not own all the flags.
	bool								m_IsOccupied;			// True if the owner empire is an occupier.
	QString								m_Name;
	qint32								m_Population;

	AuCityItem()
	{
		m_AllegEmpire = 0;
		m_IsContested = false;
		m_IsOccupied = false;
		m_Population = 0;
	}
};

class AuSpaceShip
{
public:
	QString		m_Name;
	QString		m_Design;
	quint8		m_Quality;

	AuSpaceShip()
	{
		m_Quality = 1;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuEmpireReportItem
{
public:
	AuId		m_CapitolCity;
	QList<AuCityItem>	m_Cities;		// cities and their populations and occupation status
	QString		m_Emperor;		// title and name of emperor
	quint32		m_EmpireId;		// empire record id
	QString		m_FlagFile;		// name of flag file
	QSize		m_FlagSize;
	QMap<AuId, QString>	m_Galaxies;		// galaxy names
	QString		m_Heirs;		// person names of vice presidents/heirs, separated by <br>
	QList<AuCityItem>	m_LostCities;		// lost but still allegiant cities and their populations and occupation status
	QString		m_Name;			// empire name
	quint32		m_Overlord;		// Overlord empire for vassals.
	quint8		m_Tribute;		// Tribute percent to overlord.
	QMap<AuId, QString>	m_Planets;		// planet names
	QList<AuCitizen>	m_Players;	// avatars
	bool		m_ScoreKnown;	// True after scores have been tallied.
	QList<AuId>	m_ScoreSystems;	// Total score including vassals.
	QMap<AuId, QString>	m_Sectors;		// sector names
	std::unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>	m_Stance;	// political stance toward known empires. key is empire id
	QMap<AuId, QString>	m_Systems;		// system names
	QList<quint32>	m_Vassals;	// Empire ids that are vassals to this empire.

	AuEmpireReportItem()
	{
		m_EmpireId = 0;
		m_Overlord = 0;
		m_ScoreKnown = false;
		m_Tribute = 0;
	}

	AuEmpireReportItem(const AuEmpireReportItem& c) :
		m_Cities(c.m_Cities),
		m_Galaxies(c.m_Galaxies),
		m_LostCities(c.m_LostCities),
		m_Planets(c.m_Planets),
		m_Players(c.m_Players),
		m_ScoreSystems(c.m_ScoreSystems),
		m_Sectors(c.m_Sectors),
		m_Systems(c.m_Systems),
		m_Vassals(c.m_Vassals)
	{
		m_Emperor = c.m_Emperor;
		m_EmpireId = c.m_EmpireId;
		m_FlagFile = c.m_FlagFile;
		m_FlagSize = c.m_FlagSize;
		m_Heirs = c.m_Heirs;
		m_Name = c.m_Name;
		m_Overlord = c.m_Overlord;
		m_ScoreKnown = c.m_ScoreKnown;
		m_Tribute = c.m_Tribute;
	}

public:	// Operators
	const AuEmpireReportItem& operator=(const AuEmpireReportItem& c)
	{
		m_Cities = c.m_Cities;
		m_Galaxies = c.m_Galaxies;
		m_LostCities = c.m_LostCities;
		m_Planets = c.m_Planets;
		m_Players = c.m_Players;
		m_ScoreSystems = c.m_ScoreSystems;
		m_Sectors = c.m_Sectors;
		m_Systems = c.m_Systems;
		m_Vassals = c.m_Vassals;
		m_Emperor = c.m_Emperor;
		m_EmpireId = c.m_EmpireId;
		m_FlagFile = c.m_FlagFile;
		m_FlagSize = c.m_FlagSize;
		m_Heirs = c.m_Heirs;
		m_Name = c.m_Name;
		m_Overlord = c.m_Overlord;
		m_ScoreKnown = c.m_ScoreKnown;
		m_Tribute = c.m_Tribute;
		return *this;
	}

public:	// Methods
	// Returns true if this empire is big enough to be listed as a top level empire.
	inline bool IsTopLevelEmpire() const
	{
		if(m_Overlord)
			return false;	// I am a vassal.
		if(!m_Vassals.empty() && m_Systems.size())
			return true;	// I have vassals
		if(m_ScoreSystems.size() < 2)
			return false;	// I do not have cities in at least two solar systems.
		return true;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

int main(int argc, char** argv)
{
	QCoreApplication a(argc, argv);
	QStringList args = a.arguments();

	// Create an object from each shared library that will persist through the life of the program. Otherwise, these libraries
	// get thrashed loading and unloading every time a resource is needed on Windows, which makes the program load very slowly.
	AuAtomRes atomRes;
	AuBodyPartsRes bodyPartsRes;

	DgData data;
	gp_AuGlobal = new AuStubGlobal("Shores of Hazeron Empire Report", "Hazeron Freehold", AuGlobal::WebCGI, &data);
	AuGLModelCache modelCacheGLSL;
	g_Au.SetModelCache(&modelCacheGLSL);

	/*
	int i = 0;
	for(int b=1; b<25; b++)
		i += b;
	qDebug() << "i" << i << "takes" << (i * 1000. / 60. / 60. / 24.) << "days to reach Q25 from Q1, using" << (1000/60)
		<< "min steps";
	i = 0;
	for(int b=1; b<84; b++)
		i += b;
	qDebug() << "i" << i << "takes" << (i * 1000. / 60. / 60. / 24.) << "days to reach Q84 from Q1, using" << (1000/60)
		<< "min steps";
	for(int b=1; b<134; b++)
		i += b;
	qDebug() << "i" << i << "takes" << (i * 1000. / 60. / 60. / 24.) << "days to reach Q134 from Q1, using" << (1000/60)
		<< "min steps";
	i = 0;
	for(int b=1; b<255; b++)
		i += b;
	qDebug() << "i" << i << "takes" << (i * 1000. / 60. / 60. / 24.) << "days to reach Q255 from Q1, using" << (1000/60)
		<< "min steps";
		*/

	/*
	// Quality progression.
	for(int q=1; q<=255; q++)
		qDebug() << "Q" << q << DbCommodity::QualityDesc(q) << "QBf" << QualityBoostF(q) << "QBi" << QualityBoosted(100, q);
		*/
	/*
	int q=255;
	int sub = 0;
	int step=0;
	while(q > 0)
	{
		qDebug() << "step" << (step+1) << "Q" << q;
		sub += 8;
		q -= sub;
		step++;
	}
	*/
	// Hull size progression by tech level.
	/*
	double vol = 1000;	// TL1 hull.
	double f = 1.0325;	// TL32 result is 121791292
	for(double tl=0; tl<32; tl++)
	{
		qDebug() << (quint32)vol << ",	// TL" << (tl+1);
		vol = pow(vol, f);
	}
	*/

	if(args.contains("-?") || args.contains("-help"))
	{
usage:
		qDebug() << "Usage: auempirereport [-?] [-help] [-report] [-atom id] [-limbo] [-person name]";
		qDebug() << "  Specify only one database operation to perform.";
		qDebug() << "  -? or -help - Displays this helpful information.";
		qDebug() << "  -db - Displays the host name of the auUniverse SQL server.";
		qDebug() << "";
		qDebug() << "  Queries";
		qDebug() << "  -arch atomId|stringId - Reports information about all designs created by a particular architect.";
		qDebug() << "  -atom atomId|stringId - Searches the database for an atom by numeric or string";
		qDebug() << "            atom id. Reports parent atom chain up to universe.";
		qDebug() << "  -cities - Reports information about cities.";
		qDebug() << "  -commodities - Shows all known commodities and base costs.";
		qDebug() << "  -emoji - Reads the emoji-sequences.txt file and displays the result.";
		qDebug() << "  -emojival char - Reports the emoji sequence for the 'char'.";
		qDebug() << "  -findmedia designId - Find design media in people's inventory.";
		qDebug() << "  -GD - Creates the GD? game data file in" << g_Au.CacheGame() << ".";
		qDebug() << "  -gear - Reports gear for subsequent people, e.g. -gear -person.";
		qDebug() << "  -ceru - Reports gear saved in table ceruleas.";
		qDebug() << "  -huey - Query database on Huey instead of UniverseDb.Hazeron.net.";
		qDebug() << "  -id atomId|stringId - Shows the string and numeric versions of an atom id.";
		qDebug() << "  -industries - Shows all known industries.";
		qDebug() << "  -industriesWiki - Shows all known industries.";
		qDebug() << "  -memory - Reports memory for subsequent people, e.g. -memory -person.";
		qDebug() << "  -person personName - Searches the database for all people with a certain name.";
		qDebug() << "            Reports parent atom chain up to universe.";
		qDebug() << "            CAVEAT: Does not work on names with double-spaces between words.";
		qDebug() << "            CAVEAT: Must be last option; name is assembled from remaining command line.";
		qDebug() << "  -q - Shows the quality boost progression.";
		qDebug() << "  -report - Produces empire standings report, avatar pages and blueprint exchange for web page. Check usage for";
		qDebug() << "            how this works.";
		qDebug() << "  -saveempire empireId - Saves an empire record to a file in the current folder, named empireId.emp.";
		qDebug() << "  -saveitems atomId|stringId - Saves all item atoms subordinate to atomId, to a file in the current folder.";
		qDebug() << "  -solo - Saves the data used to init the solo game.";
		qDebug() << "  -restoreitems atomId|stringId fileName - Restores item atoms from a file and makes them subordinate to atomId.";
		qDebug() << "  -saveseachests - Saves all sea chests to the file SeaChests.bin in the current folder.";
		qDebug() << "  -restoreseachests fileName - Restores all sea chests from a file.";
		qDebug() << "  -savebd designId - Saves a Bd blueprint to a file in the current folder.";
		qDebug() << "  -savesc designId - Saves an Sc blueprint to a file in the current folder.";
		qDebug() << "  -shipdesign designId - Reports brief information about a blueprint, either Sc or Dn type.";
		qDebug() << "  -shipmission atomId - Dumps the current mission of a ship.";
		qDebug() << "  -system systemName - Searches the database for all solar systems with a certain name.";
		qDebug() << "            Reports parent atom chain up to universe.";
		qDebug() << "            Reports atoms subordinate to the solar system.";
		qDebug() << "            CAVEAT: Does not work on names with double-spaces between words.";
		qDebug() << "            CAVEAT: Must be last option; name is assembled from remaining command line.";
		qDebug() << "  -systemid atomId|stringId - Same as -system only using the atom id instead of the name.";
		qDebug() << "  -world worldName - Searches the database for all worlds with a certain name.";
		qDebug() << "            Reports parent atom chain up to universe.";
		qDebug() << "            Reports geological features of the world.";
		qDebug() << "            Reports atoms subordinate to the world.";
		qDebug() << "            CAVEAT: Does not work on names with double-spaces between words.";
		qDebug() << "            CAVEAT: Must be last option; name is assembled from remaining command line.";
		qDebug() << "  -worldid atomId|stringId - Searches the database for the world with a certain id.";
		qDebug() << "            Reports parent atom chain up to universe.";
		qDebug() << "            Reports geological features of the world.";
		qDebug() << "            Reports atoms subordinate to the world.";
		qDebug() << "  -troops - Reports information about troops.";
		qDebug() << "  -u0 - Query database on u0.Hazeron.net instead of UniverseDb.Hazeron.net.";
		qDebug() << "  -ubiquity - Shows pirate ubiquity values for each galaxy.";
		qDebug() << "";
		//qDebug() << "  Patreon";
		//qDebug() << "  -patreon [file.csv] - Parses a Patreon patrons CSV file, to create the auutil/data/patrons.txt file.";
		//qDebug() << "            The default file is ~/Downloads/Members_5145026.csv";
		//qDebug() << "";
		qDebug() << "  Data integrity validation";

		qDebug() << "  -badbd - Reports bd designs that are not ok to manufacture. ";
		qDebug() << "            Lists building atoms and exchange records that reference bad designs.";
		qDebug() << "  -badbdfix - Reports bd designs that are not ok to manufacture. ";
		qDebug() << "            Saves bd designs to .SoH files that are not ok to manufacture. ";
		qDebug() << "            Lists building atoms and exchange records that reference bad designs. ";
		qDebug() << "            Deletes bad designs and the building atoms and exchange records that reference those designs. ";
		qDebug() << "  -badbdsize - Reports bd designs that are not ok to manufacture due to uncompressed size limit.";

		qDebug() << "  -badsc - Reports sc designs that are not ok to manufacture. ";
		qDebug() << "            Lists ship atoms and exchange records that reference bad designs.";
		qDebug() << "  -badscfix - Reports sc designs that are not ok to manufacture. ";
		qDebug() << "            Saves sc designs to .SoH files that are not ok to manufacture. ";
		qDebug() << "            Lists ship atoms and exchange records that reference bad designs. ";
		qDebug() << "            Deletes bad designs and the ship atoms and exchange records that reference those designs. ";
		qDebug() << "  -badscsize - Reports sc designs that are not ok to manufacture due to uncompressed size limit.";

		qDebug() << "  -dupids - Checks for duplicate atom ids.";
		qDebug() << "  -dupsectors - Checks for duplicate sectors at the same location.";
		qDebug() << "  -limbo - Reports person and officer atoms that do not have a parent atom chain";
		qDebug() << "            up to the universe. Also reports spacecraft in solar systems with no";
		qDebug() << "            primary star. AuIdList stored in 'rescue.bin'.";
		qDebug() << "  -lost - Reports spacecraft atoms in solar systems with no primary star.";
		qDebug() << "  -mproc - Reports manufacturing processes.";
		qDebug() << "  -multistars - Checks for solar systems with multiple primary stars.";
		qDebug() << "  -sectorcoords - Checks for sectors with invalid coordinates.";
		qDebug() << "  -systemdecay - Checks for solar systems that are ready to decay.";
		qDebug() << "";
		qDebug() << "  Maintenance";
		qDebug() << "  -calc - Runs the current DoCalc function. Check the code to see what this does.";
		qDebug() << "  -decaysystems - Deletes solar systems that are ready to decay.";
		qDebug() << "  -fixbdex - Updates building design exchange. Removes invalid designs.";
		qDebug() << "  -fixdb - Runs the current FixDb function. It might be wise to check the code to see what this does.";
		qDebug() << "  -fixscex - Updates spacecraft design exchange. Removes invalid designs.";
	}
	else
	{
		QSqlDatabase dbUniverse = QSqlDatabase::addDatabase("QPSQL7", "AuUniverse");
		if(!dbUniverse.isValid())
			exit(0);
		// auUniverse database
		//   Owner: aumaster
		//   Password: Light
		//   User: aumaster
		//   Password: Light
		dbUniverse.setDatabaseName("auUniverse");			// NOTE: Name is case sensitive.
		if(args.contains("-huey"))
		{
			if(args.contains("-db"))
				qDebug() << "dbUniverse hostName Huey";
			dbUniverse.setHostName("Huey");
		}
		else if(args.contains("-u0"))
		{
			if(args.contains("-db"))
				qDebug() << "dbUniverse hostName u0.Hazeron.net";
			dbUniverse.setHostName("u0.Hazeron.net");
		}
		else
		{
			if(args.contains("-db"))
				qDebug() << "dbUniverse hostName UniverseDb.Hazeron.net";
			dbUniverse.setHostName("UniverseDb.Hazeron.net");
		}
		if(!dbUniverse.open("aumaster","Light"))
		{
			QSqlError er = dbUniverse.lastError();
			QString sdv = er.driverText();
			QString sdb = er.databaseText();
			qWarning("Failed to open database connection:\n%s\n%s\n",
				(const char *)sdv.toLatin1(),
				(const char *)sdb.toLatin1());
			exit(1);
		}

		// Login database is only opened in response to certain queries.
		QSqlDatabase dbLogin = QSqlDatabase::addDatabase("QPSQL7", "AuLogin");
		if(!dbLogin.isValid())
			exit(1);
		dbLogin.setDatabaseName("auLogin");			// NOTE: Name is case sensitive.
		dbLogin.setHostName("LoginDb.Hazeron.net");
		//qDebug() << "Databases opened.";

		bool didSomething = false;
		AuEmpireReport report;
		if(args.contains("-args"))
		{
			qDebug() << "Arguments:" << args;
			qDebug() << "argc" << argc << "argv...";
			for(int ai=0; ai<argc; ai++)
				qDebug() << " " << argv[ai];
			didSomething = true;
		}
		else if(args.contains("-acct"))
		{
			if(!dbLogin.open("aumaster","Light"))
			{
				QSqlError er = dbLogin.lastError();
				QString sdv = er.driverText();
				QString sdb = er.databaseText();
				qWarning("Failed to open database connection:\n%s\n%s\n",
					(const char *)sdv.toLatin1(),
					(const char *)sdb.toLatin1());
				exit(1);
			}
			report.ReportActiveAccounts(dbLogin);
			didSomething = true;
		}
		else if(args.contains("-dupids"))
		{
			report.ReportDuplicateAtomIds(dbUniverse);
			didSomething = true;
		}
		else if(args.contains("-dupsectors"))
		{
			report.ReportDuplicateSectors(dbUniverse);
			didSomething = true;
		}
		else if(args.contains("-lost"))
		{
			AuIdList lostShips;
			report.ReportLostShips(dbUniverse, lostShips);
			didSomething = true;
		}
		else if(args.contains("-multistars"))
		{
			report.ReportMultiplePrimaries(dbUniverse);
			didSomething = true;
		}
		else if(args.contains("-qgen"))
		{
			// This code generates the quality boost factors table in audef.h.
			const float maxBoost = 2.5f;
			const float minBoost = .9f;
			qDebug() << "// Table generated by auempirereport -qgen";
			qDebug() << "float qBoostF[256] =";
			qDebug() << "{";
			qDebug() << "0.f , // Quality zero is invalid.";	// q0
			for(int quality=1; quality<=255; quality++)
			{
				float boostF;
				{
					// The range is .95x at Q1 to 1.5x at Q255.
					//return .95f + (float)(quality-1) * .55f / 254.;						// .95x >= x >= 1.5x
					// Some players felt 1.5x was too little for the time needed to reach Q255.
					//return 1.f + (float)(quality - QualityBaseline()) / (float)(255 - QualityBaseline());	// .87x >= x >= 2.x
					int x = quality - QualityBaseline();
					if(x >= 0)
					{
						float r = (float)x / (float)(255 - QualityBaseline());
						float ang = -acosf(r);
						boostF = (sinf(ang) + 1.f) * (maxBoost - 1.f) + 1.f;
					}
					else
					{
						float r = (float)x / (float)(QualityBaseline() - 1);
						float ang = acosf(r);
						boostF = (sinf(ang) - 1.f) * (1.f - minBoost) + 1.f;
					}
				}
				qDebug() << boostF << "f ,";
			}
			qDebug() << "};";
			didSomething = true;
		}
		else if(args.contains("-q"))
		{
			// Quality progression.
			for(int q=1; q<=255; q++)
				qDebug() << "Q" << q << DbCommodity::QualityDesc(q) << "QBf" << QualityBoostF(q) << "QBi" << QualityBoosted(100, q);

			int q=255;
			int sub = 0;
			int step=0;
			while(q > 0)
			{
				qDebug() << "step" << (step+1) << "Q" << q;
				sub += 8;
				q -= sub;
				step++;
			}
			didSomething = true;
		}
		else if(args.contains("-scpref"))
		{
			// Look for an empire by name.
			int argidx = args.indexOf("-scpref");

			if(argidx+1 < args.count())
			{
				// All remaining args are assembled into the empire's name.
				argidx++;
				QString empireName = args[argidx++];
				while(argidx < args.count())
					empireName += " " + args[argidx++];

				report.ReportScPref(dbUniverse, empireName);
				didSomething = true;
			}
		}
		else if(args.contains("-id"))
		{
			// Decode an atom id.
			int argidx = args.indexOf("-id");
			if(argidx+1 < args.count())
			{
				QString sid = args[argidx+1];
				AuId atomId;
				int idnum = QVariant(sid).toInt();
				if(idnum)
					atomId = AuId(idnum);	// It looks like we got a number, which could not be a string id..
				else
					atomId = AuId(sid);		// Try to interpret as a string id.
				qDebug() << "AtomId:" << atomId << atomId.StringId();
				didSomething = true;
			}
		}
		/*
		else if(args.contains("-patreon"))
		{
			// Parse a Patreon .csv file.
			QString fileName("/home/daveh/Downloads/Members_5145026.csv");	// Default patrons file.
			int argidx = args.indexOf("-patreon");
			if(argidx+1 < args.count())
				fileName = args[argidx+1];
			report.PatreonCSV(fileName);
			didSomething = true;
		}
		*/
		else
		{
			// All remaining commands need game data and ship design records.
			// Load game data.
			data.Load();
			//qDebug() << "Data loaded.";

			bool reportGear = args.contains("-gear");
			bool reportMemory = args.contains("-mem") || args.contains("-memory");
					
			if(args.contains("-commodities"))
			{
				report.ReportCommodities();
				didSomething = true;
			}
			else if(args.contains("-ceru"))
			{
				report.ReadCeruleas(dbUniverse);
				didSomething = true;
			}
			else if(args.contains("-industries"))
			{
				report.ReportIndustries();
				didSomething = true;
			}
			else if(args.contains("-industriesWiki"))
			{
				report.ReportIndustriesWiki();
				didSomething = true;
			}
			else if(args.contains("-commoditiesWiki"))
			{
				report.ReportCommoditiesWiki();
				didSomething = true;
			}
			else if(args.contains("-mproc"))
			{
				report.ReportManufacturingProcesses(dbUniverse);
				didSomething = true;
			}
			else if(args.contains("-saveitems"))
			{
				int argidx = args.indexOf("-saveitems");
				if(argidx+1 < args.count())
				{
					QString sid = args[argidx+1];
					AuId atomId;
					int idnum = QVariant(sid).toInt();
					if(idnum)
						atomId = AuId(idnum);	// It looks like we got a number, which could not be a string id..
					else
						atomId = AuId(sid);		// Try to interpret as a string id.
					report.SaveItems(dbUniverse, atomId);
					didSomething = true;
				}
			}
			else if(args.contains("-saveempire"))
			{
				int argidx = args.indexOf("-saveempire");
				if(argidx+1 < args.count())
				{
					QString sid = args[argidx+1];
					unsigned int empireId = QVariant(sid).toUInt();
					report.SaveEmpire(empireId);
					didSomething = true;
				}
			}
			else if(args.contains("-restoreitems"))
			{
				int argidx = args.indexOf("-restoreitems");
				if(argidx+1 < args.count() + 1)
				{
					QString sid = args[argidx+1];
					AuId atomId;
					int idnum = QVariant(sid).toInt();
					if(idnum)
						atomId = AuId(idnum);	// It looks like we got a number, which could not be a string id..
					else
						atomId = AuId(sid);		// Try to interpret as a string id.

					QString fileName = args[argidx+2];
					report.RestoreItems(dbUniverse, atomId, fileName);
					didSomething = true;
				}
			}
			else if(args.contains("-saveseachests"))
			{
				report.SaveSeaChests(dbUniverse);
				didSomething = true;
			}
			else if(args.contains("-restoreseachests"))
			{
				int argidx = args.indexOf("-restoreseachests");
				if(argidx+1 < args.count() + 1)
				{
					QString fileName = args[argidx+1];
					report.RestoreSeaChests(dbUniverse, fileName);
					didSomething = true;
				}
			}
			else if(args.contains("-badbd"))
			{
				report.ReportBdDesigns(dbUniverse, false, false);
				didSomething = true;
			}
			else if(args.contains("-badbdfix"))
			{
				report.ReportBdDesigns(dbUniverse, false, true);
				didSomething = true;
			}
			else if(args.contains("-badbdsize"))
			{
				report.ReportBdDesigns(dbUniverse, true, false);
				didSomething = true;
			}
			else if(args.contains("-badsc"))
			{
				report.ReportScDesigns(dbUniverse, false, false);
				didSomething = true;
			}
			else if(args.contains("-badscfix"))
			{
				report.ReportScDesigns(dbUniverse, false, true);
				didSomething = true;
			}
			else if(args.contains("-badscsize"))
			{
				report.ReportScDesigns(dbUniverse, true, false);
				didSomething = true;
			}
			else if(args.contains("-troops"))
			{
				report.ReportTroops(dbUniverse);
				didSomething = true;
			}
			else if(args.contains("-emoji"))
			{
				report.Emoji();
				didSomething = true;
			}
			else if(args.contains("-emojival"))
			{
				int argidx = args.indexOf("-emojival");
				if(argidx+1 < args.count())
				{
					QString sid = args[argidx+1];
					report.EmojiVal(sid);
					didSomething = true;
				}
			}
			else if(args.contains("-calc") || args.contains("-docalc"))
			{
				report.DoCalc();
				didSomething = true;
			}
			else if(args.contains("-ubiquity"))
			{
				report.PirateUbiquity();
				didSomething = true;
			}
			else if(args.contains("-fixdb"))
			{
				// Do the fix...or fix the doo?
				report.FixDb(dbUniverse);
				didSomething = true;
			}
			else if(args.contains("-fixbdex"))
			{
				report.FixBdExchange(dbUniverse);
				didSomething = true;
			}
			else if(args.contains("-fixscex"))
			{
				report.FixScExchange(dbUniverse);
				didSomething = true;
			}
			else if(args.contains("-savebd"))
			{
				// We have already loaded the spacecraft design records.
				int argidx = args.indexOf("-savebd");
				if(argidx+1 < args.count())
				{
					QString sid = args[argidx+1];
					quint32 designId = QVariant(sid).toInt();
					if(designId)
					{
						report.SaveBdDesign(dbUniverse, designId);
						didSomething = true;
					}
				}
			}
			else if(args.contains("-savesc"))
			{
				// We have already loaded the spacecraft design records.
				int argidx = args.indexOf("-savesc");
				if(argidx+1 < args.count())
				{
					QString sid = args[argidx+1];
					quint32 designId = QVariant(sid).toInt();
					if(designId)
					{
						report.SaveScDesign(dbUniverse, designId);
						didSomething = true;
					}
				}
			}
			else if(args.contains("-findmedia"))
			{
				int argidx = args.indexOf("-findmedia");
				if(argidx+1 < args.count())
				{
					QString sid = args[argidx+1];
					quint32 designId = QVariant(sid).toUInt();
					if(designId)
					{
						report.ReportDesignMedia(dbUniverse, designId);
						didSomething = true;
					}
				}
			}
			else if(args.contains("-findcid"))
			{
				int argidx = args.indexOf("-findcid");
				if(argidx+1 < args.count())
				{
					QString sid = args[argidx+1];
					quint32 cid = QVariant(sid).toUInt();
					if(cid)
					{
						report.ReportItemPossession(dbUniverse, (COMMODITYID)cid);
						didSomething = true;
					}
				}
			}
			else if(args.contains("-shipdesign"))
			{
				int argidx = args.indexOf("-shipdesign");
				if(argidx+1 < args.count())
				{
					QString sid = args[argidx+1];
					quint32 designId = QVariant(sid).toUInt();
					if(designId)
					{
						report.ReportDesign(dbUniverse, designId);
						didSomething = true;
					}
				}
			}
			else if(args.contains("-shipmission"))
			{
				int argidx = args.indexOf("-shipmission");
				if(argidx+1 < args.count())
				{
					QString sid = args[argidx+1];
					AuId atomId;
					int idnum = QVariant(sid).toInt();
					if(idnum)
						atomId = AuId(idnum);	// It looks like we got a number, which could not be a string id..
					else
						atomId = AuId(sid);		// Try to interpret as a string id.
					if(!atomId.IsNull())
					{
						report.ReportShipMission(dbUniverse, atomId);
						didSomething = true;
					}
				}
			}
			else if(args.contains("-GD"))
			{
				data.SaveBasicGameData();
				qDebug() << "Game data file saved in:" << QString("%1/GD1").arg(g_Au.CacheGame());
				didSomething = true;
			}
			else if(args.contains("-solo"))
			{
				data.SaveSolitaireInit(dbUniverse);
				qDebug() << "Game solo init file saved in:" << QString("%1/GS").arg(g_Au.CacheGame());
				didSomething = true;
			}
			else if(args.contains("-systemdecay"))
			{
				report.ReportDecaySystems(dbUniverse);
				didSomething = true;
			}
			else if(args.contains("-sectorcoords"))
			{
				report.ReportBadSectorCoords(dbUniverse);
				didSomething = true;
			}
			else if(args.contains("-decaysystems"))
			{
				report.DecaySystems(dbUniverse);
				didSomething = true;
			}
			else if(args.contains("-arch"))
			{
				// Look for an atom by id.
				int argidx = args.indexOf("-arch");
				if(argidx+1 < args.count())
				{
					QString sid = args[argidx+1];
					AuId atomId;
					int idnum = QVariant(sid).toInt();
					if(idnum)
						atomId = AuId(idnum);	// It looks like we got a number, which could not be a string id..
					else
						atomId = AuId(sid);		// Try to interpret as a string id.
					report.ReportArchitect(dbUniverse, atomId);
					didSomething = true;
				}
			}
			else if(args.contains("-atom"))
			{
				// Look for an atom by id.
				int argidx = args.indexOf("-atom");
				if(argidx+1 < args.count())
				{
					QString sid = args[argidx+1];
					AuId atomId;
					int idnum = QVariant(sid).toInt();
					if(idnum)
						atomId = AuId(idnum);	// It looks like we got a number, which could not be a string id..
					else
						atomId = AuId(sid);		// Try to interpret as a string id.
					report.FindAtom(dbUniverse, atomId, reportGear, reportMemory);
					didSomething = true;
				}
			}
			else if(args.contains("-person"))
			{
				// Look for a person by name.
				int argidx = args.indexOf("-person");

				if(argidx+1 < args.count())
				{
					// All remaining args are assembled into the person's name.
					// CAVEAT: This doesn't work for names with double-spaces, like 'Ainbheartach  O'Connor'.
					argidx++;
					QString personName = args[argidx++];
					while(argidx < args.count())
						personName += " " + args[argidx++];

					report.FindPerson(dbUniverse, personName, reportGear, reportMemory);
					didSomething = true;
				}
			}
			else if(args.contains("-system"))
			{
				// Look for a solar system by name.
				int argidx = args.indexOf("-system");

				if(argidx+1 < args.count())
				{
					// All remaining args are assembled into the person's name.
					// CAVEAT: This doesn't work for names with double-spaces, like 'Ainbheartach  O'Connor'.
					argidx++;
					QString name = args[argidx++];
					while(argidx < args.count())
						name += " " + args[argidx++];
					
					report.ReportSystem(dbUniverse, name);
					didSomething = true;
				}
			}
			else if(args.contains("-systemid"))
			{
				// Look for an atom by id.
				int argidx = args.indexOf("-systemid");
				if(argidx+1 < args.count())
				{
					QString sid = args[argidx+1];
					AuId atomId;
					int idnum = QVariant(sid).toInt();
					if(idnum)
						atomId = AuId(idnum);	// It looks like we got a number, which could not be a string id..
					else
						atomId = AuId(sid);		// Try to interpret as a string id.
					report.ReportSystem(dbUniverse, atomId);
					didSomething = true;
				}
			}
			else if(args.contains("-world"))
			{
				// Look for a world by name.
				int argidx = args.indexOf("-world");

				if(argidx+1 < args.count())
				{
					// All remaining args are assembled into the person's name.
					// CAVEAT: This doesn't work for names with double-spaces, like 'Ainbheartach  O'Connor'.
					argidx++;
					QString name = args[argidx++];
					while(argidx < args.count())
						name += " " + args[argidx++];
					
					report.ReportWorld(dbUniverse, name);
					didSomething = true;
				}
			}
			else if(args.contains("-worldid"))
			{
				// Look for an atom by id.
				int argidx = args.indexOf("-worldid");
				if(argidx+1 < args.count())
				{
					QString sid = args[argidx+1];
					AuId atomId;
					int idnum = QVariant(sid).toInt();
					if(idnum)
						atomId = AuId(idnum);	// It looks like we got a number, which could not be a string id..
					else
						atomId = AuId(sid);		// Try to interpret as a string id.
					report.ReportWorld(dbUniverse, atomId);
					didSomething = true;
				}
			}
			else if(args.contains("-limbo"))
			{
				// Report people and officers who appear to be in limbo.
				report.ReportLimboAtoms(dbUniverse);
				didSomething = true;
			}
			else if(args.contains("-report"))
			{
				// Generate the universe empire standings report for the web site.
				report.EmpireReport(dbUniverse, true, true);
				didSomething = true;
			}
		}

		// We're done with the databases now.
		dbUniverse.close();
		if(dbLogin.isOpen())
			dbLogin.close();
		//qDebug() << "Databases closed.";

		if(!didSomething)
			goto usage;
	}

	//qDebug() << "Done.";
	exit(0);
}

void AuEmpireReport::EmpireReport(QSqlDatabase& dbUniverse, bool reportPlayers, bool reportExchange)
{
	bool traceReport = false;

	if(traceReport)
		qDebug() << "EmpireReport Started" << QDateTime::currentDateTime();
	if(reportPlayers)
	{
		if(traceReport)
			qDebug() << "Reporting Players" << QDateTime::currentDateTime();
		ReportPlayers(dbUniverse);
	}

	if(reportExchange)
	{
		if(traceReport)
			qDebug() << "Reporting Blueprint Exchange" << QDateTime::currentDateTime();
		ReportDesignExchange(dbUniverse);
	}

	// Construct the AuCity array by querying the database.
	if(traceReport)
		qDebug() << "Constructing city data." << QDateTime::currentDateTime();
	QHash<AuId,bool> planetIds;				// key is planet id. value is not used
	QMultiMap<quint32, AuCityData> cities;	// key is empire id
	{
		// Prepare a query to select all of the capitol records in the database so we can construct the AuCityData array.
		// We do not count military bases as cities in this report.
		QString sq;
		sq = QString("SELECT id,idp,cityName,allegiance,cityPop FROM a_Bd WHERE industry=%1 OR industry=%2;")
			.arg(IID_Capitol)
			.arg(IID_BaseHeadquarters);
		QSqlQuery qryCities(QString(), dbUniverse);
		qryCities.prepare(sq);
		qryCities.exec();
		while(qryCities.next())
		{
			int qv=0;
			AuId idCity = AuId(qryCities.value(qv++).toUInt());
			AuId idPlanet = AuId(qryCities.value(qv++).toUInt());
			QString sCity = qryCities.value(qv++).toString();
			quint32 empireId = qryCities.value(qv++).toInt();
			int cityPop = qryCities.value(qv++).toInt();

			const DgData& gd = g_Au.LockDataForRead();
			bool isGalaxyCapital = gd.IsEmpireGalaxyCapital(empireId, idCity);
			bool isSectorCapital = gd.IsEmpireSectorCapital(empireId, idCity);
			g_Au.UnlockData();

			QString sPlanet, sSystem, sSector;
			AuId idSystem, idSector, idGalaxy;
			AuPoint systemLoc;
			GALAXY galaxy = GALAXY_Count;
			cities.insert(empireId, AuCityData(CITY_Capitol, idCity, sCity, idPlanet, sPlanet, idSystem, sSystem, systemLoc,
						idSector, sSector, idGalaxy, galaxy, isSectorCapital, isGalaxyCapital, empireId, false, cityPop));

			// List planets that need to be queried.
			planetIds.insert(idPlanet, true);
		}
	}

	// Load information about all the planets that host cities.
	QHash<AuId, AuPlanetData> planets;	// key is planet id
	QHash<AuId, AuSystemData> systems;	// key is solar system id
	QHash<AuId, AuSectorData> sectors;	// key is sector id
	QHash<AuId, AuGalaxyData> galaxies;	// key is galaxy id
	{
		QString sq = QString("SELECT id,name,ids FROM a_World WHERE");
		for(QHash<AuId, bool>::const_iterator pi = planetIds.cbegin(); pi != planetIds.cend(); pi++)
		{
			if(pi != planetIds.cbegin())
				sq += " or";
			sq += QString(" id=%1").arg(pi.key().GetID());
		}
		sq += ";";
		if(traceReport)
			qDebug() << "Collecting data about" << planetIds.count() << "planets";
		QSqlQuery qryPlanet(QString(), dbUniverse);
		qryPlanet.prepare(sq);
		qryPlanet.exec();
		while(qryPlanet.next())
		{
			AuPlanetData planet;

			int qv=0;
			quint32 planetId = qryPlanet.value(qv++).toUInt();
			planet.m_Name = qryPlanet.value(qv++).toString();
			planet.m_SystemId = AuId(qryPlanet.value(qv++).toUInt());
			Q_ASSERT(!planet.m_SystemId.IsNull());

			planets.insert(planetId, planet);
			if(traceReport)
				qDebug() << " planet" << planet.m_Name << planets.count() << "of" << planetIds.count();

			if(!systems.contains(planet.m_SystemId))
			{
				// Get information about the solar system.
				sq = QString("SELECT idp,name,locX,locY,locZ from a_solarsystem WHERE id=%1").arg(planet.m_SystemId.GetID());
				QSqlQuery qrySystem(QString(), dbUniverse);
				qrySystem.prepare(sq);
				qrySystem.exec();
				if(qrySystem.next())
				{
					AuSystemData system;

					int qv=0;
					system.m_SectorId = AuId(qrySystem.value(qv++).toUInt());
					system.m_Name = qrySystem.value(qv++).toString();
					system.m_Loc.x = qrySystem.value(qv++).toDouble();
					system.m_Loc.y = qrySystem.value(qv++).toDouble();
					system.m_Loc.z = qrySystem.value(qv++).toDouble();
					// Get Location in Sector Coordinates
					system.m_Loc = system.m_Loc/5.0;
					system.m_Loc = system.m_Loc/20.0;

					systems.insert(planet.m_SystemId, system);
					if(traceReport)
						qDebug() << " system" << system.m_Name;

					if(!sectors.contains(system.m_SectorId))
					{
						// Prepare the final query to determine the name of the sector.
						sq = QString("SELECT idp,name,locX,locY,locZ FROM a_Sector WHERE id=%1;").arg(system.m_SectorId.GetID());
						QSqlQuery qrySector(QString(), dbUniverse);
						qrySector.prepare(sq);
						qrySector.exec();
						if(qrySector.next())
						{
							AuSectorData sector;

							int qv=0;
							sector.m_GalaxyId = AuId(qrySector.value(qv++).toUInt());
							sector.m_Name = qrySector.value(qv++).toString();
							sector.m_Loc.x = qrySector.value(qv++).toDouble();
							sector.m_Loc.y = qrySector.value(qv++).toDouble();
							sector.m_Loc.z = qrySector.value(qv++).toDouble();

							sectors.insert(system.m_SectorId, sector);
							if(traceReport)
								qDebug() << " sector" << sector.m_Name;

							if(!galaxies.contains(sector.m_GalaxyId))
							{
								// Prepare the final query to determine the name of the sector.
								sq = QString("SELECT galaxy FROM a_Galaxy WHERE id=%1;").arg(sector.m_GalaxyId.GetID());
								QSqlQuery qryGalaxy(QString(), dbUniverse);
								qryGalaxy.prepare(sq);
								qryGalaxy.exec();
								if(qryGalaxy.next())
								{
									AuGalaxyData galaxy;

									int qv=0;
									galaxy.m_Galaxy = (GALAXY)qryGalaxy.value(qv++).toInt();

									galaxies.insert(sector.m_GalaxyId, galaxy);
									if(traceReport)
										qDebug() << " galaxy" << galaxy.Name();
								}
							}
						}
					}
				}
			}
		}
	}
	if(traceReport)
		qDebug() << "Planet and sector data constructed." << planets.count() << "planets," << sectors.count() << "sectors," << galaxies.count() << "galaxies." << QDateTime::currentDateTime();

	// Fill out information in the city data using the planet and sector data.
	for(QMultiMap<quint32, AuCityData>::iterator ci = cities.begin(); ci != cities.end(); ci++)
	{
		QHash<AuId, AuPlanetData>::const_iterator pi = planets.constFind((*ci).PlanetId());
		if(pi != planets.cend())
		{
			QHash<AuId, AuSystemData>::const_iterator si = systems.constFind((*pi).m_SystemId);
			if(si != systems.cend())
			{
				QHash<AuId, AuSectorData>::const_iterator ti = sectors.constFind((*si).m_SectorId);
				if(ti != sectors.cend())
				{
					QHash<AuId, AuGalaxyData>::const_iterator gi = galaxies.constFind((*ti).m_GalaxyId);
					if(gi != galaxies.cend())
					{
						if(traceReport)
							qDebug() << " City" << (*ci).CityName() << "planet" << (*pi).m_Name << "system" << (*si).m_Name << "sector" << (*ti).m_Name << "galaxy" << (*gi).Name();
						(*ci).SetPlanetName((*pi).m_Name);
						(*ci).SetSystemId((*pi).m_SystemId);
						(*ci).SetSystemName((*si).m_Name);
						(*ci).SetSectorId((*si).m_SectorId);
						(*ci).SetSectorName((*ti).m_Name);
						(*ci).SetGalaxyId((*ti).m_GalaxyId);
						(*ci).SetGalaxy((*gi).m_Galaxy);
					}
				}
			}
		}
	}
	if(traceReport)
		qDebug() << "City planet info collected." << QDateTime::currentDateTime();

	// Misc. totals
	quint32 avatarCount = 0;
	quint32 bdCount = 0;
	quint32 crewCount = 0;
	quint32 indCount = 0;
	quint32 officerCount = 0;
	quint32 passengerCount = 0;
	quint32 roidCount = 0;
	quint32 sectorCount = 0;
	quint32 spacecraftCount = 0;
	quint32 systemCount = 0;
	quint32 troopCount = 0;
	quint32 vehicleCount = 0;
	quint32 workerCount = 0;
	{
		QSqlQuery qsc("SELECT count(id) FROM a_Sc WHERE idp > 0 AND (state = 0 or state = 2);", dbUniverse);
		if(qsc.first())
			spacecraftCount = qsc.value(0).toUInt();

		QSqlQuery qveh("SELECT count(id) FROM a_Vehicle WHERE idp > 0;", dbUniverse);
		if(qveh.first())
			vehicleCount = qveh.value(0).toUInt();

		QSqlQuery qcc("SELECT count(id) FROM a_Person WHERE idp > 0;", dbUniverse);
		if(qcc.first())
			avatarCount = qcc.value(0).toUInt();

		QSqlQuery qroid("SELECT count(id) FROM a_Asteroid WHERE idp > 0;", dbUniverse);
		if(qroid.first())
			roidCount = qroid.value(0).toUInt();

		QSqlQuery qss("SELECT count(id) FROM a_SolarSystem WHERE idp > 0;", dbUniverse);
		if(qss.first())
			systemCount = qss.value(0).toUInt();

		QSqlQuery qsec("SELECT count(id) FROM a_Sector WHERE idp > 0 AND id IN (SELECT idp FROM a_Atom);", dbUniverse);
		if(qsec.first())
			sectorCount = qsec.value(0).toUInt();

		QSqlQuery qoff(QString("SELECT count(id) FROM a_Npc WHERE idp > 0 AND role=%1;").arg(ROLE_Officer), dbUniverse);
		if(qoff.first())
			officerCount = qoff.value(0).toUInt();

		QSqlQuery qcrw(QString("SELECT count(id) FROM a_Npc WHERE idp > 0 AND role=%1;").arg(ROLE_Crew), dbUniverse);
		if(qcrw.first())
			crewCount = qcrw.value(0).toUInt();

		QSqlQuery qind(QString("SELECT count(id) FROM a_Npc WHERE idp > 0 AND (role=%1 OR role=%2 OR role=%3 OR role=%4);")
				.arg(ROLE_IndigenousPerson)
				.arg(ROLE_IndigenousElder)
				.arg(ROLE_IndigenousHealer)
				.arg(ROLE_IndigenousStargazer), dbUniverse);
		if(qind.first())
			indCount = qind.value(0).toUInt();

		QSqlQuery qtrp(QString("SELECT count(id) FROM a_Npc WHERE idp > 0 AND role=%1;").arg(ROLE_Troop), dbUniverse);
		if(qtrp.first())
			troopCount = qtrp.value(0).toUInt();

		QSqlQuery qpas(QString("SELECT count(id) FROM a_Npc WHERE idp > 0 AND role=%1;").arg(ROLE_Passenger), dbUniverse);
		if(qpas.first())
			passengerCount = qpas.value(0).toUInt();

		QSqlQuery qwrk(QString("SELECT count(id) FROM a_Npc WHERE idp > 0 AND role=%1;").arg(ROLE_Worker), dbUniverse);
		if(qwrk.first())
			workerCount = qwrk.value(0).toUInt();

		QSqlQuery qbd("SELECT count(id) FROM a_Bd WHERE idp > 0;", dbUniverse);
		if(qbd.first())
			bdCount = qbd.value(0).toUInt();
	}

	// Select data from all the empire records in the database.
	const DgData& gd = g_Au.LockDataForRead();
	int empireCount = gd.m_gEmpires.size();
	QHash<quint32, AuEmpireReportItem> reportEmpires;	// key is empire id
	for(DgEmpireHash::const_iterator ehi = gd.m_gEmpires.cbegin(); ehi != gd.m_gEmpires.cend(); ehi++)
	{
		// Construct an item record to hold the data needed for this empire in the report.
		AuId emperorId = (*ehi).second.EmperorId();
		QString emperorName = (*ehi).second.CitizenNameWithTitle(emperorId);

		AuEmpireReportItem item;
		item.m_EmpireId = (*ehi).second.m_id;
		item.m_Name = (*ehi).second.Name();
		item.m_Emperor = QString("<a class=\"ll\" href=\"https://hazeron.com/EmpireStandings/p%1.php\">%2</a>").arg(emperorId.StringId()).arg(emperorName);

		QMultiMap<QString, AuId> heirs;	// key is vice president name and value is their atom id.
		(*ehi).second.VicePresidents(heirs);
		for(QMultiMap<QString, AuId>::const_iterator hi = heirs.cbegin(); hi != heirs.cend(); hi++)
		{
			QString haddr = QString("<a class=\"ll\" href=\"https://hazeron.com/EmpireStandings/p%1.php\">%2</a>").arg((*hi).StringId()).arg(hi.key());
			if(item.m_Heirs.isEmpty())
				item.m_Heirs = haddr;
			else
				item.m_Heirs += QString("<br>") + haddr;
		}

		item.m_FlagFile = QString("%1.png").arg(item.m_EmpireId);
		(*ehi).second.m_flag.save(item.m_FlagFile);		// Store the flag into a file so we can link to it.
		item.m_FlagSize = (*ehi).second.m_flag.size();

		// Empire citizens
		for(std::unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator eci = (*ehi).second.m_Citizens.cbegin(); eci != (*ehi).second.m_Citizens.cend(); eci++)
		{
			if((*eci).second.m_id.IsNull())
				continue;	// skip those with null ids

			// Sort list by name.
			bool inserted = false;
			for(QList<AuCitizen>::iterator ci = item.m_Players.begin(); ci != item.m_Players.end(); ci++)
			{
				if((*eci).second.m_name < (*ci).m_name)
				{
					item.m_Players.insert(ci, (*eci).second);
					inserted = true;
					break;
				}
			}
			if(!inserted)
				item.m_Players.append((*eci).second);
		}

		// Political Stance
		item.m_Stance = (*ehi).second.m_Stance;
		for(std::unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::const_iterator si = item.m_Stance.cbegin();
				si != item.m_Stance.cend(); si++)
			if((*si).second.m_stance == POL_Vassal)
			{
				item.m_Overlord = (*si).second.m_id;		// Store vassal's overlord empire id to save looping later.
				item.m_Tribute = (*si).second.m_tribute;
				break;
			}

		item.m_CapitolCity = (*ehi).second.m_capitol;

		// Discard empires with no capital city that are not vassals.
		if(item.m_CapitolCity.IsNull() && !item.m_Overlord)
			continue;

		if(!cities.isEmpty())
		{
			// Tally number of different planets, systems, sectors and galaxies on which cities are owned.
			for(QMultiMap<quint32, AuCityData>::const_iterator ci = cities.find(item.m_EmpireId);
					ci != cities.cend() && ci.key() == item.m_EmpireId; ci++)
			{
				AuCityItem cit;
				cit.m_Id = (*ci).CityId();
				cit.m_Name = (*ci).CityName();
				cit.m_IsOccupied = ((*ci).m_AllegEmpire != item.m_EmpireId) ? true : false;	// Occupied if alleg differs from empire
				cit.m_IsContested = (*ci).m_IsContested;
				cit.m_Population = (*ci).m_Population;

				bool inserted = false;
				for(QList<AuCityItem>::iterator ni = item.m_Cities.begin(); ni != item.m_Cities.end(); ni++)
				{
					int comp = (*ci).CityName().compare((*ni).m_Name, Qt::CaseInsensitive);
					if(comp == 0)
						comp = (*ci).CityName().compare((*ni).m_Name, Qt::CaseSensitive);
					if(comp < 0)
					{
						item.m_Cities.insert(ni, cit);
						inserted = true;
						break;
					}
				}
				if(!inserted)
					item.m_Cities.append(cit);

				if(!item.m_Planets.contains((*ci).PlanetId()))
					item.m_Planets.insert((*ci).PlanetId(), (*ci).PlanetName());

				if(!item.m_Systems.contains((*ci).SystemId()))
					item.m_Systems.insert((*ci).SystemId(), (*ci).SystemName());

				if(!item.m_Sectors.contains((*ci).SectorId()))
					item.m_Sectors.insert((*ci).SectorId(), (*ci).SectorName());

				if(!item.m_Galaxies.contains((*ci).GalaxyId()))
					item.m_Galaxies.insert((*ci).GalaxyId(), (*ci).GalaxyName());
			}

			// Tally cities that are still allegiant to the empire but that are not owned by the empire.
			for(QMultiMap<quint32, AuCityData>::const_iterator ci = cities.cbegin(); ci != cities.cend(); ci++)
			{
				if(ci.key() == item.m_EmpireId)
					continue;	// Skip cities that are members of the current empire. They've already been tallied.
				if((*ci).m_AllegEmpire != item.m_EmpireId)
					continue;	// Skip cities that are not allegiant to the current empire we are reporting.

				AuCityItem cit;
				cit.m_Id = (*ci).CityId();
				cit.m_Name = (*ci).CityName();
				cit.m_IsOccupied = true;				// Occupied because alleg differs from empire
				cit.m_IsContested = (*ci).m_IsContested;
				cit.m_Population = (*ci).m_Population;

				bool inserted = false;
				for(QList<AuCityItem>::iterator ni = item.m_LostCities.begin(); ni != item.m_LostCities.end(); ni++)
				{
					int comp = (*ci).CityName().compare((*ni).m_Name, Qt::CaseInsensitive);
					if(comp == 0)
						comp = (*ci).CityName().compare((*ni).m_Name, Qt::CaseSensitive);
					if(comp < 0)
					{
						item.m_LostCities.insert(ni, cit);
						inserted = true;
						break;
					}
				}
				if(!inserted)
					item.m_LostCities.append(cit);
			}
		}
		reportEmpires.insert(item.m_EmpireId, item);
	}
	g_Au.UnlockData();
	if(traceReport)
		qDebug() << "Empire data collected." << QDateTime::currentDateTime();

	// Collect vassal information now that we have the entire list.
	for(QHash<quint32, AuEmpireReportItem>::iterator vi = reportEmpires.begin(); vi != reportEmpires.end(); vi++)
	{
		if((*vi).m_Overlord)
		{
			// The empire is a vassal. Find its overlord empire, which may not be in the list.
			QHash<quint32, AuEmpireReportItem>::iterator oi = reportEmpires.find((*vi).m_Overlord);
			if(oi != reportEmpires.end())
			{
				// Overlord empire was found.
				// Watch for circular relationship here. If it is circular, break the chain right now.
				QHash<quint32, bool> overlords;				// List of overlord empires to watch for a circular relationship.
				overlords.insert((*vi).m_EmpireId, true);	// Remember id of starting empire.
				bool isCircular = false;					// true if we determine this to be a circular relationship.
				QHash<quint32, AuEmpireReportItem>::iterator pi = oi;	// pi will climb the overlord hierarchy, starting at oi
				while(pi != reportEmpires.end() && (*pi).m_Overlord)
				{
					// The overlord has an overlord.
					if(overlords.contains((*pi).m_EmpireId))
					{
						// We have encountered a circular condition.
						if(traceReport)
							qDebug() << "Circular vassal relationship avoided";
						isCircular = true;
						break;
					}

					overlords.insert((*pi).m_EmpireId, true);	// Remember id of overlord empire.
					pi = reportEmpires.find((*pi).m_Overlord);
				}

				if(isCircular)
					(*vi).m_Overlord = 0;						// Overlord results in a circular relationship; must break it.
				else
					(*oi).m_Vassals.append((*vi).m_EmpireId);	// Add the vassal's id to the overlord's list of vassals.
			}
			else
			{
				// Overlord was not found in our data. Treat this like a top-level entry. Overlord must not have a capital?
				(*vi).m_Overlord = 0;
			}
		}
	}
	if(traceReport)
		qDebug() << "Vassal information tallied." << QDateTime::currentDateTime();

	// Tally scores of all report items.
	// This requires multiple passes due to ally/vassal hierarchy.
	bool anyUnknownScore;
	do
	{
		// We will do passes until scores of all empire report items are resolved.
		anyUnknownScore = false;
		for(QHash<quint32, AuEmpireReportItem>::iterator ei = reportEmpires.begin(); ei != reportEmpires.end(); ei++)
		{
			if((*ei).m_ScoreKnown)
				continue;	// Skip those whose score is known.

			// This empire's score can only be tallied if all vassal empires have been tallied.
			// Start tallying vassals until we find one that is unknown.
			bool vassalsOk = true;
			QList<AuId> scoreSystems;
			for(QList<quint32>::const_iterator vii = (*ei).m_Vassals.cbegin(); vii != (*ei).m_Vassals.cend(); vii++)
			{
				QHash<quint32, AuEmpireReportItem>::iterator vi = reportEmpires.find(*vii);
				Q_ASSERT(vi != reportEmpires.end());	// How could this happen?
				if((*vi).m_ScoreKnown)
				{
					// Tally systems to accomodate multiple empires sharing each of them.
					for(QMap<AuId, QString>::const_iterator ssi = (*vi).m_Systems.cbegin();
							ssi != (*vi).m_Systems.cend(); ssi++)
						if(!scoreSystems.contains(ssi.key()))
							scoreSystems.append(ssi.key());
				}
				else
				{
					// Found a vassal empire that does not know its score yet.
					vassalsOk = false;
					break;
				}
			}

			if(vassalsOk)
			{
				// The score of all vassals was known or I have no vassals.
				(*ei).m_ScoreKnown = true;

				// Tally systems to accomodate multiple empires sharing each of them.
				for(QMap<AuId, QString>::const_iterator ssi = (*ei).m_Systems.cbegin();
						ssi != (*ei).m_Systems.cend(); ssi++)
					if(!scoreSystems.contains(ssi.key()))
						scoreSystems.append(ssi.key());

				(*ei).m_ScoreSystems = scoreSystems;
			}
			else
				anyUnknownScore = true;
		}
	}
	while(anyUnknownScore);
	if(traceReport)
		qDebug() << "Scores tallied." << QDateTime::currentDateTime();

	// Sort the report items by score.
	QList<AuEmpireReportItem> reportItems;
	while(!reportEmpires.isEmpty())
	{
		// Locate the highest scoring item remaining in the map.
		QHash<quint32, AuEmpireReportItem>::iterator hsii = reportEmpires.end();	// highest score item iterator
		for(QHash<quint32, AuEmpireReportItem>::iterator rii = reportEmpires.begin(); rii != reportEmpires.end(); rii++)
		{
			if(rii == reportEmpires.begin())
				hsii = rii;
			else
			{
				int comp = (*rii).m_ScoreSystems.size() - (*hsii).m_ScoreSystems.size();
				if(comp > 0)
					hsii = rii;
			}
		}

		// Append the highest scoring item to the sorted list and remove it from the source list.
		reportItems.append(*hsii);
		reportEmpires.erase(hsii);
	}
	if(traceReport)
		qDebug() << "Report items sorted." << QDateTime::currentDateTime();

	// Create web pages.
	QFile sofile;
	sofile.open(stdout, QIODevice::WriteOnly);
	QTextStream so(&sofile);
	so.setCodec("UTF-8");

	so << s_htmlHeader;

	QString title = QString("Empire Standings");
	so << s_titleBar.arg(title);

	int topLevelEmpires = 0;
	for(QList<AuEmpireReportItem>::const_iterator ri = reportItems.cbegin(); ri != reportItems.cend(); ri++)
		if((*ri).IsTopLevelEmpire())
			topLevelEmpires++;

	so << s_bodyStart;
	QDateTime now = QDateTime::currentDateTimeUtc();
	so << s_PageTitle
		.arg(now.toString());

	so << "<table><tbody>";
	so << "<tr>";
	so << "<td valign=\"top\" width=\"75%\">";

	// Output the report so items are grouped by overlord/vassal.
	so << s_tableStart;
	so << s_tableHeader;
	int rank = 1;
	bool topdog = false;
	for(QList<AuEmpireReportItem>::const_iterator ri = reportItems.cbegin(); ri != reportItems.cend(); ri++)
	{
		// Only report top-level items that have vassals or cities in more than one solar system.
		if(!(*ri).IsTopLevelEmpire())
			continue;

		if(!topdog)
		{
			// Store the top empire's flag so it will appear on the home page.
			QImage flag((*ri).m_FlagFile);
			flag = flag.convertToFormat(QImage::Format_ARGB32);
			flag = flag.scaled(96,96, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			flag.save("topdog.png");
			topdog = true;
		}

		ReportItem(so, *ri, reportItems, 0, rank++);	// The item is a top-level item. Report it and all subordinate items.
	}
	so << s_tableEnd;

	so << "</td>";
	so << "<td valign=\"top\">";

	so << s_RankSummary
		.arg(topLevelEmpires)
		.arg(empireCount);

	so << s_UniTotals
		.arg(avatarCount)
		.arg(officerCount)
		.arg(crewCount)
		.arg(troopCount)
		.arg(passengerCount)
		.arg(workerCount)
		.arg(indCount)
		.arg(spacecraftCount)
		.arg(vehicleCount)
		.arg(cities.count())
		.arg(bdCount)
		.arg(roidCount)
		.arg(systemCount)
		.arg(sectorCount);

	so << "</td>";
	so << "</tr>";
	so << "</tbody>";
	so << "</table>";

	so << s_htmlFooter;
	sofile.close();
	if(traceReport)
		qDebug() << "EmpireReport Finished" << QDateTime::currentDateTime();
}

// This is a recursive call to create a tree of subordinate items below 'item'.
void AuEmpireReport::ReportItem(QTextStream& so, const AuEmpireReportItem& item, const QList<AuEmpireReportItem>& reportItems,
		int treeDepth, int itemRank)
{
	// The following code simply constructs a web page.
	QString rankField;
	if(itemRank)
		rankField = QString("%1").arg(itemRank);

	QString name;
	QString bgColor;
	QString scoreColor;
	QString scoreIndent;
	if(treeDepth)
	{
		// Vassal names are indented and not bold.
		for(int id=0; id < treeDepth; id++)
		{
			name += "&nbsp; &nbsp; ";
			scoreIndent += "&nbsp;";
		}
		//name += "&middot; ";
		if(item.m_Tribute >= 10)
		{
			// Vassal
			name += "- ";
			bgColor = "rgb(16,4,1)";
		}
		else
		{
			// Ally
			name += "+ ";
			bgColor = "rgb(32,8,2)";
		}
		name += item.m_Name;
		scoreColor = bgColor;
	}
	else
	{
		// Top level names are bold
		name = QString("<b>%1</b>").arg(item.m_Name);
		bgColor = "rgb(64,16,4)";
		scoreColor = "rgb(100, 32, 16)";
	}
	QString scoreSystems;
	if(item.m_Vassals.isEmpty() || item.m_ScoreSystems.count() == item.m_Systems.count())
	{
		scoreSystems = QString("%1%L2").arg(scoreIndent)
			.arg(item.m_ScoreSystems.count());
	}
	else
	{
		scoreSystems = QString("%1%L3<br>%1%L4").arg(scoreIndent)
			.arg(item.m_ScoreSystems.count()).arg(item.m_Systems.count());
	}

	int flagSize;
	if(treeDepth)
		flagSize = 64;
	else
		flagSize = 96;

	int flagHt = item.m_FlagSize.height();
	int flagWid = item.m_FlagSize.width();
	if(flagHt >= flagWid)
	{
		flagWid = flagSize * flagWid / flagHt;
		if(flagWid < 1)
			flagWid = 1;
		flagHt = flagSize;
	}
	else
	{
		flagHt = flagSize * flagHt / flagWid;
		if(flagHt < 1)
			flagHt = 1;
		flagWid = flagSize;
	}

	// Some wise guy named their avatar Athena%1. :/
	// All % characters are replaced with _.
	name.replace("%","_");
	QString emp(item.m_Emperor);
	emp.replace("%","_");
	QString heirs(item.m_Heirs);
	heirs.replace("%","_");

	// Add a record to our output table.
	QString tableRow;
	if(treeDepth)
		tableRow = s_tableRowVassal;
	else
		tableRow = s_tableRow;
	so << tableRow
		.arg(item.m_FlagFile)
		.arg(name)
		.arg(emp)
		.arg(heirs)
		.arg(flagHt)
		.arg(flagWid)
		.arg(scoreSystems)
		.arg(bgColor)
		.arg(rankField);

	// Output all items that are subordinate to the current item. This is recursive.
	for(QList<AuEmpireReportItem>::const_iterator ri = reportItems.cbegin(); ri != reportItems.cend(); ri++)
		if((*ri).m_Overlord == item.m_EmpireId && (*ri).m_ScoreSystems.size())
			ReportItem(so, *ri, reportItems, treeDepth+1, 0);	// The item is subordinate to the current item.
}

// 'key' is a single character that must be made safe for use in a file name and http link.
QString AuEmpireReport::MakeSafeKey(const QString& key)
{
	QString safekey(key.toLatin1());
	if(safekey == "*")
		safekey = "Asterisk";
	else if(safekey == "?")
		safekey = "QuestionMark";
	else if(safekey == "{")
		safekey = "LeftBrace";
	else if(safekey == "}")
		safekey = "RightBrace";
	else if(safekey == "[")
		safekey = "LeftBracket";
	else if(safekey == "]")
		safekey = "RightBracket";
	else if(safekey == "<")
		safekey = "Less";
	else if(safekey == ">")
		safekey = "Greater";
	else if(safekey == "=")
		safekey = "Equal";
	else if(safekey == "'")
		safekey = "SingleQuote";
	else if(safekey == "\"")
		safekey = "DoubleQuote";
	else if(safekey == "\\")
		safekey = "BackSlash";
	else if(safekey == "/")
		safekey = "Slash";
	else if(safekey == ":")
		safekey = "Colon";
	else if(safekey == ";")
		safekey = "Semicolon";
	else if(safekey == "!")
		safekey = "Exclamation";
	return safekey;
}

void AuEmpireReport::ReportPlayers(QSqlDatabase& dbUniverse)
{
	bool traceReport = false;

	if(traceReport)
		qDebug() << "ReportPlayers Started" << QDateTime::currentDateTime();
	// Load specie names table.
	DzSpeciesHash species;
	species.Load();

	// Select avatars data from the database.
	AuIdList personIds;
	QString sq;
	sq = QString("SELECT id FROM a_Person WHERE idp >= 0;");	// include idp=0 for agent limbo detection, idp<0 is deleted char
	QSqlQuery qry(QString(), dbUniverse);
	qry.prepare(sq);
	qry.exec();
	while(qry.next())
	{
		// Read the fields from the query.
		AuId personId = AuId(qry.value(0).toUInt());
		personIds.append(personId);
	}
	if(traceReport)
		qDebug() << "personIds" << personIds.count() << QDateTime::currentDateTime();

	// Now load the entire person records for those people.
	// Running in webCGI mode, DaPerson will load selfies for people who have them.
	QMultiMap<QString, DaPerson*> players;	// Player data sorted alphabetically by upper-case name, duplicates allowed.
	AuEther ether(__FILE__, __LINE__);
	for(AuIdList::const_iterator pii = personIds.cbegin(); pii != personIds.cend(); pii++)
	{
		DaPerson* person = DaPerson::LoadSubatoms(ether, dbUniverse, AuId(0), *pii);
		if(person)
		{
			// Add the id to our map for sorting.
			person->m_AgentBits = 0;	// Don't report person as "Entity" or some animal. :P

			// Don't report players who are members of pirate empires.
			bool isPirate = DgEmpire::IsPirate(person->EmpireId());
			if(isPirate)
			{
				ether.TrashAtom(person->Id());
				continue;
			}

			//if(traceReport)
				//qDebug() << " " << person->Id() << person->Name();
			QString sortname = person->Name().toUpper();
			players.insert(sortname, person);
		}
	}

	// Add a dummy person for Silverwill.
	AuDNA dnaHuman;
	dnaHuman.InitDefaultHuman();
	DaPerson* silverwill = new DaPerson(dnaHuman, AuDNABase::Alpha, RightHanded, "Silverwill", Mature, 0);
	silverwill->SetId(AuId::Silverwill());
	players.insert("SILVERWILL", silverwill);

	if(traceReport)
		qDebug() << "players" << players.count() << QDateTime::currentDateTime();

	QMap<QString, quint32> empireNames;	// Sorted list of empire names.

	QFile file("Avatars.php");
	if(file.open(QIODevice::WriteOnly))
	{
		QTextStream so(&file);
		so.setCodec("UTF-8");
		so << s_htmlHeader;
		so << s_titleBar.arg("Avatars");
		so << s_bodyStart;
		so << QString("<div style=\"text-align: center; font-family: sans-serif;\"><big style=\"color: #ffffff; font-weight: bold; "
				"font-family: sans-serif\"><br>%1</big><br>\n"
				"<small>By Name | </small><a class=\"a1\" href=\"AvatarsByEmpire.php\">By Empire</a></div><br>\n")
			.arg("Shores of Hazeron Avatars");

		// Make a link for each avatar in the map.
		so << "<div style=\"margin-left: 10px;\">\n";
		so << "<table cellpadding=\"2\" cellspacing=\"6\" border=\"0\" width=\"80%\"><tbody>\n";
		for(QMultiMap<QString, DaPerson*>::const_iterator pi = players.cbegin(); pi != players.cend(); pi++)
		{
			so << "<tr>";

			if((*pi)->EmpireId())
			{
				const DgData& gd = g_Au.LockDataForRead();
				QString empireName;
				gd.EmpireName((*pi)->EmpireId(), empireName);
				QImage flag;
				gd.EmpireFlag((*pi)->EmpireId(), flag);
				g_Au.UnlockData();
				if(!empireNames.contains(empireName))
					empireNames.insert(empireName, (*pi)->EmpireId());

				int flagHt = flag.height();
				int flagWid = flag.width();
				if(flagHt >= flagWid)
				{
					flagWid = 32 * flagWid / flagHt;
					if(flagWid < 1)
						flagWid = 1;
					flagHt = 32;
				}
				else
				{
					flagHt = 32 * flagHt / flagWid;
					if(flagHt < 1)
						flagHt = 1;
					flagWid = 32;
				}

				so << "<td align=\"right\">";
				so << QString("<img title=\"%2\" style=\"width: %3px; height: %4px;\" alt=\"\" src=\"%1.png\"><br>")
					.arg((*pi)->EmpireId())
					.arg(empireName)
					.arg(flagWid)
					.arg(flagHt);
				so << "</td>";
			}
			else
			{
				so << "<td>";
				so << "</td>";
			}

			// Some wise guy named their avatar Athena%1. :/
			// All % characters are replaced with _.
			QString personName((*pi)->Name());
			personName.replace("%","_");

			so << "<td>";
			so << QString("<a class=\"ll\" href=\"p%1.php\">"
					"<span style=\"font-family: sans-serif;\">%2</span></a><br>")
				.arg((*pi)->Id().StringId())
				.arg(personName);

			so << "</td>";
			if((*pi)->IsLineageHighborn())
			{
				so << "<td align=\"right\">";
				so << "<small><font color=\"#ffcc00\">";
				so << (*pi)->Id().StringId();
				so << "</font></small>";
				so << "</td>";
			}
			else if((*pi)->IsLineageDemiavatar())
			{
				so << "<td align=\"right\">";
				so << "<small><font color=\"#0038b8\">";
				so << (*pi)->Id().StringId();
				so << "</font></small>";
				so << "</td>";
			}
			else
			{
				so << "<td align=\"right\">";
				so << "<small>";
				so << (*pi)->Id().StringId();
				so << "</small>";
				so << "</td>";
			}
			so << "</tr>\n";
		}
		so << "</tbody></table>\n";
		so << "</div>\n";

		so << "</body></html>\n";
		file.close();
	}

	file.setFileName("AvatarsByEmpire.php");
	if(file.open(QIODevice::WriteOnly))
	{
		QTextStream so(&file);
		so.setCodec("UTF-8");
		so << s_htmlHeader;
		so << s_titleBar.arg("Avatars");
		so << s_bodyStart;
		so << QString("<div style=\"text-align: center; font-family: sans-serif;\"><big style=\"color: #ffffff; font-weight: bold; "
				"font-family: sans-serif\"><br>%1</big><br>\n"
				"<a class=\"a1\" href=\"Avatars.php\">By Name</a><small> | By Empire</small></div><br>\n")
			.arg("Shores of Hazeron Avatars");

		// Make a link for each avatar in the map.
		so << "<div style=\"margin-left: 10px;\">\n";
		so << "<table cellpadding=\"2\" cellspacing=\"6\" border=\"0\" width=\"80%\"><tbody>\n";
		for(QMap<QString, quint32>::const_iterator ei = empireNames.cbegin(); ei != empireNames.cend(); ei++)
		{
			// Flag dimensions.
			const DgData& gd = g_Au.LockDataForRead();
			QImage flag;
			gd.EmpireFlag(*ei, flag);
			g_Au.UnlockData();

			int flagHt = flag.height();
			int flagWid = flag.width();
			if(flagHt >= flagWid)
			{
				flagWid = 128 * flagWid / flagHt;
				if(flagWid < 1)
					flagWid = 1;
				flagHt = 128;
			}
			else
			{
				flagHt = 128 * flagHt / flagWid;
				if(flagHt < 1)
					flagHt = 1;
				flagWid = 128;
			}

			// Empire name row.
			so << "<tr>";
			so << "<td align=\"right\">";
			so << QString("<img title=\"%2\" style=\"width: %3px; height: %4px;\" alt=\"\" src=\"%1.png\"><br>")
				.arg(*ei)
				.arg(ei.key())
				.arg(flagWid)
				.arg(flagHt);
			so << "</td>";
			so << "<td valign=\"middle\">";
			so << QString("<span style=\"font-family: sans-serif;\"><big><b>%1</b></big></span>")
				.arg(ei.key());
			so << "</td>";
			so << "<td>";
			so << "</td>";
			so << "</tr>\n";

			for(QMultiMap<QString, DaPerson*>::const_iterator pi = players.cbegin(); pi != players.cend(); pi++)
			{
				if((*pi)->EmpireId() != *ei)
					continue;

				// Some wise guy named their avatar Athena%1. :/
				// All % characters are replaced with _.
				QString personName((*pi)->Name());
				personName.replace("%","_");

				so << "<tr>";
				so << "<td>";
				so << "</td>";
				so << "<td>";
				so << QString("<a class=\"ll\" href=\"p%1.php\">"
						"<span style=\"font-family: sans-serif;\">%2</span></a><br>")
					.arg((*pi)->Id().StringId())
					.arg(personName);

				so << "</td>";
				if((*pi)->IsLineageHighborn())
				{
					so << "<td align=\"right\">";
					so << "<small><font color=\"#ffcc00\">";
					so << (*pi)->Id().StringId();
					so << "</font></small>";
					so << "</td>";
				}
				else if((*pi)->IsLineageDemiavatar())
				{
					so << "<td align=\"right\">";
					so << "<small><font color=\"#0038b8\">";
					so << (*pi)->Id().StringId();
					so << "</font></small>";
					so << "</td>";
				}
				else
				{
					so << "<td align=\"right\">";
					so << "<small>";
					so << (*pi)->Id().StringId();
					so << "</small>";
					so << "</td>";
				}
				so << "</tr>\n";
			}
		}
		so << "</tbody></table>\n";
		so << "</div>\n";

		so << "</body></html>\n";
		file.close();
	}

	// Now make the individual avatar pages.
	if(traceReport)
		qDebug() << "Individual pages for players" << players.count() << QDateTime::currentDateTime();
	for(QMultiMap<QString, DaPerson*>::const_iterator pi = players.cbegin(); pi != players.cend(); pi++)
	{
		DaPerson* person = *pi;
		
		// Save their selfie, if any.
		QString selfieFile;
		if(!person->SelfieNoLock().isNull())
		{
			selfieFile = QString("p%1.png").arg(person->Id().StringId());
			person->SelfieNoLock().save(selfieFile);
		}

		// Save their html page.
		file.setFileName(QString("p%1.php").arg(person->Id().StringId()));
		if(file.open(QIODevice::WriteOnly))
		{
			int imgWidth, imgHeight;
			const DgData& gd = g_Au.LockDataForRead();
			DgEmpireHash::const_iterator ei = gd.m_gEmpires.find(person->EmpireId());
			QString nameWithTitle, empireName;
			if(ei != gd.m_gEmpires.cend())
			{
				nameWithTitle = (*ei).second.CitizenNameWithTitle(person->Id());
				empireName = (*ei).second.Name();

				imgWidth = (*ei).second.m_flag.width();
				imgHeight = (*ei).second.m_flag.height();

				// Double the size of the small flag image, which has a maximum dimension of 64x64.
				if(imgWidth > imgHeight)
				{
					if(imgWidth < 128)
					{
						imgWidth *= 2;
						imgHeight *= 2;
					}
				}
				else
				{
					if(imgHeight < 128)
					{
						imgWidth *= 2;
						imgHeight *= 2;
					}
				}
			}
			else if(person->EmpireId())
			{
				nameWithTitle = person->Name();
				empireName = QString("Forgotten Empire %1").arg(person->EmpireId());
				imgWidth = 0;
				imgHeight = 0;
			}
			else
			{
				nameWithTitle = person->Name();
				imgWidth = 0;
				imgHeight = 0;
			}
			g_Au.UnlockData();

			QTextStream so(&file);
			so.setCodec("UTF-8");
			so << s_htmlHeader;
			so << s_titleBar.arg((*pi)->Name());
			so << s_bodyStart;
			/*
			so << QString("<div style=\"text-align: center; font-family: sans-serif;\">"
					"<big style=\"font-weight: bold; font-family: sans-serif\"><br>%1</big></div>").arg((*pi)->Name());

			if(person->EmpireId())
			{
				if(imgWidth > 0 && imgHeight > 0)
					so << QString("<br><div style=\"text-align: center;\">"
							"<img title=\"%4\" style=\"width: %2px; height: %3px;\" alt=\"%4 Flag\" src=\"%1.png\"></div>")
						.arg(person->EmpireId()).arg(imgWidth).arg(imgHeight).arg(empireName);
				else
					so << QString("<br><div title=\"%4\" style=\"text-align: center;\"><img alt=\"%4 Flag\" src=\"%1.png\"></div>")
						.arg(person->EmpireId()).arg(empireName);
			}
			*/

			so << "<span style=\"font-family: sans-serif;\">";

			// Fill the page with information about the person similar to the Bio window in the game.

			// Bio
			{
				// Identification
				so << "<hr style=\"width: 100%; height: 1px;\">";
				so << "<div style=\"text-align: right;\"><span style=\"font-family: sans-serif; font-weight: bold; color: rgb(128,128,128);\">"
					"Avatar</span><br></div>";

				// Link to selfie if we saved one.
				if(!selfieFile.isEmpty())
					so << QString("<img alt=\"\" src=\"p%1.png\" align=\"right\" width: 512px;>").arg(person->Id().StringId());

				// Some wise guy named their avatar Athena%1. :/
				// All % characters are replaced with _.
				nameWithTitle.replace("%","_");
				so << QString("<span style=\"color: rgb(255, 255, 255);\"><big><b>%1</b></big> <i>%2</i></span><br>")
					.arg(nameWithTitle)
					.arg(person->Id().StringId());

				if(!person->SpawnShipName().isEmpty())
					so << QString("Berthed Aboard <b>%1</b><br>").arg(person->SpawnShipName());

				/*
				if(!person->SpawnCityName().isEmpty())
				{
					//if(!person->SpawnCitySystemName().isEmpty())
						//so << QString("Home <b>%1, %2 system</b><br>").arg(person->SpawnCityName()).arg(person->SpawnCitySystemName());
					//else
						so << QString("Home <b>%1</b><br>").arg(person->SpawnCityName());
				}

				so << QString("On Line <b>%1</b><br>").arg(person->IsOnLine()?"Yes":"No");

				int minutesOn = person->Stats().m_OnLineMinutes;
				int daysOn = minutesOn / (60*24);
				minutesOn -= daysOn * (60*24);
				int hoursOn = minutesOn / 60;
				minutesOn -= hoursOn * 60;
				if(daysOn)
					so << QString("On Line Time <b>%1 days, %2 hours, %3 minutes</b><br>")
						.arg(daysOn)
						.arg(hoursOn)
						.arg(minutesOn);
				else if(hoursOn)
					so << QString("On Line Time <b>%1 hours, %2 minutes</b><br>")
						.arg(hoursOn)
						.arg(minutesOn);
				else
					so << QString("On Line Time <b>%1 minutes</b><br>")
						.arg(minutesOn);

				so << QString("Stories Started <b>%1</b><br>").arg(person->Stats().m_StoriesStarted);

				so << QString("Stories Completed <b>%1</b><br>").arg(person->Stats().m_StoriesCompleted);

				// Skills
				const AuSkillSet& skills = person->LockSkillsForRead();
				if(skills.Count())
				{
					// Sort alphabetically.
					QMap<QString, quint32> sorted;
					QMap<QString, QString> sortedImgs;
					for(QHash<AuSkill::SKILL, AuSkill>::const_iterator si = skills.Skills().cbegin();
							si != skills.Skills().cend(); si++)
					{
						sorted.insert((*si).Name(), (*si).Experience());
						QString imgname((*si).IconName());
						int lastSlash = imgname.lastIndexOf('/');
						if(lastSlash >= 0)
							imgname = imgname.right(imgname.length() - lastSlash - 1);
						sortedImgs.insert((*si).Name(), imgname);
					}

					so << "<br><table><tbody>";
					if(sorted.size() <= 10)
					{
						// One column
						so << "<tr>";
						so << "<td style=\"background-color: rgb(32, 32, 32);\"><b><small>Skill</small></b></td>";
						so << "<td style=\"background-color: rgb(32, 32, 32);\"><b><small>Level</small></b></td>";
						so << "</tr>";
						for(QMap<QString, quint32>::const_iterator si = sorted.cbegin(); si != sorted.cend(); si++)
						{
							so << "<tr>";
							so << QString("<td style=\"background-color: rgb(0, 0, 32);\"><img alt=\"\" src=\"../images/%2\" height=\"16\"><small> %1</td>")
								.arg(si.key()).arg(sortedImgs[si.key()]);
							so << QString("<td style=\"background-color: rgb(0, 0, 32); text-align: right;\"><small>%L1</td>").arg(si.value());
							so << "</tr>";
						}
					}
					else
					{
						// Two columns
						so << "<tr>";
						so << "<td style=\"background-color: rgb(32, 32, 32);\"><b><small>Skill</small></b></td>";
						so << "<td style=\"background-color: rgb(32, 32, 32);\"><b><small>Level</small></b></td>";
						so << "<td style=\"background-color: rgb(32, 32, 32);\"><b><small> </small></b></td>";	// Gutter
						so << "<td style=\"background-color: rgb(32, 32, 32);\"><b><small>Skill</small></b></td>";
						so << "<td style=\"background-color: rgb(32, 32, 32);\"><b><small>Level</small></b></td>";
						so << "</tr>";
						for(QMap<QString, quint32>::const_iterator si = sorted.cbegin(); si != sorted.cend(); si++)
						{
							so << "<tr>";
							so << QString("<td style=\"background-color: rgb(0, 0, 32);\"><img alt=\"\" src=\"../images/%2\" height=\"16\"><small> %1</small></td>")
								.arg(si.key()).arg(sortedImgs[si.key()]);
							so << QString("<td style=\"background-color: rgb(0, 0, 32); text-align: right;\"><small>%L1</small></td>").arg(si.value());

							so << QString("<td> </td>");	// Gutter

							si++;
							if(si == sorted.cend())
							{
								// Must have an odd number and we got to the end.
								so << QString("<td> </td>");
								so << QString("<td> </td>");
								so << "</tr>";
								break;
							}
							else
							{
								so << QString("<td style=\"background-color: rgb(0, 0, 32);\"><img alt=\"\" src=\"../images/%2\" height=\"16\"><small> %1</small></td>")
									.arg(si.key()).arg(sortedImgs[si.key()]);
								so << QString("<td style=\"background-color: rgb(0, 0, 32); text-align: right;\"><small>%L1</small></td>").arg(si.value());
								so << "</tr>";
							}
						}
					}
					so << "</table></tbody>";
				}
				person->UnlockSkills();
				*/

				// Bounties
				QMultiMap<AuTime, AuBountyOnItem> bounties;
				person->Bounties(bounties);
				if(!bounties.empty() || person->Stats().m_BountyCollections || person->Stats().m_BountyGrants)
				{
					so << QString("<br><b>Bounties</b><br>");
					if(person->Stats().m_BountyCollections)
					{
						so << QString("Heads Redeemed <b>%L1</b><br>")
							.arg(person->Stats().m_BountyCollections);
						so << QString("Bounty Collected <b>%L1%2</b><br>")
							.arg(person->Stats().m_BountyCollected, 0,'f',0)
							.arg(AuGlobal::CurrencySymbol());
					}
					if(person->Stats().m_BountyGrants)
					{
						// Some wise guy named their avatar Athena%1. :/
						// All % characters are replaced with _.
						QString personName(person->Name());
						personName.replace("%","_");

						so << QString("%2 Heads Redeemed by Others <b>%L1</b><br>")
							.arg(person->Stats().m_BountyGrants)
							.arg(personName);
						if(person->Stats().m_BountyGrants == 1)
							so << QString("Bounty Collected from %3 Head <b>%L1%2</b><br>")
								.arg(person->Stats().m_BountyGranted, 0,'f',0)
								.arg(AuGlobal::CurrencySymbol())
								.arg(personName);
						else
							so << QString("Bounty Collected from %3 Heads <b>%L1%2</b><br>")
								.arg(person->Stats().m_BountyGranted, 0,'f',0)
								.arg(AuGlobal::CurrencySymbol())
								.arg(personName);
					}
					if(!bounties.empty())
					{
						so << "<table><tbody>";
						so << "<tr>";
						so << "<td style=\"background-color: rgb(32, 32, 32);\"><b><small>Amount</small></b></td>";
						so << "<td style=\"background-color: rgb(32, 32, 32);\"><b><small>Reason</small></b></td>";
						so << "<td style=\"background-color: rgb(32, 32, 32);\"><b><small>By Who</small></b></td>";
						so << "<td style=\"background-color: rgb(32, 32, 32);\"><b><small>When</small></b></td>";
						so << "</tr>";
						for(QMultiMap<AuTime, AuBountyOnItem>::const_iterator bri = bounties.constBegin(); bri != bounties.constEnd(); bri++)
						{
							// Some wise guy named their avatar Athena%1. :/
							// All % characters are replaced with _.
							QString posterName((*bri).PosterName());
							posterName.replace("%","_");

							QString amount = QString("%L1%2").arg((*bri).Amount(), 0,'f',0).arg(AuGlobal::CurrencySymbol());
							QString reason = (*bri).Reason();
							QString byWho = QString("%1 <i><small>%2</small></i>")
								.arg(posterName)
								.arg((*bri).PosterId().StringId());
							QString when = bri.key().UTC().toString();

							so << "<tr>";
							so << QString("<td style=\"background-color: rgb(0, 0, 32); text-align: right;\"><small>%1</small></td>")
								.arg(amount);
							so << QString("<td style=\"background-color: rgb(0, 0, 32);\"><small>%1</small></td>").arg(reason);
							so << QString("<td style=\"background-color: rgb(0, 0, 32);\"><a class=\"ll\" href=\"p%2.php\"><small>%1</small></a></td>")
								.arg(byWho)
								.arg((*bri).PosterId().StringId());
							so << QString("<td style=\"background-color: rgb(0, 0, 32);\"><small>%1</small></td>").arg(when);
							so << "</tr>";
						}

						if(bounties.size() > 1)
						{
							so << "<tr>";
							so << QString("<td style=\"background-color: rgb(0, 0, 32);\"><hr></td>");
							so << QString("<td style=\"background-color: rgb(0, 0, 32);\"></td>");
							so << QString("<td style=\"background-color: rgb(0, 0, 32);\"></td>");
							so << "</tr>";

							so << "<tr>";
							so << QString("<td style=\"background-color: rgb(0, 0, 32); text-align: right;\"><small>%L1%2</small></td>")
								.arg(person->Bounty(), 0,'f',0)
								.arg(AuGlobal::CurrencySymbol());
							so << QString("<td style=\"background-color: rgb(0, 0, 32);\"></td>");
							so << QString("<td style=\"background-color: rgb(0, 0, 32);\"></td>");
							so << "</tr>";
						}

						so << "</table></tbody>";
					}
				}
			}

			// Biology
			{
				// Identification
				so << "<br><hr style=\"width: 100%; height: 1px;\">";
				so << "<div style=\"text-align: right;\"><span style=\"font-family: sans-serif; font-weight: bold; color: rgb(128,128,128);\">"
					"Biology</span><br></div>";

				AuSpecies personSpecies(person->Species());
				QString speciesName;
				DzSpeciesHash::const_iterator si = species.find(personSpecies);
				if(si != species.cend())
					speciesName = (*si).second.Name();
				QString dnadesc = personSpecies.Description();

				so << QString("<big><b><span style=\"color: rgb(255, 255, 255);\">%1</span></b></big><br>")
					.arg(dnadesc);

				if(!speciesName.isEmpty())
					so << QString("Species <b>%1</b><br>").arg(speciesName);

				QString lineage;
				if(person->IsLineageHighborn())
					lineage = QString("<font color=\"#ffcc00\">Highborn</font>");
				else if(person->IsLineageDemiavatar())
					lineage = QString("<font color=\"#0038b8\">Demiavatar</font>");
				else
					lineage = QString("Avatar");

				so << QString("Lineage <b>%1</b><br>").arg(lineage);

				if(person->IsLineageHighborn() || person->IsLineageDemiavatar())
					so << QString("Born <b>%1</b><br>").arg(person->CreatedTime().UTC().toString());
				else
					so << QString("Decanted <b>%1</b><br>").arg(person->CreatedTime().UTC().toString());

				QString gender = person->FemaleMale(true);

				so << QString("Gender <b>%1</b><br>").arg(gender);

				QString handed;
				if(person->IsLeftHanded())
					handed = QString("Left-handed");
				else
					handed = QString("Right-handed");

				so << QString("Handedness <b>%1</b><br>").arg(handed);

				/*
				so << QString("Health <b>%1 of %2</b><br>").arg(person->CurrentHitPoints()).arg(person->MaxHitPoints());

				so << QString("Stamina <b>%1 of %2</b><br>").arg(person->CurrentStamina()).arg(person->MaxStamina());

				so << QString("Hunger <b>%1 of %2</b><br>").arg(person->CurrentHunger()).arg(person->MaxHunger());

				if(person->Stats().m_ChildrenFathered)
				{
					if(person->Gender() == Male)
						so << QString("Females Impregnated <b>%1</b><br>").arg(person->Stats().m_ChildrenFathered);
					else
						so << QString("Mates Impregnated <b>%1</b><br>").arg(person->Stats().m_ChildrenFathered);
				}

				if(person->Stats().m_ChildrenCarried)
					so << QString("Children Conceived <b>%1</b><br>").arg(person->Stats().m_ChildrenCarried);

				if(person->Stats().m_ChildrenBorn)
					so << QString("Children Born <b>%1</b><br>").arg(person->Stats().m_ChildrenBorn);
					*/
			}

			// Empire
			if(person->EmpireId())
			{
				// Identification
				so << "<br><hr style=\"width: 100%; height: 1px;\">";
				so << "<div style=\"text-align: right;\"><span style=\"font-family: sans-serif; font-weight: bold; color: rgb(128,128,128);\">"
					"Empire</span><br></div>";

				bool hasOffice = false;
				AuOffice office;
				const DgData& gd = g_Au.LockDataForRead();
				DgEmpireHash::const_iterator ei = gd.m_gEmpires.find(person->EmpireId());
				if(ei != gd.m_gEmpires.cend())
				{
					hasOffice = (*ei).second.IsGovernor(person->Id());
					if(hasOffice)
						office = (*ei).second.CitizenOffice(person->Id());

					QString semp = QString("<big><b><span style=\"color: rgb(255, 255, 255);\">%1</span></b></big>").arg((*ei).second.HtmlName());
					so << semp + "<br>";

					if(imgWidth > 0 && imgHeight > 0)
						so << QString("<img alt=\"%2 Flag\" src=\"%1.png\" align=\"right\" style=\"width: %3px; height: %4px;\">")
							.arg(person->EmpireId()).arg(empireName).arg(imgWidth).arg(imgHeight);
					else
						so << QString("<img alt=\"%2 Flag\" src=\"%1.png\" align=\"right\">")
							.arg(person->EmpireId()).arg(empireName);

					AuId emperorId = (*ei).second.EmperorId();
					QString sempr;
					if(emperorId.IsNull())
						sempr = QString("Leader <b>-</b>");
					else
						sempr = QString("Leader <b><a class=\"ll\" href=\"p%3.php\">%1 %2</a></b>")
							.arg((*ei).second.CitizenTitle(emperorId))
							.arg((*ei).second.CitizenName(emperorId))
							.arg(emperorId.StringId());
					so << sempr + "<br>";
				}
				g_Au.UnlockData();

				so << QString("Reputation <b>%L1</b><br>").arg(person->Reputation());

				//so << QString("Credit Limit <b>%L1%2</b><br>").arg(person->CreditLimit(), 0,'f',0).arg(g_Au.CurrencySymbol());
			}

			so << "</span>";
			so << "</body></html>";
			file.close();
		}
	}
	if(traceReport)
		qDebug() << "ReportPlayers Finished" << QDateTime::currentDateTime();
}

void AuEmpireReport::ReportDesignExchange(QSqlDatabase& dbUniverse)
{
	// Spacecraft Blueprints
	AuScDesignExchange scExch;
	AuExchangeBlueprintRatings scRatings;
	QMultiMap<QString, quint32> scSorted;	// Alphabetized blueprints published for everyone.
	{
		scExch.SqlSelectAll(dbUniverse);
		for(AuScDesignExchange::const_iterator xi = scExch.cbegin(); xi != scExch.cend(); xi++)
			if((*xi).MinStanceToBuyer() == POL_Enemy)
			{
				scSorted.insert((*xi).DesignName().toUpper(), (*xi).DesignId());
				scRatings.SqlSelectSc(dbUniverse, (*xi).DesignId());
			}
	}

	// Building Blueprints
	AuBdDesignExchange bdExch;
	AuExchangeBlueprintRatings bdRatings;
	QMultiMap<QString, quint32> bdSorted;	// Alphabetized blueprints published for everyone.
	{
		bdExch.SqlSelectAll(dbUniverse);
		for(AuBdDesignExchange::const_iterator xi = bdExch.cbegin(); xi != bdExch.cend(); xi++)
			if((*xi).MinStanceToBuyer() == POL_Enemy)
			{
				bdSorted.insert((*xi).DesignName().toUpper(), (*xi).DesignId());
				bdRatings.SqlSelectBd(dbUniverse, (*xi).DesignId());
			}
	}

	QMap<QString, quint32> empireNames;	// Sorted list of empire names.

	QString intro = QString("<span style=\"font-family: sans-serif;\"><small>"
			"<p>"
			"Blueprints for spacecraft and buildings are created using the <i>Shores of Hazeron Designer</i>. "
			"The designer is a free form 3D surface modeler, accessible in-game and out-of-game. "
			"All buildings and spacecraft in Hazeron are designed by players. "
			"</p>"
			"<p>"
			"Architects publish their blueprints at the in-game <i>Blueprint Exchange</i> so other players can get them. "
			"Publishing terms, set by the architect, dictate who can access their published blueprints. "
			"The exchange contains <b>%L1 blueprints</b>. "
			"This page only shows blueprints with terms accessible to everyone, regardless of empire, listing <b>%L4 of %L2 "
			"spacecraft blueprints</b> and <b>%L5 of %L3 building blueprints</b>. "
			"</p>"
			"<p>"
			"<i>Final</i> blueprints are ready for use in the game. "
			"They are optimized from the original drawing, through a process called <i>finalizing</i>. "
			"Final spacecraft show <b>MinQ</b>, the minimum quality of materials needed to manufacture a spacecraft using the blueprint. "
			"</p>"
			"<p>"
			"An <i>Assembly</i> blueprint may be at any stage of completion. "
			"It could be ready for finalizing, as with the <i>Atlantis</i> assembly. "
			"It could be a set of useful geometry, such as furniture and consoles. "
			"An assembly is simply a saved copy of a blueprint, with nothing left out. "
			"</p>"
			"<p>"
			"Prices are in <i>cronodollars %6</i>, the money of the game. "
			"Cronos are not related to real money. "
			"<b>Spacecraft and building blueprints do not cost real money.</b> "
			"</p>"
			"</small></span></a>")
			.arg(scExch.size() + bdExch.size())
			.arg(scExch.size())
			.arg(bdExch.size())
			.arg(scSorted.size())
			.arg(bdSorted.size())
			.arg(AuGlobal::CurrencySymbol());

	QString introBd = QString("<span style=\"font-family: sans-serif;\"><small>"
			"<p>"
			"Building blueprints show a <i>Use</i> column, which is the industry intended to use the blueprint. "
			"An intended industry for a blueprint is optional. "
			"In practice, a blueprint may be used to construct any industry, as long as the blueprint meets the building code requirements for the industry. "
			"Building codes can be accessed from the construction window and in the building designer. "
			"</p>"
			"</small></span></a>");

	QFile file("Exchange.php");
	if(file.open(QIODevice::WriteOnly))
	{
		QTextStream so(&file);
		so.setCodec("UTF-8");
		so << s_htmlHeader;
		so << s_titleBar.arg("Blueprint Exchange");
		so << s_bodyStart;
		so << QString("<div style=\"text-align: center; font-family: sans-serif;\"><big style=\"color: #ffffff; font-weight: bold; "
				"font-family: sans-serif\"><br>%1</big><br>\n"
				"<small>By Name | </small><a class=\"a1\" href=\"ExchangeByEmpire.php\">By Empire</a></div>")
			.arg("Shores of Hazeron Blueprint Exchange");

		// Make a link for each design in the map.
		so << "<div style=\"margin-left: 10px;\">\n";
		so << intro;

		// Spacecraft
		{
			so << "<table style=\"text-align: left; width: 100%; font-family: sans-serif;\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\"><tbody>";
			so << "<tr>";
			so << QString("<td colspan=\"9\"; style=\"vertical-align: top; background-color: %1;\"><small><b>Spacecraft Blueprints</b></small><br></td>").arg(hazBlue2);
			so << "</tr>";
			so << "<tr>";
			so << QString("<td colspan=\"2\"; style=\"vertical-align: top; background-color: %1;\"><small><b>Blueprint</b></small><br></td>").arg(hazBlue3);
			so << QString("<td style=\"vertical-align: top; background-color: %1;\"><small><b>Rating</b></small><br></td>").arg(hazBlue3);
			so << QString("<td style=\"vertical-align: top; background-color: %1;\"><small><b>MinQ</b></small><br></td>").arg(hazBlue3);
			so << QString("<td style=\"vertical-align: top; background-color: %1;\"><small><b>Size</b></small><br></td>").arg(hazBlue3);
			so << QString("<td style=\"vertical-align: top; background-color: %1;\"><small><b>State</b></small><br></td>").arg(hazBlue3);
			so << QString("<td style=\"vertical-align: top; background-color: %1;\"><small><b>Architect</b></small><br></td>").arg(hazBlue3);
			so << QString("<td style=\"vertical-align: top; background-color: %1;\"><small><b>Empire</b></small><br></td>").arg(hazBlue3);
			so << QString("<td style=\"vertical-align: top; background-color: %1;\"><small><b>Publisher Terms</b></small><br></td>").arg(hazBlue3);
			so << "</td>";
			for(QMultiMap<QString, quint32>::const_iterator si = scSorted.cbegin(); si != scSorted.cend(); si++)
			{
				AuScDesignExchange::const_iterator xi = scExch.find(*si);
				if(xi == scExch.cend())
					continue;	// wut?
				const DgData& gd = g_Au.LockDataForRead();
				QString empireName;
				gd.EmpireName((*xi).PublisherEmpire(), empireName);
				g_Au.UnlockData();
				if(!empireNames.contains(empireName))
					empireNames.insert(empireName, (*xi).PublisherEmpire());

				so << "<tr>";
				so << "<td align=\"right\">";
				so << QString("<img title=\"%2\" style=\"height: 64px;\" alt=\"\" src=\"sc%1.png\"><br>")
					.arg((*xi).DesignId())
					.arg((*xi).DesignName());
				so << "</td>";

				so << "<td>";
				so << QString("<a class=\"ll\" href=\"sc%1.php\">"
						"<span style=\"font-family: sans-serif;\"><big><b>%2</b></big><br><small><small><i>%1</i></small></small></span></a><br>")
					.arg((*xi).DesignId())
					.arg((*xi).DesignName());
				so << "</td>";

				// Rating
				so << "<td>";
				AuExchangeBlueprintRating rating;
				if(scRatings.Rating((*xi).DesignId(), rating))
				{
					QString rih = QString("<img src=\"qrc%1\" width=\"16\" height=\"16\"> %2")
						.arg(rating.RatingAverageIconName())
						.arg(rating.RatingAverageString());
					rih.replace("qrc:data","/images");
					so << rih;
				}
				so << "</td>";

				// Quality
				so << "<td>";
				if((*xi).DesignState() == AuDesign::Final)
					so << QString("<span style=\"font-family: sans-serif;\"><small>Q%1</small></span></a><br>")
						.arg((*xi).DesignQuality());
				so << "</td>";

				// Size
				if((*xi).FaceCount() || (*xi).TriangleCount())
				{
					if((*xi).DesignState() == AuDesign::Final)
					{
						so << "<td>";
						so << QString("<span style=\"font-family: sans-serif;\"><small>%L1m%2<br>%L3 faces<br>%L4 tris</small></span></a><br>")
							.arg((*xi).HullVolume())
							.arg(AuGlobal::CubedSymbol())
							.arg((*xi).FaceCount())
							.arg((*xi).TriangleCount());
						so << "</td>";
					}
					else
					{
						so << "<td>";
						so << QString("<span style=\"font-family: sans-serif;\"><small>%L1 faces<br>%L2 tris</small></span></a><br>")
							.arg((*xi).FaceCount())
							.arg((*xi).TriangleCount());
						so << "</td>";
					}
				}
				else
				{
					if((*xi).DesignState() == AuDesign::Final)
					{
						so << "<td>";
						so << QString("<span style=\"font-family: sans-serif;\"><small>%L1m%2</small></span></a><br>")
							.arg((*xi).HullVolume())
							.arg(AuGlobal::CubedSymbol());
						so << "</td>";
					}
					else
					{
						so << "<td>";
						so << "</td>";
					}
				}

				// State
				if((*xi).DesignState() == AuDesign::Final)
				{
					so << "<td>";
					so << QString("<span style=\"font-family: sans-serif;\"><small>Final</small></span></a><br>");
					so << "</td>";
				}
				else
				{
					so << "<td>";
					so << QString("<span style=\"font-family: sans-serif;\"><small>Assembly</small></span></a><br>");
					so << "</td>";
				}

				// Architect
				so << "<td>";
				so << QString("<a class=\"ll\" href=\"p%1.php\">"
						"<span style=\"font-family: sans-serif;\"><small>%2</small></span></a><br>")
					.arg((*xi).DesignArchitectId().StringId())
					.arg((*xi).DesignArchitectName());
				so << "</td>";

				so << "<td>";
				so << QString("<span style=\"font-family: sans-serif;\"><small>%1</small></span></a><br>")
					.arg(empireName);
				so << "</td>";

				// Terms
				so << "<td>";
				{
					// Price
					QString terms;
					if((*xi).Price())
						terms = QString("Blueprint costs %L1%2.").arg((*xi).Price()).arg(AuGlobal::CurrencySymbol());
					else
						terms = QString("Blueprint is free.");

					// License
					if((*xi).CanCopy())
						terms += QString("<br>Can be copied, viewed and manufactured.");
					else if((*xi).CanView())
						terms += QString("<br>Can be viewed and manufactured.");
					else
						terms += QString("<br>Can be manufactured.");

					// Stance Restriction
					/*
					switch((*xi).MinStanceToBuyer())
					{
					case POL_Vassal:
					case POL_Enemy:
						terms += QString("<br>Accessible by all empires.");
						break;
					case POL_Neutral:
						terms += QString("<br>Accessible to all but enemy empires.");
						break;
					case POL_Friend:
						terms += QString("<br>Accessible to all but neutral and enemy empires.");
						break;
					case POL_Empire:
						terms += QString("<br>Accessible by publisher empire only.");
						break;
					}
					*/

					so << QString("<span style=\"font-family: sans-serif;\"><small>%1</small></span></a><br>")
						.arg(terms);
				}
				so << "</td>";

				so << "</tr>\n";
			}
			so << "</tbody></table>\n";
		}

		so << introBd;

		// Buildings
		{
			so << "<table style=\"text-align: left; width: 100%; font-family: sans-serif;\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\"><tbody>";
			so << "<tr>";
			so << QString("<td colspan=\"9\"; style=\"vertical-align: top; background-color: %1;\"><small><b>Building Blueprints</b></small><br></td>").arg(hazBlue2);
			so << "</tr>";
			so << "<tr>";
			so << QString("<td colspan=\"2\"; style=\"vertical-align: top; background-color: %1;\"><small><b>Blueprint</b></small><br></td>").arg(hazBlue3);
			so << QString("<td style=\"vertical-align: top; background-color: %1;\"><small><b>Rating</b></small><br></td>").arg(hazBlue3);
			so << QString("<td style=\"vertical-align: top; background-color: %1;\"><small><b>Use</b></small><br></td>").arg(hazBlue3);
			so << QString("<td style=\"vertical-align: top; background-color: %1;\"><small><b>Size</b></small><br></td>").arg(hazBlue3);
			so << QString("<td style=\"vertical-align: top; background-color: %1;\"><small><b>State</b></small><br></td>").arg(hazBlue3);
			so << QString("<td style=\"vertical-align: top; background-color: %1;\"><small><b>Architect</b></small><br></td>").arg(hazBlue3);
			so << QString("<td style=\"vertical-align: top; background-color: %1;\"><small><b>Empire</b></small><br></td>").arg(hazBlue3);
			so << QString("<td style=\"vertical-align: top; background-color: %1;\"><small><b>Publisher Terms</b></small><br></td>").arg(hazBlue3);
			so << "</td>";
			for(QMultiMap<QString, quint32>::const_iterator si = bdSorted.cbegin(); si != bdSorted.cend(); si++)
			{
				AuBdDesignExchange::const_iterator xi = bdExch.find(*si);
				if(xi == bdExch.cend())
					continue;	// wut?
				const DgData& gd = g_Au.LockDataForRead();
				QString empireName;
				gd.EmpireName((*xi).PublisherEmpire(), empireName);
				QString iname;
				if((*xi).IndustryId() != IID_None)
					iname = gd.IndustryName((*xi).IndustryId());
				g_Au.UnlockData();
				if(!empireNames.contains(empireName))
					empireNames.insert(empireName, (*xi).PublisherEmpire());

				so << "<tr>";
				so << "<td align=\"right\">";
				so << QString("<img title=\"%2\" style=\"height: 64px;\" alt=\"\" src=\"bd%1.png\"><br>")
					.arg((*xi).DesignId())
					.arg((*xi).DesignName());
				so << "</td>";

				so << "<td>";
				so << QString("<a class=\"ll\" href=\"bd%1.php\">"
						"<span style=\"font-family: sans-serif;\"><big><b>%2</b></big><br><small><small><i>%1</i></small></small></span></a><br>")
					.arg((*xi).DesignId())
					.arg((*xi).DesignName());
				so << "</td>";

				// Rating
				so << "<td>";
				AuExchangeBlueprintRating rating;
				if(bdRatings.Rating((*xi).DesignId(), rating))
				{
					QString rih = QString("<img src=\"qrc%1\" width=\"16\" height=\"16\"> %2")
						.arg(rating.RatingAverageIconName())
						.arg(rating.RatingAverageString());
					rih.replace("qrc:data","/images");
					so << rih;
				}
				so << "</td>";

				// Intended Industry
				so << "<td>";
				if(!iname.isEmpty())
					so << QString("<span style=\"font-family: sans-serif;\"><small>%1</small></span></a><br>")
						.arg(iname);
				so << "</td>";

				// Size
				if((*xi).FaceCount() || (*xi).TriangleCount())
				{
					if((*xi).DesignState() == AuDesign::Final)
					{
						so << "<td>";
						so << QString("<span style=\"font-family: sans-serif;\"><small>%L1m%2<br>%L3 faces<br>%L4 tris</small></span></a><br>")
							.arg((*xi).HullVolume())
							.arg(AuGlobal::CubedSymbol())
							.arg((*xi).FaceCount())
							.arg((*xi).TriangleCount());
						so << "</td>";
					}
					else
					{
						so << "<td>";
						so << QString("<span style=\"font-family: sans-serif;\"><small>%L1 faces<br>%L2 tris</small></span></a><br>")
							.arg((*xi).FaceCount())
							.arg((*xi).TriangleCount());
						so << "</td>";
					}
				}
				else
				{
					if((*xi).DesignState() == AuDesign::Final)
					{
						so << "<td>";
						so << QString("<span style=\"font-family: sans-serif;\"><small>%L1m%2</small></span></a><br>")
							.arg((*xi).HullVolume())
							.arg(AuGlobal::CubedSymbol());
						so << "</td>";
					}
					else
					{
						so << "<td>";
						so << "</td>";
					}
				}

				// State
				if((*xi).DesignState() == AuDesign::Final)
				{
					so << "<td>";
					so << QString("<span style=\"font-family: sans-serif;\"><small>Final</small></span></a><br>");
					so << "</td>";
				}
				else
				{
					so << "<td>";
					so << QString("<span style=\"font-family: sans-serif;\"><small>Assembly</small></span></a><br>");
					so << "</td>";
				}

				// Architect
				so << "<td>";
				so << QString("<a class=\"ll\" href=\"p%1.php\">"
						"<span style=\"font-family: sans-serif;\"><small>%2</small></span></a><br>")
					.arg((*xi).DesignArchitectId().StringId())
					.arg((*xi).DesignArchitectName());
				so << "</td>";

				so << "<td>";
				so << QString("<span style=\"font-family: sans-serif;\"><small>%1</small></span></a><br>")
					.arg(empireName);
				so << "</td>";

				// Terms
				so << "<td>";
				{
					// Price
					QString terms;
					if((*xi).Price())
						terms = QString("Blueprint costs %L1%2.").arg((*xi).Price()).arg(AuGlobal::CurrencySymbol());
					else
						terms = QString("Blueprint is free.");

					// License
					if((*xi).CanCopy())
						terms += QString("<br>Can be copied, viewed and manufactured.");
					else if((*xi).CanView())
						terms += QString("<br>Can be viewed and manufactured.");
					else
						terms += QString("<br>Can be manufactured.");

					// Stance Restriction
					/*
					switch((*xi).MinStanceToBuyer())
					{
					case POL_Vassal:
					case POL_Enemy:
						terms += QString("<br>Accessible by all empires.");
						break;
					case POL_Neutral:
						terms += QString("<br>Accessible to all but enemy empires.");
						break;
					case POL_Friend:
						terms += QString("<br>Accessible to all but neutral and enemy empires.");
						break;
					case POL_Empire:
						terms += QString("<br>Accessible by publisher empire only.");
						break;
					}
					*/

					so << QString("<span style=\"font-family: sans-serif;\"><small>%1</small></span></a><br>")
						.arg(terms);
				}
				so << "</td>";

				so << "</tr>\n";
			}
			so << "</tbody></table>\n";
		}

		so << "</div>\n";
		so << "</body></html>\n";
		file.close();
	}

	file.setFileName("ExchangeByEmpire.php");
	if(file.open(QIODevice::WriteOnly))
	{
		QTextStream so(&file);
		so.setCodec("UTF-8");
		so << s_htmlHeader;
		so << s_titleBar.arg("Blueprint Exchange by Empire");
		so << s_bodyStart;
		so << QString("<div style=\"text-align: center; font-family: sans-serif;\"><big style=\"color: #ffffff; font-weight: bold; "
				"font-family: sans-serif\"><br>%1</big><br>\n"
				"<a class=\"a1\" href=\"Exchange.php\">By Name</a><small> | By Empire</small></div>")
			.arg("Shores of Hazeron Blueprint Exchange");

		// Make a link for each design in the map.
		so << "<div style=\"margin-left: 10px;\">\n";
		so << intro;

		so << "<table style=\"text-align: left; width: 100%; font-family: sans-serif;\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\"><tbody>";
		for(QMap<QString, quint32>::const_iterator ei = empireNames.cbegin(); ei != empireNames.cend(); ei++)
		{
			// Empire name row.
			so << "<tr>";
			so << "<td align=\"right\">";
			so << QString("<img title=\"%2\" style=\"height: 64px;\" alt=\"\" src=\"%1.png\"><br>")
				.arg(*ei).arg(ei.key());
			so << "</td>";
			so << QString("<td colspan=\"7\"; style=\"vertical-align: middle; background-color: %1;\"><big><b>%2</b></big><br></td>")
				.arg(hazBlue2)
				.arg(ei.key());
			so << "</tr>\n";

			// Spacecraft Blueprints
			bool anySc = false;
			for(QMultiMap<QString, quint32>::const_iterator si = scSorted.cbegin(); si != scSorted.cend(); si++)
			{
				AuScDesignExchange::const_iterator xi = scExch.find(*si);
				if(xi == scExch.cend())
					continue;	// wut?
				if((*xi).PublisherEmpire() != *ei)
					continue;
				anySc = true;
				break;
			}
			if(anySc)
			{
				so << "<tr>";
				so << QString("<td></td>");
				so << QString("<td colspan=\"2\"; style=\"vertical-align: top; background-color: %1;\"><small><b>Spacecraft Blueprint</b></small><br></td>").arg(hazBlue3);
				so << QString("<td style=\"vertical-align: top; background-color: %1;\"><small><b>Rating</b></small><br></td>").arg(hazBlue3);
				so << QString("<td style=\"vertical-align: top; background-color: %1;\"><small><b>MinQ</b></small><br></td>").arg(hazBlue3);
				so << QString("<td style=\"vertical-align: top; background-color: %1;\"><small><b>Size</b></small><br></td>").arg(hazBlue3);
				so << QString("<td style=\"vertical-align: top; background-color: %1;\"><small><b>State</b></small><br></td>").arg(hazBlue3);
				so << QString("<td style=\"vertical-align: top; background-color: %1;\"><small><b>Architect</b></small><br></td>").arg(hazBlue3);
				so << QString("<td style=\"vertical-align: top; background-color: %1;\"><small><b>Publisher Terms</b></small><br></td>").arg(hazBlue3);
				so << "</td>";
				for(QMultiMap<QString, quint32>::const_iterator si = scSorted.cbegin(); si != scSorted.cend(); si++)
				{
					AuScDesignExchange::const_iterator xi = scExch.find(*si);
					if(xi == scExch.cend())
						continue;	// wut?
					if((*xi).PublisherEmpire() != *ei)
						continue;
					QString empireName(ei.key());

					so << "<tr>";
					so << "<td>";
					so << "</td>";

					so << "<td align=\"right\">";
					so << QString("<img title=\"%2\" style=\"height: 64px;\" alt=\"\" src=\"sc%1.png\"><br>")
						.arg((*xi).DesignId())
						.arg((*xi).DesignName());
					so << "</td>";

					so << "<td>";
					so << QString("<a class=\"ll\" href=\"sc%1.php\">"
							"<span style=\"font-family: sans-serif;\"><big><b>%2</b></big><br><small><small><i>%1</i></small></small></span></a><br>")
						.arg((*xi).DesignId())
						.arg((*xi).DesignName());
					so << "</td>";

					// Rating
					so << "<td>";
					AuExchangeBlueprintRating rating;
					if(scRatings.Rating((*xi).DesignId(), rating))
					{
						QString rih = QString("<img src=\"qrc%1\" width=\"16\" height=\"16\"> %2")
							.arg(rating.RatingAverageIconName())
							.arg(rating.RatingAverageString());
						rih.replace("qrc:data","/images");
						so << rih;
					}
					so << "</td>";

					// Quality
					so << "<td>";
					if((*xi).DesignState() == AuDesign::Final)
						so << QString("<span style=\"font-family: sans-serif;\"><small>Q%1</small></span></a><br>")
							.arg((*xi).DesignQuality());
					so << "</td>";

					// Size
					if((*xi).FaceCount() || (*xi).TriangleCount())
					{
						if((*xi).DesignState() == AuDesign::Final)
						{
							so << "<td>";
							so << QString("<span style=\"font-family: sans-serif;\"><small>%L1m%2<br>%L3 faces<br>%L4 tris</small></span></a><br>")
								.arg((*xi).HullVolume())
								.arg(AuGlobal::CubedSymbol())
								.arg((*xi).FaceCount())
								.arg((*xi).TriangleCount());
							so << "</td>";
						}
						else
						{
							so << "<td>";
							so << QString("<span style=\"font-family: sans-serif;\"><small>%L1 faces<br>%L2 tris</small></span></a><br>")
								.arg((*xi).FaceCount())
								.arg((*xi).TriangleCount());
							so << "</td>";
						}
					}
					else
					{
						if((*xi).DesignState() == AuDesign::Final)
						{
							so << "<td>";
							so << QString("<span style=\"font-family: sans-serif;\"><small>%L1m%2</small></span></a><br>")
								.arg((*xi).HullVolume())
								.arg(AuGlobal::CubedSymbol());
							so << "</td>";
						}
						else
						{
							so << "<td>";
							so << "</td>";
						}
					}

					// State
					if((*xi).DesignState() == AuDesign::Final)
					{
						so << "<td>";
						so << QString("<span style=\"font-family: sans-serif;\"><small>Final</small></span></a><br>");
						so << "</td>";
					}
					else
					{
						so << "<td>";
						so << QString("<span style=\"font-family: sans-serif;\"><small>Assembly</small></span></a><br>");
						so << "</td>";
					}

					// Architect
					so << "<td>";
					so << QString("<a class=\"ll\" href=\"p%1.php\">"
							"<span style=\"font-family: sans-serif;\"><small>%2</small></span></a><br>")
						.arg((*xi).DesignArchitectId().StringId())
						.arg((*xi).DesignArchitectName());
					so << "</td>";

					// Terms
					so << "<td>";
					{
						// Price
						QString terms;
						if((*xi).Price())
							terms = QString("Blueprint costs %L1%2.").arg((*xi).Price()).arg(AuGlobal::CurrencySymbol());
						else
							terms = QString("Blueprint is free.");

						// License
						if((*xi).CanCopy())
							terms += QString("<br>Can be copied, viewed and manufactured.");
						else if((*xi).CanView())
							terms += QString("<br>Can be viewed and manufactured.");
						else
							terms += QString("<br>Can be manufactured.");

						// Stance Restriction
						/*
						switch((*xi).MinStanceToBuyer())
						{
						case POL_Vassal:
						case POL_Enemy:
							terms += QString("<br>Accessible by all empires.");
							break;
						case POL_Neutral:
							terms += QString("<br>Accessible to all but enemy empires.");
							break;
						case POL_Friend:
							terms += QString("<br>Accessible to all but neutral and enemy empires.");
							break;
						case POL_Empire:
							terms += QString("<br>Accessible by publisher empire only.");
							break;
						}
						*/

						so << QString("<span style=\"font-family: sans-serif;\"><small>%1</small></span></a><br>")
							.arg(terms);
					}
					so << "</td>";

					so << "</tr>\n";
				}
			}

			// Building Blueprints
			bool anyBd = false;
			for(QMultiMap<QString, quint32>::const_iterator si = bdSorted.cbegin(); si != bdSorted.cend(); si++)
			{
				AuBdDesignExchange::const_iterator xi = bdExch.find(*si);
				if(xi == bdExch.cend())
					continue;	// wut?
				if((*xi).PublisherEmpire() != *ei)
					continue;
				anyBd = true;
				break;
			}
			if(anyBd)
			{
				so << "<tr>";
				so << QString("<td></td>");
				so << QString("<td colspan=\"2\"; style=\"vertical-align: top; background-color: %1;\"><small><b>Building Blueprint</b></small><br></td>").arg(hazBlue3);
				so << QString("<td style=\"vertical-align: top; background-color: %1;\"><small><b>Rating</b></small><br></td>").arg(hazBlue3);
				so << QString("<td style=\"vertical-align: top; background-color: %1;\"><small><b>Use</b></small><br></td>").arg(hazBlue3);
				so << QString("<td style=\"vertical-align: top; background-color: %1;\"><small><b>Size</b></small><br></td>").arg(hazBlue3);
				so << QString("<td style=\"vertical-align: top; background-color: %1;\"><small><b>State</b></small><br></td>").arg(hazBlue3);
				so << QString("<td style=\"vertical-align: top; background-color: %1;\"><small><b>Architect</b></small><br></td>").arg(hazBlue3);
				so << QString("<td style=\"vertical-align: top; background-color: %1;\"><small><b>Publisher Terms</b></small><br></td>").arg(hazBlue3);
				so << "</td>";
				for(QMultiMap<QString, quint32>::const_iterator si = bdSorted.cbegin(); si != bdSorted.cend(); si++)
				{
					AuBdDesignExchange::const_iterator xi = bdExch.find(*si);
					if(xi == bdExch.cend())
						continue;	// wut?
					if((*xi).PublisherEmpire() != *ei)
						continue;
					QString empireName(ei.key());
					const DgData& gd = g_Au.LockDataForRead();
					QString iname;
					if((*xi).IndustryId() != IID_None)
						iname = gd.IndustryName((*xi).IndustryId());
					g_Au.UnlockData();

					so << "<tr>";
					so << "<td>";
					so << "</td>";

					so << "<td align=\"right\">";
					so << QString("<img title=\"%2\" style=\"height: 64px;\" alt=\"\" src=\"bd%1.png\"><br>")
						.arg((*xi).DesignId())
						.arg((*xi).DesignName());
					so << "</td>";

					so << "<td>";
					so << QString("<a class=\"ll\" href=\"bd%1.php\">"
							"<span style=\"font-family: sans-serif;\"><big><b>%2</b></big><br><small><small><i>%1</i></small></small></span></a><br>")
						.arg((*xi).DesignId())
						.arg((*xi).DesignName());
					so << "</td>";

					// Rating
					so << "<td>";
					AuExchangeBlueprintRating rating;
					if(bdRatings.Rating((*xi).DesignId(), rating))
					{
						QString rih = QString("<img src=\"qrc%1\" width=\"16\" height=\"16\"> %2")
							.arg(rating.RatingAverageIconName())
							.arg(rating.RatingAverageString());
						rih.replace("qrc:data","/images");
						so << rih;
					}
					so << "</td>";

					// Intended Industry
					so << "<td>";
					if(!iname.isEmpty())
						so << QString("<span style=\"font-family: sans-serif;\"><small>%1</small></span></a><br>")
							.arg(iname);
					so << "</td>";

					// Size
					if((*xi).FaceCount() || (*xi).TriangleCount())
					{
						if((*xi).DesignState() == AuDesign::Final)
						{
							so << "<td>";
							so << QString("<span style=\"font-family: sans-serif;\"><small>%L1m%2<br>%L3 faces<br>%L4 tris</small></span></a><br>")
								.arg((*xi).HullVolume())
								.arg(AuGlobal::CubedSymbol())
								.arg((*xi).FaceCount())
								.arg((*xi).TriangleCount());
							so << "</td>";
						}
						else
						{
							so << "<td>";
							so << QString("<span style=\"font-family: sans-serif;\"><small>%L1 faces<br>%L2 tris</small></span></a><br>")
								.arg((*xi).FaceCount())
								.arg((*xi).TriangleCount());
							so << "</td>";
						}
					}
					else
					{
						if((*xi).DesignState() == AuDesign::Final)
						{
							so << "<td>";
							so << QString("<span style=\"font-family: sans-serif;\"><small>%L1m%2</small></span></a><br>")
								.arg((*xi).HullVolume())
								.arg(AuGlobal::CubedSymbol());
							so << "</td>";
						}
						else
						{
							so << "<td>";
							so << "</td>";
						}
					}

					// State
					if((*xi).DesignState() == AuDesign::Final)
					{
						so << "<td>";
						so << QString("<span style=\"font-family: sans-serif;\"><small>Final</small></span></a><br>");
						so << "</td>";
					}
					else
					{
						so << "<td>";
						so << QString("<span style=\"font-family: sans-serif;\"><small>Assembly</small></span></a><br>");
						so << "</td>";
					}

					// Architect
					so << "<td>";
					so << QString("<a class=\"ll\" href=\"p%1.php\">"
							"<span style=\"font-family: sans-serif;\"><small>%2</small></span></a><br>")
						.arg((*xi).DesignArchitectId().StringId())
						.arg((*xi).DesignArchitectName());
					so << "</td>";

					// Terms
					so << "<td>";
					{
						// Price
						QString terms;
						if((*xi).Price())
							terms = QString("Blueprint costs %L1%2.").arg((*xi).Price()).arg(AuGlobal::CurrencySymbol());
						else
							terms = QString("Blueprint is free.");

						// License
						if((*xi).CanCopy())
							terms += QString("<br>Can be copied, viewed and manufactured.");
						else if((*xi).CanView())
							terms += QString("<br>Can be viewed and manufactured.");
						else
							terms += QString("<br>Can be manufactured.");

						// Stance Restriction
						/*
						switch((*xi).MinStanceToBuyer())
						{
						case POL_Vassal:
						case POL_Enemy:
							terms += QString("<br>Accessible by all empires.");
							break;
						case POL_Neutral:
							terms += QString("<br>Accessible to all but enemy empires.");
							break;
						case POL_Friend:
							terms += QString("<br>Accessible to all but neutral and enemy empires.");
							break;
						case POL_Empire:
							terms += QString("<br>Accessible by publisher empire only.");
							break;
						}
						*/

						so << QString("<span style=\"font-family: sans-serif;\"><small>%1</small></span></a><br>")
							.arg(terms);
					}
					so << "</td>";

					so << "</tr>\n";
				}
			}
		}
		so << "</tbody></table>\n";
		so << "</div>\n";
		so << "</body></html>\n";
		file.close();
	}

	// Now make the individual design pages.
	//qDebug() << "Individual pages for designs" << players.count() << QDateTime::currentDateTime();
	// Spacecraft Blueprints
	for(AuScDesignExchange::const_iterator xi = scExch.cbegin(); xi != scExch.cend(); xi++)
	{
		// Save the design image.
		int imgWidth = 0, imgHeight = 0;
		QString imgFile = QString("sc%1.png").arg((*xi).DesignId());
		{
			// Load the image from the design record.
			QImage imgThumb;
			{
				// Load design thumbnail image. Unfortunately this is glommed into the textures field of the design. :/
				QString sq;
				sq = QString("SELECT textures FROM n_ScDesign WHERE id=%1;").arg((*xi).DesignId());
				QSqlQuery qry(QString(), dbUniverse);
				qry.prepare(sq);
				qry.exec();
				if(qry.next())
				{
					int qvi=0;		// query value index
					QByteArray ba = qry.value(qvi).toByteArray();
					QDataStream in(ba);
					in.setVersion(QDataStream::Qt_5_0);
					AuScDesign des;
					if(!des.ReadTextures(in))
					{
						in.device()->seek(0);
						des.ReadTexturesOld(in);
					}
					in >> imgThumb;
				}
			}
			if(!imgThumb.isNull())
			{
				imgThumb.save(imgFile);
				imgWidth = imgThumb.width();
				imgHeight = imgThumb.height();
			}
		}

		QString rpt = (*xi).ReportText();
		rpt.replace("qrc:data","/images");

		// Load reviews of the design.
		QString reviews = scRatings.WebReviews((*xi).DesignId());
		reviews.replace("qrc:data","/images");

		// Save the design's page.
		file.setFileName(QString("sc%1.php").arg((*xi).DesignId()));
		if(file.open(QIODevice::WriteOnly))
		{
			QTextStream so(&file);
			so.setCodec("UTF-8");
			so << s_htmlHeader;
			so << s_titleBar.arg((*xi).DesignName());
			so << s_bodyStart;
			so << QString("<div style=\"text-align: center; font-family: sans-serif;\">"
					"<big style=\"font-weight: bold; font-family: sans-serif\"><br>%1</big></div>").arg((*xi).DesignName());

			so << QString("<br><div style=\"text-align: center;\">"
					"<img title=\"%2\" style=\"width: %3px; height: %4px;\" src=\"sc%1.png\"></div>")
				.arg((*xi).DesignId())
				.arg((*xi).DesignName())
				.arg(imgWidth*3)
				.arg(imgHeight*3);

			so << "<span style=\"font-family: sans-serif;\">";

			so << "<table><tbody>";
			so << "<tr style=\"vertical-align: top;\">";
			so << "<td>";
			so << rpt;
			so << "</td>";
			so << "<td width=\"320\">";
			so << reviews;
			so << "</td>";
			so << "</tr>";
			so << "</tbody></table>";

			so << "</span>";
			so << "</body></html>";
			file.close();
		}
	}

	// Building Blueprints
	for(AuBdDesignExchange::const_iterator xi = bdExch.cbegin(); xi != bdExch.cend(); xi++)
	{
		// Save the design image.
		int imgWidth = 0, imgHeight = 0;
		QString imgFile = QString("bd%1.png").arg((*xi).DesignId());
		{
			// Load the image from the design record.
			QImage imgThumb;
			{
				// Load design thumbnail image. Unfortunately this is glommed into the textures field of the design. :/
				QString sq;
				sq = QString("SELECT textures FROM n_BdDesign WHERE id=%1;").arg((*xi).DesignId());
				QSqlQuery qry(QString(), dbUniverse);
				qry.prepare(sq);
				qry.exec();
				if(qry.next())
				{
					int qvi=0;		// query value index
					QByteArray ba = qry.value(qvi).toByteArray();
					QDataStream in(ba);
					in.setVersion(QDataStream::Qt_5_0);
					AuBdDesign des;
					if(!des.ReadTextures(in))
					{
						in.device()->seek(0);
						des.ReadTexturesOld(in);
					}
					in >> imgThumb;
				}
			}
			if(!imgThumb.isNull())
			{
				imgThumb.save(imgFile);
				imgWidth = imgThumb.width();
				imgHeight = imgThumb.height();
			}
		}

		QString rpt = (*xi).ReportText();
		rpt.replace("qrc:data","/images");

		// Load reviews of the design.
		QString reviews = bdRatings.WebReviews((*xi).DesignId());
		reviews.replace("qrc:data","/images");

		// Save the design's page.
		file.setFileName(QString("bd%1.php").arg((*xi).DesignId()));
		if(file.open(QIODevice::WriteOnly))
		{
			QTextStream so(&file);
			so.setCodec("UTF-8");
			so << s_htmlHeader;
			so << s_titleBar.arg((*xi).DesignName());
			so << s_bodyStart;
			so << QString("<div style=\"text-align: center; font-family: sans-serif;\">"
					"<big style=\"font-weight: bold; font-family: sans-serif\"><br>%1</big></div>").arg((*xi).DesignName());

			so << QString("<br><div style=\"text-align: center;\">"
					"<img title=\"%2\" style=\"width: %3px; height: %4px;\" src=\"bd%1.png\"></div>")
				.arg((*xi).DesignId())
				.arg((*xi).DesignName())
				.arg(imgWidth*3)
				.arg(imgHeight*3);

			so << "<span style=\"font-family: sans-serif;\">";

			so << "<table><tbody>";
			so << "<tr style=\"vertical-align: top;\">";
			so << "<td>";
			so << rpt;
			so << "</td>";
			so << "<td width=\"320\">";
			so << reviews;
			so << "</td>";
			so << "</tr>";
			so << "</tbody></table>";

			so << "</span>";
			so << "</body></html>";
			file.close();
		}
	}
}

// Locates an atom id in the database and reports information about it.
void AuEmpireReport::FindPerson(QSqlDatabase& dbUniverse, const QString& personName, bool reportGear, bool reportMemory)
{
	QStringList textLines;
	DaAtom::DbFindPerson(textLines, dbUniverse, personName, reportGear, reportMemory, d_PrefixStep);
	for(QStringList::const_iterator si = textLines.cbegin(); si != textLines.cend(); si++)
		qDebug() << *si;
}

// Locates an atom id in the database and reports information about it.
void AuEmpireReport::FindAtom(QSqlDatabase& dbUniverse, const AuId& atomId, bool reportGear, bool reportMemory)
{
	QStringList textLines;
	DaAtom::DbFindAtom(textLines, dbUniverse, atomId, reportGear, reportMemory, d_PrefixStep);
	for(QStringList::const_iterator si = textLines.cbegin(); si != textLines.cend(); si++)
		qDebug() << *si;
}

// Reports information about an atom and returns its parent atom id.
AuId AuEmpireReport::ReportAtom(QSqlDatabase& dbUniverse, const AuId& atomId, bool includeGear, bool includeMemory,
		const char* prefix, const char* prefixStep)
{
	QStringList textLines;
	AuId parentId = DaAtom::DbQueryAtom(textLines, dbUniverse, atomId, includeGear, includeMemory, prefix, prefixStep);
	for(QStringList::const_iterator si = textLines.cbegin(); si != textLines.cend(); si++)
		qDebug() << *si;
	return parentId;
}

void AuEmpireReport::ReportShipMission(QSqlDatabase& dbUniverse, const AuId& shipId)
{
	QString sq;
	sq = QString("SELECT name,mission FROM a_Sc WHERE id=%1;").arg(shipId.GetID());
	QSqlQuery qry(QString(), dbUniverse);
	qry.prepare(sq);
	qry.exec();
	if(qry.next())
	{
		int qv=0;		// query value index
		QString name = qry.value(qv++).toString();

		int qvi = qv++;
		if(!qry.value(qvi).isNull())
		{
			// Mission
			QByteArray ba = qry.value(qvi).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_4_1);
			AuMission mission;
			mission << in;

			qDebug() << "Mission" << mission;
		}
	}
}

// Searches for people and officers who look like they might be in limbo.
void AuEmpireReport::ReportLimboAtoms(QSqlDatabase& dbUniverse)
{
	QStringList textLines = DaAtom::DbLimboCheck(dbUniverse, d_PrefixStep);
	for(QStringList::const_iterator si = textLines.cbegin(); si != textLines.cend(); si++)
		qDebug() << *si;
}

void AuEmpireReport::ReportLostShips(QSqlDatabase& dbUniverse, AuIdList& limboRescueMe)
{
	QStringList textLines = DaAtom::DbLostShips(dbUniverse, limboRescueMe);
	for(QStringList::const_iterator si = textLines.cbegin(); si != textLines.cend(); si++)
		qDebug() << *si;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuEmpireReport::ReportSystem(QSqlDatabase& dbUniverse, const QString& systemName)
{
	QStringList textLines;
	DaAtom::DbQuerySystem(textLines, dbUniverse, systemName, d_PrefixStep);
	for(QStringList::const_iterator si = textLines.cbegin(); si != textLines.cend(); si++)
		qDebug() << *si;
}

void AuEmpireReport::ReportSystem(QSqlDatabase& dbUniverse, const AuId& systemId)
{
	QStringList textLines;
	DaAtom::DbQuerySystem(textLines, dbUniverse, systemId, d_PrefixStep);
	for(QStringList::const_iterator si = textLines.cbegin(); si != textLines.cend(); si++)
		qDebug() << *si;
}

void AuEmpireReport::ReportWorld(QSqlDatabase& dbUniverse, const QString& worldName)
{
	QStringList textLines;
	DaAtom::DbQueryWorld(textLines, dbUniverse, worldName, d_PrefixStep);
	for(QStringList::const_iterator si = textLines.cbegin(); si != textLines.cend(); si++)
		qDebug() << *si;
}

void AuEmpireReport::ReportWorld(QSqlDatabase& dbUniverse, const AuId& worldId)
{
	QStringList textLines;
	DaAtom::DbQueryWorld(textLines, dbUniverse, worldId, d_PrefixStep);
	for(QStringList::const_iterator si = textLines.cbegin(); si != textLines.cend(); si++)
		qDebug() << *si;
}

void AuEmpireReport::ReportDuplicateAtomIds(QSqlDatabase& dbUniverse)
{
	QList<quint32> atoms;

	qDebug() << "Looking for duplicate atom ids...";
	QString sq = QString("SELECT id FROM a_Atom;");
	QSqlQuery qa(sq, dbUniverse);
	while(qa.next())
	{
		quint32 id = qa.value(0).toUInt();
		atoms.append(id);
		if(atoms.count() % 1000 == 0)
			qDebug() << "Read" << atoms.count() << "atoms";
	}

	qDebug() << "Checking" << atoms.count() << "atoms for duplicate ids...";

	int dupCount = 0;

	QHash<quint32, bool> atomIds;
	for(QList<quint32>::const_iterator ai = atoms.cbegin(); ai != atoms.cend(); ai++)
	{
		if(atomIds.contains(*ai))
		{
			qDebug() << "  Duplicate: Atom id" << (*ai);
			dupCount++;
		}
		else
			atomIds.insert(*ai, true);
	}

	qDebug() << atoms.count() << "total atoms." << dupCount << "duplicate atom ids found.";
}

void AuEmpireReport::ReportDuplicateSectors(QSqlDatabase& dbUniverse)
{
	int dupCount = 0;
	int readCount = 0;

	QMap<AuPoint, quint32> sectors;	// key is sector location, value is id
	qDebug() << "Looking for sectors that exist in the same location...";
	QString sq = QString("SELECT id,locx,locy,locz FROM a_Sector;");
	QSqlQuery qa(sq, dbUniverse);
	while(qa.next())
	{
		int qv=0;
		quint32 id = qa.value(qv++).toUInt();
		AuPoint loc;
		loc.x = qa.value(qv++).toDouble();
		loc.y = qa.value(qv++).toDouble();
		loc.z = qa.value(qv++).toDouble();

		if(sectors.contains(loc))
		{
			qDebug() << "  Overlap: Sector" << id << "loc" << loc.x << loc.y << loc.z;
			dupCount++;
		}
		sectors.insert(loc, id);

		readCount++;
		if(readCount % 1000 == 0)
			qDebug() << "Read" << readCount << "sectors";
	}

	qDebug() << readCount << "total sectors." << dupCount << "overlapping sectors found.";
}

void AuEmpireReport::ReportMultiplePrimaries(QSqlDatabase& dbUniverse)
{
	int dupCount = 0;
	int readCount = 0;

	QHash<AuId, AuId> primaries;	// key is system id, value is star id
	qDebug() << "Looking for solar systems with multiple primary stars...";
	QString sq = QString("SELECT idp,id FROM a_Star WHERE idp IN (SELECT id FROM a_SolarSystem);");	// Select all primary stars.
	QSqlQuery qa(sq, dbUniverse);
	while(qa.next())
	{
		int qv=0;
		AuId id = AuId(qa.value(qv++).toUInt());
		AuId idp = AuId(qa.value(qv++).toUInt());

		if(primaries.contains(idp))
		{
			qDebug() << "  Redundant: Star" << id << "in solar system" << idp;
			dupCount++;
		}
		primaries.insert(idp, id);

		readCount++;
		if(readCount % 1000 == 0)
			qDebug() << "Read" << readCount << "primary stars";
	}

	qDebug() << readCount << "total primary stars." << dupCount << "redundant primary stars found.";
}

void AuEmpireReport::ReportDecaySystems(QSqlDatabase& dbUniverse)
{
	int readCount = 0;
	int decayCount = 0;

	AuEther ether(__FILE__, __LINE__);

	qDebug() << "Looking for decaying solar systems...";
	QString sq = QString("SELECT id FROM a_SolarSystem WHERE id IN (SELECT idp FROM a_Star);");
	QSqlQuery qa(sq, dbUniverse);
	while(qa.next())
	{
		int qv=0;
		AuId id = AuId(qa.value(qv++).toUInt());

		DaSolarSystem* system = DaSolarSystem::LoadSubatoms(ether, dbUniverse, AuId(0), LOAD_All, id);
		if(!system)
			continue;

		// Loaded the solar system.
		int lastPersonVisit = system->LastPersonVisited().ElapsedDay();
		int lastOfficerVisit = system->LastOfficerVisited().ElapsedDay();
		if(lastPersonVisit >= 7 && lastOfficerVisit >= 7 && !system->FindPersonRecursive())
		{
			// One week has passed since this system was last visited.
			// If there are no cities, this system can decay completely.
			/*DHH DaCity
			DaAtomList cities;
			system->FindCitiesRecursive(cities);
			if(cities.isEmpty())
			{
				qDebug() << "  Decay:" << system->Id() << system->Name()
					<< "lastPersonVisit" << system->LastPersonVisited() << lastPersonVisit
					<< "days ago, lastOfficerVisit" << system->LastOfficerVisited() << lastOfficerVisit
					<< "days ago";
				decayCount++;
			}
			*/
		}

		ether.TrashAtom(id);

		readCount++;
		if(readCount % 1000 == 0)
			qDebug() << "Read" << readCount << "solar systems";
	}

	qDebug() << readCount << "total non-starfield solar systems." << decayCount << "ready to decay.";
}

void AuEmpireReport::DecaySystems(QSqlDatabase& dbUniverse)
{
	int readCount = 0;
	int delSysCount = 0;
	AuIdList deletedAtomIds;

	AuEther ether(__FILE__, __LINE__);

	qDebug() << "Decaying solar system...";
	QString sq = QString("SELECT id FROM a_SolarSystem WHERE id IN (SELECT idp FROM a_Star);");
	QSqlQuery qa(sq, dbUniverse);
	while(qa.next())
	{
		int qv=0;
		AuId id = AuId(qa.value(qv++).toUInt());

		DaSolarSystem* system = DaSolarSystem::LoadSubatoms(ether, dbUniverse, AuId(0), LOAD_All, id);
		if(!system)
			continue;

		// Loaded the solar system.
		int lastPersonVisit = system->LastPersonVisited().ElapsedDay();
		int lastOfficerVisit = system->LastOfficerVisited().ElapsedDay();
		if(lastPersonVisit >= 7 && lastOfficerVisit >= 7 && !system->FindPersonRecursive())
		{
			// One week has passed since this system was last visited.
			// If there are no cities, this system can decay completely.
			/*DHH DaCity
			DaAtomList cities;
			system->FindCitiesRecursive(cities);
			if(cities.isEmpty())
			{
				qDebug() << "  Decay:" << system->Id() << system->Name()
					<< "lastPersonVisit" << system->LastPersonVisited() << lastPersonVisit
					<< "days ago, lastOfficerVisit" << system->LastOfficerVisited() << lastOfficerVisit
					<< "days ago";

				// Delete all subatoms of the solar system but not the solar system itself.
				// This returns it to the starfield-only state.
				DaAtomList deleteAtoms;
				system->ListSubatomsRecursive(deleteAtoms);
				for(DaAtomList::const_iterator di = deleteAtoms.cbegin(); di != deleteAtoms.cend(); di++)
					deletedAtomIds.append((*di)->Id());
				delSysCount++;
			}
			*/
		}

		ether.TrashAtom(id);

		readCount++;
		if(readCount % 1000 == 0)
			qDebug() << "Read" << readCount << "solar systems";
	}

	qDebug() << readCount << "total non-starfield solar systems." << delSysCount << "systems decayed. Deleting"
		<< deletedAtomIds.count() << "atoms...";

	DeleteAtoms(dbUniverse, deletedAtomIds);

	qDebug() << "Done deleting atoms from decayed solar systems.";
}

void AuEmpireReport::DeleteAtoms(QSqlDatabase& dbUniverse, const AuIdList& deleteIds)
{
	AuIdList deletedAtomIds(deleteIds);
	while(!deletedAtomIds.isEmpty())
	{
		// Build a query for a batch of the atoms that we need to delete, up to 1000 per batch.
		QString sqa("DELETE FROM a_Atom WHERE");
		QString delid;
		int delCount = 1000;									// We will delete in batches of 1000 max.
		while(!deletedAtomIds.isEmpty() && delCount--)
		{
			AuId id = deletedAtomIds.takeFirst();

			if(!delid.isEmpty())
			{
				// OR additional Id's after the first.
				sqa += " OR";
			}

			delid = QVariant(id.GetID()).toString();
			sqa += " id=" + delid;
		}
		sqa += ";";

		// Run the delete atom query.
		QSqlQuery qdela(dbUniverse);
		qdela.prepare(sqa);
		if(!qdela.exec())
			qWarning("DeleteAtoms - DELETE FAILED '%s'", (const char*)sqa.toLatin1());
	}
}

void AuEmpireReport::ReportBadSectorCoords(QSqlDatabase& dbUniverse)
{
	int readCount = 0;
	int badCount = 0;

	qDebug() << "Looking for sectors that have bad coordinates...";

	QString sq = QString("SELECT id,locx,locy,locz FROM a_Sector;");
	QSqlQuery qa(sq, dbUniverse);
	while(qa.next())
	{
		int qv=0;
		AuId id = AuId(qa.value(qv++).toUInt());
		double x = qa.value(qv++).toDouble();
		double y = qa.value(qv++).toDouble();
		double z = qa.value(qv++).toDouble();
		AuPoint sectorCoord(x,y,z);
		AuPoint roundedCoord(sectorCoord);
		roundedCoord -= AuPoint(5,5,5);
		roundedCoord.Round(10);
		roundedCoord += AuPoint(5,5,5);
		if(sectorCoord != roundedCoord)
		{
			qDebug() << "Sector" << id << "sectorCoord" << sectorCoord << "roundedCoord" << roundedCoord;
			badCount++;
		}

		readCount++;
		if(readCount % 1000 == 0)
			qDebug() << "Read" << readCount << "sectors, last was at" << sectorCoord;
	}

	qDebug() << readCount << "total sectors." << badCount << "have bad coordinates.";
}

void AuEmpireReport::ReportCommodities()
{
	qDebug("---------- Commodity Base Costs ----------");
	for(DbCommodityArray::const_iterator it = g_Au.m_CommodityArray.cbegin(); it != g_Au.m_CommodityArray.cend(); ++it)
		if((*it).m_id != CID_Invalid)
			qDebug(" %3d. %12.2f %s", (*it).m_id, (*it).BaseCost(), (const char*)(*it).m_name.toLatin1());
	qDebug("------------------------------------------");
}

GALAXY AuEmpireReport::QueryParentGalaxyRecursive(QSqlDatabase& dbUniverse, const AuId& atomId)
{
	QString sgal;
	sgal = QString("SELECT galaxy FROM a_Galaxy WHERE id=%1;").arg(atomId.GetID());
	QSqlQuery qgal(QString(), dbUniverse);
	qgal.prepare(sgal);
	qgal.exec();
	if(qgal.next())
	{
		int galaxy = qgal.value(0).toInt();
		return (GALAXY)galaxy;
	}

	// Didn't find the galaxy, so find the parent of atomId and recurse.
	QString spar;
	spar = QString("SELECT idp FROM a_Atom WHERE id=%1;").arg(atomId.GetID());
	QSqlQuery qpar(QString(), dbUniverse);
	qpar.prepare(spar);
	qpar.exec();
	if(qpar.next())
	{
		AuId parId = AuId(qpar.value(0).toUInt());
		if(!parId.IsNull())
			return QueryParentGalaxyRecursive(dbUniverse, parId);
		else
			qWarning() << "error: QueryParentGalaxyRecursive atomId" << atomId << "has parentId 0";
	}
	else
	{
		// No parent record exists?
		qWarning() << "error: QueryParentGalaxyRecursive atomId" << atomId << "does not exist";
	}
	return GALAXY_Count;
}

AuId AuEmpireReport::QueryParentSectorRecursive(QSqlDatabase& dbUniverse, const AuId& atomId, QString& sectorName)
{
	QString ssect;
	ssect = QString("SELECT id,name FROM a_Sector WHERE id=%1;").arg(atomId.GetID());
	QSqlQuery qsect(QString(), dbUniverse);
	qsect.prepare(ssect);
	qsect.exec();
	if(qsect.next())
	{
		AuId sectorId = AuId(qsect.value(0).toUInt());
		sectorName = qsect.value(1).toString();
		return sectorId;
	}

	// Didn't find the sector, so find the parent of atomId and recurse.
	QString spar;
	spar = QString("SELECT idp FROM a_Atom WHERE id=%1;").arg(atomId.GetID());
	QSqlQuery qpar(QString(), dbUniverse);
	qpar.prepare(spar);
	qpar.exec();
	if(qpar.next())
	{
		AuId parId = AuId(qpar.value(0).toUInt());
		if(!parId.IsNull())
			return QueryParentSectorRecursive(dbUniverse, parId, sectorName);
		else
			qWarning() << "error: QueryParentSectorRecursive atomId" << atomId << "has parentId 0";
	}
	else
	{
		// No parent record exists?
		qWarning() << "error: QueryParentSectorRecursive atomId" << atomId << "does not exist";
	}
	return AuId();
}

AuId AuEmpireReport::QueryParentSystemRecursive(QSqlDatabase& dbUniverse, const AuId& atomId, QString& systemName)
{
	QString ssect;
	ssect = QString("SELECT id,name FROM a_SolarSystem WHERE id=%1;").arg(atomId.GetID());
	QSqlQuery qsect(QString(), dbUniverse);
	qsect.prepare(ssect);
	qsect.exec();
	if(qsect.next())
	{
		AuId sectorId = AuId(qsect.value(0).toUInt());
		systemName = qsect.value(1).toString();
		return sectorId;
	}

	// Didn't find the sector, so find the parent of atomId and recurse.
	QString spar;
	spar = QString("SELECT idp FROM a_Atom WHERE id=%1;").arg(atomId.GetID());
	QSqlQuery qpar(QString(), dbUniverse);
	qpar.prepare(spar);
	qpar.exec();
	if(qpar.next())
	{
		AuId parId = AuId(qpar.value(0).toUInt());
		if(!parId.IsNull())
			return QueryParentSystemRecursive(dbUniverse, parId, systemName);
		else
			qWarning() << "error: QueryParentSystemRecursive atomId" << atomId << "has parentId 0";
	}
	else
	{
		// No parent record exists?
		qWarning() << "error: QueryParentSystemRecursive atomId" << atomId << "does not exist";
	}
	return AuId();
}

class TestClass
{
public:
	TestClass()
	{
		qDebug() << "TestClass constructor" << this;
	}

	~TestClass()
	{
		qDebug() << "TestClass destructor" << this;
	}
};

quint32 CalcTurretFireMs(quint32 xp)
{
	if(xp == 0)
		return 5000;
	float x = (float)xp / 10000.f;
	float fx = 1500.f / (5.f * x);
	quint32 ms = (quint32)ceilf(fx) + 500;
	if(ms > 5000)
		ms = 5000;
	return ms;
}

void AuEmpireReport::DoCalc()
{
	// From: http://eusebeia.dyndns.org/4d/icosahedron
	// The Cartesian coordinates of an origin-centered regular icosahedron of edge length 2 are:
	//
	// (0, +/- 1, +/- y)
	// (+/- 1, +/- y, 0)
	// (+/- y, 0, +/- 1)
	// 
	// where y=(1 + sqrt(5)/2 is the Golden Ratio.

	// Vertices of an icosahedron.
	double y = (sqrt(5.) + 1.) / 2.;
	AuPoint icoverts[] =
	{
		AuPoint(0, 1, y),
		AuPoint(0, 1, -y),
		AuPoint(0, -1, y),
		AuPoint(0, -1, -y),
		AuPoint(1, y, 0),
		AuPoint(1, -y, 0),
		AuPoint(-1, y, 0),
		AuPoint(-1, -y, 0),
		AuPoint(y, 0, 1),
		AuPoint(-y, 0, 1),
		AuPoint(y, 0, -1),
		AuPoint(-y, 0, -1),
	};

	// Output unit vectors.
	for(int i=0; i<12; i++)
	{
		AuPoint u(icoverts[i]);
		u.Unit();
		qDebug() << u;
	}

	return;

	/* test uninitialized variables
	int ua,ub,uc;
	qDebug() << "Uninitialized ua" << ua << "ub" << ub << "uc" << uc;
	return;
	*/

	qDebug() << "1700k" << AuGLLight::Color(0) << AuGLLight::ColorKelvin(1700);
	qDebug() << "1900k" << AuGLLight::Color(1) << AuGLLight::ColorKelvin(1900);
	qDebug() << "1950k" << AuGLLight::Color(2) << AuGLLight::ColorKelvin(1950);
	qDebug() << "2000k" << AuGLLight::Color(3) << AuGLLight::ColorKelvin(2000);
	qDebug() << "2300k" << AuGLLight::Color(4) << AuGLLight::ColorKelvin(2300);
	qDebug() << "2600k" << AuGLLight::Color(5) << AuGLLight::ColorKelvin(2600);
	qDebug() << "2700k" << AuGLLight::Color(6) << AuGLLight::ColorKelvin(2700);
	qDebug() << "2850k" << AuGLLight::Color(7) << AuGLLight::ColorKelvin(2850);
	qDebug() << "3000k" << AuGLLight::Color(8) << AuGLLight::ColorKelvin(3000);
	qDebug() << "3200k" << AuGLLight::Color(9) << AuGLLight::ColorKelvin(3200);
	qDebug() << "4000k" << AuGLLight::Color(10) << AuGLLight::ColorKelvin(4000);
	qDebug() << "4500k" << AuGLLight::Color(11) << AuGLLight::ColorKelvin(4500);
	qDebug() << "5000k" << AuGLLight::Color(12) << AuGLLight::ColorKelvin(5000);
	qDebug() << "5500k" << AuGLLight::Color(13) << AuGLLight::ColorKelvin(5500);
	qDebug() << "6500k" << AuGLLight::Color(14) << AuGLLight::ColorKelvin(6500);
	qDebug() << "7000k" << AuGLLight::Color(15) << AuGLLight::ColorKelvin(7000);
	qDebug() << "7500k" << AuGLLight::Color(16) << AuGLLight::ColorKelvin(7500);
	qDebug() << "8000k" << AuGLLight::Color(17) << AuGLLight::ColorKelvin(8000);
	return;

	qDebug() << "0xp" << CalcTurretFireMs(0) << "ms between shots";
	qDebug() << "1xp" << CalcTurretFireMs(1) << "ms between shots";
	qDebug() << "2xp" << CalcTurretFireMs(2) << "ms between shots";
	qDebug() << "720xp" << CalcTurretFireMs(720) << "ms between shots";
	qDebug() << "1000xp" << CalcTurretFireMs(1000) << "ms between shots";
	qDebug() << "1440xp" << CalcTurretFireMs(1440) << "ms between shots";
	qDebug() << "2000xp" << CalcTurretFireMs(2000) << "ms between shots";
	qDebug() << "3000xp" << CalcTurretFireMs(3000) << "ms between shots";
	qDebug() << "4000xp" << CalcTurretFireMs(4000) << "ms between shots";
	qDebug() << "20000xp" << CalcTurretFireMs(20000) << "ms between shots";
	qDebug() << "50000xp" << CalcTurretFireMs(50000) << "ms between shots";
	qDebug() << "100000xp" << CalcTurretFireMs(100000) << "ms between shots";

	return;

	AuTestCopy a(1);
	AuTestCopy b(2);

	a = b;

	qDebug() << "Calling RefArg";
	a.RefArg(b);

	qDebug() << "Calling ConstRefArg";
	a.ConstRefArg(b);

	qDebug() << "Calling ValueArg";
	a.ValueArg(b);

	qDebug() << "Calling ConstValueArg";
	a.ConstValueArg(b);

	return;
	// Ringworld sections
	{
		for(int orbit=0; orbit<16; orbit++)
		{
			float radAu = DaStar::OrbitRadius(orbit);
			qDebug() << "orbit" << orbit << "radAu" << radAu << "ringworld sections"
				<< DaWorldRingSection::RingworldSections(radAu);
		}
		return;
	}

	// Backslash test
	{
		QSqlDatabase dbUniverse = QSqlDatabase::database("AuUniverse", false);
		if(!dbUniverse.isOpen())
		{
			qWarning() << "error: AuUniverse database not open";
			return;
		}

		QString testName = QString("Haxus\\Norb\\\\Bert");
		testName = "Haxus";
		qDebug() << "qDebug testName" << testName;

		QFile sofile;
		sofile.open(stdout, QIODevice::WriteOnly);
		QTextStream so(&sofile);
		so.setCodec("UTF-8");
		so << "test name " << testName << '\n';

		QString squpd;
		squpd = QString("UPDATE a_Person SET name=? WHERE id=16;");
		QSqlQuery qupd(QString(), dbUniverse);
		qupd.prepare(squpd);
		qupd.addBindValue(testName);
		qupd.exec();

		QString sq = QString("SELECT id,name FROM a_Person WHERE id=16;");
		QSqlQuery qa(QString(), dbUniverse);
		qa.prepare(sq);
		qa.exec();
		while(qa.next())
		{
			int qv=0;
			AuId id = AuId(qa.value(qv++).toUInt());
			QString name = qa.value(qv++).toString();
			so << "read name " << name << '\n';
		}
		return;
	}

	// World sizes
	{
		qDebug() << "Planets and Asteroids";
		for(int sz=6; sz<=128; sz++)
		{
			double dia = FROUND(::Radius100km(sz) * 2. / FeetPerMeter(), 100);
			QString sdia = QString("%L1m dia").arg(dia);			//...in meters.
			if(sz == RadiusStandard100km())
				qDebug() << "size" << sz << sdia << "Earth standard";
			else
				qDebug() << "size" << sz << sdia;
		}

		qDebug() << "Gas Giants";
		for(int sz=20; sz<=40; sz++)
		{
			double dia = FROUND(::Radius100km(sz*10) * 2. / FeetPerMeter(), 100);
			QString sdia = QString("%L1m dia").arg(dia);			//...in meters.
			qDebug() << "size" << sz << sdia;
		}
		return;
	}

	// Standard Normal Distribution
	{
		// Generate lots of values.
		QHash<quint8, int> results;
		AuDice dice(1);
		dice.TimeSeed();
		int total = 1000000;
		for(int i=0; i<total; i++)
		{
			quint8 q = dice.RandomQuality();
			QHash<quint8, int>::iterator ri = results.find(q);
			if(ri != results.end())
				(*ri)++;
			else
				results.insert(q, 1);
		}

		for(quint8 q=1; q; q++)
		{
			QHash<quint8, int>::iterator ri = results.find(q);
			int c;
			if(ri != results.end())
				c = *ri;
			else
				c = 0;
			qDebug() << "Q" << q << "count" << c << "%" << ((double)(c * 100)/total);
		}
		return;
	}

	{
		qDebug() << "float" << sizeof(float) << "bytes";
		qDebug() << "double" << sizeof(double) << "bytes";
		qDebug() << "qreal" << sizeof(qreal) << "bytes";
		qDebug() << "AuMatrix4x4" << sizeof(AuMatrix4x4) << "bytes";
		qDebug() << "QMatrix4x4" << sizeof(QMatrix4x4) << "bytes";	// floats
		return;
	}

	{
		// Test FromSpecies
		AuDNA dnaHuman;
		dnaHuman.InitDefaultHuman();
		AuSpecies speciesHuman = dnaHuman.Species();

		AuDNA dna;
		dna.FromSpecies(speciesHuman);

		qDebug() << "FromSpecies result isHuman" << (dna.Species() == speciesHuman);
		if(dna.Species() != speciesHuman)
		{
			qDebug() << "dna comparison" << (dnaHuman == dna);
			qDebug() << "speciesHuman" << speciesHuman.Key().size();
			qDebug() << "dna" << dna.Species().Key().size();
		}
		return;
	}

	{
		// Compare species of dna.
		AuDNA dnaHuman;
		dnaHuman.InitDefaultHuman();
		AuSpecies speciesHuman = dnaHuman.Species();

		AuDNA dna2(dnaHuman);
		if(dnaHuman == dna2)
			qDebug() << "dnaHuman == dna2";
		dna2 = dnaHuman;
		if(dnaHuman.Species() == dna2.Species())
			qDebug() << "dnaHuman.Species == dna2.Species";
		AuDice dice;
		dice.TimeSeed();
		dna2.RandomizeSpecies(dice);
		dice.Roll(1,10);
		if(dnaHuman == dna2)
			qDebug() << "oops! dnaHuman.Species == dna2.Species";
		QList<AuDNA> uniqueDNA;
		uniqueDNA.append(dna2);
		int tries = 1000;
		int fails = 0;
		for(int i=0; i<tries; i++)
		{
			AuDNA dna3(dnaHuman);
			dna3.RandomizeSpecies(dice);
			dice.Roll(1,10);
			if(dna2 == dna3)
				qDebug() << "oops! dna2 == dna3";
			if(!uniqueDNA.contains(dna3))
				uniqueDNA.append(dna3);
			if(dnaHuman.Species() != dna3.Species())
			{
				qDebug() << (i+1) << "oops! dnaHuman.Species != dna3.Species randomized";
				fails++;
			}
			else
				qDebug() << (i+1) << "dnaHuman.Species == dna3.Species randomized";
			if((i+1)%100 == 0)
				qDebug() << (i+1) << "done";
		}
		if(fails)
			qDebug() << "Failed" << fails << "times, created" << uniqueDNA.size() << "uniqueDNA";
		else
			qDebug() << "Worked" << tries << "times, created" << uniqueDNA.size() << "uniqueDNA";
		return;
	}

	qDebug() << "sizeof DhDNA_" << sizeof(DhDNA_);
	return;
	quint8 logByte = ToLogByte(280, d_DNAMaxMM);
	qDebug() << "ToLogByte(0) = " << ToLogByte(0, d_DNAMaxMM);
	qDebug() << "ToLogByte(1) = " << ToLogByte(1, d_DNAMaxMM);
	qDebug() << "ToLogByte(2) = " << ToLogByte(2, d_DNAMaxMM);
	qDebug() << "ToLogByte(280) = " << ToLogByte(280, d_DNAMaxMM);
	qDebug() << "ToLogByte(500) = " << ToLogByte(500, d_DNAMaxMM);
	qDebug() << "ToLogByte(4999) = " << ToLogByte(4999, d_DNAMaxMM);
	qDebug() << "ToLogByte(5000) = " << ToLogByte(5000, d_DNAMaxMM);
	qDebug() << "FromLogByte(0) = " << FromLogByte(0, d_DNAMaxMM);
	qDebug() << "FromLogByte(1) = " << FromLogByte(1, d_DNAMaxMM);
	qDebug() << "FromLogByte(2) = " << FromLogByte(2, d_DNAMaxMM);
	qDebug() << "FromLogByte(" << (logByte-1) << ") = " << FromLogByte(logByte-1, d_DNAMaxMM);
	qDebug() << "FromLogByte(" << logByte << ") = " << FromLogByte(logByte, d_DNAMaxMM);
	qDebug() << "FromLogByte(" << (logByte+1) << ") = " << FromLogByte(logByte+1, d_DNAMaxMM);
	qDebug() << "FromLogByte(" << (d_LogByteRange-1) << ") = " << FromLogByte(d_LogByteRange-1, d_DNAMaxMM);
	qDebug() << "FromLogByte(" << d_LogByteRange << ") = " << FromLogByte(d_LogByteRange, d_DNAMaxMM);

		//double scale = 255. / log10(d_DNAMaxMM);
		//double exponent = logByte / scale;
		//quint16 val = (quint16)pow(10, exponent);
		//qDebug() << "fromLogByte" << logByte << "scale" << scale << "exp" << exponent << "val" << val;

	return;
	qDebug() << "sizeof unsigned int" << sizeof(unsigned int);
	qDebug() << "AuId(1234567890)" << AuId(1234567890).StringId();
	qDebug() << "StringLE26(1234567890)" << StringLE26(1234567890) << StringBE26(1234567890);
	return;

	// Test construction destruction of arrays.
	{
		qDebug() << "One single test class";
		TestClass a;
	}
	{
		qDebug() << "An array of 3 test classes";
		TestClass b[3];
	}

#if defined(Q_OS_WIN)
	quint8 osType = 1;
	quint16 osVer = QSysInfo::windowsVersion();
#elif defined(Q_OS_MACOS)
	quint8 osType = 2;
	quint16 osVer = QSysInfo::macVersion();
#else	// Q_OS_LINUX
	quint8 osType = 0;
	quint16 osVer = SEiLINUXPLATFORM;
#endif
	qDebug() << "osType" << osType << "osVer" << osVer;
	AuId id(1);
	unsigned int uid = 1;
	quint64 ui64 = 1;
	qDebug() << "AuId" << sizeof(id) << "uint" << sizeof(uid) << "uint64" << sizeof(ui64);

	// QVariant comparison tests.
	{
		QVariant vi(1);
		QVariant vis("1");
		QVariant vd(2.3);
		QVariant vds("2.3");
		QVariant vid = QVariant::fromValue(AuId(1));
		QVariant viv = QVariant::fromValue(AuId(1).GetID());
		qDebug() << vi << vis << (bool)(vi == vis);
		qDebug() << vd << vds << (bool)(vd == vds);
		qDebug() << vi << vd << (bool)(vi == vd);
		qDebug() << vis << vds << (bool)(vis == vds);
		qDebug() << vi << vds << (bool)(vi == vds);
		qDebug() << vd << vis << (bool)(vd == vis);
		qDebug() << vi << vid << (bool)(vi == vid);
		qDebug() << vi << viv << (bool)(vi == viv);
	}
	return;
	{
		// Calculate probability distribution for two dice.
		QMap<int, int> results;
		for(int d1=1; d1<=6; d1++)
		{
			for(int d2=1; d2<=6; d2++)
			{
				int roll = d1 + d2;
				QMap<int, int>::iterator ri = results.find(roll);
				if(ri != results.end())
					(*ri)++;
				else
					results.insert(roll, 1);
			}
		}
		qDebug() << "Dice Probability Result";
		for(QMap<int, int>::const_iterator ri = results.cbegin(); ri != results.cend(); ri++)
		{
			qDebug() << ri.key() << "chances" << *ri;
		}
		return;
	}

	{
		// Find mfg procs that do not consume anything.
		const DgData& gd = g_Au.LockDataForRead();
		for(DbManufacturingProcessArray::const_iterator mpi = gd.m_bManufacturingProcessArray.cbegin();
				mpi != gd.m_bManufacturingProcessArray.cend(); mpi++)
		{
			bool anyConsumed = false;
			if(DbCommodity::IsNaturalResource((*mpi).CommodityId()))
				continue;

			for(DbManufacturingComponentArray::const_iterator mcit = gd.m_bManufacturingComponentArray.cbegin();
					mcit != gd.m_bManufacturingComponentArray.cend(); mcit++)
			{
				if((*mcit).m_idmp != (*mpi).m_id)
					continue;
				if(!(*mcit).IsRequired())
					continue;
				if((*mcit).CommodityId() == CID_Money || (*mcit).CommodityId() == CID_Electricity)
					continue;
				if(!(*mcit).m_consumed && !AuZoneResource::IsFetchable((*mcit).CommodityId()))
					continue;
				anyConsumed = true;
				break;
			}
			if(!anyConsumed)
				qDebug() << (*mpi).m_id << (*mpi).m_name;
		}
		g_Au.UnlockData();
		return;
	}

	// Convert a pixel in a galaxy image to a galaxy coordinate in pc.
	QPoint gpx(342,255);
	AuPoint gloc = DaGalaxy::PixelLocation(HeartOfVictorus, gpx.x(), gpx.y());
	qDebug() << "gpx" << gpx << "in Heart of Victorus is gloc" << gloc;

	const double m2perperson = 1000;	// Square meters per person.
	// Area of a sphere.
	// A = 4 * pi * r^2
	for(int i=10; i<=120; i++)
	{
		double r = ::Radius100km(i) / FeetPerMeter();
		double area = 4 * PI * r * r;
		double pop = round(area / m2perperson);
		quint32 bld = AuCityReport::WorldBuildingLimit((quint32)pop, false);
		QString r2 = QString("%L1").arg(r*2, 0,'f',0);
		QString a = QString("%L1").arg(area, 0,'f',0);
		QString p = QString("%L1").arg(pop, 0,'f',0);
		QString b = QString("%L1").arg(bld);
		cout << "Sphere " << i << ", diameter " << r2.toLocal8Bit().data() << "m, area " << a.toLocal8Bit().data()
			<< "m" << AuGlobal::SquaredSymbol().toLocal8Bit().data() << " = max " << p.toLocal8Bit().data() << " population"
			<< " and " << b.toLocal8Bit().data() << "m" << AuGlobal::CubedSymbol().toLocal8Bit().data() << " buildings"
			<< endl;
	}

	for(int s=1; s <= 4; s++)
	{
		double sections = s * 5;
		double rwy = RadiusStandard() * PI;	// wall to wall distance in feet
		double sang = (PI*2) / sections;	// section angle
		double orad = .25 * s * FeetPerAu();	// orbit radius in feet
		double rwx = orad * sang;
		rwx /= FeetPerMeter();
		rwy /= FeetPerMeter();
		double rwa = rwx * rwy;
		double pop = round(rwa / m2perperson);
		quint32 bld = AuCityReport::WorldBuildingLimit((quint32)pop, true);
		QString rx = QString("%L1").arg(rwx, 0,'f',0);
		QString ry = QString("%L1").arg(rwy, 0,'f',0);
		QString ra = QString("%L1").arg(rwa, 0,'f',0);
		QString p = QString("%L1").arg(pop, 0,'f',0);
		QString b = QString("%L1").arg(bld);
		cout << "Ringworld with " << sections << " sections. Arc " << rx.toLocal8Bit().data() << " x "
			<< ry.toLocal8Bit().data() << "m, area " << ra.toLocal8Bit().data() << "m"
			<< AuGlobal::SquaredSymbol().toLocal8Bit().data() << " = max " << p.toLocal8Bit().data() << " population"
			<< " and " << b.toLocal8Bit().data() << "m" << AuGlobal::CubedSymbol().toLocal8Bit().data() << " buildings"
			<< endl;
	}

	/*
	const double m2perperson = 100000;	// Square meters per person.
	// Area of a sphere.
	// A = 4 * pi * r^2
	for(int i=1; i<=12; i++)
	{
		double r = DaWorldGlobe::Radius(i) / FeetPerMeter();
		double area = 4 * PI * r * r;
		double pop = area / m2perperson;
		double bld = 10 + i * 10;
		QString r2 = QString("%L1").arg(r*2, 0,'f',0);
		QString a = QString("%L1").arg(area, 0,'f',0);
		QString p = QString("%L1").arg(pop, 0,'f',0);
		QString b = QString("%L1").arg(bld, 0,'f',0);
		cout << "Sphere " << i << ", diameter " << r2.toLocal8Bit().data() << "m, area " << a.toLocal8Bit().data()
			<< "m" << AuGlobal::SquaredSymbol().toLocal8Bit().data() << " = " << p.toLocal8Bit().data() << " max population, "
			<< b.toLocal8Bit().data() << " buildings" << endl;
	}

	for(int s=1; s <= 4; s++)
	{
		double sections = s * 5;
		double rwy = DaWorldGlobe::Radius(6) * PI;	// wall to wall distance in feet
		double sang = (PI*2) / sections;	// section angle
		double orad = .25 * s * FeetPerAu();	// orbit radius in feet
		double rwx = orad * sang;
		rwx /= FeetPerMeter();
		rwy /= FeetPerMeter();
		double rwa = rwx * rwy;
		double pop = rwa / m2perperson;
		double bld = 140;
		QString rx = QString("%L1").arg(rwx, 0,'f',0);
		QString ry = QString("%L1").arg(rwy, 0,'f',0);
		QString ra = QString("%L1").arg(rwa, 0,'f',0);
		QString p = QString("%L1").arg(pop, 0,'f',0);
		QString b = QString("%L1").arg(bld, 0,'f',0);
		cout << "Ringworld with " << sections << " sections. Arc " << rx.toLocal8Bit().data() << " x "
			<< ry.toLocal8Bit().data() << "m, area " << ra.toLocal8Bit().data() << "m"
			<< AuGlobal::SquaredSymbol().toLocal8Bit().data() << " = " << p.toLocal8Bit().data() << " max population, "
			<< b.toLocal8Bit().data() << " buildings" << endl;
	}
	*/

	/*
	for(int i=1; i<=12; i++)
	{
		double r = DaWorldGlobe::Radius(i) / FeetPerMeter();
		double pop = i * 1000000;
		double bld = 200;
		QString r2 = QString("%L1").arg(r*2, 0,'f',0);
		QString p = QString("%L1").arg(pop, 0,'f',0);
		QString b = QString("%L1").arg(bld, 0,'f',0);
		cout << "Sphere " << i << ", diameter " << r2.toLocal8Bit().data() << "m = "
			<< p.toLocal8Bit().data() << " population, "
			<< b.toLocal8Bit().data() << " buildings "
			<< endl;
	}
	cout << "Ringworld arc section = 1,500,000 population, 200 buildings" << endl;
	*/
}

void AuEmpireReport::FixDb(QSqlDatabase& dbUniverse)
{
	Q_UNUSED(dbUniverse);
	//FindBrokenHenges(dbUniverse);
	//FixItems(dbUniverse);
	//FixCompression(dbUniverse);
	//FixGeoFeatures(dbUniverse);
	//FixPirateDNA(dbUniverse);
	//FixCityItemStacks(dbUniverse);
	//ReadShipCargo(dbUniverse);
	//ReadCityReportInventory(dbUniverse);
	//ReadCityReportFile();
	//FixSpawnCitySystems(dbUniverse);
	//QueryWorldBds(dbUniverse);
	//FixConditions(dbUniverse);
	/*
	FixNpcPassengers(dbUniverse);
	FixNpcOfficers(dbUniverse);
	FixNpcCrew(dbUniverse);
	FixNpcTroops(dbUniverse);
	FixNpcWorkers(dbUniverse);
	FixNpcIndigenous(dbUniverse);
	FixNpcCharacters(dbUniverse);
	*/
}

void AuEmpireReport::FindBrokenHenges(QSqlDatabase& dbUniverse)
{
	qDebug() << "Loading world to find broken henges.";

	int howMany = 0;
	int fixed = 0;
	QString sq = QString("SELECT id,name,geoFeatures FROM a_WorldLL WHERE id=2539280;");	// 2539280 = Bastion Arc 1
	//QString sq = QString("SELECT id,name,geoFeatures FROM a_WorldLL WHERE id=2539281;");	// 2539280 = Bastion Arc 2
	//QString sq = QString("SELECT id,name,geoFeatures FROM a_WorldLL WHERE id=2539282;");	// 2539280 = Bastion Arc 3
	//QString sq = QString("SELECT id,name,geoFeatures FROM a_WorldLL WHERE id=2539283;");	// 2539280 = Bastion Arc 4
	//QString sq = QString("SELECT id,name,geoFeatures FROM a_WorldLL WHERE id=2539284;");	// 2539280 = Bastion Arc 5
	//QString sq = QString("SELECT id,name,geoFeatures FROM a_WorldLL WHERE id=190788365;");	// 190788365 = KHY'ZM'DB'a Beta II
	//QString sq = QString("SELECT id,name,geoFeatures FROM a_WorldLL WHERE id=197503168;");	// = QHY'XM'VB'a Alpha I
	//QString sq = QString("SELECT id,name,geoFeatures FROM a_WorldLL WHERE id=964802;");	// = Web VI
	QSqlQuery qa(QString(), dbUniverse);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		howMany++;
		if(howMany%100 == 0)
			qDebug(" %d arc sections", howMany);

		int qv=0;
		AuId id = AuId(qa.value(qv++).toUInt());
		QString name(qa.value(qv++).toString());
		qDebug() << "Loading geological features for world" << id << name;

		int qvi=qv++;
		if(!qa.value(qvi).isNull())
		{
			// Load geological features.
			QByteArray ba = qa.value(qvi).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_4_1);

			AuGeologicalFeatures geof;
			in >> geof;

			qDebug() << "world" << id << name << "has" << geof.count() << "geological features";
			for(AuGeologicalFeatures::iterator gi = geof.begin(); gi != geof.end(); gi++)
			{
				AuGeoHenge* henge = dynamic_cast<AuGeoHenge*>(*gi);
				if(henge)
				{
					// Fix the henge.
					//*henge = AuGeoHenge(AuId(514015), 1, henge->DestShadowPanels(), 0, henge->CentralSlabAltitudeMSL(), henge->LL());

					qDebug() << " " << henge->Name() << (henge->IsActive()?"active":"inactive") << "panels" << henge->CentralSlabCount()
						<< "destSys" << henge->DestSysId() << "destSun" << henge->DestSun() << "destArc" << henge->DestArcSegment();
				}
				else
					qDebug() << " " << (*gi)->Name();
			}


			continue;	// No fix
			//geof.removeLast();
			//geof.removeLast();

			// Write the features without that last henge.
			QString squpd;
			squpd = QString("UPDATE a_WorldLL SET geoFeatures=? WHERE id=%1;").arg(id.GetID());
			QSqlQuery qupd(QString(), dbUniverse);
			qupd.prepare(squpd);
			{
				QByteArray ba;
				QDataStream out(&ba, QIODevice::WriteOnly);
				out.setVersion(QDataStream::Qt_4_1);
				out << geof;
				qupd.addBindValue(ba);
			}
			if(qupd.exec())
				fixed++;
			else
			{
				QSqlError err = qupd.lastError();
				qWarning() << "FixDb: world" << id << "geo features update failed" << squpd << err;
			}
		}
	}
	qDebug() << " " << howMany << "worlds loaded," << fixed << "fixed";
}

/*
void FixGovernanceDomains(QSqlDatabase& dbUniverse)
{
	// Fix governance domains.
	// At this point, we expect the DgData version of the empire records to be loaded, ignoring/skipping the old office data.
	qDebug() << "Fixing governance domains.";
	QHash<AuId, AuOldOffice> oldOffices;
	QSet<quint32> empireIds;
	QString sq;
	sq = QString("SELECT id,allegiance,office,governs FROM a_Person;");
	QSqlQuery qry(QString(), dbUniverse);
	qry.prepare(sq);
	qry.exec();
	while(qry.next())
	{
		AuOldOffice oo;

		int qv=0;
		AuId personId = AuId(qry.value(qv++).toUInt());
		oo.empireId = qry.value(qv++).toUInt();
		oo.office = (AuOldOffice::OFFICE)qry.value(qv++).toInt();
		if(oo.office == AuOldOffice::Citizen)
			continue;

		// Governed Atoms
		int qvi = qv++;
		if(!qry.value(qvi).isNull())
		{
			// The blob field is not null. It must be valid, even if it's a Citizen's empty hash table of governed info.
			QByteArray bcomp = qry.value(qvi).toByteArray();
			if(!bcomp.isEmpty())
			{
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,"");
				QDataStream ds(&ba, QIODevice::ReadOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				quint16 count;
				ds >> count;
				AuId id;
				QString name;
				for(;count ;count--)
				{
					ds >> id;
					ds >> name;
					oo.governs.insert(id, name);
				}
			}
		}

		oldOffices.insert(personId, oo);
		empireIds.insert(oo.empireId);
	}
	qDebug() << "Loaded" << oldOffices.count() << "old offices in" << empireIds.count() << "empires.";

	// Fix them one empire at a time.
	int updated = 0;
	DgData& gd = g_Au.LockDataForWrite();
	for(QSet<quint32>::const_iterator ei = empireIds.cbegin(); ei != empireIds.cend(); ei++)
	{
		DgEmpireHash::iterator empi = gd.m_gEmpires.find(*ei);
		if(empi == gd.m_gEmpires.end())
			continue;

		// Start at the emperor and work our way down.
		AuId boss;
		bool anyChanged = false;
		for(int office = AuOldOffice::President; office < AuOldOffice::Citizen; office++)
		{
			// Locate all the people in the empire that hold the office.
			AuId newboss;
			for(QHash<AuId, AuOldOffice>::const_iterator oi = oldOffices.cbegin(); oi != oldOffices.cend(); oi++)
			{
				if((*oi).empireId != (*empi).second.m_id)
					continue;	// Wrong empire.
				if((*oi).office != (AuOldOffice::OFFICE)office)
					continue;	// Wrong office.

				AuOffice office;
				switch((*oi).office)
				{
				case AuOldOffice::President:
					if(!(*empi).second.EmperorId().IsNull())
						continue;	// Only one emperor allowed.
					office.SetupEmperor();
					qDebug() << " " << (*empi).second.CitizenName(oi.key()) << "Emperor" << (*empi).second.Name();
					break;
				case AuOldOffice::VicePresident:
					if(boss.IsNull())
					{
						// Sets up as emperor if the current emperor id is null.
						office.SetupEmperor();
						boss = oi.key();	// In case there are more.
						qDebug() << " " << (*empi).second.CitizenName(oi.key()) << "changed Vice Emperor to Emperor"
							<< (*empi).second.Name();
					}
					else
					{
						if((*oi).governs.isEmpty())
							continue;
						office.SetupVP(boss, (*oi).governs);
						qDebug() << " " << (*empi).second.CitizenName(oi.key()) << "Vice Emperor" << *(*oi).governs.begin()
							<< "in" << (*empi).second.Name();
					}
					break;
				case AuOldOffice::Senator:
					if((*oi).governs.isEmpty())
						continue;
					office.SetupSenator(boss, (*oi).governs);
					qDebug() << " " << (*empi).second.CitizenName(oi.key()) << "Senator" << *(*oi).governs.begin()
						<< "in" << (*empi).second.Name();
					break;
				case AuOldOffice::Congressman:
					if((*oi).governs.isEmpty())
						continue;
					office.SetupCongressman(boss, (*oi).governs);
					qDebug() << " " << (*empi).second.CitizenName(oi.key()) << "Congressman" << *(*oi).governs.begin()
						<< "in" << (*empi).second.Name();
					break;
				case AuOldOffice::Governor:
					if((*oi).governs.isEmpty())
						continue;
					office.SetupGovernor(boss, (*oi).governs);
					qDebug() << " " << (*empi).second.CitizenName(oi.key()) << "Governor" << *(*oi).governs.begin()
						<< "in" << (*empi).second.Name();
					break;
				case AuOldOffice::Mayor:
					if((*oi).governs.isEmpty())
						continue;
					office.SetupMayor(boss, (*oi).governs);
					qDebug() << " " << (*empi).second.CitizenName(oi.key()) << "Mayor" << *(*oi).governs.begin()
						<< "in" << (*empi).second.Name();
					break;
				case AuOldOffice::Citizen:
					continue;
				}
				(*empi).second.CitizenAssignOffice(oi.key(), office);
				anyChanged = true;

				newboss = oi.key();
			}

			// Shuffle boss for next pass.
			if(!newboss.IsNull())
				boss = newboss;
		}

		if(anyChanged)
		{
			(*empi).second.Update(dbUniverse);
			updated++;
		}
	}
	g_Au.UnlockData();

	qDebug() << "Updated" << updated << "empire records.";
}
*/

void AuEmpireReport::ReportTroops(QSqlDatabase& dbUniverse)
{
	int troopCount = 0;

	qDebug() << "Looking for troops...";

	// Locate all troops in the database.
	QHash<AuId, int> troopParents;	// Key is parent atom, value is count of troops there.
	QString sq;
	sq = QString("SELECT id,idp FROM a_Npc WHERE role=%1;").arg(ROLE_Troop);
	QSqlQuery qry(QString(), dbUniverse);
	qry.prepare(sq);
	qry.exec();
	while(qry.next())
	{
		int qvi=0;		// query value index
		AuId atomId = AuId(qry.value(qvi++).toUInt());
		AuId parentId = AuId(qry.value(qvi++).toUInt());

		// Tally troops per parent.
		QHash<AuId, int>::iterator pi = troopParents.find(parentId);
		if(pi == troopParents.end())
			troopParents.insert(parentId, 1);
		else
			(*pi)++;

		troopCount++;
		if(troopCount % 1000 == 0)
			qDebug() << "Read" << troopCount << "troops";
	}

	qDebug() << troopCount << "total troops.";

	// Report troop concentrations.
	for(QHash<AuId, int>::const_iterator pi = troopParents.cbegin(); pi != troopParents.cend(); pi++)
	{
		if(*pi <= 50)
			continue;

		qDebug() << "Atom hosts" << *pi << "troops";
		ReportAtom(dbUniverse, pi.key(), false, false, "  ", d_PrefixStep);
	}
}

void AuEmpireReport::ReportManufacturingProcesses(QSqlDatabase& dbUniverse)
{
	Q_UNUSED(dbUniverse);
	QFile sofile;
	sofile.open(stdout, QIODevice::WriteOnly);
	QTextStream so(&sofile);
	so.setCodec("UTF-8");

	const DgData& gd = g_Au.LockDataForRead();
	for(DbIndustryArray::const_iterator ii = gd.m_bIndustryArray.cbegin(); ii != gd.m_bIndustryArray.cend(); ii++)
	{
		if((*ii).m_name.isEmpty())
			continue;

		so << "{{Production\n";
		so << "| Building = [[" << (*ii).m_name << "]]\n";

		// List manufacturing processes.

		so << "}}\n";
	}
	g_Au.UnlockData();

	sofile.close();
}

void AuEmpireReport::ReportCommoditiesWiki()
{
	QFile sofile;
	sofile.open(stdout, QIODevice::WriteOnly);
	QTextStream so(&sofile);
	so.setCodec("UTF-8");

	so << "{| class=\"wikitable\"\n";
	so << "|+\n";
	so << "| <b>Image</b> || <b>Name</b> || <b>Base Cost</b> || <b>Weight</b> || <b>Pack Size</b> || <b>Capacity</b>\n";

	QMap<QString, COMMODITYID> alphabetized;
	for(DbCommodityArray::const_iterator it = g_Au.m_CommodityArray.cbegin(); it != g_Au.m_CommodityArray.cend(); ++it)
		alphabetized.insert((*it).m_name.toUpper(), (*it).m_id);

	for(QMap<QString, COMMODITYID>::const_iterator ai = alphabetized.cbegin(); ai != alphabetized.cend(); ai++)
	{
		const DbCommodity& com = g_Au.m_CommodityArray.Commodity(*ai);
		if(com.m_id != CID_Invalid)
		{
			QString iconName = com.BitmapName();
			iconName.replace(":data/c_", "C ");

			QString industry = com.m_name;
			industry.replace(" Technology", "]] [[Technology");

			QString cost = QString("%L1%2").arg(com.BaseCost(), 0, 'f', 2).arg(g_Au.CurrencySymbol());

			QString wt = QString("%L1kg").arg(com.m_wt * KilogramsPerPound(), 0, 'f', 2);

			so << "|-\n";
			if(com.m_capacity.width() > 0 && com.m_capacity.height() > 0)
				so << "| [[File:" << iconName << "|32px|center]]"
					<< " || [[" << industry << "]]"
					<< " || " << cost
					<< " || " << wt
					<< " || " << com.m_size.width() << " x " << com.m_size.height()
					<< " || " << com.m_capacity.width() << " x " << com.m_capacity.height()
					<< "\n";
			else if(com.m_size.width() > 0 && com.m_size.height() > 0)
				so << "| [[File:" << iconName << "|32px|center]]"
					<< " || [[" << industry << "]]"
					<< " || " << cost
					<< " || " << wt
					<< " || " << com.m_size.width() << " x " << com.m_size.height()
					<< " || "
					<< "\n";
			else
				so << "| [[File:" << iconName << "|32px|center]]"
					<< " || [[" << industry << "]]"
					<< " || " << cost
					<< " || " << wt
					<< " || n/a "
					<< " || "
					<< "\n";
		}
	}

	so << "|}\n";

	sofile.close();
}

void AuEmpireReport::ReportIndustries()
{
	QFile sofile;
	sofile.open(stdout, QIODevice::WriteOnly);
	QTextStream so(&sofile);
	so.setCodec("UTF-8");

	const DgData& gd = g_Au.LockDataForRead();
	so << "Industries";
	for(DbIndustryArray::const_iterator it = gd.m_bIndustryArray.cbegin(); it != gd.m_bIndustryArray.cend(); it++)
	{
		if(!(*it).m_id)
			continue;
		so << (*it).m_id << " " << (*it).m_name << "\n";
	}
	g_Au.UnlockData();

	sofile.close();
}

void AuEmpireReport::ReportIndustriesWiki()
{
	QFile sofile;
	sofile.open(stdout, QIODevice::WriteOnly);
	QTextStream so(&sofile);
	so.setCodec("UTF-8");

	const DgData& gd = g_Au.LockDataForRead();

	QMap<QString, INDUSTRYID> alphabetized;
	for(DbIndustryArray::const_iterator it = gd.m_bIndustryArray.cbegin(); it != gd.m_bIndustryArray.cend(); it++)
		if((*it).m_id)
			alphabetized.insert((*it).m_name.toUpper(), (INDUSTRYID)(*it).m_id);

	for(QMap<QString, INDUSTRYID>::const_iterator ai = alphabetized.cbegin(); ai != alphabetized.cend(); ai++)
	{
		const DbIndustry& dbi = gd.Industry(*ai);
		if(!dbi.m_id)
			continue;
		if(dbi.DoesWhat().isEmpty())
			continue;	// Old style building only?

		QString iconName = dbi.BitmapName();
		iconName.replace(":data/b", "B");

		QString tools;
		QString modifiers;

		so << "{{Building\n";
		so << "| Name = " << dbi.m_name << "\n";
		so << "| Picture = " << iconName << "\n";
		so << "| Placement = " << dbi.BuildRuleText() << "\n";
		so << "| Description = " << dbi.DoesWhat() << "\n";
		so << "| Note = " << dbi.Note() << "\n";
		so << QString(
				"| Type = %29\n"
				"| Arena = %3\n"
				"| Auditorium = %4\n"
				"| Battery = %5\n"
				"| CommodityStorage = %6\n"
				"| FieldArea = %1\n"
				"| Generator = %8\n"
				"| GuardPost = %9\n"
				"| HomeLarge = %10\n"
				"| HomeMedium = %30\n"
				"| HomeSmall = %31\n"
				"| Livestock = %11\n"
				"| LoungeSpace = %12\n"
				"| OfficeSpace = %13\n"
				"| ParkingGroundVehicles = %14\n"
				"| ParkingSpaceVehicles = %15\n"
				"| ParkingWaterVehicles = %16\n"
				"| ParkingSpacecraft = %32\n"
				"| Radar = %18\n"
				"| ShieldGenerator = %19\n"
				"| ShopSpace = %20\n"
				"| SpaceVehicleLaunchers = %24\n"
				"| SpaceVehicleRecoverySystems = %25\n"
				"| StoreSpace = %17\n"
				"| SurgeryUnits = %21\n"
				"| Transporters = %22\n"
				"| Turrets = %23\n"
				"| WeaponBays = %26\n"
				"| Materials =\n"
				"* [[Material of Choice]]\n"
				"}}\n"
				)
			.arg(dbi.Field() > 0 ? QString("%1 Field Required").arg(dbi.Field()) :
					(dbi.Field() == 0 ? QString("Ok") : QString("No")))
			.arg(dbi.Arena() > 0 ? QString("%1 Arena Required").arg(dbi.Arena()) :
					(dbi.Arena() == 0 ? QString("Ok") : QString("No")))
			.arg(dbi.Auditorium() > 0 ? QString("%1 Auditorium Required").arg(dbi.Auditorium()) :
					(dbi.Auditorium() == 0 ? QString("Ok") : QString("No")))
			.arg(dbi.Battery() > 0 ? QString("%1m%2 Batteries Required").arg(dbi.Battery()).arg(g_Au.CubedSymbol()) :
					(dbi.Battery() == 0 ? QString("Ok") : QString("No")))
			.arg(dbi.CargoStorage() > 0 ? QString("%1m%2 Space Required").arg(dbi.CargoStorage()).arg(g_Au.CubedSymbol()) :
					(dbi.CargoStorage() == 0 ? QString("Ok") :
					 (dbi.CargoStorage() == -2 ? QString("Limited to 10%2m").arg(g_Au.CubedSymbol()) : QString("No"))))
			.arg(dbi.Generator() > 0 ? QString("%1 Generator Required").arg(dbi.Generator()) :
					(dbi.Generator() == 0 ? QString("Ok") : QString("No")))
			.arg(dbi.GuardPost() > 0 ? QString("%1 Post Required").arg(dbi.GuardPost()) :
					(dbi.GuardPost() == 0 ? QString("Ok") : QString("No")))
			.arg(dbi.HomeLarge() > 0 ? QString("%1 Large Home Required").arg(dbi.HomeLarge()) :
					(dbi.HomeLarge() == 0 ? QString("Ok") : QString("No")))
			.arg(dbi.LivestockPost() > 0 ? QString("%1 Post Required").arg(dbi.LivestockPost()) :
					(dbi.LivestockPost() == 0 ? QString("Ok") : QString("No")))
			.arg(dbi.Lounge() > 0 ? QString("%1 Lounge Required").arg(dbi.Lounge()) :
					(dbi.Lounge() == 0 ? QString("Ok") : QString("No")))
			.arg(dbi.Office() > 0 ? QString("%1 Office Required").arg(dbi.Office()) :
					(dbi.Office() == 0 ? QString("Ok") : QString("No")))
			.arg(dbi.ParkGround() > 0 ? QString("%1 Spot Required").arg(dbi.ParkGround()) :
					(dbi.ParkGround() == 0 ? QString("Ok") : QString("No")))
			.arg(dbi.ParkSpace() > 0 ? QString("%1 Spot Required").arg(dbi.ParkSpace()) :
					(dbi.ParkSpace() == 0 ? QString("Ok") : QString("No")))
			.arg(dbi.ParkWater() > 0 ? QString("%1 Spot Required").arg(dbi.ParkWater()) :
					(dbi.ParkWater() == 0 ? QString("Ok") : QString("No")))
			.arg(dbi.Store() > 0 ? QString("%1 Store Required").arg(dbi.Store()) :
					(dbi.Store() == 0 ? QString("Ok") : QString("No")))
			.arg(dbi.Sensor() > 0 ? QString("%1m%2 Radar Required").arg(dbi.Sensor()).arg(g_Au.CubedSymbol()) :
					(dbi.Sensor() == 0 ? QString("Ok") : QString("No")))
			.arg(dbi.Shield() > 0 ? QString("%1m%2 Shields Required").arg(dbi.Shield()).arg(g_Au.CubedSymbol()) :
					(dbi.Shield() == 0 ? QString("Ok") : QString("No")))
			.arg(dbi.Shop() > 0 ? QString("%1 Workshop Required").arg(dbi.Shop()) :
					(dbi.Shop() == 0 ? QString("Ok") : QString("No")))
			.arg(dbi.Surgery() > 0 ? QString("%1 Unit Required").arg(dbi.Surgery()) :
					(dbi.Surgery() == 0 ? QString("Ok") : QString("No")))
			.arg(dbi.Transporter() > 0 ? QString("%1 Transporter Required").arg(dbi.Transporter()) :
					(dbi.Transporter() == 0 ? QString("Ok") : QString("No")))
			.arg(dbi.Turret() > 0 ? QString("%1 Turret Required").arg(dbi.Turret()) :
					(dbi.Turret() == 0 ? QString("Ok") : QString("No")))
			.arg(dbi.VehicleLauncher() > 0 ? QString("%1 Launcher Required").arg(dbi.VehicleLauncher()) :
					(dbi.VehicleLauncher() == 0 ? QString("Ok") : QString("No")))
			.arg(dbi.VehicleRecovery() > 0 ? QString("%1 Recovery System Required").arg(dbi.VehicleRecovery()) :
					(dbi.VehicleRecovery() == 0 ? QString("Ok") : QString("No")))
			.arg(dbi.WeaponBay() > 0 ? QString("%1 Bay Required").arg(dbi.WeaponBay()) :
					(dbi.WeaponBay() == 0 ? QString("Ok") : QString("No")))
			.arg(BuildingStructureName(dbi.Structure()))
			.arg(dbi.HomeMedium() > 0 ? QString("%1 Medium or Large Home Required").arg(dbi.HomeMedium()) :
					(dbi.HomeMedium() == 0 ? QString("Ok") : QString("No")))
			.arg(dbi.HomeSmall() > 0 ? QString("%1 Home of Any Size Required").arg(dbi.HomeSmall()) :
					(dbi.HomeSmall() == 0 ? QString("Ok") : QString("No")))
			.arg(dbi.ParkSpacecraft() > 0 ? QString("%1 Spot Required").arg(dbi.ParkSpacecraft()) :
					(dbi.ParkSpacecraft() == 0 ? QString("Ok") : QString("No")));
		so << "\n";
	}
	g_Au.UnlockData();

	sofile.close();
}

void AuEmpireReport::ReportDesign(QSqlDatabase& db, quint32 idDesign)
{
	// Try n_ScDesign
	{
		QString sq = QString("SELECT name,ida,archname,state,streamline,structure,techLevelMin,hullVolume "
				"FROM n_ScDesign WHERE id=%1;").arg(idDesign);
		QSqlQuery qry(QString(), db);
		qry.prepare(sq);
		qry.exec();
		if(qry.next())
		{
			int qv=0;
			QString name = qry.value(qv++).toString();
			AuId archid = AuId(qry.value(qv++).toUInt());
			QString archname = qry.value(qv++).toString();
			int state = qry.value(qv++).toUInt();
			STREAMLINE streamline = (STREAMLINE)qry.value(qv++).toInt();
			SPACECRAFT_STRUCTURE structure = (SPACECRAFT_STRUCTURE)qry.value(qv++).toInt();
			int tlmin = qry.value(qv++).toInt();
			quint32 hullvol = qry.value(qv++).toUInt();
			qDebug() << "Sc" << idDesign << name
				<< "arch" << archid << archname
				<< "state" << AuDesign::StateName((AuDesign::STATE)state)
				<< "streamlined" << StreamlineName(streamline)
				<< "structure" << SpacecraftStructureName(structure)
				<< "minQ" << tlmin
				<< "hullVol" << hullvol << "cu. meters";
			return;
		}
	}
	// Failed to load n_ScDesign.
}

void AuEmpireReport::ReportDesignMedia(QSqlDatabase& db, quint32 idDesign)
{
	QString sq = QString("SELECT id,name,inv,seachest FROM a_Person;");
	QSqlQuery qry(QString(), db);
	qry.prepare(sq);
	qry.exec();
	while(qry.next())
	{
		int qv=0;
		AuId id = AuId(qry.value(qv++).toUInt());
		QString name = qry.value(qv++).toString();

		int qvi = qv++;
		if(!qry.value(qvi).isNull())
		{
			AuGear gear;
			QByteArray ba = qry.value(qvi).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_4_1);
			in >> gear;

			AuGearPosList gearPos;
			gear.ListAll(gearPos);
			for(AuGearPosList::const_iterator gi = gearPos.cbegin(); gi != gearPos.cend(); gi++)
			{
				const AuItem* item = gear.ItemAt(*gi);
				if(!item)
				{
					//qDebug() << " Gear pos" << *gi << "item not found";
					continue;
				}
				//else
					//qDebug() << " Gear pos" << *gi << "item" << item->ListDesc();
				if(item->DesignId() != idDesign)
					continue;
				qDebug() << id << name << "gear" << *gi << "item" << item->ListDesc();
			}
		}

		qvi = qv++;
		if(!qry.value(qvi).isNull())
		{
			QByteArray ba = qry.value(qvi).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_4_1);
			for(int galaxy=0; galaxy<GALAXY_Count; galaxy++)
			{
				AuItemHash seaChest;
				in >> seaChest;
				for(AuItemHash::const_iterator si = seaChest.cbegin(); si != seaChest.cend(); si++)
				{
					if((*si).second->DesignId() != idDesign)
						continue;
					qDebug() << id << name << "sea chest" << galaxy << "item" << (*si).second->ListDesc();
				}
			}
		}
	}
}

void AuEmpireReport::ReportItemPossession(QSqlDatabase& db, COMMODITYID cid)
{
	QString sq = QString("SELECT id,name,inv,seachest FROM a_Person;");
	QSqlQuery qry(QString(), db);
	qry.prepare(sq);
	qry.exec();
	while(qry.next())
	{
		int qv=0;
		AuId id = AuId(qry.value(qv++).toUInt());
		QString name = qry.value(qv++).toString();

		int qvi = qv++;
		if(!qry.value(qvi).isNull())
		{
			AuGear gear;
			QByteArray ba = qry.value(qvi).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_4_1);
			in >> gear;

			AuGearPosList gearPos;
			gear.ListAll(gearPos);
			for(AuGearPosList::const_iterator gi = gearPos.cbegin(); gi != gearPos.cend(); gi++)
			{
				const AuItem* item = gear.ItemAt(*gi);
				if(!item)
				{
					//qDebug() << " Gear pos" << *gi << "item not found";
					continue;
				}
				//else
					//qDebug() << " Gear pos" << *gi << "item" << item->ListDesc();
				if(item->CommodityId() != cid)
					continue;
				qDebug() << id << name << "gear" << *gi << "item" << item->ListDesc();
			}
		}

		qvi = qv++;
		if(!qry.value(qvi).isNull())
		{
			QByteArray ba = qry.value(qvi).toByteArray();
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_4_1);
			for(int galaxy=0; galaxy<GALAXY_Count; galaxy++)
			{
				AuItemHash seaChest;
				in >> seaChest;
				for(AuItemHash::const_iterator si = seaChest.cbegin(); si != seaChest.cend(); si++)
				{
					if((*si).second->CommodityId() != cid)
						continue;
					qDebug() << id << name << "sea chest" << galaxy << "item" << (*si).second->ListDesc();
				}
			}
		}
	}
}

void AuEmpireReport::SaveEmpire(quint32 empireId)
{
	qDebug() << "AuEmpireReport::SaveEmpire empireId" << empireId;
	// Find the empire record.
	const DgData& gd = g_Au.LockDataForRead();
	DgEmpireHash::const_iterator ehi = gd.m_gEmpires.find(empireId);
	if(ehi != gd.m_gEmpires.cend())
	{
		// Save the empire record to a file.
		QString fileName = QString("Empire%1.emp").arg(empireId);
		QFile file(fileName);
		if(file.open(QFile::WriteOnly))
		{
			QDataStream ds(&file);
			ds.setVersion(QDataStream::Qt_5_8);
			(*ehi).second >> ds;
			qDebug() << "Empire" << empireId << "saved to file" << fileName;
		}
		else
			qWarning() << "Failed to create output file" << fileName;
	}
	else
		qWarning() << "Failed to find empire" << empireId << "in" << gd.m_gEmpires.size() << "empires";
	g_Au.UnlockData();
}

void AuEmpireReport::SaveItems(QSqlDatabase& dbUniverse, const AuId& atomId)
{
	qDebug() << "AuEmpireReport::SaveItems atomId" << atomId;
	QString fileName = QString("subitems%1.bin").arg(atomId.GetID());
	QFile file(fileName);
	if(!file.open(QFile::WriteOnly))
	{
		qWarning() << "Failed to create output file.";
		return;
	}
	QDataStream ds(&file);
	ds.setVersion(QDataStream::Qt_5_8);

	int countSaved = 0;
	QString sq = QString("SELECT id, locX, locY, locZ, rotX, rotY, rotZ, timeCreate, timeModified, timeTick, timeTock, timeDeath, "
			"atRest, vecX, vecY, vecZ, item, ownerId, ownerName, ownerEmpire, cid, staticDisplay "
			"FROM a_Item WHERE idp=%1;").arg(atomId.GetID());
	QSqlQuery qry(QString(), dbUniverse);
	qry.prepare(sq);
	qry.exec();
	while(qry.next())
	{
		int qvi=0;		// query value index
		AuId id = AuId(qry.value(qvi++).toUInt());
		double locX = qry.value(qvi++).toDouble();
		double locY = qry.value(qvi++).toDouble();
		double locZ = qry.value(qvi++).toDouble();
		double rotX = qry.value(qvi++).toDouble();
		double rotY = qry.value(qvi++).toDouble();
		double rotZ = qry.value(qvi++).toDouble();
		QDateTime created = qry.value(qvi++).toDateTime();
		QDateTime modified = qry.value(qvi++).toDateTime();
		QDateTime ticked = qry.value(qvi++).toDateTime();
		QDateTime tocked = qry.value(qvi++).toDateTime();
		QDateTime death = qry.value(qvi++).toDateTime();
		created.setTimeSpec(Qt::UTC);
		modified.setTimeSpec(Qt::UTC);
		ticked.setTimeSpec(Qt::UTC);
		tocked.setTimeSpec(Qt::UTC);
		death.setTimeSpec(Qt::UTC);
		bool atRest = qry.value(qvi++).toBool();
		double vecX = qry.value(qvi++).toDouble();
		double vecY = qry.value(qvi++).toDouble();
		double vecZ = qry.value(qvi++).toDouble();
		QByteArray item = qry.value(qvi++).toByteArray();
		AuId ownerId = AuId(qry.value(qvi++).toUInt());
		QString ownerName = qry.value(qvi++).toString();
		quint32 ownerEmpire = qry.value(qvi++).toUInt();
		int cid = qry.value(qvi++).toInt();
		bool staticDisplay = qry.value(qvi++).toBool();

		ds << id;
		ds << locX;
		ds << locY;
		ds << locZ;
		ds << rotX;
		ds << rotY;
		ds << rotZ;
		ds << created;
		ds << modified;
		ds << ticked;
		ds << tocked;
		ds << death;
		ds << atRest;
		ds << vecX;
		ds << vecY;
		ds << vecZ;
		ds << item;
		ds << ownerId;
		ds << ownerName;
		ds << ownerEmpire;
		ds << cid;
		ds << staticDisplay;
		countSaved++;
	}
	qDebug() << "AuEmpireReport::SaveItems" << countSaved << "items saved to file" << fileName;
}

void AuEmpireReport::RestoreItems(QSqlDatabase& dbUniverse, const AuId& atomId, const QString& fileName)
{
	QFile file(fileName);
	if(!file.open(QFile::ReadOnly))
	{
		qWarning() << "Failed to read input file" << fileName;
		return;
	}

	QDataStream ds(&file);
	ds.setVersion(QDataStream::Qt_5_8);

	int countRestored = 0;
	AuId id;
	do
	{
		double locX;
		double locY;
		double locZ;
		double rotX;
		double rotY;
		double rotZ;
		QDateTime created;
		QDateTime modified;
		QDateTime ticked;
		QDateTime tocked;
		QDateTime death;
		bool atRest;
		double vecX;
		double vecY;
		double vecZ;
		QByteArray item;
		AuId ownerId;
		QString ownerName;
		quint32 ownerEmpire;
		int cid;
		bool staticDisplay;

		ds >> id;
		ds >> locX;
		ds >> locY;
		ds >> locZ;
		ds >> rotX;
		ds >> rotY;
		ds >> rotZ;
		ds >> created;
		ds >> modified;
		ds >> ticked;
		ds >> tocked;
		ds >> death;
		ds >> atRest;
		ds >> vecX;
		ds >> vecY;
		ds >> vecZ;
		ds >> item;
		ds >> ownerId;
		ds >> ownerName;
		ds >> ownerEmpire;
		ds >> cid;
		ds >> staticDisplay;

		if(!id.IsNull())
		{
			// Check for existing atom with that id.
			QSqlQuery qsc(QString("SELECT idp FROM a_Atom WHERE id = %1;").arg(id.GetID()), dbUniverse);
			if(qsc.next())
			{
				AuId idp = AuId(qsc.value(0).toUInt());
				qDebug() << "Skipped: Atom" << id << "already exists subordinate to" << idp;
				continue;
			}

			// Insert the item into the database.
			QSqlQuery qa(QString(), dbUniverse);
			QString sq("INSERT INTO a_Item (id,idp,locX,locY,locZ,rotX,rotY,rotZ,"
						"timeCreate,timeModified,timeTick,timeTock,timeDeath, item,atRest,vecX,"
						"vecY,vecZ,cid,ownerId,ownerName,ownerEmpire,staticDisplay"
						") VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?, ?,?,?,?,?,?,?,?,?,?);");
			qa.prepare(sq);
			qa.addBindValue(id.GetID());
			qa.addBindValue(atomId.GetID());
			qa.addBindValue(locX);
			qa.addBindValue(locY);
			qa.addBindValue(locZ);
			qa.addBindValue(rotX);
			qa.addBindValue(rotY);
			qa.addBindValue(rotZ);
			qa.addBindValue(created);
			qa.addBindValue(modified);
			qa.addBindValue(ticked);
			qa.addBindValue(tocked);
			qa.addBindValue(death);
			qa.addBindValue(item);
			qa.addBindValue(atRest);
			qa.addBindValue(vecX);
			qa.addBindValue(vecY);
			qa.addBindValue(vecZ);
			qa.addBindValue(cid);
			qa.addBindValue(ownerId.GetID());
			qa.addBindValue(ownerName);
			qa.addBindValue(ownerEmpire);
			qa.addBindValue(staticDisplay);

			if(qa.exec())
				countRestored++;
			else
			{
				QSqlError err = qa.lastError();
				qWarning() << "Item insert failed" << sq << "error message" << err;
			}
		}
	}
	while(!id.IsNull());
	file.close();
	qDebug() << "AuEmpireReport::RestoreItems" << countRestored << "items restored to" << atomId << "from file" << fileName;
}

// Loads/saves all AuItem's in the database, except ammo items in turrets and bay weapons. They weren't needed when this code was
// added.
//
// To use this to update an item where its item type does not change:
//  1. Modify AuItem::FixItem to fix the item.
//  2. Run this function.
//
// To use this to change the type of an item. e.g. Computers changed from regular items to storage devices.
//  1. Modify AuItem::FixItem to replace the item.
//  2. Run this function.
//  3. Modify AuItem::ItemType to create the appropriate object when the item is read.
void AuEmpireReport::FixItems(QSqlDatabase& dbUniverse)
{
	bool readOnly = true;
	if(readOnly)
		qDebug() << "Reading items.";
	else
		qDebug() << "Fixing items.";

	// Sc cargos. Does not fix ammo.
	{
		int howMany = 0;
		QString sq = QString("SELECT id,name,cargo FROM a_Sc;");
		QSqlQuery qa(sq, dbUniverse);
		while(qa.next())
		{
			howMany++;
			if(howMany%100 == 0)
				qDebug(" %d Scs", howMany);

			int qv=0;
			AuId id = AuId(qa.value(qv++).toUInt());
			QString name = qa.value(qv++).toString();
			int qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Read/deserialize the blob.
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				AuItemHash cargo;
				in >> cargo;

				if(!readOnly)
				{
					cargo.FixItems();

					// Update the ship in the database.
					QString squp;
					squp = QString("UPDATE a_Sc SET cargo=? WHERE id=%1;")
						.arg(id.GetID());
					QSqlQuery qup(QString(), dbUniverse);
					qup.prepare(squp);
					{
						QByteArray ba;
						QDataStream out(&ba, QIODevice::WriteOnly);
						out.setVersion(QDataStream::Qt_4_1);
						out << cargo;
						qup.addBindValue(ba);
					}

					if(!qup.exec())
						qWarning() << "error: Update of Sc failed. " << id << name;
				}
			}
		}
		qDebug() << " " << howMany << "Scs Total";
	}

	// citizen gear.
	{
		int howMany = 0;
		QString sq = QString("SELECT id,name,inv FROM a_Citizen;");
		QSqlQuery qa(sq, dbUniverse);
		while(qa.next())
		{
			howMany++;
			if(howMany%100 == 0)
				qDebug(" %d Citizens", howMany);

			int qv=0;
			AuId id = AuId(qa.value(qv++).toUInt());
			QString name = qa.value(qv++).toString();
			int qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Read/deserialize the blob.
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				AuGear gear;
				in >> gear;

				if(!readOnly)
				{
					gear.FixItems();

					// Update the citizen in the database.
					QString squp;
					squp = QString("UPDATE a_Citizen SET inv=? WHERE id=%1;")
						.arg(id.GetID());
					QSqlQuery qup(QString(), dbUniverse);
					qup.prepare(squp);
					{
						QByteArray ba;
						QDataStream out(&ba, QIODevice::WriteOnly);
						out.setVersion(QDataStream::Qt_4_1);
						out << gear;
						qup.addBindValue(ba);
					}

					if(!qup.exec())
						qWarning() << "error: Update of Citizen failed. " << id << name;
				}
			}
		}
		qDebug() << " " << howMany << "Citizens Total";
	}

	// person gear.
	{
		int howMany = 0;
		QString sq = QString("SELECT id,name,inv,seaChest FROM a_Person;");
		QSqlQuery qa(sq, dbUniverse);
		while(qa.next())
		{
			howMany++;
			if(howMany%100 == 0)
				qDebug(" %d Avatars", howMany);

			int qv=0;
			AuId id = AuId(qa.value(qv++).toUInt());
			QString name = qa.value(qv++).toString();

			AuGear gear;
			AuGear arenaGear;
			int qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Read/deserialize the blob.
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				in >> gear;
				in >> arenaGear;
			}

			qvi = qv++;
			AuItemHash seaChest;
			if(!qa.value(qvi).isNull())
			{
				// Read/deserialize the blob.
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				in >> seaChest;
			}

			if(!readOnly)
			{
				gear.FixItems();
				arenaGear.FixItems();
				seaChest.FixItems();

				// Update the citizen in the database.
				QString squp;
				squp = QString("UPDATE a_Person SET inv=?, seaChest=? WHERE id=%1;")
					.arg(id.GetID());
				QSqlQuery qup(QString(), dbUniverse);
				qup.prepare(squp);
				{
					QByteArray ba;
					QDataStream out(&ba, QIODevice::WriteOnly);
					out.setVersion(QDataStream::Qt_4_1);
					out << gear;
					out << arenaGear;
					qup.addBindValue(ba);
				}
				{
					QByteArray ba;
					QDataStream out(&ba, QIODevice::WriteOnly);
					out.setVersion(QDataStream::Qt_4_1);
					out << seaChest;
					qup.addBindValue(ba);
				}
				if(!qup.exec())
					qWarning() << "error: Update of Avatar failed. " << id << name;
			}
		}
		qDebug() << " " << howMany << "Avatars Total";
	}

	// individual items.
	{
		int howMany = 0;
		QString sq = QString("SELECT id,item FROM a_Item;");
		QSqlQuery qa(sq, dbUniverse);
		while(qa.next())
		{
			howMany++;
			if(howMany%100 == 0)
				qDebug(" %d Items", howMany);

			int qv=0;
			AuId id = AuId(qa.value(qv++).toUInt());
			int qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Read/deserialize the blob.
				AuItem* item = AuItem::Unpack(qa.value(qvi).toByteArray());
				if(!readOnly)
				{
					if(!item)
					{
						QString squp;
						squp = QString("DELETE FROM a_Item WHERE id=%1;")
							.arg(id.GetID());
						QSqlQuery qup(QString(), dbUniverse);
						qup.prepare(squp);
						if(!qup.exec())
							qWarning() << "error: Delete of Item failed. " << id;
					}
					else
					{
						if(item)
						{
							AuItem* replacement = item->FixItem();
							if(replacement)
							{
								delete item;
								item = replacement;
							}
						}

						// Update the item in the database.
						QString squp;
						squp = QString("UPDATE a_Item SET item=? WHERE id=%1;")
							.arg(id.GetID());
						QSqlQuery qup(QString(), dbUniverse);
						qup.prepare(squp);
						if(item)
							qup.addBindValue(item->Pack());
						else
							qup.addBindValue(QByteArray());
						if(!qup.exec())
							qWarning() << "error: Update of Item failed. " << id;
					}
					if(item)
						delete item;
				}
			}
		}
		qDebug() << " " << howMany << "Items Total";
	}
	if(readOnly)
		qDebug() << "Done reading items.";
	else
		qDebug() << "Done fixing items.";
}

// Saves the contents of all sea chests into a file.
void AuEmpireReport::SaveSeaChests(QSqlDatabase& dbUniverse)
{
	bool readOnly = false;
	if(readOnly)
		qDebug() << "Reading sea chests.";
	else
		qDebug() << "Saving sea chests.";

	QString fileName = QString("SeaChests.bin");
	QFile file(fileName);
	if(!readOnly && !file.open(QFile::WriteOnly))
	{
		qWarning() << "error: Failed to create output file.";
		return;
	}
	QDataStream ds(&file);
	ds.setVersion(QDataStream::Qt_5_8);

	// person gear.
	{
		int howMany = 0;
		QString sq = QString("SELECT id,name,seaChest FROM a_Person;");
		QSqlQuery qa(sq, dbUniverse);
		while(qa.next())
		{
			howMany++;
			if(howMany%100 == 0)
				qDebug(" %d Avatars", howMany);

			int qv=0;
			AuId id = AuId(qa.value(qv++).toUInt());
			QString name = qa.value(qv++).toString();

			AuItemHash seaChest[GALAXY_Count];
			int qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Read/deserialize the blob.
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				for(int galaxy=0; galaxy<GALAXY_Count; galaxy++)
					in >> seaChest[galaxy];
			}

			bool anyitems = false;
			for(int galaxy=0; galaxy<GALAXY_Count; galaxy++)
				if(!seaChest[galaxy].empty())
				{
					anyitems = true;
					break;
				}
			if(!anyitems)
				continue;

			if(!readOnly)
			{
				//for(int galaxy=0; galaxy<GALAXY_Count; galaxy++)
					//seaChest[galaxy].RemoveTech();

				// Store the sea chest data into a file.
				ds << id;
				for(int galaxy=0; galaxy<GALAXY_Count; galaxy++)
					ds << seaChest[galaxy];
			}
			else
			{
				qDebug() << id << name;
				for(int galaxy=0; galaxy<GALAXY_Count; galaxy++)
					if(!seaChest[galaxy].empty())
						qDebug() << " " << GalaxyName((GALAXY)galaxy) << seaChest[galaxy].size() << "items";
			}
		}
		qDebug() << " " << howMany << "Avatars Total";
	}
	if(readOnly)
		qDebug() << "Done reading sea chests.";
	else
	{
		file.close();
		qDebug() << "Done storing sea chests.";
	}
}

// Restores the contents of all sea chests from a file.
void AuEmpireReport::RestoreSeaChests(QSqlDatabase& dbUniverse, const QString& fileName)
{
	QFile file(fileName);
	if(!file.open(QFile::ReadOnly))
	{
		qWarning() << "Failed to read input file" << fileName;
		return;
	}

	QDataStream ds(&file);
	ds.setVersion(QDataStream::Qt_5_8);

	int countRead = 0;
	int countRestored = 0;
	AuId id;
	do
	{
		ds >> id;
		if(!id.IsNull())
		{
			// Read the sea chests from the file.
			AuItemHash readSeaChest[GALAXY_Count];
			for(int galaxy=0; galaxy<GALAXY_Count; galaxy++)
				ds >> readSeaChest[galaxy];
			countRead++;

			// Read the avatar's current sea chests.
			bool foundAvatar = false;
			QString name;
			AuItemHash curSeaChest[GALAXY_Count];
			QString sq = QString("SELECT name,seaChest FROM a_Person WHERE id=%1;").arg(id.GetID());
			QSqlQuery qa(sq, dbUniverse);
			while(qa.next())
			{
				foundAvatar = true;
				int qv=0;
				name = qa.value(qv++).toString();

				int qvi = qv++;
				if(!qa.value(qvi).isNull())
				{
					// Read/deserialize the blob.
					QByteArray ba = qa.value(qvi).toByteArray();
					QDataStream in(ba);
					in.setVersion(QDataStream::Qt_4_1);
					for(int galaxy=0; galaxy<GALAXY_Count; galaxy++)
						in >> curSeaChest[galaxy];
				}
			}
			// Avatar could have been deleted.
			if(!foundAvatar)
			{
				qDebug() << "Skipped avatar" << id << "not found in database";
				continue;
			}

			// Combine the restored sea chests with their current sea chests.
			for(int galaxy=0; galaxy<GALAXY_Count; galaxy++)
				for(AuItemHash::const_iterator ii = readSeaChest[galaxy].cbegin(); ii != readSeaChest[galaxy].cend(); ii++)
					curSeaChest[galaxy].AddItem(*(*ii).second);

			// Update the avatar record in the database with the combined sea chests.
			QString squp;
			squp = QString("UPDATE a_Person SET seaChest=? WHERE id=%1;")
				.arg(id.GetID());
			QSqlQuery qup(QString(), dbUniverse);
			qup.prepare(squp);
			{
				QByteArray ba;
				QDataStream out(&ba, QIODevice::WriteOnly);
				out.setVersion(QDataStream::Qt_4_1);
				for(int galaxy=0; galaxy<GALAXY_Count; galaxy++)
					out << curSeaChest[galaxy];
				qup.addBindValue(ba);
			}
			if(!qup.exec())
				qWarning() << "error: Update of Avatar failed. " << id << name;
			else
				qDebug() << "Updated" << id << name << "sea chests";
			countRestored++;
		}
	}
	while(!id.IsNull());
	file.close();
	qDebug() << "AuEmpireReport::RestoreSeaChests read" << countRead << "restored" << countRestored << "from file" << fileName;
}

void AuEmpireReport::ReportArchitect(QSqlDatabase& dbUniverse, const AuId& archId)
{
	Q_UNUSED(dbUniverse);
	// Sc designs
	{
		qDebug() << "Sc Designs";
		AuScDesignHash scdesigns;
		scdesigns.LoadArchitect(archId);
		for(AuScDesignHash::const_iterator di = scdesigns.cbegin(); di != scdesigns.cend(); di++)
		{
			qDebug() << (*di).second.Id() << (*di).second.Name() << (*di).second.StateName()
				<< "hullVol" << (*di).second.HullVolume()
				<< "parts" << (*di).second.Parts().size()
				<< "mem" << (*di).second.MemoryUsage() << "bytes";
		}
	}

	// Bd designs
	{
		qDebug() << "Bd Designs";
		AuBdDesignHash scdesigns;
		scdesigns.LoadArchitect(archId);
		for(AuBdDesignHash::const_iterator di = scdesigns.cbegin(); di != scdesigns.cend(); di++)
		{
			qDebug() << (*di).second.Id() << (*di).second.Name() << (*di).second.StateName()
				<< "hullVol" << (*di).second.HullVolume()
				<< "parts" << (*di).second.Parts().size()
				<< "mem" << (*di).second.MemoryUsage() << "bytes";
		}
	}
}

void AuEmpireReport::FixCompression(QSqlDatabase& dbUniverse)
{
	// Sc designs
	int sccnt = 0;
	{
		qDebug() << "Sc Designs";
		QString sq = QString("SELECT id FROM n_ScDesign;");
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		AuScDesignHash designs;
		while(qry.next())
		{
			quint32 id = qry.value(0).toUInt();
			designs.Load(id);
			qDebug() << " Updating" << (*designs.cbegin()).second.Id() << (*designs.cbegin()).second.Name();
			(*designs.cbegin()).second.Update(dbUniverse);
			designs.clear();
			sccnt++;
		}
	}

	// Bd designs
	int bdcnt = 0;
	{
		qDebug() << "Bd Designs";
		QString sq = QString("SELECT id FROM n_BdDesign;");
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		AuBdDesignHash designs;
		while(qry.next())
		{
			quint32 id = qry.value(0).toUInt();
			designs.Load(id);
			qDebug() << " Updating" << (*designs.cbegin()).second.Id() << (*designs.cbegin()).second.Name();
			(*designs.cbegin()).second.Update(dbUniverse);
			designs.clear();
			bdcnt++;
		}
	}
	qDebug() << "Updated" << sccnt << "sc designs and" << bdcnt << "bd designs";
}

void AuEmpireReport::FixGeoFeatures(QSqlDatabase& dbUniverse)
{
	// Globes
	int glcnt = 0;
	{
		qDebug() << "Globes";
		QString sq = QString("SELECT id,geoFeatures FROM a_WorldGlobe;");
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		while(qry.next())
		{
			quint32 id = qry.value(0).toUInt();
			AuGeologicalFeatures geof;
			{
				// Load geological features.
				QByteArray ba = qry.value(1).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);

				// Read old using 8 bit count.
				quint8 count;
				in >> count;
				quint8 type;
				while(count--)
				{
					in >> type;
					AuGeologicalFeature* geo = AuGeologicalFeature::Create(type);
					if(geo)
					{
						in >> *geo;
						geof.append(geo);
					}
					else
						qWarning() << "AuGeologicalFeatures::operator<< received invalid type" << type;
				}
			}

			if(!geof.empty())
			{
				QString squd;
				squd = QString("UPDATE a_WorldGlobe SET geoFeatures=? WHERE id=%1;")
					.arg(id);
				QSqlQuery qud(QString(), dbUniverse);
				qud.prepare(squd);

				// Geological Features
				QByteArray ba;
				QDataStream out(&ba, QIODevice::WriteOnly);
				out.setVersion(QDataStream::Qt_4_1);
				geof >> out;
				qud.addBindValue(ba);
				if(!qud.exec())
				{
					qWarning() << "error: Update of globe failed." << qud.lastError();
				}
				else
				{
					glcnt++;
					if(glcnt % 100 == 0)
						qDebug() << glcnt << "globes so far";
				}
			}
		}
	}

	int rwcnt = 0;
	{
		qDebug() << "Ringworld Arc Sections";
		QString sq = QString("SELECT id,geoFeatures FROM a_WorldRingSection;");
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		while(qry.next())
		{
			quint32 id = qry.value(0).toUInt();
			AuGeologicalFeatures geof;
			{
				// Load geological features.
				QByteArray ba = qry.value(1).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);

				// Read old using 8 bit count.
				quint8 count;
				in >> count;
				quint8 type;
				while(count--)
				{
					in >> type;
					AuGeologicalFeature* geo = AuGeologicalFeature::Create(type);
					if(geo)
					{
						in >> *geo;
						geof.append(geo);
					}
					else
						qWarning() << "AuGeologicalFeatures::operator<< received invalid type" << type;
				}
			}

			if(!geof.empty())
			{
				QString squd;
				squd = QString("UPDATE a_WorldRingSection SET geoFeatures=? WHERE id=%1;")
					.arg(id);
				QSqlQuery qud(QString(), dbUniverse);
				qud.prepare(squd);

				// Geological Features
				QByteArray ba;
				QDataStream out(&ba, QIODevice::WriteOnly);
				out.setVersion(QDataStream::Qt_4_1);
				geof >> out;
				qud.addBindValue(ba);
				if(!qud.exec())
				{
					qWarning() << "error: Update of ringworld arc failed." << qud.lastError();
				}
				else
				{
					rwcnt++;
					if(rwcnt % 100 == 0)
						qDebug() << rwcnt << "ringworld arcs so far";
				}
			}
		}
	}

	int ggcnt = 0;
	{
		qDebug() << "Gas Giants";
		QString sq = QString("SELECT id,geoFeatures FROM a_WorldGasGiant;");
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		qry.exec();
		while(qry.next())
		{
			quint32 id = qry.value(0).toUInt();
			AuGeologicalFeatures geof;
			{
				// Load geological features.
				QByteArray ba = qry.value(1).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);

				// Read old using 8 bit count.
				quint8 count;
				in >> count;
				quint8 type;
				while(count--)
				{
					in >> type;
					AuGeologicalFeature* geo = AuGeologicalFeature::Create(type);
					if(geo)
					{
						in >> *geo;
						geof.append(geo);
					}
					else
						qWarning() << "AuGeologicalFeatures::operator<< received invalid type" << type;
				}
			}

			if(!geof.empty())
			{
				QString squd;
				squd = QString("UPDATE a_WorldGasGiant SET geoFeatures=? WHERE id=%1;")
					.arg(id);
				QSqlQuery qud(QString(), dbUniverse);
				qud.prepare(squd);

				// Geological Features
				QByteArray ba;
				QDataStream out(&ba, QIODevice::WriteOnly);
				out.setVersion(QDataStream::Qt_4_1);
				geof >> out;
				qud.addBindValue(ba);
				if(!qud.exec())
				{
					qWarning() << "error: Update of gas giant failed." << qud.lastError();
				}
				else
				{
					ggcnt++;
					if(ggcnt % 100 == 0)
						qDebug() << ggcnt << "gas giants so far";
				}
			}
		}
	}
	qDebug() << "Updated" << glcnt << "globes" << rwcnt << "ringworld arcs" << ggcnt << "gas giants";
}

void AuEmpireReport::ReportBdDesigns(QSqlDatabase& dbUniverse, bool sizeCheckOnly, bool fixEm)
{
	if(fixEm)
		qDebug() << "Report Bad Bd Designs, Delete Bad Designs and the Atoms and Exchange Records Referencing Them";
	else
		qDebug() << "Report Bad Bd Designs";
	bool showMessages = true;
	int cnt = 0;
	QList<quint32> bada;
	QList<quint32> badd;
	{
		QString sq = QString("SELECT id,state FROM n_BdDesign;");
		//QString sq = QString("SELECT id,state FROM n_BdDesign WHERE ida=5587457 OR ida=424203399;");	// Racer or Ceruleas
		//QString sq = QString("SELECT id,state FROM n_BdDesign WHERE id=2682301 OR id=2682302;");	// Measuring Tool
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		int dcnt = 0;
		if(qry.exec())
		{
			AuBdDesignHash designs;
			while(qry.next())
			{
				dcnt++;
				quint32 id = qry.value(0).toUInt();
				AuDesign::STATE state = (AuDesign::STATE)qry.value(1).toInt();
				if(state == AuDesign::Edit)
					continue;
				qDebug() << "Loading bd design" << id;
				designs.Load(id);
				AuBdDesign* d = dynamic_cast<AuBdDesign*>(&(*designs.begin()).second);
				if(d)
				{
					cnt++;
					quint64 memSize = d->MemoryUsage();
					if(sizeCheckOnly)
					{
						// Memory usage limit is the same for Final and FinalAssembly designs.
						if(memSize > AuBdReport::SizeLimit())
						{
							qDebug() << d->StateName() << d->Id() << d->Name() << "by" << d->ArchitectId() << d->ArchitectName()
								<< "mem" << memSize << "bytes";
							if(state == AuDesign::FinalAssembly)
								bada.append(d->Id());
							else
								badd.append(d->Id());
						}
					}
					else
					{
						AuBdReport r;
						d->ReportDesign(r);
						QStringList msgs;
						bool isok;
						if(state == AuDesign::FinalAssembly)
							isok = r.OkForAssembly(msgs, *d);
						else
							isok = r.OkToConstruct(msgs, d);
						if(!isok)
						{
							if(state == AuDesign::FinalAssembly)
								bada.append(d->Id());
							else
								badd.append(d->Id());
							qDebug() << d->StateName() << d->Id() << d->Name() << "by" << d->ArchitectId() << d->ArchitectName()
								<< "mem" << memSize << "bytes";
							if(showMessages)
							{
								for(QStringList::const_iterator mi = msgs.cbegin(); mi != msgs.cend(); mi++)
								{
									QString s(*mi);
									if(s.indexOf("Error:") < 0 && s.indexOf("bytes") < 0)
										continue;
									int is = s.indexOf(">");
									if(is >= 0)
										s = s.mid(is+1);
									is = s.indexOf("<");
									s = s.left(is);
									qDebug() << "  " << s;
								}
							}
							if(fixEm)
							{
								QString fileName = QString("%1 %2 bd %3 %4.SoH")
									.arg(d->ArchitectId().GetID())
									.arg(d->ArchitectName())
									.arg(d->Id())
									.arg(d->Name());
								fileName.replace(" ",".");
								qDebug() << "Saving" << fileName;
								d->ClearArchitectForFileSave(d->ArchitectId());
								d->SaveToFile(fileName, SOH_FILE_TYPE);
							}
						}
					}
				}
				designs.clear();
				qDebug() << "Done with bd design" << id << "count" << dcnt << "cnt" << cnt;
			}
		}
		else
		{
			// Execution of the query failed. Report the error.
			QSqlError err = qry.lastError();
			qWarning() << "exec failed" << qry.lastError() << "query" << sq;
		}
	}
	qDebug() << cnt << "Total Bd designs," << badd.size() << "bad designs" << bada.size() << "bad assemblies";
	if(!sizeCheckOnly)
	{
		if(!badd.empty())
		{
			QString sq = QString("SELECT id FROM a_Bd WHERE ");
			for(QList<quint32>::const_iterator bi = badd.cbegin(); bi != badd.cend(); bi++)
			{
				if(bi != badd.cbegin())
					sq += " OR";
				sq += QString(" designId=%1").arg(*bi);
			}
			sq += ";";
			QSqlQuery qry(QString(), dbUniverse);
			qry.prepare(sq);
			if(qry.exec())
			{
				int badcnt = 0;
				while(qry.next())
				{
					AuId atomId = AuId(qry.value(0).toUInt());
					ReportAtom(dbUniverse, atomId, false, false, d_Prefix, d_PrefixStep);
					badcnt++;
				}
				qDebug() << badcnt << "DaBd atoms reference bad designs";
			}
			else
			{
				// Execution of the query failed. Report the error.
				QSqlError err = qry.lastError();
				qWarning() << "exec failed" << qry.lastError() << "query" << sq;
			}
		}
		if(!badd.empty())
		{
			QString sq = QString("SELECT id FROM n_BdExchange WHERE ");
			for(QList<quint32>::const_iterator bi = badd.cbegin(); bi != badd.cend(); bi++)
			{
				if(bi != badd.cbegin())
					sq += " OR";
				sq += QString(" id=%1").arg(*bi);
			}
			sq += ";";
			QSqlQuery qry(QString(), dbUniverse);
			qry.prepare(sq);
			if(qry.exec())
			{
				int badcnt = 0;
				while(qry.next())
				{
					quint32 id = qry.value(0).toUInt();
					qDebug() << "  " << id;
					badcnt++;
				}
				qDebug() << badcnt << "Exchange records reference bad Bd designs";
			}
			else
			{
				// Execution of the query failed. Report the error.
				QSqlError err = qry.lastError();
				qWarning() << "exec failed" << qry.lastError() << "query" << sq;
			}
		}
		if(!bada.empty())
		{
			QString sq = QString("SELECT id FROM a_Bd WHERE ");
			for(QList<quint32>::const_iterator bi = bada.cbegin(); bi != bada.cend(); bi++)
			{
				if(bi != bada.cbegin())
					sq += " OR";
				sq += QString(" designId=%1").arg(*bi);
			}
			sq += ";";
			QSqlQuery qry(QString(), dbUniverse);
			qry.prepare(sq);
			if(qry.exec())
			{
				int badcnt = 0;
				while(qry.next())
				{
					AuId atomId = AuId(qry.value(0).toUInt());
					ReportAtom(dbUniverse, atomId, false, false, d_Prefix, d_PrefixStep);
					badcnt++;
				}
				qDebug() << badcnt << "DaBd atoms reference bad assemblies";
			}
			else
			{
				// Execution of the query failed. Report the error.
				QSqlError err = qry.lastError();
				qWarning() << "exec failed" << qry.lastError() << "query" << sq;
			}
		}
		if(!bada.empty())
		{
			QString sq = QString("SELECT id FROM n_BdExchange WHERE ");
			for(QList<quint32>::const_iterator bi = bada.cbegin(); bi != bada.cend(); bi++)
			{
				if(bi != bada.cbegin())
					sq += " OR";
				sq += QString(" id=%1").arg(*bi);
			}
			sq += ";";
			QSqlQuery qry(QString(), dbUniverse);
			qry.prepare(sq);
			if(qry.exec())
			{
				int badcnt = 0;
				while(qry.next())
				{
					quint32 id = qry.value(0).toUInt();
					qDebug() << "  " << id;
					badcnt++;
				}
				qDebug() << badcnt << "Exchange records reference bad Bd assemblies";
			}
			else
			{
				// Execution of the query failed. Report the error.
				QSqlError err = qry.lastError();
				qWarning() << "exec failed" << qry.lastError() << "query" << sq;
			}
		}
	}
	if(fixEm)
	{
		if(!badd.empty())
		{
			qDebug() << "Deleting bad Bd designs";
			QString sq = QString("DELETE FROM n_BdDesign WHERE ");
			for(QList<quint32>::const_iterator bi = badd.cbegin(); bi != badd.cend(); bi++)
			{
				if(bi != badd.cbegin())
					sq += " OR";
				sq += QString(" id=%1").arg(*bi);
			}
			sq += ";";
			QSqlQuery qry(QString(), dbUniverse);
			qry.prepare(sq);
			if(qry.exec())
			{
				qDebug() << "  " << qry.numRowsAffected() << "deleted";
			}
			else
			{
				// Execution of the query failed. Report the error.
				QSqlError err = qry.lastError();
				qWarning() << "exec failed" << qry.lastError() << "query" << sq;
			}
		}
		if(!badd.empty())
		{
			qDebug() << "Deleting DaBd atoms referencing bad designs";
			QString sq = QString("DELETE FROM a_Bd WHERE ");
			for(QList<quint32>::const_iterator bi = badd.cbegin(); bi != badd.cend(); bi++)
			{
				if(bi != badd.cbegin())
					sq += " OR";
				sq += QString(" designId=%1").arg(*bi);
			}
			sq += ";";
			QSqlQuery qry(QString(), dbUniverse);
			qry.prepare(sq);
			if(qry.exec())
			{
				qDebug() << "  " << qry.numRowsAffected() << "deleted";
			}
			else
			{
				// Execution of the query failed. Report the error.
				QSqlError err = qry.lastError();
				qWarning() << "exec failed" << qry.lastError() << "query" << sq;
			}
		}
		if(!badd.empty())
		{
			qDebug() << "Deleting exchange records referencing bad Bd designs";
			QString sq = QString("DELETE FROM n_BdExchange WHERE ");
			for(QList<quint32>::const_iterator bi = badd.cbegin(); bi != badd.cend(); bi++)
			{
				if(bi != badd.cbegin())
					sq += " OR";
				sq += QString(" id=%1").arg(*bi);
			}
			sq += ";";
			QSqlQuery qry(QString(), dbUniverse);
			qry.prepare(sq);
			if(qry.exec())
			{
				qDebug() << "  " << qry.numRowsAffected() << "deleted";
			}
			else
			{
				// Execution of the query failed. Report the error.
				QSqlError err = qry.lastError();
				qWarning() << "exec failed" << qry.lastError() << "query" << sq;
			}
		}

		if(!bada.empty())
		{
			qDebug() << "Deleting bad Bd assemblies";
			QString sq = QString("DELETE FROM n_BdDesign WHERE ");
			for(QList<quint32>::const_iterator bi = bada.cbegin(); bi != bada.cend(); bi++)
			{
				if(bi != bada.cbegin())
					sq += " OR";
				sq += QString(" id=%1").arg(*bi);
			}
			sq += ";";
			QSqlQuery qry(QString(), dbUniverse);
			qry.prepare(sq);
			if(qry.exec())
			{
				qDebug() << "  " << qry.numRowsAffected() << "deleted";
			}
			else
			{
				// Execution of the query failed. Report the error.
				QSqlError err = qry.lastError();
				qWarning() << "exec failed" << qry.lastError() << "query" << sq;
			}
		}
		if(!bada.empty())
		{
			qDebug() << "Deleting DaBd atoms referencing bad assemblies";
			QString sq = QString("DELETE FROM a_Bd WHERE ");
			for(QList<quint32>::const_iterator bi = bada.cbegin(); bi != bada.cend(); bi++)
			{
				if(bi != bada.cbegin())
					sq += " OR";
				sq += QString(" designId=%1").arg(*bi);
			}
			sq += ";";
			QSqlQuery qry(QString(), dbUniverse);
			qry.prepare(sq);
			if(qry.exec())
			{
				qDebug() << "  " << qry.numRowsAffected() << "deleted";
			}
			else
			{
				// Execution of the query failed. Report the error.
				QSqlError err = qry.lastError();
				qWarning() << "exec failed" << qry.lastError() << "query" << sq;
			}
		}
		if(!bada.empty())
		{
			qDebug() << "Deleting exchange records referencing bad Bd assemblies";
			QString sq = QString("DELETE FROM n_BdExchange WHERE ");
			for(QList<quint32>::const_iterator bi = bada.cbegin(); bi != bada.cend(); bi++)
			{
				if(bi != bada.cbegin())
					sq += " OR";
				sq += QString(" id=%1").arg(*bi);
			}
			sq += ";";
			QSqlQuery qry(QString(), dbUniverse);
			qry.prepare(sq);
			if(qry.exec())
			{
				qDebug() << "  " << qry.numRowsAffected() << "deleted";
			}
			else
			{
				// Execution of the query failed. Report the error.
				QSqlError err = qry.lastError();
				qWarning() << "exec failed" << qry.lastError() << "query" << sq;
			}
		}
	}
	else if(!badd.empty() || !bada.empty())
		qDebug() << "Leaving bad designs, atoms and exchange alone. Use -badbdfix to delete them.";
	qDebug() << "Done.";
}

void AuEmpireReport::ReportScDesigns(QSqlDatabase& dbUniverse, bool sizeCheckOnly, bool fixEm)
{
	if(fixEm)
		qDebug() << "Report Bad Sc Designs, Delete Bad Designs and the Atoms and Exchange Records Referencing Them";
	else
		qDebug() << "Report Bad Sc Designs";
	bool showMessages = true;
	int cnt = 0;
	QList<quint32> bada;
	QList<quint32> badd;
	{
		QString sq = QString("SELECT id,state FROM n_ScDesign;");
		QSqlQuery qry(QString(), dbUniverse);
		qry.prepare(sq);
		int dcnt = 0;
		if(qry.exec())
		{
			AuScDesignHash designs;
			while(qry.next())
			{
				dcnt++;
				quint32 id = qry.value(0).toUInt();
				AuDesign::STATE state = (AuDesign::STATE)qry.value(1).toInt();
				if(state == AuDesign::Edit)
					continue;
				qDebug() << "Loading sc design" << id;
				designs.Load(id);
				AuScDesign* d = dynamic_cast<AuScDesign*>(&(*designs.begin()).second);
				if(d)
				{
					cnt++;
					quint64 memSize = d->MemoryUsage();
					if(sizeCheckOnly)
					{
						// Memory usage limit is the same for Final and FinalAssembly designs.
						if(memSize > AuScReport::SizeLimit())
						{
							qDebug() << d->StateName() << d->Id() << d->Name() << "by" << d->ArchitectId() << d->ArchitectName()
								<< "mem" << memSize << "bytes";
							if(state == AuDesign::FinalAssembly)
								bada.append(d->Id());
							else
								badd.append(d->Id());
						}
					}
					else
					{
						AuScReport r;
						d->ReportDesign(r);
						QStringList msgs;
						bool isok;
						if(state == AuDesign::FinalAssembly)
							isok = r.OkForAssembly(msgs, *d);
						else
							isok = r.OkToManufacture(msgs, d);
						if(!isok)
						{
							if(state == AuDesign::FinalAssembly)
								bada.append(d->Id());
							else
								badd.append(d->Id());
							qDebug() << d->StateName() << d->Id() << d->Name() << "by" << d->ArchitectId() << d->ArchitectName()
								<< "mem" << memSize << "bytes";
							if(showMessages)
							{
								for(QStringList::const_iterator mi = msgs.cbegin(); mi != msgs.cend(); mi++)
								{
									QString s(*mi);
									if(s.indexOf("Error:") < 0 && s.indexOf("bytes") < 0)
										continue;
									int is = s.indexOf(">");
									if(is >= 0)
										s = s.mid(is+1);
									is = s.indexOf("<");
									s = s.left(is);
									qDebug() << "  " << s;
								}
							}
							if(fixEm)
							{
								QString fileName = QString("%1 %2 sc %3 %4.SoH")
									.arg(d->ArchitectId().GetID())
									.arg(d->ArchitectName())
									.arg(d->Id())
									.arg(d->Name());
								fileName.replace(" ",".");
								qDebug() << "Saving" << fileName;
								d->ClearArchitectForFileSave(d->ArchitectId());
								d->SaveToFile(fileName, SOH_FILE_TYPE);
								qDebug() << "Done Saving" << fileName;
							}
						}
					}
				}
				designs.clear();
				qDebug() << "Done with sc design" << id << "count" << dcnt << "cnt" << cnt;
			}
		}
		else
		{
			// Execution of the query failed. Report the error.
			QSqlError err = qry.lastError();
			qWarning() << "exec failed" << qry.lastError() << "query" << sq;
		}
	}
	qDebug() << cnt << "Total Sc designs," << badd.size() << "bad designs" << bada.size() << "bad assemblies";
	if(!sizeCheckOnly)
	{
		if(!badd.empty())
		{
			QString sq = QString("SELECT id FROM a_Sc WHERE ");
			for(QList<quint32>::const_iterator bi = badd.cbegin(); bi != badd.cend(); bi++)
			{
				if(bi != badd.cbegin())
					sq += " OR";
				sq += QString(" idDesign=%1").arg(*bi);
			}
			sq += ";";
			QSqlQuery qry(QString(), dbUniverse);
			qry.prepare(sq);
			if(qry.exec())
			{
				int badcnt=0;
				while(qry.next())
				{
					AuId atomId = AuId(qry.value(0).toUInt());
					ReportAtom(dbUniverse, atomId, false, false, d_Prefix, d_PrefixStep);
					badcnt++;
				}
				qDebug() << badcnt << "DaSc atoms reference bad designs";
			}
			else
			{
				// Execution of the query failed. Report the error.
				QSqlError err = qry.lastError();
				qWarning() << "exec failed" << qry.lastError() << "query" << sq;
			}
		}
		if(!badd.empty())
		{
			QString sq = QString("SELECT id FROM n_ScExchange WHERE ");
			for(QList<quint32>::const_iterator bi = badd.cbegin(); bi != badd.cend(); bi++)
			{
				if(bi != badd.cbegin())
					sq += " OR";
				sq += QString(" id=%1").arg(*bi);
			}
			sq += ";";
			QSqlQuery qry(QString(), dbUniverse);
			qry.prepare(sq);
			if(qry.exec())
			{
				int badcnt=0;
				while(qry.next())
				{
					quint32 id = qry.value(0).toUInt();
					qDebug() << "  " << id;
					badcnt++;
				}
				qDebug() << badcnt << "Exchange records reference bad Sc designs";
			}
			else
			{
				// Execution of the query failed. Report the error.
				QSqlError err = qry.lastError();
				qWarning() << "exec failed" << qry.lastError() << "query" << sq;
			}
		}
		if(!bada.empty())
		{
			QString sq = QString("SELECT id FROM a_Sc WHERE ");
			for(QList<quint32>::const_iterator bi = bada.cbegin(); bi != bada.cend(); bi++)
			{
				if(bi != bada.cbegin())
					sq += " OR";
				sq += QString(" idDesign=%1").arg(*bi);
			}
			sq += ";";
			QSqlQuery qry(QString(), dbUniverse);
			qry.prepare(sq);
			if(qry.exec())
			{
				int badcnt=0;
				while(qry.next())
				{
					AuId atomId = AuId(qry.value(0).toUInt());
					ReportAtom(dbUniverse, atomId, false, false, d_Prefix, d_PrefixStep);
					badcnt++;
				}
				qDebug() << badcnt << "DaSc atoms reference bad assemblies";
			}
			else
			{
				// Execution of the query failed. Report the error.
				QSqlError err = qry.lastError();
				qWarning() << "exec failed" << qry.lastError() << "query" << sq;
			}
		}
		if(!bada.empty())
		{
			QString sq = QString("SELECT id FROM n_ScExchange WHERE ");
			for(QList<quint32>::const_iterator bi = bada.cbegin(); bi != bada.cend(); bi++)
			{
				if(bi != bada.cbegin())
					sq += " OR";
				sq += QString(" id=%1").arg(*bi);
			}
			sq += ";";
			QSqlQuery qry(QString(), dbUniverse);
			qry.prepare(sq);
			if(qry.exec())
			{
				int badcnt=0;
				while(qry.next())
				{
					quint32 id = qry.value(0).toUInt();
					qDebug() << "  " << id;
					badcnt++;
				}
				qDebug() << badcnt << "Exchange records reference bad Sc assemblies";
			}
			else
			{
				// Execution of the query failed. Report the error.
				QSqlError err = qry.lastError();
				qWarning() << "exec failed" << qry.lastError() << "query" << sq;
			}
		}
	}
	if(fixEm)
	{
		if(!badd.empty())
		{
			qDebug() << "Deleting bad Sc designs";
			QString sq = QString("DELETE FROM n_ScDesign WHERE ");
			for(QList<quint32>::const_iterator bi = badd.cbegin(); bi != badd.cend(); bi++)
			{
				if(bi != badd.cbegin())
					sq += " OR";
				sq += QString(" id=%1").arg(*bi);
			}
			sq += ";";
			QSqlQuery qry(QString(), dbUniverse);
			qry.prepare(sq);
			if(qry.exec())
			{
				qDebug() << "  " << qry.numRowsAffected() << "deleted";
			}
			else
			{
				// Execution of the query failed. Report the error.
				QSqlError err = qry.lastError();
				qWarning() << "exec failed" << qry.lastError() << "query" << sq;
			}
		}
		if(!badd.empty())
		{
			qDebug() << "Deleting DaSc atoms referencing bad designs";
			QString sq = QString("DELETE FROM a_Sc WHERE ");
			for(QList<quint32>::const_iterator bi = badd.cbegin(); bi != badd.cend(); bi++)
			{
				if(bi != badd.cbegin())
					sq += " OR";
				sq += QString(" idDesign=%1").arg(*bi);
			}
			sq += ";";
			QSqlQuery qry(QString(), dbUniverse);
			qry.prepare(sq);
			if(qry.exec())
			{
				qDebug() << "  " << qry.numRowsAffected() << "deleted";
			}
			else
			{
				// Execution of the query failed. Report the error.
				QSqlError err = qry.lastError();
				qWarning() << "exec failed" << qry.lastError() << "query" << sq;
			}
		}
		if(!badd.empty())
		{
			qDebug() << "Deleting exchange records referencing bad Sc designs";
			QString sq = QString("DELETE FROM n_ScExchange WHERE ");
			for(QList<quint32>::const_iterator bi = badd.cbegin(); bi != badd.cend(); bi++)
			{
				if(bi != badd.cbegin())
					sq += " OR";
				sq += QString(" id=%1").arg(*bi);
			}
			sq += ";";
			QSqlQuery qry(QString(), dbUniverse);
			qry.prepare(sq);
			if(qry.exec())
			{
				qDebug() << "  " << qry.numRowsAffected() << "deleted";
			}
			else
			{
				// Execution of the query failed. Report the error.
				QSqlError err = qry.lastError();
				qWarning() << "exec failed" << qry.lastError() << "query" << sq;
			}
		}

		if(!bada.empty())
		{
			qDebug() << "Deleting bad Sc assemblies";
			QString sq = QString("DELETE FROM n_ScDesign WHERE ");
			for(QList<quint32>::const_iterator bi = bada.cbegin(); bi != bada.cend(); bi++)
			{
				if(bi != bada.cbegin())
					sq += " OR";
				sq += QString(" id=%1").arg(*bi);
			}
			sq += ";";
			QSqlQuery qry(QString(), dbUniverse);
			qry.prepare(sq);
			if(qry.exec())
			{
				qDebug() << "  " << qry.numRowsAffected() << "deleted";
			}
			else
			{
				// Execution of the query failed. Report the error.
				QSqlError err = qry.lastError();
				qWarning() << "exec failed" << qry.lastError() << "query" << sq;
			}
		}
		if(!bada.empty())
		{
			qDebug() << "Deleting DaSc atoms referencing bad assemblies";
			QString sq = QString("DELETE FROM a_Sc WHERE ");
			for(QList<quint32>::const_iterator bi = bada.cbegin(); bi != bada.cend(); bi++)
			{
				if(bi != bada.cbegin())
					sq += " OR";
				sq += QString(" idDesign=%1").arg(*bi);
			}
			sq += ";";
			QSqlQuery qry(QString(), dbUniverse);
			qry.prepare(sq);
			if(qry.exec())
			{
				qDebug() << "  " << qry.numRowsAffected() << "deleted";
			}
			else
			{
				// Execution of the query failed. Report the error.
				QSqlError err = qry.lastError();
				qWarning() << "exec failed" << qry.lastError() << "query" << sq;
			}
		}
		if(!bada.empty())
		{
			qDebug() << "Deleting exchange records referencing bad Sc assemblies";
			QString sq = QString("DELETE FROM n_ScExchange WHERE ");
			for(QList<quint32>::const_iterator bi = bada.cbegin(); bi != bada.cend(); bi++)
			{
				if(bi != bada.cbegin())
					sq += " OR";
				sq += QString(" id=%1").arg(*bi);
			}
			sq += ";";
			QSqlQuery qry(QString(), dbUniverse);
			qry.prepare(sq);
			if(qry.exec())
			{
				qDebug() << "  " << qry.numRowsAffected() << "deleted";
			}
			else
			{
				// Execution of the query failed. Report the error.
				QSqlError err = qry.lastError();
				qWarning() << "exec failed" << qry.lastError() << "query" << sq;
			}
		}
	}
	else if(!badd.empty() || !bada.empty())
		qDebug() << "Leaving bad designs, atoms and exchange alone. Use -badscfix to delete them.";
	qDebug() << "Done.";
}

void AuEmpireReport::SaveBdDesign(QSqlDatabase& dbUniverse, quint32 designId)
{
	Q_UNUSED(dbUniverse);
	qDebug() << "Loading bd design" << designId;
	AuBdDesignHash designs;
	designs.Load(designId);
	AuBdDesign* d = dynamic_cast<AuBdDesign*>(&(*designs.begin()).second);
	if(d)
	{
		quint64 memSize = d->MemoryUsage();
		QStringList msgs;
		qDebug() << d->StateName() << d->Id() << d->Name() << "by" << d->ArchitectId() << d->ArchitectName()
			<< "mem" << memSize << "bytes";
		QString fileName = QString("%1 %2 bd %3 %4.SoH")
			.arg(d->ArchitectId().GetID())
			.arg(d->ArchitectName())
			.arg(d->Id())
			.arg(d->Name());
		fileName.replace(" ",".");
		qDebug() << "Saving" << fileName;
		d->ClearArchitectForFileSave(d->ArchitectId());
		d->SaveToFile(fileName, SOH_FILE_TYPE);
		qDebug() << "Done Saving" << fileName;
	}
	designs.clear();
}

void AuEmpireReport::SaveScDesign(QSqlDatabase& dbUniverse, quint32 designId)
{
	Q_UNUSED(dbUniverse);
	qDebug() << "Loading sc design" << designId;
	AuScDesignHash designs;
	designs.Load(designId);
	AuScDesign* d = dynamic_cast<AuScDesign*>(&(*designs.begin()).second);
	if(d)
	{
		quint64 memSize = d->MemoryUsage();
		QStringList msgs;
		qDebug() << d->StateName() << d->Id() << d->Name() << "by" << d->ArchitectId() << d->ArchitectName()
			<< "mem" << memSize << "bytes";
		QString fileName = QString("%1 %2 sc %3 %4.SoH")
			.arg(d->ArchitectId().GetID())
			.arg(d->ArchitectName())
			.arg(d->Id())
			.arg(d->Name());
		fileName.replace(" ",".");
		qDebug() << "Saving" << fileName;
		d->ClearArchitectForFileSave(d->ArchitectId());
		d->SaveToFile(fileName, SOH_FILE_TYPE);
		qDebug() << "Done Saving" << fileName;
	}
	designs.clear();
}

void AuEmpireReport::FixBdExchange(QSqlDatabase& dbUniverse)
{
	qDebug() << "Fixing Building Blueprint Exchange";

	// Load the design exchange records.
	AuBdDesignExchange exch;
	exch.SqlSelectAll(dbUniverse);
	qDebug() << exch.size() << "records loaded";

	// Load and analyze each associated design.
	int deleted = 0;
	int number = 1;
	QMap<quint32, bool> updated;
	AuBdDesignExchange::iterator xi = exch.begin(); 
	while(xi != exch.end())
	{
		qDebug() << number << ". Loading Building blueprint" << (*xi).DesignId() << (*xi).DesignName();
		number++;
		AuBdDesign design;
		design.Load(xi.key());
		AuBdReport report;
		design.ReportDesign(report);
		if((*xi).DesignState() == AuDesign::Final)
		{
			QStringList msgs;
			if(!report.OkToConstruct(msgs, &design))
			{
				qDebug() << "  Building blueprint" << (*xi).DesignId() << (*xi).DesignName()
					<< "by" << (*xi).DesignArchitectId() << (*xi).DesignArchitectName()
					<< "is not valid for construction. Deleted.";
				(*xi).SqlDelete(dbUniverse);
				deleted++;
				xi = exch.erase(xi);
				continue;
			}
		}
		else if((*xi).DesignState() == AuDesign::FinalAssembly)
		{
			QStringList msgs;
			if(!report.OkForAssembly(msgs, design))
			{
				qDebug() << "  Building blueprint" << (*xi).DesignId() << (*xi).DesignName()
					<< "by" << (*xi).DesignArchitectId() << (*xi).DesignArchitectName()
					<< "is not valid for assembly. Deleted.";
				(*xi).SqlDelete(dbUniverse);
				deleted++;
				xi = exch.erase(xi);
				continue;
			}
		}
		else
		{
			qDebug() << "  Building blueprint" << (*xi).DesignId() << (*xi).DesignName()
				<< "by" << (*xi).DesignArchitectId() << (*xi).DesignArchitectName()
				<< "has invalid state" << AuDesign::StateName((AuDesign::STATE)(*xi).DesignState()) << ". Deleted.";
			(*xi).SqlDelete(dbUniverse);
			deleted++;
			xi = exch.erase(xi);
			continue;
		}

		if(report.ReportAnalysis((*xi).DesignState()) != (*xi).ReportText())
		{
			qDebug() << "  Building blueprint" << (*xi).DesignId() << (*xi).DesignName()
				<< "by" << (*xi).DesignArchitectId() << (*xi).DesignArchitectName()
				<< "report updated.";

			QFile ofile(QString("bd%1o.txt").arg((*xi).DesignId()));
			if(ofile.open(QIODevice::WriteOnly))
			{
				QTextStream so(&ofile);
				so.setCodec("UTF-8");

				QStringList old = (*xi).ReportText().split("<br>");
				for(QStringList::const_iterator si = old.cbegin(); si != old.cend(); si++)
					so << *si << "\n";

				ofile.close();
			}

			QFile nfile(QString("bd%1n.txt").arg((*xi).DesignId()));
			if(nfile.open(QIODevice::WriteOnly))
			{
				QTextStream so(&nfile);
				so.setCodec("UTF-8");

				QStringList nu = report.ReportAnalysis((*xi).DesignState()).split("<br>");
				for(QStringList::const_iterator si = nu.cbegin(); si != nu.cend(); si++)
					so << *si << "\n";

				nfile.close();
			}

			if((*xi).WhenPublished().IsNull())
				(*xi).SetWhenPublished();
			(*xi).SetReport(report);
			(*xi).SqlUpdate(dbUniverse);
			updated.insert((*xi).DesignId(), true);
		}
		xi++;
	}
	qDebug() << exch.size() << "records remain valid," << updated.size() << "were updated," << deleted << "were deleted.";
	//if(!updated.empty())
		//qDebug() << "updated" << updated.keys();
}

void AuEmpireReport::FixScExchange(QSqlDatabase& dbUniverse)
{
	qDebug() << "Fixing Spacecraft Blueprint Exchange";

	// Load the design exchange records.
	AuScDesignExchange exch;
	exch.SqlSelectAll(dbUniverse);
	qDebug() << exch.size() << "records loaded";

	// Load and analyze each associated design.
	int deleted = 0;
	int number = 1;
	QMap<quint32, bool> updated;
	AuScDesignExchange::iterator xi = exch.begin(); 
	while(xi != exch.end())
	{
		qDebug() << number << ". Loading Spacecraft blueprint" << (*xi).DesignId() << (*xi).DesignName();
		number++;
		AuScDesign design;
		design.Load(xi.key());
		AuScReport report;
		design.ReportDesign(report);
		if((*xi).DesignState() == AuDesign::Final)
		{
			QStringList msgs;
			if(!report.OkToManufacture(msgs, &design))
			{
				qDebug() << "  Spacecraft blueprint" << (*xi).DesignId() << (*xi).DesignName()
					<< "by" << (*xi).DesignArchitectId() << (*xi).DesignArchitectName()
					<< "is not valid for manufacture. Deleted.";
				(*xi).SqlDelete(dbUniverse);
				deleted++;
				xi = exch.erase(xi);
				continue;
			}
		}
		else if((*xi).DesignState() == AuDesign::FinalAssembly)
		{
			QStringList msgs;
			if(!report.OkForAssembly(msgs, design))
			{
				qDebug() << "  Spacecraft blueprint" << (*xi).DesignId() << (*xi).DesignName()
					<< "by" << (*xi).DesignArchitectId() << (*xi).DesignArchitectName()
					<< "is not valid for assembly. Deleted.";
				(*xi).SqlDelete(dbUniverse);
				deleted++;
				xi = exch.erase(xi);
				continue;
			}
		}
		else
		{
			qDebug() << "  Spacecraft blueprint" << (*xi).DesignId() << (*xi).DesignName()
				<< "by" << (*xi).DesignArchitectId() << (*xi).DesignArchitectName()
				<< "has invalid state" << AuDesign::StateName((AuDesign::STATE)(*xi).DesignState()) << ". Deleted.";
			(*xi).SqlDelete(dbUniverse);
			deleted++;
			xi = exch.erase(xi);
			continue;
		}

		if(report.ReportAnalysis((*xi).DesignState()) != (*xi).ReportText())
		{
			qDebug() << "  Spacecraft blueprint" << (*xi).DesignId() << (*xi).DesignName()
				<< "by" << (*xi).DesignArchitectId() << (*xi).DesignArchitectName()
				<< "report updated.";

			QFile ofile(QString("sc%1o.txt").arg((*xi).DesignId()));
			if(ofile.open(QIODevice::WriteOnly))
			{
				QTextStream so(&ofile);
				so.setCodec("UTF-8");

				QStringList old = (*xi).ReportText().split("<br>");
				for(QStringList::const_iterator si = old.cbegin(); si != old.cend(); si++)
					so << *si << "\n";

				ofile.close();
			}

			QFile nfile(QString("sc%1n.txt").arg((*xi).DesignId()));
			if(nfile.open(QIODevice::WriteOnly))
			{
				QTextStream so(&nfile);
				so.setCodec("UTF-8");

				QStringList nu = report.ReportAnalysis((*xi).DesignState()).split("<br>");
				for(QStringList::const_iterator si = nu.cbegin(); si != nu.cend(); si++)
					so << *si << "\n";

				nfile.close();
			}

			if((*xi).WhenPublished().IsNull())
				(*xi).SetWhenPublished();
			(*xi).SetReport(report);
			(*xi).SqlUpdate(dbUniverse);
			updated.insert((*xi).DesignId(), true);
		}
		xi++;
	}
	qDebug() << exch.size() << "records remain valid," << updated.size() << "were updated," << deleted << "were deleted.";
	//if(!updated.empty())
		//qDebug() << "updated" << updated.keys();
}

void AuEmpireReport::FixPirateDNA(QSqlDatabase& dbUniverse)
{
	QString dnaFiles[PIRATE_Count] =
	{
		"",
		"Akson.chr",
		"Balorite.chr",
		"Dendrae.chr",
		"Haxu.chr",
		"Klatra.chr",
		"Malacon.chr",
		"Myntaka.chr",
		"Ogar.chr",
		"Otari.chr",
		"Seledon.chr",
		"Syth.chr",
		"Tassad.chr",
		"Vilmorti.chr",
		"Vreen.chr",
		"Zuul.chr",
	};
	for(int pi = PIRATE_Akson; pi < PIRATE_Count; pi++)
	{
		// Read the chr file.
		QFile file(dnaFiles[pi]);
		if(file.open(QFile::ReadOnly))
		{
			QDataStream ds(&file);
			ds.setVersion(QDataStream::Qt_4_1);

			DhDNA_ dna;
			quint8 gender = 0;
			bool bLefty = false;

			// File Type Byte
			bool readDNA = false;
			quint8 chrFileType;
			ds >> chrFileType;
			if(chrFileType == CHR_FILE_TYPE)
			{
				// Version Byte
				quint8 chrFileVersion;
				ds >> chrFileVersion;

				// Character Data
				QByteArray baDNA;
				ds >> baDNA;
				dna = DhDNA_(baDNA);

				ds >> gender;
				ds >> bLefty;
				readDNA = true;
			}
			else
				qWarning() << "error: Failed to read" << dnaFiles[pi] << "fileType" << chrFileType;
			file.close();
			if(readDNA)
			{
				// Write the DNA to the empire record.
				AuSpecies species = DhDNA(dna).Species();
				qDebug() << "FixPirateDNA empire" << pi << "file" << dnaFiles[pi] << "species" << species.Description();
				QString squd;
				squd = QString("UPDATE g_Empire SET capitalSpec=? WHERE id=%1;").arg(pi);
				QSqlQuery qud(QString(), dbUniverse);
				qud.prepare(squd);
				qud.addBindValue(species.Key());
				if(!qud.exec())
					qWarning() << "error: Failed to update g_Empire" << qud.lastError();
			}
		}
		else
			qWarning() << "error: Failed to open" << dnaFiles[pi];
	}
}

void AuEmpireReport::FixCityItemStacks(QSqlDatabase& dbUniverse)
{
	bool readOnly = true;
	if(readOnly)
		qDebug() << "Reading items.";
	else
		qDebug() << "Fixing items.";

	// Bds
	{
		int howMany = 0;
		int fixed = 0;
		QString sq = QString("SELECT id,name,cargo FROM a_Bd;");	//DHH animalia needs to use field cargo4
		QSqlQuery qa(sq, dbUniverse);
		while(qa.next())
		{
			howMany++;
			if(howMany%100 == 0)
				qDebug(" %d Bds", howMany);

			int qv=0;
			AuId id = AuId(qa.value(qv++).toUInt());
			QString name = qa.value(qv++).toString();
			int qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				// Read/deserialize the blob.
				QByteArray ba = qa.value(qvi).toByteArray();
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				AuItemHash oldfuel;
				AuItemHash cargo, tools;
				AuKnownPatents patents;
				DhDNA livestockDNA;
				quint16 storeCommodity;
				in >> oldfuel;
				in >> cargo;
				in >> tools;
				in >> patents;
				in >> livestockDNA;
				in >> storeCommodity;

				AuItemHash nuCargo;
				nuCargo.AddItems(cargo);
				AuItemHash nuTools;
				nuTools.AddItems(tools);

				if(cargo.size() != nuCargo.size() || tools.size() != nuTools.size())
				{
					qDebug() << "bd" << id << name << "inv" << cargo.size() << "became" << nuCargo.size() << "tools" << tools.size() << "became" << nuTools.size();
					if(!readOnly)
					{
						// Update the bd in the database.
						QString squp;
						squp = QString("UPDATE a_Bd SET cargo=? WHERE id=%1;")	//DHH animalia needs to use field cargo4
							.arg(id.GetID());
						QSqlQuery qup(QString(), dbUniverse);
						qup.prepare(squp);
						{
							QByteArray ba;
							QDataStream out(&ba, QIODevice::WriteOnly);
							out.setVersion(QDataStream::Qt_4_1);
							out << cargo;
							qup.addBindValue(ba);
						}

						if(!qup.exec())
							qWarning() << "error: Update of Bd failed. " << id << name;
					}
					fixed++;
				}
			}
		}
		if(readOnly)
			qDebug() << " Bds" << howMany << "Total" << fixed << "Need Fixing";
		else
			qDebug() << " Bds" << howMany << "Total" << fixed << "Fixed";
	}

	if(readOnly)
		qDebug() << "Done reading items.";
	else
		qDebug() << "Done fixing items.";
}

void AuEmpireReport::ReadShipCargo(QSqlDatabase& dbUniverse)
{
	QString sq;
	sq = QString("SELECT id,name,cargo FROM a_Sc WHERE id=1160358347;");
	QSqlQuery qa(QString(), dbUniverse);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		int qvi=0;
		AuId id = AuId(qa.value(qvi++).toUInt());
		QString name = qa.value(qvi++).toString();

		QByteArray ba = qa.value(qvi++).toByteArray();
		QDataStream in(ba);
		in.setVersion(QDataStream::Qt_4_1);
		AuItemHash cargo;
		cargo << in;

		qDebug() << id << name << cargo.size();
	}
}

void AuEmpireReport::ReadCeruleas(QSqlDatabase& dbUniverse)
{
	QString sq;
	sq = QString("SELECT inv FROM Ceruleas;");
	QSqlQuery qa(QString(), dbUniverse);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		int qv=0;
		AuGear gear;
		AuGear arenaGear;
		quint8 m_PowerMoveMax;
		quint8 m_PowerJump;
		QByteArray ba = qa.value(qv++).toByteArray();
		if(!ba.isEmpty())
		{
			qDebug() << "inv field" << ba.size();
			// Read/deserialize the blob.
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_4_1);
			in >> gear;
			in >> arenaGear;
			in >> m_PowerMoveMax;
			in >> m_PowerJump;
		}

		qDebug() << "gear" << gear.size() << "arena" << arenaGear.size();

		QString squ = QString("UPDATE a_Person SET inv=? WHERE id=424203399;");
		QSqlQuery qud(QString(), dbUniverse);
		qud.prepare(squ);

		// Inventory
		{
			QByteArray ba;
			QDataStream out(&ba, QIODevice::WriteOnly);
			out.setVersion(QDataStream::Qt_4_1);
			gear >> out;
			arenaGear >> out;
			quint8 ver = 0x01;
			out << ver;
			out << m_PowerMoveMax;
			out << m_PowerJump;
			qud.addBindValue(ba);
		}

		if(qud.exec())
			qDebug() << "Ceruleas inventory updated";
		else
			qWarning() << "error: Ceruleas inventory update failed.";
	}
}

void AuEmpireReport::ReadCityReportInventory(QSqlDatabase& dbUniverse)
{
	QString sq;
	sq = QString("SELECT reportHist FROM a_Bd WHERE id=1314414577;");
	QSqlQuery qa(QString(), dbUniverse);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		int qvi=0;
		QByteArray ba = qa.value(qvi++).toByteArray();
		QDataStream in(ba);
		in.setVersion(QDataStream::Qt_4_1);
		AuCityReportHistory reportHistory;
		reportHistory << in;
		QString fileName = QString("Inv%1.bin").arg(reportHistory.Last().m_Inventory->size());
		QFile file(fileName);
		if(file.open(QFile::WriteOnly))
		{
			QDataStream ds(&file);
			ds.setVersion(QDataStream::Qt_5_7);
			ds << *reportHistory.Last().m_Inventory;
			file.close();
		}
		qDebug() << "wrote" << fileName;
	}
}

void AuEmpireReport::ReadCityReportFile()
{
	QFile file("Inv67502.bin");
	if(!file.open(QFile::ReadOnly))
	{
		qWarning() << "error: no file Inv67502.bin";
		return;
	}
	QDataStream ds(&file);
	ds.setVersion(QDataStream::Qt_5_7);
	AuItemHash inv;
	ds >> inv;
	file.close();
	qDebug() << "Read" << inv.size() << "items";

	/*
	QFile nfile("Inv67502.txt");
	if(nfile.open(QIODevice::WriteOnly))
	{
		QTextStream so(&nfile);
		so.setCodec("UTF-8");

		for(AuItemHash::const_iterator ii = inv.cbegin(); ii != inv.cend(); ii++)
			so << (*ii).second->ListDesc() << "\n";

		nfile.close();
	}
	*/

	//COMMODITYID cid = CID_Milk;
	AuItemHash ainv;
	//for(AuItemHash::iterator it = inv.find(cid); it != inv.end() && (*it).first == cid; it++)
	for(AuItemHash::const_iterator it = inv.cbegin(); it != inv.cend(); it++)
		ainv.AddItem(*(*it).second);

	AuItemHash binv;
	for(AuItemHash::const_iterator it = ainv.cbegin(); it != ainv.cend(); it++)
		binv.AddItem(*(*it).second);

	AuItemHash cinv;
	for(AuItemHash::const_iterator it = binv.cbegin(); it != binv.cend(); it++)
		cinv.AddItem(*(*it).second);

	AuItemHash dinv;
	for(AuItemHash::const_iterator it = cinv.cbegin(); it != cinv.cend(); it++)
		dinv.AddItem(*(*it).second);

	AuItemHash einv;
	for(AuItemHash::const_iterator it = dinv.cbegin(); it != dinv.cend(); it++)
		einv.AddItem(*(*it).second);
	qDebug() << "inv" << inv.size()
		<< "ainv" << ainv.size()
		<< "binv" << binv.size()
		<< "cinv" << cinv.size()
		<< "dinv" << dinv.size()
		<< "einv" << dinv.size();

	QFile nfile("inv.txt");
	if(nfile.open(QIODevice::WriteOnly))
	{
		QTextStream so(&nfile);
		so.setCodec("UTF-8");
		for(AuItemHash::const_iterator ii = inv.cbegin(); ii != inv.cend(); ii++)
			so << (*ii).second->ListDesc() << "\n";
		nfile.close();
	}

	QFile afile("ainv.txt");
	if(afile.open(QIODevice::WriteOnly))
	{
		QTextStream so(&afile);
		so.setCodec("UTF-8");
		for(AuItemHash::const_iterator ii = ainv.cbegin(); ii != ainv.cend(); ii++)
			so << (*ii).second->ListDesc() << "\n";
		afile.close();
	}

	QFile bfile("binv.txt");
	if(bfile.open(QIODevice::WriteOnly))
	{
		QTextStream so(&bfile);
		so.setCodec("UTF-8");
		for(AuItemHash::const_iterator ii = binv.cbegin(); ii != binv.cend(); ii++)
			so << (*ii).second->ListDesc() << "\n";
		bfile.close();
	}

	/*
	COMMODITYID cid = CID_Cheese;
	AuItemHash::iterator ci = inv.end();
	for(AuItemHash::iterator it = inv.find(cid); it != inv.end() && (*it).first == cid; it++)
	{
		if(ci == inv.cend())
		{
			ci = it;
			qDebug() << "first" << (*it).second->ListDesc();
		}
		else
		{
			if((*ci).second->Quality() == (*it).second->Quality())
			{
				qDebug() << "Q=" << (*it).second->ListDesc();
				bool canStack = (*ci).second->CanStack((*it).second);
				if(canStack)
				{
					qDebug() << "found one that can stack";
					break;
				}
			}
		}
	}
	*/

	qDebug() << "ReadCityReportFile done";
}

void AuEmpireReport::FixSpawnCitySystems(QSqlDatabase& dbUniverse)
{
	// Select avatars data from the database.
	QHash<AuId, AuId> personIds;	// key is person id, value is their spawn bd
	QString sq;
	sq = QString("SELECT id,spawnCity FROM a_Person WHERE idp >= 0;");	// include idp=0 for agent limbo detection, idp<0 is deleted
	QSqlQuery qry(QString(), dbUniverse);
	qry.prepare(sq);
	qry.exec();
	while(qry.next())
	{
		// Read the fields from the query.
		int qv=0;
		AuId personId = AuId(qry.value(qv++).toUInt());
		AuId spawnCity = AuId(qry.value(qv++).toUInt());
		if(spawnCity.IsNull() || spawnCity == AuId(1))
			continue;	// Skip the ones with no spawn city.
		personIds.insert(personId, spawnCity);
	}

	qDebug() << "AuEmpireReport::FixSpawnCitySystems" << personIds.size() << "people found with spawn cities";

	int people = 0;
	for(QHash<AuId, AuId>::const_iterator pi = personIds.cbegin(); pi != personIds.cend(); pi++)
	{
		QString systemName;
		AuId systemId = QueryParentSystemRecursive(dbUniverse, *pi, systemName);
		if(systemId.IsNull())
		{
			qWarning() << "error: AuEmpireReport::FixSpawnCitySystems unable to fix person" << pi.key();
			continue;
		}
		// Found the parent system of their spawn city/building. Update the person record.
		QString squpd;
		squpd = QString("UPDATE a_Person SET spawnSystem=?, spawnSystemName=? WHERE id=%1;").arg(pi.key().GetID());
		QSqlQuery qupd(QString(), dbUniverse);
		qupd.prepare(squpd);
		qupd.addBindValue(systemId.GetID());
		qupd.addBindValue(systemName);
		if(qupd.exec())
			people++;
		else
		{
			QSqlError err = qupd.lastError();
			qWarning() << "error: AuEmpireReport::FixSpawnCitySystems person" << pi.key() << "spawn city system update failed"
				<< squpd << err;
		}
	}

	qDebug() << "AuEmpireReport::FixSpawnCitySystems" << people << "fixed";
}

void AuEmpireReport::QueryWorldBds(QSqlDatabase& dbUniverse)
{
	qDebug() << "AuEmpireReport::QueryWorldBds";
	QHash<AuId, QString> worlds;
	QString sq;
	sq = QString("SELECT id,name FROM a_World;");
	QSqlQuery qa(QString(), dbUniverse);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		int qvi=0;
		AuId id = AuId(qa.value(qvi++).toUInt());
		QString name = qa.value(qvi++).toString();
		worlds.insert(id, name);
	}
	unsigned int mostBds = 0;
	unsigned int worldsWithBds = 0;
	unsigned int worldsWithBds200 = 0;
	unsigned int worldsWithBds250 = 0;
	unsigned int worldsWithBds300 = 0;
	unsigned int worldsWithBds400 = 0;
	for(QHash<AuId, QString>::iterator wi = worlds.begin(); wi != worlds.end(); wi++)
	{
		QString sq;
		sq = QString("SELECT count(id) FROM a_Bd WHERE idp=%1 and industry != 13 and industry != 131;").arg(wi.key().GetID());
		QSqlQuery qa(QString(), dbUniverse);
		qa.prepare(sq);
		qa.exec();
		if(qa.next())
		{
			int qvi=0;
			unsigned int bds = qa.value(qvi++).toUInt();
			if(bds)
			{
				worldsWithBds++;
				if(bds >= 200)
					worldsWithBds200++;
				if(bds >= 250)
					worldsWithBds250++;
				if(bds >= 300)
					worldsWithBds300++;
				if(bds >= 400)
					worldsWithBds400++;
				if(bds > mostBds)
					mostBds = bds;
				qDebug() << wi.key() << *wi << bds;
			}
		}
	}
	qDebug() << "AuEmpireReport::QueryWorldBds worlds" << worlds.size() << "with bds" << worldsWithBds
		<< "with 200 or more bds" << worldsWithBds200
		<< "with 250 or more bds" << worldsWithBds250
		<< "with 300 or more bds" << worldsWithBds300
		<< "with 400 or more bds" << worldsWithBds400
		<< "most bds on world" << mostBds;
}

void AuEmpireReport::FixConditions(QSqlDatabase& dbUniverse)
{
	// Select conditions from all creatures.
	QString sq;
	sq = QString("SELECT id,name,conditions FROM a_Unit;");
	QSqlQuery qa(QString(), dbUniverse);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		int qvi=0;
		AuId id = AuId(qa.value(qvi++).toUInt());
		QString name = qa.value(qvi++).toString();
		QByteArray ba = qa.value(qvi++).toByteArray();
		AuConditionList conditions;
		if(!ba.isEmpty())
		{
			// Conditions
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_4_1);
			in >> conditions;
			qDebug() << "AuEmpireReport::FixConditions" << id << name << "read" << conditions.size() << "conditions";
		}
	}
}

void AuEmpireReport::ReportScPref(QSqlDatabase& dbUniverse, const QString& empireName)
{
	QString sq;
	sq = QString("SELECT scprefs FROM g_Empire WHERE name=?;");
	QSqlQuery qry(QString(), dbUniverse);
	qry.prepare(sq);
	qry.addBindValue(empireName);
	qry.exec();
	while(qry.next())
	{
		int qv=0;

		// Sc blueprint preferences.
		AuScPrefs scPrefs;
		QByteArray bcomp = qry.value(qv++).toByteArray();
		if(!bcomp.isEmpty())
		{
			QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,"");
			QDataStream ds(&ba, QIODevice::ReadOnly);
			ds.setVersion(QDataStream::Qt_4_1);
			ds >> scPrefs;
		}

		qDebug() << empireName;
		qDebug() << "Preferred Spacecraft Blueprints";
		for(QHash<AuScPrefs::SCROLE, AuScPref>::const_iterator pi = scPrefs.PreferredBlueprints().cbegin();
				pi != scPrefs.PreferredBlueprints().cend(); pi++)
			qDebug() << AuScPrefs::RoleName(pi.key()) << "blueprints" << (*pi).Blueprints();
	}
};

void AuEmpireReport::Emoji()
{
	/*
	QFile file("emoji-sequences.txt");
	if(!file.open(QFile::ReadOnly))
	{
		qWarning() << "error: no file emoji-sequences.txt";
		return;
	}

	QTextStream tstream(&file);
	tstream.setCodec("UTF-8");

	QMap<QString, QString> emojis;
	while(!tstream.atEnd())
	{
		QString line = tstream.readLine().trimmed();
		if(line.isEmpty())
			continue;
		
		// Parse the line
		QString comment;
		QString desc;
		QString type;
		QString code;

		int ci = line.indexOf('#');
		if(ci >= 0)
		{
			comment = line.mid(ci+1).trimmed();
			line = line.left(ci).trimmed();
		}
		if(line.isEmpty())
			continue;

		int di = line.lastIndexOf(';');
		if(di >= 0)
		{
			desc = line.mid(di+1).trimmed();
			line = line.left(di).trimmed();
		}

		int ti = line.lastIndexOf(';');
		if(ti >= 0)
		{
			type = line.mid(ti+1).trimmed();
			line = line.left(ti).trimmed();
		}

		int pi = line.indexOf("..");
		if(pi >= 0)
		{
			// codeBegin..codeEnd
			uint codeBegin = line.left(pi).trimmed().toUInt(0, 16);
			uint codeEnd = line.mid(pi+2).trimmed().toUInt(0, 16);
			QString key = desc;
			QString emo;
			for(uint uc = codeBegin; uc <= codeEnd; uc++)
			{
				emo = QString::fromUcs4(&uc, 1);
				if(!emo.isEmpty() && key.indexOf("keycap") < 0 && key.indexOf("flag:") < 0)
					emojis.insert(emo, key);
				key = "";
			}
		}
		else
		{
			int si = line.indexOf(' ');
			if(si >= 0)
			{
				// code codeModifier
				uint code[2];
				code[0] = line.left(si).trimmed().toUInt(0, 16);
				code[1] = line.mid(si+1).trimmed().toUInt(0, 16);
				if(code[0])
				{
					QString emo = QString::fromUcs4(code, 2);
					if(!emo.isEmpty() && desc.indexOf("keycap") < 0 && desc.indexOf("flag:") < 0)
						emojis.insert(emo, desc);
				}
			}
			else
			{
				// code
				uint code = line.trimmed().toUInt(0, 16);
				if(code)
				{
					QString emo = QString::fromUcs4(&code, 1);
					if(!emo.isEmpty() && desc.indexOf("keycap") < 0 && desc.indexOf("flag:") < 0)
						emojis.insert(emo, desc);
				}
			}
		}
	}

	file.close();

	// Save the result into a file.
	QFile efile("emoji.txt");
	if(efile.open(QIODevice::WriteOnly))
	{
		QTextStream so(&efile);
		so.setCodec("UTF-8");
		for(QMap<QString, QString>::const_iterator ei = emojis.cbegin(); ei != emojis.cend(); ei++)
			so << ei.key() << "," << *ei << "\n";
		efile.close();
	}
	qDebug() << "Emoji done.";
	*/

	QFile file("emoji-test.txt");
	if(!file.open(QFile::ReadOnly))
	{
		qWarning() << "error: no file emoji-test.txt";
		return;
	}

	QTextStream tstream(&file);
	tstream.setCodec("UTF-8");

	QStringList emojis;
	QStringList emojidesc;
	while(!tstream.atEnd())
	{
		QString line = tstream.readLine().trimmed();
		if(line.isEmpty())
			continue;
		
		// Parse the line
		QString comment;
		QString desc;

		int ci = line.indexOf('#');
		if(ci == 0)
			continue;
		else if(ci > 0)
		{
			comment = line.mid(ci+1).trimmed();
			line = line.left(ci).trimmed();
		}
		if(line.isEmpty())
			continue;

		int si = line.indexOf(';');
		if(si >= 0)
			line = line.left(si).trimmed();

		int di = comment.indexOf('E');	// Find the version. e.g. E13.0
		if(di >= 0)
		{
			// Desc follows version. e.g. '⌚ E0.6 watch'
			int si = comment.indexOf(' ', di);
			desc = comment.mid(si+1).trimmed();
		}

		// Build ucs4 encoded unicode.
		QVector<uint> ucs4;
		QStringList codes = line.split(' ');
		for(QStringList::const_iterator ci = codes.cbegin(); ci != codes.cend(); ci++)
		{
			uint c = (*ci).toUInt(0, 16);
			ucs4.append(c);
		}

		QString emo = QString::fromUcs4(ucs4.constData(), ucs4.size());
		emojis.append(emo);
		emojidesc.append(desc);
	}

	file.close();

	// Save the result into a file.
	QFile efile("emoji.txt");
	if(efile.open(QIODevice::WriteOnly))
	{
		QTextStream so(&efile);
		so.setCodec("UTF-8");
		QStringList::const_iterator ei = emojis.cbegin();
		QStringList::const_iterator di = emojidesc.cbegin();
		for(;ei != emojis.cend() && di != emojidesc.cend(); ei++, di++)
			so << *ei << " " << *di << "\n";
		efile.close();
	}
	qDebug() << "Emoji done.";
}

void AuEmpireReport::EmojiVal(const QString& emoji)
{
	if(emoji.isEmpty())
		return;
	qDebug() << "EmojiVal" << emoji << "len" << emoji.length();
	for(int c=0; c<emoji.length(); c++)
	{
		qDebug() << "  char" << c << "unicode" << emoji[c].unicode() << QString("%1").arg(emoji[c].unicode(), 0,16)
			<< "dec" << emoji[c].decomposition();
	}

	qDebug() << "utf16";
	const ushort* u16 = emoji.utf16();
	while(*u16)
	{
		qDebug() << "  utf16 char" << *u16 << QString("%1").arg(*u16, 0,16);
		u16++;
	}

	qDebug() << "ucs4";
	QVector<uint> ucs4 = emoji.toUcs4();
	for(QVector<uint>::const_iterator ui = ucs4.cbegin(); ui != ucs4.cend(); ui++)
		qDebug() << "  ucs4" << *ui << QString("%1").arg(*ui, 0,16);
}

// Reports remaining activation time on old-style PayPal-purchased accounts.
void AuEmpireReport::ReportActiveAccounts(QSqlDatabase& dbLogin)
{
	QDate datePayStopped(2020, 8, 14);
#if(QT_VERSION >= QT_VERSION_CHECK(5,14,0))
	QDateTime dtPayStopped(datePayStopped.startOfDay());
#else
	QDateTime dtPayStopped(datePayStopped);
#endif
	AuTime timeStopped;
	timeStopped = dtPayStopped;

	QString sq = QString("SELECT id,name,email,activated,days FROM g_Player WHERE status=0 ORDER BY email;");
	QSqlQuery qry(sq, dbLogin);
	while(qry.next())
	{
		int qv=0;
		quint32 id = qry.value(qv++).toUInt();
		QString name = qry.value(qv++).toString();
		QString email = qry.value(qv++).toString();
		QDateTime acctActivated = qry.value(qv++).toDateTime();
        acctActivated.setTimeSpec(Qt::UTC);
        AuTime activated;
		activated = acctActivated;
        unsigned int days = qry.value(qv++).toUInt();

		AuTime expires = activated.AddDays(days);
		if(expires < timeStopped)
			continue;
		int daysRemaining = timeStopped.DaysTo(expires);
		int monthsRemaining = daysRemaining/30;
		daysRemaining -= monthsRemaining * 30;

		if(monthsRemaining)
			qDebug() << id << name << email << monthsRemaining << "months" << daysRemaining << "days remaining";
		else
			qDebug() << id << name << email << daysRemaining << "days remaining";
	}
}

/*
void AuEmpireReport::PatreonCSV(const QString& fileName)
{
	QFile file(fileName);
	if(!file.open(QFile::ReadOnly))
	{
		qWarning() << "error: Failed to open input file" << fileName;
		return;
	}

	QTextStream tstream(&file);
	tstream.setCodec("UTF-8");

	QStringList headerFields;
	QMap<double, QMap<QString, AuPatron> > patronsByPledge;
	while(!tstream.atEnd())
	{
		QString line = tstream.readLine().trimmed();
		if(line.isEmpty())
			continue;
		QStringList fields = line.split(',');
		if(headerFields.empty())
		{
			headerFields = fields;
			continue;
		}
		if(fields.size() < 21)
			continue;

		QString name = fields[0];
		double pledgeAmt = fields[6].toDouble();
		QString tier = fields[8];
		QString id = fields[20];
		AuPatron patron(id, pledgeAmt, tier, name);
		if(!patron.IsValid())
			continue;

		QMap<double, QMap<QString, AuPatron> >::iterator pi = patronsByPledge.find(pledgeAmt);
		if(pi == patronsByPledge.end())
		{
			// First patron at the pledge level.
			QMap<QString, AuPatron> patrons;
			patrons.insert(name.toUpper(), patron);
			patronsByPledge.insert(pledgeAmt, patrons);
		}
		else
		{
			// Additional patron at the pledge level.
			(*pi).insert(name.toUpper(), patron);
		}
	}

	QFile sofile;
	sofile.open(stdout, QIODevice::WriteOnly);
	QTextStream so(&sofile);
	so.setCodec("UTF-8");

	// List patrons from highest contributor to lowest, sorted within pledge levels.
	for(QMap<double, QMap<QString, AuPatron> >::const_iterator ti = patronsByPledge.cend()-1; ti != patronsByPledge.cbegin()-1;
			ti--)
	{
		// List members at each pledge level alphabetically.
		for(QMap<QString, AuPatron>::const_iterator pi = (*ti).cbegin(); pi != (*ti).cend(); pi++)
			so << (*pi).Id() << "," << (*pi).Pledge() << "," << (*pi).Tier() << "," << (*pi).Name() << '\n';
	}
}
*/

void AuEmpireReport::PirateUbiquity()
{
	// Ubiquity values by galaxy.
    int ubiquity[PIRATE_Count];
	const DgData& gd = g_Au.LockDataForRead();
	for(int gi=0; gi<GALAXY_Count; gi++)
	{
		GALAXY galaxy = (GALAXY)gi;
		gd.m_gEmpires.PirateEmpireUbiquity(galaxy, ubiquity);
		int totalu = 0;
		for(int ui=0; ui<PIRATE_Count; ui++)
			totalu += ubiquity[ui];
		qDebug() << GalaxyName(galaxy) << "pirate ubiquity" << totalu << "ready" << gd.m_gEmpires.PirateUbiquity(galaxy);
	}
	g_Au.UnlockData();
}

/*
void AuEmpireReport::FixNpcOfficers(QSqlDatabase& dbUniverse)
{
	QHash<AuId, AuId> ids;	// key=atom id, value=parent id
	QString sq("SELECT id,idp FROM a_CitOfficer;");
	QSqlQuery qa(QString(), dbUniverse);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		int qvi=0;
		AuId id = AuId(qa.value(qvi++).toUInt());
		AuId pid = AuId(qa.value(qvi++).toUInt());
		ids.insert(id, pid);
	}
	if(ids.empty())
	{
		qDebug() << "No officers found to upgrade.";
		return;
	}
	qDebug() << QString("Upgrading %1 officers to Npcs.").arg(ids.size());

	// Load the old citizens from the db and convert them to npcs.
	DaAtomList npcs;
	int loaded = 0;
	for(QHash<AuId, AuId>::const_iterator ii = ids.cbegin(); ii != ids.cend(); ii++)
	{
		DaCitOfficer* cit = DaCitOfficer::LoadSubatoms(0, dbUniverse, *ii, ii.key());
		if(cit)
		{
			DaNpc* npc = new DaNpc(*cit);
			if(npc)
				npcs.append(npc);
			delete cit;
			loaded++;
		}
		else
			qWarning() << QString("error: Failed to load DaCitOfficer %1").arg(ii.key().GetID());
	}
	qDebug() << QString("Loaded %L1 DaCitOfficer records from the db.").arg(loaded);

	// Delete the converted citizens from the db.
	AuIdList delIds;
	npcs.ListIds(delIds);
	DeleteAtoms(dbUniverse, delIds);
	qDebug() << QString("Deleted %L1 DaCitOfficer records from the db.").arg(delIds.size());

	// Store the Npcs into the db.
	for(DaAtomList::const_iterator ni = npcs.cbegin(); ni != npcs.cend(); ni++)
	{
		DaAtom* npc = *ni;
		npc->Insert(dbUniverse);
	}
	qDebug() << QString("Stored %L1 DaNpc records into the db.").arg(npcs.size());
}

void AuEmpireReport::FixNpcCrew(QSqlDatabase& dbUniverse)
{
	QHash<AuId, AuId> ids;	// key=atom id, value=parent id
	QString sq("SELECT id,idp FROM a_CitCrew;");
	QSqlQuery qa(QString(), dbUniverse);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		int qvi=0;
		AuId id = AuId(qa.value(qvi++).toUInt());
		AuId pid = AuId(qa.value(qvi++).toUInt());
		ids.insert(id, pid);
	}
	if(ids.empty())
	{
		qDebug() << "No crew found to upgrade.";
		return;
	}
	qDebug() << QString("Upgrading %1 crew to Npcs.").arg(ids.size());

	// Load the old citizens from the db and convert them to npcs.
	DaAtomList npcs;
	int loaded = 0;
	for(QHash<AuId, AuId>::const_iterator ii = ids.cbegin(); ii != ids.cend(); ii++)
	{
		DaCitCrew* cit = DaCitCrew::LoadSubatoms(0, dbUniverse, *ii, ii.key());
		if(cit)
		{
			DaNpc* npc = new DaNpc(*cit);
			if(npc)
				npcs.append(npc);
			delete cit;
			loaded++;
		}
		else
			qWarning() << QString("error: Failed to load DaCitCrew %1").arg(ii.key().GetID());
	}
	qDebug() << QString("Loaded %L1 DaCitCrew records from the db.").arg(loaded);

	// Delete the converted citizens from the db.
	AuIdList delIds;
	npcs.ListIds(delIds);
	DeleteAtoms(dbUniverse, delIds);
	qDebug() << QString("Deleted %L1 DaCitCrew records from the db.").arg(delIds.size());

	// Store the Npcs into the db.
	for(DaAtomList::const_iterator ni = npcs.cbegin(); ni != npcs.cend(); ni++)
	{
		DaAtom* npc = *ni;
		npc->Insert(dbUniverse);
	}
	qDebug() << QString("Stored %L1 DaNpc records into the db.").arg(npcs.size());
}

void AuEmpireReport::FixNpcTroops(QSqlDatabase& dbUniverse)
{
	QHash<AuId, AuId> ids;	// key=atom id, value=parent id
	QString sq("SELECT id,idp FROM a_CitTroop;");
	QSqlQuery qa(QString(), dbUniverse);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		int qvi=0;
		AuId id = AuId(qa.value(qvi++).toUInt());
		AuId pid = AuId(qa.value(qvi++).toUInt());
		ids.insert(id, pid);
	}
	if(ids.empty())
	{
		qDebug() << "No troops found to upgrade.";
		return;
	}
	qDebug() << QString("Upgrading %1 troops to Npcs.").arg(ids.size());

	// Load the old citizens from the db and convert them to npcs.
	DaAtomList npcs;
	int loaded = 0;
	for(QHash<AuId, AuId>::const_iterator ii = ids.cbegin(); ii != ids.cend(); ii++)
	{
		DaCitTroop* cit = DaCitTroop::LoadSubatoms(0, dbUniverse, *ii, ii.key());
		if(cit)
		{
			DaNpc* npc = new DaNpc(*cit);
			if(npc)
				npcs.append(npc);
			delete cit;
			loaded++;
		}
		else
			qWarning() << QString("error: Failed to load DaCitTroop %1").arg(ii.key().GetID());
	}
	qDebug() << QString("Loaded %L1 DaCitTroop records from the db.").arg(loaded);

	// Delete the converted citizens from the db.
	AuIdList delIds;
	npcs.ListIds(delIds);
	DeleteAtoms(dbUniverse, delIds);
	qDebug() << QString("Deleted %L1 DaCitTroop records from the db.").arg(delIds.size());

	// Store the Npcs into the db.
	for(DaAtomList::const_iterator ni = npcs.cbegin(); ni != npcs.cend(); ni++)
	{
		DaAtom* npc = *ni;
		npc->Insert(dbUniverse);
	}
	qDebug() << QString("Stored %L1 DaNpc records into the db.").arg(npcs.size());
}

void AuEmpireReport::FixNpcWorkers(QSqlDatabase& dbUniverse)
{
	QHash<AuId, AuId> ids;	// key=atom id, value=parent id
	QString sq("SELECT id,idp FROM a_CitWorker;");
	QSqlQuery qa(QString(), dbUniverse);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		int qvi=0;
		AuId id = AuId(qa.value(qvi++).toUInt());
		AuId pid = AuId(qa.value(qvi++).toUInt());
		ids.insert(id, pid);
	}
	if(ids.empty())
	{
		qDebug() << "No workers found to upgrade.";
		return;
	}
	qDebug() << QString("Upgrading %1 workers to Npcs.").arg(ids.size());

	// Load the old citizens from the db and convert them to npcs.
	DaAtomList npcs;
	int loaded = 0;
	for(QHash<AuId, AuId>::const_iterator ii = ids.cbegin(); ii != ids.cend(); ii++)
	{
		DaCitWorker* cit = DaCitWorker::LoadSubatoms(0, dbUniverse, *ii, ii.key());
		if(cit)
		{
			DaNpc* npc = new DaNpc(*cit);
			if(npc)
				npcs.append(npc);
			delete cit;
			loaded++;
		}
		else
			qWarning() << QString("error: Failed to load DaCitWorker %1").arg(ii.key().GetID());
	}
	qDebug() << QString("Loaded %L1 DaCitWorker records from the db.").arg(loaded);

	// Delete the converted citizens from the db.
	AuIdList delIds;
	npcs.ListIds(delIds);
	DeleteAtoms(dbUniverse, delIds);
	qDebug() << QString("Deleted %L1 DaCitWorker records from the db.").arg(delIds.size());

	// Store the Npcs into the db.
	for(DaAtomList::const_iterator ni = npcs.cbegin(); ni != npcs.cend(); ni++)
	{
		DaAtom* npc = *ni;
		npc->Insert(dbUniverse);
	}
	qDebug() << QString("Stored %L1 DaNpc records into the db.").arg(npcs.size());
}

void AuEmpireReport::FixNpcIndigenous(QSqlDatabase& dbUniverse)
{
	QHash<AuId, AuId> ids;	// key=atom id, value=parent id
	QString sq("SELECT id,idp FROM a_CitIndigenous;");
	QSqlQuery qa(QString(), dbUniverse);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		int qvi=0;
		AuId id = AuId(qa.value(qvi++).toUInt());
		AuId pid = AuId(qa.value(qvi++).toUInt());
		ids.insert(id, pid);
	}
	if(ids.empty())
	{
		qDebug() << "No indigenous found to upgrade.";
		return;
	}
	qDebug() << QString("Upgrading %1 indigenous to Npcs.").arg(ids.size());

	// Load the old citizens from the db and convert them to npcs.
	DaAtomList npcs;
	int loaded = 0;
	for(QHash<AuId, AuId>::const_iterator ii = ids.cbegin(); ii != ids.cend(); ii++)
	{
		DaCitIndigenous* cit = DaCitIndigenous::LoadSubatoms(0, dbUniverse, *ii, ii.key());
		if(cit)
		{
			DaNpc* npc = new DaNpc(*cit);
			if(npc)
				npcs.append(npc);
			delete cit;
			loaded++;
		}
		else
			qWarning() << QString("error: Failed to load DaCitIndigenous %1").arg(ii.key().GetID());
	}
	qDebug() << QString("Loaded %L1 DaCitIndigenous records from the db.").arg(loaded);

	// Delete the converted citizens from the db.
	AuIdList delIds;
	npcs.ListIds(delIds);
	DeleteAtoms(dbUniverse, delIds);
	qDebug() << QString("Deleted %L1 DaCitIndigenous records from the db.").arg(delIds.size());

	// Store the Npcs into the db.
	for(DaAtomList::const_iterator ni = npcs.cbegin(); ni != npcs.cend(); ni++)
	{
		DaAtom* npc = *ni;
		npc->Insert(dbUniverse);
	}
	qDebug() << QString("Stored %L1 DaNpc records into the db.").arg(npcs.size());
}

void AuEmpireReport::FixNpcCharacters(QSqlDatabase& dbUniverse)
{
	QHash<AuId, AuId> ids;	// key=atom id, value=parent id
	QString sq("SELECT id,idp FROM a_CitCharacter;");
	QSqlQuery qa(QString(), dbUniverse);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		int qvi=0;
		AuId id = AuId(qa.value(qvi++).toUInt());
		AuId pid = AuId(qa.value(qvi++).toUInt());
		ids.insert(id, pid);
	}
	if(ids.empty())
	{
		qDebug() << "No characters found to upgrade.";
		return;
	}
	qDebug() << QString("Upgrading %1 characters to Npcs.").arg(ids.size());

	// Load the old citizens from the db and convert them to npcs.
	DaAtomList npcs;
	int loaded = 0;
	for(QHash<AuId, AuId>::const_iterator ii = ids.cbegin(); ii != ids.cend(); ii++)
	{
		DaCitCharacter* cit = DaCitCharacter::LoadSubatoms(0, dbUniverse, *ii, ii.key());
		if(cit)
		{
			DaNpc* npc = new DaNpc(*cit);
			if(npc)
				npcs.append(npc);
			delete cit;
			loaded++;
		}
		else
			qWarning() << QString("error: Failed to load DaCitCharacter %1").arg(ii.key().GetID());
	}
	qDebug() << QString("Loaded %L1 DaCitCharacter records from the db.").arg(loaded);

	// Delete the converted citizens from the db.
	AuIdList delIds;
	npcs.ListIds(delIds);
	DeleteAtoms(dbUniverse, delIds);
	qDebug() << QString("Deleted %L1 DaCitCharacter records from the db.").arg(delIds.size());

	// Store the Npcs into the db.
	for(DaAtomList::const_iterator ni = npcs.cbegin(); ni != npcs.cend(); ni++)
	{
		DaAtom* npc = *ni;
		npc->Insert(dbUniverse);
	}
	qDebug() << QString("Stored %L1 DaNpc records into the db.").arg(npcs.size());
}

void AuEmpireReport::FixNpcPassengers(QSqlDatabase& dbUniverse)
{
	QHash<AuId, AuId> ids;	// key=atom id, value=parent id
	QString sq("SELECT id,idp FROM a_CitPassenger;");
	QSqlQuery qa(QString(), dbUniverse);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		int qvi=0;
		AuId id = AuId(qa.value(qvi++).toUInt());
		AuId pid = AuId(qa.value(qvi++).toUInt());
		ids.insert(id, pid);
	}
	if(ids.empty())
	{
		qDebug() << "No passengers found to upgrade.";
		return;
	}
	qDebug() << QString("Upgrading %1 passengers to Npcs.").arg(ids.size());

	// Load the old citizens from the db and convert them to npcs.
	DaAtomList npcs;
	int loaded = 0;
	for(QHash<AuId, AuId>::const_iterator ii = ids.cbegin(); ii != ids.cend(); ii++)
	{
		DaCitPassenger* cit = DaCitPassenger::LoadSubatoms(0, dbUniverse, *ii, ii.key());
		if(cit)
		{
			DaNpc* npc = new DaNpc(*cit);
			if(npc)
				npcs.append(npc);
			delete cit;
			loaded++;
		}
		else
			qWarning() << QString("error: Failed to load DaCitPassenger %1").arg(ii.key().GetID());
	}
	qDebug() << QString("Loaded %L1 DaCitPassenger records from the db.").arg(loaded);

	// Delete the converted citizens from the db.
	AuIdList delIds;
	npcs.ListIds(delIds);
	DeleteAtoms(dbUniverse, delIds);
	qDebug() << QString("Deleted %L1 DaCitPassenger records from the db.").arg(delIds.size());

	// Store the Npcs into the db.
	for(DaAtomList::const_iterator ni = npcs.cbegin(); ni != npcs.cend(); ni++)
	{
		DaAtom* npc = *ni;
		npc->Insert(dbUniverse);
	}
	qDebug() << QString("Stored %L1 DaNpc records into the db.").arg(npcs.size());
}
*/

