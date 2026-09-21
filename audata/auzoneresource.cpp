//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auzoneresource.cpp,v 1.55 2022/02/19 18:20:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auzoneresource.h"
//
#include "audice.h"

AuZoneResource::AuZoneResource()
{
	m_Adamantite =0;
	m_AdamantiteQ =0;
	m_AirQ =0;
	m_Animals =0;				// Quantity factor. (0-100)
	m_AnimalsQ =0;				// Quality factor. (0-255)
	m_AntifluxParticleQ =0;
	m_Atmosphere =0;
	m_AtmosphereQ =0;
	m_Beans =0;
	m_BeansQ =0;
	m_Bolite =0;
	m_BoliteQ =0;
	m_BorexinoPrecipQ =0;
	m_Coal =0;
	m_CoalQ =0;
	m_CrudeOil =0;
	m_CrudeOilQ =0;
	m_CryozineQ =0;
	m_Crystals =0;
	m_CrystalsQ =0;
	m_Eludium =0;
	m_EludiumQ =0;
	m_Fiber =0;
	m_FiberQ =0;
	m_Fish =0;
	m_FishQ =0;
	m_Flomentum =0;
	m_FlomentumQ =0;
	m_Fruit =0;
	m_FruitQ =0;
	m_Gems =0;
	m_GemsQ =0;
	m_GoldOre =0;
	m_GoldOreQ =0;
	m_Grain =0;
	m_GrainQ =0;
	m_Grapes =0;
	m_GrapesQ =0;
	m_Herbs =0;
	m_HerbsQ =0;
	m_Hops =0;
	m_HopsQ =0;
	m_HydrogenQ =0;
	m_Ice =0;
	m_IceQ =0;
	m_IoplasmaQ =0;
	m_Lumenite =0;
	m_LumeniteQ =0;
	m_MagmexQ =0;
	m_Minerals =0;
	m_MineralsQ =0;
	m_MyrathaneQ =0;
	m_NaturalGas =0;
	m_NaturalGasQ =0;
	m_Nuts =0;
	m_NutsQ =0;
	m_Ore =0;
	m_OreQ =0;
	m_Phlogiston =0;
	m_PhlogistonQ =0;
	m_Polytaride =0;
	m_PolytarideQ =0;
	m_Radioactives =0;
	m_RadioactivesQ =0;
	m_Spices =0;
	m_SpicesQ =0;
	m_Stone =0;
	m_StoneQ =0;
	m_Sunlight =0;
	m_SunlightQ =0;
	m_Vegetables =0;
	m_VegetablesQ =0;
	m_Vegetation =0;
	m_VegetationQ =0;
	m_Viathol =0;
	m_ViatholQ =0;
	m_Vulcanite =0;
	m_VulcaniteQ =0;
	m_WaterInEnvironment =0;
	m_WaterInEnvironmentQ =0;
	m_Wood =0;
	m_WoodQ =0;

	// Commodities that are not fetchable from the natural environment, hence no
	// quantity factor. The quality of these commodities is affected by the
	// environment.
	m_CheeseQ = 0;
	m_EggsQ = 0;
	m_FertilizerQ = 0;
	m_MilkQ = 0;

	m_AtmType = ATMTYPE_Standard;
	m_Zone = OZONE_Inferno;

	m_Radius100km = 0;
	m_StarType = 0;
}

void AuZoneResource::SetDark()
{
	m_Sunlight = 0;
}

//static
bool AuZoneResource::IsFetchable(COMMODITYID lCID)
{
	switch(lCID)
	{
	default:
		return false;
	case CID_Atmosphere:
	case CID_FrigidAtmosphere:
	case CID_Hydrographics:
	case CID_InfernoAtmosphere:
	case CID_Magma:
	case CID_Sunlight:
	case CID_SupercooledLiquid:
	case CID_Vegetation:
		return true;
	}
}

int AuZoneResource::Fetch(COMMODITYID lCID, int workersFetching) const
{
	if(!IsFetchable(lCID))
		return 0;
	if(!FetchProbability(lCID))
		return 0;
	// Success.
	if(workersFetching)
		return workersFetching;
	else
		return 1;
}

