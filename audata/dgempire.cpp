//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dgempire.cpp,v 1.181 2022/04/08 14:51:24 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dgempire.h"
//
#include "aucity.h"
#include "auglobal.h"
#include "audna.h"
#include "austylesheet.h"
#include "dhdna.h"
#include "hashpack.h"
#include <QComboBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
// The Q_DECLARE_METATYPE macro is also required.
static int s_DgEmpireMetaType = qRegisterMetaType<DgEmpire>();

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuCitizen::operator>>(QDataStream& out) const
{
	out << m_id;
	out << m_name;
	quint8 genderOffice = (m_Gender << 4) | 0x80;	//DHH Top bit is a temporary version bit. 11/5/2015
	if(m_Office)
		genderOffice |= 0x08;
	out << genderOffice;
	if(genderOffice & 0x08)
		out << *m_Office;
}

void AuCitizen::operator<<(QDataStream& in)
{
	in >> m_id;
	in >> m_name;
	quint8 genderOffice;
	in >> genderOffice;

	m_Gender = (genderOffice >> 4) & 0x3;					// b5-b4 are the gender.

	if(genderOffice & 0x80)
	{
		// New style AuOffice governance information.
		if(genderOffice & 0x08)
		{
			if(!m_Office)
				m_Office = new AuOffice;
			in >> *m_Office;
		}
		else
		{
			if(m_Office)
				delete m_Office;
			m_Office = 0;
		}
	}
	else
	{
		// Old style OFFICE and governance domain information.
		// Obsolete office enumeration.
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

		// Old AuCitizen variables that were removed.
		QHash<AuId, QString>	m_Governs;	// Atom ids and names that are governed by this person. Empty for Citizens.
		AuId			m_idEmpVote;	// Id of player this citizen is voting for Emperor.
		AuId			m_idHeirVote;	// Id of player this citizen is voting for Heir.
		AuId			m_idSystemVote;	// Id of player this citizen is voting for Sector management.
		AuId			m_idSectorVote;	// Id of player this citizen is voting for System management.
		AuId			m_idPlanetVote;	// Id of player this citizen is voting for Planet management.
		AuId			m_idCityVote;	// Id of player this citizen is voting for City management.
		quint8			m_OfficeReq;	// Office person is requesting.
		AuId			m_idGoverningReq;	// Id of place requesting. i.e. SectorId, PlanetId, etc.
		QString			m_sGoverningReq;// Name of place requesting governance.
		qint32			m_iReputation;	// The reputation of this player in the empire
		QDateTime		m_tmAssigned;	// When a position was assigned to the citizen. Not the time a position was deassigned.

		in >> m_idEmpVote;
		in >> m_idHeirVote;
		in >> m_idSystemVote;
		in >> m_idSectorVote;
		in >> m_idPlanetVote;
		in >> m_idCityVote;
		in >> m_OfficeReq;
		in >> m_idGoverningReq;
		in >> m_sGoverningReq;
		in >> m_iReputation;
		in >> m_tmAssigned;
		in >> m_Governs;
	}
}

