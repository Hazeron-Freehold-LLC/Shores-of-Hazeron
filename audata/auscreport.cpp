//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscreport.cpp,v 1.212 2022/02/19 18:51:23 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscreport.h"
//
#include "aucombat.h"
#include "auglobal.h"
#include "auitem.h"
#include "auscdesign.h"
#include "auscparts.h"
#include "auscroute.h"
#include "dgdata.h"

#define REPORT_SERIALIZE_VER			0x12

AuScReport::AuScReport()
{
	// Design
	//m_ArchitectId;
	//m_ArchitectName;
	//m_DesignDesc;
	m_DesignId = 0;
	//m_DesignName;
	//m_PartArchitects;

	// Structure
	m_LandingOrient = LORI_Belly;
	m_StationOrient = 0;
	m_Streamline = 0;
	m_Structure = 0;

	// Volumes, in cubic meters.
	//m_Hull;
	//m_HullMax;
	//m_HullMin;
	m_ArmorVolume = 0;
	//m_Bays;
	//m_Capacitor;
	m_CryoBerthVolume = 0;
	//m_FTLDrives;
	m_FuelVolume = 0;
	m_HoldVolume = 0;
	//m_LifeSupport;
	//m_ManeuverDrive;
	//m_PowerPlant;
	//m_Sensor;
	//m_Shield;
	m_SkeletonVolume = 0;
	//m_Turrets;

	// Quality
	m_Quality = 1;
	m_CnCModule = 0;
	m_CnCQ = 1;
	//m_MfgProc

	// Stations and Equipment
	//m_CommandStations;
	//m_CrewStations;
	//m_DesignerStations;
	//m_GameTables;
	//m_GuardPosts;
	//m_Lifts;
	//m_ServicePanels;
	//m_StatusDisplays;
	//m_Transporters;
	//m_VehicleLaunchers;
	//m_VehicleLandingSystems;

	// Berths
	m_CaptBerths = 0;
	m_CrewBerths = 0;
	m_OfficerBerths = 0;
	m_PassengerBerths = 0;
	m_TroopBerths = 0;

	// Parking
	m_ParkGround = 0;
	m_ParkRocket = 0;
	m_ParkSpace = 0;
	m_ParkSpacecraft = 0;
	m_ParkWater = 0;

	// Mass
	m_MassKg = 0;
	m_MassEquipmentKg = 0;
	m_MassFuelKg = 0;
	m_MassHullKg = 0;

	// Analysis
	m_Acceleration = 0;
	m_AccelerationVert = 0;
	m_CapacitorMax = 0;
	m_FaceCount = 0;
	m_FTLDriveUnitsToEngage = 0;
	m_FuelBurnRate = 0;
	m_FuelCapacity = 0;
	m_HasBar = false;
	m_HasPantry = false;
	m_HoldCapacity = 0;
	m_HoldPowerNoShields = 0;
	m_HoldPowerWithShields = 0;
	m_IdlePowerNoShields = 0;
	m_IdlePowerWithShields = 0;
	m_LifeSupportNeeded = 0;
	m_ManeuverFuelRate = 0;
	m_SensorRange = 0;
	m_TriangleCount = 0;
	m_TurnRate = 0;
}

AuScReport::~AuScReport()
{
}

AuScReport::AuScReport(const AuScReport& c) : m_PartArchitects(c.m_PartArchitects),
	m_Bays(c.m_Bays),
	m_FTLDrives(c.m_FTLDrives),
	m_Turrets(c.m_Turrets),
	m_CitizenPosts(c.m_CitizenPosts),
	m_CommandStations(c.m_CommandStations),
	m_CrewStations(c.m_CrewStations),
	m_DesignerStations(c.m_DesignerStations),
	m_GameTables(c.m_GameTables),
	m_GuardPosts(c.m_GuardPosts),
	m_Lifts(c.m_Lifts),
	m_LivestockPosts(c.m_LivestockPosts),
	m_ServicePanels(c.m_ServicePanels),
	m_StatusDisplays(c.m_StatusDisplays),
	m_Transporters(c.m_Transporters),
	m_VehicleLandingSystems(c.m_VehicleLandingSystems),
	m_VehicleLaunchers(c.m_VehicleLaunchers)
{
	// Design
	m_ArchitectId = c.m_ArchitectId;
	m_ArchitectName = c.m_ArchitectName;
	m_DesignDesc = c.m_DesignDesc;
	m_DesignId = c.m_DesignId;
	m_DesignName = c.m_DesignName;

	// Structure
	m_LandingOrient = c.m_LandingOrient;
	m_StationOrient = c.m_StationOrient;
	m_Streamline = c.m_Streamline;
	m_Structure = c.m_Structure;

	// Volumes, in cubic meters.
	m_Hull = c.m_Hull;
	m_HullMax = c.m_HullMax;
	m_HullMin = c.m_HullMin;
	m_ArmorVolume = c.m_ArmorVolume;
	m_Capacitor = c.m_Capacitor;
	m_CryoBerthVolume = c.m_CryoBerthVolume;
	m_FuelVolume = c.m_FuelVolume;
	m_HoldVolume = c.m_HoldVolume;
	m_LifeSupport = c.m_LifeSupport;
	m_ManeuverDrive = c.m_ManeuverDrive;
	m_PowerPlant = c.m_PowerPlant;
	m_Sensor = c.m_Sensor;
	m_Shield = c.m_Shield;
	m_SkeletonVolume = c.m_SkeletonVolume;

	// Quality
	m_Quality = c.m_Quality;
	m_CnCModule = c.m_CnCModule;
	m_CnCQ = c.m_CnCQ;
	m_MfgProc = c.m_MfgProc;

	// Stations and Equipment

	// Berths
	m_CaptBerths = c.m_CaptBerths;
	m_CrewBerths = c.m_CrewBerths;
	m_OfficerBerths = c.m_OfficerBerths;
	m_PassengerBerths = c.m_PassengerBerths;
	m_TroopBerths = c.m_TroopBerths;

	// Parking
	m_ParkGround = c.m_ParkGround;
	m_ParkRocket = c.m_ParkRocket;
	m_ParkSpace = c.m_ParkSpace;
	m_ParkSpacecraft = c.m_ParkSpacecraft;
	m_ParkWater = c.m_ParkWater;

	// Mass
	m_MassKg = c.m_MassKg;
	m_MassEquipmentKg = c.m_MassEquipmentKg;
	m_MassFuelKg = c.m_MassFuelKg;
	m_MassHullKg = c.m_MassHullKg;

	// Analysis
	m_Acceleration = c.m_Acceleration;
	m_AccelerationVert = c.m_AccelerationVert;
	m_CapacitorMax = c.m_CapacitorMax;
	m_FaceCount = c.m_FaceCount;
	m_FTLDriveUnitsToEngage = c.m_FTLDriveUnitsToEngage;
	m_FuelBurnRate = c.m_FuelBurnRate;
	m_FuelCapacity = c.m_FuelCapacity;
	m_HasBar = c.m_HasBar;
	m_HasPantry = c.m_HasPantry;
	m_HoldCapacity = c.m_HoldCapacity;
	m_HoldPowerNoShields = c.m_HoldPowerNoShields;
	m_HoldPowerWithShields = c.m_HoldPowerWithShields;
	m_IdlePowerNoShields = c.m_IdlePowerNoShields;
	m_IdlePowerWithShields = c.m_IdlePowerWithShields;
	m_LifeSupportNeeded = c.m_LifeSupportNeeded;
	m_ManeuverFuelRate = c.m_ManeuverFuelRate;
	m_PartsList = c.m_PartsList;
	m_SensorRange = c.m_SensorRange;
	m_TriangleCount = c.m_TriangleCount;
	m_TurnRate = c.m_TurnRate;
}

const AuScReport& AuScReport::operator=(const AuScReport& c)
{
	// Design
	m_ArchitectId = c.m_ArchitectId;
	m_ArchitectName = c.m_ArchitectName;
	m_DesignDesc = c.m_DesignDesc;
	m_DesignId = c.m_DesignId;
	m_DesignName = c.m_DesignName;
	m_PartArchitects = c.m_PartArchitects;

	// Structure
	m_LandingOrient = c.m_LandingOrient;
	m_StationOrient = c.m_StationOrient;
	m_Streamline = c.m_Streamline;
	m_Structure = c.m_Structure;

	// Volumes, in cubic meters.
	m_Hull = c.m_Hull;
	m_HullMax = c.m_HullMax;
	m_HullMin = c.m_HullMin;
	m_ArmorVolume = c.m_ArmorVolume;
	m_Bays = c.m_Bays;
	m_Capacitor = c.m_Capacitor;
	m_CryoBerthVolume = c.m_CryoBerthVolume;
	m_FTLDrives = c.m_FTLDrives;
	m_FuelVolume = c.m_FuelVolume;
	m_HoldVolume = c.m_HoldVolume;
	m_LifeSupport = c.m_LifeSupport;
	m_ManeuverDrive = c.m_ManeuverDrive;
	m_PowerPlant = c.m_PowerPlant;
	m_Sensor = c.m_Sensor;
	m_Shield = c.m_Shield;
	m_SkeletonVolume = c.m_SkeletonVolume;
	m_Turrets = c.m_Turrets;

	// Quality
	m_Quality = c.m_Quality;
	m_CnCModule = c.m_CnCModule;
	m_CnCQ = c.m_CnCQ;
	m_MfgProc = c.m_MfgProc;

	// Stations
	m_CitizenPosts = c.m_CitizenPosts;
	m_CommandStations = c.m_CommandStations;
	m_CrewStations = c.m_CrewStations;
	m_DesignerStations = c.m_DesignerStations;
	m_GameTables = c.m_GameTables;
	m_GuardPosts = c.m_GuardPosts;
	m_Lifts = c.m_Lifts;
	m_LivestockPosts = c.m_LivestockPosts;
	m_ServicePanels = c.m_ServicePanels;
	m_StatusDisplays = c.m_StatusDisplays;
	m_Transporters = c.m_Transporters;
	m_VehicleLaunchers = c.m_VehicleLaunchers;
	m_VehicleLandingSystems = c.m_VehicleLandingSystems;

	// Berths
	m_CaptBerths = c.m_CaptBerths;
	m_CrewBerths = c.m_CrewBerths;
	m_OfficerBerths = c.m_OfficerBerths;
	m_PassengerBerths = c.m_PassengerBerths;
	m_TroopBerths = c.m_TroopBerths;

	// Parking
	m_ParkGround = c.m_ParkGround;
	m_ParkRocket = c.m_ParkRocket;
	m_ParkSpace = c.m_ParkSpace;
	m_ParkSpacecraft = c.m_ParkSpacecraft;
	m_ParkWater = c.m_ParkWater;

	// Mass
	m_MassKg = c.m_MassKg;
	m_MassEquipmentKg = c.m_MassEquipmentKg;
	m_MassFuelKg = c.m_MassFuelKg;
	m_MassHullKg = c.m_MassHullKg;

	// Analysis
	m_Acceleration = c.m_Acceleration;
	m_AccelerationVert = c.m_AccelerationVert;
	m_CapacitorMax = c.m_CapacitorMax;
	m_FaceCount = c.m_FaceCount;
	m_FTLDriveUnitsToEngage = c.m_FTLDriveUnitsToEngage;
	m_FuelBurnRate = c.m_FuelBurnRate;
	m_FuelCapacity = c.m_FuelCapacity;
	m_HasBar = c.m_HasBar;
	m_HasPantry = c.m_HasPantry;
	m_HoldCapacity = c.m_HoldCapacity;
	m_HoldPowerNoShields = c.m_HoldPowerNoShields;
	m_HoldPowerWithShields = c.m_HoldPowerWithShields;
	m_IdlePowerNoShields = c.m_IdlePowerNoShields;
	m_IdlePowerWithShields = c.m_IdlePowerWithShields;
	m_LifeSupportNeeded = c.m_LifeSupportNeeded;
	m_ManeuverFuelRate = c.m_ManeuverFuelRate;
	m_PartsList = c.m_PartsList;
	m_SensorRange = c.m_SensorRange;
	m_TriangleCount = c.m_TriangleCount;
	m_TurnRate = c.m_TurnRate;

	return *this;
}

bool AuScReport::HasHelm() const
{
	for(QHash<quint16, AuScReportStation>::const_iterator ci = m_CrewStations.cbegin(); ci != m_CrewStations.cend(); ci++)
		if((*ci).PartType() == AuScPart::Part_StationHelm || (*ci).PartType() == AuScPart::Part_ConsoleHelm)
			return true;
	return false;
}

bool AuScReport::HasEngineerStation() const
{
	for(QHash<quint16, AuScReportStation>::const_iterator ci = m_CrewStations.cbegin(); ci != m_CrewStations.cend(); ci++)
		if((*ci).PartType() == AuScPart::Part_StationEngineer || (*ci).PartType() == AuScPart::Part_ConsoleEngineer)
			return true;
	return false;
}

//static
unsigned int AuScReport::CompressedSizeLimit()
{
	return 14000000;
}

//static
unsigned int AuScReport::CompressedSizeLimitPerc()
{
	return CompressedSizeLimit()/100;
}

//static
unsigned int AuScReport::SizeLimit()
{
	return 28000000;
}

//static
unsigned int AuScReport::SizeLimitPerc()
{
	return SizeLimit()/100;
}

bool AuScReport::OkForAssembly(QStringList& msgs, const AuScDesign& design) const
{
	QString error("<span style=\"color: rgb(255, 32, 32);\">Error: %1</span>");
	QString note("<span style=\"color: rgb(32, 255, 32);\">%1</span>");
	QString warning("<span style=\"color: rgb(255, 128, 32);\">Warning: %1</span>");

	bool isOk = true;

	if(m_DesignName.isEmpty())
	{
		msgs.append(error.arg(QObject::tr("Design name is blank.")));
		isOk = false;
	}
	else if(!IsNameValid(m_DesignName))
	{
		msgs.append(error.arg(QObject::tr("Design name <b>%1</b> is invalid.").arg(m_DesignName)));
		isOk = false;
	}

	if(design.Parts().empty())
	{
		msgs.append(error.arg(QObject::tr("No parts. Add at least one part.")));
		isOk = false;
	}

	// If the uncompressed data gets too large, the servers run out of memory.
	// If the compressed data gets too large, the SQL transaction fails.
	{
		quint64 memSize = design.MemoryUsage();
		int perc = memSize / SizeLimitPerc();
		if(perc > 100)
		{
			msgs.append(error.arg(QObject::tr("%L1 bytes blueprint data exceeds the size limit of %L2 bytes. "
							"Reduce the number of faces and custom textures. "))
					.arg(memSize)
					.arg(SizeLimit()));
			isOk = false;
		}
		else if(perc >= 90)
			msgs.append(warning.arg(QObject::tr("%L1 bytes blueprint data is at %3% of the size limit of %L2 bytes. "
							"Conserve the number of faces and custom textures.")
						.arg(memSize)
						.arg(SizeLimit())
						.arg(perc)));
		else
			msgs.append(note.arg(QObject::tr("%L1 bytes blueprint data is at %3% of the size limit of %L2 bytes.")
						.arg(memSize)
						.arg(SizeLimit())
						.arg(perc)));

		QByteArray bac(qCompress(design.Pack()));
		perc = bac.size() / CompressedSizeLimitPerc();
		if(perc > 100)
		{
			msgs.append(error.arg(QObject::tr("When compressed, %L1 bytes blueprint data exceeds the compressed size limit "
							"of %L2 bytes. Reduce the number of faces and custom textures.")
						.arg(bac.size())
						.arg(CompressedSizeLimit())));
			isOk = false;
		}
		else if(perc >= 90)
			msgs.append(warning.arg(QObject::tr("When compressed, %L1 bytes blueprint data is at %3% of the compressed size limit "
							"of %L2 bytes. Conserve the number of faces and custom textures.")
						.arg(bac.size())
						.arg(CompressedSizeLimit())
						.arg(perc)));
		else
			msgs.append(note.arg(QObject::tr("When compressed, %L1 bytes blueprint data is at %3% of the compressed size limit "
							"of %L2 bytes.")
						.arg(bac.size())
						.arg(CompressedSizeLimit())
						.arg(perc)));
	}

	if(isOk)
		msgs.append(note.arg(QObject::tr("Success! The spacecraft blueprint can be used as an assembly.")));

	return isOk;
}