quint8 AuZoneResource::Quality(COMMODITYID lCID) const
{
	switch(lCID)
	{
	case CID_Adamantite:
		return m_AdamantiteQ;
	case CID_Air:
	case CID_PatentRefiningAirfromAtmosphere:
		return m_AirQ;
	case CID_Animal:
		return m_AnimalsQ;
	case CID_AntifluxParticles:
	case CID_PatentRefiningAntifluxParticlesfromAtmosphere:
		return m_AntifluxParticleQ;
	case CID_Atmosphere:
	case CID_FrigidAtmosphere:
	case CID_InfernoAtmosphere:
		// This is shared because these are mutually exclusive.
		return m_AtmosphereQ;
	case CID_Beans:
		return m_BeansQ;
	case CID_Bolite:
		return m_BoliteQ;
	case CID_BorexinoPrecipitate:
	case CID_PatentRefiningBorexinoPrecipitatefromAtmosphere:
		return m_BorexinoPrecipQ;
	case CID_Cheese:
		return m_CheeseQ;
	case CID_Coal:
		return m_CoalQ;
	case CID_CrudeOil:
		return m_CrudeOilQ;
	case CID_Cryozine:
	case CID_PatentRefiningCryozine:
		return m_CryozineQ;
	case CID_Crystals:
		return m_CrystalsQ;
	case CID_Eggs:
		return m_EggsQ;
	case CID_Eludium:
		return m_EludiumQ;
	case CID_Fertilizer:
		return m_FertilizerQ;
	case CID_Fiber:
		return m_FiberQ;
	case CID_Fish:
		return m_FishQ;
	case CID_Flomentum:
	case CID_PatentExtractingFlomentum:
		return m_FlomentumQ;
	case CID_Fruit:
		return m_FruitQ;
	case CID_Gems:
		return m_GemsQ;
	case CID_GoldOre:
		return m_GoldOreQ;
	case CID_Grain:
		return m_GrainQ;
	case CID_Grapes:
		return m_GrapesQ;
	case CID_Herbs:
		return m_HerbsQ;
	case CID_Hops:
		return m_HopsQ;
	case CID_Hydrogen:
	case CID_PatentRefiningHydrogenfromAtmosphere:
		return m_HydrogenQ;
	case CID_Ice:
		return m_IceQ;
	case CID_Ioplasma:
	case CID_PatentRefiningIoplasma:
		return m_IoplasmaQ;
	case CID_Lumenite:
		return m_LumeniteQ;
	case CID_Milk:
		return m_MilkQ;
	case CID_Magmex:
	case CID_PatentExtractingMagmex:
		return m_MagmexQ;
	case CID_Minerals:
		return m_MineralsQ;
	case CID_Myrathane:
	case CID_PatentExtractingMyrathane:
		return m_MyrathaneQ;
	case CID_NaturalGas:
		return m_NaturalGasQ;
	case CID_Nuts:
		return m_NutsQ;
	case CID_Ore:
		return m_OreQ;
	case CID_Phlogiston:
		return m_PhlogistonQ;
	case CID_Polytaride:
		return m_PolytarideQ;
	case CID_Radioactives:
		return m_RadioactivesQ;
	case CID_Spices:
		return m_SpicesQ;
	case CID_Stone:
		return m_StoneQ;
	case CID_Sunlight:
		return m_SunlightQ;
	case CID_Vegetable:
		return m_VegetablesQ;
	case CID_Hay:
	case CID_Vegetation:
		return m_VegetationQ;
	case CID_Viathol:
		return m_ViatholQ;
	case CID_Vulcanite:
		return m_VulcaniteQ;
	case CID_Hydrographics:
	case CID_Magma:
	case CID_SupercooledLiquid:
	case CID_Water:
		// This is shared because these are mutually exclusive.
		return m_WaterInEnvironmentQ;
	case CID_Wood:
		return m_WoodQ;

	default:
		break;
	}

	return 0;
}

