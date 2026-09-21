//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dbindustry.cpp,v 1.72 2022/02/19 18:20:32 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dbindustry.h"
//
#include "audef.h"
#include "dbmanufacturingcomponent.h"
#include "dbmanufacturingprocess.h"
#include "qsqldatabase.h"
#include "qsqlquery.h"
#include <QVariant>

DbIndustry::DbIndustry()
{
	m_BuildRule = 0;
	m_capacitor = 0;
	m_id = 0;
	m_population = 0;
	m_employees = 0;
	m_storage = 0;
	m_structure = BD_Invalid;
	m_field = 0;
	m_arena = 0;
	m_auditorium = 0;
	m_battery = 0;
	m_cargoStorage = 0;
	m_generator = 0;
	m_guardPost = 0;
	m_homeLarge = 0;
	m_homeMedium = 0;
	m_homeSmall = 0;
	m_livestockPost = 0;
	m_lounge = 0;
	m_office = 0;
	m_parkGround = 0;
	m_parkSpace = 0;
	m_parkWater = 0;
	m_store = 0;
	m_sensor = 0;
	m_shield = 0;
	m_shop = 0;
	m_surgery = 0;
	m_transporter = 0;
	m_turret = 0;
	m_vehicleLauncher = 0;
	m_vehicleRecovery = 0;
	m_weaponBay = 0;
}

DbIndustry::~DbIndustry()
{
}

DbIndustry::DbIndustry(const DbIndustry &c) : m_Patents(c.m_Patents)
{
	m_BuildRule = c.m_BuildRule;
	m_capacitor = c.m_capacitor;
	m_DoesWhat = c.m_DoesWhat;
	m_id = c.m_id;
	m_name = c.m_name;
	m_population = c.m_population;
	m_employees = c.m_employees;
	m_storage = c.m_storage;
	m_structure = c.m_structure;
	m_field = c.m_field;
	m_arena = c.m_arena;
	m_auditorium = c.m_auditorium;
	m_battery = c.m_battery;
	m_cargoStorage = c.m_cargoStorage;
	m_generator = c.m_generator;
	m_guardPost = c.m_guardPost;
	m_homeLarge = c.m_homeLarge;
	m_homeMedium = c.m_homeMedium;
	m_homeSmall = c.m_homeSmall;
	m_livestockPost = c.m_livestockPost;
	m_lounge = c.m_lounge;
	m_note = c.m_note;
	m_office = c.m_office;
	m_parkGround = c.m_parkGround;
	m_parkSpace = c.m_parkSpace;
	m_parkWater = c.m_parkWater;
	m_store = c.m_store;
	m_sensor = c.m_sensor;
	m_shield = c.m_shield;
	m_shop = c.m_shop;
	m_surgery = c.m_surgery;
	m_transporter = c.m_transporter;
	m_turret = c.m_turret;
	m_vehicleLauncher = c.m_vehicleLauncher;
	m_vehicleRecovery = c.m_vehicleRecovery;
	m_weaponBay = c.m_weaponBay;
}

DbIndustry& DbIndustry::operator=(const DbIndustry &c)
{
	m_BuildRule = c.m_BuildRule;
	m_capacitor = c.m_capacitor;
	m_DoesWhat = c.m_DoesWhat;
	m_id = c.m_id;
	m_name = c.m_name;
	m_Patents = c.m_Patents;
	m_population = c.m_population;
	m_employees = c.m_employees;
	m_storage = c.m_storage;
	m_structure = c.m_structure;
	m_field = c.m_field;
	m_arena = c.m_arena;
	m_auditorium = c.m_auditorium;
	m_battery = c.m_battery;
	m_cargoStorage = c.m_cargoStorage;
	m_generator = c.m_generator;
	m_guardPost = c.m_guardPost;
	m_homeLarge = c.m_homeLarge;
	m_homeMedium = c.m_homeMedium;
	m_homeSmall = c.m_homeSmall;
	m_livestockPost = c.m_livestockPost;
	m_lounge = c.m_lounge;
	m_note = c.m_note;
	m_office = c.m_office;
	m_parkGround = c.m_parkGround;
	m_parkSpace = c.m_parkSpace;
	m_parkWater = c.m_parkWater;
	m_store = c.m_store;
	m_sensor = c.m_sensor;
	m_shield = c.m_shield;
	m_shop = c.m_shop;
	m_surgery = c.m_surgery;
	m_transporter = c.m_transporter;
	m_turret = c.m_turret;
	m_vehicleLauncher = c.m_vehicleLauncher;
	m_vehicleRecovery = c.m_vehicleRecovery;
	m_weaponBay = c.m_weaponBay;
	return *this;
}