bool AuScReport::OkToManufacture(QStringList& msgs, const AuScDesign* design) const
{
	QString error("<span style=\"color: rgb(255, 32, 32);\">Error: %1</span>");
	QString note("<span style=\"color: rgb(32, 255, 32);\">%1</span>");
	QString warning("<span style=\"color: rgb(255, 128, 32);\">Warning: %1</span>");

	QString cubedSym;
	if(g_Au.IsWebCGI())
		cubedSym = QString("³");
	else
		cubedSym = g_Au.CubedSymbol();

	bool isOk = true;

	if(m_DesignName.isEmpty())
	{
		msgs.append(error.arg(QObject::tr("Design name is blank.")));
		isOk = false;
	}
	else if(!IsNameValid(m_DesignName))
	{
		msgs.append(error.arg(QObject::tr("Design name <b>%1</b> is invalid.").arg(m_DesignName)));
		isOk = false;
	}

	// If the uncompressed data gets too large, the servers run out of memory.
	// If the compressed data gets too large, the SQL transaction fails.
	if(design)
	{
		quint64 memSize = design->MemoryUsage();
		int perc = memSize / SizeLimitPerc();
		if(perc > 100)
		{
			msgs.append(error.arg(QObject::tr("%L1 bytes blueprint data exceeds the size limit of %L2 bytes. "
							"Reduce the number of faces and custom textures. "))
					.arg(memSize)
					.arg(SizeLimit()));
			isOk = false;
		}
		else if(perc >= 90)
			msgs.append(warning.arg(QObject::tr("%L1 bytes blueprint data is at %3% of the size limit of %L2 bytes. "
							"Conserve the number of faces and custom textures.")
						.arg(memSize)
						.arg(SizeLimit())
						.arg(perc)));
		else
			msgs.append(note.arg(QObject::tr("%L1 bytes blueprint data is at %3% of the size limit of %L2 bytes.")
						.arg(memSize)
						.arg(SizeLimit())
						.arg(perc)));

		QByteArray bac(qCompress(design->Pack()));
		perc = bac.size() / CompressedSizeLimitPerc();
		if(perc > 100)
		{
			msgs.append(error.arg(QObject::tr("When compressed, %L1 bytes blueprint data exceeds the compressed size limit "
							"of %L2 bytes. Reduce the number of faces and custom textures.")
						.arg(bac.size())
						.arg(CompressedSizeLimit())));
			isOk = false;
		}
		else if(perc >= 90)
			msgs.append(warning.arg(QObject::tr("When compressed, %L1 bytes blueprint data is at %3% of the compressed size limit "
							"of %L2 bytes. Conserve the number of faces and custom textures.")
						.arg(bac.size())
						.arg(CompressedSizeLimit())
						.arg(perc)));
		else
			msgs.append(note.arg(QObject::tr("When compressed, %L1 bytes blueprint data is at %3% of the compressed size limit "
							"of %L2 bytes.")
						.arg(bac.size())
						.arg(CompressedSizeLimit())
						.arg(perc)));
	}

	// Volumes/system presense.
	if(!HullVolume())
	{
		msgs.append(error.arg(QObject::tr("No hull. Add hull.")));
		isOk = false;
	}
	else if(HullVolume() > AuScDesign::MaxVolume())
	{
		msgs.append(error.arg(QObject::tr("Hull too big. Make the hull smaller.")));
		isOk = false;
	}

	if(!m_ManeuverDrive.Volume())
	{
		msgs.append(error.arg(QObject::tr("No maneuver drive. Allocate hull volume to maneuver drive.")));
		isOk = false;
	}

	if(!m_PowerPlant.Volume())
	{
		msgs.append(error.arg(QObject::tr("No power plant. Allocate hull volume to power plant.")));
		isOk = false;
	}

	if(!m_Capacitor.Volume())
	{
		msgs.append(error.arg(QObject::tr("No capacitor. Allocate hull volume to capacitor.")));
		isOk = false;
	}

	if(!m_FuelVolume)
	{
		msgs.append(error.arg(QObject::tr("No fuel. Allocate hull volume to fuel.")));
		isOk = false;
	}

	if(!m_LifeSupport.Volume())
	{
		msgs.append(error.arg(QObject::tr("No life support. Allocate hull volume to life support.")));
		isOk = false;
	}
	else if(LifeSupportNeeded() > 0)
	{
		msgs.append(error.arg(QObject::tr("Life support inadequate. Allocate at least %1m%2 to life support.")
					.arg(LifeSupportNeeded())
					.arg(cubedSym)));
		isOk = false;
	}

	if(!HasHelm())
	{
		msgs.append(error.arg(QObject::tr("No helm. Add a helm station.")));
		isOk = false;
	}

	if(Structure() == SC_Starship)
	{
		// Starship needs FTL drives.
		if(!FTLDriveHits())
		{
			msgs.append(error.arg(QObject::tr("Starship has no FTL drives. "
							"Change the structure or add an engineer station. ")));
			isOk = false;
		}
	}

	/*
	// Tech levels
	{
		if(m_BerthQ > MaxTechLevel())
		{
			msgs.append(error.arg(QObject::tr("Berth tech level too high. Reduce number of crew, passenger or troop berths.")));
			isOk = false;
		}

		if(m_EquipmentQ > MaxTechLevel())
		{
			msgs.append(error.arg(QObject::tr("Equipment tech level too high. Remove part %1 %2.")
						.arg(m_EquipmentPart)
						.arg(m_EquipmentDesc)));
			isOk = false;
		}

		if(m_MfgProc.MinimumTechLevelRequired() > MaxTechLevel())
		{
			msgs.append(error.arg(QObject::tr("Manufacturing process tech level too high.")));
			isOk = false;
		}

		if(m_ModuleCID != CID_Invalid && m_ModuleQ > MaxTechLevel())
		{
			msgs.append(error.arg(QObject::tr("Module tech level too high. Remove %1.")
						.arg(g_Au.CommodityName((COMMODITYID)m_ModuleCID))));
			isOk = false;
		}
	}
	*/

	//double carmor = (double)m_ArmorVolume / (double)HullVolume();		// Armor from 0 to 1.
	//if(carmor > .5)
		//msgs.append(warning.arg(QObject::tr("Armor exceeds 50% of volume. Excess armor is useless.")));

	// Overall model bounds.
	if(design)
	{
		AuPoint modelMin, modelMax;
		design->Parts().ModelExtents(modelMin, modelMax);
		if(modelMin.x < -AuScDesign::HullSizeLimit())
		{
			msgs.append(error.arg(QObject::tr("Model is out of bounds to port. "
							"Move the port side of the model to starboard or move the whole model starboard.")));
			isOk = false;
		}

		if(modelMax.x > AuScDesign::HullSizeLimit())
		{
			msgs.append(error.arg(QObject::tr("Model is out of bounds to starboard. "
							"Move the starboard side of the model to port or move the whole model to port.")));
			isOk = false;
		}

		if(modelMin.y < -AuScDesign::HullSizeLimit())
		{
			msgs.append(error.arg(QObject::tr("Model is out of bounds aft. "
							"Move the aft side of the model forward or move the whole model forward.")));
			isOk = false;
		}

		if(modelMax.y > AuScDesign::HullSizeLimit())
		{
			msgs.append(error.arg(QObject::tr("Model is out of bounds foreward. "
							"Move the forward side of the model aft or move the whole model aft.")));
			isOk = false;
		}

		if(modelMin.z < -AuScDesign::HullSizeLimit())
		{
			msgs.append(error.arg(QObject::tr("Model out of bounds below. "
							"Move the bottom of the model up or move the whole model up.")));
			isOk = false;
		}

		if(modelMax.z > AuScDesign::HullSizeLimit())
		{
			msgs.append(error.arg(QObject::tr("Model out of bounds aloft. "
							"Move the top of the model down or move the whole model down.")));
			isOk = false;
		}
	}

	// Hull bounds.
	{
		if(m_HullMin.x < -AuScDesign::HullSizeLimit())
		{
			msgs.append(error.arg(QObject::tr("Hull is out of bounds to port. "
							"Move the port side to starboard or move the whole design to starboard.")));
			isOk = false;
		}
		else if(m_HullMin.x > 0)
		{
			msgs.append(error.arg(QObject::tr("CG outside design extents to port. "
							"Move the port side to port or move the whole design to port.")));
			isOk = false;
		}

		if(m_HullMax.x > AuScDesign::HullSizeLimit())
		{
			msgs.append(error.arg(QObject::tr("Hull is out of bounds to starboard. "
							"Move the starboard side to port or move the whole design to port.")));
			isOk = false;
		}
		else if(m_HullMax.x < 0)
		{
			msgs.append(error.arg(QObject::tr("CG outside design extents to starboard. "
							"Move the starboard side to starboard or move the whole design to starboard.")));
			isOk = false;
		}

		if(m_HullMin.y < -AuScDesign::HullSizeLimit())
		{
			msgs.append(error.arg(QObject::tr("Hull is out of bounds aft. "
							"Move the aft side forward or move the whole design forward.")));
			isOk = false;
		}
		else if(m_HullMin.y > 0)
		{
			msgs.append(error.arg(QObject::tr("CG outside design extents aft. "
							"Move the aft side aft or move the whole design aft.")));
			isOk = false;
		}

		if(m_HullMax.y > AuScDesign::HullSizeLimit())
		{
			msgs.append(error.arg(QObject::tr("Hull is out of bounds forward. "
							"Move the forward side aft or move the whole design aft.")));
			isOk = false;
		}
		else if(m_HullMax.y < 0)
		{
			msgs.append(error.arg(QObject::tr("CG outside design extents forward. "
							"Move the forward side forward or move the whole design forward.")));
			isOk = false;
		}

		if(m_HullMin.z < -AuScDesign::HullSizeLimit())
		{
			msgs.append(error.arg(QObject::tr("Hull is out of bounds below. "
							"Move the bottom up or move the whole design up.")));
			isOk = false;
		}
		else if(m_HullMin.z > 0)
		{
			msgs.append(error.arg(QObject::tr("CG outside design extents below. "
							"Move the bottom down or move the whole design down.")));
			isOk = false;
		}

		if(m_HullMax.z > AuScDesign::HullSizeLimit())
		{
			msgs.append(error.arg(QObject::tr("Hull is out of bounds aloft. "
							"Move the top down or move the whole design down.")));
			isOk = false;
		}
		else if(m_HullMax.z < 0)
		{
			msgs.append(error.arg(QObject::tr("CG outside design extents aloft. "
							"Move the top up or move the whole design up.")));
			isOk = false;
		}
	}

	/*
	if(HullVolume() && !SkeletonVolume())
	{
		msgs.append(error.arg(QObject::tr("No framework. Allocate hull volume to framework.")));
		isOk = false;
	}
	*/

	if(!IsSpaceStation())
	{
		// Warn if too big to build on the ground.
		if(m_HullMin.x < -GroundSpawnSizeLimit() &&
				m_HullMax.x > GroundSpawnSizeLimit() &&
				m_HullMin.y < -GroundSpawnSizeLimit() &&
				m_HullMax.y > GroundSpawnSizeLimit() &&
				m_HullMin.z < -GroundSpawnSizeLimit() &&
				m_HullMax.z > GroundSpawnSizeLimit())
		{
			msgs.append(warning.arg(QObject::tr("The design must be built in space. "
							"All sides exceed the ground build size limit.")));
		}
		else if(m_HullMin.x < -GroundSpawnSizeLimit() ||
				m_HullMax.x > GroundSpawnSizeLimit() ||
				m_HullMin.y < -GroundSpawnSizeLimit() ||
				m_HullMax.y > GroundSpawnSizeLimit() ||
				m_HullMin.z < -GroundSpawnSizeLimit() ||
				m_HullMax.z > GroundSpawnSizeLimit())
		{
			QString text(QObject::tr("The design must be built in space. These sides exceed the ground build size limit: "));

			if(m_HullMin.x < -GroundSpawnSizeLimit())
				text += QObject::tr("port") + ", ";

			if(m_HullMax.x > GroundSpawnSizeLimit())
				text += QObject::tr("starboard") + ", ";

			if(m_HullMin.y < -GroundSpawnSizeLimit())
				text += QObject::tr("aft") + ", ";

			if(m_HullMax.y > GroundSpawnSizeLimit())
				text += QObject::tr("foreward") + ", ";

			if(m_HullMin.z < -GroundSpawnSizeLimit())
				text += QObject::tr("below") + ", ";

			if(m_HullMax.z > GroundSpawnSizeLimit())
				text += QObject::tr("aloft") + ", ";

			int comma = text.lastIndexOf(',');
			text[comma] = '.';

			msgs.append(warning.arg(text));
		}

		if(m_ManeuverDrive.Volume())
		{
			int vaccm = (int)ceil(ManeuverAccelerationVertical() / FeetPerMeter());
			if(vaccm < 10)
				msgs.append(warning.arg(QObject::tr("Vertical acceleration is weak. "
								"Allocate hull volume to maneuver drives or reduce mass.")));
		}
	}

	const double powerOutput = FuelBurnRate() * PowerPerFuel();	// power per second output by power plant
	if(m_PowerPlant.Volume() && IdlePowerWithShields() > powerOutput && FuelBurnRate() > 0)
		msgs.append(warning.arg(QObject::tr("Power plant is inadequate. "
					"Allocate hull volume to power plant or reduce idle power demand.")));

	if(m_Capacitor.Volume() && CapacitorMax() < IdlePowerWithShields()*60)
		msgs.append(warning.arg(QObject::tr("Capacitor is inadequate. "
					"Allocate hull volume to capacitor or reduce idle power demand.")));

	int warpDrives = 0;
	int wormholeDrives = 0;
	quint32 ftlUnitsNeeded = (quint32)ceil(m_FTLDriveUnitsToEngage / CubicFeetPerCubicMeter());	// cubic meters needed
	for(QHash<quint16, AuScReportSysVol>::const_iterator fi = FTLDrives().cbegin(); fi != FTLDrives().cend(); fi++)
	{
		COMMODITYID ftlDriveMod = (*fi).Module();
		quint8 ftlDriveQ = (*fi).Quality();
		int maxFTLSetting = AuScDesign::MaxCapacitorFTLSetting(ftlDriveMod, ftlDriveQ, m_FTLDriveUnitsToEngage, CapacitorMax());
		if((*fi).Volume() < ftlUnitsNeeded)
		{
			msgs.append(error.arg(QObject::tr("FTL drive inadequate to engage. Allocate hull volume to FTL drive %1.")
						.arg(fi.key())));
			isOk = false;
		}
		else if((*fi).Volume() == ftlUnitsNeeded)
			msgs.append(warning.arg(QObject::tr("FTL drive barely adequate to engage. Allocate hull volume to FTL drive %1.")
						.arg(fi.key())));

		if(maxFTLSetting < 0)
		{
			msgs.append(error.arg(QObject::tr("Capacitor inadequate to engage FTL drive %1. Allocate hull volume to capacitor.")
						.arg(fi.key())));
			isOk = false;
		}

		if(DbCommodity::IsWormholeFTLDriveModule((*fi).Module()))
			wormholeDrives++;
		else if(DbCommodity::IsWarpFTLDriveModule((*fi).Module()))
			warpDrives++;
	}

	if(wormholeDrives > 1)
	{
		msgs.append(error.arg(QObject::tr("Multiple wormhole FTL drives not allowed. "
						"Remove an engineer station that has a wormhole drive module.")));
		isOk = false;
	}

	if(warpDrives > 1)
	{
		msgs.append(error.arg(QObject::tr("Multiple warp FTL drives not allowed. "
						"Remove an engineer station that has a warp drive module.")));
		isOk = false;
	}

	if(EnginesRequireEngineer() && !HasEngineerStation())
	{
		msgs.append(warning.arg(QObject::tr("Power plant and maneuver drive combined volume exceeds 100m%1. "
						"Add an engineer station or reduce volume of power plant and maneuver drive.").arg(cubedSym)));
	}

	if(!m_Turrets.empty())
	{
		for(QHash<quint16, AuScReportTurret>::const_iterator ti = m_Turrets.cbegin(); ti != m_Turrets.cend(); ti++)
		{
			COMMODITYID turretMod = (*ti).Module();
			if(turretMod == CID_Invalid)
			{
				msgs.append(error.arg(QObject::tr("No module for turret %1. "
								"Select the turret and create an integrated gunner station. ").arg(ti.key())));
				isOk = false;
			}
		}
	}

	if(m_Sensor.Volume() && m_Sensor.Module() == CID_Invalid)
	{
		msgs.append(error.arg(QObject::tr("No control station for sensors. Add a sensor station "
						"or reduce volume of sensors.")));
		isOk = false;
	}
	else if(m_Sensor.Module() != CID_Invalid && !m_Sensor.Volume())
		msgs.append(warning.arg(QObject::tr("No sensors for sensor control station. "
						"Sensor range limited unless hull volume is allocated to sensors.")));

	if(m_Shield.Volume() && m_Shield.Module() == CID_Invalid)
	{
		msgs.append(error.arg(QObject::tr("No control station for shields. Add a shield station "
						"or reduce volume of shields.")));
		isOk = false;
	}
	else if(m_Shield.Module() != CID_Invalid && !m_Shield.Volume())
		msgs.append(warning.arg(QObject::tr("No shields for shield control station. "
						"Shield strength limited unless hull volume is allocated to shields.")));

	if(!HasSensor() && !WeaponBays().empty())
	{
		msgs.append(error.arg(QObject::tr("Sensor needed to lock on weapon bay targets. "
						"Add sensors or remove weapon bays.")));
		isOk = false;
	}

	for(QHash<quint16, AuScReportSysVol>::const_iterator bi = WeaponBays().begin(); bi != WeaponBays().end(); bi++)
	{
		COMMODITYID bayMod = (*bi).Module();
		quint32 wpns = (*bi).Volume();
		double wpnsCuFt = wpns * CubicFeetPerCubicMeter();

		if(wpns)
		{
			switch(bayMod)
			{
			case CID_BallisticWeaponBayModule:
				break;

			case CID_EnergyWeaponBayModule:
				{
					int projectors = AuScDesign::BayWeaponProjectors(wpnsCuFt);
					unsigned int shots = (unsigned int)(CapacitorMax() / projectors);	// truncated result, in shots
					if(!shots)
					{
						msgs.append(error.arg(QObject::tr("Capacitor inadequate to fire energy weapon bay %1 at full power. "
										"Allocate hull volume to capacitor.")
									.arg(bi.key())));
						isOk = false;
					}
					else if(shots == 1)
						msgs.append(warning.arg(QObject::tr("Capacitor adequate to fire energy weapon bay %1 only once at full power. "
										"Allocate hull volume to capacitor to increase.")
									.arg(bi.key())));
				}
				break;

			case CID_WeaponDeploymentBayModule:
			case CID_WeaponLauncherBayModule:
				break;

			case CID_AtmosphereCondenserBayModule:
			case CID_ParticleCollectorBayModule:
			case CID_ProspectorBayModule:
			case CID_SiphonBayModule:
			case CID_WildcatterBayModule:
				{
					// Calculate number of consecutive shots that can be fired at full power.
					double wpnUnits = wpnsCuFt / 256.;
					unsigned int shots = (unsigned int)(CapacitorMax() / wpnUnits);	// truncated result, in shots

					if(!shots)
					{
						msgs.append(error.arg(QObject::tr("Capacitor inadequate to fire harvester bay %1 at full power. "
										"Allocate hull volume to capacitor.")
									.arg(bi.key())));
						isOk = false;
					}
					else if(shots == 1)
						msgs.append(warning.arg(QObject::tr("Capacitor adequate to fire harvester bay %1 only once at full power. "
										"Allocate hull volume to capacitor.")
									.arg(bi.key())));
				}
				break;

			case CID_TractorBeamBayModule:
				{
					// Calculate number of consecutive shots that can be fired at full power.
					double wpnUnits = wpnsCuFt / 256.;
					unsigned int shots = (unsigned int)CapacitorMax() / wpnUnits;	// truncated result, in shots
					if(!shots)
					{
						msgs.append(error.arg(QObject::tr("Capacitor inadequate to attach tractor beam %1. "
										"Allocate hull volume to capacitor.")
									.arg(bi.key())));
						isOk = false;
					}
				}
				break;

			default:
				break;
			}
		}
		else
		{
			msgs.append(error.arg(QObject::tr("Weapon size inadequate to fire. Allocate hull volume to weapon %1.")
						.arg(bi.key())));
			isOk = false;
		}
	}

	// Quarters
	if(CaptBerths() || OfficerBerths() || CrewBerths() || PassengerBerths() || TroopBerths())
	{
		if(!CaptBerths() && Structure() != SC_Spaceship)
			msgs.append(warning.arg(QObject::tr("No captain berth for avatar. Add a captain berth.")));

		if(!OfficerBerths() && Structure() != SC_Spaceship)
			msgs.append(warning.arg(QObject::tr("No officer berth for NPC officer. Add an officer berth.")));

		if(CrewStationCount() > CrewBerths())
			msgs.append(warning.arg(QObject::tr("Not enough crew berths for %L1 crew stations. "
						"Add crew berths or remove crew stations.").arg(CrewStationCount())));

		int ts = Turrets().size() + GuardPosts().size();
		if(ts > TroopBerths())
			msgs.append(warning.arg(QObject::tr("Not enough troop berths for %L1 troop stations. "
						"Add troop berths or remove turrets or guard posts.").arg(ts)));
	}

	// Insure not too many of each kind of station or they get abused.
	int stationLimit = 10;
	if(CommandStationCount() > stationLimit)
	{
		msgs.append(error.arg(QObject::tr("Too many command stations. Limit is %1.").arg(stationLimit)));
		isOk = false;
	}
	if(m_Transporters.size() > stationLimit)
	{
		msgs.append(error.arg(QObject::tr("Too many transporter stations. Limit is %1.").arg(stationLimit)));
		isOk = false;
	}
	QHash<AuScPart::PART_TYPE, int> stationsByType;
	for(AuScReportStationHash::const_iterator si = m_CrewStations.cbegin(); si != m_CrewStations.cend(); si++)
	{
		AuScPart::PART_TYPE pt = (AuScPart::PART_TYPE)(*si).PartType();
		QHash<AuScPart::PART_TYPE, int>::iterator ti = stationsByType.find(pt);
		if(ti != stationsByType.end())
			(*ti)++;
		else
			stationsByType.insert(pt, 1);
	}
	for(QHash<AuScPart::PART_TYPE, int>::const_iterator ti = stationsByType.cbegin(); ti != stationsByType.cend(); ti++)
	{
		if(*ti > stationLimit)
		{
			AuScPart* p = AuScPart::Create(ti.key());
			QString stationType;
			if(p)
				stationType = p->TypeName();
			else
				stationType = QObject::tr("Unknown Part Type %1").arg(ti.key());
			msgs.append(error.arg(QObject::tr("Too many %2. Limit is %1.").arg(stationLimit).arg(stationType)));
			isOk = false;
		}
	}
	int turretLimit = 30;
	if(m_Turrets.size() > turretLimit)
	{
		msgs.append(error.arg(QObject::tr("Too many turrets. Limit is %1.").arg(turretLimit)));
		isOk = false;
	}

	// Does the design have any voids?
	if(design && isOk)
	{
		QMap<QString, bool> rooms;			// Map of names of room voids, for different room count.
		bool hasAssoc = false;
		bool hasDoor = false;
		bool hasEnvVoid = false;
		bool hasHullDoor = false;
		bool hasLift = false;
		bool hasLiftDoor = false;
		bool hasPath = false;
		bool hasServicePanel = false;
		int hullAndGear = 0;				// Count of hull and landing gear parts.
		const AuScPart* biggestHull = 0;
		double biggestSize = 0;
		QSet<quint16> dups;					// Ids of parts that have exact duplicates, so we don't report them twice.
		int postCitizens = 0;
		int postGuards = 0;
		int postLivestock = 0;
		for(AuScParts::const_iterator pi = design->Parts().cbegin(); pi != design->Parts().cend() && isOk; pi++)
		{
			if((*pi)->IsOmitted())
				continue;

			if((*pi)->AssociatedPartId())
				hasAssoc = true;

			if((*pi)->IsTurboLiftMesh())
				hasLift = true;
			else if((*pi)->IsTurboLiftDoor())
				hasLiftDoor = true;

			if((*pi)->IsEnvironmentVoid())
				hasEnvVoid = true;

			// Count posts.
			if((*pi)->IsCitizenPost())
				postCitizens++;
			else if((*pi)->IsGuardPost())
				postGuards++;
			else if((*pi)->IsLivestockPost())
				postLivestock++;

			if((*pi)->Type() == AuScPart::Part_PanelFTLDrive || (*pi)->Type() == AuScPart::Part_PanelWeapon)
				hasServicePanel = true;

			if(!(*pi)->CheckVoidFacesPlanar())
			{
				msgs.append(error.arg(QObject::tr("%1 %2 has one or more faces that are not planar. "
								"Faces of voids must not be bent. "
								"Fracture the bent face(s) into triangles or reshape them so they are not bent. ")
							.arg((*pi)->PartName())
							.arg(pi.key())));
				//isOk = false;
				//break;
			}

			if((*pi)->IsHullMesh() || (*pi)->IsLandingGear())
			{
				hullAndGear++;

				AuPoint min,max;
				(*pi)->HullExtents(min, max);
				double sz = min.Distance2(max);
				if(sz > biggestSize)
				{
					biggestHull = *pi;
					biggestSize = sz;
				}
			}
			else if((*pi)->IsTurboLiftMesh() || (*pi)->IsRoomVoid() || (*pi)->IsRoomMesh() || (*pi)->IsWindow())
			{
				if((*pi)->IsRoomVoid())
				{
					if(!rooms.contains((*pi)->GroupName()))
						rooms.insert((*pi)->GroupName(), true);
				}

				// Insure at least one vertex of the turbo lift is inside the hull obstruction model.
				bool anyInside = false;
				QSet<quint32> vertexIds;
				(*pi)->VertexIds(vertexIds);
				for(QSet<quint32>::const_iterator vi = vertexIds.cbegin(); vi != vertexIds.cend(); vi++)
				{
					AuScPoint p = (*pi)->VertexLoc(*vi);
					if(design->Parts().IsInsideObstructionModel(p))
					{
						anyInside = true;
						break;
					}
				}

				if(!anyInside)
				{
					msgs.append(error.arg(QObject::tr("%1 %2 is outside the hull. "
									"Modify or move the part so all of its vertices are inside the hull. ")
								.arg((*pi)->PartName())
								.arg(pi.key())));
					isOk = false;
				}

				if(!(*pi)->HasGroupName())
				{
					if((*pi)->IsTurboLiftMesh())
						msgs.append(warning.arg(QObject::tr("%1 %2 has no group. "
										"Turbo lifts should be assigned a group for lighting and air pressure. ")
									.arg((*pi)->PartName())
									.arg(pi.key())));
					else if((*pi)->IsRoomVoid())
						msgs.append(warning.arg(QObject::tr("%1 %2 has no group. "
										"Room voids should be assigned a group for lighting and air pressure. ")
									.arg((*pi)->PartName())
									.arg(pi.key())));
					else if((*pi)->IsRoomMesh())
						msgs.append(warning.arg(QObject::tr("%1 %2 has no group. "
										"Rooms should be assigned a group for lighting. ")
									.arg((*pi)->PartName())
									.arg(pi.key())));
					else if((*pi)->IsWindow() && (*pi)->Opacity() < 1)
						msgs.append(warning.arg(QObject::tr("%1 %2 has no group. "
										"Transparent windows should be assigned a group so they will be lit by room lighting. ")
									.arg((*pi)->PartName())
									.arg(pi.key())));
				}
			}
			else if((*pi)->IsHullDoor())
				hasHullDoor = true;
			else if((*pi)->IsDoor())
				hasDoor = true;
			else if((*pi)->IsPath())
			{
				hasPath = true;

				// Insure each end of the path is not inside solid mass.
				AuSegment seg((*pi)->PathSegment(0).m_Seg);
				bool end1clear = design->Parts().IsPathEverUnobstructed(seg.m_fLoc1);
				bool end2clear = design->Parts().IsPathEverUnobstructed(seg.m_fLoc2);
				if(!end1clear && !end2clear)
				{
					msgs.append(error.arg(QObject::tr("%1 %2 blocked at both ends. "
									"Move the path so each end is inside a void or outside the hull obstruction model. ")
								.arg((*pi)->PartName()))
								.arg(pi.key()));
					isOk = false;
				}
				else if(!end1clear && end2clear)
				{
					seg.m_fLoc1 /= FeetPerMeter();	// Convert to meters.
					seg.m_fLoc1.Round(.001);		// Round to nearest millimeter.
					msgs.append(error.arg(QObject::tr("%1 %2 blocked at end location (%L3, %L4, %L5)m. "
									"Move the path so each end is inside a void or outside the hull obstruction model. ")
								.arg((*pi)->PartName())
								.arg(pi.key())
								.arg(seg.m_fLoc1.x)
								.arg(seg.m_fLoc1.y)
								.arg(seg.m_fLoc1.z)));
					isOk = false;
				}
				else if(!end2clear && end1clear)
				{
					seg.m_fLoc2 /= FeetPerMeter();	// Convert to meters.
					seg.m_fLoc2.Round(.001);		// Round to nearest millimeter.
					msgs.append(error.arg(QObject::tr("%1 %2 blocked at end location (%L3, %L4, %L5)m. "
									"Move the path so each end is inside a void or outside the hull obstruction model. ")
								.arg((*pi)->PartName())
								.arg(pi.key())
								.arg(seg.m_fLoc2.x)
								.arg(seg.m_fLoc2.y)
								.arg(seg.m_fLoc2.z)));
					isOk = false;
				}
			}
			else if((*pi)->IsLooselyAttachedToHull())
			{
				// Insure at least one vertex of the mesh is inside the hull obstruction model.
				bool anyInside = false;
				QSet<quint32> vertexIds;
				(*pi)->VertexIds(vertexIds);
				for(QSet<quint32>::const_iterator vi = vertexIds.cbegin(); vi != vertexIds.cend(); vi++)
				{
					AuScPoint p = (*pi)->VertexLoc(*vi);
					if(design->Parts().IsInsideObstructionModel(p))
					{
						anyInside = true;
						break;
					}
				}

				if(!anyInside)
				{
					msgs.append(error.arg(QObject::tr("%1 %2 is completely outside the hull. "
									"Modify or move the part so at least one of its vertices is inside the hull. ")
								.arg((*pi)->PartName())
								.arg(pi.key())));
					isOk = false;
				}
			}
			else if((*pi)->IsMesh())
			{
				// Insure every vertex of the mesh is inside the hull obstruction model.
				QSet<quint32> vertexIds;
				(*pi)->VertexIds(vertexIds);
				for(QSet<quint32>::const_iterator vi = vertexIds.cbegin(); vi != vertexIds.cend(); vi++)
				{
					AuScPoint p = (*pi)->VertexLoc(*vi);
					if(design->Parts().IsInsideObstructionModel(p))
						continue;

					p /= FeetPerMeter();	// Convert to meters.
					p.Round(.001);			// Round to nearest millimeter.
					msgs.append(error.arg(QObject::tr("%1 %2 is outside the hull at (%L3, %L4, %L5)m. "
									"Modify or move the part so all of its vertices are inside the hull. ")
								.arg((*pi)->PartName())
								.arg(pi.key())
								.arg(p.x)
								.arg(p.y)
								.arg(p.z)));
					isOk = false;
					break;	// Enough whining about that part.
				}
			}

			// Check for duplicates.
			for(AuScParts::const_iterator di = design->Parts().cbegin(); di != design->Parts().cend(); di++)
			{
				if(pi.key() == di.key())
					continue;	// Don't check a part against itself.
				if(dups.contains(di.key()))
					continue;	// Already reported that part.
				if(*(*pi) != *(*di))
					continue;
				dups.insert(pi.key());
				dups.insert(di.key());
				if((*pi)->IsBerth() || (*pi)->IsPath())
				{
					msgs.append(warning.arg(QObject::tr("Part %1 %2 is an exact duplicate of part %3 %4. Delete one of them.")
								.arg(pi.key())
								.arg((*pi)->PartName())
								.arg(di.key())
								.arg((*di)->PartName())));
				}
				else
				{
					msgs.append(error.arg(QObject::tr("Part %1 %2 is an exact duplicate of part %3 %4. Delete one of them.")
								.arg(pi.key())
								.arg((*pi)->PartName())
								.arg(di.key())
								.arg((*di)->PartName())));
					isOk = false;
				}
				break;
			}

			if(!isOk)
				msgs.append(warning.arg(QObject::tr("Skipping remainder of detailed analysis due to error, to save time.")));
		}

		if(hullAndGear)
		{
			// Test hull continuity. Check for disjoint hull parts.
			QSet<const AuScPart*> hull;			// Parts of the hull and landing gear found to be contiguous with each other.

			// Start with the largest hull part. This algorithm might not work if we started with a tiny part.
			hull.insert(biggestHull);

			bool anyAdded;						// True if we add any parts to 'hull' in each pass.
			do
			{
				anyAdded = false;
				for(AuScParts::const_iterator pi = design->Parts().cbegin(); pi != design->Parts().cend(); pi++)
				{
					if((*pi)->IsOmitted())
						continue;
					if(!(*pi)->IsHullMesh() && !(*pi)->IsLandingGear())
						continue;					// We are looking for hull and landing gear parts.
					if(hull.contains(*pi))
						continue;					// Skip parts that are already part of the hull.

					// Check the part to see if it is contiguous with the hull we have accumulated so far.
					// Get vertices of the part.
					QSet<quint32> vertexIds;
					(*pi)->VertexIds(vertexIds);

					// Find at least one vertex of the part that is in the obstruction model of the accumulated hull.
					bool isCont = false;
					for(QSet<quint32>::const_iterator vi = vertexIds.cbegin(); vi != vertexIds.cend() && !isCont; vi++)
					{
						AuScPoint p = (*pi)->VertexLoc(*vi);
						for(QSet<const AuScPart*>::const_iterator hi = hull.cbegin(); hi != hull.cend(); hi++)
						{
							if((*hi)->IsInsideObstructionModel(p))
							{
								// A vertex of the part is inside the obstructio model of our accumulated hull.
								isCont = true;
								hull.insert(*pi);
								anyAdded = true;
								break;
							}
						}
					}

					if(!isCont)
					{
						// Try the same thing the other way around.
						// Find at least one vertex of the accumulated hull that is inside the obstruction model of the part.
						for(QSet<const AuScPart*>::const_iterator hi = hull.cbegin(); hi != hull.cend() && !isCont; hi++)
						{
							// Get vertices of the hull part.
							QSet<quint32> hvertexIds;
							(*hi)->VertexIds(hvertexIds);

							for(QSet<quint32>::const_iterator hvi = hvertexIds.cbegin(); hvi != hvertexIds.cend(); hvi++)
							{
								AuScPoint hp = (*hi)->VertexLoc(*hvi);
								if((*pi)->IsInsideObstructionModel(hp))
								{
									// A vertex of the hull is inside the obstruction model of the part.
									isCont = true;
									hull.insert(*pi);
									anyAdded = true;
									break;
								}
							}
						}
					}
				}
			}
			while(anyAdded);

			// Did all of our hull and landing gear parts get accounted for?
			if(hull.size() < hullAndGear)
			{
				// Report the disjoint parts.
				isOk = false;
				for(AuScParts::const_iterator pi = design->Parts().cbegin(); pi != design->Parts().cend(); pi++)
				{
					if((*pi)->IsOmitted())
						continue;
					if(!(*pi)->IsHullMesh() && !(*pi)->IsLandingGear())
						continue;					// We are looking for hull and landing gear parts.
					if(hull.contains(*pi))
						continue;					// Skip parts that are already part of the hull.
					msgs.append(error.arg(QObject::tr("%1 %2 is separate from the hull. "
									"Move the part closer to other hull or landing gear parts. ")
								.arg((*pi)->PartName())
								.arg(pi.key())));
				}
			}
		}
		else if(isOk)
		{
			msgs.append(error.arg(QObject::tr("No hull. Add one or more hull parts to define the shape of the spacecraft. ")));
			isOk = false;
		}

		if(hasEnvVoid)
			msgs.append(error.arg(QObject::tr("Environment void not permitted in spacecraft blueprint.")));

		if(rooms.empty() && isOk)
		{
			msgs.append(error.arg(QObject::tr("No room voids. Add a room void for access to the inside. ")));
			isOk = false;
		}

		if(postCitizens > 100)
		{
			msgs.append(error.arg(QObject::tr("Too many citizen posts. "
							"%1 citizen posts exceed limit of 100.")
						.arg(postCitizens)));
			isOk = false;
		}

		if(postGuards > m_TroopBerths)
		{
			msgs.append(error.arg(QObject::tr("Too many troop posts. "
							"%1 troop posts exceed the %2 troop berths available. "
							"Remove %3 of them.")
						.arg(postGuards)
						.arg(m_TroopBerths)
						.arg(m_TroopBerths - postGuards)));
			isOk = false;
		}

		if(postLivestock > 10)
		{
			msgs.append(error.arg(QObject::tr("More than 10 livestock posts. "
							"Remove at least %1 of them. ")
						.arg(postLivestock - 10)));
			isOk = false;
		}
		else if(postLivestock > 2)
			msgs.append(note.arg(QObject::tr("More than 2 livestock posts. "
						"Up to 2 will be used at a time, selected randomly when needed. ")));

		QStringList allLiftStops;
		if(hasLift)
		{
			// Check lifts.
			for(AuScParts::const_iterator pi = design->Parts().cbegin(); pi != design->Parts().cend(); pi++)
			{
				if((*pi)->IsOmitted())
					continue;
				if(!(*pi)->IsTurboLiftMesh())
					continue;
				QStringList liftStops = (*pi)->StopNames();
				if(liftStops.isEmpty())
				{
					msgs.append(error.arg(QObject::tr("Turbo lift %1 has no stops. ").arg(pi.key())));
					isOk = false;
					continue;
				}

				allLiftStops.append(liftStops);

				// Check each stop for a door with same name.
				for(QStringList::const_iterator si = liftStops.cbegin(); si != liftStops.cend(); si++)
				{
					// Locate a door with the same name as the lift stop.
					quint16 doorId = 0;
					for(AuScParts::const_iterator di = design->Parts().cbegin(); di != design->Parts().cend() && !doorId; di++)
					{
						if((*di)->IsOmitted())
							continue;
						if((*di)->IsDoor())
						{
							if((*di)->StopName() == *si)
								doorId = di.key();	// Found a door with a matching stop name.
						}
					}
					if(!doorId)
					{
						msgs.append(error.arg(QObject::tr("Turbo lift %1 has no door at stop <b>%2</b>. "
										"Create a door at the stop and change its name to %2.").arg(pi.key()).arg(*si)));
						isOk = false;
					}
				}
			}
		}

		if(hasDoor || hasHullDoor)
		{
			// Check doors to insure each has a room void associated with its open state.
			for(AuScParts::const_iterator pi = design->Parts().cbegin(); pi != design->Parts().cend(); pi++)
			{
				if((*pi)->IsOmitted())
					continue;
				if(!(*pi)->IsDoor())
					continue;

				const AuScDoor* door = dynamic_cast<const AuScDoor*>(*pi);
				if(!door)
					continue;

				if(!door->HasState(AuScDoor::Open))
				{
					msgs.append(warning.arg(QObject::tr("Door %1 has no open state. "
									"That's ok if it is just decorative.").arg(pi.key())));
					continue;
				}

				bool foundVoid = false;
				for(AuScParts::const_iterator vi = design->Parts().cbegin(); vi != design->Parts().cend(); vi++)
				{
					if((*vi)->IsOmitted())
						continue;
					if(!(*vi)->IsRoomVoid())
						continue;
					if((*vi)->AssociatedPartId() != pi.key())
						continue;
					if((*vi)->AssociatedPartState() != AuScDoor::Open)
						continue;
					foundVoid = true;
					break;
				}

				if(!foundVoid)
				{
					msgs.append(warning.arg(QObject::tr("Door %1 has no room void associated with its open state. "
									"Associate a room void with its open state.").arg(pi.key())));
				}

				if(door->IsHullDoor())
				{
					// Hull door also needs a hull void associated with its open state.
					bool foundVoid = false;
					for(AuScParts::const_iterator vi = design->Parts().cbegin(); vi != design->Parts().cend(); vi++)
					{
						if((*vi)->IsOmitted())
							continue;
						if(!(*vi)->IsHullVoid())
							continue;
						if((*vi)->AssociatedPartId() != pi.key())
							continue;
						if((*vi)->AssociatedPartState() != AuScDoor::Open)
							continue;
						foundVoid = true;
						break;
					}

					if(!foundVoid)
					{
						msgs.append(warning.arg(QObject::tr("Hull door %1 has no hull void associated with its open state. "
										"Associate a hull void with its open state.").arg(pi.key())));
					}
				}
			}
		}

		if(hasLiftDoor && isOk)
		{
			// Check turbo lift doors to make sure they really have a corresponding lift stop.
			for(AuScParts::const_iterator pi = design->Parts().cbegin(); pi != design->Parts().cend(); pi++)
			{
				if((*pi)->IsOmitted())
					continue;
				if(!(*pi)->IsTurboLiftDoor())
					continue;
				if(allLiftStops.contains((*pi)->StopName()))
					continue;
				msgs.append(error.arg(QObject::tr("Turbo lift door %1 has no turbo lift with a stop called <b>%2</b>. "
								"Clear its name or rename it to match a turbo lift stop.")
							.arg(pi.key())
							.arg((*pi)->StopName())));
				isOk = false;
			}
		}

		if(hasServicePanel)
		{
			// Check turbo lift doors to make sure they really have a corresponding lift stop.
			for(AuScParts::const_iterator pi = design->Parts().cbegin(); pi != design->Parts().cend(); pi++)
			{
				if((*pi)->IsOmitted())
					continue;
				if((*pi)->Type() != AuScPart::Part_PanelFTLDrive && (*pi)->Type() != AuScPart::Part_PanelWeapon)
					continue;
				if((*pi)->AssociatedPartId())
					continue;
				msgs.append(error.arg(QObject::tr("%1 %2 is not associated with any system. "
								"Associate it with an appropriate control station. ").arg((*pi)->PartName()).arg(pi.key())));
				isOk = false;
			}
		}

		if(hasAssoc)
		{
			// Check for existence of associated parts.
			for(AuScParts::const_iterator pi = design->Parts().cbegin(); pi != design->Parts().cend(); pi++)
			{
				if((*pi)->IsOmitted())
					continue;
				if(!(*pi)->AssociatedPartId())
					continue;
				AuScParts::const_iterator api = design->Parts().find((*pi)->AssociatedPartId());
				if(api != design->Parts().cend())
				{
					// Associated part found.
					if((*api)->IsOmitted())
					{
						msgs.append(error.arg(QObject::tr("%1 %2 is associated with omitted part %3. "
										"Dissociate it or restore the associated part. ")
									.arg((*pi)->PartName()).arg(pi.key()).arg((*pi)->AssociatedPartId())));
						isOk = false;
					}
				}
				else
				{
					msgs.append(error.arg(QObject::tr("%1 %2 is associated with a nonexistent part %3. Dissociate it. ")
								.arg((*pi)->PartName()).arg(pi.key()).arg((*pi)->AssociatedPartId())));
					isOk = false;
				}
			}
		}

		quint16 troom = m_Transporters.CheckAmbiguousRoom();
		if(troom)
		{
			msgs.append(error.arg(QObject::tr("Room void %1 contains more than one transporter station. "
							"Remove a transporter station or move it to a different room void. ")
						.arg(troom)));
			isOk = false;
		}

		if(isOk)
		{
			if(!hasPath)
				msgs.append(warning.arg(QObject::tr("No paths. NPCs cannot find their way inside. ")));

			if(!hasDoor && rooms.size() > 1)
				msgs.append(warning.arg(QObject::tr("No doors. Doors may be needed to connect rooms. ")));

			if(!hasHullDoor)
				msgs.append(warning.arg(QObject::tr("No hull doors. A door may be needed to get inside from outside. ")));
		}
	}
	else if(design)
		msgs.append(warning.arg(QObject::tr("Skipping detailed analysis due to error, to save time.")));

	if(isOk && design)
		msgs.append(note.arg(QObject::tr("Success! A spacecraft can be manufactured using the blueprint.")));

	return isOk;
}

