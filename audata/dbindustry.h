//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dbindustry.h,v 1.74 2022/02/19 18:19:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DbIndustry_H
#define DbIndustry_H

#include "audata.h"
#include "dbcommodity.h"
#include <QString>
#include <QVector>

class DbManufacturingProcessArray;
class DbManufacturingComponentArray;

// These enum values match corresponding records in the [Industry] table.
enum INDUSTRYID
{
	IID_None = 0,
	IID_TownSquare = 1,
	IID_Road = 2,
	IID_DesignStudio = 3,
	IID_Warehouse = 4,
	IID_Landscape						= 5,
	// 6 reserved, see DbIndustry::BitmapName
	// 7 reserved, see DbIndustry::BitmapName
	// 8 reserved, see DbIndustry::BitmapName
	// 9 reserved, see DbIndustry::BitmapName
	IID_BaseFlag						= 10,					// Flag point
	IID_BaseWeapon						= 11,					// Weapon system building.
	IID_BaseShield						= 12,					// Shield generator building.
	IID_StreetLight						= 13,
	// 14 reserved, see DbIndustry::BitmapName
	// 15 reserved, see DbIndustry::BitmapName
	// 16 reserved, see DbIndustry::BitmapName
	// 17 reserved, see DbIndustry::BitmapName
	IID_BaseMarineBarracks				= 18,					// Homes at a military base for space marines.
	IID_BaseGuardPost					= 19,					// Guard post.
	IID_House = 20,
	IID_Hotel = 21,
	IID_Apartment = 22,
	// 23 reserved, see DbIndustry::BitmapName
	// 24 reserved, see DbIndustry::BitmapName
	// 25 reserved, see DbIndustry::BitmapName
	// 26 reserved, see DbIndustry::BitmapName
	IID_BaseBarracks					= 27,					// Homes at a military base.
	IID_BaseHeadquarters				= 28,					// Headquarters of a military base.
	IID_BaseRadar						= 29,					// Sensors
	IID_Farm = 30,
	IID_LoggingCamp = 31,
	IID_Orchard = 32,
	IID_Wharf = 34,
	IID_Dairy = 35,
	IID_AerialPlatform = 36,
	IID_SeaPlatform = 37,
	IID_GroundPlatform = 38,
	IID_Bakery = 40,
	IID_Brewery = 41,
	IID_Distillery = 42,
	IID_GristMill = 43,
	IID_LumberMill = 44,
	IID_MeatProcessingPlant = 45,
	IID_SugarMill = 46,
	IID_TextileMill = 47,
	IID_Winery = 48,
	IID_Mine = 50,
	IID_Well = 51,
	IID_DrillingRig = 52,
	IID_CoalPowerPlant = 60,
	IID_GasolinePowerPlant = 61,
	IID_HydroelectricPowerPlant = 62,
	IID_HydrogenPowerPlant = 63,
	IID_NaturalGasPowerPlant = 64,
	IID_SolarPowerPlant = 66,
	IID_WindPowerPlant = 67,
	IID_NuclearPowerPlant = 68,
	IID_AircraftFactory = 70,
	IID_Armorer = 71,
	IID_Carpenter = 72,
	IID_ClothingFactory = 73,
	IID_ComputerFactory = 74,
	IID_ElectronicsFactory = 75,
	IID_ExplosivesFactory = 76,
	IID_GunSmith = 77,
	IID_Jeweler = 78,
	IID_MachineShop = 79,
	IID_MunitionsFactory = 80,
	IID_PharmaceuticalLab = 81,
	IID_PlasticsFactory = 82,
	IID_Refinery = 83,
	IID_RocketMotorFactory = 84,
	IID_ShipYard = 85,
	IID_Smelter = 86,
	IID_SpacecraftFactory = 87,
	IID_VehicleFactory = 88,
	IID_TreatmentPlant = 89,
	IID_NuclearFuelPlant = 90,
	IID_Bank = 100,
	IID_Broker = 101,
	IID_Casino = 102,
	IID_Grocery = 103,
	IID_RetailStore = 104,
	IID_Cantina = 105,
	IID_AirportRadar = 109,
	IID_AirportRepairShop = 110,
	IID_AirportTerminal = 111,
	IID_Arena = 112,
	IID_Zoo = 113,
	IID_Park = 114,
	IID_PoliceAcademy = 119,
	IID_Church = 120,
	IID_Hospital = 121,
	IID_FireDepartment = 122,
	IID_Capitol = 123,
	IID_PoliceDepartment = 124,
	IID_School = 125,
	IID_Hangar							= 126,
	IID_Observatory						= 127,
	IID_BaseGuardTower					= 128,					// Guard tower building.
	IID_Biodome							= 129,
	IID_MassMedia						= 130,
	IID_Fence							= 131,					// Fence or wall.
	IID_Bridge							= 132,
	IID_Tunnel							= 133,
	IID_BaseAirTerminal					= 134,
	IID_BaseMotorPool					= 135,
	IID_BaseDock						= 136,
	IID_Shed							= 137,
	IID_StorageTank						= 138,
	IID_BaseAcademy						= 139,
	IID_PaperMill						= 140,
	IID_Count	// LAST ITEM MUST NOT EXCEED A BYTE. SEE AuBdPrefs.
};

