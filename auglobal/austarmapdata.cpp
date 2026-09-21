//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austarmapdata.cpp,v 1.26 2022/04/12 20:44:25 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "austarmapdata.h"
//

// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
// The Q_DECLARE_METATYPE macro is also required.
static int s_AuStarMapSpacecraftMetaType = qRegisterMetaType<AuStarMapSpacecraft>();
static int s_AuStarMapSystemSearchMetaType = qRegisterMetaType<AuStarMapSystemSearch>();

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuStarMapSystemSearch::AuStarMapSystemSearch()
{
	// System Name
	m_CaseSense = Qt::CaseInsensitive;
	m_RegEx = false;
	m_SystemCat = true;
	m_SystemName = true;

	// Search Location
	m_Range = 0;
	m_Galaxy = ShoresOfHazeron;

	// Resources
	m_AndOr = false;
	m_MinQ = 1;

	// Stars
	m_Stars = 0;
	m_BlackHole = false;
	m_NeutronStar = false;
	m_StarTypeO = false;
	m_StarTypeB = false;
	m_StarTypeA = false;
	m_StarTypeF = false;
	m_StarTypeG = false;
	m_StarTypeK = false;
	m_StarTypeM = false;

	// Worlds
	m_Worlds = 0;
	m_GasGiantInferno = false;
	m_GasGiantInner = false;
	m_GasGiantHabitable = false;
	m_GasGiantOuter = false;
	m_GasGiantFrigid = false;
	m_GlobeInferno = 0;
	m_GlobeInner = 0;
	m_GlobeHabitable = 0;
	m_GlobeOuter = 0;
	m_GlobeFrigid = 0;
	m_RingInferno = false;
	m_RingInner = false;
	m_RingHabitable = false;
	m_RingOuter = false;
	m_RingFrigid = false;
	m_RingworldArcs = 0;
}

void AuStarMapSystemSearch::operator>>(QDataStream& out) const
{
	quint8 hasWhat = 0;
	if(!m_Name.isEmpty())
		hasWhat |= 0x01;
	if(m_Range > 0)
		hasWhat |= 0x02;
	if(!m_NaturalResources.empty())
	{
		hasWhat |= 0x04;
		if(m_AndOr)
			hasWhat |= 0x20;
	}
	if(m_Stars)
		hasWhat |= 0x08;
	if(m_Worlds)
		hasWhat |= 0x10;
	out << hasWhat;

	// System Name
	if(hasWhat & 0x01)
	{
		out << m_Name;
		quint8 ui8 = m_CaseSense;	// Bottom two bits.
		if(m_RegEx)
			ui8 |= 0x10;
		if(m_SystemCat)
			ui8 |= 0x20;
		if(m_SystemName)
			ui8 |= 0x40;
		out << ui8;
	}

	// Search Location
	if(hasWhat & 0x02)
	{
		out << m_Range;
		out << m_Galaxy;
		out << m_SearchLoc;
	}

	// Resources
	if(hasWhat & 0x04)
	{
		Q_ASSERT(m_NaturalResources.size() < 256);
		out << (quint8)m_NaturalResources.size();
		for(QList<int>::const_iterator ni = m_NaturalResources.cbegin(); ni != m_NaturalResources.cend(); ni++)
			out << (quint16)*ni;
		out << m_MinQ;
	}

	// Stars
	if(hasWhat & 0x08)
	{
		out << m_Stars;
		quint16 ui16 = 0;
		if(m_StarTypeO)
			ui16 |= 0x0001;
		if(m_StarTypeB)
			ui16 |= 0x0002;
		if(m_StarTypeA)
			ui16 |= 0x0004;
		if(m_StarTypeF)
			ui16 |= 0x0008;
		if(m_StarTypeG)
			ui16 |= 0x0010;
		if(m_StarTypeK)
			ui16 |= 0x0020;
		if(m_StarTypeM)
			ui16 |= 0x0040;
		if(m_BlackHole)
			ui16 |= 0x0080;
		if(m_NeutronStar)
			ui16 |= 0x0100;
		out << ui16;
	}

	// Worlds
	if(hasWhat & 0x10)
	{
		out << m_Worlds;

		quint8 gg = 0;
		if(m_GasGiantInferno)
			gg |= 0x01;
		if(m_GasGiantInner)
			gg |= 0x02;
		if(m_GasGiantHabitable)
			gg |= 0x04;
		if(m_GasGiantOuter)
			gg |= 0x08;
		if(m_GasGiantFrigid)
			gg |= 0x10;
		out << gg;

		out << m_GlobeInferno;
		out << m_GlobeInner;
		out << m_GlobeHabitable;
		out << m_GlobeOuter;
		out << m_GlobeFrigid;

		quint8 ring = 0;
		if(m_RingInferno)
			ring |= 0x01;
		if(m_RingInner)
			ring |= 0x02;
		if(m_RingHabitable)
			ring |= 0x04;
		if(m_RingOuter)
			ring |= 0x08;
		if(m_RingFrigid)
			ring |= 0x10;
		out << ring;

		out << m_RingworldArcs;
	}
}