QString AuScReport::ReportAnalysis(quint8 designState) const
{
	if(designState == AuDesign::Deleted)
		return QObject::tr("Deleted Design");
	else if(designState == AuDesign::Edit && !HullVolume() && !g_Au.IsWebCGI())
		return "";		// Return an empty string. That cues the caller to provide helpful design tips in place of this report.

	QString cubedSym,currencySym,squaredSym;
	if(g_Au.IsWebCGI())
	{
		cubedSym = QString("³");
		currencySym = QString("¢");
		squaredSym = QString("²");
	}
	else
	{
		cubedSym = g_Au.CubedSymbol();
		currencySym = g_Au.CurrencySymbol();
		squaredSym = g_Au.SquaredSymbol();
	}

	// Design name
	// This name includes the architect name. e.g. "Scout Ship by Bob"
	// The working design will be blank.
	QString stat;	// This will be our return value.
	if(!m_DesignName.isEmpty() && !m_ArchitectName.isEmpty())
		stat = QString("<b>%1 by %2</b><br>").arg(m_DesignName).arg(m_ArchitectName);
	else if(!m_DesignName.isEmpty())
		stat = QString("<b>%1</b><br>").arg(m_DesignName);
	else if(!m_ArchitectName.isEmpty())
		stat = QString("<b>by %1</b><br>").arg(m_ArchitectName);

	if(!m_PartArchitects.Empty())
	{
		// Sort collaborating architects alphabetically.
		QMultiMap<QString, AuId> sortedNames;
		for(QHash<AuId,QString>::const_iterator ai = m_PartArchitects.Cbegin(); ai != m_PartArchitects.Cend(); ai++)
			sortedNames.insert((*ai).toUpper(), ai.key());

		stat += "<div style=\"margin-left: 10px;\">";
		stat += QObject::tr("Additional Architects");
		stat += "<div style=\"margin-left: 10px;\">";
		for(QMultiMap<QString, AuId>::const_iterator si = sortedNames.cbegin(); si != sortedNames.cend(); si++)
		{
			QHash<AuId,QString>::const_iterator ai = m_PartArchitects.Find(*si);
			if(ai != m_PartArchitects.Cend())
				stat += QString("%1 <i>%2</i><br>").arg(*ai).arg(ai.key().StringId());
		}
		//stat = stat.left(stat.count() - 4);	// Drop the trailing '<br>'.
		stat += "</div>";
		stat += "</div>";
	}

	// Description.
	if(!m_DesignDesc.isEmpty())
	{
		stat += QObject::tr("<b>DESCRIPTION</b>");
		stat += "<div style=\"margin-left: 10px;\">";
		stat += m_DesignDesc;
		stat += "</div>";
		stat += "<hr>";
	}

	// Watch for empty design.
	if(HullVolume())
	{
		// Calculate armor "level" based on hull volume.
		double armor = ArmorLevel();
		double rarmor = FROUND(armor, .01);
		const double mass = m_MassKg;

		// Summary
		{
			stat += "<b>";
			/*
			if(!m_IsContiguous)
				stat += QObject::tr("DISJOINT ");
				*/

			stat += StreamlineName(Streamline()).toUpper() + " ";

			switch(m_Structure)
			{
			case SC_Hull:
			case SC_Deprecated:
				stat += QObject::tr("INVALID");
				break;
			case SC_Starship:
				stat += QObject::tr("STARSHIP");
				break;
			case SC_Spaceship:
				stat += QObject::tr("SPACESHIP");
				break;
			case SC_Station:
				stat += QObject::tr("STATION");
				break;
			}
			stat += "</b>";

			// Quality
			// NOTE: Ship hit points are multiplied by 10 when reported. This is due to the fact that 10 hits of damage only
			//       causes one unit of damage to the ship. Each unit of ship damage is represented by a commodity that needs
			//       to be replaced. It takes 10 points of damage inflicted to cause one unit of ship damage.
			stat += "<div style=\"margin-left: 10px;\">";
			stat += QObject::tr("Minimum Manufacture Quality %1").arg(m_Quality);
			stat += "<div style=\"margin-left: 10px;\">";
			stat += "</div>";
			stat += "</div>";

			// Estimated cost.
			quint8 costEstQ = Quality();
			if(costEstQ < 1)
				costEstQ = 1;
			stat += "<div style=\"margin-left: 10px;\">";
			stat += QObject::tr("Estimated Cost %L1%2 Using Q%4 Components")
				.arg(EstimatedCost(),0,'f',0).arg(currencySym).arg(costEstQ);
			stat += "</div>";

			// Preferred station orbit orientation.
			if(IsSpaceStation())
			{
				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("Preferred Orbit Orientation: %1")
					.arg(AuScDesign::StationOrientationName((AuScDesign::STATION_ORIENT)m_StationOrient));
				stat += "</div>";
			}
			else
			{
				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("Landing Orientation: %1")
					.arg(::LandingOrientationName(LandingOrientation()));
				stat += "</div>";
			}

			if(designState == AuDesign::Edit)
			{
				// Provide the warning and error messages that the finalizer shows.
				QStringList msgs;
				OkToManufacture(msgs, 0);
				if(!msgs.empty())
				{
					stat += "<div style=\"margin-left: 10px;\">";
					stat += QObject::tr("Notes");
					stat += "<div style=\"margin-left: 10px;\">";
					for(QStringList::const_iterator ci = msgs.cbegin(); ci != msgs.cend(); ci++)
					{
						if(ci != msgs.cbegin())
							stat += "<br>";
						stat += *ci;
					}
					stat += "</div>";
					stat += "</div>";
				}
			}
		}

		// Hull
		{
			stat += QObject::tr("<hr><b>HULL</b>");
			stat += "<div style=\"margin-left: 10px;\">";
			stat += QObject::tr("%3 Q%4 Construction<br>").arg(g_Au.CommodityName(HullMaterial())).arg(m_Hull.Quality());

			// Armor
			double hullTLMult = QualityBoostF(m_Hull.Quality());
			const DbCommodity& hullC = g_Au.Commodity(m_Hull.Module());
			double hullAbsorbK = (hullC.ArmorModeAbsorb(WM_Contact) + hullC.ArmorEffectAbsorb(WE_Generic)) * hullTLMult;
			double hullBlockK = (hullC.ArmorModeBlock(WM_Contact) + hullC.ArmorEffectBlock(WE_Generic)) * hullTLMult * (armor+1);
			if(armor)
				stat += QObject::tr("Armor %1 Absorbs %2 Damage")
					.arg(rarmor)
					.arg(g_Au.FormatArmorFactors(hullAbsorbK, hullBlockK));
			else
				stat += QObject::tr("Unarmored Absorbs %1 Damage")
					.arg(g_Au.FormatArmorFactors(hullAbsorbK, hullBlockK));

			float frame = round(SkeletonLevel() * 100);
			stat += QObject::tr("<br>Framework %1% of Structure")
				.arg(frame);

			// Volume
			double width = FROUND((m_HullMax.x - m_HullMin.x)/FeetPerMeter(), 0.01);
			double length = FROUND((m_HullMax.y - m_HullMin.y)/FeetPerMeter(), 0.01);
			double height = FROUND((m_HullMax.z - m_HullMin.z)/FeetPerMeter(), 0.01);
			stat += QObject::tr("<br>%L1m%2 Volume").arg(HullVolume()).arg(cubedSym);
			stat += "<div style=\"margin-left: 10px;\">";

			if(Structure() == SC_Station)
			{
				// Station implicitly must spawn in space. No need to confuse by highlighting sizes.
				stat += QObject::tr("%L1m Length<br>").arg(length, 0,'f',0);
				stat += QObject::tr("%L1m Width<br>").arg(width, 0,'f',0);
				stat += QObject::tr("%L1m Height").arg(height, 0,'f',0);
			}
			else
			{
				// Large spacecraft must spawn in space.
				double groundlim = GroundSpawnSizeLimit() * 2;
				if(length > groundlim)
					stat += QObject::tr("%L1m Length, <b>Must be Built at Station</b><br>").arg(length, 0,'f',0);
				else
					stat += QObject::tr("%L1m Length<br>").arg(length, 0,'f',0);
				if(width > groundlim)
					stat += QObject::tr("%L1m Width, <b>Must be Built at Station</b><br>").arg(width, 0,'f',0);
				else
					stat += QObject::tr("%L1m Width<br>").arg(width, 0,'f',0);
				if(height > groundlim)
					stat += QObject::tr("%L1m Height, <b>Must be Built at Station</b><br>").arg(height, 0,'f',0);
				else
					stat += QObject::tr("%L1m Height<br>").arg(height, 0,'f',0);
				stat = stat.left(stat.count() - 4);	// Drop the trailing '<br>'.
			}
			stat += "</div>";
			stat += "</div>";

			// Mass
			stat += "<div style=\"margin-left: 10px;\">";
			stat += QObject::tr("%L1kg Mass").arg(mass, 0,'f',0);
			stat += "<div style=\"margin-left: 10px;\">";
			stat += QObject::tr("%L1kg Equipment<br>").arg(m_MassEquipmentKg, 0,'f',0);
			stat += QObject::tr("%L1kg Fuel<br>").arg(m_MassFuelKg, 0,'f',0);
			stat += QObject::tr("%L1kg Hull").arg(m_MassHullKg, 0,'f',0);
			stat += "</div>";
			stat += "</div>";

			// Model Size
			stat += "<div style=\"margin-left: 10px;\">";
			stat += QObject::tr("Model Size");
			stat += "<div style=\"margin-left: 10px;\">";
			stat += QObject::tr("%L1 Faces<br>").arg(FaceCount());
			stat += QObject::tr("%L1 Triangles").arg(TriangleCount());
			stat += "</div>";
			stat += "</div>";
		}

		// Engines
		if(m_Capacitor.Volume() || m_PowerPlant.Volume() || m_ManeuverDrive.Volume() || !m_FTLDrives.empty())
		{
			stat += QObject::tr("<hr><b>ENGINEERING</b>");
			if(m_ManeuverDrive.Volume())
			{
				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("%L1m%2 Maneuver Drive").arg(m_ManeuverDrive.Volume()).arg(cubedSym);
				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("%1 Q%2<br>").arg(g_Au.CommodityName(m_ManeuverDrive.Module()))
					.arg(m_ManeuverDrive.Quality());

				int accm = (int)ceil(m_Acceleration / FeetPerMeter());
				if(designState != AuDesign::Final && Structure() == SC_Station)
					stat += QObject::tr("%L1m/sec%2 Acceleration "
							"<span style=\"color: rgb(32, 255, 255);\">"
							"<b>STATION</b></span><br>").arg(accm).arg(squaredSym);
				else if(designState != AuDesign::Final && accm < 200)
					stat += QObject::tr("%L1m/sec%2 Acceleration "
							"<span style=\"color: rgb(255, 255, 32);\">"
							"<b>SLOW</b></span><br>").arg(accm).arg(squaredSym);
				else if(designState != AuDesign::Final && accm >= 500)
					stat += QObject::tr("%L1m/sec%2 Acceleration "
							"<span style=\"color: rgb(32, 255, 32);\">"
							"<b><i>FAST</i></b></span><br>").arg(accm).arg(squaredSym);
				else
					stat += QObject::tr("%L1m/sec%2 Acceleration<br>").arg(accm).arg(squaredSym);

				// .16 is the acceleration at max vertical throttle. See DaSc::Acceleration.
				int vaccm = (int)ceil(m_AccelerationVert / FeetPerMeter());
				if(designState != AuDesign::Final && Structure() != SC_Station && vaccm < 10)
					stat += QObject::tr("%L1m/sec%2 Vertical Acceleration "
							"<span style=\"color: rgb(255, 255, 32);\"><b>WEAK</b></span>").arg(vaccm).arg(squaredSym);
				else
					stat += QObject::tr("%L1m/sec%2 Vertical Acceleration").arg(vaccm).arg(squaredSym);

				stat += "</div>";
				stat += "</div>";
			}

			// Show power consumption.
			stat += "<div style=\"margin-left: 10px;\">";
			stat += QObject::tr("Power Consumption");
			stat += "<div style=\"margin-left: 10px;\">";
			if(m_Shield.Volume())
			{
				stat += QObject::tr("%1 Terawatts Idling: "
						"Life Support, Maneuver Drive, Sensor Powered On. Shields Down.<br>")
					.arg(FROUND(m_IdlePowerNoShields, .0001));

				stat += QObject::tr("%1 Terawatts Idling with Shields Up: "
						"Life Support, Maneuver Drive, Sensor Powered On. Shields Up.<br>")
					.arg(FROUND(m_IdlePowerWithShields, .0001));

				stat += QObject::tr("%1 Terawatts Holding: "
						"Life Support, Sensor Powered On. Maneuver Drive Holding. Shields Down.<br>")
					.arg(FROUND(m_HoldPowerNoShields, .0001));

				stat += QObject::tr("%1 Terawatts Holding with Shields Up: "
						"Life Support, Sensor Powered On. Maneuver Drive Holding. Shields Up.")
					.arg(FROUND(m_HoldPowerWithShields, .0001));
			}
			else
			{
				stat += QObject::tr("%1 Terawatts Idling: "
						"Life Support, Maneuver Drive, Sensor Powered On.<br>")
					.arg(FROUND(m_IdlePowerNoShields, .0001));
				stat += QObject::tr("%1 Terawatts Holding: "
						"Life Support, Sensor Powered On. Maneuver Drive Holding.")
					.arg(FROUND(m_HoldPowerNoShields, .0001));
			}
			stat += "</div>";
			stat += "</div>";

			// Capacitor
			if(designState != AuDesign::Final && m_CapacitorMax < m_IdlePowerWithShields*60)
			{
				// Capacitor has less than 60 seconds of power at idle.
				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("%L1m%5 Capacitor"
						"<div style=\"margin-left: 10px;\">"
						"%3 Q%4<br>"
						"Storage %L2 Terajoules.<br>"
						"<span style=\"color: rgb(255, 255, 32);\">"
						"Less Than 60 sec. Capacitor While Idling"
						"</span>"
						"</div>")
					.arg(m_Capacitor.Volume())
					.arg(m_CapacitorMax, 0,'f',2)
					.arg(g_Au.CommodityName(m_Capacitor.Module()))
					.arg(m_Capacitor.Quality())
					.arg(cubedSym);
				stat += "</div>";
			}
			else
			{
				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("%L1m%5 Capacitor"
						"<div style=\"margin-left: 10px;\">"
						"%3 Q%4<br>"
						"Storage %L2 Terajoules."
						"</div>")
					.arg(m_Capacitor.Volume())
					.arg(m_CapacitorMax, 0,'f',2)
					.arg(g_Au.CommodityName(m_Capacitor.Module()))
					.arg(m_Capacitor.Quality())
					.arg(cubedSym);
				stat += "</div>";
			}

			// Power Plant
			const double powerOutput = m_FuelBurnRate * PowerPerFuel();	// power per second output by power plant
			if(designState != AuDesign::Final &&
					m_IdlePowerWithShields > powerOutput &&
					m_PowerPlant.Volume() && m_FuelBurnRate > 0)
			{
				const double powerNeeded = m_IdlePowerWithShields - powerOutput;		// output needed per sec for idle demand
				double needs = powerNeeded / powerOutput;
				double needsVol = (int)ceil(needs * m_PowerPlant.Volume());

				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("%L1m%2 Power Plant"
						"<div style=\"margin-left: 10px;\">"
						"%3 Q%4"
						"<br>"
						"Ouput %L5 Terawatts\n"
						"<br>"
						"<span style=\"color: rgb(255, 32, 32);\">"
						"%L7m%2 NEEDED to Meet Idle Demand%6"
						"</span>"
						"</div>")
					.arg(m_PowerPlant.Volume())
					.arg(cubedSym)
					.arg(g_Au.CommodityName(m_PowerPlant.Module()))
					.arg(m_PowerPlant.Quality())
					.arg(powerOutput, 0,'f',2)
					.arg(m_Shield.Volume() ? " With Shields Up" : "")
					.arg(needsVol, 0,'f',0);
				stat += "</div>";
			}
			else
			{
				// Calculate capacitor charge time.
				if(powerOutput > m_IdlePowerWithShields)
				{
					double excessPower = powerOutput - m_IdlePowerWithShields;
					double fillTime = m_CapacitorMax / excessPower;
					stat += "<div style=\"margin-left: 10px;\">";
					stat += QObject::tr("%L1m%2 Power Plant"
							"<div style=\"margin-left: 10px;\">"
							"%3 Q%4"
							"<br>"
							"Ouput %L5 Terawatts\n"
							"<br>"
							"Capacitor Fill Time %L7 sec While Idling%6"
							"</div>")
						.arg(m_PowerPlant.Volume())
						.arg(cubedSym)
						.arg(g_Au.CommodityName(m_PowerPlant.Module()))
						.arg(m_PowerPlant.Quality())
						.arg(powerOutput, 0,'f',2)
						.arg(m_Shield.Volume() ? " With Shields Up" : "")
						.arg(round(fillTime));
					stat += "</div>";
				}
				else
				{
					stat += "<div style=\"margin-left: 10px;\">";
					stat += QObject::tr("%L1m%2 Power Plant"
							"<div style=\"margin-left: 10px;\">"
							"%3 Q%4"
							"<br>"
							"Ouput %L5 Terawatts\n"
							"<br>"
							"<span style=\"color: rgb(255, 32, 32);\">"
							"Insufficient Power for Idle Demand%6"
							"</span>"
							"</div>")
						.arg(m_PowerPlant.Volume())
						.arg(cubedSym)
						.arg(g_Au.CommodityName(m_PowerPlant.Module()))
						.arg(m_PowerPlant.Quality())
						.arg(powerOutput, 0,'f',2)
						.arg(m_Shield.Volume() ? " With Shields Up" : "");
					stat += "</div>";
				}
			}

			// FTL drives
			quint32 ftlUnitsNeeded = (quint32)ceil(m_FTLDriveUnitsToEngage / CubicFeetPerCubicMeter());	// cubic meters needed
			for(QHash<quint16, AuScReportSysVol>::const_iterator fi = m_FTLDrives.cbegin(); fi != m_FTLDrives.cend(); fi++)
			{
				COMMODITYID ftlDriveMod = (*fi).Module();
				quint8 ftlDriveQ = (*fi).Quality();

				stat += "<div style=\"margin-left: 10px;\">";
				int maxFTLSetting = AuScDesign::MaxCapacitorFTLSetting(ftlDriveMod, ftlDriveQ, m_FTLDriveUnitsToEngage,
						m_CapacitorMax);
				double powerToEngage = PowerNeededToEngageFTLDrive(fi.key(), 0);
				if(DbCommodity::IsWarpFTLDriveModule(ftlDriveMod))
				{
					if((*fi).Volume() < ftlUnitsNeeded)
						stat += QObject::tr("%L1m%5 FTL Drive"
								"<div style=\"margin-left: 10px;\">"
								"%3 Q%4, Part Id %7<br>"
								"<span style=\"color: rgb(255, 32, 32);\">"
								"%L2m%5 NEEDED to Engage<br>"
								"</span>"
								"%L8 Terajoules to Engage at Warp 1"
								"</div>")
							.arg((*fi).Volume())
							.arg(ftlUnitsNeeded)
							.arg(g_Au.CommodityName(ftlDriveMod))
							.arg(ftlDriveQ)
							.arg(cubedSym)
							.arg(fi.key())
							.arg(powerToEngage, 0,'f',2);
					else
					{
						if(maxFTLSetting > 0)
						{
							double powerToEngageMax = PowerNeededToEngageFTLDrive(fi.key(), maxFTLSetting);
							stat += QObject::tr("%L1m%6 FTL Drive"
									"<div style=\"margin-left: 10px;\">"
									"%3 Q%4, Part Id %7<br>"
									"%L2m%6 Sufficient to Engage<br>"
									"Maximum Warp Factor %5<br>"
									"%L8 Terajoules to Engage at Warp 1<br>"
									"%L9 Terajoules to Engage at Warp %5"
									"</div>")
								.arg((*fi).Volume())
								.arg(ftlUnitsNeeded)
								.arg(g_Au.CommodityName(ftlDriveMod))
								.arg(ftlDriveQ)
								.arg(maxFTLSetting+1)
								.arg(cubedSym)
								.arg(fi.key())
								.arg(powerToEngage, 0,'f',2)
								.arg(powerToEngageMax, 0,'f',2);
						}
						else
						{
							stat += QObject::tr("%L1m%6 FTL Drive"
									"<div style=\"margin-left: 10px;\">"
									"%3 Q%4, Part Id %7<br>"
									"%L2m%6 Sufficient to Engage<br>"
									"Maximum Warp Factor %5<br>"
									"%L8 Terajoules to Engage at Warp 1"
									"</div>")
								.arg((*fi).Volume())
								.arg(ftlUnitsNeeded)
								.arg(g_Au.CommodityName(ftlDriveMod))
								.arg(ftlDriveQ)
								.arg(maxFTLSetting+1)
								.arg(cubedSym)
								.arg(fi.key())
								.arg(powerToEngage, 0,'f',2);
						}
					}
				}
				else
				{
					if((*fi).Volume() < ftlUnitsNeeded)
						stat += QObject::tr("%L1m%6 FTL Drive"
								"<div style=\"margin-left: 10px;\">"
								"%3 Q%4, Part Id %7<br>"
								"<span style=\"color: rgb(255, 32, 32);\">"
								"%L2m%6 NEEDED to Engage<br>"
								"</span>"
								"%L8 Terajoules to Engage"
								"</div>")
							.arg((*fi).Volume())
							.arg(ftlUnitsNeeded)
							.arg(g_Au.CommodityName(ftlDriveMod))
							.arg(ftlDriveQ)
							.arg(cubedSym)
							.arg(fi.key())
							.arg(powerToEngage, 0,'f',2);
					else
					{
						QString polarities = QObject::tr("Positive");
						if(maxFTLSetting >= 1)
							polarities += QObject::tr(", Negative");
						if(maxFTLSetting >= 2)
							polarities += QObject::tr(", Neutral");
						stat += QObject::tr("%L1m%6 FTL Drive"
								"<div style=\"margin-left: 10px;\">"
								"%3 Q%4, Part Id %7<br>"
								"%L2m%6 Sufficient to Engage<br>"
								"Polarities: %5<br>"
								"%L8 Terajoules to Engage"
								"</div>")
							.arg((*fi).Volume())
							.arg(ftlUnitsNeeded)
							.arg(g_Au.CommodityName(ftlDriveMod))
							.arg(ftlDriveQ)
							.arg(polarities)
							.arg(cubedSym)
							.arg(fi.key())
							.arg(powerToEngage, 0,'f',2);
					}
				}

				if(maxFTLSetting < 0)
					stat += QObject::tr("<div style=\"margin-left: 10px;\">"
							"<span style=\"color: rgb(255, 32, 32);\">"
							"INADEQUATE CAPACITOR to Engage"
							"</span>"
							"</div>");

				stat += "</div>";
			}
		}

		// Fuel Endurance
		stat += QObject::tr("<hr><b>FUEL</b>");
		if(m_FuelVolume)
		{
			// Show total fuel capacity in fuel units, which happen to be cubic feet.
			stat += "<div style=\"margin-left: 10px;\">";
			stat += QObject::tr("%L1m%2 Fuel Cell<br>%L3 Units of Hydrogen (H%4)")
				.arg(m_FuelVolume)
				.arg(cubedSym)
				.arg(m_FuelCapacity)
				.arg(squaredSym);
			stat += "<div style=\"margin-left: 10px;\">";

			double idlePower = m_IdlePowerNoShields * (24. * 3600.);	// power per day
			double idleFuel = m_ManeuverFuelRate * (24. * 3600.);			// rocket fuel per day, in cubic feet
			double idleBurnRate = idlePower / PowerPerFuel() + idleFuel;	// total fuel per day, in cubic feet
			double rrate = FROUND(idleBurnRate, 0.01);
			if(rrate < 0)
				idleBurnRate = ceil(idleBurnRate);
			else
				idleBurnRate = rrate;

			double idleDays;
			if(idleBurnRate > 0)
				idleDays = FROUND((double)m_FuelCapacity/idleBurnRate, 0.1);
			else
				idleDays = 0;

			stat += QObject::tr("%L2 days at %L1 Units H%3 per Real Day Idling")
				.arg(idleBurnRate, 0,'f',2)
				.arg(idleDays, 0,'f',1)
				.arg(squaredSym);
			stat += "</div>";

			if(m_Shield.Volume())
			{
				double idlePower = m_IdlePowerWithShields * (24. * 3600.);		// per day
				double idleBurnRate = idlePower / PowerPerFuel();	// fuel per day
				rrate = FROUND(idleBurnRate, 0.01);
				if(rrate < 0)
					idleBurnRate = ceil(idleBurnRate);
				else
					idleBurnRate = rrate;
				double idleDays;
				if(idleBurnRate > 0)
					idleDays = FROUND((double)m_FuelCapacity/idleBurnRate, 0.1);
				else
					idleDays = 0;

				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("%L2 days at %L1 Units H%3 per Real Day Idling with Shields Up")
					.arg(idleBurnRate, 0,'f',2)
					.arg(idleDays, 0,'f',1)
					.arg(squaredSym);
				stat += "</div>";
			}

			{
				// Gravity drive module reduces power usage when parked.
				double parkPower = m_HoldPowerNoShields * (24. * 3600.);	// per day
				double parkBurnRate = parkPower / PowerPerFuel();			// fuel per day
				parkBurnRate = FROUND(parkBurnRate, 0.01);
				double parkDays;
				if(parkBurnRate > 0)
					parkDays = FROUND((double)m_FuelCapacity/parkBurnRate, 0.1);
				else
					parkDays = 0;

				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("%L2 days at %L1 Units H%3 per Real Day Holding")
					.arg(parkBurnRate, 0,'f',2)
					.arg(parkDays, 0,'f',1)
					.arg(squaredSym);
				stat += "</div>";

				if(m_Shield.Volume())
				{
					double parkPower = m_HoldPowerWithShields * (24. * 3600.);	// per day
					double parkBurnRate = parkPower / PowerPerFuel();			// fuel per day
					parkBurnRate = FROUND(parkBurnRate, 0.01);
					double parkDays;
					if(parkBurnRate > 0)
						parkDays = FROUND((double)m_FuelCapacity/parkBurnRate, 0.1);
					else
						parkDays = 0;

					stat += "<div style=\"margin-left: 10px;\">";
					stat += QObject::tr("%L2 days at %L1 Units H%3 per Real Day Holding with Shields Up")
						.arg(parkBurnRate, 0,'f',2)
						.arg(parkDays, 0,'f',1)
						.arg(squaredSym);
					stat += "</div>";
				}

				stat += "</div>";
			}
		}
		else
		{
			stat += "<div style=\"margin-left: 10px;\">";
			stat += QString("<span style=\"color: rgb(255, 32, 32);\">");
			stat += QObject::tr("NO FUEL");
			stat += QString("</span>");
			stat += "</div>";
		}

		// Avionics Equipment
		if(!m_CommandStations.empty() ||
				!m_CrewStations.empty() ||
				!m_DesignerStations.empty() ||
				!m_ServicePanels.empty() ||
				!m_StatusDisplays.empty() ||
				m_LifeSupport.Volume() ||
				m_Sensor.Volume() ||
				m_Shield.Volume() ||
				!m_GameTables.empty() ||
				!m_Transporters.empty())
		{
			stat += QObject::tr("<hr><b>AVIONICS</b>");

			// Control Stations
			if(!m_CommandStations.isEmpty() || !m_CrewStations.isEmpty() || !m_Transporters.empty() ||
					!m_DesignerStations.isEmpty())
			{
				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("%1 Q%2").arg(g_Au.CommodityName((COMMODITYID)m_CnCModule)).arg(m_CnCQ);

				// List stations so they are alphabetized.
				QMultiMap<QString, quint16> stations;	// key is name, value is part id.
				for(AuScReportStationHash::const_iterator ci = m_CommandStations.cbegin(); ci != m_CommandStations.cend(); ci++)
				{
					QString station = QObject::tr("%1, Part Id %2")
						.arg((*ci).Desc())
						.arg(ci.key());
					stations.insert(station, ci.key());
				}
				for(AuScReportStationHash::const_iterator ci = m_CrewStations.cbegin(); ci != m_CrewStations.cend(); ci++)
				{
					QString station = QObject::tr("%1, Part Id %2")
						.arg((*ci).Desc())
						.arg(ci.key());
					stations.insert(station, ci.key());
				}
				for(AuScReportTransporterStationHash::const_iterator ci = m_Transporters.cbegin();
						ci != m_Transporters.cend(); ci++)
				{
					QString station = QObject::tr("%1, %3 Pads, Part Id %2")
						.arg((*ci).Desc())
						.arg(ci.key())
						.arg((*ci).Pads());
					stations.insert(station, ci.key());
				}
				for(AuScReportStationHash::const_iterator ci = m_DesignerStations.cbegin(); ci != m_DesignerStations.cend(); ci++)
				{
					QString station = QObject::tr("%1, Part Id %2")
						.arg((*ci).Desc())
						.arg(ci.key());
					stations.insert(station, ci.key());
				}

				stat += "<div style=\"margin-left: 10px;\">";
				if(Structure() == SC_Station)
				{
					// Trade range is determined by quality of command and control system.
					stat += QObject::tr("Station Service Range %4m<br>").arg(TradeRangeM(m_CnCQ));
				}
				for(QMultiMap<QString, quint16>::const_iterator si = stations.begin(); si != stations.end(); si++)
				{
					if(si != stations.begin())
						stat += "<br>";
					stat += si.key();
				}
				stat += "</div>";
				stat += "</div>";
			}

			// Service Panels
			if(!m_ServicePanels.empty())
			{
				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("Service Panels");
				// List status displays so they are alphabetized.
				QMultiMap<QString, quint16> stations;	// key is name, value is part id.
				for(AuScReportServicePanelHash::const_iterator ci = m_ServicePanels.cbegin(); ci != m_ServicePanels.cend(); ci++)
				{
					QString station = QObject::tr("%1, Part Id %2")
						.arg((*ci).Desc())
						.arg(ci.key());
					stations.insert(station, ci.key());
				}

				stat += "<div style=\"margin-left: 10px;\">";
				for(QMultiMap<QString, quint16>::const_iterator si = stations.begin(); si != stations.end(); si++)
				{
					if(si != stations.begin())
						stat += "<br>";
					stat += si.key();
				}
				stat += "</div>";
				stat += "</div>";
			}

			// Status Displays
			if(!m_StatusDisplays.empty())
			{
				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("Status Displays");
				// List status displays so they are alphabetized.
				QMultiMap<QString, quint16> stations;	// key is name, value is part id.
				for(AuScReportStatusDisplayHash::const_iterator ci = m_StatusDisplays.cbegin(); ci != m_StatusDisplays.cend(); ci++)
				{
					if((*ci).IsDecal())
						continue;
					QString station = QObject::tr("%1, Part Id %2")
						.arg((*ci).Desc())
						.arg(ci.key());
					stations.insert(station, ci.key());
				}

				stat += "<div style=\"margin-left: 10px;\">";
				for(QMultiMap<QString, quint16>::const_iterator si = stations.begin(); si != stations.end(); si++)
				{
					if(si != stations.begin())
						stat += "<br>";
					stat += si.key();
				}
				stat += "</div>";
				stat += "</div>";
			}

			if(!m_GameTables.empty())
			{
				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("%L1 Game Tables").arg(m_GameTables.size());
				stat += "</div>";
			}
			if(m_LifeSupport.Volume())
			{
				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("%L1m%2 Life Support").arg(m_LifeSupport.Volume()).arg(cubedSym);
				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("%1 Q%2").arg(g_Au.CommodityName(m_LifeSupport.Module())).arg(m_LifeSupport.Quality());
				stat += "</div>";
				stat += "</div>";
			}
			if(m_Sensor.Volume())
			{
				QString desc;
				if(m_SensorRange > FeetPerOrbit()*15)
					desc = QObject::tr("System Range");
				else if(m_SensorRange > FeetPerOrbit()/2)
					desc = QObject::tr("Planetary Range");
				else if(m_SensorRange > FeetPerMoonOrbit())
					desc = QObject::tr("Orbital Range");
				else
					desc = "Short Range";
				double meters = ceil(m_SensorRange / FeetPerMeter());
				double deepSpacePc = SensorRangeDeepSpace();

				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("%L1m%2 Sensor").arg(m_Sensor.Volume()).arg(cubedSym);
				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("%1 Q%2<br>"
						"%3 %L4m Coverage Radius<br>"
						"%L5pc Warp Signature Detection Range if Equipped with Warp FTL Drive")
					.arg(g_Au.CommodityName(m_Sensor.Module()))
					.arg(m_Sensor.Quality())
					.arg(desc)
					.arg(meters, 0,'f',0)
					.arg(deepSpacePc, 0,'f',1);
				stat += "</div>";
				stat += "</div>";
			}
			if(m_Shield.Volume())
			{
				float absorbShield = ShieldCapacitor() / ShieldDissipatePerDamagePoint();
				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("%L1m%2 Shield").arg(m_Shield.Volume()).arg(cubedSym);
				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("%1 Q%2<br>"
						"Absorbs %L3hp per Side: Above, Below, Port, Starboard, Fore, Aft")
					.arg(g_Au.CommodityName(m_Shield.Module()))
					.arg(m_Shield.Quality())
					.arg(round(absorbShield), 0,'f',0);
				stat += "</div>";
				stat += "</div>";
			}
		}

		// Weaponry
		if(!m_Turrets.empty() || !m_Bays.empty())
		{
			stat += QObject::tr("<hr><b>ARMAMENT</b>");
			for(QHash<quint16, AuScReportSysVol>::const_iterator bi = m_Bays.begin(); bi != m_Bays.end(); bi++)
			{
				COMMODITYID bayMod = (*bi).Module();
				quint32 wpns = (*bi).Volume();
				double wpnsCuFt = wpns * CubicFeetPerCubicMeter();
				quint8 bayQ = (*bi).Quality();

				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("Bay %1, %L2m%3").arg(bi.key()).arg(wpns).arg(cubedSym);

				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("%4 Q%5").arg(g_Au.CommodityName(bayMod)).arg(bayQ);
				if(wpns)
				{
					unsigned int range = (unsigned int)(AuScDesign::BayWeaponRange(bayQ)/FeetPerMeter());

					switch(bayMod)
					{
					case CID_BallisticWeaponBayModule:
						{
							int cannons = AuScDesign::BayWeaponCannons(wpnsCuFt);
							stat += QObject::tr("<br>%L1 Cannon Barrels").arg(cannons);
							stat += QObject::tr("<br>Targeting Range %L1m").arg(range);

							// Ammo Example
							AuItemAmmo ammo(CID_HeavyArmsAmmoPi, bayQ, 0);
							stat += QObject::tr("<br>Example Firing %1")
								.arg(ammo.ListDesc());
							stat += "<div style=\"margin-left: 10px;\">";
								AuCombatWeapon ammoWpn(&ammo, 1);
								if(ammo.Radius1() > 0)
									stat += QObject::tr("One Round Damage %L1-%L2hp, %L3m Radius")
										.arg(1 + ammoWpn.m_w1.m_DamageMod)
										.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod)
										.arg(round(ammoWpn.m_w1.m_Radius/FeetPerMeter()));
								else
									stat += QObject::tr("One Round Damage %L1-%L2hp")
										.arg(1 + ammoWpn.m_w1.m_DamageMod)
										.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod);

								ammoWpn = AuCombatWeapon(&ammo, cannons);
								if(ammo.Radius1() > 0)
									stat += QObject::tr("<br>Full Barrage of %L4 Damage %L1-%L2hp, %L3m Radius")
										.arg(1 + ammoWpn.m_w1.m_DamageMod)
										.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod)
										.arg(round(ammoWpn.m_w1.m_Radius/FeetPerMeter()))
										.arg(cannons);
								else
									stat += QObject::tr("<br>Full Barrage of %L4 Damage %L1-%L2hp")
										.arg(1 + ammoWpn.m_w1.m_DamageMod)
										.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod)
										.arg(cannons);
							stat += "</div>";
						}
						break;

					case CID_EnergyWeaponBayModule:
						{
							int projectors = AuScDesign::BayWeaponProjectors(wpnsCuFt);
							stat += QObject::tr("<br>%L1 Beam Projectors").arg(projectors);
							stat += QObject::tr("<br>Targeting Range %L1m").arg(range);

							// Energy Weapons
							unsigned int shots = (unsigned int)(m_CapacitorMax / projectors);	// truncated result, in shots
							stat += "<div style=\"margin-left: 10px;\">";
							if(shots == 1)
								stat += QObject::tr("Capacitor Adequate for One Shot of Energy Weapon at Full Power");
							else
								stat +=
									QObject::tr("Capacitor Adequate for %L1 Consecutive Shots of Energy Weapons at Full Power")
									.arg(shots);
							stat += "</div>";
							stat += "</div>";

							// Ammo Example
							stat += "<div style=\"margin-left: 10px;\">";
							AuItemAmmo ammo(CID_ElectronBeam, bayQ, 0);
							stat += QObject::tr("Example Firing %1")
								.arg(ammo.ListDesc());
							stat += "<div style=\"margin-left: 10px;\">";
								AuCombatWeapon ammoWpn(&ammo, 1);
								if(ammo.Radius1() > 0)
									stat += QObject::tr("One Beam Damage %L1-%L2hp, %L3m Radius")
										.arg(1 + ammoWpn.m_w1.m_DamageMod)
										.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod)
										.arg(round(ammoWpn.m_w1.m_Radius/FeetPerMeter()));
								else
									stat += QObject::tr("One Beam Damage %L1-%L2hp")
										.arg(1 + ammoWpn.m_w1.m_DamageMod)
										.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod);

								ammoWpn = AuCombatWeapon(&ammo, projectors);
								if(ammo.Radius1() > 0)
									stat += QObject::tr("<br>Full Bundle of %L4 Damage %L1-%L2hp, %L3m Radius")
										.arg(1 + ammoWpn.m_w1.m_DamageMod)
										.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod)
										.arg(round(ammoWpn.m_w1.m_Radius/FeetPerMeter()))
										.arg(projectors);
								else
									stat += QObject::tr("<br>Full Bundle of %L4 Damage %L1-%L2hp")
										.arg(1 + ammoWpn.m_w1.m_DamageMod)
										.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod)
										.arg(projectors);
							stat += "</div>";
						}
						break;

					case CID_WeaponDeploymentBayModule:
						{
							int racks = AuScDesign::BayWeaponEjectorRacks(wpnsCuFt);
							stat += QObject::tr("<br>%L1 Ejector Racks").arg(racks);
							stat += QObject::tr("<br>Targeting Range %L1m").arg(range);

							// Ammo Example
							AuItemWeapon ammo(CID_BombBl, bayQ, 0);
							stat += QObject::tr("<br>Example Dropping %1")
								.arg(ammo.ListDesc());
							stat += "<div style=\"margin-left: 10px;\">";
								AuCombatWeapon ammoWpn(&ammo, 1);
								if(ammoWpn.m_w1.m_Radius > 0)
									stat += QObject::tr("One Bomb Damage %L1-%L2hp, %L3m Radius")
										.arg(1 + ammoWpn.m_w1.m_DamageMod)
										.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod)
										.arg(round(ammoWpn.m_w1.m_Radius/FeetPerMeter()));
								else
									stat += QObject::tr("One Bomb Damage %L1-%L2hp")
										.arg(1 + ammoWpn.m_w1.m_DamageMod)
										.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod);

								ammoWpn = AuCombatWeapon(&ammo, racks);
								if(ammoWpn.m_w1.m_Radius > 0)
									stat += QObject::tr("<br>Full Load of %L4 Damage %L1-%L2hp, %L3m Radius")
										.arg(1 + ammoWpn.m_w1.m_DamageMod)
										.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod)
										.arg(round(ammoWpn.m_w1.m_Radius/FeetPerMeter()))
										.arg(racks);
								else
									stat += QObject::tr("<br>Full Load of %L4 Damage %L1-%L2hp")
										.arg(1 + ammoWpn.m_w1.m_DamageMod)
										.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod)
										.arg(racks);
							stat += "</div>";
						}
						break;

					case CID_WeaponLauncherBayModule:
						{
							int launchers = AuScDesign::BayWeaponLaunchers(wpnsCuFt);
							stat += QObject::tr("<br>%L1 Launchers").arg(launchers);
							stat += QObject::tr("<br>Targeting Range %L1m").arg(range);

							// Rocket Ammo Example
							AuItemAmmo ammo(CID_LargeRocketBl, bayQ, 0);
							stat += QObject::tr("<br>Example Launching %1")
								.arg(ammo.ListDesc());
							stat += "<div style=\"margin-left: 10px;\">";
								AuCombatWeapon ammoWpn(&ammo, 1);
								if(ammo.Radius1() > 0)
									stat += QObject::tr("One Rocket Damage %L1-%L2hp, %L3m Radius")
										.arg(1 + ammoWpn.m_w1.m_DamageMod)
										.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod)
										.arg(round(ammoWpn.m_w1.m_Radius/FeetPerMeter()));
								else
									stat += QObject::tr("One Rocket Damage %L1-%L2hp")
										.arg(1 + ammoWpn.m_w1.m_DamageMod)
										.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod);

								ammoWpn = AuCombatWeapon(&ammo, launchers);
								if(ammo.Radius1() > 0)
									stat += QObject::tr("<br>Full Volley of %L4 Damage %L1-%L2hp, %L3m Radius")
										.arg(1 + ammoWpn.m_w1.m_DamageMod)
										.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod)
										.arg(round(ammoWpn.m_w1.m_Radius/FeetPerMeter()))
										.arg(launchers);
								else
									stat += QObject::tr("<br>Full Volley of %L4 Damage %L1-%L2hp")
										.arg(1 + ammoWpn.m_w1.m_DamageMod)
										.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod)
										.arg(launchers);
							stat += "</div>";
							stat += "</div>";

							stat += "<div style=\"margin-left: 10px;\">";

							// Missile Ammo Example
							ammo = AuItemAmmo(CID_LargeMissileBl, bayQ, 0);
							stat += QObject::tr("Example Launching %1")
								.arg(ammo.ListDesc());
							stat += "<div style=\"margin-left: 10px;\">";
								ammoWpn = AuCombatWeapon(&ammo, 1);
								if(ammo.Radius1() > 0)
									stat += QObject::tr("One Missile Damage %L1-%L2hp, %L3m Radius")
										.arg(1 + ammoWpn.m_w1.m_DamageMod)
										.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod)
										.arg(round(ammoWpn.m_w1.m_Radius/FeetPerMeter()));
								else
									stat += QObject::tr("One Missile Damage %L1-%L2hp")
										.arg(1 + ammoWpn.m_w1.m_DamageMod)
										.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod);

								ammoWpn = AuCombatWeapon(&ammo, launchers);
								if(ammo.Radius1() > 0)
									stat += QObject::tr("<br>Full Volley of %L4 Damage %L1-%L2hp, %L3m Radius")
										.arg(1 + ammoWpn.m_w1.m_DamageMod)
										.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod)
										.arg(round(ammoWpn.m_w1.m_Radius/FeetPerMeter()))
										.arg(launchers);
								else
									stat += QObject::tr("<br>Full Volley of %L4 Damage %L1-%L2hp")
										.arg(1 + ammoWpn.m_w1.m_DamageMod)
										.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod)
										.arg(launchers);
							stat += "</div>";
						}
						break;

					case CID_AtmosphereCondenserBayModule:
					case CID_ParticleCollectorBayModule:
					case CID_ProspectorBayModule:
					case CID_SiphonBayModule:
					case CID_WildcatterBayModule:
						{
							// Calculate number of consecutive shots that can be fired at full power.
							double wpnUnits = wpnsCuFt / 256.;
							unsigned int shots = (unsigned int)(m_CapacitorMax / wpnUnits);	// truncated result, in shots

							// Calculate weapon power at 50% range.
							unsigned int range = (unsigned int)(AuScDesign::HarvesterBayRange(wpnsCuFt, bayQ)/FeetPerMeter());
							double wpnPower = AuScDesign::BayWeaponPower(wpnsCuFt, bayQ);
							wpnPower = round(wpnPower * .5f);	// 50% range

							// Harvest up to 1 unit for every fixed quantity of energy that hits target.
							int harvestUnits = (int)(wpnPower / AuScDesign::HarvestUnitsPerEnergyUnit());

							stat += QObject::tr("<br>Range %L1m<br>Harvest %2u at %L3m").arg(range).arg(harvestUnits).arg(range/2);
							if(shots == 1)
								stat += QObject::tr("<br>Capacitor Adequate for One Shot at Full Power");
							else
								stat += QObject::tr("<br>Capacitor Adequate for %L1 Consecutive Shots at Full Power").arg(shots);
						}
						break;

					case CID_TractorBeamBayModule:
						{
							// Calculate tractor beam range.
							quint32 tractorRange = (quint32)(AuScDesign::TractorBeamRange(wpnsCuFt, bayQ)/FeetPerMeter());
							tractorRange = round(tractorRange);

							const double tlBoost = QualityBoostF(bayQ);
							double tractorAccel = wpnsCuFt * AuScDesign::ManeuverDriveForce() / mass * tlBoost;
							tractorAccel /= FeetPerMeter();
							tractorAccel = round(tractorAccel);

							stat += QObject::tr("<br>Range %L1m<br>Force %L2m/sec%3 vs Ship of Equal Mass")
								.arg(tractorRange)
								.arg(tractorAccel)
								.arg(squaredSym);
						}
						break;

					default:
						break;
					}
				}
				stat += "</div>";
				stat += "</div>";
			}

			if(!m_Turrets.empty())
			{
				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("%L1 Turrets").arg(m_Turrets.size());
				stat += "</div>";

				// List turret stations.
				for(QHash<quint16, AuScReportTurret>::const_iterator ti = m_Turrets.cbegin(); ti != m_Turrets.cend(); ti++)
				{
					COMMODITYID turretMod = (*ti).Module();
					quint8 turretQ = (*ti).Quality();

					stat += "<div style=\"margin-left: 20px;\">";
					stat += QObject::tr("Turret - %1")
						.arg(ti.key());

					switch(turretMod)
					{
					case CID_BallisticWeaponTurretModule:
						{
							stat += "<div style=\"margin-left: 10px;\">";
								stat += QObject::tr("%1 Q%2")
									.arg(g_Au.CommodityName(turretMod))
									.arg(turretQ);

								// Ammo Example
								AuItemAmmo ammo(CID_HeavyArmsAmmoPi, turretQ, 0);
								stat += QObject::tr("<br>Example Firing %1")
									.arg(ammo.ListDesc());
								stat += "<div style=\"margin-left: 10px;\">";
									AuCombatWeapon ammoWpn(&ammo, 1);
									if(ammo.Radius1() > 0)
										stat += QObject::tr("Damage %L1-%L2hp, %L3m Radius")
											.arg(1 + ammoWpn.m_w1.m_DamageMod)
											.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod)
											.arg(round(ammoWpn.m_w1.m_Radius/FeetPerMeter()));
									else
										stat += QObject::tr("Damage %L1-%L2hp")
											.arg(1 + ammoWpn.m_w1.m_DamageMod)
											.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod);
								stat += "</div>";

							stat += "</div>";
						}
						break;

					case CID_EnergyWeaponTurretModule:
						{
							stat += "<div style=\"margin-left: 10px;\">";
								stat += QObject::tr("%1 Q%2")
									.arg(g_Au.CommodityName(turretMod))
									.arg(turretQ);

								// Ammo Example
								AuItemAmmo ammo(CID_ElectronBeam, turretQ, 0);
								stat += QObject::tr("<br>Example Firing %1")
									.arg(ammo.ListDesc());
								stat += "<div style=\"margin-left: 10px;\">";
									AuCombatWeapon ammoWpn(&ammo, 1);
									if(ammo.Radius1() > 0)
										stat += QObject::tr("Damage %L1-%L2hp, %L3m Radius")
											.arg(1 + ammoWpn.m_w1.m_DamageMod)
											.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod)
											.arg(round(ammoWpn.m_w1.m_Radius/FeetPerMeter()));
									else
										stat += QObject::tr("Damage %L1-%L2hp")
											.arg(1 + ammoWpn.m_w1.m_DamageMod)
											.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod);
								stat += "</div>";

							stat += "</div>";
						}
						break;

					case CID_WeaponLauncherTurretModule:
						{
							stat += "<div style=\"margin-left: 10px;\">";
								stat += QObject::tr("%1 Q%2")
									.arg(g_Au.CommodityName(turretMod))
									.arg(turretQ);

								// Ammo Example
								AuItemAmmo ammo(CID_SmallRocketBl, turretQ, 0);
								stat += QObject::tr("<br>Example Launching %1")
									.arg(ammo.ListDesc());
								stat += "<div style=\"margin-left: 10px;\">";
									AuCombatWeapon ammoWpn(&ammo, 1);
									if(ammo.Radius1() > 0)
										stat += QObject::tr("Damage %L1-%L2hp, %L3m Radius")
											.arg(1 + ammoWpn.m_w1.m_DamageMod)
											.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod)
											.arg(round(ammoWpn.m_w1.m_Radius/FeetPerMeter()));
									else
										stat += QObject::tr("Damage %L1-%L2hp")
											.arg(1 + ammoWpn.m_w1.m_DamageMod)
											.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod);
								stat += "</div>";
								stat += "</div>";
								stat += "<div style=\"margin-left: 10px;\">";

								ammo = AuItemAmmo(CID_SmallMissileBl, turretQ, 0);
								stat += QObject::tr("Example Launching %1")
									.arg(ammo.ListDesc());
								stat += "<div style=\"margin-left: 10px;\">";
									ammoWpn = AuCombatWeapon(&ammo, 1);
									if(ammo.Radius1() > 0)
										stat += QObject::tr("Damage %L1-%L2hp, %L3m Radius")
											.arg(1 + ammoWpn.m_w1.m_DamageMod)
											.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod)
											.arg(round(ammoWpn.m_w1.m_Radius/FeetPerMeter()));
									else
										stat += QObject::tr("Damage %L1-%L2hp")
											.arg(1 + ammoWpn.m_w1.m_DamageMod)
											.arg(ammoWpn.m_w1.m_Damage + ammoWpn.m_w1.m_DamageMod);
								stat += "</div>";

							stat += "</div>";
						}
						break;

					default:
						break;
					}

					stat += "</div>";
				}
			}
		}

		// Crew
		if(!m_CrewStations.empty() || m_ParkGround || m_ParkRocket || m_ParkSpace || m_ParkWater || !m_Turrets.empty() ||
				!m_GuardPosts.empty() || !m_Transporters.empty())
		{
			stat += QObject::tr("<hr><b>CREW</b>");
			stat += "<div style=\"margin-left: 10px;\">";
			if(CrewStationCount())
				stat += QObject::tr("%L1 Crew Stations<br>").arg(CrewStationCount());
			if(m_ParkGround)
				stat += QObject::tr("%L1 Drivers<br>").arg(m_ParkGround);
			if(m_ParkSpace + m_ParkRocket)
				stat += QObject::tr("%L1 Pilots<br>").arg(m_ParkSpace + m_ParkRocket);
			if(m_ParkWater)
				stat += QObject::tr("%L1 Boatmen<br>").arg(m_ParkWater);
			if(!m_Turrets.empty())
				stat += QObject::tr("%L1 Turret Gunners<br>").arg(m_Turrets.size());
			if(!m_GuardPosts.empty())
				stat += QObject::tr("%L1 Guards<br>").arg(m_GuardPosts.size());
			stat = stat.left(stat.length()-4);	// Drop the last <br>
			stat += "</div>";
		}

		// Quarters
		if(m_CaptBerths || m_OfficerBerths || m_CrewBerths || m_PassengerBerths || m_TroopBerths || m_CryoBerthVolume)
		{
			stat += QObject::tr("<hr><b>QUARTERS</b>");
			stat += "<div style=\"margin-left: 10px;\">";
			if(m_CaptBerths)
				stat += QObject::tr("%L1 Capt Berths<br>").arg(m_CaptBerths);

			if(m_OfficerBerths)
				stat += QObject::tr("%L1 Officer Berths<br>").arg(m_OfficerBerths);

			if(m_CrewBerths)
				stat += QObject::tr("%L1 Crew Berths<br>").arg(m_CrewBerths);

			if(m_TroopBerths)
				stat += QObject::tr("%L1 Troop Berths<br>").arg(m_TroopBerths);

			if(m_PassengerBerths)
			{
				if(IsFirstClass())
					stat += QObject::tr("%L1 1st Class Passenger Berths<br>").arg(m_PassengerBerths);
				else if(IsSecondClass())
					stat += QObject::tr("%L1 2nd Class Passenger Berths<br>").arg(m_PassengerBerths);
				else
					stat += QObject::tr("%L1 3rd Class Passenger Berths<br>").arg(m_PassengerBerths);
			}

			if(m_CryoBerthVolume)
				stat += QObject::tr("%L1 Cryo Berths<br>").arg(CryoBerths());

			stat = stat.left(stat.length()-4);	// Drop the last <br>

			stat += "</div>";
		}

		// Hold
		if(m_HoldVolume)
		{
			stat += QObject::tr("<hr><b>HOLD</b>");
			stat += "<div style=\"margin-left: 10px;\">";
			stat += QObject::tr("%L1m%2 Storage Space").arg(m_HoldVolume).arg(cubedSym);
			stat += QObject::tr("<br>Room for %L1 Commodities").arg(m_HoldCapacity);

			int teus = m_HoldCapacity / 1360;	// Drop fraction. Whole TEUs that will fit into the cargo bay.
			stat += QObject::tr("<br>Room for %L1 TEUs").arg(teus);
			stat += "</div>";
		}

		// Vehicle Bays
		if(m_ParkGround || m_ParkSpace || m_ParkRocket  || m_ParkWater || !m_VehicleLaunchers.empty() ||
				!m_VehicleLandingSystems.empty() || m_ParkSpacecraft)
		{
			stat += QObject::tr("<hr><b>VEHICLE FACILITIES</b>");
			stat += "<div style=\"margin-left: 10px;\">";

			if(!m_VehicleLaunchers.empty())
				stat += QObject::tr("<img height=\"32\" src=\"qrc%1\">%L2 Space Vehicle Launchers<br>")
					.arg(":data/ScLaunchSpaceVehicle.png")
					.arg(m_VehicleLaunchers.size());

			if(!m_VehicleLandingSystems.empty())
				stat += QObject::tr("<img height=\"32\" src=\"qrc%1\">%L2 Space Vehicle Recovery Systems<br>")
					.arg(":data/ScLandingSystem.png")
					.arg(m_VehicleLandingSystems.size());

			if(m_ParkGround)
				stat += QObject::tr("<img height=\"32\" src=\"qrc%1\">%L2 Ground Vehicle Parking Spots<br>")
					.arg(":data/ScParkGroundVehicle.png")
					.arg(m_ParkGround);
			if(m_ParkRocket)
				stat += QObject::tr("<img height=\"32\" src=\"qrc%1\">%L2 Space Rocket Parking Spots<br>")
					.arg(":data/ScParkRocket.png")
					.arg(m_ParkRocket);
			if(m_ParkSpace)
				stat += QObject::tr("<img height=\"32\" src=\"qrc%1\">%L2 Space Vehicle Parking Spots<br>")
					.arg(":data/ScParkSpaceVehicle.png")
					.arg(m_ParkSpace);
			if(m_ParkWater)
				stat += QObject::tr("<img height=\"32\" src=\"qrc%1\">%L2 Water Vehicle Parking Spots<br>")
					.arg(":data/ScParkWaterVehicle.png")
					.arg(m_ParkWater);
			if(m_ParkSpacecraft)
				stat += QObject::tr("<img height=\"32\" src=\"qrc%1\">%L2 Spacecraft Parking Spots<br>")
					.arg(":data/ScParkSpacecraft.png")
					.arg(m_ParkSpacecraft);

			stat = stat.left(stat.length()-4);	// Drop the last <br>

			stat += "</div>";
		}

		// Hit points by system.
		if(TotalHits())
		{
			// NOTE: Ship hit points are multiplied by 10 when reported. This is due to the fact that 10 hits of damage only
			//       causes one unit of damage to the ship. Each unit of ship damage is represented by a commodity that needs
			//       to be replaced. It takes 10 points of damage inflicted to cause one unit of ship damage.
			stat += QObject::tr("<hr><b>DAMAGE CONTROL</b>");
			stat += "<div style=\"margin-left: 10px;\">";
			stat += QObject::tr("%L1 Total Hit Points").arg(TotalHits());
			quint32 structure = StructureHits();
			if(structure)
			{
				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("%L1 Structure Hit Points").arg(structure);
				quint32 fuel = FuelHits();
				quint32 hold = HoldHits();
				quint32 skeleton = SkeletonHits();
				quint32 hull = HullSystemHits();
				stat += "<div style=\"margin-left: 10px;\">";
				if(skeleton)
					stat += QObject::tr("%L1 Framework<br>").arg(skeleton);
				if(fuel)
					stat += QObject::tr("%L1 Fuel Cell<br>").arg(fuel);
				if(hold)
					stat += QObject::tr("%L1 Hold<br>").arg(hold);
				if(hull)
					stat += QObject::tr("%L1 Hull<br>").arg(hull);
				stat = stat.left(stat.length()-4);	// Drop the last <br>
				stat += "</div>";
				stat += "</div>";
			}

			int systems = NonStructureHits();
			if(systems)
			{
				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("%L1 Equipment Hit Points").arg(systems);
				stat += "<div style=\"margin-left: 10px;\">";
				quint32 cnc = CommandStationCount() + CrewStationCount();
				if(CapacitorHits())
					stat += QObject::tr("%L1 Capacitor<br>").arg(CapacitorHits());
				if(cnc)
					stat += QObject::tr("%L1 Command and Control<br>").arg(cnc);
				if(FTLDriveHits())
					stat += QObject::tr("%L1 FTL Drive<br>").arg(FTLDriveHits());
				if(LifeSupportHits())
					stat += QObject::tr("%L1 Life Support<br>").arg(LifeSupportHits());
				if(ManeuverDriveHits())
					stat += QObject::tr("%L1 Maneuver Drive<br>").arg(ManeuverDriveHits());
				if(SurgeryUnitCount())
					stat += QObject::tr("%L1 Medical<br>").arg(SurgeryUnitCount());
				if(PowerPlantHits())
					stat += QObject::tr("%L1 Power Plant<br>").arg(PowerPlantHits());
				if(SensorHits())
					stat += QObject::tr("%L1 Sensor<br>").arg(SensorHits());
				if(ShieldHits())
					stat += QObject::tr("%L1 Shield<br>").arg(ShieldHits());
				if(VehicleLauncherHits())
					stat += QObject::tr("%L1 Space Vehicle Launch<br>").arg(VehicleLauncherHits());
				if(VehicleLandingHits())
					stat += QObject::tr("%L1 Space Vehicle Recovery<br>").arg(VehicleLandingHits());
				if(TransporterHits())
					stat += QObject::tr("%L1 Transporter<br>").arg(TransporterHits());
				if(Turrets().size())
					stat += QObject::tr("%L1 Turret<br>").arg(Turrets().size());
				if(WeaponBayHits())
					stat += QObject::tr("%L1 Weapon<br>").arg(WeaponBayHits());
				stat = stat.left(stat.length()-4);	// Drop the last <br>
				stat += "</div>";
				stat += "</div>";
			}
			stat += "</div>";
		}

		{
			// Manufacturing Process
			stat += QObject::tr("<hr><b>MANUFACTURE</b>");
			stat += "<div style=\"margin-left: 10px;\">";
			if(m_MfgProc.Components().empty())
			{
				// The process could not be created. Probably too many components.
				stat += QString("<span style=\"color: rgb(255, 32, 32);\">");
				stat += QObject::tr("UNABLE TO MAKE A MANUFACTURING PROCESS");
				stat += QObject::tr("<br>Design May be Too Large<br>Try Reducing the Size of the Design");
				stat += QString("</span>");
			}
			else
			{
				// The process looks valid. List the components.
				if(designState != AuDesign::Final)
				{
					stat += QObject::tr("Build time estimate and manufacturing process are based on the original design "
							"configuration. "
							"<b>This information does not change when modules are installed or systems are refit later.</b> ");
					stat += "<br>";
				}

				unsigned int buildSecs = m_MfgProc.SecondsEstimated();
				unsigned int buildDays = buildSecs / (60 * 60 * 24);
				buildSecs -= buildDays * (60 * 60 * 24);
				unsigned int buildHours = buildSecs / (60 * 60);
				buildSecs -= buildHours * (60 * 60);
				unsigned int buildMins = buildSecs / 60;
				buildSecs -= buildMins * 60;
				if(buildDays)
					stat += QObject::tr("Estimated Build Time %1 days, %2 hours, %3 minutes, %4 seconds.")
						.arg(buildDays)
						.arg(buildHours)
						.arg(buildMins)
						.arg(buildSecs);
				else if(buildHours)
					stat += QObject::tr("Estimated Build Time %1 hours, %2 minutes, %3 seconds.")
						.arg(buildHours)
						.arg(buildMins)
						.arg(buildSecs);
				else if(buildMins)
					stat += QObject::tr("Estimated Build Time %1 minutes, %2 seconds.")
						.arg(buildMins)
						.arg(buildSecs);
				else
					stat += QObject::tr("Estimated Build Time %1 seconds.")
						.arg(buildSecs);
				stat += "<br>";

				QMap<QString, DeProcessComponentArray::const_iterator> sortedCon;
				QMap<QString, DeProcessComponentArray::const_iterator> sortedMod;
				for(DeProcessComponentArray::const_iterator pc = m_MfgProc.Components().begin();
						pc != m_MfgProc.Components().end(); pc++)
					if((*pc).QtyConsumed() && (*pc).Effect() == CE_Required)
						sortedCon.insert(g_Au.CommodityName((*pc).CommodityId()), pc);
					else
						sortedMod.insert(g_Au.CommodityName((*pc).CommodityId()), pc);

				for(QMap<QString, DeProcessComponentArray::const_iterator>::const_iterator si = sortedCon.cbegin();
						si != sortedCon.cend(); si++)
				{
					DeProcessComponentArray::const_iterator pc(*si);
					if(si != sortedCon.begin())
						stat += "<br>";

					stat += QObject::tr("<img height=\"16\" src=\"qrc%3\"> %L1 %2")
						.arg((*pc).QtyConsumed())
						.arg(g_Au.CommodityName((*pc).CommodityId()))
						.arg((*pc).CommodityBitmapName());
				}
				for(QMap<QString, DeProcessComponentArray::const_iterator>::const_iterator si = sortedMod.cbegin();
						si != sortedMod.cend(); si++)
				{
					DeProcessComponentArray::const_iterator pc(*si);
					if(!sortedCon.empty() || si != sortedMod.begin())
						stat += "<br>";

					stat += QObject::tr("<img height=\"16\" src=\"qrc%2\"> <img height=\"16\" src=\"qrc%3\"> %1")
						.arg(g_Au.CommodityName((*pc).CommodityId()))
						.arg((*pc).CommodityBitmapName())
						.arg((*pc).EffectBitmapName());
				}
			}
			stat += "</div>";
		}
	}

	// Append parts list, if there is one. It is already formatted the way we like.
	stat += m_PartsList;

	return stat;
}

