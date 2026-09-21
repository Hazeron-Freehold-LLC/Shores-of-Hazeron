//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubdreport.cpp,v 1.169 2022/02/19 18:19:59 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubdreport.h"
//
#include "aubddesign.h"
#include "aucombat.h"
#include "auglobal.h"
#include "auitem.h"
#include "auscdesign.h"
#include "auscparts.h"
#include "dgdata.h"

#define REPORT_SERIALIZE_VER			17
#define d_CapitolRadius					(200*FeetPerMeter())	// Jurisdiction area radius of capitol.
#define d_PoliceRadius					(500*FeetPerMeter())	// Jurisdiction area radius of min police station with 2 offices.

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBdReport::AuBdReport()
{
	// Design
	//m_ArchitectId;
	//m_ArchitectName;
	//m_DesignDesc;
	m_DesignId = 0;
	//m_DesignName;

	// Structure
	m_Structure = 0;
	m_IndustryId = 0;

	// Volumes, in cubic meters.
	//m_Hull;
	//m_HullMax;
	//m_HullMin;
	m_FieldArea = 0;
	m_ArenaVolume = 0;
	m_ArmorVolume = 0;
	m_AuditoriumVolume = 0;
	//m_Bays;
	//m_Capacitor;
	m_HoldVolume = 0;
	m_HomeLargeVolume = 0;
	m_HomeMediumVolume = 0;
	m_HomeSmallVolume = 0;
	//m_LifeSupport;
	m_LoungeVolume = 0;
	m_OfficeVolume = 0;
	//m_PowerPlant;
	m_StoreVolume = 0;
	//m_Sensor;
	//m_Shield;
	m_ShopVolume = 0;
	m_SkeletonVolume = 0;
	//m_Turrets;

	// Quality
	m_Quality = 1;
	m_CnCModule = 0;
	m_CnCQ = 1;
	//m_ConProc

	// Stations and Equipment
	//m_CommandStations;
	//m_CrewStations;
	//m_GameTables;
	//m_GuardPosts;
	//m_Lifts;
	//m_StatusDisplays;
	//m_Transporters;
	//m_VehicleLaunchers;
	//m_VehicleLandingSystems;

	// Berths
	m_Beds = 0;

	// Parking
	m_ParkGround = 0;
	m_ParkRocket = 0;
	m_ParkSpace = 0;
	m_ParkSpacecraft = 0;
	m_ParkWater = 0;

	// Mass
	m_MassKg = 0;
	m_MassEquipmentKg = 0;
	m_MassHullKg = 0;

	// Analysis
	m_ArenaCapacity = 0;
	m_AuditoriumCapacity = 0;
	m_CapacitorMax = 5;
	m_FaceCount = 0;
	m_FieldCapacity = 0;
	m_FuelBurnRate = 0;
	m_HasBar = false;
	m_HasEnvironmentVoid = false;
	m_HasPantry = false;
	m_HoldCapacity = 0;
	m_HomeLargeCapacity = 0;
	m_HomeMediumCapacity = 0;
	m_HomeSmallCapacity = 0;
	m_IdlePowerWithShields = 0;
	m_LoungeCapacity = 0;
	m_OfficeCapacity = 0;
	m_SensorRange = 0;
	m_ShopCapacity = 0;
	m_StoreCapacity = 0;
	m_TriangleCount = 0;
}

AuBdReport::~AuBdReport()
{
}

AuBdReport::AuBdReport(const AuBdReport& c) : m_PartArchitects(c.m_PartArchitects), m_Bays(c.m_Bays),
	m_Turrets(c.m_Turrets), m_CitizenPosts(c.m_CitizenPosts), m_CommandStations(c.m_CommandStations),
	m_CrewStations(c.m_CrewStations), m_GameTables(c.m_GameTables), m_GuardPosts(c.m_GuardPosts), m_Lifts(c.m_Lifts),
	m_LivestockPosts(c.m_LivestockPosts), m_StatusDisplays(c.m_StatusDisplays), m_Transporters(c.m_Transporters),
	m_VehicleLandingSystems(c.m_VehicleLandingSystems), m_VehicleLaunchers(c.m_VehicleLaunchers)
{
	// Design
	m_ArchitectId = c.m_ArchitectId;
	m_ArchitectName = c.m_ArchitectName;
	m_DesignDesc = c.m_DesignDesc;
	m_DesignId = c.m_DesignId;
	m_DesignName = c.m_DesignName;

	// Structure
	m_Structure = c.m_Structure;
	m_IndustryId = c.m_IndustryId;

	// Volumes, in cubic meters.
	m_Hull = c.m_Hull;
	m_HullMax = c.m_HullMax;
	m_HullMin = c.m_HullMin;
	m_SiteMax = c.m_SiteMax;
	m_SiteMin = c.m_SiteMin;
	m_FieldArea = c.m_FieldArea;
	m_ArenaVolume = c.m_ArenaVolume;
	m_ArmorVolume = c.m_ArmorVolume;
	m_AuditoriumVolume = c.m_AuditoriumVolume;
	m_Capacitor = c.m_Capacitor;
	m_HoldVolume = c.m_HoldVolume;
	m_HomeLargeVolume = c.m_HomeLargeVolume;
	m_HomeMediumVolume = c.m_HomeMediumVolume;
	m_HomeSmallVolume = c.m_HomeSmallVolume;
	m_LifeSupport = c.m_LifeSupport;
	m_LoungeVolume = c.m_LoungeVolume;
	m_OfficeVolume = c.m_OfficeVolume;
	m_PowerPlant = c.m_PowerPlant;
	m_Sensor = c.m_Sensor;
	m_Shield = c.m_Shield;
	m_ShopVolume = c.m_ShopVolume;
	m_StoreVolume = c.m_StoreVolume;
	m_SkeletonVolume = c.m_SkeletonVolume;

	// Quality
	m_Quality = c.m_Quality;
	m_CnCModule = c.m_CnCModule;
	m_CnCQ = c.m_CnCQ;
	m_ConProc = c.m_ConProc;

	// Stations and Equipment

	// Berths
	m_Beds = c.m_Beds;

	// Parking
	m_ParkGround = c.m_ParkGround;
	m_ParkRocket = c.m_ParkRocket;
	m_ParkSpace = c.m_ParkSpace;
	m_ParkSpacecraft = c.m_ParkSpacecraft;
	m_ParkWater = c.m_ParkWater;

	// Mass
	m_MassKg = c.m_MassKg;
	m_MassEquipmentKg = c.m_MassEquipmentKg;
	m_MassHullKg = c.m_MassHullKg;

	// Analysis
	m_ArenaCapacity = c.m_ArenaCapacity;
	m_AuditoriumCapacity = c.m_AuditoriumCapacity;
	m_CapacitorMax = c.m_CapacitorMax;
	m_FaceCount = c.m_FaceCount;
	m_FieldCapacity = c.m_FieldCapacity;
	m_FuelBurnRate = c.m_FuelBurnRate;
	m_HasBar = c.m_HasBar;
	m_HasEnvironmentVoid = c.m_HasEnvironmentVoid;
	m_HasPantry = c.m_HasPantry;
	m_HoldCapacity = c.m_HoldCapacity;
	m_HomeLargeCapacity = c.m_HomeLargeCapacity;
	m_HomeMediumCapacity = c.m_HomeMediumCapacity;
	m_HomeSmallCapacity = c.m_HomeSmallCapacity;
	m_IdlePowerWithShields = c.m_IdlePowerWithShields;
	m_LoungeCapacity = c.m_LoungeCapacity;
	m_OfficeCapacity = c.m_OfficeCapacity;
	m_PartsList = c.m_PartsList;
	m_SensorRange = c.m_SensorRange;
	m_ShopCapacity = c.m_ShopCapacity;
	m_StoreCapacity = c.m_StoreCapacity;
	m_TriangleCount = c.m_TriangleCount;
}

const AuBdReport& AuBdReport::operator=(const AuBdReport& c)
{
	// Design
	m_ArchitectId = c.m_ArchitectId;
	m_ArchitectName = c.m_ArchitectName;
	m_DesignDesc = c.m_DesignDesc;
	m_DesignId = c.m_DesignId;
	m_DesignName = c.m_DesignName;
	m_PartArchitects = c.m_PartArchitects;

	// Structure
	m_Structure = c.m_Structure;
	m_IndustryId = c.m_IndustryId;

	// Volumes, in cubic meters.
	m_Hull = c.m_Hull;
	m_HullMax = c.m_HullMax;
	m_HullMin = c.m_HullMin;
	m_SiteMax = c.m_SiteMax;
	m_SiteMin = c.m_SiteMin;
	m_FieldArea = c.m_FieldArea;
	m_ArenaVolume = c.m_ArenaVolume;
	m_ArmorVolume = c.m_ArmorVolume;
	m_AuditoriumVolume = c.m_AuditoriumVolume;
	m_Bays = c.m_Bays;
	m_Capacitor = c.m_Capacitor;
	m_HoldVolume = c.m_HoldVolume;
	m_HomeLargeVolume = c.m_HomeLargeVolume;
	m_HomeMediumVolume = c.m_HomeMediumVolume;
	m_HomeSmallVolume = c.m_HomeSmallVolume;
	m_LifeSupport = c.m_LifeSupport;
	m_LoungeVolume = c.m_LoungeVolume;
	m_OfficeVolume = c.m_OfficeVolume;
	m_PowerPlant = c.m_PowerPlant;
	m_Sensor = c.m_Sensor;
	m_Shield = c.m_Shield;
	m_ShopVolume = c.m_ShopVolume;
	m_StoreVolume = c.m_StoreVolume;
	m_SkeletonVolume = c.m_SkeletonVolume;
	m_Turrets = c.m_Turrets;

	// Quality
	m_Quality = c.m_Quality;
	m_CnCModule = c.m_CnCModule;
	m_CnCQ = c.m_CnCQ;
	m_ConProc = c.m_ConProc;

	// Stations
	m_CitizenPosts = c.m_CitizenPosts;
	m_CommandStations = c.m_CommandStations;
	m_CrewStations = c.m_CrewStations;
	m_GameTables = c.m_GameTables;
	m_GuardPosts = c.m_GuardPosts;
	m_Lifts = c.m_Lifts;
	m_LivestockPosts = c.m_LivestockPosts;
	m_StatusDisplays = c.m_StatusDisplays;
	m_Transporters = c.m_Transporters;
	m_VehicleLaunchers = c.m_VehicleLaunchers;
	m_VehicleLandingSystems = c.m_VehicleLandingSystems;

	// Berths
	m_Beds = c.m_Beds;

	// Parking
	m_ParkGround = c.m_ParkGround;
	m_ParkRocket = c.m_ParkRocket;
	m_ParkSpace = c.m_ParkSpace;
	m_ParkSpacecraft = c.m_ParkSpacecraft;
	m_ParkWater = c.m_ParkWater;

	// Mass
	m_MassKg = c.m_MassKg;
	m_MassEquipmentKg = c.m_MassEquipmentKg;
	m_MassHullKg = c.m_MassHullKg;

	// Analysis
	m_ArenaCapacity = c.m_ArenaCapacity;
	m_AuditoriumCapacity = c.m_AuditoriumCapacity;
	m_CapacitorMax = c.m_CapacitorMax;
	m_FaceCount = c.m_FaceCount;
	m_FieldCapacity = c.m_FieldCapacity;
	m_FuelBurnRate = c.m_FuelBurnRate;
	m_HasBar = c.m_HasBar;
	m_HasEnvironmentVoid = c.m_HasEnvironmentVoid;
	m_HasPantry = c.m_HasPantry;
	m_HoldCapacity = c.m_HoldCapacity;
	m_HomeLargeCapacity = c.m_HomeLargeCapacity;
	m_HomeMediumCapacity = c.m_HomeMediumCapacity;
	m_HomeSmallCapacity = c.m_HomeSmallCapacity;
	m_IdlePowerWithShields = c.m_IdlePowerWithShields;
	m_LoungeCapacity = c.m_LoungeCapacity;
	m_OfficeCapacity = c.m_OfficeCapacity;
	m_PartsList = c.m_PartsList;
	m_SensorRange = c.m_SensorRange;
	m_ShopCapacity = c.m_ShopCapacity;
	m_StoreCapacity = c.m_StoreCapacity;
	m_TriangleCount = c.m_TriangleCount;

	return *this;
}

//static
unsigned int AuBdReport::CompressedSizeLimit()
{
	return 7000000;
}

//static
unsigned int AuBdReport::CompressedSizeLimitPerc()
{
	return CompressedSizeLimit()/100;
}

//static
unsigned int AuBdReport::SizeLimit()
{
	return 14000000;
}

//static
unsigned int AuBdReport::SizeLimitPerc()
{
	return SizeLimit()/100;
}

bool AuBdReport::OkForAssembly(QStringList& msgs, const AuBdDesign& design) const
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
		msgs.append(note.arg(QObject::tr("Success! The building blueprint can be used as an assembly.")));

	return isOk;
}