quint64 AuCitizen::MemoryUsage() const
{
	quint64 mem = sizeof(AuCitizen);
	mem += StringMemoryUsage(m_name);
	if(m_Office)
		mem += m_Office->MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

quint64 AuKnownEmpire::MemoryUsage() const
{
	quint64 mem = sizeof(AuKnownEmpire);
	mem += m_Dossier.MemoryUsage();
	mem += StringMemoryUsage(m_name);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DgEmpire::DgEmpire(const QString& name)
{
	m_Cities = new AuCityHash;
	m_ContrailHue = 170;	// Default blue
	m_ContrailSat = 127;	// Default blue
	m_DefaultStance = 1;	// 0=friend, 1=neutral, 2=enemy
	m_GameRole = EMPIRE_Player;
	m_RewCapBase = 0;
	m_RewCapCapitalEmpire = 0;
	m_RewCapCapitalGalaxy = 0;
	m_RewCapCapitalSector = 0;
	m_RewCapCity = 0;
	m_RewCapStation = 0;	// per 1000 structure points
	m_RewCapSpaceship = 0;	// per 1000 structure points
	m_RewCapStarship = 0;	// per 1000 structure points
	m_RewDiscHabitable = 0;	// per habitable world
	m_RewDiscWorm = 0;
	m_RewDmgStation = 0;	// per 10 damage points inflicted
	m_RewDmgSpaceship = 0;	// per 10 damage points inflicted
	m_RewDmgStarship = 0;	// per 10 damage points inflicted
	m_RewKillEnemy = 0;
	m_RewSalary = 0;		// for govt officials
	m_RewSpotCity = 0;
	m_RewSpotEnemy = 0;
	m_TaxIncome = 20;		// 1/2 percent increments
	m_TaxSales = 10;		// 1/2 percent increments
	m_TaxSubsidy = 5;		// 1/2 percent increments

	m_ThemeColors[AuScFace::THEME_ExteriorPrimary] = AuStyleSheet::HazeronBlue();
	m_ThemeColors[AuScFace::THEME_ExteriorSecondary] = QColor(13,13,13);
	m_ThemeColors[AuScFace::THEME_ExteriorTertiary] = AuStyleSheet::HazeronGold(2);
	m_ThemeColors[AuScFace::THEME_InteriorPrimary] = AuStyleSheet::HazeronBlue();
	m_ThemeColors[AuScFace::THEME_InteriorSecondary] = QColor(13,13,13);
	m_ThemeColors[AuScFace::THEME_InteriorTertiary] = AuStyleSheet::HazeronGold(2);

	m_id = 0;				// Database record id.
	m_immig = 1;			// Restricted
	m_name = name;

	while(m_Commerce.size() < CID_Count)
		m_Commerce.append(EXPOL_ToNeutral);	// Grow commerce array to include nu commodities that have been added.

	InitWarPolicy();	// Initialize war policy to something reasonable.
}

DgEmpire::DgEmpire()
{
	m_Cities = new AuCityHash;
	m_ContrailHue = 170;	// Default blue
	m_ContrailSat = 127;	// Default blue
	m_DefaultStance = 1;	// 0=friend, 1=neutral, 2=enemy
	m_GameRole = EMPIRE_Player;
	m_RewCapBase = 0;
	m_RewCapCapitalEmpire = 0;
	m_RewCapCapitalGalaxy = 0;
	m_RewCapCapitalSector = 0;
	m_RewCapCity = 0;
	m_RewCapStation = 0;	// per 1000 structure points
	m_RewCapSpaceship = 0;	// per 1000 structure points
	m_RewCapStarship = 0;	// per 1000 structure points
	m_RewDiscHabitable = 0;	// per habitable world
	m_RewDiscWorm = 0;
	m_RewDmgStation = 0;	// per 10 damage points inflicted
	m_RewDmgSpaceship = 0;	// per 10 damage points inflicted
	m_RewDmgStarship = 0;	// per 10 damage points inflicted
	m_RewKillEnemy = 0;
	m_RewSalary = 0;		// for govt officials
	m_RewSpotCity = 0;
	m_RewSpotEnemy = 0;
	m_TaxIncome = 20;		// 1/2 percent increments
	m_TaxSales = 10;		// 1/2 percent increments
	m_TaxSubsidy = 5;		// 1/2 percent increments

	m_ThemeColors[AuScFace::THEME_ExteriorPrimary] = AuStyleSheet::HazeronBlue();
	m_ThemeColors[AuScFace::THEME_ExteriorSecondary] = QColor(13,13,13);
	m_ThemeColors[AuScFace::THEME_ExteriorTertiary] = AuStyleSheet::HazeronGold(3);
	m_ThemeColors[AuScFace::THEME_InteriorPrimary] = AuStyleSheet::HazeronBlue();
	m_ThemeColors[AuScFace::THEME_InteriorSecondary] = QColor(13,13,13);
	m_ThemeColors[AuScFace::THEME_InteriorTertiary] = AuStyleSheet::HazeronGold(3);

	m_id = 0;		// Database record id.
	m_immig = 1;			// Restricted
}

DgEmpire::~DgEmpire()
{
	if(m_Cities)
		delete m_Cities;
}

DgEmpire::DgEmpire(const DgEmpire& c) : m_BdPrefs(c.m_BdPrefs),
	m_ScPrefs(c.m_ScPrefs),
	m_Citizens(c.m_Citizens),
	m_Commerce(c.m_Commerce),
	m_Stance(c.m_Stance), m_WarCriteria(c.m_WarCriteria)
{
	m_Announcements = c.m_Announcements;
	m_Cities = new AuCityHash(*c.m_Cities);
	m_ContrailHue = c.m_ContrailHue;
	m_ContrailSat = c.m_ContrailSat;
	m_DefaultStance = c.m_DefaultStance;
	m_GameRole = c.m_GameRole;
	m_JoinMessage = c.m_JoinMessage;
	m_OnLine = c.m_OnLine;
	m_Patents = c.m_Patents;
	m_RewCapBase = c.m_RewCapBase;
	m_RewCapCapitalEmpire = c.m_RewCapCapitalEmpire;
	m_RewCapCapitalGalaxy = c.m_RewCapCapitalGalaxy;
	m_RewCapCapitalSector = c.m_RewCapCapitalSector;
	m_RewCapCity = c.m_RewCapCity;
	m_RewCapStation = c.m_RewCapStation;
	m_RewCapSpaceship = c.m_RewCapSpaceship;
	m_RewCapStarship = c.m_RewCapStarship;
	m_RewDiscHabitable = c.m_RewDiscHabitable;
	m_RewDiscWorm = c.m_RewDiscWorm;
	m_RewDmgStation = c.m_RewDmgStation;
	m_RewDmgSpaceship = c.m_RewDmgSpaceship;
	m_RewDmgStarship = c.m_RewDmgStarship;
	m_RewKillEnemy = c.m_RewKillEnemy;
	m_RewSalary = c.m_RewSalary;
	m_RewSpotCity = c.m_RewSpotCity;
	m_RewSpotEnemy = c.m_RewSpotEnemy;
	m_TaxIncome = c.m_TaxIncome;
	m_TaxSales = c.m_TaxSales;
	m_TaxSubsidy = c.m_TaxSubsidy;

	for(int tc=0; tc<AuScFace::THEME_Count; tc++)
		m_ThemeColors[tc] = c.m_ThemeColors[tc];

	m_capitol = c.m_capitol;
	m_capitolSpecies = c.m_capitolSpecies;
	m_flag = c.m_flag;
	m_id = c.m_id;
	m_immig = c.m_immig;
	m_immigPassCrypt = c.m_immigPassCrypt;
	m_name = c.m_name;
}

const DgEmpire& DgEmpire::operator=(const DgEmpire& c)
{
	m_Announcements = c.m_Announcements;
	m_BdPrefs = c.m_BdPrefs;
	*m_Cities = *c.m_Cities;
	m_Citizens = c.m_Citizens;
	m_Commerce = c.m_Commerce;
	m_ContrailHue = c.m_ContrailHue;
	m_ContrailSat = c.m_ContrailSat;
	m_DefaultStance = c.m_DefaultStance;
	m_GameRole = c.m_GameRole;
	m_JoinMessage = c.m_JoinMessage;
	m_OnLine = c.m_OnLine;
	m_Patents = c.m_Patents;
	m_RewCapBase = c.m_RewCapBase;
	m_RewCapCapitalEmpire = c.m_RewCapCapitalEmpire;
	m_RewCapCapitalGalaxy = c.m_RewCapCapitalGalaxy;
	m_RewCapCapitalSector = c.m_RewCapCapitalSector;
	m_RewCapCity = c.m_RewCapCity;
	m_RewCapStation = c.m_RewCapStation;
	m_RewCapSpaceship = c.m_RewCapSpaceship;
	m_RewCapStarship = c.m_RewCapStarship;
	m_RewDiscHabitable = c.m_RewDiscHabitable;
	m_RewDiscWorm = c.m_RewDiscWorm;
	m_RewDmgStation = c.m_RewDmgStation;
	m_RewDmgSpaceship = c.m_RewDmgSpaceship;
	m_RewDmgStarship = c.m_RewDmgStarship;
	m_RewKillEnemy = c.m_RewKillEnemy;
	m_RewSalary = c.m_RewSalary;
	m_RewSpotCity = c.m_RewSpotCity;
	m_RewSpotEnemy = c.m_RewSpotEnemy;
	m_ScPrefs = c.m_ScPrefs;
	m_Stance = c.m_Stance;
	m_TaxIncome = c.m_TaxIncome;
	m_TaxSales = c.m_TaxSales;
	m_TaxSubsidy = c.m_TaxSubsidy;

	for(int tc=0; tc<AuScFace::THEME_Count; tc++)
		m_ThemeColors[tc] = c.m_ThemeColors[tc];

	m_capitol = c.m_capitol;
	m_capitolSpecies = c.m_capitolSpecies;
	m_flag = c.m_flag;
	m_id = c.m_id;
	m_immig = c.m_immig;
	m_immigPassCrypt = c.m_immigPassCrypt;
	m_name = c.m_name;
	m_WarCriteria = c.m_WarCriteria;

	return *this;
}

void DgEmpire::Insert(QSqlDatabase& db)
{
	Q_ASSERT(m_id != 0);

	// NOTE: The 'capturemsg' field is now used for the join message.
	QSqlQuery qa(QString(), db);
	QString sq("INSERT INTO g_Empire (id,name,announcements,immig,immigPassCrypt,flag,capitalCity,capitalDNA,capitalSpec,"
			"rewCapBase,rewCapCapital,rewCapCapitalGalaxy,rewCapCapitalSector,rewCapCity,"
			"rewCapSpaceport,rewCapSpaceship,rewCapStarship,rewDmgSpaceport,"
			"rewDmgSpaceship,rewDmgStarShip,rewDiscHabitable,rewDiscWorm,"
			"rewKillEnemy,rewSalary,rewSpotCity,rewSpotEnemy,taxIncome,"
			"taxSales,taxSubsidy,defaultStance,stance,theme,commerce,citizens,cities, capturemsg,"
			"contrailHue,contrailSat,role,bdprefs,scprefs,online) VALUES "
			"(?,?,?,?,?,?,?,?,?,"
			"?,?,?,?,?,"
			"?,?,?,?,"
			"?,?,?,?,"
			"?,?,?,?,?,"
			"?,?,?,?,?,?,?,?, ?,"
			"?,?,?,?,?,?);");
	qa.prepare(sq);
	qa.addBindValue(m_id);
	qa.addBindValue(m_name);
	qa.addBindValue(m_Announcements);
	qa.addBindValue(m_immig);
	qa.addBindValue(m_immigPassCrypt);
	{
		QByteArray ba;
		QDataStream ds(&ba, QIODevice::WriteOnly);
		ds.setVersion(QDataStream::Qt_4_1);
		ds << m_flag;
		QByteArray bcomp = qCompress(ba);
		qa.addBindValue(bcomp);
	}
	qa.addBindValue(m_capitol.GetID());

	// Species
	QByteArray bdna;
	qa.addBindValue(bdna);	// Clear the old capitalDNA field.
	qa.addBindValue(m_capitolSpecies.Key());

	qa.addBindValue(m_RewCapBase);
	qa.addBindValue(m_RewCapCapitalEmpire);
	qa.addBindValue(m_RewCapCapitalGalaxy);
	qa.addBindValue(m_RewCapCapitalSector);
	qa.addBindValue(m_RewCapCity);
	qa.addBindValue(m_RewCapStation);
	qa.addBindValue(m_RewCapSpaceship);
	qa.addBindValue(m_RewCapStarship);
	qa.addBindValue(m_RewDmgStation);
	qa.addBindValue(m_RewDmgSpaceship);
	qa.addBindValue(m_RewDmgStarship);
	qa.addBindValue(m_RewDiscHabitable);
	qa.addBindValue(m_RewDiscWorm);
	qa.addBindValue(m_RewKillEnemy);
	qa.addBindValue(m_RewSalary);
	qa.addBindValue(m_RewSpotCity);
	qa.addBindValue(m_RewSpotEnemy);
	qa.addBindValue(m_TaxIncome);
	qa.addBindValue(m_TaxSales);
	qa.addBindValue(m_TaxSubsidy);
	qa.addBindValue(m_DefaultStance);
	{
		QByteArray ba;
		QDataStream ds(&ba, QIODevice::WriteOnly);
		ds.setVersion(QDataStream::Qt_4_1);
		quint16 count = (quint16)m_Stance.size();
		ds << count;
		for(unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::const_iterator it = m_Stance.begin(); it != m_Stance.end(); it++)
			(*it).second >> ds;
		ds << m_WarCriteria;
		ds << m_Patents;
		quint8 ver = 3;
		ds << ver;
		QByteArray bcomp = qCompress(ba);
		qa.addBindValue(bcomp);
	}

	{
		QByteArray ba;
		QDataStream ds(&ba, QIODevice::WriteOnly);
		ds.setVersion(QDataStream::Qt_4_1);

		ds << (quint8)AuScFace::THEME_Count;
		for(int tc=0; tc<AuScFace::THEME_Count; tc++)
			ds << m_ThemeColors[tc];

		qa.addBindValue(ba);
	}

	{
		QByteArray ba;
		QDataStream ds(&ba, QIODevice::WriteOnly);
		ds.setVersion(QDataStream::Qt_4_1);
		ds << m_Commerce;

		QByteArray bcomp = qCompress(ba);
		qa.addBindValue(bcomp);
	}

	// Citizens
	{
		QByteArray ba;
		QDataStream ds(&ba, QIODevice::WriteOnly);
		ds.setVersion(QDataStream::Qt_4_1);
		quint16 count = (quint16)m_Citizens.size();
		ds << count;
		for(unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator it = m_Citizens.begin(); it != m_Citizens.end(); it++)
			(*it).second >> ds;
		QByteArray bcomp = qCompress(ba);
		qa.addBindValue(bcomp);
	}
	// Cities
	{
		QByteArray ba;
		QDataStream ds(&ba, QIODevice::WriteOnly);
		ds.setVersion(QDataStream::Qt_4_1);
		ds << *m_Cities;
		QByteArray bcomp = qCompress(ba);
		qa.addBindValue(bcomp);
	}
	qa.addBindValue(m_JoinMessage);
	qa.addBindValue(m_ContrailHue);
	qa.addBindValue(m_ContrailSat);
	qa.addBindValue(m_GameRole);

	// Bd blueprint preferences
	{
		QByteArray ba;
		QDataStream ds(&ba, QIODevice::WriteOnly);
		ds.setVersion(QDataStream::Qt_4_1);
		ds << m_BdPrefs;
		QByteArray bcomp = qCompress(ba);
		qa.addBindValue(bcomp);
	}

	// Sc blueprint preferences
	{
		QByteArray ba;
		QDataStream ds(&ba, QIODevice::WriteOnly);
		ds.setVersion(QDataStream::Qt_4_1);
		ds << m_ScPrefs;
		QByteArray bcomp = qCompress(ba);
		qa.addBindValue(bcomp);
	}

	qa.addBindValue(m_OnLine.UTC());

	if(!qa.exec())
	{
		// Execution of the query failed. Report the error.
		QSqlError err = qa.lastError();
		qWarning() << "DgEmpire::Insert" << m_id << m_name << "FAILED" << sq << "err" << err.text();
	}
}

void DgEmpire::Update(QSqlDatabase& db)
{
	Q_ASSERT(m_id != 0);

	// NOTE: The 'capturemsg' field is now used for the join message.
	QSqlQuery qa(QString(), db);
	QString sq = QString("UPDATE g_Empire SET name=?, announcements=?, immig=?, immigPassCrypt=?, flag=?, capitalCity=?, "
			"capitalDNA=?, capitalSpec=?, rewCapBase=?, "
			"rewCapCapital=?, rewCapCapitalGalaxy=?, rewCapCapitalSector=?, rewCapCity=?, rewCapSpaceport=?, "
			"rewCapSpaceship=?, rewCapStarship=?, rewDmgSpaceport=?, "
			"rewDmgSpaceship=?, rewDmgStarShip=?, rewDiscHabitable=?, "
			"rewDiscWorm=?, rewKillEnemy=?, rewSalary=?, rewSpotCity=?, "
			"rewSpotEnemy=?, taxIncome=?, taxSales=?, taxSubsidy=?, "
			"defaultStance=?, stance=?, theme=?, commerce=?, "
			"citizens=?, cities=?, capturemsg=?, contrailHue=?, "
			"contrailSat=?, role=?, bdprefs=?, scprefs=?, online=? WHERE id=%2;")
		.arg(m_id);
	qa.prepare(sq);
	qa.addBindValue(m_name);
	qa.addBindValue(m_Announcements);
	qa.addBindValue(m_immig);
	qa.addBindValue(m_immigPassCrypt);
	{
		QByteArray ba;
		QDataStream ds(&ba, QIODevice::WriteOnly);
		ds.setVersion(QDataStream::Qt_4_1);
		ds << m_flag;
		QByteArray bcomp = qCompress(ba);
		qa.addBindValue(bcomp);
	}
	qa.addBindValue(m_capitol.GetID());

	// Species
	QByteArray bdna;
	qa.addBindValue(bdna);	// Clear the old capitalDNA field.
	qa.addBindValue(m_capitolSpecies.Key());

	qa.addBindValue(m_RewCapBase);
	qa.addBindValue(m_RewCapCapitalEmpire);
	qa.addBindValue(m_RewCapCapitalGalaxy);
	qa.addBindValue(m_RewCapCapitalSector);
	qa.addBindValue(m_RewCapCity);
	qa.addBindValue(m_RewCapStation);
	qa.addBindValue(m_RewCapSpaceship);
	qa.addBindValue(m_RewCapStarship);
	qa.addBindValue(m_RewDmgStation);
	qa.addBindValue(m_RewDmgSpaceship);
	qa.addBindValue(m_RewDmgStarship);
	qa.addBindValue(m_RewDiscHabitable);
	qa.addBindValue(m_RewDiscWorm);
	qa.addBindValue(m_RewKillEnemy);
	qa.addBindValue(m_RewSalary);
	qa.addBindValue(m_RewSpotCity);
	qa.addBindValue(m_RewSpotEnemy);
	qa.addBindValue(m_TaxIncome);
	qa.addBindValue(m_TaxSales);
	qa.addBindValue(m_TaxSubsidy);
	qa.addBindValue(m_DefaultStance);
	{
		QByteArray ba;
		QDataStream ds(&ba, QIODevice::WriteOnly);
		ds.setVersion(QDataStream::Qt_4_1);
		quint16 count = (quint16)m_Stance.size();
		ds << count;
		for(unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::const_iterator it = m_Stance.begin(); it != m_Stance.end(); it++)
			(*it).second >> ds;
		ds << m_WarCriteria;
		ds << m_Patents;
		quint8 ver = 3;
		ds << ver;
		QByteArray bcomp = qCompress(ba);
		qa.addBindValue(bcomp);
	}

	{
		QByteArray ba;
		QDataStream ds(&ba, QIODevice::WriteOnly);
		ds.setVersion(QDataStream::Qt_4_1);

		ds << (quint8)AuScFace::THEME_Count;
		for(int tc=0; tc<AuScFace::THEME_Count; tc++)
			ds << m_ThemeColors[tc];

		qa.addBindValue(ba);
	}

	{
		QByteArray ba;
		QDataStream ds(&ba, QIODevice::WriteOnly);
		ds.setVersion(QDataStream::Qt_4_1);
		ds << m_Commerce;

		QByteArray bcomp = qCompress(ba);
		qa.addBindValue(bcomp);
	}

	// Citizens
	{
		QByteArray ba;
		QDataStream ds(&ba, QIODevice::WriteOnly);
		ds.setVersion(QDataStream::Qt_4_1);
		quint16 count = (quint16)m_Citizens.size();
		//qDebug("DgEmpire::Update Citizens count=%d", count);
		ds << count;
		for(unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator it = m_Citizens.begin(); it != m_Citizens.end(); it++)
		{
			//qDebug("DgEmpire::Update Citizen %s=%d", (const char*)(*it).m_name.toAscii(),(*it).Office());
			(*it).second >> ds;
		}
		QByteArray bcomp = qCompress(ba);
		qa.addBindValue(bcomp);
	}
	// Cities
	{
		QByteArray ba;
		QDataStream ds(&ba, QIODevice::WriteOnly);
		ds.setVersion(QDataStream::Qt_4_1);
		ds << *m_Cities;
		QByteArray bcomp = qCompress(ba);
		qa.addBindValue(bcomp);
	}
	qa.addBindValue(m_JoinMessage);
	qa.addBindValue(m_ContrailHue);
	qa.addBindValue(m_ContrailSat);
	qa.addBindValue(m_GameRole);

	// Bd blueprint preferences
	{
		QByteArray ba;
		QDataStream ds(&ba, QIODevice::WriteOnly);
		ds.setVersion(QDataStream::Qt_4_1);
		ds << m_BdPrefs;
		QByteArray bcomp = qCompress(ba);
		qa.addBindValue(bcomp);
	}

	// Sc blueprint preferences
	{
		QByteArray ba;
		QDataStream ds(&ba, QIODevice::WriteOnly);
		ds.setVersion(QDataStream::Qt_4_1);
		ds << m_ScPrefs;
		QByteArray bcomp = qCompress(ba);
		qa.addBindValue(bcomp);
	}

	qa.addBindValue(m_OnLine.UTC());

	if(!qa.exec())
	{
		// Execution of the query failed. Report the error.
		QSqlError err = qa.lastError();
		qWarning() << "DgEmpire::Update" << m_id << m_name << "FAILED" << sq << "err" << err.text();
	}
}

void DgEmpire::Tx(QDataStream& out, bool includeAnnouncements) const
{
	out << m_id;
	out << m_name;
	if(includeAnnouncements)
		out << m_Announcements;
	else
		out << QString("");
	out << m_immig;
	out << m_immigPassCrypt;
	out << m_flag;
	// client doesn't need capital city id
	out << m_RewCapBase;
	out << m_RewCapCapitalEmpire;
	out << m_RewCapCapitalSector;
	out << m_RewCapCity;
	out << m_RewCapStation;
	out << m_RewCapSpaceship;
	out << m_RewCapStarship;
	out << m_RewDmgStation;
	out << m_RewDmgSpaceship;
	out << m_RewDmgStarship;
	out << m_RewDiscHabitable;
	out << m_RewDiscWorm;
	out << m_RewKillEnemy;
	out << m_RewSalary;
	out << m_RewSpotCity;
	out << m_RewSpotEnemy;
	out << m_TaxIncome;
	out << m_TaxSales;
	out << m_TaxSubsidy;
	out << m_DefaultStance;
	{
		QByteArray ba;
		QDataStream ds(&ba, QIODevice::WriteOnly);
		ds.setVersion(QDataStream::Qt_4_1);
		quint16 count = (quint16)m_Stance.size();
		ds << count;
		for(unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::const_iterator it = m_Stance.begin(); it != m_Stance.end(); it++)
		{
			// Dossier's are too big to send them for every empire.
			// Only dossier of the empire receiving announcements is sent.
			if(includeAnnouncements)
				(*it).second >> ds;
			else
				(*it).second.TxNoDossier(ds);
		}

		// Client does not need war criteria for all empires, just the scene client's empire, so they can manage it.
		if(includeAnnouncements)
		{
			ds << m_WarCriteria;
			ds << m_Patents;
		}
		else
		{
			QHash<quint8, quint8> noWarCrit;
			ds << noWarCrit;
			AuKnownPatents pat;
			ds << pat;
		}
		QByteArray bcomp = qCompress(ba);
		out << bcomp;
	}

	for(int tc=0; tc<AuScFace::THEME_Count; tc++)
		out << m_ThemeColors[tc];

	{
		QByteArray ba;
		QDataStream ds(&ba, QIODevice::WriteOnly);
		ds.setVersion(QDataStream::Qt_4_1);
		ds << m_Commerce;

		QByteArray bcomp = qCompress(ba);
		out << bcomp;
	}
	// Citizens
	{
		QByteArray ba;
		QDataStream ds(&ba, QIODevice::WriteOnly);
		ds.setVersion(QDataStream::Qt_4_1);
		quint16 count = (quint16)m_Citizens.size();
		ds << count;
		for(unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator it = m_Citizens.begin(); it != m_Citizens.end(); it++)
			(*it).second >> ds;
		QByteArray bcomp = qCompress(ba);
		out << bcomp;
	}
	// Cities
	{
		QByteArray ba;
		QDataStream ds(&ba, QIODevice::WriteOnly);
		ds.setVersion(QDataStream::Qt_4_1);
		ds << *m_Cities;
		QByteArray bcomp = qCompress(ba);
		out << bcomp;
	}
	m_capitol >> out;
	out << m_capitolSpecies;
	out << m_ContrailHue;
	out << m_ContrailSat;
	//m_GameRole			// Game role not sent to client.
	out << m_OnLine;
	out << m_JoinMessage;
	out << m_RewCapCapitalGalaxy;

	// Blueprint preferences.
	{
		QByteArray ba;
		QDataStream ds(&ba, QIODevice::WriteOnly);
		ds.setVersion(QDataStream::Qt_4_1);
		ds << m_BdPrefs;
		ds << m_ScPrefs;
		QByteArray bcomp = qCompress(ba);
		out << bcomp;
	}
}

void DgEmpire::Rx(QDataStream& in)
{
	in >> m_id;
	in >> m_name;
	in >> m_Announcements;
	in >> m_immig;
	in >> m_immigPassCrypt;
	in >> m_flag;
	// client doesn't need capital city id
	in >> m_RewCapBase;
	in >> m_RewCapCapitalEmpire;
	in >> m_RewCapCapitalSector;
	in >> m_RewCapCity;
	in >> m_RewCapStation;
	in >> m_RewCapSpaceship;
	in >> m_RewCapStarship;
	in >> m_RewDmgStation;
	in >> m_RewDmgSpaceship;
	in >> m_RewDmgStarship;
	in >> m_RewDiscHabitable;
	in >> m_RewDiscWorm;
	in >> m_RewKillEnemy;
	in >> m_RewSalary;
	in >> m_RewSpotCity;
	in >> m_RewSpotEnemy;
	in >> m_TaxIncome;
	in >> m_TaxSales;
	in >> m_TaxSubsidy;
	in >> m_DefaultStance;
	{
		QByteArray bcomp;
		in >> bcomp;
		if(!bcomp.isEmpty())
		{
			QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,QString("%1").arg(m_id));
			QDataStream ds(&ba, QIODevice::ReadOnly);
			ds.setVersion(QDataStream::Qt_4_1);
			m_Stance.clear();
			quint16 count;
			ds >> count;
			for(;count ;count--)
			{
				AuKnownEmpire ke;
				ke << ds;
				m_Stance.insert(pair<quint32, AuKnownEmpire>(ke.m_id, ke));
			}

			ds >> m_WarCriteria;
			ds >> m_Patents;
		}
	}

	for(int tc=0; tc<AuScFace::THEME_Count; tc++)
		in >> m_ThemeColors[tc];

	{
		QByteArray bcomp;
		in >> bcomp;
		if(!bcomp.isEmpty())
		{
			QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,QString("%1").arg(m_id));
			QDataStream ds(&ba, QIODevice::ReadOnly);
			ds.setVersion(QDataStream::Qt_4_1);
			ds >> m_Commerce;
		}
		while(m_Commerce.size() < CID_Count)
			m_Commerce.append(EXPOL_ToNeutral);	// Grow commerce array to include nu commodities that have been added.
	}
	// Citizens
	{
		QByteArray bcomp;
		in >> bcomp;
		if(!bcomp.isEmpty())
		{
			QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,QString("%1").arg(m_id));
			QDataStream ds(&ba, QIODevice::ReadOnly);
			ds.setVersion(QDataStream::Qt_4_1);
			m_Citizens.clear();
			quint16 count;
			ds >> count;
			for(;count ;count--)
			{
				AuCitizen ke;
				ke << ds;
				m_Citizens.insert(pair<AuId, AuCitizen>(ke.m_id, ke));
			}
		}
	}
	// Cities
	{
		QByteArray bcomp;
		in >> bcomp;
		if(!bcomp.isEmpty())
		{
			QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,QString("%1").arg(m_id));
			QDataStream ds(&ba, QIODevice::ReadOnly);
			ds.setVersion(QDataStream::Qt_4_1);
			ds >> *m_Cities;
		}
	}
	m_capitol << in;
	in >> m_capitolSpecies;
	in >> m_ContrailHue;
	in >> m_ContrailSat;
	//m_GameRole			// Game role not sent to client.
	in >> m_OnLine;
	in >> m_JoinMessage;
	in >> m_RewCapCapitalGalaxy;

	// Blueprint preferences.
	{
		QByteArray bcomp;
		in >> bcomp;
		if(!bcomp.isEmpty())
		{
			QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,QString("%1").arg(m_id));
			QDataStream ds(&ba, QIODevice::ReadOnly);
			ds.setVersion(QDataStream::Qt_4_1);
			ds >> m_BdPrefs;
			ds >> m_ScPrefs;
		}
	}
}