// This function is used by AuItemScDesign::HtmlToolTip.
QString AuScReport::ReportToolTip() const
{
	QString stat;	// This will be our return value.

	// Design name
	// This name includes the architect name. e.g. "Scout Ship by Bob"
	// The working design will be blank.
	if(!m_DesignName.isEmpty() && !m_ArchitectName.isEmpty())
		stat = QString("<b>%1 by %2</b><br>").arg(m_DesignName).arg(m_ArchitectName);
	else if(!m_DesignName.isEmpty())
		stat = QString("<b>%1</b><br>").arg(m_DesignName);
	else if(!m_ArchitectName.isEmpty())
		stat = QString("<b>by %1</b><br>").arg(m_ArchitectName);

	if(!m_PartArchitects.Empty())
	{
		// Sort collaborating architects alphabetically.
		QMultiMap<QString, AuId> sortedNames;
		for(QHash<AuId,QString>::const_iterator ai = m_PartArchitects.Cbegin(); ai != m_PartArchitects.Cend(); ai++)
			sortedNames.insert((*ai).toUpper(), ai.key());

		stat += "<div style=\"margin-left: 10px;\">";
		stat += QObject::tr("Additional Architects");
		stat += "<div style=\"margin-left: 10px;\">";
		for(QMultiMap<QString, AuId>::const_iterator si = sortedNames.cbegin(); si != sortedNames.cend(); si++)
		{
			QHash<AuId,QString>::const_iterator ai = m_PartArchitects.Find(*si);
			if(ai != m_PartArchitects.Cend())
				stat += QString("%1 <i>%2</i><br>").arg(*ai).arg(ai.key().StringId());
		}
		//stat = stat.left(stat.count() - 4);	// Drop the trailing '<br>'.
		stat += "</div>";
		stat += "</div>";
	}

	// Description.
	if(!m_DesignDesc.isEmpty())
	{
		stat += QObject::tr("<b>DESCRIPTION</b>");
		stat += "<div style=\"margin-left: 10px;\">";
		stat += m_DesignDesc;
		stat += "</div>";
		stat += "<hr>";
	}

	// Summary
	{
		stat += "<b>";
		stat += StreamlineName(Streamline()).toUpper() + " ";

		switch(m_Structure)
		{
		case SC_Hull:
		case SC_Deprecated:
			stat += QObject::tr("INVALID");
			break;
		case SC_Starship:
			stat += QObject::tr("STARSHIP");
			break;
		case SC_Spaceship:
			stat += QObject::tr("SPACESHIP");
			break;
		case SC_Station:
			stat += QObject::tr("STATION");
			break;
		}
		stat += "</b>";

		// Quality
		// NOTE: Ship hit points are multiplied by 10 when reported. This is due to the fact that 10 hits of damage only
		//       causes one unit of damage to the ship. Each unit of ship damage is represented by a commodity that needs
		//       to be replaced. It takes 10 points of damage inflicted to cause one unit of ship damage.
		stat += "<div style=\"margin-left: 10px;\">";
		stat += QObject::tr("Minimum Manufacture Quality %1").arg(m_Quality);
		stat += "<div style=\"margin-left: 10px;\">";
		stat += "</div>";
		stat += "</div>";

		// Estimated cost.
		quint8 costEstQ = Quality();
		if(costEstQ < 1)
			costEstQ = 1;
		stat += "<div style=\"margin-left: 10px;\">";
		stat += QObject::tr("Estimated Cost %L1%2 Using Q%4 Components")
			.arg(EstimatedCost(),0,'f',0).arg(AuGlobal::CurrencySymbol()).arg(costEstQ);
		stat += "</div>";
	}
	return stat;
}