void AuStarMapSystemSearch::operator<<(QDataStream& in)
{
	quint8 hasWhat = 0;
	in >> hasWhat;

	// System Name
	if(hasWhat & 0x01)
	{
		in >> m_Name;
		quint8 ui8;
		in >> ui8;
		m_CaseSense = (Qt::CaseSensitivity)(ui8 & 0x03);	// Bottom two bits.
		m_RegEx = ui8 & 0x10;
		m_SystemCat = ui8 & 0x20;
		m_SystemName = ui8 & 0x40;
	}

	// Search Location
	if(hasWhat & 0x02)
	{
		in >> m_Range;
		in >> m_Galaxy;
		in >> m_SearchLoc;
	}

	// Resources
	m_NaturalResources.clear();
	if(hasWhat & 0x04)
	{
		quint8 c;
		in >> c;
		while(c--)
		{
			quint16 r;
			in >> r;
			m_NaturalResources.append(r);
		}
		in >> m_MinQ;
		m_AndOr = hasWhat & 0x20;
	}

	// Stars
	if(hasWhat & 0x08)
	{
		in >> m_Stars;
		quint16 ui16;
		in >> ui16;
		m_StarTypeO = ui16 & 0x0001;
		m_StarTypeB = ui16 & 0x0002;
		m_StarTypeA = ui16 & 0x0004;
		m_StarTypeF = ui16 & 0x0008;
		m_StarTypeG = ui16 & 0x0010;
		m_StarTypeK = ui16 & 0x0020;
		m_StarTypeM = ui16 & 0x0040;
		m_BlackHole = ui16 & 0x0080;
		m_NeutronStar = ui16 & 0x0100;
	}

	// Worlds
	if(hasWhat & 0x10)
	{
		in >> m_Worlds;

		quint8 gg;
		in >> gg;
		m_GasGiantInferno = gg & 0x01;
		m_GasGiantInner = gg & 0x02;
		m_GasGiantHabitable = gg & 0x04;
		m_GasGiantOuter = gg & 0x08;
		m_GasGiantFrigid = gg & 0x10;

		in >> m_GlobeInferno;
		in >> m_GlobeInner;
		in >> m_GlobeHabitable;
		in >> m_GlobeOuter;
		in >> m_GlobeFrigid;

		quint8 ring;
		in >> ring;
		m_RingInferno = ring & 0x01;
		m_RingInner = ring & 0x02;
		m_RingHabitable = ring & 0x04;
		m_RingOuter = ring & 0x08;
		m_RingFrigid = ring & 0x10;

		in >> m_RingworldArcs;
	}
}

bool AuStarMapSystemSearch::PreScanTest(const QString& systemName, const QString& catalogName, int galaxy,
		const AuPoint& systemLoc) const
{
	if(m_Range > 0)
	{
		// Check distance from search location.
		if(galaxy != m_Galaxy)
			return false;	// Way too far away.
		double dist = m_SearchLoc.Distance(systemLoc);
		if(dist > m_Range)
			return false;	// Too far away.
	}

	if(!m_Name.isEmpty() && (m_SystemName || m_SystemCat))
	{
		// Check names.
		bool foundSys = false;
		if(m_SystemName)
		{
			if((!m_RegEx && systemName.contains(m_Name, m_CaseSense)) ||
					(m_RegEx && systemName.contains(QRegExp(m_Name, m_CaseSense))))
				foundSys = true;	// Found a matching system name.
		}
		if(!foundSys && m_SystemCat)
		{
			if((!m_RegEx && catalogName.contains(m_Name, m_CaseSense)) ||
					(m_RegEx && catalogName.contains(QRegExp(m_Name, m_CaseSense))))
				foundSys = true;	// Found a matching system name.
		}
		if(!foundSys)
			return false;	// Name test(s) failed.
	}

	// Ran the gauntlet so this filter test is successful.
	return true;
}