bool DgEmpire::Select(QSqlDatabase& db, unsigned int id)
{
	// NOTE: The 'capturemsg' field is now used for the join message.
	QString sq;
	sq = QString("SELECT name, announcements, immig, immigPassCrypt, flag, capitalCity, capitalDNA,capitalSpec,"
			"rewCapBase, rewCapCapital, rewCapCapitalGalaxy, rewCapCapitalSector, rewCapCity, "
			"rewCapSpaceport, rewCapSpaceship, rewCapStarship, rewDmgSpaceport, rewDmgSpaceship, rewDmgStarShip, rewDiscHabitable, "
			"rewDiscWorm, rewKillEnemy, rewSalary, rewSpotCity, rewSpotEnemy, taxIncome, taxSales, taxSubsidy, "
			"defaultStance, stance, theme, commerce, citizens, cities, capturemsg, "
			"contrailHue, contrailSat, role, bdprefs, scprefs, online "
			"FROM g_Empire WHERE id=%1;").arg(id);
	QSqlQuery qry(sq, db);
	if(qry.first())
	{
		int qv=0;
		m_id = id;
		m_name = qry.value(qv++).toString();
		m_Announcements = qry.value(qv++).toString();
		m_immig = qry.value(qv++).toInt();
		m_immigPassCrypt = qry.value(qv++).toByteArray();
		{
			QByteArray bcomp = qry.value(qv++).toByteArray();
			if(!bcomp.isEmpty())
			{
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,QString("%1").arg(m_id));
				QDataStream ds(&ba, QIODevice::ReadOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				ds >> m_flag;
			}
		}
		m_capitol = AuId(qry.value(qv++).toUInt());

		// Species
		QByteArray bdna = qry.value(qv++).toByteArray();
		m_capitolSpecies = AuSpecies(qry.value(qv++).toByteArray());
		if(!bdna.isEmpty())
		{
			// Load old style DNA.
			DhDNA_ dna3(bdna);
			m_capitolSpecies = DhDNA(dna3).Species();	// Override capital species with old dna
		}

		m_RewCapBase = qry.value(qv++).toInt();
		m_RewCapCapitalEmpire = qry.value(qv++).toInt();
		m_RewCapCapitalGalaxy = qry.value(qv++).toInt();
		m_RewCapCapitalSector = qry.value(qv++).toInt();
		m_RewCapCity = qry.value(qv++).toInt();
		m_RewCapStation = qry.value(qv++).toInt();
		m_RewCapSpaceship = qry.value(qv++).toInt();
		m_RewCapStarship = qry.value(qv++).toInt();
		m_RewDmgStation = qry.value(qv++).toInt();
		m_RewDmgSpaceship = qry.value(qv++).toInt();
		m_RewDmgStarship = qry.value(qv++).toInt();
		m_RewDiscHabitable = qry.value(qv++).toInt();
		m_RewDiscWorm = qry.value(qv++).toInt();
		m_RewKillEnemy = qry.value(qv++).toInt();
		m_RewSalary = qry.value(qv++).toInt();
		m_RewSpotCity = qry.value(qv++).toInt();
		m_RewSpotEnemy = qry.value(qv++).toInt();
		m_TaxIncome = qry.value(qv++).toInt();
		m_TaxSales = qry.value(qv++).toInt();
		m_TaxSubsidy = qry.value(qv++).toInt();
		m_DefaultStance = qry.value(qv++).toInt();
		{
			QByteArray bcomp = qry.value(qv++).toByteArray();
			if(!bcomp.isEmpty())
			{
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,QString("%1").arg(m_id));
				QDataStream ds(&ba, QIODevice::ReadOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				m_Stance.clear();
				quint16 count;
				ds >> count;
				for(;count ;count--)
				{
					AuKnownEmpire ke;
					ke << ds;

					ke.InitDossier();

					m_Stance.insert(pair<quint32, AuKnownEmpire>(ke.m_id, ke));
				}

				ds >> m_WarCriteria;
				ds >> m_Patents;
				quint8 ver;
				ds >> ver;
				if(ver == 2)
				{
					AuKnownPatents p;
					ds >> p;
					ds >> p;
					ds >> p;
				}
				else if(ver == 1)
				{
					AuKnownPatents p;
					ds >> p;
				}
			}
		}

		{
			QByteArray ba = qry.value(qv++).toByteArray();
			if(!ba.isEmpty())
			{
				QDataStream ds(&ba, QIODevice::ReadOnly);
				ds.setVersion(QDataStream::Qt_4_1);

				quint8 c;
				ds >> c;
				for(int tc=0; tc<c; tc++)
					ds >> m_ThemeColors[tc];
			}
		}

		{
			QByteArray bcomp = qry.value(qv++).toByteArray();
			if(!bcomp.isEmpty())
			{
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,QString("%1").arg(m_id));
				QDataStream ds(&ba, QIODevice::ReadOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				ds >> m_Commerce;
			}
			while(m_Commerce.size() < CID_Count)
				m_Commerce.append(EXPOL_ToNeutral);	// Grow commerce array to include nu commodities that have been added.
		}

		// Citizens
		{
			QByteArray bcomp = qry.value(qv++).toByteArray();
			if(!bcomp.isEmpty())
			{
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,QString("%1").arg(m_id));
				QDataStream ds(&ba, QIODevice::ReadOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				m_Citizens.clear();
				quint16 count;
				ds >> count;
				//qDebug("DgEmpire::Select Citizens count=%d",count);
				for(;count ;count--)
				{
					AuCitizen ke;
					ke << ds;
					m_Citizens.insert(pair<AuId, AuCitizen>(ke.m_id, ke));
				}
			}
		}
		// Cities
		{
			QByteArray bcomp = qry.value(qv++).toByteArray();
			if(!bcomp.isEmpty())
			{
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,QString("%1").arg(m_id));
				QDataStream ds(&ba, QIODevice::ReadOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				ds >> *m_Cities;
			}
		}
		m_JoinMessage = qry.value(qv++).toString();	// captureMsg field
		m_ContrailHue = qry.value(qv++).toInt();
		m_ContrailSat = qry.value(qv++).toInt();
		m_GameRole = (EMPIRE_ROLE)qry.value(qv++).toInt();

		// Bd blueprint preferences.
		{
			QByteArray bcomp = qry.value(qv++).toByteArray();
			if(!bcomp.isEmpty())
			{
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,QString("%1").arg(m_id));
				QDataStream ds(&ba, QIODevice::ReadOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				ds >> m_BdPrefs;
			}
		}

		// Sc blueprint preferences.
		{
			QByteArray bcomp = qry.value(qv++).toByteArray();
			if(!bcomp.isEmpty())
			{
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,QString("%1").arg(m_id));
				QDataStream ds(&ba, QIODevice::ReadOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				ds >> m_ScPrefs;
			}
		}

		QDateTime online = qry.value(qv++).toDateTime();
		online.setTimeSpec(Qt::UTC);
		m_OnLine = online;
		return true;
	}
	else
		return false;
}

// Returns Name() with special characters substituted, e.g. '&' becomes '&amp;'.
QString DgEmpire::HtmlName() const
{
	// Array of characters that are forbidden in html text.
	const char badChars[] =
	{
		'&',
		'"',
		'<',
		'>',
	};
	int badCharCount = sizeof(badChars)/sizeof(const char);

	// Array of encodings that correspond to the forbidden characters.
	const char* encChars[] =
	{
		"&amp;",
		"&quot;",
		"&lt;",
		"&gt;",
	};
	Q_ASSERT(sizeof(encChars)/sizeof(const char*) == badCharCount);	// Both arrays must have the same number of entries in each.

	// Replace the forbidden characters with their corresponding html encodings.
	QString name = Name();
	for(int reChar=0; reChar<badCharCount; reChar++)
		name.replace(badChars[reChar], encChars[reChar]);
	return name;
}

void DgEmpire::operator>>(QDataStream& out) const
{
	out << m_id;					// Database record id.
	out << m_name;					// Name of empire.
	out << m_Announcements;

	int ver = 2;
	quint8 verImmig = (ver << 4) | m_immig;	// Top nibble is version.

	out << verImmig;				// 0=open, 1=restricted
	out << m_flag;					// Flag image for emperors
	out << m_capitol;				// Id of city designated as the capital.
	out << m_capitolSpecies;			// Worker species at the capital city.
	out << m_RewCapBase;
	out << m_RewCapCapitalEmpire;
	out << m_RewCapCapitalSector;
	out << m_RewCapCity;
	out << m_RewCapStation;		// per 1000 structure points
	out << m_RewCapSpaceship;		// per 1000 structure points
	out << m_RewCapStarship;		// per 1000 structure points
	out << m_RewDmgStation;		// per 10 damage points inflicted
	out << m_RewDmgSpaceship;		// per 10 damage points inflicted
	out << m_RewDmgStarship;		// per 10 damage points inflicted
	out << m_RewDiscHabitable;		// per habitable world
	out << m_RewDiscWorm;
	out << m_RewKillEnemy;
	out << m_RewSalary;			// for govt officials
	out << m_RewSpotCity;
	out << m_RewSpotEnemy;
	out << m_TaxIncome;			// 1/2 percent increments
	out << m_TaxSales;				// 1/2 percent increments
	out << m_TaxSubsidy;			// 1/2 percent increments
	out << m_DefaultStance;		// 0=friend, 1=neutral, 2=enemy. POL_ values. Cannot be POL_Empire.

	out << (quint16)m_Stance.size();
	for(unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::const_iterator ki = m_Stance.begin(); ki != m_Stance.end(); ki++)
		(*ki).second >> out;

	for(int tc=0; tc<AuScFace::THEME_Count; tc++)
		out << m_ThemeColors[tc];

	out << (quint16)m_Commerce.size();
	for(QVector<quint8>::const_iterator ci = m_Commerce.begin(); ci != m_Commerce.end(); ci++)
		out << *ci;

	out << m_JoinMessage;

	out << (quint16)m_Citizens.size();
	for(unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.begin(); ci != m_Citizens.end(); ci++)
		(*ci).second >> out;

	out << *m_Cities;
	out << m_ContrailHue;			// Hue of contrails of spacecraft and vehicles. 0-255
	out << m_ContrailSat;			// Saturation of contrails of spacecraft and vehicles. 64-191
	out << (quint8)m_GameRole;				// 0=normal empire, 1=pirate empire, 2=admin, 3=arena
	out << m_WarCriteria;
	out << m_Patents;
	out << m_RewCapCapitalGalaxy;
	out << m_BdPrefs;
	out << m_ScPrefs;
	out << m_OnLine;
	out << m_immigPassCrypt;
}