bool AuBdReport::OkToConstruct(QStringList& msgs, const AuBdDesign* design) const
{
	bool isOk = true;	// Return value.
	QString error("<span style=\"color: rgb(255, 32, 32);\">Error: %1</span>");
	QString note("<span style=\"color: rgb(32, 255, 32);\">%1</span>");
	QString warning("<span style=\"color: rgb(255, 128, 32);\">Warning: %1</span>");
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
							"of %L2 bytes. "
							"Conserve the number of faces and custom textures.")
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

	// Street lights are flimsy and may not have much structure.
	// Street lights should not have to fit within their site.
	bool hullRequired = Structure() != BD_StreetLight && Structure() != BD_Fence;
	bool siteBoundsHull = Structure() != BD_StreetLight && Structure() != BD_Fence;

	// Volumes/system presense.
	if(!HullVolume())
	{
		if(hullRequired)
		{
			if(m_FieldCapacity)
				msgs.append(warning.arg(QObject::tr("No hull.")));
			else
			{
				msgs.append(error.arg(QObject::tr("No hull. Add hull.")));
				isOk = false;
			}
		}
	}
	else if(HullVolume() > AuBdDesign::MaxVolume())
	{
		msgs.append(error.arg(QObject::tr("Hull has too much volume. Make the hull smaller.")));
		isOk = false;
	}

	// Hull bounds.
	AuPoint hullSize = m_HullMax - m_HullMin;
	if(hullSize.x > AuBdDesign::SiteSizeLimitBase(Structure()) * 2)
	{
		msgs.append(error.arg(QObject::tr("Building is too wide. Width limit is %1m in X.")
					.arg(AuBdDesign::SiteSizeLimitBase(Structure()) * 2 / FeetPerMeter(), 0, 'f', 1)));
		isOk = false;
	}
	if(hullSize.y > AuBdDesign::SiteSizeLimitBase(Structure()) * 2)
	{
		msgs.append(error.arg(QObject::tr("Building is too long. Length limit is %1m in Y.")
					.arg(AuBdDesign::SiteSizeLimitBase(Structure()) * 2 / FeetPerMeter(), 0, 'f', 1)));
		isOk = false;
	}
	if(m_HullMax.z > AuBdDesign::HullSizeLimitHeight(Structure()))
	{
		msgs.append(error.arg(QObject::tr("Building is too tall. Height limit is %1m above Z=0.")
					.arg(AuBdDesign::HullSizeLimitHeight(Structure()) / FeetPerMeter(), 0,'f',1)));
		isOk = false;
	}
	if(m_HullMin.z < -AuBdDesign::HullSizeLimitDepth(Structure()))
	{
		msgs.append(error.arg(QObject::tr("Building goes too deep. Depth limit is %1m below Z=0.")
					.arg(AuBdDesign::HullSizeLimitDepth(Structure()) / FeetPerMeter(), 0,'f',1)));
		isOk = false;
	}

	// Site bounds. Aerial and sea platforms and drilling rigs do not define a build site.
	if(Structure() == BD_AerialPlatform ||
			Structure() == BD_SeaPlatform ||
			Structure() == BD_SeaSurfaceBuilding ||
			Structure() == BD_Fence)
	{
		// These building structures must not define a site.
		if(m_SiteMin != m_SiteMax)
		{
			msgs.append(error.arg(QObject::tr("Site is not wanted for %1. "
							"Delete or omit the site meshes.").arg(BuildingStructureName(Structure()))));
			isOk = false;
		}

		if(m_HullMin.x > 0 || m_HullMin.y > 0 || m_HullMin.z > 0 ||
				m_HullMax.x < 0 || m_HullMax.y < 0 || m_HullMax.z < 0)
		{
			msgs.append(error.arg(QObject::tr("Design origin is not inside the building extents. "
							"Move the building so the origin is inside its overall extents.")));
			isOk = false;
		}
	}
	else
	{
		if(m_SiteMin == m_SiteMax)
		{
			msgs.append(error.arg(QObject::tr("Site has no bounds. "
							"Create one or more site meshes to define the build site.")));
			isOk = false;
		}

		// Check origin is in the site.
		if(m_SiteMin.x > 0 || m_SiteMin.y > 0 || m_SiteMin.z > 0 ||
				m_SiteMax.x < 0 || m_SiteMax.y < 0 || m_SiteMax.z < 0)
		{
			msgs.append(error.arg(QObject::tr("Design origin is not inside the site extents. "
							"Move the build site so the origin is inside its overall extents.")));
			isOk = false;
		}

		// Check site size.
		AuPoint siteSize = m_SiteMax - m_SiteMin;
		if(siteSize.x > AuBdDesign::SiteSizeLimitBase(Structure()) * 2)
		{
			msgs.append(error.arg(QObject::tr("Site is too wide. Width limit is %1m.")
						.arg(AuBdDesign::SiteSizeLimitBase(Structure()) * 2 / FeetPerMeter(), 0, 'f', 1)));
			isOk = false;
		}
		if(siteSize.y > AuBdDesign::SiteSizeLimitBase(Structure()) * 2)
		{
			msgs.append(error.arg(QObject::tr("Site is too long. Length limit is %1m.")
						.arg(AuBdDesign::SiteSizeLimitBase(Structure()) * 2 / FeetPerMeter(), 0, 'f', 1)));
			isOk = false;
		}
		if(m_SiteMax.z > AuBdDesign::SiteSizeLimitHeight(Structure()))
		{
			msgs.append(error.arg(QObject::tr("Site is too tall. Height limit is %1m above Z=0.")
						.arg(AuBdDesign::SiteSizeLimitHeight(Structure()) / FeetPerMeter(), 0,'f',1)));
			isOk = false;
		}
		if(m_SiteMin.z < -AuBdDesign::SiteSizeLimitDepth(Structure()))
		{
			msgs.append(error.arg(QObject::tr("Site goes too deep. Depth limit is %1m below Z=0.")
						.arg(AuBdDesign::SiteSizeLimitDepth(Structure()) / FeetPerMeter(), 0,'f',1)));
			isOk = false;
		}

		// Hull bounded by site.
		if(siteBoundsHull)
		{
			if(m_HullMin.x < m_SiteMin.x)
			{
				msgs.append(error.arg(QObject::tr("Hull is out of site bounds on the left. "
								"Move the left side of the hull to the right or move the whole hull right.")));
				isOk = false;
			}

			if(m_HullMax.x > m_SiteMax.x)
			{
				msgs.append(error.arg(QObject::tr("Hull is out of site bounds on the right. "
								"Move the right side of the hull to the left or move the whole hull left.")));
				isOk = false;
			}

			if(m_HullMin.y < m_SiteMin.y)
			{
				msgs.append(error.arg(QObject::tr("Hull is out of site bounds at the back. "
								"Move the back side of the hull forward or move the whole hull forward.")));
				isOk = false;
			}

			if(m_HullMax.y > m_SiteMax.y)
			{
				msgs.append(error.arg(QObject::tr("Hull is out of site bounds at the front. "
								"Move the front side of the hull back or move the whole hull back.")));
				isOk = false;
			}
		}
		else
		{
			if(m_HullMin.x > 0 || m_HullMin.y > 0 || m_HullMin.z > 0 ||
					m_HullMax.x < 0 || m_HullMax.y < 0 || m_HullMax.z < 0)
			{
				msgs.append(error.arg(QObject::tr("Design origin is not inside the building extents. "
								"Move the building so the origin is inside its overall extents.")));
				isOk = false;
			}
		}
	}

	/*
	if(HullVolume() && !SkeletonVolume())
	{
		msgs.append(error.arg(QObject::tr("No framework. Allocate hull volume to framework.")));
		isOk = false;
	}
	*/

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

	// Calculate armor "level" based on hull volume.
	//double carmor = (float)m_ArmorVolume / (float)HullVolume();		// Armor from 0 to 1.
	//if(carmor > .5)
		//msgs.append(warning.arg(QObject::tr("Armor exceeds 50% of volume. Excess armor is useless.")));

	switch(Structure())
	{
	case BD_Invalid:
	case BD_Count:
		{
			msgs.append(error.arg(QObject::tr("Structure %1 is not valid.").arg(Structure())));
			isOk = false;
		}
		break;

	case BD_Biodome:
		{
			if(design)
			{
				// Check for an environment void.
				bool hasEnvVoid = false;
				bool hasBuildZone = false;
				for(AuScParts::const_iterator pi = design->Parts().cbegin(); pi != design->Parts().cend(); pi++)
				{
					if((*pi)->IsOmitted())
						continue;
					if(!(*pi)->IsEnvironmentVoid())
						hasEnvVoid = true;
					else if((*pi)->IsBuildZone())
						hasBuildZone = true;
				}
				if(!hasEnvVoid)
				{
					msgs.append(error.arg(QObject::tr("No environment void. "
									"Biodome will not create a habitable environment. ")));
					isOk = false;
				}
				if(hasBuildZone)
					msgs.append(error.arg(QObject::tr("Build zone not permitted in biodome blueprint.")));
			}

			if(HasLifeSupport() && LifeSupportModule() == CID_Invalid)
				msgs.append(warning.arg(QObject::tr("Space allocated to life support but no life support module selected.")));
			else if(!HasLifeSupport() && LifeSupportModule() != CID_Invalid)
				msgs.append(warning.arg(QObject::tr("Life support module selected but no space allocated to life support.")));
			else if(HasLifeSupport() &&
					!LifeSupportOkForFrigidLand() &&
					!LifeSupportOkForFrigidSeaBottom() &&
					!LifeSupportOkForGasGiantCore() &&
					!LifeSupportOkForInfernoLand() &&
					!LifeSupportOkForSeaBottom() &&
					!LifeSupportOkForVacuumLand())
				msgs.append(warning.arg(QObject::tr("Life support inadequate for any harsh environment.")));
			else if(!HasLifeSupport())
				msgs.append(warning.arg(QObject::tr("No life support. Biodome will not create a habitable environment.")));

			if(isOk && design)
				msgs.append(note.arg(QObject::tr("Success! A %1 can be constructed using the blueprint.")
						.arg(BuildingStructureName(Structure()))));
		}
		break;

	case BD_Bridge:
		if(design)
		{
			if(isOk)
				msgs.append(note.arg(QObject::tr("Success! A bridge can be constructed using the blueprint.")));
		}
		break;

	case BD_Building:
	case BD_SeaSurfaceBuilding:
		{
			if(HasGenerator())
			{
				quint32 gen;
				switch(PowerPlantModule())
				{
				default:
					gen = 0;
					break;
				case CID_CoalPowerPlantModule:
				case CID_GasolinePowerPlantModule:
					gen = 400;
					break;
				case CID_HydroelectricPowerPlantModule:
					gen = 800;
					break;
				case CID_HydrogenPowerPlantModule:
				case CID_NaturalGasPowerPlantModule:
					gen = 600;
					break;
				case CID_NuclearPowerPlantModule:
					gen = 1000;
					break;
				case CID_SolarPowerPlantModule:
				case CID_WindPowerPlantModule:
					gen = 300*4;	// Can be up to 4x depending upon environment.
					break;
				}
				gen += gen/10;	// Computers can increase generator output by 10%.
				gen += 100;		// Add a margin
				if(gen && m_CapacitorMax < gen * GeneratorJobs())
					msgs.append(warning.arg(QObject::tr("Battery storage is inadequate for power plant. "
									"%L1 terajoules per generator are needed to store output. "
									"Add more battery or reduce number of generators. ").arg(gen)));
			}

			if(!m_Turrets.empty())
			{
				for(QHash<quint16, AuScReportTurret>::const_iterator ti = m_Turrets.cbegin(); ti != m_Turrets.cend(); ti++)
				{
					COMMODITYID turretMod = (*ti).Module();
					if(turretMod == CID_Invalid)
					{
						msgs.append(error.arg(QObject::tr("No module for turret %1. "
										"Select the turret and create an integrated gunner station. "
										"It can be in an invisible and inaccessible place. ").arg(ti.key())));
						isOk = false;
					}
				}
				if(m_Turrets.size() > 6)
				{
					if(m_Turrets.size() == 7)
						msgs.append(error.arg(QObject::tr("Too many turrets; limit is 6. Remove one turret.")));
					else
						msgs.append(error.arg(QObject::tr("Too many turrets; limit is 6. Remove %1 turrets.")
									.arg(m_Turrets.size() - 6)));
					isOk = false;
				}
			}

			if(m_Sensor.Volume() && m_Sensor.Module() == CID_Invalid)
			{
				msgs.append(error.arg(QObject::tr("No control station for radar. Add a sensor station "
								"or reduce building volume allocated to radar.")));
				isOk = false;
			}
			else if(m_Sensor.Module() != CID_Invalid && !m_Sensor.Volume())
				msgs.append(warning.arg(QObject::tr("No radar for sensor control station. "
								"Radar range limited unless building volume is allocated to radar.")));

			if(m_Shield.Volume() && m_Shield.Module() == CID_Invalid)
			{
				msgs.append(error.arg(QObject::tr("No control station for shields. Add a shield station "
								"or reduce building volume allocated to shields.")));
				isOk = false;
			}
			else if(m_Shield.Module() != CID_Invalid && !m_Shield.Volume())
				msgs.append(warning.arg(QObject::tr("No shields for shield control station. "
								"Shield strength limited unless hull volume is allocated to shields.")));

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
							int projectors = AuBdDesign::BayWeaponProjectors(wpnsCuFt);
							unsigned int shots = (unsigned int)(CapacitorMax() / projectors);	// truncated result, in shots
							if(!shots)
							{
								msgs.append(error.arg(QObject::tr("Battery inadequate to fire energy weapon bay %1 at full power. "
												"Allocate hull volume to battery.")
											.arg(bi.key())));
								isOk = false;
							}
							else if(shots == 1)
								msgs.append(warning.arg(QObject::tr("Battery adequate to fire energy weapon bay %1 only once "
												"at full power. Allocate hull volume to battery to increase.")
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
								msgs.append(error.arg(QObject::tr("Battery inadequate to fire harvester bay %1 at full power. "
												"Allocate hull volume to battery.")
											.arg(bi.key())));
								isOk = false;
							}
							else if(shots == 1)
								msgs.append(warning.arg(QObject::tr("Battery adequate to fire harvester bay %1 only once "
												"at full power. Allocate hull volume to battery.")
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
								msgs.append(error.arg(QObject::tr("Battery inadequate to attach tractor beam %1. "
												"Allocate hull volume to battery.")
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
				bool hasBuildZone = false;
				int hullAndGear = 0;				// Count of hull and landing gear parts.
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
					else if((*pi)->IsBuildZone())
						hasBuildZone = true;

					// Count posts.
					if((*pi)->IsCitizenPost())
						postCitizens++;
					else if((*pi)->IsGuardPost())
						postGuards++;
					else if((*pi)->IsLivestockPost())
						postLivestock++;

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
						if(Structure() != BD_SeaSurfaceBuilding)
						{
							// Insure every vertex of the hull is inside the site.
							QSet<quint32> vertexIds;
							(*pi)->VertexIds(vertexIds);
							for(QSet<quint32>::const_iterator vi = vertexIds.cbegin(); vi != vertexIds.cend(); vi++)
							{
								AuScPoint p = (*pi)->VertexLoc(*vi);
								if(design->Parts().InBuildingSite(p))
									continue;

								p /= FeetPerMeter();	// Convert to meters.
								p.Round(.001);			// Round to nearest millimeter.
								msgs.append(error.arg(QObject::tr("%1 %2 is outside the site at (%L3, %L4, %L5)m. "
												"Modify or move the part so all of its vertices are inside a site mesh. ")
											.arg((*pi)->PartName())
											.arg(pi.key())
											.arg(p.x)
											.arg(p.y)
											.arg(p.z)));
								isOk = false;
								break;	// Enough whining about that part.
							}
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
												"Transparent windows should be assigned a group so they will be lit by room "
												"lighting. ")
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
					/*DHH Bd
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
					*/

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
						msgs.append(error.arg(QObject::tr("Part %1 %2 is an exact duplicate of part %3 %4. Delete one of them.")
									.arg(pi.key())
									.arg((*pi)->PartName())
									.arg(di.key())
									.arg((*di)->PartName())));
						isOk = false;
						break;
					}

					if(!isOk)
						msgs.append(warning.arg(QObject::tr("Skipping remainder of detailed analysis due to error, to save time.")));
				}

				if(!hullAndGear && !m_FieldCapacity)
				{
					msgs.append(error.arg(QObject::tr("No hull. Add one or more hull parts to define the shape of the building. ")));
					isOk = false;
				}

				if(hasEnvVoid)
					msgs.append(error.arg(QObject::tr("Environment void not permitted in building blueprint.")));
				if(hasBuildZone)
					msgs.append(error.arg(QObject::tr("Build zone not permitted in building blueprint.")));

				if(postCitizens > 14)
				{
					msgs.append(error.arg(QObject::tr("More than 14 citizen posts. "
									"Remove at least %1 of them. ")
								.arg(postCitizens - 14)));
					isOk = false;
				}
				else if(postCitizens > 2)
					msgs.append(note.arg(QObject::tr("More than 2 citizen posts. "
								"Up to 2 will be used at a time, selected randomly when needed. ")));

				if(postGuards > 10)
				{
					msgs.append(error.arg(QObject::tr("More than 10 troop posts. "
									"Remove at least %1 of them. ")
								.arg(postGuards - 10)));
					isOk = false;
				}
				else if(postGuards > 5)
					msgs.append(note.arg(QObject::tr("More than 5 troop posts. "
								"Up to 5 will be used at a time, selected randomly when needed. ")));

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
							for(AuScParts::const_iterator di = design->Parts().cbegin(); di != design->Parts().cend() && !doorId;
									di++)
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

				if(!hasDoor && rooms.size() > 1)
					msgs.append(warning.arg(QObject::tr("No doors. Doors may be needed to connect rooms. ")));

				if(!hasHullDoor && !rooms.empty())
					msgs.append(warning.arg(QObject::tr("No hull doors. A door may be needed to get inside from outside. ")));
			}
			else if(design)
				msgs.append(warning.arg(QObject::tr("Skipping detailed analysis due to error, to save time.")));

			if(HasStoreSpace() && CitizenPosts().empty())
				msgs.append(warning.arg(QObject::tr("No citizen post. A citizen post is recommended, for the store proprietor. ")));

			if(HasLifeSupport() && LifeSupportModule() == CID_Invalid)
				msgs.append(warning.arg(QObject::tr("Space allocated to life support but no life support module selected.")));
			else if(!HasLifeSupport() && LifeSupportModule() != CID_Invalid)
				msgs.append(warning.arg(QObject::tr("Life support module selected but no space allocated to life support.")));
			else if(HasLifeSupport() &&
					!LifeSupportOkForFrigidLand() &&
					!LifeSupportOkForFrigidSeaBottom() &&
					!LifeSupportOkForGasGiantCore() &&
					!LifeSupportOkForInfernoLand() &&
					!LifeSupportOkForSeaBottom() &&
					!LifeSupportOkForVacuumLand())
				msgs.append(warning.arg(QObject::tr("Life support inadequate for any harsh environment.")));

			if(HasCapacitor() && CapacitorModule() == CID_Invalid)
				msgs.append(warning.arg(QObject::tr("Space allocated to battery but no battery module selected.")));
			else if(!HasCapacitor() && CapacitorModule() != CID_Invalid)
				msgs.append(warning.arg(QObject::tr("Battery module selected but no space allocated to battery.")));

			if(m_HomeLargeCapacity + m_HomeMediumCapacity + m_HomeSmallCapacity > 0xffff)
				msgs.append(warning.arg(QObject::tr("Total homes is %L1. Resident population cannot exceed %L2. "
								"%L3 of the homes will be unused.")
							.arg(m_HomeLargeCapacity + m_HomeMediumCapacity + m_HomeSmallCapacity)
							.arg(0xffff)
							.arg((m_HomeLargeCapacity + m_HomeMediumCapacity + m_HomeSmallCapacity) - 0xffff)));

			if(m_ArenaCapacity > 0xffff)
				msgs.append(warning.arg(QObject::tr("Total arenas is %L1. Worker population cannot exceed %L2. "
								"%L3 of the arenas will be unused.")
							.arg(m_ArenaCapacity)
							.arg(0xffff)
							.arg(m_ArenaCapacity - 0xffff)));
			if(m_AuditoriumCapacity > 0xffff)
				msgs.append(warning.arg(QObject::tr("Total auditoriums is %L1. Worker population cannot exceed %L2. "
								"%L3 of the auditoriums will be unused.")
							.arg(m_AuditoriumCapacity)
							.arg(0xffff)
							.arg(m_AuditoriumCapacity - 0xffff)));
			if(m_FieldCapacity > 0xffff)
				msgs.append(warning.arg(QObject::tr("Total fields is %L1. Worker population cannot exceed %L2. "
								"%L3 of the fields will be unused.")
							.arg(m_FieldCapacity)
							.arg(0xffff)
							.arg(m_FieldCapacity - 0xffff)));
			if(GeneratorJobs() > 0xffff)
				msgs.append(warning.arg(QObject::tr("Total generators is %L1. Worker population cannot exceed %L2. "
								"%L3 of the generators will be unused.")
							.arg(GeneratorJobs())
							.arg(0xffff)
							.arg(GeneratorJobs() - 0xffff)));
			if(m_LoungeCapacity > 0xffff)
				msgs.append(warning.arg(QObject::tr("Total lounges is %L1. Worker population cannot exceed %L2. "
								"%L3 of the lounges will be unused.")
							.arg(m_LoungeCapacity)
							.arg(0xffff)
							.arg(m_LoungeCapacity - 0xffff)));
			if(m_OfficeCapacity > 0xffff)
				msgs.append(warning.arg(QObject::tr("Total offices is %L1. Worker population cannot exceed %L2. "
								"%L3 of the offices will be unused.")
							.arg(m_OfficeCapacity)
							.arg(0xffff)
							.arg(m_OfficeCapacity - 0xffff)));
			if(m_ShopCapacity > 0xffff)
				msgs.append(warning.arg(QObject::tr("Total workshops is %L1. Worker population cannot exceed %L2. "
								"%L3 of the workshops will be unused.")
							.arg(m_ShopCapacity)
							.arg(0xffff)
							.arg(m_ShopCapacity - 0xffff)));
			if(m_StoreCapacity > 0xffff)
				msgs.append(warning.arg(QObject::tr("Total stores is %L1. Worker population cannot exceed %L2. "
								"%L3 of the stores will be unused.")
							.arg(m_StoreCapacity)
							.arg(0xffff)
							.arg(m_StoreCapacity - 0xffff)));

			QSet<INDUSTRYID> industries;
			Industries(industries);
			if(industries.empty())
			{
				msgs.append(error.arg(QObject::tr("Building has no useful purpose.")));
				isOk = false;
			}
			else if(IndustryId() != IID_None && !industries.contains(IndustryId()))
			{
				msgs.append(error.arg(QObject::tr("Building cannot be used for its intended purpose.")));
				isOk = false;
			}

			if(isOk && design)
				msgs.append(note.arg(QObject::tr("Success! A building can be constructed using the blueprint.")));
		}
		break;

	case BD_AerialPlatform:
	case BD_SeaPlatform:
		if(design)
		{
			// Check for build zone. Platforms may have environment voids.
			bool hasBuildZone = false;
			bool hasSite = false;
			for(AuScParts::const_iterator pi = design->Parts().cbegin(); pi != design->Parts().cend(); pi++)
			{
				if((*pi)->IsOmitted())
					continue;
				if((*pi)->IsSite())
					hasSite = true;
				if((*pi)->IsBuildZone())
					hasBuildZone = true;
			}
			if(!hasBuildZone)
			{
				msgs.append(error.arg(QObject::tr("No build zone. "
								"Platform has no place for construction of buildings. ")));
				isOk = false;
			}
			if(hasSite)
			{
				msgs.append(error.arg(QObject::tr("Site not allowed. %1 must not define a site mesh. ")
							.arg(BuildingStructureName(Structure()))));
				isOk = false;
			}

			if(isOk)
				msgs.append(note.arg(QObject::tr("Success! A %1 can be constructed using the blueprint.")
							.arg(BuildingStructureName(Structure()))));
		}
		break;

	case BD_GroundPlatform:
		if(design)
		{
			// Check for build zone. Platforms may have environment voids.
			bool hasBuildZone = false;
			bool hasSite = false;
			for(AuScParts::const_iterator pi = design->Parts().cbegin(); pi != design->Parts().cend(); pi++)
			{
				if((*pi)->IsOmitted())
					continue;
				if((*pi)->IsSite())
					hasSite = true;
				if((*pi)->IsBuildZone())
					hasBuildZone = true;
			}
			if(!hasBuildZone)
			{
				msgs.append(error.arg(QObject::tr("No build zone. "
								"Platform has no place for construction of buildings. ")));
				isOk = false;
			}
			if(!hasSite)
			{
				msgs.append(error.arg(QObject::tr("No site. %1 has no site mesh defined for itself. ")
							.arg(BuildingStructureName(Structure()))));
				isOk = false;
			}

			if(isOk)
				msgs.append(note.arg(QObject::tr("Success! A %1 can be constructed using the blueprint.")
							.arg(BuildingStructureName(Structure()))));
		}
		break;

	case BD_RoadEnclosure:
	case BD_SpaceElevator:
	case BD_Statue:
	case BD_Tunnel:
		msgs.append(error.arg(QObject::tr("%1 structure not implemented yet.").arg(BuildingStructureName(Structure()))));
		isOk = false;
		break;

	case BD_StreetLight:
		if(design)
		{
			// Check for a light bulb.
			bool hasLight = false;
			bool hasEnvVoid = false;
			bool hasBuildZone = false;
			for(AuScParts::const_iterator pi = design->Parts().cbegin(); pi != design->Parts().cend(); pi++)
			{
				if((*pi)->IsOmitted())
					continue;
				if((*pi)->IsLight())
					hasLight = true;
				if((*pi)->IsEnvironmentVoid())
					hasEnvVoid = true;
				else if((*pi)->IsBuildZone())
					hasBuildZone = true;
			}
			if(!hasLight)
				msgs.append(warning.arg(QObject::tr("No light bulb. "
								"Street light will not emit light. ")));
			if(hasEnvVoid)
				msgs.append(error.arg(QObject::tr("Environment void not permitted in street light blueprint.")));
			if(hasBuildZone)
				msgs.append(error.arg(QObject::tr("Build zone not permitted in street light blueprint.")));

			if(isOk)
				msgs.append(note.arg(QObject::tr("Success! A street light can be constructed using the blueprint.")));
		}
		break;

	case BD_Fence:
		if(design)
		{
			// Check for faces and barriers in the post group.
			AuScFaceList post;
			design->VisibleFaces(post, AuScPost::GroupNameFencePost());
			if(post.empty())
				msgs.append(warning.arg(QObject::tr("No fence post. A fence usually has a %1 group with some faces. ")
							.arg(AuScPost::GroupNameFencePost())));
			AuScFaceList postBar;
			design->BarrierFaces(postBar, AuScPost::GroupNameFencePost());
			if(postBar.empty())
				msgs.append(warning.arg(QObject::tr("No fence post barrier. "
								"A fence post with no barrier will not obstruct movement. ")));

			// Check for faces and barriers in the section group.
			AuScFaceList section;
			design->VisibleFaces(section, AuScPost::GroupNameFenceSection());
			if(section.empty())
			{
				msgs.append(error.arg(QObject::tr("No fence section. A fence must have a %1 group with some faces. "
								"If no faces are wanted, create a tiny mesh under ground, "
								"to indicate the size of the empty section between posts.")
							.arg(AuScPost::GroupNameFenceSection())));
				isOk = false;
			}
			else
			{
				AuPoint pmin,pmax;
				post.Extents(pmin,pmax);
				AuPoint smin,smax;
				section.Extents(smin,smax);
				if(smax.x < 1*FeetPerMeter())
				{
					msgs.append(error.arg(QObject::tr("Fence section too short. "
									"The +X extent of the %1 group must be at least 1 meter.")
								.arg(AuScPost::GroupNameFenceSection())));
					isOk = false;
				}
				if(smax.x < (pmax.x - pmin.x))
				{
					msgs.append(error.arg(QObject::tr("Fence section shorter than post width. "
									"The +X extent of the %1 group must not be shorter than the X width of the post.")
								.arg(AuScPost::GroupNameFenceSection())));
					isOk = false;
				}
			}

			AuScFaceList sectBar;
			design->BarrierFaces(sectBar, AuScPost::GroupNameFenceSection());
			if(sectBar.empty())
				msgs.append(warning.arg(QObject::tr("No fence section barrier. "
								"A fence section with no barrier will not obstruct movement. ")));

			// Count barriers by group.
			int postBarriers = 0;
			int sectBarriers = 0;
			for(AuScParts::const_iterator pi = design->Parts().cbegin(); pi != design->Parts().cend(); pi++)
			{
				if((*pi)->IsOmitted())
					continue;
				if((*pi)->IsBarrier())
				{
					if((*pi)->GroupName() == AuScPart::GroupNameFencePost())
						postBarriers++;
					else if((*pi)->GroupName() == AuScPart::GroupNameFenceSection())
						sectBarriers++;
				}
			}

			// Post barriers.
			if(postBarriers > 2)
			{
				msgs.append(error.arg(QObject::tr("Fence %1 group contains too many barriers. "
								"Efficiency is critical in fence obstructions. "
								"Consider using a single simple barrier to encompass the entire post. ")
							.arg(AuScPost::GroupNameFencePost())));
				isOk = false;
			}
			else if(postBarriers > 1)
			{
				msgs.append(warning.arg(QObject::tr("Fence %1 group contains more than one barrier. "
								"Efficiency is critical in fence obstructions. "
								"Consider using a single simple barrier to encompass the entire post. ")
							.arg(AuScPost::GroupNameFencePost())));
			}

			// Section barriers.
			if(sectBarriers > 3)
			{
				msgs.append(error.arg(QObject::tr("Fence %1 group contains too many barriers. "
								"Efficiency is critical in fence obstructions. "
								"Consider using a single simple barrier to encompass the entire section. ")
							.arg(AuScPost::GroupNameFenceSection())));
				isOk = false;
			}
			else if(sectBarriers > 1)
			{
				msgs.append(warning.arg(QObject::tr("Fence %1 group contains more than one barrier. "
								"Efficiency is critical in fence obstructions. "
								"Consider using a single simple barrier to encompass the entire section. ")
							.arg(AuScPost::GroupNameFenceSection())));
			}
		}

		if(IndustryId() != IID_Fence)
		{
			msgs.append(error.arg(QObject::tr("Fence/Wall must be configured as the intended industry. ")));
			isOk = false;
		}

		if(isOk && design)
			msgs.append(note.arg(QObject::tr("Success! A fence can be constructed using the blueprint.")));
		break;
	}
	return isOk;
}

QString AuBdReport::ReportAnalysis(quint8 designState) const
{
	if(designState == AuDesign::Deleted)
		return QObject::tr("Deleted Design");
	// Fence does not have to have any hull volume.
	else if(designState == AuDesign::Edit && (!HullVolume() && !m_FieldCapacity && m_Structure != BD_Fence) && !g_Au.IsWebCGI())
		return "";		// Return an empty string. That cues the caller to provide helpful design tips in place of this report.

	QString cubedSym,squaredSym;
	if(g_Au.IsWebCGI())
	{
		cubedSym = QString("³");
		squaredSym = QString("²");
	}
	else
	{
		cubedSym = g_Au.CubedSymbol();
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
	if(HullVolume() || m_FieldCapacity || Structure() == BD_Fence)
	{
		// Calculate armor "level" based on hull volume.
		double armor = ArmorLevel();
		double rarmor = FROUND(armor, .01);
		QSet<INDUSTRYID> industries;
		Industries(industries);

		// Summary
		{
			stat += "<b>";
			stat += BuildingStructureName(Structure()).toUpper();
			stat += "</b>";
			{
				stat += "<div style=\"margin-left: 10px;\">";
				if(m_ArenaVolume)
					stat += QObject::tr("%L1m%2 Arena Space, %L3 Arenas<br>")
						.arg(m_ArenaVolume)
						.arg(cubedSym)
						.arg(m_ArenaCapacity);
				if(m_AuditoriumVolume)
					stat += QObject::tr("%L1m%2 Auditorium Space, %L3 Auditoriums<br>")
						.arg(m_AuditoriumVolume)
						.arg(cubedSym)
						.arg(m_AuditoriumCapacity);
				if(m_Capacitor.Volume())
					stat += QObject::tr("%L1m%2 Batteries, %L3 Terajoules<br>")
						.arg(m_Capacitor.Volume())
						.arg(cubedSym)
						.arg(m_CapacitorMax, 0,'f',0);
				if(m_HoldVolume)
					stat += QObject::tr("%L1m%2 Commodity Storage, %L3 Units<br>")
						.arg(m_HoldVolume)
						.arg(cubedSym)
						.arg(m_HoldCapacity);;
				if(m_FieldArea)
					stat += QObject::tr("%1m%2 Field Area, %L3 Fields<br>")
						.arg(m_FieldArea)
						.arg(squaredSym)
						.arg(m_FieldCapacity);
				if(m_PowerPlant.Volume())
					stat += QObject::tr("%L1m%2 Generator Equipment, %L3 Generators<br>")
						.arg(m_PowerPlant.Volume())
						.arg(cubedSym)
						.arg(GeneratorJobs());
				if(!m_GuardPosts.empty())
					stat += QObject::tr("%L1 Guard Posts<br>")
						.arg(m_GuardPosts.size());
				if(m_HomeLargeVolume)
					stat += QObject::tr("%L1m%2 Home, Large Space, %L3 Large Homes<br>")
						.arg(m_HomeLargeVolume)
						.arg(cubedSym)
						.arg(m_HomeLargeCapacity);
				if(m_HomeMediumVolume)
					stat += QObject::tr("%L1m%2 Home, Medium Space, %L3 Medium Homes<br>")
						.arg(m_HomeMediumVolume)
						.arg(cubedSym)
						.arg(m_HomeMediumCapacity);
				if(m_HomeSmallVolume)
					stat += QObject::tr("%L1m%2 Home, Small Space, %L3 Small Homes<br>")
						.arg(m_HomeSmallVolume)
						.arg(cubedSym)
						.arg(m_HomeSmallCapacity);
				if(!m_LivestockPosts.empty())
					stat += QObject::tr("%L1 Livestock<br>")
						.arg(m_LivestockPosts.size());
				if(m_LoungeVolume)
					stat += QObject::tr("%L1m%2 Lounge Space, %L3 Lounges<br>")
						.arg(m_LoungeVolume)
						.arg(cubedSym)
						.arg(m_LoungeCapacity);
				if(m_OfficeVolume)
					stat += QObject::tr("%L1m%2 Office Space, %L3 Offices<br>")
						.arg(m_OfficeVolume)
						.arg(cubedSym)
						.arg(m_OfficeCapacity);
				if(m_ParkGround)
					stat += QObject::tr("%L1 Parking for Ground Vehicles<br>")
						.arg(m_ParkGround);
				if(m_ParkSpace + m_ParkRocket)
					stat += QObject::tr("%L1 Parking for Space Vehicles<br>")
						.arg(m_ParkSpace + m_ParkRocket);
				if(m_ParkSpacecraft)
					stat += QObject::tr("%L1 Parking for Spacecraft<br>")
						.arg(m_ParkSpacecraft);
				if(m_ParkWater)
					stat += QObject::tr("%L1 Parking for Water Vehicles<br>")
						.arg(m_ParkWater);
				if(m_Sensor.Volume())
					stat += QObject::tr("%L1m%2 Radar Equipment<br>")
						.arg(m_Sensor.Volume())
						.arg(cubedSym);
				if(m_Shield.Volume())
					stat += QObject::tr("%L1m%2 Shield Equipment<br>")
						.arg(m_Shield.Volume())
						.arg(cubedSym);
				if(IndustryId() == IID_Bank)
				{
					if(m_ShopVolume)
						stat += QObject::tr("%L1m%2 Workshop Space, %L3 Workshops to Mint Money<br>")
							.arg(m_ShopVolume)
							.arg(cubedSym)
							.arg(m_ShopCapacity);
					else
						stat += QObject::tr("No Workshop Space, Bank Cannot Mint Money<br>");
				}
				else
				{
					if(m_ShopVolume)
						stat += QObject::tr("%L1m%2 Workshop Space, %L3 Workshops<br>")
							.arg(m_ShopVolume)
							.arg(cubedSym)
							.arg(m_ShopCapacity);
				}
				if(!m_VehicleLaunchers.empty())
					stat += QObject::tr("%L1 Space Vehicle Launchers<br>")
						.arg(m_VehicleLaunchers.size());
				if(!m_VehicleLandingSystems.empty())
					stat += QObject::tr("%L1 Space Vehicle Recovery Systems<br>")
						.arg(m_VehicleLandingSystems.size());
				if(m_StoreVolume)
					stat += QObject::tr("%L1m%2 Store Space, %L3 Stores<br>")
						.arg(m_StoreVolume)
						.arg(cubedSym)
						.arg(m_StoreCapacity);
				if(SurgeryUnitCount())
					stat += QObject::tr("%L1 Surgery Units<br>")
						.arg(SurgeryUnitCount());
				if(!m_Transporters.empty())
					stat += QObject::tr("%L1 Transporters<br>")
						.arg(m_Transporters.size());
				if(!m_Turrets.empty())
					stat += QObject::tr("%L1 Turrets<br>")
						.arg(m_Turrets.size());
				if(!m_Bays.empty())
					stat += QObject::tr("%L1 Weapon Bays<br>")
						.arg(m_Bays.size());
				stat = stat.left(stat.length()-4);	// Drop the last <br>
				stat += "</div>";
			}

			// Environments
			if(Structure() != BD_Fence)
			{
				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("Environments");
				stat += "<div style=\"margin-left: 10px;\">";

				//BENV_HabitableLand		= 0x01,		// Habitable environment land.
				stat += QString("%1 <b>Ok</b>").arg(BuildingEnvironmentName(BENV_HabitableLand));
				if(m_LifeSupport.Module() != CID_Invalid)
				{
					// Harsh environments are only reported when a life support module has been specified.
					stat += "<br>";

					//BENV_UnbreathableLand	= 0x02,		// Uninhabitable environment land, inner through outer zones.
					if(LifeSupportOkForVacuumLand())
						stat += QString("%1 <b>Ok</b>").arg(BuildingEnvironmentName(BENV_UnbreathableLand));
					else
						stat += QString("%1 <b>Inadequate Life Support</b>").arg(BuildingEnvironmentName(BENV_UnbreathableLand));
					stat += "<br>";

					//BENV_FrigidLand			= 0x04,		// Land on frigid world.
					if((SkeletonOkForFrigidLand() || ArmorOkForFrigidLand()) && LifeSupportOkForFrigidLand())
						stat += QString("%1 <b>Ok</b>").arg(BuildingEnvironmentName(BENV_FrigidLand));
					else if(!(SkeletonOkForFrigidLand() || ArmorOkForFrigidLand()) && !LifeSupportOkForFrigidLand())
						stat += QString("%1 <b>Inadequate Framework and Life Support</b>")
							.arg(BuildingEnvironmentName(BENV_FrigidLand));
					else if(!(SkeletonOkForFrigidLand() || ArmorOkForFrigidLand()))
						stat += QString("%1 <b>Inadequate Framework</b>").arg(BuildingEnvironmentName(BENV_FrigidLand));
					else// if(!LifeSupportOkForFrigidLand())
						stat += QString("%1 <b>Inadequate Life Support</b>").arg(BuildingEnvironmentName(BENV_FrigidLand));
					stat += "<br>";

					//BENV_InfernoLand		= 0x08,		// Land on inferno world.
					if((SkeletonOkForInfernoLand() || ArmorOkForInfernoLand()) && LifeSupportOkForInfernoLand())
						stat += QString("%1 <b>Ok</b>").arg(BuildingEnvironmentName(BENV_InfernoLand));
					else if(!(SkeletonOkForInfernoLand() || ArmorOkForInfernoLand()) && !LifeSupportOkForInfernoLand())
						stat += QString("%1 <b>Inadequate Framework and Life Support</b>")
							.arg(BuildingEnvironmentName(BENV_InfernoLand));
					else if(!(SkeletonOkForInfernoLand() || ArmorOkForInfernoLand()))
						stat += QString("%1 <b>Inadequate Framework</b>").arg(BuildingEnvironmentName(BENV_InfernoLand));
					else// if(!LifeSupportOkForInfernoLand())
						stat += QString("%1 <b>Inadequate Life Support</b>").arg(BuildingEnvironmentName(BENV_InfernoLand));
					stat += "<br>";

					//BENV_SeaBottom			= 0x10,		// Bottom of water sea.
					if((SkeletonOkForSeaBottom() || ArmorOkForSeaBottom()) && LifeSupportOkForSeaBottom())
						stat += QString("%1 <b>Ok</b>").arg(BuildingEnvironmentName(BENV_SeaBottom));
					else if(!(SkeletonOkForSeaBottom() || ArmorOkForSeaBottom()) && !LifeSupportOkForSeaBottom())
						stat += QString("%1 <b>Inadequate Framework and Life Support</b>")
							.arg(BuildingEnvironmentName(BENV_SeaBottom));
					else if(!(SkeletonOkForSeaBottom() || ArmorOkForSeaBottom()))
						stat += QString("%1 <b>Inadequate Framework</b>").arg(BuildingEnvironmentName(BENV_SeaBottom));
					else// if(!LifeSupportOkForSeaBottom())
						stat += QString("%1 <b>Inadequate Life Support</b>").arg(BuildingEnvironmentName(BENV_SeaBottom));
					stat += "<br>";

					//BENV_FrigidSeaBottom	= 0x20,		// Bottom of supercooled liquid sea on frigid world.
					if((SkeletonOkForFrigidSeaBottom() || ArmorOkForFrigidSeaBottom()) && LifeSupportOkForFrigidSeaBottom())
						stat += QString("%1 <b>Ok</b>").arg(BuildingEnvironmentName(BENV_FrigidSeaBottom));
					else if(!(SkeletonOkForFrigidSeaBottom() || ArmorOkForFrigidSeaBottom()) && !LifeSupportOkForFrigidSeaBottom())
						stat += QString("%1 <b>Inadequate Framework and Life Support</b>")
							.arg(BuildingEnvironmentName(BENV_FrigidSeaBottom));
					else if(!(SkeletonOkForFrigidSeaBottom() || ArmorOkForFrigidSeaBottom()))
						stat += QString("%1 <b>Inadequate Framework</b>").arg(BuildingEnvironmentName(BENV_FrigidSeaBottom));
					else// if(!LifeSupportOkForFrigidSeaBottom())
						stat += QString("%1 <b>Inadequate Life Support</b>").arg(BuildingEnvironmentName(BENV_FrigidSeaBottom));
					stat += "<br>";

					//BENV_GasGiantCore		= 0x40,		// Bottom of supercooled liquid at core of gas giant.
					if((SkeletonOkForGasGiantCore() || ArmorOkForGasGiantCore()) && LifeSupportOkForGasGiantCore())
						stat += QString("%1 <b>Ok</b>").arg(BuildingEnvironmentName(BENV_GasGiantCore));
					else if(!(SkeletonOkForGasGiantCore() || ArmorOkForGasGiantCore()) && !LifeSupportOkForGasGiantCore())
						stat += QString("%1 <b>Inadequate Framework and Life Support</b>")
							.arg(BuildingEnvironmentName(BENV_GasGiantCore));
					else if(!(SkeletonOkForGasGiantCore() || ArmorOkForGasGiantCore()))
						stat += QString("%1 <b>Inadequate Framework</b>").arg(BuildingEnvironmentName(BENV_GasGiantCore));
					else// if(!LifeSupportOkForGasGiantCore())
						stat += QString("%1 <b>Inadequate Life Support</b>").arg(BuildingEnvironmentName(BENV_GasGiantCore));
				}
				stat += "</div>";
				stat += "</div>";
			}

			if(!industries.empty())
			{
				// Alphabetize.
				QMap<QString, INDUSTRYID> ind;
				const DgData& gd = g_Au.LockDataForRead();
				for(QSet<INDUSTRYID>::const_iterator ii = industries.cbegin(); ii != industries.cend(); ii++)
					ind.insert(gd.IndustryName(*ii), *ii);
				g_Au.UnlockData();

				stat += "<div style=\"margin-left: 10px;\">";
				if(IndustryId() != IID_None)
				{
					const DgData& gd = g_Au.LockDataForRead();
					stat += QObject::tr("Intended Use <b>%1</b><br>").arg(gd.IndustryName(IndustryId()));
					g_Au.UnlockData();
				}
				stat += QObject::tr("Uses");
				stat += "<div style=\"margin-left: 10px;\">";
				for(QMap<QString, INDUSTRYID>::const_iterator si = ind.cbegin(); si != ind.cend(); si++)
				{
					if(si != ind.cbegin())
						stat += "<br>";
					if(*si == IID_Bank)
					{
						if(m_ShopCapacity)
							stat += QObject::tr("%1 with Mint").arg(si.key());
						else
							stat += QObject::tr("%1 without Mint").arg(si.key());
					}
					else
						stat += si.key();
				}
				stat += "</div>";
				stat += "</div>";
			}

			if(designState == AuDesign::Edit)
			{
				// Provide the warning and error messages that the finalizer shows.
				QStringList msgs;
				OkToConstruct(msgs, 0);
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
			stat += QObject::tr("<hr><b>STRUCTURE</b>");
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

			if(Structure() != BD_Fence)
			{
				float frame = round(SkeletonLevel() * 100);
				stat += QObject::tr("<br>Framework %1% of Structure")
					.arg(frame);
			}

			stat += "</div>";

			// Volume
			if(Structure() != BD_Fence)
			{
				double width = FROUND((m_HullMax.x - m_HullMin.x)/FeetPerMeter(), 0.01);
				double length = FROUND((m_HullMax.y - m_HullMin.y)/FeetPerMeter(), 0.01);
				double height = FROUND((m_HullMax.z - m_HullMin.z)/FeetPerMeter(), 0.01);
				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("%L1m%2 Volume").arg(HullVolume()).arg(cubedSym);
				stat += "<div style=\"margin-left: 10px;\">";

				// Station implicitly must spawn in space. No need to confuse by highlighting sizes.
				stat += QObject::tr("%L1m Length<br>").arg(length, 0,'f',0);
				stat += QObject::tr("%L1m Width<br>").arg(width, 0,'f',0);
				stat += QObject::tr("%L1m Height").arg(height, 0,'f',0);
				stat += "</div>";
				stat += "</div>";
			}

			// Model Size
			stat += "<div style=\"margin-left: 10px;\">";
			stat += QObject::tr("Model Size");
			stat += "<div style=\"margin-left: 10px;\">";
			stat += QObject::tr("%L1 Faces<br>").arg(FaceCount());
			stat += QObject::tr("%L1 Triangles").arg(TriangleCount());
			stat += "</div>";
			stat += "</div>";
		}

		// Avionics Equipment
		if(!m_CommandStations.empty() ||
				!m_CrewStations.empty() ||
				m_LifeSupport.Module() != CID_Invalid ||
				!m_StatusDisplays.empty() ||
				m_Sensor.Volume() ||
				m_Shield.Volume() ||
				!m_GameTables.empty() ||
				!m_Transporters.empty())
		{
			stat += QObject::tr("<hr><b>ELECTRONICS</b>");

			// Control Stations
			if(!m_CommandStations.isEmpty() || !m_CrewStations.isEmpty() || !m_Transporters.empty())
			{
				stat += "<div style=\"margin-left: 10px;\">";
				if(m_CnCModule != CID_Invalid)
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
			if(m_LifeSupport.Module() != CID_Invalid)
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
				stat += QObject::tr("%L1m%2 Radar").arg(m_Sensor.Volume()).arg(cubedSym);
				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("%1 Q%2<br>"
						"%3 %L4m Coverage Radius<br>"
						"%L5pc Warp Signature Detection Range if Empire Has Warp Technology")
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
				float absorbCap = CapacitorMax() / ShieldDissipatePerDamagePoint();
				float absorbShield = ShieldCapacitor() / ShieldDissipatePerDamagePoint();
				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("%L1m%2 Shield").arg(m_Shield.Volume()).arg(cubedSym);
				stat += "<div style=\"margin-left: 10px;\">";
				stat += QObject::tr("%1 Q%2<br>"
						"%L6m Radius<br>"
						"Absorbs %L3hp Total Damage<br>"
						"Absorbs %L4hp using Shield Power<br>"
						"Absorbs %L5hp using Battery Power")
					.arg(g_Au.CommodityName(m_Shield.Module()))
					.arg(m_Shield.Quality())
					.arg(round(absorbShield + absorbCap), 0,'f',0)
					.arg(round(absorbShield), 0,'f',0)
					.arg(round(absorbCap), 0,'f',0)
					.arg(round(ShieldBubbleRadius() / FeetPerMeter()));
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
					unsigned int range = (unsigned int)(AuBdDesign::BayWeaponRange(bayQ)/FeetPerMeter());

					switch(bayMod)
					{
					case CID_BallisticWeaponBayModule:
						{
							int cannons = AuBdDesign::BayWeaponCannons(wpnsCuFt);
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
							int projectors = AuBdDesign::BayWeaponProjectors(wpnsCuFt);
							stat += QObject::tr("<br>%L1 Beam Projectors").arg(projectors);
							stat += QObject::tr("<br>Targeting Range %L1m").arg(range);

							// Energy Weapons
							unsigned int shots = (unsigned int)(m_CapacitorMax / projectors);	// truncated result, in shots
							stat += "<div style=\"margin-left: 10px;\">";
							if(!shots)
							{
								stat += QString("<span style=\"color: rgb(255, 32, 32);\">");
								stat += QObject::tr("INADEQUATE BATTERY to Fire Energy Weapons at Full Power"); 
								stat += QString("</span>");
							}
							else if(shots == 1)
								stat += QObject::tr("Battery Adequate for One Shot of Energy Weapon at Full Power");
							else
								stat +=
									QObject::tr("Battery Adequate for %L1 Consecutive Shots of Energy Weapons at Full Power")
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
							int racks = AuBdDesign::BayWeaponEjectorRacks(wpnsCuFt);
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
							int launchers = AuBdDesign::BayWeaponLaunchers(wpnsCuFt);
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
							unsigned int range = (unsigned int)(AuBdDesign::HarvesterBayRange(wpnsCuFt, bayQ)/FeetPerMeter());
							double wpnPower = AuBdDesign::BayWeaponPower(wpnsCuFt, bayQ);
							wpnPower = round(wpnPower * .5f);	// 50% range

							// Harvest up to 1 unit for every fixed amount of energy that hits target.
							int harvestUnits = (int)(wpnPower / AuBdDesign::HarvestUnitsPerEnergyUnit());

							stat += QObject::tr("<br>Range %L1m<br>Harvest %2u at %L3m").arg(range).arg(harvestUnits).arg(range/2);
							if(!shots)
							{
								stat += QString("<span style=\"color: rgb(255, 32, 32);\">");
								stat += QObject::tr("<br>INADEQUATE BATTERY to Fire at Full Power"); 
								stat += QString("</span>");
							}
							else if(shots == 1)
								stat += QObject::tr("<br>Battery Adequate for One Shot at Full Power");
							else
								stat += QObject::tr("<br>Battery Adequate for %L1 Consecutive Shots at Full Power").arg(shots);
						}
						break;

					case CID_TractorBeamBayModule:
						{
							// Calculate number of consecutive shots that can be fired at full power.
							double wpnUnits = wpnsCuFt / 256.;
							unsigned int shots = (unsigned int)m_CapacitorMax / wpnUnits;	// truncated result, in shots

							// Calculate tractor beam range.
							quint32 tractorRange = (quint32)(AuBdDesign::TractorBeamRange(wpnsCuFt, bayQ)/FeetPerMeter());
							tractorRange = round(tractorRange);

							const double tlBoost = QualityBoostF(bayQ);
							double tractorAccel = wpnsCuFt * AuScDesign::ManeuverDriveForce() / m_MassKg * tlBoost;
							tractorAccel /= FeetPerMeter();
							tractorAccel = round(tractorAccel);

							stat += QObject::tr("<br>Range %L1m<br>Force %L2m/sec%3 vs Ship of Equal Mass")
								.arg(tractorRange)
								.arg(tractorAccel)
								.arg(squaredSym);
							if(!shots)
							{
								stat += QString("<span style=\"color: rgb(255, 32, 32);\">");
								stat += QObject::tr("<br>INADEQUATE BATTERY to Attach Tractor Beam"); 
								stat += QString("</span>");
							}
							else
								stat += QObject::tr("<br>Battery Adequate to Attach Tractor Beam");
						}
						break;

					default:
						break;
					}
				}
				else
				{
					stat += QString("<span style=\"color: rgb(255, 32, 32);\">");
					stat += QObject::tr("<br>INADEQUATE WEAPON UNITS to Fire"); 
					stat += QString("</span>");
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

		/*
		// Crew
		if(!m_CrewStations.empty() || m_ParkGround || m_ParkRocket || m_ParkSpace || m_ParkWater || !m_Turrets.empty() ||
				!m_GuardPosts.empty() || !m_Transporters.empty() || !m_CitizenPosts.empty() || !m_LivestockPosts.empty())
		{
			stat += QObject::tr("<hr><b>PERSONNEL</b>");
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
			if(!m_CitizenPosts.empty())
				stat += QObject::tr("%L1 Citizen Posts<br>").arg(m_CitizenPosts.size());
			if(!m_GuardPosts.empty())
				stat += QObject::tr("%L1 Guard Posts<br>").arg(m_GuardPosts.size());
			if(!m_LivestockPosts.empty())
				stat += QObject::tr("%L1 Livestock Posts<br>").arg(m_LivestockPosts.size());
			stat = stat.left(stat.length()-4);	// Drop the last <br>
			stat += "</div>";
		}
	*/

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
				quint32 hold = HoldHits();
				quint32 skeleton = SkeletonHits();
				quint32 hull = HullSystemHits();
				stat += "<div style=\"margin-left: 10px;\">";
				if(hold)
					stat += QObject::tr("%L1 Commodity Storage<br>").arg(hold);
				if(skeleton)
					stat += QObject::tr("%L1 Framework<br>").arg(skeleton);
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
					stat += QObject::tr("%L1 Battery<br>").arg(CapacitorHits());
				if(cnc)
					stat += QObject::tr("%L1 Command and Control<br>").arg(cnc);
				if(PowerPlantHits())
					stat += QObject::tr("%L1 Generator<br>").arg(PowerPlantHits());
				if(LifeSupportHits())
					stat += QObject::tr("%L1 Life Support<br>").arg(LifeSupportHits());
				if(SurgeryUnitCount())
					stat += QObject::tr("%L1 Medical<br>").arg(SurgeryUnitCount());
				if(SensorHits())
					stat += QObject::tr("%L1 Radar<br>").arg(SensorHits());
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
			// Construction Process
			stat += QObject::tr("<hr><b>CONSTRUCTION</b>");
			stat += "<div style=\"margin-left: 10px;\">";
			if(!CanBeConstructed())
			{
				// The process could not be created. Probably too many components.
				stat += QString("<span style=\"color: rgb(255, 32, 32);\">");
				stat += QObject::tr("UNABLE TO MAKE A CONSTRUCTION PROCESS");
				stat += QObject::tr("<br>Design May be Too Large<br>Try Reducing the Size of the Design");
				stat += QString("</span>");
			}
			else
			{
				// The process looks valid. List the components.
				if(Structure() == BD_Fence)
				{
					stat += QObject::tr("Build time and amount of construction material depends upon final fence/wall length.");
					stat += "<br>";
				}
				else
				{
					if(designState != AuDesign::Final)
					{
						stat += QObject::tr("Build time estimate and construction process are based on the original design "
								"configuration. ");
						stat += "<br>";
					}

					unsigned int buildSecs = m_ConProc.SecondsEstimated();
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
				}

				DeConstructionProcess cproc(m_ConProc);
				if(Structure() == BD_Fence && !cproc.Components().IsComponent(CID_Lumber))
					cproc.Components().AddRequirement(CID_Lumber, CE_Required, 1);
				cproc.ChangeMaterial(CID_Lumber, HullMaterial());

				QMap<QString, DeProcessComponentArray::const_iterator> sortedCon;
				QMap<QString, DeProcessComponentArray::const_iterator> sortedMod;
				for(DeProcessComponentArray::const_iterator pc = cproc.Components().begin();
						pc != cproc.Components().end(); pc++)
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
QString AuBdReport::ReportToolTip() const
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
		stat += BuildingStructureName(Structure()).toUpper();
		stat += "</b>";

		// Quality
		// NOTE: Ship hit points are multiplied by 10 when reported. This is due to the fact that 10 hits of damage only
		//       causes one unit of damage to the ship. Each unit of ship damage is represented by a commodity that needs
		//       to be replaced. It takes 10 points of damage inflicted to cause one unit of ship damage.
		stat += "<div style=\"margin-left: 10px;\">";
		stat += QObject::tr("Minimum Construction Quality %1").arg(m_Quality);
		stat += "<div style=\"margin-left: 10px;\">";
		stat += "</div>";
		stat += "</div>";
	}
	return stat;
}

void AuBdReport::operator>>(QDataStream& out) const
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
	out << m_Structure;
	out << m_IndustryId;

	// Volumes, in cubic meters.
	out << m_Hull;
	out << m_HullMax;
	out << m_HullMin;
	out << m_SiteMax;
	out << m_SiteMin;
	out << m_FieldArea;
	out << m_ArenaVolume;
	out << m_ArmorVolume;
	out << m_AuditoriumVolume;
	out << m_Bays;
	out << m_Capacitor;
	out << m_HoldVolume;
	out << m_HomeLargeVolume;
	out << m_HomeMediumVolume;
	out << m_HomeSmallVolume;
	out << m_LifeSupport;
	out << m_LoungeVolume;
	out << m_OfficeVolume;
	out << m_PowerPlant;
	out << m_StoreVolume;
	out << m_Sensor;
	out << m_Shield;
	out << m_ShopVolume;
	out << m_SkeletonVolume;
	out << m_Turrets;

	// Quality
	out << m_Quality;
	out << m_CnCModule;
	out << m_CnCQ;
	m_ConProc >> out;

	// Stations and Equipment
	out << m_CitizenPosts;
	out << m_CommandStations;
	out << m_CrewStations;
	out << m_GameTables;
	out << m_GuardPosts;
	out << m_Lifts;
	out << m_LivestockPosts;
	out << m_StatusDisplays;
	out << m_Transporters;
	out << m_VehicleLaunchers;
	out << m_VehicleLandingSystems;

	// Berths
	out << m_Beds;

	// Parking
	out << m_ParkGround;
	out << m_ParkRocket;
	out << m_ParkSpace;
	out << m_ParkSpacecraft;
	out << m_ParkWater;

	// Mass
	out << m_MassKg;
	out << m_MassEquipmentKg;
	out << m_MassHullKg;

	// Analysis
	out << m_FieldCapacity;
	out << m_ArenaCapacity;
	out << m_AuditoriumCapacity;
	out << m_CapacitorMax;
	out << m_FaceCount;
	out << m_FuelBurnRate;
	out << m_HoldCapacity;
	out << m_HomeLargeCapacity;
	out << m_HomeMediumCapacity;
	out << m_HomeSmallCapacity;
	out << m_IdlePowerWithShields;
	out << m_LoungeCapacity;
	out << m_OfficeCapacity;
	out << m_PartsList;
	out << m_StoreCapacity;
	out << m_SensorRange;
	out << m_ShopCapacity;
	out << m_TriangleCount;

	quint8 hasWhat = 0;
	if(m_HasBar)
		hasWhat |= 0x01;
	if(m_HasEnvironmentVoid)
		hasWhat |= 0x02;
	if(m_HasPantry)
		hasWhat |= 0x04;
	out << hasWhat;
}

void AuBdReport::operator<<(QDataStream& in)
{
	quint8 ver;
	in >> ver;

	// Design
	in >> m_ArchitectId;
	in >> m_ArchitectName;
	in >> m_DesignId;
	in >> m_DesignName;
	in >> m_PartArchitects;
	if(ver >= 16)
		in >> m_DesignDesc;

	// Structure
	in >> m_Structure;
	if(ver >= 14)
		in >> m_IndustryId;

	// Volumes, in cubic meters.
	in >> m_Hull;
	in >> m_HullMax;
	in >> m_HullMin;
	in >> m_SiteMax;
	in >> m_SiteMin;
	in >> m_FieldArea;
	in >> m_ArenaVolume;
	in >> m_ArmorVolume;
	in >> m_AuditoriumVolume;
	in >> m_Bays;
	in >> m_Capacitor;
	if(ver < 13)
	{
		quint32 fuelVol;
		in >> fuelVol;	// Fuel volume used to be store separately.
		in >> m_HoldVolume;
		m_HoldVolume += fuelVol;
	}
	else
		in >> m_HoldVolume;
	in >> m_HomeLargeVolume;
	in >> m_HomeMediumVolume;
	in >> m_HomeSmallVolume;
	in >> m_LifeSupport;
	in >> m_LoungeVolume;
	in >> m_OfficeVolume;
	in >> m_PowerPlant;
	in >> m_StoreVolume;
	in >> m_Sensor;
	in >> m_Shield;
	in >> m_ShopVolume;
	if(ver >= 15)
		in >> m_SkeletonVolume;
	else
		m_SkeletonVolume = 0;
	in >> m_Turrets;

	// Quality
	in >> m_Quality;
	if(!m_Quality)
		m_Quality = 1;
	in >> m_CnCModule;
	in >> m_CnCQ;
	if(!m_CnCQ)
		m_CnCQ = 1;
	m_ConProc << in;

	// Stations
	in >> m_CitizenPosts;
	in >> m_CommandStations;
	in >> m_CrewStations;
	in >> m_GameTables;
	in >> m_GuardPosts;
	in >> m_Lifts;
	in >> m_LivestockPosts;
	in >> m_StatusDisplays;
	in >> m_Transporters;
	in >> m_VehicleLaunchers;
	in >> m_VehicleLandingSystems;

	// Berths
	in >> m_Beds;

	// Parking
	in >> m_ParkGround;
	in >> m_ParkRocket;
	in >> m_ParkSpace;
	if(ver >= 0x0c)
		in >> m_ParkSpacecraft;
	in >> m_ParkWater;

	// Mass
	in >> m_MassKg;
	in >> m_MassEquipmentKg;
	if(ver < 13)
	{
		double fuelKg;
		in >> fuelKg;
		in >> m_MassHullKg;
		m_MassKg = m_MassEquipmentKg + m_MassHullKg;	// Recalc without fuel value.
	}
	else
		in >> m_MassHullKg;

	// Analysis
	in >> m_FieldCapacity;
	in >> m_ArenaCapacity;
	in >> m_AuditoriumCapacity;
	in >> m_CapacitorMax;
	// Buildings need at least five units of capacitor to import and use power for lights, and to support farming processes that
	// substitute electricity for sunlight..
	if(m_CapacitorMax < 5)
		m_CapacitorMax = 5;
	in >> m_FaceCount;
	in >> m_FuelBurnRate;
	if(ver < 13)
	{
		quint32 fuelCap;
		in >> fuelCap;
		in >> m_HoldCapacity;
		m_HoldCapacity += fuelCap;
	}
	else
		in >> m_HoldCapacity;
	in >> m_HomeLargeCapacity;
	in >> m_HomeMediumCapacity;
	in >> m_HomeSmallCapacity;
	in >> m_IdlePowerWithShields;
	in >> m_LoungeCapacity;
	in >> m_OfficeCapacity;
	in >> m_PartsList;
	in >> m_StoreCapacity;
	in >> m_SensorRange;
	in >> m_ShopCapacity;
	in >> m_TriangleCount;

	if(ver >= 17)
	{
		quint8 hasWhat = 0;
		in >> hasWhat;
		m_HasBar = hasWhat & 0x01;
		m_HasEnvironmentVoid = hasWhat & 0x02;
		m_HasPantry = hasWhat & 0x04;
	}
}

bool AuBdReport::operator==(const AuBdReport& c) const
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
	if(m_Structure != c.m_Structure)
		return false;
	if(m_IndustryId != c.m_IndustryId)
		return false;

	// Volumes, in cubic meters.
	if(m_Hull != c.m_Hull)
		return false;
	if(m_HullMax != c.m_HullMax)
		return false;
	if(m_HullMin != c.m_HullMin)
		return false;
	if(m_SiteMax != c.m_SiteMax)
		return false;
	if(m_SiteMin != c.m_SiteMin)
		return false;
	if(m_FieldArea != c.m_FieldArea)
		return false;
	if(m_ArenaVolume != c.m_ArenaVolume)
		return false;
	if(m_ArmorVolume != c.m_ArmorVolume)
		return false;
	if(m_AuditoriumVolume != c.m_AuditoriumVolume)
		return false;
	if(m_Bays != c.m_Bays)
		return false;
	if(m_Capacitor != c.m_Capacitor)
		return false;
	if(m_HoldVolume != c.m_HoldVolume)
		return false;
	if(m_HomeLargeVolume != c.m_HomeLargeVolume)
		return false;
	if(m_HomeMediumVolume != c.m_HomeMediumVolume)
		return false;
	if(m_HomeSmallVolume != c.m_HomeSmallVolume)
		return false;
	if(m_LifeSupport != c.m_LifeSupport)
		return false;
	if(m_LoungeVolume != c.m_LoungeVolume)
		return false;
	if(m_OfficeVolume != c.m_OfficeVolume)
		return false;
	if(m_PowerPlant != c.m_PowerPlant)
		return false;
	if(m_ShopVolume != c.m_ShopVolume)
		return false;
	if(m_Sensor != c.m_Sensor)
		return false;
	if(m_Shield != c.m_Shield)
		return false;
	if(m_StoreVolume != c.m_StoreVolume)
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
	//if(m_ConProc != c.m_ConProc)
		//return false;

	// Stations
	if(m_CitizenPosts != c.m_CitizenPosts)
		return false;
	if(m_CommandStations != c.m_CommandStations)
		return false;
	if(m_CrewStations != c.m_CrewStations)
		return false;
	if(m_GameTables != c.m_GameTables)
		return false;
	if(m_GuardPosts != c.m_GuardPosts)
		return false;
	if(m_Lifts != c.m_Lifts)
		return false;
	if(m_LivestockPosts != c.m_LivestockPosts)
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
	if(m_Beds != c.m_Beds)
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
	if(m_MassHullKg != c.m_MassHullKg)
		return false;

	// Analysis
	if(m_ArenaCapacity != c.m_ArenaCapacity)
		return false;
	if(m_AuditoriumCapacity != c.m_AuditoriumCapacity)
		return false;
	if(m_CapacitorMax != c.m_CapacitorMax)
		return false;
	if(m_FaceCount != c.m_FaceCount)
		return false;
	if(m_FieldCapacity != c.m_FieldCapacity)
		return false;
	if(m_FuelBurnRate != c.m_FuelBurnRate)
		return false;
	if(m_HasBar != c.m_HasBar)
		return false;
	if(m_HasEnvironmentVoid != c.m_HasEnvironmentVoid)
		return false;
	if(m_HasPantry != c.m_HasPantry)
		return false;
	if(m_HoldCapacity != c.m_HoldCapacity)
		return false;
	if(m_HomeLargeCapacity != c.m_HomeLargeCapacity)
		return false;
	if(m_HomeMediumCapacity != c.m_HomeMediumCapacity)
		return false;
	if(m_HomeSmallCapacity != c.m_HomeSmallCapacity)
		return false;
	if(m_IdlePowerWithShields != c.m_IdlePowerWithShields)
		return false;
	if(m_LoungeCapacity != c.m_LoungeCapacity)
		return false;
	if(m_OfficeCapacity != c.m_OfficeCapacity)
		return false;
	if(m_StoreCapacity != c.m_StoreCapacity)
		return false;
	if(m_ShopCapacity != c.m_ShopCapacity)
		return false;
	if(m_SensorRange != c.m_SensorRange)
		return false;
	if(m_TriangleCount != c.m_TriangleCount)
		return false;
	if(m_PartsList != c.m_PartsList)
		return false;

	return true;
}

QString AuBdReport::CommandStationName(quint16 partId) const
{
	AuScReportStationHash::const_iterator ci = m_CommandStations.find(partId);
	if(ci != m_CommandStations.cend())
		return (*ci).Desc();
	return "";
}

QString AuBdReport::CrewStationName(quint16 partId) const
{
	AuScReportStationHash::const_iterator ci = m_CrewStations.find(partId);
	if(ci != m_CrewStations.cend())
		return (*ci).Desc();

	AuScReportTransporterStationHash::const_iterator ti = m_Transporters.find(partId);
	if(ti != m_Transporters.cend())
		return (*ti).Desc();

	return "";
}

QString AuBdReport::StatusDisplayName(quint16 partId) const
{
	AuScReportStatusDisplayHash::const_iterator ci = m_StatusDisplays.find(partId);
	if(ci != m_StatusDisplays.cend())
		return (*ci).Desc();
	return "";
}

bool AuBdReport::HasSurgeryUnit() const
{
	for(QHash<quint16, AuScReportStation>::const_iterator ci = m_CrewStations.cbegin(); ci != m_CrewStations.cend(); ci++)
		if(BdModuleSystem((*ci).Module()) == SYS_SurgeryUnit)
			return true;
	return false;
}

quint16 AuBdReport::SurgeryUnitCount() const
{
	quint16 c = 0;
	for(QHash<quint16, AuScReportStation>::const_iterator ci = m_CrewStations.cbegin(); ci != m_CrewStations.cend(); ci++)
		if(BdModuleSystem((*ci).Module()) == SYS_SurgeryUnit)
			c++;
	return c;
}

double AuBdReport::ArmorLevel() const
{
	return AuBdDesign::ArmorLevel(m_ArmorVolume, HullVolume());
}

quint32 AuBdReport::CapacitorHits() const
{
	return EquipmentVolumeHits(m_Capacitor.Volume());
}

quint32 AuBdReport::VehicleLandingHits() const
{
	return m_VehicleLandingSystems.size() * d_VehicleLandingSystemHits;
}

quint32 AuBdReport::VehicleLauncherHits() const
{
	return m_VehicleLaunchers.size() * d_VehicleLauncherHits;
}

quint32 AuBdReport::HoldHits() const
{
	return HullVolumeHits(m_HoldVolume);
}

quint32 AuBdReport::SkeletonHits() const
{
	return HullVolumeHits(m_SkeletonVolume) * 4;
}

float AuBdReport::HullHitPointsPerUnit() const
{
	const float tlBoost = QualityBoostF(m_Hull.Quality());
	switch(m_Hull.Module())
	{
	case CID_Adamantine:
		return AuBdDesign::HullDensityFactor(ArmorLevel()) * tlBoost * 1.6f;
	case CID_Vulcium:
		return AuBdDesign::HullDensityFactor(ArmorLevel()) * tlBoost * 1.5f;
	case CID_Magmium:
		return AuBdDesign::HullDensityFactor(ArmorLevel()) * tlBoost * 1.25f;
	case CID_Metal:
		return AuBdDesign::HullDensityFactor(ArmorLevel()) * tlBoost;
	default:
	case CID_Stone:
		return AuBdDesign::HullDensityFactor(ArmorLevel()) * tlBoost * 1.1f;
	case CID_Lumber:
		return AuBdDesign::HullDensityFactor(ArmorLevel()) * tlBoost * .5f;
	case CID_Wood:
		return AuBdDesign::HullDensityFactor(ArmorLevel()) * tlBoost * .75f;
	}
}

quint32 AuBdReport::HullVolumeHits(quint32 volumeCuM) const
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
float AuBdReport::EquipmentHitPointsPerUnit()
{
	return 1.f;
}

//static
quint32 AuBdReport::EquipmentVolumeHits(quint32 volumeCuM)
{
	if(volumeCuM == 0)
		return 0;
	double volCuFt = volumeCuM * CubicFeetPerCubicMeter();
	quint32 hits = (quint32)(volCuFt * EquipmentHitPointsPerUnit() / 256);
	if(!hits)
		return 1;
	return hits;
}

quint32 AuBdReport::HullHits() const
{
	if(SkeletonVolume())
		return HullVolumeHits(HullVolume() - SkeletonVolume()) + SkeletonHits();
	else
		return HullVolumeHits(HullVolume());
}

quint32 AuBdReport::LifeSupportHits() const
{
	return EquipmentVolumeHits(m_LifeSupport.Volume());
}

quint32 AuBdReport::PowerPlantHits() const
{
	return EquipmentVolumeHits(m_PowerPlant.Volume());
}

quint32 AuBdReport::SensorHits() const
{
	return EquipmentVolumeHits(m_Sensor.Volume());
}

quint32 AuBdReport::ShieldHits() const
{
	return EquipmentVolumeHits(m_Shield.Volume());
}

quint32 AuBdReport::TransporterHits() const
{
	quint32 v = 0;
	for(AuScReportTransporterStationHash::const_iterator ti = m_Transporters.cbegin(); ti != m_Transporters.cend(); ti++)
		v += (*ti).Hits();
	return v;
}

quint32 AuBdReport::TransporterHits(quint16 partId) const
{
	AuScReportTransporterStationHash::const_iterator ti = m_Transporters.find(partId);
	return (*ti).Hits();
}

quint32 AuBdReport::WeaponBayHits() const
{
	quint32 v = 0;
	for(AuScReportSysVolHash::const_iterator fi = m_Bays.cbegin(); fi != m_Bays.cend(); fi++)
		v += EquipmentVolumeHits((*fi).Volume());
	return v;
}

quint32 AuBdReport::WeaponBayHits(quint16 partId) const
{
	AuScReportSysVolHash::const_iterator fi = m_Bays.find(partId);
	if(fi == m_Bays.cend())
		return 0;
	return EquipmentVolumeHits((*fi).Volume());
}

double AuBdReport::TurretRange(quint16 which) const
{
	AuScReportTurretHash::const_iterator ti = m_Turrets.find(which);
	if(ti == m_Turrets.cend())
		return 0;
	const float tlBoost = QualityBoostF((*ti).Quality());
	return 1000.f * FeetPerMeter() * tlBoost;
}

QString AuBdReport::SystemName(const AuSysId& sysId) const
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
			//ci = m_DesignerStations.find(sysId.Which());
			//if(ci != m_DesignerStations.cend())
				//return QString("%1 %2").arg((*ci).Desc()).arg(sysId.Which());
		}
		break;

	case SYS_FTLDrive:
	case SYS_ManeuverDrive:
		break;	// n/a for buildings.
	case SYS_Fuel:
	case SYS_Hold:
	case SYS_Hull:
		return QObject::tr("%1, %2")
			.arg(sysId.BdName())
			.arg(g_Au.CommodityName(m_Hull.Module()));

	case SYS_Count:
		break;

	case SYS_LifeSupport:
		return QObject::tr("%1, %2")
			.arg(sysId.BdName())
			.arg(g_Au.CommodityName(m_LifeSupport.Module()));

	case SYS_PowerPlant:
		return QObject::tr("%1, %2")
			.arg(sysId.BdName())
			.arg(g_Au.CommodityName(m_PowerPlant.Module()));

	case SYS_Sensor:
		return QObject::tr("%1, %2")
			.arg(sysId.BdName())
			.arg(g_Au.CommodityName(m_Sensor.Module()));

	case SYS_Shield:
		return QObject::tr("%1, %2")
			.arg(sysId.BdName())
			.arg(g_Au.CommodityName(m_Shield.Module()));

	case SYS_SurgeryUnit:
		{
			AuScReportStationHash::const_iterator ci = m_CrewStations.find(sysId.Which());
			if(ci != m_CrewStations.cend())
				return QObject::tr("%1 %3, %2")
					.arg(sysId.BdName())
					.arg(g_Au.CommodityName((*ci).Module()))
					.arg(sysId.Which());
		}
		break;

	case SYS_Transporter:
		{
			AuScReportTransporterStationHash::const_iterator ci = m_Transporters.find(sysId.Which());
			if(ci != m_Transporters.cend())
				return QObject::tr("%1 %3, %2")
					.arg(sysId.BdName())
					.arg(g_Au.CommodityName((*ci).Module()))
					.arg(sysId.Which());
		}
		break;

	case SYS_Turret:
		{
			AuScReportTurretHash::const_iterator ci = m_Turrets.find(sysId.Which());
			if(ci != m_Turrets.cend())
				return QObject::tr("%1 %3, %2")
					.arg(sysId.BdName())
					.arg(g_Au.CommodityName((*ci).Module()))
					.arg(sysId.Which());
		}
		break;

	case SYS_VehicleLaunch:
	case SYS_VehicleLanding:
		if(sysId.Which())
			return QObject::tr("%1 %2")
				.arg(sysId.BdName())
				.arg(sysId.Which());
		break;

	case SYS_Weapon:
		{
			AuScReportSysVolHash::const_iterator ci = m_Bays.find(sysId.Which());
			if(ci != m_Bays.cend())
				return QObject::tr("%1 %3, %2")
					.arg(sysId.BdName())
					.arg(g_Au.CommodityName((*ci).Module()))
					.arg(sysId.Which());
		}
		break;

	case SYS_Capacitor:
		break;
	}
	return sysId.BdName();
}

QString AuBdReport::SystemName(const AuSysId& sysId, COMMODITYID module) const
{
	switch(sysId.System())
	{
	case SYS_CommandAndControl:
	case SYS_FTLDrive:
	case SYS_Fuel:
	case SYS_ManeuverDrive:
		break;	// n/a for buildings.
	case SYS_Hold:
	case SYS_Hull:
	case SYS_LifeSupport:
	case SYS_PowerPlant:
	case SYS_Sensor:
	case SYS_Shield:
	case SYS_Capacitor:
		// There can be only one of each of these systems.
		return QObject::tr("%1, %2")
			.arg(sysId.BdName())
			.arg(g_Au.CommodityName(module));

	case SYS_SurgeryUnit:
	case SYS_Turret:
	case SYS_Transporter:
	case SYS_Weapon:
		// There can be multiples of these systems so their id is helpful to distinguish them.
		return QObject::tr("%1 %2, %3")
			.arg(sysId.BdName())
			.arg(sysId.Which())
			.arg(g_Au.CommodityName(module));

	case SYS_VehicleLaunch:
	case SYS_VehicleLanding:
		if(sysId.Which())
			return QObject::tr("%1 %2")
				.arg(sysId.BdName())
				.arg(sysId.Which());
		break;

	case SYS_Count:
		break;
	}
	return sysId.BdName();
}

quint16 AuBdReport::TransporterRoomId(quint16 partId) const
{
	AuScReportTransporterStationHash::const_iterator ti = m_Transporters.find(partId);
	if(ti == m_Transporters.cend())
		return 0;
	return (*ti).RoomId();
}

float AuBdReport::HarvesterBayRange(quint16 which, unsigned int unitsFired) const
{
	AuScReportSysVolHash::const_iterator ti = m_Bays.find(which);
	if(ti == m_Bays.cend())
		return 0;
	double wpnsCuFt = unitsFired * 256;
	return AuBdDesign::HarvesterBayRange(wpnsCuFt, (*ti).Quality());
}

float AuBdReport::TractorBeamRange(quint16 which, unsigned int unitsFired) const
{
	AuScReportSysVolHash::const_iterator ti = m_Bays.find(which);
	if(ti == m_Bays.cend())
		return 0;
	double wpnsCuFt = unitsFired * 256;
	return AuBdDesign::TractorBeamRange(wpnsCuFt, (*ti).Quality());
}

float AuBdReport::WeaponBayRange(quint16 which) const
{
	AuScReportSysVolHash::const_iterator ti = m_Bays.find(which);
	if(ti == m_Bays.cend())
		return 0;
	return AuBdDesign::BayWeaponRange((*ti).Quality());
}

double AuBdReport::CapacitorUnits() const
{
	double cuft = m_Capacitor.Volume() * CubicFeetPerCubicMeter();
	return cuft / 256;
}

double AuBdReport::LifeSupportUnits() const
{
	double cuft = m_LifeSupport.Volume() * CubicFeetPerCubicMeter();
	return cuft / 256;
}

double AuBdReport::PowerPlantUnits() const
{
	double cuft = m_PowerPlant.Volume() * CubicFeetPerCubicMeter();
	return cuft / 256;
}

double AuBdReport::SensorUnits() const
{
	double cuft = m_Sensor.Volume() * CubicFeetPerCubicMeter();
	return cuft / 256;
}

double AuBdReport::ShieldCapacitor() const
{
	return ShieldUnits();
}

double AuBdReport::ShieldUnits() const
{
	double cuft = m_Shield.Volume() * CubicFeetPerCubicMeter();
	return cuft / 256;
}

double AuBdReport::WeaponBayUnits(quint16 partId) const
{
	AuScReportSysVolHash::const_iterator fi = m_Bays.find(partId);
	if(fi == m_Bays.cend())
		return 0;
	double cuft = (*fi).Volume() * CubicFeetPerCubicMeter();
	return cuft / 256;
}

QList<quint16> AuBdReport::SurgeryUnitIds() const
{
	QList<quint16> unitIds;
	for(QHash<quint16, AuScReportStation>::const_iterator ci = m_CrewStations.cbegin(); ci != m_CrewStations.cend(); ci++)
		if(BdModuleSystem((*ci).Module()) == SYS_SurgeryUnit)
			unitIds.append(ci.key());
	return unitIds;
}

COMMODITYID AuBdReport::SurgeryUnitModule(quint16 consoleId) const
{
	AuScReportStationHash::const_iterator ci = m_CrewStations.find(consoleId);
	if(ci == m_CrewStations.cend())
		return CID_Invalid;

	if(BdModuleSystem((*ci).Module()) != SYS_SurgeryUnit)
		return CID_Invalid;

	return (*ci).Module();
}

COMMODITYID AuBdReport::LifeSupportModule() const
{
	return m_LifeSupport.Module();
}

COMMODITYID AuBdReport::HullMaterial() const
{
	return HullModuleMaterial(HullModule());
}

COMMODITYID AuBdReport::HullModule() const
{
	return m_Hull.Module();
}

COMMODITYID AuBdReport::CapacitorModule() const
{
	return m_Capacitor.Module();
}

COMMODITYID AuBdReport::PowerPlantModule() const
{
	return m_PowerPlant.Module();
}

COMMODITYID AuBdReport::ShieldModule() const
{
	return m_Shield.Module();
}

COMMODITYID AuBdReport::SensorModule() const
{
	return m_Sensor.Module();
}

float AuBdReport::WeaponBayPower(quint16 which, float unitsFired) const
{
	AuScReportSysVolHash::const_iterator ti = m_Bays.find(which);
	if(ti == m_Bays.cend())
		return 0;
	double wpnsCuFt = unitsFired * 256;
	return AuBdDesign::BayWeaponPower(wpnsCuFt, (*ti).Quality());
}

// Returns shield power use rate in units per real world second.
double AuBdReport::ShieldPowerRate() const
{
	// Each shield unit uses one power units per U day.
	// Calculate use rate in power units per real second.
	const double rUnitRate = (double)UDaysPerRealDay() / (double)(24 * 3600);
	const double rUseRate = ShieldUnits() * rUnitRate;
	return rUseRate;
}

double AuBdReport::ShieldBubbleRadius() const
{
	//const double tlBoost = QualityBoostF(m_Shield.Quality());
	quint32 sv = ShieldVolume();
	double r = cbrt((double)sv / (4./3. * PI));
	r *= 20 * FeetPerMeter();// * tlBoost;
	return r;
}

// Returns sensor power use rate in units per real world second.
double AuBdReport::SensorPowerRate() const
{
	// Each sensor uses one power units per U day.
	// Calculate use rate in power units per real second.
	const double rUnitRate = (double)UDaysPerRealDay() / (double)(24 * 3600);
	const double rUseRate = SensorUnits() * rUnitRate;
	return rUseRate;
}

float AuBdReport::Density() const
{
	return MassLbs() / (float)(HullVolume() * CubicFeetPerCubicMeter());
}

QString AuBdReport::DescLine() const
{
	return QString("%1, Q%2, %L3m%4")
		.arg(BuildingStructureName(Structure()))
		.arg(Quality())
		.arg(HullVolume()).arg(g_Au.CubedSymbol());
}

bool AuBdReport::CanBeConstructed() const
{
	return !m_ConProc.Components().empty();
}

void AuBdReport::SetQuality(quint8 quality)
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
	m_LifeSupport.SetQuality(quality);
	m_PowerPlant.SetQuality(quality);
	m_Sensor.SetQuality(quality);
	m_Shield.SetQuality(quality);
	m_Turrets.SetQuality(quality);
	m_Quality = quality;
	m_CnCQ = quality;
	m_CommandStations.SetQuality(quality);
	m_CrewStations.SetQuality(quality);
	m_Transporters.SetQuality(quality);
}