bool AuStarMapSystemSearch::SystemScanNeeded() const
{
	if(!m_NaturalResources.empty())
		return true;
	if(m_Stars)
		return true;
	if(m_Worlds)
		return true;
	return false;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuStarMapCity::ListEmpires(QSet<quint32>& empiresInMap) const
{
	if(m_Empire)
		empiresInMap.insert(m_Empire);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuStarMapSpacecraft::SetAboardSpacecraft(const QString& shipName)
{
	m_Name = Name();					// Drop current suffix, if any.
	if(!shipName.isEmpty())
		m_Name += ">" + shipName;		// Append non-blank name suffix.
}

bool AuStarMapSpacecraft::IsAboardSpacecraft() const
{
	return m_Name.contains(">");
}

void AuStarMapSpacecraft::SetAtBuilding(const QString& cityOrBuildingName)
{
	m_Name = Name();					// Drop current suffix, if any.
	if(!cityOrBuildingName.isEmpty())
		m_Name += "<" + cityOrBuildingName;	// Append non-blank name suffix.
}

bool AuStarMapSpacecraft::IsAtBuilding() const
{
	return m_Name.contains("<");
}

QString AuStarMapSpacecraft::Name() const
{
	// Test for aboard spacecraft suffix.
	int idx = m_Name.indexOf(">");
	if(idx >= 0)
		return m_Name.left(idx);

	// Test for at building suffix.
	idx = m_Name.indexOf("<");
	if(idx >= 0)
		return m_Name.left(idx);

	// Return full name. No suffix was found.
	return m_Name;
}

QString AuStarMapSpacecraft::AboardName() const
{
	// Test for aboard spacecraft suffix.
	int idx = m_Name.indexOf(">");
	if(idx >= 0)
		return m_Name.mid(idx+1);

	// Test for at building suffix.
	idx = m_Name.indexOf("<");
	if(idx >= 0)
		return m_Name.mid(idx+1);

	// Return blank. No suffix was found.
	return QString();
}

void AuStarMapSpacecraft::ListEmpires(QSet<quint32>& empiresInMap) const
{
	if(m_Empire)
		empiresInMap.insert(m_Empire);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuStarMapSystem::SetDetectedSignalStrength(quint32 signalStrength)
{
	// 'signalStrength' is warp factors engaged in a system x 10, modulated across the neutral zone.
	// warp 1 across the neutral zone is 10 divided by 4 = 2
	// warp 1 in to/out of the neutral zone is 10 divided by 2 = 5
	// warp 1 anywhere else is 10
	quint8 ds;
	if(signalStrength == 0)
		ds = 0;
	else if(signalStrength <= 2)
		ds = 1;
	else if(signalStrength <= 5)
		ds = 2;
	else if(signalStrength <= 7)
		ds = 3;
	else if(signalStrength <= 10)
		ds = 4;
	else if(signalStrength <= 20)
		ds = 5;
	else if(signalStrength <= 50)
		ds = 6;
	else if(signalStrength <= 70)
		ds = 7;
	else if(signalStrength <= 100)
		ds = 8;
	else if(signalStrength <= 200)
		ds = 9;
	else if(signalStrength <= 500)
		ds = 10;
	else if(signalStrength <= 700)
		ds = 11;
	else if(signalStrength <= 1000)
		ds = 12;
	else if(signalStrength <= 2000)
		ds = 13;
	else if(signalStrength <= 5000)
		ds = 14;
	else
		ds = 15;
	m_InfoBits &= ~EnergyDetectedMask;	// Turn off energy detected bits.
	m_InfoBits |= (ds << 4);			// Turn on the bits we want.
}

void AuStarMapSystem::operator>>(QDataStream& out) const
{
	out << (quint16)m_Cities.size();
	for(unordered_map<AuId, AuStarMapCity, AuIdHash>::const_iterator ci = m_Cities.begin(); ci != m_Cities.end(); ci++)
	{
		out << (*ci).first;
		(*ci).second >> out;
	}

	out << m_InfoBits;
	out << (float)m_Loc.x;
	out << (float)m_Loc.y;
	out << (float)m_Loc.z;
	out << m_Name;
	out << m_GenHab;
	out << m_GenSeed;

	out << (quint16)m_Spacecraft.size();
	for(unordered_map<AuId, AuStarMapSpacecraft, AuIdHash>::const_iterator si = m_Spacecraft.begin();
			si != m_Spacecraft.end(); si++)
	{
		out << (*si).first;
		(*si).second >> out;
	}

	out << (quint8)m_Wormholes.count();
	for(QList<AuStarMapWormhole>::const_iterator wi = m_Wormholes.constBegin(); wi != m_Wormholes.constEnd(); wi++)
		(*wi) >> out;
}

void AuStarMapSystem::operator<<(QDataStream& in)
{
	m_Cities.clear();
	quint16 cityCount;
	in >> cityCount;
	while(cityCount--)
	{
		AuId cityId;
		in >> cityId;
		AuStarMapCity gvcity;
		gvcity << in;
		m_Cities.insert(pair<AuId, AuStarMapCity>(cityId, gvcity));
	}

	in >> m_InfoBits;
	float x,y,z;
	in >> x;
	in >> y;
	in >> z;
	m_Loc = AuPoint(x,y,z);
	in >> m_Name;
	in >> m_GenHab;
	in >> m_GenSeed;

	m_Spacecraft.clear();
	quint16 scCount;
	in >> scCount;
	while(scCount--)
	{
		AuId scId;
		in >> scId;
		AuStarMapSpacecraft gvsc;
		gvsc << in;
		m_Spacecraft.insert(pair<AuId, AuStarMapSpacecraft>(scId, gvsc));
	}

	m_Wormholes.clear();
	quint8 whCount;
	in >> whCount;
	while(whCount--)
	{
		AuStarMapWormhole gvwh;
		gvwh << in;
		m_Wormholes.append(gvwh);
	}
}

void AuStarMapSystem::ListEmpires(QSet<quint32>& empiresInMap) const
{
	for(unordered_map<AuId, AuStarMapCity, AuIdHash>::const_iterator ci = m_Cities.begin(); ci != m_Cities.end(); ci++)
		(*ci).second.ListEmpires(empiresInMap);
	for(unordered_map<AuId, AuStarMapSpacecraft, AuIdHash>::const_iterator si = m_Spacecraft.begin();
			si != m_Spacecraft.end(); si++)
		(*si).second.ListEmpires(empiresInMap);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuStarMapSector::ListEmpires(QSet<quint32>& empiresInMap) const
{
	for(unordered_map<AuId, AuStarMapSystem, AuIdHash>::const_iterator si = m_Systems.begin(); si != m_Systems.end(); si++)
		(*si).second.ListEmpires(empiresInMap);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

QString AuStarMapData::SectorNameOfSystem(const AuId& systemId) const
{
	for(unordered_map<AuId, AuStarMapSector, AuIdHash>::const_iterator si = m_Sectors.begin(); si != m_Sectors.end(); si++)
		if((*si).second.m_Systems.find(systemId) != (*si).second.m_Systems.end())
			return (*si).second.m_Name;
	return QString();
}

AuId AuStarMapData::SectorOfSystem(const AuId& systemId) const
{
	for(unordered_map<AuId, AuStarMapSector, AuIdHash>::const_iterator si = m_Sectors.begin(); si != m_Sectors.end(); si++)
		if((*si).second.m_Systems.find(systemId) != (*si).second.m_Systems.end())
			return (*si).first;
	return AuId();
}

bool AuStarMapData::WormholeDest(GALAXY destGalaxy, const AuPoint& wormDestLoc, QString& destSector, QString& destSystem) const
{
	for(unordered_map<AuId, AuStarMapSector, AuIdHash>::const_iterator si = m_Sectors.begin(); si != m_Sectors.end(); si++)
	{
		if((*si).second.Galaxy() != destGalaxy)
			continue;					// Sector is in a different galaxy.

		AuPoint relsector = wormDestLoc - (*si).second.m_Loc;
		if(fabs(relsector.x) > 5 || fabs(relsector.y) > 5 || fabs(relsector.z) > 5)
			continue;					// The destination location cannot be in si.

		destSector = (*si).second.m_Name;
		for(unordered_map<AuId, AuStarMapSystem, AuIdHash>::const_iterator yi = (*si).second.m_Systems.begin();
				yi != (*si).second.m_Systems.end(); yi++)
		{
			if((*yi).second.m_Loc.FuzzyEquals(relsector))
			{
				destSystem = (*yi).second.m_Name;
				return true;
			}
		}
	}
	return false;
}

void AuStarMapData::ListEmpires(QSet<quint32>& empiresInMap) const
{
	for(unordered_map<AuId, AuStarMapSector, AuIdHash>::const_iterator si = m_Sectors.begin(); si != m_Sectors.end(); si++)
		(*si).second.ListEmpires(empiresInMap);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

bool AuStarMapScPos::IsExpired() const
{
	// Spacecraft report in every 10 minutes.
	return m_When.elapsed()/d_MSecPerMin >= 15;
}