void DgEmpire::operator<<(QDataStream& in)
{
	in >> m_id;					// Database record id.
	in >> m_name;					// Name of empire.
	in >> m_Announcements;

	quint8 verImmig;
	in >> verImmig;

	m_immig = verImmig & 0x0f;		// 0=open, 1=restricted
	int ver = verImmig >> 4;		// Top nibble is version.

	in >> m_flag;					// Flag image for emperors
	in >> m_capitol;				// Id of city designated as the capital.
	in >> m_capitolSpecies;			// Worker species at the capital city.
	in >> m_RewCapBase;
	in >> m_RewCapCapitalEmpire;
	in >> m_RewCapCapitalSector;
	in >> m_RewCapCity;
	in >> m_RewCapStation;		// per 1000 structure points
	in >> m_RewCapSpaceship;		// per 1000 structure points
	in >> m_RewCapStarship;		// per 1000 structure points
	in >> m_RewDmgStation;		// per 10 damage points inflicted
	in >> m_RewDmgSpaceship;		// per 10 damage points inflicted
	in >> m_RewDmgStarship;		// per 10 damage points inflicted
	in >> m_RewDiscHabitable;		// per habitable world
	in >> m_RewDiscWorm;
	in >> m_RewKillEnemy;
	in >> m_RewSalary;			// for govt officials
	in >> m_RewSpotCity;
	in >> m_RewSpotEnemy;
	in >> m_TaxIncome;			// 1/2 percent increments
	in >> m_TaxSales;				// 1/2 percent increments
	in >> m_TaxSubsidy;			// 1/2 percent increments
	in >> m_DefaultStance;		// 0=friend, 1=neutral, 2=enemy. POL_ values. Cannot be POL_Empire.

	m_Stance.clear();
	quint16 incount;
	in >> incount;
	while(incount--)
	{
		AuKnownEmpire ke;
		ke << in;
		m_Stance.insert(pair<quint32, AuKnownEmpire>(ke.m_id, ke));
	}

	for(int tc=0; tc<AuScFace::THEME_Count; tc++)
		in >> m_ThemeColors[tc];

	m_Commerce.clear();
	in >> incount;
	while(incount--)
	{
		quint8 com;
		in >> com;
		m_Commerce.append(com);
	}
	while(m_Commerce.size() < CID_Count)
		m_Commerce.append(EXPOL_ToNeutral);	// Grow array to include nu commodities that have been added.

	in >> m_JoinMessage;

	m_Citizens.clear();
	in >> incount;
	while(incount--)
	{
		AuCitizen cit;
		cit << in;
		m_Citizens.insert(pair<AuId, AuCitizen>(cit.m_id, cit));
	}

	in >> *m_Cities;
	in >> m_ContrailHue;			// Hue of contrails of spacecraft and vehicles. 0-255
	in >> m_ContrailSat;			// Saturation of contrails of spacecraft and vehicles. 64-191
	quint8 ui8;
	in >> ui8;
	m_GameRole = (EMPIRE_ROLE)ui8;
	in >> m_WarCriteria;
	in >> m_Patents;

	if(ver == 0)
	{
		// Old version stored more patent lists.
		AuKnownPatents p;
		in >> p;
		in >> p;
		in >> p;
	}
	in >> m_RewCapCapitalGalaxy;
	in >> m_BdPrefs;
	in >> m_ScPrefs;
	if(ver >= 1)
		in >> m_OnLine;
	if(ver >= 2)
		in >> m_immigPassCrypt;
}

bool DgEmpire::ReconcileCities(const unordered_map<AuId, quint32, AuIdHash>& allCities)
{
	// Return value will be set true if this empire object changes.
	bool anyChange = false;

	// Clear capital city if it no longer exists or if it no longer belongs to this empire.
	if(!m_capitol.IsNull())
	{
		unordered_map<AuId, quint32, AuIdHash>::const_iterator aci = allCities.find(m_capitol);
		if(aci == allCities.end() || (*aci).second != m_id)
		{
			// Capital city does not exist or it no longer belongs to this empire.
			if(IsPirate())
				qDebug() << "  DgEmpire::ReconcileCities Capital city does not exist or it no longer belongs to this empire."
					<< m_id << Name();
			m_capitol = AuId(0);
			anyChange = true;
		}
	}

	// Remove cities from my list that no longer exist or that no longer belong to this empire.
	if(m_Cities->Reconcile(m_id, allCities))
	{
		if(IsPirate())
			qDebug() << "  DgEmpire::ReconcileCities cities changed" << m_id << Name();
		anyChange = true;
	}

	// Return true if anything changed.
	return anyChange;
}

// Returns true if the 'avatarId' citizen is removed.
bool DgEmpire::RemoveCitizen(const AuId& avatarId)
{
	unordered_map<AuId, AuCitizen, AuIdHash>::iterator ci = m_Citizens.find(avatarId);
	if(ci == m_Citizens.end())
		return false;

	// Revoke their office and reassign subordinates, choose nu emperor as necessary.
	if((*ci).second.HasOffice())
		CitizenRevokeOffice(avatarId);

	// Remove the citizen.
	m_Citizens.erase(ci);
	return true;
}

// Revokes the citizen's office entirely.
void DgEmpire::CitizenRevokeOffice(const AuId& avatarId)
{
	unordered_map<AuId, AuCitizen, AuIdHash>::iterator ci = m_Citizens.find(avatarId);
	if(ci == m_Citizens.end())
		return;
	if(!(*ci).second.HasOffice())
		return;

	if((*ci).second.IsEmperor())
	{
		// Select a nu emperor when removing the emperor. This also reassigns subordinates.
		SelectNewEmperor((*ci).first);
	}
	else if((*ci).second.HasOffice())
	{
		// When revoking an office from a citizen, we must reassign their subordinates to their boss.
		AuId bossId = (*ci).second.BossId();
		for(unordered_map<AuId, AuCitizen, AuIdHash>::iterator si = m_Citizens.begin(); si != m_Citizens.end(); si++)
			if((*si).second.BossId() == avatarId)
				(*si).second.SetBossId(bossId);
	}

	(*ci).second.RevokeOffice();
}

void DgEmpire::SelectNewEmperor(const AuId& prevEmperor)
{
	// Get old emperor's title to pass on to the nu emperor.
	QString title;
	unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(prevEmperor);
	if(ci != m_Citizens.end())
	{
		title = (*ci).second.TitleOfOffice();
		if(m_Citizens.size() < 1)
			return;	// Nobody else to assign.
	}
	else
	{
		title = QObject::tr("Emperor");
		if(m_Citizens.empty())
			return;	// Nobody to assign.
	}

	// Assign a nu citizen to be the emperor.
	unordered_map<AuId, AuCitizen, AuIdHash>::iterator besti = m_Citizens.end();
	quint32 bestValue = 0;
	bool anygov = false;				// True if we find any governing official other than the previous emperor.

	if(!prevEmperor.IsNull())
	{
		// There was a previous emperor.
		// Look for anyone directly subordinate to the previous emperor with the highest target value.
		for(unordered_map<AuId, AuCitizen, AuIdHash>::iterator si = m_Citizens.begin(); si != m_Citizens.end(); si++)
		{
			if(!(*si).second.HasOffice())
				continue;
			if((*si).first == prevEmperor)
				continue;
			anygov = true;
			if((*si).second.IsEmperor())
			{
				// Found another emperor!
				besti = si;
				break;
			}
			if((*si).second.BossId() != prevEmperor)
				continue;

			quint32 value = CitizenTargetValue((*si).first);
			if(value > bestValue)
			{
				besti = si;
				bestValue = value;
			}
		}
	}

	if(anygov && besti == m_Citizens.end())
	{
		// Did not find any governor subordinate to the emperor but there are governors. How did this happen?
		// Look for any governor with the best target value.
		for(unordered_map<AuId, AuCitizen, AuIdHash>::iterator si = m_Citizens.begin(); si != m_Citizens.end(); si++)
		{
			if(!(*si).second.HasOffice())
				continue;
			if((*si).first == prevEmperor)
				continue;

			quint32 value = CitizenTargetValue((*si).first);
			if(value > bestValue)
			{
				besti = si;
				bestValue = value;
			}
		}
	}

	if(besti == m_Citizens.end())
	{
		// Did not find any governor.
		// Look for anyone.
		for(unordered_map<AuId, AuCitizen, AuIdHash>::iterator si = m_Citizens.begin(); si != m_Citizens.end(); si++)
		{
			if((*si).first == prevEmperor)
				continue;

			besti = si;
			break;
		}
	}

	if(besti == m_Citizens.end())
		return;

	// Found a citizen to make into the nu emperor.
	AuId nuEmperorId((*besti).first);
	if(!(*besti).second.IsEmperor())
	{
		AuOffice office;
		office.SetupEmperor();
		office.SetTitleOfOffice(title);
		(*besti).second.AssignOffice(office);
	}

	// When assigning a nu emperor, we must reassign the previous emperor's subordinates to the nu emperor.
	// If there was only one direct subordinate to the previous emperor, they became the nu emperor and we don't need to do this.
	if(!prevEmperor.IsNull())
		for(unordered_map<AuId, AuCitizen, AuIdHash>::iterator si = m_Citizens.begin(); si != m_Citizens.end(); si++)
			if((*si).second.BossId() == prevEmperor)
				(*si).second.SetBossId(nuEmperorId);
}

void DgEmpire::ListAllAvatars(QComboBox& cb) const
{
	for(unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.begin(); ci != m_Citizens.end(); ci++)
	{
		QString cname = QString("%1, %2").arg((*ci).second.m_name).arg((*ci).second.m_id.StringId());
		bool inserted = false;
		for(int idx=0; idx<cb.count(); idx++)
		{
			int comp = cname.compare(cb.itemText(idx), Qt::CaseInsensitive);
			if(comp == 0)
				comp = cname.compare(cb.itemText(idx), Qt::CaseSensitive);
			if(comp < 0)
			{
				// Insert the item before the idx item.
				inserted = true;
				cb.insertItem(idx, cname, QVariant((*ci).second.m_id.GetID()));	// Add item with record id.
				break;
			}
		}
		if(!inserted)
		{
			// Append the item at the end.
			cb.addItem(cname, QVariant((*ci).second.m_id.GetID()));	// Add item with record id.
		}
	}
}

// Returns the id of the citizen who appears to be the emperor.
AuId DgEmpire::EmperorId() const
{
	for(unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.begin(); ci != m_Citizens.end(); ci++)
		if((*ci).second.IsEmperor())
			return (*ci).first;
	return AuId::NullId();
}

// 'placeIds' contains the relevant atom ids for domain testing.
// Fills 'avatarIds' with the ids of empire governors who are to receive sensor scans.
void DgEmpire::ReceivesSensorScans(const AuIdList& placeIds, AuIdList& avatarIds) const
{
	AuIdList govIds;
	DomainGovernors(placeIds, govIds);
	for(AuIdList::const_iterator gi = govIds.begin(); gi != govIds.end(); gi++)
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(*gi);
		if(ci == m_Citizens.end())
			continue;
		if((*ci).second.ReceivesSensorScans())
			avatarIds.append((*ci).first);
	}
}

// 'placeIds' contains the relevant atom ids for domain testing.
// Fills 'avatarIds' with the ids of empire governors who are to receive sensor scans.
void DgEmpire::ReceivesFleetMessages(const AuIdList& placeIds, AuIdList& avatarIds) const
{
	AuIdList govIds;
	DomainGovernors(placeIds, govIds);
	for(AuIdList::const_iterator gi = govIds.begin(); gi != govIds.end(); gi++)
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(*gi);
		if(ci == m_Citizens.end())
			continue;
		if((*ci).second.CanReceiveFleetMessages())
			avatarIds.append((*ci).first);
	}
}

quint32 DgEmpire::CitizenTargetValue(const AuId& avatarId) const
{
	quint32 tv = 0;
	for(unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.begin(); ci != m_Citizens.end(); ci++)
		if(CitizenInChainOfCommand(avatarId, (*ci).first))
			tv += (*ci).second.TargetValue();

	// Office of emperor starts at 100. Multiply that by a factor that results in a reasonable number of cronos.
	tv *= 10000;

	return tv;
}

bool DgEmpire::CitizenFilterAssignment(const AuId& avatarId, const AuId& assigneeId, AuOffice& office) const
{
	unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
	if(ci == m_Citizens.end())
		return false;

	// The emperor can grant any rights they want.
	if(!(*ci).second.IsEmperor() && !(*ci).second.FilterAssignment(office))
		return false;

	unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ai = m_Citizens.find(assigneeId);
	if(ai == m_Citizens.end())
		return false;

	// The avatar is the assignee's nu boss, unless the assignee already has a boss.
	if((*ai).second.HasOffice())
		office.SetBossId((*ai).second.BossId());	// Assignee already has an office. Don't change their current boss.
	else
		office.SetBossId(avatarId);			// Assignee does not hold any office yet. The avatar becomes their nu boss.
	return true;
}

AuId DgEmpire::BaseInSystem(const AuId& systemId) const
{
	return m_Cities->BaseInSystem(systemId);
}

AuId DgEmpire::BaseSystemInSector(const AuId& sectorId) const
{
	return m_Cities->BaseSystemInSector(sectorId);
}

bool DgEmpire::HasCityInSector(const AuId& sectorId) const
{
	return m_Cities->AnyInSector(sectorId);
}

bool DgEmpire::HasCityInSystem(const AuId& systemId) const
{
	return m_Cities->AnyInSystem(systemId);
}

// Returns id of a city on 'worldId', if any.
AuId DgEmpire::CityOnWorld(const AuId& worldId) const
{
	return m_Cities->CityOnWorld(worldId);
}

// Must have a capital city in 'galaxy'. Must be a normal empire.
bool DgEmpire::IsJoinableAsOverlord(const AuId& galaxy) const
{
	// Empire cannot become subordinate to someone who will treat them as an enemy right away.
	if(DefaultPoliticalStance() == POL_Enemy)
		return false;		// Default stance is enemy.

	// Empire must have an emperor.
	if(EmperorId().IsNull())
		return false;		// No emperor.

	// Empire must have a capital.
	if(m_capitol.IsNull())
		return false;		// No capital city.

	// Verify capital city is in the galaxy.
	unordered_map<AuId, AuCity, AuIdHash>::const_iterator ci = m_Cities->find(m_capitol);
	if(ci == m_Cities->cend())
		return false;		// No capital city record?
	if((*ci).second.GalaxyId() != galaxy)
		return false;		// Capital city is in a different galaxy.

	// Empire looks ok to be an overlord.
	return true;
}

void DgEmpire::SetThemeColor(AuScFace::COLOR_THEME theme, const QColor& color)
{
	if(theme < 0 || theme >= AuScFace::THEME_Count)
		return;

	QColor col(color);

	// Prevent colors that are too dark.
	qreal h,s,v;
	col.getHsvF(&h, &s, &v);
	if(v < .05f)
		col.setHsvF(h, s, .05f);

	m_ThemeColors[theme] = col;
}

void DgEmpire::SetThemeColors(const QColor themeColors[AuScFace::THEME_Count])
{
	for(int theme=0; theme<AuScFace::THEME_Count; theme++)
		SetThemeColor((AuScFace::COLOR_THEME)theme, themeColors[theme]);
}

