//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dbcommodity.h,v 1.210 2022/05/09 16:18:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef DbCommodity_H
#define DbCommodity_H

#include "augleffects.h"
#include "augloballib.h"
#include "aupoint.h"
#include <QImage>
#include <QString>
#include <QList>

class DgData;
class QDebug;

// Individual body positions for inventory items worn/equippped on the body.
// Body positions can hold containers such as backpacks or bags, which add
// more places to store stuff inside of them.
#define IPOSBIT(x)	(1<<x)		// Converts INVENTORYPOS to bit mask.
enum INVENTORYPOS
{
	// Negative INVENTORYPOS values are used for selecting physical body weapons and those of a mount.
	// These are not valid positions for storing inventory items.
	// All potential weapon locations of my mount, from WL_ enum. DNA determines which actually can be used, if any.
	IP_Mount_Anus = -14,
	IP_Mount_Eyes = -13,
	IP_Mount_Feet = -12,
	IP_Mount_Hands = -11,
	IP_Mount_Head = -10,
	IP_Mount_Tail = -9,
	IP_Mount_Torso = -8,
	
	// All potential weapon locations of my body, from WL_ enum. DNA determines which actually can be used, if any.
	IP_My_Anus = -7,
	IP_My_Eyes = -6,
	IP_My_Feet = -5,
	IP_My_Hands = -4,
	IP_My_Head = -3,
	IP_My_Tail = -2,
	IP_My_Torso = -1,
	
	// This is the beginning of inventory positions that can actually store items.
	// Armor (over clothing)
	IP_ArmorArms = 0,		// bitmask 0x00000001 1
	IP_ArmorFace,			// bitmask 0x00000002 2
	IP_ArmorFeet,			// bitmask 0x00000004 4
	IP_ArmorHands,			// bitmask 0x00000008 8
	IP_ArmorHead,			// bitmask 0x00000010 16
	IP_ArmorLegs,			// bitmask 0x00000020 32
	IP_ArmorLowerTorso,		// bitmask 0x00000040 64
	IP_ArmorUpperTorso,		// bitmask 0x00000080 128

	// Clothing (under armor)
	IP_ClothingArms,		// bitmask 0x00000100 256
	IP_ClothingFace,		// bitmask 0x00000200 512
	IP_ClothingHead,		// bitmask 0x00000400 1024
	IP_ClothingLegs,		// bitmask 0x00000800 2048
	IP_ClothingLowerTorso,	// bitmask 0x00001000 4096
	IP_ClothingUpperTorso,	// bitmask 0x00002000 8192

	// Other equipment besides clothing or armor.
	IP_AnkleLeft,			// bitmask 0x00004000 16384
	IP_AnkleRight,			// bitmask 0x00008000 32768
	IP_Back,				// bitmask 0x00010000 65536
	IP_HandLeft,			// bitmask 0x00020000 131072
	IP_HandRight,			// bitmask 0x00040000 262144
	IP_Neck,				// bitmask 0x00080000 524288
	IP_Ring1Left,			// bitmask 0x00100000 1048576
	IP_Ring1Right,			// bitmask 0x00200000 2097152
	IP_Ring2Left,			// bitmask 0x00400000 4194304
	IP_Ring2Right,			// bitmask 0x00800000 8388608
	IP_WaistBack,			// bitmask 0x01000000 16777216
	IP_WaistFront,			// bitmask 0x02000000 33554432
	IP_WaistLeft,			// bitmask 0x04000000 67108864
	IP_WaistRight,			// bitmask 0x08000000 134217728
	IP_WristLeft,			// bitmask 0x10000000 268435456
	IP_WristRight,			// bitmask 0x20000000 536870912

	// An item normally consumes all positions in the INVENTORY bitmask when
	// equipped.
	// When the EquipOr bit is set, the INVENTORYPOS bits that are set represent
	// all possible locations for the item, but the item only takes one of them.
	IP_EquipOr = 30,		// bitmask 0x40000000 1073741824

	// Final entry for count determination.
	// Also commonly used as the "invalid" state of this enumeration.
	IP_NumInventoryPos = 31	// NOTE: If this value must increase, DaPerson::TxUpdate must allow more bits for inventory pos.
};

class AUGLOBAL_DLL INVENTORYPOS_Hash
{
public:
	inline size_t operator()(const INVENTORYPOS& o) const
	{
		return hash<unsigned int>()(o);
	}
};

enum INVENTORYSUB
{
	IPS_Item,		// The item itself, e.g. food, medical kit.
	IPS_Weapon1,	// The weapon defined by weapon 1 properties.
	IPS_Weapon2,	// The weapon defined by weapon 2 properties.
};

enum AMMOTYPE
{
	AMMO_None = 0,
	AMMO_Energy = 1,
	AMMO_Heavy = 2,
	AMMO_Launcher = 3,
	AMMO_Pistol = 4,
	AMMO_Rifle = 5,
	AMMO_Shotgun = 6,
	AMMO_Tube = 7,
	AMMO_Energy1 = 8,	// Used for weapon 2 when it uses energy ammo from weapon 1, basically sharing. e.g. Modes of a laser rifle
	AMMO_Explosive = 9,	// Not specifically ammunition but explodes like ammo when damaged. e.g. propane tank, gigacell
};

enum COMMODITYBITS
{
	CB_Cargo				= 0x00000001,	// 1 Item can be bought/sold as cargo.
	CB_Food					= 0x00000002,	// 2 Item can be eaten/ingested but isn't liquor.
	CB_Liquor				= 0x00000004,	// 4 Item is liquor.

	CB_CureDisease			= 0x00000010,	// 16 Item cures disease.
	CB_CurePoison			= 0x00000020,	// 32 Item cures poison.

	// Note that weapon effect bits are used to identify simple physical damage inflicted by using an item on a target.
	// The 'health' field is used for the damage effect so this is incompatible with a medical kit.
	CB_Bludgeon				= 0x00000100,	// 256 Item can be used as a bludgeoning weapon.
	CB_Pierce				= 0x00000200,	// 512 Item be used as a simple piercing weapon.
	CB_Slash				= 0x00000300,	// 1024 Item can be used as a slashing weapon.

	// Indicate where this item can be purchased.
	CB_Grocery				= 0x00001000,	// 4096 Item can be purchased at grocery.
	CB_Retail				= 0x00002000,	// 8192 Item can be purchased at retail.
	CB_Cantina				= 0x00004000,	// 16384 Item can be purchased at cantina.
};

enum COMMODITYFILTER
{
	FILTER_All,
	FILTER_Ammo,
	FILTER_ArmorAndClothes,
	FILTER_ConstructionMaterials,
	FILTER_Food,
	FILTER_Fuel,
	FILTER_MedicalSupplies,
	FILTER_Misc,
	FILTER_SpacecraftDesigns,
	FILTER_SpareParts,
	FILTER_Tools,
	FILTER_Weapons,
	FILTER_BuildingDesigns,
	FILTER_NaturalResources,
	FILTER_Patents,
	FILTER_Count							// LAST ITEM
};

// Icons that correspond to each COMMODITYFILTER.
inline const char* CommodityFilterIcon(COMMODITYFILTER filter)
{
	const char* commodityFilterIcon[] =
	{
		":data/MsgSolicitSales.png",
		":data/c_RifleAmmo.png",
		":data/c_PowerArmor.png",
		":data/c_Lumber.png",
		":data/c_Vegetable.png",
		":data/MsgSolicitFuel.png",
		":data/c_MedKit.png",
		":data/Shelf.png",
		":data/c_DesignDisk.png",
		":data/UpgradeModule.png",
		":data/c_Hammer.png",
		":data/c_Pistol.png",
		":data/c_BuildingDesignDisk.png",
		":data/c_Ore.png",
		":data/c_Patent.png",
	};
	return commodityFilterIcon[filter];
}

// UNTRANSLATED names that correspond to each COMMODITYFILTER.
inline const char* CommodityFilterName(COMMODITYFILTER filter)
{
	const char* commodityFilterName[] =
	{
		"All Items",
		"Ammunition",
		"Armor and Clothing",
		"Construction Materials",
		"Food and Drinks",
		"Fuel",
		"Medical Supplies",
		"Miscellaneous",
		"Spacecraft Blueprints",
		"Spare Parts",
		"Tools",
		"Weapons",
		"Building Blueprints",
		"Natural Resources",
		"Patents",
	};
	return commodityFilterName[filter];
};

// These enum values match corresponding records in the [Commodity] table.
enum COMMODITYID
{
	// NOTE: Negative enum values are not actual commodities in the database.
	//       These are used to track damage sources in AuUnitDamage and non-commodity weapon types.
	// NOTE: There is a 1:1 correspondence betwen the values of (CID_Mount_Anus - CID_My_Torso) with (IP_Mount_Anus - IP_My_Torso).
	CID_FIRST = -34,
	CID_GivingBirth = -33,
	CID_Collision = -32,
	CID_Spaghettification = -31,		// ripped to shreds by black hole
	CID_Fire = -30,						// e.g. burning building
	CID_Acid = -29,
	CID_MedicalProcedure = -28,
	CID_ImplantBirth = -27,
	CID_Poison = -26,
	CID_Disease = -25,
	CID_Eaten = -24,
	CID_ShipCapture = -23,
	CID_Wormhole = -22,
	CID_Starvation = -21,
	CID_InternalInjuries = -20,
	CID_BaseWeapon = -19,
	CID_BaseTurret = -18,
	CID_SunBurn = -17,					// fried by star
	CID_ShipTurret = -16,
	CID_BayWeapon = -15,
	CID_Mount_Anus = -14,
	CID_Mount_Eyes = -13,
	CID_Mount_Feet = -12,
	CID_Mount_Hands = -11,
	CID_Mount_Head = -10,
	CID_Mount_Tail = -9,
	CID_Mount_Torso = -8,
	CID_My_Anus = -7,
	CID_My_Eyes = -6,
	CID_My_Feet = -5,
	CID_My_Hands = -4,
	CID_My_Head = -3,
	CID_My_Tail = -2,
	CID_My_Torso = -1,

	// The 'invalid' enum value.
	CID_Invalid = 0,		// used for 'Citizens' in commodity export restrictions

	// Beginning of enum values for commodities that exist in the database.
	CID_Flag = 1,
	CID_ComputerObsolete = 2,	// Was CID_Computer with no storage. Must keep the id to deserialize old CID_Computer data.
	CID_Electricity,
	CID_Hammer = 4,
	CID_Air,				// breathable compressed air
	CID_Helicopter,
	CID_SpaceRocket,
	CID_SpaceFighter,
	CID_SpaceTransport,
	CID_PistolAmmo,
	CID_HeavyArmsAmmoPi,
	CID_BombBl,
	CID_PlasticExplosive,
	CID_Animal = 14,
	CID_Beans,
	CID_Beer,
	CID_LeatherBodyArmor,
	CID_Bread,
	CID_Candy = 19,
	CID_Cheese,
	CID_JeansAndShirt = 21,
	CID_Coal,
	CID_CrudeOil,
	CID_Crystals,
	CID_Dynamite,
	CID_Eggs,
	CID_ElectronicPart,
	CID_FarmMachinery,
	CID_Fertilizer,
	CID_Fiber,
	CID_Pistol = 31,
	CID_Rifle,
	CID_Fish,
	CID_Flour,
	CID_Fruit,
	CID_Gasoline,
	CID_Gems,
	CID_Grain,
	CID_Grapes,
	CID_GunPowder,
	CID_WovenHelmet,
	CID_Herbs,
	CID_Hops,
	CID_Hydrogen,
	CID_Ice,
	CID_CrystalJewelry,
	CID_Leather,
	CID_Lumber,
	CID_AnimalMeat,
	CID_MechanicalPart,
	CID_Metal,
	CID_Milk = 52,
	CID_Minerals,
	CID_MineMachinery,
	CID_SmallMissileBl,
	CID_LargeMissileBl,
	CID_NaturalGas,
	CID_Nuts,
	CID_Ore,
	CID_Pastry,
	CID_Petrochemicals,
	CID_Pharmaceuticals,
	CID_Plastic,
	CID_LargeRocketMotor,
	CID_MetalPowerArmor,
	CID_Radioactives,
	CID_SmallRocketBl,
	CID_LargeRocketBl,
	CID_SmallRocketMotor,
	CID_CommercialShip,
	CID_NavalShip,
	CID_VacSuitHelmet,
	CID_MedicalKit = 73,
	CID_Spices,
	CID_RifleAmmo,
	CID_Stone,
	CID_Submarine,
	CID_Sugar = 78,
	CID_Textiles,
	CID_VacSuit,
	CID_Vegetable,
	CID_SUV,
	CID_Water,
	CID_Whiskey,
	CID_Wine,
	CID_Wood,
	CID_Atmosphere,			// used for 'Spacecraft Upgrade Installs' in commodity export restrictions
	CID_Hydrographics,
	CID_Vegetation,
	CID_MetalBodyArmor,
	CID_PlasticBodyArmor,
	CID_WovenBodyArmor,
	CID_NitrateExplosive,
	CID_Sunlight,			// used for 'Spacecraft Repairs' in commodity export restrictions
	CID_LeatherClothes,
	CID_LeatherHelmet,
	CID_PlasticHelmet,
	CID_GoldBracelet = 98,
	CID_GemRing,
	CID_FishMeat,
	CID_Vodka,
	CID_HeavyWeapon,
	CID_Tank,
	CID_APC,
	CID_MetalHelmet,
	CID_MetalPowerArmorHelmet,
	CID_Crate = 107,
	CID_Lantern = 108,
	CID_Knapsack = 109,
	CID_Backpack,
	CID_Lumenite,
	CID_BlankDisk = 112,
	CID_PatentRefiningAntifluxParticlesfromAtmosphere,
	CID_Shovel = 114,
	CID_Gin,
	CID_Knife = 116,
	CID_Shotgun = 117,
	CID_ShotgunAmmo = 118,
	CID_Antidote = 119,		// cures poison
	CID_Antitoxin = 120,	// cures disease
	CID_Rum = 121,
	CID_Tequila = 122,
	CID_FittingWrench = 123,
	CID_GlueGun = 124,
	CID_ScrewDriver = 125,
	CID_SewingNeedle = 126,
	CID_SolderIron = 127,
	CID_Tongs = 128,
	CID_Wrench = 129,
	CID_Hay = 130,
	CID_Torch = 131,
	CID_SailBoat = 132,
	CID_DirtBike = 133,
	CID_Eludium = 134,		// Required for antigravity
	CID_Pliers = 135,
	CID_TorqueWrench = 136,
	CID_Vulcanite = 137,	// Rock. Mine on Infernos. Smelt to make Vulcanium.
	CID_Ioplasma = 138,		// Gas. Refinery on Infernos.
	CID_Magmex = 139,		// Bars. Treatment Plant on Infernos.
	CID_Phlogiston = 140,	// Gas. Well on Infernos.
	CID_Bolite = 141,		// Ice. Mine on Frigids. "Hyperbolizes". Unstable.
	CID_Cryozine = 142,		// Liquid. Refinery on Frigids.
	CID_Myrathane = 143,	// Liquid. Treatment Plant on Frigids.
	CID_Polytaride = 144,	// Gas. Well on Frigids.
	CID_Vulcium = 145,
	CID_Magmium = 146,
	CID_Magma = 147,		// free in the environment
	CID_SupercooledLiquid = 148,	// used for 'Spacecraft Sales' in commodity export restrictions
	CID_InfernoAtmosphere = 149,	// used for 'Spacecraft Requisition" in commodity export restrictions
	CID_FrigidAtmosphere = 150,
	CID_MagmiumKnife = 151,
	CID_VulciumKnife = 152,
	CID_MagmiumPowerArmor = 153,
	CID_MagmiumPowerArmorHelmet = 154,
	CID_VulciumPowerArmor = 155,
	CID_VulciumPowerArmorHelmet = 156,
	CID_Money = 157,
	CID_PatentRefiningBorexinoPrecipitatefromAtmosphere = 158,
	CID_GoldOre = 159,
	CID_Gold = 160,
	CID_Computer = 161,		// with storage
	CID_GoldRing = 162,
	CID_Underwear = 163,	// Used to create underwear. No actual commodity exists. Do not make an item of this type.

