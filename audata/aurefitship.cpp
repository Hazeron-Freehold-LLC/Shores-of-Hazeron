//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aurefitship.cpp,v 1.5 2022/02/19 18:20:15 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aurefitship.h"
//

static int s_AuRefitShipSetMetaType = AuRefitShip::RegisterMetaType();

//static
int AuRefitShip::RegisterMetaType()
{
	// Must call qRegisterMetaType in order to use this class type in queued signal and slot connections.
	// The Q_DECLARE_METATYPE macro is also required.
	int metaType = qRegisterMetaType<AuRefitShip>();

	// Must call qRegisterMetaTypeStreamOperators in order to use this class type in containers of variants.
	qRegisterMetaTypeStreamOperators<AuRefitShip>();

	return metaType;
}

AuRefitShip::AuRefitShip()
{
	m_Armor = 0;
	m_Capacitor = 0;
	m_CryoBerths = 0;
	m_Fuel = 0;
	m_Hold = 0;
	m_LifeSupport = 0;
	m_ManeuverDrive = 0;
	m_MinQ = 0;
	m_PowerPlant = 0;
	m_Sensor = 0;
	m_Shield = 0;
	m_Skeleton = 0;
}

AuRefitShip::~AuRefitShip()
{
}

AuRefitShip::AuRefitShip(const AuRefitShip& c) : m_Bays(c.m_Bays), m_FTLDrives(c.m_FTLDrives)
{
	m_Armor = c.m_Armor;
	m_Capacitor = c.m_Capacitor;
	m_CryoBerths = c.m_CryoBerths;
	m_Fuel = c.m_Fuel;
	m_Hold = c.m_Hold;
	m_LifeSupport = c.m_LifeSupport;
	m_ManeuverDrive = c.m_ManeuverDrive;
	m_MinQ = c.m_MinQ;
	m_PowerPlant = c.m_PowerPlant;
	m_Sensor = c.m_Sensor;
	m_Shield = c.m_Shield;
	m_Skeleton = c.m_Skeleton;
}

const AuRefitShip& AuRefitShip::operator=(const AuRefitShip& c)
{
	m_Armor = c.m_Armor;
	m_Bays = c.m_Bays;
	m_Capacitor = c.m_Capacitor;
	m_CryoBerths = c.m_CryoBerths;
	m_FTLDrives = c.m_FTLDrives;
	m_Fuel = c.m_Fuel;
	m_Hold = c.m_Hold;
	m_LifeSupport = c.m_LifeSupport;
	m_ManeuverDrive = c.m_ManeuverDrive;
	m_MinQ = c.m_MinQ;
	m_PowerPlant = c.m_PowerPlant;
	m_Sensor = c.m_Sensor;
	m_Shield = c.m_Shield;
	m_Skeleton = c.m_Skeleton;
	return *this;
}

const AuRefitShip& AuRefitShip::operator==(const AuRefitShip& c)
{
	m_Armor = c.m_Armor;
	m_Bays = c.m_Bays;
	m_Capacitor = c.m_Capacitor;
	m_CryoBerths = c.m_CryoBerths;
	m_FTLDrives = c.m_FTLDrives;
	m_Fuel = c.m_Fuel;
	m_Hold = c.m_Hold;
	m_LifeSupport = c.m_LifeSupport;
	m_ManeuverDrive = c.m_ManeuverDrive;
	m_MinQ = c.m_MinQ;
	m_PowerPlant = c.m_PowerPlant;
	m_Sensor = c.m_Sensor;
	m_Shield = c.m_Shield;
	m_Skeleton = c.m_Skeleton;
	return *this;
}

void AuRefitShip::operator>>(QDataStream& out) const
{
	out << m_Armor;
	out << (quint16)m_Bays.size();
	for(QMap<quint16, quint32>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
	{
		out << bi.key();
		out << *bi;
	}
	out << m_Capacitor;
	out << m_CryoBerths;
	out << (quint16)m_FTLDrives.size();
	for(QMap<quint16, quint32>::const_iterator bi = m_FTLDrives.cbegin(); bi != m_FTLDrives.cend(); bi++)
	{
		out << bi.key();
		out << *bi;
	}
	out << m_Fuel;
	out << m_Hold;
	out << m_LifeSupport;
	out << m_ManeuverDrive;
	out << m_MinQ;
	out << m_PowerPlant;
	out << m_Sensor;
	out << m_Shield;
	out << m_Skeleton;
}

void AuRefitShip::operator<<(QDataStream& in)
{
	in >> m_Armor;
	quint16 bays;
	in >> bays;
	while(bays--)
	{
		quint16 id;
		quint32 vol;
		in >> id;
		in >> vol;
		m_Bays.insert(id, vol);
	}
	in >> m_Capacitor;
	in >> m_CryoBerths;
	quint16 ftlDrives;
	in >> ftlDrives;
	while(ftlDrives--)
	{
		quint16 id;
		quint32 vol;
		in >> id;
		in >> vol;
		m_FTLDrives.insert(id, vol);
	}
	in >> m_Fuel;
	in >> m_Hold;
	in >> m_LifeSupport;
	in >> m_ManeuverDrive;
	in >> m_MinQ;
	in >> m_PowerPlant;
	in >> m_Sensor;
	in >> m_Shield;
	in >> m_Skeleton;
}

quint32 AuRefitShip::Volume() const
{
	quint32 volume = 0;
	volume += m_Armor;
	for(QMap<quint16, quint32>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
		volume += *bi;
	volume += m_Capacitor;
	volume += m_CryoBerths;
	for(QMap<quint16, quint32>::const_iterator fi = m_FTLDrives.cbegin(); fi != m_FTLDrives.cend(); fi++)
		volume += *fi;
	volume += m_Fuel;
	volume += m_Hold;
	volume += m_LifeSupport;
	volume += m_ManeuverDrive;
	volume += m_PowerPlant;
	volume += m_Sensor;
	volume += m_Shield;
	volume += m_Skeleton;
	return volume;
}