class AUDATA_DLL INDUSTRYID_Hash
{
public:
	inline size_t operator()(const INDUSTRYID& o) const
	{
		return hash<unsigned int>()(o);
	}
};

class AUDATA_DLL DbIndustry
{
friend class DbIndustryArray;
public:
	quint8								m_id;
	QString								m_name;
	quint8								m_population;
private:
	quint8								m_storage;				// storage/10 to fit into a byte
	quint8								m_capacitor;			// capacitor/10 to fit into a byte
	qint8								m_employees;
	QString								m_DoesWhat;

	quint8								m_BuildRule;			// What rule governs building of this structure.
	quint8								m_structure;
	qint8								m_arena;
	qint8								m_auditorium;
	qint8								m_battery;
	qint8								m_cargoStorage;
	qint8								m_field;
	qint8								m_generator;
	qint8								m_guardPost;
	qint8								m_homeLarge;
	qint8								m_homeMedium;
	qint8								m_homeSmall;
	qint8								m_livestockPost;
	qint8								m_lounge;
	qint8								m_office;
	qint8								m_parkGround;
	qint8								m_parkSpace;
	qint8								m_parkWater;
	qint8								m_sensor;
	qint8								m_shield;
	qint8								m_shop;
	qint8								m_store;
	qint8								m_surgery;
	qint8								m_transporter;
	qint8								m_turret;
	qint8								m_vehicleLauncher;
	qint8								m_vehicleRecovery;
	qint8								m_weaponBay;
	QString								m_note;

	// Remaining values are not stored.
	QSet<COMMODITYID>					m_Patents;				// Patents required by any of my manufacturing processes. From CalcPatentsRequired

public:	// Constructors
	DbIndustry();
	~DbIndustry();
	DbIndustry(const DbIndustry &c);

public:	// Operators
	DbIndustry& operator=(const DbIndustry &c);

	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

	inline bool operator==(const DbIndustry& c) const
	{
		return m_id == c.m_id;
	}

public: // Methods
	inline qint8 Arena() const
	{
		return m_arena;
	}

	inline qint8 Auditorium() const
	{
		return m_auditorium;
	}

	inline qint8 Battery() const
	{
		return m_battery;
	}

	// Returns the small square resource bitmap used to represent this industry on buttons and textures.
	inline const char* BitmapName() const
	{
		return BitmapName(IndustryId());
	}

	static const char* BitmapName(INDUSTRYID iid);

	inline int BuildRule() const
	{
		return m_BuildRule;
	}

	QString BuildRuleText() const;

	void CalcPatentsRequired(const DbManufacturingProcessArray& mp, const DbManufacturingComponentArray& mca);