// Randomly generate a rock resource to be present in a given terrain patch.
// Ice can be present only if 'iceFactor' is nonzero, which is a multiple of the
// water in the environment value; 50 produces a lot of ice.
COMMODITYID AuZoneResource::GenerateRockResource(const AuDice& Cdice, int iceFactor) const
{
	AuDice dice(Cdice);
	// Determine random distribution based upon overall quantity factor of commodities produced from rock.
	int rockcount = 0;
	rockcount += m_Bolite;
	rockcount += m_Coal;
	rockcount += m_CrudeOil;
	rockcount += m_Crystals;
	int eludium = m_Eludium * 40;	// Common when it's present.
	rockcount += eludium;
	rockcount += m_Gems;
	int ice = (m_WaterInEnvironment + 1) * iceFactor;	// +1 enables lunar ice
	rockcount += ice;
	rockcount += m_Lumenite;
	rockcount += m_Minerals;
	rockcount += m_Ore;
	rockcount += m_Phlogiston;
	rockcount += m_Polytaride;
	rockcount += m_Radioactives;
	rockcount += m_Stone * 40;		// Weight distribution toward stone.
	rockcount += m_Vulcanite;
	rockcount += m_Adamantite;
	rockcount += m_Viathol;
	rockcount += m_GoldOre;
	if(rockcount != 0)
	{
		int rock = dice.Roll(1,rockcount,-1);	// Linear: 0 <= rock < rockcount
		//qDebug("AuZoneResource::GenerateRockResource hasIce %c rockcount %d "
				//"rock %d", hasIce?'Y':'N', rockcount, rock);

		rockcount -= m_Bolite;
		if(rock >= rockcount)
		{
			//qDebug("Bolite");
			return CID_Bolite;
		}
		rockcount -= m_Coal;
		if(rock >= rockcount)
		{
			//qDebug("Coal");
			return CID_Coal;
		}
		rockcount -= m_CrudeOil;
		if(rock >= rockcount)
		{
			//qDebug("CrudeOil");
			return CID_CrudeOil;
		}
		rockcount -= m_Crystals;
		if(rock >= rockcount)
		{
			//qDebug("Crystals");
			return CID_Crystals;
		}
		rockcount -= eludium;
		if(rock >= rockcount)
		{
			//qDebug("Eludium");
			return CID_Eludium;
		}
		rockcount -= m_Gems;
		if(rock >= rockcount)
		{
			//qDebug("Gems");
			return CID_Gems;
		}
		rockcount -= ice;
		if(rock >= rockcount)
		{
			//qDebug("Ice");
			return CID_Ice;
		}
		rockcount -= m_Lumenite;
		if(rock >= rockcount)
		{
			//qDebug("Lumenite");
			return CID_Lumenite;
		}
		rockcount -= m_Minerals;
		if(rock >= rockcount)
		{
			//qDebug("Minerals");
			return CID_Minerals;
		}
		rockcount -= m_Ore;
		if(rock >= rockcount)
		{
			//qDebug("Ore");
			return CID_Ore;
		}
		rockcount -= m_Phlogiston;
		if(rock >= rockcount)
		{
			//qDebug("Phlogiston");
			return CID_Phlogiston;
		}
		rockcount -= m_Polytaride;
		if(rock >= rockcount)
		{
			//qDebug("Polytaride");
			return CID_Polytaride;
		}
		rockcount -= m_Radioactives;
		if(rock >= rockcount)
		{
			//qDebug("Radioactives");
			return CID_Radioactives;
		}
		rockcount -= m_Vulcanite;
		if(rock >= rockcount)
		{
			//qDebug("Vulcanite");
			return CID_Vulcanite;
		}
		rockcount -= m_Adamantite;
		if(rock >= rockcount)
		{
			//qDebug("Adamantite");
			return CID_Adamantite;
		}
		rockcount -= m_Viathol;
		if(rock >= rockcount)
		{
			//qDebug("Viathol");
			return CID_Viathol;
		}
		rockcount -= m_GoldOre;
		if(rock >= rockcount)
		{
			//qDebug("Gold Ore");
			return CID_GoldOre;
		}
	}
	return CID_Stone;
}