	CID_GravCoupling = 226,		// made with eludium
	CID_CryoHeatSink,		// made with cryozine
	CID_IoTube,				// made with ioplasma
	CID_Clamp,				// repairs CryoHeatSink
	CID_OScope,				// repairs IoTube
	CID_AdvSpaceFighter,
	CID_MissionDisk,
	CID_Bone = 233,
	CID_CommandAndControlModule,
	CID_GravityDriveModule,
	CID_RocketDriveModule,
	CID_MetalHullModule,
	CID_LifeSupportModule,
	CID_HydrogenPowerPlantModule,
	CID_EnergyShieldModule = 240,
	CID_KineticShieldModule,
	CID_CloakingShieldModule,
	CID_SurgeryMedicalModule,
	CID_GeneticsMedicalModule,
	CID_CyberneticsMedicalModule,
	CID_PersonnelTransporterModule,
	CID_VehicleTransporterModule,
	CID_EnergyWeaponTurretModule = 248,

	CID_WeaponLauncherTurretModule = 250,

	CID_EnergyWeaponBayModule = 252,

	CID_WeaponLauncherBayModule = 254,

	CID_AtmosphereCondenserBayModule = 256,
	CID_ProspectorBayModule,
	CID_TractorBeamBayModule,
	CID_WormholePFTLDriveModule,
	CID_Warp1FTLDriveModule = 260,
	CID_BallisticWeaponBayModule,
	CID_BallisticWeaponTurretModule = 262,
	CID_SensorModule,
	CID_MagmiumHeatSink,
	CID_VulciumHeatSink,
	CID_Myrasplicer,
	CID_Microscope,
	CID_Gigacell,
	CID_TesloidDissipator,
	CID_NanopathCalibrator = 270,
	CID_AntifluxParticles,
	CID_BorexinoPrecipitate,
	CID_TypeOPreons,
	CID_TypeBPreons,
	CID_TypeAPreons,
	CID_TypeFPreons,
	CID_TypeGPreons,
	CID_TypeKPreons,
	CID_TypeMPreons,
	CID_MagmiumHullModule = 280,
	CID_VulciumHullModule,
	CID_Paper							= 282,
	CID_ParticleCollectorBayModule		= 283,
	CID_DNASampleKit					= 284,					// An unused kit ready for taking a DNA sample.
	CID_DNASample						= 285,					// A used sample kit, containing a valid DNA.

	CID_SiphonBayModule					= 289,
	CID_OfficerLog						= 290,					// A disk containing an officer's event log.
	CID_WildcatterBayModule				= 291,
	CID_AK47							= 292,
	CID_UtilityPouch					= 293,
	CID_GravPack						= 294,
	CID_Silage							= 295,
	CID_LaserRifle						= 296,
	CID_SmallRocketBlBu					= 297,
	CID_LargeRocketBlBu					= 298,
	CID_SmallMissileBlBu				= 299,
	CID_LargeMissileBlBu				= 300,
	CID_SmallRocketSlPi					= 301,
	CID_LargeRocketBuDi					= 302,
	CID_SmallMissileCrFr				= 303,
	CID_LargeMissileSlPo				= 304,
	CID_LaserPistol						= 305,
	CID_HeavyArmsAmmoPiRa				= 306,
	CID_HeavyArmsAmmoPiBl				= 307,
	CID_HeavyArmsAmmoPiAc				= 308,
	CID_StasisBeam						= 309,					// WE_Stun
	CID_NeutronBeam						= 310,					// WE_Radiation
	CID_ElectronBeam					= 311,					// WE_Electric
	CID_ErgRay							= 312,					// WE_Bludgeon
	CID_FreezeRay						= 313,					// WE_Cold
	CID_CompressorBeam					= 314,					// WE_Crush
	CID_DisruptorBeam					= 315,					// WE_Sonic
	CID_HeatRay							= 316,					// WE_Heat
	CID_ProtonBeam						= 317,					// WE_Pierce
	CID_DisintegratorRay				= 318,					// WE_Slash
	CID_WeaponDeploymentBayModule		= 319,
	CID_BombBlRa						= 320,
	CID_BombBlPi						= 321,
	CID_BombElSt						= 322,
	CID_BombSuPo						= 323,
	CID_BombSlCr						= 324,
	CID_Seed							= 325,
	CID_FlowerPotSmall					= 326,
	CID_FlowerPotLarge					= 327,
	CID_PictureFrame60x34				= 329,
	CID_PictureFrame22x28				= 330,
	CID_PictureFrame120x68				= 331,
	CID_Head							= 332,
	CID_Candle							= 333,
	CID_PreonRingO						= 334,
	CID_PreonRingB						= 335,
	CID_PreonRingA						= 336,
	CID_PreonRingF						= 337,
	CID_PreonRingG						= 338,
	CID_PreonRingK						= 339,
	CID_PreonRingM						= 340,
	CID_ParcX							= 341,
	CID_StoneFirePit					= 342,
	CID_Tepee							= 343,
	CID_PressureTent					= 344,
	CID_Beacon							= 345,
	CID_Drum							= 346,
	CID_IceCream						= 347,

	CID_BurningBarrel					= 349,

	CID_LifeSupportGravityModule		= 351,
	CID_CapacitorModule					= 352,
	CID_ScDesignDrawing					= 353,
	CID_ScDesignDisk					= 354,
	CID_ContainerTEU					= 355,
	CID_PatentRefiningHydrogenfromAtmosphere,
	CID_PatentRefiningAirfromAtmosphere,
	CID_PatentHelicopterApparatus,
	CID_PatentSpaceRocketApparatus,
	CID_PatentSpaceFighterF1Apparatus	= 360,
	CID_PatentSpaceTransportApparatus,
	CID_PatentMakingDynamite,
	CID_PatentMakingHeavyArmsAmmoPi,
	CID_PatentMakingPlasticExplosive,
	CID_PatentMakingBombBlBlackCat,
	CID_PatentRefiningFertilizerfromAtmosphere,
	CID_PatentRefiningFertilizerfromAir = 367,

	CID_ParcXUniform = 369,
	CID_PatentMakingLargeRocketMotor = 370,
	CID_PatentMakingPreonRing,
	CID_CapriHalterOutfit = 372,
	CID_Brain = 373,
	CID_PatentMakingPressureTent,
	CID_PatentMakingBeacon,
	CID_PatentMakingIceCream,
	CID_PatentMakingLifeSupportGravityModule,
	CID_PatentMakingCapacitorModule = 378,

	CID_PatentMakingCandy	= 380,
	CID_PatentMakingHydrogenfromNaturalGas,
	CID_Gills = 382,
	CID_PatentMakingComputer,
	CID_PatentMotorBikeApparatus,
	CID_Heart = 385,
	CID_PatentMakingRifleAmmunition,
	CID_PatentDistillingWhiskey = 387,

	CID_PatentMakingPistolAmmunition = 389,
	CID_PatentMakingGunPowder	= 390,
	CID_PatentGeneratingElectricityusingCoal,
	CID_PatentGeneratingElectricityusingGasoline,
	CID_PatentGeneratingElectricityusingHydrogen,
	CID_PatentGeneratingElectricityusingNaturalGas,
	CID_PatentGeneratingElectricityusingSunlight,
	CID_PatentGeneratingElectricityusingWind,
	CID_PatentGeneratingElectricityusingRadioactives,
	CID_PatentMakingElectronicPart,
	CID_PatentFarmMachineryApparatus,
	CID_PatentMakingKnifeusingMetalandPlastic	= 400,
	CID_PatentMakingWaterusingAirandHydrogen,
	CID_PatentMakingPistol,
	CID_PatentDistillingVodka,
	CID_Lungs = 404,
	CID_Stomach = 405,
	CID_PatentMakingRifle,
	CID_PatentMakingAK,
	CID_GreyBDU = 408,
	CID_KhakiBDU = 409,
	CID_OliveDrabBDU = 410,
	CID_PatentRefiningGasoline,
	CID_RedBinder = 412,
	CID_PatentGeneratingElectricityusingWaterinEnvironment,
	CID_PatentMakingNitrateExplosive = 414,

	CID_PatentRefiningPetrochemicals = 416,
	CID_PatentRefiningHydrogenfromWater,
	CID_PatentMakingGravCoupling,
	CID_PatentMakingCryoHeatSink,
	CID_PatentMakingMagmiumKnife	= 420,
	CID_GreenBinder = 421,
	CID_BlueBinder = 422,
	CID_PatentMakingIoTube,
	CID_PatentMakingPlastic,
	CID_PatentMineMachineryApparatus,
	CID_PatentMakingSmallMissileBlZipper = 426,

	CID_PatentMakingLargeMissileBlThumper = 429,
	CID_PatentMakingPastry	= 430,
	CID_PatentRefiningAirfromWater,
	CID_PatentMakingPharmaceuticalsusingPetrochemicals = 432,

	CID_PatentMakingMetalPowerArmorSuit = 434,
	CID_PatentMiningRadioactives,
	CID_PatentMakingSmallRocketBlPartypopper = 436,

	CID_PatentMakingLargeRocketBlThud = 439,
	CID_PatentMakingSmallRocketMotor = 440,

	CID_PatentPlasticSailboatApparatus = 442,
	CID_PatentTankApparatus,
	CID_PatentAPCApparatus,
	CID_PatentMakingMetalPowerArmorHelmet,
	CID_PatentMakingMagmiumPowerArmorHelmet,
	CID_PatentCommercialShipApparatus,
	CID_PatentBattleShipApparatus,
	CID_PatentSubmarineApparatus,
	CID_PatentWeavingTextilesusingPlastic	= 450,

	CID_PatentMakingEnvironmentSuit = 452,
	CID_PatentMakingEnvironmentSuitHelmet,
	CID_PatentSUVApparatus,
	CID_PatentDistillingGin,
	CID_PatentMakingHeavyWeapon,
	CID_PatentDistillingRum = 457,

	CID_PatentDistillingTequila = 459,

	CID_PatentMakingElectricLantern = 461,
	CID_PatentMiningLumenite,
	CID_PatentMakingBlankDisk = 463,

	CID_PatentMakingAntidoteusingHerbs = 465,
	CID_PatentMakingAntidoteusingMinerals,
	CID_PatentMakingAntidoteusingPetrochemicals,
	CID_PatentMakingAntitoxinusingAnimalCarcass,
	CID_PatentMakingAntitoxinusingEggs,
	CID_PatentMakingShotgun = 470,

	CID_BrownUniform = 472,
	CID_PatentMiningEludium,
	CID_PatentMakingShotgunAmmunition = 474,

	CID_PatentMakingGlueGun = 476,
	CID_GreyUniform,
	CID_PatentMakingSolderingIron,
	CID_RedUniform,
	CID_GreenUniform	= 480,
	CID_BlueUniform,
	CID_TealUniform,
	CID_GoldUniform,
	CID_PinkUniform,
	CID_PatentMakingGravPack,
	CID_WhiteUniform = 486,

	CID_BlackUniform = 488,
	CID_PatentMiningVulcanite = 489,
	CID_PatentMiningBolite	= 490,
	CID_PatentRefiningIoplasma,
	CID_PatentRefiningCryozine,
	CID_PatentExtractingMagmex,
	CID_PatentExtractingMyrathane,
	CID_PatentDrillingPhlogiston,
	CID_PatentDrillingPolytaride,
	CID_PatentSmeltingVulcium,
	CID_PatentSmeltingMagmium,
	CID_PatentMakingVulciumKnife,
	CID_OrangeUniform	= 500,