void DbIndustry::operator>>(QDataStream& out) const
{
	out << m_capacitor;
	out << m_id;
	out << m_name;
	out << m_population;
	out << m_employees;
	out << m_storage;
	out << m_structure;
	out << m_field;
	out << m_arena;
	out << m_auditorium;
	out << m_battery;
	out << m_cargoStorage;
	out << m_generator;
	out << m_guardPost;
	out << m_homeLarge;
	out << m_homeMedium;
	out << m_homeSmall;
	out << m_livestockPost;
	out << m_lounge;
	out << m_note;
	out << m_office;
	out << m_parkGround;
	out << m_parkSpace;
	out << m_parkWater;
	out << m_store;
	out << m_sensor;
	out << m_shield;
	out << m_shop;
	out << m_surgery;
	out << m_transporter;
	out << m_turret;
	out << m_vehicleLauncher;
	out << m_vehicleRecovery;
	out << m_weaponBay;
	out << m_DoesWhat;
	out << m_BuildRule;
}

void DbIndustry::operator<<(QDataStream& in)
{
	in >> m_capacitor;
	in >> m_id;
	in >> m_name;
	in >> m_population;
	in >> m_employees;
	in >> m_storage;
	in >> m_structure;
	in >> m_field;
	in >> m_arena;
	in >> m_auditorium;
	in >> m_battery;
	in >> m_cargoStorage;
	in >> m_generator;
	in >> m_guardPost;
	in >> m_homeLarge;
	in >> m_homeMedium;
	in >> m_homeSmall;
	in >> m_livestockPost;
	in >> m_lounge;
	in >> m_note;
	in >> m_office;
	in >> m_parkGround;
	in >> m_parkSpace;
	in >> m_parkWater;
	in >> m_store;
	in >> m_sensor;
	in >> m_shield;
	in >> m_shop;
	in >> m_surgery;
	in >> m_transporter;
	in >> m_turret;
	in >> m_vehicleLauncher;
	in >> m_vehicleRecovery;
	in >> m_weaponBay;
	in >> m_DoesWhat;
	in >> m_BuildRule;
}

void DbIndustry::Tx(QDataStream& out) const
{
	out << m_capacitor;
	out << m_id;
	out << m_name;
	out << m_population;
	out << m_employees;
	out << m_storage;
	out << m_structure;
	out << m_field;
	out << m_arena;
	out << m_auditorium;
	out << m_battery;
	out << m_cargoStorage;
	out << m_generator;
	out << m_guardPost;
	out << m_homeLarge;
	out << m_homeMedium;
	out << m_homeSmall;
	out << m_livestockPost;
	out << m_lounge;
	out << m_note;
	out << m_office;
	out << m_parkGround;
	out << m_parkSpace;
	out << m_parkWater;
	out << m_store;
	out << m_sensor;
	out << m_shield;
	out << m_shop;
	out << m_surgery;
	out << m_transporter;
	out << m_turret;
	out << m_vehicleLauncher;
	out << m_vehicleRecovery;
	out << m_weaponBay;
	out << m_DoesWhat;
	out << m_BuildRule;
}

