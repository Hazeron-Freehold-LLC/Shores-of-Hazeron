//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auoffice.cpp,v 1.23 2022/02/19 18:20:14 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auoffice.h"
//

// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
// The Q_DECLARE_METATYPE macro is also required.
static int s_AuOfficeMetaType = qRegisterMetaType<AuOffice>();

//static
AuOfficeDomainPlace* AuOfficeDomainPlace::Create(quint8 type)
{
	if(type == AuOfficeDomainCity::PlaceType())
		return new AuOfficeDomainCity;
	else if(type == AuOfficeDomainGalaxy::PlaceType())
		return new AuOfficeDomainGalaxy;
	else if(type == AuOfficeDomainSector::PlaceType())
		return new AuOfficeDomainSector;
	else if(type == AuOfficeDomainSolarSystem::PlaceType())
		return new AuOfficeDomainSolarSystem;
	else if(type == AuOfficeDomainWorld::PlaceType())
		return new AuOfficeDomainWorld;
	else if(type == AuOfficeDomainUniverse::PlaceType())
		return new AuOfficeDomainUniverse;

	qWarning() << "warning: AuOfficeDomainPlace::Create unrecognized type" << type;
	return 0;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

quint64 AuOfficeDomainAtom::MemoryUsage() const
{
	quint64 mem = sizeof(AuOfficeDomainAtom);
	mem += StringMemoryUsage(m_AtomName);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuOfficeDomain::operator>>(QDataStream& out) const
{
	Q_ASSERT(count() <= 0xffff);
	out << (quint16)count();
	for(const_iterator di = cbegin(); di != cend(); di++)
	{
		out << (*di)->Type();
		out << *(*di);
	}
}

void AuOfficeDomain::operator<<(QDataStream& in)
{
	quint16 n;
	in >> n;
	while(n--)
	{
		quint8 type;
		in >> type;
		AuOfficeDomainPlace* place = AuOfficeDomainPlace::Create(type);
		if(place)
		{
			in >> *place;
			append(place);
		}
	}
}

quint64 AuOfficeDomain::MemoryUsage() const
{
	quint64 mem = sizeof(AuOfficeDomain);
	for(const_iterator di = cbegin(); di != cend(); di++)
		mem += (*di)->MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuOffice::AuOffice(const AuOffice& c) : m_Domain(c.m_Domain)
{
	m_AssignedBy = c.m_AssignedBy;
	m_CanAcceptCitySurrender = c.m_CanAcceptCitySurrender;
	m_CanAccessGovtAccount = c.m_CanAccessGovtAccount;
	m_CanBuild = c.m_CanBuild;
	m_CanDelegatePermissions = c.m_CanDelegatePermissions;
	m_CanInvite = c.m_CanInvite;
	m_CanKick = c.m_CanKick;
	m_CanLoadOfficers = c.m_CanLoadOfficers;
	m_CanManageDossier = c.m_CanManageDossier;
	m_CanManageTrade = c.m_CanManageTrade;
	m_CanSendEmpireMail = c.m_CanSendEmpireMail;
	m_CanSetAnnouncements = c.m_CanSetAnnouncements;
	m_CanSetCapital = c.m_CanSetCapital;
	m_CanSetColorTheme = c.m_CanSetColorTheme;
	m_CanSetCommerce = c.m_CanSetCommerce;
	m_CanSetContrailColor = c.m_CanSetContrailColor;
	m_CanSetEmpireFlag = c.m_CanSetEmpireFlag;
	m_CanSetJoinPassword = c.m_CanSetJoinPassword;
	m_CanSetGalaxyCapital = c.m_CanSetGalaxyCapital;
	m_CanSetMfg = c.m_CanSetMfg;
	m_CanSetMfgSpacecraft = c.m_CanSetMfgSpacecraft;
	m_CanSetPlaceNames = c.m_CanSetPlaceNames;
	m_CanSetPoliticalStance = c.m_CanSetPoliticalStance;
	m_CanSetResearch = c.m_CanSetResearch;
	m_CanSetSectorCapital = c.m_CanSetSectorCapital;
	m_CanSetTax = c.m_CanSetTax;
	m_CanSetTitle = c.m_CanSetTitle;
	m_CanSetWarCriteria = c.m_CanSetWarCriteria;
	m_CanUseDiplomacyChannel = c.m_CanUseDiplomacyChannel;
	m_RxFleetMessages = c.m_RxFleetMessages;
	m_RxSensorScans = c.m_RxSensorScans;
	m_Title = c.m_Title;
}

const AuOffice& AuOffice::operator=(const AuOffice& c)
{
	m_AssignedBy = c.m_AssignedBy;
	m_CanAcceptCitySurrender = c.m_CanAcceptCitySurrender;
	m_CanAccessGovtAccount = c.m_CanAccessGovtAccount;
	m_CanBuild = c.m_CanBuild;
	m_CanDelegatePermissions = c.m_CanDelegatePermissions;
	m_CanInvite = c.m_CanInvite;
	m_CanKick = c.m_CanKick;
	m_CanLoadOfficers = c.m_CanLoadOfficers;
	m_CanManageDossier = c.m_CanManageDossier;
	m_CanManageTrade = c.m_CanManageTrade;
	m_CanSendEmpireMail = c.m_CanSendEmpireMail;
	m_CanSetAnnouncements = c.m_CanSetAnnouncements;
	m_CanSetCapital = c.m_CanSetCapital;
	m_CanSetColorTheme = c.m_CanSetColorTheme;
	m_CanSetCommerce = c.m_CanSetCommerce;
	m_CanSetContrailColor = c.m_CanSetContrailColor;
	m_CanSetEmpireFlag = c.m_CanSetEmpireFlag;
	m_CanSetJoinPassword = c.m_CanSetJoinPassword;
	m_CanSetGalaxyCapital = c.m_CanSetGalaxyCapital;
	m_CanSetMfg = c.m_CanSetMfg;
	m_CanSetMfgSpacecraft = c.m_CanSetMfgSpacecraft;
	m_CanSetPlaceNames = c.m_CanSetPlaceNames;
	m_CanSetPoliticalStance = c.m_CanSetPoliticalStance;
	m_CanSetResearch = c.m_CanSetResearch;
	m_CanSetSectorCapital = c.m_CanSetSectorCapital;
	m_CanSetTax = c.m_CanSetTax;
	m_CanSetTitle = c.m_CanSetTitle;
	m_CanSetWarCriteria = c.m_CanSetWarCriteria;
	m_CanUseDiplomacyChannel = c.m_CanUseDiplomacyChannel;
	m_Domain = c.m_Domain;
	m_RxFleetMessages = c.m_RxFleetMessages;
	m_RxSensorScans = c.m_RxSensorScans;
	m_Title = c.m_Title;
	return *this;
}

void AuOffice::operator>>(QDataStream& out) const
{
	quint32 bits = 0;
	bits |= m_CanUseDiplomacyChannel ?		0x00000001 : 0x00000000;
	bits |= m_CanSetEmpireFlag ?			0x00000002 : 0x00000000;
	bits |= m_CanSetJoinPassword ?			0x00000004 : 0x00000000;
	bits |= m_CanSetPoliticalStance ?		0x00000008 : 0x00000000;
	bits |= m_CanSetTitle ?					0x00000020 : 0x00000000;
	bits |= m_CanInvite ?					0x00000040 : 0x00000000;
	bits |= m_CanKick ?						0x00000080 : 0x00000000;
	bits |= m_CanAccessGovtAccount ?		0x00000100 : 0x00000000;
	bits |= m_CanBuild ?					0x00000200 : 0x00000000;
	bits |= m_CanSetMfg ?					0x00001000 : 0x00000000;
	bits |= m_CanSetMfgSpacecraft ?			0x00002000 : 0x00000000;
	bits |= m_CanSetPlaceNames ?			0x00004000 : 0x00000000;
	bits |= m_CanSetResearch ?				0x00008000 : 0x00000000;
	bits |= m_CanDelegatePermissions ?		0x00080000 : 0x00000000;
	bits |= m_CanLoadOfficers ?				0x00100000 : 0x00000000;
	bits |= m_CanSetColorTheme ?			0x00400000 : 0x00000000;

	bits |= m_RxSensorScans ?				0x01000000 : 0x00000000;
	bits |= m_RxFleetMessages ?				0x02000000 : 0x00000000;
	bits |= m_CanAcceptCitySurrender ?		0x04000000 : 0x00000000;
	bits |= m_CanSetAnnouncements ?			0x08000000 : 0x00000000;
	bits |= m_CanSetCapital ?				0x10000000 : 0x00000000;
	bits |= m_CanSetCommerce ?				0x20000000 : 0x00000000;
	bits |= m_CanSetContrailColor ?			0x40000000 : 0x00000000;
	out << bits;

	bits = 0;
	bits |= m_CanSetSectorCapital ?			0x00000002 : 0x00000000;
	bits |= m_CanSetTax ?					0x00000004 : 0x00000000;
	bits |= m_CanSendEmpireMail ?			0x00000008 : 0x00000000;
	bits |= m_CanManageDossier ?			0x00000020 : 0x00000000;
	bits |= m_CanSetWarCriteria ?			0x00000040 : 0x00000000;
	bits |= m_CanManageTrade ?				0x00000080 : 0x00000000;
	bits |= m_CanSetGalaxyCapital ?			0x00000100 : 0x00000000;
	out << bits;

	out << m_Domain;

	out << m_AssignedBy;
	out << m_Title;
}

void AuOffice::operator<<(QDataStream& in)
{
	quint32 bits;
	in >> bits;
	m_CanUseDiplomacyChannel =		bits & 0x00000001;
	m_CanSetEmpireFlag =			bits & 0x00000002;
	m_CanSetJoinPassword =			bits & 0x00000004;
	m_CanSetPoliticalStance =		bits & 0x00000008;
	m_CanSetTitle =					bits & 0x00000020;
	m_CanInvite =					bits & 0x00000040;
	m_CanKick =						bits & 0x00000080;
	m_CanAccessGovtAccount =		bits & 0x00000100;
	m_CanBuild =					bits & 0x00000200;
	m_CanSetMfg =					bits & 0x00001000;
	m_CanSetMfgSpacecraft =			bits & 0x00002000;
	m_CanSetPlaceNames =			bits & 0x00004000;
	m_CanSetResearch =				bits & 0x00008000;
	m_CanDelegatePermissions =		bits & 0x00080000;
	m_CanLoadOfficers =				bits & 0x00100000;
	m_CanSetColorTheme =			bits & 0x00400000;

	m_RxSensorScans =				bits & 0x01000000;
	m_RxFleetMessages =				bits & 0x02000000;
	m_CanAcceptCitySurrender =		bits & 0x04000000;
	m_CanSetAnnouncements =			bits & 0x08000000;
	m_CanSetCapital =				bits & 0x10000000;
	m_CanSetCommerce =				bits & 0x20000000;
	m_CanSetContrailColor =			bits & 0x40000000;

	in >> bits;
	m_CanSetSectorCapital =			bits & 0x00000002;
	m_CanSetTax =					bits & 0x00000004;
	m_CanSendEmpireMail =			bits & 0x00000008;
	m_CanManageDossier =			bits & 0x00000020;
	m_CanSetWarCriteria =			bits & 0x00000040;
	m_CanManageTrade =				bits & 0x00000080;
	m_CanSetGalaxyCapital =			bits & 0x00000100;

	in >> m_Domain;

	in >> m_AssignedBy;
	in >> m_Title;
}

// Returns true if the domain is empty on return.
bool AuOffice::RevokeDomain(const AuId& placeId)
{
	// Remove the place from their domain.
	m_Domain.DeletePlace(placeId);

	if(IsEmperor())
	{
		// The emperor's domain must contain the universe.
		AuOfficeDomainUniverse u;
		if(!m_Domain.IsPlaceInDomain(u.AtomId()))
			m_Domain.Add(u);
	}

	return m_Domain.isEmpty();
}

void AuOffice::SetBossId(const AuId& bossId)
{
	bool wasEmperor = IsEmperor();

	m_AssignedBy = bossId;

	// When changing someone to an emperor who was not previously an emperor, ensure they get all the permissions of an emperor.
	if(!wasEmperor && IsEmperor())
		SetupEmperor();
}

void AuOffice::SetupEmperor()
{
	// This id of 0 is what makes me an emperor.
	m_AssignedBy = AuId(0);
	m_Title = QObject::tr("Emperor");

	// Empire wide permissions.
	m_CanAcceptCitySurrender = true;
	m_CanDelegatePermissions = true;
	m_CanInvite = true;
	m_CanKick = true;
	m_CanManageDossier = true;
	m_CanSendEmpireMail = true;
	m_CanSetAnnouncements = true;
	m_CanSetWarCriteria = true;
	m_CanSetCapital = true;
	m_CanSetColorTheme = true;
	m_CanSetCommerce = true;
	m_CanSetContrailColor = true;
	m_CanSetEmpireFlag = true;
	m_CanSetJoinPassword = true;
	m_CanSetPoliticalStance = true;
	m_CanSetTax = true;
	m_CanSetTitle = true;
	m_CanUseDiplomacyChannel = true;

	// Domain wide permissions.
	m_CanAccessGovtAccount = true;
	m_CanBuild = true;
	m_CanManageTrade = true;
	m_CanSetGalaxyCapital = true;
	m_CanSetMfg = true;
	m_CanSetMfgSpacecraft = true;
	m_CanSetPlaceNames = true;
	m_CanSetResearch = true;
	m_CanSetSectorCapital = true;
	m_CanLoadOfficers = true;
	m_RxFleetMessages = true;
	m_RxSensorScans = true;

	// Domain
	AuOfficeDomainUniverse u;
	if(!m_Domain.IsPlaceInDomain(u.AtomId()))
		m_Domain.Add(u);
}

bool AuOffice::FilterAssignment(AuOffice& office) const
{
	if(!CanDelegate())
		return false;

	// Disable permissions that I cannot grant.
	if(!m_CanAcceptCitySurrender)
		office.m_CanAcceptCitySurrender = 0;

	if(!m_CanSetCapital)
		office.m_CanSetCapital = 0;

	if(!m_CanSetColorTheme)
		office.m_CanSetColorTheme = 0;

	if(!m_CanSetCommerce)
		office.m_CanSetCommerce = 0;

	if(!m_CanSetContrailColor)
		office.m_CanSetContrailColor = 0;

	if(!m_CanSetEmpireFlag)
		office.m_CanSetEmpireFlag = 0;

	if(!m_CanSetJoinPassword)
		office.m_CanSetJoinPassword = 0;

	if(!m_CanSetPoliticalStance)
		office.m_CanSetPoliticalStance = 0;

	if(!m_CanSetTax)
		office.m_CanSetTax = 0;

	if(!m_CanSetTitle)
		office.m_CanSetTitle = 0;

	if(!m_CanDelegatePermissions)
		office.m_CanDelegatePermissions = 0;

	if(!m_CanInvite)
		office.m_CanInvite = 0;

	if(!m_CanKick)
		office.m_CanKick = 0;

	if(!m_CanManageDossier)
		office.m_CanManageDossier = 0;

	if(!m_CanManageTrade)
		office.m_CanManageTrade = 0;

	if(!m_CanSendEmpireMail)
		office.m_CanSendEmpireMail = 0;

	if(!m_CanSetAnnouncements)
		office.m_CanSetAnnouncements = 0;

	if(!m_CanSetWarCriteria)
		office.m_CanSetWarCriteria = 0;

	if(!m_CanUseDiplomacyChannel)
		office.m_CanUseDiplomacyChannel = 0;

	if(!m_CanAccessGovtAccount)
		office.m_CanAccessGovtAccount = 0;

	if(!m_CanBuild)
		office.m_CanBuild = 0;

	if(!m_CanSetMfg)
		office.m_CanSetMfg = 0;

	if(!m_CanSetMfgSpacecraft)
		office.m_CanSetMfgSpacecraft = 0;

	if(!m_CanSetPlaceNames)
		office.m_CanSetPlaceNames = 0;

	if(!m_CanSetResearch)
		office.m_CanSetResearch = 0;

	if(!m_CanLoadOfficers)
		office.m_CanLoadOfficers = 0;

	if(!m_RxFleetMessages)
		office.m_RxFleetMessages = 0;

	if(!m_RxSensorScans)
		office.m_RxSensorScans = 0;

	//DHH govern Filter m_Domain

	return true;
}

quint64 AuOffice::MemoryUsage() const
{
	quint64 mem = sizeof(AuOffice);
	mem += StringMemoryUsage(m_Title);
	mem += m_Domain.MemoryUsage();
	return mem;
}