// Randomly generate a rock resource to be present in a given terrain patch.
// Ice can be present only if 'iceFactor' is nonzero, which is a multiple of the
// water in the environment value; 50 produces a lot of ice.
COMMODITYID AuZoneResource::GenerateRockResource(float noise, int iceFactor) const
{
	// Determine random distribution based upon overall quantity factor of commodities produced from rock.
	int rockcount = 0;
	rockcount += m_Bolite;
	rockcount += m_Coal;
	rockcount += m_CrudeOil;
	rockcount += m_Crystals;
	int eludium = m_Eludium * 40;	// Common when it's present.
	rockcount += eludium;
	rockcount += m_Gems;
	int ice = (m_WaterInEnvironment + 1) * iceFactor;	// +1 enables lunar ice
	rockcount += ice;
	rockcount += m_Lumenite;
	rockcount += m_Minerals;
	rockcount += m_Ore;
	rockcount += m_Phlogiston;
	rockcount += m_Polytaride;
	rockcount += m_Radioactives;
	rockcount += m_Stone * 40;		// Weight distribution toward stone.
	rockcount += m_Vulcanite;
	rockcount += m_Adamantite;
	rockcount += m_Viathol;
	rockcount += m_GoldOre;
	if(rockcount != 0)
	{
		int rock = (int)((noise + 1.f) * rockcount) % rockcount;
		//qDebug("AuZoneResource::GenerateRockResource hasIce %c rockcount %d "
				//"rock %d", hasIce?'Y':'N', rockcount, rock);

		rockcount -= m_Bolite;
		if(rock >= rockcount)
		{
			//qDebug("Bolite");
			return CID_Bolite;
		}
		rockcount -= m_Coal;
		if(rock >= rockcount)
		{
			//qDebug("Coal");
			return CID_Coal;
		}
		rockcount -= m_CrudeOil;
		if(rock >= rockcount)
		{
			//qDebug("CrudeOil");
			return CID_CrudeOil;
		}
		rockcount -= m_Crystals;
		if(rock >= rockcount)
		{
			//qDebug("Crystals");
			return CID_Crystals;
		}
		rockcount -= eludium;
		if(rock >= rockcount)
		{
			//qDebug("Eludium");
			return CID_Eludium;
		}
		rockcount -= m_Gems;
		if(rock >= rockcount)
		{
			//qDebug("Gems");
			return CID_Gems;
		}
		rockcount -= ice;
		if(rock >= rockcount)
		{
			//qDebug("Ice");
			return CID_Ice;
		}
		rockcount -= m_Lumenite;
		if(rock >= rockcount)
		{
			//qDebug("Lumenite");
			return CID_Lumenite;
		}
		rockcount -= m_Minerals;
		if(rock >= rockcount)
		{
			//qDebug("Minerals");
			return CID_Minerals;
		}
		rockcount -= m_Ore;
		if(rock >= rockcount)
		{
			//qDebug("Ore");
			return CID_Ore;
		}
		rockcount -= m_Phlogiston;
		if(rock >= rockcount)
		{
			//qDebug("Phlogiston");
			return CID_Phlogiston;
		}
		rockcount -= m_Polytaride;
		if(rock >= rockcount)
		{
			//qDebug("Polytaride");
			return CID_Polytaride;
		}
		rockcount -= m_Radioactives;
		if(rock >= rockcount)
		{
			//qDebug("Radioactives");
			return CID_Radioactives;
		}
		rockcount -= m_Vulcanite;
		if(rock >= rockcount)
		{
			//qDebug("Vulcanite");
			return CID_Vulcanite;
		}
		rockcount -= m_Adamantite;
		if(rock >= rockcount)
		{
			//qDebug("Adamantite");
			return CID_Adamantite;
		}
		rockcount -= m_Viathol;
		if(rock >= rockcount)
		{
			//qDebug("Viathol");
			return CID_Viathol;
		}
		rockcount -= m_GoldOre;
		if(rock >= rockcount)
		{
			//qDebug("Gold Ore");
			return CID_GoldOre;
		}
	}
	return CID_Stone;
}