void DbIndustry::Rx(QDataStream& in)
{
	in >> m_capacitor;
	in >> m_id;
	in >> m_name;
	in >> m_population;
	in >> m_employees;
	in >> m_storage;
	in >> m_structure;
	in >> m_field;
	in >> m_arena;
	in >> m_auditorium;
	in >> m_battery;
	in >> m_cargoStorage;
	in >> m_generator;
	in >> m_guardPost;
	in >> m_homeLarge;
	in >> m_homeMedium;
	in >> m_homeSmall;
	in >> m_livestockPost;
	in >> m_lounge;
	in >> m_note;
	in >> m_office;
	in >> m_parkGround;
	in >> m_parkSpace;
	in >> m_parkWater;
	in >> m_store;
	in >> m_sensor;
	in >> m_shield;
	in >> m_shop;
	in >> m_surgery;
	in >> m_transporter;
	in >> m_turret;
	in >> m_vehicleLauncher;
	in >> m_vehicleRecovery;
	in >> m_weaponBay;
	in >> m_DoesWhat;
	in >> m_BuildRule;
}

//static
const char* DbIndustry::BitmapName(INDUSTRYID iid)
{
	if(iid == IID_None)
		return 0;

	// This little helper class is used to associate CIDs to file names in a static array.
	class ID_	// Industry Data
	{
	public:
		INDUSTRYID		lID;
		const char*		pFilename;
		ID_(INDUSTRYID li, const char *fn)
		{
			lID = li;
			pFilename = fn;
		}
	};

	// NOTE: The static ID_ array is in order of the IID_ enum values so they can be indexed directly. 
	//       The IID in the object is used to verify the array hasn't gotten out of sync.
	ID_ uid[] =
	{
		ID_(IID_None, 0),
		ID_(IID_TownSquare, ":data/bTownSquare.png"),
		ID_(IID_Road, ":data/bp_RoadConcrete.bmp"),
		ID_(IID_DesignStudio, ":data/bDesignStudio.png"),
		ID_(IID_Warehouse, ":data/bWarehouse.png"),
		ID_(IID_Landscape, ":data/bp_RoadIrrigate.bmp"),
		ID_((INDUSTRYID)6, ":data/bp_RoadDirt.bmp"),	// DpRoad::ROADMAT +6 offsets to here for roads. Used for road textures.
		ID_((INDUSTRYID)7, ":data/bp_RoadAsphalt.bmp"),
		ID_((INDUSTRYID)8, ":data/bp_RoadConcrete.bmp"),
		ID_((INDUSTRYID)9, ":data/bp_RoadPressurized.bmp"),
		ID_(IID_BaseFlag, ":data/bBaseFlag.png"),
		ID_(IID_BaseWeapon, ":data/bLaserTowerBase.png"),
		ID_(IID_BaseShield, ":data/bShieldGeneratorBase.png"),
		ID_(IID_StreetLight, ":data/bStreetLight.png"),
		ID_((INDUSTRYID)14, ":data/bp_SlabDirt.bmp"),	// DpRoad::ROADMAT +14 offsets to here for slabs. Used for road textures.
		ID_((INDUSTRYID)15, ":data/bp_SlabAsphalt.bmp"),
		ID_((INDUSTRYID)16, ":data/bp_SlabConcrete.bmp"),
		ID_((INDUSTRYID)17, ":data/bp_SlabPressurized.bmp"),
		ID_(IID_BaseMarineBarracks, ":data/bBaseMarineBarracks.png"),
		ID_(IID_BaseGuardPost, ":data/bBaseGuardPost.png"),
		ID_(IID_House, ":data/bHouse.png"),
		ID_(IID_Hotel, ":data/bHotel.png"),
		ID_(IID_Apartment, ":data/bApartment.png"),
		ID_((INDUSTRYID)23, ":data/bp_RoadGrade.bmp"),	// DpRoad::ROADMAT +23 offsets to here for landscape slabs.
		ID_((INDUSTRYID)24, ":data/bp_RoadIrrigate.bmp"),
		ID_((INDUSTRYID)25, ":data/bp_RoadDefoliate.bmp"),
		ID_((INDUSTRYID)26, ":data/bp_RoadClearing.bmp"),
		ID_(IID_BaseBarracks, ":data/bBaseBarracks.png"),
		ID_(IID_BaseHeadquarters, ":data/bBaseHeadquarters.png"),
		ID_(IID_BaseRadar, ":data/bBaseRadar.png"),
		ID_(IID_Farm, ":data/bFarm.png"),
		ID_(IID_LoggingCamp, ":data/bLoggingCamp.png"),
		ID_(IID_Orchard, ":data/bOrchard.png"),
		ID_(IID_None, 0),
		ID_(IID_Wharf, ":data/bWharf.png"),
		ID_(IID_Dairy, ":data/bDairy.png"),
		ID_(IID_AerialPlatform, ":data/bAerialPlatform.png"),
		ID_(IID_SeaPlatform, ":data/bSeaPlatform.png"),
		ID_(IID_GroundPlatform, ":data/bGroundPlatform.png"),
		ID_(IID_None, 0),
		ID_(IID_Bakery, ":data/bBakery.png"),
		ID_(IID_Brewery, ":data/bBrewery.png"),
		ID_(IID_Distillery, ":data/bDistillery.png"),
		ID_(IID_GristMill, ":data/bGristMill.png"),
		ID_(IID_LumberMill, ":data/bLumberMill.png"),
		ID_(IID_MeatProcessingPlant, ":data/bMeatProcessingPlant.png"),
		ID_(IID_SugarMill, ":data/bSugarMill.png"),
		ID_(IID_TextileMill, ":data/bTextileMill.png"),
		ID_(IID_Winery, ":data/bWinery.png"),
		ID_(IID_None, 0),
		ID_(IID_Mine, ":data/bMine.png"),
		ID_(IID_Well, ":data/bWell.png"),
		ID_(IID_DrillingRig, ":data/bDrillingRig.png"),
		ID_(IID_None, 0),
		ID_(IID_None, 0),
		ID_(IID_None, 0),
		ID_(IID_None, 0),
		ID_(IID_None, 0),
		ID_(IID_None, 0),
		ID_(IID_None, 0),
		ID_(IID_CoalPowerPlant, ":data/bCoalPowerPlant.png"),
		ID_(IID_GasolinePowerPlant, ":data/bGasolinePowerPlant.png"),
		ID_(IID_HydroelectricPowerPlant, ":data/bHydroelectricPowerPlant.png"),
		ID_(IID_HydrogenPowerPlant, ":data/bHydrogenPowerPlant.png"),
		ID_(IID_NaturalGasPowerPlant, ":data/bNaturalGasPowerPlant.png"),
		ID_(IID_None, 0),
		ID_(IID_SolarPowerPlant, ":data/bSolarPowerPlant.png"),
		ID_(IID_WindPowerPlant, ":data/bWindPowerPlant.png"),
		ID_(IID_NuclearPowerPlant, ":data/bNuclearPowerPlant.png"),
		ID_(IID_None, 0),
		ID_(IID_AircraftFactory, ":data/bAircraftFactory.png"),
		ID_(IID_Armorer, ":data/bArmorer.png"),
		ID_(IID_Carpenter, ":data/bCarpenter.png"),
		ID_(IID_ClothingFactory, ":data/bClothingFactory.png"),
		ID_(IID_ComputerFactory, ":data/bComputerFactory.png"),
		ID_(IID_ElectronicsFactory, ":data/bElectronicsFactory.png"),
		ID_(IID_ExplosivesFactory, ":data/bExplosivesFactory.png"),
		ID_(IID_GunSmith, ":data/bWeaponSmith.png"),
		ID_(IID_Jeweler, ":data/bJeweler.png"),
		ID_(IID_MachineShop, ":data/bMachineShop.png"),
		ID_(IID_MunitionsFactory, ":data/bMunitionsFactory.png"),
		ID_(IID_PharmaceuticalLab, ":data/bPharmaceuticalLab.png"),
		ID_(IID_PlasticsFactory, ":data/bPlasticsFactory.png"),
		ID_(IID_Refinery, ":data/bRefinery.png"),
		ID_(IID_RocketMotorFactory, ":data/bRocketMotorFactory.png"),
		ID_(IID_ShipYard, ":data/bShipYard.png"),
		ID_(IID_Smelter, ":data/bSmelter.png"),
		ID_(IID_SpacecraftFactory, ":data/bSpacecraftFactory.png"),
		ID_(IID_VehicleFactory, ":data/bVehicleFactory.png"),
		ID_(IID_TreatmentPlant, ":data/bTreatmentPlant.png"),
		ID_(IID_NuclearFuelPlant, ":data/bNuclearFuelPlant.png"),
		ID_(IID_None, 0),
		ID_(IID_None, 0),
		ID_(IID_None, 0),
		ID_(IID_None, 0),
		ID_(IID_None, 0),
		ID_(IID_None, 0),
		ID_(IID_None, 0),
		ID_(IID_None, 0),
		ID_(IID_None, 0),
		ID_(IID_Bank, ":data/bBank.png"),
		ID_(IID_Broker, ":data/bBroker.png"),
		ID_(IID_Casino, ":data/bCasino.png"),
		ID_(IID_Grocery, ":data/bGrocery.png"),
		ID_(IID_RetailStore, ":data/bRetailStore.png"),
		ID_(IID_Cantina, ":data/bTavern.png"),
		ID_(IID_None, 0),
		ID_(IID_None, 0),
		ID_(IID_None, 0),
		ID_(IID_AirportRadar, ":data/bAirportRadar.png"),
		ID_(IID_AirportRepairShop, ":data/bAirportRepairShop.png"),
		ID_(IID_AirportTerminal, ":data/bAirportTerminal.png"),
		ID_(IID_Arena, ":data/bArena.png"),
		ID_(IID_Zoo, ":data/bZoo.png"),
		ID_(IID_Park, ":data/bPark.png"),
		ID_(IID_None, 0),
		ID_(IID_None, 0),
		ID_(IID_None, 0),
		ID_(IID_None, 0),
		ID_(IID_PoliceAcademy, ":data/bPoliceAcademy.png"),
		ID_(IID_Church, ":data/bChurch.png"),
		ID_(IID_Hospital, ":data/bHospital.png"),
		ID_(IID_FireDepartment, ":data/bFireDepartment.png"),
		ID_(IID_Capitol, ":data/bCapitol.png"),
		ID_(IID_PoliceDepartment, ":data/bPoliceDepartment.png"),
		ID_(IID_School, ":data/bSchool.png"),
		ID_(IID_Hangar, ":data/bHangar.png"),
		ID_(IID_Observatory, ":data/bObservatory.png"),
		ID_(IID_BaseGuardTower, ":data/bGuardTower.png"),
		ID_(IID_Biodome, ":data/bBiodome.png"),
		ID_(IID_MassMedia, ":data/bMassMedia.png"),
		ID_(IID_Fence, ":data/bFenceWall.png"),
		ID_(IID_Bridge, ":data/bBridge.png"),
		ID_(IID_Tunnel, ":data/bTunnel.png"),
		ID_(IID_BaseAirTerminal, ":data/bBaseAirTerminal.png"),
		ID_(IID_BaseMotorPool, ":data/bBaseMotorPool.png"),
		ID_(IID_BaseDock, ":data/bBaseDock.png"),
		ID_(IID_Shed, ":data/bShed.png"),
		ID_(IID_StorageTank, ":data/bStorageTank.png"),
		ID_(IID_BaseAcademy, ":data/bBaseAcademy.png"),
		ID_(IID_PaperMill, ":data/bPaperMill.png"),
	};
	const int ids = sizeof(uid) / sizeof(ID_);

	// Insure the iid passed is valid.
	Q_ASSERT(iid < (INDUSTRYID)ids);
	if(iid >= ids)
		return 0;
	if(!uid[iid].pFilename)
		return 0;

	// Verify the iid passed matches the corresponding index.
	// If this assert fails, you've probably added an IID without adding an entry to this table.
	Q_ASSERT(uid[iid].lID == iid);
	if(uid[iid].lID != iid)
		return 0;

	return uid[iid].pFilename;
}

