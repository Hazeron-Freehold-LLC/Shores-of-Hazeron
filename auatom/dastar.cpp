//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dastar.cpp,v 1.585 2022/05/17 16:43:32 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dastar.h"
//
#include "auatomrenderinfo.h"
#include "aucombat.h"
#include "audice.h"
#include "augleffects.h"
#include "auglobal.h"
#include "aunoise.h"
#include "auscdesign.h"
#include "ausensorblip.h"
#include "daasteroid.h"
#include "daatomlist.h"
#include "dadesigner.h"
#include "daeffect.h"
#include "dagalaxy.h"
#include "daitem.h"
#include "danpc.h"
#include "daperson.h"
#include "dasc.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dauniverse.h"
#include "davehicle.h"
#include "daworldgasgiant.h"
#include "daworldglobe.h"
#include "daworldring.h"
#include "daworldringsection.h"
#include "psweaponeffect.h"
#include <qimage.h>
#include <QPainter>
#include <QSqlError>
#include <qsqlquery.h>
#include <QVariant>
#include <QVector>

DaStar::DaStar()
{
	m_AxialTilt = 0;
	m_bySpecDec = 2;	// spectral decimal classification (0-9) (N/A for type D)
	memset(m_byZones, 0, sizeof(unsigned char) * 16);
	m_chHabitable = -1;	// number of habitable orbit (-1 if none)
	m_chOrbit = -1;		// orbit (-1 for primary)
	m_chRadius = -1;	// radius of star (in orbits)
	m_eSize = SIZE_V;	// spectral size
	m_eType = TYPE_G;	// spectral type
	m_fRotation[0] = 0.0f;
	m_fRotation[1] = 0.0f;
	m_fRotation[2] = 0.0f;
	m_InDesigner = 0;
	m_NeutronStarOutput = 0;
	m_PSStargateBeam = 0;
	m_StarNumber = -3;
	m_SurfaceTempK = 5778;	// Earth's sun Sol
	for(int i=0; i<4; i++)
		m_TexCorona[i] = 0;
	m_TexLightning = 0;
	m_TexNoise = 0;
	m_TexSun = 0;
	m_ZoneOfControl = FeetPerSolarSystem()/2 * FeetPerSolarSystem()/2;		// Default is the zone-of-control for primary stars.
}

DaStar::~DaStar()
{
	/*
#if !defined(QT_NO_DEBUG)
	if(m_SolarDisk.VAO())
		qDebug("DaStar::~DaStar - SunVAO = %d", m_SolarDisk.VAO());
#endif
*/
	if(m_PSStargateBeam)
		delete m_PSStargateBeam;
}

DaStar::DaStar(const DaStar &rhs) : DaAtom(rhs)
{
	m_bySpecDec = rhs.m_bySpecDec;
	m_chHabitable = rhs.m_chHabitable;
	m_chOrbit = rhs.m_chOrbit;
	m_chRadius = rhs.m_chRadius;
	m_InDesigner = rhs.m_InDesigner;
	m_StarNumber = rhs.m_StarNumber;
	m_ZoneOfControl = rhs.m_ZoneOfControl;

	rhs.m_StarLock.lockForRead();
	m_AxialTilt = rhs.m_AxialTilt;
	memcpy(m_byZones, rhs.m_byZones, sizeof(unsigned char) * 16);
	m_eSize = rhs.m_eSize;
	m_eType = rhs.m_eType;
	m_RingworldRefSection = rhs.m_RingworldRefSection;
	m_Stargates = rhs.m_Stargates;
	m_StargatesChange = rhs.m_StargatesChange;
	m_SurfaceTempK = rhs.m_SurfaceTempK;
	rhs.m_StarLock.unlock();

	m_fRotation[0] = rhs.m_fRotation[0];
	m_fRotation[1] = rhs.m_fRotation[1];
	m_fRotation[2] = rhs.m_fRotation[2];
	m_NeutronStarOutput = 0;
	m_PSStargateBeam = 0;
	m_SphereRot = rhs.m_SphereRot;
	for(int i=0; i<4; i++)
		m_TexCorona[i] = 0;
	m_TexLightning = 0;
	m_TexNoise = 0;
	m_TexSun = 0;
}

// Transmit updates to this atom to a client.
// 'tmLastRefresh' is the last time the client received an update.
void DaStar::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx, const DaPerson& sceneAtom)
{
	DaAtom::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);
	quint8 hasWhat = 0;
	if(lastTx.IsNull())
	{
		hasWhat |= 0x01;

		// Only send ringworld reference section when it is set.
		if(!m_RingworldRefSection.IsNull())
			hasWhat |= 0x02;

		hasWhat |= 0x04;				// Stargates
	}
	else
	{
		m_StarLock.lockForRead();
		if(!m_StargatesChange.IsNull() && m_StargatesChange >= lastTx)
			hasWhat |= 0x04;			// Stargates
		m_StarLock.unlock();
	}
	toClient << hasWhat;

	if(hasWhat & 0x01)
	{
		toClient << m_bySpecDec;

		m_StarLock.lockForRead();
		for(int iz=0; iz<16; iz++)
			toClient << m_byZones[iz];

		toClient << (quint8)m_chHabitable;
		toClient << (quint8)m_chOrbit;
		toClient << (qint8)m_chRadius;
		toClient << m_SurfaceTempK;

		// Combine size and type into a single byte.
		quint8 sizeType = (m_eSize << 4) | m_eType;

		m_StarLock.unlock();

		toClient << sizeType;
	}

	if(hasWhat & 0x02)
	{
		m_StarLock.lockForRead();
		toClient << m_RingworldRefSection;
		m_StarLock.unlock();
	}

	if(hasWhat & 0x04)
	{
		m_StarLock.lockForRead();
		toClient << m_Stargates;
		m_StarLock.unlock();
	}
}

// Receive updates to this atom from a server.
void DaStar::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	DaAtom::RxUpdate(fromServer, isFirstUpdate);
	quint8 hasWhat;
	fromServer >> hasWhat;

	if(hasWhat & 0x01)
	{
		fromServer >> m_bySpecDec;
		for(int iz=0; iz<16; iz++)
			fromServer >> m_byZones[iz];

		quint8 ui8;
		fromServer >> ui8;
		m_chHabitable = ui8;
		fromServer >> ui8;
		m_chOrbit = ui8;
		qint8 u8;
		fromServer >> u8;
		m_chRadius = u8;
		fromServer >> m_SurfaceTempK;

		// Size and type are combined into a single byte.
		fromServer >> ui8;
		m_eSize = (SIZE)(ui8 >> 4);
		m_eType = (TYPE)(ui8 & 0x0f);

		if(m_ZoneOfControl == FeetPerSolarSystem()/2 * FeetPerSolarSystem()/2)
			CalculateZoneOfControl();
	}

	if(hasWhat & 0x02)
		fromServer >> m_RingworldRefSection;

	if(hasWhat & 0x04)
		fromServer >> m_Stargates;
}

// Refresh data from valid counterpart.
bool DaStar::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	DaAtom::Refresh(pAtom, ari);

	const DaStar* pA = (const DaStar*)pAtom;
	m_bySpecDec = pA->m_bySpecDec;
	m_chHabitable = pA->m_chHabitable;
	m_chOrbit = pA->m_chOrbit;
	m_chRadius = pA->m_chRadius;
	m_ZoneOfControl = pA->m_ZoneOfControl;

	pA->m_StarLock.lockForRead();
	m_StarLock.lockForWrite();
	memcpy(m_byZones, pA->m_byZones, sizeof(unsigned char) * 16);
	m_eSize = pA->m_eSize;
	m_eType = pA->m_eType;
	m_RingworldRefSection = pA->m_RingworldRefSection;
	m_Stargates = pA->m_Stargates;
	m_SurfaceTempK = pA->m_SurfaceTempK;
	m_StarLock.unlock();
	pA->m_StarLock.unlock();

	return false;
}

DaAtom *DaStar::Clone() const
{
	return new DaStar(*this);
}

void DaStar::CreateForScan()
{
	DaAtom::CreateForScan();
}

void DaStar::CreateForPreview(quint8 previewType)
{
	// Earth's sun Sol is a G2 V.
	m_StarLock.lockForWrite();
	m_eType = TYPE_G;
	m_bySpecDec = 2;
	m_eSize = SIZE_V;
	m_SurfaceTempK = 5778;	// kelvin
	m_StarLock.unlock();
	InitZones();
	if(previewType == 3 || previewType == 9)
	{
		// Create a home world.
		AuDice dice;
		dice.Seed(GenSeed());
		CreatePlanet(HabitableZone(), HABITABLE_HomeWorld, dice);
	}
	else if(previewType == 4)
	{
		// Create a home world with a moon.
		AuDice dice;
		dice.Seed(GenSeed());
		bool hasMoon;
		do
		{
			CreatePlanet(HabitableZone(), HABITABLE_HomeWorld, dice);
			dice.Random();
			DaWorldLL* hab = GetHabitableWorld(HABITABLE_HomeWorld);
			DaAtomList moons;
			if(hab)
				hab->FindMoons(moons);
			hasMoon = !moons.empty();
			if(!hasMoon)
			{
				TrashSubatoms();
				InitZones();
			}
		}
		while(!hasMoon);
	}
	else if(previewType == 5)
	{
		// Inferno world
		AuDice dice;
		dice.Seed(GenSeed());
		CreatePlanet(LastInfernoOrbit(), HABITABLE_Unspecified, dice);
	}
	else if(previewType == 6)
	{
		// Frigid world
		AuDice dice;
		dice.Seed(GenSeed());
		CreatePlanet(FirstFrigidOrbit(), HABITABLE_Unspecified, dice);
	}
	else if(previewType == 7)
	{
		// Gas giant
		AuDice dice;
		dice.Seed(GenSeed());
		bool hasGasGiant;
		do
		{
			CreatePlanet(HabitableZone(), HABITABLE_Unspecified, dice);
			dice.Random();
			hasGasGiant = GetGasGiant();
			if(!hasGasGiant)
			{
				TrashSubatoms();
				InitZones();
			}
		}
		while(!hasGasGiant);
	}
	else if(previewType == 8)
	{
		// Ringworld
		AuDice dice;
		dice.Seed(GenSeed());
		CreateRingworld(HabitableZone(), dice);
	}

	// Signal the addition of all the nu atoms.
	DaAtomList allNew;
	ListNormalSubatomsRecursive(allNew);
	for(DaAtomList::const_iterator it = allNew.begin(); it != allNew.end(); it++)
	{
		DaAtom* pAtom = *it;
		pAtom->InitSystemPosition();
		g_Au.AtomAdded(pAtom->Id());
	}
}

// randomly generate a type for this primary star
// returns the modifier required when generating companions of this primary
// Earth's sun Sol is a G2 V.
int DaStar::GenPrimaryType(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	int lTypeRoll = dice.Roll(1,13,-1);		// [0-12]

	TYPE eTypeTable[13] = {TYPE_B,TYPE_B,TYPE_A,TYPE_M,TYPE_M,TYPE_M,TYPE_M,TYPE_M,TYPE_K,TYPE_G,TYPE_F,TYPE_F,TYPE_F};
	m_StarLock.lockForWrite();
	m_eType = eTypeTable[lTypeRoll];

	if (m_eType == TYPE_B && eTypeTable[dice.Roll(1,13) - 1] == TYPE_B)
		m_eType = TYPE_O;	// extremely rare type O occurs when we randomly roll type B twice in succession
	do 
	{
		m_bySpecDec = (unsigned char)dice.Roll(1,10,-1);
	}
	while (m_eType == TYPE_O && m_bySpecDec < 5);	// prevent specdec<5 on type O stars

	GenSurfaceTempK(dice);

	m_StarLock.unlock();
	
	return(lTypeRoll);
}

// randomly generate a size for this primary star
// returns the modifier required when generating companions of this primary
// Earth's sun Sol is a G2 V.
int DaStar::GenPrimarySize(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	int lSizeRoll = dice.Roll(1,13,-1);	// [0-12]

	SIZE eSizeTable[13] = {SIZE_Ia,SIZE_Ib,SIZE_II,SIZE_III,SIZE_IV,SIZE_V,SIZE_V,SIZE_V,SIZE_V,SIZE_V,SIZE_V,SIZE_VI,SIZE_D};
	m_StarLock.lockForWrite();
	m_eSize = eSizeTable[lSizeRoll];

	if (m_eSize == SIZE_IV && m_eType >= TYPE_K)
		m_eSize = SIZE_V;	// size IV not available for K and M stars
	if (m_eSize == SIZE_VI && (m_eType < TYPE_F || (m_eType == TYPE_F && m_bySpecDec <= 4)))
		m_eSize = SIZE_V;	// size VI not available for F4 and below

	m_StarLock.unlock();

	return(lSizeRoll);
}

// returns the size modifier required when generating companions of this black hole
void DaStar::GenSupermassiveBlackHole(const AuDice& Cdice, GALAXY galaxy, int& typeMod, int& sizeMod)
{
	AuDice dice(Cdice);
	m_StarLock.lockForWrite();
	m_eType = TYPE_SupermassiveBlackHole;
	m_bySpecDec = (unsigned char)dice.Roll(1,10,-1);
	m_eSize = SIZE_Ia;					// n/a for black hole.
	GenSurfaceTempK(dice);
	m_StarLock.unlock();

	// Calculate number of available orbits outside the photon sphere. This way we can return modifiers for generating companions
	// that insure they aren't too large to fit the available orbit space.
	int photonSphere = PhotonSphereRadiusInOrbits(galaxy);		// Range is 0 to 12.
	int availableOrbits = 15 - photonSphere;					// Range is 15 to 3.

	// Determine type modifier for companion stars based on number of available orbits outside the photosphere.
	int typeMods[16] =
	{
		0,								// Outside the possible range.
		0,								// Outside the possible range.
		0,								// Outside the possible range.
		1,								// Type O to Type G
		6,								// Type F to Type K
		1,								// Type O to Type G
		6,								// Type F to Type K
		1,								// Type O to Type G
		6,								// Type F to Type K
		1,								// Type O to Type G
		6,								// Type F to Type K
		1,								// Type O to Type G
		6,								// Type F to Type K
		1,								// Type O to Type G
		4,								// Type A to Type K
		6								// Type F to Type K
	};
	typeMod = typeMods[availableOrbits];

	// Determine size modifier for companion stars based on number of available orbits outside the photosphere.
	int sizeMods[16] =
	{
		0,								// Outside the possible range.
		0,								// Outside the possible range.
		0,								// Outside the possible range.
		8,								// Max Size IV
		8,								// Max Size IV
		7,								// Max size III
		7,								// Max size III
		6,								// Max size III
		6,								// Max size III
		5,								// Max size II
		5,								// Max size II
		4,								// Max size II
		4,								// Max size II
		3,								// Max size Ib
		3,								// Max size Ib
		2								// Max size Ib
	};
	sizeMod = sizeMods[availableOrbits];
}

void DaStar::GenStellarBlackHole(const AuDice& Cdice, int& typeMod, int& sizeMod)
{
	AuDice dice(Cdice);
	//qDebug() << "DaStar::GenStellarBlackHole" << Id() << Name();
	m_StarLock.lockForWrite();
	m_eType = TYPE_StellarBlackHole;
	m_bySpecDec = (unsigned char)dice.Roll(1,10,-1);
	m_eSize = SIZE_Ia;					// n/a for black hole.
	GenSurfaceTempK(dice);
	m_StarLock.unlock();
	typeMod = 0;
	sizeMod = 0;
}

// randomly generate a type for this companion star
// requires the modifier returned when generating the primary of this companion
void DaStar::GenCompanionType(const AuDice& Cdice, int lTypeMod)
{
	AuDice dice(Cdice);
	int lTypeRoll = dice.Roll(1,6,lTypeMod/2 - 1);
	if (lTypeRoll < 0)
		lTypeRoll = 0;
	else if (lTypeRoll > 12)
		lTypeRoll = 12;

	m_StarLock.lockForWrite();
	TYPE eTypeTable[13] = {TYPE_O,TYPE_B,TYPE_A,TYPE_F,TYPE_F,TYPE_G,TYPE_G,TYPE_K,TYPE_K,TYPE_M,TYPE_M,TYPE_M,TYPE_M};
	m_eType = eTypeTable[lTypeRoll];
	do 
	{
		m_bySpecDec = (unsigned char)dice.Roll(1,10,-1);
	}
	while (m_eType == TYPE_O && m_bySpecDec < 5);	// prevent specdec<5 on type O stars
	GenSurfaceTempK(dice);
	m_StarLock.unlock();
}

// randomly generate a size for this companion star
// requires the modifier returned when generating the primary of this companion
void DaStar::GenCompanionSize(const AuDice& Cdice, int lSizeMod)
{
	AuDice dice(Cdice);
	int lSizeRoll = dice.Roll(1,6,lSizeMod/2 - 1);
	if (lSizeRoll < 0)
		lSizeRoll = 0;
	else if (lSizeRoll > 12)
		lSizeRoll = 12;

	SIZE eSizeTable[13] = {SIZE_Ia,SIZE_Ib,SIZE_II,SIZE_III,SIZE_IV,SIZE_D,SIZE_D,SIZE_V,SIZE_V,SIZE_VI,SIZE_D,SIZE_D,SIZE_D};
	m_StarLock.lockForWrite();
	m_eSize = eSizeTable[lSizeRoll];

	if (m_eSize == SIZE_IV && m_eType >= TYPE_K && m_eType < TYPE_SupermassiveBlackHole)
		m_eSize = SIZE_V;	// size IV not available for K and M stars
	if (m_eSize == SIZE_VI && (m_eType < TYPE_F || (m_eType == TYPE_F && m_bySpecDec <= 4)))
		m_eSize = SIZE_V;	// size VI not available for F4 and below
	m_StarLock.unlock();
}

void DaStar::GenSurfaceTempK(AuDice& dice)
{
	// Black holes and neutron stars return 0.
	if(m_eType == TYPE_NeutronStar ||
			m_eType == TYPE_SupermassiveBlackHole ||
			m_eType == TYPE_StellarBlackHole)
	{
		m_SurfaceTempK = 0;
		return;
	}

	// Minimum temperature by star type.
	int typeMin[7] =
	{
		33000,	// O
		10000,	// B
		7500,	// A
		6000,	// F
		5200,	// G
		3700,	// K
		1300	// M
	};

	// Maximum temperature by star type.
	int typeMax[7] =
	{
		65000,	// O
		33000,	// B
		10000,	// A
		7500,	// F
		6000,	// G
		5200,	// K
		3700	// M
	};

	// specDec 0 is brighter than specDec 9
	int range = typeMax[m_eType] - typeMin[m_eType];				// full range of surface temps for the star type
	int specDecRange = range / 10;									// subdivided by spectral decimal value (0=hotter, 9=coolor)
	int base = typeMax[m_eType] - specDecRange * (m_bySpecDec + 1);	// base value of spec dec subrange
	m_SurfaceTempK = dice.Roll(1,specDecRange,base);
	Q_ASSERT(m_SurfaceTempK >= typeMin[TYPE_M]);
	Q_ASSERT(m_SurfaceTempK <= typeMax[TYPE_O]);
	//qDebug() << "DaStar::GenSurfaceTempK" << *this << Description() << "surfaceTemp" << m_SurfaceTempK
		//<< "range" << range << "specDecRange" << specDecRange << "base" << base;
}

// Returns the event horizon radius of this star, assuming it's a black hole.
// This is based on the radius of the galaxy.
//static
int DaStar::SchwarzschildRadiusInOrbits(GALAXY galaxy)
{
	if(galaxy == GALAXY_Count)
		return -1;	// This is a stellar black hole.

	// This is supposed to be a supermassive black hole at the center of the galaxy.
	// Wikipedia says size range is ~0.001-400AU.
	// Our largest system is 16 orbits, which is only 4 AU radius.
	// Star generation tables have the largest stars at orbit 7 radius, about 2AU.
	// Orbit 0 is the first orbit, at .25AU.
	// Let's go to orbit 8 with the largest black holes.
	// Radius of the galaxy ranges from 588pc to 2785pc.
	int radiusInOrbits = (int)(DaGalaxy::Radius(galaxy) / DaGalaxy::Radius(CrownOfOthon) * 9.) - 1;
	if(radiusInOrbits < -1)
		return -1;						// Orbit radius of -1 indicates very small size. Currently no galaxies are that small.
	else if(radiusInOrbits > 8)
		return 8;
	return radiusInOrbits;
}

// flags orbits for a black hole
// 0 = ZONE_InsideStar,
// 1 = ZONE_Unavailable,
// 2 = ZONE_Inferno,
// 3 = ZONE_Inner,
// 4 = ZONE_Habitable,
// 5 = ZONE_Outer,
// 6 = ZONE_Frigid,
void DaStar::InitBlackHoleZones(GALAXY galaxy)
{
	// Habitable zone is beyond radius of zones.
	m_chHabitable = 16;

	// Calculate radius of black hole, in orbits.
	m_chRadius = SchwarzschildRadiusInOrbits(galaxy);

	// Calculate radius of photon sphere, in orbits.
	int photonSphere = PhotonSphereRadiusInOrbits(galaxy);

	int unavail = 2;	// We will leave two orbits unavailable outside the photon sphere.
	m_StarLock.lockForWrite();
	for(int orbit=0; orbit<16; orbit++)
	{
		if(orbit <= m_chRadius)
			m_byZones[orbit] = ZONE_InsideStar;					// Orbit is inside the black hole.
		else if(orbit <= photonSphere)
			m_byZones[orbit] = ZONE_Unavailable;				// Orbit is inside the photosphere.
		else if(unavail)
		{
			m_byZones[orbit] = ZONE_Unavailable;				// Orbit is the next one outside the photosphere.
			unavail--;
		}
		else
			m_byZones[orbit] = ZONE_Frigid;						// Orbit is available but frozen.
	}
	m_StarLock.unlock();
}

// flags orbits according to spectral type and size as being
// 0 = ZONE_InsideStar,
// 1 = ZONE_Unavailable,
// 2 = ZONE_Inferno,
// 3 = ZONE_Inner,
// 4 = ZONE_Habitable,
// 5 = ZONE_Outer,
// 6 = ZONE_Frigid,
void DaStar::InitZones()
{
	switch(StarType())
	{
	case TYPE_SupermassiveBlackHole:
		Q_ASSERT(false);
		return;
	case TYPE_StellarBlackHole:
		InitBlackHoleZones(GALAXY_Count);
		return;
	case TYPE_NeutronStar:
		InitNeutronStarZones();
		return;
	default:
		break;
	}

	// byZones[Size][Orbit][Spectra]
	unsigned char byZones[8][16][15] = 
	{
		{	// Size Ia - Bright Supergiant
		//   O O B B A A F F G G K K M M M
		//   0 5 0 5 0 5 0 5 0 5 0 5 0 5 9
			{1,1,1,1,0,0,0,0,0,0,0,0,0,0,0},	// Orbit 0
			{1,1,1,1,0,0,0,0,0,0,0,0,0,0,0},	// Orbit 1
			{1,1,1,1,1,1,0,0,0,0,0,0,0,0,0},	// Orbit 2
			{1,1,1,1,1,1,1,1,0,0,0,0,0,0,0},	// Orbit 3
			{1,1,1,1,1,1,1,1,1,0,0,0,0,0,0},	// Orbit 4
			{1,1,1,1,1,1,1,1,1,1,0,0,0,0,0},	// Orbit 5
			{1,1,1,1,1,1,2,2,1,1,1,1,0,0,0},	// Orbit 6
			{1,1,1,2,2,2,2,2,2,2,2,2,2,0,0},	// Orbit 7
			{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},	// Orbit 8
			{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},	// Orbit 9
			{2,2,2,2,2,2,2,3,2,2,2,2,2,2,2},	// Orbit 10
			{2,2,2,3,3,3,3,4,3,3,3,3,3,3,3},	// Orbit 11
			{2,3,3,4,4,4,4,5,4,4,4,4,4,4,4},	// Orbit 12
			{3,4,4,5,5,5,5,6,5,5,5,5,5,5,5},	// Orbit 13
			{4,5,5,6,6,6,6,6,6,6,6,6,6,6,6},	// Orbit 14
			{5,6,6,6,6,6,6,6,6,6,6,6,6,6,6},	// Orbit 15
		},
		{	// Size Ib - Weaker Supergiant
			{1,1,1,1,1,1,1,1,1,0,0,0,0,0,0},
			{1,1,1,1,1,1,1,1,1,0,0,0,0,0,0},
			{1,1,1,1,1,1,1,1,1,1,0,0,0,0,0},
			{1,1,1,1,1,1,1,1,1,1,0,0,0,0,0},
			{1,1,1,1,1,1,1,2,2,1,1,0,0,0,0},
			{1,1,1,1,2,2,2,2,2,2,2,1,0,0,0},
			{1,1,1,2,2,2,2,2,2,2,2,2,2,0,0},
			{1,1,1,2,2,2,2,2,2,2,2,2,2,2,0},
			{1,1,2,2,2,2,2,2,2,2,2,2,2,2,2},
			{1,2,2,2,2,3,3,3,3,3,3,2,2,2,2},
			{2,2,2,3,3,4,4,4,4,4,4,3,3,2,2},
			{2,2,2,4,4,5,5,5,5,5,5,4,4,3,3},
			{2,2,3,5,5,6,6,6,6,6,6,5,5,4,4},
			{2,2,4,6,6,6,6,6,6,6,6,6,6,5,5},
			{3,3,5,6,6,6,6,6,6,6,6,6,6,6,6},
			{4,4,6,6,6,6,6,6,6,6,6,6,6,6,6},
		},
		{	// Size II - Bright Giant
			{1,1,1,1,1,1,1,1,1,1,1,0,0,0,0},
			{1,1,1,1,1,1,1,1,1,1,1,0,0,0,0},
			{1,1,1,1,1,2,2,2,2,2,2,1,0,0,0},
			{1,1,1,1,2,2,2,2,2,2,2,2,0,0,0},
			{1,1,1,1,2,2,2,2,2,2,2,2,2,0,0},
			{1,1,1,2,2,2,2,2,2,2,2,2,2,0,0},
			{1,1,1,2,2,2,2,2,2,2,2,2,2,2,2},
			{1,2,2,2,2,3,3,3,3,3,2,2,2,2,2},
			{2,2,2,2,3,4,4,4,4,4,3,3,2,2,2},
			{2,2,2,2,4,5,5,5,5,5,4,4,3,2,2},
			{2,2,2,3,5,6,6,6,6,6,5,5,4,3,3},
			{2,2,3,4,6,6,6,6,6,6,6,6,5,4,4},
			{2,2,4,5,6,6,6,6,6,6,6,6,6,5,5},
			{2,3,5,6,6,6,6,6,6,6,6,6,6,6,6},
			{3,4,6,6,6,6,6,6,6,6,6,6,6,6,6},
			{4,5,6,6,6,6,6,6,6,6,6,6,6,6,6},
		},
		{	// Size III - Giant
			{1,1,1,1,2,2,2,2,2,2,2,2,1,0,0},
			{1,1,1,1,2,2,2,2,2,2,2,2,1,0,0},
			{1,1,1,1,2,2,2,2,2,2,2,2,2,0,0},
			{1,1,1,1,2,2,2,2,2,2,2,2,2,0,0},
			{1,1,1,1,2,2,2,2,2,2,2,2,2,2,0},
			{1,1,1,2,2,2,3,3,3,2,2,2,2,2,2},
			{1,1,1,2,2,3,4,4,4,3,3,2,2,2,2},
			{1,1,2,2,3,4,5,5,5,4,4,3,3,2,2},
			{2,2,2,2,4,5,6,6,6,5,5,4,4,3,3},
			{2,2,2,3,5,6,6,6,6,6,6,5,5,4,4},
			{2,2,2,4,6,6,6,6,6,6,6,6,6,5,5},
			{2,2,3,5,6,6,6,6,6,6,6,6,6,6,6},
			{2,2,4,6,6,6,6,6,6,6,6,6,6,6,6},
			{2,3,5,6,6,6,6,6,6,6,6,6,6,6,6},
			{2,4,6,6,6,6,6,6,6,6,6,6,6,6,6},
			{3,5,6,6,6,6,6,6,6,6,6,6,6,6,6},
		},
		{	// Size IV - Subgiant
			{1,1,1,1,1,2,2,2,2,2,2,0,0,0,0},
			{1,1,1,1,2,2,2,2,2,2,2,0,0,0,0},
			{1,1,1,1,2,2,2,2,2,2,2,0,0,0,0},
			{1,1,1,2,2,2,2,2,2,2,3,0,0,0,0},
			{1,1,1,2,2,2,2,3,3,3,4,0,0,0,0},
			{1,1,1,2,2,3,3,4,4,4,5,0,0,0,0},
			{1,1,1,2,3,4,4,5,5,5,6,0,0,0,0},
			{1,2,2,2,4,5,5,6,6,6,6,0,0,0,0},
			{2,2,2,3,5,6,6,6,6,6,6,0,0,0,0},
			{2,2,2,4,6,6,6,6,6,6,6,0,0,0,0},
			{2,2,2,5,6,6,6,6,6,6,6,0,0,0,0},
			{2,2,3,6,6,6,6,6,6,6,6,0,0,0,0},
			{2,2,4,6,6,6,6,6,6,6,6,0,0,0,0},
			{2,2,5,6,6,6,6,6,6,6,6,0,0,0,0},
			{2,3,6,6,6,6,6,6,6,6,6,0,0,0,0},
			{2,4,6,6,6,6,6,6,6,6,6,0,0,0,0},
		},
		{	// Size V - Main Sequence
			{1,1,1,1,2,2,2,2,2,2,2,4,4,5,6},
			{1,1,1,1,2,2,2,2,2,3,3,5,5,6,6},
			{1,1,1,1,2,2,2,2,3,4,4,6,6,6,6},
			{1,1,1,2,2,2,2,3,4,5,5,6,6,6,6},
			{1,1,1,2,2,2,3,4,5,6,6,6,6,6,6},
			{1,1,1,2,2,3,4,5,6,6,6,6,6,6,6},
			{1,1,2,2,3,4,5,6,6,6,6,6,6,6,6},
			{1,2,2,2,4,5,6,6,6,6,6,6,6,6,6},
			{2,2,2,3,5,6,6,6,6,6,6,6,6,6,6},
			{2,2,2,4,6,6,6,6,6,6,6,6,6,6,6},
			{2,2,2,5,6,6,6,6,6,6,6,6,6,6,6},
			{2,2,3,6,6,6,6,6,6,6,6,6,6,6,6},
			{2,2,4,6,6,6,6,6,6,6,6,6,6,6,6},
			{2,2,5,6,6,6,6,6,6,6,6,6,6,6,6},
			{2,3,6,6,6,6,6,6,6,6,6,6,6,6,6},
			{2,4,6,6,6,6,6,6,6,6,6,6,6,6,6},
		},
		{	// Size VI - Sub Dwarf
			{0,0,0,0,0,0,0,2,2,3,3,5,6,6,6},
			{0,0,0,0,0,0,0,2,3,4,4,6,6,6,6},
			{0,0,0,0,0,0,0,3,4,5,5,6,6,6,6},
			{0,0,0,0,0,0,0,4,5,6,6,6,6,6,6},
			{0,0,0,0,0,0,0,5,6,6,6,6,6,6,6},
			{0,0,0,0,0,0,0,6,6,6,6,6,6,6,6},
			{0,0,0,0,0,0,0,6,6,6,6,6,6,6,6},
			{0,0,0,0,0,0,0,6,6,6,6,6,6,6,6},
			{0,0,0,0,0,0,0,6,6,6,6,6,6,6,6},
			{0,0,0,0,0,0,0,6,6,6,6,6,6,6,6},
			{0,0,0,0,0,0,0,6,6,6,6,6,6,6,6},
			{0,0,0,0,0,0,0,6,6,6,6,6,6,6,6},
			{0,0,0,0,0,0,0,6,6,6,6,6,6,6,6},
			{0,0,0,0,0,0,0,6,6,6,6,6,6,6,6},
			{0,0,0,0,0,0,0,6,6,6,6,6,6,6,6},
			{0,0,0,0,0,0,0,6,6,6,6,6,6,6,6},
		},
		{	// Size D - White Dwarf
			{3,3,4,4,5,5,6,6,6,6,6,6,6,6,6},
			{4,4,5,5,6,6,6,6,6,6,6,6,6,6,6},
			{5,5,6,6,6,6,6,6,6,6,6,6,6,6,6},
			{6,6,6,6,6,6,6,6,6,6,6,6,6,6,6},
			{6,6,6,6,6,6,6,6,6,6,6,6,6,6,6},
			{6,6,6,6,6,6,6,6,6,6,6,6,6,6,6},
			{6,6,6,6,6,6,6,6,6,6,6,6,6,6,6},
			{6,6,6,6,6,6,6,6,6,6,6,6,6,6,6},
			{6,6,6,6,6,6,6,6,6,6,6,6,6,6,6},
			{6,6,6,6,6,6,6,6,6,6,6,6,6,6,6},
			{6,6,6,6,6,6,6,6,6,6,6,6,6,6,6},
			{6,6,6,6,6,6,6,6,6,6,6,6,6,6,6},
			{6,6,6,6,6,6,6,6,6,6,6,6,6,6,6},
			{6,6,6,6,6,6,6,6,6,6,6,6,6,6,6},
			{6,6,6,6,6,6,6,6,6,6,6,6,6,6,6},
			{6,6,6,6,6,6,6,6,6,6,6,6,6,6,6},
		},
	};

	m_StarLock.lockForRead();
	int lSpectra = m_eType*2;		// Even numbers, max 12.
	if(m_bySpecDec >= 5)
		lSpectra++;					// Odd numbers, max 13.
	if(m_eType == TYPE_M && m_bySpecDec == 9)
		lSpectra = 14;				// Max 14 when Type and SpecDec are maxed out.
	m_StarLock.unlock();

	//Q_ASSERT(m_eSize >= 0 && m_eSize < 8);
	Q_ASSERT(lSpectra >= 0 && lSpectra < 15);

	m_chHabitable = -1;
	m_chRadius = -1;
	m_StarLock.lockForWrite();
	for(unsigned int lOrbit=0;lOrbit<16;lOrbit++)
	{
		m_byZones[lOrbit] = byZones[m_eSize][lOrbit][lSpectra];
		if (m_byZones[lOrbit] == ZONE_Habitable)
			m_chHabitable = lOrbit;
		else if (m_byZones[lOrbit] == ZONE_InsideStar)
			m_chRadius = lOrbit;	// Radius of star itself.
	}
	if(m_chHabitable == -1 && m_byZones[15] <= ZONE_Inner)
		m_chHabitable = 16;	// Let's pretend the next zone out farther would have been habitable (for light intensity calculations).
	m_StarLock.unlock();
}

