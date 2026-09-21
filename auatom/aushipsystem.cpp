//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aushipsystem.cpp,v 1.77 2022/02/19 17:41:09 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aushipsystem.h"
//
#include "aubdreport.h"
#include "auknownpatents.h"
#include "auscreport.h"

AuShipSystem::AuShipSystem()
{
	m_ConnectedConsumer = -1;
	m_HealthCurrent = 0;
	m_HealthMax = 0;
	m_IsOverclocked = 0;
	m_Module = CID_Invalid;
	m_PowerConsumption = 0;
	m_Quality = 1;
	m_System = 0;
	m_Which = 0;
}

AuShipSystem::AuShipSystem(SYSTEM system, quint16 which, COMMODITYID module, quint8 quality, unsigned int totalUnits,
		int systemIndex)
{
	Q_ASSERT(quality);
	m_ConnectedConsumer = systemIndex;	// The default is for a system to consume the power allocated to it.
	m_HealthCurrent = totalUnits;
	m_HealthMax = totalUnits;
	m_IsOverclocked = false;
	m_Module = module;
	m_PowerConsumption = totalUnits;
	m_Quality = quality;
	m_System = system;
	m_Which = which;
}

AuShipSystem::~AuShipSystem()
{

}

AuShipSystem::AuShipSystem(const AuShipSystem& rhs)
{
	m_ConnectedConsumer = rhs.m_ConnectedConsumer;
	m_HealthCurrent = rhs.m_HealthCurrent;
	m_HealthMax = rhs.m_HealthMax;
	m_IsOverclocked = rhs.m_IsOverclocked;
	m_Module = rhs.m_Module;
	m_PowerConsumption = rhs.m_PowerConsumption;
	m_Quality = rhs.m_Quality;
	m_System = rhs.m_System;
	m_Which = rhs.m_Which;
}

void AuShipSystem::operator>>(QDataStream& out) const
{
	// Statistics
	out << m_System;
	out << m_Which;
	out << m_Quality;
	out << m_IsOverclocked;
	out << m_Module;

	// Power
	out << m_PowerConsumption;

	// Damage
	out << m_HealthCurrent;
	out << m_HealthMax;

	// Connections
	out << m_ConnectedConsumer;
}

void AuShipSystem::operator<<(QDataStream& in)
{
	// AuShipSystemArray uses my quality as a version number.
	bool readTL = m_Quality == 255;

	// Statistics
	in >> m_System;
	in >> m_Which;
	in >> m_Quality;
	if(!m_Quality)
		m_Quality = 1;
	if(readTL)
	{
		quint8 tl;
		in >> tl;
	}
	in >> m_IsOverclocked;
	in >> m_Module;

	// Power
	in >> m_PowerConsumption;

	// Damage
	in >> m_HealthCurrent;
	in >> m_HealthMax;

	// Connections
	in >> m_ConnectedConsumer;
}

// Server sends only a subset of the system data, not connections.
// Just enough so the client can determine the quality of each system.
void AuShipSystem::Tx(QDataStream& toClient) const
{
	toClient << m_System;
	toClient << m_Which;
	toClient << m_Quality;
	toClient << m_Module;
	toClient << m_PowerConsumption;
}

