//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auplayerstats.cpp,v 1.84 2022/02/19 17:40:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auplayerstats.h"
//
#include "auspecies.h"
#include "daanimal.h"
#include "danpc.h"
#include "dastar.h"
#include "daunit.h"
#include "davehicle.h"
#include "davessel.h"
#include "daworldgasgiant.h"
#include "daworldglobe.h"
#include "daworldringsection.h"
#include "dhdna.h"

QString StarStatName(STAR_STAT_TYPE st)
{
	if(st < SST_O_Ia || st > SST_Stellar_Black_Hole)
		return QString("Invalid Star Type %1").arg(st);

	DaStar::TYPE types[] =
	{
		DaStar::TYPE_O,
		DaStar::TYPE_O,
		DaStar::TYPE_O,
		DaStar::TYPE_O,
		DaStar::TYPE_O,
		DaStar::TYPE_O,
		DaStar::TYPE_O,
		DaStar::TYPE_O,
		DaStar::TYPE_B,
		DaStar::TYPE_B,
		DaStar::TYPE_B,
		DaStar::TYPE_B,
		DaStar::TYPE_B,
		DaStar::TYPE_B,
		DaStar::TYPE_B,
		DaStar::TYPE_B,
		DaStar::TYPE_A,
		DaStar::TYPE_A,
		DaStar::TYPE_A,
		DaStar::TYPE_A,
		DaStar::TYPE_A,
		DaStar::TYPE_A,
		DaStar::TYPE_A,
		DaStar::TYPE_A,
		DaStar::TYPE_F,
		DaStar::TYPE_F,
		DaStar::TYPE_F,
		DaStar::TYPE_F,
		DaStar::TYPE_F,
		DaStar::TYPE_F,
		DaStar::TYPE_F,
		DaStar::TYPE_F,
		DaStar::TYPE_G,
		DaStar::TYPE_G,
		DaStar::TYPE_G,
		DaStar::TYPE_G,
		DaStar::TYPE_G,
		DaStar::TYPE_G,
		DaStar::TYPE_G,
		DaStar::TYPE_G,
		DaStar::TYPE_K,
		DaStar::TYPE_K,
		DaStar::TYPE_K,
		DaStar::TYPE_K,
		DaStar::TYPE_K,
		DaStar::TYPE_K,
		DaStar::TYPE_K,
		DaStar::TYPE_K,
		DaStar::TYPE_M,
		DaStar::TYPE_M,
		DaStar::TYPE_M,
		DaStar::TYPE_M,
		DaStar::TYPE_M,
		DaStar::TYPE_M,
		DaStar::TYPE_M,
		DaStar::TYPE_M,
		DaStar::TYPE_SupermassiveBlackHole,
		DaStar::TYPE_StellarBlackHole,
		DaStar::TYPE_NeutronStar,
	};

	DaStar::SIZE sizes[] =
	{
		DaStar::SIZE_Ia,
		DaStar::SIZE_Ib,
		DaStar::SIZE_II,
		DaStar::SIZE_III,
		DaStar::SIZE_IV,
		DaStar::SIZE_V,
		DaStar::SIZE_VI,
		DaStar::SIZE_D,
		DaStar::SIZE_Ia,
		DaStar::SIZE_Ib,
		DaStar::SIZE_II,
		DaStar::SIZE_III,
		DaStar::SIZE_IV,
		DaStar::SIZE_V,
		DaStar::SIZE_VI,
		DaStar::SIZE_D,
		DaStar::SIZE_Ia,
		DaStar::SIZE_Ib,
		DaStar::SIZE_II,
		DaStar::SIZE_III,
		DaStar::SIZE_IV,
		DaStar::SIZE_V,
		DaStar::SIZE_VI,
		DaStar::SIZE_D,
		DaStar::SIZE_Ia,
		DaStar::SIZE_Ib,
		DaStar::SIZE_II,
		DaStar::SIZE_III,
		DaStar::SIZE_IV,
		DaStar::SIZE_V,
		DaStar::SIZE_VI,
		DaStar::SIZE_D,
		DaStar::SIZE_Ia,
		DaStar::SIZE_Ib,
		DaStar::SIZE_II,
		DaStar::SIZE_III,
		DaStar::SIZE_IV,
		DaStar::SIZE_V,
		DaStar::SIZE_VI,
		DaStar::SIZE_D,
		DaStar::SIZE_Ia,
		DaStar::SIZE_Ib,
		DaStar::SIZE_II,
		DaStar::SIZE_III,
		DaStar::SIZE_IV,
		DaStar::SIZE_V,
		DaStar::SIZE_VI,
		DaStar::SIZE_D,
		DaStar::SIZE_Ia,
		DaStar::SIZE_Ib,
		DaStar::SIZE_II,
		DaStar::SIZE_III,
		DaStar::SIZE_IV,
		DaStar::SIZE_V,
		DaStar::SIZE_VI,
		DaStar::SIZE_D,
		DaStar::SIZE_Ia					// Always SIZE_Ia for black hole
	};

	return DaStar::Name(types[st], sizes[st]);
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuPlayerStatsItem::AuPlayerStatsItem()
{
	m_BoughtCargo = 0;
	m_BoughtInventory = 0;
	m_CraftedAtIndustry = 0;
	m_CraftedByHand = 0;
	m_Foraged = 0;
	m_HadInventory = false;
	m_HighestQ = 0;
	m_KilledBy = 0;
	m_Kills = 0;
	m_SoldCargo = 0;
	m_SoldInventory = 0;
	m_UsesAsWeapon = 0;
	m_UsesOnOther = 0;
	m_UsesOnSelf = 0;
}

void AuPlayerStatsItem::operator<<(QDataStream& in)
{
	in >> m_BoughtCargo;
	in >> m_BoughtInventory;
	in >> m_CraftedAtIndustry;
	in >> m_CraftedByHand;
	in >> m_Foraged;
	in >> m_HadInventory;
	in >> m_HighestQ;
	quint8 htl;
	in >> htl;
	in >> m_KilledBy;
	in >> m_Kills;
	in >> m_SoldCargo;
	in >> m_SoldInventory;
	in >> m_UsesAsWeapon;
	in >> m_UsesOnOther;
	in >> m_UsesOnSelf;
}

void AuPlayerStatsItem::operator>>(QDataStream& out) const
{
	out << m_BoughtCargo;
	out << m_BoughtInventory;
	out << m_CraftedAtIndustry;
	out << m_CraftedByHand;
	out << m_Foraged;
	out << m_HadInventory;
	out << m_HighestQ;
	quint8 htl = 0;
	out << htl;
	out << m_KilledBy;
	out << m_Kills;
	out << m_SoldCargo;
	out << m_SoldInventory;
	out << m_UsesAsWeapon;
	out << m_UsesOnOther;
	out << m_UsesOnSelf;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

bool AuPlayerStatsItems::Had(COMMODITYID cid) const
{
	const_iterator it = find(cid);
	if(it != end())
		return (*it).second.m_HadInventory;
	return false;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuPlayerStatsPlanet::AuPlayerStatsPlanet()
{
	m_BeamDownLand = 0;
	m_BeamDownSea = 0;
	m_BeamUpLand = 0;
	m_BeamUpSea = 0;
	m_DeepestSwim = 0;
	m_EntryEVA = 0;
	m_EntryMotorBike = 0;
	m_EntryRocket = 0;
	m_EntrySailBoat = 0;
	m_EntrySpaceFighterF1 = 0;
	m_EntrySpaceFighterF2 = 0;
	m_EntryStation = 0;
	m_EntrySpaceship = 0;
	m_EntryStarship = 0;
	m_EntrySubmarine = 0;
	m_EntrySUV = 0;
	m_EntryAPC = 0;
	m_EntryFarmMachinery = 0;
	m_EntryHelicopter = 0;
	m_EntrySpaceTransport = 0;
	m_ExitEVA = 0;
	m_ExitMotorBike = 0;
	m_ExitRocket = 0;
	m_ExitSailBoat = 0;
	m_ExitSpaceFighterF1 = 0;
	m_ExitSpaceFighterF2 = 0;
	m_ExitStation = 0;
	m_ExitSpaceship = 0;
	m_ExitStarship = 0;
	m_ExitSubmarine = 0;
	m_ExitSUV = 0;
	m_ExitAPC = 0;
	m_ExitFarmMachinery = 0;
	m_ExitHelicopter = 0;
	m_ExitSpaceTransport = 0;
	m_FirstFlags = 0;
	m_HighestFall = false;
	m_Treaded = false;
}

void AuPlayerStatsPlanet::operator<<(QDataStream& in)
{
	in >> m_BeamDownLand;
	in >> m_BeamDownSea;
	in >> m_BeamUpLand;
	in >> m_BeamUpSea;
	in >> m_DeepestSwim;
	in >> m_EntryEVA;
	in >> m_EntryMotorBike;
	in >> m_EntryRocket;
	in >> m_EntrySailBoat;
	in >> m_EntrySpaceFighterF1;
	in >> m_EntrySpaceFighterF2;
	in >> m_EntryStation;
	in >> m_EntrySpaceship;
	in >> m_EntryStarship;
	in >> m_EntrySubmarine;
	in >> m_EntrySUV;
	in >> m_ExitEVA;
	in >> m_ExitMotorBike;
	in >> m_ExitRocket;
	in >> m_ExitSailBoat;
	in >> m_ExitSpaceFighterF1;
	in >> m_ExitSpaceFighterF2;
	in >> m_ExitStation;
	in >> m_ExitSpaceship;
	in >> m_ExitStarship;
	in >> m_ExitSubmarine;
	in >> m_ExitSUV;
	in >> m_FirstFlags;
	in >> m_HighestFall;
	in >> m_Treaded;
	in >> m_EntryAPC;
	in >> m_EntryFarmMachinery;
	in >> m_EntryHelicopter;
	in >> m_EntrySpaceTransport;
	in >> m_ExitAPC;
	in >> m_ExitFarmMachinery;
	in >> m_ExitHelicopter;
	in >> m_ExitSpaceTransport;
}

void AuPlayerStatsPlanet::operator>>(QDataStream& out) const
{
	out << m_BeamDownLand;
	out << m_BeamDownSea;
	out << m_BeamUpLand;
	out << m_BeamUpSea;
	out << m_DeepestSwim;
	out << m_EntryEVA;
	out << m_EntryMotorBike;
	out << m_EntryRocket;
	out << m_EntrySailBoat;
	out << m_EntrySpaceFighterF1;
	out << m_EntrySpaceFighterF2;
	out << m_EntryStation;
	out << m_EntrySpaceship;
	out << m_EntryStarship;
	out << m_EntrySubmarine;
	out << m_EntrySUV;
	out << m_ExitEVA;
	out << m_ExitMotorBike;
	out << m_ExitRocket;
	out << m_ExitSailBoat;
	out << m_ExitSpaceFighterF1;
	out << m_ExitSpaceFighterF2;
	out << m_ExitStation;
	out << m_ExitSpaceship;
	out << m_ExitStarship;
	out << m_ExitSubmarine;
	out << m_ExitSUV;
	out << m_FirstFlags;
	out << m_HighestFall;
	out << m_Treaded;
	out << m_EntryAPC;
	out << m_EntryFarmMachinery;
	out << m_EntryHelicopter;
	out << m_EntrySpaceTransport;
	out << m_ExitAPC;
	out << m_ExitFarmMachinery;
	out << m_ExitHelicopter;
	out << m_ExitSpaceTransport;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//static
PLANET_STAT_TYPE AuPlayerStatsPlanets::StatType(const DaWorldGasGiant* planet)
{
	int bodyType = 0;		// gas giant

	int statType = planet->OrbitZone()*25;	// Calc enum offset to first entry of the 'orbit zone's range.
	statType += planet->AtmosphereType()*5;	// Offset to first entry of the 'atmosphere type's range.
	statType += bodyType;					// Offset to specific 'bodyType'.
	return (PLANET_STAT_TYPE)statType;
}

//static
PLANET_STAT_TYPE AuPlayerStatsPlanets::StatType(const DaWorldRingSection* planet)
{
	int bodyType = 4;		// ring world

	int statType = planet->OrbitZone()*25;	// Calc enum offset to first entry of the 'orbit zone's range.
	statType += planet->AtmosphereType()*5;	// Offset to first entry of the 'atmosphere type's range.
	statType += bodyType;					// Offset to specific 'bodyType'.
	return (PLANET_STAT_TYPE)statType;
}

//static
PLANET_STAT_TYPE AuPlayerStatsPlanets::StatType(const DaWorldGlobe* planet)
{
	int bodyType = planet->BodyType();
	int statType;
	if(bodyType == BODY_Planetoid)
		statType = PST_InfernoPlanetoid + planet->OrbitZone();	// Calc offset to first entry of the 'orbit zone's range.
	else if(bodyType == BODY_Ring)
		statType = PST_InfernoRing + planet->OrbitZone();			// Calc offset to first entry of the 'orbit zone's range.
	else
	{
		statType = planet->OrbitZone()*25;	// Calc enum offset to first entry of the 'orbit zone's range.
		statType += planet->AtmosphereType()*5;	// Offset to first entry of the 'atmosphere type's range.
		statType += bodyType;					// Offset to specific 'bodyType'.
	}
	return (PLANET_STAT_TYPE)statType;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuPlayerStatsStar::AuPlayerStatsStar()
{
	m_SunBurnEVA = 0;
	m_SunBurnMotorBike = 0;
	m_SunBurnRocket = 0;
	m_SunBurnSailBoat = 0;
	m_SunBurnSpaceFighterF1 = 0;
	m_SunBurnSpaceFighterF2 = 0;
	m_SunBurnStation = 0;
	m_SunBurnSpaceship = 0;
	m_SunBurnStarship = 0;
	m_SunBurnSubmarine = 0;
	m_SunBurnSUV = 0;
	m_SunBurnAPC = 0;
	m_SunBurnFarmMachinery = 0;
	m_SunBurnHelicopter = 0;
	m_SunBurnSpaceTransport = 0;
}

void AuPlayerStatsStar::operator<<(QDataStream& in)
{
	in >> m_SunBurnEVA;
	in >> m_SunBurnMotorBike;
	in >> m_SunBurnRocket;
	in >> m_SunBurnSailBoat;
	in >> m_SunBurnSpaceFighterF1;
	in >> m_SunBurnSpaceFighterF2;
	in >> m_SunBurnStation;
	in >> m_SunBurnSpaceship;
	in >> m_SunBurnStarship;
	in >> m_SunBurnSubmarine;
	in >> m_SunBurnSUV;
	in >> m_SunBurnAPC;
	in >> m_SunBurnFarmMachinery;
	in >> m_SunBurnHelicopter;
	in >> m_SunBurnSpaceTransport;
}

void AuPlayerStatsStar::operator>>(QDataStream& out) const
{
	out << m_SunBurnEVA;
	out << m_SunBurnMotorBike;
	out << m_SunBurnRocket;
	out << m_SunBurnSailBoat;
	out << m_SunBurnSpaceFighterF1;
	out << m_SunBurnSpaceFighterF2;
	out << m_SunBurnStation;
	out << m_SunBurnSpaceship;
	out << m_SunBurnStarship;
	out << m_SunBurnSubmarine;
	out << m_SunBurnSUV;
	out << m_SunBurnAPC;
	out << m_SunBurnFarmMachinery;
	out << m_SunBurnHelicopter;
	out << m_SunBurnSpaceTransport;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//static
STAR_STAT_TYPE AuPlayerStatsStars::StatType(const DaStar* star)
{
	if(star->StarType() == DaStar::TYPE_SupermassiveBlackHole)
		return SST_Supermassive_Black_Hole;
	else if(star->StarType() == DaStar::TYPE_StellarBlackHole)
		return SST_Stellar_Black_Hole;
	else if(star->StarType() == DaStar::TYPE_NeutronStar)
		return SST_Neutron_Star;
	int statType = star->StarType()*8;	// Calc enum offset to first entry of the 'star type's range.
	statType += star->Size();			// Offset to specific 'size'.
	return (STAR_STAT_TYPE)statType;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuPlayerStatsCraft::AuPlayerStatsCraft()
{
	m_Captures = 0;
	m_DeadHeads = 0;
	m_RepairsBought = 0;
	m_RepairsByHand = 0;
	m_SectorsTraversed = 0;
	m_UpgradesBought = 0;
	m_Wormholes = 0;
}

void AuPlayerStatsCraft::operator<<(QDataStream& in)
{
	in >> m_Captures;
	in >> m_DeadHeads;
	in >> m_RepairsBought;
	in >> m_RepairsByHand;
	in >> m_SectorsTraversed;
	in >> m_UpgradesBought;
	in >> m_Wormholes;
}

void AuPlayerStatsCraft::operator>>(QDataStream& out) const
{
	out << m_Captures;
	out << m_DeadHeads;
	out << m_RepairsBought;
	out << m_RepairsByHand;
	out << m_SectorsTraversed;
	out << m_UpgradesBought;
	out << m_Wormholes;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//static
CRAFT_STAT_TYPE AuPlayerStatsCrafts::StatType(const DaAtom* atom)
{
	const DaVessel* ship = dynamic_cast<const DaVessel*>(atom);
	if(ship)
		return StatType(ship);
	else
	{
		const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(atom);
		if(vehicle)
			return StatType(vehicle);
	}
	return CST_Invalid;
}

//static
CRAFT_STAT_TYPE AuPlayerStatsCrafts::StatType(const DaVessel* ship)
{
	switch(ship->Structure())
	{
	case SC_Starship:
		return CST_Starship;
	case SC_Spaceship:
		return CST_Spaceship;
	case SC_Station:
		return CST_Station;
	case SC_Hull:
	case SC_Count:
	case SC_Deprecated:
		break;
	}
	return CST_Invalid;
}

//static
CRAFT_STAT_TYPE AuPlayerStatsCrafts::StatType(const DaVehicle* vehicle)
{
	return StatType(vehicle->CommodityId());
}

//static
CRAFT_STAT_TYPE AuPlayerStatsCrafts::StatType(COMMODITYID vehicleCID)
{
	switch(vehicleCID)
	{
	case CID_DirtBike:
		return CST_MotorBike;
	case CID_SailBoat:
		return CST_SailBoat;
	case CID_SpaceFighter:
		return CST_SpaceFighterF1;
	case CID_SpaceTransport:
		return CST_SpaceTransport;
	case CID_AdvSpaceFighter:
		return CST_SpaceFighterF2;
	case CID_SpaceRocket:
		return CST_Rocket;
	case CID_Helicopter:
		return CST_Helicopter;
	case CID_Submarine:
		return CST_Submarine;
	case CID_SUV:
		return CST_SUV;
	case CID_APC:
		return CST_APC;
	case CID_FarmMachinery:
		return CST_FarmMachinery;
	default:
		break;
	}
	return CST_Invalid;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuPlayerStatsStation::AuPlayerStatsStation()
{
	m_HighestQ = 0;
	m_Minutes = 0;
}

void AuPlayerStatsStation::operator<<(QDataStream& in)
{
	in >> m_HighestQ;
	in >> m_Minutes;
}

void AuPlayerStatsStation::operator>>(QDataStream& out) const
{
	out << m_HighestQ;
	out << m_Minutes;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

int AuPlayerStatsStations::Minutes(COMMODITYID vehicleCID) const
{
	STATION_STAT_TYPE sst = StatType(vehicleCID);
	if(sst != EST_Invalid)
	{
		const_iterator it = find(sst);
		if(it != end())
			return (*it).second.m_Minutes;
	}
	return 0;
}

int AuPlayerStatsStations::Minutes(AuStation::TYPE type, bool isShipStation) const
{
	STATION_STAT_TYPE sst = StatType(type, isShipStation);
	if(sst != EST_Invalid)
	{
		const_iterator it = find(sst);
		if(it != end())
			return (*it).second.m_Minutes;
	}
	return 0;
}

//static
STATION_STAT_TYPE AuPlayerStatsStations::StatType(AuStation::TYPE type, bool isShipStation)
{
	switch(type)
	{
	case AuStation::Console_Navigation:
	case AuStation::Integrated_Navigation:
		if(isShipStation)
			return EST_ShipConsoleNavigation;
		else
			return EST_BuildingConsoleNavigation;
	case AuStation::Console_Medic:
	case AuStation::Integrated_Medic:
		return EST_ShipConsoleSurgery;
	case AuStation::Console_Engine:
	case AuStation::Integrated_Engine:
		return EST_ShipConsoleEngine;
	case AuStation::Console_FireControl:
	case AuStation::Integrated_FireControl:
		if(isShipStation)
			return EST_ShipConsoleFireControl;
		else
			return EST_BuildingConsoleFireControl;
	case AuStation::Console_Helm:
	case AuStation::Integrated_Helm:
		return EST_ShipConsolePilot;
	case AuStation::Console_Power:
	case AuStation::Integrated_Power:
		return EST_ShipConsolePower;
	case AuStation::Console_Sensor:
	case AuStation::Integrated_Sensor:
		if(isShipStation)
			return EST_ShipConsoleSensor;
		else
			return EST_BuildingConsoleSensor;
	case AuStation::Console_Shield:
	case AuStation::Integrated_Shield:
		if(isShipStation)
			return EST_ShipConsoleShield;
		else
			return EST_BuildingConsoleShield;
	case AuStation::Console_Transporter:
	case AuStation::Integrated_Transporter:
		if(isShipStation)
			return EST_ShipConsoleTransporter;
		else
			return EST_BuildingConsoleTransporter;
	case AuStation::Turret_Integrated:
	case AuStation::Turret_Bottom:
	case AuStation::Turret_Top:
	case AuStation::Turret_0:
	case AuStation::Turret_45:
	case AuStation::Turret_90:
	case AuStation::Turret_135:
	case AuStation::Turret_180:
	case AuStation::Turret_225:
	case AuStation::Turret_270:
	case AuStation::Turret_315:
		return EST_ShipTurret;
	case AuStation::Console_Command:
	case AuStation::Integrated_Command:
		return EST_ShipConsoleCommand;
	case AuStation::Table_Game:
	case AuStation::Table_Game_0:
	case AuStation::Table_Game_45:
		return EST_ShipConsoleGameTable;
	case AuStation::Console_Patient:
	case AuStation::Integrated_Designer:
	case AuStation::Integrated_Patient:
	case AuStation::Berth_Captain:
	case AuStation::Berth_Crew:
	case AuStation::Berth_Officer:
	case AuStation::Berth_Passenger:
	case AuStation::Berth_Troop:
	case AuStation::Berth_1_0:
	case AuStation::Berth_1_45:
	case AuStation::Berth_1_90:
	case AuStation::Berth_1_135:
	case AuStation::Berth_1_180:
	case AuStation::Berth_1_225:
	case AuStation::Berth_1_270:
	case AuStation::Berth_1_315:
	case AuStation::Berth_2_0:
	case AuStation::Berth_2_45:
	case AuStation::Berth_2_90:
	case AuStation::Berth_2_135:
	case AuStation::Berth_2_180:
	case AuStation::Berth_2_225:
	case AuStation::Berth_2_270:
	case AuStation::Berth_2_315:
	case AuStation::Citizen:
	case AuStation::Livestock:
	case AuStation::Troop:
	case AuStation::TYPE_Count:
		break;
	}
	return EST_Invalid;
}

//static
STATION_STAT_TYPE AuPlayerStatsStations::StatType(const DaVehicle* vehicle)
{
	return StatType(vehicle->CommodityId());
}

//static
STATION_STAT_TYPE AuPlayerStatsStations::StatType(COMMODITYID vehicleCID)
{
	switch(vehicleCID)
	{
	case CID_DirtBike:
		return EST_VehicleMotorBike;
	case CID_SailBoat:
		return EST_VehicleSailBoat;
	case CID_SpaceFighter:
		return EST_VehicleSpaceFighterF1;
	case CID_SpaceTransport:
		return EST_VehicleSpaceTransport;
	case CID_AdvSpaceFighter:
		return EST_VehicleSpaceFighterF2;
	case CID_SpaceRocket:
		return EST_VehicleRocket;
	case CID_Helicopter:
		return EST_VehicleHelicopter;
	case CID_Submarine:
		return EST_VehicleSubmarine;
	case CID_SUV:
		return EST_VehicleSUV;
	case CID_APC:
		return EST_VehicleAPC;
	case CID_FarmMachinery:
		return EST_VehicleFarmMachinery;
	default:
		return EST_Invalid;
	}
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuPlayerStatsAnimal::AuPlayerStatsAnimal()
{
	m_DNA = 0;
	m_Named = 0;
	m_KilledBy = 0;
	m_Kills = 0;
	m_Studied = 0;
}

void AuPlayerStatsAnimal::operator<<(QDataStream& in)
{
	in >> m_DNA;
	in >> m_Named;
	in >> m_KilledBy;
	in >> m_Kills;
	in >> m_Studied;
}

void AuPlayerStatsAnimal::operator>>(QDataStream& out) const
{
	out << m_DNA;
	out << m_Named;
	out << m_KilledBy;
	out << m_Kills;
	out << m_Studied;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuPlayerStatsCreature::AuPlayerStatsCreature()
{
	m_DNA = 0;
	m_Named = 0;
	m_KilledBy = 0;
	m_Kills = 0;
	m_Studied = false;
}

void AuPlayerStatsCreature::operator<<(QDataStream& in)
{
	in >> m_DNA;
	in >> m_Named;
	in >> m_KilledBy;
	in >> m_Kills;
	in >> m_Studied;
}

void AuPlayerStatsCreature::operator>>(QDataStream& out) const
{
	out << m_DNA;
	out << m_Named;
	out << m_KilledBy;
	out << m_Kills;
	out << m_Studied;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuPlayerStatsCreatures::operator<<(QDataStream& in)
{
	quint16 qty;
	in >> qty;
	bool species = qty & 0x8000;
	qty &= 0x7fff;

	if(species)
	{
		AuSpecies key;
		AuPlayerStatsCreature val;
		for(; qty; qty--)
		{
			in >> key;
			val << in;
			insert(pair<AuSpecies, AuPlayerStatsCreature>(key, val));
		}
	}
	else
	{
		QString key;
		AuPlayerStatsCreature val;
		for(; qty; qty--)
		{
			in >> key;
			val << in;

			DhDNA dna;
			if(dna.FromString(key))
			{
				AuSpecies species(dna.Species());
				insert(pair<AuSpecies, AuPlayerStatsCreature>(species, val));
			}
		}
	}
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuPlayerStatsSystem::AuPlayerStatsSystem()
{
	m_Repairs = 0;
	m_Upgrades = 0;
}

void AuPlayerStatsSystem::operator<<(QDataStream& in)
{
	in >> m_Repairs;
	in >> m_Upgrades;
}

void AuPlayerStatsSystem::operator>>(QDataStream& out) const
{
	out << m_Repairs;
	out << m_Upgrades;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuPlayerStatsKill::AuPlayerStatsKill()
{
	m_KilledBy = 0;
	m_Kills = 0;
}

void AuPlayerStatsKill::operator<<(QDataStream& in)
{
	in >> m_KilledBy;
	in >> m_Kills;
}

void AuPlayerStatsKill::operator>>(QDataStream& out) const
{
	out << m_KilledBy;
	out << m_Kills;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//static
KILL_STAT_TYPE AuPlayerStatsKills::StatType(const DaUnit* unitKilled, POLITICAL_STANCE stanceToUnit)
{
	// Calc enum offset to first entry of the 'atomType's range.
	switch(unitKilled->AtomType())
	{
	default:
		break;
	case DaAtom::ATOM_Npc:
		{
			const DaNpc* npc = dynamic_cast<const DaNpc*>(unitKilled);
			if(npc)
			{
				if(npc->IsNpcCrew())
					return (KILL_STAT_TYPE)(KST_CrewFriend + stanceToUnit);
				else if(npc->IsNpcOfficer())
					return (KILL_STAT_TYPE)(KST_OfficerFriend + stanceToUnit);
				else if(npc->IsNpcPassenger())
					return (KILL_STAT_TYPE)(KST_PassengerFriend + stanceToUnit);
				else if(npc->IsNpcTroop())
					return (KILL_STAT_TYPE)(KST_TroopFriend + stanceToUnit);
			}
		}
		break;
	case DaAtom::ATOM_Avatar:
		return (KILL_STAT_TYPE)(KST_AvatarFriend + stanceToUnit);
	case DaAtom::ATOM_Vehicle:
		switch(((DaVehicle*)unitKilled)->CommodityId())
		{
		case CID_DirtBike:
			return (KILL_STAT_TYPE)(KST_MotorBikeFriend + stanceToUnit);
		case CID_SailBoat:
			return (KILL_STAT_TYPE)(KST_SailBoatFriend + stanceToUnit);
		case CID_SpaceFighter:
			return (KILL_STAT_TYPE)(KST_SpaceFighterF1Friend + stanceToUnit);
		case CID_SpaceTransport:
			return (KILL_STAT_TYPE)(KST_SpaceTransportFriend + stanceToUnit);
		case CID_AdvSpaceFighter:
			return (KILL_STAT_TYPE)(KST_SpaceFighterF2Friend + stanceToUnit);
		case CID_SpaceRocket:
			return (KILL_STAT_TYPE)(KST_RocketFriend + stanceToUnit);
		case CID_Helicopter:
			return (KILL_STAT_TYPE)(KST_HelicopterFriend + stanceToUnit);
		case CID_Submarine:
			return (KILL_STAT_TYPE)(KST_SubmarineFriend + stanceToUnit);
		case CID_SUV:
			return (KILL_STAT_TYPE)(KST_SUVFriend + stanceToUnit);
		case CID_APC:
			return (KILL_STAT_TYPE)(KST_APCFriend + stanceToUnit);
		case CID_FarmMachinery:
			return (KILL_STAT_TYPE)(KST_FarmMachineryFriend + stanceToUnit);
		default:
			break;
		}
		break;
	}
	return KST_Invalid;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuPlayerStatsBuilding::AuPlayerStatsBuilding()
{
	m_Built = 0;
	m_ConstructionLabor = 0;
	m_ManufactureGo = 0;
}

void AuPlayerStatsBuilding::operator<<(QDataStream& in)
{
	in >> m_Built;
	in >> m_ConstructionLabor;
	in >> m_ManufactureGo;
}

void AuPlayerStatsBuilding::operator>>(QDataStream& out) const
{
	out << m_Built;
	out << m_ConstructionLabor;
	out << m_ManufactureGo;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuPlayerStats::AuPlayerStats()
{
	m_BoardedCrew = 0;
	m_BoardedCryo = 0;
	m_BoardedOfficers = 0;
	m_BoardedPassengers = 0;
	m_BoardedTroops = 0;
	m_BountyCollected = 0;
	m_BountyCollections = 0;
	m_BountyGranted = 0;
	m_BountyGrants = 0;
	//m_BuildingDesignsBought = 0;
	//m_BuildingDesignsBurned = 0;
	//m_BuildingDesignsMade = 0;
	//m_BuildingDesignsPublished = 0;
	//m_BuildingDesignsSold = 0;
	//m_BuildingsDestroyedAlly = 0;
	//m_BuildingsDestroyedEmpire = 0;
	//m_BuildingsDestroyedEnemy = 0;
	//m_BuildingsDestroyedFriend = 0;
	//m_BuildingsDestroyedNeutral = 0;
	//m_BuildingsDestroyedVassal = 0;
	//m_BuildingsHabitable = 0;
	//m_BuildingsHarsh = 0;
	m_BurnedStation = 0;
	m_BurnedSpaceship = 0;
	m_BurnedStarship = 0;
	m_ChildrenBorn = 0;
	m_ChildrenCarried = 0;
	m_ChildrenFathered = 0;
	m_ClaimedDifferentDNA = 0;
	m_ClaimedSameDNA = 0;
	m_ClaimedUninhabitable = 0;
	//m_CreaturesTame = 0;
	//m_CreaturesWild = 0;
	//m_Deaths = 0;
	m_DebarkedCryo = 0;
	m_DebarkedPassengers = 0;
	m_EVABeamFrom = 0;
	m_EVABeamTo = 0;
	m_EVADeadHead = 0;
	m_EVAMinutes = 0;
	m_EVASectorTraversed = 0;
	m_FlagsCapturedBase = 0;
	m_FlagsCapturedCapitalEmpire = 0;
	m_FlagsCapturedCapitalGalaxy = 0;
	m_FlagsCapturedCapitalSector = 0;
	m_FlagsCapturedCity = 0;
	//m_Items = 0;
	//m_Kills = 0;
	m_NamedSolarSystems = 0;
	m_NamedSpecies = 0;
	m_NamedSectors = 0;
	m_OnLineMinutes = 0;
	m_OrdersMission = 0;
	m_OrdersSquad = 0;
	//m_Planets = 0;
	m_PublishedStation = 0;
	m_PublishedSpaceship = 0;
	m_PublishedStarship = 0;
	m_ShipmentsDelivered = 0;
	m_ShipmentsJettisonedByMe = 0;
	m_ShipmentsJettisonedByOther = 0;
	m_ShipmentsLoaded = 0;
	m_ShipmentsMineDelivered = 0;
	m_ShipmentsMineJettisoned = 0;
	m_ShipmentsMineOpened = 0;
	m_ShipmentsOpenedByMe = 0;
	m_ShipmentsOpenedByOther = 0;
	//m_ShipDesignsBought = 0;
	//m_ShipDesignsMade = 0;
	//m_ShipDesignsSold = 0;
	//m_ShipsAndVehicles = 0;
	m_SpawnRecalls = 0;
	//m_Stars = 0;
	//m_Stations = 0;
	m_StoriesCompleted = 0;
	//m_StoriesFinished = 0;
	m_StoriesStarted = 0;
	//m_Systems = 0;
	m_WormholesExplored = 0;
	m_WormholesNamed = 0;
}

AuPlayerStats::~AuPlayerStats()
{
}

AuPlayerStats::AuPlayerStats(const AuPlayerStats& c)
{
	c.m_StatsLock.lock();
	m_BoardedCrew = c.m_BoardedCrew;
	m_BoardedCryo = c.m_BoardedCryo;
	m_BoardedOfficers = c.m_BoardedOfficers;
	m_BoardedPassengers = c.m_BoardedPassengers;
	m_BoardedTroops = c.m_BoardedTroops;
	m_BountyCollected = c.m_BountyCollected;
	m_BountyCollections = c.m_BountyCollections;
	m_BountyGranted = c.m_BountyGranted;
	m_BountyGrants = c.m_BountyGrants;
	m_BuildingDesignsBought = c.m_BuildingDesignsBought;
	m_BuildingDesignsBurned = c.m_BuildingDesignsBurned;
	m_BuildingDesignsMade = c.m_BuildingDesignsMade;
	m_BuildingDesignsPublished = c.m_BuildingDesignsPublished;
	m_BuildingDesignsSold = c.m_BuildingDesignsSold;
	m_BuildingsDestroyedAlly = c.m_BuildingsDestroyedAlly;
	m_BuildingsDestroyedEmpire = c.m_BuildingsDestroyedEmpire;
	m_BuildingsDestroyedEnemy = c.m_BuildingsDestroyedEnemy;
	m_BuildingsDestroyedFriend = c.m_BuildingsDestroyedFriend;
	m_BuildingsDestroyedNeutral = c.m_BuildingsDestroyedNeutral;
	m_BuildingsDestroyedVassal = c.m_BuildingsDestroyedVassal;
	m_BuildingsHabitable = c.m_BuildingsHabitable;
	m_BuildingsHarsh = c.m_BuildingsHarsh;
	m_BurnedStation = c.m_BurnedStation;
	m_BurnedSpaceship = c.m_BurnedSpaceship;
	m_BurnedStarship = c.m_BurnedStarship;
	m_ChildrenBorn = c.m_ChildrenBorn;
	m_ChildrenCarried = c.m_ChildrenCarried;
	m_ChildrenFathered = c.m_ChildrenFathered;
	m_ClaimedDifferentDNA = c.m_ClaimedDifferentDNA;
	m_ClaimedSameDNA = c.m_ClaimedUninhabitable;
	m_ClaimedSameDNA = c.m_ClaimedUninhabitable;
	m_CreaturesTame = c.m_CreaturesTame;
	m_CreaturesWild = c.m_CreaturesWild;
	m_Deaths = c.m_Deaths;
	m_DebarkedCryo = c.m_DebarkedCryo;
	m_DebarkedPassengers = c.m_DebarkedPassengers;
	m_EVABeamFrom = c.m_EVABeamFrom;
	m_EVABeamTo = c.m_EVABeamTo;
	m_EVADeadHead = c.m_EVADeadHead;
	m_EVAMinutes = c.m_EVAMinutes;
	m_EVASectorTraversed = c.m_EVASectorTraversed;
	m_FlagsCapturedBase = c.m_FlagsCapturedBase;
	m_FlagsCapturedCapitalEmpire = c.m_FlagsCapturedCapitalEmpire;
	m_FlagsCapturedCapitalGalaxy = c.m_FlagsCapturedCapitalGalaxy;
	m_FlagsCapturedCapitalSector = c.m_FlagsCapturedCapitalSector;
	m_FlagsCapturedCity = c.m_FlagsCapturedCity;
	m_Items = c.m_Items;
	m_Kills = c.m_Kills;
	m_NamedSectors = c.m_NamedSectors;
	m_NamedSolarSystems = c.m_NamedSolarSystems;
	m_NamedSpecies = c.m_NamedSpecies;
	m_OnLineMinutes = c.m_OnLineMinutes;
	m_OrdersMission = c.m_OrdersMission;
	m_OrdersSquad = c.m_OrdersSquad;
	m_Planets = c.m_Planets;
	m_PublishedStation = c.m_PublishedStation;
	m_PublishedSpaceship = c.m_PublishedSpaceship;
	m_PublishedStarship = c.m_PublishedStarship;
	m_ShipDesignsBought = c.m_ShipDesignsBought;
	m_ShipDesignsMade = c.m_ShipDesignsMade;
	m_ShipDesignsSold = c.m_ShipDesignsSold;
	m_ShipsAndVehicles = c.m_ShipsAndVehicles;
	m_ShipmentsDelivered = c.m_ShipmentsDelivered;
	m_ShipmentsJettisonedByMe = c.m_ShipmentsJettisonedByMe;
	m_ShipmentsJettisonedByOther = c.m_ShipmentsJettisonedByOther;
	m_ShipmentsLoaded = c.m_ShipmentsLoaded;
	m_ShipmentsMineDelivered = c.m_ShipmentsMineDelivered;
	m_ShipmentsMineJettisoned = c.m_ShipmentsMineJettisoned;
	m_ShipmentsMineOpened = c.m_ShipmentsMineOpened;
	m_ShipmentsOpenedByMe = c.m_ShipmentsOpenedByMe;
	m_ShipmentsOpenedByOther = c.m_ShipmentsOpenedByOther;
	m_SpawnRecalls = c.m_SpawnRecalls;
	m_Stars = c.m_Stars;
	m_Stations = c.m_Stations;
	m_StoriesCompleted = c.m_StoriesCompleted;
	m_StoriesFinished = c.m_StoriesFinished;
	m_StoriesStarted = c.m_StoriesStarted;
	m_Systems = c.m_Systems;
	m_WormholesExplored = c.m_WormholesExplored;
	m_WormholesNamed = c.m_WormholesNamed;
	c.m_StatsLock.unlock();
}

const AuPlayerStats& AuPlayerStats::operator=(const AuPlayerStats& c)
{
	c.m_StatsLock.lock();
	m_BoardedCrew = c.m_BoardedCrew;
	m_BoardedCryo = c.m_BoardedCryo;
	m_BoardedOfficers = c.m_BoardedOfficers;
	m_BoardedPassengers = c.m_BoardedPassengers;
	m_BoardedTroops = c.m_BoardedTroops;
	m_BountyCollected = c.m_BountyCollected;
	m_BountyCollections = c.m_BountyCollections;
	m_BountyGranted = c.m_BountyGranted;
	m_BountyGrants = c.m_BountyGrants;
	m_BuildingDesignsBought = c.m_BuildingDesignsBought;
	m_BuildingDesignsBurned = c.m_BuildingDesignsBurned;
	m_BuildingDesignsMade = c.m_BuildingDesignsMade;
	m_BuildingDesignsPublished = c.m_BuildingDesignsPublished;
	m_BuildingDesignsSold = c.m_BuildingDesignsSold;
	m_BuildingsDestroyedAlly = c.m_BuildingsDestroyedAlly;
	m_BuildingsDestroyedEmpire = c.m_BuildingsDestroyedEmpire;
	m_BuildingsDestroyedEnemy = c.m_BuildingsDestroyedEnemy;
	m_BuildingsDestroyedFriend = c.m_BuildingsDestroyedFriend;
	m_BuildingsDestroyedNeutral = c.m_BuildingsDestroyedNeutral;
	m_BuildingsDestroyedVassal = c.m_BuildingsDestroyedVassal;
	m_BuildingsHabitable = c.m_BuildingsHabitable;
	m_BuildingsHarsh = c.m_BuildingsHarsh;
	m_BurnedStation = c.m_BurnedStation;
	m_BurnedSpaceship = c.m_BurnedSpaceship;
	m_BurnedStarship = c.m_BurnedStarship;
	m_ChildrenBorn = c.m_ChildrenBorn;
	m_ChildrenCarried = c.m_ChildrenCarried;
	m_ChildrenFathered = c.m_ChildrenFathered;
	m_ClaimedDifferentDNA = c.m_ClaimedDifferentDNA;
	m_ClaimedSameDNA = c.m_ClaimedUninhabitable;
	m_ClaimedSameDNA = c.m_ClaimedUninhabitable;
	m_CreaturesTame = c.m_CreaturesTame;
	m_CreaturesWild = c.m_CreaturesWild;
	m_Deaths = c.m_Deaths;
	m_DebarkedCryo = c.m_DebarkedCryo;
	m_DebarkedPassengers = c.m_DebarkedPassengers;
	m_EVABeamFrom = c.m_EVABeamFrom;
	m_EVABeamTo = c.m_EVABeamTo;
	m_EVADeadHead = c.m_EVADeadHead;
	m_EVAMinutes = c.m_EVAMinutes;
	m_EVASectorTraversed = c.m_EVASectorTraversed;
	m_FlagsCapturedBase = c.m_FlagsCapturedBase;
	m_FlagsCapturedCapitalEmpire = c.m_FlagsCapturedCapitalEmpire;
	m_FlagsCapturedCapitalGalaxy = c.m_FlagsCapturedCapitalGalaxy;
	m_FlagsCapturedCapitalSector = c.m_FlagsCapturedCapitalSector;
	m_FlagsCapturedCity = c.m_FlagsCapturedCity;
	m_Items = c.m_Items;
	m_Kills = c.m_Kills;
	m_NamedSectors = c.m_NamedSectors;
	m_NamedSolarSystems = c.m_NamedSolarSystems;
	m_NamedSpecies = c.m_NamedSpecies;
	m_OnLineMinutes = c.m_OnLineMinutes;
	m_OrdersMission = c.m_OrdersMission;
	m_OrdersSquad = c.m_OrdersSquad;
	m_Planets = c.m_Planets;
	m_PublishedStation = c.m_PublishedStation;
	m_PublishedSpaceship = c.m_PublishedSpaceship;
	m_PublishedStarship = c.m_PublishedStarship;
	m_ShipDesignsBought = c.m_ShipDesignsBought;
	m_ShipDesignsMade = c.m_ShipDesignsMade;
	m_ShipDesignsSold = c.m_ShipDesignsSold;
	m_ShipmentsDelivered = c.m_ShipmentsDelivered;
	m_ShipmentsJettisonedByMe = c.m_ShipmentsJettisonedByMe;
	m_ShipmentsJettisonedByOther = c.m_ShipmentsJettisonedByOther;
	m_ShipmentsLoaded = c.m_ShipmentsLoaded;
	m_ShipmentsMineDelivered = c.m_ShipmentsMineDelivered;
	m_ShipmentsMineJettisoned = c.m_ShipmentsMineJettisoned;
	m_ShipmentsMineOpened = c.m_ShipmentsMineOpened;
	m_ShipmentsOpenedByMe = c.m_ShipmentsOpenedByMe;
	m_ShipmentsOpenedByOther = c.m_ShipmentsOpenedByOther;
	m_ShipsAndVehicles = c.m_ShipsAndVehicles;
	m_SpawnRecalls = c.m_SpawnRecalls;
	m_Stars = c.m_Stars;
	m_Stations = c.m_Stations;
	m_StoriesCompleted = c.m_StoriesCompleted;
	m_StoriesFinished = c.m_StoriesFinished;
	m_StoriesStarted = c.m_StoriesStarted;
	m_Systems = c.m_Systems;
	m_WormholesExplored = c.m_WormholesExplored;
	m_WormholesNamed = c.m_WormholesNamed;
	c.m_StatsLock.unlock();

	return *this;
}

void AuPlayerStats::operator>>(QDataStream& out) const
{
	m_StatsLock.lock();
	out << m_BoardedCrew;
	out << m_BoardedOfficers;
	out << m_BoardedPassengers;
	out << m_BoardedTroops;
	m_BuildingsDestroyedFriend >> out;
	m_BuildingsDestroyedNeutral >> out;
	m_BuildingsDestroyedEnemy >> out;
	m_BuildingsDestroyedEmpire >> out;
	m_BuildingsDestroyedAlly >> out;
	m_BuildingsDestroyedVassal >> out;
	m_BuildingsHabitable >> out;
	m_BuildingsHarsh >> out;
	out << m_BurnedStation;
	out << m_BurnedSpaceship;
	out << m_BurnedStarship;
	m_Deaths >> out;
	out << m_DebarkedPassengers;
	m_ShipDesignsBought >> out;
	m_ShipDesignsMade >> out;
	m_ShipDesignsSold >> out;
	out << m_EVABeamFrom;
	out << m_EVABeamTo;
	out << m_EVADeadHead;
	out << m_EVAMinutes;
	out << m_EVASectorTraversed;
	out << m_FlagsCapturedBase;
	out << m_FlagsCapturedCapitalEmpire;
	out << m_FlagsCapturedCity;
	m_Items >> out;
	m_Kills >> out;
	out << m_NamedSolarSystems;
	out << m_NamedSpecies;
	out << m_NamedSectors;
	out << m_OrdersMission;
	out << m_OrdersSquad;
	m_Planets >> out;
	out << m_PublishedStation;
	out << m_PublishedSpaceship;
	out << m_PublishedStarship;
	m_ShipsAndVehicles >> out;
	out << m_SpawnRecalls;
	m_Stars >> out;
	m_Stations >> out;
	out << m_StoriesCompleted;
	out << m_StoriesStarted;
	m_Systems >> out;
	out << m_WormholesExplored;
	out << m_WormholesNamed;
	out << m_OnLineMinutes;
	out << m_FlagsCapturedCapitalSector;
	m_BuildingDesignsBought >> out;
	m_BuildingDesignsBurned >> out;
	m_BuildingDesignsMade >> out;
	m_BuildingDesignsPublished >> out;
	m_BuildingDesignsSold >> out;
	m_CreaturesTame >> out;
	m_CreaturesWild >> out;
	out << m_ChildrenBorn;
	out << m_ChildrenCarried;
	out << m_ChildrenFathered;

	quint8 version = 6;
	out << version;

	// Added in version 1.
	out << (quint16)m_StoriesFinished.size();
	for(QSet<quint32>::const_iterator sfi = m_StoriesFinished.cbegin(); sfi != m_StoriesFinished.cend(); sfi++)
		out << *sfi;

	// Added in version 2.
	out << m_BoardedCryo;
	out << m_DebarkedCryo;

	// Added in version 3.
	out << m_FlagsCapturedCapitalGalaxy;

	// Added in version 4.
	out << m_BountyCollected;
	out << m_BountyCollections;
	out << m_BountyGranted;
	out << m_BountyGrants;

	// Added in version 5.
	out << m_ClaimedDifferentDNA;
	out << m_ClaimedSameDNA;
	out << m_ClaimedSameDNA;

	// Added in version 6.
	out << m_ShipmentsDelivered;
	out << m_ShipmentsJettisonedByMe;
	out << m_ShipmentsJettisonedByOther;
	out << m_ShipmentsLoaded;
	out << m_ShipmentsMineDelivered;
	out << m_ShipmentsMineJettisoned;
	out << m_ShipmentsMineOpened;
	out << m_ShipmentsOpenedByMe;
	out << m_ShipmentsOpenedByOther;

	m_StatsLock.unlock();
}

void AuPlayerStats::operator<<(QDataStream& in)
{
	in >> m_BoardedCrew;
	in >> m_BoardedOfficers;
	in >> m_BoardedPassengers;
	in >> m_BoardedTroops;
	m_BuildingsDestroyedFriend << in;
	m_BuildingsDestroyedNeutral << in;
	m_BuildingsDestroyedEnemy << in;
	m_BuildingsDestroyedEmpire << in;
	m_BuildingsDestroyedAlly << in;
	m_BuildingsDestroyedVassal << in;
	m_BuildingsHabitable << in;
	m_BuildingsHarsh << in;
	in >> m_BurnedStation;
	in >> m_BurnedSpaceship;
	in >> m_BurnedStarship;
	m_Deaths << in;
	in >> m_DebarkedPassengers;
	m_ShipDesignsBought << in;
	m_ShipDesignsMade << in;
	m_ShipDesignsSold << in;
	in >> m_EVABeamFrom;
	in >> m_EVABeamTo;
	in >> m_EVADeadHead;
	in >> m_EVAMinutes;
	in >> m_EVASectorTraversed;
	in >> m_FlagsCapturedBase;
	in >> m_FlagsCapturedCapitalEmpire;
	in >> m_FlagsCapturedCity;
	m_Items << in;
	m_Kills << in;
	in >> m_NamedSolarSystems;
	in >> m_NamedSpecies;
	in >> m_NamedSectors;
	in >> m_OrdersMission;
	in >> m_OrdersSquad;
	m_Planets << in;
	in >> m_PublishedStation;
	in >> m_PublishedSpaceship;
	in >> m_PublishedStarship;
	m_ShipsAndVehicles << in;
	in >> m_SpawnRecalls;
	m_Stars << in;
	m_Stations << in;
	in >> m_StoriesCompleted;
	in >> m_StoriesStarted;
	m_Systems << in;
	in >> m_WormholesExplored;
	in >> m_WormholesNamed;
	in >> m_OnLineMinutes;
	in >> m_FlagsCapturedCapitalSector;
	m_BuildingDesignsBought << in;
	m_BuildingDesignsBurned << in;
	m_BuildingDesignsMade << in;
	m_BuildingDesignsPublished << in;
	m_BuildingDesignsSold << in;
	m_CreaturesTame << in;
	m_CreaturesWild << in;
	in >> m_ChildrenBorn;
	in >> m_ChildrenCarried;
	in >> m_ChildrenFathered;

	quint8 version;
	in >> version;

	if(version >= 1)
	{
		m_StoriesFinished.clear();
		quint16 sc;
		in >> sc;
		while(sc--)
		{
			quint32 scriptId;
			in >> scriptId;
			m_StoriesFinished.insert(scriptId);
		}
	}

	if(version >= 2)
	{
		in >> m_BoardedCryo;
		in >> m_DebarkedCryo;
	}

	if(version >= 3)
		in >> m_FlagsCapturedCapitalGalaxy;

	if(version >= 4)
	{
		in >> m_BountyCollected;
		in >> m_BountyCollections;
		in >> m_BountyGranted;
		in >> m_BountyGrants;
	}

	if(version >= 5)
	{
		in >> m_ClaimedDifferentDNA;
		in >> m_ClaimedSameDNA;
		in >> m_ClaimedSameDNA;
	}

	if(version >= 6)
	{
		in >> m_ShipmentsDelivered;
		in >> m_ShipmentsJettisonedByMe;
		in >> m_ShipmentsJettisonedByOther;
		in >> m_ShipmentsLoaded;
		in >> m_ShipmentsMineDelivered;
		in >> m_ShipmentsMineJettisoned;
		in >> m_ShipmentsMineOpened;
		in >> m_ShipmentsOpenedByMe;
		in >> m_ShipmentsOpenedByOther;
	}
}

void AuPlayerStats::ItemBoughtAsCargo(const AuItem& theItem)
{
	m_StatsLock.lock();
	AuPlayerStatsItems::iterator it = m_Items.find(theItem.CommodityId());
	if(it != m_Items.end())
	{
		(*it).second.m_BoughtCargo += theItem.Count();
		if(theItem.Quality() > (*it).second.m_HighestQ)
			(*it).second.m_HighestQ = theItem.Quality();
	}
	else
	{
		AuPlayerStatsItem item;
		item.m_BoughtCargo = theItem.Count();
		item.m_HighestQ = theItem.Quality();
		m_Items.insert(pair<COMMODITYID, AuPlayerStatsItem>(theItem.CommodityId(), item));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::ItemBoughtAsInventory(const AuItem& theItem)
{
	m_StatsLock.lock();
	AuPlayerStatsItems::iterator it = m_Items.find(theItem.CommodityId());
	if(it != m_Items.end())
	{
		(*it).second.m_BoughtInventory += theItem.Count();
		if(theItem.Quality() > (*it).second.m_HighestQ)
			(*it).second.m_HighestQ = theItem.Quality();
	}
	else
	{
		AuPlayerStatsItem item;
		item.m_BoughtInventory = theItem.Count();
		item.m_HighestQ = theItem.Quality();
		m_Items.insert(pair<COMMODITYID, AuPlayerStatsItem>(theItem.CommodityId(), item));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::ItemForaged(const AuItem& theItem)
{
	m_StatsLock.lock();
	AuPlayerStatsItems::iterator it = m_Items.find(theItem.CommodityId());
	if(it != m_Items.end())
	{
		(*it).second.m_Foraged++;

		if(theItem.Quality() > (*it).second.m_HighestQ)
			(*it).second.m_HighestQ = theItem.Quality();
	}
	else
	{
		AuPlayerStatsItem item;
		item.m_Foraged = 1;
		item.m_HighestQ = theItem.Quality();
		m_Items.insert(pair<COMMODITYID, AuPlayerStatsItem>(theItem.CommodityId(), item));
	}
	m_StatsLock.unlock();
}

quint32 AuPlayerStats::ItemsForaged() const
{
	quint32 total = 0;
	for(AuPlayerStatsItems::const_iterator ii = m_Items.cbegin(); ii != m_Items.cend(); ii++)
		total += (*ii).second.m_Foraged;
	return total;
}

quint32 AuPlayerStats::ItemsHandCrafted() const
{
	quint32 total = 0;
	for(AuPlayerStatsItems::const_iterator ii = m_Items.cbegin(); ii != m_Items.cend(); ii++)
	{
		total += (*ii).second.m_CraftedAtIndustry;
		total += (*ii).second.m_CraftedByHand;
	}
	return total;
}

void AuPlayerStats::ItemHandCrafted(COMMODITYID cid, quint8 quality, bool industryRequired)
{
	m_StatsLock.lock();
	AuPlayerStatsItems::iterator it = m_Items.find(cid);
	if(it != m_Items.end())
	{
		if(industryRequired)
			(*it).second.m_CraftedAtIndustry++;
		else
			(*it).second.m_CraftedByHand++;
		if(quality > (*it).second.m_HighestQ)
			(*it).second.m_HighestQ = quality;
	}
	else
	{
		AuPlayerStatsItem item;
		if(industryRequired)
			item.m_CraftedAtIndustry = 1;
		else
			item.m_CraftedByHand = 1;
		item.m_HighestQ = quality;
		m_Items.insert(pair<COMMODITYID, AuPlayerStatsItem>(cid, item));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::ItemHandCrafted(const AuItem& theItem, bool industryRequired)
{
	ItemHandCrafted(theItem.CommodityId(), theItem.Quality(), industryRequired);
}

void AuPlayerStats::ItemPossessed(const AuItem& theItem)
{
	m_StatsLock.lock();
	AuPlayerStatsItems::iterator it = m_Items.find(theItem.CommodityId());
	if(it != m_Items.end())
	{
		(*it).second.m_HadInventory = true;
		if(theItem.Quality() > (*it).second.m_HighestQ)
			(*it).second.m_HighestQ = theItem.Quality();
	}
	else
	{
		AuPlayerStatsItem item;
		item.m_HadInventory = true;
		item.m_HighestQ = theItem.Quality();
		m_Items.insert(pair<COMMODITYID, AuPlayerStatsItem>(theItem.CommodityId(), item));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::ItemSoldAsCargo(const AuItem& theItem)
{
	m_StatsLock.lock();
	AuPlayerStatsItems::iterator it = m_Items.find(theItem.CommodityId());
	if(it != m_Items.end())
	{
		(*it).second.m_SoldCargo += theItem.Count();
		if(theItem.Quality() > (*it).second.m_HighestQ)
			(*it).second.m_HighestQ = theItem.Quality();
	}
	else
	{
		AuPlayerStatsItem item;
		item.m_SoldCargo = theItem.Count();
		item.m_HighestQ = theItem.Quality();
		m_Items.insert(pair<COMMODITYID, AuPlayerStatsItem>(theItem.CommodityId(), item));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::ItemSoldAsInventory(const AuItem& theItem)
{
	m_StatsLock.lock();
	AuPlayerStatsItems::iterator it = m_Items.find(theItem.CommodityId());
	if(it != m_Items.end())
	{
		(*it).second.m_SoldInventory += theItem.Count();
		if(theItem.Quality() > (*it).second.m_HighestQ)
			(*it).second.m_HighestQ = theItem.Quality();
	}
	else
	{
		AuPlayerStatsItem item;
		item.m_SoldInventory = theItem.Count();
		item.m_HighestQ = theItem.Quality();
		m_Items.insert(pair<COMMODITYID, AuPlayerStatsItem>(theItem.CommodityId(), item));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::ItemUsedAsWeapon(const AuItem& theItem)
{
	ItemUsedAsWeapon(theItem.CommodityId(), theItem.Quality());
}

void AuPlayerStats::ItemUsedAsWeapon(COMMODITYID cid, quint8 quality)
{
	m_StatsLock.lock();
	AuPlayerStatsItems::iterator it = m_Items.find(cid);
	if(it != m_Items.end())
	{
		(*it).second.m_UsesAsWeapon++;
		if(quality > (*it).second.m_HighestQ)
			(*it).second.m_HighestQ = quality;
	}
	else
	{
		AuPlayerStatsItem item;
		item.m_UsesAsWeapon = 1;
		item.m_HighestQ = quality;
		m_Items.insert(pair<COMMODITYID, AuPlayerStatsItem>(cid, item));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::ItemUsedOnOther(const AuItem& theItem)
{
	m_StatsLock.lock();
	AuPlayerStatsItems::iterator it = m_Items.find(theItem.CommodityId());
	if(it != m_Items.end())
	{
		(*it).second.m_UsesOnOther++;
		if(theItem.Quality() > (*it).second.m_HighestQ)
			(*it).second.m_HighestQ = theItem.Quality();
	}
	else
	{
		AuPlayerStatsItem item;
		item.m_UsesOnOther = 1;
		item.m_HighestQ = theItem.Quality();
		m_Items.insert(pair<COMMODITYID, AuPlayerStatsItem>(theItem.CommodityId(), item));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::ItemUsedOnSelf(const AuItem& theItem)
{
	m_StatsLock.lock();
	AuPlayerStatsItems::iterator it = m_Items.find(theItem.CommodityId());
	if(it != m_Items.end())
	{
		(*it).second.m_UsesOnSelf++;
		if(theItem.Quality() > (*it).second.m_HighestQ)
			(*it).second.m_HighestQ = theItem.Quality();
	}
	else
	{
		AuPlayerStatsItem item;
		item.m_UsesOnSelf = 1;
		item.m_HighestQ = theItem.Quality();
		m_Items.insert(pair<COMMODITYID, AuPlayerStatsItem>(theItem.CommodityId(), item));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::KilledBy(WEAPONEFFECT effect)
{
	m_StatsLock.lock();
	AuPlayerStatsDeaths::iterator it = m_Deaths.find(effect);
	if(it != m_Deaths.end())
		(*it).second++;
	else
		m_Deaths.insert(pair<WEAPONEFFECT, quint16>(effect, 1));
	m_StatsLock.unlock();
}

void AuPlayerStats::KilledByUnit(const DaUnit* killer, POLITICAL_STANCE stanceToUnit, COMMODITYID weaponCID)
{
	m_StatsLock.lock();
	AuPlayerStatsItems::iterator it = m_Items.find(weaponCID);
	if(it != m_Items.end())
	{
		(*it).second.m_KilledBy++;
	}
	else
	{
		AuPlayerStatsItem item;

		item.m_KilledBy = 1;

		m_Items.insert(pair<COMMODITYID, AuPlayerStatsItem>(weaponCID, item));
	}

	// Creatures killed by unit.
	const DaCreature* creature = dynamic_cast<const DaCreature*>(killer);
	if(creature)
	{
		AuSpecies species = creature->Species();
		const DaAnimal* animal = dynamic_cast<const DaAnimal*>(killer);
		if((animal && animal->IsWild()) || creature->AgentAnimal())
		{
			AuPlayerStatsCreatures::iterator it = m_CreaturesWild.find(species);
			if(it != m_CreaturesWild.end())
			{
				(*it).second.m_KilledBy++;
			}
			else
			{
				AuPlayerStatsCreature animal;

				animal.m_KilledBy = 1;

				m_CreaturesWild.insert(pair<AuSpecies, AuPlayerStatsCreature>(species, animal));
			}
		}
		else
		{
			AuPlayerStatsCreatures::iterator it = m_CreaturesTame.find(species);
			if(it != m_CreaturesTame.end())
			{
				(*it).second.m_KilledBy++;
			}
			else
			{
				AuPlayerStatsCreature animal;

				animal.m_KilledBy = 1;

				m_CreaturesTame.insert(pair<AuSpecies, AuPlayerStatsCreature>(species, animal));
			}
		}
	}

	// Avatar kills
	KILL_STAT_TYPE kst = AuPlayerStatsKills::StatType(killer, stanceToUnit);
	if(kst != KST_Invalid)
	{
		AuPlayerStatsKills::iterator it = m_Kills.find(kst);
		if(it != m_Kills.end())
		{
			(*it).second.m_KilledBy++;
		}
		else
		{
			AuPlayerStatsKill kill;

			kill.m_KilledBy = 1;

			m_Kills.insert(pair<KILL_STAT_TYPE, AuPlayerStatsKill>(kst, kill));
		}
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::KilledUnit(const DaUnit* unitKilled, POLITICAL_STANCE stanceToUnit, COMMODITYID weaponCID)
{
	m_StatsLock.lock();
	AuPlayerStatsItems::iterator it = m_Items.find(weaponCID);
	if(it != m_Items.end())
	{
		(*it).second.m_Kills++;
	}
	else
	{
		AuPlayerStatsItem item;

		item.m_Kills = 1;

		m_Items.insert(pair<COMMODITYID, AuPlayerStatsItem>(weaponCID, item));
	}

	// Creature killed unit.
	const DaCreature* creature = dynamic_cast<const DaCreature*>(unitKilled);
	if(creature)
	{
		AuSpecies species = creature->Species();
		const DaAnimal* animal = dynamic_cast<const DaAnimal*>(unitKilled);
		if((animal && animal->IsWild()) || creature->AgentAnimal())
		{
			AuPlayerStatsCreatures::iterator it = m_CreaturesWild.find(species);
			if(it != m_CreaturesWild.end())
			{
				(*it).second.m_Kills++;
			}
			else
			{
				AuPlayerStatsCreature animal;

				animal.m_Kills = 1;

				m_CreaturesWild.insert(pair<AuSpecies, AuPlayerStatsCreature>(species, animal));
			}
		}
		else
		{
			AuPlayerStatsCreatures::iterator it = m_CreaturesTame.find(species);
			if(it != m_CreaturesTame.end())
			{
				(*it).second.m_Kills++;
			}
			else
			{
				AuPlayerStatsCreature animal;

				animal.m_Kills = 1;

				m_CreaturesTame.insert(pair<AuSpecies, AuPlayerStatsCreature>(species, animal));
			}
		}
	}

	// Avatar kills
	KILL_STAT_TYPE kst = AuPlayerStatsKills::StatType(unitKilled, stanceToUnit);
	if(kst != KST_Invalid)
	{
		AuPlayerStatsKills::iterator it = m_Kills.find(kst);
		if(it != m_Kills.end())
		{
			(*it).second.m_Kills++;
		}
		else
		{
			AuPlayerStatsKill kill;

			kill.m_Kills = 1;

			m_Kills.insert(pair<KILL_STAT_TYPE, AuPlayerStatsKill>(kst, kill));
		}
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::PlanetBeamedDown(const DaWorld* toPlanet, bool toWater)
{
	m_StatsLock.lock();
	PLANET_STAT_TYPE pst;
	const DaWorldGasGiant* gasGiant = dynamic_cast<const DaWorldGasGiant*>(toPlanet);
	const DaWorldGlobe* globe = dynamic_cast<const DaWorldGlobe*>(toPlanet);
	const DaWorldRingSection* ringSection = dynamic_cast<const DaWorldRingSection*>(toPlanet);
	if(gasGiant)
		pst = AuPlayerStatsPlanets::StatType(gasGiant);
	else if(globe)
		pst = AuPlayerStatsPlanets::StatType(globe);
	else if(ringSection)
		pst = AuPlayerStatsPlanets::StatType(ringSection);
	else
	{
		m_StatsLock.unlock();
		return;
	}

	AuPlayerStatsPlanets::iterator it = m_Planets.find(pst);
	if(it != m_Planets.end())
	{
		if(toWater)
			(*it).second.m_BeamDownSea++;
		else
			(*it).second.m_BeamDownLand++;
	}
	else
	{
		AuPlayerStatsPlanet planet;

		if(toWater)
			planet.m_BeamDownSea = 1;
		else
			planet.m_BeamDownLand = 1;

		m_Planets.insert(pair<PLANET_STAT_TYPE, AuPlayerStatsPlanet>(pst, planet));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::PlanetBeamedUp(const DaWorld* fromPlanet, bool fromWater)
{
	m_StatsLock.lock();
	PLANET_STAT_TYPE pst;
	const DaWorldGasGiant* gasGiant = dynamic_cast<const DaWorldGasGiant*>(fromPlanet);
	const DaWorldGlobe* globe = dynamic_cast<const DaWorldGlobe*>(fromPlanet);
	const DaWorldRingSection* ringSection = dynamic_cast<const DaWorldRingSection*>(fromPlanet);
	if(gasGiant)
		pst = AuPlayerStatsPlanets::StatType(gasGiant);
	else if(globe)
		pst = AuPlayerStatsPlanets::StatType(globe);
	else if(ringSection)
		pst = AuPlayerStatsPlanets::StatType(ringSection);
	else
	{
		m_StatsLock.unlock();
		return;
	}

	AuPlayerStatsPlanets::iterator it = m_Planets.find(pst);
	if(it != m_Planets.end())
	{
		if(fromWater)
			(*it).second.m_BeamUpSea++;
		else
			(*it).second.m_BeamUpLand++;
	}
	else
	{
		AuPlayerStatsPlanet planet;

		if(fromWater)
			planet.m_BeamUpSea = 1;
		else
			planet.m_BeamUpLand = 1;

		m_Planets.insert(pair<PLANET_STAT_TYPE, AuPlayerStatsPlanet>(pst, planet));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::PlanetDeepestSwim(const DaWorld* planet, float depth)
{
	m_StatsLock.lock();
	PLANET_STAT_TYPE pst;
	const DaWorldGasGiant* gasGiant = dynamic_cast<const DaWorldGasGiant*>(planet);
	const DaWorldGlobe* globe = dynamic_cast<const DaWorldGlobe*>(planet);
	const DaWorldRingSection* ringSection = dynamic_cast<const DaWorldRingSection*>(planet);
	if(gasGiant)
		pst = AuPlayerStatsPlanets::StatType(gasGiant);
	else if(globe)
		pst = AuPlayerStatsPlanets::StatType(globe);
	else if(ringSection)
		pst = AuPlayerStatsPlanets::StatType(ringSection);
	else
	{
		m_StatsLock.unlock();
		return;
	}

	AuPlayerStatsPlanets::iterator it = m_Planets.find(pst);
	if(it != m_Planets.end())
	{
		if(depth > (*it).second.m_DeepestSwim)
			(*it).second.m_DeepestSwim = depth;
	}
	else
	{
		AuPlayerStatsPlanet planet;

		planet.m_DeepestSwim = depth;

		m_Planets.insert(pair<PLANET_STAT_TYPE, AuPlayerStatsPlanet>(pst, planet));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::PlanetHighestFall(const DaWorld* planet, float altitude)
{
	m_StatsLock.lock();
	PLANET_STAT_TYPE pst;
	const DaWorldGasGiant* gasGiant = dynamic_cast<const DaWorldGasGiant*>(planet);
	const DaWorldGlobe* globe = dynamic_cast<const DaWorldGlobe*>(planet);
	const DaWorldRingSection* ringSection = dynamic_cast<const DaWorldRingSection*>(planet);
	if(gasGiant)
		pst = AuPlayerStatsPlanets::StatType(gasGiant);
	else if(globe)
		pst = AuPlayerStatsPlanets::StatType(globe);
	else if(ringSection)
		pst = AuPlayerStatsPlanets::StatType(ringSection);
	else
	{
		m_StatsLock.unlock();
		return;
	}

	AuPlayerStatsPlanets::iterator it = m_Planets.find(pst);
	if(it != m_Planets.end())
	{
		if(altitude > (*it).second.m_HighestFall)
			(*it).second.m_HighestFall = altitude;
	}
	else
	{
		AuPlayerStatsPlanet planet;

		planet.m_HighestFall = altitude;

		m_Planets.insert(pair<PLANET_STAT_TYPE, AuPlayerStatsPlanet>(pst, planet));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::PlanetEntry(const DaWorld* planet, const DaAtom* ridingAboard)
{
	m_StatsLock.lock();
	PLANET_STAT_TYPE pst;
	const DaWorldGasGiant* gasGiant = dynamic_cast<const DaWorldGasGiant*>(planet);
	const DaWorldGlobe* globe = dynamic_cast<const DaWorldGlobe*>(planet);
	const DaWorldRingSection* ringSection = dynamic_cast<const DaWorldRingSection*>(planet);
	if(gasGiant)
		pst = AuPlayerStatsPlanets::StatType(gasGiant);
	else if(globe)
		pst = AuPlayerStatsPlanets::StatType(globe);
	else if(ringSection)
		pst = AuPlayerStatsPlanets::StatType(ringSection);
	else
	{
		m_StatsLock.unlock();
		return;
	}

	AuPlayerStatsPlanets::iterator it = m_Planets.find(pst);
	if(it != m_Planets.end())
	{
		if(!ridingAboard)
			(*it).second.m_EntryEVA++;
		else
		{
			const DaVessel* ship = dynamic_cast<const DaVessel*>(ridingAboard);
			if(ship)
			{
				switch(ship->Structure())
				{
				case SC_Starship:
					(*it).second.m_EntryStarship++;
					break;
				case SC_Hull:
				case SC_Count:
				case SC_Spaceship:
				case SC_Deprecated:
					(*it).second.m_EntrySpaceship++;
					break;
				case SC_Station:
					(*it).second.m_EntryStation++;
					break;
				}
			}
			else
			{
				const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(ridingAboard);
				if(vehicle)
				{
					switch(vehicle->CommodityId())
					{
					case CID_DirtBike:
						(*it).second.m_EntryMotorBike++;
						break;
					case CID_SailBoat:
						(*it).second.m_EntrySailBoat++;
						break;
					case CID_SpaceFighter:
						(*it).second.m_EntrySpaceFighterF1++;
						break;
					case CID_SpaceTransport:
						(*it).second.m_EntrySpaceTransport++;
						break;
					case CID_AdvSpaceFighter:
						(*it).second.m_EntrySpaceFighterF2++;
						break;
					case CID_SpaceRocket:
						(*it).second.m_EntryRocket++;
						break;
					case CID_Helicopter:
						(*it).second.m_EntryHelicopter++;
						break;
					case CID_Submarine:
						(*it).second.m_EntrySubmarine++;
						break;
					case CID_SUV:
						(*it).second.m_EntrySUV++;
						break;
					case CID_APC:
						(*it).second.m_EntryAPC++;
						break;
					case CID_FarmMachinery:
						(*it).second.m_EntryFarmMachinery++;
						break;
					default:
						break;
					}
				}
			}
		}
	}
	else
	{
		AuPlayerStatsPlanet planet;

		if(!ridingAboard)
			planet.m_EntryEVA = 1;
		else
		{
			const DaVessel* ship = dynamic_cast<const DaVessel*>(ridingAboard);
			if(ship)
			{
				switch(ship->Structure())
				{
				case SC_Starship:
					planet.m_EntryStarship = 1;
					break;
				case SC_Hull:
				case SC_Count:
				case SC_Spaceship:
				case SC_Deprecated:
					planet.m_EntrySpaceship = 1;
					break;
				case SC_Station:
					planet.m_EntryStation = 1;
					break;
				}
			}
			else
			{
				const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(ridingAboard);
				if(vehicle)
				{
					switch(vehicle->CommodityId())
					{
					case CID_DirtBike:
						planet.m_EntryMotorBike = 1;
						break;
					case CID_SailBoat:
						planet.m_EntrySailBoat = 1;
						break;
					case CID_SpaceFighter:
						planet.m_EntrySpaceFighterF1 = 1;
						break;
					case CID_SpaceTransport:
						planet.m_EntrySpaceTransport = 1;
						break;
					case CID_AdvSpaceFighter:
						planet.m_EntrySpaceFighterF2 = 1;
						break;
					case CID_SpaceRocket:
						planet.m_EntryRocket = 1;
						break;
					case CID_Helicopter:
						planet.m_EntryHelicopter = 1;
						break;
					case CID_Submarine:
						planet.m_EntrySubmarine = 1;
						break;
					case CID_SUV:
						planet.m_EntrySUV = 1;
						break;
					case CID_APC:
						planet.m_EntryAPC = 1;
						break;
					case CID_FarmMachinery:
						planet.m_EntryFarmMachinery = 1;
						break;
					default:
						break;
					}
				}
			}
		}

		m_Planets.insert(pair<PLANET_STAT_TYPE, AuPlayerStatsPlanet>(pst, planet));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::PlanetExit(const DaWorld* planet, const DaAtom* ridingAboard)
{
	m_StatsLock.lock();
	PLANET_STAT_TYPE pst;
	const DaWorldGasGiant* gasGiant = dynamic_cast<const DaWorldGasGiant*>(planet);
	const DaWorldGlobe* globe = dynamic_cast<const DaWorldGlobe*>(planet);
	const DaWorldRingSection* ringSection = dynamic_cast<const DaWorldRingSection*>(planet);
	if(gasGiant)
		pst = AuPlayerStatsPlanets::StatType(gasGiant);
	else if(globe)
		pst = AuPlayerStatsPlanets::StatType(globe);
	else if(ringSection)
		pst = AuPlayerStatsPlanets::StatType(ringSection);
	else
	{
		m_StatsLock.unlock();
		return;
	}

	AuPlayerStatsPlanets::iterator it = m_Planets.find(pst);
	if(it != m_Planets.end())
	{
		if(!ridingAboard)
			(*it).second.m_ExitEVA++;
		else
		{
			const DaVessel* ship = dynamic_cast<const DaVessel*>(ridingAboard);
			if(ship)
			{
				switch(ship->Structure())
				{
				case SC_Starship:
					(*it).second.m_ExitStarship++;
					break;
				case SC_Hull:
				case SC_Count:
				case SC_Spaceship:
				case SC_Deprecated:
					(*it).second.m_ExitSpaceship++;
					break;
				case SC_Station:
					(*it).second.m_ExitStation++;
					break;
				}
			}
			else
			{
				const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(ridingAboard);
				if(vehicle)
				{
					switch(vehicle->CommodityId())
					{
					case CID_DirtBike:
						(*it).second.m_ExitMotorBike++;
						break;
					case CID_SailBoat:
						(*it).second.m_ExitSailBoat++;
						break;
					case CID_SpaceFighter:
						(*it).second.m_ExitSpaceFighterF1++;
						break;
					case CID_SpaceTransport:
						(*it).second.m_ExitSpaceTransport++;
						break;
					case CID_AdvSpaceFighter:
						(*it).second.m_ExitSpaceFighterF2++;
						break;
					case CID_SpaceRocket:
						(*it).second.m_ExitRocket++;
						break;
					case CID_Helicopter:
						(*it).second.m_ExitHelicopter++;
						break;
					case CID_Submarine:
						(*it).second.m_ExitSubmarine++;
						break;
					case CID_SUV:
						(*it).second.m_ExitSUV++;
						break;
					case CID_APC:
						(*it).second.m_ExitAPC++;
						break;
					case CID_FarmMachinery:
						(*it).second.m_ExitFarmMachinery++;
						break;
					default:
						break;
					}
				}
			}
		}
	}
	else
	{
		AuPlayerStatsPlanet planet;

		if(!ridingAboard)
			planet.m_ExitEVA = 1;
		else
		{
			const DaVessel* ship = dynamic_cast<const DaVessel*>(ridingAboard);
			if(ship)
			{
				switch(ship->Structure())
				{
				case SC_Starship:
					planet.m_ExitStarship = 1;
					break;
				case SC_Hull:
				case SC_Count:
				case SC_Spaceship:
				case SC_Deprecated:
					planet.m_ExitSpaceship = 1;
					break;
				case SC_Station:
					planet.m_ExitStation = 1;
					break;
				}
			}
			else
			{
				const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(ridingAboard);
				if(vehicle)
				{
					switch(vehicle->CommodityId())
					{
					case CID_DirtBike:
						planet.m_ExitMotorBike = 1;
						break;
					case CID_SailBoat:
						planet.m_ExitSailBoat = 1;
						break;
					case CID_SpaceFighter:
						planet.m_ExitSpaceFighterF1 = 1;
						break;
					case CID_SpaceTransport:
						planet.m_ExitSpaceTransport = 1;
						break;
					case CID_AdvSpaceFighter:
						planet.m_ExitSpaceFighterF2 = 1;
						break;
					case CID_SpaceRocket:
						planet.m_ExitRocket = 1;
						break;
					case CID_Helicopter:
						planet.m_ExitHelicopter = 1;
						break;
					case CID_Submarine:
						planet.m_ExitSubmarine = 1;
						break;
					case CID_SUV:
						planet.m_ExitSUV = 1;
						break;
					case CID_APC:
						planet.m_ExitAPC = 1;
						break;
					case CID_FarmMachinery:
						planet.m_ExitFarmMachinery = 1;
						break;
					default:
						break;
					}
				}
			}
		}

		m_Planets.insert(pair<PLANET_STAT_TYPE, AuPlayerStatsPlanet>(pst, planet));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::PlanetTreaded(const DaWorld* planet)
{
	m_StatsLock.lock();
	PLANET_STAT_TYPE pst;
	const DaWorldGasGiant* gasGiant = dynamic_cast<const DaWorldGasGiant*>(planet);
	const DaWorldGlobe* globe = dynamic_cast<const DaWorldGlobe*>(planet);
	const DaWorldRingSection* ringSection = dynamic_cast<const DaWorldRingSection*>(planet);
	if(gasGiant)
		pst = AuPlayerStatsPlanets::StatType(gasGiant);
	else if(globe)
		pst = AuPlayerStatsPlanets::StatType(globe);
	else if(ringSection)
		pst = AuPlayerStatsPlanets::StatType(ringSection);
	else
	{
		m_StatsLock.unlock();
		return;
	}

	AuPlayerStatsPlanets::iterator it = m_Planets.find(pst);
	if(it != m_Planets.end())
	{
		(*it).second.m_Treaded = true;
	}
	else
	{
		AuPlayerStatsPlanet planet;

		planet.m_Treaded = true;

		m_Planets.insert(pair<PLANET_STAT_TYPE, AuPlayerStatsPlanet>(pst, planet));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::SunBurn(const DaStar* star, const DaAtom* ridingAboard)
{
	m_StatsLock.lock();
	STAR_STAT_TYPE sst = AuPlayerStatsStars::StatType(star);
	AuPlayerStatsStars::iterator it = m_Stars.find(sst);
	if(it != m_Stars.end())
	{
		if(!ridingAboard)
			(*it).second.m_SunBurnEVA++;
		else
		{
			const DaVessel* ship = dynamic_cast<const DaVessel*>(ridingAboard);
			if(ship)
			{
				switch(ship->Structure())
				{
				case SC_Starship:
					(*it).second.m_SunBurnStarship++;
					break;
				case SC_Hull:
				case SC_Count:
				case SC_Spaceship:
				case SC_Deprecated:
					(*it).second.m_SunBurnSpaceship++;
					break;
				case SC_Station:
					(*it).second.m_SunBurnStation++;
					break;
				}
			}
			else
			{
				const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(ridingAboard);
				if(vehicle)
				{
					switch(vehicle->CommodityId())
					{
					case CID_DirtBike:
						(*it).second.m_SunBurnMotorBike++;
						break;
					case CID_SailBoat:
						(*it).second.m_SunBurnSailBoat++;
						break;
					case CID_SpaceFighter:
						(*it).second.m_SunBurnSpaceFighterF1++;
						break;
					case CID_SpaceTransport:
						(*it).second.m_SunBurnSpaceTransport++;
						break;
					case CID_AdvSpaceFighter:
						(*it).second.m_SunBurnSpaceFighterF2++;
						break;
					case CID_SpaceRocket:
						(*it).second.m_SunBurnRocket++;
						break;
					case CID_Helicopter:
						(*it).second.m_SunBurnHelicopter++;
						break;
					case CID_Submarine:
						(*it).second.m_SunBurnSubmarine++;
						break;
					case CID_SUV:
						(*it).second.m_SunBurnSUV++;
						break;
					case CID_APC:
						(*it).second.m_SunBurnAPC++;
						break;
					case CID_FarmMachinery:
						(*it).second.m_SunBurnFarmMachinery++;
						break;
					default:
						break;
					}
				}
			}
		}
	}
	else
	{
		AuPlayerStatsStar star;

		if(!ridingAboard)
			star.m_SunBurnEVA = 1;
		else
		{
			const DaVessel* ship = dynamic_cast<const DaVessel*>(ridingAboard);
			if(ship)
			{
				switch(ship->Structure())
				{
				case SC_Starship:
					star.m_SunBurnStarship = 1;
					break;
				case SC_Hull:
				case SC_Count:
				case SC_Spaceship:
				case SC_Deprecated:
					star.m_SunBurnSpaceship = 1;
					break;
				case SC_Station:
					star.m_SunBurnStation = 1;
					break;
				}
			}
			else
			{
				const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(ridingAboard);
				if(vehicle)
				{
					switch(vehicle->CommodityId())
					{
					case CID_DirtBike:
						star.m_SunBurnMotorBike = 1;
						break;
					case CID_SailBoat:
						star.m_SunBurnSailBoat = 1;
						break;
					case CID_SpaceFighter:
						star.m_SunBurnSpaceFighterF1 = 1;
						break;
					case CID_SpaceTransport:
						star.m_SunBurnSpaceTransport = 1;
						break;
					case CID_AdvSpaceFighter:
						star.m_SunBurnSpaceFighterF2 = 1;
						break;
					case CID_SpaceRocket:
						star.m_SunBurnRocket = 1;
						break;
					case CID_Helicopter:
						star.m_SunBurnHelicopter = 1;
						break;
					case CID_Submarine:
						star.m_SunBurnSubmarine = 1;
						break;
					case CID_SUV:
						star.m_SunBurnSUV = 1;
						break;
					case CID_APC:
						star.m_SunBurnAPC = 1;
						break;
					case CID_FarmMachinery:
						star.m_SunBurnFarmMachinery = 1;
						break;
					default:
						break;
					}
				}
			}
		}

		m_Stars.insert(pair<STAR_STAT_TYPE, AuPlayerStatsStar>(sst, star));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::BoardedCrew(quint32 count)
{
	m_StatsLock.lock();
	m_BoardedCrew += count;
	m_StatsLock.unlock();
}

void AuPlayerStats::BoardedCryo(quint32 count)
{
	m_StatsLock.lock();
	m_BoardedCryo += count;
	m_StatsLock.unlock();
}

void AuPlayerStats::BoardedOfficer()
{
	m_StatsLock.lock();
	m_BoardedOfficers++;
	m_StatsLock.unlock();
}

void AuPlayerStats::BoardedPassengers(quint32 count)
{
	m_StatsLock.lock();
	m_BoardedPassengers += count;
	m_StatsLock.unlock();
}

void AuPlayerStats::BoardedTroops(quint32 count)
{
	m_StatsLock.lock();
	m_BoardedTroops += count;
	m_StatsLock.unlock();
}

void AuPlayerStats::DebarkedCryo(quint32 count)
{
	m_StatsLock.lock();
	m_DebarkedCryo += count;
	m_StatsLock.unlock();
}

void AuPlayerStats::DebarkedPassengers(quint32 count)
{
	m_StatsLock.lock();
	m_DebarkedPassengers += count;
	m_StatsLock.unlock();
}

void AuPlayerStats::EVA()
{
	m_StatsLock.lock();
	m_EVAMinutes++;
	m_StatsLock.unlock();
}

void AuPlayerStats::EVABeamedTo()
{
	m_StatsLock.lock();
	m_EVABeamTo++;
	m_StatsLock.unlock();
}

void AuPlayerStats::EVABeamedFrom()
{
	m_StatsLock.lock();
	m_EVABeamFrom++;
	m_StatsLock.unlock();
}

void AuPlayerStats::EVADeadHead()
{
	m_StatsLock.lock();
	m_EVADeadHead++;
	m_StatsLock.unlock();
}

int AuPlayerStats::EVAMinutes() const
{
	m_StatsLock.lock();
	int v = m_EVAMinutes;
	m_StatsLock.unlock();
	return v;
}

int AuPlayerStats::StationMinutes(AuStation::TYPE type, bool isShipStation) const
{
	m_StatsLock.lock();
	int v = m_Stations.Minutes(type, isShipStation);
	m_StatsLock.unlock();
	return v;
}

int AuPlayerStats::VehicleMinutes(COMMODITYID vehicleCID) const
{
	m_StatsLock.lock();
	int v = m_Stations.Minutes(vehicleCID);
	m_StatsLock.unlock();
	return v;
}

void AuPlayerStats::EVASectorChange()
{
	m_StatsLock.lock();
	m_EVASectorTraversed++;
	m_StatsLock.unlock();
}

void AuPlayerStats::SpacecraftCaptured(const DaVessel* ship)
{
	CRAFT_STAT_TYPE cst = AuPlayerStatsCrafts::StatType(ship);
	if(cst == CST_Invalid)
		return;
	m_StatsLock.lock();
	AuPlayerStatsCrafts::iterator it = m_ShipsAndVehicles.find(cst);
	if(it != m_ShipsAndVehicles.end())
	{
		(*it).second.m_Captures++;
	}
	else
	{
		AuPlayerStatsCraft craft;

		craft.m_Captures = 1;

		m_ShipsAndVehicles.insert(pair<CRAFT_STAT_TYPE, AuPlayerStatsCraft>(cst, craft));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::VehicleCaptured(const DaVehicle* vehicle)
{
	CRAFT_STAT_TYPE cst = AuPlayerStatsCrafts::StatType(vehicle);
	if(cst == CST_Invalid)
		return;
	m_StatsLock.lock();
	AuPlayerStatsCrafts::iterator it = m_ShipsAndVehicles.find(cst);
	if(it != m_ShipsAndVehicles.end())
	{
		(*it).second.m_Captures++;
	}
	else
	{
		AuPlayerStatsCraft craft;

		craft.m_Captures = 1;

		m_ShipsAndVehicles.insert(pair<CRAFT_STAT_TYPE, AuPlayerStatsCraft>(cst, craft));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::SpacecraftUpgradeBought(const DaVessel* ship)
{
	CRAFT_STAT_TYPE cst = AuPlayerStatsCrafts::StatType(ship);
	if(cst == CST_Invalid)
		return;
	m_StatsLock.lock();
	AuPlayerStatsCrafts::iterator it = m_ShipsAndVehicles.find(cst);
	if(it != m_ShipsAndVehicles.end())
	{
		(*it).second.m_UpgradesBought++;
	}
	else
	{
		AuPlayerStatsCraft craft;

		craft.m_UpgradesBought = 1;

		m_ShipsAndVehicles.insert(pair<CRAFT_STAT_TYPE, AuPlayerStatsCraft>(cst, craft));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::SpacecraftRepairsBought(const DaVessel* ship)
{
	CRAFT_STAT_TYPE cst = AuPlayerStatsCrafts::StatType(ship);
	if(cst == CST_Invalid)
		return;
	m_StatsLock.lock();
	AuPlayerStatsCrafts::iterator it = m_ShipsAndVehicles.find(cst);
	if(it != m_ShipsAndVehicles.end())
	{
		(*it).second.m_RepairsBought++;
	}
	else
	{
		AuPlayerStatsCraft craft;

		craft.m_RepairsBought = 1;

		m_ShipsAndVehicles.insert(pair<CRAFT_STAT_TYPE, AuPlayerStatsCraft>(cst, craft));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::VehicleRepairsBought(const DaVehicle* vehicle)
{
	CRAFT_STAT_TYPE cst = AuPlayerStatsCrafts::StatType(vehicle);
	if(cst == CST_Invalid)
		return;
	m_StatsLock.lock();
	AuPlayerStatsCrafts::iterator it = m_ShipsAndVehicles.find(cst);
	if(it != m_ShipsAndVehicles.end())
	{
		(*it).second.m_RepairsBought++;
	}
	else
	{
		AuPlayerStatsCraft craft;

		craft.m_RepairsBought = 1;

		m_ShipsAndVehicles.insert(pair<CRAFT_STAT_TYPE, AuPlayerStatsCraft>(cst, craft));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::SpacecraftRepairedByHand(const DaVessel* ship)
{
	CRAFT_STAT_TYPE cst = AuPlayerStatsCrafts::StatType(ship);
	if(cst == CST_Invalid)
		return;
	m_StatsLock.lock();
	AuPlayerStatsCrafts::iterator it = m_ShipsAndVehicles.find(cst);
	if(it != m_ShipsAndVehicles.end())
	{
		(*it).second.m_RepairsByHand++;
	}
	else
	{
		AuPlayerStatsCraft craft;

		craft.m_RepairsByHand = 1;

		m_ShipsAndVehicles.insert(pair<CRAFT_STAT_TYPE, AuPlayerStatsCraft>(cst, craft));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::VehicleRepairedByHand(const DaVehicle* vehicle)
{
	CRAFT_STAT_TYPE cst = AuPlayerStatsCrafts::StatType(vehicle);
	if(cst == CST_Invalid)
		return;
	m_StatsLock.lock();
	AuPlayerStatsCrafts::iterator it = m_ShipsAndVehicles.find(cst);
	if(it != m_ShipsAndVehicles.end())
	{
		(*it).second.m_RepairsByHand++;
	}
	else
	{
		AuPlayerStatsCraft craft;

		craft.m_RepairsByHand = 1;

		m_ShipsAndVehicles.insert(pair<CRAFT_STAT_TYPE, AuPlayerStatsCraft>(cst, craft));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::DeadHeaded(const DaAtom* ridingAboard)
{
	CRAFT_STAT_TYPE cst = AuPlayerStatsCrafts::StatType(ridingAboard);
	if(cst == CST_Invalid)
		return;
	m_StatsLock.lock();
	AuPlayerStatsCrafts::iterator it = m_ShipsAndVehicles.find(cst);
	if(it != m_ShipsAndVehicles.end())
	{
		(*it).second.m_DeadHeads++;
	}
	else
	{
		AuPlayerStatsCraft craft;

		craft.m_DeadHeads = 1;

		m_ShipsAndVehicles.insert(pair<CRAFT_STAT_TYPE, AuPlayerStatsCraft>(cst, craft));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::SectorChange(const DaAtom* ridingAboard)
{
	CRAFT_STAT_TYPE cst = AuPlayerStatsCrafts::StatType(ridingAboard);
	if(cst == CST_Invalid)
		return;
	m_StatsLock.lock();
	AuPlayerStatsCrafts::iterator it = m_ShipsAndVehicles.find(cst);
	if(it != m_ShipsAndVehicles.end())
	{
		(*it).second.m_SectorsTraversed++;
	}
	else
	{
		AuPlayerStatsCraft craft;

		craft.m_SectorsTraversed = 1;

		m_ShipsAndVehicles.insert(pair<CRAFT_STAT_TYPE, AuPlayerStatsCraft>(cst, craft));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::Wormholed(const DaVessel* ridingAboard)
{
	CRAFT_STAT_TYPE cst = AuPlayerStatsCrafts::StatType(ridingAboard);
	if(cst == CST_Invalid)
		return;
	m_StatsLock.lock();
	AuPlayerStatsCrafts::iterator it = m_ShipsAndVehicles.find(cst);
	if(it != m_ShipsAndVehicles.end())
	{
		(*it).second.m_Wormholes++;
	}
	else
	{
		AuPlayerStatsCraft craft;

		craft.m_Wormholes = 1;

		m_ShipsAndVehicles.insert(pair<CRAFT_STAT_TYPE, AuPlayerStatsCraft>(cst, craft));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::WormholeExplored()
{
	m_WormholesExplored++;
}

void AuPlayerStats::WormholeNamed()
{
	m_WormholesNamed++;
}

void AuPlayerStats::BuildingStationEntered(AuStation::TYPE type, quint8 quality)
{
	STATION_STAT_TYPE est = AuPlayerStatsStations::StatType(type, false);
	if(est == EST_Invalid)
		return;
	m_StatsLock.lock();
	AuPlayerStatsStations::iterator it = m_Stations.find(est);
	if(it != m_Stations.end())
	{
		if(quality > (*it).second.m_HighestQ)
			(*it).second.m_HighestQ = quality;
	}
	else
	{
		AuPlayerStatsStation station;
		station.m_HighestQ = quality;
		m_Stations.insert(pair<STATION_STAT_TYPE, AuPlayerStatsStation>(est, station));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::SpacecraftStationEntered(AuStation::TYPE type, quint8 quality)
{
	STATION_STAT_TYPE est = AuPlayerStatsStations::StatType(type, true);
	if(est == EST_Invalid)
		return;
	m_StatsLock.lock();
	AuPlayerStatsStations::iterator it = m_Stations.find(est);
	if(it != m_Stations.end())
	{
		if(quality > (*it).second.m_HighestQ)
			(*it).second.m_HighestQ = quality;
	}
	else
	{
		AuPlayerStatsStation station;
		station.m_HighestQ = quality;
		m_Stations.insert(pair<STATION_STAT_TYPE, AuPlayerStatsStation>(est, station));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::VehicleEntered(const DaVehicle* vehicle)
{
	STATION_STAT_TYPE est = AuPlayerStatsStations::StatType(vehicle);
	if(est == EST_Invalid)
		return;
	m_StatsLock.lock();
	AuPlayerStatsStations::iterator it = m_Stations.find(est);
	if(it != m_Stations.end())
	{
		if(vehicle->Quality() > (*it).second.m_HighestQ)
			(*it).second.m_HighestQ = vehicle->Quality();
	}
	else
	{
		AuPlayerStatsStation station;
		station.m_HighestQ = vehicle->Quality();
		m_Stations.insert(pair<STATION_STAT_TYPE, AuPlayerStatsStation>(est, station));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::BuildingStationOperated(AuStation::TYPE type, quint8 quality)
{
	STATION_STAT_TYPE est = AuPlayerStatsStations::StatType(type, false);
	if(est == EST_Invalid)
		return;
	m_StatsLock.lock();
	AuPlayerStatsStations::iterator it = m_Stations.find(est);
	if(it != m_Stations.end())
	{
		(*it).second.m_Minutes++;
		if(quality > (*it).second.m_HighestQ)
			(*it).second.m_HighestQ = quality;
	}
	else
	{
		AuPlayerStatsStation station;
		station.m_Minutes = 1;
		station.m_HighestQ = quality;
		m_Stations.insert(pair<STATION_STAT_TYPE, AuPlayerStatsStation>(est, station));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::SpacecraftStationOperated(AuStation::TYPE type, quint8 quality)
{
	STATION_STAT_TYPE est = AuPlayerStatsStations::StatType(type, true);
	if(est == EST_Invalid)
		return;
	m_StatsLock.lock();
	AuPlayerStatsStations::iterator it = m_Stations.find(est);
	if(it != m_Stations.end())
	{
		(*it).second.m_Minutes++;
		if(quality > (*it).second.m_HighestQ)
			(*it).second.m_HighestQ = quality;
	}
	else
	{
		AuPlayerStatsStation station;
		station.m_Minutes = 1;
		station.m_HighestQ = quality;
		m_Stations.insert(pair<STATION_STAT_TYPE, AuPlayerStatsStation>(est, station));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::SpacecraftSystemRepaired(SYSTEM system)
{
	m_StatsLock.lock();
	AuPlayerStatsSystems::iterator it = m_Systems.find(system);
	if(it != m_Systems.end())
	{
		(*it).second.m_Repairs++;
	}
	else
	{
		AuPlayerStatsSystem sys;

		sys.m_Repairs = 1;

		m_Systems.insert(pair<SYSTEM, AuPlayerStatsSystem>(system, sys));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::SpacecraftSystemUpgraded(SYSTEM system)
{
	m_StatsLock.lock();
	AuPlayerStatsSystems::iterator it = m_Systems.find(system);
	if(it != m_Systems.end())
	{
		(*it).second.m_Upgrades++;
	}
	else
	{
		AuPlayerStatsSystem sys;

		sys.m_Upgrades = 1;

		m_Systems.insert(pair<SYSTEM, AuPlayerStatsSystem>(system, sys));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::VehicleOperated(const DaVehicle* vehicle)
{
	STATION_STAT_TYPE est = AuPlayerStatsStations::StatType(vehicle);
	if(est == EST_Invalid)
		return;
	m_StatsLock.lock();
	AuPlayerStatsStations::iterator it = m_Stations.find(est);
	if(it != m_Stations.end())
	{
		(*it).second.m_Minutes++;
		if(vehicle->Quality() > (*it).second.m_HighestQ)
			(*it).second.m_HighestQ = vehicle->Quality();
	}
	else
	{
		AuPlayerStatsStation station;
		station.m_Minutes = 1;
		station.m_HighestQ = vehicle->Quality();
		m_Stations.insert(pair<STATION_STAT_TYPE, AuPlayerStatsStation>(est, station));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::AnimalRidden()
{
	m_StatsLock.lock();
	STATION_STAT_TYPE est = EST_Animal;
	AuPlayerStatsStations::iterator it = m_Stations.find(est);
	if(it != m_Stations.end())
	{
		(*it).second.m_Minutes++;
	}
	else
	{
		AuPlayerStatsStation station;

		station.m_Minutes = 1;

		m_Stations.insert(pair<STATION_STAT_TYPE, AuPlayerStatsStation>(est, station));
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::CreatureDNACollected(const DaCreature* creature)
{
	m_StatsLock.lock();
	AuSpecies species = creature->Species();
	const DaAnimal* animal = dynamic_cast<const DaAnimal*>(creature);
	if((animal && animal->IsWild()) || creature->AgentAnimal())
	{
		AuPlayerStatsCreatures::iterator it = m_CreaturesWild.find(species);
		if(it != m_CreaturesWild.end())
		{
			(*it).second.m_DNA++;
		}
		else
		{
			AuPlayerStatsCreature animal;

			animal.m_DNA = 1;

			m_CreaturesWild.insert(pair<AuSpecies, AuPlayerStatsCreature>(species, animal));
		}
	}
	else
	{
		AuPlayerStatsCreatures::iterator it = m_CreaturesTame.find(species);
		if(it != m_CreaturesTame.end())
		{
			(*it).second.m_DNA++;
		}
		else
		{
			AuPlayerStatsCreature animal;

			animal.m_DNA = 1;

			m_CreaturesTame.insert(pair<AuSpecies, AuPlayerStatsCreature>(species, animal));
		}
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::CreatureSpeciesNamed(const AuSpecies& species, bool isWildAnimal)
{
	m_StatsLock.lock();
	m_NamedSpecies++;

	if(isWildAnimal)
	{
		AuPlayerStatsCreatures::iterator it = m_CreaturesWild.find(species);
		if(it != m_CreaturesWild.end())
		{
			(*it).second.m_Named++;
		}
		else
		{
			AuPlayerStatsCreature animal;

			animal.m_Named = 1;

			m_CreaturesWild.insert(pair<AuSpecies, AuPlayerStatsCreature>(species, animal));
		}
	}
	else
	{
		AuPlayerStatsCreatures::iterator it = m_CreaturesTame.find(species);
		if(it != m_CreaturesTame.end())
		{
			(*it).second.m_Named++;
		}
		else
		{
			AuPlayerStatsCreature animal;

			animal.m_Named = 1;

			m_CreaturesTame.insert(pair<AuSpecies, AuPlayerStatsCreature>(species, animal));
		}
	}
	m_StatsLock.unlock();
}

quint32 AuPlayerStats::CreatureSpeciesNamed() const
{
	return m_NamedSpecies;
}

void AuPlayerStats::CreatureSpecieStudied(const DaCreature* creature)
{
	m_StatsLock.lock();
	AuSpecies species = creature->Species();
	const DaAnimal* animal = dynamic_cast<const DaAnimal*>(creature);
	if((animal && animal->IsWild()) || creature->AgentAnimal())
	{
		AuPlayerStatsCreatures::iterator it = m_CreaturesWild.find(species);
		if(it != m_CreaturesWild.end())
		{
			(*it).second.m_Studied = true;
		}
		else
		{
			AuPlayerStatsCreature animal;

			animal.m_Studied = true;

			m_CreaturesWild.insert(pair<AuSpecies, AuPlayerStatsCreature>(species, animal));
		}
	}
	else
	{
		AuPlayerStatsCreatures::iterator it = m_CreaturesTame.find(species);
		if(it != m_CreaturesTame.end())
		{
			(*it).second.m_Studied = true;
		}
		else
		{
			AuPlayerStatsCreature animal;

			animal.m_Studied = true;

			m_CreaturesTame.insert(pair<AuSpecies, AuPlayerStatsCreature>(species, animal));
		}
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::BuildingConstruction(INDUSTRYID iid, const DaWorld* world, quint32 laborApplied)
{
	Q_ASSERT(world);
	m_StatsLock.lock();
	if(world->HasBreathableAtmosphere())
	{
		AuPlayerStatsBuildings::iterator it = m_BuildingsHabitable.find(iid);
		if(it != m_BuildingsHabitable.end())
		{
			(*it).second.m_ConstructionLabor += laborApplied;
		}
		else
		{
			AuPlayerStatsBuilding building;

			building.m_ConstructionLabor = laborApplied;

			m_BuildingsHabitable.insert(pair<INDUSTRYID, AuPlayerStatsBuilding>(iid, building));
		}
	}
	else
	{
		AuPlayerStatsBuildings::iterator it = m_BuildingsHarsh.find(iid);
		if(it != m_BuildingsHarsh.end())
		{
			(*it).second.m_ConstructionLabor += laborApplied;
		}
		else
		{
			AuPlayerStatsBuilding building;

			building.m_ConstructionLabor = laborApplied;

			m_BuildingsHarsh.insert(pair<INDUSTRYID, AuPlayerStatsBuilding>(iid, building));
		}
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::BuildingManufacture(INDUSTRYID iid, const DaWorld* world)
{
	Q_ASSERT(world);
	m_StatsLock.lock();
	if(world->HasBreathableAtmosphere())
	{
		AuPlayerStatsBuildings::iterator it = m_BuildingsHabitable.find(iid);
		if(it != m_BuildingsHabitable.end())
		{
			(*it).second.m_ManufactureGo++;
		}
		else
		{
			AuPlayerStatsBuilding building;

			building.m_ManufactureGo = 1;

			m_BuildingsHabitable.insert(pair<INDUSTRYID, AuPlayerStatsBuilding>(iid, building));
		}
	}
	else
	{
		AuPlayerStatsBuildings::iterator it = m_BuildingsHarsh.find(iid);
		if(it != m_BuildingsHarsh.end())
		{
			(*it).second.m_ManufactureGo++;
		}
		else
		{
			AuPlayerStatsBuilding building;

			building.m_ManufactureGo = 1;

			m_BuildingsHarsh.insert(pair<INDUSTRYID, AuPlayerStatsBuilding>(iid, building));
		}
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::BuildingPlanned(INDUSTRYID iid, const DaWorld* world, int levels)
{
	Q_ASSERT(world);
	m_StatsLock.lock();
	if(world->HasBreathableAtmosphere())
	{
		AuPlayerStatsBuildings::iterator it = m_BuildingsHabitable.find(iid);
		if(it != m_BuildingsHabitable.end())
		{
			(*it).second.m_Built += levels;
		}
		else
		{
			AuPlayerStatsBuilding building;

			building.m_Built = levels;

			m_BuildingsHabitable.insert(pair<INDUSTRYID, AuPlayerStatsBuilding>(iid, building));
		}
	}
	else
	{
		AuPlayerStatsBuildings::iterator it = m_BuildingsHarsh.find(iid);
		if(it != m_BuildingsHarsh.end())
		{
			(*it).second.m_Built += levels;
		}
		else
		{
			AuPlayerStatsBuilding building;

			building.m_Built = levels;

			m_BuildingsHarsh.insert(pair<INDUSTRYID, AuPlayerStatsBuilding>(iid, building));
		}
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::BuildingDesignBought(quint32 designId)
{
	m_StatsLock.lock();
	AuPlayerStatsBuildingDesigns::iterator it = m_BuildingDesignsBought.find(designId);
	if(it != m_BuildingDesignsBought.end())
		(*it).second++;
	else
		m_BuildingDesignsBought.insert(pair<quint32, quint16>(designId, 1));
	m_StatsLock.unlock();
}

void AuPlayerStats::ShipDesignBought(quint32 designId)
{
	m_StatsLock.lock();
	AuPlayerStatsShipDesigns::iterator it = m_ShipDesignsBought.find(designId);
	if(it != m_ShipDesignsBought.end())
		(*it).second++;
	else
		m_ShipDesignsBought.insert(pair<quint32, quint16>(designId, 1));
	m_StatsLock.unlock();
}

void AuPlayerStats::BuildingDesignBurned(const QString& industryName)
{
	m_StatsLock.lock();
	AuPlayerStatsIndustryDesigns::iterator it = m_BuildingDesignsBurned.find(industryName);
	if(it != m_BuildingDesignsBurned.end())
		(*it).second++;
	else
		m_BuildingDesignsBurned.insert(pair<QString, quint16>(industryName, 1));
	m_StatsLock.unlock();
}

void AuPlayerStats::ChildBorn()
{
	m_StatsLock.lock();
	m_ChildrenBorn++;
	m_StatsLock.unlock();
}

void AuPlayerStats::ChildCarried()
{
	m_StatsLock.lock();
	m_ChildrenCarried++;
	m_StatsLock.unlock();
}

void AuPlayerStats::ChildFathered()
{
	m_StatsLock.lock();
	m_ChildrenFathered++;
	m_StatsLock.unlock();
}

void AuPlayerStats::ShipDesignBurned(SPACECRAFT_STRUCTURE structure)
{
	m_StatsLock.lock();
	switch(structure)
	{
	case SC_Starship:
		m_BurnedStarship++;
		break;
	case SC_Spaceship:
		m_BurnedSpaceship++;
		break;
	case SC_Station:
		m_BurnedStation++;
		break;
	case SC_Hull:
	case SC_Count:
	case SC_Deprecated:
		break;
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::BuildingDesignBuilt(quint32 designId)
{
	m_StatsLock.lock();
	AuPlayerStatsBuildingDesigns::iterator it = m_BuildingDesignsMade.find(designId);
	if(it != m_BuildingDesignsMade.end())
		(*it).second++;
	else
		m_BuildingDesignsMade.insert(pair<quint32, quint16>(designId, 1));
	m_StatsLock.unlock();
}

void AuPlayerStats::ShipDesignManufactured(quint32 designId)
{
	m_StatsLock.lock();
	AuPlayerStatsShipDesigns::iterator it = m_ShipDesignsMade.find(designId);
	if(it != m_ShipDesignsMade.end())
		(*it).second++;
	else
		m_ShipDesignsMade.insert(pair<quint32, quint16>(designId, 1));
	m_StatsLock.unlock();
}

void AuPlayerStats::BuildingDesignPublished(const QString& industryName)
{
	m_StatsLock.lock();
	AuPlayerStatsIndustryDesigns::iterator it = m_BuildingDesignsPublished.find(industryName);
	if(it != m_BuildingDesignsPublished.end())
		(*it).second++;
	else
		m_BuildingDesignsPublished.insert(pair<QString, quint16>(industryName, 1));
	m_StatsLock.unlock();
}

void AuPlayerStats::ShipDesignPublished(SPACECRAFT_STRUCTURE structure)
{
	m_StatsLock.lock();
	switch(structure)
	{
	case SC_Starship:
		m_PublishedStarship++;
		break;
	case SC_Spaceship:
		m_PublishedSpaceship++;
		break;
	case SC_Station:
		m_PublishedStation++;
		break;
	case SC_Hull:
	case SC_Count:
	case SC_Deprecated:
		break;
	}
	m_StatsLock.unlock();
}

void AuPlayerStats::BuildingDesignSold(quint32 designId)
{
	m_StatsLock.lock();
	AuPlayerStatsBuildingDesigns::iterator it = m_BuildingDesignsSold.find(designId);
	if(it != m_BuildingDesignsSold.end())
		(*it).second++;
	else
		m_BuildingDesignsSold.insert(pair<quint32, quint16>(designId, 1));
	m_StatsLock.unlock();
}

void AuPlayerStats::ShipDesignSold(quint32 designId)
{
	m_StatsLock.lock();
	AuPlayerStatsShipDesigns::iterator it = m_ShipDesignsSold.find(designId);
	if(it != m_ShipDesignsSold.end())
		(*it).second++;
	else
		m_ShipDesignsSold.insert(pair<quint32, quint16>(designId, 1));
	m_StatsLock.unlock();
}

void AuPlayerStats::OnLineMinute()
{
	m_StatsLock.lock();
	m_OnLineMinutes++;
	m_StatsLock.unlock();
}

quint32 AuPlayerStats::OnLineMinutes() const
{
	m_StatsLock.lock();
	quint32 v = m_OnLineMinutes;
	m_StatsLock.unlock();
	return v;
}

void AuPlayerStats::OrderMission()
{
	m_StatsLock.lock();
	m_OrdersMission++;
	m_StatsLock.unlock();
}

void AuPlayerStats::OrderSquad()
{
	m_StatsLock.lock();
	m_OrdersSquad++;
	m_StatsLock.unlock();
}

void AuPlayerStats::SpawnRecallCanceled()
{
	m_StatsLock.lock();
	if(m_SpawnRecalls)
		m_SpawnRecalls--;
	m_StatsLock.unlock();
}

void AuPlayerStats::SpawnRecalled()
{
	m_StatsLock.lock();
	m_SpawnRecalls++;
	m_StatsLock.unlock();
}

void AuPlayerStats::NamedSystem()
{
	m_StatsLock.lock();
	m_NamedSolarSystems++;
	m_StatsLock.unlock();
}

void AuPlayerStats::NamedSector()
{
	m_StatsLock.lock();
	m_NamedSectors++;
	m_StatsLock.unlock();
}

void AuPlayerStats::CapturedBase()
{
	m_StatsLock.lock();
	m_FlagsCapturedBase++;
	m_StatsLock.unlock();
}

void AuPlayerStats::CapturedEmpireCapital()
{
	m_StatsLock.lock();
	m_FlagsCapturedCapitalEmpire++;
	m_StatsLock.unlock();
}

void AuPlayerStats::CapturedGalaxyCapital()
{
	m_StatsLock.lock();
	m_FlagsCapturedCapitalGalaxy++;
	m_StatsLock.unlock();
}

void AuPlayerStats::CapturedSectorCapital()
{
	m_StatsLock.lock();
	m_FlagsCapturedCapitalSector++;
	m_StatsLock.unlock();
}

void AuPlayerStats::CapturedCity()
{
	m_StatsLock.lock();
	m_FlagsCapturedCity++;
	m_StatsLock.unlock();
}

void AuPlayerStats::StoryCompleted(quint32 scriptId)
{
	m_StatsLock.lock();
	m_StoriesCompleted++;
	m_StoriesFinished.insert(scriptId);
	m_StatsLock.unlock();
}

void AuPlayerStats::StoryStarted()
{
	m_StatsLock.lock();
	m_StoriesStarted++;
	m_StatsLock.unlock();
}

bool AuPlayerStats::HadItem(COMMODITYID cid) const
{
	m_StatsLock.lock();
	bool v = m_Items.Had(cid);
	m_StatsLock.unlock();
	return v;
}

bool AuPlayerStats::HasConstructedBuildingHabitable(INDUSTRYID iid) const
{
	bool v = false;
	m_StatsLock.lock();
	AuPlayerStatsBuildings::const_iterator it = m_BuildingsHabitable.find(iid);
	if(it != m_BuildingsHabitable.cend() && (*it).second.m_Built)
		v = true;
	m_StatsLock.unlock();
	return v;
}

bool AuPlayerStats::HasConstructedBuildingHarsh(INDUSTRYID iid) const
{
	bool v = false;
	m_StatsLock.lock();
	AuPlayerStatsBuildings::const_iterator it = m_BuildingsHarsh.find(iid);
	if(it != m_BuildingsHarsh.cend() && (*it).second.m_Built)
		v = true;
	m_StatsLock.unlock();
	return v;
}

bool AuPlayerStats::FinishedStory(quint32 scriptId) const
{
	m_StatsLock.lock();
	bool v = m_StoriesFinished.contains(scriptId);
	m_StatsLock.unlock();
	return v;
}

void AuPlayerStats::ForgetStory(quint32 scriptId)
{
	m_StatsLock.lock();
	m_StoriesFinished.remove(scriptId);
	m_StatsLock.unlock();
}

void AuPlayerStats::StoriesFinished(QList<quint32>& scriptIds) const
{
	QMap<quint32, bool> finished;	// sorted
	m_StatsLock.lock();
	for(QSet<quint32>::const_iterator si = m_StoriesFinished.cbegin(); si != m_StoriesFinished.cend(); si++)
		finished.insert(*si, true);
	m_StatsLock.unlock();
	for(QMap<quint32, bool>::const_iterator fi = finished.cbegin(); fi != finished.cend(); fi++)
		scriptIds.append(fi.key());
}

quint64 AuPlayerStats::MemoryUsage() const
{
	quint64 mem = sizeof(AuPlayerStats);
	mem += m_CreaturesTame.MemoryUsage();
	mem += m_CreaturesWild.MemoryUsage();
	mem += (sizeof(quint32) + sizeof(quint16)) * m_BuildingDesignsBought.size();
	mem += m_BuildingDesignsBurned.MemoryUsage();
	mem += (sizeof(quint32) + sizeof(quint16)) * m_BuildingDesignsMade.size();
	mem += m_BuildingDesignsPublished.MemoryUsage();
	mem += (sizeof(quint32) + sizeof(quint16)) * m_BuildingDesignsSold.size();
	mem += (sizeof(INDUSTRYID) + sizeof(quint16)) * m_BuildingsDestroyedFriend.size();
	mem += (sizeof(INDUSTRYID) + sizeof(quint16)) * m_BuildingsDestroyedNeutral.size();
	mem += (sizeof(INDUSTRYID) + sizeof(quint16)) * m_BuildingsDestroyedEnemy.size();
	mem += (sizeof(INDUSTRYID) + sizeof(quint16)) * m_BuildingsDestroyedEmpire.size();
	mem += (sizeof(INDUSTRYID) + sizeof(quint16)) * m_BuildingsDestroyedAlly.size();
	mem += (sizeof(INDUSTRYID) + sizeof(quint16)) * m_BuildingsDestroyedVassal.size();
	mem += (sizeof(INDUSTRYID) + sizeof(AuPlayerStatsBuilding)) * m_BuildingsHabitable.size();
	mem += (sizeof(INDUSTRYID) + sizeof(AuPlayerStatsBuilding)) * m_BuildingsHarsh.size();
	mem += (sizeof(WEAPONEFFECT) + sizeof(quint16)) * m_Deaths.size();
	mem += (sizeof(COMMODITYID) + sizeof(AuPlayerStatsItem)) * m_Items.size();
	mem += (sizeof(KILL_STAT_TYPE) + sizeof(AuPlayerStatsKill)) * m_Kills.size();
	mem += (sizeof(PLANET_STAT_TYPE) + sizeof(AuPlayerStatsPlanet)) * m_Planets.size();
	mem += (sizeof(quint32) + sizeof(quint16)) * m_ShipDesignsBought.size();
	mem += (sizeof(quint32) + sizeof(quint16)) * m_ShipDesignsMade.size();
	mem += (sizeof(quint32) + sizeof(quint16)) * m_ShipDesignsSold.size();
	mem += (sizeof(CRAFT_STAT_TYPE) + sizeof(AuPlayerStatsCraft)) * m_ShipsAndVehicles.size();
	mem += (sizeof(STAR_STAT_TYPE) + sizeof(AuPlayerStatsStar)) * m_Stars.size();
	mem += (sizeof(STATION_STAT_TYPE) + sizeof(AuPlayerStatsStation)) * m_Stations.size();
	mem += sizeof(quint32) * m_StoriesFinished.capacity();
	mem += (sizeof(SYSTEM) + sizeof(AuPlayerStatsSystem)) * m_Systems.size();
	return mem;
}

void AuPlayerStats::BountyCollected(double amount)
{
	m_StatsLock.lock();
	m_BountyCollected += amount;
	m_BountyCollections++;
	m_StatsLock.unlock();
}

void AuPlayerStats::BountyGranted(double amount)
{
	m_StatsLock.lock();
	m_BountyGranted += amount;
	m_BountyGrants++;
	m_StatsLock.unlock();
}

void AuPlayerStats::ClaimedHabitableDifferentDNA()
{
	m_StatsLock.lock();
	m_ClaimedDifferentDNA++;
	m_StatsLock.unlock();
}

void AuPlayerStats::ClaimedHabitableSameDNA()
{
	m_StatsLock.lock();
	m_ClaimedSameDNA++;
	m_StatsLock.unlock();
}

void AuPlayerStats::ClaimedUninhabitable()
{
	m_StatsLock.lock();
	m_ClaimedUninhabitable++;
	m_StatsLock.unlock();
}

void AuPlayerStats::ShipmentMineDelivered()
{
	m_StatsLock.lock();
	m_ShipmentsMineDelivered ++;
	m_StatsLock.unlock();
}

void AuPlayerStats::ShipmentDelivered()
{
	m_StatsLock.lock();
	m_ShipmentsDelivered ++;
	m_StatsLock.unlock();
}

quint32 AuPlayerStats::ShipmentsMineDelivered() const
{
	return m_ShipmentsMineDelivered;
}

quint32 AuPlayerStats::ShipmentsDelivered() const
{
	return m_ShipmentsDelivered;
}

void AuPlayerStats::ShipmentMineOpened()
{
	m_StatsLock.lock();
	m_ShipmentsMineOpened ++;
	m_StatsLock.unlock();
}

void AuPlayerStats::ShipmentOpenedByMe()
{
	m_StatsLock.lock();
	m_ShipmentsOpenedByMe ++;
	m_StatsLock.unlock();
}

void AuPlayerStats::ShipmentOpenedByOther()
{
	m_StatsLock.lock();
	m_ShipmentsOpenedByOther ++;
	m_StatsLock.unlock();
}

quint32 AuPlayerStats::ShipmentsMineOpened() const
{
	return m_ShipmentsMineOpened;
}

quint32 AuPlayerStats::ShipmentsOpenedByMe() const
{
	return m_ShipmentsOpenedByMe;
}

quint32 AuPlayerStats::ShipmentsOpenedByOther() const
{
	return m_ShipmentsOpenedByOther;
}

void AuPlayerStats::ShipmentMineJettisoned()
{
	m_StatsLock.lock();
	m_ShipmentsMineJettisoned ++;
	m_StatsLock.unlock();
}

void AuPlayerStats::ShipmentJettisonedByMe()
{
	m_StatsLock.lock();
	m_ShipmentsJettisonedByMe ++;
	m_StatsLock.unlock();
}

void AuPlayerStats::ShipmentJettisonedByOther()
{
	m_StatsLock.lock();
	m_ShipmentsJettisonedByOther ++;
	m_StatsLock.unlock();
}

quint32 AuPlayerStats::ShipmentsMineJettisoned() const
{
	return m_ShipmentsMineJettisoned;
}

quint32 AuPlayerStats::ShipmentsJettisonedByMe() const
{
	return m_ShipmentsJettisonedByMe;
}

quint32 AuPlayerStats::ShipmentsJettisonedByOther() const
{
	return m_ShipmentsJettisonedByOther;
}

void AuPlayerStats::ShipmentLoaded()
{
	m_StatsLock.lock();
	m_ShipmentsLoaded ++;
	m_StatsLock.unlock();
}

quint32 AuPlayerStats::ShipmentsLoaded() const
{
	return m_ShipmentsLoaded;
}

quint32 AuPlayerStats::ExploreScore() const
{
	m_StatsLock.lock();
	quint32 xp = m_ClaimedDifferentDNA +
		m_ClaimedSameDNA +
		m_ClaimedUninhabitable +
		m_NamedSolarSystems +
		m_NamedSectors +
		m_WormholesExplored +
		m_WormholesNamed;
	m_StatsLock.unlock();
	return xp;
}

quint32 AuPlayerStats::SpacecraftCaptureScore() const
{
	m_StatsLock.lock();
	quint32 xp = 0;
	for(AuPlayerStatsCrafts::const_iterator it = m_ShipsAndVehicles.cbegin(); it != m_ShipsAndVehicles.cend(); it++)
	{
		switch((*it).first)
		{
		case CST_Station:
		case CST_Spaceship:
		case CST_Starship:
			xp += (*it).second.m_Captures;
			break;
		case CST_Invalid:
		case CST_MotorBike:
		case CST_Rocket:
		case CST_SailBoat:
		case CST_SpaceFighterF1:
		case CST_SpaceFighterF2:
		case CST_Submarine:
		case CST_SUV:
		case CST_APC:
		case CST_FarmMachinery:
		case CST_Helicopter:
		case CST_SpaceTransport:
			break;
		}
	}
	m_StatsLock.unlock();
	return xp;
}

quint32 AuPlayerStats::VehiclesCaptured() const
{
	m_StatsLock.lock();
	quint32 xp = 0;
	for(AuPlayerStatsCrafts::const_iterator it = m_ShipsAndVehicles.cbegin(); it != m_ShipsAndVehicles.cend(); it++)
	{
		switch((*it).first)
		{
		case CST_APC:
		case CST_FarmMachinery:
		case CST_Helicopter:
		case CST_MotorBike:
		case CST_Rocket:
		case CST_SUV:
		case CST_SailBoat:
		case CST_SpaceFighterF1:
		case CST_SpaceFighterF2:
		case CST_SpaceTransport:
		case CST_Submarine:
			xp += (*it).second.m_Captures;
			break;
		case CST_Invalid:
		case CST_Spaceship:
		case CST_Starship:
		case CST_Station:
			break;
		}
	}
	m_StatsLock.unlock();
	return xp;
}

quint32 AuPlayerStats::VehiclesUsedAsWeapon() const
{
	COMMODITYID vehicles[] =
	{
		CID_APC,
		CID_AdvSpaceFighter,
		CID_DirtBike,
		CID_FarmMachinery,
		CID_Helicopter,
		CID_SUV,
		CID_SailBoat,
		CID_SpaceFighter,
		CID_SpaceRocket,
		CID_SpaceTransport,
		CID_Submarine,
	};
	int vehicleCount = sizeof(vehicles) / sizeof(COMMODITYID);

	quint32 used = 0;
	for(int v=0; v<vehicleCount; v++)
	{
		m_StatsLock.lock();
		AuPlayerStatsItems::const_iterator it = m_Items.find(vehicles[v]);
		if(it != m_Items.cend())
			used += (*it).second.m_UsesAsWeapon;
		m_StatsLock.unlock();
	}
	return used;
}

quint32 AuPlayerStats::BuildingsPlanned() const
{
	quint32 xp = 0;
	m_StatsLock.lock();
	for(AuPlayerStatsBuildings::const_iterator it = m_BuildingsHabitable.cbegin(); it != m_BuildingsHabitable.cend(); it++)
		xp += (*it).second.m_Built;
	for(AuPlayerStatsBuildings::const_iterator it = m_BuildingsHarsh.cbegin(); it != m_BuildingsHarsh.cend(); it++)
		xp += (*it).second.m_Built;
	m_StatsLock.unlock();
	return xp;
}

quint32 AuPlayerStats::ArchitectScore() const
{
	quint32 score = 0;
	score += m_BuildingDesignsBurned.Total();
	score += m_BuildingDesignsPublished.Total();
	return score;
}

quint16 AuPlayerStats::MissionOrders() const
{
	quint16 ord = 0;
	m_StatsLock.lock();
	ord = m_OrdersMission;
	m_StatsLock.unlock();
	return ord;
}

quint16 AuPlayerStats::SquadOrders() const
{
	quint16 ord = 0;
	m_StatsLock.lock();
	ord = m_OrdersSquad;
	m_StatsLock.unlock();
	return ord;
}