// generate an orbit for a companion star around this star
// 'lCompanion' is the number of the companion star (0 for the first one)
// depends upon m_byZones being initialized already
char DaStar::GenCompanionOrbit(const AuDice& Cdice, int lCompanion)
{
	AuDice dice(Cdice);
	int lOrbitRoll = dice.Roll(2,6);
	if(lCompanion)
		lOrbitRoll += lCompanion+2;
	if(lOrbitRoll < 0)
		lOrbitRoll = 0;
	else if(lOrbitRoll > 12)
		lOrbitRoll = 12;

	int lOrbit;
	switch (lOrbitRoll)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		lOrbit = 0;
		break;
	case 4:
		lOrbit = 1;
		break;
	case 5:
		lOrbit = 2;
		break;
	case 6:
		lOrbit = 3;
		break;
	case 7:
		lOrbit = dice.Roll(1,6,4);
		break;
	case 8:
		lOrbit = dice.Roll(1,6,5);
		break;
	case 9:
		lOrbit = dice.Roll(1,6,6);
		break;
	case 10:
		lOrbit = dice.Roll(1,6,7);
		break;
	case 11:
		lOrbit = dice.Roll(1,6,8);
		break;
	case 12:
	default:
		lOrbit = 15;
		break;
	}

	m_StarLock.lockForRead();
	while(lOrbit >= 0 && lOrbit < 16 && m_byZones[lOrbit] <= ZONE_Unavailable)
		lOrbit++;	// move outward until we find an orbit that's not inside this star and not unavailable
	if(lOrbit >= 16)
	{
		// Search for any available orbit, from inner to outer.
		lOrbit = 0;
		while(lOrbit >= 0 && lOrbit < 16 && m_byZones[lOrbit] <= ZONE_Unavailable)
			lOrbit++;	// move outward until we find an orbit that's not inside this star and not unavailable
		if(lOrbit >= 16)
		{
			qWarning() << "error: DaStar::GenCompanionOrbit" << *this << "no available orbit for companion" << lCompanion;
			lOrbit = 0;
		}
	}
	m_StarLock.unlock();
	return((char)lOrbit);
}

void DaStar::InitMaxOrbits(const AuDice& Cdice)
{
	AuDice dice(Cdice);
	// determine the highest numbered orbit available to this star and mark the rest as unavailable
	int lOrbitsMod = 0;
	switch(StarType())
	{
	case TYPE_StellarBlackHole:
	case TYPE_SupermassiveBlackHole:
		return;							// Neutron stars need all the orbits they can get.
	case TYPE_NeutronStar:
		break;
	case TYPE_M:
		lOrbitsMod -= 4;
		break;
	case TYPE_K:
		lOrbitsMod -= 2;
		break;
	default:
		break;
	}

	m_StarLock.lockForRead();
	switch(m_eSize)
	{
	case SIZE_Ia:
	case SIZE_Ib:
	case SIZE_II:
		lOrbitsMod += 8;
		break;
	case SIZE_III:
		lOrbitsMod += 4;
		break;
	default:
		break;
	}
	m_StarLock.unlock();

	int lMaxOrbits = dice.Roll(2,6,lOrbitsMod);
	if(!IsPrimary())
	{
		// Companion stars max orbits cannot exceed their maximum number of orbits.
		// The max orbits value has been temporarily stored in the m_fRotation variable.
		if(lMaxOrbits > (int)m_fRotation[0])
			lMaxOrbits = m_fRotation[0];
		m_fRotation[0] = 0;				// I'm done with this thing now.
	}
	if(lMaxOrbits < 0)
		lMaxOrbits = 0;
	m_StarLock.lockForWrite();
	for(int lOrbit=lMaxOrbits;lOrbit<16;lOrbit++)
		m_byZones[lOrbit] = ZONE_Unavailable;	// unavailable orbit
	m_StarLock.unlock();
}

// Determine minimum orbit that can be used by a companion star.
char DaStar::MinOrbitAvailableFrom(char chCompanionOrbit) const
{
	// The companion must search outward from its orbit looking for contiguous available orbits.
	char chmin = chCompanionOrbit;
	char chmax = chCompanionOrbit;
	m_StarLock.lockForRead();
	while (chmin > 0 && m_byZones[chmin-1] >= ZONE_Inferno && (chmax > 15 || m_byZones[chmax+1] >= ZONE_Inferno))
	{
		chmin--;
		chmax++;
	}
	m_StarLock.unlock();
	return chmin;
}

// Mark the range of orbits occupied by this companion star as unavailable.
void DaStar::ReserveZones(char chMinOrb, char chMaxOrb)
{
	m_StarLock.lockForWrite();
	for(int iOrb=chMinOrb; iOrb <= chMaxOrb && iOrb < 16; iOrb++)
	{
		// We don't reserve orbits inside of stars.
		if(m_byZones[iOrb] >= ZONE_Inferno)
		{
			// The orbit is not inside the star and it is not already reserved so it can be reserved.
			m_byZones[iOrb] = ZONE_Unavailable;	// reserved orbit is now unavailable to others
		}
	}
	m_StarLock.unlock();
}

// create the planets that orbit this star
void DaStar::CreatePlanets(HABITABILITY hab)
{
	//qDebug() << "DaStar::CreatePlanets" << *this;
	AuDice dice(GenSeed());
	if(!IsAnyBlackHole())
	{
		// Mark outer orbits as unavailable.
		InitMaxOrbits(dice);
		dice.Random();

		// Count available Inner, Habitable, and Outer zones.
		// Remember the habitable zone.
		int lMaxPlanets = 0;
		m_StarLock.lockForRead();
		for(unsigned int lZone=0; lZone<16; lZone++)
			if(m_byZones[lZone] >= ZONE_Inferno)
				lMaxPlanets++;
		m_StarLock.unlock();

		int lPlanets = dice.Roll(2,6);
		lPlanets = MIN(lPlanets, lMaxPlanets);

		//qDebug("DaStar::CreatePlanets %d planets %d max", lPlanets, lMaxPlanets);

		// In our happy little universe, there's always a planet in the habitable zone if the zone is available.
		m_StarLock.lockForRead();
		bool v = m_chHabitable >= 0 && m_chHabitable < 16 && m_byZones[(int)m_chHabitable] == ZONE_Habitable;
		m_StarLock.unlock();
		if(v)
		{
			// There IS a planet here.
			// Note that the planet may not be habitable even though it is in the habitable zone.
			// Check for ringworld presense. Homeworlds are never ringworlds, though it is remotely possible a companion star in a
			// homeworld system could have a ringworld; that would be fun.
			bool ringworld = false;
			if(hab != HABITABLE_HomeWorld && m_chHabitable < 3)
			{
				// A ringworld could fit in the habitable zone if there are no secondary stars too close.
				bool ringworldBlocked = false;
				if(IsPrimary())
				{
					// Only the primary can have secondary stars. Secondary stars have safely reserved their zone of control.
					// Look for a secondary star that is inside the habitable orbit.
					DaAtomList subs;
					ListNormalSubatoms(subs);
					for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
					{
						DaStar* star = dynamic_cast<DaStar*>(*it);
						if(!star)
							continue;
						if(star->Orbit() < m_chHabitable)
						{
							// Found a secondary star orbiting inside the habitable orbit of the primary star.
							// A ringworld cannot be placed around the primary star.
							ringworldBlocked = true;
							break;
						}
					}
				}

				if(!ringworldBlocked)
				{
					// The area is not blocked to ringworlds.
					// Give it a 3% chance of being a ringworld system.
					if(dice.Roll(1,100) <= 3)
						ringworld = true;
				}
			}
			if(ringworld)
			{
				CreateRingworld(m_chHabitable, dice);
				dice.Random();
				lPlanets = 0;	// No planets left around this star. They could exist around companion stars.
			}
			else
			{
				CreatePlanet(m_chHabitable, hab, dice);
				dice.Random();
				lPlanets--;
			}
		}

		// Create remaining planets.
		for(; lPlanets>0; lPlanets--)
		{
			// Find an available orbit zone.
			int lOrbit = 0;
			m_StarLock.lockForRead();
			do
			{
				lOrbit = dice.Roll(1,16,-1);	// [0-15]
			}
			while(m_byZones[lOrbit] < ZONE_Inferno);
			m_StarLock.unlock();

			CreatePlanet((char)lOrbit, HABITABLE_Unspecified, dice);
			dice.Random();
		}

		// Turn off the home system flag if we managed to create a habitable
		// world around the primary star.
		if(hab != HABITABLE_Unspecified && GetHabitableWorld(hab))
		{
			// No need to have habitability logic active for companion stars since we have created a suitable world already.
			if(hab == HABITABLE_Ringworld || hab == HABITABLE_HomeWorld)
				hab = HABITABLE_System;
			else
				hab = HABITABLE_Unspecified;
		}

		// Reset orbit zones based on star data.
		InitZones();
	}

	// Update zone-of-control radius based on planets.
	CalculateZoneOfControl();

	// Create planets on companion stars.
	if(IsPrimary())
	{
		DaAtomList subs;
		ListNormalSubatoms(subs);
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		{
			DaStar* pAtom = dynamic_cast<DaStar*>(*it);
			if(!pAtom)
				continue;
			pAtom->CreatePlanets(hab);

			// Turn off the home system flag if we managed to create a habitable
			// world around the companion star.
			if(hab != HABITABLE_Unspecified && pAtom->GetHabitableWorld(hab))
			{
				// No need to have habitability logic active for companion stars since we have created a suitable world already.
				if(hab == HABITABLE_Ringworld || hab == HABITABLE_HomeWorld)
					hab = HABITABLE_System;
				else
					hab = HABITABLE_Unspecified;
			}
		}
	}
	//qDebug() << "DaStar::CreatePlanets" << *this << "done";
}

// create the planets that orbit this star
void DaStar::CreatePlanetsForScan(HABITABILITY hab, GALAXY galaxy)
{
	//qDebug() << "DaStar::CreatePlanetsForScan around star" << Id() << Name();
	AuDice dice(GenSeed());
	if(!IsAnyBlackHole())
	{
		// Mark outer orbits as unavailable.
		InitMaxOrbits(dice);
		dice.Random();

		// Count available Inner, Habitable, and Outer zones.
		// Remember the habitable zone.
		int lMaxPlanets = 0;
		m_StarLock.lockForRead();
		for(unsigned int lZone=0; lZone<16; lZone++)
			if(m_byZones[lZone] >= ZONE_Inferno)
				lMaxPlanets++;
		m_StarLock.unlock();

		int lPlanets = dice.Roll(2,6);
		lPlanets = MIN(lPlanets, lMaxPlanets);

		//qDebug("DaStar::CreatePlanets %d planets %d max", lPlanets, lMaxPlanets);

		// In our happy little universe, there's always a planet in the habitable zone if the zone is available.
		m_StarLock.lockForRead();
		bool v = m_chHabitable >= 0 && m_chHabitable < 16 && m_byZones[(int)m_chHabitable] == ZONE_Habitable;
		m_StarLock.unlock();
		if(v)
		{
			// There IS a planet here.
			// Note that the planet may not be habitable even though it is in the habitable zone.
			// Check for ringworld presense. Homeworlds are never ringworlds, though it is remotely possible a companion star in a
			// homeworld system could have a ringworld; that would be fun.
			bool ringworld = false;
			if(hab != HABITABLE_HomeWorld && m_chHabitable < 3)
			{
				// A ringworld could fit in the habitable zone if there are no secondary stars too close.
				bool ringworldBlocked = false;
				if(IsPrimary())
				{
					// Only the primary can have secondary stars. Secondary stars have safely reserved their zone of control.
					// Look for a secondary star that is inside the habitable orbit.
					DaAtomList subs;
					ListNormalSubatoms(subs);
					for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
					{
						DaStar* star = dynamic_cast<DaStar*>(*it);
						if(!star)
							continue;
						if(star->Orbit() < m_chHabitable)
						{
							// Found a secondary star orbiting inside the habitable orbit of the primary star.
							// A ringworld cannot be placed around the primary star.
							ringworldBlocked = true;
							break;
						}
					}
				}

				if(!ringworldBlocked)
				{
					// The area is not blocked to ringworlds.
					// Give it a 3% chance of being a ringworld system.
					if(dice.Roll(1,100) <= 3)
						ringworld = true;
					//qDebug() << "DaStar::CreatePlanetsForScan" << *this << "ringworld possible, is here" << ringworld;
				}
			}
			if(ringworld)
			{
				CreateRingworldForScan(m_chHabitable, dice);	// Client local id, not a real atom id.
				dice.Random();
				lPlanets = 0;	// No planets left around this star. They could exist around companion stars.
			}
			else
			{
				CreatePlanetForScan(m_chHabitable, hab, dice);	// Client local id, not a real atom id.
				dice.Random();
				lPlanets--;
			}
		}

		// Create remaining planets.
		for(; lPlanets>0; lPlanets--)
		{
			// Find an available orbit zone.
			int lOrbit = 0;
			m_StarLock.lockForRead();
			do
			{
				lOrbit = dice.Roll(1,16,-1);	// [0-15]
			}
			while(m_byZones[lOrbit] < ZONE_Inferno);
			m_StarLock.unlock();

			CreatePlanetForScan((char)lOrbit, HABITABLE_Unspecified, dice);	// Client local id, not real atom.
			dice.Random();
		}

		// Turn off the home system flag if we managed to create a habitable
		// world around the primary star.
		if(hab != HABITABLE_Unspecified && GetHabitableWorld(hab))
		{
			// No need to have habitability logic active for companion stars since we have created a suitable world already.
			if(hab == HABITABLE_Ringworld || hab == HABITABLE_HomeWorld)
				hab = HABITABLE_System;
			else
				hab = HABITABLE_Unspecified;
		}

		// Reset orbit zones based on star data.
		InitZones();
	}

	// Update zone-of-control radius based on planets.
	CalculateZoneOfControl();

	// Create planets on companion stars.
	if(IsPrimary())
	{
		DaAtomList subs;
		ListNormalSubatoms(subs);
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		{
			DaStar* pAtom = dynamic_cast<DaStar*>(*it);
			if(!pAtom)
				continue;
			pAtom->CreatePlanetsForScan(hab, galaxy);

			// Turn off the home system flag if we managed to create a habitable
			// world around the companion star.
			if(hab != HABITABLE_Unspecified && pAtom->GetHabitableWorld(hab))
			{
				// No need to have habitability logic active for companion stars since we have created a suitable world already.
				if(hab == HABITABLE_Ringworld || hab == HABITABLE_HomeWorld)
					hab = HABITABLE_System;
				else
					hab = HABITABLE_Unspecified;
			}
		}
	}
	//qDebug() << "DaStar::CreatePlanetsForScan" << *this << "done";
}

void DaStar::CreatePlanet(int orbit, HABITABILITY hab, const AuDice& dice)
{
	if(orbit >= 16)
	{
		qWarning("DaStar::CreatePlanet - ERROR! Orbit %d exceeds 15.", orbit);
		return;
	}
	//qDebug("DaStar::CreatePlanet orbit %d hab %d", orbit, hab);

	// Orbit affects Create
	double radOrbitAu = OrbitRadius(orbit);

	m_StarLock.lockForRead();
	ORBITZONE z = (ORBITZONE)(m_byZones[orbit] - ZONE_Inferno);
	m_StarLock.unlock();

	CreateWorld(hab, radOrbitAu, z, dice);

	// AtomAdded is not called because all atoms added as a result of
	// CreatePlanet are reported at the end.
	// See DaSolarSystem::FinalizeContents
	//g_Au.AtomAdded(pPlanet->Id());

	// This orbit zone is no longer available.
	m_StarLock.lockForWrite();
	m_byZones[orbit] = ZONE_Unavailable;
	m_StarLock.unlock();
}

void DaStar::CreatePlanetForScan(int orbit, HABITABILITY hab, const AuDice& dice)
{
	if(orbit >= 16)
	{
		qWarning("DaStar::CreatePlanetForScan - ERROR! Orbit %d exceeds 15.", orbit);
		return;
	}
	//qDebug("DaStar::CreatePlanetForScan orbit %d hab %d", orbit, hab);

	// Orbit affects Create
	double radOrbitAu = OrbitRadius(orbit);

	m_StarLock.lockForRead();
	ORBITZONE z = (ORBITZONE)(m_byZones[orbit] - ZONE_Inferno);
	m_StarLock.unlock();

	CreateWorldForScan(hab, radOrbitAu, z, dice);

	// AtomAdded is not called because all atoms added as a result of
	// CreatePlanet are reported at the end.
	// See DaSolarSystem::FinalizeContents
	//g_Au.AtomAdded(pPlanet->Id());

	// This orbit zone is no longer available.
	m_StarLock.lockForWrite();
	m_byZones[orbit] = ZONE_Unavailable;
	m_StarLock.unlock();
}

void DaStar::CreateRingworld(int orbit, const AuDice& dice)
{
	// This check eliminates a release build compiler warning about orbit being below the bound of the m_byZones array.
	if(orbit < 0)
		return;

	// Orbit affects Create
	double radOrbitAu = OrbitRadius(orbit);

	m_StarLock.lockForRead();
	ORBITZONE z = (ORBITZONE)(m_byZones[orbit] - ZONE_Inferno);
	m_StarLock.unlock();

	CreateWorld(HABITABLE_Ringworld, radOrbitAu, z, dice);

	// AtomAdded is not called because all atoms added as a result of
	// CreatePlanet are reported at the end.
	// See DaSolarSystem::FinalizeContents
	//g_Au.AtomAdded(pPlanet->Id());

	// This orbit zone is no longer available. All zones closer to the sun are also unavailable. It really doesn't work to have
	// a binary orbiting in there. We need to know the space is empty for the shadow squares.
	// There will not be any more planets around the primary star. Zones beyond the habitable zone are available for binaries and
	// their planets, possibly even a trinary.
	m_StarLock.lockForWrite();
	for(int zone=0; zone<=orbit; zone++)
		if(m_byZones[zone] != ZONE_InsideStar)
			m_byZones[zone] = ZONE_Unavailable;
	m_StarLock.unlock();
}

void DaStar::CreateRingworldForScan(int orbit, const AuDice& dice)
{
	// This check eliminates a release build compiler warning about orbit being below the bound of the m_byZones array.
	if(orbit < 0)
		return;

	// Orbit affects Create
	double radOrbitAu = OrbitRadius(orbit);

	m_StarLock.lockForRead();
	ORBITZONE z = (ORBITZONE)(m_byZones[orbit] - ZONE_Inferno);
	m_StarLock.unlock();

	CreateWorldForScan(HABITABLE_Ringworld, radOrbitAu, z, dice);

	// AtomAdded is not called because all atoms added as a result of
	// CreatePlanet are reported at the end.
	// See DaSolarSystem::FinalizeContents
	//g_Au.AtomAdded(pPlanet->Id());

	// This orbit zone is no longer available. All zones closer to the sun are also unavailable. It really doesn't work to have
	// a binary orbiting in there. We need to know the space is empty for the shadow squares.
	// There will not be any more planets around the primary star. Zones beyond the habitable zone are available for binaries and
	// their planets, possibly even a trinary.
	m_StarLock.lockForWrite();
	for(int zone=0; zone<=orbit; zone++)
		if(m_byZones[zone] != ZONE_InsideStar)
			m_byZones[zone] = ZONE_Unavailable;
	m_StarLock.unlock();
}

void DaStar::CreateWorld(HABITABILITY hab, double radOrbitAu, ORBITZONE orbitZone, const AuDice& Cdice)
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	AuDice dice(Cdice);
	//qDebug("DaWorld::Create hab %d orbitRad %g", hab, m_OrbitRadius);
	if(hab == HABITABLE_Ringworld)
	{
		// Just create the ringworld. Don't create any moons.
		DaWorldRingSection* ringRefSection = new DaWorldRingSection;
		ringRefSection->ConnectSignals();

		// Create first ringworld section.
		// The first section is designated section index 0. It is the reference section for positioning of all sections of the
		// ringworld. It is also the section responsible for creating the shadow squares object.
		double sections = DaWorldRingSection::RingworldSections(radOrbitAu);	// Number of sections so each has minimum length.
		ringRefSection->CreateFirstRingworldSection(dice, radOrbitAu, orbitZone);
		dice.Random();
		m_StarLock.lockForWrite();
		m_RingworldRefSection = ringRefSection->Id();
		m_StarLock.unlock();

		ringRefSection->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
		ether->AddAtom(ringRefSection);
		ringRefSection->SetNormal();	// New atom is ready for normal thread processing.

		// Create remainder of ring sections, beginning with index 1.
		for(int sectionIndex=1; sectionIndex<sections; sectionIndex++)
		{
			DaWorldRingSection* ringSection = new DaWorldRingSection;
			ringSection->ConnectSignals();
			ringSection->CreateRingworldSection(dice, sectionIndex, *ringRefSection);
			ringSection->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
			ether->AddAtom(ringSection);
			ringSection->SetNormal();	// New atom is ready for normal thread processing.
			dice.Random();
		}
		return;
	}

	// Check to see if the nu world will be a gas giant.
	int igg = dice.Roll(1,100);
	if(igg <= 5)
	{
		// Create an asteroid belt.
		int rocks = dice.Roll(2,3,-1);	// One to five planetoids. If more than five, then update DaWorldGlobe::DefaultName.
		for(int r=0; r<rocks; r++)
		{
			DaWorldGlobe* w = new DaWorldGlobe;
			w->ConnectSignals();
			w->Create(BODY_Planetoid, hab, dice, radOrbitAu, orbitZone, r);
			w->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
			ether->AddAtom(w);
			w->SetNormal();	// New atom is ready for normal thread processing.
			dice.Random();
		}
	}
	else if(igg <= 80)
	{
		// Create a solid spherical planet, not a gas giant.
		DaWorldGlobe* w = new DaWorldGlobe;
		w->ConnectSignals();
		w->Create(BODY_Planet, hab, dice, radOrbitAu, orbitZone, 0);
		w->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
		ether->AddAtom(w);
		w->SetNormal();	// New atom is ready for normal thread processing.
		dice.Random();

		int moons = w->GenMoonCount(dice);
		dice.Random();
		if(moons)
		{
			double radm = w->AtmosphereRadius() + FirstMoonOrbitAboveAtm();	// First available moon orbit radius;
			w->CreateMoons(moons, radm, false, dice, hab, orbitZone);
			dice.Random();
		}

		// Sometimes it is fun to have a twin planet in the habitable zone. Don't bother in other zones.
		if(orbitZone == OZONE_Habitable && dice.Roll(1,100) == 1)
		{
			// Create a twin world, a solid spherical planet, not a gas giant.
			DaWorldGlobe* tw = new DaWorldGlobe;
			tw->ConnectSignals();
			tw->Create(BODY_Planet, HABITABLE_Unspecified, dice, radOrbitAu, orbitZone, 1);
			tw->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
			ether->AddAtom(tw);
			tw->SetNormal();	// New atom is ready for normal thread processing.
			dice.Random();

			int moons = tw->GenMoonCount(dice);
			dice.Random();
			if(moons)
			{
				double radm = tw->AtmosphereRadius() + FirstMoonOrbitAboveAtm();	// First available moon orbit radius;
				tw->CreateMoons(moons, radm, false, dice, hab, orbitZone);
				dice.Random();
			}
		}
	}
	else
	{
		// Create a gas giant.
		DaWorldGasGiant* w = new DaWorldGasGiant;
		w->ConnectSignals();
		w->Create(dice, radOrbitAu, orbitZone);
		w->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
		ether->AddAtom(w);
		w->SetNormal();	// New atom is ready for normal thread processing.
		dice.Random();

		int moons = w->GenMoonCount(dice);
		dice.Random();
		if(moons)
		{
			// A titan is possible around every gas giant.
			bool hasTitan = false;
			if(orbitZone == OZONE_Habitable || dice.Roll(1,10) >= 8)
				hasTitan = true;
			double radm = w->AtmosphereRadius() + FirstMoonOrbitAboveAtm();	// First available moon orbit radius;
			w->CreateMoons(moons, radm, hasTitan, dice, hab, orbitZone);
		}
	}
}