	CID_PatentMakingMagmiumPowerArmorSuit = 502,
	CID_PatentMakingVulciumPowerArmorSuit,
	CID_PatentMakingVulciumPowerArmorHelmet = 504,

	CID_PatentMakingOscilloscope = 506,
	CID_PatentSpaceFighterF2Apparatus,
	CID_PatentMakingCommandAndControlModule,
	CID_PatentMakingGravityDriveModule,
	CID_PatentMakingRocketDriveModule	= 510,

	CID_PatentMakingLifeSupportModule = 512,
	CID_PatentMakingHydrogenPowerPlantModule,
	CID_PatentMakingEnergyShieldModule,
	CID_PatentMakingKineticShieldModule,
	CID_PatentMakingCloakingShieldModule,
	CID_PatentMakingSurgeryMedicalModule,
	CID_PatentMakingGeneticsMedicalModule,
	CID_PatentMakingCyberneticsMedicalModule,
	CID_PatentMakingPersonnelTransporterModule	= 520,
	CID_PatentMakingVehicleTransporterModule,
	CID_PatentMakingEnergyWeaponTurretModule,
	CID_PatentMakingWeaponLauncherTurretModule,
	CID_PatentMakingEnergyWeaponBayModule,
	CID_PatentMakingWeaponLauncherBayModule,
	CID_PatentMakingAtmosphereCondenserBayModule,
	CID_PatentMakingProspectorBayModule,
	CID_PatentMakingTractorBeamBayModule,
	CID_PatentMakingWormholePFTLDriveModule,
	CID_PatentMakingWarp1FTLDriveModule	= 530,
	CID_PatentMakingBallisticWeaponBayModule,
	CID_PatentMakingBallisticWeaponTurretModule,
	CID_PatentMakingSensorModule,
	CID_PatentMakingMagmiumHeatSink,
	CID_PatentMakingVulciumHeatSink,
	CID_PatentMakingMyrasplicer,
	CID_PatentMakingMicroscope,
	CID_PatentMakingGigacell,
	CID_PatentMakingTesloidDissipator,
	CID_PatentMakingNanopathCalibrator	= 540,

	CID_PatentMakingParticleCollectorBayModule = 543,
	CID_PatentMakingDNASampleKit = 544,

	CID_PatentMakingWildcatterBayModule = 547,
	CID_PatentMakingSiphonBayModule,
	CID_PatentMakingLaserRifle,
	CID_PatentMakingSmallRocketBlBuFirecracker	= 550,
	CID_PatentMakingLargeRocketBlBuMolotov,
	CID_PatentMakingSmallMissileBlBuBarnburner,
	CID_PatentMakingLargeMissileBlBuFireball,
	CID_PatentMakingSmallRocketSlPiMeathook,
	CID_PatentMakingLargeRocketBuDiScudpox,
	CID_PatentMakingSmallMissileCrFrSnowball,
	CID_PatentMakingLargeMissileSlPoSideviper,
	CID_PatentMakingLaserPistol,
	CID_PatentMakingHeavyArmsAmmoPiRa,
	CID_PatentMakingHeavyArmsAmmoPiBl	= 560,
	CID_PatentMakingHeavyArmsAmmoPiAc,
	CID_PatentMakingStasisBeamLens,
	CID_PatentMakingNeutronBeamLens,
	CID_PatentMakingElectronBeamLens,
	CID_PatentMakingErgRayLens,
	CID_PatentMakingFreezeRayLens,
	CID_PatentMakingCompressorBeamLens,
	CID_PatentMakingDisruptorBeamLens,
	CID_PatentMakingHeatRayLens,
	CID_PatentMakingProtonBeamLens	= 570,
	CID_PatentMakingDisintegratorRayLens,
	CID_PatentMakingWeaponDeploymentBayModule,
	CID_PatentMakingBombBlRaLosAlamos,
	CID_PatentMakingBombBlPiPorcupine,
	CID_PatentMakingBombElStTesla,
	CID_PatentMakingBombSuPoMuffler,
	CID_PatentMakingBombSlCrMeatSlicer = 577,

	CID_PatentMakingCandleusingPetrochemicals = 584,

	CID_WormholePNFTLDriveModule = 589,
	CID_WormholePNNFTLDriveModule = 590,
	CID_Warp2FTLDriveModule = 591,
	CID_Warp3FTLDriveModule = 592,
	CID_Warp4FTLDriveModule = 593,
	CID_Warp5FTLDriveModule = 594,
	CID_Warp6FTLDriveModule = 595,
	CID_Warp7FTLDriveModule = 596,
	CID_Warp8FTLDriveModule = 597,
	CID_Warp9FTLDriveModule = 598,
	CID_PatentMakingWormholePNFTLDriveModule,
	CID_PatentMakingWormholePNNFTLDriveModule = 600,
	CID_PatentMakingWarp2FTLDriveModule,
	CID_PatentMakingWarp3FTLDriveModule,
	CID_PatentMakingWarp4FTLDriveModule,
	CID_PatentMakingWarp5FTLDriveModule,
	CID_PatentMakingWarp6FTLDriveModule,
	CID_PatentMakingWarp7FTLDriveModule,
	CID_PatentMakingWarp8FTLDriveModule,
	CID_PatentMakingWarp9FTLDriveModule = 608,
	CID_Adamantite = 609,				// Rock. Mine on core of gas giants. Smelt to make adamantine.
	CID_Adamantine = 610,				// Very hard metal.
	CID_Viathol = 611,					// Liquid, drilled from well on core of gas giants.
	CID_Flomentum = 612,				// Collected from gas giants using a treatment plant.
	CID_PatentMiningAdamantite = 613,
	CID_PatentSmeltingAdamantine = 614,
	CID_PatentDrillingViathol = 615,
	CID_PatentExtractingFlomentum = 616,
	CID_StorageCrystal = 617,
	CID_PatentMakingStorageCrystal = 618,
	CID_ThumbDrive = 619,
	CID_PatentMakingThumbDrive = 620,
	CID_PatentMakingJewelryStorage = 621,
	CID_BraceletThumbDrive = 622,
	CID_BraceletStorageCrystal = 623,
	CID_NecklaceThumbDrive = 624,
	CID_NecklaceStorageCrystal = 625,
	CID_RingThumbDrive = 626,
	CID_RingStorageCrystal = 627,
	CID_CoalPowerPlantModule = 628,
	CID_GasolinePowerPlantModule = 629,
	CID_HydroelectricPowerPlantModule = 630,
	CID_NaturalGasPowerPlantModule = 631,
	CID_NuclearPowerPlantModule = 632,
	CID_SolarPowerPlantModule = 633,
	CID_WindPowerPlantModule = 634,
	CID_WoodHullModule = 635,
	CID_LumberHullModule = 636,
	CID_StoneHullModule = 637,
	CID_AdamantineHullModule = 638,

	CID_BdDesignDrawing					= 650,
	CID_BdDesignDisk					= 651,
	CID_NuclearFuelRod					= 652,
	CID_PatentMakingNuclearFuelRod		= 653,

	CID_Plutonium						= 655,
	CID_PatentGeneratingElectricityusingNuclearFuelRod = 656,
	CID_PatentMakingPlutonium			= 657,

	CID_SpacecraftTitle					= 663,
	CID_Count													// LAST ITEM
};

class AUGLOBAL_DLL COMMODITYID_Hash
{
public:
	inline size_t operator()(const COMMODITYID& o) const
	{
		return hash<unsigned int>()(o);
	}
};

// These values describe the effect of components on a process.
// They appear in the b_ManufacturingComponent.ide field.
enum COMPEFFECT
{
	CE_None					= 1,
	CE_DecreasesTime		= 2,
	CE_DecreasesMaterials	= 3,
	CE_RequiredQ			= 4,		// Required and quality limits overall output quality. Always a patent.
	CE_Required				= 5,
	CE_IncreasesQuantity	= 6,
};

// Returns the commodity that is harvested at each setting of an atmosphere condenser.
// Negative index returns the count of possible commodities.
inline COMMODITYID HarvestAtmosphereCommodity(int index)
{
	// Resources that can be harvested with an atmosphere condenser.
	COMMODITYID atmosphereRes[] =
	{
		CID_Air,
		CID_Cryozine,
		CID_Hydrogen,
		CID_Ioplasma,
	};
	int atmosphereResCount = sizeof(atmosphereRes)/sizeof(COMMODITYID);
	if(index < 0)
		return (COMMODITYID)atmosphereResCount;
	else if(index < atmosphereResCount)
		return atmosphereRes[index];
	return atmosphereRes[0];
}

// Returns the number of different commodities that can be harvested using an atmosphere condenser.
inline int HarvestAtmosphereCount()
{
	return HarvestAtmosphereCommodity(-1);
}

inline int HarvestAtmosphereIndex(COMMODITYID cid)
{
	for(int ci=0; ci<HarvestAtmosphereCount(); ci++)
		if(HarvestAtmosphereCommodity(ci) == cid)
			return ci;
	return -1;
}

// Returns the commodity that is harvested at each setting of a prospector bay.
// Negative index returns the count of possible commodities.
inline COMMODITYID HarvestProspectorCommodity(int index)
{
	// Resources that can be harvested with a prospector bay.
	COMMODITYID res[] =
	{
		CID_Bolite,
		CID_Coal,
		CID_Crystals,
		CID_Eludium,
		CID_Gems,
		CID_GoldOre,
		CID_Ice,
		CID_Lumenite,
		CID_Minerals,
		CID_Ore,
		CID_Radioactives,
		CID_Stone,
		CID_Vulcanite,
		CID_Adamantite,
	};
	int resCount = sizeof(res)/sizeof(COMMODITYID);
	if(index < 0)
		return (COMMODITYID)resCount;
	else if(index < resCount)
		return res[index];
	return res[0];
}

// Returns the number of different commodities that can be harvested using a prospector bay.
inline int HarvestProspectorCount()
{
	return HarvestProspectorCommodity(-1);
}

inline int HarvestProspectorIndex(COMMODITYID cid)
{
	for(int ci=0; ci<HarvestProspectorCount(); ci++)
		if(HarvestProspectorCommodity(ci) == cid)
			return ci;
	return -1;
}

// Returns the commodity that is harvested at each setting of a particle collector.
// Negative index returns the count of possible commodities.
inline COMMODITYID HarvestParticleCommodity(int index)
{
	// Resources that can be harvested with a particle collector.
	COMMODITYID particleRes[] =
	{
		CID_AntifluxParticles,
		CID_BorexinoPrecipitate,
		CID_TypeOPreons,
		CID_TypeBPreons,
		CID_TypeAPreons,
		CID_TypeFPreons,
		CID_TypeGPreons,
		CID_TypeKPreons,
		CID_TypeMPreons,
	};
	int particleResCount = sizeof(particleRes)/sizeof(COMMODITYID);
	if(index < 0)
		return (COMMODITYID)particleResCount;
	else if(index < particleResCount)
		return particleRes[index];
	return particleRes[0];
}

// Returns the number of different commodities that can be harvested using a particle collector.
inline int HarvestParticleCount()
{
	return HarvestParticleCommodity(-1);
}

inline int HarvestParticleIndex(COMMODITYID cid)
{
	for(int ci=0; ci<HarvestParticleCount(); ci++)
		if(HarvestParticleCommodity(ci) == cid)
			return ci;
	return -1;
}

// Returns the commodity that is harvested at each setting of a siphon bay.
// Negative index returns the count of possible commodities.
inline COMMODITYID HarvestSiphonCommodity(int index)
{
	// Resources that can be harvested with a siphon bay.
	COMMODITYID res[] =
	{
		CID_Magmex,
		CID_Myrathane,
		CID_Water,
		CID_Flomentum,
	};
	int resCount = sizeof(res)/sizeof(COMMODITYID);
	if(index < 0)
		return (COMMODITYID)resCount;
	else if(index < resCount)
		return res[index];
	return res[0];
}

// Returns the number of different commodities that can be harvested using a siphon bay.
inline int HarvestSiphonCount()
{
	return HarvestSiphonCommodity(-1);
}

inline int HarvestSiphonIndex(COMMODITYID cid)
{
	for(int ci=0; ci<HarvestSiphonCount(); ci++)
		if(HarvestSiphonCommodity(ci) == cid)
			return ci;
	return -1;
}

// Returns the commodity that is harvested at each setting of a wildcatter bay.
// Negative index returns the count of possible commodities.
inline COMMODITYID HarvestWildcatterCommodity(int index)
{
	// Resources that can be harvested with a wildcatter bay.
	COMMODITYID res[] =
	{
		CID_NaturalGas,
		CID_CrudeOil,
		CID_Phlogiston,
		CID_Polytaride,
		CID_Water,
		CID_Viathol,
	};
	int resCount = sizeof(res)/sizeof(COMMODITYID);
	if(index < 0)
		return (COMMODITYID)resCount;
	else if(index < resCount)
		return res[index];
	return res[0];
}

// Returns the number of different commodities that can be harvested using a wildcatter bay.
inline int HarvestWildcatterCount()
{
	return HarvestWildcatterCommodity(-1);
}

inline int HarvestWildcatterIndex(COMMODITYID cid)
{
	for(int ci=0; ci<HarvestWildcatterCount(); ci++)
		if(HarvestWildcatterCommodity(ci) == cid)
			return ci;
	return -1;
}