// Randomly generate a shrub resource to be present in a given terrain patch.
COMMODITYID AuZoneResource::GenerateShrubResource(const AuDice& Cdice) const
{
	AuDice dice(Cdice);
	// Determine random distribution based upon overall quantity factor of
	// commodities produced from shrub.
	int shrubcount = 0;
	shrubcount += m_Beans;
	shrubcount += m_Fiber;
	shrubcount += m_Fruit;
	shrubcount += m_Grain;
	shrubcount += m_Grapes;
	shrubcount += m_Herbs;
	shrubcount += m_Hops;
	shrubcount += m_Nuts;
	shrubcount += m_Spices;
	shrubcount += m_Vegetables;
	shrubcount += m_Vegetation * 10;	// Weight distribution to vegetation.
	if(shrubcount != 0)
	{
		int shrub = dice.Roll(1,shrubcount,-1);	// Linear: 0 <= shrub < count

		shrubcount -= m_Beans;
		if(shrub >= shrubcount)
			return CID_Beans;
		shrubcount -= m_Fiber;
		if(shrub >= shrubcount)
			return CID_Fiber;
		shrubcount -= m_Fruit;
		if(shrub >= shrubcount)
			return CID_Fruit;
		shrubcount -= m_Grain;
		if(shrub >= shrubcount)
			return CID_Grain;
		shrubcount -= m_Grapes;
		if(shrub >= shrubcount)
			return CID_Grapes;
		shrubcount -= m_Herbs;
		if(shrub >= shrubcount)
			return CID_Herbs;
		shrubcount -= m_Hops;
		if(shrub >= shrubcount)
			return CID_Hops;
		shrubcount -= m_Nuts;
		if(shrub >= shrubcount)
			return CID_Nuts;
		shrubcount -= m_Spices;
		if(shrub >= shrubcount)
			return CID_Spices;
		shrubcount -= m_Vegetables;
		if(shrub >= shrubcount)
			return CID_Vegetable;
	}
	else
		qWarning() << "warning: AuZoneResource::GenerateShrubResource - dice shrubcount == 0";
	return CID_Hay;
}

// Randomly generate a shrub resource to be present in a given terrain patch.
COMMODITYID AuZoneResource::GenerateShrubResource(float noise) const
{
	// Determine random distribution based upon overall quantity factor of
	// commodities produced from shrub.
	int shrubcount = 0;
	shrubcount += m_Beans;
	shrubcount += m_Fiber;
	shrubcount += m_Fruit;
	shrubcount += m_Grain;
	shrubcount += m_Grapes;
	shrubcount += m_Herbs;
	shrubcount += m_Hops;
	shrubcount += m_Nuts;
	shrubcount += m_Spices;
	shrubcount += m_Vegetables;
	shrubcount += m_Vegetation * 10;	// Weight distribution to vegetation.
	if(shrubcount != 0)
	{
		int shrub = (int)((noise + 1.f) * shrubcount) % shrubcount;

		shrubcount -= m_Beans;
		if(shrub >= shrubcount)
			return CID_Beans;
		shrubcount -= m_Fiber;
		if(shrub >= shrubcount)
			return CID_Fiber;
		shrubcount -= m_Fruit;
		if(shrub >= shrubcount)
			return CID_Fruit;
		shrubcount -= m_Grain;
		if(shrub >= shrubcount)
			return CID_Grain;
		shrubcount -= m_Grapes;
		if(shrub >= shrubcount)
			return CID_Grapes;
		shrubcount -= m_Herbs;
		if(shrub >= shrubcount)
			return CID_Herbs;
		shrubcount -= m_Hops;
		if(shrub >= shrubcount)
			return CID_Hops;
		shrubcount -= m_Nuts;
		if(shrub >= shrubcount)
			return CID_Nuts;
		shrubcount -= m_Spices;
		if(shrub >= shrubcount)
			return CID_Spices;
		shrubcount -= m_Vegetables;
		if(shrub >= shrubcount)
			return CID_Vegetable;
	}
	else
		qWarning() << "warning: AuZoneResource::GenerateShrubResource - noise shrubcount == 0";
	return CID_Hay;
}

