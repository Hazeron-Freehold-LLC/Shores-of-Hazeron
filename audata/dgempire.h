//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dgempire.h,v 1.155 2022/02/19 18:19:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DgEmpire_H
#define DgEmpire_H

#include "aubdpref.h"
#include "aucityreport.h"
#include "audata.h"
#include "audossier.h"
#include "auglsl.h"
#include "auid.h"
#include "auknownpatents.h"
#include "auoffice.h"
#include "auscface.h"
#include "auscpref.h"
#include "auspecies.h"
#include "autime.h"
#include <QColor>
#include <QDataStream>
#include <QImage>
#include <QString>

using namespace std;
#include <unordered_map>

class AuCity;
class AuCityHash;
class DgEmpireHash;
class QComboBox;
class QSqlDatabase;

// Ids of the pirate empires.
enum PIRATE_EMPIRES
{
	PIRATE_Invalid,
	PIRATE_Akson,
	PIRATE_Balorite,
	PIRATE_Dendrae,
	PIRATE_Haxu,
	PIRATE_Klatra,
	PIRATE_Malacon,
	PIRATE_Myntaka,
	PIRATE_Ogar,
	PIRATE_Otari,
	PIRATE_Seledon,
	PIRATE_Syth,
	PIRATE_Tassad,
	PIRATE_Vilmorti,
	PIRATE_Vreen,
	PIRATE_Zuul,
	PIRATE_Count
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// This class is used to get the list of all the citizens info in an empire.
class AUDATA_DLL AuCitizen
{
public:
	quint8								m_Gender;				// 0=Female, 1=Male
	AuId								m_id;					// Id of the person atom.
	QString								m_name;					// Name of person atom.
	AuOffice*							m_Office;				// Office, domain, and permissions. 0 for non-govt citizens.

public:
	AuCitizen()
	{
		m_Gender = 0;
		m_Office = 0;
	}

	~AuCitizen()
	{
		if(m_Office)
			delete m_Office;
	}

	AuCitizen(const AuCitizen& c)
	{
		m_id = c.m_id;
		m_name = c.m_name;
		m_Gender = c.m_Gender;

		if(c.m_Office)
			m_Office = new AuOffice(*c.m_Office);
		else
			m_Office = 0;
	}

public:
	const AuCitizen& operator=(const AuCitizen& c)
	{
		m_id = c.m_id;
		m_name = c.m_name;
		m_Gender = c.m_Gender;

		if(m_Office)
			delete m_Office;
		if(c.m_Office)
			m_Office = new AuOffice(*c.m_Office);
		else
			m_Office = 0;

		return *this;
	}

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	inline bool operator==(const AuCitizen& c) const
	{
		if(m_id != c.m_id)
			return false;
		return true;
	}

public:
	inline void AssignOffice(const AuOffice& office)
	{
		if(m_Office)
			delete m_Office;
		m_Office = new AuOffice(office);
	}

	// Only valid for people who hold office. Regular citizens return null id.
	inline const AuId& BossId() const
	{
		if(!m_Office)
			return AuId::NullId();
		return m_Office->BossId();
	}

	// True if this citizen is a governor and they have the authority to accept surrendered cities into the empire.
	inline bool CanAcceptCitySurrender() const
	{
		if(!m_Office)
			return false;
		return m_Office->CanAcceptCitySurrender();
	}

	inline bool CanAccessGovtAccount() const
	{
		if(!m_Office)
			return false;
		return m_Office->CanAccessGovtAccount();
	}

	// True if this citizen is a governor and they have the authority to delegate their power.
	inline bool CanDelegate() const
	{
		if(!m_Office)
			return false;
		return m_Office->CanDelegate();
	}

	inline bool CanInviteCitizens() const
	{
		if(!m_Office)
			return false;
		return m_Office->CanInviteCitizens();
	}

	inline bool CanKickCitizens() const
	{
		if(!m_Office)
			return false;
		return m_Office->CanKickCitizens();
	}

	// 'placeIds' contains the relevant atom ids for domain testing.
	inline bool CanLoadOfficer(const AuIdList& placeIds) const
	{
		if(!m_Office)
			return false;
		return m_Office->CanLoadOfficer(placeIds);
	}

	inline bool CanManageDossier() const
	{
		if(!m_Office)
			return false;
		return m_Office->CanManageDossier();
	}

	inline bool CanManageTrade() const
	{
		if(!m_Office)
			return false;
		return m_Office->CanManageTrade();
	}

	inline bool CanReceiveFleetMessages() const
	{
		if(!m_Office)
			return false;
		return m_Office->CanReceiveFleetMessages();
	}

	inline bool CanSendEmpireMail() const
	{
		if(!m_Office)
			return false;
		return m_Office->CanSendEmpireMail();
	}

	inline bool CanSetAnnouncements() const
	{
		if(!m_Office)
			return false;
		return m_Office->CanSetAnnouncements();
	}

	inline bool CanSetWarCriteria() const
	{
		if(!m_Office)
			return false;
		return m_Office->CanSetWarCriteria();
	}

	inline bool CanSetCapital() const
	{
		if(!m_Office)
			return false;
		return m_Office->CanSetCapital();
	}

	inline bool CanSetColorTheme() const
	{
		if(!m_Office)
			return false;
		return m_Office->CanSetColorTheme();
	}

	inline bool CanSetCommerce() const
	{
		if(!m_Office)
			return false;
		return m_Office->CanSetCommerce();
	}

	inline bool CanSetContrailColor() const
	{
		if(!m_Office)
			return false;
		return m_Office->CanSetContrailColor();
	}

	inline bool CanSetEmpireFlag() const
	{
		if(!m_Office)
			return false;
		return m_Office->CanSetEmpireFlag();
	}

	// Returns true for emperor and anyone with permission to set sector capitals.
	inline bool CanSetGalaxyCapital() const
	{
		if(!m_Office)
			return false;
		return m_Office->CanSetGalaxyCapital();
	}

	inline bool CanSetGalaxyCapital(const AuIdList& placeIds) const
	{
		if(!m_Office)
			return false;
		return m_Office->CanSetGalaxyCapital(placeIds);
	}

	inline bool CanSetJoinPassword() const
	{
		if(!m_Office)
			return false;
		return m_Office->CanSetJoinPassword();
	}

