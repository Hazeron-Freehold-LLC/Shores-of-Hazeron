//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: defauna.cpp,v 1.40 2022/02/19 17:44:11 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// DePlanetFauna - Describes the various creatures found on a habitable planet.
// DeTerrainFauna - Describes the various creatures found in a terrain type.
//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

#include "defauna.h"
//
#include "auglobal.h"

DeFaunaEncounter::DeFaunaEncounter()
{
	m_GroupSize = 0;
}

DeFaunaEncounter::~DeFaunaEncounter()
{
}

void DeFaunaEncounter::operator>>(QDataStream& out) const
{
	m_DNA >> out;
	out << (quint8)m_GroupSize;
}

void DeFaunaEncounter::operator<<(QDataStream& in)
{
	m_DNA << in;
	quint8 ui8;
	in >> ui8;
	m_GroupSize = ui8;
}

void DeFaunaEncounter::Init(bool isRingworld, bool isGasGiant, quint16 radius100km, quint8 waterPercent, ORBITZONE orbitZone,
		quint8 atmDensity, TERRAIN tertype, TOD tod, ECOROLE role, const AuDice& Cdice)
{
	AuDice dice(Cdice);
	// Randomize the DNA.
	int sizemod;
	if(isRingworld)
		sizemod = 0;					// Ringworlds are considered to have standard gravity, which means no size modifier.
	else if(isGasGiant)
		sizemod = 2;
	else
		sizemod = 8 - radius100km/10;	// Creature size increases as planet gets smaller.

	// Let's have larger creatures in the inner and outer zones, just for fun.
	if(isGasGiant)
		sizemod += 8;
	else if(orbitZone == OZONE_Inner)
		sizemod += 4;
	else if(orbitZone == OZONE_Outer)
		sizemod += 2;

	m_DNA.Randomize(sizemod, (atmDensity>=50)?true:false, tod, dice, true);
	dice.Random();

	// Override some of the random results to fit the specifications of this encounter.
	m_DNA.EcologicalRole = role;
	if(tertype == Underwater || isGasGiant)
	{
		// Terrain is out at sea.
		m_DNA.LivesOnLand = 0;						// No chance for land dwellers here.
		m_DNA.LivesInWater = 1;						// Terrain requires sea dweller.
	}
	else if(tertype <= LowlandDesert)
	{
		// Terrain is above sea level.
		// Our terrain model doesn't create significant bodies of water at this altitude
		// so there won't be any significant amphibians.
		m_DNA.LivesOnLand = 1;						// Terrain requires land dweller.
		m_DNA.LivesInWater = 0;						// No chance for amphibians here.
	}
	else
	{
		// Terrain is at sea level.
		m_DNA.LivesOnLand = 1;						// Sea level terrain requires not
		// strictly sea dweller.
		if(waterPercent < 10)
			m_DNA.LivesInWater = 0;					// No chance for amphibians here.
		else
			m_DNA.LivesInWater = dice.Roll(1,2,-1);	// 0..1
	}

	// Group-size modifiers by ecological role.
	// Base roll is 2D3 (2-6)
	// Min is one after applying modifier.
	int groupmods[4] =
	{
		-3,	// Carnivore = 1-3 individuals per group
		0,	// Herbivore = 2-6 individuals per group
		-2,	// Omnivore = 1-4 individuals per group
		-4,	// Scavenger = 1-2 individuals per group
	};

	m_GroupSize = dice.Roll(2, 3, groupmods[role]);	// 2D3 + groupmod by role
	if(m_GroupSize < 1)
		m_GroupSize = 1;
}

// Used when generating a random animal encounter.
void DeFaunaEncounter::NewGroupSize()
{
	if(m_GroupSize > 1 && m_GroupSize <= 6)
		m_GroupSize = g_Au.Dice().Roll(1, m_GroupSize);
	else
		m_GroupSize = 1;	// Insurance against bogus data.
}

void DeFaunaEncounter::TxUpdate(QDataStream& toClient) const
{
	m_DNA >> toClient;
	toClient << (quint8)m_GroupSize;
}