void DgEmpire::SetPoliticalStance(unsigned int empire, const QString& name, POLITICAL_STANCE stance, quint8 vassalTributePercent,
		quint8 reason, const AuId& fromUnit, const QString& fromUnitName,
		POLITICAL_STANCE toStance, const AuId& toUnit, const QString& toUnitName,
		GALAXY galaxy, const QString& sector, const QString& system, const QString& world)
{
	Q_ASSERT(stance != POL_Empire);	// This value is invalid as a stance toward another empire.
	Q_ASSERT(empire != m_id);	// Don't call this on itself.

	// Check to see if we already have a stance toward the empire.
	unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::iterator it = m_Stance.find(empire);
	if(it != m_Stance.end())
	{
		(*it).second.m_stance = stance;
		if(vassalTributePercent <= 100)
			(*it).second.m_tribute = vassalTributePercent;
		else
			(*it).second.m_tribute = 100;
		(*it).second.m_whenSet.Set();

		if(reason == AuDocUnitContact::DocumentType())
			(*it).second.m_Dossier.File(AuDocUnitContact(stance, fromUnit, fromUnitName, toStance, toUnit, toUnitName, galaxy,
						sector, system, world));
		else if(reason == AuDocSetStance::DocumentType())
			(*it).second.m_Dossier.File(AuDocSetStance(stance, fromUnit, fromUnitName, toStance, galaxy, sector, system, world));
		else if(reason == AuDocReciprocalStance::DocumentType())
			(*it).second.m_Dossier.File(AuDocReciprocalStance(stance, fromUnit, fromUnitName, toUnit, toUnitName, galaxy, sector,
						system, world));
		else if(reason == AuDocInitOverlord::DocumentType())
			(*it).second.m_Dossier.File(AuDocInitOverlord(fromUnitName, toStance, galaxy, sector));
		else if(reason == AuDocNewOverlord::DocumentType())
			(*it).second.m_Dossier.File(AuDocNewOverlord(stance, fromUnit, fromUnitName, galaxy, sector, system, world));
		else if(reason == AuDocWar::DocumentType())
			(*it).second.m_Dossier.File(AuDocWar());
		else
			qWarning() << "error: DgEmpire::SetPoliticalStance unexpected reason" << reason;

		return;
	}

	// We did not already have a stance toward the empire. Add a nu one.
	AuKnownEmpire ke;
	ke.m_id = empire;
	ke.m_name = name;
	ke.m_stance = stance;
	if(vassalTributePercent <= 100)
		ke.m_tribute = vassalTributePercent;
	else
		ke.m_tribute = 100;
	ke.m_whenMet.Set();
	ke.m_whenSet.Set();

	if(reason == AuDocUnitContact::DocumentType())
		ke.m_Dossier.File(AuDocUnitContact(stance, fromUnit, fromUnitName, toStance, toUnit, toUnitName, galaxy, sector, system,
					world));
	else if(reason == AuDocSetStance::DocumentType())
		ke.m_Dossier.File(AuDocSetStance(stance, fromUnit, fromUnitName, toStance, galaxy, sector, system, world));
	else if(reason == AuDocReciprocalStance::DocumentType())
		ke.m_Dossier.File(AuDocReciprocalStance(stance, fromUnit, fromUnitName, toUnit, toUnitName, galaxy, sector, system, world));
	else if(reason == AuDocInitOverlord::DocumentType())
		ke.m_Dossier.File(AuDocInitOverlord(fromUnitName, toStance, galaxy, sector));
	else if(reason == AuDocNewOverlord::DocumentType())
		ke.m_Dossier.File(AuDocNewOverlord(stance, fromUnit, fromUnitName, galaxy, sector, system, world));
	else
		qWarning() << "error: DgEmpire::SetPoliticalStance unexpected reason" << reason;

	m_Stance.insert(pair<quint32, AuKnownEmpire>(empire, ke));
}

bool DgEmpire::InitDossier()
{
	bool anyChange = false;
	for(unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::iterator it = m_Stance.begin(); it != m_Stance.end(); it++)
		if((*it).second.InitDossier())
			anyChange = true;
	return anyChange;
}

AuTime DgEmpire::NewestNewDossierDoc() const
{
	AuTime newest;
	for(unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::const_iterator it = m_Stance.begin(); it != m_Stance.end(); it++)
	{
		AuTime t = (*it).second.NewestNewDossierDoc();
		if(t > newest)
			newest = t;
	}
	return newest;
}

void DgEmpire::ForgetForgottenEmpires(const DgEmpireHash& empires)
{
	unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::iterator it = m_Stance.begin();
	while(it != m_Stance.end())
	{
		if(empires.find((*it).first) != empires.cend())
			it++;
		else
			it = m_Stance.erase(it);
	}
}

quint32 DgEmpire::CitiesInGalaxy(const AuId& galaxy) const
{
	return m_Cities->CitiesInGalaxy(galaxy);
}

quint32 DgEmpire::SolarSystemsInGalaxy(const AuId& galaxy) const
{
	return m_Cities->SolarSystemsInGalaxy(galaxy);
}

bool DgEmpire::RecordPatent(COMMODITYID patentCID, quint8 patentQ)
{
	return m_Patents.RecordPatent(patentCID, patentQ);
}

bool DgEmpire::HasCity() const
{
	return !m_Cities->empty();
}

AuId DgEmpire::RandomCity(const QSet<AuId>& exceptIds) const
{
	return m_Cities->Random(exceptIds);
}

quint64 DgEmpire::MemoryUsage() const
{
	quint64 mem = sizeof(DgEmpire);
	mem += StringMemoryUsage(m_name);
	mem += StringMemoryUsage(m_Announcements);
	if(m_Cities)
		mem += m_Cities->MemoryUsage();
	for(unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator it = m_Citizens.cbegin(); it != m_Citizens.cend(); it++)
		mem += sizeof(AuId) + (*it).second.MemoryUsage();
	mem += sizeof(quint8) * m_Commerce.capacity();
#if(QT_VERSION >= QT_VERSION_CHECK(5,10,0))
	mem += m_flag.sizeInBytes();
#else
	mem += m_flag.byteCount();
#endif
	mem += StringMemoryUsage(m_JoinMessage);
	mem += m_Patents.MemoryUsage();
	unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>	m_Stance;	// stance toward known empires. key is empire id
	for(unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::const_iterator it = m_Stance.cbegin(); it != m_Stance.cend(); it++)
		mem += sizeof(quint32) + (*it).second.MemoryUsage();
	mem += m_WarCriteria.MemoryUsage();
	return mem;
}

// Initialize war policy to something reasonable.
void DgEmpire::InitWarPolicy()
{
	SetWarCriteria(AuDocStarshipDestroyed::DocumentType(), 1);
	SetWarCriteria(AuDocSpaceshipDestroyed::DocumentType(), 1);
	SetWarCriteria(AuDocSpaceStationDestroyed::DocumentType(), 1);
	SetWarCriteria(AuDocCityTrespassCity::DocumentType(), 1);
	SetWarCriteria(AuDocStationTrespassCity::DocumentType(), 1);
	SetWarCriteria(AuDocOfficerKilled::DocumentType(), 1);
	SetWarCriteria(AuDocCrewKilled::DocumentType(), 1);
	SetWarCriteria(AuDocTroopKilled::DocumentType(), 1);
	SetWarCriteria(AuDocClaimHijacked::DocumentType(), 1);
	SetWarCriteria(AuDocVehicleDestroyed::DocumentType(), 3);
	SetWarCriteria(AuDocCitizenKilled::DocumentType(), 5);
	SetWarCriteria(AuDocBdDamaged::DocumentType(), 3);
	SetWarCriteria(AuDocBdDestroyed::DocumentType(), 2);
	SetWarCriteria(AuDocBdCityCapitolDestroyed::DocumentType(), 1);
	SetWarCriteria(AuDocBdSectorCapitolDestroyed::DocumentType(), 1);
	SetWarCriteria(AuDocBdGalaxyCapitolDestroyed::DocumentType(), 1);
	SetWarCriteria(AuDocBdEmpireCapitolDestroyed::DocumentType(), 1);
	SetWarCriteria(AuDocBdMilitaryDamaged::DocumentType(), 2);
	SetWarCriteria(AuDocBdMilitaryDestroyed::DocumentType(), 1);
	SetWarCriteria(AuDocBdMilitaryHQDestroyed::DocumentType(), 1);
}

AuBdPref DgEmpire::PreferredBlueprints(INDUSTRYID industryId) const
{
	return m_BdPrefs.PreferredBlueprints(industryId);
}

AuScPref DgEmpire::PreferredBlueprints(AuScPrefs::SCROLE role) const
{
	return m_ScPrefs.PreferredBlueprints(role);
}

void DgEmpire::SetPreferredBlueprints(INDUSTRYID industryId, const AuBdPref& pref)
{
	//qDebug() << "DgEmpire::SetPreferredBlueprints" << m_id << Name() << "iid" << industryId
		//<< pref.Blueprints().first().BlueprintId();
	m_BdPrefs.SetPreferredBlueprints(industryId, pref);
}

void DgEmpire::SetPreferredBlueprints(AuScPrefs::SCROLE role, const AuScPref& pref)
{
	//qDebug() << "DgEmpire::SetPreferredBlueprints" << m_id << Name() << AuScPrefs::RoleName(role) << pref.Blueprints().first();
	m_ScPrefs.SetPreferredBlueprints(role, pref);
}

bool DgEmpire::IsPirateReady() const
{
	if(!IsPirate())
		return false;
	if(!m_ScPrefs.CanMakePirateEncounters())
		return false;
	QList<INDUSTRYID> baseBuildings;
	PirateBaseBuildings(baseBuildings);
	return m_BdPrefs.CanMakePirateBases(baseBuildings);
}

quint32 DgEmpire::PirateEncounterSpacecraft(int ubiquity, AuScPrefs::SCROLE& scrole) const
{
	Q_UNUSED(ubiquity);	//DHH need more pirate designs for ubiquity to be useful.
	scrole = AuScPrefs::SCROLE_Military;
	AuScPref pref = m_ScPrefs.PreferredBlueprints(scrole);
	if(pref.Blueprints().empty())
		return 0;
	return pref.Blueprints().first();
}

bool DgEmpire::DossierAddDocument(quint32 dosEmpire, const AuDocument& doc)
{
	// Pirate empires do not maintain a dossier.
	if(IsPirate())
		return false;

	unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::iterator it = m_Stance.find(dosEmpire);
	if(it == m_Stance.end())
		return false;

	// The File function could discard the document as a duplicate and thus not change.
	return (*it).second.m_Dossier.File(doc);
}

void DgEmpire::PirateBaseBuildings(QList<INDUSTRYID>& buildings) const
{
	/* Buildings that might be built at a pirate base, after the IID_BaseHeadquarters is built:
	   IID_AirportRepairShop
	   *IID_BaseAcademy
	   IID_BaseAirTerminal
	   IID_BaseBarracks
	   *IID_BaseDock
	   *IID_BaseGuardPost
	   IID_BaseGuardTower
	   IID_BaseHeadquarters
	   *IID_BaseMarineBarracks
	   *IID_BaseMotorPool
	   IID_BaseRadar
	   IID_BaseShield
	   IID_BaseWeapon
	   IID_Cantina
	   IID_NuclearPowerPlant
	   IID_SpacecraftFactory
	*/
	// Number of buildings is rank x 2
	buildings << IID_BaseHeadquarters;		// All bases start with a headquarters.
	switch(m_id)
	{
	case PIRATE_Seledon:	// Rank 1
		buildings << IID_BaseBarracks;
		buildings << IID_BaseGuardTower;	// Ballistic weapons don't need power.
		buildings << IID_BaseGuardTower;
		buildings << IID_Cantina;
		break;
	case PIRATE_Ogar:		// Rank 2
		buildings << IID_BaseBarracks;
		buildings << IID_BaseGuardTower;	// Ballistic weapons don't need power.
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseGuardTower;
		buildings << IID_Cantina;
		break;
	case PIRATE_Balorite:	// Rank 3
		buildings << IID_BaseBarracks;
		buildings << IID_BaseGuardTower;	// Rockets don't need power.
		buildings << IID_NuclearPowerPlant;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseAirTerminal;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseGuardTower;
		buildings << IID_Cantina;
		break;
	case PIRATE_Vreen:		// Rank 4
		buildings << IID_BaseBarracks;
		buildings << IID_NuclearPowerPlant;
		buildings << IID_BaseGuardTower;	// Missiles need power for targetting.
		buildings << IID_BaseShield;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseAirTerminal;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseRadar;
		buildings << IID_BaseGuardTower;
		buildings << IID_Cantina;
		break;
	case PIRATE_Klatra:		// Rank 5
		buildings << IID_BaseBarracks;
		buildings << IID_NuclearPowerPlant;
		buildings << IID_BaseGuardTower;	// Energy weapons need power.
		buildings << IID_BaseShield;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseRadar;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseAirTerminal;
		buildings << IID_BaseGuardTower;
		buildings << IID_Cantina;
		break;
	case PIRATE_Syth:		// Rank 6
		buildings << IID_BaseBarracks;
		buildings << IID_NuclearPowerPlant;
		buildings << IID_BaseGuardTower;	// Energy weapons need power.
		buildings << IID_BaseShield;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseRadar;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseAirTerminal;
		buildings << IID_BaseGuardTower;
		buildings << IID_SpacecraftFactory;
		buildings << IID_BaseGuardTower;
		buildings << IID_Cantina;
		break;
	case PIRATE_Malacon:	// Rank 7
		buildings << IID_BaseBarracks;
		buildings << IID_NuclearPowerPlant;
		buildings << IID_BaseGuardTower;	// Energy weapons need power.
		buildings << IID_BaseShield;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseRadar;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseAirTerminal;
		buildings << IID_BaseGuardTower;
		buildings << IID_SpacecraftFactory;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_Cantina;
		break;
	case PIRATE_Vilmorti:	// Rank 8
		buildings << IID_BaseBarracks;
		buildings << IID_NuclearPowerPlant;
		buildings << IID_BaseGuardTower;	// Energy weapons need power.
		buildings << IID_BaseShield;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseRadar;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseAirTerminal;
		buildings << IID_BaseGuardTower;
		buildings << IID_SpacecraftFactory;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_AirportRepairShop;
		buildings << IID_BaseGuardTower;
		buildings << IID_Cantina;
		break;
	case PIRATE_Akson:		// Rank 9
		buildings << IID_BaseBarracks;
		buildings << IID_NuclearPowerPlant;
		buildings << IID_BaseGuardTower;	// Energy weapons need power.
		buildings << IID_BaseShield;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseRadar;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseAirTerminal;
		buildings << IID_BaseGuardTower;
		buildings << IID_SpacecraftFactory;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_AirportRepairShop;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_Cantina;
		break;
	case PIRATE_Tassad:		// Rank 10
		buildings << IID_BaseBarracks;
		buildings << IID_NuclearPowerPlant;
		buildings << IID_BaseGuardTower;	// Energy weapons need power.
		buildings << IID_BaseShield;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseRadar;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseAirTerminal;
		buildings << IID_BaseGuardTower;
		buildings << IID_SpacecraftFactory;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_AirportRepairShop;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseShield;
		buildings << IID_BaseGuardTower;
		buildings << IID_Cantina;
		break;
	case PIRATE_Otari:		// Rank 11
		buildings << IID_BaseBarracks;
		buildings << IID_NuclearPowerPlant;
		buildings << IID_BaseGuardTower;	// Energy weapons need power.
		buildings << IID_BaseShield;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseRadar;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseAirTerminal;
		buildings << IID_BaseGuardTower;
		buildings << IID_SpacecraftFactory;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_AirportRepairShop;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseShield;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_Cantina;
		break;
	case PIRATE_Myntaka:	// Rank 12
		buildings << IID_BaseBarracks;
		buildings << IID_NuclearPowerPlant;
		buildings << IID_BaseGuardTower;	// Energy weapons need power.
		buildings << IID_BaseShield;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseRadar;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseAirTerminal;
		buildings << IID_BaseGuardTower;
		buildings << IID_SpacecraftFactory;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_AirportRepairShop;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseShield;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_Cantina;
		break;
	case PIRATE_Dendrae:	// Rank 13
		buildings << IID_BaseBarracks;
		buildings << IID_NuclearPowerPlant;
		buildings << IID_BaseGuardTower;	// Energy weapons need power.
		buildings << IID_BaseShield;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseRadar;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseAirTerminal;
		buildings << IID_BaseGuardTower;
		buildings << IID_SpacecraftFactory;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_AirportRepairShop;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseShield;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_Cantina;
		break;
	case PIRATE_Haxu:		// Rank 14
		buildings << IID_BaseBarracks;
		buildings << IID_NuclearPowerPlant;
		buildings << IID_BaseGuardTower;	// Energy weapons need power.
		buildings << IID_BaseShield;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseRadar;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseAirTerminal;
		buildings << IID_BaseGuardTower;
		buildings << IID_SpacecraftFactory;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_AirportRepairShop;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseShield;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseShield;
		buildings << IID_BaseGuardTower;
		buildings << IID_Cantina;
		break;
	case PIRATE_Zuul:		// Rank 15
		buildings << IID_BaseBarracks;
		buildings << IID_NuclearPowerPlant;
		buildings << IID_BaseGuardTower;	// Energy weapons need power.
		buildings << IID_BaseShield;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseRadar;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseAirTerminal;
		buildings << IID_BaseGuardTower;
		buildings << IID_SpacecraftFactory;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_AirportRepairShop;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseShield;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseShield;
		buildings << IID_BaseGuardTower;
		buildings << IID_BaseWeapon;
		buildings << IID_BaseGuardTower;
		buildings << IID_Cantina;
		break;
	}
}

