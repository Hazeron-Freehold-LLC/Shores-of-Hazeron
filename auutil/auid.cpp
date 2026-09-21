//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auid.cpp,v 1.23 2022/02/19 18:42:32 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auid.h"
//
#include <QDebug>

static int s_AuIdMetaType = AuId::RegisterMetaType();
static int s_AuIdListMetaType = AuIdList::RegisterMetaType();

static AuId s_NullId(0);

//static
int AuId::RegisterMetaType()
{
	// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
	// The Q_DECLARE_METATYPE macro is also required.
	int metaType = qRegisterMetaType<AuId>();

	// Must call qRegisterMetaTypeStreamOperators in order to use this class type in containers of variants.
	qRegisterMetaTypeStreamOperators<AuId>();

	return metaType;
}

void AuId::operator<<(QDataStream& inStream)
{
	quint32 idval;
	inStream >> idval;
	m_Id = idval;
}

void AuId::operator>>(QDataStream& outStream) const
{
	quint32 idval = m_Id;
	outStream << idval;
}

// Sets this id value from a string id formatted using StringId().
void AuId::SetStringId(const QString& sid)
{
	QString upperid = sid.toUpper().trimmed();

	// Parse the number from a backwards base 26 (A-Z) number.
	m_Id = 0;
	if(upperid == "A")
		return;	// Special 0 case.
	int imag = 1;
	for(int chr=0; chr<upperid.count(); chr++)
	{
		int ich = upperid[chr].toLatin1() - 'A';
		if(ich)
			m_Id += ich * imag;
		imag *= 26;
	}
}

QString AuId::StringId() const
{
	return StringLE26(m_Id);
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

QDebug operator<<(QDebug dbg, const AuId& id)
{
	dbg.nospace() << id.GetID();
	return dbg.space();
}

//static
const AuId& AuId::NullId()
{
	return s_NullId;
}

GALAXY AuId::Galaxy() const
{
	for(int gidx=0; gidx<GALAXY_Count; gidx++)
	{
		GALAXY galaxy = (GALAXY)gidx;
		if(GalaxyId(galaxy) == m_Id)
			return galaxy;
	}
	return GALAXY_Count;
}

//static
AuId AuId::GalaxyId(GALAXY galaxy)
{
	if(galaxy < 0 || galaxy >= GALAXY_Count)
		return NullId();

	AuId galaxyIds[] =
	{
		AuId(102),						// AndromedaRising
		AuId(103),						// BlackHole
		AuId(104),						// Core
		AuId(105),						// CrownOfOthon
		AuId(106),						// DyrathonsRetreat
		AuId(107),						// EdgeOfTheRift
		AuId(108),						// FallasEmbrace
		AuId(109),						// FallenLegionsOfMuturon
		AuId(110),						// HeartOfVictorus
		AuId(111),						// HouseZanathar
		AuId(112),						// IndigoSea
		AuId(113),						// InkarBorderRegion
		AuId(114),						// MuturonEncounter
		AuId(115),						// RansuulsFlamingSword
		AuId(116),						// SevenTen
		AuId(117),						// ShoresOfHazeron
		AuId(118),						// ThustrasEye
		AuId(119),						// VeilOfTargoss
		AuId(120),						// VreenoxEclipse
		AuId(121),						// VulcansForge
		AuId(122),						// MilkyWay
	};
	return galaxyIds[galaxy];
}

//static
AuId AuId::FirstUnreserved()
{
	return InsurerId() + 1;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//static
int AuIdList::RegisterMetaType()
{
	// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
	// The Q_DECLARE_METATYPE macro is also required.
	int metaType = qRegisterMetaType<AuIdList>();

	// Must call qRegisterMetaTypeStreamOperators in order to use this class type in containers of variants.
	qRegisterMetaTypeStreamOperators<AuIdList>();

	return metaType;
}

void AuIdList::Remove(const AuId &idRemove)
{
	int idx = indexOf(idRemove);
	if(idx >= 0)
		removeAt(idx);
}

// Returns true if the id is actually added.
bool AuIdList::AddIfUnique(const AuId &id)
{
	if(indexOf(id) >= 0)
		return false;

	append(id);
	return true;
}

void AuIdList::operator<<(QDataStream& inStream)
{
	clear();
	quint32 cnt;
	inStream >> cnt;
	AuId id;
	for(; cnt; cnt--)
	{
		id << inStream;
		append(id);
	}
}

void AuIdList::operator>>(QDataStream& outStream) const
{
	quint32 cnt = count();
	outStream << cnt;
	for(const_iterator it=constBegin(); it != constEnd(); it++)
		(*it) >> outStream;
}

