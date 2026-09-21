//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auzoneresource.h,v 1.32 2022/02/19 18:19:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuZoneResource_H
#define AuZoneResource_H

#include "audata.h"
#include "audef.h"
#include "auglobell.h"
#include "dbcommodity.h"

class AuDice;

// This class represents objects that can be acquired directly from the environment in a particular resource zone of a planet.
class AUDATA_DLL AuZoneResource
{
public:
	unsigned char		m_Adamantite;
	AuGlobeLLF			m_AdamantiteLL;							// Guaranteed valid location if amount is non zero.
	unsigned char		m_AdamantiteQ;
	unsigned char		m_AirQ;									// Quantity based on atmosphere density.
	unsigned char		m_Animals;								// Quantity factor. (0-100)
	unsigned char		m_AnimalsQ;								// Quality factor. (0-255)
	unsigned char		m_AntifluxParticleQ;
	unsigned char		m_Atmosphere;
	unsigned char		m_AtmosphereQ;
	unsigned char		m_Beans;
	unsigned char		m_BeansQ;
	unsigned char		m_Bolite;
	AuGlobeLLF			m_BoliteLL;								// Guaranteed valid location if amount is non zero.
	unsigned char		m_BoliteQ;
	unsigned char		m_BorexinoPrecipQ;
	unsigned char		m_Coal;
	AuGlobeLLF			m_CoalLL;								// Guaranteed valid location if amount is non zero.
	unsigned char		m_CoalQ;
	unsigned char		m_CrudeOil;
	AuGlobeLLF			m_CrudeOilLL;							// Guaranteed valid location if amount is non zero.
	unsigned char		m_CrudeOilQ;
	unsigned char		m_CryozineQ;							// Quantity based on atmosphere density.
	unsigned char		m_Crystals;
	AuGlobeLLF			m_CrystalsLL;							// Guaranteed valid location if amount is non zero.
	unsigned char		m_CrystalsQ;
	unsigned char		m_Eludium;
	AuGlobeLLF			m_EludiumLL;							// Guaranteed valid location if amount is non zero.
	unsigned char		m_EludiumQ;
	unsigned char		m_Fiber;
	unsigned char		m_FiberQ;
	unsigned char		m_Fish;
	unsigned char		m_FishQ;
	unsigned char		m_Flomentum;
	unsigned char		m_FlomentumQ;
	unsigned char		m_Fruit;
	unsigned char		m_FruitQ;
	unsigned char		m_Gems;
	AuGlobeLLF			m_GemsLL;								// Guaranteed valid location if amount is non zero.
	unsigned char		m_GemsQ;
	unsigned char		m_GoldOre;
	AuGlobeLLF			m_GoldOreLL;							// Guaranteed valid location if amount is non zero.
	unsigned char		m_GoldOreQ;
	unsigned char		m_Grain;
	unsigned char		m_GrainQ;
	unsigned char		m_Grapes;
	unsigned char		m_GrapesQ;
	unsigned char		m_Herbs;
	unsigned char		m_HerbsQ;
	unsigned char		m_Hops;
	unsigned char		m_HopsQ;
	unsigned char		m_HydrogenQ;							// Quantity based on atmosphere density.
	unsigned char		m_Ice;
	AuGlobeLLF			m_IceLL;								// Guaranteed valid location if amount is non zero.
	unsigned char		m_IceQ;
	unsigned char		m_IoplasmaQ;
	unsigned char		m_Lumenite;
	AuGlobeLLF			m_LumeniteLL;							// Guaranteed valid location if amount is non zero.
	unsigned char		m_LumeniteQ;
	unsigned char		m_MagmexQ;								// Quantity based on hydrographics.
	unsigned char		m_Minerals;
	AuGlobeLLF			m_MineralsLL;							// Guaranteed valid location if amount is non zero.
	unsigned char		m_MineralsQ;
	unsigned char		m_MyrathaneQ;							// Quantity based on hydrographics.
	unsigned char		m_NaturalGas;							// Guaranteed location is at m_CrudeOilLL
	unsigned char		m_NaturalGasQ;
	unsigned char		m_Nuts;
	unsigned char		m_NutsQ;
	unsigned char		m_Ore;
	AuGlobeLLF			m_OreLL;								// Guaranteed valid location if amount is non zero.
	unsigned char		m_OreQ;
	unsigned char		m_Phlogiston;
	AuGlobeLLF			m_PhlogistonLL;							// Guaranteed valid location if amount is non zero.
	unsigned char		m_PhlogistonQ;
	unsigned char		m_Polytaride;
	AuGlobeLLF			m_PolytarideLL;							// Guaranteed valid location if amount is non zero.
	unsigned char		m_PolytarideQ;
	unsigned char		m_Radioactives;
	AuGlobeLLF			m_RadioactivesLL;						// Guaranteed valid location if amount is non zero.
	unsigned char		m_RadioactivesQ;
	unsigned char		m_Spices;
	unsigned char		m_SpicesQ;
	unsigned char		m_Stone;
	unsigned char		m_StoneQ;
	unsigned char		m_Sunlight;
	unsigned char		m_SunlightQ;
	unsigned char		m_Vegetables;
	unsigned char		m_VegetablesQ;
	unsigned char		m_Vegetation;
	unsigned char		m_VegetationQ;
	unsigned char		m_Viathol;
	AuGlobeLLF			m_ViatholLL;							// Guaranteed valid location if amount is non zero.
	unsigned char		m_ViatholQ;
	unsigned char		m_Vulcanite;
	AuGlobeLLF			m_VulcaniteLL;							// Guaranteed valid location if amount is non zero.
	unsigned char		m_VulcaniteQ;
	unsigned char		m_WaterInEnvironment;
	unsigned char		m_WaterInEnvironmentQ;					// also magma, supercooled liquid
	unsigned char		m_Wood;
	unsigned char		m_WoodQ;