AuId DgEmpire::CityFounder(const AuId& cityId) const
{
	AuCityHash::const_iterator ci = m_Cities->find(cityId);
	if(ci == m_Cities->cend())
		return AuId::NullId();
	return (*ci).second.FounderId();
}

AuSpecies DgEmpire::CapitolSpecies() const
{
	if(!m_capitolSpecies.IsNull())
		return m_capitolSpecies;

	// Return default human species.
	AuDNA dna;
	dna.InitDefaultHuman();
	return dna.Species();
}

void DgEmpire::ListIndustryPatents(AuKnownPatents& patents, INDUSTRYID industryId, const DgData& gd) const
{
	m_Patents.ListIndustryPatents(patents, industryId, gd);
}

quint8 DgEmpire::PatentQuality(COMMODITYID patentCID) const
{
	return m_Patents.PatentQuality(patentCID);
}

AuId DgEmpire::JoinableCity() const
{
	return m_Cities->Joinable(m_capitol);
}

void DgEmpire::ListFriends(QList<quint32>& friends) const
{
	for(unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::const_iterator it = m_Stance.begin(); it != m_Stance.end(); it++)
		if((*it).second.m_stance == POL_Friend)
			friends.append((*it).first);
}

bool DgEmpire::CitizenOnLine()
{
	// Let us round the time to whole minutes.
	QDateTime now = AuTime::Now().UTC();
	QDate date = now.date();
	QTime time = now.time();
	int hour = time.hour();
	int min = time.minute();
	QTime rtime(hour, min);					// time with seconds truncated
	AuTime tnow;
	tnow = QDateTime(date, rtime, Qt::UTC);	// current date/time with seconds truncated
	if(tnow == m_OnLine)
		return false;						// Truncated time did not change.
	if(tnow < m_OnLine)
		return false;						// Truncated time is older than the last known online time.

	// Truncated time changed to a newer time.
	m_OnLine = tnow;
	return true;
}

bool DgEmpire::IsCitizenOnLine() const
{
	// Return true if any citizen reported being on line in the recent few minutes.
	return m_OnLine.ElapsedMin() < 15;
}

void DgEmpire::SetJoinPassCrypt(const QByteArray& passCrypt)
{
	m_immigPassCrypt = passCrypt;
}

void DgEmpire::SetJoinPassword(const QString& password)
{
	if(password.isEmpty())
	{
		m_immigPassCrypt.clear();
		return;
	}

	// Create a SHA-1 hash key from the password.
	QByteArray passCrypt;
	passCrypt.append(password.toUtf8());
	QCryptographicHash passCryptHash(QCryptographicHash::Sha1);
	passCryptHash.addData(passCrypt);
	m_immigPassCrypt = passCryptHash.result();
}

bool DgEmpire::TestJoinPassword(const QString& password) const
{
	// Empty password never works.
	// We don't allow empty password so nothing works until a real password is established.
	if(password.isEmpty())
		return false;

	// Create a SHA-1 hash key from the password.
	QByteArray passCrypt;
	passCrypt.append(password.toUtf8());
	QCryptographicHash passCryptHash(QCryptographicHash::Sha1);
	passCryptHash.addData(passCrypt);
	return m_immigPassCrypt == passCryptHash.result();	// True if the result matches the empire's encrypted password.
}

bool DgEmpire::TestJoinPassCrypt(const QByteArray& passCrypt) const
{
	// Empty password never works.
	// We don't allow empty password so nothing works until a real password is established.
	if(passCrypt.isEmpty())
		return false;
	return m_immigPassCrypt == passCrypt;	// True if the result matches the empire's encrypted password.
}

bool DgEmpire::IsJoinableByNewAvatar() const
{
	if(!IsPlayerEmpire())
		return false;		// Cannot join empire that is not a player empire.
	if(m_Cities->empty())
		return false;		// Cannot join empire that has no cities.
	if(m_immig == 0)
		return true;		// Can join empire with open immigration policy.
	if(!HasJoinPassword())
		return false;		// Cannot join restricted empire with blank password.
	return true;			// Can join since we passed all the tests.
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Load all the empire records in one select statement.
void DgEmpireHash::Load()
{
	clear();

	QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
	if(!db.isOpen())
		return;

	QString sq("SELECT id, name, announcements, immig, immigPassCrypt, flag, capitalCity, capitalDNA,capitalSpec, "
			"rewCapBase,rewCapCapital,rewCapCapitalGalaxy,rewCapCapitalSector,rewCapCity,"
			"rewCapSpaceport,rewCapSpaceship,rewCapStarship,rewDmgSpaceport,"
			"rewDmgSpaceship,rewDmgStarShip,rewDiscHabitable,rewDiscWorm,"
			"rewKillEnemy,rewSalary,rewSpotCity,rewSpotEnemy,taxIncome,"
			"taxSales,taxSubsidy,defaultStance,stance,theme,commerce,"
			"citizens,cities, captureMsg,contrailHue,contrailSat,"
			"role, bdprefs,scprefs, online FROM g_Empire;");
	QSqlQuery qry(QString(), db);
	qry.prepare(sq);
	qry.exec();
	while(qry.next())
	{
		DgEmpire ob;
		int qv=0;
		ob.m_id = qry.value(qv++).toUInt();
		ob.m_name = qry.value(qv++).toString();
		ob.m_Announcements = qry.value(qv++).toString();
		ob.m_immig = qry.value(qv++).toInt();
		ob.m_immigPassCrypt = qry.value(qv++).toByteArray();
		{
			QByteArray bcomp = qry.value(qv++).toByteArray();
			if(!bcomp.isEmpty())
			{
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,QString("%1").arg(ob.m_id));
				QDataStream ds(&ba, QIODevice::ReadOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				ds >> ob.m_flag;
			}
		}
		ob.m_capitol = AuId(qry.value(qv++).toUInt());

		// Species
		QByteArray bdna = qry.value(qv++).toByteArray();
		ob.m_capitolSpecies = AuSpecies(qry.value(qv++).toByteArray());
		if(!bdna.isEmpty())
		{
			// Load old style DNA.
			DhDNA_ dna3(bdna);
			ob.m_capitolSpecies = DhDNA(dna3).Species();	// Override capital species with old dna
		}

		ob.m_RewCapBase = qry.value(qv++).toInt();
		ob.m_RewCapCapitalEmpire = qry.value(qv++).toInt();
		ob.m_RewCapCapitalGalaxy = qry.value(qv++).toInt();
		ob.m_RewCapCapitalSector = qry.value(qv++).toInt();
		ob.m_RewCapCity = qry.value(qv++).toInt();
		ob.m_RewCapStation = qry.value(qv++).toInt();
		ob.m_RewCapSpaceship = qry.value(qv++).toInt();
		ob.m_RewCapStarship = qry.value(qv++).toInt();
		ob.m_RewDmgStation = qry.value(qv++).toInt();
		ob.m_RewDmgSpaceship = qry.value(qv++).toInt();
		ob.m_RewDmgStarship = qry.value(qv++).toInt();
		ob.m_RewDiscHabitable = qry.value(qv++).toInt();
		ob.m_RewDiscWorm = qry.value(qv++).toInt();
		ob.m_RewKillEnemy = qry.value(qv++).toInt();
		ob.m_RewSalary = qry.value(qv++).toInt();
		ob.m_RewSpotCity = qry.value(qv++).toInt();
		ob.m_RewSpotEnemy = qry.value(qv++).toInt();
		ob.m_TaxIncome = qry.value(qv++).toInt();
		ob.m_TaxSales = qry.value(qv++).toInt();
		ob.m_TaxSubsidy = qry.value(qv++).toInt();
		ob.m_DefaultStance = qry.value(qv++).toInt();
		{
			QByteArray bcomp = qry.value(qv++).toByteArray();
			if(!bcomp.isEmpty())
			{
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,QString("%1").arg(ob.m_id));
				QDataStream ds(&ba, QIODevice::ReadOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				ob.m_Stance.clear();
				quint16 count;
				ds >> count;
				for(;count ;count--)
				{
					AuKnownEmpire ke;
					ke << ds;

					ke.InitDossier();

					ob.m_Stance.insert(pair<quint32, AuKnownEmpire>(ke.m_id, ke));
				}

				ds >> ob.m_WarCriteria;
				ds >> ob.m_Patents;
				quint8 ver;
				ds >> ver;
				if(ver == 2)
				{
					AuKnownPatents p;
					ds >> p;
					ds >> p;
					ds >> p;
				}
				else if(ver == 1)
				{
					AuKnownPatents p;
					ds >> p;
				}
			}
		}

		{
			QByteArray ba = qry.value(qv++).toByteArray();
			if(!ba.isEmpty())
			{
				QDataStream ds(&ba, QIODevice::ReadOnly);
				ds.setVersion(QDataStream::Qt_4_1);

				quint8 c;
				ds >> c;
				for(int tc=0; tc<c; tc++)
					ds >> ob.m_ThemeColors[tc];
			}
		}

		{
			QByteArray bcomp = qry.value(qv++).toByteArray();
			if(!bcomp.isEmpty())
			{
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,QString("%1").arg(ob.m_id));
				QDataStream ds(&ba, QIODevice::ReadOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				ds >> ob.m_Commerce;
			}
			while(ob.m_Commerce.size() < CID_Count)
				ob.m_Commerce.append(EXPOL_ToNeutral);	// Grow array to include nu commodities that have been added.
		}
		// Citizens
		{
			QByteArray bcomp = qry.value(qv++).toByteArray();
			if(!bcomp.isEmpty())
			{
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,QString("%1").arg(ob.m_id));
				QDataStream ds(&ba, QIODevice::ReadOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				ob.m_Citizens.clear();
				quint16 count;
				ds >> count;
				//qDebug("DgEmpireHash::Load Citizens count=%d", count);
				for(;count ;count--)
				{
					AuCitizen ke;
					ke << ds;
					//qDebug("DgEmpireHash::Load Citizen%d=%s:%d",count,(const char*)ke.m_name.toAscii(),ke.Office());
					ob.m_Citizens.insert(pair<AuId, AuCitizen>(ke.m_id, ke));
				}
			}
		}
		// Cities
		{
			QByteArray bcomp = qry.value(qv++).toByteArray();
			if(!bcomp.isEmpty())
			{
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,QString("%1").arg(ob.m_id));
				QDataStream ds(&ba, QIODevice::ReadOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				ds >> *ob.m_Cities;
			}
		}
		ob.m_JoinMessage = qry.value(qv++).toString();	// captureMsg field
		ob.m_ContrailHue = qry.value(qv++).toInt();
		ob.m_ContrailSat = qry.value(qv++).toInt();
		ob.m_GameRole = (EMPIRE_ROLE)qry.value(qv++).toInt();

		// Bd blueprint preferences
		{
			QByteArray bcomp = qry.value(qv++).toByteArray();
			if(!bcomp.isEmpty())
			{
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,QString("%1").arg(ob.m_id));
				QDataStream ds(&ba, QIODevice::ReadOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				ds >> ob.m_BdPrefs;
			}
		}

		// Sc blueprint preferences
		{
			QByteArray bcomp = qry.value(qv++).toByteArray();
			if(!bcomp.isEmpty())
			{
				QByteArray ba = AuUncompress(bcomp, __FILE__, __LINE__,QString("%1").arg(ob.m_id));
				QDataStream ds(&ba, QIODevice::ReadOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				ds >> ob.m_ScPrefs;
			}
		}

		QDateTime online = qry.value(qv++).toDateTime();
		online.setTimeSpec(Qt::UTC);
		ob.m_OnLine = online;

		insert(pair<quint32, DgEmpire>(ob.m_id, ob));
		//qDebug() << "DgEmpireHash::Load loaded" << ob.m_id << ob.Name();
	}

	// Now that all empires are loaded, remove forgotten empires from their diplomacy lists.
	for(iterator it = begin(); it != end(); it++)
		(*it).second.ForgetForgottenEmpires(*this);

	if(size() == 0)
		qDebug() << "DgEmpireHash::Load loaded" << size() << "records";
}

void DgEmpireHash::operator>>(QDataStream& out) const
{
	out << (quint32)size();
	for(const_iterator it = cbegin(); it != cend(); it++)
		(*it).second >> out;
}

void DgEmpireHash::operator<<(QDataStream& in)
{
	clear();
	quint32 count;
	in >> count;
	for(;count; count--)
	{
		DgEmpire empire;
		empire << in;
		insert(pair<quint32, DgEmpire>(empire.m_id, empire));
	}
}

void DgEmpireHash::DiscardForgottenEmpires()
{
	iterator ei = begin();
	while(ei != end())
		if((*ei).second.IsForgottenEmpire())
			ei = erase(ei);
		else
			ei++;
}

quint64 DgEmpireHash::MemoryUsage() const
{
	quint64 mem = sizeof(DgEmpireHash);
	for(const_iterator it = cbegin(); it != cend(); it++)
		mem += sizeof(quint32) + (*it).second.MemoryUsage();
	return mem;
}

AuId DgEmpireHash::PirateBaseInSystem(const AuId& systemId, quint32& empireId) const
{
	for(quint32 pirateId = PIRATE_Akson; pirateId < PIRATE_Count; pirateId++)
	{
		const_iterator ei = find(pirateId);
		if(ei == cend())
			continue;
		AuId base = (*ei).second.BaseInSystem(systemId);
		if(!base.IsNull())
		{
			empireId = (*ei).first;
			return base;
		}
	}
	empireId = 0;
	return AuId();
}

AuId DgEmpireHash::PirateBaseSystemInSector(const AuId& sectorId, quint32& empireId) const
{
	for(quint32 pirateId = PIRATE_Akson; pirateId < PIRATE_Count; pirateId++)
	{
		const_iterator ei = find(pirateId);
		if(ei == cend())
			continue;
		AuId baseSystem = (*ei).second.BaseSystemInSector(sectorId);
		if(!baseSystem.IsNull())
		{
			empireId = pirateId;
			return baseSystem;
		}
	}
	empireId = 0;
	return AuId();
}

void DgEmpireHash::RemoveInhabitedSystems(AuIdList& systemIds) const
{
	AuIdList::iterator si = systemIds.begin();
	while(si != systemIds.end())
	{
		if(AnyCityInSystem(*si))
			si = systemIds.erase(si);
		else
			si++;
	}
}

bool DgEmpireHash::AnyCityInSystem(const AuId& systemId) const
{
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it).second.HasCityInSystem(systemId))
			return true;
	return false;
}