void DaStar::CreateWorldForScan(HABITABILITY hab, double radOrbitAu, ORBITZONE orbitZone, const AuDice& Cdice)
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	AuDice dice(Cdice);
	//qDebug("DaWorld::Create hab %d orbitRad %g", hab, m_OrbitRadius);
	if(hab == HABITABLE_Ringworld)
	{
		// Just create the ringworld. Don't create any moons.
		DaWorldRingSection* ringRefSection = new DaWorldRingSection;
		ringRefSection->ConnectSignals();

		// Create first ringworld section.
		// The first section is designated section index 0. It is the reference section for positioning of all sections of the
		// ringworld. It is also the section responsible for creating the shadow squares object.
		double sections = DaWorldRingSection::RingworldSections(radOrbitAu);	// Number of sections so each has minimum length.
		ringRefSection->CreateFirstRingworldSectionForScan(dice, radOrbitAu, orbitZone);
		dice.Random();
		m_StarLock.lockForWrite();
		m_RingworldRefSection = ringRefSection->Id();
		m_StarLock.unlock();

		ringRefSection->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
		ether->AddAtom(ringRefSection);
		ringRefSection->SetNormal();	// New atom is ready for normal thread processing.

		// Create remainder of ring sections, beginning with index 1.
		for(int sectionIndex=1; sectionIndex<sections; sectionIndex++)
		{
			DaWorldRingSection* ringSection = new DaWorldRingSection;
			ringSection->ConnectSignals();
			ringSection->CreateRingworldSectionForScan(dice, sectionIndex, *ringRefSection);
			ringSection->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
			ether->AddAtom(ringSection);
			ringSection->SetNormal();	// New atom is ready for normal thread processing.
			dice.Random();
		}
		return;
	}

	// Check to see if the nu world will be a gas giant.
	int igg = dice.Roll(1,100);
	if(igg <= 5)
	{
		// Create an asteroid belt.
		int rocks = dice.Roll(2,3,-1);	// One to five planetoids. If more than five, then update DaWorldGlobe::DefaultName.
		for(int r=0; r<rocks; r++)
		{
			DaWorldGlobe* w = new DaWorldGlobe;
			w->ConnectSignals();
			w->CreateForScan(BODY_Planetoid, hab, dice, radOrbitAu, orbitZone, r);
			w->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
			ether->AddAtom(w);
			w->SetNormal();	// New atom is ready for normal thread processing.
			dice.Random();
		}
	}
	else if(igg <= 80)
	{
		// Create a solid spherical planet, not a gas giant.
		DaWorldGlobe* w = new DaWorldGlobe;
		w->ConnectSignals();
		w->CreateForScan(BODY_Planet, hab, dice, radOrbitAu, orbitZone, 0);
		w->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
		ether->AddAtom(w);
		w->SetNormal();	// New atom is ready for normal thread processing.
		dice.Random();

		int moons = w->GenMoonCount(dice);
		dice.Random();
		if(moons)
		{
			double radm = w->AtmosphereRadius() + FirstMoonOrbitAboveAtm();	// First available moon orbit radius;
			w->CreateMoonsForScan(moons, radm, false, dice, hab, orbitZone);
			dice.Random();
		}

		// Sometimes it is fun to have a twin planet in the habitable zone. Don't bother in other zones.
		if(orbitZone == OZONE_Habitable && dice.Roll(1,100) == 1)
		{
			// Create a twin world, a solid spherical planet, not a gas giant.
			DaWorldGlobe* tw = new DaWorldGlobe;
			tw->ConnectSignals();
			tw->CreateForScan(BODY_Planet, HABITABLE_Unspecified, dice, radOrbitAu, orbitZone, 1);
			tw->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
			ether->AddAtom(tw);
			tw->SetNormal();	// New atom is ready for normal thread processing.
			dice.Random();

			int moons = tw->GenMoonCount(dice);
			dice.Random();
			if(moons)
			{
				double radm = tw->AtmosphereRadius() + FirstMoonOrbitAboveAtm();	// First available moon orbit radius;
				tw->CreateMoonsForScan(moons, radm, false, dice, hab, orbitZone);
				dice.Random();
			}
		}
	}
	else
	{
		// Create a gas giant.
		DaWorldGasGiant* w = new DaWorldGasGiant;
		w->ConnectSignals();
		w->CreateForScan(dice, radOrbitAu, orbitZone);
		w->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
		ether->AddAtom(w);
		w->SetNormal();	// New atom is ready for normal thread processing.
		dice.Random();

		int moons = w->GenMoonCount(dice);
		dice.Random();
		if(moons)
		{
			// A titan is possible around every gas giant.
			bool hasTitan = false;
			if(orbitZone == OZONE_Habitable || dice.Roll(1,10) >= 8)
				hasTitan = true;
			double radm = w->AtmosphereRadius() + FirstMoonOrbitAboveAtm();	// First available moon orbit radius;
			w->CreateMoonsForScan(moons, radm, hasTitan, dice, hab, orbitZone);
		}
	}
}

// Returns 0 or a planet-sized body that is in the habitable orbit zone, with
// water between 20% and 80% and a breathable atmosphere.
DaWorldLL* DaStar::GetHabitableWorld(HABITABILITY hab) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaAtom* pAtom = *it;
		DaWorldLL* pPlanet = dynamic_cast<DaWorldLL*>(pAtom);
		if(pPlanet)
		{
			if(pPlanet->IsHabitable(hab))
				return pPlanet;	// The planet matches the desired habitability criteria.
			else if(pPlanet->IsHabitableOrbit())
			{
				// The planet is in a habitable orbit. Test for a moon
				// that meets the desired habitability criteria.
				DaWorldLL* pMoon = pPlanet->FindHabitableMoon(hab);
				if(pMoon)
					return pMoon;
			}
		}
		else
		{
			DaStar *pCompanion = dynamic_cast<DaStar*>(pAtom);
			if(pCompanion)
			{
				DaWorldLL* pPlanet = pCompanion->GetHabitableWorld(hab);
				if(pPlanet)
					return pPlanet;
			}
		}
	}
	return 0;
}

DaWorldGasGiant* DaStar::GetGasGiant() const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaAtom* pAtom = *it;
		DaWorldGasGiant* gg = dynamic_cast<DaWorldGasGiant*>(pAtom);
		if(gg)
			return gg;
		else
		{
			DaStar *pCompanion = dynamic_cast<DaStar*>(pAtom);
			if(pCompanion)
			{
				DaWorldGasGiant* gg = pCompanion->GetGasGiant();
				if(gg)
					return gg;
			}
		}
	}
	return 0;
}

DaWorldLL* DaStar::GetWorld(ORBITZONE ozone) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaAtom* pAtom = *it;
		DaWorldLL* world = dynamic_cast<DaWorldLL*>(pAtom);
		if(world && world->OrbitZone() == ozone)
			return world;
		else
		{
			DaStar *pCompanion = dynamic_cast<DaStar*>(pAtom);
			if(pCompanion)
			{
				DaWorldLL* world = pCompanion->GetWorld(ozone);
				if(world)
					return world;
			}
		}
	}
	return 0;
}

void DaStar::GetRenderLightsRecursive(const DaPerson& person, const AuMatrix4x4& model, QList<AuGLSun>& suns,
		QList<AuGLLight>& lights) const
{
	if(!IsAnyBlackHole())
	{
		// Configure the point light source.
		QColor colStar = LightColor();		// Select a color based on the star's color.
		float atten;						// Attenuation of light based on solar output.
		if(person.IsEyeUnderWater())
			atten = LightAttenuation(person.SeaDepth());	// Modulate light with sea depth.
		else
			atten = LightAttenuation(0);	// Attenuation of light based on solar output.

		AuPoint sloc = model.Translation();
		if(atten)
		{
			// The plane vectors are needed by the ringworld shaders to determine vertex locations relative to the sun.
			const DaWorldRingSection* rs = FindRingworldReferenceSection();
			if(rs)
			{
				// This sun is the center of a DaWorldRingSection ringworld.
				AuPoint xvec = model.ProjectVector(AuPoint(1,0));
				AuPoint yvec = model.ProjectVector(AuPoint(0,1));
				suns.append(AuGLSun(atten, colStar, sloc, (float)Radius(), xvec, yvec, rs->AngleOfRotation(),
							rs->RingworldSections()));
			}
			else
			{
				// This sun is not the center of a DaWorldRingSection ringworld.
				suns.append(AuGLSun(atten, colStar, sloc, (float)Radius()));
			}
		}
	}

	// Collect light sources for subordinate atom lights.
	DaAtom::GetRenderLightsRecursive(person, model, suns, lights);
}

// Renders this object in a scene.
// View transformation has already been set up.
void DaStar::RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass, DaAtomList& shadSc, DaAtomList& shadScFog,
		DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog, DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	//qDebug() << "DaStar::RenderEnvironmentObject" << this << Description() << "tempK" << m_SurfaceTempK;
	// Render atoms subordinate to this star.
	DaAtomList subs;
	ListSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		(*it)->RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog, shadScGlobe,
				shadScGlobeFog, shadScRingworld, shadScRingworldFog);

	// Stars need to be alpha blended so zsort order is important.
	const AuMatrix4x4& model = rmod.Matrix(Id());
	if(IsPrimary())
	{
		// The primary star displays all the stars in the solar system so they can be zsorted.
		DaStar* stars[3];			// Stars, farthest to nearest to camera.
		for (int i=0; i<3; i++)
			stars[i] = NULL;		// Initialize each pointer before use
		float zdepth[3] = {0,0,0};	// Stars, view depths from camera.
		AuMatrix4x4 ts[3];
		int istar=0;

		// Determine if this star is in front of the camera plane.
		const AuPoint& pv = rmod.ViewLoc(Id());	// Star loc relative to eye.
		double zd = pv.z - Radius();
		if(zd < 0.0 || IsAnyBlackHole())
		{
			// Star is in front of the camera or it's a black hole.
			stars[istar] = this;
			zdepth[istar] = zd;
			ts[istar] = model;
			istar++;
		}

		// Sort in the rest of the stars according to depth in the view.
		DaAtomList subs;
		ListNormalSubatoms(subs);
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend() && istar <= 2; it++)
		{
			DaStar* pAtom = dynamic_cast<DaStar*>(*it);
			if(!pAtom)
				continue;

			const AuMatrix4x4& tSub = rmod.Matrix(pAtom->Id());
			const AuPoint& pv = rmod.ViewLoc(pAtom->Id());	// Star loc relative to env
			zd = pv.z - pAtom->Radius();
			if(zd < 0 || pAtom->IsAnyBlackHole())
			{
				// Star is in front of the camera or it's a black hole.
				if(istar == 1)
				{
					// This is the second star found.
					if(zd < zdepth[0])
					{
						// pv is farther from the camera than entry 0.
						// Insert at entry 0.
						stars[1] = stars[0];
						zdepth[1] = zdepth[0];
						ts[1] = ts[0];
						stars[0] = (DaStar*)pAtom;
						zdepth[0] = zd;
						ts[0] = tSub;
					}
					else
					{
						// pv is closer to the camera than entry 0.
						// Add to the end.
						stars[1] = (DaStar*)pAtom;
						zdepth[1] = zd;
						ts[1] = tSub;
					}
				}
				else
				{
					// This is the third star.
					if(zd < zdepth[0])
					{
						// pv is farther from the camera than entry 0.
						// Insert at entry 0.
						stars[2] = stars[1];
						zdepth[2] = zdepth[1];
						ts[2] = ts[1];
						stars[1] = stars[0];
						zdepth[1] = zdepth[0];
						ts[1] = ts[0];
						stars[0] = (DaStar*)pAtom;
						zdepth[0] = zd;
						ts[0] = tSub;
					}
					else if(zd < zdepth[1])
					{
						// pv is farther from the camera than entry 1.
						// Insert at entry 1.
						stars[2] = stars[1];
						zdepth[2] = zdepth[1];
						ts[2] = ts[1];
						stars[1] = (DaStar*)pAtom;
						zdepth[1] = zd;
						ts[1] = tSub;
					}
					else
					{
						// pv is closer to the camera than entries 0 and 1.
						// Add to the end.
						stars[2] = (DaStar*)pAtom;
						zdepth[2] = zd;
						ts[2] = tSub;
					}
				}
				istar++;
			}
		}

		// Display the suns.
		for(int star=0; star<istar; star++)
		{
			if(!stars[star])
				continue;
			if(!rmod.RenderThisPass(pass, fabs(zdepth[star])))
				continue;	// Skip based on scene depth in pass.

			if(stars[star]->IsAnyBlackHole())
				stars[star]->RenderBlackHole(projection, ts[star], *ari.re, sceneAtom);
			else if(stars[star]->IsNeutronStar())
				stars[star]->RenderNeutronStar(projection, ts[star], *ari.re);
			else
			{
				float brightness = m_Stargates.isEmpty() ? 1.f : .5f;	// Dim when stargate is active.
				stars[star]->RenderSun(projection, ts[star], *ari.re, brightness);
			}
		}
	}
	else
	{
		// This is a companion star.
		if(pass == PASS_Far && sceneAtom->CanSeeOrbitRings() && ari.m_ShowHUD)
		{
			// Draw orbit ring through companion stars in the far pass.
			ari.re->GLSL().LineShader().Select();
			ari.re->GLSL().LineShader().SetProjectionModelView(projection * rmod.Matrix(ParentId()));
			ari.re->GLSL().LineShader().SetColor(QColor(96,96,192));

			// Compile a list of vertex locations.
			AuPoint poleVec = Transform().ProjectVector(0,0,Radius()*1.5);
			AuPointArray verts;
			AuPoint start(Location());
			verts.append(start + poleVec);
			verts.MakeCircle(AuPoint(0,0), start, 128);
			verts.append(verts[1]);
			verts.append(start - poleVec);

			// Make a VAO of the vertex locations.
			AuGLVAO vao;
			vao.CreateForLines(*ari.re, verts, true, GL_STREAM_DRAW);

			// Display the VAO once and clean it up.
			vao.SelectGL();
			vao.DrawGL(GL_LINE_STRIP);
			vao.DeselectGL();
			vao.DeleteGL();

			ari.re->GLSL().LineShader().Deselect();
		}
	}

	// Render stargate energy beams only in the far pass.
	if(pass == PASS_Far && IsRingworldStar())
	{
		for(AuStargateList::const_iterator gi = m_Stargates.cbegin(); gi != m_Stargates.cend(); gi++)
		{
			if(!(*gi).IsOrigin())
				continue;
			// Render beam effects from me to the cooling towers on the ringworld arc sections.
			if(!m_PSStargateBeam)
			{
				const DaWorldRingSection* refSection = FindRingworldReferenceSection();
				double distance = refSection->OrbitRadius() - Radius()*.8;
				double velocity = 100000*FeetPerMeter();
				double lifespan = distance / velocity;

				GLuint texId = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "WAc", ":data/acidsprite.png");
				m_PSStargateBeam = new PsWeaponEffect();
				m_PSStargateBeam->SetVelocityVar(0);
				m_PSStargateBeam->SetMaxParticles(1000);
				m_PSStargateBeam->SetLifeCycle(lifespan);
				m_PSStargateBeam->SetNumToRelease(1);
				m_PSStargateBeam->SetReleaseInterval(0.0f);
				m_PSStargateBeam->SetSize(velocity);
				m_PSStargateBeam->SetTextureID(texId);
				m_PSStargateBeam->SetTextureID2(0);
				m_PSStargateBeam->SetVelocity(AuPoint(0, 0, velocity));
				m_PSStargateBeam->SetLength(distance);	// my own length so I don't go past the target
				m_PSStargateBeam->SetTargetLoc(AuPoint(0,0,distance));
				m_PSStargateBeam->SetColor(LightColor());
			}

			if(m_PSStargateBeam->Update())
			{
				DaAtomList subs;
				ListNormalSubatoms(subs);
				for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
				{
					DaWorldRingSection* ringSection = dynamic_cast<DaWorldRingSection*>(*it);
					if(!ringSection)
						continue;

					AuPoint coneLoc = ringSection->LocOfLL(AuGlobeLL(-PI,0), 100);
					AuMatrix4x4 beamM(rmod.Matrix(ringSection->Id()));
					beamM.Translate(coneLoc);
					beamM.RotateY(ringSection->SectionAngle()/2);

					double hullwid = ringSection->TerrainYSize();
					beamM.Translate(0, hullwid*3/8, 0);
					m_PSStargateBeam->Render(projection, beamM, *ari.re);

					hullwid /= 4;
					for(int beam=0; beam<3; beam++)
					{
						beamM.Translate(0,-hullwid,0);
						m_PSStargateBeam->Render(projection, beamM, *ari.re);
					}
				}
			}
		}
	}

	// Render stargates.
	for(AuStargateList::const_iterator gi = m_Stargates.cbegin(); gi != m_Stargates.cend(); gi++)
	{
		if((*gi).IsOrigin())
		{
			// Render a positive stargate wormhole effect.
			AuWormhole whg = AuWormhole("", AuWormhole::StargatePositive, (*gi).ToGalaxy(), (*gi).ToGalaxyLoc());
			whg.SetStarDistance(Radius()*1.1, AuPoint(0,0));
			AuPoint wloc = (model * whg.Transform()).Translation();
			if(rmod.RenderThisPass(pass, wloc.Length()))
				whg.Render(projection, model, *ari.re, ari.Models());
		}
		else
		{
			// Render a negative stargate wormhole effect.
			AuWormhole whg = AuWormhole("", AuWormhole::StargateNegative, (*gi).ToGalaxy(), (*gi).ToGalaxyLoc());
			whg.SetStarDistance(Radius()*1.1, AuPoint(0,0));
			AuPoint wloc = (model * whg.Transform()).Translation();
			if(rmod.RenderThisPass(pass, wloc.Length()))
				whg.Render(projection, model, *ari.re, ari.Models());
		}
	}

	// Base atom class needs to render special effects.
	DaAtom::RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, pass, shadSc, shadScFog, shadScGlobe,
			shadScGlobeFog, shadScRingworld, shadScRingworldFog);
}

// Display this sun.
void DaStar::RenderBlackHole(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, const DaPerson* sceneAtom)
{
	if(!m_SolarDisk.VAO())
		return;
	if(!m_CoreSphere.VAO())
		return;
	if(!m_CoronaSphere.VAO())
		return;

	const DaSector* sector = FindParentSector();
	if(!sector || !sector->Parent())
		return;

	// Orient flat star to face viewer.
	// Calculate the vector from the center of the sun to the eye, in sun space not eye space.
	// This calculation eliminates the tipping effect of the sun but makes it hard to fly near the sun
	// without getting destroyed.
	//AuPoint vup = ts[star].UnProject(0,0,0);	// Project the eye point from eye space into sun space.
	//vup.Unit();										// Unit vector in sun space from center of sun to the eye.
	// Orienting the sun flat to the camera plane makes flying near the sun easier but gives the sun a tipping
	// effect when looking around while near the sun. The tipping sun give a more accurate sense of where the edge
	// of the sun really is, like a rotating cross-section of the sun's sphere.
	AuPoint vup = model.UnProjectVector(AuPoint(0,0,1));
	vup.Unit();
	AuMatrix4x4 ta;
	{
		// This code was whittled down from AuPoint::GetAlignmentTransform. We know the vectors are unit vectors so
		// the math here is a lot simpler.
		// Determine normal vector to the two vectors.
		AuPoint vN = vup.Cross(AuPoint(0,0,1));
		if(!vN.IsZero())
		{
			// Rotate the transformation matrix around the normal vector by the angle between the vectors.
			vN.Unit();
			double rAng = vup.AngleTo(AuPoint(0,0,1));
			ta.RotateAxis(-rAng, vN);
		}
	}
	// The 2D angle of the eye space Z vector tells us how much to rotate the sun polygon around its Z axis so the
	// sun stays consistently aligned with the eye.
	ta.RotateZ(vup.ArcTangent_2D());
	AuMatrix4x4 mv = model * ta;

	// Rotate the refraction according to the person's current view direction relative to the galaxy.
	AuMatrix4x4 ms;
	sceneAtom->GetTransformRelativeToParent(ms, sector->Parent());
	ms.Rotate(m_SphereRot);
	AuMatrix4x4 mref;
	mref.Rotate(ms.Inverse().GetRotation());		// Inverse of person's look matrix aligns refraction vectors to the sky box.

	re.glDepthFunc(GL_LEQUAL);										// Prepare to blend additional passes at the same Z-depth.
	if(!sceneAtom->InBlackHole())
	{
		// Render the black hole sprite with its refraction of the sky box.
		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);		// Black hole disk blends with destination pixels at the edges.
		re.glDepthMask(GL_FALSE);									// Turn off depth mask.

		re.GLSL().BlackHoleShader().Select();
		re.GLSL().BlackHoleShader().SetProjectionModelView(projection * mv);
		re.GLSL().BlackHoleShader().SetTexture(m_TexSun);
		re.GLSL().BlackHoleShader().SetSkyBoxTexture(sector->SkyBoxTexture());
		re.GLSL().BlackHoleShader().SetSkyBoxRotation(mref);

		m_SolarDisk.SelectGL();
		m_SolarDisk.DrawGL();
		m_SolarDisk.DeselectGL();

		re.GLSL().BlackHoleShader().Deselect();

		re.glDepthMask(GL_TRUE);
		re.glDisable(GL_BLEND);
	}

	// Render black hole sphere model.
	re.glDisable(GL_CULL_FACE);									// So we don't see through it at great distances.

	re.GLSL().BlackHoleSphereShader().Select();
	re.GLSL().BlackHoleSphereShader().SetProjectionModelView(projection * model);

	m_CoreSphere.SelectGL();
	m_CoreSphere.DrawGL();
	m_CoreSphere.DeselectGL();

	re.GLSL().BlackHoleSphereShader().Deselect();

	re.glEnable(GL_CULL_FACE);

	if(!sceneAtom->InBlackHole())
	{
		// Render photon sphere.
		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);						// Additive blend for the photon sphere.
		re.glDepthMask(GL_FALSE);									// Turn off depth mask.

		re.GLSL().PhotonSphereShader().Select();
		re.GLSL().PhotonSphereShader().SetTexture(m_TexNoise);

		m_CoronaSphere.SelectGL();

		if(sceneAtom->InPhotonSphere())
			re.glFrontFace(GL_CCW);								// Render inside of sphere model.

		// Add several rotating cloudy textures for a gaseous movement effect.
		// CAVEAT: The shader multiplies the alpha by .5, expecting 2 passes here.
		for(int i=0; i<2; i++)
		{
			AuMatrix4x4 mps(model);
			mps.RotateZ(m_fRotation[i]);
			re.GLSL().PhotonSphereShader().SetProjectionModelView(projection * mps);
			m_CoronaSphere.DrawGL();
		}

		if(sceneAtom->InPhotonSphere())
			re.glFrontFace(GL_CW);									// Restore default.

		m_CoronaSphere.DeselectGL();

		re.GLSL().PhotonSphereShader().Deselect();

		re.glDepthMask(GL_TRUE);
		re.glDisable(GL_BLEND);
	}
	re.glDepthFunc(GL_LESS);
}

// Display this sun.
void DaStar::RenderSun(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float brightness)
{
	if(!m_SolarDisk.VAO())
		return;
	if(!m_CoreSphere.VAO())
		return;
	if(!m_CoronaSphere.VAO())
		return;

	// Orient flat star to face viewer.
	// Calculate the vector from the center of the sun to the eye, in sun space not eye space.
	// This calculation eliminates the tipping effect of the sun but makes it hard to fly near the sun
	// without getting destroyed.
	//AuPoint vup = ts[star].UnProject(0,0,0);	// Project the eye point from eye space into sun space.
	//vup.Unit();										// Unit vector in sun space from center of sun to the eye.
	// Orienting the sun flat to the camera plane makes flying near the sun easier but gives the sun a tipping
	// effect when looking around while near the sun. The tipping sun give a more accurate sense of where the edge
	// of the sun really is, like a rotating cross-section of the sun's sphere.
	AuPoint vup = model.UnProjectVector(AuPoint(0,0,1));
	vup.Unit();
	AuMatrix4x4 ta;
	{
		// This code was whittled down from AuPoint::GetAlignmentTransform. We know the vectors are unit vectors so
		// the math here is a lot simpler.
		// Determine normal vector to the two vectors.
		AuPoint vN = vup.Cross(AuPoint(0,0,1));
		if(!vN.IsZero())
		{
			// Rotate the transformation matrix around the normal vector by the angle between the vectors.
			vN.Unit();
			double rAng = vup.AngleTo(AuPoint(0,0,1));
			ta.RotateAxis(-rAng, vN);
		}
	}

	// Color for glowing aura.
	QColor sunColor(SunColor());
	float ar = sunColor.redF() * brightness;
	if(ar > 1)
		ar = 1;
	float ag = sunColor.greenF() * brightness;
	if(ag > 1)
		ag = 1;
	float ab = sunColor.blueF() *  brightness;
	if(ab > 1)
		ab = 1;

	// Color for blended layers.
	float r = ar * .8f;
	float g = ag * .8f;
	float b = ab * .8f;

	re.GLSL().TextureShader().Select();
	re.GLSL().TextureShader().SetTexture(m_TexSun);

	// Glowing aura and cloudy corona
	{
		// The 2D angle of the eye space Z vector tells us how much to rotate the sun polygon around its Z axis so the
		// sun stays consistently aligned with the eye.
		ta.RotateZ(vup.ArcTangent_2D());
		AuMatrix4x4 mv = model * ta;

		re.glDepthMask(GL_FALSE);							// Turn off depth mask.
		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);				// Remaining shader output is added to destination.
		re.glDepthFunc(GL_LEQUAL);							// Prepare to blend additional passes at the same Z-depth.
		re.glDisable(GL_CULL_FACE);

		// Add the sun glow that is larger than the solar disk.
		AuMatrix4x4 mvGlow(mv);
		mvGlow.Scale(2.5, 2.5, 2.5);
		re.GLSL().TextureShader().SetProjectionModelView(projection * mvGlow);
		re.GLSL().TextureShader().SetColor(ar,ag,ab);
		m_SolarDisk.SelectGL();
		m_SolarDisk.DrawGL();

		// Add several rotating cloudy sprites that bisect the model, for a gaseous corona effect.
		re.GLSL().TextureShader().SetColor(r/2, g/2, b/2);
		for(int i=0; i<4; i++)
		{
			re.GLSL().TextureShader().SetTexture(m_TexCorona[i]);

			AuMatrix4x4 mvDisc(model);
			mvDisc.Scale(1.15, 1.15, 1.15);
			mvDisc.RotateZ(m_fRotation[i]);
			re.GLSL().TextureShader().SetProjectionModelView(projection * mvDisc);

			m_SolarDisk.DrawGL();

			mvDisc.RotateY(PI/2);
			re.GLSL().TextureShader().SetProjectionModelView(projection * mvDisc);

			m_SolarDisk.DrawGL();

			mvDisc.RotateX(PI/2);
			re.GLSL().TextureShader().SetProjectionModelView(projection * mvDisc);

			m_SolarDisk.DrawGL();
		}

		m_SolarDisk.DeselectGL();

		re.glDepthMask(GL_TRUE);
		re.glDisable(GL_BLEND);
	}

	re.GLSL().TextureShader().SetTexture(m_TexNoise);

	// Core
	{
		// Render star sphere model.
		AuMatrix4x4 mps(model);
		mps.Rotate(m_SphereRot);

		re.GLSL().TextureShader().SetProjectionModelView(projection * mps);
		re.GLSL().TextureShader().SetColor(r,g,b);

		m_CoreSphere.SelectGL();
		m_CoreSphere.DrawGL();
		m_CoreSphere.DeselectGL();

		re.glEnable(GL_CULL_FACE);
	}

	// Corona shells
	{
		// Render corona.
		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);						// Additive blend for the photon sphere.
		re.glDepthMask(GL_FALSE);									// Turn off depth mask.
		re.glEnable(GL_POLYGON_OFFSET_FILL);
		re.glPolygonOffset(-1.f, -1.f);

		m_CoronaSphere.SelectGL();

		// Add some rotating textures for a field effect.
		for(int i=0; i<2; i++)
		{
			AuMatrix4x4 mps(model);
			mps.Rotate(m_SphereRot);
			mps.RotateZ(m_fRotation[i]);

			if(i)
			{
				mps.RotateX(PI);
				double scl = 1.01;
				mps.Scale(scl,scl,scl);
			}
			else
				mps.RotateX(PI*2/3);
			re.GLSL().TextureShader().SetProjectionModelView(projection * mps);
			m_CoronaSphere.DrawGL();
		}

		m_CoronaSphere.DeselectGL();

		re.glDepthMask(GL_TRUE);
		re.glDisable(GL_BLEND);
		re.glDisable(GL_POLYGON_OFFSET_FILL);
	}
	re.glDepthFunc(GL_LESS);

	re.GLSL().TextureShader().Deselect();
}