	// true means this industry can be an enclosure. It is not always so for platforms.
	inline bool CanBeEnclosure() const
	{
		return CanBeEnclosure(IndustryId());
	}

	// true means the industry can be an enclosure. It is not always so for platforms.
	inline static bool CanBeEnclosure(INDUSTRYID iid)
	{
		switch(iid)
		{
		case IID_AerialPlatform:
		case IID_Biodome:
		case IID_GroundPlatform:
		case IID_SeaPlatform:
			return true;
		default:
			return false;
		}
	}

	inline bool CanSupplyOrdnance() const
	{
		return CanSupplyOrdnance(IndustryId());
	}

	inline static bool CanSupplyOrdnance(INDUSTRYID iid)
	{
		switch(iid)
		{
		case IID_AirportRepairShop:
		case IID_AirportTerminal:
		case IID_BaseAirTerminal:
		case IID_BaseDock:
		case IID_BaseMotorPool:
		case IID_ShipYard:
		case IID_Wharf:
			return true;
		default:
			return false;
		}
	}

	inline int Capacitor() const
	{
		return 10 * m_capacitor;
	}

	inline qint8 CargoStorage() const
	{
		return m_cargoStorage;
	}

	static inline bool DoesCreateAirSpaceConnection(INDUSTRYID iid)
	{
		switch(iid)
		{
		case IID_AerialPlatform:
		case IID_AirportTerminal:
		case IID_BaseAirTerminal:
			return true;
		default:
			return false;
		}
	}

	static inline bool DoesCreateSeaConnection(INDUSTRYID iid)
	{
		switch(iid)
		{
		case IID_BaseDock:
		case IID_DrillingRig:
		case IID_SeaPlatform:
		case IID_Wharf:
			return true;
		default:
			return false;
		}
	}

	inline bool DoesManufacture() const
	{
		return DoesManufacture(IndustryId());
	}

	inline static bool DoesManufacture(INDUSTRYID iid)
	{
		switch(iid)
		{
		case IID_None:
		case IID_Count:
		case IID_AirportRadar:
		case IID_AirportTerminal:
		case IID_Apartment:
		case IID_Arena:
		case IID_BaseAcademy:
		case IID_BaseAirTerminal:
		case IID_BaseBarracks:
		case IID_BaseDock:
		case IID_BaseFlag:
		case IID_BaseGuardPost:
		case IID_BaseGuardTower:
		case IID_BaseHeadquarters:
		case IID_BaseMarineBarracks:
		case IID_BaseMotorPool:
		case IID_BaseRadar:
		case IID_BaseShield:
		case IID_BaseWeapon:
		case IID_Biodome:
		case IID_Bridge:
		case IID_Broker:
		case IID_Cantina:
		case IID_Capitol:
		case IID_Casino:
		case IID_Church:
		case IID_DesignStudio:
		case IID_Fence:
		case IID_FireDepartment:
		case IID_AerialPlatform:
		case IID_SeaPlatform:
		case IID_Grocery:
		case IID_GroundPlatform:
		case IID_Hangar:
		case IID_Hospital:
		case IID_Hotel:
		case IID_House:
		case IID_Landscape:
		case IID_MassMedia:
		case IID_Observatory:
		case IID_Park:
		case IID_PoliceAcademy:
		case IID_PoliceDepartment:
		case IID_RetailStore:
		case IID_Road:
		case IID_Shed:
		case IID_StorageTank:
		case IID_StreetLight:
		case IID_TownSquare:
		case IID_Tunnel:
		case IID_Warehouse:
		case IID_Zoo:
			return false;
		case IID_AirportRepairShop:
		case IID_Bank:
		case IID_Farm:
		case IID_LoggingCamp:
		case IID_Orchard:
		case IID_School:			// So the school can conduct research, 1 process per level.
		case IID_Wharf:
		case IID_Bakery:
		case IID_Brewery:
		case IID_Dairy:
		case IID_Distillery:
		case IID_DrillingRig:
		case IID_GristMill:
		case IID_LumberMill:
		case IID_MeatProcessingPlant:
		case IID_PaperMill:
		case IID_SugarMill:
		case IID_TextileMill:
		case IID_Winery:
		case IID_Mine:
		case IID_Well:
		case IID_CoalPowerPlant:
		case IID_GasolinePowerPlant:
		case IID_HydroelectricPowerPlant:
		case IID_HydrogenPowerPlant:
		case IID_NaturalGasPowerPlant:
		case IID_NuclearFuelPlant:
		case IID_NuclearPowerPlant:
		case IID_SolarPowerPlant:
		case IID_WindPowerPlant:
		case IID_AircraftFactory:
		case IID_Armorer:
		case IID_Carpenter:
		case IID_ClothingFactory:
		case IID_ComputerFactory:
		case IID_ElectronicsFactory:
		case IID_ExplosivesFactory:
		case IID_GunSmith:
		case IID_Jeweler:
		case IID_MachineShop:
		case IID_MunitionsFactory:
		case IID_PharmaceuticalLab:
		case IID_PlasticsFactory:
		case IID_Refinery:
		case IID_RocketMotorFactory:
		case IID_ShipYard:
		case IID_Smelter:
		case IID_SpacecraftFactory:
		case IID_TreatmentPlant:
		case IID_VehicleFactory:
			return true;
		}
		return false;
	}