// Randomly generate a tree resource to be present in a given terrain patch.
COMMODITYID AuZoneResource::GenerateTreeResource(const AuDice& Cdice) const
{
	AuDice dice(Cdice);
	// Determine random distribution based upon overall quantity factor of
	// commodities produced from trees.
	int treecount = 0;
	treecount += m_Fiber;
	treecount += m_Fruit;
	treecount += m_Herbs;
	treecount += m_Nuts;
	treecount += m_Spices;
	treecount += m_Wood * 40;	// Weight distribution toward wood.
	if(treecount != 0)
	{
		int tree = dice.Roll(1,treecount,-1);	// Linear: 0 <= tree < treecount

		treecount -= m_Fiber;
		if(tree >= treecount)
			return CID_Fiber;
		treecount -= m_Fruit;
		if(tree >= treecount)
			return CID_Fruit;
		treecount -= m_Herbs;
		if(tree >= treecount)
			return CID_Herbs;
		treecount -= m_Nuts;
		if(tree >= treecount)
			return CID_Nuts;
		treecount -= m_Spices;
		if(tree >= treecount)
			return CID_Spices;
	}
	return CID_Wood;
}

// Randomly generate a tree resource to be present in a given terrain patch.
COMMODITYID AuZoneResource::GenerateTreeResource(float noise) const
{
	// Determine random distribution based upon overall quantity factor of
	// commodities produced from trees.
	int treecount = 0;
	treecount += m_Fiber;
	treecount += m_Fruit;
	treecount += m_Herbs;
	treecount += m_Nuts;
	treecount += m_Spices;
	treecount += m_Wood * 40;	// Weight distribution toward wood.
	if(treecount != 0)
	{
		int tree = (int)((noise + 1.f) * treecount) % treecount;

		treecount -= m_Fiber;
		if(tree >= treecount)
			return CID_Fiber;
		treecount -= m_Fruit;
		if(tree >= treecount)
			return CID_Fruit;
		treecount -= m_Herbs;
		if(tree >= treecount)
			return CID_Herbs;
		treecount -= m_Nuts;
		if(tree >= treecount)
			return CID_Nuts;
		treecount -= m_Spices;
		if(tree >= treecount)
			return CID_Spices;
	}
	return CID_Wood;
}