// Client gets only a subset of the system data, not connections.
// Just enough so the client can determine the quality of each system.
void AuShipSystem::Rx(QDataStream& fromServer)
{
	fromServer >> m_System;
	fromServer >> m_Which;
	fromServer >> m_Quality;
	if(!m_Quality)
		m_Quality = 1;
	fromServer >> m_Module;
	fromServer >> m_PowerConsumption;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuShipSystemArray::AuShipSystemArray()
{
	m_CapacitorModule = CID_CapacitorModule;
	m_CapacitorQuality = 1;
	m_CnCModule = CID_CommandAndControlModule;
	m_CnCQuality = 1;
	m_HullModule = CID_MetalHullModule;
	m_HullQuality = 1;
	m_PowerPlantModule = CID_HydrogenPowerPlantModule;
	m_PowerPlantQuality = 1;
}

void AuShipSystemArray::operator>>(QDataStream& out) const
{
	quint16 countver = 0xc000 | count();	// b15=capacitor added, b14=TL nerfed.
	out << countver;
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		(*it) >> out;

	out << m_CapacitorModule;
	out << m_CapacitorQuality;
	out << m_CnCModule;
	out << m_CnCQuality;
	out << m_HullModule;
	out << m_HullQuality;
	out << m_PowerPlantModule;
	out << m_PowerPlantQuality;
}

void AuShipSystemArray::operator<<(QDataStream& in)
{
	quint16 countver;
	in >> countver;

	bool tlNerfed = countver & 0x4000;

	clear();
	quint16 num = countver & 0x3fff;
	for(;num;num--)
	{
		AuShipSystem ss;
		if(!tlNerfed)
			ss.SetReadTL();
		ss << in;
		append(ss);
	}

	quint8 tl;
	if(countver & 0x8000)
	{
		in >> m_CapacitorModule;
		in >> m_CapacitorQuality;
		if(!m_CapacitorQuality)
			m_CapacitorQuality = 1;
		if(!tlNerfed)
			in >> tl;
	}
	else
	{
		m_CapacitorModule = CID_CapacitorModule;
		m_CapacitorQuality = 255;
	}

	in >> m_CnCModule;
	in >> m_CnCQuality;
	if(!m_CnCQuality)
		m_CnCQuality = 1;
	if(!tlNerfed)
		in >> tl;
	in >> m_HullModule;
	in >> m_HullQuality;
	if(!m_HullQuality)
		m_HullQuality = 1;
	if(!tlNerfed)
		in >> tl;
	in >> m_PowerPlantModule;
	in >> m_PowerPlantQuality;
	if(!tlNerfed)
		in >> tl;
}

void AuShipSystemArray::Rx(QDataStream& fromServer)
{
	clear();
	quint16 num;
	fromServer >> num;
	for(;num;num--)
	{
		AuShipSystem ss;
		ss.Rx(fromServer);
		append(ss);
	}

	fromServer >> m_CapacitorModule;
	fromServer >> m_CapacitorQuality;
	if(!m_CapacitorQuality)
		m_CapacitorQuality = 1;
	fromServer >> m_CnCModule;
	fromServer >> m_CnCQuality;
	if(!m_CnCQuality)
		m_CnCQuality = 1;
	fromServer >> m_HullModule;
	fromServer >> m_HullQuality;
	if(!m_HullQuality)
		m_HullQuality = 1;
	fromServer >> m_PowerPlantModule;
	fromServer >> m_PowerPlantQuality;
	if(!m_PowerPlantQuality)
		m_PowerPlantQuality = 1;
}

void AuShipSystemArray::Tx(QDataStream& toClient) const
{
	toClient << (quint16)count();
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		(*it).Tx(toClient);

	toClient << m_CapacitorModule;
	toClient << m_CapacitorQuality;
	toClient << m_CnCModule;
	toClient << m_CnCQuality;
	toClient << m_HullModule;
	toClient << m_HullQuality;
	toClient << m_PowerPlantModule;
	toClient << m_PowerPlantQuality;
}

COMMODITYID AuShipSystemArray::SystemModule(SYSTEM system, const quint16 which) const
{
	switch(system)
	{
	case SYS_CommandAndControl:
		return (COMMODITYID)m_CnCModule;

	case SYS_Fuel:
	case SYS_Hold:
	case SYS_Hull:
		return (COMMODITYID)m_HullModule;

	case SYS_VehicleLaunch:
		return CID_MechanicalPart;

	case SYS_VehicleLanding:
		return CID_ElectronicPart;

	case SYS_Count:
		break;	// Its ok to have no module then no part damage gets inflicted to a system.

	case SYS_Capacitor:
		return (COMMODITYID)m_CapacitorModule;

	case SYS_PowerPlant:
		return (COMMODITYID)m_PowerPlantModule;

	case SYS_ManeuverDrive:
	case SYS_LifeSupport:
	case SYS_Sensor:
	case SYS_Shield:
	case SYS_SurgeryUnit:
	case SYS_Transporter:
	case SYS_Turret:
	case SYS_Weapon:
	case SYS_FTLDrive:
		for(const_iterator si = constBegin(); si != constEnd(); si++)
			if((*si).GetSystemType() == system && (*si).GetSystemWhich() == which)
				return (*si).Module();
		break;
	}
	return CID_Invalid;
}

COMMODITYID AuShipSystemArray::SystemModule(SYSTEM system) const
{
	switch(system)
	{
	case SYS_CommandAndControl:
		return (COMMODITYID)m_CnCModule;

	case SYS_Fuel:
	case SYS_Hold:
	case SYS_Hull:
		return (COMMODITYID)m_HullModule;

	case SYS_VehicleLaunch:
		return CID_MechanicalPart;

	case SYS_VehicleLanding:
		return CID_ElectronicPart;

	case SYS_Count:
		break;	// Its ok to have no module then no part damage gets inflicted to a system.

	case SYS_Capacitor:
		return (COMMODITYID)m_CapacitorModule;

	case SYS_PowerPlant:
		return (COMMODITYID)m_PowerPlantModule;

	case SYS_ManeuverDrive:
	case SYS_LifeSupport:
	case SYS_Sensor:
	case SYS_Shield:
	case SYS_SurgeryUnit:
	case SYS_Transporter:
	case SYS_Turret:
	case SYS_Weapon:
	case SYS_FTLDrive:
		for(const_iterator si = constBegin(); si != constEnd(); si++)
			if((*si).GetSystemType() == system)
				return (*si).Module();
		break;
	}
	return CID_Invalid;
}

quint8 AuShipSystemArray::SystemQuality(SYSTEM system, const quint16 which) const
{
	switch(system)
	{
	case SYS_CommandAndControl:
		return m_CnCQuality;

	case SYS_Fuel:
	case SYS_Hold:
	case SYS_Hull:
	case SYS_Count:
		return m_HullQuality;

	case SYS_Capacitor:
		return m_CapacitorQuality;

	case SYS_PowerPlant:
		return m_PowerPlantQuality;

	case SYS_ManeuverDrive:
	case SYS_LifeSupport:
	case SYS_Sensor:
	case SYS_Shield:
	case SYS_SurgeryUnit:
	case SYS_Transporter:
	case SYS_Turret:
	case SYS_Weapon:
	case SYS_FTLDrive:
	case SYS_VehicleLaunch:
	case SYS_VehicleLanding:
		for(const_iterator si = constBegin(); si != constEnd(); si++)
			if((*si).GetSystemType() == system && (*si).GetSystemWhich() == which)
				return (*si).Quality();
		break;
	}
	return 1;
}

quint8 AuShipSystemArray::SystemQuality(SYSTEM system) const
{
	switch(system)
	{
	case SYS_CommandAndControl:
		return m_CnCQuality;

	case SYS_Fuel:
	case SYS_Hold:
	case SYS_Hull:
	case SYS_Count:
		return m_HullQuality;

	case SYS_Capacitor:
		return m_CapacitorQuality;

	case SYS_PowerPlant:
		return m_PowerPlantQuality;

	case SYS_ManeuverDrive:
	case SYS_LifeSupport:
	case SYS_Sensor:
	case SYS_Shield:
	case SYS_SurgeryUnit:
	case SYS_Transporter:
	case SYS_Turret:
	case SYS_Weapon:
	case SYS_FTLDrive:
	case SYS_VehicleLaunch:
	case SYS_VehicleLanding:
		for(const_iterator si = constBegin(); si != constEnd(); si++)
			if((*si).GetSystemType() == system)
				return (*si).Quality();
		break;
	}
	return 1;
}

unsigned int AuShipSystemArray::GetPowerConsumption(SYSTEM system, const quint16 which) const
{
	for(const_iterator si = constBegin(); si != constEnd(); si++)
		if((*si).GetSystemType() == system && (*si).GetSystemWhich() == which)
			return (*si).GetPowerConsumptionCurrent();
	return 0;
}

unsigned int AuShipSystemArray::GetPowerConsumption(SYSTEM system) const
{
	for(const_iterator si = constBegin(); si != constEnd(); si++)
		if((*si).GetSystemType() == system)
			return (*si).GetPowerConsumptionCurrent();
	return 0;
}

void AuShipSystemArray::GatherShipSystems(quint8 quality, const AuScReport& design)
{
	Q_ASSERT(quality);
	clear();

	// Store quality of systems that do not have interconnections.
	m_CapacitorQuality = quality;
	m_CnCQuality = quality;
	m_HullQuality = quality;
	m_PowerPlantQuality = quality;

	int systemIndex = 0;

	if(design.LifeSupportUnits())
	{
		AuShipSystem ssTemp(SYS_LifeSupport, 0, design.LifeSupportModule(), quality, (int)ceilf(design.LifeSupportUnits()),
				systemIndex);
		append(ssTemp);
		systemIndex++;
	}
	
	if(design.ManeuverDriveUnits())
	{
		AuShipSystem ssTemp(SYS_ManeuverDrive, 0, design.ManeuverDriveModule(), quality, (int)ceilf(design.ManeuverDriveUnits()),
				systemIndex);
		append(ssTemp);
		systemIndex++;
	}

	for(AuScReportSysVolHash::const_iterator mi = design.FTLDrives().cbegin(); mi != design.FTLDrives().cend(); mi++)
	{
		AuShipSystem ssTemp(SYS_FTLDrive, mi.key(), (*mi).Module(), quality, design.EquipmentVolumeHits((*mi).Volume()),
				systemIndex);
		append(ssTemp);
		systemIndex++;
	}
	
	if(design.HasSensor())
	{
		AuShipSystem ssTemp(SYS_Sensor, 0, design.SensorModule(), quality, (int)ceilf(design.SensorUnits()), systemIndex);
		append(ssTemp);
		systemIndex++;
	}
	
	if(design.HasShield())
	{
		AuShipSystem ssTemp(SYS_Shield, 0, design.ShieldModule(), quality, (int)ceilf(design.ShieldUnits()), systemIndex);
		append(ssTemp);
		systemIndex++;
	}
	
	QList<quint16> surgIds = design.SurgeryUnitIds();
	for(QList<quint16>::const_iterator si = surgIds.cbegin(); si != surgIds.cend(); si++)
	{
		AuShipSystem ssTemp(SYS_SurgeryUnit, *si, design.SurgeryUnitModule(*si), quality, 1, systemIndex);
		append(ssTemp);
		systemIndex++;
	}

	for(AuScReportTransporterStationHash::const_iterator mi = design.Transporters().constBegin();
			mi != design.Transporters().constEnd(); mi++)
	{
		AuShipSystem ssTemp(SYS_Transporter, mi.key(), (*mi).Module(), quality, (*mi).Pads(), systemIndex);
		append(ssTemp);
		systemIndex++;
	}

	for(AuScReportSysVolHash::const_iterator mi = design.WeaponBays().constBegin(); mi != design.WeaponBays().constEnd(); mi++)
	{
		AuShipSystem ssTemp(SYS_Weapon, mi.key(), (*mi).Module(), quality, design.EquipmentVolumeHits((*mi).Volume()), systemIndex);
		append(ssTemp);
		systemIndex++;
	}
	
	for(AuScReportTurretHash::const_iterator mi = design.Turrets().constBegin(); mi != design.Turrets().constEnd(); mi++)
	{
		AuShipSystem ssTemp(SYS_Turret, mi.key(), (*mi).Module(), quality, 1, systemIndex);
		append(ssTemp);
		systemIndex++;
	}

	for(QSet<quint16>::const_iterator li = design.VehicleLandingSystems().cbegin(); li != design.VehicleLandingSystems().cend();
			li++)
	{
		AuShipSystem ssTemp(SYS_VehicleLanding, *li, CID_ElectronicPart, quality, 1, systemIndex);
		append(ssTemp);
		systemIndex++;
	}

	for(QSet<quint16>::const_iterator li = design.VehicleLaunchers().cbegin(); li != design.VehicleLaunchers().cend(); li++)
	{
		AuShipSystem ssTemp(SYS_VehicleLaunch, *li, CID_MechanicalPart, quality, 1, systemIndex);
		append(ssTemp);
		systemIndex++;
	}
}

void AuShipSystemArray::GatherShipSystems(quint8 quality, const AuBdReport& design)
{
	Q_ASSERT(quality);
	clear();

	// Store quality of systems that do not have interconnections.
	if(design.CapacitorUnits())
		m_CapacitorQuality = quality;
	if(design.CommandAndControlModule() != CID_Invalid)
		m_CnCQuality = quality;
	m_HullQuality = quality;
	if(design.PowerPlantUnits())
		m_PowerPlantQuality = quality;

	int systemIndex = 0;

	if(design.LifeSupportUnits())
	{
		AuShipSystem ssTemp(SYS_LifeSupport, 0, design.LifeSupportModule(), quality, (int)ceilf(design.LifeSupportUnits()),
				systemIndex);
		append(ssTemp);
		systemIndex++;
	}
	
	if(design.HasSensor())
	{
		AuShipSystem ssTemp(SYS_Sensor, 0, design.SensorModule(), quality, (int)ceilf(design.SensorUnits()), systemIndex);
		append(ssTemp);
		systemIndex++;
	}
	
	if(design.HasShield())
	{
		AuShipSystem ssTemp(SYS_Shield, 0, design.ShieldModule(), quality, (int)ceilf(design.ShieldUnits()), systemIndex);
		append(ssTemp);
		systemIndex++;
	}
	
	QList<quint16> surgIds = design.SurgeryUnitIds();
	for(QList<quint16>::const_iterator si = surgIds.cbegin(); si != surgIds.cend(); si++)
	{
		AuShipSystem ssTemp(SYS_SurgeryUnit, *si, design.SurgeryUnitModule(*si), quality, 1, systemIndex);
		append(ssTemp);
		systemIndex++;
	}

	for(AuScReportTransporterStationHash::const_iterator mi = design.Transporters().constBegin();
			mi != design.Transporters().constEnd(); mi++)
	{
		AuShipSystem ssTemp(SYS_Transporter, mi.key(), (*mi).Module(), quality, (*mi).Pads(), systemIndex);
		append(ssTemp);
		systemIndex++;
	}

	for(AuScReportSysVolHash::const_iterator mi = design.WeaponBays().constBegin(); mi != design.WeaponBays().constEnd(); mi++)
	{
		AuShipSystem ssTemp(SYS_Weapon, mi.key(), (*mi).Module(), quality, design.EquipmentVolumeHits((*mi).Volume()), systemIndex);
		append(ssTemp);
		systemIndex++;
	}
	
	for(AuScReportTurretHash::const_iterator mi = design.Turrets().constBegin(); mi != design.Turrets().constEnd(); mi++)
	{
		AuShipSystem ssTemp(SYS_Turret, mi.key(), (*mi).Module(), quality, 1, systemIndex);
		append(ssTemp);
		systemIndex++;
	}

	for(QSet<quint16>::const_iterator li = design.VehicleLandingSystems().cbegin(); li != design.VehicleLandingSystems().cend();
			li++)
	{
		AuShipSystem ssTemp(SYS_VehicleLanding, *li, CID_ElectronicPart, quality, 1, systemIndex);
		append(ssTemp);
		systemIndex++;
	}

	for(QSet<quint16>::const_iterator li = design.VehicleLaunchers().cbegin(); li != design.VehicleLaunchers().cend(); li++)
	{
		AuShipSystem ssTemp(SYS_VehicleLaunch, *li, CID_MechanicalPart, quality, 1, systemIndex);
		append(ssTemp);
		systemIndex++;
	}
}

// Disconnects the consumer system from the supplier system 'supplierIndex'.
void AuShipSystemArray::DisconnectConsumer(int supplierIndex)
{
	if(supplierIndex < count())
	{
		// Disconnect the consumer that supplierIndex is connected to.
		iterator si = begin() + supplierIndex;
		int consumerIndex = (*si).GetConnectedConsumer();
		if(consumerIndex != -1)		// Unless, of course, it is already disconnected.
		{
			(*si).DisconnectConsumer();
			RecalculatePowerConsumption(consumerIndex);	// Recalc power consumption based on remaining supplier connections.
		}
	}
}

bool AuShipSystemArray::RecalculatePowerConsumption(int consumerIndex)
{
	if(consumerIndex >= 0 && consumerIndex < count())
	{
		// Add up the power supplied by all systems connected to the consumer system.
		quint32 power = 0;
		for(const_iterator si = constBegin(); si != constEnd(); si++)
			if((*si).GetConnectedConsumer() == consumerIndex)
				power += (*si).GetPowerSupplyCurrent();

		// Set the power consumption if it has changed.
		iterator ci = begin() + consumerIndex;
		if(power != (*ci).GetPowerConsumptionCurrent())
		{
			(*ci).SetPowerConsumptionCurrent(power);
			return true;
		}
	}
	return false;
}

bool AuShipSystemArray::HasConnectedSupplier(int consumerIndex) const
{
	if(consumerIndex >= 0 && consumerIndex < count())
	{
		for(const_iterator si = constBegin(); si != constEnd(); si++)
			if((*si).GetConnectedConsumer() == consumerIndex)
				return true;
	}
	return false;
}

void AuShipSystemArray::Reset()
{
	int systemIndex = 0;
	for(iterator si = begin(); si != end(); si++, systemIndex++)
	{
		(*si).SetConnectedConsumer(systemIndex);	// Reconnect system to itself.
		(*si).OverclockSupplier(false);				// Turn off overclocking.
	}
}

quint8 AuShipSystemArray::HighestQuality() const
{
	// Get highest quality of systems that cannot be connected.
	quint8 highestQ = m_CnCQuality;
	if(m_HullQuality > highestQ)
		highestQ = m_HullQuality;
	if(m_PowerPlantQuality > highestQ)
		highestQ = m_PowerPlantQuality;
	if(m_CapacitorQuality > highestQ)
		highestQ = m_CapacitorQuality;

	// No need to loop if the highest quality is already at the max.
	if(highestQ < 255)
		for(const_iterator si = constBegin(); si != constEnd(); si++)
			if((*si).Quality() > highestQ)
				highestQ = (*si).Quality();

	return highestQ;
}

quint8 AuShipSystemArray::LowestQuality() const
{
	// Get lowest quality of systems that cannot be connected.
	quint8 lowestQ = m_CnCQuality;
	if(m_HullQuality < lowestQ)
		lowestQ = m_HullQuality;
	if(m_PowerPlantQuality < lowestQ)
		lowestQ = m_PowerPlantQuality;
	if(m_CapacitorQuality < lowestQ)
		lowestQ = m_CapacitorQuality;

	// No need to loop if the lowest quality is already 0.
	if(lowestQ)
		for(const_iterator si = constBegin(); si != constEnd(); si++)
			if((*si).Quality() < lowestQ)
				lowestQ = (*si).Quality();

	return lowestQ;
}

bool AuShipSystemArray::UpgradeSystem(SYSTEM system, COMMODITYID cid, quint8 quality)
{
	Q_ASSERT(quality);
	switch(system)
	{
	case SYS_Count:
		Q_ASSERT(false);
		break;
	case SYS_VehicleLaunch:
	case SYS_VehicleLanding:
		break;	// Cannot upgrade these systems.

	case SYS_CommandAndControl:
		m_CnCModule = cid;
		m_CnCQuality = quality;
		return true;

	case SYS_Fuel:
	case SYS_Hold:
	case SYS_Hull:
		m_HullModule = cid;
		m_HullQuality = quality;
		return true;

	case SYS_Capacitor:
		m_CapacitorModule = cid;
		m_CapacitorQuality = quality;
		return true;

	case SYS_PowerPlant:
		m_PowerPlantModule = cid;
		m_PowerPlantQuality = quality;
		return true;

	case SYS_ManeuverDrive:
	case SYS_LifeSupport:
	case SYS_Sensor:
	case SYS_Shield:
	case SYS_SurgeryUnit:
	case SYS_Transporter:
	case SYS_Turret:
	case SYS_Weapon:
	case SYS_FTLDrive:
		for(iterator si = begin(); si != end(); si++)
			if((*si).GetSystemType() == system)
			{
				(*si).SetModule(cid);
				(*si).SetQuality(quality);
				return true;
			}
		break;
	}
	return false;	// Failure.
}

bool AuShipSystemArray::UpgradeSystem(SYSTEM system, quint16 which, COMMODITYID cid, quint8 quality)
{
	Q_ASSERT(quality);
	switch(system)
	{
	case SYS_Count:
		Q_ASSERT(false);
		break;
	case SYS_VehicleLaunch:
	case SYS_VehicleLanding:
		break;	// Cannot upgrade these systems.

	case SYS_CommandAndControl:
		m_CnCModule = cid;
		m_CnCQuality = quality;
		return true;

	case SYS_Fuel:
	case SYS_Hold:
	case SYS_Hull:
		m_HullModule = cid;
		m_HullQuality = quality;
		return true;

	case SYS_Capacitor:
		m_CapacitorModule = cid;
		m_CapacitorQuality = quality;
		return true;

	case SYS_PowerPlant:
		m_PowerPlantModule = cid;
		m_PowerPlantQuality = quality;
		return true;

	case SYS_ManeuverDrive:
	case SYS_LifeSupport:
	case SYS_Sensor:
	case SYS_Shield:
	case SYS_SurgeryUnit:
	case SYS_Transporter:
	case SYS_Turret:
	case SYS_Weapon:
	case SYS_FTLDrive:
		for(iterator si = begin(); si != end(); si++)
			if((*si).GetSystemType() == system && (*si).GetSystemWhich() == which)
			{
				(*si).SetModule(cid);
				(*si).SetQuality(quality);
				return true;
			}
		break;
	}

	return false;	// Failure.
}

bool AuShipSystemArray::HasVehicleTransporter() const
{
	for(const_iterator si = constBegin(); si != constEnd(); si++)
		if((*si).GetSystemType() == SYS_Transporter && (*si).Module() == CID_VehicleTransporterModule)
			return true;
	return false;
}

quint64 AuShipSystemArray::MemoryUsage() const
{
	quint64 mem = sizeof(AuShipSystemArray);
	mem += sizeof(AuShipSystem) * size();
	return mem;
}

void AuShipSystemArray::RecalcHealthMax(const AuScReport& design)
{
	int systemIndex = 0;

	if(design.LifeSupportUnits())
	{
		operator[](systemIndex).SetHealthMax((int)ceilf(design.LifeSupportUnits()));
		systemIndex++;
	}
	
	if(design.ManeuverDriveUnits())
	{
		operator[](systemIndex).SetHealthMax((int)ceilf(design.ManeuverDriveUnits()));
		systemIndex++;
	}

	for(AuScReportSysVolHash::const_iterator mi = design.FTLDrives().cbegin(); mi != design.FTLDrives().cend(); mi++)
	{
		operator[](systemIndex).SetHealthMax((int)ceilf(design.EquipmentVolumeHits((*mi).Volume())));
		systemIndex++;
	}
	
	if(design.HasSensor())
	{
		operator[](systemIndex).SetHealthMax((int)ceilf(design.SensorUnits()));
		systemIndex++;
	}
	
	if(design.HasShield())
	{
		operator[](systemIndex).SetHealthMax((int)ceilf(design.ShieldUnits()));
		systemIndex++;
	}
	
	QList<quint16> surgIds = design.SurgeryUnitIds();
	for(QList<quint16>::const_iterator si = surgIds.cbegin(); si != surgIds.cend(); si++)
	{
		operator[](systemIndex).SetHealthMax(1);
		systemIndex++;
	}

	for(AuScReportTransporterStationHash::const_iterator mi = design.Transporters().constBegin();
			mi != design.Transporters().constEnd(); mi++)
	{
		operator[](systemIndex).SetHealthMax((int)ceilf((*mi).Pads()));
		systemIndex++;
	}

	for(AuScReportSysVolHash::const_iterator mi = design.WeaponBays().constBegin(); mi != design.WeaponBays().constEnd(); mi++)
	{
		operator[](systemIndex).SetHealthMax((int)ceilf(design.EquipmentVolumeHits((*mi).Volume())));
		systemIndex++;
	}
	
	for(AuScReportTurretHash::const_iterator mi = design.Turrets().constBegin(); mi != design.Turrets().constEnd(); mi++)
	{
		operator[](systemIndex).SetHealthMax(1);
		systemIndex++;
	}
}

void AuShipSystemArray::RecalcHealthMax(const AuBdReport& design)
{
	int systemIndex = 0;

	if(design.LifeSupportUnits())
	{
		operator[](systemIndex).SetHealthMax((int)ceilf(design.LifeSupportUnits()));
		systemIndex++;
	}
	
	if(design.HasSensor())
	{
		operator[](systemIndex).SetHealthMax((int)ceilf(design.SensorUnits()));
		systemIndex++;
	}
	
	if(design.HasShield())
	{
		operator[](systemIndex).SetHealthMax((int)ceilf(design.ShieldUnits()));
		systemIndex++;
	}
	
	QList<quint16> surgIds = design.SurgeryUnitIds();
	for(QList<quint16>::const_iterator si = surgIds.cbegin(); si != surgIds.cend(); si++)
	{
		operator[](systemIndex).SetHealthMax(1);
		systemIndex++;
	}

	for(AuScReportTransporterStationHash::const_iterator mi = design.Transporters().constBegin();
			mi != design.Transporters().constEnd(); mi++)
	{
		operator[](systemIndex).SetHealthMax((*mi).Pads());
		systemIndex++;
	}

	for(AuScReportSysVolHash::const_iterator mi = design.WeaponBays().constBegin(); mi != design.WeaponBays().constEnd(); mi++)
	{
		operator[](systemIndex).SetHealthMax(design.EquipmentVolumeHits((*mi).Volume()));
		systemIndex++;
	}
	
	for(AuScReportTurretHash::const_iterator mi = design.Turrets().constBegin(); mi != design.Turrets().constEnd(); mi++)
	{
		operator[](systemIndex).SetHealthMax(1);
		systemIndex++;
	}
}

bool AuShipSystemArray::DowngradeSystemQuality(const AuSysId& sysId, quint8 repairQ)
{
	if(!repairQ)
		repairQ = 1;
	bool anyChange = false;
	switch(sysId.System())
	{
	case SYS_Count:
		break;

	case SYS_CommandAndControl:
		if(m_CnCQuality > repairQ)
		{
			m_CnCQuality = repairQ;
			anyChange = true;
		}
		break;

	case SYS_Fuel:
	case SYS_Hold:
	case SYS_Hull:
		if(m_HullQuality > repairQ)
		{
			m_HullQuality = repairQ;
			anyChange = true;
		}
		break;

	case SYS_Capacitor:
		if(m_CapacitorQuality > repairQ)
		{
			m_CapacitorQuality = repairQ;
			anyChange = true;
		}
		break;

	case SYS_PowerPlant:
		if(m_PowerPlantQuality > repairQ)
		{
			m_PowerPlantQuality = repairQ;
			anyChange = true;
		}
		break;

	case SYS_ManeuverDrive:
	case SYS_LifeSupport:
	case SYS_Sensor:
	case SYS_Shield:
	case SYS_SurgeryUnit:
	case SYS_Transporter:
	case SYS_Turret:
	case SYS_Weapon:
	case SYS_FTLDrive:
	case SYS_VehicleLaunch:
	case SYS_VehicleLanding:
		for(iterator si = begin(); si != end(); si++)
			if((*si).GetSysId() == sysId)
			{
				if((*si).Quality() > repairQ)
				{
					(*si).SetQuality(repairQ);
					anyChange = true;
				}
			}
		break;
	}
	return anyChange;
}

bool AuShipSystemArray::SetSystemQuality(const AuSysId& sysId, quint8 quality)
{
	if(!quality)
		quality = 1;
	bool anyChange = false;
	switch(sysId.System())
	{
	case SYS_Count:
		break;

	case SYS_CommandAndControl:
		if(m_CnCQuality != quality)
		{
			m_CnCQuality = quality;
			anyChange = true;
		}
		break;

	case SYS_Fuel:
	case SYS_Hold:
	case SYS_Hull:
		if(m_HullQuality != quality)
		{
			m_HullQuality = quality;
			anyChange = true;
		}
		break;

	case SYS_Capacitor:
		if(m_CapacitorQuality != quality)
		{
			m_CapacitorQuality = quality;
			anyChange = true;
		}
		break;

	case SYS_PowerPlant:
		if(m_PowerPlantQuality != quality)
		{
			m_PowerPlantQuality = quality;
			anyChange = true;
		}
		break;

	case SYS_ManeuverDrive:
	case SYS_LifeSupport:
	case SYS_Sensor:
	case SYS_Shield:
	case SYS_SurgeryUnit:
	case SYS_Transporter:
	case SYS_Turret:
	case SYS_Weapon:
	case SYS_FTLDrive:
	case SYS_VehicleLaunch:
	case SYS_VehicleLanding:
		for(iterator si = begin(); si != end(); si++)
			if((*si).GetSysId() == sysId)
			{
				if((*si).Quality() != quality)
				{
					(*si).SetQuality(quality);
					anyChange = true;
				}
			}
		break;
	}
	return anyChange;
}

bool AuShipSystemArray::SystemWhiches(SYSTEM system, QSet<quint16>& whiches) const
{
	whiches.clear();
	bool hasSystem = false;
	switch(system)
	{
	case SYS_Count:
		break;

	case SYS_CommandAndControl:
		hasSystem = true;	// All spacecraft have this system.
		break;

	case SYS_Fuel:
	case SYS_Hold:
	case SYS_Hull:
		hasSystem = true;	// All spacecraft have this system.
		break;

	case SYS_Capacitor:
		hasSystem = true;	// All spacecraft have this system.
		break;

	case SYS_PowerPlant:
		hasSystem = true;	// All spacecraft have this system.
		break;

	case SYS_ManeuverDrive:
	case SYS_LifeSupport:
	case SYS_Sensor:
	case SYS_Shield:
	case SYS_SurgeryUnit:
	case SYS_Transporter:
	case SYS_Turret:
	case SYS_Weapon:
	case SYS_FTLDrive:
	case SYS_VehicleLaunch:
	case SYS_VehicleLanding:
		for(const_iterator si = cbegin(); si != cend(); si++)
			if((*si).GetSystemType() == system)
				whiches.insert((*si).GetSystemWhich());
		hasSystem = !whiches.empty();
		break;
	}
	return hasSystem;
}

bool AuShipSystemArray::Reconcile(const AuBdReport& design)
{
	// Get systems from 'design'.
	AuShipSystemArray ck;
	ck.GatherShipSystems(design.Quality(), design);

	bool reconcileNeeded;
	if(size() == ck.size())
	{
		// Compare the type and which of each system. If they differ at all, reconcile is needed.
		reconcileNeeded = false;
		for(const_iterator mi = cbegin(); mi != cend(); mi++)
			if(!ck.HasSystem((*mi).GetSysId()))
			{
				qDebug() << "AuShipSystemArray::Reconcile Bd check system not found" << (*mi).GetSysId();
				reconcileNeeded = true;
				break;
			}
	}
	else
	{
		reconcileNeeded = true;	// Sizes don't match so reconcile is definitely needed.
		//qDebug() << "AuShipSystemArray::Reconcile size mismatch requires reconcile. old" << size() << "new" << ck.size();
	}

	if(reconcileNeeded)
		GatherShipSystems(design.Quality(), design);
	return reconcileNeeded;
}

bool AuShipSystemArray::Reconcile(const AuScReport& design)
{
	// Get systems from 'design'.
	AuShipSystemArray ck;
	ck.GatherShipSystems(design.Quality(), design);

	bool reconcileNeeded;
	if(size() == ck.size())
	{
		// Compare the type and which of each system. If they differ at all, reconcile is needed.
		reconcileNeeded = false;
		for(const_iterator mi = cbegin(); mi != cend(); mi++)
			if(!ck.HasSystem((*mi).GetSysId()))
			{
				qDebug() << "AuShipSystemArray::Reconcile Sc check system not found" << (*mi).GetSysId();
				reconcileNeeded = true;
				break;
			}
	}
	else
	{
		reconcileNeeded = true;	// Sizes don't match so reconcile is definitely needed.
		//qDebug() << "AuShipSystemArray::Reconcile size mismatch requires reconcile. old" << size() << "new" << ck.size();
	}

	if(reconcileNeeded)
		GatherShipSystems(design.Quality(), design);
	return reconcileNeeded;
}

bool AuShipSystemArray::HasSystem(const AuSysId& sysId) const
{
	for(const_iterator mi = cbegin(); mi != cend(); mi++)
		if((*mi).GetSysId() == sysId)
			return true;
	return false;
}