AuSpecies DgEmpireHash::CapitolSpecies(quint32 empireId) const
{
	const_iterator it = find(empireId);
	if(it != cend())
		return (*it).second.CapitolSpecies();

	// Return default human species.
	AuDNA dna;
	dna.InitDefaultHuman();
	return dna.Species();
}

AuSpecies DgEmpireHash::RandomSpecies() const
{
	int randomEmpireIndex = g_Au.Dice().Roll(1, (unsigned int)size(), -1);
	for(const_iterator ei = cbegin(); ei != cend(); ei++)
	{
		if(randomEmpireIndex)
			randomEmpireIndex--;
		else
			return (*ei).second.CapitolSpecies();
	}

	// Return default human species.
	AuDNA dna;
	dna.InitDefaultHuman();
	return dna.Species();
}

AuBdPref DgEmpireHash::PreferredBlueprints(quint32 empireId, INDUSTRYID industryId) const
{
	// Locate the empire record for 'empireId'.
	const_iterator ei = find(empireId);
	if(ei == cend())
		return AuBdPref();	// The empire has no preferred blueprints.
	return (*ei).second.PreferredBlueprints(industryId);
}

AuScPref DgEmpireHash::PreferredBlueprints(quint32 empireId, AuScPrefs::SCROLE role) const
{
	// Locate the empire record for 'empireId'.
	const_iterator ei = find(empireId);
	if(ei == cend())
		return AuScPref();	// The empire has no preferred blueprints.
	return (*ei).second.PreferredBlueprints(role);
}

//static
int DgEmpireHash::PirateEmpireRank(quint32 empireId)
{
	if(empireId < PIRATE_Akson || empireId >= PIRATE_Count)
		return 0;
	// Rank is the assigned difficulty of an empire, based on its home galaxy location in the universe, bottom to top.
	// Pirate Empire	Rank
	//  PIRATE_Akson	9
	//  PIRATE_Balorite	3
	//  PIRATE_Dendrae	13
	//  PIRATE_Haxu		14
	//  PIRATE_Klatra	5
	//  PIRATE_Malacon	7
	//  PIRATE_Myntaka	12
	//  PIRATE_Ogar		2
	//  PIRATE_Otari	11
	//  PIRATE_Seledon	1
	//  PIRATE_Syth		6
	//  PIRATE_Tassad	10
	//  PIRATE_Vilmorti	8
	//  PIRATE_Vreen	4
	//  PIRATE_Zuul		15
	int level[PIRATE_Count] = {0,9,3,13, 14,5,7,12, 2,11,1,6, 10,8,4,15};
	return level[empireId];
}

//static
void DgEmpireHash::PirateEmpireUbiquity(GALAXY galaxy, int ubiquity[PIRATE_Count])
{
	// Ubiquity values are determined by starting at 7, at the home galaxy of each pirate empire. The value is reduced
	// by 'step' for each galaxy away, following intergalactic wormholes.
	//
	// Pirate Empire	Rank Step
	//  PIRATE_Akson	9    2 typical step
	//  PIRATE_Balorite	3    1 because they are described as everyone's common enemy for hundreds of years
	//  PIRATE_Dendrae	13   2
	//  PIRATE_Haxu		14   3 ragtag splinter group
	//  PIRATE_Klatra	5    2
	//  PIRATE_Malacon	7    2
	//  PIRATE_Myntaka	12   3 slow to emerge
	//  PIRATE_Ogar		2    2
	//  PIRATE_Otari	11   2
	//  PIRATE_Seledon	1    2
	//  PIRATE_Syth		6    1 carved deep into Universe
	//  PIRATE_Tassad	10   2
	//  PIRATE_Vilmorti	8    2
	//  PIRATE_Vreen	4    2
	//  PIRATE_Zuul		15   4 reclusive, xenophobic, furtive, rare
	//
	// Result for each galaxy.
	//   Andromeda Rising pirate ubiquity 44 ready 8
	//   Black Hole pirate ubiquity 58 ready 4
	//   Core pirate ubiquity 58 ready 4
	//   Crown of Othon pirate ubiquity 31 ready 0
	//   Dyrathon's Retreat pirate ubiquity 45 ready 8
	//   Edge of the Rift pirate ubiquity 37 ready 12
	//   Falla's Embrace pirate ubiquity 44 ready 8
	//   Fallen Legions of Muturon pirate ubiquity 47 ready 1
	//   Heart of Victorus pirate ubiquity 31 ready 0
	//   House Zanathar pirate ubiquity 58 ready 4
	//   Indigo Sea pirate ubiquity 45 ready 1
	//   In'kar Border Region pirate ubiquity 37 ready 12
	//   Muturon Encounter pirate ubiquity 47 ready 1
	//   Ransuul's Flaming Sword pirate ubiquity 45 ready 1
	//   Seven Ten pirate ubiquity 58 ready 4
	//   Shores of Hazeron pirate ubiquity 37 ready 12
	//   Thustra's Eye pirate ubiquity 31 ready 0
	//   Veil of Targoss pirate ubiquity 37 ready 12
	//   Vreenox Eclipse pirate ubiquity 45 ready 8
	//   Vulcan's Forge pirate ubiquity 31 ready 0
	//   Milky Way pirate ubiquity 0 ready 0

	ubiquity[PIRATE_Invalid] = 0;
	switch(galaxy)
	{
	case AndromedaRising:
	case FallasEmbrace:
		ubiquity[PIRATE_Akson] = 7 - 1 * 2;
		ubiquity[PIRATE_Balorite] = 7 - 3 * 1;
		ubiquity[PIRATE_Dendrae] = 7 - 3 * 2;
		ubiquity[PIRATE_Haxu] = 0;	// 7 - 3 * 3;
		ubiquity[PIRATE_Klatra] = 7 - 2 * 2;
		ubiquity[PIRATE_Malacon] = 7 - 2 * 2;
		ubiquity[PIRATE_Myntaka] = 7 - 2 * 3;
		ubiquity[PIRATE_Ogar] = 7 - 1 * 2;
		ubiquity[PIRATE_Otari] = 7 - 3 * 2;
		ubiquity[PIRATE_Seledon] = 7 - 2 * 2;
		ubiquity[PIRATE_Syth] = 7 - 2 * 1;
		ubiquity[PIRATE_Tassad] = 7 - 1 * 2;
		ubiquity[PIRATE_Vilmorti] = 7;	// home galaxy
		ubiquity[PIRATE_Vreen] = 7 - 3 * 2;
		ubiquity[PIRATE_Zuul] = 0;	// 7 - 4 * 4;
		break;
	case BlackHole:
	case Core:
		ubiquity[PIRATE_Akson] = 7 - 2 * 1;
		ubiquity[PIRATE_Balorite] = 7 - 1 * 1;
		ubiquity[PIRATE_Dendrae] = 7 - 2 * 2;
		ubiquity[PIRATE_Haxu] = 7 - 3 * 1;
		ubiquity[PIRATE_Klatra] = 7 - 2 * 2;
		ubiquity[PIRATE_Malacon] = 7;	// home galaxy
		ubiquity[PIRATE_Myntaka] = 7 - 3 * 2;
		ubiquity[PIRATE_Ogar] = 7 - 2 * 3;
		ubiquity[PIRATE_Otari] = 7 - 2 * 1;
		ubiquity[PIRATE_Seledon] = 7 - 2 * 2;
		ubiquity[PIRATE_Syth] = 7;		// home galaxy
		ubiquity[PIRATE_Tassad] = 7 - 2 * 1;
		ubiquity[PIRATE_Vilmorti] = 7 - 2 * 2;
		ubiquity[PIRATE_Vreen] = 7 - 2 * 1;
		ubiquity[PIRATE_Zuul] = 0;	// 7 - 4 * 2;
		break;
	case CrownOfOthon:
	case HeartOfVictorus:
		ubiquity[PIRATE_Akson] = 7 - 2 * 2;
		ubiquity[PIRATE_Balorite] = 7 - 1 * 4;
		ubiquity[PIRATE_Dendrae] = 7;	// home galaxy
		ubiquity[PIRATE_Haxu] = 0;	// 7 - 3 * 4;
		ubiquity[PIRATE_Klatra] = 7 - 2 * 1;
		ubiquity[PIRATE_Malacon] = 7 - 2 * 3;
		ubiquity[PIRATE_Myntaka] = 7 - 3 * 1;
		ubiquity[PIRATE_Ogar] = 0;	// 7 - 2 * 4;
		ubiquity[PIRATE_Otari] = 0;	// 7 - 2 * 4;
		ubiquity[PIRATE_Seledon] = 0;	// 7 - 2 * 5;
		ubiquity[PIRATE_Syth] = 7 - 1 * 3;
		ubiquity[PIRATE_Tassad] = 7 - 2 * 2;
		ubiquity[PIRATE_Vilmorti] = 7 - 2 * 3;
		ubiquity[PIRATE_Vreen] = 0;	// 7 - 2 * 4;
		ubiquity[PIRATE_Zuul] = 0;	// 7 - 4 * 5;
		break;
	case DyrathonsRetreat:
	case VreenoxEclipse:
		ubiquity[PIRATE_Akson] = 7 - 2 * 2;
		ubiquity[PIRATE_Balorite] = 7;	// home galaxy
		ubiquity[PIRATE_Dendrae] = 0;	// 7 - 2 * 4;
		ubiquity[PIRATE_Haxu] = 7 - 3 * 2;
		ubiquity[PIRATE_Klatra] = 7 - 2 * 3;
		ubiquity[PIRATE_Malacon] = 7 - 2 * 1;
		ubiquity[PIRATE_Myntaka] = 0;	// 7 - 3 * 3;
		ubiquity[PIRATE_Ogar] = 7 - 2 * 2;
		ubiquity[PIRATE_Otari] = 7 - 2 * 2;
		ubiquity[PIRATE_Seledon] = 7 - 2 * 1;
		ubiquity[PIRATE_Syth] = 7 - 1 * 1;
		ubiquity[PIRATE_Tassad] = 7 - 2 * 2;
		ubiquity[PIRATE_Vilmorti] = 7 - 2 * 3;
		ubiquity[PIRATE_Vreen] = 7;		// home galaxy
		ubiquity[PIRATE_Zuul] = 0;	// 7 - 4 * 3;
		break;
	case EdgeOfTheRift:
	case InkarBorderRegion:
		ubiquity[PIRATE_Akson] = 7 - 2 * 2;
		ubiquity[PIRATE_Balorite] = 7 - 1 * 2;
		ubiquity[PIRATE_Dendrae] = 0;	// 7 - 2 * 4;
		ubiquity[PIRATE_Haxu] = 0;	// 7 - 3 * 4;
		ubiquity[PIRATE_Klatra] = 7 - 2 * 3;
		ubiquity[PIRATE_Malacon] = 7 - 2 * 3;
		ubiquity[PIRATE_Myntaka] = 0;	// 7 - 3 * 3;
		ubiquity[PIRATE_Ogar] = 7;		// home galaxy
		ubiquity[PIRATE_Otari] = 0;	// 7 - 2 * 4;
		ubiquity[PIRATE_Seledon] = 7 - 2 * 1;
		ubiquity[PIRATE_Syth] = 7 - 1 * 3;
		ubiquity[PIRATE_Tassad] = 7 - 2 * 2;
		ubiquity[PIRATE_Vilmorti] = 7 - 2 * 1;
		ubiquity[PIRATE_Vreen] = 7 - 2 * 2;
		ubiquity[PIRATE_Zuul] = 0;	// 7 - 4 * 5;
		break;
	case FallenLegionsOfMuturon:
	case MuturonEncounter:
		ubiquity[PIRATE_Akson] = 7 - 2 * 1;
		ubiquity[PIRATE_Balorite] = 7 - 1 * 3;
		ubiquity[PIRATE_Dendrae] = 7 - 2 * 1;
		ubiquity[PIRATE_Haxu] = 0;	// 7 - 3 * 3;
		ubiquity[PIRATE_Klatra] = 7;	// home galaxy
		ubiquity[PIRATE_Malacon] = 7 - 2 * 2;
		ubiquity[PIRATE_Myntaka] = 7;	// home galaxy
		ubiquity[PIRATE_Ogar] = 7 - 2 * 3;
		ubiquity[PIRATE_Otari] = 7 - 2 * 3;
		ubiquity[PIRATE_Seledon] = 0;	// 7 - 2 * 4;
		ubiquity[PIRATE_Syth] = 7 - 1 * 2;
		ubiquity[PIRATE_Tassad] = 7 - 2 * 1;
		ubiquity[PIRATE_Vilmorti] = 7 - 2 * 2;
		ubiquity[PIRATE_Vreen] = 7 - 2 * 3;
		ubiquity[PIRATE_Zuul] = 0;	// 7 - 4 * 4;
		break;
	case HouseZanathar:
	case SevenTen:
		ubiquity[PIRATE_Akson] = 7;		// home galaxy
		ubiquity[PIRATE_Balorite] = 7 - 1 * 2;
		ubiquity[PIRATE_Dendrae] = 7 - 2 * 2;
		ubiquity[PIRATE_Haxu] = 7 - 3 * 2;
		ubiquity[PIRATE_Klatra] = 7 - 2 * 1;
		ubiquity[PIRATE_Malacon] = 7 - 2 * 1;
		ubiquity[PIRATE_Myntaka] = 7 - 3 * 1;
		ubiquity[PIRATE_Ogar] = 7 - 2 * 2;
		ubiquity[PIRATE_Otari] = 7 - 2 * 2;
		ubiquity[PIRATE_Seledon] = 7 - 2 * 3;
		ubiquity[PIRATE_Syth] = 7 - 1 * 1;
		ubiquity[PIRATE_Tassad] = 7;	// home galaxy
		ubiquity[PIRATE_Vilmorti] = 7 - 2 * 1;
		ubiquity[PIRATE_Vreen] = 7 - 2 * 2;
		ubiquity[PIRATE_Zuul] = 0;	// 7 - 4 * 3;
		break;
	case IndigoSea:
	case RansuulsFlamingSword:
		ubiquity[PIRATE_Akson] = 7 - 2 * 2;
		ubiquity[PIRATE_Balorite] = 7 - 1 * 2;
		ubiquity[PIRATE_Dendrae] = 0;	// 7 - 2 * 4;
		ubiquity[PIRATE_Haxu] = 7;		// home galaxy
		ubiquity[PIRATE_Klatra] = 7 - 2 * 3;
		ubiquity[PIRATE_Malacon] = 7 - 2 * 1;
		ubiquity[PIRATE_Myntaka] = 0;	// 7 - 3 * 3;
		ubiquity[PIRATE_Ogar] = 0;	// 7 - 2 * 4;
		ubiquity[PIRATE_Otari] = 7;		// home galaxy
		ubiquity[PIRATE_Seledon] = 7 - 2 * 3;
		ubiquity[PIRATE_Syth] = 7 - 1 * 1;
		ubiquity[PIRATE_Tassad] = 7 - 2 * 2;
		ubiquity[PIRATE_Vilmorti] = 7 - 2 * 3;
		ubiquity[PIRATE_Vreen] = 7 - 2 * 2;
		ubiquity[PIRATE_Zuul] = 7 - 4 * 1;
		break;
	case ShoresOfHazeron:
	case VeilOfTargoss:	
		ubiquity[PIRATE_Akson] = 7 - 2 * 3;
		ubiquity[PIRATE_Balorite] = 7 - 1 * 1;
		ubiquity[PIRATE_Dendrae] = 0;	// 7 - 2 * 5;
		ubiquity[PIRATE_Haxu] = 0;	// 7 - 3 * 3;
		ubiquity[PIRATE_Klatra] = 0;	// 7 - 2 * 4;
		ubiquity[PIRATE_Malacon] = 7 - 2 * 2;
		ubiquity[PIRATE_Myntaka] = 0;	// 7 - 3 * 4;
		ubiquity[PIRATE_Ogar] = 7 - 2 * 1;
		ubiquity[PIRATE_Otari] = 7 - 2 * 3;
		ubiquity[PIRATE_Seledon] = 7;	// home galaxy
		ubiquity[PIRATE_Syth] = 7 - 1 * 2;
		ubiquity[PIRATE_Tassad] = 7 - 2 * 3;
		ubiquity[PIRATE_Vilmorti] = 7 - 2 * 2;
		ubiquity[PIRATE_Vreen] = 7 - 2 * 1;
		ubiquity[PIRATE_Zuul] = 0;	// 7 - 4 * 4;
		break;
	case ThustrasEye:
	case VulcansForge:
		ubiquity[PIRATE_Akson] = 7 - 2 * 3;
		ubiquity[PIRATE_Balorite] = 7 - 1 * 3;
		ubiquity[PIRATE_Dendrae] = 0;	// 7 - 2 * 5;
		ubiquity[PIRATE_Haxu] = 7 - 3 * 1;
		ubiquity[PIRATE_Klatra] = 0;	// 7 - 2 * 4;
		ubiquity[PIRATE_Malacon] = 7 - 2 * 2;
		ubiquity[PIRATE_Myntaka] = 0;	// 7 - 3 * 4;
		ubiquity[PIRATE_Ogar] = 0;	// 7 - 2 * 5;
		ubiquity[PIRATE_Otari] = 7 - 2 * 1;
		ubiquity[PIRATE_Seledon] = 0;	// 7 - 2 * 4;
		ubiquity[PIRATE_Syth] = 7 - 1 * 2;
		ubiquity[PIRATE_Tassad] = 7 - 2 * 3;
		ubiquity[PIRATE_Vilmorti] = 0;	// 7 - 2 * 4;
		ubiquity[PIRATE_Vreen] = 7 - 2 * 3;
		ubiquity[PIRATE_Zuul] = 7;		// home galaxy
		break;
	case MilkyWay:
		ubiquity[PIRATE_Akson] = 0;
		ubiquity[PIRATE_Balorite] = 0;
		ubiquity[PIRATE_Dendrae] = 0;
		ubiquity[PIRATE_Haxu] = 0;
		ubiquity[PIRATE_Klatra] = 0;
		ubiquity[PIRATE_Malacon] = 0;
		ubiquity[PIRATE_Myntaka] = 0;
		ubiquity[PIRATE_Ogar] = 0;
		ubiquity[PIRATE_Otari] = 0;
		ubiquity[PIRATE_Seledon] = 0;
		ubiquity[PIRATE_Syth] = 0;
		ubiquity[PIRATE_Tassad] = 0;
		ubiquity[PIRATE_Vilmorti] = 0;
		ubiquity[PIRATE_Vreen] = 0;
		ubiquity[PIRATE_Zuul] = 0;
		break;
	case GALAXY_Count:
	default:
		// Initialize the ubiquity values.
		for(int ui=0; ui<PIRATE_Count; ui++)
			ubiquity[ui] = 0;
		break;
	}
}