// Probability of finding a resource in the environment, whether or not it is fetchable..
int AuZoneResource::FetchProbability(COMMODITYID lCID) const
{
	// Atmosphere Resources
	if(lCID == CID_Air || lCID == CID_Hydrogen)
		return m_Atmosphere;
	else if(lCID == CID_Cryozine || lCID == CID_FrigidAtmosphere)
	{
		if(m_Zone == OZONE_Frigid || IsGasGiant())
			return (m_Atmosphere * m_AtmType) / 75;
		return 0;
	}
	else if(lCID == CID_Ioplasma || lCID == CID_InfernoAtmosphere)
	{
		if(m_Zone == OZONE_Inferno && !IsGasGiant())
			return (m_Atmosphere * m_AtmType) / 75;
		return 0;
	}

	// Mineral Resources
	int iceFactor;
	if(IsGasGiant())
		iceFactor = 0;			// Gas giants do not have ice.
	else
		iceFactor = m_Zone;		// Ice prevalence is affected by orbit zone.

	// Determine random distribution based upon overall quantity factor of commodities produced from rock.
	int rockcount = 0;
	rockcount += m_Bolite;
	rockcount += m_Coal;
	rockcount += m_CrudeOil;
	rockcount += m_Crystals;
	int eludium = m_Eludium * 40;	// Common when it's present.
	rockcount += eludium;
	rockcount += m_Gems;
	int ice = (m_WaterInEnvironment + 1) * iceFactor;	// +1 enables lunar ice
	rockcount += ice;
	rockcount += m_Lumenite;
	rockcount += m_Minerals;
	rockcount += m_Ore;
	rockcount += m_Phlogiston;
	rockcount += m_Polytaride;
	rockcount += m_Radioactives;
	rockcount += m_Stone * 40;		// Weight distribution toward stone.
	rockcount += m_Vulcanite;
	rockcount += m_Adamantite;
	rockcount += m_Viathol;
	rockcount += m_GoldOre;
	switch(lCID)
	{
	case CID_Adamantite:
		if(m_Adamantite)
		{
			int prob = m_Adamantite * 100 / rockcount;
			if(prob)
				return prob;
			return 1;			// We know it's here. Show a minimal probability.
		}
		break;
	case CID_Bolite:
		if(m_Bolite)
		{
			int prob = m_Bolite * 100 / rockcount;
			if(prob)
				return prob;
			return 1;			// We know it's here. Show a minimal probability.
		}
		break;
	case CID_Coal:
		if(m_Coal)
		{
			int prob = m_Coal * 100 / rockcount;
			if(prob)
				return prob;
			return 1;			// We know it's here. Show a minimal probability.
		}
		break;
	case CID_CrudeOil:
	case CID_NaturalGas:
		if(m_CrudeOil)
		{
			int prob = m_CrudeOil * 100 / rockcount;
			if(prob)
				return prob;
			return 1;			// We know it's here. Show a minimal probability.
		}
		break;
	case CID_Crystals:
		if(m_Crystals)
		{
			int prob = m_Crystals * 100 / rockcount;
			if(prob)
				return prob;
			return 1;			// We know it's here. Show a minimal probability.
		}
		break;
	case CID_Eludium:
		if(eludium)
		{
			int prob = eludium * 100 / rockcount;
			if(prob)
				return prob;
			return 1;			// We know it's here. Show a minimal probability.
		}
		break;
	case CID_Gems:
		if(m_Gems)
		{
			int prob = m_Gems * 100 / rockcount;
			if(prob)
				return prob;
			return 1;			// We know it's here. Show a minimal probability.
		}
		break;
	case CID_GoldOre:
		if(m_GoldOre)
		{
			int prob = m_GoldOre * 100 / rockcount;
			if(prob)
				return prob;
			return 1;			// We know it's here. Show a minimal probability.
		}
		break;
	case CID_Ice:
		if(ice)
		{
			int prob = ice * 100 / rockcount;
			if(prob)
				return prob;
			return 1;			// We know it's here. Show a minimal probability.
		}
		break;
	case CID_Lumenite:
		if(m_Lumenite)
		{
			int prob = m_Lumenite * 100 / rockcount;
			if(prob)
				return prob;
			return 1;			// We know it's here. Show a minimal probability.
		}
		break;
	case CID_Minerals:
		if(m_Minerals)
		{
			int prob = m_Minerals * 100 / rockcount;
			if(prob)
				return prob;
			return 1;			// We know it's here. Show a minimal probability.
		}
		break;
	case CID_Ore:
		if(m_Ore)
		{
			int prob = m_Ore * 100 / rockcount;
			if(prob)
				return prob;
			return 1;			// We know it's here. Show a minimal probability.
		}
		break;
	case CID_Phlogiston:
		if(m_Phlogiston)
		{
			int prob = m_Phlogiston * 100 / rockcount;
			if(prob)
				return prob;
			return 1;			// We know it's here. Show a minimal probability.
		}
		break;
	case CID_Polytaride:
		if(m_Polytaride)
		{
			int prob = m_Polytaride * 100 / rockcount;
			if(prob)
				return prob;
			return 1;			// We know it's here. Show a minimal probability.
		}
		break;
	case CID_Radioactives:
		if(m_Radioactives)
		{
			int prob = m_Radioactives * 100 / rockcount;
			if(prob)
				return prob;
			return 1;			// We know it's here. Show a minimal probability.
		}
		break;
	case CID_Stone:
		if(m_Stone)
		{
			int prob = m_Stone * 100 / rockcount;
			if(prob)
				return prob;
			return 1;			// We know it's here. Show a minimal probability.
		}
		break;
	case CID_Viathol:
		if(m_Viathol)
		{
			int prob = m_Viathol * 100 / rockcount;
			if(prob)
				return prob;
			return 1;			// We know it's here. Show a minimal probability.
		}
		break;
	case CID_Vulcanite:
		if(m_Vulcanite)
		{
			int prob = m_Vulcanite * 100 / rockcount;
			if(prob)
				return prob;
			return 1;			// We know it's here. Show a minimal probability.
		}
		break;

	case CID_Magma:
	case CID_Magmex:
		if(m_Zone == OZONE_Inferno && !IsGasGiant())
			return m_WaterInEnvironment;
		break;

	case CID_Flomentum:
		return m_Flomentum;

	case CID_Myrathane:
	case CID_SupercooledLiquid:
		if(m_Zone == OZONE_Frigid || IsGasGiant())
			return m_WaterInEnvironment;
		break;

	case CID_AntifluxParticles:
		if(m_Radius100km < 10)
			return ((OZONE_Frigid - m_Zone) + 1) * 2;
		break;
	case CID_BorexinoPrecipitate:
		if(m_Radius100km >= 200 && m_Radius100km <= 400)			// Bodies from size 20 to 40 are considered Gas Giants.
			return (7 - m_StarType) * (m_Radius100km / 100 - 1);
		break;

	case CID_Atmosphere:
		return m_Atmosphere;

	case CID_Hydrographics:
	case CID_Water:
		if(m_Zone != OZONE_Frigid && m_Zone != OZONE_Inferno && !IsGasGiant())
			return m_WaterInEnvironment;
		break;

	case CID_Vegetation:
		return m_Vegetation;
	case CID_Sunlight:
		return m_Sunlight;
	default:
		break;
	}
	return 0;
}