void AuScReport::operator>>(QDataStream& out) const
{
	out << (quint8)REPORT_SERIALIZE_VER;

	// Design
	out << m_ArchitectId;
	out << m_ArchitectName;
	out << m_DesignId;
	out << m_DesignName;
	out << m_PartArchitects;
	out << m_DesignDesc;

	// Structure
	out << m_LandingOrient;
	out << m_StationOrient;
	out << m_Streamline;
	out << m_Structure;

	// Volumes, in cubic meters.
	out << m_Hull;
	out << m_HullMax;
	out << m_HullMin;
	out << m_ArmorVolume;
	out << m_Bays;
	out << m_Capacitor;
	out << m_CryoBerthVolume;
	out << m_FTLDrives;
	out << m_FuelVolume;
	out << m_HoldVolume;
	out << m_LifeSupport;
	out << m_ManeuverDrive;
	out << m_PowerPlant;
	out << m_Sensor;
	out << m_Shield;
	out << m_SkeletonVolume;
	out << m_Turrets;

	// Quality
	out << m_Quality;
	out << m_CnCModule;
	out << m_CnCQ;
	m_MfgProc >> out;

	// Stations and Equipment
	out << m_CitizenPosts;
	out << m_CommandStations;
	out << m_CrewStations;
	out << m_DesignerStations;
	out << m_GameTables;
	out << m_GuardPosts;
	out << m_Lifts;
	out << m_LivestockPosts;
	out << m_ServicePanels;
	out << m_StatusDisplays;
	out << m_Transporters;
	out << m_VehicleLaunchers;
	out << m_VehicleLandingSystems;

	// Berths
	out << m_CaptBerths;
	out << m_CrewBerths;
	out << m_OfficerBerths;
	out << m_PassengerBerths;
	out << m_TroopBerths;

	// Parking
	out << m_ParkGround;
	out << m_ParkRocket;
	out << m_ParkSpacecraft;
	out << m_ParkSpace;
	out << m_ParkWater;

	// Mass
	out << m_MassKg;
	out << m_MassEquipmentKg;
	out << m_MassFuelKg;
	out << m_MassHullKg;

	// Analysis
	out << m_Acceleration;
	out << m_AccelerationVert;
	out << m_CapacitorMax;
	out << m_FaceCount;
	out << m_FTLDriveUnitsToEngage;
	out << m_FuelBurnRate;
	out << m_FuelCapacity;
	out << m_HasBar;
	out << m_HasPantry;
	out << m_HoldCapacity;
	out << m_HoldPowerNoShields;
	out << m_HoldPowerWithShields;
	out << m_IdlePowerNoShields;
	out << m_IdlePowerWithShields;
	out << m_LifeSupportNeeded;
	out << m_ManeuverFuelRate;
	out << m_PartsList;
	out << m_SensorRange;
	out << m_TriangleCount;
	out << m_TurnRate;
}

void AuScReport::operator<<(QDataStream& in)
{
	quint8 ver;
	in >> ver;

	// Design
	in >> m_ArchitectId;
	in >> m_ArchitectName;
	in >> m_DesignId;
	in >> m_DesignName;
	if(ver >= 0x02)
		in >> m_PartArchitects;
	if(ver >= 0x11)
		in >> m_DesignDesc;

	// Structure
	if(ver >= 0x12)
		in >> m_LandingOrient;
	if(ver >= 0x04)
		in >> m_StationOrient;
	in >> m_Streamline;
	in >> m_Structure;

	// Volumes, in cubic meters.
	in >> m_Hull;
	in >> m_HullMax;
	in >> m_HullMin;
	if(ver < 0x0a)
	{
		quint8 armorQ;
		in >> armorQ;
	}
	in >> m_ArmorVolume;
	in >> m_Bays;
	in >> m_Capacitor;
	in >> m_CryoBerthVolume;
	in >> m_FTLDrives;
	in >> m_FuelVolume;
	in >> m_HoldVolume;
	in >> m_LifeSupport;
	in >> m_ManeuverDrive;
	in >> m_PowerPlant;
	in >> m_Sensor;
	in >> m_Shield;
	if(ver >= 0x10)
		in >> m_SkeletonVolume;
	else
		m_SkeletonVolume = 0;
	in >> m_Turrets;

	// Quality
	in >> m_Quality;
	if(!m_Quality)
		m_Quality = 1;
	if(ver < 0x0a)
	{
		quint8 berthQ;
		in >> berthQ;
	}
	in >> m_CnCModule;
	in >> m_CnCQ;
	if(!m_CnCQ)
		m_CnCQ = 1;
	if(ver < 0x0a)
	{
		QString eqDesc;
		quint16 eqPart;
		quint8 eqQ;
		in >> eqDesc;
		in >> eqPart;
		in >> eqQ;
	}
	if(ver < 0x02)
	{
		quint8 mfgProcQ;
		in >> mfgProcQ;
	}
	else
		m_MfgProc << in;
	if(ver < 0x0a)
	{
		quint16 modCID;
		quint8 modQ, volQ;
		in >> modCID;
		in >> modQ;
		in >> volQ;
	}

	// Stations
	if(ver >= 0x0e)
		in >> m_CitizenPosts;
	if(ver >= 0x03)
		in >> m_CommandStations;
	in >> m_CrewStations;
	if(ver >= 0x0e)
		in >> m_DesignerStations;
	if(ver < 0x05)
	{
		// At version 5, changed to an AuScGameTableHash
		AuScPartSet gameTables;
		in >> gameTables;
		m_GameTables.clear();
		for(QSet<quint16>::const_iterator gi = gameTables.cbegin(); gi != gameTables.cend(); gi++)
			m_GameTables.insert(*gi, AuScReportGameTable(0));	// We don't know the room id. Only affects lighting.
	}
	else
		in >> m_GameTables;
	in >> m_GuardPosts;
	if(ver >= 0x06)
		in >> m_Lifts;
	if(ver >= 0x0e)
		in >> m_LivestockPosts;
	if(ver >= 0x09)
	{
		in >> m_ServicePanels;
		in >> m_StatusDisplays;
	}
	if(ver >= 0x01)
	{
		in >> m_Transporters;
		in >> m_VehicleLaunchers;
		in >> m_VehicleLandingSystems;
	}

	// Berths
	in >> m_CaptBerths;
	in >> m_CrewBerths;
	in >> m_OfficerBerths;
	in >> m_PassengerBerths;
	in >> m_TroopBerths;

	// Parking
	in >> m_ParkGround;
	if(ver >= 0x08)
		in >> m_ParkRocket;
	if(ver >= 0x0c)
		in >> m_ParkSpacecraft;
	in >> m_ParkSpace;
	in >> m_ParkWater;
	// In version 0x0c, the parkSpace and parkSpacecraft were read back in backwards order. This means that a report that was
	// saved, loaded, then resaved would have these value backward. That appears to be the case on my Atlantis design #2496501.
	// Spacecraft parking spaces were new in version 0x0c so it is likely few designs have them, if any, since they serve no
	// useful purpose yet. Let's take an educated guess that if the design has spacecraft parking spots but no space vehicle
	// parking spots, they are backward. That is true for Atlantis.
	if(ver == 0x0c && m_ParkSpacecraft && !m_ParkSpace)
	{
		m_ParkSpace = m_ParkSpacecraft;
		m_ParkSpacecraft = 0;
	}

	// Mass
	in >> m_MassKg;
	in >> m_MassEquipmentKg;
	in >> m_MassFuelKg;
	in >> m_MassHullKg;

	// Analysis
	in >> m_Acceleration;
	in >> m_AccelerationVert;
	in >> m_CapacitorMax;
	if(ver >= 0x0b)
		in >> m_FaceCount;
	in >> m_FTLDriveUnitsToEngage;
	in >> m_FuelBurnRate;
	in >> m_FuelCapacity;
	if(ver >= 0x0f)
	{
		in >> m_HasBar;
		in >> m_HasPantry;
	}
	in >> m_HoldCapacity;
	in >> m_HoldPowerNoShields;
	in >> m_HoldPowerWithShields;
	in >> m_IdlePowerNoShields;
	in >> m_IdlePowerWithShields;
	in >> m_LifeSupportNeeded;
	in >> m_ManeuverFuelRate;
	if(ver >= 0x07)
		in >> m_PartsList;
	in >> m_SensorRange;
	if(ver >= 0x0b)
		in >> m_TriangleCount;
	in >> m_TurnRate;
}