void DbIndustry::CalcPatentsRequired(const DbManufacturingProcessArray& mpa, const DbManufacturingComponentArray& mca)
{
	m_Patents.clear();
	for(DbManufacturingProcessArray::const_iterator mpi = mpa.cbegin(); mpi != mpa.cend(); mpi++)
	{
		if((*mpi).m_idi != m_id)
			continue;	// Process is for a different industry.
		for(DbManufacturingComponentArray::const_iterator mci = mca.cbegin(); mci != mca.cend(); mci++)
		{
			if((*mci).m_idmp != (*mpi).m_id)
				continue;	// Component is for a different process.
			if(!(*mci).IsRequired())
				continue;	// Component is not required.
			if(!DbCommodity::IsPatent((*mci).CommodityId()))
				continue;	// Component is not a patent.
			m_Patents.insert((*mci).CommodityId());
		}
	}
}

QString DbIndustry::BuildRuleText() const
{
	QString info;
	switch(BuildRule())
	{
	case 0:	// Civilian
		info += QObject::tr("Cannot be built on worlds claimed by an alien empire. ");
		info += QObject::tr("Cannot be built in the jurisdiction of an alien city. ");
		break;
	case 1:	// Military Headquarters
		info += QObject::tr("Can only be built on worlds claimed by your empire. ");
		info += QObject::tr("Can only be built on worlds with no military headquarters. ");
		break;
	case 2:	// Military
		info += QObject::tr("Can only be built on worlds with empire military headquarters. ");
		break;
	case 3:	// Capitol
		info += QObject::tr("Cannot be built on worlds claimed by an alien empire. ");
		info += QObject::tr("Cannot be built in the jurisdiction of an existing city. ");
		break;
	case 4:	// Police
		info += QObject::tr("Can be built in the jurisdiction of an empire city. ");
		info += QObject::tr("Can be built where an empire capitol building is in its jurisdiction. ");
		break;
	case 5:	// City
		info += QObject::tr("Can only be built in the jurisdiction of an empire city. ");
		break;
	default:
		info += QObject::tr("Cannot be built. ");
		break;
	}
	return info;
}

