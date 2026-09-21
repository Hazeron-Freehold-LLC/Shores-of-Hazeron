//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auplayerstats.h,v 1.85 2022/02/19 17:32:19 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuPlayerStats_H
#define AuPlayerStats_H

#include "audnabase.h"
#include "auhash.h"
#include "auspecies.h"
#include "austation.h"
#include "dbindustry.h"
#include <QDataStream>

class AuItem;
class DaAtom;
class DaCreature;
class DaStar;
class DaUnit;
class DaVehicle;
class DaVessel;
class DaWorld;
class DaWorldGasGiant;
class DaWorldGlobe;
class DaWorldRingSection;

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Stats kept for each COMMODITYID type.

class AuPlayerStatsItem
{
public:
	quint32			m_BoughtCargo;			// count of items bought as cargo.
	quint32			m_BoughtInventory;		// count of items bought as gear items
	quint16			m_CraftedAtIndustry;	// Count handcrafted at an industry.
	quint16			m_CraftedByHand;		// Count handcrafted not at an industry.
	quint16			m_Foraged;				// count of times the item has been foraged from the environment
	bool			m_HadInventory;			// true if the item has ever been in gear item.
	quint8			m_HighestQ;				// Highest quality that resulted in a stat here.
	quint16			m_KilledBy;				// Count of times killed by item.
	quint16			m_Kills;				// Count of kills using item.
	quint32			m_SoldCargo;			// count of items sold as cargo.
	quint32			m_SoldInventory;		// count of items sold as gear items
	quint16			m_UsesAsWeapon;			// Count of uses as a weapon.
	quint16			m_UsesOnOther;			// Count of uses on others.
	quint16			m_UsesOnSelf;			// Count of uses on self. e.g. eating food, healing with med kit, light on/off, etc.

public:
	AuPlayerStatsItem();

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Items are indexed by COMMODITYID.

class AuPlayerStatsItems : public unordered_map<COMMODITYID, AuPlayerStatsItem, COMMODITYID_Hash>
{
public:
	// Returns true if the item was ever in inventory.
	bool Had(COMMODITYID cid) const;

	void operator<<(QDataStream& in)
	{
		quint16 qty;
		in >> qty;

		qint16 key;
		AuPlayerStatsItem val;
		for(; qty; qty--)
		{
			in >> key;
			val << in;
			if(key != CID_Invalid && key > CID_FIRST && key < CID_Count)
				insert(pair<COMMODITYID, AuPlayerStatsItem>((COMMODITYID)key, val));
			//else
				//qWarning() << "AuPlayerStatsItems read invalid COMMODITYID" << key;
		}
	}