	inline bool CanSetPlaceNames() const
	{
		if(!m_Office)
			return false;
		return m_Office->CanSetPlaceNames();
	}

	inline bool CanSetPoliticalStance() const
	{
		if(!m_Office)
			return false;
		return m_Office->CanSetPoliticalStance();
	}

	// Returns true for emperor and anyone with permission to set sector capitals.
	inline bool CanSetSectorCapital() const
	{
		if(!m_Office)
			return false;
		return m_Office->CanSetSectorCapital();
	}

	inline bool CanSetSectorCapital(const AuIdList& placeIds) const
	{
		if(!m_Office)
			return false;
		return m_Office->CanSetSectorCapital(placeIds);
	}

	inline bool CanSetTax() const
	{
		if(!m_Office)
			return false;
		return m_Office->CanSetTax();
	}

	inline bool CanUseDiplomacyChannel() const
	{
		if(!m_Office)
			return false;
		return m_Office->CanUseDiplomacyChannel();
	}

	inline bool FilterAssignment(AuOffice& office) const
	{
		if(!m_Office)
			return false;
		return m_Office->FilterAssignment(office);
	}

	// 0=Female, 1=Male, 2=asexual, 3=hermaphrodite
	inline GENDER Gender() const
	{
		return (GENDER)m_Gender;
	}

	inline bool HasOffice() const
	{
		return m_Office;
	}

	// An emperor is any person who holds a government office that was assigned by id 0.
	inline bool IsEmperor() const
	{
		if(!m_Office)
			return false;
		return m_Office->IsEmperor();
	}

	// Returns true if the 'placeId' is specifically in the office's domain list. Not chain of command hierarchy.
	inline bool IsPlaceInOfficeDomain(const AuId& placeId) const
	{
		if(!m_Office)
			return false;
		return m_Office->IsPlaceInDomain(placeId);
	}

	// 'placeIds' contains the relevant atom ids for domain testing.
	inline bool IsPlaceHierarchyInDomain(const AuIdList& placeIds) const
	{
		if(!m_Office)
			return false;
		return m_Office->IsPlaceHierarchyInDomain(placeIds);
	}

	quint64 MemoryUsage() const;

	inline const QString& Name() const
	{
		return m_name;
	}

	inline QString NameWithTitle() const
	{
		QString tof = TitleOfOffice();
		if(tof.isEmpty())
			return m_name;

		return tof + " " + m_name;
	}

	inline AuOffice Office() const
	{
		if(!m_Office)
			return AuOffice();
		return *m_Office;
	}

	inline bool ReceivesSensorScans() const
	{
		if(!m_Office)
			return false;
		return m_Office->ReceivesSensorScans();
	}

	inline void RevokeOffice()
	{
		if(!m_Office)
			return;
		delete m_Office;
		m_Office = 0;
	}

	// Only valid for people who hold office. Regular citizens return null id.
	inline void SetBossId(const AuId& bossId)
	{
		if(!m_Office)
			return;
		m_Office->SetBossId(bossId);
	}

	inline quint32 TargetValue() const
	{
		if(!m_Office)
			return 1;	// Ordinary citizen is the baseline, at 1 point.
		return m_Office->TargetValue();
	}

	inline QString TitleOfOffice() const
	{
		if(!m_Office)
			return "";
		return m_Office->TitleOfOffice();
	}

	// Returns the id of the universe atom.
	inline AuId UniverseDomainId() const
	{
		return AuId::Universe();
	}