int DgEmpireHash::PirateUbiquity(GALAXY galaxy) const
{
	int ubiquity[PIRATE_Count];
	PirateEmpireUbiquity(galaxy, ubiquity);

	// Check for pirate empires that are ready for encounters. That means they have to have enough building blueprints for all
	// environments and spacecraft design preferences for all roles.
	int totalUbiq = 0;
	for(quint32 pirateId = PIRATE_Akson; pirateId < PIRATE_Count; pirateId++)
	{
		int ubiq = ubiquity[pirateId];
		if(!ubiq)
			continue;	// Skip pirate empires that are not present in the galaxy.
		const_iterator ei = find(pirateId);
		if(ei == cend())
			continue;
		if(!(*ei).second.IsPirateReady())
			continue;	// Skip non-pirate empires and pirate empires that are not ready for encounters.
		totalUbiq += ubiq;
	}
	return totalUbiq;
}

quint32 DgEmpireHash::RandomPirateEmpire(GALAXY galaxy) const
{
	int ubiquity[PIRATE_Count];
	PirateEmpireUbiquity(galaxy, ubiquity);

	// Get a list of pirate empires that are ready for encounters. That means they have to have enough building blueprints for all
	// environments and spacecraft design preferences for all roles.
	QHash<quint32, int> pirateEmpires;	// key is empire id, value is ubiquity
	int totalUbiq = 0;
	for(quint32 pirateId = PIRATE_Akson; pirateId < PIRATE_Count; pirateId++)
	{
		int ubiq = ubiquity[pirateId];
		if(!ubiq)
			continue;	// Skip pirate empires that are not present in the galaxy.
		const_iterator ei = find(pirateId);
		if(ei == cend())
			continue;
		if(!(*ei).second.IsPirateReady())
			continue;	// Skip non-pirate empires and pirate empires that are not ready for encounters.
		pirateEmpires.insert((*ei).first, ubiq);
		totalUbiq += ubiq;
		//qDebug() << "DgEmpireHash::RandomPirateEmpire" << pirateId << (*ei).second.Name() << "ready, ubiq" << ubiq
			//<< "totalUbiq" << totalUbiq;
	}

	int sz = pirateEmpires.size();
	switch(sz)
	{
	case 0:		// No pirate empires are ready for encounters.
		break;
	case 1:		// Only one pirate empire is ready for encounters.
		return pirateEmpires.cbegin().key();
	default:	// Select a pirate empire at random.
		{
			int selUbiq = g_Au.Dice().Roll(1, totalUbiq);
			int findUbiq = 0;
			for(QHash<quint32, int>::const_iterator pi = pirateEmpires.cbegin(); pi != pirateEmpires.cend(); pi++)
			{
				findUbiq += *pi;
				if(selUbiq <= findUbiq)
					return pi.key();
			}
			qWarning() << "warning: DgEmpireHash::RandomPirateEmpire did not find selected empire, totalUbiq" << totalUbiq
				<< "selUbiq" << selUbiq << "findUbiq" << findUbiq;
		}
		break;
	}
	return 0;
}

void DgEmpireHash::SetPreferredBlueprints(quint32 empireId, INDUSTRYID industryId, const AuBdPref& pref)
{
	iterator ei = find(empireId);
	if(ei == end())
		return;
	(*ei).second.SetPreferredBlueprints(industryId, pref);
	if((*ei).second.IsPirateReady())
		qDebug() << "DgEmpireHash::SetPreferredBlueprints" << empireId << (*ei).second.Name() << "ready for pirate encounters";
}

void DgEmpireHash::SetPreferredBlueprints(quint32 empireId, AuScPrefs::SCROLE role, const AuScPref& pref)
{
	iterator ei = find(empireId);
	if(ei == end())
		return;
	(*ei).second.SetPreferredBlueprints(role, pref);
	if((*ei).second.IsPirateReady())
		qDebug() << "DgEmpireHash::SetPreferredBlueprints" << empireId << (*ei).second.Name() << "ready for pirate encounters";
}

void DgEmpireHash::PirateBdBlueprints(QList<quint32>& blueprints) const
{
	//qDebug() << "DgEmpireHash::PirateBdBlueprints start" << blueprints.size();
	for(quint32 pirateId = PIRATE_Akson; pirateId < PIRATE_Count; pirateId++)
	{
		const_iterator ei = find(pirateId);
		if(ei == cend())
			continue;
		if(!(*ei).second.IsPirateReady())
			continue;	// Skip non-pirate empires and pirate empires that are not ready for encounters.
		const AuBdPrefs& pref = (*ei).second.PreferredBdBlueprints();
		for(QHash<INDUSTRYID, AuBdPref>::const_iterator pi = pref.PreferredBlueprints().cbegin();
				pi != pref.PreferredBlueprints().cend(); pi++)
			for(QList<AuBlueprintPref>::const_iterator bi = (*pi).Blueprints().cbegin(); bi != (*pi).Blueprints().cend(); bi++)
				if(!blueprints.contains((*bi).BlueprintId()))
					blueprints.append((*bi).BlueprintId());
				else
					qDebug() << "DgEmpireHash::PirateBdBlueprints" << (*ei).first << (*ei).second.Name() << "blueprint dup"
						<< (*bi).BlueprintId() << "industry" << pi.key() << "prefs" << (*pi).Blueprints().size();
	}
	//qDebug() << "DgEmpireHash::PirateBdBlueprints end" << blueprints.size();
}

void DgEmpireHash::PirateScBlueprints(QList<quint32>& blueprints) const
{
	//qDebug() << "DgEmpireHash::PirateScBlueprints start" << blueprints.size();
	for(quint32 pirateId = PIRATE_Akson; pirateId < PIRATE_Count; pirateId++)
	{
		const_iterator ei = find(pirateId);
		if(ei == cend())
			continue;
		if(!(*ei).second.IsPirateReady())
			continue;	// Skip non-pirate empires and pirate empires that are not ready for encounters.
		const AuScPrefs& pref = (*ei).second.PreferredScBlueprints();
		for(QHash<AuScPrefs::SCROLE, AuScPref>::const_iterator pi = pref.PreferredBlueprints().cbegin();
				pi != pref.PreferredBlueprints().cend(); pi++)
			for(QList<quint32>::const_iterator bi = (*pi).Blueprints().cbegin(); bi != (*pi).Blueprints().cend(); bi++)
				if(!blueprints.contains(*bi))
					blueprints.append(*bi);
				else
					qDebug() << "DgEmpireHash::PirateScBlueprints" << (*ei).first << (*ei).second.Name() << "blueprint dup"
						<< (*bi) << "role" << AuScPrefs::RoleName(pi.key()) << "prefs" << (*pi).Blueprints().size();
	}
	//qDebug() << "DgEmpireHash::PirateScBlueprints end" << blueprints.size();
}

quint32 DgEmpireHash::PirateEncounterSpacecraft(quint32 empireId, GALAXY galaxy, AuScPrefs::SCROLE& scrole) const
{
	const_iterator ei = find(empireId);
	if(ei == cend())
		return 0;
	if(!(*ei).second.IsPirate())
		return 0;
	int ubiquity[PIRATE_Count];
	PirateEmpireUbiquity(galaxy, ubiquity);
	return (*ei).second.PirateEncounterSpacecraft(ubiquity[empireId], scrole);
}

void DgEmpireHash::EmpireCity(quint32 empireId, const AuId& cityId, AuCity& city) const
{
	const_iterator ei = find(empireId);
	if(ei == cend())
		city = AuCity();
	else
	{
		AuCityHash::const_iterator ci = (*ei).second.m_Cities->find(cityId);
		if(ci == (*ei).second.m_Cities->cend())
			city = AuCity();
		else
			city = (*ci).second;
	}
}

void DgEmpireHash::PirateBaseBuildings(quint32 empireId, QList<INDUSTRYID>& buildings) const
{
	const_iterator ei = find(empireId);
	if(ei == cend())
		return;
	(*ei).second.PirateBaseBuildings(buildings);
}

void DgEmpireHash::EmpireIds(QList<quint32>& empireIds) const
{
	for(const_iterator ei = cbegin(); ei != cend(); ei++)
		empireIds.append((*ei).first);
}

DgEmpireHash::const_iterator DgEmpireHash::FindCitizenEmpire(const AuId& citizenId) const
{
	for(const_iterator ei = cbegin(); ei != cend(); ei++)
		if((*ei).second.HasCitizen(citizenId))
			return ei;
	return cend();
}

quint32 DgEmpireHash::HighestId() const
{
	quint32 highest = 0;
	for(const_iterator ei = cbegin(); ei != cend(); ei++)
		if((*ei).first > highest)
			highest = (*ei).first;
	return highest;
}

void DgEmpireHash::ListFriendlies(quint32 empireId, QList<quint32>& friendlies) const
{
	for(const_iterator ei = cbegin(); ei != cend(); ei++)
		if((*ei).second.PoliticalStance(empireId) == POL_Friend)
			friendlies.append((*ei).first);
}

void DgEmpireHash::ListFriends(quint32 empireId, QList<quint32>& friends) const
{
	const_iterator ei = find(empireId);
	if(ei == cend())
		return;
	(*ei).second.ListFriends(friends);
}

bool DgEmpireHash::CitizenOnLine(quint32 empireId)
{
	iterator ei = find(empireId);
	if(ei == end())
		return false;
	return (*ei).second.CitizenOnLine();
}

bool DgEmpireHash::IsCitizenOnLine(quint32 empireId) const
{
	const_iterator ei = find(empireId);
	if(ei == cend())
		return false;
	return (*ei).second.IsCitizenOnLine();
}