	// Returns true for industries that can develop patents.
	inline bool DoesResearch() const
	{
		return DoesResearch(IndustryId());
	}

	// Returns true for industries that can develop patents. That tends to be the ones that can manufacture things but includes
	// universities and vehicle factories.
	inline static bool DoesResearch(INDUSTRYID iid)
	{
		switch(iid)
		{
		case IID_None:
		case IID_Count:
		case IID_AirportRadar:
		case IID_AirportTerminal:
		case IID_Apartment:
		case IID_Arena:
		case IID_Bank:
		case IID_BaseAcademy:
		case IID_BaseAirTerminal:
		case IID_BaseBarracks:
		case IID_BaseDock:
		case IID_BaseFlag:
		case IID_BaseGuardPost:
		case IID_BaseGuardTower:
		case IID_BaseHeadquarters:
		case IID_BaseMarineBarracks:
		case IID_BaseMotorPool:
		case IID_BaseRadar:
		case IID_BaseShield:
		case IID_BaseWeapon:
		case IID_Biodome:
		case IID_Bridge:
		case IID_Broker:
		case IID_Cantina:
		case IID_Capitol:
		case IID_Casino:
		case IID_Church:
		case IID_DesignStudio:
		case IID_Fence:
		case IID_FireDepartment:
		case IID_AerialPlatform:
		case IID_SeaPlatform:
		case IID_Grocery:
		case IID_GroundPlatform:
		case IID_Hangar:
		case IID_Hospital:
		case IID_Hotel:
		case IID_House:
		case IID_Landscape:
		case IID_MassMedia:
		case IID_Observatory:
		case IID_Park:
		case IID_PoliceAcademy:
		case IID_PoliceDepartment:
		case IID_RetailStore:
		case IID_Road:
		case IID_Shed:
		case IID_SpacecraftFactory:
		case IID_StorageTank:
		case IID_StreetLight:
		case IID_TownSquare:
		case IID_Tunnel:
		case IID_Warehouse:
		case IID_Zoo:
			return false;
		case IID_AircraftFactory:
		case IID_AirportRepairShop:		// makes modules
		case IID_Armorer:
		case IID_Bakery:
		case IID_Brewery:
		case IID_Carpenter:
		case IID_ClothingFactory:
		case IID_CoalPowerPlant:
		case IID_ComputerFactory:
		case IID_Dairy:
		case IID_Distillery:
		case IID_DrillingRig:
		case IID_ElectronicsFactory:
		case IID_ExplosivesFactory:
		case IID_Farm:
		case IID_GasolinePowerPlant:
		case IID_GristMill:
		case IID_GunSmith:
		case IID_HydroelectricPowerPlant:
		case IID_HydrogenPowerPlant:
		case IID_Jeweler:
		case IID_LoggingCamp:
		case IID_LumberMill:
		case IID_MachineShop:
		case IID_MeatProcessingPlant:
		case IID_Mine:
		case IID_MunitionsFactory:
		case IID_NaturalGasPowerPlant:
		case IID_NuclearFuelPlant:
		case IID_NuclearPowerPlant:
		case IID_Orchard:
		case IID_PaperMill:
		case IID_PharmaceuticalLab:
		case IID_PlasticsFactory:
		case IID_Refinery:
		case IID_RocketMotorFactory:
		case IID_School:				// So the university can conduct research.
		case IID_ShipYard:
		case IID_Smelter:
		case IID_SolarPowerPlant:
		case IID_SugarMill:
		case IID_TextileMill:
		case IID_TreatmentPlant:
		case IID_VehicleFactory:
		case IID_Well:
		case IID_Wharf:					// Fish
		case IID_WindPowerPlant:
		case IID_Winery:
			return true;
		}
		return false;
	}