	// Returns the name of the universe atom.
	inline QString UniverseDomainName() const
	{
		return "Universe";
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL AuKnownEmpire
{
public:
	AuDossier		m_Dossier;
	quint32			m_id;			// Id of the empire.
	QString			m_name;			// Empire name to save lookup later.
	quint8			m_stance;		// POLITICAL_STANCE toward the empire. Cannot be POL_Empire.
	quint8			m_tribute;		// Tribute percentage 0-100 for vassals. N/A for other stances.
	AuTime			m_whenMet;		// When this stance was created.
	AuTime			m_whenSet;		// When this stance was set.

public:	// Constructors
	inline AuKnownEmpire()
	{
		m_id = 0;
		m_stance = 0;
		m_tribute = 0;
	}

	inline AuKnownEmpire(const AuKnownEmpire& c) : m_Dossier(c.m_Dossier)
	{
		m_id = c.m_id;
		m_name = c.m_name;
		m_stance = c.m_stance;
		m_tribute = c.m_tribute;
		m_whenMet = c.m_whenMet;
		m_whenSet = c.m_whenSet;
	}

	inline AuKnownEmpire& operator=(const AuKnownEmpire& c)
	{
		m_Dossier = c.m_Dossier;
		m_id = c.m_id;
		m_name = c.m_name;
		m_stance = c.m_stance;
		m_tribute = c.m_tribute;
		m_whenMet = c.m_whenMet;
		m_whenSet = c.m_whenSet;
		return *this;
	}

public:	// Operators
	inline void operator>>(QDataStream& out) const
	{
		quint32 ui32 = m_id;
		if(!m_Dossier.isEmpty())
			ui32 |= 0x80000000;

		out << ui32;
		out << m_name;
		out << m_stance;
		out << m_tribute;
		out << m_whenMet.UTC();
		out << m_whenSet.UTC();

		if(ui32 & 0x80000000)
			out << m_Dossier;
	}

	inline void operator<<(QDataStream& in)
	{
		quint32 ui32;
		in >> ui32;
		m_id = ui32 & 0x7fffffff;

		in >> m_name;
		in >> m_stance;
		in >> m_tribute;

		QDateTime tm;
		in >> tm;
		tm.setTimeSpec(Qt::UTC);
		m_whenMet = tm;

		in >> tm;
		tm.setTimeSpec(Qt::UTC);
		m_whenSet = tm;

		if(ui32 & 0x80000000)
			in >> m_Dossier;
		else
			m_Dossier.clear();
	}

public:	// Methods
	inline AuTime NewestNewDossierDoc() const
	{
		return m_Dossier.NewestNewDoc();
	}

	// Initializes the dossier in a known empire that did not previously have a dossier.
	// Adds a minimal contact record and a set stance record if the set and met times are different.
	// Returns true if the dossier changes.
	inline bool InitDossier()
	{
		bool anyChange = false;

		if(m_Dossier.isEmpty())
		{
			// Add a contact record.
			AuDocUnitContact contact(m_whenMet);
			m_Dossier.File(contact);
			anyChange = true;
		}

		if(m_Dossier.size() == 1 && m_whenMet != m_whenSet)
		{
			// Add a stance changed record.
			AuDocSetStance stance(m_whenSet, (POLITICAL_STANCE)m_stance);
			m_Dossier.File(stance);
			anyChange = true;
		}

		return anyChange;
	}

	quint64 MemoryUsage() const;

	inline void TxNoDossier(QDataStream& out) const
	{
		quint32 ui32 = m_id;
		//if(!m_Dossier.isEmpty())
			//ui32 |= 0x80000000;

		out << ui32;
		out << m_name;
		out << m_stance;
		out << m_tribute;
		out << m_whenMet.UTC();
		out << m_whenSet.UTC();

		//if(ui32 & 0x80000000)
			//out << m_Dossier;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL DgEmpire
{
friend class DgEmpireHash;
public:
	enum FOUNDER_DOMAIN
	{
		InvalidDomain,
		CityDomain,
		WorldDomain,
		SolarSystemDomain,
		SectorDomain,
		GalaxyDomain,
		UniverseDomain
	};

	enum RIGHT_TO_FOUND
	{
		CannotFound,			// Citizens cannot found cities.
		FoundOnEmpty,			// Citizens can found cities on worlds with no other cities.
		FoundOnEnemy,			// Citizens can found cities on empty worlds and where only enemy cities exist.
		FoundOnNeutral,			// Citizens can found cities on empty worlds and where only neutral or enemy cities exist.
		FoundOnFriendly,		// Citizens can found cities on empty worlds and where only friendly, neutral or enemy cities exist.
		FoundOnAny,				// Citizens can found cities on any worlds.
	};

public:
	quint32								m_id;					// Database record id.
private:
	AuBdPrefs							m_BdPrefs;				// Building blueprint preferences.
	EMPIRE_ROLE							m_GameRole;				// 0=normal empire, 1=pirate empire, 2=admin, 3=arena1, 4=arena2
	quint8								m_immig;				// 0=open, 1=restricted
	QByteArray							m_immigPassCrypt;		// SHA1 encrypted password to join if immig is restricted.
	QString								m_name;					// Name of empire.
	AuScPrefs							m_ScPrefs;				// Spacecraft blueprint preferences.
public:
	QString								m_Announcements;
	AuCityHash*							m_Cities;				// Cities of the empire. Key is city id.
	unordered_map<AuId, AuCitizen, AuIdHash>	m_Citizens;		// All players in the empire.
	QVector<quint8>						m_Commerce;				// index=COMMODITYID, value=EXPORT_POLICY
	quint8								m_ContrailHue;			// Hue of contrails of spacecraft and vehicles. 0-255
	quint8								m_ContrailSat;			// Saturation of contrails of spacecraft and vehicles. 64-191
	AuId								m_capitol;				// Id of bd|city designated as the capital city.
	AuSpecies							m_capitolSpecies;		// Worker species at the capital city.
	quint8								m_DefaultStance;		// 0=friend, 1=neutral, 2=enemy. POL_ values. Cannot be POL_Empire.
	QImage								m_flag;					// Flag image for emperors
	QString								m_JoinMessage;			// Message displayed on join empires page.
	AuTime								m_OnLine;				// Last time a citizen of this empire reported being on line.
	quint32								m_RewCapBase;
	quint32								m_RewCapCapitalEmpire;
	quint32								m_RewCapCapitalGalaxy;
	quint32								m_RewCapCapitalSector;
	quint32								m_RewCapCity;
	quint32								m_RewCapStation;		// per 1000 structure points
	quint32								m_RewCapSpaceship;		// per 1000 structure points
	quint32								m_RewCapStarship;		// per 1000 structure points
	quint32								m_RewDmgStation;		// per 10 damage points inflicted
	quint32								m_RewDmgSpaceship;		// per 10 damage points inflicted
	quint32								m_RewDmgStarship;		// per 10 damage points inflicted
	quint32								m_RewDiscHabitable;		// per habitable world
	quint32								m_RewDiscWorm;
	quint32								m_RewKillEnemy;
	quint32								m_RewSalary;			// for govt officials
	quint32								m_RewSpotCity;
	quint32								m_RewSpotEnemy;
	unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>	m_Stance;	// stance toward known empires. key is empire id
	quint8								m_TaxIncome;			// 1/2 percent increments
	quint8								m_TaxSales;				// 1/2 percent increments
	quint8								m_TaxSubsidy;			// 1/2 percent increments
	QColor								m_ThemeColors[AuScFace::THEME_Count];
	AuWarCriteria						m_WarCriteria;			// Key=AuDocument::DocumentType, value=new incident count for war
private:
	AuKnownPatents						m_Patents;				// Patents known to the empire.

public:	// Constructors
	DgEmpire(const QString& name);
	DgEmpire();
	DgEmpire(const DgEmpire& c);
	~DgEmpire();

public:	// Operators
	const DgEmpire& operator=(const DgEmpire& c);

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	bool operator==(const DgEmpire& c) const
	{
		return m_id == c.m_id;
	}

public:	// Methods
	inline const QString& Announcements() const
	{
		return m_Announcements;
	}

	static inline quint8 ArenaObserverId()
	{
		return 20;
	}

	static inline quint8 ArenaTeam1Id()
	{
		return 21;
	}

	static inline quint8 ArenaTeam2Id()
	{
		return 22;
	}

	// Returns the id of my military base in 'systemId', or AuId() if none.
	AuId BaseInSystem(const AuId& systemId) const;

	// Returns the id of any system in 'sectorId' that contains one of my military bases.
	AuId BaseSystemInSector(const AuId& sectorId) const;

	// Returns the citizen species at the capital city. Returns default human if its not valid.
	AuSpecies CapitolSpecies() const;

	inline EXPORT_POLICY CommodityCommerce(COMMODITYID cid) const
	{
		if(cid < m_Commerce.count())
			return (EXPORT_POLICY)m_Commerce[cid];
		else
			return EXPOL_ToNeutral;	// Default enables export to neutrals, friends and empire.
	}

	// Returns true if any data in this empire record is changed.
	inline bool ChangeEmpireName(const QString& oldName, const QString& nuName)
	{
		// Initialize our return value.
		bool anyChange = false;

		// Am I the empire whose name is changing?
		if(m_name == oldName)
		{
			// Change my name.
			m_name = nuName;
			anyChange = true;
		}

		// Change names in my list of known empires.
		for(unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::iterator it = m_Stance.begin(); it != m_Stance.end(); it++)
			if((*it).second.m_name == oldName)
			{
				(*it).second.m_name = nuName;
				anyChange = true;
			}

		return anyChange;
	}

	quint32 CitiesInGalaxy(const AuId& galaxy) const;

	inline void CitizenAssignOffice(const AuId& avatarId, const AuOffice& office)
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return;

		(*ci).second.AssignOffice(office);
	}

	// True if a citizen is a governor and they have the authority to accept surrendered cities.
	inline bool CitizenCanAcceptCitySurrender(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;

		return (*ci).second.CanAcceptCitySurrender();
	}

	// 'placeIds' contains the relevant atom ids for domain testing.
	inline bool CitizenCanAccessGovtAccount(const AuId& avatarId, const AuIdList& placeIds) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;

		if(!(*ci).second.IsPlaceHierarchyInDomain(placeIds))
			return false;

		return (*ci).second.CanAccessGovtAccount();
	}

	// True if a citizen is a governor and they have the authority to delegate their power.
	inline bool CitizenCanDelegate(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;

		return (*ci).second.CanDelegate();
	}

	inline bool CitizenCanInviteCitizens(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;

		return (*ci).second.CanInviteCitizens();
	}

	inline bool CitizenCanKickCitizens(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;

		return (*ci).second.CanKickCitizens();
	}

	// Returns true if a citizen can load an officer from cityId.
	inline bool CitizenCanLoadOfficer(const AuId& avatarId, const AuIdList& placeIds) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;

		return (*ci).second.CanLoadOfficer(placeIds);
	}

	inline bool CitizenCanManageDossier(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;

		return (*ci).second.CanManageDossier();
	}

	inline bool CitizenCanManageTrade(const AuId& avatarId, const AuIdList& placeIds) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;

		if(!(*ci).second.IsPlaceHierarchyInDomain(placeIds))
			return false;

		return (*ci).second.CanManageTrade();
	}