// Systems of a spacecraft that contribute hit points and thus become susceptible to damage.
// NOTE: The texture HudItems.png has a 1:1 set of texture to systems.
// NOTE: Systems that can be multiples on a ship are further identified by a 'which' identifier, always a quint16. See AuSysId.
enum SYSTEM	
{
	SYS_CommandAndControl,	// One per console. 'which' is detail index in design data, sorted by deck,rid,id.
	SYS_Fuel,				// Hull for Fuel Storage
	SYS_ManeuverDrive,		// Manuever drive units
	SYS_Hold,				// Hull for Cargo Storage
	SYS_Hull,				// Hull for Living Spaces
	SYS_VehicleLaunch,		// Space vehicle launchers.
	SYS_LifeSupport,		// Life support units
	SYS_PowerPlant,			// Power plant units
	SYS_Sensor,				// Sensor units
	SYS_Shield,				// Shield units
	SYS_SurgeryUnit,		// Surgery units. 'which' is detail index in design data, sorted by deck,rid,id.
	SYS_Transporter,		// Transporter units. 'which' is encoded room id. which = (roomDeck << 8) | roomSub
	SYS_Turret,				// Turrets. 'which' is detail index in design data, sorted by deck,rid,id.
	SYS_Weapon,				// Weapon units. 'which' is roomSub.
	SYS_FTLDrive,			// FTL drive units
	SYS_Capacitor,			// Capacitor units.
	SYS_VehicleLanding,		// Space vehicle landing/recovery.
	SYS_Count				// LAST ENTRY FOR LIMIT/COUNT
};

// Returns generic name for nonspecific system.
inline QString BdSystemName(SYSTEM system)
{
	if(system >= 0 && system < SYS_Count)
	{
		const QString systems[SYS_Count] =
		{
			QObject::tr("Control Station"),
			QObject::tr("Fuel Storage"),
			QObject::tr(""),	// Maneuver drive n/a in building
			QObject::tr("Commodity Storage"),
			QObject::tr("Building Structure"),
			QObject::tr("Aircraft Launcher"),
			QObject::tr("Life Support"),
			QObject::tr("Generator"),
			QObject::tr("Radar"),
			QObject::tr("Shield"),
			QObject::tr("Medical Unit"),
			QObject::tr("Transporter"),
			QObject::tr("Turret"),
			QObject::tr("Weapon"),
			QObject::tr(""),	// FTL n/a in building
			QObject::tr("Battery"),
			QObject::tr("Aircraft Recovery System"),
		};
		return systems[system];
	}
	else
		return QString("");
}

// Returns generic name for nonspecific system.
inline QString ScSystemName(SYSTEM system)
{
	if(system >= 0 && system < SYS_Count)
	{
		const QString systems[SYS_Count] =
		{
			QObject::tr("Control Station"),
			QObject::tr("Fuel Cell"),
			QObject::tr("Maneuver Drive"),
			QObject::tr("Hold"),
			QObject::tr("Hull"),
			QObject::tr("Space Vehicle Launcher"),
			QObject::tr("Life Support"),
			QObject::tr("Power Plant"),
			QObject::tr("Sensor"),
			QObject::tr("Shield"),
			QObject::tr("Medical Unit"),
			QObject::tr("Transporter"),
			QObject::tr("Turret"),
			QObject::tr("Weapon"),
			QObject::tr("FTL Drive"),
			QObject::tr("Capacitor"),
			QObject::tr("Space Vehicle Recovery System"),
		};
		return systems[system];
	}
	else
		return QString("");
}

inline QString SystemIcon(SYSTEM system)
{
	if(system >= 0 && system < SYS_Count)
	{
		QString systemIcons[] =
		{
			":data/dp_CmdConsole.png",
			":data/MsgSolicitFuel.png",			// Fuel = Hull
			":data/dp_ManeuverDrive.png",
			":data/c_Crate.png",				// Hold = Hull
			":data/ScHull.png",					// Hull = Hull
			":data/ScLaunchSpaceVehicle.png",	// SYS_VehicleLaunch
			":data/dp_LifeSupport.png",
			":data/dp_PowerPlant.png",
			":data/dp_Sensor.png",
			":data/dp_Shield.png",
			":data/dp_OperatingTable.png",
			":data/dp_TransporterPad.png",
			":data/dp_TurretTopBottom.png",
			":data/dp_Weapon.png",
			":data/dp_WormholeDrive.png",
			":data/dp_Capacitor.png",
			":data/ScLandingSystem.png",		// SYS_VehicleLanding
		};
		return systemIcons[system];
	}
	return QString();
}

class AuShipSystemHash
{
public:
	inline size_t operator()(const SYSTEM& o) const
	{
		return hash<unsigned int>()(o);
	}
};

class AuSysId
{
	SYSTEM					m_System;
	quint16					m_Which;

public:	// Construction
	inline AuSysId()
	{
		m_System = SYS_Count;
		m_Which = 0;
	}

	inline AuSysId(SYSTEM sys)
	{
		m_System = sys;
		m_Which = 0;
	}

	inline AuSysId(SYSTEM sys, quint16 which)
	{
		m_System = sys;
		m_Which = which;
	}

public:	// Operators
	inline bool operator==(const AuSysId& c) const
	{
		if(m_System != c.m_System)
			return false;
		if(m_Which != c.m_Which)
			return false;
		return true;
	}

	inline void operator>>(QDataStream& out) const
	{
		out << (quint8)m_System;
		out << m_Which;
	}

	inline void operator<<(QDataStream& in)
	{
		quint8 ui8;
		in >> ui8;
		m_System = (SYSTEM)ui8;
		in >> m_Which;
	}

public:	// Methods
	inline QString BdName() const
	{
		return BdSystemName(System());
	}

	inline unsigned int HashKey() const
	{
		return (m_System << 16) | m_Which;
	}

	inline bool IsStructural() const
	{
		return m_System == SYS_Hull || m_System == SYS_Hold || m_System == SYS_Fuel;
	}

	COMMODITYID RandomPart(COMMODITYID systemModule) const;

	inline QString ScName() const
	{
		return ScSystemName(System());
	}

	inline SYSTEM System() const
	{
		return m_System;
	}

	inline QString Icon() const
	{
		return ::SystemIcon(m_System);
	}

	inline quint16 Which() const
	{
		return m_Which;
	}
};

inline QDataStream& operator<<(QDataStream& out, const AuSysId& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuSysId& o)
{
	o << in;
	return in;
}

QDebug operator<<(QDebug dbg, const AuSysId& sysId);

inline unsigned int qHash(const AuSysId& key)
{
	return key.HashKey();
}

inline SYSTEM BdModuleSystem(COMMODITYID module)
{
	switch(module)
	{
	default:
		return SYS_Count;				// Invalid return
	case CID_CapacitorModule:
		return SYS_Capacitor;
	case CID_CommandAndControlModule:
		return SYS_CommandAndControl;
	case CID_WoodHullModule:
	case CID_LumberHullModule:
	case CID_StoneHullModule:
	case CID_MetalHullModule:
	case CID_MagmiumHullModule:
	case CID_VulciumHullModule:
	case CID_AdamantineHullModule:
		return SYS_Hull;
	case CID_LifeSupportModule:
	case CID_LifeSupportGravityModule:
		return SYS_LifeSupport;
	case CID_CoalPowerPlantModule:
	case CID_GasolinePowerPlantModule:
	case CID_HydroelectricPowerPlantModule:
	case CID_HydrogenPowerPlantModule:
	case CID_NaturalGasPowerPlantModule:
	case CID_NuclearPowerPlantModule:
	case CID_SolarPowerPlantModule:
	case CID_WindPowerPlantModule:
		return SYS_PowerPlant;
	case CID_EnergyShieldModule:
	case CID_KineticShieldModule:
	case CID_CloakingShieldModule:
		return SYS_Shield;
	case CID_SurgeryMedicalModule:
	case CID_GeneticsMedicalModule:
	case CID_CyberneticsMedicalModule:
		return SYS_SurgeryUnit;
	case CID_PersonnelTransporterModule:
	case CID_VehicleTransporterModule:
		return SYS_Transporter;
	case CID_BallisticWeaponTurretModule:
	case CID_EnergyWeaponTurretModule:
	case CID_WeaponLauncherTurretModule:
		return SYS_Turret;
	//case CID_AtmosphereCondenserBayModule:
	//case CID_ParticleCollectorBayModule:
	//case CID_ProspectorBayModule:
	//case CID_SiphonBayModule:
	//case CID_WildcatterBayModule:
	//case CID_TractorBeamBayModule:
	case CID_EnergyWeaponBayModule:
	//case CID_WeaponDeploymentBayModule:
	case CID_BallisticWeaponBayModule:
	case CID_WeaponLauncherBayModule:
		return SYS_Weapon;
	case CID_SensorModule:
		return SYS_Sensor;
	case CID_ElectronicPart:
		return SYS_VehicleLanding;
	case CID_MechanicalPart:
		return SYS_VehicleLaunch;
	}
}

// Returns the hull module that corresponds to a hull material.
inline COMMODITYID HullMaterialModule(COMMODITYID hullMaterial)
{
	switch(hullMaterial)
	{
	default:
		break;
	case CID_Wood:
		return CID_WoodHullModule;
	case CID_Lumber:
		return CID_LumberHullModule;
	case CID_Stone:
		return CID_StoneHullModule;
	case CID_Metal:
		return CID_MetalHullModule;
	case CID_Magmium:
		return CID_MagmiumHullModule;
	case CID_Vulcium:
		return CID_VulciumHullModule;
	case CID_Adamantine:
		return CID_AdamantineHullModule;
	}
	return CID_Invalid;
}

// Returns the hull material that corresponds to a hull module.
inline COMMODITYID HullModuleMaterial(COMMODITYID hullModule)
{
	switch(hullModule)
	{
	default:
		break;
	case CID_WoodHullModule:
		return CID_Wood;
	case CID_LumberHullModule:
		return CID_Lumber;
	case CID_StoneHullModule:
		return CID_Stone;
	case CID_MetalHullModule:
		return CID_Metal;
	case CID_MagmiumHullModule:
		return CID_Magmium;
	case CID_VulciumHullModule:
		return CID_Vulcium;
	case CID_AdamantineHullModule:
		return CID_Adamantine;
	}
	return CID_Invalid;
}

inline SYSTEM ScModuleSystem(COMMODITYID module)
{
	switch(module)
	{
	default:
		return SYS_Count;				// Invalid return
	case CID_CapacitorModule:
		return SYS_Capacitor;
	case CID_CommandAndControlModule:
		return SYS_CommandAndControl;
	case CID_GravityDriveModule:
	case CID_RocketDriveModule:
		return SYS_ManeuverDrive;
	case CID_MetalHullModule:
	case CID_MagmiumHullModule:
	case CID_VulciumHullModule:
	case CID_AdamantineHullModule:
		return SYS_Hull;
	case CID_LifeSupportModule:
	case CID_LifeSupportGravityModule:
		return SYS_LifeSupport;
	case CID_HydrogenPowerPlantModule:
		return SYS_PowerPlant;
	case CID_EnergyShieldModule:
	case CID_KineticShieldModule:
	case CID_CloakingShieldModule:
		return SYS_Shield;
	case CID_SurgeryMedicalModule:
	case CID_GeneticsMedicalModule:
	case CID_CyberneticsMedicalModule:
		return SYS_SurgeryUnit;
	case CID_PersonnelTransporterModule:
	case CID_VehicleTransporterModule:
		return SYS_Transporter;
	case CID_BallisticWeaponTurretModule:
	case CID_EnergyWeaponTurretModule:
	case CID_WeaponLauncherTurretModule:
		return SYS_Turret;
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
		return SYS_Weapon;
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
		return SYS_FTLDrive;
	case CID_SensorModule:
		return SYS_Sensor;
	case CID_ElectronicPart:
		return SYS_VehicleLanding;
	case CID_MechanicalPart:
		return SYS_VehicleLaunch;
	}
}