void DeFaunaEncounter::RxUpdate(QDataStream& fromServer)
{
	m_DNA << fromServer;
	quint8 ui8;
	fromServer >> ui8;
	m_GroupSize = ui8;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

DeTerrainFauna::DeTerrainFauna()
{
}

DeTerrainFauna::~DeTerrainFauna()
{
}

void DeTerrainFauna::operator>>(QDataStream& out) const
{
	for(int e=0; e<7; e++)
		out << m_Encounters[e];
}

void DeTerrainFauna::operator<<(QDataStream& in)
{
	for(int e=0; e<7; e++)
		in >> m_Encounters[e];
}

void DeTerrainFauna::Init(bool isRingworld, bool isGasGiant, quint16 radius100km, quint8 waterPercent, ORBITZONE orbitZone,
		quint8 atmDensity, TERRAIN tertype, TOD tod, const AuDice& Cdice)
{
	AuDice dice(Cdice);
	// This 2D4-1 table gives us the following distribution of encounter probabilities:
	// Carnivore = 3:16
	// Herbivore = 6:16
	// Omnivore  = 4:16
	// Scavenger = 3:16
	ECOROLE ecoroles[7] =
	{
		ECO_Carnivore,	// 1:16
		ECO_Scavenger,	// 2:16
		ECO_Herbivore,	// 3:16
		ECO_Omnivore,	// 4:16
		ECO_Herbivore,	// 3:16
		ECO_Carnivore,	// 2:16
		ECO_Scavenger,	// 1:16
	};
	
	// Create a DNA and max group size value for each of our encounter table positions.
	// The positions correspond to a type of critter that is determed by rolling 2D4-1.
	// The result gives us a bell curve distribution of encounter probabilities that
	// are distributed in the 'ecoroles' table defined above.
	for(int tpos=0; tpos<7; tpos++)
	{
		ECOROLE ecorole = ecoroles[tpos];
		m_Encounters[tpos].Init(isRingworld, isGasGiant, radius100km, waterPercent, orbitZone, atmDensity, tertype, tod, ecorole,
				dice);
		dice.Random();
	}
}

bool DeTerrainFauna::HasAerialCreatures() const
{
	for(int tpos=0; tpos<7; tpos++)
		if(DhDNA(m_Encounters[tpos].DNA()).CanFly())
			return true;										// Found an aerial creature DNA.
	return false;
}

DeFaunaEncounter DeTerrainFauna::NewAerialEncounter() const
{
	// Make a list of the encounter table indices that can fly.
	QList<int> flyers;
	for(int tpos=0; tpos<7; tpos++)
		if(DhDNA(m_Encounters[tpos].DNA()).CanFly())
			flyers.append(tpos);

	if(flyers.isEmpty())
		return DeFaunaEncounter();

	// Roll to determine what was encountered.
	int encwhat = g_Au.Dice().Roll(1,flyers.count(),-1);
	DeFaunaEncounter enc = m_Encounters[flyers[encwhat]];
	enc.NewGroupSize();
	return enc;
}

DeFaunaEncounter DeTerrainFauna::NewEncounter() const
{
	// Roll to determine what was encountered.
	int encwhat = g_Au.Dice().Roll(2,4,-1);
	DeFaunaEncounter enc = m_Encounters[encwhat];
	enc.NewGroupSize();
	return enc;
}

DeFaunaEncounter DeTerrainFauna::NewFarmAnimal() const
{
	// Roll to determine what was encountered.
	int encwhat = g_Au.Dice().Roll(1,2,-1);
	DeFaunaEncounter enc;
	if(encwhat)
		enc = m_Encounters[2];	// First herbivore is at index 2.
	else
		enc = m_Encounters[4];	// Second herbivore is at index 4.
	enc.NewGroupSize();			// Farm animal encounters really don't use group size.
	return enc;
}

void DeTerrainFauna::TxUpdate(QDataStream& toClient) const
{
	for(int tpos=0; tpos<7; tpos++)
		m_Encounters[tpos].TxUpdate(toClient);
}

void DeTerrainFauna::RxUpdate(QDataStream& fromServer)
{
	for(int tpos=0; tpos<7; tpos++)
		m_Encounters[tpos].RxUpdate(fromServer);
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

DePlanetFauna::DePlanetFauna()
{
}

DePlanetFauna::~DePlanetFauna()
{
}

void DePlanetFauna::operator>>(QDataStream& out) const
{
	for(int tertype=0; tertype<TerrainTypes; tertype++)
		for(int timeofday=0; timeofday<TOD_Any; timeofday++)	// Dark, Dim, and Bright
			out << m_Fauna[tertype][timeofday];
}

void DePlanetFauna::operator<<(QDataStream& in)
{
	for(int tertype=0; tertype<TerrainTypes; tertype++)
		for(int timeofday=0; timeofday<TOD_Any; timeofday++)	// Dark, Dim, and Bright
			in >> m_Fauna[tertype][timeofday];
}

void DePlanetFauna::InitGasGiant(ORBITZONE orbitZone, quint8 atmDensity, const AuDice& Cdice)
{
	AuDice dice(Cdice);
	for(int tertype=0; tertype<TerrainTypes; tertype++)
		for(int timeofday=0; timeofday<TOD_Any; timeofday++)	// Dark, Dim, and Bright
		{
			m_Fauna[tertype][timeofday].Init(false, true, 0, 0, orbitZone, atmDensity, (TERRAIN)tertype, (TOD)timeofday, dice);
			dice.Random();
		}
}

void DePlanetFauna::InitGlobe(ORBITZONE orbitZone, quint8 atmDensity, quint8 waterPercent, quint16 radius100km, const AuDice& Cdice)
{
	AuDice dice(Cdice);
	for(int tertype=0; tertype<TerrainTypes; tertype++)
		for(int timeofday=0; timeofday<TOD_Any; timeofday++)	// Dark, Dim, and Bright
		{
			m_Fauna[tertype][timeofday].Init(false, false, radius100km, waterPercent, orbitZone, atmDensity, (TERRAIN)tertype,
					(TOD)timeofday, dice);
			dice.Random();
		}
}

void DePlanetFauna::InitRingworld(ORBITZONE orbitZone, quint8 atmDensity, quint8 waterPercent, const AuDice& Cdice)
{
	AuDice dice(Cdice);
	for(int tertype=0; tertype<TerrainTypes; tertype++)
		for(int timeofday=0; timeofday<TOD_Any; timeofday++)	// Dark, Dim, and Bright
		{
			m_Fauna[tertype][timeofday].Init(true, false, 0, waterPercent, orbitZone, atmDensity, (TERRAIN)tertype, (TOD)timeofday,
					dice);
			dice.Random();
		}
}

bool DePlanetFauna::HasAerialCreatures(float sunlight, TERRAIN terrainType) const
{
	// Establish time of day, which is literally "amount of daylight".
	TOD tod;
	if(sunlight < 0.04f)
		tod = TOD_Nocturnal;
	else if(sunlight < 0.4f)
		tod = TOD_Crepuscular;
	else
		tod = TOD_Diurnal;

	return m_Fauna[terrainType][tod].HasAerialCreatures();
}

DeFaunaEncounter DePlanetFauna::NewAerialEncounter(float sunlight, TERRAIN terrainType) const
{
	// Establish time of day, which is literally "amount of daylight".
	TOD tod;
	if(sunlight < 0.04f)
		tod = TOD_Nocturnal;
	else if(sunlight < 0.4f)
		tod = TOD_Crepuscular;
	else
		tod = TOD_Diurnal;

	DeFaunaEncounter enc = m_Fauna[terrainType][tod].NewAerialEncounter();
	return enc;
}

DeFaunaEncounter DePlanetFauna::NewEncounter(float sunlight, TERRAIN terrainType) const
{
	// Establish time of day, which is literally "amount of daylight".
	TOD tod;
	if(sunlight < 0.04f)
		tod = TOD_Nocturnal;
	else if(sunlight < 0.4f)
		tod = TOD_Crepuscular;
	else
		tod = TOD_Diurnal;

	DeFaunaEncounter enc = m_Fauna[terrainType][tod].NewEncounter();
	return enc;
}

DeFaunaEncounter DePlanetFauna::NewFarmAnimal(float sunlight, TERRAIN terrainType) const
{
	// Establish time of day, which is literally "amount of daylight".
	TOD tod;
	if(sunlight < 0.04f)
		tod = TOD_Nocturnal;
	else if(sunlight < 0.4f)
		tod = TOD_Crepuscular;
	else
		tod = TOD_Diurnal;

	DeFaunaEncounter enc = m_Fauna[terrainType][tod].NewFarmAnimal();
	return enc;
}

void DePlanetFauna::TxUpdate(QDataStream& toClient) const
{
	for(int iter=0; iter<TerrainTypes; iter++)
		for(int tod=0; tod<TOD_Any; tod++)	// Dark, Dim, and Bright
			m_Fauna[iter][tod].TxUpdate(toClient);
}

void DePlanetFauna::RxUpdate(QDataStream& fromServer)
{
	for(int iter=0; iter<TerrainTypes; iter++)
		for(int tod=0; tod<TOD_Any; tod++)	// Dark, Dim, and Bright
			m_Fauna[iter][tod].RxUpdate(fromServer);
}