	inline bool CitizenCanReceiveFleetMessages(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;

		return (*ci).second.CanReceiveFleetMessages();
	}

	inline bool CitizenCanSendEmpireMail(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;

		return (*ci).second.CanSendEmpireMail();
	}

	inline bool CitizenCanSetAnnouncements(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;

		return (*ci).second.CanSetAnnouncements();
	}

	inline bool CitizenCanSetWarCriteria(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;

		return (*ci).second.CanSetWarCriteria();
	}

	inline bool CitizenCanSetCapital(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;

		return (*ci).second.CanSetCapital();
	}

	inline bool CitizenCanSetColorTheme(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;

		return (*ci).second.CanSetColorTheme();
	}

	inline bool CitizenCanSetCommerce(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;

		return (*ci).second.CanSetCommerce();
	}

	inline bool CitizenCanSetContrailColor(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;

		return (*ci).second.CanSetContrailColor();
	}

	inline bool CitizenCanSetEmpireFlag(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;

		return (*ci).second.CanSetEmpireFlag();
	}

	// Returns true for emperor and anyone with permission to set sector capitals.
	inline bool CitizenCanSetGalaxyCapital(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;

		return (*ci).second.CanSetGalaxyCapital();
	}

	inline bool CitizenCanSetGalaxyCapital(const AuId& avatarId, const AuIdList& placeIds) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;

		return (*ci).second.CanSetGalaxyCapital(placeIds);
	}

	inline bool CitizenCanSetJoinPassword(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;

		return (*ci).second.CanSetJoinPassword();
	}

	inline bool CitizenCanSetPlaceNames(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;

		return (*ci).second.CanSetPlaceNames();
	}

	inline bool CitizenCanSetPoliticalStance(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;

		return (*ci).second.CanSetPoliticalStance();
	}

	// Returns true for emperor and anyone with permission to set sector capitals.
	inline bool CitizenCanSetSectorCapital(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;

		return (*ci).second.CanSetSectorCapital();
	}

	inline bool CitizenCanSetSectorCapital(const AuId& avatarId, const AuIdList& placeIds) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;