void DaStar::CreateStarModel(AuOpenGLFunctions& GLf)
{
	// Calculate the radius of the body.
	float radStar = Radius();			// Radius in feet.
	if(IsAnyBlackHole())
	{
		CreateBlackHoleSphereModel(GLf);
		CreatePhotonSphereModel(GLf);
		radStar *= 4;					// Radius of black hole model must include refraction zone.
	}
	else if(IsNeutronStar())
	{
		CreateNeutronStarSphereModel(GLf);
		CreateNeutronSphereModel(GLf);
		radStar *= 3;					// Radius of neutron star glow is larger than the neutron star.
	}
	else
	{
		CreateStarSphereModel(GLf);
		CreateCoronaSphereModel(GLf);
	}

	// Draw an octagon using six triangles.
	// sunPos array contains vertices for triangles that make up the octagon. Each row is a triangle.
	GLfloat a = radStar;
	GLfloat b = tanf(22.5f * DEG2RADf) * radStar;
	GLfloat sunPos[6*3*3] =
	{
		-a,b,0, -b,a,0, b,a,0,
		b,a,0, a,b,0, -a,b,0,
		-a,b,0, a,b,0, -a,-b,0,
		-a,-b,0, a,b,0, a,-b,0,
		a,-b,0, b,-a,0, -b,-a,0,
		-b,-a,0, -a,-b,0, a,-b,0,
	};

	// sunTex array contains texture coordinates for each of the triangles in the sunPos array.
	GLfloat ta = 0.5f - 0.382683432f/2.f;	//sinf(22.5f * DEG2RADf)/2;
	GLfloat tb = 0.5f + 0.382683432f/2.f;	//sinf(22.5f * DEG2RADf)/2;
	GLfloat sunTex[6*3*2] =
	{
		0,tb, ta,1, tb,1,
		tb,1, 1,tb, 0,tb,
		0,tb, 1,tb, 0,ta,
		0,ta, 1,tb, 1,ta,
		1,ta, tb,0, ta,0,
		ta,0, 0,ta, 1,ta,
	};

	int vertices = 6*3;
	m_SolarDisk.CreateGL(GLf, vertices, 2);

	m_SolarDisk.SelectGL();

	// bind buffer for vertices and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_SolarDisk.Buffer(0));
	GLf.glBufferData(GL_ARRAY_BUFFER, sizeof(sunPos), sunPos, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for texture coordinates and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_SolarDisk.Buffer(1));
	GLf.glBufferData(GL_ARRAY_BUFFER, sizeof(sunTex), sunTex, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// Disable the vertex array object.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

	m_SolarDisk.DeselectGL();
}

void DaStar::CreateBlackHoleSphereModel(AuOpenGLFunctions& GLf)
{
	if(m_CoreSphere.VAO())
		return;

	// Create a vertex array object to be used for our black hole sphere.
	// Tessellate a globe for the atmosphere. This tessellation only expresses latitude and longitude, not altitude of points.
	AuGlobeLLTriList psTris;
	psTris.CreateGlobeTriangles(200);

	// Turn the globe tessellation into globe surface triangles with 3D coordinates.
	double rad = Radius();
	QList<AuGlobeTriTex> tris;							// Triangles that make up this model.
	for(AuGlobeLLTriList::const_iterator ti = psTris.cbegin(); ti != psTris.cend(); ti++)
	{
		// Calculate triangle vertex coordinates.
		AuPoint terCoord0 = (*ti).m_LL0.GlobeLocation(rad);		// Location at black hole sphere radius.
		AuPoint terCoord1 = (*ti).m_LL1.GlobeLocation(rad);		// Location at black hole sphere radius.
		AuPoint terCoord2 = (*ti).m_LL2.GlobeLocation(rad);		// Location at black hole sphere radius.

		// Construct a globe triangle.
		AuGlobeTriTex tri;
		tri.m_Vert0 = AuGlobeVertTex(terCoord0, QPointF(0,0));
		tri.m_Vert1 = AuGlobeVertTex(terCoord1, QPointF(0,0));
		tri.m_Vert2 = AuGlobeVertTex(terCoord2, QPointF(0,0));

		// Append the globe triangle to the list.
		tris.append(tri);
	}

	// Store count of triangles in the black hole sphere vertex array object.
	GLuint vertices = tris.count()*3;

	// Copy surface triangles into gl array buffers.
	GLfloat* globePos = new GLfloat[vertices*3];			// vertices for triangles that make up the model.

	unsigned int globePosIdx = 0;
	for(QList<AuGlobeTriTex>::const_iterator vi = tris.cbegin(); vi != tris.cend(); vi++)
	{
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.z;

		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.z;

		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.z;
	}

	// Generate vertex array(s) for this object. We only need one.
	m_CoreSphere.CreateGL(GLf, vertices, 1);

	// Select vertex array to construct.
	m_CoreSphere.SelectGL();

	// bind buffer for vertices and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_CoreSphere.Buffer(0));
	GLf.glBufferData(GL_ARRAY_BUFFER, globePosIdx*sizeof(GLfloat), globePos, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Disable the vertex array object.
	m_CoreSphere.DeselectGL();

	delete [] globePos;
}

void DaStar::CreatePhotonSphereModel(AuOpenGLFunctions& GLf)
{
	if(m_CoronaSphere.VAO())
		return;

	// Create a vertex array object to be used for our photon sphere.
	// Tessellate a globe for the atmosphere. This tessellation only expresses latitude and longitude, not altitude of points.
	AuGlobeLLTriList psTris;
	psTris.CreateGlobeTriangles(200);

	// Turn the globe tessellation into globe surface triangles with 3D coordinates and texture coordinates.
	double rad = PhotonSphereRadius();
	QList<AuGlobeTriTex> tris;							// Triangles that make up this model.
	for(AuGlobeLLTriList::const_iterator ti = psTris.cbegin(); ti != psTris.cend(); ti++)
	{
		// Calculate triangle vertex coordinates.
		AuPoint terCoord0 = (*ti).m_LL0.GlobeLocation(rad);		// Location at photon sphere radius.
		AuPoint terCoord1 = (*ti).m_LL1.GlobeLocation(rad);		// Location at photon sphere radius.
		AuPoint terCoord2 = (*ti).m_LL2.GlobeLocation(rad);		// Location at photon sphere radius.

		// Wrap texture properly at the seam.
		float ll0long = (*ti).m_LL0.Longitude();			// Range is -PI to PI
		if(FuzzyEquals((float)(*ti).m_LL0.Longitude(), -PIf) && ((*ti).m_LL1.Longitude() > 0 || (*ti).m_LL2.Longitude() > 0))
			ll0long = PIf;
		float ll1long = (*ti).m_LL1.Longitude();
		if(FuzzyEquals((float)(*ti).m_LL1.Longitude(), -PIf) && ((*ti).m_LL0.Longitude() > 0 || (*ti).m_LL2.Longitude() > 0))
			ll1long = PIf;
		float ll2long = (*ti).m_LL2.Longitude();
		if(FuzzyEquals((float)(*ti).m_LL2.Longitude(), -PIf) && ((*ti).m_LL0.Longitude() > 0 || (*ti).m_LL1.Longitude() > 0))
			ll2long = PIf;

		// Calculate texture coordinates.
		QPointF terTex0;
		terTex0.setX(ll0long/(PIf*2));						// Range is -.5 to .5. -.5 is -PI, 0 is 0, .5 is PI.
		terTex0.setY(((*ti).m_LL0.Latitude() + PIf/2)/PIf);	// Range is 1 to 0. 1 is at the north pole, 0 is at the south pole.
		QPointF terTex1;
		terTex1.setX(ll1long/(PIf*2));
		terTex1.setY(((*ti).m_LL1.Latitude() + PIf/2)/PIf);
		QPointF terTex2;
		terTex2.setX(ll2long/(PIf*2));
		terTex2.setY(((*ti).m_LL2.Latitude() + PIf/2)/PIf);

		// Construct a globe triangle with texture coordinates.
		AuGlobeTriTex tri;
		tri.m_Vert0 = AuGlobeVertTex(terCoord0, terTex0);
		tri.m_Vert1 = AuGlobeVertTex(terCoord1, terTex1);
		tri.m_Vert2 = AuGlobeVertTex(terCoord2, terTex2);

		// Append the globe triangle to the list.
		tris.append(tri);
	}

	// Store count of triangles in the photon sphere vertex array object.
	GLuint vertices = tris.count()*3;

	// Copy surface triangles into gl array buffers.
	GLfloat* globePos = new GLfloat[vertices*3];			// vertices for triangles that make up the model.
	GLfloat* globeTex = new GLfloat[vertices*2];			// texture coords for each of the vertices

	unsigned int globePosIdx = 0;
	unsigned int globeTexIdx = 0;
	for(QList<AuGlobeTriTex>::const_iterator vi = tris.cbegin(); vi != tris.cend(); vi++)
	{
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.z;

		globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.x();
		globeTex[globeTexIdx++] = (*vi).m_Vert0.m_TexCoord.y();

		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.z;

		globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.x();
		globeTex[globeTexIdx++] = (*vi).m_Vert1.m_TexCoord.y();

		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.z;

		globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.x();
		globeTex[globeTexIdx++] = (*vi).m_Vert2.m_TexCoord.y();
	}

	// Generate vertex array(s) for this object. We only need one.
	m_CoronaSphere.CreateGL(GLf, vertices, 2);

	// Select vertex array to construct.
	m_CoronaSphere.SelectGL();

	// bind buffer for vertices and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_CoronaSphere.Buffer(0));
	GLf.glBufferData(GL_ARRAY_BUFFER, globePosIdx*sizeof(GLfloat), globePos, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for colors and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_CoronaSphere.Buffer(1));
	GLf.glBufferData(GL_ARRAY_BUFFER, globeTexIdx*sizeof(GLfloat), globeTex, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Disable the vertex array object.
	m_CoronaSphere.DeselectGL();

	delete [] globePos;
	delete [] globeTex;
}

float DaStar::NeutronStarOutput() const
{
	// Every neutron star pulses in a unique rhythm.
	quint32 g = GenSeed();	// Bit pattern we will blink.
	int cycleMs = 500 + (Id().GetID() & 0x3fff);
	float cycle = AuTime::CurrentCycle(cycleMs);
	int bit = (int)floorf(cycle * 32);
	if(bit == 32)
		bit = 0;
	if(g & (1 << bit))
	{
		m_NeutronStarOutput += .2f;
		if(m_NeutronStarOutput > 1)
			m_NeutronStarOutput = 1;
	}
	else
	{
		m_NeutronStarOutput -= .05f;
		if(m_NeutronStarOutput < .1f)
			m_NeutronStarOutput = .1f;
	}
	return m_NeutronStarOutput;
}

float DaStar::LightAttenuation(double seaDepth) const
{
	// Let's assume optimum brightness is at the radius of the habitable zone.
	double rLightRadius;
	if(IsNeutronStar())
		rLightRadius = OrbitRadius(10) * FeetPerAu() * NeutronStarOutput();	// Pulses with state.
	else
		rLightRadius = OrbitRadius(HabitableZone()) * FeetPerAu();

	// Sunlight is dimmed under the sea.
	if(seaDepth > 0)
	{
		// Calculate light level that drops off slowly then more quickly
		double fx = seaDepth/SeaDysphoticDepth();	// 0 at surface to 1 at dysphotic depth.
		if(fx > 1)
			return 0;				// No light at dysphotic depth and below.
		double ff = fx * 9 + 1;		// In the range of 1 to 10.
		double fy = log10(ff);		// Use logorithm curve to get a number from 0 to 1.
		double fillum = 1 - fy;		// Reverse so fillum drops off with depth.

		//double fillum = 1 - seaDepth/SeaDysphoticDepth();	// Linear dropoff.
		if(fillum <= 0)
			return 0;			// No sunlight. Shouldn't be possible. This formula should always calculate a tiny bit.
		rLightRadius *= fillum;
		//qDebug() << "DaStar::LightAttenuation" << *this << "seaDepth" << (seaDepth/FeetPerMeter()) << "m fillum" << fillum
			//<< "fy" << fy << "ff" << ff << "fx" << fx;
	}

	// Sunlight is dimmed when stargate is open.
	m_StarLock.lockForRead();
	if(!m_Stargates.isEmpty())
		rLightRadius /= 2;
	m_StarLock.unlock();

	// Calculate attenuation, which is 1/(lightRadius^2).
	rLightRadius *= rLightRadius;
	return (float)(1/rLightRadius);
}

// This calculates the angle to the star's current position based on real time.
// The time scale is modified such that four days in real time are equal to an earth 
// year in the model Universe.
// This results in a standard earth day of about 15.77 minutes in real time.
// All stars and planets orbit in the same direction.
double DaStar::AngleToCompanionStar() const
{
	double rRealDays = (double)g_Au.CoordinatedTime().MSecsSinceEpoch() / (double)MillisecondsPerDay();
	double rUYears = rRealDays / RealDaysPerUYear();			// This is how many years have passed in the game universe.
	double rPeriods = rUYears / Period();						// Number of periods of revolution.

	// Skew by a constant factor based on this star's GenSeed. This way all stars don't start at angle zero.
	rPeriods += AuNoise::IntegerNoise(0, GenSeed());

	// Calculate the orbit angle from the fractional period that we're in.
	double rFracPeriods = rPeriods - (int)rPeriods;
	double rAngle = rFracPeriods * PI*2.0;						// Angle to current position of this body.
	return rAngle;
}

// This is an approximation of the period based roughly on Keppler's formula:
//	t^2 = r^3 where t is time and r is radius.
// The return result is in earth years.
double DaStar::Period() const
{
	double r = OrbitRadius(m_chOrbit);	// orbit radius in au
	double r3 = r * r * r;			// r^3
	double rYear = sqrt(r3);
	return rYear;
}

void DaStar::CreateDeviceObjects(AuAtomRenderInfo& ari)
{
	DaAtom::CreateDeviceObjects(ari);
	if(!m_SolarDisk.VAO())
	{
		CreateRenderTexture(*ari.re);	// Create the render texture cache objects.
		CreateStarModel(*ari.re);		// Star model.
	}
}

void DaStar::DeleteDeviceObjects(AuAtomRenderInfo& ari)
{
	DaAtom::DeleteDeviceObjects(ari);

	if(m_CoreSphere.VAO())
		m_CoreSphere.DeleteGL();

	if(m_CoronaSphere.VAO())
		m_CoronaSphere.DeleteGL();

	if(m_SolarDisk.VAO())
		m_SolarDisk.DeleteGL();

	for(int i=0; i<4; i++)
		m_TexCorona[i] = 0;
	m_TexLightning = 0;
	m_TexNoise = 0;
	m_TexSun = 0;
}

// Create a texture map for the surface rendering.
void DaStar::CreateRenderTexture(AuGLEffects& re)
{
	Q_ASSERT(!m_TexSun);
	if(IsAnyBlackHole())
	{
		m_TexSun = re.UseTexture(Id().GetID(), "BlackHole");
		if(!m_TexSun)
		{
			// Create the black hole spacial distortion texture.
			int bhwid = 2048;
			int bhht = bhwid;
			int bhsize = bhwid * bhht;
			bhsize *= 4;					// 4 floats at each pixel form a vector with an alpha value
			float* bhdata = new float[bhsize];
			int cx = bhwid / 2;				// Center X coord
			int cy = bhht / 2;				// Center Y coord
			int sr = cx / 4;				// Schwarzschild radius is 1/4 the distance from the center to the edge of the texture.
			int rz = cx - sr - 1;			// Refraction zone is from sr to edge, minus a pixel.

			float* pbh = bhdata;
			for(int ix=0; ix<bhwid; ix++)
			{
				for(int iy=0; iy<bhht; iy++)
				{
					// Calculate distance from center of texture.
					int dx = ix - cx;
					int dy = iy - cy;
					float dist = sqrtf(dx*dx + dy*dy);

					// Pixel is in the refraction zone.
					// Calculate surface normal based on distance from edge of event horizon.
					float refdist = dist - sr;					// Negative inside the event horizon.
					float ratio = refdist / rz;					// 0 at the event horizon to 1 at the radius of the refraction zone.
					if(ratio > 1.f)
						ratio = 1.f;
					else if(ratio < -1.f)
						ratio = -1.f;
					float angle = acosf(ratio);					// Angle of the refraction vector pointing toward the center.
																// At the edges, vector angle is 0, straight up.
					float alpha;
					if(ratio > 0)
						alpha = 1.f - ratio;
					else
					{
						// Drop alpha off quickly inside so the sprite doesn't bleed through the black hole model at great distance.
						alpha = 1.f + ratio*10.f;
						if(alpha < 0.f)
							alpha = 0.f;
					}

					// Calculate refraction normal.
					AuPoint rn = AuPoint(0,0,-1).RotateY(angle);
					float angZ = atan2f((float)dy, (float)dx);
					rn = rn.RotateZ(angZ);
					*pbh = (float)rn.x/2.f + .5f;
					pbh++;
					*pbh = (float)rn.y/2.f + .5f;
					pbh++;
					*pbh = (float)rn.z/2.f + .5f;
					pbh++;
					*pbh = alpha;
					pbh++;
				}
			}

			m_TexSun = re.CreateTextureFromFloats(Id().GetID(), "BlackHole", bhdata, bhwid, bhht, false, 1, GL_RGBA);

			delete [] bhdata;
		}
	}
	else if(IsNeutronStar())
	{
		// Load texture used for glow around a neutron star.
		m_TexSun = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "SunGlow", ":data/txSunGlow.bmp", GL_LINEAR, GL_LINEAR, false, 2);

		m_TexLightning = re.CreateTextureFromBitmap(Id().GetID(), "NeutronSphere", ":data/LightningSeamless.png",
				GL_LINEAR, GL_LINEAR, false, 2);
	}
	else
	{
		// Load texture used for glow around a star.
		m_TexSun = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "SunGlow", ":data/txSunGlow.bmp", GL_LINEAR, GL_LINEAR, false, 2);

		// Load textures used for cloudy corona around stars.
		GLuint tex[8] = {0,0,0,0,0,0,0,0};
		for(int i=1; i<=7; i++)
		{
			QString stid = QString("Sun%1").arg(i);
			QString sfile = QString(":data/txSun%1.bmp").arg(i);
			tex[i] = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, stid, sfile, GL_LINEAR, GL_LINEAR, false, 2);
		}

		// Decide which of the corona textures this star uses, and in what order.
		AuDice dice;
		dice.Seed(GenSeed());
		for(int i=0; i<4; i++)
			m_TexCorona[i] = tex[dice.Roll(1,7)];	// Select a sun surface texture from the ones we created.
	}

	// Create a cloudy texture that repeats ok for the photon sphere.
	m_TexNoise = re.UseTexture(Id().GetID(), "NoiseRep");
	if(!m_TexNoise)
	{
		QImage terNoise(":data/TerNoise.png");
		QImage texPS(terNoise.width()*2, terNoise.height(), QImage::Format_ARGB32);
		QPainter ps(&texPS);
		ps.drawImage(0,0, terNoise);
		ps.translate(terNoise.width()*2,0);
		ps.scale(-1,1);
		ps.drawImage(0,0, terNoise);
		ps.end();
		m_TexNoise = re.CreateTextureFromBitmap(Id().GetID(), "NoiseRep", texPS);
	}
}

bool DaStar::IsTicker() const
{
	if(DaAtom::IsTicker())
		return true;
	return !IsPrimary() && (IsInAnyScene() || Transform().IsIdentity());
}

void DaStar::InitSystemPosition()
{
	if(!IsPrimary() && !IsDead())
	{
		// Change the world transformation matrix for this star.
		AuMatrix4x4 tCompanion;
		float rangle = AngleToCompanionStar();
		tCompanion.RotateZ(rangle);												// Rotate model space to the star's orbit angle.
		tCompanion.Translate(AuPoint(OrbitRadius(m_chOrbit)*FeetPerAu(), 0));	// Translate to location relative to parent star.
		tCompanion.RotateZ(-rangle);											// Rotate model space to the star's orbit angle.
		tCompanion.RotateX(AxialTilt());
		ChangePosNotParent(AuAtomPos(ParentId(), tCompanion));
	}
}

void DaStar::Tick(const AuElapsedTimer& last, const AuElapsedTimer& now)
{
	InitSystemPosition();
	DaAtom::Tick(last, now);
}

void DaStar::TickOnClient(const AuElapsedTimer& last)
{
	InitSystemPosition();
	DaAtom::TickOnClient(last);
}

unsigned int DaStar::IsTocker() const
{
	unsigned int ms = DaAtom::IsTocker();
	if(!IsNormal())
		return ms;

	if(!ms || ms > 3*d_MSecPerSec)
	{
		m_StarLock.lockForRead();
		bool hasSG = !m_Stargates.isEmpty();
		m_StarLock.unlock();
		if(hasSG)
			ms = 3*d_MSecPerSec;
	}

	if(!ms || ms > 30*d_MSecPerSec)
	{
		if(IsRingworldStar())
			ms = 30*d_MSecPerSec;
	}
	return ms;
}

