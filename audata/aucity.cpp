//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucity.cpp,v 1.25 2022/02/19 18:19:59 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucity.h"
//
#include "auglobal.h"

void AuCity::operator>>(QDataStream& out) const
{
	m_idSector >> out;
	m_idSystem >> out;
	m_idPlanet >> out;
	m_idCity >> out;
	out << m_sSector;
	out << m_sSystem;
	out << m_sPlanet;
	out << m_sCity;

	quint8 hasWhat = 0;
	if(m_SectorCapital)
		hasWhat |= 0x01;

	if(!m_Facilities.empty())
		hasWhat |= 0x02;

	if(!m_SystemLoc.IsZero())
		hasWhat |= 0x04;

	hasWhat |= 0x08;					// version bit says galaxy is now a GALAXY and not a string.
	hasWhat |= m_CityType << 4;			// City type in bottom 2 bits of top nibble.
	if(m_GalaxyCapital)
		hasWhat |= 0x80;
	hasWhat |= 0x40;					// hasWhat2 byte added

	quint8 hasWhat2 = 0;
	if(m_AlertCondition != ConditionGreen)
		hasWhat2 |= 0x01;
	if(m_Morale != 0)
		hasWhat2 |= 0x02;
	if(!m_FounderId.IsNull())
		hasWhat2 |= 0x04;

	out << hasWhat;
	out << hasWhat2;
	out << m_idGalaxy;
	out << (quint8)m_Galaxy;

	if(hasWhat & 0x02)
		out << m_Facilities;

	if(hasWhat & 0x04)
		out << m_SystemLoc;

	if(hasWhat2 & 0x01)
		out << m_AlertCondition;

	if(hasWhat2 & 0x02)
		out << m_Morale;

	if(hasWhat2 & 0x04)
	{
		out << m_FounderId;
		out << m_FounderName;
	}
}

void AuCity::operator<<(QDataStream& in)
{
	m_idSector << in;
	m_idSystem << in;
	m_idPlanet << in;
	m_idCity << in;
	in >> m_sSector;
	in >> m_sSystem;
	in >> m_sPlanet;
	in >> m_sCity;

	quint8 hasWhat, hasWhat2;
	in >> hasWhat;
	if(hasWhat & 0x40)
		in >> hasWhat2;
	else
		hasWhat2 = 0;

	m_SectorCapital = hasWhat & 0x01;
	m_CityType = (hasWhat >> 4) & 0x03;			// City type in bottom 2 bits of top nibble.
	m_GalaxyCapital = hasWhat & 0x80;

	in >> m_idGalaxy;
	if(hasWhat & 0x08)
	{
		quint8 ui8;
		in >> ui8;
		m_Galaxy = (GALAXY)ui8;
	}
	else
	{
		QString galaxyName;
		in >> galaxyName;
		m_Galaxy = GalaxyEnum(galaxyName);
	}

	if(hasWhat & 0x02)
		in >> m_Facilities;

	if(hasWhat & 0x04)
		in >> m_SystemLoc;

	if(hasWhat2 & 0x01)
		in >> m_AlertCondition;

	if(hasWhat2 & 0x02)
		in >> m_Morale;

	if(hasWhat2 & 0x04)
	{
		in >> m_FounderId;
		in >> m_FounderName;
	}
}

bool AuCity::operator==(const AuCity& c) const
{
	if(m_idCity != c.m_idCity)
		return false;
	if(m_Galaxy != c.m_Galaxy)
		return false;
	if(m_CityType != c.m_CityType)
		return false;
	if(m_AlertCondition != c.m_AlertCondition)
		return false;
	if(m_Morale != c.m_Morale)
		return false;
	if(m_GalaxyCapital != c.m_GalaxyCapital)
		return false;
	if(m_idPlanet != c.m_idPlanet)
		return false;
	if(m_idSystem != c.m_idSystem)
		return false;
	if(m_idSector != c.m_idSector)
		return false;
	if(m_FounderId != c.m_FounderId)
		return false;
	if(m_idGalaxy != c.m_idGalaxy)
		return false;
	if(m_sCity != c.m_sCity)
		return false;
	if(m_sPlanet != c.m_sPlanet)
		return false;
	if(m_sSystem != c.m_sSystem)
		return false;
	if(m_sSector != c.m_sSector)
		return false;
	if(m_FounderName != c.m_FounderName)
		return false;
	if(m_SectorCapital != c.m_SectorCapital)
		return false;
	if(m_SystemLoc != c.m_SystemLoc)
		return false;
	if(m_Facilities != c.m_Facilities)
		return false;
	return true;
}

QString AuCity::CityIcon() const
{
	return CityIcon(CityType());
}

//static
QString AuCity::CityIcon(CITYTYPE cityType)
{
	if(cityType < 0 || cityType > CITY_MilitaryBase)
		return "";
	QString icons[] =
	{
		QString(":data/MsgCity.png"),
		QString(":data/City.png"),
		QString(":data/MilitaryBase.png"),
	};
	return icons[cityType];
}

quint64 AuCity::MemoryUsage() const
{
	quint64 mem = sizeof(AuCity);
	mem += m_Facilities.MemoryUsage();
	mem += StringMemoryUsage(m_sSector);
	mem += StringMemoryUsage(m_sSystem);
	mem += StringMemoryUsage(m_sPlanet);
	mem += StringMemoryUsage(m_sCity);
	mem += StringMemoryUsage(m_FounderName);
	return mem;
}