		return (*ci).second.CanSetSectorCapital(placeIds);
	}

	inline bool CitizenCanSetTax(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;

		return (*ci).second.CanSetTax();
	}

	inline bool CitizenCanUseDiplomacyChannel(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;

		return (*ci).second.CanUseDiplomacyChannel();
	}

	inline bool CitizenChangeBoss(const AuId& governorId, const AuId& bossId)
	{
		// Locate the citizen to change.
		unordered_map<AuId, AuCitizen, AuIdHash>::iterator ci = m_Citizens.find(governorId);
		if(ci == m_Citizens.end())
			return false;	// Citizen not found.

		// The citizen must already hold office.
		if(!(*ci).second.HasOffice())
			return false;	// Citizen does not hold an office.

		// Do not create a circular condition by setting my boss to be someone who is subordinate to me.
		if(CitizenInChainOfCommand(governorId, bossId))
			return false;	// Citizen is in the chain of command of the boss.

		(*ci).second.SetBossId(bossId);
		return true;
	}

	bool CitizenFilterAssignment(const AuId& avatarId, const AuId& assigneeId, AuOffice& office) const;

	// Returns the gender of a citizen 'avatarId'.
	inline GENDER CitizenGender(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci != m_Citizens.end())
			return (*ci).second.Gender();
		return Male;
	}

	// Returns true if 'citizenId' is subordinate to 'avatarId' in the chain-of-command.
	// Citizens who are not governors are not subordinate to anybody so return false.
	inline bool CitizenInChainOfCommand(const AuId& avatarId, const AuId& citizenId) const
	{
		// Get info about the citizen in question. They might not answer to anybody.
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(citizenId);
		if(ci == m_Citizens.end())
			return false;
		if(!(*ci).second.HasOffice())
			return false;				// The citizen holds no office.

		// Get info about the avatar. They might not be in charge of anybody.
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ai = m_Citizens.find(avatarId);
		if(ai == m_Citizens.end())
			return false;
		if(!(*ai).second.HasOffice())
			return false;				// The avatar holds no office.

		// Every governor is in the chain of command of the emperor, including themself.
		if((*ai).second.IsEmperor())
			return true;

		// Check for the avatar in the chain of command of the citizen.
		while(ci != m_Citizens.end())
		{
			// Check the boss of the citizen.
			AuId bossId((*ci).second.BossId());
			if(bossId == (*ai).first)
				return true;			// The avatar is the citizen's boss at some level in the chain of command.

			// Find the boss citizen to work our way up the chain of command from the citizen.
			ci = m_Citizens.find(bossId);
		}

		// Here if we the avatar is not in the chain of command of the citizen.
		return false;
	}

	inline QString CitizenName(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return "";

		return (*ci).second.Name();
	}

	inline QString CitizenNameWithTitle(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return "";

		return (*ci).second.NameWithTitle();
	}

	inline AuOffice CitizenOffice(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return AuOffice();

		return (*ci).second.Office();
	}

	// Citizen reports being on line.
	// Returns true if the time changes in this empire record.
	bool CitizenOnLine();

	// Revokes the citizen's office entirely.
	void CitizenRevokeOffice(const AuId& avatarId);

	// Returns 0 if the citizen does not exist in the empire.
	// Returns 1 for ordinary citizens.
	// Returns > 1 for governors, higher numbers based on domain of all subordinates recursively.
	quint32 CitizenTargetValue(const AuId& avatarId) const;

	inline QString CitizenTitle(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return "";

		return (*ci).second.TitleOfOffice();
	}

	inline void ClearWarCriteria()
	{
		m_WarCriteria.clear();
	}

	inline QColor ContrailColor() const
	{
		QColor color;
		color.setHsvF((float)m_ContrailHue/255.f, (float)m_ContrailSat/255.f, 1);
		return color;
	}

	inline POLITICAL_STANCE DefaultPoliticalStance() const
	{
		return (POLITICAL_STANCE)m_DefaultStance;
	}

	// 'placeIds' contains the relevant atom ids for domain testing.
	// Fills 'avatarIds' with ids of governors who can govern any of the places in 'placeIds'.
	inline void DomainGovernors(const AuIdList& placeIds, AuIdList& avatarIds) const
	{
		for(unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.begin(); ci != m_Citizens.end(); ci++)
			if((*ci).second.IsPlaceHierarchyInDomain(placeIds))
				avatarIds.append((*ci).first);
	}

	// List 'commanders' of 'placeIds' that are authorized to load officers.
	inline void DomainOfficerLoaders(const AuIdList& placeIds, AuIdList& avatarIds) const
	{
		for(unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.begin(); ci != m_Citizens.end(); ci++)
			if((*ci).second.CanLoadOfficer(placeIds))
				avatarIds.append((*ci).first);
	}

	// Returns true if this empire record changes.
	bool DossierAddDocument(quint32 dosEmpire, const AuDocument& doc);

	// 'docTime' and 'docType' together are used to identify the exact document.
	inline void DossierChangeState(quint32 dosEmpire, const AuTime& docTime, quint8 docType, quint16 docIndex,
			AuDocument::DOC_STATE docState)
	{
		unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::iterator it = m_Stance.find(dosEmpire);
		if(it == m_Stance.end())
			return;

		(*it).second.m_Dossier.SetDocState(docTime, docType, docIndex, docState);
	}

	// Returns true if the war criteria calls for enemy stance based on the new and open documents in the dossier.
	inline bool DossierCheckWarCriteria(quint32 dosEmpire) const
	{
		if(m_WarCriteria.isEmpty())
			return false;

		unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::const_iterator it = m_Stance.find(dosEmpire);
		if(it == m_Stance.cend())
			return false;

		return (*it).second.m_Dossier.CheckWarCriteria(m_WarCriteria);
	}

	// Returns the id of the citizen who appears to be the emperor.
	AuId EmperorId() const;

	inline static int FlagImageMax()
	{
		return 256;
	}

	// Removes empires from my m_Stances that no longer exist in 'empires'.
	void ForgetForgottenEmpires(const DgEmpireHash& empires);

	inline EMPIRE_ROLE Role() const
	{
		return m_GameRole;
	}

	inline bool HasCitizen(const AuId& personId) const
	{
		return m_Citizens.find(personId) != m_Citizens.end();
	}

	bool HasCityInSector(const AuId& sectorId) const;

	bool HasCityInSystem(const AuId& systemId) const;

	inline bool HasJoinPassword() const
	{
		return !m_immigPassCrypt.isEmpty();
	}

	inline bool HasPatent(COMMODITYID cid, quint8 minQ) const
	{
		return m_Patents.HasPatent(cid, minQ);
	}

	inline bool HasWarpTechnology() const
	{
		return m_Patents.HasWarpTechnology();
	}

	inline unsigned int WarpTechnology() const
	{
		return m_Patents.WarpTechnology();
	}

	AuTime NewestNewDossierDoc() const;

	// Returns Name() with special characters substituted, e.g. '&' becomes '&amp;'.
	QString HtmlName() const;

	inline bool JoinOpen() const
	{
		return m_immig == 0;
	}

	inline bool JoinRestricted() const
	{
		return m_immig == 1;
	}

	// Adds documents to empty dossiers based on original encounter and stance set times.
	// Returns true if the dossier changes.
	bool InitDossier();

	// Initialize war policy to something reasonable.
	void InitWarPolicy();

	// True if empire is an agent.
	inline bool IsAgentEmpire() const
	{
		return m_GameRole == EMPIRE_Agent;
	}

	// True if empire is an arena team or observer.
	inline bool IsArenaEmpire() const
	{
		return m_GameRole == EMPIRE_ArenaTeam || m_GameRole == EMPIRE_ArenaObserver;
	}

	// Returns true if any citizen reported being online recently, like the past fifteen minutes or so.
	bool IsCitizenOnLine() const;

	// Checks for a specific 'Forgotten Empire NN' formatted name.
	inline bool IsForgottenEmpire() const
	{
		// Check for the forgotten empire specific prefix.
		QString unkPrefix("Forgotten Empire ");
		if(m_name.left(unkPrefix.length()) != unkPrefix)
			return false;

		// Check to see if the last part is really a number.
		return QVariant(m_name.mid(unkPrefix.length())).toUInt() > 0 ? true : false;
	}

	inline bool IsGovernor(const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;
		return (*ci).second.HasOffice();
	}

	// Must have a capital city in 'galaxy'. Must be a normal empire.
	bool IsJoinableAsOverlord(const AuId& galaxy) const;

	// Returns true if this empire is joinable at the start of the game by a new avatar.
	bool IsJoinableByNewAvatar() const;

	// Returns true for all pirate (AI-controlled) empires.
	inline bool IsPirate() const
	{
		//return m_GameRole == 1;
		return IsPirate(m_id);		// Client does not receive m_GameRole.
	}

	// Returns true if 'empireId' is a pirate (AI-controlled) empire.
	inline static bool IsPirate(quint32 empireId)
	{
		return empireId > PIRATE_Invalid && empireId < PIRATE_Count;
	}

	// Returns true for pirate empires that are ready for encounters. They will have preferred spacecraft and building designs that
	// are ready for generating encounters.
	bool IsPirateReady() const;

	inline bool IsPlaceInOfficeDomain(const AuId& placeId, const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;
		return (*ci).second.IsPlaceInOfficeDomain(placeId);
	}

	// 'placeIds' contains the relevant atom ids for domain testing.
	inline bool IsPlaceHierarchyInDomain(const AuIdList& placeIds, const AuId& avatarId) const
	{
		unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.find(avatarId);
		if(ci == m_Citizens.end())
			return false;
		return (*ci).second.IsPlaceHierarchyInDomain(placeIds);
	}

	// True if empire is a regular player empire.
	inline bool IsPlayerEmpire() const
	{
		return m_GameRole == EMPIRE_Player;
	}

	// Returns the best city for a new avatar who starts by joining an existing empire.
	AuId JoinableCity() const;

	void ListAllAvatars(QComboBox& cb) const;

	// Appends empires to 'friends' that this empire considers to be a friend. Empires I am friendly toward.
	// Only list explicit friends. Does not list all empires if my default stance is friend.
	void ListFriends(QList<quint32>& friends) const;

	void ListIndustryPatents(AuKnownPatents& patents, INDUSTRYID industryId, const DgData& gd) const;

	inline void ListPatents(AuKnownPatents& patents) const
	{
		patents = m_Patents;
	}

	inline const QString& Name() const
	{
		return m_name;
	}

	AuId CityFounder(const AuId& cityId) const;

	// Returns the boss of a citizen who builds a city in the 'placeIds' domain hierarchy.
	// Order is important here. Starts from the beginning of the placeIds list (world) and works its way to the end (universe).
	inline AuId CityFounderBoss(const AuIdList& placeIds) const
	{
		for(AuIdList::const_iterator pi = placeIds.cbegin(); pi != placeIds.cend(); pi++)
			for(unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.begin(); ci != m_Citizens.end(); ci++)
				if((*ci).second.IsPlaceInOfficeDomain(*pi))
					return (*ci).first;
		return AuId::NullId();
	}

	// Returns id of a city on 'worldId', if any.
	AuId CityOnWorld(const AuId& worldId) const;

	// Returns true if this empire has a city.
	bool HasCity() const;

	quint64 MemoryUsage() const;

	// Return empire id of overlord, if this empire is a vassal.
	inline quint32 Overlord() const
	{
		for(unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::const_iterator it = m_Stance.cbegin();
				it != m_Stance.cend(); it++)
			if((*it).second.m_stance == POL_Vassal)
				return (*it).first;

		// The empire is not a vassal of any known empire.
		return 0;
	}

	// Returns the list of buildings a pirate empire likes to build.
	void PirateBaseBuildings(QList<INDUSTRYID>& buildings) const;

	// Returns an sc spacecraft blueprint id to use for an encounter with a pirate ship.
	// 'ubiquity' is my ubiquity value in the galaxy of the encounter. It could be 10 to 0 or less but I have to return a valid
	//  blueprint anyway.
	// 'scrole' is an output parameter, which is the role of the returned blueprint id.
	quint32 PirateEncounterSpacecraft(int ubiquity, AuScPrefs::SCROLE& scrole) const;

	inline bool PlaceHasGovernor(const AuId& placeId) const
	{
		for(unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.begin(); ci != m_Citizens.end(); ci++)
			if((*ci).second.IsPlaceInOfficeDomain(placeId))
				return true;
		return false;
	}

	// Returns preferred blueprints to use when constructing a building for 'industryId'.
	// Defaults are the AuBdPrefs static defaults.
	AuBdPref PreferredBlueprints(INDUSTRYID industryId) const;

	const AuBdPrefs& PreferredBdBlueprints() const
	{
		return m_BdPrefs;
	}

	const AuScPrefs& PreferredScBlueprints() const
	{
		return m_ScPrefs;
	}

	// Returns preferred blueprints to use when constructing a spacecraft for 'role'.
	// Defaults are the AuScPrefs static defaults.
	AuScPref PreferredBlueprints(AuScPrefs::SCROLE role) const;

	// 'placeIds' contains the relevant atom ids for domain testing.
	// Fills 'avatarIds' with the ids of empire governors who are to receive sensor scans.
	void ReceivesFleetMessages(const AuIdList& placeIds, AuIdList& avatarIds) const;

	// 'placeIds' contains the relevant atom ids for domain testing.
	// Fills 'avatarIds' with the ids of empire governors who are to receive sensor scans.
	void ReceivesSensorScans(const AuIdList& placeIds, AuIdList& avatarIds) const;

	// Removes references to any cities that no longer exist.
	// 'allCities' is all the city ids in the universe, value is empire id of current owner of flag 0.
	// Returns true if this empire record is changed.
	bool ReconcileCities(const unordered_map<AuId, quint32, AuIdHash>& allCities);

	// Returns true if this empire record is changed.
	bool RecordPatent(COMMODITYID patentCID, quint8 patentQ);

	// Returns true if any change is made to this empire record.
	bool RemoveCitizen(const AuId& personId);

	void SelectNewEmperor(const AuId& prevEmperor);

	void Insert(QSqlDatabase& db);
	bool Select(QSqlDatabase& db, unsigned int id);
	void Update(QSqlDatabase& db);

	void Rx(QDataStream& in);
	void Tx(QDataStream& out, bool includeAnnouncements) const;

	inline bool IsEmpireKnown(quint32 empireId) const
	{
		return m_Stance.find(empireId) != m_Stance.end();
	}

	float IncomeTax() const
	{
		// Tax values are stored in 1/2 percent increments.
		return (float)m_TaxIncome / 2.f;
	}

	// Returns the name of a known empire, or blank.
	const QString NameOfKnownEmpire(quint32 empireId) const
	{
		unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::const_iterator it = m_Stance.find(empireId);
		if(it != m_Stance.end())
			return (*it).second.m_name;
		return "";
	}

	// Returns 0 if the patent is unknown.
	quint8 PatentQuality(COMMODITYID patentCID) const;

	inline POLITICAL_STANCE PoliticalStance(quint32 empireId) const
	{
		if(empireId == m_id)
			return POL_Empire;

		if(IsArenaEmpire())
		{
			if(empireId == ArenaObserverId())
				return POL_Friend;
			else if(empireId == ArenaTeam1Id() || empireId == ArenaTeam2Id())
				return POL_Enemy;
			return POL_Neutral;
		}
		else
		{
			// If the empire is known, return the political stance.
			unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::const_iterator it = m_Stance.find(empireId);
			if(it != m_Stance.end())
				return (POLITICAL_STANCE)(*it).second.m_stance;

			// The empire is not known so return the default political stance.
			return (POLITICAL_STANCE)m_DefaultStance;
		}
	}

	// Returns the id of a random city.
	// City id must not be in 'exceptIds'.
	AuId RandomCity(const QSet<AuId>& exceptIds) const;

	float SalesTax() const
	{
		// Tax values are stored in 1/2 percent increments.
		return (float)m_TaxSales / 2.f;
	}

	void SetCommodityCommerce(COMMODITYID cid, EXPORT_POLICY com)
	{
		// The commerce array is indexed using the COMMODITYID.
		// Fill default values up to and including the commodity that is set.
		while(m_Commerce.count() <= cid)
			m_Commerce.append(EXPOL_ToNeutral);
		m_Commerce[cid] = com;	// Set the commodity commerce value.
	}

	void SetDefaultPoliticalStance(POLITICAL_STANCE stance)
	{
		Q_ASSERT(stance != POL_Empire);	// This value is invalid as a default stance.
		m_DefaultStance = stance;
	}

	inline void SetJoinOpen()
	{
		m_immig = 0;
	}

	inline void SetJoinRestricted()
	{
		m_immig = 1;
	}

	// Changes current immig password. Blank disabled password access. Immig 1 then requires invitation only.
	void SetJoinPassCrypt(const QByteArray& passCrypt);
	void SetJoinPassword(const QString& password);

	// 'empire' is the empire toward whom we are setting our stance.
	// 'name' is the name of 'empire'.
	// 'stance' is the stance taken by this empire.
	// 'vassalTributePercent' is relevant when setting our stance to POL_Vassal.
	// 'reason' is an AuDocument::Type() value, for the dossier.
	// 'fromUnit' is the unit in this empire who triggered the stance setting.
	// 'toStance' is the stance taken/held by the 'empire' toward us.
	// 'toUnit' is the unit in 'empire' who triggered this stance setting.
	// 'galaxy', 'sector', 'system' and 'world' identify where this incident occurred. System and world could be blank.
	void SetPoliticalStance(unsigned int empire, const QString& name, POLITICAL_STANCE stance, quint8 vassalTributePercent,
			quint8 reason, const AuId& fromUnit, const QString& fromUnitName,
			POLITICAL_STANCE toStance, const AuId& toUnit, const QString& toUnitName,
			GALAXY galaxy, const QString& sector, const QString& system, const QString& world);

	void SetPreferredBlueprints(INDUSTRYID industryId, const AuBdPref& pref);

	void SetPreferredBlueprints(AuScPrefs::SCROLE role, const AuScPref& pref);

	inline void SetRole(EMPIRE_ROLE gameRole)
	{
		m_GameRole = gameRole;
	}

	// These can be referenced by designs that use theme colors.
	void SetThemeColor(AuScFace::COLOR_THEME theme, const QColor& color);

	// These can be referenced by designs that use theme colors.
	void SetThemeColors(const QColor themeColors[AuScFace::THEME_Count]);

	// 'docType' is the number returned by AuDocument::DocumentType().
	// 'warCount' is the number of new documents of the type in a neutral empire's dossier that triggers an automatic change to
	// enemy political stance toward them.
	inline void SetWarCriteria(quint8 docType, quint8 warCount)
	{
		m_WarCriteria.SetCriteria(docType, warCount);
	}

	quint32 SolarSystemsInGalaxy(const AuId& galaxy) const;

	inline void Subordinates(const AuId& bossId, QMultiMap<QString, AuId>& subs) const
	{
		for(unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Citizens.begin(); ci != m_Citizens.end(); ci++)
			if((*ci).second.BossId() == bossId)
				subs.insert((*ci).second.NameWithTitle(), (*ci).second.m_id);
	}

	// Returns a number in the range 0..100.
	inline float SubsidyTax() const
	{
		// Tax values are stored in 1/2 percent increments.
		return (float)m_TaxSubsidy / 2.f;
	}

	// Returns true if 'passCrypt' matches the encrypted join password.
	bool TestJoinPassCrypt(const QByteArray& passCrypt) const;

	// Returns true if 'password' encrypts to match the encrypted join password.
	bool TestJoinPassword(const QString& password) const;

	inline void ThemeColors(QColor themeColors[AuScFace::THEME_Count]) const
	{
		for(int tc=0; tc<AuScFace::THEME_Count; tc++)
			themeColors[tc] = m_ThemeColors[tc];
	}

	// Return 0-100 tribute percentage of this empire toward 'empireId'.
	inline quint8 TributePercent(quint32 empireId) const
	{
		Q_ASSERT(empireId != m_id);	// Don't call this on itself.

		// If the empire is known, return the political stance.
		unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::const_iterator it = m_Stance.find(empireId);
		if(it != m_Stance.end() && (*it).second.m_stance == POL_Vassal)
			return (*it).second.m_tribute;

		// The empire is not known or I am not a vassal of it.
		return 0;
	}

	// 'vps' is filled with vp name (key) and their atom id (value).
	inline void VicePresidents(QMultiMap<QString, AuId>& vps) const
	{
		AuId emperorId(EmperorId());
		if(!emperorId.IsNull())
			Subordinates(emperorId, vps);
	}

	// Return date/time this empire first established a stance toward 'empireid'.
	inline AuTime WhenMet(quint32 empireId) const
	{
		Q_ASSERT(empireId != m_id);	// Don't call this on itself.

		// If the empire is known, return the political stance.
		unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::const_iterator it = m_Stance.find(empireId);
		if(it != m_Stance.end())
			return (*it).second.m_whenMet;

		// The empire is not known.
		return AuTime();
	}

	// Return date/time this empire last established a stance toward 'empireid'.
	inline AuTime WhenSet(quint32 empireId) const
	{
		Q_ASSERT(empireId != m_id);	// Don't call this on itself.

		// If the empire is known, return the political stance.
		unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::const_iterator it = m_Stance.find(empireId);
		if(it != m_Stance.end())
			return (*it).second.m_whenSet;

		// The empire is not known.
		return AuTime();
	}
};