bool AuScReport::operator==(const AuScReport& c) const
{
	// Design
	if(m_ArchitectId != c.m_ArchitectId)
		return false;
	if(m_ArchitectName != c.m_ArchitectName)
		return false;
	if(m_DesignDesc != c.m_DesignDesc)
		return false;
	if(m_DesignId != c.m_DesignId)
		return false;
	if(m_DesignName != c.m_DesignName)
		return false;
	if(m_PartArchitects != c.m_PartArchitects)
		return false;

	// Structure
	if(m_LandingOrient != c.m_LandingOrient)
		return false;
	if(m_StationOrient != c.m_StationOrient)
		return false;
	if(m_Streamline != c.m_Streamline)
		return false;
	if(m_Structure != c.m_Structure)
		return false;

	// Volumes, in cubic meters.
	if(m_Hull != c.m_Hull)
		return false;
	if(m_HullMax != c.m_HullMax)
		return false;
	if(m_HullMin != c.m_HullMin)
		return false;
	if(m_ArmorVolume != c.m_ArmorVolume)
		return false;
	if(m_Bays != c.m_Bays)
		return false;
	if(m_Capacitor != c.m_Capacitor)
		return false;
	if(m_CryoBerthVolume != c.m_CryoBerthVolume)
		return false;
	if(m_FTLDrives != c.m_FTLDrives)
		return false;
	if(m_FuelVolume != c.m_FuelVolume)
		return false;
	if(m_HoldVolume != c.m_HoldVolume)
		return false;
	if(m_LifeSupport != c.m_LifeSupport)
		return false;
	if(m_ManeuverDrive != c.m_ManeuverDrive)
		return false;
	if(m_PowerPlant != c.m_PowerPlant)
		return false;
	if(m_Sensor != c.m_Sensor)
		return false;
	if(m_Shield != c.m_Shield)
		return false;
	if(m_SkeletonVolume != c.m_SkeletonVolume)
		return false;
	if(m_Turrets != c.m_Turrets)
		return false;

	// Quality
	if(m_Quality != c.m_Quality)
		return false;
	if(m_CnCModule != c.m_CnCModule)
		return false;
	if(m_CnCQ != c.m_CnCQ)
		return false;
	//if(m_MfgProc != c.m_MfgProc)
		//return false;

	// Stations
	if(m_CitizenPosts != c.m_CitizenPosts)
		return false;
	if(m_CommandStations != c.m_CommandStations)
		return false;
	if(m_CrewStations != c.m_CrewStations)
		return false;
	if(m_DesignerStations != c.m_DesignerStations)
		return false;
	if(m_GameTables != c.m_GameTables)
		return false;
	if(m_GuardPosts != c.m_GuardPosts)
		return false;
	if(m_Lifts != c.m_Lifts)
		return false;
	if(m_LivestockPosts != c.m_LivestockPosts)
		return false;
	if(m_ServicePanels != c.m_ServicePanels)
		return false;
	if(m_StatusDisplays != c.m_StatusDisplays)
		return false;
	if(m_Transporters != c.m_Transporters)
		return false;
	if(m_VehicleLaunchers != c.m_VehicleLaunchers)
		return false;
	if(m_VehicleLandingSystems != c.m_VehicleLandingSystems)
		return false;

	// Berths
	if(m_CaptBerths != c.m_CaptBerths)
		return false;
	if(m_CrewBerths != c.m_CrewBerths)
		return false;
	if(m_OfficerBerths != c.m_OfficerBerths)
		return false;
	if(m_PassengerBerths != c.m_PassengerBerths)
		return false;
	if(m_TroopBerths != c.m_TroopBerths)
		return false;

	// Parking
	if(m_ParkGround != c.m_ParkGround)
		return false;
	if(m_ParkRocket != c.m_ParkRocket)
		return false;
	if(m_ParkSpace != c.m_ParkSpace)
		return false;
	if(m_ParkSpacecraft != c.m_ParkSpacecraft)
		return false;
	if(m_ParkWater != c.m_ParkWater)
		return false;

	// Mass
	if(m_MassKg != c.m_MassKg)
		return false;
	if(m_MassEquipmentKg != c.m_MassEquipmentKg)
		return false;
	if(m_MassFuelKg != c.m_MassFuelKg)
		return false;
	if(m_MassHullKg != c.m_MassHullKg)
		return false;

	// Analysis
	if(m_Acceleration != c.m_Acceleration)
		return false;
	if(m_AccelerationVert != c.m_AccelerationVert)
		return false;
	if(m_CapacitorMax != c.m_CapacitorMax)
		return false;
	if(m_FaceCount != c.m_FaceCount)
		return false;
	if(m_FTLDriveUnitsToEngage != c.m_FTLDriveUnitsToEngage)
		return false;
	if(m_FuelBurnRate != c.m_FuelBurnRate)
		return false;
	if(m_FuelCapacity != c.m_FuelCapacity)
		return false;
	if(m_HasBar != c.m_HasBar)
		return false;
	if(m_HasPantry != c.m_HasPantry)
		return false;
	if(m_HoldCapacity != c.m_HoldCapacity)
		return false;
	if(m_HoldPowerNoShields != c.m_HoldPowerNoShields)
		return false;
	if(m_HoldPowerWithShields != c.m_HoldPowerWithShields)
		return false;
	if(m_IdlePowerNoShields != c.m_IdlePowerNoShields)
		return false;
	if(m_IdlePowerWithShields != c.m_IdlePowerWithShields)
		return false;
	if(m_LifeSupportNeeded != c.m_LifeSupportNeeded)
		return false;
	if(m_ManeuverFuelRate != c.m_ManeuverFuelRate)
		return false;
	if(m_PartsList != c.m_PartsList)
		return false;
	if(m_SensorRange != c.m_SensorRange)
		return false;
	if(m_TriangleCount != c.m_TriangleCount)
		return false;
	if(m_TurnRate != c.m_TurnRate)
		return false;

	return true;
}

QString AuScReport::CommandStationName(quint16 partId) const
{
	AuScReportStationHash::const_iterator ci = m_CommandStations.find(partId);
	if(ci != m_CommandStations.cend())
		return (*ci).Desc();
	return "";
}

QString AuScReport::CrewStationName(quint16 partId) const
{
	AuScReportStationHash::const_iterator ci = m_CrewStations.find(partId);
	if(ci != m_CrewStations.cend())
		return (*ci).Desc();

	AuScReportTransporterStationHash::const_iterator ti = m_Transporters.find(partId);
	if(ti != m_Transporters.cend())
		return (*ti).Desc();

	ci = m_DesignerStations.find(partId);
	if(ci != m_DesignerStations.cend())
		return (*ci).Desc();

	return "";
}

QString AuScReport::ServicePanelName(quint16 partId) const
{
	AuScReportServicePanelHash::const_iterator ci = m_ServicePanels.find(partId);
	if(ci != m_ServicePanels.cend())
		return (*ci).Desc();
	return "";
}

QString AuScReport::StatusDisplayName(quint16 partId) const
{
	AuScReportStatusDisplayHash::const_iterator ci = m_StatusDisplays.find(partId);
	if(ci != m_StatusDisplays.cend())
		return (*ci).Desc();
	return "";
}

bool AuScReport::HasSurgeryUnit() const
{
	for(QHash<quint16, AuScReportStation>::const_iterator ci = m_CrewStations.cbegin(); ci != m_CrewStations.cend(); ci++)
		if(ScModuleSystem((*ci).Module()) == SYS_SurgeryUnit)
			return true;
	return false;
}

quint16 AuScReport::SurgeryUnitCount() const
{
	quint16 c = 0;
	for(QHash<quint16, AuScReportStation>::const_iterator ci = m_CrewStations.cbegin(); ci != m_CrewStations.cend(); ci++)
		if(ScModuleSystem((*ci).Module()) == SYS_SurgeryUnit)
			c++;
	return c;
}

double AuScReport::ArmorLevel() const
{
	return AuScDesign::ArmorLevel(m_ArmorVolume, HullVolume());
}

quint32 AuScReport::CapacitorHits() const
{
	return EquipmentVolumeHits(m_Capacitor.Volume());
}

quint32 AuScReport::CryoBerthHits() const
{
	return EquipmentVolumeHits(m_CryoBerthVolume);
}

quint32 AuScReport::VehicleLandingHits() const
{
	return m_VehicleLandingSystems.size() * d_VehicleLandingSystemHits;
}

quint32 AuScReport::VehicleLauncherHits() const
{
	return m_VehicleLaunchers.size() * d_VehicleLauncherHits;
}

quint32 AuScReport::FTLDriveHits() const
{
	quint32 v = 0;
	for(AuScReportSysVolHash::const_iterator fi = m_FTLDrives.cbegin(); fi != m_FTLDrives.cend(); fi++)
		v += EquipmentVolumeHits((*fi).Volume());
	return v;
}

quint32 AuScReport::FTLDriveHits(quint16 partId) const
{
	AuScReportSysVolHash::const_iterator fi = m_FTLDrives.find(partId);
	if(fi == m_FTLDrives.cend())
		return 0;
	return EquipmentVolumeHits((*fi).Volume());
}

quint8 AuScReport::FTLDriveQuality(quint16 partId) const
{
	AuScReportSysVolHash::const_iterator fi = m_FTLDrives.find(partId);
	if(fi == m_FTLDrives.cend())
		return 0;
	return (*fi).Quality();
}

void AuScReport::SetFTLDriveQuality(quint16 partId, quint8 quality)
{
	AuScReportSysVolHash::iterator fi = m_FTLDrives.find(partId);
	if(fi != m_FTLDrives.end())
		(*fi).SetQuality(quality);
}

quint32 AuScReport::FuelHits() const
{
	return HullVolumeHits(m_FuelVolume);
}

quint32 AuScReport::HoldHits() const
{
	return HullVolumeHits(m_HoldVolume);
}

quint32 AuScReport::SkeletonHits() const
{
	return HullVolumeHits(m_SkeletonVolume) * 4;
}

float AuScReport::HullHitPointsPerUnit() const
{
	const float tlBoost = QualityBoostF(m_Hull.Quality());
	switch(m_Hull.Module())
	{
	case CID_Adamantine:
		return AuScDesign::HullDensityFactor(ArmorLevel(), Streamline()) * tlBoost * 1.6f;
	case CID_Vulcium:
		return AuScDesign::HullDensityFactor(ArmorLevel(), Streamline()) * tlBoost * 1.5f;
	case CID_Magmium:
		return AuScDesign::HullDensityFactor(ArmorLevel(), Streamline()) * tlBoost * 1.25f;
	case CID_Metal:
		return AuScDesign::HullDensityFactor(ArmorLevel(), Streamline()) * tlBoost;
	default:
	case CID_Stone:
		return AuScDesign::HullDensityFactor(ArmorLevel(), Streamline()) * tlBoost * 1.1f;
	case CID_Lumber:
		return AuScDesign::HullDensityFactor(ArmorLevel(), Streamline()) * tlBoost * .5f;
	case CID_Wood:
		return AuScDesign::HullDensityFactor(ArmorLevel(), Streamline()) * tlBoost * .75f;
	}
}

quint32 AuScReport::HullVolumeHits(quint32 volumeCuM) const
{
	if(volumeCuM == 0)
		return 0;
	double volCuFt = volumeCuM * CubicFeetPerCubicMeter();
	quint32 hits = (quint32)(volCuFt * HullHitPointsPerUnit() / 256);
	if(!hits)
		return 1;
	return hits;
}

//static
float AuScReport::EquipmentHitPointsPerUnit()
{
	return 1.f;
}

//static
quint32 AuScReport::EquipmentVolumeHits(quint32 volumeCuM)
{
	if(volumeCuM == 0)
		return 0;
	double volCuFt = volumeCuM * CubicFeetPerCubicMeter();
	quint32 hits = (quint32)(volCuFt * EquipmentHitPointsPerUnit() / 256);
	if(!hits)
		return 1;
	return hits;
}

quint32 AuScReport::HullHits() const
{
	if(SkeletonVolume())
		return HullVolumeHits(HullVolume() - SkeletonVolume()) + SkeletonHits();
	else
		return HullVolumeHits(HullVolume());
}

quint32 AuScReport::LifeSupportHits() const
{
	return EquipmentVolumeHits(m_LifeSupport.Volume()) + CryoBerthHits();
}

COMMODITYID AuScReport::HullMaterial() const
{
	return HullModuleMaterial(HullModule());
}

quint32 AuScReport::ManeuverDriveHits() const
{
	return EquipmentVolumeHits(m_ManeuverDrive.Volume());
}

quint32 AuScReport::PowerPlantHits() const
{
	return EquipmentVolumeHits(m_PowerPlant.Volume());
}

quint32 AuScReport::SensorHits() const
{
	return EquipmentVolumeHits(m_Sensor.Volume());
}

quint32 AuScReport::ShieldHits() const
{
	return EquipmentVolumeHits(m_Shield.Volume());
}

quint32 AuScReport::TransporterHits() const
{
	quint32 v = 0;
	for(AuScReportTransporterStationHash::const_iterator ti = m_Transporters.cbegin(); ti != m_Transporters.cend(); ti++)
		v += (*ti).Hits();
	return v;
}

quint32 AuScReport::TransporterHits(quint16 partId) const
{
	AuScReportTransporterStationHash::const_iterator ti = m_Transporters.find(partId);
	return (*ti).Hits();
}

quint8 AuScReport::TransporterQuality(quint16 partId) const
{
	AuScReportTransporterStationHash::const_iterator ti = m_Transporters.find(partId);
	if(ti != m_Transporters.end())
		return (*ti).Quality();
	return 0;
}

void AuScReport::SetTransporterQuality(quint16 partId, quint8 quality)
{
	AuScReportTransporterStationHash::iterator ti = m_Transporters.find(partId);
	if(ti != m_Transporters.end())
		(*ti).SetQuality(quality);
}

quint32 AuScReport::WeaponBayHits() const
{
	quint32 v = 0;
	for(AuScReportSysVolHash::const_iterator fi = m_Bays.cbegin(); fi != m_Bays.cend(); fi++)
		v += EquipmentVolumeHits((*fi).Volume());
	return v;
}

quint32 AuScReport::WeaponBayHits(quint16 partId) const
{
	AuScReportSysVolHash::const_iterator fi = m_Bays.find(partId);
	if(fi == m_Bays.cend())
		return 0;
	return EquipmentVolumeHits((*fi).Volume());
}

quint8 AuScReport::WeaponBayQuality(quint16 partId) const
{
	AuScReportSysVolHash::const_iterator fi = m_Bays.find(partId);
	if(fi == m_Bays.cend())
		return 0;
	return (*fi).Quality();
}

void AuScReport::SetWeaponBayQuality(quint16 partId, quint8 quality)
{
	AuScReportSysVolHash::iterator fi = m_Bays.find(partId);
	if(fi != m_Bays.end())
		(*fi).SetQuality(quality);
}

double AuScReport::TurretRange(quint16 which) const
{
	AuScReportTurretHash::const_iterator ti = m_Turrets.find(which);
	if(ti == m_Turrets.cend())
		return 0;
	const float tlBoost = QualityBoostF((*ti).Quality());
	return 1000.f * FeetPerMeter() * tlBoost;
}

quint8 AuScReport::TurretQuality(quint16 which) const
{
	AuScReportTurretHash::const_iterator ti = m_Turrets.find(which);
	if(ti == m_Turrets.cend())
		return 0;
	return (*ti).Quality();
}

void AuScReport::SetTurretQuality(quint16 which, quint8 quality)
{
	AuScReportTurretHash::iterator ti = m_Turrets.find(which);
	if(ti != m_Turrets.end())
		(*ti).SetQuality(quality);
}

QString AuScReport::SystemName(const AuSysId& sysId) const
{
	switch(sysId.System())
	{
	case SYS_CommandAndControl:
		{
			AuScReportStationHash::const_iterator ci = m_CommandStations.find(sysId.Which());
			if(ci != m_CommandStations.cend())
				return QString("%1 %2").arg((*ci).Desc()).arg(sysId.Which());
			ci = m_CrewStations.find(sysId.Which());
			if(ci != m_CrewStations.cend())
				return QString("%1 %2").arg((*ci).Desc()).arg(sysId.Which());
			AuScReportTransporterStationHash::const_iterator ti = m_Transporters.find(sysId.Which());
			if(ti != m_Transporters.cend())
				return QString("%1 %2").arg((*ti).Desc()).arg(sysId.Which());
			ci = m_DesignerStations.find(sysId.Which());
			if(ci != m_DesignerStations.cend())
				return QString("%1 %2").arg((*ci).Desc()).arg(sysId.Which());
		}
		break;

	case SYS_Fuel:
	case SYS_Hold:
	case SYS_Hull:
		return QObject::tr("%1, %2")
			.arg(sysId.ScName())
			.arg(g_Au.CommodityName(m_Hull.Module()));

	case SYS_ManeuverDrive:
		return QObject::tr("%1, %2")
			.arg(sysId.ScName())
			.arg(g_Au.CommodityName(m_ManeuverDrive.Module()));

	case SYS_Count:
		break;

	case SYS_LifeSupport:
		return QObject::tr("%1, %2")
			.arg(sysId.ScName())
			.arg(g_Au.CommodityName(m_LifeSupport.Module()));

	case SYS_PowerPlant:
		return QObject::tr("%1, %2")
			.arg(sysId.ScName())
			.arg(g_Au.CommodityName(m_PowerPlant.Module()));

	case SYS_Sensor:
		return QObject::tr("%1, %2")
			.arg(sysId.ScName())
			.arg(g_Au.CommodityName(m_Sensor.Module()));

	case SYS_Shield:
		return QObject::tr("%1, %2")
			.arg(sysId.ScName())
			.arg(g_Au.CommodityName(m_Shield.Module()));

	case SYS_SurgeryUnit:
		{
			AuScReportStationHash::const_iterator ci = m_CrewStations.find(sysId.Which());
			if(ci != m_CrewStations.cend())
				return QObject::tr("%1 %3, %2")
					.arg(sysId.ScName())
					.arg(g_Au.CommodityName((*ci).Module()))
					.arg(sysId.Which());
		}
		break;

	case SYS_Transporter:
		{
			AuScReportTransporterStationHash::const_iterator ci = m_Transporters.find(sysId.Which());
			if(ci != m_Transporters.cend())
				return QObject::tr("%1 %3, %2")
					.arg(sysId.ScName())
					.arg(g_Au.CommodityName((*ci).Module()))
					.arg(sysId.Which());
		}
		break;

	case SYS_Turret:
		{
			AuScReportTurretHash::const_iterator ci = m_Turrets.find(sysId.Which());
			if(ci != m_Turrets.cend())
				return QObject::tr("%1 %3, %2")
					.arg(sysId.ScName())
					.arg(g_Au.CommodityName((*ci).Module()))
					.arg(sysId.Which());
		}
		break;

	case SYS_VehicleLaunch:
	case SYS_VehicleLanding:
		if(sysId.Which())
			return QObject::tr("%1 %2")
				.arg(sysId.ScName())
				.arg(sysId.Which());
		break;

	case SYS_Weapon:
		{
			AuScReportSysVolHash::const_iterator ci = m_Bays.find(sysId.Which());
			if(ci != m_Bays.cend())
				return QObject::tr("%1 %3, %2")
					.arg(sysId.ScName())
					.arg(g_Au.CommodityName((*ci).Module()))
					.arg(sysId.Which());
		}
		break;

	case SYS_FTLDrive:
		{
			AuScReportSysVolHash::const_iterator ci = m_FTLDrives.find(sysId.Which());
			if(ci != m_FTLDrives.cend())
				return QObject::tr("%1 %3, %2")
					.arg(sysId.ScName())
					.arg(g_Au.CommodityName((*ci).Module()))
					.arg(sysId.Which());
		}
		break;

	case SYS_Capacitor:
		return QObject::tr("%1, %2")
			.arg(sysId.ScName())
			.arg(g_Au.CommodityName(m_Capacitor.Module()));
	}
	return sysId.ScName();
}

QString AuScReport::SystemName(const AuSysId& sysId, COMMODITYID module) const
{
	switch(sysId.System())
	{
	case SYS_CommandAndControl:
		{
			AuScReportStationHash::const_iterator ci = m_CommandStations.find(sysId.Which());
			if(ci != m_CommandStations.cend())
				return (*ci).Desc();
			ci = m_CrewStations.find(sysId.Which());
			if(ci != m_CrewStations.cend())
				return (*ci).Desc();
			AuScReportTransporterStationHash::const_iterator ti = m_Transporters.find(sysId.Which());
			if(ti != m_Transporters.cend())
				return (*ti).Desc();
			ci = m_DesignerStations.find(sysId.Which());
			if(ci != m_DesignerStations.cend())
				return (*ci).Desc();
		}
		break;
	case SYS_Fuel:
	case SYS_ManeuverDrive:
	case SYS_Hold:
	case SYS_Hull:
	case SYS_LifeSupport:
	case SYS_PowerPlant:
	case SYS_Sensor:
	case SYS_Shield:
	case SYS_Capacitor:
		// There can be only one of each of these systems.
		return QObject::tr("%1, %2")
			.arg(sysId.ScName())
			.arg(g_Au.CommodityName(module));

	case SYS_FTLDrive:
	case SYS_SurgeryUnit:
	case SYS_Turret:
	case SYS_Transporter:
	case SYS_Weapon:
		// There can be multiples of these systems so their id is helpful to distinguish them.
		return QObject::tr("%1 %2, %3")
			.arg(sysId.ScName())
			.arg(sysId.Which())
			.arg(g_Au.CommodityName(module));

	case SYS_VehicleLaunch:
	case SYS_VehicleLanding:
		if(sysId.Which())
			return QObject::tr("%1 %2")
				.arg(sysId.ScName())
				.arg(sysId.Which());
		break;

	case SYS_Count:
		break;
	}
	return sysId.ScName();
}

quint16 AuScReport::TransporterRoomId(quint16 partId) const
{
	AuScReportTransporterStationHash::const_iterator ti = m_Transporters.find(partId);
	if(ti == m_Transporters.cend())
		return 0;
	return (*ti).RoomId();
}

float AuScReport::HarvesterBayRange(quint16 which, unsigned int unitsFired) const
{
	AuScReportSysVolHash::const_iterator ti = m_Bays.find(which);
	if(ti == m_Bays.cend())
		return 0;
	double wpnsCuFt = unitsFired * 256;
	return AuScDesign::HarvesterBayRange(wpnsCuFt, (*ti).Quality());
}

float AuScReport::TractorBeamRange(quint16 which, unsigned int unitsFired) const
{
	AuScReportSysVolHash::const_iterator ti = m_Bays.find(which);
	if(ti == m_Bays.cend())
		return 0;
	double wpnsCuFt = unitsFired * 256;
	return AuScDesign::TractorBeamRange(wpnsCuFt, (*ti).Quality());
}

float AuScReport::WeaponBayRange(quint16 which) const
{
	AuScReportSysVolHash::const_iterator ti = m_Bays.find(which);
	if(ti == m_Bays.cend())
		return 0;
	return AuScDesign::BayWeaponRange((*ti).Quality());
}

double AuScReport::CapacitorUnits() const
{
	double cuft = m_Capacitor.Volume() * CubicFeetPerCubicMeter();
	return cuft / 256;
}

double AuScReport::FTLDriveUnits(quint16 partId) const
{
	AuScReportSysVolHash::const_iterator fi = m_FTLDrives.find(partId);
	if(fi == m_FTLDrives.cend())
		return 0;
	double cuft = (*fi).Volume() * CubicFeetPerCubicMeter();
	return cuft / 256;
}

double AuScReport::LifeSupportUnits() const
{
	double cuft = m_LifeSupport.Volume() * CubicFeetPerCubicMeter();
	return cuft / 256;
}

double AuScReport::ManeuverDriveUnits() const
{
	double cuft = m_ManeuverDrive.Volume() * CubicFeetPerCubicMeter();
	return cuft / 256;
}

double AuScReport::PowerPlantUnits() const
{
	double cuft = m_PowerPlant.Volume() * CubicFeetPerCubicMeter();
	return cuft / 256;
}

double AuScReport::SensorUnits() const
{
	double cuft = m_Sensor.Volume() * CubicFeetPerCubicMeter();
	return cuft / 256;
}

double AuScReport::ShieldCapacitor() const
{
	return ShieldUnits();
}

double AuScReport::ShieldUnits() const
{
	double cuft = m_Shield.Volume() * CubicFeetPerCubicMeter();
	return cuft / 256;
}

double AuScReport::WeaponBayUnits(quint16 partId) const
{
	AuScReportSysVolHash::const_iterator fi = m_Bays.find(partId);
	if(fi == m_Bays.cend())
		return 0;
	double cuft = (*fi).Volume() * CubicFeetPerCubicMeter();
	return cuft / 256;
}

float AuScReport::MaxDrag() const
{
	float maxDrag[SLINE_Count] =
	{
		256,		// SLINE_None,						// Not streamlined.
		512,		// SLINE_Atmospheric,				// Streamlined and hardened for atmospheric entry.
		640,		// SLINE_SeaSurface,				// Streamlined and hardened for sea surface operations.
		768,		// SLINE_Submersible,				// Streamlined and hardened for under water operations.
		832,		// SLINE_Subterranean,				// Streamlined and hardened for under ground operations.
	};
	return maxDrag[Streamline()];
}

bool AuScReport::IsSpaceStation() const
{
	return Structure() == SC_Station;
}

quint16 AuScReport::FTLWarpDrive() const
{
	for(AuScReportSysVolHash::const_iterator fi = m_FTLDrives.cbegin(); fi != m_FTLDrives.cend(); fi++)
		if(DbCommodity::IsWarpFTLDriveModule((*fi).Module()))
			return fi.key();
	return 0;
}

quint16 AuScReport::FTLWormholeDrive() const
{
	for(AuScReportSysVolHash::const_iterator fi = m_FTLDrives.cbegin(); fi != m_FTLDrives.cend(); fi++)
		if(DbCommodity::IsWormholeFTLDriveModule((*fi).Module()))
			return fi.key();
	return 0;
}

QList<quint16> AuScReport::SurgeryUnitIds() const
{
	QList<quint16> unitIds;
	for(QHash<quint16, AuScReportStation>::const_iterator ci = m_CrewStations.cbegin(); ci != m_CrewStations.cend(); ci++)
		if(ScModuleSystem((*ci).Module()) == SYS_SurgeryUnit)
			unitIds.append(ci.key());
	return unitIds;
}

COMMODITYID AuScReport::SurgeryUnitModule(quint16 consoleId) const
{
	AuScReportStationHash::const_iterator ci = m_CrewStations.find(consoleId);
	if(ci == m_CrewStations.cend())
		return CID_Invalid;

	if(ScModuleSystem((*ci).Module()) != SYS_SurgeryUnit)
		return CID_Invalid;

	return (*ci).Module();
}

void AuScReport::SetSurgeryUnitQuality(quint16 consoleId, quint8 quality)
{
	AuScReportStationHash::iterator ci = m_CrewStations.find(consoleId);
	if(ci == m_CrewStations.end())
		return;

	if(ScModuleSystem((*ci).Module()) != SYS_SurgeryUnit)
		return;

	(*ci).SetQuality(quality);
}

quint8 AuScReport::SurgeryUnitQuality(quint16 consoleId) const
{
	AuScReportStationHash::const_iterator ci = m_CrewStations.find(consoleId);
	if(ci == m_CrewStations.cend())
		return CID_Invalid;

	if(ScModuleSystem((*ci).Module()) != SYS_SurgeryUnit)
		return CID_Invalid;

	return (*ci).Quality();
}

COMMODITYID AuScReport::LifeSupportModule() const
{
	return m_LifeSupport.Module();
}

COMMODITYID AuScReport::HullModule() const
{
	return m_Hull.Module();
}

COMMODITYID AuScReport::CapacitorModule() const
{
	return m_Capacitor.Module();
}

COMMODITYID AuScReport::ManeuverDriveModule() const
{
	return m_ManeuverDrive.Module();
}

COMMODITYID AuScReport::PowerPlantModule() const
{
	return m_PowerPlant.Module();
}

COMMODITYID AuScReport::ShieldModule() const
{
	return m_Shield.Module();
}

COMMODITYID AuScReport::SensorModule() const
{
	return m_Sensor.Module();
}

float AuScReport::WeaponBayPower(quint16 which, float unitsFired) const
{
	AuScReportSysVolHash::const_iterator ti = m_Bays.find(which);
	if(ti == m_Bays.cend())
		return 0;
	double wpnsCuFt = unitsFired * 256;
	return AuScDesign::BayWeaponPower(wpnsCuFt, (*ti).Quality());
}