int AuCity::JoinScore(const AuId& empireCapitol) const
{
	int score = 0;
	if(m_idCity == empireCapitol)
		score += 5;
	if(CityType() == CITY_MilitaryBase)
	{
		if(m_Facilities.HasAirBase())
			score += 10;
		if(m_Facilities.HasBaseRadar())
			score += 1;
	}
	else
	{
		if(m_Facilities.HasAirport())
			score += 100;
		if(m_Facilities.HasAirportRadar())
			score += 10;
		if(m_Facilities.HasAirportRepairShop())
			score += 20;
		if(m_Facilities.HasSpacecraftFactory())
			score += 50;
		if(IsGalaxyCapital())
			score += 2;
		if(IsSectorCapital())
			score += 1;
	}
	return score;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuCityHash::operator>>(QDataStream& out) const
{
	quint16 count = (quint16)size();
	out << count;
	for(const_iterator it = cbegin(); it != cend(); it++)
		out << (*it).second;
}

void AuCityHash::operator<<(QDataStream& in)
{
	clear();
	quint16 count;
	in >> count;
	while(count--)
	{
		AuCity ke;
		in >> ke;
		insert(pair<AuId, AuCity>(ke.CityId(), ke));
	}
}

bool AuCityHash::Reconcile(quint32 empireId, const unordered_map<AuId, quint32, AuIdHash>& allCities)
{
	bool anyChange = false;
	iterator ci = begin();
	while(ci != end())
	{
		unordered_map<AuId, quint32, AuIdHash>::const_iterator aci = allCities.find((*ci).first);
		if(aci == allCities.cend() || (*aci).second != empireId)
		{
			// City does not exist or it no longer belongs to this empire.
			//if(empireId > 0 && empireId < 16)	// Is it a pirate
				//qDebug() << "  AuCityHash::Reconcile removing city" << (*ci).second.CityId() << (*ci).second.CityName();
			ci = erase(ci);
			anyChange = true;
		}
		else
			ci++;												// City exists and it belongs to this empire.
	}
	//if(empireId > 0 && empireId < 16)	// Is it a pirate
		//qDebug() << "AuCityHash::Reconcile empire" << empireId << "cities" << size() << "after reconcile";
	return anyChange;
}

bool AuCityHash::AnyInSector(const AuId& sectorId) const
{
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
		if((*ci).second.SectorId() == sectorId)
			return true;
	return false;
}

bool AuCityHash::AnyInSystem(const AuId& systemId) const
{
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
		if((*ci).second.SystemId() == systemId)
			return true;
	return false;
}

// Returns id of a city on 'worldId', if any.
AuId AuCityHash::CityOnWorld(const AuId& worldId) const
{
	if(empty())
		return AuId::NullId();

	for(const_iterator ci = cbegin(); ci != cend(); ci++)
		if((*ci).second.PlanetId() == worldId)
			return (*ci).first;

	return AuId::NullId();
}

quint32 AuCityHash::CitiesInGalaxy(const AuId& galaxy) const
{
	quint32 cities = 0;
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it).second.GalaxyId() == galaxy)
			cities++;
	return cities;
}

quint32 AuCityHash::SolarSystemsInGalaxy(const AuId& galaxy) const
{
	QSet<AuId> systems;
	for(const_iterator it = cbegin(); it != cend(); it++)
		if((*it).second.GalaxyId() == galaxy)
			systems.insert((*it).second.SystemId());
	return systems.size();
}

AuId AuCityHash::Random(const QSet<AuId>& exceptIds) const
{
	AuIdList ids;
	for(const_iterator it = cbegin(); it != cend(); it++)
		if(!exceptIds.contains((*it).first))
			ids.append((*it).first);
	if(ids.empty())
		return AuId(0);
	if(ids.size() == 1)
		return ids.first();
	int randidx = g_Au.Dice().Roll(1, ids.size(), -1);
	return ids[randidx];
}

quint64 AuCityHash::MemoryUsage() const
{
	quint64 mem = sizeof(AuCityHash);
	for(const_iterator it = cbegin(); it != cend(); it++)
		mem += sizeof(AuId) + (*it).second.MemoryUsage();
	return mem;
}

AuId AuCityHash::BaseInSystem(const AuId& systemId) const
{
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
	{
		if((*ci).second.CityType() != CITY_MilitaryBase)
			continue;
		if((*ci).second.SystemId() != systemId)
			continue;
		return (*ci).first;
	}
	return AuId();
}

AuId AuCityHash::BaseSystemInSector(const AuId& sectorId) const
{
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
	{
		if((*ci).second.SectorId() != sectorId)
			continue;
		if((*ci).second.CityType() != CITY_MilitaryBase)
			continue;
		return (*ci).second.SystemId();
	}
	return AuId();
}

AuId AuCityHash::Joinable(const AuId& empireCapitol) const
{
	if(!empireCapitol.IsNull())
	{
		// Capital city gets highest precendence.
		const_iterator capI = find(empireCapitol);
		if(capI != cend())
			return empireCapitol;
	}

	// Remaining cities are scored based on facilities available.
	QMap<int, AuId> cities;	// Key is score.
	for(const_iterator ci = cbegin(); ci != cend(); ci++)
		cities.insert((*ci).second.JoinScore(empireCapitol), (*ci).first);
	if(cities.empty())
		return AuId();	// No cities?
	return cities.last();
}