inline QDataStream& operator<<(QDataStream& out, const DgEmpire& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, DgEmpire& o)
{
	o << in;
	return in;
}

Q_DECLARE_METATYPE(DgEmpire)	// Needed to use this class in queued signal/slot connections. Must also call qRegisterMetaType.

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// key is empire id, data is empire record with matching id
class AUDATA_DLL DgEmpireHash : public unordered_map<quint32, DgEmpire, AuEmpireIdHash>
{
public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:
	// Returns true if any empire has a city in 'systemId'.
	bool AnyCityInSystem(const AuId& systemId) const;

	// Client calls this to remove all 'Forgotten Empire NN' records. These are supplied in response to empire requests from the
	// server when the server has no record of an empire that is requested. They are discarded between servers in hopes that the
	// forgotten empire record will show up there.
	void DiscardForgottenEmpires();

	AuSpecies CapitolSpecies(quint32 empireId) const;

	// Called when a citizen of 'empireId' signals that it is on line.
	// Returns true if 'empireId's record changes.
	bool CitizenOnLine(quint32 empireId);

	// Gets information about a 'cityId' owned by an 'empireId'. City information is returned in 'city'.
	// If the city is owned by a different empire, 'city' is set to the constructor default.
	void EmpireCity(quint32 empireId, const AuId& cityId, AuCity& city) const;