	const QString& DoesWhat() const
	{
		return m_DoesWhat;
	}

	inline unsigned int Employees() const
	{
		switch(IndustryId())
		{
		case IID_BaseFlag:
		case IID_TownSquare:
			return 0;
		case IID_BaseBarracks:
		case IID_BaseHeadquarters:
		case IID_BaseGuardPost:
		case IID_BaseGuardTower:
		case IID_BaseMarineBarracks:
		case IID_BaseRadar:
		case IID_BaseShield:
		case IID_BaseWeapon:
			return 1;
		default:
			return abs(m_employees);
		}
	}

	inline qint8 Field() const
	{
		return m_field;
	}

	inline qint8 Generator() const
	{
		return m_generator;
	}

	inline qint8 GuardPost() const
	{
		return m_guardPost;
	}

	inline qint8 HomeLarge() const
	{
		return m_homeLarge;
	}

	inline qint8 HomeMedium() const
	{
		return m_homeMedium;
	}

	inline qint8 HomeSmall() const
	{
		return m_homeSmall;
	}

	inline INDUSTRYID IndustryId() const
	{
		return (INDUSTRYID)m_id;
	}

	// True for buildings that are candle lit when there is no power.
	// These buildings also use 100w tungsten lights instead of standard fluorescents.
	inline bool IsCandleLit() const
	{
		return IsCandleLit(IndustryId());
	}

	inline static bool IsCandleLit(INDUSTRYID iid)
	{
		switch(iid)
		{
		case IID_Apartment:
		//case IID_BaseBarracks:
		//case IID_BaseMarineBarracks:
		case IID_Cantina:
		case IID_Casino:
		case IID_Church:
		case IID_Farm:
		case IID_Hotel:
		case IID_House:
		case IID_Orchard:
		case IID_Park:
		//case IID_PoliceAcademy:
			return true;
		default:
			break;
		}
		return false;
	}

	inline bool IsCoastal() const
	{
		return IsCoastal(IndustryId());
	}

	inline static bool IsCoastal(INDUSTRYID industryId)
	{
		switch(industryId)
		{
		case IID_HydroelectricPowerPlant:
		case IID_ShipYard:
		case IID_Wharf:
			return true;
		default:
			break;
		}
		return false;
	}

	inline bool IsMilitary()
	{
		return IsMilitary(IndustryId());
	}

	inline static bool IsMilitary(INDUSTRYID iid)
	{
		switch(iid)
		{
		case IID_BaseAcademy:
		case IID_BaseAirTerminal:
		case IID_BaseBarracks:
		case IID_BaseDock:
		case IID_BaseGuardPost:
		case IID_BaseGuardTower:
		case IID_BaseHeadquarters:
		case IID_BaseMarineBarracks:
		case IID_BaseMotorPool:
		case IID_BaseRadar:
		case IID_BaseShield:
		case IID_BaseWeapon:
			return true;
		default:
			return false;
		}
	}