// Returns true if 'cid' is derived from natural rock formations by mines or wells.
//static
bool AuZoneResource::IsMineral(COMMODITYID cid)
{
	switch(cid)
	{
	case CID_Adamantite:
	case CID_Bolite:
	case CID_Coal:
	case CID_CrudeOil:
	case CID_Crystals:
	case CID_Eludium:
	case CID_Gems:
	case CID_GoldOre:
	case CID_Ice:
	case CID_Lumenite:
	case CID_Minerals:
	case CID_NaturalGas:
	case CID_Ore:
	case CID_Phlogiston:
	case CID_Polytaride:
	case CID_Radioactives:
	case CID_Stone:
	case CID_Viathol:
	case CID_Vulcanite:
		return true;
	default:
		break;
	}
	return false;
}

COMMODITYID AuZoneResource::GuaranteedRockAt(const AuGlobeLLF& rockLL) const
{
	if(m_Adamantite && rockLL.FuzzyCompareIgnoreSign(m_AdamantiteLL))
		return CID_Adamantite;
	if(m_Bolite && rockLL.FuzzyCompareIgnoreSign(m_BoliteLL))
		return CID_Bolite;
	if(m_Coal && rockLL.FuzzyCompareIgnoreSign(m_CoalLL))
		return CID_Coal;
	if(m_CrudeOil && rockLL.FuzzyCompareIgnoreSign(m_CrudeOilLL))
		return CID_CrudeOil;
	if(m_Crystals && rockLL.FuzzyCompareIgnoreSign(m_CrystalsLL))
		return CID_Crystals;
	if(m_Eludium && rockLL.FuzzyCompareIgnoreSign(m_EludiumLL))
		return CID_Eludium;
	if(m_Gems && rockLL.FuzzyCompareIgnoreSign(m_GemsLL))
		return CID_Gems;
	if(m_GoldOre && rockLL.FuzzyCompareIgnoreSign(m_GoldOreLL))
		return CID_GoldOre;
	if(m_Ice && rockLL.FuzzyCompareIgnoreSign(m_IceLL))
		return CID_Ice;
	if(m_Lumenite && rockLL.FuzzyCompareIgnoreSign(m_LumeniteLL))
		return CID_Lumenite;
	if(m_Minerals && rockLL.FuzzyCompareIgnoreSign(m_MineralsLL))
		return CID_Minerals;
	if(m_Ore && rockLL.FuzzyCompareIgnoreSign(m_OreLL))
		return CID_Ore;
	if(m_Phlogiston && rockLL.FuzzyCompareIgnoreSign(m_PhlogistonLL))
		return CID_Phlogiston;
	if(m_Polytaride && rockLL.FuzzyCompareIgnoreSign(m_PolytarideLL))
		return CID_Polytaride;
	if(m_Radioactives && rockLL.FuzzyCompareIgnoreSign(m_RadioactivesLL))
		return CID_Radioactives;
	if(m_Viathol && rockLL.FuzzyCompareIgnoreSign(m_ViatholLL))
		return CID_Viathol;
	if(m_Vulcanite && rockLL.FuzzyCompareIgnoreSign(m_VulcaniteLL))
		return CID_Vulcanite;
	return CID_Invalid;
}