class AUGLOBAL_DLL DbCommodity
{
friend class DbCommodityArray;
public:
	COMMODITYID			m_id;
	quint8				m_ammo1Qty;			// Round count for weapons that use ammo. ZERO FOR AMMO COMMODITIES
	AMMOTYPE			m_ammo1Type;
	quint8				m_ammo2Qty;			// Round count for weapons that use ammo. ZERO FOR AMMO COMMODITIES
	AMMOTYPE			m_ammo2Type;
protected:
	float								m_ArmorEffectAbsorbAcid;
	float								m_ArmorEffectAbsorbBludgeon;
	float								m_ArmorEffectAbsorbCold;
	float								m_ArmorEffectAbsorbCrush;
	float								m_ArmorEffectAbsorbDisease;
	float								m_ArmorEffectAbsorbElectric;
	float								m_ArmorEffectAbsorbHeat;
	float								m_ArmorEffectAbsorbImplant;
	float								m_ArmorEffectAbsorbPierce;
	float								m_ArmorEffectAbsorbPoison;
	float								m_ArmorEffectAbsorbRadiation;
	float								m_ArmorEffectAbsorbSlash;
	float								m_ArmorEffectAbsorbSonic;
	float								m_ArmorEffectAbsorbStun;
	float								m_ArmorEffectAbsorbSuffocate;
	float								m_ArmorEffectBlockAcid;
	float								m_ArmorEffectBlockBludgeon;
	float								m_ArmorEffectBlockCold;
	float								m_ArmorEffectBlockCrush;
	float								m_ArmorEffectBlockDisease;
	float								m_ArmorEffectBlockElectric;
	float								m_ArmorEffectBlockHeat;
	float								m_ArmorEffectBlockImplant;
	float								m_ArmorEffectBlockPierce;
	float								m_ArmorEffectBlockPoison;
	float								m_ArmorEffectBlockRadiation;
	float								m_ArmorEffectBlockSlash;
	float								m_ArmorEffectBlockSonic;
	float								m_ArmorEffectBlockStun;
	float								m_ArmorEffectBlockSuffocate;
	float								m_ArmorModeAbsorbArea;
	float								m_ArmorModeAbsorbContact;
	float								m_ArmorModeAbsorbProjectile;
	float								m_ArmorModeAbsorbPsionic;
	float								m_ArmorModeBlockArea;
	float								m_ArmorModeBlockContact;
	float								m_ArmorModeBlockProjectile;
	float								m_ArmorModeBlockPsionic;
	quint8								m_armorSpec;			// WEAPONEFFECT of specialty, armor wears down less vs effect.
public:
	quint32				m_bitsWear;			// OR'd INVENTORYPOS bits
	QSize				m_capacity;			// from capwid and capht fields
	quint32				m_combits;			// OR'd COMMODITYBITS from AuDef
	quint8				m_food;				// Food nutrition value of one unit.
	qint8				m_health;			// Health effect of usage.
	QString				m_iconPath;			// Path to small square image, 32x32 pixels, png
	QString				m_modelPath;		// Path to the model used for this item 
	float				m_insertHeight;		// Height above insertion point when on the ground and up vector applied
	AuPoint				m_UpVector;			// Which way up is in the model
	quint8				m_light;			// Light radius in 10s of feet
	QString				m_name;
	quint8				m_newcond;			// Condition points when nu.
	QSize				m_size;				// from sizewid and sizeht fields
	quint8				m_stackLimit;
	WEAPONMODE			m_wpn1Mode;
	quint16				m_wpn1Range;
	WEAPONEFFECT		m_wpn1Effect1;
	quint16				m_wpn1Damage1;
	quint8				m_wpn1DamageMod1;
	quint16				m_wpn1Radius1;
	quint8				m_wpn1AP1;			// Armor Piercing percentage points. Negates armor absorption.
	quint16				m_wpn1ST1;			// Strike Through points. Negates armor block.
	WEAPONEFFECT		m_wpn1Effect2;
	quint16				m_wpn1Damage2;
	quint8				m_wpn1DamageMod2;
	quint16				m_wpn1Radius2;
	quint8				m_wpn1AP2;			// Armor Piercing percentage points. Negates armor absorption.
	quint16				m_wpn1ST2;			// Strike Through points. Negates armor block.
	WEAPONMODE			m_wpn2Mode;
	quint16				m_wpn2Range;
	WEAPONEFFECT		m_wpn2Effect1;
	quint16				m_wpn2Damage1;
	quint8				m_wpn2DamageMod1;
	quint16				m_wpn2Radius1;
	quint8				m_wpn2AP1;			// Armor Piercing percentage points. Negates armor absorption.
	quint16				m_wpn2ST1;			// Strike Through points. Negates armor block.
	WEAPONEFFECT		m_wpn2Effect2;
	quint16				m_wpn2Damage2;
	quint8				m_wpn2DamageMod2;
	quint16				m_wpn2Radius2;
	quint8				m_wpn2AP2;			// Armor Piercing percentage points. Negates armor absorption.
	quint16				m_wpn2ST2;			// Strike Through points. Negates armor block.
	float				m_wt;				// in pounds for each unit

private:
	float				m_baseCost;	// Not a database value. Calculated after database loaded in DgData. -1 until calculated.

public:	// Construction
	DbCommodity();
	~DbCommodity();
	DbCommodity(const DbCommodity &c);
	DbCommodity& operator=(const DbCommodity &c);

public:	// Methods
	inline int AimAxis() const
	{
		return AimAxis(m_id);
	}

	// Returns the axis that is aligned with the hand aim direction when this item is held in hand.
	// Returns 0=X, 1=Y, 2=Z
	static int AimAxis(COMMODITYID cid);

	// Returns the fraction of overall damage absorbed by this armor vs a weapon 'effect', in the range 0 to 1.
	inline float ArmorEffectAbsorb(WEAPONEFFECT effect) const
	{
		switch(effect)
		{
		case WE_None:
		case WE_Invalid:
		case WE_Bleeding:
		case WE_Starving:
		case WE_Corrosion:
		case WE_Audible:
		case WE_Unused:
			// These damage effects are internal, not intended to be blocked by armor.
			break;
		case WE_Acid:
			return m_ArmorEffectAbsorbAcid;
		case WE_Suffocate:
			return m_ArmorEffectAbsorbSuffocate;
		case WE_Stun:
			return m_ArmorEffectAbsorbStun;
		case WE_Poison:
			return m_ArmorEffectAbsorbPoison;
		case WE_Radiation:
			return m_ArmorEffectAbsorbRadiation;
		case WE_Disease:
			return m_ArmorEffectAbsorbDisease;
		case WE_Electric:
			return m_ArmorEffectAbsorbElectric;
		case WE_Bludgeon:
			return m_ArmorEffectAbsorbBludgeon;
		case WE_Cold:
			return m_ArmorEffectAbsorbCold;
		case WE_Crush:
			return m_ArmorEffectAbsorbCrush;
		case WE_Sonic:
			return m_ArmorEffectAbsorbSonic;
		case WE_Heat:
			return m_ArmorEffectAbsorbHeat;
		case WE_Pierce:
			return m_ArmorEffectAbsorbPierce;
		case WE_Implant:
			return m_ArmorEffectAbsorbImplant;
		case WE_Slash:
			return m_ArmorEffectAbsorbSlash;
		case WE_Generic:
			return 0;
		}
		return 0;
	}

	// Returns the amount of damage absorbed by this armor vs a weapon 'effect', in the range 0 to n.
	inline float ArmorEffectBlock(WEAPONEFFECT effect) const
	{
		switch(effect)
		{
		case WE_None:
		case WE_Invalid:
		case WE_Bleeding:
		case WE_Starving:
		case WE_Corrosion:
		case WE_Audible:
		case WE_Unused:
			// These damage effects are internal, not intended to be blocked by armor.
			break;
		case WE_Acid:
			return m_ArmorEffectBlockAcid;
		case WE_Suffocate:
			return m_ArmorEffectBlockSuffocate;
		case WE_Stun:
			return m_ArmorEffectBlockStun;
		case WE_Poison:
			return m_ArmorEffectBlockPoison;
		case WE_Radiation:
			return m_ArmorEffectBlockRadiation;
		case WE_Disease:
			return m_ArmorEffectBlockDisease;
		case WE_Electric:
			return m_ArmorEffectBlockElectric;
		case WE_Bludgeon:
			return m_ArmorEffectBlockBludgeon;
		case WE_Cold:
			return m_ArmorEffectBlockCold;
		case WE_Crush:
			return m_ArmorEffectBlockCrush;
		case WE_Sonic:
			return m_ArmorEffectBlockSonic;
		case WE_Heat:
			return m_ArmorEffectBlockHeat;
		case WE_Pierce:
			return m_ArmorEffectBlockPierce;
		case WE_Implant:
			return m_ArmorEffectBlockImplant;
		case WE_Slash:
			return m_ArmorEffectBlockSlash;
		case WE_Generic:
			return 0;
		}
		return 0;
	}

	// Returns the fraction of overall damage absorbed by this armor vs a weapon 'mode', in the range 0 to 1.
	inline float ArmorModeAbsorb(WEAPONMODE mode) const
	{
		switch(mode)
		{
		case WM_Not:
		case WM_Internal:
		default:
			// These damage modes are internal, not intended to be blocked by armor.
			return 0;
		case WM_Area:
		case WM_Detonator:
		case WM_Fuse:
			return m_ArmorModeAbsorbArea;
		case WM_Contact:
			return m_ArmorModeAbsorbContact;
		case WM_Projectile:
			return m_ArmorModeAbsorbProjectile;
		case WM_Psionic:
			return m_ArmorModeAbsorbPsionic;
		}
	}

	// Returns the amount of damage absorbed by this armor vs a weapon 'mode', in the range 0 to n.
	inline float ArmorModeBlock(WEAPONMODE mode) const
	{
		switch(mode)
		{
		case WM_Not:
		case WM_Internal:
		default:
			// These damage modes are internal, not intended to be blocked by armor.
			return 0;
		case WM_Area:
		case WM_Detonator:
		case WM_Fuse:
			return m_ArmorModeBlockArea;
		case WM_Contact:
			return m_ArmorModeBlockContact;
		case WM_Projectile:
			return m_ArmorModeBlockProjectile;
		case WM_Psionic:
			return m_ArmorModeBlockPsionic;
		}
	}

	inline WEAPONEFFECT ArmorSpec() const
	{
		return (WEAPONEFFECT)m_armorSpec;
	}

	// Returns calculated base cost or -1 if not calculated.
	inline float BaseCost() const
	{
		return m_baseCost;
	}

	// Returns the small square resource bitmap id used to represent this
	// component on signs and things.
	inline const QString BitmapName() const
	{
		return m_iconPath;
	}

	// True if this item can be stored in a cargo list such as a person's inventory, the inventory of a city, or the hold of a ship.
	inline bool CanBeCargo() const
	{
		if(!(m_combits & CB_Cargo))
			return false;										// Item can't be bought|sold as cargo.
		if(m_size.width() <= 0 || m_size.height() <= 0 || m_size.width() > 12 || m_size.height() > 20)
			return false;										// Size required for cargo items, from 1x1 to 12x20 boxes in size.
		return true;
	}

	// Returns true if the item is food or liquor, regardless of whether it has any food or intoxicant value.
	inline bool CanBeEaten() const
	{
		return CanBeEaten(m_combits);
	}

	// Returns true if the item is food or liquor, regardless of whether it has any food or intoxicant value.
	static inline bool CanBeEaten(quint32 combits)
	{
		return (combits & (CB_Food|CB_Liquor)) ? true : false;
	}

	static bool CanCombine(COMMODITYID cid);

	static quint8 DataStorageSpace(COMMODITYID storageMediaCID, quint8 quality);

	// Returns true if this commodity makes an explosion sound when it hits a target, instead of a slicing or slashing sound.
	inline bool HitMakesExplosionSound() const
	{
		return HitMakesExplosionSound(m_id);
	}

	// Returns true if a commodity id makes an explosion sound when it hits a target, instead of a slicing or slashing sound.
	static bool HitMakesExplosionSound(COMMODITYID cid);

	inline bool IsAmmo() const
	{
		return IsAmmo(m_ammo1Type, m_ammo1Qty);
	}

	static inline bool IsAmmo(AMMOTYPE ammoType, quint8 ammoQty)
	{
		// Weapons and ammo both use the m_ammo1Type field.
		// Ammo can be distinguished because it always has a quantity of zero. The weapon determines the quantity when it is loaded.
		return (ammoType != AMMO_None && ammoType != AMMO_Explosive && ammoQty == 0) ? true : false;
	}

	// Returns true if 'itemCID' can be loaded as ammunition into 'weaponCID', which is expected to be a bay module,
	// a turret module, or a vehicle cid.
	static inline bool IsAmmunitionForWeapon(COMMODITYID weaponCID, COMMODITYID itemCID)
	{
		switch(weaponCID)
		{
		case CID_BallisticWeaponTurretModule:
		case CID_BallisticWeaponBayModule:
			return IsFiredFromHeavyWeapon(itemCID);

		case CID_EnergyWeaponTurretModule:
		case CID_EnergyWeaponBayModule:
			return IsEnergyWeapon(itemCID);

		case CID_WeaponDeploymentBayModule:
			return IsDeployableWeapon(itemCID);

		case CID_WeaponLauncherBayModule:
		case CID_WeaponLauncherTurretModule:
			return IsRocket(itemCID) || IsMissile(itemCID);

		case CID_AdvSpaceFighter:
		case CID_SpaceFighter:
		case CID_SpaceTransport:
		case CID_APC:
		case CID_Tank:
			return IsVehicleAmmunition(itemCID);

		default:
			break;
		}
		return false;
	}

	// Includes helmet.
	inline bool IsArmorOrHelmet() const
	{
		return IsArmorOrHelmet(m_bitsWear);
	}

	// Includes helmet.
	static inline bool IsArmorOrHelmet(quint32 bitsWear)
	{
		return (bitsWear & (IPOSBIT(IP_ArmorArms)|
								IPOSBIT(IP_ArmorFace)|
								IPOSBIT(IP_ArmorFeet)|
								IPOSBIT(IP_ArmorHands)|
								IPOSBIT(IP_ArmorHead)|
								IPOSBIT(IP_ArmorLegs)|
								IPOSBIT(IP_ArmorLowerTorso)|
								IPOSBIT(IP_ArmorUpperTorso)));
	}

	// Includes helmets and hats.
	inline bool IsArmorHelmetClothingOrHat() const
	{
		return IsArmorHelmetClothingOrHat(m_bitsWear);
	}

	static inline bool IsArmorHelmetClothingOrHat(quint32 bitsWear)
	{
		return (bitsWear & (IPOSBIT(IP_ArmorArms)|
								IPOSBIT(IP_ArmorFace)|
								IPOSBIT(IP_ArmorFeet)|
								IPOSBIT(IP_ArmorHands)|
								IPOSBIT(IP_ArmorHead)|
								IPOSBIT(IP_ArmorLegs)|
								IPOSBIT(IP_ArmorLowerTorso)|
								IPOSBIT(IP_ArmorUpperTorso)|
								IPOSBIT(IP_ClothingArms)|
								IPOSBIT(IP_ClothingFace)|
								IPOSBIT(IP_ClothingHead)|
								IPOSBIT(IP_ClothingLegs)|
								IPOSBIT(IP_ClothingLowerTorso)|
								IPOSBIT(IP_ClothingUpperTorso)));
	}

