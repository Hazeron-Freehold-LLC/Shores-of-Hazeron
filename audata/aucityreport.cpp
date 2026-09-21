//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucityreport.cpp,v 1.164 2022/05/04 20:33:50 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucityreport.h"
//
#include "auitem.h"
#include "aubddesign.h"
#include "dgdata.h"
#include "audna.h"
#include "dhdna.h"

#define d_AbandonDaysPerMoralePoint		(1)						// Days after d_AbandonDaysBegin per point of morale loss.

static int s_MT_AuCityIndustry = qRegisterMetaType<AuCityIndustry>();
static int s_MT_AuCityIndustryHash = qRegisterMetaType<AuCityIndustryHash>();
static int s_MT_AuCityReport = qRegisterMetaType<AuCityReport>();
static int s_MT_AuCityReportHistory = qRegisterMetaType<AuCityReportHistory>();
static int s_HomeApartment = 0;
static int s_HomeCondo = 0;
static int s_HomeFarm = 0;
static int s_HomeHouse = 0;
static int s_HomeOrchard = 0;
static float s_BuildingVolumePerPerson = ((float)AuBdDesign::MaxVolume() / 721210);	// building volume allotted per person on Earth

AuCityCitizen::AuCityCitizen(const AuSpecies& species, ECOROLE ecoRole, bool isAlien, quint32 population)
{
	m_EcoRole = ecoRole;
	m_IsAlien = isAlien;
	m_Population = population;
	m_SpeciesDesc = species.Description();
	m_SpeciesName = species.Name();
}

void AuCityCitizen::operator>>(QDataStream& out) const
{
	quint8 hasWhat = m_EcoRole;	// bottom nibble
	if(m_IsAlien)
		hasWhat |= 0x80;
	out << hasWhat;
	out << m_Population;
	out << m_SpeciesName;
	out << m_SpeciesDesc;
}