void DaStar::Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans)
{
	DaAtom::Tock(last, now, scripts, tockTrans);

	// Stargates decay in Tock so it happens when I'm not in anyone's scene. The decay time isn't critical so it's ok if the timing
	// isn't perfect.
	bool closeOrigins = false;
	DaWorldRingSection* refSection = 0;
	if(IsRingworldStar())
	{
		// Origin positive stargates shut off when cut off by the shadow squares.
		refSection = FindRingworldControlSection();
		if(refSection && refSection->IsStargateBeamBlocked())
			closeOrigins = true;		// Shut off origins.

		// Signal the connected worlds, which might not have existed an hour ago, either never explored or decayed and returned.
		// Worlds create henges in their geology when they receive this message.
		// They also notify the global list of henges, which is used by the story engine to find ringworlds and henges.
		if(g_Au.IsSceneSolo())
		{
			if(!m_TexSun && last.IsNewPeriod(now, d_MSecPerMin, Id().GetID()))
			{
				// The glTexture variable is used in solo to know if we have done this yet.
				// It's easier to test if I don't have to wait an hour.
				m_TexSun = 1;
				CreateRingworldHenges();	// Ring sections create henges and signal their destinations to reciprocate.
			}
		}
		else
		{
			if(last.IsNewPeriod(now, d_MSecPerHr, Id().GetID()) ||
					(!m_TexSun && last.IsNewPeriod(now, d_MSecPerMin, Id().GetID())))
			{
				// The glTexture variable is used on the server to know if we have done this yet.
				// It's easier to test if I don't have to wait an hour.
				m_TexSun = 1;

				// There is no need to signal stargates if they have been used recently. That process does this same work.
				bool recentStargate = false;
				m_StarLock.lockForRead();
				if(!m_StargatesChange.IsNull() && m_StargatesChange.ElapsedHour() < 1)
					recentStargate = true;
				m_StarLock.unlock();
				if(!recentStargate)
					CreateRingworldHenges();	// Ring sections create henges and signal their destinations to reciprocate.
			}
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	m_StarLock.lockForWrite();
	int decayed = m_Stargates.Decay(closeOrigins);
	if(decayed)
	{
		m_StargatesChange.Set();
		m_StarLock.unlock();

		// Notify the server farm when an origin positive stargate closes.
		if(decayed == 2)
		{
			// Deactivate the ringworld control center when an origin positive stargate closes.
			if(refSection)
			{
				refSection->DeactivateControlCenter();

				// Deactivate the henges on every ringworld section.
				DeactivateRingworldHenges();
			}

			// Notify the server farm so the destination negative stargate can close too.
			const DaSolarSystem* system = FindParentSolarSystem();
			const DaGalaxy* galaxy = system->FindParentGalaxy();
			if(galaxy)
			{
				AuPoint galaxyLoc;
				system->GetLocationRelativeToParent(galaxyLoc, galaxy);
				emit StargateClosed(galaxy->Id(), galaxyLoc);
			}
		}

		// Notify client.
		SetModified(NOTIFY_Scene);	// Stargates are not stored in the db.
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}
	else
		m_StarLock.unlock();
}

void DaStar::AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim)
{
	// Change the world transformation matrix for this star.
	if(!IsPrimary())
	{
		// Update zone-of-control radius based on planets.
		if(m_ZoneOfControl == FeetPerSolarSystem()/2 * FeetPerSolarSystem()/2)
			CalculateZoneOfControl();

		InitSystemPosition();
	}

	// Increase the rotation value for the texture animation. This is done regardless of visibility due to the
	// mini model of ringworlds.
	AuTime now(AuTime::Now());
	float val = now.MinCycle();
	if(IsAnyBlackHole())
	{
		int alg1 = Id().GetID() % 10;	// 0 to 9
		int alg2 = alg1 + 1;			// 1 to 10
		int alg3 = alg1 + 2;			// 2 to 11
		m_SphereRot.x += AuNoiseF::SmoothCubicCoherentNoise(alg1, val) * DEG2RADf/3;
		m_SphereRot.y += AuNoiseF::SmoothCubicCoherentNoise(alg2, val) * DEG2RADf/3;
		m_SphereRot.z += AuNoiseF::SmoothCubicCoherentNoise(alg3, val) * DEG2RADf/3;

		m_fRotation[0] = now.Cycle(30000) * PIf*2;
		m_fRotation[1] = now.Cycle(15000) * PIf*2;
		m_fRotation[2] = 0;	// n/a
		m_fRotation[3] = 0;	// n/a
	}
	else if(IsNeutronStar())
	{
		int alg1 = Id().GetID() % 10;	// 0 to 9
		int alg2 = alg1 + 1;			// 1 to 10
		int alg3 = alg1 + 2;			// 2 to 11
		m_SphereRot.x += AuNoiseF::SmoothCubicCoherentNoise(alg1, val) * DEG2RADf;
		m_SphereRot.y += AuNoiseF::SmoothCubicCoherentNoise(alg2, val) * DEG2RADf;
		m_SphereRot.z += AuNoiseF::SmoothCubicCoherentNoise(alg3, val) * DEG2RADf;

		m_fRotation[0] = 0;	// n/a
		m_fRotation[1] = 0;	// n/a
		m_fRotation[2] = 0;	// n/a
		m_fRotation[3] = 0;	// n/a
	}
	else
	{
		int alg1 = Id().GetID() % 10;	// 0 to 9
		int alg2 = alg1 + 1;			// 1 to 10
		int alg3 = alg1 + 2;			// 2 to 11
		m_SphereRot.x += AuNoiseF::SmoothCubicCoherentNoise(alg1, val) * DEG2RADf/100;
		m_SphereRot.y += AuNoiseF::SmoothCubicCoherentNoise(alg2, val) * DEG2RADf/100;
		m_SphereRot.z += AuNoiseF::SmoothCubicCoherentNoise(alg3, val) * DEG2RADf/100;

		m_fRotation[0] = now.Cycle(30000) * PIf*2;
		m_fRotation[1] = now.Cycle(-45000) * PIf*2;
		m_fRotation[2] = now.Cycle(60000) * PIf*2;
		m_fRotation[3] = now.Cycle(-90000) * PIf*2;
	}
	if(m_SphereRot.x > PI)
		m_SphereRot.x -= PI*2;
	else if(m_SphereRot.x < -PI)
		m_SphereRot.x += PI*2;

	if(m_SphereRot.y > PI)
		m_SphereRot.y -= PI*2;
	else if(m_SphereRot.y < -PI)
		m_SphereRot.y += PI*2;

	if(m_SphereRot.z > PI)
		m_SphereRot.z -= PI*2;
	else if(m_SphereRot.z < -PI)
		m_SphereRot.z += PI*2;


	// This will take care of the subatoms, particularly the terrain patches.
	DaAtom::AnimateDeviceObjects(ari, sceneAtom, lastAnim);
}

// Returns the radius of the star in feet, event horizon radius for black holes.
double DaStar::Radius() const
{
	double radStar = OrbitRadius(m_chRadius);	// Radius in au.
	if(m_chRadius <= 0)
	{
		// Small star. Calculate a smaller size.
		m_StarLock.lockForRead();
		radStar -= (m_byZones[0] - 1) * 0.01;	// Radius gets smaller when inner orbit zone is colder.
		m_StarLock.unlock();
	}
	radStar *= FeetPerAu();
	return radStar;
}

double DaStar::PhotonSphereRadius() const
{
	return Radius() * 1.5;
}

double DaStar::NeutronSphereRadius() const
{
	return Radius() * 1.05;
}

// Returns true if a child moving to 'fnuloc' from 'foldloc' departs the influence of this atom.
bool DaStar::CheckChildExit(const AuPoint &/*foldloc*/, const AuPoint &fnuloc, const DaAtom* /*who*/) const
{
	double rd = fnuloc.Length2();				// Squared distance to the child's proposed location.
	if(rd > m_ZoneOfControl)
	{
		if(IsPrimary() && dynamic_cast<const DaDesigner*>(Parent()))
			return false;	// Cannot depart primary star's zone in the designer preview.
		return true;
	}
	return false;
}

// Returns true if movement from foldloc to fnuloc would enter the area of influence of this atom.
// 'foldloc' and 'fnuloc' are relative to the parent.
bool DaStar::CheckPeerJoin(const AuPoint &/*foldloc*/, const AuPoint &fnuloc, const DaAtom*)
{
	double rd = fnuloc.Distance2(Location());	// Squared distance to the peer's proposed location.
	if(rd < m_ZoneOfControl)
		return true;
	return false;
}

AuWormhole DaStar::CheckWormholeEntry(const AuPoint& oldloc, const AuPoint& nuloc, float& entryAngle) const
{
	//qDebug() << "DaStar::CheckWormholeEntry" << oldloc << nuloc;
	const DaSolarSystem* ss = dynamic_cast<const DaSolarSystem*>(FindEnvironmentAtom());
	if(!ss)
	{
		// No solar system or wormholes in the designer preview.
		entryAngle = -1;
		return AuWormhole();	// Invalid return object.
	}

	AuPoint soldloc, snuloc;
	if(IsPrimary())
	{
		soldloc = oldloc;
		snuloc = nuloc;
	}
	else
	{
		// Convert locations relative to this companion star to be relative to the parent star/solar system.
		soldloc = Transform().Project(oldloc);
		snuloc = Transform().Project(nuloc);
	}

	// Check for entry into permanent system wormhole.
	AuWormhole wh = ss->CheckWormholeEntry(soldloc, snuloc, entryAngle);
	if(wh.IsValid())
		return wh;

	// Check stargates since no system wormhole was hit.
	m_StarLock.lockForRead();
	bool hasSG = !m_Stargates.isEmpty();
	m_StarLock.unlock();

	if(hasSG)
	{
		double whStarDistance = Radius() * 1.1;	// Might acquire m_Starlock.

		m_StarLock.lockForRead();
		for(AuStargateList::const_iterator gi = m_Stargates.cbegin(); gi != m_Stargates.cend(); gi++)
		{
			AuWormhole whg = AuWormhole("", (*gi).IsOrigin() ? AuWormhole::StargatePositive : AuWormhole::StargateNegative,
					(*gi).ToGalaxy(), (*gi).ToGalaxyLoc());
			whg.SetStarDistance(whStarDistance, AuPoint(0,0));
			float angle = whg.CheckEntry(ss, soldloc, snuloc);
			if(angle >= 0)
			{
				m_StarLock.unlock();
				entryAngle = angle;
				return whg;
			}
		}
		m_StarLock.unlock();
	}

	return AuWormhole();
}

// Text of this star on a display screen.
QString DaStar::Name() const
{
	if(!Parent())
		return tr("Star with no Parent");
	QString sysName;
	const DaAtom* sysOrDesigner = FindEnvironmentAtom();
	if(sysOrDesigner)
		sysName = sysOrDesigner->Name();
	int number = StarNumber();
	switch(number)
	{
	case 0:
		return QString("%1 Alpha")
			.arg(sysName);
	case 1:
		return QString("%1 Beta")
			.arg(sysName);
	case 2:
		return QString("%1 Gamma")
			.arg(sysName);
	case -2:
		return QString("%1 Black Hole")
			.arg(sysName);
	default:
		return sysName;
	}
}

// Text of this star on a display screen.
// Static version.
QString DaStar::Name(DaStar::TYPE eType, DaStar::SIZE eSize, unsigned char bySpecDec)
{
	QString sdec;
	sdec = QString("%1").arg(bySpecDec);

	const char *ptype;
	const char *pcolor;
	switch(eType)
	{
	case TYPE_O:
		ptype = "O";
		pcolor = "Blue";
		break;
	case TYPE_B:
		ptype = "B";
		pcolor = "Blue White";
		break;
	case TYPE_A:
		ptype = "A";
		pcolor = "White";
		break;
	case TYPE_F:
		ptype = "F";
		pcolor = "Yellow White";
		break;
	default:
		Q_ASSERT(false);
	case TYPE_G:
		ptype = "G";
		pcolor = "Yellow";
		break;
	case TYPE_K:
		ptype = "K";
		pcolor = "Orange";
		break;
	case TYPE_M:
		ptype = "M";
		pcolor = "Red";
		break;
	case TYPE_SupermassiveBlackHole:
		return tr("Supermassive Black Hole");
	case TYPE_StellarBlackHole:
		return tr("Stellar Black Hole");
	case TYPE_NeutronStar:
		return tr("Neutron Star");
	}

	QString snom;
	switch(eSize)
	{
	case SIZE_Ia:
		snom = tr("%1%2 Ia Bright Supergiant").arg(ptype).arg(sdec);
		break;
	case SIZE_Ib:
		snom = tr("%1%2 Ib Weaker Supergiant").arg(ptype).arg(sdec);
		break;
	case SIZE_II:
		snom = tr("%1%2 II Bright Giant").arg(ptype).arg(sdec);
		break;
	case SIZE_III:
		snom = tr("%1%2 III Giant").arg(ptype).arg(sdec);
		break;
	case SIZE_IV:
		snom = tr("%1%2 IV Subgiant").arg(ptype).arg(sdec);
		break;
	case SIZE_V:
		snom = tr("%1%2 V Main Sequence").arg(ptype).arg(sdec);
		break;
	case SIZE_VI:
		snom = tr("%1%2 VI Subdwarf").arg(ptype).arg(sdec);
		break;
	case SIZE_D:
		snom = tr("D%1 %2 Dwarf").arg(ptype).arg(pcolor);
		break;
	}
	return snom;
}

QString DaStar::Description() const
{
	m_StarLock.lockForRead();
	TYPE t = m_eType;
	SIZE s = m_eSize;
	m_StarLock.unlock();
	return Name(t, s, m_bySpecDec);
}

// Text of this star on a display screen, as short as possible.
// Static version.
QString DaStar::ScreenName(DaStar::TYPE eType, DaStar::SIZE eSize, unsigned char bySpecDec)
{
	QString sdec;
	sdec = QString("%1").arg(bySpecDec);

	const char *ptype;
	switch(eType)
	{
	case TYPE_O:
		ptype = "O";
		break;
	case TYPE_B:
		ptype = "B";
		break;
	case TYPE_A:
		ptype = "A";
		break;
	case TYPE_F:
		ptype = "F";
		break;
	default:
		Q_ASSERT(false);
	case TYPE_G:
		ptype = "G";
		break;
	case TYPE_K:
		ptype = "K";
		break;
	case TYPE_M:
		ptype = "M";
		break;
	case TYPE_StellarBlackHole:
	case TYPE_SupermassiveBlackHole:
		return tr("Black Hole");
	case TYPE_NeutronStar:
		return tr("Neutron Star");
	}

	QString snom;
	switch(eSize)
	{
	case SIZE_Ia:
		snom = tr("%1%2 Ia").arg(ptype).arg(sdec);
		break;
	case SIZE_Ib:
		snom = tr("%1%2 Ib").arg(ptype).arg(sdec);
		break;
	case SIZE_II:
		snom = tr("%1%2 II").arg(ptype).arg(sdec);
		break;
	case SIZE_III:
		snom = tr("%1%2 III").arg(ptype).arg(sdec);
		break;
	case SIZE_IV:
		snom = tr("%1%2 IV").arg(ptype).arg(sdec);
		break;
	case SIZE_V:
		snom = tr("%1%2 V").arg(ptype).arg(sdec);
		break;
	case SIZE_VI:
		snom = tr("%1%2").arg(ptype).arg(sdec);
		break;
	case SIZE_D:
		snom = tr("D%1").arg(ptype);
		break;
	}

	return snom;
}

// Text of this star on a display screen.
// Static version.
QString DaStar::Name(DaStar::TYPE eType, DaStar::SIZE eSize)
{
	const char *ptype;
	const char *pcolor;
	switch(eType)
	{
	case TYPE_O:
		ptype = "O";
		pcolor = "Blue";
		break;
	case TYPE_B:
		ptype = "B";
		pcolor = "Blue White";
		break;
	case TYPE_A:
		ptype = "A";
		pcolor = "White";
		break;
	case TYPE_F:
		ptype = "F";
		pcolor = "Yellow White";
		break;
	default:
		Q_ASSERT(false);
	case TYPE_G:
		ptype = "G";
		pcolor = "Yellow";
		break;
	case TYPE_K:
		ptype = "K";
		pcolor = "Orange";
		break;
	case TYPE_M:
		ptype = "M";
		pcolor = "Red";
		break;
	case TYPE_StellarBlackHole:
		return tr("Stellar Black Hole");
	case TYPE_SupermassiveBlackHole:
		return tr("Supermassive Black Hole");
	case TYPE_NeutronStar:
		return tr("Neutron Star");
	}

	QString snom;
	switch(eSize)
	{
	case SIZE_Ia:
		snom = tr("%1 Ia Bright Supergiant").arg(ptype);
		break;
	case SIZE_Ib:
		snom = tr("%1 Ib Weaker Supergiant").arg(ptype);
		break;
	case SIZE_II:
		snom = tr("%1 II Bright Giant").arg(ptype);
		break;
	case SIZE_III:
		snom = tr("%1 III Giant").arg(ptype);
		break;
	case SIZE_IV:
		snom = tr("%1 IV Subgiant").arg(ptype);
		break;
	case SIZE_V:
		snom = tr("%1 V Main Sequence").arg(ptype);
		break;
	case SIZE_VI:
		snom = tr("%1 VI Subdwarf").arg(ptype);
		break;
	case SIZE_D:
		snom = tr("D%1 %2 Dwarf").arg(ptype).arg(pcolor);
		break;
	}

	return snom;
}

void DaStar::SensorScan(AuSensorBlipList& blips, const AuPoint& scannerLoc, const AuId& scannerId, quint32 scannerEmpire,
		int scannerDomain, const AuId& scannerWorld,
		float scannerSensorRange, float scannerWeaponRange, int scanContent, const AuId& lockedBlip,
		unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash>& stanceCache) const
{
	// Must scan me if looking for locked-on blip and I'm it.
	bool scanMe = true;
	if((!(scanContent & 0x0008) || !Is(lockedBlip)))
	{
		// Not looking for locked-on blip or I am not the locked-on blip.
		// I am only scanned for sensor consoles, fire control consoles, transporter consoles, and AI pilots.
		if(!(scanContent & 0x0017))
			scanMe = false;
	}

	double blipRange = EnvLocation().Distance(scannerLoc);
	if(scanMe)
	{
		bool inSensorRange = blipRange < scannerSensorRange;
		if(IsAnyBlackHole())
			blips.Add(AuBlipBlackHole(StarType(), Radius(), inSensorRange, Id(), EnvLocation()));
		else if(IsNeutronStar())
		{
			if(IsPrimary() || StarNumber() <= 0)
				blips.Add(AuBlipNeutronStar(Radius(), inSensorRange, Id(), EnvLocation()));
			else
				blips.Add(AuBlipNeutronStarCompanion(Radius(), inSensorRange, Id(), EnvLocation()));
		}
		else
		{
			if(IsPrimary() || StarNumber() <= 0)
				blips.Add(AuBlipStar(StarType(), Size(), SpectralClass(), Radius(), inSensorRange, Id(), EnvLocation()));
			else
				blips.Add(AuBlipStarCompanion(StarType(), Size(), SpectralClass(), Radius(), inSensorRange, Id(), EnvLocation()));
		}
	}

	if(HasSubatoms())
	{
		// Zone ranges are taken from :CheckChildExit.
		int scanSubs = 1;	// 0 to not scan any subatoms, 1 to scan all subatoms, 2 to scan only subordinate planets and stars
		if(IsPrimary())
		{
			if(blipRange > scannerSensorRange + FeetPerSolarSystem()/2)
				scanSubs = 2;			// Too far to scan tiny subatoms in my zone of influence. Must get planets and stars though.
		}
		else
		{
			if(blipRange > scannerSensorRange + OrbitRadius(m_chOrbit) * FeetPerAu()/2)
				scanSubs = 2;			// Too far to scan tiny subatoms in my zone of influence. Must get planets and stars though.
		}

		// I am scanning my subatoms and I actually have some.
		if(scanSubs == 1)
			DaAtom::SensorScan(blips, scannerLoc, scannerId, scannerEmpire, scannerDomain, scannerWorld, scannerSensorRange,
					scannerWeaponRange, scanContent, lockedBlip, stanceCache);
		else if(scanSubs == 2)
		{
			// Only scan sub planets.
			DaAtomList subs;
			ListNormalSubatoms(subs);
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				DaAtom* pAtom = *it;
				if(!dynamic_cast<DaWorld*>(*it) && !dynamic_cast<DaStar*>(*it) && !dynamic_cast<DaAsteroid*>(*it))
					continue;	// Not a planet or a star.
				pAtom->SensorScan(blips, scannerLoc, scannerId, scannerEmpire, scannerDomain, scannerWorld, scannerSensorRange,
						scannerWeaponRange, scanContent, lockedBlip, stanceCache);
			}
		}
	}
}

void DaStar::ListPlanets(DaAtomList &al) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaWorld* pAtom = dynamic_cast<DaWorld*>(*it);
		if(!pAtom)
			continue;
		al.append(pAtom);
	}
}

bool DaStar::SolarSystemNameChanged(const QString& oldName, const QString& nuName, bool notifyDb)
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	if(subs.empty())
		return false;

	// Notify subordinate worlds of this sun.
	bool anyChange = false;
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		if((*it)->SolarSystemNameChanged(oldName, nuName, notifyDb))
			anyChange = true;
	return anyChange;
}

void DaStar::PrepareForScan()
{
	// Change the world transformation matrix for this star.
	InitSystemPosition();

	// Notify subordinate worlds of this sun.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		if((*it)->IsNormal())
			(*it)->PrepareForScan();
}

// Insert first time database record.
void DaStar::Insert(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QSqlQuery qa(QString(), db);
	QString sq = QString("INSERT INTO a_Star (id,idp, locX,locY,locZ, rotX,rotY,rotZ, timeCreate,timeModified,timeTock,"
			"timeDeath, specType,specDec,specSize,radius,orbit,habZone,tempK,orbitZones) "
			"VALUES (?,?, ?,?,?, ?,?,?, ?,?,?,?, ?,?,?,?,?,?,?,?);");
	qa.prepare(sq);
	qa.addBindValue(Id().GetID());
	qa.addBindValue(pos.ParentId().GetID());
	qa.addBindValue(FuzzyClamp(vLoc.x));
	qa.addBindValue(FuzzyClamp(vLoc.y));
	qa.addBindValue(FuzzyClamp(vLoc.z));
	qa.addBindValue(FuzzyClamp(vRot.x));
	qa.addBindValue(FuzzyClamp(vRot.y));
	qa.addBindValue(FuzzyClamp(vRot.z));
	qa.addBindValue(CreatedTime().UTC());
	qa.addBindValue(ModifyTime().UTC());
	qa.addBindValue(TockTime().UTC());
	qa.addBindValue(DeathTime().UTC());

	// Star specific data.
	qa.addBindValue(m_eType);
	qa.addBindValue(m_bySpecDec);
	qa.addBindValue(m_eSize);
	qa.addBindValue(m_chRadius);
	qa.addBindValue(m_chOrbit);
	qa.addBindValue(m_chHabitable);
	qa.addBindValue((qint16)m_SurfaceTempK);

	char orbitZones[17];
	for(int ioz=0; ioz<16; ioz++)
		orbitZones[ioz] = '0' + m_byZones[ioz];
	orbitZones[16] = '\0';
	qa.addBindValue(orbitZones);

	if(!qa.exec())
		qWarning("DaStar::Insert %s FAILED.", (const char*)Name().toLatin1());
}

// Update existing database record.
void DaStar::Update(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QString sq = QString("UPDATE a_Star SET idp=?, locX=?, locY=?, locZ=?, rotX=?, rotY=?, rotZ=?, timeModified=?, "
			"timeTock=?, timeDeath=? WHERE id=%2;").arg(Id().GetID());
	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	qa.addBindValue(pos.ParentId().GetID());
	qa.addBindValue(FuzzyClamp(vLoc.x));
	qa.addBindValue(FuzzyClamp(vLoc.y));
	qa.addBindValue(FuzzyClamp(vLoc.z));
	qa.addBindValue(FuzzyClamp(vRot.x));
	qa.addBindValue(FuzzyClamp(vRot.y));
	qa.addBindValue(FuzzyClamp(vRot.z));
	qa.addBindValue(ModifyTime().UTC());
	qa.addBindValue(TockTime().UTC());
	qa.addBindValue(DeathTime().UTC());

	if(!qa.exec())
	{
		// Execution of the query failed. Report the error.
		QSqlError err = qa.lastError();
		qWarning("DaStar::Update %d %s FAILED '%s' err '%s'.",
				Id().GetID(), (const char*)Name().toLatin1(), (const char*)sq.toLatin1(), (const char*)err.text().toLatin1());
	}
	else if(qa.numRowsAffected() == 0)
	{
		// Execution succeeded but did not affect any records.
		// This happens when an atom is created that gets transferred to another server before the INSERT query is ever posted.
		// On arrival to the destination server, all atoms are touched with SetModified so they'll get saved.
		//qDebug("DaStar::Update %d %s affected NO records. Trying Insert instead.",
				//Id().GetID(), (const char*)Name().toAscii());
		Insert(db);
	}
}

// static
// Load all of these atoms whose parent is idParent.
// Returns 0 if 'idToLoad' == 0, which loads all atoms subordinate to 'aParent'.
// Returns address of loaded atom if 'idToLoad' != 0 and it is successfully loaded.
DaStar* DaStar::LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, 
		LOADSUBATOMS loadWhat, const AuId& idToLoad)
{
	QString sq;
	sq = QString("SELECT id,locX,locY,locZ,rotX,rotY,rotZ,timeCreate,timeModified,"
			"timeTock,timeDeath, specType,specDec,specSize,radius,orbit,habZone,tempK,orbitZones FROM a_Star WHERE ");
	if(!idToLoad.IsNull())
		sq += QString(" id=%1").arg(idToLoad.GetID());
	else
		sq += QString(" idp=%1").arg(parentId.GetID());
	sq += ";";
	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		DaStar *pnu = new DaStar;
		if(pnu)
		{
			// DaAtom values.
			int qv=0;
			quint32 id = qa.value(qv++).toUInt();
			pnu->SetId(AuId(id));
			double vx = qa.value(qv++).toDouble();
			double vy = qa.value(qv++).toDouble();
			double vz = qa.value(qv++).toDouble();
			double rx = qa.value(qv++).toDouble();
			double ry = qa.value(qv++).toDouble();
			double rz = qa.value(qv++).toDouble();
			AuAtomPos pos(parentId, vx,vy,vz, rx,ry,rz);
			pnu->SetPosAndOrient(pos);

			QDateTime tmCreated = qa.value(qv++).toDateTime();
			QDateTime tmModify = qa.value(qv++).toDateTime();
			QDateTime tmTocked = qa.value(qv++).toDateTime();
			QDateTime tmDeath = qa.value(qv++).toDateTime();
			tmCreated.setTimeSpec(Qt::UTC);
			tmModify.setTimeSpec(Qt::UTC);
			tmTocked.setTimeSpec(Qt::UTC);
			tmDeath.setTimeSpec(Qt::UTC);
			QDateTime now(QDateTime::currentDateTime());
			now.setTimeSpec(Qt::UTC);
			if(tmCreated > now)
				tmCreated = now;
			if(tmModify > now)
				tmModify = now;
			if(tmTocked > now)
				tmTocked = now;
			pnu->SetCreatedTime(tmCreated);
			pnu->SetModifyTime(tmModify);
			pnu->SetTockTime(tmTocked);
			pnu->SetDeathTime(tmDeath);

			// DaStar values.
			pnu->m_eType = (TYPE)qa.value(qv++).toInt();
			pnu->m_bySpecDec = qa.value(qv++).toInt();
			pnu->m_eSize = (SIZE)qa.value(qv++).toInt();
			pnu->m_chRadius = qa.value(qv++).toInt();
			pnu->m_chOrbit = qa.value(qv++).toInt();
			pnu->m_chHabitable = qa.value(qv++).toInt();
			pnu->m_SurfaceTempK = qa.value(qv++).toUInt();
			QString orbitZones = qa.value(qv++).toString();
			if(orbitZones.length() == 16)
				for(int ioz=0; ioz<16; ioz++)
					pnu->m_byZones[ioz] = QString(orbitZones[ioz]).toInt();

			pnu->ConnectSignals();
			ether.AddAtom(pnu);
			pnu->SetNormal();	// New atom is ready for normal thread processing. This must be done before loading subatoms.

			if(pnu->m_SurfaceTempK == 0)
			{
				AuDice dice;
				dice.Seed(id);
				pnu->GenSurfaceTempK(dice);
			}

			// Load subatoms of the nu atom AFTER adding it to its pnu parent.
			if((loadWhat & LOAD_Moons) == LOAD_Moons)
			{
				// Caller wants more than just this star.
				if(pnu->IsPrimary())	// Only primary star can have sub-stars.
					DaStar::LoadSubatoms(ether, db, pnu->Id(), loadWhat);
				DaWorldGasGiant::LoadSubatoms(ether, db, pnu->Id(), loadWhat);
				DaWorldGlobe::LoadSubatoms(ether, db, pnu->Id(), loadWhat);
				DaWorldRingSection::LoadSubatoms(ether, db, pnu->Id(), loadWhat);

				pnu->CalculateZoneOfControl();
				pnu->ChooseRingworldRefSection();
			}
			if(loadWhat & LOAD_GamePieces)
			{
				DaAsteroid::LoadSubatoms(ether, db, pnu->Id());
				DaSc::LoadSubatoms(ether, db, pnu->Id());
				DaVehicle::LoadSubatoms(ether, db, pnu->Id());
				DaPerson::LoadSubatoms(ether, db, pnu->Id());
				DaItem::LoadSubatoms(ether, db, pnu->Id());
				DaNpc::LoadSubatoms(ether, db, pnu->Id());
			}
		}
		if(!idToLoad.IsNull())
			return pnu;
	}
	return 0;
}

// Returns -2=black hole, -1=Solitaire, 0=Primary, 1=Secondary, 2=Tertiary
int DaStar::StarNumber() const
{
	// This has caused numerous crashed because of a world getting ticked in a solar system that has been trashed.
	if(!IsNormal())
		return 0;

	if(m_StarNumber > -3 && m_StarNumber < 3)
		return m_StarNumber;

	if(IsAnyBlackHole())
	{
		m_StarNumber = -2;
		return m_StarNumber;
	}

	//qDebug("DaStar::StarNumber");
	if(IsPrimary())
	{
		// This is the primary star. Are there any companion stars?
		DaAtomList subs;
		ListNormalSubatoms(subs);
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		{
			DaAtom* pAtom = *it;
			if(!pAtom->IsNormal())
				continue;
			if(dynamic_cast<const DaStar*>(pAtom))
			{
				// I have a companion star.
				m_StarNumber = 0;
				return m_StarNumber;
			}
		}

		// I am a solitaire, without companion stars.
		m_StarNumber = -1;
		return m_StarNumber;
	}

	const DaStar* parent = (const DaStar*)Parent();
	Q_ASSERT(parent);
	if(!parent)
	{
		m_StarNumber = -1;
		return m_StarNumber;
	}

	bool isBlackHoleSystem = parent->IsAnyBlackHole();

	// Search the children of the parent star to see if I have any peer stars.
	DaAtomList al;
	parent->ListStars(al);
	if(al.count() == 1)
	{
		if(isBlackHoleSystem)
			m_StarNumber = -1;									// I will appear to be the only primary in a black hole system.
		else
			m_StarNumber = 1;									// I am the only subordinate star so I must be secondary.
		return m_StarNumber;
	}

	int baseIndex = isBlackHoleSystem ? 0 : 1;					// So first companion star around a black hole is 'Alpha'.
	Q_ASSERT(al.count() == 2);									// This is the only condition left.
	if(al.count() == 2)
	{
		// Compare my orbit to that of my peer star.
		DaStar* psib = (DaStar*)al.first();
		if(psib == this)
			psib = (DaStar*)al.last();
		if(!psib || m_chOrbit < psib->m_chOrbit)
			m_StarNumber = baseIndex;							// I am the secondary star.
		else
			m_StarNumber = baseIndex + 1;						// I am the tertiary star.
		return m_StarNumber;
	} 
	else
	{
		qWarning("DaStar::StarNumber - Parent star has %d subordinate stars.", al.count());
		// I must be the secondary star.
		m_StarNumber = baseIndex;
		return m_StarNumber;
	}
}

int DaStar::StarNumber(const DaAtomList& stars) const
{
	if(IsAnyBlackHole())
		return -2;

	if(IsPrimary())
	{
		// This is the primary star. Are there any companion stars?
		if(stars.size() > 1)
			return 0;	// I have a companion star.

		// I am a solitaire, without companion stars.
		return -1;
	}

	bool isBlackHoleSystem = false;
	if(stars.size() > 1)
	{
		const DaStar* star = (const DaStar*)stars.first();
		if(star->IsAnyBlackHole())
			isBlackHoleSystem = true;
	}

	// Search the children of the parent star to see if I have any peer stars.
	if(stars.count() == 2)
	{
		if(isBlackHoleSystem)
			return -1;											// I will appear to be the only primary in a black hole system.
		else
			return 1;											// I am the only subordinate star so I must be secondary.
	}

	int baseIndex = isBlackHoleSystem ? 0 : 1;					// So first companion star around a black hole is 'Alpha'.
	Q_ASSERT(stars.count() == 3);								// This is the only condition left.
	if(stars.count() == 3)
	{
		// Compare my orbit to that of my peer star.
		DaStar* psib = (DaStar*)stars[1];
		if(psib == this)
			psib = (DaStar*)stars.last();
		if(m_chOrbit < psib->m_chOrbit)
			return baseIndex;									// I am the secondary star.
		else
			return baseIndex + 1;								// I am the tertiary star.
	} 
	else
	{
		// I must be the secondary star.
		qWarning() << "error: DaStar::StarNumber" << *this << "parent star has" << stars.size() << "subordinate stars.";
		return baseIndex;
	}
}

void DaStar::ProcessDemandTally(const AuItem& item,
		unsigned int& stock, unsigned int& population, unsigned int& needCount, unsigned int& produceCount) const
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	if(subs.empty())
		return;

	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		// Collect demand data from companion stars.
		DaAtom* pAtom = *it;
		const DaStar* star = dynamic_cast<const DaStar *>(pAtom);
		if(star)
			star->ProcessDemandTally(item, stock, population, needCount, produceCount);
		else
		{
			// Collect demand data from planets and their moons.
			const DaWorld* planet = dynamic_cast<const DaWorld *>(pAtom);
			if(planet)
				planet->ProcessDemandTally(AuId(), item, stock, population, needCount, produceCount);
		}
	}
}

bool DaStar::MoveSubatom(DaAtom* pMove, const AuPoint& vMove, int /*elapsedMs*/)
{
	//if(pMove->FindSubatomId(AuId(AuId::Haxus())))
		//qDebug() << "DaStar::MoveSubatom" << Id() << Name() << "atom" << pMove->Id() << pMove->Name()
			//<< "loc" << pMove->Location();
	// Objects cannot move once they get very far inside my radius. They are stuck there like a fly in amber (or chewy nougat).
	double radius = Radius() - AtmosphereTopMSL();
	if(pMove->Location().Length2() <= radius*radius)
		return false;

	DaVessel* ship = dynamic_cast<DaVessel*>(pMove);
	if(ship)
	{
		// A spacecraft is moving in my domain.
		// Check for wormhole entry.
		if(ship->IsReadyToWormhole())
		{
			// The ship is ready to wormhole.
			// Determine the destination point of the move.
			AuPoint oldloc(ship->Location());
			AuPoint nuloc = oldloc + vMove;
			float entryAngle = -1;
			AuWormhole wh = CheckWormholeEntry(oldloc, nuloc, entryAngle);
			if(wh.IsValid() && entryAngle >= 0)
				return TraverseWormhole(ship, wh, entryAngle);
		}
	}

	return DaAtom::MoveSubatom(pMove, vMove, 0);
}

bool DaStar::TraverseWormhole(DaVessel* ship, const AuWormhole& wh, float entryAngle)
{
	if(g_Au.IsSceneClient())
	{
		// Notify the server to do this work too.
		// Find the parent solar system.
		DaSolarSystem* pmyss = dynamic_cast<DaSolarSystem*>(FindEnvironmentAtom());
		if(!pmyss)
			return false;	// No parent solar system? Arena?

		ship->SetTraveling();			// Enter traveling state until we finish the transition.
		emit EnteredWormholeOnClient(ship->Id(), pmyss->Id(), wh, entryAngle);
		return true;
	}

	// The ship has entered a wormhole with its wormhole drives charged and ready.
	ship->DetachRiders();									// Shed any chaff clinging to the outside of the hull.

	// Entry with engines at the wrong polarity destroys the ship.
	quint16 whichFTL = ship->FTLWormholeDrive();
	if(wh.DrivePolarity() != ship->FTLDriveSetting(whichFTL)+1)
	{
		// FTL polarity wrong, ship destroyed.
		ship->DamageAndDestroy(tr("Vaporized by wormhole entry with incorrect FTL drive field polarity."));
		return true;
	}

	// Entry at too steep of an angle damages the ship.
	if(entryAngle > PIf/4)
	{
		AuCombatAttacker attacker(tr("steep wormhole entry angle"));

		AuCombatWeapon weapon(WM_Internal);
		weapon.m_CID = CID_Wormhole;
		weapon.m_w1.m_Effect = WE_Electric;
		weapon.m_w2.m_Effect = WE_Radiation;

		AuCombatResult result;
		int entryDegrees = (int)(entryAngle * RAD2DEGf);
		int dmgRange = ship->DesignHitsTotal() * entryDegrees / 180;
		result.m_Damage1 = g_Au.Dice().Roll(1,dmgRange);
		result.m_Damage2 = g_Au.Dice().Roll(1,dmgRange);

		CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
		if(result.m_IsDestroyed)
			return true;				// Ship destroyed by wormhole.
	}

	// When a ship enters a wormhole, it is destroyed if there is nobody online who is aboard.
	if(!ship->DepartingSolarSystem())
		return true;	// Nobody online was aboard. Tsk.

	// Spacecraft can tow atoms with tractor beams.
	AuIdList towed;
	DaVessel* departShip = dynamic_cast<DaVessel*>(ship);
	if(departShip)
		departShip->ListTractorBeamTowed(towed);

	// Wormhole jump, all systems are go.
	if(!TraverseWormhole(ship, wh))
		return false;

	// Primary ship made it. Take any towed vessels along too.
	for(AuIdList::const_iterator ti = towed.cbegin(); ti != towed.cend(); ti++)
	{
		DaVessel* towedShip = dynamic_cast<DaVessel*>(FindIdRecursive(*ti));
		if(!towedShip || !towedShip->IsNormal())
			continue;
		TraverseWormhole(towedShip, wh);
	}

	return true;
}