// Returns life support power use rate in units per real world second.
double AuScReport::LifeSupportPowerRate() const
{
	// Each unit uses one power unit per U day.
	// Calculate use rate in power units per real second.
	const double rUnitRate = (double)UDaysPerRealDay() / (double)(24 * 3600);
	const double rUseRate = LifeSupportUnits() * rUnitRate;
	return rUseRate;
}

// Returns FTL drive idle power use rate in units per real world second.
double AuScReport::FTLDrivePowerRate(quint16 partId) const
{
	// Each FTL drive uses one power unit per U day.
	// Calculate use rate in power units per real second.
	const double rUnitRate = (double)UDaysPerRealDay() / (double)(24 * 3600);
	const double rUseRate = FTLDriveUnits(partId) * rUnitRate;
	return rUseRate;
}

// Returns maneuver drive fuel use rate in units per real world second.
// Only rocket drives use fuel.
double AuScReport::ManeuverFuelRate() const
{
	// Rocket drives do not use power.
	if(m_ManeuverDrive.Module() != CID_RocketDriveModule)
		return 0.f;

	// Each maneuver drive uses one power unit per U day.
	// Calculate use rate in power units per real second.
	const double rUnitRate = (double)UDaysPerRealDay() / (double)(24 * 3600);
	const double rUseRate = ManeuverDriveUnits() * rUnitRate / PowerPerFuel();
	return rUseRate;
}

// Returns maneuver drive power use rate in units per real world second.
double AuScReport::ManeuverPowerRate() const
{
	// Rocket drives do not use power.
	if(m_ManeuverDrive.Module() == CID_RocketDriveModule)
		return 0.f;

	// Each maneuver drive uses one power unit per U day.
	// Calculate use rate in power units per real second.
	const double rUnitRate = (double)UDaysPerRealDay() / (double)(24 * 3600);
	const double rUseRate = ManeuverDriveUnits() * rUnitRate;
	return rUseRate;
}

// Returns shield power use rate in units per real world second.
double AuScReport::ShieldPowerRate() const
{
	// Each shield unit uses one power units per U day.
	// Calculate use rate in power units per real second.
	const double rUnitRate = (double)UDaysPerRealDay() / (double)(24 * 3600);
	const double rUseRate = ShieldUnits() * rUnitRate;
	return rUseRate;
}

// Returns sensor power use rate in units per real world second.
double AuScReport::SensorPowerRate() const
{
	// Each sensor uses one power units per U day.
	// Calculate use rate in power units per real second.
	const double rUnitRate = (double)UDaysPerRealDay() / (double)(24 * 3600);
	const double rUseRate = SensorUnits() * rUnitRate;
	return rUseRate;
}

float AuScReport::Density() const
{
	return MassLbs() / (float)(HullVolume() * CubicFeetPerCubicMeter());
}

QString AuScReport::DescLine() const
{
	if(m_Streamline != SLINE_None)
		return QString("%1 %2, Q%3, %L4m%5")
			.arg(StreamlineName(Streamline()))
			.arg(SpacecraftStructureName(Structure()))
			.arg(Quality())
			.arg(HullVolume()).arg(g_Au.CubedSymbol());
	else
		return QString("%1, Q%2, %L3m%4")
			.arg(SpacecraftStructureName(Structure()))
			.arg(Quality())
			.arg(HullVolume()).arg(g_Au.CubedSymbol());
}

quint8 AuScReport::MaxCapacitorFTLSetting(quint16 whichFTL) const
{
	AuScReportSysVolHash::const_iterator fi = m_FTLDrives.find(whichFTL);
	if(fi == m_FTLDrives.cend())
		return 0;

	return AuScDesign::MaxCapacitorFTLSetting((*fi).Module(), (*fi).Quality(), m_FTLDriveUnitsToEngage, m_CapacitorMax);
}

bool AuScReport::MustBeBuiltInSpace() const
{
	if(IsSpaceStation())
		return true;

	AuPoint min, max;
	Extents(min, max);
	return (min.x < -GroundSpawnSizeLimit() ||
			max.x > GroundSpawnSizeLimit() ||
			min.y < -GroundSpawnSizeLimit() ||
			max.y > GroundSpawnSizeLimit() ||
			min.z < -GroundSpawnSizeLimit() ||
			max.z > GroundSpawnSizeLimit());
}

bool AuScReport::CanBeManufactured() const
{
	return m_MfgProc.ManufacturingProcessId();
}

double AuScReport::EstimatedCost() const
{
	return m_MfgProc.EstimatedCost(m_Quality);
}

double AuScReport::PowerNeededToEngageFTLDrive(quint16 whichFTL, quint8 driveSetting) const
{
	AuScReportSysVolHash::const_iterator fi = m_FTLDrives.find(whichFTL);
	if(fi == m_FTLDrives.cend())
		return -1;

	const float tlBoost = (float)QualityBoostF((*fi).Quality());
	switch((*fi).Module())
	{
	case CID_WormholePFTLDriveModule:
	case CID_WormholePNFTLDriveModule:
	case CID_WormholePNNFTLDriveModule:
		return AuScDesign::PowerNeededToEngageFTLDrive(m_FTLDriveUnitsToEngage) / tlBoost;

	case CID_Warp1FTLDriveModule:
	case CID_Warp2FTLDriveModule:
	case CID_Warp3FTLDriveModule:
	case CID_Warp4FTLDriveModule:
	case CID_Warp5FTLDriveModule:
	case CID_Warp6FTLDriveModule:
	case CID_Warp7FTLDriveModule:
	case CID_Warp8FTLDriveModule:
	case CID_Warp9FTLDriveModule:
		// Warp drive has 9 warp factor settings.
		// Factor 0 must use the same amount of power as the wormhole drive, so module upgrades don't result in useless ships.
		return AuScDesign::PowerNeededToEngageFTLDrive(m_FTLDriveUnitsToEngage) * (driveSetting * 2 + 1) / tlBoost;

	default:
		break;
	}
	return -1;
}

void AuScReport::SetQuality(quint8 quality)
{
	Q_ASSERT(quality);
	// Sets the quality in all systems. Called when this report is storing information on a live spacecraft, that has just been
	// initialized.
	// 'quality' is expected to be no less than the current m_Quality, which is the max of all system qualities.
	if(quality < m_Quality)
		quality = m_Quality;

	m_Hull.SetQuality(quality);
	m_Bays.SetQuality(quality);
	m_Capacitor.SetQuality(quality);
	m_FTLDrives.SetQuality(quality);
	m_LifeSupport.SetQuality(quality);
	m_ManeuverDrive.SetQuality(quality);
	m_PowerPlant.SetQuality(quality);
	m_Sensor.SetQuality(quality);
	m_Shield.SetQuality(quality);
	m_Turrets.SetQuality(quality);
	m_Quality = quality;
	m_CnCQ = quality;
	m_CommandStations.SetQuality(quality);
	m_CrewStations.SetQuality(quality);
	m_Transporters.SetQuality(quality);
	m_DesignerStations.SetQuality(quality);
}

quint16 AuScReport::ClosestTransporterInRoom(const AuScDesign& scd, quint16 roomId, const AuPoint& loc) const
{
	quint16 closest = 0;
	double closestDist2 = 0;
	for(AuScReportTransporterStationHash::const_iterator ti = m_Transporters.cbegin(); ti != m_Transporters.cend(); ti++)
	{
		if((*ti).RoomId() != roomId)
			continue;
		AuPoint tloc = scd.OperatorLocation(ti.key());
		double dist2 = tloc.Distance2(loc);
		if(!closest || dist2 < closestDist2)
		{
			closest = ti.key();
			closestDist2 = dist2;
		}
	}
	return closest;
}

quint16 AuScReport::TurboLiftStopDoor(quint16 liftId, const QString& stopName) const
{
	AuScReportLiftHash::const_iterator li = m_Lifts.find(liftId);
	if(li == m_Lifts.cend())
		return 0;	// No lift with that id.

	AuLiftStopHash::const_iterator si = (*li).find(stopName);
	if(si == (*li).cend())
		return 0;	// No stop with that name.

	return (*si).DoorId();
}

quint16 AuScReport::ClosestRemoteDoor(bool forVehicle, const AuPoint& loc, const AuDoorStateHash& doorStates) const
{
	double dist;
	return doorStates.ClosestRemoteDoor(forVehicle, loc, dist);
}

void AuScReport::ModuleInstalled(quint16 which, COMMODITYID moduleCID, quint8 moduleQ)
{
	Q_ASSERT(moduleQ);
	switch(moduleCID)
	{
	default:
		return;
	case CID_MetalHullModule:
	case CID_MagmiumHullModule:
	case CID_VulciumHullModule:
	case CID_AdamantineHullModule:
		m_Hull.SetModule(moduleCID);
		m_Hull.SetQuality(moduleQ);
		break;
	case CID_AtmosphereCondenserBayModule:
	case CID_ParticleCollectorBayModule:
	case CID_ProspectorBayModule:
	case CID_SiphonBayModule:
	case CID_WildcatterBayModule:
	case CID_TractorBeamBayModule:
	case CID_EnergyWeaponBayModule:
	case CID_WeaponDeploymentBayModule:
	case CID_BallisticWeaponBayModule:
	case CID_WeaponLauncherBayModule:
		{
			AuScReportSysVolHash::iterator bi = m_Bays.find(which);
			if(bi != m_Bays.end())
			{
				(*bi).SetModule(moduleCID);
				(*bi).SetQuality(moduleQ);
			}
		}
		break;
	case CID_CapacitorModule:
		m_Capacitor.SetModule(moduleCID);
		m_Capacitor.SetQuality(moduleQ);
		break;
	case CID_WormholePFTLDriveModule:
	case CID_WormholePNFTLDriveModule:
	case CID_WormholePNNFTLDriveModule:
	case CID_Warp1FTLDriveModule:
	case CID_Warp2FTLDriveModule:
	case CID_Warp3FTLDriveModule:
	case CID_Warp4FTLDriveModule:
	case CID_Warp5FTLDriveModule:
	case CID_Warp6FTLDriveModule:
	case CID_Warp7FTLDriveModule:
	case CID_Warp8FTLDriveModule:
	case CID_Warp9FTLDriveModule:
		{
			AuScReportSysVolHash::iterator fi = m_FTLDrives.find(which);
			if(fi != m_FTLDrives.end())
			{
				(*fi).SetModule(moduleCID);
				(*fi).SetQuality(moduleQ);
			}
		}
		break;
	case CID_LifeSupportModule:
	case CID_LifeSupportGravityModule:
		m_LifeSupport.SetModule(moduleCID);
		m_LifeSupport.SetQuality(moduleQ);
		break;
	case CID_GravityDriveModule:
	case CID_RocketDriveModule:
		m_ManeuverDrive.SetModule(moduleCID);
		m_ManeuverDrive.SetQuality(moduleQ);
		break;
	case CID_HydrogenPowerPlantModule:
		m_PowerPlant.SetModule(moduleCID);
		m_PowerPlant.SetQuality(moduleQ);
		break;
	case CID_SensorModule:
		m_Sensor.SetModule(moduleCID);
		m_Sensor.SetQuality(moduleQ);
		break;
	case CID_EnergyShieldModule:
	case CID_KineticShieldModule:
	case CID_CloakingShieldModule:
		m_Shield.SetModule(moduleCID);
		m_Shield.SetQuality(moduleQ);
		break;
	case CID_BallisticWeaponTurretModule:
	case CID_EnergyWeaponTurretModule:
	case CID_WeaponLauncherTurretModule:
		{
			AuScReportTurretHash::iterator ti = m_Turrets.find(which);
			if(ti != m_Turrets.end())
			{
				(*ti).SetModule(moduleCID);
				(*ti).SetQuality(moduleQ);
			}
		}
		break;
	case CID_CommandAndControlModule:
		m_CnCModule = moduleCID;
		m_CnCQ = moduleQ;
		break;
	case CID_SurgeryMedicalModule:
	case CID_GeneticsMedicalModule:
	case CID_CyberneticsMedicalModule:
		{
			AuScReportStationHash::iterator si = m_CrewStations.find(which);
			if(si != m_CrewStations.end())
			{
				(*si).SetModule(moduleCID);
				(*si).SetQuality(moduleQ);
			}
		}
		break;
	case CID_PersonnelTransporterModule:
	case CID_VehicleTransporterModule:
		{
			AuScReportTransporterStationHash::iterator ti = m_Transporters.find(which);
			if(ti != m_Transporters.end())
			{
				(*ti).SetModule(moduleCID);
				(*ti).SetQuality(moduleQ);
			}
		}
		break;
	}

	if(moduleQ > m_Quality)
		m_Quality = moduleQ;

	// Update values that are determined by analysis of the design.
	UpdateCalculatedValues();
}

quint16 AuScReport::VehicleComplement(VEHICLE_SPOT_TYPE spotType) const
{
	switch(spotType)
	{
	case SPOT_Invalid:
	case SPOT_Count:
	default:
		return 0;
	case SPOT_GroundVehicle:
		return m_ParkGround;
	case SPOT_Rocket:
		return m_ParkRocket;
	case SPOT_SpaceVehicle:
		return m_ParkSpace;
	case SPOT_WaterVehicle:
		return m_ParkWater;
	}
}