	void operator>>(QDataStream& out) const
	{
		out << (quint16)size();
		for(const_iterator it = begin(); it != end(); it++)
		{
			out << (qint16)(*it).first;
			(*it).second >> out;
		}
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Stats kept for each planet type.

class AuPlayerStatsPlanet
{
public:
	quint16					m_BeamDownLand;				// count of times beamed down on land
	quint16					m_BeamDownSea;				// count of times beamed down into sea/magma/supercooled liquid.
	quint16					m_BeamUpLand;				// count of times beamed up from land
	quint16					m_BeamUpSea;				// count of times beamed up out of sea/magma/supercooled liquid.
	float					m_DeepestSwim;				// deepest depth ever swam in the sea eva on this planet type, in feet
	quint16					m_EntryEVA;					// Count of atmosphere entries.
	quint16					m_EntryMotorBike;
	quint16					m_EntryRocket;
	quint16					m_EntrySailBoat;
	quint16					m_EntrySpaceFighterF1;
	quint16					m_EntrySpaceFighterF2;
	quint16					m_EntryStation;
	quint16					m_EntrySpaceship;
	quint16					m_EntryStarship;
	quint16					m_EntrySubmarine;
	quint16					m_EntrySUV;
	quint16					m_EntryAPC;
	quint16					m_EntryFarmMachinery;
	quint16					m_EntryHelicopter;
	quint16					m_EntrySpaceTransport;
	quint16					m_ExitEVA;
	quint16					m_ExitMotorBike;
	quint16					m_ExitRocket;
	quint16					m_ExitSailBoat;
	quint16					m_ExitSpaceFighterF1;
	quint16					m_ExitSpaceFighterF2;
	quint16					m_ExitStation;
	quint16					m_ExitSpaceship;
	quint16					m_ExitStarship;
	quint16					m_ExitSubmarine;
	quint16					m_ExitSUV;
	quint16					m_ExitAPC;
	quint16					m_ExitFarmMachinery;
	quint16					m_ExitHelicopter;
	quint16					m_ExitSpaceTransport;
	quint16					m_FirstFlags;				// Count of first-flags planted on this planet type.
	float					m_HighestFall;				// highest altitude ever off the ground eva on this planet type
	bool					m_Treaded;					// true if ever walked on this planet type.

public:
	AuPlayerStatsPlanet();

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Planets are indexed by PLANET_STAT_TYPE.

// Planet types are the combinations of planet type, orbit zone and atmosphere type.
enum PLANET_STAT_TYPE
{
	PST_InfernoStandardGasGiant,
	PST_InfernoStandardTitan,
	PST_InfernoStandardMoon,
	PST_InfernoStandardPlanet,
	PST_InfernoStandardRingworld,
	PST_InfernoTaintedGasGiant,
	PST_InfernoTaintedTitan,
	PST_InfernoTaintedMoon,
	PST_InfernoTaintedPlanet,
	PST_InfernoTaintedRingworld,
	PST_InfernoExoticGasGiant,
	PST_InfernoExoticTitan,
	PST_InfernoExoticMoon,
	PST_InfernoExoticPlanet,
	PST_InfernoExoticRingworld,
	PST_InfernoCorrosiveGasGiant,
	PST_InfernoCorrosiveTitan,
	PST_InfernoCorrosiveMoon,
	PST_InfernoCorrosivePlanet,
	PST_InfernoCorrosiveRingworld,
	PST_InfernoInsidiousGasGiant,
	PST_InfernoInsidiousTitan,
	PST_InfernoInsidiousMoon,
	PST_InfernoInsidiousPlanet,
	PST_InfernoInsidiousRingworld,
	PST_InnerStandardGasGiant,
	PST_InnerStandardTitan,
	PST_InnerStandardMoon,
	PST_InnerStandardPlanet,
	PST_InnerStandardRingworld,
	PST_InnerTaintedGasGiant,
	PST_InnerTaintedTitan,
	PST_InnerTaintedMoon,
	PST_InnerTaintedPlanet,
	PST_InnerTaintedRingworld,
	PST_InnerExoticGasGiant,
	PST_InnerExoticTitan,
	PST_InnerExoticMoon,
	PST_InnerExoticPlanet,
	PST_InnerExoticRingworld,
	PST_InnerCorrosiveGasGiant,
	PST_InnerCorrosiveTitan,
	PST_InnerCorrosiveMoon,
	PST_InnerCorrosivePlanet,
	PST_InnerCorrosiveRingworld,
	PST_InnerInsidiousGasGiant,
	PST_InnerInsidiousTitan,
	PST_InnerInsidiousMoon,
	PST_InnerInsidiousPlanet,
	PST_InnerInsidiousRingworld,
	PST_HabitableStandardGasGiant,
	PST_HabitableStandardTitan,
	PST_HabitableStandardMoon,
	PST_HabitableStandardPlanet,
	PST_HabitableStandardRingworld,
	PST_HabitableTaintedGasGiant,
	PST_HabitableTaintedTitan,
	PST_HabitableTaintedMoon,
	PST_HabitableTaintedPlanet,
	PST_HabitableTaintedRingworld,
	PST_HabitableExoticGasGiant,
	PST_HabitableExoticTitan,
	PST_HabitableExoticMoon,
	PST_HabitableExoticPlanet,
	PST_HabitableExoticRingworld,
	PST_HabitableCorrosiveGasGiant,
	PST_HabitableCorrosiveTitan,
	PST_HabitableCorrosiveMoon,
	PST_HabitableCorrosivePlanet,
	PST_HabitableCorrosiveRingworld,
	PST_HabitableInsidiousGasGiant,
	PST_HabitableInsidiousTitan,
	PST_HabitableInsidiousMoon,
	PST_HabitableInsidiousPlanet,
	PST_HabitableInsidiousRingworld,
	PST_OuterStandardGasGiant,
	PST_OuterStandardTitan,
	PST_OuterStandardMoon,
	PST_OuterStandardPlanet,
	PST_OuterStandardRingworld,
	PST_OuterTaintedGasGiant,
	PST_OuterTaintedTitan,
	PST_OuterTaintedMoon,
	PST_OuterTaintedPlanet,
	PST_OuterTaintedRingworld,
	PST_OuterExoticGasGiant,
	PST_OuterExoticTitan,
	PST_OuterExoticMoon,
	PST_OuterExoticPlanet,
	PST_OuterExoticRingworld,
	PST_OuterCorrosiveGasGiant,
	PST_OuterCorrosiveTitan,
	PST_OuterCorrosiveMoon,
	PST_OuterCorrosivePlanet,
	PST_OuterCorrosiveRingworld,
	PST_OuterInsidiousGasGiant,
	PST_OuterInsidiousTitan,
	PST_OuterInsidiousMoon,
	PST_OuterInsidiousPlanet,
	PST_OuterInsidiousRingworld,
	PST_FrigidStandardGasGiant,
	PST_FrigidStandardTitan,
	PST_FrigidStandardMoon,
	PST_FrigidStandardPlanet,
	PST_FrigidStandardRingworld,
	PST_FrigidTaintedGasGiant,
	PST_FrigidTaintedTitan,
	PST_FrigidTaintedMoon,
	PST_FrigidTaintedPlanet,
	PST_FrigidTaintedRingworld,
	PST_FrigidExoticGasGiant,
	PST_FrigidExoticTitan,
	PST_FrigidExoticMoon,
	PST_FrigidExoticPlanet,
	PST_FrigidExoticRingworld,
	PST_FrigidCorrosiveGasGiant,
	PST_FrigidCorrosiveTitan,
	PST_FrigidCorrosiveMoon,
	PST_FrigidCorrosivePlanet,
	PST_FrigidCorrosiveRingworld,
	PST_FrigidInsidiousGasGiant,
	PST_FrigidInsidiousTitan,
	PST_FrigidInsidiousMoon,
	PST_FrigidInsidiousPlanet,
	PST_FrigidInsidiousRingworld,
	PST_InfernoRing,
	PST_InnerRing,
	PST_HabitableRing,
	PST_OuterRing,
	PST_FrigidRing,
	PST_InfernoPlanetoid,
	PST_InnerPlanetoid,
	PST_HabitablePlanetoid,
	PST_OuterPlanetoid,
	PST_FrigidPlanetoid,
};

const QString s_PlanetStatType[] =
{
	"Gas Giant, Standard Atm, Inferno Orbit",
	"Titan, Standard Atm, Inferno Orbit",
	"Moon, Standard Atm, Inferno Orbit",
	"Planet, Standard Atm, Inferno Orbit",
	"Ringworld, Standard Atm, Inferno Orbit",
	"Gas Giant, Tainted Atm, Inferno Orbit",
	"Titan, Tainted Atm, Inferno Orbit",
	"Moon, Tainted Atm, Inferno Orbit",
	"Planet, Tainted Atm, Inferno Orbit",
	"Ringworld, Tainted Atm, Inferno Orbit",
	"Gas Giant, Exotic Atm, Inferno Orbit",
	"Titan, Exotic Atm, Inferno Orbit",
	"Moon, Exotic Atm, Inferno Orbit",
	"Planet, Exotic Atm, Inferno Orbit",
	"Ringworld, Exotic Atm, Inferno Orbit",
	"Gas Giant, Corrosive Atm, Inferno Orbit",
	"Titan, Corrosive Atm, Inferno Orbit",
	"Moon, Corrosive Atm, Inferno Orbit",
	"Planet, Corrosive Atm, Inferno Orbit",
	"Ringworld, Corrosive Atm, Inferno Orbit",
	"Gas Giant, Insidious Atm, Inferno Orbit",
	"Titan, Insidious Atm, Inferno Orbit",
	"Moon, Insidious Atm, Inferno Orbit",
	"Planet, Insidious Atm, Inferno Orbit",
	"Ringworld, Insidious Atm, Inferno Orbit",
	"Gas Giant, Standard Atm, Inner Orbit",
	"Titan, Standard Atm, Inner Orbit",
	"Moon, Standard Atm, Inner Orbit",
	"Planet, Standard Atm, Inner Orbit",
	"Ringworld, Standard Atm, Inner Orbit",
	"Gas Giant, Tainted Atm, Inner Orbit",
	"Titan, Tainted Atm, Inner Orbit",
	"Moon, Tainted Atm, Inner Orbit",
	"Planet, Tainted Atm, Inner Orbit",
	"Ringworld, Tainted Atm, Inner Orbit",
	"Gas Giant, Exotic Atm, Inner Orbit",
	"Titan, Exotic Atm, Inner Orbit",
	"Moon, Exotic Atm, Inner Orbit",
	"Planet, Exotic Atm, Inner Orbit",
	"Ringworld, Exotic Atm, Inner Orbit",
	"Gas Giant, Corrosive Atm, Inner Orbit",
	"Titan, Corrosive Atm, Inner Orbit",
	"Moon, Corrosive Atm, Inner Orbit",
	"Planet, Corrosive Atm, Inner Orbit",
	"Ringworld, Corrosive Atm, Inner Orbit",
	"Gas Giant, Insidious Atm, Inner Orbit",
	"Titan, Insidious Atm, Inner Orbit",
	"Moon, Insidious Atm, Inner Orbit",
	"Planet, Insidious Atm, Inner Orbit",
	"Ringworld, Insidious Atm, Inner Orbit",
	"Gas Giant, Standard Atm, Habitable Orbit",
	"Titan, Standard Atm, Habitable Orbit",
	"Moon, Standard Atm, Habitable Orbit",
	"Planet, Standard Atm, Habitable Orbit",
	"Ringworld, Standard Atm, Habitable Orbit",
	"Gas Giant, Tainted Atm, Habitable Orbit",
	"Titan, Tainted Atm, Habitable Orbit",
	"Moon, Tainted Atm, Habitable Orbit",
	"Planet, Tainted Atm, Habitable Orbit",
	"Ringworld, Tainted Atm, Habitable Orbit",
	"Gas Giant, Exotic Atm, Habitable Orbit",
	"Titan, Exotic Atm, Habitable Orbit",
	"Moon, Exotic Atm, Habitable Orbit",
	"Planet, Exotic Atm, Habitable Orbit",
	"Ringworld, Exotic Atm, Habitable Orbit",
	"Gas Giant, Corrosive Atm, Habitable Orbit",
	"Titan, Corrosive Atm, Habitable Orbit",
	"Moon, Corrosive Atm, Habitable Orbit",
	"Planet, Corrosive Atm, Habitable Orbit",
	"Ringworld, Corrosive Atm, Habitable Orbit",
	"Gas Giant, Insidious Atm, Habitable Orbit",
	"Titan, Insidious Atm, Habitable Orbit",
	"Moon, Insidious Atm, Habitable Orbit",
	"Planet, Insidious Atm, Habitable Orbit",
	"Ringworld, Insidious Atm, Habitable Orbit",
	"Gas Giant, Standard Atm, Outer Orbit",
	"Titan, Standard Atm, Outer Orbit",
	"Moon, Standard Atm, Outer Orbit",
	"Planet, Standard Atm, Outer Orbit",
	"Ringworld, Standard Atm, Outer Orbit",
	"Gas Giant, Tainted Atm, Outer Orbit",
	"Titan, Tainted Atm, Outer Orbit",
	"Moon, Tainted Atm, Outer Orbit",
	"Planet, Tainted Atm, Outer Orbit",
	"Ringworld, Tainted Atm, Outer Orbit",
	"Gas Giant, Exotic Atm, Outer Orbit",
	"Titan, Exotic Atm, Outer Orbit",
	"Moon, Exotic Atm, Outer Orbit",
	"Planet, Exotic Atm, Outer Orbit",
	"Ringworld, Exotic Atm, Outer Orbit",
	"Gas Giant, Corrosive Atm, Outer Orbit",
	"Titan, Corrosive Atm, Outer Orbit",
	"Moon, Corrosive Atm, Outer Orbit",
	"Planet, Corrosive Atm, Outer Orbit",
	"Ringworld, Corrosive Atm, Outer Orbit",
	"Gas Giant, Insidious Atm, Outer Orbit",
	"Titan, Insidious Atm, Outer Orbit",
	"Moon, Insidious Atm, Outer Orbit",
	"Planet, Insidious Atm, Outer Orbit",
	"Ringworld, Insidious Atm, Outer Orbit",
	"Gas Giant, Standard Atm, Frigid Orbit",
	"Titan, Standard Atm, Frigid Orbit",
	"Moon, Standard Atm, Frigid Orbit",
	"Planet, Standard Atm, Frigid Orbit",
	"Ringworld, Standard Atm, Frigid Orbit",
	"Gas Giant, Tainted Atm, Frigid Orbit",
	"Titan, Tainted Atm, Frigid Orbit",
	"Moon, Tainted Atm, Frigid Orbit",
	"Planet, Tainted Atm, Frigid Orbit",
	"Ringworld, Tainted Atm, Frigid Orbit",
	"Gas Giant, Exotic Atm, Frigid Orbit",
	"Titan, Exotic Atm, Frigid Orbit",
	"Moon, Exotic Atm, Frigid Orbit",
	"Planet, Exotic Atm, Frigid Orbit",
	"Ringworld, Exotic Atm, Frigid Orbit",
	"Gas Giant, Corrosive Atm, Frigid Orbit",
	"Titan, Corrosive Atm, Frigid Orbit",
	"Moon, Corrosive Atm, Frigid Orbit",
	"Planet, Corrosive Atm, Frigid Orbit",
	"Ringworld, Corrosive Atm, Frigid Orbit",
	"Gas Giant, Insidious Atm, Frigid Orbit",
	"Titan, Insidious Atm, Frigid Orbit",
	"Moon, Insidious Atm, Frigid Orbit",
	"Planet, Insidious Atm, Frigid Orbit",
	"Ringworld, Insidious Atm, Frigid Orbit",
	"Planetoid, Inferno Orbit",
	"Planetoid, Inner Orbit",
	"Planetoid, Habitable Orbit",
	"Planetoid, Outer Orbit",
	"Planetoid, Frigid Orbit",
};

class PLANET_STAT_Hash
{
public:
	inline size_t operator()(const PLANET_STAT_TYPE& o) const
	{
		return hash<unsigned int>()(o);
	}
};

class AuPlayerStatsPlanets : public unordered_map<PLANET_STAT_TYPE, AuPlayerStatsPlanet, PLANET_STAT_Hash>
{
public:
	void operator<<(QDataStream& in)
	{
		quint8 qty;
		in >> qty;

		quint8 key;
		AuPlayerStatsPlanet val;
		for(; qty; qty--)
		{
			in >> key;
			val << in;
			if(/*key >= PST_InfernoStandardGasGiant && */key <= PST_FrigidInsidiousRingworld)
				insert(pair<PLANET_STAT_TYPE, AuPlayerStatsPlanet>((PLANET_STAT_TYPE)key, val));
			//else
				//qWarning() << "AuPlayerStatsPlanets read invalid PLANET_STAT_TYPE" << key;
		}
	}

	void operator>>(QDataStream& out) const
	{
		out << (quint8)size();
		for(const_iterator it = begin(); it != end(); it++)
		{
			out << (quint8)(*it).first;
			(*it).second >> out;
		}
	}

	static PLANET_STAT_TYPE StatType(const DaWorldGasGiant* planet);

	static PLANET_STAT_TYPE StatType(const DaWorldGlobe* planet);

	static PLANET_STAT_TYPE StatType(const DaWorldRingSection* planet);
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuPlayerStatsStar
{
public:
	quint16			m_SunBurnEVA;				// count of times burned up in star
	quint16			m_SunBurnMotorBike;
	quint16			m_SunBurnRocket;
	quint16			m_SunBurnSailBoat;
	quint16			m_SunBurnSpaceFighterF1;
	quint16			m_SunBurnSpaceFighterF2;
	quint16			m_SunBurnStation;
	quint16			m_SunBurnSpaceship;
	quint16			m_SunBurnStarship;
	quint16			m_SunBurnSubmarine;
	quint16			m_SunBurnSUV;
	quint16			m_SunBurnAPC;
	quint16			m_SunBurnFarmMachinery;
	quint16			m_SunBurnHelicopter;
	quint16			m_SunBurnSpaceTransport;

public:
	AuPlayerStatsStar();

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Stars are indexed by STAR_STAT_TYPE.

// Star types are the combinations of star type and size.
enum STAR_STAT_TYPE
{
	SST_O_Ia,
	SST_O_Ib,
	SST_O_II,
	SST_O_III,
	SST_O_IV,
	SST_O_V,
	SST_O_VI,
	SST_O_D,
	SST_B_Ia,
	SST_B_Ib,
	SST_B_II,
	SST_B_III,
	SST_B_IV,
	SST_B_V,
	SST_B_VI,
	SST_B_D,
	SST_A_Ia,
	SST_A_Ib,
	SST_A_II,
	SST_A_III,
	SST_A_IV,
	SST_A_V,
	SST_A_VI,
	SST_A_D,
	SST_F_Ia,
	SST_F_Ib,
	SST_F_II,
	SST_F_III,
	SST_F_IV,
	SST_F_V,
	SST_F_VI,
	SST_F_D,
	SST_G_Ia,
	SST_G_Ib,
	SST_G_II,
	SST_G_III,
	SST_G_IV,
	SST_G_V,
	SST_G_VI,
	SST_G_D,
	SST_K_Ia,
	SST_K_Ib,
	SST_K_II,
	SST_K_III,
	SST_K_IV,
	SST_K_V,
	SST_K_VI,
	SST_K_D,
	SST_M_Ia,
	SST_M_Ib,
	SST_M_II,
	SST_M_III,
	SST_M_IV,
	SST_M_V,
	SST_M_VI,
	SST_M_D,
	SST_Supermassive_Black_Hole,
	SST_Stellar_Black_Hole,
	SST_Neutron_Star,
};

QString StarStatName(STAR_STAT_TYPE st);

class STAR_STAT_Hash
{
public:
	inline size_t operator()(const STAR_STAT_TYPE& o) const
	{
		return hash<unsigned int>()(o);
	}
};

class AuPlayerStatsStars : public unordered_map<STAR_STAT_TYPE, AuPlayerStatsStar, STAR_STAT_Hash>
{
public:
	void operator<<(QDataStream& in)
	{
		quint8 qty;
		in >> qty;

		quint8 key;
		AuPlayerStatsStar val;
		for(; qty; qty--)
		{
			in >> key;
			val << in;
			if(/*key >= SST_O_Ia && */key <= SST_Stellar_Black_Hole)
				insert(pair<STAR_STAT_TYPE, AuPlayerStatsStar>((STAR_STAT_TYPE)key, val));
			//else
				//qWarning() << "AuPlayerStatsStars read invalid STAR_STAT_TYPE" << key;
		}
	}

	void operator>>(QDataStream& out) const
	{
		out << (quint8)size();
		for(const_iterator it = begin(); it != end(); it++)
		{
			out << (quint8)(*it).first;
			(*it).second >> out;
		}
	}

	static STAR_STAT_TYPE StatType(const DaStar* star);
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Stats for all kinds of craft, including vehicles, spacecraft, and stations.

class AuPlayerStatsCraft
{
public:
	quint16					m_Captures;					// Count of this craft captured by the person.
	quint16					m_DeadHeads;				// Count of dead-heads from one system to another in this craft
	quint16					m_RepairsBought;			// Count of repairs bought for spacecraft or V presses to repair vehicle.
	quint16					m_RepairsByHand;			// count of times the person repaired this craft by hand
	quint16					m_SectorsTraversed;			// count of times the person has traversed a sector boundary in this craft
	quint16					m_UpgradesBought;			// Count of upgrade modules bought for spacecraft.
	quint16					m_Wormholes;				// count of times the person wormholed in this craft

public:
	AuPlayerStatsCraft();

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Craft are indexed by CRAFT_STAT_TYPE.

// Craft types
enum CRAFT_STAT_TYPE
{
	CST_Invalid,
	CST_MotorBike,
	CST_Rocket,
	CST_SailBoat,
	CST_SpaceFighterF1,
	CST_SpaceFighterF2,
	CST_Station,
	CST_Spaceship,
	CST_Starship,
	CST_Submarine,
	CST_SUV,
	CST_APC,
	CST_FarmMachinery,
	CST_Helicopter,
	CST_SpaceTransport,
};

const QString s_CraftStatType[] =
{
	"Invalid",
	"Motor Bike",
	"Rocket",
	"Sail Boat",
	"Space Fighter F1",
	"Space Fighter F2",
	"Station",
	"Spaceship",
	"Starship",
	"Submarine",
	"SUV",
	"APC",
	"Farm Machinery",
	"Helicopter",
	"Space Transport",
};

class CRAFT_STAT_Hash
{
public:
	inline size_t operator()(const CRAFT_STAT_TYPE& o) const
	{
		return hash<unsigned int>()(o);
	}
};

class AuPlayerStatsCrafts : public unordered_map<CRAFT_STAT_TYPE, AuPlayerStatsCraft, CRAFT_STAT_Hash>
{
public:
	void operator<<(QDataStream& in)
	{
		quint8 qty;
		in >> qty;

		quint8 key;
		AuPlayerStatsCraft val;
		for(; qty; qty--)
		{
			in >> key;
			val << in;
			if(/*key >= CST_Invalid && */key <= CST_SpaceTransport)
				insert(pair<CRAFT_STAT_TYPE, AuPlayerStatsCraft>((CRAFT_STAT_TYPE)key, val));
			//else
				//qWarning() << "AuPlayerStatsCrafts read invalid CRAFT_STAT_TYPE" << key;
		}
	}

	void operator>>(QDataStream& out) const
	{
		out << (quint8)size();
		for(const_iterator it = begin(); it != end(); it++)
		{
			out << (quint8)(*it).first;
			(*it).second >> out;
		}
	}

	static CRAFT_STAT_TYPE StatType(COMMODITYID vehicleCID);

	static CRAFT_STAT_TYPE StatType(const DaAtom* atom);

	static CRAFT_STAT_TYPE StatType(const DaVessel* ship);

	static CRAFT_STAT_TYPE StatType(const DaVehicle* vehicle);
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Stats for all stations that can be E'd to, including vehicles and riding animals.

class AuPlayerStatsStation
{
public:
	quint8			m_HighestQ;				// Highest Q station occupied.
	quint32			m_Minutes;				// Minutes (roughly) station occupied.

public:
	AuPlayerStatsStation();

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Stations are indexed by STATION_STAT_TYPE.

enum STATION_STAT_TYPE
{
	EST_Invalid,
	EST_Animal,
	EST_BuildingConsoleFireControl,
	EST_BuildingConsoleNavigation,
	EST_BuildingConsoleShield,
	EST_BuildingConsoleSensor,
	EST_BuildingConsoleTransporter,
	EST_BuildingTurret,
	EST_ShipConsoleCommand,
	EST_ShipConsoleEngine,
	EST_ShipConsoleFireControl,
	EST_ShipConsoleGameTable,
	EST_ShipConsoleNavigation,
	EST_ShipConsolePilot,
	EST_ShipConsolePower,
	EST_ShipConsoleSensor,
	EST_ShipConsoleShield,
	EST_ShipConsoleSurgery,
	EST_ShipConsoleTransporter,
	EST_ShipTurret,
	EST_VehicleMotorBike,
	EST_VehicleRocket,
	EST_VehicleSailBoat,
	EST_VehicleSpaceFighterF1,
	EST_VehicleSpaceFighterF2,
	EST_VehicleSubmarine,
	EST_VehicleSUV,
	EST_VehicleAPC,
	EST_VehicleFarmMachinery,
	EST_VehicleHelicopter,
	EST_VehicleSpaceTransport,
};

const QString s_StationStatType[] =
{
	"Invalid",
	"Riding Animal",
	"Fire Control Station in Building",
	"Navigation Station in Building",
	"Shield Station in Building",
	"Sensor Station in Building",
	"Transporter Station in Building",
	"Turret in Building",
	"Captain's Chair on Spacecraft",
	"Engine Station on Spacecraft",
	"Fire Control Station on Spacecraft",
	"Game Table on Spacecraft",
	"Navigation Station on Spacecraft",
	"Helm on Spacecraft",
	"Power Relay Station on Spacecraft",
	"Sensor Station on Spacecraft",
	"Shield Station on Spacecraft",
	"Surgery Station on Spacecraft",
	"Transporter Station on Spacecraft",
	"Turret on Spacecraft",
	"Motor Bike",
	"Rocket",
	"Sail Boat",
	"Space Fighter F1",
	"Space Fighter F2",
	"Submarine",
	"SUV",
	"APC",
	"Farm Machinery",
	"Helicopter",
	"Space Transport",
};

class STATION_STAT_Hash
{
public:
	inline size_t operator()(const STATION_STAT_TYPE& o) const
	{
		return hash<unsigned int>()(o);
	}
};

class AuPlayerStatsStations : public unordered_map<STATION_STAT_TYPE, AuPlayerStatsStation, STATION_STAT_Hash>
{
public:
	int Minutes(COMMODITYID vehicleCID) const;

	int Minutes(AuStation::TYPE type, bool isShipStation) const;

	void operator<<(QDataStream& in)
	{
		quint8 qty;
		in >> qty;

		quint8 key;
		AuPlayerStatsStation val;
		for(; qty; qty--)
		{
			in >> key;
			val << in;
			if(/*key >= EST_Invalid && */key <= EST_VehicleSpaceTransport)
				insert(pair<STATION_STAT_TYPE, AuPlayerStatsStation>((STATION_STAT_TYPE)key, val));
			//else
				//qWarning() << "AuPlayerStatsStations read invalid STATION_STAT_TYPE" << key;
		}
	}

	void operator>>(QDataStream& out) const
	{
		out << (quint8)size();
		for(const_iterator it = begin(); it != end(); it++)
		{
			out << (quint8)(*it).first;
			(*it).second >> out;
		}
	}

	static STATION_STAT_TYPE StatType(COMMODITYID vehicleCID);

	// 'isShipStation' is false when manning a station on the ground, at a defense base.
	static STATION_STAT_TYPE StatType(AuStation::TYPE type, bool isShipStation);

	static STATION_STAT_TYPE StatType(const DaVehicle* vehicle);
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Stats for kinds of animals.

class AuPlayerStatsAnimal
{
public:
	quint16			m_DNA;						// count of times the person has gotten DNA from this kind of animal
	quint16			m_Named;					// count of times the person has named this kind of animal
	quint16			m_KilledBy;					// count of times the person has been killed by this kind of animal
	quint16			m_Kills;					// Count of times the person has killed this kind of animal
	quint16			m_Studied;					// count of times the person has zoologically studied this kind of animal

public:
	AuPlayerStatsAnimal();

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Animals are indexed by ANIMAL_STAT_TYPE.

// Animal types are the combinations of animal environment, strategy and ecological role. 
enum ANIMAL_STAT_TYPE
{
	AST_WaitingCarnivore,
	AST_WaitingHerbivore,
	AST_WaitingOmnivore,
	AST_WaitingScavenger,
	AST_GrazingCarnivore,
	AST_GrazingHerbivore,
	AST_GrazingOmnivore,
	AST_GrazingScavenger,
	AST_HuntingCarnivore,
	AST_HuntingHerbivore,
	AST_HuntingOmnivore,
	AST_HuntingScavenger,
	AST_SneakingCarnivore,
	AST_SneakingHerbivore,
	AST_SneakingOmnivore,
	AST_SneakingScavenger,
	AST_AerialWaitingCarnivore,
	AST_AerialWaitingHerbivore,
	AST_AerialWaitingOmnivore,
	AST_AerialWaitingScavenger,
	AST_AerialGrazingCarnivore,
	AST_AerialGrazingHerbivore,
	AST_AerialGrazingOmnivore,
	AST_AerialGrazingScavenger,
	AST_AerialHuntingCarnivore,
	AST_AerialHuntingHerbivore,
	AST_AerialHuntingOmnivore,
	AST_AerialHuntingScavenger,
	AST_AerialSneakingCarnivore,
	AST_AerialSneakingHerbivore,
	AST_AerialSneakingOmnivore,
	AST_AerialSneakingScavenger,
	AST_AmphibiousWaitingCarnivore,
	AST_AmphibiousWaitingHerbivore,
	AST_AmphibiousWaitingOmnivore,
	AST_AmphibiousWaitingScavenger,
	AST_AmphibiousGrazingCarnivore,
	AST_AmphibiousGrazingHerbivore,
	AST_AmphibiousGrazingOmnivore,
	AST_AmphibiousGrazingScavenger,
	AST_AmphibiousHuntingCarnivore,
	AST_AmphibiousHuntingHerbivore,
	AST_AmphibiousHuntingOmnivore,
	AST_AmphibiousHuntingScavenger,
	AST_AmphibiousSneakingCarnivore,
	AST_AmphibiousSneakingHerbivore,
	AST_AmphibiousSneakingOmnivore,
	AST_AmphibiousSneakingScavenger,
	AST_AquaerialWaitingCarnivore,
	AST_AquaerialWaitingHerbivore,
	AST_AquaerialWaitingOmnivore,
	AST_AquaerialWaitingScavenger,
	AST_AquaerialGrazingCarnivore,
	AST_AquaerialGrazingHerbivore,
	AST_AquaerialGrazingOmnivore,
	AST_AquaerialGrazingScavenger,
	AST_AquaerialHuntingCarnivore,
	AST_AquaerialHuntingHerbivore,
	AST_AquaerialHuntingOmnivore,
	AST_AquaerialHuntingScavenger,
	AST_AquaerialSneakingCarnivore,
	AST_AquaerialSneakingHerbivore,
	AST_AquaerialSneakingOmnivore,
	AST_AquaerialSneakingScavenger,
	AST_AquaticWaitingCarnivore,
	AST_AquaticWaitingHerbivore,
	AST_AquaticWaitingOmnivore,
	AST_AquaticWaitingScavenger,
	AST_AquaticGrazingCarnivore,
	AST_AquaticGrazingHerbivore,
	AST_AquaticGrazingOmnivore,
	AST_AquaticGrazingScavenger,
	AST_AquaticHuntingCarnivore,
	AST_AquaticHuntingHerbivore,
	AST_AquaticHuntingOmnivore,
	AST_AquaticHuntingScavenger,
	AST_AquaticSneakingCarnivore,
	AST_AquaticSneakingHerbivore,
	AST_AquaticSneakingOmnivore,
	AST_AquaticSneakingScavenger,
	AST_TriphibiousWaitingCarnivore,
	AST_TriphibiousWaitingHerbivore,
	AST_TriphibiousWaitingOmnivore,
	AST_TriphibiousWaitingScavenger,
	AST_TriphibiousGrazingCarnivore,
	AST_TriphibiousGrazingHerbivore,
	AST_TriphibiousGrazingOmnivore,
	AST_TriphibiousGrazingScavenger,
	AST_TriphibiousHuntingCarnivore,
	AST_TriphibiousHuntingHerbivore,
	AST_TriphibiousHuntingOmnivore,
	AST_TriphibiousHuntingScavenger,
	AST_TriphibiousSneakingCarnivore,
	AST_TriphibiousSneakingHerbivore,
	AST_TriphibiousSneakingOmnivore,
	AST_TriphibiousSneakingScavenger,
};

const QString s_AnimalStatType[] =
{
	"Waiting Carnivore",
	"Waiting Herbivore",
	"Waiting Omnivore",
	"Waiting Scavenger",
	"Grazing Carnivore",
	"Grazing Herbivore",
	"Grazing Omnivore",
	"Grazing Scavenger",
	"Hunting Carnivore",
	"Hunting Herbivore",
	"Hunting Omnivore",
	"Hunting Scavenger",
	"Sneaking Carnivore",
	"Sneaking Herbivore",
	"Sneaking Omnivore",
	"Sneaking Scavenger",
	"Aerial Waiting Carnivore",
	"Aerial Waiting Herbivore",
	"Aerial Waiting Omnivore",
	"Aerial Waiting Scavenger",
	"Aerial Grazing Carnivore",
	"Aerial Grazing Herbivore",
	"Aerial Grazing Omnivore",
	"Aerial Grazing Scavenger",
	"Aerial Hunting Carnivore",
	"Aerial Hunting Herbivore",
	"Aerial Hunting Omnivore",
	"Aerial Hunting Scavenger",
	"Aerial Sneaking Carnivore",
	"Aerial Sneaking Herbivore",
	"Aerial Sneaking Omnivore",
	"Aerial Sneaking Scavenger",
	"Amphibious Waiting Carnivore",
	"Amphibious Waiting Herbivore",
	"Amphibious Waiting Omnivore",
	"Amphibious Waiting Scavenger",
	"Amphibious Grazing Carnivore",
	"Amphibious Grazing Herbivore",
	"Amphibious Grazing Omnivore",
	"Amphibious Grazing Scavenger",
	"Amphibious Hunting Carnivore",
	"Amphibious Hunting Herbivore",
	"Amphibious Hunting Omnivore",
	"Amphibious Hunting Scavenger",
	"Amphibious Sneaking Carnivore",
	"Amphibious Sneaking Herbivore",
	"Amphibious Sneaking Omnivore",
	"Amphibious Sneaking Scavenger",
	"Aquaerial Waiting Carnivore",
	"Aquaerial Waiting Herbivore",
	"Aquaerial Waiting Omnivore",
	"Aquaerial Waiting Scavenger",
	"Aquaerial Grazing Carnivore",
	"Aquaerial Grazing Herbivore",
	"Aquaerial Grazing Omnivore",
	"Aquaerial Grazing Scavenger",
	"Aquaerial Hunting Carnivore",
	"Aquaerial Hunting Herbivore",
	"Aquaerial Hunting Omnivore",
	"Aquaerial Hunting Scavenger",
	"Aquaerial Sneaking Carnivore",
	"Aquaerial Sneaking Herbivore",
	"Aquaerial Sneaking Omnivore",
	"Aquaerial Sneaking Scavenger",
	"Aquatic Waiting Carnivore",
	"Aquatic Waiting Herbivore",
	"Aquatic Waiting Omnivore",
	"Aquatic Waiting Scavenger",
	"Aquatic Grazing Carnivore",
	"Aquatic Grazing Herbivore",
	"Aquatic Grazing Omnivore",
	"Aquatic Grazing Scavenger",
	"Aquatic Hunting Carnivore",
	"Aquatic Hunting Herbivore",
	"Aquatic Hunting Omnivore",
	"Aquatic Hunting Scavenger",
	"Aquatic Sneaking Carnivore",
	"Aquatic Sneaking Herbivore",
	"Aquatic Sneaking Omnivore",
	"Aquatic Sneaking Scavenger",
	"Triphibious Waiting Carnivore",
	"Triphibious Waiting Herbivore",
	"Triphibious Waiting Omnivore",
	"Triphibious Waiting Scavenger",
	"Triphibious Grazing Carnivore",
	"Triphibious Grazing Herbivore",
	"Triphibious Grazing Omnivore",
	"Triphibious Grazing Scavenger",
	"Triphibious Hunting Carnivore",
	"Triphibious Hunting Herbivore",
	"Triphibious Hunting Omnivore",
	"Triphibious Hunting Scavenger",
	"Triphibious Sneaking Carnivore",
	"Triphibious Sneaking Herbivore",
	"Triphibious Sneaking Omnivore",
	"Triphibious Sneaking Scavenger",
};

class ANIMAL_STAT_Hash
{
public:
	inline size_t operator()(const ANIMAL_STAT_TYPE& o) const
	{
		return hash<unsigned int>()(o);
	}
};

class AuPlayerStatsAnimals : public unordered_map<ANIMAL_STAT_TYPE, AuPlayerStatsAnimal, ANIMAL_STAT_Hash>
{
public:
	void operator<<(QDataStream& in)
	{
		quint8 qty;
		in >> qty;

		quint8 key;
		AuPlayerStatsAnimal val;
		for(; qty; qty--)
		{
			in >> key;
			val << in;
			if(/*key >= AST_WaitingCarnivore && */key <= AST_TriphibiousSneakingScavenger)
				insert(pair<ANIMAL_STAT_TYPE, AuPlayerStatsAnimal>((ANIMAL_STAT_TYPE)key, val));
			//else
				//qWarning() << "AuPlayerStatsAnimals read invalid ANIMAL_STAT_TYPE" << key;
		}
	}

	void operator>>(QDataStream& out) const
	{
		out << (quint8)size();
		for(const_iterator it = begin(); it != end(); it++)
		{
			out << (quint8)(*it).first;
			(*it).second >> out;
		}
	}

	inline static ANIMAL_STAT_TYPE StatType(int environ, AuDNABase::ECOSTRATEGY strategy, ECOROLE role)
	{
		int statType = environ*16;	// Calc enum offset to first entry of the 'environ's range.
		statType += strategy*4;		// Offset to first entry of the 'strategy's range.
		statType += role;			// Offset to specific 'role'.
		return (ANIMAL_STAT_TYPE)statType;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Stats for kinds of animals, by DNA.

class AuPlayerStatsCreature
{
public:
	quint16			m_DNA;						// count of times the person has gotten DNA from this kind of animal
	quint16			m_Named;					// count of times the person has named this kind of animal
	quint16			m_KilledBy;					// count of times the person has been killed by this kind of animal
	quint16			m_Kills;					// Count of times the person has killed this kind of animal
	bool			m_Studied;					// true if the person has zoologically studied this kind of animal

public:
	AuPlayerStatsCreature();

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Creatures are indexed by their species

class AuPlayerStatsCreatures : public unordered_map<AuSpecies, AuPlayerStatsCreature, AuSpeciesHash>
{
public:
	void operator>>(QDataStream& out) const
	{
		quint16 qty = (quint16)size();
		qty |= 0x8000;
		out << qty;

		for(const_iterator it = begin(); it != end(); it++)
		{
			out << (*it).first;
			(*it).second >> out;
		}
	}

	void operator<<(QDataStream& in);

	quint64 MemoryUsage() const
	{
		quint64 mem = sizeof(AuPlayerStatsCreatures);
		for(const_iterator it = begin(); it != end(); it++)
			mem += (*it).first.MemoryUsage() + sizeof(AuPlayerStatsCreature);
		return mem;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Stats for spacecraft systems.

class AuPlayerStatsSystem
{
public:
	quint16			m_Repairs;				// count of times the person has repaired this system by hand
	quint16			m_Upgrades;				// count of times the person has upgraded this system by hand

public:
	AuPlayerStatsSystem();

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Systems are indexed by SYSTEM

class AuPlayerStatsSystems : public unordered_map<SYSTEM, AuPlayerStatsSystem, AuShipSystemHash>
{
public:
	void operator<<(QDataStream& in)
	{
		quint8 qty;
		in >> qty;

		quint8 key;
		AuPlayerStatsSystem val;
		for(; qty; qty--)
		{
			in >> key;
			val << in;
			if(/*key >= SYS_CommandAndControl && */key < SYS_Count)
				insert(pair<SYSTEM, AuPlayerStatsSystem>((SYSTEM)key, val));
			//else
				//qWarning() << "AuPlayerStatsSystems read invalid SYSTEM" << key;
		}
	}

	void operator>>(QDataStream& out) const
	{
		out << (quint8)size();
		for(const_iterator it = begin(); it != end(); it++)
		{
			out << (quint8)(*it).first;
			(*it).second >> out;
		}
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Stats for killing a person or citizen.

class AuPlayerStatsKill
{
public:
	quint16			m_KilledBy;
	quint16			m_Kills;

public:
	AuPlayerStatsKill();

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Kills are indexed by KILL_STAT_TYPE.

enum KILL_STAT_TYPE
{
	KST_Invalid,
	KST_CrewFriend,
	KST_CrewNeutral,
	KST_CrewEnemy,
	KST_CrewEmpire,
	KST_CrewAlly,
	KST_CrewVassal,
	KST_MotorBikeFriend,
	KST_MotorBikeNeutral,
	KST_MotorBikeEnemy,
	KST_MotorBikeEmpire,
	KST_MotorBikeAlly,
	KST_MotorBikeVassal,
	KST_OfficerFriend,
	KST_OfficerNeutral,
	KST_OfficerEnemy,
	KST_OfficerEmpire,
	KST_OfficerAlly,
	KST_OfficerVassal,
	KST_PassengerFriend,
	KST_PassengerNeutral,
	KST_PassengerEnemy,
	KST_PassengerEmpire,
	KST_PassengerAlly,
	KST_PassengerVassal,
	KST_AvatarFriend,
	KST_AvatarNeutral,
	KST_AvatarEnemy,
	KST_AvatarEmpire,
	KST_AvatarAlly,
	KST_AvatarVassal,
	KST_RocketFriend,
	KST_RocketNeutral,
	KST_RocketEnemy,
	KST_RocketEmpire,
	KST_RocketAlly,
	KST_RocketVassal,
	KST_SailBoatFriend,
	KST_SailBoatNeutral,
	KST_SailBoatEnemy,
	KST_SailBoatEmpire,
	KST_SailBoatAlly,
	KST_SailBoatVassal,
	KST_SpaceFighterF1Friend,
	KST_SpaceFighterF1Neutral,
	KST_SpaceFighterF1Enemy,
	KST_SpaceFighterF1Empire,
	KST_SpaceFighterF1Ally,
	KST_SpaceFighterF1Vassal,
	KST_SpaceFighterF2Friend,
	KST_SpaceFighterF2Neutral,
	KST_SpaceFighterF2Enemy,
	KST_SpaceFighterF2Empire,
	KST_SpaceFighterF2Ally,
	KST_SpaceFighterF2Vassal,
	KST_StationFriend,
	KST_StationNeutral,
	KST_StationEnemy,
	KST_StationEmpire,
	KST_StationAlly,
	KST_StationVassal,
	KST_SpaceshipFriend,
	KST_SpaceshipNeutral,
	KST_SpaceshipEnemy,
	KST_SpaceshipEmpire,
	KST_SpaceshipAlly,
	KST_SpaceshipVassal,
	KST_StarshipFriend,
	KST_StarshipNeutral,
	KST_StarshipEnemy,
	KST_StarshipEmpire,
	KST_StarshipAlly,
	KST_StarshipVassal,
	KST_SubmarineFriend,
	KST_SubmarineNeutral,
	KST_SubmarineEnemy,
	KST_SubmarineEmpire,
	KST_SubmarineAlly,
	KST_SubmarineVassal,
	KST_SUVFriend,
	KST_SUVNeutral,
	KST_SUVEnemy,
	KST_SUVEmpire,
	KST_SUVAlly,
	KST_SUVVassal,
	KST_TroopFriend,
	KST_TroopNeutral,
	KST_TroopEnemy,
	KST_TroopEmpire,
	KST_TroopAlly,
	KST_TroopVassal,
	KST_APCFriend,
	KST_APCNeutral,
	KST_APCEnemy,
	KST_APCEmpire,
	KST_APCAlly,
	KST_APCVassal,
	KST_FarmMachineryFriend,
	KST_FarmMachineryNeutral,
	KST_FarmMachineryEnemy,
	KST_FarmMachineryEmpire,
	KST_FarmMachineryAlly,
	KST_FarmMachineryVassal,
	KST_HelicopterFriend,
	KST_HelicopterNeutral,
	KST_HelicopterEnemy,
	KST_HelicopterEmpire,
	KST_HelicopterAlly,
	KST_HelicopterVassal,
	KST_SpaceTransportFriend,
	KST_SpaceTransportNeutral,
	KST_SpaceTransportEnemy,
	KST_SpaceTransportEmpire,
	KST_SpaceTransportAlly,
	KST_SpaceTransportVassal,
};

const QString s_KillStatType[] =
{
	"Invalid",
	"Crew, Friend",
	"Crew, Neutral",
	"Crew, Enemy",
	"Crew, Empire",
	"Crew, Ally",
	"Crew, Vassal",
	"Motor Bike, Friend",
	"Motor Bike, Neutral",
	"Motor Bike, Enemy",
	"Motor Bike, Empire",
	"Motor Bike, Ally",
	"Motor Bike, Vassal",
	"Officer, Friend",
	"Officer, Neutral",
	"Officer, Enemy",
	"Officer, Empire",
	"Officer, Ally",
	"Officer, Vassal",
	"Passenger, Friend",
	"Passenger, Neutral",
	"Passenger, Enemy",
	"Passenger, Empire",
	"Passenger, Ally",
	"Passenger, Vassal",
	"Avatar, Friend",
	"Avatar, Neutral",
	"Avatar, Enemy",
	"Avatar, Empire",
	"Avatar, Ally",
	"Avatar, Vassal",
	"Rocket, Friend",
	"Rocket, Neutral",
	"Rocket, Enemy",
	"Rocket, Empire",
	"Rocket, Ally",
	"Rocket, Vassal",
	"Sail Boat, Friend",
	"Sail Boat, Neutral",
	"Sail Boat, Enemy",
	"Sail Boat, Empire",
	"Sail Boat, Ally",
	"Sail Boat, Vassal",
	"Space Fighter F1, Friend",
	"Space Fighter F1, Neutral",
	"Space Fighter F1, Enemy",
	"Space Fighter F1, Empire",
	"Space Fighter F1, Ally",
	"Space Fighter F1, Vassal",
	"Space Fighter F2, Friend",
	"Space Fighter F2, Neutral",
	"Space Fighter F2, Enemy",
	"Space Fighter F2, Empire",
	"Space Fighter F2, Ally",
	"Space Fighter F2, Vassal",
	"Station, Friend",
	"Station, Neutral",
	"Station, Enemy",
	"Station, Empire",
	"Station, Ally",
	"Station, Vassal",
	"Spaceship, Friend",
	"Spaceship, Neutral",
	"Spaceship, Enemy",
	"Spaceship, Empire",
	"Spaceship, Ally",
	"Spaceship, Vassal",
	"Starship, Friend",
	"Starship, Neutral",
	"Starship, Enemy",
	"Starship, Empire",
	"Starship, Ally",
	"Starship, Vassal",
	"Submarine, Friend",
	"Submarine, Neutral",
	"Submarine, Enemy",
	"Submarine, Empire",
	"Submarine, Ally",
	"Submarine, Vassal",
	"SUV, Friend",
	"SUV, Neutral",
	"SUV, Enemy",
	"SUV, Empire",
	"SUV, Ally",
	"SUV, Vassal",
	"Troop, Friend",
	"Troop, Neutral",
	"Troop, Enemy",
	"Troop, Empire",
	"Troop, Ally",
	"Troop, Vassal",
	"APC, Friend",
	"APC, Neutral",
	"APC, Enemy",
	"APC, Empire",
	"APC, Ally",
	"APC, Vassal",
	"Farm Machinery, Friend",
	"Farm Machinery, Neutral",
	"Farm Machinery, Enemy",
	"Farm Machinery, Empire",
	"Farm Machinery, Ally",
	"Farm Machinery, Vassal",
	"Helicopter, Friend",
	"Helicopter, Neutral",
	"Helicopter, Enemy",
	"Helicopter, Empire",
	"Helicopter, Ally",
	"Helicopter, Vassal",
	"Space Transport, Friend",
	"Space Transport, Neutral",
	"Space Transport, Enemy",
	"Space Transport, Empire",
	"Space Transport, Ally",
	"Space Transport, Vassal",
};

class KILL_STAT_Hash
{
public:
	inline size_t operator()(const KILL_STAT_TYPE& o) const
	{
		return hash<unsigned int>()(o);
	}
};

class AuPlayerStatsKills : public unordered_map<KILL_STAT_TYPE, AuPlayerStatsKill, KILL_STAT_Hash>
{
public:
	void operator<<(QDataStream& in)
	{
		quint8 qty;
		in >> qty;

		quint8 key;
		AuPlayerStatsKill val;
		for(; qty; qty--)
		{
			in >> key;
			val << in;
			if(/*key >= KST_Invalid && */key <= KST_SpaceTransportVassal)
				insert(pair<KILL_STAT_TYPE, AuPlayerStatsKill>((KILL_STAT_TYPE)key, val));
			//else
				//qWarning() << "AuPlayerStatsKills read invalid KILL_STAT_TYPE" << key;
		}
	}

	void operator>>(QDataStream& out) const
	{
		out << (quint8)size();
		for(const_iterator it = begin(); it != end(); it++)
		{
			out << (quint8)(*it).first;
			(*it).second >> out;
		}
	}

	static KILL_STAT_TYPE StatType(const DaUnit* unitKilled, POLITICAL_STANCE stanceToUnit);
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Key is WEAPONEFFECT that caused death; value is count of deaths due to that cause.

class AuPlayerStatsDeaths : public unordered_map<WEAPONEFFECT, quint16, WEAPONEFFECT_Hash>
{
public:
	void operator<<(QDataStream& in)
	{
		quint8 qty;
		in >> qty;

		quint8 key;
		quint16 val;
		for(; qty; qty--)
		{
			in >> key;
			in >> val;
			if(/*key >= WE_None && */key <= WE_Corrosion)
				insert(pair<WEAPONEFFECT, quint16>((WEAPONEFFECT)key, val));
			//else
				//qWarning() << "AuPlayerStatsDeaths read invalid WEAPONEFFECT" << key;
		}
	}

	void operator>>(QDataStream& out) const
	{
		out << (quint8)size();
		for(const_iterator it = begin(); it != end(); it++)
		{
			out << (quint8)(*it).first;
			out << (*it).second;
		}
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Stats related to industries.

class AuPlayerStatsBuilding
{
public:
	quint16					m_Built;					// count of buildings ever planned, whether or not construction completed
	quint32					m_ConstructionLabor;		// total construction labor applied to the building type
	quint16					m_ManufactureGo;			// count of times Go button pushed.

public:
	AuPlayerStatsBuilding();

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Key is INDUSTRYID; value is industry stats.

class AuPlayerStatsBuildings : public unordered_map<INDUSTRYID, AuPlayerStatsBuilding, INDUSTRYID_Hash>
{
public:
	void operator<<(QDataStream& in)
	{
		quint8 qty;
		in >> qty;

		quint8 key;
		AuPlayerStatsBuilding val;
		for(; qty; qty--)
		{
			in >> key;
			val << in;
			if(/*key >= IID_None && */key < IID_Count)
				insert(pair<INDUSTRYID, AuPlayerStatsBuilding>((INDUSTRYID)key, val));
			//else
				//qWarning() << "AuPlayerStatsBuildings read invalid INDUSTRYID" << key;
		}
	}

	void operator>>(QDataStream& out) const
	{
		out << (quint8)size();
		for(const_iterator it = begin(); it != end(); it++)
		{
			out << (quint8)(*it).first;
			(*it).second >> out;
		}
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Key is INDUSTRYID; value is count of buildings destroyed of that industry.

class AuPlayerStatsBuildingKills : public unordered_map<INDUSTRYID, quint16, INDUSTRYID_Hash>
{
public:
	void operator<<(QDataStream& in)
	{
		quint8 qty;
		in >> qty;

		quint8 key;
		quint16 val;
		for(; qty; qty--)
		{
			in >> key;
			in >> val;
			if(/*key >= IID_None && */key < IID_Count)
				insert(pair<INDUSTRYID, quint16>((INDUSTRYID)key, val));
			//else
				//qWarning() << "AuPlayerStatsBuildingKills read invalid INDUSTRYID" << key;
		}
	}

	void operator>>(QDataStream& out) const
	{
		out << (quint8)size();
		for(const_iterator it = begin(); it != end(); it++)
		{
			out << (quint8)(*it).first;
			out << (*it).second;
		}
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Key is building design id; value is count of building designs.

class AuPlayerStatsBuildingDesigns : public unordered_map<quint32, quint16, AuDesignIdHash>
{
public:
	void operator<<(QDataStream& in)
	{
		quint16 qty;
		in >> qty;

		quint32 key;
		quint16 val;
		for(; qty; qty--)
		{
			in >> key;
			in >> val;
			insert(pair<quint32, quint16>(key, val));
		}
	}

	void operator>>(QDataStream& out) const
	{
		out << (quint16)size();
		for(const_iterator it = begin(); it != end(); it++)
		{
			out << (quint32)(*it).first;
			out << (*it).second;
		}
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Key is building design industry name; value is count of designs.

class AuPlayerStatsIndustryDesigns : public map<QString, quint16>
{
public:
	void operator<<(QDataStream& in)
	{
		quint8 qty;
		in >> qty;

		QString key;
		quint16 val;
		for(; qty; qty--)
		{
			in >> key;
			in >> val;
			insert(pair<QString, quint16>(key, val));
		}
	}

	void operator>>(QDataStream& out) const
	{
		out << (quint8)size();
		for(const_iterator it = begin(); it != end(); it++)
		{
			out << (*it).first;
			out << (*it).second;
		}
	}

	quint32 Total() const
	{
		quint32 t = 0;
		for(const_iterator it = begin(); it != end(); it++)
			t += (*it).second;
		return t;
	}

	quint64 MemoryUsage() const
	{
		quint64 mem = sizeof(AuPlayerStatsIndustryDesigns);
		for(const_iterator it = begin(); it != end(); it++)
			mem += StringMemoryUsage((*it).first) + sizeof(quint16);
		return mem;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Key is ship design id; value is count of ship designs.

class AuPlayerStatsShipDesigns : public unordered_map<quint32, quint16, AuDesignIdHash>
{
public:
	void operator<<(QDataStream& in)
	{
		quint16 qty;
		in >> qty;

		quint32 key;
		quint16 val;
		for(; qty; qty--)
		{
			in >> key;
			in >> val;
			insert(pair<quint32, quint16>(key, val));
		}
	}

	void operator>>(QDataStream& out) const
	{
		out << (quint16)size();
		for(const_iterator it = begin(); it != end(); it++)
		{
			out << (quint32)(*it).first;
			out << (*it).second;
		}
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuPlayerStats
{
friend class AuEmpireReport;
private:
	mutable QMutex						m_StatsLock;
	quint16					m_BoardedCrew;				// count of crew boarded
	quint16					m_BoardedCryo;				// count of cryo boarded
	quint16					m_BoardedOfficers;			// count of officers boarded
	quint16					m_BoardedPassengers;		// count of passengers boarded
	quint16					m_BoardedTroops;			// count of troops boarded
	double					m_BountyCollected;			// Total bounty collected, in cronos.
	quint16					m_BountyCollections;		// Count of bounties collected.
	double					m_BountyGranted;			// Total bounty granted, in cronos. Bounties collected off my head.
	quint16					m_BountyGrants;				// Count of bounties granted.
	AuPlayerStatsBuildingDesigns	m_BuildingDesignsBought;	// Number of building designs that the person has bought.
	AuPlayerStatsIndustryDesigns	m_BuildingDesignsBurned;	// Key is industry name, value is count.
	AuPlayerStatsBuildingDesigns	m_BuildingDesignsMade;		// Number of person's building designs that have been built.
	AuPlayerStatsIndustryDesigns	m_BuildingDesignsPublished;	// Key is industry name, value is count.
	AuPlayerStatsBuildingDesigns	m_BuildingDesignsSold;		// Number of person's building designs bought by players.
	AuPlayerStatsBuildingKills	m_BuildingsDestroyedFriend;		// count of friendly industries destroyed
	AuPlayerStatsBuildingKills	m_BuildingsDestroyedNeutral;	// count of neutral industries destroyed
	AuPlayerStatsBuildingKills	m_BuildingsDestroyedEnemy;		// count of enemy industries destroyed
	AuPlayerStatsBuildingKills	m_BuildingsDestroyedEmpire;		// count of empire industries destroyed
	AuPlayerStatsBuildingKills	m_BuildingsDestroyedAlly;		// count of ally industries destroyed
	AuPlayerStatsBuildingKills	m_BuildingsDestroyedVassal;		// count of vassal industries destroyed
	AuPlayerStatsBuildings	m_BuildingsHabitable;		// building stats in tame environments
	AuPlayerStatsBuildings	m_BuildingsHarsh;			// building stats in harsh environments
	quint16					m_BurnedStation;			// count of times person has burned a design onto a disk.
	quint16					m_BurnedSpaceship;			// count of times person has burned a design onto a disk.
	quint16					m_BurnedStarship;			// count of times person has burned a design onto a disk.
	quint16					m_ChildrenBorn;				// count of times person has given birth.
	quint16					m_ChildrenCarried;			// count of times person has gotten pregnant
	quint16					m_ChildrenFathered;			// count of times person has impregnated someone
	quint16					m_ClaimedDifferentDNA;		// Habitable worlds claimed with different DNA from avatar.
	quint16					m_ClaimedSameDNA;			// Habitable worlds claimed with same DNA as avatar.
	quint16					m_ClaimedUninhabitable;		// Uninhabitable worlds claimed.
	AuPlayerStatsCreatures	m_CreaturesTame;			// tame creatures (animals, people and citizens)
	AuPlayerStatsCreatures	m_CreaturesWild;			// wild creatures (animals)
	AuPlayerStatsDeaths		m_Deaths;
	quint16					m_DebarkedCryo;				// count of cryo travellers debarked
	quint16					m_DebarkedPassengers;		// count of passengers debarked
	quint16					m_EVABeamFrom;				// count of times beamed from EVA.
	quint16					m_EVABeamTo;				// count of times beamed to EVA.
	quint16					m_EVADeadHead;				// count of times dead-headed EVA.
	quint32					m_EVAMinutes;				// Minutes (roughly) in EVA.
	quint16					m_EVASectorTraversed;		// count of times traversed sectors EVA.
	quint16					m_FlagsCapturedBase;		// Base flags captured.
	quint16					m_FlagsCapturedCapitalEmpire;	// Empire capital city flags captured.
	quint16					m_FlagsCapturedCapitalGalaxy;	// Galaxy capital city flags captured.
	quint16					m_FlagsCapturedCapitalSector;	// Sector capital city flags captured.
	quint16					m_FlagsCapturedCity;		// City flags captured.
	AuPlayerStatsItems		m_Items;
	AuPlayerStatsKills		m_Kills;
	quint16					m_NamedSolarSystems;				// Count of sectors named by the person.
	quint16					m_NamedSpecies;				// Count of different species named by the person.
	quint16					m_NamedSectors;				// Count of solar systems named by the person.
	quint32					m_OnLineMinutes;
	quint16					m_OrdersMission;			// count of times the person issued a mission order
	quint16					m_OrdersSquad;				// count of times the person issued a auquad order
	AuPlayerStatsPlanets	m_Planets;
	quint16					m_PublishedStation;			// count of times the person has published a design.
	quint16					m_PublishedSpaceship;		// count of times the person has published a design.
	quint16					m_PublishedStarship;		// count of times the person has published a design.
	quint32					m_ShipmentsDelivered;		// Shipments of someone else's I delivered.
	quint32					m_ShipmentsJettisonedByMe;	// Shipments of someone else's that I opened.
	quint32					m_ShipmentsJettisonedByOther;	// Shipments of mine that someone else opened.
	quint32					m_ShipmentsLoaded;			// Shipments of I loaded.
	quint32					m_ShipmentsMineDelivered;	// Shipments of mine that I delivered.
	quint32					m_ShipmentsMineJettisoned;	// Shipments of mine that I opened.
	quint32					m_ShipmentsMineOpened;		// Shipments of mine that I opened.
	quint32					m_ShipmentsOpenedByMe;		// Shipments of someone else's that I opened.
	quint32					m_ShipmentsOpenedByOther;	// Shipments of mine that someone else opened.
	AuPlayerStatsShipDesigns	m_ShipDesignsBought;	// Number of ship designs that the person has bought.
	AuPlayerStatsShipDesigns	m_ShipDesignsMade;		// Number of person's ship designs that have been manufactured.
	AuPlayerStatsShipDesigns	m_ShipDesignsSold;		// Number of person's ship designs that have been bought by players.
	AuPlayerStatsCrafts		m_ShipsAndVehicles;
	quint16					m_SpawnRecalls;				// Number of times player initiates recall home.
	AuPlayerStatsStars		m_Stars;
	AuPlayerStatsStations	m_Stations;
	quint16					m_StoriesCompleted;			// Number of story lines completed.
	QSet<quint32>			m_StoriesFinished;			// Script ids of stories that have been finished.
	quint16					m_StoriesStarted;			// Number of story lines started.
	AuPlayerStatsSystems	m_Systems;
	quint16					m_WormholesExplored;		// Number of nu wormholes explored, whether or not credited with discovery.
	quint16					m_WormholesNamed;			// Number of wormholes named after the person.

public:
	AuPlayerStats();
	virtual ~AuPlayerStats();
	AuPlayerStats(const AuPlayerStats& c);

	const AuPlayerStats& operator=(const AuPlayerStats& c);

	void operator<<(QDataStream& in);

	void operator>>(QDataStream& out) const;

public:
	void AnimalRidden();

	quint32 ArchitectScore() const;

	void BoardedCrew(quint32 count);

	void BoardedCryo(quint32 count);

	void BoardedOfficer();

	void BoardedPassengers(quint32 count);

	void BoardedTroops(quint32 count);

	void BountyCollected(double amount);

	inline quint16 BountiesCollected() const
	{
		return m_BountyCollections;
	}

	void BountyGranted(double amount);

	void BuildingConstruction(INDUSTRYID iid, const DaWorld* world, quint32 laborApplied);

	void BuildingDesignBought(quint32 designId);

	void BuildingDesignBuilt(quint32 designId);

	void BuildingDesignBurned(const QString& industryName);

	void BuildingDesignPublished(const QString& industryName);

	void BuildingDesignSold(quint32 designId);

	void BuildingManufacture(INDUSTRYID iid, const DaWorld* world);

	void BuildingPlanned(INDUSTRYID iid, const DaWorld* world, int levels);

	quint32 BuildingsPlanned() const;

	void BuildingStationEntered(AuStation::TYPE type, quint8 quality);

	void BuildingStationOperated(AuStation::TYPE type, quint8 quality);

	void CapturedBase();

	void CapturedCity();

	void CapturedEmpireCapital();

	void CapturedGalaxyCapital();

	void CapturedSectorCapital();

	void ChildBorn();

	void ChildCarried();

	void ChildFathered();

	void ClaimedHabitableDifferentDNA();

	void ClaimedHabitableSameDNA();

	void ClaimedUninhabitable();

	void CreatureDNACollected(const DaCreature* creature);

	void CreatureSpeciesNamed(const AuSpecies& species, bool isWildAnimal);

	quint32 CreatureSpeciesNamed() const;

	void CreatureSpecieStudied(const DaCreature* creature);

	void DeadHeaded(const DaAtom* ridingAboard);

	void DebarkedCryo(quint32 count);

	void DebarkedPassengers(quint32 count);

	void EVA();

	void EVABeamedFrom();

	void EVABeamedTo();

	void EVADeadHead();

	// Returns the number of minutes of EVA experience.
	int EVAMinutes() const;

	void EVASectorChange();

	quint32 ExploreScore() const;

	bool FinishedStory(quint32 scriptId) const;

	// Removes memory of finishing a story script.
	void ForgetStory(quint32 scriptId);

	// Returns true if an item has ever been possessed.
	bool HadItem(COMMODITYID cid) const;

	bool HasConstructedBuildingHabitable(INDUSTRYID iid) const;

	bool HasConstructedBuildingHarsh(INDUSTRYID iid) const;

	void ItemBoughtAsCargo(const AuItem& theItem);

	void ItemBoughtAsInventory(const AuItem& theItem);

	void ItemForaged(const AuItem& theItem);

	void ItemHandCrafted(const AuItem& theItem, bool industryRequired);

	void ItemHandCrafted(COMMODITYID cid, quint8 quality, bool industryRequired);

	void ItemPossessed(const AuItem& theItem);

	quint32 ItemsForaged() const;

	// Returns total items hand crafted.
	quint32 ItemsHandCrafted() const;

	void ItemSoldAsCargo(const AuItem& theItem);

	void ItemSoldAsInventory(const AuItem& theItem);

	void ItemUsedAsWeapon(COMMODITYID cid, quint8 quality);

	void ItemUsedAsWeapon(const AuItem& theItem);

	void ItemUsedOnOther(const AuItem& theItem);

	void ItemUsedOnSelf(const AuItem& theItem);

	void KilledBy(WEAPONEFFECT effect);

	// 'killer' may be 0
	void KilledByUnit(const DaUnit* killer, POLITICAL_STANCE stanceToUnit, COMMODITYID weaponCID);

	void KilledUnit(const DaUnit* unitKilled, POLITICAL_STANCE stanceToUnit, COMMODITYID weaponCID);

	quint64 MemoryUsage() const;

	quint16 MissionOrders() const;

	void NamedSystem();

	void NamedSector();

	void OnLineMinute();

	// Returns the number of minutes on line.
	quint32 OnLineMinutes() const;

	void OrderMission();

	void OrderSquad();

	void PlanetBeamedDown(const DaWorld* toPlanet, bool toWater);

	void PlanetBeamedUp(const DaWorld* fromPlanet, bool fromWater);

	void PlanetDeepestSwim(const DaWorld* planet, float depth);

	void PlanetEntry(const DaWorld* planet, const DaAtom* ridingAboard);

	void PlanetExit(const DaWorld* planet, const DaAtom* ridingAboard);

	void PlanetHighestFall(const DaWorld* planet, float altitude);

	void PlanetTreaded(const DaWorld* planet);

	void SectorChange(const DaAtom* ridingAboard);

	void ShipDesignBought(quint32 designId);

	void ShipDesignBurned(SPACECRAFT_STRUCTURE structure);

	void ShipDesignManufactured(quint32 designId);

	void ShipDesignPublished(SPACECRAFT_STRUCTURE structure);

	void ShipDesignSold(quint32 designId);

	void SpacecraftCaptured(const DaVessel* ship);

	quint32 SpacecraftCaptureScore() const;

	void SpacecraftRepairedByHand(const DaVessel* ship);

	void SpacecraftRepairsBought(const DaVessel* ship);

	void SpacecraftStationEntered(AuStation::TYPE type, quint8 quality);

	void SpacecraftStationOperated(AuStation::TYPE type, quint8 quality);

	void SpacecraftSystemRepaired(SYSTEM system);

	void SpacecraftSystemUpgraded(SYSTEM system);

	void SpacecraftUpgradeBought(const DaVessel* ship);

	void SpawnRecallCanceled();

	void SpawnRecalled();

	quint16 SquadOrders() const;

	void StoriesFinished(QList<quint32>& scriptIds) const;

	void StoryCompleted(quint32 scriptId);

	void StoryStarted();

	void SunBurn(const DaStar* star, const DaAtom* ridingAboard);

	void ShipmentDelivered();

	quint32 ShipmentsDelivered() const;

	void ShipmentJettisonedByMe();

	quint32 ShipmentsJettisonedByMe() const;

	void ShipmentJettisonedByOther();

	quint32 ShipmentsJettisonedByOther() const;

	void ShipmentLoaded();

	quint32 ShipmentsLoaded() const;

	void ShipmentMineDelivered();

	quint32 ShipmentsMineDelivered() const;

	void ShipmentMineJettisoned();

	quint32 ShipmentsMineJettisoned() const;

	void ShipmentMineOpened();

	quint32 ShipmentsMineOpened() const;

	void ShipmentOpenedByMe();

	quint32 ShipmentsOpenedByMe() const;

	void ShipmentOpenedByOther();

	quint32 ShipmentsOpenedByOther() const;

	int StationMinutes(AuStation::TYPE type, bool isShipStation) const;

	void VehicleCaptured(const DaVehicle* vehicle);

	void VehicleEntered(const DaVehicle* vehicle);

	// Returns the number of minutes of Vehicle experience.
	int VehicleMinutes(COMMODITYID vehicleCID) const;

	void VehicleOperated(const DaVehicle* vehicle);

	void VehicleRepairedByHand(const DaVehicle* vehicle);

	void VehicleRepairsBought(const DaVehicle* vehicle);

	quint32 VehiclesCaptured() const;

	quint32 VehiclesUsedAsWeapon() const;

	void Wormholed(const DaVessel* ridingAboard);

	void WormholeExplored();

	void WormholeNamed();
};

inline QDataStream& operator<<(QDataStream& out, const AuPlayerStats& o)
{
	o >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuPlayerStats& o)
{
	o << in;
	return in;
}

#endif // AuPlayerStats_H