	inline bool IsPatentRequired(COMMODITYID patentCID) const
	{
		return m_Patents.contains(patentCID);
	}

	inline bool IsPlatform() const
	{
		return IsPlatform(IndustryId());
	}

	inline static bool IsPlatform(INDUSTRYID iid)
	{
		switch(iid)
		{
		case IID_AerialPlatform:
		case IID_GroundPlatform:
		case IID_SeaPlatform:
			return true;
		default:
			return false;
		}
	}

	inline bool IsPowerPlant() const
	{
		return IsPowerPlant(IndustryId());
	}

	inline static bool IsPowerPlant(INDUSTRYID iid)
	{
		switch(iid)
		{
		case IID_CoalPowerPlant:
		case IID_GasolinePowerPlant:
		case IID_HydroelectricPowerPlant:
		case IID_HydrogenPowerPlant:
		case IID_NaturalGasPowerPlant:
		case IID_NuclearFuelPlant:
		case IID_NuclearPowerPlant:
		case IID_SolarPowerPlant:
		case IID_WindPowerPlant:
			return true;
		default:
			return false;
		}
	}

	inline qint8 LivestockPost() const
	{
		return m_livestockPost;
	}

	inline qint8 Lounge() const
	{
		return m_lounge;
	}

	// Color of this industry's marker on a map.
	static QColor MapColor(INDUSTRYID iid);

	inline QColor MapColor() const
	{
		return MapColor(IndustryId());
	}

	const QString& Name() const
	{
		return m_name;
	}

	QString Note() const;

	inline qint8 Office() const
	{
		return m_office;
	}

	inline qint8 ParkGround() const
	{
		return m_parkGround;
	}

	inline qint8 ParkSpace() const
	{
		return m_parkSpace;
	}

	qint8 ParkSpacecraft() const;

	inline qint8 ParkWater() const
	{
		return m_parkWater;
	}

	void Rx(QDataStream& in);

	inline qint8 Sensor() const
	{
		return m_sensor;
	}

	inline qint8 Shield() const
	{
		return m_shield;
	}

	inline qint8 Shop() const
	{
		return m_shop;
	}

	inline int Storage() const
	{
		return 10 * m_storage;
	}

	inline qint8 Store() const
	{
		return m_store;
	}

	inline BUILDING_STRUCTURE Structure() const
	{
		return (BUILDING_STRUCTURE)m_structure;
	}

	inline qint8 Surgery() const
	{
		return m_surgery;
	}

	inline qint8 Transporter() const
	{
		return m_transporter;
	}

	inline unsigned int Troops() const
	{
		switch(IndustryId())
		{
		case IID_TownSquare:
		case IID_BaseFlag:
		case IID_BaseShield:
		case IID_BaseWeapon:
		case IID_BaseGuardTower:
			return abs(m_employees);
		default:
			return 0;
		}
	}

	inline qint8 Turret() const
	{
		return m_turret;
	}

	void Tx(QDataStream& out) const;

	inline qint8 VehicleLauncher() const
	{
		return m_vehicleLauncher;
	}

	inline qint8 VehicleRecovery() const
	{
		return m_vehicleRecovery;
	}

	inline qint8 WeaponBay() const
	{
		return m_weaponBay;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUDATA_DLL DbIndustryArray : public QVector<DbIndustry>
{
public:	// Operators
	void operator>>(QDataStream& out) const;

	void operator<<(QDataStream& in);

public:	// Methods
	// Add record using its id as the index into this array.
	void Add(const DbIndustry& rec);

	void CalcPatentsRequired(const DbManufacturingProcessArray& mp, const DbManufacturingComponentArray& mca);

	void Load();
};

#endif // DbIndustry_H