bool DaStar::TraverseWormhole(DaVessel* ship, const AuWormhole& wh)
{
	AuEther* ether = Ether();
	if(!ether)
		return false;
	// Find the parent solar system.
	DaSolarSystem* pmyss = dynamic_cast<DaSolarSystem*>(FindEnvironmentAtom());
	if(!pmyss)
		return false;	// No parent solar system? Arena?

	// Find the parent sector.
	DaSector *ps = pmyss->FindParentSector();
	Q_ASSERT(ps);
	if(!ps)
		return false;	// No parent sector?

	// Player Statistics
	DaAtomList people;
	ship->FindPeopleAwakeRecursive(people);
	for(DaAtomList::const_iterator pi = people.begin(); pi != people.end(); pi++)
	{
		DaPerson* person = (DaPerson*)*pi;
		if(!person->IsOnLine() && !person->IsAIControlled())
			continue;
		person->Stats().Wormholed(ship);
		person->IncreaseSkill(AuSkill::SpaceExploration, 1);
	}

	DaAtom* destAtom;
	DaSolarSystem* pdestss;
	AuId destParentId;
	AuPoint destGalaxyLoc;
	if(wh.IsIntergalactic())
	{
		// Going intergalactic!
		pdestss = 0;
		DaUniverse* universe = FindParentUniverse();
		Q_ASSERT(universe);
		destAtom = universe->FindNormalSubatomId(wh.ToGalaxy());
		Q_ASSERT(destAtom);

		if(wh.Polarity() == AuWormhole::IntergalacticOneWay)
		{
			// The destination location is the same as the departure location, relative to the galaxy.
			double rotZ = (GenSeed() % 360) * DEG2RAD;
			destGalaxyLoc = pmyss->GalaxyLoc().RotateZ(rotZ);
		}

		// Destination atom parent is also the galaxy id. This is what the parent atom id gets set to in the database, so it can
		// be restored intelligently in case of server crash, i.e. rescued from limbo.
		destParentId = destAtom->Id();

		// Player Statistics
		for(DaAtomList::const_iterator pi = people.begin(); pi != people.end(); pi++)
			((DaPerson*)*pi)->Stats().SectorChange(ship);
	}
	else if(wh.IsStargate())
	{
		pdestss = 0;
		DaUniverse* universe = FindParentUniverse();
		Q_ASSERT(universe);
		destAtom = universe->FindNormalSubatomId(wh.ToGalaxy());
		Q_ASSERT(destAtom);

		destGalaxyLoc = wh.GalaxyDestLoc();							// Galaxy location for stargates.

		// Player Statistics
		bool isAtLeastASector = true;
		const DaGalaxy* galaxy = ship->FindParentGalaxy();
		if(galaxy && destAtom->Is(galaxy->Id()))
		{
			const DaSector* sector = ship->FindParentSector();
			AuPoint sectorLoc = sector->Location();
			if(fabs(sectorLoc.x - destGalaxyLoc.x) < 5 &&
					fabs(sectorLoc.y - destGalaxyLoc.y) < 5 &&
					fabs(sectorLoc.z - destGalaxyLoc.z) < 5)
				isAtLeastASector = false;
			if(g_Au.m_LogMessages & LOG_Travel)
				qDebug() << "DaStar::TraverseWormhole" << Id() << Name() << "ship" << ship->Id() << ship->Name()
					<< "sectorLoc" << sectorLoc << "destLoc" << destGalaxyLoc;
		}
		if(isAtLeastASector)
			for(DaAtomList::const_iterator pi = people.begin(); pi != people.end(); pi++)
				((DaPerson*)*pi)->Stats().SectorChange(ship);
	}
	else
	{
		// Find the destination solar system.
		// Check in this sector first before checking the entire galaxy.
		pdestss = dynamic_cast<DaSolarSystem*>(ps->FindNormalSubatomId(wh.ToSystem()));
		if(!pdestss)
		{
			// The atom is changing sectors.
			// Not in my sector so try the entire galaxy.
			pdestss = dynamic_cast<DaSolarSystem*>(ether->FindNormalAtom(wh.ToSystem()));

			// Player Statistics
			for(DaAtomList::const_iterator pi = people.begin(); pi != people.end(); pi++)
				((DaPerson*)*pi)->Stats().SectorChange(ship);
		}
		if(!pdestss)
		{
			// No destination solar system? Not likely.
			quint16 whichFTL = ship->FTLWormholeDrive();
			ship->DischargeFTLDrive(whichFTL);

			if(g_Au.m_LogMessages & LOG_Error)
			{
				QString logmsg = QString("Spacecraft %1 %2 entered wormhole to non-existent system.")
					.arg(ship->Id().GetID()).arg(ship->Name());
				g_Au.Error(logmsg);
			}

			return false;
		}
		destAtom = pdestss;
		destParentId = destAtom->ParentId();
	}

	ship->SetTraveling();			// Mark the atom as traveling elsewhere, so it will be ignored locally.

	if(!wh.IsStargate())
	{
		// Give the captain credit for exploring my wormhole.
		// The empire who owns the ship will now "know" about the wormhole.
		// Stargates are temporary so they do not get "explored".
		pmyss->WormholeExplored(wh, ship, false);
	}

	// Update waypoints in ship course. Discharge FTL drive.
	ship->TraversedWormhole(pmyss, wh);

	// Calculate location in next system.
	// We'll set a vector value that indicates the polarity of the wormhole entered. When the ship emerges in the destination
	// system, its MoveSubatom call will work out the proper location. Refer to the logic at the beginning of this function.
	// The direction doesn't matter to Stargates.
	// See DaVessel::ExitWormhole for the other end of this, where these lengths are interpreted.
	AuPoint fdest = wh.SectorVecToDestination();
	if(wh.Polarity() == AuWormhole::Positive)
		fdest.Unit(1);	// 1=positive
	else if(wh.Polarity() == AuWormhole::Negative)
		fdest.Unit(2);	// 2=negative
	else if(wh.Polarity() == AuWormhole::Intergalactic)
		fdest.Unit(3);	// 3=intergalactic
	else if(wh.Polarity() == AuWormhole::StargatePositive)
		fdest.Unit(4);	// 4=stargate positive
	else if(wh.Polarity() == AuWormhole::StargateNegative)
		fdest.Unit(5);	// 5=stargate negative
	else if(wh.Polarity() == AuWormhole::IntergalacticOneWay)
		fdest.Unit(6);	// 6=intergalactic one way

	AuMatrix4x4 t;
	if(pdestss)
		t.UnRotate(pdestss->Rotation());
	t.Translate(fdest);
	ship->ForcePos(AuAtomPos(Id(), t));
	ship->InvalidateEnvironmentRecursive();
	ship->SetAtRest();
	ship->SetModified();

	if(g_Au.m_LogMessages & LOG_Travel)
	{
		QString logmsg;
		if(pdestss)
			logmsg = QString("Spacecraft %1 %2 entered wormhole from system %5 %6 to system %3 %4.")
				.arg(ship->Id().GetID()).arg(ship->Name())
				.arg(pdestss->Id().GetID()).arg(pdestss->Name())
				.arg(pmyss->Id().GetID()).arg(pmyss->Name());
		else if(wh.IsStargate())
			logmsg = QString("Spacecraft %1 %2 entered stargate from system %3 %4 to %5 %6 at %7,%8,%9.")
				.arg(ship->Id().GetID()).arg(ship->Name())
				.arg(pmyss->Id().GetID()).arg(pmyss->Name())
				.arg(destAtom->Id().GetID()).arg(destAtom->Name())
				.arg(destGalaxyLoc.x).arg(destGalaxyLoc.y).arg(destGalaxyLoc.z);
		else if(wh.Polarity() == AuWormhole::IntergalacticOneWay)
			logmsg = QString("Spacecraft %1 %2 entered one-way intergalactic wormhole from system %3 %4 to %5 %6 destGalaxyLoc %7,%8,%9.")
				.arg(ship->Id().GetID()).arg(ship->Name())
				.arg(pmyss->Id().GetID()).arg(pmyss->Name())
				.arg(destAtom->Id().GetID()).arg(destAtom->Name())
				.arg(destGalaxyLoc.x).arg(destGalaxyLoc.y).arg(destGalaxyLoc.z);
		else
			logmsg = QString("Spacecraft %1 %2 entered intergalactic wormhole from system %3 %4 to %5 %6 dir %7,%8,%9.")
				.arg(ship->Id().GetID()).arg(ship->Name())
				.arg(pmyss->Id().GetID()).arg(pmyss->Name())
				.arg(destAtom->Id().GetID()).arg(destAtom->Name())
				.arg(fdest.x).arg(fdest.y).arg(fdest.z);
		g_Au.Log(logmsg);
	}

	// Cannot handoff the atom to the destination system directly due to mutex locking.
	// We will post the departing atom data as a signal so the app can do the transfer.
	if(wh.Polarity() == AuWormhole::IntergalacticOneWay)
	{
		emit TravelGalaxyLoc(ship->Id(), pmyss->Id(), destAtom->Id(), destGalaxyLoc);
	}
	else if(wh.IsStargate())
	{
		// 'destGalaxyLoc' is the destination galaxy location in pc.
		if(wh.Polarity() == AuWormhole::StargatePositive)
		{
			// Send advance notification to open the negative stargate at the other end. It sometimes doesn't get opened right away,
			// particularly when the destination solar system had to be created when the stargate was opened.
			const DaGalaxy* fromGalaxy = pmyss->FindParentGalaxy();
			if(fromGalaxy)
			{
				AuPoint fromGalaxyLoc;
				pmyss->GetLocationRelativeToParent(fromGalaxyLoc, fromGalaxy);
				emit StargateOpened(fromGalaxy->Id(), fromGalaxyLoc, destAtom->Id(), destGalaxyLoc,
						pmyss->PositiveStargateExpiry());
			}
		}

		emit TravelStargate(ship->Id(), pmyss->Id(), destAtom->Id(), destGalaxyLoc);
	}
	else
	{
		emit Travel(ship->Id(), pmyss->Id(), destAtom->Id(), destParentId);
	}
	return true;
}

DaVehicle* DaStar::FindNewVehicle(COMMODITYID vehicleCID, unsigned int empireId)
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaAtom* pAtom = *it;
		DaStar* pStar = dynamic_cast<DaStar *>(pAtom);
		if(pStar)
		{
			DaVehicle* vehicle = pStar->FindNewVehicle(vehicleCID, empireId);
			if(vehicle)
				return vehicle;
		}
		else
		{
			DaWorld* pPlanet = dynamic_cast<DaWorld *>(pAtom);
			if(pPlanet)
			{
				DaVehicle* vehicle = pPlanet->FindNewVehicle(vehicleCID, empireId, AuId(0));
				if(vehicle)
					return vehicle;
			}
		}
	}
	return 0;
}

DaVehicle* DaStar::FindNewVehicleType(VEHICLE_SPOT_TYPE spotType, unsigned int empireId)
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaAtom* pAtom = *it;
		DaStar* pStar = dynamic_cast<DaStar *>(pAtom);
		if(pStar)
		{
			DaVehicle* vehicle = pStar->FindNewVehicleType(spotType, empireId);
			if(vehicle)
				return vehicle;
		}
		else
		{
			DaWorld* pPlanet = dynamic_cast<DaWorld *>(pAtom);
			if(pPlanet)
			{
				DaVehicle* vehicle = pPlanet->FindNewVehicleType(spotType, empireId, AuId(0));
				if(vehicle)
					return vehicle;
			}
		}
	}
	return 0;
}

DaVehicle* DaStar::FindNewVehicleType(VEHICLE_SPOT_TYPE spotType, const DaBd* fetcher)
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaAtom* pAtom = *it;
		DaStar* pStar = dynamic_cast<DaStar *>(pAtom);
		if(pStar)
		{
			DaVehicle* vehicle = pStar->FindNewVehicleType(spotType, fetcher);
			if(vehicle)
				return vehicle;
		}
		else
		{
			DaWorld* pPlanet = dynamic_cast<DaWorld *>(pAtom);
			if(pPlanet)
			{
				DaVehicle* vehicle = pPlanet->FindNewVehicleType(spotType, fetcher);
				if(vehicle)
					return vehicle;
			}
		}
	}
	return 0;
}

void DaStar::FindStartupResources(int& eludium, int& lumenite)
{
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaAtom* pAtom = *it;
		DaStar* pStar = dynamic_cast<DaStar *>(pAtom);
		if(pStar)
		{
			// Look at the planets orbiting subordinate stars, recursively.
			pStar->FindStartupResources(eludium, lumenite);
		}
		else
		{
			DaWorld* pPlanet = dynamic_cast<DaWorld *>(pAtom);
			if(pPlanet)
			{
				// Found a planet. Check it and its moons.
				pPlanet->FindStartupResources(eludium, lumenite);
			}
		}
	}
}

void DaStar::CalculateZoneOfControl()
{
	if(IsPrimary())
	{
		// Primary stars claim the radius of the entire solar system, whether it is used or not.
		// We store the squared value to save square-roots later.
		if(IsSupermassiveBlackHole())
			m_ZoneOfControl = FeetPerSolarSystem() * FeetPerSolarSystem();		// So you can see them farther away.
		else
			m_ZoneOfControl = FeetPerSolarSystem()/2 * FeetPerSolarSystem()/2;
	}
	else
	{
		// Secondary and tertiary stars limit the zone to their own radius, expanded by the planets that orbit it.
		m_ZoneOfControl = Radius();		// Start with the radius of the star itself.

		// Expand zone-of-control radius to fit planets.
		DaAtomList subs;
		ListNormalSubatoms(subs);
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		{
			DaWorld* planet = dynamic_cast<DaWorld*>(*it);
			if(!planet)
				continue;
			double orbitRadius = planet->OrbitRadius();
			if(orbitRadius > m_ZoneOfControl)
				m_ZoneOfControl = orbitRadius;
		}

		// Expand zone-of-control.
		m_ZoneOfControl += OrbitAltAboveSun() + 1100*FeetPerMeter();

		// We store the squared value to save square-roots later.
		m_ZoneOfControl *= m_ZoneOfControl;
	}
}

int DaStar::FetchProbability(COMMODITYID cid) const
{
	int prob;
	if(cid == CID_TypeOPreons && StarType() == TYPE_O)
		prob = 5;
	else if(cid == CID_TypeBPreons && StarType() == TYPE_B)
		prob = 5;
	else if(cid == CID_TypeAPreons && StarType() == TYPE_A)
		prob = 5;
	else if(cid == CID_TypeFPreons && StarType() == TYPE_F)
		prob = 5;
	else if(cid == CID_TypeGPreons && StarType() == TYPE_G)
		prob = 5;
	else if(cid == CID_TypeKPreons && StarType() == TYPE_K)
		prob = 5;
	else if(cid == CID_TypeMPreons && StarType() == TYPE_M)
		prob = 5;
	else
		prob = 0;
	return prob;
}

quint8 DaStar::ResourceQuality(COMMODITYID cid) const
{
	// Return 0 for invalid combinations.
	if(cid == CID_TypeOPreons)
	{
		if(StarType() != TYPE_O)
			return 0;
	}
	else if(cid == CID_TypeBPreons)
	{
		if(StarType() != TYPE_B)
			return 0;
	}
	else if(cid == CID_TypeAPreons)
	{
		if(StarType() != TYPE_A)
			return 0;
	}
	else if(cid == CID_TypeFPreons)
	{
		if(StarType() != TYPE_F)
			return 0;
	}
	else if(cid == CID_TypeGPreons)
	{
		if(StarType() != TYPE_G)
			return 0;
	}
	else if(cid == CID_TypeKPreons)
	{
		if(StarType() != TYPE_K)
			return 0;
	}
	else if(cid == CID_TypeMPreons)
	{
		if(StarType() != TYPE_M)
			return 0;
	}
	else
		return 0;	// No other commodities are produced by this star.

	// Generate the quality of my one-and-only resource.
	AuDice dice;
	dice.Seed(GenSeed());
	return (quint8)dice.RandomQuality();
}

DaAtom* DaStar::SightTarget(const DaAtom* sightingAtom, const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, DaAtom* pBest,
		AuPoint& sightedLoc, unsigned int& sightedPart, bool& sightedShield, AuPoint* sightedSurfaceNormal) const
{
	// Determine my location in the space of the sighting atom.
	// If I am far enough away from the sight segment, this atom and my subordinate atoms can be ignored.
	AuPoint myLoc = tModel.Translation();
	double dist2 = myLoc.DistToSegment(eyeLoc, sightedLoc);
	if(dist2*dist2 > m_ZoneOfControl)							// Compare squared distances.
	{
		//qDebug() << "DaStar::SightTarget" << Id() << Name() << "not in my zone of control";
		return pBest;
	}

	// Base class performs a recursive search.
	return DaAtom::SightTarget(sightingAtom, eyeLoc, tModel, pBest, sightedLoc, sightedPart, sightedShield, sightedSurfaceNormal);
}

void DaStar::ListSceneSubatoms(const DaPerson *pSceneAtom, const AuPoint& cameraEnvLoc, DaAtomList& sceneAtoms)
{
	if(pSceneAtom->IsInAtmosphere())
	{
		// Scene atom is inside an atmosphere.
		DaAtomList subs;
		ListNormalSubatoms(subs);
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		{
			if(dynamic_cast<DaWorld*>(*it) || dynamic_cast<DaStar*>(*it) || dynamic_cast<DaAsteroid*>(*it))
			{
				// Add the planet or star and its subatoms to the list.
				// Atoms between worlds are not needed.
				sceneAtoms.AddIfUnique(*it);
				(*it)->ListSceneSubatoms(pSceneAtom, cameraEnvLoc, sceneAtoms);
			}
		}
	}
	else if(pSceneAtom->FindParentStarInEnv() == this)
	{
		// Scene atom is AT this star and not inside an atmosphere.
		// We need everything.
		DaAtom::ListSceneSubatoms(pSceneAtom, cameraEnvLoc, sceneAtoms);
	}
	else
	{
		// The scene atom is not at this star and not inside an atmosphere.
		// We only need the big stuff that might be seen from a distance.
		DaAtomList subs;
		ListNormalSubatoms(subs);
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
		{
			if(dynamic_cast<DaWorld*>(*it) || dynamic_cast<DaStar*>(*it) || dynamic_cast<DaAsteroid*>(*it))
			{
				// Add the planet or star and its subatoms to the list.
				sceneAtoms.AddIfUnique(*it);
				(*it)->ListSceneSubatoms(pSceneAtom, cameraEnvLoc, sceneAtoms);
			}
			else
			{
				if(dynamic_cast<DaVessel*>(*it) ||
						dynamic_cast<DaVehicle*>(*it) ||
						dynamic_cast<DaEffect*>(*it))
				{
					if(!(*it)->IsInScene(pSceneAtom, cameraEnvLoc))
						continue;
					// Add the ship or vehicle to the list.
					sceneAtoms.AddIfUnique(*it);
				}
			}
		}
	}
}

quint32 DaStar::GenSeed() const
{
	// Select a seed value for this star that is based on the seed value of my parent solar system.
	quint32 genSeed;
	{
		const DaAtom* env = FindEnvironmentAtom();
		const DaSolarSystem* ss = dynamic_cast<const DaSolarSystem*>(env);
		if(ss)
			genSeed = ss->GenSeed();
		else
		{
			const DaDesigner* des = dynamic_cast<const DaDesigner*>(env);
			if(des)
				genSeed = des->GenSeed();
			else
				genSeed = 0;
		}
	}

	// Incorporate star number so the primary and its companion stars are not identical.
	int starNumber = StarNumber();
	return GenSeed(genSeed, starNumber);
}

quint32 DaStar::GenSeed(quint32 systemGenSeed, int starNumber) const
{
	// Incorporate star number so the primary and its companion stars are not identical.
	quint32 genSeed = systemGenSeed;
	switch(starNumber)
	{
	default:
	case -2:
	case -1:
	case 0:
		// The primary star GenSeed is the same as the solar system. This just saves work; there's no clever trickery here.
		break;

	case 1:
	case 2:
		// Use noise to select a genSeed value for companion stars.
		do
		{
			// The starNumber is used to select the noise algorithm so the GenSeed for both companions is not identical.
			double noise = AuNoise::IntegerNoise(starNumber, genSeed);
			genSeed += (quint32)(noise * 0xffffffff);
		}
		while(!genSeed);
		break;
	}
	return genSeed;
}

void DaStar::AdjustNavLocation(const AuPoint& shipLoc, AuPoint& navLoc) const
{
	AuPoint sysLoc = EnvLocation();
	double navDist = navLoc.Distance(sysLoc);
	double safeDist = Radius() + ParkAltAboveSun();				// Navigators never plan to go closer than parking orbit.
	if(navDist < safeDist)
	{
		// Nav location is too close to this sun. Move away.
		if(shipLoc.FuzzyEquals(sysLoc))
		{
			// Ship location is at the center of this sun.
			// Move navLoc straight away from the sun to a safe distance.
			AuPoint navVec = navLoc - sysLoc;
			navVec.Unit(safeDist);
			navLoc = sysLoc + navVec;
		}
		else
		{
			// Nav point is too close to the sun.
			// Move the nav point toward the shipLoc to a safe distance.
			AuPoint navVec = shipLoc - sysLoc;
			navVec.Unit(safeDist);
			navLoc = sysLoc + navVec;
		}
	}

	DaAtom::AdjustNavLocation(shipLoc, navLoc);
}

void DaStar::ChooseRingworldRefSection()
{
	m_StarLock.lockForWrite();
	m_RingworldRefSection = AuId(0);
	m_StarLock.unlock();

	// Locate the ring section with index 0.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		const DaWorldRingSection* ringSection = dynamic_cast<const DaWorldRingSection*>(*it);
		if(!ringSection)
			continue;
		if(ringSection->SectionIndex() == 0)
		{
			m_StarLock.lockForWrite();
			m_RingworldRefSection = ringSection->Id();
			m_StarLock.unlock();
			break;
		}
	}
}

DaWorldRingSection* DaStar::FindRingworldControlSection()
{
	if(!IsRingworldStar())
		return 0;
	return dynamic_cast<DaWorldRingSection*>(FindNormalSubatomId(RingworldRefSection()));
}

DaWorldRingSection* DaStar::FindRingworldArc(int segmentIndex)
{
	if(!IsRingworldStar())
		return 0;

	if(segmentIndex == 0)
		return FindRingworldReferenceSection();

	// Locate the ring section with the desired index.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaWorldRingSection* ringSection = dynamic_cast<DaWorldRingSection*>(*it);
		if(!ringSection)
			continue;
		if(ringSection->SectionIndex() == segmentIndex)
			return ringSection;
	}
	return 0;
}

DaWorldRingSection* DaStar::FindRingworldReferenceSection()
{
	if(!IsRingworldStar())
		return 0;
	return dynamic_cast<DaWorldRingSection*>(FindNormalSubatomId(RingworldRefSection()));
}

const DaWorldRingSection* DaStar::FindRingworldReferenceSection() const
{
	if(!IsRingworldStar())
		return 0;
	return dynamic_cast<const DaWorldRingSection*>(FindNormalSubatomId(RingworldRefSection()));
}

// Sets 'north' and 'east' to unit vectors from loc that point to the north and east respectively, parallel with the surface.
// Sets 'up' to a unit vector pointing up from loc with respect to gravity.
void DaStar::CompassVectors(const AuPoint& loc, AuPoint& north, AuPoint& east, AuPoint& up) const
{
	// Determine north direction vector.
	if(loc.y > 0)
	{
		// loc is north of the equator.
		AuPoint vn = AuPoint(0,0,Radius()) - loc;					// Vector to north pole from loc.
		double rdn = vn.Length();									// Distance to north pole.
		if(rdn != 0.0)
			north = vn/rdn;
		else
			north = AuPoint(0,0,1);
	}
	else
	{
		// loc is south of the equator.
		AuPoint vn = loc - AuPoint(0,0,-Radius());					// Vector from south pole to loc.
		double rdn = vn.Length();									// Distance to south pole.
		if(rdn != 0.0)
			north = vn/rdn;
		else
			north = AuPoint(0,0,1);
	}

	// Calculate the east and north vectors to be parallel to the globe at loc.
	up = -GravityDir(loc);
	east = north.Cross(up);
	east.Unit();
	north = up.Cross(east);
	north.Unit();
}

void DaStar::OpenStargate(const AuId& toGalaxy, const AuPoint& toGalaxyLoc, bool origin, const AuTime& expire)
{
	//qDebug() << "DaStar::OpenStargate" << Id() << Name() << "to" << toGalaxy << toGalaxyLoc << "origin" << origin << "expire"
		//<< expire << "now" << now;
	DaSolarSystem* system = FindParentSolarSystem();
	if(origin)
	{
		// Create the origin stargate.
		m_StarLock.lockForWrite();
		m_Stargates.Open(toGalaxy, toGalaxyLoc, true, expire);
		m_StargatesChange.Set();
		m_StarLock.unlock();

		// Notify the farm to put a corresponding stargate at the destination. This should also wake up the destination system.
		const DaGalaxy* galaxy = system->FindParentGalaxy();
		if(galaxy)
		{
			AuPoint fromGalaxyLoc;
			system->GetLocationRelativeToParent(fromGalaxyLoc, galaxy);
			emit StargateOpened(galaxy->Id(), fromGalaxyLoc, toGalaxy, toGalaxyLoc, expire);
		}

		// Light up the ringworld control center when originating on a ringworld.
		DaWorldRingSection* controlSection = FindRingworldControlSection();
		if(controlSection)
		{
			controlSection->ActivateControlCenter();

			// Activate the henges on every ringworld section.
			ActivateRingworldHenges();
		}
	}
	else
	{
		// When opening a destination stargate, it might actually already exist. We get redundant notifications of inbound stargates
		// to insure the destination end stargate exists. It is possible for us to miss the initial notification when the system
		// must be generated from scratch.
		m_StarLock.lockForWrite();
		if(expire > AuTime::Now() && !m_Stargates.HasDestination(toGalaxy, toGalaxyLoc))
			m_Stargates.Open(toGalaxy, toGalaxyLoc, false, expire);
		m_StargatesChange.Set();
		m_StarLock.unlock();

		system->SetLastPersonVisited();	// Prevent quick decay.
	}

	// Stargates are not stored in the database but the client needs to know that one opened.
	SetModified(NOTIFY_Scene);
	TickTock();
}

void DaStar::CloseStargate(const AuId& fromGalaxy, const AuPoint& fromGalaxyLoc)
{
	//qDebug() << "DaStar::CloseStargate" << Id() << Name() << "from" << fromGalaxy << fromGalaxyLoc;
	m_StarLock.lockForWrite();
	if(m_Stargates.Close(fromGalaxy, fromGalaxyLoc))
	{
		//qDebug() << "DaStar::CloseStargate" << Id() << Name() << "stargate closed";
		m_StargatesChange.Set();
		SetModified(NOTIFY_Scene);		// Stargates are not stored in the database but the client needs to know that one opened.
	}
	m_StarLock.unlock();
}

struct AuRes_
{
	COMMODITYID	cid;
	quint8		qual;
	quint8		prob;
};