void AuCityCitizen::operator<<(QDataStream& in)
{
	quint8 hasWhat;
	in >> hasWhat;
	m_EcoRole = (ECOROLE)(hasWhat & 0x0f);	// bottom nibble
	m_IsAlien = hasWhat & 0x80;
	in >> m_Population;
	in >> m_SpeciesName;
	in >> m_SpeciesDesc;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuCityIndustry::operator>>(QDataStream& out) const
{
	quint32 lver = m_Levels;
	lver |= 0xc0000000;

	out << (quint8)0;	// Used to be highest Q.
	out << lver;
	out << m_LevelsInUse;
}

void AuCityIndustry::operator<<(QDataStream& in)
{
	quint32 lver;
	quint8 ui8;
	in >> ui8;			// Used to be highest Q.
	in >> lver;
	m_Levels = lver & 0x3fffffff;
	if(lver & 0x80000000)
		in >> m_LevelsInUse;
	else
		m_LevelsInUse = m_Levels;
	if(!(lver & 0x40000000))
		in >> ui8;		// Used to be lowest Q.
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

quint64 AuCityIndustryHash::MemoryUsage() const
{
	quint64 mem = sizeof(AuCityIndustryHash);
	for(const_iterator ii = cbegin(); ii != cend(); ii++)
		mem += sizeof(INDUSTRYID) + sizeof(AuCityIndustry);
	return mem;
}

quint32 AuCityIndustryHash::TotalLevels() const
{
	quint32 t = 0;
	for(const_iterator ii = cbegin(); ii != cend(); ii++)
		t += (*ii).Levels();
	return t;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

quint64 AuCityReportShip::MemoryUsage() const
{
	quint64 mem = sizeof(AuCityReportShip);
	mem += StringMemoryUsage(m_SpotDesc);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuFlag::operator<<(QDataStream& in)
{
	QDateTime tm;
	in >> tm;
	tm.setTimeSpec(Qt::UTC);
	m_Capture = tm;
	in >> m_DevLL;
	in >> m_Empire;
	in >> m_Level;
	in >> m_Terrain;
}

void AuFlag::operator>>(QDataStream& out) const
{
	out << m_Capture.UTC();
	out << m_DevLL;
	out << m_Empire;
	out << m_Level;
	out << m_Terrain;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuFlagArray::operator<<(QDataStream& in)
{
	clear();
	quint8 count;
	in >> count;
	AuFlag flag;
	for(int flagIndex=0;flagIndex<count;flagIndex++)
	{
		flag << in;

		// The IID is not stored. The first flag is always the town square. The rest are base flags.
		if(flagIndex == 0)
			flag.m_IID = IID_TownSquare;
		else
			flag.m_IID = IID_BaseFlag;

		append(flag);
	}
}

void AuFlagArray::operator>>(QDataStream& out) const
{
	out << (quint8)count();
	for(int flag=0; flag<count(); flag++)
		at(flag) >> out;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCityReport::AuCityReport()
{
	m_AirConsumed = 0;
	m_AirSupply = 0;
	m_AteEnoughFood = false;
	m_CityType = CITY_Capitol;
	m_ConsumedLiquor = false;
	m_Decayed[0] = IID_None;
	m_Decayed[1] = IID_None;
	m_Decayed[2] = IID_None;
	m_EventLog = new AuEventLog;
	m_FoodPerMeal = 0;
	m_FoodTypes = 0;
	m_FoodUnits = 0;
	m_FounderStatus = FOUNDER_Unknown;
	m_Galaxy = ShoresOfHazeron;
	m_HasBreathableAtmo = false;
	m_HasDestroyedBuildings = 0;
	m_HomesLg = 0;
	m_HomesMd = 0;
	m_HomesSm = 0;
	m_Inventory = new AuItemHash;
	m_IsCapitalOfEmpire = false;
	m_IsCapitalOfGalaxy = false;
	m_IsCapitalOfSector = false;
	m_IsHabitablePlanet = false;
	m_IsIsolated = false;
	m_IsOnRingworld = false;
	m_Jobs = 0;
	m_JobsNonConst = 0;
	m_JobsPro = 0;
	m_JobsRepair = 0;
	m_LastAvatarAccountActive = 0;
	m_LastTribute = 0;
	m_LastVisit = 0;
	m_Lounges = 0;
	m_LoyaltyOrig = 0;
	m_MaxPop = 0;
	m_MessageType = AuMailMsg::MSG_CityStatusReport;
	m_PopBirths = 0;
	m_PopDeaths = 0;
	m_PopIndigenousArr = 0;
	m_PopIndigenousDep = 0;
	m_PopOrig = 0;
	m_PopSeafarersArr = 0;
	m_PopSeafarersDep = 0;
	m_PopTravellersArr = 0;
	m_PopTravellersDep = 0;
	m_PopWanderersArr = 0;
	m_PopWanderersDep = 0;
	m_Power = 0;
	m_SatOrig = 0;
	m_StoresGrocery = 0;
	m_StoresRetail = 0;
	m_Tools = new AuItemHash;
	m_TroopsAtCity = 0;
	m_TroopsDrafted = 0;
	m_Unemployed = 0;
	m_WorldBuildingCount = 0;
	m_WorldBuildingVol = 0;
	m_Zone = 0;
	m_ZonePop = 0;
	m_iCapacitor = 0;
	m_iStorage = 0;
}

AuCityReport::~AuCityReport()
{
	delete m_EventLog;
	delete m_Inventory;
	delete m_Tools;
}

AuCityReport::AuCityReport(const AuCityReport& c) : m_Bank(c.m_Bank), m_Industries(c.m_Industries)
{
	m_AirConsumed = c.m_AirConsumed;
	m_AirSupply = c.m_AirSupply;
	m_AteEnoughFood = c.m_AteEnoughFood;
	m_CaptName = c.m_CaptName;
	m_Citizens = c.m_Citizens;
	m_CityType = c.m_CityType;
	m_ConsumedLiquor = c.m_ConsumedLiquor;
	m_Decayed[0] = c.m_Decayed[0];
	m_Decayed[1] = c.m_Decayed[1];
	m_Decayed[2] = c.m_Decayed[2];
	m_DistressRpt = c.m_DistressRpt;
	m_EventLog = new AuEventLog(*c.m_EventLog);
	m_Flags = c.m_Flags;
	m_FoodPerMeal = c.m_FoodPerMeal;
	m_FoodTypes = c.m_FoodTypes;
	m_FoodUnits = c.m_FoodUnits;
	m_FounderId = c.m_FounderId;
	m_FounderName = c.m_FounderName;
	m_FounderStatus = c.m_FounderStatus;
	m_Galaxy = c.m_Galaxy;
	m_HasBreathableAtmo = c.m_HasBreathableAtmo;
	m_HasDestroyedBuildings = c.m_HasDestroyedBuildings;
	m_HomesLg = c.m_HomesLg;
	m_HomesMd = c.m_HomesMd;
	m_HomesSm = c.m_HomesSm;
	m_Inventory = new AuItemHash(*c.m_Inventory);
	m_IsCapitalOfEmpire = c.m_IsCapitalOfEmpire;
	m_IsCapitalOfGalaxy = c.m_IsCapitalOfGalaxy;
	m_IsCapitalOfSector = c.m_IsCapitalOfSector;
	m_IsHabitablePlanet = c.m_IsHabitablePlanet;
	m_IsIsolated = c.m_IsIsolated;
	m_IsOnRingworld = c.m_IsOnRingworld;
	m_Jobs = c.m_Jobs;
	m_JobsNonConst = c.m_JobsNonConst;
	m_JobsPro = c.m_JobsPro;
	m_JobsRepair = c.m_JobsRepair;
	m_LL = c.m_LL;
	m_LastAvatarAccountActive = c.m_LastAvatarAccountActive;
	m_LastTribute = c.m_LastTribute;
	m_LastVisit = c.m_LastVisit;
	m_Lounges = c.m_Lounges;
	m_LoyaltyOrig = c.m_LoyaltyOrig;
	m_MaxPop = c.m_MaxPop;
	m_MessageType = c.m_MessageType;
	m_Name = c.m_Name;
	m_NativeSpecies = c.m_NativeSpecies;
	m_NativeSpeciesDesc = c.m_NativeSpeciesDesc;
	m_PatentRnD = c.m_PatentRnD;
	m_Patents = c.m_Patents;
	m_PlanetDesc = c.m_PlanetDesc;
	m_PlanetName = c.m_PlanetName;
	m_PopBirths = c.m_PopBirths;
	m_PopDeaths = c.m_PopDeaths;
	m_PopIndigenousArr = c.m_PopIndigenousArr;
	m_PopIndigenousDep = c.m_PopIndigenousDep;
	m_PopOrig = c.m_PopOrig;
	m_PopSeafarersArr = c.m_PopSeafarersArr;
	m_PopSeafarersDep = c.m_PopSeafarersDep;
	m_PopTravellersArr = c.m_PopTravellersArr;
	m_PopTravellersDep = c.m_PopTravellersDep;
	m_PopWanderersArr = c.m_PopWanderersArr;
	m_PopWanderersDep = c.m_PopWanderersDep;
	m_Power = c.m_Power;
	m_SatOrig = c.m_SatOrig;
	m_SectorDesc = c.m_SectorDesc;
	m_SectorName = c.m_SectorName;
	m_Ships = c.m_Ships;
	m_StoresGrocery = c.m_StoresGrocery;
	m_StoresRetail = c.m_StoresRetail;
	m_SystemDesc = c.m_SystemDesc;
	m_SystemName = c.m_SystemName;
	m_Tools = new AuItemHash(*c.m_Tools);
	m_TroopsAtCity = c.m_TroopsAtCity;
	m_TroopsDrafted = c.m_TroopsDrafted;
	m_Unemployed = c.m_Unemployed;
	m_Vehicles = c.m_Vehicles;
	m_WhenCompiled = c.m_WhenCompiled;
	m_WorldBuildingCount = c.m_WorldBuildingCount;
	m_WorldBuildingVol = c.m_WorldBuildingVol;
	m_Zone = c.m_Zone;
	m_ZonePop = c.m_ZonePop;
	m_iCapacitor = c.m_iCapacitor;
	m_iStorage = c.m_iStorage;
}

const AuCityReport& AuCityReport::operator=(const AuCityReport& c)
{
	m_AirConsumed = c.m_AirConsumed;
	m_AirSupply = c.m_AirSupply;
	m_AteEnoughFood = c.m_AteEnoughFood;
	m_Bank = c.m_Bank;
	m_CaptName = c.m_CaptName;
	m_Citizens = c.m_Citizens;
	m_CityType = c.m_CityType;
	m_ConsumedLiquor = c.m_ConsumedLiquor;
	m_Decayed[0] = c.m_Decayed[0];
	m_Decayed[1] = c.m_Decayed[1];
	m_Decayed[2] = c.m_Decayed[2];
	m_DistressRpt = c.m_DistressRpt;
	*m_EventLog = *c.m_EventLog;
	m_Flags = c.m_Flags;
	m_FoodPerMeal = c.m_FoodPerMeal;
	m_FoodTypes = c.m_FoodTypes;
	m_FoodUnits = c.m_FoodUnits;
	m_FounderId = c.m_FounderId;
	m_FounderName = c.m_FounderName;
	m_FounderStatus = c.m_FounderStatus;
	m_Galaxy = c.m_Galaxy;
	m_HasBreathableAtmo = c.m_HasBreathableAtmo;
	m_HasDestroyedBuildings = c.m_HasDestroyedBuildings;
	m_HomesLg = c.m_HomesLg;
	m_HomesMd = c.m_HomesMd;
	m_HomesSm = c.m_HomesSm;
	m_Industries = c.m_Industries;
	*m_Inventory = *c.m_Inventory;
	m_IsCapitalOfEmpire = c.m_IsCapitalOfEmpire;
	m_IsCapitalOfGalaxy = c.m_IsCapitalOfGalaxy;
	m_IsCapitalOfSector = c.m_IsCapitalOfSector;
	m_IsHabitablePlanet = c.m_IsHabitablePlanet;
	m_IsIsolated = c.m_IsIsolated;
	m_IsOnRingworld = c.m_IsOnRingworld;
	m_Jobs = c.m_Jobs;
	m_JobsNonConst = c.m_JobsNonConst;
	m_JobsPro = c.m_JobsPro;
	m_JobsRepair = c.m_JobsRepair;
	m_LL = c.m_LL;
	m_LastAvatarAccountActive = c.m_LastAvatarAccountActive;
	m_LastTribute = c.m_LastTribute;
	m_LastVisit = c.m_LastVisit;
	m_Lounges = c.m_Lounges;
	m_LoyaltyOrig = c.m_LoyaltyOrig;
	m_MaxPop = c.m_MaxPop;
	m_MessageType = c.m_MessageType;
	m_Name = c.m_Name;
	m_NativeSpecies = c.m_NativeSpecies;
	m_NativeSpeciesDesc = c.m_NativeSpeciesDesc;
	m_PatentRnD = c.m_PatentRnD;
	m_Patents = c.m_Patents;
	m_PlanetDesc = c.m_PlanetDesc;
	m_PlanetName = c.m_PlanetName;
	m_PopBirths = c.m_PopBirths;
	m_PopDeaths = c.m_PopDeaths;
	m_PopIndigenousArr = c.m_PopIndigenousArr;
	m_PopIndigenousDep = c.m_PopIndigenousDep;
	m_PopOrig = c.m_PopOrig;
	m_PopSeafarersArr = c.m_PopSeafarersArr;
	m_PopSeafarersDep = c.m_PopSeafarersDep;
	m_PopTravellersArr = c.m_PopTravellersArr;
	m_PopTravellersDep = c.m_PopTravellersDep;
	m_PopWanderersArr = c.m_PopWanderersArr;
	m_PopWanderersDep = c.m_PopWanderersDep;
	m_Power = c.m_Power;
	m_SatOrig = c.m_SatOrig;
	m_SectorDesc = c.m_SectorDesc;
	m_SectorName = c.m_SectorName;
	m_Ships = c.m_Ships;
	m_StoresGrocery = c.m_StoresGrocery;
	m_StoresRetail = c.m_StoresRetail;
	m_SystemDesc = c.m_SystemDesc;
	m_SystemName = c.m_SystemName;
	*m_Tools = *c.m_Tools;
	m_TroopsAtCity = c.m_TroopsAtCity;
	m_TroopsDrafted = c.m_TroopsDrafted;
	m_Unemployed = c.m_Unemployed;
	m_Vehicles = c.m_Vehicles;
	m_WhenCompiled = c.m_WhenCompiled;
	m_WorldBuildingCount = c.m_WorldBuildingCount;
	m_WorldBuildingVol = c.m_WorldBuildingVol;
	m_Zone = c.m_Zone;
	m_ZonePop = c.m_ZonePop;
	m_iCapacitor = c.m_iCapacitor;
	m_iStorage = c.m_iStorage;
	return *this;
}

void AuCityReport::operator>>(QDataStream& out) const
{
	quint16 bits = 0x8000;			// Top bit says we have a version number. No top bit is version 0.
	if(m_AteEnoughFood)
		bits |= 0x0001;
	if(m_ConsumedLiquor)
		bits |= 0x0002;
	if(m_HasDestroyedBuildings & 0x01)
		bits |= 0x0004;
	if(m_IsCapitalOfEmpire)
		bits |= 0x0008;
	if(m_IsCapitalOfSector)
		bits |= 0x0010;
	if(m_IsHabitablePlanet)
		bits |= 0x0020;
	if(m_IsIsolated)
		bits |= 0x0040;
	if(m_HasBreathableAtmo)
		bits |= 0x0080;
	if(m_IsOnRingworld)
		bits |= 0x0100;
	if(!m_Patents.empty())
		bits |= 0x0200;
	if(!m_FounderId.IsNull())
		bits |= 0x0400;
	if(m_HasDestroyedBuildings & 0x02)
		bits |= 0x0800;
	bits |= m_CityType << (16-4);		// City type stored in b13-b12.
	if(m_IsCapitalOfGalaxy)
		bits |= 0x4000;
	out << bits;

	const quint8 version = 15;
	out << version;

	out << m_AirConsumed;
	out << m_Bank;						// version 12
	out << m_CaptName;
	out << m_Citizens;					// version 13 data structure changed to AuSpecies vs DhDNA
	out << (quint8)m_Decayed[0];
	out << (quint8)m_Decayed[1];
	out << (quint8)m_Decayed[2];
	out << m_DistressRpt;
	out << *m_EventLog;
	out << m_Flags;
	out << m_Galaxy;
	out << m_HomesLg;
	out << m_HomesMd;
	out << m_HomesSm;
	out << m_iCapacitor;
	out << m_Industries;
	out << *m_Inventory;
	out << m_iStorage;
	out << m_Jobs;
	out << m_JobsNonConst;
	out << m_JobsRepair;
	out << m_LastTribute;
	out << m_LastVisit;
	out << m_LastAvatarAccountActive;
	out << m_LL;
	out << m_LoyaltyOrig;
	out << m_MaxPop;
	out << m_MessageType;
	out << m_Name;
	out << m_NativeSpecies;
	out << m_NativeSpeciesDesc;
	out << m_PlanetName;
	out << m_PlanetDesc;
	out << m_PopOrig;
	out << m_PopBirths;
	out << m_PopIndigenousArr;
	out << m_PopTravellersArr;
	out << (quint32)0;
	out << m_PatentRnD;
	out << m_SatOrig;
	out << m_SectorName;
	out << m_SectorDesc;
	out << m_Ships;
	out << m_SystemName;
	out << m_SystemDesc;
	out << *m_Tools;
	out << m_TroopsAtCity;
	out << m_TroopsDrafted;
	out << m_Unemployed;
	out << (quint8)m_Vehicles.count();
	for(QList<COMMODITYID>::const_iterator vi = m_Vehicles.cbegin(); vi != m_Vehicles.cend(); vi++)
		out << (quint16)*vi;
	out << m_WhenCompiled.UTC();
	out << m_Zone;
	out << m_ZonePop;

	// Added at version 1 to eliminate the need to store the entire inventory.
	out << m_FoodTypes;					// Version 1.
	out << m_FoodUnits;					// Version 1.
	out << m_Power;						// Version 1.

	if(bits & 0x0200)
		out << m_Patents;

	if(bits & 0x0400)
	{
		out << (quint8)m_FounderStatus;	// Version 15
		out << m_FounderId;
		out << m_FounderName;
	}

	out << (quint32)m_PopIndigenousDep;	// was m_Arenas in version 9
	out << (quint32)m_PopTravellersDep;	// was m_Casinos in version 9
	out << (quint32)m_PopDeaths;		// was m_Churches in version 9
	out << m_AirSupply;					// was m_Hospitals in version 9
	out << m_Lounges;
	out << m_FoodPerMeal;				// was m_Parks in version 9
	out << m_WorldBuildingCount;		// was m_Schools in version 9
	out << m_StoresGrocery;
	out << m_StoresRetail;
	out << m_JobsPro;					// was m_Zoos in version 9

	out << m_WorldBuildingVol;

	// Added at version 11.
	out << m_PopSeafarersArr;
	out << m_PopSeafarersDep;
	out << m_PopWanderersArr;
	out << m_PopWanderersDep;
}

void AuCityReport::operator<<(QDataStream& in)
{
	quint16 bits;
	in >> bits;
	m_AteEnoughFood = (bits & 0x0001);
	m_ConsumedLiquor = (bits & 0x0002);
	if(bits & 0x0004)
		m_HasDestroyedBuildings = 0x01;				// heavily damaged
	m_IsCapitalOfEmpire = (bits & 0x0008);
	m_IsCapitalOfGalaxy = (bits & 0x4000);
	m_IsCapitalOfSector = (bits & 0x0010);
	m_IsHabitablePlanet = (bits & 0x0020);
	m_IsIsolated = (bits & 0x0040);
	m_HasBreathableAtmo = (bits & 0x0080);
	m_IsOnRingworld = (bits & 0x0100);
	if(bits & 0x0800)
		m_HasDestroyedBuildings |= 0x02;			// on fire
	m_CityType = (bits >> (16-4)) & 0x03;			// City type stored in b13-b12.

	quint8 version = 0;
	if(bits & 0x8000)
		in >> version;

	in >> m_AirConsumed;
	if(version < 14)
	{
		quint32 allegEmpire, allegLevel;
		in >> allegEmpire;
		in >> allegLevel;
	}
	if(version >= 12)
	{
		in >> m_Bank;
	}
	else
	{
		// Old Balance Sheet
		// m_Bank variables relate to the money that belongs to the people. This is where money that is manufactured is deposited.
		// This is also the money used to buy and sell supplies.
		double								m_BankBalance;			// Current balance of money held by the people.
		double								m_BankExCommission;		// Expense due to architect commissions paid out.
		double								m_BankExConfiscated;	// Expenses due to wealth confiscation by the govt.
		double								m_BankExEmigrate;		// Expenses due to emigration of citizens.
		double								m_BankExLoans;			// Expenses due to loans taken by citizens.
		double								m_BankExPurchases;		// Expenses due to purchase of products.
		double								m_BankExRewards;		// Expenses due to rewards paid out.
		double								m_BankExRnD;			// Expenses due to research and development
		double								m_BankExSalary;			// Expenses due to paying for work.
		double								m_BankExWholesale;		// Expense due to wholesale trade with other cities.
		double								m_BankExWithdrawal;		// Expense due to withdrawal of funds by citizens.
		double								m_BankInImmigrate;		// Income from immigration of citizens personal bank accounts.
		double								m_BankInLoans;			// Income from loan payments by citizens.
		double								m_BankInMfg;			// Income from local money production.
		double								m_BankInRnD;			// Income from RnD returns (i.e. canceled processes).
		double								m_BankInSalary;			// Income from work performed by citizens.
		double								m_BankInSales;			// Income from sales of local products.
		double								m_BankInWholesale;		// Expense due to wholesale trade with other cities.
		// m_bs variables relate to the government account.
		//  - Current balance is not allowed to become negative.
		//  - StartingBalance for a city report period is calculated from the current balance, subtracting the income and adding the
		//    expenses.
		//  - CityReport lists the balance sheet then zeros the income and expenses. This is a period.
		double								m_bsCurrentBalance;		// Balance at the start of a period.
		double								m_bsExRewards;			// Expense due to rewards paid out.
		double								m_bsExRnD;				// Expense due to research and development.
		double								m_bsExTribute;			// Expense due to tribute paid to capital.
		double								m_bsExWithdrawal;		// Expense due to withdrawal by govt official.
		double								m_bsExWorks;			// Expense due to works projects.
		double								m_bsInConfiscated;		// Income from wealth confiscation from the citizens.
		double								m_bsInDeposit;			// Income from deposit by govt official.
		double								m_bsInIncomeTax;		// Income from income tax.
		double								m_bsInRnD;				// Income from RnD, due to canceling an RnD mfg process.
		double								m_bsInSalesTax;			// Income from sales tax.
		double								m_bsInSubsidyTax;		// Income from subsidy tax.
		double								m_bsInTribute;			// Income from tribute from subordinate cities, for capitals.

		in >> m_BankBalance;
		in >> m_BankExCommission;
		in >> m_BankExConfiscated;
		in >> m_BankExEmigrate;
		if(version >= 3)
			in >> m_BankExLoans;
		else
			m_BankExLoans = 0;
		in >> m_BankExPurchases;
		in >> m_BankExRewards;
		if(version >= 6)
			in >> m_BankExRnD;
		else
			m_BankExRnD = 0;
		in >> m_BankExSalary;
		in >> m_BankExWholesale;
		if(version >= 2)
			in >> m_BankExWithdrawal;
		else
			m_BankExWithdrawal = 0;
		in >> m_BankInImmigrate;
		if(version >= 3)
			in >> m_BankInLoans;
		else
			m_BankInLoans = 0;
		in >> m_BankInMfg;
		if(version >= 6)
			in >> m_BankInRnD;
		else
			m_BankInRnD = 0;
		in >> m_BankInSalary;
		in >> m_BankInSales;
		in >> m_BankInWholesale;
		in >> m_bsCurrentBalance;
		in >> m_bsExRewards;
		in >> m_bsExRnD;
		in >> m_bsExTribute;
		in >> m_bsExWithdrawal;
		in >> m_bsExWorks;
		in >> m_bsInConfiscated;
		in >> m_bsInDeposit;
		in >> m_bsInIncomeTax;
		in >> m_bsInRnD;
		in >> m_bsInSalesTax;
		in >> m_bsInSubsidyTax;
		in >> m_bsInTribute;

		// Build our new AuBank object from the data.
		m_Bank = AuBank(m_BankBalance + m_bsCurrentBalance);
		if(m_BankExEmigrate > 0)
			m_Bank.CivDebitEmigration(m_BankExEmigrate);
		if(m_BankExLoans > 0)
			m_Bank.CivDebitLoans(m_BankExLoans);
		if(m_BankExPurchases > 0)
			m_Bank.CivDebitPurchases(m_BankExPurchases);
		if(m_BankExRewards > 0)
			m_Bank.CivDebitRewards(m_BankExRewards);
		if(m_BankExRnD > 0)
			m_Bank.CivDebitRnD(m_BankExRnD);
		if(m_BankExSalary > 0)
			m_Bank.CivDebitSalary(m_BankExSalary);
		if(m_BankExWholesale > 0)
			m_Bank.CivDebitWholesale(m_BankExWholesale);
		if(m_BankExWithdrawal > 0)
			m_Bank.CivDebitWithdrawal(m_BankExWithdrawal);
		if(m_BankInImmigrate > 0)
			m_Bank.CivCreditImmigration(m_BankInImmigrate);
		if(m_BankInLoans > 0)
			m_Bank.CivCreditLoans(m_BankInLoans);
		if(m_BankInMfg > 0)
			m_Bank.CivCreditMint(m_BankInMfg);
		if(m_BankInRnD > 0)
			m_Bank.CivCreditRnD(m_BankInRnD);
		if(m_BankInSalary > 0)
			m_Bank.CivCreditSalary(m_BankInSalary);
		if(m_BankInSales > 0)
			m_Bank.CivCreditSales(m_BankInSales);
		if(m_BankInWholesale > 0)
			m_Bank.CivCreditWholesale(m_BankInWholesale);
		if(m_bsExRewards > 0)
			m_Bank.GovDebitRewards(m_bsExRewards);
		if(m_bsExRnD > 0)
			m_Bank.CivDebitRnD(m_bsExRnD);
		if(m_bsExTribute > 0)
			m_Bank.GovDebitTribute(m_bsExTribute);
		if(m_bsExWithdrawal > 0)
			m_Bank.GovDebitWithdrawal(m_bsExWithdrawal);
		if(m_bsExWorks > 0)
			m_Bank.GovDebitWorks(m_bsExWorks);
		if(m_bsInDeposit > 0)
			m_Bank.GovCreditDeposit(m_bsInDeposit);
		if(m_bsInIncomeTax > 0)
			m_Bank.GovCreditTaxIncome(m_bsInIncomeTax);
		if(m_bsInRnD > 0)
			m_Bank.CivCreditRnD(m_bsInRnD);
		if(m_bsInSalesTax > 0)
			m_Bank.GovCreditTaxSales(m_bsInSalesTax);
		if(m_bsInSubsidyTax > 0)
			m_Bank.GovCreditTaxSubsidy(m_bsInSubsidyTax);
		if(m_bsInTribute > 0)
			m_Bank.GovCreditTribute(m_bsInTribute);
	}
	in >> m_CaptName;

	DhDNA dna;
	QMap<DhDNA,quint32> citizenDNA;			// Population per each citizen DNA.
	if(version >= 13)
		in >> m_Citizens;
	else if(version >= 7)
		in >> citizenDNA;
	else
		in >> dna;

	quint8 ui8;
	in >> ui8;
	m_Decayed[0] = (INDUSTRYID)ui8;
	in >> ui8;
	m_Decayed[1] = (INDUSTRYID)ui8;
	in >> ui8;
	m_Decayed[2] = (INDUSTRYID)ui8;
	in >> m_DistressRpt;
	in >> *m_EventLog;
	in >> m_Flags;
	in >> m_Galaxy;
	if(version >= 7)
	{
		in >> m_HomesLg;
		in >> m_HomesMd;
		in >> m_HomesSm;
	}
	in >> m_iCapacitor;
	if(version >= 5)
		in >> m_Industries;
	else
	{
		// Formerly stored in a fixed size array and stored with each index equal to the INDUSTRYID.
		m_Industries.clear();
		in >> ui8;
		for(int ii=0; ii<ui8; ii++)
		{
			AuCityIndustry ci;
			in >> ci;

			// Store it only if there is any levels.
			if(ci.Levels())
				m_Industries.insert((INDUSTRYID)ii, ci);
		}
	}
	in >> *m_Inventory;
	in >> m_iStorage;
	in >> m_Jobs;
	in >> m_JobsNonConst;
	in >> m_JobsRepair;
	in >> m_LastTribute;
	in >> m_LastVisit;
	if(version >= 3)
		in >> m_LastAvatarAccountActive;
	in >> m_LL;
	in >> m_LoyaltyOrig;
	in >> m_MaxPop;
	in >> m_MessageType;
	in >> m_Name;

	DhDNA nativeDNA;
	if(version >= 13)
	{
		in >> m_NativeSpecies;
		in >> m_NativeSpeciesDesc;
	}
	else
	{
		in >> nativeDNA;
		const DgData& gd = g_Au.LockDataForRead();
		m_NativeSpecies = gd.SpeciesName(nativeDNA.Species(), false);
		g_Au.UnlockData();
		m_NativeSpeciesDesc = nativeDNA.SpeciesDescription();
	}

	in >> m_PlanetName;
	in >> m_PlanetDesc;
	in >> m_PopOrig;
	if(version >= 7)
	{
		in >> m_PopBirths;
		in >> m_PopIndigenousArr;
		in >> m_PopTravellersArr;
	}
	quint32 usedToBePowerConsumed;
	in >> usedToBePowerConsumed;
	in >> m_PatentRnD;
	in >> m_SatOrig;
	in >> m_SectorName;
	in >> m_SectorDesc;
	in >> m_Ships;
	in >> m_SystemName;
	in >> m_SystemDesc;
	in >> *m_Tools;
	in >> m_TroopsAtCity;
	in >> m_TroopsDrafted;
	in >> m_Unemployed;
	m_Vehicles.clear();
	in >> ui8;
	for(int iv=0; iv<ui8; iv++)
	{
		quint16 cid;
		in >> cid;
		m_Vehicles.append((COMMODITYID)cid);
	}

	QDateTime tm;
	in >> tm;
	tm.setTimeSpec(Qt::UTC);
	m_WhenCompiled = tm;

	in >> m_Zone;
	if(version >= 7)
		in >> m_ZonePop;
	else
	{
		quint16 zpop;
		in >> zpop;
		m_ZonePop = zpop;
	}

	if(version >= 1)
	{
		in >> m_FoodTypes;
		in >> m_FoodUnits;
		in >> m_Power;
	}
	else
	{
		// Collect these numbers from old version city report. It should still have its complete inventory.
		ECOROLE workerEcoRole = m_Citizens.empty() ? ECO_Omnivore : Citizen().EcoRole();
		m_FoodTypes = m_Inventory->AvailableFoodTypes(workerEcoRole);
		m_FoodUnits = m_Inventory->TotalFoodUnits(workerEcoRole);
		m_Power = m_Inventory->RawCount(CID_Electricity);
	}

	if(bits & 0x0200)
		in >> m_Patents;

	if(bits & 0x0400)
	{
		if(version >= 15)
		{
			quint8 ui8;
			in >> ui8;
			m_FounderStatus = (FOUNDER_STATUS)ui8;
		}
		in >> m_FounderId;
		in >> m_FounderName;
	}

	if(version < 7)
	{
		// Construct new citizens list from old dna.
		bool isAlien = !dna.IsSexuallyCompatible(nativeDNA);
		m_Citizens.insert(dna.Species(), AuCityCitizen(dna.Species(), dna.EcoRole(), isAlien, m_PopOrig));
	}
	else if(version < 13)
	{
		// Construct new citizens list from old dna list.
		for(QMap<DhDNA, quint32>::const_iterator ci = citizenDNA.cbegin(); ci != citizenDNA.cend(); ci++)
		{
			bool isAlien = !ci.key().IsSexuallyCompatible(nativeDNA);
			m_Citizens.insert(ci.key().Species(), AuCityCitizen(ci.key().Species(), ci.key().EcoRole(), isAlien, *ci));
		}
	}

	if(version >= 8)
	{
		quint32 oldval;
		in >> oldval;
		m_PopIndigenousDep = oldval;	// was m_Arenas in ver 9
		in >> oldval;
		m_PopTravellersDep = oldval;	// was m_Casinos in ver 9
		in >> oldval;
		m_PopDeaths = oldval;			// was m_Churches in ver 9
		in >> m_AirSupply;				// was m_Hospitals in ver 9
		in >> m_Lounges;
		in >> m_FoodPerMeal;			// was m_Parks in ver 9
		in >> m_WorldBuildingCount;		// was m_Schools in ver 9
		in >> m_StoresGrocery;
		in >> m_StoresRetail;
		in >> m_JobsPro;				// was m_Zoos in ver 9
	}

	if(version == 9)
	{
		quint16 wb;
		in >> wb;	// Count of buildings, not volume.
		m_WorldBuildingCount = wb;
	}
	else if(version >= 10)
		in >> m_WorldBuildingVol;

	if(version >= 11)
	{
		in >> m_PopSeafarersArr;
		in >> m_PopSeafarersDep;
		in >> m_PopWanderersArr;
		in >> m_PopWanderersDep;
	}
}

// static
int AuCityReport::HomesPerLevel(INDUSTRYID iid)
{
	// The homes/population value per industry level never changes
	// during program execution so we'll look it up only once and
	// share it with all other cities via static variables.
	if(!s_HomeApartment)
	{
		// Population data query requires game data mutex.
		const DgData& gd = g_Au.LockDataForRead();
		s_HomeApartment = gd.Industry(IID_Apartment).m_population;
		s_HomeFarm = gd.Industry(IID_Farm).m_population;
		s_HomeCondo = gd.Industry(IID_Hotel).m_population;
		s_HomeHouse = gd.Industry(IID_House).m_population;
		s_HomeOrchard = gd.Industry(IID_Orchard).m_population;
		g_Au.UnlockData();
	}

	switch(iid)
	{
	case IID_Apartment:
		return s_HomeApartment;
	case IID_Farm:
		return s_HomeFarm;
	case IID_Hotel:
		return s_HomeCondo;
	case IID_House:
		return s_HomeHouse;
	case IID_Orchard:
		return s_HomeOrchard;
	default:
		return 0;
	}
}

QString AuCityReport::Title() const
{
	if(m_CityType == CITY_MilitaryBase)
	{
		switch(m_MessageType)
		{
		case AuMailMsg::MSG_CityStatusReport:
		default:
			return QObject::tr("Military Base Status Report");
		case AuMailMsg::MSG_CityStatusReportInfo:
			return QObject::tr("Military Base Notice Report");
		case AuMailMsg::MSG_CityDistressReport:
			return QObject::tr("Military Base Distress Report");
		case AuMailMsg::MSG_CityFinalDecayReport:
			return QObject::tr("Military Base Final Decay Report");
		}
	}
	else
	{
		switch(m_MessageType)
		{
		case AuMailMsg::MSG_CityStatusReport:
		default:
			return QObject::tr("City Status Report");
		case AuMailMsg::MSG_CityStatusReportInfo:
			return QObject::tr("City Notice Report");
		case AuMailMsg::MSG_CityDistressReport:
			return QObject::tr("City Distress Report");
		case AuMailMsg::MSG_CityFinalDecayReport:
			return QObject::tr("City Final Decay Report");
		}
	}
}

qint8 AuCityReport::Morale() const
{
	switch((CITYTYPE)m_CityType)
	{
	case CITY_City_Obsolete:
		return 0;
	case CITY_Capitol:
		return MoraleCapitol();
	case CITY_MilitaryBase:
		return MoraleHQ();
	}
	return 0;
}

int AuCityReport::RequiredCantinas() const
{
	switch((CITYTYPE)m_CityType)
	{
	case CITY_City_Obsolete:
		break;
	case CITY_Capitol:
	case CITY_MilitaryBase:
		if(m_PopOrig >= 50)
		{
			int req = m_PopOrig / (50*3);
			if(!req)
				req = 1;
			return req;
		}
		break;
	}
	return 0;
}

int AuCityReport::RequiredHospitals() const
{
	switch((CITYTYPE)m_CityType)
	{
	case CITY_City_Obsolete:
		break;
	case CITY_Capitol:
		if(m_PopOrig >= 80)
		{
			int req = m_PopOrig / (80*3);
			if(!req)
				req = 1;
			return req;
		}
		break;
	case CITY_MilitaryBase:
		break;
	}
	return 0;
}

int AuCityReport::RequiredArenas() const
{
	switch((CITYTYPE)m_CityType)
	{
	case CITY_City_Obsolete:
		break;
	case CITY_Capitol:
		if(m_PopOrig >= 175)
		{
			int req = m_PopOrig / (175*3);
			if(!req)
				req = 1;
			return req;
		}
		break;
	case CITY_MilitaryBase:
		break;
	}
	return 0;
}

int AuCityReport::RequiredCasinos() const
{
	switch((CITYTYPE)m_CityType)
	{
	case CITY_City_Obsolete:
		break;
	case CITY_Capitol:
		if(m_PopOrig >= 200)
		{
			int req = m_PopOrig / (200*3);
			if(!req)
				req = 1;
			return req;
		}
		break;
	case CITY_MilitaryBase:
		break;
	}
	return 0;
}

int AuCityReport::RequiredChurches() const
{
	switch((CITYTYPE)m_CityType)
	{
	case CITY_City_Obsolete:
		break;
	case CITY_Capitol:
		if(m_PopOrig >= 45)
		{
			int req = m_PopOrig / (45*3);
			if(!req)
				req = 1;
			return req;
		}
		break;
	case CITY_MilitaryBase:
		break;
	}
	return 0;
}

int AuCityReport::RequiredGrocery() const
{
	switch((CITYTYPE)m_CityType)
	{
	case CITY_City_Obsolete:
		break;
	case CITY_Capitol:
		if(m_PopOrig >= 100)
		{
			int req = m_PopOrig / (100*3);
			if(!req)
				req = 1;
			return req;
		}
		break;
	case CITY_MilitaryBase:
		break;
	}
	return 0;
}

int AuCityReport::RequiredParks() const
{
	switch((CITYTYPE)m_CityType)
	{
	case CITY_City_Obsolete:
		break;
	case CITY_Capitol:
		if(m_PopOrig >= 90)
		{
			int req = m_PopOrig / (90*3);
			if(!req)
				req = 1;
			return req;
		}
		break;
	case CITY_MilitaryBase:
		break;
	}
	return 0;
}

int AuCityReport::RequiredRetail() const
{
	switch((CITYTYPE)m_CityType)
	{
	case CITY_City_Obsolete:
		break;
	case CITY_Capitol:
	case CITY_MilitaryBase:
		if(m_PopOrig >= 55)
		{
			int req = m_PopOrig / (55*3);
			if(!req)
				req = 1;
			return req;
		}
		break;
	}
	return 0;
}

int AuCityReport::RequiredUniversities() const
{
	switch((CITYTYPE)m_CityType)
	{
	case CITY_City_Obsolete:
		break;
	case CITY_Capitol:
		if(m_PopOrig >= 70)
		{
			int req = m_PopOrig / (70*3);
			if(!req)
				req = 1;
			return req;
		}
		break;
	case CITY_MilitaryBase:
		if(m_PopOrig >= 250)
		{
			int req = m_PopOrig / (250*3);
			if(!req)
				req = 1;
			return req;
		}
		break;
	}
	return 0;
}

int AuCityReport::RequiredZoos() const
{
	switch((CITYTYPE)m_CityType)
	{
	case CITY_City_Obsolete:
		break;
	case CITY_Capitol:
		if(m_PopOrig >= 150)
		{
			int req = m_PopOrig / (150*3);
			if(!req)
				req = 1;
			return req;
		}
		break;
	case CITY_MilitaryBase:
		break;
	}
	return 0;
}

qint8 AuCityReport::MoraleCapitol() const
{
	// When a city is abandoned it is subject to decay.
	int decayMod = 0;
	if(HasDecay())
	{
		for(int decay=0; decay<3; decay++)
			if(m_Decayed[decay] != IID_None)
				decayMod++;
	}

	// Get info about the locals.
	bool isAlienCity = false;									// true if this city is inhabited by aliens on a habitable world
	if(m_IsHabitablePlanet && !m_Citizens.empty() && Citizen().IsAlien())
		isAlienCity = true;

	int worldOverpopPenalty = 0;
	int hungerPenalty = 0;
	int hospitalPenalty = 0;
	//float loyalty = m_LoyaltyOrig;

	// Tally homes.
	unsigned int homesTotal = m_HomesLg + m_HomesMd + m_HomesSm;

	// Adjust satisfaction based on various factors.
	int iSatChange = 0;
	int req_hospitals = 0;
	if(m_PopOrig)
	{
		// Air/Power Consumption
		bool gasping = false;
		if(!m_HasBreathableAtmo)
		{
			// Living in harsh conditions erodes satisfaction.
			if(!HasPower())
			{
				int upen = 2;
				iSatChange -= upen;
			}
			else
			{
				int upen = 1;
				iSatChange -= upen;
			}

			// Air Consumption
			const quint32 airNeeded = m_PopOrig;	// 1 unit of air per citizen per report
			if(!m_AirSupply)
			{
				// No air to go around.
				int upen = 5;
				iSatChange -= upen;
				gasping = true;
			}
			else
			{
				if(m_AirSupply < airNeeded)
				{
					// Not enough air to go around.
					int upen;
					if(m_AirSupply < airNeeded/2)
						upen = 2;
					else
						upen = 1;
					iSatChange -= upen;
					gasping = true;
				}
			}
		}

		// Food Consumption
		bool hungry = false;
		if(!m_FoodTypes)
		{
			// No food to go around.
			hungerPenalty = 5;
			iSatChange -= hungerPenalty;
			hungry = true;
		}
		else if(!m_AteEnoughFood)
		{
			// Not enough food to go around.
			// People don't care about food variety when there isn't enough to go around.
			hungerPenalty = 2;
			iSatChange -= hungerPenalty;
			hungry = true;
		}
		else
		{
			// Food variety improves satisfaction.
			// People don't care about food variety when there isn't enough to go around.
			int foodMod = m_FoodTypes/4;	// Generally 1 point per 4 food types.
			if(foodMod > 2)
				foodMod = 2;				// max +2 food variety bonus
			if(foodMod)
				iSatChange += foodMod;
		}

		// Founder status affects satisfaction.
		switch(m_FounderStatus)
		{
		case FOUNDER_Unknown:
		case FOUNDER_FreeOrDead:
		case FOUNDER_Vassal:
		case FOUNDER_NoEmpire:
		case FOUNDER_NoCity:
		case FOUNDER_NoFounder:
		case FOUNDER_BadStance:
			break;
		case FOUNDER_Empire:
			iSatChange += 2;
			break;
		case FOUNDER_Friend:
			iSatChange += 1;
			break;
		case FOUNDER_Neutral:
			iSatChange -= 1;
			break;
		case FOUNDER_Enemy:
			iSatChange -= 2;
			break;
		}

		// Living in unwelcome conditions erodes satisfaction.
		if(isAlienCity)
		{
			int upen = 1;
			iSatChange -= upen;
		}

		// World overpopulation reduces morale.
		if(m_ZonePop > m_MaxPop/2)
		{
			// Penalty for population pressure starts at 1/2 the max population +1 and reaches -20 at the max population.
			const int penaltyAtMaxPop = 20;									// Morale penalty when population reaches world max.
			int incPenalty = m_MaxPop/2 * (100/penaltyAtMaxPop) / 100;		// Population increment for each -1 penalty.
			int overPop = m_ZonePop - m_MaxPop/2;							// Amount of overpopulation.
			worldOverpopPenalty = overPop / incPenalty;						// No upper limit, -1 per increment.
			iSatChange -= worldOverpopPenalty;
		}

		if(m_PopOrig > homesTotal && !m_HasDestroyedBuildings)
		{
			// Penalty for homelessness when not war induced.
			int overPop = m_PopOrig - homesTotal;
			int homelessnessPenalty = (overPop - (50 * BuildingLevels(IID_AirportTerminal)));	// Number of actual homeless.
			int vagrantTol = m_PopOrig / 100;		// Number of actual homeless per -1 to morale = 1% of population.
			if(vagrantTol > 1)
				homelessnessPenalty /= vagrantTol;
			if(homelessnessPenalty > 0)
				iSatChange -= homelessnessPenalty;
		}

		// Decay affects morale.
		if(decayMod)
			iSatChange -= decayMod;

		// Abandonment reduces morale.
		iSatChange += AbandonMod();

		// When buildings are destroyed due to battle damage, the people pull together.
		if(m_HasDestroyedBuildings)
		{
			if(m_HasDestroyedBuildings == 0x03)
				iSatChange -= 2;
			else
				iSatChange -= 1;
		}

		quint32 bldLimit = WorldBuildingLimit();
		if(m_WorldBuildingVol > bldLimit)
		{
			int upen = (m_WorldBuildingVol - bldLimit) / 1000 + 1;
			iSatChange -= upen;
		}

		if(m_WorldBuildingCount > WorldBuildingCountLimit())
		{
			int upen = m_WorldBuildingCount - WorldBuildingCountLimit();
			iSatChange -= upen;
		}

		{
			// Cramped living conditions reduce satisfaction.
			quint32 nicehomes = m_HomesMd + (m_HomesLg * 2);
			if(m_HomesSm > nicehomes)
			{
				// Apartments account for more than 1/2 of the living space.
				quint32 upen = m_HomesSm;
				if(nicehomes > 1)
					upen /= nicehomes;
				if(upen > 3)
					upen = 3;
				if(upen)
				{
					iSatChange -= upen;
				}
			}

			// Unemployment affects morale.
			// People may wonder why zero unemployment causes dissatisfaction.
			// Unemployed citizens translate into time-off, if you consider it shared between everyone who is working.
			// Zero unemployment means everyone is working too hard and there's no time to relax and enjoy life.
			// Overworked penalty is not applied if there are empty homes. This situation translates into opportunity
			// for immigrants.
			if((quint32)(m_Jobs + m_JobsPro + m_JobsRepair) > m_PopOrig)
			{
				// There are more jobs than people.
				if(m_PopOrig >= homesTotal)
				{
					// Homes are full. There's no way for anyone to immigrate so the existing populace
					// has to work extra hard to do everything. They aren't happy about this.
					int upen = 1;
					iSatChange -= upen;
				}
			}
			else
			{
				float ur = (float)m_Unemployed/(float)m_PopOrig;
				int upen = (int)(ur*5.0);	// Unemployment penalty.
				if(upen)
				{
					ur = round(ur*100);		// Round to percentage.
					iSatChange -= upen;
				}
			}

			// Interesting activities improve satisfaction or simply reduce satisfaction when missing.
			// Bases reduce satisfaction when there are too many. Base limits are applied per resource zone.
			//int allow_troops = m_PopOrig/30;	// If this calculation changes, update DaWorld::MaxFlagsInResourceZone().
			int req_arenas = RequiredArenas();
			int req_casinos = RequiredCasinos();
			int req_churches = RequiredChurches();
			//int req_fire = m_PopOrig/40;
			int req_grocery = RequiredGrocery();
			req_hospitals = RequiredHospitals();
			int req_parks = RequiredParks();
			int req_retail = RequiredRetail();
			int req_univs = RequiredUniversities();
			int req_cantinas = RequiredCantinas();
			int req_zoos = RequiredZoos();

			int arenamod = m_Industries.Levels(IID_Arena) - req_arenas;
			if(arenamod < 0)
			{
				//if(arenamod < -2)
					//arenamod = -2;
				iSatChange += arenamod;
			}

			int cantinamod = m_Lounges - req_cantinas + (m_ConsumedLiquor?1:0);
			if(cantinamod)
			{
				//if(cantinamod < -2)
					//cantinamod = -2;
				if(hungry || gasping)
					cantinamod = MIN(cantinamod, m_ConsumedLiquor?1:0);
				else
					cantinamod = MIN(cantinamod, m_ConsumedLiquor?2:1);
				if(cantinamod)
				{
					iSatChange += cantinamod;
				}
			}

			int casinomod = m_Industries.Levels(IID_Casino) - req_casinos;
			if(casinomod < 0)
			{
				//if(casinomod < -2)
					//casinomod = -2;
				iSatChange += casinomod;
			}

			int churchmod = m_Industries.Levels(IID_Church) - req_churches;
			if(churchmod)
			{
				//if(churchmod < -2)
					//churchmod = -2;
				if(hungry || gasping)
					churchmod = MIN(churchmod, 1);
				else
					churchmod = MIN(churchmod, 2);
				iSatChange += churchmod;
			}

			int grocerymod = m_StoresGrocery - req_grocery;
			if(grocerymod < 0)
			{
				//if(grocerymod < -2)
					//grocerymod = -2;
				iSatChange += grocerymod;
			}

			// Hospital modifier affects infant mortality rate below.
			hospitalPenalty = req_hospitals - m_Industries.Levels(IID_Hospital);
			if(hospitalPenalty > 0)
			{
				//if(hospitalPenalty > 2)
					//hospitalPenalty = 2;
				iSatChange -= hospitalPenalty;
			}
			else
				hospitalPenalty = 0;

			int parkmod = m_Industries.Levels(IID_Park) - req_parks;
			if(parkmod < 0)
			{
				//if(parkmod < -2)
					//parkmod = -2;
				iSatChange += parkmod;
			}

			int retailmod = m_StoresRetail - req_retail;
			if(retailmod < 0)
			{
				//if(retailmod < -2)
					//retailmod = -2;
				iSatChange += retailmod;
			}

			int univmod = m_Industries.Levels(IID_School) - req_univs;
			if(univmod < 0)
			{
				//if(univmod < -2)
					//univmod = -2;
				iSatChange += univmod;
			}

			int zoomod = m_Industries.Levels(IID_Zoo) - req_zoos;
			if(zoomod < 0)
			{
				//if(zoomod < -2)
					//zoomod = -2;
				iSatChange += zoomod;
			}
		}
	}

	// Adjust morale now so we can use the nu value to affect the population growth.
	int nusat = iSatChange;
	if(nusat < -128)
		nusat = -128;
	else if(nusat > 127)
		nusat = 127;
	return nusat;
}

qint8 AuCityReport::MoraleHQ() const
{
	// When a city is abandoned it is subject to decay.
	int decayMod = 0;
	if(HasDecay())
	{
		for(int decay=0; decay<3; decay++)
			if(m_Decayed[decay] != IID_None)
				decayMod++;
	}

	// Get info about the locals.
	bool isAlienCity = false;									// true if this city is inhabited by aliens on a habitable world
	if(m_IsHabitablePlanet && !m_Citizens.empty() && Citizen().IsAlien())
		isAlienCity = true;

	int worldOverpopPenalty = 0;
	int hungerPenalty = 0;
	//int hospitalPenalty = 0;
	//float loyalty = m_LoyaltyOrig;

	// Tally homes.
	unsigned int homesTotal = m_HomesLg + m_HomesMd + m_HomesSm;

	// Adjust satisfaction based on various factors.
	int iSatChange = 0;
	/*
	int req_hospitals = 0;
	int troopContingent = 0;		// Total troop contingent of the city due to bases and guard towers.
	*/
	if(m_PopOrig)
	{
		// Air/Power Consumption
		bool gasping = false;
		if(!m_HasBreathableAtmo)
		{
			// Living in harsh conditions erodes satisfaction.
			if(!HasPower())
			{
				int upen = 2;
				iSatChange -= upen;
			}
			else
			{
				int upen = 1;
				iSatChange -= upen;
			}

			// Air Consumption
			const quint32 airNeeded = m_PopOrig;	// 1 unit of air per citizen per report
			if(!m_AirSupply)
			{
				// No air to go around.
				int upen = 5;
				iSatChange -= upen;
				gasping = true;
			}
			else
			{
				if(m_AirSupply < airNeeded)
				{
					// Not enough air to go around.
					int upen;
					if(m_AirSupply < airNeeded/2)
						upen = 2;
					else
						upen = 1;
					iSatChange -= upen;
					gasping = true;
				}
			}
		}

		// Food Consumption
		bool hungry = false;
		if(!m_FoodTypes)
		{
			// No food to go around.
			hungerPenalty = 5;
			iSatChange -= hungerPenalty;
			hungry = true;
		}
		else if(!m_AteEnoughFood)
		{
			// Not enough food to go around.
			// People don't care about food variety when there isn't enough to go around.
			hungerPenalty = 2;
			iSatChange -= hungerPenalty;
			hungry = true;
		}
		else
		{
			// Food variety improves satisfaction.
			// People don't care about food variety when there isn't enough to go around.
			int foodMod = m_FoodTypes/4;	// Generally 1 point per 4 food types.
			if(foodMod > 2)
				foodMod = 2;				// max +2 food variety bonus
			if(foodMod)
				iSatChange += foodMod;
		}

		// Founder status affects satisfaction.
		switch(m_FounderStatus)
		{
		case FOUNDER_Unknown:
		case FOUNDER_FreeOrDead:
		case FOUNDER_Vassal:
		case FOUNDER_NoEmpire:
		case FOUNDER_NoCity:
		case FOUNDER_NoFounder:
		case FOUNDER_BadStance:
			break;
		case FOUNDER_Empire:
			iSatChange += 2;
			break;
		case FOUNDER_Friend:
			iSatChange += 1;
			break;
		case FOUNDER_Neutral:
			iSatChange -= 1;
			break;
		case FOUNDER_Enemy:
			iSatChange -= 2;
			break;
		}

		// Living in unwelcome conditions erodes satisfaction.
		if(isAlienCity)
		{
			int upen = 1;
			iSatChange -= upen;
		}

		// World overpopulation reduces morale.
		if(m_ZonePop > m_MaxPop/2)
		{
			// Penalty for population pressure starts at 1/2 the max population +1 and reaches -20 at the max population.
			const int penaltyAtMaxPop = 20;									// Morale penalty when population reaches world max.
			int incPenalty = m_MaxPop/2 * (100/penaltyAtMaxPop) / 100;		// Population increment for each -1 penalty.
			int overPop = m_ZonePop - m_MaxPop/2;							// Amount of overpopulation.
			worldOverpopPenalty = overPop / incPenalty;						// No upper limit, -1 per increment.
			iSatChange -= worldOverpopPenalty;
		}

		if(m_PopOrig > homesTotal && !m_HasDestroyedBuildings)
		{
			// Penalty for homelessness when not war induced.
			int overPop = m_PopOrig - homesTotal;
			int homelessnessPenalty = (overPop - (50 * BuildingLevels(IID_BaseAirTerminal)));	// Number of actual homeless.
			int vagrantTol = m_PopOrig / 100;		// Number of actual homeless per -1 to morale = 1% of population.
			if(vagrantTol > 1)
				homelessnessPenalty /= vagrantTol;
			if(homelessnessPenalty > 0)
				iSatChange -= homelessnessPenalty;
		}

		// Decay affects morale.
		if(decayMod)
			iSatChange -= decayMod;

		// Abandonment reduces morale.
		iSatChange += AbandonMod();

		// When buildings are destroyed due to battle damage, the people pull together.
		if(m_HasDestroyedBuildings)
		{
			if(m_HasDestroyedBuildings == 0x03)
				iSatChange -= 2;
			else
				iSatChange -= 1;
		}

		quint32 bldLimit = WorldBuildingLimit();
		if(m_WorldBuildingVol > bldLimit)
		{
			int upen = (m_WorldBuildingVol - bldLimit) / 1000 + 1;
			iSatChange -= upen;
		}

		if(m_WorldBuildingCount > WorldBuildingCountLimit())
		{
			int upen = m_WorldBuildingCount - WorldBuildingCountLimit();
			iSatChange -= upen;
		}

		{
			// Cramped living conditions reduce satisfaction.
			/*
			quint32 nicehomes = m_HomesMd + (m_HomesLg * 2);
			if(m_HomesSm > nicehomes)
			{
				// Apartments account for more than 1/2 of the living space.
				quint32 upen = m_HomesSm / 2;
				if(nicehomes > 1)
					upen /= nicehomes;
				if(upen > 3)
					upen = 3;
				if(upen)
				{
					iSatChange -= upen;
				}
			}
			*/

			// Interesting activities improve satisfaction or simply reduce satisfaction when missing.
			// Bases reduce satisfaction when there are too many. Base limits are applied per resource zone.
			//int allow_troops = m_PopOrig/30;	// If this calculation changes, update DaWorld::MaxFlagsInResourceZone().
			//int req_arenas = RequiredArenas();
			//int req_casinos = RequiredCasinos();
			//int req_churches = RequiredChurches();
			//int req_fire = m_PopOrig/40;
			//int req_grocery = RequiredGrocery();
			//req_hospitals = RequiredHospitals();
			//int req_parks = RequiredParks();
			int req_retail = RequiredRetail();
			int req_univs = RequiredUniversities();
			int req_cantinas = RequiredCantinas();
			//int req_zoos = RequiredZoos();

			/*
			int arenamod = m_Industries.Levels(IID_Arena) - req_arenas;
			if(arenamod < 0)
			{
				if(arenamod < -2)
					arenamod = -2;
				iSatChange += arenamod;
				rpt += QObject::tr("%1 Arena wanted.<br>").arg(arenamod);
			}
			*/

			int cantinamod = m_Lounges - req_cantinas + (m_ConsumedLiquor?1:0);
			if(cantinamod)
			{
				//if(cantinamod < -2)
					//cantinamod = -2;
				if(hungry || gasping)
					cantinamod = MIN(cantinamod, m_ConsumedLiquor?1:0);
				else
					cantinamod = MIN(cantinamod, m_ConsumedLiquor?2:1);
				if(cantinamod)
				{
					iSatChange += cantinamod;
				}
			}

			/*
			int casinomod = m_Industries.Levels(IID_Casino) - req_casinos;
			if(casinomod < 0)
			{
				if(casinomod < -2)
					casinomod = -2;
				iSatChange += casinomod;
				rpt += QObject::tr("%1 Casino wanted.<br>").arg(casinomod);
			}

			int churchmod = m_Industries.Levels(IID_Church) - req_churches;
			if(churchmod)
			{
				if(churchmod < -2)
					churchmod = -2;
				if(hungry || gasping)
					churchmod = MIN(churchmod, 1);
				else
					churchmod = MIN(churchmod, 2);
				iSatChange += churchmod;
				if(churchmod < 0)
					rpt += QObject::tr("%1 Church wanted.<br>").arg(churchmod);
				else
					rpt += QObject::tr("+%1 Church.<br>").arg(churchmod);
			}

			int grocerymod = m_StoresGrocery - req_grocery;
			if(grocerymod < 0)
			{
				if(grocerymod < -2)
					grocerymod = -2;
				iSatChange += grocerymod;
				rpt += QObject::tr("%1 Grocery wanted.<br>").arg(grocerymod);
			}

			// Hospital modifier affects infant mortality rate below.
			hospitalPenalty = req_hospitals - m_Industries.Levels(IID_Hospital);
			if(hospitalPenalty > 0)
			{
				if(hospitalPenalty > 2)
					hospitalPenalty = 2;
				iSatChange -= hospitalPenalty;
				rpt += QObject::tr("%1 Hospital wanted.<br>").arg(-hospitalPenalty);
			}
			else
				hospitalPenalty = 0;

			int parkmod = m_Industries.Levels(IID_Park) - req_parks;
			if(parkmod < 0)
			{
				if(parkmod < -2)
					parkmod = -2;
				iSatChange += parkmod;
				rpt += QObject::tr("%1 Park wanted.<br>").arg(parkmod);
			}
			*/

			int retailmod = m_StoresRetail - req_retail;
			if(retailmod < 0)
			{
				//if(retailmod < -2)
					//retailmod = -2;
				iSatChange += retailmod;
			}

			int univmod = m_Industries.Levels(IID_BaseAcademy) - req_univs;
			if(univmod < 0)
			{
				//if(univmod < -2)
					//univmod = -2;
				iSatChange += univmod;
			}

			/*
			int zoomod = m_Industries.Levels(IID_Zoo) - req_zoos;
			if(zoomod < 0)
			{
				if(zoomod < -2)
					zoomod = -2;
				iSatChange += zoomod;
				rpt += QObject::tr("%1 Zoo wanted.<br>").arg(zoomod);
			}
			*/
		}
	}

	// Adjust morale now so we can use the nu value to affect the population growth.
	int nusat = iSatChange;
	if(nusat < -128)
		nusat = -128;
	else if(nusat > 127)
		nusat = 127;
	return nusat;
}

QString AuCityReport::ReportHtml(const AuCityReport& prevReport, bool mailFormat) const
{
	switch((CITYTYPE)m_CityType)
	{
	case CITY_City_Obsolete:
		return "";
	case CITY_Capitol:
		return ReportCapitol(prevReport, mailFormat);
	case CITY_MilitaryBase:
		return ReportHQ(prevReport, mailFormat);
	}
	return "";
}

QString AuCityReport::ReportCapitol(const AuCityReport& prevReport, bool mailFormat) const
{
	QString rpt;	// This will be our return value.
	if(mailFormat)
	{
		// Mail format expects an encoded date-time value for localization.
		// It also does not expect an icon at the beginning.
		rpt = g_Au.EncodeUTCString(m_WhenCompiled) + "<br><br>";
	}
	else
	{
		// Start with icon and date-time stamp.
		QString typeIcon;
		switch(m_MessageType)
		{
		case AuMailMsg::MSG_CityStatusReport:
		default:
			typeIcon = ":data/MailCityStatus.png";
			break;
		case AuMailMsg::MSG_CityStatusReportInfo:
			typeIcon = ":data/MailCityStatusInfo.png";
			break;
		case AuMailMsg::MSG_CityDistressReport:
			typeIcon = ":data/MailCityDistress.png";
			break;
		case AuMailMsg::MSG_CityFinalDecayReport:
			typeIcon = ":data/MailCityDecayed.png";
			break;
		}

		rpt += QString("<img height=\"32\" src=\"qrc%1\"><br>%2<br><big><b>%3</b></big><p>")
			.arg(typeIcon)
			.arg(m_WhenCompiled.UTC().toLocalTime().toString(Qt::SystemLocaleShortDate))
			.arg(Title());
	}

	// Information about the city.
	rpt += "<b>" + m_Name + "</b><p style=\"margin-left: 10px;\">";
	rpt += m_PlanetName + ", " + m_PlanetDesc + "<br>";		// Planet
	rpt += m_SystemName + ' ' + m_SystemDesc + "<br>";
	rpt += m_SectorName + ' ' + m_SectorDesc + "<br>";	// Sector
	rpt += QString("%1<br>").arg(GalaxyName((GALAXY)m_Galaxy));

	// Longitude and latitude coordinates.
	// On globe world, my location is really my location.
	// On old-style sphere and ringworlds, this city's location is the real long lat on its parent planet, in terrain units.
	QString strLongLat = m_LL.ToString();

	rpt += QObject::tr("%1, Resource Zone %2").arg(strLongLat).arg(m_Zone+1);
	if(m_IsCapitalOfEmpire)
		rpt += QObject::tr("<br>Empire Capital City");
	if(m_IsCapitalOfGalaxy)
		rpt += QObject::tr("<br>Galaxy Capital City");
	if(m_IsCapitalOfSector)
		rpt += QObject::tr("<br>Sector Capital City");
	if(!m_FounderId.IsNull())
		rpt += QObject::tr("<br>Founder %1 <i>%2</i>").arg(m_FounderName).arg(m_FounderId.StringId());
	rpt += "</p>";

	// When reporting distress situations, we like to see the reasons right at the top of the  This index will
	// help us to insert it there later.
	int distressIdx = rpt.length();	// Insertion point for the distress portion of the report, if any.

	// When a city is more than a week old, it is subject to decay.
	// That is, one or more developed tiles per city report may be be deleted, saving the town square for last.
	// When the town square is deleted, this city atom dies and the terrain itself becomes subject to decay.
	int decayMod = 0;
	if(HasDecay())
	{
		rpt += QObject::tr("<b>DECAY</b><p style=\"margin-left: 10px;\">");
		rpt += QObject::tr("The city is abandoned.<br>");
		for(int decay=0; decay<3; decay++)
		{
			if(m_Decayed[decay] != IID_None)
			{
				const DgData& gd = g_Au.LockDataForRead();
				QString ind = gd.IndustryName(m_Decayed[decay]);
				g_Au.UnlockData();
				rpt += QObject::tr("%1 crumbled away.<br>").arg(ind);
				decayMod++;
			}
		}
		if(rpt.right(4) == "<br>")
			rpt = rpt.left(rpt.count() - 4);	// Drop the trailing <br>.
		rpt += "</p>";
	}

	// City Event Log
	QString eventRpt = m_EventLog->ReportHtml("EVENT LOG", AuTime());
	if(!eventRpt.isEmpty())
	{
		if(mailFormat)
			rpt += eventRpt;							// Mail decodes UTC strings at the receiving end.
		else
			rpt += g_Au.DecodeUTCStrings(eventRpt);		// Report must decode UTC strings now.
	}

	// Get info about the locals.
	bool isAlienCity = false;									// true if this city is inhabited by aliens on a habitable world
	if(m_IsHabitablePlanet && !m_Citizens.empty() && Citizen().IsAlien())
		isAlienCity = true;

	int worldOverpopPenalty = 0;
	int hungerPenalty = 0;
	int hospitalPenalty = 0;
	//float loyalty = m_LoyaltyOrig;

	// Determine the number of buildings in the city, after decay.
	AuCityIndustryHash industries(prevReport.m_Industries);	// Copy previous values to see the change.

	// Tally homes.
	unsigned int homesTotal = m_HomesLg + m_HomesMd + m_HomesSm;

	// Adjust satisfaction based on various factors.
	rpt += QObject::tr("<b>MORALE %1</b><p style=\"margin-left: 10px;\">").arg(MoraleCapitol());
	int iSatChange = 0;
	int req_hospitals = 0;
	if(m_PopOrig)
	{
		// Air/Power Consumption
		bool gasping = false;
		if(!m_HasBreathableAtmo)
		{
			// Living in harsh conditions erodes satisfaction.
			if(!HasPower())
			{
				int upen = 2;
				iSatChange -= upen;
				rpt += QObject::tr("-%1 Harsh Environment Without Power. Power Needed.<br>").arg(upen);
			}
			else
			{
				int upen = 1;
				iSatChange -= upen;
				rpt += QObject::tr("-%1 Harsh Environment.<br>").arg(upen);
			}

			// Air Consumption
			const quint32 airNeeded = m_PopOrig;	// 1 unit of air per citizen per report
			if(!m_AirSupply)
			{
				// No air to go around.
				int upen = 5;
				quint32 suffocated = m_PopOrig / 10;	// 10% of the remaining population suffocates to death.
				if(suffocated < 4)
					suffocated = 4;				// At least 4 citizens suffocated to death.
				if(suffocated > m_PopOrig)
					suffocated = m_PopOrig;	// The last of the population suffocated to death.
				if(m_AirConsumed < 0)
				{
					if(suffocated == 1)
						rpt += QObject::tr("-%1 Suffocation. One citizen will suffocate to death. Air is needed.<br>")
							.arg(upen);
					else
						rpt += QObject::tr("-%1 Suffocation. %L2 citizens will suffocate to death. Air is needed.<br>")
							.arg(upen).arg(suffocated);
				}
				else
				{
					if(suffocated == 1)
						rpt += QObject::tr("-%1 Suffocation. One citizen suffocated to death. Air is needed.<br>")
							.arg(upen);
					else
						rpt += QObject::tr("-%1 Suffocation. %L2 citizens suffocated to death. Air is needed.<br>")
							.arg(upen).arg(suffocated);
				}
				iSatChange -= upen;
				gasping = true;
			}
			else
			{
				if(m_AirSupply < airNeeded)
				{
					// Not enough air to go around.
					int upen;
					if(m_AirSupply < airNeeded/2)
						upen = 2;
					else
						upen = 1;
					rpt += QObject::tr("-%1 Gasping. More air is needed.<br>").arg(upen);
					iSatChange -= upen;
					gasping = true;
				}
			}
		}

		// Food Consumption
		bool hungry = false;
		if(!m_FoodTypes)
		{
			// No food to go around.
			hungerPenalty = 5;
			quint32 starved = m_PopOrig / 10;	// 10% of the remaining population starves to death.
			if(starved < 4)
				starved = 4;					// At least 4 citizens starved to death.
			if(starved > m_PopOrig)
				starved = m_PopOrig;			// The last of the population starved to death.
			if(m_AirConsumed < 0)
			{
				// Air consumption is unknown when this is an intermediate report, of present conditions, not a regular city cycle
				// report of the state at the end of each city cycle.
				if(starved == 1)
					rpt += QObject::tr("-%1 Starvation. One citizen will starve to death. Food is needed.<br>")
						.arg(hungerPenalty);
				else
					rpt += QObject::tr("-%1 Starvation. %L2 citizens will starve to death. Food is needed.<br>")
						.arg(hungerPenalty)
						.arg(starved);
			}
			else
			{
				if(starved == 1)
					rpt += QObject::tr("-%1 Starvation. One citizen starved to death. Food is needed.<br>")
						.arg(hungerPenalty);
				else
					rpt += QObject::tr("-%1 Starvation. %L2 citizens starved to death. Food is needed.<br>")
						.arg(hungerPenalty)
						.arg(starved);
			}
			iSatChange -= hungerPenalty;
			hungry = true;
		}
		else if(!m_AteEnoughFood)
		{
			// Not enough food to go around.
			// People don't care about food variety when there isn't enough to go around.
			hungerPenalty = 2;
			rpt += QObject::tr("-%1 Hunger. More food is needed.<br>").arg(hungerPenalty);
			iSatChange -= hungerPenalty;
			hungry = true;
		}
		else
		{
			// Food variety improves satisfaction.
			// People don't care about food variety when there isn't enough to go around.
			int foodMod = m_FoodTypes/4;	// Generally 1 point per 4 food types.
			if(foodMod > 2)
				foodMod = 2;				// max +2 food variety bonus
			if(foodMod)
			{
				iSatChange += foodMod;
				rpt += QObject::tr("+%1 Food Variety. %2 food types.<br>").arg(foodMod).arg(m_FoodTypes);
			}
		}

		// Founder status affects satisfaction.
		switch(m_FounderStatus)
		{
		case FOUNDER_Unknown:
		case FOUNDER_FreeOrDead:
		case FOUNDER_Vassal:
		case FOUNDER_NoEmpire:
		case FOUNDER_NoCity:
		case FOUNDER_NoFounder:
		case FOUNDER_BadStance:
			break;
		case FOUNDER_Empire:
			iSatChange += 2;
			rpt += QObject::tr("+2 Founder is a famous city planner in our empire.<br>");
			break;
		case FOUNDER_Friend:
			rpt += QObject::tr("+1 Founder remains an esteemed friend of our empire.<br>");
			iSatChange += 1;
			break;
		case FOUNDER_Neutral:
			rpt += QObject::tr("-1 Forsaken. Our empire is neutral to the founder's empire.<br>");
			iSatChange -= 1;
			break;
		case FOUNDER_Enemy:
			rpt += QObject::tr("-2 Occupied. Our empire is an enemy of the founder.<br>");
			iSatChange -= 2;
			break;
		}

		// Living in unwelcome conditions erodes satisfaction.
		if(isAlienCity)
		{
			int upen = 1;
			iSatChange -= upen;
			rpt += QObject::tr("-%1 Alien Invader.<br>").arg(upen);
		}

		// World overpopulation reduces morale.
		if(m_ZonePop > m_MaxPop/2)
		{
			// Penalty for population pressure starts at 1/2 the max population +1 and reaches -20 at the max population.
			const int penaltyAtMaxPop = 20;									// Morale penalty when population reaches world max.
			int incPenalty = m_MaxPop/2 * (100/penaltyAtMaxPop) / 100;		// Population increment for each -1 penalty.
			int overPop = m_ZonePop - m_MaxPop/2;							// Amount of overpopulation.
			worldOverpopPenalty = overPop / incPenalty;						// No upper limit, -1 per increment.
			if(worldOverpopPenalty)
			{
				iSatChange -= worldOverpopPenalty;
				rpt += QObject::tr("-%1 World Overpopulation. World population is %L2.<br>")
					.arg(worldOverpopPenalty).arg(m_ZonePop);
			}
		}

		if(m_PopOrig > homesTotal && !m_HasDestroyedBuildings)
		{
			// Penalty for homelessness when not war induced.
			int overPop = m_PopOrig - homesTotal;
			int homelessnessPenalty = (overPop - (50 * BuildingLevels(IID_AirportTerminal)));	// Number of actual homeless.
			int vagrantTol = m_PopOrig / 100;		// Number of actual homeless per -1 to morale = 1% of population.
			if(vagrantTol > 1)
				homelessnessPenalty /= vagrantTol;
			if(homelessnessPenalty > 0)
			{
				iSatChange -= homelessnessPenalty;
				rpt += QObject::tr("-%1 Homelessness. More homes needed.<br>").arg(homelessnessPenalty);
			}
		}

		// Decay affects morale.
		if(decayMod)
		{
			iSatChange -= decayMod;
			rpt += QObject::tr("-%1 Decay.<br>").arg(decayMod);
		}

		// Abandonment reduces morale.
		int abandonMod = AbandonMod();
		iSatChange += abandonMod;
		if(abandonMod > 0)
			rpt += QObject::tr("+%1 Cosmopolitan. An avatar visited this system recently.<br>")
				.arg(abandonMod);
		else if(IsAbandoned())
		{
			if(m_LastAvatarAccountActive == 0xffff && m_LastVisit == 0xffff)
				rpt += QObject::tr("%1 Abandoned. "
						"No active avatar has ever called this system their land "
						"no on-line avatar has ever visited this system.<br>")
					.arg(abandonMod);
			else if(m_LastAvatarAccountActive == 0xffff)
				rpt += QObject::tr("%1 Abandoned. "
						"No active avatar has ever called this system their land "
						"the last on-line avatar to visit this system was %2 days ago.<br>")
					.arg(abandonMod).arg(m_LastVisit);
			else if(m_LastVisit == 0xffff)
				rpt += QObject::tr("%1 Abandoned. "
						"The last time an active avatar called this system their land was %2 days ago and "
						"no on-line avatar has ever visited this system. Impossible? I just report what they tell me.<br>")
					.arg(abandonMod).arg(m_LastAvatarAccountActive);
			else
				rpt += QObject::tr("%1 Abandoned. "
						"The last time an active avatar called this system their land was %2 days ago and "
						"the last on-line avatar to visit this system was %3 days ago.<br>")
					.arg(abandonMod).arg(m_LastAvatarAccountActive).arg(m_LastVisit);
		}

		// When buildings are destroyed due to battle damage, the people pull together.
		if(m_HasDestroyedBuildings)
		{
			if(m_HasDestroyedBuildings == 0x03)
			{
				iSatChange -= 2;
				rpt += QObject::tr("-2 Destroyed and Burning Buildings.<br>");
			}
			else if(m_HasDestroyedBuildings & 0x01)
			{
				iSatChange -= 1;
				rpt += QObject::tr("-1 Destroyed Buildings.<br>");
			}
			else
			{
				iSatChange -= 1;
				rpt += QObject::tr("-1 Burning Buildings.<br>");
			}
		}

		quint32 bldLimit = WorldBuildingLimit();
		if(m_WorldBuildingVol > bldLimit)
		{
			int upen = (m_WorldBuildingVol - bldLimit) / 1000 + 1;
			iSatChange -= upen;
			rpt += QObject::tr("-%1 Too Much Building Volume on World.<br>").arg(upen);
		}

		if(m_WorldBuildingCount > WorldBuildingCountLimit())
		{
			int upen = m_WorldBuildingCount - WorldBuildingCountLimit();
			iSatChange -= upen;
			rpt += QObject::tr("-%1 Too Many Buildings on World.<br>").arg(upen);
		}

		{
			// Cramped living conditions reduce satisfaction.
			quint32 nicehomes = m_HomesMd + (m_HomesLg * 2);
			if(m_HomesSm > nicehomes)
			{
				// Apartments account for more than 1/2 of the living space.
				quint32 upen = m_HomesSm;
				if(nicehomes > 1)
					upen /= nicehomes;
				if(upen > 3)
					upen = 3;
				if(upen)
				{
					iSatChange -= upen;
					rpt += QObject::tr("-%1 Cramped Living Conditions. Need more medium or large homes.<br>").arg(upen);
				}
			}

			// Unemployment affects morale.
			// People may wonder why zero unemployment causes dissatisfaction.
			// Unemployed citizens translate into time-off, if you consider it shared between everyone who is working.
			// Zero unemployment means everyone is working too hard and there's no time to relax and enjoy life.
			// Overworked penalty is not applied if there are empty homes. This situation translates into opportunity
			// for immigrants.
			if((quint32)(m_Jobs + m_JobsPro + m_JobsRepair) > m_PopOrig)
			{
				// There are more jobs than people.
				if(m_PopOrig >= homesTotal)
				{
					// Homes are full. There's no way for anyone to immigrate so the existing populace
					// has to work extra hard to do everything. They aren't happy about this.
					int upen = 1;
					iSatChange -= upen;
					rpt += QObject::tr("-%1 Overworked. Need homes for immigration of more citizens.<br>").arg(upen);
				}
			}
			else
			{
				float ur = (float)m_Unemployed/(float)m_PopOrig;
				int upen = (int)(ur*5.0);	// Unemployment penalty.
				if(upen)
				{
					ur = round(ur*100);		// Round to percentage.
					iSatChange -= upen;
					rpt += QObject::tr("-%1 %2% Unemployment.<br>").arg(upen).arg(ur);
				}
			}

			// Interesting activities improve satisfaction or simply reduce satisfaction when missing.
			// Bases reduce satisfaction when there are too many. Base limits are applied per resource zone.
			//int allow_troops = m_PopOrig/30;	// If this calculation changes, update DaWorld::MaxFlagsInResourceZone().
			int req_arenas = RequiredArenas();
			int req_casinos = RequiredCasinos();
			int req_churches = RequiredChurches();
			//int req_fire = m_PopOrig/40;
			int req_grocery = RequiredGrocery();
			req_hospitals = RequiredHospitals();
			int req_parks = RequiredParks();
			int req_retail = RequiredRetail();
			int req_univs = RequiredUniversities();
			int req_cantinas = RequiredCantinas();
			int req_zoos = RequiredZoos();

			int arenamod = m_Industries.Levels(IID_Arena) - req_arenas;
			if(arenamod < 0)
			{
				//if(arenamod < -2)
					//arenamod = -2;
				iSatChange += arenamod;
				rpt += QObject::tr("%1 Arena wanted.<br>").arg(arenamod);
			}

			int cantinamod = m_Lounges - req_cantinas + (m_ConsumedLiquor?1:0);
			if(cantinamod)
			{
				//if(cantinamod < -2)
					//cantinamod = -2;
				if(hungry || gasping)
					cantinamod = MIN(cantinamod, m_ConsumedLiquor?1:0);
				else
					cantinamod = MIN(cantinamod, m_ConsumedLiquor?2:1);
				if(cantinamod)
				{
					iSatChange += cantinamod;
					if(cantinamod < 0)
						rpt += QObject::tr("%1 Lounge wanted.<br>").arg(cantinamod);
					else if(m_Lounges)
						rpt += QObject::tr("+%1 Lounge.<br>").arg(cantinamod);
					else
						rpt += QObject::tr("+%1 Liquor.<br>").arg(cantinamod);
				}
			}

			int casinomod = m_Industries.Levels(IID_Casino) - req_casinos;
			if(casinomod < 0)
			{
				//if(casinomod < -2)
					//casinomod = -2;
				iSatChange += casinomod;
				rpt += QObject::tr("%1 Casino wanted.<br>").arg(casinomod);
			}

			int churchmod = m_Industries.Levels(IID_Church) - req_churches;
			if(churchmod)
			{
				//if(churchmod < -2)
					//churchmod = -2;
				if(hungry || gasping)
					churchmod = MIN(churchmod, 1);
				else
					churchmod = MIN(churchmod, 2);
				iSatChange += churchmod;
				if(churchmod < 0)
					rpt += QObject::tr("%1 Church wanted.<br>").arg(churchmod);
				else
					rpt += QObject::tr("+%1 Church.<br>").arg(churchmod);
			}

			int grocerymod = m_StoresGrocery - req_grocery;
			if(grocerymod < 0)
			{
				//if(grocerymod < -2)
					//grocerymod = -2;
				iSatChange += grocerymod;
				rpt += QObject::tr("%1 Grocery wanted.<br>").arg(grocerymod);
			}

			// Hospital modifier affects infant mortality rate below.
			hospitalPenalty = req_hospitals - m_Industries.Levels(IID_Hospital);
			if(hospitalPenalty > 0)
			{
				//if(hospitalPenalty > 2)
					//hospitalPenalty = 2;
				iSatChange -= hospitalPenalty;
				rpt += QObject::tr("%1 Hospital wanted.<br>").arg(-hospitalPenalty);
			}
			else
				hospitalPenalty = 0;

			int parkmod = m_Industries.Levels(IID_Park) - req_parks;
			if(parkmod < 0)
			{
				//if(parkmod < -2)
					//parkmod = -2;
				iSatChange += parkmod;
				rpt += QObject::tr("%1 Park wanted.<br>").arg(parkmod);
			}

			int retailmod = m_StoresRetail - req_retail;
			if(retailmod < 0)
			{
				//if(retailmod < -2)
					//retailmod = -2;
				iSatChange += retailmod;
				rpt += QObject::tr("%1 Retail Store wanted.<br>").arg(retailmod);
			}

			int univmod = m_Industries.Levels(IID_School) - req_univs;
			if(univmod < 0)
			{
				//if(univmod < -2)
					//univmod = -2;
				iSatChange += univmod;
				rpt += QObject::tr("%1 University wanted.<br>").arg(univmod);
			}

			int zoomod = m_Industries.Levels(IID_Zoo) - req_zoos;
			if(zoomod < 0)
			{
				//if(zoomod < -2)
					//zoomod = -2;
				iSatChange += zoomod;
				rpt += QObject::tr("%1 Zoo wanted.<br>").arg(zoomod);
			}
		}
	}
	else
		rpt += QObject::tr("No citizens.<br>");

	if(rpt.right(4) == "<br>")
		rpt = rpt.left(rpt.count() - 4);	// Drop the trailing <br>.
	rpt += "</p>";

	// Adjust morale now so we can use the nu value to affect the population growth.
	int nusat = iSatChange;
	if(nusat < -128)
		nusat = -128;
	else if(nusat > 127)
		nusat = 127;

	// Report population changes, based on nu morale, starvation and suffocation.
	rpt += QObject::tr("<b>POPULATION %L1</b><p style=\"margin-left: 10px;\">").arg(m_PopOrig);
	if(m_PopBirths || m_PopDeaths ||
			m_PopIndigenousArr || m_PopIndigenousDep ||
			m_PopSeafarersArr || m_PopSeafarersDep ||
			m_PopTravellersArr || m_PopTravellersDep ||
			m_PopWanderersArr || m_PopWanderersDep)
	{
		// Births and deaths.
		if(m_PopBirths)
		{
			if(m_PopBirths > 1)
				rpt += QObject::tr("+%1 %2 %1 babies were born.<br>").arg(m_PopBirths).arg(g_Au.MiddleDotSymbol());
			else
				rpt += QObject::tr("+1 %2 A baby was born.<br>").arg(g_Au.MiddleDotSymbol());
		}
		if(m_PopDeaths)
		{
			if(m_PopDeaths > 1)
				rpt += QObject::tr("-%1 %2 %1 citizens died.<br>").arg(m_PopDeaths).arg(g_Au.MiddleDotSymbol());
			else
				rpt += QObject::tr("-1 %2 A citizen died.<br>").arg(g_Au.MiddleDotSymbol());
		}

		// Indigenous
		if(m_PopIndigenousArr)
		{
			if(m_PopIndigenousArr > 1)
				rpt += QObject::tr("+%1 %2 %1 indigenous people emerged from the wilderness.<br>").arg(m_PopIndigenousArr)
					.arg(g_Au.MiddleDotSymbol());
			else
				rpt += QObject::tr("+1 %2 An indigenous person emerged from the wilderness.<br>").arg(g_Au.MiddleDotSymbol());
		}
		if(m_PopIndigenousDep)
		{
			if(m_PopIndigenousDep > 1)
				rpt += QObject::tr("-%1 %2 %1 citizens disappeared into the wilderness.<br>").arg(m_PopIndigenousDep)
					.arg(g_Au.MiddleDotSymbol());
			else
				rpt += QObject::tr("-1 %2 A citizen disappeared into the wilderness.<br>").arg(g_Au.MiddleDotSymbol());
		}

		// Land travellers.
		if(m_PopWanderersArr)
		{
			if(m_PopWanderersArr > 1)
				rpt += QObject::tr("+%1 %2 %1 wanderers entered the city from the road.<br>").arg(m_PopWanderersArr)
					.arg(g_Au.MiddleDotSymbol());
			else
				rpt += QObject::tr("+1 %2 A wanderer entered the city from the road.<br>").arg(g_Au.MiddleDotSymbol());
		}
		if(m_PopWanderersDep)
		{
			if(m_PopWanderersDep > 1)
				rpt += QObject::tr("-%1 %2 %1 citizens headed out on the open road.<br>").arg(m_PopWanderersDep)
					.arg(g_Au.MiddleDotSymbol());
			else
				rpt += QObject::tr("-1 %2 A citizen headed out on the open road.<br>").arg(g_Au.MiddleDotSymbol());
		}

		// Seafarers
		if(m_PopSeafarersArr)
		{
			if(m_PopSeafarersArr > 1)
				rpt += QObject::tr("+%1 %2 %1 seafarers made landfall at the wharf.<br>").arg(m_PopSeafarersArr)
					.arg(g_Au.MiddleDotSymbol());
			else
				rpt += QObject::tr("+1 %2 A seafarer made landfall at the wharf.<br>").arg(g_Au.MiddleDotSymbol());
		}
		if(m_PopSeafarersDep)
		{
			if(m_PopSeafarersDep > 1)
				rpt += QObject::tr("-%1 %2 %1 citizens struck out on the salty sea.<br>").arg(m_PopSeafarersDep)
					.arg(g_Au.MiddleDotSymbol());
			else
				rpt += QObject::tr("-1 %2 A citizen struck out on the salty sea.<br>").arg(g_Au.MiddleDotSymbol());
		}

		// Air|space travellers.
		if(m_PopTravellersArr)
		{
			if(HasAirport())
			{
				if(m_PopTravellersArr > 1)
					rpt += QObject::tr("+%1 %2 %1 travellers arrived at the airport.<br>").arg(m_PopTravellersArr)
						.arg(g_Au.MiddleDotSymbol());
				else
					rpt += QObject::tr("+1 %2 A traveller arrived at the airport.<br>").arg(g_Au.MiddleDotSymbol());
			}
			else
			{
				if(m_PopTravellersArr > 1)
					rpt += QObject::tr("+%1 %2 %1 travellers arrived from space.<br>").arg(m_PopTravellersArr)
						.arg(g_Au.MiddleDotSymbol());
				else
					rpt += QObject::tr("+1 %2 A traveller arrived from space.<br>").arg(g_Au.MiddleDotSymbol());
			}
		}
		if(m_PopTravellersDep)
		{
			if(m_PopTravellersDep > 1)
				rpt += QObject::tr("-%1 %2 %1 citizens departed to travel the stars.<br>").arg(m_PopTravellersDep)
					.arg(g_Au.MiddleDotSymbol());
			else
				rpt += QObject::tr("-1 %2 A citizen departed to travel the stars.<br>").arg(g_Au.MiddleDotSymbol());
		}
	}
	else
		rpt += QObject::tr("No change.<br>");

	// Describe the locals.
	if(m_Citizens.size() > 1)
	{
		rpt += QObject::tr("Citizens are mixed races.<br>");
		for(QMap<AuSpecies, AuCityCitizen>::const_iterator ci = m_Citizens.cbegin(); ci != m_Citizens.cend(); ci++)
		{
			QString citizenName = (*ci).SpeciesName();
			QString citizenDesc = (*ci).SpeciesDesc();
			if(citizenName != citizenDesc && !citizenName.isEmpty())
				rpt += QObject::tr("%L3 are %1, %2.<br>").arg(citizenName).arg(citizenDesc).arg((*ci).Population());
			else
				rpt += QObject::tr("%L3 are %1.<br>").arg(citizenDesc).arg((*ci).Population());
		}
	}
	else if(!m_Citizens.empty())
	{
		QMap<AuSpecies, AuCityCitizen>::const_iterator ci = m_Citizens.cbegin();
		QString citizenName = (*ci).SpeciesName();
		QString citizenDesc = (*ci).SpeciesDesc();
		if(citizenName != citizenDesc && !citizenName.isEmpty())
			rpt += QObject::tr("Citizens are %1, %2.<br>").arg(citizenName).arg(citizenDesc);
		else
			rpt += QObject::tr("Citizens are %1.<br>").arg(citizenDesc);
	}

	if(isAlienCity)
	{
		if(!m_NativeSpecies.isEmpty() && m_NativeSpecies != m_NativeSpeciesDesc)
			rpt += QObject::tr("Indigenous are %1, %2.<br>").arg(m_NativeSpecies).arg(m_NativeSpeciesDesc);
		else
			rpt += QObject::tr("Indigenous are %1.<br>").arg(m_NativeSpeciesDesc);
	}

	// Report if the parent planet has a ship captain available.
	if(!m_CaptName.isEmpty())
		rpt += QObject::tr("University graduate %1 is ready for command assignment aboard a spacecraft.<br>")
			.arg(m_CaptName);

	if(rpt.right(4) == "<br>")
		rpt = rpt.left(rpt.count() - 4);	// Drop the trailing <br>.
	rpt += "</p>";

	// Homes
	rpt += QObject::tr("<b>HOMES %6</b>"
			"<p style=\"margin-left: 10px;\">"
				"Large %9<br>"
				"Medium %8<br>"
				"Small %7"
				"</p>")
		.arg(m_HomesLg + m_HomesMd + m_HomesSm)
		.arg(m_HomesSm)
		.arg(m_HomesMd)
		.arg(m_HomesLg);

	// Jobs
	rpt += QObject::tr("<b>JOBS %1</b>"
			"<p style=\"margin-left: 10px;\">"
			"Construction %2<br>"
			"Manufacturing %3<br>"
			"Professional %4<br>"
			"Service %5"
			"</p>")
		.arg(m_Jobs + m_JobsRepair + m_JobsNonConst + m_JobsPro)
		.arg(m_JobsNonConst)
		.arg(m_Jobs)
		.arg(m_JobsPro)
		.arg(m_JobsRepair);

	// Power status
	if(m_iCapacitor)
	{
		rpt += QObject::tr("<b>POWER RESERVE %L1</b><p style=\"margin-left: 10px;\">").arg(m_Power);
		rpt += QObject::tr("Reserve Capacity %L1<br>").arg(m_iCapacitor);
		int percent = Percent(m_Power, m_iCapacitor);
		rpt += QObject::tr("%1% Reserve").arg(percent);
		rpt += "</p>";
	}

	// Show info about taxes accumulated and spent locally.
	if(BuildingLevels(IID_Bank))
	{
		rpt += QObject::tr("<b>BANK %L1%2</b><p style=\"margin-left: 10px;\">")
			.arg(m_Bank.BalanceEnding(), 0,'f',0)
			.arg(g_Au.CurrencySymbol());
		if(m_LastTribute > 0)
			rpt += QObject::tr("Previous Tribute %L1%2<br>")
				.arg(m_LastTribute, 0,'f',0)
				.arg(g_Au.CurrencySymbol());
		rpt += m_Bank.HtmlReport();
		rpt += "</p>";
	}

	// Spacecraft - All ships in town and nu ships in orbit.
	if(!m_Ships.isEmpty())
	{
		rpt += QObject::tr("<b>SPACECRAFT</b><p style=\"margin-left: 10px;\">");
		if(m_Industries.HasSensors() || m_Industries.Levels(IID_Observatory))
			rpt += QObject::tr("All spacecraft in orbit and in this city, regardless of empire.<br>");
		else
			rpt += QObject::tr("Empire-flagged spacecraft in orbit and all spacecraft in this city regardless of empire.<br>");

		// Make a table of the spacecraft.
		rpt += QObject::tr("<table border=\"0\" cellpadding=\"2\" "
			"cellspacing=\"0\" style=\"background-color: rgb(39, 64, 118);\"><tbody>");
		//rpt += QObject::tr("<tr>"
				//"<td><b>Spacecraft</b></td><td></td>"
				//"</tr>");
		for(QList<AuCityReportShip>::const_iterator si = m_Ships.cbegin(); si != m_Ships.cend(); si++)
		{
			QString icon;
			if((*si).m_Location == 2)
				icon = ":data/bHangar.png";
			else if((*si).m_IsSpaceStation)
				icon = ":data/CommStation.png";
			else
				icon = ":data/c_Spacecraft.png";

			QString note;
			if((*si).m_Location == 2)
				note = QObject::tr("Hangar, %1").arg((*si).m_ShipLL.ToString());
			else if((*si).m_Location == 1)
				note = QObject::tr("Ground, %1").arg((*si).m_ShipLL.ToString());
			else
				note = QObject::tr("Orbit, %1").arg((*si).m_ShipLL.ToString());

			if((*si).m_IsNew)
				note += ", New";

			rpt += QObject::tr("<tr><td style=\"vertical-align: top;\"><img height=\"32\" align=\"left\" src=\"qrc%1\"> %2</td>"
					"<td style=\"vertical-align: top;\">%3</td></tr>")
				.arg(icon)
				.arg((*si).m_SpotDesc)
				.arg(note);
		}
		rpt += "</tbody></table><br>";
		rpt += "</p>";
	}

	// Make a map of the facilities table contents so it will be alphabetized.
	QMap<QString, AuCityIndustryIID> facil;
	for(AuCityIndustryHash::const_iterator ii = m_Industries.cbegin(); ii != m_Industries.cend(); ii++)
	{
		// Don't show zeros. Shouldn't exist any more now that it's a hash and not an array.
		if((*ii).Levels() == 0)
			continue;
		const DgData& gd = g_Au.LockDataForRead();
		QString ind = gd.IndustryName(ii.key());
		g_Au.UnlockData();
		facil.insert(ind, AuCityIndustryIID(*ii, ii.key()));
	}
	rpt += QObject::tr("<b>FACILITIES</b><p style=\"margin-left: 10px;\">");
	if(!facil.isEmpty())
	{
		rpt += QObject::tr("Lounges: %L1<br>").arg(m_Lounges);
		rpt += QObject::tr("Retail Stores: %L1<br>").arg(m_StoresRetail);

		rpt += QObject::tr("<table width=\"100%\" border=\"0\" cellpadding=\"2\" "
			"cellspacing=\"0\" style=\"background-color: rgb(7, 62, 94);\"><tbody>");
		rpt += QObject::tr("<tr><td><b>Name</b></td><td><b></b></td>"
				"<td></td>"
				"<td><b>Name</b></td><td><b></b></td></tr>");
		for(QMap<QString, AuCityIndustryIID>::const_iterator mit = facil.cbegin(); mit != facil.cend(); mit++)
		{
			QString sFacility = mit.key();
			int levelsTotal = (*mit).Levels();
			int levelsInUse = (*mit).LevelsInUse();
			QString levels;
			if(DbIndustry::DoesManufacture((*mit).IndustryId()))
				levels = QString("%L1, %L2 in use").arg(levelsTotal).arg(levelsInUse);
			else
				levels = QString("%L1").arg(levelsTotal);
			INDUSTRYID iid0 = (*mit).IndustryId();
			mit++;
			if(mit != facil.cend())
			{
				// We have two columns to list in this row, could be the last row but it doesn't matter.
				int levelsTotal1 = (*mit).Levels();
				int levelsInUse1 = (*mit).LevelsInUse();
				QString levels1;
				if(DbIndustry::DoesManufacture((*mit).IndustryId()))
					levels1 = QString("%L1, %L2 in use").arg(levelsTotal1).arg(levelsInUse1);
				else
					levels1 = QString("%L1").arg(levelsTotal1);

				rpt += QObject::tr("<tr>"
						"<td valign=\"top\"><img height=\"32\" align=\"left\" src=\"qrc%7\"> %1</td>"
						"<td valign=\"top\">%3</td>"
						"<td valign=\"top\"></td>"
						"<td valign=\"top\"><img height=\"32\" align=\"left\" src=\"qrc%8\"> %4</td>"
						"<td valign=\"top\">%6</td>"
						"</tr>")
					.arg(sFacility)
					.arg(levels)
					.arg(mit.key())
					.arg(levels1)
					.arg(DbIndustry::BitmapName(iid0))
					.arg(DbIndustry::BitmapName((*mit).IndustryId()));
			}
			else
			{
				// We only have one column to list in this last row.
				rpt += QObject::tr("<tr>"
						"<td valign=\"top\"><img height=\"32\" align=\"left\" src=\"qrc%4\"> %1</td>"
						"<td valign=\"top\">%3</td>"
						"</tr>")
					.arg(sFacility)
					.arg(levels)
					.arg(DbIndustry::BitmapName(iid0));
				break;		// Break out because we know we're done
			}
		}
		rpt += "</tbody></table><br>";
	}
	rpt += "</p>";

	// Vehicles
	if(!m_Vehicles.empty())
	{
		rpt += QObject::tr("<b>VEHICLES</b><p style=\"margin-left: 10px;\">");
		rpt += QObject::tr("Vehicles available to construction and manufacturing processes.<br>");

		// Make a table of the vehicles available.
		rpt += QObject::tr("<table border=\"0\" cellpadding=\"2\" "
			"cellspacing=\"0\" style=\"background-color: rgb(39, 64, 118);\"><tbody>");
		rpt += QObject::tr("<tr>"
				"<td><b>Vehicle</b></td>"
				"</tr>");
		for(QList<COMMODITYID>::const_iterator vi = m_Vehicles.cbegin(); vi != m_Vehicles.cend(); vi++)
			rpt += QObject::tr("<tr><td valign=\"top\"><img height=\"32\" align=\"middle\" src=\"qrc%1\"> %2</td></tr>")
				.arg(g_Au.CommodityBitmapName(*vi)).arg(g_Au.CommodityName(*vi));
		rpt += "</tbody></table><br>";
		rpt += "</p>";
	}

	// Inventory
	if(!m_Inventory->empty() || !m_Tools->empty())
	{
		rpt += QObject::tr("<b>INVENTORY</b><p style=\"margin-left: 10px;\">");
		if(m_PopOrig)
		{
			if(!m_HasBreathableAtmo)
			{
				rpt += QObject::tr("Air: %L1 total units in stock<br>").arg(m_AirSupply);
				rpt += QObject::tr("Air Consumption: %L1 units per report period<br>").arg(m_PopOrig);
			}
			rpt += QObject::tr("Food: %L1 total nutrition value in stock<br>").arg(m_FoodUnits);
			rpt += QObject::tr("Food Consumption: %L1 nutrition value per report period<br>").arg(m_FoodPerMeal * m_PopOrig);
		}
		//rpt += QObject::tr("Storage Capacity Per Commodity %1<br>").arg(m_iStorage);
		// Make a map of the inventory table contents so it will be alphabetized.
		QString qual;
		QMultiMap<QString, QMap<CommoditySubKey, QString> > inven;		// Key is commodity name.
		QMultiMap<QString, QMap<CommoditySubKey, QString> > invArmor;	// Key is commodity name.
		QMultiMap<QString, QMap<CommoditySubKey, QString> > invFood;	// Key is commodity name.
		QMultiMap<QString, QMap<CommoditySubKey, QString> > invMat;		// Key is commodity name.
		QMultiMap<QString, QMap<CommoditySubKey, QString> > invMed;		// Key is commodity name.
		QMultiMap<QString, QMap<CommoditySubKey, QString> > invTool;	// Key is commodity name.
		QMultiMap<QString, QMap<CommoditySubKey, QString> > invToolInUse;	// Key is commodity name.
		QMultiMap<QString, QMap<CommoditySubKey, QString> > invWeapon;	// Key is commodity name.
		QMap<QString, QString> bitmap;		// Key is commodity name.

		/*
		{
			QString fileName = QString("Inv%1.bin").arg(m_Inventory->size());
			QFile file(fileName);
			if(file.open(QFile::WriteOnly))
			{
				QDataStream ds(&file);
				ds.setVersion(QDataStream::Qt_5_7);
				ds << *m_Inventory;
				file.close();
			}
			qDebug() << "wrote" << fileName;
		}
		*/

		for(AuItemHash::const_iterator it = m_Inventory->begin(); it != m_Inventory->end(); it++)
		{
			if((*it).second->CommodityId() == CID_Electricity)
				continue;	// Electricity is listed as power reserve.

			// Select the appropriate inventory list category.
			QMultiMap<QString, QMap<CommoditySubKey, QString> >* pi;
			if((*it).second->IsTool())	// Check before IsWeapon
				pi = &invTool;
			else if((*it).second->IsConstructionMaterial() || (*it).second->IsSparePart() || (*it).second->IsFuel())
				pi = &invMat;
			else if((*it).second->IsAmmo() || (*it).second->IsWeaponByDesign())
				pi = &invWeapon;
			else if((*it).second->IsArmorHelmetClothingOrHat())
				pi = &invArmor;
			else if((*it).second->CanBeEaten())
				pi = &invFood;
			else if((*it).second->IsMedical())
				pi = &invMed;
			else
				pi = &inven;

			QString commod = (*it).second->ListName();
			if(!bitmap.contains(commod))
				bitmap.insert(commod, QString(g_Au.CommodityBitmapName((*it).second->CommodityId())));

			QMultiMap<QString, QMap<CommoditySubKey, QString> >::iterator iit = pi->find(commod);
			if(iit == pi->end())
				iit = pi->insert(commod, QMap<CommoditySubKey, QString>());	// Create a nu entry for the commodity.

			if(iit != pi->end())
			{
				CommoditySubKey key((*it).second->Quality(), (*it).second->EmpireOfOrigin());
				if((*it).second->EmpireOfOrigin())
				{
					// The item has a known empire of origin.
					QString empireOfOrigin;
					if((*it).second->EmpireOfOrigin() != EmpireId(0))
					{
						// Item was created in another empire.
						const DgData& gd = g_Au.LockDataForRead();
						gd.EmpireName((*it).second->EmpireOfOrigin(), empireOfOrigin);
						g_Au.UnlockData();
					}
					if(!empireOfOrigin.isEmpty())
					{
						// Item was created in another empire.
						(*iit).insert(key, QObject::tr("%1 Q%2 Made in %3").arg((*it).second->Count())
								.arg((*it).second->Quality())
								.arg(empireOfOrigin));
					}
					else
					{
						// Item was created in my empire.
						(*iit).insert(key, QObject::tr("%1 Q%2").arg((*it).second->Count()).arg((*it).second->Quality()));
					}
				}
				else
				{
					// The item does not have a known empire of origin. Probably hand made or a carcass.
					(*iit).insert(key, QObject::tr("<i>%1 Q%2</i>").arg((*it).second->Count()).arg((*it).second->Quality()));
				}
			}
		}
		for(AuItemHash::const_iterator it = m_Tools->begin(); it != m_Tools->end(); it++)
		{
			if((*it).second->CommodityId() == CID_Electricity)
				continue;	// Electricity is listed as power reserve.

			// Select the appropriate inventory list category.
			QString commod = (*it).second->ListName();
			if(!bitmap.contains(commod))
				bitmap.insert(commod, QString(g_Au.CommodityBitmapName((*it).second->CommodityId())));

			QMultiMap<QString, QMap<CommoditySubKey, QString> >::iterator iit = invToolInUse.find(commod);
			if(iit == invToolInUse.end())
				iit = invToolInUse.insert(commod, QMap<CommoditySubKey, QString>());	// Create a nu entry for the commodity.

			if(iit != invToolInUse.end())
			{
				CommoditySubKey key((*it).second->Quality(), (*it).second->EmpireOfOrigin());
				if((*it).second->EmpireOfOrigin())
				{
					// The item has a known empire of origin.
					QString empireOfOrigin;
					if((*it).second->EmpireOfOrigin() != EmpireId(0))
					{
						// Item was created in another empire.
						const DgData& gd = g_Au.LockDataForRead();
						gd.EmpireName((*it).second->EmpireOfOrigin(), empireOfOrigin);
						g_Au.UnlockData();
					}
					if(!empireOfOrigin.isEmpty())
					{
						// Item was created in another empire.
						(*iit).insert(key, QObject::tr("%1 Q%2 Made in %3")
								.arg((*it).second->Count())
								.arg((*it).second->Quality())
								.arg(empireOfOrigin));
					}
					else
					{
						// Item was created in my empire.
						(*iit).insert(key, QObject::tr("%1 Q%2").arg((*it).second->Count()).arg((*it).second->Quality()));
					}
				}
				else
				{
					// The item does not have a known empire of origin. Probably hand made or a carcass.
					(*iit).insert(key, QObject::tr("<i>%1 Q%2</i>").arg((*it).second->Count()).arg((*it).second->Quality()));
				}
			}
		}

		// Create a table for each category of commodities.
		// Food and Drinks
		if(!invFood.empty())
		{
			rpt += QObject::tr("<table border=\"0\" cellpadding=\"2\" "
				"cellspacing=\"0\" style=\"background-color: rgb(49, 36, 94);\"><tbody>");
			rpt += QObject::tr("<tr>"
					"<td><b>Food and Drinks</b></td>"
					"<td></td>"
					"</tr>");
			for(QMultiMap<QString, QMap<CommoditySubKey, QString> >::const_iterator iit = invFood.cbegin();
					iit != invFood.cend(); iit++)
			{
				QString scommod;
				for(QMap<CommoditySubKey, QString>::const_iterator sci = (*iit).cbegin(); sci != (*iit).cend(); sci++)
					if(scommod.isEmpty())
						scommod = *sci;
					else
						scommod += "<br>" + *sci;
				rpt += QObject::tr("<tr><td valign=\"top\"><img height=\"16\" align=\"middle\" src=\"qrc%3\"> %1</td><td valign=\"top\">%2</td></tr>")
					.arg(iit.key())
					.arg(scommod)
					.arg(bitmap[iit.key()]);
			}
			rpt += "</tbody></table><br>";
		}

		// Tools
		if(!invToolInUse.empty())
		{
			rpt += QObject::tr("<table border=\"0\" cellpadding=\"2\" "
				"cellspacing=\"0\" style=\"background-color: rgb(49, 36, 94);\"><tbody>");
			rpt += QObject::tr("<tr>"
					"<td><b>Tools In Use</b></td>"
					"<td></td>"
					"</tr>");
			for(QMultiMap<QString, QMap<CommoditySubKey, QString> >::const_iterator iit = invToolInUse.cbegin();
					iit != invToolInUse.cend(); iit++)
			{
				QString scommod;
				for(QMap<CommoditySubKey, QString>::const_iterator sci = (*iit).cbegin(); sci != (*iit).cend(); sci++)
					if(scommod.isEmpty())
						scommod = *sci;
					else
						scommod += "<br>" + *sci;
				rpt += QObject::tr("<tr><td valign=\"top\"><img height=\"16\" align=\"middle\" src=\"qrc%3\"> %1</td><td valign=\"top\">%2</td></tr>")
					.arg(iit.key())
					.arg(scommod)
					.arg(bitmap[iit.key()]);
			}
			rpt += "</tbody></table><br>";
		}
		if(!invTool.empty())
		{
			rpt += QObject::tr("<table border=\"0\" cellpadding=\"2\" "
				"cellspacing=\"0\" style=\"background-color: rgb(49, 36, 94);\"><tbody>");
			rpt += QObject::tr("<tr>"
					"<td><b>Tools</b></td>"
					"<td></td>"
					"</tr>");
			for(QMultiMap<QString, QMap<CommoditySubKey, QString> >::const_iterator iit = invTool.cbegin();
					iit != invTool.cend(); iit++)
			{
				QString scommod;
				for(QMap<CommoditySubKey, QString>::const_iterator sci = (*iit).cbegin(); sci != (*iit).cend(); sci++)
					if(scommod.isEmpty())
						scommod = *sci;
					else
						scommod += "<br>" + *sci;
				rpt += QObject::tr("<tr><td valign=\"top\"><img height=\"16\" align=\"middle\" src=\"qrc%3\"> %1</td><td valign=\"top\">%2</td></tr>")
					.arg(iit.key())
					.arg(scommod)
					.arg(bitmap[iit.key()]);
			}
			rpt += "</tbody></table><br>";
		}

		// Construction Materials, Spare Parts and Fuel
		if(!invMat.empty())
		{
			rpt += QObject::tr("<table border=\"0\" cellpadding=\"2\" "
				"cellspacing=\"0\" style=\"background-color: rgb(49, 36, 94);\"><tbody>");
			rpt += QObject::tr("<tr>"
					"<td><b>Construction Materials<br>Spare Parts and Fuel</b></td>"
					"<td></td>"
					"</tr>");
			for(QMultiMap<QString, QMap<CommoditySubKey, QString> >::const_iterator iit = invMat.cbegin();
					iit != invMat.cend(); iit++)
			{
				QString scommod;
				for(QMap<CommoditySubKey, QString>::const_iterator sci = (*iit).cbegin(); sci != (*iit).cend(); sci++)
					if(scommod.isEmpty())
						scommod = *sci;
					else
						scommod += "<br>" + *sci;
				rpt += QObject::tr("<tr><td valign=\"top\"><img height=\"16\" align=\"middle\" src=\"qrc%3\"> %1</td><td valign=\"top\">%2</td></tr>")
					.arg(iit.key())
					.arg(scommod)
					.arg(bitmap[iit.key()]);
			}
			rpt += "</tbody></table><br>";
		}

		// Medical
		if(!invMed.empty())
		{
			rpt += QObject::tr("<table border=\"0\" cellpadding=\"2\" "
				"cellspacing=\"0\" style=\"background-color: rgb(49, 36, 94);\"><tbody>");
			rpt += QObject::tr("<tr>"
					"<td><b>Medical</b></td>"
					"<td></td>"
					"</tr>");
			for(QMultiMap<QString, QMap<CommoditySubKey, QString> >::const_iterator iit = invMed.cbegin();
					iit != invMed.cend(); iit++)
			{
				QString scommod;
				for(QMap<CommoditySubKey, QString>::const_iterator sci = (*iit).cbegin(); sci != (*iit).cend(); sci++)
					if(scommod.isEmpty())
						scommod = *sci;
					else
						scommod += "<br>" + *sci;
				rpt += QObject::tr("<tr><td valign=\"top\"><img height=\"16\" align=\"middle\" src=\"qrc%3\"> %1</td><td valign=\"top\">%2</td></tr>")
					.arg(iit.key())
					.arg(scommod)
					.arg(bitmap[iit.key()]);
			}
			rpt += "</tbody></table><br>";
		}

		// Armor and Clothing
		if(!invArmor.empty())
		{
			rpt += QObject::tr("<table border=\"0\" cellpadding=\"2\" "
				"cellspacing=\"0\" style=\"background-color: rgb(49, 36, 94);\"><tbody>");
			rpt += QObject::tr("<tr>"
					"<td><b>Armor and Clothing</b></td>"
					"<td></td>"
					"</tr>");
			for(QMultiMap<QString, QMap<CommoditySubKey, QString> >::const_iterator iit = invArmor.cbegin();
					iit != invArmor.cend(); iit++)
			{
				QString scommod;
				for(QMap<CommoditySubKey, QString>::const_iterator sci = (*iit).cbegin(); sci != (*iit).cend(); sci++)
					if(scommod.isEmpty())
						scommod = *sci;
					else
						scommod += "<br>" + *sci;
				rpt += QObject::tr("<tr><td valign=\"top\"><img height=\"16\" align=\"middle\" src=\"qrc%3\"> %1</td><td valign=\"top\">%2</td></tr>")
					.arg(iit.key())
					.arg(scommod)
					.arg(bitmap[iit.key()]);
			}
			rpt += "</tbody></table><br>";
		}

		// Weapons and Ammunition
		if(!invWeapon.empty())
		{
			rpt += QObject::tr("<table border=\"0\" cellpadding=\"2\" "
				"cellspacing=\"0\" style=\"background-color: rgb(49, 36, 94);\"><tbody>");
			rpt += QObject::tr("<tr>"
					"<td><b>Weapons and Ammunition</b></td>"
					"<td></td>"
					"</tr>");
			for(QMultiMap<QString, QMap<CommoditySubKey, QString> >::const_iterator iit = invWeapon.cbegin();
					iit != invWeapon.cend(); iit++)
			{
				QString scommod;
				for(QMap<CommoditySubKey, QString>::const_iterator sci = (*iit).cbegin(); sci != (*iit).cend(); sci++)
					if(scommod.isEmpty())
						scommod = *sci;
					else
						scommod += "<br>" + *sci;
				rpt += QObject::tr("<tr><td valign=\"top\"><img height=\"16\" align=\"middle\" src=\"qrc%3\"> %1</td><td valign=\"top\">%2</td></tr>")
					.arg(iit.key())
					.arg(scommod)
					.arg(bitmap[iit.key()]);
			}
			rpt += "</tbody></table><br>";
		}

		// Miscellaneous
		if(!inven.empty())
		{
			rpt += QObject::tr("<table border=\"0\" cellpadding=\"2\" "
				"cellspacing=\"0\" style=\"background-color: rgb(49, 36, 94);\"><tbody>");
			rpt += QObject::tr("<tr>"
					"<td><b>Miscellaneous</b></td>"
					"<td></td>"
					"</tr>");
			for(QMultiMap<QString, QMap<CommoditySubKey, QString> >::const_iterator iit = inven.cbegin();
					iit != inven.cend(); iit++)
			{
				QString scommod;
				for(QMap<CommoditySubKey, QString>::const_iterator sci = (*iit).cbegin(); sci != (*iit).cend(); sci++)
					if(scommod.isEmpty())
						scommod = *sci;
					else
						scommod += "<br>" + *sci;
				rpt += QObject::tr("<tr><td valign=\"top\"><img height=\"16\" align=\"middle\" src=\"qrc%3\"> %1</td><td valign=\"top\">%2</td></tr>")
					.arg(iit.key())
					.arg(scommod)
					.arg(bitmap[iit.key()]);
			}
			rpt += "</tbody></table><br>";
		}
		rpt += "</p>";
	}

	// Research and Development
	// Make a map of the patent development processes in progress so it will be alphabetized.
	if(!m_PatentRnD.isEmpty())
	{
		rpt += QObject::tr("<b>RESEARCH AND DEVELOPMENT</b><p style=\"margin-left: 10px;\">");
		rpt += QObject::tr("<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" style=\"background-color: rgb(41, 41, 82);\">"
				"<tbody>");
		rpt += QObject::tr("<tr>"
				"<td><b>Technology</b></td>"
				"<td><b>Processes</b></td>"
				"</tr>");
		for(QMap<QString, quint16>::const_iterator ri = m_PatentRnD.cbegin(); ri != m_PatentRnD.cend(); ri++)
		{
			rpt += QString("<tr>"
					"<td>%1</td>"
					"<td>%2</td>"
					"</tr>")
				.arg(ri.key()).arg(ri.value());
		}
		rpt += "</tbody></table><br>";
		rpt += "</p>";
	}

	// Spacecraft Manufacturing Potential
	if(BuildingLevels(IID_SpacecraftFactory))
	{
		class AuShipComponent
		{
		public:
			COMMODITYID		m_Component;	// Component of spacecraft manufacture.
			COMMODITYID		m_Tool;			// Tool needed to fix a component.
			INDUSTRYID		m_IID0;			// Industry that makes the component.
			INDUSTRYID		m_IID1;			// Industry that makes the component.

			AuShipComponent(COMMODITYID component, COMMODITYID tool, INDUSTRYID iid0, INDUSTRYID iid1=IID_None)
			{
				m_Component = component;
				m_Tool = tool;
				m_IID0 = iid0;
				m_IID1 = iid1;
			}
		};
		AuShipComponent shipComponents[] =
		{
			AuShipComponent(CID_Adamantine, CID_Hammer, IID_Smelter),
			AuShipComponent(CID_AtmosphereCondenserBayModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_BallisticWeaponBayModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_BallisticWeaponTurretModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_CapacitorModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_CloakingShieldModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_CommandAndControlModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_Computer, CID_ScrewDriver, IID_ComputerFactory),
			AuShipComponent(CID_CyberneticsMedicalModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_ElectronicPart, CID_SolderIron, IID_ElectronicsFactory),
			AuShipComponent(CID_EnergyShieldModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_EnergyWeaponBayModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_EnergyWeaponTurretModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_GeneticsMedicalModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_GravityDriveModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_HydrogenPowerPlantModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_IoTube, CID_Clamp, IID_ElectronicsFactory),
			AuShipComponent(CID_KineticShieldModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_LifeSupportGravityModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_LifeSupportModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_Magmium, CID_Hammer, IID_Smelter),
			AuShipComponent(CID_MechanicalPart, CID_Wrench, IID_MachineShop, IID_PlasticsFactory),
			AuShipComponent(CID_Metal, CID_Hammer, IID_Smelter),
			AuShipComponent(CID_ParticleCollectorBayModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_PersonnelTransporterModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_Plastic, CID_GlueGun, IID_PlasticsFactory),
			AuShipComponent(CID_ProspectorBayModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_RocketDriveModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_SensorModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_SiphonBayModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_SurgeryMedicalModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_Textiles, CID_SewingNeedle, IID_TextileMill),
			AuShipComponent(CID_TractorBeamBayModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_VehicleTransporterModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_Vulcium, CID_Hammer, IID_Smelter),
			AuShipComponent(CID_Warp1FTLDriveModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_Warp2FTLDriveModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_Warp3FTLDriveModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_Warp4FTLDriveModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_Warp5FTLDriveModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_Warp6FTLDriveModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_Warp7FTLDriveModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_Warp8FTLDriveModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_Warp9FTLDriveModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_WeaponDeploymentBayModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_WeaponLauncherBayModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_WeaponLauncherTurretModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_WildcatterBayModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_WormholePFTLDriveModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_WormholePNFTLDriveModule, CID_FittingWrench, IID_AirportRepairShop),
			AuShipComponent(CID_WormholePNNFTLDriveModule, CID_FittingWrench, IID_AirportRepairShop),
		};
		int shipComponentCount = sizeof(shipComponents)/sizeof(AuShipComponent);

		rpt += QObject::tr("<b>SPACECRAFT MANUFACTURING POTENTIAL</b><p style=\"margin-left: 10px;\">");
		if(!m_Inventory->empty())
		{
			rpt += QObject::tr("<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" style=\"background-color: "
					"rgb(36, 48, 94);\"><tbody>");
			rpt += QObject::tr("<tr>"
					"<td><b>Component</b></td>"
					"<td><b>Q</b></td>"
					"<td><b>Industry</b></td>"
					"<td><b>Tool</b></td>"
					"</tr>");
			const DgData& gd = g_Au.LockDataForRead();
			for(int scomp = 0; scomp < shipComponentCount; scomp++)
			{
				rpt += QString("<tr>");

				// Ship Component
				Q_ASSERT(shipComponents[scomp].m_Component != CID_Invalid);
				AuItemHash::const_iterator compI = m_Inventory->FindBest(shipComponents[scomp].m_Component);
				if(compI != m_Inventory->end())
					rpt += QString("<td valign=\"top\"><img height=\"16\" align=\"middle\" src=\"qrc%1\"> %2</td><td valign=\"top\">%3</td>")
						.arg(g_Au.CommodityBitmapName(shipComponents[scomp].m_Component))
						.arg(g_Au.CommodityName(shipComponents[scomp].m_Component))
						.arg((*compI).second->Quality());
				else
					rpt += QString("<td valign=\"top\"><img height=\"16\" align=\"middle\" src=\"qrc%1\"> <b>No %2</b></td><td valign=\"top\"></td>")
						.arg(g_Au.CommodityBitmapName(shipComponents[scomp].m_Component))
						.arg(g_Au.CommodityName(shipComponents[scomp].m_Component));

				// Info about the industry that makes the component.
				if(shipComponents[scomp].m_IID0 == IID_None)
				{
					// No industry makes the component. So far the only exception is antiflux particles, which are gathered by
					// spacecraft particle collector bays.
					QString s0 = g_Au.CommodityName(CID_ParticleCollectorBayModule);
					rpt += QString("<td valign=\"top\"><img height=\"16\" align=\"middle\" src=\"qrc%3\"> %1</td>")
						.arg(s0)
						.arg(":data/c_Spacecraft.png");
				}
				else if(shipComponents[scomp].m_IID1 == IID_None)
				{
					// The component is only produced at one industry.
					QString s0;
					if(BuildingLevels(shipComponents[scomp].m_IID0))
						s0 = QString("<img height=\"16\" align=\"middle\" src=\"qrc%2\"> %1")
							.arg(gd.IndustryName(shipComponents[scomp].m_IID0))
							.arg(DbIndustry::BitmapName(shipComponents[scomp].m_IID0));
					else
						s0 = QObject::tr("<img height=\"16\" align=\"middle\" src=\"qrc%2\"> No %1")
							.arg(gd.IndustryName(shipComponents[scomp].m_IID0))
							.arg(DbIndustry::BitmapName(shipComponents[scomp].m_IID0));

					rpt += QString("<td valign=\"top\">%1</td>")
						.arg(s0);
				}
				else
				{
					// The component is produced at two industries.
					QString s0;
					if(BuildingLevels(shipComponents[scomp].m_IID0))
						s0 = QString("<img height=\"16\" align=\"middle\" src=\"qrc%2\"> %1")
							.arg(gd.IndustryName(shipComponents[scomp].m_IID0))
							.arg(DbIndustry::BitmapName(shipComponents[scomp].m_IID0));
					else
						s0 = QObject::tr("<img height=\"16\" align=\"middle\" src=\"qrc%2\"> No %1")
							.arg(gd.IndustryName(shipComponents[scomp].m_IID0))
							.arg(DbIndustry::BitmapName(shipComponents[scomp].m_IID0));
					QString s1;
					if(BuildingLevels(shipComponents[scomp].m_IID1))
						s1 = QString("<img height=\"16\" align=\"middle\" src=\"qrc%2\"> %1")
							.arg(gd.IndustryName(shipComponents[scomp].m_IID1))
							.arg(DbIndustry::BitmapName(shipComponents[scomp].m_IID1));
					else
						s1 = QObject::tr("<img height=\"16\" align=\"middle\" src=\"qrc%2\"> No %1")
							.arg(gd.IndustryName(shipComponents[scomp].m_IID1))
							.arg(DbIndustry::BitmapName(shipComponents[scomp].m_IID1));

					rpt += QString("<td valign=\"top\">%1<br>%2</td>")
						.arg(s0)
						.arg(s1);
				}

				// Tool
				if(m_Inventory->contains(shipComponents[scomp].m_Tool))
					rpt += QString("<td valign=\"top\"><img height=\"16\" align=\"middle\" src=\"qrc%1\"> %2</td>")
						.arg(g_Au.CommodityBitmapName(shipComponents[scomp].m_Tool))
						.arg(g_Au.CommodityName(shipComponents[scomp].m_Tool));
				else
					rpt += QString("<td valign=\"top\"><img height=\"16\" align=\"middle\" src=\"qrc%1\"> <b>No %2</b></td>")
						.arg(g_Au.CommodityBitmapName(shipComponents[scomp].m_Tool))
						.arg(g_Au.CommodityName(shipComponents[scomp].m_Tool));

				rpt += QString("</tr>");
			}
			g_Au.UnlockData();
			rpt += "</tbody></table><br>";
		}
		rpt += "</p>";
	}

	// Patents
	if(!m_Patents.empty())
	{
		rpt += QObject::tr("<b>PATENTS</b><p style=\"margin-left: 10px;\">");
		rpt += QObject::tr("Patents available to manufacturing processes.<br>");

		// Make a table of the vehicles available.
		rpt += QObject::tr("<table border=\"0\" cellpadding=\"2\" "
			"cellspacing=\"0\" style=\"background-color: rgb(41, 41, 82);\"><tbody>");
		rpt += QObject::tr("<tr>"
				"<td><b>Patent</b></td>"
				"</tr>");
		QMap<QString, COMMODITYID> sortedPatents;
		for(QHash<COMMODITYID, quint8>::const_iterator vi = m_Patents.cbegin(); vi != m_Patents.cend(); vi++)
			if(!DbCommodity::IsInvalid(vi.key()))
				sortedPatents.insert(g_Au.CommodityName(vi.key()), vi.key());
		for(QMap<QString, COMMODITYID>::const_iterator si = sortedPatents.cbegin(); si != sortedPatents.cend(); si++)
			rpt += QObject::tr("<tr><td valign=\"top\"><img height=\"32\" align=\"middle\" src=\"qrc%1\"> %2 Q%3</td></tr>")
				.arg(g_Au.CommodityBitmapName(*si))
				.arg(si.key())
				.arg(m_Patents.PatentQuality(*si));
		rpt += "</tbody></table><br>";
		rpt += "</p>";
	}

	// Insert distress information at the head of the report.
	if(!m_DistressRpt.isEmpty())
		rpt.insert(distressIdx, m_DistressRpt);
	return rpt;
}

QString AuCityReport::ReportHQ(const AuCityReport& prevReport, bool mailFormat) const
{
	QString rpt;	// This will be our return value.
	if(mailFormat)
	{
		// Mail format expects an encoded date-time value for localization.
		// It also does not expect an icon at the beginning.
		rpt = g_Au.EncodeUTCString(m_WhenCompiled) + "<br><br>";
	}
	else
	{
		// Start with icon and date-time stamp.
		QString typeIcon;
		switch(m_MessageType)
		{
		case AuMailMsg::MSG_CityStatusReport:
		default:
			typeIcon = ":data/MailCityStatus.png";
			break;
		case AuMailMsg::MSG_CityStatusReportInfo:
			typeIcon = ":data/MailCityStatusInfo.png";
			break;
		case AuMailMsg::MSG_CityDistressReport:
			typeIcon = ":data/MailCityDistress.png";
			break;
		case AuMailMsg::MSG_CityFinalDecayReport:
			typeIcon = ":data/MailCityDecayed.png";
			break;
		}

		rpt += QString("<img height=\"32\" src=\"qrc%1\"><br>%2<br><big><b>%3</b></big><p>")
			.arg(typeIcon)
			.arg(m_WhenCompiled.UTC().toLocalTime().toString(Qt::SystemLocaleShortDate))
			.arg(Title());
	}

	// Information about the city.
	rpt += "<b>" + m_Name + "</b><p style=\"margin-left: 10px;\">";
	rpt += m_PlanetName + ", " + m_PlanetDesc + "<br>";		// Planet
	rpt += m_SystemName + ' ' + m_SystemDesc + "<br>";
	rpt += m_SectorName + ' ' + m_SectorDesc + "<br>";	// Sector
	rpt += QString("%1<br>").arg(GalaxyName((GALAXY)m_Galaxy));

	// Longitude and latitude coordinates.
	rpt += QObject::tr("Headquarters at %1").arg(m_LL.ToString());
	if(!m_FounderId.IsNull())
		rpt += QObject::tr("<br>Founder %1 <i>%2</i>").arg(m_FounderName).arg(m_FounderId.StringId());
	rpt += "</p>";

	// When reporting distress situations, we like to see the reasons right at the top of the  This index will
	// help us to insert it there later.
	int distressIdx = rpt.length();	// Insertion point for the distress portion of the report, if any.

	// When a city is more than a week old, it is subject to decay.
	// That is, one or more developed tiles per city report may be be deleted, saving the town square for last.
	// When the town square is deleted, this city atom dies and the terrain itself becomes subject to decay.
	int decayMod = 0;
	if(HasDecay())
	{
		rpt += QObject::tr("<b>DECAY</b><p style=\"margin-left: 10px;\">");
		rpt += QObject::tr("The base is abandoned.<br>");
		for(int decay=0; decay<3; decay++)
		{
			if(m_Decayed[decay] != IID_None)
			{
				const DgData& gd = g_Au.LockDataForRead();
				QString ind = gd.IndustryName(m_Decayed[decay]);
				g_Au.UnlockData();
				rpt += QObject::tr("%1 crumbled away.<br>").arg(ind);
				decayMod++;
			}
		}
		if(rpt.right(4) == "<br>")
			rpt = rpt.left(rpt.count() - 4);	// Drop the trailing <br>.
		rpt += "</p>";
	}

	// City Event Log
	QString eventRpt = m_EventLog->ReportHtml("EVENT LOG", AuTime());
	if(!eventRpt.isEmpty())
	{
		if(mailFormat)
			rpt += eventRpt;							// Mail decodes UTC strings at the receiving end.
		else
			rpt += g_Au.DecodeUTCStrings(eventRpt);		// Report must decode UTC strings now.
	}

	// Get info about the locals.
	bool isAlienCity = false;									// true if this city is inhabited by aliens on a habitable world
	if(m_IsHabitablePlanet && !m_Citizens.empty() && Citizen().IsAlien())
		isAlienCity = true;

	int worldOverpopPenalty = 0;
	int hungerPenalty = 0;
	//int hospitalPenalty = 0;
	//float loyalty = m_LoyaltyOrig;

	// Determine the number of buildings in the city, after decay.
	AuCityIndustryHash industries(prevReport.m_Industries);	// Copy previous values to see the change.

	// Tally homes.
	unsigned int homesTotal = m_HomesLg + m_HomesMd + m_HomesSm;

	// Adjust satisfaction based on various factors.
	rpt += QObject::tr("<b>MORALE %1</b><p style=\"margin-left: 10px;\">").arg(MoraleHQ());
	int iSatChange = 0;
	//int req_hospitals = 0;
	if(m_PopOrig)
	{
		// Air/Power Consumption
		bool gasping = false;
		if(!m_HasBreathableAtmo)
		{
			// Living in harsh conditions erodes satisfaction.
			if(!HasPower())
			{
				int upen = 2;
				iSatChange -= upen;
				rpt += QObject::tr("-%1 Harsh Environment Without Power. Power Needed.<br>").arg(upen);
			}
			else
			{
				int upen = 1;
				iSatChange -= upen;
				rpt += QObject::tr("-%1 Harsh Environment.<br>").arg(upen);
			}

			// Air Consumption
			const quint32 airNeeded = m_PopOrig;	// 1 unit of air per citizen per report
			if(!m_AirSupply)
			{
				// No air to go around.
				int upen = 5;
				quint32 suffocated = m_PopOrig / 10;	// 10% of the remaining population suffocates to death.
				if(suffocated < 4)
					suffocated = 4;				// At least 4 citizens suffocated to death.
				if(suffocated > m_PopOrig)
					suffocated = m_PopOrig;	// The last of the population suffocated to death.
				if(m_AirConsumed < 0)
				{
					if(suffocated == 1)
						rpt += QObject::tr("-%1 Suffocation. One troop will suffocate to death. Air is needed.<br>")
							.arg(upen);
					else
						rpt += QObject::tr("-%1 Suffocation. %L2 troops will suffocate to death. Air is needed.<br>")
							.arg(upen).arg(suffocated);
				}
				else
				{
					if(suffocated == 1)
						rpt += QObject::tr("-%1 Suffocation. One troop suffocated to death. Air is needed.<br>")
							.arg(upen);
					else
						rpt += QObject::tr("-%1 Suffocation. %L2 troops suffocated to death. Air is needed.<br>")
							.arg(upen).arg(suffocated);
				}
				iSatChange -= upen;
				gasping = true;
			}
			else
			{
				if(m_AirSupply < airNeeded)
				{
					// Not enough air to go around.
					int upen;
					if(m_AirSupply < airNeeded/2)
						upen = 2;
					else
						upen = 1;
					rpt += QObject::tr("-%1 Gasping. More air is needed.<br>").arg(upen);
					iSatChange -= upen;
					gasping = true;
				}
			}
		}

		// Food Consumption
		bool hungry = false;
		if(!m_FoodTypes)
		{
			// No food to go around.
			hungerPenalty = 5;
			quint32 starved = m_PopOrig / 10;	// 10% of the remaining population starves to death.
			if(starved < 4)
				starved = 4;					// At least 4 citizens starved to death.
			if(starved > m_PopOrig)
				starved = m_PopOrig;			// The last of the population starved to death.
			if(m_AirConsumed < 0)
			{
				// Air consumption is unknown when this is an intermediate report, of present conditions, not a regular city cycle
				// report of the state at the end of each city cycle.
				if(starved == 1)
					rpt += QObject::tr("-%1 Starvation. One troop will starve to death. Food is needed.<br>")
						.arg(hungerPenalty);
				else
					rpt += QObject::tr("-%1 Starvation. %L2 troops will starve to death. Food is needed.<br>")
						.arg(hungerPenalty)
						.arg(starved);
			}
			else
			{
				if(starved == 1)
					rpt += QObject::tr("-%1 Starvation. One troop starved to death. Food is needed.<br>")
						.arg(hungerPenalty);
				else
					rpt += QObject::tr("-%1 Starvation. %L2 troops starved to death. Food is needed.<br>")
						.arg(hungerPenalty)
						.arg(starved);
			}
			iSatChange -= hungerPenalty;
			hungry = true;
		}
		else if(!m_AteEnoughFood)
		{
			// Not enough food to go around.
			// People don't care about food variety when there isn't enough to go around.
			hungerPenalty = 2;
			rpt += QObject::tr("-%1 Hunger. More food is needed.<br>").arg(hungerPenalty);
			iSatChange -= hungerPenalty;
			hungry = true;
		}
		else
		{
			// Food variety improves satisfaction.
			// People don't care about food variety when there isn't enough to go around.
			int foodMod = m_FoodTypes/4;	// Generally 1 point per 4 food types.
			if(foodMod > 2)
				foodMod = 2;				// max +2 food variety bonus
			if(foodMod)
			{
				iSatChange += foodMod;
				rpt += QObject::tr("+%1 Food Variety. %2 food types.<br>").arg(foodMod).arg(m_FoodTypes);
			}
		}

		// Founder status affects satisfaction.
		switch(m_FounderStatus)
		{
		case FOUNDER_Unknown:
		case FOUNDER_FreeOrDead:
		case FOUNDER_Vassal:
		case FOUNDER_NoEmpire:
		case FOUNDER_NoCity:
		case FOUNDER_NoFounder:
		case FOUNDER_BadStance:
			break;
		case FOUNDER_Empire:
			iSatChange += 2;
			rpt += QObject::tr("+2 Founder is a famous city planner in our empire.<br>");
			break;
		case FOUNDER_Friend:
			rpt += QObject::tr("+1 Founder remains an esteemed friend of our empire.<br>");
			iSatChange += 1;
			break;
		case FOUNDER_Neutral:
			rpt += QObject::tr("-1 Forsaken. Our empire is neutral to the founder's empire.<br>");
			iSatChange -= 1;
			break;
		case FOUNDER_Enemy:
			rpt += QObject::tr("-2 Occupied. Our empire is an enemy of the founder.<br>");
			iSatChange -= 2;
			break;
		}

		// Living in unwelcome conditions erodes satisfaction.
		if(isAlienCity)
		{
			int upen = 1;
			iSatChange -= upen;
			rpt += QObject::tr("-%1 Alien Invader.<br>").arg(upen);
		}

		// World overpopulation reduces morale.
		if(m_ZonePop > m_MaxPop/2)
		{
			// Penalty for population pressure starts at 1/2 the max population +1 and reaches -20 at the max population.
			const int penaltyAtMaxPop = 20;									// Morale penalty when population reaches world max.
			int incPenalty = m_MaxPop/2 * (100/penaltyAtMaxPop) / 100;		// Population increment for each -1 penalty.
			int overPop = m_ZonePop - m_MaxPop/2;							// Amount of overpopulation.
			worldOverpopPenalty = overPop / incPenalty;						// No upper limit, -1 per increment.
			if(worldOverpopPenalty)
			{
				iSatChange -= worldOverpopPenalty;
				rpt += QObject::tr("-%1 World Overpopulation. World population is %L2.<br>")
					.arg(worldOverpopPenalty).arg(m_ZonePop);
			}
		}

		if(m_PopOrig > homesTotal && !m_HasDestroyedBuildings)
		{
			// Penalty for homelessness when not war induced.
			int overPop = m_PopOrig - homesTotal;
			int homelessnessPenalty = (overPop - (50 * BuildingLevels(IID_BaseAirTerminal)));	// Number of actual homeless.
			int vagrantTol = m_PopOrig / 100;		// Number of actual homeless per -1 to morale = 1% of population.
			if(vagrantTol > 1)
				homelessnessPenalty /= vagrantTol;
			if(homelessnessPenalty > 0)
			{
				iSatChange -= homelessnessPenalty;
				rpt += QObject::tr("-%1 Bivouac. More homes needed.<br>").arg(homelessnessPenalty);
			}
		}

		// Decay affects morale.
		if(decayMod)
		{
			iSatChange -= decayMod;
			rpt += QObject::tr("-%1 Decay.<br>").arg(decayMod);
		}

		// Abandonment reduces morale.
		int abandonMod = AbandonMod();
		iSatChange += abandonMod;
		if(abandonMod > 0)
			rpt += QObject::tr("+%1 Cosmopolitan. An avatar visited this system recently.<br>")
				.arg(abandonMod);
		else if(IsAbandoned())
		{
			if(m_LastAvatarAccountActive == 0xffff && m_LastVisit == 0xffff)
				rpt += QObject::tr("%1 Abandoned. "
						"No active avatar has ever called this system their land and "
						"no on-line avatar has ever visited this system.<br>")
					.arg(abandonMod);
			else if(m_LastAvatarAccountActive == 0xffff)
				rpt += QObject::tr("%1 Abandoned. "
						"No active avatar has ever called this system their land and "
						"the last on-line avatar to visit this system was %2 days ago.<br>")
					.arg(abandonMod).arg(m_LastVisit);
			else if(m_LastVisit == 0xffff)
				rpt += QObject::tr("%1 Abandoned. "
						"The last time an active avatar called this system their land was %2 days ago and "
						"no on-line avatar has ever visited this system. Impossible? I just report what they tell me.<br>")
					.arg(abandonMod).arg(m_LastAvatarAccountActive);
			else
				rpt += QObject::tr("%1 Abandoned. "
						"The last time an active avatar called this system their land was %2 days ago and "
						"the last on-line avatar to visit this system was %3 days ago.<br>")
					.arg(abandonMod).arg(m_LastAvatarAccountActive).arg(m_LastVisit);
		}

		// When buildings are destroyed due to battle damage, the people pull together.
		if(m_HasDestroyedBuildings)
		{
			if(m_HasDestroyedBuildings == 0x03)
			{
				iSatChange -= 2;
				rpt += QObject::tr("-2 Destroyed and Burning Buildings.<br>");
			}
			else if(m_HasDestroyedBuildings & 0x01)
			{
				iSatChange -= 1;
				rpt += QObject::tr("-1 Destroyed Buildings.<br>");
			}
			else
			{
				iSatChange -= 1;
				rpt += QObject::tr("-1 Burning Buildings.<br>");
			}
		}

		quint32 bldLimit = WorldBuildingLimit();
		if(m_WorldBuildingVol > bldLimit)
		{
			int upen = (m_WorldBuildingVol - bldLimit) / 1000 + 1;
			iSatChange -= upen;
			rpt += QObject::tr("-%1 Too Much Building Volume on World.<br>").arg(upen);
		}

		if(m_WorldBuildingCount > WorldBuildingCountLimit())
		{
			int upen = m_WorldBuildingCount - WorldBuildingCountLimit();
			iSatChange -= upen;
			rpt += QObject::tr("-%1 Too Many Buildings on World.<br>").arg(upen);
		}

		{
			// Cramped living conditions reduce satisfaction.
			/*
			quint32 nicehomes = m_HomesMd + (m_HomesLg * 2);
			if(m_HomesSm > nicehomes)
			{
				// Apartments account for more than 1/2 of the living space.
				quint32 upen = m_HomesSm / 2;
				if(nicehomes > 1)
					upen /= nicehomes;
				if(upen > 3)
					upen = 3;
				if(upen)
				{
					iSatChange -= upen;
					rpt += QObject::tr("-%1 Cramped Living Conditions. Need more medium or large homes.<br>").arg(upen);
				}
			}
			*/

			// Interesting activities improve satisfaction or simply reduce satisfaction when missing.
			// Bases reduce satisfaction when there are too many. Base limits are applied per resource zone.
			//int allow_troops = m_PopOrig/30;	// If this calculation changes, update DaWorld::MaxFlagsInResourceZone().
			//int req_arenas = RequiredArenas();
			//int req_casinos = RequiredCasinos();
			//int req_churches = RequiredChurches();
			//int req_fire = m_PopOrig/40;
			//int req_grocery = RequiredGrocery();
			//req_hospitals = RequiredHospitals();
			//int req_parks = RequiredParks();
			int req_retail = RequiredRetail();
			int req_univs = RequiredUniversities();
			int req_cantinas = RequiredCantinas();
			//int req_zoos = RequiredZoos();

			/*
			int arenamod = m_Industries.Levels(IID_Arena) - req_arenas;
			if(arenamod < 0)
			{
				if(arenamod < -2)
					arenamod = -2;
				iSatChange += arenamod;
				rpt += QObject::tr("%1 Arena wanted.<br>").arg(arenamod);
			}
			*/

			int cantinamod = m_Lounges - req_cantinas + (m_ConsumedLiquor?1:0);
			if(cantinamod)
			{
				//if(cantinamod < -2)
					//cantinamod = -2;
				if(hungry || gasping)
					cantinamod = MIN(cantinamod, m_ConsumedLiquor?1:0);
				else
					cantinamod = MIN(cantinamod, m_ConsumedLiquor?2:1);
				if(cantinamod)
				{
					iSatChange += cantinamod;
					if(cantinamod < 0)
						rpt += QObject::tr("%1 Lounge wanted.<br>").arg(cantinamod);
					else if(m_Lounges)
						rpt += QObject::tr("+%1 Lounge.<br>").arg(cantinamod);
					else
						rpt += QObject::tr("+%1 Liquor.<br>").arg(cantinamod);
				}
			}

			/*
			int casinomod = m_Industries.Levels(IID_Casino) - req_casinos;
			if(casinomod < 0)
			{
				if(casinomod < -2)
					casinomod = -2;
				iSatChange += casinomod;
				rpt += QObject::tr("%1 Casino wanted.<br>").arg(casinomod);
			}

			int churchmod = m_Industries.Levels(IID_Church) - req_churches;
			if(churchmod)
			{
				if(churchmod < -2)
					churchmod = -2;
				if(hungry || gasping)
					churchmod = MIN(churchmod, 1);
				else
					churchmod = MIN(churchmod, 2);
				iSatChange += churchmod;
				if(churchmod < 0)
					rpt += QObject::tr("%1 Church wanted.<br>").arg(churchmod);
				else
					rpt += QObject::tr("+%1 Church.<br>").arg(churchmod);
			}

			int grocerymod = m_StoresGrocery - req_grocery;
			if(grocerymod < 0)
			{
				if(grocerymod < -2)
					grocerymod = -2;
				iSatChange += grocerymod;
				rpt += QObject::tr("%1 Grocery wanted.<br>").arg(grocerymod);
			}

			// Hospital modifier affects infant mortality rate below.
			hospitalPenalty = req_hospitals - m_Industries.Levels(IID_Hospital);
			if(hospitalPenalty > 0)
			{
				if(hospitalPenalty > 2)
					hospitalPenalty = 2;
				iSatChange -= hospitalPenalty;
				rpt += QObject::tr("%1 Hospital wanted.<br>").arg(-hospitalPenalty);
			}
			else
				hospitalPenalty = 0;

			int parkmod = m_Industries.Levels(IID_Park) - req_parks;
			if(parkmod < 0)
			{
				if(parkmod < -2)
					parkmod = -2;
				iSatChange += parkmod;
				rpt += QObject::tr("%1 Park wanted.<br>").arg(parkmod);
			}
			*/

			int retailmod = m_StoresRetail - req_retail;
			if(retailmod < 0)
			{
				//if(retailmod < -2)
					//retailmod = -2;
				iSatChange += retailmod;
				rpt += QObject::tr("%1 Retail Store wanted.<br>").arg(retailmod);
			}

			int univmod = m_Industries.Levels(IID_BaseAcademy) - req_univs;
			if(univmod < 0)
			{
				//if(univmod < -2)
					//univmod = -2;
				iSatChange += univmod;
				rpt += QObject::tr("%1 Star Fleet Academy wanted.<br>").arg(univmod);
			}

			/*
			int zoomod = m_Industries.Levels(IID_Zoo) - req_zoos;
			if(zoomod < 0)
			{
				if(zoomod < -2)
					zoomod = -2;
				iSatChange += zoomod;
				rpt += QObject::tr("%1 Zoo wanted.<br>").arg(zoomod);
			}
			*/
		}
	}
	else
		rpt += QObject::tr("No troops.<br>");

	if(rpt.right(4) == "<br>")
		rpt = rpt.left(rpt.count() - 4);	// Drop the trailing <br>.
	rpt += "</p>";

	// Adjust morale now so we can use the nu value to affect the population growth.
	int nusat = iSatChange;
	if(nusat < -128)
		nusat = -128;
	else if(nusat > 127)
		nusat = 127;

	// Report population changes, based on nu morale, starvation and suffocation.
	rpt += QObject::tr("<b>POPULATION %L1</b><p style=\"margin-left: 10px;\">").arg(m_PopOrig);
	if(m_PopBirths || m_PopDeaths ||
			m_PopIndigenousArr || m_PopIndigenousDep ||
			m_PopSeafarersArr || m_PopSeafarersDep ||
			m_PopTravellersArr || m_PopTravellersDep ||
			m_PopWanderersArr || m_PopWanderersDep)
	{
		// Births and deaths.
		if(m_PopBirths)
		{
			if(m_PopBirths > 1)
				rpt += QObject::tr("+%1 %2 %1 citizens enlisted.<br>").arg(m_PopBirths).arg(g_Au.MiddleDotSymbol());
			else
				rpt += QObject::tr("+1 %2 A citizen enlisted.<br>").arg(g_Au.MiddleDotSymbol());
		}
		if(m_PopDeaths)
		{
			if(m_PopDeaths > 1)
				rpt += QObject::tr("-%1 %2 %1 troops died.<br>").arg(m_PopDeaths).arg(g_Au.MiddleDotSymbol());
			else
				rpt += QObject::tr("-1 %2 A troop died.<br>").arg(g_Au.MiddleDotSymbol());
		}

		// Indigenous
		if(m_PopIndigenousArr)
		{
			if(m_PopIndigenousArr > 1)
				rpt += QObject::tr("+%1 %2 %1 indigenous people enlisted.<br>").arg(m_PopIndigenousArr).arg(g_Au.MiddleDotSymbol());
			else
				rpt += QObject::tr("+1 %2 An indigenous person enlisted.<br>").arg(g_Au.MiddleDotSymbol());
		}
		if(m_PopIndigenousDep)
		{
			if(m_PopIndigenousDep > 1)
				rpt += QObject::tr("-%1 %2 %1 troops mustered out to live with the indigenous.<br>").arg(m_PopIndigenousDep)
					.arg(g_Au.MiddleDotSymbol());
			else
				rpt += QObject::tr("-1 %2 A troop mustered out to live with the indigenous.<br>").arg(g_Au.MiddleDotSymbol());
		}

		// Land travellers.
		if(m_PopWanderersArr)
		{
			if(m_PopWanderersArr > 1)
				rpt += QObject::tr("+%1 %2 %1 recruits arrived by road.<br>").arg(m_PopWanderersArr).arg(g_Au.MiddleDotSymbol());
			else
				rpt += QObject::tr("+1 %2 A recruit arrived by road.<br>").arg(g_Au.MiddleDotSymbol());
		}
		if(m_PopWanderersDep)
		{
			if(m_PopWanderersDep > 1)
				rpt += QObject::tr("-%1 %2 %1 troops marched to another position.<br>").arg(m_PopWanderersDep)
					.arg(g_Au.MiddleDotSymbol());
			else
				rpt += QObject::tr("-1 %2 A troop marched to another position.<br>").arg(g_Au.MiddleDotSymbol());
		}

		// Seafarers
		if(m_PopSeafarersArr)
		{
			if(m_PopSeafarersArr > 1)
				rpt += QObject::tr("+%1 %2 %1 recruits arrived at the dock.<br>").arg(m_PopSeafarersArr)
					.arg(g_Au.MiddleDotSymbol());
			else
				rpt += QObject::tr("+1 %2 A recruit arrived at the dock.<br>").arg(g_Au.MiddleDotSymbol());
		}
		if(m_PopSeafarersDep)
		{
			if(m_PopSeafarersDep > 1)
				rpt += QObject::tr("-%1 %2 %1 troops departed on a mission by sea.<br>").arg(m_PopSeafarersDep)
					.arg(g_Au.MiddleDotSymbol());
			else
				rpt += QObject::tr("-1 %2 A troop departed on a mission by sea.<br>").arg(g_Au.MiddleDotSymbol());
		}

		// Air|space travellers.
		if(m_PopTravellersArr)
		{
			if(m_Industries.HasAirBase())
			{
				if(m_PopTravellersArr > 1)
					rpt += QObject::tr("+%1 %2 %1 recruits arrived at the air terminal.<br>").arg(m_PopTravellersArr)
						.arg(g_Au.MiddleDotSymbol());
				else
					rpt += QObject::tr("+1 %2 A recruit arrived at the air terminal.<br>").arg(g_Au.MiddleDotSymbol());
			}
			else
			{
				if(m_PopTravellersArr > 1)
					rpt += QObject::tr("+%1 %2 %1 recruits arrived from space.<br>").arg(m_PopTravellersArr)
						.arg(g_Au.MiddleDotSymbol());
				else
					rpt += QObject::tr("+1 %2 A recruit arrived from space.<br>").arg(g_Au.MiddleDotSymbol());
			}
		}
		if(m_PopTravellersDep)
		{
			if(m_PopTravellersDep > 1)
				rpt += QObject::tr("-%1 %2 %1 troops were dispatched under top secret orders via the air terminal.<br>")
					.arg(m_PopTravellersDep).arg(g_Au.MiddleDotSymbol());
			else
				rpt += QObject::tr("-1 %2 A troop was dispatched under top secret orders via the air terminal.<br>")
					.arg(g_Au.MiddleDotSymbol());
		}
	}
	else
		rpt += QObject::tr("No change.<br>");

	// Describe the locals.
	if(m_Citizens.size() > 1)
	{
		rpt += QObject::tr("Troops are mixed races.<br>");
		for(QMap<AuSpecies, AuCityCitizen>::const_iterator ci = m_Citizens.cbegin(); ci != m_Citizens.cend(); ci++)
		{
			QString citizenName = (*ci).SpeciesName();
			QString citizenDesc = (*ci).SpeciesDesc();
			if(citizenName != citizenDesc && !citizenName.isEmpty())
				rpt += QObject::tr("%L3 are %1, %2.<br>").arg(citizenName).arg(citizenDesc).arg((*ci).Population());
			else
				rpt += QObject::tr("%L3 are %1.<br>").arg(citizenDesc).arg((*ci).Population());
		}
	}
	else if(!m_Citizens.empty())
	{
		QMap<AuSpecies, AuCityCitizen>::const_iterator ci = m_Citizens.cbegin();
		QString citizenName = (*ci).SpeciesName();
		QString citizenDesc = (*ci).SpeciesDesc();
		if(citizenName != citizenDesc && !citizenName.isEmpty())
			rpt += QObject::tr("Troops are %1, %2.<br>").arg(citizenName).arg(citizenDesc);
		else
			rpt += QObject::tr("Troops are %1.<br>").arg(citizenDesc);
	}

	if(isAlienCity)
	{
		if(!m_NativeSpecies.isEmpty() && m_NativeSpecies != m_NativeSpeciesDesc)
			rpt += QObject::tr("Indigenous are %1, %2.<br>").arg(m_NativeSpecies).arg(m_NativeSpeciesDesc);
		else
			rpt += QObject::tr("Indigenous are %1.<br>").arg(m_NativeSpeciesDesc);
	}

	// Report if the parent planet has a ship captain available.
	if(!m_CaptName.isEmpty())
		rpt += QObject::tr("Star Fleet Academy Cadet %1 is ready for command assignment aboard a fleet spacecraft.<br>")
			.arg(m_CaptName);

	if(rpt.right(4) == "<br>")
		rpt = rpt.left(rpt.count() - 4);	// Drop the trailing <br>.
	rpt += "</p>";

	// Homes
	rpt += QObject::tr("<b>BARRACKS %6</b>"
			"<p style=\"margin-left: 10px;\">"
				"Large %9<br>"
				"Medium %8<br>"
				"Small %7"
				"</p>")
		.arg(m_HomesLg + m_HomesMd + m_HomesSm)
		.arg(m_HomesSm)
		.arg(m_HomesMd)
		.arg(m_HomesLg);

	// Jobs
	rpt += QObject::tr("<b>JOBS %1</b>"
			"<p style=\"margin-left: 10px;\">"
			"Construction %2<br>"
			"Manufacturing %3<br>"
			"Professional %4<br>"
			"Service %5"
			"</p>")
		.arg(m_Jobs + m_JobsRepair + m_JobsNonConst + m_JobsPro)
		.arg(m_JobsNonConst)
		.arg(m_Jobs)
		.arg(m_JobsPro)
		.arg(m_JobsRepair);

	// Power status
	if(m_iCapacitor)
	{
		rpt += QObject::tr("<b>POWER RESERVE %L1</b><p style=\"margin-left: 10px;\">").arg(m_Power);
		rpt += QObject::tr("Reserve Capacity %L1<br>").arg(m_iCapacitor);
		int percent = Percent(m_Power, m_iCapacitor);
		rpt += QObject::tr("%1% Reserve").arg(percent);
		rpt += "</p>";
	}

	// Show info about taxes accumulated and spent locally.
	if(BuildingLevels(IID_Bank))
	{
		rpt += QObject::tr("<b>BANK %L1%2</b><p style=\"margin-left: 10px;\">")
			.arg(m_Bank.BalanceEnding(), 0,'f',0)
			.arg(g_Au.CurrencySymbol());
		if(m_LastTribute > 0)
			rpt += QObject::tr("Previous Tribute %L1%2<br>")
				.arg(m_LastTribute, 0,'f',0)
				.arg(g_Au.CurrencySymbol());
		rpt += m_Bank.HtmlReport();
		rpt += "</p>";
	}

	// Spacecraft - All ships in town and nu ships in orbit.
	if(!m_Ships.isEmpty())
	{
		rpt += QObject::tr("<b>SPACECRAFT</b><p style=\"margin-left: 10px;\">");
		if(m_Industries.HasSensors() || m_Industries.Levels(IID_Observatory))
			rpt += QObject::tr("All spacecraft in orbit and in this city, regardless of empire.<br>");
		else
			rpt += QObject::tr("Empire-flagged spacecraft in orbit and all spacecraft in this city regardless of empire.<br>");

		// Make a table of the spacecraft.
		rpt += QObject::tr("<table border=\"0\" cellpadding=\"2\" "
			"cellspacing=\"0\" style=\"background-color: rgb(39, 64, 118);\"><tbody>");
		//rpt += QObject::tr("<tr>"
				//"<td><b>Spacecraft</b></td><td></td>"
				//"</tr>");
		for(QList<AuCityReportShip>::const_iterator si = m_Ships.cbegin(); si != m_Ships.cend(); si++)
		{
			QString icon;
			if((*si).m_Location == 2)
				icon = ":data/bHangar.png";
			else if((*si).m_IsSpaceStation)
				icon = ":data/CommStation.png";
			else
				icon = ":data/c_Spacecraft.png";

			QString note;
			if((*si).m_Location == 2)
				note = QObject::tr("Hangar, %1").arg((*si).m_ShipLL.ToString());
			else if((*si).m_Location == 1)
				note = QObject::tr("Ground, %1").arg((*si).m_ShipLL.ToString());
			else
				note = QObject::tr("Orbit, %1").arg((*si).m_ShipLL.ToString());

			if((*si).m_IsNew)
				note += ", New";

			rpt += QObject::tr("<tr><td style=\"vertical-align: top;\"><img height=\"32\" align=\"left\" src=\"qrc%1\"> %2</td>"
					"<td style=\"vertical-align: top;\">%3</td></tr>")
				.arg(icon)
				.arg((*si).m_SpotDesc)
				.arg(note);
		}
		rpt += "</tbody></table><br>";
		rpt += "</p>";
	}

	// Make a map of the facilities table contents so it will be alphabetized.
	QMap<QString, AuCityIndustryIID> facil;
	for(AuCityIndustryHash::const_iterator ii = m_Industries.cbegin(); ii != m_Industries.cend(); ii++)
	{
		// Don't show zeros. Shouldn't exist any more now that it's a hash and not an array.
		if((*ii).Levels() == 0)
			continue;
		const DgData& gd = g_Au.LockDataForRead();
		QString ind = gd.IndustryName(ii.key());
		g_Au.UnlockData();
		facil.insert(ind, AuCityIndustryIID(*ii, ii.key()));
	}
	if(!facil.isEmpty())
	{
		rpt += QObject::tr("<b>FACILITIES</b><p style=\"margin-left: 10px;\">");
		rpt += QObject::tr("Lounges: %L1. Includes local civilian lounges.<br>").arg(m_Lounges);
		rpt += QObject::tr("Retail Stores: %L1. Includes local civilian stores.<br>").arg(m_StoresRetail);

		rpt += QObject::tr("<table width=\"100%\" border=\"0\" cellpadding=\"2\" "
			"cellspacing=\"0\" style=\"background-color: rgb(7, 62, 94);\"><tbody>");
		rpt += QObject::tr("<tr><td><b>Name</b></td><td><b></b></td>"
				"<td></td>"
				"<td><b>Name</b></td><td><b></b></td></tr>");
		for(QMap<QString, AuCityIndustryIID>::const_iterator mit = facil.cbegin(); mit != facil.cend(); mit++)
		{
			QString sFacility = mit.key();
			int levelsTotal = (*mit).Levels();
			int levelsInUse = (*mit).LevelsInUse();
			QString levels;
			if(DbIndustry::DoesManufacture((*mit).IndustryId()))
				levels = QString("%L1, %L2 in use").arg(levelsTotal).arg(levelsInUse);
			else
				levels = QString("%L1").arg(levelsTotal);
			INDUSTRYID iid0 = (*mit).IndustryId();
			mit++;
			if(mit != facil.cend())
			{
				// We have two columns to list in this row, could be the last row but it doesn't matter.
				int levelsTotal1 = (*mit).Levels();
				int levelsInUse1 = (*mit).LevelsInUse();
				QString levels1;
				if(DbIndustry::DoesManufacture((*mit).IndustryId()))
					levels1 = QString("%L1, %L2 in use").arg(levelsTotal1).arg(levelsInUse1);
				else
					levels1 = QString("%L1").arg(levelsTotal1);

				rpt += QObject::tr("<tr>"
						"<td valign=\"top\"><img height=\"32\" align=\"left\" src=\"qrc%7\"> %1</td>"
						"<td valign=\"top\">%3</td>"
						"<td valign=\"top\"></td>"
						"<td valign=\"top\"><img height=\"32\" align=\"left\" src=\"qrc%8\"> %4</td>"
						"<td valign=\"top\">%6</td>"
						"</tr>")
					.arg(sFacility)
					.arg(levels)
					.arg(mit.key())
					.arg(levels1)
					.arg(DbIndustry::BitmapName(iid0))
					.arg(DbIndustry::BitmapName((*mit).IndustryId()));
			}
			else
			{
				// We only have one column to list in this last row.
				rpt += QObject::tr("<tr>"
						"<td valign=\"top\"><img height=\"32\" align=\"left\" src=\"qrc%4\"> %1</td>"
						"<td valign=\"top\">%3</td>"
						"</tr>")
					.arg(sFacility)
					.arg(levels)
					.arg(DbIndustry::BitmapName(iid0));
				break;		// Break out because we know we're done
			}
		}
		rpt += "</tbody></table><br></p>";
	}

	// Vehicles
	if(!m_Vehicles.empty())
	{
		rpt += QObject::tr("<b>VEHICLES</b><p style=\"margin-left: 10px;\">");
		rpt += QObject::tr("Vehicles available to construction and manufacturing processes.<br>");

		// Make a table of the vehicles available.
		rpt += QObject::tr("<table border=\"0\" cellpadding=\"2\" "
			"cellspacing=\"0\" style=\"background-color: rgb(39, 64, 118);\"><tbody>");
		rpt += QObject::tr("<tr>"
				"<td><b>Vehicle</b></td>"
				"</tr>");
		for(QList<COMMODITYID>::const_iterator vi = m_Vehicles.cbegin(); vi != m_Vehicles.cend(); vi++)
			rpt += QObject::tr("<tr><td valign=\"top\"><img height=\"32\" align=\"middle\" src=\"qrc%1\"> %2</td></tr>")
				.arg(g_Au.CommodityBitmapName(*vi)).arg(g_Au.CommodityName(*vi));
		rpt += "</tbody></table><br>";
		rpt += "</p>";
	}

	// Inventory
	if(!m_Inventory->empty() || !m_Tools->empty())
	{
		rpt += QObject::tr("<b>INVENTORY</b><p style=\"margin-left: 10px;\">");
		if(m_PopOrig)
		{
			if(!m_HasBreathableAtmo)
			{
				rpt += QObject::tr("Air: %L1 total units in stock<br>").arg(m_AirSupply);
				rpt += QObject::tr("Air Consumption: %L1 units per report period<br>").arg(m_PopOrig);
			}
			rpt += QObject::tr("Food: %L1 total nutrition value in stock<br>").arg(m_FoodUnits);
			rpt += QObject::tr("Food Consumption: %L1 nutrition value per report period<br>").arg(m_FoodPerMeal * m_PopOrig);
		}
		//rpt += QObject::tr("Storage Capacity Per Commodity %1<br>").arg(m_iStorage);
		// Make a map of the inventory table contents so it will be alphabetized.
		QString qual;
		QMultiMap<QString, QMap<CommoditySubKey, QString> > inven;		// Key is commodity name.
		QMultiMap<QString, QMap<CommoditySubKey, QString> > invArmor;	// Key is commodity name.
		QMultiMap<QString, QMap<CommoditySubKey, QString> > invFood;	// Key is commodity name.
		QMultiMap<QString, QMap<CommoditySubKey, QString> > invMat;		// Key is commodity name.
		QMultiMap<QString, QMap<CommoditySubKey, QString> > invMed;		// Key is commodity name.
		QMultiMap<QString, QMap<CommoditySubKey, QString> > invTool;	// Key is commodity name.
		QMultiMap<QString, QMap<CommoditySubKey, QString> > invToolInUse;	// Key is commodity name.
		QMultiMap<QString, QMap<CommoditySubKey, QString> > invWeapon;	// Key is commodity name.
		QMap<QString, QString> bitmap;		// Key is commodity name.
		for(AuItemHash::const_iterator it = m_Inventory->begin(); it != m_Inventory->end(); it++)
		{
			if((*it).second->CommodityId() == CID_Electricity)
				continue;	// Electricity is listed as power reserve.

			// Select the appropriate inventory list category.
			QMultiMap<QString, QMap<CommoditySubKey, QString> >* pi;
			if((*it).second->IsTool())	// Check before IsWeapon
				pi = &invTool;
			else if((*it).second->IsConstructionMaterial() || (*it).second->IsSparePart() || (*it).second->IsFuel())
				pi = &invMat;
			else if((*it).second->IsAmmo() || (*it).second->IsWeaponByDesign())
				pi = &invWeapon;
			else if((*it).second->IsArmorHelmetClothingOrHat())
				pi = &invArmor;
			else if((*it).second->CanBeEaten())
				pi = &invFood;
			else if((*it).second->IsMedical())
				pi = &invMed;
			else
				pi = &inven;

			QString commod = (*it).second->ListName();
			if(!bitmap.contains(commod))
				bitmap.insert(commod, QString(g_Au.CommodityBitmapName((*it).second->CommodityId())));

			QMultiMap<QString, QMap<CommoditySubKey, QString> >::iterator iit = pi->find(commod);
			if(iit == pi->end())
				iit = pi->insert(commod, QMap<CommoditySubKey, QString>());	// Create a nu entry for the commodity.

			if(iit != pi->end())
			{
				CommoditySubKey key((*it).second->Quality(), (*it).second->EmpireOfOrigin());
				if((*it).second->EmpireOfOrigin())
				{
					// The item has a known empire of origin.
					QString empireOfOrigin;
					if((*it).second->EmpireOfOrigin() != EmpireId(0))
					{
						// Item was created in another empire.
						const DgData& gd = g_Au.LockDataForRead();
						gd.EmpireName((*it).second->EmpireOfOrigin(), empireOfOrigin);
						g_Au.UnlockData();
					}
					if(!empireOfOrigin.isEmpty())
					{
						// Item was created in another empire.
						(*iit).insert(key, QObject::tr("%1 Q%2 Made in %3").arg((*it).second->Count())
								.arg((*it).second->Quality())
								.arg(empireOfOrigin));
					}
					else
					{
						// Item was created in my empire.
						(*iit).insert(key, QObject::tr("%1 Q%2").arg((*it).second->Count()).arg((*it).second->Quality()));
					}
				}
				else
				{
					// The item does not have a known empire of origin. Probably hand made or a carcass.
					(*iit).insert(key, QObject::tr("<i>%1 Q%2</i>").arg((*it).second->Count()).arg((*it).second->Quality()));
				}
			}
		}
		for(AuItemHash::const_iterator it = m_Tools->begin(); it != m_Tools->end(); it++)
		{
			if((*it).second->CommodityId() == CID_Electricity)
				continue;	// Electricity is listed as power reserve.

			// Select the appropriate inventory list category.
			QString commod = (*it).second->ListName();
			if(!bitmap.contains(commod))
				bitmap.insert(commod, QString(g_Au.CommodityBitmapName((*it).second->CommodityId())));

			QMultiMap<QString, QMap<CommoditySubKey, QString> >::iterator iit = invToolInUse.find(commod);
			if(iit == invToolInUse.end())
				iit = invToolInUse.insert(commod, QMap<CommoditySubKey, QString>());	// Create a nu entry for the commodity.

			if(iit != invToolInUse.end())
			{
				CommoditySubKey key((*it).second->Quality(), (*it).second->EmpireOfOrigin());
				if((*it).second->EmpireOfOrigin())
				{
					// The item has a known empire of origin.
					QString empireOfOrigin;
					if((*it).second->EmpireOfOrigin() != EmpireId(0))
					{
						// Item was created in another empire.
						const DgData& gd = g_Au.LockDataForRead();
						gd.EmpireName((*it).second->EmpireOfOrigin(), empireOfOrigin);
						g_Au.UnlockData();
					}
					if(!empireOfOrigin.isEmpty())
					{
						// Item was created in another empire.
						(*iit).insert(key, QObject::tr("%1 Q%2 Made in %3")
								.arg((*it).second->Count())
								.arg((*it).second->Quality())
								.arg(empireOfOrigin));
					}
					else
					{
						// Item was created in my empire.
						(*iit).insert(key, QObject::tr("%1 Q%2").arg((*it).second->Count()).arg((*it).second->Quality()));
					}
				}
				else
				{
					// The item does not have a known empire of origin. Probably hand made or a carcass.
					(*iit).insert(key, QObject::tr("<i>%1 Q%2</i>").arg((*it).second->Count()).arg((*it).second->Quality()));
				}
			}
		}

		// Create a table for each category of commodities.
		// Food and Drinks
		if(!invFood.empty())
		{
			rpt += QObject::tr("<table border=\"0\" cellpadding=\"2\" "
				"cellspacing=\"0\" style=\"background-color: rgb(49, 36, 94);\"><tbody>");
			rpt += QObject::tr("<tr>"
					"<td><b>Food and Drinks</b></td>"
					"<td></td>"
					"</tr>");
			for(QMultiMap<QString, QMap<CommoditySubKey, QString> >::const_iterator iit = invFood.cbegin();
					iit != invFood.cend(); iit++)
			{
				QString scommod;
				for(QMap<CommoditySubKey, QString>::const_iterator sci = (*iit).cbegin(); sci != (*iit).cend(); sci++)
					if(scommod.isEmpty())
						scommod = *sci;
					else
						scommod += "<br>" + *sci;
				rpt += QObject::tr("<tr><td valign=\"top\"><img height=\"16\" align=\"middle\" src=\"qrc%3\"> %1</td><td valign=\"top\">%2</td></tr>")
					.arg(iit.key())
					.arg(scommod)
					.arg(bitmap[iit.key()]);
			}
			rpt += "</tbody></table><br>";
		}

		// Tools
		if(!invToolInUse.empty())
		{
			rpt += QObject::tr("<table border=\"0\" cellpadding=\"2\" "
				"cellspacing=\"0\" style=\"background-color: rgb(49, 36, 94);\"><tbody>");
			rpt += QObject::tr("<tr>"
					"<td><b>Tools In Use</b></td>"
					"<td></td>"
					"</tr>");
			for(QMultiMap<QString, QMap<CommoditySubKey, QString> >::const_iterator iit = invToolInUse.cbegin();
					iit != invToolInUse.cend(); iit++)
			{
				QString scommod;
				for(QMap<CommoditySubKey, QString>::const_iterator sci = (*iit).cbegin(); sci != (*iit).cend(); sci++)
					if(scommod.isEmpty())
						scommod = *sci;
					else
						scommod += "<br>" + *sci;
				rpt += QObject::tr("<tr><td valign=\"top\"><img height=\"16\" align=\"middle\" src=\"qrc%3\"> %1</td><td valign=\"top\">%2</td></tr>")
					.arg(iit.key())
					.arg(scommod)
					.arg(bitmap[iit.key()]);
			}
			rpt += "</tbody></table><br>";
		}
		if(!invTool.empty())
		{
			rpt += QObject::tr("<table border=\"0\" cellpadding=\"2\" "
				"cellspacing=\"0\" style=\"background-color: rgb(49, 36, 94);\"><tbody>");
			rpt += QObject::tr("<tr>"
					"<td><b>Tools</b></td>"
					"<td></td>"
					"</tr>");
			for(QMultiMap<QString, QMap<CommoditySubKey, QString> >::const_iterator iit = invTool.cbegin();
					iit != invTool.cend(); iit++)
			{
				QString scommod;
				for(QMap<CommoditySubKey, QString>::const_iterator sci = (*iit).cbegin(); sci != (*iit).cend(); sci++)
					if(scommod.isEmpty())
						scommod = *sci;
					else
						scommod += "<br>" + *sci;
				rpt += QObject::tr("<tr><td valign=\"top\"><img height=\"16\" align=\"middle\" src=\"qrc%3\"> %1</td><td valign=\"top\">%2</td></tr>")
					.arg(iit.key())
					.arg(scommod)
					.arg(bitmap[iit.key()]);
			}
			rpt += "</tbody></table><br>";
		}

		// Construction Materials, Spare Parts and Fuel
		if(!invMat.empty())
		{
			rpt += QObject::tr("<table border=\"0\" cellpadding=\"2\" "
				"cellspacing=\"0\" style=\"background-color: rgb(49, 36, 94);\"><tbody>");
			rpt += QObject::tr("<tr>"
					"<td><b>Construction Materials<br>Spare Parts and Fuel</b></td>"
					"<td></td>"
					"</tr>");
			for(QMultiMap<QString, QMap<CommoditySubKey, QString> >::const_iterator iit = invMat.cbegin();
					iit != invMat.cend(); iit++)
			{
				QString scommod;
				for(QMap<CommoditySubKey, QString>::const_iterator sci = (*iit).cbegin(); sci != (*iit).cend(); sci++)
					if(scommod.isEmpty())
						scommod = *sci;
					else
						scommod += "<br>" + *sci;
				rpt += QObject::tr("<tr><td valign=\"top\"><img height=\"16\" align=\"middle\" src=\"qrc%3\"> %1</td><td valign=\"top\">%2</td></tr>")
					.arg(iit.key())
					.arg(scommod)
					.arg(bitmap[iit.key()]);
			}
			rpt += "</tbody></table><br>";
		}

		// Medical
		if(!invMed.empty())
		{
			rpt += QObject::tr("<table border=\"0\" cellpadding=\"2\" "
				"cellspacing=\"0\" style=\"background-color: rgb(49, 36, 94);\"><tbody>");
			rpt += QObject::tr("<tr>"
					"<td><b>Medical</b></td>"
					"<td></td>"
					"</tr>");
			for(QMultiMap<QString, QMap<CommoditySubKey, QString> >::const_iterator iit = invMed.cbegin();
					iit != invMed.cend(); iit++)
			{
				QString scommod;
				for(QMap<CommoditySubKey, QString>::const_iterator sci = (*iit).cbegin(); sci != (*iit).cend(); sci++)
					if(scommod.isEmpty())
						scommod = *sci;
					else
						scommod += "<br>" + *sci;
				rpt += QObject::tr("<tr><td valign=\"top\"><img height=\"16\" align=\"middle\" src=\"qrc%3\"> %1</td><td valign=\"top\">%2</td></tr>")
					.arg(iit.key())
					.arg(scommod)
					.arg(bitmap[iit.key()]);
			}
			rpt += "</tbody></table><br>";
		}

		// Armor and Clothing
		if(!invArmor.empty())
		{
			rpt += QObject::tr("<table border=\"0\" cellpadding=\"2\" "
				"cellspacing=\"0\" style=\"background-color: rgb(49, 36, 94);\"><tbody>");
			rpt += QObject::tr("<tr>"
					"<td><b>Armor and Clothing</b></td>"
					"<td></td>"
					"</tr>");
			for(QMultiMap<QString, QMap<CommoditySubKey, QString> >::const_iterator iit = invArmor.cbegin();
					iit != invArmor.cend(); iit++)
			{
				QString scommod;
				for(QMap<CommoditySubKey, QString>::const_iterator sci = (*iit).cbegin(); sci != (*iit).cend(); sci++)
					if(scommod.isEmpty())
						scommod = *sci;
					else
						scommod += "<br>" + *sci;
				rpt += QObject::tr("<tr><td valign=\"top\"><img height=\"16\" align=\"middle\" src=\"qrc%3\"> %1</td><td valign=\"top\">%2</td></tr>")
					.arg(iit.key())
					.arg(scommod)
					.arg(bitmap[iit.key()]);
			}
			rpt += "</tbody></table><br>";
		}

		// Weapons and Ammunition
		if(!invWeapon.empty())
		{
			rpt += QObject::tr("<table border=\"0\" cellpadding=\"2\" "
				"cellspacing=\"0\" style=\"background-color: rgb(49, 36, 94);\"><tbody>");
			rpt += QObject::tr("<tr>"
					"<td><b>Weapons and Ammunition</b></td>"
					"<td></td>"
					"</tr>");
			for(QMultiMap<QString, QMap<CommoditySubKey, QString> >::const_iterator iit = invWeapon.cbegin();
					iit != invWeapon.cend(); iit++)
			{
				QString scommod;
				for(QMap<CommoditySubKey, QString>::const_iterator sci = (*iit).cbegin(); sci != (*iit).cend(); sci++)
					if(scommod.isEmpty())
						scommod = *sci;
					else
						scommod += "<br>" + *sci;
				rpt += QObject::tr("<tr><td valign=\"top\"><img height=\"16\" align=\"middle\" src=\"qrc%3\"> %1</td><td valign=\"top\">%2</td></tr>")
					.arg(iit.key())
					.arg(scommod)
					.arg(bitmap[iit.key()]);
			}
			rpt += "</tbody></table><br>";
		}

		// Miscellaneous
		if(!inven.empty())
		{
			rpt += QObject::tr("<table border=\"0\" cellpadding=\"2\" "
				"cellspacing=\"0\" style=\"background-color: rgb(49, 36, 94);\"><tbody>");
			rpt += QObject::tr("<tr>"
					"<td><b>Miscellaneous</b></td>"
					"<td></td>"
					"</tr>");
			for(QMultiMap<QString, QMap<CommoditySubKey, QString> >::const_iterator iit = inven.cbegin();
					iit != inven.cend(); iit++)
			{
				QString scommod;
				for(QMap<CommoditySubKey, QString>::const_iterator sci = (*iit).cbegin(); sci != (*iit).cend(); sci++)
					if(scommod.isEmpty())
						scommod = *sci;
					else
						scommod += "<br>" + *sci;
				rpt += QObject::tr("<tr><td valign=\"top\"><img height=\"16\" align=\"middle\" src=\"qrc%3\"> %1</td><td valign=\"top\">%2</td></tr>")
					.arg(iit.key())
					.arg(scommod)
					.arg(bitmap[iit.key()]);
			}
			rpt += "</tbody></table><br>";
		}
		rpt += "</p>";
	}

	// Insert distress information at the head of the report.
	if(!m_DistressRpt.isEmpty())
		rpt.insert(distressIdx, m_DistressRpt);
	return rpt;
}

void AuCityReport::MinimizeData()
{
	m_Inventory->Clear();
}

bool AuCityReport::HasAir() const		// Return true if any air is in stock.
{
	return m_Inventory->contains(CID_Air);
}

bool AuCityReport::HasPower() const		// Return true if any electricity is in stock.
{
	// Cities on ringworlds always have all the power they need.
	if(m_IsOnRingworld)
		return true;
	return m_Power;
}

bool AuCityReport::HasRadar() const
{
	return m_Industries.HasAirportRadar() || m_Industries.HasBaseRadar();
}

void AuCityReport::AddResidentSpecies(const AuSpecies& species, bool isAlien, quint32 residents)
{
	QMap<AuSpecies, AuCityCitizen>::iterator ci = m_Citizens.find(species);
	if(ci != m_Citizens.end())
		(*ci).AddPopulation(residents);
	else
	{
		ECOROLE ecoRole = species.EcoRole();
		m_Citizens.insert(species, AuCityCitizen(species, ecoRole, isAlien, residents));
	}
}

// Return worker DNA for this entire city.
AuCityCitizen AuCityReport::Citizen() const
{
	// Find the one with the highest population.
	QMap<AuSpecies, AuCityCitizen>::const_iterator hi = m_Citizens.cbegin();
	if(hi == m_Citizens.cend())
		return AuCityCitizen();

	for(QMap<AuSpecies, AuCityCitizen>::const_iterator ci = m_Citizens.cbegin()+1; ci != m_Citizens.cend(); ci++)
		if((*hi).Population() < (*ci).Population())
			hi = ci;	// Remember the higher population.
	return *hi;
}

quint64 AuCityReport::MemoryUsage() const
{
	quint64 mem = 0;
	mem += sizeof(m_AirConsumed);
	mem += sizeof(m_AirSupply);
	mem += sizeof(m_AteEnoughFood);
	mem += sizeof(m_Bank);
	mem += StringMemoryUsage(m_CaptName);
	mem += sizeof(m_CityType);
	mem += sizeof(m_ConsumedLiquor);
	mem += sizeof(m_Decayed);
	mem += StringMemoryUsage(m_DistressRpt);			// Accumulates reasons for distress.
	mem += sizeof(m_EventLog);
	if(m_EventLog)
		mem += m_EventLog->MemoryUsage();
	mem += sizeof(AuFlag) * m_Flags.capacity();
	mem += sizeof(m_FoodTypes);
	mem += sizeof(m_FoodUnits);
	mem += sizeof(m_FounderId);
	mem += StringMemoryUsage(m_FounderName);
	mem += sizeof(m_Galaxy);
	mem += sizeof(m_HasBreathableAtmo);
	mem += sizeof(m_HasDestroyedBuildings);
	mem += sizeof(m_iCapacitor);
	mem += m_Industries.MemoryUsage();
	mem += sizeof(m_Inventory);
	if(m_Inventory)
		mem += m_Inventory->MemoryUsage();
	mem += sizeof(m_IsCapitalOfEmpire);
	mem += sizeof(m_IsCapitalOfGalaxy);
	mem += sizeof(m_IsCapitalOfSector);
	mem += sizeof(m_IsHabitablePlanet);
	mem += sizeof(m_IsIsolated);
	mem += sizeof(m_IsOnRingworld);
	mem += sizeof(m_iStorage);
	mem += sizeof(m_Jobs);
	mem += sizeof(m_JobsNonConst);
	mem += sizeof(m_JobsPro);
	mem += sizeof(m_JobsRepair);
	mem += sizeof(m_LastAvatarAccountActive);
	mem += sizeof(m_LastTribute);
	mem += sizeof(m_LastVisit);
	mem += sizeof(m_LL);
	mem += sizeof(m_LoyaltyOrig);
	mem += sizeof(m_MaxPop);
	mem += sizeof(m_MessageType);
	mem += StringMemoryUsage(m_Name);					// DaCity::HtmlName
	mem += StringMemoryUsage(m_NativeSpecies);
	mem += StringMemoryUsage(m_NativeSpeciesDesc);
	mem += m_Patents.MemoryUsage();
	mem += StringMemoryUsage(m_PlanetDesc);
	mem += StringMemoryUsage(m_PlanetName);
	mem += sizeof(m_PopOrig);
	mem += sizeof(m_Power);
	for(QMap<QString, quint16>::const_iterator pi = m_PatentRnD.cbegin(); pi != m_PatentRnD.cend(); pi++)
		mem += StringMemoryUsage(pi.key()) + sizeof(quint16);
	mem += sizeof(m_SatOrig);
	mem += StringMemoryUsage(m_SectorName);
	mem += StringMemoryUsage(m_SectorDesc);
	for(QList<AuCityReportShip>::const_iterator si = m_Ships.cbegin(); si != m_Ships.cend(); si++)
		mem += (*si).MemoryUsage();
	mem += StringMemoryUsage(m_SystemName);
	mem += StringMemoryUsage(m_SystemDesc);
	mem += sizeof(m_Tools);
	if(m_Tools)
		mem += m_Tools->MemoryUsage();
	mem += sizeof(m_TroopsAtCity);
	mem += sizeof(m_TroopsDrafted);
	mem += sizeof(m_Unemployed);
	mem += sizeof(COMMODITYID) * m_Vehicles.size();
	mem += sizeof(m_WhenCompiled);
	mem += sizeof(m_Zone);
	mem += sizeof(m_ZonePop);
	mem += sizeof(m_HomesLg);
	mem += sizeof(m_HomesMd);
	mem += sizeof(m_HomesSm);
	mem += sizeof(m_PopBirths);
	mem += sizeof(m_PopDeaths);
	mem += sizeof(m_PopIndigenousArr);
	mem += sizeof(m_PopIndigenousDep);
	mem += sizeof(m_PopTravellersArr);
	mem += sizeof(m_PopTravellersDep);
	return mem;
}

quint32 AuCityReport::WorldBuildingLimit() const
{
	return WorldBuildingLimit(m_MaxPop, m_IsOnRingworld);
}

//static
quint32 AuCityReport::WorldBuildingLimit(quint32 maxPopulation, bool isRingworld)
{
	quint32 bldLimit = (quint32)round(s_BuildingVolumePerPerson * maxPopulation);
	if(isRingworld)
		bldLimit /= 2;	// Reduce the limit for ringworlds because the linear calculation gives them 5.7x that of a size 8 world.
	return bldLimit;
}

//static
quint32 AuCityReport::WorldBuildingCountLimit()
{
	return 250;
}

quint32 AuCityReport::Population() const
{
	quint32 pop = 0;
	for(QMap<AuSpecies, AuCityCitizen>::const_iterator ci = m_Citizens.cbegin(); ci != m_Citizens.cend(); ci++)
		pop += (*ci).Population();
	return pop;
}

ALERT_CONDITION AuCityReport::AlertCondition() const
{
	if(m_PopOrig)
	{
		// Population needed for starvation and suffocation to be a factor.
		if(!m_HasBreathableAtmo && !m_AirSupply)
			return ConditionRed;		// Suffocating

		if(!m_FoodTypes)
			return ConditionRed;		// Starving
		else if(!m_AteEnoughFood)
			return ConditionYellow;		// Hungry
	}

	// Destroyed buildings are a sign of trouble.
	if(m_HasDestroyedBuildings)
		return ConditionYellow;

	// Any alert in the log is at least a yellow alert. Those things don't kill me.
	if(m_EventLog->ContainsAlert(AuTime()))
		return ConditionYellow;

	// Nothing piped up so it's condition green.
	return ConditionGreen;
}

bool AuCityReport::IsAbandoned() const
{
	if(m_LastVisit == 0xffff && m_LastAvatarAccountActive == 0xffff)
		return true;	// Nobody has ever visited or been active here? Not likely.
	int days;
	if(m_LastVisit == 0xffff)
		days = m_LastAvatarAccountActive;		// Nobody has ever visited my system? Not likely.
	else if(m_LastAvatarAccountActive == 0xffff)
		days = m_LastVisit;				// No active avatar has ever had a home in my supersector? That is possible.
	else if(m_LastVisit < m_LastAvatarAccountActive)
		days = m_LastVisit;				// Last solar system visit was more recent than last active avatar sector ping.
	else
		days = m_LastAvatarAccountActive;		// Last active avatar sector ping was more recent than last solar system visit.
	return days > d_AbandonDaysBegin;
}

int AuCityReport::AbandonMod() const
{
	if(m_LastVisit == 0)
		return 1;						// Morale bonus while avatar is in the system.
	else if(m_LastVisit == 0xffff && m_LastAvatarAccountActive == 0xffff)
		return -100;

	int days;
	if(m_LastVisit == 0xffff)
		days = m_LastAvatarAccountActive;		// Nobody has ever visited my system? Not likely.
	else if(m_LastAvatarAccountActive == 0xffff)
		days = m_LastVisit;				// No active avatar has ever had a home in my supersector? That is possible.
	else if(m_LastVisit < m_LastAvatarAccountActive)
		days = m_LastVisit;				// Last solar system visit was more recent than last active avatar sector ping.
	else
		days = m_LastAvatarAccountActive;		// Last active avatar sector ping was more recent than last solar system visit.
	if(days <= d_AbandonDaysBegin)
		return 0;						// No abandonment penalty.

	// It has been a while since this solar system was visited by an online avatar or this sector was pinged by an active avatar.
	return (d_AbandonDaysBegin - days) / d_AbandonDaysPerMoralePoint;			// Negative number result.
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuCityReportHistory::Append(const AuCityReport& report)
{
	append(report);

	// Keep 48 hours of history for each city.
	int histlen = (48 * 60 * 60) / d_CityReportInterval;
	while(count() > histlen)
		removeFirst();

	// Remove the inventory from all but the last 8 reports (2 hours/~2 game days).
	int countToErase = count() - 8;
	if(countToErase <= 0)
		return;

	int index=0;
	for(iterator ri = begin(); ri != end(); ri++, index++)
	{
		if(index >= countToErase)
			break;
		(*ri).MinimizeData();
	}
}

quint64 AuCityReportHistory::MemoryUsage() const
{
	quint64 mem = sizeof(AuCityReportHistory);
	for(const_iterator ri = cbegin(); ri != cend(); ri++)
		mem += (*ri).MemoryUsage();
	return mem;
}