QColor DbIndustry::MapColor(INDUSTRYID iid)
{
	switch(iid)
	{
	case IID_AirportTerminal:
		return QColor(32,255,32,255);	// Green
	case IID_Capitol:
		return QColor(50,108,255,255);	// Hazeron Blue
	case IID_TownSquare:
		return QColor(192,32,255,255);	// Purple
	case IID_BaseHeadquarters:
		return QColor(255,32,32,255);	// Red
	case IID_BaseAcademy:
	case IID_BaseAirTerminal:
	case IID_BaseBarracks:
	case IID_BaseDock:
	case IID_BaseGuardPost:
	case IID_BaseGuardTower:
	case IID_BaseMarineBarracks:
	case IID_BaseMotorPool:
	case IID_BaseRadar:
	case IID_BaseShield:
	case IID_BaseWeapon:
		return QColor(255,128,32,255);	// Orange
	case IID_Road:
		return QColor(32,32,32,100);	// Dark grey.
	case IID_None:						// Road Slab
		return QColor(0,0,0,255);		// Black
	default:
		return QColor(255,255,32,100);	// Yellow
	}
}

qint8 DbIndustry::ParkSpacecraft() const
{
	switch(IndustryId())
	{
	case IID_SpacecraftFactory:
	case IID_AirportTerminal:
	case IID_BaseAirTerminal:
		return 1;
	case IID_Broker:
		return 0;
	default:
		return -1;
	}
}