QString DaStar::SystemScanReport(int scanEx) const
{
	QString sOrbiting;
	if(!IsPrimary())
	{
		// Orbital position.
		double orbitAngle = PI/2 - Location().ArcTangent_2D();
		while(orbitAngle < 0)
			orbitAngle += PI*2;
		orbitAngle = round(orbitAngle*RAD2DEG);

		sOrbiting = QObject::tr("Orbiting %1 at %2 %3%4")
			.arg(Parent()->Name())
			.arg(AuSensorBlip::FormatBigNumber(Location().Length()))
			.arg(orbitAngle).arg(g_Au.DegreeSymbol());
	}

	double rdia = (Radius() / FeetPerAu()) * 2.;

	QString sType;
	QString sSize;
	QString sSpecDec;
	QString sTempK;
	QString sLight;
	QMap<QString, AuRes_> reslist;
	if(StarType() == DaStar::TYPE_StellarBlackHole)
	{
		sType = tr("Stellar Black Hole");
		double rrad = (Radius() / FeetPerAu());
		sSize = QObject::tr("%1au Schwarzschild Radius").arg(rrad, 0,'f',2);
		sLight = QObject::tr("No Habitable Zone");
	}
	else if(StarType() == DaStar::TYPE_SupermassiveBlackHole)
	{
		sType = tr("Supermassive Black Hole");
		double rrad = (Radius() / FeetPerAu());
		sSize = QObject::tr("%1au Schwarzschild Radius").arg(rrad, 0,'f',2);
		sLight = QObject::tr("No Habitable Zone");
	}
	else if(StarType() == DaStar::TYPE_NeutronStar)
	{
		sType = tr("Neutron Star");
		sSize = QObject::tr("%1au Core Diameter").arg(rdia, 0,'f',2);
		sLight = QObject::tr("No Habitable Zone - Too Cool");
	}
	else
	{
		const char *ptype;
		const char *pcolor;
		switch(StarType())
		{
		case DaStar::TYPE_O:
			ptype = "O";
			pcolor = "Blue";
			break;
		case DaStar::TYPE_B:
			ptype = "B";
			pcolor = "Blue White";
			break;
		case DaStar::TYPE_A:
			ptype = "A";
			pcolor = "White";
			break;
		case DaStar::TYPE_F:
			ptype = "F";
			pcolor = "Yellow White";
			break;
		default:
			Q_ASSERT(false);
		case DaStar::TYPE_G:
			ptype = "G";
			pcolor = "Yellow";
			break;
		case DaStar::TYPE_K:
			ptype = "K";
			pcolor = "Orange";
			break;
		case DaStar::TYPE_M:
			ptype = "M";
			pcolor = "Red";
			break;
		}
		sType = QString("Type %1 %2").arg(ptype).arg(pcolor);

		switch(Size())
		{
		case DaStar::SIZE_Ia:
			sSize = QObject::tr("Size Ia Bright Supergiant");
			sSpecDec = QObject::tr("Spectral Class %1").arg(SpectralClass());
			break;
		case DaStar::SIZE_Ib:
			sSize = QObject::tr("Size Ib Weaker Supergiant");
			sSpecDec = QObject::tr("Spectral Class %1").arg(SpectralClass());
			break;
		case DaStar::SIZE_II:
			sSize = QObject::tr("Size II Bright Giant");
			sSpecDec = QObject::tr("Spectral Class %1").arg(SpectralClass());
			break;
		case DaStar::SIZE_III:
			sSize = QObject::tr("Size III Giant");
			sSpecDec = QObject::tr("Spectral Class %1").arg(SpectralClass());
			break;
		case DaStar::SIZE_IV:
			sSize = QObject::tr("Size IV Subgiant");
			sSpecDec = QObject::tr("Spectral Class %1").arg(SpectralClass());
			break;
		case DaStar::SIZE_V:
			sSize = QObject::tr("Size V Main Sequence");
			sSpecDec = QObject::tr("Spectral Class %1").arg(SpectralClass());
			break;
		case DaStar::SIZE_VI:
			sSize = QObject::tr("Size VI Subdwarf");
			sSpecDec = QObject::tr("Spectral Class %1").arg(SpectralClass());
			break;
		case DaStar::SIZE_D:
			sSize = QObject::tr("Size D Dwarf");
			// Spectral decimal classification does not apply to dwarfs.
			break;
		}

		sTempK = QObject::tr("Surface Temperature %L1%2K").arg(SurfaceTempK()).arg(AuGlobal::DegreeSymbol());

		// Light intensity
		double rHabLight = DaStar::OrbitRadius(HabitableZone());	// Orbit radius in au
		if(HabitableZone() == -1)
		{
			// Habitable radius is within the star.
			sLight = QObject::tr("No Habitable Zone - Too Cool");
		}
		else if(HabitableZone() >= 16)
		{
			// Habitable radius is beyond the solar system.
			sLight = QObject::tr("No Habitable Zone - Too Hot");
		}
		else
		{
			if(rHabLight <= rdia/2.)
				// Habitable radius is within the star.
				sLight = QObject::tr("No Habitable Zone - Too Cool");
			else
				sLight = QObject::tr("Habitable Zone at %1au").arg(rHabLight);
		}

		// Make a list of the natural resources the scanner can detect.
		// Stars are always in the system in my scene.
		if(scanEx == 2)
		{
			// Scan star for resources.
			QString cname;
			COMMODITYID starres[] =
			{
				CID_TypeOPreons,
				CID_TypeBPreons,
				CID_TypeAPreons,
				CID_TypeFPreons,
				CID_TypeGPreons,
				CID_TypeKPreons,
				CID_TypeMPreons,
			};
			int starrescount = sizeof(starres)/sizeof(COMMODITYID);
			for(int sri=0; sri<starrescount; sri++)
			{
				AuRes_ r;
				r.cid = starres[sri];
				r.prob = FetchProbability(r.cid);
				if(r.prob)
				{
					// Show Q factor and probability % only if this resource is present.
					cname = g_Au.CommodityName(r.cid);
					r.qual = ResourceQuality(r.cid);
					reslist.insert(cname, r);
				}
			}
		}
	}

	// Put it all together.
	QString rpt = QString("<html><big><b>%1</b></big><p>").arg(Name());

	// Show general info about the star.
	if(!sOrbiting.isEmpty())
		rpt += sOrbiting + "<br>";
	else
		rpt += QObject::tr("Primary<br>");
	rpt += sType + "<br>";
	if(!sSpecDec.isEmpty())
		rpt += sSpecDec + "<br>";
	if(!sTempK.isEmpty())
		rpt += sTempK + "<br>";
	rpt += sSize + "<br>";
	rpt += sLight;

	// Photosphere
	if(IsAnyBlackHole())
	{
		rdia = (PhotonSphereRadius() / FeetPerAu()) * 2.;
		QString sDia;
		sDia = QObject::tr("%1au Diameter").arg(rdia, 0,'f',2);

		rpt += QObject::tr("<hr><b>Photon Sphere</b><br>");
		rpt += sDia;
	}
	else if(IsNeutronStar())
	{
		rdia = (NeutronSphereRadius() / FeetPerAu()) * 2.;
		QString sDia;
		sDia = QObject::tr("%1au Diameter").arg(rdia, 0,'f',2);

		rpt += QObject::tr("<hr><b>Neutron Sphere</b><br>");
		rpt += sDia;
	}
	else
	{
		QString sDia;
		sDia = QObject::tr("%1au Diameter").arg(rdia, 0,'f',2);

		rpt += QObject::tr("<hr><b>Photosphere</b><br>");
		rpt += sDia;
	}

	if(!reslist.isEmpty())
	{
		rpt += "<br><table border=\"0\" cellpadding=\"2\" cellspacing=\"0\" style=\"background-color: rgb(3, 18, 48);\">";
		rpt += "<tbody>";
		rpt += "<tr><td colspan=\"2\"><b>Resource</b></td><td><b>Quality and Abundance</b></td></tr>";
		for(QMap<QString,AuRes_>::const_iterator ri = reslist.begin(); ri != reslist.end(); ri++)
			rpt += QString("<tr><td><img height=\"16\" src=\"qrc%1\"></td><td>%2</td><td>Q%3 %4%</td></tr>")
				.arg(g_Au.CommodityBitmapName((*ri).cid))
				.arg(g_Au.CommodityName((*ri).cid))
				.arg((*ri).qual)
				.arg((*ri).prob);
		rpt += "</tbody></table>";
	}

	rpt += "</html>";
	return rpt;
}

void DaStar::SystemScanReportXML(QXmlStreamWriter& stream, int scanEx) const
{
	QString sOrbiting;
	if(!IsPrimary())
	{
		// Orbital position.
		double orbitAngle = PI/2 - Location().ArcTangent_2D();
		while(orbitAngle < 0)
			orbitAngle += PI*2;
		orbitAngle = round(orbitAngle*RAD2DEG);

		sOrbiting = QObject::tr("Orbiting %1 at %2 %3%4")
			.arg(Parent()->Name())
			.arg(AuSensorBlip::FormatBigNumber(Location().Length()))
			.arg(orbitAngle).arg(g_Au.DegreeSymbol());
	}

	double rdia = (Radius() / FeetPerAu()) * 2.;

	QString sType;
	QString sSize;
	QString sSpecDec;
	QString sTempK;
	QString sLight;
	QMap<QString, AuRes_> reslist;
	if(StarType() == DaStar::TYPE_StellarBlackHole)
	{
		sType = tr("Stellar Black Hole");
		double rrad = (Radius() / FeetPerAu());
		sSize = QObject::tr("%1au Schwarzschild Radius").arg(rrad);
		sLight = QObject::tr("No Habitable Zone");
	}
	else if(StarType() == DaStar::TYPE_SupermassiveBlackHole)
	{
		sType = tr("Supermassive Black Hole");
		double rrad = (Radius() / FeetPerAu());
		sSize = QObject::tr("%1au Schwarzschild Radius").arg(rrad);
		sLight = QObject::tr("No Habitable Zone");
	}
	else if(StarType() == DaStar::TYPE_NeutronStar)
	{
		sType = tr("Neutron Star");
		sLight = QObject::tr("No Habitable Zone - Too Cool");
	}
	else
	{
		const char *ptype;
		const char *pcolor;
		switch(StarType())
		{
		case DaStar::TYPE_O:
			ptype = "O";
			pcolor = "Blue";
			break;
		case DaStar::TYPE_B:
			ptype = "B";
			pcolor = "Blue White";
			break;
		case DaStar::TYPE_A:
			ptype = "A";
			pcolor = "White";
			break;
		case DaStar::TYPE_F:
			ptype = "F";
			pcolor = "Yellow White";
			break;
		default:
			Q_ASSERT(false);
		case DaStar::TYPE_G:
			ptype = "G";
			pcolor = "Yellow";
			break;
		case DaStar::TYPE_K:
			ptype = "K";
			pcolor = "Orange";
			break;
		case DaStar::TYPE_M:
			ptype = "M";
			pcolor = "Red";
			break;
		}
		sType = QString("%1 %2").arg(ptype).arg(pcolor);

		switch(Size())
		{
		case DaStar::SIZE_Ia:
			sSize = QObject::tr("Ia Bright Supergiant");
			sSpecDec = QObject::tr("%1").arg(SpectralClass());
			break;
		case DaStar::SIZE_Ib:
			sSize = QObject::tr("Ib Weaker Supergiant");
			sSpecDec = QObject::tr("%1").arg(SpectralClass());
			break;
		case DaStar::SIZE_II:
			sSize = QObject::tr("II Bright Giant");
			sSpecDec = QObject::tr("%1").arg(SpectralClass());
			break;
		case DaStar::SIZE_III:
			sSize = QObject::tr("III Giant");
			sSpecDec = QObject::tr("%1").arg(SpectralClass());
			break;
		case DaStar::SIZE_IV:
			sSize = QObject::tr("IV Subgiant");
			sSpecDec = QObject::tr("%1").arg(SpectralClass());
			break;
		case DaStar::SIZE_V:
			sSize = QObject::tr("V Main Sequence");
			sSpecDec = QObject::tr("%1").arg(SpectralClass());
			break;
		case DaStar::SIZE_VI:
			sSize = QObject::tr("VI Subdwarf");
			sSpecDec = QObject::tr("%1").arg(SpectralClass());
			break;
		case DaStar::SIZE_D:
			sSize = QObject::tr("D Dwarf");
			// Spectral decimal classification does not apply to dwarfs.
			break;
		}

		sTempK = QObject::tr("%L1%2K").arg(SurfaceTempK()).arg(AuGlobal::DegreeSymbol());

		// Light intensity
		double rHabLight = DaStar::OrbitRadius(HabitableZone());	// Orbit radius in au
		if(HabitableZone() == -1)
		{
			// Habitable radius is within the star.
			sLight = QObject::tr("No Habitable Zone - Too Cool");
		}
		else if(HabitableZone() >= 16)
		{
			// Habitable radius is beyond the solar system.
			sLight = QObject::tr("No Habitable Zone - Too Hot");
		}
		else
		{
			if(rHabLight <= rdia/2.)
				// Habitable radius is within the star.
				sLight = QObject::tr("No Habitable Zone - Too Cool");
			else
				sLight = QObject::tr("Habitable Zone at %1au").arg(rHabLight);
		}

		// Make a list of the natural resources the scanner can detect.
		// Stars are always in the system in my scene.
		if(scanEx == 2)
		{
			// Scan star for resources.
			QString cname;
			COMMODITYID starres[] =
			{
				CID_TypeOPreons,
				CID_TypeBPreons,
				CID_TypeAPreons,
				CID_TypeFPreons,
				CID_TypeGPreons,
				CID_TypeKPreons,
				CID_TypeMPreons,
			};
			int starrescount = sizeof(starres)/sizeof(COMMODITYID);
			for(int sri=0; sri<starrescount; sri++)
			{
				AuRes_ r;
				r.cid = starres[sri];
				r.prob = FetchProbability(r.cid);
				if(r.prob)
				{
					// Show Q factor and probability % only if this resource is present.
					cname = g_Au.CommodityName(r.cid);
					r.qual = ResourceQuality(r.cid);
					reslist.insert(cname, r);
				}
			}
		}
	}

	// Put it all together.
	stream.writeStartElement("star");
	stream.writeAttribute("starId", Id().StringId());
	stream.writeAttribute("name", Name());
	stream.writeAttribute("type", sType);

	// Show general info about the star.
	if(!sOrbiting.isEmpty())
		stream.writeAttribute("orbit", sOrbiting);
	else
		stream.writeAttribute("orbit", "Primary");

	stream.writeAttribute("size", sSize);
	if(!sSpecDec.isEmpty())
		stream.writeAttribute("spectralClass", sSpecDec);
	if(!sTempK.isEmpty())
		stream.writeAttribute("surfaceTempK", sTempK);
	stream.writeAttribute("hab", sLight);

	// Photosphere
	if(IsAnyBlackHole())
	{
		QString sDia;
		rdia = (PhotonSphereRadius() / FeetPerAu()) * 2.;
		sDia = QObject::tr("%1au Diameter").arg(rdia);

		stream.writeAttribute("shell", "Photon Sphere");
		stream.writeAttribute("diameter", sDia);
	}
	else if(IsNeutronStar())
	{
		QString sDia;
		rdia = (NeutronSphereRadius() / FeetPerAu()) * 2.;
		sDia = QObject::tr("%1au Diameter").arg(rdia);

		stream.writeAttribute("shell", "Neutron Sphere");
		stream.writeAttribute("diameter", sDia);
	}
	else
	{
		QString sDia;
		sDia = QObject::tr("%1au Diameter").arg(rdia);

		stream.writeAttribute("shell", "Photosphere");
		stream.writeAttribute("diameter", sDia);
	}

	if(!reslist.isEmpty())
	{
		for(QMap<QString,AuRes_>::const_iterator ri = reslist.begin(); ri != reslist.end(); ri++)
		{
			stream.writeStartElement("resource");
			stream.writeAttribute("name", g_Au.CommodityName((*ri).cid));
			stream.writeAttribute("quality", QString("%1").arg((*ri).qual));
			stream.writeAttribute("abundance", QString("%1").arg((*ri).prob));
			stream.writeEndElement();
		}
	}

	stream.writeEndElement();
}

void DaStar::FindClosestHarvesterTargetRecursive(COMMODITYID harvesterModule, const AuPoint& shipSysLoc, float gunRange,
		const DaAtom*& targetAtom, double& targetRange) const
{
	// Check world.
	DaAtom::FindClosestHarvesterTargetRecursive(harvesterModule, shipSysLoc, gunRange, targetAtom, targetRange);

	if(!CanBeHarvested(harvesterModule))
		return;
	if(IsAnyBlackHole() || IsNeutronStar())
		return;

	// Perform range test.
	AuPoint targetVec = WeaponLockLocation(shipSysLoc, harvesterModule) - shipSysLoc;	// Vector to target from ship's location.
	double range = targetVec.Length();
	if(range > gunRange)
		return;

	if(targetAtom && range > targetRange)
		return;		// I am not closer than the current target.

	//DHH Check for obstructions between the gun and me?

	// I am the closest target.
	targetAtom = this;
	targetRange = range;
}

AuPoint DaStar::WeaponLockLocation(const AuPoint& shipSysLoc, COMMODITYID harvesterModule) const
{
	Q_UNUSED(harvesterModule);
	AuPoint vecToWeapon = shipSysLoc - EnvLocation();		// Vec to ship from planet, relative to the system.
	vecToWeapon.Unit(Radius());									// Vec scaled to photosphere.
	return EnvLocation() + vecToWeapon;						// System relative target location.
}

double DaStar::PilotAvoidRadius() const
{
	if(IsAnyBlackHole())
		return PhotonSphereRadius() + OrbitAltAboveSun();
	else if(IsNeutronStar())
		return NeutronSphereRadius() + OrbitAltAboveSun();
	else if(IsRingworldStar())
	{
		// Pilots avoid the shadow squares around ringworld stars. It's helpful if I add their radius to mine.
		const DaWorldRingSection* ringRef = FindRingworldReferenceSection();
		double seaRadius = ringRef->OrbitRadius();
		double shadowZOut = seaRadius/2 + 100*FeetPerMeter();           // Dist from sun to outside face of shadow square.
		double myRadius = Radius();                                 // My radius.
		if(shadowZOut > myRadius)
			return shadowZOut + OrbitAltAboveSun();             // Shadow squares are outside my radius.
		else
			return myRadius + OrbitAltAboveSun();               // My radius is bigger than the shadow squares.
	}
	else
		return Radius() + OrbitAltAboveSun();
}

bool DaStar::RoughExtents(AuBox& fExt) const
{
	Q_UNUSED(fExt);
	return false;
}

void DaStar::ActivateRingworldHenges()
{
	// This is already known to be true.
	//if(!IsRingworldStar())
		//return;

	// Locate the ring section with index 0.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaWorldRingSection* ringSection = dynamic_cast<DaWorldRingSection*>(*it);
		if(!ringSection)
			continue;
		ringSection->ActivateHenges();
	}
}

void DaStar::CreateRingworldHenges()
{
	//qDebug() << "DaStar::CreateRingworldHenges" << *this;
	// This is already known to be true.
	//if(!IsRingworldStar())
		//return;

	// Locate the ring section with index 0.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaWorldRingSection* ringSection = dynamic_cast<DaWorldRingSection*>(*it);
		if(!ringSection)
			continue;
		ringSection->CreateHenges();
	}
}

void DaStar::DeactivateRingworldHenges()
{
	// This is already known to be true.
	//if(!IsRingworldStar())
		//return;

	// Locate the ring section with index 0.
	DaAtomList subs;
	ListNormalSubatoms(subs);
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaWorldRingSection* ringSection = dynamic_cast<DaWorldRingSection*>(*it);
		if(!ringSection)
			continue;
		ringSection->DeactivateHenges();
	}
}

bool DaStar::HengeActivated(const AuId& fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
		quint8 destArcSegment)
{
	// We are being notified by a distant world to open a henge gate here.
	DaWorldLL* world;
	if(IsRingworldStar())
	{
		// Locate the destination arc segment on a ringworld.
		world = FindRingworldArc(destArcSegment);
	}
	else
	{
		// I am not a ringworld star.
		world = GetHabitableWorld(HABITABLE_HomeWorld);
		if(!world)
			world = GetHabitableWorld(HABITABLE_System);
		if(!world)
		{
			// No habitable world was found. Any world will do.
			DaAtomList planets;
			FindPlanetsRecursive(planets);

			// Sort these planets by id so we are consistent about the order, which will remain the same if the system regens.
			QMap<AuId, DaWorldLL*> worlds;
			for(DaAtomList::const_iterator si = planets.cbegin(); si != planets.cend(); si++)
			{
				world = dynamic_cast<DaWorldLL*>(*si);
				if(world && world->IsNormal())
					worlds.insert(world->Id(), world);	// Found a world that will work.
			}

			if(worlds.isEmpty())
				return false;

			world = worlds.begin().value();
		}
	}
	if(!world)
		return false;

	// We found a world on which to open a henge portal.
	return world->HengeActivated(fromSysId, fromSun, fromShadowPanels, fromArcSegment);
}

void DaStar::HengeCreated(DaSolarSystem* sys, const AuId& mySectorId, const AuId& fromSysId, quint8 fromSun,
		quint8 fromShadowPanels, quint8 fromArcSegment, quint8 destSun, quint8 destArcSegment)
{
	// We are being notified by a distant world to create a henge gate here.
	DaWorldLL* world;
	if(IsRingworldStar())
	{
		// Locate the destination arc segment on a ringworld.
		world = FindRingworldArc(destArcSegment);
	}
	else
	{
		// I am not a ringworld star.
		world = GetHabitableWorld(HABITABLE_HomeWorld);
		if(!world)
			world = GetHabitableWorld(HABITABLE_System);
		if(!world)
		{
			// No habitable world was found. Any world will do.
			DaAtomList planets;
			FindPlanetsRecursive(planets);

			// Sort these planets by id so we are consistent about the order, which will remain the same if the system regens.
			QMap<AuId, DaWorldLL*> worlds;
			for(DaAtomList::const_iterator si = planets.cbegin(); si != planets.cend(); si++)
			{
				world = dynamic_cast<DaWorldLL*>(*si);
				if(world && world->IsNormal())
					worlds.insert(world->Id(), world);	// Found a world that will work.
			}

			if(worlds.isEmpty())
				return;

			world = worlds.begin().value();
		}
	}
	if(!world)
		return;

	// We found a world on which to open a henge portal.
	world->HengeCreated(sys, mySectorId, fromSysId, fromSun, fromShadowPanels, fromArcSegment, destSun, destArcSegment);
}

void DaStar::HengeDeactivated(const AuId& fromSysId, quint8 fromSun, quint8 fromShadowPanels, quint8 fromArcSegment,
		quint8 destArcSegment)
{
	// We are being notified by a distant world to close a henge gate here.
	DaWorldLL* world;
	if(IsRingworldStar())
	{
		// Locate the destination arc segment on a ringworld.
		world = FindRingworldArc(destArcSegment);
	}
	else
	{
		// I am not a ringworld star.
		world = GetHabitableWorld(HABITABLE_HomeWorld);
		if(!world)
			world = GetHabitableWorld(HABITABLE_System);
		if(!world)
		{
			// No habitable world was found. Any world will do.
			DaAtomList planets;
			FindPlanetsRecursive(planets);

			// Sort these planets by id so we are consistent about the order, which will remain the same if the system regens.
			QMap<AuId, DaWorldLL*> worlds;
			for(DaAtomList::const_iterator si = planets.cbegin(); si != planets.cend(); si++)
			{
				world = dynamic_cast<DaWorldLL*>(*si);
				if(world && world->IsNormal())
					worlds.insert(world->Id(), world);	// Found a world that will work.
			}

			if(worlds.isEmpty())
				return;

			world = worlds.begin().value();
		}
	}
	if(!world)
		return;

	// We found a world.
	world->HengeDeactivated(fromSysId, fromSun, fromShadowPanels, fromArcSegment);
}

void DaStar::HengeTeleportArrived(DaAtom* arrived, quint8 destArc, const AuId& fromSysId, quint8 fromSun, quint8 fromArc)
{
	DaWorldLL* world;
	if(IsRingworldStar())
	{
		// Locate the destination arc segment on a ringworld.
		world = FindRingworldArc(destArc);
	}
	else
	{
		// I am not a ringworld star.
		world = GetHabitableWorld(HABITABLE_HomeWorld);
		if(!world)
			world = GetHabitableWorld(HABITABLE_System);
		if(!world)
		{
			// No habitable world was found. Any world will do.
			DaAtomList planets;
			FindPlanetsRecursive(planets);

			// Sort these planets by id so we are consistent about the order, which will remain the same if the system regens.
			QMap<AuId, DaWorldLL*> worlds;
			for(DaAtomList::const_iterator si = planets.cbegin(); si != planets.cend(); si++)
			{
				world = dynamic_cast<DaWorldLL*>(*si);
				if(world && world->IsNormal())
					worlds.insert(world->Id(), world);	// Found a world that will work.
			}

			if(worlds.isEmpty())
				return;

			world = worlds.begin().value();
		}
	}
	if(!world)
		return;

	world->HengeTeleportArrived(arrived, fromSysId, fromSun, fromArc);
}

void DaStar::SystemScanResources(QMultiMap<quint8, COMMODITYID>& atmRes, QMultiMap<quint8, COMMODITYID>& hydRes,
		QMultiMap<quint8, COMMODITYID>& geoRes, QMultiMap<quint8, COMMODITYID>& bioRes) const
{
	Q_UNUSED(hydRes);
	Q_UNUSED(geoRes);
	Q_UNUSED(bioRes);

	// Make a list of the natural resources the scanner can detect.
	QString cname;
	COMMODITYID starres[] =
	{
		CID_TypeOPreons,
		CID_TypeBPreons,
		CID_TypeAPreons,
		CID_TypeFPreons,
		CID_TypeGPreons,
		CID_TypeKPreons,
		CID_TypeMPreons,
	};
	int starrescount = sizeof(starres)/sizeof(COMMODITYID);
	for(int sri=0; sri<starrescount; sri++)
	{
		COMMODITYID cid = starres[sri];
		if(FetchProbability(cid))
			atmRes.insert(ResourceQuality(cid), cid);
	}
}

// Returns the orbit number of the first frigid orbit. Used when creating previews.
unsigned int DaStar::FirstFrigidOrbit() const
{
	for(unsigned int lOrbit=0; lOrbit<16; lOrbit++)
		if(m_byZones[lOrbit] == ZONE_Frigid)
			return lOrbit;
	return 15;
}

// Returns the orbit number of the last inferno orbit. Used when creating previews.
unsigned int DaStar::LastInfernoOrbit() const
{
	unsigned int inferno = 0;
	for(unsigned int lOrbit=0; lOrbit<16; lOrbit++)
		if(m_byZones[lOrbit] == ZONE_InsideStar)
			inferno = lOrbit+1;
		else if(m_byZones[lOrbit] == ZONE_Inferno)
			inferno = lOrbit;
	return inferno;
}

bool DaStar::InDesignerPreview() const
{
	if(!m_InDesigner)
		m_InDesigner = FindParentDesignerInEnv() ? 2 : 1;
	return m_InDesigner == 2;
}

quint64 DaStar::MemoryUsage() const
{
	quint64 mem = DaAtom::MemoryUsage();
	mem += sizeof(DaStar) - sizeof(DaAtom);
	mem += m_Stargates.MemoryUsage();
	return mem;
}

DaStar* DaStar::FindParentStarInEnv()
{
	return this;
}

const DaStar* DaStar::FindParentStarInEnv() const
{
	return this;
}

// Color of the sun on nav consoles and sensor-type consoles.
//static
QColor DaStar::ChartColor(TYPE eType)
{
	// Colors from the SunColor function.
	QColor starColors[] =
	{
		QColor(56, 98, 255),		// Light Blue
		QColor(87, 125, 255),		// Lighter Blue
		QColor(148, 174, 255),		// Even Lighter Blue
		QColor(242, 240, 255),		// Lightest Blue, Almost White
		QColor(255, 234, 214),		// Yellow
		QColor(255, 165, 66),		// Pale Orange
		QColor(255, 166, 3),		// Red Orange
		QColor(110, 0, 0),			// Red
		QColor(130, 0, 0),			// Red
		QColor(1, 0, 180)			// Blue
	};
	Q_ASSERT(eType >= TYPE_O && eType <= TYPE_NeutronStar);
	return starColors[eType];
}

// Color used for the sun sprite.
QColor DaStar::SunColor() const
{
	switch(StarType())
	{
	case TYPE_O:
	case TYPE_B:
	case TYPE_A:
	case TYPE_F:
	case TYPE_G:
	case TYPE_K:
	case TYPE_M:
		{
			// The color returned is the LightColor with the saturation doubled, up to a max of 99%.
			// The color saturation is increased because we will be doing additive blending with the color.
			QColor sunColor;
			m_StarLock.lockForRead();
			sunColor = m_SunColor;
			m_StarLock.unlock();
			if(sunColor == QColor())
			{
				sunColor = AuGLLight::ColorKelvin(SurfaceTempK());
				qreal h,s,v;
				sunColor.getHsvF(&h,&s,&v);
				s *= 2;
				if(s > .99f)
					s = .99f;
				sunColor.setHsvF(h,s,v);
				m_StarLock.lockForWrite();
				m_SunColor = sunColor;
				m_StarLock.unlock();
			}
			return sunColor;
		}
	case TYPE_SupermassiveBlackHole:
	case TYPE_StellarBlackHole:
		break;
	case TYPE_NeutronStar:
		return QColor(1,0,180);
	}
	return QColor(0,0,0);

	/*
	// The color returned is the LightColor with the saturation doubled, up to a max of 99%.
	// The color saturation is increased because we will be doing additive blending with the color.
	// The values are precalculated here for speed.
	// Colors are based on the star colors from 'http://www.vendian.org/mncharity/dir3/starcolor/', with 2x saturation.
	QColor starColors[] =
	{
		QColor(56, 98, 255),		// Light Blue					HSV: 227, 78, 100
		QColor(87, 125, 255),		// Lighter Blue					HSV: 226, 66, 100
		QColor(148, 174, 255),		// Even Lighter Blue			HSV: 225, 42, 100
		QColor(242, 240, 255),		// Lightest Blue, Almost White	HSV: 248, 6, 100
		QColor(255, 234, 214),		// Yellow						HSV: 29, 16, 100
		QColor(255, 165, 66),		// Pale Orange					HSV: 31, 74, 100
		QColor(255, 166, 3),		// Red Orange					HSV: 39, 99, 100
		QColor(0, 0, 0),			// Black Hole					HSV:
		QColor(0, 0, 0),			// Black Hole					HSV:
		QColor(1, 0, 180)			// Neutron Star					HSV:
	};
	//Q_ASSERT(m_eType >= TYPE_O && m_eType <= TYPE_NeutronStar);
	return starColors[StarType()];
	*/
}

void DaStar::GenNeutronStar(const AuDice& dice, int& typeMod, int& sizeMod)
{
	Q_UNUSED(dice);
	m_StarLock.lockForWrite();
	m_eType = TYPE_NeutronStar;
	m_SurfaceTempK = 0;
	m_bySpecDec = 0;
	m_eSize = SIZE_Ia;					// n/a for neutron star
	m_StarLock.unlock();
	typeMod = 0;
	sizeMod = 0;
}

void DaStar::InitNeutronStarZones()
{
	m_chHabitable = -1;
	m_chRadius = -1;
	m_StarLock.lockForWrite();
	for(unsigned int orbit=0; orbit<16; orbit++)
	{
		if(orbit < 2)
			m_byZones[orbit] = ZONE_Unavailable;				// Orbit is in the blast radius of the supernova.
		else
			m_byZones[orbit] = ZONE_Frigid;
	}
	m_StarLock.unlock();
}