void AuScReport::HullVolumeDistribution(quint32& hullVolume, quint32& armor, quint32& capacitor, quint32& cryoBerths, quint32& fuel,
		quint32& hold, quint32& lifeSupport, quint32& maneuverDrive, quint32& powerPlant, quint32& sensor, bool& hasSensor,
		quint32& shield, bool& hasShield, QMap<quint16, AuScReportSysVol>& ftlDrives, QMap<quint16, AuScReportSysVol>& bays,
		quint32& skeleton) const
{
	hullVolume = HullVolume();
	armor = m_ArmorVolume;
	capacitor = m_Capacitor.Volume();
	cryoBerths = m_CryoBerthVolume;
	fuel = m_FuelVolume;
	hold = m_HoldVolume;
	lifeSupport = m_LifeSupport.Volume();
	maneuverDrive = m_ManeuverDrive.Volume();
	powerPlant = m_PowerPlant.Volume();
	hasSensor = HasSensor();
	if(hasSensor)
		sensor = m_Sensor.Volume();
	else
		sensor = 0;
	hasShield = HasShield();
	if(hasShield)
		shield = m_Shield.Volume();
	else
		shield = 0;
	for(AuScReportSysVolHash::const_iterator fi = m_FTLDrives.cbegin(); fi != m_FTLDrives.cend(); fi++)
		ftlDrives.insert(fi.key(), *fi);
	for(AuScReportSysVolHash::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
		bays.insert(bi.key(), *bi);
	skeleton = m_SkeletonVolume;
}

// 'volume' is in cubic meters.
void AuScReport::SetArmorVolume(quint32 volume)
{
	m_ArmorVolume = volume;
}

// 'volume' is in cubic meters.
void AuScReport::SetBayVolume(quint16 partId, quint32 volume)
{
	AuScReportSysVolHash::iterator bi = m_Bays.find(partId);
	if(bi != m_Bays.end())
		(*bi).SetVolume(volume);
}

// 'volume' is in cubic meters.
void AuScReport::SetCapacitorVolume(quint32 volume)
{
	m_Capacitor.SetVolume(volume);
}

// 'volume' is in cubic meters.
void AuScReport::SetCryoBerthVolume(quint32 volume)
{
	m_CryoBerthVolume = volume;
}

// 'volume' is in cubic meters.
void AuScReport::SetFTLDriveVolume(quint16 partId, quint32 volume)
{
	AuScReportSysVolHash::iterator fi = m_FTLDrives.find(partId);
	if(fi != m_FTLDrives.end())
		(*fi).SetVolume(volume);
}

// 'volume' is in cubic meters.
void AuScReport::SetFuelVolume(quint32 volume)
{
	m_FuelVolume = volume;
}

// 'volume' is in cubic meters.
void AuScReport::SetHoldVolume(quint32 volume)
{
	m_HoldVolume = volume;
}

// 'volume' is in cubic meters.
void AuScReport::SetSkeletonVolume(quint32 volume)
{
	m_SkeletonVolume = volume;
}

// 'volume' is in cubic meters.
void AuScReport::SetLifeSupportVolume(quint32 volume)
{
	m_LifeSupport.SetVolume(volume);
}

// 'volume' is in cubic meters.
void AuScReport::SetManeuverDriveVolume(quint32 volume)
{
	m_ManeuverDrive.SetVolume(volume);
}

// 'volume' is in cubic meters.
void AuScReport::SetPowerPlantVolume(quint32 volume)
{
	m_PowerPlant.SetVolume(volume);
}

// 'volume' is in cubic meters.
void AuScReport::SetSensorVolume(quint32 volume)
{
	m_Sensor.SetVolume(volume);
}

// 'volume' is in cubic meters.
void AuScReport::SetShieldVolume(quint32 volume)
{
	m_Shield.SetVolume(volume);
}

void AuScReport::UpdateCalculatedValues()
{
	// Mass
	m_MassEquipmentKg = round(CalcEquipmentMass() * KilogramsPerPound());
	m_MassFuelKg = round(CalcFuelMass() * KilogramsPerPound());
	m_MassHullKg = round(CalcHullMass() * KilogramsPerPound());
	m_MassKg = m_MassEquipmentKg + m_MassFuelKg + m_MassHullKg;

	// Analysis
	m_Acceleration = CalcManeuverAcceleration();
	m_AccelerationVert = m_Acceleration * .16;	// .16 is the acceleration at max vertical throttle. See DaSc::Acceleration.
	m_CapacitorMax = CalcCapacitorMax();
	m_FTLDriveUnitsToEngage = CalcFTLDriveUnitsToEngage();
	m_FuelBurnRate = CalcFuelBurnRate();
	m_FuelCapacity = CalcFuelCapacity();
	m_HoldCapacity = CalcHoldCapacity();
	m_HoldPowerNoShields = CalcPowerUsageHolding(false);
	m_HoldPowerWithShields = CalcPowerUsageHolding(true);
	m_IdlePowerNoShields = CalcPowerUsageIdling(false);
	m_IdlePowerWithShields = CalcPowerUsageIdling(true);

	double lifeSupportNeeded = CalcLifeSupportNeeded();
	double lscuft = LifeSupportVolume() * CubicFeetPerCubicMeter();
	if(lscuft < lifeSupportNeeded)
		m_LifeSupportNeeded = round((lifeSupportNeeded - lscuft) * CubicMetersPerCubicFoot());
	else
		m_LifeSupportNeeded = 0;

	m_ManeuverFuelRate = CalcManeuverFuelRate();
	m_SensorRange = CalcSensorRange();
	m_TurnRate = CalcTurnRate();
}

double AuScReport::CalcEquipmentMass() const
{
	double mass = 0;

	// These values were taken from the old n_DetailType table, which expressed them in kg per unit.
	// e.g. A maneuver drive unit was 2250 kg. All units were 256 cubic feet.
	// Mass due to maneuver drives.
	if(ManeuverDriveVolume())
	{
		double mu = (2250 * PoundsPerKilogram()) / 256.;
		mass += mu * ManeuverDriveVolume() * CubicFeetPerCubicMeter();
	}

	if(PowerPlantVolume())
	{
		double mu = (800 * PoundsPerKilogram()) / 256.;
		mass += mu * PowerPlantVolume() * CubicFeetPerCubicMeter();
	}

	if(CapacitorVolume())
	{
		double mu = (800 * PoundsPerKilogram()) / 256.;
		mass += mu * CapacitorVolume() * CubicFeetPerCubicMeter();
	}

	quint32 bay = BayVolume();
	if(bay)
	{
		double mu = (1800 * PoundsPerKilogram()) / 256.;
		mass += mu * bay * CubicFeetPerCubicMeter();
	}

	quint32 ftl = FTLDriveVolume();
	if(ftl)
	{
		double mu = (700 * PoundsPerKilogram()) / 256.;
		mass += mu * ftl * CubicFeetPerCubicMeter();
	}

	if(SensorVolume())
	{
		double mu = (450 * PoundsPerKilogram()) / 256.;
		mass += mu * SensorVolume() * CubicFeetPerCubicMeter();
	}

	if(ShieldVolume())
	{
		double mu = (450 * PoundsPerKilogram()) / 256.;
		mass += mu * ShieldVolume() * CubicFeetPerCubicMeter();
	}

	return mass;
}

double AuScReport::CalcFuelMass() const
{
	return AuScDesign::FuelDensity() * FuelVolume() * CubicFeetPerCubicMeter();
}

double AuScReport::CalcHullMass() const
{
	return AuScDesign::HullDensity(ArmorLevel(), Streamline()) * HullVolume() * CubicFeetPerCubicMeter();
}

double AuScReport::CalcCapacitorMax() const
{
	// These values were taken from the old DqDesignAnalysis, which expressed everything in system units.
	// All of the old system units were 256 cubic feet.
	const double tlBoost = QualityBoostF(m_Capacitor.Quality());
	return (CapacitorVolume() * CubicFeetPerCubicMeter()) / 256. * tlBoost;
}

// Returns the number of cubic feet of FTL drive units needed to engage.
double AuScReport::CalcFTLDriveUnitsToEngage() const
{
	// These values were taken from the old DqDesignAnalysis, which expressed everything in system units.
	// All of the old system units were 256 cubic feet.
	return pow(.75 * HullVolume() * CubicFeetPerCubicMeter() * PI, 0.33333333) * 256.;
}

// Returns power plant fuel burn rate in cubic feet per real world second.
// produced from each cubic foot of fuel that is burned.
double AuScReport::CalcFuelBurnRate() const
{
	// Calculate burn rate in cubic feet per real second.
	// A drive unit is 256 cubic feet.
	const double fpuday = 24.;	// Power plant fuel burn per Universe day.
	const double rUnitRate = (double)UDaysPerRealDay() / (24. * 3600. / fpuday) / 256.;
	const double rBurnRate = PowerPlantVolume() * CubicFeetPerCubicMeter() * rUnitRate;
	return rBurnRate;
}

quint32 AuScReport::CalcFuelCapacity() const
{
	// Returns the number of commodity units that can fit in the fuel cells.
	if(!FuelVolume())
		return 0;						// No fuel cell space at all.

	// Calculate this the way the old ship designs did it, based on cubic feet.
	quint32 fuelVol = (quint32)(FuelVolume() * CubicFeetPerCubicMeter());
	if(fuelVol < 1)
		return 1;
	quint32 fuel = fuelVol;
	return fuel;
}

quint32 AuScReport::CalcHoldCapacity() const
{
	// Returns the number of commodity units that can fit in the hold.
	if(!HoldVolume())
		return 0;						// No hold space at all.

	// Calculate this the way the old ship designs did it, based on cubic feet.
	quint32 holdVol = (quint32)(HoldVolume() * CubicFeetPerCubicMeter());
	if(holdVol < 3)
		return 1;
	quint32 hold = holdVol / 3;
	return hold;
}

// Power usage per real world second when holding
// - Gravity drives on HOLD
// - Life Support on
// - Sensors on
// - Shields on
double AuScReport::CalcPowerUsageHolding(bool shieldsUp) const
{
	const double life = LifeSupportPowerRate();
	const double maneuver = ManeuverPowerRateHolding();
	const double sensor = SensorPowerRate();
	const double shield = shieldsUp ? ShieldPowerRate() : 0;
	const double idlepower = life + maneuver + sensor + shield;
	return idlepower;
}

// Power usage per real world second when idle
// - Gravity drives on
// - Life Support on
// - Sensors on
// - Shields on
double AuScReport::CalcPowerUsageIdling(bool shieldsUp) const
{
	const double life = LifeSupportPowerRate();
	const double maneuver = ManeuverPowerRate();
	const double sensor = SensorPowerRate();
	const double shield = shieldsUp ? ShieldPowerRate() : 0;
	const double idlepower = life + maneuver + sensor + shield;
	return idlepower;
}

// Returns cubic feet of life support needed.
double AuScReport::CalcLifeSupportNeeded() const
{
	// A spacecraft must have certain minimum components.
	int berths = CaptBerths() + CrewBerths() + OfficerBerths() + PassengerBerths() + TroopBerths();
	const double tlBoost = QualityBoostF(m_LifeSupport.Quality());
	double lifeSupportNeeded = berths * CubicFeetPerCubicMeter() / tlBoost;		// 1 cubic meter of ls per berth.
	if(lifeSupportNeeded < CubicFeetPerCubicMeter())
		lifeSupportNeeded = CubicFeetPerCubicMeter();	// At least one cubic meter of life support is needed.
	return lifeSupportNeeded;
}

double AuScReport::CalcManeuverAcceleration() const
{
	if(!HullVolume())
		return 0;
	const double tlBoost = QualityBoostF(m_ManeuverDrive.Quality());
	const double mass = CalcEquipmentMass() + CalcFuelMass() + CalcHullMass();	// Total mass in pounds.
	return (ManeuverDriveVolume() * CubicFeetPerCubicMeter() * AuScDesign::ManeuverDriveForce()) / mass * tlBoost; 
}

// Returns maneuver drive fuel use rate in units per real world second.
// Only rocket drives use fuel.
double AuScReport::CalcManeuverFuelRate() const
{
	// Rocket drives do not use power.
	if(m_ManeuverDrive.Module() != CID_RocketDriveModule)
		return 0.;

	// Each maneuver drive uses one power unit per U day.
	// Calculate use rate in power units per real second.
	// A drive unit is 256 cubic feet.
	const double rUnitRate = (double)UDaysPerRealDay() / (24. * 3600.) / 256.;
	const double rUseRate = (ManeuverDriveVolume() * CubicFeetPerCubicMeter()) * rUnitRate / PowerPerFuel();
	return rUseRate;
}

double AuScReport::CalcSensorRange() const
{
	if(SensorVolume() <= 0)
		return 0;

	// A sensor unit is 256 cubic feet.
	const double tlBoost = QualityBoostF(m_Sensor.Quality());
	return (SensorVolume() * CubicFeetPerCubicMeter())/256. * FeetPerMoonOrbit() / 2. * tlBoost;
}

// Turn performance in radians/sec
double AuScReport::CalcTurnRate() const
{
	//DHH calculate this so the farthest point does not move faster than 1000fps.
	double rmaneuver = ManeuverAcceleration();
	double rdegpersec = rmaneuver / 25.;
	double rturnrate = rdegpersec * DEG2RAD;
	return rturnrate;
}

quint32 AuScReport::BayVolume() const
{
	quint32 v = 0;
	for(QHash<quint16, AuScReportSysVol>::const_iterator bi = m_Bays.begin(); bi != m_Bays.end(); bi++)
		v += (*bi).Volume();
	return v;
}

quint32 AuScReport::FTLDriveVolume() const
{
	quint32 v = 0;
	for(QHash<quint16, AuScReportSysVol>::const_iterator fi = m_FTLDrives.begin(); fi != m_FTLDrives.end(); fi++)
		v += (*fi).Volume();
	return v;
}

bool AuScReport::SetBayVolumes(const QMap<quint16, quint32>& bays)
{
	bool anyChange = false;
	for(QMap<quint16, quint32>::const_iterator bi = bays.cbegin(); bi != bays.cend(); bi++)
	{
		AuScReportSysVolHash::iterator si = m_Bays.find(bi.key());
		if(si == m_Bays.end())
			continue;
		if((*si).Volume() == *bi)
			continue;
		(*si).SetVolume(*bi);
		anyChange = true;
	}
	return anyChange;
}

bool AuScReport::SetFTLDriveVolumes(const QMap<quint16, quint32>& ftlDrives)
{
	bool anyChange = false;
	for(QMap<quint16, quint32>::const_iterator fi = ftlDrives.cbegin(); fi != ftlDrives.cend(); fi++)
	{
		AuScReportSysVolHash::iterator si = m_FTLDrives.find(fi.key());
		if(si == m_FTLDrives.end())
			continue;
		if((*si).Volume() == *fi)
			continue;
		(*si).SetVolume(*fi);
		anyChange = true;
	}
	return anyChange;
}

// NOTE: If you change this function be sure to change AuScDesign::ListComponents.
void AuScReport::ListComponents(DeProcessComponentArray& pc, const AuScParts& parts) const
{
	int commandStations = m_CommandStations.size() + m_CrewStations.size();
	if(commandStations)
		pc.AddRequirement(CommandAndControlModule(), CE_Required, commandStations);

	int lifeSupportUnits = (int)ceil(m_LifeSupport.Volume() * CubicFeetPerCubicMeter() / 256.f);
	pc.AddRequirement(m_LifeSupport.Module(), CE_Required, lifeSupportUnits);

	int driveUnits = (int)ceil(m_ManeuverDrive.Volume() * CubicFeetPerCubicMeter() / 256.f);
	pc.AddRequirement(m_ManeuverDrive.Module(), CE_Required, driveUnits);

	int powerPlantUnits = (int)ceil(m_PowerPlant.Volume() * CubicFeetPerCubicMeter() / 256.f);
	pc.AddRequirement(m_PowerPlant.Module(), CE_Required, powerPlantUnits);

	int capacitorUnits = (int)ceil(m_Capacitor.Volume() * CubicFeetPerCubicMeter() / 256.f);
	pc.AddRequirement(m_Capacitor.Module(), CE_Required, capacitorUnits);

	int sensorUnits = (int)ceil(m_Sensor.Volume() * CubicFeetPerCubicMeter() / 256.f);
	if(sensorUnits)
		pc.AddRequirement(m_Sensor.Module(), CE_Required, sensorUnits);

	int shieldUnits = (int)ceil(m_Shield.Volume() * CubicFeetPerCubicMeter() / 256.f);
	if(shieldUnits)
		pc.AddRequirement(m_Shield.Module(), CE_Required, shieldUnits);

	for(QHash<quint16, AuScReportSysVol>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
	{
		int bayHits = (int)ceil((*bi).Volume() * CubicFeetPerCubicMeter() / 256.f);
		COMMODITYID bmod = (*bi).Module();
		pc.AddRequirement(bmod, CE_Required, bayHits);
	}

	for(QHash<quint16, AuScReportSysVol>::const_iterator bi = m_FTLDrives.cbegin(); bi != m_FTLDrives.cend(); bi++)
	{
		int ftlHits = (int)ceil((*bi).Volume() * CubicFeetPerCubicMeter() / 256.f);
		COMMODITYID bmod = (*bi).Module();
		pc.AddRequirement(bmod, CE_Required, ftlHits);
	}

	parts.ListComponents(pc, Streamline(), ArmorLevel(), HullMaterial());
}

quint8 AuScReport::BayQ() const
{
	quint8 qual = 0;
	for(QHash<quint16, AuScReportSysVol>::const_iterator bi = m_Bays.begin(); bi != m_Bays.end(); bi++)
		if(qual < (*bi).Quality())
			qual = (*bi).Quality();
	return qual;
}

quint8 AuScReport::FTLDriveQ() const
{
	quint8 qual = 0;
	for(QHash<quint16, AuScReportSysVol>::const_iterator fi = m_FTLDrives.begin(); fi != m_FTLDrives.end(); fi++)
		if(qual < (*fi).Quality())
			qual = (*fi).Quality();
	return qual;
}

double AuScReport::RoughHullRadius() const
{
	double max2 = m_HullMax.Length2();
	double min2 = m_HullMin.Length2();
	if(max2 > min2)
		return sqrt(max2);
	else
		return sqrt(min2);
}

bool AuScReport::EnginesRequireEngineer() const
{
	return (m_PowerPlant.Volume() + m_ManeuverDrive.Volume()) > 100;
}

quint64 AuScReport::MemoryUsage() const
{
	quint64 mem = 0;
	mem += sizeof(m_ArchitectId);
	mem += StringMemoryUsage(m_ArchitectName);
	mem += StringMemoryUsage(m_DesignDesc);
	mem += sizeof(m_DesignId);
	mem += StringMemoryUsage(m_DesignName);
	mem += m_PartArchitects.MemoryUsage();

	mem += sizeof(m_LandingOrient);
	mem += sizeof(m_StationOrient);
	mem += sizeof(m_Streamline);
	mem += sizeof(m_Structure);

	mem += sizeof(m_Hull);
	mem += sizeof(m_HullMax);
	mem += sizeof(m_HullMin);
	mem += sizeof(m_ArmorVolume);
	mem += m_Bays.MemoryUsage();
	mem += sizeof(m_Capacitor);
	mem += sizeof(m_CryoBerthVolume);
	mem += m_FTLDrives.MemoryUsage();
	mem += sizeof(m_FuelVolume);
	mem += sizeof(m_HoldVolume);
	mem += sizeof(m_LifeSupport);
	mem += sizeof(m_ManeuverDrive);
	mem += sizeof(m_PowerPlant);
	mem += sizeof(m_Sensor);
	mem += sizeof(m_Shield);
	mem += sizeof(m_SkeletonVolume);
	mem += m_Turrets.MemoryUsage();

	mem += sizeof(m_Quality);
	mem += sizeof(m_CnCModule);
	mem += sizeof(m_CnCQ);
	mem += m_MfgProc.MemoryUsage();

	mem += m_CitizenPosts.MemoryUsage();
	mem += m_CommandStations.MemoryUsage();
	mem += m_CrewStations.MemoryUsage();
	mem += m_DesignerStations.MemoryUsage();
	mem += m_GameTables.MemoryUsage();
	mem += m_GuardPosts.MemoryUsage();
	mem += m_Lifts.MemoryUsage();
	mem += m_LivestockPosts.MemoryUsage();
	mem += m_ServicePanels.MemoryUsage();
	mem += m_StatusDisplays.MemoryUsage();
	mem += m_Transporters.MemoryUsage();
	mem += m_VehicleLandingSystems.MemoryUsage();
	mem += m_VehicleLaunchers.MemoryUsage();

	mem += sizeof(m_CaptBerths);
	mem += sizeof(m_CrewBerths);
	mem += sizeof(m_OfficerBerths);
	mem += sizeof(m_PassengerBerths);
	mem += sizeof(m_TroopBerths);

	mem += sizeof(m_ParkGround);
	mem += sizeof(m_ParkRocket);
	mem += sizeof(m_ParkSpace);
	mem += sizeof(m_ParkSpacecraft);
	mem += sizeof(m_ParkWater);

	mem += sizeof(m_MassKg);
	mem += sizeof(m_MassEquipmentKg);
	mem += sizeof(m_MassFuelKg);
	mem += sizeof(m_MassHullKg);

	mem += sizeof(m_Acceleration);
	mem += sizeof(m_AccelerationVert);
	mem += sizeof(m_CapacitorMax);
	mem += sizeof(m_FaceCount);
	mem += sizeof(m_FTLDriveUnitsToEngage);
	mem += sizeof(m_FuelBurnRate);
	mem += sizeof(m_FuelCapacity);
	mem += sizeof(m_HasBar);
	mem += sizeof(m_HasPantry);
	mem += sizeof(m_HoldCapacity);
	mem += sizeof(m_HoldPowerNoShields);
	mem += sizeof(m_HoldPowerWithShields);
	mem += sizeof(m_IdlePowerNoShields);
	mem += sizeof(m_IdlePowerWithShields);
	mem += sizeof(m_LifeSupportNeeded);
	mem += sizeof(m_ManeuverFuelRate);
	mem += StringMemoryUsage(m_PartsList);
	mem += sizeof(m_SensorRange);
	mem += sizeof(m_TriangleCount);
	mem += sizeof(m_TurnRate);
	return mem;
}

double AuScReport::SensorRangeDeepSpace() const
{
	return AuDesign::SensorRangeDeepSpace(m_SensorRange);
}

bool AuScReport::DowngradeSystemModules(quint8 repairQ, const QSet<AuSysId>& systemsRepaired)
{
	if(!repairQ)
		repairQ = 1;
	bool anyChange = false;
	for(QSet<AuSysId>::const_iterator ri = systemsRepaired.cbegin(); ri != systemsRepaired.cend(); ri++)
	{
		switch((*ri).System())
		{
		case SYS_CommandAndControl:
			if(m_CnCQ > repairQ)
			{
				m_CnCQ = repairQ;
				anyChange = true;
			}
			break;

		case SYS_Fuel:
		case SYS_Hold:
		case SYS_Hull:
			if(m_Hull.Quality() > repairQ)
			{
				m_Hull.SetQuality(repairQ);
				anyChange = true;
			}
			break;

		case SYS_ManeuverDrive:
			if(m_ManeuverDrive.Quality() > repairQ)
			{
				m_ManeuverDrive.SetQuality(repairQ);
				anyChange = true;
			}
			break;

		case SYS_LifeSupport:
			if(m_LifeSupport.Quality() > repairQ)
			{
				m_LifeSupport.SetQuality(repairQ);
				anyChange = true;
			}
			break;

		case SYS_PowerPlant:
			if(m_PowerPlant.Quality() > repairQ)
			{
				m_PowerPlant.SetQuality(repairQ);
				anyChange = true;
			}
			break;

		case SYS_Sensor:
			if(m_Sensor.Quality() > repairQ)
			{
				m_Sensor.SetQuality(repairQ);
				anyChange = true;
			}
			break;

		case SYS_Shield:
			if(m_Shield.Quality() > repairQ)
			{
				m_Shield.SetQuality(repairQ);
				anyChange = true;
			}
			break;

		case SYS_SurgeryUnit:
			if(SurgeryUnitQuality((*ri).Which()) > repairQ)
			{
				SetSurgeryUnitQuality((*ri).Which(), repairQ);
				anyChange = true;
			}
			break;

		case SYS_Transporter:
			if(TransporterQuality((*ri).Which()) > repairQ)
			{
				SetTransporterQuality((*ri).Which(), repairQ);
				anyChange = true;
			}
			break;

		case SYS_Turret:
			if(TurretQuality((*ri).Which()) > repairQ)
			{
				SetTurretQuality((*ri).Which(), repairQ);
				anyChange = true;
			}
			break;

		case SYS_Weapon:
			if(WeaponBayQuality((*ri).Which()) > repairQ)
			{
				SetWeaponBayQuality((*ri).Which(), repairQ);
				anyChange = true;
			}
			break;

		case SYS_FTLDrive:
			if(FTLDriveQuality((*ri).Which()) > repairQ)
			{
				SetFTLDriveQuality((*ri).Which(), repairQ);
				anyChange = true;
			}
			break;

		case SYS_Capacitor:
			if(m_Capacitor.Quality() > repairQ)
			{
				m_Capacitor.SetQuality(repairQ);
				anyChange = true;
			}
			break;

		case SYS_VehicleLaunch:
		case SYS_VehicleLanding:
		case SYS_Count:
			break;
		}
	}
	return anyChange;
}

float AuScReport::SkeletonLevel() const
{
	if(!m_Hull.Volume())
		return 0;
	return (float)m_SkeletonVolume / (float)m_Hull.Volume();
}

// 'volume' is in cubic meters.
void AuScReport::RefitArmorVolume(quint32 volume, quint8 minQ)
{
	if(m_ArmorVolume == volume)
		return;
	if(m_ArmorVolume < volume && m_Hull.Quality() > minQ)
		m_Hull.SetQuality(minQ);	// Volume increased so Q was reduced.
	m_ArmorVolume = volume;
}

// 'volume' is in cubic meters.
void AuScReport::RefitBayVolume(quint16 partId, quint32 volume, quint8 minQ)
{
	AuScReportSysVolHash::iterator bi = m_Bays.find(partId);
	if(bi != m_Bays.end() && (*bi).Volume() != volume)
	{
		if((*bi).Volume() < volume && (*bi).Quality() > minQ)
			(*bi).SetQuality(minQ);	// Volume increased so Q was reduced.
		(*bi).SetVolume(volume);
	}
}

bool AuScReport::RefitBayVolumes(const QMap<quint16, quint32>& bays, quint8 minQ)
{
	bool anyChange = false;
	for(QMap<quint16, quint32>::const_iterator bi = bays.cbegin(); bi != bays.cend(); bi++)
	{
		AuScReportSysVolHash::iterator si = m_Bays.find(bi.key());
		if(si == m_Bays.end())
			continue;
		if((*si).Volume() == *bi)
			continue;
		if((*si).Volume() < *bi && (*si).Quality() > minQ)
			(*si).SetQuality(minQ);	// Volume increased so Q was reduced.
		(*si).SetVolume(*bi);
		anyChange = true;
	}
	return anyChange;
}

// 'volume' is in cubic meters.
void AuScReport::RefitCapacitorVolume(quint32 volume, quint8 minQ)
{
	if(m_Capacitor.Volume() == volume)
		return;
	if(m_Capacitor.Volume() < volume && m_Capacitor.Quality() > minQ)
		m_Capacitor.SetQuality(minQ);	// Volume increased so Q was reduced.
	m_Capacitor.SetVolume(volume);
}

// 'volume' is in cubic meters.
void AuScReport::RefitCryoBerthVolume(quint32 volume, quint8 minQ)
{
	Q_UNUSED(minQ);
	m_CryoBerthVolume = volume;
}

// 'volume' is in cubic meters.
void AuScReport::RefitFTLDriveVolume(quint16 partId, quint32 volume, quint8 minQ)
{
	AuScReportSysVolHash::iterator fi = m_FTLDrives.find(partId);
	if(fi != m_FTLDrives.end() && (*fi).Volume() != volume)
	{
		if((*fi).Volume() < volume && (*fi).Quality() > minQ)
			(*fi).SetQuality(minQ);	// Volume increased so Q was reduced.
		(*fi).SetVolume(volume);
	}
}

bool AuScReport::RefitFTLDriveVolumes(const QMap<quint16, quint32>& ftlDrives, quint8 minQ)
{
	bool anyChange = false;
	for(QMap<quint16, quint32>::const_iterator fi = ftlDrives.cbegin(); fi != ftlDrives.cend(); fi++)
	{
		AuScReportSysVolHash::iterator si = m_FTLDrives.find(fi.key());
		if(si == m_FTLDrives.end())
			continue;
		if((*si).Volume() == *fi)
			continue;
		if((*si).Volume() < *fi && (*si).Quality() > minQ)
			(*si).SetQuality(minQ);	// Volume increased so Q was reduced.
		(*si).SetVolume(*fi);
		anyChange = true;
	}
	return anyChange;
}

// 'volume' is in cubic meters.
void AuScReport::RefitFuelVolume(quint32 volume, quint8 minQ)
{
	if(m_FuelVolume == volume)
		return;
	if(m_FuelVolume < volume && m_Hull.Quality() > minQ)
		m_Hull.SetQuality(minQ);	// Volume increased so Q was reduced.
	m_FuelVolume = volume;
}

// 'volume' is in cubic meters.
void AuScReport::RefitHoldVolume(quint32 volume, quint8 minQ)
{
	if(m_HoldVolume == volume)
		return;
	if(m_HoldVolume < volume && m_Hull.Quality() > minQ)
		m_Hull.SetQuality(minQ);	// Volume increased so Q was reduced.
	m_HoldVolume = volume;
}

// 'volume' is in cubic meters.
void AuScReport::RefitSkeletonVolume(quint32 volume, quint8 minQ)
{
	if(m_SkeletonVolume == volume)
		return;
	if(m_SkeletonVolume < volume && m_Hull.Quality() > minQ)
		m_Hull.SetQuality(minQ);	// Volume increased so Q was reduced.
	m_SkeletonVolume = volume;
}

// 'volume' is in cubic meters.
void AuScReport::RefitLifeSupportVolume(quint32 volume, quint8 minQ)
{
	if(m_LifeSupport.Volume() == volume)
		return;
	if(m_LifeSupport.Volume() < volume && m_LifeSupport.Quality() > minQ)
		m_LifeSupport.SetQuality(minQ);	// Volume increased so Q was reduced.
	m_LifeSupport.SetVolume(volume);
}

// 'volume' is in cubic meters.
void AuScReport::RefitManeuverDriveVolume(quint32 volume, quint8 minQ)
{
	if(m_ManeuverDrive.Volume() == volume)
		return;
	if(m_ManeuverDrive.Volume() < volume && m_ManeuverDrive.Quality() > minQ)
		m_ManeuverDrive.SetQuality(minQ);	// Volume increased so Q was reduced.
	m_ManeuverDrive.SetVolume(volume);
}

// 'volume' is in cubic meters.
void AuScReport::RefitPowerPlantVolume(quint32 volume, quint8 minQ)
{
	if(m_PowerPlant.Volume() == volume)
		return;
	if(m_PowerPlant.Volume() < volume && m_PowerPlant.Quality() > minQ)
		m_PowerPlant.SetQuality(minQ);	// Volume increased so Q was reduced.
	m_PowerPlant.SetVolume(volume);
}

// 'volume' is in cubic meters.
void AuScReport::RefitSensorVolume(quint32 volume, quint8 minQ)
{
	if(m_Sensor.Volume() == volume)
		return;
	if(m_Sensor.Volume() < volume && m_Sensor.Quality() > minQ)
		m_Sensor.SetQuality(minQ);	// Volume increased so Q was reduced.
	m_Sensor.SetVolume(volume);
}

// 'volume' is in cubic meters.
void AuScReport::RefitShieldVolume(quint32 volume, quint8 minQ)
{
	if(m_Shield.Volume() == volume)
		return;
	if(m_Shield.Volume() < volume && m_Shield.Quality() > minQ)
		m_Shield.SetQuality(minQ);	// Volume increased so Q was reduced.
	m_Shield.SetVolume(volume);
}

quint32 AuScReport::SystemHits(const AuSysId& sysId) const
{
	switch(sysId.System())
	{
	case SYS_CommandAndControl:
		if(sysId.Which())
			return 1;
		return CommandStationCount() + CrewStationCount();
	case SYS_Fuel:
		return FuelHits();
	case SYS_ManeuverDrive:
		return ManeuverDriveHits();
	case SYS_Hold:
		return HoldHits();
	case SYS_Hull:
		return HullSystemHits();
	case SYS_LifeSupport:
		return LifeSupportHits();
	case SYS_PowerPlant:
		return PowerPlantHits();
	case SYS_Sensor:
		return SensorHits();
	case SYS_Shield:
		return ShieldHits();
	case SYS_SurgeryUnit:
		if(HasSurgeryUnit())
		{
			if(sysId.Which())
				return 1;
			return SurgeryUnitCount();
		}
		break;
	case SYS_Transporter:
		if(m_Transporters.size())
		{
			if(sysId.Which())
				return TransporterHits(sysId.Which());
			return TransporterHits();
		}
		break;
	case SYS_Turret:
		if(m_Turrets.size())
		{
			if(sysId.Which())
				return 1;
			return m_Turrets.size();
		}
		break;
	case SYS_VehicleLanding:
		if(m_VehicleLandingSystems.size())
		{
			if(sysId.Which())
				return d_VehicleLandingSystemHits;
			return VehicleLandingHits();
		}
		break;
	case SYS_VehicleLaunch:
		if(m_VehicleLaunchers.size())
		{
			if(sysId.Which())
				return d_VehicleLauncherHits;
			return VehicleLauncherHits();
		}
		break;
	case SYS_Weapon:
		if(sysId.Which())
			return WeaponBayHits(sysId.Which());
		return WeaponBayHits();
	case SYS_FTLDrive:
		if(sysId.Which())
			return FTLDriveHits(sysId.Which());
		return FTLDriveHits();
	case SYS_Capacitor:
		return CapacitorHits();
	case SYS_Count:
		break;
	}
	return 0;
}

quint32 AuScReport::HullSystemHits() const
{
	// Subtract hits from other hull-based systems.
	// Flight deck hits are not calculated from hull volume so they are not subtracted here.
	return HullHits() - (FuelHits() + HoldHits() + SkeletonHits());
}

void AuScReport::SystemIds(QSet<AuSysId>& sysIds, bool structural) const
{
	if(structural)
	{
		if(FuelVolume())
			sysIds.insert(SYS_Fuel);
		if(HoldVolume())
			sysIds.insert(SYS_Hold);
		sysIds.insert(SYS_Hull);
	}
	else
	{
		for(AuScReportStationHash::const_iterator si = m_CommandStations.cbegin(); si != m_CommandStations.cend(); si++)
			sysIds.insert(AuSysId(SYS_CommandAndControl, si.key()));
		for(AuScReportStationHash::const_iterator si = m_CrewStations.cbegin(); si != m_CrewStations.cend(); si++)
			sysIds.insert(AuSysId(SYS_CommandAndControl, si.key()));

		if(HasManeuverDrive())
			sysIds.insert(SYS_ManeuverDrive);
		if(HasLifeSupport())
			sysIds.insert(SYS_LifeSupport);
		if(HasPowerPlant())
			sysIds.insert(SYS_PowerPlant);
		if(HasSensor())
			sysIds.insert(SYS_Sensor);
		if(HasShield())
			sysIds.insert(SYS_Shield);

		QList<quint16> surgery = SurgeryUnitIds();
		for(QList<quint16>::const_iterator si = surgery.cbegin(); si != surgery.cend(); si++)
			sysIds.insert(AuSysId(SYS_SurgeryUnit, *si));

		for(AuScReportTransporterStationHash::const_iterator si = m_Transporters.cbegin(); si != m_Transporters.cend(); si++)
			sysIds.insert(AuSysId(SYS_Transporter, si.key()));

		for(AuScReportTurretHash::const_iterator si = m_Turrets.cbegin(); si != m_Turrets.cend(); si++)
			sysIds.insert(AuSysId(SYS_Turret, si.key()));

		for(AuScReportSysVolHash::const_iterator si = m_Bays.cbegin(); si != m_Bays.cend(); si++)
			sysIds.insert(AuSysId(SYS_Weapon, si.key()));

		for(AuScReportSysVolHash::const_iterator si = m_FTLDrives.cbegin(); si != m_FTLDrives.cend(); si++)
			sysIds.insert(AuSysId(SYS_FTLDrive, si.key()));

		for(QSet<quint16>::const_iterator si = m_VehicleLandingSystems.cbegin(); si != m_VehicleLandingSystems.cend(); si++)
			sysIds.insert(AuSysId(SYS_VehicleLanding, *si));

		for(QSet<quint16>::const_iterator si = m_VehicleLaunchers.cbegin(); si != m_VehicleLaunchers.cend(); si++)
			sysIds.insert(AuSysId(SYS_VehicleLaunch, *si));

		if(HasCapacitor())
			sysIds.insert(SYS_Capacitor);
	}
}

quint32 AuScReport::SystemModuleCount(const AuSysId& sysId) const
{
	switch(sysId.System())
	{
	case SYS_CommandAndControl:
		if(sysId.Which())
			return 1;
		return CommandStationCount() + CrewStationCount();
	case SYS_ManeuverDrive:
		return (quint32)ceil(ManeuverDriveUnits());
	case SYS_Hull:
	case SYS_Fuel:
	case SYS_Hold:
		// Return the number of units of metal, magmium, etc. that were needed to build the ship.
		return m_MfgProc.Components().CountHullMaterials();
	case SYS_LifeSupport:
		return (quint32)ceil(LifeSupportUnits());
	case SYS_PowerPlant:
		return (quint32)ceil(PowerPlantUnits());
	case SYS_Sensor:
		return (quint32)ceil(SensorUnits());
	case SYS_Shield:
		return (quint32)ceil(ShieldUnits());
	case SYS_SurgeryUnit:
		if(HasSurgeryUnit())
		{
			if(sysId.Which())
				return 1;
			return SurgeryUnitCount();
		}
		break;
	case SYS_Transporter:
		if(m_Transporters.size())
		{
			if(sysId.Which())
				return TransporterHits(sysId.Which());
			return TransporterHits();
		}
		break;
	case SYS_Turret:
		if(m_Turrets.size())
		{
			if(sysId.Which())
				return 1;
			return m_Turrets.size();
		}
		break;
	case SYS_VehicleLaunch:
		if(m_VehicleLaunchers.size())
		{
			if(sysId.Which())
				return 1;
			return m_VehicleLaunchers.size();
		}
		break;
	case SYS_VehicleLanding:
		if(m_VehicleLandingSystems.size())
		{
			if(sysId.Which())
				return 1;
			return m_VehicleLandingSystems.size();
		}
		break;
	case SYS_Weapon:
		return WeaponBayUnits(sysId.Which());
	case SYS_FTLDrive:
		return FTLDriveUnits(sysId.Which());
	case SYS_Capacitor:
		return CapacitorUnits();
	case SYS_Count:
		break;
	}
	return 0;
}

bool AuScReport::SetSystemQuality(const AuSysId& sysId, quint8 quality)
{
	if(!quality)
		quality = 1;
	bool anyChange = false;
	switch(sysId.System())
	{
	case SYS_CommandAndControl:
		if(m_CnCQ != quality)
		{
			m_CnCQ = quality;
			anyChange = true;
		}
		break;

	case SYS_Fuel:
	case SYS_Hold:
	case SYS_Hull:
		if(m_Hull.Quality() != quality)
		{
			m_Hull.SetQuality(quality);
			anyChange = true;
		}
		break;

	case SYS_ManeuverDrive:
		if(m_ManeuverDrive.Quality() != quality)
		{
			m_ManeuverDrive.SetQuality(quality);
			anyChange = true;
		}
		break;

	case SYS_LifeSupport:
		if(m_LifeSupport.Quality() != quality)
		{
			m_LifeSupport.SetQuality(quality);
			anyChange = true;
		}
		break;

	case SYS_PowerPlant:
		if(m_PowerPlant.Quality() != quality)
		{
			m_PowerPlant.SetQuality(quality);
			anyChange = true;
		}
		break;

	case SYS_Sensor:
		if(m_Sensor.Quality() != quality)
		{
			m_Sensor.SetQuality(quality);
			anyChange = true;
		}
		break;

	case SYS_Shield:
		if(m_Shield.Quality() != quality)
		{
			m_Shield.SetQuality(quality);
			anyChange = true;
		}
		break;

	case SYS_SurgeryUnit:
		if(SurgeryUnitQuality(sysId.Which()) != quality)
		{
			SetSurgeryUnitQuality(sysId.Which(), quality);
			anyChange = true;
		}
		break;

	case SYS_Transporter:
		if(TransporterQuality(sysId.Which()) != quality)
		{
			SetTransporterQuality(sysId.Which(), quality);
			anyChange = true;
		}
		break;

	case SYS_Turret:
		if(TurretQuality(sysId.Which()) != quality)
		{
			SetTurretQuality(sysId.Which(), quality);
			anyChange = true;
		}
		break;

	case SYS_Weapon:
		if(WeaponBayQuality(sysId.Which()) != quality)
		{
			SetWeaponBayQuality(sysId.Which(), quality);
			anyChange = true;
		}
		break;

	case SYS_FTLDrive:
		if(FTLDriveQuality(sysId.Which()) != quality)
		{
			SetFTLDriveQuality(sysId.Which(), quality);
			anyChange = true;
		}
		break;

	case SYS_Capacitor:
		if(m_Capacitor.Quality() != quality)
		{
			m_Capacitor.SetQuality(quality);
			anyChange = true;
		}
		break;

	case SYS_VehicleLaunch:
	case SYS_VehicleLanding:
	case SYS_Count:
		break;
	}
	return anyChange;
}