	// Return true if wild animals will ignore people and food within 15' of this item when it is in the environment.
	static inline bool IsAvoidedByWildAnimals(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_BurningBarrel:
		case CID_PressureTent:
		case CID_StoneFirePit:
		case CID_Tepee:
			return true;
		default:
			break;
		}
		return false;
	}

	// Test for armor that goes on the body, not the head.
	inline bool IsBodyArmor() const
	{
		return IsBodyArmor(m_bitsWear);
	}

	// Test for armor that goes on the body, not the head.
	static inline bool IsBodyArmor(quint32 bitsWear)
	{
		return (bitsWear & (IPOSBIT(IP_ArmorArms)|
								IPOSBIT(IP_ArmorFace)|
								IPOSBIT(IP_ArmorFeet)|
								IPOSBIT(IP_ArmorHands)|
								//IPOSBIT(IP_ArmorHead)|
								IPOSBIT(IP_ArmorLegs)|
								IPOSBIT(IP_ArmorLowerTorso)|
								IPOSBIT(IP_ArmorUpperTorso)));
	}

	inline bool IsBrokerItem() const
	{
		return m_combits & CB_Cargo;
	}

	inline bool IsCantinaItem() const
	{
		return m_combits & CB_Cantina;
	}

	// Test for clothing that goes on the body, not the head.
	inline bool IsClothing() const
	{
		return IsClothing(m_bitsWear);
	}

	// Test for clothing that goes on the body, not the head.
	static inline bool IsClothing(quint32 bitsWear)
	{
		return (bitsWear & (IPOSBIT(IP_ClothingArms)|
								IPOSBIT(IP_ClothingFace)|
								//IPOSBIT(IP_ClothingHead)|
								IPOSBIT(IP_ClothingLegs)|
								IPOSBIT(IP_ClothingLowerTorso)|
								IPOSBIT(IP_ClothingUpperTorso)));
	}

	static inline bool IsFilterValid(COMMODITYFILTER filter)
	{
		return filter >= FILTER_All &&
			filter < FILTER_Count;
	}

	inline bool IsGroceryItem() const
	{
		return m_combits & CB_Grocery;
	}

	static inline bool IsGroundVehicle(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_APC:
		case CID_DirtBike:
		case CID_FarmMachinery:
		case CID_SUV:
			return true;
		case CID_AdvSpaceFighter:
		case CID_Helicopter:
		case CID_SailBoat:
		case CID_SpaceFighter:
		case CID_SpaceRocket:
		case CID_SpaceTransport:
		case CID_Submarine:
		default:
			break;
		}
		return false;
	}

	// Returns true for items that are imported by hospital buildings.
	bool IsHospitalItem() const;

	// Returns true for items that are imported by military air terminal buildings.
	bool IsMilitaryAirBaseItem() const;

	// Returns true for items that are imported by military dock buildings.
	bool IsMilitaryDockItem() const;

	// Returns true for items that are imported by military HQ buildings.
	bool IsMilitaryHQItem() const;

	// Returns true for items that are imported by military motor pool buildings.
	bool IsMilitaryMotorPoolItem() const;

	inline bool IsPreon() const
	{
		return IsPreon(m_id);
	}

	static bool IsPreon(COMMODITYID cid);

	inline bool IsRetailItem() const
	{
		return m_combits & CB_Retail;
	}

	// Note that a space rocket is also a space vehicle.
	static inline bool IsSpaceRocket(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_SpaceRocket:
			return true;
		case CID_APC:
		case CID_AdvSpaceFighter:
		case CID_DirtBike:
		case CID_FarmMachinery:
		case CID_Helicopter:
		case CID_SUV:
		case CID_SailBoat:
		case CID_SpaceFighter:
		case CID_SpaceTransport:
		case CID_Submarine:
		default:
			break;
		}
		return false;
	}

	static inline bool IsSpaceVehicle(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_AdvSpaceFighter:
		case CID_SpaceFighter:
		case CID_SpaceRocket:
		case CID_SpaceTransport:
			return true;
		case CID_APC:
		case CID_DirtBike:
		case CID_FarmMachinery:
		case CID_Helicopter:
		case CID_SUV:
		case CID_SailBoat:
		case CID_Submarine:
		default:
			break;
		}
		return false;
	}

	static inline bool IsWarpFTLDriveModule(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_Warp1FTLDriveModule:
		case CID_Warp2FTLDriveModule:
		case CID_Warp3FTLDriveModule:
		case CID_Warp4FTLDriveModule:
		case CID_Warp5FTLDriveModule:
		case CID_Warp6FTLDriveModule:
		case CID_Warp7FTLDriveModule:
		case CID_Warp8FTLDriveModule:
		case CID_Warp9FTLDriveModule:
			return true;
		default:
			break;
		}
		return false;
	}

	static inline bool IsWaterVehicle(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_SailBoat:
		case CID_Submarine:
			return true;
		case CID_APC:
		case CID_AdvSpaceFighter:
		case CID_DirtBike:
		case CID_FarmMachinery:
		case CID_Helicopter:
		case CID_SUV:
		case CID_SpaceFighter:
		case CID_SpaceRocket:
		case CID_SpaceTransport:
		default:
			break;
		}
		return false;
	}


	static inline bool IsWormholeFTLDriveModule(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_WormholePFTLDriveModule:
		case CID_WormholePNFTLDriveModule:
		case CID_WormholePNNFTLDriveModule:
			return true;
		default:
			break;
		}
		return false;
	}

	// Test for clothing that goes on the head, not the body.
	inline bool IsHat() const
	{
		return IsHat(m_bitsWear);
	}

	// Test for clothing that goes on the head, not the body.
	static inline bool IsHat(quint32 bitsWear)
	{
		return (bitsWear & IPOSBIT(IP_ClothingHead));
	}

	// Test for armor that goes on the head, not the body.
	inline bool IsHelmet() const
	{
		return IsHelmet(m_bitsWear);
	}

	// Test for armor that goes on the head, not the body.
	static inline bool IsHelmet(quint32 bitsWear)
	{
		return (bitsWear & IPOSBIT(IP_ArmorHead));
	}

	// Return true if wild animals will view the item as a threat when it is in the environment.
	static inline bool IsThreatToWildAnimals(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_BurningBarrel:
		case CID_StoneFirePit:
			return true;
		default:
			break;
		}
		return false;
	}

	static inline bool IsBayWeaponModule(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_BallisticWeaponBayModule:
		case CID_EnergyWeaponBayModule:
		case CID_WeaponDeploymentBayModule:
		case CID_WeaponLauncherBayModule:
			return true;
		default:
			break;
		}
		return false;
	}

	inline bool IsBdDesign() const
	{
		return (m_id == CID_BdDesignDisk || m_id == CID_BdDesignDrawing) ? true : false;
	}

	static inline bool IsCarnivoreFood(COMMODITYID cid)
	{
		switch(cid)
		{
		default:
			break;
		case CID_Animal:
		case CID_AnimalMeat:
		case CID_Brain:
		case CID_Cheese:
		case CID_Eggs:
		case CID_Fish:
		case CID_FishMeat:
		case CID_Gills:
		case CID_Head:
		case CID_Heart:
		case CID_IceCream:
		case CID_Lungs:
		case CID_Milk:
		case CID_Stomach:
		case CID_Water:			// All creatures can eat water.
			return true;
		}
		return false;
	}

	inline bool IsConstructionMaterial() const
	{
		return IsConstructionMaterial(m_id);
	}

	static inline bool IsConstructionMaterial(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_Adamantine:
		case CID_Computer:
		case CID_CrudeOil:
		case CID_ElectronicPart:
		case CID_GravCoupling:	// For gas giant construction.
		case CID_Lumber:
		case CID_Magmium:
		case CID_MechanicalPart:
		case CID_Metal:
		case CID_Plastic:
		case CID_Stone:
		case CID_Textiles:
		case CID_Vulcium:
		case CID_Wood:
			return true;
		default:
			return false;
		}
	}

	static inline bool IsDeployableWeapon(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_BombBl:
		case CID_BombBlRa:
		case CID_BombBlPi:
		case CID_BombElSt:
		case CID_BombSuPo:
		case CID_BombSlCr:
			return true;
		default:
			break;
		}
		return false;
	}

	static inline bool IsEnergyWeapon(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_StasisBeam:
		case CID_NeutronBeam:
		case CID_ElectronBeam:
		case CID_ErgRay:
		case CID_FreezeRay:
		case CID_CompressorBeam:
		case CID_DisruptorBeam:
		case CID_HeatRay:
		case CID_ProtonBeam:
		case CID_DisintegratorRay:
			return true;
		default:
			return false;
		}
	}

	// True for items that cannot be in someone's inventory.
	// These items only exist in the environment, so processes can depend on them and fetch them from the environment.
	inline bool IsEnvironmentalFactor() const
	{
		return IsEnvironmentalFactor(m_id);
	}

	inline static bool IsEnvironmentalFactor(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_Atmosphere:
		case CID_FrigidAtmosphere:
		case CID_Hydrographics:
		case CID_InfernoAtmosphere:
		case CID_Magma:
		case CID_SupercooledLiquid:
		case CID_Vegetation:
		case CID_Sunlight:
			return true;
		default:
			return false;
		}
	}

	inline bool IsEnvironmentHelmet() const
	{
		return IsEnvironmentHelmet(m_id);
	}

	inline static bool IsEnvironmentHelmet(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_VacSuitHelmet:
		case CID_MetalPowerArmorHelmet:
		case CID_MagmiumPowerArmorHelmet:
		case CID_VulciumPowerArmorHelmet:
			return true;
		default:
			return false;
		}
	}

	inline bool IsEnvironmentSuit() const
	{
		return IsEnvironmentSuit(m_id);
	}

	inline static bool IsEnvironmentSuit(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_VacSuit:
		case CID_MetalPowerArmor:
		case CID_MagmiumPowerArmor:
		case CID_VulciumPowerArmor:
			return true;
		default:
			return false;
		}
	}

	// True for commodities that are fired from a gun, like a bullet. Not things with their own acceleration.
	static inline bool IsFiredFromHeavyWeapon(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_HeavyArmsAmmoPi:
		case CID_HeavyArmsAmmoPiAc:
		case CID_HeavyArmsAmmoPiBl:
		case CID_HeavyArmsAmmoPiRa:
			return true;
		default:
			break;
		}
		return false;
	}

	inline bool IsFlowerPot()
	{
		return IsFlowerPot(m_id);
	}

	static inline bool IsFlowerPot(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_FlowerPotLarge:
		case CID_FlowerPotSmall:
			return true;
		default:
			break;
		}
		return false;
	}

	inline bool IsFood() const
	{
		return IsFood(m_combits);
	}

	static inline bool IsFood(quint32 combits)
	{
		return (combits & CB_Food) ? true : false;
	}

	inline bool IsFood(ECOROLE ecoRole) const
	{
		return IsFood(m_id, m_combits, ecoRole);
	}

	static inline bool IsFood(COMMODITYID cid, quint32 combits, ECOROLE ecoRole)
	{
		if(IsFood(combits))
		{
			switch(ecoRole)
			{
			case ECO_Carnivore:
				return IsCarnivoreFood(cid);
			case ECO_Herbivore:
				return IsHerbivoreFood(cid);
			case ECO_Omnivore:
				return IsOmnivoreFood(cid);
			case ECO_Scavenger:
				return IsScavengerFood(cid);
			}
		}
		return false;
	}

	static inline bool IsFreeToFriendlyGovShip(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_BombBl:
		case CID_BombBlRa:
		case CID_BombBlPi:
		case CID_BombElSt:
		case CID_BombSuPo:
		case CID_BombSlCr:
		case CID_Electricity:			// Prevent people from selling electricity generated using their free fuel.
		case CID_HeavyArmsAmmoPi:
		case CID_HeavyArmsAmmoPiAc:
		case CID_HeavyArmsAmmoPiBl:
		case CID_HeavyArmsAmmoPiRa:
		case CID_Hydrogen:
		case CID_LargeMissileBl:
		case CID_LargeRocketBl:
		case CID_SmallMissileBl:
		case CID_SmallRocketBl:
		case CID_LargeMissileBlBu:
		case CID_LargeRocketBlBu:
		case CID_SmallMissileBlBu:
		case CID_SmallRocketBlBu:
		case CID_SmallRocketSlPi:
		case CID_LargeRocketBuDi:
		case CID_SmallMissileCrFr:
		case CID_LargeMissileSlPo:
		case CID_StasisBeam:
		case CID_NeutronBeam:
		case CID_ElectronBeam:
		case CID_ErgRay:
		case CID_FreezeRay:
		case CID_CompressorBeam:
		case CID_DisruptorBeam:
		case CID_HeatRay:
		case CID_ProtonBeam:
		case CID_DisintegratorRay:
			return true;
		default:
			return false;
		}
	}

	inline bool IsFuel() const
	{
		switch(m_id)
		{
		case CID_Electricity:	// Sort of like fuel. Works in this category for all the purposes IsFuel is called so far.
		case CID_Coal:			// Fuel for coal power plant.
		case CID_Gasoline:		// Fuel for vehicles and gasoline power plant.
		case CID_Hydrogen:		// Fuel for spacecraft and hydrogen power plant.
		case CID_NaturalGas:	// Fuel for natural gas power plant.
		case CID_NuclearFuelRod:	// Fuel for new style nuclear power plant.
		case CID_Radioactives:	// Fuel for old nuclear power plant that burned radioactives like coal.
			return true;
		default:
			break;
		}
		return false;
	}

	static inline bool IsHarvesterModule(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_AtmosphereCondenserBayModule:
		case CID_ParticleCollectorBayModule:
		case CID_ProspectorBayModule:
		case CID_SiphonBayModule:
		case CID_WildcatterBayModule:
			return true;
		default:
			break;
		}
		return false;
	}

	static inline bool IsHerbivoreFood(COMMODITYID cid)
	{
		switch(cid)
		{
		default:
			return false;
		case CID_Beans:
		case CID_Bread:
		case CID_Candy:
		case CID_Flour:
		case CID_Fruit:
		case CID_Grain:
		case CID_Grapes:
		case CID_Hay:
		case CID_Herbs:
		case CID_Hops:
		case CID_IceCream:
		case CID_Milk:
		case CID_Nuts:
		case CID_Pastry:
		case CID_Silage:
		case CID_Spices:
		case CID_Sugar:
		case CID_Vegetable:
		case CID_Water:			// All creatures can eat water.
			return true;
		}
	}

	inline bool IsIntoxicant() const
	{
		return IsIntoxicant(m_combits);
	}

	static inline bool IsIntoxicant(quint32 combits)
	{
		return (combits & CB_Liquor) ? true : false;
	}

	// Returns true for cids that are in the range CID_Invalid to CID_Count but that are not really valid.
	static bool IsInvalid(COMMODITYID cid);

	// Returns true for items whose intended purpose is to be a light source. This is different from items that can emit light as
	// a feature of their use, such as an environment suit. If a weapon is made that emits light, it should return false here.
	// Lamps are sometimes also weapons, like candles and torches, simply because they can burn. This function is used by the
	// IsWeaponByDesign function to distinguish torches etc. from items intended to be used as weapons.
	inline bool IsLamp() const
	{
		return IsLamp(m_id);
	}
	static inline bool IsLamp(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_Candle:
		case CID_Torch:
		case CID_Lantern:
			return true;
		default:
			break;
		}
		return false;
	}

	inline bool IsMedical() const
	{
		return IsMedical(m_id);
	}

	static inline bool IsMedical(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_Antidote:			// Cures poison.
		case CID_Antitoxin:			// Cures disease.
		//case CID_Defibrilator:
		case CID_MedicalKit:		// Heals hit points.
		case CID_Water:				// Decontaminates acid.
		case CID_DNASampleKit:		// Captures DNA samples
			return true;
		default:
			break;
		}
		return false;
	}

	static inline bool IsMissile(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_LargeMissileBl:
		case CID_LargeMissileBlBu:
		case CID_LargeMissileSlPo:
		case CID_SmallMissileBl:
		case CID_SmallMissileBlBu:
		case CID_SmallMissileCrFr:
			return true;
		default:
			break;
		}
		return false;
	}

	inline bool IsModule() const
	{
		return IsModule(m_id);
	}

	static bool IsModule(COMMODITYID cid);

	inline bool IsNaturalResource() const
	{
		return IsNaturalResource(m_id);
	}

	static inline bool IsNaturalResource(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_Adamantite:
		case CID_Air:
		case CID_Animal:
		case CID_AntifluxParticles:
		case CID_Atmosphere:
		case CID_Beans:
		case CID_Bolite:
		case CID_BorexinoPrecipitate:
		case CID_Coal:
		case CID_CrudeOil:
		case CID_Cryozine:
		case CID_Crystals:
		case CID_Eludium:
		case CID_Fiber:
		case CID_Fish:
		case CID_Flomentum:
		case CID_Fruit:
		case CID_Gems:
		case CID_GoldOre:
		case CID_Grain:
		case CID_Grapes:
		case CID_Hay:
		//case CID_Head:
		case CID_Herbs:
		case CID_Hops:
		case CID_Hydrogen:
		case CID_Ice:
		case CID_Ioplasma:
		case CID_Lumenite:
		case CID_Magmex:
		case CID_Minerals:
		case CID_Myrathane:
		case CID_NaturalGas:
		case CID_Nuts:
		case CID_Ore:
		case CID_Phlogiston:
		case CID_Polytaride:
		case CID_Radioactives:
		//case CID_Seed:
		case CID_Spices:
		case CID_Stone:
		case CID_TypeAPreons:
		case CID_TypeBPreons:
		case CID_TypeFPreons:
		case CID_TypeGPreons:
		case CID_TypeKPreons:
		case CID_TypeMPreons:
		case CID_TypeOPreons:
		case CID_Vegetable:
		case CID_Vegetation:
		case CID_Viathol:
		case CID_Vulcanite:
		case CID_Water:
		case CID_Wood:
			return true;
		default:
			break;
		}
		return false;
	}

	// Returns true for obsolete items, i.e. tech items.
	inline bool IsObsolete() const
	{
		return DbCommodity::IsObsolete((COMMODITYID)m_id);
	}

	static bool IsObsolete(COMMODITYID cid);

	static inline bool IsOmnivoreFood(COMMODITYID cid)
	{
		switch(cid)
		{
		default:
			return false;
		case CID_Animal:
		case CID_AnimalMeat:
		case CID_Beans:
		case CID_Brain:
		case CID_Bread:
		case CID_Candy:
		case CID_Cheese:
		case CID_Eggs:
		case CID_Fish:
		case CID_FishMeat:
		case CID_Flour:
		case CID_Fruit:
		case CID_Gills:
		case CID_Grain:
		case CID_Grapes:
		case CID_Head:
		case CID_Heart:
		case CID_Herbs:
		case CID_IceCream:
		case CID_Lungs:
		case CID_Milk:
		case CID_Nuts:
		case CID_Pastry:
		case CID_Spices:
		case CID_Stomach:
		case CID_Sugar:
		case CID_Vegetable:
		case CID_Water:			// All creatures can eat water.
			return true;
		}
	}

	// Returns true if this item is ordnance for vehicles, spacecraft, or cities.
	static inline bool IsOrdnance(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_BombBl:
		case CID_BombBlRa:
		case CID_BombBlPi:
		case CID_BombElSt:
		case CID_BombSuPo:
		case CID_BombSlCr:
		case CID_HeavyArmsAmmoPi:
		case CID_HeavyArmsAmmoPiAc:
		case CID_HeavyArmsAmmoPiBl:
		case CID_HeavyArmsAmmoPiRa:
		case CID_LargeMissileBl:
		case CID_LargeMissileBlBu:
		case CID_LargeMissileSlPo:
		case CID_LargeRocketBl:
		case CID_LargeRocketBlBu:
		case CID_LargeRocketBuDi:
		case CID_SmallMissileBl:
		case CID_SmallMissileBlBu:
		case CID_SmallMissileCrFr:
		case CID_SmallRocketBl:
		case CID_SmallRocketBlBu:
		case CID_SmallRocketSlPi:
		case CID_StasisBeam:
		case CID_NeutronBeam:
		case CID_ElectronBeam:
		case CID_ErgRay:
		case CID_FreezeRay:
		case CID_CompressorBeam:
		case CID_DisruptorBeam:
		case CID_HeatRay:
		case CID_ProtonBeam:
		case CID_DisintegratorRay:
			return true;
		default:
			break;
		}
		return false;
	}

	inline bool IsPatent() const
	{
		return DbCommodity::IsPatent((COMMODITYID)m_id);
	}

	static bool IsPatent(COMMODITYID cid);

	inline bool IsPicture()
	{
		return IsPicture(m_id);
	}

	static inline bool IsPicture(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_PictureFrame120x68:
		case CID_PictureFrame22x28:
		case CID_PictureFrame60x34:
			return true;
		default:
			break;
		}
		return false;
	}

	static inline bool IsRocket(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_LargeRocketBl:
		case CID_LargeRocketBlBu:
		case CID_LargeRocketBuDi:
		case CID_SmallRocketBl:
		case CID_SmallRocketBlBu:
		case CID_SmallRocketSlPi:
			return true;
		default:
			break;
		}
		return false;
	}

	static inline bool IsScavengerFood(COMMODITYID cid)
	{
		switch(cid)
		{
		default:
			return false;
		case CID_Animal:
		case CID_AnimalMeat:
		case CID_Bone:
		case CID_Brain:
		case CID_Cheese:
		case CID_Eggs:
		case CID_Fish:
		case CID_FishMeat:
		case CID_Gills:
		case CID_Head:
		case CID_Heart:
		case CID_IceCream:
		case CID_Leather:
		case CID_Lungs:
		case CID_Milk:
		case CID_Stomach:
		case CID_Water:			// All creatures can eat water.
			return true;
		}
	}

	inline bool IsScDesign() const
	{
		return (m_id == CID_ScDesignDisk || m_id == CID_ScDesignDrawing) ? true : false;
	}

	inline bool IsScPatent() const
	{
		return DbCommodity::IsScPatent((COMMODITYID)m_id);
	}

	static bool IsScPatent(COMMODITYID cid);

	static inline bool IsShrubResource(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_Beans:
		case CID_Fiber:
		case CID_Fruit:
		case CID_Grain:
		case CID_Grapes:
		case CID_Hay:
		case CID_Herbs:
		case CID_Hops:
		case CID_Nuts:
		case CID_Spices:
		case CID_Vegetable:
			return true;
		default:
			break;
		}
		return false;
	}

	// Returns true if this item is ordnance for the bay weapon of a station.
	// Space ports don't load bombs.
	static inline bool IsStationOrdnance(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_HeavyArmsAmmoPi:
		case CID_HeavyArmsAmmoPiAc:
		case CID_HeavyArmsAmmoPiBl:
		case CID_HeavyArmsAmmoPiRa:
		case CID_LargeMissileBl:
		case CID_LargeMissileBlBu:
		case CID_LargeMissileSlPo:
		case CID_LargeRocketBl:
		case CID_LargeRocketBlBu:
		case CID_LargeRocketBuDi:
		case CID_StasisBeam:
		case CID_NeutronBeam:
		case CID_ElectronBeam:
		case CID_ErgRay:
		case CID_FreezeRay:
		case CID_CompressorBeam:
		case CID_DisruptorBeam:
		case CID_HeatRay:
		case CID_ProtonBeam:
		case CID_DisintegratorRay:
			return true;
		default:
			break;
		}
		return false;
	}

	// Spare parts are those commodities that are replaced on a spacecraft
	// using a tool to repair damage.
	inline bool IsSparePart() const
	{
		return IsSparePart(m_id);
	}

	static bool IsSparePart(COMMODITYID cid);

	static inline bool IsBdModule(SYSTEM system, COMMODITYID module)
	{
		return system == BdModuleSystem(module);
	}

	static inline bool IsScModule(SYSTEM system, COMMODITYID module)
	{
		return system == ScModuleSystem(module);
	}

	inline bool IsStorageLiquid() const
	{
		return IsStorageLiquid((COMMODITYID)m_id);
	}

	static bool IsStorageLiquid(COMMODITYID cid);

	inline bool IsTool() const
	{
		return IsTool(m_id);
	}

	static inline bool IsTool(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_Clamp:
		case CID_FittingWrench:
		case CID_GlueGun:
		case CID_Hammer:
		case CID_Knife:
		case CID_MagmiumKnife:
		case CID_Microscope:
		case CID_NanopathCalibrator:
		case CID_OScope:
		case CID_Pliers:
		case CID_ScrewDriver:
		case CID_SewingNeedle:
		case CID_Shovel:
		case CID_SolderIron:
		case CID_Tongs:
		case CID_TorqueWrench:
		case CID_VulciumKnife:
		case CID_Wrench:
			return true;
		default:
			break;
		}
		return false;
	}

	static inline bool IsTreeResource(COMMODITYID cid)
	{
		switch(cid)
		{
		default:
			return false;
		case CID_Fiber:
		case CID_Fruit:
		case CID_Herbs:
		case CID_Nuts:
		case CID_Spices:
		case CID_Wood:
			return true;
		}
	}

	static inline bool IsTurretModule(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_BallisticWeaponTurretModule:
		case CID_EnergyWeaponTurretModule:
		case CID_WeaponLauncherTurretModule:
			return true;
		default:
			break;
		}
		return false;
	}

	// Returns true if this item is ordnance for turrets on spacecraft or buildings.
	static inline bool IsTurretOrdnance(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_HeavyArmsAmmoPi:
		case CID_HeavyArmsAmmoPiAc:
		case CID_HeavyArmsAmmoPiBl:
		case CID_HeavyArmsAmmoPiRa:
		case CID_LargeMissileBl:
		case CID_LargeMissileBlBu:
		case CID_LargeMissileSlPo:
		case CID_LargeRocketBl:
		case CID_LargeRocketBlBu:
		case CID_LargeRocketBuDi:
		case CID_SmallMissileBl:
		case CID_SmallMissileBlBu:
		case CID_SmallMissileCrFr:
		case CID_SmallRocketBl:
		case CID_SmallRocketBlBu:
		case CID_SmallRocketSlPi:
		case CID_StasisBeam:
		case CID_NeutronBeam:
		case CID_ElectronBeam:
		case CID_ErgRay:
		case CID_FreezeRay:
		case CID_CompressorBeam:
		case CID_DisruptorBeam:
		case CID_HeatRay:
		case CID_ProtonBeam:
		case CID_DisintegratorRay:
			return true;
		default:
			break;
		}
		return false;
	}

	inline bool IsVehicle() const
	{
		return DbCommodity::IsVehicle((COMMODITYID)m_id);
	}

	static inline bool IsVehicle(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_AdvSpaceFighter:
		case CID_APC:
		case CID_CommercialShip:
		case CID_DirtBike:
		case CID_FarmMachinery:
		case CID_Helicopter:
		case CID_MineMachinery:
		case CID_NavalShip:
		case CID_SailBoat:
		case CID_SpaceFighter:
		case CID_SpaceRocket:
		case CID_SpaceTransport:
		case CID_Submarine:
		case CID_SUV:
		case CID_Tank:
			return true;
		default:
			break;
		}
		return false;
	}

	// Returns true if this item is ordnance for a vehicle, e.g. rockets for space fighter.
	static inline bool IsVehicleAmmunition(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_BombBl:
		case CID_BombBlRa:
		case CID_BombBlPi:
		case CID_BombElSt:
		case CID_BombSuPo:
		case CID_BombSlCr:
		case CID_HeavyArmsAmmoPi:
		case CID_HeavyArmsAmmoPiAc:
		case CID_HeavyArmsAmmoPiBl:
		case CID_HeavyArmsAmmoPiRa:
		case CID_LargeMissileBl:
		case CID_LargeMissileBlBu:
		case CID_LargeMissileSlPo:
		case CID_LargeRocketBl:
		case CID_LargeRocketBlBu:
		case CID_LargeRocketBuDi:
		case CID_SmallMissileBl:
		case CID_SmallMissileBlBu:
		case CID_SmallMissileCrFr:
		case CID_SmallRocketBl:
		case CID_SmallRocketBlBu:
		case CID_SmallRocketSlPi:
		case CID_StasisBeam:
		case CID_NeutronBeam:
		case CID_ElectronBeam:
		case CID_ErgRay:
		case CID_FreezeRay:
		case CID_CompressorBeam:
		case CID_DisruptorBeam:
		case CID_HeatRay:
		case CID_ProtonBeam:
		case CID_DisintegratorRay:
			return true;
		default:
			break;
		}
		return false;
	}

	inline bool IsWardrobeItem() const
	{
		return IsWardrobeItem(m_id);
	}

	inline static bool IsWardrobeItem(COMMODITYID cid)
	{
		switch(cid)
		{
		case CID_Backpack:
		case CID_BlackUniform:
		case CID_BlueUniform:
		case CID_BrownUniform:
		case CID_Candle:
		case CID_CapriHalterOutfit:
		case CID_CrystalJewelry:
		case CID_GemRing:
		case CID_GoldRing:
		case CID_GoldUniform:
		case CID_GravPack:
		case CID_GreenUniform:
		case CID_GreyBDU:
		case CID_GreyUniform:
		case CID_KhakiBDU:
		case CID_Knapsack:
		case CID_Lantern:
		case CID_LeatherClothes:
		case CID_JeansAndShirt:
		case CID_GoldBracelet:
		case CID_OliveDrabBDU:
		case CID_OrangeUniform:
		case CID_ParcXUniform:
		case CID_PinkUniform:
		case CID_PreonRingA:
		case CID_PreonRingB:
		case CID_PreonRingF:
		case CID_PreonRingG:
		case CID_PreonRingK:
		case CID_PreonRingM:
		case CID_PreonRingO:
		case CID_RedUniform:
		case CID_TealUniform:
		case CID_Torch:
		case CID_UtilityPouch:
		case CID_VacSuit:
		case CID_VacSuitHelmet:
		case CID_WhiteUniform:
			return true;
		default:
			// Most commodities are not found in the wardrobe of a ship's cabin.
			break;
		}
		return false;
	}

	// Returns true if this item is inherently a weapon. Returns false for
	// other items, including those that could be used as a weapon.
	inline bool IsWeapon() const
	{
		return (IsWeapon(IPS_Weapon1) || IsWeapon(IPS_Weapon2)) ? true : false;
	}

	// Returns true for items intended to be used as weapons. This is distinct from items that can be used as weapons but aren't
	// specifically weapons, like torches or rocks.
	inline bool IsWeaponByDesign() const
	{
		if(!IsWeapon())
			return false;
		if(IsTool())
			return false;
		if(IsLamp())
			return false;
		return true;
	}

	// Returns true if this item is inherently a weapon. Returns false for
	// other items, including those that could be used as a weapon.
	static inline bool IsWeapon(WEAPONMODE wpn1Mode, WEAPONMODE wpn2Mode)
	{
		return (wpn1Mode != WM_Not || wpn2Mode != WM_Not) ? true : false;
	}

	inline bool IsWeapon(INVENTORYSUB sub) const
	{
		return IsWeapon(sub, m_combits, m_wpn1Mode, m_wpn2Mode);
	}

	static inline bool IsWeapon(INVENTORYSUB sub, quint32 combits, WEAPONMODE wpn1Mode, WEAPONMODE wpn2Mode)
	{
		switch(sub)
		{
		case IPS_Item:
			// Item may be usable as a physical weapon.
			// This bit test actually checks for slash, bludgeon, and pierce.
			if(combits & CB_Slash)
				return true;
			break;
		case IPS_Weapon1:
			if(wpn1Mode != WM_Not)
				return true;
			break;
		case IPS_Weapon2:
			if(wpn2Mode != WM_Not)
				return true;
			break;
		}
		return false;
	}

	inline bool IsWeaponThatUsesAmmo(INVENTORYSUB sub) const
	{
		return IsWeaponThatUsesAmmo(sub, m_combits,
				m_wpn1Mode, m_ammo1Type, m_ammo1Qty,
				m_wpn2Mode, m_ammo2Type, m_ammo2Qty);
	}

	static inline bool IsWeaponThatUsesAmmo(INVENTORYSUB sub, quint32 combits,
			WEAPONMODE wpn1Mode, AMMOTYPE /*ammo1Type*/, quint8 ammo1Qty,
			WEAPONMODE wpn2Mode, AMMOTYPE ammo2Type, quint8 ammo2Qty)
	{
		switch(sub)
		{
		case IPS_Item:
			// Item may be usable as a physical weapon but it never needs ammo.
			return false;

		case IPS_Weapon1:
			return (ammo1Qty && IsWeapon(sub, combits, wpn1Mode, wpn2Mode));

		case IPS_Weapon2:
			// Energy1 ammo is treated as a firing mode of weapon 1 so it uses the ammo of weapon 1.
			if(ammo2Type == AMMO_Energy1)
				return (ammo1Qty && IsWeapon(sub, combits, wpn1Mode, wpn2Mode));
			else
				return (ammo2Qty && IsWeapon(sub, combits, wpn1Mode, wpn2Mode));
		}
		return false;
	}

	inline bool IsWorn(INVENTORYPOS wearPos) const
	{
		if(wearPos < IP_ArmorArms || wearPos > IP_WristRight)
			return false;
		return m_bitsWear & IPOSBIT(wearPos);
	}

	inline bool IsWornOnBody() const
	{
		return m_bitsWear &
			(IPOSBIT(IP_ArmorArms) |
			 IPOSBIT(IP_ArmorFeet) |
			 IPOSBIT(IP_ArmorHands) |
			 IPOSBIT(IP_ArmorLegs) |
			 IPOSBIT(IP_ArmorLowerTorso) |
			 IPOSBIT(IP_ArmorUpperTorso) |
			 IPOSBIT(IP_ClothingArms) |
			 IPOSBIT(IP_ClothingLegs) |
			 IPOSBIT(IP_ClothingLowerTorso) |
			 IPOSBIT(IP_ClothingUpperTorso));
	}

	inline bool IsWornOnDigit() const
	{
		return m_bitsWear &
			(IPOSBIT(IP_Ring1Left) |
			 IPOSBIT(IP_Ring1Right) |
			 IPOSBIT(IP_Ring2Left) |
			 IPOSBIT(IP_Ring2Right));
	}

	inline bool IsWornOnFace() const
	{
		return m_bitsWear &
			(IPOSBIT(IP_ClothingFace) |
			 IPOSBIT(IP_ArmorFace));
	}

	inline bool IsWornOnHead() const
	{
		return m_bitsWear &
			(IPOSBIT(IP_ClothingHead) |
			 IPOSBIT(IP_ArmorHead));
	}

	inline bool IsWornOnLimb() const
	{
		return m_bitsWear &
			(IPOSBIT(IP_AnkleLeft) |
			 IPOSBIT(IP_AnkleRight) |
			 IPOSBIT(IP_WristLeft) |
			 IPOSBIT(IP_WristRight));
	}

	inline bool IsWornOnNeck() const
	{
		return m_bitsWear & IPOSBIT(IP_Neck);
	}

	static inline int MaxTurretRounds(COMMODITYID ordCID)
	{
		switch(ordCID)
		{
		default:
		case CID_BombBl:
		case CID_BombBlRa:
		case CID_BombBlPi:
		case CID_BombElSt:
		case CID_BombSuPo:
		case CID_BombSlCr:
			break;
		case CID_StasisBeam:
		case CID_NeutronBeam:
		case CID_ElectronBeam:
		case CID_ErgRay:
		case CID_FreezeRay:
		case CID_CompressorBeam:
		case CID_DisruptorBeam:
		case CID_HeatRay:
		case CID_ProtonBeam:
		case CID_DisintegratorRay:
			// It costs one energy unit to reload an energy weapon turret.
			return 100;
		case CID_HeavyArmsAmmoPi:
		case CID_HeavyArmsAmmoPiRa:
		case CID_HeavyArmsAmmoPiBl:
		case CID_HeavyArmsAmmoPiAc:
			return 300;
		case CID_LargeMissileBl:
		case CID_LargeMissileBlBu:
		case CID_LargeMissileSlPo:
			return 15;
		case CID_LargeRocketBl:
		case CID_LargeRocketBlBu:
		case CID_LargeRocketBuDi:
			return 25;
		case CID_SmallMissileBl:
		case CID_SmallMissileBlBu:
		case CID_SmallMissileCrFr:
			return 30;
		case CID_SmallRocketBl:
		case CID_SmallRocketBlBu:
		case CID_SmallRocketSlPi:
			return 50;
		}
		return 0;
	}

	// Returns the name of the .3dsN file that represents this commodity when it appears individually in the environment.
	inline const QString ModelName() const
	{
		return m_modelPath;
	}

	inline bool operator==(const DbCommodity& c) const
	{
		return m_id == c.m_id;
	}

	// NOTE: Streaming operators currently only known to be used to store&load auscript and austoryeditor data file.
	void operator<<(QDataStream& in);
	void operator>>(QDataStream& out) const;

	// Returns a word to describe quality.
	static inline QString QualityDesc(unsigned char byQuality)
	{
		struct level
		{
			const char* name;
			int minqual;
		}
		levels[] =
		{
			// This list was generated by subtracting a growing quantity (8 or 9) at each step down from 255.
			{"Perfect", 255},
			{"Excellent", 247},
			{"Superb", 231},
			{"Fine", 207},
			{"Good", 175},
			{"Fair", 135},
			{"Poor", 87},				// This is the QualityBaseline, where the QualityBoostF function returns 1x.
			{"Bad", 31},
			{"Flawed", 1}
			/* This is the old list from the beginning.
			{"Perfect", 255},
			{"Excellent", 245},
			{"Superb", 225},
			{"Fine", 195},
			{"Good", 155},
			{"Fair", 115},
			{"Poor", 75},
			{"Bad", 35},
			{"Flawed", 1}
			*/
		};
		int levelcount = sizeof(levels)/sizeof(level);

		for(int lv=0; lv<levelcount; lv++)
		{
			if(byQuality >= levels[lv].minqual)
				return QString(levels[lv].name);
		}
		return QString("?Qual");
	}

	static COMMODITYID RandomFood(ECOROLE ecoRole);

	void Rx(QDataStream& in);

	inline void SetBaseCost(float baseCost)
	{
		m_baseCost = baseCost;
	}

	// Performs a random test to determine failure of a single item due to 'quality'.
	// Returns true if the commodity did not fail.
	// Returns false if the commodity failed.
	static bool TestQuality(quint8 quality);

	// Calculates the failure rate of a certain quantity of a commodity, based on 'quality'.
	// Returns the number of units of 'count' that do NOT fail due to quality failure rate.
	// The number returned will never exceed 'count'.
	static quint32 TestFailure(quint8 quality, quint32 count);

	void Tx(QDataStream& out) const;

	inline AuPoint WeaponMuzzleLoc() const
	{
		return WeaponMuzzleLoc(m_id);
	}

	// Returns muzzle location relative to the origin, as if aiming in the +X direction.
	static AuPoint WeaponMuzzleLoc(COMMODITYID cid);
};