void DaStar::CreateStarSphereModel(AuOpenGLFunctions& GLf)
{
	if(m_CoreSphere.VAO())
		return;

	// Create a vertex array object to be used for our black hole sphere.
	// Tessellate a globe for the atmosphere. This tessellation only expresses latitude and longitude, not altitude of points.
	AuGlobeLLTriList psTris;
	psTris.CreateGlobeTriangles(200);

	// Turn the globe tessellation into globe surface triangles with 3D coordinates.
	double rad = Radius() - 1000;
	QList<AuGlobeTriTex> tris;							// Triangles that make up this model.
	for(AuGlobeLLTriList::const_iterator ti = psTris.cbegin(); ti != psTris.cend(); ti++)
	{
		// Calculate triangle vertex coordinates.
		AuPoint terCoord0 = (*ti).m_LL0.GlobeLocation(rad);		// Location at neutron star core sphere radius.
		AuPoint terCoord1 = (*ti).m_LL1.GlobeLocation(rad);		// Location at neutron star core sphere radius.
		AuPoint terCoord2 = (*ti).m_LL2.GlobeLocation(rad);		// Location at neutron star core sphere radius.

		// Wrap texture properly at the seam.
		float ll0long = (*ti).m_LL0.Longitude();
		if(FuzzyEquals((float)(*ti).m_LL0.Longitude(), -PIf) && ((*ti).m_LL1.Longitude() > 0 || (*ti).m_LL2.Longitude() > 0))
			ll0long = PIf;
		float ll1long = (*ti).m_LL1.Longitude();
		if(FuzzyEquals((float)(*ti).m_LL1.Longitude(), -PIf) && ((*ti).m_LL0.Longitude() > 0 || (*ti).m_LL2.Longitude() > 0))
			ll1long = PIf;
		float ll2long = (*ti).m_LL2.Longitude();
		if(FuzzyEquals((float)(*ti).m_LL2.Longitude(), -PIf) && ((*ti).m_LL0.Longitude() > 0 || (*ti).m_LL1.Longitude() > 0))
			ll2long = PIf;

		// Calculate texture coordinates.
		QPointF terTex0;
		terTex0.setX(ll0long/(PIf*2) + .5f);
		terTex0.setY((*ti).m_LL0.Latitude()/PIf + .5f);
		QPointF terTex1;
		terTex1.setX(ll1long/(PIf*2) + .5f);
		terTex1.setY((*ti).m_LL1.Latitude()/PIf + .5f);
		QPointF terTex2;
		terTex2.setX(ll2long/(PIf*2) + .5f);
		terTex2.setY((*ti).m_LL2.Latitude()/PIf + .5f);

		// Construct a globe triangle with texture coordinates.
		AuGlobeTriTex tri;
		tri.m_Vert0 = AuGlobeVertTex(terCoord0, terTex0);
		tri.m_Vert1 = AuGlobeVertTex(terCoord1, terTex1);
		tri.m_Vert2 = AuGlobeVertTex(terCoord2, terTex2);

		// Append the globe triangle to the list.
		tris.append(tri);
	}

	// Store count of triangles in the black hole sphere vertex array object.
	GLuint vertices = tris.count()*3;

	// Copy surface triangles into gl array buffers.
	GLfloat* texCoord = new GLfloat[vertices*2];
	GLfloat* globePos = new GLfloat[vertices*3];			// vertices for triangles that make up the model.

	unsigned int tidx = 0;
	unsigned int globePosIdx = 0;
	for(QList<AuGlobeTriTex>::const_iterator vi = tris.cbegin(); vi != tris.cend(); vi++)
	{
		// Vertex 0
		texCoord[tidx++] = (float)(*vi).m_Vert0.m_TexCoord.x();
		texCoord[tidx++] = (float)(*vi).m_Vert0.m_TexCoord.y();

		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.z;

		// Vertex 1
		texCoord[tidx++] = (float)(*vi).m_Vert1.m_TexCoord.x();
		texCoord[tidx++] = (float)(*vi).m_Vert1.m_TexCoord.y();

		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.z;

		// Vertex 2
		texCoord[tidx++] = (float)(*vi).m_Vert2.m_TexCoord.x();
		texCoord[tidx++] = (float)(*vi).m_Vert2.m_TexCoord.y();

		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.z;
	}

	// Generate vertex array(s) for this object. We only need one.
	m_CoreSphere.CreateGL(GLf, vertices, 2);

	// Select vertex array to construct.
	m_CoreSphere.SelectGL();

	// bind buffer for vertices and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_CoreSphere.Buffer(0));
	GLf.glBufferData(GL_ARRAY_BUFFER, globePosIdx*sizeof(GLfloat), globePos, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for tex coords and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_CoreSphere.Buffer(1));
	GLf.glBufferData(GL_ARRAY_BUFFER, tidx*sizeof(GLfloat), texCoord, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Disable the vertex array object.
	m_CoreSphere.DeselectGL();

	delete [] globePos;
	delete [] texCoord;
}

void DaStar::CreateNeutronStarSphereModel(AuOpenGLFunctions& GLf)
{
	if(m_CoreSphere.VAO())
		return;

	// Create a vertex array object to be used for our black hole sphere.
	// Tessellate a globe for the atmosphere. This tessellation only expresses latitude and longitude, not altitude of points.
	AuGlobeLLTriList psTris;
	psTris.CreateGlobeTriangles(200);

	// Turn the globe tessellation into globe surface triangles with 3D coordinates.
	double rad = Radius();
	QList<AuGlobeTriTex> tris;							// Triangles that make up this model.
	for(AuGlobeLLTriList::const_iterator ti = psTris.cbegin(); ti != psTris.cend(); ti++)
	{
		// Calculate triangle vertex coordinates.
		AuPoint terCoord0 = (*ti).m_LL0.GlobeLocation(rad);		// Location at neutron star core sphere radius.
		AuPoint terCoord1 = (*ti).m_LL1.GlobeLocation(rad);		// Location at neutron star core sphere radius.
		AuPoint terCoord2 = (*ti).m_LL2.GlobeLocation(rad);		// Location at neutron star core sphere radius.

		// Wrap texture properly at the seam.
		float ll0long = (*ti).m_LL0.Longitude();
		if(FuzzyEquals((float)(*ti).m_LL0.Longitude(), -PIf) && ((*ti).m_LL1.Longitude() > 0 || (*ti).m_LL2.Longitude() > 0))
			ll0long = PIf;
		float ll1long = (*ti).m_LL1.Longitude();
		if(FuzzyEquals((float)(*ti).m_LL1.Longitude(), -PIf) && ((*ti).m_LL0.Longitude() > 0 || (*ti).m_LL2.Longitude() > 0))
			ll1long = PIf;
		float ll2long = (*ti).m_LL2.Longitude();
		if(FuzzyEquals((float)(*ti).m_LL2.Longitude(), -PIf) && ((*ti).m_LL0.Longitude() > 0 || (*ti).m_LL1.Longitude() > 0))
			ll2long = PIf;

		// Calculate texture coordinates.
		QPointF terTex0;
		terTex0.setX(ll0long/(PIf*2) + .5f);
		terTex0.setY((*ti).m_LL0.Latitude()/PIf + .5f);
		QPointF terTex1;
		terTex1.setX(ll1long/(PIf*2) + .5f);
		terTex1.setY((*ti).m_LL1.Latitude()/PIf + .5f);
		QPointF terTex2;
		terTex2.setX(ll2long/(PIf*2) + .5f);
		terTex2.setY((*ti).m_LL2.Latitude()/PIf + .5f);

		// Construct a globe triangle with texture coordinates.
		AuGlobeTriTex tri;
		tri.m_Vert0 = AuGlobeVertTex(terCoord0, terTex0);
		tri.m_Vert1 = AuGlobeVertTex(terCoord1, terTex1);
		tri.m_Vert2 = AuGlobeVertTex(terCoord2, terTex2);

		// Append the globe triangle to the list.
		tris.append(tri);
	}

	// Store count of triangles in the black hole sphere vertex array object.
	GLuint vertices = tris.count()*3;

	// Copy surface triangles into gl array buffers.
	GLfloat* texCoord = new GLfloat[vertices*2];
	GLfloat* globePos = new GLfloat[vertices*3];			// vertices for triangles that make up the model.

	unsigned int tidx = 0;
	unsigned int globePosIdx = 0;
	for(QList<AuGlobeTriTex>::const_iterator vi = tris.cbegin(); vi != tris.cend(); vi++)
	{
		// Vertex 0
		texCoord[tidx++] = (float)(*vi).m_Vert0.m_TexCoord.x();
		texCoord[tidx++] = (float)(*vi).m_Vert0.m_TexCoord.y();

		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.z;

		// Vertex 1
		texCoord[tidx++] = (float)(*vi).m_Vert1.m_TexCoord.x();
		texCoord[tidx++] = (float)(*vi).m_Vert1.m_TexCoord.y();

		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.z;

		// Vertex 2
		texCoord[tidx++] = (float)(*vi).m_Vert2.m_TexCoord.x();
		texCoord[tidx++] = (float)(*vi).m_Vert2.m_TexCoord.y();

		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.z;
	}

	// Generate vertex array(s) for this object. We only need one.
	m_CoreSphere.CreateGL(GLf, vertices, 2);

	// Select vertex array to construct.
	m_CoreSphere.SelectGL();

	// bind buffer for vertices and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_CoreSphere.Buffer(0));
	GLf.glBufferData(GL_ARRAY_BUFFER, globePosIdx*sizeof(GLfloat), globePos, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for tex coords and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_CoreSphere.Buffer(1));
	GLf.glBufferData(GL_ARRAY_BUFFER, tidx*sizeof(GLfloat), texCoord, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Disable the vertex array object.
	m_CoreSphere.DeselectGL();

	delete [] globePos;
	delete [] texCoord;
}

void DaStar::CreateCoronaSphereModel(AuOpenGLFunctions& GLf)
{
	if(m_CoronaSphere.VAO())
		return;

	// Create a vertex array object to be used for our neutron sphere.
	// Tessellate a globe for the atmosphere. This tessellation only expresses latitude and longitude, not altitude of points.
	AuGlobeLLTriList psTris;
	psTris.CreateGlobeTriangles(200);

	// Turn the globe tessellation into globe surface triangles with 3D coordinates and texture coordinates.
	double rad = Radius();
	QList<AuGlobeTriTex> tris;							// Triangles that make up this model.
	for(AuGlobeLLTriList::const_iterator ti = psTris.cbegin(); ti != psTris.cend(); ti++)
	{
		// Calculate triangle vertex coordinates.
		AuPoint terCoord0 = (*ti).m_LL0.GlobeLocation(rad);		// Location at neutron sphere radius.
		AuPoint terCoord1 = (*ti).m_LL1.GlobeLocation(rad);		// Location at neutron sphere radius.
		AuPoint terCoord2 = (*ti).m_LL2.GlobeLocation(rad);		// Location at neutron sphere radius.

		// Wrap texture properly at the seam.
		float ll0long = (*ti).m_LL0.Longitude();			// Range is -PI to PI
		if(FuzzyEquals((float)(*ti).m_LL0.Longitude(), -PIf) && ((*ti).m_LL1.Longitude() > 0 || (*ti).m_LL2.Longitude() > 0))
			ll0long = PIf;
		float ll1long = (*ti).m_LL1.Longitude();
		if(FuzzyEquals((float)(*ti).m_LL1.Longitude(), -PIf) && ((*ti).m_LL0.Longitude() > 0 || (*ti).m_LL2.Longitude() > 0))
			ll1long = PIf;
		float ll2long = (*ti).m_LL2.Longitude();
		if(FuzzyEquals((float)(*ti).m_LL2.Longitude(), -PIf) && ((*ti).m_LL0.Longitude() > 0 || (*ti).m_LL1.Longitude() > 0))
			ll2long = PIf;

		// Calculate texture coordinates.
		QPointF terTex0;
		terTex0.setX(ll0long/(PIf*2) + .5f);
		terTex0.setY((*ti).m_LL0.Latitude()/PIf + .5f);
		QPointF terTex1;
		terTex1.setX(ll1long/(PIf*2) + .5f);
		terTex1.setY((*ti).m_LL1.Latitude()/PIf + .5f);
		QPointF terTex2;
		terTex2.setX(ll2long/(PIf*2) + .5f);
		terTex2.setY((*ti).m_LL2.Latitude()/PIf + .5f);

		// Construct a globe triangle with texture coordinates.
		AuGlobeTriTex tri;
		tri.m_Vert0 = AuGlobeVertTex(terCoord0, terTex0);
		tri.m_Vert1 = AuGlobeVertTex(terCoord1, terTex1);
		tri.m_Vert2 = AuGlobeVertTex(terCoord2, terTex2);

		// Append the globe triangle to the list.
		tris.append(tri);
	}

	// Store count of triangles in the photon sphere vertex array object.
	GLuint vertices = tris.count()*3;

	// Copy surface triangles into gl array buffers.
	GLfloat* globePos = new GLfloat[vertices*3];			// vertices for triangles that make up the model.
	GLfloat* globeTex = new GLfloat[vertices*2];			// texture coords for each of the vertices

	unsigned int globePosIdx = 0;
	unsigned int globeTexIdx = 0;
	for(QList<AuGlobeTriTex>::const_iterator vi = tris.cbegin(); vi != tris.cend(); vi++)
	{
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.z;

		globeTex[globeTexIdx++] = (float)(*vi).m_Vert0.m_TexCoord.x();
		globeTex[globeTexIdx++] = (float)(*vi).m_Vert0.m_TexCoord.y();

		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.z;

		globeTex[globeTexIdx++] = (float)(*vi).m_Vert1.m_TexCoord.x();
		globeTex[globeTexIdx++] = (float)(*vi).m_Vert1.m_TexCoord.y();

		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.z;

		globeTex[globeTexIdx++] = (float)(*vi).m_Vert2.m_TexCoord.x();
		globeTex[globeTexIdx++] = (float)(*vi).m_Vert2.m_TexCoord.y();
	}

	// Generate vertex array(s) for this object. We only need one.
	m_CoronaSphere.CreateGL(GLf, vertices, 2);

	// Select vertex array to construct.
	m_CoronaSphere.SelectGL();

	// bind buffer for vertices and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_CoronaSphere.Buffer(0));
	GLf.glBufferData(GL_ARRAY_BUFFER, globePosIdx*sizeof(GLfloat), globePos, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for colors and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_CoronaSphere.Buffer(1));
	GLf.glBufferData(GL_ARRAY_BUFFER, globeTexIdx*sizeof(GLfloat), globeTex, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Disable the vertex array object.
	m_CoronaSphere.DeselectGL();

	delete [] globePos;
	delete [] globeTex;
}

void DaStar::CreateNeutronSphereModel(AuOpenGLFunctions& GLf)
{
	if(m_CoronaSphere.VAO())
		return;

	// Create a vertex array object to be used for our neutron sphere.
	// Tessellate a globe for the atmosphere. This tessellation only expresses latitude and longitude, not altitude of points.
	AuGlobeLLTriList psTris;
	psTris.CreateGlobeTriangles(200);

	// Turn the globe tessellation into globe surface triangles with 3D coordinates and texture coordinates.
	double rad = NeutronSphereRadius();
	QList<AuGlobeTriTex> tris;							// Triangles that make up this model.
	for(AuGlobeLLTriList::const_iterator ti = psTris.cbegin(); ti != psTris.cend(); ti++)
	{
		// Calculate triangle vertex coordinates.
		AuPoint terCoord0 = (*ti).m_LL0.GlobeLocation(rad);		// Location at neutron sphere radius.
		AuPoint terCoord1 = (*ti).m_LL1.GlobeLocation(rad);		// Location at neutron sphere radius.
		AuPoint terCoord2 = (*ti).m_LL2.GlobeLocation(rad);		// Location at neutron sphere radius.

		// Wrap texture properly at the seam.
		float ll0long = (*ti).m_LL0.Longitude();			// Range is -PI to PI
		if(FuzzyEquals((float)(*ti).m_LL0.Longitude(), -PIf) && ((*ti).m_LL1.Longitude() > 0 || (*ti).m_LL2.Longitude() > 0))
			ll0long = PIf;
		float ll1long = (*ti).m_LL1.Longitude();
		if(FuzzyEquals((float)(*ti).m_LL1.Longitude(), -PIf) && ((*ti).m_LL0.Longitude() > 0 || (*ti).m_LL2.Longitude() > 0))
			ll1long = PIf;
		float ll2long = (*ti).m_LL2.Longitude();
		if(FuzzyEquals((float)(*ti).m_LL2.Longitude(), -PIf) && ((*ti).m_LL0.Longitude() > 0 || (*ti).m_LL1.Longitude() > 0))
			ll2long = PIf;

		// Calculate texture coordinates.
		QPointF terTex0;
		terTex0.setX(ll0long/(PIf*2) + .5f);
		terTex0.setY((*ti).m_LL0.Latitude()/PIf + .5f);
		QPointF terTex1;
		terTex1.setX(ll1long/(PIf*2) + .5f);
		terTex1.setY((*ti).m_LL1.Latitude()/PIf + .5f);
		QPointF terTex2;
		terTex2.setX(ll2long/(PIf*2) + .5f);
		terTex2.setY((*ti).m_LL2.Latitude()/PIf + .5f);

		// Construct a globe triangle with texture coordinates.
		AuGlobeTriTex tri;
		tri.m_Vert0 = AuGlobeVertTex(terCoord0, terTex0);
		tri.m_Vert1 = AuGlobeVertTex(terCoord1, terTex1);
		tri.m_Vert2 = AuGlobeVertTex(terCoord2, terTex2);

		// Append the globe triangle to the list.
		tris.append(tri);
	}

	// Store count of triangles in the photon sphere vertex array object.
	GLuint vertices = tris.count()*3;

	// Copy surface triangles into gl array buffers.
	GLfloat* globePos = new GLfloat[vertices*3];			// vertices for triangles that make up the model.
	GLfloat* globeTex = new GLfloat[vertices*2];			// texture coords for each of the vertices

	unsigned int globePosIdx = 0;
	unsigned int globeTexIdx = 0;
	for(QList<AuGlobeTriTex>::const_iterator vi = tris.cbegin(); vi != tris.cend(); vi++)
	{
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert0.m_Loc.z;

		globeTex[globeTexIdx++] = (float)(*vi).m_Vert0.m_TexCoord.x();
		globeTex[globeTexIdx++] = (float)(*vi).m_Vert0.m_TexCoord.y();

		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert1.m_Loc.z;

		globeTex[globeTexIdx++] = (float)(*vi).m_Vert1.m_TexCoord.x();
		globeTex[globeTexIdx++] = (float)(*vi).m_Vert1.m_TexCoord.y();

		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.x;
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.y;
		globePos[globePosIdx++] = (float)(*vi).m_Vert2.m_Loc.z;

		globeTex[globeTexIdx++] = (float)(*vi).m_Vert2.m_TexCoord.x();
		globeTex[globeTexIdx++] = (float)(*vi).m_Vert2.m_TexCoord.y();
	}

	// Generate vertex array(s) for this object. We only need one.
	m_CoronaSphere.CreateGL(GLf, vertices, 2);

	// Select vertex array to construct.
	m_CoronaSphere.SelectGL();

	// bind buffer for vertices and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_CoronaSphere.Buffer(0));
	GLf.glBufferData(GL_ARRAY_BUFFER, globePosIdx*sizeof(GLfloat), globePos, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	// bind buffer for colors and copy data into buffer.
	GLf.glBindBuffer(GL_ARRAY_BUFFER, m_CoronaSphere.Buffer(1));
	GLf.glBufferData(GL_ARRAY_BUFFER, globeTexIdx*sizeof(GLfloat), globeTex, GL_STATIC_DRAW);
	GLf.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);		// Describe the data in the buffer.

	GLf.glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Disable the vertex array object.
	m_CoronaSphere.DeselectGL();

	delete [] globePos;
	delete [] globeTex;
}

void DaStar::RenderNeutronStar(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re)
{
	if(!m_SolarDisk.VAO())
		return;
	if(!m_CoreSphere.VAO())
		return;
	if(!m_CoronaSphere.VAO())
		return;

	// Orient flat star to face viewer.
	// Calculate the vector from the center of the sun to the eye, in sun space not eye space.
	// This calculation eliminates the tipping effect of the sun but makes it hard to fly near the sun
	// without getting destroyed.
	//AuPoint vup = ts[star].UnProject(0,0,0);	// Project the eye point from eye space into sun space.
	//vup.Unit();										// Unit vector in sun space from center of sun to the eye.
	// Orienting the sun flat to the camera plane makes flying near the sun easier but gives the sun a tipping
	// effect when looking around while near the sun. The tipping sun give a more accurate sense of where the edge
	// of the sun really is, like a rotating cross-section of the sun's sphere.
	AuPoint vup = model.UnProjectVector(AuPoint(0,0,1));
	vup.Unit();
	AuMatrix4x4 ta;
	{
		// This code was whittled down from AuPoint::GetAlignmentTransform. We know the vectors are unit vectors so
		// the math here is a lot simpler.
		// Determine normal vector to the two vectors.
		AuPoint vN = vup.Cross(AuPoint(0,0,1));
		if(!vN.IsZero())
		{
			// Rotate the transformation matrix around the normal vector by the angle between the vectors.
			vN.Unit();
			double rAng = vup.AngleTo(AuPoint(0,0,1));
			ta.RotateAxis(-rAng, vN);
		}
	}

	QColor sunColor(SunColor());
	float neutronOutput = NeutronStarOutput();

	re.GLSL().TextureShader().Select();
	re.GLSL().TextureShader().SetColor(sunColor.redF(), sunColor.greenF(), sunColor.blueF(), neutronOutput);
	re.GLSL().TextureShader().SetTexture(m_TexSun);

	// Glowing aura
	{
		// The 2D angle of the eye space Z vector tells us how much to rotate the sun polygon around its Z axis so the
		// sun stays consistently aligned with the eye.
		ta.RotateZ(vup.ArcTangent_2D());
		AuMatrix4x4 mv = model * ta;

		re.glDepthMask(GL_FALSE);							// Turn off depth mask.
		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);				// Remaining shader output is added to destination.

		// Add the sun glow that is larger than the solar disk.
		re.GLSL().TextureShader().SetProjectionModelView(projection * mv);
		m_SolarDisk.SelectGL();
		m_SolarDisk.DrawGL();
		m_SolarDisk.DeselectGL();

		re.glDepthMask(GL_TRUE);
		re.glDisable(GL_BLEND);
	}

	// Core
	{
		// Render neutron star sphere model.
		re.glDisable(GL_CULL_FACE);									// So we don't see through it at great distances.

		AuMatrix4x4 mps(model);
		mps.Rotate(m_SphereRot);

		re.GLSL().TextureShader().SetProjectionModelView(projection * mps);
		re.GLSL().TextureShader().SetTexture(m_TexNoise);
		re.GLSL().TextureShader().SetColor(sunColor.redF()/2, sunColor.greenF()/2, sunColor.blueF()/2);

		m_CoreSphere.SelectGL();
		m_CoreSphere.DrawGL();
		m_CoreSphere.DeselectGL();

		re.glEnable(GL_CULL_FACE);
	}

	// Photon sphere
	{
		// Render photon sphere.
		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE);						// Additive blend for the photon sphere.
		re.glDepthMask(GL_FALSE);									// Turn off depth mask.

		re.GLSL().TextureShader().SetTexture(m_TexLightning);

		m_CoronaSphere.SelectGL();

		// Add some rotating textures for a field effect.
		float r = sunColor.redF()/2 + neutronOutput/2;
		if(r > 1)
			r = 1;
		float g = sunColor.greenF()/2;
		if(g > 1)
			g = 1;
		float b = sunColor.blueF()/2 +  neutronOutput;
		if(b > 1)
			b = 1;
		re.GLSL().TextureShader().SetColor(r,g,b);

		AuPoint rotation = m_SphereRot * 1.5;
		for(int i=0; i<2; i++)
		{
			AuMatrix4x4 mps(model);
			mps.Rotate(rotation);
			if(i)
			{
				double scl = 1.1;
				mps.Scale(scl,scl,scl);
			}
			re.GLSL().TextureShader().SetProjectionModelView(projection * mps);
			m_CoronaSphere.DrawGL();
			rotation *= 2;
		}

		m_CoronaSphere.DeselectGL();

		re.glDepthMask(GL_TRUE);
		re.glDisable(GL_BLEND);
	}
	re.glDepthFunc(GL_LESS);

	re.GLSL().TextureShader().Deselect();
}

COMMODITYID DaStar::CorrespondingHarvestResource(COMMODITYID cidToHarvest) const
{
	switch(cidToHarvest)
	{
	case CID_TypeOPreons:
	case CID_TypeBPreons:
	case CID_TypeAPreons:
	case CID_TypeFPreons:
	case CID_TypeGPreons:
	case CID_TypeKPreons:
	case CID_TypeMPreons:
		// Return the proper type of preons for this star.
		switch(StarType())
		{
		case TYPE_O:
			return CID_TypeOPreons;
		case TYPE_B:
			return CID_TypeBPreons;
		case TYPE_A:
			return CID_TypeAPreons;
		case TYPE_F:
			return CID_TypeFPreons;
		case TYPE_G:
			return CID_TypeGPreons;
		case TYPE_K:
			return CID_TypeKPreons;
		case TYPE_M:
			return CID_TypeMPreons;
		case TYPE_SupermassiveBlackHole:
		case TYPE_StellarBlackHole:
		case TYPE_NeutronStar:
			break;
		}
		break;
	default:
		break;
	}
	return cidToHarvest;
}

DaAtom::ATOM_TYPE DaStar::AtomType() const
{
	return ATOM_Star;
}

bool DaStar::FindNaturalResource(COMMODITYID resCID, quint8 minQ) const
{
	return ResourceQuality(resCID) >= minQ;
}

void DaStar::operator>>(QDataStream& out) const
{
	DaAtom::operator>>(out);
	out << m_bySpecDec;
	out << (qint8)m_chHabitable;
	out << (qint8)m_chOrbit;
	out << (qint8)m_chRadius;
	out << m_InDesigner;
	out << m_StarNumber;
	out << m_ZoneOfControl;

	m_StarLock.lockForRead();
	out << m_AxialTilt;
	for(int z=0; z<16; z++)
		out << m_byZones[z];
	out << (quint8)m_eSize;

	quint8 vertype = 0x10 | m_eType;
	out << vertype;
	out << m_SurfaceTempK;

	out << m_Stargates;
	out << AuTime();	//m_StargatesChange;

	out << m_RingworldRefSection;
	m_StarLock.unlock();
}

void DaStar::operator<<(QDataStream& in)
{
	DaAtom::operator<<(in);
	in >> m_bySpecDec;
	qint8 i8;
	in >> i8;
	m_chHabitable = i8;
	in >> i8;
	m_chOrbit = i8;
	in >> i8;
	m_chRadius = i8;
	in >> m_InDesigner;
	in >> m_StarNumber;
	in >> m_ZoneOfControl;

	m_StarLock.lockForWrite();
	in >> m_AxialTilt;
	for(int z=0; z<16; z++)
		in >> m_byZones[z];
	quint8 ui8;
	in >> ui8;
	m_eSize = (SIZE)ui8;
	in >> ui8;
	m_eType = (TYPE)(ui8 & 0x0f);
	if(ui8 & 0x10)
		in >> m_SurfaceTempK;
	else
	{
		AuDice dice;
		dice.Seed(Id().GetID());
		GenSurfaceTempK(dice);
	}

	in >> m_Stargates;
	AuTime _StargatesChange;
	in >> _StargatesChange;

	in >> m_RingworldRefSection;
	m_StarLock.unlock();
}

double DaStar::AxialTilt() const
{
	m_StarLock.lockForRead();
	double v = m_AxialTilt;
	m_StarLock.unlock();
	if(v == 0)
	{
		// Calculate and cache my axial tilt.
		AuDice dice(GenSeed());
		v = dice.RandomGaussianDistribution(0, (PI/2.)/3.);
		if(v == 0)
			v = .00001;	// Close enough to prevent recaching.
		m_StarLock.lockForWrite();
		m_AxialTilt = v;
		m_StarLock.unlock();
		//qDebug() << "DaStar::AxialTilt" << *this << "tilt" << (v*RAD2DEG);
	}
	return v;
}

BODY_TYPE DaStar::BodyType() const
{
	BODY_TYPE byStarType[] =
	{
		BODY_TypeOStar,				// TYPE_O,
		BODY_TypeBStar,				// TYPE_B,
		BODY_TypeAStar,				// TYPE_A,
		BODY_TypeFStar,				// TYPE_F,
		BODY_TypeGStar,				// TYPE_G,
		BODY_TypeKStar,				// TYPE_K,
		BODY_TypeMStar,				// TYPE_M,
		BODY_SupermassiveBlackHole,	// TYPE_SupermassiveBlackHole,
		BODY_StellarBlackHole,		// TYPE_StellarBlackHole,
		BODY_NeutronStar,			// TYPE_NeutronStar
	};
	return byStarType[StarType()];
}

bool DaStar::CouldHaveRingworld(quint32 systemGenSeed, const DaAtomList& starsInSystem)
{
	// This is cobbled from CreatePlanetsForScan.
	if(IsAnyBlackHole())
		return false;
	int starNumber = StarNumber(starsInSystem);
	AuDice dice(GenSeed(systemGenSeed, starNumber));

	// Mark outer orbits as unavailable.
	InitMaxOrbits(dice);
	dice.Random();

	dice.Roll(2,6);	// lPlanets

	// In our happy little universe, there's always a planet in the habitable zone if the zone is available.
	m_StarLock.lockForRead();
	bool v = m_chHabitable >= 0 && m_chHabitable < 16 && m_byZones[(int)m_chHabitable] == ZONE_Habitable;
	m_StarLock.unlock();
	if(!v)
		return false;	// There is no planet here.

	// There IS a planet here.
	// Note that the planet may not be habitable even though it is in the habitable zone.
	// Check for ringworld presense. Homeworlds are never ringworlds, though it is remotely possible a companion star in a
	// homeworld system could have a ringworld; that would be fun.
	if(m_chHabitable >= 3)
		return false;	// Habitable orbit is too large.

	// The habitable zone is 0 through 3. A ringworld could fit here if there are no secondary stars too close.
	if(IsPrimary())
	{
		// Only the primary can have secondary stars. Secondary stars have safely reserved their zone of control.
		// Look for a secondary star that is inside the habitable orbit.
		for(DaAtomList::const_iterator it = starsInSystem.cbegin(); it != starsInSystem.cend(); it++)
		{
			DaStar* star = dynamic_cast<DaStar*>(*it);
			if(!star)
				continue;
			if(star == this)
				continue;
			if(star->Orbit() < m_chHabitable)
			{
				// Found a secondary star orbiting inside the habitable orbit of the primary star.
				// A ringworld cannot be placed around the primary star.
				return false;
			}
		}
	}

	// The area is not blocked to ringworlds.
	// Give it a 3% chance of being a ringworld system.
	return dice.Roll(1,100) <= 3;
}

double DaStar::IntergalacticWormholeDistance() const
{
	// This calculation places the intergalactic wormholes outside the pilot's obstacle avoidance range, which is
	// the PhotonSphereRadius + RoughRadius of his ship.
	return PhotonSphereRadius() + AuScDesign::HullSizeLimit()*4;
}