	// Commodities that are not fetchable from the natural environment, hence no quantity factor.
	// The quality of these commodities is affected by the environment.
	unsigned char		m_CheeseQ;
	unsigned char		m_EggsQ;
	unsigned char		m_FertilizerQ;
	unsigned char		m_MilkQ;

	// Planet Info
	ATMTYPE				m_AtmType;
	ORBITZONE			m_Zone;
	quint16				m_Radius100km;							// Radius in 100s of km.

	// Star Info
	unsigned char		m_StarType;								// DaStar::TYPE - 0..6 = OBAFGKM

public:
	AuZoneResource();

	// Returns a quantity of the resource fetched from the environment.
	// 'howMany' is the number of workers fetching the material.
	// Returns 0 if IsFetchable returns false.
	int Fetch(COMMODITYID lCID, int workersFetching) const;

	// Probability of finding a resource in the resource zone, whether or not it is fetchable.
	int FetchProbability(COMMODITYID lCID) const;

	COMMODITYID GenerateRockResource(const AuDice& dice, int iceFactor) const;

	// 'noise' is a coherent noise factor in the range of -1 to 1.
	COMMODITYID GenerateRockResource(float noise, int iceFactor) const;

	COMMODITYID GenerateShrubResource(const AuDice& dice) const;

	// 'noise' is a coherent noise factor in the range of -1 to 1.
	COMMODITYID GenerateShrubResource(float noise) const;

	COMMODITYID GenerateTreeResource(const AuDice& dice) const;

	// 'noise' is a coherent noise factor in the range of -1 to 1.
	COMMODITYID GenerateTreeResource(float noise) const;

	// Returns the commodity id of any guaranteed rock resource at 'rockLL'.
	COMMODITYID GuaranteedRockAt(const AuGlobeLLF& rockLL) const;

	static bool IsFetchable(COMMODITYID lCID);

	inline bool IsGasGiant() const
	{
		return m_Radius100km >= 200 && m_Radius100km <= 400;
	}

	// Returns true if 'cid' is derived from natural rock formations by mines or wells.
	static bool IsMineral(COMMODITYID cid);

	inline bool IsRingworld() const
	{
		return m_Radius100km == 500;
	}

	// Returns the quality of a natural resource.
	quint8 Quality(COMMODITYID lCID) const;

	void SetDark();
};

#endif // AuZoneResource_H