quint16 AuBdReport::ClosestTransporterInRoom(const AuBdDesign& scd, quint16 roomId, const AuPoint& loc) const
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

quint16 AuBdReport::TurboLiftStopDoor(quint16 liftId, const QString& stopName) const
{
	AuScReportLiftHash::const_iterator li = m_Lifts.find(liftId);
	if(li == m_Lifts.cend())
		return 0;	// No lift with that id.

	AuLiftStopHash::const_iterator si = (*li).find(stopName);
	if(si == (*li).cend())
		return 0;	// No stop with that name.

	return (*si).DoorId();
}

quint16 AuBdReport::ClosestRemoteDoor(bool forVehicle, const AuPoint& loc, const AuDoorStateHash& doorStates) const
{
	double dist;
	return doorStates.ClosestRemoteDoor(forVehicle, loc, dist);
}

void AuBdReport::ModuleInstalled(quint16 which, COMMODITYID moduleCID, quint8 moduleQ)
{
	Q_ASSERT(moduleQ);
	switch(moduleCID)
	{
	default:
		return;
	case CID_WoodHullModule:
	case CID_LumberHullModule:
	case CID_StoneHullModule:
	case CID_MetalHullModule:
	case CID_MagmiumHullModule:
	case CID_VulciumHullModule:
	case CID_AdamantineHullModule:
		m_Hull.SetModule(moduleCID);
		m_Hull.SetQuality(moduleQ);
		break;
	case CID_CapacitorModule:
		m_Capacitor.SetModule(moduleCID);
		m_Capacitor.SetQuality(moduleQ);
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
	case CID_LifeSupportModule:
	case CID_LifeSupportGravityModule:
		m_LifeSupport.SetModule(moduleCID);
		m_LifeSupport.SetQuality(moduleQ);
		break;
	case CID_CoalPowerPlantModule:
	case CID_GasolinePowerPlantModule:
	case CID_HydroelectricPowerPlantModule:
	case CID_HydrogenPowerPlantModule:
	case CID_NaturalGasPowerPlantModule:
	case CID_NuclearPowerPlantModule:
	case CID_SolarPowerPlantModule:
	case CID_WindPowerPlantModule:
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

quint16 AuBdReport::VehicleComplement(VEHICLE_SPOT_TYPE spotType) const
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

void AuBdReport::HullVolumeDistribution(quint32& hullVolume, quint32& arena, quint32& armor, quint32& auditorium,
		quint32& capacitor, quint32& hold, quint32& homeLarge, quint32& homeMedium, quint32& homeSmall, quint32& lifeSupport,
		quint32& lounge, quint32& office, quint32& powerPlant, quint32& store, quint32& sensor, bool& hasSensor, quint32& shield,
		bool& hasShield, quint32& shop, QMap<quint16, AuScReportSysVol>& bays, quint32& skeleton) const
{
	hullVolume = HullVolume();
	arena = m_ArenaVolume;
	armor = m_ArmorVolume;
	auditorium = m_AuditoriumVolume;
	capacitor = m_Capacitor.Volume();
	hold = m_HoldVolume;
	homeLarge = m_HomeLargeVolume;
	homeMedium = m_HomeMediumVolume;
	homeSmall = m_HomeSmallVolume;
	lounge = m_LoungeVolume;
	store = m_StoreVolume;
	lifeSupport = m_LifeSupport.Volume();
	office = m_OfficeVolume;
	powerPlant = m_PowerPlant.Volume();
	shop = m_ShopVolume;
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
	for(AuScReportSysVolHash::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
		bays.insert(bi.key(), *bi);
	skeleton = m_SkeletonVolume;
}

void AuBdReport::SetArenaVolume(quint32 volume)
{
	m_ArenaVolume = volume;
}

void AuBdReport::SetAuditoriumVolume(quint32 volume)
{
	m_AuditoriumVolume = volume;
}

// 'volume' is in cubic meters.
void AuBdReport::SetArmorVolume(quint32 volume)
{
	m_ArmorVolume = volume;
}

// 'volume' is in cubic meters.
void AuBdReport::SetBayVolume(quint16 partId, quint32 volume)
{
	AuScReportSysVolHash::iterator bi = m_Bays.find(partId);
	if(bi != m_Bays.end())
		(*bi).SetVolume(volume);
}

// 'volume' is in cubic meters.
void AuBdReport::SetCapacitorVolume(quint32 volume)
{
	m_Capacitor.SetVolume(volume);
}

// 'volume' is in cubic meters.
void AuBdReport::SetHoldVolume(quint32 volume)
{
	m_HoldVolume = volume;
}

// 'volume' is in cubic meters.
void AuBdReport::SetSkeletonVolume(quint32 volume)
{
	m_SkeletonVolume = volume;
}

// 'volume' is in cubic meters.
void AuBdReport::SetHomeLargeVolume(quint32 volume)
{
	m_HomeLargeVolume = volume;
}

// 'volume' is in cubic meters.
void AuBdReport::SetHomeMediumVolume(quint32 volume)
{
	m_HomeMediumVolume = volume;
}

// 'volume' is in cubic meters.
void AuBdReport::SetHomeSmallVolume(quint32 volume)
{
	m_HomeSmallVolume = volume;
}

// 'volume' is in cubic meters.
void AuBdReport::SetLifeSupportVolume(quint32 volume)
{
	m_LifeSupport.SetVolume(volume);
}

void AuBdReport::SetLoungeVolume(quint32 volume)
{
	m_LoungeVolume = volume;
}

// 'volume' is in cubic meters.
void AuBdReport::SetOfficeVolume(quint32 volume)
{
	m_OfficeVolume = volume;
}

// 'volume' is in cubic meters.
void AuBdReport::SetPowerPlantVolume(quint32 volume)
{
	m_PowerPlant.SetVolume(volume);
}

void AuBdReport::SetStoreVolume(quint32 volume)
{
	m_StoreVolume = volume;
}

// 'volume' is in cubic meters.
void AuBdReport::SetShopVolume(quint32 volume)
{
	m_ShopVolume = volume;
}

// 'volume' is in cubic meters.
void AuBdReport::SetSensorVolume(quint32 volume)
{
	m_Sensor.SetVolume(volume);
}

// 'volume' is in cubic meters.
void AuBdReport::SetShieldVolume(quint32 volume)
{
	m_Shield.SetVolume(volume);
}

void AuBdReport::UpdateCalculatedValues()
{
	// Mass
	m_MassEquipmentKg = round(CalcEquipmentMass() * KilogramsPerPound());
	m_MassHullKg = round(CalcHullMass() * KilogramsPerPound());
	m_MassKg = m_MassEquipmentKg + m_MassHullKg;

	// Analysis
	m_ArenaCapacity = CalcArenaCapacity();
	m_AuditoriumCapacity = CalcAuditoriumCapacity();
	m_CapacitorMax = CalcCapacitorMax();
	m_FieldCapacity = CalcFieldCapacity();
	m_FuelBurnRate = CalcFuelBurnRate();
	m_HoldCapacity = CalcHoldCapacity();
	m_HomeLargeCapacity = CalcHomeLargeCapacity();
	m_HomeMediumCapacity = CalcHomeMediumCapacity();
	m_HomeSmallCapacity = CalcHomeSmallCapacity();
	m_LoungeCapacity = CalcLoungeCapacity();
	m_OfficeCapacity = CalcOfficeCapacity();
	m_StoreCapacity = CalcStoreCapacity();
	m_SensorRange = CalcSensorRange();
	m_ShopCapacity = CalcShopCapacity();
	m_IdlePowerWithShields = CalcPowerUsageIdling(true);
}

double AuBdReport::CalcEquipmentMass() const
{
	double mass = 0;

	// These values were taken from the old n_DetailType table, which expressed them in kg per unit.
	// e.g. A maneuver drive unit was 2250 kg. All units were 256 cubic feet.
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

double AuBdReport::CalcHullMass() const
{
	return AuBdDesign::HullDensity(ArmorLevel()) * HullVolume() * CubicFeetPerCubicMeter();
}

double AuBdReport::CalcCapacitorMax() const
{
	// These values were taken from the old DqDesignAnalysis, which expressed everything in system units.
	// All of the old system units were 256 cubic feet.
	double cap = (CapacitorVolume() * CubicFeetPerCubicMeter()) / 256.;
	// Buildings need at least five units of capacitor to import and use power for lights, and to support farming processes that
	// substitute electricity for sunlight..
	if(cap < 5)
		cap = 5;
	return cap;
}

// Returns power plant fuel burn rate in cubic feet per real world second.
// produced from each cubic foot of fuel that is burned.
double AuBdReport::CalcFuelBurnRate() const
{
	// Calculate burn rate in cubic feet per real second.
	// A drive unit is 256 cubic feet.
	const double fpuday = 24.;	// Power plant fuel burn per Universe day.
	const double rUnitRate = (double)UDaysPerRealDay() / (24. * 3600. / fpuday) / 256.;
	const double rBurnRate = PowerPlantVolume() * CubicFeetPerCubicMeter() * rUnitRate;
	return rBurnRate;
}

quint32 AuBdReport::CalcHoldCapacity() const
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

quint32 AuBdReport::CalcArenaCapacity() const
{
	if(ArenaVolume() < AuBdDesign::VolumeFirstArena())
		return 0;						// Not enough space for one.
	return (ArenaVolume() - AuBdDesign::VolumeFirstArena())  / AuBdDesign::VolumeAddArena() + 1;
}

quint32 AuBdReport::CalcAuditoriumCapacity() const
{
	if(AuditoriumVolume() < AuBdDesign::VolumeFirstAuditorium())
		return 0;						// Not enough space for one.
	return (AuditoriumVolume() - AuBdDesign::VolumeFirstAuditorium())  / AuBdDesign::VolumeAddAuditorium() + 1;
}

quint32 AuBdReport::CalcFieldCapacity() const
{
	if(FieldArea() < AuBdDesign::AreaFirstField())
		return 0;						// Not enough space for one.
	return (FieldArea() - AuBdDesign::AreaFirstField())  / AuBdDesign::AreaAddField() + 1;
}

quint32 AuBdReport::CalcLoungeCapacity() const
{
	if(LoungeVolume() < AuBdDesign::VolumeFirstLounge())
		return 0;						// Not enough space for one.
	return (LoungeVolume() - AuBdDesign::VolumeFirstLounge())  / AuBdDesign::VolumeAddLounge() + 1;
}

quint32 AuBdReport::CalcStoreCapacity() const
{
	if(StoreVolume() < AuBdDesign::VolumeFirstStore())
		return 0;						// Not enough space for one.
	return (StoreVolume() - AuBdDesign::VolumeFirstStore())  / AuBdDesign::VolumeAddStore() + 1;
}

quint32 AuBdReport::CalcHomeLargeCapacity() const
{
	if(HomeLargeVolume() < AuBdDesign::VolumeFirstHomeLarge())
		return 0;						// Not enough space for one.
	return (HomeLargeVolume() - AuBdDesign::VolumeFirstHomeLarge())  / AuBdDesign::VolumeAddHomeLarge() + 1;
}

quint32 AuBdReport::CalcHomeMediumCapacity() const
{
	if(HomeMediumVolume() < AuBdDesign::VolumeFirstHomeMedium())
		return 0;						// Not enough space for one.
	return (HomeMediumVolume() - AuBdDesign::VolumeFirstHomeMedium())  / AuBdDesign::VolumeAddHomeMedium() + 1;
}

quint32 AuBdReport::CalcHomeSmallCapacity() const
{
	if(HomeSmallVolume() < AuBdDesign::VolumeFirstHomeSmall())
		return 0;						// Not enough space for one.
	return (HomeSmallVolume() - AuBdDesign::VolumeFirstHomeSmall())  / AuBdDesign::VolumeAddHomeSmall() + 1;
}

quint32 AuBdReport::CalcOfficeCapacity() const
{
	if(OfficeVolume() < AuBdDesign::VolumeFirstOffice())
		return 0;						// Not enough space for one.
	return (OfficeVolume() - AuBdDesign::VolumeFirstOffice())  / AuBdDesign::VolumeAddOffice() + 1;
}

quint32 AuBdReport::CalcShopCapacity() const
{
	if(ShopVolume() < AuBdDesign::VolumeFirstShop())
		return 0;						// Not enough space for one.
	return (ShopVolume() - AuBdDesign::VolumeFirstShop())  / AuBdDesign::VolumeAddShop() + 1;
}

quint32 AuBdReport::GeneratorJobs() const
{
	if(PowerPlantVolume() < AuBdDesign::VolumeFirstGenerator())
		return 0;						// Not enough space for one.
	return (PowerPlantVolume() - AuBdDesign::VolumeFirstGenerator())  / AuBdDesign::VolumeAddGenerator() + 1;
}

// Returns life support power use rate in units per real world second.
double AuBdReport::LifeSupportPowerRate() const
{
	// Each unit uses one power unit per U day.
	// Calculate use rate in power units per real second.
	const double rUnitRate = (double)UDaysPerRealDay() / (double)(24 * 3600);
	const double rUseRate = LifeSupportUnits() * rUnitRate;
	return rUseRate;
}

// Power usage per real world second when idle
// - Gravity drives on
// - Life Support on
// - Radar on
// - Shields on
double AuBdReport::CalcPowerUsageIdling(bool shieldsUp) const
{
	const double life = LifeSupportPowerRate();
	const double sensor = SensorPowerRate();
	const double shield = shieldsUp ? ShieldPowerRate() : 0;
	const double idlepower = life + sensor + shield;
	return idlepower;
}

double AuBdReport::CalcSensorRange() const
{
	if(SensorVolume() <= 0)
		return 0;

	// A sensor unit is 256 cubic feet.
	const double tlBoost = QualityBoostF(m_Sensor.Quality());
	return (SensorVolume() * CubicFeetPerCubicMeter())/256. * FeetPerMoonOrbit() / 2. * tlBoost;
}

quint32 AuBdReport::BayVolume() const
{
	quint32 v = 0;
	for(QHash<quint16, AuScReportSysVol>::const_iterator bi = m_Bays.begin(); bi != m_Bays.end(); bi++)
		v += (*bi).Volume();
	return v;
}

bool AuBdReport::SetBayVolumes(const QMap<quint16, quint32>& bays)
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

// NOTE: If you change this function be sure to change AuBdDesign::ListComponents.
void AuBdReport::ListComponents(DeProcessComponentArray& pc, const AuScParts& parts) const
{
	// Ascribe parts based on hull volumes.
	if(Structure() == BD_AerialPlatform)
	{
		quint32 gravCouplings = HullVolume() / 1000;	// 1 grav coupling per 1000 cubic meters of hull
		if(!gravCouplings)
			gravCouplings = 1;
		pc.AddRequirement(CID_ElectronicPart, CE_Required, 1 * gravCouplings);
		pc.AddRequirement(CID_GravCoupling, CE_Required, 1 * gravCouplings);
		pc.AddRequirement(CID_Metal, CE_Required, 3 * gravCouplings);
		pc.AddRequirement(CID_MechanicalPart, CE_Required, 2 * gravCouplings);
		pc.AddRequirement(CID_Plastic, CE_Required, 1 * gravCouplings);
	}

	// Life Support
	int lifeSupportUnits = (int)ceil(m_LifeSupport.Volume() * CubicFeetPerCubicMeter() / 256.f);
	if(lifeSupportUnits)
	{
		//pc.AddRequirement(CID_Air, CE_Required, 10 * lifeSupportUnits);
		pc.AddRequirement(CID_ElectronicPart, CE_Required, 1 * lifeSupportUnits);
		pc.AddRequirement(CID_MechanicalPart, CE_Required, 1 * lifeSupportUnits);
		pc.AddRequirement(CID_Metal, CE_Required, 1 * lifeSupportUnits);
		pc.AddRequirement(CID_Plastic, CE_Required, 1 * lifeSupportUnits);
		pc.AddRequirement(CID_Textiles, CE_Required, 1 * lifeSupportUnits);
		if(m_LifeSupport.Module() == CID_LifeSupportGravityModule)
			pc.AddRequirement(CID_GravCoupling, CE_Required, 1 * lifeSupportUnits);
	}

	int powerPlantUnits = (int)ceil(m_PowerPlant.Volume() * CubicFeetPerCubicMeter() / 256.f);
	if(powerPlantUnits)
	{
		if(m_PowerPlant.Module() == CID_NuclearPowerPlantModule ||
				m_PowerPlant.Module() == CID_SolarPowerPlantModule)
			pc.AddRequirement(CID_ElectronicPart, CE_Required, 1 * powerPlantUnits);
		pc.AddRequirement(CID_Metal, CE_Required, 1 * powerPlantUnits);
		pc.AddRequirement(CID_MechanicalPart, CE_Required, 2 * powerPlantUnits);
		if(m_PowerPlant.Module() == CID_NuclearPowerPlantModule)
			pc.AddRequirement(CID_Plastic, CE_Required, 1 * powerPlantUnits);
	}

	int capacitorUnits = (int)ceil(m_Capacitor.Volume() * CubicFeetPerCubicMeter() / 256.f);
	if(capacitorUnits)
	{
		if(m_PowerPlant.Module() == CID_NuclearPowerPlantModule ||
				m_PowerPlant.Module() == CID_SolarPowerPlantModule)
			pc.AddRequirement(CID_ElectronicPart, CE_Required, 1 * powerPlantUnits);
		pc.AddRequirement(CID_Metal, CE_Required, 2 * capacitorUnits);
	}

	int sensorUnits = (int)ceil(m_Sensor.Volume() * CubicFeetPerCubicMeter() / 256.f);
	if(sensorUnits)
	{
		pc.AddRequirement(CID_ElectronicPart, CE_Required, 3 * sensorUnits);
		pc.AddRequirement(CID_MechanicalPart, CE_Required, 1 * sensorUnits);
		pc.AddRequirement(CID_Plastic, CE_Required, 1 * sensorUnits);
	}

	int shieldUnits = (int)ceil(m_Shield.Volume() * CubicFeetPerCubicMeter() / 256.f);
	if(shieldUnits)
	{
		COMMODITYID shieldMod = m_Shield.Module();
		if(shieldMod == CID_KineticShieldModule)
		{
			pc.AddRequirement(CID_ElectronicPart, CE_Required, 5 * shieldUnits);
			pc.AddRequirement(CID_IoTube, CE_Required, 1 * shieldUnits);
			pc.AddRequirement(CID_MagmiumHeatSink, CE_Required, 1 * shieldUnits);
			pc.AddRequirement(CID_Plastic, CE_Required, 1 * shieldUnits);
		}
		else if(shieldMod == CID_CloakingShieldModule)
		{
			pc.AddRequirement(CID_ElectronicPart, CE_Required, 5 * shieldUnits);
			pc.AddRequirement(CID_IoTube, CE_Required, 1 * shieldUnits);
			pc.AddRequirement(CID_Plastic, CE_Required, 1 * shieldUnits);
			pc.AddRequirement(CID_TesloidDissipator, CE_Required, 1 * shieldUnits);
			pc.AddRequirement(CID_VulciumHeatSink, CE_Required, 1 * shieldUnits);
		}
		else	// CID_EnergyShieldModule
		{
			pc.AddRequirement(CID_CryoHeatSink, CE_Required, 1 * shieldUnits);
			pc.AddRequirement(CID_ElectronicPart, CE_Required, 5 * shieldUnits);
			pc.AddRequirement(CID_Plastic, CE_Required, 1 * shieldUnits);
		}
	}

	for(QHash<quint16, AuScReportSysVol>::const_iterator bi = m_Bays.cbegin(); bi != m_Bays.cend(); bi++)
	{
		int bayHits = (int)ceil((*bi).Volume() * CubicFeetPerCubicMeter() / 256.f);
		COMMODITYID bmod = (*bi).Module();
		if(bmod == CID_AtmosphereCondenserBayModule)
		{
			pc.AddRequirement(CID_ElectronicPart, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_Metal, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_MechanicalPart, CE_Required, 2 * bayHits);
		}
		else if(bmod == CID_ProspectorBayModule)
		{
			pc.AddRequirement(CID_ElectronicPart, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_Metal, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_MechanicalPart, CE_Required, 2 * bayHits);
		}
		else if(bmod == CID_SiphonBayModule)
		{
			pc.AddRequirement(CID_ElectronicPart, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_Metal, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_MechanicalPart, CE_Required, 2 * bayHits);
		}
		else if(bmod == CID_WildcatterBayModule)
		{
			pc.AddRequirement(CID_ElectronicPart, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_Metal, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_MechanicalPart, CE_Required, 2 * bayHits);
		}
		else if(bmod == CID_ParticleCollectorBayModule)
		{
			pc.AddRequirement(CID_ElectronicPart, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_Metal, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_MechanicalPart, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_TesloidDissipator, CE_Required, 1 * bayHits);
		}
		else if(bmod == CID_TractorBeamBayModule)
		{
			pc.AddRequirement(CID_CryoHeatSink, CE_Required, 1 * bayHits);
			pc.AddRequirement(CID_ElectronicPart, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_IoTube, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_Metal, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_MechanicalPart, CE_Required, 2 * bayHits);
		}
		else	// CID_WeaponBayModule)
		{
			pc.AddRequirement(CID_ElectronicPart, CE_Required, 1 * bayHits);
			pc.AddRequirement(CID_HeavyWeapon, CE_Required, 1 * bayHits);
			pc.AddRequirement(CID_Metal, CE_Required, 2 * bayHits);
			pc.AddRequirement(CID_MechanicalPart, CE_Required, 2 * bayHits);
		}
	}

	parts.ListComponents(pc, SLINE_None, ArmorLevel(), HullMaterial());
}

quint8 AuBdReport::BayQ() const
{
	quint8 qual = 0;
	for(QHash<quint16, AuScReportSysVol>::const_iterator bi = m_Bays.begin(); bi != m_Bays.end(); bi++)
		if(qual < (*bi).Quality())
			qual = (*bi).Quality();
	return qual;
}

// Fills 'industries' with the ids of all industries that can use this building.
void AuBdReport::Industries(QSet<INDUSTRYID>& industries) const
{
	const DgData& gd = g_Au.LockDataForRead();
	for(DbIndustryArray::const_iterator ii = gd.m_bIndustryArray.cbegin(); ii != gd.m_bIndustryArray.cend(); ii++)
		if(IndustryCanUse(*ii) == 0)
			industries.insert((*ii).IndustryId());
	g_Au.UnlockData();
}

// Returns:
// 0 = Ok
// 1 = Structure Incorrect
// 2 = Field needed
// 3 = Field not allowed
// 4 = Home, Small needed
// 5 = Home, Small not allowed
// 6 = Arena needed
// 7 = Arena not allowed
// 8 = Auditorium needed
// 9 = Auditorium not allowed
// 10 = Battery needed
// 11 = Battery not allowed
// 12 = Commodity Storage needed
// 13 = Commodity Storage not allowed
// 14 = Fuel Storage needed
// 15 = Fuel Storage not allowed
// 16 = Generator needed
// 17 = Generator not allowed
// 18 = Guard post needed
// 19 = Guard post not allowed
// 20 = Home, Medium needed
// 21 = Home, Medium not allowed
// 22 = Livestock needed
// 23 = Livestock not allowed
// 24 = Lounge needed
// 25 = Lounge not allowed
// 26 = Office needed
// 27 = Office not allowed
// 28 = Ground parking spot needed
// 29 = Ground parking spot not allowed
// 30 = Space parking spot needed
// 31 = Space parking spot not allowed
// 32 = Water parking spot needed
// 33 = Water parking spot not allowed
// 34 = Store needed
// 35 = Store not allowed
// 36 = Sensor needed
// 37 = Sensor not allowed
// 38 = Shield needed
// 39 = Shield not allowed
// 40 = Shop needed
// 41 = Shop not allowed
// 42 = Surgery needed
// 43 = Surgery not allowed
// 44 = Transporter needed
// 45 = Transporter not allowed
// 46 = Turret needed
// 47 = Turret not allowed
// 48 = Vehicle launcher needed
// 49 = Vehicle launcher not allowed
// 50 = Vehicle landing system needed
// 51 = Vehicle landing system not allowed
// 52 = Weapon bay needed
// 53 = Weapon bay not allowed
// 54 = Sensor needed for transporter.
// 55 = Sensor needed for weapon.
// 56 = Office needed for industry with shop.
// 57 = Office needed for industry with generator.
// 58 = Home, Large needed
// 59 = Home, Large not allowed
// 60 = Spacecraft parking spot needed
// 61 = Spacecraft parking spot not allowed
// 62 = Power plant module needed.
// 63 = Incompatible power plant module.
int AuBdReport::IndustryCanUse(const DbIndustry& dbi) const
{
	if(Structure() != dbi.Structure())
		return 1;

	if(dbi.Field() > 0)
	{
		if(FieldCapacity() < (quint32)dbi.Field())
			return 2;
	}
	else if(dbi.Field() < 0)
	{
		if(FieldCapacity())
			return 3;
	}

	if(dbi.Arena() > 0)
	{
		if(ArenaCapacity() < (quint32)dbi.Arena())
			return 6;
	}
	else if(dbi.Arena() < 0)
	{
		if(ArenaCapacity())
			return 7;
	}

	if(dbi.Auditorium() > 0)
	{
		if(AuditoriumCapacity() < (quint32)dbi.Auditorium())
			return 8;
	}
	else if(dbi.Auditorium() < 0)
	{
		if(AuditoriumCapacity())
			return 9;
	}

	if(dbi.Battery() > 0)
	{
		if(CapacitorVolume() < (quint32)dbi.Battery())
			return 10;
	}
	else if(dbi.Battery() < 0)
	{
		if(HasCapacitor())
			return 11;
	}

	if(dbi.CargoStorage() > 0)
	{
		if(HoldVolume() < (quint32)dbi.CargoStorage())
			return 12;
	}
	else if(dbi.CargoStorage() < 0)
	{
		if(HasHold())
			return 13;
	}

	if(dbi.Generator() > 0)
	{
		if(GeneratorJobs() < (quint32)dbi.Generator())
			return 16;
	}
	else if(dbi.Generator() < 0)
	{
		if(GeneratorJobs())
			return 17;
	}

	if(dbi.GuardPost() > 0)
	{
		if(GuardPosts().size() < dbi.GuardPost())
			return 18;
	}
	else if(dbi.GuardPost() < 0)
	{
		if(GuardPosts().size())
			return 19;
	}

	if(dbi.HomeLarge() > 0)
	{
		if(HomeLargeCapacity() < (quint32)dbi.HomeLarge())
			return 58;
	}
	else if(dbi.HomeLarge() < 0)
	{
		if(HomeLargeCapacity())
			return 59;
	}

	if(dbi.HomeMedium() > 0)
	{
		// Allow larger homes to fulfill the requirment.
		if(HomeMediumCapacity() + HomeLargeCapacity() < (quint32)dbi.HomeMedium())
			return 20;
	}
	else if(dbi.HomeMedium() < 0)
	{
		if(HomeMediumCapacity())
			return 21;
	}

	if(dbi.HomeSmall() > 0)
	{
		// Allow larger homes to fulfill the requirment.
		if(HomeSmallCapacity() + HomeMediumCapacity() + HomeLargeCapacity() < (quint32)dbi.HomeSmall())
			return 4;
	}
	else if(dbi.HomeSmall() < 0)
	{
		if(HomeSmallCapacity())
			return 5;
	}

	if(dbi.LivestockPost() > 0)
	{
		if(LivestockPosts().size() < dbi.LivestockPost())
			return 22;
	}
	else if(dbi.LivestockPost() < 0)
	{
		if(LivestockPosts().size())
			return 23;
	}

	if(dbi.Lounge() > 0)
	{
		if(LoungeCapacity() < (quint32)dbi.Lounge())
			return 24;
	}
	else if(dbi.Lounge() < 0)
	{
		if(LoungeCapacity())
			return 25;
	}

	if(dbi.Office() > 0)
	{
		if(OfficeCapacity() < (quint32)dbi.Office())
			return 26;
	}
	else if(dbi.Office() < 0)
	{
		if(OfficeCapacity())
			return 27;
	}

	if(dbi.ParkGround() > 0)
	{
		if(ParkingSpotsGround() < dbi.ParkGround())
			return 28;
	}
	else if(dbi.ParkGround() < 0)
	{
		if(ParkingSpotsGround())
			return 29;
	}

	if(dbi.ParkSpace() > 0)
	{
		if(ParkingSpotsSpace() + ParkingSpotsRocket() < dbi.ParkSpace())
			return 30;
	}
	else if(dbi.ParkSpace() < 0)
	{
		if(ParkingSpotsSpace() + ParkingSpotsRocket())
			return 31;
	}

	if(dbi.ParkSpacecraft() > 0)
	{
		if(ParkingSpotsSpacecraft() < dbi.ParkSpacecraft())
			return 60;
	}
	else if(dbi.ParkSpacecraft() < 0)
	{
		if(ParkingSpotsSpacecraft())
			return 61;
	}

	if(dbi.ParkWater() > 0)
	{
		if(ParkingSpotsWater() < dbi.ParkWater())
			return 32;
	}
	else if(dbi.ParkWater() < 0)
	{
		if(ParkingSpotsWater())
			return 33;
	}

	if(dbi.Store() > 0)
	{
		if(StoreCapacity() < (quint32)dbi.Store())
			return 34;
	}
	else if(dbi.Store() < 0)
	{
		if(StoreCapacity())
			return 35;
	}

	if(dbi.Sensor() > 0)
	{
		if(SensorVolume() < (quint32)dbi.Sensor())
			return 36;
	}
	else if(dbi.Sensor() < 0)
	{
		if(HasSensor())
			return 37;
	}

	if(dbi.Shield() > 0)
	{
		if(ShieldVolume() < (quint32)dbi.Shield())
			return 38;
	}
	else if(dbi.Shield() < 0)
	{
		if(HasShield())
			return 39;
	}

	if(dbi.Shop() > 0)
	{
		if(ShopCapacity() < (quint32)dbi.Shop())
			return 40;
	}
	else if(dbi.Shop() < 0)
	{
		if(ShopCapacity())
			return 41;
	}

	if(dbi.Surgery() > 0)
	{
		if(SurgeryUnitCount() < dbi.Surgery())
			return 42;
	}
	else if(dbi.Surgery() < 0)
	{
		if(HasSurgeryUnit())
			return 43;
	}

	if(dbi.Transporter() > 0)
	{
		if(Transporters().size() < dbi.Transporter())
			return 44;
	}
	else if(dbi.Transporter() < 0)
	{
		if(HasTransporter())
			return 45;
	}

	if(dbi.Turret() > 0)
	{
		if(Turrets().size() < dbi.Turret())
			return 46;
	}
	else if(dbi.Turret() < 0)
	{
		if(HasTurret())
			return 47;
	}

	if(dbi.VehicleLauncher() > 0)
	{
		if(VehicleLaunchers().size() < dbi.VehicleLauncher())
			return 48;
	}
	else if(dbi.VehicleLauncher() < 0)
	{
		if(HasVehicleLauncher())
			return 49;
	}

	if(dbi.VehicleRecovery() > 0)
	{
		if(VehicleLandingSystems().size() < dbi.VehicleRecovery())
			return 50;
	}
	else if(dbi.VehicleRecovery() < 0)
	{
		if(HasVehicleLandingSystem())
			return 51;
	}

	if(dbi.WeaponBay() > 0)
	{
		if(WeaponBays().size() < dbi.WeaponBay())
			return 52;
	}
	else if(dbi.WeaponBay() < 0)
	{
		if(HasWeaponBay())
			return 53;
	}

	if(HasTransporter() && !HasSensor())
		return 54;
	//if(HasWeaponBay() && !HasSensor())
		//return 55;
	if(ShopCapacity() && !OfficeCapacity())
		return 56;
	if(GeneratorJobs() && !OfficeCapacity())
		return 57;

	// Power plant requires a compatible power plant module.
	switch(dbi.IndustryId())
	{
	default:
		break;
	case IID_CoalPowerPlant:
		if(PowerPlantModule() == CID_Invalid)
			return 62;
		if(PowerPlantModule() != CID_CoalPowerPlantModule)
			return 63;
		break;
	case IID_GasolinePowerPlant:
		if(PowerPlantModule() == CID_Invalid)
			return 62;
		if(PowerPlantModule() != CID_GasolinePowerPlantModule)
			return 63;
		break;
	case IID_HydroelectricPowerPlant:
		if(PowerPlantModule() == CID_Invalid)
			return 62;
		if(PowerPlantModule() != CID_HydroelectricPowerPlantModule)
			return 63;
		break;
	case IID_HydrogenPowerPlant:
		if(PowerPlantModule() == CID_Invalid)
			return 62;
		if(PowerPlantModule() != CID_HydrogenPowerPlantModule)
			return 63;
		break;
	case IID_NaturalGasPowerPlant:
		if(PowerPlantModule() == CID_Invalid)
			return 62;
		if(PowerPlantModule() != CID_NaturalGasPowerPlantModule)
			return 63;
		break;
	case IID_NuclearPowerPlant:
		if(PowerPlantModule() == CID_Invalid)
			return 62;
		if(PowerPlantModule() != CID_NuclearPowerPlantModule)
			return 63;
		break;
	case IID_SolarPowerPlant:
		if(PowerPlantModule() == CID_Invalid)
			return 62;
		if(PowerPlantModule() != CID_SolarPowerPlantModule)
			return 63;
		break;
	case IID_WindPowerPlant:
		if(PowerPlantModule() == CID_Invalid)
			return 62;
		if(PowerPlantModule() != CID_WindPowerPlantModule)
			return 63;
		break;
	}

	return 0;	// Success if we made it this far.
}

//static
QString AuBdReport::IndustryCanUseMessage(int resultCode)
{
	QString resultMsg[] =
	{
		QObject::tr("Ok"),
		QObject::tr("Structure Incorrect"),
		QObject::tr("Field needed"),
		QObject::tr("Field not allowed"),
		QObject::tr("Small home needed"),
		QObject::tr("Small home not allowed"),
		QObject::tr("Arena needed"),
		QObject::tr("Arena not allowed"),
		QObject::tr("Auditorium needed"),
		QObject::tr("Auditorium not allowed"),
		QObject::tr("Battery needed"),
		QObject::tr("Battery not allowed"),
		QObject::tr("Commodity storage needed"),
		QObject::tr("Commodity storage not allowed"),
		QObject::tr("Fuel storage needed"),
		QObject::tr("Fuel storage not allowed"),
		QObject::tr("Generator needed"),
		QObject::tr("Generator not allowed"),
		QObject::tr("Guard post needed"),
		QObject::tr("Guard post not allowed"),
		QObject::tr("Medium home needed"),
		QObject::tr("Medium home not allowed"),
		QObject::tr("Livestock needed"),
		QObject::tr("Livestock not allowed"),
		QObject::tr("Lounge needed"),
		QObject::tr("Lounge not allowed"),
		QObject::tr("Office needed"),
		QObject::tr("Office not allowed"),
		QObject::tr("Ground parking spot needed"),
		QObject::tr("Ground parking spot not allowed"),
		QObject::tr("Space parking spot needed"),
		QObject::tr("Space parking spot not allowed"),
		QObject::tr("Water parking spot needed"),
		QObject::tr("Water parking spot not allowed"),
		QObject::tr("Store needed"),
		QObject::tr("Store not allowed"),
		QObject::tr("Sensor needed"),
		QObject::tr("Sensor not allowed"),
		QObject::tr("Shield needed"),
		QObject::tr("Shield not allowed"),
		QObject::tr("Workshop needed"),
		QObject::tr("Workshop not allowed"),
		QObject::tr("Surgery needed"),
		QObject::tr("Surgery not allowed"),
		QObject::tr("Transporter needed"),
		QObject::tr("Transporter not allowed"),
		QObject::tr("Turret needed"),
		QObject::tr("Turret not allowed"),
		QObject::tr("Vehicle launcher needed"),
		QObject::tr("Vehicle launcher not allowed"),
		QObject::tr("Vehicle landing system needed"),
		QObject::tr("Vehicle landing system not allowed"),
		QObject::tr("Weapon bay needed"),
		QObject::tr("Weapon bay not allowed"),
		QObject::tr("Sensor needed for transporter"),
		QObject::tr("Sensor needed for weapon"),
		QObject::tr("Office needed for workshop"),
		QObject::tr("Office needed for generator"),
		QObject::tr("Large home needed"),
		QObject::tr("Large home not allowed"),
		QObject::tr("Spacecraft parking spot needed"),
		QObject::tr("Spacecraft parking spot not allowed"),
		QObject::tr("Generator module needed."),
		QObject::tr("Incompatible generator module."),
	};
	int rc = sizeof(resultMsg) / sizeof(QString);
	if(resultCode >= rc)
		return QObject::tr("Failed for unknown reason %1.").arg(resultCode);
	return resultMsg[resultCode];
}

// Returns html formatted info about the result of using this building for industry 'dbi'.
void AuBdReport::IndustryInfo(const DbIndustry& dbi, quint32& homesLarge, quint32& homesMedium, quint32& homesSmall,
		quint32& manufacturingJobs, quint32& serviceJobs, quint32& stores, quint32& lounges, quint32& proJobs) const
{
	// Insure the building meets code.
	if(IndustryCanUse(dbi))
	{
		homesLarge = 0;
		homesMedium = 0;
		homesSmall = 0;
		lounges = 0;
		manufacturingJobs = 0;
		proJobs = 0;
		serviceJobs = 0;
		stores = 0;
		return;
	}

	// Report what the building has to offer.
	homesLarge = m_HomeLargeCapacity;
	homesMedium = m_HomeMediumCapacity;
	homesSmall = m_HomeSmallCapacity;
	lounges = m_LoungeCapacity;
	stores = m_StoreCapacity;

	serviceJobs = //m_Bays.size() +
		//m_GuardPosts.size() +
		//(HasSensor() ? 1 : 0) +
		//(HasShield() ? 1 : 0) +
		//SurgeryUnitCount() +
		m_Transporters.size() +
		//m_Turrets.size() +
		m_VehicleLaunchers.size() +
		m_VehicleLandingSystems.size();

	manufacturingJobs = 0;
	proJobs = 0;
	if(dbi.DoesManufacture())
	{
		serviceJobs += m_ArenaCapacity +
			m_LoungeCapacity +
			m_OfficeCapacity +
			m_StoreCapacity;

		// Only buildings that manufacture things can create manufacturing jobs. This includes patent development.
		if(dbi.IndustryId() == IID_SpacecraftFactory ||
				dbi.IndustryId() == IID_AircraftFactory ||
				dbi.IndustryId() == IID_ShipYard ||
				dbi.IndustryId() == IID_VehicleFactory)
			manufacturingJobs = 1;								// These industries are limited to one process.
		else
		{
			if(dbi.Auditorium() > 0)
				manufacturingJobs = m_AuditoriumCapacity;		// Auditoriums create manufacturing jobs if required. University.
			else if(dbi.Auditorium() == 0)
				serviceJobs += m_AuditoriumCapacity;			// Auditoriums always create jobs when permitted.

			if(!manufacturingJobs)
			{
				if(dbi.Field() > 0)
					manufacturingJobs = m_FieldCapacity;		// Fields create manufacturing jobs if required. Agriculture.
				else if(dbi.Generator() > 0)
					manufacturingJobs = GeneratorJobs();		// Generators create manufacturing jobs if required. Power plants.
				else if(dbi.Shop() >= 0)
				{
					if(m_ShopCapacity)
						manufacturingJobs = m_ShopCapacity;		// Shops create manufacturing jobs if required. Factories.
					else if(dbi.IndustryId() == IID_Mine)
					{
						// A mine with no shop uses its office, if it has one, as a shop to mine stone, to make noob startup easier.
						manufacturingJobs = 1;
						if(m_OfficeCapacity)
							serviceJobs--;
					}
				}
			}
		}
	}
	else
	{
		if(dbi.Arena() > 0)
			proJobs += m_ArenaCapacity;
		else
			serviceJobs += m_ArenaCapacity;

		if(dbi.Auditorium() > 0)
			proJobs += m_AuditoriumCapacity;
		else
			serviceJobs += m_AuditoriumCapacity;				// Auditoriums always create jobs. Capitol.

		if(dbi.Field() > 0)
			proJobs += m_FieldCapacity;
		//else
			//serviceJobs += m_FieldCapacity;					// Fields create jobs if required. Zoo.

		if(dbi.GuardPost() > 0)
			proJobs += GuardPosts().size();
		else
			serviceJobs += GuardPosts().size();

		if(dbi.Lounge() > 0)
			proJobs += m_LoungeCapacity;
		else
			serviceJobs += m_LoungeCapacity;

		if(dbi.Sensor() > 0)
			proJobs += HasSensor() ? 1 : 0;
		else
			serviceJobs += HasSensor() ? 1 : 0;

		if(dbi.Shield() > 0)
			proJobs += HasShield() ? 1 : 0;
		else
			serviceJobs += HasShield() ? 1 : 0;

		if(dbi.Shop() > 0)
			proJobs += m_ShopCapacity;
		else
			serviceJobs += m_ShopCapacity;

		if(dbi.Store() > 0)
			proJobs += m_StoreCapacity;
		else
			serviceJobs += m_StoreCapacity;

		if(dbi.Turret() > 0)
			proJobs += Turrets().size();
		else
			serviceJobs += Turrets().size();

		if(dbi.WeaponBay() > 0)
			proJobs += WeaponBays().size();
		else
			serviceJobs += WeaponBays().size();

		// Many buildings require an office. If they don't provide proJobs from other specific things, then base it on offices.
		// This picks up hospitals and police departments. Who knows what else.
		if(dbi.Office() > 0 && proJobs == 0)
			proJobs += m_OfficeCapacity;
		else
			serviceJobs += m_OfficeCapacity;
	}
}

void AuBdReport::SetConstructionMaterial(COMMODITYID cid)
{
	COMMODITYID hullMod = HullMaterialModule(cid);
	if(hullMod == CID_Invalid)
		return;
	m_Hull.SetModule(hullMod);
}

double AuBdReport::RoughHullRadius() const
{
	double max2 = HullMax().Length2();
	double min2 = HullMin().Length2();
	if(max2 > min2)
		return sqrt(max2);
	else
		return sqrt(min2);
}

double AuBdReport::RoughSiteRadius() const
{
	double max2 = SiteMax().Length2();
	double min2 = SiteMin().Length2();
	if(max2 > min2)
		return sqrt(max2);
	else
		return sqrt(min2);
}

bool AuBdReport::ArmorOkForFrigidLand() const
{
	return ArmorLevel() >= 1;
}

bool AuBdReport::ArmorOkForFrigidSeaBottom() const
{
	return ArmorLevel() >= 4;
}

bool AuBdReport::ArmorOkForGasGiantCore() const
{
	return ArmorLevel() >= 5;
}

bool AuBdReport::ArmorOkForInfernoLand() const
{
	return ArmorLevel() >= 2;
}

bool AuBdReport::ArmorOkForSeaBottom() const
{
	return ArmorLevel() >= 3;
}

bool AuBdReport::SkeletonOkForFrigidLand() const
{
	return SkeletonLevel() >= .1f/2.f;	// The div by 2 makes the result match the test using armor.
}

bool AuBdReport::SkeletonOkForFrigidSeaBottom() const
{
	return SkeletonLevel() >= .4f/2.f;	// The div by 2 makes the result match the test using armor.
}

bool AuBdReport::SkeletonOkForGasGiantCore() const
{
	return SkeletonLevel() >= .5f/2.f;	// The div by 2 makes the result match the test using armor.
}

bool AuBdReport::SkeletonOkForInfernoLand() const
{
	return SkeletonLevel() >= .2f/2.f;	// The div by 2 makes the result match the test using armor.
}

bool AuBdReport::SkeletonOkForSeaBottom() const
{
	return SkeletonLevel() >= .3f/2.f;	// The div by 2 makes the result match the test using armor.
}

int AuBdReport::LifeSupportNominalDemand() const
{
	int lsnom = (m_ArenaCapacity*3) +
		m_AuditoriumCapacity +
		(m_FieldCapacity*5) +
		m_HomeLargeCapacity +
		m_HomeMediumCapacity +
		m_HomeSmallCapacity +
		m_LoungeCapacity +
		m_OfficeCapacity +
		m_ShopCapacity +
		m_StoreCapacity;
	if(LifeSupportModule() != CID_Invalid && (Structure() == BD_Biodome || IsPlatform()))
		lsnom += 1;	// Biodomes and platforms need at least one unit of life support if they have a life support module.
	return lsnom;
}

bool AuBdReport::LifeSupportOkForFrigidLand() const
{
	unsigned int lsnom = LifeSupportNominalDemand();
	return LifeSupportVolume() >= lsnom * 3;
}

bool AuBdReport::LifeSupportOkForFrigidSeaBottom() const
{
	unsigned int lsnom = LifeSupportNominalDemand();
	return LifeSupportVolume() >= lsnom * 2;
}

bool AuBdReport::LifeSupportOkForGasGiantCore() const
{
	unsigned int lsnom = LifeSupportNominalDemand();
	return LifeSupportVolume() >= lsnom * 7 / 2;
}

bool AuBdReport::LifeSupportOkForInfernoLand() const
{
	unsigned int lsnom = LifeSupportNominalDemand();
	return LifeSupportVolume() >= lsnom * 4;
}

bool AuBdReport::LifeSupportOkForSeaBottom() const
{
	unsigned int lsnom = LifeSupportNominalDemand();
	return LifeSupportVolume() >= (lsnom + 1) / 2;
}

bool AuBdReport::LifeSupportOkForVacuumLand() const
{
	unsigned int lsnom = LifeSupportNominalDemand();
	return LifeSupportVolume() >= lsnom;
}

bool AuBdReport::CouldBeOccupied() const
{
	if(m_HomeLargeCapacity ||
			m_HomeMediumCapacity ||
			m_HomeSmallCapacity)
		return true;	// Could be occupied by residents.
	quint32 jobs = m_ArenaCapacity +
		m_Bays.size() +
		m_GuardPosts.size() +
		m_LoungeCapacity +
		m_OfficeCapacity +
		(HasSensor() ? 1 : 0) +
		(HasShield() ? 1 : 0) +
		m_StoreCapacity +
		SurgeryUnitCount() +
		m_Transporters.size() +
		m_Turrets.size() +
		m_VehicleLaunchers.size() +
		m_VehicleLandingSystems.size();
	if(jobs)
		return true;
	if(m_AuditoriumCapacity)
		return true;
	if(m_FieldCapacity)
		return true;
	if(GeneratorJobs())
		return true;	// Enough power plant to employ a worker.
	return false;
}

quint64 AuBdReport::MemoryUsage() const
{
	quint64 mem = 0;
	mem += sizeof(m_ArchitectId);
	mem += StringMemoryUsage(m_ArchitectName);
	mem += StringMemoryUsage(m_DesignDesc);
	mem += sizeof(m_DesignId);
	mem += StringMemoryUsage(m_DesignName);
	mem += m_PartArchitects.MemoryUsage();

	mem += sizeof(m_Structure);
	mem += sizeof(m_IndustryId);

	mem += sizeof(m_Hull);
	mem += sizeof(m_HullMax);
	mem += sizeof(m_HullMin);
	mem += sizeof(m_SiteMax);
	mem += sizeof(m_SiteMin);
	mem += sizeof(m_ArenaVolume);
	mem += sizeof(m_ArmorVolume);
	mem += sizeof(m_AuditoriumVolume);
	mem += m_Bays.MemoryUsage();
	mem += sizeof(m_Capacitor);
	mem += sizeof(m_FieldArea);
	mem += sizeof(m_HoldVolume);
	mem += sizeof(m_HomeLargeVolume);
	mem += sizeof(m_HomeMediumVolume);
	mem += sizeof(m_HomeSmallVolume);
	mem += sizeof(m_LifeSupport);
	mem += sizeof(m_LoungeVolume);
	mem += sizeof(m_OfficeVolume);
	mem += sizeof(m_PowerPlant);
	mem += sizeof(m_Sensor);
	mem += sizeof(m_Shield);
	mem += sizeof(m_ShopVolume);
	mem += sizeof(m_StoreVolume);
	mem += sizeof(m_SkeletonVolume);
	mem += m_Turrets.MemoryUsage();

	mem += sizeof(m_Quality);
	mem += sizeof(m_CnCModule);
	mem += sizeof(m_CnCQ);
	mem += m_ConProc.MemoryUsage();

	mem += m_CitizenPosts.MemoryUsage();
	mem += m_CommandStations.MemoryUsage();
	mem += m_CrewStations.MemoryUsage();
	mem += m_GameTables.MemoryUsage();
	mem += m_GuardPosts.MemoryUsage();
	mem += m_Lifts.MemoryUsage();
	mem += m_LivestockPosts.MemoryUsage();
	mem += m_StatusDisplays.MemoryUsage();
	mem += m_Transporters.MemoryUsage();
	mem += m_VehicleLandingSystems.MemoryUsage();
	mem += m_VehicleLaunchers.MemoryUsage();

	mem += sizeof(m_Beds);

	mem += sizeof(m_ParkGround);
	mem += sizeof(m_ParkRocket);
	mem += sizeof(m_ParkSpace);
	mem += sizeof(m_ParkSpacecraft);
	mem += sizeof(m_ParkWater);

	mem += sizeof(m_MassKg);
	mem += sizeof(m_MassEquipmentKg);
	mem += sizeof(m_MassHullKg);

	mem += sizeof(m_ArenaCapacity);
	mem += sizeof(m_AuditoriumCapacity);
	mem += sizeof(m_CapacitorMax);
	mem += sizeof(m_FaceCount);
	mem += sizeof(m_FieldCapacity);
	mem += sizeof(m_FuelBurnRate);
	mem += sizeof(m_HasBar);
	mem += sizeof(m_HasEnvironmentVoid);
	mem += sizeof(m_HasPantry);
	mem += sizeof(m_HoldCapacity);
	mem += sizeof(m_HomeLargeCapacity);
	mem += sizeof(m_HomeMediumCapacity);
	mem += sizeof(m_HomeSmallCapacity);
	mem += sizeof(m_IdlePowerWithShields);
	mem += sizeof(m_LoungeCapacity);
	mem += sizeof(m_OfficeCapacity);
	mem += StringMemoryUsage(m_PartsList);
	mem += sizeof(m_SensorRange);
	mem += sizeof(m_ShopCapacity);
	mem += sizeof(m_StoreCapacity);
	mem += sizeof(m_TriangleCount);
	return mem;
}

double AuBdReport::PoliceJurisdictionRadius() const
{
	// Police stations are required to have at least 2 offices.
	quint32 offices = OfficeCapacity();
	if(offices < 1)
		return 0;
	//double oArea = (d_PoliceRadius * d_PoliceRadius * PI / 2);		// Coverage area of each police station office.
	//double jArea = oArea * OfficeCapacity();
	//double jRadius = round(sqrt(jArea/PI));	// Radius in feet.
	//return jRadius;
	return round(sqrt(d_PoliceRadius*d_PoliceRadius / 2 * offices));	// Simplified.
}

double AuBdReport::CapitolJurisdictionRadius() const
{
	return d_CapitolRadius;
}

//static
double AuBdReport::MaxSensorRange()
{
	// A sensor unit is 256 cubic feet.
	const double tlBoost = QualityBoostF(255);
	return (AuBdDesign::MaxVolume() * CubicFeetPerCubicMeter())/256. * FeetPerMoonOrbit() / 2. * tlBoost;
}

double AuBdReport::SensorRangeDeepSpace() const
{
	// Tests using Airport Radar 1000, which has 1000 cubic meters of radar.
	//return sqrt(MaxSensorRange() / FeetPerMeter()) / 500.;	// 964.2pc
	//return sqrt(m_SensorRange / FeetPerMeter()) / 500.;	// 1.8pc
	//return sqrt(MaxSensorRange()) / 1000.;	// 873.2pc
	//return sqrt(m_SensorRange) / 1000.;	// 1.6pc
	//return cbrt(MaxSensorRange() / FeetPerMeter());	// 6,148.1pc
	//return cbrt(m_SensorRange / FeetPerMeter());	// 93.0pc
	//return cbrt(MaxSensorRange());	// 9,135.6pc
	//return cbrt(m_SensorRange);	// 138.2pc
	//return cbrt(MaxSensorRange() / FeetPerMeter()) / 10.;	// 614.8pc
	//return cbrt(m_SensorRange / FeetPerMeter()) / 10.;	// 9.3pc
	return AuDesign::SensorRangeDeepSpace(m_SensorRange);
}

float AuBdReport::SkeletonLevel() const
{
	if(!m_Hull.Volume())
		return 0;
	return (float)m_SkeletonVolume / (float)m_Hull.Volume();
}

quint8 AuBdReport::EnvironmentBits() const
{
	quint8 benv = 0;
	// All blueprints are ok for habitable land construction, so far.
	benv |= BENV_HabitableLand;			// Habitable environment land.

	if(LifeSupportOkForVacuumLand())
		benv |= BENV_UnbreathableLand;	// Uninhabitable environment land, inner through outer zones.

	if((SkeletonOkForFrigidLand() || ArmorOkForFrigidLand()) && LifeSupportOkForFrigidLand())
		benv |= BENV_FrigidLand;		// Land on frigid world.

	if((SkeletonOkForInfernoLand() || ArmorOkForInfernoLand()) && LifeSupportOkForInfernoLand())
		benv |= BENV_InfernoLand;		// Land on inferno world.

	if((SkeletonOkForSeaBottom() || ArmorOkForSeaBottom()) && LifeSupportOkForSeaBottom())
		benv |= BENV_SeaBottom;			// Bottom of water sea.

	if((SkeletonOkForFrigidSeaBottom() || ArmorOkForFrigidSeaBottom()) && LifeSupportOkForFrigidSeaBottom())
		benv |= BENV_FrigidSeaBottom;	// Bottom of supercooled liquid sea on frigid world.

	if((SkeletonOkForGasGiantCore() || ArmorOkForGasGiantCore()) && LifeSupportOkForGasGiantCore())
		benv |= BENV_GasGiantCore;		// Bottom of supercooled liquid at core of gas giant.

	return benv;
}

quint16 AuBdReport::MedicalStationInRoom(quint16 roomId) const
{
	for(AuScReportStationHash::const_iterator ci = m_CrewStations.cbegin(); ci != m_CrewStations.cend(); ci++)
		if((*ci).IsMedic(roomId))
			return ci.key();
	return false;
}

quint16 AuBdReport::TransporterStationInRoom(quint16 roomId) const
{
	for(AuScReportTransporterStationHash::const_iterator ci = m_Transporters.cbegin(); ci != m_Transporters.cend(); ci++)
		if((*ci).RoomId() == roomId)
			return ci.key();
	return false;
}

void AuBdReport::BuildZoneCheckLocs(AuPointArray& siteLocs) const
{
	// Add my site center point. It is a good one for the caller to test first.
	AuPoint siteCen(SiteCenter());
	siteLocs.append(siteCen);
	// Add perimeter points. Order doesn't really matter.
	siteLocs.append(AuPoint(m_SiteMin.x, m_SiteMin.y));
	siteLocs.append(AuPoint(m_SiteMin.x, 0));
	siteLocs.append(AuPoint(m_SiteMin.x, m_SiteMax.y));
	siteLocs.append(AuPoint(0, m_SiteMax.y));
	siteLocs.append(AuPoint(m_SiteMax.x, m_SiteMax.y));
	siteLocs.append(AuPoint(m_SiteMax.x, 0));
	siteLocs.append(AuPoint(m_SiteMax.x, m_SiteMin.y));
	siteLocs.append(AuPoint(0, m_SiteMin.y));
}

quint32 AuBdReport::SystemHits(const AuSysId& sysId) const
{
	switch(sysId.System())
	{
	case SYS_CommandAndControl:
		if(sysId.Which())
			return 1;
		return CommandStationCount() + CrewStationCount();
	case SYS_Fuel:
	case SYS_ManeuverDrive:
		break;
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
		break;
	case SYS_Capacitor:
		return CapacitorHits();
	case SYS_Count:
		break;
	}
	return 0;
}

quint32 AuBdReport::HullSystemHits() const
{
	// Subtract hits from other hull-based systems.
	// Flight deck hits are not calculated from hull volume so they are not subtracted here.
	return HullHits() - (HoldHits() + SkeletonHits());
}

void AuBdReport::SystemIds(QSet<AuSysId>& sysIds, bool structural) const
{
	if(structural)
	{
		//if(FuelVolume())
			//sysIds.insert(SYS_Fuel);
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

		if(HasLifeSupport())
			sysIds.insert(SYS_LifeSupport);
		if(HasGenerator())
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

		//for(AuScReportSysVolHash::const_iterator si = m_FTLDrives.cbegin(); si != m_FTLDrives.cend(); si++)
			//sysIds.insert(AuSysId(SYS_FTLDrive, si.key()));

		for(QSet<quint16>::const_iterator si = m_VehicleLandingSystems.cbegin(); si != m_VehicleLandingSystems.cend(); si++)
			sysIds.insert(AuSysId(SYS_VehicleLanding, *si));

		for(QSet<quint16>::const_iterator si = m_VehicleLaunchers.cbegin(); si != m_VehicleLaunchers.cend(); si++)
			sysIds.insert(AuSysId(SYS_VehicleLaunch, *si));

		if(HasCapacitor())
			sysIds.insert(SYS_Capacitor);
	}
}