// I'm not sure why this is needed. The build started failing when I added AUGLOBAL_DLL to export DbCommodity. The error
// indicated that it couldn't find a qHash function for a const DbCommodity, hmm. - D.H.H Wed Dec 5, 2012
inline unsigned int qHash(const DbCommodity& key)
{
	return key.m_id;
}

class AUGLOBAL_DLL DbCommodityArray : public QList<DbCommodity>
{
public:
	// Add record using its id as the index into this array.
	void Add(const DbCommodity& rec);

	// May return -1 if not calculated yet.
	inline float BaseCost(COMMODITYID idCommodity) const
	{
		Q_ASSERT(idCommodity < count());
		const_iterator it = begin() + idCommodity;
		return((*it).BaseCost());
	}

	inline const DbCommodity& Commodity(COMMODITYID idCommodity) const
	{
		Q_ASSERT(idCommodity < count());
		const_iterator it = constBegin() + idCommodity;
		return *it;
	}

	// Returns the base cost of a commodity modified by quality.
	float CommodityQualityCost(COMMODITYID idCommodity, unsigned int quality) const;

	// Lists all commodity ids that fit into a specified filter category.
	void ListFilterCommodities(QList<COMMODITYID>& commodities, COMMODITYFILTER filter);

	void Load();

	// NOTE: Streaming operators currently only known to be used to store&load auscript and austoryeditor data file.
	void operator<<(QDataStream& in);
	void operator>>(QDataStream& out) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUGLOBAL_DLL CommoditySubKey
{
public:
	quint8		m_Quality;
	quint32		m_EmpireOfOrigin;

	CommoditySubKey(quint8 quality, quint32 empireOfOrigin)
	{
		Q_ASSERT(quality);
		m_Quality = quality;
		m_EmpireOfOrigin = empireOfOrigin;
	}

	// This operator< actually does a > comparison so the map will be sorted descending according to quality and empire of origin.
	bool operator<(const CommoditySubKey& c) const
	{
		if(m_Quality > c.m_Quality)
			return true;
		if(m_Quality == c.m_Quality && m_EmpireOfOrigin > c.m_EmpireOfOrigin)
			return true;
		return false;
	}
};

#endif // DbCommodity_H