	// Retrieves a list of all the empire ids, which are the keys to this hash.
	void EmpireIds(QList<quint32>& empireIds) const;

	const_iterator FindCitizenEmpire(const AuId& citizenId) const;

	quint32 HighestId() const;

	// Returns true if any citizen of 'empireId' was online recently, like the past fifteen minutes or so.
	bool IsCitizenOnLine(quint32 empireId) const;

	// Appends empires to 'friendlies' that consider 'empireId' to be a friend. Empires friendly toward 'empireId'.
	void ListFriendlies(quint32 empireId, QList<quint32>& friendlies) const;

	// Appends empires to 'friends' that 'empireId' considers to be a friend. Empires 'empireId' is friendly toward.
	void ListFriends(quint32 empireId, QList<quint32>& friends) const;

	AuSpecies RandomSpecies() const;

	// Removes inhabited systems from 'systemIds'.
	// Removes solar system ids from 'systemIds' that contain a city of any empire, according to empire records.
	void RemoveInhabitedSystems(AuIdList& systemIds) const;

	void Load();

	quint64 MemoryUsage() const;

	// Returns the list of buildings a pirate empire likes to build.
	void PirateBaseBuildings(quint32 empireId, QList<INDUSTRYID>& buildings) const;

	// Returns the Bd id of the pirate base HQ in 'systemId', or AuId() if none.
	// 'empireId' is an output parameter, set to the pirate empire whose base was found.
	AuId PirateBaseInSystem(const AuId& systemId, quint32& empireId) const;