QString DbIndustry::Note() const
{
	QString jobNote;
	if(DoesManufacture())
	{
		// Only buildings that manufacture things can create manufacturing jobs. This includes patent development.
		if(IndustryId() == IID_SpacecraftFactory ||
				IndustryId() == IID_AircraftFactory ||
				IndustryId() == IID_ShipYard ||
				IndustryId() == IID_VehicleFactory)
			jobNote = QObject::tr("Each building can run one manufacturing job.");
		else
		{
			if(Auditorium() > 0)
				jobNote = QObject::tr("Auditoriums in the blueprint determine the number of manufacturing jobs.");
			else if(Field() > 0)
				jobNote = QObject::tr("Fields in the blueprint determine the number of manufacturing jobs.");
			else if(Generator() > 0)
				jobNote = QObject::tr("Generators in the blueprint determine the number of manufacturing jobs.");
			else if(Shop() >= 0)
			{
				jobNote = QObject::tr("Workshops in the blueprint determine the number of manufacturing jobs.");
				// A mine with no shop uses its office, if it has one, as a shop to mine stone, to make noob startup easier.
				if(IndustryId() == IID_Mine)
					jobNote += QObject::tr(" One job making stone can be run when the building has no workshops.");
				else if(IndustryId() == IID_Bank)
					jobNote += QObject::tr(" A bank building with no workshops cannot mint money.");
			}
		}
	}
	else
	{
		// Buildings that do not manufacture things can create professional jobs.
		if(Arena() > 0)
			jobNote = QObject::tr("Arenas in the blueprint determine the number of professional jobs.");

		else if(Auditorium() > 0)
			jobNote = QObject::tr("Auditoriums in the blueprint determine the number of professional jobs.");

		else if(Field() > 0)
			jobNote = QObject::tr("Fields in the blueprint determine the number of professional jobs.");

		else if(GuardPost() > 0)
			jobNote = QObject::tr("Guard posts in the blueprint determine the number of professional jobs.");

		else if(Lounge() > 0)
			jobNote = QObject::tr("Lounges in the blueprint determine the number of professional jobs.");

		else if(Sensor() > 0)
			jobNote = QObject::tr("Radar in the blueprint adds one professional job.");

		else if(Shield() > 0)
			jobNote = QObject::tr("Shield in the blueprint adds one professional job.");

		else if(Shop() > 0)
			jobNote = QObject::tr("Workshops in the blueprint determine the number of professional jobs.");

		else if(Store() > 0)
			jobNote = QObject::tr("Stores in the blueprint determine the number of professional jobs.");

		else if(Turret() > 0)
			jobNote = QObject::tr("Turrets in the blueprint determine the number of professional jobs.");

		else if(WeaponBay() > 0)
			jobNote = QObject::tr("Weapon bays in the blueprint determine the number of professional jobs.");

		// Many buildings require an office. If they don't provide pro jobs from other specific things, then base it on offices.
		// This picks up hospitals and police departments. Who knows what else.
		else if(Office() > 0 && jobNote.isEmpty())
			jobNote = QObject::tr("Offices in the blueprint determine the number of professional jobs.");
	}

	QString note(m_note);
	if(IsPowerPlant())
	{
		if(!note.isEmpty())
			note += "</p><p>";
		note += QObject::tr("Power plants are strategic resources. "
				"If a military hq is present, power plants join the military base, if they are not part of a city.");
	}
	else if(IndustryId() == IID_Bank ||
			IndustryId() == IID_Capitol ||
			IndustryId() == IID_PoliceDepartment)
	{
		if(!note.isEmpty())
			note += "</p><p>";
		note += QObject::tr("<b>An empire is required to construct this building. </b>"
				"To create an empire, start by hand crafting a flag.");
	}

	if(note.isEmpty())
		return QString("<p>%1</p>").arg(jobNote);
	else if(!jobNote.isEmpty())
		return QString("<p>%1</p><p>%2</p>").arg(note).arg(jobNote);
	else
		return QObject::tr("<p>%1</p><p>Creates no manufacturing or professional jobs.</p>").arg(note);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void DbIndustryArray::Load()
{
	clear();
	resize(IID_Count);

	QSqlDatabase db = QSqlDatabase::database("AuUniverse", false);
	if(!db.isOpen())
		return;

	DbIndustry ob;
	QString sq("SELECT id, name, pop, emp, storage, buildRule, capacitor, "
			"structure, field, arena, auditorium, battery, cargoStorage, generator, guardPost, homeLarge, homeMedium, "
			"homeSmall, livestockPost, lounge, note, office, parkGround, parkSpace, parkWater, store, sensor, shield, shop, "
			"surgery, transporter, turret, vehicleLauncher, vehicleRecovery, weaponBay, doesWhat "
			"FROM b_Industry;");
	QSqlQuery qry(QString(), db);
	qry.prepare(sq);
	qry.exec();
	while(qry.next())
	{
		int qv=0;
		ob.m_id = qry.value(qv++).toUInt();
		ob.m_name = qry.value(qv++).toString();
		ob.m_population = qry.value(qv++).toUInt();
		ob.m_employees = qry.value(qv++).toInt();
		ob.m_storage = qry.value(qv++).toUInt();
		ob.m_BuildRule = qry.value(qv++).toUInt();
		ob.m_capacitor = qry.value(qv++).toUInt();
		ob.m_structure = qry.value(qv++).toUInt();
		ob.m_field = qry.value(qv++).toInt();
		ob.m_arena = qry.value(qv++).toInt();
		ob.m_auditorium = qry.value(qv++).toInt();
		ob.m_battery = qry.value(qv++).toInt();
		ob.m_cargoStorage = qry.value(qv++).toInt();
		ob.m_generator = qry.value(qv++).toInt();
		ob.m_guardPost = qry.value(qv++).toInt();
		ob.m_homeLarge = qry.value(qv++).toInt();
		ob.m_homeMedium = qry.value(qv++).toInt();
		ob.m_homeSmall = qry.value(qv++).toInt();
		ob.m_livestockPost = qry.value(qv++).toInt();
		ob.m_lounge = qry.value(qv++).toInt();
		ob.m_note = qry.value(qv++).toString();
		ob.m_office = qry.value(qv++).toInt();
		ob.m_parkGround = qry.value(qv++).toInt();
		ob.m_parkSpace = qry.value(qv++).toInt();
		ob.m_parkWater = qry.value(qv++).toInt();
		ob.m_store = qry.value(qv++).toInt();
		ob.m_sensor = qry.value(qv++).toInt();
		ob.m_shield = qry.value(qv++).toInt();
		ob.m_shop = qry.value(qv++).toInt();
		ob.m_surgery = qry.value(qv++).toInt();
		ob.m_transporter = qry.value(qv++).toInt();
		ob.m_turret = qry.value(qv++).toInt();
		ob.m_vehicleLauncher = qry.value(qv++).toInt();
		ob.m_vehicleRecovery = qry.value(qv++).toInt();
		ob.m_weaponBay = qry.value(qv++).toInt();
		ob.m_DoesWhat = qry.value(qv++).toString();
		Add(ob);
	}
}

// Store record using its id as the index for fast lookup performance.
void DbIndustryArray::Add(const DbIndustry& rec)
{
	Q_ASSERT(rec.m_id < IID_Count);
	if(isEmpty())
		resize(IID_Count);
	iterator it = begin() + rec.m_id;
	*it = rec;
}

void DbIndustryArray::operator<<(QDataStream& in)
{
	clear();
	quint16 count;
	in >> count;
	for(;count; count--)
	{
		DbIndustry industry;
		industry << in;
		append(industry);
	}
}

void DbIndustryArray::operator>>(QDataStream& out) const
{
	out << (quint16)count();
	for(const_iterator it = constBegin(); it != constEnd(); it++)
		(*it) >> out;
}

void DbIndustryArray::CalcPatentsRequired(const DbManufacturingProcessArray& mpa, const DbManufacturingComponentArray& mca)
{
	for(iterator ii = begin(); ii != end(); ii++)
		(*ii).CalcPatentsRequired(mpa, mca);
}