	// Returns the id of a solar system contained in 'sectorId' that has a pirate base, or AuId() if none.
	// 'empireId' is an output parameter, set to the pirate empire whose base was found.
	AuId PirateBaseSystemInSector(const AuId& sectorId, quint32& empireId) const;

	// Fills 'blueprints' with all preferred bd blueprint ids from pirate empires that are ready for encounters.
	// Ids are appended to 'blueprints'; it is not cleared.
	void PirateBdBlueprints(QList<quint32>& blueprints) const;

	// Returns the ranking of this pirate empire, or 0 if it is not a pirate empire.
	// Ranking starts at 1, with the least difficult empire Seledon, and goes up to 15, with the most difficult empire Zuul.
	static int PirateEmpireRank(quint32 empireId);

	static void PirateEmpireUbiquity(GALAXY galaxy, int ubiquity[PIRATE_Count]);

	// Returns an sc spacecraft blueprint id to use for an encounter with a pirate ship.
	// 'empireId' is the pirate empire that has been encountered.
	// 'galaxy' is the galaxy in which the encounter occurred.
	// 'scrole' is an output parameter, which is the role of the returned blueprint id.
	quint32 PirateEncounterSpacecraft(quint32 empireId, GALAXY galaxy, AuScPrefs::SCROLE& scrole) const;

	// Fills 'blueprints' with all preferred sc blueprint ids from pirate empires that are ready for encounters.
	// Ids are appended to 'blueprints'; it is not cleared.
	void PirateScBlueprints(QList<quint32>& blueprints) const;

	// Returns the total ubiquity of all pirate empires in 'galaxy' that are ready for encounters.
	int PirateUbiquity(GALAXY galaxy) const;

	AuBdPref PreferredBlueprints(quint32 empireId, INDUSTRYID industryId) const;

	AuScPref PreferredBlueprints(quint32 empireId, AuScPrefs::SCROLE role) const;

	// Randomly select a pirate empire id for an encounter in 'galaxy'. The returned pirate empire will be ready for encounters.
	quint32 RandomPirateEmpire(GALAXY galaxy) const;

	void SetPreferredBlueprints(quint32 empireId, INDUSTRYID industryId, const AuBdPref& pref);

	void SetPreferredBlueprints(quint32 empireId, AuScPrefs::SCROLE role, const AuScPref& pref);
};

inline QDataStream& operator<<(QDataStream& out, const DgEmpireHash& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, DgEmpireHash& o)
{
	o << in;
	return in;
}

#endif // DgEmpire_H
